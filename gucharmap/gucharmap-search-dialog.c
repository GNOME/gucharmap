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
#include <gtk/gtk.h>
#include <stdlib.h>
#include <string.h>
#include "gucharmap-search-dialog.h"
#include "gucharmap-window.h"
#include "gucharmap-unicode-info.h"
#include "gucharmap-marshal.h"
#include "gucharmap-intl.h"

#define GUCHARMAP_SEARCH_DIALOG_GET_PRIVATE(o) (G_TYPE_INSTANCE_GET_PRIVATE ((o), gucharmap_search_dialog_get_type (), GucharmapSearchDialogPrivate))

enum
{
  SEARCH_START,
  SEARCH_FINISH,
  NUM_SIGNALS
};

static guint gucharmap_search_dialog_signals[NUM_SIGNALS] = { 0, 0 };

enum
{
  GUCHARMAP_RESPONSE_PREVIOUS,
  GUCHARMAP_RESPONSE_NEXT
};

typedef struct _GucharmapSearchDialogPrivate GucharmapSearchDialogPrivate;
typedef struct _GucharmapSearchState GucharmapSearchState;

struct _GucharmapSearchState
{
  GucharmapCodepointList *list;
  gchar                  *search_string;
  const gchar            *no_leading_space;  /* points into search_string */
  gint                    start_index;
  gint                    curr_index;
  GucharmapDirection      increment;
  gboolean                whole_word;
  gint                    found_index;       /* index of the found character */
  /* true if there are known to be no matches, or there is known to be
   * exactly one match and it has been found */
  gboolean                dont_search;
  gpointer                saved_data;        /* holds some data to pass back to the caller */
  gint                    list_num_chars;    /* last_index + 1 */
};

struct _GucharmapSearchDialogPrivate
{
  GucharmapWindow       *guw;
  GtkWidget             *entry;
  GucharmapSearchState  *search_state;
  GtkWidget             *prev_button;
  GtkWidget             *next_button;
};

static const gchar *
utf8_strcasestr (const gchar *haystack, 
                 const gchar *needle)
{
  gint needle_len = strlen (needle);
  gint haystack_len = strlen (haystack);
  const gchar *p, *q, *r;

  for (p = haystack;  p + needle_len <= haystack + haystack_len;  p = g_utf8_next_char (p))
    {
      for (q = needle, r = p;  *q && *r;  q = g_utf8_next_char (q), r = g_utf8_next_char (r))
        {
          gunichar lc0 = g_unichar_tolower (g_utf8_get_char (r));
          gunichar lc1 = g_unichar_tolower (g_utf8_get_char (q));
          if (lc0 != lc1)
            goto next;
        }
      return p;

      next:
        ;
    }

  return NULL;
}

static gboolean
matches (gunichar     wc,
         const gchar *search_string)
{
  const gchar *haystack, *haystack_nfd;
  gboolean matches;
  gchar *needle_nfd;


  needle_nfd = g_utf8_normalize (search_string, -1, G_NORMALIZE_NFD);

  haystack = gucharmap_get_unicode_name (wc);
  if (haystack)
    {
      /* character names are ascii, so are nfd */
      haystack_nfd = haystack;
      /* haystack_nfd = g_utf8_normalize (haystack, -1, G_NORMALIZE_NFD); */
      matches = utf8_strcasestr (haystack_nfd, needle_nfd) != NULL;
      /* g_free (haystack_nfd); */
      if (matches)
        goto yes;
    }

  /* XXX: other strings */

  g_free (needle_nfd);
  return FALSE;

yes: 
  g_free (needle_nfd);
  return TRUE;
}

/* string should have no leading spaces */
static gint
check_for_explicit_codepoint (const GucharmapCodepointList *list,
                              const gchar                  *string)
{
  const gchar *nptr;
  gchar *endptr;

  /* check for explicit decimal codepoint */
  nptr = string;
  if (g_ascii_strncasecmp (string, "&#", 2) == 0)
    nptr = string + 2;
  else if (*string == '#')
    nptr = string + 1;

  if (nptr != string)
    {
      gunichar wc = strtoul (nptr, &endptr, 10);
      if (endptr != nptr)
        {
          gint index = gucharmap_codepoint_list_get_index ((GucharmapCodepointList *) list, wc);
          if (index != -1)
            return index;
        }
    }

  /* check for explicit hex code point */
  nptr = string;
  if (g_ascii_strncasecmp (string, "&#x", 3) == 0)
    nptr = string + 3;
  else if (g_ascii_strncasecmp (string, "U+", 2) == 0 || g_ascii_strncasecmp (string, "0x", 2) == 0)
    nptr = string + 2;

  if (nptr != string)
    {
      gunichar wc = strtoul (nptr, &endptr, 16);
      if (endptr != nptr)
        {
          gint index = gucharmap_codepoint_list_get_index ((GucharmapCodepointList *) list, wc);
          if (index != -1)
            return index;
        }
    }

  return -1;
}

static gboolean
quick_checks (GucharmapSearchState *search_state)
{
  gint index;

  if (search_state->dont_search)
    return TRUE;

  /* caller should check for empty string */
  if (search_state->search_string[0] == '\0')
    {
      search_state->dont_search = TRUE;
      return TRUE;
    }

  /* check for explicit codepoint */
  index = check_for_explicit_codepoint (search_state->list, search_state->no_leading_space);
  if (index != -1)
    {
      search_state->found_index = index;
      search_state->dont_search = TRUE;
      return TRUE;
    }

  /* if there is only one character, return it as the found character */
  if (g_utf8_strlen (search_state->search_string, -1) == 1)
    {
      index = gucharmap_codepoint_list_get_index ((GucharmapCodepointList *) search_state->list, 
                                                  g_utf8_get_char (search_state->search_string));
      if (index != -1)
        {
          search_state->found_index = index;
          search_state->dont_search = TRUE;
          return TRUE;
        }
    }

  return FALSE;
}

static gboolean
idle_search (GucharmapSearchDialog *search_dialog)
{
  GucharmapSearchDialogPrivate *priv = GUCHARMAP_SEARCH_DIALOG_GET_PRIVATE (search_dialog);
  GTimer *timer = g_timer_new ();
  gunichar wc;
  gint index;

  if (quick_checks (priv->search_state))
    return FALSE;

  /* XXX: search with leading spaces? */

  /* search without leading spaces */
  do
    {
      priv->search_state->curr_index = (priv->search_state->curr_index + priv->search_state->increment + priv->search_state->list_num_chars) % priv->search_state->list_num_chars;
      wc = gucharmap_codepoint_list_get_char (priv->search_state->list, priv->search_state->curr_index);

      if (!gucharmap_unichar_validate (wc) || !gucharmap_unichar_isdefined (wc))
        continue;

      /* no leading spaces */
      if (matches (wc, priv->search_state->no_leading_space))
        {
          if (priv->search_state->found_index == priv->search_state->curr_index)
            priv->search_state->dont_search = TRUE;  /* this is the only match */

          priv->search_state->found_index = priv->search_state->curr_index;

          return FALSE;
        }

      if (g_timer_elapsed (timer, NULL) > 0.050)
        return TRUE;
    }
  while (priv->search_state->curr_index != priv->search_state->start_index);

  /* jump to the first nonspace character unless it’s plain ascii */
  if (*priv->search_state->no_leading_space < 0x20 || *priv->search_state->no_leading_space > 0x7e)
    {
      index = gucharmap_codepoint_list_get_index (priv->search_state->list, g_utf8_get_char (priv->search_state->no_leading_space));
      if (index != -1)
        {
          priv->search_state->found_index = index;
          priv->search_state->dont_search = TRUE;
        }
    }

  priv->search_state->dont_search = TRUE;
  return FALSE;
}

/**
 * gucharmap_search_state_get_found_char:
 * @search_state: 
 * Return value: 
 **/
gunichar
gucharmap_search_state_get_found_char (GucharmapSearchState *search_state)
{
  if (search_state->found_index > 0)
    return gucharmap_codepoint_list_get_char (search_state->list, search_state->found_index);
  else
    return (gunichar)(-1);
}

/**
 * gucharmap_search_state_free:
 * @search_state: 
 **/
void
gucharmap_search_state_free (GucharmapSearchState *search_state)
{
  g_free (search_state->search_string);
  g_free (search_state);
}

/**
 * gucharmap_search_state_new:
 * @list: a #GucharmapCodepointList to be searched
 * @search_string: the text to search for
 * @start_index: the starting point within @list
 * @direction: forward or backward
 * @whole_word: %TRUE if it should match whole words
 *
 * Initializes a #GucharmapSearchState to search for the next character in
 * the codepoint list that matches @search_string. Assumes input is valid.
 *
 * Return value: the new #GucharmapSearchState.
 **/
GucharmapSearchState * 
gucharmap_search_state_new (const GucharmapCodepointList *list, 
                            const gchar                  *search_string, 
                            gint                          start_index, 
                            GucharmapDirection            direction, 
                            gboolean                      whole_word)
{
  GucharmapSearchState *search_state;

  g_assert (direction == GUCHARMAP_DIRECTION_BACKWARD || direction == GUCHARMAP_DIRECTION_FORWARD);

  search_state = g_new (GucharmapSearchState, 1);

  search_state->list = (GucharmapCodepointList *) list;
  search_state->list_num_chars = gucharmap_codepoint_list_get_last_index (search_state->list) + 1;

  search_state->search_string = g_strdup (search_string);

  search_state->increment = direction;
  search_state->whole_word = whole_word;

  search_state->found_index = -1;
  search_state->dont_search = FALSE;

  search_state->start_index = start_index;
  search_state->curr_index = start_index;

  /* set pointer to first non-space character in the search string */
  for (search_state->no_leading_space = search_string;
       g_unichar_isspace (g_utf8_get_char (search_state->no_leading_space));
       search_state->no_leading_space = g_utf8_next_char (search_state->no_leading_space));

  return search_state;
}

static void
information_dialog (GucharmapSearchDialog *search_dialog,
                    const gchar           *message)
{
  GtkWidget *dialog, *hbox, *icon, *label;

  /* follow hig guidelines */
  dialog = gtk_dialog_new ();
  gtk_window_set_title (GTK_WINDOW (dialog), _("Information"));
  gtk_container_set_border_width (GTK_CONTAINER (dialog), 6);
  gtk_dialog_set_has_separator (GTK_DIALOG (dialog), FALSE);
  gtk_window_set_destroy_with_parent (GTK_WINDOW (dialog), TRUE);
  gtk_box_set_spacing (GTK_BOX (GTK_DIALOG (dialog)->vbox), 12);
  gtk_window_set_icon (GTK_WINDOW (dialog), gtk_window_get_icon (GTK_WINDOW (search_dialog)));
  gtk_window_set_resizable (GTK_WINDOW (dialog), FALSE);
  gtk_window_set_modal (GTK_WINDOW (dialog), TRUE);
  gtk_window_set_transient_for (GTK_WINDOW (dialog), GTK_WINDOW (search_dialog));

  gtk_dialog_add_button (GTK_DIALOG (dialog), GTK_STOCK_OK, GTK_RESPONSE_ACCEPT);

  hbox = gtk_hbox_new (FALSE, 12);
  gtk_widget_show (hbox);
  gtk_container_set_border_width (GTK_CONTAINER (hbox), 6);
  gtk_box_pack_start (GTK_BOX (GTK_DIALOG (dialog)->vbox), hbox, FALSE, FALSE, 0);

  icon = gtk_image_new_from_stock (GTK_STOCK_DIALOG_INFO, GTK_ICON_SIZE_DIALOG);
  gtk_widget_show (icon);
  gtk_box_pack_start (GTK_BOX (hbox), icon, FALSE, FALSE, 0);

  label = gtk_label_new (message);
  gtk_widget_show (label);
  gtk_label_set_line_wrap (GTK_LABEL (label), TRUE);
  gtk_box_pack_start (GTK_BOX (hbox), label, FALSE, FALSE, 0);

  g_signal_connect (dialog, "response", G_CALLBACK (gtk_widget_destroy), NULL);

  gtk_widget_show (dialog);
}

static void
search_completed (GucharmapSearchDialog *search_dialog)
{
  GucharmapSearchDialogPrivate *priv = GUCHARMAP_SEARCH_DIALOG_GET_PRIVATE (search_dialog);
  gunichar found_char = gucharmap_search_state_get_found_char (priv->search_state);

  g_signal_emit (search_dialog, gucharmap_search_dialog_signals[SEARCH_FINISH], 0, found_char);

  if (found_char == (gunichar)(-1))
    information_dialog (search_dialog, _("Not found."));

  gtk_widget_set_sensitive (priv->prev_button, TRUE);
  gtk_widget_set_sensitive (priv->next_button, TRUE);

  gdk_window_set_cursor (GTK_WIDGET (search_dialog)->window, NULL);
}

void
gucharmap_search_dialog_start_search (GucharmapSearchDialog *search_dialog,
                                      GucharmapDirection     direction)
{
  GucharmapSearchDialogPrivate *priv = GUCHARMAP_SEARCH_DIALOG_GET_PRIVATE (search_dialog);
  GucharmapCodepointList *list;
  gunichar start_char;
  gint start_index;

  GdkCursor *cursor = gdk_cursor_new (GDK_WATCH);
  gdk_window_set_cursor (GTK_WIDGET (search_dialog)->window, cursor);
  gdk_cursor_unref (cursor);

  if (priv->search_state)
    gucharmap_search_state_free (priv->search_state);

  list = (GucharmapCodepointList *) gucharmap_chapters_get_book_codepoint_list (gucharmap_charmap_get_chapters (priv->guw->charmap));
  start_char = gucharmap_table_get_active_character (priv->guw->charmap->chartable);
  start_index = gucharmap_codepoint_list_get_index (list, start_char);
  priv->search_state = gucharmap_search_state_new (list, gtk_entry_get_text (GTK_ENTRY (priv->entry)), start_index, direction, FALSE);

  gtk_widget_set_sensitive (priv->prev_button, FALSE);
  gtk_widget_set_sensitive (priv->next_button, FALSE);

  g_signal_emit (search_dialog, gucharmap_search_dialog_signals[SEARCH_START], 0);

  g_idle_add_full (G_PRIORITY_DEFAULT_IDLE, (GSourceFunc) idle_search, search_dialog, (GDestroyNotify) search_completed);
}

static void
search_find_response (GtkDialog *dialog,
                      gint       response)
{
  GucharmapSearchDialog *search_dialog = GUCHARMAP_SEARCH_DIALOG (dialog);

  switch (response)
    {
      case GUCHARMAP_RESPONSE_PREVIOUS:
        gucharmap_search_dialog_start_search (search_dialog, GUCHARMAP_DIRECTION_BACKWARD);
        break;

      case GUCHARMAP_RESPONSE_NEXT:
        gucharmap_search_dialog_start_search (search_dialog, GUCHARMAP_DIRECTION_FORWARD);
        break;

      default:
        gtk_widget_hide (GTK_WIDGET (search_dialog));
        break;
    }
}

static void
entry_changed (GtkEntry              *entry,
               GucharmapSearchDialog *search_dialog)
{
  GucharmapSearchDialogPrivate *priv = GUCHARMAP_SEARCH_DIALOG_GET_PRIVATE (search_dialog);
  const gchar *text = gtk_entry_get_text (entry);

  if (text[0] == '\0')
    {
      gtk_widget_set_sensitive (priv->prev_button, FALSE);
      gtk_widget_set_sensitive (priv->next_button, FALSE);
    }
  else
    {
      gtk_widget_set_sensitive (priv->prev_button, TRUE);
      gtk_widget_set_sensitive (priv->next_button, TRUE);
    }
}

static void
gucharmap_search_dialog_init (GucharmapSearchDialog *search_dialog)
{
  GucharmapSearchDialogPrivate *priv = GUCHARMAP_SEARCH_DIALOG_GET_PRIVATE (search_dialog);
  GtkWidget *hbox, *label;

  /* follow hig guidelines */
  gtk_window_set_title (GTK_WINDOW (search_dialog), _("Find"));
  gtk_container_set_border_width (GTK_CONTAINER (search_dialog), 6);
  gtk_dialog_set_has_separator (GTK_DIALOG (search_dialog), FALSE);
  gtk_window_set_destroy_with_parent (GTK_WINDOW (search_dialog), TRUE);
  gtk_box_set_spacing (GTK_BOX (GTK_DIALOG (search_dialog)->vbox), 12);
  gtk_window_set_resizable (GTK_WINDOW (search_dialog), FALSE);

  g_signal_connect (search_dialog, "delete-event", G_CALLBACK (gtk_widget_hide), NULL);

  /* add buttons */
  gtk_dialog_add_button (GTK_DIALOG (search_dialog), GTK_STOCK_CANCEL, GTK_RESPONSE_CANCEL);
  priv->prev_button = gtk_dialog_add_button (GTK_DIALOG (search_dialog), _("_Previous"), GUCHARMAP_RESPONSE_PREVIOUS);
  priv->next_button = gtk_dialog_add_button (GTK_DIALOG (search_dialog), _("_Next"), GUCHARMAP_RESPONSE_NEXT);
  gtk_dialog_set_default_response (GTK_DIALOG (search_dialog), GUCHARMAP_RESPONSE_NEXT);

  hbox = gtk_hbox_new (FALSE, 12);
  gtk_widget_show (hbox);
  gtk_container_set_border_width (GTK_CONTAINER (hbox), 6);
  gtk_box_pack_start (GTK_BOX (GTK_DIALOG (search_dialog)->vbox), hbox, FALSE, FALSE, 0);

  label = gtk_label_new_with_mnemonic (_("_Search:"));
  gtk_widget_show (label);
  gtk_box_pack_start (GTK_BOX (hbox), label, FALSE, FALSE, 0);

  priv->entry = gtk_entry_new ();
  gtk_widget_show (priv->entry);
  gtk_entry_set_activates_default (GTK_ENTRY (priv->entry), TRUE);
  gtk_box_pack_start (GTK_BOX (hbox), priv->entry, TRUE, TRUE, 0);
  g_signal_connect (priv->entry, "changed", G_CALLBACK (entry_changed), search_dialog);

  /* since the entry is empty */
  gtk_widget_set_sensitive (priv->prev_button, FALSE);
  gtk_widget_set_sensitive (priv->next_button, FALSE);

  priv->search_state = NULL;
  priv->guw = NULL;

  g_signal_connect (GTK_DIALOG (search_dialog), "response", G_CALLBACK (search_find_response), NULL);
}

static void 
gucharmap_search_dialog_finalize (GObject *object)
{
  GucharmapSearchDialog *search_dialog = GUCHARMAP_SEARCH_DIALOG (object);
  GucharmapSearchDialogPrivate *priv = GUCHARMAP_SEARCH_DIALOG_GET_PRIVATE (search_dialog);

  if (priv->search_state)
    gucharmap_search_state_free (priv->search_state);
}

static void
gucharmap_search_dialog_class_init (GucharmapSearchDialogClass *clazz)
{
  g_type_class_add_private (clazz, sizeof (GucharmapSearchDialogPrivate));

  G_OBJECT_CLASS (clazz)->finalize = gucharmap_search_dialog_finalize;

  clazz->search_start = NULL;
  clazz->search_finish = NULL;

  gucharmap_search_dialog_signals[SEARCH_START] =
      g_signal_new ("search-start", gucharmap_search_dialog_get_type (), G_SIGNAL_RUN_FIRST, 
                    G_STRUCT_OFFSET (GucharmapSearchDialogClass, search_start), NULL, NULL, 
                    gucharmap_marshal_VOID__VOID, G_TYPE_NONE, 0);
  gucharmap_search_dialog_signals[SEARCH_FINISH] =
      g_signal_new ("search-finish", gucharmap_search_dialog_get_type (), G_SIGNAL_RUN_FIRST, 
                    G_STRUCT_OFFSET (GucharmapSearchDialogClass, search_finish), NULL, NULL, 
                    gucharmap_marshal_VOID__UINT, G_TYPE_NONE, 1, G_TYPE_UINT);
}

GType
gucharmap_search_dialog_get_type ()
{
  static GType t = 0;

  if (t == 0)
    {
      static const GTypeInfo type_info =
        {
          sizeof (GucharmapSearchDialogClass),
          NULL,
          NULL,
          (GClassInitFunc) gucharmap_search_dialog_class_init,
          NULL,
          NULL,
          sizeof (GucharmapSearchDialog),
          0,
          (GInstanceInitFunc) gucharmap_search_dialog_init,
          NULL
        };

      t = g_type_register_static (gtk_dialog_get_type (), "GucharmapSearchDialog", &type_info, 0);
    }

  return t;
}

GtkWidget * 
gucharmap_search_dialog_new (GucharmapWindow *guw)
{
  GucharmapSearchDialog *search_dialog = g_object_new (gucharmap_search_dialog_get_type (), NULL);
  GucharmapSearchDialogPrivate *priv = GUCHARMAP_SEARCH_DIALOG_GET_PRIVATE (search_dialog);

  priv->guw = guw;

  gtk_window_set_transient_for (GTK_WINDOW (search_dialog), GTK_WINDOW (guw));

  if (guw)
    gtk_window_set_icon (GTK_WINDOW (search_dialog), gtk_window_get_icon (GTK_WINDOW (guw)));

  return GTK_WIDGET (search_dialog);

}
