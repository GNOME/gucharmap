dnl $Id$
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

AC_INIT([GNOME Character Map],[1.5.1],[http://bugzilla.gnome.org/enter_bug.cgi?product=gucharmap],[gucharmap])

AC_CONFIG_SRCDIR([gucharmap/gucharmap.h])
AC_CONFIG_HEADERS([config.h])
AM_INIT_AUTOMAKE([1.9 foreign dist-bzip2 no-dist-gzip])

AM_MAINTAINER_MODE

# checks for progs
AC_PROG_CC
AC_DISABLE_STATIC
AC_PROG_LIBTOOL
AC_PROG_INTLTOOL([0.29])

# checks for stuff
GETTEXT_PACKAGE=gucharmap
AC_SUBST(GETTEXT_PACKAGE)
AC_DEFINE_UNQUOTED([GETTEXT_PACKAGE],["$GETTEXT_PACKAGE"],[Define the gettext package to be used])
ALL_LINGUAS="am ar az be bg bn bs ca cs cy da de el en_CA en_GB eo es et eu fa fi fr ga gl gu hi hr hu id is it ja ko lt mi mk ml mn ms nb ne nl nn no pa pl pt pt_BR ro ru rw sk sq sr sr@Latn sv ta th tr uk vi wa xh zh_CN zh_TW"
AM_GLIB_GNU_GETTEXT

GNOME_DOC_INIT

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

# if pango >= 1.2.1, UNICHAR_MAX = 0x10ffff

PKG_CHECK_MODULES(PANGO, pango >= 1.2.1, full_unicode=yes, full_unicode=no)
if test "x$full_unicode" = "xyes" ; then
    AC_DEFINE([UNICHAR_MAX],[0x0010ffff],[The last unicode character we support])
else
    AC_DEFINE([UNICHAR_MAX],[0x0000ffff],[The last unicode character we support])
fi

# gtk is required
PKG_CHECK_MODULES(GTK, gobject-2.0 >= 2.3.0 gtk+-2.0 >= 2.6.0)

AC_ARG_ENABLE(gnome,
              AC_HELP_STRING([--disable-gnome],
	                     [don't build with GNOME support]))
if test "x$enable_gnome" != "xno"; then
    PKG_CHECK_MODULES(GNOME, libgnomeui-2.0 >= 2.13.2 libgnome-2.0 >= 2.13.7, 
                    have_gnome=yes, have_gnome=no)
else
    AC_MSG_NOTICE([GNOME support disabled with --disable-gnome])
    have_gnome=no
fi  

GNOMEPKGS=""
SUB_DIRS="po pixmaps gucharmap"
if test "x$have_gnome" = "xyes" 
then
    GNOMEPKGS="libgnomeui-2.0 libgnome-2.0"
    SUB_DIRS="$SUB_DIRS help"
fi
AC_SUBST(GNOMEPKGS)
AC_SUBST(SUB_DIRS)

# checks for funcs 
AC_CHECK_FUNCS([bind_textdomain_codeset])

AC_SUBST(PANGO_CFLAGS)
AC_SUBST(PANGO_LIBS)

AC_SUBST(GTK_CFLAGS)
AC_SUBST(GTK_LIBS)

AC_SUBST(GNOME_CFLAGS)
AC_SUBST(GNOME_LIBS)

if test "x$have_gnome" = "xyes" ; then
    AC_DEFINE([HAVE_GNOME],[1],[Define if have gnome])
fi

GLIB_GENMARSHAL=`$PKG_CONFIG --variable=glib_genmarshal glib-2.0`
AC_SUBST(GLIB_GENMARSHAL)

AH_TEMPLATE([ENABLE_UNIHAN], [Define if you want CJK ideograph information])
AC_ARG_ENABLE(unihan, 
              AC_HELP_STRING([--disable-unihan], 
                             [don't build in CJK ideograph information]))
if test "x$enable_unihan" != "xno" ; then
    AC_DEFINE([ENABLE_UNIHAN],[1],[Define if you want CJK ideograph information])
fi

AC_CONFIG_FILES([
gucharmap.pc
Makefile
gucharmap/Makefile 
po/Makefile.in
pixmaps/Makefile 
help/Makefile
help/de/Makefile
help/fr/Makefile
help/it/Makefile
help/ja/Makefile
help/ko/Makefile
help/sv/Makefile
help/zh_CN/Makefile
help/zh_TW/Makefile
gucharmap.spec
gucharmap.nsi])

AC_OUTPUT
