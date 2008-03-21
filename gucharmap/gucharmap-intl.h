#ifndef GUCHARMAP_INTL_H
#define GUCHARMAP_INTL_H

#include <glib/gi18n-lib.h>

G_BEGIN_DECLS

#define I_(string) g_intern_static_string (string)

void _gucharmap_intl_ensure_initialized (void);

G_END_DECLS

#endif /* #ifndef GUCHARMAP_INTL_H */
