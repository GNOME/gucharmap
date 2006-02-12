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
#include "gucharmap-block-chapters.h"
#include "gucharmap-intl.h"
#include "unicode-blocks.h"

enum 
{
  BLOCK_CHAPTERS_LABEL = 0,
  BLOCK_CHAPTERS_UNICODE_BLOCK_PTR,
  BLOCK_CHAPTERS_NUM_COLUMNS
};

static void
selection_changed (GtkTreeSelection       *selection,
                   GucharmapBlockChapters *chapters)
{
  GtkTreeModel *model;
  GtkTreeIter iter;

  /* this should always return true */
  if (gtk_tree_selection_get_selected (selection, &model, &iter))
    /* XXX: parent should have api to do this I guess */
    g_signal_emit_by_name (GUCHARMAP_CHAPTERS (chapters), "changed", 0); 
}

static void
gucharmap_block_chapters_init (GucharmapBlockChapters *chapters)
{
  GucharmapChapters *parent = GUCHARMAP_CHAPTERS (chapters);
  GtkCellRenderer *cell;
  GtkTreeIter iter;
  GtkTreeViewColumn *column;
  GtkTreeSelection *selection;
  gint i;

  parent->book_list = NULL;
  parent->tree_model = GTK_TREE_MODEL (gtk_list_store_new (BLOCK_CHAPTERS_NUM_COLUMNS, 
                                                           G_TYPE_STRING, G_TYPE_POINTER));

  gtk_list_store_append (GTK_LIST_STORE (parent->tree_model), &iter);
  gtk_list_store_set (GTK_LIST_STORE (parent->tree_model), &iter, 
                      BLOCK_CHAPTERS_LABEL, _("All"), 
                      BLOCK_CHAPTERS_UNICODE_BLOCK_PTR, NULL, 
                      -1);

  for (i = 0;  i < G_N_ELEMENTS (unicode_blocks); i++)
    {
      gtk_list_store_append (GTK_LIST_STORE (parent->tree_model), &iter);
      gtk_list_store_set (GTK_LIST_STORE (parent->tree_model), &iter, 
                          BLOCK_CHAPTERS_LABEL, _(unicode_blocks[i].block_name), 
                          BLOCK_CHAPTERS_UNICODE_BLOCK_PTR, unicode_blocks + i,
                          -1);
    }

  parent->tree_view = gtk_tree_view_new_with_model (GTK_TREE_MODEL (parent->tree_model));

  cell = gtk_cell_renderer_text_new ();
  column = gtk_tree_view_column_new ();
  gtk_tree_view_column_pack_start (column, cell, FALSE);
  gtk_tree_view_column_set_title (column, _("Unicode Block"));
  gtk_tree_view_column_add_attribute (column, cell, "text", BLOCK_CHAPTERS_LABEL);
  gtk_tree_view_append_column (GTK_TREE_VIEW (parent->tree_view), column);

  selection = gtk_tree_view_get_selection (GTK_TREE_VIEW (parent->tree_view));
  gtk_tree_selection_set_mode (selection, GTK_SELECTION_BROWSE);

  gtk_tree_model_get_iter_first (parent->tree_model, &iter); /* XXX: check return val */
  gtk_tree_selection_select_iter (selection, &iter);

  g_signal_connect (G_OBJECT (selection), "changed", G_CALLBACK (selection_changed), chapters);

  gtk_container_add (GTK_CONTAINER (chapters), parent->tree_view);
  gtk_widget_show (parent->tree_view);
}

static void
finalize (GObject *object)
{
  GucharmapChapters *chapters = GUCHARMAP_CHAPTERS (object);

  /* XXX: what to free and how? */
  g_object_unref (chapters->tree_model);
  /* g_object_unref (chapters->tree_view); */

  if (chapters->book_list)
    g_object_unref (chapters->book_list);
}

static GucharmapCodepointList * 
get_codepoint_list (GucharmapChapters *chapters)
{
  GtkTreeSelection *selection;
  GtkTreeModel *model;
  GtkTreeIter iter;

  selection = gtk_tree_view_get_selection (GTK_TREE_VIEW (chapters->tree_view));

  if (gtk_tree_selection_get_selected (selection, &model, &iter))
    {
      UnicodeBlock *unicode_block;
      gtk_tree_model_get (model, &iter, BLOCK_CHAPTERS_UNICODE_BLOCK_PTR, &unicode_block, -1);

      /* special "All" block */
      if (unicode_block == NULL)
        return gucharmap_codepoint_list_new (0, UNICHAR_MAX);
      else
        return gucharmap_codepoint_list_new (unicode_block->start, unicode_block->end);
    }
  else
    return NULL;
}

static G_CONST_RETURN GucharmapCodepointList * 
get_book_codepoint_list (GucharmapChapters *chapters)
{
  if (chapters->book_list == NULL)
    chapters->book_list = gucharmap_codepoint_list_new (0, UNICHAR_MAX);

  return chapters->book_list;
}

/* XXX linear search */
static gboolean
go_to_character (GucharmapChapters *chapters, 
                 gunichar           wc)
{
  GucharmapChapters *parent = GUCHARMAP_CHAPTERS (chapters);
  GtkTreeSelection *selection;
  GtkTreeIter iter;

  if (wc > UNICHAR_MAX)
    return FALSE;

  /* skip "All" block */
  if (!gtk_tree_model_get_iter_first (parent->tree_model, &iter))
    return FALSE;

  while (gtk_tree_model_iter_next (parent->tree_model, &iter))
    {
      UnicodeBlock *unicode_block;
      gtk_tree_model_get (parent->tree_model, &iter, BLOCK_CHAPTERS_UNICODE_BLOCK_PTR, &unicode_block, -1);
      if (wc >= unicode_block->start && wc <= unicode_block->end)
        {
          selection = gtk_tree_view_get_selection (GTK_TREE_VIEW (parent->tree_view));
          if (!gtk_tree_selection_iter_is_selected (selection, &iter))
            {
              GtkTreePath *path = gtk_tree_model_get_path (parent->tree_model, &iter);
              gtk_tree_view_set_cursor (GTK_TREE_VIEW (parent->tree_view), path, NULL, FALSE);
              gtk_tree_path_free (path);
            }
          return TRUE;
        }
    }

  /* "All" is the last resort */
  if (!gtk_tree_model_get_iter_first (parent->tree_model, &iter))
    return FALSE;

  selection = gtk_tree_view_get_selection (GTK_TREE_VIEW (parent->tree_view));
  if (!gtk_tree_selection_iter_is_selected (selection, &iter))
    {
      GtkTreePath *path = gtk_tree_model_get_path (parent->tree_model, &iter);
      gtk_tree_view_set_cursor (GTK_TREE_VIEW (parent->tree_view), path, NULL, FALSE);
      gtk_tree_path_free (path);
    }

  return TRUE;
}

static void
gucharmap_block_chapters_class_init (GucharmapBlockChaptersClass *clazz)
{
  GObjectClass *gobject_class = G_OBJECT_CLASS (clazz);
  GucharmapChaptersClass *chapters_class = GUCHARMAP_CHAPTERS_CLASS (clazz);

  gobject_class->finalize = finalize;
  chapters_class->get_codepoint_list = get_codepoint_list;
  chapters_class->get_book_codepoint_list = get_book_codepoint_list;
  chapters_class->go_to_character = go_to_character;

  gucharmap_intl_ensure_initialized ();
}

GType 
gucharmap_block_chapters_get_type (void)
{
  static GType t = 0;

  if (t == 0)
    {
      static const GTypeInfo type_info =
        {
          sizeof (GucharmapBlockChaptersClass),
          NULL,
          NULL,
          (GClassInitFunc) gucharmap_block_chapters_class_init,
          NULL,
          NULL,
          sizeof (GucharmapBlockChapters),
          0,
          (GInstanceInitFunc) gucharmap_block_chapters_init,
          NULL
        };

      t = g_type_register_static (gucharmap_chapters_get_type (), "GucharmapBlockChapters", &type_info, 0);
    }

  return t;
}

GtkWidget * 
gucharmap_block_chapters_new (void)
{
  return GTK_WIDGET (g_object_new (gucharmap_block_chapters_get_type (), NULL));
}

