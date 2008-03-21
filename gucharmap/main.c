/* $Id$ */
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

#include <config.h>

#include <stdlib.h>

#include <glib/gi18n.h>
#include <gtk/gtk.h>

#include <gucharmap/gucharmap.h>
#include "gucharmap-settings.h"

gint
main (gint argc, gchar **argv)
{
  GtkWidget *window;
  GdkScreen *screen;
  gint monitor;
  GdkRectangle rect;
  GError *error = NULL;
  gchar *new_font = NULL;
  GOptionEntry goptions[] =
  {
    { "font", 0, 0, G_OPTION_ARG_STRING, &new_font,
      N_("Font to start with; ex: 'Serif 27'"), N_("FONT") },
    { NULL }
  };

  bindtextdomain (GETTEXT_PACKAGE, LOCALEDIR);
  bind_textdomain_codeset (GETTEXT_PACKAGE, "UTF-8");
  textdomain (GETTEXT_PACKAGE);

  if (!gtk_init_with_args (&argc, &argv, "", goptions, GETTEXT_PACKAGE, &error))
    {
      g_printerr ("%s\n", error->message);
      g_error_free (error);

      exit (1);
    }

  gucharmap_init ();
  g_set_application_name (_("Gucharmap"));
  gtk_window_set_default_icon_name ("gucharmap");

  window = gucharmap_window_new ();
  gucharmap_window_set_text_to_copy_visible (GUCHARMAP_WINDOW (window), TRUE);
  gucharmap_window_set_font_selection_visible (GUCHARMAP_WINDOW (window), TRUE);
  gucharmap_window_set_file_menu_visible (GUCHARMAP_WINDOW (window), TRUE);

  screen = gtk_window_get_screen (GTK_WINDOW (window));
  monitor = gdk_screen_get_monitor_at_point (screen, 0, 0);
  gdk_screen_get_monitor_geometry (screen, monitor, &rect);
  gtk_window_set_default_size (GTK_WINDOW (window), rect.width * 9/16, rect.height * 9/16);

  gucharmap_settings_add_window (GTK_WINDOW (window));

  /* make the starting font 50% bigger than the default font */
  if (new_font == NULL) /* new_font could be set by command line option */
    {
      GucharmapMiniFontSelection *fontsel = gucharmap_window_get_mini_font_selection (GUCHARMAP_WINDOW (window));
      gint default_size = PANGO_PIXELS (2.0 * pango_font_description_get_size (window->style->font_desc));
      gucharmap_mini_font_selection_set_default_font_size (fontsel, default_size);

      new_font = gucharmap_settings_get_font ();
      if (new_font)
        {
          PangoFontDescription *fd = pango_font_description_from_string (new_font);

          /* revert to default font size */
          if (0 == pango_font_description_get_size (fd))
            gucharmap_mini_font_selection_reset_font_size (fontsel);
	  else
	    gucharmap_mini_font_selection_set_font_name (fontsel, new_font);

          pango_font_description_free (fd);
	  g_free (new_font);
        }
      else
        gucharmap_mini_font_selection_reset_font_size (fontsel);
    }

  g_signal_connect (G_OBJECT (window), "destroy", G_CALLBACK (gtk_main_quit), NULL);

  gtk_widget_show (window);

  gtk_main ();

  gucharmap_shutdown ();

  return 0;
}
