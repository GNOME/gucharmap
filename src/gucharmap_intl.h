/* $Id$ */

#ifndef GUCHARMAP_INTL_H

#if HAVE_CONFIG_H
# include "config.h"
#endif

#undef _
#undef N_

#ifdef ENABLE_NLS

gchar * gucharmap_gettext (const gchar *str);

# include <libintl.h>

# define _(String) gucharmap_gettext(String)

# ifdef gettext_noop
#  define N_(String) gettext_noop(String)
# else
#  define N_(String) (String)
# endif

#else /* NLS is disabled */

# define _(String) (String)
# define N_(String) (String)

# undef textdomain
# undef gettext
# undef dgettext
# undef dcgettext
# undef bindtextdomain

# define textdomain(String) (String)
# define gettext(String) (String)
# define dgettext(Domain,String) (String)
# define dcgettext(Domain,String,Type) (String)
# define bindtextdomain(Domain,Directory) (Domain) 

#endif

#endif /* #ifndef GUCHARMAP_INTL_H */
