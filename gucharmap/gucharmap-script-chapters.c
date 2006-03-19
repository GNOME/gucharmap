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
#include <gtk/gtk.h>
#include <stdlib.h>
#include <string.h>
#include "gucharmap-intl.h"
#include "gucharmap-unicode-info.h"
#include "gucharmap-script-chapters.h"
#include "gucharmap-script-codepoint-list.h"
#include "gucharmap-chapters.h"

enum 
{
  SCRIPT_CHAPTERS_SCRIPT_TRANSLATED = 0,
  SCRIPT_CHAPTERS_SCRIPT_UNTRANSLATED,
  SCRIPT_CHAPTERS_NUM_COLUMNS
};

static void
selection_changed (GtkTreeSelection        *selection,
                   GucharmapScriptChapters *chapters)
{
  GtkTreeModel *model;
  GtkTreeIter iter;

  /* this should always return true */
  if (gtk_tree_selection_get_selected (selection, &model, &iter))
    /* XXX: parent should have api to do this I guess */
    g_signal_emit_by_name (GUCHARMAP_CHAPTERS (chapters), "changed", 0); 
}

static void
gucharmap_script_chapters_init (GucharmapScriptChapters *chapters)
{
  GucharmapChapters *parent = GUCHARMAP_CHAPTERS (chapters);
  const gchar **unicode_scripts = gucharmap_unicode_list_scripts ();
  GtkCellRenderer *cell;
  GtkTreeIter iter;
  GtkTreeViewColumn *column;
  GtkTreeSelection *selection;
  gint i;
  parent->tree_model = GTK_TREE_MODEL (gtk_list_store_new (SCRIPT_CHAPTERS_NUM_COLUMNS, G_TYPE_STRING, G_TYPE_STRING));

  for (i = 0;  unicode_scripts[i]; i++)
    {
      gtk_list_store_append (GTK_LIST_STORE (parent->tree_model), &iter);
      gtk_list_store_set (GTK_LIST_STORE (parent->tree_model), &iter, 
                          SCRIPT_CHAPTERS_SCRIPT_TRANSLATED, _(unicode_scripts[i]),
                          SCRIPT_CHAPTERS_SCRIPT_UNTRANSLATED, unicode_scripts[i],
                          -1);
    }

  parent->tree_view = gtk_tree_view_new_with_model (GTK_TREE_MODEL (parent->tree_model));

  cell = gtk_cell_renderer_text_new ();
  column = gtk_tree_view_column_new ();
  gtk_tree_view_column_pack_start (column, cell, FALSE);
  gtk_tree_view_column_set_title (column, _("Script"));
  gtk_tree_view_column_add_attribute (column, cell, "text", SCRIPT_CHAPTERS_SCRIPT_TRANSLATED);
  gtk_tree_view_append_column (GTK_TREE_VIEW (parent->tree_view), column);

  gtk_tree_sortable_set_sort_column_id (GTK_TREE_SORTABLE (parent->tree_model), 
                                        SCRIPT_CHAPTERS_SCRIPT_TRANSLATED, 
                                        GTK_SORT_ASCENDING);

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
      GucharmapCodepointList *list;
      gchar *script_untranslated;

      gtk_tree_model_get (model, &iter, SCRIPT_CHAPTERS_SCRIPT_UNTRANSLATED, &script_untranslated, -1);

      list = gucharmap_script_codepoint_list_new ();
      if (!gucharmap_script_codepoint_list_set_script (GUCHARMAP_SCRIPT_CODEPOINT_LIST (list), script_untranslated))
        {
          g_error ("gucharmap_script_codepoint_list_set_script (\"%s\") failed\n", script_untranslated);
          /* not reached */
          return NULL;
        }

      g_free (script_untranslated);
      return list;
    }
  else
    return NULL;
}

static gboolean
append_script (GtkTreeModel                 *model,
               GtkTreePath                  *path,
               GtkTreeIter                  *iter,
               GucharmapScriptCodepointList *list)
{
  gchar *script_untranslated;

  gtk_tree_model_get (model, iter, SCRIPT_CHAPTERS_SCRIPT_UNTRANSLATED, &script_untranslated, -1);

  gucharmap_script_codepoint_list_append_script (list, script_untranslated);

  return FALSE;
}

static G_CONST_RETURN GucharmapCodepointList * 
get_book_codepoint_list (GucharmapChapters *chapters)
{
  if (chapters->book_list == NULL)
    {
      chapters->book_list = gucharmap_script_codepoint_list_new ();
      gtk_tree_model_foreach (chapters->tree_model, (GtkTreeModelForeachFunc) append_script, chapters->book_list);
    }

  return chapters->book_list;
}

static gboolean
go_to_character (GucharmapChapters *chapters, 
                 gunichar           wc)
{
  GucharmapChapters *parent = GUCHARMAP_CHAPTERS (chapters);
  GtkTreeSelection *selection;
  const gchar *script, *temp;
  GtkTreeIter iter;

  script = gucharmap_unicode_get_script_for_char (wc);
  if (script == NULL)
    return FALSE;

  if (!gtk_tree_model_get_iter_first (parent->tree_model, &iter))
    return FALSE;

  do
    {
      gtk_tree_model_get (parent->tree_model, &iter, SCRIPT_CHAPTERS_SCRIPT_UNTRANSLATED, &temp, -1);
      if (strcmp (script, temp) == 0)
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
  while (gtk_tree_model_iter_next (parent->tree_model, &iter));

  return FALSE;
}

static void
gucharmap_script_chapters_class_init (GucharmapScriptChaptersClass *clazz)
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
gucharmap_script_chapters_get_type (void)
{
  static GType t = 0;

  if (t == 0)
    {
      static const GTypeInfo type_info =
        {
          sizeof (GucharmapScriptChaptersClass),
          NULL,
          NULL,
          (GClassInitFunc) gucharmap_script_chapters_class_init,
          NULL,
          NULL,
          sizeof (GucharmapScriptChapters),
          0,
          (GInstanceInitFunc) gucharmap_script_chapters_init,
          NULL
        };

      t = g_type_register_static (gucharmap_chapters_get_type (), "GucharmapScriptChapters", &type_info, 0);
    }

  return t;
}

GtkWidget * 
gucharmap_script_chapters_new (void)
{
  return GTK_WIDGET (g_object_new (gucharmap_script_chapters_get_type (), NULL));
}

