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
#include "tabulus.h"

#if 0
static GtkWidget *charmap;

void
fontsel_changed_cb (GtkTreeSelection *selection, gpointer data)
{
  GtkWidget *fontsel = GTK_WIDGET (data);
  gchar *newfont = gtk_font_selection_get_font_name (GTK_FONT_SELECTION (fontsel));
  charmap_set_font (CHARMAP (charmap), newfont);
}
#endif


int
main (int argc, char **argv)
{
  static GtkWidget *window = NULL;
  GtkWidget *vbox;
  GtkWidget *tabulus;
  GtkWidget *fontsel;

  gtk_init (&argc, &argv);

  window = gtk_window_new (GTK_WINDOW_TOPLEVEL);
  gtk_window_set_title (GTK_WINDOW (window), "Unicode Character Map");

  g_signal_connect (G_OBJECT (window), "destroy",
                    G_CALLBACK (gtk_main_quit), NULL);

  gtk_container_set_border_width (GTK_CONTAINER (window), 5);

  vbox = gtk_vbox_new (FALSE, 0);
  gtk_container_add (GTK_CONTAINER (window), vbox);

  tabulus = tabulus_new (8, 12);
  gtk_box_pack_start (GTK_BOX (vbox), tabulus, TRUE, TRUE, 0);

#if 0
  fontsel = gtk_font_selection_new ();
  g_signal_connect (gtk_tree_view_get_selection (GTK_TREE_VIEW (GTK_FONT_SELECTION (fontsel)->size_list)), 
          "changed", G_CALLBACK (fontsel_changed_cb), fontsel);

  gtk_box_pack_start (GTK_BOX (vbox), fontsel, TRUE, TRUE, 0);
#endif

  gtk_widget_show_all (window);

  gtk_main ();

  return 0;
}
