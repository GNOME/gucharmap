/* $Id$ */
/*
 * Copyright (c) 2003 Noah Levitt
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


#ifndef GUCHARMAP_UNICODE_INFO_H
#define GUCHARMAP_UNICODE_INFO_H

#include <glib.h>


G_BEGIN_DECLS


typedef struct
{
  gunichar start;
  gunichar end;
  const gchar *name;
}
GucharmapUnicodeBlock;

extern const GucharmapUnicodeBlock gucharmap_unicode_blocks[];

gint gucharmap_count_blocks (gunichar max);


/* return values are read-only */
G_CONST_RETURN gchar * gucharmap_get_unicode_name (gunichar uc);
G_CONST_RETURN gchar * gucharmap_get_unicode_data_name (gunichar uc);
G_CONST_RETURN gchar * gucharmap_get_unicode_category_name (gunichar uc);
G_CONST_RETURN gchar * gucharmap_get_unicode_kDefinition (gunichar uc);
G_CONST_RETURN gchar * gucharmap_get_unicode_kCantonese (gunichar uc);
G_CONST_RETURN gchar * gucharmap_get_unicode_kMandarin (gunichar uc);
G_CONST_RETURN gchar * gucharmap_get_unicode_kTang (gunichar uc);
G_CONST_RETURN gchar * gucharmap_get_unicode_kKorean (gunichar uc);
G_CONST_RETURN gchar * gucharmap_get_unicode_kJapaneseKun (gunichar uc);
G_CONST_RETURN gchar * gucharmap_get_unicode_kJapaneseOn (gunichar uc);
G_CONST_RETURN gchar * gucharmap_get_hangul_syllable_name (gunichar s);

/* A wrapper for g_unicode_canonical_decomposition that also does hangul
 * decomposition. 
 * See http://bugzilla.gnome.org/show_bug.cgi?id=100456
 * Will no longer be necessary once my patch is accepted.
 */
gunichar * gucharmap_unicode_canonical_decomposition (gunichar ch, 
                                                      gsize *result_len);

/* starts search at start */
gunichar gucharmap_find_substring_match (gunichar start, 
                                         const gchar *search_text, 
                                         gint direction);

/* nameslist stuff */
G_CONST_RETURN gchar ** gucharmap_get_nameslist_stars       (gunichar  wc);
G_CONST_RETURN gchar ** gucharmap_get_nameslist_equals      (gunichar  wc);
gunichar *              gucharmap_get_nameslist_exes        (gunichar  wc);
G_CONST_RETURN gchar ** gucharmap_get_nameslist_pounds      (gunichar  wc);
G_CONST_RETURN gchar ** gucharmap_get_nameslist_colons      (gunichar  wc);
gboolean                gucharmap_unichar_validate          (gunichar  wc);
gint                    gucharmap_unichar_to_printable_utf8 (gunichar  wc, 
                                                             gchar    *outbuf);
GUnicodeType            gucharmap_unichar_type              (gunichar  wc);
gboolean                gucharmap_unichar_isdefined         (gunichar  wc);
gboolean                gucharmap_unichar_isgraph           (gunichar  wc);

/* XXX: defined in gucharmap-script-codepoint-list.c */
G_CONST_RETURN gchar **  gucharmap_unicode_list_scripts     ();

G_END_DECLS

#endif  /* #ifndef GUCHARMAP_UNICODE_INFO_H */

