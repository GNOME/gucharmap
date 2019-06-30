/*
 * Copyright © 2004 Noah Levitt
 * Copyright © 2008 Christian Persch
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
 * 51 Franklin Street, Fifth Floor, Boston, MA 02110-1335 USA
 */

#ifndef GUCHARMAP_H
#define GUCHARMAP_H

#define __GUCHARMAP_GUCHARMAP_H_INSIDE__

#include <glib.h>

#if defined(GUCHARMAP_DISABLE_DEPRECATION_WARNINGS) || !GLIB_CHECK_VERSION (2, 31, 0)
#define GUCHARMAP_DEPRECATED
#define GUCHARMAP_DEPRECATED_FOR(f)
#else
#define GUCHARMAP_DEPRECATED G_DEPRECATED
#define GUCHARMAP_DEPRECATED_FOR(f) G_DEPRECATED_FOR(f)
#endif

#include <gucharmap/gucharmap-version.h>

#include <gucharmap/gucharmap-codepoint-list.h>
#include <gucharmap/gucharmap-block-codepoint-list.h>
#include <gucharmap/gucharmap-script-codepoint-list.h>

#include <gucharmap/gucharmap-chapters-model.h>
#include <gucharmap/gucharmap-block-chapters-model.h>
#include <gucharmap/gucharmap-script-chapters-model.h>

#include <gucharmap/gucharmap-chartable.h>
#include <gucharmap/gucharmap-charmap.h>

#include <gucharmap/gucharmap-type-builtins.h>

#include <gucharmap/gucharmap-unicode-info.h>

#undef GUCHARMAP_DEPRECATED
#undef GUCHARMAP_DEPRECATED_FOR
#undef __GUCHARMAP_GUCHARMAP_H_INSIDE__

#endif /* #ifndef GUCHARMAP_H */
