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

distuninstallcheck_listfiles = find . -type -f -print | grep -v scrollkeeper

distcleancheck_listfiles = find . -type -f print | grep -v 'omf\.out'

AUTOMAKE_OPTIONS = foreign

desktopdir 	 = $(datadir)/applications
desktop_in_files = gucharmap.desktop.in
desktop_DATA 	 = $(desktop_in_files:.desktop.in=.desktop)
@INTLTOOL_DESKTOP_RULE@

SUBDIRS = po pixmaps gucharmap help

EXTRA_DIST = gtkrc.win32 gucharmap.nsi.in gucharmap.pc.in \
             intltool-extract.in intltool-merge.in intltool-update.in \
             gucharmap.desktop.in gucharmap.spec.in $(desktop_DATA)

pkgconfigdir = $(libdir)/pkgconfig
pkgconfig_DATA = gucharmap.pc

DISTCLEANFILES = \
	intltool-update \
	intltool-extract \
	intltool-merge

