/* $Id$ */
/*
 * Copyright (c) 2002  Noah Levitt <nlevitt@users.sourceforge.net>
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

#include <gtk/gtk.h>
#include <unicode_info.h>

#include <unicode_data.cI>
#include <unicode_blocks.cI>
#if ENABLE_UNIHAN
# include <unicode_unihan.cI>
#endif


/* constants for hangul (de)composition, see UAX #15 */
#define SBase 0xAC00
#define LBase 0x1100
#define VBase 0x1161
#define TBase 0x11A7
#define LCount 19
#define VCount 21
#define TCount 28
#define NCount (VCount * TCount)
#define SCount (LCount * NCount)

static const gchar * const JAMO_L_TABLE[] = {
  "G", "GG", "N", "D", "DD", "R", "M", "B", "BB",
  "S", "SS", "", "J", "JJ", "C", "K", "T", "P", "H", NULL
};

static const gchar * const JAMO_V_TABLE[] = {
  "A", "AE", "YA", "YAE", "EO", "E", "YEO", "YE", "O",
  "WA", "WAE", "OE", "YO", "U", "WEO", "WE", "WI",
  "YU", "EU", "YI", "I"
};

static const gchar * const JAMO_T_TABLE[] = {
  "", "G", "GG", "GS", "N", "NJ", "NH", "D", "L", "LG", "LM",
  "LB", "LS", "LT", "LP", "LH", "M", "B", "BS",
  "S", "SS", "NG", "J", "C", "K", "T", "P", "H"
};

/* computes the hangul name as per UAX #15 */
const gchar *
get_hangul_syllable_name (gunichar s)
{
  static gchar buf[32];
  gint SIndex = s - SBase;
  gint LIndex, VIndex, TIndex;

  if (SIndex < 0 || SIndex >= SCount)
    return "";

  LIndex = SIndex / NCount;
  VIndex = (SIndex % NCount) / TCount;
  TIndex = SIndex % TCount;

  g_snprintf (buf, 32, "HANGUL SYLLABLE %s%s%s", JAMO_L_TABLE[LIndex],
              JAMO_V_TABLE[VIndex], JAMO_T_TABLE[TIndex]);

  return buf;
}


const gchar *
get_unicode_name (gunichar uc)
{
  if (uc >= 0x3400 && uc <= 0x4DB5)
    return "<CJK Ideograph Extension A>";
  else if (uc >= 0x4e00 && uc <= 0x9fa5)
    return "<CJK Ideograph>";
  else if (uc >= 0xac00 && uc <= 0xd7af)
    return get_hangul_syllable_name (uc);
  else if (uc >= 0xD800 && uc <= 0xDB7F) 
    return "<Non Private Use High Surrogate>";
  else if (uc >= 0xDB80 && uc <= 0xDBFF) 
    return "<Private Use High Surrogate>";
  else if (uc >= 0xDC00 && uc <= 0xDFFF)
    return "<Low Surrogate, Last>";
  else if (uc >= 0xE000 && uc <= 0xF8FF) 
    return "<Private Use>";
  else if (uc >= 0xF0000 && uc <= 0xFFFFD)
    return "<Plane 15 Private Use>";
  else if (uc >= 0x100000 && uc <= 0x10FFFD)
    return "<Plane 16 Private Use>";
  else if (uc >= 0x20000 && uc <= 0x2A6D6)
    return "<CJK Ideograph Extension B>";
  else
    {
      const gchar *x = get_unicode_data_name (uc);
      if (x == NULL)
        return "<not assigned>";
      else
        return x;
    }
}


const gchar *
get_unicode_category_name (gunichar uc)
{
  switch (g_unichar_type (uc))
    {
      case G_UNICODE_CONTROL: return "Other, Control";
      case G_UNICODE_FORMAT: return "Other, Format";
      case G_UNICODE_UNASSIGNED: return "Other, Not Assigned";
      case G_UNICODE_PRIVATE_USE: return "Other, Private Use";
      case G_UNICODE_SURROGATE: return "Other, Surrogate";
      case G_UNICODE_LOWERCASE_LETTER: return "Letter, Lowercase";
      case G_UNICODE_MODIFIER_LETTER: return "Letter, Modifier";
      case G_UNICODE_OTHER_LETTER: return "Letter, Other";
      case G_UNICODE_TITLECASE_LETTER: return "Letter, Titlecase";
      case G_UNICODE_UPPERCASE_LETTER: return "Letter, Uppercase";
      case G_UNICODE_COMBINING_MARK: return "Mark, Spacing Combining";
      case G_UNICODE_ENCLOSING_MARK: return "Mark, Enclosing";
      case G_UNICODE_NON_SPACING_MARK: return "Mark, Non-Spacing";
      case G_UNICODE_DECIMAL_NUMBER: return "Number, Decimal Digit";
      case G_UNICODE_LETTER_NUMBER: return "Number, Letter";
      case G_UNICODE_OTHER_NUMBER: return "Number, Other";
      case G_UNICODE_CONNECT_PUNCTUATION: return "Punctuation, Connector";
      case G_UNICODE_DASH_PUNCTUATION: return "Punctuation, Dash";
      case G_UNICODE_CLOSE_PUNCTUATION: return "Punctuation, Close";
      case G_UNICODE_FINAL_PUNCTUATION: return "Punctuation, Final quote ";
      case G_UNICODE_INITIAL_PUNCTUATION: return "Punctuation, Initial quote";
      case G_UNICODE_OTHER_PUNCTUATION: return "Punctuation, Other";
      case G_UNICODE_OPEN_PUNCTUATION: return "Punctuation, Open";
      case G_UNICODE_CURRENCY_SYMBOL: return "Symbol, Currency";
      case G_UNICODE_MODIFIER_SYMBOL: return "Symbol, Modifier";
      case G_UNICODE_MATH_SYMBOL: return "Symbol, Math";
      case G_UNICODE_OTHER_SYMBOL: return "Symbol, Other";
      case G_UNICODE_LINE_SEPARATOR: return "Separator, Line";
      case G_UNICODE_PARAGRAPH_SEPARATOR: return "Separator, Paragraph";
      case G_UNICODE_SPACE_SEPARATOR: return "Separator, Space";
      default: return "";
    }
}


/* counts the number of entries in unicode_blocks with start <= max */
gint 
count_blocks (gunichar max)
{
  gint i;

  for (i = 0;  unicode_blocks[i].start != (gunichar)(-1)
               && unicode_blocks[i].start < max;  i++);

  return i;
}


/* http://www.unicode.org/unicode/reports/tr15/#Hangul */
static gunichar *
hangul_decomposition (gunichar s, gsize *result_len)
{
  gunichar *r = g_malloc (3 * sizeof (gunichar));
  gint SIndex = s - SBase;

  /* not a hangul syllable */
  if (SIndex < 0 || SIndex >= SCount)
    {
      r[0] = s;
      *result_len = 1;
    }
  else
    {
      gunichar L = LBase + SIndex / NCount;
      gunichar V = VBase + (SIndex % NCount) / TCount;
      gunichar T = TBase + SIndex % TCount;

      r[0] = L;
      r[1] = V;

      if (T != TBase) 
        {
          r[2] = T;
          *result_len = 3;
        }
      else
        *result_len = 2;
    }

  return r;
}


/*
 * See http://bugzilla.gnome.org/show_bug.cgi?id=100456
 *
 * unicode_canonical_decomposition:
 * @ch: a Unicode character.
 * @result_len: location to store the length of the return value.
 *
 * Computes the canonical decomposition of a Unicode character.  
 * 
 * Return value: a newly allocated string of Unicode characters.
 *   @result_len is set to the resulting length of the string.
 */
gunichar *
unicode_canonical_decomposition (gunichar ch, gsize   *result_len)
{
  if (ch >= 0xac00 && ch <= 0xd7af)  /* Hangul syllable */
    return hangul_decomposition (ch, result_len);
  else 
    return g_unicode_canonical_decomposition (ch, result_len);
}



/* does a binary search on unicode_data */
const gchar *
get_unicode_data_name (gunichar uc)
{
  gint min = 0;
  gint mid;
  gint max = sizeof (unicode_data) / sizeof (UnicodeData) - 1;

  if (uc < unicode_data[0].index || uc > unicode_data[max].index)
    return "";

  while (max >= min) 
    {
      mid = (min + max) / 2;
      if (uc > unicode_data[mid].index)
        min = mid + 1;
      else if (uc < unicode_data[mid].index)
        max = mid - 1;
      else
        return unicode_data[mid].name;
    }

  return NULL;
}


/* ascii case-insensitive substring search (source ripped from glib) */
static const gchar *
ascii_case_strrstr (const gchar *haystack, const gchar *needle)
{
  gsize i;
  gsize needle_len;
  gsize haystack_len;
  const gchar *p;
      
  g_return_val_if_fail (haystack != NULL, NULL);
  g_return_val_if_fail (needle != NULL, NULL);

  needle_len = strlen (needle);
  haystack_len = strlen (haystack);

  if (needle_len == 0)
    return haystack;

  if (haystack_len < needle_len)
    return NULL;
  
  p = haystack + haystack_len - needle_len;

  while (p >= haystack)
    {
      for (i = 0; i < needle_len; i++)
        if (g_ascii_tolower (p[i]) != g_ascii_tolower (needle[i]))
          goto next;
      
      return p;
      
    next:
      p--;
    }
  
  return NULL;
}


/* case insensitive; returns (gunichar)(-1) if nothing found */
gunichar
find_next_substring_match (gunichar start, gunichar unichar_max,
                           const gchar *search_text)
{
  gint min = 0;
  gint mid = 0;
  gint max = sizeof (unicode_data) / sizeof (UnicodeData) - 1;
  gint i0;
  gint i;

  /* locate the start character by binary search */
  if (start < unicode_data[0].index || start > unichar_max)
    i0 = 0;
  else
    {
      while (max >= min) 
        {
          mid = (min + max) / 2;
          if (start > unicode_data[mid].index)
            min = mid + 1;
          else if (start < unicode_data[mid].index)
            max = mid - 1;
          else
            break;
        }

      i0 = mid;
    }

  /* try substring match on each */
  max = sizeof (unicode_data) / sizeof (UnicodeData);
  for (i = i0+1;  i != i0;  )
    {
      if (ascii_case_strrstr (unicode_data[i].name, search_text) != NULL)
        return unicode_data[i].index;

      i++;
      if (i >= max || unicode_data[i].index > unichar_max)
        i = 0;
    }

  /* if the start character matches we want to return a match */
  if (ascii_case_strrstr (unicode_data[i].name, search_text) != NULL)
    return unicode_data[i].index;

  return (gunichar)(-1);
}


#if ENABLE_UNIHAN

/* does a binary search; also caches most recent, since it will often be
 * called in succession on the same character */
static const Unihan *
_get_unihan (gunichar uc)
{
  static gunichar most_recent_searched;
  static const Unihan *most_recent_result;
  gint min = 0;
  gint mid;
  gint max = sizeof (unihan) / sizeof (Unihan) - 1;

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


const gchar * 
get_unicode_kDefinition (gunichar uc)
{
  const Unihan *uh = _get_unihan (uc);
  if (uh == NULL)
    return "";
  else
    return uh->kDefinition;
}

const gchar * 
get_unicode_kCantonese (gunichar uc)
{
  const Unihan *uh = _get_unihan (uc);
  if (uh == NULL)
    return "";
  else
    return uh->kCantonese;
}

const gchar * 
get_unicode_kMandarin (gunichar uc)
{
  const Unihan *uh = _get_unihan (uc);
  if (uh == NULL)
    return "";
  else
    return uh->kMandarin;
}

const gchar * 
get_unicode_kTang (gunichar uc)
{
  const Unihan *uh = _get_unihan (uc);
  if (uh == NULL)
    return "";
  else
    return uh->kTang;
}

const gchar * 
get_unicode_kKorean (gunichar uc)
{
  const Unihan *uh = _get_unihan (uc);
  if (uh == NULL)
    return "";
  else
    return uh->kKorean;
}

const gchar * 
get_unicode_kJapaneseKun (gunichar uc)
{
  const Unihan *uh = _get_unihan (uc);
  if (uh == NULL)
    return "";
  else
    return uh->kJapeneseKun;
}

const gchar * 
get_unicode_kJapaneseOn (gunichar uc)
{
  const Unihan *uh = _get_unihan (uc);
  if (uh == NULL)
    return "";
  else
    return uh->kJapaneseOn;
}

#else /* #if ENABLE_UNIHAN */

const gchar * 
get_unicode_kDefinition (gunichar uc)
{
  return "This feature was not compiled in.";
}

const gchar * 
get_unicode_kCantonese (gunichar uc)
{
  return "This feature was not compiled in.";
}

const gchar * 
get_unicode_kMandarin (gunichar uc)
{
  return "This feature was not compiled in.";
}

const gchar * 
get_unicode_kTang (gunichar uc)
{
  return "This feature was not compiled in.";
}

const gchar * 
get_unicode_kKorean (gunichar uc)
{
  return "This feature was not compiled in.";
}

const gchar * 
get_unicode_kJapaneseKun (gunichar uc)
{
  return "This feature was not compiled in.";
}

const gchar * 
get_unicode_kJapaneseOn (gunichar uc)
{
  return "This feature was not compiled in.";
}

#endif /* #else (#if ENABLE_UNIHAN) */

