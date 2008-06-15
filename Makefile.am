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


# Build ChangeLog from GIT or SVN history
# (SVN r1521 was the first ChangeLog-less checkin).
# Only build this when in a git or svn checkout.

ChangeLog:
	@if test -f $(top_srcdir)/.git/HEAD; then \
		git log --stat > $@; \
	elif test -f $(top_srcdir)/.svn/entries; then \
		svn log -v --xml -r HEAD:1520 $(SVN_ROOT)/$(SVN_MODULE)/$(SVN_BRANCH) | \
			xsltproc --stringparam strip-prefix "$(SVN_MODULE)/$(SVN_BRANCH)" \
				 --stringparam include-rev "yes" $(top_srcdir)/svn2cl.xsl - > $@; \
	fi

dist: ChangeLog

.PHONY: ChangeLog

@INTLTOOL_DESKTOP_RULE@
@INTLTOOL_SCHEMAS_RULE@
