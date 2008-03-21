/*
 * Copyright © 2004 Noah Levitt
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
 
#ifndef GUCHARMAP_WINDOW_H
#define GUCHARMAP_WINDOW_H

#include <gtk/gtk.h>
#include <gucharmap/gucharmap.h>
#include "gucharmap-mini-fontsel.h"

G_BEGIN_DECLS

#define GUCHARMAP_TYPE_WINDOW             (gucharmap_window_get_type ())
#define GUCHARMAP_WINDOW(o)               (G_TYPE_CHECK_INSTANCE_CAST ((o), GUCHARMAP_TYPE_WINDOW, GucharmapWindow))
#define GUCHARMAP_WINDOW_CLASS(k) (G_TYPE_CHECK_CLASS_CAST((k), GUCHARMAP_TYPE_WINDOW, GucharmapWindowClass))
#define GUCHARMAP_IS_WINDOW(o)            (G_TYPE_CHECK_INSTANCE_TYPE ((o), GUCHARMAP_TYPE_WINDOW))
#define GUCHARMAP_IS_WINDOW_CLASS(k)      (G_TYPE_CHECK_CLASS_TYPE ((k), GUCHARMAP_TYPE_WINDOW))
#define GUCHARMAP_WINDOW_GET_CLASS(o)     (G_TYPE_INSTANCE_GET_CLASS ((o), GUCHARMAP_TYPE_WINDOW, GucharmapWindowClass))

typedef struct _GucharmapWindow GucharmapWindow;
typedef struct _GucharmapWindowClass GucharmapWindowClass;

struct _GucharmapWindow
{
  GtkWindow parent;

  GucharmapCharmap *charmap;
  GtkWidget *status;

  GtkWidget *fontsel;
  GtkWidget *text_to_copy_container; /* the thing to show/hide */
  GtkWidget *text_to_copy_entry;

  GtkUIManager *uimanager;

  GtkActionGroup *action_group;

  GtkWidget *search_dialog; /* takes care of all aspects of searching */

  GtkWidget *progress;

  guint save_last_char_idle_id;

  guint font_selection_visible : 1;
  guint text_to_copy_visible   : 1;
  guint file_menu_visible      : 1;

  GucharmapChaptersMode chapters_mode; 
};

struct _GucharmapWindowClass
{
  GtkWindowClass parent_class;
};

GType                        gucharmap_window_get_type                   (void);
GtkWidget *                  gucharmap_window_new                        (void);
void                         gucharmap_window_set_font_selection_visible (GucharmapWindow *guw, 
                                                                          gboolean         visible);
void                         gucharmap_window_set_text_to_copy_visible   (GucharmapWindow *guw, 
                                                                          gboolean         visible);
void                         gucharmap_window_set_file_menu_visible      (GucharmapWindow *guw, 
                                                                          gboolean         visible);
GucharmapMiniFontSelection * gucharmap_window_get_mini_font_selection    (GucharmapWindow *guw);

GdkCursor *                 _gucharmap_window_progress_cursor (void);

G_END_DECLS

#endif /* #ifndef GUCHARMAP_WINDOW_H */
