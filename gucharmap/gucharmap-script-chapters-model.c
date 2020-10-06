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
#include <gtk/gtk.h>
#include <stdlib.h>
#include <string.h>

#include "gucharmap.h"
#include "gucharmap-private.h"

static gboolean
is_special_script (const char *script)
{
  return g_str_equal (script, "Common") ||
    g_str_equal (script, "Inherited") ||
    g_str_equal (script, "Unknown");
}

static void
gucharmap_script_chapters_model_init (GucharmapScriptChaptersModel *model)
{
  GucharmapChaptersModel *chapters_model = GUCHARMAP_CHAPTERS_MODEL (model);
  GtkListStore *store = GTK_LIST_STORE (model);
  const gchar **unicode_scripts;
  GtkTreeIter iter;
  guint i;
  GType types[] = {
    G_TYPE_STRING,
    G_TYPE_STRING,
    PANGO_TYPE_ATTR_LIST,
  };
  PangoAttrList *attr_list;

  attr_list = pango_attr_list_new ();
  pango_attr_list_insert (attr_list, pango_attr_style_new (PANGO_STYLE_ITALIC));

  gtk_list_store_set_column_types (store, G_N_ELEMENTS (types), types);

  unicode_scripts = gucharmap_unicode_list_scripts ();
  for (i = 0;  unicode_scripts[i]; i++)
    {
      gtk_list_store_append (store, &iter);
      gtk_list_store_set (store, &iter,
                          GUCHARMAP_CHAPTERS_MODEL_COLUMN_ID, unicode_scripts[i],
                          GUCHARMAP_CHAPTERS_MODEL_COLUMN_LABEL, _(unicode_scripts[i]),
                          _GUCHARMAP_CHAPTERS_MODEL_COLUMN_LABEL_ATTRS,
                                  is_special_script (unicode_scripts[i]) ? attr_list : NULL,
                          -1);
    }
  g_free (unicode_scripts);
  pango_attr_list_unref (attr_list);

  chapters_model->priv->sort_column = GUCHARMAP_CHAPTERS_MODEL_COLUMN_LABEL;
}

static GucharmapCodepointList *
get_codepoint_list (GucharmapChaptersModel *chapters,
                    GtkTreeIter *iter)
{
  GtkTreeModel *model = GTK_TREE_MODEL (chapters);
  GucharmapCodepointList *list;
  gchar *script_untranslated;

  gtk_tree_model_get (model, iter, GUCHARMAP_CHAPTERS_MODEL_COLUMN_ID, &script_untranslated, -1);

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

static gboolean
append_script (GtkTreeModel                 *model,
               GtkTreePath                  *path,
               GtkTreeIter                  *iter,
               GucharmapScriptCodepointList *list)
{
  gchar *script_untranslated;

  gtk_tree_model_get (model, iter, GUCHARMAP_CHAPTERS_MODEL_COLUMN_ID, &script_untranslated, -1);

  gucharmap_script_codepoint_list_append_script (list, script_untranslated);

  return FALSE;
}

static GucharmapCodepointList *
get_book_codepoint_list (GucharmapChaptersModel *chapters)
{
  GucharmapChaptersModelPrivate *priv = chapters->priv;

  if (priv->book_list == NULL)
    {
      GtkTreeModel *model = GTK_TREE_MODEL (chapters);

      priv->book_list = gucharmap_script_codepoint_list_new ();
      gtk_tree_model_foreach (model, (GtkTreeModelForeachFunc) append_script, priv->book_list);
    }

  return g_object_ref (priv->book_list);
}

static gboolean
character_to_iter (GucharmapChaptersModel *chapters_model,
                   gunichar           wc,
                   GtkTreeIter       *iter)
{
  const char *script;

  script = gucharmap_unicode_get_script_for_char (wc);
  if (script == NULL)
    return FALSE;

  return gucharmap_chapters_model_id_to_iter (chapters_model, script, iter);
}

static void
gucharmap_script_chapters_model_class_init (GucharmapScriptChaptersModelClass *clazz)
{
  GucharmapChaptersModelClass *chapters_class = GUCHARMAP_CHAPTERS_MODEL_CLASS (clazz);

  _gucharmap_intl_ensure_initialized ();

  chapters_class->title = _("Script");
  chapters_class->character_to_iter = character_to_iter;
  chapters_class->get_codepoint_list = get_codepoint_list;
  chapters_class->get_book_codepoint_list = get_book_codepoint_list;
}

G_DEFINE_TYPE (GucharmapScriptChaptersModel, gucharmap_script_chapters_model, GUCHARMAP_TYPE_CHAPTERS_MODEL)

GucharmapChaptersModel *
gucharmap_script_chapters_model_new (void)
{
  return g_object_new (gucharmap_script_chapters_model_get_type (), NULL);
}
