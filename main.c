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


static GtkWidget *charmap;


static void
fontsel_changed (GtkTreeSelection *selection, gpointer data)
{
  gchar *new_font;
  
  new_font = gtk_font_selection_get_font_name (GTK_FONT_SELECTION (data));
  charmap_set_font (CHARMAP (charmap), new_font);
}


static void
toggle_fontsel (GtkToggleButton *togglebutton, gpointer *fontsel)
{
  GtkWidget *toplevel;
  gint width, height;
  GtkRequisition requisition;

  toplevel = gtk_widget_get_toplevel (GTK_WIDGET (togglebutton));
  gtk_window_get_size (GTK_WINDOW (toplevel), &width, &height);

  gtk_widget_size_request (GTK_WIDGET (fontsel), &requisition);

  if (gtk_toggle_button_get_active (togglebutton))
    {
      g_printerr ("toggle_fontsel: %dx%d -> %dx%d\n", width, height,
                   width, height + requisition.height);
      gtk_widget_show (GTK_WIDGET (fontsel));
      gtk_window_resize (GTK_WINDOW (toplevel), width, 
                         height + requisition.height);
    }
  else
    {
      g_printerr ("toggle_fontsel: %dx%d -> %dx%d\n", width, height,
                   width, height - requisition.height);

      gtk_widget_hide (GTK_WIDGET (fontsel));
      gtk_window_resize (GTK_WINDOW (toplevel), width, 
                         height - requisition.height);
    }
}


gint
main (gint argc, gchar **argv)
{
  GtkWidget *window = NULL;
  GtkWidget *vbox;
  GtkWidget *fontsel;
  GtkWidget *fontsel_toggle;

  gtk_init (&argc, &argv);

  window = gtk_window_new (GTK_WINDOW_TOPLEVEL);
  gtk_window_set_title (GTK_WINDOW (window), "Unicode Character Map");

  g_signal_connect (G_OBJECT (window), "destroy",
                    G_CALLBACK (gtk_main_quit), NULL);

  gtk_container_set_border_width (GTK_CONTAINER (window), 5);

  vbox = gtk_vbox_new (FALSE, 5);
  gtk_container_add (GTK_CONTAINER (window), vbox);

  charmap = charmap_new ();
  gtk_box_pack_start (GTK_BOX (vbox), charmap, TRUE, TRUE, 0);

  fontsel_toggle = gtk_toggle_button_new_with_label (
          "show font selection");
  gtk_box_pack_start (GTK_BOX (vbox), fontsel_toggle, FALSE, FALSE, 0);
  gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON (fontsel_toggle), TRUE);

  fontsel = gtk_font_selection_new ();
  gtk_box_pack_start (GTK_BOX (vbox), fontsel, FALSE, FALSE, 0);

  g_signal_connect (G_OBJECT (fontsel_toggle), "toggled",
                    G_CALLBACK (toggle_fontsel), fontsel);

  g_signal_connect (
          gtk_tree_view_get_selection (GTK_TREE_VIEW (
                  GTK_FONT_SELECTION (fontsel)->size_list)), 
          "changed", G_CALLBACK (fontsel_changed), fontsel);


  gtk_widget_show_all (window);

  gtk_main ();

  return 0;
}
