/* $Id: gucharmap-block-chapters.h 919 2005-09-08 13:35:59Z behdad $ */
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

/* block means unicode block */

#ifndef GUCHARMAP_CHAPTERS_VIEW_H
#define GUCHARMAP_CHAPTERS_VIEW_H

#include <gtk/gtktreeview.h>
#include <gucharmap/gucharmap-chapters-model.h>

G_BEGIN_DECLS

#define GUCHARMAP_CHAPTERS_VIEW(obj) \
            (G_TYPE_CHECK_INSTANCE_CAST ((obj), gucharmap_chapters_view_get_type (), GucharmapChaptersView))

#define GUCHARMAP_CHAPTERS_VIEW_CLASS(clazz) \
            (G_TYPE_CHECK_CLASS_CAST ((clazz), gucharmap_chapters_view_get_type (), GucharmapChaptersViewClass))

#define GUCHARMAP_IS_CHAPTERS_VIEW(obj) \
            (G_TYPE_CHECK_INSTANCE_TYPE ((obj), gucharmap_chapters_view_get_type ()))

#define GUCHARMAP_CHAPTERS_VIEW_GET_CLASS(obj) \
            (G_TYPE_INSTANCE_GET_CLASS ((obj), gucharmap_chapters_view_get_type (), GucharmapChaptersViewClass))

typedef struct _GucharmapChaptersView GucharmapChaptersView;
typedef struct _GucharmapChaptersViewClass GucharmapChaptersViewClass;

struct _GucharmapChaptersView
{
  GtkTreeView parent_instance;

  /*< private >*/
  GtkTreeViewColumn *column;
  GucharmapChaptersModel *model;
};

struct _GucharmapChaptersViewClass
{
  GtkTreeViewClass parent_class;
};

GType       gucharmap_chapters_view_get_type (void);
GtkWidget * gucharmap_chapters_view_new      (void);

void                    gucharmap_chapters_view_set_model (GucharmapChaptersView *view,
                                                           GucharmapChaptersModel *model);
GucharmapChaptersModel *gucharmap_chapters_view_get_model (GucharmapChaptersView *view);

gboolean           gucharmap_chapters_view_select_character (GucharmapChaptersView *view,
                                                             gunichar           wc);
GucharmapCodepointList *                gucharmap_chapters_view_get_codepoint_list      (GucharmapChaptersView *view);
G_CONST_RETURN GucharmapCodepointList * gucharmap_chapters_view_get_book_codepoint_list (GucharmapChaptersView *view);

void               gucharmap_chapters_view_next         (GucharmapChaptersView *view);
void               gucharmap_chapters_view_previous     (GucharmapChaptersView *view);

gchar *            gucharmap_chapters_view_get_selected  (GucharmapChaptersView *view);
gboolean           gucharmap_chapters_view_set_selected  (GucharmapChaptersView *view,
                                                          const gchar       *name);
G_END_DECLS

#endif /* #ifndef GUCHARMAP_CHAPTERS_VIEW_H */
