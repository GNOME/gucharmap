/*
 * Copyright © 2004 Noah Levitt
 * Copyright © 2007 Christian Persch
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

#ifndef GUCHARMAP_CHARTABLE_H
#define GUCHARMAP_CHARTABLE_H

#include <gtk/gtk.h>

#include <gucharmap/gucharmap-codepoint-list.h>
#include <gucharmap/gucharmap-macros.h>

G_BEGIN_DECLS

#define GUCHARMAP_TYPE_CHARTABLE             (gucharmap_chartable_get_type ())
#define GUCHARMAP_CHARTABLE(o)               (G_TYPE_CHECK_INSTANCE_CAST ((o), GUCHARMAP_TYPE_CHARTABLE, GucharmapChartable))
#define GUCHARMAP_CHARTABLE_CLASS(k)         (G_TYPE_CHECK_CLASS_CAST((k), GUCHARMAP_TYPE_CHARTABLE, GucharmapChartableClass))
#define GUCHARMAP_IS_CHARTABLE(o)            (G_TYPE_CHECK_INSTANCE_TYPE ((o), GUCHARMAP_TYPE_CHARTABLE))
#define GUCHARMAP_IS_CHARTABLE_CLASS(k)      (G_TYPE_CHECK_CLASS_TYPE ((k), GUCHARMAP_TYPE_CHARTABLE))
#define GUCHARMAP_CHARTABLE_GET_CLASS(o)     (G_TYPE_INSTANCE_GET_CLASS ((o), GUCHARMAP_TYPE_CHARTABLE, GucharmapChartableClass))

typedef struct _GucharmapChartable        GucharmapChartable;
typedef struct _GucharmapChartablePrivate GucharmapChartablePrivate;
typedef struct _GucharmapChartableClass   GucharmapChartableClass;

struct _GucharmapChartable
{
  GtkDrawingArea parent_instance;

  /*< private >*/
  GucharmapChartablePrivate *priv;
};

struct _GucharmapChartableClass
{
  GtkDrawingAreaClass parent_class;

  void    (* set_scroll_adjustments) (GucharmapChartable *chartable,
                                      GtkAdjustment      *hadjustment,
                                      GtkAdjustment      *vadjustment);
  gboolean (* move_cursor)           (GucharmapChartable *chartable,
                                      GtkMovementStep     step,
                                      gint                count);
  void (* activate) (GucharmapChartable *chartable);
  void (* copy_clipboard) (GucharmapChartable *chartable);
  void (* paste_clipboard) (GucharmapChartable *chartable);

  void (* set_active_char) (GucharmapChartable *chartable, guint ch);
  void (* status_message) (GucharmapChartable *chartable, const gchar *message);
};

_GUCHARMAP_PUBLIC
GType gucharmap_chartable_get_type (void);
_GUCHARMAP_PUBLIC
GtkWidget * gucharmap_chartable_new (void);
_GUCHARMAP_PUBLIC
void gucharmap_chartable_set_font_desc (GucharmapChartable *chartable,
                                        PangoFontDescription *font_desc);
_GUCHARMAP_PUBLIC
PangoFontDescription * gucharmap_chartable_get_font_desc (GucharmapChartable *chartable);
_GUCHARMAP_PUBLIC
void gucharmap_chartable_set_font_fallback (GucharmapChartable *chartable,
                                            gboolean enable_font_fallback);
_GUCHARMAP_PUBLIC
gboolean gucharmap_chartable_get_font_fallback (GucharmapChartable *chartable);
_GUCHARMAP_PUBLIC
gunichar gucharmap_chartable_get_active_character (GucharmapChartable *chartable);
_GUCHARMAP_PUBLIC
void gucharmap_chartable_set_active_character (GucharmapChartable *chartable,
                                               gunichar wc);
_GUCHARMAP_PUBLIC
void gucharmap_chartable_set_zoom_enabled (GucharmapChartable *chartable,
                                           gboolean enabled);
_GUCHARMAP_PUBLIC
gboolean gucharmap_chartable_get_zoom_enabled (GucharmapChartable *chartable);
_GUCHARMAP_PUBLIC
void gucharmap_chartable_set_snap_pow2 (GucharmapChartable *chartable,
                                        gboolean snap);
_GUCHARMAP_PUBLIC
gboolean gucharmap_chartable_get_snap_pow2 (GucharmapChartable *chartable);
_GUCHARMAP_PUBLIC
void gucharmap_chartable_set_codepoint_list (GucharmapChartable         *chartable,
                                             GucharmapCodepointList *codepoint_list);
_GUCHARMAP_PUBLIC
GucharmapCodepointList * gucharmap_chartable_get_codepoint_list (GucharmapChartable *chartable);

G_END_DECLS

#endif  /* #ifndef GUCHARMAP_CHARTABLE_H */
