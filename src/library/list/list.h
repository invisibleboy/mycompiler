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
 *      File:   list.h
 *      Author: Pohua Chang
 *      Copyright (c) 1991 Pohua Chang, Wen-Mei Hwu. All rights reserved.  
 *      All rights granted to the University of Illinois.
 *      The University of Illinois software License Agreement
 *      specifies the terms and conditions for redistribution.
\*****************************************************************************/
#ifndef IMPACT_LIST_H
#define IMPACT_LIST_H

/* 10/29/02 REK Adding config.h */
#include <config.h>
#include <stdio.h>
#include <library/i_types.h>

/*--------------------------------------------------------------------------
 *-------------------------------------------------------------------------*/
/*
 *      Macro definitions to make interface woth lex less painful 
 */
#define TokenBuffer(n) (yytext+(n))     /* current token in text form */
#define TokenLength yyleng      /* length of yytext */
#define TokenChar(n) (yytext[n])        /* Nth character of yytext */

/* 
 *      Definitions for the input buffer.
 *      These information will be destroyed as soon as
 *      the next token comes in.
 */

#ifdef __cplusplus
extern "C"
{
#endif

  extern int TokenType;         /* token type */
  extern char *TokenName;       /* char/string/identifier */
  extern ITintmax TokenInteger; /* scalar */
  extern double TokenReal;      /* floating point scalar */
  /* extern int lex_error; */
  /* Make it compile with gcc4 -KF 10/2005 */

#ifdef __cplusplus
}
#endif

/* token type */
#define L_ERROR         260
#define L_ID            261
#define L_INT           262
#define L_REAL          263
#define L_STRING        264
#define L_CHAR          265

#ifdef __cplusplus
extern "C"
{
#endif

  /* extern FILE *SwitchFile (FILE * fpt); */ /* (file ptr) */
  /* Make it compile with gcc4 -KF 10/2005 */

#ifdef __cplusplus
}
#endif

/*--------------------------------------------------------------------------
 *-------------------------------------------------------------------------*/

typedef struct _node
{
  short type;                   /* token type */
  union
  {
    ITintmax scalar;            /* scalar value */
    double real;                /* floating point value */
    char *string;               /* string */
    struct _node *child;        /* child list */
  }
  value;
  struct _node *sibling;        /* sibling */
  /* BCC - used for long lists */
  struct _node *last_child;     /* last child */
}
NODE, *LIST;

/* node type */
#define T_EOF           -1      /* end of file */
#define T_NULL          0       /* undefined node */
#define T_LIST          1       /* list */
#define T_ID            2       /* identifier */
#define T_INT           3       /* scalar */
#define T_REAL          4       /* floating point number */
#define T_STRING        5       /* string literal */
#define T_CHAR          6       /* character literal */
#define T_OPER          7       /* special operators */

/* some useful macro */
#define NodeType(x)     x->type
#define IntegerOf(x)    (ITicast(x->value.scalar))
#define ScalarOf(x)     x->value.scalar
#define RealOf(x)       x->value.real
#define StringOf(x)     x->value.string
#define ChildOf(x)      x->value.child
#define SiblingOf(x)    x->sibling
#define LastChildOf(x)  x->last_child

#ifdef __cplusplus
extern "C"
{
#endif

#if 0
/* interface to lexer */
  extern int lexOpen (char *file_name, int inclusion);
  extern int lexClose (char *file_name);
#endif
int
old_lexOpen(char *file_name, int inclusion);
int
old_lexClose (char *file_name);

/* export functions */
  extern char *old_FindString (char *str);
  extern LIST DisposeNode (LIST node);
  extern LIST GetNode (void);
  extern void PrintNode (FILE * F, LIST node);
  extern void print_node (LIST node, int indent);

  extern LIST GetInteger (LIST list, int *val);
  extern LIST GetReal (LIST list, double *val);
  extern LIST GetString (LIST list, char **val);
  extern LIST GetId (LIST list, char **val);
  extern void RemoveAllString (void);   /* BCC - 8/22/96 */
  extern void FreeDeadList (void);      /* BCC - 8/25/96 */

#ifdef __cplusplus
}
#endif

#endif
