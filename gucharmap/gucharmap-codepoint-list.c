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
 * 59 Temple Place, Suite 330, Boston, MA 02110-1301  USA
 */

#include "config.h"
#include <glib.h>
#include "gucharmap-codepoint-list.h"
#include "gucharmap-private.h"

typedef struct _DefaultCodepointListPrivate DefaultCodepointListPrivate;

struct _DefaultCodepointListPrivate
{
  gunichar start;
  gunichar end;
};

#define GUCHARMAP_CODEPOINT_LIST_GET_PRIVATE(o) \
            (G_TYPE_INSTANCE_GET_PRIVATE ((o), gucharmap_codepoint_list_get_type (), DefaultCodepointListPrivate))

static gunichar 
default_get_char (GucharmapCodepointList *list, 
                  gint                    index)
{
  DefaultCodepointListPrivate *priv = GUCHARMAP_CODEPOINT_LIST_GET_PRIVATE (list);

  if (index > (gint)priv->end - priv->start)
    return (gunichar)(-1);
  else
    return (gunichar) priv->start + index;
}

static gint 
default_get_index (GucharmapCodepointList *list, 
                   gunichar                wc)
{
  DefaultCodepointListPrivate *priv = GUCHARMAP_CODEPOINT_LIST_GET_PRIVATE (list);

  if (wc < priv->start || wc > priv->end)
    return -1;
  else
    return wc - priv->start;
}

static gint
default_get_last_index (GucharmapCodepointList *list)
{
  DefaultCodepointListPrivate *priv = GUCHARMAP_CODEPOINT_LIST_GET_PRIVATE (list);

  return priv->end - priv->start;
}

static void
gucharmap_codepoint_list_init (GucharmapCodepointList *list)
{
}

static void
gucharmap_codepoint_list_class_init (GucharmapCodepointListClass *clazz)
{
  g_type_class_add_private (clazz, sizeof (DefaultCodepointListPrivate));

  /* the default implementation is all unicode codepoints in order */
  clazz->get_char = default_get_char;
  clazz->get_index = default_get_index;
  clazz->get_last_index = default_get_last_index;
}

G_DEFINE_TYPE (GucharmapCodepointList, gucharmap_codepoint_list, G_TYPE_OBJECT)

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
                                   gint                    index)
{
  g_return_val_if_fail (GUCHARMAP_IS_CODEPOINT_LIST (list), (gunichar)(-1));

  return GUCHARMAP_CODEPOINT_LIST_GET_CLASS (list)->get_char (list, index);
}

/**
 * gucharmap_codepoint_list_get_index:
 * @list: a #GucharmapCodepointList
 * @wc: character for which to find the index
 * 
 * Return value: index of @wc, or -1 if @wc is not in this
 * codepoint list.
 **/
gint
gucharmap_codepoint_list_get_index (GucharmapCodepointList *list, 
                                    gunichar                wc)
{
  g_return_val_if_fail (GUCHARMAP_IS_CODEPOINT_LIST (list), -1);

  return GUCHARMAP_CODEPOINT_LIST_GET_CLASS (list)->get_index (list, wc);
}

/**
 * gucharmap_codepoint_list_get_last_index:
 * @list: a #GucharmapCodepointList
 *
 * Return value: last index in this codepoint list.
 **/
gint
gucharmap_codepoint_list_get_last_index (GucharmapCodepointList *list)
{
  g_return_val_if_fail (GUCHARMAP_IS_CODEPOINT_LIST (list), -1);

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
gucharmap_codepoint_list_new (gunichar start,
                              gunichar end)
{
  GucharmapCodepointList *list;
  DefaultCodepointListPrivate *priv;

  list = GUCHARMAP_CODEPOINT_LIST (g_object_new (gucharmap_codepoint_list_get_type (), NULL));
  priv = GUCHARMAP_CODEPOINT_LIST_GET_PRIVATE (list);

  /* XXX: what to do if start > end, etc */

  priv->start = start;

  if (end <= UNICHAR_MAX)
    priv->end = end;
  else
    priv->end = UNICHAR_MAX;

  return list;
}
