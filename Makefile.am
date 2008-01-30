## $Id: Makefile.am,v 1.21 2006/02/12 00:07:14 behdad Exp $
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

SUBDIRS = po pixmaps gucharmap

if HAVE_GNOME_DOC_UTILS
SUBDIRS += help
endif

distuninstallcheck_listfiles = find . -type -f -print | grep -v scrollkeeper

distcleancheck_listfiles = find . -type -f print | grep -v 'omf\.out'

desktopdir 	 = $(datadir)/applications
desktop_in_files = gucharmap.desktop.in
desktop_DATA 	 = $(desktop_in_files:.desktop.in=.desktop)
@INTLTOOL_DESKTOP_RULE@

schemadir         = $(GCONF_SCHEMA_FILE_DIR)
schema_in_files   = gucharmap.schemas.in
schema_DATA       = $(schema_in_files:.schemas.in=.schemas)
@INTLTOOL_SCHEMAS_RULE@

EXTRA_DIST = gtkrc.win32 gucharmap.nsi.in gucharmap.pc.in \
             intltool-extract.in intltool-merge.in intltool-update.in \
             gnome-doc-utils.make \
             gucharmap.desktop.in gucharmap.spec.in $(desktop_DATA) \
	     $(schema_in_files) COPYING.UNICODE

pkgconfigdir = $(libdir)/pkgconfig
pkgconfig_DATA = gucharmap.pc

DISTCHECK_CONFIGURE_FLAGS = --disable-scrollkeeper --disable-schemas-install

DISTCLEANFILES = \
        gnome-doc-utils.make \
	intltool-update \
	intltool-extract \
	intltool-merge \
	$(schema_DATA)

if GCONF_SCHEMAS_INSTALL
install-data-local:
	if test -z "$(DESTDIR)" ; then \
		GCONF_CONFIG_SOURCE=$(GCONF_SCHEMA_CONFIG_SOURCE) $(GCONFTOOL) --makefile-install-rule $(top_builddir)/$(schema_DATA); \
	fi
else
install-data-local:
endif
