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

AM_CFLAGS = $(GTK_CFLAGS) $(GNOME_CFLAGS) 

localedir = $(datadir)/locale
AM_CPPFLAGS = -DLOCALEDIR=\"$(localedir)\" -DGTK_DISABLE_DEPRECATED=1 \
              -DICON_PATH=\"$(datadir)/pixmaps/gucharmap.png\"

libgucharmap_la_LIBADD = @GTK_LIBS@

lib_LTLIBRARIES = libgucharmap.la

libgucharmap_la_SOURCES = gucharmap_marshal.c gucharmap_marshal.h \
                          charmap.c charmap.h \
                          unicode_data.c unicode_info.c \
                          unicode_info.h unihan.c \
                          gucharmap_intl.c gucharmap_intl.h \
                          mini_fontsel.c mini_fontsel.h

# this is different from the project version
# http://sources.redhat.com/autobook/autobook/autobook_91.html
libgucharmap_la_LDFLAGS = -version-info 1:0:0


libgucharmapincludedir = $(includedir)/gucharmap
libgucharmapinclude_HEADERS = charmap.h unicode_info.h mini_fontsel.h


bin_PROGRAMS = gucharmap

gucharmap_SOURCES = main.c

if OS_WIN32

gucharmap_LDADD = gucharmaprc.o libgucharmap.la @GTK_LIBS@ @POPT_LIBS@

gucharmaprc.o: gucharmaprc.rc
	windres -i $< -o $@

else

gucharmap_LDADD = libgucharmap.la @GTK_LIBS@ @GNOME_LIBS@ @POPT_LIBS@

endif


gucharmap_marshal.h: gucharmap_marshal.list
	( @GLIB_GENMARSHAL@ --prefix=gucharmap_marshal gucharmap_marshal.list \
                            --header > gucharmap_marshal.tmp \
	  && mv gucharmap_marshal.tmp gucharmap_marshal.h ) \
        || ( rm -f gucharmap_marshal.tmp && exit 1 )

gucharmap_marshal.c: gucharmap_marshal.h
	( @GLIB_GENMARSHAL@ --prefix=gucharmap_marshal gucharmap_marshal.list \
                            --body > gucharmap_marshal.tmp \
          && mv gucharmap_marshal.tmp gucharmap_marshal.c ) \
        || ( rm -f gucharmap_marshal.tmp && exit 1 )


# don't do the symlink stuff if this is windows
if OS_WIN32

install-exec-hook:
        

uninstall-local:


else

# install a symlink to the binary called "charmap"
install-exec-hook:
	test -e "$(DESTDIR)$(bindir)/charmap" \
        || ln -s "$(DESTDIR)$(bindir)/gucharmap" "$(DESTDIR)$(bindir)/charmap"

# remove bin/charmap if it's a symlink (it's a pain to test what it's
# a symlink to)
uninstall-local:
	test -h "$(DESTDIR)$(bindir)/charmap" \
	&& rm -f "$(DESTDIR)$(bindir)/charmap"

endif


EXTRA_DIST = gucharmap_marshal.list gucharmaprc.rc
