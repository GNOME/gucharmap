/* $Id$ */
/*
 * Copyright (c) 2003  Noah Levitt <nlevitt аt users.sourceforge.net>
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
#include <stdlib.h>
#include "charmap.h"
#include "unicode_info.h"
#include "gucharmap_intl.h"
#include "gucharmap_marshal.h"


#define abs(x) ((x) >= 0 ? (x) : (-x))
#define font_height(font_metrics) ((pango_font_metrics_get_ascent (font_metrics) + pango_font_metrics_get_descent (font_metrics)) / PANGO_SCALE)


#define set_statusbar_message(x,y) 

/* only the label is visible in the block selector */
enum 
{
  BLOCK_SELECTOR_LABEL = 0,
  BLOCK_SELECTOR_UC_START,
  BLOCK_SELECTOR_UNICODE_BLOCK,
  BLOCK_SELECTOR_NUM_COLUMNS
};

enum 
{
  CAPTION_LABEL = 0,
  CAPTION_VALUE,
  CAPTION_NUM_COLUMNS
};


enum 
{
  ACTIVATE = 0,
  STATUS_MESSAGE,
  NUM_SIGNALS
};


static guint charmap_signals[NUM_SIGNALS] = { 0, 0 };

static const GtkTargetEntry dnd_target_table[] =
  {
    { "UTF8_STRING", 0, 0 },
    { "COMPOUND_TEXT", 0, 0 },
    { "TEXT", 0, 0 },
    { "STRING", 0, 0 }
  };


/* return value is read-only, should not be freed */
static gchar *
unichar_to_printable_utf8 (gunichar uc)
{
  static gchar buf[12];
  gint x;

  /* XXX: 0x2029: workaround for pango 1.0.3 bug
   * http://bugzilla.gnome.org/show_bug.cgi?id=88824 */
  /* http://www.cl.cam.ac.uk/~mgk25/unicode.html#utf-8 --"Also note that
   * the code positions U+D800 to U+DFFF (UTF-16 surrogates) as well as
   * U+FFFE and U+FFFF must not occur in normal UTF-8" */
  if ((g_unichar_isdefined (uc) && ! g_unichar_isgraph (uc)) 
      || ! is_valid_character (uc) || uc == 0x2029)
    return "";
  
  /* Unicode Standard 3.2, section 2.6, "By convention, diacritical marks
   * used by the Unicode Standard may be exhibited in (apparent) isolation
   * by applying them to U+0020 SPACE or to U+00A0 NO BREAK SPACE." */

  /* 17:10 < owen> noah: I'm *not* claiming that what Pango does currently
   *               is right, but convention isn't a requirement. I think
   *               it's probably better to do the Uniscribe thing and put
   *               the lone combining mark on a dummy character and require
   *               ZWJ
   * 17:11 < noah> owen: do you mean that i should put a ZWJ in there, or
   *               that pango will do that?
   * 17:11 < owen> noah: I mean, you should (assuming some future more
   *               capable version of Pango) put it in there
   */

  if (g_unichar_type (uc) == G_UNICODE_COMBINING_MARK
      || g_unichar_type (uc) == G_UNICODE_ENCLOSING_MARK
      || g_unichar_type (uc) == G_UNICODE_NON_SPACING_MARK)
    {
      buf[0] = ' ';
      buf[1] = '\xe2'; /* ZERO */ 
      buf[2] = '\x80'; /* WIDTH */
      buf[3] = '\x8d'; /* JOINER (0x200D) */
      x = g_unichar_to_utf8 (uc, buf+4);
      buf[x+4] = '\0';
    }
  else
    {
      x = g_unichar_to_utf8 (uc, buf);
      buf[x] = '\0';
    }

  return buf;
}


static void
set_caption (Charmap *charmap)
{
  GtkTreeIter iter;
  GtkTreeModel *model;
  guchar ubuf[7];
  gchar *temp;
  gunichar *decomposition;
  GString *gstemp;
  gsize result_len;
  gint i, n;

  model = GTK_TREE_MODEL (charmap->caption->caption_model);

  /* name */
  gtk_tree_model_get_iter (
          model, &iter, 
          gtk_tree_row_reference_get_path (charmap->caption->name));
  gtk_tree_store_set (charmap->caption->caption_model, &iter, CAPTION_VALUE, 
                      get_unicode_name (charmap->active_char), -1);

  /* codepoint */
  gtk_tree_model_get_iter (
          model, &iter, 
          gtk_tree_row_reference_get_path (charmap->caption->codepoint));
  temp = g_strdup_printf ("U+%4.4X (%u)", 
                          charmap->active_char, charmap->active_char);
  gtk_tree_store_set (charmap->caption->caption_model, &iter, 
                      CAPTION_VALUE, temp, -1);
  g_free (temp);

  /* category */
  gtk_tree_model_get_iter (
          model, &iter, 
          gtk_tree_row_reference_get_path (charmap->caption->category));
  gtk_tree_store_set (charmap->caption->caption_model, &iter, CAPTION_VALUE, 
                      get_unicode_category_name (charmap->active_char), -1);

  /* utf-8 */
  gstemp = g_string_new (NULL);
  n = g_unichar_to_utf8 (charmap->active_char, ubuf);
  for (i = 0;  i < n;  i++)
    g_string_append_printf (gstemp, "0x%2.2X ", ubuf[i]);
  gtk_tree_model_get_iter (
          model, &iter, 
          gtk_tree_row_reference_get_path (charmap->caption->utf8));
  gtk_tree_store_set (charmap->caption->caption_model, &iter, 
                      CAPTION_VALUE, gstemp->str, -1);
  g_string_free (gstemp, TRUE);

  /* decomposition */
  decomposition = unicode_canonical_decomposition (charmap->active_char,
                                                   &result_len);
  gstemp = g_string_new (NULL);
  g_string_printf (gstemp, "%s [U+%4.4X]", 
                   unichar_to_printable_utf8 (decomposition[0]), 
                   decomposition[0]);
  for (i = 1;  i < result_len;  i++)
    g_string_append_printf (gstemp, " + %s [U+%4.4X]", 
                            unichar_to_printable_utf8 (decomposition[i]), 
                            decomposition[i]);
  gtk_tree_model_get_iter (
          model, &iter, 
          gtk_tree_row_reference_get_path (charmap->caption->decomposition));
  gtk_tree_store_set (charmap->caption->caption_model, &iter, CAPTION_VALUE, 
                      gstemp->str, -1);
  g_free (decomposition);
  g_string_free (gstemp, TRUE);

#if ENABLE_UNIHAN
  /* kDefinition */
  gtk_tree_model_get_iter (
          model, &iter, 
          gtk_tree_row_reference_get_path (charmap->caption->kDefinition));
  gtk_tree_store_set (charmap->caption->caption_model, &iter, CAPTION_VALUE, 
                      get_unicode_kDefinition (charmap->active_char), -1);

  /* kMandarin */
  gtk_tree_model_get_iter (
          model, &iter, 
          gtk_tree_row_reference_get_path (charmap->caption->kMandarin));
  gtk_tree_store_set (charmap->caption->caption_model, &iter, CAPTION_VALUE, 
                      get_unicode_kMandarin (charmap->active_char), -1);

  /* kJapaneseOn */
  gtk_tree_model_get_iter (
          model, &iter, 
          gtk_tree_row_reference_get_path (charmap->caption->kJapaneseOn));
  gtk_tree_store_set (charmap->caption->caption_model, &iter, CAPTION_VALUE, 
                      get_unicode_kJapaneseOn (charmap->active_char), -1);

  /* kJapaneseKun */
  gtk_tree_model_get_iter (
          model, &iter, 
          gtk_tree_row_reference_get_path (charmap->caption->kJapaneseKun));
  gtk_tree_store_set (charmap->caption->caption_model, &iter, CAPTION_VALUE, 
                      get_unicode_kJapaneseKun (charmap->active_char), -1);

  /* kCantonese */
  gtk_tree_model_get_iter (
          model, &iter, 
          gtk_tree_row_reference_get_path (charmap->caption->kCantonese));
  gtk_tree_store_set (charmap->caption->caption_model, &iter, CAPTION_VALUE, 
                      get_unicode_kCantonese (charmap->active_char), -1);

  /* kTang */
  gtk_tree_model_get_iter (
          model, &iter, 
          gtk_tree_row_reference_get_path (charmap->caption->kTang));
  gtk_tree_store_set (charmap->caption->caption_model, &iter, CAPTION_VALUE, 
                      get_unicode_kTang (charmap->active_char), -1);

  /* kKorean */
  gtk_tree_model_get_iter (
          model, &iter, 
          gtk_tree_row_reference_get_path (charmap->caption->kKorean));
  gtk_tree_store_set (charmap->caption->caption_model, &iter, CAPTION_VALUE, 
                      get_unicode_kKorean (charmap->active_char), -1);
#endif /* #if ENABLE_UNIHAN */
}


/* XXX: linear search (but N is small) */
static GtkTreePath *
find_block_index_tree_path (Charmap *charmap, gunichar uc)
{
  gint i;

  for (i = 0;  i < charmap->block_index_size;  i++)
    if (charmap->block_index[i].start > uc)
      break;

  return charmap->block_index[i-1].tree_path;
}


/* selects the active block in the block selector tree view based on the
 * active character */
static void
set_active_block (Charmap *charmap)
{
  GtkTreePath *parent = NULL;
  GtkTreePath *tree_path;
  
  tree_path = find_block_index_tree_path (charmap, charmap->active_char);

  /* block our "changed" handler */
  g_signal_handler_block (G_OBJECT (charmap->block_selection), 
                          charmap->block_selection_changed_handler_id);

  if (gtk_tree_path_get_depth (tree_path) == 2)
    {
      parent = gtk_tree_path_copy (tree_path);
      gtk_tree_path_up (parent);

      if (! gtk_tree_view_row_expanded (
              GTK_TREE_VIEW (charmap->block_selector_view), parent))
        tree_path = parent;
    }

  gtk_tree_view_set_cursor (GTK_TREE_VIEW (charmap->block_selector_view),
                            tree_path, NULL, FALSE);

  g_signal_handler_unblock (G_OBJECT (charmap->block_selection),
                            charmap->block_selection_changed_handler_id);

  if (parent != NULL)
    gtk_tree_path_free (parent);
}


/* computes the column width based solely on the font size */
static gint
bare_minimal_column_width (Charmap *charmap)
{
  /* XXX: width is not available, so use height */
  return font_height (charmap->font_metrics) + 7;
}


static gint
minimal_column_width (Charmap *charmap)
{
  gint total_extra_pixels;
  gint bare_minimal_width = bare_minimal_column_width (charmap);

  total_extra_pixels = charmap->chartable->allocation.width 
                       - (charmap->cols * bare_minimal_width + 1);

  return bare_minimal_width + total_extra_pixels / charmap->cols;
}


/* not all columns are necessarily the same width because of padding */
static gint
column_width (Charmap *charmap, gint col)
{
  gint num_padded_columns;
  gint min_col_w = minimal_column_width (charmap);

  num_padded_columns = charmap->chartable->allocation.width 
                       - (min_col_w * charmap->cols + 1);

  if (charmap->cols - col <= num_padded_columns)
    return min_col_w + 1;
  else
    return min_col_w;
}


/* calculates the position of the left end of the column (just to the right
 * of the left border) */
/* XXX: calling this repeatedly is not the most efficient, but it probably
 * is the most readable */
static gint
x_offset (Charmap *charmap, gint col)
{
  gint c, x;

  for (c = 0, x = 1;  c < col;  c++)
    x += column_width (charmap, c);

  return x;
}


/* computes the row height based solely on the font size */
static gint
bare_minimal_row_height (Charmap *charmap)
{
  return font_height (charmap->font_metrics) + 7;
}


static gint
minimal_row_height (Charmap *charmap)
{
  gint total_extra_pixels;
  gint bare_minimal_height = bare_minimal_row_height (charmap);

  total_extra_pixels = charmap->chartable->allocation.height 
                       - (charmap->rows * bare_minimal_height + 1);

  return bare_minimal_height + total_extra_pixels / charmap->rows;
}


/* not all rows are necessarily the same height because of padding */
static gint
row_height (Charmap *charmap, gint row)
{
  gint num_padded_rows;
  gint min_row_h = minimal_row_height (charmap);

  num_padded_rows = charmap->chartable->allocation.height -
                    (min_row_h * charmap->rows + 1);

  if (charmap->rows - row <= num_padded_rows)
    return min_row_h + 1;
  else
    return min_row_h;
}


/* calculates the position of the top end of the row (just below the top
 * border) */
/* XXX: calling this repeatedly is not the most efficient, but it probably
 * is the most readable */
static gint
y_offset (Charmap *charmap, gint row)
{
  gint r, y;

  for (r = 0, y = 1;  r < row;  r++)
    y += row_height (charmap, r);

  return y;
}


static void
set_scrollbar_adjustment (Charmap *charmap)
{
  /* block our "value_changed" handler */
  g_signal_handler_block (G_OBJECT (charmap->adjustment),
                          charmap->adjustment_changed_handler_id);

  gtk_adjustment_set_value (GTK_ADJUSTMENT (charmap->adjustment), 
                            1.0 * charmap->page_first_char / charmap->cols);

  g_signal_handler_unblock (G_OBJECT (charmap->adjustment),
                            charmap->adjustment_changed_handler_id);
}


/* depends on directionality */
static gunichar
rowcol_to_unichar (Charmap *charmap, gint row, gint col)
{
  if (gtk_widget_get_direction (charmap->chartable) == GTK_TEXT_DIR_RTL)
    return charmap->page_first_char + row * charmap->cols 
           + (charmap->cols - col - 1);
  else
    return charmap->page_first_char + row * charmap->cols + col;
}


/* depends on directionality */
static gint
unichar_column (Charmap *charmap, gunichar uc)
{
  if (gtk_widget_get_direction (charmap->chartable) == GTK_TEXT_DIR_RTL)
    return charmap->cols - (uc - charmap->page_first_char) % charmap->cols - 1;
  else
    return (uc - charmap->page_first_char) % charmap->cols;
}


static void
draw_character (Charmap *charmap, gint row, gint col)
{
  gint padding_x, padding_y;
  gint char_width, char_height;
  gint square_width, square_height; 
  gunichar uc;
  GdkGC *gc;

  uc = rowcol_to_unichar (charmap, row, col);

  if (uc < 0 || uc > UNICHAR_MAX)
    return;

  if (GTK_WIDGET_HAS_FOCUS (charmap->chartable) && uc == charmap->active_char)
    gc = charmap->chartable->style->text_gc[GTK_STATE_SELECTED];
  else if (uc == charmap->active_char)
    gc = charmap->chartable->style->text_gc[GTK_STATE_ACTIVE];
  else
    gc = charmap->chartable->style->text_gc[GTK_STATE_NORMAL];

  square_width = column_width (charmap, col) - 1;
  square_height = row_height (charmap, row) - 1;

  pango_layout_set_text (charmap->pango_layout, 
                         unichar_to_printable_utf8 (uc), 
                         -1);

  pango_layout_get_pixel_size (charmap->pango_layout, 
                               &char_width, &char_height);

  /* (square_width - char_width)/2 is the smaller half */
  padding_x = (square_width - char_width) - (square_width - char_width)/2;
  padding_y = (square_height - char_height) - (square_height - char_height)/2;

  gdk_draw_layout (charmap->chartable_pixmap, gc,
                   x_offset (charmap, col) + padding_x,
                   y_offset (charmap, row) + padding_y,
                   charmap->pango_layout);
}


static void
draw_square_bg (Charmap *charmap, gint row, gint col)
{
  gint square_width, square_height; 
  gunichar uc;
  GdkGC *gc;

  uc = rowcol_to_unichar (charmap, row, col);

  if (GTK_WIDGET_HAS_FOCUS (charmap->chartable) && uc == charmap->active_char)
    gc = charmap->chartable->style->base_gc[GTK_STATE_SELECTED];
  else if (uc == charmap->active_char)
    gc = charmap->chartable->style->base_gc[GTK_STATE_ACTIVE];
  else
    gc = charmap->chartable->style->base_gc[GTK_STATE_NORMAL];

  square_width = column_width (charmap, col) - 1;
  square_height = row_height (charmap, row) - 1;

  gdk_draw_rectangle (charmap->chartable_pixmap, gc, TRUE, 
                      x_offset (charmap, col), y_offset (charmap, row),
                      square_width, square_height);
}


static void
expose_square (Charmap *charmap, gint row, gint col)
{
  gtk_widget_queue_draw_area (charmap->chartable, 
                              x_offset (charmap, col),
                              y_offset (charmap, row),
                              column_width (charmap, col) - 1,
                              row_height (charmap, row) - 1);
}


static void
draw_square (Charmap *charmap, gint row, gint col)
{
  draw_square_bg (charmap, row, col);
  draw_character (charmap, row, col);
}


static void
draw_borders (Charmap *charmap)
{
  gint x, y, col, row;

  /* vertical lines */
  gdk_draw_line (charmap->chartable_pixmap,
                 charmap->chartable->style->fg_gc[GTK_STATE_INSENSITIVE], 
                 0, 0, 0, charmap->chartable->allocation.height - 1);
  for (col = 0, x = 0;  col < charmap->cols;  col++)
    {
      x += column_width (charmap, col);
      gdk_draw_line (charmap->chartable_pixmap,
                     charmap->chartable->style->fg_gc[GTK_STATE_INSENSITIVE], 
                     x, 0, x, charmap->chartable->allocation.height - 1);
    }

  /* horizontal lines */
  gdk_draw_line (charmap->chartable_pixmap,
                 charmap->chartable->style->fg_gc[GTK_STATE_INSENSITIVE], 
                 0, 0, charmap->chartable->allocation.width - 1, 0);
  for (row = 0, y = 0;  row < charmap->rows;  row++)
    {
      y += row_height (charmap, row);
      gdk_draw_line (charmap->chartable_pixmap,
                     charmap->chartable->style->fg_gc[GTK_STATE_INSENSITIVE], 
                     0, y, charmap->chartable->allocation.width - 1, y);
    }
}


/* draws the backing store pixmap */
static void
draw_chartable_from_scratch (Charmap *charmap)
{
  gint row, col;

  /* plain background */
  gdk_draw_rectangle (charmap->chartable_pixmap,
                      charmap->chartable->style->base_gc[GTK_STATE_NORMAL], 
                      TRUE, 0, 0, 
                      charmap->chartable->allocation.width, 
                      charmap->chartable->allocation.height);
  draw_borders (charmap);

  /* draw the characters */
  for (row = 0;  row < charmap->rows;  row++)
    for (col = 0;  col < charmap->cols;  col++)
      {
        gunichar uc = rowcol_to_unichar (charmap, row, col);

        /* for others, the background was drawn in a big swath */
        if (uc == charmap->active_char)
          draw_square_bg (charmap, row, col);

        draw_character (charmap, row, col);
      }
}


/* redraws the screen from the backing pixmap */
static gint
expose_event (GtkWidget *widget, 
              GdkEventExpose *event, 
              gpointer callback_data)
{
  Charmap *charmap;

  charmap = CHARMAP (callback_data);

  if (charmap->chartable_pixmap == NULL)
    {
      charmap->chartable_pixmap = gdk_pixmap_new (
              charmap->chartable->window, 
              charmap->chartable->allocation.width,
              charmap->chartable->allocation.height, -1);

      draw_chartable_from_scratch (charmap);
    }

  gdk_draw_drawable (charmap->chartable->window,
                     widget->style->fg_gc[GTK_STATE_NORMAL],
                     charmap->chartable_pixmap,
                     event->area.x, event->area.y,
                     event->area.x, event->area.y,
                     event->area.width, event->area.height);

  return FALSE;
}


static void
draw_and_expose_character_square (Charmap *charmap, gunichar uc)
{
  gint row = (uc - charmap->page_first_char) / charmap->cols;
  gint col = unichar_column (charmap, uc);

  if (row >= 0 && row < charmap->rows && col >= 0 && col < charmap->cols)
    {
      draw_square (charmap, row, col);
      expose_square (charmap, row, col);
    }
}


static void
copy_rows (Charmap *charmap, gint row_offset)
{
  gint num_padded_rows;
  gint from_row, to_row;

  num_padded_rows = charmap->chartable->allocation.height -
                    (minimal_row_height (charmap) * charmap->rows + 1);

  if (abs (row_offset) < charmap->rows - num_padded_rows)
    {
      gint num_rows, height;

      if (row_offset > 0)
        {
          from_row = row_offset;
          to_row = 0;
          num_rows = charmap->rows - num_padded_rows - from_row;
        }
      else
        {
          from_row = 0;
          to_row = -row_offset;
          num_rows = charmap->rows - num_padded_rows - to_row;
        }

      height = y_offset (charmap, num_rows) - y_offset (charmap, 0) - 1;

      gdk_draw_drawable (charmap->chartable_pixmap,
                         charmap->chartable->style->base_gc[GTK_STATE_NORMAL], 
                         charmap->chartable_pixmap, 
                         0, y_offset (charmap, from_row), 
                         0, y_offset (charmap, to_row),
                         charmap->chartable->allocation.width, 
                         height);
    }

  if (abs (row_offset) < num_padded_rows)
    {
      /* don't need num_rows or height, cuz we can go off the end */
      if (row_offset > 0)
        {
          from_row = charmap->rows - num_padded_rows + row_offset;
          to_row = charmap->rows - num_padded_rows;
        }
      else
        {
          from_row = charmap->rows - num_padded_rows;
          to_row = charmap->rows - num_padded_rows - row_offset;
        }

      /* it's ok to go off the end (so use allocation.height) */
      gdk_draw_drawable (charmap->chartable_pixmap,
                         charmap->chartable->style->base_gc[GTK_STATE_NORMAL], 
                         charmap->chartable_pixmap, 
                         0, y_offset (charmap, from_row), 
                         0, y_offset (charmap, to_row),
                         charmap->chartable->allocation.width, 
                         charmap->chartable->allocation.height);
    }
}


static void
redraw_rows (Charmap *charmap, gint row_offset)
{
  gint row, col, start_row, end_row;

  if (row_offset > 0) 
    {
      start_row = charmap->rows - row_offset;
      end_row = charmap->rows - 1;
    }
  else
    {
      start_row = 0;
      end_row = -row_offset - 1;
    }

  for (row = 0;  row <= charmap->rows;  row++)
    {
      gboolean draw_row = FALSE;

      draw_row = draw_row || (row >= start_row && row <= end_row);

      if (row + row_offset >= 0 && row + row_offset <= charmap->rows)
        draw_row = draw_row || (row_height (charmap, row) 
                                != row_height (charmap, row + row_offset));

      if (draw_row)
        {
          /* g_print ("redraw_rows: redrawing row %d\n", row); */
          for (col = 0;  col < charmap->cols;  col++)
            draw_square (charmap, row, col);
        }
    }
}


/* Redraws whatever needs to be redrawn, in the character table and caption
 * and everything, and exposes what needs to be exposed. */
static void
redraw (Charmap *charmap)
{
  gint row_offset;
  gboolean actives_done = FALSE;

  row_offset = ((gint) charmap->page_first_char 
                - (gint) charmap->old_page_first_char)
               / charmap->cols;

#ifdef G_PLATFORM_WIN32

  if (row_offset != 0)
    {
      /* get around the bug in gdkdrawable-win32.c */
      /* yup, this makes it really slow */
      draw_chartable_from_scratch (charmap);
      gtk_widget_queue_draw (charmap->chartable);
      actives_done = TRUE;
    }

#else /* #ifdef G_PLATFORM_WIN32 */

  if (row_offset >= charmap->rows || row_offset <= -charmap->rows)
    {
      draw_chartable_from_scratch (charmap);
      gtk_widget_queue_draw (charmap->chartable);
      actives_done = TRUE;
    }
  else if (row_offset != 0)
    {
      copy_rows (charmap, row_offset);
      redraw_rows (charmap, row_offset);
      draw_borders (charmap);
      gtk_widget_queue_draw (charmap->chartable);
    }

#endif /* #else (#ifdef G_PLATFORM_WIN32) */

  if (charmap->active_char != charmap->old_active_char)
    {
      set_caption (charmap);
      set_active_block (charmap);
      set_scrollbar_adjustment (charmap); /* XXX */

      if (!actives_done)
        {
          draw_and_expose_character_square (charmap, charmap->old_active_char);
          draw_and_expose_character_square (charmap, charmap->active_char);
        }
    }

  charmap->old_page_first_char = charmap->page_first_char;
  charmap->old_active_char = charmap->active_char;
}


/* XXX: the logic this function uses to set page_first_char is hideous and
 * probably wrong */
static void
set_active_character (Charmap *charmap, gunichar uc)
{
  gint offset;

  g_return_if_fail (uc >= 0 && uc <= UNICHAR_MAX);

  charmap->old_active_char = charmap->active_char;
  charmap->old_page_first_char = charmap->page_first_char;

  charmap->active_char = uc;

  /* active_char is still on the current page */
  if (uc - charmap->page_first_char < charmap->rows * charmap->cols)
    return;

  /* move the page_first_char as far as active_char has moved */
  offset = (gint) charmap->active_char - (gint) charmap->old_active_char;

  if ((gint) charmap->old_page_first_char + offset >= 0)
    charmap->page_first_char = charmap->old_page_first_char + offset;
  else
    charmap->page_first_char = 0;

  /* round down so that it's a multiple of charmap->cols */
  charmap->page_first_char -= (charmap->page_first_char % charmap->cols);

  /* go back up if we should have rounded up */
  if (charmap->active_char - charmap->page_first_char 
          >= charmap->rows * charmap->cols)
    charmap->page_first_char += charmap->cols;
}


static PangoLayout *
layout_scaled_glyph (Charmap *charmap, gunichar uc, gdouble scale)
{
  gint default_font_size, font_size;
  PangoLayout *layout;
  PangoFontDescription *font_desc;

  default_font_size = pango_font_description_get_size (
      gtk_widget_get_style (GTK_WIDGET (charmap->chartable))->font_desc);

  if (default_font_size <= 0)
    default_font_size = 10 * PANGO_SCALE;

  font_desc = pango_font_description_copy (
      gtk_widget_get_style (charmap->chartable)->font_desc);

  font_size = scale * default_font_size;
  if (font_size <= 0)
    font_size = default_font_size;

  pango_font_description_set_size (font_desc, font_size);

  layout = pango_layout_new (pango_layout_get_context (charmap->pango_layout));

  pango_layout_set_font_description (layout, font_desc);
  pango_layout_set_text (layout, unichar_to_printable_utf8 (uc), -1);

  pango_font_description_free (font_desc);

  return layout;
}


static gdouble
compute_drag_icon_scale (Charmap *charmap)
{
  gint screen_height;
  gdouble limit;

#if GTK_CHECK_VERSION (2,1,1)
  screen_height = gdk_screen_get_height (
      gtk_widget_get_screen (charmap->chartable));
#else
  screen_height = gdk_screen_height ();
#endif

  limit = (0.3 * screen_height) / bare_minimal_row_height (charmap);

  return CLAMP (limit, 0.1, 12.0);
}


static GdkPixbuf *
create_drag_icon (Charmap *charmap)
{
  enum { PADDING = 8 };

  PangoLayout *pango_layout;
  PangoRectangle ink_rect;
  gint pixmap_width, pixmap_height;
  GtkStyle *style;
  GdkPixmap *pixmap;
  GdkPixbuf *pixbuf;

  /* Apply the scaling.  Unfortunately not all fonts seem to be scalable.
   * We could fall back to GdkPixbuf scaling, but that looks butt ugly :/
   */
  pango_layout = layout_scaled_glyph (charmap, charmap->active_char, 
                                      compute_drag_icon_scale (charmap));

  pango_layout_get_pixel_extents (pango_layout, &ink_rect, NULL);

  /* Make the GdkPixmap large enough to account for
   * possible offsets in the ink extents of the glyph.
   */
  pixmap_width  = ink_rect.width  + abs(ink_rect.x) + 2 * PADDING;
  pixmap_height = ink_rect.height + abs(ink_rect.y) + 2 * PADDING;

  style = gtk_widget_get_style (charmap->chartable);

  pixmap = gdk_pixmap_new (charmap->chartable->window,
                           pixmap_width, pixmap_height,
                           -1);

  gdk_draw_rectangle (pixmap,
                      style->base_gc[GTK_STATE_NORMAL],
                      TRUE,
                      0, 0,
                      pixmap_width, pixmap_height);

  /* Draw a rectangular border while taking ink_rect offsets
   * into account.  Superfluous edges will be clipped later.
   */
  gdk_draw_rectangle (pixmap,
                      style->fg_gc[GTK_STATE_INSENSITIVE], 
                      FALSE,
                      MAX(0, ink_rect.x) + 1,
                      MAX(0, ink_rect.y) + 1,
                      ink_rect.width  + 2 * PADDING - 3,
                      ink_rect.height + 2 * PADDING - 3);

  /* Now draw the glyph.  The coordinates are adapted
   * in order to compensate negative ink_rect offsets.
   */
  gdk_draw_layout (pixmap,
                   style->text_gc[GTK_STATE_NORMAL],
                   MAX(0, -ink_rect.x) + PADDING,
                   MAX(0, -ink_rect.y) + PADDING,
                   pango_layout);

  /* Finally extract the area that has actually been drawn on;
   * projected parts are discarded.  The resulting area is exactly
   * that of the rectangle drawn earlier plus 1 pixel border width.
   */
  pixbuf = gdk_pixbuf_get_from_drawable (NULL, pixmap, NULL,
                                         MAX(0, ink_rect.x),
                                         MAX(0, ink_rect.y),
                                         0, 0,
                                         ink_rect.width  + 2 * PADDING,
                                         ink_rect.height + 2 * PADDING);

  g_object_unref (pango_layout);
  g_object_unref (pixmap);

  return pixbuf;
}


static void
set_top_row (Charmap *charmap, gint row)
{
  gint r, c;

  g_return_if_fail (row >= 0 && row <= UNICHAR_MAX / charmap->cols);

  charmap->old_page_first_char = charmap->page_first_char;
  charmap->old_active_char = charmap->active_char;

  charmap->page_first_char = row * charmap->cols;

  /* character is still on the visible page */
  if (charmap->active_char - charmap->page_first_char 
          < charmap->rows * charmap->cols)
    return;

  c = charmap->old_active_char % charmap->cols;

  if (charmap->page_first_char < charmap->old_page_first_char)
    r = charmap->rows - 1;
  else
    r = 0;

  charmap->active_char = charmap->page_first_char + r * charmap->cols + c;
  if (charmap->active_char > UNICHAR_MAX)
    charmap->active_char = UNICHAR_MAX;
}


static void
move_home (Charmap *charmap)
{
  set_active_character (charmap, 0x0000);
}

static void
move_end (Charmap *charmap)
{
  set_active_character (charmap, UNICHAR_MAX);
}

static void
move_up (Charmap *charmap)
{
  if (charmap->active_char >= charmap->cols)
    set_active_character (charmap, charmap->active_char - charmap->cols);
}

static void
move_down (Charmap *charmap)
{
  if (charmap->active_char <= UNICHAR_MAX - charmap->cols)
    set_active_character (charmap, charmap->active_char + charmap->cols);
}


static void
move_cursor (Charmap *charmap, gint offset)
{
  if (charmap->active_char + offset >= 0
      && charmap->active_char + offset <= UNICHAR_MAX)
    set_active_character (charmap, charmap->active_char + offset);
}

static void
move_left (Charmap *charmap)
{
  if (gtk_widget_get_direction (charmap->chartable) == GTK_TEXT_DIR_RTL)
    move_cursor (charmap, 1);
  else
    move_cursor (charmap, -1);
}

static void
move_right (Charmap *charmap)
{
  if (gtk_widget_get_direction (charmap->chartable) == GTK_TEXT_DIR_RTL)
    move_cursor (charmap, -1);
  else
    move_cursor (charmap, 1);
}

static void
move_page_up (Charmap *charmap)
{
  if (charmap->active_char >= charmap->cols * charmap->rows)
    set_active_character (charmap, 
                          charmap->active_char - charmap->cols * charmap->rows);
  else if (charmap->active_char > 0)
    set_active_character (charmap, 0);
}

static void
move_page_down (Charmap *charmap)
{
  if (charmap->active_char < UNICHAR_MAX - charmap->cols * charmap->rows)
    set_active_character (charmap, 
                          charmap->active_char + charmap->cols * charmap->rows);
  else if (charmap->active_char < UNICHAR_MAX)
    set_active_character (charmap, UNICHAR_MAX);
}


/* mostly for moving around in the charmap */
static gint
key_press_event (GtkWidget *widget, 
                 GdkEventKey *event, 
                 gpointer callback_data)
{
  Charmap *charmap;
  gunichar old_active_char;
  gunichar old_page_first_char;

  charmap = CHARMAP (callback_data);
  old_active_char = charmap->active_char;
  old_page_first_char = charmap->page_first_char;

  /* move the cursor or whatever depending on which key was pressed */
  switch (event->keyval)
    {
      case GDK_Home: case GDK_KP_Home:
        move_home (charmap);
        break;

      case GDK_End: case GDK_KP_End:
        move_end (charmap);
        break;

      case GDK_Up: case GDK_KP_Up: case GDK_k:
        move_up (charmap);
        break;

      case GDK_Down: case GDK_KP_Down: case GDK_j:
        move_down (charmap);
        break;

      case GDK_Left: case GDK_KP_Left: case GDK_h:
        move_left (charmap);
        break;

      case GDK_Right: case GDK_KP_Right: case GDK_l:
        move_right (charmap);
        break;

      case GDK_Page_Up: case GDK_b: case GDK_minus:
        move_page_up (charmap);
        break;

      case GDK_Page_Down:
        move_page_down (charmap);
        break;

      case GDK_Return: case GDK_KP_Enter: case GDK_space:
	g_signal_emit (charmap, charmap_signals[ACTIVATE], 0, 
		       charmap->active_char);
        return TRUE;

      /* pass on other keys, like tab and stuff that shifts focus */
      default:
        return FALSE;
    }

  redraw (charmap);

  return TRUE;
}


/* for mouse clicks */
static gunichar
get_char_at (Charmap *charmap, gint x, gint y)
{
  gint r, c, x0, y0;
  gunichar rv;

  for (c = 0, x0 = 0;  x0 < x;  c++)
    x0 += column_width (charmap, c);

  for (r = 0, y0 = 0;  y0 < y;  r++)
    y0 += row_height (charmap, r);

  rv = rowcol_to_unichar (charmap, r-1, c-1);

  /* XXX: check this somewhere else? */
  if (rv > UNICHAR_MAX)
    return UNICHAR_MAX;

  return rv;
}


typedef struct
  {
    Charmap *charmap;
    GdkEventButton *event;
  }
DragTimeoutData;


static DragTimeoutData *
drag_timeout_data_new (Charmap *charmap, GdkEventButton *event)
{
  DragTimeoutData *timeout_data;

  timeout_data = g_new (DragTimeoutData, 1);
  timeout_data->charmap = charmap;
  timeout_data->event = (GdkEventButton *)gdk_event_copy ((GdkEvent *)event);

  return timeout_data;
}


/* Take a gpointer argument to avoid dangerous function pointer casts. */
static void
drag_timeout_data_free (gpointer data)
{
  DragTimeoutData *timeout_data;

  timeout_data = (DragTimeoutData *)data;
  gdk_event_free ((GdkEvent *)timeout_data->event);
  g_free (timeout_data);
}


static gboolean
drag_begin_timeout (gpointer data)
{
  DragTimeoutData *timeout_data;
  GtkTargetList *target_list;

  timeout_data = (DragTimeoutData *)data;

  /* Indicate that the timeout has fired. */
  timeout_data->charmap->drag_begin_timeout_id = 0;

  target_list = gtk_target_list_new (dnd_target_table,
                                     G_N_ELEMENTS(dnd_target_table));

  gtk_drag_begin (timeout_data->charmap->chartable,
                  target_list,
                  GDK_ACTION_COPY,
                  timeout_data->event->button,
                  (GdkEvent *)timeout_data->event);

  gtk_target_list_unref (target_list);

  return FALSE; /* don't call me again */
}


static void
start_drag_begin_timeout (Charmap *charmap, GdkEventButton *event)
{
  gint double_click_time;

  g_return_if_fail (charmap->drag_begin_timeout_id == 0);

  g_object_get (gtk_widget_get_settings (charmap->chartable),
                "gtk-double-click-time", &double_click_time,
                NULL);

  charmap->drag_begin_timeout_id = g_timeout_add_full (
      G_PRIORITY_DEFAULT_IDLE,
      double_click_time,
      drag_begin_timeout,
      drag_timeout_data_new (charmap, event),
      drag_timeout_data_free);
}


static void
cancel_drag_begin_timeout (Charmap *charmap)
{
  if (charmap->drag_begin_timeout_id != 0)
    {
      g_source_remove (charmap->drag_begin_timeout_id);
      charmap->drag_begin_timeout_id = 0;
    }
}


/*  - single click with left button: activate character under pointer
 *  - double-click with left button: add active character to text_to_copy
 *  - single-click with middle button: jump to selection_primary
 */
static gint
button_press_event (GtkWidget *widget, 
                    GdkEventButton *event, 
                    Charmap *charmap)
{
  if (event->button == 1)
    cancel_drag_begin_timeout (charmap);

  /* in case we lost keyboard focus and are clicking to get it back */
  gtk_widget_grab_focus (charmap->chartable);

  /* double-click */
  if (event->button == 1 && event->type == GDK_2BUTTON_PRESS)
    {
      g_signal_emit (charmap, charmap_signals[ACTIVATE], 0, 
	             charmap->active_char);
    }
  /* single-click */
  else if (event->button == 1 && event->type == GDK_BUTTON_PRESS)
    {
      set_active_character (charmap,
                            get_char_at (charmap, event->x, event->y));
      redraw (charmap);
      start_drag_begin_timeout (charmap, event);
    }
  else if (event->button == 2)
    {
      charmap_identify_clipboard (charmap, 
                                  gtk_clipboard_get (GDK_SELECTION_PRIMARY));
    }

  /* need to return false so it gets drag events */
  return FALSE;
}


static gint
button_release_event (GtkWidget *widget, 
                      GdkEventButton *event, 
                      Charmap *charmap)
{
  if (event->button == 1)
    cancel_drag_begin_timeout (charmap);

  return FALSE;
}


static void
block_selection_changed (GtkTreeSelection *selection, 
                         gpointer user_data)
{
  GtkTreeModel *model;
  GtkTreeIter iter;
  Charmap *charmap;
  gunichar uc_start;

  charmap = CHARMAP (user_data);

  if (gtk_tree_selection_get_selected (selection, &model, &iter))
    {
      gtk_tree_model_get (model, &iter, BLOCK_SELECTOR_UC_START, 
                          &uc_start, -1);

      set_active_character (charmap, uc_start);
      redraw (charmap);
    }
}


/* makes the list of unicode blocks and code points */
static GtkWidget *
make_unicode_block_selector (Charmap *charmap)
{
  GtkWidget *scrolled_window;
  GtkTreeIter iter;
  GtkTreeIter child_iter;
  GtkCellRenderer *cell;
  GtkTreeViewColumn *column;
  gchar buf[12];
  gunichar uc;
  gint i, bi;

  scrolled_window = gtk_scrolled_window_new (NULL, NULL);

  gtk_scrolled_window_set_policy (GTK_SCROLLED_WINDOW (scrolled_window),
                                  GTK_POLICY_NEVER, GTK_POLICY_AUTOMATIC);
  gtk_scrolled_window_set_shadow_type (GTK_SCROLLED_WINDOW (scrolled_window), 
                                       GTK_SHADOW_ETCHED_IN);

  charmap->block_selector_model = gtk_tree_store_new (
          BLOCK_SELECTOR_NUM_COLUMNS, G_TYPE_STRING, 
          G_TYPE_UINT, G_TYPE_POINTER);

  /* UNICHAR_MAX / BLOCK_SIZE is U+XXXX blocks, count_blocks is named blocks */
  charmap->block_index_size = (UNICHAR_MAX / BLOCK_SIZE) + 2
                              + count_blocks (UNICHAR_MAX);
  charmap->block_index = g_malloc (charmap->block_index_size 
                                   * sizeof (block_index_t));
  bi = 0;

  for (i = 0;  unicode_blocks[i].start != (gunichar)(-1)
               && unicode_blocks[i].start <= UNICHAR_MAX;  i++)
    {
      gtk_tree_store_append (charmap->block_selector_model, &iter, NULL);
      gtk_tree_store_set (charmap->block_selector_model, &iter, 
                          BLOCK_SELECTOR_LABEL, unicode_blocks[i].name, 
                          BLOCK_SELECTOR_UC_START, unicode_blocks[i].start,
                          BLOCK_SELECTOR_UNICODE_BLOCK, &(unicode_blocks[i]),
                          -1);
      charmap->block_index[bi].start = unicode_blocks[i].start;
      charmap->block_index[bi].tree_path = gtk_tree_model_get_path (
              GTK_TREE_MODEL (charmap->block_selector_model), &iter);
      bi++;

      if (unicode_blocks[i].start % BLOCK_SIZE == 0)
        uc = unicode_blocks[i].start;
      else
        uc = unicode_blocks[i].start + BLOCK_SIZE 
            - (unicode_blocks[i].start % BLOCK_SIZE);

      for ( ; uc >= unicode_blocks[i].start && uc <= unicode_blocks[i].end 
              && uc <= UNICHAR_MAX;  uc += BLOCK_SIZE) 
        {
          g_snprintf (buf, sizeof (buf), "U+%4.4X", uc);
	  gtk_tree_store_append (charmap->block_selector_model, 
                                 &child_iter, &iter);
	  gtk_tree_store_set (charmap->block_selector_model, &child_iter, 
                              BLOCK_SELECTOR_LABEL, buf, 
                              BLOCK_SELECTOR_UC_START, uc, 
                              BLOCK_SELECTOR_UNICODE_BLOCK, NULL, -1);
          charmap->block_index[bi].start = uc;
          charmap->block_index[bi].tree_path = gtk_tree_model_get_path (
                  GTK_TREE_MODEL (charmap->block_selector_model), &child_iter);
          bi++;
        }
    }

  /* terminate value that is bigger than the biggest character */
  charmap->block_index[bi].start = UNICHAR_MAX + 1;
  charmap->block_index[bi].tree_path = NULL;

  /* we have the model, now make the view */
  charmap->block_selector_view = gtk_tree_view_new_with_model (
          GTK_TREE_MODEL (charmap->block_selector_model));
  charmap->block_selection = gtk_tree_view_get_selection (
          GTK_TREE_VIEW (charmap->block_selector_view));
  gtk_tree_view_set_headers_visible (
          GTK_TREE_VIEW (charmap->block_selector_view), FALSE);

  cell = gtk_cell_renderer_text_new ();
  column = gtk_tree_view_column_new_with_attributes (NULL, cell, 
                                                     "text", 0, NULL);

  gtk_tree_view_append_column (GTK_TREE_VIEW (charmap->block_selector_view),
                               GTK_TREE_VIEW_COLUMN (column));

  gtk_tree_selection_set_mode (charmap->block_selection, GTK_SELECTION_BROWSE);
  charmap->block_selection_changed_handler_id = g_signal_connect (
          G_OBJECT (charmap->block_selection), "changed", 
          G_CALLBACK (block_selection_changed), charmap);

  gtk_container_add (GTK_CONTAINER (scrolled_window), 
                     charmap->block_selector_view);

  gtk_widget_show_all (scrolled_window);

  return scrolled_window;
}


static GtkWidget *
make_caption (Charmap *charmap)
{
  GtkTreeIter iter0, iter1;
  GtkWidget *tree_view;
  GtkTreeViewColumn *column;
  GtkCellRenderer *cell;
  GtkTreeModel *model;
  GtkWidget *scrolled_window;
  gint ypad;

  charmap->caption = g_malloc (sizeof (Caption));

  charmap->caption->caption_model = gtk_tree_store_new (CAPTION_NUM_COLUMNS,
                                                        G_TYPE_STRING, 
                                                        G_TYPE_STRING);

  /* save some typing */
  model = GTK_TREE_MODEL (charmap->caption->caption_model);

  gtk_tree_store_append (charmap->caption->caption_model, &iter0, NULL);
  charmap->caption->name = gtk_tree_row_reference_new (
          model, gtk_tree_model_get_path (model, &iter0));
  gtk_tree_store_set (charmap->caption->caption_model, &iter0,
                      CAPTION_LABEL, _("Name"), -1);

  gtk_tree_store_append (charmap->caption->caption_model, &iter0, NULL);
  charmap->caption->codepoint = gtk_tree_row_reference_new (
          model, gtk_tree_model_get_path (model, &iter0));
  gtk_tree_store_set (charmap->caption->caption_model, &iter0,
                      CAPTION_LABEL, _("Unicode code point"), -1);

  gtk_tree_store_append (charmap->caption->caption_model, &iter1, &iter0);
  charmap->caption->category = gtk_tree_row_reference_new (
          model, gtk_tree_model_get_path (model, &iter1));
  gtk_tree_store_set (charmap->caption->caption_model, &iter1,
                      CAPTION_LABEL, _("Unicode category"), -1);

  gtk_tree_store_append (charmap->caption->caption_model, &iter1, &iter0);
  charmap->caption->decomposition = gtk_tree_row_reference_new (
          model, gtk_tree_model_get_path (model, &iter1));
  gtk_tree_store_set (charmap->caption->caption_model, &iter1,
                      CAPTION_LABEL, _("Canonical decomposition"), -1);

  gtk_tree_store_append (charmap->caption->caption_model, &iter1, &iter0);
  charmap->caption->utf8 = gtk_tree_row_reference_new (
          model, gtk_tree_model_get_path (model, &iter1));
  gtk_tree_store_set (charmap->caption->caption_model, &iter1,
                      CAPTION_LABEL, _("UTF-8"), -1);

#if ENABLE_UNIHAN
  gtk_tree_store_append (charmap->caption->caption_model, &iter1, &iter0);
  charmap->caption->kDefinition = gtk_tree_row_reference_new (
          model, gtk_tree_model_get_path (model, &iter1));
  gtk_tree_store_set (charmap->caption->caption_model, &iter1,
                      CAPTION_LABEL, _("CJK ideograph definition"), -1);

  gtk_tree_store_append (charmap->caption->caption_model, &iter1, &iter0);
  charmap->caption->kMandarin = gtk_tree_row_reference_new (
          model, gtk_tree_model_get_path (model, &iter1));
  gtk_tree_store_set (charmap->caption->caption_model, &iter1,
                      CAPTION_LABEL, _("Mandarin pronunciation"), -1);

  gtk_tree_store_append (charmap->caption->caption_model, &iter1, &iter0);
  charmap->caption->kJapaneseOn = gtk_tree_row_reference_new (
          model, gtk_tree_model_get_path (model, &iter1));
  gtk_tree_store_set (charmap->caption->caption_model, &iter1,
                      CAPTION_LABEL, _("Japanese On pronunciation"), -1);

  gtk_tree_store_append (charmap->caption->caption_model, &iter1, &iter0);
  charmap->caption->kJapaneseKun = gtk_tree_row_reference_new (
          model, gtk_tree_model_get_path (model, &iter1));
  gtk_tree_store_set (charmap->caption->caption_model, &iter1,
                      CAPTION_LABEL, _("Japanese Kun pronunciation"), -1);

  gtk_tree_store_append (charmap->caption->caption_model, &iter1, &iter0);
  charmap->caption->kCantonese = gtk_tree_row_reference_new (
          model, gtk_tree_model_get_path (model, &iter1));
  gtk_tree_store_set (charmap->caption->caption_model, &iter1,
                      CAPTION_LABEL, _("Cantonese pronunciation"), -1);

  gtk_tree_store_append (charmap->caption->caption_model, &iter1, &iter0);
  charmap->caption->kTang = gtk_tree_row_reference_new (
          model, gtk_tree_model_get_path (model, &iter1));
  gtk_tree_store_set (charmap->caption->caption_model, &iter1,
                      CAPTION_LABEL, _("Tang pronunciation"), -1);

  gtk_tree_store_append (charmap->caption->caption_model, &iter1, &iter0);
  charmap->caption->kKorean = gtk_tree_row_reference_new (
          model, gtk_tree_model_get_path (model, &iter1));
  gtk_tree_store_set (charmap->caption->caption_model, &iter1,
                      CAPTION_LABEL, _("Korean pronunciation"), -1);
#endif /* #if ENABLE_UNIHAN */

  /* now make the tree view */
  tree_view = gtk_tree_view_new_with_model (
          GTK_TREE_MODEL (charmap->caption->caption_model));
  gtk_tree_view_set_headers_visible (GTK_TREE_VIEW (tree_view), FALSE);

  cell = gtk_cell_renderer_text_new ();
  column = gtk_tree_view_column_new_with_attributes (NULL, cell,
                                                     "text", CAPTION_LABEL,
                                                     NULL);
  g_object_set (G_OBJECT (cell), "weight", PANGO_WEIGHT_BOLD, NULL);
  gtk_tree_view_append_column (GTK_TREE_VIEW (tree_view), column);

  cell = gtk_cell_renderer_text_new ();
  column = gtk_tree_view_column_new_with_attributes (NULL, cell,
                                                     "text", CAPTION_VALUE,
                                                     NULL);
  gtk_tree_view_append_column (GTK_TREE_VIEW (tree_view), column);

  /* make it "editable" so the value can be copied and pasted */
  g_object_set (G_OBJECT (cell), "editable", TRUE, NULL); 
  g_object_get (G_OBJECT (cell), "ypad", &ypad, NULL);
  ypad += 2;  /* give it a few more pixels vertically */
  g_object_set (G_OBJECT (cell), "ypad", ypad, NULL);

  /* do this so it doesn't manically change size */
  gtk_cell_renderer_text_set_fixed_height_from_font (
          GTK_CELL_RENDERER_TEXT (cell), 1);

  scrolled_window = gtk_scrolled_window_new (NULL, NULL);
  gtk_scrolled_window_add_with_viewport (GTK_SCROLLED_WINDOW (scrolled_window),
                                         tree_view);
  gtk_scrolled_window_set_policy (GTK_SCROLLED_WINDOW (scrolled_window),
                                  GTK_POLICY_ALWAYS, GTK_POLICY_NEVER);

  gtk_widget_show_all (scrolled_window);
  gtk_widget_hide (scrolled_window);

  return scrolled_window;
}


static void       
selection_text_received (GtkClipboard *clipboard, 
                         const gchar *text,
                         gpointer data)
{
  gunichar uc;
  Charmap *charmap = CHARMAP (data);

  if (text == NULL)
    {
      if (clipboard == gtk_clipboard_get (GDK_SELECTION_CLIPBOARD))
        g_signal_emit (charmap, charmap_signals[STATUS_MESSAGE], 0, 
		       _("Clipboard is empty."));
      else
        g_signal_emit (charmap, charmap_signals[STATUS_MESSAGE], 0, 
		       _("There is no selected text."));
      return;
    }

  uc = g_utf8_get_char_validated (text, -1);

  if (uc == (gunichar)(-2) || uc == (gunichar)(-1) || uc > UNICHAR_MAX)
    {
      g_signal_emit (charmap, charmap_signals[STATUS_MESSAGE], 0, 
                     _("Unknown character, unable to identify."));
    }
  else
    {
      g_signal_emit (charmap, charmap_signals[STATUS_MESSAGE], 0, 
	             _("Character found."));
      set_active_character (charmap, uc);
      redraw (charmap);
    }
}


static void
scroll_charmap (GtkAdjustment *adjustment, Charmap *charmap)
{
  set_top_row (charmap, (gint) gtk_adjustment_get_value (adjustment));
  redraw (charmap);
}


static gboolean    
focus_in_or_out_event (GtkWidget *widget, GdkEventFocus *event, 
                       gpointer user_data)
{
  Charmap *charmap = CHARMAP (user_data);
  if (charmap->chartable != NULL && charmap->chartable_pixmap != NULL)
    draw_and_expose_character_square (charmap, charmap->active_char);
  return FALSE;
}


static GtkWidget *
make_scrollbar (Charmap *charmap)
{
  charmap->adjustment = gtk_adjustment_new (0.0, 0.0, 
                                            1.0 * UNICHAR_MAX / charmap->cols, 
                                            2.0, 3.0 * charmap->rows, 
                                            0.0);

  charmap->adjustment_changed_handler_id = g_signal_connect (
          G_OBJECT (charmap->adjustment), "value_changed",
          G_CALLBACK (scroll_charmap), charmap);

  return gtk_vscrollbar_new (GTK_ADJUSTMENT (charmap->adjustment));
}


static void
mouse_wheel_up (Charmap *charmap)
{
  if (charmap->page_first_char > charmap->rows * charmap->cols / 2)
    set_top_row (charmap, 
                 (charmap->page_first_char - charmap->rows * charmap->cols / 2) 
                  / charmap->cols);
  else 
    set_top_row (charmap, 0);

  redraw (charmap);
}


static void
mouse_wheel_down (Charmap *charmap)
{
  if (charmap->page_first_char 
          < UNICHAR_MAX - charmap->rows * charmap->cols / 2)
    {
      set_top_row (charmap, (charmap->page_first_char 
                             + charmap->rows * charmap->cols / 2) 
                            / charmap->cols);
    }
  else 
    {
      set_top_row (charmap, UNICHAR_MAX / charmap->cols);
    }

  redraw (charmap);
}


/* mouse wheel scrolls by half a page */
static gboolean    
mouse_wheel_event (GtkWidget *widget, GdkEventScroll *event, Charmap *charmap)
{
  switch (event->direction)
    {
      case GDK_SCROLL_UP:
        mouse_wheel_up (charmap);
        break;

      case GDK_SCROLL_DOWN:
        mouse_wheel_down (charmap);
        break;

      default:
        break;
    }

  return TRUE;
}


static void
style_set (GtkWidget *widget, GtkStyle *previous_style, Charmap *charmap)
{
  if (charmap->chartable_pixmap != NULL)
    g_object_unref (charmap->chartable_pixmap);
  charmap->chartable_pixmap = NULL;

  gtk_widget_queue_draw (charmap->chartable);
}


static void
size_allocate (GtkWidget *widget, GtkAllocation *allocation, Charmap *charmap)
{
  gint old_rows, old_cols;
  GtkAdjustment *adjustment;

  old_rows = charmap->rows;
  old_cols = charmap->cols;

  charmap->cols = (allocation->width - 1) / bare_minimal_column_width (charmap);
  charmap->rows = (allocation->height - 1) / bare_minimal_row_height (charmap);

  /* avoid a horrible floating point exception crash */
  if (charmap->rows < 1)
    charmap->rows = 1;
  if (charmap->cols < 1)
    charmap->cols = 1;

  /* force pixmap to be redrawn on next expose event */
  if (charmap->chartable_pixmap != NULL)
    g_object_unref (charmap->chartable_pixmap);
  charmap->chartable_pixmap = NULL;

  if (charmap->rows == old_rows && charmap->cols == old_cols)
    return;

  charmap->page_first_char = charmap->active_char 
                             - (charmap->active_char % charmap->cols);

  /* adjust the adjustment, since it's based on the size of a row */
  adjustment = GTK_ADJUSTMENT (charmap->adjustment);
  adjustment->upper = 1.0 * UNICHAR_MAX / charmap->cols;
  adjustment->page_increment = 3.0 * charmap->rows;
  gtk_adjustment_changed (adjustment);
  set_scrollbar_adjustment (charmap);
}


static void
drag_begin (GtkWidget *widget, 
            GdkDragContext *context,
            Charmap *charmap)
{
  GdkPixbuf *drag_icon;

  cancel_drag_begin_timeout (charmap);

  drag_icon = create_drag_icon (charmap);
  gtk_drag_set_icon_pixbuf (context, drag_icon, -8, -8);
  g_object_unref (drag_icon);
}


static void
drag_data_get (GtkWidget *widget, 
               GdkDragContext *context,
               GtkSelectionData *selection_data,
               guint info,
               guint time,
               Charmap *charmap)

{
  gchar buf[7];
  gint n;

  n = g_unichar_to_utf8 (charmap->active_char, buf);
  gtk_selection_data_set_text (selection_data, buf, n);
}


static void
drag_data_received (GtkWidget *widget,
                    GdkDragContext *context,
                    gint x,
                    gint y,
                    GtkSelectionData *selection_data,
                    guint info,
                    guint time,
                    Charmap *charmap)
{
  gchar *text;
  gunichar uc;

  text = gtk_selection_data_get_text (selection_data);

  if (text == NULL) /* XXX: say something in the statusbar? */
    return;

  uc = g_utf8_get_char_validated (text, -1);

  if (uc == (gunichar)(-2) || uc == (gunichar)(-1) || uc > UNICHAR_MAX)
    {
      g_signal_emit (charmap, charmap_signals[STATUS_MESSAGE], 0, 
                     _("Unknown character, unable to identify."));
    }
  else
    {
      g_signal_emit (charmap, charmap_signals[STATUS_MESSAGE], 0, 
	             _("Character found."));
      set_active_character (charmap, uc);
      redraw (charmap);
    }

  g_free (text);
}


static GtkWidget *
make_chartable (Charmap *charmap)
{
  GtkWidget *hbox;

  charmap->chartable = gtk_drawing_area_new ();

  gtk_widget_set_events (charmap->chartable, 
          GDK_EXPOSURE_MASK | GDK_KEY_PRESS_MASK | GDK_BUTTON_PRESS_MASK |
          GDK_BUTTON_RELEASE_MASK | GDK_FOCUS_CHANGE_MASK | GDK_SCROLL_MASK);

  g_signal_connect (G_OBJECT (charmap->chartable), "expose_event",
                    G_CALLBACK (expose_event), charmap);
  g_signal_connect (G_OBJECT (charmap->chartable), "key_press_event",
                    G_CALLBACK (key_press_event), charmap);
  g_signal_connect (G_OBJECT (charmap->chartable), "button_press_event",
                    G_CALLBACK (button_press_event), charmap);
  g_signal_connect (G_OBJECT (charmap->chartable), "button_release_event",
                    G_CALLBACK (button_release_event), charmap);
  g_signal_connect (G_OBJECT (charmap->chartable), "focus-in-event",
                    G_CALLBACK (focus_in_or_out_event), charmap);
  g_signal_connect (G_OBJECT (charmap->chartable), "focus-out-event",
                    G_CALLBACK (focus_in_or_out_event), charmap);
  g_signal_connect (G_OBJECT (charmap->chartable), "scroll-event",
                    G_CALLBACK (mouse_wheel_event), charmap);
  g_signal_connect (G_OBJECT (charmap->chartable), "style-set",
                    G_CALLBACK (style_set), charmap);
  g_signal_connect (G_OBJECT (charmap->chartable), "size-allocate",
                    G_CALLBACK (size_allocate), charmap);

  gtk_drag_dest_set (charmap->chartable, GTK_DEST_DEFAULT_ALL,
                     dnd_target_table, G_N_ELEMENTS (dnd_target_table),
                     GDK_ACTION_COPY);

  g_signal_connect (G_OBJECT (charmap->chartable), "drag-data-received",
                    G_CALLBACK (drag_data_received), charmap);


  gtk_drag_source_set (charmap->chartable, GDK_BUTTON1_MASK, 
                       dnd_target_table, G_N_ELEMENTS (dnd_target_table),
                       GDK_ACTION_COPY);

  g_signal_connect (G_OBJECT (charmap->chartable), "drag-begin",
                    G_CALLBACK (drag_begin), charmap);

  g_signal_connect (G_OBJECT (charmap->chartable), "drag-data-get",
                    G_CALLBACK (drag_data_get), charmap);

  /* this is required to get key_press events */
  GTK_WIDGET_SET_FLAGS (charmap->chartable, GTK_CAN_FOCUS);

  hbox = gtk_hbox_new (FALSE, 1);
  gtk_box_pack_start (GTK_BOX (hbox), charmap->chartable, TRUE, TRUE, 0);
  gtk_box_pack_start (GTK_BOX (hbox), make_scrollbar (charmap), 
                      FALSE, FALSE, 0);

  gtk_widget_show_all (hbox);

  return hbox;
}


void
charmap_class_init (CharmapClass *clazz)
{
  clazz->activate = NULL;
  clazz->status_message = NULL;

  charmap_signals[ACTIVATE] =
      g_signal_new ("activate", charmap_get_type (), G_SIGNAL_RUN_FIRST,
                    G_STRUCT_OFFSET (CharmapClass, activate),
                    NULL, NULL, gucharmap_marshal_VOID__UINT, G_TYPE_NONE, 
		    1, G_TYPE_UINT);

  charmap_signals[STATUS_MESSAGE] =
      g_signal_new ("status-message", charmap_get_type (), G_SIGNAL_RUN_FIRST,
                    G_STRUCT_OFFSET (CharmapClass, status_message),
                    NULL, NULL, gucharmap_marshal_VOID__STRING, G_TYPE_NONE, 
		    1, G_TYPE_STRING);
}


/* does all the initial construction */
void
charmap_init (Charmap *charmap)
{
  GtkWidget *hpaned;
  GtkWidget *vbox;
  GtkWidget *caption;

  charmap->rows = CHARMAP_MIN_ROWS;
  charmap->cols = CHARMAP_MIN_COLS;

  charmap->drag_begin_timeout_id = 0;

  gtk_box_set_spacing (GTK_BOX (charmap), 6);
  gtk_container_set_border_width (GTK_CONTAINER (charmap), 6);

  /* vbox for charmap and caption */
  vbox = gtk_vbox_new (FALSE, 6);
  gtk_box_pack_start (GTK_BOX (vbox), make_chartable (charmap), TRUE, TRUE, 0);
  caption = make_caption (charmap);
  gtk_widget_show (caption);
  gtk_box_pack_start (GTK_BOX (vbox), caption, FALSE, FALSE, 0);
  gtk_widget_show (vbox);
  /* end vbox for charmap and caption*/

  /* put stuff in top hpaned */
  hpaned = gtk_hpaned_new ();
  gtk_paned_pack1 (GTK_PANED (hpaned), make_unicode_block_selector (charmap), 
                   FALSE, TRUE);
  gtk_paned_pack2 (GTK_PANED (hpaned), vbox, TRUE, TRUE);
  /* done with panes */

  /* start packing stuff in the outer vbox (the Charmap itself) */
  gtk_box_pack_start (GTK_BOX (charmap), hpaned, TRUE, TRUE, 0);
  gtk_widget_show (hpaned);
  /* end packing stuff in the outer vbox (the Charmap itself) */

  charmap->font_name = NULL;

  charmap->font_metrics = pango_context_get_metrics (
          gtk_widget_get_pango_context (charmap->chartable),
          charmap->chartable->style->font_desc, NULL);

  charmap->pango_layout = pango_layout_new (
          gtk_widget_get_pango_context (charmap->chartable));

  pango_layout_set_font_description (charmap->pango_layout,
                                     charmap->chartable->style->font_desc);

  charmap->page_first_char = (gunichar) 0x0000;
  charmap->active_char = (gunichar) 0x0000;

  set_caption (charmap);
  set_active_block (charmap);
  set_scrollbar_adjustment (charmap);
}


GtkWidget *
charmap_new ()
{
  return GTK_WIDGET (g_object_new (charmap_get_type (), NULL));
}


GType
charmap_get_type ()
{
  static GType charmap_type = 0;

  if (!charmap_type)
    {
      static const GTypeInfo charmap_info =
      {
        sizeof (CharmapClass),
        NULL,           /* base_init */
        NULL,           /* base_finalize */
        (GClassInitFunc) charmap_class_init,
        NULL,           /* class_finalize */
        NULL,           /* class_data */
        sizeof (Charmap),
        0,              /* n_preallocs */
        (GInstanceInitFunc) charmap_init,
      };

      charmap_type = g_type_register_static (GTK_TYPE_VBOX, "Charmap", 
                                             &charmap_info, 0);
    }

  return charmap_type;
}


void 
charmap_set_font (Charmap *charmap, gchar *font_name)
{
  PangoFontDescription *font_desc;

  /* if it's the same as the current font, do nothing */
  if (charmap->font_name != NULL
      && g_ascii_strcasecmp (charmap->font_name, font_name) == 0)
    return;
  else
    {
      g_free (charmap->font_name);
      charmap->font_name = NULL;
      charmap->font_name = g_strdup (font_name);
    }

  font_desc = pango_font_description_from_string (charmap->font_name);

  /* ensure style so that this has an effect even before it's realized */
  gtk_widget_ensure_style (charmap->chartable);
  gtk_widget_modify_font (charmap->chartable, font_desc);

  charmap->font_metrics = pango_context_get_metrics (
          gtk_widget_get_pango_context (charmap->chartable),
          charmap->chartable->style->font_desc, NULL);

  /* new pango layout for the new font */
  g_object_unref (charmap->pango_layout);
  charmap->pango_layout = pango_layout_new (
          gtk_widget_get_pango_context (charmap->chartable));

  pango_layout_set_font_description (charmap->pango_layout,
                                     charmap->chartable->style->font_desc);

  pango_font_description_free (font_desc);

  /* force pixmap to be redrawn on next expose event */
  if (charmap->chartable_pixmap != NULL)
    g_object_unref (charmap->chartable_pixmap);
  charmap->chartable_pixmap = NULL;
}


void
charmap_identify_clipboard (Charmap *charmap, GtkClipboard *clipboard)
{
  gtk_clipboard_request_text (clipboard, selection_text_received, charmap);
}


void 
charmap_expand_block_selector (Charmap *charmap)
{
  gtk_tree_view_expand_all (GTK_TREE_VIEW (charmap->block_selector_view));

  /* have to send it an expose event or the change won't happen right away */
  gtk_widget_queue_draw (gtk_widget_get_parent (charmap->block_selector_view));
}


void 
charmap_collapse_block_selector (Charmap *charmap)
{
  gtk_tree_view_collapse_all (GTK_TREE_VIEW (charmap->block_selector_view));

  /* have to send it an expose event or the change won't happen right away */
  gtk_widget_queue_draw (gtk_widget_get_parent (charmap->block_selector_view));
}


void
charmap_go_to_character (Charmap *charmap, gunichar uc)
{
  gchar *message;

  if (uc >= 0 && uc <= UNICHAR_MAX)
    {
      set_active_character (charmap, uc);
      redraw (charmap);
    }

  message = g_strdup_printf ("Jumped to U+%4.4X.", uc);
  g_signal_emit (charmap, charmap_signals[STATUS_MESSAGE], 0, message);
  g_free (message);
}


charmap_search_result_t
charmap_search (Charmap *charmap, const gchar *search_text)
{
  gunichar uc;
  charmap_search_result_t result;

  if (search_text[0] == '\0')
    {
      return NOTHING_TO_SEARCH_FOR;
    }
  
  uc = find_next_substring_match (charmap->active_char, UNICHAR_MAX, 
                                  search_text);
  if (uc != (gunichar)(-1) && uc <= UNICHAR_MAX)
    {
      if (uc <= charmap->active_char)
        result = WRAPPED;
      else
        result = FOUND;

      set_active_character (charmap, uc);
      redraw (charmap);
    }
  else
    result = NOT_FOUND;

  return result;
}

