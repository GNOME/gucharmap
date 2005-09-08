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
 * 59 Temple Place, Suite 330, Boston, MA 02111-1307  USA
 */

#include "config.h"
#include <gtk/gtk.h>
#include <stdlib.h>
#include <string.h>
#include "gucharmap-mini-fontsel.h"
#include "gucharmap-intl.h"
#include "gucharmap-marshal.h"

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

static guint gucharmap_mini_font_selection_signals[NUM_SIGNALS] = { 0 };


/* looks up PangoFontFamily by family name, since no such function is in
 * the api */
static GHashTable *pango_font_family_hash = NULL;


static gint
cmp_families (const void *a, const void *b)
{
  const char *a_name = pango_font_family_get_name (*(PangoFontFamily **)a);
  const char *b_name = pango_font_family_get_name (*(PangoFontFamily **)b);
  
  return g_utf8_collate (a_name, b_name);
}


/* also initializes the hash table pango_font_families */
static void
show_available_families (GucharmapMiniFontSelection *fontsel)
{
  PangoFontFamily **families;
  GList *family_names = NULL;
  gint n_families, i;

  /* keys are strings */
  pango_font_family_hash = g_hash_table_new (g_str_hash, g_str_equal);
  
  pango_context_list_families (
          gtk_widget_get_pango_context (GTK_WIDGET (fontsel)),
          &families, &n_families);
  qsort (families, n_families, sizeof (PangoFontFamily *), cmp_families);

  for (i = 0;  i < n_families;  i++)
    {
      /* must strdup for the hash */
      gchar *family_name = g_strdup (pango_font_family_get_name (families[i]));

      /* insert into the hash */
      g_hash_table_insert (pango_font_family_hash, family_name, families[i]);

      /* add to the list */
      family_names = g_list_append (family_names, (gpointer) family_name);
    }

  gtk_combo_set_popdown_strings (GTK_COMBO (fontsel->family), family_names);
    
  g_list_free (family_names);
  g_free (families);
}


static void
set_family (GucharmapMiniFontSelection *fontsel, 
            const gchar *new_family)
{
  pango_font_description_set_family (fontsel->font_desc, new_family);
  g_signal_emit (fontsel, gucharmap_mini_font_selection_signals[CHANGED], 0);
}


static void 
family_changed (GtkWidget *widget, 
                GucharmapMiniFontSelection *fontsel)
{
  const gchar *new_family;

  new_family = gtk_entry_get_text (
                 GTK_ENTRY (GTK_COMBO (fontsel->family)->entry));

  if (new_family[0] == '\0') /* empty string */
    return;

  set_family (fontsel, new_family);
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
                    NULL, NULL, gucharmap_marshal_VOID__VOID,
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
  AtkObject *accessib;

  gtk_widget_ensure_style (GTK_WIDGET (fontsel));
  fontsel->font_desc = pango_font_description_copy (GTK_WIDGET (fontsel)->style->font_desc);
  fontsel->default_size = pango_font_description_get_size (fontsel->font_desc);

  fontsel->size_adj = gtk_adjustment_new (pango_font_description_get_size (fontsel->font_desc) / PANGO_SCALE, 
                                          MIN_FONT_SIZE, MAX_FONT_SIZE, 1, 9, 0);

  accessib = gtk_widget_get_accessible (GTK_WIDGET (fontsel));
  atk_object_set_name (accessib, _("Font"));

  gtk_box_set_spacing (GTK_BOX (fontsel), 6);

  fontsel->family = gtk_combo_new ();
  gtk_widget_show (fontsel->family);
  accessib = gtk_widget_get_accessible (fontsel->family);
  atk_object_set_name (accessib, _("Font Family"));
  gtk_editable_set_editable (GTK_EDITABLE (GTK_COMBO (fontsel->family)->entry),
                             FALSE);

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

  show_available_families (fontsel);

  g_signal_connect (G_OBJECT (GTK_COMBO (fontsel->family)->entry), "changed",
                    G_CALLBACK (family_changed), fontsel);

  gtk_box_pack_start (GTK_BOX (fontsel), fontsel->family, FALSE, FALSE, 0);
  gtk_box_pack_start (GTK_BOX (fontsel), fontsel->bold, FALSE, FALSE, 0);
  gtk_box_pack_start (GTK_BOX (fontsel), fontsel->italic, FALSE, FALSE, 0);
  gtk_box_pack_start (GTK_BOX (fontsel), fontsel->size, FALSE, FALSE, 0);

  gtk_entry_set_text (GTK_ENTRY (GTK_COMBO (fontsel->family)->entry),
                      pango_font_description_get_family (fontsel->font_desc));

  gtk_container_set_border_width (GTK_CONTAINER (fontsel), 6);

  gtk_widget_show_all (GTK_WIDGET (fontsel));
}


GtkWidget *
gucharmap_mini_font_selection_new (void)
{
  return GTK_WIDGET (g_object_new (gucharmap_mini_font_selection_get_type (), 
                                   NULL));
}


GType
gucharmap_mini_font_selection_get_type (void)
{
  static GType gucharmap_mini_font_selection_type = 0;

  if (gucharmap_mini_font_selection_type == 0)
    {
      static const GTypeInfo gucharmap_mini_font_selection_info =
      {
        sizeof (GucharmapMiniFontSelectionClass),
        NULL,           /* base_init */
        NULL,           /* base_finalize */
        (GClassInitFunc) gucharmap_mini_font_selection_class_init,
        NULL,           /* class_finalize */
        NULL,           /* class_data */
        sizeof (GucharmapMiniFontSelection),
        0,              /* n_preallocs */
        (GInstanceInitFunc) gucharmap_mini_font_selection_init
      };

      gucharmap_mini_font_selection_type = g_type_register_static (
              GTK_TYPE_HBOX, "GucharmapMiniFontSelection", 
              &gucharmap_mini_font_selection_info, 0);
    }

  return gucharmap_mini_font_selection_type;
}


/* XXX: should do error checking */
gboolean 
gucharmap_mini_font_selection_set_font_name (GucharmapMiniFontSelection *fontsel,
                                             const gchar *fontname)
{
  pango_font_description_free (fontsel->font_desc);

  fontsel->font_desc = pango_font_description_from_string (fontname);

  gtk_entry_set_text (GTK_ENTRY (GTK_COMBO (fontsel->family)->entry), 
                      pango_font_description_get_family (fontsel->font_desc));
    
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
