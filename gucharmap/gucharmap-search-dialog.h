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

/* GucharmapSearchDialog handles all aspects of searching */

#ifndef GUCHARMAP_SEARCH_DIALOG_H
#define GUCHARMAP_SEARCH_DIALOG_H

#include <gtk/gtk.h>
#include <gucharmap/gucharmap-window.h>

G_BEGIN_DECLS

#define GUCHARMAP_SEARCH_DIALOG(obj) (G_TYPE_CHECK_INSTANCE_CAST ((obj), gucharmap_search_dialog_get_type (), GucharmapSearchDialog))

typedef enum
{
  GUCHARMAP_DIRECTION_BACKWARD = -1,
  GUCHARMAP_DIRECTION_FORWARD = 1
}
GucharmapDirection;

typedef struct _GucharmapSearchDialog GucharmapSearchDialog;
typedef struct _GucharmapSearchDialogClass GucharmapSearchDialogClass;

struct _GucharmapSearchDialog
{
  GtkDialog parent;
};

struct _GucharmapSearchDialogClass
{
  GtkDialogClass parent_class;

  /* signals */
  void (* search_start)  ();
  void (* search_finish) (gunichar found_char);
};

GType       gucharmap_search_dialog_get_type     ();
GtkWidget * gucharmap_search_dialog_new          (GucharmapWindow *parent);
void        gucharmap_search_dialog_start_search (GucharmapSearchDialog *search_dialog,
                                                  GucharmapDirection     direction);

G_END_DECLS

#endif /* #ifndef GUCHARMAP_SEARCH_DIALOG_H */
