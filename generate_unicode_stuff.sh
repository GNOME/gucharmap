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
# src/unicode_data.c and src/unihan.c
#

UNZIP=`which unzip`
WGET=`which wget`
unidir="$PWD/unicode.org"
srcdir="$PWD/src"


function die()
{
    echo "error: $1"
    exit 1
}

function download()
{

    if [ "x$WGET" = "x" ] ; then
        echo
        echo "error: wget not found, can't download files"
        echo
        echo "You can download these files yourself and put them in $unidir:"
        echo
        echo "http://www.unicode.org/Public/UNIDATA/UnicodeData.txt"
        echo "http://www.unicode.org/Public/UNIDATA/Unihan.zip"
        echo
    fi

    $WGET --directory-prefix=$unidir "http://www.unicode.org/Public/UNIDATA/$1"
}



if [ -e $unidir ] && [ ! -d $unidir ] ; then
    echo "error: $unidir exists and is not a directory"
    exit 1
fi

if [ ! -e $unidir ] ; then
    echo "mkdir $unidir"
    mkdir $unidir
fi

for f in UnicodeData.txt Unihan.zip ; do
    if [ ! -e $unidir/$f ] ; then
        download $f
    else
        echo "already have $unidir/$f, not downloading"
    fi
done


if [ -e $srcdir/unicode_data.c ] ; then
    /bin/cp $srcdir/unicode_data.c $srcdir/unicode_data.old
fi

/bin/cat > $srcdir/unicode_data.c <<EOF
/* unicode_data.c */
/* THIS IS A GENERATED FILE. */

#include <gtk/gtk.h>
#include <string.h>
#include "unicode_info.h"

typedef struct 
{
  gunichar index;
  gchar *name;
} 
unicode_data_t;


static unicode_data_t unicode_data[] =
{
EOF

/usr/bin/awk -F';' '{print "  { 0x" $1 ", \"" $2 "\" },"}' \
   <  "$unidir/UnicodeData.txt" \
   >> "$srcdir/unicode_data.c"

/bin/cat >> $srcdir/unicode_data.c <<EOF
};


/* does a binary search on unicode_data */
gchar *
get_unicode_data_name (gunichar uc)
{
  gint min = 0;
  gint mid;
  gint max = sizeof (unicode_data) / sizeof (unicode_data_t) - 1;

  if (uc < unicode_data[0].index || uc > unicode_data[max].index)
    return "";

  while (max >= min) 
    {
      mid = (min + max) / 2;
      if (uc > unicode_data[mid].index)
        min = mid + 1;
      else if (uc < unicode_data[mid].index)
        max = mid - 1;
      else
        return unicode_data[mid].name;
    }

  return NULL;
}


/* ascii case-insensitive substring search (source ripped from glib) */
static gchar *
ascii_case_strrstr (const gchar *haystack, const gchar *needle)
{
  gsize i;
  gsize needle_len;
  gsize haystack_len;
  const gchar *p;
      
  g_return_val_if_fail (haystack != NULL, NULL);
  g_return_val_if_fail (needle != NULL, NULL);

  needle_len = strlen (needle);
  haystack_len = strlen (haystack);

  if (needle_len == 0)
    return (gchar *)haystack;

  if (haystack_len < needle_len)
    return NULL;
  
  p = haystack + haystack_len - needle_len;

  while (p >= haystack)
    {
      for (i = 0; i < needle_len; i++)
        if (g_ascii_tolower (p[i]) != g_ascii_tolower (needle[i]))
          goto next;
      
      return (gchar *)p;
      
    next:
      p--;
    }
  
  return NULL;
}


/* case insensitive; returns (gunichar)(-1) if nothing found */
gunichar
find_next_substring_match (gunichar start, gunichar unichar_max,
                           const gchar *search_text)
{
  gint min = 0;
  gint mid = 0;
  gint max = sizeof (unicode_data) / sizeof (unicode_data_t) - 1;
  gint i0;
  gint i;

  /* locate the start character by binary search */
  if (start < unicode_data[0].index || start > unichar_max)
    i0 = 0;
  else
    {
      while (max >= min) 
        {
          mid = (min + max) / 2;
          if (start > unicode_data[mid].index)
            min = mid + 1;
          else if (start < unicode_data[mid].index)
            max = mid - 1;
          else
            break;
        }

      i0 = mid;
    }

  /* try substring match on each */
  max = sizeof (unicode_data) / sizeof (unicode_data_t);
  for (i = i0+1;  i != i0;  )
    {
      if (ascii_case_strrstr (unicode_data[i].name, search_text) != NULL)
        return unicode_data[i].index;

      i++;
      if (i >= max || unicode_data[i].index > unichar_max)
        i = 0;
    }

  /* if the start character matches we want to return a match */
  if (ascii_case_strrstr (unicode_data[i].name, search_text) != NULL)
    return unicode_data[i].index;

  return (gunichar)(-1);
}

EOF
