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

#ifndef GUCHARMAP_CHARMAP_H
#define GUCHARMAP_CHARMAP_H

#include <gtk/gtk.h>
#include <gucharmap/gucharmap-types.h>
#include <gucharmap/gucharmap-chartable.h>
#include <gucharmap/gucharmap-chapters-model.h>
#include <gucharmap/gucharmap-chapters-view.h>

G_BEGIN_DECLS

#define GUCHARMAP_CHARMAP(obj) (G_TYPE_CHECK_INSTANCE_CAST ((obj), \
                                gucharmap_charmap_get_type (), \
                                GucharmapCharmap))

#define GUCHARMAP_CHARMAP_CLASS(clazz) (G_TYPE_CHECK_CLASS_CAST ((clazz), \
                                        gucharmap_charmap_get_type (),\
                                        GucharmapCharmapClass))

#define GUCHARMAP_IS_CHARMAP(obj) (G_TYPE_CHECK_INSTANCE_TYPE ((obj), \
                                   gucharmap_charmap_get_type ()))

GType                 gucharmap_charmap_get_type           (void);

GtkWidget *           gucharmap_charmap_new                (void);

void      gucharmap_charmap_set_active_character (GucharmapCharmap *charmap,
                                                  gunichar           uc);
gunichar  gucharmap_charmap_get_active_character (GucharmapCharmap *charmap);

void      gucharmap_charmap_set_active_chapter (GucharmapCharmap *charmap,
                                                const gchar *chapter);
char *    gucharmap_charmap_get_active_chapter (GucharmapCharmap *charmap);

void                     gucharmap_charmap_set_font           (GucharmapCharmap  *charmap,
                                                               const gchar       *font_name);

void                     gucharmap_charmap_set_font_desc      (GucharmapCharmap  *charmap,
                                                               PangoFontDescription *font_desc);

GucharmapChartable *     gucharmap_charmap_get_chartable      (GucharmapCharmap  *charmap);

GucharmapChaptersView *  gucharmap_charmap_get_chapters_view  (GucharmapCharmap       *charmap);

void                     gucharmap_charmap_set_chapters_model (GucharmapCharmap       *charmap,
                                                               GucharmapChaptersModel *model);

GucharmapChaptersModel * gucharmap_charmap_get_chapters_model (GucharmapCharmap       *charmap);

GucharmapCodepointList * gucharmap_charmap_get_book_codepoint_list (GucharmapCharmap *charmap);

G_END_DECLS

#endif  /* #ifndef GUCHARMAP_CHARMAP_H */
