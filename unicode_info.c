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

static gchar *JAMO_L_TABLE[] = {
  "G", "GG", "N", "D", "DD", "R", "M", "B", "BB",
  "S", "SS", "", "J", "JJ", "C", "K", "T", "P", "H", NULL
};

static gchar *JAMO_V_TABLE[] = {
  "A", "AE", "YA", "YAE", "EO", "E", "YEO", "YE", "O",
  "WA", "WAE", "OE", "YO", "U", "WEO", "WE", "WI",
  "YU", "EU", "YI", "I"
};

static gchar *JAMO_T_TABLE[] = {
  "", "G", "GG", "GS", "N", "NJ", "NH", "D", "L", "LG", "LM",
  "LB", "LS", "LT", "LP", "LH", "M", "B", "BS",
  "S", "SS", "NG", "J", "C", "K", "T", "P", "H"
};

/* computes the hangul name as per UAX #15 */
gchar *
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


gchar *
get_unicode_name (gunichar uc)
{
  if (uc >= 0xac00 && uc <= 0xd7af)
    return get_hangul_syllable_name (uc);
  else
    return get_unicode_data_name (uc);
}


gchar *
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
