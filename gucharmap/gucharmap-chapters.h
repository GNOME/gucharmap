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

#ifndef GUCHARMAP_CHAPTERS_H
#define GUCHARMAP_CHAPTERS_H

#include <gtk/gtk.h>
#include <gucharmap/gucharmap-codepoint-list.h>

G_BEGIN_DECLS

#define GUCHARMAP_CHAPTERS(obj) \
            (G_TYPE_CHECK_INSTANCE_CAST ((obj), gucharmap_chapters_get_type (), GucharmapChapters))

#define GUCHARMAP_CHAPTERS_CLASS(clazz) \
            (G_TYPE_CHECK_CLASS_CAST ((clazz), gucharmap_chapters_get_type (), GucharmapChaptersClass))

#define IS_GUCHARMAP_CHAPTERS(obj) \
            (G_TYPE_CHECK_INSTANCE_TYPE ((obj), gucharmap_chapters_get_type ()))

#define GUCHARMAP_CHAPTERS_GET_CLASS(obj) \
            (G_TYPE_INSTANCE_GET_CLASS ((obj), gucharmap_chapters_get_type (), GucharmapChaptersClass))

typedef struct _GucharmapChapters GucharmapChapters;
typedef struct _GucharmapChaptersClass GucharmapChaptersClass;

struct _GucharmapChapters
{
  GtkScrolledWindow parent;

  /*< protected >*/
  GtkTreeModel           *tree_model;
  GtkWidget              *tree_view;
  GucharmapCodepointList *book_list;
};

struct _GucharmapChaptersClass
{
  GtkScrolledWindowClass parent_class;

  void                                    (* changed)                 (GucharmapChapters *chapters);
  GucharmapCodepointList *                (* get_codepoint_list)      (GucharmapChapters *chapters);
  G_CONST_RETURN GucharmapCodepointList * (* get_book_codepoint_list) (GucharmapChapters *chapters);
  gboolean                                (* go_to_character)         (GucharmapChapters *chapters, 
                                                                       gunichar           wc);
};

GType                                   gucharmap_chapters_get_type                ();
GucharmapCodepointList *                gucharmap_chapters_get_codepoint_list      (GucharmapChapters *chapters);
G_CONST_RETURN GucharmapCodepointList * gucharmap_chapters_get_book_codepoint_list (GucharmapChapters *chapters);
gboolean                                gucharmap_chapters_go_to_character         (GucharmapChapters *chapters, 
                                                                                    gunichar           wc);

G_END_DECLS

#endif /* #ifndef GUCHARMAP_CHAPTERS_H */
