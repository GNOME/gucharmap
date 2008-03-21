/*
 * Copyright © 2004 Noah Levitt
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

#include <config.h>
#include <glib.h>

#include "gucharmap.h"
#include "gucharmap-private.h"

struct _GucharmapBlockCodepointListPrivate
{
  gunichar start;
  gunichar end;
};

static gunichar
get_char (GucharmapCodepointList *list,
          gint                    index)
{
  GucharmapBlockCodepointList *block_list = GUCHARMAP_BLOCK_CODEPOINT_LIST (list);
  GucharmapBlockCodepointListPrivate *priv = block_list->priv;

  if (index > (gint)priv->end - priv->start)
    return (gunichar)(-1);
  else
    return (gunichar) priv->start + index;
}

static gint 
get_index (GucharmapCodepointList *list,
           gunichar                wc)
{
  GucharmapBlockCodepointList *block_list = GUCHARMAP_BLOCK_CODEPOINT_LIST (list);
  GucharmapBlockCodepointListPrivate *priv = block_list->priv;

  if (wc < priv->start || wc > priv->end)
    return -1;
  else
    return wc - priv->start;
}

static gint
get_last_index (GucharmapCodepointList *list)
{
  GucharmapBlockCodepointList *block_list = GUCHARMAP_BLOCK_CODEPOINT_LIST (list);
  GucharmapBlockCodepointListPrivate *priv = block_list->priv;

  return priv->end - priv->start;
}

static void
gucharmap_block_codepoint_list_init (GucharmapBlockCodepointList *list)
{
  list->priv = G_TYPE_INSTANCE_GET_PRIVATE (list, GUCHARMAP_TYPE_BLOCK_CODEPOINT_LIST, GucharmapBlockCodepointListPrivate);
}

static void
gucharmap_block_codepoint_list_class_init (GucharmapBlockCodepointListClass *klass)
{
  GucharmapCodepointListClass *codepoint_list_class = GUCHARMAP_CODEPOINT_LIST_CLASS (klass);

  g_type_class_add_private (klass, sizeof (GucharmapBlockCodepointListPrivate));

  codepoint_list_class->get_char = get_char;
  codepoint_list_class->get_index = get_index;
  codepoint_list_class->get_last_index = get_last_index;
}

G_DEFINE_TYPE (GucharmapBlockCodepointList, gucharmap_block_codepoint_list, GUCHARMAP_TYPE_CODEPOINT_LIST)

/**
 * gucharmap_block_codepoint_list_new:
 *
 * Creates a new codepoint list.
 *
 * Return value: the newly-created #GucharmapBlockCodepointList. Use
 * g_object_unref() to free the result.
 **/
GucharmapCodepointList *
gucharmap_block_codepoint_list_new (gunichar start,
                                    gunichar end)
{
  GucharmapCodepointList *list;
  GucharmapBlockCodepointListPrivate *priv;

  g_return_val_if_fail (start <= end, NULL);

  list = g_object_new (GUCHARMAP_TYPE_BLOCK_CODEPOINT_LIST, NULL);
  priv = GUCHARMAP_BLOCK_CODEPOINT_LIST (list)->priv;

  /* XXX: what to do if start > end, etc */

  priv->start = start;

  if (end <= UNICHAR_MAX)
    priv->end = end;
  else
    priv->end = UNICHAR_MAX;

  return list;
}
