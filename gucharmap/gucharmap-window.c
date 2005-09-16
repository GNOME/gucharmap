/* $Id$ */
/*
 * Copyright (c) 2004 Noah Levitt
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

#include "config.h"

#include <stdlib.h>
#include <string.h>
#include <gtk/gtk.h>
#include <gdk/gdkkeysyms.h>
#if HAVE_GNOME
# include <gnome.h>
#endif
#include "gucharmap-window.h"
#include "gucharmap-mini-fontsel.h"
#include "gucharmap-unicode-info.h"
#include "gucharmap-script-chapters.h"
#include "gucharmap-block-chapters.h"
#include "gucharmap-intl.h"
#include "gucharmap-search-dialog.h"

#ifndef ICON_PATH
# define ICON_PATH ""
#endif

#define GUCHARMAP_WINDOW_GET_PRIVATE(o) \
            (G_TYPE_INSTANCE_GET_PRIVATE ((o), gucharmap_window_get_type (), GucharmapWindowPrivate))

typedef enum
{
  CHAPTERS_SCRIPT,
  CHAPTERS_BLOCK
}
ChaptersMode;

typedef struct _GucharmapWindowPrivate GucharmapWindowPrivate;

struct _GucharmapWindowPrivate
{
  GtkWidget *status;
  GtkAccelGroup *accel_group;

  GtkWidget *fontsel;
  GtkWidget *text_to_copy_container; /* the thing to show/hide */
  GtkWidget *text_to_copy_entry;

  GtkWidget *file_menu_item;
  GtkWidget *quit_menu_item;
  GtkWidget *next_chapter_menu_item;
  GtkWidget *prev_chapter_menu_item;

  GdkPixbuf *icon;

  GtkWidget *search_dialog; /* takes care of all aspects of searching */

  GtkWidget *progress;

  /* menu items to disable while searching */
  GtkWidget *find_menu_item;
  GtkWidget *find_next_menu_item;
  GtkWidget *find_prev_menu_item;

  gboolean font_selection_visible;
  gboolean text_to_copy_visible;
  gboolean file_menu_visible;

  ChaptersMode chapters_mode; 
};

static GtkWindowClass *parent_class = NULL;

static void
status_message (GtkWidget       *widget, 
                const gchar     *message, 
                GucharmapWindow *guw)
{
  GucharmapWindowPrivate *priv = GUCHARMAP_WINDOW_GET_PRIVATE (guw);

  gtk_statusbar_pop (GTK_STATUSBAR (priv->status), 0); 

  if (message)
    gtk_statusbar_push (GTK_STATUSBAR (priv->status), 0, message);
}

static gboolean
update_progress_bar (GucharmapWindow *guw)
{
  GucharmapWindowPrivate *priv = GUCHARMAP_WINDOW_GET_PRIVATE (guw);
  gdouble fraction_completed;

  fraction_completed = gucharmap_search_dialog_get_completed (GUCHARMAP_SEARCH_DIALOG (priv->search_dialog));

  if (fraction_completed < 0 || fraction_completed > 1)
    {
      gtk_progress_bar_set_fraction (GTK_PROGRESS_BAR (priv->progress), 0);
      gtk_progress_bar_set_text (GTK_PROGRESS_BAR (priv->progress), NULL);
      return FALSE;
    }
  else
    {
      gtk_progress_bar_set_fraction (GTK_PROGRESS_BAR (priv->progress), fraction_completed);
      return TRUE;
    }
}

/* "progress" aka "busy-interactive" cursor (pointer + watch)
 * from mozilla 
 * caller should gdk_cursor_unref */
GdkCursor *
_gucharmap_window_progress_cursor (void)
{
  /* MOZ_CURSOR_SPINNING */
  static const char moz_spinning_bits[] = 
    {
      0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x04, 0x00, 0x00,
      0x00, 0x0c, 0x00, 0x00, 0x00, 0x1c, 0x00, 0x00, 0x00, 0x3c, 0x00,
      0x00, 0x00, 0x7c, 0x00, 0x00, 0x00, 0xfc, 0x00, 0x00, 0x00, 0xfc,
      0x01, 0x00, 0x00, 0xfc, 0x3b, 0x00, 0x00, 0x7c, 0x38, 0x00, 0x00,
      0x6c, 0x54, 0x00, 0x00, 0xc4, 0xdc, 0x00, 0x00, 0xc0, 0x44, 0x00,
      0x00, 0x80, 0x39, 0x00, 0x00, 0x80, 0x39, 0x00, 0x00, 0x00, 0x00,
      0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
      0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
      0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
      0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
      0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
      0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00 
    };
  static const char moz_spinning_mask_bits[] = 
    {
      0x00, 0x00, 0x00, 0x00, 0x06, 0x00, 0x00, 0x00, 0x0e, 0x00, 0x00,
      0x00, 0x1e, 0x00, 0x00, 0x00, 0x3e, 0x00, 0x00, 0x00, 0x7e, 0x00,
      0x00, 0x00, 0xfe, 0x00, 0x00, 0x00, 0xfe, 0x01, 0x00, 0x00, 0xfe,
      0x3b, 0x00, 0x00, 0xfe, 0x7f, 0x00, 0x00, 0xfe, 0x7f, 0x00, 0x00,
      0xfe, 0xfe, 0x00, 0x00, 0xee, 0xff, 0x01, 0x00, 0xe4, 0xff, 0x00,
      0x00, 0xc0, 0x7f, 0x00, 0x00, 0xc0, 0x7f, 0x00, 0x00, 0x80, 0x39,
      0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
      0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
      0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
      0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
      0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
      0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00 
    };

  GdkPixmap *cursor, *mask;
  GdkCursor *gdkcursor;
  GdkColor fg = { 0, 0, 0, 0 };             /* black */
  GdkColor bg = { 0, 65535, 65535, 65535 }; /* white */

  cursor = gdk_bitmap_create_from_data (NULL, moz_spinning_bits, 32, 32);
  mask = gdk_bitmap_create_from_data (NULL, moz_spinning_mask_bits, 32, 32);

  gdkcursor = gdk_cursor_new_from_pixmap (cursor, mask, &fg, &bg, 2, 2);

  gdk_bitmap_unref (cursor);
  gdk_bitmap_unref (mask);

  return gdkcursor;
}

static void
search_start (GucharmapSearchDialog *search_dialog,
              GucharmapWindow       *guw)
{
  GdkCursor *cursor;
  GucharmapWindowPrivate *priv = GUCHARMAP_WINDOW_GET_PRIVATE (guw);
  g_assert (IS_GUCHARMAP_WINDOW (guw));

  cursor = _gucharmap_window_progress_cursor ();
  gdk_window_set_cursor (GTK_WIDGET (guw)->window, cursor);
  gdk_cursor_unref (cursor);

  gtk_widget_set_sensitive (priv->find_menu_item, FALSE);
  gtk_widget_set_sensitive (priv->find_next_menu_item, FALSE);
  gtk_widget_set_sensitive (priv->find_prev_menu_item, FALSE);

  gtk_progress_bar_set_text (GTK_PROGRESS_BAR (priv->progress), _("Searching..."));
  g_timeout_add (100, (GSourceFunc) update_progress_bar, guw);
}

static void
search_finish (GucharmapSearchDialog *search_dialog,
               gunichar               found_char,
               GucharmapWindow       *guw)
{
  GucharmapWindowPrivate *priv = GUCHARMAP_WINDOW_GET_PRIVATE (guw);

  gtk_progress_bar_set_fraction (GTK_PROGRESS_BAR (priv->progress), 0);
  gtk_progress_bar_set_text (GTK_PROGRESS_BAR (priv->progress), NULL);

  if (found_char != (gunichar)(-1))
    gucharmap_charmap_go_to_character (guw->charmap, found_char);
  /* not-found dialog handled by GucharmapSearchDialog */

  gdk_window_set_cursor (GTK_WIDGET (guw)->window, NULL);

  gtk_widget_set_sensitive (priv->find_menu_item, TRUE);
  gtk_widget_set_sensitive (priv->find_next_menu_item, TRUE);
  gtk_widget_set_sensitive (priv->find_prev_menu_item, TRUE);
}

static void
search_find (GtkWidget       *widget, 
             GucharmapWindow *guw)
{
  GucharmapWindowPrivate *priv = GUCHARMAP_WINDOW_GET_PRIVATE (guw);
  g_assert (IS_GUCHARMAP_WINDOW (guw));

  if (priv->search_dialog == NULL)
    {
      priv->search_dialog = gucharmap_search_dialog_new (guw);
      g_signal_connect (priv->search_dialog, "search-start", G_CALLBACK (search_start), guw);
      g_signal_connect (priv->search_dialog, "search-finish", G_CALLBACK (search_finish), guw);
    }

  gtk_window_present (GTK_WINDOW (priv->search_dialog));
}

static void
search_find_next (GtkWidget       *widget, 
                  GucharmapWindow *guw)
{
  GucharmapWindowPrivate *priv = GUCHARMAP_WINDOW_GET_PRIVATE (guw);

  if (priv->search_dialog)
    gucharmap_search_dialog_start_search (GUCHARMAP_SEARCH_DIALOG (priv->search_dialog), GUCHARMAP_DIRECTION_FORWARD);
  else
    search_find (widget, guw);
}

static void
search_find_prev (GtkWidget       *widget, 
                  GucharmapWindow *guw)
{
  GucharmapWindowPrivate *priv = GUCHARMAP_WINDOW_GET_PRIVATE (guw);

  if (priv->search_dialog)
    gucharmap_search_dialog_start_search (GUCHARMAP_SEARCH_DIALOG (priv->search_dialog), GUCHARMAP_DIRECTION_BACKWARD);
  else
    search_find (widget, guw);
}

static void
font_bigger (GtkWidget       *widget, 
             GucharmapWindow *guw)
{
  GucharmapWindowPrivate *priv = GUCHARMAP_WINDOW_GET_PRIVATE (guw);
  gint size, increment;

  size = gucharmap_mini_font_selection_get_font_size (GUCHARMAP_MINI_FONT_SELECTION (priv->fontsel));
  increment = MAX (size / 5, 1);
  gucharmap_mini_font_selection_set_font_size (GUCHARMAP_MINI_FONT_SELECTION (priv->fontsel), size + increment);
}

static void
font_smaller (GtkWidget       *widget, 
              GucharmapWindow *guw)
{
  GucharmapWindowPrivate *priv = GUCHARMAP_WINDOW_GET_PRIVATE (guw);
  gint size, increment;

  size = gucharmap_mini_font_selection_get_font_size (GUCHARMAP_MINI_FONT_SELECTION (priv->fontsel));
  increment = MAX (size / 5, 1);
  gucharmap_mini_font_selection_set_font_size (GUCHARMAP_MINI_FONT_SELECTION (priv->fontsel), size - increment);
}

static void
font_default (GtkWidget       *widget, 
              GucharmapWindow *guw)
{
  GucharmapWindowPrivate *priv = GUCHARMAP_WINDOW_GET_PRIVATE (guw);
  gucharmap_mini_font_selection_reset_font_size (GUCHARMAP_MINI_FONT_SELECTION (priv->fontsel));
}

static void
snap_cols_pow2 (GtkCheckMenuItem *mi, 
                GucharmapWindow  *guw)
{
  gucharmap_table_set_snap_pow2 (guw->charmap->chartable, gtk_check_menu_item_get_active (mi));
}

#if HAVE_GNOME
static void
help_about (GtkWidget       *widget, 
            GucharmapWindow *guw)
{
  GucharmapWindowPrivate *priv = GUCHARMAP_WINDOW_GET_PRIVATE (guw);
  static GtkWidget *about = NULL;

  if (about == NULL) 
    {
      const gchar *authors[] = 
        { 
          "Noah Levitt <nlevitt@columbia.edu>", 
          "Daniel Elstner <daniel.elstner@gmx.net>", 
          "Padraig O'Briain <Padraig.Obriain@sun.com>",
          NULL 
        };
      const gchar *documenters[] =
	{
	  "Chee Bin HOH <cbhoh@gnome.org>",
          "Sun Microsystems",
          NULL
	};	  
      const gchar *translator_credits;

      translator_credits = _("translator_credits");
      if (strcmp (translator_credits, "translator_credits") == 0)
        translator_credits = NULL;

      about = gnome_about_new ("gucharmap", VERSION, "Copyright © 2004 Noah Levitt <nlevitt@columbia.edu>",
                               _("Character Map"), authors, documenters, translator_credits, priv->icon);

      /* set the widget pointer to NULL when the widget is destroyed */
      g_signal_connect (G_OBJECT (about), "destroy", G_CALLBACK (gtk_widget_destroyed), &about);
      gtk_window_set_icon (GTK_WINDOW (about), priv->icon);
    }

  gtk_window_present (GTK_WINDOW (about));
}

static GtkWidget *
make_gnome_help_menu (GucharmapWindow *guw)
{
  GucharmapWindowPrivate *priv = GUCHARMAP_WINDOW_GET_PRIVATE (guw);
  GnomeUIInfo help_menu[] =
  {
    GNOMEUIINFO_HELP (PACKAGE),
    GNOMEUIINFO_MENU_ABOUT_ITEM (help_about, guw),
    GNOMEUIINFO_END
  };
  GtkWidget *menu;

  menu = gtk_menu_new ();

  gnome_app_fill_menu (GTK_MENU_SHELL (menu), help_menu, priv->accel_group, TRUE, 0);

  return menu;
}
#endif /* #if HAVE_GNOME */

static void
prev_character (GtkWidget       *button,
                GucharmapWindow *guw)
{
  gint index = guw->charmap->chartable->active_cell;
  gunichar wc;

  do
    {
      index--;

      if (index <= 0)
        index = gucharmap_codepoint_list_get_last_index (guw->charmap->chartable->codepoint_list);

      wc = gucharmap_codepoint_list_get_char (guw->charmap->chartable->codepoint_list, index);
    }
  while (!gucharmap_unichar_isdefined (wc) || !gucharmap_unichar_validate (wc));

  gucharmap_table_set_active_character (guw->charmap->chartable, wc);
}

static void
next_character (GtkWidget       *button,
                GucharmapWindow *guw)
{
  gint index = guw->charmap->chartable->active_cell;
  gunichar wc;

  do
    {
      index++;

      if (index >= gucharmap_codepoint_list_get_last_index (guw->charmap->chartable->codepoint_list))
        index = 0;

      wc = gucharmap_codepoint_list_get_char (guw->charmap->chartable->codepoint_list, index);
    }
  while (!gucharmap_unichar_isdefined (wc) || !gucharmap_unichar_validate (wc));

  gucharmap_table_set_active_character (guw->charmap->chartable, wc);
}

static void
next_chapter (GtkWidget       *button,
              GucharmapWindow *guw)
{
  GucharmapChapters *chapters = gucharmap_charmap_get_chapters (guw->charmap);
  gucharmap_chapters_next (chapters);
}

static void
prev_chapter (GtkWidget       *button,
              GucharmapWindow *guw)
{
  GucharmapChapters *chapters = gucharmap_charmap_get_chapters (guw->charmap);
  gucharmap_chapters_previous (chapters);
}

static void
view_by_script (GtkCheckMenuItem *check_menu_item,
                GucharmapWindow  *guw)
{
  if (gtk_check_menu_item_get_active (check_menu_item))
    {
      GucharmapWindowPrivate *priv = GUCHARMAP_WINDOW_GET_PRIVATE (guw);
      gucharmap_charmap_set_chapters (guw->charmap, GUCHARMAP_CHAPTERS (gucharmap_script_chapters_new ()));
      gtk_label_set_text (GTK_LABEL (gtk_bin_get_child (GTK_BIN (priv->next_chapter_menu_item))), _("Next Script"));
      gtk_label_set_text (GTK_LABEL (gtk_bin_get_child (GTK_BIN (priv->prev_chapter_menu_item))), _("Previous Script"));
    }
}

static void
view_by_block (GtkCheckMenuItem *check_menu_item,
                GucharmapWindow  *guw)
{
  if (gtk_check_menu_item_get_active (check_menu_item))
    {
      GucharmapWindowPrivate *priv = GUCHARMAP_WINDOW_GET_PRIVATE (guw);
      gucharmap_charmap_set_chapters (guw->charmap, GUCHARMAP_CHAPTERS (gucharmap_block_chapters_new ()));
      gtk_label_set_text (GTK_LABEL (gtk_bin_get_child (GTK_BIN (priv->next_chapter_menu_item))), _("Next Block"));
      gtk_label_set_text (GTK_LABEL (gtk_bin_get_child (GTK_BIN (priv->prev_chapter_menu_item))), _("Previous Block"));
    }
}

static GtkWidget *
make_menu (GucharmapWindow *guw)
{
  GucharmapWindowPrivate *priv = GUCHARMAP_WINDOW_GET_PRIVATE (guw);
  GtkWidget *menubar;
  GtkWidget *file_menu, *view_menu, *search_menu, *go_menu;
  GtkWidget *view_menu_item, *search_menu_item, *go_menu_item;
  GtkWidget *menu_item;
#if HAVE_GNOME
  GtkWidget *help_menu_item;
#endif
  guint forward_keysym, back_keysym;
  GSList *group = NULL; 

  if (gtk_widget_get_direction (GTK_WIDGET (guw)) == GTK_TEXT_DIR_RTL)
    {
      forward_keysym = GDK_Left;
      back_keysym = GDK_Right;
    }
  else
    {
      forward_keysym = GDK_Right;
      back_keysym = GDK_Left;
    }

  priv->accel_group = gtk_accel_group_new ();
  gtk_window_add_accel_group (GTK_WINDOW (guw), priv->accel_group);
  g_object_unref (priv->accel_group);

  /* make the menu bar */
  menubar = gtk_menu_bar_new ();
  priv->file_menu_item = gtk_menu_item_new_with_mnemonic (_("_File"));
  gtk_menu_shell_append (GTK_MENU_SHELL (menubar), priv->file_menu_item);
  view_menu_item = gtk_menu_item_new_with_mnemonic (_("_View"));
  gtk_menu_shell_append (GTK_MENU_SHELL (menubar), view_menu_item);
  search_menu_item = gtk_menu_item_new_with_mnemonic (_("_Search"));
  gtk_menu_shell_append (GTK_MENU_SHELL (menubar), search_menu_item);
  go_menu_item = gtk_menu_item_new_with_mnemonic (_("_Go"));
  gtk_menu_shell_append (GTK_MENU_SHELL (menubar), go_menu_item);
  /* finished making the menu bar */

  /* make the file menu */
  file_menu = gtk_menu_new ();
  gtk_menu_set_accel_group (GTK_MENU (file_menu), priv->accel_group);
  gtk_menu_item_set_submenu (GTK_MENU_ITEM (priv->file_menu_item), file_menu);
  priv->quit_menu_item = gtk_image_menu_item_new_from_stock (GTK_STOCK_QUIT, priv->accel_group);
  g_signal_connect (G_OBJECT (priv->quit_menu_item), "activate",
                    G_CALLBACK (gtk_main_quit), NULL);
  gtk_menu_shell_append (GTK_MENU_SHELL (file_menu), priv->quit_menu_item);
  /* finished making the file menu */

  /* make the view menu */
  view_menu = gtk_menu_new ();
  gtk_menu_set_accel_group (GTK_MENU (view_menu), priv->accel_group);
  gtk_menu_item_set_submenu (GTK_MENU_ITEM (view_menu_item), view_menu);

  menu_item = gtk_radio_menu_item_new_with_mnemonic (group, _("By _Script"));
  gtk_check_menu_item_set_active (GTK_CHECK_MENU_ITEM (menu_item), FALSE);
  gtk_menu_shell_append (GTK_MENU_SHELL (view_menu), menu_item);
  g_signal_connect (menu_item, "toggled", G_CALLBACK (view_by_script), guw);
  group = gtk_radio_menu_item_get_group (GTK_RADIO_MENU_ITEM (menu_item));

  menu_item = gtk_radio_menu_item_new_with_mnemonic (group, _("By _Unicode Block"));
  gtk_check_menu_item_set_active (GTK_CHECK_MENU_ITEM (menu_item), FALSE);
  gtk_menu_shell_append (GTK_MENU_SHELL (view_menu), menu_item);
  g_signal_connect (menu_item, "toggled", G_CALLBACK (view_by_block), guw);

  /* separator */
  gtk_menu_shell_append (GTK_MENU_SHELL (view_menu), gtk_menu_item_new ());

  menu_item = gtk_check_menu_item_new_with_label (_("Snap Columns to Power of Two"));
  g_signal_connect (menu_item,  "activate", G_CALLBACK (snap_cols_pow2), guw);
  gtk_menu_shell_append (GTK_MENU_SHELL (view_menu), menu_item);

  /* separator */
  gtk_menu_shell_append (GTK_MENU_SHELL (view_menu), gtk_menu_item_new ());

  /* ctrl-+ */
  menu_item = gtk_menu_item_new_with_mnemonic (_("Zoom _In"));
  gtk_widget_add_accelerator (menu_item, "activate", priv->accel_group,
                              GDK_plus, GDK_CONTROL_MASK, GTK_ACCEL_VISIBLE);
  gtk_widget_add_accelerator (menu_item, "activate", priv->accel_group,
                              GDK_KP_Add, GDK_CONTROL_MASK, 0);
  g_signal_connect (G_OBJECT (menu_item), "activate",
                    G_CALLBACK (font_bigger), guw);
  gtk_menu_shell_append (GTK_MENU_SHELL (view_menu), menu_item);

  /* ctrl-- */
  menu_item = gtk_menu_item_new_with_mnemonic (_("Zoom _Out"));
  gtk_widget_add_accelerator (menu_item, "activate", priv->accel_group,
                              GDK_minus, GDK_CONTROL_MASK, GTK_ACCEL_VISIBLE);
  gtk_widget_add_accelerator (menu_item, "activate", priv->accel_group,
                              GDK_KP_Subtract, GDK_CONTROL_MASK, 0);
  g_signal_connect (G_OBJECT (menu_item), "activate",
                    G_CALLBACK (font_smaller), guw);
  gtk_menu_shell_append (GTK_MENU_SHELL (view_menu), menu_item);

  /* ctrl-= */
  menu_item = gtk_menu_item_new_with_mnemonic (_("_Normal Size"));
  gtk_widget_add_accelerator (menu_item, "activate", priv->accel_group,
                              GDK_equal, GDK_CONTROL_MASK, GTK_ACCEL_VISIBLE);
  gtk_widget_add_accelerator (menu_item, "activate", priv->accel_group,
                              GDK_KP_Equal, GDK_CONTROL_MASK, 0);
  g_signal_connect (G_OBJECT (menu_item), "activate",
                    G_CALLBACK (font_default), guw);
  gtk_menu_shell_append (GTK_MENU_SHELL (view_menu), menu_item);

  /* make the search menu */
  search_menu = gtk_menu_new ();
  gtk_menu_set_accel_group (GTK_MENU (search_menu), priv->accel_group);
  gtk_menu_item_set_submenu (GTK_MENU_ITEM (search_menu_item), search_menu);

  priv->find_menu_item = gtk_image_menu_item_new_with_mnemonic (_("_Find..."));
  gtk_image_menu_item_set_image (GTK_IMAGE_MENU_ITEM (priv->find_menu_item), 
                                 gtk_image_new_from_stock (GTK_STOCK_FIND, GTK_ICON_SIZE_MENU));
  gtk_widget_add_accelerator (priv->find_menu_item, "activate", priv->accel_group,
                              GDK_f, GDK_CONTROL_MASK, GTK_ACCEL_VISIBLE);
  g_signal_connect (G_OBJECT (priv->find_menu_item), "activate", G_CALLBACK (search_find), guw);
  gtk_menu_shell_append (GTK_MENU_SHELL (search_menu), priv->find_menu_item);

  priv->find_next_menu_item = gtk_image_menu_item_new_with_mnemonic (_("Find _Next"));
  gtk_image_menu_item_set_image (GTK_IMAGE_MENU_ITEM (priv->find_next_menu_item), 
                                 gtk_image_new_from_stock (GTK_STOCK_FIND, GTK_ICON_SIZE_MENU));
  gtk_widget_add_accelerator (priv->find_next_menu_item, "activate", priv->accel_group,
                              GDK_g, GDK_CONTROL_MASK, GTK_ACCEL_VISIBLE);
  g_signal_connect (G_OBJECT (priv->find_next_menu_item), "activate",
                    G_CALLBACK (search_find_next), guw);
  gtk_menu_shell_append (GTK_MENU_SHELL (search_menu), priv->find_next_menu_item);

  priv->find_prev_menu_item = gtk_image_menu_item_new_with_mnemonic (_("Find _Previous"));
  gtk_image_menu_item_set_image (
          GTK_IMAGE_MENU_ITEM (priv->find_prev_menu_item), 
          gtk_image_new_from_stock (GTK_STOCK_FIND, GTK_ICON_SIZE_MENU));
  gtk_widget_add_accelerator (priv->find_prev_menu_item, "activate", priv->accel_group,
                              GDK_g, GDK_SHIFT_MASK | GDK_CONTROL_MASK, 
                              GTK_ACCEL_VISIBLE);
  g_signal_connect (G_OBJECT (priv->find_prev_menu_item), "activate",
                    G_CALLBACK (search_find_prev), guw);
  gtk_menu_shell_append (GTK_MENU_SHELL (search_menu), priv->find_prev_menu_item);
  /* finished making the search menu */

  /* make the go menu */
  go_menu = gtk_menu_new ();
  gtk_menu_set_accel_group (GTK_MENU (go_menu), priv->accel_group);
  gtk_menu_item_set_submenu (GTK_MENU_ITEM (go_menu_item), go_menu);

  menu_item = gtk_menu_item_new_with_mnemonic (_("_Next Character"));
  gtk_widget_add_accelerator (menu_item, "activate", priv->accel_group,
                              GDK_n, GDK_CONTROL_MASK, GTK_ACCEL_VISIBLE);
  g_signal_connect (G_OBJECT (menu_item), "activate", G_CALLBACK (next_character), guw);
  gtk_menu_shell_append (GTK_MENU_SHELL (go_menu), menu_item);

  menu_item = gtk_menu_item_new_with_mnemonic (_("_Previous Character"));
  gtk_widget_add_accelerator (menu_item, "activate", priv->accel_group,
                              GDK_p, GDK_CONTROL_MASK, GTK_ACCEL_VISIBLE);
  g_signal_connect (G_OBJECT (menu_item), "activate", G_CALLBACK (prev_character), guw);
  gtk_menu_shell_append (GTK_MENU_SHELL (go_menu), menu_item);

  /* separator */
  gtk_menu_shell_append (GTK_MENU_SHELL (go_menu), gtk_menu_item_new ());

  switch (priv->chapters_mode)
    {
      case CHAPTERS_SCRIPT:
        priv->next_chapter_menu_item = gtk_menu_item_new_with_label (_("Next Script"));
        priv->prev_chapter_menu_item = gtk_menu_item_new_with_label (_("Previous Script"));
        break;

      case CHAPTERS_BLOCK:
        priv->next_chapter_menu_item = gtk_menu_item_new_with_label (_("Next Block"));
        priv->prev_chapter_menu_item = gtk_menu_item_new_with_label (_("Previous Block"));
        break;

      default:
        g_assert_not_reached ();
    }

  gtk_widget_add_accelerator (priv->next_chapter_menu_item, "activate", priv->accel_group,
                              GDK_Page_Down, GDK_CONTROL_MASK, GTK_ACCEL_VISIBLE);
  g_signal_connect (priv->next_chapter_menu_item, "activate", G_CALLBACK (next_chapter), guw);
  gtk_menu_shell_append (GTK_MENU_SHELL (go_menu), priv->next_chapter_menu_item);

  gtk_widget_add_accelerator (priv->prev_chapter_menu_item, "activate", priv->accel_group,
                              GDK_Page_Up, GDK_CONTROL_MASK, GTK_ACCEL_VISIBLE);
  g_signal_connect (priv->prev_chapter_menu_item, "activate", G_CALLBACK (prev_chapter), guw);
  gtk_menu_shell_append (GTK_MENU_SHELL (go_menu), priv->prev_chapter_menu_item);

#if HAVE_GNOME
  /* if we are the input module and are running inside a non-gnome gtk+
   * program, doing gnome stuff will generate warnings */
  if (gnome_program_get () != NULL)
    {
      /* make the help menu */
      help_menu_item = gtk_menu_item_new_with_mnemonic (_("_Help"));
      gtk_menu_shell_append (GTK_MENU_SHELL (menubar), help_menu_item);
      gtk_menu_item_set_submenu (GTK_MENU_ITEM (help_menu_item), make_gnome_help_menu (guw));
      /* finished making the help menu */
    }
#endif /* #if HAVE_GNOME */

  gtk_widget_show_all (menubar);

  if (! priv->file_menu_visible)
    {
      gtk_widget_hide (priv->file_menu_item);
      gtk_widget_set_sensitive (priv->quit_menu_item, FALSE);
    }

  return menubar;
}

static void
fontsel_changed (GucharmapMiniFontSelection *fontsel, 
                 GucharmapWindow            *guw)
{
  gchar *font_name = gucharmap_mini_font_selection_get_font_name (fontsel);

  gucharmap_table_set_font (guw->charmap->chartable, font_name);

  g_free (font_name);
}

static void
insert_character_in_text_to_copy (GucharmapTable  *chartable, 
                                  gunichar         wc, 
                                  GucharmapWindow *guw)
{
  GucharmapWindowPrivate *priv = GUCHARMAP_WINDOW_GET_PRIVATE (guw);
  gchar ubuf[7];
  gint pos;

  g_return_if_fail (gucharmap_unichar_validate (wc));

  /* don't do anything if text_to_copy is not active */
  if (!priv->text_to_copy_visible)
    return;

  ubuf[g_unichar_to_utf8 (wc, ubuf)] = '\0';
  pos = gtk_editable_get_position (GTK_EDITABLE (priv->text_to_copy_entry));
  gtk_editable_insert_text (GTK_EDITABLE (priv->text_to_copy_entry), ubuf, -1, &pos);
  gtk_editable_set_position (GTK_EDITABLE (priv->text_to_copy_entry), pos);
}

static void
edit_copy (GtkWidget *widget, GucharmapWindow *guw)
{
  GucharmapWindowPrivate *priv = GUCHARMAP_WINDOW_GET_PRIVATE (guw);

  /* if nothing is selected, select the whole thing */
  if (! gtk_editable_get_selection_bounds (
              GTK_EDITABLE (priv->text_to_copy_entry), NULL, NULL))
    gtk_editable_select_region (GTK_EDITABLE (priv->text_to_copy_entry), 0, -1);

  gtk_editable_copy_clipboard (GTK_EDITABLE (priv->text_to_copy_entry));
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
  GucharmapWindowPrivate *priv = GUCHARMAP_WINDOW_GET_PRIVATE (guw);
  GtkWidget *button;
  GtkWidget *label;
  GtkWidget *hbox;
  GtkTooltips *tooltips;

  tooltips = gtk_tooltips_new ();

  hbox = gtk_hbox_new (FALSE, 6);

  label = gtk_label_new_with_mnemonic (_("_Text to copy:"));
  gtk_box_pack_start (GTK_BOX (hbox), label, FALSE, FALSE, 0);
  gtk_widget_show (label);

  priv->text_to_copy_entry = gtk_entry_new ();
  gtk_box_pack_start (GTK_BOX (hbox), priv->text_to_copy_entry, TRUE, TRUE, 0);
  gtk_widget_show (priv->text_to_copy_entry);

  gtk_label_set_mnemonic_widget (GTK_LABEL (label), priv->text_to_copy_entry);

  /* the copy button */
  button = gtk_button_new_from_stock (GTK_STOCK_COPY); 
  gtk_widget_show (button);
  gtk_widget_set_sensitive (button, FALSE);
  g_signal_connect (G_OBJECT (button), "clicked",
                    G_CALLBACK (edit_copy), guw);
  gtk_box_pack_start (GTK_BOX (hbox), button, FALSE, FALSE, 0);

  g_signal_connect (G_OBJECT (priv->text_to_copy_entry), "changed",
                    G_CALLBACK (entry_changed_sensitize_button), button);

  gtk_tooltips_set_tip (tooltips, button, _("Copy to the clipboard."), NULL);

  return hbox;
}

static void
load_icon (GucharmapWindow *guw)
{
  GucharmapWindowPrivate *priv = GUCHARMAP_WINDOW_GET_PRIVATE (guw);
  GError *error = NULL;

#ifdef G_PLATFORM_WIN32

  gchar *package_root, *icon_path;

  package_root = g_win32_get_package_installation_directory (NULL, NULL);
  icon_path = g_build_filename (package_root, "share", "pixmaps", "gucharmap.png");
  priv->icon = gdk_pixbuf_new_from_file (icon_path, &error);
  g_free (package_root);
  g_free (icon_path);

#else  /* #ifdef G_PLATFORM_WIN32 */

  priv->icon = gdk_pixbuf_new_from_file (ICON_PATH, &error);

#endif /* #ifdef G_PLATFORM_WIN32 */

  if (error != NULL)
    {
      g_assert (priv->icon == NULL);
      g_warning ("Error loading icon: %s\n", error->message);
      g_error_free (error);
    }
  else
    gtk_window_set_icon (GTK_WINDOW (guw), priv->icon);
}

static void
status_realize (GtkWidget       *status,
                GucharmapWindow *guw)
{
  GucharmapWindowPrivate *priv = GUCHARMAP_WINDOW_GET_PRIVATE (guw);

  /* increase the height a bit so it doesn't resize itself */
  gtk_widget_set_size_request (priv->status, -1, priv->status->allocation.height + 9);
}

static void
pack_stuff_in_window (GucharmapWindow *guw)
{
  GucharmapWindowPrivate *priv = GUCHARMAP_WINDOW_GET_PRIVATE (guw);
  GtkWidget *chapters;
  GtkWidget *big_vbox;
  GtkWidget *hbox;

  switch (priv->chapters_mode)
    {
      case CHAPTERS_SCRIPT:
        chapters = gucharmap_script_chapters_new ();
        break;

      case CHAPTERS_BLOCK:
        chapters = gucharmap_block_chapters_new ();
        break;

      default:
        g_assert_not_reached ();
    }
  guw->charmap = GUCHARMAP_CHARMAP (gucharmap_charmap_new (GUCHARMAP_CHAPTERS (chapters)));

  big_vbox = gtk_vbox_new (FALSE, 0);
  gtk_container_add (GTK_CONTAINER (guw), big_vbox);

  gtk_box_pack_start (GTK_BOX (big_vbox), make_menu (guw), FALSE, FALSE, 0);

  hbox = gtk_hbox_new (FALSE, 0);
  gtk_widget_show (hbox);
  gtk_box_pack_start (GTK_BOX (big_vbox), hbox, FALSE, FALSE, 0);
  gtk_box_pack_start (GTK_BOX (big_vbox), GTK_WIDGET (guw->charmap), 
                      TRUE, TRUE, 0);

  priv->fontsel = gucharmap_mini_font_selection_new ();
  g_signal_connect (priv->fontsel, "changed", G_CALLBACK (fontsel_changed), guw);
  gtk_box_pack_start (GTK_BOX (hbox), priv->fontsel, FALSE, FALSE, 0);
  gtk_widget_show (GTK_WIDGET (guw->charmap));

  priv->text_to_copy_container = make_text_to_copy (guw);
  gtk_container_set_border_width (GTK_CONTAINER (priv->text_to_copy_container), 6);
  gtk_box_pack_start (GTK_BOX (big_vbox), priv->text_to_copy_container, FALSE, FALSE, 0);
  g_signal_connect (guw->charmap->chartable, "activate", G_CALLBACK (insert_character_in_text_to_copy), guw);

  
  hbox = gtk_hbox_new (FALSE, 0);
  gtk_box_pack_start (GTK_BOX (big_vbox), hbox, FALSE, FALSE, 0);

  priv->status = gtk_statusbar_new ();
  gtk_statusbar_set_has_resize_grip (GTK_STATUSBAR (priv->status), FALSE);
  gtk_box_pack_start (GTK_BOX (hbox), priv->status, TRUE, TRUE, 0);
  gtk_widget_show (priv->status);
  g_signal_connect (priv->status, "realize", G_CALLBACK (status_realize), guw);

  priv->progress = gtk_progress_bar_new ();
  gtk_box_pack_start (GTK_BOX (hbox), priv->progress, FALSE, FALSE, 0);

#if 0
  grip = gtk_statusbar_new ();
  gtk_statusbar_set_has_resize_grip (GTK_STATUSBAR (grip), TRUE);
  gtk_box_pack_start (GTK_BOX (hbox), grip, FALSE, FALSE, 0);
#endif
  gtk_widget_show_all (hbox);

  g_signal_connect (guw->charmap, "status-message", G_CALLBACK (status_message), guw);

  gtk_widget_show (big_vbox);
}

static void
gucharmap_window_init (GucharmapWindow *guw)
{
  GucharmapWindowPrivate *priv = GUCHARMAP_WINDOW_GET_PRIVATE (guw);

  gtk_window_set_title (GTK_WINDOW (guw), _("Character Map"));

  priv->font_selection_visible = FALSE;
  priv->text_to_copy_visible = FALSE;
  priv->file_menu_visible = FALSE;
  priv->chapters_mode = CHAPTERS_SCRIPT;

  priv->search_dialog = NULL;

  load_icon (guw);
  gtk_window_set_icon (GTK_WINDOW (guw), priv->icon);

  pack_stuff_in_window (guw);
}

static void
show_all (GtkWidget *widget)
{
  gtk_widget_show (widget);
}

static void
window_finalize (GObject *object)
{
#if 0
  GucharmapWindow *guw = GUCHARMAP_WINDOW (object);
  GucharmapWindowPrivate *priv = GUCHARMAP_WINDOW_GET_PRIVATE (guw);

  /*
  if (priv->last_search)
    g_free (priv->last_search);
    */
#endif
  G_OBJECT_CLASS (parent_class)->finalize (object);
}

static void
gucharmap_window_class_init (GucharmapWindowClass *clazz)
{
  parent_class = g_type_class_peek_parent (clazz);
  GTK_WIDGET_CLASS (clazz)->show_all = show_all;
  G_OBJECT_CLASS (clazz)->finalize = window_finalize;
  g_type_class_add_private (clazz, sizeof (GucharmapWindowPrivate));
}

GType 
gucharmap_window_get_type (void)
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
          (GInstanceInitFunc) gucharmap_window_init
        };

      gucharmap_window_type = g_type_register_static (GTK_TYPE_WINDOW,
                                                      "GucharmapWindow",
                                                      &gucharmap_window_info,
                                                      0);
    }

  return gucharmap_window_type;
}

GtkWidget * 
gucharmap_window_new (void)
{
  return GTK_WIDGET (g_object_new (gucharmap_window_get_type (), NULL));
}

void 
gucharmap_window_set_font_selection_visible (GucharmapWindow *guw, 
                                             gboolean         visible)
{
  GucharmapWindowPrivate *priv = GUCHARMAP_WINDOW_GET_PRIVATE (guw);
  priv->font_selection_visible = visible;

  if (priv->font_selection_visible)
    gtk_widget_show (priv->fontsel);
  else
    gtk_widget_hide (priv->fontsel);
}

void 
gucharmap_window_set_text_to_copy_visible (GucharmapWindow *guw, 
                                           gboolean         visible)
{
  GucharmapWindowPrivate *priv = GUCHARMAP_WINDOW_GET_PRIVATE (guw);
  priv->text_to_copy_visible = visible;

  if (priv->text_to_copy_visible)
    gtk_widget_show (priv->text_to_copy_container);
  else
    gtk_widget_hide (priv->text_to_copy_container);
}

void 
gucharmap_window_set_file_menu_visible (GucharmapWindow *guw, 
                                        gboolean         visible)
{
  GucharmapWindowPrivate *priv = GUCHARMAP_WINDOW_GET_PRIVATE (guw);
  priv->file_menu_visible = visible;

  if (priv->file_menu_visible)
    {
      gtk_widget_show (priv->file_menu_item);
      gtk_widget_set_sensitive (priv->quit_menu_item, TRUE);
    }
  else
    {
      gtk_widget_hide (priv->file_menu_item);
      gtk_widget_set_sensitive (priv->quit_menu_item, FALSE);
    }
}

GucharmapMiniFontSelection *
gucharmap_window_get_mini_font_selection (GucharmapWindow *guw)
{
  GucharmapWindowPrivate *priv = GUCHARMAP_WINDOW_GET_PRIVATE (guw);
  return GUCHARMAP_MINI_FONT_SELECTION (priv->fontsel);
}
