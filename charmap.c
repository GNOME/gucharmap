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

#include <stdarg.h>
#include <time.h>
#include <stdio.h>


#if 1
static void
debug (const char *format, ...)
{
  static char timestamp[80];
  va_list arg;
  time_t now;

  now = time (NULL);
  strftime (timestamp, sizeof (timestamp), "%c", localtime (&now));
  fprintf(stderr, "[%s] ", timestamp);

  va_start(arg, format);
  vfprintf(stderr, format, arg);
  va_end(arg);
}
#else
#define debug(x...)
#endif


/* return value is read-only, should not be freed */
static gchar *
unichar_to_printable_utf8 (gunichar uc)
{
  static gchar buf[8];
  gint x;
  
  if (g_unichar_type (uc) == G_UNICODE_COMBINING_MARK
          || g_unichar_type (uc) == G_UNICODE_ENCLOSING_MARK
          || g_unichar_type (uc) == G_UNICODE_NON_SPACING_MARK)
    {
      buf[0] = ' ';
      x = g_unichar_to_utf8 (uc, buf+1);
      buf[x+1] = '\0';
      debug ("%s is a combining character\n", buf);
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
  gchar *escaped_utf8_buf, *escaped_unicode_info;
  gchar *caption_markup;

  escaped_utf8_buf = g_markup_escape_text (
          unichar_to_printable_utf8 (charmap->active_char), -1);
  escaped_unicode_info = g_markup_escape_text (
          get_unicode_info (charmap->active_char), -1);

  /* n.b. the string below has utf8 quotes in it */
  caption_markup = g_strdup_printf (
          "<span size=\"large\">U+%4.4X “%s” %s</span>",
          charmap->active_char, escaped_utf8_buf, escaped_unicode_info);

  gtk_label_set_markup (GTK_LABEL (charmap->caption), caption_markup);

  g_free (caption_markup);
  g_free (escaped_utf8_buf);
  g_free (escaped_unicode_info);
}


/* XXX: may want to have some smartness here to avoid resizing too much */
static gint 
calculate_square_dimension (PangoFontMetrics *font_metrics)
{
  debug ("calculate_square_dimension\n");

  /* XXX: can't get max width for the font, so just use the height */
  return 2 + (pango_font_metrics_get_ascent (font_metrics) 
              + pango_font_metrics_get_descent (font_metrics)) 
             / PANGO_SCALE;
}


/* redraws the backing store pixmap */
static void
draw_tabulus_pixmap (Charmap *charmap)
{
  gint x, y, row, col;
  gint square_width, square_height; 
  gint char_width, char_height;
  gint width, height;
  GdkGC *gc;

  debug ("draw_tabulus_pixmap\n");

  /* width and height of a square */
  square_width = calculate_square_dimension (charmap->font_metrics);
  square_height = calculate_square_dimension (charmap->font_metrics);

  width = (square_width+1) * CHARMAP_COLS + 1;
  height = (square_height+1) * CHARMAP_ROWS + 1;

  /* plain background */
  gdk_draw_rectangle (charmap->tabulus_pixmap,
                      charmap->tabulus->style->base_gc[GTK_STATE_NORMAL], 
                      TRUE, 0, 0, width, height);

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

  pango_layout_set_font_description (charmap->pango_layout,
                                     charmap->tabulus->style->font_desc);

  /* draw the characters */
  for (row = 0;  row < CHARMAP_ROWS;  row++)
    for (col = 0;  col < CHARMAP_COLS;  col++)
      {
        gunichar uc = charmap->page_first_char + row * CHARMAP_COLS + col;

        if (uc == charmap->active_char)
          {
            gc = charmap->tabulus->style->text_gc[GTK_STATE_ACTIVE];
            gdk_draw_rectangle (
                    charmap->tabulus_pixmap,
                    charmap->tabulus->style->base_gc[GTK_STATE_ACTIVE],
                    TRUE, 
                    (square_width+1) * col + 1, (square_height+1) * row + 1, 
                    square_width, square_height);

            /* XXX: seems like an ok place to set the caption, no? */
            set_caption (charmap);
          }
        else 
          gc = charmap->tabulus->style->text_gc[GTK_STATE_NORMAL];

        if (! g_unichar_isgraph (uc))
          continue;

        pango_layout_set_text (charmap->pango_layout, 
                               unichar_to_printable_utf8 (uc), 
                               -1);

        pango_layout_get_pixel_size (charmap->pango_layout, 
                                     &char_width, &char_height);

        gdk_draw_layout (
                charmap->tabulus_pixmap, gc,
                (square_width+1) * col + (square_width - char_width) / 2, 
                (square_height+1) * row + (square_height - char_height) / 2, 
                charmap->pango_layout);
      }

}


/* redraws the screen from the backing pixmap */
static gint
expose_event (GtkWidget *widget, 
              GdkEventExpose *event, 
              gpointer callback_data)
{
  Charmap *charmap;

  debug ("expose_event\n");

  charmap = CHARMAP (callback_data);

  if (charmap->tabulus_pixmap == NULL)
    {
      charmap->tabulus_pixmap = gdk_pixmap_new (
              charmap->tabulus->window, charmap->tabulus->allocation.width,
              charmap->tabulus->allocation.height, -1);
      draw_tabulus_pixmap (charmap);
    }

  gdk_draw_drawable (charmap->tabulus->window,
                     widget->style->fg_gc[GTK_STATE_NORMAL],
                     charmap->tabulus_pixmap,
                     event->area.x, event->area.y,
                     event->area.x, event->area.y,
                     event->area.width, event->area.height);

  return FALSE;
}


/* draws the square that this character is in */
static void
expose_char_for_redraw (Charmap *charmap, gunichar uc)
{
  gint row, col, x, y, dim;

  g_assert (uc >= charmap->page_first_char);
  g_assert (uc < charmap->page_first_char + CHARMAP_ROWS * CHARMAP_COLS);

  row = (uc - charmap->page_first_char) / CHARMAP_COLS;
  col = uc % CHARMAP_ROWS;

  dim = calculate_square_dimension (charmap->font_metrics);

  x = col * (dim + 1) + 1;
  y = row * (dim + 1) + 1;

  gdk_draw_drawable (charmap->tabulus->window,
                     charmap->tabulus->style->fg_gc[GTK_STATE_NORMAL],
                     charmap->tabulus_pixmap,
                     x, y, x, y, dim, dim);
  /* gtk_widget_queue_draw_area (charmap->tabulus, x, y, dim, dim); */
}


/* for moving around in the charmap */
/* XXX: redrawing could be much more efficient */
static gint
key_press_event (GtkWidget *widget, 
                 GdkEventKey *event, 
                 gpointer callback_data)
{
  Charmap *charmap;
  gunichar old_active_char;
  gunichar old_page_first_char;

  debug ("key_press_event\n");

  charmap = CHARMAP (callback_data);
  old_active_char = charmap->active_char;

  /* move the cursor depending on which key was pressed */
  switch (event->keyval)
    {
      case GDK_Home: case GDK_KP_Home:
        charmap->active_char = 0x0000;
        break;

      case GDK_End: case GDK_KP_End:
        charmap->active_char = UNICHAR_MAX;
        break;

      case GDK_Up: case GDK_KP_Up: case GDK_k:
        if (charmap->active_char >= CHARMAP_COLS)
          charmap->active_char -= CHARMAP_COLS;
        break;

      case GDK_Down: case GDK_KP_Down: case GDK_j:
        if (charmap->active_char <= UNICHAR_MAX - CHARMAP_COLS)
          charmap->active_char += CHARMAP_COLS;
        break;

      case GDK_Left: case GDK_KP_Left: case GDK_h:
        if (charmap->active_char > 0)
          charmap->active_char--;
        break;

      case GDK_Right: case GDK_KP_Right: case GDK_l:
        if (charmap->active_char < UNICHAR_MAX)
          charmap->active_char++;
        break;

      case GDK_Page_Up: case GDK_b: case GDK_minus:
        if (charmap->active_char >= CHARMAP_COLS * CHARMAP_ROWS)
          charmap->active_char -= CHARMAP_COLS * CHARMAP_ROWS;
        else if (charmap->active_char > 0)
          charmap->active_char = 0;
        break;

      case GDK_Page_Down: case GDK_space:
        if (charmap->active_char < UNICHAR_MAX - CHARMAP_COLS * CHARMAP_ROWS)
          charmap->active_char += CHARMAP_COLS * CHARMAP_ROWS;
        else if (charmap->active_char < UNICHAR_MAX)
          charmap->active_char = UNICHAR_MAX;
        break;

      default:
        return FALSE; /* don't redraw */
    }

  old_page_first_char = charmap->page_first_char;

  /* move to the page with this active char */
  charmap->page_first_char 
      = charmap->active_char - (charmap->active_char % 
                                (CHARMAP_COLS * CHARMAP_ROWS));

  if (charmap->page_first_char != old_page_first_char)
    {
      draw_tabulus_pixmap (charmap);
      gtk_widget_queue_draw (GTK_WIDGET (charmap->tabulus));
    }
  else if (charmap->active_char != old_active_char)
    {
      draw_tabulus_pixmap (charmap);

      expose_char_for_redraw (charmap, charmap->active_char);
      expose_char_for_redraw (charmap, old_active_char);
    }

  return FALSE;
}


static gunichar
get_char_at (Charmap *charmap, gint x, gint y)
{
  gint row, col, dim;

  dim = calculate_square_dimension (charmap->font_metrics);

  row = y / (dim + 1);
  if (row >= CHARMAP_ROWS)
    row = CHARMAP_ROWS - 1;

  col = x / (dim + 1);
  if (col >= CHARMAP_COLS)
    col = CHARMAP_COLS - 1;

  return charmap->page_first_char + row * CHARMAP_COLS + col;
}


static gint
button_press_event (GtkWidget *widget, 
                    GdkEventButton *event, 
                    gpointer callback_data)
{
  Charmap *charmap = CHARMAP (callback_data);
  gunichar old_active_char;

  old_active_char = charmap->active_char;
  charmap->active_char = get_char_at (charmap, event->x, event->y);

  if (charmap->active_char != old_active_char)
    {
      draw_tabulus_pixmap (charmap);
    
      expose_char_for_redraw (charmap, charmap->active_char);
      expose_char_for_redraw (charmap, old_active_char);
    }

  /* in case we lost keyboard focus and are clicking to get it back */
  gtk_widget_grab_focus (charmap->tabulus);

  return FALSE;
}


void
charmap_class_init (CharmapClass *clazz)
{
  debug ("charmap_class_init\n");
}


void
charmap_init (Charmap *charmap)
{
  gint square_dimension;

  debug ("charmap_init\n");

  charmap->tabulus = gtk_drawing_area_new ();

  gtk_widget_set_events (charmap->tabulus, GDK_EXPOSURE_MASK 
                                           | GDK_KEY_PRESS_MASK
                                           | GDK_BUTTON_PRESS_MASK);

  g_signal_connect (G_OBJECT (charmap->tabulus), "expose_event",
                    G_CALLBACK (expose_event), charmap);
  g_signal_connect (G_OBJECT (charmap->tabulus), "key_press_event",
                    G_CALLBACK (key_press_event), charmap);
  g_signal_connect (G_OBJECT (charmap->tabulus), "button_press_event",
                    G_CALLBACK (button_press_event), charmap);

  /* this is required to get key_press events (XXX: i think) */
  GTK_WIDGET_SET_FLAGS (charmap->tabulus, GTK_CAN_FOCUS);
  gtk_widget_grab_focus (charmap->tabulus);

  gtk_box_pack_start (GTK_BOX (charmap), charmap->tabulus, TRUE, TRUE, 0);

  charmap->font_name = pango_font_description_to_string (
          charmap->tabulus->style->font_desc);

  charmap->font_metrics = pango_context_get_metrics (
          gtk_widget_get_pango_context (charmap->tabulus),
          charmap->tabulus->style->font_desc, NULL);

  charmap->pango_layout = pango_layout_new (
          gtk_widget_get_pango_context (charmap->tabulus));

  /* size the drawing area - the +1 is for the 1-pixel borders*/
  square_dimension = calculate_square_dimension (charmap->font_metrics);
  gtk_widget_set_size_request (charmap->tabulus, 
                               CHARMAP_COLS * (square_dimension + 1) + 1,
                               CHARMAP_ROWS * (square_dimension + 1) + 1);

  charmap->page_first_char = (gunichar) 0x0000;
  charmap->active_char = (gunichar) 0x0000;

  charmap->caption = gtk_label_new ("");
  gtk_label_set_selectable (GTK_LABEL (charmap->caption), TRUE);
  set_caption (charmap);
  gtk_box_pack_start (GTK_BOX (charmap), charmap->caption, TRUE, TRUE, 0);
}


GtkWidget *
charmap_new ()
{
  debug ("charmap_new\n");
  return GTK_WIDGET (g_object_new (charmap_get_type (), NULL));
}


GtkType
charmap_get_type ()
{
  static GtkType charmap_type = 0;

  debug ("charmap_get_type\n");

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
  gint square_dimension;
  PangoFontDescription *font_desc;

  if (charmap->font_name != NULL
      || g_ascii_strcasecmp (charmap->font_name, font_name) != 0)
    {
      g_free (charmap->font_name);
      charmap->font_name = NULL;
    }
  else
    return;

    {
      charmap->font_name = font_name;
      font_desc = pango_font_description_from_string (font_name);

      gtk_widget_modify_font (charmap->tabulus, font_desc);

      charmap->font_metrics = pango_context_get_metrics (
              gtk_widget_get_pango_context (charmap->tabulus),
              charmap->tabulus->style->font_desc, NULL);

      /* XXX: unref something? */
      charmap->pango_layout = pango_layout_new (
              gtk_widget_get_pango_context (charmap->tabulus));

      /* size the drawing area - the +1 is for the 1-pixel borders*/
      square_dimension = calculate_square_dimension (charmap->font_metrics);
      gtk_widget_set_size_request (charmap->tabulus, 
                                   CHARMAP_COLS * (square_dimension + 1) + 1,
                                   CHARMAP_ROWS * (square_dimension + 1) + 1);

      draw_tabulus_pixmap (charmap);
      gtk_widget_queue_draw (GTK_WIDGET (charmap->tabulus));
    }
}


