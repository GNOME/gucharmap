NULL =

appdatadir = $(datadir)/metainfo
appdata_in_files = gucharmap.appdata.xml.in
nodist_appdata_DATA = $(appdata_in_files:.xml.in=.xml)

EXTRA_DIST = \
	$(appdata_in_files) \
	$(NULL)

CLEANFILES = \
	$(nodist_appdata_DATA) \
	$(NULL)

DISTCLEANFILES = \
	$(nodist_appdata_DATA) \
	$(NULL)

@INTLTOOL_XML_RULE@

-include $(top_srcdir)/git.mk

