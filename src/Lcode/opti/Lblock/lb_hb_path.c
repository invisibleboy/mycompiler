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
 *      File :          l_path.c
 *      Description :   Information for each path thru a region
 *      Creation Date : February 1994
 *      Authors :       Scott Mahlke
 *       Included with Lblock in its original form from Lhyper -- KMC 4/98 
 *        with minor changes to create traceregions instead of Lhyper regions
 *==========================================================================*/
/* 10/29/02 REK Adding config.h */
#include <config.h>
#include "lb_hb_hyperblock.h"

#undef DEBUG_PATH
#undef DEBUG_DEPENDENCE
#undef DEBUG_PATH_INFO
#undef DEBUG_SELECT
#undef DEBUG_SELECT_PENALTY

#define LB_MAX_PATHS     65536
#define LB_MAX_Z_PATHS   32768

#define ERR	stderr

#define DEBUG_PATH

typedef struct _LB_hb_Path {
  double exec_ratio;
  double priority;
  int id;			/* unique path id */
  int selected;			/* path included in hyperblock */
  int num_blocks;		/* number of blocks in path */
  int *blocks;			/* array of cb id's */
  Set block_set;
  int num_ops;
  int flags;
  int dep_height;
} LB_hb_Path;

static L_Alloc_Pool *LB_hb_path_pool = NULL;
static List LB_hb_all_paths = NULL;
static Set LB_hb_path_all_blocks = NULL;
static Stack *LB_path_stack;
static Set *B;
static L_Cb *LB_path_cb;

static int LB_hb_path_max_id = 0;
static int LB_hb_path_max_dep_height = 0;
static int LB_hb_path_total_ops = 0;
static int LB_hb_path_total_paths = 0;
static int LB_hb_path_total_zpaths = 0;
int LB_hb_path_max_path_exceeded = 0;

static LB_hb_Path *LB_hb_new_path (void);
static void LB_hb_delete_path (LB_hb_Path *);
static void LB_hb_delete_all_path (void);
void LB_hb_print_path (FILE *, LB_hb_Path *);
void LB_hb_print_path_list (FILE *, LB_hb_Path *);


int
LB_hb_select_all_blocks (L_Cb * cb)
{
  return L_find_attr (cb->attr, HB_SELECT_ALL_ATTR) ? 1 : 0;
}


int
LB_hb_select_exact_blocks (L_Cb * cb)
{
  return L_find_attr (cb->attr, HB_SELECT_EXACT_ATTR) ? 1 : 0;
}


int
LB_hb_ignore_block (L_Cb * cb)
{
  return L_find_attr (cb->attr, HB_IGNORE_ATTR) ? 1 : 0;
}


Set
LB_hb_find_exact_blocks (L_Cb * cb)
{
  int i;
  L_Attr *attr;
  Set blocks = NULL;

  if (!(attr = L_find_attr (cb->attr, HB_SELECT_EXACT_ATTR)))
    L_punt ("L_find_exact_blocks: no attr found");

  for (i = 0; i < attr->max_field; i++)
    if (L_is_int_constant (attr->field[i]))
      blocks = Set_add (blocks, attr->field[i]->value.i);

  return (blocks);
}

/*=========================================================================*/
/*
 *    LB_hb_Path creation/deletion routines
 */
/*=========================================================================*/

static LB_hb_Path *
LB_hb_new_path (void)
{
  LB_hb_Path *path;

  if (!LB_hb_path_pool)
    LB_hb_path_pool = L_create_alloc_pool ("LB_hb_Path",
					   sizeof (LB_hb_Path), 16);

  path = (LB_hb_Path *) L_alloc (LB_hb_path_pool);
  path->id = ++LB_hb_path_max_id;
  path->selected = 0;
  path->num_blocks = 0;
  path->blocks = NULL;
  path->block_set = NULL;
  path->num_ops = 0;
  path->flags = 0;
  path->exec_ratio = 0.0;
  path->priority = 0.0;
  path->dep_height = 0;

  return (path);
}


static void
LB_hb_delete_path (LB_hb_Path * path)
{
  if (!path)
    return;

  if (path->blocks)
    {
      Lcode_free (path->blocks);
      path->blocks = NULL;
    }

  path->block_set = Set_dispose (path->block_set);

  L_free (LB_hb_path_pool, path);

  return;
}


static void
LB_hb_delete_all_path (void)
{
  LB_hb_Path *ptr;

  List_start (LB_hb_all_paths);
  while ((ptr =( LB_hb_Path *) List_next (LB_hb_all_paths)))
    LB_hb_delete_path (ptr);

  LB_hb_all_paths = List_reset (LB_hb_all_paths);
  return;
}


void
LB_hb_print_path (FILE * F, LB_hb_Path * path)
{
  int i;

  fprintf (F, "Path: id = %d\tselected = %d\n", path->id, path->selected);
  fprintf (F, "\tnum_blocks = %d\n", path->num_blocks);
  fprintf (F, "\tblocks: ");
  for (i = 0; i < path->num_blocks; i++)
    fprintf (F, "%d ", path->blocks[i]);

  fprintf (F, "\n");
  fprintf (F, "\tnum_ops = %d\n", path->num_ops);
  fprintf (F, "\tdep_height = %d\n", path->dep_height);
  fprintf (F, "\texec_ratio = %7.6f\n", path->exec_ratio);
  fprintf (F, "\tpriority = %7.6f\n", path->priority);
  fprintf (F, "\tflags: ");
  if (path->flags == 0)
    {
      fprintf (F, "NONE ");
    }
  else
    {
      if (L_EXTRACT_BIT_VAL (path->flags, L_TRACEREGION_FLAG_HAS_UNSAFE_JSR))
	fprintf (F, "HAS_UNSAFE_JSR ");
      if (L_EXTRACT_BIT_VAL (path->flags, L_TRACEREGION_FLAG_HAS_JSR))
	fprintf (F, "HAS_JSR ");
      if (L_EXTRACT_BIT_VAL (path->flags, L_TRACEREGION_FLAG_HAS_POINTER_ST))
	fprintf (F, "HAS_POINTER_ST ");
    }
  fprintf (F, "\n");
}


void
LB_hb_print_path_list (FILE * F, LB_hb_Path * list)
{
  LB_hb_Path *ptr;

  fprintf (ERR, "Path info\n");

  List_start (LB_hb_all_paths);
  while ((ptr = List_next (LB_hb_all_paths)))
    LB_hb_print_path (F, ptr);
}


/*=========================================================================*/
/*
 *    Finding all paths modified version of Fig8.11 in
 *      Combinatorial Algorithms, Reingold et al.
 */
/*=========================================================================*/

static void
LB_hb_unmark (L_Cb * cb)
{
  int i, num_elem, *buf = NULL;
  L_Cb *buf_cb;

  cb->flags = L_SET_BIT_FLAG (cb->flags, L_CB_VISITED);

  num_elem = Set_size (B[cb->id]);
  if (num_elem == 0)
    return;

  buf = (int *) alloca (sizeof (int) * num_elem);
  Set_2array (B[cb->id], buf);

  for (i = 0; i < num_elem; i++)
    {
      B[cb->id] = Set_delete (B[cb->id], buf[i]);
      buf_cb = L_cb_hash_tbl_find (L_fn->cb_hash_tbl, buf[i]);
      if (!L_EXTRACT_BIT_VAL (buf_cb->flags, L_CB_VISITED))
	LB_hb_unmark (buf_cb);
    }
}


static void
LB_hb_record_path (L_Cb * header)
{
  int i, num_blocks, *block_buf = NULL;
  LB_hb_Path *path = NULL;

  num_blocks = Stack_get_contents (LB_path_stack, &block_buf);
  path = LB_hb_new_path ();
  path->num_blocks = num_blocks;
  path->blocks = block_buf;
  path->block_set = Stack_get_content_set (LB_path_stack);

  LB_hb_all_paths = List_insert_last (LB_hb_all_paths, path);

  LB_hb_path_total_paths++;

  LB_hb_path_all_blocks = Set_union_acc(LB_hb_path_all_blocks,
					path->block_set);

  for (i = 0; i < num_blocks; i++)
    {
      L_Cb *cb;
      cb = L_cb_hash_tbl_find (L_fn->cb_hash_tbl, block_buf[i]);
      if (cb->weight < LB_hb_min_cb_weight)
	{
	  LB_hb_path_total_zpaths++;
	  break;
	}
    }

#ifdef DEBUG_PATH
  fprintf (ERR, "> Record path %d (header %d)\n", path->id, header->id);
  fprintf (ERR, "\tblocks: ");
  for (i = 0; i < path->num_blocks; i++)
    fprintf (ERR, "%d ", path->blocks[i]);

  fprintf (ERR, "\n");
#endif

  return;
}


static void
LB_hb_find_path (L_Cb * start, L_Cb * end, Set blocks, L_Cb * cb, int *flag)
{
  int g;
  L_Flow *flow;
  L_Cb *dst_cb;

  *flag = 0;
  Stack_push_top (LB_path_stack, cb->id);

  cb->flags = L_CLR_BIT_FLAG (cb->flags, L_CB_VISITED);

  if (cb == end)
    {
      LB_hb_record_path (start);
      *flag = 1;
    }
  else
    {
      /* search all successors to cb */

      L_Flow *max_flow = cb->dest_flow;

      for (flow = cb->dest_flow; flow; flow = flow->next_flow)
	if (flow->weight > max_flow->weight)
	  max_flow = flow;
      
      for (flow = cb->dest_flow; flow; flow = flow->next_flow)
	{
	  dst_cb = flow->dst_cb;
	    
	  /* only consider cb's in the block Set */
	  if ((dst_cb->id != end->id) && !Set_in (blocks, dst_cb->id))
	    continue;

	  /* speed things up by avoiding blocks we won't include
	     due to low weight */

	  if ((dst_cb->weight < LB_hb_min_cb_weight) &&
	      (!LB_hb_min_ratio_sens_opct ||
	       (LB_hb_path_total_zpaths > LB_MAX_Z_PATHS)))
	    continue;

	  /* dont include a jump_rg */
	  if (L_EXTRACT_BIT_VAL (dst_cb->flags, L_CB_HAS_JRG))
	    continue;

	  /* dont include a block already selected for a hyperblock
	   * loop unless it is a peelable loop */
	  if (L_EXTRACT_BIT_VAL (dst_cb->flags, L_CB_HYPERBLOCK_LOOP) &&
	      !L_find_attr (dst_cb->attr, LB_MARKED_FOR_LOOP_PEEL))
	    continue;
	  
	  if (L_EXTRACT_BIT_VAL (dst_cb->flags, L_CB_VISITED))
	    {
	      if ((LB_hb_path_total_paths > LB_MAX_PATHS) &&
		  (*flag == 0) && 
		  Set_in (LB_hb_path_all_blocks, dst_cb->id))
		{
		  LB_hb_path_max_path_exceeded = 1;
		  if (flow != max_flow)
		    continue;
		}
	      LB_hb_find_path (start, end, blocks, dst_cb, &g);
	      *flag = *flag || g;
	    }
	}
    }

  if (*flag)
    {
      LB_hb_unmark (cb);
    }
  else
    {
      for (flow = cb->dest_flow; flow != NULL; flow = flow->next_flow)
	{
	  dst_cb = flow->dst_cb;

	  /* only consider cb's in blocks Set */
	  if (!Set_in (blocks, dst_cb->id))
	    continue;

	  /* speed things up by avoiding blocks we won't include
	     due to low weight */

	  if (!LB_hb_min_ratio_sens_opct &&
	      (dst_cb->weight < LB_hb_min_cb_weight))
	    continue;

	  /* dont include a jump_rg */
	  if (L_EXTRACT_BIT_VAL (dst_cb->flags, L_CB_HAS_JRG))
	    continue;

	  /* dont include a block already selected for a hyperblock
	   * loop unless it is a peelable loop */
	  if (L_EXTRACT_BIT_VAL (dst_cb->flags, L_CB_HYPERBLOCK_LOOP) &&
	      !L_find_attr (dst_cb->attr, LB_MARKED_FOR_LOOP_PEEL))
	    continue;

	  B[dst_cb->id] = Set_add (B[dst_cb->id], cb->id);
	}
    }

  (void) Stack_pop (LB_path_stack);
}


int
LB_hb_find_all_paths (L_Cb * start, L_Cb * end, Set blocks)
{
  int i, flag, num_cb, *buf = NULL;
  L_Cb *cb;

#ifdef DEBUG_PATH
  fprintf (ERR, "Finding all paths (%d -> %d)\n", start->id, end->id);
  Set_print (ERR, "b", blocks);
#endif

  Stack_clear (LB_path_stack);
  LB_hb_delete_all_path ();

  LB_hb_path_max_id = 0;
  LB_hb_path_max_dep_height = 0;
  LB_hb_path_total_ops = 0;
  LB_hb_path_total_paths = 0;
  LB_hb_path_total_zpaths = 0;
  LB_hb_path_max_path_exceeded = 0;
  LB_hb_path_all_blocks = Set_dispose (LB_hb_path_all_blocks);

  if (LB_hb_select_all_blocks (start) || 
      LB_hb_select_exact_blocks (start) ||
      LB_hb_ignore_block (start) ||
      LB_hb_ignore_block (end) ||
      L_EXTRACT_BIT_VAL (start->flags, L_CB_HAS_JRG) ||
      L_EXTRACT_BIT_VAL (end->flags, L_CB_HAS_JRG))
    return 0;

  num_cb = Set_size (blocks);
  buf = (int *) alloca (sizeof (int) * num_cb);
  Set_2array (blocks, buf);

  for (i = 0; i < num_cb; i++)
    {
      cb = L_cb_hash_tbl_find (L_fn->cb_hash_tbl, buf[i]);
      cb->flags = L_SET_BIT_FLAG (cb->flags, L_CB_VISITED);
      if (B[cb->id])
	B[cb->id] = Set_dispose (B[cb->id]);
    }

  LB_hb_find_path (start, end, blocks, start, &flag);

  return List_size (LB_hb_all_paths);
}

/*=======================================================================*/
/*
 *    Path data collection functions
 */
/*=======================================================================*/

static void
LB_hb_find_total_num_ops (Set blocks)
{
  int i, num_cb, *buf = NULL;
  L_Cb *cb;
  L_Attr *attr;

  num_cb = Set_size (blocks);
  buf = (int *) alloca (sizeof (int) * num_cb);
  Set_2array (blocks, buf);

  for (i = 0, LB_hb_path_total_ops = 0; i < num_cb; i++)
    {
      int cb_size;
      cb = L_cb_hash_tbl_find (L_fn->cb_hash_tbl, buf[i]);
      cb_size = L_cb_size (cb);
      if ((attr = L_find_attr (cb->attr, LB_MARKED_FOR_LOOP_PEEL)))
	cb_size *= attr->field[0]->value.i;
      LB_hb_path_total_ops += cb_size;
    }
  return;
}


static void
LB_hb_find_path_num_ops (LB_hb_Path * path)
{
  int i, num_ops;
  L_Cb *cb;
  L_Attr *attr;

  for (i = 0, num_ops = 0; i < path->num_blocks; i++)
    {
      int cb_size;
      cb = L_cb_hash_tbl_find (L_fn->cb_hash_tbl, path->blocks[i]);
      cb_size = L_cb_size (cb);
      if ((attr = L_find_attr (cb->attr, LB_MARKED_FOR_LOOP_PEEL)))
	cb_size *= attr->field[0]->value.i;

      num_ops += cb_size;
    }
  path->num_ops = num_ops;
  return;
}


static void
LB_hb_find_path_exec_ratio (LB_hb_Path * path)
{
  int i;
  L_Cb *curr_cb, *next_cb, *head_cb, *dst_cb;
  L_Flow *flow;
  double exec_ratio, weight, weight_to_next;
  Set cur_path_blocks = NULL;

  if (path == NULL)
    L_punt ("LB_hb_find_path_exec_ratio: path is NULL");

  exec_ratio = 1.0;

  head_cb = L_cb_hash_tbl_find (L_fn->cb_hash_tbl, path->blocks[0]);

  curr_cb = head_cb;

  weight = head_cb->weight;
  weight_to_next = 0.0;
  for (flow = head_cb->dest_flow; flow != NULL; flow = flow->next_flow)
    {
      dst_cb = flow->dst_cb;
      if (Set_in (LB_hb_path_all_blocks, dst_cb->id))
	weight_to_next += flow->weight;
    }

  exec_ratio *= (weight != 0.0) ? (weight_to_next / weight) : 0.0;
  cur_path_blocks = Set_add (cur_path_blocks, head_cb->id);

  for (i = 1; i < path->num_blocks; i++)
    {
      next_cb = L_cb_hash_tbl_find (L_fn->cb_hash_tbl, path->blocks[i]);
      weight = curr_cb->weight;
      weight_to_next = 0.0;
      for (flow = curr_cb->dest_flow; flow != NULL; flow = flow->next_flow)
	{
	  dst_cb = flow->dst_cb;
	  if ((dst_cb == next_cb) || (dst_cb == curr_cb))
	    weight_to_next += flow->weight;

	  /* SAM 3-98 - semi hack to account for possible peeled loop
             exec ratio */

	  else if ((Set_in (cur_path_blocks, dst_cb->id)) &&
		   (L_find_attr (dst_cb->attr, LB_MARKED_FOR_LOOP_PEEL)))
	    weight_to_next += flow->weight;
	}

      exec_ratio *= (weight != 0.0) ? (weight_to_next / weight) : 0.0;
      cur_path_blocks = Set_add (cur_path_blocks, curr_cb->id);
      curr_cb = next_cb;
    }

  path->exec_ratio = exec_ratio;
  Set_dispose (cur_path_blocks);
  return;
}


static void
LB_hb_find_path_flags (LB_hb_Path * path)
{
  int i, flags;
  L_Oper *oper;
  L_Cb *cb;

  if (!path)
    L_punt ("LB_hb_find_path_flags: path is NULL");

  flags = 0;
  for (i = 0; i < path->num_blocks; i++)
    {
      cb = L_cb_hash_tbl_find (L_fn->cb_hash_tbl, path->blocks[i]);
      for (oper = cb->first_op; oper != NULL; oper = oper->next_op)
	{
	  if (L_subroutine_call_opcode (oper))
	    {
	      flags = L_SET_BIT_FLAG (flags, 
				      L_TRACEREGION_FLAG_HAS_JSR);
	      if (!L_side_effect_free_sub_call (oper))
		flags = L_SET_BIT_FLAG (flags, 
					L_TRACEREGION_FLAG_HAS_UNSAFE_JSR);
	    }

	  if (L_general_store_opcode (oper) && L_pointer_store (oper))
	    flags = L_SET_BIT_FLAG (flags, 
				    L_TRACEREGION_FLAG_HAS_POINTER_ST);
	}
    }
  path->flags = flags;
  return;
}


/*
 *    Here we build a temporary cb to calculate the height
 */
static int
LB_hb_find_path_dep_height (LB_hb_Path * path)
{
  int i, dep_height;
  L_Cb *cb;
  L_Oper *oper, *new_oper;
  L_Flow *old_flow, *new_flow;

  for (i = 0; i < path->num_blocks; i++)
    {
      cb = L_cb_hash_tbl_find (L_fn->cb_hash_tbl, path->blocks[i]);
      /* omit jumps since they won't appear in hyperblock */
      for (oper = cb->first_op; oper; oper = oper->next_op)
	{
	  if (L_uncond_branch_opcode (oper))
	    continue;
	  new_oper = L_copy_operation (oper);
	  L_insert_oper_after (LB_path_cb, LB_path_cb->last_op, new_oper);
	  if (L_cond_branch_opcode (oper))
	    {
	      old_flow = L_find_flow_for_branch (cb, oper);
	      if (!(oper->next_op) &&
		  ((i + 1) < path->num_blocks) &&
		  (old_flow->dst_cb->id == path->blocks[i + 1]))
		old_flow = L_find_last_flow (cb->dest_flow);

	      new_flow = L_copy_single_flow (old_flow);
	      new_flow->cc = 1;
	      LB_path_cb->dest_flow =
		L_concat_flow (LB_path_cb->dest_flow, new_flow);
	    }
	}
    }

  dep_height = LB_hb_calc_cb_dep_height (LB_path_cb);

  /* delete opers */
  L_delete_all_oper (LB_path_cb->first_op, 1);
  L_delete_all_flow (LB_path_cb->dest_flow);

  LB_path_cb->dest_flow = NULL;
  LB_path_cb->first_op = NULL;
  LB_path_cb->last_op = NULL;

  if (dep_height > LB_hb_path_max_dep_height)
    LB_hb_path_max_dep_height = dep_height;

  return dep_height;
}


static double
LB_hb_find_path_priority (LB_hb_Path * path)
{
  double h_ratio, op_ratio, priority;

  if (LB_hb_exclude_all_jsrs &&
      L_EXTRACT_BIT_VAL (path->flags, L_TRACEREGION_FLAG_HAS_JSR))
    {
      priority = 0.0;
    }
  else if (LB_hb_exclude_all_pointer_stores &&
	   L_EXTRACT_BIT_VAL (path->flags, L_TRACEREGION_FLAG_HAS_POINTER_ST))
    {
      priority = 0.0;
    }
  else
    {
      h_ratio = 1.0 - ((double) path->dep_height / 
		       (double) LB_hb_path_max_dep_height);
      op_ratio = 1.0 - ((double) path->num_ops / 
			(double) LB_hb_path_total_ops);
      priority = path->exec_ratio * (0.1 + h_ratio + op_ratio);
      if (L_EXTRACT_BIT_VAL (path->flags, L_TRACEREGION_FLAG_HAS_UNSAFE_JSR))
	priority *= 0.25;
      else if (L_EXTRACT_BIT_VAL (path->flags, L_TRACEREGION_FLAG_HAS_JSR))
	priority *= 0.01;
    }

  return priority;
}


static LB_hb_Sort_List *sort_buf;


static void
swap (int i, int j)
{
  double temp_weight;
  void *temp_ptr;

  temp_weight = sort_buf[i].weight;
  temp_ptr = sort_buf[i].ptr;
  sort_buf[i].weight = sort_buf[j].weight;
  sort_buf[i].ptr = sort_buf[j].ptr;
  sort_buf[j].weight = temp_weight;
  sort_buf[j].ptr = temp_ptr;
}


static void
heapify (int p, int size)
{
  int s, l, r;

  r = (p+1) << 1;
  l = r - 1; 

  if (l < size && sort_buf[l].weight > sort_buf[p].weight)
    s = l;
  else
    s = p;

  if (r < size && sort_buf[r].weight > sort_buf[s].weight)
    s = r;

  if (s != p)
    {
      swap (s, p);
      heapify (s, size);
    }
  return;
}

/*
 *    Sort paths by priority
 */
static void
LB_hb_sort_paths (void)
{
  int count, i;
  LB_hb_Path *ptr;

  count = List_size (LB_hb_all_paths);

  if (count == 0)
    L_punt ("LB_hb_sort_paths: 0 paths?");

  /* allocate an array of structs to sort */

  sort_buf =
    (LB_hb_Sort_List *) Lcode_malloc (sizeof (LB_hb_Sort_List) * count);

  if (!sort_buf)
    L_punt ("LB_hb_sort_paths: malloc out of space!");

  /* unlink linked list of paths, put elements in sort_buf array */

  List_start (LB_hb_all_paths);
  for (i = 0; (ptr = List_next (LB_hb_all_paths)); i++)
    {
      sort_buf[i].weight = ptr->priority;
      sort_buf[i].ptr = (void *) ptr;
    }

  LB_hb_all_paths = List_reset(LB_hb_all_paths);

  /* heap sort by path priority */

  for (i = (count >> 1) - 1; i >= 0; i--)
    heapify (i, count);

  LB_hb_all_paths = List_insert_last (LB_hb_all_paths,
				      sort_buf[0].ptr);

  for (i = count - 1; i > 0; i--)
    {
      sort_buf[0].weight = sort_buf[i].weight;
      sort_buf[0].ptr = sort_buf[i].ptr;

      heapify (0, i);

      LB_hb_all_paths = List_insert_last (LB_hb_all_paths,
					  sort_buf[0].ptr);
    }

  Lcode_free (sort_buf);
  sort_buf = NULL;

  return;
}


void
LB_hb_find_path_info (L_Cb * start, L_Cb * end, Set blocks)
{
  LB_hb_Path *ptr;

#ifdef DEBUG_PATH_INFO
  fprintf (ERR, "Find path info for region: %d -> %d\n", start->id, end->id);
  Set_print (ERR, "b", blocks);
#endif

  if (LB_hb_select_all_blocks (start) || LB_hb_select_exact_blocks (start))
    return;

  LB_hb_find_total_num_ops (blocks);

  List_start (LB_hb_all_paths);
  while ((ptr = (LB_hb_Path *)List_next (LB_hb_all_paths)))
    {
      LB_hb_find_path_num_ops (ptr);
      LB_hb_find_path_exec_ratio (ptr);
      LB_hb_find_path_flags (ptr);
      /* JWS 20000623
       * Don't bother to schedule paths that are ineligible anyway.
       */
      if (ptr->exec_ratio >= LB_hb_path_min_exec_ratio)
	{
	  ptr->dep_height = LB_hb_find_path_dep_height (ptr);
	  ptr->priority = LB_hb_find_path_priority (ptr);
	}
      else
	{
	  ptr->dep_height = 0;
	  ptr->priority = 0.0;
	}
    }

  LB_hb_sort_paths ();

#ifdef DEBUG_PATH_INFO
  LB_hb_print_path_list (ERR);
#endif
}

/*=======================================================================*/
/*
 *    Global var init rountines
 */
/*=======================================================================*/

void
LB_hb_init_path_globals (void)
{
  int i;

  /* alloc global vars */
  LB_path_cb = L_create_cb (0.0);
  LB_path_stack = Stack_create ();
  B = NULL;
  if (!(B = (Set *) Lcode_malloc (sizeof (Set) * (L_fn->max_cb_id + 1))))
    L_punt ("LB_hb_init_path_globals: malloc out of space");

  for (i = 0; i <= L_fn->max_cb_id; i++)
    B[i] = NULL;
  
  if (LB_hb_all_paths)
    L_warn ("Paths left over");

  LB_hb_all_paths = NULL;
  return;
}

void
LB_hb_deinit_path_globals ()
{
  int i;

  /* free up global vars */
  L_delete_cb (L_fn, LB_path_cb);
  LB_path_cb = NULL;
  LB_hb_delete_all_path ();

  Stack_clear (LB_path_stack);
  Stack_delete (LB_path_stack);
  for (i = 0; i <= L_fn->max_cb_id; i++)
    if (B[i])
      B[i] = Set_dispose (B[i]);

  Lcode_free (B);
  B = NULL;
}


int
LB_hb_path_contains_excludable_hazard (LB_hb_Path * path, int main_path)
{
  if (L_EXTRACT_BIT_VAL (path->flags, L_TRACEREGION_FLAG_HAS_UNSAFE_JSR))
    {
      switch (LB_hb_unsafe_jsr_hazard_method)
	{
	case LB_HAZARD_EXCLUDE_ALL:
	  return (1);
	case LB_HAZARD_EXCLUDE_NON_TRACE:
	  if (!main_path)
	    return (1);
	  break;
	case LB_HAZARD_EXCLUDE_HEURISTIC:
	  /* The path priority should be lower so that will take care
             of exclusion */
	  break;
	case LB_HAZARD_IGNORE:
	  break;
	default:
	  break;
	}
    }
  else if (L_EXTRACT_BIT_VAL (path->flags, L_TRACEREGION_FLAG_HAS_JSR))
    {
      switch (LB_hb_safe_jsr_hazard_method)
	{
	case LB_HAZARD_EXCLUDE_ALL:
	  return (1);
	case LB_HAZARD_EXCLUDE_NON_TRACE:
	  if (!main_path)
	    return (1);
	  break;
	case LB_HAZARD_EXCLUDE_HEURISTIC:
	  /* The path priority should be lower so that will take care
             of exclusion */
	  break;
	case LB_HAZARD_IGNORE:
	  break;
	default:
	  break;
	}
    }
  if (L_EXTRACT_BIT_VAL (path->flags, L_TRACEREGION_FLAG_HAS_POINTER_ST))
    {
      switch (LB_hb_pointer_st_hazard_method)
	{
	case LB_HAZARD_EXCLUDE_ALL:
	  return (1);
	case LB_HAZARD_EXCLUDE_NON_TRACE:
	  if (!main_path)
	    return (1);
	  break;
	case LB_HAZARD_EXCLUDE_HEURISTIC:
	  /* The path priority should be lower so that will take care
             of exclusion */
	  break;
	case LB_HAZARD_IGNORE:
	  break;
	default:
	  break;
	}
    }

  return (0);
}

/*=======================================================================*/
/*
 *    Heuristic to select subset of paths for inclusion in hyperblock
 */
/*=======================================================================*/

LB_TraceRegion *
LB_hb_select_trivial (int type, L_Cb *start, L_Cb * end, Set all_blocks,
		      int id)
{
  LB_TraceRegion *tr;
  Set blocks = NULL;

  if (LB_hb_select_all_blocks (start))
    blocks = Set_copy (all_blocks);
  else if (LB_hb_select_exact_blocks (start))
    blocks = LB_hb_find_exact_blocks (start);
  else
    L_punt ("LB_hb_select_trivial: unmarked region");

#ifdef DEBUG_SELECT
  fprintf (ERR, "  Selection heuristics overridden.\n");
#endif

  tr = LB_create_traceregion (L_fn, id, start, blocks, type);

  tr->exec_ratio = 1.0;
  tr->slots_used = 0;
  tr->slots_avail = 0;
  tr->dep_height = 0;

  Set_dispose (blocks);

  return tr;
}


LB_TraceRegion *
LB_hb_select_paths (int type, L_Cb * start, L_Cb * end, Set all_blocks,
		    int id)
{
  int i, slots_used, slots_avail, new_ops, new_jsr;
  int skipping, skipped_prev, flags, max_dep_height, ref_dep_height;
  double tot_exec_ratio, ref_exec_ratio, last_selected;

  Set blocks = NULL;
  LB_hb_Path *main_path, *curr_path;
  L_Cb *cb;
  L_Oper *oper;
  LB_TraceRegion *new_region;

#ifdef DEBUG_SELECT
  fprintf (ERR, "> PATH-SEL [%d:%d] (fn %s)\n", start->id,
	   end->id, L_fn->name);
#endif

   /* real selection procedure below */

  if (!List_size(LB_hb_all_paths))
    L_punt ("LB_hb_form_region: no paths to consider??");

  /* Compute reference dependence height -- dependence height of "average"
   * execution path through the region
   */

  {
    double dep_acc = 0.0, exec_acc = 0.0;

    main_path = List_first (LB_hb_all_paths);
    List_start (LB_hb_all_paths);
    while ((curr_path = List_next (LB_hb_all_paths)))
      {
	exec_acc += curr_path->exec_ratio;
	dep_acc += curr_path->exec_ratio * curr_path->dep_height;	  
      }

    ref_dep_height = (exec_acc == 0.0) ? 0 : 
      (int) ((dep_acc + exec_acc / 2) / exec_acc);

    if (!ref_dep_height)
      ref_dep_height = main_path->dep_height;
  }

#ifdef DEBUG_SELECT
      fprintf (ERR, "  Average dependence height %d\n", ref_dep_height);
#endif

  List_start (LB_hb_all_paths);
  main_path = (LB_hb_Path *) List_next (LB_hb_all_paths);

  if (main_path->exec_ratio < LB_hb_path_min_main_exec_ratio)
    {
#ifdef DEBUG_SELECT
      fprintf (ERR, "  Region rejected: main path exec ratio (%f)\n",
	       main_path->exec_ratio);
#endif
      return NULL;
    }

  if (main_path->priority <= 0.0)
    {
#ifdef DEBUG_SELECT
      fprintf (ERR, "  Region rejected: main path priority (%f)\n",
	       main_path->priority);
#endif
      return NULL;
    }

  max_dep_height = 0;
  flags = 0;

  main_path->selected = 1;
  flags |= main_path->flags;
  if (main_path->dep_height > max_dep_height)
    max_dep_height = main_path->dep_height;
  last_selected = main_path->priority;
  blocks = Set_union_acc (blocks, main_path->block_set);
  slots_used = main_path->num_ops;
  skipping = 0;

  {
    int issue_slots, growth_slots;

    issue_slots = (int)((main_path->dep_height + 1) * LB_hb_issue_width);
    growth_slots = (int) (main_path->num_ops * LB_hb_path_max_op_growth);

    if (issue_slots < growth_slots)
      slots_avail = issue_slots;
    else
      slots_avail = growth_slots;
  }

  tot_exec_ratio = ref_exec_ratio = main_path->exec_ratio;

#ifdef DEBUG_SELECT
  fprintf (ERR, "  (path %d er %0.5f dh %d no %d)", main_path->id,
	   main_path->exec_ratio, main_path->dep_height, new_ops);
  Set_print (ERR, "b", main_path->block_set);

  fprintf (ERR, "  Main path (%d slots / %d avail)(exec ratio %f)(dep height %d)\n",
	   slots_used, slots_avail, main_path->exec_ratio,
	   main_path->dep_height);
#endif

  while ((curr_path = List_next (LB_hb_all_paths)))
    {
      int opct_saved, small_opct;
      
      opct_saved = 0;
      skipped_prev = skipping;
      skipping = 1;

      /* OPERATION COUNT */

      new_ops = 0; new_jsr = 0;
      for (i = 0; i < curr_path->num_blocks; i++)
	{
	  if (!Set_in (blocks, curr_path->blocks[i]))
	    {
	      cb = L_cb_hash_tbl_find (L_fn->cb_hash_tbl, 
				       curr_path->blocks[i]);
	      for (oper = cb->first_op; oper; oper = oper->next_op)
		{
		  if (!L_uncond_branch_opcode(oper))
		    new_ops++;
		  if (L_subroutine_call_opcode (oper))
		    new_jsr = 1;
		}
	    }
	}

      small_opct = (new_ops <= LB_hb_min_ratio_sens_opct) && !new_jsr ? 1 : 0;

#ifdef DEBUG_SELECT
      fprintf (ERR, "  (path %d er %0.5f dh %d no %d)", curr_path->id,
	       curr_path->exec_ratio, curr_path->dep_height, new_ops);
      Set_print (ERR, "b", curr_path->block_set);
#endif

      /* DEPENDENCE HEIGHT */

      if (curr_path->dep_height > (ref_dep_height * LB_hb_path_max_dep_growth))
	{
#ifdef DEBUG_SELECT
	  fprintf (ERR, "  EXCL(dep height %d)\n", curr_path->dep_height);
#endif
	  LB_hb_all_paths = List_delete_current (LB_hb_all_paths);
	  LB_hb_delete_path (curr_path);
	  continue;
	}

      if (((curr_path->dep_height + 10) < ref_dep_height) &&
	  (curr_path->exec_ratio > 0.10))
	{
#ifdef DEBUG_SELECT
	  fprintf (ERR, "  EXCL(dep height %d)\n", curr_path->dep_height);
#endif
	  LB_hb_all_paths = List_delete_current (LB_hb_all_paths);
	  LB_hb_delete_path (curr_path);
	  continue;
	}

      /* EXCLUDABLE HAZARDS */

      if (LB_hb_path_contains_excludable_hazard (curr_path, 0))
	{
#ifdef DEBUG_SELECT
	  fprintf (ERR, "  EXCL(hazard)\n");
#endif
	  LB_hb_all_paths = List_delete_current (LB_hb_all_paths);
	  LB_hb_delete_path (curr_path);
	  continue;
	}

      if ((new_ops + slots_used) > slots_avail)
	{
#ifdef DEBUG_SELECT
	  fprintf (ERR, "  EXCL(slots %d)\n", new_ops);
#endif
	  LB_hb_all_paths = List_delete_current (LB_hb_all_paths);
	  LB_hb_delete_path (curr_path);
	  continue;
	}

      /* EXECUTION RATIO */

      if ((ref_exec_ratio > 0.50) && 
	  (curr_path->exec_ratio / ref_exec_ratio) <
	  LB_hb_path_main_rel_exec_ratio)
	{
#ifdef DEBUG_SELECT
	  fprintf (ERR, "  EXCL (main rel exec ratio %f)\n", curr_path->exec_ratio);
#endif
	  if (!small_opct)
	    {
	      LB_hb_all_paths = List_delete_current (LB_hb_all_paths);
	      LB_hb_delete_path (curr_path);
	      continue;
	    }
	  else
	    opct_saved++;
	}

      if ((curr_path->exec_ratio < LB_hb_path_min_exec_ratio) &&
	  (ref_exec_ratio >= LB_hb_path_min_exec_ratio))
	{
#ifdef DEBUG_SELECT
	  fprintf (ERR, "  EXCL (exec ratio %f)\n", curr_path->exec_ratio);
#endif
	  if (!small_opct)
	    {
	      LB_hb_all_paths = List_delete_current (LB_hb_all_paths);
	      LB_hb_delete_path (curr_path);
	      continue;
	    }
	  else
	    opct_saved++;
	}

      /* PRIORITY RATIO */
	  
      if (curr_path->priority < (last_selected * 
				 LB_hb_path_min_priority_ratio *
				 (skipped_prev ? 1.0 : 0.25)))
	{
#ifdef DEBUG_SELECT
	  fprintf (ERR, "  EXCL (priority %f / last %f)\n",
		   curr_path->priority, last_selected);
#endif
	  if (!small_opct)
	    {
	      LB_hb_all_paths = List_delete_current (LB_hb_all_paths);
	      LB_hb_delete_path (curr_path);
	      continue;
	    }
	  else
	    opct_saved++;
	}

      if (opct_saved)
	{
	  /* tolerate only very tiny dep height increase due to inclusion of
	   * op count saved paths
	   */
	  if (curr_path->dep_height > (ref_dep_height + 1))
	    {
	      LB_hb_all_paths = List_delete_current (LB_hb_all_paths);
	      LB_hb_delete_path (curr_path);
	      continue;
	    }

#ifdef DEBUG_SELECT
	  fprintf (ERR, "  (saved by op count (%d ops))\n", new_ops);
#endif
	}

#ifdef DEBUG_SELECT
      fprintf (ERR, "  INCL\n");
#endif

      curr_path->selected = 1;
      flags |= curr_path->flags;
      if (curr_path->dep_height > max_dep_height)
	max_dep_height = curr_path->dep_height;
      last_selected = curr_path->priority;
      skipping = 0;
      slots_used += new_ops;
      tot_exec_ratio += curr_path->exec_ratio;
      blocks = Set_union_acc (blocks, curr_path->block_set);
    }
  
#ifdef DEBUG_SELECT
  {
    double avail_exec_ratio;

    avail_exec_ratio = 0.0;

    List_start (LB_hb_all_paths);
    while ((curr_path = (LB_hb_Path *)List_next (LB_hb_all_paths)))
      avail_exec_ratio += curr_path->exec_ratio;

    fprintf (ERR, "  SUMMARY [%d:%d]\n  ", start->id, end->id);
    Set_print (ERR, "all_cb", all_blocks);
    fprintf (ERR, "  ");
    Set_print (ERR, "selected_cb", blocks);
    fprintf (ERR, "  %d blocks included / %d blocks avail\n",
	     Set_size (blocks), Set_size (all_blocks));
    fprintf (ERR, "  %d slots used / %d slots available\n",
	     slots_used, slots_avail);
    fprintf (ERR, "  %7.6f execution covered / %7.6f execution avail\n",
	     tot_exec_ratio, avail_exec_ratio);
  }
#endif

  blocks = Set_intersect_acc (blocks, all_blocks);
  
  {
    int bcnt;
    double min_rat;

    bcnt = Set_size (blocks);

    min_rat = tot_exec_ratio / 5.0;
    
    if (bcnt)
      {
	int *barr = malloc (bcnt * sizeof (int)), i, del = 0;

	Set_2array (blocks, barr);

	for (i = 0; i < bcnt; i++)
	  {
	    double rat = 0.0;

	    List_start (LB_hb_all_paths);
	    while ((curr_path = (LB_hb_Path *)List_next (LB_hb_all_paths)))
	      {
		if (!curr_path->selected ||
		    !Set_in (curr_path->block_set, barr[i]))
		  continue;
		rat += curr_path->exec_ratio;
	      }

	    if (rat < min_rat)
	      {
		L_Cb *cb;
		L_Oper *op;
		int opcnt = 0;
		
		cb = L_cb_hash_tbl_find (L_fn->cb_hash_tbl, barr[i]);

		for (op = cb->first_op; op; op = op->next_op)
		  if (op->opc != Lop_DEFINE)
		    opcnt++;

		if (opcnt > 6)
		  {
		    blocks = Set_delete (blocks, barr[i]);
		    fprintf (stderr, "> Knocked block %d out of HB %d (%0.4f)\n",
			     barr[i], start->id, rat);
		    del = 1;
		  }
	      }
	  }

	free (barr);

	if (del)
	  {
	    int good = 0;

	    tot_exec_ratio = 0.0;
	    List_start (LB_hb_all_paths);
	    while ((curr_path = (LB_hb_Path *)List_next (LB_hb_all_paths)))
	      {
		if (!curr_path->selected)
		  continue;
		if (!Set_subtract_empty (curr_path->block_set, blocks))
		  curr_path->selected = 0;
		else
		  tot_exec_ratio += curr_path->exec_ratio;
		good = 1;
	      }

	    blocks = Set_dispose (blocks);

	    if (!good)
	      {
		fprintf (stderr, "> Knocking killed HB %d\n",
			 start->id);
		return NULL;
	      }

	    List_start (LB_hb_all_paths);
	    while ((curr_path = (LB_hb_Path *)List_next (LB_hb_all_paths)))
	      {
		if (!curr_path->selected)
		  continue;
		blocks = Set_union_acc (blocks, curr_path->block_set);
	      }
	  }
      }
  }

  /*
   *        Create new region, fill in all fields!
   */

  new_region = LB_create_traceregion (L_fn, id, start, blocks, type);
  new_region->exec_ratio = tot_exec_ratio;
  new_region->slots_used = slots_used;
  new_region->slots_avail = slots_avail;

#ifdef DEBUG_SELECT_PENALTY
  {
    double pen = 0.0;
    List_start (LB_hb_all_paths);
    while ((curr_path = (LB_hb_Path *)List_next (LB_hb_all_paths)))
      {
	if (!curr_path->selected)
	  continue;

	pen += curr_path->exec_ratio * 
	  (max_dep_height - curr_path->dep_height);
      }

    if (pen > 0.001)
      fprintf (ERR, "  Header %d Penalty %0.3f cy\n", start->id, pen);
  }
#endif

  new_region->flags |= flags;
  new_region->dep_height = max_dep_height;

  Set_dispose (blocks);

  return new_region;
}

/*
 * LB_hb_path_region_formation
 * ----------------------------------------------------------------------
 * Run the path-based region selector on the region cbs.  Return a new
 * traceregion or NULL if the resulting traceregion would be invalid.
 */

LB_TraceRegion *
LB_hb_path_region_formation (L_Cb *header_cb, L_Cb *exit_cb,
			     Set region_cb, int type,
			     LB_TraceRegion_Header *header)
{
  LB_TraceRegion *tr;

  LB_hb_find_path_info (header_cb, exit_cb, region_cb);
  tr = LB_hb_select_paths (type, header_cb,
			   exit_cb, region_cb,
			   header->next_id++);

  return tr;
}

