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

#include "config.h"
#include <glib.h>
#include <gucharmap/gucharmap-codepoint-list.h>

static gunichar 
default_get_char (GucharmapCodepointList *list, 
                  guint                   index)
{
  if (index > UNICHAR_MAX)
    return (gunichar)(-1);
  else
    return (gunichar) index;
}

static guint
default_get_index (GucharmapCodepointList *list, 
                   gunichar                wc)
{
  if (wc > UNICHAR_MAX)
    return (guint)(-1);
  else
    return (guint) wc;
}

static guint
default_get_last_index (GucharmapCodepointList *list)
{
  return (guint) UNICHAR_MAX;
}

static void
gucharmap_codepoint_list_class_init (GucharmapCodepointListClass *clazz)
{
  /* the default implementation is all unicode codepoints in order */
  clazz->get_char = default_get_char;
  clazz->get_index = default_get_index;
  clazz->get_last_index = default_get_last_index;
}

GType
gucharmap_codepoint_list_get_type ()
{
  static GType t = 0;

  if (t == 0)
    {
      static const GTypeInfo type_info =
        {
          sizeof (GucharmapCodepointListClass),
          NULL,
          NULL,
          (GClassInitFunc) gucharmap_codepoint_list_class_init,
          NULL,
          NULL,
          sizeof (GucharmapCodepointList),
          0,
          NULL
        };

      t = g_type_register_static (G_TYPE_OBJECT, "GucharmapCodepointList", &type_info, 0);
    }

  return t;
}

/**
 * gucharmap_codepoint_list_get_char:
 * @list: a #GucharmapCodepointList
 * @index: index indicating which character to get
 * 
 * Return value: code point at index @index in the codepoint list, or
 *   (gunichar)(-1) if @index is beyond the last index.
 **/
gunichar 
gucharmap_codepoint_list_get_char (GucharmapCodepointList *list, 
                                   guint                   index)
{
  g_return_val_if_fail (IS_GUCHARMAP_CODEPOINT_LIST (list), (gunichar)(-1));

  return GUCHARMAP_CODEPOINT_LIST_GET_CLASS (list)->get_char (list, index);
}

/**
 * gucharmap_codepoint_list_get_index:
 * @list: a #GucharmapCodepointList
 * @wc: character for which to find the index
 * 
 * Return value: index of @wc, or (guint)(-1) if @wc is not in this
 * codepoint list.
 **/
guint
gucharmap_codepoint_list_get_index (GucharmapCodepointList *list, 
                                    gunichar                wc)
{
  g_return_val_if_fail (IS_GUCHARMAP_CODEPOINT_LIST (list), (guint)(-1));

  return GUCHARMAP_CODEPOINT_LIST_GET_CLASS (list)->get_index (list, wc);
}

/**
 * gucharmap_codepoint_list_get_last_index:
 * @list: a #GucharmapCodepointList
 *
 * Return value: last index in this codepoint list.
 **/
guint
gucharmap_codepoint_list_get_last_index (GucharmapCodepointList *list)
{
  g_return_val_if_fail (IS_GUCHARMAP_CODEPOINT_LIST (list), (guint)(-1));

  return GUCHARMAP_CODEPOINT_LIST_GET_CLASS (list)->get_last_index (list);
}

/**
 * gucharmap_codepoint_list_new:
 *
 * Creates a new codepoint list.
 *
 * Return value: the newly-created #GucharmapCodepointList. Use
 * g_object_unref() to free the result.
 **/
GucharmapCodepointList * 
gucharmap_codepoint_list_new ()
{
  return g_object_new (gucharmap_codepoint_list_get_type (), NULL);
}
