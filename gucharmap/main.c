/*
 * Copyright © 2004 Noah Levitt
 * Copyright © 2007, 2008 Christian Persch
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
  GError *error = NULL;
  char *font = NULL;
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

  g_set_application_name (_("Character Map"));
  gtk_window_set_default_icon_name (GUCHARMAP_ICON_NAME);

  window = gucharmap_window_new ();
  g_signal_connect (window, "destroy",
                    G_CALLBACK (gtk_main_quit), NULL);

  screen = gtk_window_get_screen (GTK_WINDOW (window));
  monitor = gdk_screen_get_monitor_at_point (screen, 0, 0);
  gdk_screen_get_monitor_geometry (screen, monitor, &rect);
  gtk_window_set_default_size (GTK_WINDOW (window), rect.width * 9/16, rect.height * 9/16);

  /* No --font argument, use the stored font (if any) */
  if (!font)
    font = gucharmap_settings_get_font ();

  if (font)
    {
      gucharmap_window_set_font (GUCHARMAP_WINDOW (window), font);
      g_free (font);
    }

  gucharmap_settings_add_window (GTK_WINDOW (window));

  gtk_window_present (GTK_WINDOW (window));

  gtk_main ();

  gucharmap_settings_shutdown ();

  return 0;
}
