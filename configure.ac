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
m4_define([gucharmap_version_micro],[3])
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

AM_MAINTAINER_MODE

# checks for progs
AC_PROG_CC
AC_DISABLE_STATIC
AC_PROG_LIBTOOL
IT_PROG_INTLTOOL([0.39.99])

AM_PROG_CC_C_O

# checks for stuff
GNOME_COMMON_INIT
GNOME_DEBUG_CHECK
GNOME_COMPILE_WARNINGS([maximum])
GNOME_MAINTAINER_MODE_DEFINES

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

GLIB_REQUIRED=2.16.3
GTK_REQUIRED=2.12.0
PKG_CHECK_MODULES([GTK],[glib-2.0 >= $GLIB_REQUIRED gtk+-2.0 >= $GTK_REQUIRED])

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

# ****
# i18n
# ****

GETTEXT_PACKAGE=gucharmap
AC_SUBST([GETTEXT_PACKAGE])
AC_DEFINE_UNQUOTED([GETTEXT_PACKAGE],["$GETTEXT_PACKAGE"],[The gucharmap gettext domain])
AM_GLIB_GNU_GETTEXT

# ********
# Bindings
# ********

AC_MSG_CHECKING([whether python bindings are requested])
AC_ARG_ENABLE([python-bindings],
        AS_HELP_STRING([--enable-python-bindings],[Enable python bindings]),
        [],[enable_python_bindings=no])
AC_MSG_RESULT([$enable_python_bindings])

if test "$enable_python_bindings" = "yes"; then
  AM_PATH_PYTHON([2.4])
  AM_CHECK_PYTHON_HEADERS([],[AC_MSG_ERROR([Python headers not found])])

  PY_PREFIX=`$PYTHON -c 'import sys ; print sys.prefix'`
  PY_EXEC_PREFIX=`$PYTHON -c 'import sys ; print sys.exec_prefix'`
  PYTHON_LIBS="-lpython$PYTHON_VERSION"
  PYTHON_LIB_LOC="-L$PY_EXEC_PREFIX/lib/python$PYTHON_VERSION/config"
  PYTHON_CFLAGS="-I$PY_PREFIX/include/python$PYTHON_VERSION"
  PYTHON_MAKEFILE="$PY_EXEC_PREFIX/lib/python$PYTHON_VERSION/config/Makefile"
  PYTHON_LOCALMODLIBS=`sed -n -e 's/^LOCALMODLIBS=\(.*\)/\1/p' $PYTHON_MAKEFILE`
  PYTHON_BASEMODLIBS=`sed -n -e 's/^BASEMODLIBS=\(.*\)/\1/p' $PYTHON_MAKEFILE`
  PYTHON_OTHER_LIBS=`sed -n -e 's/^LIBS=\(.*\)/\1/p' $PYTHON_MAKEFILE`
  PYTHON_EXTRA_LIBS="$PYTHON_LOCALMODLIBS $PYTHON_BASEMODLIBS $PYTHON_OTHER_LIBS"
  AC_SUBST([PYTHON_LIBS])
  AC_SUBST([PYTHON_LIB_LOC])
  AC_SUBST([PYTHON_CFLAGS])
  AC_SUBST([PYTHON_EXTRA_LIBS])

  dnl FIXME: do we really need this test?
  AC_MSG_CHECKING([whether we can build a shared library depending on libpython])
  rm -rf testpython
  mkdir testpython
  cd testpython
  cat > testpython.c <<EOF
#include <Python.h>
int testpython (void)
{
Py_Exit (0);
}
EOF
  if /bin/sh ../libtool --mode=compile ${CC} $PYTHON_CFLAGS -c testpython.c >/dev/null 2>&1 && \
          /bin/sh ../libtool --mode=link ${CC} -o testpython.la -rpath `pwd` -module -avoid-version $PYTHON_LIB_LOC testpython.lo $PYTHON_LIBS $PYTHON_EXTRA_LIBS >/dev/null 2>&1 && \
          grep 'dlname.*testpython' testpython.la >/dev/null 2>&1; then
          AC_MSG_RESULT([yes])
  else
          AC_MSG_ERROR([Can't link to libpython])
  fi
  cd ..
  rm -rf testpython
  
  PYGTK_REQUIRED=2.7.1
  PKG_CHECK_MODULES([PYGTK],[pygtk-2.0 >= $PYGTK_REQUIRED])

  AC_SUBST([PYGTK_CFLAGS])
  AC_SUBST([PYGTK_LIBS])

  AC_MSG_CHECKING([for pygtk defs])
  PYGTK_DEFSDIR=`$PKG_CONFIG --variable=defsdir pygtk-2.0`
  AC_MSG_RESULT([$PYGTK_DEFSDIR])

  AC_MSG_CHECKING([for pygtk codegen])
  PYGTK_CODEGEN="$PYTHON `$PKG_CONFIG --variable=codegendir pygtk-2.0`/codegen.py"
  AC_MSG_RESULT([$PYGTK_CODEGEN])

  AC_MSG_CHECKING([for pygtk h2def])
  PYGTK_H2DEF="$PYTHON `$PKG_CONFIG --variable=codegendir pygtk-2.0`/h2def.py"
  AC_MSG_RESULT([$PYGTK_H2DEF])

  AC_SUBST([PYGTK_DEFSDIR])
  AC_SUBST([PYGTK_CODEGEN])
  AC_SUBST([PYGTK_H2DEF])

  dnl Check for -fno-strict-aliasing
  FLAGS="-fno-strict-aliasing"
  save_CFLAGS="$CFLAGS"
  CFLAGS="$CFLAGS $FLAGS"
  AC_MSG_CHECKING([whether [$]CC understands $FLAGS])
  AC_TRY_COMPILE([], [], [compiler_has_option=yes], [compiler_has_option=no])
  CFLAGS="$save_CFLAGS"
  AC_MSG_RESULT($compiler_has_option)
  if test $compiler_has_option = yes; then
          NO_STRICT_ALIASING_CFLAGS="$FLAGS"
  fi
  AC_SUBST([NO_STRICT_ALIASING_CFLAGS])
fi

AM_CONDITIONAL([ENABLE_PYTHON_BINDINGS],[test "$enable_python_bindings" = "yes"])

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
gucharmap-2.pc
gucharmap.desktop.in
gucharmap.nsi
gucharmap.spec
bindings/Makefile
bindings/python/Makefile
docs/Makefile
docs/reference/Makefile
gucharmap/Makefile
gucharmap/gucharmap.h
help/Makefile
help/it/Makefile
help/ja/Makefile
help/zh_CN/Makefile
help/zh_TW/Makefile
pixmaps/Makefile
po/Makefile.in
])

AC_OUTPUT
