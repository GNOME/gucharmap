/*
 * Copyright (c) 2005 Jason Allen
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
 * 59 Temple Place, Suite 330, Boston, MA 02110-1301  USA
 */

#include "config.h"

#include <string.h>
#include <glib.h>
#include <glib/gi18n-lib.h>

#include "gucharmap-chapters.h"
#include "gucharmap-settings.h"
#include "gucharmap-intl.h"
#if HAVE_GCONF
#include <gconf/gconf-client.h>
static GConfClient *client;
#endif

#define WINDOW_STATE_TIMEOUT 1000 /* ms */

#define GCONF_PREFIX "/apps/gucharmap"

static gunichar
get_first_non_underscore_char (const char *str)
{
  const char *p;

  if (!str)
    return 0;

  for (p = str; p && *p; p = g_utf8_find_next_char (p, NULL))
    {
      gunichar ch;

      ch = g_utf8_get_char (p);
      if (g_unichar_isalpha (ch))
        return ch;
    }

  return 0;
}

static gunichar
get_default_last_char (void)
{
  return get_first_non_underscore_char (_("_File")); /* use a super-common string */
}

static ChaptersMode
get_default_chapters_mode (void)
{
  /* XXX: In the future, do something based on chapters mode and locale 
   * or something. */
  return CHAPTERS_SCRIPT;
}

static gchar *
get_default_font (void)
{
  return NULL;
}

static gboolean
get_default_snap_pow2 (void)
{
  return FALSE;
}

#if HAVE_GCONF

void
gucharmap_settings_initialize (void)
{
  client = gconf_client_get_default ();

  if (client == NULL) {
    g_message(_("GConf could not be initialized."));
    return;
  }

  gconf_client_add_dir (client, GCONF_PREFIX,
                        GCONF_CLIENT_PRELOAD_ONELEVEL, NULL);
}

void
gucharmap_settings_shutdown (void)
{
  gconf_client_remove_dir (client, GCONF_PREFIX, NULL);
  g_object_unref(client);
  client = NULL;
}

static gboolean
gucharmap_settings_initialized (void) 
{
  return (client != NULL);
}

ChaptersMode
gucharmap_settings_get_chapters_mode (void)
{
  ChaptersMode ret;
  gchar *mode;
  
  mode = gconf_client_get_string (client, GCONF_PREFIX"/chapters_mode", NULL);
  if (mode == NULL)
    return get_default_chapters_mode ();

  if (strcmp (mode, "script") == 0)
    ret = CHAPTERS_SCRIPT;
  else if (strcmp (mode, "block") == 0)
    ret = CHAPTERS_BLOCK;
  else
    ret = get_default_chapters_mode ();

  g_free (mode);
  return ret;
}

void
gucharmap_settings_set_chapters_mode (ChaptersMode mode)
{
  switch (mode)
    {
      case CHAPTERS_SCRIPT:
        gconf_client_set_string (client, GCONF_PREFIX"/chapters_mode", "script", NULL);
      break;

      case CHAPTERS_BLOCK:
        gconf_client_set_string (client, GCONF_PREFIX"/chapters_mode", "block", NULL);
      break;
    }
}

gchar *
gucharmap_settings_get_font (void)
{
  if (!gucharmap_settings_initialized ()) {
      return get_default_font ();
  }
  
  return gconf_client_get_string (client, GCONF_PREFIX"/font", NULL);
}

void
gucharmap_settings_set_font (gchar *fontname)
{
  if (!gucharmap_settings_initialized ()) {
      return;
  }
  
  gconf_client_set_string (client, GCONF_PREFIX"/font", fontname, NULL);
}

gunichar
gucharmap_settings_get_last_char (void)
{
  /* See bug 469053 */
  gchar *str;
  gunichar c;

  if (!gucharmap_settings_initialized ()) {
      return get_default_last_char ();
  }

  str = gconf_client_get_string (client, GCONF_PREFIX"/last_char", NULL);
  if (!str) {
    return get_default_last_char ();
  }

  /* FIXME: use g_ascii_strtoull */
  sscanf (str, "U+%X", &c);
  g_free(str);
  if (c > 0 && c < UNICHAR_MAX)
    return c;

  return get_default_last_char ();
}

void
gucharmap_settings_set_last_char (gunichar wc)
{
  char str[32];

  if (!gucharmap_settings_initialized ()) {
      return;
  }
  
  g_snprintf (str, sizeof (str), "U+%04X", wc);
  str[sizeof (str) - 1] = '\0';
  gconf_client_set_string (client, GCONF_PREFIX"/last_char", str, NULL);
}

gboolean
gucharmap_settings_get_snap_pow2 (void)
{
  if (!gucharmap_settings_initialized ()) {
      return get_default_snap_pow2 ();
  }
  
  return gconf_client_get_bool (client, GCONF_PREFIX"/snap_cols_pow2", NULL);
}

void
gucharmap_settings_set_snap_pow2 (gboolean snap_pow2)
{
  if (!gucharmap_settings_initialized ()) {
      return;
  }
  
  gconf_client_set_bool (client, GCONF_PREFIX"/snap_cols_pow2", snap_pow2, NULL);
}

#else /* HAVE_GCONF */

void
gucharmap_settings_initialize (void)
{
  return;
}

void 
gucharmap_settings_shutdown (void)
{
  return;
}

static gboolean
gucharmap_settings_initialized (void)
{
  return FALSE;
}

void
gucharmap_settings_set_chapter (gchar *chapter)
{
  return;
}

ChaptersMode
gucharmap_settings_get_chapters_mode (void)
{
  return get_default_chapters_mode();
}

void
gucharmap_settings_set_chapters_mode (ChaptersMode mode)
{
  return;
}

gchar *
gucharmap_settings_get_font (void)
{
  return get_default_font ();
}

void
gucharmap_settings_set_font (gchar *fontname)
{
  return;
}

gunichar
gucharmap_settings_get_last_char (void)
{
  return get_default_last_char ();
}

void
gucharmap_settings_set_last_char (gunichar wc)
{
  return;
}

gboolean
gucharmap_settings_get_snap_pow2 (void)
{
  return get_default_snap_pow2 ();
}

void
gucharmap_settings_set_snap_pow2 (gboolean snap_pow2)
{
  return;
}

#endif /* HAVE_GCONF */

#ifdef HAVE_GCONF

typedef struct {
  guint timeout_id;
  int width;
  int height;
  guint is_maximised : 1;
  guint is_fullscreen : 1;
} WindowState;

static gboolean
window_state_timeout_cb (WindowState *state)
{
  gconf_client_set_int (client, GCONF_PREFIX "/width", state->width, NULL);
  gconf_client_set_int (client, GCONF_PREFIX "/height", state->height, NULL);

  state->timeout_id = 0;
  return FALSE;
}

static void
free_window_state (WindowState *state)
{
  if (state->timeout_id != 0) {
    g_source_remove (state->timeout_id);

    /* And store now */
    window_state_timeout_cb (state);
  }

  g_slice_free (WindowState, state);
}

static gboolean
window_configure_event_cb (GtkWidget *widget,
                           GdkEventConfigure *event,
                           WindowState *state)
{
  if (!state->is_maximised && !state->is_fullscreen &&
      (state->width != event->width || state->height != event->height)) {
    state->width = event->width;
    state->height = event->height;

    if (state->timeout_id == 0) {
      state->timeout_id = g_timeout_add (WINDOW_STATE_TIMEOUT,
                                         (GSourceFunc) window_state_timeout_cb,
                                         state);
    }
  }

  return FALSE;
}

static gboolean
window_state_event_cb (GtkWidget *widget,
                       GdkEventWindowState *event,
                       WindowState *state)
{
  if (event->changed_mask & GDK_WINDOW_STATE_MAXIMIZED) {
    state->is_maximised = (event->new_window_state & GDK_WINDOW_STATE_MAXIMIZED) != 0;
    gconf_client_set_bool (client, GCONF_PREFIX "/maximized", state->is_maximised, NULL);
  }
  if (event->changed_mask & GDK_WINDOW_STATE_FULLSCREEN) {
    state->is_fullscreen = (event->new_window_state & GDK_WINDOW_STATE_FULLSCREEN) != 0;
    gconf_client_set_bool (client, GCONF_PREFIX "/fullscreen", state->is_fullscreen, NULL);
  }

  return FALSE;
}

#endif /* HAVE_GCONF */

/**
 * gucharma_settings_add_window:
 * @window: a #GtkWindow
 *
 * Restore the window configuration, and persist changes to the window configuration:
 * window width and height, and maximised and fullscreen state.
 * @window must not be realised yet.
 */
void
gucharmap_settings_add_window (GtkWindow *window)
{
#ifdef HAVE_GCONF
  WindowState *state;
  int width, height;
  gboolean maximised, fullscreen;

  g_return_if_fail (GTK_IS_WINDOW (window));
  g_return_if_fail (!GTK_WIDGET_REALIZED (window));

  state = g_slice_new0 (WindowState);
  g_object_set_data_full (G_OBJECT (window), "GamesConf::WindowState",
                          state, (GDestroyNotify) free_window_state);

  g_signal_connect (window, "configure-event",
                    G_CALLBACK (window_configure_event_cb), state);
  g_signal_connect (window, "window-state-event",
                    G_CALLBACK (window_state_event_cb), state);

  maximised = gconf_client_get_bool (client, GCONF_PREFIX "/maximized", NULL);
  fullscreen = gconf_client_get_bool (client, GCONF_PREFIX "/fullscreen", NULL);
  width = gconf_client_get_int (client, GCONF_PREFIX "/width", NULL);
  height = gconf_client_get_int (client, GCONF_PREFIX "/height", NULL);

  if (width > 0 && height > 0) {
    gtk_window_set_default_size (GTK_WINDOW (window), width, height);
  }
  if (maximised) {
    gtk_window_maximize (GTK_WINDOW (window));
  }
  if (fullscreen) {
    gtk_window_fullscreen (GTK_WINDOW (window));
  }
#endif /* HAVE_GCONF */
}