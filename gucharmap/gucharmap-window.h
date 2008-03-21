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
 
#if !defined (__GUCHARMAP_GUCHARMAP_H_INSIDE__) && !defined (GUCHARMAP_COMPILATION)
#error "Only <gucharmap/gucharmap.h> can be included directly."
#endif

#ifndef GUCHARMAP_WINDOW_H
#define GUCHARMAP_WINDOW_H

#include <gtk/gtk.h>
#include <gucharmap/gucharmap-types.h>
#include <gucharmap/gucharmap-charmap.h>
#include <gucharmap/gucharmap-mini-fontsel.h>

G_BEGIN_DECLS

#define GUCHARMAP_WINDOW(obj) (G_TYPE_CHECK_INSTANCE_CAST ((obj), gucharmap_window_get_type (), GucharmapWindow))

#define GUCHARMAP_WINDOW_CLASS(clazz) (G_TYPE_CHECK_CLASS_CAST ((clazz), gucharmap_window_get_type (), GucharmapWindowClass))

#define GUCHARMAP_IS_WINDOW(obj) (G_TYPE_CHECK_INSTANCE_TYPE ((obj), gucharmap_window_get_type ()))

GType                        gucharmap_window_get_type                   (void);
GtkWidget *                  gucharmap_window_new                        (void);
void                         gucharmap_window_set_font_selection_visible (GucharmapWindow *guw, 
                                                                          gboolean         visible);
void                         gucharmap_window_set_text_to_copy_visible   (GucharmapWindow *guw, 
                                                                          gboolean         visible);
void                         gucharmap_window_set_file_menu_visible      (GucharmapWindow *guw, 
                                                                          gboolean         visible);
GucharmapMiniFontSelection * gucharmap_window_get_mini_font_selection    (GucharmapWindow *guw);

G_END_DECLS

#endif /* #ifndef GUCHARMAP_WINDOW_H */


