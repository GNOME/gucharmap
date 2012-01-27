NULL =

ACLOCAL_AMFLAGS = -I m4 ${ACLOCAL_FLAGS}
GITIGNOREFILES = m4

SUBDIRS = po pixmaps gucharmap docs

if ENABLE_CHARMAP
if HAVE_GNOME_DOC_UTILS
SUBDIRS += help
endif
endif

distuninstallcheck_listfiles = find . -type -f -print | grep -v scrollkeeper

distcleancheck_listfiles = find . -type -f print | grep -v 'omf\.out'

desktopdir = $(datadir)/applications
desktop_in_files = gucharmap.desktop.in.in
desktop_DATA 	 = $(desktop_in_files:.desktop.in.in=.desktop)

pkgconfigdir = $(libdir)/pkgconfig
pkgconfig_DATA = gucharmap-$(GUCHARMAP_API_PC_VERSION).pc

EXTRA_DIST = \
	ChangeLog.pre-2-23 \
	ChangeLog.README \
	autogen.sh \
	gtkrc.win32 \
	gnome-doc-utils.make \
	org.gnome.gucharmap.schema.xml.in \
	COPYING.UNICODE \
	$(NULL)

DISTCHECK_CONFIGURE_FLAGS = \
	--disable-scrollkeeper \
	--disable-schemas-install \
	--enable-gtk-doc \
	--disable-introspection \
	--enable-charmap \
	--with-gtk=3.0

CLEANFILES = \
	$(desktop_DATA) \
	$(gsettings_schema) \
	$(NULL)

MAINTAINERCLEANFILES = \
	$(srcdir)/INSTALL \
	$(srcdir)/aclocal.m4 \
	$(srcdir)/autoscan.log \
	$(srcdir)/compile \
	$(srcdir)/config.guess \
	$(srcdir)/config.h.in \
	$(srcdir)/config.sub \
	$(srcdir)/configure.scan \
	$(srcdir)/depcomp \
	$(srcdir)/install-sh \
	$(srcdir)/ltmain.sh \
	$(srcdir)/missing \
	$(srcdir)/mkinstalldirs \
	$(srcdir)/omf.make \
	$(srcdir)/xmldocs.make \
	$(srcdir)/gtk-doc.make \
	$(srcdir)/gnome-doc-utils.make \
	`find "$(srcdir)" -type f -name Makefile.in -print`

ChangeLog:
	$(AM_V_GEN) if test -f $(top_srcdir)/.git/HEAD; then \
		git log --stat > $@; \
	fi

dist: ChangeLog

.PHONY: ChangeLog

@INTLTOOL_DESKTOP_RULE@

-include $(top_srcdir)/git.mk
