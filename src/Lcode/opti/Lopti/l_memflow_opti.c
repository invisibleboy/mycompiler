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
/*===========================================================================
 *      File :          l_memflow_opti.c
 *      Description :   global memory flow optimization
 *      Info Needed :   mem avail defn, mem reach defn
 *      Creation Date : July, 1999
 *      Author :        Erik Nystrom.
 *
 *      (C) Copyright 1990, Erik Nystrom, Wen-mei Hwu.
 *      All rights granted to University of Illinois Board of Regents.
 *==========================================================================*/
/* 02/07/03 REK Modifying L_global_memflow_multiloadstore_load, 
 *              L_global_memflow_multistore_load to not optimize opers marked
 *              volatile. */

/* 10/29/02 REK Adding config.h */
#include <config.h>
#include "l_opti.h"


int
L_load_compatible_each_store (L_Func * fn, L_Oper * oper, Set RDEF)
{
  int i, size;
  int *buf;
  L_Oper *use_op;

  size = Set_size (RDEF);
  if (!size)
    return 0;

  buf = (int *) Lcode_malloc (sizeof (int) * size);
  Set_2array (RDEF, buf);

  for (i = 0; i < size; i++)
    {
      use_op = L_oper_hash_tbl_find_oper (fn->oper_hash_tbl, buf[i]);

      if (!L_compatible_load_store (oper, use_op))
        return 0;
    }
  Lcode_free (buf);

  return 1;
}

int
L_load_compatible_each_load (L_Func * fn, L_Oper * oper, Set RDEF)
{
  int i, size;
  int *buf;
  L_Oper *use_op;

  size = Set_size (RDEF);
  if (!size)
    return 0;

  buf = (int *) Lcode_malloc (sizeof (int) * size);
  Set_2array (RDEF, buf);

  for (i = 0; i < size; i++)
    {
      use_op = L_oper_hash_tbl_find_oper (fn->oper_hash_tbl, buf[i]);

      if (!L_same_opcode (oper, use_op))
        return 0;
    }
  Lcode_free (buf);

  return 1;
}

int
L_load_postdominates_each_store (L_Func * fn, L_Cb * cb, L_Oper * oper,
                                 Set RDEF)
{
  int i, size;
  int *buf;
  L_Cb *use_cb;

  size = Set_size (RDEF);
  if (!size)
    return 0;

  buf = (int *) Lcode_malloc (sizeof (int) * size);
  Set_2array (RDEF, buf);

  for (i = 0; i < size; i++)
    {
      use_cb = L_oper_hash_tbl_find_cb (L_fn->oper_hash_tbl, buf[i]);

      if (!(L_in_cb_PDOM_set (use_cb, cb->id)))
        return 0;
    }
  Lcode_free (buf);

  return 1;
}


int
L_cb_set_find_visit (L_Cb * start, L_Cb * stop)
{
  L_Flow *flow_indx;
  int found_stop;

  if (start == stop)
    {
      return 1;
    }
  if (L_EXTRACT_BIT_VAL (start->flags, L_CB_RESERVED_TEMP1))
    {
      return 0;
    }
  if (L_EXTRACT_BIT_VAL (start->flags, L_CB_RESERVED_TEMP2))
    {
      return 0;
    }
  start->flags = L_SET_BIT_FLAG (start->flags, L_CB_RESERVED_TEMP1);

  for (flow_indx = start->dest_flow; flow_indx;
       flow_indx = flow_indx->next_flow)
    {
      found_stop = L_cb_set_find_visit (flow_indx->dst_cb, stop);

      if (found_stop)
        return 1;
    }

  return (0);
}


int
L_cb_set_dominates (L_Func * fn, L_Cb * start, L_Cb * stop, Set cb_set)
{
  int found_epilogue;
  L_Cb *cb;

  /* Clear the visited bit flag */
  for (cb = fn->first_cb; cb != NULL; cb = cb->next_cb)
    {
      cb->flags = L_CLR_BIT_FLAG (cb->flags, L_CB_RESERVED_TEMP1);
      cb->flags = L_CLR_BIT_FLAG (cb->flags, L_CB_RESERVED_TEMP2);

      if (Set_in (cb_set, cb->id))
        cb->flags = L_SET_BIT_FLAG (cb->flags, L_CB_RESERVED_TEMP2);
    }

  found_epilogue = L_cb_set_find_visit (start, stop);

  if (found_epilogue)
    return 0;
  else
    return 1;
}


int
L_store_union_dominates_load (L_Func * fn, L_Cb * cb, L_Oper * oper, Set RDEF)
{
  int i, size;
  int *buf;
  L_Cb *def_cb, *start;
  Set cbset;
  int val;

  cbset = NULL;
  /* start needs to dominate all nodes in function */
  start = fn->first_cb;

  /* Convert RDEF is a set of cbs */
  size = Set_size (RDEF);
  if (!size)
    return 0;

  buf = (int *) Lcode_malloc (sizeof (int) * size);
  Set_2array (RDEF, buf);
  for (i = 0; i < size; i++)
    {
      def_cb = L_oper_hash_tbl_find_cb (L_fn->oper_hash_tbl, buf[i]);

      cbset = Set_add (cbset, def_cb->id);
    }
  Lcode_free (buf);

  /* Does cbset dominate cb from start */
  val = L_cb_set_dominates (fn, start, cb, cbset);
  cbset = Set_dispose (cbset);

  return val;
}


int
L_loadstore_union_postdominates_amb (L_Func * fn, L_Cb * cb, L_Oper * oper,
                                     Set LDST, Set AMB)
{
  int i, size;
  int *buf;
  L_Cb *def_cb;
  Set pd_cbset;
  int val = 0;

  pd_cbset = NULL;


  /* Convert RDEF is a set of cbs */
  size = Set_size (LDST);
  if (!size)
    return 0;

  buf = (int *) Lcode_malloc (sizeof (int) * size);
  Set_2array (LDST, buf);
  for (i = 0; i < size; i++)
    {
      def_cb = L_oper_hash_tbl_find_cb (L_fn->oper_hash_tbl, buf[i]);

      pd_cbset = Set_add (pd_cbset, def_cb->id);
    }
  Lcode_free (buf);


  /* Foreach cb in AMB, Does pd_cbset post_dominate it */
  size = Set_size (AMB);
  if (!size)
    return 1;

  buf = (int *) Lcode_malloc (sizeof (int) * size);
  Set_2array (AMB, buf);
  for (i = 0; i < size; i++)
    {
      def_cb = L_oper_hash_tbl_find_cb (L_fn->oper_hash_tbl, buf[i]);

      val = L_cb_set_dominates (fn, def_cb, cb, pd_cbset);
      if (!val)
        break;
    }

  Lcode_free (buf);
  pd_cbset = Set_dispose (pd_cbset);

  return val;
}


int
L_global_memflow_multistore_load (L_Func * fn, L_Cb * cb, L_Oper * oper)
{
  int change = 0;
  Set RAMB, RDEF;
  L_Oper *use_op, *new_op;
  L_Cb *use_cb;
  L_Operand *src, *dest;
  int i, size;
  int *buf;


  if (!L_general_load_opcode (oper))
    return 0;

  /* 02/07/03 REK Adding a check to make sure we don't touch a volatile
   *              oper. */
  if (L_EXTRACT_BIT_VAL (oper->flags, L_OPER_VOLATILE))
    return 0;

  RAMB = NULL;
  RDEF = NULL;

  /*fprintf(stderr,"MFMSL: Examining load %d\n",oper->id); */

  RDEF = L_get_mem_oper_RIN_set_rid (oper);

  /* Are there any reaching, ambiguous stores */
  RAMB =
    L_get_mem_oper_RIN_defining_opers (oper, (MDF_RET_AMB | MDF_RET_STORES));
  if (Set_size (RAMB))
    {
      Set_dispose (RAMB);
      return change;
    }

  /* Are there any reaching, unsafe jsrs */
  RAMB =
    L_get_mem_oper_RIN_defining_opers (oper, (MDF_RET_AMB | MDF_RET_JSRS));
  if (Set_size (RAMB))
    {
      Set_dispose (RAMB);
      return change;
    }

  /* Get all reaching stores */
  RDEF =
    L_get_mem_oper_RIN_defining_opers (oper, (MDF_RET_DEP | MDF_RET_STORES));
  if (!Set_size (RDEF))
    {
      return change;
    }

  /* fprintf(stderr,"MFMSL: Dep Stores reach load %d\n",oper->id);
     Set_print( stderr, "MSMSL: ", RDEF); */

  if (!L_load_compatible_each_store (fn, oper, RDEF))
    {
      /* fprintf(stderr,"MFMSL: Load is incompatible with store\n"); */
      return change;
    }

  if (!L_store_union_dominates_load (fn, cb, oper, RDEF))
    {
      /* fprintf(stderr,"MFMSL: Path to load exists not through a store\n"); */
      return change;
    }


  /* Load can be removed, moves added */
  if (Lopti_debug_memflow)
    fprintf (stderr, "MFMSL: red Load %d can be converted\n", oper->id);

  dest = L_new_register_operand (++L_fn->max_reg_id,
                                 L_return_old_ctype (oper->dest[0]),
                                 L_PTYPE_NULL);
  /* convert load to a move */
  change = 1;
  L_convert_to_extended_move (oper, L_copy_operand (oper->dest[0]), dest);

  /* add move to all stores */
  size = Set_size (RDEF);
  buf = (int *) Lcode_malloc (sizeof (int) * size);
  Set_2array (RDEF, buf);
  for (i = 0; i < size; i++)
    {
      use_op = L_oper_hash_tbl_find_oper (fn->oper_hash_tbl, buf[i]);
      use_cb = L_oper_hash_tbl_find_cb (L_fn->oper_hash_tbl, buf[i]);

      if (Lopti_debug_memflow)
        fprintf (stderr, "MFMSL: Converting store %d cb %d\n", use_op->id,
                 use_cb->id);

      /* change store to read from a new register */
      src = use_op->src[2];
      use_op->src[2] = L_copy_operand (dest);

      /* add new move before store */
      new_op = L_create_move_using (L_copy_operand (dest), src, use_op);
      L_insert_oper_before (use_cb, use_op, new_op);
    }
  Lcode_free (buf);

  return change;
}


int
L_global_memflow_multiloadstore_load (L_Func * fn, L_Cb * cb, L_Oper * oper)
{
  return 0;

#if 0
  int change = 0;
  Set RAMB_ST, RAMB_JSR;
  Set RDEF_ST, RDEF_LD;

  if (!L_general_load_opcode (oper))
    return 0;

  /* 02/07/03 REK Adding a check to make sure we don't touch a volatile
   *              oper. */
  if (L_EXTRACT_BIT_VAL (oper->flags, L_OPER_VOLATILE))
    return 0;

  RDEF_ST = NULL;
  RDEF_LD = NULL;
  RAMB_ST = NULL;
  RAMB_JSR = NULL;

  /* Get all reaching stores */
  RDEF_ST =
    L_get_mem_oper_RIN_defining_opers (oper, (MDF_RET_DEP | MDF_RET_STORES));

  /* Get all reaching loads */
  RDEF_LD =
    L_get_mem_oper_RIN_defining_opers (oper, (MDF_RET_DEP | MDF_RET_LOADS));

  if (!Set_size (RDEF_LD) || !Set_size (RDEF_ST))
    return change;


  fprintf (stderr, "\n\nMFMLSL: Examining load %d\n", oper->id);

  /* Are there any reaching, ambiguous stores */
  RAMB_ST =
    L_get_mem_oper_RIN_defining_opers (oper, (MDF_RET_AMB | MDF_RET_STORES));

  /* Are there any reaching, unsafe jsrs */
  RAMB_JSR =
    L_get_mem_oper_RIN_defining_opers (oper, (MDF_RET_AMB | MDF_RET_JSRS));

  fprintf (stderr, "MFMLSL: Dep Stores reach load %d ", oper->id);
  Set_print (stderr, "MSMLSL: ", RDEF_ST);
  fprintf (stderr, "MFMLSL: Dep Loads reach load %d ", oper->id);
  Set_print (stderr, "MSMLSL: ", RDEF_LD);
  fprintf (stderr, "MFMLSL: Amb Stores reach load %d ", oper->id);
  Set_print (stderr, "MSMLSL: ", RAMB_ST);
  fprintf (stderr, "MFMLSL: Unsafe jsrs reach load %d ", oper->id);
  Set_print (stderr, "MSMLSL: ", RAMB_JSR);

  if (!L_load_compatible_each_store (fn, oper, RDEF_ST))
    {
      fprintf (stderr, "MFMLSL: Load is incompatible with a store\n");
      Set_dispose (RDEF);
      Set_dispose (RDEF_LD);
      Set_dispose (RAMB_ST);
      Set_dispose (RAMB_JSR);
      return change;
    }

  if (!L_load_compatible_each_load (fn, oper, RDEF_LD))
    {
      fprintf (stderr, "MFMLSL: Load is incompatible with a store\n");
      Set_dispose (RDEF);
      Set_dispose (RDEF_LD);
      Set_dispose (RAMB_ST);
      Set_dispose (RAMB_JSR);
      return change;
    }

  if (!L_store_union_dominates_load
      (fn, cb, oper, Set_union (RDEF_LD, RDEF_ST)))
    {
      fprintf (stderr,
               "MFMLSL: Path to load exists not through a store/load\n");
      Set_dispose (RDEF);
      Set_dispose (RDEF_LD);
      Set_dispose (RAMB_ST);
      Set_dispose (RAMB_JSR);
      return change;
    }

  if (!L_loadstore_union_postdominates_amb (fn, cb, oper,
                                            Set_union (RDEF_LD, RDEF_ST),
                                            Set_union (RAMB_ST, RAMB_JSR)))
    {
      fprintf (stderr, "MFMLSL: A amb st/jsr lies between ld/st and load \n");
      Set_dispose (RDEF);
      Set_dispose (RDEF_LD);
      Set_dispose (RAMB_ST);
      Set_dispose (RAMB_JSR);
      return change;
    }


  /* Load can be removed, moves added */
  fprintf (stderr, "MFMLSL: red Load %d can be converted\n", oper->id);

  Set_dispose (RDEF);
  Set_dispose (RDEF_LD);
  Set_dispose (RAMB_ST);
  Set_dispose (RAMB_JSR);
  return change;
#endif
}


int
L_global_memflow_optimization (L_Func * fn)
{
  int opti_applied;
  L_Cb *cb;
  L_Oper *op;
  int c1;
  int load_op, store_op, jsr_op;

  if (Lopti_do_memflow_opti == 0)
    return (0);

  /* Don't do these optis if the number of loads/stores/jsrs
     is excessive since it is a memory hog */
  load_op = 0;
  store_op = 0;
  jsr_op = 0;
  for (cb = fn->first_cb; cb != NULL; cb = cb->next_cb)
    {
      for (op = cb->first_op; op != NULL; op = op->next_op)
        {
          if (L_general_load_opcode (op))
            load_op++;
          if (L_general_store_opcode (op))
            store_op++;
          if (L_general_subroutine_call_opcode (op))
            jsr_op++;
        }
    }

  if (load_op > Lopti_memflow_bypass_load)
    return (0);
  if (store_op > Lopti_memflow_bypass_store)
    return (0);
  if (jsr_op > Lopti_memflow_bypass_jsr)
    return (0);
  if ((load_op + store_op + jsr_op) > Lopti_memflow_bypass_total)
    return (0);

  if (Lopti_debug_memflow)
    fprintf (stderr, "Doing memflow\n");

  L_do_flow_analysis (fn, MEM_REACHING_DEFINITION);

  opti_applied = 0;
  c1 = 0;

  /* Call Various Opti's on each oper */
  for (cb = fn->first_cb; cb != NULL; cb = cb->next_cb)
    {
      if (L_EXTRACT_BIT_VAL (cb->flags, L_CB_BOUNDARY))
        continue;

      for (op = cb->first_op; op != NULL; op = op->next_op)
        {
          /* Opti routines */

          if (Lopti_do_memflow_multistore_load)
            {
              c1 += L_global_memflow_multistore_load (fn, cb, op);
            }

          L_global_memflow_multiloadstore_load (fn, cb, op);

        }
    }


  if (Lopti_do_memflow_multistore_load)
    {
      Lopti_cnt_memflow_multistore_load = c1;
      STAT_COUNT ("L_memflow_multistore_load", c1, NULL);
    }

  if (c1)
    opti_applied = 1;

  return (opti_applied);
}
