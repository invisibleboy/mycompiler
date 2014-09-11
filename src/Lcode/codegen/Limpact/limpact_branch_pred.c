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
 *  File:  limpact_branch_pred.c
 *
 *  Description:
 *    Mark static branch prediction in attributes
 *
 *  Creation Date : June 1994
 *
 *  Author:  Scott A. Mahlke, Wen-mei Hwu
 *
\************************************************************************/
/* 10/29/02 REK Adding config.h */
#include <config.h>
#include <Lcode/limpact_main.h>

#define BP_ATTR_NAME    "bpred"

#define FALLTHRU        0
#define TAKEN           1


static void
L_branch_pred_by_profile (L_Func * fn)
{
  L_Cb *cb;
  L_Oper *oper;
  L_Flow *flow;
  L_Attr *attr;
  double weight, taken_weight;

  for (cb = fn->first_cb; cb != NULL; cb = cb->next_cb)
    {
      weight = cb->weight;
      flow = cb->dest_flow;
      for (oper = cb->first_op; oper != NULL; oper = oper->next_op)
        {
          if (!(L_general_branch_opcode (oper) ||
                L_subroutine_call_opcode (oper) ||
                L_subroutine_return_opcode (oper)))
            continue;

          /* jsr, rts always taken */
          if (L_subroutine_call_opcode (oper) ||
              L_subroutine_return_opcode (oper))
            {
              attr = L_new_attr (BP_ATTR_NAME, 1);
              L_set_int_attr_field (attr, 0, TAKEN);
            }

          /* cond br or predicated jump */
          else if (L_cond_branch_opcode (oper) ||
                   (L_uncond_branch_opcode (oper) && L_is_predicated (oper)))
            {
              taken_weight = flow->weight;
              weight -= taken_weight;
              flow = flow->next_flow;
              attr = L_new_attr (BP_ATTR_NAME, 1);
              if (taken_weight > weight)
                L_set_int_attr_field (attr, 0, TAKEN);
              else
                L_set_int_attr_field (attr, 0, FALLTHRU);
            }

          /* unpredicated jump or jump_rg always taken */
          else
            {
              /* error check here */
              if (oper != cb->last_op)
                L_punt ("L_branch_pred_profile: misplaced jmp or jrg");
              attr = L_new_attr (BP_ATTR_NAME, 1);
              L_set_int_attr_field (attr, 0, TAKEN);
            }

          /* add the attribute to the oper */
          oper->attr = L_concat_attr (oper->attr, attr);
        }
    }
}


static void
L_branch_pred_by_direction (L_Func * fn)
{
  int position, cur_pos, target_pos, *cb_layout;
  L_Cb *cb;
  L_Oper *oper;
  L_Attr *attr;

  cb_layout = (int *) Lcode_malloc (sizeof (int) * (fn->max_cb_id + 1));

  /* record layout of cbs in funct */
  position = 1;
  for (cb = fn->first_cb; cb != NULL; cb = cb->next_cb)
    {
      cb_layout[cb->id] = position;
      position++;
    }

  /* do the prediction */
  for (cb = fn->first_cb; cb != NULL; cb = cb->next_cb)
    {
      for (oper = cb->first_op; oper != NULL; oper = oper->next_op)
        {
          if (!(L_general_branch_opcode (oper) ||
                L_subroutine_call_opcode (oper) ||
                L_subroutine_return_opcode (oper)))
            continue;

          if (L_cond_branch_opcode (oper))
            {
              cur_pos = cb_layout[cb->id];
              target_pos = cb_layout[oper->src[2]->value.cb->id];
              if (target_pos <= cur_pos)
                {               /* backward taken */
                  attr = L_new_attr (BP_ATTR_NAME, 1);
                  L_set_int_attr_field (attr, 0, TAKEN);
                }
              else
                {               /* forward nottaken */
                  attr = L_new_attr (BP_ATTR_NAME, 1);
                  L_set_int_attr_field (attr, 0, FALLTHRU);
                }
            }

          /* rest are always taken: jsr, rts, jump, jump_rg */
          else
            {
              attr = L_new_attr (BP_ATTR_NAME, 1);
              L_set_int_attr_field (attr, 0, TAKEN);
            }

          /* add the attribute to the oper */
          oper->attr = L_concat_attr (oper->attr, attr);
        }
    }

  Lcode_free (cb_layout);
}


void
L_mark_static_branch_pred (L_Func * fn, char *bp_model_name)
{
  if (!strcasecmp (bp_model_name, "profile"))
    {
      L_branch_pred_by_profile (fn);
    }
  else if (!strcasecmp (bp_model_name, "heuristic"))
    {
      L_punt ("L_mark_static_branch_pred: model %s unavailable now",
              bp_model_name);
    }
  else if (!strcasecmp (bp_model_name, "direction"))
    {
      L_branch_pred_by_direction (fn);
    }
  else
    {
      L_punt ("L_mark_static_branch_pred: illegal model %s specified",
              bp_model_name);
    }
}
