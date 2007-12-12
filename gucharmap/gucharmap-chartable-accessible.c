/*
 * Copyright (c) 2003  Sun Microsystems Inc.
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 2 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public
 * License along with this library; if not, write to the
 * Free Software Foundation, Inc., 59 Temple Place - Suite 330,
 * Boston, MA 02111-1307, USA.
 */

#include <config.h>

#include <string.h>

#include <gtk/gtk.h>

#include "gucharmap.h"
#include "gucharmap-intl.h"
#include "gucharmap-chartable.h"
#include "gucharmap-chartable-private.h"
#include "gucharmap-chartable-accessible.h"
#include "gucharmap-chartable-cell-accessible.h"

typedef struct
{
  AtkObject *cell;
  gint      index;
} GucharmapChartableCellAccessibleInfo;

static gpointer gucharmap_chartable_accessible_parent_class;

static GList*
get_cell_list (GucharmapChartableAccessible *accessible)
{
  return accessible->cells;
}

static void
set_cell_list (GucharmapChartableAccessible *accessible, GList *list)
{
  accessible->cells = list;
}

static AtkObject*
find_cell (GucharmapChartableAccessible *table,
	   gint index)
{
  GucharmapChartableCellAccessibleInfo *info;
  GList *cell_list;
  GList *l;

  cell_list = get_cell_list (table);

  for (l = cell_list; l; l = l->next)
    {
      info = (GucharmapChartableCellAccessibleInfo *) (l->data);

      if (index == info->index)
        return info->cell;
    }

  return NULL;
}

static void
set_cell_visibility (GucharmapChartable  *chartable,
                     GucharmapChartableCellAccessible  *cell,
                     gboolean emit_signal)
{
  gucharmap_chartable_cell_accessible_add_state (cell, ATK_STATE_VISIBLE, emit_signal);

  if (cell->index >= chartable->page_first_cell &&
      cell->index < chartable->page_first_cell 
                    + chartable->rows * chartable->cols)
    {
      gucharmap_chartable_cell_accessible_add_state (cell, ATK_STATE_SHOWING, emit_signal);
    }
  else 
    {
      gucharmap_chartable_cell_accessible_remove_state (cell, ATK_STATE_SHOWING, emit_signal);
    }
}


static GucharmapChartableCellAccessibleInfo*
find_cell_info (GucharmapChartableAccessible *table,
		AtkObject *cell,
		GList **list)
{
  GList *l;
  GList *cell_list;
  GucharmapChartableCellAccessibleInfo *cell_info;

  cell_list = get_cell_list (table);
  for (l = cell_list; l; l= l->next)
    {
      cell_info = (GucharmapChartableCellAccessibleInfo *) l->data;
      if (cell_info->cell == cell)
        {
          if (list)
            *list = l;
          return cell_info;
        }
    }

  return NULL;
}


static void
cell_info_remove (GucharmapChartableAccessible  *table,
		  AtkObject *cell)
{
  GucharmapChartableCellAccessibleInfo *info;
  GList *l, *cell_list;

  info = find_cell_info (table, cell, &l);
  if (info)
    {
      cell_list = get_cell_list (table);
      set_cell_list (table, g_list_remove_link (cell_list, l));
      g_free (info);
      return;
    }

  g_warning ("No cell removed in cell_info_remove\n");
}


static void
cell_destroyed (gpointer data)
{
  AtkObject *cell;
  AtkObject *parent;

  g_return_if_fail (IS_GUCHARMAP_CHARTABLE_CELL_ACCESSIBLE (data));

  cell = ATK_OBJECT (data);
 
  parent = atk_object_get_parent (cell); 
  g_return_if_fail (IS_GUCHARMAP_CHARTABLE_ACCESSIBLE (parent));
  cell_info_remove (GUCHARMAP_CHARTABLE_ACCESSIBLE (parent), cell);
}


static void
cell_info_new (GucharmapChartableAccessible *table,
               AtkObject *cell,
               gint index)
{
  GucharmapChartableCellAccessibleInfo *info;
  GList *cell_list;

  info = g_new (GucharmapChartableCellAccessibleInfo, 1);
  info->cell = cell;
  info->index = index;

  cell_list = get_cell_list (table);
  /* FIXME: g_list_append? come on! */
  set_cell_list (table, g_list_append (cell_list, info));
  /* Setup weak reference notification */

  g_object_weak_ref (G_OBJECT (cell),
                     (GWeakNotify) cell_destroyed,
                     cell);
}


static AtkObject*
gucharmap_chartable_accessible_ref_child (AtkObject *obj, gint i)
{
  GtkWidget *widget;
  GucharmapChartable *chartable;
  AtkObject *child;
  GucharmapChartableAccessible *table;
  gchar* name;

  widget = GTK_ACCESSIBLE (obj)->widget;
  if (widget == NULL)
    /* State is defunct */
    return NULL;

  if (i > UNICHAR_MAX)
    return NULL;

  chartable = GUCHARMAP_CHARTABLE (widget);
  table = GUCHARMAP_CHARTABLE_ACCESSIBLE (obj);

  /*
   * Check whether the child is cached
   */
  child = find_cell (table, i);
  if (child)
    {
      return g_object_ref (child);
    } 

  child = gucharmap_chartable_cell_accessible_new ();
  gucharmap_chartable_cell_accessible_initialise (GUCHARMAP_CHARTABLE_CELL_ACCESSIBLE (child),
                            GTK_WIDGET (chartable), obj, i);
  /* Set the name of the cell */
  name = g_strdup_printf("U+%4.4X %s", i, gucharmap_get_unicode_name (i));
  atk_object_set_name (child, name);
  g_free (name);
  set_cell_visibility (chartable, GUCHARMAP_CHARTABLE_CELL_ACCESSIBLE (child), FALSE);
  cell_info_new (table, child, i);

  return child; 
}


static AtkObject* 
gucharmap_chartable_accessible_ref_at (AtkTable *table,
                             gint    row,
                             gint    column)
{
  GtkWidget *widget;
  GucharmapChartable *chartable;
  AtkObject *child;
  gint index;

  widget = GTK_ACCESSIBLE (table)->widget;
  if (widget == NULL)
    /* State is defunct */
    return NULL;

  chartable = GUCHARMAP_CHARTABLE (widget);

  index =  row * chartable->cols + column;

  child = gucharmap_chartable_accessible_ref_child (ATK_OBJECT (table), index);

  return child;
}


static AtkObject*
gucharmap_chartable_accessible_ref_accessible_at_point (AtkComponent *component,
                                              gint x, 
                                              gint y,
                                              AtkCoordType coord_type)
{
  GtkWidget *widget;
  GucharmapChartable *chartable;
  gint x_pos, y_pos;
  gint row, col;

  widget = GTK_ACCESSIBLE (component)->widget;
  if (widget == NULL)
    /* State is defunct */
    return NULL;

  chartable = GUCHARMAP_CHARTABLE (widget);

  atk_component_get_extents (component, &x_pos, &y_pos,
                             NULL, NULL, coord_type);

  /* Find cell at offset x - x_pos, y - y_pos */

  x_pos = x - x_pos;
  y_pos = y - y_pos;

  for (col = 0; col < chartable->cols; col++) 
    {
      if (x_pos < _gucharmap_chartable_x_offset (chartable, col))
        {
          col--;
          break;
        }
    }
  if (col == chartable->cols || col < 0)
    return NULL;

  for (row = 0; row < chartable->rows; row++) 
    {
      if (y_pos < _gucharmap_chartable_y_offset (chartable, row))
        {
          row--;
          break;
        }
    }
  if (row == chartable->rows || row < 0)
    return NULL;

  row += chartable->page_first_cell / chartable->cols;

  return gucharmap_chartable_accessible_ref_at (ATK_TABLE (component), row, col);
}


static void
gucharmap_chartable_accessible_component_interface_init (AtkComponentIface *iface)
{
  g_return_if_fail (iface != NULL);

  iface->ref_accessible_at_point = gucharmap_chartable_accessible_ref_accessible_at_point;
}


static AtkStateSet*
gucharmap_chartable_accessible_ref_state_set (AtkObject *obj)
{
  AtkStateSet *state_set;
  GtkWidget *widget;

  state_set = ATK_OBJECT_CLASS (gucharmap_chartable_accessible_parent_class)->ref_state_set (obj);
  widget = GTK_ACCESSIBLE (obj)->widget;

  if (widget != NULL)
    atk_state_set_add_state (state_set, ATK_STATE_MANAGES_DESCENDANTS);

  return state_set;
}

/* FIXMEchpe: shouldn't this get the number from the chartable's codepoint list instead?? */
static gint
gucharmap_chartable_accessible_get_n_children (AtkObject *obj)
{
  GtkWidget *widget;

  widget = GTK_ACCESSIBLE (obj)->widget;
  if (widget == NULL)
    /* State is defunct */
    return 0;

  return UNICHAR_MAX + 1;
}


static void
clear_cached_data (GucharmapChartableAccessible *table)
{
  GList *l;
  GList *cell_list;

  cell_list = get_cell_list (table);
  if (cell_list)
    {
      for (l = cell_list; l; l = l->next)
        {
          g_free (l->data);
        }
      g_list_free (cell_list);

      set_cell_list (table, NULL);
    }
}

static void
set_focus_object (AtkObject *obj,
                  AtkObject *focus_obj)
{
  g_object_set_data (G_OBJECT (obj), "chartable-accessible-focus-object", focus_obj);
}

static AtkObject*
get_focus_object (AtkObject *obj)
{
  return g_object_get_data (G_OBJECT (obj), "chartable-accessible-focus-object");
}

static void
gucharmap_chartable_accessible_finalize (GObject *obj)
{
  GucharmapChartableAccessible *table;
  AtkObject *focus_obj;

  table = GUCHARMAP_CHARTABLE_ACCESSIBLE (obj);

  focus_obj = get_focus_object (ATK_OBJECT (obj));
  if (focus_obj)
    g_object_unref (focus_obj);

  clear_cached_data (table);

  G_OBJECT_CLASS (gucharmap_chartable_accessible_parent_class)->finalize (obj);
}

static void
traverse_cells (AtkObject *obj)
{
  GucharmapChartableAccessible *table;
  GtkWidget *widget;
  GList *l;
  GList *cell_list;
  GucharmapChartable *chartable;
  GucharmapChartableCellAccessibleInfo *info;

  g_return_if_fail (IS_GUCHARMAP_CHARTABLE_ACCESSIBLE (obj));

  widget = GTK_ACCESSIBLE (obj)->widget;
  if (!widget)
    /* Widget is being deleted */
    return;
  
  table = GUCHARMAP_CHARTABLE_ACCESSIBLE (obj);
  chartable = GUCHARMAP_CHARTABLE (widget);

  cell_list = get_cell_list (table);
  for (l = cell_list; l; l = l->next)
    {
      info = l->data;
      set_cell_visibility (chartable, GUCHARMAP_CHARTABLE_CELL_ACCESSIBLE (info->cell), TRUE);
    }
  g_signal_emit_by_name (obj, "visible_data_changed"); 
}


static void
adjustment_changed (GtkAdjustment *adj,
                    AtkObject     *obj)
{
  traverse_cells (obj);
}


static void
size_allocated (GtkWidget     *widget,
                GtkAllocation *alloc,
                gpointer      data)
{
  g_return_if_fail (ATK_IS_OBJECT (data));
  traverse_cells (ATK_OBJECT (data));
}


static AtkObject*
find_object (GucharmapChartable   *chartable,
             gunichar  uc,
             AtkObject *obj)
{
  gint row, column;

  row = uc / chartable->cols;
  column = _gucharmap_chartable_cell_column (chartable, uc);

  return atk_table_ref_at (ATK_TABLE (obj), row, column);
}

static void
sync_active_char (GucharmapChartable  *chartable,
                  GParamSpec *pspec,
                  gpointer data)
{
  gunichar uc;
  AtkObject *child;
  AtkObject *focus_obj;

  uc = gucharmap_chartable_get_active_character (chartable);

  child = find_object (chartable, uc, data);
  focus_obj = get_focus_object (data);
  if (focus_obj != child)
    {
      gucharmap_chartable_cell_accessible_remove_state (GUCHARMAP_CHARTABLE_CELL_ACCESSIBLE (focus_obj), ATK_STATE_FOCUSED, FALSE);
      gucharmap_chartable_cell_accessible_add_state (GUCHARMAP_CHARTABLE_CELL_ACCESSIBLE (child), ATK_STATE_FOCUSED, FALSE);
    }  
  g_object_unref (focus_obj);
  set_focus_object (data, child);
  g_signal_emit_by_name (data, "active-descendant-changed", child);
}

static void
gucharmap_chartable_accessible_set_scroll_adjustments (GucharmapChartable *chartable,
                                                       GtkAdjustment *hadjustment,
                                                       GtkAdjustment *vadjustment,
                                                       AtkObject *obj)
{
  GucharmapChartableAccessible *accessible = GUCHARMAP_CHARTABLE_ACCESSIBLE (obj);

  if (accessible->vadjustment != vadjustment)
    {
      GtkAdjustment **adjustment_ptr = &accessible->vadjustment;
      g_object_remove_weak_pointer (G_OBJECT (accessible->vadjustment),
                                    (gpointer *) adjustment_ptr);
      g_signal_handlers_disconnect_by_func (accessible->vadjustment,
                                            G_CALLBACK (adjustment_changed),
                                            obj);
      accessible->vadjustment = vadjustment;
      g_object_add_weak_pointer (G_OBJECT (accessible->vadjustment),
                                 (gpointer *) adjustment_ptr);
      g_signal_connect (chartable->vadjustment, "value-changed",
                        G_CALLBACK (adjustment_changed), obj);
    }
}

static void
gucharmap_chartable_accessible_initialize (AtkObject *obj,
                                           gpointer  data)
{
  GtkWidget *widget;
  AtkObject *focus_obj;
  GucharmapChartableAccessible *accessible;
  GucharmapChartable *chartable;

  ATK_OBJECT_CLASS (gucharmap_chartable_accessible_parent_class)->initialize (obj, data);

  widget = GTK_WIDGET (data);
  accessible = GUCHARMAP_CHARTABLE_ACCESSIBLE (obj);
  chartable = GUCHARMAP_CHARTABLE (widget);

  if (chartable->vadjustment)
    {
      GtkAdjustment **adjustment_ptr = &accessible->vadjustment;

      accessible->vadjustment = chartable->vadjustment;
      g_object_add_weak_pointer (G_OBJECT (accessible->vadjustment),
                                 (gpointer *) adjustment_ptr);
      g_signal_connect (chartable->vadjustment, "value-changed",
                        G_CALLBACK (adjustment_changed), obj);
    }

  g_signal_connect_after (data, "set-scroll-adjustments",
                          G_CALLBACK (gucharmap_chartable_accessible_set_scroll_adjustments),
                          obj);
  g_signal_connect (widget, "size-allocate",
                    G_CALLBACK (size_allocated), obj);
  g_signal_connect (chartable, "notify::active-character",
                    G_CALLBACK (sync_active_char), obj);

  focus_obj = find_object (chartable, chartable->active_cell, obj);
  set_focus_object (obj, focus_obj);  
}

static void
gucharmap_chartable_accessible_destroyed (GtkWidget *widget,
                                          AtkObject *obj)
{
  GucharmapChartableAccessible *accessible = GUCHARMAP_CHARTABLE_ACCESSIBLE (obj);

  if (accessible->vadjustment)
    {
      GtkAdjustment **adjustment_ptr = &accessible->vadjustment;

      g_object_remove_weak_pointer (G_OBJECT (accessible->vadjustment),
                                    (gpointer *) adjustment_ptr);
          
      g_signal_handlers_disconnect_by_func (accessible->vadjustment,
                                            G_CALLBACK (adjustment_changed),
                                            obj);
      accessible->vadjustment = NULL;
    }

  g_signal_handlers_disconnect_by_func (widget,
                                        G_CALLBACK (gucharmap_chartable_accessible_set_scroll_adjustments),
                                        obj);
  g_signal_handlers_disconnect_by_func (widget,
                                        G_CALLBACK (size_allocated),
                                        obj);
  g_signal_handlers_disconnect_by_func (widget,
                                        G_CALLBACK (sync_active_char),
                                        obj);
}

static void
gucharmap_chartable_accessible_connect_widget_destroyed (GtkAccessible *accessible)
{
  if (accessible->widget)
    {
      g_signal_connect_after (accessible->widget,
                              "destroy",
                              G_CALLBACK (gucharmap_chartable_accessible_destroyed),
                              accessible);
    }

  GTK_ACCESSIBLE_CLASS (gucharmap_chartable_accessible_parent_class)->connect_widget_destroyed (accessible);
}

static void
gucharmap_chartable_accessible_class_init (GucharmapChartableAccessibleClass *klass)
{
  GObjectClass *gobject_class = G_OBJECT_CLASS (klass);
  AtkObjectClass *atk_object_class = ATK_OBJECT_CLASS (klass);
  GtkAccessibleClass *accessible_class = GTK_ACCESSIBLE_CLASS (klass);

  gobject_class->finalize = gucharmap_chartable_accessible_finalize;

  accessible_class->connect_widget_destroyed = gucharmap_chartable_accessible_connect_widget_destroyed;

  atk_object_class->get_n_children = gucharmap_chartable_accessible_get_n_children;
  atk_object_class->ref_child = gucharmap_chartable_accessible_ref_child;
  atk_object_class->ref_state_set = gucharmap_chartable_accessible_ref_state_set;
  atk_object_class->initialize = gucharmap_chartable_accessible_initialize;
}

static gint
gucharmap_chartable_accessible_get_n_columns (AtkTable *table)
{
  GtkWidget *widget;
  GucharmapChartable *chartable;

  widget = GTK_ACCESSIBLE (table)->widget;
  if (widget == NULL)
    /* State is defunct */
    return 0;

  chartable = GUCHARMAP_CHARTABLE (widget);

  return chartable->cols;
}


static gint
gucharmap_chartable_accessible_get_column_extent_at (AtkTable       *table,
                                           gint           row,
                                           gint           column)
{
  GtkWidget *widget;

  widget = GTK_ACCESSIBLE (table)->widget;
  if (widget == NULL)
    /* State is defunct */
    return 0;

  return 1;
}


static gint
gucharmap_chartable_accessible_get_n_rows (AtkTable *table)
{
  GtkWidget *widget;
  GucharmapChartable *chartable;
  gint n_rows;

  widget = GTK_ACCESSIBLE (table)->widget;
  if (widget == NULL)
    /* State is defunct */
    return 0;

  chartable = GUCHARMAP_CHARTABLE (widget);

  n_rows = UNICHAR_MAX / chartable->cols + 1;

  return n_rows;
}

 
static gint
gucharmap_chartable_accessible_get_row_extent_at (AtkTable *table,
                                        gint    row,
                                        gint    column)
{
  GtkWidget *widget;

  widget = GTK_ACCESSIBLE (table)->widget;
  if (widget == NULL)
    /* State is defunct */
    return 0;

  return 1;
}

 
static gint
gucharmap_chartable_accessible_get_index_at (AtkTable *table,
                                   gint     row,
                                   gint     column)
{
  GtkWidget *widget;
  GucharmapChartable *chartable;

  widget = GTK_ACCESSIBLE (table)->widget;
  if (widget == NULL)
    /* State is defunct */
    return -1;

  chartable = GUCHARMAP_CHARTABLE (widget);

  return row * chartable->cols + column;
}


static gint
gucharmap_chartable_accessible_get_column_at_index (AtkTable *table,
                                          gint     index)
{
  GtkWidget *widget;
  GucharmapChartable *chartable;

  widget = GTK_ACCESSIBLE (table)->widget;
  if (widget == NULL)
    /* State is defunct */
    return -1;

  chartable = GUCHARMAP_CHARTABLE (widget);

  return index % chartable->cols;
}


static gint
gucharmap_chartable_accessible_get_row_at_index (AtkTable *table,
                                                 gint     index)
{
  GtkWidget *widget;
  GucharmapChartable *chartable;

  widget = GTK_ACCESSIBLE (table)->widget;
  if (widget == NULL)
    /* State is defunct */
    return -1;

  chartable = GUCHARMAP_CHARTABLE (widget);

  return index / chartable->cols;
}


static void
gucharmap_chartable_accessible_table_interface_init (AtkTableIface *iface)
{
  g_return_if_fail (iface != NULL);

  iface->ref_at = gucharmap_chartable_accessible_ref_at;
  iface->get_n_columns = gucharmap_chartable_accessible_get_n_columns;
  iface->get_column_extent_at = gucharmap_chartable_accessible_get_column_extent_at;
  iface->get_n_rows = gucharmap_chartable_accessible_get_n_rows;
  iface->get_row_extent_at = gucharmap_chartable_accessible_get_row_extent_at;
  iface->get_index_at = gucharmap_chartable_accessible_get_index_at;
  iface->get_column_at_index = gucharmap_chartable_accessible_get_column_at_index;
  iface->get_row_at_index = gucharmap_chartable_accessible_get_row_at_index;
}

GType
gucharmap_chartable_accessible_get_type (void)
{
  static volatile gsize type__volatile = 0;

  if (g_once_init_enter (&type__volatile))
    {
      GTypeInfo typeinfo =
      {
        sizeof (GucharmapChartableAccessibleClass),
        (GBaseInitFunc) NULL, /* base init */
        (GBaseFinalizeFunc) NULL, /* base finalize */
        (GClassInitFunc) gucharmap_chartable_accessible_class_init, /* class init */
        (GClassFinalizeFunc) NULL, /* class finalize */
        NULL, /* class data */
        sizeof (GucharmapChartableAccessible), /* instance size */
        0, /* nb preallocs */
        (GInstanceInitFunc) NULL, /* instance init */
        NULL /* value table */
      };
      const GInterfaceInfo atk_table_info =
      {
          (GInterfaceInitFunc) gucharmap_chartable_accessible_table_interface_init,
          (GInterfaceFinalizeFunc) NULL,
          NULL
      };
      const GInterfaceInfo atk_component_info =
      {
          (GInterfaceInitFunc) gucharmap_chartable_accessible_component_interface_init,
          (GInterfaceFinalizeFunc) NULL,
          NULL
      };
      AtkObjectFactory *factory;
      GType derived_type;
      GTypeQuery query;
      GType derived_atk_type;
      GType type;

      /* Figure out the size of the class and instance we are deriving from */
      derived_type = g_type_parent (GUCHARMAP_TYPE_CHARTABLE);
      factory = atk_registry_get_factory (atk_get_default_registry (), 
                                          derived_type);
      derived_atk_type = atk_object_factory_get_accessible_type (factory);
      g_type_query (derived_atk_type, &query);
      typeinfo.class_size = query.class_size;
      typeinfo.instance_size = query.instance_size;

      type = g_type_register_static (derived_atk_type,
                                     "GucharmapChartableAccessible",
                                     &typeinfo, 0);

      g_type_add_interface_static (type, ATK_TYPE_TABLE,
                                   &atk_table_info);
      g_type_add_interface_static (type, ATK_TYPE_COMPONENT,
                                   &atk_component_info);

      g_once_init_leave (&type__volatile, type);
    }

  return type__volatile;
}

/* API */

AtkObject* 
gucharmap_chartable_accessible_new (GucharmapChartable *chartable)
{
  GObject *object;
  AtkObject *accessible;

  g_assert (IS_GUCHARMAP_CHARTABLE (chartable));

  object = g_object_new (gucharmap_chartable_accessible_get_type (), NULL);
  accessible = ATK_OBJECT (object);

  /* atk is fucked up... */
  atk_object_initialize (accessible, GTK_WIDGET (chartable));
  accessible->role = ATK_ROLE_TABLE;
  atk_object_set_name (accessible, _("Character Table"));

  return accessible;
}
