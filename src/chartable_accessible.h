/* $Id$ */
/* 
 * Copyright (c) 2003  Sun Microsystems Inc.
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Library General Public
 * License as published by the Free Software Foundation; either
 * version 2 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Library General Public License for more details.
 *
 * You should have received a copy of the GNU Library General Public
 * License along with this library; if not, write to the
 * Free Software Foundation, Inc., 59 Temple Place - Suite 330,
 * Boston, MA 02111-1307, USA.
 */

#ifndef CHARTABLE_ACCESSIBLE_H
#define CHARTABLE_ACCESSIBLE_H

#include <gtk/gtkaccessible.h>


G_BEGIN_DECLS


#define CHARTABLE_ACCESSIBLE(obj) (G_TYPE_CHECK_INSTANCE_CAST ((obj), \
                                   chartable_accessible_get_type (), \
                                   ChartableAccessible))
#define CHARTABLE_ACCESSIBLE_CLASS(klass) (G_TYPE_CHECK_CLASS_CAST ((klass), \
                                           chartable_accessible_get_type (), \
                                           ChartableAccessibleClass))
#define IS_CHARTABLE_ACCESSIBLE(obj) (G_TYPE_CHECK_INSTANCE_TYPE ((obj), \
                                      chartable_accessible_get_type ()))
#define IS_CHARTABLE_ACCESSIBLE_CLASS(klass) \
    (G_TYPE_CHECK_CLASS_TYPE ((klass), chartable_accessible_get_type ()))
#define CHARTABLE_ACCESSIBLE_GET_CLASS(obj) (G_TYPE_INSTANCE_GET_CLASS ((obj),\
                                             chartable_accessible_get_type (),\
                                             ChartableAccessibleClass))


typedef struct _ChartableAccessible      ChartableAccessible;
typedef struct _ChartableAccessibleClass ChartableAccessibleClass;


struct _ChartableAccessible
{
  GtkAccessible parent;
};


GType chartable_accessible_get_type (void);


struct _ChartableAccessibleClass
{
  GtkAccessibleClass parent_class;
};


AtkObject *chartable_accessible_new (GtkWidget *widget);


G_END_DECLS


#endif /* #ifndef CHARTABLE_ACCESSIBLE_H */
