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
#include <stdlib.h>
#if HAVE_GNOME
# include <gnome.h>
# include <locale.h> /* we call setlocale(3) */
#endif
#if !HAVE_GNOME
# include <popt.h>
#endif
#include <gucharmap/charmap.h>
#include <gucharmap/gucharmap_intl.h>
#include <gucharmap/gucharmap_window.h>
#include <gucharmap/mini_fontsel.h>


static gchar *new_font = NULL;
static struct poptOption options[] = 
{ 
  { "font", '\0', POPT_ARG_STRING, &new_font, 0, 
    N_("Font to start with; ex: 'Serif 27'"), NULL },
#if !HAVE_GNOME
  POPT_AUTOHELP  /* gnome does this automatically */
#endif
  { NULL, '\0', 0, NULL, 0 }
};


gint
main (gint argc, gchar **argv)
{
  GtkWidget *window;
  gchar *orig_font = NULL;
  PangoFontDescription *font_desc = NULL;
  GdkScreen *screen;
#if !HAVE_GNOME
  poptContext popt_context;
  gint rc;
#endif  /* #if !HAVE_GNOME */

#if !HAVE_GNOME
  gtk_init (&argc, &argv);
#else 
  setlocale (LC_ALL, "");
#endif

  /* translate --help message */
  options[0].descrip = _(options[0].descrip);

#if HAVE_GNOME
  gnome_program_init ("gucharmap", VERSION, LIBGNOMEUI_MODULE, argc, argv,
                      GNOME_PARAM_POPT_TABLE, options, NULL);
#else
  popt_context = poptGetContext ("gucharmap", argc, (const gchar **) argv, 
                                 options, 0);
  rc = poptGetNextOpt (popt_context);

  if (rc != -1)
    {
       g_printerr ("%s: %s\n", 
                   poptBadOption (popt_context, POPT_BADOPTION_NOALIAS),
                   poptStrerror (rc));

       exit (1);
    }
#endif  /* else (#if HAVE_GNOME) */

  window = gucharmap_window_new ();
  gucharmap_window_set_text_to_copy_visible (GUCHARMAP_WINDOW (window), TRUE);
  gucharmap_window_set_font_selection_visible (GUCHARMAP_WINDOW (window), TRUE);

  screen = gtk_window_get_screen (GTK_WINDOW (window));
  gtk_window_set_default_size (GTK_WINDOW (window), 
                               gdk_screen_get_width (screen) * 1/2,
                               gdk_screen_get_height (screen) * 9/16);


  /* make the starting font 50% bigger than the default font */
  if (new_font == NULL) /* new_font could be set by command line option */
    {
      orig_font = mini_font_selection_get_font_name (
              MINI_FONT_SELECTION (GUCHARMAP_WINDOW (window)->fontsel));

      font_desc = pango_font_description_from_string (orig_font);
      pango_font_description_set_size (
              font_desc, pango_font_description_get_size (font_desc) * 3/2);
      new_font = pango_font_description_to_string (font_desc);
    }
  /* this sends the changed signal: */
  mini_font_selection_set_font_name (
          MINI_FONT_SELECTION (GUCHARMAP_WINDOW (window)->fontsel), new_font);

  g_signal_connect (G_OBJECT (window), "destroy", 
                    G_CALLBACK (gtk_main_quit), NULL);

  chartable_grab_focus (GUCHARMAP_WINDOW (window)->charmap->chartable);
  gtk_widget_show_all (window);

  gtk_main ();

  return 0;
}

