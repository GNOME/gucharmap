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
 * 59 Temple Place, Suite 330, Boston, MA 02110-1301  USA
 */

#include "config.h"

#include <gtk/gtk.h>
#include <gdk/gdkkeysyms.h>
#include <stdlib.h>
#include <string.h>

#include "gucharmap-charmap.h"
#include "gucharmap-unicode-info.h"
#include "gucharmap-intl.h"
#include "gucharmap-marshal.h"
#include "gucharmap-settings.h"

struct _GucharmapCharmap
{
  GtkHPaned parent;

  GucharmapChaptersView *chapters_view;
  GucharmapChartable *chartable;
  GtkTextView *details_view;

  PangoFontDescription *font_desc;

  GdkCursor *hand_cursor;
  GdkCursor *regular_cursor;

  guint hovering_over_link   : 1;
  guint showing_details_page : 1;
  guint last_character_set   : 1;
};


struct _GucharmapCharmapClass
{
  GtkHPanedClass parent_class;

  void (* status_message) (GucharmapCharmap *charmap, const gchar *message);
  void (* link_clicked) (GucharmapCharmap *charmap, 
                         gunichar old_character,
                         gunichar new_character);
};

enum
{
  STATUS_MESSAGE,
  LINK_CLICKED,
  NUM_SIGNALS
};

enum {
  PROP_0,
  PROP_CHAPTERS_MODEL
};

static guint gucharmap_charmap_signals[NUM_SIGNALS];

gboolean _gucharmap_unicode_has_nameslist_entry (gunichar uc);

static void gucharmap_charmap_class_init (GucharmapCharmapClass *klass);
static void gucharmap_charmap_init       (GucharmapCharmap *charmap);

G_DEFINE_TYPE (GucharmapCharmap, gucharmap_charmap, GTK_TYPE_HPANED)

static void
gucharmap_charmap_finalize (GObject *object)
{
  GucharmapCharmap *charmap = GUCHARMAP_CHARMAP (object);

  gdk_cursor_unref (charmap->hand_cursor);
  gdk_cursor_unref (charmap->regular_cursor);

  if (charmap->font_desc)
    pango_font_description_free (charmap->font_desc);

  G_OBJECT_CLASS (gucharmap_charmap_parent_class)->finalize (object);
}

static void
gucharmap_charmap_set_property (GObject *object,
                                guint prop_id,
                                const GValue *value,
                                GParamSpec *pspec)
{
  GucharmapCharmap *charmap = GUCHARMAP_CHARMAP (object);

  switch (prop_id) {
    case PROP_CHAPTERS_MODEL:
      gucharmap_charmap_set_chapters_model (charmap, g_value_get_object (value));
      break;
    default:
      G_OBJECT_WARN_INVALID_PROPERTY_ID (object, prop_id, pspec);
      break;
  }
}

static void
gucharmap_charmap_class_init (GucharmapCharmapClass *clazz)
{
  GObjectClass *object_class = G_OBJECT_CLASS (clazz);

  _gucharmap_intl_ensure_initialized ();

  object_class->set_property = gucharmap_charmap_set_property;
  object_class->finalize = gucharmap_charmap_finalize;

  gucharmap_charmap_signals[STATUS_MESSAGE] =
      g_signal_new (I_("status-message"), gucharmap_charmap_get_type (),
                    G_SIGNAL_RUN_FIRST,
                    G_STRUCT_OFFSET (GucharmapCharmapClass, status_message),
                    NULL, NULL, g_cclosure_marshal_VOID__STRING, G_TYPE_NONE,
                    1, G_TYPE_STRING);

  gucharmap_charmap_signals[LINK_CLICKED] =
      g_signal_new (I_("link-clicked"), gucharmap_charmap_get_type (),
                    G_SIGNAL_RUN_FIRST,
                    G_STRUCT_OFFSET (GucharmapCharmapClass, link_clicked),
                    NULL, NULL, _gucharmap_marshal_VOID__UINT_UINT, G_TYPE_NONE, 
                    2, G_TYPE_UINT, G_TYPE_UINT);

  g_object_class_install_property (object_class,
                                   PROP_CHAPTERS_MODEL,
                                   g_param_spec_object ("chapters-model", NULL, NULL,
                                                        GUCHARMAP_TYPE_CHAPTERS_MODEL,
                                                        G_PARAM_WRITABLE | G_PARAM_CONSTRUCT | G_PARAM_STATIC_NAME | G_PARAM_STATIC_NICK | G_PARAM_STATIC_BLURB));
}

static void
gucharmap_charmap_set_font_desc_internal (GucharmapCharmap *charmap, 
                                          PangoFontDescription *font_desc)
{
  if (charmap->font_desc)
    pango_font_description_free (charmap->font_desc);

  charmap->font_desc = font_desc; /* adopted */

  gucharmap_chartable_set_font_desc (charmap->chartable, font_desc);
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

  buffer = gtk_text_view_get_buffer (charmap->details_view);
  gtk_text_buffer_set_text (buffer, "", 0);

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
chartable_status_message (GucharmapCharmap *charmap,
                          const gchar *message)
{
  g_signal_emit (charmap, gucharmap_charmap_signals[STATUS_MESSAGE], 
                 0, message);
}

static void
chartable_sync_active_char (GtkWidget *widget,
                            GParamSpec *pspec,
                            GucharmapCharmap *charmap)
{
  GString *gs;
  const gchar *temp;
  const gchar **temps;
  gint i;
  gunichar wc;

  wc = gucharmap_chartable_get_active_character (charmap->chartable);

  if (charmap->showing_details_page)
    set_details (charmap, wc);

  g_idle_add (gucharmap_active_char_save, GUINT_TO_POINTER(wc));

  gs = g_string_sized_new (256);
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

  chartable_status_message (charmap, gs->str);
  g_string_free (gs, TRUE);
}

/* this creates all the named text tags we'll be using in set_details */
static void
create_tags (GucharmapCharmap *charmap)
{
  GtkTextBuffer *buffer;
  gint default_font_size;

  buffer = gtk_text_view_get_buffer (charmap->details_view);

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
                         gucharmap_chartable_get_active_character (charmap->chartable), 
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

  buffer = gtk_text_view_get_buffer (charmap->details_view);

  gtk_text_view_get_iter_at_location (charmap->details_view,
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
        gdk_window_set_cursor (gtk_text_view_get_window (charmap->details_view, GTK_TEXT_WINDOW_TEXT), charmap->hand_cursor);
      else
        gdk_window_set_cursor (gtk_text_view_get_window (charmap->details_view, GTK_TEXT_WINDOW_TEXT), charmap->regular_cursor);
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

static void
notebook_switch_page (GtkNotebook *notebook,
                      GtkNotebookPage *page /* useless */,
                      guint page_num,
                      GucharmapCharmap *charmap)
{
  charmap->showing_details_page = (page_num == 1);

  if (charmap->showing_details_page)
    set_details (charmap, gucharmap_chartable_get_active_character (charmap->chartable));
  else
    {
      GtkTextBuffer *buffer;

      buffer = gtk_text_view_get_buffer (charmap->details_view);
      gtk_text_buffer_set_text (buffer, "", 0);
    }
}

static void
chapters_view_selection_changed_cb (GtkTreeSelection *selection,
                                    GucharmapCharmap *charmap)
{
  GucharmapCodepointList *codepoint_list;
  GtkTreeIter iter;

  if (!gtk_tree_selection_get_selected (selection, NULL, &iter))
    return;

  codepoint_list = gucharmap_chapters_view_get_codepoint_list (charmap->chapters_view);
  gucharmap_chartable_set_codepoint_list (charmap->chartable, codepoint_list);
  g_object_unref (codepoint_list);
}

static void
gucharmap_charmap_init (GucharmapCharmap *charmap)
{
  GtkWidget *scrolled_window, *view, *notebook, *chartable, *textview;
  GtkTreeSelection *selection;

  /* FIXME: move this to realize */
  charmap->hand_cursor = gdk_cursor_new (GDK_HAND2);
  charmap->regular_cursor = gdk_cursor_new (GDK_XTERM);
  charmap->hovering_over_link = FALSE;

  /* Left pane */
  scrolled_window = gtk_scrolled_window_new (NULL, NULL);
  gtk_scrolled_window_set_policy (GTK_SCROLLED_WINDOW (scrolled_window),
                                  GTK_POLICY_NEVER, GTK_POLICY_AUTOMATIC);
  gtk_scrolled_window_set_shadow_type (GTK_SCROLLED_WINDOW (scrolled_window),
                                       GTK_SHADOW_ETCHED_IN);

  view = gucharmap_chapters_view_new ();
  charmap->chapters_view = GUCHARMAP_CHAPTERS_VIEW (view);
  selection = gtk_tree_view_get_selection (GTK_TREE_VIEW (view));
  g_signal_connect (selection, "changed",
                    G_CALLBACK (chapters_view_selection_changed_cb), charmap);

  gtk_container_add (GTK_CONTAINER (scrolled_window), view);
  gtk_widget_show (view);
  gtk_paned_pack1 (GTK_PANED (charmap), scrolled_window, FALSE, TRUE);
  gtk_widget_show (scrolled_window);

  /* Right pane */
  notebook = gtk_notebook_new ();

  /* Chartable page */
  scrolled_window = gtk_scrolled_window_new (NULL, NULL);
  gtk_scrolled_window_set_policy (GTK_SCROLLED_WINDOW (scrolled_window),
                                  GTK_POLICY_NEVER, GTK_POLICY_AUTOMATIC);
  gtk_scrolled_window_set_shadow_type (GTK_SCROLLED_WINDOW (scrolled_window),
                                       GTK_SHADOW_NONE);

  chartable = gucharmap_chartable_new ();
  charmap->chartable = GUCHARMAP_CHARTABLE (chartable);

  g_signal_connect_swapped (chartable, "status-message",
                            G_CALLBACK (chartable_status_message), charmap);
  g_signal_connect (chartable, "notify::active-character",
                    G_CALLBACK (chartable_sync_active_char), charmap);

  gtk_container_add (GTK_CONTAINER (scrolled_window), chartable);
  gtk_widget_show (chartable);

  gtk_notebook_append_page (GTK_NOTEBOOK (notebook),
                            scrolled_window,
                            gtk_label_new_with_mnemonic (_("Characte_r Table")));
  gtk_widget_show (scrolled_window);

  /* Details page */
  scrolled_window = gtk_scrolled_window_new (NULL, NULL);
  gtk_scrolled_window_set_policy (GTK_SCROLLED_WINDOW (scrolled_window),
                                  GTK_POLICY_AUTOMATIC, GTK_POLICY_AUTOMATIC);
  gtk_scrolled_window_set_shadow_type (GTK_SCROLLED_WINDOW (scrolled_window), 
                                       GTK_SHADOW_NONE);

  textview = gtk_text_view_new ();
  charmap->details_view = GTK_TEXT_VIEW (textview);
  gtk_text_view_set_editable (GTK_TEXT_VIEW (textview), FALSE);
  gtk_text_view_set_wrap_mode (GTK_TEXT_VIEW (textview),
                               GTK_WRAP_WORD);

  g_signal_connect (textview, "key-press-event",
                    G_CALLBACK (details_key_press_event), charmap);
  g_signal_connect (textview, "event-after",
                    G_CALLBACK (details_event_after), charmap);
  g_signal_connect (textview, "motion-notify-event",
                    G_CALLBACK (details_motion_notify_event), charmap);
  g_signal_connect (textview, "visibility-notify-event",
                    G_CALLBACK (details_visibility_notify_event), charmap);

  create_tags (charmap);

  gtk_container_add (GTK_CONTAINER (scrolled_window), textview);
  gtk_widget_show (textview);

  gtk_notebook_append_page (GTK_NOTEBOOK (notebook),
                            scrolled_window,
                            gtk_label_new_with_mnemonic (_("Character _Details")));
  gtk_widget_show (scrolled_window);

  gtk_notebook_set_current_page (GTK_NOTEBOOK (notebook), 0);
  g_signal_connect (notebook, "switch-page",
                    G_CALLBACK (notebook_switch_page), charmap);

  gtk_paned_pack2 (GTK_PANED (charmap), notebook, TRUE, TRUE);
  gtk_widget_show (notebook);
}

GtkWidget *
gucharmap_charmap_new (void)
{
  return g_object_new (gucharmap_charmap_get_type (), NULL);
}

/**
 * gucharmap_chartable_set_font:
 * @chartable: a #GucharmapChartable
 * @font_name:
 *
 * Sets @font_name as the font to use to display the character table.
 */
void
gucharmap_charmap_set_font (GucharmapCharmap *charmap, 
                            const gchar *font_name)
{
  PangoFontDescription *font_desc;

  g_return_if_fail (GUCHARMAP_IS_CHARMAP (charmap));
  g_return_if_fail (font_name != NULL);

  font_desc = pango_font_description_from_string (font_name);
  if (charmap->font_desc &&
      pango_font_description_equal (font_desc, charmap->font_desc))
    {
      pango_font_description_free (font_desc);
      return;
    }

  gucharmap_charmap_set_font_desc_internal (charmap, font_desc /* adopting */);
}

/**
 * gucharmap_chartable_set_font_desc:
 * @chartable: a #GucharmapChartable
 * @font_desc: a #PangoFontDescription
 *
 * Sets @font_desc as the font to use to display the character table.
 */
void
gucharmap_charmap_set_font_desc (GucharmapCharmap *charmap,
                                 PangoFontDescription *font_desc)
{
  g_return_if_fail (GUCHARMAP_IS_CHARMAP (charmap));
  g_return_if_fail (font_desc != NULL);

  if (charmap->font_desc &&
      pango_font_description_equal (font_desc, charmap->font_desc))
    return;

  gucharmap_charmap_set_font_desc_internal (charmap,
                                            pango_font_description_copy (font_desc));
}

void
gucharmap_charmap_go_to_character (GucharmapCharmap *charmap, 
                                   gunichar          wc)
{
  gboolean status;

  /* FIXME: move wc validation up here? */

  status = gucharmap_chapters_view_select_character (charmap->chapters_view, wc);
  if (!status)
    g_warning ("gucharmap_chapters_view_select_character failed (U+%04X)\n", wc);

  if (wc <= UNICHAR_MAX)
    gucharmap_chartable_set_active_character (charmap->chartable, wc);
}

/**
 * gucharmap_charmap_get_chartable:
 * @charmap:
 *
 * Returns: the #GucharmapChartable in @charmap
 */
GucharmapChartable *
gucharmap_charmap_get_chartable (GucharmapCharmap *charmap)
{
  return charmap->chartable;
}

void
gucharmap_charmap_set_chapters_model (GucharmapCharmap  *charmap,
                                      GucharmapChaptersModel *model)
{
  gunichar wc;

  gucharmap_chapters_view_set_model (charmap->chapters_view, model);
  if (!model)
    return;

  if (charmap->last_character_set)
    wc = gucharmap_chartable_get_active_character (charmap->chartable);
  else
    wc = gucharmap_settings_get_last_char ();

  gucharmap_charmap_go_to_character (charmap, wc);
  charmap->last_character_set = TRUE;
}

GucharmapChaptersModel *
gucharmap_charmap_get_chapters_model (GucharmapCharmap *charmap)
{
  return gucharmap_chapters_view_get_model (charmap->chapters_view);
}

GucharmapChaptersView *
gucharmap_charmap_get_chapters_view  (GucharmapCharmap *charmap)
{
  return charmap->chapters_view;
}

GucharmapCodepointList *
gucharmap_charmap_get_book_codepoint_list (GucharmapCharmap *charmap)
{
  GucharmapCodepointList *codepoint_list;
  codepoint_list = (GucharmapCodepointList *) gucharmap_chapters_view_get_book_codepoint_list (charmap->chapters_view);
  return g_object_ref (codepoint_list);
}

