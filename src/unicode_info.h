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


#ifndef UNICODE_INFO_H
#define UNICODE_INFO_H

#include <gtk/gtk.h>


G_BEGIN_DECLS


#ifndef UNICHAR_MAX
# define UNICHAR_MAX 0x0000ffff
#endif


typedef struct
{
  gunichar start;
  gunichar end;
  const gchar *name;
}
UnicodeBlock;


typedef struct 
{
  gunichar index;
  const gchar *name;
} 
UnicodeData;


typedef struct 
{
  gunichar index;
  const gchar *kDefinition;
  const gchar *kCantonese;
  const gchar *kMandarin;
  const gchar *kTang;
  const gchar *kKorean;
  const gchar *kJapeneseKun;
  const gchar *kJapaneseOn;
} 
Unihan;


extern const UnicodeBlock unicode_blocks[];

gint count_blocks (gunichar max);


/* return values are read-only */
G_CONST_RETURN gchar * get_unicode_name (gunichar uc);
G_CONST_RETURN gchar * get_unicode_data_name (gunichar uc);
G_CONST_RETURN gchar * get_unicode_category_name (gunichar uc);
G_CONST_RETURN gchar * get_unicode_kDefinition (gunichar uc);
G_CONST_RETURN gchar * get_unicode_kCantonese (gunichar uc);
G_CONST_RETURN gchar * get_unicode_kMandarin (gunichar uc);
G_CONST_RETURN gchar * get_unicode_kTang (gunichar uc);
G_CONST_RETURN gchar * get_unicode_kKorean (gunichar uc);
G_CONST_RETURN gchar * get_unicode_kJapaneseKun (gunichar uc);
G_CONST_RETURN gchar * get_unicode_kJapaneseOn (gunichar uc);
G_CONST_RETURN gchar * get_hangul_syllable_name (gunichar s);

/* A wrapper for g_unicode_canonical_decomposition that also does hangul
 * decomposition. 
 * See http://bugzilla.gnome.org/show_bug.cgi?id=100456
 * Will no longer be necessary once my patch is accepted.
 */
gunichar * unicode_canonical_decomposition (gunichar ch, gsize *result_len);

/* starts search at start */
gunichar find_next_substring_match (gunichar start, gunichar unichar_max,
                                    const gchar *search_text);


G_END_DECLS

#endif  /* #ifndef UNICODE_INFO_H */

