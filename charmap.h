/* $Id$ */


#ifndef CHARMAP_H
#define CHARMAP_H

#include <gtk/gtk.h>


#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

#define CHARMAP(obj)         GTK_CHECK_CAST (obj, charmap_get_type (), Charmap)
#define CHARMAP_CLASS(clazz) GTK_CHECK_CLASS_CAST (clazz, charmap_get_type (), CharmapClass)
#define IS_CHARMAP(obj)      GTK_CHECK_TYPE (obj, charmap_get_type ())


typedef struct _Charmap Charmap;
typedef struct _CharmapClass CharmapClass;


typedef struct _Square Square;

struct _Square
{
  GtkWidget *event_box;
  GtkWidget *label;
  guint16 row, col;
  Charmap *charmap; /* the charmap to which this square belongs */
};


struct _Charmap
{
  GtkVBox parent;

  GtkWidget *character_selector;
  GtkWidget *table;
  Square ***squares;
  Square *selected;

  GtkWidget *block_selector;
  gchar *font_name;
  gint rows, columns;
  gunichar block_start;

  GtkWidget *caption;
};

struct _CharmapClass
{
  GtkVBoxClass parent_class;
};

GtkType     charmap_get_type (void);
GtkWidget * charmap_new (void);
void        charmap_set_font (Charmap *charmap, gchar *font_name);
gchar *     charmap_get_font (Charmap *charmap);


#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif  /* #ifndef CHARMAP_H */


