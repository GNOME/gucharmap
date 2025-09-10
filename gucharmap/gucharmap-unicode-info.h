/*
 * Copyright © 2004 Noah Levitt
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

#if !defined (__GUCHARMAP_GUCHARMAP_H_INSIDE__) && !defined (GUCHARMAP_COMPILATION)
#error "Only <gucharmap/gucharmap.h> can be included directly."
#endif

#ifndef GUCHARMAP_UNICODE_INFO_H
#define GUCHARMAP_UNICODE_INFO_H

#include <glib.h>

#include <gucharmap/gucharmap-macros.h>

G_BEGIN_DECLS

/**
 * GucharmapUnicodeVersion:
 * @GUCHARMAP_UNICODE_VERSION_UNASSIGNED: Unassigned Unicode version
 * @GUCHARMAP_UNICODE_VERSION_1_1: Unicode version 1.1
 * @GUCHARMAP_UNICODE_VERSION_2_0: Unicode version 2.0
 * @GUCHARMAP_UNICODE_VERSION_2_1: Unicode version 2.1
 * @GUCHARMAP_UNICODE_VERSION_3_0: Unicode version 3.0
 * @GUCHARMAP_UNICODE_VERSION_3_1: Unicode version 3.1
 * @GUCHARMAP_UNICODE_VERSION_3_2: Unicode version 3.2
 * @GUCHARMAP_UNICODE_VERSION_4_0: Unicode version 4.0
 * @GUCHARMAP_UNICODE_VERSION_4_1: Unicode version 4.1
 * @GUCHARMAP_UNICODE_VERSION_5_0: Unicode version 5.0
 * @GUCHARMAP_UNICODE_VERSION_5_1: Unicode version 5.1
 * @GUCHARMAP_UNICODE_VERSION_5_2: Unicode version 5.2
 * @GUCHARMAP_UNICODE_VERSION_6_0: Unicode version 6.0
 * @GUCHARMAP_UNICODE_VERSION_6_1: Unicode version 6.1
 * @GUCHARMAP_UNICODE_VERSION_6_2: Unicode version 6.2
 * @GUCHARMAP_UNICODE_VERSION_6_3: Unicode version 6.3
 * @GUCHARMAP_UNICODE_VERSION_7_0: Unicode version 7.0
 * @GUCHARMAP_UNICODE_VERSION_8_0: Unicode version 8.0
 * @GUCHARMAP_UNICODE_VERSION_9_0: Unicode version 9.0
 * @GUCHARMAP_UNICODE_VERSION_10_0: Unicode version 10.0
 * @GUCHARMAP_UNICODE_VERSION_11_0: Unicode version 11.0
 * @GUCHARMAP_UNICODE_VERSION_12_0: Unicode version 12.0
 * @GUCHARMAP_UNICODE_VERSION_12_1: Unicode version 12.1
 * @GUCHARMAP_UNICODE_VERSION_13_0: Unicode version 13.0
 * @GUCHARMAP_UNICODE_VERSION_14_0: Unicode version 14.0
 * @GUCHARMAP_UNICODE_VERSION_15_0: Unicode version 15.0
 * @GUCHARMAP_UNICODE_VERSION_15_1: Unicode version 15.1
 * @GUCHARMAP_UNICODE_VERSION_16_0: Unicode version 16.0
 * @GUCHARMAP_UNICODE_VERSION_17_0: Unicode version 17.0
 * @GUCHARMAP_UNICODE_VERSION_LATEST: Latest Unicode version
 */
typedef enum {
  GUCHARMAP_UNICODE_VERSION_UNASSIGNED,
  GUCHARMAP_UNICODE_VERSION_1_1,
  GUCHARMAP_UNICODE_VERSION_2_0,
  GUCHARMAP_UNICODE_VERSION_2_1,
  GUCHARMAP_UNICODE_VERSION_3_0,
  GUCHARMAP_UNICODE_VERSION_3_1,
  GUCHARMAP_UNICODE_VERSION_3_2,
  GUCHARMAP_UNICODE_VERSION_4_0,
  GUCHARMAP_UNICODE_VERSION_4_1,
  GUCHARMAP_UNICODE_VERSION_5_0,
  GUCHARMAP_UNICODE_VERSION_5_1,
  GUCHARMAP_UNICODE_VERSION_5_2,
  GUCHARMAP_UNICODE_VERSION_6_0,
  GUCHARMAP_UNICODE_VERSION_6_1,
  GUCHARMAP_UNICODE_VERSION_6_2,
  GUCHARMAP_UNICODE_VERSION_6_3,
  GUCHARMAP_UNICODE_VERSION_7_0,
  GUCHARMAP_UNICODE_VERSION_8_0,
  GUCHARMAP_UNICODE_VERSION_9_0,
  GUCHARMAP_UNICODE_VERSION_10_0,
  GUCHARMAP_UNICODE_VERSION_11_0,
  GUCHARMAP_UNICODE_VERSION_12_0,
  GUCHARMAP_UNICODE_VERSION_12_1,
  GUCHARMAP_UNICODE_VERSION_13_0,
  GUCHARMAP_UNICODE_VERSION_14_0,
  GUCHARMAP_UNICODE_VERSION_15_0,
  GUCHARMAP_UNICODE_VERSION_15_1,
  GUCHARMAP_UNICODE_VERSION_16_0,
  GUCHARMAP_UNICODE_VERSION_17_0,
  GUCHARMAP_UNICODE_VERSION_LATEST = GUCHARMAP_UNICODE_VERSION_17_0 /* private, will move forward with each revision */
} GucharmapUnicodeVersion;

/* return values are read-only */
_GUCHARMAP_PUBLIC
const gchar *  gucharmap_get_unicode_name                (gunichar uc);
_GUCHARMAP_PUBLIC
const gchar *  gucharmap_get_unicode_data_name           (gunichar uc);
_GUCHARMAP_PUBLIC
gint                    gucharmap_get_unicode_data_name_count     (void);
_GUCHARMAP_PUBLIC
GucharmapUnicodeVersion gucharmap_get_unicode_version             (gunichar uc);
_GUCHARMAP_PUBLIC
const gchar *  gucharmap_get_unicode_category_name       (gunichar uc);
_GUCHARMAP_PUBLIC
gint                    gucharmap_get_unihan_count                (void);
_GUCHARMAP_PUBLIC
const gchar *  gucharmap_get_unicode_kDefinition         (gunichar uc);
_GUCHARMAP_PUBLIC
const gchar *  gucharmap_get_unicode_kCantonese          (gunichar uc);
_GUCHARMAP_PUBLIC
const gchar *  gucharmap_get_unicode_kMandarin           (gunichar uc);
_GUCHARMAP_PUBLIC
const gchar *  gucharmap_get_unicode_kTang               (gunichar uc);
_GUCHARMAP_PUBLIC
const gchar *  gucharmap_get_unicode_kKorean             (gunichar uc);
_GUCHARMAP_PUBLIC
const gchar *  gucharmap_get_unicode_kJapaneseKun        (gunichar uc);
_GUCHARMAP_PUBLIC
const gchar *  gucharmap_get_unicode_kJapaneseOn         (gunichar uc);
_GUCHARMAP_PUBLIC
const gchar *  gucharmap_get_unicode_kHangul             (gunichar uc);
_GUCHARMAP_PUBLIC
const gchar *  gucharmap_get_unicode_kVietnamese         (gunichar uc);

/* nameslist stuff */
_GUCHARMAP_PUBLIC
const gchar ** gucharmap_get_nameslist_stars             (gunichar  uc);
_GUCHARMAP_PUBLIC
const gchar ** gucharmap_get_nameslist_equals            (gunichar  uc);
_GUCHARMAP_PUBLIC
gunichar *              gucharmap_get_nameslist_exes              (gunichar  uc);
_GUCHARMAP_PUBLIC
const gchar ** gucharmap_get_nameslist_pounds            (gunichar  uc);
_GUCHARMAP_PUBLIC
const gchar ** gucharmap_get_nameslist_colons            (gunichar  uc);
_GUCHARMAP_PUBLIC
gboolean                gucharmap_unichar_validate                (gunichar  uc);
_GUCHARMAP_PUBLIC
gint                    gucharmap_unichar_to_printable_utf8       (gunichar  uc,
                                                                   gchar    *outbuf);
_GUCHARMAP_PUBLIC
GUnicodeType            gucharmap_unichar_type                    (gunichar  uc);
_GUCHARMAP_PUBLIC
gboolean                gucharmap_unichar_isdefined               (gunichar  uc);
_GUCHARMAP_PUBLIC
gboolean                gucharmap_unichar_isgraph                 (gunichar  uc);

/* defined in gucharmap-script-codepoint-list.c */
_GUCHARMAP_PUBLIC
const gchar ** gucharmap_unicode_list_scripts            (void);
_GUCHARMAP_PUBLIC
const gchar *  gucharmap_unicode_get_script_for_char     (gunichar wc);

_GUCHARMAP_PUBLIC
const gchar *  gucharmap_unicode_version_to_string       (GucharmapUnicodeVersion version);

/* doesn't really belong here, but no better place was available */
_GUCHARMAP_PUBLIC
gunichar     gucharmap_unicode_get_locale_character (void);

G_END_DECLS

#endif  /* #ifndef GUCHARMAP_UNICODE_INFO_H */
