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

#include "config.h"
#include <glib.h>
#include "gucharmap-intl.h"
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
  GPtrArray *ranges;
};

#define GUCHARMAP_SCRIPT_CODEPOINT_LIST_GET_PRIVATE(o) \
            (G_TYPE_INSTANCE_GET_PRIVATE ((o), gucharmap_script_codepoint_list_get_type (), \
                                          ScriptCodepointListPrivate))

static gint
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

  return -1;
}


/* *ranges should be freed by caller */
/* adds unlisted characters to the "Common" script */
static gboolean
get_chars_for_script (const gchar            *script,
                      UnicodeRange          **ranges,
                      gint                   *size)
{
  gint i, j, index;
  gint script_index, common_script_index;
  gint prev_end;

  script_index = find_script (script);
  common_script_index = find_script ("Common");
  if (script_index == -1)
    return FALSE;

  j = 0;

  if (script_index == common_script_index)
    {
      prev_end = -1;
      for (i = 0;  i < G_N_ELEMENTS (unicode_scripts);  i++)
	{
	  if (unicode_scripts[i].start > prev_end + 1)
	    j++;
	  prev_end = unicode_scripts[i].end;
	}
      if (unicode_scripts[i-1].end < UNICHAR_MAX)
	j++;
    }

  for (i = 0;  i < G_N_ELEMENTS (unicode_scripts);  i++)
    if (unicode_scripts[i].script_index == script_index)
      j++;

  *size = j;
  *ranges = g_new (UnicodeRange, *size);

  j = 0, index = 0, prev_end = -1;

  for (i = 0;  i < G_N_ELEMENTS (unicode_scripts);  i++)
    {
      if (script_index == common_script_index)
	{
	  if (unicode_scripts[i].start > prev_end + 1)
	    {
	      (*ranges)[j].start = prev_end + 1;
	      (*ranges)[j].end = unicode_scripts[i].start - 1;
	      (*ranges)[j].index = index;
      
	      index += (*ranges)[j].end - (*ranges)[j].start + 1;
	      j++;
	    }

	  prev_end = unicode_scripts[i].end;
	}

      if (unicode_scripts[i].script_index == script_index)
	{
	  (*ranges)[j].start = unicode_scripts[i].start;
	  (*ranges)[j].end = unicode_scripts[i].end;
	  (*ranges)[j].index = index;

	  index += (*ranges)[j].end - (*ranges)[j].start + 1;
	  j++;
	}
    }

  if (script_index == common_script_index)
    {
      if (unicode_scripts[i-1].end < UNICHAR_MAX)
	{
	  (*ranges)[j].start = unicode_scripts[i-1].end + 1;
	  (*ranges)[j].end = UNICHAR_MAX;
	  (*ranges)[j].index = index;
	  j++;
	}
    }


  g_assert (j == *size);

  return TRUE;
}

static void
ensure_initialized (GucharmapScriptCodepointList *guscl)
{
  ScriptCodepointListPrivate *priv = GUCHARMAP_SCRIPT_CODEPOINT_LIST_GET_PRIVATE (guscl);
  gboolean success;

  if (priv->ranges != NULL)
    return;

  success = gucharmap_script_codepoint_list_set_script (guscl, "Latin");

  g_assert (success);
}

static gunichar 
get_char (GucharmapCodepointList *list, 
          gint                    index)
{
  GucharmapScriptCodepointList *guscl = GUCHARMAP_SCRIPT_CODEPOINT_LIST (list);
  ScriptCodepointListPrivate *priv = GUCHARMAP_SCRIPT_CODEPOINT_LIST_GET_PRIVATE (guscl);
  gint min, mid, max;

  ensure_initialized (guscl);

  min = 0;
  max = priv->ranges->len - 1;

  while (max >= min) 
    {
      UnicodeRange *range;

      mid = (min + max) / 2;
      range = (UnicodeRange *) (priv->ranges->pdata[mid]);

      if (index > range->index + range->end - range->start)
        min = mid + 1;
      else if (index < range->index)
        max = mid - 1;
      else
        return range->start + index - range->index;
    }

  return (gunichar)(-1);
}

/* XXX: linear search */
static gint
get_index (GucharmapCodepointList *list, 
           gunichar                wc)
{
  GucharmapScriptCodepointList *guscl = GUCHARMAP_SCRIPT_CODEPOINT_LIST (list);
  ScriptCodepointListPrivate *priv = GUCHARMAP_SCRIPT_CODEPOINT_LIST_GET_PRIVATE (guscl);
  gint i;

  ensure_initialized (guscl);

  for (i = 0;  i < priv->ranges->len;  i++)
    {
      UnicodeRange *range = (UnicodeRange *) priv->ranges->pdata[i];
      if (wc >= range->start && wc <= range->end)
        return range->index + wc - range->start;
    }

  return -1;
}

static gint
get_last_index (GucharmapCodepointList *list)
{
  GucharmapScriptCodepointList *guscl = GUCHARMAP_SCRIPT_CODEPOINT_LIST (list);
  ScriptCodepointListPrivate *priv = GUCHARMAP_SCRIPT_CODEPOINT_LIST_GET_PRIVATE (guscl);
  UnicodeRange *last_range;

  ensure_initialized (guscl);

  last_range = (UnicodeRange *) (priv->ranges->pdata[priv->ranges->len-1]);

  return last_range->index + last_range->end - last_range->start;
}

static void
finalize (GObject *object)
{
  GucharmapScriptCodepointList *guscl = GUCHARMAP_SCRIPT_CODEPOINT_LIST (object);
  ScriptCodepointListPrivate *priv = GUCHARMAP_SCRIPT_CODEPOINT_LIST_GET_PRIVATE (guscl);

  if (priv->ranges)
    g_ptr_array_free (priv->ranges, TRUE);
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

  _gucharmap_intl_ensure_initialized ();
}

static void 
gucharmap_script_codepoint_list_init (GucharmapScriptCodepointList *guscl)
{
  ScriptCodepointListPrivate *priv = GUCHARMAP_SCRIPT_CODEPOINT_LIST_GET_PRIVATE (guscl);
  priv->ranges = NULL;
}

G_DEFINE_TYPE (GucharmapScriptCodepointList, gucharmap_script_codepoint_list, gucharmap_codepoint_list_get_type ())

/**
 * gucharmap_script_codepoint_list_new:
 *
 * Creates a new script codepoint list. The default script is Latin.
 *
 * Return value: the newly-created #GucharmapCodepointList. Use
 * g_object_unref() to free the result.
 **/
GucharmapCodepointList * 
gucharmap_script_codepoint_list_new (void)
{
  return GUCHARMAP_CODEPOINT_LIST (g_object_new (gucharmap_script_codepoint_list_get_type (), NULL));
}

/**
 * gucharmap_script_codepoint_list_set_script:
 * @list: a GucharmapScriptCodepointList
 * @script: the script name
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
  const gchar *scripts[2];

  scripts[0] = script;
  scripts[1] = NULL;

  return gucharmap_script_codepoint_list_set_scripts (list, scripts);
}

/**
 * gucharmap_script_codepoint_list_set_scripts:
 * @list: a GucharmapScriptCodepointList
 * @scripts: NULL-terminated array of script names
 *
 * Sets multiple scripts for the codepoint list. Codepoints are sorted
 * according to their order in @scripts.
 *
 * Return value: %TRUE on success, %FALSE if any of the scripts don’t
 * exist, in which case the script is not changed.
 **/
gboolean
gucharmap_script_codepoint_list_set_scripts (GucharmapScriptCodepointList  *list,
	                                     const gchar                  **scripts)
{
  ScriptCodepointListPrivate *priv = GUCHARMAP_SCRIPT_CODEPOINT_LIST_GET_PRIVATE (list);
  UnicodeRange *ranges;
  gint i, j, size;
  
  priv->ranges = g_ptr_array_new ();
  for (i = 0;  scripts[i];  i++)
    if (get_chars_for_script (scripts[i], &ranges, &size))
      {
        for (j = 0;  j < size;  j++)
          g_ptr_array_add (priv->ranges, g_memdup (ranges + j, sizeof (ranges[j])));
        g_free (ranges);
      }
    else
      {
        g_ptr_array_free (priv->ranges, TRUE);
        return FALSE;
      }

  return TRUE;
}

/**
 * gucharmap_script_codepoint_list_append_script:
 * @list: a GucharmapScriptCodepointList
 * @script: the script name
 *
 * Appends the characters in @script to the codepoint list.
 *
 * Return value: %TRUE on success, %FALSE if there is no such script, in
 * which case the codepoint list is not changed.
 **/
gboolean
gucharmap_script_codepoint_list_append_script (GucharmapScriptCodepointList  *list,
                                               const gchar                   *script)
{
  ScriptCodepointListPrivate *priv = GUCHARMAP_SCRIPT_CODEPOINT_LIST_GET_PRIVATE (list);
  UnicodeRange *ranges;
  gint j, size, index0;

  if (priv->ranges == NULL)
    priv->ranges = g_ptr_array_new ();

  if (priv->ranges->len > 0)
    {
      UnicodeRange *last_range = (UnicodeRange *) priv->ranges->pdata[priv->ranges->len - 1];
      index0 = last_range->index  + last_range->end - last_range->start + 1;
    }
  else
    index0 = 0;

  if (get_chars_for_script (script, &ranges, &size))
    {
      for (j = 0;  j < size;  j++)
        {
          UnicodeRange *range = g_memdup (ranges + j, sizeof (ranges[j]));
          range->index += index0;
          g_ptr_array_add (priv->ranges, range);
        }
      g_free (ranges);

      return TRUE;
    }

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
gucharmap_unicode_list_scripts (void)
{
  return unicode_script_list;
}

/**
 * gucharmap_unicode_get_script_for_char:
 * @wc: a character
 *
 * Return value: The English (untranslated) name of the script to which the
 * character belongs. Characters that don't belong to an actual script
 * return %"Common".
 **/
G_CONST_RETURN gchar *
gucharmap_unicode_get_script_for_char (gunichar wc)
{
  gint min = 0;
  gint mid;
  gint max = sizeof (unicode_scripts) / sizeof (UnicodeScript) - 1;

  if (wc > UNICHAR_MAX)
    return NULL;
  
  while (max >= min) 
    {
      mid = (min + max) / 2;
      if (wc > unicode_scripts[mid].end)
        min = mid + 1;
      else if (wc < unicode_scripts[mid].start)
        max = mid - 1;
      else
        return unicode_script_list[unicode_scripts[mid].script_index];
    }

  /* Unicode assigns "Common" as the script name for any character not
   * specifically listed in Scripts.txt */
  return N_("Common");
}
