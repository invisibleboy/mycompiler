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
 *  File:  limpact_phase2_opti.c
 *
 *  Description:
 *    Phase 2 peep hole opti for IMPACT architecture.
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

#undef DEBUG_BLOCK_ST
#undef DEBUG_BLOCK_LD

static void
L_insert_pred_block_store (L_Cb * cb, L_Oper * jsr)
{
  int count;
  L_Operand *base, *offset, *pred;
  L_Oper *ptr, *prev, *new_op;
  L_Attr *attr, *orig_attr;

#ifdef DEBUG_BLOCK_ST
  fprintf (stderr, "Enter L_insert_pred_block_store op %d cb %d\n",
           jsr->id, cb->id);
#endif

  if (jsr == NULL)
    L_punt ("L_insert_pred_block_store: jsr is NULL");

  /* no stores */
  if (jsr->prev_op == NULL)
    return;

  count = 0;
  base = NULL;
  offset = NULL;
  pred = NULL;
  orig_attr = NULL;

  for (ptr = jsr->prev_op; ptr != NULL; ptr = prev)
    {
      prev = ptr->prev_op;
      if (!L_general_store_opcode (ptr))
        break;
      if (!(attr = L_find_attr (ptr->attr, "regalloc1")))
        break;
      if (attr->field[0]->value.i != R_JSR_SAVE_CODE)
        break;
      if ((L_is_register (ptr->src[2])) &&
          (L_is_ctype_predicate (ptr->src[2])))
        {
          /* save the attribute of one of the sts for the block store */
          if (orig_attr == NULL)
            {
              orig_attr = L_copy_attr (ptr->attr);
            }

          /* save, check base address of all PRED_ST's */
          if (base == NULL)
            {
              base = L_copy_operand (ptr->src[0]);
            }
          else
            {
              if (!L_same_operand (base, ptr->src[0]))
                L_punt ("L_insert_pred_block_store: base addr doesnt match");
            }

          /* save, check offset address of all PRED_ST's */
          if (offset == NULL)
            {
              offset = L_copy_operand (ptr->src[1]);
            }
          else
            {
              if (!L_same_operand (offset, ptr->src[1]))
                L_punt
                  ("L_insert_pred_block_store: offset addr doesnt match");
            }

          /* save, check pred of all PRED_ST's */
          if (pred == NULL)
            {
              pred = L_copy_operand (ptr->pred[0]);
            }
          else
            {
              if (!L_same_operand (pred, ptr->pred[0]))
                L_punt ("L_insert_pred_block_store: pred doesnt match");
            }

          count++;
          L_delete_oper (cb, ptr);
        }
    }

  /* no stores */
  if (count == 0)
    return;

  /* insert block store */
#ifdef DEBUG_BLOCK_ST
  fprintf (stderr, "Insert a block store in cb %d\n", cb->id);
#endif
  new_op = L_create_new_op (Lop_PRED_ST_BLK);
  new_op->src[0] = base;
  new_op->src[1] = offset;
  new_op->src[2] = L_new_macro_operand (L_MAC_PRED_ALL, L_CTYPE_PREDICATE,
                                        L_PTYPE_NULL);
  new_op->pred[0] = pred;
  new_op->attr = orig_attr;
  L_insert_oper_before (cb, jsr, new_op);
}

static void
L_insert_pred_block_load (L_Cb * cb, L_Oper * jsr)
{
  int count;
  L_Operand *base, *offset, *pred;
  L_Oper *ptr, *next, *new_op;
  L_Attr *attr, *orig_attr;

#ifdef DEBUG_BLOCK_LD
  fprintf (stderr, "Enter L_insert_pred_block_load op %d cb %d\n",
           jsr->id, cb->id);
#endif

  if (jsr == NULL)
    L_punt ("L_insert_pred_block_load: jsr is NULL");

  /* no loads */
  if (jsr->next_op == NULL)
    return;

  count = 0;
  base = NULL;
  offset = NULL;
  pred = NULL;
  orig_attr = NULL;

  for (ptr = jsr->next_op; ptr != NULL; ptr = next)
    {
      next = ptr->next_op;
      if (!L_general_load_opcode (ptr))
        break;
      attr = L_find_attr (ptr->attr, "regalloc1");
      if (!attr)
        break;
      if (attr->field[0]->value.i != R_JSR_SAVE_CODE)
        break;
      if ((L_is_register (ptr->dest[0])) &&
          (L_is_ctype_predicate (ptr->dest[0])))
        {
          /* save the attribute of one of the lds for the block load */
          if (orig_attr == NULL)
            {
              orig_attr = L_copy_attr (ptr->attr);
            }

          /* save, check base address of all PRED_LD's */
          if (base == NULL)
            {
              base = L_copy_operand (ptr->src[0]);
            }
          else
            {
              if (!L_same_operand (base, ptr->src[0]))
                L_punt ("L_insert_pred_block_load: base addr doesnt match");
            }

          /* save, check offset address of all PRED_LD's */
          if (offset == NULL)
            {
              offset = L_copy_operand (ptr->src[1]);
            }
          else
            {
              if (!L_same_operand (offset, ptr->src[1]))
                L_punt ("L_insert_pred_block_load: offset addr doesnt match");
            }

          /* save, check pred of all PRED_LD's */
          if (pred == NULL)
            {
              pred = L_copy_operand (ptr->pred[0]);
            }
          else
            {
              if (!L_same_operand (pred, ptr->pred[0]))
                L_punt ("L_insert_pred_block_store: pred doesnt match");
            }

          count++;
          L_delete_oper (cb, ptr);
        }
    }

  /* no loads */
  if (count == 0)
    return;

  /* insert block load */
#ifdef DEBUG_BLOCK_LD
  fprintf (stderr, "Insert a block load in cb %d\n", cb->id);
#endif
  new_op = L_create_new_op (Lop_PRED_LD_BLK);
  new_op->dest[0] = L_new_macro_operand (L_MAC_PRED_ALL, L_CTYPE_PREDICATE,
                                         L_PTYPE_NULL);
  new_op->src[0] = base;
  new_op->src[1] = offset;
  new_op->pred[0] = pred;
  new_op->attr = orig_attr;
  L_insert_oper_after (cb, jsr, new_op);
}

void
O_insert_pred_block_ops (L_Func * fn)
{
  L_Cb *cb;
  L_Oper *oper, *next;

  if (Limpact_num_prd_caller_reg <= 0)
    return;

  for (cb = fn->first_cb; cb != NULL; cb = cb->next_cb)
    {
      for (oper = cb->first_op; oper != NULL; oper = next)
        {
          next = oper->next_op;
          if (!L_subroutine_call_opcode (oper))
            continue;
          L_insert_pred_block_store (cb, oper);
          L_insert_pred_block_load (cb, oper);
        }
    }
}
