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
/**************************************************************************\
 *
 *  File: phase1_param.c
 *
 *  Description: input parameter subst, return value 
 *               Moved from Mopti-ia64
 *
 *  Authors:  Jim Pierce, Dan Connors
 *
 *
\************************************************************************/

/* 10/29/02 REK Adding config.h */
#include <config.h>
#include "ltahoe_main.h"
#include "ltahoe_op_query.h"

#undef IPARM_DEBUG
#define MAX_INPUT_PARAMS 8

typedef struct ip_mac_subst_info
{
  L_Operand *mac_operand;
  L_Operand *reg_operand;
  int reg_value;
}
IP_MAC_SUBST_INFO;


static void
check_input_params (L_Func * fn, IP_MAC_SUBST_INFO * ip_array, int ip_moves)
{
  L_Cb *cb;
  L_Oper *oper;
  int i, use_count[MAX_INPUT_PARAMS];

  for (i = 0; i < MAX_INPUT_PARAMS; i++)
    use_count[i] = 0;

  for (cb = fn->first_cb; cb; cb = cb->next_cb)
    {
      for (oper = cb->first_op; oper; oper = oper->next_op)
	{
	  for (i = 0; i < ip_moves; i++)
	    if (L_is_src_operand (ip_array[i].mac_operand, oper))
	      use_count[i]++;
	}
    }

  for (i = 0; i < ip_moves; i++)
    if (use_count[i] > 1)
      ip_array[i].reg_value = -1;

  return;
}


static int
find_input_parameter_moves (L_Func * fn, IP_MAC_SUBST_INFO * ip_array)
{
  L_Cb *cb;
  L_Oper *oper;
  int ip_moves = 0;

  cb = fn->first_cb;
  for (oper = cb->first_op; oper; oper = oper->next_op)
    {

      if (oper->opc == Lop_MOV && L_is_macro (oper->src[0]) &&
	  LT_is_input_param_operand (oper->src[0]) &&
	  L_is_register (oper->dest[0]))
	{
#ifdef IPARM_DEBUG
	  fprintf (stderr, "Found parameter move: %d - %d\n",
		   oper->id, oper->dest[0]->value.r);
#endif
	  ip_array[ip_moves].mac_operand = L_copy_operand (oper->src[0]);
	  ip_array[ip_moves].reg_operand = L_copy_operand (oper->dest[0]);
	  ip_array[ip_moves].reg_value = oper->dest[0]->value.r;
	  ip_moves++;
	}
    }
  check_input_params (fn, ip_array, ip_moves);
  return (ip_moves);
}


static void
free_ip_mov_array (IP_MAC_SUBST_INFO * ip_array, int ip_moves)
{
  int i;

  for (i = 0; i < ip_moves; i++)
    {
      L_delete_operand (ip_array[i].mac_operand);
      L_delete_operand (ip_array[i].reg_operand);
    }
}


static L_Operand *
operand_match_any_ip_moves (IP_MAC_SUBST_INFO * ip_array,
			    int ip_moves, L_Operand * match_operand)
{
  int i;

  for (i = 0; i < ip_moves; i++)
    {
      if (ip_array[i].reg_value == match_operand->value.r)
	return ip_array[i].mac_operand;
    }
  return NULL;
}


static void
subst_input_macros (L_Func * fn, IP_MAC_SUBST_INFO * ip_array, int ip_moves)
{
  L_Cb *cb;
  L_Oper *oper;
  L_Operand *new_operand;
  int i;

  for (cb = fn->first_cb; cb; cb = cb->next_cb)
    {
      for (oper = cb->first_op; oper; oper = oper->next_op)
	{
	  /* Check destination */
	  if (L_is_register (oper->dest[0]) &&
	      (new_operand = operand_match_any_ip_moves (ip_array,
							 ip_moves,
							 oper->dest[0])))
	    {
	      L_delete_operand (oper->dest[0]);
	      oper->dest[0] = L_copy_operand (new_operand);
	    }

	  /* Check sources */
	  for (i = 0; oper->src[i] && (i < L_max_src_operand); i++)
	    {
	      if (L_is_register (oper->src[i]) &&
		  (new_operand = operand_match_any_ip_moves (ip_array,
							     ip_moves,
							     oper->src[i])))
		{
		  L_delete_operand (oper->src[i]);
		  oper->src[i] = L_copy_operand (new_operand);
		}
	    }
	}
    }
  return;
}


/* JEP 5/96 - It is assumed that if rn = Pm is found in the prologue
   block, Pm will never be found elsewhere in the function.  */

/* JEP 7/96 - This is an incorrect assumption.  Currently, a check
   is made, no subst is done if Pm is found as a source */

void
Ltahoe_ip_subst (L_Func * fn)
{
  IP_MAC_SUBST_INFO ip_array[MAX_INPUT_PARAMS];
  int ip_moves;

  ip_moves = find_input_parameter_moves (fn, ip_array);

  if (ip_moves)
    {
      subst_input_macros (fn, ip_array, ip_moves);
      free_ip_mov_array (ip_array, ip_moves);
    }
}
