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
## 59 Temple Place, Suite 330, Boston, MA 02110-1301  USA
##

NULL =

localedir = $(datadir)/locale

BUILT_SOURCES = gucharmap-marshal.c gucharmap-marshal.h

lib_LTLIBRARIES = libgucharmap.la

libgucharmap_la_SOURCES = \
	gucharmap-block-chapters-model.c gucharmap-block-chapters-model.h \
	gucharmap-chapters-model.c gucharmap-chapters-model.h \
	gucharmap-chapters-view.c gucharmap-chapters-view.h \
	gucharmap-charmap.c gucharmap-charmap.h \
	gucharmap-chartable-accessible.c gucharmap-chartable-accessible.h \
	gucharmap-chartable-cell-accessible.c gucharmap-chartable-cell-accessible.h \
	gucharmap-chartable.c gucharmap-chartable.h gucharmap-chartable-private.h \
	gucharmap-codepoint-list.c gucharmap-codepoint-list.h \
	gucharmap-init.c gucharmap-init.h \
	gucharmap-marshal.c gucharmap-marshal.h \
	gucharmap-private.h \
	gucharmap-script-chapters-model.c gucharmap-script-chapters-model.h \
	gucharmap-script-codepoint-list.c gucharmap-script-codepoint-list.h \
	gucharmap-unicode-info.c gucharmap-unicode-info.h \
	unicode-blocks.h unicode-categories.h unicode-names.h \
	unicode-nameslist.h unicode-scripts.h unicode-unihan.h \
	$(NULL)

libgucharmap_la_CPPFLAGS = \
	-I$(top_srcdir)			\
	-DGUCHARMAP_COMPILATION		\
	-DDATADIR=\"$(datadir)\"	\
	-DLOCALEDIR=\"$(localedir)\"	\
	$(DISABLE_DEPRECATED)		\
	$(AM_CPPFLAGS)

libgucharmap_la_CFLAGS = \
	$(PANGO_CFLAGS)	\
	$(GTK_CFLAGS)	\
	$(WARN_CFLAGS)	\
	$(AM_CFLAGS)

libgucharmap_la_LDFLAGS = \
	-version-info $(LIBGUCHARMAP_LT_VERSION)	\
	-export-symbols-regex "^gucharmap_*"		\
	$(AM_LDFLAGS)

libgucharmap_la_LIBADD = \
	$(GTK_LIBS)	\
	$(INTL_LIBS)

libgucharmapincludedir = $(includedir)/gucharmap-2/gucharmap
libgucharmapinclude_HEADERS = \
	gucharmap-block-chapters-model.h \
	gucharmap-chapters-model.h \
	gucharmap-chapters-view.h \
	gucharmap-charmap.h \
	gucharmap-chartable.h \
	gucharmap-codepoint-list.h \
	gucharmap.h \
	gucharmap-init.h \
	gucharmap-script-chapters-model.h \
	gucharmap-script-codepoint-list.h \
	gucharmap-types.h \
	gucharmap-unicode-info.h \
	$(NULL)

bin_PROGRAMS = gucharmap

gucharmap_SOURCES = \
	gucharmap-mini-fontsel.c \
	gucharmap-mini-fontsel.h \
	gucharmap-search-dialog.c \
	gucharmap-search-dialog.h \
	gucharmap-settings.c \
	gucharmap-settings.h \
	gucharmap-window.c \
 	gucharmap-window.h \
	main.c \
	$(NULL)

gucharmap_CPPFLAGS = \
	-I$(top_srcdir)			\
	-DDATADIR=\"$(datadir)\"	\
	-DLOCALEDIR=\"$(localedir)\"	\
	$(DISABLE_DEPRECATED)		\
	$(AM_CPPFLAGS)

gucharmap_CFLAGS = \
	$(PANGO_CFLAGS)	\
	$(GTK_CFLAGS)	\
	$(GCONF_CFLAGS)	\
	$(WARN_CFLAGS)	\
	$(AM_CFLAGS)

gucharmap_LDFLAGS = \
	$(AM_LDFLAGS)

gucharmap_LDADD = \
	libgucharmap.la	\
	$(PANGO_LIBS)	\
	$(GTK_LIBS)	\
	$(GCONF_LIBS)	\
	$(INTL_LIBS)

if OS_WIN32
gucharmap_LDADD += gucharmaprc.o
gucharmaprc.o: gucharmaprc.rc
	windres -i $< -o $@
endif

gucharmap-marshal.h: $(srcdir)/gucharmap-marshal.list
	( @GLIB_GENMARSHAL@ --prefix=_gucharmap_marshal $(srcdir)/gucharmap-marshal.list \
                            --header \
                            --internal > gucharmap-marshal.h.tmp \
	  && mv gucharmap-marshal.h.tmp gucharmap-marshal.h ) \
        || ( rm -f gucharmap-marshal.h.tmp && exit 1 )

gucharmap-marshal.c: $(srcdir)/gucharmap-marshal.list
	( @GLIB_GENMARSHAL@ --prefix=_gucharmap_marshal $(srcdir)/gucharmap-marshal.list \
			    --header \
                            --body \
                            --internal > gucharmap-marshal.c.tmp \
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
