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
/****************************************************************************\
 *      File:   r_df_query.c
 *      Author: David August, Wen-mei Hwu
 *      Based loosely on the original by Rick Hank
 *      Creation Date:  November 1996
\****************************************************************************/

/* 10/29/02 REK Adding config.h */
#include <config.h>
#include <Lcode/l_main.h>

#if 0
/*
 * DOM/PDOM QUERIES
 * ----------------------------------------------------------------------
 */

int
D_in_cb_DOM_set (PRED_FLOW *pred_flow, L_Cb *cb, int cb_id)
{
  PF_CB *pf_cb;

  if (!pred_flow || !cb)
    L_punt ("D_in_cb_DOM_set: received NULL!\n");

  pf_cb = (PF_CB *) HashTable_find_or_null (pred_flow->hash_cb_pfcb, cb->id);
  if (!pf_cb)
    L_punt ("D_in_cb_DOM_set: pf_cb %d not in hash\n", cb->id);

  return (Set_in (pf_cb->info->dom, cb_id));
}
#endif

/*
 * LIVE VARIABLE QUERIES
 * ----------------------------------------------------------------------
 */


int
D_in_cb_IN_set (PRED_FLOW * pred_flow, L_Cb * cb, int operand_id)
{
  PF_CB *pf_cb;

  if (!pred_flow || !cb)
    L_punt ("D_in_cb_IN_set: received NULL!\n");

  pf_cb = (PF_CB *) HashTable_find_or_null (pred_flow->hash_cb_pfcb, cb->id);
  if (!pf_cb)
    L_punt ("D_in_cb_IN_set: pf_cb %d not in hash\n", cb->id);

  return (Set_in (pf_cb->info->v_in, operand_id));
}


int
D_in_cb_OUT_set (PRED_FLOW * pred_flow, L_Cb * cb, int operand_id)
{
  PF_CB *pf_cb;

  if (!pred_flow || !cb)
    L_punt ("D_in_cb_OUT_set: received NULL!\n");

  pf_cb = (PF_CB *) HashTable_find_or_null (pred_flow->hash_cb_pfcb, cb->id);
  if (!pf_cb)
    L_punt ("D_in_cb_OUT_set: pf_cb %d not in hash\n", cb->id);

  return (Set_in (pf_cb->info->v_out, operand_id));
}


Set
D_get_cb_IN_set (PRED_FLOW * pred_flow, L_Cb * cb)
{
  PF_CB *pf_cb;

  if (!pred_flow || !cb)
    L_punt ("D_get_cb_IN_set: received NULL!\n");

  pf_cb = (PF_CB *) HashTable_find_or_null (pred_flow->hash_cb_pfcb, cb->id);
  if (!pf_cb)
    L_punt ("D_get_cb_IN_set: pf_cb %d not in hash\n", cb->id);

  return (pf_cb->info->v_in);
}


Set
D_get_cb_OUT_set (PRED_FLOW * pred_flow, L_Cb * cb)
{
  PF_CB *pf_cb;

  if (!pred_flow || !cb)
    L_punt ("D_get_cb_OUT_set: received NULL!\n");

  pf_cb = (PF_CB *) HashTable_find_or_null (pred_flow->hash_cb_pfcb, cb->id);
  if (!pf_cb)
    L_punt ("D_get_cb_OUT_set: pf_cb %d not in hash\n", cb->id);

  return (pf_cb->info->v_out);
}


void
D_add_to_cb_IN_set (PRED_FLOW * pred_flow, L_Cb * cb, int operand_id)
{
  PF_CB *pf_cb;

  if (!pred_flow || !cb)
    L_punt ("D_in_cb_IN_set: received NULL!\n");

  pf_cb = (PF_CB *) HashTable_find_or_null (pred_flow->hash_cb_pfcb, cb->id);
  if (!pf_cb)
    L_punt ("D_in_cb_IN_set: pf_cb %d not in hash\n", cb->id);

  pf_cb->info->v_in = Set_add (pf_cb->info->v_in, operand_id);

  return;
}


int
D_in_oper_IN_set (PRED_FLOW * pred_flow, L_Oper * oper, int operand_id)
{
  PF_OPER *pf_oper;

  if (!pred_flow || !oper)
    L_punt ("D_in_oper_IN_set: received NULL!\n");

  pf_oper =
    (PF_OPER *) HashTable_find_or_null (pred_flow->hash_oper_pfoper,
                                        oper->id);
  if (!pf_oper)
    L_punt ("D_in_oper_IN_set: pf_oper %d not in hash\n", oper->id);

  return (Set_in (pf_oper->info->v_in, operand_id));
}


int
D_in_oper_OUT_set (PRED_FLOW * pred_flow, L_Cb * cb, L_Oper * oper,
                   int operand_id, int path)
{
  PF_OPER *pf_oper;
  PF_CB *pf_cb;

  if (!pred_flow || !oper)
    L_punt ("D_in_oper_OUT_set: received NULL!\n");

  if (L_cond_branch (oper) || L_check_branch_opcode (oper))
    {
      switch (path)
        {
        case BOTH_PATHS:
          {
            pf_oper =
              (PF_OPER *) HashTable_find_or_null (pred_flow->hash_oper_pfoper,
                                                  oper->id);
            if (!pf_oper)
              L_punt ("D_in_oper_OUT_set: pf_oper %d not in hash\n",
                      oper->id);
            return (Set_in (pf_oper->info->v_out, operand_id));
          }
        case TAKEN_PATH:
          {
	    L_Cb *target = L_find_branch_dest(oper);

            pf_cb =
              (PF_CB *) HashTable_find_or_null (pred_flow->hash_cb_pfcb,
						target->id);
            if (!pf_cb)
              L_punt ("D_in_oper_OUT_set: pf_cb %d not in hash\n",
                      (L_find_branch_dest (oper))->id);
            return (Set_in (pf_cb->info->v_in, operand_id));
          }
        case FALL_THRU_PATH:
          {
            if (oper->next_op != NULL)
              {
                /* the IN set of the next instruction contains the fall thru */
                /* out set of the conditional branch instruction             */
                pf_oper =
                  (PF_OPER *)
                  HashTable_find_or_null (pred_flow->hash_oper_pfoper,
                                          oper->next_op->id);
                if (!pf_oper)
                  L_punt ("D_in_oper_OUT_set: pf_oper %d not in hash\n",
                          oper->next_op->id);
                return (Set_in (pf_oper->info->v_in, operand_id));
              }
            else
              {
                /* the IN set of the fall thru cb contains the the fall thru */
                /* out set of the conditional branch instruction             */
                L_Cb *n_cb;
                n_cb = cb->next_cb;
                while (n_cb->first_op == NULL)
                  n_cb = n_cb->next_cb;
                pf_cb =
                  (PF_CB *) HashTable_find_or_null (pred_flow->hash_cb_pfcb,
                                                    n_cb->id);
                if (!pf_cb)
                  L_punt ("D_in_oper_OUT_set: pf_cb %d not in hash\n",
                          n_cb->id);
                return (Set_in (pf_cb->info->v_in, operand_id));
              }
          }
        default:
          L_punt ("D_in_oper_OUT_set: invalid path type %d\n", path);
        }
    }
  else
    {
      pf_oper =
        (PF_OPER *) HashTable_find_or_null (pred_flow->hash_oper_pfoper,
                                            oper->id);
      if (!pf_oper)
        L_punt ("D_in_oper_OUT_set: pf_oper %d not in hash\n", oper->id);
      return (Set_in (pf_oper->info->v_out, operand_id));
    }

  return (0);
}


Set
D_get_oper_IN_set (PRED_FLOW * pred_flow, L_Oper * oper)
{
  PF_OPER *pf_oper;

  if (!pred_flow || !oper)
    L_punt ("D_get_oper_IN_set: received NULL!\n");

  pf_oper =
    (PF_OPER *) HashTable_find_or_null (pred_flow->hash_oper_pfoper,
                                        oper->id);
  if (!pf_oper)
    L_punt ("D_in_oper_IN_set: pf_oper %d not in hash\n", oper->id);

  return (pf_oper->info->v_in);
}


Set
D_get_oper_OUT_set (PRED_FLOW * pred_flow, L_Cb * cb, L_Oper * oper, int path)
{
  PF_OPER *pf_oper;
  PF_CB *pf_cb;

  if (!pred_flow || !oper)
    L_punt ("D_in_oper_OUT_set: received NULL!\n");

  if (L_cond_branch (oper) || L_check_branch_opcode (oper))
    {
      switch (path)
        {
        case BOTH_PATHS:
          {
            pf_oper =
              (PF_OPER *) HashTable_find_or_null (pred_flow->hash_oper_pfoper,
                                                  oper->id);
            if (!pf_oper)
              L_punt ("D_get_oper_OUT_set: pf_oper %d not in hash\n",
                      oper->id);
            return (pf_oper->info->v_out);
          }
        case TAKEN_PATH:
          {
            pf_cb =
              (PF_CB *) HashTable_find_or_null (pred_flow->hash_cb_pfcb,
                                                (L_find_branch_dest
                                                 (oper))->id);
            if (!pf_cb)
              L_punt ("D_get_oper_OUT_set: pf_cb %d not in hash\n",
                      (L_find_branch_dest (oper))->id);
            return (pf_cb->info->v_in);
          }
        case FALL_THRU_PATH:
          {
            if (oper->next_op != NULL)
              {
                /* the IN set of the next instruction contains the fall thru */
                /* out set of the conditional branch instruction             */
                pf_oper =
                  (PF_OPER *)
                  HashTable_find_or_null (pred_flow->hash_oper_pfoper,
                                          oper->next_op->id);
                if (!pf_oper)
                  L_punt ("D_get_oper_OUT_set: pf_oper %d not in hash\n",
                          oper->next_op->id);
                return (pf_oper->info->v_in);
              }
            else
              {
                /* the IN set of the fall thru cb contains the the fall thru */
                /* out set of the conditional branch instruction             */
                L_Cb *n_cb;
                n_cb = cb->next_cb;
                while (n_cb->first_op == NULL)
                  n_cb = n_cb->next_cb;
                pf_cb =
                  (PF_CB *) HashTable_find_or_null (pred_flow->hash_cb_pfcb,
                                                    n_cb->id);
                if (!pf_cb)
                  L_punt ("D_get_oper_OUT_set: pf_cb %d not in hash\n",
                          n_cb->id);
                return (pf_cb->info->v_in);
              }
          }
        default:
          L_punt ("D_in_oper_OUT_set: invalid path type %d\n", path);
        }
    }
  else
    {
      pf_oper =
        (PF_OPER *) HashTable_find_or_null (pred_flow->hash_oper_pfoper,
                                            oper->id);
      if (!pf_oper)
        L_punt ("D_get_oper_OUT_set: pf_oper %d not in hash\n", oper->id);
      return (pf_oper->info->v_out);
    }

  return (NULL);
}


/*
 * REACHING DEFINITION QUERIES
 * ----------------------------------------------------------------------
 */


int
D_in_cb_RIN_set (PRED_FLOW * pred_flow, L_Cb * cb,
		 L_Oper * reaching_oper, int operand_id)
{
  PF_CB *pf_cb;
  PF_OPER *reaching_pf_oper;
  PF_OPERAND *pf_operand;

  if (!pred_flow || !cb)
    L_punt ("D_in_cb_IN_set: received NULL!\n");

  pf_cb = (PF_CB *) HashTable_find_or_null (pred_flow->hash_cb_pfcb, cb->id);
  if (!pf_cb)
    L_punt ("D_in_cb_IN_set: pf_cb %d not in hash\n", cb->id);

  reaching_pf_oper =
    (PF_OPER *) HashTable_find_or_null (pred_flow->hash_oper_pfoper,
                                        reaching_oper->id);
  if (!reaching_pf_oper)
    L_punt ("D_in_cb_RIN_set: pf_oper %d not in hash\n", reaching_oper->id);

  PF_FOREACH_OPERAND(pf_operand, reaching_pf_oper->dest)
    if (pf_operand->reg == operand_id)
      return (Set_in (pf_cb->info->r_in, pf_operand->id));

  L_punt ("D_in_cb_RIN_set: invalid pair, L_Oper: %d, Operand_id %d\n",
          reaching_oper->id, operand_id);

  return (0);
}


int
D_in_oper_RIN_set (PRED_FLOW * pred_flow, L_Oper * oper,
                   L_Oper * reaching_oper, int operand_id)
{
  PF_OPER *pf_oper;
  PF_OPER *reaching_pf_oper;
  PF_OPERAND *pf_operand;

  if (!pred_flow || !oper)
    L_punt ("D_in_oper_RIN_set: received NULL!\n");

  pf_oper =
    (PF_OPER *) HashTable_find_or_null (pred_flow->hash_oper_pfoper,
                                        oper->id);
  if (!pf_oper)
    L_punt ("D_in_oper_RIN_set: pf_oper %d not in hash\n", oper->id);

  reaching_pf_oper =
    (PF_OPER *) HashTable_find_or_null (pred_flow->hash_oper_pfoper,
                                        reaching_oper->id);
  if (!reaching_pf_oper)
    L_punt ("D_in_oper_RIN_set: pf_oper %d not in hash\n", reaching_oper->id);

  PF_FOREACH_OPERAND(pf_operand, reaching_pf_oper->dest)
    if (pf_operand->reg == operand_id)
      return (Set_in (pf_oper->info->r_in, pf_operand->id));

  L_punt ("D_in_oper_RIN_set: invalid pair, L_Oper: %d, Operand_id %d\n",
          reaching_oper->id, operand_id);
  return (0);
}


int
D_in_oper_ROUT_set (PRED_FLOW * pred_flow, L_Oper * oper,
                    L_Oper * reaching_oper, int operand_id, int path)
{
  PF_OPER *pf_oper;
  PF_OPER *reaching_pf_oper;
  PF_OPERAND *pf_operand;

  if (!pred_flow || !oper)
    L_punt ("D_in_oper_ROUT_set: received NULL!\n");

  pf_oper =
    (PF_OPER *) HashTable_find_or_null (pred_flow->hash_oper_pfoper,
                                        oper->id);
  if (!pf_oper)
    L_punt ("D_in_oper_ROUT_set: pf_oper %d not in hash\n", oper->id);

  reaching_pf_oper =
    (PF_OPER *) HashTable_find_or_null (pred_flow->hash_oper_pfoper,
                                        reaching_oper->id);
  if (!reaching_pf_oper)
    L_punt ("D_in_oper_ROUT_set: pf_oper %d not in hash\n",
            reaching_oper->id);

  PF_FOREACH_OPERAND(pf_operand, reaching_pf_oper->dest)
    if (pf_operand->reg == operand_id)
      return (Set_in (pf_oper->info->r_out, pf_operand->id));

  L_punt ("D_in_oper_ROUT_set: invalid pair, L_Oper: %d, Operand_id %d\n",
          reaching_oper->id, operand_id);
  return (0);
}


Set
D_get_cb_RIN_set (PRED_FLOW * pred_flow, L_Cb * cb)
{
  PF_CB *pf_cb;

  if (!pred_flow || !cb)
    L_punt ("D_get_cb_IN_set: received NULL!\n");

  pf_cb = (PF_CB *) HashTable_find_or_null (pred_flow->hash_cb_pfcb, cb->id);
  if (!pf_cb)
    L_punt ("D_get_cb_IN_set: pf_cb %d not in hash\n", cb->id);

  return (pf_cb->info->r_in);
}


Set
D_get_oper_RIN_set (PRED_FLOW * pred_flow, L_Oper * oper)
{
  PF_OPER *pf_oper;

  if (!pred_flow || !oper)
    L_punt ("D_get_oper_RIN_set: received NULL!\n");

  pf_oper =
    (PF_OPER *) HashTable_find_or_null (pred_flow->hash_oper_pfoper,
                                        oper->id);
  if (!pf_oper)
    L_punt ("D_get_oper_RIN_set: pf_oper %d not in hash\n", oper->id);

  return (pf_oper->info->r_in);
}


Set
D_get_oper_ROUT_set (PRED_FLOW * pred_flow, L_Oper * oper)
{
  PF_OPER *pf_oper;

  if (!pred_flow || !oper)
    L_punt ("D_get_oper_ROUT_set: received NULL!\n");

  pf_oper =
    (PF_OPER *) HashTable_find_or_null (pred_flow->hash_oper_pfoper,
                                        oper->id);
  if (!pf_oper)
    L_punt ("D_get_oper_ROUT_set: pf_oper %d not in hash\n", oper->id);

  return (pf_oper->info->r_out);
}


/* Warning this creates a set, caller must free it */
Set
D_get_RIN_defining_opers (PRED_FLOW * pred_flow, Set RIN, int operand_id)
{
  PF_OPERAND *pf_operand;
  Set all_def_opers, rdef_opers;
  int id, index, num_def_opers, *buf;
  Set intersecting;
  int num_intersecting;

  all_def_opers =
    (Set) HashTable_find_or_null (pred_flow->hash_RD_operand_def, operand_id);
  rdef_opers = NULL;

  num_def_opers = Set_size (all_def_opers);
  if (num_def_opers == 0)
    return (rdef_opers);

  intersecting = Set_intersect (all_def_opers, RIN);

  num_intersecting = Set_size (intersecting);
  if (num_intersecting > 0)
    {
      buf = (int *) alloca (sizeof (int) * num_intersecting);
      Set_2array (intersecting, buf);

      for (index = 0; index < num_intersecting; index++)
        {
          id = buf[index];
          pf_operand =
            (PF_OPERAND *) HashTable_find_or_null (pred_flow->hash_pf_operand,
                                                   id);
          if (!pf_operand)
            L_punt
              ("D_get_oper_RIN_defining_opers: pf_operand %d not in hash\n",
               id);

          if (pf_operand->reg == operand_id)
            rdef_opers = Set_add (rdef_opers, pf_operand->pf_oper->oper->id);
        }
    }

  intersecting = Set_dispose(intersecting);

  return (rdef_opers);
}



Set
D_get_oper_RIN_defining_opers (PRED_FLOW *pred_flow, L_Oper *oper,
                               int operand_id)
{
  Set RIN = D_get_oper_RIN_set (pred_flow, oper);
  return D_get_RIN_defining_opers (pred_flow, RIN, operand_id);
}


Set
D_get_cb_RIN_defining_opers (PRED_FLOW *pred_flow, L_Cb *cb,
			     int operand_id)
{
  Set RIN = D_get_cb_RIN_set (pred_flow, cb);
  return D_get_RIN_defining_opers (pred_flow, RIN, operand_id);
}



/* Warning this creates a set, caller must free it */
Set
D_get_oper_ROUT_using_opers (PRED_FLOW * pred_flow, L_Oper * oper,
                             L_Operand * operand, int operand_id)
{
  PF_OPERAND *pf_operand;
  PF_OPER *pf_oper;
  Set all_use_opers, ruse_opers;
  int id, index, num_use_opers, *buf;
  Set RIN;
  int defining_pf_operand_id;

  all_use_opers =
    (Set) HashTable_find_or_null (pred_flow->hash_RD_operand_use, operand_id);
  ruse_opers = NULL;

  num_use_opers = Set_size (all_use_opers);
  if (num_use_opers == 0)
    return (ruse_opers);

  buf = (int *) alloca (sizeof (int) * num_use_opers);
  Set_2array (all_use_opers, buf);

  pf_oper =
    (PF_OPER *) HashTable_find_or_null (pred_flow->hash_oper_pfoper,
                                        oper->id);
  if (!pf_oper)
    L_punt ("D_get_oper_ROUT_using_opers: pf_oper %d not in hash\n",
            oper->id);

  defining_pf_operand_id = -1;
  PF_FOREACH_OPERAND(pf_operand, pf_oper->dest)
    if (pf_operand->operand == operand)
      {
	defining_pf_operand_id = pf_operand->id;
	break;
      }

  if (defining_pf_operand_id == -1)
    L_punt ("D_get_oper_ROUT_using_opers: "
            "no pf_operand found for destination operand");

  for (index = 0; index < num_use_opers; index++)
    {
      id = buf[index];

      pf_oper =
        (PF_OPER *) HashTable_find_or_null (pred_flow->hash_oper_pfoper, id);
      if (!pf_oper)
        L_punt ("D_get_oper_ROUT_using_opers: pf_oper %d not in hash\n", id);
      RIN = pf_oper->info->r_in;

      if (!Set_in (RIN, defining_pf_operand_id))
        continue;

      ruse_opers = Set_add (ruse_opers, id);
    }

  return (ruse_opers);
}


/*
 * AVAILABLE DEFINITION QUERIES
 * ----------------------------------------------------------------------
 */


int
D_in_oper_AIN_set (PRED_FLOW * pred_flow, L_Oper * oper,
                   L_Oper * reaching_oper, int operand_id)
{
  PF_OPER *pf_oper;
  PF_OPER *reaching_pf_oper;
  PF_OPERAND *pf_operand;

  if (!pred_flow || !oper || !reaching_oper)
    L_punt ("D_in_oper_AIN_set: received NULL!\n");

  pf_oper =
    (PF_OPER *) HashTable_find_or_null (pred_flow->hash_oper_pfoper,
                                        oper->id);
  if (!pf_oper)
    L_punt ("D_in_oper_AIN_set: pf_oper %d not in hash\n", oper->id);

  reaching_pf_oper =
    (PF_OPER *) HashTable_find_or_null (pred_flow->hash_oper_pfoper,
                                        reaching_oper->id);
  if (!reaching_pf_oper)
    L_punt ("D_in_oper_AIN_set: pf_oper %d not in hash\n", reaching_oper->id);

  PF_FOREACH_OPERAND(pf_operand, reaching_pf_oper->dest)
    if (pf_operand->reg == operand_id)
      return (Set_in (pf_oper->info->a_in, pf_operand->id));

  L_punt ("D_in_oper_AIN_set: invalid pair, L_Oper: %d, Operand_id %d\n",
          reaching_oper->id, operand_id);
  return (0);
}


int
D_in_oper_AOUT_set (PRED_FLOW * pred_flow, L_Oper * oper,
                    L_Oper * reaching_oper, int operand_id, int path)
{
  PF_OPER *pf_oper;
  PF_OPER *reaching_pf_oper;
  PF_OPERAND *pf_operand;

  if (!pred_flow || !oper || !reaching_oper)
    L_punt ("D_in_oper_AOUT_set: received NULL!\n");

  pf_oper =
    (PF_OPER *) HashTable_find_or_null (pred_flow->hash_oper_pfoper,
                                        oper->id);
  if (!pf_oper)
    L_punt ("D_in_oper_AOUT_set: pf_oper %d not in hash\n", oper->id);

  reaching_pf_oper =
    (PF_OPER *) HashTable_find_or_null (pred_flow->hash_oper_pfoper,
                                        reaching_oper->id);
  if (!reaching_pf_oper)
    L_punt ("D_in_oper_AOUT_set: pf_oper %d not in hash\n",
            reaching_oper->id);

  PF_FOREACH_OPERAND(pf_operand, reaching_pf_oper->dest)
    if (pf_operand->reg == operand_id)
      return (Set_in (pf_oper->info->a_out, pf_operand->id));

  L_punt ("D_in_oper_AOUT_set: invalid pair, L_Oper: %d, Operand_id %d\n",
          reaching_oper->id, operand_id);
  return (0);
}


Set
D_get_oper_AIN_set (PRED_FLOW * pred_flow, L_Oper * oper)
{
  PF_OPER *pf_oper;

  if (!pred_flow || !oper)
    L_punt ("D_get_oper_AIN_set: received NULL!\n");

  pf_oper =
    (PF_OPER *) HashTable_find_or_null (pred_flow->hash_oper_pfoper,
                                        oper->id);
  if (!pf_oper)
    L_punt ("D_get_oper_AIN_set: pf_oper %d not in hash\n", oper->id);

  return (pf_oper->info->a_in);
}


Set
D_get_oper_AOUT_set (PRED_FLOW * pred_flow, L_Oper * oper)
{
  PF_OPER *pf_oper;

  if (!pred_flow || !oper)
    L_punt ("D_get_oper_AOUT_set: received NULL!\n");

  pf_oper =
    (PF_OPER *) HashTable_find_or_null (pred_flow->hash_oper_pfoper,
                                        oper->id);
  if (!pf_oper)
    L_punt ("D_get_oper_AOUT_set: pf_oper %d not in hash\n", oper->id);

  return (pf_oper->info->a_out);
}


/* Warning this creates a set, caller must free it */
Set
D_get_oper_AIN_defining_opers (PRED_FLOW * pred_flow, L_Oper * oper,
                               int operand_id)
{
  PF_OPERAND *pf_operand;
  Set all_def_opers, rdef_opers, AIN;
  int id, index, num_def_opers, *buf;

  AIN = D_get_oper_AIN_set (pred_flow, oper);
  all_def_opers =
    (Set) HashTable_find_or_null (pred_flow->hash_RD_operand_def, operand_id);
  rdef_opers = NULL;

  num_def_opers = Set_size (all_def_opers);
  if (num_def_opers == 0)
    return (rdef_opers);

  buf = (int *) alloca (sizeof (int) * num_def_opers);
  Set_2array (all_def_opers, buf);

  for (index = 0; index < num_def_opers; index++)
    {
      id = buf[index];
      if (!Set_in (AIN, id))
        continue;
      pf_operand =
        (PF_OPERAND *) HashTable_find_or_null (pred_flow->hash_pf_operand,
                                               id);
      if (!pf_operand)
        L_punt ("D_get_oper_AIN_defining_opers: pf_operand %d not in hash\n",
                id);

      if (pf_operand->reg == operand_id)
        rdef_opers = Set_add (rdef_opers, pf_operand->pf_oper->oper->id);
    }

  return (rdef_opers);
}


/*
 * AVAILABLE EXPRESSION QUERIES
 * ----------------------------------------------------------------------
 */


int
D_in_cb_EIN_set (PRED_FLOW * pred_flow, L_Cb *cb, L_Oper *reach_oper)
{
  PF_CB *pf_cb;

  if (!pred_flow || !cb || !reach_oper)
    L_punt ("D_in_cb_EIN_set: received NULL!\n");

  pf_cb =
    (PF_CB *) HashTable_find_or_null (pred_flow->hash_cb_pfcb,
                                        cb->id);
  if (!pf_cb)
    L_punt ("D_in_cb_EIN_set: pf_cb %d not in hash\n", cb->id);

  return (Set_in (pf_cb->info->e_in, reach_oper->id));
}


int
D_in_cb_EOUT_set (PRED_FLOW * pred_flow, L_Cb *cb, L_Oper *reach_oper)
{
  PF_CB *pf_cb;

  if (!pred_flow || !cb || !reach_oper)
    L_punt ("D_in_cb_EOUT_set: received NULL!\n");

  pf_cb =
    (PF_CB *) HashTable_find_or_null (pred_flow->hash_cb_pfcb,
                                        cb->id);
  if (!pf_cb)
    L_punt ("D_in_cb_EOUT_set: pf_cb %d not in hash\n", cb->id);

  return (Set_in (pf_cb->info->e_out, reach_oper->id));
}


Set
D_get_cb_EIN_set (PRED_FLOW * pred_flow, L_Cb *cb)
{
  PF_CB *pf_cb;

  if (!pred_flow || !cb)
    L_punt ("D_get_cb_EIN_set: received NULL!\n");

  pf_cb =
    (PF_CB *) HashTable_find_or_null (pred_flow->hash_cb_pfcb,
                                        cb->id);
  if (!pf_cb)
    L_punt ("D_get_cb_EIN_set: pf_cb %d not in hash\n", cb->id);

  return (pf_cb->info->e_in);
}

Set
D_get_cb_EOUT_set (PRED_FLOW * pred_flow, L_Cb *cb)
{
  PF_CB *pf_cb;

  if (!pred_flow || !cb)
    L_punt ("D_get_cb_EOUT_set: received NULL!\n");

  pf_cb =
    (PF_CB *) HashTable_find_or_null (pred_flow->hash_cb_pfcb,
                                        cb->id);
  if (!pf_cb)
    L_punt ("D_get_cb_EOUT_set: pf_cb %d not in hash\n", cb->id);

  return (pf_cb->info->e_out);
}


int
D_in_oper_EIN_set (PRED_FLOW * pred_flow, L_Oper * oper, L_Oper * reach_oper)
{
  PF_OPER *pf_oper;

  if (!pred_flow || !oper || !reach_oper)
    L_punt ("D_in_oper_EIN_set: received NULL!\n");

  pf_oper =
    (PF_OPER *) HashTable_find_or_null (pred_flow->hash_oper_pfoper,
                                        oper->id);
  if (!pf_oper)
    L_punt ("D_in_oper_EIN_set: pf_oper %d not in hash\n", oper->id);

  return (Set_in (pf_oper->info->e_in, reach_oper->id));
}


int
D_in_oper_EOUT_set (PRED_FLOW * pred_flow, L_Oper * oper, L_Oper * reach_oper)
{
  PF_OPER *pf_oper;

  if (!pred_flow || !oper || !reach_oper)
    L_punt ("D_in_oper_EOUT_set: received NULL!\n");

  pf_oper =
    (PF_OPER *) HashTable_find_or_null (pred_flow->hash_oper_pfoper,
                                        oper->id);
  if (!pf_oper)
    L_punt ("D_in_oper_EOUT_set: pf_oper %d not in hash\n", oper->id);

  return (Set_in (pf_oper->info->e_out, reach_oper->id));
}


Set
D_get_oper_EIN_set (PRED_FLOW * pred_flow, L_Oper * oper)
{
  PF_OPER *pf_oper;

  if (!pred_flow || !oper)
    L_punt ("D_get_oper_EIN_set: received NULL!\n");

  pf_oper =
    (PF_OPER *) HashTable_find_or_null (pred_flow->hash_oper_pfoper,
                                        oper->id);
  if (!pf_oper)
    L_punt ("D_get_oper_EIN_set: pf_oper %d not in hash\n", oper->id);

  return (pf_oper->info->e_in);
}


Set
D_get_oper_EOUT_set (PRED_FLOW * pred_flow, L_Oper * oper)
{
  PF_OPER *pf_oper;

  if (!pred_flow || !oper)
    L_punt ("D_get_oper_EOUT_set: received NULL!\n");

  pf_oper =
    (PF_OPER *) HashTable_find_or_null (pred_flow->hash_oper_pfoper,
                                        oper->id);
  if (!pf_oper)
    L_punt ("D_get_oper_EOUT_set: pf_oper %d not in hash\n", oper->id);

  return (pf_oper->info->e_out);
}


/*
 * MEMORY REACHING DEFINITION QUERIES
 * ----------------------------------------------------------------------
 */


Set
D_get_mem_oper_RIN_set (PRED_FLOW * pred_flow, L_Oper * oper)
{
  PF_OPERAND *pf_operand;

  if (!pred_flow || !oper)
    L_punt ("D_get_mem_oper_RIN_set: received NULL!\n");

  pf_operand =
    (PF_OPERAND *) HashTable_find_or_null (pred_flow->hash_pf_mem_operand,
                                           oper->id);
  if (!pf_operand)
    L_punt ("D_get_mem_oper_RIN_set: pf_operand %d not in hash\n", oper->id);

  return (pf_operand->pf_oper->info->mem_r_in);
}


Set
D_get_mem_oper_ROUT_set (PRED_FLOW * pred_flow, L_Oper * oper)
{
  PF_OPERAND *pf_operand;

  if (!pred_flow || !oper)
    L_punt ("D_get_mem_oper_ROUT_set: received NULL!\n");

  pf_operand =
    (PF_OPERAND *) HashTable_find_or_null (pred_flow->hash_pf_mem_operand,
                                           oper->id);
  if (!pf_operand)
    L_punt ("D_get_mem_oper_ROUT_set: pf_operand %d not in hash\n", oper->id);

  return (pf_operand->pf_oper->info->mem_r_out);
}


Set
D_get_mem_oper_RIN_set_rid (PRED_FLOW * pred_flow, L_Oper * oper)
{
  PF_OPERAND *pf_operand;
  Set r, rid;
  int *buf, num, i;

  if (!pred_flow || !oper)
    L_punt ("D_get_mem_oper_RIN_set_rid: received NULL!\n");

  pf_operand =
    (PF_OPERAND *) HashTable_find_or_null (pred_flow->hash_pf_mem_operand,
                                           oper->id);
  if (!pf_operand)
    L_punt ("D_get_mem_oper_RIN_set_rid: pf_operand %d not in hash\n",
            oper->id);
  r = pf_operand->pf_oper->info->mem_r_in;

  num = Set_size (r);
  if (!num)
    return NULL;

  rid = NULL;
  buf = (int *) alloca (sizeof (int) * num);
  Set_2array (r, buf);
  for (i = 0; i < num; i++)
    {
      pf_operand =
        (PF_OPERAND *) HashTable_find_or_null (pred_flow->hash_pf_operand,
                                               buf[i]);
      if (!pf_operand)
        L_punt ("D_get_mem_oper_RIN_set_rid: pf_operand %d not in hash\n",
                buf[i]);
      rid = Set_add (rid, pf_operand->pf_oper->oper->id);
    }

  return rid;
}


Set
D_get_mem_oper_ROUT_set_rid (PRED_FLOW * pred_flow, L_Oper * oper)
{
  PF_OPERAND *pf_operand;
  Set r, rid;
  int *buf, num, i;

  if (!pred_flow || !oper)
    L_punt ("D_get_mem_oper_ROUT_set_rid: received NULL!\n");

  pf_operand =
    (PF_OPERAND *) HashTable_find_or_null (pred_flow->hash_pf_mem_operand,
                                           oper->id);
  if (!pf_operand)
    L_punt ("D_get_mem_oper_ROUT_set_rid: pf_operand %d not in hash\n",
            oper->id);

  r = pf_operand->pf_oper->info->mem_r_out;

  num = Set_size (r);
  if (!num)
    return NULL;

  rid = NULL;
  buf = (int *) alloca (sizeof (int) * num);
  Set_2array (r, buf);
  for (i = 0; i < num; i++)
    {
      pf_operand =
        (PF_OPERAND *) HashTable_find_or_null (pred_flow->hash_pf_operand,
                                               buf[i]);
      if (!pf_operand)
        L_punt ("D_get_mem_oper_ROUT_set_rid: pf_operand %d not in hash\n",
                buf[i]);
      rid = Set_add (rid, pf_operand->pf_oper->oper->id);
    }

  return rid;
}


/* Warning this creates a set, caller must free it */
Set
D_get_mem_oper_RIN_defining_opers (PRED_FLOW * pred_flow, L_Oper * oper,
                                   int flags)
{
  PF_OPERAND *pf_operand;
  Set conflicts, dep_conflicts, amb_conflicts;
  Set rin_def_opers, RIN;
  int id, index, *buf;
  Set rin_conflicts;
  L_Oper *def_oper;
  int num;

  if (!(pred_flow && oper))
    L_punt ("D_get_mem_oper_RIN_defining_opers: pred_flow or oper NULL!\n");

  if (!(L_general_load_opcode (oper) ||
        L_general_store_opcode (oper) || L_subroutine_call_opcode (oper)))
    {
      /* Only works for memory opers */
      return NULL;
    }

  /* Get all reaching mem opers */
  rin_def_opers = NULL;
  pf_operand =
    (PF_OPERAND *) HashTable_find_or_null (pred_flow->hash_pf_mem_operand,
                                           oper->id);
  if (!pf_operand)
    L_punt ("D_get_mem_oper_RIN_defining_opers1: pf_operand %d not in hash\n",
            oper->id);
  RIN = pf_operand->pf_oper->info->mem_r_in;

  /* Create the chosen set of conflicts */
  dep_conflicts = NULL;
  amb_conflicts = NULL;
  if (flags & MDF_RET_DEP)
    {
      dep_conflicts =
        (Set) HashTable_find_or_null (pred_flow->hash_mem_oper_conflict,
                                      pf_operand->id);
    }
  if (flags & MDF_RET_AMB)
    {
      amb_conflicts =
        (Set) HashTable_find_or_null (pred_flow->hash_amb_mem_oper_conflict,
                                      pf_operand->id);
    }
  conflicts = Set_union (dep_conflicts, amb_conflicts);

  /* This is all reaching mem opers that have conflicting addresses  */
  rin_conflicts = Set_intersect (conflicts, RIN);

  /* Get count of reaching conflicts */
  num = Set_size (rin_conflicts);

  if (num > 0)
    {

      buf = (int *) alloca (sizeof (int) * num);
      Set_2array (rin_conflicts, buf);

      for (index = 0; index < num; index++)
        {
          id = buf[index];
          pf_operand =
            (PF_OPERAND *) HashTable_find_or_null (pred_flow->hash_pf_operand,
                                                   id);
          if (!pf_operand)
            L_punt ("D_get_mem_oper_RIN_defining_opers4: "
                    "pf_operand %d not in hash\n", id);

          def_oper = pf_operand->pf_oper->oper;
          if ((flags & MDF_RET_LOADS) && L_general_load_opcode (def_oper))
            rin_def_opers = Set_add (rin_def_opers, def_oper->id);
          if ((flags & MDF_RET_STORES) && L_general_store_opcode (def_oper))
            rin_def_opers = Set_add (rin_def_opers, def_oper->id);
          if ((flags & MDF_RET_JSRS) && L_subroutine_call_opcode (def_oper))
            rin_def_opers = Set_add (rin_def_opers, def_oper->id);
        }
    }

  conflicts = Set_dispose (conflicts);
  rin_conflicts = Set_dispose (rin_conflicts);

  return (rin_def_opers);
}


/* Warning this creates a set, caller must free it */
Set
D_get_mem_oper_ROUT_using_opers (PRED_FLOW * pred_flow, L_Oper * oper,
                                 int flags)
{
  PF_OPERAND *pf_operand;
  Set conflicts, dep_conflicts, amb_conflicts;
  Set rin_use_opers, RIN;
  int id, index, *buf;
  int num, pf_def_operand_id;
  L_Oper *use_oper;

  if (!(pred_flow && oper))
    L_punt ("D_get_mem_oper_ROUT_using_opers: pred_flow or oper NULL!\n");

  if (!(L_general_load_opcode (oper) ||
        L_general_store_opcode (oper) || L_subroutine_call_opcode (oper)))
    {
      /* Only works for memory opers */
      return NULL;
    }

  /* Get all reaching mem opers */
  rin_use_opers = NULL;
  pf_operand =
    (PF_OPERAND *) HashTable_find_or_null (pred_flow->hash_pf_mem_operand,
                                           oper->id);
  if (!pf_operand)
    L_punt ("D_get_mem_oper_ROUT_using_opers: pf_operand %d not in hash\n",
            oper->id);
  pf_def_operand_id = pf_operand->id;

  /* Create the chosen set of conflicts */
  dep_conflicts = NULL;
  amb_conflicts = NULL;
  if (flags & MDF_RET_DEP)
    {
      dep_conflicts =
        (Set) HashTable_find_or_null (pred_flow->hash_mem_oper_conflict,
                                      pf_operand->id);
    }
  if (flags & MDF_RET_AMB)
    {
      amb_conflicts =
        (Set) HashTable_find_or_null (pred_flow->hash_amb_mem_oper_conflict,
                                      pf_operand->id);
    }
  conflicts = Set_union (dep_conflicts, amb_conflicts);

  /* Get count of reaching conflicts */
  num = Set_size (conflicts);

  if (num > 0)
    {

      buf = (int *) alloca (sizeof (int) * num);
      Set_2array (conflicts, buf);

      for (index = 0; index < num; index++)
        {
          id = buf[index];
          pf_operand =
            (PF_OPERAND *) HashTable_find_or_null (pred_flow->hash_pf_operand,
                                                   id);
          if (!pf_operand)
            L_punt
              ("D_get_mem_oper_ROUT_using_opers: pf_operand %d not in hash\n",
               id);

          /* Check to see if def oper is in RIN of conflicting oper */
          RIN = pf_operand->pf_oper->info->mem_r_in;
          if (!Set_in (RIN, pf_def_operand_id))
            continue;

          /* A use has been found */
          use_oper = pf_operand->pf_oper->oper;
          if ((flags & MDF_RET_LOADS) && L_general_load_opcode (use_oper))
            rin_use_opers = Set_add (rin_use_opers, use_oper->id);
          if ((flags & MDF_RET_STORES) && L_general_store_opcode (use_oper))
            rin_use_opers = Set_add (rin_use_opers, use_oper->id);
          if ((flags & MDF_RET_JSRS) && L_subroutine_call_opcode (use_oper))
            rin_use_opers = Set_add (rin_use_opers, use_oper->id);
        }
    }

  conflicts = Set_dispose (conflicts);

  return (rin_use_opers);
}


/*
 * MEMORY AVAILABLE DEFINITION QUERIES
 * ----------------------------------------------------------------------
 */

Set
D_get_mem_oper_AIN_set (PRED_FLOW * pred_flow, L_Oper * oper)
{
  PF_OPERAND *pf_operand;

  if (!pred_flow || !oper)
    L_punt ("D_get_mem_oper_AIN_set: received NULL!\n");

  pf_operand =
    (PF_OPERAND *) HashTable_find_or_null (pred_flow->hash_pf_mem_operand,
                                           oper->id);
  if (!pf_operand)
    L_punt ("D_get_mem_oper_AIN_set: pf_operand %d not in hash\n", oper->id);

  return (pf_operand->pf_oper->info->mem_a_in);
}


Set
D_get_mem_oper_AOUT_set (PRED_FLOW * pred_flow, L_Oper * oper)
{
  PF_OPERAND *pf_operand;

  if (!pred_flow || !oper)
    L_punt ("D_get_mem_oper_AOUT_set: received NULL!\n");

  pf_operand =
    (PF_OPERAND *) HashTable_find_or_null (pred_flow->hash_pf_mem_operand,
                                           oper->id);
  if (!pf_operand)
    L_punt ("D_get_mem_oper_AOUT_set: pf_operand %d not in hash\n", oper->id);

  return (pf_operand->pf_oper->info->mem_a_out);
}

Set
D_get_mem_oper_AIN_set_rid (PRED_FLOW * pred_flow, L_Oper * oper)
{
  PF_OPERAND *pf_operand;
  Set r, rid;
  int *buf, num, i;

  if (!pred_flow || !oper)
    L_punt ("D_get_mem_oper_AIN_set_rid: received NULL!\n");

  pf_operand =
    (PF_OPERAND *) HashTable_find_or_null (pred_flow->hash_pf_mem_operand,
                                           oper->id);
  if (!pf_operand)
    L_punt ("D_get_mem_oper_AIN_set_rid: pf_operand %d not in hash\n",
            oper->id);
  r = pf_operand->pf_oper->info->mem_a_in;

  num = Set_size (r);
  if (!num)
    return NULL;

  rid = NULL;
  buf = (int *) alloca (sizeof (int) * num);
  Set_2array (r, buf);
  for (i = 0; i < num; i++)
    {
      pf_operand =
        (PF_OPERAND *) HashTable_find_or_null (pred_flow->hash_pf_operand,
                                               buf[i]);
      if (!pf_operand)
        L_punt ("D_get_mem_oper_AIN_set_rid: pf_operand %d not in hash\n",
                buf[i]);
      rid = Set_add (rid, pf_operand->pf_oper->oper->id);
    }

  return rid;
}

Set
D_get_mem_oper_AOUT_set_rid (PRED_FLOW * pred_flow, L_Oper * oper)
{
  PF_OPERAND *pf_operand;
  Set r, rid;
  int *buf, num, i;

  if (!pred_flow || !oper)
    L_punt ("D_get_mem_oper_AOUT_set_rid: received NULL!\n");

  pf_operand =
    (PF_OPERAND *) HashTable_find_or_null (pred_flow->hash_pf_mem_operand,
                                           oper->id);
  if (!pf_operand)
    L_punt ("D_get_mem_oper_AOUT_set_rid: pf_operand %d not in hash\n",
            oper->id);
  r = pf_operand->pf_oper->info->mem_a_out;

  num = Set_size (r);
  if (!num)
    return NULL;

  rid = NULL;
  buf = (int *) alloca (sizeof (int) * num);
  Set_2array (r, buf);
  for (i = 0; i < num; i++)
    {
      pf_operand =
        (PF_OPERAND *) HashTable_find_or_null (pred_flow->hash_pf_operand,
                                               buf[i]);
      if (!pf_operand)
        L_punt ("D_get_mem_oper_AOUT_set_rid: pf_operand %d not in hash\n",
                buf[i]);
      rid = Set_add (rid, pf_operand->pf_oper->oper->id);
    }

  return rid;
}

/* Warning this creates a set, caller must free it */
Set
D_get_mem_oper_AIN_defining_opers (PRED_FLOW * pred_flow, L_Oper * oper,
                                   int flags)
{
  PF_OPERAND *pf_operand;
  Set conflicts, dep_conflicts, amb_conflicts;
  Set ain_def_opers, AIN;
  int id, index, *buf;
  Set ain_conflicts;
  int num;
  L_Oper *def_oper;

  if (!(pred_flow && oper))
    L_punt ("D_get_mem_oper_AIN_defining_opers: pred_flow or oper NULL!\n");

  if (!(L_general_load_opcode (oper) ||
        L_general_store_opcode (oper) || L_subroutine_call_opcode (oper)))
    {
      /* Only works for memory opers */
      return NULL;
    }

  /* Get all reaching mem opers */
  ain_def_opers = NULL;
  pf_operand =
    (PF_OPERAND *) HashTable_find_or_null (pred_flow->hash_pf_mem_operand,
                                           oper->id);
  if (!pf_operand)
    L_punt ("D_get_mem_oper_AIN_defining_opers1: pf_operand %d not in hash\n",
            oper->id);
  AIN = pf_operand->pf_oper->info->mem_a_in;

  /* Create the chosen set of conflicts */
  dep_conflicts = NULL;
  amb_conflicts = NULL;
  if (flags & MDF_RET_DEP)
    {
      dep_conflicts =
        (Set) HashTable_find_or_null (pred_flow->hash_mem_oper_conflict,
                                      pf_operand->id);
    }
  if (flags & MDF_RET_AMB)
    {
      amb_conflicts =
        (Set) HashTable_find_or_null (pred_flow->hash_amb_mem_oper_conflict,
                                      pf_operand->id);
    }
  conflicts = Set_union (dep_conflicts, amb_conflicts);
  /* This is all available mem opers that have conflicting addresses  */
  ain_conflicts = Set_intersect (conflicts, AIN);

  /* Get count of available conflicts */
  num = Set_size (ain_conflicts);

  if (num > 0)
    {

      buf = (int *) alloca (sizeof (int) * num);
      Set_2array (ain_conflicts, buf);

      for (index = 0; index < num; index++)
        {
          id = buf[index];
          pf_operand =
            (PF_OPERAND *) HashTable_find_or_null (pred_flow->hash_pf_operand,
                                                   id);
          if (!pf_operand)
            L_punt ("D_get_mem_oper_AIN_defining_opers4: "
                    "pf_operand %d not in hash\n", id);

          def_oper = pf_operand->pf_oper->oper;
          if ((flags & MDF_RET_LOADS) && L_general_load_opcode (def_oper))
            ain_def_opers = Set_add (ain_def_opers, def_oper->id);
          if ((flags & MDF_RET_STORES) && L_general_store_opcode (def_oper))
            ain_def_opers = Set_add (ain_def_opers, def_oper->id);
          if ((flags & MDF_RET_JSRS) && L_subroutine_call_opcode (def_oper))
            ain_def_opers = Set_add (ain_def_opers, def_oper->id);
        }
    }

  conflicts = Set_dispose (conflicts);
  ain_conflicts = Set_dispose (ain_conflicts);

  return (ain_def_opers);
}


/*
 * PARTIAL CODE ELIMINATION QUERIES
 * ----------------------------------------------------------------------
 */

int
D_PRE_cb_no_changes (PRED_FLOW * pred_flow, L_Cb * cb, Set ignore_set)
{
  PF_CB *pf_cb;
  PF_BB *pf_bb;
  DF_PCE_INFO *pce;
  Set change_set;

  if (!pred_flow || !cb)
    L_punt ("D_PRE_cb_no_changes: received NULL!\n");

  pf_cb =  (PF_CB *) HashTable_find_or_null (pred_flow->hash_cb_pfcb, cb->id);
  if (!pf_cb)
    L_punt ("D_PRE_cb_no_changes: pf_cb %d not in hash.\n", cb->id);

  if (Set_empty (ignore_set))
    {
      PF_FOREACH_BB(pf_bb, pf_cb->pf_bbs)
        {
	  pce = pf_bb->info->pce_info;
	  if (!pce)
	    L_punt("Calling D_PRE_cb_no_changes without running PRE "
		   "dataflow!\n");
          if (!Set_empty (pce->n_insert))
            return 0;
          if (!Set_empty (pce->x_insert))
            return 0;
          if (!Set_empty (pce->n_replace))
            return 0;
          if (!Set_empty (pce->x_replace))
            return 0;
        }
    }
  else
    {
      change_set = NULL;
      PF_FOREACH_BB(pf_bb, pf_cb->pf_bbs)
        {
	  pce = pf_bb->info->pce_info;
	  if (!pce)
	    L_punt("Calling D_PRE_cb_no_changes without running PCE "
		   "dataflow!\n");

	  if (!Set_subtract_empty (pce->n_insert, ignore_set))
	    return 0;
	  if (!Set_subtract_empty (pce->x_insert, ignore_set))
	    return 0;
	  if (!Set_subtract_empty (pce->n_replace, ignore_set))
	    return 0;
	  if (!Set_subtract_empty (pce->x_replace, ignore_set))
	    return 0;

#if 0
          change_set = 
	    Set_union_acc (change_set, pce->n_insert);
          change_set = 
	    Set_union_acc (change_set, pce->x_insert);
          change_set = 
	    Set_union_acc (change_set, pce->n_replace);
          change_set = 
	    Set_union_acc (change_set, pce->x_replace);
#endif
	}
#if 0
      if (!(Set_subtract_empty (change_set, ignore_set)))
	{
	  Set_dispose (change_set);
	  return 0;
	}
#endif
      Set_dispose (change_set);
    }
  return 1;
}


int
D_PDE_cb_no_changes (PRED_FLOW * pred_flow, L_Cb * cb)
{
  PF_CB * pf_cb;
  PF_BB * pf_bb;
  DF_PCE_INFO * pce;

  if (!pred_flow || !cb)
    L_punt ("D_PDE_cb_no_changes: received NULL!\n");

  pf_cb = (PF_CB *) HashTable_find_or_null (pred_flow->hash_cb_pfcb, cb->id);
  if (!pf_cb)
    L_punt ("D_PDE_cb_no_changes: pf_cb %d not in hash.\n", cb->id);

  PF_FOREACH_BB (pf_bb, pf_cb->pf_bbs)
    {
      pce = pf_bb->info->pce_info;
      if (!pce)
	L_punt ("Calling D_PDE_cb_no_changes without running PDE "
		"dataflow!\n");
      if (!Set_empty (pce->n_insert))
	return 0;
      if (!Set_empty (pce->x_replace))
	return 0;
      if (!Set_empty (pce->pred_clear))
	return 0;
    }
  return 1;
}

DF_PCE_INFO *
D_get_PCE_bb_info (PRED_FLOW * pred_flow, L_Cb * cb, L_Oper * first_op)
{
  PF_CB * pf_cb;
  PF_BB * pf_bb;

  /* Don't check for first_op: might be an empty CB. */
  if (!pred_flow || !cb)
    L_punt ("D_get_PCE_bb_info: received NULL!");
  pf_cb = (PF_CB *) HashTable_find_or_null (pred_flow->hash_cb_pfcb, cb->id);
  if (!pf_cb)
    L_punt ("D_get_PCE_bb_info: pf_cb &d not in hash.", cb->id);

  PF_FOREACH_BB (pf_bb, pf_cb->pf_bbs)
    {
      if (pf_bb->first_op == first_op)
	return (pf_bb->info->pce_info);
    }
  L_punt ("D_get_PCE_bb_info: basic block beginning with oper %d not "
	  "in cb %d's list.", first_op->id, cb->id);
  return NULL;
}

Set
D_get_PCE_bb_complement_set (PRED_FLOW * pred_flow, L_Cb * cb,
			     L_Oper * first_op)
{
  PF_CB *pf_cb;
  PF_BB *pf_bb;
 
  /* Don't check for first_op here: might be an empty. CB */
  if (!pred_flow || !cb)
    L_punt ("D_get_PCE_bb_complement_set: received NULL!");
  pf_cb = (PF_CB *) HashTable_find_or_null (pred_flow->hash_cb_pfcb, cb->id);
  if (!pf_cb)
    L_punt ("D_get_PCE_bb_complement_set: pf_cb %d not in hash.", cb->id);

  PF_FOREACH_BB(pf_bb, pf_cb->pf_bbs)
    {
      if (pf_bb->first_op == first_op)
	return (pf_bb->info->pce_info->complement);
    }
  L_punt ("D_get_PCE_bb_complement_set: basic block beginning with oper %d "
	  "not in cb %d's list.", first_op->id, cb->id);
  return NULL;
}


Set
D_get_PCE_bb_nd_safe_set (PRED_FLOW * pred_flow, L_Cb * cb, L_Oper * first_op)
{
  PF_CB *pf_cb;
  PF_BB *pf_bb;
 
  /* Don't check for first_op here: might be an empty CB */
  if (!pred_flow || !cb)
    L_punt ("D_get_PCE_bb_nd_safe_set: received NULL!");
  pf_cb = (PF_CB *) HashTable_find_or_null (pred_flow->hash_cb_pfcb, cb->id);
  if (!pf_cb)
    L_punt ("D_get_PCE_bb_nd_safe_set: pf_cb %d not in hash.", cb->id);

  PF_FOREACH_BB(pf_bb, pf_cb->pf_bbs)
    {
      if (pf_bb->first_op == first_op)
	return (pf_bb->info->pce_info->nd_safe);
    }
  L_punt ("D_get_PCE_bb_nd_safe_set: basic block beginning with oper %d not ",
          "in cb %d's list.", first_op->id, cb->id);
  return NULL;
}


Set
D_get_PCE_bb_xd_safe_set (PRED_FLOW * pred_flow, L_Cb * cb, L_Oper * first_op)
{
  PF_CB *pf_cb;
  PF_BB *pf_bb;
 
  /* Don't check for first_op here: might be an empty CB */
  if (!pred_flow || !cb)
    L_punt ("D_get_PCE_bb_xd_safe_set: received NULL!");
  pf_cb = (PF_CB *) HashTable_find_or_null (pred_flow->hash_cb_pfcb, cb->id);
  if (!pf_cb)
    L_punt ("D_get_PCE_bb_xd_safe_set: pf_cb %d not in hash.", cb->id);

  PF_FOREACH_BB(pf_bb, pf_cb->pf_bbs)
    {
      if (pf_bb->first_op == first_op)
	return (pf_bb->info->pce_info->xd_safe);
    }
  L_punt ("D_get_PCE_bb_xd_safe_set: basic block beginning with oper %d not ",
          "in cb %d's list.", first_op->id, cb->id);
  return NULL;
}


Set
D_get_PCE_bb_n_insert_set (PRED_FLOW * pred_flow, L_Cb * cb, L_Oper * first_op)
{
  PF_CB *pf_cb;
  PF_BB *pf_bb;
 
  /* Don't check for first_op here: might be an empty CB */
  if (!pred_flow || !cb)
    L_punt ("D_get_PCE_bb_n_insert_set: received NULL!");

  pf_cb = (PF_CB *) HashTable_find_or_null (pred_flow->hash_cb_pfcb, cb->id);
  if (!pf_cb)
    L_punt ("D_get_PCE_bb_n_insert_set: pf_cb %d not in hash.", cb->id);

  PF_FOREACH_BB(pf_bb, pf_cb->pf_bbs)
    {
      if (pf_bb->first_op == first_op)
	return (pf_bb->info->pce_info->n_insert);
    }
  L_punt ("D_get_PCE_bb_n_insert_set: basic block beginning with oper %d not ",
          "in cb %d's list.", first_op->id, cb->id);
  return NULL;
}


Set
D_get_PCE_bb_x_insert_set (PRED_FLOW * pred_flow, L_Cb * cb, L_Oper * first_op)
{
  PF_CB *pf_cb;
  PF_BB *pf_bb;

  /* Don't check for first_op here: might be an empty CB */
  if (!pred_flow || !cb)
    L_punt ("D_get_PCE_bb_x_insert_set: received NULL!");

  pf_cb = (PF_CB *) HashTable_find_or_null (pred_flow->hash_cb_pfcb, cb->id);
  if (!pf_cb)
    L_punt ("D_get_PCE_bb_x_insert_set: pf_cb %d not in hash.", cb->id);

  PF_FOREACH_BB(pf_bb, pf_cb->pf_bbs)
    {
      if (pf_bb->first_op == first_op)
	return (pf_bb->info->pce_info->x_insert);
    }
  L_punt ("D_get_PCE_bb_x_insert_set: basic block beginning with oper %d not ",
          "in cb %d's list.", first_op->id, cb->id);
  return NULL;
}


Set
D_get_PCE_bb_n_replace_set (PRED_FLOW * pred_flow, L_Cb * cb, 
			    L_Oper * first_op)
{
  PF_CB *pf_cb;
  PF_BB *pf_bb;
 
  if (!pred_flow || !cb || !first_op)
    L_punt ("D_get_PCE_bb_n_replace_set: received NULL!");

  pf_cb = (PF_CB *) HashTable_find_or_null (pred_flow->hash_cb_pfcb, cb->id);
  if (!pf_cb)
    L_punt ("D_get_PCE_bb_n_replace_set: pf_cb %d not in hash.", cb->id);

  PF_FOREACH_BB(pf_bb, pf_cb->pf_bbs)
    {
      if (pf_bb->first_op == first_op)
	return (pf_bb->info->pce_info->n_replace);
    }
  L_punt ("D_get_PCE_bb_n_replace_set: basic block beginning with oper %d not ",
          "in cb %d's list.", first_op->id, cb->id);
  return NULL;
}


Set
D_get_PCE_bb_x_replace_set (PRED_FLOW * pred_flow, L_Cb * cb, 
			    L_Oper * first_op)
{
  PF_CB *pf_cb;
  PF_BB *pf_bb;
 
  if (!pred_flow || !cb || !first_op)
    L_punt ("D_get_PCE_bb_x_replace_set: received NULL!");

  pf_cb = (PF_CB *) HashTable_find_or_null (pred_flow->hash_cb_pfcb, cb->id);
  if (!pf_cb)
    L_punt ("D_get_PCE_bb_x_replace_set: pf_cb %d not in hash.", cb->id);

  PF_FOREACH_BB(pf_bb, pf_cb->pf_bbs)
    {
      if (pf_bb->first_op == first_op)
	return (pf_bb->info->pce_info->x_replace);
    }
  L_punt ("D_get_PCE_bb_x_replace_set: basic block beginning with oper %d "
	  "not in cb %d's list.", first_op->id, cb->id);
  return NULL;
}


PF_BB *
D_find_pf_bb (PRED_FLOW * pred_flow, L_Cb * cb, L_Oper * first_op)
{
  PF_CB * pf_cb;
  PF_BB * pf_bb;

  if (!pred_flow || !cb)
    L_punt ("D_find_pf_bb: received NULL!");

  pf_cb = (PF_CB *) HashTable_find_or_null (pred_flow->hash_cb_pfcb, cb->id);
  if (!pf_cb)
    L_punt ("D_find_pf_bb: pf_cb %d not in hash.", cb->id);

  PF_FOREACH_BB (pf_bb, pf_cb->pf_bbs)
    {
      if (pf_bb->first_op == first_op)
	return pf_bb;
    }
  L_punt ("D_find_pf_bb: pf_bb beginning with oper %d not in cb %d's list.",
	  first_op->id, cb->id);
  return NULL;
}
