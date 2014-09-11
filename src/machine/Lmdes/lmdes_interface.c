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
/******************************************************************************\
 *  File:  lmdes_interface.c
 *
 *  Description:
 *    Lcode interface functions to lmdes
 *
 *  Creation Date :  June, 1993
 *
 *  Authors:  John C. Gyllenhaal, Wen-mei Hwu
 *
 *
\******************************************************************************/
/* 10/29/02 REK Adding config.h */
#include <config.h>
#include <library/i_error.h>
#include <Lcode/l_main.h>
#include "lmdes.h"
#include "lmdes_interface.h"

/* Global variables that the build mdes_info routines use */
static int _build_mdes_info_initialized = 0;
static int **_io_list_dest = NULL;
static int **_io_list_src = NULL;
static int **_io_list_pred = NULL;
static int *_io_list = NULL;


static int (*mdes_get_operand_type_fptr) (L_Operand * operand);

static void
L_initialize_build_mdes_info (void)
{
  int dest_size, src_size, pred_size, list_size;
  int num_dest, num_src, num_pred;
  int num_operands;
  int i;

  /* Get the number of each operand type that mdes expects */
  num_dest = mdes_num_operands (MDES_DEST);
  num_src = mdes_num_operands (MDES_SRC);
  num_pred = mdes_num_operands (MDES_PRED);
  num_operands = num_dest + num_src + num_pred;


  /* Make sure Mcode has as least as many operands as mdes */
  if (L_max_dest_operand < num_dest)
    I_punt ("L_build_cb_mdes_info: mdes dests > mcode dests");
  if (L_max_src_operand < num_src)
    I_punt ("L_build_cb_mdes_info: mdes srcs > mcode src");
  /* Only 1 predicate for now - RAB 
   * Changed back to using L_max_pred_operand instead of 1-JCG 7/22/96 
   */
  if (L_max_pred_operand < num_pred)
    I_punt ("L_build_cb_mdes_info: mdes preds > mcode preds");

  dest_size = (L_max_dest_operand + 1) * sizeof (int *);
  src_size = (L_max_src_operand + 1) * sizeof (int *);
  /* only 1 predicate for now - RAB */
  pred_size = (1 /* L_max_pred_operand */  + 1) * sizeof (int *);
  list_size = num_operands * sizeof (int);

  if (((_io_list_dest = (int **) malloc (dest_size)) == NULL) ||
      ((_io_list_src = (int **) malloc (src_size)) == NULL) ||
      ((_io_list_pred = (int **) malloc (pred_size)) == NULL) ||
      ((_io_list = (int *) malloc (list_size)) == NULL))
    {
      I_punt ("L_build_cb_mdes_info: Out of memory");
    }

  /*
   * Point each dest/src/pred at the propriate place in the io_list
   * Initialize the unused entries to NULL (for debugging purposes)
   */
  for (i = 0; i < num_dest; i++)
    _io_list_dest[i] = &_io_list[operand_index (MDES_DEST, i)];
  for (; i <= L_max_dest_operand; i++)
    _io_list_dest[i] = NULL;

  for (i = 0; i < num_src; i++)
    _io_list_src[i] = &_io_list[operand_index (MDES_SRC, i)];
  for (; i <= L_max_src_operand; i++)
    _io_list_src[i] = NULL;

  for (i = 0; i < num_pred; i++)
    _io_list_pred[i] = &_io_list[operand_index (MDES_PRED, i)];

  /* only 1 predicate for now - rab */
  for (; i <= 1 /* L_max_pred_operand */ ; i++)
    _io_list_pred[i] = NULL;

  /* Get the function from mspec that maps lcode operands to mdes ids */
  mdes_get_operand_type_fptr = M_mdes_operand_type ();

  /* Flag that we have initilized everything properly */
  _build_mdes_info_initialized = 1;
}


static void
print_mdes_info_debug_info (L_Oper * op, int opcode, char *func_name)
{
  int num_dest, num_src, num_pred;
  int i;

  fprintf (stderr, "  Func %s, op %i, opcode %i ",
	   L_fn->name, op->id, opcode);

  /* Print out opcode name if valid opcode */
  if ((opcode > lmdes->max_opcode) ||
      (opcode < 0) || (lmdes->op_table[opcode] == NULL))
    {
      fprintf (stderr, "(undefined opcode):\n");
    }
  else
    {
      fprintf (stderr, "(%s):\n", lmdes->op_table[opcode]->external_name);
    }

  num_dest = mdes_num_operands (MDES_DEST);
  num_src = mdes_num_operands (MDES_SRC);
  num_pred = mdes_num_operands (MDES_PRED);

  fprintf (stderr, "    Dests: ");
  for (i = 0; i < num_dest; i++)
    mdes_print_IO_set (stderr, *_io_list_dest[i]);
  fprintf (stderr, "\n");

  fprintf (stderr, "    Srcs:  ");
  for (i = 0; i < num_src; i++)
    mdes_print_IO_set (stderr, *_io_list_src[i]);
  fprintf (stderr, "\n");

  fprintf (stderr, "    Preds: ");
  for (i = 0; i < num_pred; i++)
    mdes_print_IO_set (stderr, *_io_list_pred[i]);
  fprintf (stderr, "\n");
  I_punt ("%s cannot continue.", func_name);
}

/* Build the io list for a op, using the function ptr that mspec returned
 * for assigning operands ids.
 */
static int
mdes_build_iolist (L_Oper * op, int **dest, int **src, int **pred)
{
  int i;
  int num_dest, num_src, num_pred;
  int too_many_operands;

  /* Assume that there are not too many operands */
  too_many_operands = 0;

  /* Get number of mdes operands for ease of use */
  num_dest = mdes_num_operands (MDES_DEST);
  num_src = mdes_num_operands (MDES_SRC);
  num_pred = mdes_num_operands (MDES_PRED);

  /* Get each operand types and make sure it doesn't define more
   * than the mdes allows
   */
  for (i = 0; i < num_dest; i++)
    *dest[i] = mdes_get_operand_type_fptr (op->dest[i]);

  for (; i < L_max_dest_operand; i++)
    {
      if (op->dest[i] != NULL)
	{
	  I_punt ("mdes_build_io_list: op %i dest[%i] out of bounds (%i "
		  "defined in mdes.)", op->id, i, num_dest);
	}
    }


  for (i = 0; i < num_src; i++)
    *src[i] = mdes_get_operand_type_fptr (op->src[i]);

  for (; i < L_max_src_operand; i++)
    {
      if (op->src[i] != NULL)
	{
	  I_punt ("mdes_build_io_list: op %i src[%i] out of bounds (%i "
		  "defined in mdes.)", op->id, i, num_src);
	}
    }

  /* Only pred[0] is a real operand, the rest are for informational
   * purposes only.
   */
  if (num_pred > 0)
    {
      *pred[0] = mdes_get_operand_type_fptr (op->pred[0]);

      for (i = 1; i < num_pred; i++)
	*pred[i] = mdes_get_operand_type_fptr (NULL);
    }

  return op->proc_opc;
}


/*
 * Mcode interface to mdes.  Builds the mdes_info structure for
 * each op in the cb.
 * A Mspec function is called to get the mdes operand type for each operand
 */
void
L_build_cb_mdes_info (L_Cb * cb)
{
  int num_operands, null_operand;
  int i;
  L_Oper *op;
  int opcode;

  /* If this is the first time in this function, initialize everything */
  if (!_build_mdes_info_initialized)
    {
      L_initialize_build_mdes_info ();
    }

  /* Get total number of operands (length of io_list) */
  num_operands = mdes_operand_count ();
  null_operand = mdes_null_operand ();

  /* Build io_list and mdes_info for each oper in cb */
  for (op = cb->first_op; op != NULL; op = op->next_op)
    {
      /* Intialize all the operands in the io_list to NULL */
      for (i = 0; i < num_operands; i++)
	_io_list[i] = null_operand;

      /* Build the io_list for the oper (dest, src, and pred contain
       * pointers to the io_list.  This function also returns
       * the opcode to use for this op.
       */
      opcode = mdes_build_iolist (op, _io_list_dest, _io_list_src,
				  _io_list_pred);

      /* Warn if op->mdse_info is not NULL */
      if (op->mdes_info != NULL)
	L_warn ("fn %s op %i: free mdes_info before building new mdes_info",
		L_fn->name, op->id);

      /* Build the mdes_info for the oper */
      op->mdes_info = build_mdes_info (opcode, _io_list);

      /* Print error message if could not build mdes info */
      if (op->mdes_info == NULL)
	{
	  print_mdes_info_debug_info (op, opcode, "L_build_cb_mdes_info");
	}
    }
}

/*
 * Mcode interface to mdes.  Builds the mdes_info structure for
 * an oper.
 * A Mspec function is called to get the mdes operand type for each operand
 */
void
L_build_oper_mdes_info (L_Oper * op)
{
  int num_operands, null_operand;
  int i;
  int opcode;

  if (op == NULL)
    I_punt ("L_build_oper_mdes_info: NULL op passed");

  /* If this is the first time in this function, initialize everything */
  if (!_build_mdes_info_initialized)
    {
      L_initialize_build_mdes_info ();
    }

  /* Get total number of operands (length of io_list) */
  num_operands = mdes_operand_count ();
  null_operand = mdes_null_operand ();

  /* Intialize all the operands in the io_list to NULL */
  for (i = 0; i < num_operands; i++)
    _io_list[i] = null_operand;

  /* Build the io_list for the oper (dest, src, and pred contain
   * pointers to the io_list.  This function also returns
   * the opcode to use for this op.
   */
  opcode = mdes_build_iolist (op, _io_list_dest, _io_list_src, _io_list_pred);

  /* Warn if op->mdse_info is not NULL */
  if (op->mdes_info != NULL)
    L_warn ("fn %s op %i: free mdes_info before building new mdes_info",
	    L_fn->name, op->id);

  /* Build the mdes_info for the oper */
  op->mdes_info = build_mdes_info (opcode, _io_list);

  /* Print error message if could not build mdes info */
  if (op->mdes_info == NULL)
    {
      print_mdes_info_debug_info (op, opcode, "L_build_cb_mdes_info");
    }
}


/*
 * frees all the mdes_info's allocated for a cb
 */
void
L_free_cb_mdes_info (L_Cb * cb)
{
  L_Oper *op;

  /* Free the mdes_info for each oper in cb and set field to NULL */
  for (op = cb->first_op; op != NULL; op = op->next_op)
    {
      if (op->mdes_info != NULL)
	{
	  free_mdes_info (op->mdes_info);
	  op->mdes_info = NULL;
	}
    }
}

/*
 * frees the oper's mdes info if allocated
 */
void
L_free_oper_mdes_info (L_Oper * op)
{
  if (op == NULL)
    I_punt ("L_free_oper_mdes_info: NULL op passed");

  if (op->mdes_info != NULL)
    {
      free_mdes_info (op->mdes_info);
      op->mdes_info = NULL;
    }
}
