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
 *	File:	pl_pr.c
 *	Author:	Po-hua Chang, Wen-mei Hwu
 *	Creation Date:	June 1990
 *	Modified:  Roger A. Bringmann  2/8/93
 *	    Changed Lcode paranthesization format
 *	Revised: Dave Gallagher, Scott Mahlke - 6/94
 *		Build Lcode structure rather than just prining out text file
 *      Revised by: Ben-Chung Cheng - June 1995
 *              Change M_SIZE_INT, M_SIZE_CHAR to P_INT_SIZE, P_CHAR_SIZE
 *              Those should be determined in runtime, not compile time
 *      Revised by: John Sias - July 2000
 *              64-bit
 *      Revised by: Chien-Wei Li - 12/2001 for P-to-L, 
 *              H_CHAR_SIZE to P_CHAR_SIZE, 
 *              H_INT_SIZE to P_INT_SIZE, 
 *              H_SHORT_SIZE to P_SHORT_SIZE, 
 *              H_UNSIGNED_CHAR_MASK to P_UNSIGNED_CHAR_MASK,
 *	  	H_UNSIGNED_SHORT_MASK to P_UNSIGNED_SHORT_MASK,
 *              H_punt to P_punt.
 *                          
\*****************************************************************************/
/*****************************************************************************\
 * BCC - Existing bugs : casting a label/string address to short/char - 6/27/95
\*****************************************************************************/

#include <config.h>
#include "pl_main.h"
#include <library/string_symbol.h>

/* extern declarations */

/* forward declarations */
static void PL_gen_expand (L_Cb * cb, char *fn, _PL_Operand arg[],
			   _M_Type ret_type, _M_Type mtype[],
			   int argc, L_Attr * attr1, Expr expr);

L_Operand *PL_gen_operand (PL_Operand op);
L_Operand *PL_gen_cast_operand (L_Cb *, PL_Operand);

static L_Oper *PL_gen_subroutine_call_for_operation (L_Cb *, PL_Operand,
						     PL_Operand, PL_Operand,
						     int opc);

static L_Attr *PL_create_stack_ref_attr (L_Oper * oper);

/*
 * MTYPE -> CTYPE CONVERSION
 * ----------------------------------------------------------------------
 */

static int
PL_mtype2ctype (int type, int unsign)
{
  int size;
  int ctype = 0;

  switch (type)
    {
    case M_TYPE_FLOAT:
      ctype = L_CTYPE_FLOAT;
      break;
    case M_TYPE_DOUBLE:
      ctype = L_CTYPE_DOUBLE;
      break;
    case M_TYPE_CHAR:
      ctype = unsign ? L_CTYPE_UCHAR : L_CTYPE_CHAR;
      break;
    case M_TYPE_SHORT:
      ctype = unsign ? L_CTYPE_USHORT : L_CTYPE_SHORT;
      break;
    case M_TYPE_INT:
      ctype = unsign ? L_CTYPE_UINT : L_CTYPE_INT;
      break;
    case M_TYPE_LONG:
      size = PL_MType_Size(M_TYPE_LONG);
      if (size == 64)
	ctype = unsign ? L_CTYPE_ULLONG : L_CTYPE_LLONG;
      else if (size == 32)
	ctype = unsign ? L_CTYPE_UINT : L_CTYPE_INT;
      else
	P_punt ("PL_mtype2ctype: unusual pointer size");
      break;
    case M_TYPE_LLONG:
      ctype = unsign ? L_CTYPE_ULLONG : L_CTYPE_LLONG;
      break;
    case M_TYPE_POINTER:
      if (P_POINTER_SIZE == 64)
	ctype = L_CTYPE_LLONG;
      else if (P_POINTER_SIZE == 32)
	ctype = L_CTYPE_INT;
      else
	P_punt ("PL_mtype2ctype: unusual pointer size");
      break;
    default:
      ctype = 0;
      break;
    }
  return ctype;
}


int
PL_reg_ctype (int type, int unsign)
{
  int ctype;

  if (L_propagate_sign_size_ctype_info)
    {
      switch (type)
	{
	case M_TYPE_FLOAT:
	  ctype = L_CTYPE_FLOAT;
	  break;
	case M_TYPE_DOUBLE:
	  ctype = L_CTYPE_DOUBLE;
	  break;
	case M_TYPE_CHAR:
	  ctype = unsign ? L_CTYPE_UCHAR : L_CTYPE_CHAR;
	  break;
	case M_TYPE_SHORT:
	  ctype = unsign ? L_CTYPE_USHORT : L_CTYPE_SHORT;
	  break;
	case M_TYPE_INT:
	  ctype = unsign ? L_CTYPE_UINT : L_CTYPE_INT;
	  break;
	case M_TYPE_LONG:
	  ctype = unsign ? L_CTYPE_ULONG : L_CTYPE_LONG;
	  break;
	case M_TYPE_LLONG:
	  ctype = unsign ? L_CTYPE_ULLONG : L_CTYPE_LLONG;
	  break;
	case M_TYPE_POINTER:
	  ctype = L_CTYPE_POINTER;
	  break;
	default:
	  ctype = 0;
	  break;
	}
    }
  else
    {
      switch (type)
	{
	case M_TYPE_FLOAT:
#ifdef PL_GEN_FLOAT_OPERANDS
	  ctype = L_CTYPE_FLOAT;
	  break;
#endif
	case M_TYPE_DOUBLE:
	  ctype = L_CTYPE_DOUBLE;
	  break;
	default:
	  ctype = PL_native_int_reg_ctype;
	}
    }
  return ctype;
}

void
PL_reg_ctype_bitwidth (PL_Operand op, L_Operand * oper, int type, int unsign)
{
  int ctype;

  if(op->type == PL_R) {
    int bitwidth;
    int stored_bw;
    int save;

    // ugly temporary hack because Pcode doesnt generate
    // host_layoutinfo.md anymore
    save = M_use_layout_database;
    M_use_layout_database = 0;

    bitwidth = M_type_size(type);  

    // reset parameter
    M_use_layout_database = save;
        
    if(op->data_type == 8) 
      ctype = PL_mtype2ctype(3, op->unsign);
    else
      ctype = PL_mtype2ctype(op->data_type, op->unsign);  
    
    stored_bw = (int) HashTable_find_or_null(operandHash, (int) oper->value.r);

    if(!stored_bw) 
      HashTable_insert (operandHash, (int) oper->value.r, (int*) bitwidth);
    else if(bitwidth != stored_bw) {      
      if(bitwidth > stored_bw) {
	HashTable_remove (operandHash, (int) oper->value.r);
	HashTable_insert (operandHash, (int) oper->value.r, (int*) bitwidth);
      }
      //      L_punt("two incompatible bitwidths for register: %d -- %d and %d\n", 
      //	     oper->value.r, bitwidth, stored_bw);    
    }
  }
  
  return;
}

int
PL_parm_ctype (int type, int unsign)
{
  int ctype = 0;

  switch (type)
    {
    case M_TYPE_FLOAT:
      ctype = L_CTYPE_FLOAT;
      break;
    case M_TYPE_DOUBLE:
      ctype = L_CTYPE_DOUBLE;
      break;
    case M_TYPE_CHAR:
      ctype = unsign ? L_CTYPE_UCHAR : L_CTYPE_CHAR;
      break;
    case M_TYPE_SHORT:
      ctype = unsign ? L_CTYPE_USHORT : L_CTYPE_SHORT;
      break;
    case M_TYPE_INT:
      ctype = unsign ? L_CTYPE_UINT : L_CTYPE_INT;
      break;
    case M_TYPE_LONG:
      ctype = unsign ? L_CTYPE_ULONG : L_CTYPE_LONG;
      break;
    case M_TYPE_LLONG:
      ctype = unsign ? L_CTYPE_ULLONG : L_CTYPE_LLONG;
      break;
    case M_TYPE_POINTER:
      ctype = L_CTYPE_POINTER;
      break;
    case M_TYPE_UNION:
    case M_TYPE_STRUCT:
      ctype = PL_native_int_reg_ctype;
      break;
    default:
      P_punt ("PL_parm_ctype : unknown MTYPE %d", type);
      break;
    }

  return ctype;
}


/*----------------------------------------------------------------------*/


/*
 *	For all PL_gen_XXX functions,
 *	we require each operand field to be unique.
 *	So we can modify them when necessary.
 *	This is incoherent to gen_func.c ... etc, need to fix
 *	in the future.
 */
/*----------------------------------------------------------------------*/
/*
 *	preserve type as much as possible,
 *	but make non-scalar types scalar (at least
 *	when they are used in a register -- address --)
 */
static int
PL_reg_mtype (int type)
{
  switch (type)
    {
    case M_TYPE_CHAR:
    case M_TYPE_BIT_CHAR:
      return M_TYPE_CHAR;
    case M_TYPE_SHORT:
    case M_TYPE_BIT_SHORT:
      return M_TYPE_SHORT;
    case M_TYPE_INT:
      return M_TYPE_INT;
    case M_TYPE_BIT_LONG:
    case M_TYPE_LONG:
      return M_TYPE_LONG;
    case M_TYPE_BIT_LLONG:
    case M_TYPE_LLONG:
      return M_TYPE_LLONG;
    case M_TYPE_FLOAT:
    case M_TYPE_DOUBLE:
      return type;
    case M_TYPE_POINTER:
    case M_TYPE_BLOCK:
    case M_TYPE_UNION:
    case M_TYPE_STRUCT:
      return M_TYPE_POINTER;	/* address */
    case M_TYPE_VOID:
      P_punt ("PL_reg_mtype: VOID is not allowed in register");
      return 0;
    default:
      P_punt ("PL_reg_mtype: illegal type");
      return 0;
    }
}


/* PL_gen_cast
 * ----------------------------------------------------------------------
 * JWS 20011215 - replaces cast_gen_*
 *
 * BCC - inside bcc_cast_XXXX, don't use any functions that will call 
 * bcc_cast_XXXX in order to avoid indefinite cast - 6/17/95
 */

void
PL_gen_cast (L_Cb * cb, PL_Operand dest, PL_Operand src, int cast_opc)
{
  L_Oper *new_oper;

  if (dest == src)
    P_punt ("PL_gen_cast: arguments must be unique");

  new_oper = PL_new_loper (NULL, cast_opc);
  new_oper->dest[0] = PL_gen_operand (dest);
  new_oper->src[0] = PL_gen_operand (src);
  L_insert_oper_after (cb, cb->last_op, new_oper);

  if (cast_opc == Lop_I_F || cast_opc == Lop_I_F2)
    new_oper->com[0] = PL_mtype2ctype (src->data_type, src->unsign);
  else if (cast_opc == Lop_F_I || cast_opc == Lop_F2_I)
    new_oper->com[0] = PL_mtype2ctype (dest->data_type, dest->unsign);

  return;
}


/*----------------------------------------------------------------------*/
/*
 *	This function does not provide enough capability
 *	to handle all function calls. We use this only
 *	to generate calls to internal functions.
 *	Do not allow structure arguments.
 *      Added return_mtype -ITI/JCG 4/99
 */
static void
PL_gen_expand (L_Cb * cb, char *fn, _PL_Operand arg[],
	       _M_Type ret_mtype, _M_Type mtype[], int argc,
	       L_Attr * attr1, Expr expr)
{
  int i;
  _PL_Operand fn_addr, dest, src1, src2, src3;
  _M_Type mtype2[PL_MAX_PARAM_VAR];
  long offset[PL_MAX_PARAM_VAR];
  int mode[PL_MAX_PARAM_VAR];
  int reg[PL_MAX_PARAM_VAR];
  int paddr[PL_MAX_PARAM_VAR];
  int su_sreg[PL_MAX_PARAM_VAR];
  int su_ereg[PL_MAX_PARAM_VAR];
  int pcount;
/******    variables added for X86 patch  *******/
  _M_Type local_mtype[PL_MAX_PARAM_VAR];
  _M_Type rev_type2[PL_MAX_PARAM_VAR];
  long rev_offset[PL_MAX_PARAM_VAR];
  int rev_mode[PL_MAX_PARAM_VAR];
  int rev_reg[PL_MAX_PARAM_VAR];
  int rev_paddr[PL_MAX_PARAM_VAR];
  int num;
/***********************************************/

  int param_size;
  int tmcount = 0;
  char *base_macro;
  L_Attr *attr = NULL, *new_attr = NULL;

  attr = L_concat_attr (attr1, attr);
  for (i = 0; i < argc; i++)
    {
      mtype2[i] = mtype[i];
    }
  /*
   *  For passing arguments to callee functions.
   *  However, here, we do not allow passing structure
   *  arguments. 
   */
  param_size = M_fnvar_layout (argc, mtype2, offset, mode, reg, paddr,
			       &base_macro, su_sreg, su_ereg,
			       &pcount, 0, M_PUT_FNVAR);
  for (i = 0; i < argc; i++)
    {
      offset[i] /= P_CHAR_SIZE;
      paddr[i] /= P_CHAR_SIZE;
    }

  if (M_arch != M_X86)
    {
      for (i = 0; i < argc; i++)
	local_mtype[i] = mtype[i];
    }
  else
    {
      /* Patch for X86 - reverses the order of all function
         parameters, so they will be generated right-to-left, per X86
         standard. */

      num = argc;

      for (i = 0; i < num; i++)
	{
	  rev_type2[i] = mtype2[i];
	  rev_mode[i] = mode[i];
	  rev_reg[i] = reg[i];
	  rev_offset[i] = offset[i];
	  rev_paddr[i] = paddr[i];
	}

      for (i = 0; i < num; i++)
	{
	  local_mtype[num - 1 - i] = mtype[i];
	  mtype2[num - 1 - i] = rev_type2[i];
	  mode[num - 1 - i] = rev_mode[i];
	  reg[num - 1 - i] = rev_reg[i];
	  offset[num - 1 - i] = rev_offset[i];
	  paddr[num - 1 - i] = rev_paddr[i];
	}
    }

/*******************************************************/

  for (i = 0; i < argc; i++)
    {
      int argn = (M_arch != M_X86) ? i : (argc - 1 - i);

      if (mode[i] == M_THRU_REGISTER)
	{
	  char line[64];
	  sprintf (line, "$P%d", reg[i]);

	  PL_new_macro (&dest, line, mtype2[i].type, mtype2[i].unsign);

	  PL_gen_mov (cb, &dest, arg + argn, L_copy_attr (attr));
	}
      else if (mode[i] == M_THRU_MEMORY)
	{
	  if ((local_mtype[i].type == M_TYPE_UNION) ||
	      (local_mtype[i].type == M_TYPE_STRUCT))
	    P_punt ("PL_gen_expand: do not allow structure parameters");

	  if (local_mtype[i].type != mtype2[i].type)
	    {
	      PL_new_register (&src3, PL_next_reg_id (), mtype2[i].type,
			       mtype2[i].unsign);

	      PL_gen_mov (cb, &src3, arg + argn, L_copy_attr (attr));
	    }
	  else
	    {
	      src3 = arg[argn];
	    }
	  PL_new_macro (&src1, base_macro, M_TYPE_POINTER, 0);
	  PL_new_int (&src2, offset[i], 0);
	  new_attr = L_new_attr ("tm", 1);
	  L_set_int_attr_field (new_attr, 0, L_TM_START_VALUE + tmcount++);
	  new_attr = L_concat_attr (new_attr, L_copy_attr (attr));
	  PL_gen_store (cb, expr, &src1, &src2, &src3, src3.data_type,
			new_attr);
	}
      else
	{
	  fprintf (stderr, "# indirect memory fnvar mode is used\n");
	  P_punt ("PL_gen_expand: do not allow structure parameters");
	}
    }

  PL_new_label (&fn_addr, fn, 1);

  /* REH 3/7/93 add attributes to jsr to identify parameters used */
  /* ITI/JCG 4/99 Enhanced attributes to match normal jsr attributes */
  {
    L_Attr *func_attr = NULL;
    L_Attr *tr_attr = NULL;
    L_Attr *tm_attr = NULL;
    L_Attr *tmo_attr = NULL;
    L_Attr *tmso_attr = NULL;
    L_Attr *count_attr = NULL;
    int trcount = 0;
    tmcount = 0;
    new_attr = NULL;
    for (i = 0; i < argc; i++)
      {
	if ((mode[i] == M_THRU_REGISTER) ||
	    (mode[i] == M_INDIRECT_THRU_REGISTER))
	  {
	    if (!trcount)
	      tr_attr = L_new_attr ("tr", 0);

	    L_set_macro_attr_field (tr_attr, trcount++, L_MAC_P0 + reg[i],
				    PL_parm_ctype (mtype2[i].type,
						   mtype2[i].unsign),
				    L_PTYPE_NULL);

	    /* Also, add structure offsets for indirect thru
	     * register -ITI/JCG 4/99
	     */
	    if (mode[i] == M_INDIRECT_THRU_REGISTER)
	      {
		if (!tmso_attr)
		  tmso_attr = L_new_attr ("tmso", 0);

		L_set_int_attr_field (tmso_attr, i, paddr[i]);
	      }
	  }
	else
	  {
	    if (!tmcount)
	      {
		tm_attr = L_new_attr ("tm", 0);
		tmo_attr = L_new_attr ("tmo", 0);
	      }
	    L_set_int_attr_field (tm_attr, tmcount,
				  L_TM_START_VALUE + tmcount);


	    /* To make Lemulate robust, explicitly specify memory offsets
	     * for thru-memory parameters on JSRs.  -ITI/JCG 4/99
	     */
	    L_set_int_attr_field (tmo_attr, tmcount, offset[i]);

	    /* Also, add structure offsets for indirect thru
	     * memory -ITI/JCG 4/99
	     */
	    if (mode[i] == M_INDIRECT_THRU_MEMORY)
	      {
		if (!tmso_attr)
		  tmso_attr = L_new_attr ("tmso", 0);

		L_set_int_attr_field (tmso_attr, i, paddr[i]);
	      }
	    tmcount++;
	  }
      }
    if (tr_attr != NULL)
      func_attr = L_concat_attr (func_attr, tr_attr);
    if (tm_attr != NULL)
      func_attr = L_concat_attr (func_attr, tm_attr);
    if (tmo_attr != NULL)
      func_attr = L_concat_attr (func_attr, tmo_attr);
    if (tmso_attr != NULL)
      func_attr = L_concat_attr (func_attr, tmso_attr);
    count_attr = L_new_attr ("op", 0);
    L_set_int_attr_field (count_attr, 0, pcount);
    func_attr = L_concat_attr (func_attr, count_attr);

    /*
     *  ignore the return value. (NO LONGER! -ITI/JCG 4/99)
     *  Although, for now, will punt if return a structure (not
     *  expected to happen). -ITI/JCG 4/99
     */
    /* If the function returns a structure, punt for now! */
    if ((ret_mtype.type == M_TYPE_UNION) || (ret_mtype.type == M_TYPE_STRUCT))
      P_punt ("PL_gen_expand: returning structure expected and "
	      "unimplemented!");

    /* Provide the return register macro name */
    new_attr = L_new_attr ("ret", 1);

    L_set_macro_attr_field (new_attr, 0,
			    L_MAC_P0 + M_return_register (ret_mtype.type,
							  M_GET_FNVAR),
			    PL_parm_ctype (ret_mtype.type, ret_mtype.unsign),
			    L_PTYPE_NULL);

    func_attr = L_concat_attr (func_attr, new_attr);


    new_attr = L_new_attr ("param_size", 1);
    L_set_int_attr_field (new_attr, 0, param_size / P_CHAR_SIZE);
    func_attr = L_concat_attr (func_attr, new_attr);
    func_attr = L_concat_attr (func_attr, L_copy_attr (attr));
    PL_gen_jsr (cb, expr->parentexpr, &(fn_addr), argc, func_attr);
    /*EMN*/
  }

  if (attr)
    L_delete_all_attr (attr);

  return;
}

/*----------------------------------------------------------------------*/

void
PL_new_cb (PL_Operand op, int cb_id)
{
  op->type = PL_CB;
  op->data_type = M_TYPE_INT;	/* branch offset */
  op->unsign = 0;
  op->value.cb = cb_id;
}

void
PL_new_register (PL_Operand op, int reg_id, int type, int unsign)
{
  type = PL_reg_mtype (type);
  op->type = PL_R;
  op->data_type = type;
  op->unsign = unsign;
  op->value.r = reg_id;
  if (!PLM_valid_register_type (type))
    {
      fprintf (stderr, "# type = %d\n", type);
      P_punt ("PL_new_register: illegal type");
    }
}

char *
PL_mangle_name(char *label, Key key)
{
  char *retstr = NULL;
  char *buffer;

  buffer = malloc(strlen(label) + 100);

  /* Mangle static variable names */
  sprintf(buffer, "%s_%d_%d", label, 
	  key.file,
	  key.sym);
  
  retstr = C_findstr (buffer);
  free(buffer);

  return retstr;
}

char *
PL_fmt_var_name(char *label, Key var_key)
{
  char *retstr = NULL;
  int mangle = 0;
  
  if (P_ValidKey(var_key))
    {
      VarDcl vardcl;
      FuncDcl fndcl;

      if ((vardcl = PST_GetVarDclEntry(PL_symtab, var_key)))
	{
	  if (P_GetVarDclQualifier(vardcl) & VQ_STATIC)
	    mangle = 1;
	}
      else if ((fndcl = PST_GetFuncDclEntry(PL_symtab, var_key)))
	{
	  if (P_GetFuncDclQualifier(fndcl) & VQ_STATIC)
	    mangle = 1;
	}
      else
	assert(0);
    }
  
  if (PL_MANGLE_NAMES && mangle)
    retstr = PL_mangle_name(label, var_key);
  else
    retstr = C_findstr (label);

  return retstr;
}

void
PL_new_label (PL_Operand op, char *label, int is_func)
{
  op->type = PL_L;
  op->data_type = M_TYPE_POINTER;
  op->unsign = 0;
  op->value.l = C_findstr (label);
  op->is_func_label = is_func;
}

void
PL_new_macro (PL_Operand op, char *name, int type, int unsign)
{
  type = PL_MType_Convert(type);
  op->type = PL_MAC;
  op->data_type = type;
  op->unsign = unsign;
  op->value.mac = C_findstr (name);
  if (!PLM_valid_register_type (type))
    {
      fprintf (stderr, "# type = %d\n", type);
      P_punt ("PL_new_macro: illegal type");
    }
}

void
PL_new_int_const (PL_Operand op, ITintmax value, int type, int unsign)
{
  op->type = PL_I;
  op->data_type = type;
  op->unsign = unsign;
  op->value.i = value;
  return;
}

void
PL_new_pointer (PL_Operand op, ITintmax value)
{
  op->type = PL_I;
  op->data_type = M_TYPE_POINTER;
  op->unsign = 0;
  op->value.i = value;
}

void
PL_new_float (PL_Operand op, double value)
{
  op->type = PL_F;
  op->data_type = M_TYPE_FLOAT;
  op->unsign = 0;
  op->value.f = value;
}

void
PL_new_double (PL_Operand op, double value)
{
  op->type = PL_F2;
  op->data_type = M_TYPE_DOUBLE;
  op->unsign = 0;
  op->value.f2 = value;
}

void
PL_new_string (PL_Operand op, char *str)
{
  /* JWS 20020228 -- generate string table for each file */
  PL_new_label (op, PL_get_string_label (str), 0);
}

/*----------------------------------------------------------------------*/

int
is_integer (int type)
{
  switch (type)
    {
    case M_TYPE_BIT_LLONG:
    case M_TYPE_BIT_LONG:
    case M_TYPE_BIT_INT:
    case M_TYPE_BIT_SHORT:
    case M_TYPE_BIT_CHAR:
    case M_TYPE_CHAR:
    case M_TYPE_SHORT:
    case M_TYPE_INT:
    case M_TYPE_LONG:
    case M_TYPE_LLONG:
    case M_TYPE_POINTER:
    case M_TYPE_BLOCK:		/* starting address of a block */
      return 1;
    default:
      return 0;
    }
}


int
is_float (int type)
{
  return (type == M_TYPE_FLOAT);
}


int
is_double (int type)
{
  return (type == M_TYPE_DOUBLE);
}


int
default_unary_promotion (int type)
{
  int prom_type = M_TYPE_INT;

  if (type == M_TYPE_DOUBLE)
    prom_type = M_TYPE_DOUBLE;
  else if (type == M_TYPE_FLOAT)
    prom_type = M_TYPE_FLOAT;
  else if (type == M_TYPE_LLONG)
    prom_type = M_TYPE_LLONG;
  else if (type == M_TYPE_POINTER)
    prom_type = M_TYPE_POINTER;
  else if (type == M_TYPE_BLOCK)
    prom_type = M_TYPE_BLOCK;
  else if (type == M_TYPE_LONG)
    prom_type = M_TYPE_LONG;

  return prom_type;
}


void
iso_default_promotion (M_Type rtype,
		       int type1, int unsign1, int type2, int unsign2)
{
  int ptype = M_TYPE_INT, punsign = 0;

  if (!rtype)
    P_punt ("iso_default_promotion: destination type unspecified");

  if ((type1 == M_TYPE_DOUBLE) || (type2 == M_TYPE_DOUBLE))
    ptype = M_TYPE_DOUBLE;
  else if ((type1 == M_TYPE_FLOAT) || (type2 == M_TYPE_FLOAT))
    ptype = M_TYPE_FLOAT;
  else if ((type1 == M_TYPE_LLONG) || (type2 == M_TYPE_LLONG))
    ptype = M_TYPE_LLONG;
  else if ((type1 == M_TYPE_POINTER) || (type2 == M_TYPE_POINTER))
    ptype = M_TYPE_POINTER;
  else if ((type1 == M_TYPE_BLOCK) || (type2 == M_TYPE_BLOCK))
    ptype = M_TYPE_BLOCK;
  else if ((type1 == M_TYPE_LONG) || (type2 == M_TYPE_LONG))
    ptype = M_TYPE_LONG;

  if ((unsign1 && (PL_MType_Size(ptype) == PL_MType_Size(type1))) ||
      (unsign2 && (PL_MType_Size(ptype) == PL_MType_Size(type2))))
    punsign = 1;

  rtype->type = ptype;
  rtype->unsign = punsign;

  return;
}


int
eval_mtype (int type)
{
  switch (type)
    {
    case M_TYPE_FLOAT:
      return M_TYPE_FLOAT;
    case M_TYPE_DOUBLE:
      return M_TYPE_DOUBLE;
    case M_TYPE_VOID:
    case M_TYPE_BIT_INT:
    case M_TYPE_BIT_SHORT:
    case M_TYPE_BIT_CHAR:
    case M_TYPE_CHAR:
    case M_TYPE_SHORT:
    case M_TYPE_INT:
      return M_TYPE_INT;
    case M_TYPE_LONG:
    case M_TYPE_BIT_LONG:
      return (P_LONG_SIZE == 64) ? M_TYPE_LLONG : M_TYPE_INT;
    case M_TYPE_POINTER:
    case M_TYPE_UNION:
    case M_TYPE_STRUCT:
    case M_TYPE_BLOCK:
      return (P_POINTER_SIZE == 64) ? M_TYPE_LLONG : M_TYPE_INT;
    case M_TYPE_LLONG:
    case M_TYPE_BIT_LLONG:
      return M_TYPE_LLONG;
    default:
      return PL_native_int_reg_mtype;
    }
}


ITintmax
PL_int_native_value (PL_Operand op)
{
  ITintmax val = 0;

  if (op->type != PL_I)
    P_punt ("PL_int_native_value: called on op other than int const");

  if (op->unsign)
    {
      val = ((ITintmax) (op->value.i));
    }
  else
    {
      switch (op->data_type)
	{
	case M_TYPE_CHAR:
	  val = (ITintmax) ((char) op->value.i);
	  break;
	case M_TYPE_SHORT:
	  val = (ITintmax) ((short) op->value.i);
	  break;
	case M_TYPE_INT:
	  val = (ITintmax) ((int) op->value.i);
	  break;
	case M_TYPE_LONG:
	  if (PL_native_int_reg_ctype == L_CTYPE_INT)
	    val = (ITintmax) ((int) op->value.i);
	  else if (PL_native_int_reg_ctype == L_CTYPE_LLONG)
	    val = op->value.i;
	  break;
	case M_TYPE_LLONG:
	  val = op->value.i;
	  break;
	case M_TYPE_POINTER:
#if 1
	  val = op->value.i;
#else
	  P_punt ("PL_int_native_value: signed pointer?");
#endif
	  break;
	default:
	  P_punt ("PL_int_native_value: unknown M_type %d", op->data_type);
	  break;
	}
    }
  return val;
}


L_Operand *
PL_gen_operand (PL_Operand op)
{
  L_Operand *new_operand = NULL;
  L_Cb *cb;
  char name[256];
  int ctype, mac_num;

  if (!op)
    P_punt ("PL_gen_operand: no argument");

  switch (op->type)
    {
    case PL_CB:
      cb = L_cb_hash_tbl_find_and_alloc (L_fn->cb_hash_tbl, op->value.cb);
      new_operand = L_new_cb_operand (cb);
      break;
    case PL_L:
      sprintf (name, "_%s", PL_M_fn_label_name (op->value.l, op->is_func_label));
      new_operand = L_new_gen_label_operand (name);
      break;
    case PL_I:
      if (M_arch == M_TAHOE)
	{
	  /* cast (zxt, sxt) constants */
	  new_operand = L_new_int_operand (PL_int_native_value (op),
					   PL_native_int_reg_ctype);
	}
      else
	{
	  ctype = PL_reg_ctype (op->data_type, op->unsign);
	  if (!ctype)
	    L_warn ("PL_Operand : unknown M_type %d", op->data_type);
	  new_operand = L_new_int_operand (op->value.i, ctype);
	}
      break;
    case PL_S:
      new_operand = L_new_gen_string_operand (op->value.s);
      break;
    case PL_F:
      /* BCC - use float type argument - 8/22/96 */
      new_operand = L_new_float_operand ((float) op->value.f);
      break;
    case PL_F2:
      new_operand = L_new_double_operand (op->value.f2);
      break;
    case PL_R:
      ctype = PL_reg_ctype (op->data_type, op->unsign);
      new_operand = L_new_register_operand (op->value.r, ctype, L_PTYPE_NULL);
      if(PL_annotate_bitwidths && M_arch == M_PLAYDOH)
	PL_reg_ctype_bitwidth(op, new_operand, op->data_type, op->unsign);
      break;
    case PL_MAC:
      ctype = PL_reg_ctype (op->data_type, op->unsign);
      mac_num = L_macro_id (op->value.mac);
      new_operand = L_new_macro_operand (mac_num, ctype, L_PTYPE_NULL);
      break;
    default:
      P_punt ("PL_gen_operand: illegal type");
    }
  return (new_operand);
}


L_Operand *
PL_gen_cast_operand (L_Cb * cb, PL_Operand src)
{
  L_Operand *ret_opd;
  _PL_Operand dest;
  int opc = 0;

  switch (src->type)
    {
    case PL_R:
    case PL_I:
    case PL_MAC:
      switch (src->data_type)
	{
	case M_TYPE_CHAR:
	case M_TYPE_BIT_CHAR:
	  opc = src->unsign ? Lop_ZXT_C : Lop_SXT_C;
	  break;
	case M_TYPE_SHORT:
	case M_TYPE_BIT_SHORT:
	  opc = src->unsign ? Lop_ZXT_C2 : Lop_SXT_C2;
	  break;
	case M_TYPE_INT:
	case M_TYPE_BIT_INT:
	  if (PL_native_int_reg_mtype != M_TYPE_INT)
	    opc = src->unsign ? Lop_ZXT_I : Lop_SXT_I;
	  break;
	case M_TYPE_LONG:
	case M_TYPE_BIT_LONG:
	  if (PL_native_int_size != P_LONG_SIZE &&
	      P_LONG_SIZE == P_INT_SIZE)
	    opc = src->unsign ? Lop_ZXT_I : Lop_SXT_I;
	  break;
	case M_TYPE_VOID:
	case M_TYPE_FLOAT:
	case M_TYPE_DOUBLE:
	case M_TYPE_LLONG:
	case M_TYPE_BIT_LLONG:
	case M_TYPE_POINTER:
	case M_TYPE_UNION:
	case M_TYPE_STRUCT:
	case M_TYPE_BLOCK:
	  break;
	default:
	  L_warn ("PL_gen_cast_operand : unknown M type %d",
		  src->data_type);
	  break;
	}
      break;
    case PL_CB:
    case PL_L:
    case PL_F:
    case PL_F2:
    case PL_S:
      break;
    default:
      L_warn ("PL_gen_cast_operand : unknown HC type %d", src->type);
      break;
    }

  if (opc)
    {
      PL_new_register (&dest, PL_next_reg_id (),
		       src->data_type, src->unsign);
      PL_gen_cast (cb, &dest, src, opc);
      src = &dest;
    }

  ret_opd = PL_gen_operand (src);

  return ret_opd;
}

/* BCC - added - 6/15/95 */
static void bcc_cast_char (L_Cb *, PL_Operand, PL_Operand, int);
static void bcc_cast_short (L_Cb *, PL_Operand, PL_Operand, int);
static void bcc_cast_int (L_Cb *, PL_Operand, PL_Operand, int, int);
static void bcc_cast_float (L_Cb *, PL_Operand, PL_Operand, int);
static void bcc_cast_double (L_Cb *, PL_Operand, PL_Operand, int);

void
PL_cast (L_Cb * cb, PL_Operand dest, PL_Operand src, int type, int unsign)
{
  switch (type)
    {
    case M_TYPE_CHAR:
      bcc_cast_char (cb, dest, src, unsign);
      break;
    case M_TYPE_SHORT:
      bcc_cast_short (cb, dest, src, unsign);
      break;
    case M_TYPE_INT:
    case M_TYPE_LONG:
    case M_TYPE_LLONG:
    case M_TYPE_POINTER:
      bcc_cast_int (cb, dest, src, type, unsign);
      break;
    case M_TYPE_FLOAT:
      bcc_cast_float (cb, dest, src, unsign);
      break;
    case M_TYPE_DOUBLE:
      bcc_cast_double (cb, dest, src, unsign);
      break;
    default:
      P_punt ("Illegal dest type %d for PL_cast", type);
      break;
    }
  return;
}

#define P_VAL_SIZE (8 * sizeof (ITintmax))

static void
bcc_cast_char (L_Cb * cb, PL_Operand dest, PL_Operand src, int unsign)
{
  ITintmax value;
  _PL_Operand src1;

  src1 = *src;
  switch (src1.type)
    {
    case PL_CB:
    case PL_L:
    case PL_S:
      /* BCC - Remove the restriction, but this is a bug, because
         upper 24 bits are not chopped - 6/27/95 P_punt("Illegal cast
         to char"); */
      *dest = src1;
      break;
    case PL_I:
      value = src1.value.i;
      value <<= P_VAL_SIZE - P_CHAR_SIZE;
      value >>= P_VAL_SIZE - P_CHAR_SIZE;
      if (unsign)
	value &= P_UNSIGNED_CHAR_MASK;
      PL_new_char (dest, value, unsign);
      break;
    case PL_F:
      value = (long) src1.value.f;
      value <<= P_VAL_SIZE - P_CHAR_SIZE;
      value >>= P_VAL_SIZE - P_CHAR_SIZE;
      if (unsign)
	value &= P_UNSIGNED_CHAR_MASK;
      PL_new_char (dest, value, unsign);
      break;
    case PL_F2:
      value = (long) src1.value.f2;
      value <<= P_VAL_SIZE - P_CHAR_SIZE;
      value >>= P_VAL_SIZE - P_CHAR_SIZE;
      if (unsign)
	value &= P_UNSIGNED_CHAR_MASK;
      PL_new_char (dest, value, unsign);
      break;
    case PL_R:
    case PL_MAC:
      {
	_PL_Operand temp;

	if (is_float (src1.data_type))
	  {
	    PL_new_register (&temp, PL_next_reg_id (), M_TYPE_INT, 0);
	    PL_gen_cast (cb, &temp, &src1, Lop_F_I);
	    src1 = temp;
	  }
	else if (is_double (src1.data_type))
	  {
	    PL_new_register (&temp, PL_next_reg_id (), M_TYPE_INT, 0);
	    PL_gen_cast (cb, &temp, &src1, Lop_F2_I);
	    src1 = temp;
	  }

	PL_gen_cast (cb, dest, &src1, unsign ? Lop_ZXT_C : Lop_SXT_C);
      }
      break;
    default:
      P_punt ("cast_char: Incorrect src data type");
      break;
    }
}


static void
bcc_cast_short (L_Cb * cb, PL_Operand dest, PL_Operand src, int unsign)
{
  ITintmax value;
  _PL_Operand src1;

  src1 = *src;
  switch (src1.type)
    {
    case PL_CB:
    case PL_L:
    case PL_S:
      /* BCC - Remove the restriction, but this is a bug, because
         upper 16 bits are not chopped - 6/27/95 P_punt("Illegal cast
         to short"); */
      *dest = src1;
      break;
    case PL_I:
      value = src1.value.i;
      value <<= P_VAL_SIZE - P_SHORT_SIZE;
      value >>= P_VAL_SIZE - P_SHORT_SIZE;
      if (unsign)
	value &= P_UNSIGNED_SHORT_MASK;
      PL_new_short (dest, value, unsign);
      break;
    case PL_F:
      value = (long) src1.value.f;
      value <<= P_VAL_SIZE - P_SHORT_SIZE;
      value >>= P_VAL_SIZE - P_SHORT_SIZE;
      if (unsign)
	value &= P_UNSIGNED_SHORT_MASK;
      PL_new_short (dest, value, unsign);
      break;
    case PL_F2:
      value = (long) src1.value.f2;
      value <<= P_VAL_SIZE - P_SHORT_SIZE;
      value >>= P_VAL_SIZE - P_SHORT_SIZE;
      if (unsign)
	value &= P_UNSIGNED_SHORT_MASK;
      PL_new_short (dest, value, unsign);
      break;
    case PL_R:
    case PL_MAC:
      {
	_PL_Operand temp;	/* , offset, mask; CWL */

	if (is_float (src1.data_type))
	  {
	    PL_new_register (&temp, PL_next_reg_id (), M_TYPE_INT, 0);
	    PL_gen_cast (cb, &temp, &src1, Lop_F_I);
	    src1 = temp;
	  }
	else if (is_double (src1.data_type))
	  {
	    PL_new_register (&temp, PL_next_reg_id (), M_TYPE_INT, 0);
	    PL_gen_cast (cb, &temp, &src1, Lop_F2_I);
	    src1 = temp;
	  }

	PL_gen_cast (cb, dest, &src1, unsign ? Lop_ZXT_C2 : Lop_SXT_C2);
      }
      break;
    default:
      P_punt ("cast_short: Incorrect src data type");
      break;
    }
}


static void
bcc_cast_int (L_Cb * cb, PL_Operand dest, PL_Operand src, int type,
	      int unsign)
{
  ITintmax value;
  _PL_Operand src1;

  src1 = *src;
  switch (src1.type)
    {
    case PL_CB:
    case PL_L:
    case PL_S:
      /* BCC - Remove the restriction - 6/27/95 
         P_punt("Illegal cast to int"); */
    case PL_I:
      /* JWS -- this is not quite correct */
      *dest = src1;
      break;
    case PL_F:
      value = (long) src1.value.f;
      PL_new_int_const (dest, value, type, unsign);
      break;
    case PL_F2:
      value = (long) src1.value.f2;
      PL_new_int_const (dest, value, type, unsign);
      break;
    case PL_R:
    case PL_MAC:
      {
	if (is_float (src1.data_type))
	  {
	    PL_gen_cast (cb, dest, &src1, Lop_F_I);
	  }
	else if (is_double (src1.data_type))
	  {
	    PL_gen_cast (cb, dest, &src1, Lop_F2_I);
	  }
	else if (is_integer (src1.data_type))
	  {
	    if (PL_MType_Size(src1.data_type) > PL_MType_Size(type))
	      {
		if (PL_MType_Size(type) != PL_MType_Size(M_TYPE_INT))
		  P_punt ("bcc_cast_int: Invalid destination type\n");

		PL_gen_cast (cb, dest, &src1, unsign ? Lop_ZXT_I : Lop_SXT_I);
	      }
            else if (dest->data_type == M_TYPE_LLONG)
              {
                PL_gen_cast (cb, dest, &src1, unsign ? Lop_ZXT_I : Lop_SXT_I);
              }
	    else
	      {
		*dest = src1;
	      }

	    dest->data_type = type;
	    dest->unsign = unsign;
	  }
	else
	  {
	    P_punt ("Illegal cast to int from register");
	  }
      }
      break;
    default:
      P_punt ("cast_int: Incorrect src data type");
      break;
    }
}


static void
bcc_cast_float (L_Cb * cb, PL_Operand dest, PL_Operand src, int unsign)
{
  _PL_Operand src1;

  if (dest->data_type != M_TYPE_FLOAT)
    P_punt ("cast_float: Incorrect dest data type");
  src1 = *src;
  switch (src1.type)
    {
    case PL_CB:
    case PL_L:
    case PL_S:
      P_punt ("Illegal cast to float");
    case PL_I:
      PL_new_float (dest, (double) src1.value.i);
      break;
    case PL_F:
      *dest = src1;
      break;
    case PL_F2:
      PL_new_float (dest, (double) src1.value.f2);
      break;
    case PL_R:
    case PL_MAC:
      {
	if (is_double (src1.data_type))
	  PL_gen_cast (cb, dest, &src1, Lop_F2_F);
	else if (is_integer (src1.data_type))
	  PL_gen_cast (cb, dest, &src1, Lop_I_F);
	else if (is_float (src1.data_type))
	  *dest = src1;
	else
	  P_punt ("Illegal cast to float from register");
      }
      break;
    default:
      P_punt ("cast_float: Incorrect src data type");
      break;
    }
}


static void
bcc_cast_double (L_Cb * cb, PL_Operand dest, PL_Operand src, int unsign)
{
  _PL_Operand src1;

  if (dest->data_type != M_TYPE_DOUBLE)
    P_punt ("cast_double: Incorrect dest data type");
  src1 = *src;
  switch (src1.type)
    {
    case PL_CB:
    case PL_L:
    case PL_S:
      P_punt ("Illegal cast to double");
    case PL_I:
      PL_new_double (dest, (double) src1.value.i);
      break;
    case PL_F:
      PL_new_double (dest, (double) src1.value.f);
      break;
    case PL_F2:
      *dest = src1;
      break;
    case PL_R:
    case PL_MAC:
      if (is_float (src1.data_type))
	PL_gen_cast (cb, dest, &src1, Lop_F_F2);
      else if (is_integer (src1.data_type))
	PL_gen_cast (cb, dest, &src1, Lop_I_F2);
      else if (is_double (src1.data_type))
	*dest = src1;
      else
	P_punt ("Illegal cast to double from register");
      break;
    default:
      P_punt ("cast_double: Incorrect src data type");
      break;
    }
}


void
PL_gen_block_mov_attr (Expr expr, L_Oper * oper, int size, int pos)
{
  L_Attr *attr_list = NULL, *old_attr;
  L_Attr *comp_attr, *attr;
  int f;
  char *index, *lbname, *lbbase, *lbext;
  char *lbnew;
  int offset;
  long int integer;
  double real;
  char *pragmastr, *delim;
  Pragma ptr;

  if (!expr)
    return;

  for (ptr = expr->pragma; ptr != NULL; ptr = ptr->next)
    {
      char *buffer = NULL;

      pragmastr = PL_read_attr_name_from_pragma_str (ptr->specifier, &buffer);
      if ((!strncmp (buffer, "ACC_NAME", 8) && PL_generate_acc_name_attrs) ||
	  (!strcmp (buffer, "COMPREUSE")))
	{
	  /* printf("%s, ",buffer); */
	  if (!(old_attr = L_find_attr (attr_list, buffer)))
	    {
	      attr = L_new_attr (buffer, 0);
	      f = 0;
	    }
	  else
	    {
	      attr = old_attr;
	      f = attr->max_field;
	    }

	  while (pragmastr)
	    {
	      char *field_buffer = NULL;

	      pragmastr =
		PL_read_attr_field_from_pragma_str (pragmastr, &field_buffer,
						    &integer, &real, &delim);
	      switch (delim[1])
		{
		case '$':
		  L_set_string_attr_field (attr, f, field_buffer);
		  break;
		case '!':
		  L_set_label_attr_field (attr, f, field_buffer);
		  break;
		case '%':
		  L_set_int_attr_field (attr, f, integer);
		  break;
		case '#':
		  L_set_double_attr_field (attr, f, real);
		  break;
		default:
		  P_punt
		    ("PL_gen_block_mov_attr: unknown pragma delimiter type");
		}

	      free(field_buffer);
	      f++;
	    }
	  if (!old_attr)
	    attr_list = L_concat_attr (attr_list, attr);
	}

      free(buffer);
    }
  /* printf("\n"); */

  if (!attr_list)
    return;

  L_concat_attr (oper->attr, attr_list);
  if (!(comp_attr = L_find_attr (attr_list, "COMPREUSE")))
    return;

  for (f = 0; f < comp_attr->max_field; f++)
    {
      lbname = comp_attr->field[f]->value.l;
      lbbase = (char *) malloc (sizeof (char) * (strlen (lbname) + 1));
      lbbase[0] = 0;
      lbext = (char *) malloc (sizeof (char) * (strlen (lbname) + 1));
      lbext[0] = 0;

      /* printf("lbname[%s]\n",lbname); */
      index = strrchr (lbname, (int) ('.'));
      if (!index)
	{
	  strcpy (lbbase, lbname);
	  lbbase[strlen (lbbase) - 1] = '\0';
	  offset = 0;
	}
      else
	{
	  strncpy (lbbase, lbname, (int) (index - lbname));
	  lbbase[index - lbname] = '\0';
	  strcpy (lbext, index + 1);
	  sscanf (lbext, "%d", &offset);
	}
      /*
         printf("lbbase[%s] lbext[%s] offset[%d] size[%d] pos[%d]\n",
         lbbase, lbext, offset, size, pos);
       */

      lbnew = (char *) malloc (sizeof (char) * (strlen (lbbase) + 1000));
      sprintf (lbnew, "%s.%d_%d\"", lbbase, (offset + pos * size),
	       (offset + pos * size + size - 1));
      /* comp_attr->field[f]->value.l = lbnew; */
      L_set_string_attr_field (comp_attr, f, lbnew);

      /* printf("newname[%s]\n",lbnew); */

      free (lbnew);
      free (lbbase);
      free (lbext);
    }
  return;
}


void
PL_gen_block_mov (L_Cb * cb, Expr expr, PL_Operand dest, int dest_offset,
		  PL_Operand src, int src_offset, 
		  int type_size,
		  int type_align,
		  L_Attr * attr1, int sreg, int ereg, int load_from_reg)
{
  int size = 0;
  int align = 0;
  int copy_size = 0;
  int copy_mtype = 0;
  int N = 0;
  int i = 0;
  int LoadOffset = 0;
  int StoreOffset = 0;
  _PL_Operand dest_addr, src_addr, offset;
  L_Attr *store_stack_attr = NULL;
  L_Attr *load_stack_attr = NULL;
  L_Attr *tmp_attr = NULL;

  /* When structs are passed through registers, then
     some or all of the loads in a gen_block_mov
     can be skipped. Get first data from 
     sreg ... ereg and the remainer from memory */
  int current_reg = 0;;

  current_reg = sreg;
  size = type_size;
  align = type_align;
  if (align == 0)
    P_punt ("PL_gen_block_mov: align cannot be 0");
  if ((size % align) != 0)
    P_punt ("PL_gen_block_mov: size must be a multiple of align");

  /*
   *  compute base address.
   **************************************************/
  if (dest_offset == 0)
    {
      /* register already holds base address */
      dest_addr = *dest;
    }
  else
    {
      /* add offset to address to get actual base address */
      PL_new_int (&offset, dest_offset, 0);
      PL_new_register (&dest_addr, PL_next_reg_id (), M_TYPE_POINTER, 0);
      PL_gen_add (cb, &dest_addr, dest, &offset, 1, L_copy_attr (attr1));
      store_stack_attr = PL_create_stack_ref_attr (cb->last_op);
    }

  if (src_offset == 0)
    {
      /* register already holds base address */
      src_addr = *src;
    }
  else
    {
      /* add offset to address to get actual base address */
      PL_new_int (&offset, src_offset, 0);
      PL_new_register (&src_addr, PL_next_reg_id (), M_TYPE_POINTER, 0);
      PL_gen_add (cb, &src_addr, src, &offset, 1, L_copy_attr (attr1));
      load_stack_attr = PL_create_stack_ref_attr (cb->last_op);
    }

  /*
   *  generate copy code.
   **************************************************/
#define PL_UNWRAP_LIMIT	64

  /* Determine the best size at which to copy */
  if (align >= (PL_native_int_align / P_CHAR_SIZE))
    {
      /*
       * OBJECT ALIGNMENT IS MORE THAN (OR SAME) AS NATIVE REG
       *  - Use maximum ld size
       */
      copy_size = (PL_native_int_align / P_CHAR_SIZE);

      if ((align % copy_size) != 0)
	{
	  fprintf (stderr, "size=%d, align=%d\n", size, align);
	  P_punt ("PL_gen_block_mov: bad align");
	}
    }
  else
    {
      /*
       * OBJECT ALIGNMENT IS LESS THAN NATIVE REG
       *  - Use alignment as size
       */
      copy_size = align;
    }

  /* Convert this size to an MTYPE */
  switch (copy_size)
    {
    case 8:
      copy_mtype = M_TYPE_LLONG;
      break;
    case 4:
      copy_mtype = M_TYPE_INT;
      break;
    case 2:
      copy_mtype = M_TYPE_SHORT;
      break;
    case 1:
      copy_mtype = M_TYPE_CHAR;
      break;
    default:
      P_punt ("Block move: invalid copy_size\n");
      break;
    }

  /* Generate the actual set of ld/st for copying */
  N = size / copy_size;
  if (N <= PL_UNWRAP_LIMIT)
    {
      _PL_Operand temp;
      _PL_Operand dest, src1, src2, src3;
      L_Attr *block_move_attr;

      block_move_attr = L_new_attr ("block_move", 0);
      block_move_attr = L_concat_attr (block_move_attr, L_copy_attr (attr1));

      if (M_arch != M_X86)
	{
	  LoadOffset = 0;
	  StoreOffset = 0;
	}
      else if (load_from_reg)
	{
	  LoadOffset = ((N - i - 1) - (ereg - sreg)) * copy_size;
	  StoreOffset = (N - i - 1) * copy_size;
	}
      else
	{
	  LoadOffset = (N - i - 1) * copy_size;
	  StoreOffset = ((N - i - 1) - (ereg - sreg)) * copy_size;
	}

      for (i = 0; i < N; i++)
	{
	  Expr nsexpr;

	  /* THE LOAD */
	  PL_new_register (&temp, PL_next_reg_id (), copy_mtype, 0);
	  dest = temp;
	  src1 = src_addr;

	  /* DMG - Patch for X86 - need to reverse order in which
	     structure elements are moved, so that for moves
	     which are function parameter passing, the push
	     opers are laid out in correct order  */

	  PL_new_int (&src2, LoadOffset, 0);

	  /* EMN 10/7/99 - COMPREUSE, ACC_NAME flags need 
	   * to be propagated if it
	   * existed on expr. Added  PL_gen_block_mov_attr. */

	  /* SAM 6/94 - load expr is really the sibling if it exists */

	  /* CWL - 12/2001 for P-to-L.
	   * In Hcode, you can get expr's sibling using expr->sibling.
	   * In Pcode, this is not true for arguments of function call
	   * and for comma expression. The Pcode structures for function-call
	   * and comma expressions are like:
	   *
	   *   expr->opcode==OP_call:
	   *       {expr->operands}                           function name
	   *       {"("}
	   *       {expr->operands->sibling} {","}            1st argument
	   *       {expr->operands->sibling->next} {","}      2nd argument
	   *       ...                                        3rd ...
	   *       {")"}
	   *
	   *   expr->opcode==OP_compexpr:
	   *       {"("} {expr->operands}                     recursion
	   *       [{","} {expr->operands->next}]             last expr
	   *       {")"}
	   *
	   * So if expr->sibling is NIL, its "sibling" may be in expr->next.
	   */

	  if (expr && ((nsexpr = expr->sibling) || (nsexpr = expr->next)))
	    {
	      if (load_from_reg && current_reg < ereg)
		{
		  /* Get value from current_reg instead */
		  printf ("Load: Using register %d instead \n", current_reg);
		  PL_new_register (&temp, current_reg,
				   PL_native_int_reg_mtype, 0);
		  PL_gen_mov (cb, &dest, &temp,
			      L_copy_attr (block_move_attr));
		  current_reg++;
		}
	      else
		{
		  L_Oper *op;
		  tmp_attr = NULL;
		  if (load_stack_attr)
		    {
		      tmp_attr = L_copy_attr (load_stack_attr);
		      tmp_attr->field[1]->value.i += LoadOffset;
		    }
		  tmp_attr =
		    L_concat_attr (tmp_attr, L_copy_attr (block_move_attr));
		  op = PL_gen_load (cb, nsexpr,
				    &dest, &src1, &src2, 1, tmp_attr);
		  if (load_stack_attr)
		    {
		      op->flags = L_SET_BIT_FLAG (op->flags, L_OPER_SAFE_PEI);
		      op->flags = L_SET_BIT_FLAG (op->flags,
						  L_OPER_STACK_REFERENCE);
		    }
		  LoadOffset += copy_size;

		  PL_gen_block_mov_attr (nsexpr, op, copy_size, i);
		}
	    }
	  else
	    {
	      if (load_from_reg && current_reg < ereg)
		{
		  /* Get value from current_reg instead */
		  printf ("Load: Using register %d instead \n", current_reg);
		  PL_new_register (&temp, current_reg,
				   PL_native_int_reg_mtype, 0);
		  PL_gen_mov (cb, &dest, &temp,
			      L_copy_attr (block_move_attr));
		  current_reg++;
		}
	      else
		{
		  L_Oper *op;
		  tmp_attr = NULL;
		  if (load_stack_attr)
		    {
		      tmp_attr = L_copy_attr (load_stack_attr);
		      tmp_attr->field[1]->value.i += LoadOffset;
		    }
		  tmp_attr =
		    L_concat_attr (tmp_attr, L_copy_attr (block_move_attr));
		  op = PL_gen_load (cb, expr, &dest, &src1, &src2, 1, 
				    tmp_attr);
		  if (load_stack_attr)
		    {
		      op->flags = L_SET_BIT_FLAG (op->flags, 
						  L_OPER_SAFE_PEI);
		      op->flags = L_SET_BIT_FLAG (op->flags,
						  L_OPER_STACK_REFERENCE);
		    }
		  LoadOffset += copy_size;
		  PL_gen_block_mov_attr (expr, op, copy_size, i);
		}
	    }

	  /* THE STORE */
	  src1 = dest_addr;
	  PL_new_int (&src2, StoreOffset, 0);
	  src3 = dest;
	  if (!load_from_reg && current_reg < ereg)
	    {
	      /*printf("Store: Using register %d instead \n",current_reg); */
	      PL_new_register (&temp, current_reg, PL_native_int_reg_mtype,
			       0);
	      PL_gen_mov (cb, &temp, &src3, L_copy_attr (block_move_attr));
	      current_reg++;
	    }
	  else
	    {
	      L_Oper *op;
	      tmp_attr = NULL;
	      if (store_stack_attr)
		{
		  tmp_attr = L_copy_attr (store_stack_attr);
		  tmp_attr->field[1]->value.i += StoreOffset;
		}
	      tmp_attr =
		L_concat_attr (tmp_attr, L_copy_attr (block_move_attr));
	      op = PL_gen_store (cb, expr, &src1, &src2, &src3, src3.data_type,
				 tmp_attr);
	      if (store_stack_attr)
		{
		  op->flags = L_SET_BIT_FLAG (op->flags, L_OPER_SAFE_PEI);
		  op->flags = L_SET_BIT_FLAG (op->flags,
					      L_OPER_STACK_REFERENCE);
		}
	      StoreOffset += copy_size;
	      PL_gen_block_mov_attr (expr, op, copy_size, i);
	    }
	}

      if (block_move_attr)
	L_delete_all_attr (block_move_attr);
    }
  else
    {
      L_Attr *call_info_attr = NULL;
      _PL_Operand arg[4];
      _M_Type type[4];
      _M_Type ret_type;

      P_warn ("PL_gen_block_mov: generating a call to memcpy()");

      /* Call memcpy(dest_addr, src_addr, length). -JCG 4/30/98 */

      arg[0] = dest_addr;
      PL_M_pointer (type + 0);
      arg[1] = src_addr;
      PL_M_pointer (type + 1);
      PL_new_int (arg + 2, size, 0);
      PL_M_int (type + 2, 0);
      PL_M_pointer (&ret_type);

      /*EMN*/
      PL_gen_expand (cb, "memcpy", arg, ret_type,
		     type, 3, L_copy_attr(attr1), expr);

      /* SAM 12-04: We need to do some minor cleanup here before adding the new call_info
         attribute.  The memcpy jsr may have an old call_info attribute on it, so delete
         that first. Note I assume cb->last_op is always the right handle to the op inserted
         by the call to PL_gen_expand above. */
      call_info_attr = L_find_attr(cb->last_op->attr, "call_info");
      if (call_info_attr != NULL)
        cb->last_op->attr = L_delete_attr(cb->last_op->attr, call_info_attr);
      /* Add call_info attribute to attr1 -JCG 5/24/98 */
      call_info_attr = L_new_attr ("call_info", 1);
      L_set_string_attr_field (call_info_attr, 0,
			       "\"void+P%void+P%void+P%int\"");
      cb->last_op->attr = L_concat_attr (cb->last_op->attr, call_info_attr);
    }

  if (attr1)
    L_delete_all_attr (attr1);
  if (load_stack_attr)
    L_delete_all_attr (load_stack_attr);
  if (store_stack_attr)
    L_delete_all_attr (store_stack_attr);
}


void
PL_gen_mov (L_Cb * cb, PL_Operand dest, PL_Operand src, L_Attr * attr)
{
  int opc = 0;
  ITuint8 com = 0;
  L_Oper *new_oper;

  if (dest == src)
    P_punt ("PL_gen_mov: arguments must be unique");

  if (is_integer (dest->data_type))
    {
      if (is_float (src->data_type))
	{
	  opc = Lop_F_I;
	  com = PL_mtype2ctype (dest->data_type, dest->unsign);
	}
      else if (is_double (src->data_type))
	{
	  opc = Lop_F2_I;
	  com = PL_mtype2ctype (dest->data_type, dest->unsign);
	}
      else
	opc = Lop_MOV;
    }
  else if (is_float (dest->data_type))
    {
      if (is_integer (src->data_type))
	{
	  opc = Lop_I_F;
	  com = PL_mtype2ctype (src->data_type, src->unsign);
	}
      else if (is_double (src->data_type))
	opc = Lop_F2_F;
      else
	opc = Lop_MOV_F;
    }
  else if (is_double (dest->data_type))
    {
      if (is_integer (src->data_type))
	{
	  opc = Lop_I_F2;
	  com = PL_mtype2ctype (src->data_type, src->unsign);
	}
      else if (is_float (src->data_type))
	opc = Lop_F_F2;
      else
	opc = Lop_MOV_F2;
    }
  else
    {
      P_punt ("PL_gen_mov: illegal arguments");
    }

  new_oper = PL_new_loper (NULL, opc);
  new_oper->dest[0] = PL_gen_operand (dest);
  new_oper->src[0] = PL_gen_operand (src);
  new_oper->com[0] = com;

  if (attr)
    new_oper->attr = L_concat_attr (new_oper->attr, attr);

  L_insert_oper_after (cb, cb->last_op, new_oper);

  return;
}


/*
 *	For memory accesses insert the name of the array/var that
 *	is being accessed to assist Lcode memory disambiguation
 */
static void
PL_gen_label_attr (Expr expr, L_Oper * oper)
{
  char buf[128];
  L_Attr *attr;
  Expr operands;

  if (expr == NULL || expr == ((void *) (-1)))
    return;

  if (expr->opcode == OP_var)
    {
      sprintf (buf, "lab_%s", expr->value.var.name);
      attr = L_new_attr (buf, 0);
      oper->attr = L_concat_attr (oper->attr, attr);
    }
  else if ((expr->opcode == OP_index) ||
	   (expr->opcode == OP_indr))
    {
      operands = expr->operands;
      if (operands->opcode == OP_var)
	{
	  sprintf (buf, "lab_%s", operands->value.var.name);
	  attr = L_new_attr (buf, 0);
	  oper->attr = L_concat_attr (oper->attr, attr);
        }
    }

  return;
}


static int
PL_is_stack_ref (L_Oper * oper)
{
  if (!(M_is_stack_operand (oper->src[0]) ||
	M_is_stack_operand (oper->src[1])))
    return 0;

  if (L_is_int_constant (oper->src[0]) || L_is_int_constant (oper->src[1]))
    return 1;
  else
    return 0;
}


static L_Attr *
PL_create_stack_ref_attr (L_Oper * oper)
{
  L_Attr *stack_attr;

  stack_attr = L_new_attr (STACK_ATTR_NAME, 2);

  /* always store the base first */
  if (M_is_stack_operand (oper->src[0]))
    {
      if (!L_is_int_constant (oper->src[1]))
	{
	  L_delete_all_attr (stack_attr);
	  return NULL;
	}
      stack_attr->field[0] = L_copy_operand (oper->src[0]);
      stack_attr->field[1] = L_copy_operand (oper->src[1]);
    }
  else if (M_is_stack_operand (oper->src[1]))
    {
      if (!L_is_int_constant (oper->src[0]))
	{
	  L_delete_all_attr (stack_attr);
	  return NULL;
	}
      stack_attr->field[0] = L_copy_operand (oper->src[1]);
      stack_attr->field[1] = L_copy_operand (oper->src[0]);
    }
  else
    {
      L_delete_all_attr (stack_attr);
      return NULL;
    }

  return stack_attr;
}


void
PL_mark_as_stack_ref (L_Oper * oper)
{
  L_Attr *stack_attr;

  oper->flags = L_SET_BIT_FLAG (oper->flags, L_OPER_SAFE_PEI);
  oper->flags = L_SET_BIT_FLAG (oper->flags, L_OPER_STACK_REFERENCE);
  stack_attr = L_new_attr (STACK_ATTR_NAME, 2);

  /* always store the base first */
  if (M_is_stack_operand (oper->src[0]))
    {
      if (!L_is_int_constant (oper->src[1]))
	P_punt ("PL_mark_as_stack_ref: oper %d src1 not int const\n",
		oper->id);
      stack_attr->field[0] = L_copy_operand (oper->src[0]);
      stack_attr->field[1] = L_copy_operand (oper->src[1]);
    }
  else if (M_is_stack_operand (oper->src[1]))
    {
      if (!L_is_int_constant (oper->src[0]))
	P_punt ("PL_mark_as_stack_ref: oper %d src0 not int const\n",
		oper->id);
      stack_attr->field[0] = L_copy_operand (oper->src[1]);
      stack_attr->field[1] = L_copy_operand (oper->src[0]);
    }
  else
    {
      DB_print_oper (oper);
      P_punt ("PL_mark_as_stack_ref: oper %d does not have a stack offset",
	      oper->id);
    }

  oper->attr = L_concat_attr (oper->attr, stack_attr);
}


L_Oper *
PL_gen_load (L_Cb * cb, Expr expr, PL_Operand dest, PL_Operand src1,
	     PL_Operand src2, int unsign, L_Attr * attr1)
{
  int opc = 0;
  L_Oper *new_oper;

  if ((dest == src1) || (dest == src2) || (src1 == src2))
    P_punt ("PL_gen_load: arguments must be unique");

  if (!is_integer (src1->data_type) || !is_integer (src2->data_type))
    P_punt ("Non-int operands for PL_gen_load");

  switch (dest->data_type)
    {
    case M_TYPE_CHAR:
      opc = unsign ? Lop_LD_UC : Lop_LD_C;
      break;
    case M_TYPE_SHORT:
      opc = unsign ? Lop_LD_UC2 : Lop_LD_C2;
      break;
    case M_TYPE_INT:
      opc = ((M_arch == M_TAHOE) && unsign) ? Lop_LD_UI : Lop_LD_I;
      break;
    case M_TYPE_LONG:
      if (P_LONG_SIZE == 64)
	opc = Lop_LD_Q;
      else if (P_LONG_SIZE == 32)
	{
	  if ((M_arch == M_TAHOE) && unsign)
	    opc = Lop_LD_UI;
	  else
	    opc = Lop_LD_I;
	}
      else
	P_punt ("PL_gen_load: unusual pointer size");
      break;
    case M_TYPE_POINTER:
      if (P_POINTER_SIZE == 64)
	opc = Lop_LD_Q;
      else if (P_POINTER_SIZE == 32)
	{
	  if ((M_arch == M_TAHOE) && unsign)
	    opc = Lop_LD_UI;
	  else
	    opc = Lop_LD_I;
	}
      else
	P_punt ("PL_gen_load: unusual pointer size");
      break;
    case M_TYPE_LLONG:
      opc = Lop_LD_Q;
      break;
    case M_TYPE_FLOAT:
      opc = Lop_LD_F;
      break;
    case M_TYPE_DOUBLE:
      opc = Lop_LD_F2;
      break;
    default:
      P_punt ("PL_gen_load: illegal data type");
    }

  new_oper = PL_new_loper (expr, opc);
  new_oper->dest[0] = PL_gen_operand (dest);
  new_oper->src[0] = PL_gen_operand (src1);
  new_oper->src[1] = PL_gen_operand (src2);

  if (PL_generate_label_attrs)
    PL_gen_label_attr (expr, new_oper);

  if (PL_is_stack_ref (new_oper))
    PL_mark_as_stack_ref (new_oper);

  if (attr1)
    new_oper->attr = L_concat_attr (new_oper->attr, attr1);

  L_insert_oper_after (cb, cb->last_op, new_oper);
  return new_oper;
}


L_Oper *
PL_gen_store (L_Cb * cb, Expr expr, PL_Operand src1, PL_Operand src2,
	      PL_Operand src3, int data_type, L_Attr * attr1)
{
  int opc = 0;
  L_Oper *new_oper;
  _PL_Operand temp;

  if ((src3 == src1) || (src3 == src2) || (src1 == src2))
    P_punt ("PL_gen_store: arguments must be unique");
  if (!is_integer (src1->data_type) || !is_integer (src2->data_type))
    P_punt ("Non-int operands for PL_gen_store");

  switch (data_type)
    {
    case M_TYPE_CHAR:
      opc = Lop_ST_C;
      break;
    case M_TYPE_SHORT:
      opc = Lop_ST_C2;
      break;
    case M_TYPE_POINTER:
      if (P_POINTER_SIZE == 64)
	opc = Lop_ST_Q;
      else if (P_POINTER_SIZE == 32)
	opc = Lop_ST_I;
      else
	P_punt ("PL_gen_store: unusual pointer size");
      break;
    case M_TYPE_LONG:
      if (P_LONG_SIZE == 64)
	opc = Lop_ST_Q;
      else if (P_LONG_SIZE == 32)
	opc = Lop_ST_I;
      else
	P_punt ("PL_gen_load: unusual pointer size");
      break;
    case M_TYPE_LLONG:
      opc = Lop_ST_Q;
      break;
    case M_TYPE_INT:
      opc = Lop_ST_I;
      break;
    case M_TYPE_FLOAT:
      opc = Lop_ST_F;

      if (is_double (src3->data_type))
	{
	  PL_new_register (&temp, PL_next_reg_id (), M_TYPE_FLOAT, 0);
	  PL_cast (cb, &temp, src3, M_TYPE_FLOAT, 0);
	  src3 = &temp;
	}
      break;
    case M_TYPE_DOUBLE:
      opc = Lop_ST_F2;

      if (is_float (src3->data_type))
	{
	  PL_new_register (&temp, PL_next_reg_id (), M_TYPE_DOUBLE, 0);
	  PL_cast (cb, &temp, src3, M_TYPE_DOUBLE, 0);
	  src3 = &temp;
	}
      break;
    default:
      P_punt ("PL_gen_store: illegal data type");
    }

  new_oper = PL_new_loper (expr, opc);
  new_oper->src[0] = PL_gen_operand (src1);
  new_oper->src[1] = PL_gen_operand (src2);
  new_oper->src[2] = PL_gen_operand (src3);

  if (PL_generate_label_attrs)
    PL_gen_label_attr (expr, new_oper);

  if (PL_is_stack_ref (new_oper))
    PL_mark_as_stack_ref (new_oper);

  if (attr1)
    new_oper->attr = L_concat_attr (new_oper->attr, attr1);

  L_insert_oper_after (cb, cb->last_op, new_oper);
  return new_oper;
}


static int
PL_cast_equivalent (int type1, int uns1, int type2, int uns2)
{
  int int1, int2;

  if ((type1 == type2) && (uns1 == uns2))
    return 1;

  int1 = is_integer (type1);
  int2 = is_integer (type2);

  if (int1 ^ int2)
    return 0;

  if (int1)
    {
      if (PL_MType_Size(type1) != PL_MType_Size(type2))
	return 0;
      if (uns1 != uns2)
	return 0;
    }
  else
    {
      if (type1 != type2)
	return 0;
    }

  return 1;
}


static int
PL_setup_binary_operands (L_Cb * cb,
			  PL_Operand dest, PL_Operand src1, PL_Operand src2,
			  PL_Operand tdest, PL_Operand tsrc1,
			  PL_Operand tsrc2, M_Type cmtype)
{
  int typeC, unsC, implicit_cast = 0;

  if ((dest == src1) || (dest == src2) || (src1 == src2))
    P_punt ("PL_setup_binary_operands: arguments must be unique");

  iso_default_promotion (cmtype,
			 src1->data_type, src1->unsign,
			 src2->data_type, src2->unsign);

  typeC = eval_mtype (cmtype->type);
  unsC = cmtype->unsign;

  if (PL_cast_equivalent (src1->data_type, src1->unsign, typeC, unsC))
    {
      *tsrc1 = *src1;
    }
  else
    {
      PL_new_register (tsrc1, PL_next_reg_id (), typeC, unsC);
      PL_cast (cb, tsrc1, src1, typeC, unsC);
    }

  if (PL_cast_equivalent (src2->data_type, src2->unsign, typeC, unsC))
    {
      *tsrc2 = *src2;
    }
  else
    {
      PL_new_register (tsrc2, PL_next_reg_id (), typeC, unsC);
      PL_cast (cb, tsrc2, src2, typeC, unsC);
    }

  if (PL_cast_equivalent (dest->data_type, dest->unsign, typeC, unsC))
    {
      *tdest = *dest;
    }
  else
    {
      PL_new_register (tdest, PL_next_reg_id (), typeC, unsC);
      implicit_cast = 1;
    }

  return implicit_cast;
}


static L_Oper *
PL_finish_binary_op (L_Cb * cb, int opc, PL_Operand tdest, PL_Operand fdest,
		     PL_Operand src1, PL_Operand src2, int implicit_cast,
		     L_Attr * attr)
{
  L_Oper *new_oper;

  new_oper = PL_new_loper (NULL, opc);
  new_oper->dest[0] = PL_gen_operand (tdest);
  new_oper->src[0] = PL_gen_operand (src1);
  new_oper->src[1] = PL_gen_operand (src2);

  L_insert_oper_after (cb, cb->last_op, new_oper);

  if (attr)
    new_oper->attr = L_concat_attr (new_oper->attr, attr);

  if (implicit_cast)
    PL_cast (cb, fdest, tdest, fdest->data_type, fdest->unsign);

  return new_oper;
}


void
PL_gen_add (L_Cb * cb, PL_Operand dest, PL_Operand src1, PL_Operand src2,
	    int unsign, L_Attr * attr1)
{
  int opc;
  _PL_Operand Src1, Src2, Dest;
  _M_Type mtype;
  int implicit_cast;

  implicit_cast = PL_setup_binary_operands (cb, dest, src1, src2,
					    &Dest, &Src1, &Src2, &mtype);

  switch (mtype.type)
    {
    case M_TYPE_DOUBLE:
      opc = Lop_ADD_F2;
      break;
    case M_TYPE_FLOAT:
      opc = Lop_ADD_F;
      break;
    default:
      opc = unsign ? Lop_ADD_U : Lop_ADD;
      break;
    }

  PL_finish_binary_op (cb, opc, &Dest, dest, &Src1, &Src2,
		       implicit_cast, attr1);

  return;
}


void
PL_gen_sub (L_Cb * cb, PL_Operand dest, PL_Operand src1,
	    PL_Operand src2, int unsign, L_Attr * attr1)
{
  int opc;
  L_Oper *new_oper;
  _PL_Operand Src1, Src2, Dest;
  int implicit_cast = 0;

  if ((dest == src1) || (dest == src2) || (src1 == src2))
    P_punt ("PL_gen_sub: arguments must be unique");

  if (is_double (src1->data_type) || is_double (src2->data_type))
    {
      if (is_double (src1->data_type))
	{
	  Src1 = *src1;
	}
      else
	{
	  PL_new_register (&Src1, PL_next_reg_id (), M_TYPE_DOUBLE, 0);
	  PL_cast (cb, &Src1, src1, M_TYPE_DOUBLE, 0);
	}
      if (is_double (src2->data_type))
	{
	  Src2 = *src2;
	}
      else
	{
	  PL_new_register (&Src2, PL_next_reg_id (), M_TYPE_DOUBLE, 0);
	  PL_cast (cb, &Src2, src2, M_TYPE_DOUBLE, 0);
	}
      if (is_double (dest->data_type))
	{
	  Dest = *dest;
	}
      else
	{
	  PL_new_register (&Dest, PL_next_reg_id (), M_TYPE_DOUBLE, 0);
	  implicit_cast = 1;
	}

      opc = Lop_SUB_F2;
    }
  else if (is_float (src1->data_type) || is_float (src2->data_type))
    {
      if (is_float (src1->data_type))
	{
	  Src1 = *src1;
	}
      else
	{
	  PL_new_register (&Src1, PL_next_reg_id (), M_TYPE_FLOAT, 0);
	  PL_cast (cb, &Src1, src1, M_TYPE_FLOAT, 0);
	}

      if (is_float (src2->data_type))
	{
	  Src2 = *src2;
	}
      else
	{
	  PL_new_register (&Src2, PL_next_reg_id (), M_TYPE_FLOAT, 0);
	  PL_cast (cb, &Src2, src2, M_TYPE_FLOAT, 0);
	}

      if (is_float (dest->data_type))
	{
	  Dest = *dest;
	}
      else
	{
	  PL_new_register (&Dest, PL_next_reg_id (), M_TYPE_FLOAT, 0);
	  implicit_cast = 1;
	}

      opc = Lop_SUB_F;
    }
  else
    {
      Src1 = *src1;
      Src2 = *src2;
      switch (dest->data_type)
	{
	case M_TYPE_FLOAT:
	case M_TYPE_DOUBLE:
	  PL_new_register (&Dest, PL_next_reg_id (),
			   PL_native_int_reg_mtype, 0);
	  implicit_cast = 1;
	  break;
	case M_TYPE_CHAR:
	case M_TYPE_SHORT:
	  PL_new_register (&Dest, PL_next_reg_id (),
			   PL_native_int_reg_mtype, 0);
	  implicit_cast = 1;
	  break;
	default:
	  Dest = *dest;
	}

      opc = unsign ? Lop_SUB_U : Lop_SUB;
    }

  new_oper = PL_new_loper (NULL, opc);
  new_oper->dest[0] = PL_gen_operand (&Dest);
  new_oper->src[0] = PL_gen_operand (&Src1);
  new_oper->src[1] = PL_gen_operand (&Src2);

  if (attr1)
    new_oper->attr = L_concat_attr (new_oper->attr, attr1);

  L_insert_oper_after (cb, cb->last_op, new_oper);

  if (implicit_cast)
    PL_cast (cb, dest, &Dest, dest->data_type, unsign);

  return;
}


void
PL_gen_mul (L_Cb * cb, PL_Operand dest, PL_Operand src1, PL_Operand src2,
	    M_Type mtype, int unsign, L_Attr * attr1)
{
  L_Oper *new_oper = NULL;
  _PL_Operand Src1, Src2, Dest;
  int implicit_cast = 0;

  if ((dest == src1) || (dest == src2) || (src1 == src2))
    P_punt ("PL_gen_mul: arguments must be unique");

  if (is_double (src1->data_type) || is_double (src2->data_type))
    {
      if (is_double (src1->data_type))
	{
	  Src1 = *src1;
	}
      else
	{
	  PL_new_register (&Src1, PL_next_reg_id (), M_TYPE_DOUBLE, 0);
	  PL_cast (cb, &Src1, src1, M_TYPE_DOUBLE, 0);
	}

      if (is_double (src2->data_type))
	{
	  Src2 = *src2;
	}
      else
	{
	  PL_new_register (&Src2, PL_next_reg_id (), M_TYPE_DOUBLE, 0);
	  PL_cast (cb, &Src2, src2, M_TYPE_DOUBLE, 0);
	}

      if (is_double (dest->data_type))
	{
	  Dest = *dest;
	}
      else
	{
	  PL_new_register (&Dest, PL_next_reg_id (), M_TYPE_DOUBLE, 0);
	  implicit_cast = 1;
	}

      new_oper = PL_new_loper (NULL, Lop_MUL_F2);
    }
  else if (is_float (src1->data_type) || is_float (src2->data_type))
    {
      if (is_float (src1->data_type))
	{
	  Src1 = *src1;
	}
      else
	{
	  PL_new_register (&Src1, PL_next_reg_id (), M_TYPE_FLOAT, 0);
	  PL_cast (cb, &Src1, src1, M_TYPE_FLOAT, 0);
	}

      if (is_float (src2->data_type))
	{
	  Src2 = *src2;
	}
      else
	{
	  PL_new_register (&Src2, PL_next_reg_id (), M_TYPE_FLOAT, 0);
	  PL_cast (cb, &Src2, src2, M_TYPE_FLOAT, 0);
	}

      if (is_float (dest->data_type))
	{
	  Dest = *dest;
	}
      else
	{
	  PL_new_register (&Dest, PL_next_reg_id (), M_TYPE_FLOAT, 0);
	  implicit_cast = 1;
	}

      new_oper = PL_new_loper (NULL, Lop_MUL_F);
    }
  else if (is_integer (src1->data_type) && is_integer (src2->data_type))
    {
      Src1 = *src1;
      Src2 = *src2;
      switch (dest->data_type)
	{
	case M_TYPE_FLOAT:
	case M_TYPE_DOUBLE:
	  PL_new_register (&Dest, PL_next_reg_id (),
			   PL_native_int_reg_mtype, 0);
	  implicit_cast = 1;
	  break;
	case M_TYPE_CHAR:
	case M_TYPE_SHORT:
	  PL_new_register (&Dest, PL_next_reg_id (),
			   PL_native_int_reg_mtype, 0);
	  implicit_cast = 1;
	  break;
	default:
	  Dest = *dest;
	}

      new_oper = PL_new_loper (NULL, unsign ? Lop_MUL_U : Lop_MUL);

      if (M_arch == M_TAHOE)
	{
	  if (mtype)
	    {
	      new_oper->com[0] = PL_mtype2ctype (mtype->type, mtype->unsign);
	    }
	  else if ((src1->data_type == M_TYPE_SHORT) &&
		   (src2->data_type == M_TYPE_SHORT) && 
		   (src1->unsign == src2->unsign))
	    {
	      new_oper->com[0] = PL_mtype2ctype (M_TYPE_SHORT, src1->unsign);
	    }
	}
    }

  new_oper->dest[0] = PL_gen_operand (&Dest);
  new_oper->src[0] = PL_gen_operand (&Src1);
  new_oper->src[1] = PL_gen_operand (&Src2);

  if (attr1)
    new_oper->attr = L_concat_attr (new_oper->attr, attr1);

  L_insert_oper_after (cb, cb->last_op, new_oper);

  if (implicit_cast)
    PL_cast (cb, dest, &Dest, dest->data_type, unsign);

  return;
}


void
PL_gen_div (L_Cb * cb, PL_Operand dest, PL_Operand src1, PL_Operand src2,
	    int mtype_type, int is_unsigned,
	    L_Attr * attr)
{
  L_Oper *new_oper = NULL;
  _PL_Operand Src1, Src2, Dest;
  int implicit_cast = 0;

  if ((dest == src1) || (dest == src2) || (src1 == src2))
    P_punt ("PL_gen_div: arguments must be unique");

  if (is_double (src1->data_type) || is_double (src2->data_type))
    {
      if (is_double (src1->data_type))
	{
	  Src1 = *src1;
	}
      else
	{
	  PL_new_register (&Src1, PL_next_reg_id (), M_TYPE_DOUBLE, 0);
	  PL_cast (cb, &Src1, src1, M_TYPE_DOUBLE, 0);
	}

      if (is_double (src2->data_type))
	{
	  Src2 = *src2;
	}
      else
	{
	  PL_new_register (&Src2, PL_next_reg_id (), M_TYPE_DOUBLE, 0);
	  PL_cast (cb, &Src2, src2, M_TYPE_DOUBLE, 0);
	}

      if (is_double (dest->data_type))
	{
	  Dest = *dest;
	}
      else
	{
	  PL_new_register (&Dest, PL_next_reg_id (), M_TYPE_DOUBLE, 0);
	  implicit_cast = 1;
	}

      if ((PL_use_subroutine_call) && (M_subroutine_call (Lop_DIV_F2)))
	{
	  /* Support for machine software emulation of Lcode operations */
	  new_oper = PL_gen_subroutine_call_for_operation (cb, &Dest,
							   &Src1, &Src2,
							   Lop_DIV_F2);
	}
      else
	{
	  new_oper = PL_new_loper (NULL, Lop_DIV_F2);
	  new_oper->dest[0] = PL_gen_operand (&Dest);
	  new_oper->src[0] = PL_gen_operand (&Src1);
	  new_oper->src[1] = PL_gen_operand (&Src2);
	}

    }
  else if (is_float (src1->data_type) || is_float (src2->data_type))
    {
      if (is_float (src1->data_type))
	{
	  Src1 = *src1;
	}
      else
	{
	  PL_new_register (&Src1, PL_next_reg_id (), M_TYPE_FLOAT, 0);
	  PL_cast (cb, &Src1, src1, M_TYPE_FLOAT, 0);
	}
      if (is_float (src2->data_type))
	{
	  Src2 = *src2;
	}
      else
	{
	  PL_new_register (&Src2, PL_next_reg_id (), M_TYPE_FLOAT, 0);
	  PL_cast (cb, &Src2, src2, M_TYPE_FLOAT, 0);
	}
      if (is_float (dest->data_type))
	{
	  Dest = *dest;
	}
      else
	{
	  PL_new_register (&Dest, PL_next_reg_id (), M_TYPE_FLOAT, 0);
	  implicit_cast = 1;
	}

      if ((PL_use_subroutine_call) && (M_subroutine_call (Lop_DIV_F)))
	{
	  /* Support for machine software emulation of Lcode operations */
	  new_oper = PL_gen_subroutine_call_for_operation (cb, &Dest,
							   &Src1, &Src2,
							   Lop_DIV_F);
	}
      else
	{
	  new_oper = PL_new_loper (NULL, Lop_DIV_F);
	  new_oper->dest[0] = PL_gen_operand (&Dest);
	  new_oper->src[0] = PL_gen_operand (&Src1);
	  new_oper->src[1] = PL_gen_operand (&Src2);
	}
    }
  else if (is_integer (src1->data_type) && is_integer (src2->data_type))
    {
      Src1 = *src1;
      Src2 = *src2;

      switch (dest->data_type)
	{
	case M_TYPE_FLOAT:
	case M_TYPE_DOUBLE:
	  PL_new_register (&Dest, PL_next_reg_id (),
			   PL_native_int_reg_mtype, 0);
	  implicit_cast = 1;
	  break;
	case M_TYPE_CHAR:
	case M_TYPE_SHORT:
	  PL_new_register (&Dest, PL_next_reg_id (), M_TYPE_INT, 0);
	  implicit_cast = 1;
	  break;
	default:
	  Dest = *dest;
	}
      if (((PL_use_subroutine_call) && (M_subroutine_call (Lop_DIV))) ||
	  ((PL_use_subroutine_call) && (M_subroutine_call (Lop_DIV_U))))
	{
	  /* Support for machine software emulation of Lcode operations */

	  new_oper = PL_gen_subroutine_call_for_operation (cb, &Dest,
							   &Src1, &Src2,
							   is_unsigned ? Lop_DIV_U
							   : Lop_DIV);
	}
      else
	{
	  new_oper = PL_new_loper (NULL, is_unsigned ? Lop_DIV_U : Lop_DIV);
	  new_oper->dest[0] = PL_gen_operand (&Dest);
	  new_oper->src[0] = PL_gen_cast_operand (cb, &Src1);
	  new_oper->src[1] = PL_gen_cast_operand (cb, &Src2);

	  if ((M_arch == M_TAHOE) && (mtype_type != -1))
	    new_oper->com[0] = PL_mtype2ctype (mtype_type, is_unsigned);
	}
    }
  else
    {
      P_punt ("PL_gen_div: unsupported divide functionality");
    }

  if (attr)
    new_oper->attr = L_concat_attr (new_oper->attr, attr);

  L_insert_oper_after (cb, cb->last_op, new_oper);

  if (implicit_cast)
    PL_cast (cb, dest, &Dest, dest->data_type, is_unsigned);

  return;
}


void
PL_gen_mod (L_Cb * cb, PL_Operand dest, PL_Operand src1, PL_Operand src2,
	    int mtype_type, int is_unsigned,
	    L_Attr * attr)
{
  L_Oper *new_oper = NULL;

  if ((dest == src1) || (dest == src2) || (src1 == src2))
    P_punt ("PL_gen_mod: arguments must be unique");
  if (is_float (dest->data_type) || is_double (dest->data_type))
    {
      P_punt ("PL_gen_mod: arguments must be integer type");
    }
  else
    {
      if (!is_integer (src1->data_type) || !is_integer (src2->data_type))
	P_punt ("Non-int operands for PL_gen_mod (1)");
      if (((PL_use_subroutine_call) && (M_subroutine_call (Lop_REM))) ||
	  ((PL_use_subroutine_call) && (M_subroutine_call (Lop_REM_U))))
	{
	  /* Support for machine software emulation of Lcode operations */

	  new_oper = PL_gen_subroutine_call_for_operation (cb, dest,
							   src1, src2,
							   is_unsigned ? Lop_REM_U
							   : Lop_REM);
	}
      else
	{
	  new_oper = PL_new_loper (NULL, is_unsigned ? Lop_REM_U : Lop_REM);
	  new_oper->dest[0] = PL_gen_operand (dest);
	  new_oper->src[0] = PL_gen_cast_operand (cb, src1);
	  new_oper->src[1] = PL_gen_cast_operand (cb, src2);

	  if ((M_arch == M_TAHOE) && (mtype_type != -1))
	    new_oper->com[0] = PL_mtype2ctype (mtype_type, is_unsigned);
	}
    }

  if (attr)
    new_oper->attr = L_concat_attr (new_oper->attr, attr);

  L_insert_oper_after (cb, cb->last_op, new_oper);
  return;
}


void
PL_gen_abs (L_Cb * cb, PL_Operand dest, PL_Operand src1, L_Attr * attr1)
{
  L_Oper *new_oper;

  if (dest == src1)
    P_punt ("PL_gen_abs: arguments must be unique");

  if (is_float (dest->data_type))
    {
      if (!is_float (src1->data_type))
	P_punt ("PL_gen_abs: incorrect src1 data type (float)");
      new_oper = PL_new_loper (NULL, Lop_ABS_F);
      new_oper->src[0] = PL_gen_operand (src1);
    }
  else if (is_double (dest->data_type))
    {
      if (!is_double (src1->data_type))
	P_punt ("PL_gen_abs: incorrect src1 data type (double)");
      new_oper = PL_new_loper (NULL, Lop_ABS_F2);
      new_oper->src[0] = PL_gen_operand (src1);
    }
  else
    {
      if (!is_integer (src1->data_type))
	P_punt ("PL_gen_abs: incorrect src1 data type (integer)");
      new_oper = PL_new_loper (NULL, Lop_ABS);
      new_oper->src[0] = PL_gen_cast_operand (cb, src1);
    }

  new_oper->dest[0] = PL_gen_operand (dest);

  if (attr1)
    new_oper->attr = L_concat_attr (new_oper->attr, attr1);

  L_insert_oper_after (cb, cb->last_op, new_oper);
  return;
}


void
PL_gen_logic (L_Cb * cb, int plop, PL_Operand dest, PL_Operand src1,
	      PL_Operand src2, L_Attr * attr1)
{
  L_Oper *new_oper;
  int opc = 0;

  if ((dest == src1) || (dest == src2) || (src1 == src2))
    P_punt ("PL_gen_logic: arguments must be unique");

  if (!is_integer (src1->data_type) || !is_integer (src2->data_type) ||
      !is_integer (dest->data_type))
    P_punt ("PL_gen_logic: Non-int operands");

  switch (plop)
    {
    case PL_RET_OR:
      opc = Lop_OR;
      break;
    case PL_RET_AND:
      opc = Lop_AND;
      break;
    case PL_RET_XOR:
      opc = Lop_XOR;
      break;
    default:
      P_punt ("PL_gen_logic: Invalid plop %d\n");
    }

  new_oper = PL_new_loper (NULL, opc);
  new_oper->dest[0] = PL_gen_operand (dest);
  new_oper->src[0] = PL_gen_operand (src1);
  new_oper->src[1] = PL_gen_operand (src2);

  if (attr1)
    new_oper->attr = L_concat_attr (new_oper->attr, attr1);

  L_insert_oper_after (cb, cb->last_op, new_oper);
  return;
}


void
PL_gen_lsl (L_Cb * cb, PL_Operand dest, PL_Operand src1, PL_Operand src2,
	    L_Attr * attr1)
{
  L_Oper *new_oper;

  if ((dest == src1) || (dest == src2) || (src1 == src2))
    P_punt ("PL_gen_lsl: arguments must be unique");

  if (!is_integer (src1->data_type) || !is_integer (src2->data_type) ||
      !is_integer (dest->data_type))
    P_punt ("Non-int operands for PL_gen_lsl");

  new_oper = PL_new_loper (NULL, Lop_LSL);
  new_oper->dest[0] = PL_gen_operand (dest);
  new_oper->src[0] = PL_gen_operand (src1);
  new_oper->src[1] = PL_gen_cast_operand (cb, src2);

  if (attr1)
    new_oper->attr = L_concat_attr (new_oper->attr, attr1);

  L_insert_oper_after (cb, cb->last_op, new_oper);
  return;
}


void
PL_gen_lsr (L_Cb * cb, PL_Operand dest, PL_Operand src1, PL_Operand src2,
	    L_Attr * attr1)
{
  L_Oper *new_oper;
  if ((dest == src1) || (dest == src2) || (src1 == src2))
    P_punt ("PL_gen_lsr: arguments must be unique");

  if (!is_integer (src1->data_type) || !is_integer (src2->data_type) ||
      !is_integer (dest->data_type))
    P_punt ("Non-int operands for PL_gen_lsr");

  new_oper = PL_new_loper (NULL, Lop_LSR);
  new_oper->dest[0] = PL_gen_operand (dest);
  new_oper->src[0] = PL_gen_cast_operand (cb, src1);
  new_oper->src[1] = PL_gen_cast_operand (cb, src2);

  if (!src1->unsign)
    P_warn ("PL_gen_lsr: func %s op %d had signed operand\n",
	    L_fn->name, new_oper->id);

  if (attr1)
    new_oper->attr = L_concat_attr (new_oper->attr, attr1);

  L_insert_oper_after (cb, cb->last_op, new_oper);
  return;
}


void
PL_gen_asr (L_Cb * cb, PL_Operand dest, PL_Operand src1, PL_Operand src2,
	    L_Attr * attr)
{
  L_Oper *new_oper;

  if ((dest == src1) || (dest == src2) || (src1 == src2))
    P_punt ("PL_gen_asr: arguments must be unique");

  if (!is_integer (src1->data_type) || !is_integer (src2->data_type) ||
      !is_integer (dest->data_type))
    P_punt ("Non-int operands for PL_gen_asr");

  new_oper = PL_new_loper (NULL, Lop_ASR);
  new_oper->dest[0] = PL_gen_operand (dest);
  new_oper->src[0] = PL_gen_cast_operand (cb, src1);
  new_oper->src[1] = PL_gen_cast_operand (cb, src2);

  if (attr)
    new_oper->attr = L_concat_attr (new_oper->attr, attr);

  L_insert_oper_after (cb, cb->last_op, new_oper);
  return;
}


void
PL_gen_extract_bits (L_Cb * cb, PL_Operand dest, PL_Operand src0,
		     PL_Operand src1, PL_Operand src2, int unsign)
{
  L_Oper *new_oper;

  if (!is_integer (src1->data_type) || !is_integer (src2->data_type) ||
      !is_integer (dest->data_type))
    P_punt ("Non-int operands for PL_gen_extract_bits");

  new_oper = PL_new_loper (NULL, unsign ? Lop_EXTRACT_U : Lop_EXTRACT);
  new_oper->dest[0] = PL_gen_operand (dest);
  new_oper->src[0] = PL_gen_operand (src0);
  new_oper->src[1] = PL_gen_operand (src1);
  new_oper->src[2] = PL_gen_operand (src2);

  L_insert_oper_after (cb, cb->last_op, new_oper);
  return;
}


void
PL_gen_deposit_bits (L_Cb * cb, PL_Operand dest, PL_Operand src0,
		     PL_Operand src1, PL_Operand src2, PL_Operand src3)
{
  L_Oper *new_oper;

  if (!is_integer (src1->data_type) || !is_integer (src2->data_type) ||
      !is_integer (dest->data_type))
    P_punt ("Non-int operands for PL_gen_deposit_bits");

  new_oper = PL_new_loper (NULL, Lop_BIT_DEPOSIT);
  new_oper->dest[0] = PL_gen_operand (dest);
  new_oper->src[0] = PL_gen_operand (src0);
  new_oper->src[1] = PL_gen_operand (src1);
  new_oper->src[2] = PL_gen_operand (src2);
  new_oper->src[3] = PL_gen_operand (src3);

  L_insert_oper_after (cb, cb->last_op, new_oper);
  return;
}


void
PL_gen_cbr (L_Cb *src_cb, L_Cb *dest_cb, PL_Operand op1, PL_Operand op2,
	    ITuint8 com, int unsign, L_Attr * attr)
{
  L_Oper *new_oper = NULL;

  int type = eval_mtype (op1->data_type);

  switch (type)
    {
    case M_TYPE_FLOAT:
      new_oper = PL_new_loper (NULL, Lop_BR_F);
      L_set_compare (new_oper, L_CTYPE_DOUBLE, com);
      break;
    case M_TYPE_DOUBLE:
      new_oper = PL_new_loper (NULL, Lop_BR_F);
      L_set_compare (new_oper, L_CTYPE_DOUBLE, com);
      break;
    case M_TYPE_LLONG:
      new_oper = PL_new_loper (NULL, Lop_BR);
      L_set_compare (new_oper, unsign ? L_CTYPE_ULLONG : L_CTYPE_LLONG, com);
      break;
    case M_TYPE_INT:
      new_oper = PL_new_loper (NULL, Lop_BR);
      L_set_compare (new_oper, unsign ? L_CTYPE_UINT : L_CTYPE_INT, com);
      break;
    default:
      P_punt ("gen_cbr: Bad mtype %d", type);
      break;
    }

  new_oper->src[0] = PL_gen_operand (op1);

  if (op2)
    {
      new_oper->src[1] = PL_gen_operand (op2);
    }
  else
    {
      int ctype;

      switch (type)
	{
	case M_TYPE_FLOAT:
	  new_oper->src[1] = L_new_float_operand ((float) 0.0);
	  break;
	case M_TYPE_DOUBLE:
	  new_oper->src[1] = L_new_double_operand (0.0);
	  break;

	case M_TYPE_CHAR:
	case M_TYPE_SHORT:
	case M_TYPE_INT:
	case M_TYPE_LONG:
	case M_TYPE_LLONG:
	case M_TYPE_POINTER:
	  ctype = PL_reg_ctype (type, unsign);

	  new_oper->src[1] = L_new_int_operand (0, ctype);
	  break;

	default:
	  L_warn ("gen_beq_0 : unknown mtype %d", type);
	  break;
	}
    }

  new_oper->src[2] = L_new_cb_operand (dest_cb);

  if (attr)
    new_oper->attr = L_concat_attr (new_oper->attr, attr);

  L_insert_oper_after (src_cb, src_cb->last_op, new_oper);

  return;
}


void
PL_gen_rcmp (L_Cb * cb, PL_Operand dest, PL_Operand src1, PL_Operand src2,
	     ITuint8 com, int unsign, L_Attr * attr)
{
  L_Oper *new_oper = NULL;
  _M_Type mtype;
  int type;

  iso_default_promotion (&mtype,
			 src1->data_type, src1->unsign,
			 src2->data_type, src1->unsign);

  type = eval_mtype (mtype.type);

  if ((dest == src1) || (dest == src2) || (src1 == src2))
    P_punt ("PL_gen_rcmp: arguments must be unique");

  if (dest->data_type != M_TYPE_INT)
    P_punt ("PL_gen_rcmp: dest should be of integer type");

  switch (type)
    {
    case M_TYPE_FLOAT:
      new_oper = PL_new_loper (NULL, Lop_RCMP_F);
      //L_set_compare (new_oper, L_CTYPE_DOUBLE, com);
      L_set_compare (new_oper, L_CTYPE_FLOAT, com);
      break;
    case M_TYPE_DOUBLE:
      new_oper = PL_new_loper (NULL, Lop_RCMP_F);
      L_set_compare (new_oper, L_CTYPE_DOUBLE, com);
      break;
    case M_TYPE_LLONG:
      new_oper = PL_new_loper (NULL, Lop_RCMP);
      L_set_compare (new_oper, unsign ? L_CTYPE_ULLONG : L_CTYPE_LLONG, com);
      break;
    case M_TYPE_INT:
      new_oper = PL_new_loper (NULL, Lop_RCMP);
      L_set_compare (new_oper, unsign ? L_CTYPE_UINT : L_CTYPE_INT, com);
      break;
    default:
      P_punt ("PL_gen_rcmp: Bad mtype %d", type);
      break;
    }

  new_oper->src[0] = PL_gen_operand (src1);

  if (src2)
    {
      new_oper->src[1] = PL_gen_operand (src2);
    }
  else
    {
      int ctype;

      switch (type)
	{
	case M_TYPE_FLOAT:
	  new_oper->src[1] = L_new_float_operand ((float) 0.0);
	  break;
	case M_TYPE_DOUBLE:
	  new_oper->src[1] = L_new_double_operand (0.0);
	  break;

	case M_TYPE_CHAR:
	case M_TYPE_SHORT:
	case M_TYPE_INT:
	case M_TYPE_LONG:
	case M_TYPE_LLONG:
	case M_TYPE_POINTER:
	  ctype = PL_reg_ctype (type, unsign);

	  new_oper->src[1] = L_new_int_operand (0, ctype);
	  break;

	default:
	  L_warn ("gen_beq_0 : unknown mtype %d", type);
	  break;
	}
    }

  new_oper->dest[0] = PL_gen_operand (dest);

  if (attr)
    new_oper->attr = L_concat_attr (new_oper->attr, attr);

  L_insert_oper_after (cb, cb->last_op, new_oper);
  return;
}


void
PL_gen_f2_f (L_Cb * cb, PL_Operand dest, PL_Operand src1, L_Attr * attr)
{
  L_Oper *new_oper;

  if (dest == src1)
    P_punt ("PL_gen_f2_f: arguments must be unique");

  if (!is_double (src1->data_type))
    P_punt ("PL_gen_f2_f: incorrect src type");
  if (!is_float (dest->data_type))
    P_punt ("PL_gen_f2_f: incorrect dest type");

  new_oper = PL_new_loper (NULL, Lop_F2_F);
  new_oper->dest[0] = PL_gen_operand (dest);
  new_oper->src[0] = PL_gen_operand (src1);

  if (attr)
    new_oper->attr = L_concat_attr (new_oper->attr, attr);

  L_insert_oper_after (cb, cb->last_op, new_oper);
  return;
}


void
PL_gen_i_f (L_Cb * cb, PL_Operand dest, PL_Operand src1, L_Attr * attr)
{
  L_Oper *new_oper;

  if (dest == src1)
    P_punt ("PL_gen_i_f: arguments must be unique");

  if (!is_integer (src1->data_type))
    P_punt ("PL_gen_i_f: incorrect src type");
  if (!is_float (dest->data_type))
    P_punt ("PL_gen_i_f: incorrect dest type");

  new_oper = PL_new_loper (NULL, Lop_I_F);
  new_oper->dest[0] = PL_gen_operand (dest);
  new_oper->src[0] = PL_gen_cast_operand (cb, src1);

  if (attr)
    new_oper->attr = L_concat_attr (new_oper->attr, attr);

  L_insert_oper_after (cb, cb->last_op, new_oper);
  return;
}


void
PL_gen_f_f2 (L_Cb * cb, PL_Operand dest, PL_Operand src1, L_Attr * attr)
{
  L_Oper *new_oper;

  if (dest == src1)
    P_punt ("PL_gen_f_f2: arguments must be unique");

  if (!is_float (src1->data_type))
    P_punt ("PL_gen_f_f2: incorrect src type");
  if (!is_double (dest->data_type))
    P_punt ("PL_gen_f_f2: incorrect dest type");

  new_oper = PL_new_loper (NULL, Lop_F_F2);
  new_oper->dest[0] = PL_gen_operand (dest);
  new_oper->src[0] = PL_gen_operand (src1);

  if (attr)
    new_oper->attr = L_concat_attr (new_oper->attr, attr);

  L_insert_oper_after (cb, cb->last_op, new_oper);
  return;
}


void
PL_gen_i_f2 (L_Cb * cb, PL_Operand dest, PL_Operand src1, L_Attr * attr)
{
  L_Oper *new_oper;

  if (dest == src1)
    P_punt ("PL_gen_i_f2: arguments must be unique");

  if (!is_integer (src1->data_type))
    P_punt ("PL_gen_i_f2: incorrect src type");
  if (!is_double (dest->data_type))
    P_punt ("PL_gen_i_f2: incorrect dest type");

  new_oper = PL_new_loper (NULL, Lop_I_F2);
  new_oper->dest[0] = PL_gen_operand (dest);
  new_oper->src[0] = PL_gen_cast_operand (cb, src1);

  if (attr)
    new_oper->attr = L_concat_attr (new_oper->attr, attr);

  L_insert_oper_after (cb, cb->last_op, new_oper);
  return;
}


void
PL_gen_f2_i (L_Cb * cb, PL_Operand dest, PL_Operand src1, L_Attr * attr)
{
  L_Oper *new_oper;

  if (dest == src1)
    P_punt ("PL_gen_f2_i: arguments must be unique");

  if (!is_double (src1->data_type))
    P_punt ("PL_gen_f2_i: incorrect src type");
  if (!is_integer (dest->data_type))
    P_punt ("PL_gen_f2_i: incorrect dest type");

  new_oper = PL_new_loper (NULL, Lop_F2_I);
  new_oper->dest[0] = PL_gen_operand (dest);
  new_oper->src[0] = PL_gen_operand (src1);

  if (attr)
    new_oper->attr = L_concat_attr (new_oper->attr, attr);

  L_insert_oper_after (cb, cb->last_op, new_oper);
  return;
}


void
PL_gen_f_i (L_Cb * cb, PL_Operand dest, PL_Operand src1, L_Attr * attr)
{
  L_Oper *new_oper;

  if (dest == src1)
    P_punt ("PL_gen_f_i: arguments must be unique");

  if (!is_float (src1->data_type))
    P_punt ("PL_gen_f_i: incorrect src type");
  if (!is_integer (dest->data_type))
    P_punt ("PL_gen_f_i: incorrect dest type");

  new_oper = PL_new_loper (NULL, Lop_F_I);
  new_oper->dest[0] = PL_gen_operand (dest);
  new_oper->src[0] = PL_gen_operand (src1);

  if (attr)
    new_oper->attr = L_concat_attr (new_oper->attr, attr);

  L_insert_oper_after (cb, cb->last_op, new_oper);
  return;
}


void
PL_gen_alloc (L_Cb * cb, PL_Operand dest, int size, int align)
{
  L_Oper *new_oper;

  if (dest->data_type != M_TYPE_POINTER)
    P_punt ("PL_gen_alloc: incorrect dest type");

  new_oper = PL_new_loper (NULL, Lop_ALLOC);
  new_oper->dest[0] = PL_gen_operand (dest);
  new_oper->src[0] = L_new_gen_int_operand (size);
  new_oper->src[1] = L_new_gen_int_operand (align);

  L_insert_oper_after (cb, cb->last_op, new_oper);
  return;
}


/*----------------------------------------------------------------------*/
/* THIS IS FOR THE EXCLUSIVE USE OF THE STATIC BRANCH ANALYSIS WORK */

#define FPRINTF_STDOUT 1
#define FPRINTF_STDERR 2

static int
PL_classify_fprintf (Expr expr)
{
  Expr op1, op2, iob_op, iob_op1, iob_op2;

  if (!expr)
    return (0);

  if (expr->opcode != OP_call)
    return (0);

  op1 = expr->operands;
  if (op1->opcode != OP_var)
    return (0);
  if (strcmp (op1->value.var.name, "fprintf"))
    return (0);

  op2 = expr->operands->sibling;
  if (op2->opcode == OP_addr)
    {
      iob_op = op2->operands;
      if (iob_op->opcode != OP_index)
	return (0);
    }
  else if (op2->opcode == OP_add)
    iob_op = op2;
  else
    return (0);

  iob_op1 = iob_op->operands;
  if (iob_op1->opcode != OP_var)
    return (0);
  if (strcmp (iob_op1->value.var.name, "__iob"))
    return (0);

  iob_op2 = iob_op->operands->sibling;

  if ((iob_op2->opcode != OP_int) ||
      (PST_IsUnsignedTypeExpr(PL_symtab,expr)))
    return (0);

  if (iob_op2->value.scalar == 1)
    return (FPRINTF_STDOUT);
  else if (iob_op2->value.scalar == 2)
    return (FPRINTF_STDERR);
  else
    return (0);
}


L_Oper *
PL_gen_jsr (L_Cb * cb, Expr expr, PL_Operand src, int argc, L_Attr * attr)
{
  L_Oper *new_oper;
  L_Attr *new_attr = NULL;

  new_oper = PL_new_loper (expr, Lop_JSR);
  new_oper->src[0] = PL_gen_operand (src);
  if (attr)
    new_oper->attr = attr;

  if (expr != NULL && expr->pragma != NULL)
    {
      new_attr = PL_gen_attr_from_pragma (expr->pragma);
      new_oper->attr = L_concat_attr (new_oper->attr, new_attr);
    }

  if (PL_generate_static_branch_attrs)
    {
      int is_special_fprintf = PL_classify_fprintf (expr);

      if (is_special_fprintf)
	{
	  if (is_special_fprintf == FPRINTF_STDOUT)
	    new_attr = PL_gen_attr ("fprintf_stdout", 0);
	  else if (is_special_fprintf == FPRINTF_STDERR)
	    new_attr = PL_gen_attr ("fprintf_stderr", 0);
	  else
	    P_punt ("internal error for FPRINTF handling");
	  new_oper->attr = L_concat_attr (new_oper->attr, new_attr);
	}
    }

  /* SAM 9-95, added to mark setjmp and longjmps and sync jsrs */
  if (L_op_in_synchronization_func_table (new_oper))
    new_oper->flags = L_SET_BIT_FLAG (new_oper->flags, L_OPER_SYNC);

  L_insert_oper_after (cb, cb->last_op, new_oper);
  return new_oper;
}


void
PL_gen_lcode_select_i (L_Cb * cb, PL_Operand dest,
		       _PL_Operand src[], int n_src)
{
  L_Oper *new_oper;

  if (n_src < 3)
    P_punt ("PL_gen_lcode_select_i: not enough arguments");

  if (!is_integer (src->data_type) || !is_integer ((src + 1)->data_type) ||
      !is_integer ((src + 2)->data_type))
    P_punt ("PL_gen_lcode_select_i: incorrect src types");

  new_oper = PL_new_loper (NULL, Lop_SELECT);
  new_oper->dest[0] = PL_gen_operand (dest);
  new_oper->src[0] = PL_gen_operand (src);
  new_oper->src[1] = PL_gen_operand (src + 1);
  new_oper->src[2] = PL_gen_operand (src + 2);

  L_insert_oper_after (cb, cb->last_op, new_oper);
  return;
}


void
PL_gen_lcode_select_f (L_Cb * cb, PL_Operand dest,
		       _PL_Operand src[], int n_src)
{
  L_Oper *new_oper;

  if (n_src < 3)
    P_punt ("PL_gen_lcode_select_f: not enough arguments");

  if (!is_integer (src->data_type) || !is_float ((src + 1)->data_type) ||
      !is_float ((src + 2)->data_type))
    P_punt ("PL_gen_lcode_select_f: incorrect src types");

  new_oper = PL_new_loper (NULL, Lop_SELECT_F);
  new_oper->dest[0] = PL_gen_operand (dest);
  new_oper->src[0] = PL_gen_operand (src);
  new_oper->src[1] = PL_gen_operand (src + 1);
  new_oper->src[2] = PL_gen_operand (src + 2);

  L_insert_oper_after (cb, cb->last_op, new_oper);
  return;
}


void
PL_gen_lcode_select_f2 (L_Cb * cb, PL_Operand dest,
			_PL_Operand src[], int n_src)
{
  L_Oper *new_oper;

  if (n_src < 3)
    P_punt ("PL_gen_lcode_select_f2: not enough arguments");

  if (!is_integer (src->data_type) || !is_double ((src + 1)->data_type) ||
      !is_double ((src + 2)->data_type))
    P_punt ("PL_gen_lcode_select_f2: incorrect src types");

  new_oper = PL_new_loper (NULL, Lop_SELECT_F2);
  new_oper->dest[0] = PL_gen_operand (dest);
  new_oper->src[0] = PL_gen_operand (src);
  new_oper->src[1] = PL_gen_operand (src + 1);
  new_oper->src[2] = PL_gen_operand (src + 2);

  L_insert_oper_after (cb, cb->last_op, new_oper);
  return;
}


void
PL_gen_lcode_rev (L_Cb * cb, PL_Operand dest, _PL_Operand src[], int n_src)
{
  L_Oper *new_oper;

  if (n_src < 1)
    P_punt ("PL_gen_lcode_rev: not enough arguments");

  if (!is_integer (src->data_type))
    P_punt ("PL_gen_lcode_rev: incorrect src type");

  new_oper = PL_new_loper (NULL, Lop_REV);
  new_oper->dest[0] = PL_gen_operand (dest);
  new_oper->src[0] = PL_gen_operand (src);

  L_insert_oper_after (cb, cb->last_op, new_oper);
  return;
}


void
PL_gen_lcode_bit_pos (L_Cb * cb, PL_Operand dest,
		      _PL_Operand src[], int n_src)
{
  L_Oper *new_oper;

  if (n_src < 1)
    P_punt ("PL_gen_lcode_bit_pos: not enough arguments");

  if (!is_integer (src->data_type))
    P_punt ("PL_gen_lcode_bit_pos: incorrect src type");

  new_oper = PL_new_loper (NULL, Lop_BIT_POS);
  new_oper->dest[0] = PL_gen_operand (dest);
  new_oper->src[0] = PL_gen_operand (src);

  L_insert_oper_after (cb, cb->last_op, new_oper);
  return;
}


void
PL_gen_lcode_abs_i (L_Cb * cb, PL_Operand dest, _PL_Operand src[], int n_src)
{
  L_Oper *new_oper;

  if (n_src < 1)
    P_punt ("PL_gen_lcode_abs_i: not enough arguments");

  if (!is_integer (src->data_type))
    P_punt ("PL_gen_lcode_abs_i: incorrect src type");

  new_oper = PL_new_loper (NULL, Lop_ABS);
  new_oper->dest[0] = PL_gen_operand (dest);
  new_oper->src[0] = PL_gen_operand (src);

  L_insert_oper_after (cb, cb->last_op, new_oper);
  return;
}


void
PL_gen_lcode_abs_f (L_Cb * cb, PL_Operand dest, _PL_Operand src[], int n_src)
{
  L_Oper *new_oper;

  if (n_src < 1)
    P_punt ("PL_gen_lcode_abs_f: not enough arguments");

  if (!is_float (src->data_type))
    P_punt ("PL_gen_lcode_abs_f: incorrect src type");

  new_oper = PL_new_loper (NULL, Lop_ABS_F);
  new_oper->dest[0] = PL_gen_operand (dest);
  new_oper->src[0] = PL_gen_operand (src);

  L_insert_oper_after (cb, cb->last_op, new_oper);
  return;
}


void
PL_gen_lcode_abs_f2 (L_Cb * cb, PL_Operand dest, _PL_Operand src[], int n_src)
{
  L_Oper *new_oper;

  if (n_src < 1)
    P_punt ("PL_gen_lcode_abs_f2: not enough arguments");

  if (!is_double (src->data_type))
    P_punt ("PL_gen_lcode_abs_f2: incorrect src type");

  new_oper = PL_new_loper (NULL, Lop_ABS_F2);
  new_oper->dest[0] = PL_gen_operand (dest);
  new_oper->src[0] = PL_gen_operand (src);

  L_insert_oper_after (cb, cb->last_op, new_oper);
  return;
}


void
PL_gen_lcode_sqrt_f2 (L_Cb * cb, PL_Operand dest, _PL_Operand src[], int n_src)
{
  L_Oper *new_oper;

  if (n_src < 1)
    P_punt ("PL_gen_lcode_sqrt_f2: not enough arguments");

  if (!is_double (src->data_type))
    P_punt ("PL_gen_lcode_sqrt_f2: incorrect src type");

  new_oper = PL_new_loper (NULL, Lop_SQRT_F2);
  new_oper->dest[0] = PL_gen_operand (dest);
  new_oper->src[0] = PL_gen_operand (src);

  L_insert_oper_after (cb, cb->last_op, new_oper);
  return;
}


void
PL_gen_lcode_min_f2 (L_Cb * cb, PL_Operand dest, _PL_Operand src[], int n_src)
{
  L_Oper *new_oper;

  if (n_src < 2)
    P_punt ("PL_gen_lcode_min_f2: not enough arguments");

  if (!is_double (src->data_type))
    P_punt ("PL_gen_lcode_min_f2: incorrect src type");

  new_oper = PL_new_loper (NULL, Lop_MIN_F2);
  new_oper->dest[0] = PL_gen_operand (dest);
  new_oper->src[0] = PL_gen_operand (src);
  new_oper->src[1] = PL_gen_operand (src + 1);

  L_insert_oper_after (cb, cb->last_op, new_oper);
  return;
}


void
PL_gen_lcode_max_f2 (L_Cb * cb, PL_Operand dest, _PL_Operand src[], int n_src)
{
  L_Oper *new_oper;

  if (n_src < 2)
    P_punt ("PL_gen_lcode_max_f2: not enough arguments");

  if (!is_double (src->data_type))
    P_punt ("PL_gen_lcode_max_f2: incorrect src type");

  new_oper = PL_new_loper (NULL, Lop_MAX_F2);
  new_oper->dest[0] = PL_gen_operand (dest);
  new_oper->src[0] = PL_gen_operand (src);
  new_oper->src[1] = PL_gen_operand (src + 1);

  L_insert_oper_after (cb, cb->last_op, new_oper);
  return;
}


void
PL_gen_lcode_co_proc (L_Cb * cb, PL_Operand dest,
		      _PL_Operand src[], int n_src)
{
  L_Oper *new_oper;

  if (n_src < 2)
    P_punt ("PL_gen_lcode_co_proc: not enough arguments");

  if (!is_integer (src->data_type) || !is_integer ((src + 1)->data_type))
    P_punt ("PL_gen_lcode_co_proc: incorrect src types");

  new_oper = PL_new_loper (NULL, Lop_CO_PROC);
  new_oper->dest[0] = PL_gen_operand (dest);
  new_oper->src[0] = PL_gen_operand (src);
  new_oper->src[1] = PL_gen_operand (src + 1);

  L_insert_oper_after (cb, cb->last_op, new_oper);
  return;
}


void
PL_gen_lcode_fetch_add (L_Cb * cb, PL_Operand dest,
			_PL_Operand src[], int n_src)
{
  L_Oper *new_oper;

  if (n_src < 2)
    P_punt ("PL_gen_lcode_fetch_add: not enough arguments");

  if (!is_integer (src->data_type) || !is_integer ((src + 1)->data_type))
    P_punt ("PL_gen_lcode_fetch_add: incorrect src types");

  new_oper = PL_new_loper (NULL, Lop_FETCH_AND_ADD);
  new_oper->dest[0] = PL_gen_operand (dest);
  new_oper->src[0] = PL_gen_operand (src);
  new_oper->src[1] = PL_gen_operand (src + 1);

  L_insert_oper_after (cb, cb->last_op, new_oper);
  return;
}


void
PL_gen_lcode_fetch_or (L_Cb * cb, PL_Operand dest,
		       _PL_Operand src[], int n_src)
{
  L_Oper *new_oper;

  if (n_src < 2)
    P_punt ("PL_gen_lcode_fetch_or: not enough arguments");

  if (!is_integer (src->data_type) || !is_integer ((src + 1)->data_type))
    P_punt ("PL_gen_lcode_fetch_or: incorrect src types");

  new_oper = PL_new_loper (NULL, Lop_FETCH_AND_OR);
  new_oper->dest[0] = PL_gen_operand (dest);
  new_oper->src[0] = PL_gen_operand (src);
  new_oper->src[1] = PL_gen_operand (src + 1);

  L_insert_oper_after (cb, cb->last_op, new_oper);
  return;
}


void
PL_gen_lcode_fetch_and (L_Cb * cb, PL_Operand dest, _PL_Operand src[],
			int n_src)
{
  L_Oper *new_oper;

  if (n_src < 2)
    P_punt ("PL_gen_lcode_fetch_and: not enough arguments");

  if (!is_integer (src->data_type) || !is_integer ((src + 1)->data_type))
    P_punt ("PL_gen_lcode_fetch_and: incorrect src types");

  new_oper = PL_new_loper (NULL, Lop_FETCH_AND_AND);
  new_oper->dest[0] = PL_gen_operand (dest);
  new_oper->src[0] = PL_gen_operand (src);
  new_oper->src[1] = PL_gen_operand (src + 1);

  L_insert_oper_after (cb, cb->last_op, new_oper);
  return;
}


void
PL_gen_lcode_fetch_st (L_Cb * cb, PL_Operand dest,
		       _PL_Operand src[], int n_src)
{
  L_Oper *new_oper;

  if (n_src < 2)
    P_punt ("PL_gen_lcode_fetch_st: not enough arguments");

  if (!is_integer (src->data_type) || !is_integer ((src + 1)->data_type))
    P_punt ("PL_gen_lcode_fetch_st: incorrect src types");

  new_oper = PL_new_loper (NULL, Lop_FETCH_AND_ST);
  new_oper->dest[0] = PL_gen_operand (dest);
  new_oper->src[0] = PL_gen_operand (src);
  new_oper->src[1] = PL_gen_operand (src + 1);

  L_insert_oper_after (cb, cb->last_op, new_oper);
  return;
}


void
PL_gen_lcode_fetch_cond_st (L_Cb * cb, PL_Operand dest,
			    _PL_Operand src[], int n_src)
{
  L_Oper *new_oper;

  if (n_src < 3)
    P_punt ("PL_gen_lcode_fetch_cond_st: not enough arguments");

  if (!is_integer (src->data_type) || !is_integer ((src + 1)->data_type) ||
      !is_integer ((src + 2)->data_type))
    P_punt ("PL_gen_lcode_fetch_cond_st: incorrect src types");

  new_oper = PL_new_loper (NULL, Lop_FETCH_AND_COND_ST);
  new_oper->dest[0] = PL_gen_operand (dest);
  new_oper->src[0] = PL_gen_operand (src);
  new_oper->src[1] = PL_gen_operand (src + 1);
  new_oper->src[2] = PL_gen_operand (src + 2);

  L_insert_oper_after (cb, cb->last_op, new_oper);
  return;
}


static L_Oper *
PL_gen_subroutine_call_for_operation (L_Cb * cb, PL_Operand dest,
				      PL_Operand src1, PL_Operand src2,
				      int opc)
{
  L_Oper *new_oper;
  L_Operand *new_operand;
  int macro_operand_type = 0, macro_operand_sign = 0;
  L_Attr *attr = NULL;
  _PL_Operand operand;
  L_Attr *func_attr = NULL, *new_attr, *tr_attr;
  char line[128];
  char *new_label = NULL, name[256];

  if (M_arch != M_TI)
    P_punt
      ("PL_gen_subroutine_call: currently this function works for TI only");

  if (L_propagate_sign_size_ctype_info)
    {
      switch (opc)
	{
	case Lop_DIV:
	case Lop_REM:
	  macro_operand_sign = 0;
	  macro_operand_type = M_TYPE_INT;
	  break;
	case Lop_DIV_U:
	case Lop_REM_U:
	  macro_operand_sign = 1;
	  macro_operand_type = M_TYPE_INT;
	  break;
	case Lop_DIV_F:
	case Lop_DIV_F2:
	  macro_operand_sign = 0;
	  macro_operand_type = M_TYPE_FLOAT;
	  break;
	default:
	  P_punt ("PL_gen_subroutine_call: unsupported opcode %d", opc);
	}
    }
  else
    {
      macro_operand_sign = 0;
      switch (opc)
	{
	case Lop_DIV:
	case Lop_DIV_U:
	case Lop_REM:
	case Lop_REM_U:
	  macro_operand_type = M_TYPE_INT;
	  break;
	case Lop_DIV_F:
	case Lop_DIV_F2:
	  macro_operand_type = M_TYPE_FLOAT;
	  break;
	default:
	  P_punt ("PL_gen_subroutine_call: unsupported opcode %d", opc);
	}
    }

  sprintf (line, "$P0");
  PL_new_macro (&operand, line, macro_operand_type, macro_operand_sign);
  PL_gen_mov (cb, &operand, src1, attr);

  sprintf (line, "$P1");
  PL_new_macro (&operand, line, macro_operand_type, macro_operand_sign);
  PL_gen_mov (cb, &operand, src2, attr);

  tr_attr = L_new_attr ("tr", 0);

  L_set_macro_attr_field (tr_attr, 0, L_MAC_P0,
			  PL_parm_ctype (macro_operand_type,
					 macro_operand_sign), L_PTYPE_NULL);
  L_set_macro_attr_field (tr_attr, 1, L_MAC_P1,
			  PL_parm_ctype (macro_operand_type,
					 macro_operand_sign), L_PTYPE_NULL);

  func_attr = L_concat_attr (func_attr, tr_attr);

  new_attr = L_new_attr ("ret", 1);

  L_set_macro_attr_field (new_attr, 0, L_MAC_P6,
			  PL_parm_ctype (macro_operand_type,
					 macro_operand_sign), L_PTYPE_NULL);

  func_attr = L_concat_attr (func_attr, new_attr);

  new_attr = L_new_attr ("param_size", 1);
  L_set_int_attr_field (new_attr, 0, 16);
  func_attr = L_concat_attr (func_attr, new_attr);

  new_oper = PL_new_loper (NULL, Lop_JSR);

  switch (opc)
    {
    case Lop_DIV_F:
    case Lop_DIV_F2:
      new_label = (char *) PL_M_fn_label_name ("divide_f", 1);
      break;
    case Lop_DIV:
      new_label = (char *) PL_M_fn_label_name ("divide", 1);
      break;
    case Lop_DIV_U:
      new_label = (char *) PL_M_fn_label_name ("divide_u", 1);
      break;
    case Lop_REM:
      new_label = (char *) PL_M_fn_label_name ("remainder", 1);
      break;
    case Lop_REM_U:
      new_label = (char *) PL_M_fn_label_name ("remainder_u", 1);
      break;
    default:
      P_punt ("PL_gen_subroutine_call: unsupported opcode %d", opc);
    }

  sprintf (name, "_%s", new_label);
  new_operand = L_new_gen_label_operand (name);
  new_oper->src[0] = new_operand;

  new_oper->attr = L_concat_attr (new_oper->attr, func_attr);

  sprintf (line, "$P6");
  PL_new_macro (&operand, line, macro_operand_type, 0);
  PL_gen_mov (cb, dest, &operand, attr);
  return new_oper;
}
