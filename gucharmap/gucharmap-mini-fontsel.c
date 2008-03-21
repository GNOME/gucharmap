/* $Id$ */
/*
 * Copyright (c) 2004 Noah Levitt
 *
 * This program is free software; you can redistribute it and/or modify it
 * under the terms of the GNU General Public License as published by the
 * Free Software Foundation; either version 2 of the License, or (at your
 * option) any later version.
 *
 * This program is distributed in the hope that it will be useful, but
 * WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License along
 * with this program; if not, write to the Free Software Foundation, Inc.,
 * 59 Temple Place, Suite 330, Boston, MA 02110-1301  USA
 */

#include <config.h>

#include <glib/gi18n-lib.h>
#include <gtk/gtk.h>
#include <stdlib.h>
#include <string.h>
#include "gucharmap-mini-fontsel.h"
#include "gucharmap-marshal.h"
#include "gucharmap-private.h"

enum
{
  MIN_FONT_SIZE = 5,
  MAX_FONT_SIZE = 400,
};

enum
{
  CHANGED,
  NUM_SIGNALS
};

enum
{
  COL_FAMILIY
};

static guint gucharmap_mini_font_selection_signals[NUM_SIGNALS];


static void
fill_font_families_combo (GucharmapMiniFontSelection *fontsel)
{
  PangoFontFamily **families;
  int n_families, i;

  pango_context_list_families (
          gtk_widget_get_pango_context (GTK_WIDGET (fontsel)),
          &families, &n_families);

  for (i = 0;  i < n_families;  i++)
    {
      PangoFontFamily *family = families[i];
      GtkTreeIter iter;

      gtk_list_store_insert_with_values (fontsel->family_store,
                                         &iter,
                                         -1,
                                         COL_FAMILIY, pango_font_family_get_name (family),
                                         -1);
    }

  g_free (families);

  /* Now turn on sorting in the combo box */
  gtk_tree_sortable_set_sort_column_id (GTK_TREE_SORTABLE (fontsel->family_store),
                                        COL_FAMILIY,
                                        GTK_SORT_ASCENDING);
}


static void
update_font_familiy_combo (GucharmapMiniFontSelection *fontsel)
{
  GtkTreeModel *model = GTK_TREE_MODEL (fontsel->family_store);
  GtkTreeIter iter;
  const char *font_family;
  gboolean found = FALSE;

  font_family = pango_font_description_get_family (fontsel->font_desc);
  if (!font_family || !font_family[0])
    return;

  if (!gtk_tree_model_get_iter_first (model, &iter))
    return;

  do {
    char *family;

    gtk_tree_model_get (model, &iter, COL_FAMILIY, &family, -1);
    found = family && strcmp (family, font_family) == 0;
    g_free (family);
  } while (!found && gtk_tree_model_iter_next (model, &iter));

  if (found) {
    gtk_combo_box_set_active_iter (GTK_COMBO_BOX (fontsel->family), &iter);
  }
}


static void
set_family (GucharmapMiniFontSelection *fontsel, 
            const gchar *new_family)
{
  pango_font_description_set_family (fontsel->font_desc, new_family);
  g_signal_emit (fontsel, gucharmap_mini_font_selection_signals[CHANGED], 0);
}


static void 
family_changed (GtkComboBox *combo,
                GucharmapMiniFontSelection *fontsel)
{
  GtkTreeIter iter;
  char *family;

  if (!gtk_combo_box_get_active_iter (combo, &iter))
    return;

  gtk_tree_model_get (GTK_TREE_MODEL (fontsel->family_store),
                      &iter,
                      COL_FAMILIY, &family,
                      -1);
  if (!family)
    return;

  set_family (fontsel, family);
  g_free (family);
}


/* size is in points */
static void
set_size (GucharmapMiniFontSelection *fontsel, 
          gint size)
{
  pango_font_description_set_size (
	  fontsel->font_desc, 
	  PANGO_SCALE * CLAMP (size, MIN_FONT_SIZE, MAX_FONT_SIZE));
  g_signal_emit (fontsel, gucharmap_mini_font_selection_signals[CHANGED], 0);
}


static void 
size_changed (GtkAdjustment *adjustment, 
              GucharmapMiniFontSelection *fontsel)
{
  if ((gint) gtk_adjustment_get_value (adjustment) 
      != pango_font_description_get_size (fontsel->font_desc))
    set_size (fontsel, (gint) gtk_adjustment_get_value (adjustment));
}


static void
mini_font_selection_finalize (GObject *object)
{
  GucharmapMiniFontSelection *fontsel = GUCHARMAP_MINI_FONT_SELECTION (object);
  pango_font_description_free (fontsel->font_desc);
}


static void
gucharmap_mini_font_selection_class_init (GucharmapMiniFontSelectionClass *clazz)
{
  clazz->changed = NULL;

  gucharmap_mini_font_selection_signals[CHANGED] =
      g_signal_new ("changed", gucharmap_mini_font_selection_get_type (), 
		    G_SIGNAL_RUN_FIRST,
                    G_STRUCT_OFFSET (GucharmapMiniFontSelectionClass, changed),
                    NULL, NULL, g_cclosure_marshal_VOID__VOID,
                    G_TYPE_NONE, 0);

  G_OBJECT_CLASS (clazz)->finalize = mini_font_selection_finalize;
}


static void
bold_toggled (GtkToggleButton *toggle,
              GucharmapMiniFontSelection *fontsel)
{
  if (gtk_toggle_button_get_active (toggle))
    pango_font_description_set_weight (fontsel->font_desc, PANGO_WEIGHT_BOLD);
  else
    pango_font_description_set_weight (fontsel->font_desc, PANGO_WEIGHT_NORMAL);

  g_signal_emit (fontsel, gucharmap_mini_font_selection_signals[CHANGED], 0);
}


static void
italic_toggled (GtkToggleButton *toggle,
                GucharmapMiniFontSelection *fontsel)
{
  if (gtk_toggle_button_get_active (toggle))
    pango_font_description_set_style (fontsel->font_desc, PANGO_STYLE_ITALIC);
  else
    pango_font_description_set_style (fontsel->font_desc, PANGO_STYLE_NORMAL);

  g_signal_emit (fontsel, gucharmap_mini_font_selection_signals[CHANGED], 0);
}


static void
gucharmap_mini_font_selection_init (GucharmapMiniFontSelection *fontsel)
{
  GtkCellRenderer *renderer;
  AtkObject *accessib;

  gtk_widget_ensure_style (GTK_WIDGET (fontsel));
  fontsel->font_desc = pango_font_description_copy (GTK_WIDGET (fontsel)->style->font_desc);
  fontsel->default_size = pango_font_description_get_size (fontsel->font_desc);

  fontsel->size_adj = gtk_adjustment_new (pango_font_description_get_size (fontsel->font_desc) / PANGO_SCALE, 
                                          MIN_FONT_SIZE, MAX_FONT_SIZE, 1, 9, 0);

  accessib = gtk_widget_get_accessible (GTK_WIDGET (fontsel));
  atk_object_set_name (accessib, _("Font"));

  gtk_box_set_spacing (GTK_BOX (fontsel), 6);

  fontsel->family_store = gtk_list_store_new (1, G_TYPE_STRING);
  fontsel->family = gtk_combo_box_new_with_model (GTK_TREE_MODEL (fontsel->family_store));
  g_object_unref (fontsel->family_store);
  renderer = gtk_cell_renderer_text_new ();
  gtk_cell_layout_pack_start (GTK_CELL_LAYOUT (fontsel->family), renderer, TRUE);
  gtk_cell_layout_set_attributes (GTK_CELL_LAYOUT (fontsel->family), renderer,
                                  "text", COL_FAMILIY,
                                  NULL);
  gtk_widget_show (fontsel->family);
  accessib = gtk_widget_get_accessible (fontsel->family);
  atk_object_set_name (accessib, _("Font Family"));

  fontsel->bold = gtk_toggle_button_new_with_mnemonic (GTK_STOCK_BOLD);
  gtk_button_set_use_stock (GTK_BUTTON (fontsel->bold), TRUE);
  gtk_widget_show (fontsel->bold);
  g_signal_connect (fontsel->bold, "toggled",
                    G_CALLBACK (bold_toggled), fontsel);

  fontsel->italic = gtk_toggle_button_new_with_mnemonic (GTK_STOCK_ITALIC);
  gtk_button_set_use_stock (GTK_BUTTON (fontsel->italic), TRUE);
  gtk_widget_show (fontsel->italic);
  g_signal_connect (fontsel->italic, "toggled",
                    G_CALLBACK (italic_toggled), fontsel);

  fontsel->size = gtk_spin_button_new (GTK_ADJUSTMENT (fontsel->size_adj),
                                       0, 0);
  gtk_widget_show (fontsel->size);
  accessib = gtk_widget_get_accessible (fontsel->size);
  atk_object_set_name (accessib, _("Font Size"));
  g_signal_connect (fontsel->size_adj, "value-changed",
                    G_CALLBACK (size_changed), fontsel);

  fill_font_families_combo (fontsel);
  update_font_familiy_combo (fontsel);
    
  g_signal_connect (fontsel->family, "changed",
                    G_CALLBACK (family_changed), fontsel);

  gtk_box_pack_start (GTK_BOX (fontsel), fontsel->family, FALSE, FALSE, 0);
  gtk_box_pack_start (GTK_BOX (fontsel), fontsel->bold, FALSE, FALSE, 0);
  gtk_box_pack_start (GTK_BOX (fontsel), fontsel->italic, FALSE, FALSE, 0);
  gtk_box_pack_start (GTK_BOX (fontsel), fontsel->size, FALSE, FALSE, 0);

  gtk_container_set_border_width (GTK_CONTAINER (fontsel), 6);

  gtk_widget_show_all (GTK_WIDGET (fontsel));
}


GtkWidget *
gucharmap_mini_font_selection_new (void)
{
  return GTK_WIDGET (g_object_new (gucharmap_mini_font_selection_get_type (), 
                                   NULL));
}

G_DEFINE_TYPE (GucharmapMiniFontSelection, gucharmap_mini_font_selection, GTK_TYPE_HBOX)

/* XXX: should do error checking */
gboolean 
gucharmap_mini_font_selection_set_font_name (GucharmapMiniFontSelection *fontsel,
                                             const gchar *fontname)
{
  pango_font_description_free (fontsel->font_desc);

  fontsel->font_desc = pango_font_description_from_string (fontname);

  update_font_familiy_combo (fontsel);
    
  /* treat oblique and italic both as italic */
  gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON (fontsel->italic), pango_font_description_get_style (fontsel->font_desc) == PANGO_STYLE_ITALIC || pango_font_description_get_style (fontsel->font_desc) == PANGO_STYLE_OBLIQUE);

  gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON (fontsel->bold), pango_font_description_get_weight (fontsel->font_desc) > PANGO_WEIGHT_NORMAL);

  gtk_adjustment_set_value (
          GTK_ADJUSTMENT (fontsel->size_adj), 
          pango_font_description_get_size (fontsel->font_desc) / PANGO_SCALE);

  g_signal_emit (fontsel, gucharmap_mini_font_selection_signals[CHANGED], 0);

  return TRUE;
}



gchar * 
gucharmap_mini_font_selection_get_font_name (GucharmapMiniFontSelection *fontsel)
{
  return pango_font_description_to_string (fontsel->font_desc);
}


/* returns font size in points */
gint
gucharmap_mini_font_selection_get_font_size (GucharmapMiniFontSelection *fontsel)
{
  return pango_font_description_get_size (fontsel->font_desc) / PANGO_SCALE;
}


/* size in points */
void
gucharmap_mini_font_selection_set_font_size (GucharmapMiniFontSelection *fontsel, 
                                             gint size)
{
  gtk_adjustment_set_value (GTK_ADJUSTMENT (fontsel->size_adj), size);
  set_size (fontsel, size);
}

/* size in points */
void
gucharmap_mini_font_selection_set_default_font_size (GucharmapMiniFontSelection *fontsel, 
                                                     gint                        size)
{
  fontsel->default_size = size;
}

/* size in points */
void
gucharmap_mini_font_selection_reset_font_size (GucharmapMiniFontSelection *fontsel)
{
  gucharmap_mini_font_selection_set_font_size (fontsel, fontsel->default_size);
}
