/* $Id$ */

#ifdef HAVE_CONFIG_H
# include "config.h"
#endif

#ifdef ENABLE_NLS

#include <libintl.h>
#include <gtk/gtk.h>

gchar *
gucharmap_gettext (const gchar *str)
{
  static gboolean gucharmap_gettext_initialized = FALSE;

  if (!gucharmap_gettext_initialized)
    {
      bindtextdomain (GETTEXT_PACKAGE, LOCALEDIR);
#ifdef HAVE_BIND_TEXTDOMAIN_CODESET
      bind_textdomain_codeset (GETTEXT_PACKAGE, "UTF-8");
#endif
      gucharmap_gettext_initialized = TRUE;
    }
  
#if 0
    {
      gchar *rv;
      rv = dgettext (GETTEXT_PACKAGE, str);
      g_printerr ("gucharmap_gettext: returing \"%s\"\n", rv);
    }
#endif

  return dgettext (GETTEXT_PACKAGE, str);
}

#endif /* #ifdef ENABLE_NLS */
