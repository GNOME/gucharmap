NULL =

appdatadir = $(datadir)/appdata
appdata_in_files = gucharmap.appdata.xml.in
nodist_appdata_DATA = $(appdata_in_files:.xml.in=.xml)

APPDATA_VALIDATE_FLAGS = --nonet
appdata-validate: $(nodist_appdata_DATA)
	for f in $^; do \
		$(APPDATA_VALIDATE) $(APPDATA_VALIDATE_FLAGS) $$f; \
	done

#check-local: appdata-validate

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

