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

#ifndef GUCHARMAP_CHARTABLE_H
#define GUCHARMAP_CHARTABLE_H

#include <gtk/gtkdrawingarea.h>
#include <gucharmap/gucharmap-codepoint-list.h>

G_BEGIN_DECLS

#define GUCHARMAP_TYPE_CHARTABLE (gucharmap_chartable_get_type ())

#define GUCHARMAP_CHARTABLE(obj) (G_TYPE_CHECK_INSTANCE_CAST ((obj), \
                              gucharmap_chartable_get_type (), GucharmapChartable))

#define GUCHARMAP_CHARTABLE_CLASS(clazz) (G_TYPE_CHECK_CLASS_CAST ((clazz), gucharmap_chartable_get_type (), GucharmapChartableClass))

#define IS_GUCHARMAP_CHARTABLE(obj) (G_TYPE_CHECK_INSTANCE_TYPE ((obj), gucharmap_chartable_get_type ()))

typedef struct _GucharmapChartable GucharmapChartable;
typedef struct _GucharmapChartableClass GucharmapChartableClass;

GType gucharmap_chartable_get_type (void);
GtkWidget * gucharmap_chartable_new (void);
void gucharmap_chartable_set_font (GucharmapChartable *chartable, 
                                   const gchar *font_name);
gunichar gucharmap_chartable_get_active_character (GucharmapChartable *chartable);
void gucharmap_chartable_set_active_character (GucharmapChartable *chartable, 
                                               gunichar uc);
void gucharmap_chartable_set_zoom_enabled (GucharmapChartable *chartable,
                                           gboolean enabled);
void gucharmap_chartable_set_snap_pow2 (GucharmapChartable *chartable,
                                        gboolean snap);
void gucharmap_chartable_set_codepoint_list (GucharmapChartable         *chartable,
                                             GucharmapCodepointList *list);
GucharmapCodepointList * gucharmap_chartable_get_codepoint_list (GucharmapChartable *chartable);
gint gucharmap_chartable_get_active_cell (GucharmapChartable *chartable);

G_END_DECLS

#endif  /* #ifndef GUCHARMAP_CHARTABLE_H */
