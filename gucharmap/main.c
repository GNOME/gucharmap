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


#include <gtk/gtk.h>
#include <stdlib.h>
#include "charmap.h"
#include "mini_fontsel.h"
#if HAVE_GNOME
# include "gnome.h"
#endif
#include "gucharmap_intl.h"

#ifndef ICON_PATH
# define ICON_PATH ""
#endif


static GtkWidget *charmap;
static GtkWidget *text_to_copy;
static GtkWidget *search_entry;
static GdkPixbuf *icon;

typedef struct 
{
  GtkWidget *entry;
  GtkWidget *label;
} 
EntryAndLabel;


static void
expand_collapse (GtkCheckMenuItem *mi, gpointer data)
{
  if (gtk_check_menu_item_get_active (mi))
    charmap_expand_block_selector (CHARMAP (charmap));
  else
    charmap_collapse_block_selector (CHARMAP (charmap));
}


static void
jump_code_point_response (GtkDialog *dialog, gint response, EntryAndLabel *eal)
{

  if (response == GTK_RESPONSE_OK)
    {
      const gchar *text;
      gchar *message;
      gchar *endptr;
      glong l;

      text = gtk_entry_get_text (GTK_ENTRY (eal->entry));

      l = strtol (text, &endptr, 16);

      if (endptr != text && l >= 0 && l <= UNICHAR_MAX)
        charmap_go_to_character (CHARMAP (charmap), (gunichar) l);
      else
        {
          message = g_strdup_printf (_("Not a valid code point to jump to. Must be a hexadecimal number between 0 and %4.4X."), UNICHAR_MAX);
          gtk_label_set_text (GTK_LABEL (eal->label), message);
          g_free (message);
          return;
        }
    }

  g_free (eal);
  gtk_widget_destroy (GTK_WIDGET (dialog));
}


static void
jump_code_point (GtkWidget *widget, gpointer data)
{
  GtkWidget *dialog;
  EntryAndLabel *eal;
  GtkWidget *vbox;

  dialog = gtk_dialog_new_with_buttons (
          _("Go to hex code point"),
          GTK_WINDOW (gtk_widget_get_toplevel (widget)),
          GTK_DIALOG_DESTROY_WITH_PARENT, 
          GTK_STOCK_CANCEL, GTK_RESPONSE_CANCEL, 
          GTK_STOCK_OK, GTK_RESPONSE_OK, 
          NULL);

  vbox = gtk_vbox_new (FALSE, 6);
  gtk_container_set_border_width (GTK_CONTAINER (vbox), 6);

  gtk_box_pack_start (GTK_BOX (GTK_DIALOG (dialog)->vbox), vbox,
                      FALSE, FALSE, 0);

  eal = g_malloc (sizeof (EntryAndLabel));
  eal->entry = gtk_entry_new ();
  gtk_entry_set_activates_default (GTK_ENTRY (eal->entry), TRUE);
  gtk_entry_set_max_length (GTK_ENTRY (eal->entry), 8); 
  gtk_entry_set_width_chars (GTK_ENTRY (eal->entry), 8);

  gtk_box_pack_start (GTK_BOX (vbox),
                      gtk_label_new (_("Enter unicode code point")),
                      FALSE, FALSE, 0);

  gtk_box_pack_start (GTK_BOX (vbox), eal->entry, FALSE, FALSE, 0);

  eal->label = gtk_label_new ("");
  gtk_label_set_line_wrap (GTK_LABEL (eal->label), TRUE);
  gtk_box_pack_start (GTK_BOX (vbox), eal->label, FALSE, FALSE, 0);

  gtk_dialog_set_default_response (GTK_DIALOG (dialog), GTK_RESPONSE_OK);

  g_signal_connect (GTK_DIALOG (dialog), "response", 
                    G_CALLBACK (jump_code_point_response), eal);

  gtk_widget_show_all (dialog);

  gtk_widget_grab_focus (eal->entry);
}


static void
jump_clipboard (GtkWidget *widget, gpointer data)
{
  charmap_identify_clipboard (CHARMAP (charmap), 
                              gtk_clipboard_get (GDK_SELECTION_CLIPBOARD));
}


static void
append_character_to_text_to_copy (Charmap *charmap, gunichar uc)
{
  static gchar buf[TEXT_TO_COPY_MAXLENGTH];
  static gchar ubuf[7];
  gint n;

  n = g_unichar_to_utf8 (uc, ubuf);
  ubuf[n] = '\0';

  g_snprintf (buf, TEXT_TO_COPY_MAXLENGTH, "%s%s", 
              gtk_entry_get_text (GTK_ENTRY (text_to_copy)), ubuf);

  gtk_entry_set_text (GTK_ENTRY (text_to_copy), buf);
}


static gint
copy_button_clicked (GtkWidget *widget, gpointer callback_data)
{
  GtkClipboard *clipboard = gtk_clipboard_get (GDK_SELECTION_CLIPBOARD);

  /* select it so it's in SELECTION_PRIMARY */
  gtk_editable_select_region (GTK_EDITABLE (text_to_copy), 0, -1);

  /* copy to SELECTION_CLIPBOARD */
  gtk_clipboard_set_text (
          clipboard, 
          gtk_entry_get_text (GTK_ENTRY (text_to_copy)), -1);

  /* set_statusbar_message (charmap, _("Text copied to clipboard.")); */

  return TRUE;
}


static gint
clear_button_clicked (GtkWidget *widget, gpointer callback_data)
{
  gtk_entry_set_text (GTK_ENTRY (text_to_copy), "");
  /* set_statusbar_message (charmap, _("Text-to-copy entry box cleared.")); */
  return TRUE;
}


static void
do_search (GtkWidget *widget, gpointer data)
{
  const gchar *search_text;

  search_text = gtk_entry_get_text (GTK_ENTRY (search_entry));
  charmap_search (CHARMAP (charmap), search_text);
}


static GtkWidget *
make_search ()
{
  GtkWidget *hbox;
  GtkWidget *button;
  GtkTooltips *tooltips;

  tooltips = gtk_tooltips_new ();

  /* search */
  hbox = gtk_hbox_new (FALSE, 6);

  search_entry = gtk_entry_new ();
  g_signal_connect (G_OBJECT (search_entry), "activate",
                    G_CALLBACK (do_search), NULL);
  gtk_box_pack_start (GTK_BOX (hbox), search_entry, TRUE, TRUE, 0);

  button = gtk_button_new_from_stock (GTK_STOCK_FIND);
  g_signal_connect (G_OBJECT (button), "clicked",
                    G_CALLBACK (do_search), NULL);
  gtk_box_pack_start (GTK_BOX (hbox), button, FALSE, FALSE, 0);

  gtk_tooltips_set_tip (tooltips, button, _("Search for the next occurrence of this string in a character's Unicode name."), NULL);

  gtk_widget_show_all (hbox);

  return hbox;
}


static GtkWidget *
make_text_to_copy ()
{
  GtkWidget *hbox;
  GtkWidget *button;
  GtkWidget *label;
  GtkTooltips *tooltips;

  hbox = gtk_hbox_new (FALSE, 6);

  tooltips = gtk_tooltips_new ();

  label = gtk_label_new (_("Text to copy:"));
  gtk_box_pack_start (GTK_BOX (hbox), label, FALSE, FALSE, 0);

  text_to_copy = gtk_entry_new ();
  gtk_entry_set_max_length (GTK_ENTRY (text_to_copy), 
                            TEXT_TO_COPY_MAXLENGTH);
  gtk_box_pack_start (GTK_BOX (hbox), text_to_copy, TRUE, TRUE, 0);

  /* the copy button */
  button = gtk_button_new_from_stock (GTK_STOCK_COPY); 
  g_signal_connect (G_OBJECT (button), "clicked",
                    G_CALLBACK (copy_button_clicked), NULL);
  gtk_box_pack_start (GTK_BOX (hbox), button, FALSE, FALSE, 0);

  gtk_tooltips_set_tip (tooltips, button, _("Copy to the clipboard."), NULL);

  /* the clear button */
  button = gtk_button_new_from_stock (GTK_STOCK_CLEAR);
  g_signal_connect (G_OBJECT (button), "clicked",
                    G_CALLBACK (clear_button_clicked), NULL);
  gtk_box_pack_start (GTK_BOX (hbox), button, FALSE, FALSE, 0);

  gtk_widget_show_all (hbox);

  return hbox;
}


#if HAVE_GNOME
static void
help_about (GtkWidget *widget, gpointer data)
{
  GtkWidget *about;
  const gchar *authors[] = { "Noah Levitt <nlevitt аt columbia.edu>", NULL };
  const gchar *translator_credits;

  translator_credits = _("translator_credits");
  if (strcmp (translator_credits, "translator_credits") == 0)
    translator_credits = NULL;

  about = gnome_about_new (
          "gucharmap", VERSION, 
          "Copyright © 2003 Noah Levitt <nlevitt аt columbia.edu>",
          _("Unicode Character Map"), authors, NULL, translator_credits, icon);

  gtk_widget_show (about);
}


static GtkWidget *
make_gnome_help_menu ()
{
  GnomeUIInfo help_menu[] =
  {
    GNOMEUIINFO_MENU_ABOUT_ITEM (help_about, NULL),
    GNOMEUIINFO_END
  };
  GtkWidget *menu;

  menu = gtk_menu_new ();

  gnome_app_fill_menu (GTK_MENU_SHELL (menu), help_menu, NULL, TRUE, 0);

  return menu;
}
#endif /* #if HAVE_GNOME */


static void
fontsel_changed (MiniFontSelection *fontsel, Charmap *charmap)
{
  charmap_set_font (charmap, mini_font_selection_get_font_name (fontsel));
}


static GtkWidget *
make_menu ()
{
  GtkWidget *menubar;
  GtkWidget *file_menu, *view_menu, *goto_menu;
  GtkWidget *file_menu_item, *view_menu_item;
  GtkWidget *goto_menu_item, *help_menu_item;
  GtkWidget *menu_item;

  /* make the menu bar */
  menubar = gtk_menu_bar_new ();
  file_menu_item = gtk_menu_item_new_with_mnemonic (_("Char_map"));
  gtk_menu_shell_append (GTK_MENU_SHELL (menubar), file_menu_item);
  view_menu_item = gtk_menu_item_new_with_mnemonic (_("_View"));
  gtk_menu_shell_append (GTK_MENU_SHELL (menubar), view_menu_item);
  goto_menu_item = gtk_menu_item_new_with_mnemonic (_("_Go to"));
  gtk_menu_shell_append (GTK_MENU_SHELL (menubar), goto_menu_item);
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
  /* finished making the view menu */

  /* make the goto menu */
  goto_menu = gtk_menu_new ();
  gtk_menu_item_set_submenu (GTK_MENU_ITEM (goto_menu_item), goto_menu);

  menu_item = gtk_menu_item_new_with_mnemonic (_("_Hex Code Point..."));
  g_signal_connect (G_OBJECT (menu_item), "activate",
                    G_CALLBACK (jump_code_point), NULL);
  gtk_menu_shell_append (GTK_MENU_SHELL (goto_menu), menu_item);

  menu_item = gtk_menu_item_new_with_mnemonic (_("Character in _Clipboard"));
  g_signal_connect (G_OBJECT (menu_item), "activate",
                    G_CALLBACK (jump_clipboard), NULL); 
  gtk_menu_shell_append (GTK_MENU_SHELL (goto_menu), menu_item);
  /* finished making the goto menu */

#if HAVE_GNOME
  /* make the help menu */
  help_menu_item = gtk_menu_item_new_with_mnemonic (_("_Help"));
  gtk_menu_shell_append (GTK_MENU_SHELL (menubar), help_menu_item);
  gtk_menu_item_set_submenu (GTK_MENU_ITEM (help_menu_item), 
                             make_gnome_help_menu ());
  /* finished making the help menu */
#endif /* #if HAVE_GNOME */

  gtk_widget_show_all (menubar);
  return menubar;
}


gint
main (gint argc, gchar **argv)
{
  GtkWidget *window = NULL;
  GtkWidget *vbox;
  GtkWidget *hbox;
  GtkWidget *statusbar;
  GtkWidget *fontsel;
  GtkWidget *toolbar; /* the fontsel goes on this */
  GtkTooltips *tooltips;
  gchar *orig_font, *new_font;
  PangoFontDescription *font_desc;
  GError *error = NULL;

#if HAVE_GNOME
  gnome_program_init ("gucharmap", VERSION, LIBGNOMEUI_MODULE, argc, argv,
                      NULL, NULL, NULL);
#else
  gtk_init (&argc, &argv);
#endif

  tooltips = gtk_tooltips_new ();

  window = gtk_window_new (GTK_WINDOW_TOPLEVEL);
  gtk_window_set_default_size (GTK_WINDOW (window), 
                               gdk_screen_width () * 1/2,
                               gdk_screen_height () * 1/2);
  gtk_window_set_title (GTK_WINDOW (window), _("Unicode Character Map"));
  icon = gdk_pixbuf_new_from_file (ICON_PATH, &error);
  if (error != NULL)
    {
      g_assert (icon == NULL);
      g_warning ("Error loading icon: %s\n", error->message);
      g_error_free (error);
    }
  else
    gtk_window_set_icon (GTK_WINDOW (window), icon);

  g_signal_connect (G_OBJECT (window), "destroy",
                    G_CALLBACK (gtk_main_quit), NULL);

  vbox = gtk_vbox_new (FALSE, 0);
  gtk_container_add (GTK_CONTAINER (window), vbox);

  gtk_box_pack_start (GTK_BOX (vbox), make_menu (), FALSE, FALSE, 0);

  toolbar = gtk_toolbar_new ();
  fontsel = mini_font_selection_new ();
  gtk_toolbar_append_widget (GTK_TOOLBAR (toolbar), fontsel, NULL, NULL);
  gtk_box_pack_start (GTK_BOX (vbox), toolbar, FALSE, FALSE, 0);
  gtk_widget_show_all (toolbar);

  /* hbox has search and text_to_copy */
  hbox = gtk_hbox_new (FALSE, 18); /* space between the parts */
  gtk_box_pack_start (GTK_BOX (hbox), make_search (), TRUE, TRUE, 0);
  gtk_box_pack_start (GTK_BOX (hbox), make_text_to_copy (), TRUE, TRUE, 0);
  gtk_box_pack_start (GTK_BOX (vbox), hbox, FALSE, FALSE, 6);
  gtk_container_set_border_width (GTK_CONTAINER (hbox), 6);
  gtk_widget_show (hbox);
  /* end hbox */

  charmap = charmap_new ();
  gtk_widget_show (charmap);
  gtk_box_pack_start (GTK_BOX (vbox), charmap, TRUE, TRUE, 0);

  g_signal_connect (fontsel, "changed", G_CALLBACK (fontsel_changed), charmap);
  g_signal_connect (charmap, "activate", 
	            G_CALLBACK (append_character_to_text_to_copy), NULL);

  /* make the starting font 50% bigger than the default font */
  orig_font = mini_font_selection_get_font_name (MINI_FONT_SELECTION (fontsel));
  font_desc = pango_font_description_from_string (orig_font);
  pango_font_description_set_size (
          font_desc, pango_font_description_get_size (font_desc) * 3/2);
  new_font = pango_font_description_to_string (font_desc);
  /* this sends the changed signal: */
  mini_font_selection_set_font_name (MINI_FONT_SELECTION (fontsel), new_font);
  pango_font_description_free (font_desc);
  g_free (orig_font);
  g_free (new_font);

  gtk_widget_show (vbox);
  gtk_widget_show (window);

  /* show the statusbar */
  statusbar = charmap_get_statusbar (CHARMAP (charmap));
  gtk_box_pack_start (GTK_BOX (vbox), statusbar, FALSE, FALSE, 0);
  gtk_widget_show (statusbar);
  gtk_statusbar_set_has_resize_grip (GTK_STATUSBAR (statusbar), TRUE);

  gtk_main ();

  return 0;
}
