/* $Id$ */
/*
 * Copyright (c) 2003  Noah Levitt <nlevitt аt users.sourceforge.net>
 *
 * This program is free software; you can redistribute it and/or modify it
 * under the terms of the GNU General Public License as published by the
 * Free Software Foundation; either version 2 of the License, or (at your
 * option) any later version.
 *
 * This program is distributed in the hope that it will be useful, but
 * WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License along
 * with this program; if not, write to the Free Software Foundation, Inc.,
 * 59 Temple Place, Suite 330, Boston, MA 02111-1307  USA
 */

/* reads Unihan.txt from stdin, prints unicode_unihan.c on stdout */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>


void *
xmalloc (size_t size)
{
  register void *p;

  p = malloc(size);

  if (p)
    return p;

  fprintf (stderr, "malloc(%u) failed\n", (unsigned) size);
  exit (EXIT_FAILURE);
}


void *
xrealloc (void *ptr, size_t size)
{
  register void *p;

  p = realloc (ptr, size);

  if (p)
    return p;

  fprintf (stderr, "realloc (0x%p, %u) failed\n", ptr, (unsigned) size);
  exit (EXIT_FAILURE);
}


/* gets characters up to and including newline */
size_t
getline (FILE *fin, char **bufptr, size_t *bsize)
{
  size_t i;
  char c, *buf;

  if (feof (fin) || bsize == 0 || *bsize == 0
      || bufptr == 0 || *bufptr == 0)
    return (size_t) 0;

  buf = *bufptr;
  i = 0;

  for (c = getc (fin);  !feof (fin);  c = getc (fin))
  {
    buf[i] = c;
    i++;

    if (i >= *bsize - 1) 
      {
        *bsize *= 2;
        buf = xrealloc(buf, (*bsize) * sizeof(char));
        *bufptr = buf;
      }

    if (c == '\n')
      break;
  }

  buf[i] = '\0';
  return i;
}


/* returns newly allocated string which should be freed by caller */
char *
quote (char *str)
{
  char *buf;
  int i, j;

  /* malloc the absolute max space */
  buf = xmalloc (2 * strlen (str) + 8); 

  buf[0] = '"';
  for (i=0, j=1;  str[i];  i++)
    {
      if (str[i] == '"')
        {
          buf[j] = '\\';
          buf[j+1] = '"';
          j += 2;
        }
      else
        {
          buf[j] = str[i];
          j++;
        }
    }

  buf[j] = '"';
  buf[j+1] = '\0';

  return buf;
}


void
process_unihan_txt (FILE *fin)
{
  char *buf, *bp;
  size_t bsize, len;
  unsigned long uc = 0, luc;
  char *kDefinition=0, *kCantonese=0, *kMandarin=0, *kTang=0,
       *kKorean=0, *kJapaneseKun=0, *kJapaneseOn=0;

  bsize = 32;
  buf = xmalloc (bsize);

  for (len = getline (fin, &buf, &bsize);  len > 0;
       len = getline (fin, &buf, &bsize))
    {
      if (buf[0] == '#')
        continue; 

      luc = strtoul (buf + 2, NULL, 16);
      if (uc != luc)
        {
          unsigned long remember = uc;

          uc = luc;

          if (kDefinition == 0 && kCantonese == 0 && kMandarin == 0 
              && kTang == 0 && kKorean == 0 && kJapaneseKun == 0 
              && kJapaneseOn==0)
            continue;

          printf ("  { 0x%4.4lX, %s, %s, %s, %s, %s, %s, %s },\n",
                  remember, 
                  kDefinition ? kDefinition : "0",
                  kCantonese ? kCantonese : "0", 
                  kMandarin ? kMandarin : "0",
                  kTang ? kTang : "0", 
                  kKorean ? kKorean : "0",
                  kJapaneseKun ? kJapaneseKun : "0", 
                  kJapaneseOn ? kJapaneseOn : "0");

          if (kDefinition) { free (kDefinition); kDefinition = NULL; }
          if (kCantonese) { free (kCantonese); kCantonese = NULL; }
          if (kMandarin) { free (kMandarin); kMandarin = NULL; }
          if (kTang) { free (kTang); kTang = NULL; }
          if (kKorean) { free (kKorean); kKorean = NULL; }
          if (kJapaneseKun) { free (kJapaneseKun); kJapaneseKun = NULL; }
          if (kJapaneseOn) { free (kJapaneseOn); kJapaneseOn = NULL; }
        }

      buf[len-1] = '\0'; /* we don't want the newline */

      /* find the beginning of the k******* thing */
      bp = strchr (buf + 6, '\t');
      bp++;

      if (strstr (bp, "kDefinition") == bp)
        kDefinition = quote (strchr (bp + 5, '\t') + 1);
      if (strstr (bp, "kCantonese") == bp)
        kCantonese = quote (strchr (bp + 5, '\t') + 1);
      if (strstr (bp, "kMandarin") == bp)
        kMandarin = quote (strchr (bp + 5, '\t') + 1);
      if (strstr (bp, "kTang") == bp)
        kTang = quote (strchr (bp + 5, '\t') + 1);
      if (strstr (bp, "kKorean") == bp)
        kKorean = quote (strchr (bp + 5, '\t') + 1);
      if (strstr (bp, "kJapaneseKun") == bp)
        kJapaneseKun = quote (strchr (bp + 5, '\t') + 1);
      if (strstr (bp, "kJapaneseOn") == bp)
        kJapaneseOn = quote (strchr (bp + 5, '\t') + 1);
    }
}


int
main ()
{
  printf ("/* unicode_unihan.cI */\n");
  printf ("/* THIS IS A GENERATED FILE. */\n");
  printf ("/* http://www.unicode.org/Public/UNIDATA/Unihan.zip */\n");
  printf ("\n");
  printf ("\n");
  printf ("#if HAVE_CONFIG_H\n");
  printf ("# include <config.h>\n");
  printf ("#endif\n");
  printf ("#include <unicode_info.h>\n");
  printf ("\n");
  printf ("\n");
  printf ("#if ENABLE_UNIHAN\n");
  printf ("\n");
  printf ("const Unihan unihan[] =\n");
  printf ("{\n");

  process_unihan_txt (stdin);

  printf ("};\n");
  printf ("\n");
  printf ("\n");
  printf ("#endif /* #if ENABLE_UNIHAN */\n");
  printf ("\n");

  exit (EXIT_SUCCESS);
}


