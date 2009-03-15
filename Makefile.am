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

schemadir         = $(GCONF_SCHEMA_FILE_DIR)
schema_in_files   = gucharmap.schemas.in
schema_DATA       = $(schema_in_files:.schemas.in=.schemas)

pkgconfigdir = $(libdir)/pkgconfig
pkgconfig_DATA = gucharmap-2.pc

EXTRA_DIST = \
	ChangeLog.pre-2-23 \
	po/ChangeLog.pre-2-23 \
	help/ChangeLog.pre-2-23 \
	ChangeLog.README \
	autogen.sh \
	gtkrc.win32 \
	gnome-doc-utils.make \
	$(schema_in_files) \
	COPYING.UNICODE \
	$(NULL)

DISTCHECK_CONFIGURE_FLAGS = \
	--disable-scrollkeeper \
	--disable-schemas-install \
	--enable-gtk-doc

DISTCLEANFILES = \
        gnome-doc-utils.make \
	$(schema_DATA)

install-data-local: $(schema_DATA)
if GCONF_SCHEMAS_INSTALL
	if test -z "$(DESTDIR)" ; then \
		for p in $^ ; do \
			GCONF_CONFIG_SOURCE=$(GCONF_SCHEMA_CONFIG_SOURCE) $(GCONFTOOL) --makefile-install-rule $$p 2>&1 > /dev/null; \
		done \
	fi
endif


ChangeLog:
	if test -f $(top_srcdir)/.git/HEAD; then \
		git log --stat > $@; \
	fi

dist: ChangeLog

.PHONY: ChangeLog

@INTLTOOL_DESKTOP_RULE@
@INTLTOOL_SCHEMAS_RULE@
