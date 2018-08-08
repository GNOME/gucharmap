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

gucharmap.appdata.xml: gucharmap.appdata.xml.in
	$(AM_V_GEN)$(MSGFMT) --xml --template $< -d $(top_srcdir)/po -o $@

-include $(top_srcdir)/git.mk

