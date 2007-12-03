/*
 * Copyright (c) 2005 Jason Allen
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
#include <glib.h>

#include "gucharmap-chapters.h"
#include "gucharmap-settings.h"
#include "gucharmap-intl.h"
#if HAVE_GCONF
#include <gconf/gconf-client.h>
#endif

#define GCONF_PREFIX "/apps/gucharmap"

typedef struct _GucharmapSettings GucharmapSettings;

struct _GucharmapSettings
{
#if HAVE_GCONF
  GConfClient *gc;
#else
  gpointer dummy;
#endif
};

static GucharmapSettings settings;

static gchar *
get_default_chapter (void)
{
  /* XXX: In the future, do something based on chapters mode and locale 
   * or something. */
  return NULL;
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

static gunichar
get_default_last_char (void)
{
  return 0;
}

static gboolean
get_default_snap_pow2 (void)
{
  return FALSE;
}

static gint
get_default_window_width (void)
{
  return -1;
}

static gint
get_default_window_height (void)
{
  return -1;
}

static gboolean
get_default_window_maximized (void)
{
  return FALSE;
}


#if HAVE_GCONF

void
gucharmap_settings_initialize (void)
{
  if (settings.gc != NULL)
    return;

  settings.gc = gconf_client_get_default ();

  if (settings.gc == NULL) {
    g_message(_("GConf could not be initialized."));
    return;
  }

  gconf_client_add_dir (settings.gc, GCONF_PREFIX,
                        GCONF_CLIENT_PRELOAD_ONELEVEL, NULL);
}

void
gucharmap_settings_shutdown (void)
{
  gconf_client_remove_dir (settings.gc, GCONF_PREFIX, NULL);
  g_object_unref(settings.gc);
  settings.gc = NULL;
}

static gboolean
gucharmap_settings_initialized (void) 
{
  return (settings.gc != NULL);
}

gchar *
gucharmap_settings_get_chapter (void)
{
#if 0
  gchar *chapter = NULL;

  if (gucharmap_settings_initialized())
    chapter = gconf_client_get_string (settings.gc, GCONF_PREFIX"/chapter", NULL);

  if (chapter) 
    return chapter;
  else
    return get_default_chapter();
#endif
  return get_default_chapter ();
}

void
gucharmap_settings_set_chapter (gchar *chapter)
{
#if 0
  if (!gucharmap_settings_initialized ()) {
      return;
  }
  gconf_client_set_string (settings.gc, GCONF_PREFIX"/chapter", chapter, NULL);
#endif
}

ChaptersMode
gucharmap_settings_get_chapters_mode (void)
{
#if 0
  ChaptersMode ret;
  gchar *mode = NULL;
  
  if (gucharmap_settings_initialized ())
  	mode = gconf_client_get_string (settings.gc, GCONF_PREFIX"/chapters_mode", NULL);

  if (mode == NULL)
    return get_default_chapters_mode ();

  if (g_ascii_strncasecmp (mode, "script", 6) == 0)
    ret = CHAPTERS_SCRIPT;
  else if (g_ascii_strncasecmp (mode, "block", 5) == 0)
    ret = CHAPTERS_BLOCK;
  else
    ret = get_default_chapters_mode ();

  g_free (mode);
  return ret;
#endif
  return get_default_chapters_mode ();
}

void
gucharmap_settings_set_chapters_mode (ChaptersMode mode)
{
#if 0
  if (!gucharmap_settings_initialized ()) {
      return;
  }

  switch (mode)
    {
      case CHAPTERS_SCRIPT:
        gconf_client_set_string (settings.gc, GCONF_PREFIX"/chapters_mode", "script", NULL);
      break;

      case CHAPTERS_BLOCK:
        gconf_client_set_string (settings.gc, GCONF_PREFIX"/chapters_mode", "block", NULL);
      break;
    }
#endif
}

gchar *
gucharmap_settings_get_font (void)
{
  if (!gucharmap_settings_initialized ()) {
      return get_default_font ();
  }
  
  return gconf_client_get_string (settings.gc, GCONF_PREFIX"/font", NULL);
}

void
gucharmap_settings_set_font (gchar *fontname)
{
  if (!gucharmap_settings_initialized ()) {
      return;
  }
  
  gconf_client_set_string (settings.gc, GCONF_PREFIX"/font", fontname, NULL);
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

  str = gconf_client_get_string (settings.gc, GCONF_PREFIX"/last_char", NULL);
  if (!str) {
    return get_default_last_char ();
  }

  /* FIXME: use g_ascii_strtoull */
  sscanf (str, "U+%X", &c);
  g_free(str);
  return c;
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
  gconf_client_set_string (settings.gc, GCONF_PREFIX"/last_char", str, NULL);
}

gboolean
gucharmap_settings_get_snap_pow2 (void)
{
  if (!gucharmap_settings_initialized ()) {
      return get_default_snap_pow2 ();
  }
  
  return gconf_client_get_bool (settings.gc, GCONF_PREFIX"/snap_cols_pow2", NULL);
}

void
gucharmap_settings_set_snap_pow2 (gboolean snap_pow2)
{
  if (!gucharmap_settings_initialized ()) {
      return;
  }
  
  gconf_client_set_bool (settings.gc, GCONF_PREFIX"/snap_cols_pow2", snap_pow2, NULL);
}

gint
gucharmap_settings_get_window_width (void)
{
  if (!gucharmap_settings_initialized ()) {
      return get_default_window_width ();
  }
  
  return gconf_client_get_int (settings.gc, GCONF_PREFIX"/width", NULL);
}

void
gucharmap_settings_set_window_width (gint width)
{
  if (!gucharmap_settings_initialized ()) {
      return;
  }
  
  gconf_client_set_int (settings.gc, GCONF_PREFIX"/width", width, NULL);
}

gint
gucharmap_settings_get_window_height (void)
{
  if (!gucharmap_settings_initialized ()) {
      return get_default_window_height ();
  }
  
  return gconf_client_get_int (settings.gc, GCONF_PREFIX"/height", NULL);
}

void
gucharmap_settings_set_window_height (gint height)
{
  if (!gucharmap_settings_initialized ()) {
      return;
  }
  
  gconf_client_set_int (settings.gc, GCONF_PREFIX"/height", height, NULL);
}

gboolean
gucharmap_settings_get_window_maximized (void)
{
  if (!gucharmap_settings_initialized ()) {
      return get_default_window_maximized ();
  }
  
  return gconf_client_get_bool (settings.gc, GCONF_PREFIX"/maximized", NULL);
}

void
gucharmap_settings_set_window_maximized (gboolean maximized)
{
  if (!gucharmap_settings_initialized ()) {
      return;
  }
  
  gconf_client_set_bool (settings.gc, GCONF_PREFIX"/maximized", maximized, NULL);
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

gchar *
gucharmap_settings_get_chapter (void)
{
  return get_default_chapter ();
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

gint
gucharmap_settings_get_window_width (void)
{
  return get_default_window_width ();
}

void
gucharmap_settings_set_window_width (gint width)
{
  return;
}

gint
gucharmap_settings_get_window_height (void)
{
  return get_default_window_height ();
}

void
gucharmap_settings_set_window_height (gint height)
{
  return;
}

gboolean
gucharmap_settings_get_window_maximized (void)
{
  return get_default_window_maximized ();
}

void
gucharmap_settings_set_window_maximized (gboolean maximized)
{
  return;
}

#endif /* HAVE_GCONF */

