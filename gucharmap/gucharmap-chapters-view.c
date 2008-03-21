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
 * 59 Temple Place, Suite 330, Boston, MA 02110-1301  USA
 */

#include <config.h>
#include <string.h>
#include <gtk/gtk.h>
#include "gucharmap-settings.h"
#include "gucharmap-chapters-view.h"
#include "gucharmap-private.h"
#include "unicode-blocks.h"

static void
select_iter (GucharmapChaptersView *view,
             GtkTreeIter *iter)
{
  GtkTreeView *tree_view = GTK_TREE_VIEW (view);
  GtkTreeSelection *selection;
  GtkTreePath *path;

  selection = gtk_tree_view_get_selection (tree_view);
  gtk_tree_selection_select_iter (selection, iter);

  path = gtk_tree_model_get_path (gtk_tree_view_get_model (tree_view), iter);
  gtk_tree_view_set_cursor (tree_view, path, NULL, FALSE);
  gtk_tree_view_scroll_to_cell (tree_view, path, NULL, FALSE, 0.5, 0);
  gtk_tree_path_free (path);
}

static void
gucharmap_chapters_view_init (GucharmapChaptersView *view)
{
  GtkTreeView *tree_view = GTK_TREE_VIEW (view);
  GtkCellRenderer *cell;
  GtkTreeViewColumn *column;
  GtkTreeSelection *selection;

  cell = gtk_cell_renderer_text_new ();
  column = view->column = gtk_tree_view_column_new ();
  gtk_tree_view_column_pack_start (column, cell, FALSE);
  gtk_tree_view_column_add_attribute (column, cell, "text", CHAPTERS_LABEL_COL);
  gtk_tree_view_append_column (tree_view, column);

  selection = gtk_tree_view_get_selection (tree_view);
  gtk_tree_selection_set_mode (selection, GTK_SELECTION_BROWSE);
}

static void
gucharmap_chapters_view_class_init (GucharmapChaptersViewClass *klass)
{
}

G_DEFINE_TYPE (GucharmapChaptersView, gucharmap_chapters_view, GTK_TYPE_TREE_VIEW)

GtkWidget * 
gucharmap_chapters_view_new (void)
{
  return g_object_new (gucharmap_chapters_view_get_type (), NULL);
}

GucharmapChaptersModel *
gucharmap_chapters_view_get_model (GucharmapChaptersView *view)
{
  return view->model;
}

void
gucharmap_chapters_view_set_model (GucharmapChaptersView *view,
                                   GucharmapChaptersModel *model)
{
  GtkTreeView *tree_view = GTK_TREE_VIEW (view);

  view->model = model;
  gtk_tree_view_set_model (tree_view, GTK_TREE_MODEL (model));
  if (!model)
    return;

  gtk_tree_view_column_set_title (view->column, gucharmap_chapters_model_get_title (model));
}

/**
 * gucharmap_view_view_next:
 * @view: a #GucharmapChapters
 *
 * Moves to the next chapter if applicable.
 **/
void
gucharmap_chapters_view_next (GucharmapChaptersView *view)
{
  GtkTreeView *tree_view = GTK_TREE_VIEW (view);
  GtkTreeSelection *selection = gtk_tree_view_get_selection (tree_view);
  GtkTreeModel *model;
  GtkTreePath *path;
  GtkTreeIter iter;

  if (!gtk_tree_selection_get_selected (selection, &model, &iter))
    return;
  if (!gtk_tree_model_iter_next (model, &iter))
    return;
   
  path = gtk_tree_model_get_path (model, &iter);
  gtk_tree_view_set_cursor (tree_view, path, NULL, FALSE);
  gtk_tree_path_free (path);
}

/**
 * gucharmap_chapters_view_previous:
 * @view: a #GucharmapChapters
 *
 * Moves to the previous chapter if applicable.
 **/
void
gucharmap_chapters_view_previous (GucharmapChaptersView *view)
{
  GtkTreeView *tree_view = GTK_TREE_VIEW (view);
  GtkTreeSelection *selection = gtk_tree_view_get_selection (tree_view);
  GtkTreeModel *model;
  GtkTreePath *path;
  GtkTreeIter iter;

  if (!gtk_tree_selection_get_selected (selection, &model, &iter))
    return;

  path = gtk_tree_model_get_path (model, &iter);
  if (gtk_tree_path_prev (path))
    gtk_tree_view_set_cursor (tree_view, path, NULL, FALSE);
  gtk_tree_path_free (path);
}


/**
 * gucharmap_chapter_view_get_selected:
 * @view: a #GucharmapChapters
 *
 * Returns a newly allocated string containing
 * the name of the currently selected chapter
 **/
gchar*
gucharmap_chapters_view_get_selected (GucharmapChaptersView *view)
{
  GtkTreeView *tree_view = GTK_TREE_VIEW (view);
  GtkTreeSelection *selection = gtk_tree_view_get_selection (tree_view);
  GtkTreeModel *model;
  GtkTreeIter iter;
  gchar *name = NULL;

  if (gtk_tree_selection_get_selected (selection, &model, &iter))
    gtk_tree_model_get(model, &iter, CHAPTERS_ID_COL, &name, -1);

  return name;
}

/**
 * gucharmap_chapter_view_set_selected:
 * @view: a #GucharmapChapters
 * @name: 
 *
 * Sets the selection to the row specified by @name
 * Return value: %TRUE on success, %FALSE on failure
 **/
gboolean
gucharmap_chapters_view_set_selected (GucharmapChaptersView *view,
                                      const char        *name)
{
  GtkTreeIter iter;

  if (!gucharmap_chapters_model_id_to_iter (view->model, name, &iter))
    return FALSE;

  select_iter (view, &iter);
  return TRUE;
}

/**
 * gucharmap_chapters_view_settings_init:
 * @view: a #GucharmapChaptersView
 * @wc: a character
 *
 * Return value: %TRUE on success, %FALSE on failure.
 **/
gboolean
gucharmap_chapters_view_select_character (GucharmapChaptersView *view,
                                          gunichar           wc)
{
  GtkTreeIter iter;

  g_return_val_if_fail (GUCHARMAP_IS_CHAPTERS_VIEW (view), FALSE);

  if (wc > UNICHAR_MAX)
    return FALSE;

  if (!gucharmap_chapters_model_character_to_iter (view->model, wc, &iter))
    return FALSE;

  select_iter (view, &iter);
  return TRUE;
}

/**
 * gucharmap_chapters_view_get_codepoint_list:
 * @view: a #GucharmapChaptersView
 *
 * Creates a new #GucharmapCodepointList representing the characters in the
 * current chapter.
 *
 * Return value: the newly-created #GucharmapCodepointList, or NULL if
 * there is no chapter selected. The caller should release the result with
 * g_object_unref() when finished.
 **/
GucharmapCodepointList * 
gucharmap_chapters_view_get_codepoint_list (GucharmapChaptersView *view)
{
  GtkTreeSelection *selection;
  GtkTreeIter iter;
  
  g_return_val_if_fail (GUCHARMAP_IS_CHAPTERS_VIEW (view), NULL);

  selection = gtk_tree_view_get_selection (GTK_TREE_VIEW (view));
  if (!gtk_tree_selection_get_selected (selection, NULL, &iter))
    return NULL;

  return gucharmap_chapters_model_get_codepoint_list (view->model, &iter);
}

/**
 * gucharmap_chapters_view_get_codepoint_list:
 * @view: a #GucharmapChaptersView
 *
 * Return value: a #GucharmapCodepointList representing all the characters
 * in all the chapters. It should not be modified or freed.
 **/
G_CONST_RETURN GucharmapCodepointList * 
gucharmap_chapters_view_get_book_codepoint_list (GucharmapChaptersView *view)
{
  g_return_val_if_fail (GUCHARMAP_IS_CHAPTERS_VIEW (view), NULL);

  return gucharmap_chapters_model_get_book_codepoint_list (view->model);
}

gboolean
gucharmap_chapters_view_select_locale (GucharmapChaptersView *view)
{
  return gucharmap_chapters_view_select_character (view,
                                                   gucharmap_settings_get_locale_character ());
}
