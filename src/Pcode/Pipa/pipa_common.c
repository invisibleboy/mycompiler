/*****************************************************************************\
 *
 *                    Illinois Open Source License
 *                     University of Illinois/NCSA
 *                         Open Source License
 *
 * Copyright (c) 2004, The University of Illinois at Urbana-Champaign.
 * All rights reserved.
 *
 * Developed by:
 *
 *              IMPACT Research Group
 *
 *              University of Illinois at Urbana-Champaign
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
 *      File:    pipa_common.c
 *      Author:  Erik Nystrom
 *      Copyright (c) 2003  Erik Nystrom, Wen-mei Hwu
 *              and The Board of Trustees of the University of Illinois.
 *              All rights reserved.
 *      The University of Illinois software License Agreement
 *      specifies the terms and conditions for redistribution.
\*****************************************************************************/

#include "pipa_common.h"
#include <sys/timeb.h>
#include <sys/types.h>
#include <dirent.h>

#include <sys/resource.h>
#include <unistd.h>

int ipa_pointer_size = 0;

void
catch(char *str, int val1, int val2)
{
  if (val2 & (0x0004 | 0x0008))
    printf("CATCH %s %x <- %x \n",str, val1, val2);
}

/*****************************************************************************
 * Simple Helper functions
 *****************************************************************************/

double
IPA_GetTime ()
{
  struct timeb curtime;
  double t;

  ftime (&curtime);

  t = (double) curtime.time;
  t += ((double) curtime.millitm) / 1000;

  return t;
}

#define RU_USER(u)  get_dtime((u).ru_utime.tv_sec, (u).ru_utime.tv_usec)
#define RU_SYS(u)  get_dtime((u).ru_stime.tv_sec, (u).ru_stime.tv_usec)

static double
get_dtime(long s, long us)
{
  double d;

  d = (double)s + (((double)us) / 1000000.0);

  /*printf("DTIME %ld %ld %f \n",s,us,d);*/

  return d;
}

void
IPA_GetMemUse()
{
  static struct rusage prev_usage;
  static struct rusage start_usage;
  struct rusage usage;
  static int first = 1;

  if (first)
    {
      getrusage(RUSAGE_SELF, &prev_usage);
      start_usage = prev_usage;
      first = 0;
    }
  else
    {
      getrusage(RUSAGE_SELF, &usage);

#if 0
      printf("%f \n",RU_USER(usage));
      printf("%f \n",RU_USER(start_usage));
      printf("%f \n",RU_SYS(usage));
      printf("%f \n",RU_SYS(start_usage));
#endif
#if 0
      printf("MEM %ld : TIME (u%0.2f  s%0.2f) (u%0.3f s%0.3f) \n",
	     usage.ru_maxrss,
	     RU_USER(usage) - RU_USER(prev_usage),
	     RU_SYS(usage) - RU_SYS(prev_usage),
	     RU_USER(usage) - RU_USER(start_usage),
	     RU_SYS(usage) - RU_SYS(start_usage));
#endif
      prev_usage = usage;
    }
}

void
IPA_TrackTime(int end_round)
{
  static double prev_time = 0.0;
  static double round_time = 0.0;
  static double start_time = 0.0;
  double cur_time;

  if (prev_time == 0.0)
    {
      prev_time = IPA_GetTime ();
      round_time = prev_time;
      start_time = prev_time;
    }
  else
    {
      cur_time = IPA_GetTime ();
/*       printf("## TIME %0.2f (R %0.2f) (T %0.2f) ##\n", */
/* 	     cur_time - prev_time, */
/* 	     cur_time - round_time, */
/* 	     cur_time - start_time); */
      
      prev_time = cur_time;
      if (end_round)
	round_time = cur_time;
    }
}

void
IPA_find_file_strtoken (FILE * file, char *string)
{
  char buffer[1024];

  buffer[0] = 0;
  while ((!feof (file)) && (strcmp (string, buffer) != 0))
    {
      fscanf (file, "%s", buffer);
#if 0
      printf ("SKIP: [%s] ", buffer);
#endif
    }
}

void
IPA_find_file_chartoken (FILE * file, char token)
{
  char buffer = 0;

  while ((!feof (file)) && (token != buffer))
    {
      buffer = getc (file);
#if 0
      printf ("SKIP: [%c] ", buffer);
#endif
    }
}

int
IPA_find_file_isnext_char (FILE * file, char token)
{
  char ch;

  while ((ch = fgetc (file)) && isspace (ch));

  if (ch == token)
    return 1;

  ungetc (ch, file);
  return 0;
}

FILE *
IPA_db_stdout ()
{
  return stdout;
}

int
IPA_mapid (HashTable id_htab, int oldid)
{
  int new_id;

  if (!id_htab)
    return oldid;

  new_id = (int) (long) HashTable_find_or_null (id_htab, oldid);
  if (!new_id)
    return oldid;

  return new_id;
}

FILE *
IPA_fopen(char *basedir, char *ipa_subdir, char *name, char *ext, char *rw)
{
  FILE *file;
  DIR *dir;
  char fullname[256];
  int index;
  
  fullname[0] = 0;
  index = 0;

  if (basedir)
    index += sprintf((fullname + index),"%s/", basedir);

  if (ipa_subdir)
    {
      index += sprintf((fullname + index),"%s/", ipa_subdir);

      if (!(dir = opendir (fullname)))
	{
	  if (basedir)
	    I_punt("Directory %s does not exist\n",fullname);
	  else
	    {
	      char dircmd[256];
	      sprintf(dircmd,"mkdir %s",fullname);
	      system (dircmd);
	    }
	}
      else
	closedir (dir);
    }

  if (name)
    index += sprintf((fullname + index),"%s", name);

  if (ext)
    index += sprintf((fullname + index),".%s", ext);
  
  file = fopen(fullname, rw);
  
  return file;
}
