/* $Id$ */
/*
 * Copyright (c) 2002  Noah Levitt <nlevitt@users.sourceforge.net>
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

#include <gtk/gtk.h>
#include "tabulus.h"

#if 1
#define debug(x...) g_printerr (x)
#else
#define debug(x...)
#endif



static void
tabulus_class_init (TabulusClass *clazz)
{
  debug ("tabulus_class_init\n");
}


static Cell *
cell_new (guint16 row, 
          guint16 col)
{
  Cell *cell = g_new (Cell, 1);

  cell->row = row;
  cell->col = col;

  cell->label = gtk_label_new ("x");
  cell->event_box = gtk_event_box_new ();

  gtk_container_add (GTK_CONTAINER (cell->event_box), cell->label);

  return cell;
}


static void
tabulus_init (Tabulus *tabulus)
{
  debug ("tabulus_init\n");

  tabulus->cells = NULL;
  tabulus->selected = NULL;
  tabulus->rows = 0;
  tabulus->cols = 0;

  tabulus->table = gtk_table_new (1, 1, TRUE);
  gtk_table_set_row_spacings (GTK_TABLE (tabulus->table), 2);
  gtk_table_set_col_spacings (GTK_TABLE (tabulus->table), 2);
  gtk_container_add (GTK_CONTAINER (tabulus), tabulus->table);

  tabulus_resize (tabulus, 1, 1);
}


GtkWidget *
tabulus_new (guint16 rows, guint16 cols)
{
  GtkWidget *w;

  debug ("tabulus_new\n");

  if (rows <= 0)
    rows = 1;
  if (cols <= 0)
    cols = 1;

  w = GTK_WIDGET (g_object_new (tabulus_get_type (), NULL));

  tabulus_resize (TABULUS (w), rows, cols);

  return w;
}


GtkType
tabulus_get_type ()
{
  static GtkType tabulus_type = 0;

  debug ("tabulus_get_type\n");

  if (!tabulus_type)
    {
      static const GTypeInfo tabulus_info =
      {
        sizeof (TabulusClass),
        NULL,           /* base_init */
        NULL,           /* base_finalize */
        (GClassInitFunc) tabulus_class_init,
        NULL,           /* class_finalize */
        NULL,           /* class_data */
        sizeof (Tabulus),
        0,              /* n_preallocs */
        (GInstanceInitFunc) tabulus_init,
      };

      tabulus_type = g_type_register_static (GTK_TYPE_BIN, "Tabulus", 
                                             &tabulus_info, 0);
    }

  return tabulus_type;
}


static void 
free_cell (Cell *cell)
{
  gtk_widget_destroy (cell->label);
  gtk_widget_destroy (cell->event_box);
  g_free (cell);
}


static void
free_row (Cell **row, guint16 ncols)
{
  guint16 col;

  for (col = 0;  col < ncols;  col++)
    free_cell (row[col]);

  g_free (row);
}


static Cell **
resize_row (Cell **row, guint16 row_num, 
            guint16 old_cols, guint16 new_cols)
{
  guint16 col;

  /* row == null is ok here */
  for (col = new_cols;  col < old_cols;  col++)
    free_cell (row[col]);

  /* row == null is ok here, too */
  row = g_realloc (row, new_cols * sizeof (Cell *));

  for (col = old_cols;  col < new_cols;  col++)
    row[col] = cell_new (row_num, col);

  return row;
}


void
tabulus_resize (Tabulus *tabulus, 
                guint16 rows, 
                guint16 cols)
{
  guint16 old_rows, old_cols;
  guint16 row, col;

  debug ("tabulus_resize\n");

  g_return_if_fail (IS_TABULUS (tabulus));
  g_return_if_fail (tabulus->table != NULL);
  g_return_if_fail (rows > 0);
  g_return_if_fail (cols > 0);

  if (rows == tabulus->rows && cols == tabulus->cols)
    return;

  old_rows = tabulus->rows;
  old_cols = tabulus->cols;

  tabulus->rows = rows;
  tabulus->cols = cols;

  /* if the selected cell will cease to exist, unselect it */
  if (tabulus->selected != NULL
      && (tabulus->selected->row > rows 
          || tabulus->selected->col > cols))
    tabulus->selected = NULL;

  /* tabulus->cells can be null, but only if old_rows == 0 */
  for (row = rows;  row < old_rows;  row++)
    free_row (tabulus->cells[row], old_cols);

  tabulus->cells = g_realloc (tabulus->cells, rows * sizeof (Cell **));

  /* make the new rows null */
  for (row = old_rows;  row < rows;  row++)
    tabulus->cells[row] = NULL;

  for (row = 0;  row < rows;  row++)
    tabulus->cells[row] = resize_row (tabulus->cells[row], 
                                      row, old_cols, cols);

  gtk_table_resize (GTK_TABLE (tabulus->table), rows, cols);

  for (row = old_rows;  row < rows;  row++)
    for (col = old_cols;  col < cols;  col++)
      gtk_table_attach_defaults (GTK_TABLE (tabulus->table),
                                 tabulus->cells[row][col]->event_box,
                                 col, col+1, row, row+1);
}

