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
 *	File :		l_codegen.c
 *	Description :	Determine non-loop-variant load/store pairs
 *	Creation Date :	Sept 2001
 *	Author : 	Christopher Shannon
 *
 *==========================================================================*/

#include <config.h>
#include <Lcode/l_main.h>
#include <Lcode/l_opti.h>

int
L_load_store_pair (L_Oper *oper1, L_Oper *oper2)
{
  if ((L_load_opcode (oper1) && L_store_opcode (oper2)) ||
      (L_load_opcode (oper2) && L_store_opcode (oper1)))
    {
      return (L_same_operand (oper1->src[0], oper2->src[0]) &&
	      L_same_operand (oper1->src[1], oper2->src[1]));
    }
  else
    return 0;
}

void
L_clear_inner_carried_flag (L_Oper *oper1, L_Oper *oper2)
{
  int i;
  L_Sync_Info *sync_info;

  if (!(sync_info = oper1->sync_info))
    return;

  for (i = 0; i < sync_info->num_sync_in; i++)
    {
      if (sync_info->sync_in[i]->dep_oper == oper2)
	{
	  sync_info->sync_in[i]->info &= ~SET_INNER_CARRIED (0);
	  sync_info->sync_in[i]->info |= SET_OUTER_CARRIED (0);
	}
    }

  for (i = 0; i < sync_info->num_sync_out; i++)
    {
      if (sync_info->sync_out[i]->dep_oper == oper2)
	{
#if 0
	  if (sync_info->sync_out[i]->info & SET_INNER_CARRIED (0))
	    printf ("Clearing INNER_CARRIED flag %d->%d\n", oper1->id, 
		    oper2->id);
#endif
	  
	  sync_info->sync_out[i]->info &= ~SET_INNER_CARRIED (0);
	  sync_info->sync_out[i]->info |= SET_OUTER_CARRIED (0);
	}
    }

  if (!(sync_info = oper2->sync_info))
    return;

  for (i = 0; i < sync_info->num_sync_in; i++)
    {
      if (sync_info->sync_in[i]->dep_oper == oper1)
	{
	  sync_info->sync_in[i]->info &= ~SET_INNER_CARRIED (0);
	  sync_info->sync_in[i]->info |= SET_OUTER_CARRIED (0);
	}
    }
  for (i = 0; i < sync_info->num_sync_out; i++)
    {
      if (sync_info->sync_out[i]->dep_oper == oper1)
	{
#if 0
	  if (sync_info->sync_out[i]->info & SET_INNER_CARRIED (0))
	    printf ("Clearing INNER_CARRIED flag %d->%d\n", oper2->id, 
		    oper1->id);
#endif

	  sync_info->sync_out[i]->info &= ~SET_INNER_CARRIED (0);
	  sync_info->sync_out[i]->info |= SET_OUTER_CARRIED (0);
	}
    }
}

void
L_loop_carried_dep_reduction (L_Loop *loop)
{
  /* 10/25/04 REK Commenting out unused variables to quiet compiler warnings.
   */
#if 0
  int change, j, same_cb;
  L_Sync_Info *sync_info;
#endif
  int i, num_cb, *loop_cb = NULL, num_backedge_cb, *backedge_cb =
    NULL, num_out_cb, *out_cb = NULL, increment = 0;
  L_Cb *cb, *last_cb;
  L_Oper *ind_op, *oper, *oper2, *last_use;
  L_Operand *temp, *inductor_reg, *ind_offset;

  /* setup cb array */
  num_cb = Set_size (loop->loop_cb);
  if (num_cb > 0)
    {
      loop_cb = (int *) Lcode_malloc (sizeof (int) * num_cb);
      Set_2array (loop->loop_cb, loop_cb);
    }

  /* setup backedge_cb array */
  num_backedge_cb = Set_size (loop->back_edge_cb);
  if (num_backedge_cb > 0)
    {
      backedge_cb = (int *) Lcode_malloc (sizeof (int) * num_backedge_cb);
      Set_2array (loop->back_edge_cb, backedge_cb);
    }

  /* setup out_cb array */
  num_out_cb = Set_size (loop->out_cb);
  if (num_out_cb > 0)
    {
      out_cb = (int *) Lcode_malloc (sizeof (int) * num_out_cb);
      Set_2array (loop->out_cb, out_cb);
    }

  L_find_all_ind_info (loop, loop_cb, num_cb);

  for (i = 0; i < num_cb; i++)
    {
      cb = L_cb_hash_tbl_find (L_fn->cb_hash_tbl, loop_cb[i]);

      for (ind_op = cb->first_op; ind_op != NULL; ind_op = ind_op->next_op)
        {
          /*
           *  match pattern
           */
          if (!(Set_in (loop->basic_ind_var_op, ind_op->id)))
            continue;
          if (!(L_int_add_opcode (ind_op) || L_int_sub_opcode (ind_op)))
            continue;

          last_use = L_find_last_use_of_ind_var (loop, loop_cb, num_cb,
						 backedge_cb, num_backedge_cb,
						 cb, ind_op);
          if (!(L_load_opcode (last_use) || L_store_opcode (last_use)))
	    continue;
          if (L_marked_as_post_increment (last_use) ||
              L_marked_as_pre_increment (last_use))
            continue;

          /* if increment is sub, convert it to an add */
          if (L_int_sub_opcode (ind_op) && L_is_int_constant (ind_op->src[1]))
            {
              temp = ind_op->src[1];
              ind_op->src[1] = L_new_gen_int_operand (-(temp->value.i));
              L_delete_operand (temp);
              L_change_opcode (ind_op, L_corresponding_add (ind_op));
            }

	  inductor_reg = ind_op->dest[0];

	  /* Determine the increment of the inductor */
	  if (L_same_operand (inductor_reg, ind_op->src[0]))
	    increment = ind_op->src[1]->value.i;
	  else if (L_same_operand (inductor_reg, ind_op->src[1]))
	    increment = ind_op->src[0]->value.i;
	  else
	    L_punt ("assumption violated");

	  last_cb = L_oper_hash_tbl_find_cb (L_fn->oper_hash_tbl, 
					     last_use->id);

	  for (oper = last_cb->first_op; oper && (oper != last_use); 
	       oper = oper->next_op)
	    {
	      if (!(L_load_opcode (oper) || L_store_opcode (oper)))
		continue;

	      if (L_same_operand (inductor_reg, oper->src[0]))
		ind_offset = oper->src[1];
	      else if (L_same_operand (inductor_reg, oper->src[1]))
		ind_offset = oper->src[0];
	      else
		continue;

	      if (L_is_int_constant (ind_offset))
		{
		  if (ind_offset != oper->src[1])
		    L_punt ("constant in src[0]");

		  /* Look for any corresponding load/store from same
		     inductor within increment */
		  for (oper2 = oper->next_op; oper2 && (oper2 != ind_op); 
		       oper2 = oper2->next_op)
		    {
		      if (!(L_load_opcode (oper2) || L_store_opcode (oper2)))
			continue;

		      if (!(L_same_operand (oper2->src[0], inductor_reg) &&
			    L_is_int_constant (oper2->src[1])))
			continue;

		      if ((increment > 0) && 
			  (oper2->src[1]->value.i >= 
			   (ind_offset->value.i + increment)))
			continue;

		      if ((increment < 0) && 
			  (oper2->src[1]->value.i <= 
			   (ind_offset->value.i + increment)))
			continue;

		      L_clear_inner_carried_flag (oper, oper2);
		    }
		}
	      else
		{
		  /* Look for any corresponding load/store with exact
		     same operands */
		  for (oper2 = oper->next_op; oper2 && (oper2 != ind_op);
		       oper2 = oper2->next_op)
		    {
		      if (!(L_load_opcode (oper2) || L_store_opcode (oper2)))
			continue;

		      if (!(L_same_operand (oper->src[0], oper2->src[0]) &&
			    L_same_operand (oper->src[1], oper2->src[1])))
			continue;

		      L_clear_inner_carried_flag (oper, oper2);
		    }
		}
	    }
        }
    }

  if (loop_cb != NULL)
    Lcode_free (loop_cb);
  if (backedge_cb != NULL)
    Lcode_free (backedge_cb);
  if (out_cb != NULL)
    Lcode_free (out_cb);
}

void 
Linduct_process_func (L_Func *fn)
{
  int i, num_loops, *loop_array;
  L_Loop *loop;

  /***************************************************************/
  L_do_flow_analysis (fn, DOMINATOR_CB | LIVE_VARIABLE | 
		      AVAILABLE_DEFINITION | REACHING_DEFINITION | 
		      AVAILABLE_EXPRESSION);

  L_loop_detection (fn, 1);

  L_find_basic_ind_var (fn);
  /***************************************************************/

  num_loops = 0;
  for (loop = fn->first_loop; loop != NULL; loop = loop->next_loop)
    num_loops++;

  if (num_loops == 0)
    return;

  loop_array = (int *) Lcode_malloc (sizeof (int) * num_loops);
  L_sort_loops (loop_array, num_loops);

  for (i = 0; i < num_loops; i++)
    {
      loop = L_find_loop (L_fn, loop_array[i]);

      /* 
       * REH - You'd be surprised where a boundary cb could
       *       crop up.  If it is a loop header, the loop can't
       *         be loop optimized!
       */
      if (L_EXTRACT_BIT_VAL (loop->header->flags, L_CB_BOUNDARY))
	continue;

      L_loop_carried_dep_reduction (loop);
    }
}

static int 
process_input () 
{
    switch (L_token_type) 
      {
      case L_INPUT_EOF:
      case L_INPUT_MS:
      case L_INPUT_VOID:
      case L_INPUT_BYTE:
      case L_INPUT_WORD:
      case L_INPUT_LONG:
      case L_INPUT_LONGLONG:
      case L_INPUT_FLOAT:
      case L_INPUT_DOUBLE:
      case L_INPUT_ALIGN:
      case L_INPUT_ASCII:
      case L_INPUT_ASCIZ:
      case L_INPUT_RESERVE:
      case L_INPUT_GLOBAL:
      case L_INPUT_WB:
      case L_INPUT_WW:
      case L_INPUT_WI:
      case L_INPUT_WQ:
      case L_INPUT_WF:
      case L_INPUT_WF2:
      case L_INPUT_WS:
      case L_INPUT_ELEMENT_SIZE:
      case L_INPUT_DEF_STRUCT:
      case L_INPUT_DEF_UNION:
      case L_INPUT_DEF_ENUM:
      case L_INPUT_FIELD:
      case L_INPUT_ENUMERATOR:
	L_print_data (L_OUT, L_data);
	L_delete_data (L_data);
	break;
      case L_INPUT_EVENT_LIST:
	L_print_event_list (L_OUT,L_event_list);
	L_delete_event_list (L_event_list);
	break;
      case L_INPUT_RESULT_LIST:
	L_print_result_list (L_OUT,L_result_list);
	L_delete_event_list (L_result_list);
	break;
      case L_INPUT_FUNCTION:
	L_define_fn_name (L_fn->name);
	
	Linduct_process_func (L_fn);
	
	L_print_func (L_OUT, L_fn);
	L_delete_func (L_fn);
        break;
      default:
        L_punt ("process_input: illegal token");
      }
    return 0;
}

/*
 *      Read module specific parameters
 */

void 
L_gen_code(Parm_Macro_List *command_line_macro_list)
{
  L_open_input_file (L_input_file);
  
  while (L_get_input() != L_INPUT_EOF) 
    {
      process_input();
    }
  
  L_close_input_file (L_input_file);
}
