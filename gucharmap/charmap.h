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

G_BEGIN_DECLS


#define CHARMAP(obj) (G_TYPE_CHECK_INSTANCE_CAST ((obj), charmap_get_type (), \
                                                  Charmap))

#define CHARMAP_CLASS(clazz) (G_TYPE_CHECK_CLASS_CAST ((clazz), \
                                                       charmap_get_type (),\
                                                       CharmapClass))

#define IS_CHARMAP(obj) (G_TYPE_CHECK_INSTANCE_TYPE ((obj), \
                         charmap_get_type ()))


#define CHARMAP_MIN_ROWS 4
#define CHARMAP_MIN_COLS 4


/* 0x100, a standard increment for paging unicode */
#define BLOCK_SIZE 256


typedef struct _Charmap Charmap;
typedef struct _CharmapClass CharmapClass;


typedef struct 
{
  gunichar start;
  GtkTreePath *tree_path;
} 
BlockIndex;


typedef enum
{
  CHARMAP_NOT_FOUND,
  CHARMAP_FOUND,
  CHARMAP_WRAPPED,
  CHARMAP_NOTHING_TO_SEARCH_FOR
}
CharmapSearchResult;


/* the order of the captions */
typedef enum
{
  CHARMAP_CAPTION_CHARACTER = 0,
  CHARMAP_CAPTION_CATEGORY,
  CHARMAP_CAPTION_DECOMPOSITION,
  CHARMAP_CAPTION_UTF8,
  CHARMAP_CAPTION_OTHER_REPS,
#if ENABLE_UNIHAN
  CHARMAP_CAPTION_KDEFINITION,
  CHARMAP_CAPTION_KMANDARIN,
  CHARMAP_CAPTION_KJAPANESEON,
  CHARMAP_CAPTION_KJAPANESEKUN,
  CHARMAP_CAPTION_KCANTONESE,
  CHARMAP_CAPTION_KTANG,
  CHARMAP_CAPTION_KKOREAN,
#endif
  /* nameslist stuff */
  CHARMAP_CAPTION_EQUALS,
  CHARMAP_CAPTION_STARS,
  CHARMAP_CAPTION_EXES,
  CHARMAP_CAPTION_POUNDS,
  CHARMAP_CAPTION_COLONS,

  CHARMAP_CAPTION_COUNT
}
CharmapCaption;


struct _Charmap
{
  GtkVBox parent;

  /* rows and columns on a page */
  gint rows, cols;

  GtkWidget *chartable;         /* GtkDrawingArea */
  GdkPixmap *chartable_pixmap; 
  GtkWidget *zoom_window;
  GdkPixbuf *zoom_pixbuf;

  gchar *font_name;
  PangoFontMetrics *font_metrics;
  PangoLayout *pango_layout;

  gunichar page_first_char;  /* the character in the upper left square */
  gunichar active_char;

  /* helps us know what to redraw */
  gunichar old_page_first_char;
  gunichar old_active_char;

  /* the unicode block selection list */
  GtkTreeSelection *block_selection;
  GtkTreeStore *block_selector_model;
  GtkWidget *block_selector_view;
  gulong block_selection_changed_handler_id; 

  BlockIndex *block_index;
  gint block_index_size;

  /* for the scrollbar */
  GtkObject *adjustment; 
  gulong adjustment_changed_handler_id; 

  /* the caption */
  GtkTreeRowReference *caption_rows[CHARMAP_CAPTION_COUNT];
  GtkTreeStore *caption_model;

  gboolean zoom_mode_enabled;
};


struct _CharmapClass
{
  GtkVBoxClass parent_class;

  void (* activate) (Charmap *charmap, gunichar uc);
  void (* status_message) (Charmap *charmap, gchar *message);
  void (* set_active_char) (Charmap *charmap, guint ch);
};


GtkType charmap_get_type (void);
GtkWidget * charmap_new (void);
void charmap_set_font (Charmap *charmap, gchar *font_name);
void charmap_identify_clipboard (Charmap *charmap, GtkClipboard *clipboard);
void charmap_expand_block_selector (Charmap *charmap);
void charmap_collapse_block_selector (Charmap *charmap);
void charmap_go_to_character (Charmap *charmap, gunichar uc);
CharmapSearchResult charmap_search (Charmap *charmap, const gchar *search_text);
void charmap_hide_caption (Charmap *charmap, CharmapCaption caption_id);
void charmap_show_caption (Charmap *charmap, CharmapCaption caption_id);
void charmap_zoom_enable (Charmap *charmap);
void charmap_zoom_disable (Charmap *charmap);

G_END_DECLS

#endif  /* #ifndef CHARMAP_H */

