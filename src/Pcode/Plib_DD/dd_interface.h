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


#ifndef __PLIB_DD_NEW_DD_INTERFACE_H
#define __PLIB_DD_NEW_DD_INTERFACE_H

#include <stdio.h>
#include <stdlib.h>
#include <Pcode/pcode.h>
#include <Pcode/extension.h>

extern SymbolTable DD_symbol_table; 

/* JWS - WARNING: P_DepType is used interchangeably with P_DDnature! */

typedef enum
{
  DT_NONE,
  DT_FLOW,
  DT_ANTI,
  DT_OUTPUT,
  DT_INPUT
}
P_DepType;

typedef enum
{
  DDIR_LT 	= 1,
  DDIR_EQ	= 2,
  DDIR_LE	= 3,
  DDIR_GT 	= 4,
  DDIR_NE	= 5,
  DDIR_GE	= 6,
  DDIR_ALL 	= 7
}
P_DepDir;

#define DEP_HEAD		'h'
#define DEP_TAIL		't'

/***************************************************************************************/

typedef struct _P_DepInfo
{
  int 		id;		/* dependence id */
  int 		expr;		/* the other end of the dependence */
  P_DepType 	dep_type;	/* type of the dependence */
  char		node_type;	/* 's': source, 't': target */	
  short 	depth;		/* depth of the dependence */
  short		*dir;		/* dependence direction */
  short		*dist; 		/* dependence distance */
  char 		*known;		/* dependence distance i known */
}
_P_DepInfo, *P_DepInfo;

#define Get_DepInfo_id(d)		((d)->id)
#define Set_DepInfo_id(d, i)		((d)->id = (i))
#define Get_DepInfo_expr(d)		((d)->expr)
#define Set_DepInfo_expr(d, e)		((d)->expr = (e))
#define Get_DepInfo_dep_type(d)		((d)->dep_type)
#define Set_DepInfo_dep_type(d, t)	((d)->dep_type = (t))
#define Get_DepInfo_node_type(d)	((d)->node_type)
#define Set_DepInfo_node_type(d, t)	((d)->node_type = node_type)
#define Get_DepInfo_depth(d)		((d)->depth)
#define Set_DepInfo_depth(d, m)		((d)->depth = (m))
#define Get_DepInfo_dir(d)		((d)->dir)
#define Set_DepInfo_dir(d, r)		((d)->dir = (r))
#define Get_DepInfo_dir_i(d, i)		((d)->dir[(i)])
#define Set_DepInfo_dir_i(d, i, r)	((d)->dir[(i)] = (r))
#define Get_DepInfo_dist(d)		((d)->dist)
#define Set_DepInfo_dist(d, s)		((d)->dist = (s))
#define Get_DepInfo_dist_i(d, i)	((d)->dist[(i)])
#define Set_DepInfo_dist_i(d, i, s)	((d)->dist[(i)] = (s))
#define Get_DepInfo_known(d)		((d)->known)
#define Set_DepInfo_known(d, u)		((d)->known = (u))	
#define Get_DepInfo_known_i(d, i)	((d)->known[(i)])
#define Set_DepInfo_known_i(d, i, u)	((d)->known[(i)] = (u))

extern P_DepInfo P_NewDepInfo (int id, int expr, P_DepType dep_type, char node_type, short depth);
extern P_DepInfo P_CopyDepInfo (P_DepInfo dep);
extern void P_FreeDepInfo (P_DepInfo dep);

extern void PrintFuncDepInfo (FILE *outf, FuncDcl func, int expr_ext_deplist_idx);

/***************************************************************************************/

typedef struct _P_DepList
{
  List deps; 
}
_P_DepList, *P_DepList;

#define Get_DepList_deps(d)		((d)->deps)
#define Set_DepList_deps(d, s)		((d)->deps = (s))

extern Extension P_DepList_alloc (void);
extern Extension P_DepList_free (Extension x);
extern Extension P_DepList_copy (Extension x);
extern char *P_DepList_write (char *sig, Extension x);
extern void P_DepList_read (Extension x, char *sig, char *raw);

/***************************************************************************************/

extern void DD_ReadParameter (Parm_Parse_Info * ppi);
extern void DD_SetUpExtension (void);
extern void DD_DependenceTest (FuncDcl func);

#endif /* __PLIB_DD_NEW_DD_INTERFACE_H */
