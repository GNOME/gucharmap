/* $Id$ */

#include <gtk/gtk.h>
#include "charmap.h"

static GtkWidget *charmap;

void
fontsel_changed_cb (GtkTreeSelection *selection, gpointer data)
{
  GtkWidget *fontsel = GTK_WIDGET (data);
  gchar *newfont = gtk_font_selection_get_font_name (GTK_FONT_SELECTION (fontsel));
  charmap_set_font (CHARMAP (charmap), newfont);
}


int
main (int argc, char **argv)
{
  static GtkWidget *window = NULL;
  GtkWidget *vbox;
  GtkWidget *fontsel;

  gtk_init (&argc, &argv);

  window = gtk_window_new (GTK_WINDOW_TOPLEVEL);
  gtk_window_set_title (GTK_WINDOW (window), "Unicode Character Map");

  g_signal_connect (G_OBJECT (window), "destroy",
                    G_CALLBACK (gtk_main_quit), NULL);

  gtk_container_set_border_width (GTK_CONTAINER (window), 5);

  vbox = gtk_vbox_new (FALSE, 0);
  gtk_container_add (GTK_CONTAINER (window), vbox);

  charmap = charmap_new ();
  gtk_box_pack_start (GTK_BOX (vbox), charmap, TRUE, TRUE, 0);

  fontsel = gtk_font_selection_new ();
  g_signal_connect (gtk_tree_view_get_selection (GTK_TREE_VIEW (GTK_FONT_SELECTION (fontsel)->size_list)), 
          "changed", G_CALLBACK (fontsel_changed_cb), fontsel);

  gtk_box_pack_start (GTK_BOX (vbox), fontsel, TRUE, TRUE, 0);

  gtk_widget_show_all (window);

  gtk_main ();

  return 0;
}
