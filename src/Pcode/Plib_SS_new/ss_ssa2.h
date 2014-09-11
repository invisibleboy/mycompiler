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
#ifndef _PSS_H
#define _PSS_H

#include <Pcode/pcode.h>
#include <Pcode/cfg.h>
#include <Pcode/loop.h>
#include <library/stack.h>
#include <library/set.h>

typedef enum
{
  NORMAL    = 0x00000001,

  /* Merge definitions */
  PHI       = 0x00000002,
  MU        = 0x00000004, /*!< Merge phi node at the top of a loop */

  /* Def types with no explicit definition */
  PARAM     = 0x00000008, /*!< This Def type implies the variable is
			   * initialized outside the function as a
			   * parameter. */
  
  UNDEF     = 0x00000010, /*!< Uses being linked to a Def of this type imply 
			   * the var is used before it is defined. */
  
  UNUSED    = 0x00000020, /*!< This type designates that the definition is not
			   * present in the code.  It is the initial type of
			   * the bottom entry of the Def stack unless it is
			   * referenced by a parameter use or and UNDEF
			   * use. */

  /* Tells Pipa which definitions are internal temporaries. */
  PIPA_TEMP = 0x00000040, /*!< A definition with this bit active is a merge
			   * node and nothing more than an internal
			   * temporary. */
  
  /* Valid combinations when the PIPA_TEMP bit is set.  Only defined for
   * gdb debugging considerations. */
  PIPA_TEMP_PHI    = 0x00000042,
  PIPA_TEMP_MU     = 0x00000044,
  PIPA_TEMP_UNDEF  = 0x00000050,
  PIPA_TEMP_UNUSED = 0x00000060,
}
PSS_DefType;

/*! Merge type definitions */
#define MERGE (PHI | MU)

/*! Def types with no explicit definition */
#define UNINITIALIZED (PARAM | UNDEF | UNUSED)

/* Type Field query macros. */
#define NORMAL_TYPE(type)		((type) & NORMAL)
#define PHI_TYPE(type)			((type) & PHI)
#define MU_TYPE(type)			((type) & MU)
#define PARAM_TYPE(type)		((type) & PARAM)
#define UNDEF_TYPE(type)		((type) & UNDEF)
#define UNUSED_TYPE(type)		((type) & UNUSED)
#define MERGE_TYPE(type)		((type) & MERGE)
#define UNINITIALIZED_TYPE(type)	((type) & UNINITIALIZED)
#define PIPA_TEMP_TYPE(type)		((type) & PIPA_TEMP)

/*! Sets the type field.  Only allows one type bit to be high at a time along
 * with potentially the PIPA_TEMP bit. */
#define SET_DEF_TYPE(a, type) ((a) = (type==PIPA_TEMP) ? ((a)|PIPA_TEMP) : \
			             ((a)&PIPA_TEMP) | (type) )

typedef struct _PSS_Use
{
  Expr var;
  PC_Block bb;

  struct _PSS_Use *next;

  void *ext;
}
_PSS_Use, *PSS_Use;


typedef struct _PSS_Def
{
  int subscr;         /*!< SSA subscript */
  int name;           /*!< Similar to the subscript, but may represent SCC
                           or lifetime information. */

  Expr var;           /*!< variable being defined (OP_var) */
  PC_Block bb;        /*!< BB in which the def occurs */
  PSS_DefType type;   /*!< Type of definition ex. (NORMAL, PHI) */

  PSS_Use uses;       /*!< List of uses of this variable */

  struct _PSS_Def *next;
  struct _PSS_Def *prev;

  void *ext;
}
_PSS_Def, *PSS_Def;


typedef struct _PSS_Base
{
  VarDcl vdcl;

  int addr_taken;
  int assign_count;

  int def_count;      /*!< The number of defs uncovered for this variable */
  PSS_Def defs;       /*!< All definitions in the cfg */
  Set def_bbs;	      /*!< Set of bb IDs that modify this var */
  Stack *def_var_stk; /*!< Stack of PSS_Def structures */
  
  struct _PSS_Base *next;

  void *ext;
}
_PSS_Base, *PSS_Base;


typedef struct _PSS_BaseTbl
{
  PSS_Base first;
  PSS_Base last;

  int num_valid;
}
_PSS_BaseTbl, *PSS_BaseTbl;


#define UNDEF_SUBSCR -1

/****************************************************************************
        Export function header
****************************************************************************/
extern PSS_BaseTbl PSS_ComputeSSA (PC_Graph cfg);
extern PSS_BaseTbl PSS_DeleteSSA (PC_Graph cfg, PSS_BaseTbl baseTbl);
extern void PSS_NormalizeSubscrs (PSS_BaseTbl table, int first_subscr);

extern List PSS_GetSubExprByOpcode_List (Expr expr, _Opcode opcode);
extern int PSS_VarIsUse (Expr v);
extern int PSS_Is_Loop_Invariant (Expr expr, PC_Loop loop);

/* Struct management functions */
extern PSS_BaseTbl PSS_BaseTbl_Insert (PSS_BaseTbl table, 
					   PSS_Base entry);
extern PSS_BaseTbl PSS_BaseTbl_Free (PSS_BaseTbl table);

extern PSS_Base PSS_Base_New (VarDcl vdcl, PSS_Def def);
extern PSS_Base PSS_Base_Free (PSS_Base base);

extern PSS_Use PSS_Use_New (Expr var, PC_Block bb);
extern PSS_Use PSS_Use_Free (PSS_Use u);

extern PSS_Def PSS_Def_New (Expr var, PC_Block bb, PSS_DefType type);
extern PSS_Def PSS_Def_Free (PSS_Def d);

extern void PSS_AddUseToDef (Expr var, PC_Block bb);
extern void PSS_RemoveUseFromDef (Expr var);

extern void PSS_AddDefToBase (PSS_Def def, PSS_Base base);
extern void PSS_RemoveDefFromBase (PSS_Def rm_def, PSS_Base base);


/* Printing/debuging functions */
extern void P_PrintPcodeSSAGraph_stdout (PC_Graph cfg, int file, int sym);
extern void P_PrintPcodeSSAGraph_file (PC_Graph cfg, int file, int sym);
extern void P_PrintPcodeSSAGraph (FILE *fp, PC_Graph cfg, VarDcl vdcl);
extern void P_PrintPcodeLifetimeGraph (PC_Graph cfg, VarDcl vdcl);
extern void P_PrintPcodeSSAVars (PC_Graph cfg, PSS_BaseTbl varTbl);
extern void P_PrintUseDefChain (PC_Graph cfg, Expr expr);


/* Extension fields: ss_ext.c */
extern int PS_var_ext;
extern void PS_def_handlers (void);

#define PS_GetSSABaseEntry(v) \
   (((PSS_Base)P_GetVarDclExtL ((v), PS_var_ext)))
#define PS_SetSSABaseEntry(v,x) \
   (((PSS_Base)P_SetVarDclExtL ((v), PS_var_ext, (x))))


#endif
