/* $Id$ */
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

#ifdef HAVE_CONFIG_H
# include <config.h>
#endif

#include <stdlib.h>
#include <gtk/gtk.h>

#include <gucharmap/gucharmap.h>
#include "charcell_accessible.h"


extern gint gucharmap_table_x_offset (GucharmapTable *chartable, 
                                      gint col);
extern gint gucharmap_table_y_offset (GucharmapTable *chartable, 
                                      gint row);
extern gint gucharmap_table_unichar_column (GucharmapTable *chartable, 
                                            gunichar uc);
extern gint gucharmap_table_row_height (GucharmapTable *chartable, 
                                        gint row);
extern gint gucharmap_table_column_width (GucharmapTable *chartable, 
                                          gint col);
extern void gucharmap_table_redraw (GucharmapTable *chartable, 
                                    gboolean move_zoom);


static gpointer parent_class = NULL;


static void
charcell_accessible_object_finalize (GObject *obj)
{
  CharcellAccessible *cell = CHARCELL_ACCESSIBLE (obj);

  if (cell->activate_description)
      g_free (cell->activate_description);

  if (cell->action_idle_handler)
    {
      g_source_remove (cell->action_idle_handler);
      cell->action_idle_handler = 0;
    }
     
  if (cell->state_set)
    g_object_unref (cell->state_set);
  G_OBJECT_CLASS (parent_class)->finalize (obj);
}


static gint
charcell_accessible_get_index_in_parent (AtkObject *obj)
{
  CharcellAccessible *cell;

  g_return_val_if_fail (IS_CHARCELL_ACCESSIBLE (obj), 0);
  cell = CHARCELL_ACCESSIBLE (obj);

  return cell->index;
}


static AtkStateSet *
charcell_accessible_ref_state_set (AtkObject *obj)
{
  CharcellAccessible *cell = CHARCELL_ACCESSIBLE (obj);
  g_return_val_if_fail (cell->state_set, NULL);

  g_object_ref(cell->state_set);
  return cell->state_set;
}


static void	 
charcell_accessible_class_init (CharcellAccessibleClass *klass)
{
  AtkObjectClass *class = ATK_OBJECT_CLASS (klass);
  GObjectClass *g_object_class = G_OBJECT_CLASS (klass);

  parent_class = g_type_class_peek_parent (klass);
  g_object_class->finalize = charcell_accessible_object_finalize;

  class->get_index_in_parent = charcell_accessible_get_index_in_parent;
  class->ref_state_set = charcell_accessible_ref_state_set;
}


static void
charcell_accessible_object_init (CharcellAccessible *cell)
{
  cell->state_set = atk_state_set_new ();
  cell->widget = NULL;
  cell->index = 0;
  cell->action_idle_handler = 0;
  atk_state_set_add_state (cell->state_set, ATK_STATE_TRANSIENT);
  atk_state_set_add_state (cell->state_set, ATK_STATE_ENABLED);
}


static void 
charcell_accessible_get_extents (AtkComponent *component,
                                 gint         *x,
                                 gint         *y,
                                 gint         *width,
                                 gint         *height,
                                 AtkCoordType coord_type)
{
  CharcellAccessible *cell;
  AtkObject *cell_parent;
  GucharmapTable *chartable;
  gint real_x, real_y, real_width, real_height;
  gint row, column;

  g_return_if_fail (IS_CHARCELL_ACCESSIBLE (component));

  cell = CHARCELL_ACCESSIBLE (component);

  cell_parent = atk_object_get_parent (ATK_OBJECT (cell));

  /*
   * Is the cell visible on the screen
   */
  chartable = GUCHARMAP_TABLE (cell->widget);
  if (cell->index >= chartable->page_first_char && cell->index < chartable->page_first_char + chartable->rows * chartable->cols)
    {
      atk_component_get_extents (ATK_COMPONENT (cell_parent), 
                                 &real_x, &real_y, &real_width, &real_height, 
                                 coord_type);
      row = (cell->index - chartable->page_first_char)/ chartable->cols;
      column = gucharmap_table_unichar_column (chartable, cell->index);
      *x = real_x + gucharmap_table_x_offset (chartable, column);
      *y = real_y + gucharmap_table_y_offset (chartable, row);
      *width = gucharmap_table_column_width (chartable, column);
      *height = gucharmap_table_row_height (chartable, row);
    }
  else
    {
      *x = G_MININT;
      *y = G_MININT;
    }
}


static gboolean 
charcell_accessible_grab_focus (AtkComponent *component)
{
  CharcellAccessible *cell;
  GucharmapTable *chartable;

  g_return_val_if_fail (IS_CHARCELL_ACCESSIBLE (component), FALSE);

  cell = CHARCELL_ACCESSIBLE (component);

  chartable = GUCHARMAP_TABLE (cell->widget);
  gucharmap_table_set_active_character (chartable, cell->index);
  gucharmap_table_redraw (chartable, TRUE);
  return TRUE;
}


static void
charcell_accessible_component_interface_init (AtkComponentIface *iface)
{
  g_return_if_fail (iface != NULL);

  iface->get_extents = charcell_accessible_get_extents;
  iface->grab_focus = charcell_accessible_grab_focus;
}


static gint
charcell_accessible_action_get_n_actions (AtkAction *action)
{
  return 1;
}


static gboolean
idle_do_action (gpointer data)
{
  CharcellAccessible *cell;
  GucharmapTable *chartable;

  cell = CHARCELL_ACCESSIBLE (data);

  chartable = GUCHARMAP_TABLE (cell->widget);
  gucharmap_table_set_active_character (chartable, cell->index);
  gucharmap_table_redraw (chartable, TRUE);
  g_signal_emit_by_name (chartable, "activate", cell->index);
  return FALSE; 
}


static gboolean
charcell_accessible_action_do_action (AtkAction *action,
                           gint      index)
{
  CharcellAccessible *cell;

  cell = CHARCELL_ACCESSIBLE (action);
  if (index == 0)
    {
      if (cell->action_idle_handler)
        return FALSE;
      cell->action_idle_handler = g_idle_add (idle_do_action, cell);
      return TRUE;
    }
  else
    return FALSE;
}


static G_CONST_RETURN gchar*
charcell_accessible_action_get_name (AtkAction *action,
                          gint      index)
{
  if (index == 0)
    return "activate";
  else
    return NULL;
}


static G_CONST_RETURN gchar *
charcell_accessible_action_get_description (AtkAction *action,
                                 gint      index)
{
  CharcellAccessible *cell;

  cell = CHARCELL_ACCESSIBLE (action);
  if (index == 0)
    return cell->activate_description;
  else
    return NULL;
}


static gboolean
charcell_accessible_action_set_description (AtkAction   *action,
			         gint        index,
                                 const gchar *desc)
{
  CharcellAccessible *cell;

  cell = CHARCELL_ACCESSIBLE (action);
  if (index == 0)
    {
      g_free (cell->activate_description);
      cell->activate_description = g_strdup (desc);
      return TRUE;
    }
  else
    return FALSE;

}


static void
charcell_accessible_action_interface_init (AtkActionIface *iface)
{
  g_return_if_fail (iface != NULL);

  iface->get_n_actions = charcell_accessible_action_get_n_actions;
  iface->do_action = charcell_accessible_action_do_action;
  iface->get_name = charcell_accessible_action_get_name;
  iface->get_description = charcell_accessible_action_get_description;
  iface->set_description = charcell_accessible_action_set_description;
}


GType
charcell_accessible_get_type (void)
{
  static GType type = 0;

  if (!type)
    {
      static const GTypeInfo tinfo =
      {
        sizeof (CharcellAccessibleClass),
        (GBaseInitFunc) NULL, /* base init */
        (GBaseFinalizeFunc) NULL, /* base finalize */
        (GClassInitFunc) charcell_accessible_class_init, /* class init */
        (GClassFinalizeFunc) NULL, /* class finalize */
        NULL, /* class data */
        sizeof (CharcellAccessible), /* instance size */
        0, /* nb preallocs */
        (GInstanceInitFunc) charcell_accessible_object_init, /* instance init */
        NULL /* value table */
      };

      static const GInterfaceInfo atk_component_info =
      {
        (GInterfaceInitFunc) charcell_accessible_component_interface_init,
        (GInterfaceFinalizeFunc) NULL,
        NULL
      };

     static const GInterfaceInfo atk_action_info =
     {
       (GInterfaceInitFunc) charcell_accessible_action_interface_init,
       (GInterfaceFinalizeFunc) NULL,
       NULL
     };

      type = g_type_register_static (ATK_TYPE_OBJECT,
                                     "CharcellAccessible", &tinfo, 0);
      g_type_add_interface_static (type, ATK_TYPE_COMPONENT,
                                   &atk_component_info);
      g_type_add_interface_static (type, ATK_TYPE_ACTION,
                                   &atk_action_info);
    }
  return type;
}


AtkObject *
charcell_accessible_new (void)
{
  GObject *object;
  AtkObject *atk_object;

  object = g_object_new (charcell_accessible_get_type (), NULL);

  g_return_val_if_fail (object != NULL, NULL);

  atk_object = ATK_OBJECT (object);
  atk_object->role = ATK_ROLE_TABLE_CELL;

  return atk_object;
}


static void
charcell_accessible_destroyed (GtkWidget       *widget,
                    CharcellAccessible        *cell)
{
  /*
   * This is the signal handler for the "destroy" signal for the 
   * GtkWidget. We set the  pointer location to NULL;
   */
  cell->widget = NULL;
}


void 
charcell_accessible_init (CharcellAccessible   *cell,
                          GtkWidget *widget,
                          AtkObject *parent,
                          gint      index)
{
  g_return_if_fail (IS_CHARCELL_ACCESSIBLE (cell));
  g_return_if_fail (GTK_IS_WIDGET (widget));

  cell->widget = widget;
  atk_object_set_parent (ATK_OBJECT (cell), parent);
  cell->index = index;
  cell->activate_description = g_strdup ("Activate the cell");

  g_signal_connect_object (G_OBJECT (widget),
                           "destroy",
                           G_CALLBACK (charcell_accessible_destroyed ),
                           cell, 0);
}


gboolean
charcell_accessible_add_state (CharcellAccessible      *cell, 
                               AtkStateType state_type,
                               gboolean     emit_signal)
{
  if (!atk_state_set_contains_state (cell->state_set, state_type))
    {
      gboolean rc;
    
      rc = atk_state_set_add_state (cell->state_set, state_type);
      /*
       * The signal should only be generated if the value changed,
       * not when the cell is set up.  So states that are set
       * initially should pass FALSE as the emit_signal argument.
       */

      if (emit_signal)
        {
          atk_object_notify_state_change (ATK_OBJECT (cell), state_type, TRUE);
          /* If state_type is ATK_STATE_VISIBLE, additional notification */
          if (state_type == ATK_STATE_VISIBLE)
            g_signal_emit_by_name (cell, "visible_data_changed");
        }

      return rc;
    }
  else
    return FALSE;
}


gboolean
charcell_accessible_remove_state (CharcellAccessible *cell, 
                                  AtkStateType state_type,
                                  gboolean emit_signal)
{
  if (atk_state_set_contains_state (cell->state_set, state_type))
    {
      gboolean rc;

      rc = atk_state_set_remove_state (cell->state_set, state_type);
      /*
       * The signal should only be generated if the value changed,
       * not when the cell is set up.  So states that are set
       * initially should pass FALSE as the emit_signal argument.
       */

      if (emit_signal)
        {
          atk_object_notify_state_change (ATK_OBJECT (cell), state_type, FALSE);
          /* If state_type is ATK_STATE_VISIBLE, additional notification */
          if (state_type == ATK_STATE_VISIBLE)
            g_signal_emit_by_name (cell, "visible_data_changed");
        }

      return rc;
    }
  else
    return FALSE;
}

 
