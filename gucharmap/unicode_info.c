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

#include <gtk/gtk.h>
#include "unicode_info.h"

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


const unicode_block_t unicode_blocks[] =
{
  { 0x0000, 0x007F, "Basic Latin" },
  { 0x0080, 0x00FF, "Latin-1 Supplement" },
  { 0x0100, 0x017F, "Latin Extended-A" },
  { 0x0180, 0x024F, "Latin Extended-B" },
  { 0x0250, 0x02AF, "IPA Extensions" },
  { 0x02B0, 0x02FF, "Spacing Modifier Letters" },
  { 0x0300, 0x036F, "Combining Diacritical Marks" },
  { 0x0370, 0x03FF, "Greek and Coptic" },
  { 0x0400, 0x04FF, "Cyrillic" },
  { 0x0500, 0x052F, "Cyrillic Supplementary" },
  { 0x0530, 0x058F, "Armenian" },
  { 0x0590, 0x05FF, "Hebrew" },
  { 0x0600, 0x06FF, "Arabic" },
  { 0x0700, 0x074F, "Syriac" },
  { 0x0780, 0x07BF, "Thaana" },
  { 0x0900, 0x097F, "Devanagari" },
  { 0x0980, 0x09FF, "Bengali" },
  { 0x0A00, 0x0A7F, "Gurmukhi" },
  { 0x0A80, 0x0AFF, "Gujarati" },
  { 0x0B00, 0x0B7F, "Oriya" },
  { 0x0B80, 0x0BFF, "Tamil" },
  { 0x0C00, 0x0C7F, "Telugu" },
  { 0x0C80, 0x0CFF, "Kannada" },
  { 0x0D00, 0x0D7F, "Malayalam" },
  { 0x0D80, 0x0DFF, "Sinhala" },
  { 0x0E00, 0x0E7F, "Thai" },
  { 0x0E80, 0x0EFF, "Lao" },
  { 0x0F00, 0x0FFF, "Tibetan" },
  { 0x1000, 0x109F, "Myanmar" },
  { 0x10A0, 0x10FF, "Georgian" },
  { 0x1100, 0x11FF, "Hangul Jamo" },
  { 0x1200, 0x137F, "Ethiopic" },
  { 0x13A0, 0x13FF, "Cherokee" },
  { 0x1400, 0x167F, "Unified Canadian Aboriginal Syllabics" },
  { 0x1680, 0x169F, "Ogham" },
  { 0x16A0, 0x16FF, "Runic" },
  { 0x1700, 0x171F, "Tagalog" },
  { 0x1720, 0x173F, "Hanunoo" },
  { 0x1740, 0x175F, "Buhid" },
  { 0x1760, 0x177F, "Tagbanwa" },
  { 0x1780, 0x17FF, "Khmer" },
  { 0x1800, 0x18AF, "Mongolian" },
  { 0x1E00, 0x1EFF, "Latin Extended Additional" },
  { 0x1F00, 0x1FFF, "Greek Extended" },
  { 0x2000, 0x206F, "General Punctuation" },
  { 0x2070, 0x209F, "Superscripts and Subscripts" },
  { 0x20A0, 0x20CF, "Currency Symbols" },
  { 0x20D0, 0x20FF, "Combining Diacritical Marks for Symbols" },
  { 0x2100, 0x214F, "Letterlike Symbols" },
  { 0x2150, 0x218F, "Number Forms" },
  { 0x2190, 0x21FF, "Arrows" },
  { 0x2200, 0x22FF, "Mathematical Operators" },
  { 0x2300, 0x23FF, "Miscellaneous Technical" },
  { 0x2400, 0x243F, "Control Pictures" },
  { 0x2440, 0x245F, "Optical Character Recognition" },
  { 0x2460, 0x24FF, "Enclosed Alphanumerics" },
  { 0x2500, 0x257F, "Box Drawing" },
  { 0x2580, 0x259F, "Block Elements" },
  { 0x25A0, 0x25FF, "Geometric Shapes" },
  { 0x2600, 0x26FF, "Miscellaneous Symbols" },
  { 0x2700, 0x27BF, "Dingbats" },
  { 0x27C0, 0x27EF, "Miscellaneous Mathematical Symbols-A" },
  { 0x27F0, 0x27FF, "Supplemental Arrows-A" },
  { 0x2800, 0x28FF, "Braille Patterns" },
  { 0x2900, 0x297F, "Supplemental Arrows-B" },
  { 0x2980, 0x29FF, "Miscellaneous Mathematical Symbols-B" },
  { 0x2A00, 0x2AFF, "Supplemental Mathematical Operators" },
  { 0x2E80, 0x2EFF, "CJK Radicals Supplement" },
  { 0x2F00, 0x2FDF, "Kangxi Radicals" },
  { 0x2FF0, 0x2FFF, "Ideographic Description Characters" },
  { 0x3000, 0x303F, "CJK Symbols and Punctuation" },
  { 0x3040, 0x309F, "Hiragana" },
  { 0x30A0, 0x30FF, "Katakana" },
  { 0x3100, 0x312F, "Bopomofo" },
  { 0x3130, 0x318F, "Hangul Compatibility Jamo" },
  { 0x3190, 0x319F, "Kanbun" },
  { 0x31A0, 0x31BF, "Bopomofo Extended" },
  { 0x31F0, 0x31FF, "Katakana Phonetic Extensions" },
  { 0x3200, 0x32FF, "Enclosed CJK Letters and Months" },
  { 0x3300, 0x33FF, "CJK Compatibility" },
  { 0x3400, 0x4DBF, "CJK Unified Ideographs Extension A" },
  { 0x4E00, 0x9FFF, "CJK Unified Ideographs" },
  { 0xA000, 0xA48F, "Yi Syllables" },
  { 0xA490, 0xA4CF, "Yi Radicals" },
  { 0xAC00, 0xD7AF, "Hangul Syllables" },
  { 0xD800, 0xDB7F, "High Surrogates" },
  { 0xDB80, 0xDBFF, "High Private Use Surrogates" },
  { 0xDC00, 0xDFFF, "Low Surrogates" },
  { 0xE000, 0xF8FF, "Private Use Area" },
  { 0xF900, 0xFAFF, "CJK Compatibility Ideographs" },
  { 0xFB00, 0xFB4F, "Alphabetic Presentation Forms" },
  { 0xFB50, 0xFDFF, "Arabic Presentation Forms-A" },
  { 0xFE00, 0xFE0F, "Variation Selectors" },
  { 0xFE20, 0xFE2F, "Combining Half Marks" },
  { 0xFE30, 0xFE4F, "CJK Compatibility Forms" },
  { 0xFE50, 0xFE6F, "Small Form Variants" },
  { 0xFE70, 0xFEFF, "Arabic Presentation Forms-B" },
  { 0xFF00, 0xFFEF, "Halfwidth and Fullwidth Forms" },
  { 0xFFF0, 0xFFFF, "Specials" },
  { 0x10300, 0x1032F, "Old Italic" },
  { 0x10330, 0x1034F, "Gothic" },
  { 0x10400, 0x1044F, "Deseret" },
  { 0x1D000, 0x1D0FF, "Byzantine Musical Symbols" },
  { 0x1D100, 0x1D1FF, "Musical Symbols" },
  { 0x1D400, 0x1D7FF, "Mathematical Alphanumeric Symbols" },
  { 0x20000, 0x2A6DF, "CJK Unified Ideographs Extension B" },
  { 0x2F800, 0x2FA1F, "CJK Compatibility Ideographs Supplement" },
  { 0xE0000, 0xE007F, "Tags" },
  { 0xF0000, 0xFFFFF, "Supplementary Private Use Area-A" },
  { 0x100000, 0x10FFFF, "Supplementary Private Use Area-B" },
  { (gunichar)-1, (gunichar)-1, NULL }
};


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


gboolean
is_valid_character (gunichar uc)
{
  return (uc >= 0 && uc <= 0x10ffff && uc != 0xfffe && uc != 0xffff 
          && (uc < 0xd800 || uc > 0xdfff));
}
