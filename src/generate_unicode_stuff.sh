#!/bin/sh
#
# $Id$
# 

#
# Copyright (c) 2002  Noah Levitt <nlevitt аt users.sourceforge.net>
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
# unicode_data.cI and unihan.cI
#

UNZIP=`which unzip`
WGET=`which wget`
SED=`which sed`
MV=`which mv`
ECHO=`which echo`
MKDIR=`which mkdir`
AWK=`which awk`

unidir="$PWD/unicode.org"
srcdir="$PWD"


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


function stringify()
{
    if [ "x$1" = "x" ] ; then
        echo 0
    else
        echo "\"$1\""
    fi
}

# reads from stdin, writes to stdout
function write_unihan()
{
    $ECHO "/* unicode_unihan.cI */" 
    $ECHO "/* THIS IS A GENERATED FILE. */"
    $ECHO "/* http://www.unicode.org/Public/UNIDATA/Unihan.zip */"
    $ECHO ""
    $ECHO ""
    $ECHO "#if HAVE_CONFIG_H"
    $ECHO "# include <config.h>"
    $ECHO "#endif"
    $ECHO "#include <unicode_info.h>"
    $ECHO ""
    $ECHO ""
    $ECHO "#if ENABLE_UNIHAN"
    $ECHO ""
    $ECHO "const Unihan unihan[] ="
    $ECHO "{"

    unset kDefinition kCantonese kMandarin kTang 
    unset kKorean kJapaneseKun kJapaneseOn
    
    curr_index="U+3400"
    
    while read index property value
    do
        case $index in
            "#") continue ;;
            "#*") continue ;;
        esac
    
        if [ "x$index" != "x$curr_index" ] 
        then
            hex=`$ECHO $curr_index | $SED 's/^U+/0x/'`
            curr_index=$index
    
            if [ "x$kDefinition" = "x" ] && [ "x$kCantonese" = "x" ] && [ "x$kMandarin" = "x" ] && [ "x$kTang" = "x" ] && [ "x$kKorean" = "x" ] && [ "x$kJapaneseKun" = "x" ] && [ "x$kJapaneseOn" = "x" ] ;
            then
                continue;
            fi

            QQkDefinition=`stringify "$kDefinition"`
            QQkCantonese=`stringify "$kCantonese"`
            QQkMandarin=`stringify "$kMandarin"`
            QQkTang=`stringify "$kTang"`
            QQkKorean=`stringify "$kKorean"`
            QQkJapaneseKun=`stringify "$kJapaneseKun"`
            QQkJapaneseOn=`stringify "$kJapaneseOn"`

            $ECHO "  { $hex, $QQkDefinition, $QQkCantonese, $QQkMandarin, $QQkTang, $QQkKorean, $QQkJapaneseKun, $QQkJapaneseOn },"
    
            unset kDefinition kCantonese kMandarin kTang
            unset kKorean kJapaneseKun kJapaneseOn
        fi
    
        case $property in
            "kDefinition")  kDefinition=`$ECHO $value | $SED 's/\"/\\\"/g'` ;;
            "kCantonese")   kCantonese=`$ECHO $value | $SED 's/\"/\\\"/g'` ;;
            "kMandarin")    kMandarin=`$ECHO $value | $SED 's/\"/\\\"/g'` ;;
            "kTang")        kTang=`$ECHO $value | $SED 's/\"/\\\"/g'` ;;
            "kKorean")      kKorean=`$ECHO $value | $SED 's/\"/\\\"/g'` ;;
            "kJapaneseKun") kJapaneseKun=`$ECHO $value | $SED 's/\"/\\\"/g'` ;;
            "kJapaneseOn")  kJapaneseOn=`$ECHO $value | $SED 's/\"/\\\"/g'` ;;
        esac
    done

    $ECHO "};"
    $ECHO ""
    $ECHO ""
    $ECHO "#endif /* #if ENABLE_UNIHAN */"
    $ECHO ""
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

    $ECHO -n "writing $out..."
    write_unicode_data < $unidir/UnicodeData.txt > $out
    $ECHO "done"
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

    $ECHO -n "writing $out (this will take a long time)..."
    $UNZIP -c $unidir/Unihan.zip | write_unihan > $out
    $ECHO "done"
}

# end of functions


# this is where the program starts 

case "x$1" in
    "xunicode_data.cI") do_unicode_data ;;
    "xunicode_unihan.cI") do_unihan ;;
    *) 
        echo "usage: $0 FILE_TO_GENERATE"
        echo "       where FILE_TO_GENERATE is unicode_data.cI or unicode_unihan.cI"
        exit 1
esac


