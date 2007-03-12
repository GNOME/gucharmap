## $Id: Makefile.am,v 1.82 2006/09/04 22:41:10 behdad Exp $
##
## Copyright (c) 2003  Noah Levitt
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

AM_CFLAGS = $(PANGO_CFLAGS) $(GTK_CFLAGS) $(GNOME_CFLAGS)

localedir = $(datadir)/locale
AM_CPPFLAGS = -I$(top_srcdir) \
              -DDATADIR=\"$(datadir)\" \
              -DLOCALEDIR=\"$(localedir)\"
## -DGTK_DISABLE_DEPRECATED=1

libgucharmap_la_LIBADD = @GTK_LIBS@ @GNOME_LIBS@

lib_LTLIBRARIES = libgucharmap.la

BUILT_SOURCES = gucharmap-marshal.c gucharmap-marshal.h

libgucharmap_la_SOURCES = gucharmap-marshal.c gucharmap-marshal.h \
                          gucharmap-intl.c gucharmap-intl.h \
                          chartable_accessible.c chartable_accessible.h \
                          charcell_accessible.c charcell_accessible.h \
                          gucharmap-charmap.c gucharmap-charmap.h \
                          gucharmap-mini-fontsel.c gucharmap-mini-fontsel.h \
                          gucharmap-table.c gucharmap-table.h gucharmap-table-private.h \
                          gucharmap-unicode-info.c gucharmap-unicode-info.h \
                          gucharmap-window.c gucharmap-window.h \
			  gucharmap-codepoint-list.c gucharmap-codepoint-list.h \
			  gucharmap-script-codepoint-list.c gucharmap-script-codepoint-list.h \
			  gucharmap-chapters.c gucharmap-chapters.h \
			  gucharmap-block-chapters.c gucharmap-block-chapters.h \
			  gucharmap-script-chapters.c gucharmap-script-chapters.h \
			  unicode-blocks.h unicode-categories.h unicode-names.h \
			  unicode-nameslist.h unicode-scripts.h unicode-unihan.h \
			  gucharmap-search-dialog.h gucharmap-search-dialog.c

# this is different from the project version
libgucharmap_la_LDFLAGS = -version-info 6:1:0

libgucharmapincludedir = $(includedir)/gucharmap
libgucharmapinclude_HEADERS = gucharmap.h \
                              gucharmap-charmap.h \
                              gucharmap-table.h \
                              gucharmap-window.h \
                              gucharmap-mini-fontsel.h \
                              gucharmap-unicode-info.h \
			      gucharmap-codepoint-list.h \
			      gucharmap-script-codepoint-list.h \
			      gucharmap-block-chapters.h \
			      gucharmap-chapters.h \
			      gucharmap-script-chapters.h \
			      gucharmap-search-dialog.h

bin_PROGRAMS = gucharmap
gucharmap_SOURCES = main.c
gucharmap_LDADD = libgucharmap.la @PANGO_LIBS@ @GTK_LIBS@ @GNOME_LIBS@

if OS_WIN32
gucharmap_LDADD += gucharmaprc.o
gucharmaprc.o: gucharmaprc.rc
	windres -i $< -o $@
else
gucharmap_LDADD += @GNOME_LIBS@
endif

gucharmap-marshal.h: $(srcdir)/gucharmap-marshal.list
	( @GLIB_GENMARSHAL@ --prefix=_gucharmap_marshal $(srcdir)/gucharmap-marshal.list \
                            --header > gucharmap-marshal.h.tmp \
	  && mv gucharmap-marshal.h.tmp gucharmap-marshal.h ) \
        || ( rm -f gucharmap-marshal.h.tmp && exit 1 )

gucharmap-marshal.c: $(srcdir)/gucharmap-marshal.list
	( @GLIB_GENMARSHAL@ --prefix=_gucharmap_marshal $(srcdir)/gucharmap-marshal.list \
			    --header \
                            --body > gucharmap-marshal.c.tmp \
          && mv gucharmap-marshal.c.tmp gucharmap-marshal.c ) \
        || ( rm -f gucharmap-marshal.c.tmp && exit 1 )

# don't do the symlink stuff if this is windows
if OS_WIN32

install-exec-hook:
        
uninstall-local:

else

# install a symlink to the binary called "charmap"
install-exec-hook:
	test -e "$(DESTDIR)$(bindir)/charmap" \
        || (cd "$(DESTDIR)$(bindir)" && ln -s gucharmap charmap) ; \
	test -e "$(DESTDIR)$(bindir)/gnome-character-map" \
        || (cd "$(DESTDIR)$(bindir)" && ln -s gucharmap gnome-character-map)

# remove bin/charmap if it's a symlink (it's a pain to test what it's
# a symlink to)
uninstall-local:
	test -h "$(DESTDIR)$(bindir)/charmap" \
	&& rm -f "$(DESTDIR)$(bindir)/charmap" ; \
	test -h "$(DESTDIR)$(bindir)/gnome-character-map" \
	&& rm -f "$(DESTDIR)$(bindir)/gnome-character-map"

endif

CLEANFILES = gucharmap-marshal.c gucharmap-marshal.h

EXTRA_DIST = gucharmap-marshal.list gucharmaprc.rc 
