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
 *      File:   c_list.h
 *      Author: Po-hua Chang, Wen-mei Hwu
 *      Creation Date:  June 1990
 *      Modified By: XXX, date, time, why
\*****************************************************************************/
/*==========================================================================
 *      Description :   LIST interface.
 *==========================================================================*/
#ifndef C_LIST_H
#define C_LIST_H
/* 10/29/02 REK Adding config.h */
#include <config.h>
#include <library/c_basic.h>
#include <library/c_symbol.h>

#define Cl_EOF          -1      /* end of file */
#define Cl_NULL         0       /* undefined node */
#define Cl_LIST         1       /* list */
#define Cl_ID           2       /* identifier */
#define Cl_INT          3       /* scalar */
#define Cl_REAL         4       /* floating point number */
#define Cl_STRING       5       /* string literal */
#define Cl_CHAR         6       /* character literal */
#define Cl_OPER         7       /* special operators */
#define Cl_MACRO        8       /* # */
#define Cl_EOLN         9       /* \n (only after #) */
#define Cl_STD_GROUP    10      /* Stardard groups (src and dest) */
#define Cl_OPT_GROUP    11      /* Optional groups (flags, pred, attr) */
#define Cl_OPT_SYNC     12      /* Optional syncs */

typedef struct _Cl_List
{
  char *file;                   /* file name */
  int line;                     /* line number */
  int column;                   /* column position */
  short type;                   /* token type */
  C_String string;              /* token string */
  union
  {
    C_Integer scalar;           /* scalar value */
    C_Double real;              /* floating point value */
    struct _Cl_List *child;     /* child list */
  }
  value;
  struct _Cl_List *sibling;     /* sibling */
}
_Cl_List, *Cl_List;

/* some useful macro */
#define Cl_NodeType(x)  x->type
#define Cl_StringOf(x)  x->string
#define Cl_IntegerOf(x) x->value.scalar /* Cl_INT, Cl_OPER */
#define Cl_RealOf(x)    x->value.real   /* Cl_REAL */
#define Cl_ChildOf(x)   x->value.child  /* Cl_LIST */
#define Cl_SiblingOf(x) x->sibling

#define Cl_File(x)      x->file
#define Cl_Line(x)      x->line
#define Cl_Column(x)    x->column

#define Cl_MAX_STRING_TABLE_SIZE        5121

#ifdef __cplusplus
extern "C"
{
#endif

/* 
 *      Interface to C lexer.
 */
  extern int Cl_open (char *file_name, int inclusion);  /* (file, inclusion) */
  extern int Cl_close (char *file_name);        /* (file) */
  extern int C_name_tbl;
  extern int Cl_seek (int);     /* (offset) */
  extern int Cl_tell (int offset);

/*
 *      LIST interface.
 *      no Cl_MACRO, Cl_EOLN
 */
  extern Cl_List Cl_get (void); /* read in a node */
  extern Cl_List Cl_integer (Cl_List list, C_Integer * val); /* (list, &int) */
  extern Cl_List Cl_real (Cl_List list, C_Double * val);  /* (list, &double) */
  extern Cl_List Cl_string (Cl_List list, char **val);  /* (list, &ptr) */
  extern Cl_List Cl_char (Cl_List list, char **val);    /* (list, &ptr) */
  extern Cl_List Cl_identifier (Cl_List list, char **val);   /* (list, &ptr) */

/*
 *      C interface.
 *      no Cl_LIST
 */
  extern Cl_List Cl_get_C_token (void); /* read in a node */

/*
 *      Other functions.
 */
  extern Cl_List Cl_dispose (Cl_List node);     /* dispose a node */
  extern void Cl_print (FILE * F, Cl_List node);        /* print a node */

#ifdef __cplusplus
}
#endif

#endif
