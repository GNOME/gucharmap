include $(top_srcdir)/gnome-doc-utils.make
dist-hook: doc-dist-hook

DOC_MODULE = gucharmap
DOC_ENTITIES = legal.xml
DOC_INCLUDES = 

DOC_LINGUAS = bg ca cs de el en_GB es fi fr gl ko oc pt_BR ru sl sv vi zh_CN

# These should be converted to use gnome-doc-utils
# http://live.gnome.org/GnomeDocUtilsTranslationMigration
# Please remember also to remove help/langcode/Makefile from toplevel
# configure.ac's AC_CONFIG_FILES, and remove your directory from
# the SUBDIRS below.
#
# Only continue to support these for gtk 2

if HAVE_GTK_2
SUBDIRS = it ja zh_HK zh_TW
endif

-include $(top_srcdir)/git.mk
