/* $Id$ */
/*
 * Copyright (c) 2003  Noah Levitt <nlevitt аt columbia.edu>
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
 *
 * gtk+ input method module using gucharmap
 */

#ifdef HAVE_CONFIG_H
# include <config.h>
#endif

#include <gtk/gtkimcontextsimple.h>
#include <gtk/gtkimmodule.h>
#include <gtk/gtk.h>
#include <gmodule.h>
#include <string.h>
#include <gucharmap/gucharmap_window.h>
#include <gucharmap/gucharmap_intl.h>


GType type_imgucharmap = 0;
GucharmapWindow *the_guw = NULL;


static void
chartable_activate (Chartable *chartable, 
                    gunichar uc, 
                    GtkIMContextSimple *im_context)
{
  gchar buf[7];
  buf [g_unichar_to_utf8 (uc, buf)] = '\0';
  g_signal_emit_by_name (im_context, "commit", &buf);
}


static void
disconnect_handler (GucharmapWindow *guw)
{
  gulong *handler_id;

  handler_id = g_object_get_data (G_OBJECT (gdk_display_get_default ()),
                                  "gucharmap-activate-handler-id");
  if (handler_id != NULL)
    {
      g_signal_handler_disconnect (guw->charmap->chartable, *handler_id);
      g_free (handler_id);
      g_object_set_data (G_OBJECT (gdk_display_get_default ()),
                         "gucharmap-activate-handler-id", NULL);
    }
}


static void
connect_handler (GucharmapWindow *guw, GtkIMContext *im_context)
{
  gulong *handler_id;

  handler_id = g_malloc (sizeof (gulong));

  *handler_id = g_signal_connect (guw->charmap->chartable, "activate", 
                                  G_CALLBACK (chartable_activate), im_context);

  g_object_set_data (G_OBJECT (gdk_display_get_default ()),
                     "gucharmap-activate-handler-id", handler_id);
}


static void
focus_in (GtkIMContext *im_context)
{
  GucharmapWindow *guw;

  guw = g_object_get_data (G_OBJECT (gdk_display_get_default ()), 
                           "gucharmap-window");

  g_assert (guw != NULL);

  if (GTK_WIDGET_VISIBLE (GTK_WIDGET (guw)))
    {
      /* disconnect from the old im_context, connect to the new */
      disconnect_handler (guw);
      connect_handler (guw, im_context);
    }
}


static void
set_client_window (GtkIMContext *context, GdkWindow *window)
{
  GucharmapWindow *guw;

  guw = g_object_get_data (G_OBJECT (gdk_display_get_default ()), 
                           "gucharmap-window");

  /* the window we were connected to could have been destroyed */
  if (window == NULL && guw != NULL)
    disconnect_handler (guw);
}


static void
imgucharmap_class_init (GtkIMContextSimpleClass *clazz)
{
  GTK_IM_CONTEXT_CLASS (clazz)->set_client_window = set_client_window;
  GTK_IM_CONTEXT_CLASS (clazz)->focus_in = focus_in;
}


void 
im_module_exit ()
{
  GucharmapWindow *guw;

  guw = g_object_get_data (G_OBJECT (gdk_display_get_default ()), 
                           "gucharmap-window");

  g_assert (guw != NULL);

  disconnect_handler (guw);
  gtk_widget_hide (GTK_WIDGET (guw));
}


static void
imgucharmap_init (GtkIMContextSimple *im_context)
{
  GdkScreen *screen;
  GtkWidget *guw = NULL;

  guw = g_object_get_data (G_OBJECT (gdk_display_get_default ()), 
                           "gucharmap-window");
  if (guw == NULL)
    {
      guw = gucharmap_window_new ();

      screen = gtk_window_get_screen (GTK_WINDOW (guw));
      gtk_window_set_default_size (GTK_WINDOW (guw), 
                                   gdk_screen_get_width (screen) * 5/12, 
                                   gdk_screen_get_height (screen) * 1/3); 

      g_signal_connect (guw, "delete-event", 
                        G_CALLBACK (gtk_widget_hide_on_delete), NULL);

      g_object_set_data (G_OBJECT (gdk_display_get_default ()), 
                         "gucharmap-window", guw);

      the_guw = g_object_ref (guw);
    }


  gtk_widget_show_all (guw);
}


static void
imgucharmap_register_type (GTypeModule *module)
{
  static const GTypeInfo object_info =
  {
    sizeof (GtkIMContextSimpleClass),
    (GBaseInitFunc) NULL,
    (GBaseFinalizeFunc) NULL,
    (GClassInitFunc) imgucharmap_class_init,
    NULL,           /* class_finalize */
    NULL,           /* class_data */
    sizeof (GtkIMContextSimple),
    0,
    (GInstanceInitFunc) imgucharmap_init,
  };

  type_imgucharmap = 
    g_type_module_register_type (module,
				 GTK_TYPE_IM_CONTEXT_SIMPLE,
				 "GtkIMContextGucharmap",
				 &object_info, 0);
}


static const GtkIMContextInfo imgucharmap_info = 
{
  "imgucharmap",                /* ID */
  N_("Unicode Character Map"),  /* Human readable name */
  GETTEXT_PACKAGE,              /* Translation domain */
  LOCALEDIR,                    /* Dir for bindtextdomain */
  ""    /* Languages for which this module is the default */
};


static const GtkIMContextInfo *info_list[] = {
  &imgucharmap_info
};


void
im_module_init (GTypeModule *module)
{
  imgucharmap_register_type (module);
}


void 
im_module_list (const GtkIMContextInfo ***contexts, gint *n_contexts)
{
  *contexts = info_list;
  *n_contexts = G_N_ELEMENTS (info_list);
}


GtkIMContext *
im_module_create (const gchar *context_id)
{
  if (strcmp (context_id, "imgucharmap") == 0)
    return GTK_IM_CONTEXT (g_object_new (type_imgucharmap, NULL));
  else
    return NULL;
}


/* need to make the module resident so that the static variables in the
 * various gucharmap classes can't unset */
const gchar * 
g_module_check_init (GModule *module)
{
  g_module_make_resident (module);
  return NULL;
}


