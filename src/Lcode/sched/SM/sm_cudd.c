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
 *      File:   sm_cudd.c
 *      Provides: interface to CUDD BDD expressions for SM
 *      Author: John W. Sias
 *      Creation Date:  August 2003
\*****************************************************************************/

#include <config.h>
#include "sm.h"
#include <bdd/cudd.h>
#include <library/i_bdd_interface.h>
#include "sm_cudd.h"

/* SM_logexpr_init
 * ----------------------------------------------------------------------
 * Initializes a logic expression for the execution condition of the
 * specified register action sm_ract.  A pointer to the expression is
 * stored in the locaion pointed to by p_sm_le.  A void* is used to
 * encapsulate the definition of the logic expression---for now it
 * is simply a CUDD BDD node.
 */
void
SM_logexpr_init (void **p_sm_le, SM_Reg_Action *sm_ract)
{
  PG_Pred_Graph *pg = PG_pred_graph;
  SM_Oper *sm_op = sm_ract->sm_op;

  if (sm_ract->flags & (SM_PRED_UNCOND_DEF|SM_PRED_UNCOND_USE))
    *p_sm_le = pg->one;
  else if (!(sm_ract->flags & SM_DEF_ACTION) && sm_op->lcode_op->pred[1])
    *p_sm_le = sm_op->lcode_op->pred[1]->value.pred.ssa->node;
  else if (sm_op->lcode_op->pred[0])
    *p_sm_le = sm_op->lcode_op->pred[0]->value.pred.ssa->node;
  else
    *p_sm_le = pg->one;

  Cudd_Ref (*p_sm_le);

  return;
}


/* SM_logexpr_sub_accum_def
 * ----------------------------------------------------------------------
 * Performs a subtract-accumulate operation on the logic expression
 * *p_sm_le, using the execution condition of the register action
 * sm_def_act, and return a boolean test for non-emptiness of the result.
 * This applies definition, as opposed to use, rules to sm_def_act.
 */
int /* ne */
SM_logexpr_sub_accum_def (void **p_sm_le, SM_Reg_Action *sm_def_act)
{
  DdNode *tmp;
  PG_Pred_Graph *pg = PG_pred_graph;
  SM_Oper *sm_op = sm_def_act->sm_op;

  if (sm_def_act->flags & (SM_TRANSPARENT_ACTION|SM_PRED_TRANS_DEF))
    {
      /* Do not affect liveness */
      tmp = *p_sm_le;
    }
  else
    {
      if ((sm_def_act->flags & SM_PRED_UNCOND_DEF) || 
	  !sm_op->lcode_op->pred[0])
	tmp = pg->one;
      else
	tmp = sm_op->lcode_op->pred[0]->value.pred.ssa->node;

      tmp = Cudd_bddAnd (pg->dd, (DdNode *)*p_sm_le, Cudd_Not (tmp));
      Cudd_Ref (tmp);
      Cudd_RecursiveDeref (pg->dd, *p_sm_le);
    }

  *p_sm_le = tmp;
  return (tmp != pg->zero);
}


/* SM_logexpr_ne
 * ----------------------------------------------------------------------
 * Return 1 if the logic expression is not empty; 0 otherwise.
 */
int
SM_logexpr_ne (void *p_sm_le)
{
  PG_Pred_Graph *pg = PG_pred_graph;
  return ((DdNode *)p_sm_le != pg->zero);
}


/* SM_logexpr_dispose
 * ----------------------------------------------------------------------
 * Clean up a logic expression.
 */
void
SM_logexpr_dispose (void **p_sm_le)
{
  PG_Pred_Graph *pg = PG_pred_graph;

  Cudd_RecursiveDeref (pg->dd, *p_sm_le);
  *p_sm_le = NULL;
  return;
}
