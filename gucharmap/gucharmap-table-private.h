/* $Id: gucharmap-table.h 692 2004-02-01 00:17:34Z nlevitt $ */
/*
 * Copyright (c) 2007 Christian Persch
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

#ifndef GUCHARMAP_TABLE_PRIVATE_H
#define GUCHARMAP_TABLE_PRIVATE_H

G_BEGIN_DECLS

gint _gucharmap_table_cell_column	(GucharmapTable *chartable,
					 guint cell);
gint _gucharmap_table_column_width	(GucharmapTable *chartable,
					 gint col);
gint _gucharmap_table_x_offset		(GucharmapTable *chartable,
					 gint col);
gint _gucharmap_table_row_height	(GucharmapTable *chartable,
		 			 gint row);
gint _gucharmap_table_y_offset		(GucharmapTable *chartable,
					 gint row);
void _gucharmap_table_redraw		(GucharmapTable *chartable,
					 gboolean move_zoom);

G_END_DECLS

#endif  /* #ifndef GUCHARMAP_TABLE_PRIVATE_H */
