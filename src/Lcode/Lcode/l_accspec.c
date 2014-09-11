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
 *      File :          l_accspec.c
 *      Description :   Lcode access specifiers
 *      Creation Date : May 2004
 *      Author :        John W. Sias, Wen-mei Hwu
 *
 *==========================================================================*/

#include <config.h>
#include "l_main.h"
#include "l_accspec.h"
#include "l_sync.h"
#include "Lcode/l_build_prototype_info.h"

/*! \brief Analyze access specifiers for memory operation independence.
 *
 * \param op1
 *  First list
 * \param op2
 *  Second list
 * \param have_def
 *  Flag indicating whether one of the AccSpecs must be a def.
 *
 * \return
 *  1 - lists are independent
 *  0 - list are dependent
 *
 * \note This function assumes L_func_acc_specs = 1, does not use any
 * omega information.
 *
 * This function is used on separate lists to determine potential alias.
 * Unlike the function below, it does not check for omega-test dependences.
 * PCE is currently the only user of this check, and is not able to use
 * more descriptive access information.
 */
int
L_mem_indep_acc_specs_lists (L_AccSpec *as1, L_AccSpec *as2)
{
  L_AccSpec *cas1, *cas2;

  if (!as1 || !as2)
    return 1;

  for (cas1 = as1; cas1; cas1 = cas1->next)
    for (cas2 = as2; cas2; cas2 = cas2->next)
      {
	if (!(cas1->id == cas2->id && cas1->version == cas2->version))
	  continue;

	return 0;
      }
  return 1;
}



/*! \brief Analyze access specifiers and acc-omega-based sync arcs, if
 * present, for memory operation independence.
 *
 * \param op1
 *  First operation
 * \param op2
 *  Second operation
 * \param dep_flags
 *  Dependence flags with which to filter acc-omega-based sync arcs
 *
 * \return
 *  2 - operations are independent (by sync arcs)
 *  1 - operations are independent (by acc specs)
 *  0 - operations are dependent
 *
 * \note This function assumes L_func_acc_specs = 1.  If
 * L_func_acc_omega = 1, any available sync arcs are used to refine
 * dependence information.  \sa L_analyze_syncs().
 *
 * Note that the notion of "dependence" here is really more like "alias";
 * it is bidirectional.  This matches (perhaps unfortunately) the semantics
 * of \sa L_analyze_syncs().  \sa L_mem_indep_acc_specs_cross() has the
 * expected directional semantics iff omega-bearing sync arcs are present.
 */
int
L_mem_indep_acc_specs (L_Oper *op1, L_Oper *op2, int dep_flags)
{
  L_AccSpec *as1, *as2, *cas1, *cas2;

  /*
   * ntclark 7/25/06 
   *
   * If a function returns a struct by value, it needs to show up as
   * being dependent on stack loads to the stack space written by the
   * function. I'm making JSRs returning structs dependent on all
   * memory ops as a conservative fix.
   */

  char return_type_buf[TYPE_BUF_SIZE];
  L_Attr *call_info;
  if(L_subroutine_call_opcode(op1)) {
    call_info = L_find_attr(op1->attr, "call_info");

    L_get_call_info (NULL, op1, call_info, return_type_buf, NULL,
		     sizeof(return_type_buf));

    if (L_convert_type_to_ctype (return_type_buf) == CTYPE_STRUCT) {
      return 0;
    }
  }
  if(L_subroutine_call_opcode(op2)) {
    char return_type_buf[TYPE_BUF_SIZE];
    call_info = L_find_attr(op2->attr, "call_info");

    L_get_call_info (NULL, op2, call_info, return_type_buf, NULL,
		     sizeof(return_type_buf));

    if (L_convert_type_to_ctype (return_type_buf) == CTYPE_STRUCT) {
      return 0;
    }
  }

  if (!(as1 = op1->acc_info) || !(as2 = op2->acc_info))
    return 1;

  for (cas1 = as1; cas1; cas1 = cas1->next)
    for (cas2 = as2; cas2; cas2 = cas2->next)
      {
	if ((cas1->id != cas2->id) || (cas1->version != cas2->version))
	  continue;

	/* Access to same object */

	if (cas1->size == -1 || cas2->size == -1)
	  {
	    /* Sizes non-descriptive */
	    return (L_func_acc_omega &&
		    (L_analyze_syncs (op1, op2, dep_flags) == 1)) ?
	      2 : 0;
	  }
	else
	  {
	    int o1 = cas1->offset, o2 = cas2->offset,
	      s1 = cas1->size, s2 = cas2->size;

	    /* Determine if accesses overlap */
	    if ((o1 <= o2 && (o1 + s1) > o2) ||
		(o2 < o1 && (o2 + s2) > o1))
	      return (L_func_acc_omega &&
		      (L_analyze_syncs (op1, op2, dep_flags) == 1)) ?
		2 : 0;
	  }
      }

  /* No relevant acc specs --- ops must be independent! */

  return 1;
}


/*! \brief Analyze access specifiers and acc-omega-based sync arcs, if
 * present, for cross-iteration dependence.
 *
 * \param op1
 *  First operation
 * \param op2
 *  Second operation
 * \param dep_flags
 *  Dependence flags with which to filter acc-omega-based sync arcs
 * \param forward
 *  Flag indicating that \a to_oper appears after \a from_oper in the cb.  
 * \param distance
 *  Pointer to integer to hold the dependence distance result, if found.
 *
 * \return
 *  2 - operations are independent (by sync arcs)
 *  1 - operations are independent (by acc specs)
 *  0 - operations are dependent
 *
 * \note This function assumes L_func_acc_specs = 1.  If
 * L_func_acc_omega = 1, any available sync arcs are used to refine
 * dependence information.  \sa L_analyze_syncs_cross().  
 */
int
L_mem_indep_acc_specs_cross (L_Oper *op1, L_Oper *op2, int dep_flags,
			     int forward, int *distance)
{
  L_AccSpec *as1, *as2, *cas1, *cas2;

  if (!(as1 = op1->acc_info) || !(as2 = op2->acc_info))
    return 1;

  for (cas1 = as1; cas1; cas1 = cas1->next)
    for (cas2 = as2; cas2; cas2 = cas2->next)
      {
	if ((cas1->id != cas2->id) || (cas1->version != cas2->version))
	  continue;

	/* Access to same object */

	if (cas1->size == -1 || cas2->size == -1)
	  {
	    /* Sizes non-descriptive */
	    return (L_func_acc_omega &&
		    (L_analyze_syncs_cross (op1, op2, dep_flags,
					    forward, distance) == 1)) ?
	      2 : 0;
	  }
	else
	  {
	    int o1 = cas1->offset, o2 = cas2->offset,
	      s1 = cas1->size, s2 = cas2->size;

	    /* Determine if accesses overlap */
	    if ((o1 <= o2 && (o1 + s1) > o2) ||
		(o2 < o1 && (o2 + s2) > o1))
	      return (L_func_acc_omega &&
		      (L_analyze_syncs_cross (op1, op2, dep_flags,
					      forward, distance) == 1)) ?
		2 : 0;
	  }
      }

  /* No relevant acc specs --- ops must be independent! */

  return 1;
}


/*! \brief Copy an access specifier list.
 *
 * \param mas
 *  Pointer to head of list to be copied
 *
 * \return
 *  Copy of the access specifier list.
 */
L_AccSpec *
L_copy_mem_acc_spec_list (L_AccSpec *mas)
{
  L_AccSpec *first = NULL, *last = NULL, *curr;

  while (mas)
    {
      curr = L_copy_mem_acc_spec (mas);
      curr->next = NULL;
      if (last)
	last->next = curr;
      else
	first = curr;
      last = curr;
      mas = mas->next;
    }

  return first;
}


/*! \brief Copy an access specifier list, changing uses to defs.
 *
 * \param mas
 *  Pointer to head of list to be copied
 *
 * \return
 *  Copy of the access specifier list with uses changed to defs.
 */
L_AccSpec *
L_copy_mem_acc_spec_list_as_def (L_AccSpec *mas)
{
  L_AccSpec *first = NULL, *last = NULL, *curr;

  while (mas)
    {
      curr = L_copy_mem_acc_spec (mas);
      curr->is_def = 1;
      curr->next = NULL;
      if (last)
	last->next = curr;
      else
	first = curr;
      last = curr;
      mas = mas->next;
    }

  return first;
}


/*! \brief Copy an access specifier list, changing defs to uses.
 *
 * \param mas
 *  Pointer to head of list to be copied
 *
 * \return
 *  Copy of the access specifier list with defs changed to uses.
 */
L_AccSpec *
L_copy_mem_acc_spec_list_as_use (L_AccSpec *mas)
{
  L_AccSpec *first = NULL, *last = NULL, *curr;

  while (mas)
    {
      curr = L_copy_mem_acc_spec (mas);
      curr->is_def = 0;
      curr->next = NULL;
      if (last)
	last->next = curr;
      else
	first = curr;
      last = curr;
      mas = mas->next;
    }

  return first;
}


/*! \brief Augment an operation's access spec list with the access specs of
 *   another operation.  Redundant access specs are suppressed.
 *
 * \param op_to
 *  Operation receiving copied access specs
 *
 * \param op_fr
 *  Operation donating access specs (this op's list will not be modified)
 */
void
L_merge_acc_spec_list (L_Oper *op_to, L_Oper *op_fr)
{
  L_AccSpec *mas_to, *mas_fr, *mas_tail = NULL;

  for (mas_fr = op_fr->acc_info; mas_fr; mas_fr = mas_fr->next)
    {
      for (mas_to = op_to->acc_info; mas_to; mas_to = mas_to->next)
	{
	  mas_tail = mas_to;

	  if (mas_to->id != mas_fr->id)
	    continue;
  
	  if (mas_to->version != mas_fr->version)
	    continue;

	  if (mas_to->offset != mas_fr->offset)
	    continue;

	  if ((mas_to->size != -1) &&
	      mas_to->size != mas_fr->size)
	    continue;

	  break;
	}

      if (!mas_to)
	{
	  /* Need to copy */
	  L_AccSpec *mas_new;

	  mas_new = L_copy_mem_acc_spec (mas_fr);

	  if (mas_tail)
	    mas_tail->next = mas_new;
	  else
	    op_to->acc_info = mas_new;
	}
    }
  return;
}
