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
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.	 See the GNU
 * Library General Public License for more details.
 *
 * You should have received a copy of the GNU Library General Public
 * License along with this library; if not, write to the
 * Free Software Foundation, Inc., 59 Temple Place - Suite 330,
 * Boston, MA 02111-1307, USA.
 */

#ifndef CHARCELL_ACCESSIBLE_H
#define CHARCELL_ACCESSIBLE_H

#include <atk/atk.h>


G_BEGIN_DECLS


#define CHARCELL_ACCESSIBLE(obj) (G_TYPE_CHECK_INSTANCE_CAST ((obj), charcell_accessible_get_type (), CharcellAccessible))
#define CHARCELL_ACCESSIBLE_CLASS(klass) (G_TYPE_CHECK_CLASS_CAST ((klass), charcell_accessible_get_type (), CharcellAccessibleClass))
#define IS_CHARCELL_ACCESSIBLE(obj) (G_TYPE_CHECK_INSTANCE_TYPE ((obj), charcell_accessible_get_type ()))
#define IS_CHARCELL_ACCESSIBLE_CLASS(klass) (G_TYPE_CHECK_CLASS_TYPE ((klass), charcell_accessible_get_type ()))
#define CHARCELL_ACCESSIBLE_GET_CLASS(obj) (G_TYPE_INSTANCE_GET_CLASS ((obj), charcell_accessible_get_type, CharcellAccessibleClass))


typedef struct _CharcellAccessible      CharcellAccessible;
typedef struct _CharcellAccessibleClass CharcellAccessibleClass;
  

struct _CharcellAccessible
{
  AtkObject parent;

  GtkWidget    *widget;
  gint         index;
  AtkStateSet  *state_set;
  gchar        *activate_description;
  guint        action_idle_handler;
};


GType charcell_accessible_get_type (void);


struct _CharcellAccessibleClass
{
  AtkObjectClass parent_class;
};


AtkObject* charcell_accessible_new (void);
void charcell_accessible_init (CharcellAccessible *cell,
                               GtkWidget          *widget, 
                               AtkObject          *parent,
		               gint                index);

gboolean charcell_accessible_add_state (CharcellAccessible *cell,
                                        AtkStateType        state_type,
                                        gboolean            emit_signal);

gboolean charcell_accessible_remove_state (CharcellAccessible *cell,
                                           AtkStateType        state_type,
                                           gboolean            emit_signal);


G_END_DECLS


#endif /* CHARCELL_ACCESSIBLE_H */
