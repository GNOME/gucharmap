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

typedef enum {
  UNICODE_CATEGORY_Lu = 101, /* Letter, Uppercase */
  UNICODE_CATEGORY_Ll, /* Letter, Lowercase */
  UNICODE_CATEGORY_Lt, /* Letter, Titlecase */
  UNICODE_CATEGORY_Lm, /* Letter, Modifier */
  UNICODE_CATEGORY_Lo, /* Letter, Other */
  UNICODE_CATEGORY_Mn, /* Mark, Non-Spacing */
  UNICODE_CATEGORY_Mc, /* Mark, Spacing Combining */
  UNICODE_CATEGORY_Me, /* Mark, Enclosing */
  UNICODE_CATEGORY_Nd, /* Number, Decimal Digit */
  UNICODE_CATEGORY_Nl, /* Number, Letter */
  UNICODE_CATEGORY_No, /* Number, Other */
  UNICODE_CATEGORY_Pc, /* Punctuation, Connector */
  UNICODE_CATEGORY_Pd, /* Punctuation, Dash */
  UNICODE_CATEGORY_Ps, /* Punctuation, Open */
  UNICODE_CATEGORY_Pe, /* Punctuation, Close */
  UNICODE_CATEGORY_Pi, /* Punctuation, Initial quote */
  UNICODE_CATEGORY_Pf, /* Punctuation, Final quote  */
  UNICODE_CATEGORY_Po, /* Punctuation, Other */
  UNICODE_CATEGORY_Sm, /* Symbol, Math */
  UNICODE_CATEGORY_Sc, /* Symbol, Currency */
  UNICODE_CATEGORY_Sk, /* Symbol, Modifier */
  UNICODE_CATEGORY_So, /* Symbol, Other */
  UNICODE_CATEGORY_Zs, /* Separator, Space */
  UNICODE_CATEGORY_Zl, /* Separator, Line */
  UNICODE_CATEGORY_Zp, /* Separator, Paragraph */
  UNICODE_CATEGORY_Cc, /* Other, Control */
  UNICODE_CATEGORY_Cf, /* Other, Format */
  UNICODE_CATEGORY_Cs, /* Other, Surrogate */
  UNICODE_CATEGORY_Co, /* Other, Private Use */
  UNICODE_CATEGORY_Cn, /* Other, Not Assigned */
} unicode_category_t;


/* return values are read-only */
gchar * get_unicode_name (gunichar uc);
gchar * get_unicode_category_name (gunichar uc);


#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif  /* #ifndef UNICODE_INFO_H */

