#!/bin/sh
#
# $Id$
# 

#
# Copyright (c) 2003  Noah Levitt <nlevitt аt users.sourceforge.net>
#
# This program is free software; you can redistribute it and/or modify it
# under the terms of the GNU General Public License as published by the
# Free Software Foundation; either version 2 of the License, or (at your
# option) any later version.
#
# This program is distributed in the hope that it will be useful, but
# WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
# General Public License for more details.
#
# You should have received a copy of the GNU General Public License along
# with this program; if not, write to the Free Software Foundation, Inc.,
# 59 Temple Place, Suite 330, Boston, MA 02111-1307  USA
#

#
# This script gets files from unicode.org and generates
# unicode_data.cI 
# unicode_unihan.cI
# unicode_blocks.cI
# unicode_nameslist.cI
#

#
# set absolute paths on these if you need to
#
UNZIP=`which unzip`
WGET=`which wget`
SED=`which sed`
MV=`which mv`
RM=`which rm`
ECHO=`which echo`
MKDIR=`which mkdir`
AWK=`which awk`
TOUCH=`which touch`
EXPR=`which expr`
TRUE=" : "
CAT=`which cat`
BC=`which bc`
ICONV=`which iconv`
PERL=`which perl`

GENERATE_UNIHAN="./generate_unihan"
GENERATE_NAMESLIST="./generate_nameslist"
GENERATE_CATEGORIES="./generate_categories.pl"

unidir="$PWD/unicode.org"
srcdir="$PWD"

PATH=""


download()
{
    if [ "x$WGET" = "x" ] ; then
        $ECHO
        $ECHO "error: wget not found, can't download files"
        $ECHO
        $ECHO "You can download these files yourself and put them in $unidir:"
        $ECHO
        $ECHO "http://www.unicode.org/Public/UNIDATA/UnicodeData.txt"
        $ECHO "http://www.unicode.org/Public/UNIDATA/Unihan.zip"
        $ECHO "http://www.unicode.org/Public/UNIDATA/NamesList.txt"
        $ECHO "http://www.unicode.org/Public/UNIDATA/Blocks.txt"
        $ECHO
        exit 1
    fi

    $WGET --directory-prefix=$unidir \
          "http://www.unicode.org/Public/UNIDATA/$1" || exit 1
}


# reads from stdin, writes to stdout
write_unicode_data()
{
    $ECHO "/* unicode_data.cI */"
    $ECHO "/* THIS IS A GENERATED FILE. */"
    $ECHO "/* http://www.unicode.org/Public/UNIDATA/UnicodeData.txt */"
    $ECHO "" 
    $ECHO "const UnicodeData unicode_data[] ="
    $ECHO "{"

    $AWK -F';' '{print "  { 0x" $1 ", \"" $2 "\" },"}' 

    $ECHO "};"
}


write_blocks()
{
    $ECHO "/* unicode_blocks.cI */"
    $ECHO "/* THIS IS A GENERATED FILE. */"
    $ECHO "/* http://www.unicode.org/Public/UNIDATA/Blocks.txt */"
    $ECHO "" 
    $ECHO "const UnicodeBlock unicode_blocks[] ="
    $ECHO "{"

    while read line
    do
        case $line in
            "#"*) continue ;;
        esac 

        start=`$ECHO $line | $SED 's/^\([^.]*\)\.\.\([^;]*\); \(.*\)$/\1/'`
        end=`$ECHO $line | $SED 's/^\([^.]*\)\.\.\([^;]*\); \(.*\)$/\2/'`
        name=`$ECHO $line | $SED 's/^\([^.]*\)\.\.\([^;]*\); \(.*\)$/\3/'`

        $ECHO "  { 0x$start, 0x$end, N_(\"$name\") },"
    done

    $ECHO "  { (gunichar)(-1), (gunichar)(-1), NULL }"
    $ECHO "};"
}


backup()
{
    if [ -f $1 ] ; then
        $ECHO "backing up existing $1 to $1.old"
        $MV -f $1 $1.old
    fi
}


make_download_dir()
{
    if [ -f $unidir ] ; then
        $ECHO "error: $unidir exists and is not a directory"
        exit 1
    fi
    
    if [ ! -d $unidir ] ; then
        $ECHO "creating directory $unidir"
        $MKDIR $unidir
    fi
}


do_unicode_data()
{
    make_download_dir

    f="$unidir/UnicodeData.txt"
    if [ ! -f $f ] ; then
        download "UnicodeData.txt"
    else
        $ECHO "already have $f, not downloading"
    fi

    out=$srcdir/unicode_data.cI
    backup $out

    $ECHO -n "generating $out..."
    write_unicode_data < $f > $out
    $ECHO " done"
}


do_categories()
{
    make_download_dir

    f="$unidir/UnicodeData.txt"
    if [ ! -f $f ] ; then
        download "UnicodeData.txt"
    else
        $ECHO "already have $f, not downloading"
    fi

    out=$srcdir/unicode_categories.cI
    backup $out

    $ECHO -n "generating $out..."
    $PERL $GENERATE_CATEGORIES < $f > $out
    $ECHO " done"
}


do_unihan()
{
    if [ "x$UNZIP" = "x" ] ; then
        $ECHO
        $ECHO "error: unzip not found, can't unzip Unihan.zip"
        $ECHO
        exit 1
    fi

    make_download_dir

    f="$unidir/Unihan.zip"
    if [ ! -f $f ] ; then
        download "Unihan.zip"
    else
        $ECHO "already have $f, not downloading"
    fi

    out=$srcdir/unicode_unihan.cI
    backup $out

    $ECHO -n "generating $out... this might take a minute..."
    $UNZIP -c $unidir/Unihan.zip | $GENERATE_UNIHAN > $out
    $ECHO " done"
}


do_blocks()
{
    make_download_dir

    f="$unidir/Blocks.txt"
    if [ ! -f $f ] ; then
        download "Blocks.txt"
    else
        $ECHO "already have $f, not downloading"
    fi

    out=$srcdir/unicode_blocks.cI
    backup $out

    $ECHO -n "generating $out..."
    write_blocks < $f > $out
    $ECHO " done"
}


do_nameslist()
{
    make_download_dir

    if [ "x$ICONV" = "x" ] ; then
        $ECHO
        $ECHO "error: iconv not found"
        $ECHO
        $ECHO "Unforunately, as of the writing of this script, NamesList.txt "
        $ECHO "is encoded in iso8859-1, which means it must be iconv'd to "
	$ECHO "UTF-8. Why unicode.org has done us this misdeed, I do not know."
        $ECHO
        exit 1
    fi

    f="$unidir/NamesList.txt"
    if [ ! -f $f ] ; then
        download "NamesList.txt"
    else
        $ECHO "already have $f, not downloading"
    fi

    out="$srcdir/unicode_nameslist.cI"
    backup $out

    $ECHO -n "generating $out..."
    $ICONV -f "ISO8859-1" -t "UTF-8" $f | $GENERATE_NAMESLIST > $out
    $ECHO " done"
}

# end of functions


# this is where the program starts 

case "x$1" in
    "xunicode_data.cI") do_unicode_data ;;
    "xunicode_unihan.cI") do_unihan ;;
    "xunicode_blocks.cI") do_blocks ;;
    "xunicode_nameslist.cI") do_nameslist ;;
    "xunicode_categories.cI") do_categories ;;
    *) 
        echo "usage: $0 FILE_TO_GENERATE"
        echo "       where FILE_TO_GENERATE is unicode_data.cI" 
        echo "                              or unicode_categories.cI"
        echo "                              or unicode_unihan.cI"
        echo "                              or unicode_blocks.cI"
        echo "                              or unicode_nameslist.cI"
        exit 1
esac


