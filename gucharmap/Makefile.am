## $Id$
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

AM_CFLAGS = $(GTK_CFLAGS) $(GNOME_CFLAGS)

localedir = $(datadir)/locale
AM_CPPFLAGS = -I$(top_srcdir) \
              -DDATADIR=\"$(datadir)\" \
              -DLOCALEDIR=\"$(localedir)\" \
              -DICON_PATH=\"$(datadir)/pixmaps/gucharmap.png\"
## -DGTK_DISABLE_DEPRECATED=1

libgucharmap_la_LIBADD = @GTK_LIBS@ @GNOME_LIBS@

lib_LTLIBRARIES = libgucharmap.la

libgucharmap_la_SOURCES = gucharmap-marshal.c gucharmap-marshal.h \
                          gucharmap-intl.c gucharmap-intl.h \
                          chartable_accessible.c chartable_accessible.h \
                          charcell_accessible.c charcell_accessible.h \
                          gucharmap-charmap.c gucharmap-charmap.h \
                          gucharmap-mini-fontsel.c gucharmap-mini-fontsel.h \
                          gucharmap-table.c gucharmap-table.h \
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
libgucharmap_la_LDFLAGS = -version-info 3:3:0

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

if OS_WIN32

gucharmap_LDADD = gucharmaprc.o libgucharmap.la @GTK_LIBS@ @POPT_LIBS@

gucharmaprc.o: gucharmaprc.rc
	windres -i $< -o $@

else

gucharmap_LDADD = libgucharmap.la @GTK_LIBS@ @GNOME_LIBS@ @POPT_LIBS@

endif


moduledir = $(libdir)/gtk-2.0/$(GTK_BINARY_VERSION)/immodules
module_LTLIBRARIES = im-gucharmap.la
im_gucharmap_la_LDFLAGS = -rpath $(moduledir) -avoid-version -module
im_gucharmap_la_SOURCES = imgucharmap.c
im_gucharmap_la_LIBADD = libgucharmap.la @GTK_LIBS@ @GNOME_LIBS@

gucharmap-marshal.h: gucharmap-marshal.list
	( @GLIB_GENMARSHAL@ --prefix=gucharmap_marshal gucharmap-marshal.list \
                            --header > gucharmap-marshal.tmp \
	  && mv gucharmap-marshal.tmp gucharmap-marshal.h ) \
        || ( rm -f gucharmap-marshal.tmp && exit 1 )

gucharmap-marshal.c: gucharmap-marshal.h
	( @GLIB_GENMARSHAL@ --prefix=gucharmap_marshal gucharmap-marshal.list \
                            --body > gucharmap-marshal.tmp \
          && mv gucharmap-marshal.tmp gucharmap-marshal.c ) \
        || ( rm -f gucharmap-marshal.tmp && exit 1 )

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

if BUILD_GTK_IMMODULES

all-local: gtk.immodules

noinst_PROGRAMS = im-module-file
im_module_file_SOURCES = im-module-file.c
im_module_file_LDADD = @GTK_LIBS@

gtk.immodules: $(module_LTLIBRARIES)
	$(GTK_QUERY_IMMODULES) $(module_LTLIBRARIES) > gtk.immodules ;

install-data-hook: im-module-file
	LD_LIBRARY_PATH=$(DESTDIR)$(libdir):$$LD_LIBRARY_PATH GTK_PATH=$$GTK_PATH:$(DESTDIR)$(libdir)/gtk-2.0 $(GTK_QUERY_IMMODULES) > `./im-module-file` ;

else

install-data-hook: 

all-local: 

endif

CLEANFILES = gucharmap-marshal.c gucharmap-marshal.h

EXTRA_DIST = gucharmap-marshal.list gucharmaprc.rc 

