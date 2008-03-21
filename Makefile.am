NULL =

ACLOCAL_AMFLAGS = -I m4

SUBDIRS = po pixmaps gucharmap bindings docs

if HAVE_GNOME_DOC_UTILS
SUBDIRS += help
endif

distuninstallcheck_listfiles = find . -type -f -print | grep -v scrollkeeper

distcleancheck_listfiles = find . -type -f print | grep -v 'omf\.out'

desktopdir = $(datadir)/applications
desktop_in_files = gucharmap.desktop.in.in
desktop_DATA 	 = $(desktop_in_files:.desktop.in.in=.desktop)
@INTLTOOL_DESKTOP_RULE@

schemadir         = $(GCONF_SCHEMA_FILE_DIR)
schema_in_files   = gucharmap.schemas.in
schema_DATA       = $(schema_in_files:.schemas.in=.schemas)
@INTLTOOL_SCHEMAS_RULE@

EXTRA_DIST = \
	autogen.sh \
	gtkrc.win32 \
	intltool-extract.in \
	intltool-merge.in \
	intltool-update.in \
	gnome-doc-utils.make \
	$(schema_in_files) \
	COPYING.UNICODE \
	$(NULL)

pkgconfigdir = $(libdir)/pkgconfig
pkgconfig_DATA = gucharmap-2.pc

DISTCHECK_CONFIGURE_FLAGS = \
	--disable-scrollkeeper \
	--disable-schemas-install \
	--enable-gtk-doc

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
endif
