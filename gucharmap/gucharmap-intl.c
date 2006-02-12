/* $Id$ */

#include "config.h"

#include "gucharmap-intl.h"

void
gucharmap_intl_ensure_initialized (void)
{
#ifdef ENABLE_NLS
  static gboolean gucharmap_gettext_initialized = FALSE;

  if (!gucharmap_gettext_initialized)
    {
      bindtextdomain (GETTEXT_PACKAGE, LOCALEDIR);
#ifdef HAVE_BIND_TEXTDOMAIN_CODESET
      bind_textdomain_codeset (GETTEXT_PACKAGE, "UTF-8");
#endif

      gucharmap_gettext_initialized = TRUE;
    }
#endif /* #ifdef ENABLE_NLS */
}
