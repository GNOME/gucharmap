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

#if !defined (__GUCHARMAP_GUCHARMAP_H_INSIDE__) && !defined (GUCHARMAP_COMPILATION)
#error "Only <gucharmap/gucharmap.h> can be included directly."
#endif

#ifndef GUCHARMAP_CHARMAP_H
#define GUCHARMAP_CHARMAP_H

#include <gtk/gtk.h>

#include <gucharmap/gucharmap-chapters-model.h>
#include <gucharmap/gucharmap-chapters-view.h>
#include <gucharmap/gucharmap-chartable.h>
#include <gucharmap/gucharmap-macros.h>

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
  GtkPaned parent;

  /*< private >*/
  GucharmapCharmapPrivate *priv;
};

struct _GucharmapCharmapClass
{
  GtkPanedClass parent_class;

  void (* status_message) (GucharmapCharmap *charmap, const gchar *message);
  void (* link_clicked) (GucharmapCharmap *charmap, 
                         gunichar old_character,
                         gunichar new_character);
  void (* _gucharmap_reserved0) (void);
  void (* _gucharmap_reserved1) (void);
  void (* _gucharmap_reserved2) (void);
  void (* _gucharmap_reserved3) (void);
};

_GUCHARMAP_PUBLIC
GType                 gucharmap_charmap_get_type           (void);

_GUCHARMAP_PUBLIC
GtkWidget *           gucharmap_charmap_new                (void);

_GUCHARMAP_PUBLIC
void      gucharmap_charmap_set_active_character (GucharmapCharmap *charmap,
                                                  gunichar           uc);
_GUCHARMAP_PUBLIC
gunichar  gucharmap_charmap_get_active_character (GucharmapCharmap *charmap);

_GUCHARMAP_PUBLIC
void      gucharmap_charmap_set_active_chapter (GucharmapCharmap *charmap,
                                                const gchar *chapter);
_GUCHARMAP_PUBLIC
char *    gucharmap_charmap_get_active_chapter (GucharmapCharmap *charmap);

_GUCHARMAP_PUBLIC
void gucharmap_charmap_next_chapter     (GucharmapCharmap *charmap);
_GUCHARMAP_PUBLIC
void gucharmap_charmap_previous_chapter (GucharmapCharmap *charmap);

_GUCHARMAP_PUBLIC
void                     gucharmap_charmap_set_font_desc      (GucharmapCharmap  *charmap,
                                                               PangoFontDescription *font_desc);

_GUCHARMAP_PUBLIC
PangoFontDescription *   gucharmap_charmap_get_font_desc      (GucharmapCharmap  *charmap);

_GUCHARMAP_PUBLIC
void                     gucharmap_charmap_set_font_fallback  (GucharmapCharmap *charmap,
                                                               gboolean enable_font_fallback);
_GUCHARMAP_PUBLIC
gboolean                 gucharmap_charmap_get_font_fallback  (GucharmapCharmap *charmap);

_GUCHARMAP_PUBLIC
GucharmapChaptersView *  gucharmap_charmap_get_chapters_view  (GucharmapCharmap       *charmap);

_GUCHARMAP_PUBLIC
void                     gucharmap_charmap_set_chapters_model (GucharmapCharmap       *charmap,
                                                               GucharmapChaptersModel *model);

_GUCHARMAP_PUBLIC
GucharmapChaptersModel * gucharmap_charmap_get_chapters_model (GucharmapCharmap       *charmap);

_GUCHARMAP_PUBLIC
GucharmapCodepointList * gucharmap_charmap_get_active_codepoint_list (GucharmapCharmap *charmap);

_GUCHARMAP_PUBLIC
GucharmapCodepointList * gucharmap_charmap_get_book_codepoint_list (GucharmapCharmap *charmap);

_GUCHARMAP_PUBLIC
void     gucharmap_charmap_set_chapters_visible (GucharmapCharmap *charmap,
                                                 gboolean visible);

_GUCHARMAP_PUBLIC
gboolean gucharmap_charmap_get_chapters_visible (GucharmapCharmap *charmap);

/**
 * GucharmapCharmapPageType:
 * @GUCHARMAP_CHARMAP_PAGE_CHARTABLE: Character table page
 * @GUCHARMAP_CHARMAP_PAGE_DETAILS: Character detail page
 */
typedef enum {
  GUCHARMAP_CHARMAP_PAGE_CHARTABLE,
  GUCHARMAP_CHARMAP_PAGE_DETAILS
} GucharmapCharmapPageType;

_GUCHARMAP_PUBLIC
void     gucharmap_charmap_set_page_visible (GucharmapCharmap *charmap,
                                             int page,
                                             gboolean visible);

_GUCHARMAP_PUBLIC
gboolean gucharmap_charmap_get_page_visible (GucharmapCharmap *charmap,
                                             int page);

_GUCHARMAP_PUBLIC
void gucharmap_charmap_set_active_page (GucharmapCharmap *charmap,
                                        int page);

_GUCHARMAP_PUBLIC
int  gucharmap_charmap_get_active_page (GucharmapCharmap *charmap);

_GUCHARMAP_PUBLIC
void gucharmap_charmap_set_snap_pow2 (GucharmapCharmap *charmap,
                                      gboolean snap);
_GUCHARMAP_PUBLIC
gboolean gucharmap_charmap_get_snap_pow2 (GucharmapCharmap *charmap);

/* private; FIXMEchpe remove */
_GUCHARMAP_PUBLIC
GucharmapChartable *     gucharmap_charmap_get_chartable      (GucharmapCharmap  *charmap);

/* Hide deprecated stuff from GI */

#ifndef __GI_SCANNER__

_GUCHARMAP_PUBLIC
GUCHARMAP_DEPRECATED_FOR(gtk_orientable_set_orientation)
void           gucharmap_charmap_set_orientation (GucharmapCharmap *charmap,
                                                  GtkOrientation orientation);
_GUCHARMAP_PUBLIC
GUCHARMAP_DEPRECATED_FOR(gtk_orientable_get_orientation)
GtkOrientation gucharmap_charmap_get_orientation (GucharmapCharmap *charmap);

#endif /* !__GI_SCANNER__ */

G_END_DECLS

#endif  /* #ifndef GUCHARMAP_CHARMAP_H */
