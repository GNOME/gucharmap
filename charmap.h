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

#ifndef CHARMAP_H
#define CHARMAP_H

#include <gtk/gtk.h>

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */


#define CHARMAP(obj)         GTK_CHECK_CAST (obj, charmap_get_type (), Charmap)
#define CHARMAP_CLASS(clazz) GTK_CHECK_CLASS_CAST (clazz, charmap_get_type (), CharmapClass)
#define IS_CHARMAP(obj)      GTK_CHECK_TYPE (obj, charmap_get_type ())

#define CHARMAP_ROWS 16
#define CHARMAP_COLS 16

/* largest legal unicode character */
#define UNICHAR_MAX 0x0010ffff


typedef struct _Charmap Charmap;
typedef struct _CharmapClass CharmapClass;


struct _Charmap
{
  GtkVBox parent;

  GtkWidget *tabulus;         /* GtkDrawingArea* */
  GdkPixmap *tabulus_pixmap; 

  gchar *font_name;
  PangoFontMetrics *font_metrics;
  PangoLayout *pango_layout;

  gunichar page_first_char;  /* the character in the upper left box */
  gunichar active_char;      /* (gunichar)(-1) if none selected */
};


struct _CharmapClass
{
  GtkVBoxClass parent_class;
};


GtkType charmap_get_type (void);
GtkWidget * charmap_new ();


#ifdef __cplusplus
}
#endif  /* #ifdef __cplusplus */

#endif  /* #ifndef CHARMAP_H */
