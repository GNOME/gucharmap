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

#ifndef GUCHARMAP_TABLE_H
#define GUCHARMAP_TABLE_H

#include <gtk/gtk.h>

G_BEGIN_DECLS


#define GUCHARMAP_TABLE(obj) (G_TYPE_CHECK_INSTANCE_CAST ((obj), \
                              gucharmap_table_get_type (), GucharmapTable))

#define GUCHARMAP_TABLE_CLASS(clazz) (G_TYPE_CHECK_CLASS_CAST ((clazz), \
                                                         gucharmap_table_get_type (),\
                                                         GucharmapTableClass))

#define IS_GUCHARMAP_TABLE(obj) (G_TYPE_CHECK_INSTANCE_TYPE ((obj), \
                                 gucharmap_table_get_type ()))


typedef struct _GucharmapTable GucharmapTable;
typedef struct _GucharmapTableClass GucharmapTableClass;


struct _GucharmapTable
{
  GtkHBox parent;

  /* rows and columns on a page */
  gint rows, cols;

  GtkWidget *drawing_area;         /* GtkDrawingArea */
  GdkPixmap *pixmap; 

  gchar *font_name;
  PangoFontMetrics *font_metrics;
  PangoLayout *pango_layout;

  gunichar page_first_char;  /* the character in the upper left square */
  gunichar active_char;
  gunichar old_page_first_char; /* helps us know what to redraw */
  gunichar old_active_char;

  /* for the scrollbar */
  GtkObject *adjustment; 
  gulong adjustment_changed_handler_id; 

  GtkWidget *zoom_window;
  GdkPixmap *zoom_pixmap;
  gboolean zoom_mode_enabled;

  gboolean snap_pow2_enabled;
};


struct _GucharmapTableClass
{
  GtkHBoxClass parent_class;

  void (* activate) (GucharmapTable *chartable, gunichar uc);
  void (* set_active_char) (GucharmapTable *chartable, guint ch);
  void (* status_message) (GucharmapTable *chartable, const gchar *message);
};


GType gucharmap_table_get_type (void);
GtkWidget * gucharmap_table_new (void);
void gucharmap_table_set_font (GucharmapTable *chartable, 
                               const gchar *font_name);
gunichar gucharmap_table_get_active_character (GucharmapTable *chartable);
void gucharmap_table_set_active_character (GucharmapTable *chartable, 
                                           gunichar uc);
void gucharmap_table_zoom_enable (GucharmapTable *chartable);
void gucharmap_table_zoom_disable (GucharmapTable *chartable);
void gucharmap_table_identify_clipboard (GucharmapTable *chartable, 
                                         GtkClipboard *clipboard);
void gucharmap_table_grab_focus (GucharmapTable *chartable);
void gucharmap_table_set_snap_pow2 (GucharmapTable *chartable, 
                                    gboolean snap);


G_END_DECLS

#endif  /* #ifndef GUCHARMAP_TABLE_H */


