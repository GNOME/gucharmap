/* $Id$ */
/*
 * Copyright (c) 2003  Noah Levitt <nlevitt аt users.sourceforge.net>
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

#ifdef HAVE_CONFIG_H
# include <config.h>
#endif

#include <gtk/gtk.h>
#include <gdk/gdkkeysyms.h>
#include <stdlib.h>

#include <charmap.h>
#include <unicode_info.h>
#include <gucharmap_intl.h>
#include <gucharmap_marshal.h>
#include <chartable_accessible.h>
#include <chartable.h>

/* 0x100, a standard increment for paging unicode */
#define BLOCK_SIZE 256

/* only the label is visible in the block selector */
enum 
{
  BLOCK_SELECTOR_LABEL = 0,
  BLOCK_SELECTOR_UC_START,
  BLOCK_SELECTOR_UNICODE_BLOCK,
  BLOCK_SELECTOR_NUM_COLUMNS
};

enum 
{
  CAPTION_LABEL = 0,
  CAPTION_VALUE,
  CAPTION_NUM_COLUMNS
};

enum 
{
  STATUS_MESSAGE = 0,
  NUM_SIGNALS
};

static gchar **caption_labels;

static guint charmap_signals[NUM_SIGNALS] = { 0 };


/* return value is read-only, should not be freed */
static const gchar *
unichar_to_printable_utf8 (gunichar uc)
{
  static gchar buf[12];
  gint x;

  /* http://www.cl.cam.ac.uk/~mgk25/unicode.html#utf-8 --"Also note that
   * the code positions U+D800 to U+DFFF (UTF-16 surrogates) as well as
   * U+FFFE and U+FFFF must not occur in normal UTF-8" */
  if (! g_unichar_validate (uc) || (! g_unichar_isgraph (uc) 
              && g_unichar_type (uc) != G_UNICODE_PRIVATE_USE))
    return "";
  
  /* Unicode Standard 3.2, section 2.6, "By convention, diacritical marks
   * used by the Unicode Standard may be exhibited in (apparent) isolation
   * by applying them to U+0020 SPACE or to U+00A0 NO BREAK SPACE." */

  /* 17:10 < owen> noah: I'm *not* claiming that what Pango does currently
   *               is right, but convention isn't a requirement. I think
   *               it's probably better to do the Uniscribe thing and put
   *               the lone combining mark on a dummy character and require
   *               ZWJ
   * 17:11 < noah> owen: do you mean that i should put a ZWJ in there, or
   *               that pango will do that?
   * 17:11 < owen> noah: I mean, you should (assuming some future more
   *               capable version of Pango) put it in there
   */

  if (g_unichar_type (uc) == G_UNICODE_COMBINING_MARK
      || g_unichar_type (uc) == G_UNICODE_ENCLOSING_MARK
      || g_unichar_type (uc) == G_UNICODE_NON_SPACING_MARK)
    {
      buf[0] = ' ';
      buf[1] = '\xe2'; /* ZERO */ 
      buf[2] = '\x80'; /* WIDTH */
      buf[3] = '\x8d'; /* JOINER (0x200D) */
      x = g_unichar_to_utf8 (uc, buf+4);
      buf[x+4] = '\0';
    }
  else
    {
      x = g_unichar_to_utf8 (uc, buf);
      buf[x] = '\0';
    }

  return buf;
}


static void
status_message (Charmap *charmap, const gchar *message)
{
  g_signal_emit (charmap, charmap_signals[STATUS_MESSAGE], 0, message);
}


/* ucs is terminated with (gunichar)(-1). Return value should be freed with
 * free_array_of_strings */
static gchar **
make_array_of_char_descs (gunichar *ucs)
{
  gchar **descs;
  gint i;

  if (ucs == NULL)
    return NULL;

  /* count them and allocate the array */
  for (i = 0;  ucs[i] != (gunichar)(-1);  i++);
  descs = g_malloc ((i+1) * sizeof (gchar *));

  for (i = 0;  ucs[i] != (gunichar)(-1);  i++)
    descs[i] = g_strdup_printf ("\342\200\252%s [U+%4.4X %s]",
                                unichar_to_printable_utf8 (ucs[i]), 
                                ucs[i], get_unicode_name (ucs[i]));

  descs[i] = NULL;
  return descs;
}


static void
free_array_of_strings (gchar **strs)
{
  gint i;

  for (i = 0;  strs[i] != NULL;  i++)
    g_free (strs[i]);

  g_free (strs);
}


/* sets the value for the referenced row */
static void 
set_caption_value (GtkTreeStore *tree_store, 
                   GtkTreeRowReference *rowref, 
                   const gchar *value)
{
  GtkTreeIter iter;

  gtk_tree_model_get_iter (GTK_TREE_MODEL (tree_store), &iter, 
                           gtk_tree_row_reference_get_path (rowref));
  gtk_tree_store_set (tree_store, &iter, 
                      CAPTION_VALUE, value, -1);
}


/* wraps the string in <LTRE>%s<PDF> */
static void
set_caption_value_ltr (GtkTreeStore *tree_store, 
                       GtkTreeRowReference *rowref, 
                       const gchar *value)
{
  gchar *temp;

  if (value == NULL)
    set_caption_value (tree_store, rowref, NULL);
  else
    {
      temp = g_strdup_printf ("\342\200\252%s\342\200\254", value);
      set_caption_value (tree_store, rowref, temp);
      g_free (temp);
    }
}


/* makes child rows of the referenced row if there is more than one value;
 * values is null-terminated; also deletes extra child rows if there are
 * any */
/* algo:
 *   if (values == null)
 *     set_caption (null)
 *   else
 *     {
 *       set_row_value (values[0]);
 *       child_iter = gtk_tree_model_iter_children ();
 *
 *       for (i = 1;  values[i] != NULL;  i++)
 *         {
 *           if (no child_iter)
 *             make_child_iter;
 *
 *           set_row_value (values[0])
 *         }
 *     }
 *
 *   if (child_iter is not set)
 *     child_iter = gtk_tree_model_iter_children ();
 *
 *   delete remaining children
 */
static void
set_caption_values (GtkTreeStore *tree_store, 
                    GtkTreeRowReference *rowref, 
                    const gchar **values)
{
  GtkTreeIter iter, child_iter;
  gboolean have_another_row = FALSE;
  gint i;

  gtk_tree_model_get_iter (GTK_TREE_MODEL (tree_store), &iter, 
                           gtk_tree_row_reference_get_path (rowref));

  if (values == NULL)
    {
      gtk_tree_store_set (tree_store, &iter, 
                          CAPTION_VALUE, NULL, -1);
      have_another_row = gtk_tree_model_iter_children (
                             GTK_TREE_MODEL (tree_store), &child_iter, &iter);
    }
  else
    {
      gtk_tree_store_set (tree_store, &iter, 
                          CAPTION_VALUE, values[0], -1);

      have_another_row = gtk_tree_model_iter_children (
                             GTK_TREE_MODEL (tree_store), &child_iter, &iter);

      for (i = 1;  values[i] != NULL;  i++)
        {
          if (! have_another_row)
            gtk_tree_store_append (tree_store, &child_iter, &iter);

          gtk_tree_store_set (tree_store, &child_iter, 
                              CAPTION_VALUE, values[i], -1);

          have_another_row = gtk_tree_model_iter_next (
                                 GTK_TREE_MODEL (tree_store), &child_iter);
        }
    }

  /* delete remaining rows */
  while (have_another_row)
    {
      GtkTreeIter temp_iter = child_iter;
      have_another_row = gtk_tree_model_iter_next (GTK_TREE_MODEL (tree_store),
                                                   &temp_iter);

      gtk_tree_store_remove (tree_store, &child_iter);

      child_iter = temp_iter;
    }

}


static void
set_caption_values_ltr (GtkTreeStore *tree_store, 
                        GtkTreeRowReference *rowref, 
                        const gchar **values)
{
  gchar **ltr_values;
  gint i;

  if (values == NULL)
    {
      set_caption_values (tree_store, rowref, NULL);
      return;
    }

  /* count them and allocate the array */
  for (i = 0;  values[i] != NULL;  i++);
  ltr_values = g_malloc ((i+1) * sizeof (gchar *));

  for (i = 0;  values[i] != NULL;  i++)
    ltr_values[i] = g_strdup_printf ("\342\200\252%s\342\200\254", values[i]);
  ltr_values[i] = NULL;

  set_caption_values (tree_store, rowref, (const gchar **) ltr_values);

  free_array_of_strings (ltr_values);
}


/* \342\200\216 is U+200E LEFT-TO-RIGHT MARK */
/* \342\200\217 is U+200F RIGHT-TO-LEFT MARK */
/* \342\200\252 is U+202A LEFT-TO-RIGHT EMBEDDING */
/* \342\200\254 is U+202C POP DIRECTIONAL FORMATTING */
static void
set_caption (Charmap *charmap, gunichar uc)
{
  guchar ubuf[7];
  gint i, n;

  /* codepoint and name */
  if (charmap->caption_rows[CHARMAP_CAPTION_CHARACTER])
    {
      gchar *temp = g_strdup_printf ("\342\200\252U+%4.4X %s\342\200\254", uc, 
                                     get_unicode_name (uc));
      set_caption_value (charmap->caption_model, 
                         charmap->caption_rows[CHARMAP_CAPTION_CHARACTER],
                         temp);
      g_free (temp);
    }

  /* category */
  if (charmap->caption_rows[CHARMAP_CAPTION_CATEGORY])
    {
      set_caption_value (charmap->caption_model, 
                         charmap->caption_rows[CHARMAP_CAPTION_CATEGORY],
                         get_unicode_category_name (uc));
    }

  /* utf-8 */
  if (charmap->caption_rows[CHARMAP_CAPTION_UTF8])
    {
      GString *gstemp = g_string_new (NULL);
      n = g_unichar_to_utf8 (uc, ubuf);
      for (i = 0;  i < n;  i++)
        g_string_append_printf (gstemp, "0x%2.2X\342\200\217 ", ubuf[i]);

      set_caption_value (charmap->caption_model, 
                         charmap->caption_rows[CHARMAP_CAPTION_UTF8],
                         gstemp->str);

      g_string_free (gstemp, TRUE);
    }

  /* other representations (for C, html) */
  if (charmap->caption_rows[CHARMAP_CAPTION_OTHER_REPS])
    {
      GString *gstemp = g_string_new ("\342\200\252");
      n = g_unichar_to_utf8 (uc, ubuf);
      for (i = 0;  i < n;  i++)
        g_string_append_printf (gstemp, "\\%3.3o", ubuf[i]);
      g_string_append_printf (gstemp, "\342\200\254\t");
      g_string_append_printf (gstemp, "\342\200\252&#%d;\342\200\254", uc);

      set_caption_value (charmap->caption_model, 
                         charmap->caption_rows[CHARMAP_CAPTION_OTHER_REPS],
                         gstemp->str);

      g_string_free (gstemp, TRUE);
    }

  /* decomposition */
  if (charmap->caption_rows[CHARMAP_CAPTION_DECOMPOSITION])
    {
      GString *gstemp;
      gunichar *decomposition;
      gsize result_len;

      decomposition = unicode_canonical_decomposition (uc,
                                                       &result_len);
      gstemp = g_string_new ("\342\200\252");
      g_string_append_printf (gstemp, "%s [U+%4.4X]", 
                              unichar_to_printable_utf8 (decomposition[0]), 
                              decomposition[0]);
      for (i = 1;  i < result_len;  i++)
        g_string_append_printf (gstemp, 
                                " + %s [U+%4.4X]", 
                                unichar_to_printable_utf8 (decomposition[i]), 
                                decomposition[i]);

      set_caption_value (charmap->caption_model, 
                         charmap->caption_rows[CHARMAP_CAPTION_DECOMPOSITION],
                         gstemp->str);

      g_free (decomposition);
      g_string_free (gstemp, TRUE);
    }

#if ENABLE_UNIHAN
  /* kDefinition */
  if (charmap->caption_rows[CHARMAP_CAPTION_KDEFINITION])
    set_caption_value_ltr (charmap->caption_model, 
                           charmap->caption_rows[CHARMAP_CAPTION_KDEFINITION],
                           get_unicode_kDefinition (uc));

  /* kMandarin */
  if (charmap->caption_rows[CHARMAP_CAPTION_KMANDARIN])
    set_caption_value_ltr (charmap->caption_model, 
                           charmap->caption_rows[CHARMAP_CAPTION_KMANDARIN],
                           get_unicode_kMandarin (uc));

  /* kJapaneseOn */
  if (charmap->caption_rows[CHARMAP_CAPTION_KJAPANESEON])
    set_caption_value_ltr (charmap->caption_model, 
                           charmap->caption_rows[CHARMAP_CAPTION_KJAPANESEON],
                           get_unicode_kJapaneseOn (uc));

  /* kJapaneseKun */
  if (charmap->caption_rows[CHARMAP_CAPTION_KJAPANESEKUN])
    set_caption_value_ltr (charmap->caption_model, 
                           charmap->caption_rows[CHARMAP_CAPTION_KJAPANESEKUN],
                           get_unicode_kJapaneseKun (uc));

  /* kCantonese */
  if (charmap->caption_rows[CHARMAP_CAPTION_KCANTONESE])
    set_caption_value_ltr (charmap->caption_model, 
                           charmap->caption_rows[CHARMAP_CAPTION_KCANTONESE],
                           get_unicode_kCantonese (uc));

  /* kTang */
  if (charmap->caption_rows[CHARMAP_CAPTION_KTANG])
    set_caption_value_ltr (charmap->caption_model, 
                           charmap->caption_rows[CHARMAP_CAPTION_KTANG],
                           get_unicode_kTang (uc));

  /* kKorean */
  if (charmap->caption_rows[CHARMAP_CAPTION_KKOREAN])
    set_caption_value_ltr (charmap->caption_model, 
                           charmap->caption_rows[CHARMAP_CAPTION_KKOREAN],
                           get_unicode_kKorean (uc));
#endif /* #if ENABLE_UNIHAN */


  /* nameslist stars */
  if (charmap->caption_rows[CHARMAP_CAPTION_STARS])
    {
      const gchar **stars = get_nameslist_stars (uc);

      set_caption_values_ltr (charmap->caption_model, 
                              charmap->caption_rows[CHARMAP_CAPTION_STARS],
                              stars);

      if (stars)
        g_free (stars);
    }

  /* nameslist pounds */
  if (charmap->caption_rows[CHARMAP_CAPTION_POUNDS])
    {
      const gchar **pounds = get_nameslist_pounds (uc);

      set_caption_values_ltr (charmap->caption_model, 
                              charmap->caption_rows[CHARMAP_CAPTION_POUNDS],
                              pounds);

      if (pounds)
        g_free (pounds);
    }

  /* nameslist equals */
  if (charmap->caption_rows[CHARMAP_CAPTION_EQUALS])
    {
      const gchar **equals = get_nameslist_equals (uc);

      set_caption_values_ltr (charmap->caption_model, 
                              charmap->caption_rows[CHARMAP_CAPTION_EQUALS],
                              equals);

      if (equals)
        g_free (equals);
    }

  /* nameslist colons */
  if (charmap->caption_rows[CHARMAP_CAPTION_COLONS])
    {
      const gchar **colons = get_nameslist_colons (uc);

      set_caption_values_ltr (charmap->caption_model, 
                              charmap->caption_rows[CHARMAP_CAPTION_COLONS],
                              colons);

      if (colons)
        g_free (colons);
    }

  /* nameslist exes */
  if (charmap->caption_rows[CHARMAP_CAPTION_EXES])
    {
      gunichar *exes = get_nameslist_exes (uc);
      gchar **values;

      values = make_array_of_char_descs (exes);

      set_caption_values (charmap->caption_model, 
                          charmap->caption_rows[CHARMAP_CAPTION_EXES],
                          (const gchar **) values);

      if (values)
        free_array_of_strings (values);

      if (exes)
        g_free (exes);
    }
}


/* XXX: linear search (but N is small) */
static GtkTreePath *
find_block_index_tree_path (Charmap *charmap, gunichar uc)
{
  gint i;

  for (i = 0;  i < charmap->block_index_size;  i++)
    if (charmap->block_index[i].start > uc)
      break;

  return charmap->block_index[i-1].tree_path;
}


/* selects the active block in the block selector tree view based on the
 * active character */
static void
set_active_block (Charmap *charmap, gunichar uc)
{
  GtkTreePath *parent = NULL;
  GtkTreePath *tree_path;
  
  tree_path = find_block_index_tree_path (charmap, uc);

  /* block our "changed" handler */
  g_signal_handler_block (G_OBJECT (charmap->block_selection), 
                          charmap->block_selection_changed_handler_id);

  if (gtk_tree_path_get_depth (tree_path) == 2)
    {
      parent = gtk_tree_path_copy (tree_path);
      gtk_tree_path_up (parent);

      if (! gtk_tree_view_row_expanded (
              GTK_TREE_VIEW (charmap->block_selector_view), parent))
        tree_path = parent;
    }

  gtk_tree_view_set_cursor (GTK_TREE_VIEW (charmap->block_selector_view),
                            tree_path, NULL, FALSE);

  g_signal_handler_unblock (G_OBJECT (charmap->block_selection),
                            charmap->block_selection_changed_handler_id);

  if (parent != NULL)
    gtk_tree_path_free (parent);
}


static void
block_selection_changed (GtkTreeSelection *selection, 
                         gpointer user_data)
{
  GtkTreeModel *model;
  GtkTreeIter iter;
  Charmap *charmap;
  gunichar uc_start;

  charmap = CHARMAP (user_data);

  if (gtk_tree_selection_get_selected (selection, &model, &iter))
    {
      gtk_tree_model_get (model, &iter, BLOCK_SELECTOR_UC_START, 
                          &uc_start, -1);

      chartable_set_active_character (charmap->chartable, uc_start);
    }
}


/* makes the list of unicode blocks and code points */
static GtkWidget *
make_unicode_block_selector (Charmap *charmap)
{
  GtkWidget *scrolled_window;
  GtkTreeIter iter;
  GtkTreeIter child_iter;
  GtkCellRenderer *cell;
  GtkTreeViewColumn *column;
  gchar buf[12];
  gunichar uc;
  gint i, bi;

  scrolled_window = gtk_scrolled_window_new (NULL, NULL);

  gtk_scrolled_window_set_policy (GTK_SCROLLED_WINDOW (scrolled_window),
                                  GTK_POLICY_NEVER, GTK_POLICY_AUTOMATIC);
  gtk_scrolled_window_set_shadow_type (GTK_SCROLLED_WINDOW (scrolled_window), 
                                       GTK_SHADOW_ETCHED_IN);

  charmap->block_selector_model = gtk_tree_store_new (
          BLOCK_SELECTOR_NUM_COLUMNS, G_TYPE_STRING, 
          G_TYPE_UINT, G_TYPE_POINTER);

  /* UNICHAR_MAX / BLOCK_SIZE is U+XXXX blocks, count_blocks is named blocks */
  charmap->block_index_size = (UNICHAR_MAX / BLOCK_SIZE) + 2
                              + count_blocks (UNICHAR_MAX);
  charmap->block_index = g_malloc (charmap->block_index_size 
                                   * sizeof (BlockIndex));
  bi = 0;

  for (i = 0;  unicode_blocks[i].start != (gunichar)(-1)
               && unicode_blocks[i].start <= UNICHAR_MAX;  i++)
    {
      gtk_tree_store_append (charmap->block_selector_model, &iter, NULL);
      gtk_tree_store_set (charmap->block_selector_model, &iter, 
                          BLOCK_SELECTOR_LABEL, unicode_blocks[i].name, 
                          BLOCK_SELECTOR_UC_START, unicode_blocks[i].start,
                          BLOCK_SELECTOR_UNICODE_BLOCK, &(unicode_blocks[i]),
                          -1);
      charmap->block_index[bi].start = unicode_blocks[i].start;
      charmap->block_index[bi].tree_path = gtk_tree_model_get_path (
              GTK_TREE_MODEL (charmap->block_selector_model), &iter);
      bi++;

      if (unicode_blocks[i].start % BLOCK_SIZE == 0)
        uc = unicode_blocks[i].start;
      else
        uc = unicode_blocks[i].start + BLOCK_SIZE 
            - (unicode_blocks[i].start % BLOCK_SIZE);

      /* U+0000, U+0100, etc */
      for ( ; uc >= unicode_blocks[i].start && uc <= unicode_blocks[i].end 
              && uc <= UNICHAR_MAX;  uc += BLOCK_SIZE) 
        {
          g_snprintf (buf, sizeof (buf), "U+%4.4X", uc);
	  gtk_tree_store_append (charmap->block_selector_model, 
                                 &child_iter, &iter);
	  gtk_tree_store_set (charmap->block_selector_model, &child_iter, 
                              BLOCK_SELECTOR_LABEL, buf, 
                              BLOCK_SELECTOR_UC_START, uc, 
                              BLOCK_SELECTOR_UNICODE_BLOCK, NULL, -1);
          charmap->block_index[bi].start = uc;
          charmap->block_index[bi].tree_path = gtk_tree_model_get_path (
                  GTK_TREE_MODEL (charmap->block_selector_model), &child_iter);
          bi++;
        }
    }

  /* terminate value that is bigger than the biggest character */
  charmap->block_index[bi].start = UNICHAR_MAX + 1;
  charmap->block_index[bi].tree_path = NULL;

  /* we have the model, now make the view */
  charmap->block_selector_view = gtk_tree_view_new_with_model (
          GTK_TREE_MODEL (charmap->block_selector_model));
  charmap->block_selection = gtk_tree_view_get_selection (
          GTK_TREE_VIEW (charmap->block_selector_view));
  gtk_tree_view_set_headers_visible (
          GTK_TREE_VIEW (charmap->block_selector_view), FALSE);

  cell = gtk_cell_renderer_text_new ();
  column = gtk_tree_view_column_new_with_attributes (NULL, cell, 
                                                     "text", 0, NULL);

  gtk_tree_view_append_column (GTK_TREE_VIEW (charmap->block_selector_view),
                               GTK_TREE_VIEW_COLUMN (column));

  gtk_tree_selection_set_mode (charmap->block_selection, GTK_SELECTION_BROWSE);
  charmap->block_selection_changed_handler_id = g_signal_connect (
          G_OBJECT (charmap->block_selection), "changed", 
          G_CALLBACK (block_selection_changed), charmap);

  gtk_container_add (GTK_CONTAINER (scrolled_window), 
                     charmap->block_selector_view);

  gtk_widget_show_all (scrolled_window);

  return scrolled_window;
}


static GtkWidget *
make_caption (Charmap *charmap)
{
  GtkWidget *tree_view;
  GtkTreeViewColumn *column;
  GtkCellRenderer *cell;
  GtkWidget *scrolled_window;
  gint ypad;

  charmap->caption_model = gtk_tree_store_new (CAPTION_NUM_COLUMNS,
                                               G_TYPE_STRING, 
                                               G_TYPE_STRING);

  /* now make the tree view */
  tree_view = gtk_tree_view_new_with_model (
          GTK_TREE_MODEL (charmap->caption_model));
  gtk_tree_view_set_headers_visible (GTK_TREE_VIEW (tree_view), FALSE);

  cell = gtk_cell_renderer_text_new ();
  column = gtk_tree_view_column_new_with_attributes (NULL, cell,
                                                     "text", CAPTION_LABEL,
                                                     NULL);
  g_object_set (G_OBJECT (cell), "weight", PANGO_WEIGHT_BOLD, NULL);
  gtk_tree_view_append_column (GTK_TREE_VIEW (tree_view), column);

  cell = gtk_cell_renderer_text_new ();
  column = gtk_tree_view_column_new_with_attributes (NULL, cell,
                                                     "text", CAPTION_VALUE,
                                                     NULL);
  gtk_tree_view_append_column (GTK_TREE_VIEW (tree_view), column);

  /* make it "editable" so the value can be copied and pasted */
  g_object_set (G_OBJECT (cell), "editable", TRUE, NULL); 

  /* give each row a few more pixels vertically */
  g_object_get (G_OBJECT (cell), "ypad", &ypad, NULL);
  ypad += 2;
  g_object_set (G_OBJECT (cell), "ypad", ypad, NULL);

  /* do this so it doesn't manically change size */
  gtk_cell_renderer_text_set_fixed_height_from_font (
          GTK_CELL_RENDERER_TEXT (cell), 1);

  scrolled_window = gtk_scrolled_window_new (NULL, NULL);
  gtk_scrolled_window_add_with_viewport (GTK_SCROLLED_WINDOW (scrolled_window),
                                         tree_view);
  gtk_scrolled_window_set_policy (GTK_SCROLLED_WINDOW (scrolled_window),
                                  GTK_POLICY_ALWAYS, GTK_POLICY_NEVER);

  charmap_show_caption (charmap, CHARMAP_CAPTION_CHARACTER);

  gtk_widget_show_all (scrolled_window);
  gtk_widget_hide (scrolled_window);

  return scrolled_window;
}


void
charmap_class_init (CharmapClass *clazz)
{
  clazz->status_message = NULL;

  charmap_signals[STATUS_MESSAGE] =
      g_signal_new ("status-message", charmap_get_type (), G_SIGNAL_RUN_FIRST,
                    G_STRUCT_OFFSET (CharmapClass, status_message),
                    NULL, NULL, gucharmap_marshal_VOID__STRING, G_TYPE_NONE, 
		    1, G_TYPE_STRING);
}


void
active_char_set (GtkWidget *widget, gunichar uc, Charmap *charmap)
{
  set_caption (charmap, uc);
  set_active_block (charmap, uc);
}


/* does all the initial construction */
void
charmap_init (Charmap *charmap)
{
  GtkWidget *hpaned;
  GtkWidget *vbox;
  GtkWidget *caption;
  GtkWidget *temp;
  AtkObject *accessib;

  accessib = gtk_widget_get_accessible (GTK_WIDGET (charmap));
  atk_object_set_name (accessib, _("Character Map"));

  /* init the caption labels */
  caption_labels = g_malloc (CHARMAP_CAPTION_COUNT * sizeof (gchar *));
  caption_labels[CHARMAP_CAPTION_CHARACTER] =  _("Character"),
  caption_labels[CHARMAP_CAPTION_CATEGORY] =  _("Unicode category"), 
  caption_labels[CHARMAP_CAPTION_DECOMPOSITION] =  _("Canonical decomposition"), 
  caption_labels[CHARMAP_CAPTION_UTF8] =  _("UTF-8"), 
  caption_labels[CHARMAP_CAPTION_OTHER_REPS] =  _("Other representations"), 
  caption_labels[CHARMAP_CAPTION_EXES] = _("See also");
  caption_labels[CHARMAP_CAPTION_COLONS] = _("Equivalents");
  caption_labels[CHARMAP_CAPTION_EQUALS] = _("Alias names");
  caption_labels[CHARMAP_CAPTION_POUNDS] = _("Approximate equivalents");
  caption_labels[CHARMAP_CAPTION_STARS] = _("Notes");
#if ENABLE_UNIHAN
  caption_labels[CHARMAP_CAPTION_KDEFINITION] =  _("CJK ideograph definition"), 
  caption_labels[CHARMAP_CAPTION_KMANDARIN] =  _("Mandarin pronunciation"), 
  caption_labels[CHARMAP_CAPTION_KJAPANESEON] =  _("Japanese On pronunciation"), 
  caption_labels[CHARMAP_CAPTION_KJAPANESEKUN] =  _("Japanese Kun pronunciation"), 
  caption_labels[CHARMAP_CAPTION_KCANTONESE] =  _("Cantonese pronunciation"), 
  caption_labels[CHARMAP_CAPTION_KTANG] =  _("Tang pronunciation"), 
  caption_labels[CHARMAP_CAPTION_KKOREAN] =  _("Korean pronunciation"), 
#endif

  gtk_box_set_spacing (GTK_BOX (charmap), 6);
  gtk_container_set_border_width (GTK_CONTAINER (charmap), 6);

  /* vbox for charmap and caption */
  vbox = gtk_vbox_new (FALSE, 6);
  charmap->chartable = CHARTABLE (chartable_new ());
  g_signal_connect (G_OBJECT (charmap->chartable), "set-active-char", 
                    G_CALLBACK (active_char_set), charmap);
  g_signal_connect_swapped (G_OBJECT (charmap->chartable), "status-message", 
                            G_CALLBACK (status_message), charmap);
  gtk_box_pack_start (GTK_BOX (vbox), GTK_WIDGET (charmap->chartable), 
                      TRUE, TRUE, 0);

  caption = make_caption (charmap);
  accessib = gtk_widget_get_accessible (caption);
  atk_object_set_name (accessib, _("Details on the Current Character"));
  gtk_widget_show (caption);
  gtk_box_pack_start (GTK_BOX (vbox), caption, FALSE, FALSE, 0);
  gtk_widget_show (vbox);
  /* end vbox for charmap and caption*/

  /* the panes */
  hpaned = gtk_hpaned_new ();

  temp = make_unicode_block_selector (charmap);
  accessib = gtk_widget_get_accessible (temp);
  atk_object_set_name (accessib, _("List of Unicode Blocks"));

  gtk_paned_pack1 (GTK_PANED (hpaned), temp, FALSE, TRUE);
  gtk_paned_pack2 (GTK_PANED (hpaned), vbox, TRUE, TRUE);
  /* done with panes */

  /* start packing stuff in the outer vbox (the Charmap itself) */
  gtk_box_pack_start (GTK_BOX (charmap), hpaned, TRUE, TRUE, 0);
  gtk_widget_show (hpaned);
  /* end packing stuff in the outer vbox (the Charmap itself) */

  set_caption (charmap, charmap->chartable->active_char);
  set_active_block (charmap, charmap->chartable->active_char);
}


GtkWidget *
charmap_new (void)
{
  return GTK_WIDGET (g_object_new (charmap_get_type (), NULL));
}


GType
charmap_get_type (void)
{
  static GType charmap_type = 0;

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
        sizeof (Charmap),
        0,              /* n_preallocs */
        (GInstanceInitFunc) charmap_init,
      };

      charmap_type = g_type_register_static (GTK_TYPE_VBOX, "Charmap", 
                                             &charmap_info, 0);
    }

  return charmap_type;
}


void 
charmap_set_font (Charmap *charmap, const gchar *font_name)
{
  chartable_set_font (charmap->chartable, font_name);
}


void
charmap_identify_clipboard (Charmap *charmap, GtkClipboard *clipboard)
{
  chartable_identify_clipboard (charmap->chartable, clipboard);
}


void 
charmap_expand_block_selector (Charmap *charmap)
{
  gtk_tree_view_expand_all (GTK_TREE_VIEW (charmap->block_selector_view));

  /* have to send it an expose event or the change won't happen right away */
  gtk_widget_queue_draw (gtk_widget_get_parent (charmap->block_selector_view));
}


void 
charmap_collapse_block_selector (Charmap *charmap)
{
  gtk_tree_view_collapse_all (GTK_TREE_VIEW (charmap->block_selector_view));

  /* have to send it an expose event or the change won't happen right away */
  gtk_widget_queue_draw (gtk_widget_get_parent (charmap->block_selector_view));
}


void
charmap_go_to_character (Charmap *charmap, gunichar uc)
{
  gchar *message;

  if (uc >= 0 && uc <= UNICHAR_MAX)
    {
      chartable_set_active_character (charmap->chartable, uc);
      message = g_strdup_printf ("Jumped to U+%4.4X.", uc);
      status_message (charmap, message);
      g_free (message);
    }
}


CharmapSearchResult
charmap_search (Charmap *charmap, const gchar *search_text)
{
  gunichar uc;
  CharmapSearchResult result;

  if (search_text[0] == '\0')
    return CHARMAP_NOTHING_TO_SEARCH_FOR;
  
  uc = find_next_substring_match (charmap->chartable->active_char, 
                                  UNICHAR_MAX, search_text);
  if (uc != (gunichar)(-1) && uc <= UNICHAR_MAX)
    {
      if (uc <= charmap->chartable->active_char)
        result = CHARMAP_WRAPPED;
      else
        result = CHARMAP_FOUND;

      chartable_set_active_character (charmap->chartable, uc);
    }
  else
    result = CHARMAP_NOT_FOUND;

  return result;
}


/* captions appear in numerical (by CharmapCaption value) order */
static gint
compute_position_to_insert_at (Charmap *charmap, 
                               CharmapCaption caption_id)
{
  CharmapCaption i;
  gint position;

  for (i = 0, position = 0;  i < caption_id;  i++)
    if (charmap->caption_rows[i])
      position++;

  return position;
}



void
charmap_show_caption (Charmap *charmap, CharmapCaption caption_id)
{
  GtkTreeIter iter;
  GtkTreeModel *model;

  model = GTK_TREE_MODEL (charmap->caption_model);

  if (charmap->caption_rows[caption_id] == NULL)
    {
      gtk_tree_store_insert (charmap->caption_model, &iter, NULL, 
                             compute_position_to_insert_at (charmap, 
                                                            caption_id));

      charmap->caption_rows[caption_id] = gtk_tree_row_reference_new (
              model, gtk_tree_model_get_path (model, &iter));

      gtk_tree_store_set (charmap->caption_model, &iter, CAPTION_LABEL, 
                          caption_labels[caption_id], -1);
    }

  set_caption (charmap, charmap->chartable->active_char);
}


void
charmap_hide_caption (Charmap *charmap, CharmapCaption caption_id)
{
  GtkTreeIter iter;
  GtkTreeModel *model;

  model = GTK_TREE_MODEL (charmap->caption_model);

  if (charmap->caption_rows[caption_id])
    {
      gtk_tree_model_get_iter (model, &iter, 
                               gtk_tree_row_reference_get_path (
                                   charmap->caption_rows[caption_id]));
      gtk_tree_store_remove (charmap->caption_model, &iter);

      gtk_tree_row_reference_free (charmap->caption_rows[caption_id]);
      charmap->caption_rows[caption_id] = NULL;
    }
}


void
charmap_zoom_enable (Charmap *charmap)
{
  chartable_zoom_enable (charmap->chartable);
}


void
charmap_zoom_disable (Charmap *charmap)
{
  chartable_zoom_disable (charmap->chartable);
}


Chartable *
charmap_get_chartable (Charmap *charmap)
{
  return charmap->chartable;
}


