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

struct _GucharmapSearchState
{
  GucharmapCodepointList *list;
  gchar                  *search_string;
  const gchar            *no_leading_space;  /* points into search_string */
  gint                    start_index;
  gint                    curr_index;
  GucharmapDirection      increment;
  gboolean                whole_word;
  gint                    found_index;       /* index of the found character */
  /* true if there are known to be know matches, or there is known to be
   * exactly one match and it has been found */
  gboolean                dont_search;
  gpointer                saved_data;        /* holds some data to pass back to the caller */
  gint                    list_num_chars;    /* last_index + 1 */
};

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
 * gucharmap_search_state_get_saved_data:
 * @search_state: 
 * Return value: 
 **/
gpointer
gucharmap_search_state_get_saved_data (GucharmapSearchState *search_state)
{
  return search_state->saved_data;
}

/**
 * gucharmap_search_state_get_found_char:
 * @search_state: 
 * Return value: 
 **/
gunichar
gucharmap_search_state_get_found_char (GucharmapSearchState *search_state)
{
  if (search_state->found_index > 0)
    return gucharmap_codepoint_list_get_char (search_state->list, search_state->found_index);
  else
    return (gunichar)(-1);
}

/**
 * gucharmap_search_state_free:
 * @search_state: 
 **/
void
gucharmap_search_state_free (GucharmapSearchState *search_state)
{
  g_free (search_state->search_string);
  g_free (search_state);
}

/**
 * gucharmap_search_state_new:
 * @list: a #GucharmapCodepointList to be searched
 * @search_string: the text to search for
 * @start_index: the starting point within @list
 * @direction: forward or backward
 * @whole_word: %TRUE if it should match whole words
 * @saved_data: data to keep track of
 *
 * Initializes a #GucharmapSearchState to search for the next character in
 * the codepoint list that matches @search_string. Assumes input is valid.
 *
 * Return value: the new #GucharmapSearchState.
 **/
GucharmapSearchState * 
gucharmap_search_state_new (const GucharmapCodepointList *list, 
                            const gchar                  *search_string, 
                            gint                          start_index, 
                            GucharmapDirection            direction, 
                            gboolean                      whole_word,
                            const gpointer                saved_data)
{
  GucharmapSearchState *search_state;

  g_assert (direction == GUCHARMAP_BACKWARD || direction == GUCHARMAP_FORWARD);

  search_state = g_new (GucharmapSearchState, 1);

  search_state->list = (GucharmapCodepointList *) list;
  search_state->list_num_chars = gucharmap_codepoint_list_get_last_index (search_state->list) + 1;

  search_state->search_string = g_strdup (search_string);

  search_state->increment = (direction == GUCHARMAP_BACKWARD) ? -1 : 1; 
  search_state->whole_word = whole_word;

  search_state->found_index = -1;
  search_state->dont_search = FALSE;

  search_state->start_index = start_index;
  search_state->curr_index = start_index;

  search_state->saved_data = saved_data;

  /* set pointer to first non-space character in the search string */
  for (search_state->no_leading_space = search_string;
       g_unichar_isspace (g_utf8_get_char (search_state->no_leading_space));
       search_state->no_leading_space = g_utf8_next_char (search_state->no_leading_space));

  return search_state;
}

static gboolean
quick_checks (GucharmapSearchState *search_state)
{
  gint index;

  if (search_state->dont_search)
    return TRUE;

  /* caller should check for empty string */
  if (search_state->search_string[0] == '\0')
    {
      search_state->dont_search = TRUE;
      return TRUE;
    }

  /* check for explicit codepoint */
  index = check_for_explicit_codepoint (search_state->list, search_state->no_leading_space);
  if (index != -1)
    {
      search_state->found_index = index;
      search_state->dont_search = TRUE;
      return TRUE;
    }

  /* if there is only one character, return it as the found character */
  if (g_utf8_strlen (search_state->search_string, -1) == 1)
    {
      index = gucharmap_codepoint_list_get_index ((GucharmapCodepointList *) search_state->list, 
                                                  g_utf8_get_char (search_state->search_string));
      if (index != -1)
        {
          search_state->found_index = index;
          search_state->dont_search = TRUE;
          return TRUE;
        }
    }

  return FALSE;
}

/**
 * gucharmap_idle_search:
 * @search_state: 
 *
 * Return value: %FALSE if the search is completed, %TRUE otherwise.
 **/
gboolean
gucharmap_idle_search (GucharmapSearchState *search_state)
{
  GTimer *timer = g_timer_new ();
  gint index;

  if (quick_checks (search_state))
    return FALSE;

  /* XXX: search with leading spaces? */

  /* search without leading spaces */
  do
    {
      search_state->curr_index = (search_state->curr_index + search_state->increment + search_state->list_num_chars) % search_state->list_num_chars;
      gunichar wc = gucharmap_codepoint_list_get_char (search_state->list, search_state->curr_index);

      if (!gucharmap_unichar_validate (wc) || !gucharmap_unichar_isdefined (wc))
        continue;

      /* no leading spaces */
      if (matches (wc, search_state->no_leading_space))
        {
          if (search_state->found_index == search_state->curr_index)
            search_state->dont_search = TRUE;  /* this is the only match */

          search_state->found_index = search_state->curr_index;

          return FALSE;
        }

      if (g_timer_elapsed (timer, NULL) > 0.050)
        return TRUE;
    }
  while (search_state->curr_index != search_state->start_index);

  /* jump to the first nonspace character unless it’s plain ascii */
  if (*search_state->no_leading_space < 0x20 || *search_state->no_leading_space > 0x7e)
    {
      index = gucharmap_codepoint_list_get_index (search_state->list, g_utf8_get_char (search_state->no_leading_space));
      if (index != -1)
        {
          search_state->found_index = index;
          search_state->dont_search = TRUE;
        }
    }

  search_state->dont_search = TRUE;
  return FALSE;
}

