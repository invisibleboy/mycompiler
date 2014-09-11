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
/*****************************************************************************
 *	File:	 util.c
 *      Authors: Robert Kidd and Wen-mei Hwu
 *	Modified from code written by:	Po-hua Chang, David August,
 *               Nancy Warter, Grant Haab, Krishna Subramanian
 * 	Copyright (c) 2003 Robert Kidd, David August, Nancy Warter,
 *               Grant Haab, Krishna Subramanian, Po-hua Chang, Wen-mei Hwu
 *		 and The Board of Trustees of the University of Illinois.
 *		 All rights reserved.
 *      License Agreement specifies the terms and conditions for 
 *      redistribution.
 *****************************************************************************/

#include <config.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <malloc.h>
#include "impact_global.h"
#include "util.h"
#include "perror.h"

/* String manipulation functions. */
/*
 * Remove " " from "name", and returns the number of characters in " ". 
 * -1 is returned if str is not	of the form "name".
 */
int
P_RemoveDQ (char *str, char buf[], int N)
{
  int len = strlen (str);
  int i;

  if (len < 2)
    return -1;
  if (str[0] != '"')
    return -1;
  if (str[len - 1] != '"')
    return -1;
  if (N < len - 1)
    P_punt ("struct.c:RemoveDQ:%d buffer is not large enough", __LINE__);
  for (i = 1; i < (len - 1); i++)
    {
      buf[i - 1] = str[i];
    }
  buf[len - 2] = '\0';
  return (len - 1);
}

/* 
 * Produce a copy of the given string without the double quotes. 
 */
char *
P_DQString2String (char *str)
{
  char *new;
  int len;

  if (str == NULL)
    P_punt ("struct.c:DQString2String:%d null string pointer passed",
	    __LINE__ - 1);
  len = strlen (str) - 1;	/* minus two quotes plus '\0' */

  if ((new = (char *) malloc (sizeof (char) * len)) == NULL)
    P_punt ("struct.c:DQString2String:%d out of memory", __LINE__);

  if (P_RemoveDQ (str, new, len) < 0)
    P_punt ("struct.c:DQString2String:%d not double-quoted string", __LINE__);
  return new;
}

/*
 * Produce a copy of the given string, but change all characters which are
 * illegal in an identifier to the character "$".  This allows filenames
 * and the like to be converted to a string that can be used as an identifier.
 * An identifier is defined as follows:  [_,$,A-Z,a-z][_,$,A-Z,a-z,0-9]*
 */

char *
P_String2Ident (char *str)
{
  char *ident;
  int i, len;

  if (str == NULL)
    P_punt ("struct.c:String2Ident:%d null string pointer passed", __LINE__);
  len = strlen (str) + 1;
  if ((ident = (char *) malloc (sizeof (char) * len)) == NULL)
    P_punt ("struct.c:String2Ident:%d out of memory", __LINE__);

  if (str[0] == '\0')
    {
      ident[0] = '\0';
      return ident;
    }

  ident[0] = (!isalpha (str[0]) && (str[0] != '_') && (str[0] != '$')) ?
    '$' : str[0];

  for (i = 1; i < len - 1; i++)
    {
      ident[i] = (!isalnum (str[i]) && (str[i] != '_') && (str[i] != '$')) ?
	'$' : str[i];
    }
  ident[len - 1] = '\0';
  return ident;
}

/*! \brief Adds double quotes to the end of a string.
 *
 * \param str
 *  the string
 *
 * \return
 *  A pointer to a new string that is equivalent to str with double quotes
 *  at both ends.
 *
 * It is the caller's responsibility to free the string returned by this
 * function.
 */
char *
P_AddDQ (char *str)
{
  char *result = NULL;
  int len = strlen (str) + 3;

  if ((result = malloc (len)))
    {
      snprintf (result, len, "\"%s\"", str);
    }
  else
    {
      P_punt ("util.c:P_AddDQ:%d malloc failed", __LINE__);
    }

  return (result);
}

/*! \brief Matches a filename, ignoring the directory if necessary.
 *
 * \param name
 *  the name (or path) being tested.
 * \param wanted
 *  the filename to match.
 *
 * \return
 *  If the first n characters of \a name match \a wanted, returns TRUE.
 *  Otherwise, returns FALSE.
 *
 * This function matches \a wanted against \a name.  It compares the
 * first strlen (wanted) characters of \a wanted against name.  If they
 * match, this function returns TRUE.  If they don't, this function tries
 * to match, ignoring any directory specified in \a name (everything
 * before the final '/' character).  If neither case matches, returns
 * FALSE.
 *
 * P_NameCheck ("platform/__impact_lib.pc", "__impact_lib") returns TRUE
 * P_NameCheck ("platform/__impact_lib.c", "__impact_lib.c") returns TRUE
 * P_NameCheck ("__impact_lib.c", "__impact_lib.c") returns TRUE
 * P_NameCheck ("__impact_lib.c", "foo.c") returns FALSE
 */
bool
P_NameCheck (char *name, char *wanted)
{
  char *last_slash;
  int wanted_len = strlen (wanted);
  bool result = FALSE;

  if (strncmp (name, wanted, wanted_len) == 0 || \
      ((last_slash = strrchr (name, '/')) && \
       strncmp (last_slash + 1, wanted, wanted_len) == 0))
    result = TRUE;

  return (result);
}

/*! \brief Inverts a relative path.
 *
 * \param path
 *  the relative path to invert.
 * \param last_is_dir
 *  if the last component of \a path is a directory, pass TRUE.  If the last
 *  component of \a path is a file, pass FALSE.
 *
 * \return
 *  The inverse of \a path.  It is the caller's responsibility to free this
 *  string.
 *
 * Path: foo/bar/dir/file (last_is_dir = FALSE)
 * Inverse: ../../..
 * Path: foo/bar/dir/dir1 (last_is_dir = TRUE)
 * Inverse: ../../../..
 *
 * \note If \a path is an absolute path, this function returns NULL.
 */
char *
P_InvertPath (char *path, bool last_is_dir)
{
  char *result = NULL, *prev_slash = NULL, *cur_slash = path;
  int slash_count = 0;

  if (path[0] != '\0' && path[0] != '/')
    {
      while (prev_slash = cur_slash + 1,
	     (cur_slash = strchr (cur_slash + 1, '/')) != NULL)
	slash_count++;

      if (last_is_dir && prev_slash[0] != '\0')
	slash_count++;

      /* Print ../ for each slash_count, with a null instead of the final
       * '/'. */
      result = malloc (slash_count * 3);
      result[0] = '\0';
      while (slash_count > 1)
	{
	  strcat (result, "../");
	  slash_count--;
	}
      strcat (result, "..");
    }

  return (result);
}
