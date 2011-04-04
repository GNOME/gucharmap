# Copyright © 2003  Noah Levitt
# Copyright © 2007, 2008, 2009 Christian Persch
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

m4_define([gucharmap_version_major],[3])
m4_define([gucharmap_version_minor],[0])
m4_define([gucharmap_version_micro],[1])
m4_define([gucharmap_version_extra],[])
m4_define([gucharmap_version],[gucharmap_version_major.gucharmap_version_minor.gucharmap_version_micro()gucharmap_version_extra])

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

AC_CONFIG_MACRO_DIR([m4])
AC_CONFIG_SRCDIR([gucharmap/gucharmap.h.in])
AC_CONFIG_HEADERS([config.h])

AM_INIT_AUTOMAKE([1.9 foreign dist-bzip2 no-dist-gzip])

m4_ifdef([AM_SILENT_RULES],[AM_SILENT_RULES([yes])])

if test -z "$enable_maintainer_mode"; then
  enable_maintainer_mode=yes
fi
AM_MAINTAINER_MODE([enable])

# checks for progs
AC_PROG_CC
AC_DISABLE_STATIC
AC_PROG_LIBTOOL
IT_PROG_INTLTOOL([0.40.0])

AM_PROG_CC_C_O

# checks for stuff
GNOME_COMMON_INIT
GNOME_DEBUG_CHECK
GNOME_COMPILE_WARNINGS([maximum])
#GNOME_MAINTAINER_MODE_DEFINES

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

# Which gtk+ API version to compile against

AC_MSG_CHECKING([which gtk+ version to compile against])
AC_ARG_WITH([gtk],
  [AS_HELP_STRING([--with-gtk=2.0|3.0],[which gtk+ version to compile against (default: 2.0)])],
  [case "$with_gtk" in
     2.0|3.0) ;;
     *) AC_MSG_ERROR([invalid gtk version specified]) ;;
   esac],
  [with_gtk=2.0])
AC_MSG_RESULT([$with_gtk])

case "$with_gtk" in
  2.0) GTK_API_VERSION=2.0
       GTK_REQUIRED=2.14.0
       GUCHARMAP_API_VERSION=2.0
       GUCHARMAP_API_MAJOR_VERSION=2
       GUCHARMAP_API_MINOR_VERSION=0
       GUCHARMAP_API_PC_VERSION=2
       GUCHARMAP_LIBRARY_SUFFIX=
       ;;
  3.0) GTK_API_VERSION=3.0
       GTK_REQUIRED=3.0.0
       GUCHARMAP_API_VERSION=2.90
       GUCHARMAP_API_MAJOR_VERSION=2
       GUCHARMAP_API_MINOR_VERSION=90
       GUCHARMAP_API_PC_VERSION=2.90
       GUCHARMAP_LIBRARY_SUFFIX="-$GUCHARMAP_API_VERSION"
       ;;
esac

AC_SUBST([GTK_API_VERSION])
AC_SUBST([GUCHARMAP_API_VERSION])
AC_SUBST([GUCHARMAP_API_VERSION_U],[AS_TR_SH([$GUCHARMAP_API_VERSION])])
AC_SUBST([GUCHARMAP_API_MAJOR_VERSION])
AC_SUBST([GUCHARMAP_API_MINOR_VERSION])
AC_SUBST([GUCHARMAP_API_PC_VERSION])
AC_SUBST([GUCHARMAP_LIBRARY_SUFFIX_U],[AS_TR_SH([$GUCHARMAP_LIBRARY_SUFFIX])])

AM_CONDITIONAL([HAVE_GTK_2],[test "$with_gtk" = "2.0"])
AM_CONDITIONAL([HAVE_GTK_3],[test "$with_gtk" = "3.0"])

GLIB_REQUIRED=2.16.3

PKG_CHECK_MODULES([GTK],[
  glib-2.0 >= $GLIB_REQUIRED
  gtk+-$GTK_API_VERSION >= $GTK_REQUIRED
])

GLIB_GENMARSHAL="$($PKG_CONFIG --variable=glib_genmarshal glib-2.0)"
AC_SUBST([GLIB_GENMARSHAL])
GLIB_MKENUMS="$($PKG_CONFIG --variable=glib_mkenums glib-2.0)"
AC_SUBST([GLIB_MKENUMS])

AC_ARG_ENABLE([gconf],
    [AS_HELP_STRING([--disable-gconf],[don't build with gconf support])],
    [],[enable_gconf=yes])

if test "$enable_gconf" = "yes"; then
    GCONFPKGS="gconf-2.0 gthread-2.0"
    PKG_CHECK_MODULES([GCONF],[$GCONFPKGS])

    AC_DEFINE([HAVE_GCONF], [1], [Define if have gconf])
else
    GCONFPKGS=
    AC_MSG_NOTICE([GCONF support disabled with --disable-gconf])
fi  
AC_SUBST([GCONFPKGS])

AM_GCONF_SOURCE_2
AC_PATH_PROG([GCONFTOOL],[gconftool-2],[no])

AC_MSG_CHECKING([whether to build the charmap programme])
AC_ARG_ENABLE([charmap],
  [AS_HELP_STRING([--disable-charmap],[disable building the charmap programme])],
  [],[enable_charmap=yes])
AC_MSG_RESULT([$enable_charmap])

AM_CONDITIONAL([ENABLE_CHARMAP],[test "$enable_charmap" = "yes"])

# checks for funcs 
AC_CHECK_FUNCS([bind_textdomain_codeset])

AC_SUBST(GTK_CFLAGS)
AC_SUBST(GTK_LIBS)

AC_SUBST(GCONF_CFLAGS)
AC_SUBST(GCONF_LIBS)

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

# ****
# i18n
# ****

GETTEXT_PACKAGE=gucharmap
AC_SUBST([GETTEXT_PACKAGE])
AC_DEFINE_UNQUOTED([GETTEXT_PACKAGE],["$GETTEXT_PACKAGE"],[The gucharmap gettext domain])
AM_GLIB_GNU_GETTEXT

# *************
# Introspection
# *************

# No automagic please!
if test -z "$enable_introspection"; then
  enable_introspection=no
fi

GOBJECT_INTROSPECTION_CHECK([0.9.0])

# ***************
# API & User Docs
# ***************

GNOME_DOC_INIT([0.9.0],[have_gdu=yes],[have_gdu=no])
if test "$have_gdu" = "no"; then
	AC_MSG_WARN([GNOME doc utils not found; disabling user manual])
fi

GTK_DOC_CHECK([1.0])

# *****************************************************************************

AC_SUBST([GUCHARMAP_VERSION_MAJOR],[gucharmap_version_major])
AC_SUBST([GUCHARMAP_VERSION_MINOR],[gucharmap_version_minor])
AC_SUBST([GUCHARMAP_VERSION_MICRO],[gucharmap_version_micro])
AC_SUBST([LIBGUCHARMAP_LT_VERSION],[gucharmap_lt_version_info])
AC_SUBST([LIBGUCHARMAP_LT_CURRENT_MINUS_AGE],[gucharmap_lt_current_minus_age])

AC_SUBST([AM_CPPFLAGS])
AC_SUBST([AM_CFLAGS])
AC_SUBST([AM_LDFLAGS])

AC_CONFIG_FILES([
Makefile
gucharmap.desktop.in
gucharmap.nsi
gucharmap.spec
docs/Makefile
docs/reference/Makefile
gucharmap/Makefile
gucharmap/gucharmap.h
help/Makefile
help/it/Makefile
help/ja/Makefile
help/zh_HK/Makefile
help/zh_TW/Makefile
pixmaps/Makefile
po/Makefile.in
])

AC_CONFIG_FILES([gucharmap-${GUCHARMAP_API_PC_VERSION}.pc:gucharmap.pc.in],[GUCHARMAP_API_PC_VERSION=$GUCHARMAP_API_PC_VERSION])

AC_OUTPUT
