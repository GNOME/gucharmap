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

/* GucharmapSearchDialog handles all aspects of searching */

#if !defined (__GUCHARMAP_GUCHARMAP_H_INSIDE__) && !defined (GUCHARMAP_COMPILATION)
#error "Only <gucharmap/gucharmap.h> can be included directly."
#endif

#ifndef GUCHARMAP_SEARCH_DIALOG_H
#define GUCHARMAP_SEARCH_DIALOG_H

#include <gtk/gtk.h>
#include <gucharmap/gucharmap-types.h>
#include <gucharmap/gucharmap-window.h>

G_BEGIN_DECLS

#define GUCHARMAP_SEARCH_DIALOG(obj) (G_TYPE_CHECK_INSTANCE_CAST ((obj), gucharmap_search_dialog_get_type (), GucharmapSearchDialog))

typedef enum
{
  GUCHARMAP_DIRECTION_BACKWARD = -1,
  GUCHARMAP_DIRECTION_FORWARD = 1
}
GucharmapDirection;

GType       gucharmap_search_dialog_get_type      (void);
GtkWidget * gucharmap_search_dialog_new           (GucharmapWindow *parent);
void        gucharmap_search_dialog_start_search  (GucharmapSearchDialog *search_dialog,
                                                   GucharmapDirection     direction);
gdouble     gucharmap_search_dialog_get_completed (GucharmapSearchDialog *search_dialog); 

G_END_DECLS

#endif /* #ifndef GUCHARMAP_SEARCH_DIALOG_H */
