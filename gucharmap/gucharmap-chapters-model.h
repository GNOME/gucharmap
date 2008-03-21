/*
 * Copyright (c) 2004 Noah Levitt
 * Copyright (C) 2007 Christian Persch
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

#ifndef GUCHARMAP_CHAPTERS_MODEL_H
#define GUCHARMAP_CHAPTERS_MODEL_H

#include <gtk/gtkliststore.h>
#include <gucharmap/gucharmap-types.h>
#include <gucharmap/gucharmap-codepoint-list.h>

G_BEGIN_DECLS

#define GUCHARMAP_TYPE_CHAPTERS_MODEL (gucharmap_chapters_model_get_type ())

#define GUCHARMAP_CHAPTERS_MODEL(obj) \
            (G_TYPE_CHECK_INSTANCE_CAST ((obj), gucharmap_chapters_model_get_type (), GucharmapChaptersModel))

#define GUCHARMAP_CHAPTERS_MODEL_CLASS(clazz) \
            (G_TYPE_CHECK_CLASS_CAST ((clazz), gucharmap_chapters_model_get_type (), GucharmapChaptersModelClass))

#define GUCHARMAP_IS_CHAPTERS_MODEL(obj) \
            (G_TYPE_CHECK_INSTANCE_TYPE ((obj), gucharmap_chapters_model_get_type ()))

#define GUCHARMAP_CHAPTERS_MODEL_GET_CLASS(obj) \
            (G_TYPE_INSTANCE_GET_CLASS ((obj), gucharmap_chapters_model_get_type (), GucharmapChaptersModelClass))

typedef enum
{
  CHAPTERS_SCRIPT = 0,
  CHAPTERS_BLOCK  = 1
}
ChaptersMode;

enum {
  CHAPTERS_ID_COL    = 0,
  CHAPTERS_LABEL_COL = 1
};

GType                                   gucharmap_chapters_model_get_type                (void);
GucharmapCodepointList *                gucharmap_chapters_model_get_codepoint_list      (GucharmapChaptersModel *chapters,
                                                                                          GtkTreeIter            *iter);
const char *                            gucharmap_chapters_model_get_title               (GucharmapChaptersModel *chapters);
G_CONST_RETURN GucharmapCodepointList * gucharmap_chapters_model_get_book_codepoint_list (GucharmapChaptersModel *chapters);
gboolean                                gucharmap_chapters_model_character_to_iter       (GucharmapChaptersModel *chapters,
                                                                                          gunichar                wc,
                                                                                          GtkTreeIter            *iter);
gboolean                                gucharmap_chapters_model_id_to_iter              (GucharmapChaptersModel *model,
                                                                                          const char             *id,
                                                                                          GtkTreeIter            *iter);

G_END_DECLS

#endif /* #ifndef GUCHARMAP_CHAPTERS_MODEL_H */
