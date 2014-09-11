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
/*===========================================================================*\
 *
 *  File:  l_debug.h
 *
 *  Description:
 *    Header file for modules of propagating source information to assembly
 *    files
 *
 *  Creation Date :  September, 1995
 *
 *  Author:  Le-Chun Wu, Wen-mei Hwu
 *
 *
 *===========================================================================*/

#ifndef L_DEBUG_H
#define L_DEBUG_H

/* 10/29/02 REK Adding config.h */
#include <config.h>
#include <library/llist.h>

/* These defines must correspond to the TypeClassQual enum in Pcode/pcode.h */

/* L_DATA_* */

/* (qualifier) */
#define L_DATA_CONST        0x00000001   /* constant qualifier */
#define L_DATA_VOLATILE     0x00000002   /* volatile qualifier */
#define L_DATA_NOALIAS      0x00000004   /* no_alias qualifier */
/* (class) */
#define L_DATA_REGISTER     0x00000008   /* auto in register */
#define L_DATA_AUTO         0x00000010   /* auto */
#define L_DATA_STATIC       0x00000020   /* static */
#define L_DATA_EXTERN       0x00000040   /* extern */
#define L_DATA_GLOBAL       0x00000080   /* global variable */
#define L_DATA_PARAMETER    0x00000100   /* function parameter */
/* (type) */
#define L_DATA_VOID         0x00000200   /* void */
#define L_DATA_CHAR         0x00000400   /* char */
#define L_DATA_SHORT        0x00000800   /* short */
#define L_DATA_INT          0x00001000   /* int */
#define L_DATA_LONG         0x00002000   /* long */
#define L_DATA_LONGLONG     0x00004000   /* long long */

#define L_DATA_FLOAT        0x00008000   /* float */
#define L_DATA_DOUBLE       0x00010000   /* double */
#define L_DATA_LONGDOUBLE   0x00020000   /* long double */

#define L_DATA_SIGNED       0x00040000   /* signed */
#define L_DATA_UNSIGNED     0x00080000   /* unsigned */

#define L_DATA_STRUCT       0x00100000   /* struct */
#define L_DATA_UNION        0x00200000   /* union */
#define L_DATA_ENUM         0x00400000   /* enum */
#define L_DATA_VARARG       0x00800000   /* vararg */
#define L_DATA_BIT_FIELD    0x01000000   /* bit field */

#define L_DATA_TYPEDEF      0x02000000   /* typedef */

typedef struct L_Type {
  int type;                     /* type bit_vector */
  char *struct_name;            /* structure name 
                                   (struct/union/enum) */
  struct L_Dcltr *dcltr;        /* access pattern */
} L_Type;

#define L_D_ARRY  1             /* array access */
#define L_D_PTR   2             /* pointer access */
#define L_D_FUNC  3             /* function call */

typedef struct L_Dcltr {
  int method;                   /* accessing method */
  struct L_Expr *index;         /* array index */
  struct L_Dcltr *next;         /* next accessing declarator */
} L_Dcltr;

typedef struct L_Struct_Dcl {
  char *name;                   /* identification */
  struct L_Field *fields;       /* field definition */
  void *ext;                    /* extension field */
} L_Struct_Dcl;

typedef struct L_Union_Dcl {
  char *name;                   /* identification */
  struct L_Field *fields;       /* field definition */
  void *ext;                    /* extension field */
} L_Union_Dcl;

typedef struct L_Field {
  char *name;                   /* name of the field */
  struct L_Type *type;          /* field type */
  struct L_Expr *bit_field;     /* number of bits */
  struct L_Field *next;         /* next field */
  void *ext;                    /* extension field */
} L_Field;

typedef struct L_Enum_Dcl {
  char *name;                   /* identification */
  struct L_Enum_Field *fields;  /* fields */
} L_Enum_Dcl;

typedef struct L_Enum_Field {
  char *name;                   /* name of field */
  struct L_Expr *value;         /* constant value */
  struct L_Enum_Field *next;    /* next enum field */
} L_Enum_Field;


#define L_ST_STRUCT       1     /* struct tag table */
#define L_ST_UNION        2     /* union tag table */
#define L_ST_ENUM         3     /* enum tag table */

/* local variable information */
typedef struct L_Local_Var {
  char *name;                   /* local variable name */
  L_Type *data_type;            /* variable type information */
  char loc_type;                /* 'r': register, 'm': memory */
  int loc;                      /* register number or stack offset */
  int scope;                    /* variable scope information */
} L_Local_Var;

extern struct _Lptr *lvar_lst;

#endif
