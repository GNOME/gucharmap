/*
 * Copyright (c) 2004 Noah Levitt
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

#include <gtk/gtk.h>
#include <gdk/gdkkeysyms.h>
#include "gucharmap-marshal.h"
#include "gucharmap-intl.h"
#include "gucharmap-chartable.h"
#include "gucharmap-table.h"
#include "gucharmap-unicode-info.h"

enum 
{
  ACTIVATE = 0,
  SET_ACTIVE_CHAR,
  STATUS_MESSAGE,
  NUM_SIGNALS
};

static guint signals[NUM_SIGNALS];

static void
activate (GucharmapChartable *real_table, GucharmapTable *chartable)
{
  g_signal_emit (chartable, signals[ACTIVATE], 0,
                 gucharmap_chartable_get_active_character (real_table));
}

static void
sync_active_char (GucharmapChartable *real_table, GParamSpec *pspec, GucharmapTable *chartable)
{
  g_signal_emit (chartable, signals[SET_ACTIVE_CHAR], 0,
                 gucharmap_chartable_get_active_character (real_table));
}

static void
status_message (GucharmapChartable *real_table, const gchar *message, GucharmapTable *chartable)
{
  g_signal_emit (chartable, signals[STATUS_MESSAGE], 0, message);
}

static void
gucharmap_table_init (GucharmapTable *chartable)
{
  chartable->scrolled_window = gtk_scrolled_window_new (NULL, NULL);
  gtk_scrolled_window_set_policy (GTK_SCROLLED_WINDOW (chartable->scrolled_window),
                                  GTK_POLICY_NEVER, GTK_POLICY_AUTOMATIC);

  chartable->drawing_area = gucharmap_chartable_new ();
  g_signal_connect (GUCHARMAP_CHARTABLE (chartable->drawing_area), "activate",
                    G_CALLBACK (activate), chartable);
  g_signal_connect (GUCHARMAP_CHARTABLE (chartable->drawing_area), "status-message",
                    G_CALLBACK (status_message), chartable);
  g_signal_connect (GUCHARMAP_CHARTABLE (chartable->drawing_area), "notify::active-character",
                    G_CALLBACK (sync_active_char), chartable);

  gtk_container_add (GTK_CONTAINER (chartable->scrolled_window),
                     chartable->drawing_area);
  gtk_widget_show (chartable->drawing_area);

  gtk_box_pack_start (GTK_BOX (chartable), chartable->scrolled_window,
                      TRUE, TRUE, 0);
  gtk_widget_show (chartable->scrolled_window);

  gtk_widget_show_all (GTK_WIDGET (chartable));
}

static void
gucharmap_table_class_init (GucharmapTableClass *klass)
{
  klass->activate = NULL;
  klass->set_active_char = NULL;

  signals[ACTIVATE] =
      g_signal_new ("activate", gucharmap_table_get_type (), G_SIGNAL_RUN_FIRST,
                    G_STRUCT_OFFSET (GucharmapTableClass, activate),
                    NULL, NULL, g_cclosure_marshal_VOID__UINT, G_TYPE_NONE,
		    1, G_TYPE_UINT);

  signals[SET_ACTIVE_CHAR] =
      g_signal_new ("set-active-char", gucharmap_table_get_type (),
                    G_SIGNAL_RUN_FIRST,
                    G_STRUCT_OFFSET (GucharmapTableClass, set_active_char),
                    NULL, NULL, g_cclosure_marshal_VOID__UINT, G_TYPE_NONE,
		    1, G_TYPE_UINT);

  signals[STATUS_MESSAGE] =
      g_signal_new ("status-message", gucharmap_table_get_type (), G_SIGNAL_RUN_FIRST,
                    G_STRUCT_OFFSET (GucharmapTableClass, status_message),
                    NULL, NULL, g_cclosure_marshal_VOID__STRING, G_TYPE_NONE,
		    1, G_TYPE_STRING);
}

G_DEFINE_TYPE (GucharmapTable, gucharmap_table, GTK_TYPE_HBOX)

GtkWidget *
gucharmap_table_new (void)
{
  return GTK_WIDGET (g_object_new (gucharmap_table_get_type (), NULL));
}

void
gucharmap_table_zoom_enable (GucharmapTable *chartable)
{
  gucharmap_chartable_set_zoom_enabled (GUCHARMAP_CHARTABLE (chartable->drawing_area), TRUE);
}

void
gucharmap_table_zoom_disable (GucharmapTable *chartable)
{
  gucharmap_chartable_set_zoom_enabled (GUCHARMAP_CHARTABLE (chartable->drawing_area), FALSE);
}

void 
gucharmap_table_set_font (GucharmapTable *chartable, const gchar *font_name)
{
  gucharmap_chartable_set_font (GUCHARMAP_CHARTABLE (chartable->drawing_area), font_name);
}

gunichar 
gucharmap_table_get_active_character (GucharmapTable *chartable)
{
  return gucharmap_chartable_get_active_character (GUCHARMAP_CHARTABLE (chartable->drawing_area));
}

void
gucharmap_table_set_active_character (GucharmapTable *chartable, 
                                      gunichar        wc)
{
  gucharmap_chartable_set_active_character (GUCHARMAP_CHARTABLE (chartable->drawing_area), wc);
}

void
gucharmap_table_grab_focus (GucharmapTable *chartable)
{
  gtk_widget_grab_focus (chartable->drawing_area);
}

void 
gucharmap_table_set_snap_pow2 (GucharmapTable *chartable, gboolean snap)
{
  gucharmap_chartable_set_snap_pow2 (GUCHARMAP_CHARTABLE (chartable->drawing_area), snap);
}

void
gucharmap_table_set_codepoint_list (GucharmapTable         *chartable,
                                    GucharmapCodepointList *list)
{
  gucharmap_chartable_set_codepoint_list (GUCHARMAP_CHARTABLE (chartable->drawing_area), list);
}

GucharmapCodepointList *
gucharmap_table_get_codepoint_list (GucharmapTable *chartable)
{
  return gucharmap_chartable_get_codepoint_list (GUCHARMAP_CHARTABLE (chartable->drawing_area));
}

gint
gucharmap_table_get_active_cell (GucharmapTable *chartable)
{
  return gucharmap_chartable_get_active_cell (GUCHARMAP_CHARTABLE (chartable->drawing_area));
}
