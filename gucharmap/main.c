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
#include "gucharmap_intl.h"
#include "../pixmaps/gucharmap.xpm"  /* defines gucharmap_xpm */


#define VBOX_SPACING 3


static GtkWidget *charmap;


static void
fontsel_changed (GtkTreeSelection *selection, gpointer data)
{
  gchar *new_font;
  
  new_font = gtk_font_selection_get_font_name (GTK_FONT_SELECTION (data));
  charmap_set_font (CHARMAP (charmap), new_font);
  g_free (new_font);

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
      gtk_widget_show (GTK_WIDGET (fontsel));
      gtk_window_resize (GTK_WINDOW (toplevel), width, 
                         height + requisition.height + VBOX_SPACING);
    }
  else
    {
      gtk_widget_hide (GTK_WIDGET (fontsel));
      gtk_window_resize (GTK_WINDOW (toplevel), width, 
                         height - requisition.height - VBOX_SPACING);
    }
}


static void
expand_collapse (GtkWidget *widget, gpointer data)
{
  g_printerr ("expand_collapse\n");
}


static void
jump_code_point (GtkWidget *widget, gpointer data)
{
  g_printerr ("jump_code_point\n");
}


static void
jump_selection (GtkWidget *widget, gpointer data)
{
  g_printerr ("jump_selection\n");
}


static void
jump_clipboard (GtkWidget *widget, gpointer data)
{
  g_printerr ("jump_clipboard\n");
}


static void
help_about (GtkWidget *widget, gpointer data)
{
  g_printerr ("help_about\n");
}


static void
show_hide_details (GtkWidget *widget, gpointer data)
{
  g_printerr ("show_hide_details\n");
}


static GtkWidget *
make_menu ()
{
  GtkWidget *menubar;
  GtkWidget *file_menu, *view_menu, *actions_menu, *help_menu;
  GtkWidget *file_menu_item, *view_menu_item;
  GtkWidget *actions_menu_item, *help_menu_item;
  GtkWidget *menu_item;

  /* make the menu bar */
  menubar = gtk_menu_bar_new ();
  file_menu_item = gtk_menu_item_new_with_mnemonic (_("_File"));
  gtk_menu_shell_append (GTK_MENU_SHELL (menubar), file_menu_item);
  view_menu_item = gtk_menu_item_new_with_mnemonic (_("_View"));
  gtk_menu_shell_append (GTK_MENU_SHELL (menubar), view_menu_item);
  actions_menu_item = gtk_menu_item_new_with_mnemonic (_("_Actions"));
  gtk_menu_shell_append (GTK_MENU_SHELL (menubar), actions_menu_item);
  help_menu_item = gtk_menu_item_new_with_mnemonic (_("_Help"));
  gtk_menu_shell_append (GTK_MENU_SHELL (menubar), help_menu_item);
  /* finished making the menu bar */

  /* make the file menu */
  file_menu = gtk_menu_new ();
  gtk_menu_item_set_submenu (GTK_MENU_ITEM (file_menu_item),
                             file_menu);
  menu_item = gtk_image_menu_item_new_from_stock (GTK_STOCK_QUIT, NULL);
  g_signal_connect (G_OBJECT (menu_item), "activate",
                    G_CALLBACK (gtk_main_quit), NULL);
  gtk_menu_shell_append (GTK_MENU_SHELL (file_menu), menu_item);
  /* finished making the file menu */

  /* make the view menu */
  view_menu = gtk_menu_new ();
  gtk_menu_item_set_submenu (GTK_MENU_ITEM (view_menu_item), view_menu);

  menu_item = gtk_check_menu_item_new_with_mnemonic (_("Expand/Collapse All"));
  g_signal_connect (G_OBJECT (menu_item), "activate",
                    G_CALLBACK (expand_collapse), NULL);
  gtk_menu_shell_append (GTK_MENU_SHELL (view_menu), menu_item);

  menu_item = gtk_check_menu_item_new_with_mnemonic (_("Show/Hide Details"));
  g_signal_connect (G_OBJECT (menu_item), "activate",
                    G_CALLBACK (show_hide_details), NULL);
  gtk_menu_shell_append (GTK_MENU_SHELL (view_menu), menu_item);
  /* finished making the view menu */

  /* make the actions menu */
  actions_menu = gtk_menu_new ();
  gtk_menu_item_set_submenu (GTK_MENU_ITEM (actions_menu_item), actions_menu);

  menu_item = gtk_menu_item_new_with_mnemonic (_("Jump to _Hex Code Point"));
  g_signal_connect (G_OBJECT (menu_item), "activate",
                    G_CALLBACK (jump_code_point), NULL);
  gtk_menu_shell_append (GTK_MENU_SHELL (actions_menu), menu_item);

  menu_item = gtk_menu_item_new_with_mnemonic (
          _("Jump to First Character in the _Clipboard"));
  g_signal_connect (G_OBJECT (menu_item), "activate",
                    G_CALLBACK (jump_clipboard), NULL);
  gtk_menu_shell_append (GTK_MENU_SHELL (actions_menu), menu_item);

  menu_item = gtk_menu_item_new_with_mnemonic (
          _("Jump to First Character in the _Selected Text"));
  g_signal_connect (G_OBJECT (menu_item), "activate",
                    G_CALLBACK (jump_selection), NULL);
  gtk_menu_shell_append (GTK_MENU_SHELL (actions_menu), menu_item);
  /* finished making the actions menu */

  /* make the help menu */
  help_menu = gtk_menu_new ();
  gtk_menu_item_set_submenu (GTK_MENU_ITEM (help_menu_item), help_menu);

  menu_item = gtk_menu_item_new_with_mnemonic (_("_About"));
  g_signal_connect (G_OBJECT (menu_item), "activate",
                    G_CALLBACK (help_about), NULL);
  gtk_menu_shell_append (GTK_MENU_SHELL (help_menu), menu_item);
  /* finished making the help menu */

  return menubar;
}


gint
main (gint argc, gchar **argv)
{
  GtkWidget *window = NULL;
  GtkWidget *vbox;
  GtkWidget *fontsel;
  GtkWidget *fontsel_toggle;
  GtkWidget *statusbar;
  GtkTooltips *tooltips;
  gchar *orig_font, *new_font;
  PangoFontDescription *font_desc;

  gtk_init (&argc, &argv);


  tooltips = gtk_tooltips_new ();

  window = gtk_window_new (GTK_WINDOW_TOPLEVEL);
  gtk_window_set_title (GTK_WINDOW (window), _("Unicode Character Map"));
  gtk_window_set_icon (
          GTK_WINDOW (window), 
          gdk_pixbuf_new_from_xpm_data ((const char **) gucharmap_xpm));

  g_signal_connect (G_OBJECT (window), "destroy",
                    G_CALLBACK (gtk_main_quit), NULL);

  vbox = gtk_vbox_new (FALSE, VBOX_SPACING);
  gtk_container_add (GTK_CONTAINER (window), vbox);

  gtk_box_pack_start (GTK_BOX (vbox), make_menu (), FALSE, FALSE, 0);

  charmap = charmap_new ();
  gtk_box_pack_start (GTK_BOX (vbox), charmap, TRUE, TRUE, 0);

  fontsel_toggle = gtk_toggle_button_new_with_label (GTK_STOCK_SELECT_FONT);
  gtk_button_set_use_stock (GTK_BUTTON (fontsel_toggle), TRUE);
  gtk_box_pack_start (GTK_BOX (vbox), fontsel_toggle, FALSE, FALSE, 0);

  fontsel = gtk_font_selection_new ();

  g_signal_connect (G_OBJECT (fontsel_toggle), "toggled",
                    G_CALLBACK (toggle_fontsel), fontsel);
  gtk_tooltips_set_tip (tooltips, fontsel_toggle, 
                        _("Show/hide font selection."), NULL);

  g_signal_connect (
          gtk_tree_view_get_selection (GTK_TREE_VIEW (
                  GTK_FONT_SELECTION (fontsel)->family_list)), 
          "changed", G_CALLBACK (fontsel_changed), fontsel);
  g_signal_connect (
          gtk_tree_view_get_selection (GTK_TREE_VIEW (
                  GTK_FONT_SELECTION (fontsel)->face_list)), 
          "changed", G_CALLBACK (fontsel_changed), fontsel);
  g_signal_connect (
          gtk_tree_view_get_selection (GTK_TREE_VIEW (
                  GTK_FONT_SELECTION (fontsel)->size_list)), 
          "changed", G_CALLBACK (fontsel_changed), fontsel);

  gtk_window_set_default_size (GTK_WINDOW (window), 
                               gdk_screen_width () * 1/2,
                               gdk_screen_height () * 1/2);

  /* make the starting font 3/2 of the default selection in fontsel */
  orig_font = gtk_font_selection_get_font_name (GTK_FONT_SELECTION (fontsel));
  font_desc = pango_font_description_from_string (orig_font);
  pango_font_description_set_size (
          font_desc, pango_font_description_get_size (font_desc) * 3/2);
  new_font = pango_font_description_to_string (font_desc);
  /* this sends the changed signal: */
  gtk_font_selection_set_font_name (GTK_FONT_SELECTION (fontsel), new_font);
  pango_font_description_free (font_desc);
  g_free (orig_font);
  g_free (new_font);

  /* show everything so far */
  gtk_widget_show_all (window);

  /* don't show the fontsel */
  gtk_box_pack_start (GTK_BOX (vbox), fontsel, FALSE, FALSE, 0);
  gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON (fontsel_toggle), FALSE);

  /* show the statusbar */
  statusbar = charmap_get_statusbar (CHARMAP (charmap));
  gtk_box_pack_start (GTK_BOX (vbox), statusbar, FALSE, FALSE, 0);
  gtk_widget_show (statusbar);
  gtk_statusbar_set_has_resize_grip (GTK_STATUSBAR (statusbar), TRUE);

  gtk_main ();

  return 0;
}
