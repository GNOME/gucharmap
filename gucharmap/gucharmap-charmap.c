/*
 * Copyright (c) 2004 Noah Levitt
 * Copyright (c) 2007, 2008 Christian Persch
 * Copyright (c) 2016 DaeHyun Sung
 *
 * This program is free software; you can redistribute it and/or modify it
 * under the terms of the GNU General Public License as published by the
 * Free Software Foundation; either version 3 of the License, or (at your
 * option) any later version.
 *
 * This program is distributed in the hope that it will be useful, but
 * WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License along
 * with this program; if not, write to the Free Software Foundation, Inc.,
 * 51 Franklin Street, Fifth Floor, Boston, MA 02110-1335 USA
 */

#include <config.h>

#include <glib/gi18n-lib.h>
#include <gtk/gtk.h>
#include <stdlib.h>
#include <string.h>

#include "gucharmap.h"
#include "gucharmap-marshal.h"
#include "gucharmap-private.h"

struct _GucharmapCharmapPrivate {
  GtkWidget *notebook;
  GucharmapChaptersView *chapters_view;
  GucharmapChartable *chartable;
  GtkTextView *details_view;
  GtkTextTag *text_tag_gimongous;
  GtkTextTag *text_tag_big;

  PangoFontDescription *font_desc;

  GdkCursor *hand_cursor;
  GdkCursor *regular_cursor;

  guint active_page;

  guint hovering_over_link   : 1;
  guint last_character_set   : 1;
};

enum
{
  STATUS_MESSAGE,
  LINK_CLICKED,
  NUM_SIGNALS
};

enum {
  PROP_0,
  PROP_CHAPTERS_MODEL,
  PROP_ACTIVE_CHAPTER,
  PROP_ACTIVE_CHARACTER,
  PROP_ACTIVE_CODEPOINT_LIST,
  PROP_ACTIVE_PAGE,
  PROP_SNAP_POW2,
  PROP_FONT_DESC,
  PROP_FONT_FALLBACK
};

static guint gucharmap_charmap_signals[NUM_SIGNALS];

static void gucharmap_charmap_class_init (GucharmapCharmapClass *klass);
static void gucharmap_charmap_init       (GucharmapCharmap *charmap);

G_DEFINE_TYPE (GucharmapCharmap, gucharmap_charmap, GTK_TYPE_PANED)

static void
gucharmap_charmap_finalize (GObject *object)
{
  GucharmapCharmap *charmap = GUCHARMAP_CHARMAP (object);
  GucharmapCharmapPrivate *priv = charmap->priv;

  g_object_unref (priv->hand_cursor);
  g_object_unref (priv->regular_cursor);

  if (priv->font_desc)
    pango_font_description_free (priv->font_desc);

  G_OBJECT_CLASS (gucharmap_charmap_parent_class)->finalize (object);
}

static void
gucharmap_charmap_get_property (GObject *object,
                                guint prop_id,
                                GValue *value,
                                GParamSpec *pspec)
{
  GucharmapCharmap *charmap = GUCHARMAP_CHARMAP (object);
  GucharmapCharmapPrivate *priv = charmap->priv;

  switch (prop_id) {
    case PROP_CHAPTERS_MODEL:
      g_value_set_object (value, gucharmap_charmap_get_chapters_model (charmap));
      break;
    case PROP_ACTIVE_CHAPTER:
      g_value_take_string (value, gucharmap_chapters_view_get_selected (priv->chapters_view));
      break;
    case PROP_ACTIVE_CHARACTER:
      g_value_set_uint (value, gucharmap_charmap_get_active_character (charmap));
      break;
    case PROP_ACTIVE_CODEPOINT_LIST:
      g_value_set_object (value, gucharmap_charmap_get_active_codepoint_list (charmap));
      break;
    case PROP_ACTIVE_PAGE:
      g_value_set_uint (value, gucharmap_charmap_get_active_page (charmap));
      break;
    case PROP_FONT_DESC:
      g_value_set_boxed (value, gucharmap_charmap_get_font_desc (charmap));
      break;
    case PROP_FONT_FALLBACK:
      g_value_set_boolean (value, gucharmap_charmap_get_font_fallback (charmap));
      break;
    case PROP_SNAP_POW2:
      g_value_set_boolean (value, gucharmap_charmap_get_snap_pow2 (charmap));
      break;
    default:
      G_OBJECT_WARN_INVALID_PROPERTY_ID (object, prop_id, pspec);
      break;
  }
}

static void
gucharmap_charmap_set_property (GObject *object,
                                guint prop_id,
                                const GValue *value,
                                GParamSpec *pspec)
{
  GucharmapCharmap *charmap = GUCHARMAP_CHARMAP (object);
  GucharmapCharmapPrivate *priv = charmap->priv;

  switch (prop_id) {
    case PROP_CHAPTERS_MODEL:
      gucharmap_charmap_set_chapters_model (charmap, g_value_get_object (value));
      break;
    case PROP_ACTIVE_CHAPTER:
      gucharmap_chapters_view_set_selected (priv->chapters_view,
                                            g_value_get_string (value));
      break;
    case PROP_ACTIVE_CHARACTER:
      gucharmap_charmap_set_active_character (charmap, g_value_get_uint (value));
      break;
    case PROP_ACTIVE_PAGE:
      gucharmap_charmap_set_active_page (charmap, g_value_get_uint (value));
      break;
    case PROP_FONT_DESC:
      gucharmap_charmap_set_font_desc (charmap, g_value_get_boxed (value));
      break;
    case PROP_FONT_FALLBACK:
      gucharmap_charmap_set_font_fallback (charmap, g_value_get_boolean (value));
      break;
    case PROP_SNAP_POW2:
      gucharmap_charmap_set_snap_pow2 (charmap, g_value_get_boolean (value));
      break;
    case PROP_ACTIVE_CODEPOINT_LIST: /* read-only */
    default:
      G_OBJECT_WARN_INVALID_PROPERTY_ID (object, prop_id, pspec);
      break;
  }
}

static void
gucharmap_charmap_class_init (GucharmapCharmapClass *klass)
{
  GObjectClass *object_class = G_OBJECT_CLASS (klass);

  _gucharmap_intl_ensure_initialized ();

  object_class->get_property = gucharmap_charmap_get_property;
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

  g_object_class_install_property
    (object_class,
     PROP_CHAPTERS_MODEL,
     g_param_spec_object ("chapters-model", NULL, NULL,
                         GUCHARMAP_TYPE_CHAPTERS_MODEL,
                         G_PARAM_WRITABLE |
                         G_PARAM_CONSTRUCT |
                         G_PARAM_STATIC_NAME |
                         G_PARAM_STATIC_NICK |
                         G_PARAM_STATIC_BLURB));

  g_object_class_install_property
    (object_class,
     PROP_ACTIVE_CHAPTER,
     g_param_spec_string ("active-chapter", NULL, NULL,
                          NULL,
                          G_PARAM_READWRITE |
                          G_PARAM_STATIC_NAME |
                          G_PARAM_STATIC_NICK |
                          G_PARAM_STATIC_BLURB));

  g_object_class_install_property
    (object_class,
     PROP_ACTIVE_CHARACTER,
     g_param_spec_uint ("active-character", NULL, NULL,
                        0,
                        UNICHAR_MAX,
                        0,
                        G_PARAM_READWRITE |
                        G_PARAM_STATIC_NAME |
                        G_PARAM_STATIC_NICK |
                        G_PARAM_STATIC_BLURB));

  g_object_class_install_property
    (object_class,
     PROP_ACTIVE_CODEPOINT_LIST,
     g_param_spec_object ("active-codepoint-list", NULL, NULL,
                          GUCHARMAP_TYPE_CODEPOINT_LIST,
                          G_PARAM_READABLE |
                          G_PARAM_STATIC_NAME |
                          G_PARAM_STATIC_NICK |
                          G_PARAM_STATIC_BLURB));

  g_object_class_install_property
    (object_class,
     PROP_ACTIVE_PAGE,
     g_param_spec_uint ("active-page", NULL, NULL,
                        0,
                        G_MAXUINT,
                        0,
                        G_PARAM_READWRITE |
                        G_PARAM_STATIC_NAME |
                        G_PARAM_STATIC_NICK |
                        G_PARAM_STATIC_BLURB));

  g_object_class_install_property
    (object_class,
     PROP_FONT_DESC,
     g_param_spec_boxed ("font-desc", NULL, NULL,
                         PANGO_TYPE_FONT_DESCRIPTION,
                         G_PARAM_READWRITE |
                         G_PARAM_STATIC_NAME |
                         G_PARAM_STATIC_NICK |
                         G_PARAM_STATIC_BLURB));

  /**
   * GucharmapCharmap:font-fallback:
   *
   * Whether font fallback is enabled.
   *
   * Since: 2.34
   */
  g_object_class_install_property
    (object_class,
     PROP_FONT_FALLBACK,
     g_param_spec_boolean ("font-fallback", NULL, NULL,
                           TRUE,
                           G_PARAM_READWRITE | G_PARAM_STATIC_STRINGS));

  g_object_class_install_property
    (object_class,
     PROP_SNAP_POW2,
     g_param_spec_boolean ("snap-power-2", NULL, NULL,
                           FALSE,
                           G_PARAM_READWRITE |
                           G_PARAM_STATIC_NAME |
                           G_PARAM_STATIC_NICK |
                           G_PARAM_STATIC_BLURB));

  g_type_class_add_private (object_class, sizeof (GucharmapCharmapPrivate));
}

static void
gucharmap_charmap_update_text_tags (GucharmapCharmap *charmap)
{
  GucharmapCharmapPrivate *priv = charmap->priv;
  GtkStyle *style;
  int default_font_size;

  style = gtk_widget_get_style (GTK_WIDGET (priv->details_view));
  default_font_size = pango_font_description_get_size (style->font_desc);

  if (priv->font_desc)
    g_object_set (priv->text_tag_gimongous, "font-desc", priv->font_desc, NULL);

  /* FIXME: do we need to consider whether the font size is absolute or not? */
  g_object_set (priv->text_tag_gimongous,
                "size", 8 * default_font_size,
                "left-margin", PANGO_PIXELS (5 * default_font_size),
                NULL);
  g_object_set (priv->text_tag_big,
                "size", default_font_size * 5 / 4,
                NULL);
}

static void
gucharmap_charmap_set_font_desc_internal (GucharmapCharmap *charmap, 
                                          PangoFontDescription *font_desc /* adopting */,
                                          gboolean in_notification)
{
  GucharmapCharmapPrivate *priv = charmap->priv;
  GObject *object = G_OBJECT (charmap);
  gboolean equal;

  g_object_freeze_notify (object);

  equal = priv->font_desc != NULL &&
          pango_font_description_equal (priv->font_desc, font_desc);

  if (priv->font_desc)
    pango_font_description_free (priv->font_desc);

  priv->font_desc = font_desc; /* adopted */

  if (!in_notification)
    gucharmap_chartable_set_font_desc (priv->chartable, font_desc);

  if (gtk_widget_get_style (GTK_WIDGET (priv->details_view)))
    gucharmap_charmap_update_text_tags (charmap);

  if (!equal)
    g_object_notify (G_OBJECT (charmap), "font-desc");

  g_object_thaw_notify (object);
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
  char buf[7];
  GUnicodeType t;
  gboolean is_graph, is_Mn;
  char nbsp[3] = "\302\240\0"; /* U+00A0 NO-BREAK SPACE */

  t = g_unichar_type (uc);
  is_Mn = t == G_UNICODE_NON_SPACING_MARK /* Mn */;
  is_graph = g_unichar_isgraph (uc);

  buf[g_unichar_to_utf8 (uc, buf)] = '\0';

  str = g_strdup_printf ("%s%s%sU+%4.4X %s",
                         /* Per unicode standard, exhibit nonspacing marks on NBSP */
                         is_graph && is_Mn ? nbsp : "",
                         is_graph ? buf : "",
                         is_graph ? " " : "",
                         uc,
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

typedef enum {
  TAG_NONE,
  TAG_FONT,
  TAG_NO_BREAK,
  TAG_INITIAL,
  TAG_MEDIAL,
  TAG_FINAL,
  TAG_ISOLATED,
  TAG_CIRCLE,
  TAG_SUPER,
  TAG_SUB,
  TAG_VERTICAL,
  TAG_WIDE,
  TAG_NARROW,
  TAG_SMALL,
  TAG_SQUARE,
  TAG_FRACTION,
  TAG_COMPAT
} CompatibilityFormattingTag;

static void
insert_compatibility_formatting_tag (GucharmapCharmap *charmap,
                                     GtkTextBuffer *buffer,
                                     GtkTextIter *iter,
                                     CompatibilityFormattingTag tag)
{
  static const char *tag_names[] = {
    [TAG_NONE] = "",
    [TAG_FONT] = N_("Font variant of"),
    [TAG_NO_BREAK] = N_("No-break version of"),
    [TAG_INITIAL] = N_("Initial presentation form of"),
    [TAG_MEDIAL] = N_("Medial presentation form of"),
    [TAG_FINAL] = N_("Final presentation form of"),
    [TAG_ISOLATED] = N_("Isolated presentation form of"),
    [TAG_CIRCLE] = N_("Encircled form of"),
    [TAG_SUPER] = N_("Superscript form of"),
    [TAG_SUB] = N_("Subscript form of"),
    [TAG_VERTICAL] = N_("Vertical layout presentation form of"),
    [TAG_WIDE] = N_("Wide compatibility character of"),
    [TAG_NARROW] = N_("Narrow compatibility character of"),
    [TAG_SMALL] = N_("Small variant form of"),
    [TAG_SQUARE] = N_("CJK squared font variant of"),
    [TAG_FRACTION] = N_("Vulgar fraction form of"),
    [TAG_COMPAT] = N_("Unspecified compatibility character of")
  };
  char *str;

  str = g_strdup_printf ("%s ", _(tag_names[tag]));
  gtk_text_buffer_insert (buffer, iter, str, -1);
  g_free (str);
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

static CompatibilityFormattingTag
parse_tag (const char *str,
           gsize len)
{
  if (strncmp (str, "font", len) == 0)
    return TAG_FONT;
  if (strncmp (str, "noBreak", len) == 0)
    return TAG_NO_BREAK;
  if (strncmp (str, "initial", len) == 0)
    return TAG_INITIAL;
  if (strncmp (str, "medial", len) == 0)
    return TAG_MEDIAL;
  if (strncmp (str, "final", len) == 0)
    return TAG_FINAL;
  if (strncmp (str, "isolated", len) == 0)
    return TAG_ISOLATED;
  if (strncmp (str, "circle", len) == 0)
    return TAG_CIRCLE;
  if (strncmp (str, "super", len) == 0)
    return TAG_SUPER;
  if (strncmp (str, "sub", len) == 0)
    return TAG_SUB;
  if (strncmp (str, "vertical", len) == 0)
    return TAG_VERTICAL;
  if (strncmp (str, "wide", len) == 0)
    return TAG_WIDE;
  if (strncmp (str, "narrow", len) == 0)
    return TAG_NARROW;
  if (strncmp (str, "small", len) == 0)
    return TAG_SMALL;
  if (strncmp (str, "square", len) == 0)
    return TAG_SQUARE;
  if (strncmp (str, "fraction", len) == 0)
    return TAG_FRACTION;
  if (strncmp (str, "compat", len) == 0)
    return TAG_COMPAT;

  g_printerr ("unrecognised tag '%s'\n", str);
  return TAG_NONE;
}

/* returns the string after the tag */
static const char *
find_tag (const char *str,
          CompatibilityFormattingTag *tag)
{
  const char *csb;

  *tag = TAG_NONE;

  if (str[0] != '<')
    return str;

  csb = strchr (str, '>');
  if (csb == NULL)
    return str;

  *tag = parse_tag (str + 1, csb - str - 1);

  str = csb + 1;
  while (g_ascii_isspace (*str))
    str++;

  return str;
}

static void
insert_string_link_codepoints (GucharmapCharmap *charmap,
                               GtkTextBuffer *buffer,
                               GtkTextIter *iter,
                               const gchar *str)
{
  const gchar *p1, *p2;
  CompatibilityFormattingTag tag;

  p1 = find_tag (str, &tag);
  if (tag != TAG_NONE)
    insert_compatibility_formatting_tag (charmap, buffer, iter, tag);

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
          /* Ignore non-codepoints for compatibility formatting tags */
          if (tag == TAG_NONE)
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
conditionally_insert_decomposition (GucharmapCharmap *charmap, 
                                    GtkTextBuffer *buffer,
                                    GtkTextIter *iter,
                                    gunichar uc,
                                    gboolean compat)
{
  gunichar decomposition[G_UNICHAR_MAX_DECOMPOSITION_LENGTH];
  gsize result_len, i;

  result_len = g_unichar_fully_decompose (uc,
                                          compat,
                                          decomposition,
                                          G_N_ELEMENTS (decomposition));

  if (result_len == 1)
    return;

  /* Only insert compat decomp if different from canonical decomp */
  if (compat) {
    gunichar canonical[G_UNICHAR_MAX_DECOMPOSITION_LENGTH];
    gsize canonical_len;

    canonical_len = g_unichar_fully_decompose (uc,
                                               FALSE /* canonical */,
                                               canonical,
                                               G_N_ELEMENTS (canonical));

    if (canonical_len == result_len &&
        memcmp (decomposition, canonical, result_len * sizeof (gunichar)) == 0)
      return;
  }

  gtk_text_buffer_insert (buffer, iter,
                          compat
                          ? _("Compatibility decomposition:")
                          : _("Canonical decomposition:"),
                          -1);
  gtk_text_buffer_insert (buffer, iter, " ", -1);

  insert_codepoint (charmap, buffer, iter, decomposition[0]);
  for (i = 1;  i < result_len;  i++)
    {
      gtk_text_buffer_insert (buffer, iter, " + ", -1);
      insert_codepoint (charmap, buffer, iter, decomposition[i]);
    }

  gtk_text_buffer_insert (buffer, iter, "\n", -1);
}

static void
set_details (GucharmapCharmap *charmap,
             gunichar uc)
{
  GucharmapCharmapPrivate *priv = charmap->priv;
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
  GucharmapUnicodeVersion version;

  buffer = gtk_text_view_get_buffer (priv->details_view);
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

  /* Unicode version */
  version = gucharmap_get_unicode_version (uc);
  if (version != GUCHARMAP_UNICODE_VERSION_UNASSIGNED)
    insert_vanilla_detail (charmap, buffer, &iter,
                           _("In Unicode since:"),
                           gucharmap_unicode_version_to_string (version));

  /* character category */
  insert_vanilla_detail (charmap, buffer, &iter, _("Unicode category:"),
                         gucharmap_get_unicode_category_name (uc));

  /* canonical decomposition */
  conditionally_insert_decomposition (charmap, buffer, &iter, uc, FALSE);
  /* compatibility decomposition */
  conditionally_insert_decomposition (charmap, buffer, &iter, uc, TRUE);

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

  /* this isn't so bad efficiency-wise */
  if (gucharmap_get_unicode_kDefinition (uc)
      || gucharmap_get_unicode_kCantonese (uc)
      || gucharmap_get_unicode_kMandarin (uc)
      || gucharmap_get_unicode_kJapaneseOn (uc)
      || gucharmap_get_unicode_kJapaneseKun (uc)
      || gucharmap_get_unicode_kTang (uc)
      || gucharmap_get_unicode_kHangul(uc)
      || gucharmap_get_unicode_kVietnamese(uc))
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
    
      csp = gucharmap_get_unicode_kHangul (uc);
      if (csp)
        insert_vanilla_detail (charmap, buffer, &iter,
                               _("Korean Pronunciation:"), csp);
      
      csp = gucharmap_get_unicode_kVietnamese (uc);
      if (csp)
        insert_vanilla_detail (charmap, buffer, &iter, 
                               _("Vietnamese Pronunciation:"), csp);
    }
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
  GucharmapCharmapPrivate *priv = charmap->priv;
  GString *gs;
  const gchar *temp;
  const gchar **temps;
  gint i;
  gunichar wc;

  wc = gucharmap_chartable_get_active_character (priv->chartable);

  /* Forward the notification */
  g_object_notify (G_OBJECT (charmap), "active-character");

  if (priv->active_page == GUCHARMAP_CHARMAP_PAGE_DETAILS)
    set_details (charmap, wc);

  gs = g_string_sized_new (256);
  g_string_append_printf (gs, "U+%4.4X %s", wc, 
                          gucharmap_get_unicode_name (wc));

  temp = gucharmap_get_unicode_kDefinition (wc);
  if (temp)
    g_string_append_printf (gs, "   %s", temp);

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

static void
chartable_sync_font_desc (GucharmapChartable *chartable,
                          GParamSpec *pspec,
                          GucharmapCharmap *charmap)
{
  GucharmapCharmapPrivate *priv = charmap->priv;
  PangoFontDescription *font_desc;
  
  font_desc = gucharmap_chartable_get_font_desc (chartable);
  gucharmap_charmap_set_font_desc_internal (charmap,
                                            pango_font_description_copy (font_desc),
                                   //FIXME         FALSE,
                                            priv->font_desc != NULL /* Do notify if we didn't have a font desc yet */);
}

static void
chartable_notify_cb (GtkWidget *widget,
                     GParamSpec *pspec,
                     GObject *charmap)
{
  const char *prop_name;

  if (pspec->name == I_("codepoint-list"))
    prop_name = "active-codepoint-list";
  else if (pspec->name == I_("snap-pow2"))
    prop_name = pspec->name;
  else
    return;

  /* Forward the notification */
  g_object_notify (charmap, prop_name);
}

static void
follow_if_link (GucharmapCharmap *charmap,
                GtkTextIter *iter)
{
  GucharmapCharmapPrivate *priv = charmap->priv;
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
                         gucharmap_chartable_get_active_character (priv->chartable),
                         uc);
          gucharmap_charmap_set_active_character (charmap, uc);
          break;
        }
    }

  if (tags) 
    g_slist_free (tags);
}

static void
details_style_set (GtkWidget *widget,
                   GtkStyle *previous_style,
                   GucharmapCharmap *charmap)
{
  gucharmap_charmap_update_text_tags (charmap);
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
      case GDK_KEY_Return:
      case GDK_KEY_ISO_Enter:
      case GDK_KEY_KP_Enter:
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
  GucharmapCharmapPrivate *priv = charmap->priv;
  GSList *tags = NULL, *tagp = NULL;
  GtkTextIter iter;
  gboolean hovering_over_link = FALSE;

  gtk_text_view_get_iter_at_location (priv->details_view,
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

  if (hovering_over_link != priv->hovering_over_link)
    {
      priv->hovering_over_link = hovering_over_link;

      if (hovering_over_link)
        gdk_window_set_cursor (gtk_text_view_get_window (priv->details_view, GTK_TEXT_WINDOW_TEXT), priv->hand_cursor);
      else
        gdk_window_set_cursor (gtk_text_view_get_window (priv->details_view, GTK_TEXT_WINDOW_TEXT), priv->regular_cursor);
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

  return FALSE;
}

static gboolean
details_visibility_notify_event (GtkWidget *text_view,
                                 GdkEventVisibility *event,
                                 GucharmapCharmap *charmap)
{
  gint wx, wy, bx, by;

  gdk_window_get_pointer (gtk_widget_get_window (text_view), &wx, &wy, NULL);

  gtk_text_view_window_to_buffer_coords (GTK_TEXT_VIEW (text_view), 
                                         GTK_TEXT_WINDOW_WIDGET,
                                         wx, wy, &bx, &by);

  set_cursor_if_appropriate (charmap, bx, by);

  return FALSE;
}

static void
notebook_switch_page (GtkNotebook *notebook,
                      GtkWidget *page,
                      guint page_num,
                      GucharmapCharmap *charmap)
{
  GucharmapCharmapPrivate *priv = charmap->priv;

  priv->active_page = page_num;

  if (page_num == GUCHARMAP_CHARMAP_PAGE_DETAILS)
    set_details (charmap, gucharmap_chartable_get_active_character (priv->chartable));
  else if (page_num == GUCHARMAP_CHARMAP_PAGE_CHARTABLE)
    {
      GtkTextBuffer *buffer;

      buffer = gtk_text_view_get_buffer (priv->details_view);
      gtk_text_buffer_set_text (buffer, "", 0);
    }

  g_object_notify (G_OBJECT (charmap), "active-page");
}

static void
chapters_view_selection_changed_cb (GtkTreeSelection *selection,
                                    GucharmapCharmap *charmap)
{
  GucharmapCharmapPrivate *priv = charmap->priv;
  GucharmapCodepointList *codepoint_list;
  GtkTreeIter iter;

  if (!gtk_tree_selection_get_selected (selection, NULL, &iter))
    return;

  codepoint_list = gucharmap_chapters_view_get_codepoint_list (priv->chapters_view);
  gucharmap_chartable_set_codepoint_list (priv->chartable, codepoint_list);
  g_object_unref (codepoint_list);

  g_object_notify (G_OBJECT (charmap), "active-chapter");
}

static void
gucharmap_charmap_init (GucharmapCharmap *charmap)
{
  GucharmapCharmapPrivate *priv;
  GtkWidget *scrolled_window, *view, *chartable, *textview;
  GtkTreeSelection *selection;
  GtkTextBuffer *buffer;
  int page;

  priv = charmap->priv = G_TYPE_INSTANCE_GET_PRIVATE (charmap, GUCHARMAP_TYPE_CHARMAP, GucharmapCharmapPrivate);

  /* FIXME: move this to realize */
  priv->hand_cursor = gdk_cursor_new (GDK_HAND2);
  priv->regular_cursor = gdk_cursor_new (GDK_XTERM);
  priv->hovering_over_link = FALSE;

  /* Left pane */
  scrolled_window = gtk_scrolled_window_new (NULL, NULL);
  gtk_scrolled_window_set_policy (GTK_SCROLLED_WINDOW (scrolled_window),
                                  GTK_POLICY_NEVER, GTK_POLICY_AUTOMATIC);
  gtk_scrolled_window_set_shadow_type (GTK_SCROLLED_WINDOW (scrolled_window),
                                       GTK_SHADOW_ETCHED_IN);

  view = gucharmap_chapters_view_new ();
  priv->chapters_view = GUCHARMAP_CHAPTERS_VIEW (view);
  selection = gtk_tree_view_get_selection (GTK_TREE_VIEW (view));
  g_signal_connect (selection, "changed",
                    G_CALLBACK (chapters_view_selection_changed_cb), charmap);

  gtk_container_add (GTK_CONTAINER (scrolled_window), view);
  gtk_widget_show (view);
  gtk_paned_pack1 (GTK_PANED (charmap), scrolled_window, FALSE, FALSE);
  gtk_widget_show (scrolled_window);

  /* Right pane */
  priv->notebook = gtk_notebook_new ();

  /* Chartable page */
  scrolled_window = gtk_scrolled_window_new (NULL, NULL);
  gtk_scrolled_window_set_policy (GTK_SCROLLED_WINDOW (scrolled_window),
                                  GTK_POLICY_NEVER, GTK_POLICY_ALWAYS);
  gtk_scrolled_window_set_shadow_type (GTK_SCROLLED_WINDOW (scrolled_window),
                                       GTK_SHADOW_NONE);
#if GTK_CHECK_VERSION (3, 15, 9)
  gtk_scrolled_window_set_overlay_scrolling (GTK_SCROLLED_WINDOW (scrolled_window),
                                             FALSE);
#endif

  chartable = gucharmap_chartable_new ();
  priv->chartable = GUCHARMAP_CHARTABLE (chartable);

  g_signal_connect_swapped (chartable, "status-message",
                            G_CALLBACK (chartable_status_message), charmap);
  g_signal_connect (chartable, "notify::active-character",
                    G_CALLBACK (chartable_sync_active_char), charmap);
  g_signal_connect (chartable, "notify::font-desc",
                    G_CALLBACK (chartable_sync_font_desc), charmap);
  g_signal_connect (chartable, "notify::codepoint-list",
                    G_CALLBACK (chartable_notify_cb), charmap);
  g_signal_connect (chartable, "notify::snap-pow2",
                    G_CALLBACK (chartable_notify_cb), charmap);

  gtk_container_add (GTK_CONTAINER (scrolled_window), chartable);
  gtk_widget_show (chartable);

  page = gtk_notebook_append_page (GTK_NOTEBOOK (priv->notebook),
                                   scrolled_window,
                                   gtk_label_new_with_mnemonic (_("Characte_r Table")));
  g_assert (page == GUCHARMAP_CHARMAP_PAGE_CHARTABLE);
  gtk_widget_show (scrolled_window);

  /* Details page */
  scrolled_window = gtk_scrolled_window_new (NULL, NULL);
  gtk_scrolled_window_set_policy (GTK_SCROLLED_WINDOW (scrolled_window),
                                  GTK_POLICY_AUTOMATIC, GTK_POLICY_AUTOMATIC);
  gtk_scrolled_window_set_shadow_type (GTK_SCROLLED_WINDOW (scrolled_window), 
                                       GTK_SHADOW_NONE);

  textview = gtk_text_view_new ();
  priv->details_view = GTK_TEXT_VIEW (textview);
  gtk_text_view_set_editable (GTK_TEXT_VIEW (textview), FALSE);
  gtk_text_view_set_cursor_visible (GTK_TEXT_VIEW (textview), FALSE);
  gtk_text_view_set_wrap_mode (GTK_TEXT_VIEW (textview),
                               GTK_WRAP_WORD);

  g_signal_connect (textview, "style-set",
                    G_CALLBACK (details_style_set), charmap);
  g_signal_connect (textview, "key-press-event",
                    G_CALLBACK (details_key_press_event), charmap);
  g_signal_connect (textview, "event-after",
                    G_CALLBACK (details_event_after), charmap);
  g_signal_connect (textview, "motion-notify-event",
                    G_CALLBACK (details_motion_notify_event), charmap);
  g_signal_connect (textview, "visibility-notify-event",
                    G_CALLBACK (details_visibility_notify_event), charmap);

  buffer = gtk_text_view_get_buffer (priv->details_view);
  priv->text_tag_gimongous =
    gtk_text_buffer_create_tag (buffer, "gimongous",
                                NULL);
  priv->text_tag_big =
    gtk_text_buffer_create_tag (buffer, "big",
                                NULL);
  gtk_text_buffer_create_tag (buffer, "bold",
                              "weight", PANGO_WEIGHT_BOLD,
                              NULL);
  gtk_text_buffer_create_tag (buffer, "detail-value",
                              NULL);

  gtk_container_add (GTK_CONTAINER (scrolled_window), textview);
  gtk_widget_show (textview);

  page = gtk_notebook_append_page (GTK_NOTEBOOK (priv->notebook),
                                   scrolled_window,
                                   gtk_label_new_with_mnemonic (_("Character _Details")));
  g_assert (page == GUCHARMAP_CHARMAP_PAGE_DETAILS);
  gtk_widget_show (scrolled_window);

  priv->active_page = 0;
  gtk_notebook_set_current_page (GTK_NOTEBOOK (priv->notebook), 0);
  g_signal_connect (priv->notebook, "switch-page",
                    G_CALLBACK (notebook_switch_page), charmap);

  gtk_paned_pack2 (GTK_PANED (charmap), priv->notebook, TRUE, FALSE);
  gtk_widget_show (priv->notebook);

  gtk_widget_set_hexpand (GTK_WIDGET (charmap), TRUE);
  gtk_widget_set_vexpand (GTK_WIDGET (charmap), TRUE);
}

GtkWidget *
gucharmap_charmap_new (void)
{
  return g_object_new (GUCHARMAP_TYPE_CHARMAP,
                       "orientation", GTK_ORIENTATION_HORIZONTAL,
                       NULL);
}

#ifndef GUCHARMAP_DISABLE_DEPRECATED_SOURCE

/**
 * gucharmap_charmap_set_orientation:
 * @charmap:
 * @orientation:
 *
 * Deprecated: 2.26
 */
void
gucharmap_charmap_set_orientation (GucharmapCharmap *charmap,
                                   GtkOrientation orientation)
{
  gtk_orientable_set_orientation (GTK_ORIENTABLE (charmap), orientation);
}

/**
 * gucharmap_charmap_get_orientation:
 * @charmap:
 *
 * Deprecated: 2.25.0
 */
GtkOrientation
gucharmap_charmap_get_orientation (GucharmapCharmap *charmap)
{
  g_return_val_if_fail (GUCHARMAP_IS_CHARMAP (charmap), GTK_ORIENTATION_HORIZONTAL);

  return gtk_orientable_get_orientation (GTK_ORIENTABLE (charmap));
}

#endif /* !GUCHARMAP_DISABLE_DEPRECATED_SOURCE */

/**
 * gucharmap_charmap_set_font_desc:
 * @charmap:
 * @font_desc: a #PangoFontDescription
 *
 * Sets @font_desc as the font to use to display the character table.
 */
void
gucharmap_charmap_set_font_desc (GucharmapCharmap *charmap,
                                 PangoFontDescription *font_desc)
{
  GucharmapCharmapPrivate *priv;

  g_return_if_fail (GUCHARMAP_IS_CHARMAP (charmap));
  g_return_if_fail (font_desc != NULL);

  priv = charmap->priv;
  if (priv->font_desc &&
      pango_font_description_equal (font_desc, priv->font_desc))
    return;

  gucharmap_charmap_set_font_desc_internal (charmap,
                                            pango_font_description_copy (font_desc),
                                            FALSE);
}

/**
 * gucharmap_charmap_get_font_desc:
 * @charmap: a #GucharmapCharmap
 *
 * Returns: the #PangoFontDescription used to display the character table.
 *   The returned object is owned by @charmap and must not be modified or freed.
 */
PangoFontDescription *
gucharmap_charmap_get_font_desc (GucharmapCharmap *charmap)
{
  g_return_val_if_fail (GUCHARMAP_IS_CHARMAP (charmap), NULL);

  return charmap->priv->font_desc;
}

/**
 * gucharmap_charmap_set_font_fallback:
 * @charmap: a #GucharmapCharmap
 * @enable_font_fallback: whether to enable font fallback
 *
 * Since: 2.34
 */
void 
gucharmap_charmap_set_font_fallback (GucharmapCharmap *charmap,
                                     gboolean enable_font_fallback)
{
  g_return_if_fail (GUCHARMAP_IS_CHARMAP (charmap));

  gucharmap_chartable_set_font_fallback (charmap->priv->chartable, 
                                         enable_font_fallback);

  g_object_notify (G_OBJECT (charmap), "font-fallback");
}

/**
 * gucharmap_charmap_get_font_fallback:
 * @charmap: a #GucharmapCharmap
 *
 * Returns: whether font fallback is enabled
 *
 * Since: 2.34
 */
gboolean
gucharmap_charmap_get_font_fallback (GucharmapCharmap *charmap)
{
  g_return_val_if_fail (GUCHARMAP_IS_CHARMAP (charmap), FALSE);

  return gucharmap_chartable_get_font_fallback (charmap->priv->chartable);
}

void
gucharmap_charmap_set_active_character (GucharmapCharmap *charmap,
                                        gunichar          wc)
{
  GucharmapCharmapPrivate *priv;

   if (wc > UNICHAR_MAX)
    return;

  priv = charmap->priv;
  if (!gucharmap_chapters_view_select_character (priv->chapters_view, wc)) {
    g_warning ("gucharmap_chapters_view_select_character failed (U+%04X)\n", wc);
    return;
  }

  gucharmap_chartable_set_active_character (priv->chartable, wc);
}

gunichar
gucharmap_charmap_get_active_character (GucharmapCharmap *charmap)
{
  GucharmapCharmapPrivate *priv = charmap->priv;

  return gucharmap_chartable_get_active_character (priv->chartable);
}

void
gucharmap_charmap_set_active_chapter (GucharmapCharmap *charmap,
                                      const gchar *chapter)
{
  GucharmapCharmapPrivate *priv = charmap->priv;

  gucharmap_chapters_view_set_selected (priv->chapters_view, chapter);
}

char *
gucharmap_charmap_get_active_chapter (GucharmapCharmap *charmap)
{
  GucharmapCharmapPrivate *priv = charmap->priv;

  return gucharmap_chapters_view_get_selected (priv->chapters_view);
}

void
gucharmap_charmap_next_chapter (GucharmapCharmap *charmap)
{
  GucharmapCharmapPrivate *priv = charmap->priv;

  gucharmap_chapters_view_next (priv->chapters_view);
}

void
gucharmap_charmap_previous_chapter (GucharmapCharmap *charmap)
{
  GucharmapCharmapPrivate *priv = charmap->priv;

  gucharmap_chapters_view_previous (priv->chapters_view);
}

/**
 * gucharmap_charmap_get_chartable:
 * @charmap: a #GucharmapCharmap
 *
 * Returns: (transfer none): a #GucharmapChartable
 */
GucharmapChartable *
gucharmap_charmap_get_chartable (GucharmapCharmap *charmap)
{
  GucharmapCharmapPrivate *priv = charmap->priv;

  return priv->chartable;
}

void
gucharmap_charmap_set_chapters_model (GucharmapCharmap  *charmap,
                                      GucharmapChaptersModel *model)
{
  GucharmapCharmapPrivate *priv = charmap->priv;
  GObject *object = G_OBJECT (charmap);
  gunichar wc;

  g_object_freeze_notify (object);

  g_object_notify (G_OBJECT (charmap), "chapters-model");

  gucharmap_chapters_view_set_model (priv->chapters_view, model);
  if (!model) {
    g_object_thaw_notify (object);
    return;
  }

  if (priv->last_character_set) {
    wc = gucharmap_chartable_get_active_character (priv->chartable);
    gucharmap_charmap_set_active_character (charmap, wc);
  }

  priv->last_character_set = TRUE;

  g_object_thaw_notify (object);
}

/**
 * gucharmap_charmap_get_chapters_model:
 * @charmap: a #GucharmapCharmap
 *
 * Gets the #GucharmapChaptersModel associated with the #GucharmapChaptersView
 * of @charmap.
 *
 * Returns: (transfer none): a #GucharmapChaptersModel
 */
GucharmapChaptersModel *
gucharmap_charmap_get_chapters_model (GucharmapCharmap *charmap)
{
  GucharmapCharmapPrivate *priv = charmap->priv;

  return gucharmap_chapters_view_get_model (priv->chapters_view);
}

/**
 * gucharmap_charmap_get_chapters_view:
 * @charmap: a #GucharmapCharmap
 *
 * Gets the #GucharmapChaptersView of @charmap
 *
 * Returns: (transfer none): the #GucharmapChaptersView
 */
GucharmapChaptersView *
gucharmap_charmap_get_chapters_view (GucharmapCharmap *charmap)
{
  GucharmapCharmapPrivate *priv = charmap->priv;

  return priv->chapters_view;
}

/**
 * gucharmap_charmap_get_book_codepoint_list:
 * @charmap: a #GucharmapCharmap
 *
 * Returns: (transfer full): the GucharmapCodepointList. Must be freed with
 * g_object_unref().
 */
GucharmapCodepointList *
gucharmap_charmap_get_book_codepoint_list (GucharmapCharmap *charmap)
{
  GucharmapCharmapPrivate *priv = charmap->priv;

  return gucharmap_chapters_view_get_book_codepoint_list (priv->chapters_view);
}

void
gucharmap_charmap_set_chapters_visible (GucharmapCharmap *charmap,
                                        gboolean visible)
{
  GucharmapCharmapPrivate *priv = charmap->priv;

  g_object_set (priv->chapters_view, "visible", visible, NULL);
}

gboolean
gucharmap_charmap_get_chapters_visible (GucharmapCharmap *charmap)
{
  GucharmapCharmapPrivate *priv = charmap->priv;

  return gtk_widget_get_visible (GTK_WIDGET (priv->chapters_view));
}

void
gucharmap_charmap_set_page_visible (GucharmapCharmap *charmap,
                                    int page,
                                    gboolean visible)
{
  GucharmapCharmapPrivate *priv = charmap->priv;
  GtkWidget *page_widget;

  page_widget = gtk_notebook_get_nth_page (GTK_NOTEBOOK (priv->notebook), page);
  if (!page_widget)
    return;

  g_object_set (page_widget, "visible", visible, NULL);
}

gboolean
gucharmap_charmap_get_page_visible (GucharmapCharmap *charmap,
                                    int page)
{
  GucharmapCharmapPrivate *priv = charmap->priv;
  GtkWidget *page_widget;

  page_widget = gtk_notebook_get_nth_page (GTK_NOTEBOOK (priv->notebook), page);
  if (!page_widget)
    return FALSE;

  return gtk_widget_get_visible (page_widget);
}

void
gucharmap_charmap_set_active_page (GucharmapCharmap *charmap,
                                   int page)
{
  GucharmapCharmapPrivate *priv = charmap->priv;

  gtk_notebook_set_current_page (GTK_NOTEBOOK (priv->notebook), page);
}

int
gucharmap_charmap_get_active_page (GucharmapCharmap *charmap)
{
  GucharmapCharmapPrivate *priv = charmap->priv;

  return priv->active_page;
}

/**
 * gucharmap_charmap_get_active_codepoint_list:
 * @charmap: a #GucharmapCharmap
 *
 * Gets the @GucharmapCodepointList associated with the #GucharmapChartable of
 * @charmap.
 *
 * Returns: (transfer none): the #GucharmapCodepointList
 */
GucharmapCodepointList *
gucharmap_charmap_get_active_codepoint_list (GucharmapCharmap *charmap)
{
  GucharmapCharmapPrivate *priv = charmap->priv;

  return gucharmap_chartable_get_codepoint_list (priv->chartable);
}

/**
 * gucharmap_charmap_set_snap_pow2:
 * @charmap: a #GucharmapCharmap
 * @snap: whether to enable or disable snapping
 *
 * Sets whether the number columns the character table shows should
 * always be a power of 2.
 */
void
gucharmap_charmap_set_snap_pow2 (GucharmapCharmap *charmap,
                                 gboolean snap)
{
  GucharmapCharmapPrivate *priv = charmap->priv;

  /* This will cause us to emit our prop notification too */
  gucharmap_chartable_set_snap_pow2 (priv->chartable, snap);
}

/**
 * gucharmap_charmap_get_snap_pow2:
 * @charmap: a #GucharmapCharmap
 *
 * Returns whether the number of columns the character table shows is
 * always a power of 2.
 */
gboolean
gucharmap_charmap_get_snap_pow2 (GucharmapCharmap *charmap)
{
  GucharmapCharmapPrivate *priv = charmap->priv;

  return gucharmap_chartable_get_snap_pow2 (priv->chartable);
}
