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

AC_PREREQ([2.50])
AC_INIT([gucharmap], [0.8.0])
AM_CONFIG_HEADER(config.h)
AM_INIT_AUTOMAKE(gucharmap, 0.8.0)

AM_MAINTAINER_MODE

# checks for progs
AC_PROG_CC
AC_PROG_LIBTOOL
AC_PROG_INTLTOOL([0.22])

AC_ARG_ENABLE(gtk-immodules,
              AC_HELP_STRING([--disable-gtk-immodules],
                             [don't generate gtk.immodules]))
AM_CONDITIONAL(BUILD_GTK_IMMODULES, test "x$enable_gtk_immodules" != "xno")

if test "x$enable_gtk_immodules" != "xno" ;
then
    AC_CHECK_PROGS([GTK_QUERY_IMMODULES], [gtk-query-immodules-2.0])

    if test "x$GTK_QUERY_IMMODULES" = "x" ;
    then
        AC_MSG_ERROR([gtk-query-immodules-2.0 not found.])
    fi
fi


# checks for stuff
GETTEXT_PACKAGE=gucharmap
AC_SUBST(GETTEXT_PACKAGE)
AC_DEFINE_UNQUOTED(GETTEXT_PACKAGE, "$GETTEXT_PACKAGE", 
                   [Define the gettext package to be used])
ALL_LINGUAS="am be cs da de el eo es fa fi fr ga it ja ko mk ms nl no pl pt pt_BR ru sv uk zh_TW zh_CN"
AM_GLIB_GNU_GETTEXT


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
AH_TEMPLATE([UNICHAR_MAX], [The last unicode character we support])
PKG_CHECK_MODULES(PANGO, pango >= 1.2.1, full_unicode=yes, full_unicode=no)
if test "x$full_unicode" = "xyes" ; then
    AC_DEFINE(UNICHAR_MAX, 0x0010ffff)
else
    AC_DEFINE(UNICHAR_MAX, 0x0000ffff)
fi


# gtk is required
PKG_CHECK_MODULES(GTK, gtk+-2.0 >= 2.2.0)
GTK_BINARY_VERSION=`pkg-config --variable gtk_binary_version gtk+-2.0`

AC_ARG_ENABLE(gnome,
              AC_HELP_STRING([--disable-gnome],
	                     [don't build with GNOME support]))
if test "x$enable_gnome" != "xno"; then
    PKG_CHECK_MODULES(GNOME, libgnomeui-2.0 >= 2.2.0 libgnome-2.0 >= 2.2.0, 
                    have_gnome=yes, have_gnome=no)
else
    AC_MSG_NOTICE([GNOME support disabled with --disable-gnome])
    have_gnome=no
fi  

GNOMEPKGS=""
if test "x$have_gnome" = "xno" 
then
    libs_bak="$LIBS"
    LIBS=""

    AC_CHECK_LIB(popt, poptGetNextOpt,, AC_MSG_ERROR([popt is required]))

    POPT_LIBS="$LIBS"
    LIBS="$libs_bak"
    
    AC_SUBST(POPT_LIBS)
else
	GNOMEPKGS="libgnomeui-2.0 libgnome-2.0"
fi
AC_SUBST(GNOMEPKGS)

# checks for funcs 
AC_CHECK_FUNCS([bind_textdomain_codeset])


AC_SUBST(GTK_CFLAGS)
AC_SUBST(GTK_LIBS)
AC_SUBST(GTK_BINARY_VERSION)

AC_SUBST(GNOME_CFLAGS)
AC_SUBST(GNOME_LIBS)

AH_TEMPLATE([HAVE_GNOME], [Define if have gnome])
if test "x$have_gnome" = "xyes" ; then
    AC_DEFINE(HAVE_GNOME)
fi


GLIB_GENMARSHAL=`pkg-config --variable=glib_genmarshal glib-2.0`
AC_SUBST(GLIB_GENMARSHAL)


AH_TEMPLATE([ENABLE_UNIHAN], [Define if you want CJK ideograph information])
AC_ARG_ENABLE(unihan, 
              AC_HELP_STRING([--disable-unihan], 
                             [don't build in CJK ideograph information]))
if test "x$enable_unihan" != "xno" ; then
    AC_DEFINE(ENABLE_UNIHAN)
fi


AC_CONFIG_FILES([
gucharmap.pc
Makefile
gucharmap/Makefile 
gucharmap/unicode/Makefile 
po/Makefile.in
pixmaps/Makefile 
gucharmap.spec
gucharmap.nsi])

AC_OUTPUT
