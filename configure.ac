# Copyright © 2003  Noah Levitt
# Copyright © 2007, 2008 Christian Persch
#
# This program is free software; you can redistribute it and/or modify it
# under the terms of the GNU General Public License as published by the
# Free Software Foundation; either version 2 of the License, or (at your
# option) any later version.
#
# This program is distributed in the hope that it will be useful, but
# WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
# General Public License for more details.
#
# You should have received a copy of the GNU General Public License along
# with this program; if not, write to the Free Software Foundation, Inc.,
# 59 Temple Place, Suite 330, Boston, MA 02110-1301  USA

m4_define([gucharmap_version_major],[2])
m4_define([gucharmap_version_minor],[23])
m4_define([gucharmap_version_micro],[0])
m4_define([gucharmap_version],[gucharmap_version_major.gucharmap_version_minor.gucharmap_version_micro])

# Before making a release, the libtool version should be modified.
# The string is of the form C:R:A.
# - If interfaces have been changed or added, but binary compatibility has
#   been preserved, change to C+1:0:A+1
# - If binary compatibility has been broken (eg removed or changed interfaces)
#   change to C+1:0:0
# - If the interface is the same as the previous version, change to C:R+1:A
m4_define([gucharmap_lt_current],[7])
m4_define([gucharmap_lt_revision],[0])
m4_define([gucharmap_lt_age],[0])
m4_define([gucharmap_lt_version_info],[gucharmap_lt_current:gucharmap_lt_revision:gucharmap_lt_age])
m4_define([gucharmap_lt_current_minus_age],[m4_eval(gucharmap_lt_current - gucharmap_lt_age)])

AC_PREREQ([2.56])

AC_INIT([GNOME Character Map],[gucharmap_version],[http://bugzilla.gnome.org/enter_bug.cgi?product=gucharmap],[gucharmap])

AC_CONFIG_SRCDIR([gucharmap/gucharmap-charmap.h])
AC_CONFIG_HEADERS([config.h])
AM_INIT_AUTOMAKE([1.9 gnu dist-bzip2 no-dist-gzip])

AM_MAINTAINER_MODE

# checks for progs
AC_PROG_CC
AC_DISABLE_STATIC
AC_PROG_LIBTOOL
IT_PROG_INTLTOOL([0.35.0])

AM_PROG_CC_C_O

# checks for stuff
GETTEXT_PACKAGE=gucharmap
AC_SUBST(GETTEXT_PACKAGE)
AC_DEFINE_UNQUOTED([GETTEXT_PACKAGE],["$GETTEXT_PACKAGE"],[Define the gettext package to be used])
AM_GLIB_GNU_GETTEXT

GNOME_COMMON_INIT
GNOME_DEBUG_CHECK
GNOME_COMPILE_WARNINGS([maximum])
GNOME_MAINTAINER_MODE_DEFINES

GNOME_DOC_INIT([0.9.0],[have_gdu=yes],[have_gdu=no])
if test "$have_gdu" = "no"; then
	AC_MSG_WARN([GNOME doc utils not found; disabling user manual])
fi

# check for win32
case "$host" in
  *-*-mingw*) os_win32="yes" ;; 
  *) os_win32="no" ;; 
esac

if test "x$os_win32" = "xyes"
then
    CFLAGS="$CFLAGS -mms-bitfields -mwindows"
fi
AM_CONDITIONAL(OS_WIN32, test "x$os_win32" = "xyes")

GTK_REQUIRED=2.12.0

PKG_CHECK_MODULES([GTK],[gtk+-2.0 >= $GTK_REQUIRED])

GLIB_GENMARSHAL="$($PKG_CONFIG --variable=glib_genmarshal glib-2.0)"
AC_SUBST([GLIB_GENMARSHAL])
GLIB_MKENUMS="$($PKG_CONFIG --variable=glib_mkenums glib-2.0)"
AC_SUBST([GLIB_MKENUMS])

AC_ARG_ENABLE([gconf],
              [AS_HELP_STRING([--disable-gconf],
	                      [don't build with gconf support])])
if test "x$enable_gconf" != "xno"; then
    GCONFPKGS="gconf-2.0"
    PKG_CHECK_MODULES(GCONF, gconf-2.0,
                have_gconf=yes, have_gconf=no)
else
    GCONFPKGS=
    AC_MSG_NOTICE([GCONF support disabled with --disable-gconf])
    have_gconf=no
fi  
AC_SUBST([GCONFPKGS])

AM_GCONF_SOURCE_2
AC_PATH_PROG([GCONFTOOL],[gconftool-2],[no])

# checks for funcs 
AC_CHECK_FUNCS([bind_textdomain_codeset])

AC_SUBST(PANGO_CFLAGS)
AC_SUBST(PANGO_LIBS)

AC_SUBST(GTK_CFLAGS)
AC_SUBST(GTK_LIBS)

AC_SUBST(GCONF_CFLAGS)
AC_SUBST(GCONF_LIBS)

if test "x$have_gconf" = "xyes" ; then
    AC_DEFINE([HAVE_GCONF], [1], [Define if have gconf])
fi

AH_TEMPLATE([ENABLE_UNIHAN], [Define if you want CJK ideograph information])
AC_ARG_ENABLE(unihan, 
              AC_HELP_STRING([--disable-unihan], 
                             [don't build in CJK ideograph information]))
if test "x$enable_unihan" != "xno" ; then
    AC_DEFINE([ENABLE_UNIHAN],[1],[Define if you want CJK ideograph information])
fi

# Some utilities
AC_PROG_LN_S

# AC_PATH_PROG([GTK_BUILDER_CONVERT],[gtk-builder-convert],[false])
# if test "$GTK_BUILDER_CONVERT" = "false"; then
#   AC_MSG_ERROR([gtk-builder-convert not found])
# fi
# 
# AC_PATH_PROG([XMLLINT],[xmllint],[false])
# if test "$XMLLINT" = "false"; then
#   AC_MSG_ERROR([xmllint not found])
# fi
# 
# AC_PATH_PROG([HEXDUMP],[hexdump],[false])
# if test "$HEXDUMP" = "false"; then
#   AC_MSG_ERROR([hexdump not found])
# fi

AC_SUBST([GUCHARMAP_VERSION_MAJOR],[gucharmap_version_major])
AC_SUBST([GUCHARMAP_VERSION_MINOR],[gucharmap_version_minor])
AC_SUBST([GUCHARMAP_VERSION_MICRO],[gucharmap_version_micro])
AC_SUBST([LIBGUCHARMAP_LT_VERSION],[gucharmap_lt_version_info])
AC_SUBST([LIBGUCHARMAP_LT_CURRENT_MINUS_AGE],[gucharmap_lt_current_minus_age])

AC_CONFIG_FILES([
gucharmap.desktop.in
gucharmap-2.pc
Makefile
gucharmap/Makefile
gucharmap/gucharmap.h
po/Makefile.in
pixmaps/Makefile
help/Makefile
help/it/Makefile
help/ja/Makefile
help/zh_CN/Makefile
help/zh_TW/Makefile
gucharmap.spec
gucharmap.nsi
])

AC_OUTPUT
