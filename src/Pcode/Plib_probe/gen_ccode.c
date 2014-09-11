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
 *	File:	gen_ccode.c
 *	Author: David August, Nancy Warter and Wen-mei Hwu
 *	Code Modified from code written by Po-hua Chang
 *	Replaced with C code generation by Chien-wei Li
 \****************************************************************************/

#include <config.h>
#include <Pcode/impact_global.h>
#include <Pcode/struct.h>
#include <Pcode/parms.h>
#include <library/c_basic.h>
#include <library/l_alloc_new.h>
#include <library/i_list.h>
#include <string.h>
#include <Pcode/pcode.h>
#include <Pcode/struct.h>
#include <Pcode/symtab.h>
#include <Pcode/symtab_i.h>
#include <Pcode/struct_symtab.h>
#include <Pcode/query.h>
#include <Pcode/cast.h>
#include <Pcode/write.h>
#include <math.h>
#include <machine/m_spec.h>
/* For useless var */
#include <Pcode/flatten.h>
#include <Pcode/probe.h>
#include <Pcode/cfg.h>
#include <Pcode/loop.h>

#include "gen_ccode.h"		/* CWL */

extern int total_lps;

/* C output file -- set by all exported functions */

static FILE *Fcout = NULL;

/* DIA: 1/11/93 Create global variable for GenStmts. */

#if !NEW_PROBING
static int Expr_Is_Explicit = TRUE;
#endif

/* LCW - current profiling execution count */
static double current_prof_count;


/* BCC - flag about if the current token is inside a function - 4/1/96 */
static int INSIDE_FUNCTION;

#define PP_MANGLE_NAMES 0

/*********************************************/

typedef enum
{
  DclTok_STR,			/* () or identifier */
  DclTok_QUALIFIER,		/* volatile, constant, ... */
  DclTok_ABS_PARAM,		/* abstract parameter list */
  DclTok_FORMAL_PARAM,		/* formal parameter list */
  DclTok_EXPR,			/* expression */
}
_P2C_DcltrToken;

/* for the preprocessing of Declarator */

typedef struct _P2C_DcltrNode
{
  _P2C_DcltrToken token;
  union
  {
    char *str;
    _TypeQual qualifier;
    Param abs_param;
    VarList formal_param;
    Expr expr;
  }
  value;
  struct _P2C_DcltrNode *next;
}
_P2C_DcltrNode, *P2C_DcltrNode;

/*
   priority table of C operators 
*/
static int precedence[] = {
  -1,				/* --- no 0 --- */
  15,				/* OP_var           =1, */
  15,				/* OP_enum          =2, */
  15,				/* OP_int           =3, */
  15,				/* OP_real          =4, */
  15,				/* OP_error         =5, */
  15,				/* OP_char          =6, */
  15,				/* OP_string        =7, */
  14,				/* OP_dot           =8, */
  14,				/* OP_arrow         =9, */
  13,				/* OP_cast          =10, */
  13,				/* OP_expr_size     =11, */
  13,				/* OP_type_size     =12, */
  2,				/* OP_quest         =13, */
  3,				/* OP_disj          =14, */
  4,				/* OP_conj          =15, */
  0,				/* OP_compexpr      =16, */
  1,				/* OP_assign        =17, */
  5,				/* OP_or            =18, */
  6,				/* OP_xor           =19, */
  7,				/* OP_and           =20, */
  8,				/* OP_eq            =21, */
  8,				/* OP_ne            =22, */
  9,				/* OP_lt            =23, */
  9,				/* OP_le            =24, */
  9,				/* OP_ge            =25, */
  9,				/* OP_gt            =26, */
  10,				/* OP_rshft         =27, */
  10,				/* OP_lshft         =28, */
  11,				/* OP_add           =29, */
  11,				/* OP_sub           =30, */
  12,				/* OP_mul           =31, */
  12,				/* OP_div           =32, */
  12,				/* OP_mod           =33, */
  13,				/* OP_neg           =34, */
  13,				/* OP_not           =35, */
  13,				/* OP_inv           =36, */
  -1,				/* --- no 37 --- */
  13,				/* OP_preinc        =38, */
  13,				/* OP_predec        =39, */
  13,				/* OP_postinc       =40, */
  13,				/* OP_postdec       =41, */
  1,				/* OP_Aadd          =42, */
  1,				/* OP_Asub          =43, */
  1,				/* OP_Amul          =44, */
  1,				/* OP_Adiv          =45, */
  1,				/* OP_Amod          =46, */
  1,				/* OP_Arshft        =47, */
  1,				/* OP_Alshft        =48, */
  1,				/* OP_Aand          =49, */
  1,				/* OP_Aor           =50, */
  1,				/* OP_Axor          =51, */
  13,				/* OP_indr          =52, */
  13,				/* OP_addr          =53, */
  14,				/* OP_index         =54, */
  14,				/* OP_call          =55, */
  15,				/* OP_float         =56, */
  15				/* OP_double        =57, */
};

/*****************************************************

   static function header

******************************************************/

static P2C_DcltrNode new_P2C_DcltrNode (_P2C_DcltrToken token,
					P2C_DcltrNode next);


static int GenRemainingLocalDataDcl (Stmt st);
static int GenAsmStmt (AsmStmt asmstmt, int level);

static int GenParam (Param param_head);
static int GenIndent (int level);
static int GenInit (Init init);
static int GenTypeQualifier (_TypeQual type, int last_print);
static int GenStorageClassSpecifier (_VarQual vq, int last_print);
static int GenTypeSpecifier (Key type, int last_print, int style);
static int GenType (Key type, int last_print, int style);
static int GenAbstractType (Key type, int style);
static int GenLabels (Label labels, int level);
static int GenCompound (Stmt stmt, Compound compound, int level);
static int GenIfStmt (IfStmt ifstmt, int level);
static int GenSwitchStmt (SwitchStmt switchstmt, int level);
static int GenWhileLoop (SerLoop serloop, int level);
static int GenForLoop (SerLoop serloop, int level);
static int GenDoLoop (SerLoop serloop, int level);
static int GenSerLoop (SerLoop serloop, int level);
static int GenStmt (Stmt stmt, int level);
static int GenFields (Field fields, int level);
static int GenStructDcl (StructDcl st, int level);
static int GenUnionDcl (UnionDcl un, int level);
static int GenFuncDcl (FuncDcl func, int insert_probes);
static int GenDcltr (Key type, char *name, int printing_func,
		     VarList formal_param, int declare_var, int k_and_r_args);
static int GenVarDcl (VarDcl var);
static int GenVar (VarDcl var, int level);
static int GenVarList (VarList var_list, int level);
static void GenOpcode (int opcode);
static int GenProbedExpr (Expr expr);

/*****************************************************/

/* forward decls */

void GenParamDataDcl ();
/* static GenLocalDataDcl(); */
static int PP_get_loop_number (Stmt st);	/* LCW - 3/24/99 */

/* NJW - notes */
/* 9/10 - Don't have to worry about include files, run through cpp first 
 * 	- Do we want to pass line positions through hcode?? if line_yes x
 *	- Union, Struct, Enum, Var - all moved out - already renamed.
 *	- Gen_HCODE_Struct, Gen_HCODE_Enum, Gen_HCODE_Union x
 *	- Gen_HCODE_LinePos, Gen_HCODE_Var x
 *	- GenStructDcl, GenUnionDcl, GenEnumDcl, GenGlobalDataDcl x
 *	- GenStructField x
 *	- GenType, GenDcltr, GenInit x
 *	- Need to add Global and Parameter types - based on GenGlobalDataDcl
 *	  etc?? x
 * 9/11	- Need to convert the following:
 *	  GenOpcode [x], GenProbedExprPragma [x], GenProbedExpr [x], GenBasicBlock [ ], 
 *	  GenFunc [x], GenParamDataDcl [x], GenLocalDataDcl [x]
 *	- Add GenBBPragma[x], GenFnPragma[x]
 *	- Gen_HCODE_Func [x]
 *	- Gen_HCODE_Init (used for debugging purposes) [x]
 *	- Add sync opcode to hcode.  use expression pragmas to indicate 
 *	  sync type (e.g. advance, await, mutex_begin, mutex_end) and sync
 *	  operands (e.g. marker for advance, marker and distance for await,
 *	  sync_var for mutex_begin and end)
 *	- Need GenBasicBlockPragma's to pass doall, doacross, doserial,
 *	  dosuper.
 *	- Need to rename local variables with scoping (or something) because
 *	  Hcode only uses one symbol table for all variables. (done in pcode.c)
 * 9/12 - Don't think location allowed anywhere but before function in hcode?
 *	- GenStmts[ ]
 * 9/13 - Have to make sure that have a goto at the end of each basic block
 *	  except when it ends with: return, if, or switch.
 * 9/14 - finish case and default - check genstmts.
 *
 * 10/92 - add a clear string symbol table so that labels in different functions
 *         are unique (This is the only use of the string symbol table that I
 *         know of - if it is ever used for anything else, a new table should
 *         be defined - since this is cleared after every function is processed)
 */
/* have to handle special cases: comma, enum/var, sync 
 *   comma:  when process an expression list (expr->next), deternmine how many 
 *	     expressions are in the list and add that many "(comma ". after 
 *	     writing first two expressions from the list, print ")" and after 
 *	     that put ")" after each expression. 
 *	pp2p output looks like:
 *	  ((ASSIGN (VAR i) (INT 0)) (ASSIGN (VAR x) (INT 5)))
 *	Cugc output looks like:
 *	  (comma (assign (var i___1) (signed 0)) (assign (var x__1) (signed 5)))
 *	
 *   enum/var:  if expr->enum_flag then print "enum" else "var"
 *
 *   sync:  for advance, await, mutex (and the end of a mutex) need to
 *	    print out "sync" followed by an expression pragma.
 *	    The pragmas for sync:  (must be string)
 *		"ADVANCE___marker"
 *		"AWAIT___marker___distance"
 *		"MUTEXBEGIN___syncvar"
 *		"MUTEXEND___syncvar"
 * The bb pragmas for DOALL, DOACROSS, DOSERIAL, DOSUPER:   (must be a string)
 *    	"DOALL"
 *    	"DOACROSS"
 *    	"DOSERIAL"
 *    	"DOSUPER"
 */
/*========================================================================*/

static void
GenOpcode (int opcode)
{
  switch (opcode)
    {
    case OP_disj:
      fprintf (Fcout, " || ");
      break;
    case OP_conj:
      fprintf (Fcout, " && ");
      break;
    case OP_assign:
      fprintf (Fcout, " = ");
      break;
    case OP_or:
      fprintf (Fcout, " | ");
      break;
    case OP_xor:
      fprintf (Fcout, " ^ ");
      break;
    case OP_and:
      fprintf (Fcout, " & ");
      break;
    case OP_eq:
      fprintf (Fcout, " == ");
      break;
    case OP_ne:
      fprintf (Fcout, " != ");
      break;
    case OP_lt:
      fprintf (Fcout, " < ");
      break;
    case OP_le:
      fprintf (Fcout, " <= ");
      break;
    case OP_ge:
      fprintf (Fcout, " >= ");
      break;
    case OP_gt:
      fprintf (Fcout, " > ");
      break;
    case OP_rshft:
      fprintf (Fcout, " >> ");
      break;
    case OP_lshft:
      fprintf (Fcout, " << ");
      break;
    case OP_add:
      fprintf (Fcout, " + ");
      break;
    case OP_sub:
      fprintf (Fcout, " - ");
      break;
    case OP_mul:
      fprintf (Fcout, " * ");
      break;
    case OP_div:
      fprintf (Fcout, " / ");
      break;
    case OP_mod:
      fprintf (Fcout, " %% ");
      break;
    case OP_neg:
      fprintf (Fcout, " - ");
      break;
    case OP_not:
      fprintf (Fcout, " ! ");
      break;
    case OP_inv:
      fprintf (Fcout, " ~ ");
      break;
      /*
         case OP_abs :              fprintf(Fcout, "abs ");     break;
       */
    case OP_preinc:
      fprintf (Fcout, " ++ ");
      break;
    case OP_predec:
      fprintf (Fcout, " -- ");
      break;
    case OP_postinc:
      fprintf (Fcout, " ++ ");
      break;
    case OP_postdec:
      fprintf (Fcout, " -- ");
      break;
    case OP_Aadd:
      fprintf (Fcout, " += ");
      break;
    case OP_Asub:
      fprintf (Fcout, " -= ");
      break;
    case OP_Amul:
      fprintf (Fcout, " *= ");
      break;
    case OP_Adiv:
      fprintf (Fcout, " /= ");
      break;
    case OP_Amod:
      fprintf (Fcout, " %%= ");
      break;
    case OP_Arshft:
      fprintf (Fcout, " >>= ");
      break;
    case OP_Alshft:
      fprintf (Fcout, " <<= ");
      break;
    case OP_Aand:
      fprintf (Fcout, " &= ");
      break;
    case OP_Aor:
      fprintf (Fcout, " |= ");
      break;
    case OP_Axor:
      fprintf (Fcout, " ^= ");
      break;
    case OP_indr:
      fprintf (Fcout, " * ");
      break;
    case OP_addr:
      fprintf (Fcout, " & ");
      break;
    case OP_dot:
      fprintf (Fcout, ".");
      break;
    case OP_arrow:
      fprintf (Fcout, "->");
      break;

    case OP_compexpr:
    case OP_var:

    case OP_int:
    case OP_real:
      /* BCC - added - 8/4/96 */
    case OP_float:
    case OP_double:
    case OP_char:
    case OP_string:
    case OP_cast:
    case OP_expr_size:
    case OP_type_size:
    case OP_error:

      I_punt ("gen_ccode: shouldn't generate this opcode");
    default:
      I_punt ("GenOpcode : illegal opcode");
    }
}

/* generate expression. */
/* have to handle special cases: comma, enum/var, sync 
 * handle enum/var here.  handle comma and sync in GenBasicBlock
 */
/* OP_int -> signed, OP_real-> float */
static int
GenProbedExpr (Expr expr)
{
  int opcode;
  Expr arg;
  double old_count = 0.0;	/* LCW */
  int rel_opcode;		/* LCW */

#if !NEW_PROBING
  int comma_inserted = 0;	/* LCW - 3/24/99 */
#endif
  Expr opnd;

  if (!expr)
    return 0;

  /* LCW - put current profiling weight to the expression - 10/30/95 */
  if (PP_annotate)
    {
      expr->profile = P_NewProfEXPR ();
      expr->profile->count = (double) current_prof_count;
    }

  /*
   *    print expressions.
   */
  opcode = expr->opcode;
  switch (opcode)
    {
    case OP_var:
      if (!strcmp (expr->value.var.name, "__PRETTY_FUNCTION__"))
	fprintf (Fcout, "__IMPACT_PRETTY_FUNCTION__");
      else if (!strcmp (expr->value.var.name, "__FUNCTION__"))
	fprintf (Fcout, "__IMPACT_FUNCTION__");
      else
	{
	  SymTabEntry ste = PSI_GetSymTabEntry (expr->value.var.key);
	  if (ste->type == ET_VAR_LOCAL &&
	      strncmp (expr->value.var.name, "__builtin_", 10))
	    fprintf (Fcout, "_PP_auto_%s_%d_%d", expr->value.var.name,
		     expr->value.var.key.file, expr->value.var.key.sym);
#if PP_MANGLE_NAMES
	  else if (ste->type == ET_VAR_GLOBAL &&
		   (ste->entry.var_dcl->qualifier & VQ_STATIC))
	    fprintf (Fcout, "_PP_static_%s_%d_%d", expr->value.var.name,
		     expr->value.var.key.file, expr->value.var.key.sym);
	  else if (ste->type == ET_FUNC &&
		   (ste->entry.func_dcl->qualifier & VQ_STATIC))
	    fprintf (Fcout, "_PP_static_%s_%d_%d", expr->value.var.name,
		     expr->value.var.key.file, expr->value.var.key.sym);
#endif
	  else
	    fprintf (Fcout, "%s", expr->value.var.name);
	}
      break;

    case OP_int:
      {
	TypeDcl t = PSI_GetTypeDclEntry (PSI_ReduceTypedefs (expr->type));
	_BasicType bt = P_GetTypeDclBasicType (t);

	/* BCC - need to represent (unsigned int) -1 - 10/31/96 */
	if (bt & BT_UNSIGNED)
	  {
	    if (bt & BT_LONGLONG)
	      fprintf (Fcout, "(" ITuintmaxcast ITuintmaxformat
		       ITuintmaxsuffix ")", expr->value.uscalar);
	    else
	      fprintf (Fcout, "((unsigned int) " ITuintmaxformat "U)",
		       expr->value.uscalar);
	  }
	else
	  {			/* SINGED */
	    if (bt & BT_LONGLONG)
	      fprintf (Fcout, "(" ITintmaxcast ITintmaxformat ")",
		       expr->value.scalar);
	    else if (expr->value.scalar == ((-2147483647) - 1))
	      fprintf (Fcout, "((-2147483647)-1)");
	    else
	      fprintf (Fcout, ITintmaxformat, expr->value.scalar);
	    /* fprintf(Fcout, "%d", expr->value.scalar); */
	  }
      }
      break;

    case OP_char:
      {
	TypeDcl t = PSI_GetTypeDclEntry (PSI_ReduceTypedefs (expr->type));
	_BasicType bt = P_GetTypeDclBasicType (t);
	/* BCC - we need to represent (unsigned char) - 10/31/96 */
	if (bt & BT_UNSIGNED)
	  fprintf (Fcout, "(unsigned char) %s", expr->value.string);
	/* CWL - 01/07/01 */
	else
	  fprintf (Fcout, "%s", expr->value.string);
	break;
      }

    case OP_float:
      /* FLOAT: Use eight decimal digits.
       * Insert portable workarounds for "special" values.
       */
      {
	float val = (float) expr->value.real;

	if (isnan (val))
	  fprintf (Fcout, "(float) (0.0/0.0)");
	else if (isinf (val) > 0)
	  fprintf (Fcout, "(float) (1.0/0.0)");
	else if (isinf (val) < 0)
	  fprintf (Fcout, "(float) (-1.0/0.0)");
	else
	  fprintf (Fcout, "(float) %1.8e", val);
      }
      break;

    case OP_double:
    case OP_real:
      /* DOUBLE: Use sixteen decimal digits.
       * Insert portable workarounds for "special" values.
       */
      {
	double val = (double) expr->value.real;

	if (isnan (val))
	  fprintf (Fcout, "(double) (0.0/0.0)");
	else if (isinf (val) > 0)
	  fprintf (Fcout, "(double) (1.0/0.0)");
	else if (isinf (val) < 0)
	  fprintf (Fcout, "(double) (-1.0/0.0)");
	else
	  fprintf (Fcout, "(double) %1.16e", val);
      }
      break;

    case OP_string:
      fprintf (Fcout, "\"%s\"", expr->value.string);
      break;

    case OP_cast:
      fputc ('(', Fcout);
      GenAbstractType (expr->type, 0);
      fprintf (Fcout, ") ");
      opnd = expr->operands;
      if (precedence[opnd->opcode] <= precedence[OP_cast])
	{
	  fputc ('(', Fcout);
	  GenProbedExpr (opnd);
	  fputc (')', Fcout);
	}
      else
	GenProbedExpr (opnd);
      break;

    case OP_type_size:
      fprintf (Fcout, "sizeof(");
      GenAbstractType (expr->value.type, P2C_PRINT_LONGLONG);
      fputc (')', Fcout);
      break;

    case OP_expr_size:

      if (!(arg = expr->operands))
	I_punt ("GenProbedExpr : sizeof missing argument");

      fprintf (Fcout, "sizeof(");
      GenProbedExpr (arg);
      fputc (')', Fcout);

      break;

    case OP_quest:
      {
	Expr opnd;		/* CWL - 01/07/01 */

	opnd = expr->operands;
	if (!opnd || (precedence[opnd->opcode] <= precedence[OP_quest]))
	  {
	    fputc ('(', Fcout);
	    GenProbedExpr (opnd);
	    fputc (')', Fcout);
	  }
	else
	  {
	    GenProbedExpr (opnd);
	  }

	fprintf (Fcout, " ? (");

	opnd = expr->operands->sibling;

	if (INSIDE_FUNCTION && (PP_probe || PP_annotate))
	  {
	    PP_C_insert_middle_probe (Fcout, next_probe++);
	    if (opnd)
	      {
		fprintf (Fcout, ", (");
		GenProbedExpr (opnd);
		fputc (')', Fcout);
	      }

	    if (PP_annotate)
	      {
		old_count = current_prof_count;
		/* LCW - modified to read floating-point profile weights and to
		   handle the case when EOF is encountered - 3/5/97 */
		if (fscanf (Fprofile, "%lf", &current_prof_count) == EOF)
		  current_prof_count = 0.0;
	      }
	    current_prof_count = old_count;
	  }
	else
	  {
	    GenProbedExpr (opnd);
	  }

	fprintf (Fcout, ") : (");
	opnd = expr->operands->sibling->sibling;

	if (INSIDE_FUNCTION && (PP_probe || PP_annotate))
	  {
	    PP_C_insert_middle_probe (Fcout, next_probe++);
	    if (opnd)
	      {
		fprintf (Fcout, ", (");
		GenProbedExpr (opnd);
		fputc (')', Fcout);
	      }

	    if (PP_annotate)
	      {
		old_count = current_prof_count;
		/* LCW - modified to read floating-point profile weights and to
		   handle the case when EOF is encountered - 3/5/97 */
		if (fscanf (Fprofile, "%lf", &current_prof_count) == EOF)
		  current_prof_count = 0.0;
	      }

	    current_prof_count = old_count;
	  }
	else
	  {
	    GenProbedExpr (opnd);
	  }

	fputc (')', Fcout);

	break;
      }

    case OP_call:
      num_total_func_call++;

      /* LCW - check if this is an exit call. If it is, check if it is in a 
         loop, if it is, insert a function call to update the loop iter 
         counter - 3/24/99 */
#if !NEW_PROBING
      if (PP_probe_lp && INSIDE_FUNCTION)
	{
	  comma_inserted = 0;
	  arg = expr->operands;
	  if ((arg->value.var.name != NULL) &&
	      !strcmp (arg->value.var.name, "exit"))
	    {
	      Stmt loopstmt;
	      for (loopstmt = expr->parentstmt; loopstmt;
		   loopstmt = loopstmt->parent)
		{
		  if (loopstmt->type == ST_SERLOOP)
		    {
		      int *loop_id_ptr;
		      if ((loop_id_ptr = (int *) loopstmt->ext))
			{
			  PP_C_gen_update_lp_iter_func_in_middle (Fcout,
								  *loop_id_ptr);
			  fprintf (Fcout, ", ");
			}
		      else
			{
			  I_warn ("GenProbedExpr: lack of loop id info");
			}
		    }
		}
	    }
	}
#endif

      /* callee */

      arg = expr->operands;
      if (precedence[arg->opcode] <= precedence[OP_call])
	fputc ('(', Fcout);

      if (PP_probe_ip && (arg->opcode != OP_var))
	{
	  /* indirect */
	  fputc ('(', Fcout);
	  fprintf (Fcout, "_PP_record_ip (%d,", next_ipc_id++);
	  GenProbedExpr (arg);
	  fputc (')', Fcout);
	  fputc (',', Fcout);
	  GenProbedExpr (arg);
	  fputc (')', Fcout);
	}
      else
	{
	  GenProbedExpr (arg);
	}

      if (precedence[arg->opcode] <= precedence[OP_call])
	fputc (')', Fcout);

      if (PP_annotate_ip && (arg->opcode != OP_var))
	{
	  PP_annotate_ipc (expr, next_ipc_id++);
	}

      /* Generate argument list
       * ----------------------------------------------------------------------
       */

      fputc ('(', Fcout);

      for (arg = expr->operands->sibling; arg; arg = arg->next)
	{
	  if (precedence[arg->opcode] <= precedence[OP_compexpr])
	    {
	      fputc ('(', Fcout);
	      GenProbedExpr (arg);
	      fputc (')', Fcout);
	    }
	  else
	    {
	      GenProbedExpr (arg);
	    }

	  if (arg->next)
	    fprintf (Fcout, ", ");
	}

      fputc (')', Fcout);
      break;

    case OP_index:
      opnd = expr->operands;
      if (precedence[opnd->opcode] <= precedence[opcode])
	{
	  fputc ('(', Fcout);
	  GenProbedExpr (opnd);
	  fputc (')', Fcout);
	}
      else
	GenProbedExpr (opnd);
      fputc ('[', Fcout);
      GenProbedExpr (expr->operands->sibling);
      fputc (']', Fcout);
      break;
      /* binary */
    case OP_disj:
    case OP_conj:
      /* LCW - insert probe to each conditional expression of a conj or disj
       * expression - 10/08/95
       */
      /* BCC - 4/1/96 
       * Don't probe codes in global data declarations since it is executed 
       * only once and the probe might screw up the syntax.
       */
      if (INSIDE_FUNCTION && (PP_probe || PP_annotate))
	{
	  rel_opcode = expr->operands->opcode;

	  /* CWL - 01/09/00 we should insert probe for every OP_conj
	     and OP_disj, however, the following code will just insert
	     probe for the first OP_conj/OP_disj.  keep this way for
	     comparing PtoL and PtoHtoL. Since the Pcode is flattened
	     before profiling, this should be no problem. We may
	     modify this later.  */
	  fputc ('(', Fcout);
	  if ((rel_opcode != OP_disj) && (rel_opcode != OP_conj))
	    {
	      fputc ('(', Fcout);
	      PP_C_insert_middle_probe (Fcout, next_probe++);
	      fprintf (Fcout, ", ");

	      /* LCW - read the profiling data - 10/24/95 */
	      if (PP_annotate)
		{
		  old_count = current_prof_count;
		  /* LCW - modified to read floating-point profile weights and to
		     handle the case when EOF is encountered - 3/5/97 */
		  if (fscanf (Fprofile, "%lf", &current_prof_count) == EOF)
		    current_prof_count = 0.0;
		}
	    }

	  GenProbedExpr (expr->operands);

	  if ((rel_opcode != OP_disj) && (rel_opcode != OP_conj))
	    fputc (')', Fcout);
	  fputc (')', Fcout);

	  /* LCW - reset the profiling data - 10/30/95 */
	  if (PP_annotate)
	    {
	      P_RemoveProfEXPR (expr->profile);
	      expr->profile = P_NewProfEXPR ();
	      expr->profile->count = expr->operands->profile->count;
	      current_prof_count = old_count;
	    }

	  /* CWL - 01/07/01 */
	  fputc (' ', Fcout);
	  GenOpcode (opcode);
	  fputc (' ', Fcout);

	  rel_opcode = (expr->operands->sibling)->opcode;
	  fputc ('(', Fcout);
	  if ((rel_opcode != OP_disj) && (rel_opcode != OP_conj))
	    {
	      fputc ('(', Fcout);
	      PP_C_insert_middle_probe (Fcout, next_probe++);
	      fprintf (Fcout, ", ");

	      /* LCW - read the profiling data - 10/24/95 */
	      if (PP_annotate)
		{
		  /* LCW - modified to read floating-point profile weights and to
		     handle the case when EOF is encountered - 3/5/97 */
		  if (fscanf (Fprofile, "%lf", &current_prof_count) == EOF)
		    current_prof_count = 0.0;
		  old_count = current_prof_count;
		}
	    }

	  fputc (' ', Fcout);
	  GenProbedExpr (expr->operands->sibling);
	  if ((rel_opcode != OP_disj) && (rel_opcode != OP_conj))
	    fprintf (Fcout, ")");
	  fputc (')', Fcout);
	  if (PP_annotate)
	    current_prof_count = old_count;
	  break;
	}
    case OP_or:
    case OP_xor:
    case OP_and:
    case OP_eq:
    case OP_ne:
    case OP_lt:
    case OP_le:
    case OP_ge:
    case OP_gt:
    case OP_rshft:
    case OP_lshft:
    case OP_add:
    case OP_sub:
    case OP_mul:
    case OP_div:
    case OP_mod:

      opnd = expr->operands;
      if (precedence[opnd->opcode] <= precedence[opcode])
	{
	  fputc ('(', Fcout);
	  GenProbedExpr (opnd);
	  fputc (')', Fcout);
	}
      else
	GenProbedExpr (opnd);
      GenOpcode (opcode);

      opnd = expr->operands->sibling;
      if (precedence[opnd->opcode] <= precedence[opcode])
	{
	  fputc ('(', Fcout);
	  GenProbedExpr (opnd);
	  fputc (')', Fcout);
	}
      else
	GenProbedExpr (opnd);

      break;

      /*
       * BCC - 11/13/97
       * If the assignment is of the form "@arg.... = i" or
       * "@return.... = j", just print the right-hand-side as i or j.
       * If the assignment is of the form "k = @param....", just ignore
       * the whole thing.
       */
    case OP_assign:

      if (
#if 0
	   IsUselessVar (expr->operands, NULL)
#else
	   0
#endif
	)
	GenProbedExpr (expr->operands->sibling);
      else
	{
	  Expr op1, op2;
	  int paren;
	  
	  op1 = expr->operands;
	  op2 = op1->sibling;

	  paren = (precedence[op2->opcode] <= precedence[opcode]);

	  /* CWL - for __builtin_va_start, just print the rhs */

	  if (PP_annotate || !P_FindPragma (op2->pragma, "use_ret_as_parm0"))
	    {

              /* If LHS is a cast, just print the value.  gcc4 doesn't allow
                 casts on the LHS.  -KF 12/2005 */

              if (op1->opcode == OP_cast)
                {
                  GenProbedExpr (op1->operands);
                }
              else
                {
                  GenProbedExpr (expr->operands);
                }

	      fprintf (Fcout, " = ");
	    }

	  if (paren)
	    fputc ('(', Fcout);

	  GenProbedExpr (op2);

	  if (paren)
	    fputc (')', Fcout);
	}
      break;

    case OP_Aadd:
    case OP_Asub:
    case OP_Amul:
    case OP_Adiv:
    case OP_Amod:
    case OP_Arshft:
    case OP_Alshft:
    case OP_Aand:
    case OP_Aor:
    case OP_Axor:

      opnd = expr->operands;
      if (precedence[opnd->opcode] <= precedence[opcode])
	{
	  fputc ('(', Fcout);
	  GenProbedExpr (opnd);
	  fputc (')', Fcout);
	}
      else
	GenProbedExpr (opnd);
      GenOpcode (opcode);
      opnd = expr->operands->sibling;
      if (precedence[opnd->opcode] <= precedence[opcode])
	{
	  fputc ('(', Fcout);
	  GenProbedExpr (opnd);
	  fputc (')', Fcout);
	}
      else
	{
	  GenProbedExpr (opnd);
	}

      break;

      /* the second operand is in value.string */
    case OP_dot:
    case OP_arrow:

      opnd = expr->operands;
      if (precedence[opnd->opcode] <= precedence[opcode])
	{
	  fputc ('(', Fcout);
	  GenProbedExpr (opnd);
	  fputc (')', Fcout);
	}
      else
	GenProbedExpr (opnd);
      GenOpcode (opcode);
      fprintf (Fcout, "%s", expr->value.string);
      break;

/* unary */
/*  case OP_abs : */
    case OP_neg:
    case OP_not:
    case OP_inv:
    case OP_preinc:
    case OP_predec:
      /* CWL - 01/07/01 */
      GenOpcode (opcode);
      opnd = expr->operands;
      if (precedence[opnd->opcode] <= precedence[opcode])
	{
	  fputc ('(', Fcout);
	  GenProbedExpr (opnd);
	  fputc (')', Fcout);
	}
      else
	GenProbedExpr (opnd);
      break;

    case OP_postinc:
    case OP_postdec:

      opnd = expr->operands;
      if (precedence[opnd->opcode] <= precedence[opcode])
	{
	  fputc ('(', Fcout);
	  GenProbedExpr (opnd);
	  fputc (')', Fcout);
	}
      else
	GenProbedExpr (opnd);
      GenOpcode (opcode);
      break;

    case OP_addr:

      fputc ('&', Fcout);
      opnd = expr->operands;
      if (precedence[opnd->opcode] <= precedence[OP_addr])
	{
	  fputc ('(', Fcout);
	  GenProbedExpr (opnd);
	  fputc (')', Fcout);
	}
      else
	GenProbedExpr (opnd);

      break;

    case OP_indr:

      fputc ('*', Fcout);
      opnd = expr->operands;
      if (precedence[opnd->opcode] <= precedence[OP_indr])
	{
	  fputc ('(', Fcout);
	  GenProbedExpr (opnd);
	  fputc (')', Fcout);
	}
      else
	GenProbedExpr (opnd);

      break;

    case OP_compexpr:
      {
	Expr opnd;

	/* 03/16/04 REK Adding parens around compexpr so the commas don't
	 *              confuse the C parser. */
	fprintf (Fcout, "(");

	opnd = expr->operands;
	GenProbedExpr (opnd);
	opnd = opnd->next;
	while (opnd)
	  {
	    fprintf (Fcout, ", ");
	    GenProbedExpr (opnd);
	    opnd = opnd->next;
	  }

	fprintf (Fcout, ")");

	/* LCW - reset the profiling data - 10/30/95 */
	if (PP_annotate)
	  {
	    P_RemoveProfEXPR (expr->profile);
	    expr->profile = P_NewProfEXPR ();
	    expr->profile->count = expr->operands->profile->count;
	  }
	break;
	/* don't do anything for the following */
      }

    case OP_stmt_expr:
      {
	if (!PP_probe)
	  {
	    fprintf (Fcout, "(");
	    GenStmt (expr->value.stmt, 1);
	    fprintf (Fcout, ")");
	  }
      }
      break;

      /*
         case OP_error :            
       */
    default:
      I_punt ("GenProbedExpr : illegal expression");
    }

  return 1;
}


static void
PP_GenProbedStmt (Stmt st)
{
  Expr ex;

  if (!st)
    return;
  if (st->shadow)
    return;

  switch (st->type)
    {
    case ST_EXPR:
      ex = st->stmtstruct.expr;
      GenIndent (1);
      GenProbedExpr (ex);
      fprintf (Fcout, ";\n");
      break;
    case ST_RETURN:
      ex = st->stmtstruct.ret;
      GenIndent (1);
      fprintf (Fcout, "return ");
      GenProbedExpr (ex);
      fprintf (Fcout, ";\n");
      break;
    case ST_NOOP:
      break;
    case ST_COMPOUND:
#if 0
      {
	Stmt sst;
	/* Processing these actually yields an error! */
	for (sst = st->stmtstruct.compound->stmt_list; sst;
	     sst = sst->lex_next)
	  PP_GenProbedStmt (sst);
      }
#endif
      break;
    default:
      P_warn ("Bad stmt type %d", st->type);
    }
  return;
}


/*! \brief Inserts probe pstmts in CFG blocks, for each lp hdr and lp exit.
 *
 * \return void.
 */
static void
PP_process_loops ()
{
  PC_Loop lp;
  PC_PStmt ps;
  int i, num_ex = 0;
  int *ex;

  for (lp = PC_cfg->lp; lp; lp = lp->next)
    {
      /* Add a loop head pstmt to the loop header bb. */
      {
	PC_Probe pb = PC_NewProbe (PC_PT_LpHead, lp->ID);
	ps =
	  PC_NewPStmtProbe (PC_T_Probe, PC_FindBlock (PC_cfg, lp->head), pb);
	PC_FreeProbe (pb);
      }

      /* Convert set of lp exits into an array of exit bb IDs. */
      ex = (int *) calloc (Set_size (lp->exits), sizeof (int));
      num_ex = Set_2array (lp->exits, ex);

      /* Add loop exit pstmts to the loop exit bbs. */
      for (i = 0; i < num_ex; i++)
	{
	  PC_Probe pb = PC_NewProbe (PC_PT_LpExit, lp->ID);
	  ps =
	    PC_NewPStmtProbe (PC_T_Probe, PC_FindBlock (PC_cfg, ex[i]), pb);
	  PC_FreeProbe (pb);
	}

      free (ex);
    }
}


/*! \brief Processes each CFG block to translate corresponding pcode stmts into
 *   C code stmts. This translation must happen after insertion of probe pstmts 
 *   in PP_process_loops ().
 *
 * \param st
 *  the Stmt to process.
 *
 * \return void.
 */
static void
PP_process_stmts (Stmt st)
{
  PC_Block bb = NULL;
  PC_PStmt ps = NULL;

#if !NEW_PROBING
  int SerloopHeader = 0;
#endif

  INSIDE_FUNCTION = 1;

  for (bb = PC_cfg->first_bb; bb; bb = bb->next)
    {
      fprintf (Fcout, "BB_%d:\n", bb->ID);

#if !NEW_PROBING
      if (SerloopHeader)
	{
	  /* LCW - the flag will be needed in the following. It
	     will be set to false later. - 3/24/99 */
	  if (!((PP_probe_lp || PP_annotate_ip) && INSIDE_FUNCTION))
	    /* Set flag to false.  Will be set to true after the
	       NT_SerloopInitCond block for the next Serloop is
	       generated. */
	    SerloopHeader = FALSE;
	}
#endif

      if (bb == PC_cfg->first_bb)
	{
	  GenIndent (1);
	  fprintf (Fcout, "_PP_initialize();\n");
	}

#if NEW_PROBING
      /* Tahir: new loop counter insertion implemented */
      if (PP_probe_lp || PP_annotate_lp)
	{
	  /* insert the initialization of loop iter counters in the first
	     basic block of each function */
	  if (bb == PC_cfg->first_bb)
	    PP_C_gen_loop_iter_counter_initial (Fcout, last_lp_id, total_lps);

	  for (ps = bb->first_ps; ps; ps = ps->succ)
	    {
	      /* insert a loop iter counter in the loop header BB. */
	      if (ps->type == PC_T_Probe
		  && ps->data.probe->type == PC_PT_LpHead)
		PP_C_insert_loop_iter_counter (Fcout, ps->data.probe->ID);
	      /* record loop counter value, and then reset it for next time. */
	      else if (ps->type == PC_T_Probe &&
		       ps->data.probe->type == PC_PT_LpExit)
		PP_C_gen_update_lp_iter_func (Fcout, ps->data.probe->ID);
	    }
	}

      last_lp_id = total_lps;	/* Remember last id for next func's loops. */
#endif

      /* LCW - insert a probe at the begining of each BB and generate the 
       * function call atexit(&dump_probe_array) if this is the first BB
       * of the main function - 10/10/95 */
      /* BCC - 4/1/96 
       * Don't probe codes in global data declarations since it is executed 
       * only once and the probe might screw up the syntax.
       */
      if (INSIDE_FUNCTION && (PP_probe || PP_annotate))
	{
	  GenIndent (1);
	  PP_C_insert_probe (Fcout, next_probe++);

	  /* LCW - get current profiling weight - 10/23/95 */

	  if (PP_annotate &&
	      fscanf (Fprofile, "%lf", &current_prof_count) == EOF)
	    current_prof_count = 0.0;
	}

      /* Process stmts inside block */

      {
	PC_PStmt pst = NULL;

	for (pst = bb->first_ps; pst; pst = pst->succ)
	  if (pst->type != PC_T_Probe)
	    PP_GenProbedStmt (pst->data.stmt);
      }

      /* Generate block control flow */

      switch (bb->cont_type)
	{
	case CNT_RETURN:
	case CNT_GOTO:
	  GenIndent (1);
	  fprintf (Fcout, "goto BB_%d;\n", bb->s_flow->dest_bb->ID);
	  break;
	case CNT_IF:
	  GenIndent (1);
	  fprintf (Fcout, "if ((");
	  GenProbedExpr (bb->cond);
	  fprintf (Fcout, ") == (");
	  GenProbedExpr (bb->s_flow->flow_cond);
	  fprintf (Fcout, ")) goto BB_%d; else goto BB_%d;\n",
		   bb->s_flow->dest_bb->ID,
		   bb->s_flow->s_next_flow->dest_bb->ID);
	  break;
	case CNT_SWITCH:
	  {
	    PC_Flow fl;
	    fprintf (Fcout, "switch (");
	    GenProbedExpr (bb->cond);
	    fprintf (Fcout, ")\n");
	    GenIndent (1);
	    fprintf (Fcout, "{\n");

	    for (fl = bb->s_flow; fl; fl = fl->s_next_flow)
	      {
		GenIndent (2);

		if (IsDefaultExpr (fl->flow_cond))
		  {
		    if (fl->s_next_flow)
		      P_punt ("default is not last in switch");

		    fprintf (Fcout, "default: goto BB_%d;\n",
			     fl->dest_bb->ID);
		  }
		else
		  {
		    fprintf (Fcout, "case (");
		    GenProbedExpr (fl->flow_cond);
		    fprintf (Fcout, "): goto BB_%d;\n", fl->dest_bb->ID);
		  }
	      }

	    GenIndent (1);
	    fprintf (Fcout, "}\n");
	  }
	case CNT_ENTRY:
	case CNT_EXIT:
	  break;
	case CNT_BREAK:
	default:
	  if (bb->s_flow)
	    {
	      GenIndent (1);
	      fprintf (Fcout, "goto BB_%d;\n", bb->s_flow->dest_bb->ID);
	      if (bb->s_flow->s_next_flow)
		P_warn ("Bad cfg block type (2) %d", bb->cont_type);
	    }
	  else
	    {
	      P_warn ("Bad cfg block type (1) %d, BB #%d",
		      bb->cont_type, bb->ID);
	    }
	}
    }

  INSIDE_FUNCTION = 0;
  return;
}

/*********************************************
**  Generate Statement Code Ends            **
**  DIA: 1/6/93                             **
*********************************************/

/* NJW - 12/15  Generate the remaining local data declarations
   CWL - 01/03/01 call P2C to generate C code 
 */
static int
GenRemainingLocalDataDcl (Stmt st)
{
  /* VarList ptr; */
  if (st == 0)
    return 0;

  switch (st->type)
    {
    case ST_IF:
      GenRemainingLocalDataDcl (st->stmtstruct.ifstmt->then_block);
      if (st->stmtstruct.ifstmt->else_block)
	GenRemainingLocalDataDcl (st->stmtstruct.ifstmt->else_block);
      break;
    case ST_SWITCH:
      GenRemainingLocalDataDcl (st->stmtstruct.switchstmt->switchbody);
      break;
    case ST_PARLOOP:
      GenRemainingLocalDataDcl (st->stmtstruct.parloop->pstmt->stmt);
      break;
    case ST_SERLOOP:
      GenRemainingLocalDataDcl (st->stmtstruct.serloop->loop_body);
      break;
    case ST_PSTMT:
      GenRemainingLocalDataDcl (st->stmtstruct.pstmt->stmt);
      break;
    case ST_BODY:
      GenRemainingLocalDataDcl (st->stmtstruct.bodystmt->statement);
      break;
    case ST_EPILOGUE:
      GenRemainingLocalDataDcl (st->stmtstruct.epiloguestmt->statement);
      break;
    case ST_COMPOUND:
      GenVarList (st->stmtstruct.compound->var_list, 1);
      GenRemainingLocalDataDcl (st->stmtstruct.compound->stmt_list);
      break;
    default:
      break;
    }
  if (st->lex_next != 0)
    GenRemainingLocalDataDcl (st->lex_next);
  return 1;
}


/*========================================================================*/
/*
 * LCW - Check if the expression contains function call(s) - 1/30/97 
 */
static int
contain_func_call (struct _Expr *expr)
{
  if (expr->opcode == OP_call)
    return (1);

  if (expr->operands != NULL)
    if (contain_func_call (expr->operands))
      return (1);

  if (expr->sibling != NULL)
    if (contain_func_call (expr->sibling))
      return (1);

  return (0);
}


/* LCW - scan the function and return the number of the loops in it -3/24/99 */
static int
PP_get_loop_number_expr (Expr expr)
{
  Expr ptr;
  int loops_no = 0;

  switch (expr->opcode)
    {
    case OP_stmt_expr:
      printf ("Loop STMTEXPR\n");
      loops_no += PP_get_loop_number (expr->value.stmt);
    default:
      /* Do nothing */
      break;
    }

  /* Recurse through rest of expr tree */
  for (ptr = expr->operands; ptr != 0; ptr = ptr->sibling)
    loops_no += PP_get_loop_number_expr (ptr);

  return loops_no;
}


static int
PP_get_loop_number (Stmt st)
{
  int loops_no = 0;
  Stmt StmtIndx;

  if (st == NULL)
    return (0);

  /* reset the ext field for later use */
  st->ext = 0;

  switch (st->type)
    {
    case ST_CONT:
    case ST_BREAK:
    case ST_RETURN:
    case ST_GOTO:
    case ST_ADVANCE:
    case ST_AWAIT:
    case ST_NOOP:
    case ST_ASM:
      break;
    case ST_EXPR:
      loops_no += PP_get_loop_number_expr (st->stmtstruct.expr);
      break;
    case ST_IF:
      loops_no += PP_get_loop_number_expr (st->stmtstruct.ifstmt->cond_expr);
      loops_no += PP_get_loop_number (st->stmtstruct.ifstmt->then_block);
      loops_no += PP_get_loop_number (st->stmtstruct.ifstmt->else_block);
      break;
    case ST_PSTMT:
      loops_no += PP_get_loop_number (st->stmtstruct.pstmt->stmt);
      break;
    case ST_MUTEX:
      loops_no += PP_get_loop_number (st->stmtstruct.mutex->statement);
      break;
    case ST_COBEGIN:
      I_punt ("PP_get_loop_number:  Can't handle cobegin yet");
      break;
    case ST_SWITCH:
      loops_no += PP_get_loop_number (st->stmtstruct.switchstmt->switchbody);
      break;
    case ST_COMPOUND:
      StmtIndx = st->stmtstruct.compound->stmt_list;
      while (StmtIndx != NULL)
	{
	  loops_no += PP_get_loop_number (StmtIndx);
	  StmtIndx = StmtIndx->lex_next;
	}
      break;
    case ST_PARLOOP:
      StmtIndx = st->stmtstruct.parloop->pstmt->stmt;

      StmtIndx = StmtIndx->stmtstruct.compound->stmt_list;
      while (StmtIndx && StmtIndx->type != ST_BODY &&
	     StmtIndx->type != ST_EPILOGUE)
	{
	  loops_no += PP_get_loop_number (StmtIndx);
	  StmtIndx = StmtIndx->lex_next;
	}

      loops_no += PP_get_loop_number (StmtIndx);

      if (StmtIndx)
	{
	  StmtIndx = StmtIndx->lex_next;
	  while (StmtIndx)
	    {
	      loops_no += PP_get_loop_number (StmtIndx);
	      StmtIndx = StmtIndx->lex_next;
	    }
	}

      break;
    case ST_SERLOOP:
      loops_no++;
      switch (st->stmtstruct.serloop->loop_type)
	{
	case LT_FOR:
	case LT_WHILE:
	  loops_no += PP_get_loop_number (st->stmtstruct.serloop->loop_body);
	  break;
	case LT_DO:
	  loops_no += PP_get_loop_number (st->stmtstruct.serloop->loop_body);
	  break;
	}
      break;
    case ST_BODY:
      loops_no += PP_get_loop_number (st->stmtstruct.bodystmt->statement);
      break;
    case ST_EPILOGUE:
      loops_no += PP_get_loop_number (st->stmtstruct.epiloguestmt->statement);
      break;
    default:
      I_punt ("PP_get_loop_number: Invalid instruction type");
    }

  return (loops_no);
}

/*****************************************************************************\
 *	File:	gen_ccode.c
 *	Author:	Chien-Wei Li and Wen-mei Hwu
\*****************************************************************************/
/*-----------------------------------------------------------*/

static int
GenIndent (int level)
{
  int i;

  for (i = 0; i < level; i++)
    fprintf (Fcout, "   ");
  return 1;
}

static P2C_DcltrNode
new_P2C_DcltrNode (_P2C_DcltrToken token, P2C_DcltrNode next)
{
  P2C_DcltrNode n;

  n = (P2C_DcltrNode) malloc (sizeof (_P2C_DcltrNode));
  if (!n)
    I_punt ("new_P2C_DcltrNode: malloc fail");
  n->token = token;
  n->next = next;
  return n;
}


static void
GenAttribute (Pragma pragma)
{
  while (pragma != NULL && strcmp (pragma->specifier, "Cattr") != 0 &&
	 pragma->next != NULL)
    pragma = pragma->next;

  if (pragma != NULL && strcmp (pragma->specifier, "Cattr") == 0)
    {
      fprintf (Fcout, " __attribute__ ((");

      while (pragma != NULL)
	{
	  if (strcmp (pragma->specifier, "Cattr") == 0)
	    {
	      Expr pragma_expr = pragma->expr->next;
	      char temp_str[256];

	      strcpy (temp_str, pragma->expr->value.string);
	      fprintf (Fcout, "%s", strtok (temp_str, "\""));
	      if (pragma_expr != NULL)
		fprintf (Fcout, " (");
	      while (pragma_expr != NULL)
		{
		  switch (pragma_expr->opcode)
		    {
		    case OP_string:
		      fprintf (Fcout, "%s", pragma->expr->next->value.string);
		      break;
		    case OP_int:
		      fprintf (Fcout, "%lli",
			       pragma->expr->next->value.scalar);
		      break;
		    default:
		      I_punt ("Gen_Pragma: invalid opcode type");
		      break;
		    }

		  pragma_expr = pragma_expr->next;
		  if (pragma_expr != NULL)
		    fprintf (Fcout, ", ");
		}
	      if (pragma->expr->next != NULL)
		fprintf (Fcout, ")");
	    }
	  pragma = pragma->next;
	  if (pragma != NULL && strcmp (pragma->specifier, "Cattr") == 0)
	    fprintf (Fcout, ", ");
	}
      fprintf (Fcout, ")) ");
    }
}

/* IMS - 8/7/03
 * Add support for "register"
 * 
 * For example:
 * register int i asm ("r13");
 * 
 * This is the 2nd part of the support -> it will print the 
 * asm ("r13") part.  The first part is stored as the storage
 * class so we dont need to deal with it.
 */
static int
GenRegisterDcl (Pragma pragma)
{
  while (pragma != NULL)
    {
      if (strcmp (pragma->specifier, "Cregister") == 0)
	{
	  fprintf (Fcout, " asm (");
	  fprintf (Fcout, pragma->expr->value.string);
	  fprintf (Fcout, ") ");
	  return 1;
	}			/* Pragma Cregister */
      pragma = pragma->next;
    }				/* while pragma */
  return 0;
}				/* GenRegisterDcl_part2 */

/* IMS - 7/8/03
 * Add inline to a function
 */
static int
GenInlineFunction (Pragma pragma)
{
  int is_inline = 0;
  while (pragma != NULL)
    {
      if (strcmp (pragma->specifier, "Cinline") == 0)
	{
	  /* Determine if we have an "extern inline" function or not */
	  if (strcmp (pragma->expr->value.string, "\"extern inline\"") == 0)
	    fprintf (Fcout, "extern inline ");
	  else
	    fprintf (Fcout, "inline ");
	  is_inline = 1;
	  break;		/* while */
	}			/* if */
      else
	pragma = pragma->next;
    }				/* while */
  return is_inline;
}				/* GenInlineFunction */

/* CWL-
   GenParam: print a list of parameters. 
*/
static int
GenParam (Param param_head)
{
  Param ptr;

  for (ptr = param_head; ptr; ptr = ptr->next)
    {
      GenAbstractType (ptr->key, P2C_PRINT_LONGLONG);
      if (ptr->next)
	fprintf (Fcout, ", ");
    }
  return 1;
}


/* 
   return 0: for no init 
   return 1: for simple init 
   return 2: for aggregate init
*/
static int
GenInit (Init init)
{
  Init cur_init;
  int r;

  if (!init)
    return 0;
  if (init->expr)
    {				/* simple init expression */
      GenProbedExpr (init->expr);
      return 1;
    }
  if (init->set)
    {				/* aggregate init expression */
      fputc ('{', Fcout);
      cur_init = init->set;
      while (cur_init)
	{
	  r = GenInit (cur_init);
	  cur_init = cur_init->next;
	  if (cur_init)
	    fprintf (Fcout, ",\n");	/* fputc(',', Fcout); */
	  if (r == 2)
	    fputc ('\n', Fcout);	/* aggregate init */
	}
      fputc ('}', Fcout);
      return 2;
    }
  return 0;
}


static int
GenTypeQualifier (_TypeQual tq, int last_print)
{
  int i = 0;

  if (tq & TY_CONST)
    {
      if (last_print)
	fprintf (Fcout, " ");
      fprintf (Fcout, "const");
      i = 1;
    }
  if (tq & TY_VOLATILE)
    {
      if (i == 1)
	fprintf (Fcout, " ");
      fprintf (Fcout, "volatile");
      i = 1;
    }
  if (tq & TY_SYNC)
    {
      if (i == 1)
	fprintf (Fcout, " ");
      fprintf (Fcout, "sync");
      i = 1;
    }				/* ??? */

  return (i | last_print);
}


static int
GenStorageClassSpecifier (_VarQual vq, int last_print)
{
  if (vq & VQ_REGISTER)
    {
      if (last_print)
	fputc (' ', Fcout);
      fprintf (Fcout, "register");
      return 1;
    }
  if (vq & VQ_STATIC)
    {
      if (last_print)
	fputc (' ', Fcout);
      fprintf (Fcout, "static");
      return 1;
    }
  if (vq & VQ_EXTERN)
    {
      if (last_print)
	fputc (' ', Fcout);
      fprintf (Fcout, "extern");
      return 1;
    }
  return last_print;
}


static int
GenTypeSpecifier (Key type, int last_print, int style)
{
  TypeDcl t = PSI_GetTypeDclEntry (type);
  _BasicType bt;

  bt = P_GetTypeDclBasicType (t);

  while (bt & (BT_POINTER | BT_ARRAY | BT_FUNC))
    {
      t = PSI_GetTypeDclEntry (t->type);
      bt = P_GetTypeDclBasicType (t);
    }

#if 0
  if (P_IsSignedTypeDcl (t))
    {
      if (last_print)
	fputc (' ', Fcout);
      fprintf (Fcout, "signed");
      last_print = 1;
    }
#endif
  if (P_IsUnsignedTypeDcl (t))
    {
      if (last_print)
	fputc (' ', Fcout);
      fprintf (Fcout, "unsigned");
      last_print = 1;
    }
  if (bt & BT_VOID)
    {
      if (last_print)
	fputc (' ', Fcout);
      fprintf (Fcout, "void");
      last_print = 1;
    }
  else if (bt & BT_CHAR)
    {
      if (last_print)
	fputc (' ', Fcout);
      fprintf (Fcout, "char");
      last_print = 1;
    }
  else if (bt & BT_SHORT)
    {
      if (last_print)
	fputc (' ', Fcout);
      fprintf (Fcout, "short");
      last_print = 1;
    }
  else if (bt & BT_INT)
    {
      if (last_print)
	fputc (' ', Fcout);
      fprintf (Fcout, "int");
      last_print = 1;
    }
  else if (bt & BT_LONG)
    {
      if (last_print)
	fputc (' ', Fcout);
      fprintf (Fcout, "long");
      last_print = 1;
    }
  else if (bt & BT_LONGLONG)
    {
      fprintf (Fcout, "%s%s", last_print ? " " : "", "long long");
      last_print = 1;
    }
  else if (bt & BT_FLOAT)
    {
      fprintf (Fcout, "%sfloat", last_print ? " " : "");
      last_print = 1;
    }
  else if (bt & BT_DOUBLE)
    {
      fprintf (Fcout, "%sdouble", last_print ? " " : "");
      last_print = 1;
    }
  else if (bt & BT_LONGDOUBLE)
    {
      fprintf (Fcout, "%slong double", last_print ? " " : "");
      last_print = 1;
    }
  else if (bt & BT_STRUCT)
    {
      fprintf (Fcout, "%sstruct %s", last_print ? " " : "", t->name);
      last_print = 1;
    }
  else if (bt & BT_UNION)
    {
      fprintf (Fcout, "%sunion %s", last_print ? " " : "", t->name);
      last_print = 1;
    }
  else if (bt & BT_ENUM)
    {
      fprintf (Fcout, "%senum %s", last_print ? " " : "", t->name);
      last_print = 1;
    }
#if P2C_USE_TYPEDEFS
  else if (bt & BT_TYPEDEF_E)
    {
      fprintf (Fcout, "%s%s", last_print ? " " : "", t->name);
      last_print = 1;
    }
  else if (bt & BT_TYPEDEF_I)
#else
  else if (bt & BT_TYPEDEF)
#endif
    {
      last_print = GenTypeSpecifier (t->type, last_print, style);
    }
  else if (bt & BT_VARARG)
    {
      fprintf (Fcout, "...");
      last_print = 1;
    }

  return last_print;
}

static int
GenType (Key type, int last_print, int style)
{
  int ret;
  _TypeQual tq = PSI_GetTypeQualifier (type);

  /* print type qualifier */
  ret = GenTypeQualifier (tq, last_print);

  /* print type specifier */
  return GenTypeSpecifier (type, ret, style);
}

static int
GenAbstractType (Key type, int style)
{
  /* print type */
  GenType (type, 0, style);
  fputc (' ', Fcout);

  /* print declarator */
  return GenDcltr (type, "", 0, (VarList) 0, 0, 0);
}

#if !NEW_PROBING
static int
GenTypedef (TypeDcl td)
{
  _BasicType bt = P_GetTypeDclBasicType (td);

  if (!(bt & BT_TYPEDEF_E))
    return 0;

  if (!P_ValidKey (td->type))
    return 0;

  fprintf (Fcout, "typedef ");
  GenType (td->type, 0, P2C_PRINT_LONGLONG);
  fprintf (Fcout, " %s;\n", td->name);
  return 1;
}
#endif

static int
GenLabels (Label labels, int level)
{
  Label cur_label;

  cur_label = labels;
  while (cur_label)
    {
      switch (cur_label->type)
	{
	case LB_LABEL:
	  fprintf (Fcout, "%s:\n", cur_label->val);
	  break;
	case LB_CASE:
	  GenIndent (level - 1);
	  fprintf (Fcout, "case ");
	  GenProbedExpr (cur_label->data.expression);
	  fprintf (Fcout, ":\n");
	  break;
	case LB_DEFAULT:
	  GenIndent (level - 1);
	  fprintf (Fcout, "default :\n");
	  break;
	default:
	  I_punt ("GenLabels: unknown cur_label->type");
	}
      cur_label = cur_label->next;
    }
  return 1;
}

static int
GenCompound (Stmt stmt, Compound compound, int level)
{
  Stmt current;
  GenIndent (level - 1);
  fprintf (Fcout, "{\n");

  /* print local variable declarations */
  GenVarList (compound->var_list, level);

  /* print the compound statement body */
  current = compound->stmt_list;
  while (current)
    {
      GenStmt (current, level);
      current = current->lex_next;
      if (current)
	fputc ('\n', Fcout);
    }
  GenIndent (level - 1);
  fprintf (Fcout, "}\n");
  return 1;
}

static int
GenIfStmt (IfStmt ifstmt, int level)
{
  GenIndent (level);
  /* print conditional expression */
  fprintf (Fcout, "if (");
  GenProbedExpr (ifstmt->cond_expr);
  fprintf (Fcout, ")\n");

  /* generate then-block */
  GenStmt (ifstmt->then_block, level + 1);

  /* generate else-block */
  if (ifstmt->else_block)
    {
      GenIndent (level);
      fprintf (Fcout, "else\n");
      GenStmt (ifstmt->else_block, level + 1);
    }
  return 1;
}

static int
GenSwitchStmt (SwitchStmt switchstmt, int level)
{
  GenIndent (level);

  /* print switch (expr) */
  fprintf (Fcout, "switch (");
  GenProbedExpr (switchstmt->expression);
  fprintf (Fcout, ")\n");

  /* print switch body */
  GenStmt (switchstmt->switchbody, level + 1);
  return 1;
}

/* PrintMultilineString
 * IMS - 7/25/03 
 *
 * This function will take a string (str) and print it. However, 
 * when it encounters a \n, it will print the \n, finish the 
 * string (add quotes), start a new line, add quotes, and continue 
 * with the string.
 *
 * Inputs:
 * - char * str 
 * - int level: level of the current function
 *
 * Outputs:
 * - currently just a 0
 *
 */
static int
PrintMultilineString (char *str, int level)
{
  int i = 0;

  while (str[i] != '\0')
    {
      /* check for newline */
      if (str[i] == '\\' && str[i + 1] == 'n')
	{
	  /* Print /n, quotes, */
	  fprintf (Fcout, "\\n\"");

	  /* lets get the next line started (if necessary) */
	  if (str[i + 2] == '\0' || str[i + 2] == '\"')
	    break;		/* we are finished */
	  else
	    {
	      fprintf (Fcout, "\n");
	      GenIndent (level + 1);
	      fprintf (Fcout, "  \"");
	    }

	  i++;			/* We just did 2 characters */
	}			/* if newline */
      else			/* just a normal character */
	{
	  fputc (str[i], Fcout);
	}			/* not a newline */
      i++;
    }				/* While not end of string */

  return 0;
}				/* PrintMultilineString */

static int
GenAsmStmt (AsmStmt asmstmt, int level)
{
  Expr expr, oprd_expr;
  int first;
  int has_outputs, has_inputs;

  GenIndent (level);
  fprintf (Fcout, "asm ");
  /* IMS - 7/25/03 */
  if (asmstmt->is_volatile)
    fprintf (Fcout, "volatile ");
  fprintf (Fcout, "(");
  /* IMS - 7/25/03
   * So I originally thought that I could not print out a string
   * all at once--that it needed to be spaced nicely.  I was wrong,
   * but this fix looks nice at least.
   */
  PrintMultilineString (asmstmt->asm_string->value.string, level);
  fprintf (Fcout, "\n");
  /* IMS - 7/8/03
   * If there is not asmstmt->asm_operands or clobbers,  then we 
   * can skip all this stuff about the INPUTS and OUTPUTS.
   */
  if (asmstmt->asm_clobbers || asmstmt->asm_operands)
    {
      has_outputs = 1;
#if 0
      for (expr = asmstmt->asm_operands; expr; expr = expr->next)
	{
	  oprd_expr = expr;
	  expr = expr->next;
	  if (!(oprd_expr->value.asmoprd->modifiers & 0x02))
	    continue;
	  has_outputs = 1;
	  break;
	}
#endif

      has_inputs = 0;
      for (expr = asmstmt->asm_operands; expr; expr = expr->next)
	{
	  oprd_expr = expr;
	  expr = expr->next;
	  if ((oprd_expr->value.asmoprd->modifiers & 0x02))
	    continue;
	  has_inputs = 1;
	  break;
	}

      /* OUTPUTS
         : "<modifiers><constraints>" (<var expr>), */
      if (has_outputs)
	{
	  GenIndent (level);
	  fprintf (Fcout, ": ");
	  first = 1;
	  for (expr = asmstmt->asm_operands; expr; expr = expr->next)
	    {
	      oprd_expr = expr;
	      expr = expr->next;
	      assert (expr);
	      assert (oprd_expr->opcode == OP_asm_oprd);
	      /* Find the outputs */
	      if (!(oprd_expr->value.asmoprd->modifiers & 0x02))
		continue;
	      if (!first)
		fprintf (Fcout, ", ");
	      first = 0;
	      fprintf (Fcout, "\"");
	      fprintf (Fcout, "%s",
		       P_Asmmod2String (oprd_expr->value.asmoprd->modifiers));
	      fprintf (Fcout, "%s", oprd_expr->value.asmoprd->constraints);
	      fprintf (Fcout, "\" (");
	      GenProbedExpr (expr);
	      fprintf (Fcout, ")");
	    }
	  fprintf (Fcout, "\n");
	}
      else /* no outputs */ if (has_inputs || asmstmt->asm_clobbers)
	{
	  GenIndent (level);
	  fprintf (Fcout, ": \n");	// Then we still need to have a colon placeholder
	}			/* end if */

      /* INPUTS
         : "<modifiers><constraints>" (<var expr>), */
      if (has_inputs)
	{
	  GenIndent (level);
	  fprintf (Fcout, ": ");
	  first = 1;
	  for (expr = asmstmt->asm_operands; expr; expr = expr->next)
	    {
	      oprd_expr = expr;
	      expr = expr->next;
	      assert (expr);
	      assert (oprd_expr->opcode == OP_asm_oprd);
	      /* Find the inputs */
	      if (oprd_expr->value.asmoprd->modifiers & 0x02)
		continue;
	      if (!first)
		fprintf (Fcout, ", ");
	      first = 0;
	      fprintf (Fcout, "\"");
	      fprintf (Fcout, "%s",
		       P_Asmmod2String (oprd_expr->value.asmoprd->modifiers));
	      fprintf (Fcout, "%s", oprd_expr->value.asmoprd->constraints);
	      fprintf (Fcout, "\" (");
	      GenProbedExpr (expr);
	      fprintf (Fcout, ")");
	    }
	  fprintf (Fcout, "\n");
	}
      else /* no outputs */ if (asmstmt->asm_clobbers)
	{
	  GenIndent (level);
	  fprintf (Fcout, ": \n");	// Then we still need to have a colon placeholder
	}			/* end if */
    }				/* if we have inputs / outputs */

  /* : "<clobbers>" */
  if (asmstmt->asm_clobbers != NULL)
    {
      GenIndent (level);
      fprintf (Fcout, ": ");
      first = 1;
      for (expr = asmstmt->asm_clobbers; expr; expr = expr->next)
	{
	  if (!first)
	    fprintf (Fcout, ", ");
	  first = 0;
	  fprintf (Fcout, "%s", expr->value.string);
	}
      fprintf (Fcout, "\n");
    }

  GenIndent (level);
  fprintf (Fcout, ");\n");
  return 1;
}

static int
GenWhileLoop (SerLoop serloop, int level)
{
  GenIndent (level);
  fprintf (Fcout, "while (");
  /* print conditional expression */
  GenProbedExpr (serloop->cond_expr);
  fprintf (Fcout, ")\n");
  /* print loop body */
  GenStmt (serloop->loop_body, level + 1);
  return 1;
}

static int
GenForLoop (SerLoop serloop, int level)
{
  GenIndent (level);
  fprintf (Fcout, "for (");
  /* print initial expression */
  GenProbedExpr (serloop->init_expr);
  fprintf (Fcout, " ;\n");
  /* print conditional expression */
  GenIndent (level);
  fprintf (Fcout, "      ");	/* aligned with initial expression */
  GenProbedExpr (serloop->cond_expr);
  fprintf (Fcout, " ;\n");
  /* print iteration expression */
  GenIndent (level);
  fprintf (Fcout, "      ");	/* aligned with initial expression */
  GenProbedExpr (serloop->iter_expr);
  fprintf (Fcout, ")\n");
  /* print loop body */
  GenStmt (serloop->loop_body, level + 1);
  return 1;
}

static int
GenDoLoop (SerLoop serloop, int level)
{
  GenIndent (level);
  fprintf (Fcout, "do\n");
  /* generate loop body */
  GenStmt (serloop->loop_body, level + 1);
  /* generate conditional expression */
  GenIndent (level);
  fprintf (Fcout, "while (");
  GenProbedExpr (serloop->cond_expr);
  fprintf (Fcout, ");\n");
  return 1;
}

static int
GenSerLoop (SerLoop serloop, int level)
{
  switch (serloop->loop_type)
    {
    case LT_WHILE:
      GenWhileLoop (serloop, level);
      break;
    case LT_FOR:
      GenForLoop (serloop, level);
      break;
    case LT_DO:
      GenDoLoop (serloop, level);
      break;
    default:
      I_punt ("GenSerLoop: unknown loop type");
    }
  return 1;
}

static int
GenStmt (Stmt stmt, int level)
{
  if (stmt->shadow)
    return 0;
  /* generate label */
  GenLabels (stmt->labels, level);
  switch (stmt->type)
    {
    case ST_NOOP:
      GenIndent (level);
      fprintf (Fcout, "/* NOOP */;\n");
      return 0;
    case ST_CONT:
      GenIndent (level);
      fprintf (Fcout, "continue;\n");
      return 1;
    case ST_BREAK:
      GenIndent (level);
      fprintf (Fcout, "break;\n");
      return 1;
    case ST_RETURN:
      GenIndent (level);
      fprintf (Fcout, "return ");
      GenProbedExpr (stmt->stmtstruct.ret);
      fprintf (Fcout, ";\n");
      return 1;
    case ST_GOTO:
      GenIndent (level);
      fprintf (Fcout, "goto %s;\n", stmt->stmtstruct.label.val);
      return 1;
    case ST_COMPOUND:
      if (stmt->parent && (stmt->parent->type == ST_COMPOUND))
	GenCompound (stmt, stmt->stmtstruct.compound, level + 1);
      else
	GenCompound (stmt, stmt->stmtstruct.compound, level);
      return 1;
    case ST_IF:
      GenIfStmt (stmt->stmtstruct.ifstmt, level);
      return 1;
    case ST_SWITCH:
      GenSwitchStmt (stmt->stmtstruct.switchstmt, level);
      return 1;
    case ST_ASM:
      GenAsmStmt (stmt->stmtstruct.asmstmt, level);
      return 1;
    case ST_PSTMT:
      I_punt ("GenStmt: ST_PSTMT");
    case ST_ADVANCE:
      I_punt ("GenStmt: ST_ADVANCE");
    case ST_AWAIT:
      I_punt ("GenStmt: ST_AWAIT");
    case ST_MUTEX:
      I_punt ("GenStmt: ST_MUTEX");
    case ST_COBEGIN:
      I_punt ("GenStmt: ST_COBEGIN");
    case ST_PARLOOP:
      I_punt ("GenStmt: ST_PARLOOP");
    case ST_SERLOOP:
      GenSerLoop (stmt->stmtstruct.serloop, level);
      return 1;
    case ST_EXPR:
      GenIndent (level);
      GenProbedExpr (stmt->stmtstruct.expr);
      fprintf (Fcout, ";\n");
      return 1;
    case ST_BODY:		/* C extension ??? */
      I_punt ("GenStmt: ST_BODY");
    case ST_EPILOGUE:		/* C extension ??? */
      I_punt ("GenStmt: ST_EPILOGUE");
    default:
      I_punt ("GenStmt: unknown stmt->type");
    }
  return 0;
}

static int
GenFields (Field fields, int level)
{
  Field cur_field;
  cur_field = fields;
  while (cur_field)
    {
      GenIndent (level);
#if 0
      if (GenType (cur_field->type,
		   P2C_PRINT_STORAGE_CLASS | P2C_PRINT_LONGLONG))
#endif
      if (GenType (cur_field->type, 0, P2C_PRINT_LONGLONG))
	{
	  fputc (' ', Fcout);
	  GenDcltr (cur_field->type, cur_field->name,  0, (VarList) 0, 1, 0);
	}
      /* print bit field */
      if (cur_field->is_bit_field)
	{
	  fprintf (Fcout, " : %d", cur_field->bit_size);
	}
      if (cur_field->name && !strcmp ("__jmpbuf", cur_field->name))
	fprintf (Fcout, " __attribute__ ((aligned (16))) ");
      fprintf (Fcout, ";\n");
      cur_field = cur_field->next;
    }
  return 1;
}

static int
GenStructDcl (StructDcl st, int level)
{
  if (!st)
    I_punt ("/*** GenStruct: null st ***/\n\n");
  /* print header */
  GenIndent (level - 1);
#if 0
  if (st->new_name)
    fprintf (Fcout, "struct %s", st->new_name);
  else
#endif
    fprintf (Fcout, "struct %s", st->name);
  if (st->fields)
    {
      fprintf (Fcout, " {\n");
      /* print fields */
      GenFields (st->fields, level);
      GenIndent (level - 1);
      fprintf (Fcout, "}");
    }
  else if (P_GetStructDclQualifier (st) & SQ_EMPTY)
    {
      fprintf (Fcout, " { char __dummy; }");
    }
  /* IMS - 7/28/03 - Struct Attributes */
  GenAttribute (st->pragma);

  fprintf (Fcout, ";\n");
  return 1;
}

/**************************************************\
 * IMS - 7/4/03 
 * Creates assembly that is not in any function
\**************************************************/
static int
GenAsm (AsmDcl ad, int level)
{
  /* We are just going to use the same function
   * that the asm inside of functions uses */

  // Create a statement out of the Dcl
  AsmStmt asmstmt = P_NewAsmStmt ();
  asmstmt->is_volatile = ad->is_volatile;
  asmstmt->asm_clobbers = ad->asm_clobbers;
  asmstmt->asm_string = ad->asm_string;
  asmstmt->asm_operands = ad->asm_operands;
  GenAsmStmt (asmstmt, 0);
  return 0;
}

static int
GenUnionDcl (UnionDcl un, int level)
{
  if (!un)
    I_punt ("/*** GenUnion: null un ***/\n\n");
  /* print header */
  GenIndent (level - 1);
#if 0
  if (un->new_name)
    fprintf (Fcout, "union %s", un->new_name);
  else
#endif
    fprintf (Fcout, "union %s", un->name);
  if (un->fields)
    {
      fprintf (Fcout, " {\n");
      /* print fields */
      GenFields (un->fields, level);
      GenIndent (level - 1);
      fprintf (Fcout, "}");
    }
  /* IMS - 7/28/03 - Struct Attributes */
  GenAttribute (un->pragma);

  fprintf (Fcout, ";\n");
  return 1;
}


static FuncDcl PP_func;

static int
GenFuncDcl (FuncDcl func, int insert_probes)
{
  TypeDcl ftdcl;
  int start_probe = 0, i;
  if (!func)
    I_punt ("GenFuncDcl : nil input");
  if (!func->name)
    I_punt ("GenFuncDcl : no name");

  ftdcl = PSI_GetTypeDclEntry (func->type);

  if (insert_probes && P_GetFuncDclStmt (func))
    {
      PC_Function (func, 1, insert_probes ? PC_SPLIT_CRIT : 0);
      next_probe_BB = PC_cfg->num_bbs + PC_cfg->bb_id_offset + 1;
#if 1
      PP_annotate = PP_annotate_lp = 0;
#endif
      /* 
         CWL - 01/03/00
         if inserting probes, generate C code
         when doing annotation, there should be no need to generate any code.
       */
      if (PP_probe || PP_probe_lp)
	{
	  fprintf (Fcout, "extern void _PP_initialize();\n");
	  if (PP_probe)
	    {
	      fprintf (Fcout, "extern long %s[];\n", probe_array_name);
	      start_probe = next_probe;
	    }
	  if (PP_probe_lp)
	    {
	      fprintf (Fcout, "extern void _PP_loop_iter_update();\n");
	    }

	  if (PP_probe_ip)
	    {
	      fprintf (Fcout, "extern void _PP_init_ip_table (int, char *);\n"
		       "extern void *_PP_record_ip (int, void *);\n"
		       "extern void *_PP_dump_ip (void);\n");
	    }
	}

      if (PP_annotate)
	{
	  start_probe = next_probe;
	}

      /* BCC - signaling it's inside a function - 4/1/96 */
      INSIDE_FUNCTION = 1;
    }

  /* AES - 06/30/03 */
  GenAttribute (func->pragma);
  /* IMS - 7/8/03
   * Check if inlined
   */

  GenInlineFunction (func->pragma);

  /* print return type */

  {
    int ret = GenStorageClassSpecifier (func->qualifier, 0);
    GenType (ftdcl->type, ret, P2C_PRINT_LONGLONG);
  }

  fputc (' ', Fcout);

  PP_func = func;

  {
    char *name = func->name;

    if (func->qualifier & VQ_STATIC)
      {
	name = alloca (strlen (name) + 64);
#if PP_MANGLE_NAMES
	sprintf (name, "_PP_static_%s_%d_%d", func->name,
		 func->key.file, func->key.sym);
#else
	sprintf (name, "%s", func->name);
#endif
      }

    /* print function name (declarator) */
    GenDcltr (func->type, name, 1, func->param, 0, \
	      func->stmt != NULL && \
	      P_TstFuncDclQualifier (func, VQ_APP_ELLIPSIS | VQ_OLD_PARAM));
  }

  if (!func->stmt)
    {
      fprintf (Fcout, ";\n");
      return 1;
    }

  if (insert_probes)
    {
      /* CWL - 01/03/01 generate function body */
      fprintf (Fcout, "{\n");
      /* print local variable definition from compound stmts. */

      if (func->local)
	GenVarList (func->local, 1);
      GenRemainingLocalDataDcl (func->stmt);
      /* LCW - generate variable declaration for loop iter counters -
         3/24/99 */
      if (PP_probe_lp || PP_annotate_lp)
	for (i = last_lp_id; i < total_lps; i++)
	  {
	    GenIndent (1);
	    fprintf (Fcout, "unsigned long _PP_LP_COUNTER_%d;\n", i);
	  }

      /* Extraction of BB profile is now done by PC_Function() - JWS */

#if NEW_PROBING
      /* pre-process all loops inserting probe pstmts in header & exit bbs. */
      PP_process_loops ();
#endif

      /* print function body */
      PP_process_stmts (func->stmt);

      fprintf (Fcout, "}\n");	/* CWL - 01/03/01 end of function */

      /* LCW - put the function profile in the Pcode function pragma -
         1/28/97 */
      if (PP_annotate)
	{
	  Expr weight_expr = NULL;
	  if (func->stmt->profile)
	    {
	      weight_expr = P_NewDoubleExpr (func->stmt->profile->count);
	    }
	  else if (func->stmt->stmtstruct.compound->stmt_list &&
		   func->stmt->stmtstruct.compound->stmt_list->profile)
	    {
	      weight_expr =
		P_NewDoubleExpr (func->stmt->stmtstruct.compound->stmt_list->
				 profile->count);
	    }

	  if (weight_expr)
	    {
	      Pragma prg = P_NewPragmaWithSpecExpr ("\"profile\"",
						    weight_expr);
	      func->pragma = P_AppendPragmaNext (func->pragma, prg);
	      P_RemoveExpr (weight_expr);
	    }
	}

      /* LCW - write probe status to file - 10/12/95 */
      if (PP_probe)
	fprintf (Fallprobe, "%s %d %d\n", func->name, start_probe,
		 next_probe - 1);
      /* BCC - signaling it's finished parsing a function - 4/1/96 */
      INSIDE_FUNCTION = 0;
    }
  else
    {
      /* print function body */
      GenStmt (func->stmt, 1);
      fputc ('\n', Fcout);
    }

  fflush (Fcout);
  return 1;
}



static int
AM_priority (_BasicType method)
{
  /* 
     There should be neither array of function nor function returning
     array. That is we should not have case like ...()[] or ...[]().
     So [] and () should be of the same priority. 
   */

  if (method & (BT_ARRAY | BT_FUNC))
    return 2;
  else if (method & BT_POINTER)
    return 1;
  else
    return 0;
}


/* CWL -
   GenDcltr: print out an identifier and its declarators.
   It is done in two steps:
   1) pre-processing : build a link list of declarators with parentheses added
                      if necessary.
   2) dump the link list. 
*/
static int
GenDcltr (Key type, char *name, int printing_func,
	  VarList formal_param, int declare_var, int k_and_r_args)
{
  P2C_DcltrNode head, tail, ptr;
  VarList cur_formal_param;
  int cur_AM_priority, last_AM_priority;
  Key type_key;
  TypeDcl dcl = NULL;

  type_key = PSI_ReduceTypedefs (type);

  if (P_ValidKey (type_key))
    dcl = PSI_GetTypeDclEntry (type_key);

  /* pre-processing */
  tail = head = new_P2C_DcltrNode (DclTok_STR, NULL);
  head->value.str = name;

  if (dcl)
    {				/* first declarator */
      _BasicType bt = dcl->basic_type;

      if (bt & BT_POINTER)
	{
	  ptr = new_P2C_DcltrNode (DclTok_QUALIFIER, head);
	  ptr->value.qualifier = dcl->qualifier;
	  head = ptr;
	}
      if (bt & BT_ARRAY)
	{
	  ptr = new_P2C_DcltrNode (DclTok_EXPR, NULL);
	  ptr->value.expr = dcl->details.array_size;
	  tail->next = ptr;
	  tail = ptr;
	}
      else if (bt & BT_FUNC)
	{
	  if (printing_func)
	    {
	      ptr = new_P2C_DcltrNode (DclTok_FORMAL_PARAM, NULL);
	      ptr->value.formal_param = formal_param;
	    }
	  else
	    {
	      ptr = new_P2C_DcltrNode (DclTok_ABS_PARAM, NULL);
#if 0
	      {
		Param cur_param;
		for (cur_param = dcl->details.param; cur_param;
		     cur_param = cur_param->next)
		  {
		    VarDcl v = PSI_GetVarDclEntry (cur_param->key);
		    TypeDcl t = PSI_GetTypeDclEntry (v->type);
		    _BasicType pbt = P_GetTypeDclBasicType (t);

		    if (pbt & BT_VARARG)
		      {
			declare_var = 0;
			break;
		      }
		  }
	      }
	      ptr->value.abs_param = ((declare_var) ? 0 : dcl->details.param);
#else
	      ptr->value.abs_param = dcl->details.param;
#endif
	    }
	  tail->next = ptr;
	  tail = ptr;
	}

      last_AM_priority = AM_priority (bt);
      type_key = PSI_ReduceTypedefs (dcl->type);

      while (P_ValidKey (type_key) && (dcl = PSI_GetTypeDclEntry (type_key)))
	{
	  _BasicType bt = dcl->basic_type;

	  cur_AM_priority = AM_priority (bt);

	  if (cur_AM_priority > last_AM_priority)
	    {
	      /* wrap the declarator with "(" and ")" */
	      ptr = new_P2C_DcltrNode (DclTok_STR, head);
	      ptr->value.str = "(";
	      head = ptr;
	      ptr = new_P2C_DcltrNode (DclTok_STR, NULL);
	      ptr->value.str = ")";
	      tail->next = ptr;
	      tail = ptr;
	    }

	  if (bt & BT_POINTER)
	    {
	      ptr = new_P2C_DcltrNode (DclTok_QUALIFIER, head);
	      ptr->value.qualifier = dcl->qualifier;
	      head = ptr;
	    }
	  if (bt & BT_ARRAY)
	    {
	      ptr = new_P2C_DcltrNode (DclTok_EXPR, NULL);
	      ptr->value.expr = dcl->details.array_size;
	      tail->next = ptr;
	      tail = ptr;
	    }
	  else if (bt & BT_FUNC)
	    {
	      ptr = new_P2C_DcltrNode (DclTok_ABS_PARAM, NULL);
	      ptr->value.abs_param = dcl->details.param;
	      tail->next = ptr;
	      tail = ptr;
	    }

	  last_AM_priority = cur_AM_priority;

	  type_key = PSI_ReduceTypedefs (dcl->type);
	}
    }

  /* print out the pre-processed declarators */
  while (head)
    {
      switch (head->token)
	{
	case DclTok_STR:
	  fprintf (Fcout, head->value.str);
	  break;
	case DclTok_QUALIFIER:
	  fputc ('*', Fcout);
	  switch (head->value.qualifier)
	    {
	    case TY_DEFAULT:
	      break;
	    case TY_CONST:
	      fprintf (Fcout, " const ");
	      break;
	    case TY_VOLATILE:
	      fprintf (Fcout, " volatile ");
	      break;

#if 0
	      /* 2/13/04 REK These have been moved to the variable/function
	       * qualifier. */
	    case TY_CDECL:	/* C extension ??? */
	      fprintf (Fcout, "/*!!! DQ_CDECL !!!*/");
	      break;
	    case TY_STDCALL:	/* C extension ??? */
	      fprintf (Fcout, "/*!!! DQ_STDCALL !!!*/");
	      break;
	    case TY_FASTCALL:	/* C extension ??? */
	      fprintf (Fcout, "/*!!! DQ_FASTCALL !!!*/");
	      break;
#endif
	    default:
	      P_punt ("GenDcltr: unknown qualifier 0x%08x",
		      head->value.qualifier);
	    }
	  break;
	case DclTok_EXPR:
	  fputc ('[', Fcout);
	  GenProbedExpr (head->value.expr);
	  fputc (']', Fcout);
	  break;
	case DclTok_ABS_PARAM:
	  fputc ('(', Fcout);
	  GenParam (head->value.abs_param);
	  fputc (')', Fcout);
	  break;
	case DclTok_FORMAL_PARAM:
	  {
	    /* We can usually write ANSI style parameters.  However, if we
	     * have an old style varargs function, we must write K&R style
	     * parameters to keep gcc happy. */
	    VarDcl vd;
	    int cnt = 0;
	    fputc ('(', Fcout);
	    cur_formal_param = head->value.formal_param;
	    if (k_and_r_args)
	      {
		List_start (cur_formal_param);
		while ((vd = (VarDcl)List_next (cur_formal_param)))
		  {
		    if (cnt > 0)
		      fprintf (Fcout, ", ");
		    fprintf (Fcout, P_GetVarDclName (vd));
		    cnt++;
		  }
		fprintf (Fcout, ")\n\t");
	      }

	    cnt = 0;
	    List_start (cur_formal_param);
	    while ((vd = (VarDcl) List_next (cur_formal_param)))
	      {
		if (cnt > 0)
		  {
		    if (k_and_r_args)
		      fprintf (Fcout, ";\n\t");
		    else
		      fprintf (Fcout, ", ");
		  }

		GenType (vd->type, 0, P2C_PRINT_LONGLONG);

		if (PSI_GetTypeBasicType (vd->type) & BT_VARARG)
		  break;

		fputc (' ', Fcout);
		GenVarDcl (vd);
		cnt++;
	      }

	    if (!vd && P_FindPragma (PP_func->pragma, "append_gcc_ellipsis"))
	      {
		if (k_and_r_args)
		  fprintf (Fcout, "; ...\n");
		else
		  fprintf (Fcout, ", ...");
	      }

	    if (!k_and_r_args)
	      fputc (')', Fcout);
	  }
	  break;
	}
      ptr = head;
      head = head->next;
      free (ptr);
    }
  return 1;
}


static int
GenVarDcl (VarDcl var)
{
  char *name = var->name;

  if (!strcmp (name, "__PRETTY_FUNCTION__"))
    name = "__IMPACT_PRETTY_FUNCTION__";
  else if (!strcmp (name, "__FUNCTION__"))
    name = "__IMPACT_FUNCTION__";
  else
    {
      SymTabEntry ste = PSI_GetSymTabEntry (var->key);
      if (ste->type == ET_VAR_LOCAL && strncmp (var->name, "__builtin_", 10))
	{
	  name = alloca (strlen (name) + 64);
	  sprintf (name, "_PP_auto_%s_%d_%d", var->name,
		   var->key.file, var->key.sym);
	}
#if PP_MANGLE_NAMES
      else if (ste->type == ET_VAR_GLOBAL && (var->qualifier & VQ_STATIC))
	{
	  name = alloca (strlen (name) + 64);
	  sprintf (name, "_PP_static_%s_%d_%d", var->name,
		   var->key.file, var->key.sym);
	}
#endif
    }

  /* print var name */
  GenDcltr (var->type, name, 0, (VarList) 0, 1, 0);
  GenAttribute (var->pragma);

  /* IMS - 8/7/03
   * Support for "register"
   */
  GenRegisterDcl (var->pragma);

  /* print initial value */
  if (var->init)
    {
      fputc ('=', Fcout);
      GenInit (var->init);
    }
  return 1;
}

static int
GenVar (VarDcl var, int level)
{
  int ret;

  GenIndent (level);
  /* print type */
  ret = GenStorageClassSpecifier (var->qualifier, 0);
  GenType (var->type, ret, P2C_PRINT_LONGLONG);
  fputc (' ', Fcout);
  /* print var name and init */
  GenVarDcl (var);
  fprintf (Fcout, ";\n");
  return 0;
}

static int
GenVarList (VarList var_list, int level)
{
  VarDcl var;

  List_start (var_list);
  while ((var = (VarDcl) List_next (var_list)))
    {
      GenVar (var, level);
    }

  return 1;
}

/* ---------------------------------------------------------------------------- 
   exported functions
-----------------------------------------------------------------------------*/

int
Gen_CCODE_Struct (FILE * FL, StructDcl st)
{
  Fcout = FL;
  GenStructDcl (st, 1);
  return 1;
}

int
Gen_CCODE_Union (FILE * FL, UnionDcl un)
{
  Fcout = FL;
  GenUnionDcl (un, 1);
  return 1;
}

/* IMS - 8/4/03 */
int
Gen_CCODE_Asm (FILE * FL, AsmDcl ad)
{
  Fcout = FL;
  GenAsm (ad, 1);
  return 1;
}


int
Gen_CCODE_GlobalVar (FILE * FL, VarDcl var)
{
  int ret;

  if (!var)
    I_punt ("!!! GenGlobalVar: null var !!!");
  Fcout = FL;

  /* IMS - 7/15/03
   * Function declarations are global vars, so we need to print inline
   */
  GenInlineFunction (var->pragma);

  ret = GenStorageClassSpecifier (var->qualifier, 0);
  GenType (var->type, ret, P2C_PRINT_LONGLONG);
  fputc (' ', Fcout);
  GenVarDcl (var);
  fprintf (Fcout, ";\n");
  return 1;
}

int
Gen_CCODE_Func (FILE * FL, FuncDcl fn)
{
  Fcout = FL;

  GenFuncDcl (fn, PP_probe || PP_annotate || PP_probe_lp || PP_annotate_ip);
  return 1;
}

int
Gen_CCODE_Expr (FILE * FL, Expr expr)
{
  Fcout = FL;
  GenProbedExpr (expr);
  return 1;
}

int
Gen_CCODE_Type (FILE * FL, Key type, int style)
{
  Fcout = FL;
  GenType (type, 0, style);
  return 1;
}

int
Gen_CCODE_Indent (FILE * FL, int level)
{
  Fcout = FL;
  GenIndent (level);
  return 1;
}

int
Gen_CCODE_Typedef (FILE * FL, TypeDcl td)
{
  Fcout = FL;
#if P2C_USE_TYPEDEFS
  GenTypedef (td);
#endif
  return 1;
}
