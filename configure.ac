dnl $Id$
dnl
dnl Copyright (c) 2002  Noah Levitt <nlevitt@users.sourceforge.net>
dnl
dnl This program is free software; you can redistribute it and/or modify it
dnl under the terms of the GNU General Public License as published by the
dnl Free Software Foundation; either version 2 of the License, or (at your
dnl option) any later version.
dnl
dnl This program is distributed in the hope that it will be useful, but
dnl WITHOUT ANY WARRANTY; without even the implied warranty of
dnl MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
dnl General Public License for more details.
dnl
dnl You should have received a copy of the GNU General Public License along
dnl with this program; if not, write to the Free Software Foundation, Inc.,
dnl 59 Temple Place, Suite 330, Boston, MA 02111-1307  USA
dnl

AC_PREREQ([2.50])
AC_INIT([gucharmap], [0.1], [nlevitt@users.sourceforge.net])
AM_CONFIG_HEADER(config.h)
AM_INIT_AUTOMAKE(gucharmap, 0.1)

# checks for progs
AC_PROG_CC
AC_PROG_LIBTOOL

# checks for stuff
GETTEXT_PACKAGE=gucharmap
AC_SUBST(GETTEXT_PACKAGE)
AC_DEFINE_UNQUOTED(GETTEXT_PACKAGE, "$GETTEXT_PACKAGE", 
                   [Define the gettext package to be used])
ALL_LINGUAS="ar az bg ca cs da de el en_GB es et eu fi fr ga gl hu it ja ko lt lv nl nn no pl pt_BR pt ro ru sk sl sv ta tr uk vi wa zh_CN zh_TW"
AM_GLIB_GNU_GETTEXT

PKG_CHECK_MODULES(GUCHARMAP, gtk+-2.0 >= 2.0.0)

# checks for funcs 
AC_CHECK_FUNCS([bind_textdomain_codeset])

AC_SUBST(GUCHARMAP_CFLAGS)
AC_SUBST(GUCHARMAP_LIBS)

AC_OUTPUT([Makefile po/Makefile.in])
