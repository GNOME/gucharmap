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

#include <config.h>

#include <stdlib.h>

#include <glib/gi18n.h>
#include <gtk/gtk.h>

#include <gucharmap/gucharmap.h>
#include "gucharmap-settings.h"
#include "gucharmap-window.h"

gint
main (gint argc, gchar **argv)
{
  GtkWidget *window;
  GdkScreen *screen;
  gint monitor;
  GdkRectangle rect;
  GucharmapMiniFontSelection *fontsel;
  GError *error = NULL;
  gchar *font = NULL;
  GOptionEntry goptions[] =
  {
    { "font", 0, 0, G_OPTION_ARG_STRING, &font,
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

  gucharmap_settings_initialize ();

  g_set_application_name (_("Gucharmap"));
  gtk_window_set_default_icon_name (GUCHARMAP_ICON_NAME);

  window = gucharmap_window_new ();
  g_signal_connect (window, "destroy",
                    G_CALLBACK (gtk_main_quit), NULL);

  screen = gtk_window_get_screen (GTK_WINDOW (window));
  monitor = gdk_screen_get_monitor_at_point (screen, 0, 0);
  gdk_screen_get_monitor_geometry (screen, monitor, &rect);
  gtk_window_set_default_size (GTK_WINDOW (window), rect.width * 9/16, rect.height * 9/16);

  /* FIXMEchpe: move all this into gucharmap-window */
  /* make the starting font 50% bigger than the default font */
  fontsel = gucharmap_window_get_mini_font_selection (GUCHARMAP_WINDOW (window));
  gucharmap_mini_font_selection_set_default_font_size (fontsel,
                                                       PANGO_PIXELS (2.0 * pango_font_description_get_size (window->style->font_desc)));

  if (font) {
    PangoFontDescription *font_desc;
    int size;

    font_desc = pango_font_description_from_string (font);
    gucharmap_mini_font_selection_set_font_name (fontsel, font);

    size = pango_font_description_get_size (font_desc);
    if (0 != size) {
      gucharmap_mini_font_selection_set_default_font_size (fontsel,
                                                           PANGO_PIXELS (size));
    }

    pango_font_description_free (font_desc);
    g_free (font);
  }

  gucharmap_mini_font_selection_reset_font_size (fontsel);

  gucharmap_settings_add_window (GTK_WINDOW (window));
  gtk_widget_show (window);

  gtk_main ();

  gucharmap_settings_shutdown ();

  return 0;
}
