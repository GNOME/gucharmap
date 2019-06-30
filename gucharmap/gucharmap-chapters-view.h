/*
 * Copyright © 2004 Noah Levitt
 *
 * This program is free software; you can redistribute it and/or modify it
 * under the terms of the GNU General Public License as published by the
 * Free Software Foundation; either version 3 of the License, or (at your
 * option) any later version.
 *
 * This program is distributed in the hope that it will be useful, but
 * WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License along
 * with this program; if not, write to the Free Software Foundation, Inc.,
 * 51 Franklin Street, Fifth Floor, Boston, MA 02110-1335 USA
 */

/* block means unicode block */

#if !defined (__GUCHARMAP_GUCHARMAP_H_INSIDE__) && !defined (GUCHARMAP_COMPILATION)
#error "Only <gucharmap/gucharmap.h> can be included directly."
#endif

#ifndef GUCHARMAP_CHAPTERS_VIEW_H
#define GUCHARMAP_CHAPTERS_VIEW_H

#include <gtk/gtk.h>

#include <gucharmap/gucharmap-chapters-model.h>
#include <gucharmap/gucharmap-macros.h>

G_BEGIN_DECLS

#define GUCHARMAP_TYPE_CHAPTERS_VIEW             (gucharmap_chapters_view_get_type ())
#define GUCHARMAP_CHAPTERS_VIEW(o)               (G_TYPE_CHECK_INSTANCE_CAST ((o), GUCHARMAP_TYPE_CHAPTERS_VIEW, GucharmapChaptersView))
#define GUCHARMAP_CHAPTERS_VIEW_CLASS(k)         (G_TYPE_CHECK_CLASS_CAST((k), GUCHARMAP_TYPE_CHAPTERS_VIEW, GucharmapChaptersViewClass))
#define GUCHARMAP_IS_CHAPTERS_VIEW(o)            (G_TYPE_CHECK_INSTANCE_TYPE ((o), GUCHARMAP_TYPE_CHAPTERS_VIEW))
#define GUCHARMAP_IS_CHAPTERS_VIEW_CLASS(k)      (G_TYPE_CHECK_CLASS_TYPE ((k), GUCHARMAP_TYPE_CHAPTERS_VIEW))
#define GUCHARMAP_CHAPTERS_VIEW_GET_CLASS(o)     (G_TYPE_INSTANCE_GET_CLASS ((o), GUCHARMAP_TYPE_CHAPTERS_VIEW, GucharmapChaptersViewClass))

typedef struct _GucharmapChaptersView         GucharmapChaptersView;
typedef struct _GucharmapChaptersViewPrivate  GucharmapChaptersViewPrivate;
typedef struct _GucharmapChaptersViewClass    GucharmapChaptersViewClass;

struct _GucharmapChaptersView
{
  GtkTreeView parent_instance;

  /*< private >*/
  GucharmapChaptersViewPrivate *priv;
};

struct _GucharmapChaptersViewClass
{
  GtkTreeViewClass parent_class;
};

_GUCHARMAP_PUBLIC
GType       gucharmap_chapters_view_get_type (void);

_GUCHARMAP_PUBLIC
GtkWidget * gucharmap_chapters_view_new      (void);

_GUCHARMAP_PUBLIC
void                    gucharmap_chapters_view_set_model (GucharmapChaptersView *view,
                                                           GucharmapChaptersModel *model);
_GUCHARMAP_PUBLIC
GucharmapChaptersModel *gucharmap_chapters_view_get_model (GucharmapChaptersView *view);

_GUCHARMAP_PUBLIC
gboolean           gucharmap_chapters_view_select_character (GucharmapChaptersView *view,
                                                             gunichar           wc);
_GUCHARMAP_PUBLIC
GucharmapCodepointList * gucharmap_chapters_view_get_codepoint_list      (GucharmapChaptersView *view);
_GUCHARMAP_PUBLIC
GucharmapCodepointList * gucharmap_chapters_view_get_book_codepoint_list (GucharmapChaptersView *view);

_GUCHARMAP_PUBLIC
void               gucharmap_chapters_view_next         (GucharmapChaptersView *view);
_GUCHARMAP_PUBLIC
void               gucharmap_chapters_view_previous     (GucharmapChaptersView *view);

_GUCHARMAP_PUBLIC
gchar *            gucharmap_chapters_view_get_selected  (GucharmapChaptersView *view);
_GUCHARMAP_PUBLIC
gboolean           gucharmap_chapters_view_set_selected  (GucharmapChaptersView *view,
                                                          const gchar       *name);

_GUCHARMAP_PUBLIC
gboolean           gucharmap_chapters_view_select_locale (GucharmapChaptersView *view);

G_END_DECLS

#endif /* #ifndef GUCHARMAP_CHAPTERS_VIEW_H */
