/* $Id$ */
/*
 * Copyright (c) 2003 Noah Levitt
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

#ifndef GUCHARMAP_MINI_FONTSEL_H
#define GUCHARMAP_MINI_FONTSEL_H

#include <gtk/gtk.h>


G_BEGIN_DECLS


#define GUCHARMAP_MINI_FONT_SELECTION(obj) \
        (G_TYPE_CHECK_INSTANCE_CAST ((obj), gucharmap_mini_font_selection_get_type (), \
                                     GucharmapMiniFontSelection))

#define GUCHARMAP_MINI_FONT_SELECTION_CLASS(clazz) \
        (G_TYPE_CHECK_CLASS_CAST ((clazz), gucharmap_mini_font_selection_get_type (), \
                                  GucharmapMiniFontSelectionClass))

#define IS_GUCHARMAP_MINI_FONT_SELECTION(obj) \
        G_TYPE_CHECK_INSTANCE_TYPE ((obj), gucharmap_mini_font_selection_get_type ())


typedef struct _GucharmapMiniFontSelection GucharmapMiniFontSelection;
typedef struct _GucharmapMiniFontSelectionClass GucharmapMiniFontSelectionClass;


struct _GucharmapMiniFontSelection
{
  GtkHBox parent;

  GtkWidget *family; /* combo box */
  GtkWidget *bold;   /* toggle button*/
  GtkWidget *italic; /* toggle button*/

  GtkObject *size_adj; 
  GtkWidget *size;   /* spin button */
  
  PangoFontDescription *font_desc;
};


struct _GucharmapMiniFontSelectionClass
{
  GtkHBoxClass parent_class;

  void (* changed) (GucharmapMiniFontSelection *fontsel);
};


GType gucharmap_mini_font_selection_get_type ();
GtkWidget * gucharmap_mini_font_selection_new ();
gboolean gucharmap_mini_font_selection_set_font_name (GucharmapMiniFontSelection *fontsel,
                                                      const gchar *fontname);
gchar * gucharmap_mini_font_selection_get_font_name (GucharmapMiniFontSelection *fontsel);
gint gucharmap_mini_font_selection_get_font_size (GucharmapMiniFontSelection *fontsel);
void gucharmap_mini_font_selection_set_font_size (GucharmapMiniFontSelection *fontsel,
                                                  gint size);


G_END_DECLS


#endif /* #ifndef GUCHARMAP_MINI_FONTSEL_H */

