## $Id$
##
## Copyright (c) 2002  Noah Levitt <nlevitt@users.sourceforge.net>
##
## This program is free software; you can redistribute it and/or modify it
## under the terms of the GNU General Public License as published by the
## Free Software Foundation; either version 2 of the License, or (at your
## option) any later version.
##
## This program is distributed in the hope that it will be useful, but
## WITHOUT ANY WARRANTY; without even the implied warranty of
## MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
## General Public License for more details.
##
## You should have received a copy of the GNU General Public License along
## with this program; if not, write to the Free Software Foundation, Inc.,
## 59 Temple Place, Suite 330, Boston, MA 02111-1307  USA
##

AM_CFLAGS = $(GUCHARMAP_CFLAGS) -Wall -ansi 

localedir = $(datadir)/locale
AM_CPPFLAGS = -DLOCALEDIR=\"$(localedir)\"

libgucharmap_la_LIBADD = @GUCHARMAP_LIBS@

lib_LTLIBRARIES = libgucharmap.la

libgucharmap_la_SOURCES = charmap.c charmap.h \
                          unicode_data.c unicode_info.c \
                          unicode_info.h unihan.c \
                          gucharmap_intl.c gucharmap_intl.h

# this is different from the project version
# http://sources.redhat.com/autobook/autobook/autobook_91.html
libgucharmap_la_LDFLAGS = -version-info 1:0:0


libgucharmapincludedir = $(includedir)/gucharmap
libgucharmapinclude_HEADERS = charmap.h unicode_info.h


bin_PROGRAMS = gucharmap

gucharmap_LDADD = libgucharmap.la  @GUCHARMAP_LIBS@

gucharmap_SOURCES = main.c

