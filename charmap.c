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
#include <stdlib.h>
#include "charmap.h"
#include "unicode_names.h"


/*
 * when not selected: 
 * when selected: gtk_widget_modify_bg (..., GTK_STATE_NORMAL, &(...->style->base[GTK_STATE_SELECTED]));
 */
/*
 * GTK_STATE_NORMAL GTK_STATE_ACTIVE GTK_STATE_PRELIGHT GTK_STATE_SELECTED GTK_STATE_INSENSITIVE
 * fg[5];   bg[5];   light[5];   dark[5];   mid[5];   text[5];   base[5];   text_aa[5];
 */
/* gtk_label_set_selectable (GTK_LABEL (charmap->squares[row][col]->label), TRUE); */


#if 0
#define debug(x...) g_print (x)
#else
#define debug(x...)
#endif

static void
charmap_class_init (CharmapClass *clazz)
{
  debug ("charmap_class_init starting\n");
  debug ("charmap_class_init finished\n");
}


/* converts "U+1234" to 0x1234 */
static gunichar
block_label_to_unicar (gchar *label)
{
    return (gunichar) strtol (label + 2, NULL, 16);
}


/* don't free or modify what this returns */
static gchar *
friendly_unichar_to_utf8 (gunichar uc)
{
  static gchar buf[8];
  gint n;

  if (g_unichar_isgraph (uc))
    {
      n = g_unichar_to_utf8 (uc, buf);

      if (g_unichar_type (uc) == G_UNICODE_BREAK_COMBINING_MARK)
        {
          buf[n] = ' ';
          buf[n+1] = '\0';
        }
      else 
          buf[n] = '\0';
    }
  else
    {
      buf[0] = ' ';
      buf[1] = '\0';
    }

  return buf;
}


static void
fill_character_table (Charmap *charmap)
{
  gint row, col;

  for (row = 0;  row < charmap->rows;  row++) 
    for (col = 0;  col < charmap->columns;  col++)
      {
        gunichar uc = charmap->block_start + (row * charmap->columns) + col;
        gtk_label_set_text (GTK_LABEL (charmap->squares[row][col]->label), 
                            friendly_unichar_to_utf8 (uc));
      }
}


static void
block_selection_changed_cb (GtkTreeSelection *selection, 
                            gpointer data)
{
  Charmap *charmap;
  GtkTreeIter iter;
  GtkTreeModel *model;
  gunichar new_block_start;
  gchar *block_label;

  debug ("block_selection_changed_cb starting\n");

  charmap = CHARMAP (data);

  if (gtk_tree_selection_get_selected (selection, &model, &iter))
    {
      gtk_tree_model_get (model, &iter, 0, &block_label, -1);
      new_block_start = block_label_to_unicar (block_label);

      if (new_block_start != charmap->block_start)
        {
          charmap->block_start = new_block_start;
          fill_character_table (charmap);
        }

      g_free (block_label);
    }

  debug ("block_selection_changed_cb finished\n");
}


static GtkWidget *
make_unicode_block_selector (Charmap *charmap)
{
  GtkWidget *scrolled_window;
  GtkWidget *tree_view;
  GtkListStore *model;
  GtkCellRenderer *cell;
  GtkTreeViewColumn *column;
  GtkTreeSelection *selection;
  GtkTreeIter iter;
  gchar buf[10];
  gunichar i;

  debug ("make_unicode_block_selector starting\n");

  scrolled_window = gtk_scrolled_window_new (NULL, NULL);
  gtk_scrolled_window_set_policy (GTK_SCROLLED_WINDOW (scrolled_window),
                                  GTK_POLICY_NEVER, GTK_POLICY_AUTOMATIC);

  model = gtk_list_store_new (1, G_TYPE_STRING);
  tree_view = gtk_tree_view_new ();
  selection = gtk_tree_view_get_selection (GTK_TREE_VIEW (tree_view));
  gtk_tree_view_set_headers_visible (GTK_TREE_VIEW (tree_view), FALSE);

  gtk_container_add (GTK_CONTAINER (scrolled_window), tree_view);
  gtk_tree_view_set_model (GTK_TREE_VIEW (tree_view), GTK_TREE_MODEL (model));

  for (i = 0;  i < 0x10000;  i += charmap->rows * charmap->columns)
    {
      g_snprintf (buf, 10, "U+%4.4X", i);
      gtk_list_store_append (GTK_LIST_STORE (model), &iter);
      gtk_list_store_set (GTK_LIST_STORE (model), &iter, 0, buf, -1);
    }

  cell = gtk_cell_renderer_text_new ();
  column = gtk_tree_view_column_new_with_attributes (NULL, cell, "text", 0, NULL);

  gtk_tree_view_append_column (GTK_TREE_VIEW (tree_view),
                               GTK_TREE_VIEW_COLUMN (column));

  gtk_tree_selection_set_mode (selection, GTK_SELECTION_BROWSE);
  g_signal_connect (G_OBJECT (selection), "changed", 
                    G_CALLBACK (block_selection_changed_cb), charmap);

  debug ("make_unicode_block_selector finished\n");
  return scrolled_window;
}


static void
character_clicked_cb (GtkEventBox *event_box, 
                      GdkEvent *event, 
                      gpointer data)
{
  Square *square = (Square *) data;
  Charmap *charmap = square->charmap;

  if (square != charmap->selected)
    {
      gchar *caption_text;
      gunichar uc;

      if (charmap->selected != NULL)
        gtk_widget_set_state (charmap->selected->event_box, GTK_STATE_NORMAL);

      gtk_widget_set_state (square->event_box, GTK_STATE_SELECTED);
      square->charmap->selected = square;

      uc = charmap->block_start 
          + square->row * charmap->columns 
          + square->col;

      caption_text = g_strdup_printf ("U+%4.4X %s %s", uc, 
                                      friendly_unichar_to_utf8 (uc),
                                      get_unicode_name (uc));

      /* XXX do we need to keep track of old caption text to free it here? */

      gtk_label_set_text (GTK_LABEL (charmap->caption), caption_text);
    }
}


static Square *
square_new (guint16 row, 
            guint16 col, 
            GtkWidget *event_box, 
            GtkWidget *label, 
            Charmap *charmap)
{
  Square *square;

  square = g_new (Square, 1);
  square->row = row;
  square->col = col;
  square->event_box = event_box;
  square->label = label;
  square->charmap = charmap;

  return square;
}


static GtkWidget *
init_character_selector (Charmap *charmap)
{
  gint row, col;

  charmap->table = gtk_table_new (charmap->rows, charmap->columns, TRUE);
  gtk_table_set_row_spacings (GTK_TABLE (charmap->table), 1);
  gtk_table_set_col_spacings (GTK_TABLE (charmap->table), 1);

  charmap->squares = g_malloc (charmap->rows * sizeof (Square **));

  for (row = 0;  row < charmap->rows;  row++)
    {
      charmap->squares[row] = g_malloc (charmap->columns * sizeof (Square *));

      for (col = 0;  col < charmap->columns;  col++)
        {
          charmap->squares[row][col] = square_new (row, col, gtk_event_box_new (), gtk_label_new ("x"), charmap);

          gtk_widget_set_events (charmap->squares[row][col]->event_box, 
                                 GDK_BUTTON_PRESS_MASK);

          g_signal_connect (G_OBJECT (charmap->squares[row][col]->event_box), 
                            "button_press_event",
                            G_CALLBACK (character_clicked_cb), 
                            charmap->squares[row][col]);

          /* change the colors to look like text */
          gtk_widget_modify_bg (charmap->squares[row][col]->event_box, 
                                GTK_STATE_NORMAL, 
                                &(GTK_WIDGET(charmap)->style->base[GTK_STATE_NORMAL]));

          gtk_widget_modify_text (charmap->squares[row][col]->event_box, 
                                GTK_STATE_NORMAL, 
                                &(GTK_WIDGET(charmap)->style->text[GTK_STATE_NORMAL]));

          gtk_widget_modify_bg (charmap->squares[row][col]->event_box, 
                                GTK_STATE_SELECTED, 
                                &(GTK_WIDGET(charmap)->style->base[GTK_STATE_SELECTED]));

          /* XXX can't figure out which style is which */
          gtk_widget_modify_text (charmap->squares[row][col]->event_box, 
                                GTK_STATE_SELECTED, 
                                &(GTK_WIDGET(charmap)->style->text[GTK_STATE_NORMAL]));

          gtk_container_add (GTK_CONTAINER (charmap->squares[row][col]->event_box),
                             charmap->squares[row][col]->label);

          gtk_table_attach_defaults (GTK_TABLE (charmap->table), 
                                     charmap->squares[row][col]->event_box, 
                                     col, col + 1, row, row + 1);
        }
    }

  fill_character_table (charmap);

  return charmap->table;
}


static void
charmap_init (Charmap *charmap)
{
  GtkWidget *hbox;
  debug ("charmap_init starting\n");
  charmap->rows = 16;
  charmap->columns = 16;
  charmap->block_start = 0x0000;
  charmap->selected = NULL;

  gtk_box_set_spacing (GTK_BOX (charmap), 10);

  charmap->block_selector = make_unicode_block_selector (charmap);
  charmap->character_selector = init_character_selector (charmap);
  charmap->caption = gtk_label_new ("Unicode Character Map and Font Viewer");
  gtk_label_set_selectable (GTK_LABEL (charmap->caption), TRUE);
  gtk_label_set_justify (GTK_LABEL (charmap->caption), GTK_JUSTIFY_LEFT);

  hbox = gtk_hbox_new (FALSE, 4);
  gtk_box_pack_start (GTK_BOX (hbox), charmap->character_selector, 
                      TRUE, TRUE, 0);
  gtk_box_pack_start (GTK_BOX (hbox), charmap->block_selector, TRUE, TRUE, 0);

  gtk_box_pack_start (GTK_BOX (charmap), hbox, TRUE, TRUE, 0);
  gtk_box_pack_start (GTK_BOX (charmap), charmap->caption, TRUE, TRUE, 0);

  debug ("charmap_init finished\n");
}


GtkWidget*
charmap_new ()
{
  GtkWidget *w;
  debug ("charmap_new starting\n");

  w = GTK_WIDGET (g_object_new (charmap_get_type (), NULL));

  debug ("charmap_new finished\n");
  return w;
}


GtkType
charmap_get_type ()
{
  static GtkType charmap_type = 0;
  debug ("charmap_get_type starting\n");

  if (!charmap_type)
    {
      static const GTypeInfo charmap_info =
      {
	sizeof (CharmapClass),
        NULL,           /* base_init */
        NULL,           /* base_finalize */
        (GClassInitFunc) charmap_class_init,
        NULL,           /* class_finalize */
        NULL,           /* class_data */
        sizeof (GtkCalendar),
        16,             /* n_preallocs */
        (GInstanceInitFunc) charmap_init,
      };

      charmap_type = g_type_register_static (GTK_TYPE_VBOX, "Charmap", 
                                             &charmap_info, 0);
    }

  debug ("charmap_get_type finished\n");
  return charmap_type;
}


void
charmap_set_font (Charmap *charmap, gchar *font_name)
{
  if (charmap->font_name == NULL 
          || g_ascii_strcasecmp (charmap->font_name, font_name) != 0)
    {
      PangoFontDescription *font_desc;
      int row, col;

      g_free (charmap->font_name);
      charmap->font_name = g_strdup (font_name);

      font_desc = pango_font_description_from_string (charmap->font_name);

      for (row = 0;  row < charmap->rows;  row++)
        for (col = 0;  col < charmap->columns;  col++)
          gtk_widget_modify_font (charmap->squares[row][col]->label, 
                                  font_desc);
    }
}


/* the value returned is read-only */
gchar *
charmap_get_font (Charmap *charmap)
{
    return charmap->font_name;
}
