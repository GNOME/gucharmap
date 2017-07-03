/*
 * Copyright © 2004 Noah Levitt
 * Copyright © 2007 Christian Persch
 *
 * This program is free software; you can redistribute it and/or modify it
 * under the terms of the GNU General Public License as published by the
 * Free Software Foundation; either version 3 of the License, or (at your
 * option) any later version.
 *
 * This program is distributed in the hope that it will be useful, but
 * WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License along
 * with this program; if not, write to the Free Software Foundation, Inc.,
 * 51 Franklin Street, Fifth Floor, Boston, MA 02110-1335 USA
 */

#include <config.h>
#include <glib/gi18n-lib.h>

#include "gucharmap.h"
#include "gucharmap-private.h"

#include "unicode-blocks.h"

enum 
{
  BLOCK_CHAPTERS_MODEL_ID = 0,
  BLOCK_CHAPTERS_MODEL_LABEL,
  BLOCK_CHAPTERS_MODEL_UNICODE_BLOCK_PTR,
  BLOCK_CHAPTERS_MODEL_NUM_COLUMNS
};

static void
gucharmap_block_chapters_model_init (GucharmapBlockChaptersModel *model)
{
  GtkListStore *store = GTK_LIST_STORE (model);
  GtkTreeIter iter;
  guint i;
  GType types[] = {
    G_TYPE_STRING,
    G_TYPE_STRING,
    G_TYPE_POINTER
  };

  gtk_list_store_set_column_types (store, G_N_ELEMENTS (types), types);

  gtk_list_store_append (store, &iter);
  gtk_list_store_set (store, &iter,
                      BLOCK_CHAPTERS_MODEL_ID, "All",
                      BLOCK_CHAPTERS_MODEL_LABEL, _("All"), 
                      BLOCK_CHAPTERS_MODEL_UNICODE_BLOCK_PTR, NULL, 
                      -1);

  for (i = 0;  i < G_N_ELEMENTS (unicode_blocks); i++)
    {
      const char *block_name;

      block_name = unicode_blocks_strings + unicode_blocks[i].block_name_index;

      gtk_list_store_append (store, &iter);
      gtk_list_store_set (store, &iter,
                          BLOCK_CHAPTERS_MODEL_ID, block_name,
                          BLOCK_CHAPTERS_MODEL_LABEL, _(block_name),
                          BLOCK_CHAPTERS_MODEL_UNICODE_BLOCK_PTR, unicode_blocks + i,
                          -1);
    }
}

static GucharmapCodepointList *
get_codepoint_list (GucharmapChaptersModel *chapters,
                    GtkTreeIter *iter)
{
  GtkTreeModel *model = GTK_TREE_MODEL (chapters);
  UnicodeBlock *unicode_block;

  gtk_tree_model_get (model, iter, BLOCK_CHAPTERS_MODEL_UNICODE_BLOCK_PTR, &unicode_block, -1);

  /* special "All" block */
  if (unicode_block == NULL)
    return gucharmap_block_codepoint_list_new (0, UNICHAR_MAX);
    
  return gucharmap_block_codepoint_list_new (unicode_block->start, unicode_block->end);
}

static GucharmapCodepointList *
get_book_codepoint_list (GucharmapChaptersModel *model)
{
  GucharmapChaptersModelPrivate *model_priv = model->priv;

  if (model_priv->book_list == NULL) {
    model_priv->book_list = gucharmap_block_codepoint_list_new (0, UNICHAR_MAX);
  }

  return g_object_ref (model_priv->book_list);
}

/* XXX linear search */
static gboolean
character_to_iter (GucharmapChaptersModel *chapters,
                   gunichar           wc,
                   GtkTreeIter       *_iter)
{
  GtkTreeModel *model = GTK_TREE_MODEL (chapters);
  GtkTreeIter iter;

  if (wc > UNICHAR_MAX)
    return FALSE;

  /* skip "All" block */
  if (!gtk_tree_model_get_iter_first (model, &iter))
    return FALSE;

  while (gtk_tree_model_iter_next (model, &iter))
    {
      UnicodeBlock *unicode_block;
      gtk_tree_model_get (model, &iter, BLOCK_CHAPTERS_MODEL_UNICODE_BLOCK_PTR, &unicode_block, -1);
      if (wc >= unicode_block->start && wc <= unicode_block->end)
        {
          *_iter = iter;
          return TRUE;
        }
    }

  /* "All" is the last resort */
  return gtk_tree_model_get_iter_first (model, _iter);
}

static void
gucharmap_block_chapters_model_class_init (GucharmapBlockChaptersModelClass *clazz)
{
  GucharmapChaptersModelClass *chapters_class = GUCHARMAP_CHAPTERS_MODEL_CLASS (clazz);

  _gucharmap_intl_ensure_initialized ();

  chapters_class->title = _("Unicode Block");
  chapters_class->character_to_iter = character_to_iter;
  chapters_class->get_codepoint_list = get_codepoint_list;
  chapters_class->get_book_codepoint_list = get_book_codepoint_list;
}

G_DEFINE_TYPE (GucharmapBlockChaptersModel, gucharmap_block_chapters_model, GUCHARMAP_TYPE_CHAPTERS_MODEL)

GucharmapChaptersModel *
gucharmap_block_chapters_model_new (void)
{
  return g_object_new (gucharmap_block_chapters_model_get_type (), NULL);
}
