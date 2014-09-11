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
/***************************************************************************\
 *
 *  File:  limpact_phase1_func.c
 *
 *  Description:
 *    Annotate Lcode to machine specific Lcode for the IMPACT architecture
 *
 *  Creation Date : June 1993
 *
 *  Author:  Scott A. Mahlke, Roger A. Bringmann, Richard E. Hank, 
 *           John C. Gyllenhaal, David I. August, Daniel A. Connors, 
 *           John W. Sias, Wen-mei Hwu
 *
\************************************************************************/
/* 10/29/02 REK Adding config.h */
#include <config.h>
#include <Lcode/limpact_main.h>

L_Cb *mcb = NULL;
static L_Operand *retaddr_save_operand = NULL;


void
Limpact_annotate_stack_ref (L_Oper * op, int mac, int offset, int spill)
{
  L_Attr *stack_attr;

  /* Add the stack oper flag and attribute for memory disambiguation.
     This is needed since there are no sync arcs on these stores. */

  op->flags = L_SET_BIT_FLAG (op->flags, L_OPER_STACK_REFERENCE);
  op->flags = L_SET_BIT_FLAG (op->flags, L_OPER_SAFE_PEI);

  if (!(stack_attr = L_find_attr (op->attr, "stack")))
    {
      stack_attr = L_new_attr ("stack", 2);
      op->attr = L_concat_attr (op->attr, stack_attr);

      /* base */
      stack_attr->field[0] = L_new_macro_operand (mac, L_CTYPE_INT,
						  L_PTYPE_NULL);
      /* offset */
      stack_attr->field[1] = L_new_gen_int_operand (offset);
    }
  else
    {
      stack_attr->field[0]->value.mac = mac;
      stack_attr->field[1]->value.i = offset;
    }

  if (spill)
    {
      if (!(stack_attr = L_find_attr (op->attr, "offset")))
	{
	  stack_attr = L_new_attr ("offset", 1);
	  op->attr = L_concat_attr (op->attr, stack_attr);

	  stack_attr->field[0] = L_new_gen_int_operand (offset);
	}
      else
	{
	  stack_attr->field[0]->value.i = offset;
	}
    }
  return;
}


static void
L_annotate_prologue (L_Func *fn, L_Cb *cb, L_Oper *oper)
{
  L_Oper *new_oper;
  L_Oper *mov_oper;
  int leaf;

  leaf = L_EXTRACT_BIT_VAL (fn->flags, L_FUNC_LEAF);

  new_oper = L_copy_parent_oper (oper);
  if (oper->sync_info != NULL)
    L_punt("Why does prologue have sync_info?");
    
  L_insert_oper_before (cb, oper, new_oper);
  
  if (Limpact_return_address_in_reg && !leaf)
    {
      /* save the Return Pointer if this is a non-leaf function */
      mov_oper = L_create_new_op (Lop_MOV);
      mov_oper->src[0] = L_new_macro_operand(L_MAC_RETADDR, 
					     M_native_int_register_ctype(), 
					     L_PTYPE_NULL);
      mov_oper->dest[0] = L_new_register_operand(++L_fn->max_reg_id,
						 M_native_int_register_ctype(),
						 L_PTYPE_NULL);
      retaddr_save_operand = mov_oper->dest[0];
      L_insert_oper_after (cb, new_oper, mov_oper);
    }
}

static void
L_annotate_epilogue (L_Func *fn, L_Cb *cb, L_Oper *oper)
{
  L_Oper *new_oper; 
  L_Oper *mov_oper;

  int leaf;
  
  leaf = L_EXTRACT_BIT_VAL (fn->flags, L_FUNC_LEAF);
  
  new_oper = L_copy_parent_oper (oper);
  if (oper->sync_info != NULL)
    L_punt("Why does epilogue have sync_info?");
    
  L_insert_oper_before (cb, oper, new_oper); 
  
  if (Limpact_return_address_in_reg && !leaf)
    { 
      if (!retaddr_save_operand)
	L_punt("Attempted to annotate EPILOGUE of non-leaf function without"
	       "setting retaddr_save_operand");

      /* save the Return Pointer if this is a non-leaf function */
      mov_oper = L_create_new_op (Lop_MOV);

      mov_oper->src[0] = L_copy_operand(retaddr_save_operand);
      mov_oper->dest[0] = L_new_macro_operand(L_MAC_RETADDR, 
					      M_native_int_register_ctype(), 
					      L_PTYPE_NULL);
      L_insert_oper_before (cb, new_oper, mov_oper);
    }
}

static void
L_annotate_jsr (L_Func *fn, L_Cb *cb, L_Oper *oper)
{
  L_Oper *new_oper;
 
  new_oper = L_copy_parent_oper (oper);

  if (Limpact_return_address_in_reg)
    new_oper->dest[0] = L_new_macro_operand(L_MAC_RETADDR, 
					    M_native_int_register_ctype(), 
					    L_PTYPE_NULL);
  if (oper->sync_info != NULL)
    {
      new_oper->sync_info = L_copy_sync_info (oper->sync_info);
      L_add_to_child_list (oper, new_oper);
    }

  if (oper->acc_info)
    new_oper->acc_info = L_copy_mem_acc_spec_list (oper->acc_info);

  L_insert_oper_before (cb, oper, new_oper);
}

static void
L_annotate_rts (L_Func *fn, L_Cb *cb, L_Oper *oper)
{
  L_Oper *new_oper;

  new_oper = L_copy_parent_oper (oper);
  
  if (Limpact_return_address_in_reg)
    new_oper->src[0] = L_new_macro_operand(L_MAC_RETADDR, 
  					   M_native_int_register_ctype(), 
					   L_PTYPE_NULL);

  if (oper->sync_info != NULL)
    {
      new_oper->sync_info = L_copy_sync_info (oper->sync_info);
      L_add_to_child_list (oper, new_oper);
    }

  if (oper->acc_info)
    new_oper->acc_info = L_copy_mem_acc_spec_list (oper->acc_info);

  L_insert_oper_before (cb, oper, new_oper);
}

/*
 *      THIS IS JUST A DUMMY ROUTINE FOR LIMPACT !!!
 */

void
L_annotate_oper (L_Func * fn, L_Cb * cb, L_Oper * oper)
{
  int op = oper->opc;
  L_Oper *new_oper;
  L_Operand *dest;
  int i;

  switch (op)
    {
    case Lop_PROLOGUE:
      L_annotate_prologue(fn, cb, oper);
      break;

    case Lop_EPILOGUE:
      L_annotate_epilogue(fn, cb, oper);
      break;

    case Lop_JSR:
    case Lop_JSR_FS:
      L_annotate_jsr(fn, cb, oper);
      break;

    case Lop_RTS:
    case Lop_RTS_FS:
      L_annotate_rts(fn, cb, oper);
      break;
      
    case Lop_JUMP:
    case Lop_JUMP_FS:
    case Lop_JUMP_RG:
    case Lop_JUMP_RG_FS:

    case Lop_DEFINE:

    case Lop_ALLOC:

    case Lop_BR:
    case Lop_BR_F:

    case Lop_NO_OP:

    case Lop_MOV:
    case Lop_SELECT:

    case Lop_LD_PRE_UC:
    case Lop_LD_PRE_C:
    case Lop_LD_PRE_UC2:
    case Lop_LD_PRE_C2:
    case Lop_LD_PRE_I:
    case Lop_LD_PRE_F:
    case Lop_LD_PRE_F2:

    case Lop_LD_POST_UC:
    case Lop_LD_POST_C:
    case Lop_LD_POST_UC2:
    case Lop_LD_POST_C2:
    case Lop_LD_POST_I:
    case Lop_LD_POST_F:
    case Lop_LD_POST_F2:

    case Lop_LD_UC:
    case Lop_LD_C:
    case Lop_LD_UC2:
    case Lop_LD_C2:
    case Lop_LD_I:
    case Lop_LD_F:
    case Lop_LD_F2:

    case Lop_PRED_LD:
    case Lop_PRED_ST:

    case Lop_ST_PRE_C:
    case Lop_ST_PRE_C2:
    case Lop_ST_PRE_I:
    case Lop_ST_PRE_F:
    case Lop_ST_PRE_F2:

    case Lop_ST_POST_C:
    case Lop_ST_POST_C2:
    case Lop_ST_POST_I:
    case Lop_ST_POST_F:
    case Lop_ST_POST_F2:

    case Lop_ST_C:
    case Lop_ST_C2:
    case Lop_ST_I:
    case Lop_ST_F:
    case Lop_ST_F2:

    case Lop_ADD:
    case Lop_ADD_U:
    case Lop_SUB:
    case Lop_SUB_U:

    case Lop_OR:
    case Lop_NOR:
    case Lop_AND:
    case Lop_NAND:
    case Lop_XOR:
    case Lop_NXOR:
    case Lop_AND_NOT:
    case Lop_OR_NOT:
    case Lop_AND_COMPL:
    case Lop_OR_COMPL:

    case Lop_RCMP:
    case Lop_RCMP_F:
    case Lop_CMP:
    case Lop_CMP_F:

    case Lop_MUL:
    case Lop_MUL_U:
    case Lop_REM:
    case Lop_REM_U:
    case Lop_DIV:
    case Lop_DIV_U:
    case Lop_MUL_ADD:
    case Lop_MUL_ADD_U:
    case Lop_MUL_SUB:
    case Lop_MUL_SUB_U:

    case Lop_LSL:
    case Lop_LSR:
    case Lop_ASR:

    case Lop_MOV_F:
    case Lop_MOV_F2:
    case Lop_SELECT_F:
    case Lop_SELECT_F2:

    case Lop_ADD_F:
    case Lop_ADD_F2:
    case Lop_SUB_F2:
    case Lop_SUB_F:
    case Lop_MUL_F:
    case Lop_MUL_F2:
    case Lop_DIV_F:
    case Lop_DIV_F2:
    case Lop_MUL_ADD_F:
    case Lop_MUL_SUB_F:
    case Lop_MUL_ADD_F2:
    case Lop_MUL_SUB_F2:

    case Lop_MUL_SUB_REV:
    case Lop_MUL_SUB_REV_U:
    case Lop_MUL_SUB_REV_F:
    case Lop_MUL_SUB_REV_F2:

    case Lop_ABS:
    case Lop_ABS_F:
    case Lop_ABS_F2:

    case Lop_F2_F:
    case Lop_F_F2:
    case Lop_F2_I:
    case Lop_F_I:
    case Lop_I_F2:
    case Lop_I_F:

    case Lop_PREF_LD:

    case Lop_MEM_COPY:
    case Lop_MEM_COPY_BACK:
    case Lop_MEM_COPY_CHECK:
    case Lop_MEM_COPY_RESET:
    case Lop_MEM_COPY_SETUP:
    case Lop_MEM_COPY_TAG:

    case Lop_CHECK:

    case Lop_INTRINSIC: /* ITI/JWJ 7/99 */

      /* next 10 cases by: ITI/JWJ 8.11.1999 */
    case Lop_L_MAC:
    case Lop_L_MSU:
    case Lop_ADD_SAT:
    case Lop_ADD_SAT_U:
    case Lop_SUB_SAT:
    case Lop_SUB_SAT_U:
    case Lop_MUL_SAT:
    case Lop_MUL_SAT_U:
    case Lop_SAT:
    case Lop_SAT_U:

    case Lop_EXTRACT_C:
    case Lop_EXTRACT_C2:
    case Lop_EXTRACT:
    case Lop_EXTRACT_U:
    case Lop_DEPOSIT:
    case Lop_SXT_C:
    case Lop_SXT_C2:
    case Lop_ZXT_C:
    case Lop_ZXT_C2:

      new_oper = L_copy_parent_oper (oper);
      if (oper->sync_info != NULL)
        {
          new_oper->sync_info = L_copy_sync_info (oper->sync_info);
          L_add_to_child_list (oper, new_oper);
        }

      if (oper->acc_info)
	new_oper->acc_info = L_copy_mem_acc_spec_list (oper->acc_info);

      L_insert_oper_before (cb, oper, new_oper);
      break;

    case Lop_PRED_CLEAR:
    case Lop_PRED_SET:
    case Lop_PRED_COPY:

      if (Limpact_convert_init_pred_to_uncond_defs)
        {
          for (i = 0; i < L_max_dest_operand; i++)
            {
              dest = oper->dest[i];
              if (dest == NULL)
                continue;
              if (dest->ptype != L_PTYPE_NULL)
		{
		  DB_print_oper(oper);
		  L_punt ("L_annotate_oper: init pred (op %d) with non-NULL "
			  "ptype found.", oper->id);
		}

              if (L_debug_messages)
                fprintf (stdout, "Change r %d (op %d) to uncond predicate\n",
                         dest->value.r, oper->id);

              new_oper = L_create_new_op (Lop_CMP);
              new_oper->com[0] = L_CTYPE_INT;
              new_oper->com[1] = Lcmp_COM_EQ;
              new_oper->pred[0] = L_copy_operand (oper->pred[0]);
              new_oper->pred[1] = L_copy_operand (oper->pred[1]);
              new_oper->dest[0] = L_copy_operand (dest);
              new_oper->src[0] = L_new_gen_int_operand (0);
              new_oper->src[1] = L_new_gen_int_operand (0);

              if (L_pred_clear (oper))
                {
                  new_oper->dest[0]->ptype = L_PTYPE_UNCOND_F;
                }
              else if (L_pred_set (oper))
                {
                  new_oper->dest[0]->ptype = L_PTYPE_UNCOND_T;
                }
              else if (L_pred_copy (oper))
                {
		  if (new_oper->pred[0])
		    {
		      DB_print_oper(oper);
		      L_punt ("L_annotate_oper: "
			      "pred copy is predicated!: op %d\n", oper->id);
		    }
                  new_oper->dest[0]->ptype = L_PTYPE_UNCOND_T;
		  new_oper->pred[0] = L_copy_operand(oper->src[0]);
                  new_oper->pred[0]->ptype = L_PTYPE_NULL;
                }
              else
                {
                  L_punt ("L_annotate_oper: "
                          "opc not supported: op %d\n", oper->id);
                }

              new_oper->parent_op = oper;

              L_insert_oper_before (cb, oper, new_oper);
            }
        }
      else
        {
          new_oper = L_copy_parent_oper (oper);
          if (oper->sync_info != NULL)
            {
              new_oper->sync_info = L_copy_sync_info (oper->sync_info);
              L_add_to_child_list (oper, new_oper);
            }

	  if (oper->acc_info)
	    new_oper->acc_info = L_copy_mem_acc_spec_list (oper->acc_info);

          L_insert_oper_before (cb, oper, new_oper);
        }
      break;

    default:
      L_print_oper (stderr, oper);
      L_punt ("annotate_oper: unsupported or unrecognized opcode",
              L_ERR_INTERNAL);
    }
}

#if 0 
/* Deprecated - RDB 12/01 */

/*
 * Annotate varargs.  This is adapted from Lhppa.
 * JWS 3/26/98
 */
static void
L_add_vararg_stores (L_Func * fn, L_Attr * attr)
{
  L_Oper *store, *new_oper;
  int i, store_offset, store_preg;

  if (attr != NULL)
    {
      if (attr->field[0]->value.i >= 0)
        {
          store_offset = -4;
          store_preg = L_MAC_P0;
        }
      else
        {
          store_offset = attr->field[0]->value.i + 4;
          switch (store_offset)
            {
            case -4:
              store_preg = L_MAC_P0;
              break;
            case -8:
              store_preg = L_MAC_P1;
              break;
            case -12:
              store_preg = L_MAC_P2;
              break;
            }
        }
    }
  else
    {
      store_offset = -4;
      store_preg = L_MAC_P0;
    }

  if (store_offset <= -16)
    return;                     /* nothing need be done */

  /* Find the store operation and add the missing ones */

  for (store = fn->first_cb->first_op; store != NULL; store = store->next_op)
    {

      if (store->opc == Lop_ST_I &&
          L_is_int_constant (store->src[0]) &&
          store->src[0]->value.i == store_offset &&
          L_is_macro (store->src[1]) &&
          (store->src[1]->value.mac == L_MAC_IP) &&
          L_is_macro (store->src[2]) &&
          store->src[2]->value.mac == store_preg)
        break;
    }
  if (store == NULL)
    {
      L_warn ("L_annotate_varargs couldn't find varargs store in first cb\n");
      return;
    }

  for (i = store_offset - 4; i >= -16; i -= 4)
    {

      new_oper = L_create_new_op_using (Lop_ST_I, store->parent_op);
      new_oper->src[0] = L_new_gen_int_operand (i);
      new_oper->src[1] = L_new_macro_operand (L_MAC_IP, L_CTYPE_INT, 0);
      new_oper->src[2] = L_new_macro_operand (++store_preg, L_CTYPE_INT, 0);

      L_insert_oper_after (fn->first_cb, store, new_oper);

      /* REH - 4/18/96 Also need to insert Lop_DEFINE      */
      /* operations to indicate that the parms are live-in */
      new_oper = L_create_new_op (Lop_DEFINE);
      new_oper->dest[0] = L_new_macro_operand (store_preg, L_CTYPE_INT, 0);
      L_insert_oper_after (fn->first_cb, fn->first_cb->first_op, new_oper);
    }
}

static void
L_annotate_builtin_va_start (L_Func * fn, L_Cb * cb)
{
  L_Oper *jsr, *def_p0, *def_p1, *new_oper;
  L_Operand *dest_operand;
  L_Attr *attr;

  /* Five steps are required to correct the varargs     */
  /* emitted by IMPACT                                  */
  /* 1)  Find and delete the jsr to ___builtin_va_start */
  /* 2)  Find the first definition of $P0, assumed to be */
  /*     int the form $P0 <- (r x i)                    */
  /* 3)  Find the first defeintion of $P1, delete it    */
  /* 4)  If attr "use_ret_as_parm0" not present:        */
  /*        Add a mov:  (r x i) <- $FP                  */
  /*     Otherwise, if attr present -ITI/JCG 4/99       */
  /*        Add a mov:  (r P15 i) <- $FP                */
  /* 5)  Find the st_i $FP,-4,$P0 instruction in the    */
  /*     first cb and add 3 more after it               */
  /*        st_i $FP, -8,$P1                            */
  /*        st_i $FP,-12,$P2                            */
  /*        st_i $FP,-16,$P3                            */

  /* Locate jsr */

  for (jsr = cb->first_op; jsr != NULL; jsr = jsr->next_op)
    {
      if ((jsr->opc == Lop_JSR || jsr->opc == Lop_JSR_FS) &&
          (!strcmp (jsr->src[0]->value.l, "___builtin_va_start") ||
           !strcmp (jsr->src[0]->value.l, "_$fn___builtin_va_start")))
        break;
    }
  if (jsr == NULL)
    L_punt
      ("L_annotate_varargs couldn't find jsr to ___builtin_va_start in cb %d\n",
       cb->id);

  /* Locate first definition of $P0 */

  for (def_p0 = jsr->prev_op; def_p0 != NULL; def_p0 = def_p0->prev_op)
    {
      if (def_p0->dest[0] &&
          L_is_macro (def_p0->dest[0]) &&
          def_p0->dest[0]->value.mac == L_MAC_P0)
        break;
    }
  if (def_p0 == NULL)
    L_punt ("L_annotate_varargs couldn't find definition of $P0\n");

  /* If "use_ret_as_parm0" not present, use def_p0->src[0] as dest
   * operand (old patchup code required) -ITI/JCG 4/99
   */
  if (L_find_attr (jsr->attr, "use_ret_as_parm0") == NULL)
    {
      dest_operand = def_p0->src[0];
    }
  /* Otherwise, use return register as dest operand (patch required
   * for code generated from source as of 4/7/99) -ITI/JCG 4/99
   */
  else
    {
      if ((attr = L_find_attr (jsr->attr, "ret")) == NULL)
        {
          L_punt ("L_annotate_varargs couldn't find ret attr!\n");
        }
      dest_operand = attr->field[0];
    }

  /* Locate first definition of $P1 */
  for (def_p1 = jsr->prev_op; def_p1 != NULL; def_p1 = def_p1->prev_op)
    {
      if (def_p1->dest[0] &&
          L_is_macro (def_p1->dest[0]) &&
          def_p1->dest[0]->value.mac == L_MAC_P1)
        break;
    }
  if (def_p1 == NULL)
    L_punt ("L_annotate_varargs couldn't find definition of $P1\n");

  /* Create new instruction */

  if ((attr = L_find_attr (fn->attr, "VARARG")) != NULL)
    {
      if (attr->field[0]->value.i >= 0)
        {
          new_oper = L_create_new_op_using (Lop_MOV, jsr->parent_op);
          new_oper->src[0] = L_new_macro_operand (L_MAC_IP, L_CTYPE_INT, 0);
          new_oper->dest[0] = L_copy_operand (dest_operand);
          L_insert_oper_after (cb, jsr, new_oper);
        }
      else
        {
          new_oper = L_create_new_op_using (Lop_ADD, jsr->parent_op);
          new_oper->src[0] =
            L_new_gen_int_operand (attr->field[0]->value.i + 4);
          new_oper->src[1] = L_new_macro_operand (L_MAC_IP, L_CTYPE_INT, 0);
          new_oper->dest[0] = L_copy_operand (dest_operand);
          L_insert_oper_after (cb, jsr, new_oper);
        }
    }
  else
    {
      new_oper = L_create_new_op_using (Lop_MOV, jsr->parent_op);
      new_oper->src[0] = L_new_macro_operand (L_MAC_IP, L_CTYPE_INT, 0);
      new_oper->dest[0] = L_copy_operand (dest_operand);
      L_insert_oper_after (cb, jsr, new_oper);
    }

  /* Delete opers */
  L_delete_oper (cb, jsr);
  L_delete_oper (cb, def_p0);
  L_delete_oper (cb, def_p1);
}
#endif

/******************************************************************************\
 *
 * L_process_func - annotate an Lcode function
 *
 * To ensure correct ordering of new Mopers within an Lcode function
 * we will annotate the Lcode oper into a list of Mcode opers [1 or more
 * Mcode opers].  This Mcode list of opers will be inserted on at a time
 * into the control block of the Lcode oper just before it.
 *
\******************************************************************************/
void
L_process_func (L_Func * fn)
{
  /* initializations */
  L_Oper *oper;
  L_Cb *cb;

  if (L_do_software_pipelining)
    {
      Lpipe_softpipe_loop_prep (fn);
    }
  else
    {
      for (cb = fn->first_cb; cb != NULL; cb = cb->next_cb)
        cb->flags = L_CLR_BIT_FLAG (cb->flags, L_CB_SOFTPIPE);
    }

  if (L_debug_messages)
    fprintf (stderr, "Annotating %s\n", fn->name);

  /*
   * To maintain sync arcs, we must re-link all of the child opers
   * together after annotation.  To facilitate this, we maintain an
   * array of child pointers, indexed by parent id. -DAC 9/14/97
   */
  L_init_child_list (fn);

  /* Split branches so every branch becomes predicate define and jump */
  if (Limpact_do_branch_split)
    {
      L_branch_split_func (fn);
    }

  /*
   * Initialize appropriate function variables to ensure correct
   * memory cleanup.  This is only needed when you are going to
   * create new parent Lcode operations which only occurs in phase 1
   * so far. 
   */
  L_mcode_init_function ();

  /* Loop through all of the control blocks within the function */
  for (cb = fn->first_cb; cb != NULL; cb = cb->next_cb)
    {
      /*
       * Perform Lcode to Lcode transformations to simplify code annotation
       * process.  Some of these also relate to performance improvements
       * (ie strength reduction).
       */
      mcb = cb;

      /* Loop through all of the Lcode opers within a control block */
      oper = cb->first_op;
      while (oper != NULL)
        {
          /* Annotate the current Lcode oper */
          L_annotate_oper (fn, cb, oper);

          /* Convert the Lcode oper to a parent */
          oper = L_convert_to_parent (mcb, oper);
        }
    }

  /* Walks through fn and points all sync arcs to other children, 
     not parents */
  L_relink_child_sync_arcs (fn);
  L_deinit_child_list (fn);

  if (Limpact_annotate_varargs)
    Limpact_adjust_fp_parameter_passing (fn); 
}


/*
 * Global initializations
 */

void
L_init (Parm_Macro_List * command_line_macro_list)
{
  if (L_do_software_pipelining)
    Lpipe_loop_prep_init (command_line_macro_list);
  return;
}

void
L_cleanup ()
{
  if (L_do_software_pipelining)
    Lpipe_loop_prep_end ();
  return;
}
