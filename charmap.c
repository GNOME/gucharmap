/* $Id$ */
/*
 * Copyright (c) 2002  Noah Levitt <nlevitt@users.sourceforge.net>
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

#include <gtk/gtk.h>
#include <gdk/gdkkeysyms.h>
#include "charmap.h"
#include "unicode_info.h"

#define abs(x) ((x) >= 0 ? (x) : (-x))

/* return value is read-only, should not be freed */
static gchar *
unichar_to_printable_utf8 (gunichar uc)
{
  static gchar buf[12];
  gint x;

  if (! g_unichar_isgraph (uc))
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
#define BUFLEN 512
  static gchar buf[BUFLEN];
  gunichar *decomposition;
  gsize result_len;
  gint i;
  gchar *bp;

  g_snprintf (buf, BUFLEN, "codepoint: U+%4.4X", charmap->active_char);
  gtk_label_set_text (GTK_LABEL (charmap->caption->codepoint), buf);

  g_snprintf (buf, BUFLEN, "character: %s", 
              unichar_to_printable_utf8 (charmap->active_char));
  gtk_label_set_text (GTK_LABEL (charmap->caption->character), buf);

  g_snprintf (buf, BUFLEN, "category: %s", 
              get_unicode_category_name (charmap->active_char));
  gtk_label_set_text (GTK_LABEL (charmap->caption->category), buf);

  g_snprintf (buf, BUFLEN, "name: %s", 
              get_unicode_name (charmap->active_char));
  gtk_label_set_text (GTK_LABEL (charmap->caption->name), buf);

  g_snprintf (buf, BUFLEN, "kDefinition: %s", 
              get_unicode_kDefinition (charmap->active_char));
  gtk_label_set_text (GTK_LABEL (charmap->caption->kDefinition), buf);

  g_snprintf (buf, BUFLEN, "kCantonese: %s", 
              get_unicode_kCantonese (charmap->active_char));
  gtk_label_set_text (GTK_LABEL (charmap->caption->kCantonese), buf);

  g_snprintf (buf, BUFLEN, "kMandarin: %s", 
              get_unicode_kMandarin (charmap->active_char));
  gtk_label_set_text (GTK_LABEL (charmap->caption->kMandarin), buf);

  g_snprintf (buf, BUFLEN, "kTang: %s", 
              get_unicode_kTang (charmap->active_char));
  gtk_label_set_text (GTK_LABEL (charmap->caption->kTang), buf);

  g_snprintf (buf, BUFLEN, "kKorean: %s", 
              get_unicode_kKorean (charmap->active_char));
  gtk_label_set_text (GTK_LABEL (charmap->caption->kKorean), buf);

  g_snprintf (buf, BUFLEN, "kJapaneseOn: %s", 
              get_unicode_kJapaneseOn (charmap->active_char));
  gtk_label_set_text (GTK_LABEL (charmap->caption->kJapaneseOn), buf);

  g_snprintf (buf, BUFLEN, "kJapaneseKun: %s", 
              get_unicode_kJapaneseKun (charmap->active_char));
  gtk_label_set_text (GTK_LABEL (charmap->caption->kJapaneseKun), buf);

  /* do the decomposition */
  decomposition = unicode_canonical_decomposition (charmap->active_char,
                                                   &result_len);
  bp = buf;
  bp += g_snprintf (buf, BUFLEN, "decomposition: %s [U+%4.4X]", 
                    unichar_to_printable_utf8 (decomposition[0]),
                    decomposition[0]);
  for (i = 1;  i < result_len;  i++)
    bp += g_snprintf (bp, buf + BUFLEN - bp, " + %s [U+%4.4X]", 
                      unichar_to_printable_utf8 (decomposition[i]),
                      decomposition[i]);
  gtk_label_set_text (GTK_LABEL (charmap->caption->decomposition), buf);

  g_free (decomposition);
}


/* selects the active block in the block selector tree view based on the
 * active character */
static void
set_active_block (Charmap *charmap)
{
  GtkTreePath *tree_path = NULL;
  /* to check for bouncy loops */
  unicode_block_t *unicode_block = NULL;
  unicode_block_t *unicode_block_prev = NULL;
  unicode_block_t *unicode_block_prev_prev = NULL;
  gboolean valid;
  GtkTreeIter iter;

  /* try to start with the current selection */
  valid = gtk_tree_selection_get_selected (charmap->block_selection,
                                           NULL, &iter);
  if (!valid)
    {
      valid = gtk_tree_model_get_iter_first (
              GTK_TREE_MODEL (charmap->block_selector_model), &iter);
    }

  tree_path = gtk_tree_model_get_path (
                  GTK_TREE_MODEL (charmap->block_selector_model), &iter);
  while (valid)
    {
      valid = gtk_tree_model_get_iter (
              GTK_TREE_MODEL (charmap->block_selector_model), &iter,
              tree_path);

      unicode_block_prev_prev = unicode_block_prev;
      unicode_block_prev = unicode_block;
      gtk_tree_model_get (GTK_TREE_MODEL (charmap->block_selector_model), 
                          &iter, BLOCK_SELECTOR_UNICODE_BLOCK, &unicode_block, 
                          -1);

      if (unicode_block == unicode_block_prev_prev)
        goto set_active_block_finished;

      if (charmap->active_char >= unicode_block->start
          && charmap->active_char <= unicode_block->end)
        {
          /* block our "changed" handler */
          g_signal_handler_block (G_OBJECT (charmap->block_selection), 
                                  charmap->block_selection_changed_handler_id);
          gtk_tree_selection_select_path (charmap->block_selection, tree_path);
          g_signal_handler_unblock (
                  G_OBJECT (charmap->block_selection),
                  charmap->block_selection_changed_handler_id);

          gtk_tree_view_scroll_to_cell (
                  GTK_TREE_VIEW (charmap->block_selector_view),
                  tree_path, NULL, FALSE, 0, 0);

          goto set_active_block_finished;
        }
      else if (charmap->active_char < unicode_block->start)
        {
          valid = gtk_tree_path_prev (tree_path);
        }
      else if (charmap->active_char > unicode_block->end)
        {
          /* XXX: this junk is cuz gtk_tree_path_next returns void */
          valid = gtk_tree_model_get_iter (
                  GTK_TREE_MODEL (charmap->block_selector_model), &iter,
                  tree_path);
          valid = gtk_tree_model_iter_next (
                  GTK_TREE_MODEL (charmap->block_selector_model), &iter);
          if (valid)
            gtk_tree_path_next (tree_path);
        }
    }

set_active_block_finished:
  if (tree_path != NULL)
    gtk_tree_path_free (tree_path);
  return;
}


static gint 
calculate_square_dimension_x (PangoFontMetrics *font_metrics)
{
  /* XXX: can't get max width for the font, so just use the height */
  return 5 + (pango_font_metrics_get_ascent (font_metrics) 
              + pango_font_metrics_get_descent (font_metrics)) 
             / PANGO_SCALE;
}


static gint 
calculate_square_dimension_y (PangoFontMetrics *font_metrics)
{
  return 5 + (pango_font_metrics_get_ascent (font_metrics) 
              + pango_font_metrics_get_descent (font_metrics)) 
             / PANGO_SCALE;
}


static gint
calculate_tabulus_dimension_x (Charmap *charmap)
{
  return charmap->cols * (calculate_square_dimension_x (charmap->font_metrics) 
                          + 1) + 1;
}


static gint
calculate_tabulus_dimension_y (Charmap *charmap)
{
  return charmap->rows * (calculate_square_dimension_y (charmap->font_metrics) 
                          + 1) + 1;
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


static void
draw_character (Charmap *charmap, gint row, gint col)
{
  gint padding_x, padding_y;
  gint char_width, char_height;
  gint square_width, square_height; 
  gunichar uc;
  GdkGC *gc;

  uc = charmap->page_first_char + row * charmap->cols + col;

  if (GTK_WIDGET_HAS_FOCUS (charmap->tabulus) && uc == charmap->active_char)
    gc = charmap->tabulus->style->text_gc[GTK_STATE_SELECTED];
  else if (uc == charmap->active_char)
    gc = charmap->tabulus->style->text_gc[GTK_STATE_ACTIVE];
  else
    gc = charmap->tabulus->style->text_gc[GTK_STATE_NORMAL];

  square_width = calculate_square_dimension_x (charmap->font_metrics);
  square_height = calculate_square_dimension_y (charmap->font_metrics);

  pango_layout_set_text (charmap->pango_layout, 
                         unichar_to_printable_utf8 (uc), 
                         -1);

  pango_layout_get_pixel_size (charmap->pango_layout, 
                               &char_width, &char_height);

  /* (square_width - char_width)/2 is the smaller half */
  padding_x = (square_width - char_width) - (square_width - char_width)/2;
  padding_y = (square_height - char_height) - (square_height - char_height)/2;

  /* extra +1 is for the uncounted border */
  gdk_draw_layout (charmap->tabulus_pixmap, gc,
                   (square_width+1) * col + 1 + padding_x,
                   (square_height+1) * row + 1 + padding_y,
                   charmap->pango_layout);
}


static void
draw_square_bg (Charmap *charmap, gint row, gint col)
{
  gint square_width, square_height; 
  gunichar uc;
  GdkGC *gc;

  uc = charmap->page_first_char + row * charmap->cols + col;

  if (GTK_WIDGET_HAS_FOCUS (charmap->tabulus) && uc == charmap->active_char)
    gc = charmap->tabulus->style->base_gc[GTK_STATE_SELECTED];
  else if (uc == charmap->active_char)
    gc = charmap->tabulus->style->base_gc[GTK_STATE_ACTIVE];
  else
    gc = charmap->tabulus->style->base_gc[GTK_STATE_NORMAL];

  square_width = calculate_square_dimension_x (charmap->font_metrics);
  square_height = calculate_square_dimension_y (charmap->font_metrics);

  gdk_draw_rectangle (charmap->tabulus_pixmap, gc, TRUE, 
                      (square_width+1) * col + 1, (square_height+1) * row + 1, 
                      square_width, square_height);
}


static void
expose_square (Charmap *charmap, gint row, gint col)
{
  gint square_width, square_height;

  square_width = calculate_square_dimension_x (charmap->font_metrics);
  square_height = calculate_square_dimension_y (charmap->font_metrics);

  gtk_widget_queue_draw_area (charmap->tabulus, 
                              (square_width+1) * col + 1, 
                              (square_height+1) * row + 1, 
                              square_width, square_height);
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
  gint x, y;
  gint width, height; 
  gint square_width, square_height; 

  square_width = calculate_square_dimension_x (charmap->font_metrics);
  square_height = calculate_square_dimension_y (charmap->font_metrics);

  width = calculate_tabulus_dimension_x (charmap);
  height = calculate_tabulus_dimension_y (charmap);

  /* vertical lines */
  for (x = 0;  x < width;  x += square_width + 1)
    {
      gdk_draw_line (charmap->tabulus_pixmap,
                     charmap->tabulus->style->fg_gc[GTK_STATE_INSENSITIVE], 
                     x, 0, x, height - 1);
    }

  /* horizontal lines */
  for (y = 0;  y < height;  y += square_height + 1)
    {
      gdk_draw_line (charmap->tabulus_pixmap,
                     charmap->tabulus->style->fg_gc[GTK_STATE_INSENSITIVE], 
                     0, y, width - 1, y);
    }
}


/* draws the backing store pixmap */
static void
draw_tabulus_from_scratch (Charmap *charmap)
{
  gint row, col;
  gint width, height;

  width = calculate_tabulus_dimension_x (charmap);
  height = calculate_tabulus_dimension_y (charmap);

  /* plain background */
  gdk_draw_rectangle (charmap->tabulus_pixmap,
                      charmap->tabulus->style->base_gc[GTK_STATE_NORMAL], 
                      TRUE, 0, 0, width, height);
  draw_borders (charmap);

  /* draw the characters */
  for (row = 0;  row < charmap->rows;  row++)
    for (col = 0;  col < charmap->cols;  col++)
      {
        gunichar uc = charmap->page_first_char + row * charmap->cols + col;

        /* for others, the background was drawn in a big swath */
        if (uc == charmap->active_char)
          draw_square_bg (charmap, row, col);

        if (g_unichar_isgraph (uc))
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

  if (charmap->tabulus_pixmap == NULL)
    {
      charmap->tabulus_pixmap = gdk_pixmap_new (
              charmap->tabulus->window, 
              calculate_tabulus_dimension_x (charmap),
              calculate_tabulus_dimension_y (charmap), -1);

      draw_tabulus_from_scratch (charmap);
    }

  gdk_draw_drawable (charmap->tabulus->window,
                     widget->style->fg_gc[GTK_STATE_NORMAL],
                     charmap->tabulus_pixmap,
                     event->area.x, event->area.y,
                     event->area.x, event->area.y,
                     event->area.width, event->area.height);

  return FALSE;
}


static void
draw_and_expose_character_square (Charmap *charmap, gunichar uc)
{
  gint row = (uc - charmap->page_first_char) / charmap->cols;
  gint col = (uc - charmap->page_first_char) % charmap->cols;

  if (row >= 0 && row < charmap->rows && col >= 0 && col < charmap->cols)
    {
      draw_square (charmap, row, col);
      expose_square (charmap, row, col);
    }
}


/* copies the portion of the tabulus that is on the new and old to its new
 * position */
static void
shift_area (Charmap *charmap, gint row_offset)
{
  gint rows;
  gint square_width, square_height, width, height;
  gint area_height;
  gint ysrc, ydest;

  square_width = calculate_square_dimension_x (charmap->font_metrics);
  square_height = calculate_square_dimension_y (charmap->font_metrics);

  width = calculate_tabulus_dimension_x (charmap);
  height = calculate_tabulus_dimension_y (charmap);

  rows = charmap->rows - abs (row_offset);
  area_height = rows * (square_height + 1) + 1;

  if (row_offset > 0) /* moving up */
    {
      ysrc = height - area_height;
      ydest = 0;
    }
  else /* moving down */
    {
      ysrc = 0;
      ydest = height - area_height;
    }

  gdk_draw_drawable (charmap->tabulus_pixmap,
                     charmap->tabulus->style->base_gc[GTK_STATE_NORMAL], 
                     charmap->tabulus_pixmap, 0, ysrc, 0, ydest,
                     width, area_height);
}


/* Redraws the squares in the rows that are newly on the tabulus, based on
 * row_offset; also redraws the active and old_active squares as
 * appropriate. */
static void
draw_squares_after_shift (Charmap *charmap, gint row_offset)
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

  for (row = start_row;  row <= end_row;  row++)
    for (col = 0;  col < charmap->cols;  col++)
      draw_square (charmap, row, col);

  row = (charmap->active_char - charmap->page_first_char) / charmap->cols;
  if (row < start_row || row > end_row)
    {
      col = (charmap->active_char - charmap->page_first_char) % charmap->cols;
      draw_square (charmap, row, col);
    }

  row = (charmap->old_active_char - charmap->page_first_char) / charmap->cols;
  if (row >= 0 && row < charmap->cols && (row < start_row || row > end_row))
    {
      col = (charmap->old_active_char - charmap->page_first_char) 
            % charmap->cols;
      draw_square (charmap, row, col);
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

  if (row_offset >= charmap->rows || row_offset <= -charmap->rows)
    {
      draw_tabulus_from_scratch (charmap);
      gtk_widget_queue_draw (charmap->tabulus);
      actives_done = TRUE;
    }
  else if (row_offset != 0)
    {
      shift_area (charmap, row_offset);
      draw_squares_after_shift (charmap, row_offset);
      gtk_widget_queue_draw (charmap->tabulus);
      actives_done = TRUE;
    }

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


static void
append_character_to_text_to_copy (Charmap *charmap)
{
  static gchar buf[TEXT_TO_COPY_MAXLENGTH];
  static gchar ubuf[7];

  g_unichar_to_utf8 (charmap->active_char, ubuf);
  g_snprintf (buf, TEXT_TO_COPY_MAXLENGTH, "%s%s", 
              gtk_entry_get_text (GTK_ENTRY (charmap->text_to_copy)), ubuf);

  gtk_entry_set_text (GTK_ENTRY (charmap->text_to_copy), buf);
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


static void
set_top_row (Charmap *charmap, gint row)
{
  gint r, c;

  g_return_if_fail (row >= 0 && row <= UNICHAR_MAX / charmap->cols);

  charmap->old_page_first_char = charmap->page_first_char;
  charmap->old_active_char = charmap->active_char;

  charmap->page_first_char = row * charmap->cols;

  if (charmap->active_char - charmap->page_first_char 
          < charmap->rows * charmap->rows)
    return;

  c = charmap->old_active_char % charmap->cols;

  if (charmap->page_first_char < charmap->old_page_first_char)
    r = charmap->rows - 1;
  else
    r = 0;

  charmap->active_char = charmap->page_first_char + r * charmap->cols + c;
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
  else
    set_active_character (charmap, 0);
}

static void
move_down (Charmap *charmap)
{
  if (charmap->active_char <= UNICHAR_MAX - charmap->cols)
    set_active_character (charmap, charmap->active_char + charmap->cols);
  else
    set_active_character (charmap, UNICHAR_MAX);
}

static void
move_left (Charmap *charmap)
{
  if (charmap->active_char > 0)
    set_active_character (charmap, charmap->active_char - 1);
}

static void
move_right (Charmap *charmap)
{
  if (charmap->active_char < UNICHAR_MAX)
    set_active_character (charmap, charmap->active_char + 1);

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

      case GDK_Page_Down: case GDK_space:
        move_page_down (charmap);
        break;

      case GDK_Return: case GDK_KP_Enter:
        append_character_to_text_to_copy (charmap);
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
  gint row, col, w, h;
  gunichar rv;

  w = calculate_square_dimension_x (charmap->font_metrics);
  h = calculate_square_dimension_y (charmap->font_metrics);

  row = y / (h + 1);
  if (row >= charmap->rows)
    row = charmap->rows - 1;

  col = x / (w + 1);
  if (col >= charmap->cols)
    col = charmap->cols - 1;

  rv = charmap->page_first_char + row * charmap->cols + col;

  /* XXX: check this somewhere else? */
  if (rv > UNICHAR_MAX)
    return UNICHAR_MAX;

  return rv;
}


static gint
copy_button_clicked (GtkWidget *widget,
                     gpointer callback_data)
{
  Charmap *charmap = CHARMAP (callback_data);
  GtkClipboard *clipboard = gtk_clipboard_get (GDK_SELECTION_CLIPBOARD);
  gchar *text;

  /* select it so it's in SELECTION_PRIMARY */
  gtk_editable_select_region (GTK_EDITABLE (charmap->text_to_copy), 0, -1);

  /* copy to SELECTION_CLIPBOARD */
  text = gtk_editable_get_chars (GTK_EDITABLE (charmap->text_to_copy), 0, -1);
  gtk_clipboard_set_text (clipboard, text, -1);

  return TRUE;
}


static gint
clear_button_clicked (GtkWidget *widget,
                      gpointer callback_data)
{
  Charmap *charmap = CHARMAP (callback_data);
  gtk_entry_set_text (GTK_ENTRY (charmap->text_to_copy), "");
  return TRUE;
}


/*  - single click with left button: activate character under pointer
 *  - double-click with left button: add active character to text_to_copy
 *  - # single-click with middle button: not implemented
 */
static gint
button_press_event (GtkWidget *widget, 
                    GdkEventButton *event, 
                    gpointer callback_data)
{
  Charmap *charmap = CHARMAP (callback_data);

  /* in case we lost keyboard focus and are clicking to get it back */
  gtk_widget_grab_focus (charmap->tabulus);

  /* double-click */
  if (event->button == 1 && event->type == GDK_2BUTTON_PRESS)
    {
      append_character_to_text_to_copy (charmap);
    }
  /* single-click */
  else if (event->button == 1 && event->type == GDK_BUTTON_PRESS) 
    {
      set_active_character (charmap, 
                            get_char_at (charmap, event->x, event->y));
      redraw (charmap);
    }

  return TRUE;
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
      gtk_tree_model_get (model, &iter, 1, &uc_start, -1);

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
  gint i;

  scrolled_window = gtk_scrolled_window_new (NULL, NULL);
  gtk_scrolled_window_set_policy (GTK_SCROLLED_WINDOW (scrolled_window),
                                  GTK_POLICY_NEVER, GTK_POLICY_AUTOMATIC);
  gtk_scrolled_window_set_shadow_type (GTK_SCROLLED_WINDOW (scrolled_window), 
                                       GTK_SHADOW_ETCHED_IN);

  charmap->block_selector_model = gtk_tree_store_new (
          BLOCK_SELECTOR_NUM_COLUMNS, G_TYPE_STRING, 
          G_TYPE_UINT, G_TYPE_POINTER);

  for (i = 0;  unicode_blocks[i].start != (gunichar)(-1)
               && unicode_blocks[i].start <= UNICHAR_MAX;  i++)
    {
      gtk_tree_store_append (charmap->block_selector_model, &iter, NULL);
      gtk_tree_store_set (charmap->block_selector_model, &iter, 
                          BLOCK_SELECTOR_LABEL, unicode_blocks[i].name, 
                          BLOCK_SELECTOR_UC_START, unicode_blocks[i].start,
                          BLOCK_SELECTOR_UNICODE_BLOCK, &(unicode_blocks[i]),
                          -1);

      for (uc = charmap->rows * charmap->cols + unicode_blocks[i].start - 
                (unicode_blocks[i].start % (charmap->rows * charmap->cols));
           uc >= unicode_blocks[i].start 
             && uc <= unicode_blocks[i].end 
             && uc <= UNICHAR_MAX;  
           uc += charmap->rows * charmap->cols) 
        {
          g_snprintf (buf, 12, "U+%4.4X", uc);
	  gtk_tree_store_append (charmap->block_selector_model, 
                                 &child_iter, &iter);
	  gtk_tree_store_set (charmap->block_selector_model, &child_iter, 
                              BLOCK_SELECTOR_LABEL, buf, 
                              BLOCK_SELECTOR_UC_START, uc, 
                              BLOCK_SELECTOR_UNICODE_BLOCK, NULL, -1);
        }
    }

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

  return scrolled_window;
}


static GtkWidget *
make_caption (Charmap *charmap)
{
  GtkWidget *scrolled_window;
  GtkWidget *table;

  /* most of the rest of this is setting up the caption */
  charmap->caption = g_malloc (sizeof (Caption));
  table = gtk_table_new (6, 4, FALSE);
  gtk_table_set_row_spacings (GTK_TABLE (table), 3);
  gtk_table_set_col_spacings (GTK_TABLE (table), 10);

  charmap->caption->codepoint = gtk_label_new ("codepoint: ");
  charmap->caption->character = gtk_label_new ("character: ");;
  charmap->caption->category = gtk_label_new ("category: ");
  charmap->caption->name = gtk_label_new ("name: ");
  charmap->caption->kDefinition = gtk_label_new ("kDefinition: ");
  charmap->caption->kCantonese = gtk_label_new ("kCantonese: ");
  charmap->caption->kKorean = gtk_label_new ("kKorean: ");
  charmap->caption->kJapaneseOn = gtk_label_new ("kJapaneseOn: ");
  charmap->caption->kTang = gtk_label_new ("kTang: ");
  charmap->caption->kMandarin = gtk_label_new ("kMandarin: ");
  charmap->caption->kJapaneseKun = gtk_label_new ("kJapaneseKun: ");
  charmap->caption->decomposition = gtk_label_new ("decomposition: ");

  gtk_label_set_selectable (GTK_LABEL (charmap->caption->codepoint), TRUE);
  gtk_label_set_selectable (GTK_LABEL (charmap->caption->character), TRUE);
  gtk_label_set_selectable (GTK_LABEL (charmap->caption->category), TRUE);
  gtk_label_set_selectable (GTK_LABEL (charmap->caption->name), TRUE);
  gtk_label_set_selectable (GTK_LABEL (charmap->caption->kDefinition), TRUE);
  gtk_label_set_selectable (GTK_LABEL (charmap->caption->kCantonese), TRUE);
  gtk_label_set_selectable (GTK_LABEL (charmap->caption->kKorean), TRUE);
  gtk_label_set_selectable (GTK_LABEL (charmap->caption->kJapaneseKun), TRUE);
  gtk_label_set_selectable (GTK_LABEL (charmap->caption->kJapaneseOn), TRUE);
  gtk_label_set_selectable (GTK_LABEL (charmap->caption->kTang), TRUE);
  gtk_label_set_selectable (GTK_LABEL (charmap->caption->kMandarin), TRUE);
  gtk_label_set_selectable (GTK_LABEL (charmap->caption->decomposition), TRUE);

  gtk_misc_set_alignment (GTK_MISC (charmap->caption->codepoint), 0, 0);
  gtk_misc_set_alignment (GTK_MISC (charmap->caption->character), 0, 0);
  gtk_misc_set_alignment (GTK_MISC (charmap->caption->category), 0, 0);
  gtk_misc_set_alignment (GTK_MISC (charmap->caption->name), 0, 0);
  gtk_misc_set_alignment (GTK_MISC (charmap->caption->kDefinition), 0, 0);
  gtk_misc_set_alignment (GTK_MISC (charmap->caption->kCantonese), 0, 0);
  gtk_misc_set_alignment (GTK_MISC (charmap->caption->kKorean), 0, 0);
  gtk_misc_set_alignment (GTK_MISC (charmap->caption->kJapaneseKun), 0, 0);
  gtk_misc_set_alignment (GTK_MISC (charmap->caption->kJapaneseOn), 0, 0);
  gtk_misc_set_alignment (GTK_MISC (charmap->caption->kTang), 0, 0);
  gtk_misc_set_alignment (GTK_MISC (charmap->caption->kMandarin), 0, 0);
  gtk_misc_set_alignment (GTK_MISC (charmap->caption->decomposition), 0, 0);

  /*
   * *------------------------------------------------*
   * | codepoint:   | character:  | category:         |
   * *------------------------------------------------*
   * | name:                                          |
   * *------------------------------------------------*
   * | decomposition:                                 |
   * *------------------------------------------------*
   * | kDefinition:                                   |
   * *------------------------------------------------*
   * | kCantonese:  | kMandarin:  | kTang: | kKorean: |
   * *------------------------------------------------*
   * | kJapaneseKun:              | kJapaneseOn:      |
   * *------------------------------------------------*
   */

  gtk_table_attach_defaults (GTK_TABLE (table), charmap->caption->codepoint,
                             0, 1, 0, 1);
  gtk_table_attach_defaults (GTK_TABLE (table), charmap->caption->character,
                             1, 2, 0, 1);
  gtk_table_attach_defaults (GTK_TABLE (table), charmap->caption->category,
                             2, 4, 0, 1);

  gtk_table_attach_defaults (GTK_TABLE (table), charmap->caption->name,
                             0, 4, 1, 2);

  gtk_table_attach_defaults (GTK_TABLE (table), 
                             charmap->caption->decomposition, 
                             0, 4, 2, 3);

  gtk_table_attach_defaults (GTK_TABLE (table), charmap->caption->kDefinition,
                             0, 4, 3, 4);

  gtk_table_attach_defaults (GTK_TABLE (table), charmap->caption->kCantonese,
                             0, 1, 4, 5);
  gtk_table_attach_defaults (GTK_TABLE (table), charmap->caption->kMandarin,
                             1, 2, 4, 5);
  gtk_table_attach_defaults (GTK_TABLE (table), charmap->caption->kTang,
                             2, 3, 4, 5);
  gtk_table_attach_defaults (GTK_TABLE (table), charmap->caption->kKorean,
                             3, 4, 4, 5);

  gtk_table_attach_defaults (GTK_TABLE (table), charmap->caption->kJapaneseKun,
                             0, 2, 5, 6);
  gtk_table_attach_defaults (GTK_TABLE (table), charmap->caption->kJapaneseOn,
                             2, 4, 5, 6);


  scrolled_window = gtk_scrolled_window_new (NULL, NULL);
  gtk_scrolled_window_add_with_viewport (GTK_SCROLLED_WINDOW (scrolled_window),
                                         table);
  gtk_scrolled_window_set_policy  (GTK_SCROLLED_WINDOW (scrolled_window), 
                                   GTK_POLICY_AUTOMATIC, GTK_POLICY_NEVER);

  return scrolled_window;
}


static void       
selection_text_received (GtkClipboard *clipboard, 
                         const gchar *text,
                         gpointer data)
{
  gunichar uc;
  gunichar old_page_first_char, old_active_char;
  Charmap *charmap;

  if (text == NULL)
    return;

  charmap = CHARMAP (data);
  old_page_first_char = charmap->page_first_char;
  old_active_char = charmap->active_char;

  uc = g_utf8_get_char_validated (text, -1);

  g_return_if_fail (uc != (gunichar)(-2) && uc != (gunichar)(-1));

  set_active_character (charmap, uc);
  redraw (charmap);
}


static void
identify_clipboard (GtkWidget *widget,
                    Charmap *charmap)
{
  GtkClipboard *clipboard;
  clipboard = gtk_clipboard_get (GDK_SELECTION_CLIPBOARD);
  gtk_clipboard_request_text (clipboard, selection_text_received, charmap);
}

static void
identify_primary (GtkWidget *widget,
                  Charmap *charmap)
{
  GtkClipboard *clipboard;
  clipboard = gtk_clipboard_get (GDK_SELECTION_PRIMARY);
  gtk_clipboard_request_text (clipboard, selection_text_received, charmap);
}


static GtkWidget *
make_paste_button (Charmap *charmap)
{
  GtkWidget *button;
  GtkWidget *frame;
  GtkWidget *vbox;

  vbox = gtk_vbox_new (FALSE, 2);

  button = gtk_button_new_with_label ("identify character from selection");
  g_signal_connect (G_OBJECT (button), "clicked",
                    G_CALLBACK (identify_primary), charmap);
  gtk_box_pack_start (GTK_BOX (vbox), button, FALSE, FALSE, 0);

  button = gtk_button_new_with_label ("identify character from clipboard");
  g_signal_connect (G_OBJECT (button), "clicked",
                    G_CALLBACK (identify_clipboard), charmap);
  gtk_box_pack_start (GTK_BOX (vbox), button, FALSE, FALSE, 0);

  frame = gtk_frame_new (NULL);
  gtk_container_add (GTK_CONTAINER (frame), vbox);

  return frame;
}


static GtkWidget *
make_text_to_copy (Charmap *charmap)
{
  GtkWidget *hbox;
  GtkWidget *frame;
  GtkWidget *button;
  GtkWidget *label;

  hbox = gtk_hbox_new (FALSE, 5);

  label = gtk_label_new ("text to copy:");
  gtk_box_pack_start (GTK_BOX (hbox), label, FALSE, FALSE, 0);

  charmap->text_to_copy = gtk_entry_new ();
  gtk_entry_set_max_length (GTK_ENTRY (charmap->text_to_copy), 
                            TEXT_TO_COPY_MAXLENGTH);
  gtk_box_pack_start (GTK_BOX (hbox), charmap->text_to_copy, TRUE, TRUE, 0);

  /* the copy button */
  button = gtk_button_new_from_stock (GTK_STOCK_COPY); 
  g_signal_connect (G_OBJECT (button), "clicked",
                    G_CALLBACK (copy_button_clicked), charmap);
  gtk_box_pack_start (GTK_BOX (hbox), button, FALSE, FALSE, 0);

  /* the clear button */
  button = gtk_button_new_from_stock (GTK_STOCK_CLEAR);
  g_signal_connect (G_OBJECT (button), "clicked",
                    G_CALLBACK (clear_button_clicked), charmap);
  gtk_box_pack_start (GTK_BOX (hbox), button, FALSE, FALSE, 0);

  /* put the text_to_copy stuff in a frame */
  frame = gtk_frame_new (NULL);
  gtk_container_add (GTK_CONTAINER (frame), hbox);

  return frame;
}


/* XXX: should adjust by row, not active character */
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
  if (charmap->tabulus != NULL && charmap->tabulus_pixmap != NULL)
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
gboolean    
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
    }

  return TRUE;
}


static void
style_set (GtkWidget *widget, GtkStyle *previous_style, Charmap *charmap)
{
  if (charmap->tabulus_pixmap != NULL)
    g_object_unref (charmap->tabulus_pixmap);
  charmap->tabulus_pixmap = NULL;

  gtk_widget_queue_draw (charmap->tabulus);
}


static void
size_allocate (GtkWidget *widget, GtkAllocation *allocation, Charmap *charmap)
{
  gint old_rows, old_cols;
  GtkAdjustment *adjustment;

  old_rows = charmap->rows;
  old_cols = charmap->cols;

  charmap->cols = (allocation->width - 1)
                  / (calculate_square_dimension_x (charmap->font_metrics) + 1);
  charmap->rows = (allocation->height - 1)
                  / (calculate_square_dimension_y (charmap->font_metrics) + 1);

  if (charmap->rows == old_rows && charmap->cols == old_cols)
    return;

  charmap->page_first_char = charmap->active_char 
                             - (charmap->active_char % charmap->cols);

  /* force pixmap to be redrawn on next expose event */
  if (charmap->tabulus_pixmap != NULL)
    g_object_unref (charmap->tabulus_pixmap);
  charmap->tabulus_pixmap = NULL;

  /* adjust the adjustment, since it's based on the size of a row */
  adjustment = GTK_ADJUSTMENT (charmap->adjustment);
  adjustment->upper = 1.0 * UNICHAR_MAX / charmap->cols;
  adjustment->page_increment = 3.0 * charmap->rows;
  gtk_adjustment_changed (adjustment);
  set_scrollbar_adjustment (charmap);
}


void
charmap_class_init (CharmapClass *clazz)
{
}


/* does all the initial construction */
void
charmap_init (Charmap *charmap)
{
  GtkWidget *hbox;
  GtkWidget *vbox;

  charmap->rows = CHARMAP_MIN_ROWS;
  charmap->cols = CHARMAP_MIN_COLS;

  gtk_box_set_spacing (GTK_BOX (charmap), 5);

  charmap->tabulus = gtk_drawing_area_new ();

  gtk_widget_set_events (charmap->tabulus, GDK_EXPOSURE_MASK 
                                           | GDK_KEY_PRESS_MASK
                                           | GDK_BUTTON_PRESS_MASK
                                           | GDK_FOCUS_CHANGE_MASK
                                           | GDK_SCROLL_MASK);

  g_signal_connect (G_OBJECT (charmap->tabulus), "expose_event",
                    G_CALLBACK (expose_event), charmap);
  g_signal_connect (G_OBJECT (charmap->tabulus), "key_press_event",
                    G_CALLBACK (key_press_event), charmap);
  g_signal_connect (G_OBJECT (charmap->tabulus), "button_press_event",
                    G_CALLBACK (button_press_event), charmap);
  g_signal_connect (G_OBJECT (charmap->tabulus), "focus-in-event",
                    G_CALLBACK (focus_in_or_out_event), charmap);
  g_signal_connect (G_OBJECT (charmap->tabulus), "focus-out-event",
                    G_CALLBACK (focus_in_or_out_event), charmap);
  g_signal_connect (G_OBJECT (charmap->tabulus), "scroll-event",
                    G_CALLBACK (mouse_wheel_event), charmap);
  g_signal_connect (G_OBJECT (charmap->tabulus), "style-set",
                    G_CALLBACK (style_set), charmap);
  g_signal_connect (G_OBJECT (charmap->tabulus), "size-allocate",
                    G_CALLBACK (size_allocate), charmap);

  /* this is required to get key_press events */
  GTK_WIDGET_SET_FLAGS (charmap->tabulus, GTK_CAN_FOCUS);
  gtk_widget_grab_focus (charmap->tabulus);

  hbox = gtk_hbox_new (FALSE, 3);

  gtk_box_pack_start (GTK_BOX (charmap), hbox, TRUE, TRUE, 0);

  gtk_box_pack_start (GTK_BOX (hbox), charmap->tabulus, TRUE, TRUE, 0);
  gtk_box_pack_start (GTK_BOX (hbox), make_scrollbar (charmap), 
                      FALSE, FALSE, 0);

  vbox = gtk_vbox_new (FALSE, 3);
  gtk_box_pack_start (GTK_BOX (vbox), make_unicode_block_selector (charmap), 
                      TRUE, TRUE, 0);
  gtk_box_pack_start (GTK_BOX (vbox), make_paste_button (charmap), 
                      FALSE, FALSE, 0);
  gtk_box_pack_start (GTK_BOX (hbox), vbox, FALSE, FALSE, 0);


  charmap->font_name = NULL;

  charmap->font_metrics = pango_context_get_metrics (
          gtk_widget_get_pango_context (charmap->tabulus),
          charmap->tabulus->style->font_desc, NULL);

  charmap->pango_layout = pango_layout_new (
          gtk_widget_get_pango_context (charmap->tabulus));

  pango_layout_set_font_description (charmap->pango_layout,
                                     charmap->tabulus->style->font_desc);

  /* size the drawing area */
  gtk_widget_set_size_request (
          charmap->tabulus, 
          calculate_tabulus_dimension_x (charmap),
          calculate_tabulus_dimension_y (charmap));

  charmap->page_first_char = (gunichar) 0x0000;
  charmap->active_char = (gunichar) 0x0000;

  /* the caption */
  gtk_box_pack_start (GTK_BOX (charmap), make_caption (charmap), 
                      FALSE, FALSE, 0);

  /* the text_to_copy */
  gtk_box_pack_start (GTK_BOX (charmap), make_text_to_copy (charmap), 
                      FALSE, FALSE, 0);

  set_caption (charmap);
  set_active_block (charmap);
  set_scrollbar_adjustment (charmap);
}


GtkWidget *
charmap_new ()
{
  return GTK_WIDGET (g_object_new (charmap_get_type (), NULL));
}


GtkType
charmap_get_type ()
{
  static GtkType charmap_type = 0;

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
  gtk_widget_ensure_style (charmap->tabulus);
  gtk_widget_modify_font (charmap->tabulus, font_desc);

  charmap->font_metrics = pango_context_get_metrics (
          gtk_widget_get_pango_context (charmap->tabulus),
          charmap->tabulus->style->font_desc, NULL);

  /* new pango layout for the new font */
  g_object_unref (charmap->pango_layout);
  charmap->pango_layout = pango_layout_new (
          gtk_widget_get_pango_context (charmap->tabulus));

  pango_layout_set_font_description (charmap->pango_layout,
                                     charmap->tabulus->style->font_desc);

  pango_font_description_free (font_desc);

  /* force pixmap to be redrawn on next expose event */
  if (charmap->tabulus_pixmap != NULL)
    g_object_unref (charmap->tabulus_pixmap);
  charmap->tabulus_pixmap = NULL;
}


/* call this to let the charmap set window geometry hints; this will make
 * resizing snap nicely to rows and columns; should do this when changing
 * the font as well as at initialization */
void 
charmap_set_geometry_hints (Charmap *charmap, GtkWindow *window)
{
  GdkGeometry hints;

  hints.width_inc = calculate_square_dimension_x (charmap->font_metrics) + 1;
  hints.height_inc = calculate_square_dimension_y (charmap->font_metrics) + 1;

  hints.min_width = hints.width_inc * CHARMAP_MIN_COLS + 1;
  hints.min_height = hints.height_inc * CHARMAP_MIN_ROWS + 1;

  hints.base_width = 1;
  hints.base_height = 1;

  gtk_window_set_geometry_hints (window, charmap->tabulus, &hints,
          GDK_HINT_RESIZE_INC | GDK_HINT_MIN_SIZE | GDK_HINT_BASE_SIZE);
}

