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
 *	File:	pl_data.c
 *	Author:	Po-hua Chang, Wen-mei Hwu
 *	Creation Date:	June 1990
 *	 Revised: Dave Gallagher, Scott Mahlke - 6/94
 *              Build Lcode structure rather than just printing out text file
 *      Revised by: Ben-Chung Cheng - June 1995
 *              Change M_SIZE_INT, M_SIZE_CHAR to H_INT_SIZE, H_CHAR_SIZE
 *              Those should be determined in runtime, not compile time
 *        Chien-Wei Li
 *          change H_CHAR_SIZE, H_INT_SIZE, H_SHORT_SIZE, and H_LONG_SIZE to 
 *                 P_CHAR_SIZE, P_INT_SIZE, P_SHORT_SIZE, and P_LONG_SIZE.
 *                 
\*****************************************************************************/
#include <config.h>
#include "pl_main.h"
#include <Pcode/util.h>

#undef DEBUG_STRUCT_INIT

#undef TEST_SIZE

#define MIN_BSS_OBJECT_SIZE	8	/* 8 bytes */

/*
 * Swaps byte order in word 'W'.  Used to convert architectures with
 * different byte orders (assumes 32 bit words).
 * Please report bugs found in this macro to John Gyllenhaal.
 */
#define SWAP_BYTES_32(W)        ((((unsigned long) W) << 24) | \
	   		        ((((unsigned long) W) >> 24) & 0xff) | \
			        ((((unsigned long) W) >> 8) & 0xff00) | \
				((((unsigned long) W) & 0xff00) << 8))

#define SWAP_BYTES_16(H)        (((((unsigned long) H) << 8) & 0xff00) | \
	   		         ((((unsigned long) H) >> 8) & 0xff))

/* extern declarations */
extern void Gen_C_Expr (FILE *, Expr);

/* forward declarations */
void PL_gen_var (L_Datalist * list, VarDcl var);
static void PL_gen_init (L_Datalist * list, Key type, Init init, char *label,
			 long offset);
static L_Expr *PL_gen_expr (Expr expr);

/* BCC - added to handle the offset calculation of a->b->c->d - 5/31/95 */
static L_Expr *L_reduce_int_expr (L_Expr * expr);

/*-------------------------------------------------------------------------*/
static char *last_ms = "???";

void
PL_ms (L_Datalist * list, char *name)
{
  L_Data *new_data;

  if (strcmp (name, last_ms))
    {
      new_data = L_new_data (L_INPUT_MS);

      new_data->N = L_ms_id (name);

      L_concat_datalist_element (list, L_new_datalist_element (new_data));

      last_ms = name;
    }
}

/* SAM 8-97 */
void
PL_invalidate_last_ms ()
{
  last_ms = "??bla??";
}


/*-------------------------------------------------------------------------*/
/* LCW - add one more parameter to pass the type information of a variable
   to lcode for debugging - 4/12/96 */


L_Data *
PL_gen_global (L_Datalist * list, char *name, Key type, int objid)
{
  L_Data *new_data;

  new_data = L_new_data (L_INPUT_GLOBAL);
  new_data->address = L_new_expr_label (name);

  new_data->id = objid;

  /* LCW - insert L_Type in L_Data structure - 4/12/96 */
  if (PL_emit_source_info || PL_emit_data_type_info)
    new_data->h_type = L_gen_type (type);
  else
    new_data->h_type = NULL;

  L_concat_datalist_element (list, L_new_datalist_element (new_data));

  return new_data;
}


void
PL_gen_void (L_Datalist * list, char *name)
{
  L_Data *new_data;

  new_data = L_new_data (L_INPUT_VOID);
  new_data->address = L_new_expr_label (name);

  L_concat_datalist_element (list, L_new_datalist_element (new_data));
}


void
PL_gen_scalar (L_Datalist * list, int type, int size, Expr expr, char *name)
{
  L_Data *new_data;

  new_data = L_new_data (type);
  new_data->address = L_new_expr_label (name);

  new_data->N = size;

  if (expr)
    {
      L_Expr *new_expr;

      new_expr = PL_gen_expr (expr);
      new_data->value = new_expr;
    }

  L_concat_datalist_element (list, L_new_datalist_element (new_data));
}


void
PL_gen_align (L_Datalist * list, int size, char *name)
{
  L_Data *new_data;
  L_Expr *new_expr;

  new_data = L_new_data (L_INPUT_ALIGN);

  new_expr = L_new_expr_label (name);

  new_data->address = new_expr;
  new_data->N = size;

  L_concat_datalist_element (list, L_new_datalist_element (new_data));
}


void
PL_gen_element_size (L_Datalist * list, int size, char *name)
{
  L_Data *new_data;
  L_Expr *new_expr;

  new_data = L_new_data (L_INPUT_ELEMENT_SIZE);

  new_expr = L_new_expr_label (name);

  new_data->address = new_expr;
  new_data->N = size;

  L_concat_datalist_element (list, L_new_datalist_element (new_data));
}


void
PL_gen_reserve (L_Datalist * list, int size)
{
  L_Data *new_data;

  new_data = L_new_data (L_INPUT_RESERVE);

  new_data->N = size;

  L_concat_datalist_element (list, L_new_datalist_element (new_data));
}


void
PL_gen_var (L_Datalist * list, VarDcl var)
{
  Key type;
  _BasicType bt;
  _VarQual tq;
  int type_size;
  int type_align;
  int mtype_type;
  char *name;

  name = PL_fmt_var_name(var->name, var->key);

  /*
   *  map big array that does not require initialization
   *  to (ms bss)
   */

  type = var->type;
  bt = PST_GetTypeBasicType(PL_symtab, type);
  tq = P_GetVarDclQualifier(var);

  /* CWL - 12/17/00 
   *  don't generate lcode for external variables
   */

  if ((tq & VQ_EXTERN) || ((tq & VQ_GLOBAL) && (bt & BT_FUNC)))
    return;

  mtype_type = PL_key_get_mtype(type);
  type_size = PL_key_get_size (type);
  type_align = PL_key_get_align (type);

  if (M_arch != M_TAHOE)
    {
      if (tq & (VQ_STATIC | VQ_GLOBAL))
	{
	  if (!var->init && (type_size >= MIN_BSS_OBJECT_SIZE))
	    PL_ms (list, "bss");
	  else
	    PL_ms (list, "data");
	}
    }
  else
    {
      if (!var->init)
	PL_ms (list, (type_size <= MIN_BSS_OBJECT_SIZE) ? "sbss" : "bss");
      else
	PL_ms (list, (type_size <= MIN_BSS_OBJECT_SIZE) ? "sdata" : "data");
    }

#if 0
  /* JWS: "global" appears to be a misnomer! All file-level data tokens
   * seem to need them (L_gp_rel, for example, expects them).
   */
  if (tq & VQ_GLOBAL)
#endif
    {
      if (PL_mark_glob_objids)
	{
	  int found_id = 0;
	  if (var->pragma)
	    {
	      /* HCH 5/10/04: transmit Pipa object ids through to Lcode */
	      Pragma prag;
	      Expr expr;
	      
	      prag = P_GetVarDclPragma (var);
	      if (P_FindPragma (prag, "OBJID"))
		{
		  expr = P_GetPragmaExpr (prag);
		  found_id = P_GetExprScalar(expr);
		}
	    }
	  PL_gen_global (list, name, type, found_id);
	}
      else 
	{
	  PL_gen_global (list, name, type, 0);
	}
    }

  if (!(tq & (VQ_STATIC | VQ_GLOBAL)))
    return;

  {
    Init init;
    init = var->init;

    if (!name)
      name = PL_DEFAULT_LABEL;


    if (bt & BT_POINTER)
      {
	PL_gen_scalar (list, L_INPUT_LONG, 1,
		       init ? init->expr : NULL, name);
      }
    else if (bt & BT_ARRAY)
      {
	int element_size;

	/* Determine the array element size */
	element_size = PL_key_get_size (PST_GetTypeType(PL_symtab, type));
	PL_gen_align (list, type_align, name);
	PL_gen_element_size (list, element_size, name);
	PL_gen_reserve (list, type_size);

	if (init)
	  PL_gen_init (list, type, init, name, 0);

#ifdef TEST_SIZE
	if (type_align < 0)
	  P_punt ("PL_gen_var: "
		  "align of structure must not be negative (arry)");
	if (type_size < 0)
	  P_punt ("PL_gen_var: "
		  "size of structure must not be negative (arry)");
#endif
      }
    else if (bt & BT_FUNC)
      {
	/** do nothing **/
      }
    else
      {
	int ltype = 0;
	switch (mtype_type)
	  {
	  case M_TYPE_VOID:
	    PL_gen_void (list, name);
	    break;
	  case M_TYPE_CHAR:
	    PL_gen_scalar (list, L_INPUT_BYTE, 1,
			   init ? init->expr : NULL, name);
	    break;
	  case M_TYPE_SHORT:
	    ltype = PL_M_no_short_int () ? L_INPUT_LONG : L_INPUT_WORD;

	    PL_gen_scalar (list, ltype, 1, init ? init->expr : NULL, name);
	    break;
	  case M_TYPE_INT:
	    PL_gen_scalar (list, L_INPUT_LONG, 1,
			   init ? init->expr : NULL, name);
	    break;
	  case M_TYPE_LONG:
	    if (P_LONG_SIZE == 64)
	      ltype = L_INPUT_LONGLONG;
	    else if (P_LONG_SIZE == 32)
	      ltype = L_INPUT_LONG;
	    else if (P_LONG_SIZE == 16)
	      ltype = L_INPUT_LONG;
	    else
	      P_punt ("PL_gen_var: illegal long size %d", P_LONG_SIZE);

	    PL_gen_scalar (list, ltype, 1, init ? init->expr : NULL, name);
	    break;
	  case M_TYPE_LLONG:
	    PL_gen_scalar (list, L_INPUT_LONGLONG, 1,
			   init ? init->expr : NULL, name);
	    break;
	  case M_TYPE_POINTER:
	    if (P_POINTER_SIZE == 64)
	      ltype = L_INPUT_LONGLONG;
	    else if (P_POINTER_SIZE == 32)
	      ltype = L_INPUT_LONG;
	    else
	      P_punt ("PL_gen_var: illegal pointer size %d", P_POINTER_SIZE);

	    PL_gen_scalar (list, ltype, 1, init ? init->expr : NULL, name);
	    break;
	  case M_TYPE_FLOAT:
	    PL_gen_scalar (list, L_INPUT_FLOAT, 1,
			   init ? init->expr : NULL, name);
	    break;
	  case M_TYPE_DOUBLE:
	    PL_gen_scalar (list, L_INPUT_DOUBLE, 1,
			   init ? init->expr : NULL, name);
	    break;
	  case M_TYPE_UNION:
	  case M_TYPE_STRUCT:
	  case M_TYPE_BLOCK:
	    PL_gen_align (list, type_align, name);

	    PL_gen_reserve (list, type_size);
	    if (init)
	      PL_gen_init (list, type, init, name, 0);
	    break;
	  default:
	    P_punt ("PL_gen_var: illegal mtype");
	  }
      }
  }

  return;
}


L_Expr *
L_new_addr (char *label, int offset, Key type)
{
  char *name;
  L_Expr *new_expr;

  name = PL_M_fn_label_name (label, PST_IsFunctionType(PL_symtab, type));
  new_expr = L_new_expr_addr (name, offset);

  return (new_expr);
}


L_Expr *
L_new_addr_no_underscore (char *label, int offset)
{
  L_Expr *new_expr;

  if (!offset)
    {
      new_expr =
	L_new_expr_label_no_underscore ((char *)
					PL_M_fn_label_name (label, 0));
    }
  else
    {
      new_expr =
	L_new_expr_add (L_new_expr_label_no_underscore
			((char *) PL_M_fn_label_name (label, 0)),
			L_new_expr_int ((ITintmax) offset));
    }

  return (new_expr);
}


static void
PL_gen_init (L_Datalist * list, Key type, Init init, char *label, long offset)
{
  Type rtype;
  _BasicType bt;
  Expr expr;
  L_Data *new_data = NULL;

  rtype = PST_ReduceTypedefs (PL_symtab, type);
  bt = PST_GetTypeBasicType(PL_symtab, rtype);
  
  if (!init)
    return;

  if (bt & BT_POINTER)
    {
      int ltype = 0;

      /*
       *  A pointer is treated as machine reg ctype
       */
      if (!(expr = init->expr))
	P_punt ("initializer of a pointer must be a simple expression");
      
      if (PL_native_int_reg_ctype == L_CTYPE_INT)
	ltype = L_INPUT_WI;
      else if (PL_native_int_reg_ctype == L_CTYPE_LLONG)
	ltype = L_INPUT_WQ;
      else
	P_punt ("PL_gen_init: Unsupported native machine reg ctype %d\n",
		PL_native_int_reg_ctype);

      new_data = L_new_data_w (ltype,
			       L_new_addr (label, offset, rtype),
			       PL_gen_expr (expr));
      L_concat_datalist_element (list, L_new_datalist_element (new_data));
    }
  else if (bt & BT_ARRAY)
    {
      Type etype = PST_GetTypeType (PL_symtab, rtype);

      if ((PST_GetTypeBasicType(PL_symtab, etype) & BT_CHAR) && 
	  (expr = init->expr) && (expr->opcode == OP_string))
	{
	  /* Handle special case of using a string constant to
	   * initialize a char array.  This can't be handled by
	   * PL_gen_expr(), since that would create a pointer to
	   * an immutable string, not a mutable char array initialized
	   * to the value of the specified string.
	   */
	  char *str = P_AddDQ(expr->value.string);
	  new_data = L_new_data_w (L_INPUT_WS, 
				   L_new_addr (label, offset, rtype),
				   L_new_expr_string (str));
	  free(str);
	  if (new_data)
	    L_concat_datalist_element (list, 
				       L_new_datalist_element (new_data));
	}
      else
	{
	  Init initializer;
	  ITintmax local_offset, element_size;

	  if (!(initializer = init->set))
	    P_punt ("expecting an array initializer\n");

	  /* Recur on each element */

	  element_size = PL_key_get_size (etype);
	  local_offset = offset;
	  for (; initializer; initializer = initializer->next)
	    {
	      PL_gen_init (list, etype, initializer, label, local_offset);
	      local_offset += element_size;
	    }
	}
    }
  else if (bt & BT_UNION)
    {
      Type utype = PST_GetTypeType (PL_symtab, rtype);
      UnionDcl un;
      Field field;
      /*
       *  For union structure, we initialize the first field.
       */
      if (!(un = PST_GetUnionDclEntry (PL_symtab, utype)))
	P_punt ("PL_gen_init: cannot initialize an undefined union\n");

      if (!(field = un->fields))
	P_punt ("PL_gen_init: cannot initialize empty union %s\n",
		un->name);

      if (!init->set)
	P_warn ("PL_gen_init: expected an aggregate in union %s; "
		"making default assignment\n",
		un->name);
      else
	init = init->set;

      PL_gen_init (list, field->type, init, label, offset + field->offset);
    }
  else if (bt & BT_STRUCT)
    {
      Type stype = PST_GetTypeType (PL_symtab, rtype);
      StructDcl st;
      Field field, ptr, last;
      ITintmax word_mask;
      Init initializer;

      if (!(st = PST_GetStructDclEntry (PL_symtab, stype)))
	P_punt ("cannot initialize an undefined struct");

      if (!(field = st->fields))
	P_punt ("cannot initialize empty struct %s", st->name);

      if (!(initializer = init->set))
	P_punt ("Bad aggregate initializer for struct %s", st->name);

      /*
       *  need to handle bit fields very carefully.
       */
      word_mask = ~((P_INT_SIZE / P_CHAR_SIZE) - 1);

      for (; field && initializer; field = last)
	{
	  /* handle one word each time through loop JEM 5/23/96 */
	  Field start = field;	/* first field in the word */
	  int is_bit = field->is_bit_field;
	  int start_offset = field->offset;

	  if (field->offset != PST_GetFieldContainerOffset (PL_symtab, field))
	    P_punt ("field initialization offset error");

	  /* Find end of word, and determine if word contains bitfields.
	     JEM 5/23/96 */

	  for (ptr = start->next; ptr; ptr = ptr->next)
	    {
	      if ((field->offset & word_mask) != (ptr->offset & word_mask))
		break;

	      is_bit |= ptr->is_bit_field;
	    }

	  last = ptr;	/* first field in next word */
	  
	  if (is_bit)
	    {
	      /* Word contains bitfields: pack into one word */

	      Expr red_expr;
	      int total_size = 0, initializer_type = 0;
	      ITuintmax data, value = 0;

	      for (ptr = start; initializer && ptr != last; 
		   ptr = ptr->next, initializer = initializer->next)
		{
		  ITuintmax f_bit_mask;
		  int f_end, f_size, f_bit_shift;

		  f_size = PL_bit_field_info(ptr, &f_bit_mask, 
					     &f_bit_shift);

		  if (!(expr = initializer->expr))
		    P_punt ("Bit field missing initializer");

		  red_expr = PST_ReduceExpr (PL_symtab, expr);

		  if (!P_IsIntegralExpr (red_expr))
		    P_punt ("illegal initializer for a bit field");

		  data = P_IntegralExprValue (red_expr);
		  red_expr = PST_RemoveExpr (PL_symtab, red_expr);

#ifdef DEBUG_STRUCT_INIT
		  printf ("original initializer = %d\n", data);
#endif
		  /* get the data size (in bits) of load unit */

		  f_end = f_bit_shift + f_size;

		  if (f_end > total_size)
		    total_size = f_end;

		  /* Place bit field in proper location in its load
		     unit.        
		     f_bit_shift - bit offset from LSB of load unit. 
		     f_bit_mask - mask off everything in load unit 
		     except for bitfield.  */

		  /* The "load unit" is the unit (e.g. char, long, short)
		     that must be loaded to access the bitfield. */
		  
		  data = (data << f_bit_shift) & f_bit_mask;

		  /* or data for current field into word value */
		  value |= data;

#ifdef DEBUG_STRUCT_INIT
		  printf ("value so far = %d\n", value);
#endif
		}

	      /* Create the initializer.  JEM 5/23/96 */

	      /* For BIG ENDIAN machines, we have packed the 
		 bitfield load units with reference to the upper end of 
		 the word.  If we don't need the whole word, then we          
		 need to shift the data so that it is correctly offset
		 from the new upper end. JEM 5/23/96 */

	      if (total_size <= P_CHAR_SIZE)
		{
		  initializer_type = L_INPUT_WB;
		  if (PL_native_order == M_BIG_ENDIAN)
		    value = value >> (P_INT_SIZE - P_CHAR_SIZE);
		}
	      else if (total_size <= P_SHORT_SIZE)
		{
		  initializer_type = L_INPUT_WW;
		  if (PL_native_order == M_BIG_ENDIAN)
		    value = value >> (P_INT_SIZE - P_SHORT_SIZE);
		}
	      else if (total_size <= P_INT_SIZE)
		{
		  initializer_type = L_INPUT_WI;
		  if (PL_native_order == M_BIG_ENDIAN)
		    value = value >> (P_INT_SIZE - P_INT_SIZE);
		}
	      else
		{
		  P_punt ("bitfield is larger than an int");
		}

#ifdef DEBUG_STRUCT_INIT
	      printf ("final value = %d\n", value);
#endif

	      new_data = L_new_data_w (initializer_type,
				       L_new_addr (label,
						   (offset +
						    start_offset), rtype),
				       L_new_expr_int (value));

	      L_concat_datalist_element (list, 
					 L_new_datalist_element (new_data));
	    }
	  else
	    {
	      /* No bitfields: handle normally */

	      for (ptr = start; initializer && ptr != last; 
		   ptr = ptr->next, initializer = initializer->next)
		{
#ifdef DEBUG_STRUCT_INIT
		  printf ("Normal word initialization: "
			  "has initializer =  %d\n", initializer);
#endif
		  PL_gen_init (list, ptr->type, initializer, label,
			       offset + ptr->offset);
		}
	    }
	}
    }
  else if (bt & BT_FUNC)
    {
      P_punt ("cannot initialize a function");
    }
  else
    {
      int ltype =0;

      if (!(expr = init->expr))
	P_punt ("initializer must be a simple expression");

      switch (PL_key_get_mtype (rtype))
	{
	case M_TYPE_CHAR:
	  ltype = L_INPUT_WB;
	  break;
	case M_TYPE_SHORT:
	  ltype = (!PL_M_no_short_int ()) ? L_INPUT_WW : L_INPUT_WI;
	  break;
	case M_TYPE_INT:
	  ltype = L_INPUT_WI;
	  break;
	case M_TYPE_LONG:
	  if (P_LONG_SIZE == 64)
	    ltype = L_INPUT_WQ;
	  else if (P_LONG_SIZE == 32)
	    ltype = L_INPUT_WI;
	  else if (P_LONG_SIZE == 16)
	    ltype = L_INPUT_WI;
	  else
	    P_punt ("PL_gen_init: Unsupported long size %d\n",
		    P_LONG_SIZE);
	  break;
	case M_TYPE_LLONG:
	  ltype = L_INPUT_WQ;
	  break;
	case M_TYPE_POINTER:
	  if (P_POINTER_SIZE == 32)
	    ltype = L_INPUT_WI;
	  else if (P_POINTER_SIZE == 64)
	    ltype = L_INPUT_WQ;
	  else if (P_POINTER_SIZE == 16)
	    ltype = L_INPUT_WI;
	  else
	    P_punt ("PL_gen_init: Unsupported pointer size %d\n",
		    P_POINTER_SIZE);
	  break;
	case M_TYPE_FLOAT:
	    ltype = L_INPUT_WF;
	  break;
	case M_TYPE_DOUBLE:
	    ltype = L_INPUT_WF2;
	  break;
	default:
	  P_punt ("PL_gen_init: illegal expression");
	}

      new_data = L_new_data_w (ltype, L_new_addr (label, offset, rtype),
			       PL_gen_expr (expr));
      L_concat_datalist_element (list, L_new_datalist_element (new_data));
    }
  return;
}


static L_Expr *
PL_gen_expr (Expr expr)
{
  Key type;
  Expr op1, op2;
  ITintmax value;
  L_Expr *new_expr = NULL;

  if (!expr)
    return NULL;

  type = PST_ExprType(PL_symtab, expr);
  switch (expr->opcode)
    {
    case OP_var:
      {
	char *name = PL_fmt_var_name(expr->value.var.name,
				     expr->value.var.key);
	int is_func = PL_is_func_var(expr->value.var.key);
	new_expr = L_new_expr_label (PL_M_fn_label_name (name, is_func));
      }
      break;

    case OP_enum:
      /* CWL - 08/22/00
       * Pcode should have "inlined" all enum's.
       */
      P_punt ("PL_gen_expr: OP_enum");
      break;

    case OP_int:
      value = P_IntegralExprValue (expr);
      new_expr = L_new_expr_int (value);
      break;

    case OP_char:
      value = P_IntegralExprValue (expr);
      new_expr = L_new_expr_int (value & 0xFF);
      break;

    case OP_float:
      if (!(PST_GetTypeBasicType(PL_symtab, type) & BT_FLOAT))
	P_punt ("PL_gen_expr: OP_float type mismatch");

      new_expr = L_new_expr_float (expr->value.real);
      break;
      /* BCC - added - 8/5/96 */

    case OP_double:
      if (!(PST_GetTypeBasicType(PL_symtab, type) & BT_DOUBLE))
	P_punt ("PL_gen_expr: OP_cwdouble type mismatch");

      new_expr = L_new_expr_double (expr->value.real);
      break;

    case OP_string:
      /* Find an appropriate immutable string for a string constant */
      new_expr = L_new_expr_label (PL_get_string_label (expr->value.string));
      break;

    case OP_cast:
      /* NJW - ignore cast for initializations (2/93) */
      new_expr = PL_gen_expr (expr->operands);
      break;

    case OP_neg:
      new_expr = L_new_expr (L_EXPR_NEG);
      new_expr->A = PL_gen_expr (expr->operands);
      break;

    case OP_inv:
      new_expr = L_new_expr (L_EXPR_COM);
      new_expr->A = PL_gen_expr (expr->operands);
      break;

    case OP_expr_size:
      op1 = expr->operands;
      type = op1->type;
      value = PL_key_get_size (type);
      new_expr = L_new_expr_int (value);
      break;

    case OP_type_size:
      value = PL_key_get_size (expr->value.type);
      new_expr = L_new_expr_int (value);
      break;

    case OP_add:
      {
	int is_ptr1;
	op1 = expr->operands;
	op2 = op1->sibling;
	is_ptr1 = PST_IsPointerType (PL_symtab, op1->type) ||
	  PST_IsArrayType (PL_symtab, op1->type);
	if (is_ptr1)
	  {
	    int size = PL_key_get_size (PST_GetTypeType(PL_symtab, op1->type));

	    new_expr =
	      L_new_expr_add (PL_gen_expr (op1),
			      L_new_expr_mul (PL_gen_expr (op2),
					      L_new_expr_int ((ITintmax)
							      size)));
	  }
	else
	  {
	    new_expr = L_new_expr_add (PL_gen_expr (op1), PL_gen_expr (op2));
	  }
	break;
      }

    case OP_sub:
      {
	int is_ptr1, is_ptr2;
	op1 = expr->operands;
	op2 = op1->sibling;
	is_ptr1 = PST_IsPointerType (PL_symtab, op1->type) ||
	  PST_IsArrayType (PL_symtab, op1->type);
	is_ptr2 = PST_IsPointerType (PL_symtab, op2->type) ||
	  PST_IsArrayType (PL_symtab, op2->type);

	if (is_ptr1 | is_ptr2)
	  {
	    int size = PL_key_get_size (PST_GetTypeType(PL_symtab, op1->type));

	    if (is_ptr1 & !is_ptr2)
	      {
		new_expr =
		  L_new_expr_sub (PL_gen_expr (op1),
				  L_new_expr_mul (PL_gen_expr (op2),
						  L_new_expr_int ((ITintmax)
								  size)));
	      }
	    else if (is_ptr1 & is_ptr2)
	      {
		new_expr =
		  L_new_expr_div (L_new_expr_sub (PL_gen_expr (op1),
						  PL_gen_expr (op2)),
				  L_new_expr_int ((ITintmax) size));
	      }
	    else
	      {
		new_expr =
		  L_new_expr_sub (PL_gen_expr (op1), PL_gen_expr (op2));
	      }
	  }
	else
	  {
	    new_expr = L_new_expr_sub (PL_gen_expr (op1), PL_gen_expr (op2));
	  }
	break;
      }

    case OP_mul:
      op1 = expr->operands;
      op2 = op1->sibling;
      new_expr = L_new_expr_mul (PL_gen_expr (op1), PL_gen_expr (op2));
      break;

    case OP_div:
      op1 = expr->operands;
      op2 = op1->sibling;
      new_expr = L_new_expr_div (PL_gen_expr (op1), PL_gen_expr (op2));
      break;

    case OP_addr:
      /*
       *  The operand must be a OP_var.
       */
      if (!(op1 = expr->operands))
	P_punt ("PL_gen_expr: illegal initializer: "
		"& can be applied only on variables");
      if (op1->opcode == OP_var)
	{
	  char *name = PL_fmt_var_name(op1->value.var.name,
				       op1->value.var.key);
	  int is_func = PL_is_func_var(op1->value.var.key);
	 new_expr = 
	   L_new_expr_label ((char *)PL_M_fn_label_name (name, is_func));
	}
      else
	{
	  int opcode = op1->opcode;
	  /* BCC - adding OP_arrow - 5/29/95 */
	  if ((opcode != OP_index) && (opcode != OP_dot)
	      && (opcode != OP_arrow))
	    P_punt ("PL_gen_expr: illegal initializer: "
		    "& can be applied only on variables and . []");
	  new_expr = PL_gen_expr (op1);
	}
      break;

    case OP_indr:
      /*
       *  The operand must be a pointer
       */
      if (!(op1 = expr->operands))
	P_punt ("PL_gen_expr: illegal initializer: operand missing for (*)");
      if (!PST_IsPointerType (PL_symtab, op1->type) && 
	  !PST_IsArrayType (PL_symtab, op1->type))
	P_punt ("PL_gen_expr: illegal initializer: "
		"* can be applied only on pointers");

      new_expr = PL_gen_expr (op1);
      break;

    case OP_arrow:
      {
	Field field;
	L_Expr *before_reduce;

	if (!(op1 = expr->operands))
	  P_punt ("PL_gen_expr: illegal initializer: x->y: corrupted x");

	field = PL_key_get_field(PST_GetTypeType(PL_symtab, op1->type),
				 expr->value.string);

	if (field->offset == 0)
	  {
	    before_reduce = PL_gen_expr (op1);
	    new_expr = L_reduce_int_expr (before_reduce);
	  }
	else
	  {
	    before_reduce = L_new_expr_add (PL_gen_expr (op1),
					    L_new_expr_int ((ITintmax)
							    field->offset));
	    new_expr = L_reduce_int_expr (before_reduce);
	  }
	break;
      }
    case OP_dot:
      {
	Field field;
	L_Expr *before_reduce;

	if (!(op1 = expr->operands))
	  P_punt ("PL_gen_expr: illegal initializer: x.y: corrupted x");

	field = PL_key_get_field(PST_GetTypeType(PL_symtab, op1->type),
				 expr->value.string);

	if (field->offset == 0)
	  {
	    before_reduce = PL_gen_expr (op1);
	    new_expr = L_reduce_int_expr (before_reduce);
	  }
	else
	  {
	    before_reduce = L_new_expr_add (PL_gen_expr (op1),
					    L_new_expr_int ((ITintmax)
							    field->offset));
	    new_expr = L_reduce_int_expr (before_reduce);
	  }
	break;
      }
    case OP_index:
      {
	Expr base, offset;
	int size, N;
	op1 = expr->operands;
	op2 = op1->sibling;
	if (!op1 || !op2)
	  P_punt ("PL_gen_expr: illegal initializer: x[y]");

	base = op1;
	offset = PST_ReduceExpr (PL_symtab, op2);
	if (!P_IsIntegralExpr (offset))
	  {
	    fprintf (stderr, "(a[x]) in initailizer: x must be a constant");
	    P_punt ("PL_gen_expr: the initializer is too complex");
	  }
	N = P_IntegralExprValue (offset);
	size = PL_key_get_size (PST_ExprType(PL_symtab, expr));	/* size of 1 element */
	if (!N)
	  {
	    new_expr = PL_gen_expr (base);
	  }
	else
	  {
	    /* The addition is:
	     *          base_addr + (offset_into_array * size_of_elements) */
	    new_expr = L_new_expr_add (PL_gen_expr (base),
				       L_new_expr_int ((ITintmax) size * N));
	  }
	P_RemoveExpr (offset);
	break;
      }
    default:
      P_punt ("PL_gen_expr: illegal initializer: too complex");
    }

  return (new_expr);
}


/* Added by REH 4/14/93 for HPPA */
int
PL_is_function (char *fn_name)
{
  int is_func = 0;
  Key scope = {PL_file_scope, 1}, key;

  key = PST_ScopeFindByNameR (PL_symtab, scope, fn_name, ET_FUNC);

  if (P_ValidKey (key))
    is_func = 1;

  return (is_func);
}


/* BCC - added to reduce offset calculation - 5/31/95 */
static L_Expr *
L_reduce_int_expr (L_Expr * expr)
{
  L_Expr *new_expr, *expr1, *expr2;

  switch (expr->type)
    {
    case L_EXPR_ADD:
      expr1 = L_reduce_int_expr (expr->A);
      if (expr1 == expr->A)
	{
	  expr->A = 0;
	}
      else
	{
	  L_delete_expr_element (expr->A);
	  expr->A = 0;
	}
      expr2 = L_reduce_int_expr (expr->B);
      if (expr2 == expr->B)
	{
	  expr->B = 0;
	}
      else
	{
	  L_delete_expr_element (expr->B);
	  expr->B = 0;
	}

      if (expr1->type == L_EXPR_INT && expr2->type == L_EXPR_INT)
	{
	  new_expr = L_new_expr (L_EXPR_INT);
	  new_expr->value.i = expr1->value.i + expr2->value.i;
	  L_delete_expr_element (expr);
	  L_delete_expr_element (expr1);
	  L_delete_expr_element (expr2);
	}
      else
	{
	  new_expr = expr;
	  new_expr->A = expr1;
	  new_expr->B = expr2;
	}
      break;
    default:
      new_expr = expr;
      break;
    }
  return new_expr;
}


/* LCW - convert Pcode type to Lcode type - 4/12/96 */

L_Type *
L_gen_type (Key type)
{
  L_Type *ltype;
  L_Dcltr *f_ldcl = NULL, *l_ldcl = NULL, *ldcl;
  _BasicType bt;

  ltype = (L_Type *) L_alloc (L_alloc_type);

  while (PST_GetTypeBasicType(PL_symtab, type) & (BT_ARRAY|BT_POINTER))
    {
      ldcl = (L_Dcltr *) L_alloc (L_alloc_dcltr);
	  
      if (!f_ldcl)
	f_ldcl = ldcl;
      if (l_ldcl)
	l_ldcl->next = ldcl;
      l_ldcl = ldcl;

      ldcl->index = NULL;
      if (PST_GetTypeBasicType(PL_symtab, type) & BT_ARRAY)
	{
	  Expr index = PST_GetTypeArraySize(PL_symtab, type);
	  ldcl->method = L_D_ARRY;
	  if (index)
	    ldcl->index = PL_gen_expr (index);
	}
      else if (PST_GetTypeBasicType(PL_symtab, type) & BT_POINTER)
	ldcl->method = L_D_PTR;
      else if (PST_GetTypeBasicType(PL_symtab, type) & BT_FUNC)
	ldcl->method = L_D_FUNC;

      type = PST_GetTypeType(PL_symtab, type);
    }

  if (l_ldcl)
    l_ldcl->next = NULL;
      
  ltype->dcltr = f_ldcl;      

  /* 03/01/04 REK Removing BT_SIGNED.  An unsigned integer type has
   *              BT_UNSIGNED set.  Otherwise, the type is signed. */
  bt = (PST_GetTypeBasicType(PL_symtab, type) & ~(BT_BIT_FIELD|BT_UNSIGNED));

  switch (bt)
    {
    case BT_VOID: ltype->type = L_DATA_VOID; break;
    case BT_CHAR: ltype->type = L_DATA_CHAR; break;
    case BT_SHORT: ltype->type = L_DATA_SHORT; break;
    case BT_INT: ltype->type = L_DATA_INT; break;
    case BT_LONG: ltype->type = L_DATA_LONG; break;
    case BT_LONGLONG: ltype->type = L_DATA_LONGLONG; break;
    case BT_FLOAT: ltype->type = L_DATA_FLOAT; break;
    case BT_DOUBLE: ltype->type = L_DATA_DOUBLE; break;
    case BT_LONGDOUBLE: ltype->type = L_DATA_LONGDOUBLE; break;
    case BT_STRUCT: ltype->type = L_DATA_STRUCT; break;
    case BT_UNION: ltype->type = L_DATA_UNION; break;
    case BT_VARARG:ltype->type = L_DATA_VARARG; break;
    case BT_FUNC: 
    case BT_POINTER:
      if (P_POINTER_SIZE == 64)
	ltype->type = L_DATA_LONGLONG;
      else if (P_POINTER_SIZE == 32)
	ltype->type = L_DATA_INT;
      else
	assert(0);
      break;
    default:
      assert(0);
    }

  if (PST_GetTypeBasicType(PL_symtab, type) & BT_BIT_FIELD)
    ltype->type |= L_DATA_BIT_FIELD;
#if 0
  /* 03/01/04 REK Replacing comparison against BT_SIGNED with call to
   *              PST_IsSignedType(). */
  if (PST_IsSignedType (PL_symtab, type))
    ltype->type |= L_DATA_SIGNED;
#endif
  if (PST_IsUnsignedType (PL_symtab, type))
    ltype->type |= L_DATA_UNSIGNED;
  
  if (PST_GetTypeBasicType(PL_symtab, type) & BT_STRUCT)
    {
      StructDcl stdcl;
      stdcl = PST_GetStructDclEntry (PL_symtab, PST_GetTypeType(PL_symtab, type));
#if PL_MANGLE_NAMES
      ltype->struct_name = strdup (PL_mangle_name(stdcl->name, stdcl->key));
#else
      ltype->struct_name = strdup (stdcl->name);
#endif
    }
  else if (PST_GetTypeBasicType(PL_symtab, type) & BT_UNION)
    {
      UnionDcl undcl;
      undcl = PST_GetUnionDclEntry (PL_symtab, PST_GetTypeType(PL_symtab, type));
#if PL_MANGLE_NAMES
      ltype->struct_name = strdup (PL_mangle_name(undcl->name, undcl->key));
#else
      ltype->struct_name = strdup (undcl->name);
#endif
    }
  else
    {
      ltype->struct_name = NULL;
    }

  return ltype;
}


/* LCW -- strip the double quotes of a string - 4/21/96 */
static char *
StripDoubleQuotes (char *dst_str, char *src_str)
{
  char *ptr1, *ptr2;

  ptr1 = dst_str;
  ptr2 = src_str;
  while (*ptr2 != '\0')
    if (*ptr2 != '\"')
      *ptr1++ = *ptr2++;
    else
      ptr2++;
  *ptr1 = '\0';
  return dst_str;
}

/* LCW - generate L_Data for structure and union fields - 4/15/96 */
void
PL_gen_field (L_Datalist * list, Field fld)
{
  L_Data *new_data;
  L_Expr *new_expr;
  Expr tmp_expr;
  Key tmp_key;

  new_data = L_new_data (L_INPUT_FIELD);
  new_expr = L_new_expr (L_EXPR_LABEL);

  if (fld->name != NULL)
    {
      new_expr->value.l = (char *) malloc (strlen (fld->name) + 1);
      StripDoubleQuotes (new_expr->value.l, fld->name);
    }
  else
    new_expr->value.l = NULL;

  new_data->address = new_expr;

  tmp_key.file = PL_file_scope;
  tmp_key.sym = 0;
  tmp_expr = PST_ScopeNewIntExpr(PL_symtab,
				 tmp_key,
				 fld->bit_size);
  if (fld->is_bit_field)
    new_data->value = PL_gen_expr (tmp_expr);
  P_RemoveExpr(tmp_expr);

  new_data->h_type = L_gen_type (fld->type);

  L_concat_datalist_element (list, L_new_datalist_element (new_data));
}


static void
PL_gen_dummy_field (L_Datalist *list)
{
  L_Data *new_data;  L_Expr *new_expr;
  Key chtype;

  new_data = L_new_data (L_INPUT_FIELD);
  new_expr = L_new_expr (L_EXPR_LABEL);

  new_expr->value.l = strdup ("__dummy");
  new_data->address = new_expr;

  chtype = PST_FindBasicType (PL_symtab, BT_CHAR);
  new_data->h_type = L_gen_type (chtype);

  L_concat_datalist_element (list, L_new_datalist_element (new_data));
}


/* LCW - generate L_Data for structure type - 4/15/96 */
void
PL_gen_struct (L_Datalist * list, StructDcl st)
{
  L_Data *new_data;
  L_Expr *new_expr;
  struct _Field *fldptr;
  char *stname;

  assert (st->name);
#if PL_MANGLE_NAMES
  stname = PL_mangle_name(st->name, st->key);
#else
  stname = st->name;
#endif

  new_data = L_new_data (L_INPUT_DEF_STRUCT);

  new_expr = L_new_expr (L_EXPR_LABEL);

  if (stname != NULL)
    {
      new_expr->value.l = (char *) malloc (strlen (stname) + 1);
      strcpy (new_expr->value.l, stname);
    }
  else
    {
      new_expr->value.l = NULL;
    }

  new_data->address = new_expr;

  if (strcmp (last_ms, "bss") && strcmp (last_ms, "data"))
    PL_ms (list, "bss");

  L_concat_datalist_element (list, L_new_datalist_element (new_data));

  if (!st->fields && (P_GetStructDclQualifier (st) & SQ_EMPTY))
    {
      P_warn ("Generating empty struct %s", stname ? stname : "(null)");
      PL_gen_dummy_field (list);
    }
  else
    {
      for (fldptr = st->fields; fldptr != NULL; fldptr = fldptr->next)
	PL_gen_field (list, fldptr);
    }
}

/* LCW - generate L_Data for union type - 4/15/96 */
void
PL_gen_union (L_Datalist * list, UnionDcl un)
{
  L_Data *new_data;
  L_Expr *new_expr;
  struct _Field *fldptr;
  char *unname;

  assert (un->name);
#if PL_MANGLE_NAMES
  unname = PL_mangle_name(un->name, un->key);
#else
  unname = un->name;
#endif

  new_data = L_new_data (L_INPUT_DEF_UNION);

  new_expr = L_new_expr (L_EXPR_LABEL);

  if (unname != NULL)
    {
      new_expr->value.l = (char *) malloc (strlen (unname) + 1);
      strcpy (new_expr->value.l, unname);
    }
  else
    {
      new_expr->value.l = NULL;
    }

  new_data->address = new_expr;

  if (strcmp (last_ms, "bss") && strcmp (last_ms, "data"))
    PL_ms (list, "bss");

  L_concat_datalist_element (list, L_new_datalist_element (new_data));

  for (fldptr = un->fields; fldptr != NULL; fldptr = fldptr->next)
    PL_gen_field (list, fldptr);
}

/* LCW - generate L_Data for enumerator - 4/15/96 */
void
PL_gen_enumerator (L_Datalist * list, EnumField fld)
{
  L_Data *new_data;
  L_Expr *new_expr;

  new_data = L_new_data (L_INPUT_ENUMERATOR);

  new_expr = L_new_expr (L_EXPR_LABEL);

  if (fld->name != NULL)
    {
      new_expr->value.s = (char *) malloc (strlen (fld->name) + 1);
      /*      strcpy(new_expr->value.l, fld->name); */
      StripDoubleQuotes (new_expr->value.l, fld->name);
    }
  else
    new_expr->value.l = NULL;

  new_data->address = new_expr;

  if (fld->value != NULL)
    new_data->value = PL_gen_expr (fld->value);

  L_concat_datalist_element (list, L_new_datalist_element (new_data));
}

/* LCW - generate L_Data for enum type - 4/15/96 */
void
PL_gen_enum (L_Datalist * list, EnumDcl en)
{
  L_Data *new_data;
  L_Expr *new_expr;
  struct _EnumField *fldptr;

  assert(0);

  new_data = L_new_data (L_INPUT_DEF_ENUM);

  new_expr = L_new_expr (L_EXPR_LABEL);

  if (en->name != NULL)
    {
      new_expr->value.l = (char *) malloc (strlen (en->name) + 1);
      strcpy (new_expr->value.l, en->name);
    }
  else
    new_expr->value.l = NULL;

  new_data->address = new_expr;

  if (strcmp (last_ms, "bss") && strcmp (last_ms, "data"))
    PL_ms (list, "bss");

  L_concat_datalist_element (list, L_new_datalist_element (new_data));

  for (fldptr = en->fields; fldptr != NULL; fldptr = fldptr->next)
    PL_gen_enumerator (list, fldptr);
}

/* ====================================================================== */

/*
 * JWS 20020228
 * ----------------------------------------------------------------------
 * String generation incorporated into PtoL
 */

static STRING_Symbol_Table *PL_string_table = NULL;
static int PL_string_counter = 0;

void
PL_init_strings (void)
{
  if (PL_string_table)
    STRING_delete_symbol_table (PL_string_table, free);

  PL_string_table = STRING_new_symbol_table ("PL_string_table", 512);
  PL_string_counter = 0;

}

void
PL_forget_strings (void)
{
  if (PL_string_table)
    STRING_delete_symbol_table (PL_string_table, free);

  PL_string_table = STRING_new_symbol_table ("PL_string_table", 512);
}

void
PL_deinit_strings (void)
{
  if (PL_string_table)
    STRING_delete_symbol_table (PL_string_table, free);

  PL_string_table = NULL;
  PL_string_counter = 0;
}

static int
PL_string_length (char *s)
{
  /* C string escape characters
   * ------------------------------------------------------------
   * \ddd, d an octal digit
   * \xdd. d a hex digit
   * \' \" \b \t \n \f \r \\
   */
  int len = 0;
  for (; *s != '\0'; s++)
    {
      len++;
      if (*s == '\\')
	{
	  if (*(s + 1) && isdigit (*(s + 1)) &&
	      *(s + 2) && isdigit (*(s + 2)) &&
	      *(s + 3) && isdigit (*(s + 3)))
	    s += 3;
	  else if (*(s + 1) && (*(s + 1) == 'x') &&
		   *(s + 2) && isxdigit (*(s + 2)) &&
		   *(s + 3) && isxdigit (*(s + 3)))
	    s += 3;
	  else
	    s++;
	}
    }
  return len - 1;		/* - two quotes + \0 */
}

static char *
PL_gen_string_label (void)
{
  int i;
  char *labelname;
  Dyn_str_t *fn = NULL;
  Dyn_str_t *buf = NULL;
  
  fn = PL_dstr_new(256);
  buf = PL_dstr_new(256);

  PL_dstr_sprintf (fn, F_input);

  for (i = 0; i < 512 && fn->str[i] != 0; i++)
    if (!isalnum (fn->str[i]) && (fn->str[i] != '_'))
      fn->str[i] = '_';

  PL_dstr_sprintf (buf, "__str_%d_%s__", PL_string_counter++, fn->str);
  labelname = strdup (buf->str);

  PL_dstr_free(fn);
  PL_dstr_free(buf);

  return labelname;
}

static char *
PL_dump_new_string (char *input_str)
{
  int cnt;
  char *labelname;
  L_Data *data;
  Key cp_type;
  char *str;

  str = P_AddDQ(input_str);
  labelname = PL_gen_string_label ();
  cnt = PL_string_length (str);

  if (!L_string_datalist->first_element)
    {
      if (M_arch == M_TAHOE)
	PL_ms (L_string_datalist, "rodata");
      else
	PL_ms (L_string_datalist, "data");
    }

  cp_type = PST_FindBasicType (PL_symtab, BT_CHAR);
  cp_type = PST_FindPointerToType (PL_symtab, cp_type);

  data = PL_gen_global (L_string_datalist, labelname, cp_type, 0);

  data->h_type = L_new_type ();
  data->h_type->type = L_DATA_GLOBAL | L_DATA_CHAR;
  data->h_type->dcltr = L_new_dcltr ();
  data->h_type->dcltr->method = L_D_ARRY;
  data->h_type->dcltr->index = L_new_expr (L_EXPR_INT);
  data->h_type->dcltr->index->value.i = cnt;

  PL_gen_align (L_string_datalist, 1, labelname);
  PL_gen_element_size (L_string_datalist, 1, labelname);
  PL_gen_reserve (L_string_datalist, cnt);

  data = L_new_data (L_INPUT_WS);
  data->value = L_new_expr_string (str);
  data->address = L_new_expr_label (labelname);
  L_concat_datalist_element (L_string_datalist,
			     L_new_datalist_element (data));

  free(str);
  return labelname;
}

char *
PL_get_string_label (char *str)
{
  STRING_Symbol *sym;
  char *labelname;

  if (!(sym = STRING_find_symbol (PL_string_table, str)))
    {
      labelname = PL_dump_new_string (str);
      sym = STRING_add_symbol (PL_string_table, str, labelname);
      sym->data = (void *) labelname;
    }

  return (char *) sym->data;
}
