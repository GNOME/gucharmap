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


#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

/* return values are read-only */
gchar * get_unicode_name (gunichar uc);
gchar * get_unicode_category_name (gunichar uc);
gchar * get_unicode_kDefinition (gunichar uc);
gchar * get_unicode_kCantonese (gunichar uc);
gchar * get_unicode_kMandarin (gunichar uc);
gchar * get_unicode_kTang (gunichar uc);
gchar * get_unicode_kKorean (gunichar uc);
gchar * get_unicode_kJapaneseKun (gunichar uc);
gchar * get_unicode_kJapaneseOn (gunichar uc);

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif  /* #ifndef UNICODE_INFO_H */

