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

#ifndef GUCHARMAP_TABLE_H
#define GUCHARMAP_TABLE_H

#include <gtk/gtk.h>
#include <gucharmap/gucharmap-codepoint-list.h>

G_BEGIN_DECLS

#define GUCHARMAP_TYPE_TABLE (gucharmap_table_get_type ())

#define GUCHARMAP_TABLE(obj) (G_TYPE_CHECK_INSTANCE_CAST ((obj), \
                              gucharmap_table_get_type (), GucharmapTable))

#define GUCHARMAP_TABLE_CLASS(clazz) (G_TYPE_CHECK_CLASS_CAST ((clazz), gucharmap_table_get_type (), GucharmapTableClass))

#define IS_GUCHARMAP_TABLE(obj) (G_TYPE_CHECK_INSTANCE_TYPE ((obj), gucharmap_table_get_type ()))

typedef struct _GucharmapTable GucharmapTable;
typedef struct _GucharmapTableClass GucharmapTableClass;

struct _GucharmapTable
{
  GtkHBox parent;

  /* FIXME: remove all this crap when bumping the ABI version */
  /* rows and columns on a page */
  gint _unused_1, _unused_2;

  GtkWidget *drawing_area;
  GtkWidget *scrolled_window;

  gpointer _unused_4;
  gpointer _unused_5;

  gint _unused_6;
  gint _unused_7;
  gint _unused_8;
  gint _unused_9;

  /* for the scrollbar */
  gpointer _unused_10;
  gulong _unused_11;
  gpointer _unused_12;

  gpointer _unused_13;
  gpointer _unused_14;
  gboolean _unused_15;

  gboolean _unused_16;

  /* for dragging (#114534) */
  gdouble _unused_17, _unused_18;

  gpointer _unused_19;

  gpointer _unused_20;
  gboolean _unused_21;
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
void gucharmap_table_grab_focus (GucharmapTable *chartable);
void gucharmap_table_set_snap_pow2 (GucharmapTable *chartable, 
                                    gboolean snap);
void gucharmap_table_set_codepoint_list (GucharmapTable         *chartable,
                                         GucharmapCodepointList *list);
GucharmapCodepointList * gucharmap_table_get_codepoint_list (GucharmapTable         *chartable);
gint gucharmap_table_get_active_cell (GucharmapTable *chartable);

G_END_DECLS

#endif  /* #ifndef GUCHARMAP_TABLE_H */
