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

#ifdef HAVE_CONFIG_H
# include <config.h>
#endif

#include <gtk/gtk.h>
#include <gdk/gdkkeysyms.h>
#include <stdlib.h>
#include <string.h>

#include <gucharmap/gucharmap.h>
#include "gucharmap_intl.h"
#include "gucharmap_marshal.h"
#include "chartable_accessible.h"


/* 0x100, a standard increment for paging unicode */
#define PAGE_SIZE 256

/* only the label is visible in the block selector */
enum 
{
  BLOCK_SELECTOR_LABEL = 0,
  BLOCK_SELECTOR_UC_START,
  BLOCK_SELECTOR_UNICODE_BLOCK,
  BLOCK_SELECTOR_NUM_COLUMNS
};

enum 
{
  STATUS_MESSAGE = 0,
  LINK_CLICKED,
  NUM_SIGNALS
};

static guint gucharmap_charmap_signals[NUM_SIGNALS] = { 0, 0 };


static void
status_message (GucharmapCharmap *charmap, const gchar *message)
{
  g_signal_emit (charmap, gucharmap_charmap_signals[STATUS_MESSAGE], 
                 0, message);
}


/* XXX: linear search (but N is small) */
static GtkTreePath *
find_block_index_tree_path (GucharmapCharmap *charmap, gunichar uc)
{
  gint i;

  for (i = 0;  i < charmap->block_index_size;  i++)
    if (charmap->block_index[i].start > uc)
      break;

  return charmap->block_index[i-1].tree_path;
}


/* selects the active block in the block selector tree view based on the
 * active character */
static void
set_active_block (GucharmapCharmap *charmap, gunichar uc)
{
  GtkTreePath *parent = NULL;
  GtkTreePath *tree_path;
  
  tree_path = find_block_index_tree_path (charmap, uc);

  /* block our "changed" handler */
  g_signal_handler_block (G_OBJECT (charmap->block_selection), 
                          charmap->block_selection_changed_handler_id);

  gtk_tree_view_set_cursor (GTK_TREE_VIEW (charmap->block_selector_view),
                            tree_path, NULL, FALSE);

  g_signal_handler_unblock (G_OBJECT (charmap->block_selection),
                            charmap->block_selection_changed_handler_id);

  if (parent != NULL)
    gtk_tree_path_free (parent);
}


static void
block_selection_changed (GtkTreeSelection *selection, 
                         gpointer user_data)
{
  GtkTreeModel *model;
  GtkTreeIter iter;
  GucharmapCharmap *charmap;
  gunichar uc_start;

  charmap = GUCHARMAP_CHARMAP (user_data);

  if (gtk_tree_selection_get_selected (selection, &model, &iter))
    {
      gtk_tree_model_get (model, &iter, BLOCK_SELECTOR_UC_START, 
                          &uc_start, -1);

      gucharmap_table_set_active_character (charmap->chartable, uc_start);
    }
}


/* makes the list of unicode blocks and code points */
static GtkWidget *
make_unicode_block_selector (GucharmapCharmap *charmap)
{
  GtkWidget *scrolled_window;
  GtkTreeIter iter;
  GtkCellRenderer *cell;
  GtkTreeViewColumn *column;
  gint i, bi;

  scrolled_window = gtk_scrolled_window_new (NULL, NULL);

  gtk_scrolled_window_set_policy (GTK_SCROLLED_WINDOW (scrolled_window),
                                  GTK_POLICY_NEVER, GTK_POLICY_AUTOMATIC);
  gtk_scrolled_window_set_shadow_type (GTK_SCROLLED_WINDOW (scrolled_window), 
                                       GTK_SHADOW_ETCHED_IN);

  charmap->block_selector_model = gtk_tree_store_new (
          BLOCK_SELECTOR_NUM_COLUMNS, G_TYPE_STRING, 
          G_TYPE_UINT, G_TYPE_POINTER);

  charmap->block_index_size = gucharmap_count_blocks (UNICHAR_MAX) + 1;
  charmap->block_index = g_malloc (charmap->block_index_size 
                                   * sizeof (GucharmapBlockIndex));
  bi = 0;

  for (i = 0;  gucharmap_unicode_blocks[i].start != (gunichar)(-1)
               && gucharmap_unicode_blocks[i].start <= UNICHAR_MAX;  i++)
    {
      gtk_tree_store_append (charmap->block_selector_model, &iter, NULL);
      gtk_tree_store_set (charmap->block_selector_model, &iter, 
                          BLOCK_SELECTOR_LABEL, 
                          _(gucharmap_unicode_blocks[i].name),
                          BLOCK_SELECTOR_UC_START, 
                          gucharmap_unicode_blocks[i].start,
                          BLOCK_SELECTOR_UNICODE_BLOCK, 
                          &(gucharmap_unicode_blocks[i]),
                          -1);
      charmap->block_index[bi].start = gucharmap_unicode_blocks[i].start;
      charmap->block_index[bi].tree_path = gtk_tree_model_get_path (
              GTK_TREE_MODEL (charmap->block_selector_model), &iter);
      bi++;
    }

  /* terminate value that is bigger than the biggest character */
  charmap->block_index[bi].start = UNICHAR_MAX + 1;
  charmap->block_index[bi].tree_path = NULL;

  /* we have the model, now make the view */
  charmap->block_selector_view = gtk_tree_view_new_with_model (
          GTK_TREE_MODEL (charmap->block_selector_model));
  charmap->block_selection = gtk_tree_view_get_selection (
          GTK_TREE_VIEW (charmap->block_selector_view));
  gtk_tree_view_set_headers_visible (
          GTK_TREE_VIEW (charmap->block_selector_view), FALSE);

  cell = gtk_cell_renderer_text_new ();
  column = gtk_tree_view_column_new_with_attributes (NULL, cell, 
                                                     "text", 0, NULL);

  gtk_tree_view_append_column (GTK_TREE_VIEW (charmap->block_selector_view),
                               GTK_TREE_VIEW_COLUMN (column));

  gtk_tree_selection_set_mode (charmap->block_selection, GTK_SELECTION_BROWSE);
  charmap->block_selection_changed_handler_id = g_signal_connect (
          G_OBJECT (charmap->block_selection), "changed", 
          G_CALLBACK (block_selection_changed), charmap);

  gtk_container_add (GTK_CONTAINER (scrolled_window), 
                     charmap->block_selector_view);

  gtk_widget_show_all (scrolled_window);

  return scrolled_window;
}


void
gucharmap_charmap_class_init (GucharmapCharmapClass *clazz)
{
  clazz->status_message = NULL;

  gucharmap_charmap_signals[STATUS_MESSAGE] =
      g_signal_new ("status-message", gucharmap_charmap_get_type (), 
                    G_SIGNAL_RUN_FIRST,
                    G_STRUCT_OFFSET (GucharmapCharmapClass, status_message),
                    NULL, NULL, gucharmap_marshal_VOID__STRING, G_TYPE_NONE, 
		    1, G_TYPE_STRING);

  gucharmap_charmap_signals[LINK_CLICKED] =
      g_signal_new ("link-clicked", gucharmap_charmap_get_type (), 
                    G_SIGNAL_RUN_FIRST,
                    G_STRUCT_OFFSET (GucharmapCharmapClass, link_clicked),
                    NULL, NULL, gucharmap_marshal_VOID__UINT_UINT, G_TYPE_NONE, 
		    2, G_TYPE_UINT, G_TYPE_UINT);
}


static void
insert_vanilla_detail (GucharmapCharmap *charmap, 
                       GtkTextBuffer *buffer,
                       GtkTextIter *iter,
                       const gchar *name,
                       const gchar *value)
{
  gtk_text_buffer_insert (buffer, iter, name, -1);
  gtk_text_buffer_insert (buffer, iter, " ", -1);
  gtk_text_buffer_insert_with_tags_by_name (buffer, iter, value, -1,
                                            "detail-value", NULL);
  gtk_text_buffer_insert (buffer, iter, "\n", -1);
}


/* makes a nice string and makes it a link to the character */
static void
insert_codepoint (GucharmapCharmap *charmap,
                  GtkTextBuffer *buffer,
                  GtkTextIter *iter,
                  gunichar uc)
{
  gchar *str;
  GtkTextTag *tag;

  str = g_strdup_printf ("U+%4.4X %s", uc,
                         gucharmap_get_unicode_name (uc));

  tag = gtk_text_buffer_create_tag (buffer, NULL, 
                                    "foreground", "blue", 
                                    "underline", PANGO_UNDERLINE_SINGLE, 
                                    NULL);
  /* add one so that zero is the "nothing" value, since U+0000 is a character */
  g_object_set_data (G_OBJECT (tag), "link_character", 
                     (gpointer) (uc + 1));

  gtk_text_buffer_insert_with_tags (buffer, iter, str, -1, tag, NULL);

  g_free (str);
}


static void
insert_chocolate_detail_codepoints (GucharmapCharmap *charmap,
                                    GtkTextBuffer *buffer,
                                    GtkTextIter *iter,
                                    const gchar *name,
                                    const gunichar *ucs)
{
  gint i;

  gtk_text_buffer_insert (buffer, iter, name, -1);
  gtk_text_buffer_insert (buffer, iter, "\n", -1);

  for (i = 0;  ucs[i] != (gunichar)(-1);  i++)
    {
      gtk_text_buffer_insert (buffer, iter, " • ", -1);
      insert_codepoint (charmap, buffer, iter, ucs[i]);
      gtk_text_buffer_insert (buffer, iter, "\n", -1);
    }

  gtk_text_buffer_insert (buffer, iter, "\n", -1);
}

#define is_hex_digit(c) (((c) >= '0' && (c) <= '9') \
                         || ((c) >= 'A' && (c) <= 'F'))

/* returns a pointer to the start of [0-9A-F]{4}, or null if not found */
static const gchar *
find_codepoint (const gchar *str)
{
  gint i;

  /* what we are searching for is ascii; in this case, we don't have to
   * worry about multibyte characters at all */
  for (i = 0;  i + 3 < strlen (str);  i++)
    {
      if (is_hex_digit (str[i]) && is_hex_digit (str[i+1]) 
          && is_hex_digit (str[i+2]) && is_hex_digit (str[i+3]))
        return str + i;
    }

  return NULL;
}


static void
insert_string_link_codepoints (GucharmapCharmap *charmap,
                               GtkTextBuffer *buffer,
                               GtkTextIter *iter,
                               const gchar *str)
{
  const gchar *p1, *p2;

  p1 = str;
  for (;;)
    {
      p2 = find_codepoint (p1);
      if (p2 != NULL)
        {
          gunichar uc;
          gtk_text_buffer_insert (buffer, iter, p1, p2 - p1);
          uc = strtoul (p2, (gchar **) &p1, 16);
          insert_codepoint (charmap, buffer, iter, uc);
        }
      else
        {
          gtk_text_buffer_insert (buffer, iter, p1, -1);
          break;
        }
    }
}

/* values is a null-terminated array of strings */
static void
insert_chocolate_detail (GucharmapCharmap *charmap,
                         GtkTextBuffer *buffer,
                         GtkTextIter *iter,
                         const gchar *name,
                         const gchar **values,
                         gboolean expand_codepoints)
{
  gint i;

  gtk_text_buffer_insert (buffer, iter, name, -1);
  gtk_text_buffer_insert (buffer, iter, "\n", -1);

  for (i = 0;  values[i];  i++)
    {
      gtk_text_buffer_insert (buffer, iter, " • ", -1);
      if (expand_codepoints)
        insert_string_link_codepoints (charmap, buffer, iter, values[i]);
      else
        gtk_text_buffer_insert (buffer, iter, values[i], -1);
      gtk_text_buffer_insert (buffer, iter, "\n", -1);
    }

  gtk_text_buffer_insert (buffer, iter, "\n", -1);
}


static void
insert_heading (GucharmapCharmap *charmap, 
                GtkTextBuffer *buffer,
                GtkTextIter *iter,
                const gchar *heading)
{
  gtk_text_buffer_insert (buffer, iter, "\n", -1);
  gtk_text_buffer_insert_with_tags_by_name (buffer, iter, heading, -1, 
                                            "bold", NULL);
  gtk_text_buffer_insert (buffer, iter, "\n\n", -1);
}


static void
conditionally_insert_canonical_decomposition (GucharmapCharmap *charmap, 
                                              GtkTextBuffer *buffer,
                                              GtkTextIter *iter,
                                              gunichar uc)
{
  gunichar *decomposition;
  gsize result_len;
  gint i;

  decomposition = gucharmap_unicode_canonical_decomposition (uc, &result_len);

  if (result_len == 1)
    {
      g_free (decomposition);
      return;
    }

  gtk_text_buffer_insert (buffer, iter, _("Canonical decomposition:"), -1);
  gtk_text_buffer_insert (buffer, iter, " ", -1);

  insert_codepoint (charmap, buffer, iter, decomposition[0]);
  for (i = 1;  i < result_len;  i++)
    {
      gtk_text_buffer_insert (buffer, iter, " + ", -1);
      insert_codepoint (charmap, buffer, iter, decomposition[i]);
    }

  gtk_text_buffer_insert (buffer, iter, "\n", -1);

  g_free (decomposition);
}

static void
set_details (GucharmapCharmap *charmap,
             gunichar uc)
{
  GtkTextBuffer *buffer;
  GtkTextIter iter;
  GString *gstemp;
  gchar *temp;
  const gchar *csp;
  gchar buf[12];
  guchar ubuf[7];
  gint n, i;
  const gchar **csarr;
  gunichar *ucs;

  buffer = gtk_text_view_get_buffer (GTK_TEXT_VIEW (charmap->details));
  gtk_text_buffer_set_text (buffer, "", -1);

  gtk_text_buffer_get_start_iter (buffer, &iter);
  gtk_text_buffer_place_cursor (buffer, &iter);
  gtk_text_buffer_insert (buffer, &iter, "\n\n", -1);

  n = gucharmap_unichar_to_printable_utf8 (uc, buf);
  if (n == 0)
    gtk_text_buffer_insert_with_tags_by_name (
            buffer, &iter, _("[not a printable character]"), -1, NULL);
  else
    {
      /* give it some space to breathe */
      /* gtk_text_buffer_insert (buffer, &iter, "     ", -1); */
      gtk_text_buffer_insert_with_tags_by_name (buffer, &iter, "   ", -1, 
                                                "gimongous", NULL);
      gtk_text_buffer_insert_with_tags_by_name (buffer, &iter, buf, n, 
                                                "gimongous", NULL);
    }

  gtk_text_buffer_insert (buffer, &iter, "\n\n", -1);
                                             
  /* character name */
  temp = g_strdup_printf ("U+%4.4X %s\n", 
                          uc, gucharmap_get_unicode_name (uc));
  gtk_text_buffer_insert_with_tags_by_name (buffer, &iter, temp, -1,
                                            "big", "bold", NULL);
  g_free (temp);

  insert_heading (charmap, buffer, &iter, _("General Character Properties"));

  /* character category */
  insert_vanilla_detail (charmap, buffer, &iter, _("Unicode category:"),
                         gucharmap_get_unicode_category_name (uc));

  /* canonical decomposition */
  conditionally_insert_canonical_decomposition (charmap, buffer, &iter, uc);

  insert_heading (charmap, buffer, &iter, _("Various Useful Representations"));

  n = g_unichar_to_utf8 (uc, ubuf);

  /* UTF-8 */
  gstemp = g_string_new (NULL);
  for (i = 0;  i < n;  i++)
    g_string_append_printf (gstemp, "0x%2.2X ", ubuf[i]);
  g_string_erase (gstemp, gstemp->len - 1, -1);
  insert_vanilla_detail (charmap, buffer, &iter, _("UTF-8:"), gstemp->str);
  g_string_free (gstemp, TRUE);

  /* octal \012\234 UTF-8 */
  gstemp = g_string_new (NULL);
  for (i = 0;  i < n;  i++)
    g_string_append_printf (gstemp, "\\%3.3o", ubuf[i]);
  insert_vanilla_detail (charmap, buffer, &iter, 
                         _("Octal escaped UTF-8:"), gstemp->str);
  g_string_free (gstemp, TRUE);

  /* entity reference */
  temp = g_strdup_printf ("&#%d;", uc);
  insert_vanilla_detail (charmap, buffer, &iter, 
                         _("Decimal entity reference:"), temp);
  g_free (temp);

  insert_heading (charmap, buffer, &iter, 
                  _("Annotations and Cross References"));

  /* nameslist equals (alias names) */
  csarr = gucharmap_get_nameslist_equals (uc);
  if (csarr != NULL)
    {
      insert_chocolate_detail (charmap, buffer, &iter,
                               _("Alias names:"), csarr, FALSE);
      g_free (csarr);
    }

  /* nameslist stars (notes) */
  csarr = gucharmap_get_nameslist_stars (uc);
  if (csarr != NULL)
    {
      insert_chocolate_detail (charmap, buffer, &iter,
                               _("Notes:"), csarr, TRUE);
      g_free (csarr);
    }

  /* nameslist exes (see also) */
  ucs = gucharmap_get_nameslist_exes (uc);
  if (ucs != NULL)
    {
      insert_chocolate_detail_codepoints (charmap, buffer, &iter,
                                          _("See also:"), ucs);
      g_free (ucs);
    }

  /* nameslist pounds (approximate equivalents) */
  csarr = gucharmap_get_nameslist_pounds (uc);
  if (csarr != NULL)
    {
      insert_chocolate_detail (charmap, buffer, &iter,
                               _("Approximate equivalents:"), csarr, TRUE);
      g_free (csarr);
    }

  /* nameslist colons (equivalents) */
  csarr = gucharmap_get_nameslist_colons (uc);
  if (csarr != NULL)
    {
      insert_chocolate_detail (charmap, buffer, &iter,
                               _("Equivalents:"), csarr, TRUE);
      g_free (csarr);
    }

#if ENABLE_UNIHAN

  insert_heading (charmap, buffer, &iter, _("CJK Ideograph Information"));

  csp = gucharmap_get_unicode_kDefinition (uc);
  if (csp)
    insert_vanilla_detail (charmap, buffer, &iter,
                           _("Definition in English:"), csp);

  csp = gucharmap_get_unicode_kMandarin (uc);
  if (csp)
    insert_vanilla_detail (charmap, buffer, &iter,
                           _("Mandarin Pronunciation:"), csp);

  csp = gucharmap_get_unicode_kJapaneseOn (uc);
  if (csp)
    insert_vanilla_detail (charmap, buffer, &iter,
                           _("Japanese On Pronunciation:"), csp);

  csp = gucharmap_get_unicode_kJapaneseKun (uc);
  if (csp)
    insert_vanilla_detail (charmap, buffer, &iter,
                           _("Japanese Kun Pronunciation:"), csp);

  csp = gucharmap_get_unicode_kTang (uc);
  if (csp)
    insert_vanilla_detail (charmap, buffer, &iter,
                           _("Tang Pronunciation:"), csp);

  csp = gucharmap_get_unicode_kKorean (uc);
  if (csp)
    insert_vanilla_detail (charmap, buffer, &iter,
                           _("Korean Pronunciation:"), csp);
#endif /* #if ENABLE_UNIHAN */
}


static void
active_char_set (GtkWidget *widget, 
                 gunichar uc, 
                 GucharmapCharmap *charmap)
{
  GString *gs;
  const gchar *temp;
  const gchar **temps;
  gint i;

  set_active_block (charmap, uc);
  set_details (charmap, uc);

  gs = g_string_new (NULL);
  g_string_append_printf (gs, "U+%4.4X %s", uc, 
                          gucharmap_get_unicode_name (uc));

#if ENABLE_UNIHAN
  temp = gucharmap_get_unicode_kDefinition (uc);
  if (temp)
    g_string_append_printf (gs, "   %s", temp);
#endif

  temps = gucharmap_get_nameslist_equals (uc);
  if (temps)
    {
      g_string_append_printf (gs, "   = %s", temps[0]);
      for (i = 1;  temps[i];  i++)
        g_string_append_printf (gs, "; %s", temps[i]);
      g_free (temps);
    }

  temps = gucharmap_get_nameslist_stars (uc);
  if (temps)
    {
      g_string_append_printf (gs, "   • %s", temps[0]);
      for (i = 1;  temps[i];  i++)
        g_string_append_printf (gs, "; %s", temps[i]);
      g_free (temps);
    }

  status_message (charmap, gs->str);
  g_string_free (gs, TRUE);
}


/* this creates all the named text tags we'll be using in set_details */
static void
create_tags (GucharmapCharmap *charmap)
{
  GtkTextBuffer *buffer;
  gint default_font_size;

  buffer = gtk_text_view_get_buffer (GTK_TEXT_VIEW (charmap->details));

  default_font_size = pango_font_description_get_size (
	  GTK_WIDGET (charmap)->style->font_desc);

  gtk_text_buffer_create_tag (buffer, "gimongous", 
	                      "size", 8 * default_font_size, 
			      NULL);
  gtk_text_buffer_create_tag (buffer, "bold",
	                      "weight", PANGO_WEIGHT_BOLD,
			      NULL);
  gtk_text_buffer_create_tag (buffer, "big",
                              "size", default_font_size * 5 / 4,
                              NULL);
  gtk_text_buffer_create_tag (buffer, "detail-value",
                              NULL);
}


static void
follow_if_link (GucharmapCharmap *charmap,
                GtkTextIter *iter)
{
  GSList *tags = NULL, *tagp = NULL;

  tags = gtk_text_iter_get_tags (iter);
  for (tagp = tags;  tagp != NULL;  tagp = tagp->next)
    {
      GtkTextTag *tag = tagp->data;
      gunichar uc;

      /* subtract 1 because U+0000 is a character; see above where we set
       * "link_character" */
      uc = (gunichar) g_object_get_data (G_OBJECT (tag), "link_character") - 1;

      if (uc != (gunichar)(-1)) 
        {
          g_signal_emit (charmap, gucharmap_charmap_signals[LINK_CLICKED], 
                          0, charmap->chartable->active_char, uc);
          gucharmap_table_set_active_character (charmap->chartable, uc);
          break;
        }
    }

  if (tags) 
    g_slist_free (tags);
}


static gboolean
details_key_press_event (GtkWidget *text_view,
                         GdkEventKey *event,
                         GucharmapCharmap *charmap)
{
  GtkTextIter iter;
  GtkTextBuffer *buffer;

  switch (event->keyval)
    {
      case GDK_Return: 
      case GDK_KP_Enter:
        buffer = gtk_text_view_get_buffer (GTK_TEXT_VIEW (text_view));
        gtk_text_buffer_get_iter_at_mark (buffer, &iter, 
                                          gtk_text_buffer_get_insert (buffer));
        follow_if_link (charmap, &iter);
        break;

      default:
        break;
    }

  return FALSE;
}


static gboolean
details_event_after (GtkWidget *text_view,
                     GdkEvent *ev,
                     GucharmapCharmap *charmap)
{
  GtkTextIter start, end, iter;
  GtkTextBuffer *buffer;
  GdkEventButton *event;
  gint x, y;

  if (ev->type != GDK_BUTTON_RELEASE)
    return FALSE;

  event = (GdkEventButton *) ev;

  if (event->button != 1)
    return FALSE;

  buffer = gtk_text_view_get_buffer (GTK_TEXT_VIEW (text_view));

  /* we shouldn't follow a link if the user has selected something */
  gtk_text_buffer_get_selection_bounds (buffer, &start, &end);
  if (gtk_text_iter_get_offset (&start) != gtk_text_iter_get_offset (&end))
    return FALSE;

  gtk_text_view_window_to_buffer_coords (GTK_TEXT_VIEW (text_view), 
                                         GTK_TEXT_WINDOW_WIDGET,
                                         event->x, event->y, &x, &y);

  gtk_text_view_get_iter_at_location (GTK_TEXT_VIEW (text_view), &iter, x, y);

  follow_if_link (charmap, &iter);

  return FALSE;
}


static void
set_cursor_if_appropriate (GucharmapCharmap *charmap,
                           gint x,
                           gint y)
{
  GSList *tags = NULL, *tagp = NULL;
  GtkTextBuffer *buffer;
  GtkTextIter iter;
  gboolean hovering_over_link = FALSE;

  buffer = gtk_text_view_get_buffer (GTK_TEXT_VIEW (charmap->details));

  gtk_text_view_get_iter_at_location (GTK_TEXT_VIEW (charmap->details), 
                                      &iter, x, y);

  tags = gtk_text_iter_get_tags (&iter);
  for (tagp = tags;  tagp != NULL;  tagp = tagp->next)
    {
      GtkTextTag *tag = tagp->data;
      gunichar uc;

      /* subtract 1 because U+0000 is a character; see above where we set
       * "link_character" */
      uc = (gunichar) g_object_get_data (G_OBJECT (tag), "link_character") - 1;

      if (uc != (gunichar)(-1)) 
        {
          hovering_over_link = TRUE;
          break;
        }
    }

  if (hovering_over_link != charmap->hovering_over_link)
    {
      charmap->hovering_over_link = hovering_over_link;

      if (hovering_over_link)
        gdk_window_set_cursor (gtk_text_view_get_window (GTK_TEXT_VIEW (charmap->details), GTK_TEXT_WINDOW_TEXT), charmap->hand_cursor);
      else
        gdk_window_set_cursor (gtk_text_view_get_window (GTK_TEXT_VIEW (charmap->details), GTK_TEXT_WINDOW_TEXT), charmap->regular_cursor);
    }

  if (tags) 
    g_slist_free (tags);
}


static gboolean
details_motion_notify_event (GtkWidget *text_view,
                             GdkEventMotion *event,
                             GucharmapCharmap *charmap)
{
  gint x, y;

  gtk_text_view_window_to_buffer_coords (GTK_TEXT_VIEW (text_view), 
                                         GTK_TEXT_WINDOW_WIDGET,
                                         event->x, event->y, &x, &y);

  set_cursor_if_appropriate (charmap, x, y);

  gdk_window_get_pointer (text_view->window, NULL, NULL, NULL);
  return FALSE;
}


static gboolean
details_visibility_notify_event (GtkWidget *text_view,
                                 GdkEventVisibility *event,
                                 GucharmapCharmap *charmap)
{
  gint wx, wy, bx, by;

  gdk_window_get_pointer (text_view->window, &wx, &wy, NULL);

  gtk_text_view_window_to_buffer_coords (GTK_TEXT_VIEW (text_view), 
                                         GTK_TEXT_WINDOW_WIDGET,
                                         wx, wy, &bx, &by);

  set_cursor_if_appropriate (charmap, bx, by);

  return FALSE;
}


static GtkWidget *
make_details_page (GucharmapCharmap *charmap)
{
  GtkWidget *scrolled_window;

  scrolled_window = gtk_scrolled_window_new (NULL, NULL);
  gtk_widget_show (scrolled_window);
  gtk_scrolled_window_set_policy (GTK_SCROLLED_WINDOW (scrolled_window),
                                  GTK_POLICY_AUTOMATIC, GTK_POLICY_AUTOMATIC);
  gtk_scrolled_window_set_shadow_type (GTK_SCROLLED_WINDOW (scrolled_window), 
                                       GTK_SHADOW_ETCHED_IN);

  charmap->details = gtk_text_view_new ();
  gtk_widget_show (charmap->details);
  gtk_text_view_set_editable (GTK_TEXT_VIEW (charmap->details), FALSE);
  gtk_text_view_set_wrap_mode (GTK_TEXT_VIEW (charmap->details), 
                               GTK_WRAP_WORD);

  g_signal_connect (G_OBJECT (charmap->details), "key-press-event",
                    G_CALLBACK (details_key_press_event), charmap);
  g_signal_connect (G_OBJECT (charmap->details), "event-after",
                    G_CALLBACK (details_event_after), charmap);
  g_signal_connect (G_OBJECT (charmap->details), "motion-notify-event", 
                    G_CALLBACK (details_motion_notify_event), charmap);
  g_signal_connect (G_OBJECT (charmap->details), "visibility-notify-event", 
                    G_CALLBACK (details_visibility_notify_event), charmap);

  create_tags (charmap);

  gtk_container_add (GTK_CONTAINER (scrolled_window), charmap->details);

  return scrolled_window;
}


static GtkWidget *
make_chartable_pane (GucharmapCharmap *charmap)
{
  GtkWidget *notebook;

  notebook = gtk_notebook_new ();
  gtk_widget_show (notebook);

  charmap->chartable = GUCHARMAP_TABLE (gucharmap_table_new ());
  gtk_widget_show (GTK_WIDGET (charmap->chartable));
  g_signal_connect (G_OBJECT (charmap->chartable), "set-active-char", 
                    G_CALLBACK (active_char_set), charmap);
  g_signal_connect_swapped (G_OBJECT (charmap->chartable), "status-message", 
                            G_CALLBACK (status_message), charmap);

  gtk_notebook_append_page (
          GTK_NOTEBOOK (notebook), GTK_WIDGET (charmap->chartable),
          gtk_label_new_with_mnemonic (_("Characte_r Table")));
  gtk_notebook_append_page (
          GTK_NOTEBOOK (notebook), make_details_page (charmap), 
          gtk_label_new_with_mnemonic (_("Character _Details")));

  return notebook;
}


void
gucharmap_charmap_init (GucharmapCharmap *charmap)
{
  AtkObject *accessib;
  GtkWidget *block_selector;

  charmap->hand_cursor = gdk_cursor_new (GDK_HAND2);
  charmap->regular_cursor = gdk_cursor_new (GDK_XTERM);
  charmap->hovering_over_link = FALSE;

  block_selector = make_unicode_block_selector (charmap);
  accessib = gtk_widget_get_accessible (block_selector);
  atk_object_set_name (accessib, _("List of Unicode Blocks"));

  gtk_paned_pack1 (GTK_PANED (charmap), block_selector, FALSE, TRUE);
  gtk_paned_pack2 (GTK_PANED (charmap), make_chartable_pane (charmap), 
                   TRUE, TRUE);

  set_active_block (charmap, charmap->chartable->active_char);
  set_details (charmap, charmap->chartable->active_char);
}


GtkWidget *
gucharmap_charmap_new (void)
{
  return GTK_WIDGET (g_object_new (gucharmap_charmap_get_type (), NULL));
}


GType
gucharmap_charmap_get_type (void)
{
  static GType gucharmap_charmap_type = 0;

  if (!gucharmap_charmap_type)
    {
      static const GTypeInfo gucharmap_charmap_info =
      {
        sizeof (GucharmapCharmapClass),
        NULL,           /* base_init */
        NULL,           /* base_finalize */
        (GClassInitFunc) gucharmap_charmap_class_init,
        NULL,           /* class_finalize */
        NULL,           /* class_data */
        sizeof (GucharmapCharmap),
        0,              /* n_preallocs */
        (GInstanceInitFunc) gucharmap_charmap_init,
      };

      gucharmap_charmap_type = g_type_register_static (GTK_TYPE_HPANED, 
                                                       "GucharmapCharmap", 
                                                       &gucharmap_charmap_info,
                                                       0);
    }

  return gucharmap_charmap_type;
}


void 
gucharmap_charmap_set_font (GucharmapCharmap *charmap, 
                            const gchar *font_name)
{
  gucharmap_table_set_font (charmap->chartable, font_name);
}


void
gucharmap_charmap_identify_clipboard (GucharmapCharmap *charmap, 
                                      GtkClipboard *clipboard)
{
  gucharmap_table_identify_clipboard (charmap->chartable, clipboard);
}


void
gucharmap_charmap_go_to_character (GucharmapCharmap *charmap, 
                                   gunichar uc)
{
  if (uc >= 0 && uc <= UNICHAR_MAX)
    gucharmap_table_set_active_character (charmap->chartable, uc);
}


/* direction is +1 (forward) or -1 (backward) */
GucharmapSearchResult
gucharmap_charmap_search (GucharmapCharmap *charmap, 
                          const gchar *search_text, 
                          gint direction)
{
  gunichar uc;
  GucharmapSearchResult result;

  g_assert (direction == -1 || direction == 1);

  if (search_text[0] == '\0')
    return GUCHARMAP_NOTHING_TO_SEARCH_FOR;
  
  uc = gucharmap_find_substring_match (charmap->chartable->active_char, 
                             search_text, direction);
  if (uc != (gunichar)(-1) && uc <= UNICHAR_MAX)
    {
      if ((direction == 1 && uc <= charmap->chartable->active_char)
          || (direction == -1 && uc >= charmap->chartable->active_char))
        result = GUCHARMAP_WRAPPED;
      else
        result = GUCHARMAP_FOUND;

      gucharmap_table_set_active_character (charmap->chartable, uc);
    }
  else
    result = GUCHARMAP_NOT_FOUND;

  return result;
}


void
gucharmap_charmap_zoom_enable (GucharmapCharmap *charmap)
{
  gucharmap_table_zoom_enable (charmap->chartable);
}


void
gucharmap_charmap_zoom_disable (GucharmapCharmap *charmap)
{
  gucharmap_table_zoom_disable (charmap->chartable);
}


GucharmapTable *
gucharmap_charmap_get_chartable (GucharmapCharmap *charmap)
{
  return charmap->chartable;
}


