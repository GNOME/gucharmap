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

#ifndef GUCHARMAP_SEARCH_H
#define GUCHARMAP_SEARCH_H

#include <glib.h>
#include "gucharmap-codepoint-list.h"

G_BEGIN_DECLS

typedef enum
{
  GUCHARMAP_BACKWARD = -1,
  GUCHARMAP_FORWARD = 1
}
GucharmapDirection;

typedef struct _GucharmapSearchState GucharmapSearchState;

gpointer               gucharmap_search_state_get_saved_data (GucharmapSearchState         *search_state);
gunichar               gucharmap_search_state_get_found_char (GucharmapSearchState         *search_state);
void                   gucharmap_search_state_free           (GucharmapSearchState         *search_state);
GucharmapSearchState * gucharmap_search_state_new            (const GucharmapCodepointList *list, 
                                                              const gchar                  *search_string, 
                                                              gint                          start_index, 
                                                              GucharmapDirection            direction, 
                                                              gboolean                      whole_word,
                                                              const gpointer                saved_data);
gboolean               gucharmap_idle_search                 (GucharmapSearchState         *search_state);

G_END_DECLS

#endif /* #ifndef GUCHARMAP_SEARCH_H */
