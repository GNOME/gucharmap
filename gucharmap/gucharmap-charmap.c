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
#include <gdk/gdkkeysyms.h>
#include <stdlib.h>
#include <string.h>

#include "gucharmap-charmap.h"
#include "gucharmap-unicode-info.h"
#include "gucharmap-script-chapters.h"
#include "gucharmap-intl.h"
#include "gucharmap-marshal.h"
#include "gucharmap-settings.h"
#include "chartable_accessible.h"

gboolean _gucharmap_unicode_has_nameslist_entry (gunichar uc);

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

static void 
charmap_finalize (GObject *object)
{
  GucharmapCharmap *charmap = GUCHARMAP_CHARMAP (object);

  gdk_cursor_unref (charmap->hand_cursor);
  gdk_cursor_unref (charmap->regular_cursor);
}

static void
gucharmap_charmap_class_init (GucharmapCharmapClass *clazz)
{
  clazz->status_message = NULL;

  gucharmap_charmap_signals[STATUS_MESSAGE] =
      g_signal_new ("status-message", gucharmap_charmap_get_type (), 
                    G_SIGNAL_RUN_FIRST,
                    G_STRUCT_OFFSET (GucharmapCharmapClass, status_message),
                    NULL, NULL, _gucharmap_marshal_VOID__STRING, G_TYPE_NONE, 
                    1, G_TYPE_STRING);

  gucharmap_charmap_signals[LINK_CLICKED] =
      g_signal_new ("link-clicked", gucharmap_charmap_get_type (), 
                    G_SIGNAL_RUN_FIRST,
                    G_STRUCT_OFFSET (GucharmapCharmapClass, link_clicked),
                    NULL, NULL, _gucharmap_marshal_VOID__UINT_UINT, G_TYPE_NONE, 
                    2, G_TYPE_UINT, G_TYPE_UINT);

  G_OBJECT_CLASS (clazz)->finalize = charmap_finalize;

  _gucharmap_intl_ensure_initialized ();
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
  g_object_set_data (G_OBJECT (tag), "link-character", GUINT_TO_POINTER (uc + 1));

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

/* - "XXXX-YYYY" used in annotation of U+003D 
 * - Annotation of U+03C0 uses ".XXXX" as a fractional number,
 *   so don't add "." to the list.
 * Add here if you know more. */
#define is_blank(c) (  ((c) == ' ')	\
		    || ((c) == '-') )

#define is_blank_or_hex_or(a,b) (  !((a) < len)	\
				|| is_blank(str[a])	\
				|| (is_hex_digit(str[a]) && (b)) )

/* returns a pointer to the start of (?=[^ -])[0-9A-F]{4,5,6}[^0-9A-F],
 * or null if not found */
static const gchar *
find_codepoint (const gchar *str)
{
  guint i, len;

  /* what we are searching for is ascii; in this case, we don't have to
   * worry about multibyte characters at all */
  len = strlen (str);
  for (i = 0;  i + 3 < len;  i++)
    {
      if ( ( !(i > 0) || is_blank(str[i-1]) )
	  && is_hex_digit (str[i+0]) && is_hex_digit (str[i+1]) 
	  && is_hex_digit (str[i+2]) && is_hex_digit (str[i+3])
	  && is_blank_or_hex_or(i+4,    is_blank_or_hex_or(i+5,
		      (i+6 < len) || !is_hex_digit (str[i+6]))) )
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
  guint i;

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
  guchar utf8[7];
  gint n, i;
  const gchar **csarr;
  gunichar *ucs;
  gunichar2 *utf16;

  buffer = gtk_text_view_get_buffer (GTK_TEXT_VIEW (charmap->details));
  gtk_text_buffer_set_text (buffer, "", -1);

  gtk_text_buffer_get_start_iter (buffer, &iter);
  gtk_text_buffer_place_cursor (buffer, &iter);
  gtk_text_buffer_insert (buffer, &iter, "\n", -1);

  n = gucharmap_unichar_to_printable_utf8 (uc, buf);
  if (n == 0)
    gtk_text_buffer_insert (
            buffer, &iter, _("[not a printable character]"), -1);
  else
    gtk_text_buffer_insert_with_tags_by_name (buffer, &iter, buf, n, 
                                              "gimongous", NULL);

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

  /* representations */
  if (g_unichar_break_type(uc) != G_UNICODE_BREAK_SURROGATE)
    {
      insert_heading (charmap, buffer, &iter, _("Various Useful Representations"));

      n = g_unichar_to_utf8 (uc, (gchar *)utf8);
      utf16 = g_ucs4_to_utf16 (&uc, 1, NULL, NULL, NULL);

      /* UTF-8 */
      gstemp = g_string_new (NULL);
      for (i = 0;  i < n;  i++)
	g_string_append_printf (gstemp, "0x%2.2X ", utf8[i]);
      g_string_erase (gstemp, gstemp->len - 1, -1);
      insert_vanilla_detail (charmap, buffer, &iter, _("UTF-8:"), gstemp->str);
      g_string_free (gstemp, TRUE);

      /* UTF-16 */
      gstemp = g_string_new (NULL);
      g_string_append_printf (gstemp, "0x%4.4X", utf16[0]);
      if (utf16[0] != '\0' && utf16[1] != '\0')
	g_string_append_printf (gstemp, " 0x%4.4X", utf16[1]);
      insert_vanilla_detail (charmap, buffer, &iter, _("UTF-16:"), gstemp->str);
      g_string_free (gstemp, TRUE);

      /* an empty line */
      gtk_text_buffer_insert (buffer, &iter, "\n", -1);

      /* C octal \012\234 */
      gstemp = g_string_new (NULL);
      for (i = 0;  i < n;  i++)
	g_string_append_printf (gstemp, "\\%3.3o", utf8[i]);
      insert_vanilla_detail (charmap, buffer, &iter, 
			     _("C octal escaped UTF-8:"), gstemp->str);
      g_string_free (gstemp, TRUE);

      /* XML decimal entity */
      if ((0x0001 <= uc && uc <= 0xD7FF) ||
	  (0xE000 <= uc && uc <= 0xFFFD) ||
	  (0x10000 <= uc && uc <= 0x10FFFF))
	{
	  temp = g_strdup_printf ("&#%d;", uc);
	  insert_vanilla_detail (charmap, buffer, &iter, 
				 _("XML decimal entity:"), temp);
	  g_free (temp);
	}

      g_free(utf16);
    }

  /* annotations */
  if (_gucharmap_unicode_has_nameslist_entry (uc))
    {
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
    }

#if ENABLE_UNIHAN

  /* this isn't so bad efficiency-wise */
  if (gucharmap_get_unicode_kDefinition (uc)
      || gucharmap_get_unicode_kCantonese (uc)
      || gucharmap_get_unicode_kMandarin (uc)
      || gucharmap_get_unicode_kJapaneseOn (uc)
      || gucharmap_get_unicode_kJapaneseKun (uc)
      || gucharmap_get_unicode_kTang (uc)
      || gucharmap_get_unicode_kKorean (uc))
    {
      insert_heading (charmap, buffer, &iter, _("CJK Ideograph Information"));

      csp = gucharmap_get_unicode_kDefinition (uc);
      if (csp)
        insert_vanilla_detail (charmap, buffer, &iter,
                               _("Definition in English:"), csp);
    
      csp = gucharmap_get_unicode_kMandarin (uc);
      if (csp)
        insert_vanilla_detail (charmap, buffer, &iter,
                               _("Mandarin Pronunciation:"), csp);
    
      csp = gucharmap_get_unicode_kCantonese (uc);
      if (csp)
        insert_vanilla_detail (charmap, buffer, &iter,
                               _("Cantonese Pronunciation:"), csp);
    
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
    }
#endif /* #if ENABLE_UNIHAN */
}

static gboolean
gucharmap_active_char_save (gpointer last_char)
{
  gucharmap_settings_set_last_char (GPOINTER_TO_UINT (last_char));
  return FALSE;
}

static void
active_char_set (GtkWidget        *widget, 
                 gunichar          wc, 
                 GucharmapCharmap *charmap)
{
  GString *gs;
  const gchar *temp;
  const gchar **temps;
  gint i;

  set_details (charmap, wc);

  g_idle_add (gucharmap_active_char_save, GUINT_TO_POINTER(wc));

  gs = g_string_new (NULL);
  g_string_append_printf (gs, "U+%4.4X %s", wc, 
                          gucharmap_get_unicode_name (wc));

#if ENABLE_UNIHAN
  temp = gucharmap_get_unicode_kDefinition (wc);
  if (temp)
    g_string_append_printf (gs, "   %s", temp);
#endif

  temps = gucharmap_get_nameslist_equals (wc);
  if (temps)
    {
      g_string_append_printf (gs, "   = %s", temps[0]);
      for (i = 1;  temps[i];  i++)
        g_string_append_printf (gs, "; %s", temps[i]);
      g_free (temps);
    }

  temps = gucharmap_get_nameslist_stars (wc);
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
                              "left-margin", PANGO_PIXELS (5 * default_font_size),
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
       * "link-character" */
      uc = GPOINTER_TO_UINT (g_object_get_data (G_OBJECT (tag), "link-character")) - 1;

      if (uc != (gunichar)(-1)) 
        {
          g_signal_emit (charmap, gucharmap_charmap_signals[LINK_CLICKED], 0, 
                         gucharmap_table_get_active_character (charmap->chartable), 
                         uc);
          gucharmap_charmap_go_to_character (charmap, uc);
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
       * "link-character" */
      uc = GPOINTER_TO_UINT (g_object_get_data (G_OBJECT (tag), "link-character")) - 1;

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
  GtkWidget *scrolled_window = NULL;

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
make_chartable_pane (GucharmapCharmap       *charmap)
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

static void
chapter_changed (GucharmapChapters *chapters,
                 GucharmapCharmap  *charmap)
{
  gucharmap_table_set_codepoint_list (charmap->chartable, gucharmap_chapters_get_codepoint_list (chapters));
}

static void
gucharmap_charmap_init (GucharmapCharmap *charmap)
{
}

GtkWidget *
gucharmap_charmap_new (GucharmapChapters *chapters)
{
  GucharmapCharmap *charmap = g_object_new (gucharmap_charmap_get_type (), NULL);
  GtkWidget *pane2;

  charmap->hand_cursor = gdk_cursor_new (GDK_HAND2);
  charmap->regular_cursor = gdk_cursor_new (GDK_XTERM);
  charmap->hovering_over_link = FALSE;
  gtk_widget_show (GTK_WIDGET (chapters));

  g_signal_connect (G_OBJECT (chapters), "changed", G_CALLBACK (chapter_changed), charmap);

  pane2 = make_chartable_pane (charmap);
  gtk_paned_pack1 (GTK_PANED (charmap), GTK_WIDGET (chapters), FALSE, TRUE);
  gtk_paned_pack2 (GTK_PANED (charmap), pane2, TRUE, TRUE);

  gucharmap_charmap_go_to_character (charmap, gucharmap_settings_get_last_char ());

  return GTK_WIDGET (charmap);
}

G_DEFINE_TYPE (GucharmapCharmap, gucharmap_charmap, GTK_TYPE_HPANED)

void 
gucharmap_charmap_set_font (GucharmapCharmap *charmap, 
                            const gchar *font_name)
{
  gucharmap_table_set_font (charmap->chartable, font_name);
}

void
gucharmap_charmap_go_to_character (GucharmapCharmap *charmap, 
                                   gunichar          wc)
{
  GucharmapChapters *chapters = gucharmap_charmap_get_chapters (charmap);
  gboolean status;

  status = gucharmap_chapters_go_to_character (chapters, wc);
  if (!status)
    g_warning ("gucharmap_chapters_go_to_character failed (%04X)\n", wc);

  if (wc <= UNICHAR_MAX)
    gucharmap_table_set_active_character (charmap->chartable, wc);
}

GucharmapTable *
gucharmap_charmap_get_chartable (GucharmapCharmap *charmap)
{
  return charmap->chartable;
}

void
gucharmap_charmap_set_chapters (GucharmapCharmap  *charmap,
                                GucharmapChapters *chapters)
{
  gtk_container_remove (GTK_CONTAINER (charmap), GTK_PANED (charmap)->child1);
  gtk_paned_pack1 (GTK_PANED (charmap), GTK_WIDGET (chapters), FALSE, TRUE);
  g_signal_connect (G_OBJECT (chapters), "changed", G_CALLBACK (chapter_changed), charmap);
  gtk_widget_show (GTK_WIDGET (chapters));

  /* Keep the same character selected as before */
  gucharmap_charmap_go_to_character (charmap, gucharmap_table_get_active_character (charmap->chartable));
}

GucharmapChapters *
gucharmap_charmap_get_chapters (GucharmapCharmap  *charmap)
{
  return GUCHARMAP_CHAPTERS (GTK_PANED (charmap)->child1);
}
