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
/*! \file
 * \brief Input/output utility routines
 *
 * \author Robert Kidd, Wen-mei Hwu
 *
 * This file contains utility routines used to read and write Pcode.
 */
/*****************************************************************************/

#include <config.h>
#include <stdio.h>
#include <unistd.h>
#include <fcntl.h>
#include <errno.h>
#include <string.h>
#include "io_util.h"
#include "read.h"
#include "perror.h"

/*! \brief Opens a file for reading and obtains a shared lock.
 *
 * \param name
 *  the file to open.
 *
 * \return
 *  A FILE * for the opened file.  If the file cannot be opened,
 *  returns NULL.
 *
 * \note If \a name is the string "stdin", this function returns
 * a file handle for standard input.
 */
FILE *
P_file_open_r (char *name)
{
  FILE *result;
  struct flock fl = {0};

  if (strcmp (name, "stdin") == 0)
    {
      result = stdin;
    }
  else
    {
      if ((result = fopen (name, "r")))
	{
	  fl.l_type = F_RDLCK;

	  /* Lock the file */
	  if (fcntl (fileno (result), F_SETLK, &fl) == -1)
	    {
	      fclose (result);
	      perror ("Error locking file");
	      P_punt ("io_util.c:P_file_open_r:%d Could not obtain shared "
		      "lock on\nfile %s", __LINE__ - 1, name);
	    }
	}
    }

  return (result);
}

/*! \brief Opens a file for writing and obtains an exclusive lock.
 *
 * \param name
 *  the file to open.
 *
 * \return
 *  A FILE * for the opened file.
 *
 * \note If \a name is the string "stdout" or "stderr", this function returns a
 * filehandle for standard output or standard error.
 */
FILE *
P_file_open_w (char *name)
{
  FILE *result;
  struct flock fl = {0};

  if (strcmp (name, "stdout") == 0)
    {
      result = stdout;
    }
  else if (strcmp (name, "stderr") == 0)
    {
      result = stderr;
    }
  else
    {
      if (!(result = fopen (name, "w")))
	P_punt ("io_util.c:P_file_open_w:%d Unable to open file %s for "
		"writing", __LINE__ - 1, name);

      fl.l_type = F_WRLCK;

      /* Lock the file */
      if (fcntl (fileno (result), F_SETLK, &fl) == -1)
	{
	  fclose (result);
	  perror ("Error locking file");
	  P_punt ("io_util.c:P_file_open_w:%d Could not obtain exclusive lock "
		  "on\nfile %s", __LINE__ - 1, name);
	}
    }

  return (result);
}

/*! \brief Closes a file
 *
 * \param file
 *  the open filehandle to close.
 */
void
P_file_close (FILE *file)
{
  struct flock fl = {0};

  if (file != stdin && file != stdout && file != stderr)
    {
      /* Tell the parser we're done with this file. */
      P_read_finish (file);

      fl.l_type = F_UNLCK;

      /* Unlock the file */
      fcntl (fileno (file), F_SETLK, &fl);

      fclose (file);
    }
}

