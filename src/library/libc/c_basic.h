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
 *      File:   c_basic.h
 *      Author: Po-hua Chang and Wen-mei Hwu
 *      Creation Date:  June 1990
 *      Modified By: XXX, date, time, why
 *      Copyright (c) 1991 Po-hua Chang and Wen-mei Hwu. All rights reserved.  
 *      All rights granted to the University of Illinois.
 *      The University of Illinois software License Agreement
 *      specifies the terms and conditions for redistribution.
\*****************************************************************************/
#ifndef C_BASIC_H
#define C_BASIC_H
/* 10/29/02 REK Adding config.h */
#include <config.h>
#include <time.h>

#include <library/i_types.h>

/*===========================================================================
 *      Description :   Basic data operations and system functions.
 *==========================================================================*/

/*
 *      BASIC DATA TYPES 
 */
typedef char Boolean;
typedef int Integer;
typedef float Float;
typedef double Double;
typedef char String;
typedef void *Pointer;
typedef long (*Function) (char *, ...); /* a function returning void */
typedef char C_Boolean;         /* boolean value */
typedef ITintmax C_Integer;     /* make sure can contain Pointer */
typedef float C_Float;          /* single-precision floating-point numbers */
typedef double C_Double;        /* double-precision floating-point numbers */
typedef void *C_Pointer;        /* general purpose pointer */
typedef char *C_String;         /* character string */
typedef long (*C_Function) (char *, ...);       /* a function returning void */

typedef C_Boolean *Cb;          /* boolean array */
typedef C_Integer *Ci;          /* integer array */
typedef C_Float *Cf;            /* float array */
typedef C_Double *Cd;           /* double array */

#define C_TYPE_VOID     0       /* a multiple-type data holder */
#define C_TYPE_INTEGER  1
#define C_TYPE_FLOAT    2
#define C_TYPE_DOUBLE   3
#define C_TYPE_POINTER  4

typedef union
{
  C_Boolean b;
  C_Integer i;
  C_Float f;
  C_Double d;
  C_Pointer p;
}
C_Data;

#define C_TRUE                  1       /* true boolean value */
#define C_FALSE                 0       /* false boolean value */

#define C_ALL_ONE               0xFFFFFFFF

#define C_NIL                   0       /* nil pointer/index */

#define C_SUCCESS               1
#define C_FAILURE               0       /* status of a function call */
#define C_BAD_INDEX             -1      /* illegal index */

#define C_INT_MIN               -2147483647L
#define C_INT_MAX               +2147483647L

#define C_FLOAT_DIG             6       /* decimal digits of precision */
#define C_FLOAT_EPSILON         1E-5    /* smallest number X s.t. 1+X<>X */
#define C_FLOAT_MAX             1E+37
#define C_FLOAT_MIN             1E-37

#define C_DOUBLE_DIG            9       /* decimal digits of precision */
#define C_DOUBLE_EPSILON        1E-8    /* smallest number X s.t. 1+X<>X */
#define C_DOUBLE_MAX            1E+37
#define C_DOUBLE_MIN            1E-37

/*
 *      OPERATIONS ON BASIC DATA TYPES
 */
#define C_NU            '\0'    /* null */
#define C_NL            '\n'    /* new line */
#define C_HT            '\t'    /* horizontal tab */
#define C_VT            '\v'    /* vertical tab */
#define C_BS            '\b'    /* back space */
#define C_CR            '\r'    /* carriage return */
#define C_FF            '\f'    /* form feed */
#define C_ALERT         '\a'    /* alert */
#define C_BACKSLASH     '\\'
#define C_QUESTION      '\?'
#define C_S_QUOTE       '\''
#define C_D_QUOTE       '\"'

#ifdef __cplusplus
extern "C"
{
#endif

/*
 *      CHARACTER OPERATIONS.
 */
  extern int C_is_space (int ch);
  extern int C_is_alpha (int ch);
  extern int C_is_lower (int ch);
  extern int C_is_upper (int ch);
  extern int C_is_digit (int ch);
  extern int C_is_alpha_numeric (int ch);
  extern int C_to_lower (int ch);
  extern int C_to_upper (int ch);
  extern int C_is_hex_digit (int ch);
  extern int C_is_oct_digit (int ch);

  extern int C_is_control ();   /* (int c) */
  extern int C_is_graph ();     /* (int c) */

/* 
 *      INTEGER OPERATIONS.
 */
  extern int C_log2 (C_Integer n);
  extern int C_is_log2 (C_Integer n);
  extern C_Integer C_pow2 (int n);
  extern C_Integer C_mask (int n);

/*
 *      STRING OPERATIONS
 */
  extern int C_strlen (C_String str);
  extern C_String C_strcpy (C_String dst, C_String src);
  extern C_String C_strncpy (C_String dst, C_String src, int n);
  extern C_String C_strcat (C_String dst, C_String src);
  extern C_String C_strncat (C_String dst, C_String src, int n);
  extern int C_strcmp (C_String str1, C_String str2);
  extern int C_strncmp (C_String str1, C_String str2, int n);
  extern C_String C_strchr (C_String str, int ch);
  extern C_String C_strrchr (C_String str, int ch);
  extern int C_strspn (C_String str, C_String ct);
  extern int C_strcspn (C_String str, C_String ct);
  extern C_String C_strpbrk (C_String str, C_String ct);
  extern C_String C_strstr (C_String str1, C_String str2);
  extern C_String C_strrstr (C_String str1, C_String str2);
  extern C_String C_strsave (C_String str);
  extern int C_str2char (C_String str);
  extern C_String C_savestr (C_String str);
  extern C_String C_findstr (C_String str);
  extern C_Pointer C_addptr (C_String str, C_Pointer ptr);
  extern C_Pointer C_findptr (C_String str);
  extern void C_for_all_ptr (int (*fn) (C_String name, C_Pointer ptr));
  extern void C_RemoveAllString ();     /* BCC - 8/22/96 */

/*
 *      NUMBER CONVERSION OPERATIONS
 */
  extern C_Float C_string_to_float (C_String str);
  extern C_String C_float_to_string (C_Float value, C_String line);
  extern C_Double C_string_to_double (C_String str);
  extern C_String C_double_to_string (C_Double value, C_String line);
  extern C_String C_char_to_string (int value, C_String line);
  extern C_Integer C_string_to_integer (C_String str);
  extern C_String C_integer_to_string (C_Integer value, C_String line);
  extern C_Integer C_string_to_hex (C_String str);
  extern C_String C_hex_to_string (C_Integer value, C_String line);
  extern C_Integer C_string_to_oct (C_String str);
  extern C_String C_oct_to_string (C_Integer value, C_String line);

/*
 *      Description     :       System diagnostic functions.
 */
  extern void C_abort (void);
  extern void C_exit (int status);
  extern void C_assert (int expression, char *error_mesg);

/*
 *      Description     :       Memory allocation.
 */
  extern C_Pointer C_calloc (int num_obj, int size_obj);
  extern C_Pointer C_malloc (int size);
  extern C_Pointer C_calloc2 (int num_obj, int size_obj);
  extern C_Pointer C_malloc2 (int size);
  extern void C_free (C_Pointer ptr);

#ifdef __cplusplus
}
#endif


/*
 *      Description     :       File operations.
 */
typedef int C_File;

#define C_STDIN         0       /* standard input */
#define C_STDOUT        1       /* standard output */
#define C_STDERR        2       /* standard error output */

/*
 *      QUERY OPERATIONS
 */
typedef struct
{
  short does_not_exist;         /* no such named file */
  short search_denied;          /* no permission to search */
  short is_regular_file;        /* is a regular file? */
  long last_access_time;        /* last use/modify time */
  long last_modify_time;        /* last modify time */
  long total_size;              /* total size in bytes */
  int owner_id;                 /* user ID of owner */
}
C_Fstat;

/*
 *      OPEN / CLOSE REGULAR FILE
 */
#define C_OPEN_READ_ONLY        0
#define C_OPEN_WRITE_ONLY       1
#define C_OPEN_APPEND_ONLY      2

#ifdef __cplusplus
extern "C"
{
#endif

  extern int C_lookup_file (char *fname, C_Fstat * st);
  extern C_File C_open_file (C_String file_name, int mode);
  extern int C_flush_file (C_File file);
  extern int C_close_file (C_File file);
  extern int C_create_tmp_file (C_String name);
  extern int C_remove_file (C_String name);
  extern int C_rename_file (C_String name, C_String new_name);

/*
 *      CREATE / DESTROY DIRECTORY
 */
  extern int C_create_directory (C_String dir_name);
  extern int C_destroy_directory (C_String dir_name);
  extern int C_create_tmp_directory (C_String dir_name);

/*
 *      DATABASE (organized directory)
 */
  extern int C_database_size (C_String db_name);
  extern int C_create_database (C_String db_name);
  extern int C_destroy_database (C_String db_name);
  extern int C_connect_database (C_String db_name);
  extern int C_disconnect_database (C_String db_name);
  extern C_File C_read_record (C_String record_name);
  extern C_File C_write_record (C_String record_name);
  extern int C_destroy_record (C_String record_name);

/*
 *      READING / WRITING FILE
 */
  extern C_String C_file_name (C_File file);
/* token oriented */
  extern int C_eof (C_File file);
  extern int C_read_char (C_File file, C_Integer * value);
  extern int C_read_word (C_File file, C_String line, int length);
  extern int C_read_integer (C_File file, C_Integer * value);
  extern int C_read_float (C_File file, C_Float * value);
  extern int C_read_double (C_File file, C_Double * value);
  extern int C_read_line (C_File file, C_String line, int length);
  extern int C_read_block (C_File file, C_String line, int length);
/* fixed length */
  extern int C_write_char (C_File file, C_Integer value);
  extern int C_write_word (C_File file, C_String line);
  extern int C_write_line (C_File file, C_String line);
  extern int C_write_block (C_File file, C_String line, int length);
  extern int C_write_integer (C_File file, C_Integer value);
  extern int C_write_float (C_File file, C_Float value);
  extern int C_write_double (C_File file, C_Double value);

/*
 *      Description     :       Time functions.
 */
  extern long C_time (void);
  extern char *C_asctime (time_t tm);

#ifdef __cplusplus
}
#endif

#endif
