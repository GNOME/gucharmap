/* $Id$ */
/*
 * Copyright (c) 2002  Noah Levitt <nlevitt@users.sourceforge.net>
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

#include <gtk/gtk.h>
#include <stdlib.h>
#include <string.h>
#include "mini_fontsel.h"


static gint
cmp_families (const void *a, const void *b)
{
  const char *a_name = pango_font_family_get_name (*(PangoFontFamily **)a);
  const char *b_name = pango_font_family_get_name (*(PangoFontFamily **)b);
  
  return g_utf8_collate (a_name, b_name);
}


static void
show_available_fonts (MiniFontSelection *fontsel)
{
  PangoFontFamily **families;
  GList *family_names = NULL;
  gint n_families, i;
  
  g_printerr ("show_available_fonts\n");

  pango_context_list_families (
          gtk_widget_get_pango_context (GTK_WIDGET (fontsel)),
          &families, &n_families);
  qsort (families, n_families, sizeof (PangoFontFamily *), cmp_families);

  for (i = 0;  i < n_families;  i++)
    family_names = g_list_append (
            family_names, 
            (gpointer) pango_font_family_get_name (families[i]));

  gtk_combo_set_popdown_strings (GTK_COMBO (fontsel->family), family_names);
    
  g_list_free (family_names);
}


static int
compare_font_descriptions (const PangoFontDescription *a, const PangoFontDescription *b)
{
  int val = strcmp (pango_font_description_get_family (a), 
                    pango_font_description_get_family (b));
  if (val != 0)
    return val;

  if (pango_font_description_get_weight (a) 
      != pango_font_description_get_weight (b))
    return pango_font_description_get_weight (a) 
           - pango_font_description_get_weight (b);

  if (pango_font_description_get_style (a) 
      != pango_font_description_get_style (b))
    return pango_font_description_get_style (a) 
           - pango_font_description_get_style (b);
  
  if (pango_font_description_get_stretch (a) 
      != pango_font_description_get_stretch (b))
    return pango_font_description_get_stretch (a) 
           - pango_font_description_get_stretch (b);

  if (pango_font_description_get_variant (a) 
      != pango_font_description_get_variant (b))
    return pango_font_description_get_variant (a) 
            - pango_font_description_get_variant (b);

  return 0;
}


static int
faces_sort_func (const void *a, const void *b)
{
  PangoFontDescription *desc_a, *desc_b;
  int rv;

  desc_a = pango_font_face_describe (*(PangoFontFace **)a);
  desc_b = pango_font_face_describe (*(PangoFontFace **)b);
  
  rv = compare_font_descriptions (desc_a, desc_b);

  pango_font_description_free (desc_a);
  pango_font_description_free (desc_b);

  return rv;
}


/* This fills the font style list with all the possible style combinations
   for the current font family. */
static void
show_available_styles (MiniFontSelection *fontsel)
{
  PangoFontFace **faces;
  GList *face_names;
  gint n_faces, i;

  g_printerr ("gtk_font_selection_show_available_styles\n");

  pango_font_family_list_faces (fontsel->family_value, &faces, &n_faces);
  qsort (faces, n_faces, sizeof (PangoFontFace *), faces_sort_func);

  for (i=0; i < n_faces; i++)
    face_names = g_list_append (
            face_names, (gpointer) pango_font_face_get_face_name (faces[i]));

  gtk_combo_set_popdown_strings (GTK_COMBO (fontsel->style), face_names);

  g_list_free (face_names);
  g_free (faces);
}


static void family_changed (GtkWidget *widget, MiniFontSelection *fontsel)
{
  g_printerr ("family_changed: %s\n", 
              gtk_entry_get_text (
                  GTK_ENTRY (GTK_COMBO (fontsel->family)->entry)));
}


static void style_changed (GtkWidget *widget, MiniFontSelection *fontsel)
{
  g_printerr ("style_changed: %s\n", 
              gtk_entry_get_text (
                  GTK_ENTRY (GTK_COMBO (fontsel->style)->entry)));
}


static void size_changed (GtkAdjustment *adjustment, 
                          MiniFontSelection *fontsel)
{
  g_printerr ("size_changed: %d\n", 
              (gint) gtk_adjustment_get_value (adjustment));
}


void
mini_font_selection_class_init (MiniFontSelectionClass *clazz)
{
  g_printerr ("mini_font_selection_class_init\n");
}


void
mini_font_selection_init (MiniFontSelection *fontsel)
{
  g_printerr ("mini_font_selection_init\n");

  gtk_box_set_spacing (GTK_BOX (fontsel), 10);

  fontsel->family = gtk_combo_new ();
  fontsel->style = gtk_combo_new ();

  fontsel->size_adj = gtk_adjustment_new (14, 5, 500, 1, 2, 0);
  fontsel->size = gtk_spin_button_new (GTK_ADJUSTMENT (fontsel->size_adj),
                                       0, 0);

  gtk_editable_set_editable (GTK_EDITABLE (GTK_COMBO (fontsel->family)->entry),
                                           FALSE);
  gtk_editable_set_editable (GTK_EDITABLE (GTK_COMBO (fontsel->style)->entry),
                                           FALSE);

  g_signal_connect (G_OBJECT (GTK_COMBO (fontsel->family)->entry), "changed",
                    G_CALLBACK (family_changed), fontsel);
  g_signal_connect (G_OBJECT (GTK_COMBO (fontsel->style)->entry), "changed",
                    G_CALLBACK (style_changed), fontsel);
  g_signal_connect (fontsel->size_adj, "value_changed",
                    G_CALLBACK (size_changed), fontsel);

  gtk_box_pack_start (GTK_BOX (fontsel), fontsel->family, FALSE, FALSE, 0);
  gtk_box_pack_start (GTK_BOX (fontsel), fontsel->style, FALSE, FALSE, 0);
  gtk_box_pack_start (GTK_BOX (fontsel), fontsel->size, FALSE, FALSE, 0);

  show_available_fonts (fontsel);
}


GtkWidget *
mini_font_selection_new ()
{
  g_printerr ("mini_font_selection_new\n");
  return GTK_WIDGET (g_object_new (mini_font_selection_get_type (), NULL));
}


GtkType
mini_font_selection_get_type ()
{
  static GtkType mini_font_selection_type = 0;

  g_printerr ("mini_font_selection_get_type\n");

  if (mini_font_selection_type == 0)
    {
      static const GTypeInfo mini_font_selection_info =
      {
        sizeof (MiniFontSelectionClass),
        NULL,           /* base_init */
        NULL,           /* base_finalize */
        (GClassInitFunc) mini_font_selection_class_init,
        NULL,           /* class_finalize */
        NULL,           /* class_data */
        sizeof (MiniFontSelection),
        0,              /* n_preallocs */
        (GInstanceInitFunc) mini_font_selection_init
      };

      mini_font_selection_type = g_type_register_static (
              GTK_TYPE_HBOX, "MiniFontSelection", 
              &mini_font_selection_info, 0);
    }

  return mini_font_selection_type;
}


gboolean mini_font_selection_set_font_name (MiniFontSelection *fontsel,
                                            const gchar *fontname)
{
  g_printerr ("mini_font_selection_set_font_name (\"%s\")\n", fontname);
  return FALSE;
}


gchar * 
mini_font_selection_get_font_name (MiniFontSelection *fontsel)
{
  g_printerr ("mini_font_selection_get_font_name\n");
  return "Sans 20";
}
