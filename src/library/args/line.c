/*****************************************************************************\
 *
 *		      Illinois Open Source License
 *                     University of Illinois/NCSA
 *                         Open Source License
 *
 * Copyright (c) 2004, The University of Illinois at Urbana-Champaign.
 * All rights reserved.
 *
 * Developed by:             
 *
 *		IMPACT Research Group
 *
 *		University of Illinois at Urbana-Champaign
 *
 *              http://www.crhc.uiuc.edu/IMPACT
 *              http://www.gelato.org
 *
 * Permission is hereby granted, free of charge, to any person
 * obtaining a copy of this software and associated documentation
 * files (the "Software"), to deal with the Software without
 * restriction, including without limitation the rights to use, copy,
 * modify, merge, publish, distribute, sublicense, and/or sell copies
 * of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * Redistributions of source code must retain the above copyright
 * notice, this list of conditions and the following disclaimers.
 *
 * Redistributions in binary form must reproduce the above copyright
 * notice, this list of conditions and the following disclaimers in
 * the documentation and/or other materials provided with the
 * distribution.
 *
 * Neither the names of the IMPACT Research Group, the University of
 * Illinois, nor the names of its contributors may be used to endorse
 * or promote products derived from this Software without specific
 * prior written permission.  THE SOFTWARE IS PROVIDED "AS IS",
 * WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT
 * LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A
 * PARTICULAR PURPOSE AND NONINFRINGEMENT.  IN NO EVENT SHALL THE
 * CONTRIBUTORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES
 * OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR
 * OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE
 * OR THE USE OR OTHER DEALINGS WITH THE SOFTWARE.
 *
\*****************************************************************************/
/*****************************************************************************\
 *      File:   line.c
 *      Author: Pohua Paul Chang
 *      Copyright (c) 1991 Pohua Paul Chang, Wen-Mei Hwu. All rights reserved.  
 *      All rights granted to the University of Illinois.
 *      The University of Illinois software License Agreement
 *      specifies the terms and conditions for redistribution.
\*****************************************************************************/

/* 10/29/02 REK Adding config.h */
#include <config.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <library/line.h>

static void
Punt (char *mesg)
{
  fprintf (stderr, "%s\n", mesg);
  exit (-1);
}

#define WHITE_SPACE(ch) ((ch==' ')||(ch=='\t')||(ch==''))

/*
 *      Read one line from file.
 *      Skip empty lines.
 *      Compress all white spaces between words into single spaces.
 *      Terminate the line with \0
 *      Punt if the buffer is not large enough.
 *      Else, return 0 upon EOF.
 *      Otherwise, return 1;
 */
int
ReadLine (FILE * F, char buf[], int len)
{
  int white_space;
  int ch, i;
  i = 0;
  white_space = 0;
  while ((ch = getc (F)) != EOF)
    {
      if ((i == 0) && (ch == '\n'))     /* skip over empty line */
        continue;
      if (ch == '\n')           /* exit loop upon EOLN */
        break;
      if (WHITE_SPACE (ch))
        {                       /* do nothing for white spaces */
          white_space = 1;
        }
      else
        {
          if ((i != 0) && white_space)
            {
              buf[i++] = ' ';
              if (i >= len)
                Punt ("ReadLine: buffer overflows");
            }
          buf[i++] = ch;
          if (i >= len)
            Punt ("ReadLine: buffer overflows");
          white_space = 0;
        }
    }
  buf[i] = '\0';
  if (i == 0)
    return 0;                   /* EOF */
  return 1;
}
/*
 *      Read one word from file.
 *      Skip over white spaces.
 *      Punt if the buffer is not large enough.
 *      Else, return 0 upon EOF.
 *      Otherwise, return 1.
 */
int
ReadWord (FILE * F, char buf[], int len)
{
  int ch, i;
  /*
   *  Skip over white spaces first.
   */
  while ((ch = getc (F)) != EOF)
    {                           /* stop when EOF has been reached */
      if (ch == '\n')           /* skip over EOLN */
        continue;
      if (!WHITE_SPACE (ch))
        {                       /* read until a non-white-space */
          ungetc (ch, F);
          break;
        }
    }
  if (ch == EOF)
    return 0;
  i = 0;
  while ((ch = getc (F)) != EOF)
    {
      if ((ch == '\n') || WHITE_SPACE (ch))
        break;
      buf[i++] = ch;
      if (i >= len)
        Punt ("ReadWord: buffer overflows");
    }
  buf[i] = '\0';
  if (i == 0)
    return 0;                   /* EOF */
  return 1;
}
/*
 *      Find the starting point of each word in the line.
 *      Returns -1 if the buffer is not large enough.
 *      Otherwise, returns the number of words found.
 */
int
ParseLine (char *line, char *buf[], int len)
{
  int i, n;
  i = n = 0;
  while (line[i] != '\0')
    {
      /*
       *      Skip over white-spaces.
       */
      while (WHITE_SPACE (line[i]))
        i++;
      if (line[i] == '\0')
        break;
      buf[n++] = (line + i);
      if (n >= len)
        Punt ("ParseLine: buffer overflows");
      /*
       *      Go to the next white space.
       */
      while ((line[i] != '\0') && !WHITE_SPACE (line[i]))
        i++;
    }
  buf[n] = 0;
  return n;
}
/*
 *      Find the starting point of each word in the line.
 *      Returns -1 if the buffer is not large enough.
 *      Otherwise, returns the number of words found.
 *      Make each word a separate string.
 */
int
ParseLine2 (char *line, char *buf[], int len)
{
  int i, n;
  i = n = 0;
  while (line[i] != '\0')
    {
      /*
       *      Skip over white-spaces.
       */
      while (WHITE_SPACE (line[i]))
        i++;
      if (line[i] == '\0')
        break;
      buf[n++] = (line + i);
      if (n >= len)
        Punt ("ParseLine: buffer overflows");
      /*
       *      Go to the next white space.
       */
      while ((line[i] != '\0') && !WHITE_SPACE (line[i]))
        i++;
      if (line[i] == '\0')
        break;
      line[i] = '\0';
      i++;
    }
  buf[n] = 0;
  return n;
}
/*
 *      Return 1 if the first part of the word
 *      totally match the prefix.
 */
int
MatchPrefix (char *line, char *prefix)
{
  int len, i;
  len = strlen (prefix);
  if (strlen (line) < len)
    return 0;
  for (i = 0; i < len; i++)
    if (line[i] != prefix[i])
      return 0;
  return 1;
}
/* 
 *      Return 1 if pattern matches name; 
 *      otherwise return 0. 
 *      Wildcard character is allowed in pattern. 
 *      ? matches any one character.
 *      * matches from 1 to N characters. 
 */
int
WildcardMatch (char *pattern, char *name)
{
  if ((name[0] == '\0') && (pattern[0] == '\0'))
    return 1;
  if (pattern[0] == '?')
    {
      return WildcardMatch (pattern + 1, name + 1);
    }
  else if (pattern[0] == '*')
    {
      int i;
      i = 1;
      if ((name[1] == '\0') && (pattern[1] == '\0'))
        return 1;
      while (name[i] != '\0')
        {
          /*
           * find the next possible starting point in name 
           * that matches the rest of the pattern.
           */
          while ((name[i] != '\0') && (name[i] != pattern[1])
                 && (pattern[1] != '*') && (pattern[1] != '?'))
            i += 1;
          if (WildcardMatch (pattern + 1, name + i))
            return 1;
          i += 1;
        }
      return 0;
    }
  else if (pattern[0] == name[0])
    {
      return WildcardMatch (pattern + 1, name + 1);
    }
  else
    return 0;
}
/*
 *      Remove the last field from the source string.
 *      Return 1 if successful, Return 0 if not successful
 *      for any reason.
 *      If the seperator character does not appear in the
 *      source string, then the result is an empty string,
 *      and 1 is returned.
 */
int
RemovePostfix (char src[], char seperator, char buf[], int buf_len)
{
  register int src_len, i, j;
  src_len = strlen (src);
  buf[0] = 0;                   /* default result is an empty string */
  if (src_len == 0)
    return 1;                   /* nothing needs to be done */
  for (i = src_len - 1; i >= 0; i--)    /* find the seperator */
    if (src[i] == seperator)
      break;
  if (i < 0)
    return 1;                   /* the entire src is deleted */
  if (buf_len <= i)
    return 0;                   /* buffer is not long enough */
  for (j = 0; j < i; j++)
    buf[j] = src[j];
  buf[j] = '\0';
  return 1;
}
/*
 *      Remove the first field from the source string.
 *      Return 1 if successful, Return 0 if not successful
 *      for any reason.
 *      If the seperator character does not appear in the
 *      source string, then the result is an empty string,
 *      and 1 is returned.
 */
int
RemovePrefix (char src[], char seperator, char buf[], int buf_len)
{
  register int src_len, i, j, k;
  buf[0] = '\0';
  src_len = strlen (src);
  for (i = 0; i < src_len; i++) /* find the first seperator */
    if (src[i] == seperator)
      break;
  j = src_len - (i + 1);        /* number of char needs to be copied */
  if (j >= buf_len)
    return 0;
  for (k = 0; k <= j; k++)
    buf[k] = src[i + k + 1];
  buf[k] = '\0';
  return 1;
}
/*
 *      Extract the first field from the source string.
 *      Return 1 if successful, Return 0 if not successful
 *      for any reason.
 *      If the seperator character does not appear in the
 *      source string, then the result is the src string.
 */
int
GetPrefix (char src[], char seperator, char buf[], int buf_len)
{
  register char *ch;
  register int i;
  i = 0;
  for (ch = src; (*ch != seperator) && (*ch != '\0'); ch++)
    {
      if (i >= buf_len)
        {
          buf[buf_len - 1] = '\0';
          return 0;
        }
      buf[i] = *ch;
      i += 1;
    }
  buf[i] = '\0';
  return 1;
}


#ifdef DEBUG_LINE
int
main (void)
{
  char line[512];
  char *ptr[100];
  int i, j;

  RemovePostfix ("f:9:ccode", ':', line, 512);
  printf ("%s\n", line);

  RemovePostfix ("abc:def", ':', line, 512);
  printf ("%s\n", line);
  RemovePostfix (":abc:", ':', line, 512);
  printf ("%s\n", line);
  RemovePostfix ("abc:def:ghi", ':', line, 512);
  printf ("%s\n", line);
  RemovePostfix ("abc.def.ghi", ':', line, 512);
  printf ("%s\n", line);

  RemovePrefix ("abc:def", ':', line, 512);
  printf ("%s\n", line);
  RemovePrefix (":abc:", ':', line, 512);
  printf ("%s\n", line);
  RemovePrefix ("abc:def:ghi", ':', line, 512);
  printf ("%s\n", line);
  RemovePrefix ("abc.def.ghi", ':', line, 512);
  printf ("%s\n", line);

/****
#define M(x, y) WildcardMatch(x, y)
        printf("%d\n", M("abc", "abc"));
        printf("%d\n", M("abcd", "abc"));
        printf("%d\n", M("abc", "abcd"));
        printf("%d\n", M("ab?d", "abcd"));
        printf("%d\n", M("a*d", "abcd"));
        printf("%d\n", M("a*d", "abce"));
        printf("%d\n", M("a*de", "abcdfdde"));
        printf("%d\n", M("ab?", "abc"));
        printf("%d\n", M("ab*", "abc"));
        printf("%d\n", M("a?*", "abc"));
        printf("%d\n", M("a**", "abc"));
        printf("%d\n", M("*", "abc"));
        printf("%d\n", M("a*d", "ad"));
        printf("%d\n", M("???", "abc"));
        printf("%d\n", M("***", "abc"));
        printf("%d\n", M("*?*", "abc"));
        printf("%d\n", M("**?", "abc"));
****/
/***
        while (ReadWord(stdin, line, 512) != 0) {
            printf("> %s\n", line);
        }
 ***/
/****
        while (ReadLine(stdin, line, 512) != 0) {
            printf("> %s\n", line);
            i = ParseLine(line, ptr, 100);
            for (j=0; j<i; j++) {
                printf("# %s\n", ptr[j]);
            }
        }
****/

  return 0;
}
#endif
