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
#include <string.h>
#include <stdlib.h>
#include "gucharmap-codepoint-list.h"
#include "gucharmap-search.h"
#include "gucharmap-unicode-info.h"

static const gchar *
utf8_strcasestr (const gchar *haystack, 
                 const gchar *needle)
{
  gint needle_len = strlen (needle);
  gint haystack_len = strlen (haystack);
  const gchar *p, *q, *r;

  for (p = haystack;  p + needle_len <= haystack + haystack_len;  p = g_utf8_next_char (p))
    {
      for (q = needle, r = p;  *q && *r;  q = g_utf8_next_char (q), r = g_utf8_next_char (r))
        {
          gunichar lc0 = g_unichar_tolower (g_utf8_get_char (r));
          gunichar lc1 = g_unichar_tolower (g_utf8_get_char (q));
          if (lc0 != lc1)
            goto next;
        }
      return p;

      next:
        ;
    }

  return NULL;
}

static gboolean
matches (gunichar     wc,
         const gchar *search_string)
{
  const gchar *haystack, *haystack_nfd;
  gboolean matches;
  gchar *needle_nfd;


  needle_nfd = g_utf8_normalize (search_string, -1, G_NORMALIZE_NFD);

  haystack = gucharmap_get_unicode_name (wc);
  if (haystack)
    {
      /* character names are ascii, so are nfd */
      haystack_nfd = haystack;
      /* haystack_nfd = g_utf8_normalize (haystack, -1, G_NORMALIZE_NFD); */
      matches = utf8_strcasestr (haystack_nfd, needle_nfd) != NULL;
      /* g_free (haystack_nfd); */
      if (matches)
        goto yes;
    }

  /* XXX: other strings */

  g_free (needle_nfd);
  return FALSE;

yes: 
  g_free (needle_nfd);
  return TRUE;
}

static gint 
find_next (const GucharmapCodepointList *list,
           const gchar                  *search_string,
           gint                          start_index,
           GucharmapDirection            direction,
           gboolean                      whole_word)
{
  gint i;
  gint n = gucharmap_codepoint_list_get_last_index ((GucharmapCodepointList *) list) + 1;
  gint inc = (direction == GUCHARMAP_BACKWARD) ? -1 : 1; 

  i = start_index;
  do
    {
      i = (i + inc + n) % n;
      gunichar wc = gucharmap_codepoint_list_get_char ((GucharmapCodepointList *) list, i);

      if (!gucharmap_unichar_validate (wc) || !gucharmap_unichar_isdefined (wc))
        continue;

      if (matches (wc, search_string))
        return i;
    }
  while (i != start_index);

  return -1;
}

/* string should have no leading spaces */
static gint
check_for_explicit_codepoint (const GucharmapCodepointList *list,
                              const gchar                  *string)
{
  const gchar *nptr;
  gchar *endptr;

  /* check for explicit decimal codepoint */
  nptr = string;
  if (g_ascii_strncasecmp (string, "&#", 2) == 0)
    nptr = string + 2;
  else if (*string == '#')
    nptr = string + 1;

  if (nptr != string)
    {
      gunichar wc = strtoul (nptr, &endptr, 10);
      if (endptr != nptr)
        {
          gint index = gucharmap_codepoint_list_get_index ((GucharmapCodepointList *) list, wc);
          if (index != -1)
            return index;
        }
    }

  /* check for explicit hex code point */
  nptr = string;
  if (g_ascii_strncasecmp (string, "&#x", 3) == 0)
    nptr = string + 3;
  else if (g_ascii_strncasecmp (string, "U+", 2) == 0 || g_ascii_strncasecmp (string, "0x", 2) == 0)
    nptr = string + 2;

  if (nptr != string)
    {
      gunichar wc = strtoul (nptr, &endptr, 16);
      if (endptr != nptr)
        {
          gint index = gucharmap_codepoint_list_get_index ((GucharmapCodepointList *) list, wc);
          if (index != -1)
            return index;
        }
    }

  return -1;
}

/**
 * gucharmap_find_next:
 * @list: a #GucharmapCodepointList to be searched
 * @search_string: the text to search for
 * @start_index: the starting point within @list
 * @direction: forward or backward
 * @whole_word: %TRUE if it should match whole words
 *
 * Finds the next character in the codepoint list that matches @search_string.
 *
 * Return value: the index of the found character into the codepoint list,
 * or -1 if not found.
 **/
gint 
gucharmap_find_next (const GucharmapCodepointList *list,
                     const gchar                  *search_string,
                     gint                          start_index,
                     GucharmapDirection            direction,
                     gboolean                      whole_word)
{
  const gchar *no_leading_space;
  gint index;

  g_assert (direction == GUCHARMAP_BACKWARD || direction == GUCHARMAP_FORWARD);

  /* caller should check for empty string */
  if (search_string[0] == '\0')
    return -1;

  /* skip spaces */
  for (no_leading_space = search_string;
       g_unichar_isspace (g_utf8_get_char (no_leading_space));
       no_leading_space = g_utf8_next_char (no_leading_space));

  /* check for explicit codepoint */
  index = check_for_explicit_codepoint (list, no_leading_space);
  if (index != -1)
    return index;

  /* if there is only one character, return it as the found character */
  if (g_utf8_strlen (search_string, -1) == 1)
    {
      index = gucharmap_codepoint_list_get_index ((GucharmapCodepointList *) list, g_utf8_get_char (search_string));
      if (index != -1)
        return index;
    }

#if 0
  /* “Unicode character names contain only uppercase Latin letters A through
   * Z, digits, space, and hyphen-minus.” If first character is not one of
   * those, jump to it.
   * */
  if (!(*no_leading_space >= 'A' && *no_leading_space <= 'Z') || 
       (*no_leading_space >= 'a' && *no_leading_space <= 'z') || 
       (*no_leading_space >= '0' && *no_leading_space <= '9') || 
       (*no_leading_space == '-') || 
       (*no_leading_space == '\0'))
    {
      index = gucharmap_codepoint_list_get_index (list, g_utf8_get_char (no_leading_space));
      if (index != -1)
        return index;
    }
#endif

  /* search with leading spaces */
  index = find_next (list, search_string, start_index, direction, whole_word);
  if (index != -1)
    return index;

  /* search without leading spaces */
  index = find_next (list, no_leading_space, start_index, direction, whole_word);
  if (index != -1)
    return index;

  /* jump to the character unless it’s plain ascii */
  if (*no_leading_space < 0x20 || *no_leading_space > 0x7e)
    {
      index = gucharmap_codepoint_list_get_index ((GucharmapCodepointList *) list, g_utf8_get_char (no_leading_space));
      if (index != -1)
        return index;
    }

  return -1;
}
