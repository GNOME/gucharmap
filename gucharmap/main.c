/*
 * Copyright © 2004 Noah Levitt
 * Copyright © 2007, 2008 Christian Persch
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
 * 59 Temple Place, Suite 330, Boston, MA 02110-1301  USA
 */

#include <config.h>

#include <stdlib.h>

#include <glib/gi18n.h>
#include <gtk/gtk.h>

#include <gucharmap/gucharmap.h>
#include "gucharmap-window.h"

#define UI_RESOURCE "/org/gnome/charmap/gucharmap-menus.ui"
 
static gboolean
option_version_cb (const gchar *option_name,
                   const gchar *value,
                   gpointer     data,
                   GError     **error)
{
  g_print ("%s %s\n", _("GNOME Character Map"), VERSION);

  exit (EXIT_SUCCESS);
  return FALSE;
}

static void
startup_cb (GApplication *application,
            gpointer      data)
{
  GtkBuilder *builder = gtk_builder_new ();
  GMenuModel *model;

  gtk_builder_add_from_resource (builder, UI_RESOURCE, NULL);

#ifdef ENABLE_PRINTING
  model = G_MENU_MODEL (gtk_builder_get_object (builder, "printing"));

  g_menu_append (G_MENU (model), _("Page _Setup"), "win.page-setup");
/* g_menu_append (G_MENU (model), _("Print Preview"), "win.print-preview"); */
  g_menu_append (G_MENU (model), _("_Print"), "win.print");
#endif

  model = G_MENU_MODEL (gtk_builder_get_object (builder, "go-chapter"));
  g_object_set_data (G_OBJECT (application), "go-chapter-menu", model);

  model = G_MENU_MODEL (gtk_builder_get_object (builder, "menubar"));
  gtk_application_set_menubar (GTK_APPLICATION (application), model);

  gtk_application_add_accelerator (GTK_APPLICATION (application),
                                   "<Primary>Page_Down", "win.next-chapter",
                                   NULL);
  gtk_application_add_accelerator (GTK_APPLICATION (application),
                                   "<Primary>Page_Up", "win.previous-chapter",
                                   NULL);

  g_object_unref (builder);
}

static void
gucharmap_activate (GApplication *application,
                    gpointer      unused)
{
  GList *windows = gtk_application_get_windows (GTK_APPLICATION (application));
  gtk_window_present (GTK_WINDOW (windows->data));
}

int
main (int argc, char **argv)
{
  GtkWidget *window;
  GdkScreen *screen;
  int monitor;
  GdkRectangle rect;
  GError *error = NULL;
  char *font = NULL;
  GtkApplication *application;
  guint status;
  GOptionEntry goptions[] =
  {
    { "font", 0, 0, G_OPTION_ARG_STRING, &font,
      N_("Font to start with; ex: 'Serif 27'"), N_("FONT") },
    { "version", 0, G_OPTION_FLAG_HIDDEN | G_OPTION_FLAG_NO_ARG, 
      G_OPTION_ARG_CALLBACK, option_version_cb, NULL, NULL },
    { NULL }
  };

  bindtextdomain (GETTEXT_PACKAGE, LOCALEDIR);
  bind_textdomain_codeset (GETTEXT_PACKAGE, "UTF-8");
  textdomain (GETTEXT_PACKAGE);

#ifdef HAVE_GCONF
  /* GConf uses ORBit2 which need GThread. See bug #565516 */
  g_thread_init (NULL);
#endif

  /* Set programme name explicitly (see bug #653115) */
  g_set_prgname("gucharmap");

  if (!gtk_init_with_args (&argc, &argv, "", goptions, GETTEXT_PACKAGE, &error))
    {
      g_printerr ("%s\n", error->message);
      g_error_free (error);

      exit (1);
    }

  g_set_application_name (_("Character Map"));
  gtk_window_set_default_icon_name (GUCHARMAP_ICON_NAME);

  application = gtk_application_new ("org.gnome.Charmap",
                                     G_APPLICATION_NON_UNIQUE);
  g_signal_connect (application, "startup", G_CALLBACK (startup_cb), NULL);
  g_signal_connect (application, "activate",
                    G_CALLBACK (gucharmap_activate), NULL);

  g_application_register (G_APPLICATION (application), NULL, NULL);

  window = gucharmap_window_new (application);

  screen = gtk_window_get_screen (GTK_WINDOW (window));
  monitor = gdk_screen_get_monitor_at_point (screen, 0, 0);
#if GTK_CHECK_VERSION (3, 3, 5)
  gdk_screen_get_monitor_workarea (screen, monitor, &rect);
#else
  gdk_screen_get_monitor_geometry (screen, monitor, &rect);
#endif
  gtk_window_set_default_size (GTK_WINDOW (window), rect.width * 9/16, rect.height * 9/16);

  if (font)
    {
      gucharmap_window_set_font (GUCHARMAP_WINDOW (window), font);
      g_free (font);
    }

  gtk_window_present (GTK_WINDOW (window));

  status = g_application_run (G_APPLICATION (application), argc, argv);
  g_object_unref (application);

  return status;
}
