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

gint gucharmap_find_next (const GucharmapCodepointList *list,
                          const gchar            *search_text,
                          gint                    start_index,
                          GucharmapDirection      direction,
                          gboolean                whole_word);

G_END_DECLS

#endif /* #ifndef GUCHARMAP_SEARCH_H */
