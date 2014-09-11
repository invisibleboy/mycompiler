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

#include <config.h>
#include <Lcode/l_main.h>
#include "l_ssa.h"
#include "l_dom.h"

#define L_SSA_PRUNED 1
#define L_SSA_DEBUG  0

static L_Alloc_Pool *LS_alloc_ssa = NULL;
static L_Alloc_Pool *LS_alloc_basename = NULL;
static ITint32 LS_ssa_max_id = 0;

HashTable LS_hash_ssa_id = NULL;
HashTable LS_hash_basename = NULL;

L_SSA_BaseName *
L_new_basename (L_Operand *opd)
{
  L_SSA_BaseName *bn;

  bn = L_alloc (LS_alloc_basename);

  bn->opd = L_copy_operand (opd);
  bn->opd->ptype = L_PTYPE_NULL;
  bn->defnodes = NULL;
  bn->pseudodefnodes = NULL;
  bn->phinodes = NULL;
  bn->ssas = NULL;
  bn->ssaset = NULL;
  bn->def_stk = NULL;
  bn->maxid = 0;
  HashTable_insert (LS_hash_basename, L_REG_MAC_INDEX (opd), bn);

  return bn;
}

L_SSA_BaseName *
L_delete_basename (L_SSA_BaseName *bn)
{
#if 0
  HashTable_remove (LS_hash_basename, L_REG_MAC_INDEX (bn->opd));
#endif

  if (bn->opd)
    L_delete_operand (bn->opd);

  if (bn->defnodes)
    Set_dispose (bn->defnodes);

  if (bn->pseudodefnodes)
    Set_dispose (bn->pseudodefnodes);

  if (bn->phinodes)
    Set_dispose (bn->phinodes);

  if (bn->ssas)
    List_reset (bn->ssas);

  if (bn->ssaset)
    Set_dispose (bn->ssaset);

  L_free (LS_alloc_basename, bn);

  return NULL;
}

L_SSA *
L_new_ssa (L_Oper *def_op, L_Operand *def_opd, ITint32 node_id)
{
  L_SSA *ssa;
  L_SSA_BaseName *bn;
  LD_ControlNode *cn;

  ssa = L_alloc (LS_alloc_ssa);

  ssa->def = def_op;
  ssa->defopd = def_opd;
  ssa->flags = 0;
  ssa->dfid = L_REG_MAC_INDEX (def_opd);
  ssa->rd_pr = NULL;
  ssa->rd_lb = NULL;
  ssa->uses = NULL;
  ssa->info = NULL;

  if (!(bn = HashTable_find_or_null (LS_hash_basename, ssa->dfid)))
    bn = L_new_basename (def_opd);

  ssa->id = bn->maxid++;
  ssa->gid = ++LS_ssa_max_id;

  HashTable_insert (LS_hash_ssa_id, ssa->id, ssa);

  bn->defnodes = Set_add (bn->defnodes, node_id);

  if (L_is_ctype_predicate (def_opd))
    {
      cn = LDNODE (node_id);
      node_id = cn->uncond->dfn;
    }

  bn->pseudodefnodes = Set_add (bn->pseudodefnodes, node_id);

  bn->ssaset = Set_add (bn->ssaset, ssa->id);

  ssa->bn = bn;

  def_opd->ssa = ssa;

  ssa->dfn = node_id;

  return ssa;
}

L_SSA *
L_delete_ssa (L_SSA *ssa)
{
  if (ssa->rd_pr)
    List_reset (ssa->rd_pr);

  if (ssa->rd_lb)
    List_reset (ssa->rd_lb);

  if (ssa->uses)
    List_reset (ssa->uses);

#if 0
  HashTable_remove (LS_hash_ssa_id, ssa->id);
#endif

  L_free (LS_alloc_ssa, ssa);

  return NULL;
}


void
L_ssa_init (L_Func *fn)
{
  if (LS_hash_ssa_id)
    HashTable_reset_func (LS_hash_ssa_id, 
			  (void (*)(void *)) L_delete_ssa);
  else
    LS_hash_ssa_id = HashTable_create (512);

  if (LS_hash_basename)
    HashTable_reset_func (LS_hash_basename, 
			  (void (*)(void *)) L_delete_basename);
  else
    LS_hash_basename = HashTable_create (512);

  LS_ssa_max_id = 0;

  if (!LS_alloc_ssa)
    LS_alloc_ssa = L_create_alloc_pool ("L_SSA", sizeof (L_SSA), 256);

  if (!LS_alloc_basename)
    LS_alloc_basename = L_create_alloc_pool ("L_SSA_BaseName", 
					     sizeof (L_SSA_BaseName), 256);

  return;
}

void
L_ssa_deinit (L_Func *fn)
{
  if (LS_hash_ssa_id)
    {
      HashTable_free_func (LS_hash_ssa_id, 
			   (void (*)(void *)) L_delete_ssa);

      LS_hash_ssa_id = NULL;
    }

  if (LS_hash_basename)
    {
      HashTable_free_func (LS_hash_basename, 
			   (void (*)(void *)) L_delete_basename);

      LS_hash_basename = NULL;
    }

  LS_ssa_max_id = 0;

  if (LS_alloc_ssa)
    {
      L_free_alloc_pool (LS_alloc_ssa);
      LS_alloc_ssa = NULL;
    }

  if (LS_alloc_basename)
    {
      L_free_alloc_pool (LS_alloc_basename);
      LS_alloc_basename = NULL;
    }

  return;
}

void LS_push_def (int i, L_SSA *d, List *dl);

static L_SSA *
LS_handle_uninit (L_Operand *opd)
{
  L_SSA *newssa;
  L_Oper *def;
  L_Attr *attr;
  L_Cb *cb = L_fn->first_cb;
  LD_ControlNode *dcb = (LD_ControlNode *) cb->ext;

  if (L_is_register (opd))
    L_warn ("Lcode SSA: Uninitialized use of register r%d", 
	    opd->value.r);
  else
    L_warn ("Lcode SSA: Uninitialized use of macro %s",
	    L_macro_name(opd->value.mac));

  def = L_create_new_op (Lop_DEFINE);
  attr = L_new_attr ("SSAuninit",0);
  def->attr = L_concat_attr (def->attr, attr);
  def->dest[0] = L_copy_operand (opd);
  if (L_is_ctype_predicate (def->dest[0]))
    L_assign_ptype_uncond_true(def->dest[0]);
  L_insert_oper_before (cb, dcb->opS, def);

  dcb->opS = def;

  newssa = L_new_ssa (def, def->dest[0], dcb->dfn);
  LS_push_def (L_REG_MAC_INDEX (opd), newssa, NULL);

  return newssa;
}

L_SSA *
LS_top_def (L_Operand *opd)
{
  L_SSA_BaseName *bn;
  L_SSA *topssa;
  int i;

  i = L_REG_MAC_INDEX (opd);

  if (!(bn = HashTable_find_or_null (LS_hash_basename, i)) ||
      !(topssa = List_last (bn->def_stk)))
    topssa = LS_handle_uninit (opd);

  return topssa;
}


void
LS_push_def (int i, L_SSA *d, List *dl)
{
  L_SSA_BaseName *bn;

  bn = HashTable_find (LS_hash_basename, i);

  bn->def_stk = List_insert_last (bn->def_stk, d);

  if (dl)
#ifdef LP64_ARCHITECTURE
    *dl = List_insert_last (*dl, (void *)((long)i));
#else
    *dl = List_insert_last (*dl, (void *) i);
#endif

  return;
}

void
LS_pop_def (int i)
{
  L_SSA_BaseName *bn;

  bn = HashTable_find (LS_hash_basename, i);

  List_last (bn->def_stk);
  bn->def_stk = List_delete_current (bn->def_stk);

  return;
}

void
LS_annotate_phis (LD_ControlNode *cn)
{
  L_Oper *op;
  L_SSA *ssa, *dssa;

  for (op = cn->opS; op; op = op->next_op)
    {
      if (L_ssa_opcode (op))
	{
	  ssa = op->dest[0]->ssa;
	  dssa = LS_top_def (op->dest[0]);
	  if (ssa->dfn > dssa->dfn)
	    ssa->rd_pr = List_insert_last (ssa->rd_pr, dssa);
	  else
	    ssa->rd_lb = List_insert_last (ssa->rd_lb, dssa);
	}

      if (op == cn->opE)
	break;
    }

}

void
L_ssa_populate_uses (L_Func *fn)
{
  L_Cb *cb;
  L_Oper *op;
  L_Operand *opd;
  L_Attr *attr;
  int i;

  for (cb = fn->first_cb; cb; cb = cb->next_cb)
    {
      for (op = cb->first_op; op; op = op->next_op)
	{
	  if ((opd = op->pred[0]) && opd->ssa)
	    opd->ssa->uses = List_insert_last (opd->ssa->uses, (void *)op);

	  for (i = 0; i < L_max_src_operand; i++)
	    {
	      if ((opd = op->src[i]) && opd->ssa)
		opd->ssa->uses = List_insert_last (opd->ssa->uses, (void *)op);
	    }

	  if ((attr = L_find_attr (op->attr, "tr")))
	    {
	      for (i = 0; i < attr->max_field; i++)
		{
		  if ((opd = attr->field[i]) && opd->ssa)
		    opd->ssa->uses = List_insert_last (opd->ssa->uses, (void *)op);
		}
	    }

	  if (L_ssa_opcode (op))
	    {
	      L_SSA *ssa = op->dest[0]->ssa, *rd;
	      
	      List_start (ssa->rd_pr);
	      while ((rd = List_next (ssa->rd_pr)))
		rd->uses = List_insert_last (rd->uses, op);

	      List_start (ssa->rd_lb);
	      while ((rd = List_next (ssa->rd_lb)))
		rd->uses = List_insert_last (rd->uses, op);
	    }
        }
    }
  
  return;
}


int
L_ssa_insert_phi_functions (L_SSA_BaseName *bn)
{
  int cnt, i, *buf, ins = 0;

  if (!(cnt = Set_size (bn->phinodes)))
    return 0;

  buf = alloca (cnt * sizeof (int));

  Set_2array (bn->phinodes, buf);

  for (i = 0; i < cnt; i++)
    {
      LD_ControlNode *n = LDNODE(buf[i]);
      L_Cb *cb = n->cb;
      L_Oper *phi;

#if L_SSA_PRUNED
      if (!Set_in (n->live_in, L_REG_MAC_INDEX(bn->opd)))
	continue;
#endif

      ins++;
	
      phi = L_create_new_op (Lop_PHI);

      L_insert_oper_before (cb, n->opS, phi);

      if (n->guard)
	phi->pred[0] = L_copy_operand (n->guard);

      n->opS = phi;

      phi->dest[0] = L_copy_operand (bn->opd);

      L_new_ssa (phi, phi->dest[0], n->dfn);
    }

  return ins;
}

extern L_SSA *LS_top_def (L_Operand *opd);
extern void LS_push_def (int i, L_SSA *d, List *dl);
extern void LS_annotate_phis (LD_ControlNode *cn);

void
L_SSA_subs_recur (LD_ControlNode *n)
{
  L_Cb *cb;
  L_Oper *op, *nop;
  L_Operand *opd;
  LD_ControlNode *chi, *sn, *unode;
  L_Attr *attr;
  int i;
  int un, ucp; /* uncond promotion */
  List dlist = NULL;

  cb = n->cb;

  /* Annotate uses, push defs */

  ucp = 0;
  unode = n;

  un = ((n->flags & LD_NODE_UNCOND) != 0);

  for (op = n->opS; op; op = nop)
    {
      /* Annotate sources */

      /* PREDICATE GUARD -- annotate in uncond pass */

      if (un && (opd = op->pred[0]) && L_is_variable (opd))
	opd->ssa = LS_top_def (opd);

      if (!ucp)
	{
	  /* SOURCES -- annotate in predicated pass */

	  for (i = 0; i < L_max_src_operand; i++)
	    {
	      if ((opd = op->src[i]) &&
		  L_is_variable (opd))
		opd->ssa = LS_top_def (opd);
	    }
	  if ((L_subroutine_call_opcode (op) ||
	       L_subroutine_return_opcode (op)) && 
	      (attr = L_find_attr (op->attr, "tr")))
	    {
	      for (i = 0; i < attr->max_field; i++)
		{
		  if ((opd = attr->field[i]) &&
		      L_is_variable (opd))
		    opd->ssa = LS_top_def (opd);
		}
	    }

	  /* Push definitions */

	  for (i = 0; i < L_max_dest_operand; i++)
	    {
	      if ((opd = op->dest[i]) &&
		  L_is_variable (opd))
		LS_push_def (L_REG_MAC_INDEX (opd), opd->ssa, &dlist);
	    }

	  if (L_subroutine_call_opcode (op) && 
	      (attr = L_find_attr (op->attr, "ret")))
	    {
	      for (i = 0; i < attr->max_field; i++)
		{
		  if ((opd = attr->field[i]) &&
		      L_is_variable (opd))
		    LS_push_def (L_REG_MAC_INDEX (opd), opd->ssa, &dlist);
		}
	    }
	}
      else if (un && L_pred_define_opcode (op))
	{
	  for (i = 0; i < L_max_dest_operand; i++)
	    {
	      if ((opd = op->dest[i]) &&
		  L_is_variable (opd) &&
		  L_is_ctype_predicate (opd))
		LS_push_def (L_REG_MAC_INDEX (opd), opd->ssa, &dlist);
	    }
	}

      nop = op->next_op;

      if (op == n->opE)
	{
	  if (!n->lsucc || !un || (n->lsucc->uncond != unode))
	    break;

	  n = n->lsucc;
	  nop = n->opS;
	  ucp = 1;
	}
    }

  n = unode;

  List_start (n->succ);
  while ((sn = (LD_ControlNode *) List_next (n->succ)))
    LS_annotate_phis (sn);

  /* Recurse on dom tree children */

  chi = n->dtchi;

  while (chi)
    {
      L_SSA_subs_recur (chi);
      chi = chi->dtsib;
    }

  /* Pop defs */

  List_start (dlist);
#ifdef LP64_ARCHITECTURE
  while ((i = (int)((long) List_next (dlist))))
#else
  while ((i = (int) List_next (dlist)))
#endif
    {
      dlist = List_delete_current (dlist);
      LS_pop_def (i);
    }

  return;
}

void
L_form_ssa (L_Func *fn)
{
  L_Cb *cb;
  ITint32 id;

  /* Set up dummy operations for predicated code */

  {
    for (cb = fn->first_cb; cb; cb = cb->next_cb)
      {
	int cprev = 1;
	L_Oper *nop, *op = cb->first_op;

	if (!op)
	  {
	    nop = L_create_new_op (Lop_NO_OP);
	    L_insert_oper_before (cb, op, nop);
	  }
	else
	  {
	    for (; op; op = op->next_op)
	      {
		if (cprev && op->pred[0])
		  {
		    nop = L_create_new_op (Lop_NO_OP);
		    L_insert_oper_before (cb, op, nop);	    
		  }
		cprev = L_is_control_oper (op);
	      }
	  }
      }
  }

  /* Set up live variable for pruning */

#if L_SSA_PRUNED
  L_do_flow_analysis (fn, LIVE_VARIABLE);
#endif

  /* Build the dominator tree */

  {
    LD_dominator (fn);
  }

  /* Create ssa data structures */

  {
    L_ssa_init(fn);

    for (cb = fn->first_cb; cb; cb = cb->next_cb)
      {
	LD_ControlNode *dcb = (LD_ControlNode *) cb->ext;
	L_Oper *op;

	for (dcb = (LD_ControlNode *) cb->ext; dcb; dcb = dcb->lsucc)
	  {
	    id = dcb->dfn;

	    for (op = dcb->opS; op; op = op->next_op)
	      {
		int i;
		L_Attr *attr;
		L_Operand *opd;

		for (i = 0; i < L_max_dest_operand; i++)
		  if ((opd = op->dest[i]))
		    L_new_ssa (op, opd, id);

		for (i = 0; i < L_max_src_operand; i++)
		  if ((opd = op->src[i]))
		    opd->ssa = NULL;

		if (L_subroutine_call_opcode (op) ||
		    L_subroutine_return_opcode (op))
		  {
		    if ((attr = L_find_attr (op->attr, "ret")))
		      {
			for (i = 0; i < attr->max_field; i++)
			  if ((opd = attr->field[i]))
			    L_new_ssa (op, opd, id);
		      }

		    if ((attr = L_find_attr (op->attr, "tr")))
		      {
			for (i = 0; i < attr->max_field; i++)
			  if ((opd = attr->field[i]))
			    opd->ssa = NULL;
		      }
		  }
	      
		if (op == dcb->opE)
		  break;
	      }
	  }
      }
  }

  /* Create phi nodes */

  {
    extern HashTable LS_hash_basename;
    L_SSA_BaseName *bn;
    int cnt;

    HashTable_start (LS_hash_basename);

    while ((bn = (L_SSA_BaseName *) HashTable_next (LS_hash_basename)))
      {
	bn->phinodes = LD_df_plus (bn->pseudodefnodes);

	cnt = L_ssa_insert_phi_functions (bn);

#if L_SSA_DEBUG
	fprintf (stderr, "RMID %5d : %5d defs %5d phis\n",
		 L_REG_MAC_INDEX(bn->opd), Set_size (bn->defnodes), 
		 cnt);

	Set_print (stderr, "DEF", bn->defnodes);
	Set_print (stderr, "DF+", bn->phinodes);
#endif
      }
  }

  /* Annotate subscripts */

  {
    L_SSA_subs_recur (LDNODE(1));
  }

#if L_SSA_DEBUG
  printf ("<<< DONE\n");
#endif

  return;
  
}


int
L_exit_ssa (L_Func *fn)
{
  int new_names = 0;
  L_Cb *cb;
  L_Oper *op, *nop;

  for (cb = fn->first_cb; cb; cb = cb->next_cb)
    {
      for (op = cb->first_op; op; op = nop)
	{
	  nop = op->next_op;

	  if (L_ssa_opcode (op) || op->opc == Lop_NO_OP)
	    L_delete_oper (cb, op);
	  else if ((op->opc == Lop_DEFINE) && 
		   L_find_attr (op->attr, "SSAuninit"))
	    L_delete_oper (cb, op);
	}
    }

  L_ssa_deinit (fn);

  return new_names;
}
