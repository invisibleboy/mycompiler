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
 *      File :          l_addrpred.c
 *      Description :   Address prediction
 *      Creation Date : Sept 1998
 *      Author :        Ben-Chung Cheng, Wen-mei Hwu
 *
 *==========================================================================*/

/* 10/29/02 REK Adding config.h */
#include <config.h>
#include <Lcode/l_main.h>

/*****************************************************************************\
 * BCC - algorithm outline - 9/9/98
 * (1) Loops are processed from inner out
 * (2) If a load has been marked, skip it. This load must be contained in an
 *     inner loop, or be the load for the base pointer in a pointer-chasing 
 *     loop
 * (3) If a load is found to use the same base and dest register, remember the
 *     register id. Also, insert all dest reg id's into the loaded_reg_id set.
 * (4) Now process all instructions to grow the loaded_reg_id set.
 * (5) If no pointer-chasing load is found, find the reg id which is used most
 *     as the base register.
 * (6) Scan the code again, find all loads that use the base reg id and mark
 *     those as L_SETUP_IMPLIED_REG. For those loads whose source reg id is not
 *     in the loaded_reg_id set, mark those as L_PREDICT. For the rest loads,
 *     mark those as L_DONT_PREDICT.
\*****************************************************************************/

int
L_loop_check_load_for_linked_list_base (L_Loop * loop, int *pred, int *early,
                                        int *dp)
{
  int change, i, num_cb, *loop_cb = NULL, num_out_cb, *out_cb = NULL, temp;
  int base_reg_id = -2, largest_value = 1;
  Lint loaded_reg_id = 0;
  L_Cb *cb, *header;
  L_Oper *op;
  L_Attr *attr;
  INT_Symbol_Table *int_table;
  INT_Symbol *int_symbol;

  /* setup cb array */
  num_cb = Set_size (loop->loop_cb);
  if (num_cb > 0)
    {
      loop_cb = (int *) Lcode_malloc (sizeof (int) * num_cb);
      Set_2array (loop->loop_cb, loop_cb);
    }

  /* setup out_cb array */
  num_out_cb = Set_size (loop->out_cb);
  if (num_out_cb > 0)
    {
      out_cb = (int *) Lcode_malloc (sizeof (int) * num_out_cb);
      Set_2array (loop->out_cb, out_cb);
    }

  L_find_all_ind_info (loop, loop_cb, num_cb);

  header = loop->header;
  for (i = 0; i < num_cb; i++)
    {
      cb = L_cb_hash_tbl_find (L_fn->cb_hash_tbl, loop_cb[i]);
      for (op = cb->first_op; op != NULL; op = op->next_op)
        {
          if (!L_load_opcode (op))
            continue;
          if (L_is_reg (op->dest[0]))
            {
              if (!InLint (loaded_reg_id, op->dest[0]->value.r))
                loaded_reg_id = AppendLint (NewLint (op->dest[0]->value.r),
                                            loaded_reg_id);
            }

          if (L_find_attr (op->attr, L_SETUP_IMPLIED_REG) ||
              L_find_attr (op->attr, L_DONT_PREDICT) ||
              L_find_attr (op->attr, L_PREDICT))
            continue;

          if ((base_reg_id == -2) && L_is_reg (op->dest[0]) &&
              L_same_operand (op->dest[0], op->src[0]))
            {
              base_reg_id = op->src[0]->value.r;
            }
        }
    }

  change = 1;

  /* BCC - 9/7/98
   * Initially, loaded_reg_id should only contain the register ids that are
   * destinations of load instructions. Here more register ids are added to
   * loaded_reg_id set if one of the instruction's source operands are loaded
   * from memory.
   */
  while (change)
    {
      change = 0;
      for (i = 0; i < num_cb; i++)
        {
          cb = L_cb_hash_tbl_find (L_fn->cb_hash_tbl, loop_cb[i]);
          for (op = cb->first_op; op != NULL; op = op->next_op)
            {
              if (L_store_opcode (op))
                continue;
              if (!L_is_reg (op->dest[0]))
                continue;
              if (L_is_reg (op->src[0]))
                {
                  if (InLint (loaded_reg_id, op->src[0]->value.r) &&
                      !InLint (loaded_reg_id, op->dest[0]->value.r))
                    {
                      loaded_reg_id =
                        AppendLint (NewLint (op->dest[0]->value.r),
                                    loaded_reg_id);
                      change = 1;
                    }
                }
              if (L_is_reg (op->src[1]))
                {
                  if (InLint (loaded_reg_id, op->src[1]->value.r) &&
                      !InLint (loaded_reg_id, op->dest[0]->value.r))
                    {
                      loaded_reg_id =
                        AppendLint (NewLint (op->dest[0]->value.r),
                                    loaded_reg_id);
                      change = 1;
                    }
                }
            }
        }
    }

  /* Now, loaded_reg_id should contain all register ids whose contents are
   * loaded from memory in each iteration. If base_red_id is not set, search
   * for the largest group of load-dependent loads that use the same base
   * register. Then mark these loads as L_SETUP_IMPLIED_REG.
   */
  if (base_reg_id == -2)
    {
      int_table = INT_new_symbol_table ("src_reg_table", 32);
      for (i = 0; i < num_cb; i++)
        {
          cb = L_cb_hash_tbl_find (L_fn->cb_hash_tbl, loop_cb[i]);
          for (op = cb->first_op; op != NULL; op = op->next_op)
            {
              if (!L_load_opcode (op))
                continue;
              if (L_find_attr (op->attr, L_SETUP_IMPLIED_REG) ||
                  L_find_attr (op->attr, L_DONT_PREDICT) ||
                  L_find_attr (op->attr, L_PREDICT))
                continue;
              if (L_is_reg (op->src[0]) && L_is_int_constant (op->src[1]) &&
                  InLint (loaded_reg_id, op->src[0]->value.r))
                {
                  int_symbol =
                    INT_find_symbol (int_table, op->src[0]->value.r);
                  if (int_symbol)
                    {
#ifdef LP64_ARCHITECTURE
		      temp = (int)((long)(int_symbol->data));
#else
                      temp = (int) int_symbol->data;
#endif
                      temp++;
#ifdef LP64_ARCHITECTURE
		      int_symbol->data = (void *)((long)temp);
#else
                      int_symbol->data = (void *) temp;
#endif
                    }
                  else
                    INT_add_symbol (int_table, op->src[0]->value.r,
                                    (void *) 1);
                }
            }
        }

      for (int_symbol = int_table->head_symbol;
           int_symbol; int_symbol = int_symbol->next_symbol)
        {
#ifdef LP64_ARCHITECTURE
	  if ((int)((long)(int_symbol->data)) > largest_value)
#else
          if ((int) int_symbol->data > largest_value)
#endif
            {
#ifdef LP64_ARCHITECTURE
	      largest_value = (int)((long)(int_symbol->data));
#else
              largest_value = (int) int_symbol->data;
#endif
              base_reg_id = int_symbol->value;
            }
        }
      INT_delete_symbol_table (int_table, 0);
    }

  for (i = 0; i < num_cb; i++)
    {
      cb = L_cb_hash_tbl_find (L_fn->cb_hash_tbl, loop_cb[i]);
      for (op = cb->first_op; op != NULL; op = op->next_op)
        {
          if (!L_load_opcode (op))
            continue;
          if (L_find_attr (op->attr, L_SETUP_IMPLIED_REG) ||
              L_find_attr (op->attr, L_DONT_PREDICT) ||
              L_find_attr (op->attr, L_PREDICT))
            continue;
          if (L_is_reg (op->src[0]) && L_is_int_constant (op->src[1]))
            {
              if (op->src[0]->value.r == base_reg_id)
                {
                  attr = L_new_attr (L_SETUP_IMPLIED_REG, 0);
                  op->attr = L_concat_attr (op->attr, attr);
                  (*early)++;
                }
              else if (InLint (loaded_reg_id, op->src[0]->value.r))
                {
                  attr = L_new_attr (L_DONT_PREDICT, 0);
                  op->attr = L_concat_attr (op->attr, attr);
                  (*dp)++;
                }
              else
                {
                  attr = L_new_attr (L_PREDICT, 0);
                  op->attr = L_concat_attr (op->attr, attr);
                  (*pred)++;
                }
            }
          else if ((L_is_reg (op->src[0]) &&
                    InLint (loaded_reg_id, op->src[0]->value.r)) ||
                   (L_is_reg (op->src[1]) &&
                    InLint (loaded_reg_id, op->src[1]->value.r)))
            {
              attr = L_new_attr (L_DONT_PREDICT, 0);
              op->attr = L_concat_attr (op->attr, attr);
              (*dp)++;
            }
          else
            {
              attr = L_new_attr (L_PREDICT, 0);
              op->attr = L_concat_attr (op->attr, attr);
              (*pred)++;
            }
        }
    }

  if (loop_cb != NULL)
    Lcode_free (loop_cb);
  if (out_cb != NULL)
    Lcode_free (out_cb);
  if (loaded_reg_id)
    FreeLint (loaded_reg_id);

  return change;
}

/* BCC - micro31 address prediction flags */

void
L_sort_loops (int *loop_array, int num_loops)
{
  int i;
  Set loop_set;
  L_Loop *loop;

  loop_set = NULL;

  for (i = 0; i < num_loops; i++)
    {
      for (loop = L_fn->first_loop; loop != NULL; loop = loop->next_loop)
        {
          if (Set_in (loop_set, loop->id))
            continue;
          if (Set_subtract_empty (loop->nested_loops, loop_set))
            {
              loop_set = Set_add (loop_set, loop->id);
              loop_array[i] = loop->id;
              break;
            }
        }
      if (!loop_array[i])
        L_punt ("L_sort_loops: Loops are not properly nested");
    }

  Set_dispose (loop_set);
}


void
L_find_linked_list_base (L_Func * fn, int *pred, int *early, int *dp)
{
  int i, num_loops, *loop_array, flag;
  L_Loop *loop;

  num_loops = 0;
  for (loop = fn->first_loop; loop != NULL; loop = loop->next_loop)
    {
      num_loops++;
    }

  if (num_loops == 0)
    return;

  loop_array = (int *) Lcode_malloc (sizeof (int) * num_loops);
  L_sort_loops (loop_array, num_loops);

  flag = 0;
  while (!flag)
    {

      flag = 1;
      /* BCC - process inner loops first - 9/9/98 */
      for (i = 0; i < num_loops; i++)
        {

          loop = L_find_loop (L_fn, loop_array[i]);

          L_loop_check_load_for_linked_list_base (loop, pred, early, dp);
        }
    }

  /* clean up after last loop opti */
  Lcode_free (loop_array);
  return;
}


void
L_process_acylic_code (L_Func * fn, int *pred, int *early, int *dp)
{
  L_Cb *cb;
  L_Oper *op;
  L_Attr *attr;
  INT_Symbol_Table *int_table;
  INT_Symbol *int_symbol;
  int base_reg_id = 0, count;

  int_table = INT_new_symbol_table ("src_reg_table", 32);
  for (cb = fn->first_cb; cb; cb = cb->next_cb)
    {
      for (op = cb->first_op; op; op = op->next_op)
        {
          if (!L_load_opcode (op))
            continue;
          if (L_find_attr (op->attr, L_SETUP_IMPLIED_REG) ||
              L_find_attr (op->attr, L_DONT_PREDICT) ||
              L_find_attr (op->attr, L_PREDICT))
            continue;
          if (L_is_int_constant (op->src[1]) &&
              (L_is_reg (op->src[0]) || L_is_macro (op->src[0])))
            {
              if (L_is_reg (op->src[0]))
                {
                  base_reg_id = op->src[0]->value.r;
                }
              else if (L_is_macro (op->src[0]))
                {
                  base_reg_id = -1;
                }
              int_symbol = INT_find_symbol (int_table, base_reg_id);
              if (int_symbol)
                {
#ifdef LP64_ARCHITECTURE
		  count = (int)((long)(int_symbol->data));
#else
                  count = (int) int_symbol->data;
#endif
                  count++;
#ifdef LP64_ARCHITECTURE
		  int_symbol->data = (void *)((long)count);
#else
                  int_symbol->data = (void *) count;
#endif
                }
              else
                {
                  INT_add_symbol (int_table, base_reg_id, (void *) 1);
                }
            }
        }
    }
  for (cb = fn->first_cb; cb; cb = cb->next_cb)
    {
      for (op = cb->first_op; op; op = op->next_op)
        {
          if (!L_load_opcode (op))
            continue;
          if (L_find_attr (op->attr, L_SETUP_IMPLIED_REG) ||
              L_find_attr (op->attr, L_DONT_PREDICT) ||
              L_find_attr (op->attr, L_PREDICT))
            continue;
          if (L_find_attr (op->attr, "label"))
            {
              attr = L_new_attr (L_PREDICT, 0);
              op->attr = L_concat_attr (op->attr, attr);
              (*pred)++;
            }
          else if (L_is_macro (op->src[0]))
            {
              attr = L_new_attr (L_SETUP_IMPLIED_REG, 0);
              op->attr = L_concat_attr (op->attr, attr);
              (*early)++;
            }
          else
            {
              if (L_is_int_constant (op->src[1]))
                {
                  base_reg_id = op->src[0]->value.r;
                  int_symbol = INT_find_symbol (int_table, base_reg_id);
                  if (int_symbol)
                    {
#ifdef LP64_ARCHITECTURE
		      count = (int)((long)(int_symbol->data));
#else
                      count = (int) int_symbol->data;
#endif
                      if (count > 1)
                        {
                          attr = L_new_attr (L_SETUP_IMPLIED_REG, 0);
                          op->attr = L_concat_attr (op->attr, attr);
                          (*early)++;
                        }
                      else
                        {
                          attr = L_new_attr (L_DONT_PREDICT, 0);
                          op->attr = L_concat_attr (op->attr, attr);
                          (*dp)++;
                        }
                    }
                  else
                    {
                      attr = L_new_attr (L_DONT_PREDICT, 0);
                      op->attr = L_concat_attr (op->attr, attr);
                      (*dp)++;
                    }
                }
              else
                {
                  attr = L_new_attr (L_DONT_PREDICT, 0);
                  op->attr = L_concat_attr (op->attr, attr);
                  (*dp)++;
                }
            }
        }
    }
  INT_delete_symbol_table (int_table, 0);
  return;
}


void
L_generate_address_prediction_flags (L_Func * fn)
{
  L_Attr *attr;

  int pred = 0, early = 0, dp = 0;

  if (L_find_attr (fn->attr, "ADDR_PRED"))
    return;
  attr = L_new_attr ("ADDR_PRED", 1);
  fn->attr = L_concat_attr (fn->attr, attr);
  if (fn->first_loop == 0)
    {
      L_do_flow_analysis (fn, DOMINATOR_CB);
      L_loop_detection (fn, 1);
    }
  L_find_linked_list_base (fn, &pred, &early, &dp);
#if 0
  L_process_acylic_code (fn, &pred, &early, &dp);
  fprintf (stderr, "%s\n", fn->name);
  fprintf (stderr, "\tpred= %3d  early= %3d  dp= %3d  %f\n",
           pred, early, dp, ((float) (pred + early)) / (pred + early + dp));
#endif
  return;
}
