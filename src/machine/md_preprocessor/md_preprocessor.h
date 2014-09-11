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
 *      File:   md_preprocessor.h
 * 
 *      Description: Header file for the IMPACT Meta-Description Language
 *                   preprocessor
 * 
 *      Creation Date:  October 1994
 * 
 *      Authors: John C. Gyllenhaal and Wen-mei Hwu
 *
\*****************************************************************************/

/* 10/29/02 REK Adding config.h */
#include <config.h>
#include <stdio.h>
#include <stdlib.h>
#include <ctype.h>
#include <stdarg.h>
#include <string.h>
/*#include <strings.h>*/
#include <library/l_alloc_new.h>

#include <library/mbuf.h>
#include <library/mfile.h>
#include "psymbol.h"


typedef struct Pptr
{
  Mptr *mptr;
  Mbuf *expanded;
  int expanded_pos;
  int quoted;
  int scanned;
}
Pptr;

/* High level symbol table structures */
typedef struct Pdef
{
  char *name;			/* redundent but eases debugging */
  char *val;
  int allow_implicit_replacement;
  int level;			/* 0 environment, 1 in text, 2 command line */
}
Pdef;

/* Used to create list of strings */
typedef struct String_Node
{
  char *string;
  struct String_Node *next;
}
String_Node;

/* Used in $for handling to allow one or more variables to be
 * defined in parallel.
 */
typedef struct Value_List
{
  char *name;			/* Name of variable defined */
  int allow_implicit_replacement;	/* Set if ! before name */
  String_Node *first_value;	/* First in linked list of values */
  String_Node *last_value;	/* Last in linked list of values */
  int value_count;		/* Number of values in list */
  struct Value_List *next_list;	/* Next value list in $for() */
}
Value_List;

#ifdef __cplusplus
extern "C"
{
#endif

/* Processed character input prototypes (Pptr) */
  extern Pptr *create_Pptr (Mfile * mfile);
  extern Pptr *copy_Pptr (Pptr * orig_pptr);
  extern void move_Pptr (Pptr * old_pptr, Pptr * new_pptr);
  extern void free_Pptr (Pptr * pptr);

  extern int Pexpand_text (Pptr * pptr);
  extern int Pgetc (Pptr * pptr);
  extern int Ppeekc (Pptr * pptr);
  extern void Pungetc (Pptr * pptr, int ch);
  extern void Pungets (Pptr * pptr, char *string);

  extern void Pputc (Pptr * pptr, int ch);

  extern char *Pget_alnum_string (Pptr * pptr);
  extern char *Pget_identifier (Pptr * pptr);
  extern char *Pget_stripped_line (Pptr * pptr);
  extern char *Pget_bounded_string (Pptr * pptr);
  extern char *Pget_quoted_string (Pptr * pptr, int strip_quotes);
  extern char *Pget_for_string (Pptr * pptr);
  extern void Pskip_whitespace (Pptr * pptr);
  extern void Pskip_whitespace_no_nl (Pptr * pptr);
  extern void Perror (Pptr * pptr, char *fmt, ...);
  extern void L_punt (char *fmt, ...);

/* Recursive calculation routines */
  extern int Pcalc_C_int_expr (Pptr * pptr, int current_precedence);
  extern int Pcalc_C_int_factor (Pptr * pptr);

  extern double Pcalc_C_float_expr (Pptr * pptr, int current_precedence);
  extern double Pcalc_C_float_factor (Pptr * pptr);


/* High level symbol table prototypes */
  extern char *Plookup (char *name, int implicit_replacement);
  extern Pdef *create_Pdef (char *name, char *val,
			    int allow_implicit_replacement, int level);
  extern void free_Pdef (void *def_v);
  extern void add_Pdef (char *name, char *val, int allow_implicit_replacement,
			int level);
  extern void delete_Pdef (char *name);


/* Recursive preprocessor prototypes */
  extern Pptr *process_body (Pptr * pptr);
  extern Pptr *process_directive (Pptr * pptr);
  extern Pptr *process_def_directive (Pptr * pptr);
  extern Pptr *process_for_directive (Pptr * pptr, Pptr * placemark);


/* Parsing type prototypes */
  extern int Piscomment (char *token);

/* General Prototypes */
  extern void preprocess (Pptr * ptr);

#ifdef __cplusplus
}
#endif
