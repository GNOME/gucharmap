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
#include "charmap.h"

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
  gint x, y;

  debug ("draw_tabulus_pixmap\n");

  if (charmap->tabulus_pixmap != NULL)
    gdk_pixmap_unref (charmap->tabulus_pixmap);

  charmap->tabulus_pixmap = gdk_pixmap_new (charmap->tabulus->window,
                                            charmap->tabulus->allocation.width,
                                            charmap->tabulus->allocation.height,
                                            -1);

  /* a plain background */
  gdk_draw_rectangle (charmap->tabulus_pixmap,
                      charmap->tabulus->style->base_gc[GTK_STATE_NORMAL], 
                      TRUE, 
                      0, 0, 
                      charmap->tabulus->allocation.width,
                      charmap->tabulus->allocation.height);

  /* vertical lines */
  for (x = charmap->tabulus->allocation.width / CHARMAP_COLS;
       x < charmap->tabulus->allocation.width;
       x += charmap->tabulus->allocation.width / CHARMAP_COLS)
    {
      gdk_draw_line (charmap->tabulus_pixmap,
                     charmap->tabulus->style->fg_gc[GTK_STATE_INSENSITIVE], 
                     x, 0, x, charmap->tabulus->allocation.height - 1);
    }

  /* horizontal lines */
  for (y = charmap->tabulus->allocation.height / CHARMAP_ROWS;
       y < charmap->tabulus->allocation.height;
       y += charmap->tabulus->allocation.height / CHARMAP_ROWS)
    {
      gdk_draw_line (charmap->tabulus_pixmap,
                     charmap->tabulus->style->fg_gc[GTK_STATE_INSENSITIVE], 
                     0, y, charmap->tabulus->allocation.width - 1, y);
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
    draw_tabulus_pixmap (charmap);

  gdk_draw_drawable (charmap->tabulus->window,
                     widget->style->fg_gc[GTK_STATE_NORMAL],
                     charmap->tabulus_pixmap,
                     event->area.x, event->area.y,
                     event->area.x, event->area.y,
                     event->area.width, event->area.height);

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
  g_signal_connect (G_OBJECT (charmap->tabulus), "expose_event",
                    G_CALLBACK (expose_event), charmap);

  gtk_box_pack_start (GTK_BOX (charmap), charmap->tabulus, TRUE, TRUE, 0);

  /* init the font information */
  charmap->font_name = pango_font_description_to_string (
          charmap->tabulus->style->font_desc);

  charmap->font_metrics = pango_context_get_metrics (
          gtk_widget_get_pango_context (charmap->tabulus),
          charmap->tabulus->style->font_desc, NULL);

  /* size the drawing area */
  square_dimension = calculate_square_dimension (charmap->font_metrics);
  gtk_widget_set_size_request (charmap->tabulus, 
                               CHARMAP_COLS * square_dimension,
                               CHARMAP_ROWS * square_dimension);
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

