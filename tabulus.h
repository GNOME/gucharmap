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

#ifndef TABULUS_H
#define TABULUS_H

#include <gtk/gtk.h>

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */


#define TABULUS(obj)         GTK_CHECK_CAST (obj, tabulus_get_type (), Tabulus)
#define TABULUS_CLASS(clazz) GTK_CHECK_CLASS_CAST (clazz, tabulus_get_type (), TabulusClass)
#define IS_TABULUS(obj)      GTK_CHECK_TYPE (obj, tabulus_get_type ())


typedef struct _Tabulus Tabulus;
typedef struct _TabulusClass TabulusClass;


typedef struct _Cell Cell;

struct _Cell
{
  GtkWidget *event_box;
  GtkWidget *label;     /* XXX: maybe this can be a general widget */
  guint16 row, col;
#if 0
   Charmap *charmap; /* the charmap to which this square belongs */
#endif
};


struct _Tabulus
{
  Cell ***cells;
  Cell *selected;
  guint16 rows, cols;
};


struct _TabulusClass
{
  GtkBinClass parent_class;
};


GtkType      tabulus_get_type (void);
GtkWidget *  tabulus_new (guint16 rows, guint16 cols);
void         tabulus_resize (Tabulus *tabulus, guint16 rows, guint16 cols);


#ifdef __cplusplus
}
#endif  /* #ifdef __cpulusplus */

#endif  /* #ifndef TABULUS_H */
