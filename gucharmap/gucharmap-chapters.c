/* $Id$ */
/*
 * Copyright (c) 2004 Noah Levitt
 * Copyright (C) 2007 Christian Persch
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
#include "gucharmap-chapters-model.h"
#include "gucharmap-chapters-view.h"
#include "gucharmap-marshal.h"
#include "gucharmap-intl.h"
#include <string.h>

enum
{
  CHANGED,
  NUM_SIGNALS
};

enum
{
  PROP_0,
  PROP_CHAPTERS_MODEL
};

static guint gucharmap_chapters_signals[NUM_SIGNALS];
static void gucharmap_chapters_class_init (GucharmapChaptersClass *klass);
static void gucharmap_chapters_init       (GucharmapChapters *chapters);
static GObject *gucharmap_chapters_constructor (GType type,
                                                guint n_construct_properties,
                                                GObjectConstructParam *construct_params);
static void gucharmap_chapters_set_property (GObject *object,
                                             guint prop_id,
                                             const GValue *value,
                                             GParamSpec *pspec);

G_DEFINE_TYPE (GucharmapChapters, gucharmap_chapters, GTK_TYPE_SCROLLED_WINDOW)

static void
selection_changed (GtkTreeSelection  *selection,
                   GucharmapChapters *chapters)
{
  GtkTreeIter iter;

  /* this should always return true */
  if (gtk_tree_selection_get_selected (selection, NULL, &iter))
    /* XXX: parent should have api to do this I guess */
    g_signal_emit (chapters, gucharmap_chapters_signals[CHANGED], 0);
/*
  if ((chapter = gucharmap_chapter_get_string (GUCHARMAP_CHAPTERS (chapters))))
    {
      gucharmap_settings_set_chapter (chapter);
      g_free (chapter);
    }*/
}

static GucharmapCodepointList * 
default_get_codepoint_list (GucharmapChapters *chapters)
{
  return gucharmap_chapters_view_get_codepoint_list (GUCHARMAP_CHAPTERS_VIEW (chapters->tree_view));
}

static const GucharmapCodepointList *
default_get_book_codepoint_list (GucharmapChapters *chapters)
{
  return gucharmap_chapters_view_get_book_codepoint_list (GUCHARMAP_CHAPTERS_VIEW (chapters->tree_view));
}

static gboolean
default_go_to_character (GucharmapChapters *chapters,
                         gunichar wc)
{
  return gucharmap_chapters_view_select_character (GUCHARMAP_CHAPTERS_VIEW (chapters->tree_view), wc);
}

static void
gucharmap_chapters_init (GucharmapChapters *chapters)
{
  gtk_scrolled_window_set_policy (GTK_SCROLLED_WINDOW (chapters),
                                  GTK_POLICY_NEVER, GTK_POLICY_AUTOMATIC);
  gtk_scrolled_window_set_shadow_type (GTK_SCROLLED_WINDOW (chapters), GTK_SHADOW_ETCHED_IN);

  chapters->tree_view = gucharmap_chapters_view_new ();
}

static GObject *
gucharmap_chapters_constructor (GType type,
                                guint n_construct_properties,
                                GObjectConstructParam *construct_params)
{
  GObject *object;
  GucharmapChapters *chapters;
  GtkTreeSelection *selection;

  object = G_OBJECT_CLASS (gucharmap_chapters_parent_class)->constructor
            (type, n_construct_properties, construct_params);
  chapters = GUCHARMAP_CHAPTERS (object);

  gtk_container_add (GTK_CONTAINER (chapters), chapters->tree_view);
  gtk_widget_show (chapters->tree_view);

  selection = gtk_tree_view_get_selection (GTK_TREE_VIEW (chapters->tree_view));
  g_signal_connect (selection, "changed", G_CALLBACK (selection_changed), chapters);
    
  return object;
}

static void
gucharmap_chapters_set_property (GObject *object,
                                 guint prop_id,
                                 const GValue *value,
                                 GParamSpec *pspec)
{
  GucharmapChapters *chapters = GUCHARMAP_CHAPTERS (object);
  switch (prop_id) {
    case PROP_CHAPTERS_MODEL:
      gucharmap_chapters_set_model (chapters, g_value_get_object (value));
      break;
    default:
      G_OBJECT_WARN_INVALID_PROPERTY_ID (object, prop_id, pspec);
      break;
  }
}

static void
gucharmap_chapters_class_init (GucharmapChaptersClass *clazz)
{
  GObjectClass *object_class = G_OBJECT_CLASS (clazz);

  _gucharmap_intl_ensure_initialized ();

  object_class->set_property = gucharmap_chapters_set_property;
  object_class->constructor = gucharmap_chapters_constructor;

  clazz->get_codepoint_list = default_get_codepoint_list;
  clazz->get_book_codepoint_list = default_get_book_codepoint_list;
  clazz->go_to_character = default_go_to_character;

  gucharmap_chapters_signals[CHANGED] =
      g_signal_new ("changed", gucharmap_chapters_get_type (), G_SIGNAL_RUN_FIRST,
                    G_STRUCT_OFFSET (GucharmapChaptersClass, changed),
                    NULL, NULL, g_cclosure_marshal_VOID__VOID, G_TYPE_NONE, 0);

  g_object_class_install_property (object_class,
                                   PROP_CHAPTERS_MODEL,
                                   g_param_spec_object ("chapters-model", NULL, NULL,
                                                        GUCHARMAP_TYPE_CHAPTERS_MODEL,
                                                        G_PARAM_WRITABLE | G_PARAM_CONSTRUCT | G_PARAM_STATIC_NAME | G_PARAM_STATIC_NICK | G_PARAM_STATIC_BLURB));
}

/**
 * gucharmap_chapters_new_with_model:
 * @model: a #GucharmapChaptersModel
 *
 * Returns: a new #GucharmapChapters using @model
 */
GtkWidget *
gucharmap_chapters_new_with_model (GucharmapChaptersModel *model)
{
  return g_object_new (GUCHARMAP_TYPE_CHAPTERS,
                       "hadjustment", NULL,
                       "vadjustment", NULL,
                       "chapters-model", model,
                       NULL);
}

void
gucharmap_chapters_set_model (GucharmapChapters *chapters,
                              GucharmapChaptersModel *model)
{
  chapters->tree_model = GTK_TREE_MODEL (model);

  gucharmap_chapters_view_set_model (GUCHARMAP_CHAPTERS_VIEW (chapters->tree_view), model);
}

GucharmapChaptersModel *
gucharmap_chapters_get_model (GucharmapChapters *chapters)
{
  return GUCHARMAP_CHAPTERS_MODEL (chapters->tree_model);
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
  gucharmap_chapters_view_next (GUCHARMAP_CHAPTERS_VIEW (chapters->tree_view));
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
  gucharmap_chapters_view_previous (GUCHARMAP_CHAPTERS_VIEW (chapters->tree_view));
}

/**
 * gucharmap_chapter_get_string:
 * @chapters: a #GucharmapChapters
 *
 * Returns a newly allocated string containing
 * the name of the currently selected chapter
 **/
gchar*
gucharmap_chapter_get_string (GucharmapChapters *chapters)
{
  return gucharmap_chapters_view_get_selected (GUCHARMAP_CHAPTERS_VIEW (chapters->tree_view));
}

/**
 * gucharmap_chapter_get_string:
 * @chapters: a #GucharmapChapters
 * @name: Unicode block name
 *
 * Sets the selection to the row specified by @name
 * Return value: %TRUE on success, %FALSE on failure
 **/
gboolean
gucharmap_chapter_set_string (GucharmapChapters *chapters,
                              const char        *name)
{
  return gucharmap_chapters_view_set_selected (GUCHARMAP_CHAPTERS_VIEW (chapters->tree_view), name);
}
