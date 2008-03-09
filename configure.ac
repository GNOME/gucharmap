dnl $Id: configure.ac,v 1.138 2006/09/04 22:41:10 behdad Exp $
dnl
dnl Copyright (c) 2003  Noah Levitt
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

AC_PREREQ([2.56])

AC_INIT([GNOME Character Map],[2.22.0],[http://bugzilla.gnome.org/enter_bug.cgi?product=gucharmap],[gucharmap])

AC_CONFIG_SRCDIR([gucharmap/gucharmap.h])
AC_CONFIG_HEADERS([config.h])
AM_INIT_AUTOMAKE([1.9 gnu dist-bzip2 no-dist-gzip])

AM_MAINTAINER_MODE

# checks for progs
AC_PROG_CC
AC_DISABLE_STATIC
AC_PROG_LIBTOOL
IT_PROG_INTLTOOL([0.35.0])

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
AM_CONDITIONAL(HAVE_GNOME_DOC_UTILS, test "$have_gdu" = "yes")

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

AC_DEFINE([UNICHAR_MAX],[0x0010ffff],[The last unicode character we support])

# gtk is required
GOBJECT_REQUIRED=2.3.0
GTK_REQUIRED=2.12.0

PKG_CHECK_MODULES([GTK],[
  gobject-2.0 >= $GOBJECT_REQUIRED
  gtk+-2.0 >= $GTK_REQUIRED])

AC_ARG_ENABLE([gconf],
              [AS_HELP_STRING([--disable-gconf],
	                      [don't build with gconf support])])
if test "x$enable_gconf" != "xno"; then
    PKG_CHECK_MODULES(GCONF, gconf-2.0,
                have_gconf=yes, have_gconf=no)
else
    AC_MSG_NOTICE([GCONF support disabled with --disable-gconf])
    have_gconf=no
fi  

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

GCONFPKGS=
if test "x$have_gconf" = "xyes" ; then
    AC_DEFINE([HAVE_GCONF], [1], [Define if have gconf])
    GCONFPKGS="gconf-2.0"
fi
AC_SUBST([GCONFPKGS])

GLIB_GENMARSHAL=`$PKG_CONFIG --variable=glib_genmarshal glib-2.0`
AC_SUBST(GLIB_GENMARSHAL)

AH_TEMPLATE([ENABLE_UNIHAN], [Define if you want CJK ideograph information])
AC_ARG_ENABLE(unihan, 
              AC_HELP_STRING([--disable-unihan], 
                             [don't build in CJK ideograph information]))
if test "x$enable_unihan" != "xno" ; then
    AC_DEFINE([ENABLE_UNIHAN],[1],[Define if you want CJK ideograph information])
fi

# Before making a release, the LIBGUCHARMAP_LT_VERSION string should be modified.
# The string is of the form C:R:A.
# - If interfaces have been changed or added, but binary compatibility has
#   been preserved, change to C+1:0:A+1
# - If binary compatibility has been broken (eg removed or changed interfaces)
#   change to C+1:0:0
# - If the interface is the same as the previous version, change to C:R+1:A
LIBGUCHARMAP_LT_VERSION=6:2:0
AC_SUBST([LIBGUCHARMAP_LT_VERSION])

AC_CONFIG_FILES([
gucharmap.pc
Makefile
gucharmap/Makefile 
po/Makefile.in
pixmaps/Makefile 
help/Makefile
help/it/Makefile
help/ja/Makefile
help/zh_CN/Makefile
help/zh_TW/Makefile
gucharmap.spec
gucharmap.nsi])

AC_OUTPUT
