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
#include <string.h>
#include "gucharmap-script-codepoint-list.h"
#include "unicode-scripts.h"

typedef struct
{
  gunichar start;
  gunichar end;
  gint index;   /* index of @start in the codepoint list */
}
UnicodeRange;

typedef struct _ScriptCodepointListPrivate ScriptCodepointListPrivate;

struct _ScriptCodepointListPrivate 
{
  gchar         *script;
  UnicodeRange  *ranges;
  gint           n_ranges;
};

#define GUCHARMAP_SCRIPT_CODEPOINT_LIST_GET_PRIVATE(o) \
            (G_TYPE_INSTANCE_GET_PRIVATE ((o), gucharmap_script_codepoint_list_get_type (), \
                                          ScriptCodepointListPrivate))

#if 0
G_CONST_RETURN gchar *
gucharmap_get_script_for_char (gunichar wc)
{
  gint min = 0;
  gint mid;
  gint max = sizeof (unicode_scripts) / sizeof (UnicodeScript) - 1;

  if (!gucharmap_unichar_isdefined (wc))
    return NULL;
  
  while (max >= min) 
    {
      mid = (min + max) / 2;
      if (wc > unicode_scripts[mid].end)
        min = mid + 1;
      else if (wc < unicode_scripts[mid].start)
        max = mid - 1;
      else
        return unicode_scripts[mid].script;
    }

  /* Unicode assigns "Common" as the script name for any character not
   * specifically listed in Scripts.txt */
  return _("Common");
}
#endif

static guint
find_script (const gchar *script)
{
  gint min, mid, max;

  min = 0;
  max = sizeof (unicode_script_list) / sizeof (gchar *) - 2;  /* it’s NULL-terminated */

  while (max >= min) 
    {
      mid = (min + max) / 2;

      if (strcmp (script, unicode_script_list[mid]) > 0)
        min = mid + 1;
      else if (strcmp (script, unicode_script_list[mid]) < 0)
        max = mid - 1;
      else
        return mid;
    }

  return (guint)(-1);
}

/* *ranges should be freed by caller */
static gboolean
get_chars_for_script (const gchar            *script,
                      UnicodeRange          **ranges,
                      gint                   *size)
{
  gint i, j, index;
  guint script_index = find_script (script);

  if (script_index == (guint)(-1))
    return FALSE;

  for (i = 0, j = 0;  i < sizeof (unicode_scripts) / sizeof (UnicodeScript);  i++)
    if (unicode_scripts[i].script_index == script_index)
      j++;

  *size = j;
  *ranges = g_new (UnicodeRange, *size);

  for (i = 0, j = 0, index = 0;  i < sizeof (unicode_scripts) / sizeof (UnicodeScript);  i++)
    if (unicode_scripts[i].script_index == script_index)
      {
        (*ranges)[j].start = unicode_scripts[i].start;
        (*ranges)[j].end = unicode_scripts[i].end;
        (*ranges)[j].index = index;

        index += (*ranges)[j].end - (*ranges)[j].start + 1;
        j++;
      }

  g_assert (j == *size);

  return TRUE;
}

static void
ensure_initialized (GucharmapScriptCodepointList *guscl)
{
  ScriptCodepointListPrivate *priv = GUCHARMAP_SCRIPT_CODEPOINT_LIST_GET_PRIVATE (guscl);
  gboolean success;

  if (priv->script != NULL)
    return;

  success = gucharmap_script_codepoint_list_set_script (guscl, "Latin");

  g_assert (success);
}

static gunichar 
get_char (GucharmapCodepointList *list, 
          guint                   index)
{
  GucharmapScriptCodepointList *guscl = GUCHARMAP_SCRIPT_CODEPOINT_LIST (list);
  ScriptCodepointListPrivate *priv = GUCHARMAP_SCRIPT_CODEPOINT_LIST_GET_PRIVATE (guscl);
  gint min, mid, max;

  ensure_initialized (guscl);

  min = 0;
  max = priv->n_ranges - 1;

  while (max >= min) 
    {
      mid = (min + max) / 2;
      if (index > priv->ranges[mid].index + priv->ranges[mid].end - priv->ranges[mid].start)
        min = mid + 1;
      else if (index < priv->ranges[mid].index)
        max = mid - 1;
      else
        return priv->ranges[mid].start + index - priv->ranges[mid].index;
    }

  return (gunichar)(-1);
}

static guint
get_index (GucharmapCodepointList *list, 
           gunichar                wc)
{
  GucharmapScriptCodepointList *guscl = GUCHARMAP_SCRIPT_CODEPOINT_LIST (list);
  ScriptCodepointListPrivate *priv = GUCHARMAP_SCRIPT_CODEPOINT_LIST_GET_PRIVATE (guscl);
  gint min, mid, max;

  ensure_initialized (guscl);

  if (!gucharmap_unichar_isdefined (wc))
    return (gunichar)(-1);
  
  min = 0;
  max = priv->n_ranges - 1;

  while (max >= min) 
    {
      mid = (min + max) / 2;
      if (wc > priv->ranges[mid].end)
        min = mid + 1;
      else if (wc < priv->ranges[mid].start)
        max = mid - 1;
      else
        return priv->ranges[mid].index + wc - priv->ranges[mid].start;
    }

  return (guint)(-1);
}

static guint
get_last_index (GucharmapCodepointList *list)
{
  GucharmapScriptCodepointList *guscl = GUCHARMAP_SCRIPT_CODEPOINT_LIST (list);
  ScriptCodepointListPrivate *priv = GUCHARMAP_SCRIPT_CODEPOINT_LIST_GET_PRIVATE (guscl);

  ensure_initialized (guscl);

  return priv->ranges[priv->n_ranges-1].index + priv->ranges[priv->n_ranges-1].end - priv->ranges[priv->n_ranges-1].start;
}

static void
finalize (GObject *object)
{
  GucharmapScriptCodepointList *guscl = GUCHARMAP_SCRIPT_CODEPOINT_LIST (object);
  ScriptCodepointListPrivate *priv = GUCHARMAP_SCRIPT_CODEPOINT_LIST_GET_PRIVATE (guscl);

  if (priv->script)
    g_free (priv->script);

  if (priv->ranges)
    g_free (priv->ranges);
}

static void
gucharmap_script_codepoint_list_class_init (GucharmapScriptCodepointListClass *clazz)
{
  GucharmapCodepointListClass *codepoint_list_class = GUCHARMAP_CODEPOINT_LIST_CLASS (clazz);
  GObjectClass *gobject_class = G_OBJECT_CLASS (clazz);

  g_type_class_add_private (codepoint_list_class, sizeof (ScriptCodepointListPrivate));

  codepoint_list_class->get_char = get_char;
  codepoint_list_class->get_index = get_index;
  codepoint_list_class->get_last_index = get_last_index;
  
  gobject_class->finalize = finalize;
}

static void 
gucharmap_script_codepoint_list_init (GucharmapScriptCodepointList *guscl)
{
  ScriptCodepointListPrivate *priv = GUCHARMAP_SCRIPT_CODEPOINT_LIST_GET_PRIVATE (guscl);
  priv->script = NULL;
  priv->ranges = NULL;
  priv->n_ranges = 0;
}

GType
gucharmap_script_codepoint_list_get_type ()
{
  static GType t = 0;

  if (t == 0)
    {
      static const GTypeInfo type_info =
        {
          sizeof (GucharmapScriptCodepointListClass),
          NULL,
          NULL,
          (GClassInitFunc) gucharmap_script_codepoint_list_class_init,
          NULL,
          NULL,
          sizeof (GucharmapScriptCodepointList),
          0,
          (GInstanceInitFunc) gucharmap_script_codepoint_list_init,
          NULL
        };

      t = g_type_register_static (gucharmap_codepoint_list_get_type (), 
                                  "GucharmapScriptCodepointList", &type_info, 0);
    }

  return t;
}

/**
 * gucharmap_script_codepoint_list_new:
 *
 * Creates a new script codepoint list. The default script is Latin.
 *
 * Return value: the newly-created #GucharmapCodepointList. Use
 * g_object_unref() to free the result.
 **/
GucharmapCodepointList * 
gucharmap_script_codepoint_list_new ()
{
  return GUCHARMAP_CODEPOINT_LIST (g_object_new (gucharmap_script_codepoint_list_get_type (), NULL));
}

/**
 * gucharmap_script_codepoint_list_set_script:
 *
 * Sets the script for the codepoint list. 
 *
 * Return value: %TRUE on success, %FALSE if there is no such script, in
 * which case the script is not changed.
 **/
gboolean 
gucharmap_script_codepoint_list_set_script (GucharmapScriptCodepointList *list,
                                            const gchar                  *script)
{
  ScriptCodepointListPrivate *priv = GUCHARMAP_SCRIPT_CODEPOINT_LIST_GET_PRIVATE (list);
  UnicodeRange *ranges;
  gint size;

  if (priv->script && strcmp (script, priv->script) == 0)
    return TRUE;

  if (get_chars_for_script (script, &ranges, &size))
    {
      g_free (priv->script);
      g_free (priv->ranges);
      
      priv->script = g_strdup (script);
      priv->ranges = ranges;
      priv->n_ranges = size;

      return TRUE;
    }
  else
    return FALSE;
}

/**
 * gucharmap_unicode_list_scripts:
 *
 * Return value: NULL-terminated array of script names. These have been
 * marked for translation with N_(). Neither the list nor the scripts
 * should be modified by the caller.
 **/
G_CONST_RETURN gchar **
gucharmap_unicode_list_scripts ()
{
  return unicode_script_list;
}

