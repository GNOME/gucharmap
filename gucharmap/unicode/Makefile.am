## $Id$
##
## Copyright (c) 2003  Noah Levitt <nlevitt аt columbia.edu>
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


BUILT_SOURCES = unicode_data.cI unicode_unihan.cI unicode_blocks.cI \
		unicode_nameslist.cI

unicode_unihan.cI: generate_unihan
	./generator.sh unicode_unihan.cI

unicode_data.cI:
	./generator.sh unicode_data.cI

unicode_blocks.cI:
	./generator.sh unicode_blocks.cI

unicode_nameslist.cI: generate_nameslist
	./generator.sh unicode_nameslist.cI

noinst_PROGRAMS = generate_unihan generate_nameslist
generate_unihan_SOURCES = generate_unihan.c
generate_nameslist_SOURCES = generate_nameslist.c

CLEANFILES = unicode_unihan.cI.old unicode_data.cI.old \
             unicode_blocks.cI.old unicode_nameslist.cI.old

MAINTAINERCLEANFILES = unicode_unihan.cI unicode_data.cI unicode_blocks.cI \
                       unicode_nameslist.cI 


EXTRA_DIST = generate_nameslist.c generate_unihan.c \
             generator.sh unicode_data.cI unicode_unihan.cI \
             unicode_blocks.cI unicode_nameslist.cI



