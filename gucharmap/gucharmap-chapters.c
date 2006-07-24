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
#include "gucharmap-chapters.h"
#include "gucharmap-marshal.h"

enum
{
  CHANGED,
  NUM_SIGNALS
};

static guint gucharmap_chapters_signals[NUM_SIGNALS] = { 0 };

static GucharmapCodepointList * 
default_get_codepoint_list (GucharmapChapters *chapters)
{
  return gucharmap_codepoint_list_new (0, UNICHAR_MAX);
}

static void
gucharmap_chapters_init (GucharmapChapters *chapters)
{
  /* have to do this so that the scrollbars are inited for some reason */
  gtk_widget_set (GTK_WIDGET (chapters), "hadjustment", NULL, "vadjustment", NULL, NULL);

  gtk_scrolled_window_set_policy (GTK_SCROLLED_WINDOW (chapters), 
                                  GTK_POLICY_NEVER, GTK_POLICY_AUTOMATIC);
  gtk_scrolled_window_set_shadow_type (GTK_SCROLLED_WINDOW (chapters), GTK_SHADOW_ETCHED_IN);
}

static void
gucharmap_chapters_class_init (GucharmapChaptersClass *clazz)
{
  clazz->get_codepoint_list = default_get_codepoint_list;

  clazz->changed = NULL;
  gucharmap_chapters_signals[CHANGED] =
      g_signal_new ("changed", gucharmap_chapters_get_type (), G_SIGNAL_RUN_FIRST,
                    G_STRUCT_OFFSET (GucharmapChaptersClass, changed),
                    NULL, NULL, _gucharmap_marshal_VOID__VOID, G_TYPE_NONE, 0);
}

GType 
gucharmap_chapters_get_type (void)
{
  static GType t = 0;

  if (t == 0)
    {
      static const GTypeInfo type_info =
        {
          sizeof (GucharmapChaptersClass),
          NULL,
          NULL,
          (GClassInitFunc) gucharmap_chapters_class_init,
          NULL,
          NULL,
          sizeof (GucharmapChapters),
          0,
          (GInstanceInitFunc) gucharmap_chapters_init,
          NULL
        };

      t = g_type_register_static (GTK_TYPE_SCROLLED_WINDOW, "GucharmapChapters", &type_info, 0);
    }

  return t;
}

/**
 * gucharmap_chapters_get_codepoint_list:
 * @chapters: a #GucharmapChapters
 *
 * Creates a new #GucharmapCodepointList representing the characters in the
 * current chapter.
 *
 * Return value: the newly-created #GucharmapCodepointList, or NULL if
 * there is no chapter selected. The caller should release the result with
 * g_object_unref() when finished.
 **/
GucharmapCodepointList * 
gucharmap_chapters_get_codepoint_list (GucharmapChapters *chapters)
{
  g_return_val_if_fail (IS_GUCHARMAP_CHAPTERS (chapters), NULL);

  return GUCHARMAP_CHAPTERS_GET_CLASS (chapters)->get_codepoint_list (chapters);
}

/**
 * gucharmap_chapters_get_codepoint_list:
 * @chapters: a #GucharmapChapters
 *
 * Return value: a #GucharmapCodepointList representing all the characters
 * in all the chapters. It should not be modified or freed.
 **/
G_CONST_RETURN GucharmapCodepointList * 
gucharmap_chapters_get_book_codepoint_list (GucharmapChapters *chapters)
{
  g_return_val_if_fail (IS_GUCHARMAP_CHAPTERS (chapters), NULL);

  return GUCHARMAP_CHAPTERS_GET_CLASS (chapters)->get_book_codepoint_list (chapters);
}

/**
 * gucharmap_chapters_go_to_character:
 * @chapters: a #GucharmapChapters
 * @wc: a character
 *
 * Return value: %TRUE on success, %FALSE on failure.
 **/
gboolean
gucharmap_chapters_go_to_character (GucharmapChapters *chapters, 
                                    gunichar           wc)
{
  g_return_val_if_fail (IS_GUCHARMAP_CHAPTERS (chapters), FALSE);

  return GUCHARMAP_CHAPTERS_GET_CLASS (chapters)->go_to_character (chapters, wc);
}

/**
 * gucharmap_chapters_next:
 * @chapters: a #GucharmapChapters
 *
 * Moves to the next chapter if applicable.
 **/
void
gucharmap_chapters_next (GucharmapChapters *chapters)
{
  GtkTreeSelection *selection = gtk_tree_view_get_selection (GTK_TREE_VIEW (chapters->tree_view));
  GtkTreeIter iter;

  if (gtk_tree_selection_get_selected (selection, NULL, &iter))
    if (gtk_tree_model_iter_next (chapters->tree_model, &iter))
      {
        GtkTreePath *path = gtk_tree_model_get_path (chapters->tree_model, &iter);
        gtk_tree_view_set_cursor (GTK_TREE_VIEW (chapters->tree_view), path, NULL, FALSE);
        gtk_tree_path_free (path);
      }
}

/**
 * gucharmap_chapters_next:
 * @chapters: a #GucharmapChapters
 *
 * Moves to the previous chapter if applicable.
 **/
void
gucharmap_chapters_previous (GucharmapChapters *chapters)
{
  GtkTreeSelection *selection = gtk_tree_view_get_selection (GTK_TREE_VIEW (chapters->tree_view));
  GtkTreeIter iter;

  if (gtk_tree_selection_get_selected (selection, NULL, &iter))
    {
      GtkTreePath *path = gtk_tree_model_get_path (chapters->tree_model, &iter);
      if (gtk_tree_path_prev (path))
        gtk_tree_view_set_cursor (GTK_TREE_VIEW (chapters->tree_view), path, NULL, FALSE);
      gtk_tree_path_free (path);
    }
}
