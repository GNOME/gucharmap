/* $Id$ */
/*
 * Copyright (c) 2003  Noah Levitt <nlevitt@users.sourceforge.net>
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

#ifdef HAVE_CONFIG_H
# include <config.h>
#endif

#include <gtk/gtk.h>

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */


#define CHARMAP(obj) (G_TYPE_CHECK_INSTANCE_CAST ((obj), charmap_get_type (), \
                                                  Charmap))

#define CHARMAP_CLASS(clazz) (G_TYPE_CHECK_CLASS_CAST ((clazz), \
                                                       charmap_get_type (),\
                                                       CharmapClass))

#define IS_CHARMAP(obj) (G_TYPE_CHECK_INSTANCE_TYPE ((obj), \
                         charmap_get_type ()))


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


typedef struct 
{
  gunichar start;
  GtkTreePath *tree_path;
} 
block_index_t;


typedef enum
{
  NOT_FOUND,
  FOUND,
  WRAPPED,
  NOTHING_TO_SEARCH_FOR
}
charmap_search_result_t;


struct _Charmap
{
  GtkVBox parent;

  /* rows and columns on a page */
  gint rows, cols;

  GtkWidget *chartable;         /* GtkDrawingArea */
  GdkPixmap *chartable_pixmap; 

  guint drag_begin_timeout_id;

  gchar *font_name;
  PangoFontMetrics *font_metrics;
  PangoLayout *pango_layout;

  gunichar page_first_char;  /* the character in the upper left square */
  gunichar active_char;

  /* helps us know what to redraw */
  gunichar old_page_first_char;
  gunichar old_active_char;

  Caption *caption;

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
};


struct _CharmapClass
{
  GtkVBoxClass parent_class;

  void (* activate) (Charmap *charmap);
  void (* status_message) (Charmap *charmap);
};


struct _Caption
{
  GtkTreeStore *caption_model;

  GtkTreeRowReference *codepoint;
  GtkTreeRowReference *category;
  GtkTreeRowReference *name;
  GtkTreeRowReference *decomposition;
  GtkTreeRowReference *utf8;

#if ENABLE_UNIHAN
  GtkTreeRowReference *kDefinition;
  GtkTreeRowReference *kCantonese;
  GtkTreeRowReference *kKorean;
  GtkTreeRowReference *kJapaneseOn;
  GtkTreeRowReference *kJapaneseKun;
  GtkTreeRowReference *kTang;
  GtkTreeRowReference *kMandarin;
#endif
};


GtkType charmap_get_type (void);
GtkWidget * charmap_new ();
void charmap_set_font (Charmap *charmap, gchar *font_name);
void charmap_identify_clipboard (Charmap *charmap, GtkClipboard *clipboard);
void charmap_expand_block_selector (Charmap *charmap);
void charmap_collapse_block_selector (Charmap *charmap);
void charmap_go_to_character (Charmap *charmap, gunichar uc);
charmap_search_result_t charmap_search (Charmap *charmap, 
                                        const gchar *search_text);


#ifdef __cplusplus
}
#endif  /* #ifdef __cplusplus */

#endif  /* #ifndef CHARMAP_H */
