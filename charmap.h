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

#ifndef CHARMAP_H
#define CHARMAP_H

#include <gtk/gtk.h>

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */


#define CHARMAP(obj)         GTK_CHECK_CAST (obj, charmap_get_type (), Charmap)
#define CHARMAP_CLASS(clazz) GTK_CHECK_CLASS_CAST (clazz, charmap_get_type (),\
                                                   CharmapClass)
#define IS_CHARMAP(obj)      GTK_CHECK_TYPE (obj, charmap_get_type ())

#define CHARMAP_MIN_ROWS 4
#define CHARMAP_MIN_COLS 4

/* largest legal unicode character */
/* #define UNICHAR_MAX 0x0010ffff  XXX: gtk has problems */
#define UNICHAR_MAX 0x0000ffff

#define TEXT_TO_COPY_MAXLENGTH 4096

/* 0x100, a standard increment for paging unicode */
#define BLOCK_SIZE 256


typedef struct _Charmap Charmap;
typedef struct _CharmapClass CharmapClass;

typedef struct _Caption Caption;


/* the columns in the tree view (only the first is visible */
enum 
{
  BLOCK_SELECTOR_LABEL = 0,
  BLOCK_SELECTOR_UC_START,
  BLOCK_SELECTOR_UNICODE_BLOCK,
  BLOCK_SELECTOR_NUM_COLUMNS
};


typedef struct 
{
  gunichar start;
  GtkTreePath *tree_path;
} 
block_index_t;


struct _Charmap
{
  GtkVBox parent;

  /* rows and columns on a page */
  gint rows, cols;

  GtkWidget *chartable;         /* GtkDrawingArea */
  GdkPixmap *chartable_pixmap; 

  gchar *font_name;
  PangoFontMetrics *font_metrics;
  PangoLayout *pango_layout;

  gunichar page_first_char;  /* the character in the upper left square */
  gunichar active_char;

  /* helps us know what to redraw */
  gunichar old_page_first_char;
  gunichar old_active_char;

  Caption *caption;

  GtkWidget *text_to_copy;

  /* the unicode block selection list */
  GtkTreeSelection *block_selection;
  GtkTreeStore *block_selector_model;
  GtkWidget *block_selector_view;
  gulong block_selection_changed_handler_id; 

  block_index_t *block_index;
  gint block_index_size;

  /* for the scrollbar */
  GtkObject *adjustment; 
  gulong adjustment_changed_handler_id; 

  /* search */
  GtkWidget *search_entry;
  GtkWidget *jump_entry;
};


struct _CharmapClass
{
  GtkVBoxClass parent_class;
};


struct _Caption
{
  /* labels */
  GtkWidget *codepoint;
  GtkWidget *character;
  GtkWidget *category;
  GtkWidget *name;
  GtkWidget *kDefinition;
  GtkWidget *kCantonese;
  GtkWidget *kKorean;
  GtkWidget *kJapaneseOn;
  GtkWidget *kJapaneseKun;
  GtkWidget *kTang;
  GtkWidget *kMandarin;
  GtkWidget *decomposition;
  GtkWidget *utf8;
};


GtkType charmap_get_type (void);
GtkWidget * charmap_new ();
void charmap_set_font (Charmap *charmap, gchar *font_name);
void charmap_set_geometry_hints (Charmap *charmap, GtkWindow *window);

#ifdef __cplusplus
}
#endif  /* #ifdef __cplusplus */

#endif  /* #ifndef CHARMAP_H */
