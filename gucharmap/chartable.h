/* $Id$ */
/*
 * Copyright (c) 2003  Noah Levitt <nlevitt аt columbia.edu>
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

#ifndef CHARTABLE_H
#define CHARTABLE_H

#include <gtk/gtk.h>

G_BEGIN_DECLS


#define CHARTABLE(obj) (G_TYPE_CHECK_INSTANCE_CAST ((obj), \
                        chartable_get_type (), Chartable))

#define CHARTABLE_CLASS(clazz) (G_TYPE_CHECK_CLASS_CAST ((clazz), \
                                                       chartable_get_type (),\
                                                       ChartableClass))

#define IS_CHARTABLE(obj) (G_TYPE_CHECK_INSTANCE_TYPE ((obj), \
                         chartable_get_type ()))


typedef struct _Chartable Chartable;
typedef struct _ChartableClass ChartableClass;


struct _Chartable
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
  GdkPixbuf *zoom_pixbuf;
  gboolean zoom_mode_enabled;
};


struct _ChartableClass
{
  GtkHBoxClass parent_class;

  void (* activate) (Chartable *chartable, gunichar uc);
  void (* set_active_char) (Chartable *chartable, guint ch);
  void (* status_message) (Chartable *chartable, const gchar *message);
};


GType chartable_get_type (void);
GtkWidget * chartable_new (void);
void chartable_set_font (Chartable *chartable, const gchar *font_name);
gunichar chartable_get_active_character (Chartable *chartable);
void chartable_set_active_character (Chartable *chartable, gunichar uc);
void chartable_zoom_enable (Chartable *chartable);
void chartable_zoom_disable (Chartable *chartable);
void chartable_identify_clipboard (Chartable *chartable, 
                                   GtkClipboard *clipboard);
void chartable_grab_focus (Chartable *chartable);


G_END_DECLS

#endif  /* #ifndef CHARTABLE_H */


