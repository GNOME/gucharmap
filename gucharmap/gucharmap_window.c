/* $Id$ */
/*
 * Copyright (c) 2003  Noah Levitt <nlevitt аt columbia.edu>
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

#if HAVE_CONFIG_H
# include <config.h>
#endif

#include <stdlib.h>
#include <string.h>
#include <gtk/gtk.h>
#include <gdk/gdkkeysyms.h>
#if HAVE_GNOME
# include <gnome.h>
#endif
#include <gucharmap/gucharmap_window.h>
#include <gucharmap/charmap.h>
#include <gucharmap/gucharmap_intl.h>
#include <gucharmap/mini_fontsel.h>
#include <gucharmap/unicode_info.h>

#ifndef ICON_PATH
# define ICON_PATH ""
#endif


static void
jump_clipboard (GtkWidget *widget, GucharmapWindow *guw)
{
  charmap_identify_clipboard (guw->charmap, 
                              gtk_clipboard_get (GDK_SELECTION_CLIPBOARD));
}


static void
jump_code_point_response (GtkDialog *dialog, 
                          gint response, 
                          GPtrArray *stuff)
{
  GucharmapWindow *guw = g_ptr_array_index (stuff, 0);
  GtkWidget *entry = g_ptr_array_index (stuff, 1);
  GtkWidget *label = g_ptr_array_index (stuff, 2);

  if (response == GTK_RESPONSE_OK)
    {
      const gchar *text;
      gchar *message;
      gchar *endptr;
      glong l;

      text = gtk_entry_get_text (GTK_ENTRY (entry));

      l = strtol (text, &endptr, 16);

      if (*text != '\0' && *endptr == '\0' && l >= 0 && l <= UNICHAR_MAX)
        charmap_go_to_character (guw->charmap, (gunichar) l);
      else
        {
          message = g_strdup_printf (_("Not a valid code point to jump to. Must be a hexadecimal number between 0 and %4.4X."), UNICHAR_MAX);
          gtk_label_set_text (GTK_LABEL (label), message);
          g_free (message);
          return;
        }
    }

  g_ptr_array_free (stuff, FALSE);
  gtk_widget_destroy (GTK_WIDGET (dialog));
}


static void
jump_code_point (GtkWidget *widget, GucharmapWindow *guw)
{
  GtkWidget *dialog;
  GtkWidget *vbox;
  GtkWidget *hbox;
  GtkWidget *entry;
  GtkWidget *label;
  GPtrArray *stuff;

  dialog = gtk_dialog_new_with_buttons (_("Go to hex code point"),
                                        GTK_WINDOW (guw),
                                        GTK_DIALOG_DESTROY_WITH_PARENT, 
                                        GTK_STOCK_CANCEL, GTK_RESPONSE_CANCEL, 
                                        GTK_STOCK_OK, GTK_RESPONSE_OK, 
                                        NULL);

  gtk_window_set_icon (GTK_WINDOW (dialog), guw->icon);

  vbox = gtk_vbox_new (FALSE, 6);
  gtk_container_set_border_width (GTK_CONTAINER (vbox), 6);

  gtk_box_pack_start (GTK_BOX (GTK_DIALOG (dialog)->vbox), vbox,
                      FALSE, FALSE, 0);

  entry = gtk_entry_new ();
  gtk_entry_set_activates_default (GTK_ENTRY (entry), TRUE);
  gtk_entry_set_max_length (GTK_ENTRY (entry), 8); 
  gtk_entry_set_width_chars (GTK_ENTRY (entry), 8);
  hbox = gtk_hbox_new (FALSE, 0);
  label = gtk_label_new_with_mnemonic (_("_Enter unicode code point"));
  gtk_box_pack_start (GTK_BOX (hbox), label, FALSE, FALSE, 0);
  gtk_label_set_justify (GTK_LABEL (label), GTK_JUSTIFY_LEFT);
  gtk_box_pack_start (GTK_BOX (vbox), hbox, FALSE, FALSE, 0);

  gtk_label_set_mnemonic_widget (GTK_LABEL (label), entry);

  gtk_box_pack_start (GTK_BOX (vbox), entry, FALSE, FALSE, 0);

  hbox = gtk_hbox_new (FALSE, 0);
  label = gtk_label_new (NULL);
  gtk_box_pack_start (GTK_BOX (hbox), label, FALSE, FALSE, 0);
  gtk_label_set_justify (GTK_LABEL (label), GTK_JUSTIFY_LEFT);
  gtk_label_set_line_wrap (GTK_LABEL (label), TRUE);
  gtk_box_pack_start (GTK_BOX (vbox), hbox, FALSE, FALSE, 0);

  gtk_dialog_set_default_response (GTK_DIALOG (dialog), GTK_RESPONSE_OK);

  stuff = g_ptr_array_sized_new (3);
  g_ptr_array_add (stuff, guw);
  g_ptr_array_add (stuff, entry);
  g_ptr_array_add (stuff, label);

  g_signal_connect (GTK_DIALOG (dialog), "response", 
                    G_CALLBACK (jump_code_point_response), stuff);

  gtk_widget_show_all (dialog);

  gtk_widget_grab_focus (entry);
}


static void
set_status (GucharmapWindow *guw, const gchar *message)
{
  /* underflow is allowed */
  gtk_statusbar_pop (GTK_STATUSBAR (guw->status), 0); 
  if (message != NULL)
    gtk_statusbar_push (GTK_STATUSBAR (guw->status), 0, message);
}


static void
status_message (GtkWidget *widget, const gchar *message, GucharmapWindow *guw)
{
  set_status (guw, message);
}


/* direction is -1 (back) or +1 (forward) */
static void
do_search (GucharmapWindow *guw, 
           const gchar *search_text, 
           gint direction)
{
  g_assert (direction == -1 || direction == 1);

  switch (charmap_search (guw->charmap, search_text, direction))
    {
      case CHARMAP_NOT_FOUND:
        set_status (guw, _("Not found."));
        break;

      case CHARMAP_FOUND:
        set_status (guw, _("Found."));
        break;

      case CHARMAP_WRAPPED:
        set_status (guw, _("Search wrapped."));
        break;

      case CHARMAP_NOTHING_TO_SEARCH_FOR:
        set_status (guw, _("Nothing to search for."));
        break;

      default:
        g_warning ("charmap_search returned an unexpected result; this should never happen");
    }
}


static void
search_find_response (GtkDialog *dialog, gint response, GPtrArray *stuff)
{
  GucharmapWindow *guw = g_ptr_array_index (stuff, 0);
  GtkWidget *entry = g_ptr_array_index (stuff, 1);

  if (response == GTK_RESPONSE_OK)
    {
      if (guw->last_search != NULL)
        g_free (guw->last_search);

      guw->last_search = g_strdup (gtk_entry_get_text (GTK_ENTRY (entry)));

      do_search (guw, guw->last_search, 1);
    }

  g_ptr_array_free (stuff, FALSE);
  gtk_widget_destroy (GTK_WIDGET (dialog));
}


static void
search_find (GtkWidget *widget, GucharmapWindow *guw)
{
  GtkWidget *dialog;
  GtkWidget *hbox;
  GtkWidget *label;
  GtkWidget *entry;
  GPtrArray *stuff;

  dialog = gtk_dialog_new_with_buttons (_("Find"),
                                        GTK_WINDOW (guw),
                                        GTK_DIALOG_DESTROY_WITH_PARENT, 
                                        GTK_STOCK_CANCEL, GTK_RESPONSE_CANCEL, 
                                        GTK_STOCK_FIND, GTK_RESPONSE_OK, 
                                        NULL);

  gtk_window_set_icon (GTK_WINDOW (dialog), guw->icon);
  gtk_dialog_set_default_response (GTK_DIALOG (dialog), GTK_RESPONSE_OK);

  hbox = gtk_hbox_new (FALSE, 12);
  gtk_container_set_border_width (GTK_CONTAINER (hbox), 6);
  gtk_box_pack_start (GTK_BOX (GTK_DIALOG (dialog)->vbox), hbox,
                      FALSE, FALSE, 0);

  label = gtk_label_new_with_mnemonic (_("_Search:"));
  gtk_box_pack_start (GTK_BOX (hbox), label, FALSE, FALSE, 0);

  entry = gtk_entry_new ();
  gtk_entry_set_activates_default (GTK_ENTRY (entry), TRUE);
  if (guw->last_search != NULL)
    gtk_entry_set_text (GTK_ENTRY (entry), guw->last_search);
  gtk_box_pack_start (GTK_BOX (hbox), entry, TRUE, TRUE, 0);

  gtk_label_set_mnemonic_widget (GTK_LABEL (label), entry);

  stuff = g_ptr_array_sized_new (2);
  g_ptr_array_add (stuff, guw);
  g_ptr_array_add (stuff, entry);

  g_signal_connect (GTK_DIALOG (dialog), "response", 
                    G_CALLBACK (search_find_response), stuff);

  gtk_widget_show_all (dialog);

  gtk_widget_grab_focus (entry);
}


static void
search_find_next (GtkWidget *widget, GucharmapWindow *guw)
{
  if (guw->last_search != NULL)
    do_search (guw, guw->last_search, 1);
  else
    search_find (widget, guw);
}


static void
search_find_prev (GtkWidget *widget, GucharmapWindow *guw)
{
  if (guw->last_search != NULL)
    do_search (guw, guw->last_search, -1);
  /* XXX: else, open up search dialog, but search backwards :-( */
}


static void
set_caption_visible (GucharmapWindow *guw, 
                     CharmapCaption caption_id, 
                     gboolean visible)
{
  if (visible)
    {
      guw->caption_show[caption_id] = TRUE;
      charmap_show_caption (guw->charmap, caption_id);
    }
  else
    {
      guw->caption_show[caption_id] = FALSE;
      charmap_hide_caption (guw->charmap, caption_id);
    }
}


static void
show_hide_caption_category (GtkCheckMenuItem *mi, GucharmapWindow *guw)
{
  set_caption_visible (guw, CHARMAP_CAPTION_CATEGORY, 
                       gtk_check_menu_item_get_active (mi));
}

static void
show_hide_caption_decomposition (GtkCheckMenuItem *mi, GucharmapWindow *guw)
{
  set_caption_visible (guw, CHARMAP_CAPTION_DECOMPOSITION, 
                       gtk_check_menu_item_get_active (mi));
}

static void
show_hide_caption_utf8 (GtkCheckMenuItem *mi, GucharmapWindow *guw)
{
  set_caption_visible (guw, CHARMAP_CAPTION_UTF8, 
                       gtk_check_menu_item_get_active (mi));
}

static void
show_hide_caption_other_reps (GtkCheckMenuItem *mi, GucharmapWindow *guw)
{
  set_caption_visible (guw, CHARMAP_CAPTION_OTHER_REPS, 
                       gtk_check_menu_item_get_active (mi));
}

static void
show_hide_caption_equals (GtkCheckMenuItem *mi, GucharmapWindow *guw)
{
  set_caption_visible (guw, CHARMAP_CAPTION_EQUALS, 
                       gtk_check_menu_item_get_active (mi));
}

static void
show_hide_caption_stars (GtkCheckMenuItem *mi, GucharmapWindow *guw)
{
  set_caption_visible (guw, CHARMAP_CAPTION_STARS, 
                       gtk_check_menu_item_get_active (mi));
}

static void
show_hide_caption_exes (GtkCheckMenuItem *mi, GucharmapWindow *guw)
{
  set_caption_visible (guw, CHARMAP_CAPTION_EXES, 
                       gtk_check_menu_item_get_active (mi));
}

static void
show_hide_caption_pounds (GtkCheckMenuItem *mi, GucharmapWindow *guw)
{
  set_caption_visible (guw, CHARMAP_CAPTION_POUNDS, 
                       gtk_check_menu_item_get_active (mi));
}

static void
show_hide_caption_colons (GtkCheckMenuItem *mi, GucharmapWindow *guw)
{
  set_caption_visible (guw, CHARMAP_CAPTION_COLONS, 
                       gtk_check_menu_item_get_active (mi));
}

#if ENABLE_UNIHAN
static void
show_hide_caption_kdefinition (GtkCheckMenuItem *mi, GucharmapWindow *guw)
{
  set_caption_visible (guw, CHARMAP_CAPTION_KDEFINITION, 
                       gtk_check_menu_item_get_active (mi));
}

static void
show_hide_caption_kmandarin (GtkCheckMenuItem *mi, GucharmapWindow *guw)
{
  set_caption_visible (guw, CHARMAP_CAPTION_KMANDARIN, 
                       gtk_check_menu_item_get_active (mi));
}

static void
show_hide_caption_kjapaneseon (GtkCheckMenuItem *mi, GucharmapWindow *guw)
{
  set_caption_visible (guw, CHARMAP_CAPTION_KJAPANESEON, 
                       gtk_check_menu_item_get_active (mi));
}

static void
show_hide_caption_kjapanesekun (GtkCheckMenuItem *mi, GucharmapWindow *guw)
{
  set_caption_visible (guw, CHARMAP_CAPTION_KJAPANESEKUN, 
                       gtk_check_menu_item_get_active (mi));
}

static void
show_hide_caption_kcantonese (GtkCheckMenuItem *mi, GucharmapWindow *guw)
{
  set_caption_visible (guw, CHARMAP_CAPTION_KCANTONESE, 
                       gtk_check_menu_item_get_active (mi));
}

static void
show_hide_caption_ktang (GtkCheckMenuItem *mi, GucharmapWindow *guw)
{
  set_caption_visible (guw, CHARMAP_CAPTION_KTANG, 
                       gtk_check_menu_item_get_active (mi));
}

static void
show_hide_caption_kkorean (GtkCheckMenuItem *mi, GucharmapWindow *guw)
{
  set_caption_visible (guw, CHARMAP_CAPTION_KKOREAN, 
                       gtk_check_menu_item_get_active (mi));
}
#endif /* #if ENABLE_UNIHAN */


static void
show_hide_nameslist (GtkCheckMenuItem *mi, GucharmapWindow *guw)
{
  static const CharmapCaption nameslist_caption_ids[] = 
    { 
      CHARMAP_CAPTION_STARS,
      CHARMAP_CAPTION_EXES,
      CHARMAP_CAPTION_COLONS,
      CHARMAP_CAPTION_EQUALS,
      CHARMAP_CAPTION_POUNDS,
    };
  gint i;

  if (gtk_check_menu_item_get_active (mi))
    {
      gtk_widget_set_sensitive (guw->nameslist_options_menu_item, TRUE);

      /* show the checked nameslist captions */
      for (i = 0;  i < G_N_ELEMENTS (nameslist_caption_ids);  i++)
        if (guw->caption_show[nameslist_caption_ids[i]])
          charmap_show_caption (guw->charmap, nameslist_caption_ids[i]);
    }
  else
    {
      gtk_widget_set_sensitive (guw->nameslist_options_menu_item, FALSE);

      /* hide all the nameslist captions */
      for (i = 0;  i < G_N_ELEMENTS (nameslist_caption_ids);  i++)
        charmap_hide_caption (guw->charmap, nameslist_caption_ids[i]);
    }
}


#if ENABLE_UNIHAN
static void
show_hide_unihan (GtkCheckMenuItem *mi, GucharmapWindow *guw)
{
  static const CharmapCaption unihan_caption_ids[] = 
    { 
      CHARMAP_CAPTION_KDEFINITION,
      CHARMAP_CAPTION_KMANDARIN,
      CHARMAP_CAPTION_KJAPANESEON,
      CHARMAP_CAPTION_KJAPANESEKUN,
      CHARMAP_CAPTION_KCANTONESE,
      CHARMAP_CAPTION_KTANG,
      CHARMAP_CAPTION_KKOREAN,
    };
  gint i;

  if (gtk_check_menu_item_get_active (mi))
    {
      gtk_widget_set_sensitive (guw->unihan_options_menu_item, TRUE);

      /* show the checked unicode captions */
      for (i = 0;  i < G_N_ELEMENTS (unihan_caption_ids);  i++)
        if (guw->caption_show[unihan_caption_ids[i]])
          charmap_show_caption (guw->charmap, unihan_caption_ids[i]);
    }
  else
    {
      gtk_widget_set_sensitive (guw->unihan_options_menu_item, FALSE);

      /* hide all the unicode captions */
      for (i = 0;  i < G_N_ELEMENTS (unihan_caption_ids);  i++)
        charmap_hide_caption (guw->charmap, unihan_caption_ids[i]);
    }
}
#endif


static void
show_hide_unicode (GtkCheckMenuItem *mi, GucharmapWindow *guw)
{
  static const CharmapCaption unicode_caption_ids[] = 
    { 
      CHARMAP_CAPTION_CATEGORY, 
      CHARMAP_CAPTION_DECOMPOSITION,
      CHARMAP_CAPTION_UTF8,
      CHARMAP_CAPTION_OTHER_REPS
    };
  gint i;

  if (gtk_check_menu_item_get_active (mi))
    {
      gtk_widget_set_sensitive (guw->unicode_options_menu_item, TRUE);

      /* show the checked unicode captions */
      for (i = 0;  i < G_N_ELEMENTS (unicode_caption_ids);  i++)
        if (guw->caption_show[unicode_caption_ids[i]])
          charmap_show_caption (guw->charmap, unicode_caption_ids[i]);
    }
  else
    {
      gtk_widget_set_sensitive (guw->unicode_options_menu_item, FALSE);

      /* hide all the unicode captions */
      for (i = 0;  i < G_N_ELEMENTS (unicode_caption_ids);  i++)
        charmap_hide_caption (guw->charmap, unicode_caption_ids[i]);
    }
}


static void
toggle_zoom_mode (GtkCheckMenuItem *mi, GucharmapWindow *guw)
{
  if (gtk_check_menu_item_get_active (mi))
    {
      charmap_zoom_enable (guw->charmap);

      /* leave zoom mode by pressing escape (or by the normal means) */
      gtk_widget_add_accelerator (GTK_WIDGET (mi), "activate", 
                                  guw->accel_group, GDK_Escape, 0, 0);

      set_status (guw, _("Zoom mode enabled. Press <Esc> to disable zoom."));
    }
  else
    {
      charmap_zoom_disable (guw->charmap);

      /* but escape won't enter zoom mode, that would be too weird */
      gtk_widget_remove_accelerator (GTK_WIDGET (mi), guw->accel_group,
                                     GDK_Escape, 0);

      set_status (guw, _("Zoom mode disabled."));
    }
}


static void
font_bigger (GtkWidget *widget, GucharmapWindow *guw)
{
  gint size, increment;

  size = mini_font_selection_get_font_size (MINI_FONT_SELECTION (guw->fontsel));
  increment = MAX (size / 12, 1);
  mini_font_selection_set_font_size (MINI_FONT_SELECTION (guw->fontsel), 
                                     size + increment);
}


static void
font_smaller (GtkWidget *widget, GucharmapWindow *guw)
{
  gint size, increment;

  size = mini_font_selection_get_font_size (MINI_FONT_SELECTION (guw->fontsel));
  increment = MAX (size / 12, 1);
  mini_font_selection_set_font_size (MINI_FONT_SELECTION (guw->fontsel), 
                                     size - increment);
}



static void
expand_collapse (GtkCheckMenuItem *mi, GucharmapWindow *guw)
{
  if (gtk_check_menu_item_get_active (mi))
    charmap_expand_block_selector (guw->charmap);
  else
    charmap_collapse_block_selector (guw->charmap);

}


#if HAVE_GNOME
static void
help_about (GtkWidget *widget, GucharmapWindow *guw)
{
  GtkWidget *about;
  const gchar *authors[] = { "Noah Levitt <nlevitt аt columbia.edu>", 
                             "Daniel Elstner <daniel.elstner аt gmx.net>", 
                             "Padraig O'Briain <Padraig.Obriain аt sun.com>",
                             NULL };
  const gchar *translator_credits;

  translator_credits = _("translator_credits");
  if (strcmp (translator_credits, "translator_credits") == 0)
    translator_credits = NULL;

  about = gnome_about_new (
          "gucharmap", VERSION, 
          "Copyright © 2003 Noah Levitt <nlevitt аt columbia.edu>",
          _("Unicode Character Map"), authors, NULL, 
          translator_credits, guw->icon);

  gtk_window_set_icon (GTK_WINDOW (about), guw->icon);
  gtk_widget_show (about);
}


static GtkWidget *
make_gnome_help_menu (GucharmapWindow *guw)
{
  GnomeUIInfo help_menu[] =
  {
    GNOMEUIINFO_MENU_ABOUT_ITEM (help_about, guw),
    GNOMEUIINFO_END
  };
  GtkWidget *menu;

  menu = gtk_menu_new ();

  gnome_app_fill_menu (GTK_MENU_SHELL (menu), help_menu, NULL, TRUE, 0);

  return menu;
}
#endif /* #if HAVE_GNOME */


static GtkWidget *
make_menu (GucharmapWindow *guw)
{
  GtkWidget *menubar;
  GtkWidget *file_menu, *view_menu, *search_menu;
  GtkWidget *file_menu_item, *view_menu_item, *search_menu_item;
  GtkWidget *menu_item;
  GtkWidget *unicode_details_menu, *unihan_details_menu, 
            *nameslist_details_menu;
#if HAVE_GNOME
  GtkWidget *help_menu_item;
#endif

  guw->accel_group = gtk_accel_group_new ();
  gtk_window_add_accel_group (GTK_WINDOW (guw), guw->accel_group);
  g_object_unref (guw->accel_group);

  /* make the menu bar */
  menubar = gtk_menu_bar_new ();
  file_menu_item = gtk_menu_item_new_with_mnemonic (_("Char_map"));
  gtk_menu_shell_append (GTK_MENU_SHELL (menubar), file_menu_item);
  view_menu_item = gtk_menu_item_new_with_mnemonic (_("_View"));
  gtk_menu_shell_append (GTK_MENU_SHELL (menubar), view_menu_item);
  search_menu_item = gtk_menu_item_new_with_mnemonic (_("_Search"));
  gtk_menu_shell_append (GTK_MENU_SHELL (menubar), search_menu_item);
  /* finished making the menu bar */

  /* make the file menu */
  file_menu = gtk_menu_new ();
  gtk_menu_set_accel_group (GTK_MENU (file_menu), guw->accel_group);
  gtk_menu_item_set_submenu (GTK_MENU_ITEM (file_menu_item), file_menu);
  menu_item = gtk_image_menu_item_new_from_stock (GTK_STOCK_QUIT, 
                                                  guw->accel_group);
  g_signal_connect (G_OBJECT (menu_item), "activate",
                    G_CALLBACK (gtk_main_quit), NULL);
  gtk_menu_shell_append (GTK_MENU_SHELL (file_menu), menu_item);
  /* finished making the file menu */

  /* make the view menu */
  view_menu = gtk_menu_new ();
  gtk_menu_set_accel_group (GTK_MENU (view_menu), guw->accel_group);
  gtk_menu_item_set_submenu (GTK_MENU_ITEM (view_menu_item), view_menu);

  menu_item = gtk_check_menu_item_new_with_mnemonic (_("Expand/Collapse All"));
  g_signal_connect (G_OBJECT (menu_item), "activate",
                    G_CALLBACK (expand_collapse), guw);
  gtk_menu_shell_append (GTK_MENU_SHELL (view_menu), menu_item);

  /* separator */
  gtk_menu_shell_append (GTK_MENU_SHELL (view_menu), gtk_menu_item_new ());

  /* ctrl-+ or ctrl-= */
  menu_item = gtk_menu_item_new_with_mnemonic (_("Zoom _In"));
  gtk_widget_add_accelerator (menu_item, "activate", guw->accel_group,
                              GDK_plus, GDK_CONTROL_MASK, GTK_ACCEL_VISIBLE);
  gtk_widget_add_accelerator (menu_item, "activate", guw->accel_group,
                              GDK_equal, GDK_CONTROL_MASK, 0);
  gtk_widget_add_accelerator (menu_item, "activate", guw->accel_group,
                              GDK_KP_Add, GDK_CONTROL_MASK, 0);
  g_signal_connect (G_OBJECT (menu_item), "activate",
                    G_CALLBACK (font_bigger), guw);
  gtk_menu_shell_append (GTK_MENU_SHELL (view_menu), menu_item);

  /* ctrl-- */
  menu_item = gtk_menu_item_new_with_mnemonic (_("Zoom _Out"));
  gtk_widget_add_accelerator (menu_item, "activate", guw->accel_group,
                              GDK_minus, GDK_CONTROL_MASK, GTK_ACCEL_VISIBLE);
  gtk_widget_add_accelerator (menu_item, "activate", guw->accel_group,
                              GDK_KP_Subtract, GDK_CONTROL_MASK, 0);
  g_signal_connect (G_OBJECT (menu_item), "activate",
                    G_CALLBACK (font_smaller), guw);
  gtk_menu_shell_append (GTK_MENU_SHELL (view_menu), menu_item);

  /* separator */
  gtk_menu_shell_append (GTK_MENU_SHELL (view_menu), gtk_menu_item_new ());

  /* ctrl-<enter> */
  menu_item = gtk_check_menu_item_new_with_mnemonic (_("_Zoom Mode"));
  gtk_widget_add_accelerator (menu_item, "activate", guw->accel_group,
                              GDK_Return, GDK_CONTROL_MASK, GTK_ACCEL_VISIBLE);
  gtk_widget_add_accelerator (menu_item, "activate", guw->accel_group,
                              GDK_KP_Enter, GDK_CONTROL_MASK, 0);
  g_signal_connect (G_OBJECT (menu_item), "activate",
                    G_CALLBACK (toggle_zoom_mode), guw);
  gtk_menu_shell_append (GTK_MENU_SHELL (view_menu), menu_item);

  /* separator */
  gtk_menu_shell_append (GTK_MENU_SHELL (view_menu), gtk_menu_item_new ());

  menu_item = gtk_check_menu_item_new_with_mnemonic (_("_Unicode Details"));
  g_signal_connect (G_OBJECT (menu_item), "activate",
                    G_CALLBACK (show_hide_unicode), guw);
  gtk_menu_shell_append (GTK_MENU_SHELL (view_menu), menu_item);

  /* the unicode details submenu */
  guw->unicode_options_menu_item = gtk_menu_item_new_with_mnemonic (_("Options"));
  gtk_menu_shell_append (GTK_MENU_SHELL (view_menu),  
                         guw->unicode_options_menu_item);
  gtk_widget_set_sensitive (guw->unicode_options_menu_item, FALSE);

  unicode_details_menu = gtk_menu_new ();
  gtk_menu_item_set_submenu (GTK_MENU_ITEM (guw->unicode_options_menu_item), 
                             unicode_details_menu);

  menu_item = gtk_check_menu_item_new_with_mnemonic (_("_Category"));
  gtk_menu_shell_append (GTK_MENU_SHELL (unicode_details_menu), menu_item);
  gtk_check_menu_item_set_active (GTK_CHECK_MENU_ITEM (menu_item), 
                                  guw->caption_show[CHARMAP_CAPTION_CATEGORY]);
  g_signal_connect (G_OBJECT (menu_item), "activate",
                    G_CALLBACK (show_hide_caption_category), guw);

  menu_item = gtk_check_menu_item_new_with_mnemonic (
          _("Canonical _Decomposition"));
  gtk_menu_shell_append (GTK_MENU_SHELL (unicode_details_menu), menu_item);
  gtk_check_menu_item_set_active (
          GTK_CHECK_MENU_ITEM (menu_item), 
          guw->caption_show[CHARMAP_CAPTION_DECOMPOSITION]);
  g_signal_connect (G_OBJECT (menu_item), "activate",
                    G_CALLBACK (show_hide_caption_decomposition), guw);

  menu_item = gtk_check_menu_item_new_with_mnemonic (_("_UTF-8"));
  gtk_menu_shell_append (GTK_MENU_SHELL (unicode_details_menu), menu_item);
  gtk_check_menu_item_set_active (GTK_CHECK_MENU_ITEM (menu_item), 
                                  guw->caption_show[CHARMAP_CAPTION_UTF8]);
  g_signal_connect (G_OBJECT (menu_item), "activate",
                    G_CALLBACK (show_hide_caption_utf8), guw);

  menu_item = gtk_check_menu_item_new_with_mnemonic (
          _("_Other Representations"));
  gtk_menu_shell_append (GTK_MENU_SHELL (unicode_details_menu), menu_item);
  gtk_check_menu_item_set_active (
          GTK_CHECK_MENU_ITEM (menu_item), 
          guw->caption_show[CHARMAP_CAPTION_OTHER_REPS]);
  g_signal_connect (G_OBJECT (menu_item), "activate",
                    G_CALLBACK (show_hide_caption_other_reps), guw);

  /* separator */
  gtk_menu_shell_append (GTK_MENU_SHELL (view_menu), gtk_menu_item_new ());

  menu_item = gtk_check_menu_item_new_with_mnemonic (_("_Annotations and Cross References"));
  g_signal_connect (G_OBJECT (menu_item), "activate",
                    G_CALLBACK (show_hide_nameslist), guw);
  gtk_menu_shell_append (GTK_MENU_SHELL (view_menu), menu_item);

  /* the namelist details submenu */
  guw->nameslist_options_menu_item = gtk_menu_item_new_with_mnemonic (_("Options"));
  gtk_menu_shell_append (GTK_MENU_SHELL (view_menu), 
                         guw->nameslist_options_menu_item);
  gtk_widget_set_sensitive (guw->nameslist_options_menu_item, FALSE);

  nameslist_details_menu = gtk_menu_new ();
  gtk_menu_item_set_submenu (GTK_MENU_ITEM (guw->nameslist_options_menu_item), 
                             nameslist_details_menu);

  menu_item = gtk_check_menu_item_new_with_mnemonic (_("_Alias Names"));
  gtk_menu_shell_append (GTK_MENU_SHELL (nameslist_details_menu), menu_item);
  gtk_check_menu_item_set_active (GTK_CHECK_MENU_ITEM (menu_item), 
                                  guw->caption_show[CHARMAP_CAPTION_EQUALS]);
  g_signal_connect (G_OBJECT (menu_item), "activate",
                    G_CALLBACK (show_hide_caption_equals), guw);

  menu_item = gtk_check_menu_item_new_with_mnemonic (_("_Notes"));
  gtk_menu_shell_append (GTK_MENU_SHELL (nameslist_details_menu), menu_item);
  gtk_check_menu_item_set_active (GTK_CHECK_MENU_ITEM (menu_item), 
                                  guw->caption_show[CHARMAP_CAPTION_STARS]);
  g_signal_connect (G_OBJECT (menu_item), "activate",
                    G_CALLBACK (show_hide_caption_stars), guw);

  menu_item = gtk_check_menu_item_new_with_mnemonic (_("_See Also"));
  gtk_menu_shell_append (GTK_MENU_SHELL (nameslist_details_menu), menu_item);
  gtk_check_menu_item_set_active (GTK_CHECK_MENU_ITEM (menu_item), 
                                  guw->caption_show[CHARMAP_CAPTION_EXES]);
  g_signal_connect (G_OBJECT (menu_item), "activate",
                    G_CALLBACK (show_hide_caption_exes), guw);

  menu_item = gtk_check_menu_item_new_with_mnemonic (_("_Approximate Equivalents"));
  gtk_menu_shell_append (GTK_MENU_SHELL (nameslist_details_menu), menu_item);
  gtk_check_menu_item_set_active (GTK_CHECK_MENU_ITEM (menu_item), 
                                  guw->caption_show[CHARMAP_CAPTION_POUNDS]);
  g_signal_connect (G_OBJECT (menu_item), "activate",
                    G_CALLBACK (show_hide_caption_pounds), guw);

  menu_item = gtk_check_menu_item_new_with_mnemonic (_("_Equivalents"));
  gtk_menu_shell_append (GTK_MENU_SHELL (nameslist_details_menu), menu_item);
  gtk_check_menu_item_set_active (GTK_CHECK_MENU_ITEM (menu_item), 
                                  guw->caption_show[CHARMAP_CAPTION_COLONS]);
  g_signal_connect (G_OBJECT (menu_item), "activate",
                    G_CALLBACK (show_hide_caption_colons), guw);

#if ENABLE_UNIHAN
  /* separator */
  gtk_menu_shell_append (GTK_MENU_SHELL (view_menu), gtk_menu_item_new ());

  menu_item = gtk_check_menu_item_new_with_mnemonic (
          _("_CJK Ideograph Details"));
  g_signal_connect (G_OBJECT (menu_item), "activate",
                    G_CALLBACK (show_hide_unihan), guw);
  gtk_menu_shell_append (GTK_MENU_SHELL (view_menu), menu_item);

  /* the unihan details submenu */
  guw->unihan_options_menu_item = gtk_menu_item_new_with_mnemonic (_("Options"));
  gtk_menu_shell_append (GTK_MENU_SHELL (view_menu), 
                         guw->unihan_options_menu_item);
  gtk_widget_set_sensitive (guw->unihan_options_menu_item, FALSE);

  unihan_details_menu = gtk_menu_new ();
  gtk_menu_item_set_submenu (GTK_MENU_ITEM (guw->unihan_options_menu_item), 
                             unihan_details_menu);

  menu_item = gtk_check_menu_item_new_with_mnemonic (
          _("CJK Ideograph _Definition"));
  gtk_menu_shell_append (GTK_MENU_SHELL (unihan_details_menu), menu_item);
  gtk_check_menu_item_set_active (
          GTK_CHECK_MENU_ITEM (menu_item), 
          guw->caption_show[CHARMAP_CAPTION_KDEFINITION]);
  g_signal_connect (G_OBJECT (menu_item), "activate",
                    G_CALLBACK (show_hide_caption_kdefinition), guw);

  menu_item = gtk_check_menu_item_new_with_mnemonic (
          _("_Mandarin Pronunciation"));
  gtk_menu_shell_append (GTK_MENU_SHELL (unihan_details_menu), menu_item);
  gtk_check_menu_item_set_active (
          GTK_CHECK_MENU_ITEM (menu_item), 
          guw->caption_show[CHARMAP_CAPTION_KMANDARIN]);
  g_signal_connect (G_OBJECT (menu_item), "activate",
                    G_CALLBACK (show_hide_caption_kmandarin), guw);

  menu_item = gtk_check_menu_item_new_with_mnemonic (
          _("Japanese _On Pronunciation"));
  gtk_menu_shell_append (GTK_MENU_SHELL (unihan_details_menu), menu_item);
  gtk_check_menu_item_set_active (
          GTK_CHECK_MENU_ITEM (menu_item), 
          guw->caption_show[CHARMAP_CAPTION_KJAPANESEON]);
  g_signal_connect (G_OBJECT (menu_item), "activate",
                    G_CALLBACK (show_hide_caption_kjapaneseon), guw);

  menu_item = gtk_check_menu_item_new_with_mnemonic (
          _("_Japanese Kun Pronunciation"));
  gtk_menu_shell_append (GTK_MENU_SHELL (unihan_details_menu), menu_item);
  gtk_check_menu_item_set_active (
          GTK_CHECK_MENU_ITEM (menu_item), 
          guw->caption_show[CHARMAP_CAPTION_KJAPANESEKUN]);
  g_signal_connect (G_OBJECT (menu_item), "activate",
                    G_CALLBACK (show_hide_caption_kjapanesekun), guw);

  menu_item = gtk_check_menu_item_new_with_mnemonic (
          _("_Cantonese Pronunciation"));
  gtk_menu_shell_append (GTK_MENU_SHELL (unihan_details_menu), menu_item);
  gtk_check_menu_item_set_active (
          GTK_CHECK_MENU_ITEM (menu_item), 
          guw->caption_show[CHARMAP_CAPTION_KCANTONESE]);
  g_signal_connect (G_OBJECT (menu_item), "activate",
                    G_CALLBACK (show_hide_caption_kcantonese), guw);

  menu_item = gtk_check_menu_item_new_with_mnemonic (
          _("_Tang Pronunciation"));
  gtk_menu_shell_append (GTK_MENU_SHELL (unihan_details_menu), menu_item);
  gtk_check_menu_item_set_active (GTK_CHECK_MENU_ITEM (menu_item), 
                                  guw->caption_show[CHARMAP_CAPTION_KTANG]);
  g_signal_connect (G_OBJECT (menu_item), "activate",
                    G_CALLBACK (show_hide_caption_ktang), guw);

  menu_item = gtk_check_menu_item_new_with_mnemonic (
          _("_Korean Pronunciation"));
  gtk_menu_shell_append (GTK_MENU_SHELL (unihan_details_menu), menu_item);
  gtk_check_menu_item_set_active (GTK_CHECK_MENU_ITEM (menu_item), 
                                  guw->caption_show[CHARMAP_CAPTION_KKOREAN]);
  g_signal_connect (G_OBJECT (menu_item), "activate",
                    G_CALLBACK (show_hide_caption_kkorean), guw);
#endif

  /* make the search menu */
  search_menu = gtk_menu_new ();
  gtk_menu_set_accel_group (GTK_MENU (search_menu), guw->accel_group);
  gtk_menu_item_set_submenu (GTK_MENU_ITEM (search_menu_item), search_menu);

  /* ctrl-f */
  menu_item = gtk_image_menu_item_new_with_mnemonic (_("_Find..."));
  gtk_image_menu_item_set_image (
          GTK_IMAGE_MENU_ITEM (menu_item), 
          gtk_image_new_from_stock (GTK_STOCK_FIND, GTK_ICON_SIZE_MENU));
  gtk_widget_add_accelerator (menu_item, "activate", guw->accel_group,
                              GDK_f, GDK_CONTROL_MASK, GTK_ACCEL_VISIBLE);
  g_signal_connect (G_OBJECT (menu_item), "activate",
                    G_CALLBACK (search_find), guw);
  gtk_menu_shell_append (GTK_MENU_SHELL (search_menu), menu_item);

  /* ctrl-g */
  menu_item = gtk_image_menu_item_new_with_mnemonic (_("Find _Next"));
  gtk_image_menu_item_set_image (
          GTK_IMAGE_MENU_ITEM (menu_item), 
          gtk_image_new_from_stock (GTK_STOCK_FIND, GTK_ICON_SIZE_MENU));
  gtk_widget_add_accelerator (menu_item, "activate", guw->accel_group,
                              GDK_g, GDK_CONTROL_MASK, GTK_ACCEL_VISIBLE);
  g_signal_connect (G_OBJECT (menu_item), "activate",
                    G_CALLBACK (search_find_next), guw);
  gtk_menu_shell_append (GTK_MENU_SHELL (search_menu), menu_item);

  /* shift-ctrl-g */
  menu_item = gtk_image_menu_item_new_with_mnemonic (_("Find _Previous"));
  gtk_image_menu_item_set_image (
          GTK_IMAGE_MENU_ITEM (menu_item), 
          gtk_image_new_from_stock (GTK_STOCK_FIND, GTK_ICON_SIZE_MENU));
  gtk_widget_add_accelerator (menu_item, "activate", guw->accel_group,
                              GDK_g, GDK_SHIFT_MASK | GDK_CONTROL_MASK, 
                              GTK_ACCEL_VISIBLE);
  g_signal_connect (G_OBJECT (menu_item), "activate",
                    G_CALLBACK (search_find_prev), guw);
  gtk_menu_shell_append (GTK_MENU_SHELL (search_menu), menu_item);
  /* finished making the search menu */

  /* separator */
  gtk_menu_shell_append (GTK_MENU_SHELL (search_menu), gtk_menu_item_new ());

  /* ctrl-h */
  menu_item = gtk_menu_item_new_with_mnemonic (_("_Hex Code Point..."));
  gtk_widget_add_accelerator (menu_item, "activate", guw->accel_group,
                              GDK_h, GDK_CONTROL_MASK, GTK_ACCEL_VISIBLE);
  g_signal_connect (G_OBJECT (menu_item), "activate",
                    G_CALLBACK (jump_code_point), guw);
  gtk_menu_shell_append (GTK_MENU_SHELL (search_menu), menu_item);

  /* ctrl-p */
  menu_item = gtk_menu_item_new_with_mnemonic (_("Character in _Clipboard"));
  gtk_widget_add_accelerator (menu_item, "activate", guw->accel_group,
                              GDK_p, GDK_CONTROL_MASK, GTK_ACCEL_VISIBLE);
  g_signal_connect (G_OBJECT (menu_item), "activate",
                    G_CALLBACK (jump_clipboard), guw); 
  gtk_menu_shell_append (GTK_MENU_SHELL (search_menu), menu_item);
  /* finished making the search menu */

#if HAVE_GNOME
  /* make the help menu */
  help_menu_item = gtk_menu_item_new_with_mnemonic (_("_Help"));
  gtk_menu_shell_append (GTK_MENU_SHELL (menubar), help_menu_item);
  gtk_menu_item_set_submenu (GTK_MENU_ITEM (help_menu_item), 
                             make_gnome_help_menu (guw));
  /* finished making the help menu */
#endif /* #if HAVE_GNOME */

  gtk_widget_show_all (menubar);
  return menubar;
}


static void
fontsel_changed (MiniFontSelection *fontsel, GucharmapWindow *guw)
{
  gchar *font_name = mini_font_selection_get_font_name (fontsel);

  chartable_set_font (guw->charmap->chartable, font_name);

  g_free (font_name);
}


static void
append_character_to_text_to_copy (Chartable *chartable, 
                                  gunichar uc, 
                                  GucharmapWindow *guw)
{
  GString *gs;
  gchar ubuf[7];
  gint n;

  /* don't do anything if text_to_copy is not active */
  if (! guw->text_to_copy_visible)
    return;

  if (! unichar_validate (uc))
    {
      set_status (guw, _("The selected code point is not a valid unicode character."));
      return;
    }

  n = g_unichar_to_utf8 (uc, ubuf);
  ubuf[n] = '\0';

  gs = g_string_new (gtk_entry_get_text (GTK_ENTRY (guw->text_to_copy_entry)));
  g_string_append (gs, ubuf);

  gtk_entry_set_text (GTK_ENTRY (guw->text_to_copy_entry), gs->str);

  g_string_free (gs, TRUE);

  set_status (guw, NULL);
}


static gint
edit_copy (GtkWidget *widget, GucharmapWindow *guw)
{
  /* if nothing is selected, select the whole thing */
  if (! gtk_editable_get_selection_bounds (
              GTK_EDITABLE (guw->text_to_copy_entry), NULL, NULL))
    gtk_editable_select_region (GTK_EDITABLE (guw->text_to_copy_entry), 0, -1);

  gtk_editable_copy_clipboard (GTK_EDITABLE (guw->text_to_copy_entry));

  set_status (guw, _("Text copied to clipboard."));

  return TRUE;
}


static void
entry_changed_sensitize_button (GtkEditable *editable, GtkWidget *button)
{
  const gchar *entry_text = gtk_entry_get_text (GTK_ENTRY (editable));
  gtk_widget_set_sensitive (button, entry_text[0] != '\0');
}


static GtkWidget *
make_text_to_copy (GucharmapWindow *guw)
{
  GtkWidget *button;
  GtkWidget *label;
  GtkWidget *hbox;
  GtkTooltips *tooltips;

  tooltips = gtk_tooltips_new ();

  hbox = gtk_hbox_new (FALSE, 6);

  label = gtk_label_new_with_mnemonic (_("_Text to copy:"));
  gtk_box_pack_start (GTK_BOX (hbox), label, FALSE, FALSE, 0);
  gtk_widget_show (label);

  guw->text_to_copy_entry = gtk_entry_new ();
  gtk_box_pack_start (GTK_BOX (hbox), guw->text_to_copy_entry, TRUE, TRUE, 0);
  gtk_widget_show (guw->text_to_copy_entry);

  gtk_label_set_mnemonic_widget (GTK_LABEL (label), guw->text_to_copy_entry);

  /* the copy button */
  button = gtk_button_new_from_stock (GTK_STOCK_COPY); 
  gtk_widget_show (button);
  gtk_widget_set_sensitive (button, FALSE);
  g_signal_connect (G_OBJECT (button), "clicked",
                    G_CALLBACK (edit_copy), guw);
  gtk_box_pack_start (GTK_BOX (hbox), button, FALSE, FALSE, 0);

  g_signal_connect (G_OBJECT (guw->text_to_copy_entry), "changed",
                    G_CALLBACK (entry_changed_sensitize_button), button);

  gtk_tooltips_set_tip (tooltips, button, _("Copy to the clipboard."), NULL);

  return hbox;
}


void
load_icon (GucharmapWindow *guw)
{
  GError *error = NULL;

#ifdef G_PLATFORM_WIN32

  gchar *package_root, *icon_path;

  package_root = g_win32_get_package_installation_directory (NULL, NULL);
  icon_path = g_build_filename (package_root, "share", 
                                "pixmaps", "gucharmap.png");
  guw->icon = gdk_pixbuf_new_from_file (icon_path, &error);
  g_free (package_root);
  g_free (icon_path);

#else  /* #ifdef G_PLATFORM_WIN32 */

  guw->icon = gdk_pixbuf_new_from_file (ICON_PATH, &error);

#endif /* #ifdef G_PLATFORM_WIN32 */

  if (error != NULL)
    {
      g_assert (guw->icon == NULL);
      g_warning ("Error loading icon: %s\n", error->message);
      g_error_free (error);
    }
  else
    gtk_window_set_icon (GTK_WINDOW (guw), guw->icon);
}


void
pack_stuff_in_window (GucharmapWindow *guw)
{
  GtkWidget *big_vbox;

  guw->charmap = CHARMAP (charmap_new ());

  big_vbox = gtk_vbox_new (FALSE, 0);
  gtk_container_add (GTK_CONTAINER (guw), big_vbox);

  gtk_box_pack_start (GTK_BOX (big_vbox), make_menu (guw), FALSE, FALSE, 0);

  guw->fontsel = mini_font_selection_new ();
  g_signal_connect (guw->fontsel, "changed", 
                    G_CALLBACK (fontsel_changed), guw);

  gtk_box_pack_start (GTK_BOX (big_vbox), guw->fontsel, FALSE, FALSE, 0);

  gtk_widget_show (GTK_WIDGET (guw->charmap));
  gtk_box_pack_start (GTK_BOX (big_vbox), GTK_WIDGET (guw->charmap), 
                      TRUE, TRUE, 0);

  guw->text_to_copy_container = make_text_to_copy (guw);
  gtk_container_set_border_width (GTK_CONTAINER (guw->text_to_copy_container), 
                                  6);
  gtk_box_pack_start (GTK_BOX (big_vbox), guw->text_to_copy_container, 
                      FALSE, FALSE, 0);
  g_signal_connect (guw->charmap->chartable, "activate", 
                    G_CALLBACK (append_character_to_text_to_copy), guw);

  guw->status = gtk_statusbar_new ();
  gtk_box_pack_start (GTK_BOX (big_vbox), guw->status, FALSE, FALSE, 0);
  gtk_widget_show (guw->status);

  g_signal_connect (guw->charmap, "status-message",
                    G_CALLBACK (status_message), guw);

  gtk_widget_show (big_vbox);
}


static void
gucharmap_window_init (GucharmapWindow *guw)
{
  gtk_window_set_title (GTK_WINDOW (guw), _("Unicode Character Map"));

  guw->font_selection_visible = FALSE;
  guw->text_to_copy_visible = FALSE;
  guw->last_search = NULL;

  /* which captions to show by default, when enabled */
  guw->caption_show[CHARMAP_CAPTION_CATEGORY] = TRUE;
  guw->caption_show[CHARMAP_CAPTION_DECOMPOSITION] = TRUE;
  guw->caption_show[CHARMAP_CAPTION_STARS] = TRUE;
  guw->caption_show[CHARMAP_CAPTION_EXES] = TRUE;
#if ENABLE_UNIHAN
  guw->caption_show[CHARMAP_CAPTION_KDEFINITION] = TRUE;
  guw->caption_show[CHARMAP_CAPTION_KMANDARIN] = TRUE;
#endif

  load_icon (guw);
  gtk_window_set_icon (GTK_WINDOW (guw), guw->icon);

  pack_stuff_in_window (guw);
}


static void
show_all (GtkWidget *widget)
{
  gtk_widget_show (widget);
}


static void
gucharmap_window_class_init (GucharmapWindowClass *clazz)
{
  GTK_WIDGET_CLASS (clazz)->show_all = show_all;
}


GType 
gucharmap_window_get_type ()
{
  static GType gucharmap_window_type = 0;

  if (! gucharmap_window_type)
    {
      static const GTypeInfo gucharmap_window_info =
        {
          sizeof (GucharmapWindowClass),
          NULL,
          NULL,
          (GClassInitFunc) gucharmap_window_class_init,
          NULL,
          NULL,
          sizeof (GucharmapWindow),
          0,
          (GInstanceInitFunc) gucharmap_window_init,
        };

      gucharmap_window_type = g_type_register_static (GTK_TYPE_WINDOW,
                                                      "GucharmapWindow",
                                                      &gucharmap_window_info,
                                                      0);
    }

  return gucharmap_window_type;
}


GtkWidget * 
gucharmap_window_new ()
{
  return GTK_WIDGET (g_object_new (gucharmap_window_get_type (), NULL));
}


void 
gucharmap_window_set_font_selection_visible (GucharmapWindow *guw, 
                                             gboolean visible)
{
  guw->font_selection_visible = visible;

  if (guw->font_selection_visible)
    gtk_widget_show (guw->fontsel);
  else
    gtk_widget_hide (guw->fontsel);
}


void 
gucharmap_window_set_text_to_copy_visible (GucharmapWindow *guw, 
                                           gboolean visible)
{
  guw->text_to_copy_visible = visible;

  if (guw->text_to_copy_visible)
    gtk_widget_show (guw->text_to_copy_container);
  else
    gtk_widget_hide (guw->text_to_copy_container);
}


