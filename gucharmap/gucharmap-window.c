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
#ifdef HAVE_GNOME
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

  GtkUIManager *uimanager;

  GtkActionGroup *action_group;

  GtkWidget *search_dialog; /* takes care of all aspects of searching */

  GtkWidget *progress;

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
  GtkAction *action;
  GucharmapWindowPrivate *priv = GUCHARMAP_WINDOW_GET_PRIVATE (guw);

  g_assert (IS_GUCHARMAP_WINDOW (guw));

  cursor = _gucharmap_window_progress_cursor ();
  gdk_window_set_cursor (GTK_WIDGET (guw)->window, cursor);
  gdk_cursor_unref (cursor);

  action = gtk_action_group_get_action (priv->action_group, "Find");
  gtk_action_set_sensitive (action, FALSE);
  action = gtk_action_group_get_action (priv->action_group, "FindNext");
  gtk_action_set_sensitive (action, FALSE);
  action = gtk_action_group_get_action (priv->action_group, "FindPrevious");
  gtk_action_set_sensitive (action, FALSE);

  gtk_progress_bar_set_text (GTK_PROGRESS_BAR (priv->progress), _("Searching..."));
  g_timeout_add (100, (GSourceFunc) update_progress_bar, guw);
}

static void
search_finish (GucharmapSearchDialog *search_dialog,
               gunichar               found_char,
               GucharmapWindow       *guw)
{
  GucharmapWindowPrivate *priv = GUCHARMAP_WINDOW_GET_PRIVATE (guw);
  GtkAction *action;

  gtk_progress_bar_set_fraction (GTK_PROGRESS_BAR (priv->progress), 0);
  gtk_progress_bar_set_text (GTK_PROGRESS_BAR (priv->progress), NULL);

  if (found_char != (gunichar)(-1))
    gucharmap_charmap_go_to_character (guw->charmap, found_char);
  /* not-found dialog handled by GucharmapSearchDialog */

  gdk_window_set_cursor (GTK_WIDGET (guw)->window, NULL);

  action = gtk_action_group_get_action (priv->action_group, "Find");
  gtk_action_set_sensitive (action, TRUE);
  action = gtk_action_group_get_action (priv->action_group, "FindNext");
  gtk_action_set_sensitive (action, TRUE);
  action = gtk_action_group_get_action (priv->action_group, "FindPrevious");
  gtk_action_set_sensitive (action, TRUE);
}

static void
search_find (GtkAction       *action, 
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
search_find_next (GtkAction       *action, 
                  GucharmapWindow *guw)
{
  GucharmapWindowPrivate *priv = GUCHARMAP_WINDOW_GET_PRIVATE (guw);

  if (priv->search_dialog)
    gucharmap_search_dialog_start_search (GUCHARMAP_SEARCH_DIALOG (priv->search_dialog), GUCHARMAP_DIRECTION_FORWARD);
  else
    search_find (action, guw);
}

static void
search_find_prev (GtkAction       *action, 
                  GucharmapWindow *guw)
{
  GucharmapWindowPrivate *priv = GUCHARMAP_WINDOW_GET_PRIVATE (guw);

  if (priv->search_dialog)
    gucharmap_search_dialog_start_search (GUCHARMAP_SEARCH_DIALOG (priv->search_dialog), GUCHARMAP_DIRECTION_BACKWARD);
  else
    search_find (action, guw);
}

static void
font_bigger (GtkAction       *action, 
             GucharmapWindow *guw)
{
  GucharmapWindowPrivate *priv = GUCHARMAP_WINDOW_GET_PRIVATE (guw);
  gint size, increment;

  size = gucharmap_mini_font_selection_get_font_size (GUCHARMAP_MINI_FONT_SELECTION (priv->fontsel));
  increment = MAX (size / 5, 1);
  gucharmap_mini_font_selection_set_font_size (GUCHARMAP_MINI_FONT_SELECTION (priv->fontsel), size + increment);
}

static void
font_smaller (GtkAction       *action, 
              GucharmapWindow *guw)
{
  GucharmapWindowPrivate *priv = GUCHARMAP_WINDOW_GET_PRIVATE (guw);
  gint size, increment;

  size = gucharmap_mini_font_selection_get_font_size (GUCHARMAP_MINI_FONT_SELECTION (priv->fontsel));
  increment = MAX (size / 5, 1);
  gucharmap_mini_font_selection_set_font_size (GUCHARMAP_MINI_FONT_SELECTION (priv->fontsel), size - increment);
}

static void
font_default (GtkAction       *action, 
              GucharmapWindow *guw)
{
  GucharmapWindowPrivate *priv = GUCHARMAP_WINDOW_GET_PRIVATE (guw);
  gucharmap_mini_font_selection_reset_font_size (GUCHARMAP_MINI_FONT_SELECTION (priv->fontsel));
}

static void
snap_cols_pow2 (GtkAction 	 *action, 
                GucharmapWindow  *guw)
{
  gucharmap_table_set_snap_pow2 (guw->charmap->chartable,
  				 gtk_toggle_action_get_active (GTK_TOGGLE_ACTION (action)));
}

#ifdef HAVE_GNOME
static void
help_contents (GtkAction *action,
	       gpointer  data)
{
  GError *error = NULL;

  gnome_help_display ("gucharmap.xml", NULL, &error);

  if (error != NULL)
    {
      g_warning ("%s", error->message);
      g_error_free (error);
    }
}
#endif

static void
help_about (GtkAction       *action, 
            GucharmapWindow *guw)
{
  GucharmapWindowPrivate *priv = GUCHARMAP_WINDOW_GET_PRIVATE (guw);

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

  const gchar *license[] = {
    N_("Gucharmap is free software; you can redistribute it and/or modify "
       "it under the terms of the GNU General Public License as published by "
       "the Free Software Foundation; either version 2 of the License, or "
       "(at your option) any later version."),
    N_("Permission is hereby granted, free of charge, to any person obtaining "
       "a copy of the Unicode data files to deal in them without restriction, "
       "including without limitation the rights to use, copy, modify, merge, "
       "publish, distribute, and/or sell copies."),
    N_("Gucharmap and the Unicode data files are distributed in the hope that "
       "they will be useful, but WITHOUT ANY WARRANTY; without even the implied "
       "warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See "
       "the GNU General Public License and Unicode Copyright for more details."),
    N_("You should have received a copy of the GNU General Public License "
       "along with Gucharmap; if not, write to the Free Software Foundation, Inc., "
       "59 Temple Place, Suite 330, Boston, MA  02111-1307  USA"),
    N_("Also you should have received a copy of the Unicode Copyright along "
       "with Gucharmap; you can always find it at Unicode's website: "
       "http://www.unicode.org/copyright.html")
  };
  gchar *license_trans;
  license_trans = g_strconcat (_(license[0]), "\n\n", _(license[1]), "\n\n",
			       _(license[2]), "\n\n", _(license[3]), "\n\n",
			       _(license[4]), "\n\n", NULL);

  gtk_show_about_dialog (GTK_WINDOW (guw),
  			 "authors", authors,
			 "comments", _("GNOME Character Map\n"
				       "based on the Unicode Character Database"),
			 "copyright", "Copyright © 2004 Noah Levitt <nlevitt@columbia.edu>\n"
				      "Copyright © 1991-2005 Unicode, Inc.",
			 "documenters", documenters,
			 "license", license_trans,
			 "name", _("Gucharmap"),
			 "logo-icon-name", "gucharmap",
			 "translator-credits", _("translator-credits"),
			 "version", VERSION,
			 "website", "http://live.gnome.org/Gucharmap",
			 "wrap-license", TRUE,
			 NULL);

  g_free (license_trans);
}

static void
prev_character (GtkAction       *action,
                GucharmapWindow *guw)
{
  gint index = guw->charmap->chartable->active_cell;
  gint start = index;
  gunichar wc;

  do
    {
      index--;

      if (index < 0)
        index = gucharmap_codepoint_list_get_last_index (guw->charmap->chartable->codepoint_list);

      wc = gucharmap_codepoint_list_get_char (guw->charmap->chartable->codepoint_list, index);
    }
  while ((!gucharmap_unichar_isdefined (wc) || !gucharmap_unichar_validate (wc)) && index != start);

  gucharmap_table_set_active_character (guw->charmap->chartable, wc);
}

static void
next_character (GtkAction       *action,
                GucharmapWindow *guw)
{
  gint index = guw->charmap->chartable->active_cell;
  gint start = index;
  gunichar wc;

  do
    {
      index++;

      if (index > gucharmap_codepoint_list_get_last_index (guw->charmap->chartable->codepoint_list))
        index = 0;

      wc = gucharmap_codepoint_list_get_char (guw->charmap->chartable->codepoint_list, index);
    }
  while ((!gucharmap_unichar_isdefined (wc) || !gucharmap_unichar_validate (wc)) && index != start);

  gucharmap_table_set_active_character (guw->charmap->chartable, wc);
}

static void
next_chapter (GtkAction       *action,
              GucharmapWindow *guw)
{
  GucharmapChapters *chapters = gucharmap_charmap_get_chapters (guw->charmap);
  gucharmap_chapters_next (chapters);
}

static void
prev_chapter (GtkAction       *action,
              GucharmapWindow *guw)
{
  GucharmapChapters *chapters = gucharmap_charmap_get_chapters (guw->charmap);
  gucharmap_chapters_previous (chapters);
}

static void
chapters_set_labels (const gchar     *labelnext,
		     const gchar     *labelprev,
		     GucharmapWindow *guw)
{
  GtkAction *action;
  GucharmapWindowPrivate *priv = GUCHARMAP_WINDOW_GET_PRIVATE (guw);

  action = gtk_action_group_get_action (priv->action_group, "NextChapter");
  g_object_set ( G_OBJECT (action), "label", labelnext, NULL);
  action = gtk_action_group_get_action (priv->action_group, "PreviousChapter");
  g_object_set ( G_OBJECT (action), "label", labelprev, NULL);
}

enum {
  VIEW_BY_SCRIPT,
  VIEW_BY_BLOCK
};

static void
view_by (GtkAction        *action,
	 GtkRadioAction   *radioaction,
         GucharmapWindow  *guw)
{
  switch (gtk_radio_action_get_current_value (radioaction))
    {
      case VIEW_BY_SCRIPT:
      	gucharmap_charmap_set_chapters (guw->charmap, GUCHARMAP_CHAPTERS (gucharmap_script_chapters_new ()));
	chapters_set_labels (_("Next Script"), _("Previous Script"), guw);
	break;
      
      case VIEW_BY_BLOCK:
        gucharmap_charmap_set_chapters (guw->charmap, GUCHARMAP_CHAPTERS (gucharmap_block_chapters_new ()));
	chapters_set_labels (_("Next Block"), _("Previous Block"), guw);
	break;
      
      default:
        g_assert_not_reached ();
    }
}

/* create the menu entries */
/* tooltips are NULL because they are never actually shown in the program */

static const GtkActionEntry menu_entries[] =
{
  { "File", NULL, N_("_File"), NULL, NULL, NULL },
  { "View", NULL, N_("_View"), NULL, NULL, NULL },
  { "Search", NULL, N_("_Search"), NULL, NULL, NULL },
  { "Go", NULL, N_("_Go"), NULL, NULL, NULL },
  { "Help", NULL, N_("_Help"), NULL, NULL, NULL },

  { "Quit", GTK_STOCK_QUIT, N_("_Quit"), "<control>Q",
    NULL, G_CALLBACK (gtk_main_quit) },

  { "ZoomIn", NULL, N_("Zoom _In"), "<control>plus",
    NULL, G_CALLBACK (font_bigger) },
  { "ZoomOut", NULL, N_("Zoom _Out"), "<control>minus",
    NULL, G_CALLBACK (font_smaller) },
  { "NormalSize", NULL, N_("_Normal Size"), "<control>equal",
    NULL, G_CALLBACK (font_default) },

  { "Find", GTK_STOCK_FIND, N_("_Find..."), "<control>F",
    NULL, G_CALLBACK (search_find) },
  { "FindNext", GTK_STOCK_FIND, N_("Find _Next"), "<control>G",
    NULL, G_CALLBACK (search_find_next) },
  { "FindPrevious", GTK_STOCK_FIND, N_("Find _Previous"), "<shift><control>G",
    NULL, G_CALLBACK (search_find_prev) },

  { "NextCharacter", NULL, N_("_Next Character"), "<control>N",
    NULL, G_CALLBACK (next_character) },
  { "PreviousCharacter", NULL, N_("_Previous Character"), "<control>P",
    NULL, G_CALLBACK (prev_character) },
  { "NextChapter", NULL, N_("Next Script"), "<control>Page_Down",
    NULL, G_CALLBACK (next_chapter) },
  { "PreviousChapter", NULL, N_("Previous Script"), "<control>Page_Up",
    NULL, G_CALLBACK (prev_chapter) },

#ifdef HAVE_GNOME
  { "HelpContents", GTK_STOCK_HELP, N_("_Contents"), "F1",
    NULL, G_CALLBACK (help_contents) },
#endif
  { "About", GTK_STOCK_ABOUT, N_("_About"), NULL,
    NULL, G_CALLBACK (help_about) }
};

static const GtkRadioActionEntry radio_menu_entries [] =
{
  { "ByScript", NULL, N_("By _Script"), NULL,
    NULL, VIEW_BY_SCRIPT },
  { "ByUnicodeBlock", NULL, N_("By _Unicode Block"), NULL,
    NULL, VIEW_BY_BLOCK }
};

static const char ui_info [] =
"  <menubar name=\"MenuBar\">"
"    <menu name=\"FileMenu\" action=\"File\">"
"	 <menuitem name=\"FileQuitMenu\" action=\"Quit\" />"
"    </menu>"
"    <menu name=\"ViewMenu\" action=\"View\">"
"	 <menuitem name=\"ViewByScriptMenu\" action=\"ByScript\" />"
"	 <menuitem name=\"ViewByUnicodeBlockMenu\" action=\"ByUnicodeBlock\" />"
"	 <separator />"
"	 <menuitem name=\"ViewSnapColumnsMenu\" action=\"SnapColumns\" />"
"	 <separator />"
"	 <menuitem name=\"ViewZoomInMenu\" action=\"ZoomIn\" />"
"	 <menuitem name=\"ViewZoomOutMenu\" action=\"ZoomOut\" />"
"	 <menuitem name=\"ViewNormalSizeMenu\" action=\"NormalSize\" />"
"    </menu>"
"    <menu name=\"SearchMenu\" action=\"Search\">"
"	 <menuitem name=\"SearchFindMenu\" action=\"Find\" />"
"	 <menuitem name=\"SearchFindNextMenu\" action=\"FindNext\" />"
"	 <menuitem name=\"SearchFindPreviousMenu\" action=\"FindPrevious\" />"
"    </menu>"
"    <menu name=\"GoMenu\" action=\"Go\">"
"	 <menuitem name=\"GoNextCharacterMenu\" action=\"NextCharacter\" />"
"	 <menuitem name=\"GoPreviousCharacterMenu\" action=\"PreviousCharacter\" />"
"	 <separator />"
"	 <menuitem name=\"GoNextChapterMenu\" action=\"NextChapter\" />"
"	 <menuitem name=\"GoPreviousChapterMenu\" action=\"PreviousChapter\" />"
"    </menu>"
"    <menu name=\"HelpMenu\" action=\"Help\">"
#ifdef HAVE_GNOME
"	 <menuitem name=\"HelpContentsMenu\" action=\"HelpContents\" />"
#endif
"	 <menuitem name=\"HelpAboutMenu\" action=\"About\" />"
"    </menu>"
"  </menubar>";

static GtkWidget *
make_menu (GucharmapWindow *guw)
{
  GucharmapWindowPrivate *priv = GUCHARMAP_WINDOW_GET_PRIVATE (guw);
  GtkWidget *menubar;
  GtkAction *action;
  guint forward_keysym, back_keysym;
  GtkToggleAction *toggle_menu;

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

  toggle_menu = gtk_toggle_action_new ( "SnapColumns",
					_("Snap Columns to Power of Two"),
					NULL,
					NULL);
  g_signal_connect (G_OBJECT(toggle_menu), "toggled", G_CALLBACK (snap_cols_pow2), guw);
  /* make the menu bar */

  priv->uimanager = gtk_ui_manager_new();

  gtk_window_add_accel_group ( GTK_WINDOW (guw),
  			       gtk_ui_manager_get_accel_group (priv->uimanager) );
  
  priv->action_group = gtk_action_group_new ("gucharmap_actions");
  gtk_action_group_set_translation_domain (priv->action_group, GETTEXT_PACKAGE);

  gtk_action_group_add_actions (priv->action_group,
  				menu_entries,
				G_N_ELEMENTS (menu_entries),
				guw);
  gtk_action_group_add_radio_actions (priv->action_group,
  				      radio_menu_entries,
				      G_N_ELEMENTS (radio_menu_entries),
				      VIEW_BY_SCRIPT,
				      G_CALLBACK (view_by),
				      guw);

  gtk_action_group_add_action (priv->action_group,
  			       GTK_ACTION (toggle_menu));
  switch (priv->chapters_mode)
    {
      case CHAPTERS_SCRIPT:
	break;
      
      case CHAPTERS_BLOCK:
        chapters_set_labels (_("Next Block"), _("Previous Block"), guw);
	break;
      
      default:
        g_assert_not_reached ();
    }

  gtk_ui_manager_insert_action_group (priv->uimanager,
    					priv->action_group,
					0);
  
  if ( !gtk_ui_manager_add_ui_from_string (priv->uimanager,
                                           ui_info,
                                           -1,
                                           NULL) )
    {
      g_message ("building menus failed.");
  }
  
  menubar = gtk_ui_manager_get_widget (priv->uimanager, "/MenuBar");

  gtk_widget_show_all (menubar);

  if (! priv->file_menu_visible)
    {
      action = gtk_action_group_get_action (priv->action_group, "File");
      gtk_action_set_visible (action, FALSE);
      action = gtk_action_group_get_action (priv->action_group, "Quit");
      gtk_action_set_sensitive (action, FALSE);
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
  gtk_editable_delete_selection (GTK_EDITABLE (priv->text_to_copy_entry));
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

  gtk_window_set_icon_name (GTK_WINDOW (guw), "gucharmap");

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

  _gucharmap_intl_ensure_initialized ();
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
  GtkAction *action;

  priv->file_menu_visible = visible;

  if (priv->file_menu_visible)
    {
      action = gtk_action_group_get_action (priv->action_group, "File");
      gtk_action_set_visible (action, TRUE);
      action = gtk_action_group_get_action (priv->action_group, "Quit");
      gtk_action_set_sensitive (action, TRUE);
    }
  else
    {
      action = gtk_action_group_get_action (priv->action_group, "File");
      gtk_action_set_visible (action, FALSE);
      action = gtk_action_group_get_action (priv->action_group, "Quit");
      gtk_action_set_sensitive (action, FALSE);
    }
}

GucharmapMiniFontSelection *
gucharmap_window_get_mini_font_selection (GucharmapWindow *guw)
{
  GucharmapWindowPrivate *priv = GUCHARMAP_WINDOW_GET_PRIVATE (guw);
  return GUCHARMAP_MINI_FONT_SELECTION (priv->fontsel);
}
