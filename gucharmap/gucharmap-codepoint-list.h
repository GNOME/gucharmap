/* $Id$ */
/*
 * Copyright (c) 2004 Noah Levitt
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

#ifndef GUCHARMAP_CODEPOINT_LIST_H
#define GUCHARMAP_CODEPOINT_LIST_H

#include <glib-object.h>

G_BEGIN_DECLS

#define GUCHARMAP_CODEPOINT_LIST(obj) \
            (G_TYPE_CHECK_INSTANCE_CAST ((obj), gucharmap_codepoint_list_get_type (), GucharmapCodepointList))

#define GUCHARMAP_CODEPOINT_LIST_CLASS(clazz) \
            (G_TYPE_CHECK_CLASS_CAST ((clazz), gucharmap_codepoint_list_get_type (), GucharmapCodepointListClass))

#define IS_GUCHARMAP_CODEPOINT_LIST(obj) \
             (G_TYPE_CHECK_INSTANCE_TYPE ((obj), gucharmap_codepoint_list_get_type ()))

#define GUCHARMAP_CODEPOINT_LIST_GET_CLASS(obj) \
             (G_TYPE_INSTANCE_GET_CLASS ((obj), gucharmap_codepoint_list_get_type (), GucharmapCodepointListClass))

typedef struct _GucharmapCodepointList GucharmapCodepointList;
typedef struct _GucharmapCodepointListClass GucharmapCodepointListClass;

struct _GucharmapCodepointList
{
  GObject parent;
};

struct _GucharmapCodepointListClass
{
  GObjectClass parent_class;

  /* zero is the first index */
  gint     (*get_last_index) (GucharmapCodepointList *list);
  gunichar (*get_char)       (GucharmapCodepointList *list, 
                              gint                    index);
  gint     (*get_index)      (GucharmapCodepointList *list, 
                              gunichar                wc);
};

GType                    gucharmap_codepoint_list_get_type       ();
GucharmapCodepointList * gucharmap_codepoint_list_new            (gunichar start,
                                                                  gunichar end);
gunichar                 gucharmap_codepoint_list_get_char       (GucharmapCodepointList *list, 
                                                                  gint                    index);
gint                     gucharmap_codepoint_list_get_index      (GucharmapCodepointList *list, 
                                                                  gunichar                wc);
gint                     gucharmap_codepoint_list_get_last_index (GucharmapCodepointList *list);

G_END_DECLS

#endif /* #ifndef GUCHARMAP_CODEPOINT_LIST_H */

