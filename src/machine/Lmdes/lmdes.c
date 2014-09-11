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
 *
 *  File:  lmdes.c
 *
 *  Description:
 *    Reads in a low-level mdes file and builds all the necessary
 *    structures and arrays.
 *
 *  Creation Date :  May, 1993
 *
 *  Authors:  John C. Gyllenhaal, Scott Mahlke, Wen-mei Hwu
 *
\*****************************************************************************/
/* 10/29/02 REK Adding config.h */
#include <config.h>
#include <string.h>
#include <stdlib.h>
#include <stdarg.h>
#include <malloc.h>
#include "lmdes.h"
#include <library/l_alloc_new.h>
#include <library/i_error.h>


/* Pointer through which the current LMDES is accessed */
Mdes *lmdes;

/* The version of the lmdes file this routine is expecting */
#define MDES_VERSION 3

/* Internal functions, do not use externally */
static Mdes *load_lmdes (char *file_name);
void Malloc_struct (void **ptr, int size);
void Malloc_name (char **ptr, char *name);

/* Info structures for structures that can be dynamically allocated */
static L_Alloc_Pool *Mdes_Info_pool = NULL;
static L_Alloc_Pool *Mdes_Compatable_Alt_pool = NULL;

/*
 * Returns 1 if a MDES file has been loaded (L_init_lmdes called)
 * returns 0 otherwise.
 * Useful for determining if a L_init_lmdes should be called.
 */
int
lmdes_initialized (void)
{
  if (lmdes == NULL)
    return (0);
  else
    return (1);
}
/*
 * Returns 1 if opcode is specified in the mdes file.
 * Returns 0 if not.
 * Useful for gathering stats on all mdes opcodes in a pass.
 */
int
mdes_defined_opcode (unsigned int opcode)
{
  /* See if opcode is in lmdes op_table */
  if ((opcode > lmdes->max_opcode) || (lmdes->op_table[opcode] == NULL))
    {
      return (0);
    }

  /* Otherwise, legal opcode */
  return (1);
}

/*
 * Returns the maximum opcode defined in the mdes file.
 */
int
mdes_max_opcode (void)
{
  return (lmdes->max_opcode);
}

/*  
 * Returns 1 if any of the alt_flags specified by mask are set for the 
 * opcode passed.
 *
 * Returns 0 otherwise.
 */
int
op_flag_set (unsigned int opcode, int mask)
{
  /* Make sure opcode is valid */
  if ((opcode > lmdes->max_opcode) || (lmdes->op_table[opcode] == NULL))
    {
      I_punt ("op_flag_set: opcode (%i) not defined in mdes file %s",
	      opcode, lmdes->file_name);
    }

  if ((lmdes->op_table[opcode]->op_flags & mask) != 0)
    return (1);
  else
    return (0);
}

/*
 * Returns 1 if any of the alt_flags specified by mask are set for the 
 * specified alternative.
 * 
 * Returns 0 otherwise.
 */
int
alt_flag_set (unsigned int opcode, unsigned int alt_no, int mask)
{
  Mdes_Operation *op = NULL;

  /* Make sure valid opcode specified */
  if ((opcode > lmdes->max_opcode) ||
      ((op = lmdes->op_table[opcode]) == NULL))
    {
      I_punt ("alt_flag_set: opcode (%i) not defined in mdes file %s",
	      opcode, lmdes->file_name);
    }

  /* Make sure valid alt_no specified */
  if (alt_no >= op->num_alts)
    {
      I_punt ("alt_flag_set: alt (%i) >= number of alts (%i)\n",
	      alt_no, op->num_alts);
    }

  if ((op->alt[alt_no].alt_flags & mask) != 0)
    return (1);
  else
    return (0);
}
/*  
 * Returns 1 if any of the alt_flags specified by mask are set for any 
 * of the compatable alternatives for the opcode passed.
 * "The compatable alternatives" are those which have the 
 * operand types which match the instruction being scheduled.
 *
 * Returns 0 otherwise.
 */
int
any_alt_flag_set (Mdes_Info * mdes_info, int mask)
{
  Mdes_Compatable_Alt *comp_alt;

  for (comp_alt = mdes_info->compatable_alts; comp_alt != NULL;
       comp_alt = comp_alt->next)
    {
      if ((comp_alt->alt->alt_flags & mask) != 0)
	return (1);
    }
  return (0);
}

/* 
 * Returns the total number of src, dest and pred operands
 * Will be made into macro later.
 */
int
mdes_operand_count (void)
{
  return (lmdes->operand_count);
}

/*
 * Returns the total number of src, dest, pred, sync_in, and sync_out
 * operands.  Will be made into macro later.
 */
int
mdes_latency_count (void)
{
  return (lmdes->latency_count);
}

/*
 * Returns the number of operands of the specific type there are
 */
int
mdes_num_operands (unsigned int operand_type)
{
  if (lmdes == NULL)
    I_punt ("mdes_num_operands: mdes file not loaded");

  if (operand_type > 4)
    I_punt ("mdes_num_operands: operand_type %i out of bounds", operand_type);

  return (lmdes->number[operand_type]);
}

/*
 * Returns the null operand external id 
 */
int
mdes_null_operand (void)
{
  return (lmdes->null_external_id);
}

/*
 * Given an operand index, it prints out the operand type and
 * which operand number to out.
 */
void
print_mdes_operand_name (FILE * out, Mdes * mdes, unsigned int index)
{
  int operand_number;

  if (index >= mdes->latency_count)
    {
      fprintf (out, "operand index (%i) out of bounds", index);
      return;
    }

  if (index >= mdes->offset[MDES_SYNC_OUT])
    {
      operand_number = index - mdes->offset[MDES_SYNC_OUT];
      fprintf (out, "MDES_SYNC_OUT[%i]", operand_number);
    }
  else if (index >= mdes->offset[MDES_SYNC_IN])
    {
      operand_number = index - mdes->offset[MDES_SYNC_IN];
      fprintf (out, "MDES_SYNC_IN[%i]", operand_number);
    }
  else if (index >= mdes->offset[MDES_SRC])
    {
      operand_number = index - mdes->offset[MDES_SRC];
      fprintf (out, "MDES_SRC[%i]", operand_number);
    }
  else if (index >= mdes->offset[MDES_DEST])
    {
      operand_number = index - mdes->offset[MDES_DEST];
      fprintf (out, "MDES_DEST[%i]", operand_number);
    }
  else if (index >= mdes->offset[MDES_PRED])
    {
      operand_number = index - mdes->offset[MDES_PRED];
      fprintf (out, "MDES_PRED[%i]", operand_number);
    }
  else
    {
      I_punt ("print_mdes_operand_name: index %i not found", index);
    }

}

/*
 * Returns 1 if IO_sets set1 and set2 have at least one bit set
 * in common.
 * Otherwise, returns 0.
 */
static int
IO_sets_intersect (Mdes * mdes, Mdes_IO_Set * set1, Mdes_IO_Set * set2)
{
  int i;

  for (i = 0; i < mdes->IOmask_width; i++)
    {
      /* Return 1 if this part of the mask has a common bit set */
      if ((set1->mask[i] & set2->mask[i]) != 0)
	return (1);
    }

  /* Masks don't intersect, return 0 */
  return (0);
}

static int
IO_items_compatable (Mdes * mdes, Mdes_IO_Set ** operand_type1,
		     Mdes_IO_Set ** operand_type2)
{
  int i;

  /* Test all the operands for compatablitiy */
  for (i = 0; i < mdes->operand_count; i++)
    {
      /* If the operand i is not compatable, return 0 */
      if (!IO_sets_intersect (mdes, operand_type1[i], operand_type2[i]))
	{
	  return (0);
	}
    }
  /* All operands compatable, return 1 */
  return (1);
}

/* 
 * Prints out IO_Set name for the given external id.
 */
void
mdes_print_IO_set (FILE * out, unsigned int id)
{
  if ((id > lmdes->max_IO_set_id) || (lmdes->IO_set_table[id] == NULL))
    {
      fprintf (out, "(Illegal IO_Set id %i) ", id);
    }
  else
    {
      fprintf (out, "%-5s ", lmdes->IO_set_table[id]->name);
    }
}

/*
 * Builds the compatable alt list for the operation.
 * Returns an Mdes_Info structure with this list in it.
 *
 * Returns NULL on error.
 */
Mdes_Info *
build_mdes_info (unsigned int opcode, int *io_list)
{
  Mdes_Info *info;
  Mdes_Operation *op;
  Mdes_Compatable_Alt *tail, *comp_alt;
  Mdes_Alt *alt;
  Mdes_IO_Set **operand_type, **alt_operand_type;
  int i;

  /* Punt if opcode not defined */
  if ((opcode > lmdes->max_opcode) || (lmdes->op_table[opcode] == NULL))
    {
      fprintf (stderr,
	       "build_mdes_info: opcode %i undefined in lmdes (%s)\n",
	       opcode, lmdes->file_name);
      return (NULL);
    }

  /* Get the operation the opcode corresponds to */
  op = lmdes->op_table[opcode];

  /*
   * Build operand_type list from io_list 
   * use mdes->operand_type_buf to hold the io_list_types
   * (so I don't need to keep mallocing/freeing stuff)
   */
  operand_type = lmdes->operand_type_buf;
  for (i = 0; i < lmdes->operand_count; i++)
    {
      /* Make sure valid operand type specified */
      if ((io_list[i] > lmdes->max_IO_set_id) ||
	  (lmdes->IO_set_table[io_list[i]] == NULL))
	{
	  fprintf (stderr, "build_mdes_info: ");
	  print_mdes_operand_name (stderr, lmdes, i);
	  fprintf (stderr, " = %i. Undefined in lmdes (%s)\n",
		   io_list[i], lmdes->file_name);
	  return (NULL);
	}
      operand_type[i] = lmdes->IO_set_table[io_list[i]];
    }

  /* Allocate and initialize mdes_info structure */
  info = (Mdes_Info *) L_alloc (Mdes_Info_pool);
  info->opcode = opcode;
  info->compatable_alts = NULL;
  info->num_compatable_alts = 0;

  /* 
   * Test all the alts for compatable alts given this io_list,
   * and build compatable alt list
   */
  tail = NULL;
  for (i = 0; i < op->num_alts; i++)
    {
      /* Get alt and it's IO_item for easy access */
      alt = &op->alt[i];
      alt_operand_type = alt->IO_item->operand_type;

      /* If alt and io_list operands types are compatable,
       * add alt to compatable_alts list in mdes_info.
       */
      if (IO_items_compatable (lmdes, alt_operand_type, operand_type))
	{
	  comp_alt =
	    (Mdes_Compatable_Alt *) L_alloc (Mdes_Compatable_Alt_pool);
	  comp_alt->alt = alt;
	  comp_alt->next = NULL;

	  /* Add to end of linked list */
	  if (tail == NULL)
	    info->compatable_alts = comp_alt;
	  else
	    tail->next = comp_alt;

	  tail = comp_alt;

	  /* Update num compatable */
	  info->num_compatable_alts++;
	}
    }

  /* Make sure there is at least one compatable alt */
  if (info->num_compatable_alts < 1)
    {
      fprintf (stderr,
	       "build_mdes_info: No alts for opcode %i match the io_list.\n",
	       opcode);
      return (NULL);
    }

  /* Return the info structure just built */
  return (info);
}

void
free_mdes_info (Mdes_Info * info)
{
  Mdes_Compatable_Alt *alt, *next_alt;

  for (alt = info->compatable_alts; alt != NULL; alt = next_alt)
    {
      next_alt = alt->next;
      L_free (Mdes_Compatable_Alt_pool, alt);
    }
  L_free (Mdes_Info_pool, info);
}

/* 
 * Returns the operand index for a specified operand.
 * This version also detects 'out of range' operand specification
 */
int
operand_index (unsigned int operand_type, unsigned int operand_number)
{
  int index;

  /* Make sure legal operand type */
  if (operand_type > 4)
    I_punt ("operand_index: operand type %i must be between 0 and 4",
	    operand_type);

  /* Make sure operand_number is not out of bounds */
  if ((lmdes != NULL) && (operand_number >= lmdes->number[operand_type]))
    I_punt ("operand_index: illegal %s operand number %i (%i defined).",
	    lmdes->name[operand_type], operand_number,
	    lmdes->number[operand_type]);

  /* Calculate index */
  index = lmdes->offset[operand_type] + operand_number;

  return (index);
}

/* 
 * Returns the operand type for a specified operand_index.
 * This version also detects 'out of range' operand_index specification
 */
int
operand_type (unsigned int operand_index)
{
  int type;

  /* Make sure legal operand_index */
  if (operand_index >= lmdes->latency_count)
    I_punt ("operand_type: operand index %i must be between 0 and %i",
	    operand_index, lmdes->latency_count - 1);

  /* Lookup type */
  type = lmdes->index_type[operand_index];

  return (type);
}

/* 
 * Returns the operand number for a specified operand_index.
 * This version also detects 'out of range' operand_index specification
 */
int
operand_number (unsigned int operand_index)
{
  int number;

  /* Make sure legal operand_index */
  if (operand_index >= lmdes->latency_count)
    I_punt
      ("operand_number: operand index %i must be between 0 and %i",
       operand_index, lmdes->latency_count - 1);

  /* Lookup number */
  number = lmdes->index_number[operand_index];

  return (number);
}

/*
 * Returns the maximum operand type for the specified operand
 * for the compatable alternatives specified in mdes_info.
 */
int
max_operand_time (Mdes_Info * mdes_info, int operand_index)
{
  Mdes_Compatable_Alt *compatable_alt;
  int max_time;

  if (mdes_info == NULL)
    I_punt ("max_operand_type: mdes_info is NULL");

  if (mdes_info->compatable_alts == NULL)
    I_punt ("max_operand_type: No compatable alts in mdes_info");

  if (operand_index >= lmdes->latency_count)
    I_punt ("max_operand_type: operand_index (%i) out of bounds",
	    operand_index);

  /* Intiallize max to first alt's operand time */
  max_time =
    mdes_info->compatable_alts->alt->latency->operand_latency[operand_index];

  /* Search rest of compatable alts for max operand time */
  for (compatable_alt = mdes_info->compatable_alts->next;
       compatable_alt != NULL; compatable_alt = compatable_alt->next)
    {
      if (compatable_alt->alt->latency->operand_latency[operand_index] >
	  max_time)
	{
	  max_time =
	    compatable_alt->alt->latency->operand_latency[operand_index];
	}
    }
  return (max_time);
}

/*
 * Returns the minimum operand type for the specified operand
 * for the compatable alternatives specified in mdes_info.
 */
int
min_operand_time (Mdes_Info * mdes_info, int operand_index)
{
  Mdes_Compatable_Alt *compatable_alt;
  int min_time;

  if (mdes_info == NULL)
    I_punt ("min_operand_type: mdes_info is NULL");

  if (mdes_info->compatable_alts == NULL)
    I_punt ("min_operand_type: No compatable alts in mdes_info");

  if (operand_index >= lmdes->latency_count)
    I_punt ("min_operand_type: operand_index (%i) out of bounds",
	    operand_index);

  /* Intiallize max to first alt's operand time */
  min_time =
    mdes_info->compatable_alts->alt->latency->operand_latency[operand_index];

  /* Search rest of compatable alts for max operand time */
  for (compatable_alt = mdes_info->compatable_alts->next;
       compatable_alt != NULL; compatable_alt = compatable_alt->next)
    {
      if (compatable_alt->alt->latency->operand_latency[operand_index] <
	  min_time)
	{
	  min_time =
	    compatable_alt->alt->latency->operand_latency[operand_index];
	}
    }
  return (min_time);
}

/*
 * Returns the earliest time that one of the compatable alternatives
 * for this oper can be scheduled.
 */
int
mdes_calc_min_ready_time (Mdes_Info * mdes_info, int *ready_time)
{
  Mdes_Compatable_Alt *comp_alt;
  int *lat;
  int time, min_time;
  int i;

  if (mdes_info == NULL)
    I_punt ("mdes_calc_min_ready_time: mdes_info is NULL");

  min_time = -1;
  for (comp_alt = mdes_info->compatable_alts; comp_alt != NULL;
       comp_alt = comp_alt->next)
    {
      /* Get the latency times for this alt */
      lat = comp_alt->alt->latency->operand_latency;

      /* Get max contraint from all operands */
      time = 0;
      for (i = 0; i < lmdes->latency_count; i++)
	{
	  if ((ready_time[i] - lat[i]) > time)
	    time = ready_time[i] - lat[i];
	}

      /* If this is the first alt looked at, make this the min */
      if (min_time == -1)
	min_time = time;

      /* Otherwise, take the min of these max constraint times */
      else
	{
	  if (min_time > time)
	    min_time = time;
	}
    }

  return (min_time);
}

/*
 * Returns the operand latency for the specified alt and operand
 */
int
mdes_operand_latency (unsigned int opcode, unsigned int alt_no,
		      unsigned int operand_index)
{
  Mdes_Operation *op = NULL;
  int latency;

  /* Make sure opcode is valid */
  if ((opcode > lmdes->max_opcode) ||
      ((op = lmdes->op_table[opcode]) == NULL))
    {
      I_punt ("mdes_operand_latency: opcode (%i) not defined", opcode);
    }

  /* Make sure valid alt_no specified */
  if (alt_no >= op->num_alts)
    {
      I_punt ("mdes_operand_latency: alt (%i) >= number of alts (%i)\n",
	      alt_no, op->num_alts);
    }

  /* Make sure operand index is valid */
  if (operand_index >= lmdes->latency_count)
    I_punt ("mdes_operand_latency: operand_index (%i) out of bounds",
	    operand_index);



  /* Return the operand latency */
  latency = op->alt[alt_no].latency->operand_latency[operand_index];
  return (latency);
}

/* 
 * This routine is intended to give a "best" choice for an 
 * alternative id when an opcode has not been scheduled such
 * as when scheduling for infinite issue or for approximating
 * the "best" choice for instruction latencies.
 *
 * Chooses the alternative with the lowest mdes_max_completion_time
 * (latency).  In case of ties, the lowest alt number will be
 * chosen.
 */
int
mdes_heuristic_alt_id (int opcode)
{
  Mdes_Operation *op = NULL;
  int min_alt;
  int latency, min_latency;
  int alt;

  /* Make sure opcode is valid */
  if ((opcode > lmdes->max_opcode) ||
      ((op = lmdes->op_table[opcode]) == NULL))
    {
      I_punt ("mdes_heuristic_alt_id: opcode (%i) not defined", opcode);
    }

  /* If have not already selected the heuristic alt, do it now */
  if (op->heuristic_alt < 0)
    {
      /* Get the latency of the first alternative */
      min_alt = 0;
      min_latency = mdes_max_completion_time (opcode, min_alt);

      for (alt = 1; alt < op->num_alts; alt++)
	{
	  latency = mdes_max_completion_time (opcode, alt);
	  if (latency < min_latency)
	    {
	      min_latency = latency;
	      min_alt = alt;
	    }
	}

      /* Set heuristic alt to this "best" alternative */
      op->heuristic_alt = min_alt;
    }

  return (op->heuristic_alt);
}

int
mdes_max_completion_time (int opcode, int alt_no)
{
  Mdes_Operation *op = NULL;
  int i, max_time, num_dests, dest_offset, *lat;

  /* Make sure opcode is valid */
  if ((opcode > lmdes->max_opcode) ||
      ((op = lmdes->op_table[opcode]) == NULL))
    {
      I_punt ("mdes_max_completion_time: opcode (%i) not defined", opcode);
    }

  /* Make sure valid alt_no specified */
  if (alt_no >= op->num_alts)
    {
      I_punt ("mdes_max_completion_time: alt (%i) >= number of alts (%i)\n",
	      alt_no, op->num_alts);
    }

  max_time = 0;
  num_dests = lmdes->number[MDES_DEST];
  dest_offset = operand_index (MDES_DEST, 0);
  lat = op->alt[alt_no].latency->operand_latency;

  for (i = 0; i < num_dests; i++)
    {
      if (lat[dest_offset + i] > max_time)
	max_time = lat[dest_offset + i];
    }

  return (max_time);
}


/*
 * Loads the low level mdes file and points 'lmdes' at the
 * Mdes structure containing the loaded information.
 *
 * Punts if a .lmdes2 file is passed to it.  If this routine
 * is called instead of L_init_lmdes2(), it is assumed that the
 * module does not support .lmdes2.
 */
void
L_init_lmdes (char *mdes_file_name)
{
  int len;

  if (lmdes != NULL)
    fprintf (stderr, "John- You need to free the old lmdes file");

  /* Punt if file name ends in .lmdes2 */
  len = strlen (mdes_file_name);
  if ((len > 7) &&
      (mdes_file_name[len - 7] == '.') &&
      (mdes_file_name[len - 6] == 'l') &&
      (mdes_file_name[len - 5] == 'm') &&
      (mdes_file_name[len - 4] == 'd') &&
      (mdes_file_name[len - 3] == 'e') &&
      (mdes_file_name[len - 2] == 's') && (mdes_file_name[len - 1] == '2'))
    {
      I_punt ("L_init_lmdes: lmdes2 file unexpected (%s)\n\n"
	      "This modules either:\n"
	      "  1) Does not support .lmdes2 resource representation\n"
	      "  2) or didn't call L_init_lmdes2().\n\n"
	      "L_init_lmdes() is provided for backward compatibility.\n",
	      mdes_file_name);
    }
  else
    {
      lmdes = load_lmdes (mdes_file_name);
    }

  /* Create alloc pools for mdes */
  Mdes_Info_pool = L_create_alloc_pool ("Mdes_Info", sizeof (Mdes_Info), 4);
  Mdes_Compatable_Alt_pool =
    L_create_alloc_pool ("Mdes_Compatable_Alt",
			 sizeof (Mdes_Compatable_Alt), 4);
}

/*
 * Loads the low level mdes file and points 'lmdes' at the
 * Mdes structure containing the loaded information.
 *
 * Supports both the loading of .lmdes and .lmdes2 files.  Should
 * be called by routines that can tolerate both, or the routines
 * should check that the proper verson was passed to it (by checking the
 * lmdes->mdes2 pointer).
 */
void
L_init_lmdes2 (char *mdes_file_name, int num_pred, int num_dest,
	       int num_src, int num_sync)
{
  int len;

  if (lmdes != NULL)
    fprintf (stderr, "Currently old lmdes file is not freed (sorry).\n");

  /* Build from md description if file name ends in .lmdes2 */
  len = strlen (mdes_file_name);
  if ((len > 7) &&
      (mdes_file_name[len - 7] == '.') &&
      (mdes_file_name[len - 6] == 'l') &&
      (mdes_file_name[len - 5] == 'm') &&
      (mdes_file_name[len - 4] == 'd') &&
      (mdes_file_name[len - 3] == 'e') &&
      (mdes_file_name[len - 2] == 's') && (mdes_file_name[len - 1] == '2'))
    {
#if 0
      printf ("Building mdes version1 structures from version2 mdes!\n");
#endif
      lmdes = load_lmdes_from_version2 (mdes_file_name, num_pred, num_dest,
					num_src, num_sync);
    }
  else
    {
      lmdes = load_lmdes (mdes_file_name);
    }

  /* Create alloc pools for mdes */
  Mdes_Info_pool = L_create_alloc_pool ("Mdes_Info", sizeof (Mdes_Info), 4);
  Mdes_Compatable_Alt_pool =
    L_create_alloc_pool ("Mdes_Compatable_Alt",
			 sizeof (Mdes_Compatable_Alt), 4);
}


void
Malloc_struct (void **ptr, int size)
{
  if ((*ptr = (void *) malloc (size)) == NULL)
    I_punt ("Malloc_struct: Out of memory");
}

void
Malloc_name (char **ptr, char *name)
{
  int size;
  size = strlen (name) + 1;
  if ((*ptr = (char *) malloc (size)) == NULL)
    I_punt ("Malloc_name: Out of memory");

  strcpy (*ptr, name);
}

static void
LM_read_mask (FILE * in, int *mask, int width)
{
  int i;
  for (i = 0; i < width; i++)
    {
      if (fscanf (in, "%8x", &mask[i]) != 1)
	I_punt ("LM_read_mask: error reading mask");
    }
}

/*
 * reads low-level mdes file and builds data structures.
 */
static Mdes *
load_lmdes (char *file_name)
{
  Mdes *mdes;
  FILE *in;
  int version;
  int struct_size;
  int data_size;
  int set_table_size;
  int rused_size;
  int slots_size;
  int option_size;
  int alt_size;
  int buf_size;
  int *mask_ptr;
  int *array_ptr;
  int *slot_ptr;
  Mdes_IO_Set **type_ptr;
  Mdes_Rused *rused_ptr;
  Mdes_Rmask *option_ptr;
  Mdes_Alt *alt_ptr;
  int total_string_len;
  int total_used;
  int total_slots;
  int total_options;
  int total_alts;
  Mdes_IO_Set *IO_set;
  Mdes_IO_Item *IO_item;
  Mdes_Resource *resource;
  Mdes_ResList *reslist;
  Mdes_Rused *rused;
  Mdes_Rmask *option;
  Mdes_Latency *latency;
  Mdes_Alt *alt;
  Mdes_Operation *operation;
  char *name_buf_ptr;
  int IO_set_id, IO_item_id, reslist_id, latency_id;
  int i, j, k, m;

  /* Open the file for reading */
  if ((in = fopen (file_name, "r")) == NULL)
    I_punt ("load_lmdes: Unable to open mdes file '%s' for reading",
	    file_name);

  /* Malloc mdes structure and copy file name to field */
  Malloc_struct ((void **) &mdes, sizeof (Mdes));
  Malloc_name (&mdes->file_name, file_name);

  /* This is build from version1 file.  Mark by placing NULL in mdes2 */
  mdes->mdes2 = NULL;

  /* Read version */
  version = -1;
  if (fscanf (in, "Lmdes Version %i\n\n", &version) != 1)
    I_punt ("load_lmdes: '%s' is not a lmdes file", file_name);


  /* Make sure file format is what we expect */
  if (version != MDES_VERSION)
    {
      fprintf (stderr,
	       "load_lmdes: '%s' internal format is incompatable with this "
	       "reader.\n", file_name);
      fprintf (stderr,
	       "load_lmdes: It's internal version is %i not the expected %i.\n",
	       version, MDES_VERSION);

      /* Suggest a corrective action */
      if (version < MDES_VERSION)
	{
	  fprintf (stderr,
		   "load_lmdes: Please rebuild '%s' from the corresponding "
		   "hmdes file.\n", file_name);
	  I_punt ("load_lmdes: The hmdes file Version# does NOT "
		  "need to be modified.");
	}

      else
	{
	  I_punt ("load_lmdes: Please recompile this program with the new "
		  "lmdes library.");
	}
    }

  /* Read in other paramaters (mainly processor_model) */
  fscanf (in, "proc_model: %d\n", &mdes->processor_model);

  /* Read number of src, dest, etc. operands */
  fscanf (in, "sizes: %d %d %d %d %d %d %d\n\n",
	  &mdes->number[MDES_PRED], &mdes->number[MDES_DEST],
	  &mdes->number[MDES_SRC], &mdes->number[MDES_SYNC_IN],
	  &mdes->number[MDES_SYNC_OUT], &mdes->max_slot, &mdes->num_slots);


  /*
   * Get total operand count, latency count, and calculate offsets into
   * arrays for pred, dest, src, etc. operands
   */
  mdes->operand_count = mdes->number[MDES_PRED] + mdes->number[MDES_DEST] +
    mdes->number[MDES_SRC];

  mdes->latency_count = mdes->operand_count + mdes->number[MDES_SYNC_IN] +
    mdes->number[MDES_SYNC_OUT];

  mdes->offset[MDES_PRED] = 0;
  mdes->offset[MDES_DEST] = mdes->offset[MDES_PRED] + mdes->number[MDES_PRED];
  mdes->offset[MDES_SRC] = mdes->offset[MDES_DEST] + mdes->number[MDES_DEST];
  mdes->offset[MDES_SYNC_IN] = mdes->offset[MDES_SRC] +
    mdes->number[MDES_SRC];
  mdes->offset[MDES_SYNC_OUT] = mdes->offset[MDES_SYNC_IN] +
    mdes->number[MDES_SYNC_IN];

  /* Set names for each operand type, for error messages */
  mdes->name[MDES_PRED] = "pred";
  mdes->name[MDES_DEST] = "dest";
  mdes->name[MDES_SRC] = "src";
  mdes->name[MDES_SYNC_IN] = "sync_in";
  mdes->name[MDES_SYNC_OUT] = "sync_out";

  /* Malloc structures to allow reverse mapping from operand_index
   * to operand_type and operand_number.
   */
  Malloc_struct ((void **) &mdes->index_type,
		 mdes->latency_count * sizeof (int));
  Malloc_struct ((void **) &mdes->index_number,
		 mdes->latency_count * sizeof (int));

  /* Init reverse map structure for MDES_PRED */
  for (i = 0; i < mdes->number[MDES_PRED]; i++)
    {
      mdes->index_type[mdes->offset[MDES_PRED] + i] = MDES_PRED;
      mdes->index_number[mdes->offset[MDES_PRED] + i] = i;
    }

  /* Init reverse map structure for MDES_DEST */
  for (i = 0; i < mdes->number[MDES_DEST]; i++)
    {
      mdes->index_type[mdes->offset[MDES_DEST] + i] = MDES_DEST;
      mdes->index_number[mdes->offset[MDES_DEST] + i] = i;
    }

  /* Init reverse map structure for MDES_SRC */
  for (i = 0; i < mdes->number[MDES_SRC]; i++)
    {
      mdes->index_type[mdes->offset[MDES_SRC] + i] = MDES_SRC;
      mdes->index_number[mdes->offset[MDES_SRC] + i] = i;
    }

  /* Init reverse map structure for MDES_SYNC_IN */
  for (i = 0; i < mdes->number[MDES_SYNC_IN]; i++)
    {
      mdes->index_type[mdes->offset[MDES_SYNC_IN] + i] = MDES_SYNC_IN;
      mdes->index_number[mdes->offset[MDES_SYNC_IN] + i] = i;
    }

  /* Init reverse map structure for MDES_SYNC_OUT */
  for (i = 0; i < mdes->number[MDES_SYNC_OUT]; i++)
    {
      mdes->index_type[mdes->offset[MDES_SYNC_OUT] + i] = MDES_SYNC_OUT;
      mdes->index_number[mdes->offset[MDES_SYNC_OUT] + i] = i;
    }

  if (fscanf (in, "IO_Sets_begin %i %i %i %i %i %i\n", &mdes->IOmask_width,
	      &mdes->num_reg_files, &mdes->num_IO_sets,
	      &mdes->null_external_id,
	      &mdes->max_IO_set_id, &total_string_len) != 6)
    I_punt ("load_lmdes: error reading IO_Sets_begin");


  /* Allocate array of IO_Sets */
  struct_size = mdes->num_IO_sets * sizeof (Mdes_IO_Set);
  Malloc_struct ((void **) &mdes->IO_set, struct_size);

  /* Allocate array to map external_id's to IO_Sets */
  set_table_size = (mdes->max_IO_set_id + 1) * sizeof (Mdes_IO_Set *);
  Malloc_struct ((void **) &mdes->IO_set_table, set_table_size);

  /* Allocate array of IOmasks */
  data_size = mdes->num_IO_sets * (mdes->IOmask_width * sizeof (int));
  Malloc_struct ((void **) &mask_ptr, data_size);

  /* Allocate all the name buffer space in one shot */
  Malloc_struct ((void **) &name_buf_ptr, total_string_len);

  /* Initialize IO_Set_table to NULL pointers */
  for (i = 0; i <= mdes->max_IO_set_id; i++)
    mdes->IO_set_table[i] = NULL;

  /* Read in the IO_sets */
  for (i = 0; i < mdes->num_IO_sets; i++)
    {
      IO_set = &mdes->IO_set[i];
      IO_set->name = name_buf_ptr;
      IO_set->mask = mask_ptr;

      fscanf (in, "%i %i %s", &IO_set->id, &IO_set->external_id,
	      IO_set->name);

      /* Put in IO_Set_table */
      if (IO_set->external_id != -1)
	mdes->IO_set_table[IO_set->external_id] = IO_set;

      /* Read mask from file */
      LM_read_mask (in, IO_set->mask, mdes->IOmask_width);

      /* Move name buf pointer after terminator */
      name_buf_ptr += strlen (IO_set->name) + 1;

      /* Move mask pointer after this mask */
      mask_ptr += mdes->IOmask_width;

    }
  fscanf (in, "\nIO_Sets_end\n\n");

  if (fscanf (in, "IO_Items_begin %i %i\n", &mdes->num_IO_items,
	      &total_string_len) != 2)
    I_punt ("load_lmdes: error reading IO_Items_begin");

  /* Allocate array of IO_Items */
  struct_size = mdes->num_IO_items * sizeof (Mdes_IO_Item);
  Malloc_struct ((void **) &mdes->IO_item, struct_size);

  /* Allocate array of operand types */
  data_size = mdes->num_IO_items * mdes->operand_count *
    sizeof (Mdes_IO_Set *);
  Malloc_struct ((void **) &type_ptr, data_size);

  /* Allocate all the name buffer space in one shot */
  Malloc_struct ((void **) &name_buf_ptr, total_string_len);

  for (i = 0; i < mdes->num_IO_items; i++)
    {
      IO_item = &mdes->IO_item[i];
      IO_item->name = name_buf_ptr;
      IO_item->operand_type = type_ptr;

      fscanf (in, "%d %s", &IO_item->id, IO_item->name);

      for (j = 0; j < mdes->operand_count; j++)
	{
	  fscanf (in, "%d", &IO_set_id);

	  /* Get pointer to appropriate IO_Set */
	  IO_item->operand_type[j] = &mdes->IO_set[IO_set_id];
	}

      /* Move name buf pointer after terminator */
      name_buf_ptr += strlen (IO_item->name) + 1;

      /* Move type ptr to next operand type array */
      type_ptr += mdes->operand_count;
    }
  fscanf (in, "\nIO_Items_end\n\n");

  if (fscanf (in, "Resources_begin %i %i\n", &mdes->num_resources,
	      &total_string_len) != 2)
    I_punt ("load_lmdes: error reading Resources_begin");

  /* Allocate array of resources */
  struct_size = mdes->num_resources * sizeof (Mdes_Resource);
  Malloc_struct ((void **) &mdes->resource, struct_size);

  /* Allocate all the name buffer space in one shot */
  Malloc_struct ((void **) &name_buf_ptr, total_string_len);

  for (i = 0; i < mdes->num_resources; i++)
    {
      resource = &mdes->resource[i];
      resource->name = name_buf_ptr;

      fscanf (in, "%d %s\n", &resource->id, resource->name);

      /* Move name buf pointer after terminator */
      name_buf_ptr += strlen (resource->name) + 1;
    }
  fscanf (in, "\nResources_end\n\n");

  if (fscanf (in, "ResList_begin %d %d %d %d %d %d\n", &mdes->num_reslists,
	      &total_used, &total_slots, &total_options, &mdes->Rmask_width,
	      &total_string_len) != 6)
    I_punt ("load_lmdes: error reading ResList_begin");

  /* Allocate array of resources */
  struct_size = mdes->num_reslists * sizeof (Mdes_ResList);
  Malloc_struct ((void **) &mdes->reslist, struct_size);

  /* Allocate array of Rused structs */
  rused_size = total_used * sizeof (Mdes_Rused);
  Malloc_struct ((void **) &rused_ptr, rused_size);

  /* Allocate array of slot options */
  slots_size = total_slots * sizeof (int);
  Malloc_struct ((void **) &slot_ptr, slots_size);

  /* Allocate array of options */
  option_size = total_options * sizeof (Mdes_Rmask);
  Malloc_struct ((void **) &option_ptr, option_size);

  /* Allocate array of ints for uncond/pred fields of options */
  data_size = total_options * 2 * mdes->Rmask_width * sizeof (int);
  Malloc_struct ((void **) &array_ptr, data_size);

  /* Allocate all the name buffer space in one shot */
  Malloc_struct ((void **) &name_buf_ptr, total_string_len);

  for (i = 0; i < mdes->num_reslists; i++)
    {
      reslist = &mdes->reslist[i];
      reslist->name = name_buf_ptr;
      reslist->used = rused_ptr;
      reslist->slot_options = slot_ptr;

      fscanf (in, "%d %s %d %d %d\n", &reslist->id, reslist->name,
	      &reslist->num_used, &reslist->num_slot_options,
	      &reslist->num_RU_entries_required);

      /* Read in slot options */
      for (j = 0; j < reslist->num_slot_options; j++)
	{
	  fscanf (in, "%d", &reslist->slot_options[j]);
	}
      fscanf (in, "\n");

      for (j = 0; j < reslist->num_used; j++)
	{
	  rused = &reslist->used[j];
	  rused->option = option_ptr;

	  fscanf (in, "%d %d %d %d\n", &rused->start_usage,
		  &rused->end_usage, &rused->num_options, &rused->flags);

	  for (k = 0; k < rused->num_options; k++)
	    {
	      option = &rused->option[k];

	      /* Grab int arrays of size mdes->Rmask_width for uncond/pred */
	      option->uncond = array_ptr;
	      array_ptr += mdes->Rmask_width;
	      option->pred = array_ptr;
	      array_ptr += mdes->Rmask_width;

	      /* Read in uncond mask */
	      for (m = 0; m < mdes->Rmask_width; m++)
		{
		  fscanf (in, "%8x", &option->uncond[m]);
		}

	      /* Read in pred mask */
	      for (m = 0; m < mdes->Rmask_width; m++)
		{
		  fscanf (in, "%8x", &option->pred[m]);
		}
	    }
	  /* Move option pointer after this option array */
	  option_ptr += rused->num_options;
	}
      /* Move slot pointer after slot option array */
      slot_ptr += reslist->num_slot_options;

      /* Move name buf pointer after terminator */
      name_buf_ptr += strlen (reslist->name) + 1;

      /* Move rused pointer to free space */
      rused_ptr += reslist->num_used;
    }
  fscanf (in, "\nResList_end\n\n");

  if (fscanf (in, "Latencies_begin %i %i\n", &mdes->num_latencies,
	      &total_string_len) != 2)
    I_punt ("load_lmdes: error reading Latencies_begin");

  /* Allocate array of latencies */
  struct_size = mdes->num_latencies * sizeof (Mdes_Latency);
  Malloc_struct ((void **) &mdes->latency, struct_size);

  /* Allocate all operand_latency arrays in one shot */
  data_size = mdes->num_latencies * mdes->latency_count * sizeof (int);
  Malloc_struct ((void **) &array_ptr, data_size);

  /* Allocate all the name buffer space in one shot */
  Malloc_struct ((void **) &name_buf_ptr, total_string_len);

  for (i = 0; i < mdes->num_latencies; i++)
    {
      latency = &mdes->latency[i];
      latency->name = name_buf_ptr;
      latency->operand_latency = array_ptr;

      fscanf (in, "%d %s %d", &latency->id, latency->name,
	      &latency->exception);

      for (j = 0; j < mdes->latency_count; j++)
	{
	  fscanf (in, "%d", &latency->operand_latency[j]);
	}

      /* Move name buf pointer after terminator */
      name_buf_ptr += strlen (latency->name) + 1;

      /* Move operand latency array ptr to next array */
      array_ptr += mdes->latency_count;
    }
  fscanf (in, "\nLatencies_end\n\n");


  if (fscanf (in, "Operations_begin %i %i %i %i\n",
	      &mdes->num_operations, &total_alts,
	      &mdes->max_opcode, &total_string_len) != 4)
    I_punt ("load_lmdes: error reading Operations_begin");

  /* Allocate array of Operations */
  struct_size = mdes->num_operations * sizeof (Mdes_Operation);
  Malloc_struct ((void **) &mdes->operation, struct_size);

  /* Allocate all alts in one shot */
  alt_size = total_alts * sizeof (Mdes_Alt);
  Malloc_struct ((void **) &alt_ptr, alt_size);

  /* Allocate op_table (indexed by opcode) */
  data_size = (mdes->max_opcode + 1) * sizeof (Mdes_Operation *);
  Malloc_struct ((void **) &mdes->op_table, data_size);

  /* Allocate all the name buffer space in one shot */
  Malloc_struct ((void **) &name_buf_ptr, total_string_len);


  /* Intialize all op_table pointers to NULL */
  for (i = 0; i <= mdes->max_opcode; i++)
    mdes->op_table[i] = NULL;

  /* Read in operations */
  for (i = 0; i < mdes->num_operations; i++)
    {
      operation = &mdes->operation[i];
      operation->external_name = name_buf_ptr;
      operation->alt = alt_ptr;
      operation->id = i;
      operation->heuristic_alt = -1;	/* Used for heuristics */

      fscanf (in, "%d %s %d %x\n", &operation->opcode,
	      operation->external_name,
	      &operation->num_alts, &operation->op_flags);

      /* Insert into op_table */
      mdes->op_table[operation->opcode] = operation;

      /* Move name buf pointer after terminator */
      name_buf_ptr += strlen (operation->external_name) + 1;

      /* Read in alts for operation */
      for (j = 0; j < operation->num_alts; j++)
	{
	  alt = &operation->alt[j];
	  alt->id = j;
	  alt->asm_name = name_buf_ptr;
	  alt->operation = operation;

	  fscanf (in, "%s %x %d %d %d\n", alt->asm_name,
		  &alt->alt_flags, &IO_item_id, &reslist_id, &latency_id);

	  /* Point at appropriate structures using ids */
	  alt->IO_item = &mdes->IO_item[IO_item_id];
	  alt->reslist = &mdes->reslist[reslist_id];
	  alt->table = NULL;	/* Since loading .lmdes not .lmdes2 */
	  alt->latency = &mdes->latency[latency_id];

	  /* Move name buf pointer after terminator */
	  name_buf_ptr += strlen (alt->asm_name) + 1;
	}

      /* Move alt ptr to next alt array */
      alt_ptr += operation->num_alts;
    }
  fscanf (in, "\nOperations_end\n\n");

  /* Allocate temporary buffers used by Mdes routines */
  buf_size = mdes->operand_count * sizeof (Mdes_IO_Set *);
  Malloc_struct ((void **) &mdes->operand_type_buf, buf_size);

  /* Initialize mdes2 parameters to their implicit values for mdes1 */
  mdes->check_resources_for_only_one_alt = 0;

  return (mdes);
}

/*
 * Writes mdes file to out in a format simular to the low-level format 
 */
void
print_mdes (FILE * out, Mdes * mdes)
{
  Mdes_IO_Set *IO_set;
  Mdes_IO_Item *IO_item;
  Mdes_Resource *resource;
  Mdes_ResList *reslist;
  Mdes_Rused *rused;
  Mdes_Rmask *option;
  Mdes_Latency *latency;
  Mdes_Alt *alt;
  Mdes_Operation *operation;
  int i, j, k, m;

  /* Debug, print out internal data structures contents  */
  fprintf (out, "\nSizes  : ");
  fprintf (out, "%i %i %i %i %i\n",
	   mdes->number[MDES_PRED], mdes->number[MDES_DEST],
	   mdes->number[MDES_SRC], mdes->number[MDES_SYNC_IN],
	   mdes->number[MDES_SYNC_OUT]);

  fprintf (out, "\nOffsets: ");
  fprintf (out, "%i %i %i %i %i\n",
	   mdes->offset[MDES_PRED], mdes->offset[MDES_DEST],
	   mdes->offset[MDES_SRC], mdes->offset[MDES_SYNC_IN],
	   mdes->offset[MDES_SYNC_OUT]);

  fprintf (out, "\nIO_sets:\n");
  for (i = 0; i < mdes->num_IO_sets; i++)
    {
      IO_set = &mdes->IO_set[i];
      fprintf (out, "%2i %2i %-11s ", IO_set->id, IO_set->external_id,
	       IO_set->name);
      for (j = 0; j < mdes->IOmask_width; j++)
	fprintf (out, "%08x ", IO_set->mask[j]);
      fprintf (out, "\n");
    }

  fprintf (out, "\nIO_Items:\n");
  for (i = 0; i < mdes->num_IO_items; i++)
    {
      IO_item = &mdes->IO_item[i];
      fprintf (out, "%2i %-11s ", IO_item->id, IO_item->name);
      for (j = 0; j < mdes->operand_count; j++)
	fprintf (out, "%2i ", IO_item->operand_type[j]->id);
      fprintf (out, "\n");
    }

  fprintf (out, "\nResources:\n");
  for (i = 0; i < mdes->num_resources; i++)
    {
      resource = &mdes->resource[i];
      fprintf (out, "%2i %-11s\n", resource->id, resource->name);
    }

  fprintf (out, "\nResList:\n");
  for (i = 0; i < mdes->num_reslists; i++)
    {
      reslist = &mdes->reslist[i];

      fprintf (out, "%2i %-11s %2i %2i %2i\n", reslist->id, reslist->name,
	       reslist->num_used, reslist->num_slot_options,
	       reslist->num_RU_entries_required);

      fprintf (out, "  ");
      for (j = 0; j < reslist->num_slot_options; j++)
	{
	  fprintf (out, "%2i ", reslist->slot_options[j]);
	}
      fprintf (out, "\n");

      for (j = 0; j < reslist->num_used; j++)
	{
	  rused = &reslist->used[j];

	  fprintf (out, "  %2i %2i %2i\n", rused->start_usage,
		   rused->end_usage, rused->num_options);

	  for (k = 0; k < rused->num_options; k++)
	    {
	      option = &rused->option[k];

	      fprintf (out, "    ");
	      for (m = 0; m < mdes->Rmask_width; m++)
		{
		  fprintf (out, "%08x ", option->uncond[m]);
		}
	      fprintf (out, "  ");
	      for (m = 0; m < mdes->Rmask_width; m++)
		{
		  fprintf (out, "%08x ", option->pred[m]);
		}
	      fprintf (out, "\n");
	    }
	}
    }

  fprintf (out, "\nLatencies:\n");
  for (i = 0; i < mdes->num_latencies; i++)
    {
      latency = &mdes->latency[i];
      fprintf (out, "%2i %-11s %2i ", latency->id, latency->name,
	       latency->exception);

      for (j = 0; j < mdes->latency_count; j++)
	{
	  fprintf (out, "%2i ", latency->operand_latency[j]);
	}
      fprintf (out, "\n");
    }

  fprintf (out, "\nOperations:\n");
  for (i = 0; i < mdes->num_operations; i++)
    {
      operation = &mdes->operation[i];

      fprintf (out, "%3i %-11s %2i %08x\n", operation->opcode,
	       operation->external_name, operation->num_alts,
	       operation->op_flags);
      for (j = 0; j < operation->num_alts; j++)
	{
	  alt = &operation->alt[j];

	  fprintf (out, "   %-11s %08x  %2i %2i %2i\n",
		   alt->asm_name, alt->alt_flags, alt->IO_item->id,
		   alt->reslist->id, alt->latency->id);
	}
    }

  fprintf (out, "\nOperations sorted by opcode (testing mdes->op_table):\n");
  for (i = 0; i <= mdes->max_opcode; i++)
    {
      if (mdes->op_table[i] != NULL)
	fprintf (out, "%4i %-11s\n", mdes->op_table[i]->opcode,
		 mdes->op_table[i]->external_name);
    }
}
