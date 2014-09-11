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
/*************************************************************************\
 *
 *  File:  limpact_phase2_reg.c
 *
 *  Description:
 *    Interface to register allocator for IMPACT code generator
 *
 *  Creation Date :  June 1993
 *
 *  Author:  Scott A. Mahlke, Roger A. Bringmann, Richard E. Hank, 
 *           John C. Gyllenhaal, Wen-mei Hwu
 *
 *
\************************************************************************/
/* 10/29/02 REK Adding config.h */
#include <config.h>
#include <Lcode/limpact_main.h>

int *caller_prd_reg_map;
int *callee_prd_reg_map;
int *caller_int_reg_map;
int *callee_int_reg_map;
int *caller_flt_reg_map;
int *callee_flt_reg_map;
int *caller_dbl_reg_map;
int *callee_dbl_reg_map;

int num_flt_callee_reg;

Set caller_int_set = NULL;
Set callee_int_set = NULL;
Set caller_float_set = NULL;
Set callee_float_set = NULL;
Set caller_double_set = NULL;
Set callee_double_set = NULL;
Set caller_predicate_set = NULL;
Set callee_predicate_set = NULL;


/*
 *      Just check if function hyperblock flag is set correctly
 *      so we can handle old files, can get rid of this eventually.
 */
static void
L_check_func_hyperblock_flag (L_Func * fn)
{
  int func_flag, cb_flag;
  L_Cb *cb;

  cb_flag = 0;
  for (cb = fn->first_cb; cb != NULL; cb = cb->next_cb)
    {
      if (L_EXTRACT_BIT_VAL (cb->flags, L_CB_HYPERBLOCK))
        {
          cb_flag = 1;
          break;
        }
    }

  func_flag = L_EXTRACT_BIT_VAL (fn->flags, L_FUNC_HYPERBLOCK);
  if (cb_flag != func_flag)
    {
      fprintf (stderr,
               "Limpact: WARNING - hyperblock func flag not correct!\n");
      if (cb_flag)
        {
          fn->flags = L_SET_BIT_FLAG (fn->flags, L_FUNC_HYPERBLOCK);
        }
      else
        {
          fn->flags = L_CLR_BIT_FLAG (fn->flags, L_FUNC_HYPERBLOCK);
        }
    }
}

/* Hack to fixup predicate attributes for spill code inserted SAM 10-94 */
static void
L_fix_vpred_attrs (L_Func * fn)
{
  L_Cb *cb;
  L_Oper *oper, *ptr;
  L_Attr *attr, *new_attr;

  for (cb = fn->first_cb; cb != NULL; cb = cb->next_cb)
    {
      if (!L_EXTRACT_BIT_VAL (cb->flags, L_CB_HYPERBLOCK))
        continue;
      for (oper = cb->first_op; oper != NULL; oper = oper->next_op)
        {
          if (!L_EXTRACT_BIT_VAL (oper->flags, L_OPER_SPILL_CODE))
            continue;
          if (oper->pred[0] == NULL)
            continue;

          if (L_store_opcode (oper))
            {
              /* search up first */
              for (ptr = oper->prev_op; ptr != NULL; ptr = ptr->prev_op)
                {
                  if (L_EXTRACT_BIT_VAL (ptr->flags, L_OPER_SPILL_CODE))
                    continue;
                  if (L_same_operand (oper->pred[0], ptr->pred[0]))
                    break;
                }
              if (ptr == NULL)
                {               /* try down */
                  for (ptr = oper->next_op; ptr != NULL; ptr = ptr->next_op)
                    {
                      if (L_EXTRACT_BIT_VAL (ptr->flags, L_OPER_SPILL_CODE))
                        continue;
                      if (L_same_operand (oper->pred[0], ptr->pred[0]))
                        break;
                    }
                }
              if (ptr == NULL)
                L_punt ("L_fix_vpred_attrs: no match found for op %d",
                        oper->id);

              attr = L_find_attr (ptr->attr, L_VPRED_PRD_ATTR_NAME);
              if (attr == NULL)
                L_punt ("L_fix_vpred_attrs: no vpred attr found");
              new_attr = L_copy_attr_element (attr);
              oper->attr = L_concat_attr (oper->attr, new_attr);
            }

          else if (L_load_opcode (oper))
            {
              /* look down first */
              for (ptr = oper->next_op; ptr != NULL; ptr = ptr->next_op)
                {
                  if (L_EXTRACT_BIT_VAL (ptr->flags, L_OPER_SPILL_CODE))
                    continue;
                  if (L_same_operand (oper->pred[0], ptr->pred[0]))
                    break;
                }
              if (ptr == NULL)
                {               /* try up */
                  for (ptr = oper->prev_op; ptr != NULL; ptr = ptr->prev_op)
                    {
                      if (L_EXTRACT_BIT_VAL (ptr->flags, L_OPER_SPILL_CODE))
                        continue;
                      if (L_same_operand (oper->pred[0], ptr->pred[0]))
                        break;
                    }
                }
              if (ptr == NULL)
                L_punt
                  ("L_fix_vpred_attrs: no match found for op %d, oper->id");

              attr = L_find_attr (ptr->attr, L_VPRED_PRD_ATTR_NAME);
              if (attr == NULL)
                L_punt ("L_fix_vpred_attrs: no vpred attr found");
              new_attr = L_copy_attr_element (attr);
              oper->attr = L_concat_attr (oper->attr, new_attr);
            }

          else
            {
              L_punt ("L_fix_vpred_attrs: illegal spill oper");
            }
        }
    }
}

#if 0
static void
L_unallocate_pred_regs (L_Func * fn)
{
  int i;
  L_Cb *cb;
  L_Oper *oper;
  L_Attr *attr;
  L_Operand *field, *dest;

  for (cb = fn->first_cb; cb != NULL; cb = cb->next_cb)
    {
      if (!L_EXTRACT_BIT_VAL (cb->flags, L_CB_HYPERBLOCK))
        continue;
      for (oper = cb->first_op; oper != NULL; oper = oper->next_op)
        {
          if (oper->pred[0] != NULL)
            {
              attr = L_find_attr (oper->attr, L_VPRED_PRD_ATTR_NAME);
              if (attr == NULL)
                L_punt ("L_unallocate_pred_regs: no VPP attr for op %d",
                        oper->id);
              field = attr->field[0];
              if (!L_is_register (field))
                L_punt ("L_unallocate_pred_regs: corrupt VPP attr for op %d",
                        oper->id);
              L_delete_operand (oper->pred[0]);
              oper->pred[0] = L_new_register_operand (field->value.r + 2048,
                                                      L_CTYPE_PREDICATE,
                                                      L_PTYPE_NULL);
            }
          for (i = 0; i < L_max_dest_operand; i++)
            {
              dest = oper->dest[i];
              if (!L_is_register (dest))
                continue;
              if (!L_is_ctype_predicate (dest))
                continue;
              attr = L_find_attr (oper->attr, L_VPRED_DST_ATTR_NAME);
              if (attr == NULL)
                L_punt ("L_unallocate_pred_regs: no VPD attr for op %d",
                        oper->id);
              field = attr->field[i];
              if (!L_is_register (field))
                L_punt ("L_unallocate_pred_regs: corrupt VPD attr for op %d",
                        oper->id);
              L_delete_operand (dest);
              oper->dest[i] = L_new_register_operand (field->value.r + 2048,
                                                      L_CTYPE_PREDICATE,
                                                      field->ptype);
            }
        }
    }
}
#endif

/* 
 * Impact assumes that there is a 64-bit port to memory for
 * both loads and stores.
 */
#define LOAD_COST       1.0
#define STORE_COST      1.0

double
R_callee_cost (int ctype, int leaf, int callee_allocated)
{
  double cost = 0.0;
  int loads_per_cycle, stores_per_cycle;

  /* currently assumes uniform model */
  loads_per_cycle = mdes_total_slots ();
  stores_per_cycle = mdes_total_slots ();

  switch (ctype)
    {
    case L_CTYPE_INT:
    case L_CTYPE_FLOAT:
    case L_CTYPE_DOUBLE:
    case L_CTYPE_BTR:
    case L_CTYPE_POINTER:
      cost = LOAD_COST / loads_per_cycle + STORE_COST / stores_per_cycle;
      break;
    case L_CTYPE_PREDICATE:
      cost = 0;
      break;
    default:
      L_punt ("R_callee_cost: invalid ctype of %d", ctype);
    }

  return cost;
}

double
R_caller_cost (int ctype, int leaf)
{
  double cost = 0.0;
  int loads_per_cycle, stores_per_cycle;

  if (leaf)
    return 0;

  /* currently assumes uniform model */
  loads_per_cycle = mdes_total_slots ();
  stores_per_cycle = mdes_total_slots ();

  switch (ctype)
    {
    case L_CTYPE_INT:
    case L_CTYPE_FLOAT:
    case L_CTYPE_DOUBLE:
    case L_CTYPE_BTR:
    case L_CTYPE_POINTER:
      cost = LOAD_COST / loads_per_cycle + STORE_COST / stores_per_cycle;
      break;
    case L_CTYPE_PREDICATE:
      cost = 0;
      break;

    default:
      L_punt ("R_caller_cost: invalid ctype of %d", ctype);
    }

  return cost;
}

double
R_spill_load_cost (int ctype)
{
  double cost = 0.0;
  int loads_per_cycle;

  /* currently assumes uniform model */
  loads_per_cycle = mdes_total_slots ();

  switch (ctype)
    {
    case L_CTYPE_INT:
    case L_CTYPE_FLOAT:
    case L_CTYPE_DOUBLE:
    case L_CTYPE_BTR:
    case L_CTYPE_POINTER:
      cost = LOAD_COST;
      break;
    case L_CTYPE_PREDICATE:
      cost = LOAD_COST;
      break;

    default:
      L_punt ("R_spill_load_cost: invalid ctype of %d", ctype);
    }

  return cost;
}

double
R_spill_store_cost (int ctype)
{
  double cost = 0.0;
  int stores_per_cycle;

  /* currently assumes uniform model */
  stores_per_cycle = mdes_total_slots ();

  switch (ctype)
    {
    case L_CTYPE_INT:
    case L_CTYPE_FLOAT:
    case L_CTYPE_DOUBLE:
    case L_CTYPE_BTR:
    case L_CTYPE_POINTER:
      cost = STORE_COST;
      break;
    case L_CTYPE_PREDICATE:
      cost = STORE_COST;
      break;

    default:
      L_punt ("R_spill_store_cost: invalid ctype of %d", ctype);
    }

  return cost;
}

L_Oper *
O_fill_reg (int reg, int type, L_Operand * operand, int fill_offset,
            L_Operand ** pred, int type_flag)
{
  int op = 0, i;
  L_Oper *new_oper;
  L_Attr *attr;

  switch (L_operand_case_ctype (operand))
    {
    case L_CTYPE_INT:
      op = Lop_LD_I;
      break;
    case L_CTYPE_FLOAT:
      op = Lop_LD_F;
      break;
    case L_CTYPE_DOUBLE:
      op = Lop_LD_F2;
      break;
    case L_CTYPE_PREDICATE:
      op = Lop_PRED_LD;
      break;
    default:
      L_punt ("O_fill_reg: unsupported register type",
              L_return_old_ctype (operand));
    }

  new_oper = L_create_new_op (op);
  new_oper->flags =
    L_SET_BIT_FLAG (new_oper->flags, L_OPER_SPILL_CODE | L_OPER_SAFE_PEI);
  if (
      (M_impact_model == M_IM_SPARC_LCODE
       || M_impact_model == M_IM_SPARC_MCODE))
    {
      /* Lsparc likes the integer in src[1] */
      new_oper->src[1] = L_new_gen_int_operand (fill_offset);
      new_oper->src[0] =
        L_new_macro_operand (L_MAC_SP, L_CTYPE_INT, L_PTYPE_NULL);
    }
  else
    {
      new_oper->src[0] = L_new_gen_int_operand (fill_offset);
      new_oper->src[1] =
        L_new_macro_operand (L_MAC_SP, L_CTYPE_INT, L_PTYPE_NULL);
    }
  if (L_is_reg_direct (type))
    new_oper->dest[0] =
      L_new_register_operand (reg, L_return_old_ctype (operand),
                              L_PTYPE_NULL);
  else
    new_oper->dest[0] =
      L_new_macro_operand (reg, L_return_old_ctype (operand), L_PTYPE_NULL);

  if (pred != NULL)
    {
      for (i = 0; i < L_max_pred_operand; i++)
        {
          new_oper->pred[i] = L_copy_operand (pred[i]);
        }
      if ((operand->ptype == L_PTYPE_UNCOND_T) |
          (operand->ptype == L_PTYPE_UNCOND_F))
        {
          L_delete_operand (new_oper->pred[0]);
          new_oper->pred[0] = NULL;
        }
    }

  /* flag load as being inserted by the register allocator */
  attr = L_new_attr ("regalloc1", 1);
  L_set_int_attr_field (attr, 0, type_flag);
  new_oper->attr = L_concat_attr (new_oper->attr, attr);

  /* Add the spill offset attribute */
  attr = L_new_attr ("offset", 1);
  L_set_int_attr_field (attr, 0, fill_offset);
  new_oper->attr = L_concat_attr (new_oper->attr, attr);

  return (new_oper);
}

L_Oper *
O_spill_reg (int reg, int type, L_Operand * operand, int spill_offset,
             L_Operand ** pred, int type_flag)
{
  int op = 0, i;
  L_Oper *new_oper;
  L_Attr *attr;

  switch (L_operand_case_ctype (operand))
    {
    case L_CTYPE_INT:
      op = Lop_ST_I;
      break;
    case L_CTYPE_PREDICATE:
      op = Lop_PRED_ST;
      break;
    case L_CTYPE_FLOAT:
      op = Lop_ST_F;
      break;
    case L_CTYPE_DOUBLE:
      op = Lop_ST_F2;
      break;
    default:
      L_punt ("O_spill_reg: unsupported register type",
              L_return_old_ctype (operand));
    }
  new_oper = L_create_new_op (op);
  new_oper->flags =
    L_SET_BIT_FLAG (new_oper->flags, L_OPER_SPILL_CODE | L_OPER_SAFE_PEI);
  if (
      (M_impact_model == M_IM_SPARC_LCODE
       || M_impact_model == M_IM_SPARC_MCODE))
    {
      /* Lsparc likes the integer in src[1] */
      new_oper->src[1] = L_new_gen_int_operand (spill_offset);
      new_oper->src[0] =
        L_new_macro_operand (L_MAC_SP, L_CTYPE_INT, L_PTYPE_NULL);
    }
  else
    {
      new_oper->src[0] = L_new_gen_int_operand (spill_offset);
      new_oper->src[1] =
        L_new_macro_operand (L_MAC_SP, L_CTYPE_INT, L_PTYPE_NULL);
    }
  if (L_is_reg_direct (type))
    new_oper->src[2] =
      L_new_register_operand (reg, L_return_old_ctype (operand),
                              L_PTYPE_NULL);
  else
    new_oper->src[2] =
      L_new_macro_operand (reg, L_return_old_ctype (operand), L_PTYPE_NULL);

  if (pred != NULL)
    {
      for (i = 0; i < L_max_pred_operand; i++)
        {
          new_oper->pred[i] = L_copy_operand (pred[i]);
        }
      if ((operand->ptype == L_PTYPE_UNCOND_T) |
          (operand->ptype == L_PTYPE_UNCOND_F))
        {
          L_delete_operand (new_oper->pred[0]);
          new_oper->pred[0] = NULL;
        }
    }

  /* flag store as being inserted by the register allocator */
  attr = L_new_attr ("regalloc1", 1);
  L_set_int_attr_field (attr, 0, type_flag);
  new_oper->attr = L_concat_attr (new_oper->attr, attr);

  /* Add the spill offset attribute */
  attr = L_new_attr ("offset", 1);
  L_set_int_attr_field (attr, 0, spill_offset);
  new_oper->attr = L_concat_attr (new_oper->attr, attr);

  return (new_oper);
}

L_Oper *
O_jump_oper (int opc, L_Cb * dest_cb)
{
  L_Oper *new_oper = L_create_new_op (opc);
  new_oper->src[0] = L_new_cb_operand (dest_cb);

  return (new_oper);
}

static L_Oper *
new_load_operation (int index, int type, int offset, int type_flag)
{
  L_Oper *oper = NULL;
  L_Attr *attr;

  switch (type)
    {
    case L_CTYPE_PREDICATE:
      oper = L_create_new_op (Lop_PRED_LD);
      oper->dest[0] = L_new_register_operand (index, L_CTYPE_PREDICATE,
                                              L_PTYPE_NULL);
      oper->src[0] = L_new_gen_int_operand (offset);
      oper->src[1] =
        L_new_macro_operand (L_MAC_SP, L_CTYPE_INT, L_PTYPE_NULL);
      break;
    case L_CTYPE_INT:
      oper = L_create_new_op (Lop_LD_I);
      oper->dest[0] =
        L_new_register_operand (index, L_CTYPE_INT, L_PTYPE_NULL);
      oper->src[0] = L_new_gen_int_operand (offset);
      oper->src[1] =
        L_new_macro_operand (L_MAC_SP, L_CTYPE_INT, L_PTYPE_NULL);
      break;
    case L_CTYPE_FLOAT:
    case L_CTYPE_DOUBLE:
      oper = L_create_new_op (Lop_LD_F2);
      oper->dest[0] =
        L_new_register_operand (index, L_CTYPE_DOUBLE, L_PTYPE_NULL);
      oper->src[0] = L_new_gen_int_operand (offset);
      oper->src[1] =
        L_new_macro_operand (L_MAC_SP, L_CTYPE_INT, L_PTYPE_NULL);
      break;
    }

  oper->flags =
    L_SET_BIT_FLAG (oper->flags, L_OPER_SPILL_CODE | L_OPER_SAFE_PEI);

  /* Add attribute making as inserted by register allocator */
  attr = L_new_attr ("regalloc1", 1);
  L_set_int_attr_field (attr, 0, type_flag);
  oper->attr = L_concat_attr (oper->attr, attr);

  /* Add the spill code offset attribute */
  attr = L_new_attr ("offset", 1);
  L_set_int_attr_field (attr, 0, offset);
  oper->attr = L_concat_attr (oper->attr, attr);

  return oper;
}

static L_Oper *
new_store_operation (int index, int type, int offset, int type_flag)
{
  L_Oper *oper = NULL;
  L_Attr *attr;

  switch (type)
    {
    case L_CTYPE_PREDICATE:
      oper = L_create_new_op (Lop_PRED_ST);
      oper->src[0] = L_new_gen_int_operand (offset);
      oper->src[1] =
        L_new_macro_operand (L_MAC_SP, L_CTYPE_INT, L_PTYPE_NULL);
      oper->src[2] =
        L_new_register_operand (index, L_CTYPE_PREDICATE, L_PTYPE_NULL);
      break;
    case L_CTYPE_INT:
      oper = L_create_new_op (Lop_ST_I);
      oper->src[0] = L_new_gen_int_operand (offset);
      oper->src[1] =
        L_new_macro_operand (L_MAC_SP, L_CTYPE_INT, L_PTYPE_NULL);
      oper->src[2] =
        L_new_register_operand (index, L_CTYPE_INT, L_PTYPE_NULL);
      break;
    case L_CTYPE_FLOAT:
    case L_CTYPE_DOUBLE:
      oper = L_create_new_op (Lop_ST_F2);
      oper->src[0] = L_new_gen_int_operand (offset);
      oper->src[1] =
        L_new_macro_operand (L_MAC_SP, L_CTYPE_INT, L_PTYPE_NULL);
      oper->src[2] =
        L_new_register_operand (index, L_CTYPE_DOUBLE, L_PTYPE_NULL);
      break;
    }

  oper->flags =
    L_SET_BIT_FLAG (oper->flags, L_OPER_SPILL_CODE | L_OPER_SAFE_PEI);

  /* Add attribute making as inserted by register allocator */
  attr = L_new_attr ("regalloc1", 1);
  L_set_int_attr_field (attr, 0, type_flag);
  oper->attr = L_concat_attr (oper->attr, attr);

  /* Add the spill code offset attribute */
  attr = L_new_attr ("offset", 1);
  L_set_int_attr_field (attr, 0, offset);
  oper->attr = L_concat_attr (oper->attr, attr);

  return oper;
}

static L_Oper *
new_blk_load_operation (int offset, int type_flag)
{
  L_Oper *oper;
  L_Attr *attr;

  oper = L_create_new_op (Lop_PRED_LD_BLK);
  oper->dest[0] = L_new_macro_operand (L_MAC_PRED_ALL, L_CTYPE_PREDICATE,
                                       L_PTYPE_NULL);
  oper->src[0] = L_new_gen_int_operand (offset);
  oper->src[1] = L_new_macro_operand (L_MAC_SP, L_CTYPE_INT, L_PTYPE_NULL);

  oper->flags =
    L_SET_BIT_FLAG (oper->flags, L_OPER_SPILL_CODE | L_OPER_SAFE_PEI);

  /* Add attribute making as inserted by register allocator */
  attr = L_new_attr ("regalloc1", 1);
  L_set_int_attr_field (attr, 0, type_flag);
  oper->attr = L_concat_attr (oper->attr, attr);

  /* Add the spill code offset attribute */
  attr = L_new_attr ("offset", 1);
  L_set_int_attr_field (attr, 0, offset);
  oper->attr = L_concat_attr (oper->attr, attr);

  return (oper);
}

static L_Oper *
new_blk_store_operation (int offset, int type_flag)
{
  L_Oper *oper;
  L_Attr *attr;

  oper = L_create_new_op (Lop_PRED_ST_BLK);
  oper->src[0] = L_new_gen_int_operand (offset);
  oper->src[1] = L_new_macro_operand (L_MAC_SP, L_CTYPE_INT, L_PTYPE_NULL);
  oper->src[2] = L_new_macro_operand (L_MAC_PRED_ALL, L_CTYPE_PREDICATE,
                                      L_PTYPE_NULL);

  oper->flags =
    L_SET_BIT_FLAG (oper->flags, L_OPER_SPILL_CODE | L_OPER_SAFE_PEI);

  /* Add attribute making as inserted by register allocator */
  attr = L_new_attr ("regalloc1", 1);
  L_set_int_attr_field (attr, 0, type_flag);
  oper->attr = L_concat_attr (oper->attr, attr);

  /* Add the spill code offset attribute */
  attr = L_new_attr ("offset", 1);
  L_set_int_attr_field (attr, 0, offset);
  oper->attr = L_concat_attr (oper->attr, attr);

  return (oper);
}

#if 0
static void
free_register_maps (void)
{
  free (caller_int_reg_map);
  free (callee_int_reg_map);
  free (caller_flt_reg_map);
  free (caller_dbl_reg_map);
  free (callee_flt_reg_map);
  free (callee_dbl_reg_map);
  free (caller_prd_reg_map);
  free (callee_prd_reg_map);
}
#endif

void
O_register_init (void)
{
  int i, base, num_prd_caller_reg, num_prd_callee_reg, num_int_caller_reg,
    num_int_callee_reg, num_flt_caller_reg,
    num_dbl_caller_reg, num_dbl_callee_reg;
  static int init = 0;
  if (init)
    return;
  init = 1;

  /*
   *  Define register banks.
   */
  if ( (Limpact_num_int_caller_reg + Limpact_num_int_callee_reg) & 0x1)
    L_punt ("init_register: Limpact requires even number of integer regs");
  if ( (Limpact_num_flt_caller_reg + Limpact_num_flt_callee_reg) & 0x1)
    L_punt ("init_register: Limpact requires even number of float regs");
  if ( (Limpact_num_dbl_caller_reg + Limpact_num_dbl_callee_reg) & 0x1)
    L_punt ("init_register: Limpact requires even number of double regs");
  if (((Limpact_num_dbl_caller_reg + Limpact_num_dbl_callee_reg)* 2) != 
      (Limpact_num_flt_caller_reg + Limpact_num_flt_callee_reg))
    L_punt ("init_register: Limpact requires float = 2x double regs ");

  num_int_caller_reg = Limpact_num_int_caller_reg;
  num_int_callee_reg = Limpact_num_int_callee_reg;

  num_flt_caller_reg = Limpact_num_flt_caller_reg;
  num_flt_callee_reg = Limpact_num_flt_callee_reg;

  num_dbl_caller_reg = Limpact_num_dbl_caller_reg;
  num_dbl_callee_reg = Limpact_num_dbl_callee_reg;

  num_prd_caller_reg = Limpact_num_prd_caller_reg;
  num_prd_callee_reg = Limpact_num_prd_callee_reg;

  base = 0;

  caller_int_reg_map = MALLOC (int, num_int_caller_reg);
  for (i = 0; i < num_int_caller_reg; i++)
    caller_int_reg_map[i] = i + 1;

  R_define_physical_bank_with_rot (R_CALLER,            /* bank saving conv */
                                   R_INT,               /* bank data type */
                                   num_int_caller_reg,  /* num registers */
                                   1,                   /* register size */
                                   R_OVERLAP_INT,       /* overlaping banks */
                                   caller_int_reg_map,  /* register map ptr */
                                   &caller_int_set,     /* set of caller int
                                                           registers used */
                                   0,   /* num_int_caller_reg */
				        /* max num rotating regs */
                                   0,   /* starting ptr into reg map for rot
                                           regs */
                                   0);  /* multiple reg allocation chunk */

  base += num_int_caller_reg;

  callee_int_reg_map = MALLOC (int, num_int_callee_reg);
  for (i = 0; i < num_int_callee_reg; i++)
    callee_int_reg_map[i] = i + base + 1;

  R_define_physical_bank_with_rot (R_CALLEE,
                                   R_INT,
                                   num_int_callee_reg,
                                   1,
                                   R_OVERLAP_INT,
                                   callee_int_reg_map,
                                   &callee_int_set, num_int_callee_reg, 0, 8);

  base += num_int_callee_reg;

  caller_flt_reg_map = MALLOC (int, num_flt_caller_reg);
  caller_dbl_reg_map = MALLOC (int, num_flt_caller_reg);
  for (i = 0; i < num_flt_caller_reg; i++)
    {
      caller_flt_reg_map[i] = i + base + 1;
    }
  for (i = 0; i < num_flt_caller_reg; i++)
    {
      caller_dbl_reg_map[i] = i + base + 1 + num_flt_caller_reg;
    }

  R_define_physical_bank_with_rot (R_CALLER, R_FLOAT, num_flt_caller_reg, 1,
				   R_OVERLAP_FLOAT | R_OVERLAP_DOUBLE,
				   caller_flt_reg_map, &caller_float_set,
				   0, /* num_flt_caller_reg */
                                   0, 0);

  R_define_physical_bank_with_rot (R_CALLER, R_DOUBLE, num_dbl_caller_reg, 2,
				   R_OVERLAP_FLOAT | R_OVERLAP_DOUBLE, 
				   caller_dbl_reg_map, &caller_double_set,
				   0,        /* num_dbl_caller_reg */
                                   0, 0);

  base += (num_flt_caller_reg * 2);

  callee_flt_reg_map = MALLOC (int, num_flt_callee_reg);
  callee_dbl_reg_map = MALLOC (int, num_flt_callee_reg);
  for (i = 0; i < num_flt_callee_reg; i++)
    callee_flt_reg_map[i] = i + base + 1;

  for (i = 0; i < num_flt_callee_reg; i++)
    callee_dbl_reg_map[i] = i + base + 1 + num_flt_callee_reg;

  {
    extern int M_Limpact_dbl_offset;
    /* Mechanism for Limpact to specify the offset between the left float
     * register numer and the double register number.  Typically, this
     * offset should be set to num_flt_callee_reg in Limpact. -JCG 6/99
     * See Mspec/ml_impact.c for details.
     */
    M_Limpact_dbl_offset = num_flt_callee_reg;
  }

  R_define_physical_bank_with_rot (R_CALLEE,
                                   R_FLOAT,
                                   num_flt_callee_reg,
                                   1,
                                   R_OVERLAP_FLOAT | R_OVERLAP_DOUBLE,
                                   callee_flt_reg_map,
                                   &callee_float_set,
                                   num_flt_callee_reg, 0, 8);

  R_define_physical_bank_with_rot (R_CALLEE,
                                   R_DOUBLE,
                                   num_dbl_callee_reg,
                                   2,
                                   R_OVERLAP_FLOAT | R_OVERLAP_DOUBLE,
                                   callee_dbl_reg_map,
                                   &callee_double_set,
                                   num_dbl_callee_reg, 0, 4);

  base += (num_flt_callee_reg * 2);

  caller_prd_reg_map = MALLOC (int, num_prd_caller_reg);
  for (i = 0; i < num_prd_caller_reg; i++)
    caller_prd_reg_map[i] = i + base + 1;
  
  R_define_physical_bank_with_rot (R_CALLER, 
				   R_PREDICATE, 
				   num_prd_caller_reg, 
				   1, 
				   R_OVERLAP_PREDICATE, 
				   caller_prd_reg_map, 
				   &caller_predicate_set, 
				   0,     /* num_prd_caller_reg */
                                   0, 0);
  
  base += num_prd_caller_reg;

  callee_prd_reg_map = MALLOC (int, num_prd_callee_reg);
  for (i = 0; i < num_prd_callee_reg; i++)
    callee_prd_reg_map[i] = i + base + 1;

  R_define_physical_bank_with_rot (R_CALLEE,
                                   R_PREDICATE,
                                   num_prd_callee_reg,
                                   1,
                                   R_OVERLAP_PREDICATE,
                                   callee_prd_reg_map,
                                   &callee_predicate_set,
                                   num_prd_callee_reg, 0, 8);

  base += num_prd_callee_reg;

  /*
   *  free register maps, register allocator has made a copy of them.
   */
  /* REH 4/18/95 Don't free them, cause they */
  /* are needed for O_register_allocation    */
#if 0
  free_register_maps ();
#endif

}

/* SAM 5-95 not needed anymore! */
#if 0
static int
machine_register_type (int reg)
{
  if (reg <= 0)
    L_punt ("machine_register_type: illegal reg number");

  if ((reg >= 1) && (reg <= Limpact_num_int_reg))
    return (L_CTYPE_INT);

  if ((reg >= Limpact_num_int_reg + 1) &&
      (reg <= Limpact_num_int_reg + Limpact_num_flt_reg * 2))
    return (L_CTYPE_DOUBLE);

  if ((reg >= Limpact_num_int_reg + Limpact_num_flt_reg * 2 + 1) &&
      (reg <= Limpact_num_int_reg + Limpact_num_flt_reg * 2 +
       Limpact_num_prd_caller_reg + Limpact_num_prd_callee_reg))
    return (L_CTYPE_PREDICATE);

  L_punt ("machine_register_type: illegal reg number");
  return (0);
}
#endif

void
O_register_allocation (L_Func * fn, Parm_Macro_List * command_line_macro_list,
		       int *size_of_swap)
{
  L_Oper *oper;
  L_Cb *cb;
  int i, spill_space, size_int, size_flt, double_index;
  Set fcle, icle;
  int *callee_reg_array, n_callee;

  L_check_func_hyperblock_flag (fn);

  /* Reset the register usage sets */
  caller_int_set = Set_dispose (caller_int_set);
  callee_int_set = Set_dispose (callee_int_set);
  caller_float_set = Set_dispose (caller_float_set);
  callee_float_set = Set_dispose (callee_float_set);
  caller_double_set = Set_dispose (caller_double_set);
  callee_double_set = Set_dispose (callee_double_set);
  caller_predicate_set = Set_dispose (caller_predicate_set);
  callee_predicate_set = Set_dispose (callee_predicate_set);

  spill_space = R_register_allocation (fn, command_line_macro_list);

  /*  IN ORDER TO BE ABLE TO PERFORM REGISTER ALLOCATION TWICE  */
  /*  THE ONLY ADDITIONAL WORK WE ARE ALLOWED TO DO HERE IS THE */
  /*  INSERTION OF THE "CALLEE" SAVED REGISTERS.                */
  /*  THUS THE FINAL SWAP SPACE WILL BE THE VALUE RETURNED BY   */
  /*  REGISTER ALLOCATION PLUS THE SPACE REQUIRED FOR CALLEE SVS */

  fcle = icle = 0;

  /* Place Callee-saved integer registers used into the icle set */
  callee_reg_array = MALLOC (int, Set_size (callee_int_set) +
                             Set_size (callee_float_set) +
                             Set_size (callee_double_set));
  n_callee = Set_2array (callee_int_set, callee_reg_array);
  for (i = 0; i < n_callee; i++)
    icle = Set_add (icle, callee_reg_array[i]);

  /* Place the double register corresponding to Callee-saved */
  /* float register used in the fcle set                     */
  n_callee = Set_2array (callee_float_set, callee_reg_array);
  for (i = 0; i < n_callee; i++)
    {
      int j, float_reg;

      if (callee_reg_array[i] & 0x1)
	float_reg = callee_reg_array[i];
      else
	float_reg = callee_reg_array[i] - 1;

      /* Find the corresponding double name in the callee_dbl_reg_map */
      for (j = 0; j < num_flt_callee_reg; j++)
        if (callee_flt_reg_map[j] == float_reg)
          break;
      if (j == num_flt_callee_reg)
        L_punt ("O_register_allocation: unable to find float reg %d\n",
                float_reg);

      /* If j is even, it is the double index, if it is odd, we
       * need to subtract one to make it even (every other register
       * is used in the double map, and it maps to the float register
       * at the same index and index + 1). -JCG 6/99
       */
      if ((j & 0x1) == 0)
	double_index = j;
      else
	double_index = j - 1;

      fcle = Set_add (fcle, callee_dbl_reg_map[double_index]);
    }

  /* Place the Callee-saved double registers used into the fcle set */
  n_callee = Set_2array (callee_double_set, callee_reg_array);
  for (i = 0; i < n_callee; i++)
    fcle = Set_add (fcle, callee_reg_array[i]);

  size_int = Set_size (icle);
  size_flt = Set_size (fcle);
  spill_space += size_int * 4 + size_flt * 8;
  Set_2array (fcle, callee_reg_array);
  Set_2array (icle, callee_reg_array + size_flt);

  for (cb = fn->first_cb; cb != NULL; cb = cb->next_cb)
    {
      L_Oper *next_oper, *new_oper;
      for (oper = cb->first_op; oper != NULL; oper = next_oper)
        {
          int opc, k, swap_offset;
          next_oper = oper->next_op;    /* this is important */
          opc = oper->opc;
          /*
           *  Additional save/restore.
           */
          switch (opc)
            {
            case Lop_PROLOGUE:
              /*
               *  Insert save code after it.
               */
              swap_offset = spill_space;
              swap_offset += (8 - (swap_offset % 8));

              /* REH 1/18/95 - Sparc is has register windows, so we don't */
              /* need to insert callee-saved code                         */
              if ((M_impact_model != M_IM_SPARC_LCODE &&
                   M_impact_model != M_IM_SPARC_MCODE))
                {
                  for (k = 0; k < size_flt; k++)
                    {
                      new_oper = new_store_operation (callee_reg_array[k],
                                                      L_CTYPE_DOUBLE,
                                                      swap_offset,
                                                      R_CALLEE_SAVE_CODE);
                      L_insert_oper_after (cb, oper, new_oper);
                      swap_offset += 8;
                    }
                  for (k = size_flt; k < (size_flt + size_int); k++)
                    {
                      new_oper = new_store_operation (callee_reg_array[k],
                                                      L_CTYPE_INT,
                                                      swap_offset,
                                                      R_CALLEE_SAVE_CODE);
                      L_insert_oper_after (cb, oper, new_oper);
                      swap_offset += 4;
                    }
                }

              /* insert a PRED_ST_BLK if there are any callee save pred regs */
              if (L_EXTRACT_BIT_VAL (fn->flags, L_FUNC_HYPERBLOCK))
                {
                  new_oper = new_blk_store_operation (swap_offset,
                                                      R_CALLEE_SAVE_CODE);
                  L_insert_oper_after (cb, oper, new_oper);
                  swap_offset +=
                    ((Limpact_num_prd_callee_reg
                      + Limpact_num_prd_caller_reg + 31) / 32) * 4;
                }

              new_oper = L_create_new_op (Lop_DEFINE);
              new_oper->dest[0] =
                L_new_macro_operand (L_MAC_SWAP_SIZE, L_CTYPE_INT,
                                     L_PTYPE_NULL);
              new_oper->src[0] = L_new_gen_int_operand (swap_offset);
              L_insert_oper_before (cb, oper, new_oper);

	      /* Return the total size of the swap space on the stack */
	      *size_of_swap = swap_offset;

              break;

            case Lop_EPILOGUE:
              /*
               *  Insert restore code before it.
               */
              swap_offset = spill_space;
              swap_offset += (8 - (swap_offset % 8));

              /* REH 1/18/95 - Sparc is has register windows, so we don't */
              /* need to insert callee-saved code                         */
              if ((M_impact_model != M_IM_SPARC_LCODE &&
                   M_impact_model != M_IM_SPARC_MCODE))
                {
                  for (k = 0; k < size_flt; k++)
                    {
                      new_oper = new_load_operation (callee_reg_array[k],
                                                     L_CTYPE_DOUBLE,
                                                     swap_offset,
                                                     R_CALLEE_SAVE_CODE);
                      L_insert_oper_before (cb, oper, new_oper);
                      swap_offset += 8;
                    }
                  for (k = size_flt; k < (size_flt + size_int); k++)
                    {
                      new_oper = new_load_operation (callee_reg_array[k],
                                                     L_CTYPE_INT, swap_offset,
                                                     R_CALLEE_SAVE_CODE);
                      L_insert_oper_before (cb, oper, new_oper);
                      swap_offset += 4;
                    }
                }

              /* insert a PRED_ST_BLK if there are any callee save pred regs */
              if ((L_EXTRACT_BIT_VAL (fn->flags, L_FUNC_HYPERBLOCK)))
                {
                  new_oper = new_blk_load_operation (swap_offset,
                                                     R_CALLEE_SAVE_CODE);
                  L_insert_oper_before (cb, oper, new_oper);
                  swap_offset +=
                    ((Limpact_num_prd_callee_reg
                      + Limpact_num_prd_caller_reg + 31) / 32) * 4;
                }

              break;
            default:
              break;
            }
        }
    }

  /*
   *  Free up resource.
   */
  free (callee_reg_array);

  caller_int_set = Set_dispose (caller_int_set);
  callee_int_set = Set_dispose (callee_int_set);
  caller_float_set = Set_dispose (caller_float_set);
  callee_float_set = Set_dispose (callee_float_set);
  caller_double_set = Set_dispose (caller_double_set);
  callee_double_set = Set_dispose (callee_double_set);
  caller_predicate_set = Set_dispose (caller_predicate_set);
  callee_predicate_set = Set_dispose (callee_predicate_set);
}


/****************************************************************************
 *
 * routine:
 * purpose: 
 * input:
 * output:
 * returns:
 * modified:
 * note:
 *-------------------------------------------------------------------------*/

R_Physical_Bank *
O_locate_rot_reg_bank (L_Func * fn, R_Reg * vreg)
{
  R_Physical_Bank *bank;

  int nth_rot_reg = vreg->nth_rot_reg;

  if (nth_rot_reg < 0)
    L_punt ("O_locate_rot_reg_bank: vreg->nth_rot_reg < 0!");

  switch (vreg->type)
    {
    case R_PREDICATE:
      bank = (R_bank + R_CALLEE + R_PREDICATE);
      vreg->rclass = R_CALLEE;
      vreg->type = R_PREDICATE;

      break;

    case R_INT:
      bank = (R_bank + R_CALLEE + R_INT);
      vreg->rclass = R_CALLEE;
      vreg->type = R_INT;

      break;

    case R_FLOAT:
      bank = (R_bank + R_CALLEE + R_FLOAT);
      vreg->rclass = R_CALLEE;
      vreg->type = R_FLOAT;

      break;

    case R_DOUBLE:
      bank = (R_bank + R_CALLEE + R_DOUBLE);
      vreg->rclass = R_CALLEE;
      vreg->type = R_DOUBLE;

      break;

    default:
      bank = NULL;
    }

  vreg->nth_rot_reg = nth_rot_reg;

  return bank;
}
