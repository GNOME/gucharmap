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

#include <gucharmap/gucharmap.h>
#include "gucharmap_intl.h"
#include "gucharmap_marshal.h"
#include "chartable_accessible.h"


/* 0x100, a standard increment for paging unicode */
#define PAGE_SIZE 256

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
  STATUS_MESSAGE = 0,
  NUM_SIGNALS
};

static guint gucharmap_charmap_signals[NUM_SIGNALS] = { 0 };


static void
status_message (GucharmapCharmap *charmap, const gchar *message)
{
  g_signal_emit (charmap, gucharmap_charmap_signals[STATUS_MESSAGE], 
                 0, message);
}


/* XXX: linear search (but N is small) */
static GtkTreePath *
find_block_index_tree_path (GucharmapCharmap *charmap, gunichar uc)
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
set_active_block (GucharmapCharmap *charmap, gunichar uc)
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
  GucharmapCharmap *charmap;
  gunichar uc_start;

  charmap = GUCHARMAP_CHARMAP (user_data);

  if (gtk_tree_selection_get_selected (selection, &model, &iter))
    {
      gtk_tree_model_get (model, &iter, BLOCK_SELECTOR_UC_START, 
                          &uc_start, -1);

      gucharmap_table_set_active_character (charmap->chartable, uc_start);
    }
}


/* makes the list of unicode blocks and code points */
static GtkWidget *
make_unicode_block_selector (GucharmapCharmap *charmap)
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

  /* UNICHAR_MAX / PAGE_SIZE is U+XXXX blocks, gucharmap_count_blocks is named blocks */
  charmap->block_index_size = (UNICHAR_MAX / PAGE_SIZE) + 2
                              + gucharmap_count_blocks (UNICHAR_MAX);
  charmap->block_index = g_malloc (charmap->block_index_size 
                                   * sizeof (BlockIndex));
  bi = 0;

  for (i = 0;  gucharmap_unicode_blocks[i].start != (gunichar)(-1)
               && gucharmap_unicode_blocks[i].start <= UNICHAR_MAX;  i++)
    {
      gtk_tree_store_append (charmap->block_selector_model, &iter, NULL);
      gtk_tree_store_set (charmap->block_selector_model, &iter, 
                          BLOCK_SELECTOR_LABEL, 
                          _(gucharmap_unicode_blocks[i].name),
                          BLOCK_SELECTOR_UC_START, 
                          gucharmap_unicode_blocks[i].start,
                          BLOCK_SELECTOR_UNICODE_BLOCK, 
                          &(gucharmap_unicode_blocks[i]),
                          -1);
      charmap->block_index[bi].start = gucharmap_unicode_blocks[i].start;
      charmap->block_index[bi].tree_path = gtk_tree_model_get_path (
              GTK_TREE_MODEL (charmap->block_selector_model), &iter);
      bi++;

      if (gucharmap_unicode_blocks[i].start % PAGE_SIZE == 0)
        uc = gucharmap_unicode_blocks[i].start;
      else
        uc = gucharmap_unicode_blocks[i].start + PAGE_SIZE 
            - (gucharmap_unicode_blocks[i].start % PAGE_SIZE);

      /* U+0000, U+0100, etc */
      for ( ; uc >= gucharmap_unicode_blocks[i].start 
              && uc <= gucharmap_unicode_blocks[i].end 
              && uc <= UNICHAR_MAX;  
           uc += PAGE_SIZE) 
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


void
gucharmap_charmap_class_init (GucharmapCharmapClass *clazz)
{
  clazz->status_message = NULL;

  gucharmap_charmap_signals[STATUS_MESSAGE] =
      g_signal_new ("status-message", gucharmap_charmap_get_type (), 
                    G_SIGNAL_RUN_FIRST,
                    G_STRUCT_OFFSET (GucharmapCharmapClass, status_message),
                    NULL, NULL, gucharmap_marshal_VOID__STRING, G_TYPE_NONE, 
		    1, G_TYPE_STRING);
}


/* returns NULL if the character decomposes into itself; otherwise returns
 * a GString which should be g_string_freed by the caller */
static GString *
get_canonical_decomposition (GucharmapCharmap *charmap,
                             gunichar uc)
{
  GString *gs;
  gunichar *decomposition;
  gsize result_len;
  gint i;

  decomposition = gucharmap_unicode_canonical_decomposition (uc, 
                                                             &result_len);
  if (result_len == 1)
    {
      g_free (decomposition);
      return NULL;
    }

  gs = g_string_new (_("Canonical decomposition: \t"));

  g_string_append_printf (gs, "U+%4.4X %s", decomposition[0], 
                          gucharmap_get_unicode_name (decomposition[0]));

  for (i = 1;  i < result_len;  i++)
    g_string_append_printf (gs, " + U+%4.4X %s", decomposition[i], 
                            gucharmap_get_unicode_name (decomposition[i]));

  g_string_append_c (gs, '\n');

  g_free (decomposition);

  return gs;
}


/* returns NULL if there are no exes; otherwise returns a GString which
 * should be g_string_freed by the caller */
static GString *
get_see_also (GucharmapCharmap *charmap,
              gunichar uc)
{
  GString *gs;
  gunichar *exes = gucharmap_get_nameslist_exes (uc);
  gint i;

  if (exes == NULL)
    return NULL;

  gs = g_string_new (NULL);
  
  g_string_append_printf (gs, _("See also: \t\t\t\t\tU+%4.4X %s\n"), 
                          exes[0], gucharmap_get_unicode_name (exes[0]));
  for (i = 1;  exes[i] != (gunichar)(-1);  i++)
    g_string_append_printf (gs, _("\t\t\t\t\t\tU+%4.4X %s\n"), 
                            exes[i], gucharmap_get_unicode_name (exes[i]));

  g_free (exes);

  return gs;
}

/* returns NULL if there are no equals; otherwise returns a GString which
 * should be g_string_freed by the caller */
static GString *
get_alias_names (GucharmapCharmap *charmap,
                 gunichar uc)
{
  const gchar **equals = gucharmap_get_nameslist_equals (uc);
  GString *gs;
  gint i;

  if (equals == NULL)
    return NULL;

  gs = g_string_new (NULL);

  g_string_append_printf (gs, _("Alias names: \t\t\t\t%s\n"), equals[0]);
  for (i = 1;  equals[i] != NULL;  i++)
    g_string_append_printf (gs, _("\t\t\t\t\t\t%s\n"), equals[i]);

  g_free (equals);

  return gs;
}


/* returns NULL if there are no stars; otherwise returns a GString which
 * should be g_string_freed by the caller */
static GString *
get_notes (GucharmapCharmap *charmap,
           gunichar uc)
{
  const gchar **stars = gucharmap_get_nameslist_stars (uc);
  GString *gs;
  gint i;

  if (stars == NULL)
    return NULL;

  gs = g_string_new (NULL);

  g_string_append_printf (gs, _("Notes: \t\t\t\t\t%s\n"), stars[0]);
  for (i = 1;  stars[i] != NULL;  i++)
    g_string_append_printf (gs, _("\t\t\t\t\t\t%s\n"), stars[i]);

  g_free (stars);

  return gs;
}


/* returns NULL if there are no pounds; otherwise returns a GString which
 * should be g_string_freed by the caller */
static GString *
get_approximate_equivalents (GucharmapCharmap *charmap,
                             gunichar uc)
{
  const gchar **pounds = gucharmap_get_nameslist_pounds (uc);
  GString *gs;
  gint i;

  if (pounds == NULL)
    return NULL;

  gs = g_string_new (NULL);

  g_string_append_printf (gs, _("Approximate equivalents: \t%s\n"), 
                          pounds[0]);
  for (i = 1;  pounds[i] != NULL;  i++)
    g_string_append_printf (gs, _("\t\t\t\t\t\t%s\n"), pounds[i]);

  g_free (pounds);

  return gs;
}


/* returns NULL if there are no colons; otherwise returns a GString which
 * should be g_string_freed by the caller */
static GString *
get_equivalents (GucharmapCharmap *charmap,
                 gunichar uc)
{
  const gchar **colons = gucharmap_get_nameslist_colons (uc);
  GString *gs;
  gint i;

  if (colons == NULL)
    return NULL;

  gs = g_string_new (NULL);

  g_string_append_printf (gs, _("Equivalents: \t\t\t%s\n"), 
                          colons[0]);
  for (i = 1;  colons[i] != NULL;  i++)
    g_string_append_printf (gs, _("\t\t\t\t\t\t\t%s\n"), colons[i]);

  g_free (colons);

  return gs;
}


static void
set_details (GucharmapCharmap *charmap,
             gunichar uc)
{
  GtkTextBuffer *buffer;
  GtkTextIter iter;
  GString *gstemp;
  gchar *temp;
  gchar buf[12];
  guchar ubuf[7];
  gint n, i;

  buffer = gtk_text_view_get_buffer (GTK_TEXT_VIEW (charmap->details));
  gtk_text_buffer_set_text (buffer, "", -1);

  gtk_text_buffer_get_start_iter (buffer, &iter);

  n = gucharmap_unichar_to_printable_utf8 (uc, buf);
  if (n == 0)
    gtk_text_buffer_insert_with_tags_by_name (
            buffer, &iter, _("[not a printable character]"), -1, NULL);
  else
    {
      gtk_text_buffer_insert_with_tags_by_name (buffer, &iter, 
                                                buf, n, 
                                                "gimongous", NULL);
    }

  gtk_text_buffer_insert (buffer, &iter, "\n\n", -1);
                                             
  /* character name */
  temp = g_strdup_printf ("U+%4.4X %s\n\n", 
                          uc, gucharmap_get_unicode_name (uc));
  gtk_text_buffer_insert_with_tags_by_name (buffer, &iter, temp, -1,
                                            "big", "bold", NULL);
  g_free (temp);

  gtk_text_buffer_insert_with_tags_by_name (
          buffer, &iter, _("General Character Properties\n\n"), -1, 
          "bold", NULL);

  /* character category */
  temp = g_strdup_printf (_("Unicode category: \t\t\t%s\n"), 
                          gucharmap_get_unicode_category_name (uc));
  gtk_text_buffer_insert (buffer, &iter, temp, -1);
  g_free (temp);

  /* canonical decomposition */
  gstemp = get_canonical_decomposition (charmap, uc);
  if (gstemp != NULL)
    {
      gtk_text_buffer_insert (buffer, &iter, gstemp->str, gstemp->len);
      g_string_free (gstemp, TRUE);
    }


  n = g_unichar_to_utf8 (uc, ubuf);

  gtk_text_buffer_insert (buffer, &iter, "\n\n", -1);
  gtk_text_buffer_insert_with_tags_by_name (
          buffer, &iter, _("Various Useful Representations\n\n"), -1, 
          "bold", NULL);

  /* UTF-8 */
  gstemp = g_string_new (_("UTF-8: \t\t\t\t\t"));
  for (i = 0;  i < n;  i++)
    g_string_append_printf (gstemp, "0x%2.2X ", ubuf[i]);
  g_string_append_c (gstemp, '\n');
  gtk_text_buffer_insert (buffer, &iter, gstemp->str, gstemp->len);
  g_string_free (gstemp, TRUE);

  /* octal \012\234 UTF-8 */
  gstemp = g_string_new (_("Octal escaped UTF-8: \t\t"));
  for (i = 0;  i < n;  i++)
    g_string_append_printf (gstemp, "\\%3.3o", ubuf[i]);
  g_string_append_c (gstemp, '\n');
  gtk_text_buffer_insert (buffer, &iter, gstemp->str, gstemp->len);
  g_string_free (gstemp, TRUE);

  /* entity reference */
  temp = g_strdup_printf (_("Decimal entity reference: \t&#%d;\n"), 
                          uc);
  gtk_text_buffer_insert (buffer, &iter, temp, -1);
  g_free (temp);

  gtk_text_buffer_insert (buffer, &iter, "\n\n", -1);
  gtk_text_buffer_insert_with_tags_by_name (
          buffer, &iter, _("Annotations and Cross References\n\n"), -1, 
          "bold", NULL);

  /* nameslist equals (alias names) */
  gstemp = get_alias_names (charmap, uc);
  if (gstemp != NULL)
    {
      gtk_text_buffer_insert (buffer, &iter, gstemp->str, gstemp->len);
      g_string_free (gstemp, TRUE);
    }

  /* nameslist stars (notes) */
  gstemp = get_notes (charmap, uc);
  if (gstemp != NULL)
    {
      gtk_text_buffer_insert (buffer, &iter, gstemp->str, gstemp->len);
      g_string_free (gstemp, TRUE);
    }

  /* nameslist exes (see also) */
  gstemp = get_see_also (charmap, uc);
  if (gstemp != NULL)
    {
      gtk_text_buffer_insert (buffer, &iter, gstemp->str, gstemp->len);
      g_string_free (gstemp, TRUE);
    }

  /* nameslist pounds (approximate equivalents) */
  gstemp = get_approximate_equivalents (charmap, uc);
  if (gstemp != NULL)
    {
      gtk_text_buffer_insert (buffer, &iter, gstemp->str, gstemp->len);
      g_string_free (gstemp, TRUE);
    }

  /* nameslist colons (equivalents) */
  gstemp = get_equivalents (charmap, uc);
  if (gstemp != NULL)
    {
      gtk_text_buffer_insert (buffer, &iter, gstemp->str, gstemp->len);
      g_string_free (gstemp, TRUE);
    }
}


static void
active_char_set (GtkWidget *widget, 
                 gunichar uc, 
                 GucharmapCharmap *charmap)
{
  gchar *temp;

  set_active_block (charmap, uc);
  set_details (charmap, uc);

  temp = g_strdup_printf ("U+%4.4X %s", uc, gucharmap_get_unicode_name (uc));
  status_message (charmap, temp);
  g_free (temp);
}


static GtkWidget *
make_chartable_page (GucharmapCharmap *charmap)
{
  GtkWidget *hpaned;
  GtkWidget *block_selector;
  AtkObject *accessib;

  hpaned = gtk_hpaned_new ();
  gtk_widget_show (hpaned);

  charmap->chartable = GUCHARMAP_TABLE (gucharmap_table_new ());
  g_signal_connect (G_OBJECT (charmap->chartable), "set-active-char", 
                    G_CALLBACK (active_char_set), charmap);
  g_signal_connect_swapped (G_OBJECT (charmap->chartable), "status-message", 
                            G_CALLBACK (status_message), charmap);

  block_selector = make_unicode_block_selector (charmap);
  accessib = gtk_widget_get_accessible (block_selector);
  atk_object_set_name (accessib, _("List of Unicode Blocks"));

  gtk_paned_pack1 (GTK_PANED (hpaned), block_selector, FALSE, TRUE);
  gtk_paned_pack2 (GTK_PANED (hpaned), GTK_WIDGET (charmap->chartable), 
                   TRUE, TRUE);

  return hpaned;
}


/* this creates all the named text tags we'll be using in set_details */
static void
create_tags (GucharmapCharmap *charmap)
{
  GtkTextBuffer *buffer;
  gint default_font_size;

  buffer = gtk_text_view_get_buffer (GTK_TEXT_VIEW (charmap->details));

  default_font_size = pango_font_description_get_size (
	  GTK_WIDGET (charmap)->style->font_desc);

  gtk_text_buffer_create_tag (buffer, "gimongous", 
	                      "size", 8 * default_font_size, 
			      NULL);
  gtk_text_buffer_create_tag (buffer, "bold",
	                      "weight", PANGO_WEIGHT_BOLD,
			      NULL);
  gtk_text_buffer_create_tag (buffer, "big",
                              "size", default_font_size * 5 / 4,
                              NULL);
}


static GtkWidget *
make_details_page (GucharmapCharmap *charmap)
{
  GtkWidget *scrolled_window;

  scrolled_window = gtk_scrolled_window_new (NULL, NULL);
  gtk_widget_show (scrolled_window);
  gtk_scrolled_window_set_policy (GTK_SCROLLED_WINDOW (scrolled_window),
                                  GTK_POLICY_AUTOMATIC, GTK_POLICY_AUTOMATIC);

  charmap->details = gtk_text_view_new ();
  gtk_widget_show (charmap->details);
  /* gtk_text_view_set_editable (GTK_TEXT_VIEW (charmap->details), FALSE); */

  create_tags (charmap);

  gtk_container_add (GTK_CONTAINER (scrolled_window), charmap->details);

  return scrolled_window;
}


/* does all the initial construction */
void
gucharmap_charmap_init (GucharmapCharmap *charmap)
{
  AtkObject *accessib;

  accessib = gtk_widget_get_accessible (GTK_WIDGET (charmap));
  atk_object_set_name (accessib, _("Character Map"));

  gtk_notebook_append_page (
          GTK_NOTEBOOK (charmap), make_chartable_page (charmap), 
          gtk_label_new_with_mnemonic (_("Characte_r Table")));
  gtk_notebook_append_page (
          GTK_NOTEBOOK (charmap), make_details_page (charmap), 
          gtk_label_new_with_mnemonic (_("Character _Details")));

  set_active_block (charmap, charmap->chartable->active_char);
  set_details (charmap, charmap->chartable->active_char);
}


GtkWidget *
gucharmap_charmap_new (void)
{
  return GTK_WIDGET (g_object_new (gucharmap_charmap_get_type (), NULL));
}


GType
gucharmap_charmap_get_type (void)
{
  static GType gucharmap_charmap_type = 0;

  if (!gucharmap_charmap_type)
    {
      static const GTypeInfo gucharmap_charmap_info =
      {
        sizeof (GucharmapCharmapClass),
        NULL,           /* base_init */
        NULL,           /* base_finalize */
        (GClassInitFunc) gucharmap_charmap_class_init,
        NULL,           /* class_finalize */
        NULL,           /* class_data */
        sizeof (GucharmapCharmap),
        0,              /* n_preallocs */
        (GInstanceInitFunc) gucharmap_charmap_init,
      };

      gucharmap_charmap_type = g_type_register_static (GTK_TYPE_NOTEBOOK, 
                                                       "GucharmapCharmap", 
                                                       &gucharmap_charmap_info,
                                                       0);
    }

  return gucharmap_charmap_type;
}


void 
gucharmap_charmap_set_font (GucharmapCharmap *charmap, 
                            const gchar *font_name)
{
  gucharmap_table_set_font (charmap->chartable, font_name);
}


void
gucharmap_charmap_identify_clipboard (GucharmapCharmap *charmap, 
                                      GtkClipboard *clipboard)
{
  gucharmap_table_identify_clipboard (charmap->chartable, clipboard);
}


void 
gucharmap_charmap_expand_block_selector (GucharmapCharmap *charmap)
{
  gtk_tree_view_expand_all (GTK_TREE_VIEW (charmap->block_selector_view));

  /* have to send it an expose event or the change won't happen right away */
  gtk_widget_queue_draw (gtk_widget_get_parent (charmap->block_selector_view));
}


void 
gucharmap_charmap_collapse_block_selector (GucharmapCharmap *charmap)
{
  gtk_tree_view_collapse_all (GTK_TREE_VIEW (charmap->block_selector_view));

  /* have to send it an expose event or the change won't happen right away */
  gtk_widget_queue_draw (gtk_widget_get_parent (charmap->block_selector_view));
}


void
gucharmap_charmap_go_to_character (GucharmapCharmap *charmap, 
                                   gunichar uc)
{
  gchar *message;

  if (uc >= 0 && uc <= UNICHAR_MAX)
    {
      gucharmap_table_set_active_character (charmap->chartable, uc);
      message = g_strdup_printf ("Jumped to U+%4.4X.", uc);
      status_message (charmap, message);
      g_free (message);
    }
}


/* direction is +1 (forward) or -1 (backward) */
GucharmapSearchResult
gucharmap_charmap_search (GucharmapCharmap *charmap, 
                          const gchar *search_text, 
                          gint direction)
{
  gunichar uc;
  GucharmapSearchResult result;

  g_assert (direction == -1 || direction == 1);

  if (search_text[0] == '\0')
    return GUCHARMAP_NOTHING_TO_SEARCH_FOR;
  
  uc = gucharmap_find_substring_match (charmap->chartable->active_char, 
                             search_text, direction);
  if (uc != (gunichar)(-1) && uc <= UNICHAR_MAX)
    {
      if ((direction == 1 && uc <= charmap->chartable->active_char)
          || (direction == -1 && uc >= charmap->chartable->active_char))
        result = GUCHARMAP_WRAPPED;
      else
        result = GUCHARMAP_FOUND;

      gucharmap_table_set_active_character (charmap->chartable, uc);
    }
  else
    result = GUCHARMAP_NOT_FOUND;

  return result;
}


void
gucharmap_charmap_zoom_enable (GucharmapCharmap *charmap)
{
  gucharmap_table_zoom_enable (charmap->chartable);
}


void
gucharmap_charmap_zoom_disable (GucharmapCharmap *charmap)
{
  gucharmap_table_zoom_disable (charmap->chartable);
}


GucharmapTable *
gucharmap_charmap_get_chartable (GucharmapCharmap *charmap)
{
  return charmap->chartable;
}


