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
 *      File:   c_basic.c
 *      Author: Po-hua Chang
 *      Creation Date:  June 1990
 *      Modified By: XXX, date, time, why
 *      Copyright (c) 1991 Po-hua Chang and Wen-mei Hwu. All rights reserved.  
 *      All rights granted to the University of Illinois.
 *      The University of Illinois software License Agreement
 *      specifies the terms and conditions for redistribution.
\*****************************************************************************/

/*===========================================================================
 *      Description :   Basic data anf file operations, plus system functions.
 *==========================================================================*/
/* 10/29/02 REK Adding config.h */
#include <config.h>
#include <stdio.h>
#include <string.h>
#include <errno.h>
#include <sys/types.h>
/*#include <sys/stat.h>*/
#include <signal.h>
#ifndef __WIN32__
#include <unistd.h>
#endif
#include <time.h>
#include <stdlib.h>
#include <malloc.h>
#include <library/c_basic.h>

/*------------------------------------------------------------------------*/
#undef DEBUG_MALLOC

/*------------------------------------------------------------------------*/
/*
 *      BASIC DATA TYPES 
 */
int
C_is_space (int ch)
{
  return ((ch == ' ') || (ch == C_NL) || (ch == C_HT) || (ch == C_VT)
          || (ch == C_CR) || (ch == C_FF));
}
int
C_is_alpha (int ch)
{
  return (((ch >= 'a') && (ch <= 'z')) || ((ch >= 'A') && (ch <= 'Z')));
}
int
C_is_lower (int ch)
{
  return ((ch >= 'a') && (ch <= 'z'));
}
int
C_is_upper (int ch)
{
  return ((ch >= 'A') && (ch <= 'Z'));
}
int
C_is_digit (int ch)
{
  return ((ch >= '0') && (ch <= '9'));
}
int
C_is_alpha_numeric (int ch)
{
  return (C_is_alpha (ch) || C_is_digit (ch));
}
int
C_to_lower (int ch)
{
  if (C_is_upper (ch))
    {
      return (ch - 'A' + 'a');
    }
  else
    {
      return ch;
    }
}
int
C_to_upper (int ch)
{
  if (C_is_lower (ch))
    {
      return (ch - 'a' + 'A');
    }
  else
    {
      return ch;
    }
}
int
C_is_hex_digit (int ch)
{
  return (((ch >= '0') && (ch <= '9')) ||
          ((ch >= 'a') && (ch <= 'f')) || ((ch >= 'A') && (ch <= 'F')));
}
int
C_is_oct_digit (int ch)
{
  return ((ch >= '0') && (ch <= '7'));
}
int
C_is_control (int ch)           /* ASCII */
{
  if (ch == 0x7F)
    return 1;                   /* DEL */
  return ((ch >= 0) && (ch <= 0x1F));   /* US */
}
int
C_is_graph (int ch)             /* ASCII */
{
  return ((ch >= 0x20) && (ch <= 0x7E));
}
/*
 *      INTEGER OPERATIONS
 */
#define NUM_BITS        32
static unsigned long p2[32] = {
  1,                            /* 2^0 */
  2,                            /* 2^1 */
  4,                            /* 2^2 */
  8,                            /* 2^3 */
  16,                           /* 2^4 */
  32,                           /* 2^5 */
  64,                           /* 2^6 */
  128,                          /* 2^7 */
  256,                          /* 2^8 */
  512,                          /* 2^9 */
  1024,                         /* 2^10 */
  2 * 1024,                     /* 2^11 */
  4 * 1024,                     /* 2^12 */
  8 * 1024,                     /* 2^13 */
  16 * 1024,                    /* 2^14 */
  32 * 1024,                    /* 2^15 */
  64 * 1024,                    /* 2^16 */
  128 * 1024,                   /* 2^17 */
  256 * 1024,                   /* 2^18 */
  512 * 1024,                   /* 2^19 */
  1024 * 1024,                  /* 2^20 */
  2 * 1024 * 1024,              /* 2^21 */
  4 * 1024 * 1024,              /* 2^22 */
  8 * 1024 * 1024,              /* 2^23 */
  16 * 1024 * 1024,             /* 2^24 */
  32 * 1024 * 1024,             /* 2^25 */
  64 * 1024 * 1024,             /* 2^26 */
  128 * 1024 * 1024,            /* 2^27 */
  256 * 1024 * 1024,            /* 2^28 */
  512 * 1024 * 1024,            /* 2^29 */
  1024 * 1024 * 1024,           /* 2^30 */
  2UL * 1024 * 1024 * 1024      /* 2^31 */
};
static unsigned long m2[33] = {
  0x00000000,
  0x00000001,
  0x00000003,
  0x00000007,
  0x0000000F,
  0x0000001F,
  0x0000003F,
  0x0000007F,
  0x000000FF,
  0x000001FF,
  0x000003FF,
  0x000007FF,
  0x00000FFF,
  0x00001FFF,
  0x00003FFF,
  0x00007FFF,
  0x0000FFFF,
  0x0001FFFF,
  0x0003FFFF,
  0x0007FFFF,
  0x000FFFFF,
  0x001FFFFF,
  0x003FFFFF,
  0x007FFFFF,
  0x00FFFFFF,
  0x01FFFFFF,
  0x03FFFFFF,
  0x07FFFFFF,
  0x0FFFFFFF,
  0x1FFFFFFF,
  0x3FFFFFFF,
  0x7FFFFFFF,
  0xFFFFFFFF,
};

/*
 *      if n is a power of 2, return log(n)
 *      else return log(n)+1
 */
int
C_log2 (C_Integer n)
{
  int i;
  for (i = 0; i < NUM_BITS; i++)
    {
      if (n <= p2[i])
        return i;               /* p2[i] is enough to contain n */
    }
  return -1;                    /* too big */
}
int
C_is_log2 (C_Integer n)
{
  int i;
  for (i = 0; i < NUM_BITS; i++)
    {
      if (n == p2[i])
        return 1;
      if (n < p2[i])
        return 0;               /* not a power of 2 */
    }
  return 0;                     /* too big */
}
C_Integer C_pow2 (int n)
{
  if ((n >= 0) && (n < NUM_BITS))
    return p2[n];
  return -1;                    /* too big or too small */
}
C_Integer C_mask (int n)
{
  if ((n >= 0) && (n < NUM_BITS))
    return m2[n];
  return 0;                     /* too big or too small */
}
/*
 *      STRING OPERATIONS
 */
int
C_strlen (C_String str)
{
  if (str == 0)
    return 0;
  return strlen (str);
}
C_String C_strcpy (C_String dst, C_String src)
{
  return strcpy (dst, src);
}
C_String C_strncpy (C_String dst, C_String src, int n)
{
  return strncpy (dst, src, n);
}
C_String C_strcat (C_String dst, C_String src)
{
  return strcat (dst, src);
}
C_String C_strncat (C_String dst, C_String src, int n)
{
  return strncat (dst, src, n);
}
int
C_strcmp (C_String str1, C_String str2)
{
  return strcmp (str1, str2);
}
int
C_strncmp (C_String str1, C_String str2, int n)
{
  return strncmp (str1, str2, n);
}
C_String C_strchr (C_String str, int ch)
{
  return strchr (str, ch);
}
C_String C_strrchr (C_String str, int ch)
{
  return strrchr (str, ch);
}
int
C_strspn (C_String str, C_String ct)
{
  return strspn (str, ct);
}
int
C_strcspn (C_String str, C_String ct)
{
  return strcspn (str, ct);
}
C_String C_strpbrk (C_String str, C_String ct)
{
  return strpbrk (str, ct);
}
C_String C_strstr (C_String str1, C_String str2)
{
  int i, len, max;
  len = strlen (str2);
  max = strlen (str1);
  if (max < len)
    return C_NIL;
  for (i = 0; i <= (max - len); i++)
    {
      if (!strncmp (str1 + i, str2, len))
        return (str1 + i);
    }
  return C_NIL;
}
C_String C_strrstr (C_String str1, C_String str2)
{
  int i, len, max;
  len = strlen (str2);
  max = strlen (str1);
  if (max < len)
    return C_NIL;
  for (i = (max - len); i >= 0; i--)
    {
      if (!strncmp (str1 + i, str2, len))
        return (str1 + i);
    }
  return C_NIL;
}
C_String C_strsave (C_String str)
{
  C_String new_str;
#if SUN
  new_str = strdup (str);
  C_assert (new_str != 0, "C_strsave failed: out of memory space");
#else
  new_str = (C_String) C_malloc (sizeof (char) * (strlen (str) + 1));
  C_assert (new_str != 0, "C_strsave failed: out of memory space");
  strcpy (new_str, str);
#endif
  return new_str;
}
/* convert the parser T_char_lit representation (string) to scalar value. */
/* the character string contains 'X' around the character X.
 * in this function, we simply assume ' exist and really do no
 * checking on whther or not ' really is present.
 * ## trigraph style is currently missing. ##
 */
int
C_str2char (C_String str)
{
  int n;
  /* str = "'ch'" */
  /* type 1 : simple format */
  if (str[1] != '\\')           /* \ */
    return str[1];
/* type 2 : \ooo (octal) *//* octal */
  if ((str[2] >= '0') && (str[2] <= '7'))
    {
      if ((str[2] == '0') && (str[3] == '\''))
        return C_NU;
      sscanf (str + 2, "%o", &n);
      return n;
    }
/* type 3 : \xhh (hex) *//* hexdecimal */
  /* BCC - bug fix for a['\x'] - 4/14/96 */
  if ((str[2] == 'x' || str[2] == 'X') && str[3] != '\'')
    {
      sscanf (str + 3, "%x", &n);
      return n;
    }
  /* type 4 : \control */
  switch (str[2])
    {                           /* control character */
    case 'n':
      return C_NL;
    case 't':
      return C_HT;
    case 'v':
      return C_VT;
    case 'b':
      return C_BS;
    case 'r':
      return C_CR;
    case 'f':
      return C_FF;
    case 'a':
      return C_ALERT;
    default:
      return (str[2]);
    }

}
/*--------------------------------------------------------------------------*/
/* We will implement a binary tree string management. */
typedef struct _Str
{
  C_String name;
  C_Pointer ptr;
  struct _Str *left;
  struct _Str *right;
}
_Str, *Str;
static Str str_root = 0;

/* String Save Function. */
C_String C_savestr (C_String str)
{
  C_String new_str = (C_String) C_malloc2 (C_strlen (str) + 1);
  C_strcpy (new_str, str);
  return (new_str);
}
/*      Find a string.  */
static Str
FindS (Str T, C_String str)
{
  int diff;
  if (T == 0)
    return 0;
  if (str == 0)
    return 0;
  diff = C_strcmp (str, T->name);
  if (diff == 0)
    return (T);                 /* find a match */
  if (diff > 0)
    return (FindS (T->left, str));      /* search left */
  else
    return (FindS (T->right, str));     /* search right */
}
/*      Add a string.   */
static Str
AddS (Str T, C_String str)
{
  int diff;
  Str new_str;
  if (str == 0)
    return 0;
  if (T == 0)
    {                           /* add new string */
      new_str = (Str) C_malloc2 (sizeof (_Str));
      new_str->name = C_savestr (str);
      new_str->ptr = 0;
      new_str->left = new_str->right = 0;
      return (new_str);
    }
  diff = C_strcmp (str, T->name);
  if (diff == 0)
    return (T);                 /* already exist */
  if (diff > 0)
    {
      T->left = AddS (T->left, str);    /* add to left */
      return (T);
    }
  else
    {
      T->right = AddS (T->right, str);  /* add to right */
      return (T);
    }
}

static int string_count, string_length;

/*      Find and Add a string.   */
static Str
FindAndAddS (Str T, C_String str, Str * node)
{
  int diff;
  Str new_str;

  if (T == 0)
    {                           /* add new string */
      string_count++;
      string_length += strlen (str) + 1;
      new_str = (Str) C_malloc2 (sizeof (_Str));
      *node = new_str;
      new_str->name = C_savestr (str);
      new_str->ptr = 0;
      new_str->left = new_str->right = 0;
      return (new_str);
    }
  diff = C_strcmp (str, T->name);
  if (diff == 0)
    {
      *node = T;
      return (T);               /* already exist */
    }
  if (diff > 0)
    {
      T->left = FindAndAddS (T->left, str, node);       /* add to left */
      return (T);
    }
  else
    {
      T->right = FindAndAddS (T->right, str, node);     /* add to right */
      return (T);
    }
}

/* In the old implementation, the tree needs to be searched 3 times to
 * insert a new string.
 */
#if 0
/*      This function maintains all define-once-only names.
 *      It maintains a table of a list of unique names used
 *      by the entire program. This will save some space.
 */
C_String C_findstr (C_String str)
{
  Str ptr;
  if ((str == 0) || (str[0] == '\0'))
    {
      C_assert (0, "C_findstr(str): str cannot be a null string");
    }
  ptr = FindS (str_root, str);
  if (ptr == 0)
    {
      str_root = AddS (str_root, str);
      ptr = FindS (str_root, str);
    }
  return (ptr->name);
}
#endif

C_String C_findstr (C_String str)
{
  Str ptr;

  if ((str == 0) || (str[0] == '\0'))
    {
      C_assert (0, "C_findstr(str): str cannot be a null string");
    }
  str_root = FindAndAddS (str_root, str, &ptr);
  return (ptr->name);
}

C_Pointer C_addptr (C_String str, C_Pointer ptr)
{
  Str p;
  if ((str == 0) || (str[0] == '\0'))
    {
      C_assert (0, "C_addptr(str): str cannot be a null string");
    }
  p = FindS (str_root, str);
  if (p == 0)
    {
      str_root = AddS (str_root, str);
      p = FindS (str_root, str);
    }
  p->ptr = ptr;
  return (p->ptr);
}
C_Pointer C_findptr (C_String str)
{
  Str ptr;
  if ((str == 0) || (str[0] == '\0'))
    {
      C_assert (0, "C_findptr(str): str cannot be a null string");
    }
  ptr = FindS (str_root, str);
  if (ptr == 0)
    return 0;
  return (ptr->ptr);
}
static void
VisitS (Str T, int (*fn) (C_String name, C_Pointer ptr))
{
  if (T == 0)
    return;
  VisitS (T->left, fn);
  VisitS (T->right, fn);
  if (fn == 0)
    return;
  (*fn) (T->name, T->ptr);
}
void
C_for_all_ptr (int (*fn) (C_String name, C_Pointer ptr))
{
  VisitS (str_root, fn);
}
/*--------------------------------------------------------------------------*/
/*
 *      NUMBER CONVERSION OPERATIONS
 */
C_Float C_string_to_float (C_String str)
{
  return ((C_Float) atof (str));
}
C_String C_float_to_string (C_Float value, C_String line)
{
  sprintf (line, "%e", (double) value);
  return line;
}
C_Double C_string_to_double (C_String str)
{
  return ((C_Double) atof (str));
}
C_String C_double_to_string (C_Double value, C_String line)
{
  sprintf (line, "%e", (double) value);
  return line;
}
C_Integer C_string_to_char (C_String str)       /* ANSI C standard */
{
  int ch1, ch2;
  ch1 = str[0];
  if (C_is_alpha_numeric (ch1))
    {
      return ch1;
    }
  else if (ch1 == C_BACKSLASH)
    {
      ch2 = str[1];
      switch (ch2)
        {
        case 'n':
          return C_NL;
        case 't':
          return C_HT;
        case 'v':
          return C_VT;
        case 'b':
          return C_BS;
        case 'r':
          return C_CR;
        case 'f':
          return C_FF;
        case 'a':
          return C_ALERT;
        default:
          if (ch2 == 'x')
            {
              return C_string_to_hex (str + 2);
            }
          else if (C_is_oct_digit (ch2))
            {
              return C_string_to_oct (str + 1);
            }
          else
            {
              return ch2;
            }
        }
    }
  else if (ch1 == C_QUESTION)
    {
      ch2 = str[1];
      if (ch2 == C_QUESTION)
        {
          switch (str[2])
            {
            case '=':
              return '#';
            case '/':
              return '\\';
            case '\'':
              return '^';
            case '(':
              return '[';
            case ')':
              return ']';
            case '!':
              return '|';
            case '<':
              return '{';
            case '>':
              return '}';
            case '-':
              return '~';
            default:
              return '?';
            }
        }
      else
        {
          return C_QUESTION;
        }
    }
  else
    {
      return ch1;
    }
}
C_String C_char_to_string (int value, C_String line)
{
  if (C_is_control (value))
    {                           /* control characters */
      line[0] = C_S_QUOTE;
      line[1] = '\\';
      line[3] = C_S_QUOTE;
      line[4] = '\0';
      switch (value)
        {
        case C_NL:
          line[2] = 'n';
          break;
        case C_HT:
          line[2] = 't';
          break;
        case C_VT:
          line[2] = 'v';
          break;
        case C_BS:
          line[2] = 'b';
          break;
        case C_CR:
          line[2] = 'r';
          break;
        case C_FF:
          line[2] = 'f';
          break;
        case C_ALERT:
          line[2] = 'a';
          break;
        default:                /* unknown control character */
          sprintf (line, "'\\%o'", value);
        }
    }
  else if (C_is_graph (value))
    {                           /* printing characters */
      line[0] = C_S_QUOTE;
      line[1] = value;
      line[2] = C_S_QUOTE;
      line[3] = '\0';
    }
  else
    {
      sprintf (line, "'\\%o'", value);
    }
  return line;
}
C_Integer C_string_to_integer (C_String str)
{
  C_Integer imax;
  sscanf (str, ITintmaxformat, &imax);
  return (imax);
}
C_String C_integer_to_string (C_Integer value, C_String line)
{
  sprintf (line, ITintmaxformat, value);
  return line;
}
C_Integer C_string_to_hex (C_String str)
{
  C_Integer value;
  sscanf (str, ITintmaxhexfmt, &value);
  return value;
}
C_String C_hex_to_string (C_Integer value, C_String line)
{
  sprintf (line, "Ox" ITintmaxhexfmt, value);
  return line;
}
C_Integer C_string_to_oct (C_String str)
{
  C_Integer value;
  sscanf (str, ITintmaxoctfmt, &value);
  return value;
}
C_String C_oct_to_string (C_Integer value, C_String line)
{
  sprintf (line, "O" ITintmaxoctfmt, value);
  return line;
}
/*--------------------------------------------------------------------------*/
/*
 *      Assertion and error reporting.
 */
void
C_abort (void)
{
  abort ();
}
void
C_exit (int status)
{
  exit (status);
}
void
C_assert (int expression, char *error_mesg)
{
  if (expression == 0)
    {
      fprintf (stderr, "# fatal error: %s\n", error_mesg);
      C_exit (1);
    }
}
/*--------------------------------------------------------------------------*/
#ifdef DEBUG_MALLOC
static int malloc_size = 0;
#endif

/*
 *      Memory allocation.
 */
C_Pointer C_calloc (int num_obj, int size_obj)
{
  C_Pointer new_ptr;
  new_ptr = (C_Pointer) calloc (num_obj, size_obj);
  return new_ptr;
}
C_Pointer C_malloc (int size)
{
  C_Pointer new_ptr;
  new_ptr = (C_Pointer) malloc (size);
#ifdef DEBUG_MALLOC
  malloc_size += size;
  fprintf (stderr, "> malloc(%d) : %d\n", size, malloc_size);
#endif
  return new_ptr;
}
C_Pointer C_calloc2 (int num_obj, int size_obj)
{
  C_Pointer new_ptr;
  new_ptr = C_calloc (num_obj, size_obj);
  C_assert (new_ptr != 0, "out of memory space");
  return new_ptr;
}
C_Pointer C_malloc2 (int size)
{
  C_Pointer new_ptr;
  new_ptr = C_malloc (size);
  C_assert (new_ptr != 0, "out of memory space");
  return new_ptr;
}
void
C_free (C_Pointer ptr)
{
  free (ptr);
}
/*--------------------------------------------------------------------------*/
/*
 *      File handling.
 */
#define MAX_FILE                512
#define MAX_NAME_LENGTH         512

static struct
{
  short defined;
  short eof;
  short mode;
  char *name;
  FILE *F;
}
files[MAX_FILE];

#define INIT_FILE       {if (!init) init_file();}
static int init = 0;
static void
init_file (void)
{
  int i;
  for (i = 0; i < MAX_FILE; i++)
    {
      files[i].defined = 0;
    }
  files[C_STDIN].defined = 1;
  files[C_STDIN].mode = C_OPEN_READ_ONLY;
  files[C_STDIN].name = "stdin";
  files[C_STDIN].F = stdin;
  files[C_STDOUT].defined = 1;
  files[C_STDOUT].mode = C_OPEN_WRITE_ONLY;
  files[C_STDOUT].name = "stdout";
  files[C_STDOUT].F = stdout;
  files[C_STDERR].defined = 1;
  files[C_STDERR].mode = C_OPEN_WRITE_ONLY;
  files[C_STDERR].name = "stderr";
  files[C_STDERR].F = stderr;
  init = 1;
}
static int
find_file (char *name)
{
  int i;
  /*
   *  see if the entry has been defined.
   */
  for (i = 0; i < MAX_FILE; i++)
    if (files[i].defined && !C_strcmp (name, files[i].name))
      return i;
  return C_BAD_INDEX;
}
static int
new_file (char *name)
{
  int i;
  /*
   *  create a new entry.
   */
  for (i = 0; i < MAX_FILE; i++)
    if (files[i].defined == 0)
      break;
  C_assert (i != MAX_FILE, "too many files");
  files[i].defined = 1;
  files[i].eof = 0;
  files[i].mode = 0;
  files[i].name = C_strsave (name);
  files[i].F = 0;
  return i;
}
static void
free_file (int i)
{
  files[i].defined = 0;
  files[i].eof = 1;
  files[i].mode = 0;
  files[i].name = 0;
  files[i].F = 0;
}
/*---------------------------------------------------------------------------*/
extern int errno;
/*
 *      Returns 0 if the file exists. Otherwise, returns 1.
 */

/*
 *      OPEN / CLOSE REGULAR FILE
 */
C_File C_open_file (C_String file_name, int mode)
{
  int id;
  FILE *F = NULL;
  INIT_FILE;
  id = find_file (file_name);
  C_assert (id == C_BAD_INDEX,
            "C_open_file failed: cannot reopen an open file");
  switch (mode)
    {                           /* open file */
    case C_OPEN_READ_ONLY:
      F = fopen (file_name, "r");
      break;
    case C_OPEN_WRITE_ONLY:
      F = fopen (file_name, "w");
      break;
    case C_OPEN_APPEND_ONLY:
      F = fopen (file_name, "a");
      break;
    default:
      C_assert (0, "C_open_file: illegal mode");
    }
  if (F == 0)
    return C_BAD_INDEX;         /* cannot open file */
  id = new_file (file_name);
  files[id].eof = 0;
  files[id].mode = mode;
  files[id].F = F;
  return id;
}
int
C_flush_file (C_File file)
{
  INIT_FILE;
  if ((file < 0) || (file >= MAX_FILE))
    return C_FAILURE;           /* illegal file id */
  if (!files[file].defined)
    return C_FAILURE;           /* inactive file */
  if (files[file].mode == C_OPEN_READ_ONLY)
    return C_FAILURE;           /* cannot flush an input file */
  fflush (files[file].F);
  return C_SUCCESS;
}
int
C_close_file (C_File file)
{
  INIT_FILE;
  if ((file < 0) || (file >= MAX_FILE))
    return C_FAILURE;           /* illegal file id */
  if (!files[file].defined)
    return C_FAILURE;           /* inactive file */
  fclose (files[file].F);
  free_file (file);
  return C_SUCCESS;
}
int
C_create_tmp_file (C_String name)
{
  INIT_FILE;
  C_strcpy (name, "___tmp.XXXXXX");
#ifndef WIN32
  mkstemp (name);
#else
  _mktemp (name);
#endif
  return C_open_file (name, C_OPEN_WRITE_ONLY);
}
int
C_remove_file (C_String name)
{
#ifndef WIN32
  if (unlink (name) == 0)
    return C_SUCCESS;
#else
  if (_unlink (name) == 0)
    return C_SUCCESS;
#endif

  return C_FAILURE;
}
int
C_rename_file (C_String name, C_String new_name)
{
  int status;
#ifndef __WIN32__
  status = link (name, new_name);       /* must be on the same disk */
#else
  /* ADA 5/29/96: Win96/NT has no link, use rename() */
  status = rename (name, new_name);     /* must be on the same disk */
#endif
  if (status == C_BAD_INDEX)
    return C_FAILURE;
  if (unlink (name) == 0)
    return C_SUCCESS;
  return C_FAILURE;
}
/*---------------------------------------------------------------------------*/
/*
 *      CREATE / DESTROY DIRECTORY
 */
int
C_destroy_directory (C_String dir_name)
{
#ifndef WIN32
  if (rmdir (dir_name) == 0)
    return C_SUCCESS;
#else
  if (_rmdir (dir_name) == 0)
    return C_SUCCESS;
#endif
  return C_FAILURE;
}
/*
 *      Organized directory.
 */
static C_String db_prefix = 0;
static char db_index[MAX_NAME_LENGTH];

static C_String
index_file (C_String db_prefix)
{
  sprintf (db_index, "%s/__INDEX__", db_prefix);
  return db_index;
}
static int
add_record_index (C_String db_prefix, C_String name)
{
  int fi;
  index_file (db_prefix);
  fi = C_open_file (db_index, C_OPEN_APPEND_ONLY);
  C_assert (fi != C_BAD_INDEX, "database: cannot open the index file");
  C_write_line (fi, name);
  C_close_file (fi);
  return C_SUCCESS;
}
static int
delete_record_index (C_String db_prefix, C_String name)
{
  int fi, ft;
  char tmpfile[MAX_NAME_LENGTH];
  char line[MAX_NAME_LENGTH];
  index_file (db_prefix);
  fi = C_open_file (db_index, C_OPEN_READ_ONLY);
  C_assert (fi != C_BAD_INDEX, "database: cannot open the index file");
  ft = C_create_tmp_file (tmpfile);
  C_assert (ft != C_BAD_INDEX, "database: cannot operate on the index  file");
  for (;;)
    {
      C_read_line (fi, line, MAX_NAME_LENGTH);
      if (C_eof (fi))
        break;
      if (!strcmp (line, name))
        continue;
      C_write_line (ft, line);
    }
  C_close_file (fi);
  C_close_file (ft);
  C_remove_file (db_index);
  C_rename_file (tmpfile, db_index);
  return C_SUCCESS;
}
static int
destroy_all_records (C_String db_name)
{
  char line[MAX_NAME_LENGTH];
  char name[MAX_NAME_LENGTH];
  int fi;
  index_file (db_name);
  fi = C_open_file (db_index, C_OPEN_READ_ONLY);
  if (fi == C_BAD_INDEX)
    return C_FAILURE;           /* corrupted database */
  for (;;)
    {
      C_read_line (fi, line, MAX_NAME_LENGTH);
      if (C_eof (fi))
        break;
      sprintf (name, "%s/%s", db_name, line);
      C_remove_file (name);
    }
  C_close_file (fi);
  C_remove_file (db_index);
  return C_SUCCESS;
}
/*
 *      DATABASE (organized directory)
 */
int
C_database_size (C_String db_name)
{
  int size, fi;
  char line[MAX_NAME_LENGTH];
  index_file (db_name);
  fi = C_open_file (db_index, C_OPEN_READ_ONLY);
  if (fi == C_BAD_INDEX)
    return -1;
  size = 0;
  for (;;)
    {
      C_read_line (fi, line, MAX_NAME_LENGTH);
      if (C_eof (fi))
        break;
      size += 1;
    }
  C_close_file (fi);
  return size;
}

int
C_destroy_database (C_String db_name)
{
  destroy_all_records (db_name);
  return C_destroy_directory (db_name);
}

int
C_disconnect_database (C_String db_name)
{
  db_prefix = 0;
  return C_SUCCESS;             /* do nothing */
}
C_File C_read_record (C_String record_name)
{
  char name[MAX_NAME_LENGTH];
  int i;
  C_assert (db_prefix != 0, "C_read_record: not connected to any database");
  sprintf (name, "%s/%s", db_prefix, record_name);
  i = C_open_file (name, C_OPEN_READ_ONLY);
  return i;
}
C_File C_write_record (C_String record_name)
{
  char name[MAX_NAME_LENGTH];
  int i;
  C_assert (db_prefix != 0, "C_write_record: not connected to any database");
  sprintf (name, "%s/%s", db_prefix, record_name);
  i = C_open_file (name, C_OPEN_WRITE_ONLY);
  add_record_index (db_prefix, record_name);
  return i;
}
int
C_destroy_record (C_String record_name)
{
  char name[MAX_NAME_LENGTH];
  int i;
  C_assert (db_prefix != 0,
            "C_destroy_record: not connected to any database");
  sprintf (name, "%s/%s", db_prefix, record_name);
  i = C_remove_file (name);
  delete_record_index (db_prefix, record_name);
  return i;
}
/*
 *      READING / WRITING FILE
 */
C_String C_file_name (C_File file)
{
  INIT_FILE;
  if ((file < 0) || (file >= MAX_FILE))
    return 0;                   /* illegal file id */
  if (!files[file].defined)
    return 0;                   /* inactive file */
  return files[file].name;
}
int
C_eof (C_File file)
{
  INIT_FILE;
  if ((file < 0) || (file >= MAX_FILE))
    return 1;                   /* illegal file id */
  if (!files[file].defined)
    return 1;                   /* inactive file */
  return files[file].eof;
}
int
C_read_char (C_File file, C_Integer * value)
{
  int ch;
  FILE *F;
  INIT_FILE;
  if ((file < 0) || (file >= MAX_FILE))
    return 0;                   /* illegal file id */
  if (!files[file].defined)
    return 0;                   /* inactive file */
  F = files[file].F;
  ch = getc (F);
  *value = ch;
  files[file].eof = (ch == EOF);
  return (ch != EOF);
}
int
C_read_word (C_File file, C_String line, int length)
{
  int ch, i;
  FILE *F;
  INIT_FILE;
  if ((file < 0) || (file >= MAX_FILE))
    return 0;                   /* illegal file id */
  if (!files[file].defined)
    return 0;                   /* inactive file */
  F = files[file].F;
  i = 0;
  while ((ch = getc (F)) != EOF)
    {                           /* skip over white space */
      if (!C_is_space (ch))
        {
          line[i] = ch;
          i += 1;
          break;
        }
    }
  while ((ch = getc (F)) != EOF)
    {                           /* read until white space */
      if (C_is_space (ch))
        break;
      line[i] = ch;
      i += 1;
      if (i >= (length - 1))
        break;
    }
  line[i] = '\0';
  files[file].eof = (ch == EOF);
  return (ch != EOF);
}
int
C_read_integer (C_File file, C_Integer * value)
{
  FILE *F;
  INIT_FILE;
  if ((file < 0) || (file >= MAX_FILE))
    return 0;                   /* illegal file id */
  if (!files[file].defined)
    return 0;                   /* inactive file */
  F = files[file].F;
  return fscanf (F, ITintmaxformat, value);
}
int
C_read_float (C_File file, C_Float * value)
{
  int i;
  float f;
  FILE *F;
  INIT_FILE;
  if ((file < 0) || (file >= MAX_FILE))
    return 0;                   /* illegal file id */
  if (!files[file].defined)
    return 0;                   /* inactive file */
  F = files[file].F;
  i = fscanf (F, "%e", &f);
  *value = f;
  return i;
}
int
C_read_double (C_File file, C_Double * value)
{
  int i;
  float f;
  FILE *F;
  INIT_FILE;
  if ((file < 0) || (file >= MAX_FILE))
    return 0;                   /* illegal file id */
  if (!files[file].defined)
    return 0;                   /* inactive file */
  F = files[file].F;
  i = fscanf (F, "%e", &f);
  *value = f;
  return i;
}
int
C_read_line (C_File file, C_String line, int length)
{
  int ch, i;
  FILE *F;
  INIT_FILE;
  if ((file < 0) || (file >= MAX_FILE))
    return 0;                   /* illegal file id */
  if (!files[file].defined)
    return 0;                   /* inactive file */
  F = files[file].F;
  i = 0;
  while ((ch = getc (F)) != EOF)
    {
      if (ch == C_NL)           /* exit loop upon EOLN */
        break;
      line[i] = ch;
      i += 1;
      if (i >= (length - 1))
        break;
    }
  line[i] = '\0';
  files[file].eof = (ch == EOF);
  return (ch != EOF);
}
int
C_read_block (C_File file, C_String line, int length)
{
  FILE *F;
  INIT_FILE;
  if ((file < 0) || (file >= MAX_FILE))
    return 0;                   /* illegal file id */
  if (!files[file].defined)
    return 0;                   /* inactive file */
  F = files[file].F;
  return (fread (line, sizeof (char), length, F));
}
int
C_write_char (C_File file, C_Integer value)
{
  FILE *F;
  INIT_FILE;
  if ((file < 0) || (file >= MAX_FILE))
    return 0;                   /* illegal file id */
  if (!files[file].defined)
    return 0;                   /* inactive file */
  F = files[file].F;
  return (putc (value, F));
}
int
C_write_word (C_File file, C_String line)
{
  FILE *F;
  INIT_FILE;
  if ((file < 0) || (file >= MAX_FILE))
    return 0;                   /* illegal file id */
  if (!files[file].defined)
    return 0;                   /* inactive file */
  F = files[file].F;
  return (fprintf (F, " %s", line));
}
int
C_write_line (C_File file, C_String line)
{
  FILE *F;
  INIT_FILE;
  if ((file < 0) || (file >= MAX_FILE))
    return 0;                   /* illegal file id */
  if (!files[file].defined)
    return 0;                   /* inactive file */
  F = files[file].F;
  return (fprintf (F, "%s\n", line));
}
int
C_write_block (C_File file, C_String line, int length)
{
  FILE *F;
  INIT_FILE;
  if ((file < 0) || (file >= MAX_FILE))
    return 0;                   /* illegal file id */
  if (!files[file].defined)
    return 0;                   /* inactive file */
  F = files[file].F;
  return (fwrite (line, sizeof (char), length, F));
}
int
C_write_integer (C_File file, C_Integer value)
{
  FILE *F;
  INIT_FILE;
  if ((file < 0) || (file >= MAX_FILE))
    return 0;                   /* illegal file id */
  if (!files[file].defined)
    return 0;                   /* inactive file */
  F = files[file].F;
  return (fprintf (F, " " ITintmaxformat, value));
}
int
C_write_float (C_File file, C_Float value)
{
  FILE *F;
  INIT_FILE;
  if ((file < 0) || (file >= MAX_FILE))
    return 0;                   /* illegal file id */
  if (!files[file].defined)
    return 0;                   /* inactive file */
  F = files[file].F;
  return (fprintf (F, " %.10f", value));
}
int
C_write_double (C_File file, C_Double value)
{
  FILE *F;
  INIT_FILE;
  if ((file < 0) || (file >= MAX_FILE))
    return 0;                   /* illegal file id */
  if (!files[file].defined)
    return 0;                   /* inactive file */
  F = files[file].F;
  return (fprintf (F, " %.10f", value));
}
/*---------------------------------------------------------------------------*/
/*
 *      Time functions.
 */
long
C_time (void)
{
  return time (0);
}
char *
C_asctime (time_t tm)
{
  return ctime (&tm);
}

/* BCC - added for garbage collection - 8/22/96 */
static void
RemoveS (Str ptr)
{
  if (ptr == 0)
    return;
  RemoveS (ptr->left);
  ptr->left = 0;
  RemoveS (ptr->right);
  ptr->right = 0;
  free (ptr->name);
  free (ptr);
}

/* BCC - added for garbage collection - 8/22/96 */
void
C_RemoveAllString ()
{
  RemoveS (str_root);
  str_root = 0;
}
