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

/* reads Unihan.txt from stdin, prints unicode_unihan.cI on stdout */

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
        buf = xrealloc (buf, (*bsize) * sizeof (char));
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
process_nameslist_txt (FILE *fin)
{
  char *line, *temp;
  size_t bsize, len;
  int equal_i=0, ex_i=0, star_i=0, pound_i=0, colon_i=0;
  int equal0_i=-1, ex0_i=-1, star0_i=-1, pound0_i=-1, colon0_i=-1;
  FILE *equal_file, *ex_file, *star_file, *pound_file, *colon_file, *main_file;
  unsigned uc, ucv;
  char c;

  equal_file = tmpfile ();
  ex_file = tmpfile ();
  star_file = tmpfile ();
  pound_file = tmpfile ();
  colon_file = tmpfile ();
  main_file = tmpfile ();

  bsize = 32;
  line = xmalloc (bsize);

  for (len = getline (fin, &line, &bsize);  len > 0;
       len = getline (fin, &line, &bsize))
    {
      line[len-1] = '\0';

      if (line[0] == '\t')
        {
          switch (line[1])
            {
              case '=':
                if (equal0_i == -1)
                  equal0_i = equal_i;
                equal_i++;
                temp = quote (line + 3);
                fprintf (equal_file, "  { 0x%4.4X, %s },\n", uc, temp);
                free (temp);
                break;

              case '*':
                if (star0_i == -1)
                  star0_i = star_i;
                star_i++;
                temp = quote (line + 3);
                fprintf (star_file, "  { 0x%4.4X, %s },\n", uc, temp);
                free (temp);
                break;

              case '#':
                if (pound0_i == -1)
                  pound0_i = pound_i;
                pound_i++;
                temp = quote (line + 3);
                fprintf (pound_file, "  { 0x%4.4X, %s },\n", uc, temp);
                free (temp);
                break;

              case ':':
                if (colon0_i == -1)
                  colon0_i = colon_i;
                colon_i++;
                temp = quote (line + 3);
                fprintf (colon_file, "  { 0x%4.4X, %s },\n", uc, temp);
                free (temp);
                break;

              case 'x':
                if (ex0_i == -1)
                  ex0_i = ex_i;
                ex_i++;
                ucv = (unsigned) strtoul (strrchr (line, ' ') + 1, NULL, 16);
                fprintf (ex_file, "  { 0x%4.4X, 0x%4.4X },\n", uc, ucv);
                break;

              default:
                continue;
            }
        }
      else if (line[0] == '@')
        continue;
      else 
        {
          if (equal0_i != -1 || ex0_i != -1 || star0_i != -1 
              || pound0_i != -1 || colon0_i != -1)
            {
              fprintf (main_file, "  { 0x%4.4X, %d, %d, %d, %d, %d },\n",
                       uc, equal0_i, star0_i, ex0_i, pound0_i, colon0_i);
            }

          equal0_i = -1; 
          ex0_i    = -1; 
          star0_i  = -1; 
          pound0_i = -1; 
          colon0_i = -1;

          uc = (unsigned) strtoul (line, NULL, 16);
        }
    }

    printf ("/* unicode_nameslist.cI */\n");
    printf ("/* THIS IS A GENERATED FILE. */\n");
    printf ("/* http://www.unicode.org/Public/UNIDATA/NamesList.txt */\n");
    printf ("\n");
    printf ("const UnicharString names_list_equals[] =\n");
    printf ("{\n");
    rewind (equal_file);
    for (c = getc (equal_file);  !feof (equal_file);  c = getc (equal_file))
      putchar (c);
    fclose (equal_file);
    printf ("  { (gunichar)(-1), 0 }\n");
    printf ("};\n");
    printf ("\n");

    printf ("\n");
    printf ("const UnicharString names_list_stars[] =\n");
    printf ("{\n");
    rewind (star_file);
    for (c = getc (star_file);  !feof (star_file);  c = getc (star_file))
      putchar (c);
    fclose (star_file);
    printf ("  { (gunichar)(-1), 0 }\n");
    printf ("};\n");
    printf ("\n");

    printf ("\n");
    printf ("const UnicharString names_list_pounds[] =\n");
    printf ("{\n");
    rewind (pound_file);
    for (c = getc (pound_file);  !feof (pound_file);  c = getc (pound_file))
      putchar (c);
    fclose (pound_file);
    printf ("  { (gunichar)(-1), 0 }\n");
    printf ("};\n");
    printf ("\n");

    printf ("\n");
    printf ("const UnicharUnichar names_list_exes[] =\n");
    printf ("{\n");
    rewind (ex_file);
    for (c = getc (ex_file);  !feof (ex_file);  c = getc (ex_file))
      putchar (c);
    fclose (ex_file);
    printf ("  { (gunichar)(-1), 0 }\n");
    printf ("};\n");
    printf ("\n");

    printf ("\n");
    printf ("const UnicharString names_list_colons[] =\n");
    printf ("{\n");
    rewind (colon_file);
    for (c = getc (colon_file);  !feof (colon_file);  c = getc (colon_file))
      putchar (c);
    fclose (colon_file);
    printf ("  { (gunichar)(-1), 0 }\n");
    printf ("};\n");
    printf ("\n");

    printf ("\n");
    printf ("const NamesList names_list[] =\n");
    printf ("{\n");
    rewind (main_file);
    for (c = getc (main_file);  !feof (main_file);  c = getc (main_file))
      putchar (c);
    fclose (main_file);
    printf ("};\n");
    printf ("\n");
}


int
main ()
{
  process_nameslist_txt (stdin);

  exit (EXIT_SUCCESS);
}


