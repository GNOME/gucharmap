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
#include "unicode_info.h"
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
static GtkWidget *status;
static GtkWidget *search_status;
static GtkWidget *text_to_copy_status;
static GdkPixbuf *icon;

typedef struct 
{
  GtkWidget *entry;
  GtkWidget *label;
} 
EntryAndLabel;


static void
set_status (const gchar *message)
{
  /* underflow is allowed */
  gtk_statusbar_pop (GTK_STATUSBAR (status), 0); 
  if (message != NULL)
    gtk_statusbar_push (GTK_STATUSBAR (status), 0, message);
}


static void
status_message (GtkWidget *widget, const gchar *message)
{
  set_status (message);
}


static void
set_search_status (const gchar *message)
{
  gtk_label_set_text (GTK_LABEL (search_status), message);
  set_status (message);
}


static void
set_text_to_copy_status (const gchar *message)
{
  gtk_label_set_text (GTK_LABEL (text_to_copy_status), message);
  set_status (message);
}


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

  eal->label = gtk_label_new (NULL);
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
  GString *gs;
  gchar ubuf[7];
  gint n;

  if (! is_valid_character (uc))
    {
      set_text_to_copy_status (_("The selected code point is not a valid unicode character."));
      return;
    }

  n = g_unichar_to_utf8 (uc, ubuf);
  ubuf[n] = '\0';

  gs = g_string_new (gtk_entry_get_text (GTK_ENTRY (text_to_copy)));
  g_string_append (gs, ubuf);

  gtk_entry_set_text (GTK_ENTRY (text_to_copy), gs->str);

  g_string_free (gs, TRUE);

  set_text_to_copy_status (NULL);
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

  set_text_to_copy_status (_("Text copied to clipboard."));

  return TRUE;
}


static gint
clear_button_clicked (GtkWidget *widget, gpointer callback_data)
{
  gtk_entry_set_text (GTK_ENTRY (text_to_copy), "");
  set_text_to_copy_status (_("Text-to-copy entry box cleared."));
  return TRUE;
}


static void
do_search (GtkWidget *widget, gpointer data)
{
  const gchar *search_text;

  search_text = gtk_entry_get_text (GTK_ENTRY (search_entry));
  switch (charmap_search (CHARMAP (charmap), search_text))
    {
      case NOT_FOUND:
        set_search_status (_("Not found."));
        break;

      case FOUND:
        set_search_status (_("Found."));
        break;

      case WRAPPED:
        set_search_status (_("Search wrapped."));
        break;

      case NOTHING_TO_SEARCH_FOR:
        set_search_status (_("Nothing to search for."));
        break;

      default:
        g_warning ("charmap_search returned an unexpected result; this should never happen");
    }
}


static GtkWidget *
make_search ()
{
  GtkWidget *hbox;
  GtkWidget *vbox;
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

  vbox = gtk_vbox_new (FALSE, 0);
  gtk_box_pack_start (GTK_BOX (vbox), hbox, FALSE, FALSE, 0);

  /* the status message label */
  hbox = gtk_hbox_new (FALSE, 0); 
  search_status = gtk_label_new (NULL);
  gtk_label_set_justify (GTK_LABEL (search_status), GTK_JUSTIFY_LEFT);
  gtk_box_pack_start (GTK_BOX (hbox), search_status, FALSE, FALSE, 0);
  gtk_box_pack_start (GTK_BOX (vbox), hbox, FALSE, FALSE, 0);

  gtk_widget_show_all (vbox);

  return vbox;
}


static GtkWidget *
make_text_to_copy ()
{
  GtkWidget *button;
  GtkWidget *label;
  GtkWidget *table;
  GtkWidget *hbox;
  GtkTooltips *tooltips;

  table = gtk_table_new (2, 2, FALSE);
  gtk_table_set_col_spacings (GTK_TABLE (table), 6);

  tooltips = gtk_tooltips_new ();

  label = gtk_label_new (_("Text to copy:"));
  /* gtk_table_attach_defaults (GTK_TABLE (table), label, 0, 1, 0, 1); */
  gtk_table_attach (GTK_TABLE (table), label, 0, 1, 0, 1, 0, 0, 0, 0);

  hbox = gtk_hbox_new (FALSE, 6);
  gtk_table_attach_defaults (GTK_TABLE (table), hbox, 1, 2, 0, 1);

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

  /* the status message label */
  hbox = gtk_hbox_new (FALSE, 0); 
  text_to_copy_status = gtk_label_new (NULL);
  gtk_label_set_justify (GTK_LABEL (text_to_copy_status), GTK_JUSTIFY_LEFT);
  gtk_box_pack_start (GTK_BOX (hbox), text_to_copy_status, FALSE, FALSE, 0);
  gtk_table_attach_defaults (GTK_TABLE (table), hbox, 1, 2, 1, 2);

  gtk_widget_show_all (table);

  return table;
}


#if HAVE_GNOME
static void
help_about (GtkWidget *widget, gpointer data)
{
  GtkWidget *about;
  const gchar *authors[] = { "Noah Levitt <nlevitt аt columbia.edu>", 
                             "Daniel Elstner <daniel.elstner аt gmx.net>", 
                             NULL };
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
  GtkWidget *big_vbox;
  GtkWidget *hbox;
  GtkWidget *fontsel;
  GtkWidget *toolbar; /* the fontsel goes on this */
  GtkWidget *spacer;
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

  big_vbox = gtk_vbox_new (FALSE, 0);
  gtk_container_add (GTK_CONTAINER (window), big_vbox);

  gtk_box_pack_start (GTK_BOX (big_vbox), make_menu (), FALSE, FALSE, 0);

  toolbar = gtk_toolbar_new ();
  fontsel = mini_font_selection_new ();
  gtk_toolbar_append_widget (GTK_TOOLBAR (toolbar), fontsel, NULL, NULL);
  gtk_box_pack_start (GTK_BOX (big_vbox), toolbar, FALSE, FALSE, 0);
  gtk_widget_show_all (toolbar);

  /* some empty space */
  spacer = gtk_alignment_new (0, 0, 0, 0); 
  gtk_widget_set_size_request (spacer, -1, 12);
  gtk_widget_show (spacer);
  gtk_box_pack_start (GTK_BOX (big_vbox), spacer, FALSE, FALSE, 0);

  /* hbox has search and text_to_copy */
  hbox = gtk_hbox_new (FALSE, 30); /* space between the parts */
  gtk_box_pack_start (GTK_BOX (hbox), make_search (), TRUE, TRUE, 0);
  gtk_box_pack_start (GTK_BOX (hbox), make_text_to_copy (), TRUE, TRUE, 0);
  gtk_container_set_border_width (GTK_CONTAINER (hbox), 6);
  gtk_box_pack_start (GTK_BOX (big_vbox), hbox, FALSE, FALSE, 0);
  gtk_widget_show (hbox);
  /* end hbox */

  charmap = charmap_new ();
  gtk_widget_show (charmap);
  gtk_box_pack_start (GTK_BOX (big_vbox), charmap, TRUE, TRUE, 0);

  status = gtk_statusbar_new ();
  gtk_box_pack_start (GTK_BOX (big_vbox), status, FALSE, FALSE, 0);
  gtk_widget_show (status);

  g_signal_connect (fontsel, "changed", G_CALLBACK (fontsel_changed), charmap);
  g_signal_connect (charmap, "activate", 
                    G_CALLBACK (append_character_to_text_to_copy), NULL);
  g_signal_connect (charmap, "status-message",
                    G_CALLBACK (status_message), NULL);

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

  gtk_widget_show (big_vbox);
  gtk_widget_show (window);

  /* XXX: slightly evil cuz we're not supposed to know about chartable */
  gtk_widget_grab_focus (CHARMAP (charmap)->chartable);

  gtk_main ();

  return 0;
}
