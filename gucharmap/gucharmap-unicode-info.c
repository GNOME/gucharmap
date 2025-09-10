/*
 * Copyright © 2004 Noah Levitt
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

#include <gtk/gtk.h>
#include <string.h>

#include <glib/gi18n-lib.h>

#include "gucharmap.h"
#include "gucharmap-private.h"

#include "unicode-names.h"
#include "unicode-blocks.h"
#include "unicode-nameslist.h"
#include "unicode-categories.h"
#include "unicode-versions.h"
#include "unicode-unihan.h"

/* constants for hangul (de)composition, see UAX #15 */
#define SBase 0xAC00
#define LCount 19
#define VCount 21
#define TCount 28
#define NCount (VCount * TCount)
#define SCount (LCount * NCount)

static const gchar JAMO_L_TABLE[][4] = {
  "G", "GG", "N", "D", "DD", "R", "M", "B", "BB",
  "S", "SS", "", "J", "JJ", "C", "K", "T", "P", "H"
};

static const gchar JAMO_V_TABLE[][4] = {
  "A", "AE", "YA", "YAE", "EO", "E", "YEO", "YE", "O",
  "WA", "WAE", "OE", "YO", "U", "WEO", "WE", "WI",
  "YU", "EU", "YI", "I"
};

static const gchar JAMO_T_TABLE[][4] = {
  "", "G", "GG", "GS", "N", "NJ", "NH", "D", "L", "LG", "LM",
  "LB", "LS", "LT", "LP", "LH", "M", "B", "BS",
  "S", "SS", "NG", "J", "C", "K", "T", "P", "H"
};

const gchar *
gucharmap_get_unicode_name (gunichar wc)
{
  static gchar buf[64];

  _gucharmap_intl_ensure_initialized ();

  if ((wc >= 0x3400 && wc <= 0x4dbf)       /* CJK Unified Ideographs Extension A */
      || (wc >= 0x4e00 && wc <= 0x9fff)    /* CJK Unified Ideographs             */
      || (wc >= 0x20000 && wc <= 0x2a6df)  /* CJK Unified Ideographs Extension B */
      || (wc >= 0x2a700 && wc <= 0x2b73f)  /* CJK Unified Ideographs Extension C */
      || (wc >= 0x2b740 && wc <= 0x2b81d)  /* CJK Unified Ideographs Extension D */
      || (wc >= 0x2b820 && wc <= 0x2cead)  /* CJK Unified Ideographs Extension E */
      || (wc >= 0x2ceb0 && wc <= 0x2ebe0)  /* CJK Unified Ideographs Extension F */
      || (wc >= 0x2ebf0 && wc <= 0x2ee5d)  /* CJK Unified Ideographs Extension I */
      || (wc >= 0x30000 && wc <= 0x3134a)  /* CJK Unified Ideographs Extension G */
      || (wc >= 0x31350 && wc <= 0x323af)  /* CJK Unified Ideographs Extension H */
      || (wc >= 0x323b0 && wc <= 0x33479)) /* CJK Unified Ideographs Extension J */
    {
      g_snprintf (buf, sizeof (buf), "CJK UNIFIED IDEOGRAPH-%04X", wc);
      return buf;
    }
  else if ((wc >= 0xf900 && wc <= 0xfaff) || /* CJK Compatibility Ideographs            */
           (wc >= 0x2f800 && wc <= 0x2fa1d)) /* CJK Compatibility Ideographs Supplement */
    {
      g_snprintf (buf, sizeof (buf), "CJK COMPATIBILITY IDEOGRAPH-%04X", wc);
      return buf;
  }
  else if ((wc >= 0x17000 && wc <= 0x187ff) || /* Tangut            */
           (wc >= 0x18d00 && wc <= 0x18d1e))   /* Tangut Supplement */
    {
      g_snprintf (buf, sizeof (buf), "TANGUT IDEOGRAPH-%05X", wc);
      return buf;
  }
  else if (wc >= 0x18800 && wc <= 0x18aff) {
      g_snprintf (buf, sizeof (buf), "TANGUT COMPONENT-%03u", wc - 0x18800 + 1);
      return buf;
  }
  else if (wc >= 0x18d80 && wc <= 0x18df2) {
      g_snprintf (buf, sizeof (buf), "TANGUT COMPONENT-%03u", wc - 0x18d80 + 769);
      return buf;
  }
  else if (wc >= 0x18b00 && wc <= 0x18cd5) {
      g_snprintf (buf, sizeof (buf), "KHITAN SMALL SCRIPT CHARACTER-%05X", wc);
      return buf;
  }
  else if (wc >= 0x1b170 && wc <= 0x1b2fb) {
      g_snprintf (buf, sizeof (buf), "NUSHU CHARACTER-%05X", wc);
      return buf;
  }
  else if (wc >= 0x13460 && wc <= 0x143fa) {
      g_snprintf (buf, sizeof (buf), "EGYPTIAN HIEROGLYPH-%05X", wc);
      return buf;
  }
  else if (wc >= 0xac00 && wc <= 0xd7af)
    {
      /* compute hangul syllable name as per UAX #15 */
      gint SIndex = wc - SBase;
      gint LIndex, VIndex, TIndex;

      if (SIndex < 0 || SIndex >= SCount)
        return "";

      LIndex = SIndex / NCount;
      VIndex = (SIndex % NCount) / TCount;
      TIndex = SIndex % TCount;

      g_snprintf (buf, sizeof (buf), "HANGUL SYLLABLE %s%s%s", 
                  JAMO_L_TABLE[LIndex], JAMO_V_TABLE[VIndex], JAMO_T_TABLE[TIndex]);

      return buf;
    }
  else if (wc >= 0xD800 && wc <= 0xDB7F) 
    return _("<Non Private Use High Surrogate>");
  else if (wc >= 0xDB80 && wc <= 0xDBFF) 
    return _("<Private Use High Surrogate>");
  else if (wc >= 0xDC00 && wc <= 0xDFFF)
    return _("<Low Surrogate>");
  else if (wc >= 0xE000 && wc <= 0xF8FF) 
    return _("<Private Use>");
  else if (wc >= 0xF0000 && wc <= 0xFFFFD)
    return _("<Plane 15 Private Use>");
  else if (wc >= 0x100000 && wc <= 0x10FFFD)
    return _("<Plane 16 Private Use>");
  else
    {
      const gchar *x = gucharmap_get_unicode_data_name (wc);
      if (x == NULL)
        return _("<not assigned>");
      else
        return x;
    }
}

const gchar *
gucharmap_get_unicode_category_name (gunichar wc)
{
  _gucharmap_intl_ensure_initialized ();

  switch (gucharmap_unichar_type (wc))
    {
      case G_UNICODE_CONTROL: return _("Other, Control");
      case G_UNICODE_FORMAT: return _("Other, Format");
      case G_UNICODE_UNASSIGNED: return _("Other, Not Assigned");
      case G_UNICODE_PRIVATE_USE: return _("Other, Private Use");
      case G_UNICODE_SURROGATE: return _("Other, Surrogate");
      case G_UNICODE_LOWERCASE_LETTER: return _("Letter, Lowercase");
      case G_UNICODE_MODIFIER_LETTER: return _("Letter, Modifier");
      case G_UNICODE_OTHER_LETTER: return _("Letter, Other");
      case G_UNICODE_TITLECASE_LETTER: return _("Letter, Titlecase");
      case G_UNICODE_UPPERCASE_LETTER: return _("Letter, Uppercase");
      case G_UNICODE_COMBINING_MARK: return _("Mark, Spacing Combining");
      case G_UNICODE_ENCLOSING_MARK: return _("Mark, Enclosing");
      case G_UNICODE_NON_SPACING_MARK: return _("Mark, Non-Spacing");
      case G_UNICODE_DECIMAL_NUMBER: return _("Number, Decimal Digit");
      case G_UNICODE_LETTER_NUMBER: return _("Number, Letter");
      case G_UNICODE_OTHER_NUMBER: return _("Number, Other");
      case G_UNICODE_CONNECT_PUNCTUATION: return _("Punctuation, Connector");
      case G_UNICODE_DASH_PUNCTUATION: return _("Punctuation, Dash");
      case G_UNICODE_CLOSE_PUNCTUATION: return _("Punctuation, Close");
      case G_UNICODE_FINAL_PUNCTUATION: return _("Punctuation, Final Quote");
      case G_UNICODE_INITIAL_PUNCTUATION: return _("Punctuation, Initial Quote");
      case G_UNICODE_OTHER_PUNCTUATION: return _("Punctuation, Other");
      case G_UNICODE_OPEN_PUNCTUATION: return _("Punctuation, Open");
      case G_UNICODE_CURRENCY_SYMBOL: return _("Symbol, Currency");
      case G_UNICODE_MODIFIER_SYMBOL: return _("Symbol, Modifier");
      case G_UNICODE_MATH_SYMBOL: return _("Symbol, Math");
      case G_UNICODE_OTHER_SYMBOL: return _("Symbol, Other");
      case G_UNICODE_LINE_SEPARATOR: return _("Separator, Line");
      case G_UNICODE_PARAGRAPH_SEPARATOR: return _("Separator, Paragraph");
      case G_UNICODE_SPACE_SEPARATOR: return _("Separator, Space");
      default: return "";
    }
}

/* does a binary search on unicode_names */
const gchar *
gucharmap_get_unicode_data_name (gunichar uc)
{
  gint min = 0;
  gint mid;
  gint max = G_N_ELEMENTS(unicode_names) - 1;

  if (uc < unicode_names[0].index || uc > unicode_names[max].index)
    return "";

  while (max >= min) 
    {
      mid = (min + max) / 2;
      if (uc > unicode_names[mid].index)
        min = mid + 1;
      else if (uc < unicode_names[mid].index)
        max = mid - 1;
      else
        return unicode_name_get_name(&unicode_names[mid]);
    }

  return NULL;
}

gint
gucharmap_get_unicode_data_name_count (void)
{
  return G_N_ELEMENTS (unicode_names);
}

/* does a binary search on unicode_versions */
GucharmapUnicodeVersion
gucharmap_get_unicode_version (gunichar uc)
{
  gint min = 0;
  gint mid;
  gint max = G_N_ELEMENTS (unicode_versions) - 1;

  if (uc < unicode_versions[0].start || uc > unicode_versions[max].end)
    return GUCHARMAP_UNICODE_VERSION_UNASSIGNED;

  while (max >= min)
    {
      mid = (min + max) / 2;

      if (uc > unicode_versions[mid].end)
        min = mid + 1;
      else if (uc < unicode_versions[mid].start)
        max = mid - 1;
      else if ((uc >= unicode_versions[mid].start) && (uc <= unicode_versions[mid].end))
        return unicode_versions[mid].version;
    }

  return GUCHARMAP_UNICODE_VERSION_UNASSIGNED;
}

const gchar *
gucharmap_unicode_version_to_string (GucharmapUnicodeVersion version)
{
  g_return_val_if_fail (version >= GUCHARMAP_UNICODE_VERSION_UNASSIGNED &&
                        version <= GUCHARMAP_UNICODE_VERSION_LATEST, NULL);

  if (G_UNLIKELY (version == GUCHARMAP_UNICODE_VERSION_UNASSIGNED))
    return NULL;

  return unicode_version_strings + unicode_version_string_offsets[version - 1];
}

gint
gucharmap_get_unihan_count (void)
{
  return G_N_ELEMENTS (unihan);
}

/* does a binary search; also caches most recent, since it will often be
 * called in succession on the same character */
static const Unihan *
_get_unihan (gunichar uc)
{
  static gunichar most_recent_searched;
  static const Unihan *most_recent_result;
  gint min = 0;
  gint mid;
  gint max = G_N_ELEMENTS(unihan) - 1;


  if (uc < unihan[0].index || uc > unihan[max].index)
    return NULL;

  if (uc == most_recent_searched)
    return most_recent_result;

  most_recent_searched = uc;

  while (max >= min) 
    {
      mid = (min + max) / 2;
      if (uc > unihan[mid].index)
        min = mid + 1;
      else if (uc < unihan[mid].index)
        max = mid - 1;
      else
        {
          most_recent_result = unihan + mid;
          return unihan + mid;
        }
    }

  most_recent_result = NULL;
  return NULL;
}

/* does a binary search; also caches most recent, since it will often be
 * called in succession on the same character */
static const NamesList *
get_nameslist (gunichar uc)
{
  static gunichar most_recent_searched;
  static const NamesList *most_recent_result;
  gint min = 0;
  gint mid;
  gint max = G_N_ELEMENTS (names_list) - 1;

  if (uc < names_list[0].index || uc > names_list[max].index)
    return NULL;

  if (uc == most_recent_searched)
    return most_recent_result;

  most_recent_searched = uc;

  while (max >= min) 
    {
      mid = (min + max) / 2;
      if (uc > names_list[mid].index)
        min = mid + 1;
      else if (uc < names_list[mid].index)
        max = mid - 1;
      else
        {
          most_recent_result = names_list + mid;
          return names_list + mid;
        }
    }

  most_recent_result = NULL;
  return NULL;
}

G_GNUC_INTERNAL gboolean
_gucharmap_unicode_has_nameslist_entry (gunichar uc)
{
  return get_nameslist (uc) != NULL;
}

/* returns newly allocated array of gunichar terminated with -1 */
gunichar *
gucharmap_get_nameslist_exes (gunichar uc)
{
  const NamesList *nl;
  gunichar *exes;
  gunichar i, count;
  
  nl = get_nameslist (uc);

  if (nl == NULL || nl->exes_index == -1)
    return NULL;

  /* count the number of exes */
  for (i = 0;  names_list_exes[nl->exes_index + i].index == uc;  i++);
  count = i;

  exes = g_malloc ((count + 1) * sizeof (gunichar));
  for (i = 0;  i < count;  i++)
    exes[i] = names_list_exes[nl->exes_index + i].value;
  exes[count] = (gunichar)(-1);

  return exes;
}

/**
 * gucharmap_get_nameslist_equals:
 * @uc: a gunichar
 *
 * Returns: (transfer container): newly allocated null-terminated array of gchar*
 * the items are const, but the array should be freed by the caller
 */
const gchar **
gucharmap_get_nameslist_equals (gunichar uc)
{
  const NamesList *nl;
  const gchar **equals;
  gunichar i, count;

  nl = get_nameslist (uc);

  if (nl == NULL || nl->equals_index == -1)
    return NULL;

  /* count the number of equals */
  for (i = 0;  names_list_equals[nl->equals_index + i].index == uc;  i++);
  count = i;

  equals = g_malloc ((count + 1) * sizeof (gchar *));
  for (i = 0;  i < count;  i++)
    equals[i] = names_list_equals_strings + names_list_equals[nl->equals_index + i].string_index;
  equals[count] = NULL;

  return equals;
}

/**
 * gucharmap_get_nameslist_stars:
 * @uc: a #gunichar
 *
 * Returns: (transfer container): newly allocated null-terminated array of gchar*
 * the items are const, but the array should be freed by the caller
 */
const gchar **
gucharmap_get_nameslist_stars (gunichar uc)
{
  const NamesList *nl;
  const gchar **stars;
  gunichar i, count;

  nl = get_nameslist (uc);

  if (nl == NULL || nl->stars_index == -1)
    return NULL;

  /* count the number of stars */
  for (i = 0;  names_list_stars[nl->stars_index + i].index == uc;  i++);
  count = i;

  stars = g_malloc ((count + 1) * sizeof (gchar *));
  for (i = 0;  i < count;  i++)
    stars[i] = names_list_stars_strings + names_list_stars[nl->stars_index + i].string_index;
  stars[count] = NULL;

  return stars;
}

/**
 * gucharmap_get_nameslist_pounds:
 * @uc: a #gunichar
 *
 * Returns: (transfer container): newly allocated null-terminated array of gchar*
 * the items are const, but the array should be freed by the caller
 */
const gchar **
gucharmap_get_nameslist_pounds (gunichar uc)
{
  const NamesList *nl;
  const gchar **pounds;
  gunichar i, count;
  
  nl = get_nameslist (uc);

  if (nl == NULL || nl->pounds_index == -1)
    return NULL;

  /* count the number of pounds */
  for (i = 0;  names_list_pounds[nl->pounds_index + i].index == uc;  i++);
  count = i;

  pounds = g_malloc ((count + 1) * sizeof (gchar *));
  for (i = 0;  i < count;  i++)
    pounds[i] = names_list_pounds_strings + names_list_pounds[nl->pounds_index + i].string_index;
  pounds[count] = NULL;

  return pounds;
}

/**
 * gucharmap_get_nameslist_colons:
 * @uc: a #gunichar
 *
 * Returns: (transfer container): newly allocated null-terminated array of gchar*
 * the items are const, but the array should be freed by the caller
 */
const gchar **
gucharmap_get_nameslist_colons (gunichar uc)
{
  const NamesList *nl;
  const gchar **colons;
  gunichar i, count;

  nl = get_nameslist (uc);

  if (nl == NULL || nl->colons_index == -1)
    return NULL;

  /* count the number of colons */
  for (i = 0;  names_list_colons[nl->colons_index + i].index == uc;  i++);
  count = i;

  colons = g_malloc ((count + 1) * sizeof (gchar *));
  for (i = 0;  i < count;  i++)
    colons[i] = names_list_colons_strings + names_list_colons[nl->colons_index + i].string_index;
  colons[count] = NULL;

  return colons;
}

/* Wrapper, in case we want to support a newer unicode version than glib */
gboolean
gucharmap_unichar_validate (gunichar ch)
{
  return g_unichar_validate (ch);
}

/**
 * gucharmap_unichar_to_printable_utf8:
 * @uc: a unicode character 
 * @outbuf: output buffer, must have at least 10 bytes of space.
 *          If %NULL, the length will be computed and returned
 *          and nothing will be written to @outbuf.
 *
 * Converts a single character to UTF-8 suitable for rendering. Check the
 * source to see what this means. ;-)
 * 
 *
 * Return value: number of bytes written
 **/
gint
gucharmap_unichar_to_printable_utf8 (gunichar uc, gchar *outbuf)
{
  /* Unicode Standard 3.2, section 2.6, "By convention, diacritical marks
   * used by the Unicode Standard may be exhibited in (apparent) isolation
   * by applying them to U+0020 SPACE or to U+00A0 NO BREAK SPACE." */

  /* 17:10 < owen> noah: I'm *not* claiming that what Pango does currently
   *               is right, but convention isn't a requirement. I think
   *               it's probably better to do the Uniscribe thing and put
   *               the lone combining mark on a dummy character and require
   *               ZWJ
   * 17:11 < noah> owen: do you mean that i should put a ZWJ in there, or
   *               that pango will do that?
   * 17:11 < owen> noah: I mean, you should (assuming some future more
   *               capable version of Pango) put it in there
   */

  if (! gucharmap_unichar_validate (uc) || (! gucharmap_unichar_isgraph (uc) 
      && gucharmap_unichar_type (uc) != G_UNICODE_PRIVATE_USE))
    return 0;
  else if (gucharmap_unichar_type (uc) == G_UNICODE_COMBINING_MARK
      || gucharmap_unichar_type (uc) == G_UNICODE_ENCLOSING_MARK
      || gucharmap_unichar_type (uc) == G_UNICODE_NON_SPACING_MARK)
    {
      gint x;

      outbuf[0] = ' ';
      outbuf[1] = '\xe2'; /* ZERO */ 
      outbuf[2] = '\x80'; /* WIDTH */
      outbuf[3] = '\x8d'; /* JOINER (0x200D) */

      x = g_unichar_to_utf8 (uc, outbuf + 4);

      return x + 4;
    }
  else
    return g_unichar_to_utf8 (uc, outbuf);
}

/**
 * gucharmap_unichar_type:
 * @uc: a Unicode character
 * 
 * Classifies a Unicode character by type.
 * 
 * Return value: the type of the character.
 **/
GUnicodeType
gucharmap_unichar_type (gunichar uc)
{
  gint min = 0;
  gint mid;
  gint max = sizeof (unicode_categories) / sizeof (UnicodeCategory) - 1;

  if (uc < unicode_categories[0].start || uc > unicode_categories[max].end)
    return G_UNICODE_UNASSIGNED;

  while (max >= min) 
    {
      mid = (min + max) / 2;
      if (uc > unicode_categories[mid].end)
        min = mid + 1;
      else if (uc < unicode_categories[mid].start)
        max = mid - 1;
      else
        return unicode_categories[mid].category;
    }

  return G_UNICODE_UNASSIGNED;
}

/**
 * gucharmap_unichar_isdefined:
 * @uc: a Unicode character
 * 
 * Determines if a given character is assigned in the Unicode
 * standard.
 *
 * Return value: %TRUE if the character has an assigned value
 **/
gboolean
gucharmap_unichar_isdefined (gunichar uc)
{
  return gucharmap_unichar_type (uc) != G_UNICODE_UNASSIGNED;
}

/**
 * gucharmap_unichar_isgraph:
 * @uc: a Unicode character
 * 
 * Determines whether a character is printable and not a space
 * (returns %FALSE for control characters, format characters, and
 * spaces). g_unichar_isprint() is similar, but returns %TRUE for
 * spaces. Given some UTF-8 text, obtain a character value with
 * g_utf8_get_char().
 * 
 * Return value: %TRUE if @c is printable unless it's a space
 **/
gboolean
gucharmap_unichar_isgraph (gunichar uc)
{
  GUnicodeType t = gucharmap_unichar_type (uc);

  /* From http://www.unicode.org/versions/Unicode9.0.0/ch09.pdf, p16
   * "Unlike most other format control characters, however, they should be
   *  rendered with a visible glyph, even in circumstances where no suitable
   *  digit or sequence of digits follows them in logical order."
   * There the standard talks about the ar signs spanning numbers, but
   * I think this should apply to all Prepended_Concatenation_Mark format
   * characters.
   * Instead of parsing the corresponding data file, just hardcode the
   * (few!) existing characters here.
   */
  if (t == G_UNICODE_FORMAT)
    return (uc >= 0x0600 && uc <= 0x0605) || 
	   uc == 0x06DD ||
           uc == 0x070F ||
           uc == 0x08E2 ||
           uc == 0x110BD;

  return (t != G_UNICODE_CONTROL
          && t != G_UNICODE_UNASSIGNED
          && t != G_UNICODE_PRIVATE_USE
          && t != G_UNICODE_SURROGATE
          && t != G_UNICODE_SPACE_SEPARATOR);
}

static gunichar
get_first_non_underscore_char (const char *str)
{
  const char *p;

  if (!str)
    return 0;

  for (p = str; p && *p; p = g_utf8_find_next_char (p, NULL))
    {
      gunichar ch;

      ch = g_utf8_get_char (p);
      if (g_unichar_isalpha (ch))
        return ch;
    }

  return 0;
}

/**
 * gucharmap_unicode_get_locale_character:
 *
 * Determines a character that's commonly used in the current
 * locale's script.
 * 
 * Returns: a unicode character
 */
gunichar
gucharmap_unicode_get_locale_character (void)
{
  GtkStockItem item;
  if (!gtk_stock_lookup (GTK_STOCK_FIND, &item))
    return 0;

  return get_first_non_underscore_char (item.label);
}
