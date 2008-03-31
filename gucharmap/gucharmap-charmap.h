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

#if !defined (__GUCHARMAP_GUCHARMAP_H_INSIDE__) && !defined (GUCHARMAP_COMPILATION)
#error "Only <gucharmap/gucharmap.h> can be included directly."
#endif

#ifndef GUCHARMAP_CHARMAP_H
#define GUCHARMAP_CHARMAP_H

#include <gtk/gtk.h>

#include <gucharmap/gucharmap-chapters-model.h>
#include <gucharmap/gucharmap-chapters-view.h>
#include <gucharmap/gucharmap-chartable.h>

G_BEGIN_DECLS

#define GUCHARMAP_TYPE_CHARMAP             (gucharmap_charmap_get_type ())
#define GUCHARMAP_CHARMAP(o)               (G_TYPE_CHECK_INSTANCE_CAST ((o), GUCHARMAP_TYPE_CHARMAP, GucharmapCharmap))
#define GUCHARMAP_CHARMAP_CLASS(k)         (G_TYPE_CHECK_CLASS_CAST((k), GUCHARMAP_TYPE_CHARMAP, GucharmapCharmapClass))
#define GUCHARMAP_IS_CHARMAP(o)            (G_TYPE_CHECK_INSTANCE_TYPE ((o), GUCHARMAP_TYPE_CHARMAP))
#define GUCHARMAP_IS_CHARMAP_CLASS(k)      (G_TYPE_CHECK_CLASS_TYPE ((k), GUCHARMAP_TYPE_CHARMAP))
#define GUCHARMAP_CHARMAP_GET_CLASS(o)     (G_TYPE_INSTANCE_GET_CLASS ((o), GUCHARMAP_TYPE_CHARMAP, GucharmapCharmapClass))

typedef struct _GucharmapCharmap        GucharmapCharmap;
typedef struct _GucharmapCharmapPrivate GucharmapCharmapPrivate;
typedef struct _GucharmapCharmapClass   GucharmapCharmapClass;

struct _GucharmapCharmap
{
  GtkHPaned parent;

  /*< private >*/
  GucharmapCharmapPrivate *priv;
};

struct _GucharmapCharmapClass
{
  GtkHPanedClass parent_class;

  void (* status_message) (GucharmapCharmap *charmap, const gchar *message);
  void (* link_clicked) (GucharmapCharmap *charmap, 
                         gunichar old_character,
                         gunichar new_character);
  void (* _gucharmap_reserved0) (void);
  void (* _gucharmap_reserved1) (void);
  void (* _gucharmap_reserved2) (void);
  void (* _gucharmap_reserved3) (void);
};

GType                 gucharmap_charmap_get_type           (void);

GtkWidget *           gucharmap_charmap_new                (void);

void      gucharmap_charmap_set_active_character (GucharmapCharmap *charmap,
                                                  gunichar           uc);
gunichar  gucharmap_charmap_get_active_character (GucharmapCharmap *charmap);

void      gucharmap_charmap_set_active_chapter (GucharmapCharmap *charmap,
                                                const gchar *chapter);
char *    gucharmap_charmap_get_active_chapter (GucharmapCharmap *charmap);

void                     gucharmap_charmap_set_font_desc      (GucharmapCharmap  *charmap,
                                                               PangoFontDescription *font_desc);

PangoFontDescription *   gucharmap_charmap_get_font_desc      (GucharmapCharmap  *charmap);

GucharmapChartable *     gucharmap_charmap_get_chartable      (GucharmapCharmap  *charmap);

GucharmapChaptersView *  gucharmap_charmap_get_chapters_view  (GucharmapCharmap       *charmap);

void                     gucharmap_charmap_set_chapters_model (GucharmapCharmap       *charmap,
                                                               GucharmapChaptersModel *model);

GucharmapChaptersModel * gucharmap_charmap_get_chapters_model (GucharmapCharmap       *charmap);

GucharmapCodepointList * gucharmap_charmap_get_book_codepoint_list (GucharmapCharmap *charmap);

G_END_DECLS

#endif  /* #ifndef GUCHARMAP_CHARMAP_H */
