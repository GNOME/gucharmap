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


typedef struct _Charmap Charmap;
typedef struct _CharmapClass CharmapClass;


typedef struct _Square Square;

struct _Square
{
  GtkWidget *event_box;
  GtkWidget *label;
  guint16 row, col;
  Charmap *charmap; /* the charmap to which this square belongs */
};


struct _Charmap
{
  GtkVBox parent;

  GtkWidget *character_selector;
  GtkWidget *table;
  Square ***squares;
  Square *selected;

  GtkWidget *block_selector;
  gchar *font_name;
  gint rows, columns;
  gunichar block_start;

  GtkWidget *caption;
};

struct _CharmapClass
{
  GtkVBoxClass parent_class;
};

GtkType     charmap_get_type (void);
GtkWidget * charmap_new (void);
void        charmap_set_font (Charmap *charmap, gchar *font_name);
gchar *     charmap_get_font (Charmap *charmap);


#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif  /* #ifndef CHARMAP_H */


