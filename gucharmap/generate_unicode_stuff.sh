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
#

UNZIP=`which unzip`
WGET=`which wget`
SED=`which sed`
MV=`which mv`
ECHO=`which echo`
MKDIR=`which mkdir`
AWK=`which awk`
GENERATE_UNIHAN="./generate_unihan"

unidir="$PWD/unicode.org"
srcdir="$PWD"

PATH=""


function download()
{
    if [ "x$WGET" = "x" ] ; then
        $ECHO
        $ECHO "error: wget not found, can't download files"
        $ECHO
        $ECHO "You can download these files yourself and put them in $unidir:"
        $ECHO
        $ECHO "http://www.unicode.org/Public/UNIDATA/UnicodeData.txt"
        $ECHO "http://www.unicode.org/Public/UNIDATA/Unihan.zip"
        $ECHO
        exit 1
    fi

    $WGET --directory-prefix=$unidir \
          "http://www.unicode.org/Public/UNIDATA/$1" || exit 1
}


# reads from stdin, writes to stdout
function write_unicode_data()
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


function write_blocks()
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

        bounds=`$ECHO $line | $AWK -F'; ' '{ print $1 }'`
        name=`$ECHO $line | $AWK -F'; ' '{ print $2 }'`
        start=`$ECHO $bounds | $AWK -F'\.\.' '{ print $1 }'`
        end=`$ECHO $bounds | $AWK -F'\.\.' '{ print $2 }'`

        $ECHO "  { 0x$start, 0x$end, \"$name\" },"
    done

    $ECHO "{ (gunichar)-1, (gunichar)-1, NULL }"
    $ECHO "};"
}


function backup()
{
    if [ -e $1 ] ; then
        $ECHO "backing up existing $1 to $1.old"
        $MV -f $1 $1.old
    fi
}


function make_download_dir()
{
    if [ -e $unidir ] && [ ! -d $unidir ] ; then
        $ECHO "error: $unidir exists and is not a directory"
        exit 1
    fi
    
    if [ ! -e $unidir ] ; then
        $ECHO "creating directory $unidir"
        $MKDIR $unidir
    fi
}


function do_unicode_data()
{
    make_download_dir

    f="$unidir/UnicodeData.txt"
    if [ ! -e $f ] ; then
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


function do_unihan()
{
    if [ "x$UNZIP" = "x" ] ; then
        $ECHO
        $ECHO "error: unzip not found, can't unzip Unihan.zip"
        $ECHO
        exit 1
    fi

    make_download_dir

    f="$unidir/Unihan.zip"
    if [ ! -e $f ] ; then
        download "Unihan.zip"
    else
        $ECHO "already have $f, not downloading"
    fi

    out=$srcdir/unicode_unihan.cI
    backup $out

    $ECHO -n "generating $out... this may take a minute..."
    $UNZIP -c $unidir/Unihan.zip | $GENERATE_UNIHAN > $out
    $ECHO " done"
}


function do_blocks()
{
    make_download_dir

    f="$unidir/Blocks.txt"
    if [ ! -e $f ] ; then
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

# end of functions


# this is where the program starts 

case "x$1" in
    "xunicode_data.cI") do_unicode_data ;;
    "xunicode_unihan.cI") do_unihan ;;
    "xunicode_blocks.cI") do_blocks ;;
    *) 
        echo "usage: $0 FILE_TO_GENERATE"
        echo "       where FILE_TO_GENERATE is unicode_data.cI" 
        echo "                              or unicode_unihan.cI"
        echo "                              or unicode_blocks.cI"
        exit 1
esac


