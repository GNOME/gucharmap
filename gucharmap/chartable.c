/* $Id$ */
/*
 * Copyright (c) 2003  Noah Levitt <nlevitt аt columbia.edu>
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


#ifdef HAVE_CONFIG_H
# include <config.h>
#endif

#include <gtk/gtk.h>
#include <gdk/gdkkeysyms.h>

#include <chartable.h>
#include <unicode_info.h>
#include <gucharmap_marshal.h>
#include <chartable_accessible.h>
#include <gucharmap_intl.h>


enum 
{
  ACTIVATE = 0,
  SET_ACTIVE_CHAR,
  STATUS_MESSAGE,
  NUM_SIGNALS
};

static guint chartable_signals[NUM_SIGNALS] = { 0, 0, 0 };


static const GtkTargetEntry dnd_target_table[] =
{
  { "UTF8_STRING", 0, 0 },
  { "COMPOUND_TEXT", 0, 0 },
  { "TEXT", 0, 0 },
  { "STRING", 0, 0 }
};


/* depends on directionality */
static gunichar
rowcol_to_unichar (Chartable *chartable, gint row, gint col)
{
  if (gtk_widget_get_direction (chartable->drawing_area) == GTK_TEXT_DIR_RTL)
    return chartable->page_first_char + row * chartable->cols 
           + (chartable->cols - col - 1);
  else
    return chartable->page_first_char + row * chartable->cols + col;
}


/* Depends on directionality. Column 0 is the furthest left.  */
gint
chartable_unichar_column (Chartable *chartable, gunichar uc)
{
  if (gtk_widget_get_direction (chartable->drawing_area) == GTK_TEXT_DIR_RTL)
    return chartable->cols - (uc - chartable->page_first_char) % chartable->cols - 1;
  else
    return (uc - chartable->page_first_char) % chartable->cols;
}


static gint
font_height (PangoFontMetrics *font_metrics)
{
  gint height;

  height = pango_font_metrics_get_ascent (font_metrics) +
           pango_font_metrics_get_descent (font_metrics);

  return PANGO_PIXELS (height);
}


/* computes the column width based solely on the font size */
static gint
bare_minimal_column_width (Chartable *chartable)
{
  /* XXX: width is not available, so use height */
  return font_height (chartable->font_metrics) + 7;
}


static gint
minimal_column_width (Chartable *chartable)
{
  gint total_extra_pixels;
  gint bare_minimal_width = bare_minimal_column_width (chartable);

  total_extra_pixels = chartable->drawing_area->allocation.width 
                       - (chartable->cols * bare_minimal_width + 1);

  return bare_minimal_width + total_extra_pixels / chartable->cols;
}


/* not all columns are necessarily the same width because of padding */
gint
chartable_column_width (Chartable *chartable, gint col)
{
  gint num_padded_columns;
  gint min_col_w = minimal_column_width (chartable);

  num_padded_columns = chartable->drawing_area->allocation.width 
                       - (min_col_w * chartable->cols + 1);

  if (chartable->cols - col <= num_padded_columns)
    return min_col_w + 1;
  else
    return min_col_w;
}


/* calculates the position of the left end of the column (just to the right
 * of the left border) */
/* XXX: calling this repeatedly is not the most efficient, but it probably
 * is the most readable */
gint
chartable_x_offset (Chartable *chartable, gint col)
{
  gint c, x;

  for (c = 0, x = 1;  c < col;  c++)
    x += chartable_column_width (chartable, c);

  return x;
}


/* computes the row height based solely on the font size */
static gint
bare_minimal_row_height (Chartable *chartable)
{
  return font_height (chartable->font_metrics) + 7;
}


static gint
minimal_row_height (Chartable *chartable)
{
  gint total_extra_pixels;
  gint bare_minimal_height = bare_minimal_row_height (chartable);

  total_extra_pixels = chartable->drawing_area->allocation.height 
                       - (chartable->rows * bare_minimal_height + 1);

  return bare_minimal_height + total_extra_pixels / chartable->rows;
}


/* not all rows are necessarily the same height because of padding */
gint
chartable_row_height (Chartable *chartable, gint row)
{
  gint num_padded_rows;
  gint min_row_h = minimal_row_height (chartable);

  num_padded_rows = chartable->drawing_area->allocation.height -
                    (min_row_h * chartable->rows + 1);

  if (chartable->rows - row <= num_padded_rows)
    return min_row_h + 1;
  else
    return min_row_h;
}


/* calculates the position of the top end of the row (just below the top
 * border) */
/* XXX: calling this repeatedly is not the most efficient, but it probably
 * is the most readable */
gint
chartable_y_offset (Chartable *chartable, gint row)
{
  gint r, y;

  for (r = 0, y = 1;  r < row;  r++)
    y += chartable_row_height (chartable, r);

  return y;
}


static AtkObject*
chartable_accessible_factory_create_accessible (GObject *obj)
{
  GtkWidget *widget;

  g_return_val_if_fail (GTK_IS_WIDGET (obj), NULL);

  widget = GTK_WIDGET (obj);
  return chartable_accessible_new (widget);
}


/* the window must be realized, or window->window will be null */
static void
set_window_background (GtkWidget *window, GdkPixmap *pixmap)
{
  gdk_window_set_back_pixmap (window->window, pixmap, FALSE);
}


static gint
compute_zoom_font_size (Chartable *chartable)
{
  gint screen_height;
  gdouble limit;
  gdouble scale;
  gint font_size;

#if GTK_CHECK_VERSION (2,1,1)
  screen_height = gdk_screen_get_height (
          gtk_widget_get_screen (chartable->drawing_area));
#else
  screen_height = gdk_screen_height ();
#endif

  limit = (0.3 * screen_height) / bare_minimal_row_height (chartable);
  scale = CLAMP (limit, 1.0, 12.0);

  font_size = pango_font_description_get_size (
          gtk_widget_get_style (chartable->drawing_area)->font_desc);

  return scale * ((font_size > 0) ? font_size : 10 * PANGO_SCALE);
}


static PangoLayout *
layout_scaled_glyph (Chartable *chartable, gunichar uc, gint font_size)
{
  PangoFontDescription *font_desc;
  PangoLayout *layout;
  gchar buf[11];

  font_desc = pango_font_description_copy (
          gtk_widget_get_style (chartable->drawing_area)->font_desc);
  pango_font_description_set_size (font_desc, font_size);

  layout = pango_layout_new (
          pango_layout_get_context (chartable->pango_layout));

  pango_layout_set_font_description (layout, font_desc);

  buf[unichar_to_printable_utf8 (uc, buf)] = '\0';
  pango_layout_set_text (layout, buf, -1);

  pango_font_description_free (font_desc);

  return layout;
}


static GdkPixmap *
create_glyph_pixmap (Chartable *chartable, gint font_size)
{
  enum { PADDING = 8 };

  PangoLayout *pango_layout;
  PangoRectangle ink_rect;
  gint pixmap_width, pixmap_height;
  GtkStyle *style;
  GdkPixmap *pixmap;

  /* Apply the scaling.  Unfortunately not all fonts seem to be scalable.
   * We could fall back to GdkPixbuf scaling, but that looks butt ugly :-/
   */
  pango_layout = layout_scaled_glyph (chartable,
                                      chartable->active_char,
                                      font_size);

  pango_layout_get_pixel_extents (pango_layout, &ink_rect, NULL);

  /* Make the GdkPixmap large enough to account for possible offsets in the
   * ink extents of the glyph. */
  pixmap_width  = ink_rect.width  + 2 * PADDING;
  pixmap_height = ink_rect.height + 2 * PADDING;

  style = gtk_widget_get_style (chartable->drawing_area);

  pixmap = gdk_pixmap_new (chartable->drawing_area->window,
                           pixmap_width, pixmap_height, -1);

  gdk_draw_rectangle (pixmap, style->base_gc[GTK_STATE_NORMAL],
                      TRUE, 0, 0, pixmap_width, pixmap_height);

  /* Draw a rectangular border, taking ink_rect offsets into account. */
  gdk_draw_rectangle (pixmap, style->fg_gc[GTK_STATE_INSENSITIVE], 
                      FALSE, 1, 1, 
                      ink_rect.width  + 2 * PADDING - 3,
                      ink_rect.height + 2 * PADDING - 3);

  /* Now draw the glyph.  The coordinates are adapted
   * in order to compensate negative ink_rect offsets. */
  gdk_draw_layout (pixmap, style->text_gc[GTK_STATE_NORMAL],
                   -ink_rect.x + PADDING, -ink_rect.y + PADDING,
                   pango_layout);

  g_object_unref (pango_layout);

  return pixmap;
}


static void
get_appropriate_upper_left_xy (Chartable *chartable, 
                               gint width,  gint height,
                               gint x_root, gint y_root,
                               gint *x,     gint *y)
{
  gint row, col;

  row = (chartable->active_char - chartable->page_first_char) / chartable->cols;
  col = chartable_unichar_column (chartable, chartable->active_char);

  *x = x_root;
  *y = y_root;

  if (row >= chartable->rows / 2)
    *y -= height;

  if (col >= chartable->cols / 2)
    *x -= width;
}


/* places the zoom window toward the inside of the coordinates */
static void
place_zoom_window (Chartable *chartable, gint x_root, gint y_root)
{
  gint width, height;
  gint x, y;

  g_return_if_fail (chartable->zoom_window != NULL);

  /* This works because we always use gtk_widget_set_size_request() */
  gtk_widget_get_size_request (chartable->zoom_window, &width, &height);

  get_appropriate_upper_left_xy (chartable, width, height, 
                                 x_root, y_root, &x, &y);

  gtk_window_move (GTK_WINDOW (chartable->zoom_window), x, y);
}


static void
update_zoom_window (Chartable *chartable)
{
  gint width, height;

  g_return_if_fail (chartable->zoom_window != NULL);

  if (chartable->zoom_pixmap != NULL)
    g_object_unref (chartable->zoom_pixmap);

  chartable->zoom_pixmap = create_glyph_pixmap (
          chartable, compute_zoom_font_size (chartable));


  if (GTK_WIDGET_REALIZED (chartable->zoom_window))
    {
      set_window_background (chartable->zoom_window, chartable->zoom_pixmap);
      gdk_window_clear (chartable->zoom_window->window);
    }

  gdk_drawable_get_size (GDK_DRAWABLE (chartable->zoom_pixmap), 
                         &width, &height);

  gtk_widget_set_size_request (chartable->zoom_window, width, height);
  gtk_window_resize (GTK_WINDOW (chartable->zoom_window), width, height);
}


static void
make_zoom_window (Chartable *chartable)
{
  /* if there is already a zoom window, do nothing */
  if (chartable->zoom_window)
    return;

  chartable->zoom_window = gtk_window_new (GTK_WINDOW_TOPLEVEL);
  gtk_window_set_type_hint (GTK_WINDOW (chartable->zoom_window), 
                            GDK_WINDOW_TYPE_HINT_UTILITY);
  gtk_window_set_decorated (GTK_WINDOW (chartable->zoom_window), FALSE);

  gtk_window_set_screen (GTK_WINDOW (chartable->zoom_window),
                         gtk_widget_get_screen (chartable->drawing_area));

  /* Prevent the window from being painted with the default background. */
  gtk_widget_set_app_paintable (chartable->zoom_window, TRUE);
}


static void
destroy_zoom_window (Chartable *chartable)
{
  if (chartable->zoom_window)
    {
      GtkWidget *zoom_window;

      zoom_window = chartable->zoom_window;
      chartable->zoom_window = NULL;

      gdk_window_set_cursor (chartable->drawing_area->window, NULL);
      gtk_object_destroy (GTK_OBJECT (zoom_window));
    }
}


static GType
chartable_accessible_factory_get_accessible_type (void)
{
  return chartable_accessible_get_type ();
}


static void
chartable_accessible_factory_class_init (AtkObjectFactoryClass *clazz)
{
  clazz->create_accessible = chartable_accessible_factory_create_accessible;
  clazz->get_accessible_type = chartable_accessible_factory_get_accessible_type;
}


static GType
chartable_accessible_factory_get_type (void)
{
  static GType t = 0;

  if (!t)
    {
      static const GTypeInfo tinfo =
      {
        sizeof (AtkObjectFactoryClass),
        NULL,
        NULL,
        (GClassInitFunc) chartable_accessible_factory_class_init,
        NULL,
        NULL,
        sizeof (AtkObjectClass),
        0,
        NULL,
        NULL
      };
      t = g_type_register_static (ATK_TYPE_OBJECT_FACTORY, 
                                  "ChartableAccessibleFactory",
                                  &tinfo, 0);
    }
  return t;
}


/* XXX: the logic this function uses to set page_first_char is hideous and
 * probably wrong */
static void
set_active_char (Chartable *chartable, gunichar uc)
{
  gint offset;

  g_return_if_fail (uc >= 0 && uc <= UNICHAR_MAX);

  chartable->old_active_char = chartable->active_char;
  chartable->old_page_first_char = chartable->page_first_char;

  chartable->active_char = uc;

  /* update page, if necessary */
  if (uc - chartable->page_first_char >= chartable->rows * chartable->cols)
    {
      /* move the page_first_char as far as active_char has moved */
      offset = (gint) chartable->active_char 
               - (gint) chartable->old_active_char;
    
      if ((gint) chartable->old_page_first_char + offset >= 0)
        chartable->page_first_char = chartable->old_page_first_char + offset;
      else
        chartable->page_first_char = 0;
    
      /* round down so that it's a multiple of chartable->cols */
      chartable->page_first_char 
          -= (chartable->page_first_char % chartable->cols);
    
      /* go back up if we should have rounded up */
      if (chartable->active_char - chartable->page_first_char 
          >= chartable->rows * chartable->cols)
        chartable->page_first_char += chartable->cols;
    }

  g_signal_emit (chartable, chartable_signals[SET_ACTIVE_CHAR], 
                 0, chartable->active_char);
}


static void
draw_borders (Chartable *chartable)
{
  gint x, y, col, row;

  /* dark_gc[GTK_STATE_NORMAL] seems to be what is used to draw the borders
   * around widgets, so we use it for the lines */

  /* vertical lines */
  gdk_draw_line (chartable->pixmap,
                 chartable->drawing_area->style->dark_gc[GTK_STATE_NORMAL], 
                 0, 0, 0, chartable->drawing_area->allocation.height - 1);
  for (col = 0, x = 0;  col < chartable->cols;  col++)
    {
      x += chartable_column_width (chartable, col);
      gdk_draw_line (chartable->pixmap,
                     chartable->drawing_area->style->dark_gc[GTK_STATE_NORMAL], 
                     x, 0, x, chartable->drawing_area->allocation.height - 1);
    }

  /* horizontal lines */
  gdk_draw_line (chartable->pixmap,
                 chartable->drawing_area->style->dark_gc[GTK_STATE_NORMAL], 
                 0, 0, chartable->drawing_area->allocation.width - 1, 0);
  for (row = 0, y = 0;  row < chartable->rows;  row++)
    {
      y += chartable_row_height (chartable, row);
      gdk_draw_line (chartable->pixmap,
                     chartable->drawing_area->style->dark_gc[GTK_STATE_NORMAL], 
                     0, y, chartable->drawing_area->allocation.width - 1, y);
    }
}


static void
set_scrollbar_adjustment (Chartable *chartable)
{
  /* block our "value_changed" handler */
  g_signal_handler_block (G_OBJECT (chartable->adjustment),
                          chartable->adjustment_changed_handler_id);

  gtk_adjustment_set_value (GTK_ADJUSTMENT (chartable->adjustment), 
                            1.0 * chartable->page_first_char / chartable->cols);

  g_signal_handler_unblock (G_OBJECT (chartable->adjustment),
                            chartable->adjustment_changed_handler_id);
}


static void
draw_character (Chartable *chartable, gint row, gint col)
{
  gint padding_x, padding_y;
  gint char_width, char_height;
  gint square_width, square_height; 
  gunichar uc;
  GdkGC *gc;
  gchar buf[10];
  gint n;

  uc = rowcol_to_unichar (chartable, row, col);

  if (uc < 0 || uc > UNICHAR_MAX || ! unichar_validate (uc) 
      || ! unichar_isdefined (uc))
    return;

  if (GTK_WIDGET_HAS_FOCUS (chartable->drawing_area) 
      && uc == chartable->active_char)
    gc = chartable->drawing_area->style->text_gc[GTK_STATE_SELECTED];
  else if (uc == chartable->active_char)
    gc = chartable->drawing_area->style->text_gc[GTK_STATE_ACTIVE];
  else
    gc = chartable->drawing_area->style->text_gc[GTK_STATE_NORMAL];

  square_width = chartable_column_width (chartable, col) - 1;
  square_height = chartable_row_height (chartable, row) - 1;

  n = unichar_to_printable_utf8 (uc, buf); 
  pango_layout_set_text (chartable->pango_layout, buf, n);

  pango_layout_get_pixel_size (chartable->pango_layout, 
                               &char_width, &char_height);

  /* (square_width - char_width)/2 is the smaller half */
  padding_x = (square_width - char_width) - (square_width - char_width)/2;
  padding_y = (square_height - char_height) - (square_height - char_height)/2;

  gdk_draw_layout (chartable->pixmap, gc,
                   chartable_x_offset (chartable, col) + padding_x,
                   chartable_y_offset (chartable, row) + padding_y,
                   chartable->pango_layout);
}


static void
draw_square_bg (Chartable *chartable, gint row, gint col)
{
  gint square_width, square_height; 
  gunichar uc;
  GdkGC *gc;

  uc = rowcol_to_unichar (chartable, row, col);

  if (GTK_WIDGET_HAS_FOCUS (chartable->drawing_area) 
      && uc == chartable->active_char)
    gc = chartable->drawing_area->style->base_gc[GTK_STATE_SELECTED];
  else if (uc == chartable->active_char)
    gc = chartable->drawing_area->style->base_gc[GTK_STATE_ACTIVE];
  else if (! unichar_validate (uc))
    gc = chartable->drawing_area->style->fg_gc[GTK_STATE_INSENSITIVE];
  else if (! unichar_isdefined (uc))
    gc = chartable->drawing_area->style->bg_gc[GTK_STATE_INSENSITIVE];
  else 
    gc = chartable->drawing_area->style->base_gc[GTK_STATE_NORMAL];

  square_width = chartable_column_width (chartable, col) - 1;
  square_height = chartable_row_height (chartable, row) - 1;

  gdk_draw_rectangle (chartable->pixmap, gc, TRUE, 
                      chartable_x_offset (chartable, col), 
		      chartable_y_offset (chartable, row),
                      square_width, square_height);
}


static void
expose_square (Chartable *chartable, gint row, gint col)
{
  gtk_widget_queue_draw_area (chartable->drawing_area, 
                              chartable_x_offset (chartable, col),
                              chartable_y_offset (chartable, row),
                              chartable_column_width (chartable, col) - 1,
                              chartable_row_height (chartable, row) - 1);
}


static void
draw_square (Chartable *chartable, gint row, gint col)
{
  draw_square_bg (chartable, row, col);
  draw_character (chartable, row, col);
}


static void
draw_and_expose_character_square (Chartable *chartable, gunichar uc)
{
  gint row = (uc - chartable->page_first_char) / chartable->cols;
  gint col = chartable_unichar_column (chartable, uc);

  if (row >= 0 && row < chartable->rows && col >= 0 && col < chartable->cols)
    {
      draw_square (chartable, row, col);
      expose_square (chartable, row, col);
    }
}


/* draws the backing store pixmap */
static void
draw_chartable_from_scratch (Chartable *chartable)
{
  gint row, col;

  /* plain background */
  gdk_draw_rectangle (
          chartable->pixmap,
          chartable->drawing_area->style->base_gc[GTK_STATE_NORMAL], 
          TRUE, 0, 0, 
          chartable->drawing_area->allocation.width, 
          chartable->drawing_area->allocation.height);
  draw_borders (chartable);

  /* draw the characters */
  for (row = 0;  row < chartable->rows;  row++)
    for (col = 0;  col < chartable->cols;  col++)
      {
        gunichar uc = rowcol_to_unichar (chartable, row, col);

        /* don't need to draw the plain background, only the other ones */
        if (uc == chartable->active_char
            || ! unichar_validate (uc)
            || ! unichar_isdefined (uc))
          draw_square_bg (chartable, row, col);

        draw_character (chartable, row, col);
      }
}


static void
copy_rows (Chartable *chartable, gint row_offset)
{
  gint num_padded_rows;
  gint from_row, to_row;

  num_padded_rows = chartable->drawing_area->allocation.height -
                    (minimal_row_height (chartable) * chartable->rows + 1);

  if (ABS (row_offset) < chartable->rows - num_padded_rows)
    {
      gint num_rows, height;

      if (row_offset > 0)
        {
          from_row = row_offset;
          to_row = 0;
          num_rows = chartable->rows - num_padded_rows - from_row;
        }
      else
        {
          from_row = 0;
          to_row = -row_offset;
          num_rows = chartable->rows - num_padded_rows - to_row;
        }

      height = chartable_y_offset (chartable, num_rows) 
               - chartable_y_offset (chartable, 0) - 1;

      gdk_draw_drawable (
              chartable->pixmap,
              chartable->drawing_area->style->base_gc[GTK_STATE_NORMAL], 
              chartable->pixmap, 
              0, chartable_y_offset (chartable, from_row), 
              0, chartable_y_offset (chartable, to_row),
              chartable->drawing_area->allocation.width, height);
    }

  if (ABS (row_offset) < num_padded_rows)
    {
      /* don't need num_rows or height, cuz we can go off the end */
      if (row_offset > 0)
        {
          from_row = chartable->rows - num_padded_rows + row_offset;
          to_row = chartable->rows - num_padded_rows;
        }
      else
        {
          from_row = chartable->rows - num_padded_rows;
          to_row = chartable->rows - num_padded_rows - row_offset;
        }

      /* it's ok to go off the end (so use allocation.height) */
      gdk_draw_drawable (
              chartable->pixmap,
              chartable->drawing_area->style->base_gc[GTK_STATE_NORMAL], 
              chartable->pixmap, 
              0, chartable_y_offset (chartable, from_row), 
              0, chartable_y_offset (chartable, to_row),
              chartable->drawing_area->allocation.width, 
              chartable->drawing_area->allocation.height);
    }
}


static void
redraw_rows (Chartable *chartable, gint row_offset)
{
  gint row, col, start_row, end_row;

  if (row_offset > 0) 
    {
      start_row = chartable->rows - row_offset;
      end_row = chartable->rows - 1;
    }
  else
    {
      start_row = 0;
      end_row = -row_offset - 1;
    }

  for (row = 0;  row <= chartable->rows;  row++)
    {
      gboolean draw_row = FALSE;

      draw_row = draw_row || (row >= start_row && row <= end_row);

      if (row + row_offset >= 0 && row + row_offset <= chartable->rows)
        {
          draw_row = draw_row || (chartable_row_height (chartable, row) 
                                  != chartable_row_height (chartable,  
                                                         row + row_offset));
        }

      if (draw_row)
        {
          for (col = 0;  col < chartable->cols;  col++)
            draw_square (chartable, row, col);
        }
    }
}


static void
get_root_coords_at_active_char (Chartable *chartable, 
                                gint *x_root, gint *y_root)
{
  gint x, y;
  gint row, col;

  gdk_window_get_origin (chartable->drawing_area->window, &x, &y);

  row = (chartable->active_char - chartable->page_first_char) / chartable->cols;
  col = chartable_unichar_column (chartable, chartable->active_char);

  *x_root = x + chartable_x_offset (chartable, col);
  *y_root = y + chartable_y_offset (chartable, row);
}


/* retunrs the coords of the innermost corner of the square */
static void
get_appropriate_active_char_corner_xy (Chartable *chartable, gint *x, gint *y)
{
  gint x0, y0;
  gint row, col;

  get_root_coords_at_active_char (chartable, &x0, &y0);

  row = (chartable->active_char - chartable->page_first_char) / chartable->cols;
  col = chartable_unichar_column (chartable, chartable->active_char);

  *x = x0;
  *y = y0;

  if (row < chartable->rows / 2)
    *y += chartable_row_height (chartable, row);

  if (col < chartable->cols / 2)
    *x += chartable_column_width (chartable, col);
}


/* Redraws whatever needs to be redrawn, in the character table and caption
 * and everything, and exposes what needs to be exposed. */
void
chartable_redraw (Chartable *chartable, gboolean move_zoom)
{
  gint row_offset;
  gboolean actives_done = FALSE;

  row_offset = ((gint) chartable->page_first_char 
                - (gint) chartable->old_page_first_char)
               / chartable->cols;

#ifdef G_PLATFORM_WIN32

  if (row_offset != 0)
    {
      /* get around the bug in gdkdrawable-win32.c */
      /* yup, this makes it really slow */
      draw_chartable_from_scratch (chartable);
      gtk_widget_queue_draw (chartable->drawing_area);
      actives_done = TRUE;
    }

#else /* #ifdef G_PLATFORM_WIN32 */

  if (row_offset >= chartable->rows || row_offset <= -chartable->rows)
    {
      draw_chartable_from_scratch (chartable);
      gtk_widget_queue_draw (chartable->drawing_area);
      actives_done = TRUE;
    }
  else if (row_offset != 0)
    {
      copy_rows (chartable, row_offset);
      redraw_rows (chartable, row_offset);
      draw_borders (chartable);
      gtk_widget_queue_draw (chartable->drawing_area);
    }

#endif /* #else (#ifdef G_PLATFORM_WIN32) */

  if (chartable->active_char != chartable->old_active_char)
    {
      set_scrollbar_adjustment (chartable); /* XXX */

      if (!actives_done)
        {
          draw_and_expose_character_square (chartable, 
                                            chartable->old_active_char);
          draw_and_expose_character_square (chartable, 
                                            chartable->active_char);
        }

      if (chartable->zoom_window)
        update_zoom_window (chartable);

      if (move_zoom && chartable->zoom_window)
        {
          gint x, y;

          get_appropriate_active_char_corner_xy (chartable, &x, &y);
          place_zoom_window (chartable, x, y);
        }
    }

  chartable->old_page_first_char = chartable->page_first_char;
  chartable->old_active_char = chartable->active_char;
}


/* redraws the screen from the backing pixmap */
static gint
expose_event (GtkWidget *widget, 
              GdkEventExpose *event, 
              Chartable *chartable)
{
  gdk_window_set_back_pixmap (widget->window, NULL, FALSE);

  if (chartable->pixmap == NULL)
    {
      chartable->pixmap = gdk_pixmap_new (
              chartable->drawing_area->window, 
              chartable->drawing_area->allocation.width,
              chartable->drawing_area->allocation.height, -1);

      draw_chartable_from_scratch (chartable);

      /* the zoom window may need to be redrawn and repositioned */
      if (chartable->zoom_window)
        {
          gint x, y;

          update_zoom_window (chartable);
          get_appropriate_active_char_corner_xy (chartable, &x, &y);
          place_zoom_window (chartable, x, y);
        }
    }

  gdk_draw_drawable (chartable->drawing_area->window,
                     widget->style->fg_gc[GTK_STATE_NORMAL],
                     chartable->pixmap,
                     event->area.x, event->area.y,
                     event->area.x, event->area.y,
                     event->area.width, event->area.height);

  return FALSE;
}


void
chartable_class_init (ChartableClass *clazz)
{
  clazz->activate = NULL;
  clazz->set_active_char = NULL;

  chartable_signals[ACTIVATE] =
      g_signal_new ("activate", chartable_get_type (), G_SIGNAL_RUN_FIRST,
                    G_STRUCT_OFFSET (ChartableClass, activate),
                    NULL, NULL, gucharmap_marshal_VOID__UINT, G_TYPE_NONE, 
		    1, G_TYPE_UINT);

  chartable_signals[SET_ACTIVE_CHAR] =
      g_signal_new ("set_active_char", chartable_get_type (), 
                    G_SIGNAL_RUN_FIRST,
                    G_STRUCT_OFFSET (ChartableClass, set_active_char),
                    NULL, NULL, gucharmap_marshal_VOID__UINT, G_TYPE_NONE, 
		    1, G_TYPE_UINT);

  chartable_signals[STATUS_MESSAGE] =
      g_signal_new ("status-message", chartable_get_type (), G_SIGNAL_RUN_FIRST,
                    G_STRUCT_OFFSET (ChartableClass, status_message),
                    NULL, NULL, gucharmap_marshal_VOID__STRING, G_TYPE_NONE, 
		    1, G_TYPE_STRING);
}


static void
size_allocate (GtkWidget *widget, 
               GtkAllocation *allocation, 
               Chartable *chartable)
{
  gint old_rows, old_cols;
  GtkAdjustment *adjustment;

  old_rows = chartable->rows;
  old_cols = chartable->cols;

  chartable->cols = (allocation->width - 1) / bare_minimal_column_width (chartable);
  chartable->rows = (allocation->height - 1) / bare_minimal_row_height (chartable);

  /* avoid a horrible floating point exception crash */
  if (chartable->rows < 1)
    chartable->rows = 1;
  if (chartable->cols < 1)
    chartable->cols = 1;

  /* force pixmap to be redrawn on next expose event */
  if (chartable->pixmap != NULL)
    g_object_unref (chartable->pixmap);
  chartable->pixmap = NULL;

  if (chartable->rows == old_rows && chartable->cols == old_cols)
    return;

  chartable->page_first_char = chartable->active_char 
                             - (chartable->active_char % chartable->cols);

  /* adjust the adjustment, since it's based on the size of a row */
  adjustment = GTK_ADJUSTMENT (chartable->adjustment);
  adjustment->upper = 1.0 * UNICHAR_MAX / chartable->cols;
  adjustment->page_increment = 3.0 * chartable->rows;
  gtk_adjustment_changed (adjustment);
  set_scrollbar_adjustment (chartable);
}


static void
move_home (Chartable *chartable)
{
  set_active_char (chartable, 0x0000);
}

static void
move_end (Chartable *chartable)
{
  set_active_char (chartable, UNICHAR_MAX);
}

static void
move_up (Chartable *chartable)
{
  if (chartable->active_char >= chartable->cols)
    set_active_char (chartable, chartable->active_char - chartable->cols);
}

static void
move_down (Chartable *chartable)
{
  if (chartable->active_char <= UNICHAR_MAX - chartable->cols)
    set_active_char (chartable, chartable->active_char + chartable->cols);
}


static void
move_cursor (Chartable *chartable, gint offset)
{
  if (chartable->active_char + offset >= 0
      && chartable->active_char + offset <= UNICHAR_MAX)
    set_active_char (chartable, chartable->active_char + offset);
}

static void
move_left (Chartable *chartable)
{
  if (gtk_widget_get_direction (chartable->drawing_area) == GTK_TEXT_DIR_RTL)
    move_cursor (chartable, 1);
  else
    move_cursor (chartable, -1);
}

static void
move_right (Chartable *chartable)
{
  if (gtk_widget_get_direction (chartable->drawing_area) == GTK_TEXT_DIR_RTL)
    move_cursor (chartable, -1);
  else
    move_cursor (chartable, 1);
}

static void
move_page_up (Chartable *chartable)
{
  if (chartable->active_char >= chartable->cols * chartable->rows)
    {
      set_active_char (
              chartable, 
              chartable->active_char - chartable->cols * chartable->rows);
    }
  else if (chartable->active_char > 0)
    set_active_char (chartable, 0);
}

static void
move_page_down (Chartable *chartable)
{
  if (chartable->active_char < UNICHAR_MAX - chartable->cols * chartable->rows)
    {
      set_active_char (
              chartable, 
              chartable->active_char + chartable->cols * chartable->rows);
    }
  else if (chartable->active_char < UNICHAR_MAX)
    set_active_char (chartable, UNICHAR_MAX);
}


/* mostly for moving around in the chartable */
static gint
key_press_event (GtkWidget *widget, 
                 GdkEventKey *event, 
                 Chartable *chartable)
{
  /* move the cursor or whatever depending on which key was pressed */
  switch (event->keyval)
    {
      case GDK_Home: case GDK_KP_Home:
        move_home (chartable);
        break;

      case GDK_End: case GDK_KP_End:
        move_end (chartable);
        break;

      case GDK_Up: case GDK_KP_Up: case GDK_k:
        move_up (chartable);
        break;

      case GDK_Down: case GDK_KP_Down: case GDK_j:
        move_down (chartable);
        break;

      case GDK_Left: case GDK_KP_Left: case GDK_h:
        move_left (chartable);
        break;

      case GDK_Right: case GDK_KP_Right: case GDK_l:
        move_right (chartable);
        break;

      case GDK_Page_Up: case GDK_KP_Page_Up: case GDK_b:
        move_page_up (chartable);
        break;

      case GDK_Page_Down: case GDK_KP_Page_Down:
        move_page_down (chartable);
        break;

      case GDK_Return: case GDK_KP_Enter: case GDK_space:
	g_signal_emit (chartable, chartable_signals[ACTIVATE], 0, 
		       chartable->active_char);
        return TRUE;

      /* pass on other keys, like tab and stuff that shifts focus */
      default:
        return FALSE;
    }

  chartable_redraw (chartable, TRUE);

  return TRUE;
}


static void
set_top_row (Chartable *chartable, gint row)
{
  gint r, c;

  g_return_if_fail (row >= 0 && row <= UNICHAR_MAX / chartable->cols);

  chartable->old_page_first_char = chartable->page_first_char;
  chartable->old_active_char = chartable->active_char;

  chartable->page_first_char = row * chartable->cols;

  /* character is still on the visible page */
  if (chartable->active_char - chartable->page_first_char 
      < chartable->rows * chartable->cols)
    return;

  c = chartable->old_active_char % chartable->cols;

  if (chartable->page_first_char < chartable->old_page_first_char)
    r = chartable->rows - 1;
  else
    r = 0;

  chartable->active_char = chartable->page_first_char + r * chartable->cols + c;
  if (chartable->active_char > UNICHAR_MAX)
    chartable->active_char = UNICHAR_MAX;

  g_signal_emit (chartable, chartable_signals[SET_ACTIVE_CHAR], 
                 0, chartable->active_char);
}


static void
scroll_chartable (GtkAdjustment *adjustment, Chartable *chartable)
{
  set_top_row (chartable, (gint) gtk_adjustment_get_value (adjustment));
  chartable_redraw (chartable, TRUE);
}


/* for mouse clicks */
static gunichar
get_char_at (Chartable *chartable, gint x, gint y)
{
  gint r, c, x0, y0;
  gunichar rv;

  for (c = 0, x0 = 0;  x0 <= x && c < chartable->cols;  c++)
    x0 += chartable_column_width (chartable, c);

  for (r = 0, y0 = 0;  y0 <= y && r < chartable->rows;  r++)
    y0 += chartable_row_height (chartable, r);

  rv = rowcol_to_unichar (chartable, r-1, c-1);

  /* XXX: check this somewhere else? */
  if (rv > UNICHAR_MAX)
    return UNICHAR_MAX;

  return rv;
}


static void
status_message (Chartable *chartable, const gchar *message)
{
  g_signal_emit (chartable, chartable_signals[STATUS_MESSAGE], 0, message);
}


static void
selection_text_received (GtkClipboard *clipboard, 
                         const gchar *text,
                         Chartable *chartable)
{
  gunichar uc;

  if (text == NULL)
    {
      if (clipboard == gtk_clipboard_get (GDK_SELECTION_CLIPBOARD))
        status_message (chartable, _("Clipboard is empty."));
      else
        status_message (chartable, _("There is no selected text."));
      return;
    }

  uc = g_utf8_get_char_validated (text, -1);

  if (uc == (gunichar)(-2) || uc == (gunichar)(-1) || uc > UNICHAR_MAX)
    {
      status_message (chartable, _("Unknown character, unable to identify."));
    }
  else
    {
      status_message (chartable, _("Character found."));
      set_active_char (chartable, uc);
      chartable_redraw (chartable, TRUE);
    }
}


void
chartable_identify_clipboard (Chartable *chartable, GtkClipboard *clipboard)
{
  gtk_clipboard_request_text (
          clipboard, (GtkClipboardTextReceivedFunc) selection_text_received, 
          chartable);
}


/*  - single click with left button: activate character under pointer
 *  - double-click with left button: add active character to text_to_copy
 *  - single-click with middle button: jump to selection_primary
 */
static gint
button_press_event (GtkWidget *widget, 
                    GdkEventButton *event, 
                    Chartable *chartable)
{
  /* in case we lost keyboard focus and are clicking to get it back */
  gtk_widget_grab_focus (chartable->drawing_area);

  /* double-click */
  if (event->button == 1 && event->type == GDK_2BUTTON_PRESS)
    {
      g_signal_emit (chartable, chartable_signals[ACTIVATE], 
                     0, chartable->active_char);
    }
  /* single-click */ 
  else if (event->button == 1 && event->type == GDK_BUTTON_PRESS) 
    {
      set_active_char (chartable, get_char_at (chartable, event->x, event->y));
      chartable_redraw (chartable, TRUE);
    }
  else if (event->button == 2)
    {
      chartable_identify_clipboard (chartable, 
                                    gtk_clipboard_get (GDK_SELECTION_PRIMARY));
    }
  else if (event->button == 3)
    {
      set_active_char (
              chartable, get_char_at (chartable, event->x, event->y));

      make_zoom_window (chartable);
      chartable_redraw (chartable, FALSE);

      if (chartable->active_char == chartable->old_active_char)
        update_zoom_window (chartable); 

      place_zoom_window (chartable, event->x_root, event->y_root);
      gtk_widget_show (chartable->zoom_window);

      /* must do this after gtk_widget_show */
      set_window_background (chartable->zoom_window, chartable->zoom_pixmap);
      gdk_window_clear (chartable->zoom_window->window);
    }

  /* need to return false so it gets drag events */
  return FALSE;
}


static gint
button_release_event (GtkWidget *widget, 
                      GdkEventButton *event, 
                      Chartable *chartable)
{
  if (!chartable->zoom_mode_enabled && event->button == 3)
    destroy_zoom_window (chartable);

  return FALSE;
}


static GtkWidget *
make_scrollbar (Chartable *chartable)
{
  chartable->adjustment = gtk_adjustment_new (
          0.0, 0.0, 1.0 * UNICHAR_MAX / chartable->cols, 
          2.0, 3.0 * chartable->rows, 0.0);

  chartable->adjustment_changed_handler_id = g_signal_connect (
          G_OBJECT (chartable->adjustment), "value-changed",
          G_CALLBACK (scroll_chartable), chartable);

  return gtk_vscrollbar_new (GTK_ADJUSTMENT (chartable->adjustment));
}


static gint
motion_notify_event (GtkWidget *widget, 
                     GdkEventMotion *event, 
                     Chartable *chartable)
{
  if ((event->state & GDK_BUTTON3_MASK) != 0 && chartable->zoom_window)
    {
      gunichar uc;

      uc = get_char_at (chartable, MAX (0, event->x), MAX (0, event->y));

      if (uc != chartable->active_char)
        {
          set_active_char (chartable, uc);
          chartable_redraw (chartable, FALSE);
        }

      place_zoom_window (chartable, event->x_root, event->y_root);
    }

  return FALSE;
}


static gboolean
focus_in_or_out_event (GtkWidget *widget, 
                       GdkEventFocus *event,
                       Chartable *chartable)
{
  if (chartable->drawing_area != NULL && chartable->pixmap != NULL)
    draw_and_expose_character_square (chartable, chartable->active_char);

  return FALSE;
}


static void
mouse_wheel_up (Chartable *chartable)
{
  if (chartable->page_first_char > chartable->rows * chartable->cols / 2)
    set_top_row (chartable, (chartable->page_first_char 
                             - chartable->rows * chartable->cols / 2) 
                            / chartable->cols);
  else 
    set_top_row (chartable, 0);

  chartable_redraw (chartable, TRUE);
}


static void
mouse_wheel_down (Chartable *chartable)
{
  if (chartable->page_first_char 
          < UNICHAR_MAX - chartable->rows * chartable->cols / 2)
    {
      set_top_row (chartable, (chartable->page_first_char
                               + chartable->rows * chartable->cols / 2) 
                              / chartable->cols);
    }
  else 
    {
      set_top_row (chartable, UNICHAR_MAX / chartable->cols);
    }

  chartable_redraw (chartable, TRUE);
}


/* mouse wheel scrolls by half a page */
static gboolean    
mouse_wheel_event (GtkWidget *widget, 
                   GdkEventScroll *event, 
                   Chartable *chartable)
{
  switch (event->direction)
    {
      case GDK_SCROLL_UP:
        mouse_wheel_up (chartable);
        break;

      case GDK_SCROLL_DOWN:
        mouse_wheel_down (chartable);
        break;

      default:
        break;
    }

  return TRUE;
}


static void
style_set (GtkWidget *widget, 
           GtkStyle *previous_style, 
           Chartable *chartable)
{
  if (chartable->pixmap != NULL)
    g_object_unref (chartable->pixmap);
  chartable->pixmap = NULL;

  gtk_widget_queue_draw (chartable->drawing_area);
}


static void
drag_data_received (GtkWidget *widget,
                    GdkDragContext *context,
                    gint x, gint y,
                    GtkSelectionData *selection_data,
                    guint info,
                    guint time,
                    Chartable *chartable)
{
  gchar *text;
  gunichar uc;

  text = gtk_selection_data_get_text (selection_data);

  if (text == NULL) /* XXX: say something in the statusbar? */
    return;

  uc = g_utf8_get_char_validated (text, -1);

  if (uc == (gunichar)(-2) || uc == (gunichar)(-1) || uc > UNICHAR_MAX)
    {
      status_message (chartable, _("Unknown character, unable to identify."));
    }
  else
    {
      status_message (chartable, _("Character found."));
      set_active_char (chartable, uc);
      chartable_redraw (chartable, TRUE);
    }

  g_free (text);
}


static gint
compute_drag_font_size (Chartable *chartable)
{
  gint font_size;

  font_size = pango_font_description_get_size (
          gtk_widget_get_style (GTK_WIDGET (chartable))->font_desc);

  return 5 * ((font_size > 0) ? font_size : 10 * PANGO_SCALE);
}


static void
drag_begin (GtkWidget *widget, 
            GdkDragContext *context,
            Chartable *chartable)
{
  GdkPixmap *drag_icon;

  drag_icon = create_glyph_pixmap (chartable, 
                                   compute_drag_font_size (chartable));
  gtk_drag_set_icon_pixmap (context, gtk_widget_get_colormap (widget), 
                            drag_icon, NULL, -8, -8);
  g_object_unref (drag_icon);
}


static void
drag_data_get (GtkWidget *widget, 
               GdkDragContext *context,
               GtkSelectionData *selection_data,
               guint info,
               guint time,
               Chartable *chartable)

{
  gchar buf[7];
  gint n;

  n = g_unichar_to_utf8 (chartable->active_char, buf);
  gtk_selection_data_set_text (selection_data, buf, n);
}


/* does all the initial construction */
void
chartable_init (Chartable *chartable)
{
  PangoContext *context;
  AtkObject *accessible;

  chartable->zoom_mode_enabled = FALSE;
  chartable->zoom_window = NULL;
  chartable->zoom_pixmap = NULL;
  chartable->font_metrics = NULL;

  accessible = gtk_widget_get_accessible (GTK_WIDGET (chartable));
  atk_object_set_name (accessible, _("Character Table"));

  chartable->drawing_area = gtk_drawing_area_new ();

  gtk_widget_set_events (chartable->drawing_area,
          GDK_EXPOSURE_MASK | GDK_KEY_PRESS_MASK | GDK_BUTTON_PRESS_MASK |
          GDK_BUTTON_RELEASE_MASK | GDK_BUTTON3_MOTION_MASK |
          GDK_FOCUS_CHANGE_MASK | GDK_SCROLL_MASK);

  g_signal_connect (G_OBJECT (chartable->drawing_area), "expose_event",
                    G_CALLBACK (expose_event), chartable);
  g_signal_connect (G_OBJECT (chartable->drawing_area), "size-allocate",
                    G_CALLBACK (size_allocate), chartable);
  g_signal_connect (G_OBJECT (chartable->drawing_area), "key-press-event",
                    G_CALLBACK (key_press_event), chartable);
  g_signal_connect (G_OBJECT (chartable->drawing_area), "button-press-event",
                    G_CALLBACK (button_press_event), chartable);
  g_signal_connect (G_OBJECT (chartable->drawing_area), "button-release-event",
                    G_CALLBACK (button_release_event), chartable);
  g_signal_connect (G_OBJECT (chartable->drawing_area), "motion-notify-event",
                    G_CALLBACK (motion_notify_event), chartable);
  g_signal_connect (G_OBJECT (chartable->drawing_area), "focus-in-event",
                    G_CALLBACK (focus_in_or_out_event), chartable);
  g_signal_connect (G_OBJECT (chartable->drawing_area), "focus-out-event",
                    G_CALLBACK (focus_in_or_out_event), chartable);
  g_signal_connect (G_OBJECT (chartable->drawing_area), "scroll-event",
                    G_CALLBACK (mouse_wheel_event), chartable);
  g_signal_connect (G_OBJECT (chartable->drawing_area), "style-set",
                    G_CALLBACK (style_set), chartable);

  gtk_drag_dest_set (chartable->drawing_area, GTK_DEST_DEFAULT_ALL,
                     dnd_target_table, G_N_ELEMENTS (dnd_target_table),
                     GDK_ACTION_COPY);

  g_signal_connect (G_OBJECT (chartable->drawing_area), "drag-data-received",
                    G_CALLBACK (drag_data_received), chartable);

  gtk_drag_source_set (chartable->drawing_area, GDK_BUTTON1_MASK, 
                       dnd_target_table, G_N_ELEMENTS (dnd_target_table),
                       GDK_ACTION_COPY);

  g_signal_connect (G_OBJECT (chartable->drawing_area), "drag-begin",
                    G_CALLBACK (drag_begin), chartable);

  g_signal_connect (G_OBJECT (chartable->drawing_area), "drag-data-get",
                    G_CALLBACK (drag_data_get), chartable);

  /* this is required to get key_press events */
  GTK_WIDGET_SET_FLAGS (chartable->drawing_area, GTK_CAN_FOCUS);

  gtk_box_pack_start (GTK_BOX (chartable), chartable->drawing_area, 
                      TRUE, TRUE, 0);
  gtk_box_pack_start (GTK_BOX (chartable), make_scrollbar (chartable), 
                      FALSE, FALSE, 0);

  if (GTK_IS_ACCESSIBLE (gtk_widget_get_accessible (GTK_WIDGET (chartable))))
    {
      /* Accessibility support is enabled */
      atk_registry_set_factory_type (atk_get_default_registry (),
                                     GTK_TYPE_DRAWING_AREA,
                                     chartable_accessible_factory_get_type ());
    }

  gtk_widget_show_all (GTK_WIDGET (chartable));

  chartable->font_name = NULL;

  chartable->font_metrics = pango_context_get_metrics (
          gtk_widget_get_pango_context (chartable->drawing_area),
          chartable->drawing_area->style->font_desc, NULL);

  context = gtk_widget_get_pango_context (chartable->drawing_area);
  chartable->pango_layout = pango_layout_new (context);

  pango_layout_set_font_description (chartable->pango_layout,
                                     chartable->drawing_area->style->font_desc);

  chartable->page_first_char = (gunichar) 0x0000;
  chartable->active_char = (gunichar) 0x0000;
}


GtkWidget *
chartable_new (void)
{
  return GTK_WIDGET (g_object_new (chartable_get_type (), NULL));
}


GType
chartable_get_type (void)
{
  static GType chartable_type = 0;

  if (!chartable_type)
    {
      static const GTypeInfo chartable_info =
      {
        sizeof (ChartableClass),
        NULL,           /* base_init */
        NULL,           /* base_finalize */
        (GClassInitFunc) chartable_class_init,
        NULL,           /* class_finalize */
        NULL,           /* class_data */
        sizeof (Chartable),
        0,              /* n_preallocs */
        (GInstanceInitFunc) chartable_init,
      };

      chartable_type = g_type_register_static (GTK_TYPE_HBOX, "Chartable", 
                                               &chartable_info, 0);
    }

  return chartable_type;
}


void
chartable_zoom_enable (Chartable *chartable)
{
  gint x, y;

  chartable->zoom_mode_enabled = TRUE;

  make_zoom_window (chartable);
  update_zoom_window (chartable);

  get_appropriate_active_char_corner_xy (chartable, &x, &y);
  place_zoom_window (chartable, x, y);

  gtk_widget_show (chartable->zoom_window);

  /* must do this after gtk_widget_show */
  set_window_background (chartable->zoom_window, chartable->zoom_pixmap);
  gdk_window_clear (chartable->zoom_window->window);
}


void
chartable_zoom_disable (Chartable *chartable)
{
  chartable->zoom_mode_enabled = FALSE;
  destroy_zoom_window (chartable);
}


void 
chartable_set_font (Chartable *chartable, const gchar *font_name)
{
  PangoFontDescription *font_desc;

  /* if it's the same as the current font, do nothing */
  if (chartable->font_name != NULL
      && g_ascii_strcasecmp (chartable->font_name, font_name) == 0)
    return;
  else
    {
      g_free (chartable->font_name);
      chartable->font_name = NULL;
      chartable->font_name = g_strdup (font_name);
    }

  font_desc = pango_font_description_from_string (chartable->font_name);

  /* ensure style so that this has an effect even before it's realized */
  gtk_widget_ensure_style (chartable->drawing_area);
  gtk_widget_modify_font (chartable->drawing_area, font_desc);

  /* free the old font metrics */
  if (chartable->font_metrics != NULL)
    pango_font_metrics_unref (chartable->font_metrics);

  chartable->font_metrics = pango_context_get_metrics (
          gtk_widget_get_pango_context (chartable->drawing_area),
          chartable->drawing_area->style->font_desc, NULL);

  /* new pango layout for the new font */
  g_object_unref (chartable->pango_layout);
  chartable->pango_layout = pango_layout_new (
          gtk_widget_get_pango_context (chartable->drawing_area));

  pango_layout_set_font_description (chartable->pango_layout,
                                     chartable->drawing_area->style->font_desc);

  pango_font_description_free (font_desc);

  /* force pixmap to be redrawn on next expose event */
  if (chartable->pixmap != NULL)
    g_object_unref (chartable->pixmap);
  chartable->pixmap = NULL;
}


gunichar 
chartable_get_active_character (Chartable *chartable)
{
  return chartable->active_char;
}


void
chartable_set_active_character (Chartable *chartable, gunichar uc)
{
  set_active_char (chartable, uc);
  chartable_redraw (chartable, TRUE);
}


void
chartable_grab_focus (Chartable *chartable)
{
  gtk_widget_grab_focus (chartable->drawing_area);
}


