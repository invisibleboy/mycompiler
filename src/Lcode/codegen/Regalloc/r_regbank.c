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
 *
 *      File :          r_regbank.c
 *      Description :   Register bank description functions
 *      Creation Date : February 1994
 *      Author :        Richard Hank, Wen-mei Hwu
 *
 * Revision 1.5  95/01/10  17:44:06  17:44:06  hank (Richard E. Hank)
 * Several minor bugs in the manipulation of the available and
 * used register stacks have been fixed.
 *
 * Revision 1.1  94/03/16  20:53:40  20:53:40  hank (Richard E. Hank)
 * Initial revision
 *
 *
 *===========================================================================*/
/*===========================================================================*/

/* 10/29/02 REK Adding config.h */
#include <config.h>
#include "r_regalloc.h"

#ifdef __cplusplus
extern "C"
{
#endif

  extern int Lsched_loads_per_cycle (void);

#ifdef __cplusplus
}
#endif

/*===========================================================================
 *
 *      Func :   R_define_physical_bank() 
 *      Desc :   Provides register allocator with description of
 *               available processor registers.
 *	Input:	 see r_regalloc.h
 *	Output:	 none
 *
 *	Side Effects: allocates space for all register banks, global <R_bank>
 *		      and maps, global <R_map>
 *
 *===========================================================================*/

void
R_define_physical_bank_with_rot (int rclass, int type, int num_reg,
				 int reg_size, int overlap,
				 int *reg_array, Set * used,
				 int num_rot_reg, int first_rot_reg,
				 int num_rot_reg_alloc)
{
  int i, n_reg, *map, mask;
  int overlap_bank = overlap;
  R_Physical_Bank *bank = NULL;

  if (R_bank == NULL)
    {
      R_bank = CALLOC (R_Physical_Bank, R_MAX_BANK);
      if (R_bank == NULL)
	L_punt ("Register Allocation: unable to allocate register banks");
      R_map = CALLOC (int *, R_MAX_BANK);
      if (R_map == NULL)
	L_punt ("Register Allocation: unable to allocate register maps");
    }

  switch (rclass)
    {
    case R_CALLER:
    case R_CALLEE:
    case R_MACRO_CALLER:
    case R_MACRO_CALLEE:
      switch (type)
	{
	  /* make sure the register bank overlaps itself */
	case R_PREDICATE:
	  overlap_bank |= R_OVERLAP_PREDICATE;
	  break;
	case R_INT:
	  overlap_bank |= R_OVERLAP_INT;
	  break;
	case R_FLOAT:
	  overlap_bank |= R_OVERLAP_FLOAT;
	  break;
	case R_DOUBLE:
	  overlap_bank |= R_OVERLAP_DOUBLE;
	  break;
	case R_BTR:
	  overlap_bank |= R_OVERLAP_BTR;
	  break;
	case R_QUAD:
	  overlap_bank |= R_OVERLAP_QUAD;
	  break;
	case R_POINTER:
	  overlap_bank |= R_OVERLAP_POINTER;
	  break;
	default:
	  L_punt ("Register Allocation: invalid register bank type: %d",
		  type);
	  break;
	}
      bank = R_bank + rclass + type;
      map = R_map[type + rclass];
      break;
    default:
      L_punt ("Register Allocation: invalid register bank rclass: %d",
	      rclass);
      break;
    }

  if (used == NULL)
    L_punt ("Register Allocation: "
	    "NULL pointer passed for register usage set!");

  bank->defined = 1;
  bank->rclass = rclass;
  bank->type = type;
  bank->num_reg = num_reg;
  bank->num_rot_reg = num_rot_reg;
  bank->rot_reg_ofs = first_rot_reg;
  bank->num_rot_reg_alloc = num_rot_reg_alloc;
  bank->reg_size = reg_size;
  bank->overlap = overlap_bank;
  bank->used_reg = used;
  bank->base_index = 0x7FFFFFFF;

  bank->avail_reg = NULL;
  bank->alloc_reg = NULL;

  mask = 0;
  for (i = 0; i < reg_size - 1; i++)
    mask = mask | 1 << i;
  bank->mask = ~mask;

  /* Initialize internal fields, these are redundant in the current impl. */
  bank->max = num_reg * reg_size;
  bank->res_inc = 1;

  n_reg = bank->num_reg * bank->reg_size;
  if (n_reg)
    {
      map = (int *) malloc (sizeof (int) * n_reg);
      for (i = 0; i < n_reg; i++)
	map[i] = reg_array[i];
    }
  else
    {
      map = NULL;
    }

  /* Free any previously existing bank definition */
  if (R_map[type + rclass] != NULL)
    free (R_map[type + rclass]);

  R_map[type + rclass] = map;
}

static void
R_invalid_bank_overlap (R_Physical_Bank * bank, int type)
{
  fprintf (stdout, "The %s,%s register bank as defined overlaps with\n",
	   BANK_NAME (bank->type), CLASS_NAME (bank->rclass));
  fprintf (stdout, "a %s,%s register bank which does not exist.\n",
	   BANK_NAME (type), CLASS_NAME (bank->rclass));
  fprintf (stdout, "Please examine your register bank definitions.\n");

  exit (-1);
}

void
R_define_physical_bank (int rclass, int type, int num_reg, int reg_size,
			int overlap, int *reg_array, Set * used)
{
  R_define_physical_bank_with_rot (rclass, type, num_reg, reg_size,
				   overlap, reg_array, used, 0, 0, 0);
}

/*===========================================================================
 *
 *      Func :  R_analyze_bank_overlap()
 *      Desc :  Analyzes the defined register banks and assigns overlap
 *		indices based on the overlap information provided
 *      Input:  
 *      Output: 
 *      Side Effects: assigns values to <base_index> for each register bank
 *
 *      Added loads_per_cycle parameter to eliminate call-back to
 *      old scheduler. -JCG 6/99
 *===========================================================================*/
void
R_analyze_bank_overlap (void)
{
  int i, j, base_index = 0, max_size;
  R_Physical_Bank *bank, *overlap_bank;
  Stack *stack;

  /* Reset register bank stacks and base indices */
  for (i = 0; i < R_MAX_BANK; i++)
    {
      bank = R_bank + i;
      bank->base_index = 0x7FFFFFFF;
      Clear_Stack (bank->alloc_reg);
      Clear_Stack (bank->avail_reg);
      bank->alloc_reg = NULL;
      bank->avail_reg = NULL;
      for (j = 0; j < R_NUM_TYPES; j++)
	bank->overlap_cnt[j] = 0;
      R_global_overlap[i] = 0;
      R_max_overlap_size[i] = 0;
    }

  for (i = 0; i < R_MAX_BANK; i++)
    {
      bank = R_bank + i;

      if (!bank->defined)
	continue;

      if (bank->type == R_INT)
	R_global_overlap[R_INT] |= bank->overlap;
      else if (bank->type == R_DOUBLE)
	R_global_overlap[R_DOUBLE] |= bank->overlap;
      else if (bank->type == R_FLOAT)
	R_global_overlap[R_FLOAT] |= bank->overlap;
      else if (bank->type == R_PREDICATE)
	R_global_overlap[R_PREDICATE] |= bank->overlap;
      else if (bank->type == R_BTR)
	R_global_overlap[R_BTR] |= bank->overlap;
      else if (bank->type == R_QUAD)
	R_global_overlap[R_QUAD] |= bank->overlap;
      else if (bank->type == R_POINTER)
	R_global_overlap[R_POINTER] |= bank->overlap;

      max_size = bank->reg_size;

      bank->base_index = base_index;

      if ((bank->overlap & R_OVERLAP_PREDICATE) &&
	  (bank->type != R_PREDICATE))
	{
	  overlap_bank = R_bank + R_PREDICATE + bank->rclass;
	  if (!overlap_bank->defined)
	    R_invalid_bank_overlap (bank, R_PREDICATE);

	  if (overlap_bank->base_index < bank->base_index)
	    bank->base_index = overlap_bank->base_index;

	  bank->overlap_cnt[R_PREDICATE / R_TYPE_INC] =
	    R_MAX (bank->reg_size / overlap_bank->reg_size, 1);

	  if (overlap_bank->reg_size > max_size)
	    max_size = overlap_bank->reg_size;
	}
      if ((bank->overlap & R_OVERLAP_INT) && (bank->type != R_INT))
	{
	  overlap_bank = R_bank + R_INT + bank->rclass;
	  if (!overlap_bank->defined)
	    R_invalid_bank_overlap (bank, R_INT);

	  if (overlap_bank->base_index < bank->base_index)
	    bank->base_index = overlap_bank->base_index;

	  bank->overlap_cnt[R_INT / R_TYPE_INC] =
	    R_MAX (bank->reg_size / overlap_bank->reg_size, 1);

	  if (overlap_bank->reg_size > max_size)
	    max_size = overlap_bank->reg_size;
	}
      if ((bank->overlap & R_OVERLAP_FLOAT) && (bank->type != R_FLOAT))
	{
	  overlap_bank = R_bank + R_FLOAT + bank->rclass;
	  if (!overlap_bank->defined)
	    R_invalid_bank_overlap (bank, R_FLOAT);

	  if (overlap_bank->base_index < bank->base_index)
	    bank->base_index = overlap_bank->base_index;

	  bank->overlap_cnt[R_FLOAT / R_TYPE_INC] =
	    R_MAX (bank->reg_size / overlap_bank->reg_size, 1);

	  if (overlap_bank->reg_size > max_size)
	    max_size = overlap_bank->reg_size;
	}
      if ((bank->overlap & R_OVERLAP_DOUBLE) && (bank->type != R_DOUBLE))
	{
	  overlap_bank = R_bank + R_DOUBLE + bank->rclass;
	  if (!overlap_bank->defined)
	    R_invalid_bank_overlap (bank, R_DOUBLE);

	  if (overlap_bank->base_index < bank->base_index)
	    bank->base_index = overlap_bank->base_index;

	  bank->overlap_cnt[R_DOUBLE / R_TYPE_INC] =
	    R_MAX (bank->reg_size / overlap_bank->reg_size, 1);

	  if (overlap_bank->reg_size > max_size)
	    max_size = overlap_bank->reg_size;
	}
      if ((bank->overlap & R_OVERLAP_BTR) && (bank->type != R_BTR))
	{
	  overlap_bank = R_bank + R_BTR + bank->rclass;
	  if (!overlap_bank->defined)
	    R_invalid_bank_overlap (bank, R_BTR);

	  if (overlap_bank->base_index < bank->base_index)
	    bank->base_index = overlap_bank->base_index;

	  bank->overlap_cnt[R_BTR / R_TYPE_INC] =
	    R_MAX (bank->reg_size / overlap_bank->reg_size, 1);

	  if (overlap_bank->reg_size > max_size)
	    max_size = overlap_bank->reg_size;
	}
      if ((bank->overlap & R_OVERLAP_QUAD) && (bank->type != R_QUAD))
	{
	  overlap_bank = R_bank + R_QUAD + bank->rclass;
	  if (!overlap_bank->defined)
	    R_invalid_bank_overlap (bank, R_QUAD);

	  if (overlap_bank->base_index < bank->base_index)
	    bank->base_index = overlap_bank->base_index;

	  bank->overlap_cnt[R_QUAD / R_TYPE_INC] =
	    R_MAX (bank->reg_size / overlap_bank->reg_size, 1);

	  if (overlap_bank->reg_size > max_size)
	    max_size = overlap_bank->reg_size;
	}
      if ((bank->overlap & R_OVERLAP_POINTER) && (bank->type != R_POINTER))
        {
          overlap_bank = R_bank + R_POINTER + bank->rclass;
          if (!overlap_bank->defined)
            R_invalid_bank_overlap (bank, R_POINTER);

          if (overlap_bank->base_index < bank->base_index)
            bank->base_index = overlap_bank->base_index;

          bank->overlap_cnt[R_POINTER / R_TYPE_INC] =
            R_MAX (bank->reg_size / overlap_bank->reg_size, 1);

          if (overlap_bank->reg_size > max_size)
            max_size = overlap_bank->reg_size;
        }

      /* No overlapping banks were found with smaller base_index, */
      /* so increment the base index past the current bank       */
      if (bank->base_index == base_index)
	{
	  base_index += bank->num_reg * bank->reg_size;
	  /* ensure that base_index is always even */
	  if (base_index & 0x1)
	    base_index += 1;
	}

      if (R_max_overlap_size[bank->type] < max_size)
	R_max_overlap_size[bank->type] = max_size;

      /* Initialize amount for <alloc_reg> stack initialization */
      bank->alloc_init = (max_size / bank->reg_size) * (mdes_total_slots ());
      /* Currently assume uniform -JCG 6/99 */

      /* Initialize the available register stack for the bank */
      stack = New_Stack ();
      for (j = bank->base_index;
	   j < bank->base_index + bank->num_reg * bank->reg_size;
	   j += bank->reg_size)
	Push_Bot (stack, (void *)(long int) j);

      if (M_arch != M_X86 || (R_Round_Robin_Allocation == 1))
	{
	  bank->avail_reg = stack;
	  bank->alloc_reg = New_Stack ();
	}
      else
	{
	  /* Since we are allocating an X86 architecture, don't bother */
	  /* to try to spread the register usage.  Place all registers */
	  /* on the alloc stack.                                       */
	  bank->avail_reg = New_Stack ();
	  bank->alloc_reg = stack;
	}
    }
}

void
R_reset_register_stacks (void)
{
  int i, j;
  R_Physical_Bank *bank;
  Stack *stack;

  for (i = 0; i < R_MAX_BANK; i++)
    {
      bank = R_bank + i;

      if (!bank->defined)
	continue;

      Clear_Stack (bank->alloc_reg);
      Clear_Stack (bank->avail_reg);

      /* 
       * If the stack is not empty, reinitialize it.  None empty
       * stacks are those overlapping with banks that have been
       * previously reinitialized
       */

      stack = bank->avail_reg;
      for (j = bank->base_index;
	   j < bank->base_index + bank->num_reg * bank->reg_size;
	   j += bank->reg_size)
	Push_Bot (stack, (void *)(long int) j);
    }
}

/*===========================================================================
 *
 *      Func :  R_find_free_register()
 *      Desc :  Searchs for a free register for the given live range based on
 *		its type and rclass. In the following order:
 *	 	Ex: type = INT, rclass = CALLER.
 *		    1) INT,MACRO_CALLER
 *		    2) INT,CALLER
 *		    3) INT,MACRO_CALLEE 
 *		    4) INT,CALLEE
 *      Input:  Node node		- current live range 
 *		Set reserved_resource 	- unavailable registers
 *      Output: Available register or -1 if none 
 *
 *      Side Effects:  none 
 *
 *===========================================================================*/
int
R_find_free_register_in_bank (R_Physical_Bank * reg_bank,
			      Set reserved_resource, R_Reg * vreg)
{
  int j, k, ok, res, reg, res_inc, res_max;
  int *overlap_cnt, overlap_reg;
  Stack *alloc_reg, *avail_reg;
  StackElmt *stack_reg, *ptr, *next;
  int arch = M_arch;
#if 0
  int null, dbl_num;
#endif

  ok = 0;

  /* X86 flag to selectively activate round robin allocation. */
  if (R_Round_Robin_Allocation == 1)
    arch = -1;

#if 0
  fprintf (stdout, "\n**%s,%s bank:\n", BANK_NAME (reg_bank->type),
	   CLASS_NAME (reg_bank->rclass));
  fprintf (stdout, "alloc:\n");
  Stack_Print (reg_bank->alloc_reg);
  fprintf (stdout, "\navail:\n");
  Stack_Print (reg_bank->avail_reg);
  fprintf (stdout, "**\n");
#endif

  res_inc = reg_bank->res_inc;
  res_max = reg_bank->res_inc * reg_bank->reg_size;

  alloc_reg = reg_bank->alloc_reg;
  avail_reg = reg_bank->avail_reg;
  overlap_cnt = reg_bank->overlap_cnt;
  /* 
   * Determine if there is an available register in the 
   * stack of already allocated registers, i.e. attempt
   * to minimize register usage
   */
  stack_reg = Stack_Ptr_Reset (alloc_reg);
  while ((stack_reg != NULL) && (vreg->rotating == 0))
    {
      ok = 1;
      reg = (int)(long int) Stack_Elmt_Data (stack_reg);

      for (res = reg; res < reg + res_max; res += res_inc)
	{
	  if (Set_in (reserved_resource, res))
	    {
	      ok = 0;
	      break;
	    }
	}
      if (ok)
	{
#if 0
	  fprintf (stdout, "-> %d is available.\n", reg);
#endif
	  /* If the we are allocating for an x86 architecture, don't */
	  /* round robin the registers on the alloc stack.  The arch */
	  /* doesn't have enough registers to try to minimize the    */
	  /* impact of register allocation to postpass scheduling.   */
	  /* All registers are placed on the alloc stack.            */
	  if (arch == M_X86)
	    return (reg);
	  /*
	   *  If a previously allocated register is available  
	   *  place it on the bottom of the stack to minimize
	   *  unnecessary reuse of registers.
	   */
/*
	    if ( Stack_Is_Bottom(alloc_reg) )
		fprintf(stdout,"\tBOTTOM ELEMENT\n");
		*/
	  Delete_Stack (alloc_reg, stack_reg);
	  Push_Bot (alloc_reg, (void *)(long int) reg);
#if 0
	  fprintf (stdout, "alloc after:\n");
	  Stack_Print (reg_bank->alloc_reg);
	  fprintf (stdout, "\n");
#endif
	  /*
	   *  Update the stacks of overlapping register banks
	   *  to reflect the change in the current stack 
	   */
	  for (j = 0; j < R_NUM_TYPES; j++)
	    {
	      int cnt, mask, overlap_size;
	      Stack *overlap_alloc;
	      R_Physical_Bank *overlap_bank;

	      if ((cnt = overlap_cnt[j]))
		{
		  overlap_bank = R_bank + (j * R_TYPE_INC) + reg_bank->rclass;
		  overlap_alloc = overlap_bank->alloc_reg;
		  overlap_size = overlap_bank->reg_size;

#if 0
		  fprintf (stdout, "-> adjusting overlapping bank\n");
		  fprintf (stdout, "-> %s,%s bank:\n",
			   BANK_NAME (overlap_bank->type),
			   CLASS_NAME (overlap_bank->rclass));
		  fprintf (stdout, "before\n");
		  Stack_Print (overlap_bank->alloc_reg);
#endif

		  ptr = Stack_Ptr_Reset (overlap_alloc);

		  /* The overlapping register bank contains larger registers */
		  /* than the current bank, so compare the register to be    */
		  /* moved is the register in the overlapping stack that is  */
		  /* equal to < reg & overlap_bank->mask >                   */
		  if (reg_bank->reg_size < overlap_size)
		    {
		      mask = overlap_bank->mask;
		      while (cnt && ptr)
			{
			  overlap_reg = (int)(long int) Stack_Elmt_Data (ptr);
			  next = Stack_Find_Next (overlap_alloc);
			  if ((reg & mask) == overlap_reg)
			    {
			      Delete_Stack (overlap_alloc, ptr);
			      Push_Bot (overlap_alloc, 
					(void *)(long int) overlap_reg);
			      cnt--;
			    }
			  ptr = next;
			}
		    }
		  /* The overlapping register bank contains smaller
                     registers than the current bank, so the registers
                     to be moved are those where < overlap_reg &
                     reg_bank->mask > is equal to the current register */
		  else
		    {
		      mask = reg_bank->mask;
		      while (cnt && ptr)
			{
			  overlap_reg = (int)(long int) Stack_Elmt_Data (ptr);
			  next = Stack_Find_Next (overlap_alloc);
			  if ((overlap_reg & mask) == reg)
			    {
			      Delete_Stack (overlap_alloc, ptr);
			      Push_Bot (overlap_alloc, 
					(void *)(long int) overlap_reg);
			      cnt--;
			    }
			  ptr = next;
			}
		    }

#if 0
		  fprintf (stdout, "\nafter\n");
		  Stack_Print (overlap_bank->alloc_reg);
		  fprintf (stdout, "\n");
#endif
		}
	    }
	  return (reg);
	}
      /*
       * Find the next register on the allocated register stack to
       * see if it is available
       */
      stack_reg = Stack_Find_Next (alloc_reg);
    }

  /*
   * Since we've made it this far, none of the previously allocated
   * registers are free, so grab registers off of the available
   * register stack until we find one that we can use.  In most
   * cases, the first one will be available, the exception to this
   * is macroregisters.
   */
  reg = 0;

#if 0
  R_get_rot_regs (L_fn, &null, &null, &null, &null,
		  &null, &dbl_num, &null, &null);
#endif
  
  while (reg != -1)
    {
      /*
       * Move registers from the available register stack to
       * the allocated register stack in increments of <alloc_init>
       * checking each one to see if it is usable
       */

      if (vreg->rotating == 1)
	{
	  int nth_rot_reg = vreg->nth_rot_reg;
	  int overlap_offset = 0;

	  if (vreg->type == R_FLOAT)
	    {
#if 0
	      /* The overlap_offset allocates the floats as
		 higher numbered registers immediately following
		 the doubles in the shared register file.  This is
		 currently a quick-fix and should be set
		 automatically in the analyze_overlap
		 functions. MCM */
	      overlap_offset = dbl_num;
#else
	      L_punt("Shouldn't find a FLOAT. MCM");
#endif
	    }
	  
	  /* nth_rot_reg is the n-th register from the beginning of
	     the rotating register set. Reg_size is relative to the
	     base register size for overlapped registers.  For
	     example, doubles are size 2 while floats are size 1
	     when overlapped for HPPA.  ints are size 1 for IMPACT
	     and HPPA because there are no sub-register sizes.  On
	     x86, I am not sure about the sizes for accesses to eax
	     vs ah. MCM */

	  reg = (nth_rot_reg * reg_bank->reg_size) + overlap_offset
	    + reg_bank->rot_reg_ofs + reg_bank->base_index;

	  if ((reg_bank->base_index + reg_bank->rot_reg_ofs) > reg ||
	      (reg_bank->base_index + reg_bank->rot_reg_ofs
	       + (reg_bank->num_rot_reg * reg_bank->reg_size)) <= reg)
	    {
	      return (-1);
	    }

	  stack_reg = Stack_Find (avail_reg, (void *)(long int) reg);

	  if (stack_reg == NULL || Set_in (reserved_resource, reg))
	    {
	      L_punt ("R_find_free_register_in_bank: rot reg unavailable.");
	    }
	  reg = (int)(long int) Stack_Elmt_Data (stack_reg);
	  Delete_Stack (avail_reg, stack_reg);
	  Push_Bot (alloc_reg, (void *)(long int) reg);
	  ok = 1;
	}
      else
	{
	  for (j = 0; j < reg_bank->alloc_init; j++)
	    {
	      if ((reg = (int)(long int) Stack_Top (avail_reg)) != -1)
		{
		  ok = 1;
		  for (res = reg; res < reg + res_max; res += res_inc)
		    {
		      if (Set_in (reserved_resource, res))
			{
			  ok = 0;
			  break;
			}
		    }
		  Push_Bot (alloc_reg, Pop (avail_reg));
		  if (ok)
		    {
		      j++;
		      break;
		    }
		}
	    }
	  for (; j < reg_bank->alloc_init; j++)
	    {
	      if (((int)(long int) Stack_Top (avail_reg)) != -1)
		Push_Bot (alloc_reg, Pop (avail_reg));
	    }
	}
#if 0
      fprintf (stdout, "-> %d off of available register stack.\n", reg);
#endif

      if (vreg->rotating == 1)
	{
	  /*
	   *  Update the stacks of overlapping register banks
	   *  to reflect the change in the current stack 
	   */
	  for (j = 0; j < R_NUM_TYPES; j++)
	    {
	      int cnt, mask, overlap_size;
	      Stack *overlap_alloc;
	      Stack *overlap_avail;
	      R_Physical_Bank *overlap_bank;

	      if ((cnt = overlap_cnt[j]))
		{
		  overlap_bank = R_bank + (j * R_TYPE_INC) + reg_bank->rclass;
		  overlap_avail = overlap_bank->avail_reg;
		  overlap_alloc = overlap_bank->alloc_reg;
		  overlap_size = overlap_bank->reg_size;

#if 0
		  fprintf (stdout, "-> adjusting overlapping bank\n");
		  fprintf (stdout, "-> %s,%s bank:\n",
			   BANK_NAME (overlap_bank->type),
			   CLASS_NAME (overlap_bank->rclass));
		  fprintf (stdout, "before\n");
		  Stack_Print (overlap_bank->avail_reg);
#endif

		  ptr = Stack_Ptr_Reset (overlap_avail);

		  /* The overlapping register bank contains larger registers */
		  /* than the current bank, so compare the register to be    */
		  /* moved is the register in the overlapping stack that is  */
		  /* equal to < reg & overlap_bank->mask >                   */
		  if (reg_bank->reg_size < overlap_size)
		    {
		      mask = overlap_bank->mask;
		      while (cnt && ptr)
			{
			  overlap_reg = (int)(long int) Stack_Elmt_Data (ptr);
			  next = Stack_Find_Next (overlap_avail);
			  if ((reg & mask) == overlap_reg)
			    {
			      Delete_Stack (overlap_avail, ptr);
			      Push_Bot (overlap_alloc, 
					(void *)(long int) overlap_reg);
			      cnt--;
			    }
			  ptr = next;
			}
		    }
		  /* The overlapping register bank contains smaller
                     registers than the current bank, so the registers
                     to be moved are those where < overlap_reg &
                     reg_bank->mask > is equal to the current register */
		  else
		    {
		      mask = reg_bank->mask;
		      while (cnt && ptr)
			{
			  overlap_reg = (int)(long int) Stack_Elmt_Data (ptr);
			  next = Stack_Find_Next (overlap_avail);
			  if ((overlap_reg & mask) == reg)
			    {
			      Delete_Stack (overlap_avail, ptr);
			      Push_Bot (overlap_alloc, 
					(void *)(long int) overlap_reg);
			      cnt--;
			    }
			  ptr = next;
			}
		    }
		}
	    }
	}
      else
	{
	  /*
	   * Update the stacks of overlapping register banks
	   * to reflect the change in the current stack
	   */
	  for (j = 0; j < R_NUM_TYPES; j++)
	    {
	      int cnt;
	      Stack *overlap_alloc, *overlap_avail;
	      R_Physical_Bank *overlap_bank;

	      if ((cnt = overlap_cnt[j]))
		{
		  int init;

		  overlap_bank = R_bank + (j * R_TYPE_INC) + reg_bank->rclass;
		  overlap_alloc = overlap_bank->alloc_reg;
		  overlap_avail = overlap_bank->avail_reg;
#if 0
		  fprintf (stdout, "-> adjusting overlapping bank\n");
		  fprintf (stdout, "-> %s,%s bank:\n",
			   BANK_NAME (overlap_bank->type),
			   CLASS_NAME (overlap_bank->rclass));
		  fprintf (stdout, "before\n");
		  Stack_Print (overlap_bank->alloc_reg);
#endif
		  init = overlap_bank->alloc_init / cnt;
		  for (k = 0; k < init; k++)
		    {
		      while (cnt)
			{
			  if (((int)(long int) Stack_Top (overlap_avail)) 
			      != -1)
			    Push_Bot (overlap_alloc, Pop (overlap_avail));
			  cnt--;
			}
		      cnt = overlap_cnt[j];
		    }
#if 0
		  fprintf (stdout, "\nafter\n");
		  Stack_Print (overlap_bank->alloc_reg);
		  fprintf (stdout, "\n");
#endif
		}
	    }
	}
      if (ok)
	return (reg);

      reg = (int)(long int) Stack_Top (avail_reg);
    }

  /* At this point, we have found no available register */
  return (-1);
}


int
R_find_free_register (R_Reg * vreg, Set reserved_resource, int final)
{
  int mac_rclass = -1, vreg_rclass;
  int res, res_inc, res_max, ok;
  R_Physical_Bank *reg_bank;
  L_Region_Regmap *regmap;
  int free = -2;

  if ((vreg->flags & R_REGION_CONSTRAINED))
    {
      regmap = L_find_region_regmap (R_Region, vreg->index);
      if (regmap != NULL && regmap->phys_reg != -1)
	{
	  reg_bank = R_bank + vreg->type + regmap->type;
	  res_inc = reg_bank->res_inc;
	  res_max = reg_bank->res_inc * reg_bank->reg_size;
	  free = regmap->phys_reg;
	  ok = 1;
	  for (res = free; res < free + res_max; res += res_inc)
	    {
	      if (Set_in (reserved_resource, res))
		{
		  ok = 0;
		  /*
		     fprintf(stdout,"R_find_free_register: "
		                    "can't use global allocation %d -> %d\n",
		     vreg->index,regmap->phys_reg);


		     if ( vreg->flags & R_PREALLOCATED_FLYBY )
		     fprintf(stdout,"FLYBY's global is illegal locally\n");
		   */
		  if (final)
		    vreg->flags &= (~(R_REGION_CONSTRAINED));
		  return (-1);
		}
	    }
	  if (ok)
	    {
	      vreg->rclass = regmap->type;
	      return (regmap->phys_reg);
	    }
	}
    }

  if (vreg->rclass == R_PREFER_SPILL)
    return (-1);

#if 0
  fprintf (stdout, "*\nvreg %d\n*\n", vreg->index);
#endif

  /* First check to see if we can place the virtual register in */
  /* macro register                                             */
  if (R_Macro_Allocation)
    {
      mac_rclass = (vreg->rclass == R_CALLER) ? 
	R_MACRO_CALLER : R_MACRO_CALLEE;

      reg_bank = R_bank + vreg->type + mac_rclass;

      if (reg_bank->defined &&
	  ((free = R_find_free_register_in_bank (reg_bank,
						 reserved_resource,
						 vreg)) != -1))
	{
	  vreg->rclass = mac_rclass;
	  return (free);
	}
    }
  /* Now, check to see if we can place the register in the desired bank */
  reg_bank = R_bank + vreg->type + vreg->rclass;
  if (reg_bank->defined &&
      ((free = R_find_free_register_in_bank (reg_bank,
					     reserved_resource, vreg)) != -1))
    {
      return (free);
    }

  if ((vreg->rclass == R_CALLER) ?
      (vreg->callee_benefit >= 0) : (vreg->caller_benefit >= 0))
    {
      if (R_Macro_Allocation)
	{
	  /* First try to use macro register in other bank */
	  mac_rclass = (mac_rclass == R_MACRO_CALLER) ? 
	    R_MACRO_CALLEE : R_MACRO_CALLER;
	  reg_bank = R_bank + vreg->type + mac_rclass;

	  if (reg_bank->defined &&
	      ((free = R_find_free_register_in_bank (reg_bank,
						     reserved_resource,
						     vreg)) != -1))
	    {
	      vreg->rclass = mac_rclass;
	      return (free);
	    }
	}

      /* Now try the other bank since cost is not negative */
      vreg_rclass = ((vreg->rclass == R_CALLER) ? R_CALLEE : R_CALLER);
      reg_bank = R_bank + vreg->type + vreg_rclass;
      if (reg_bank->defined &&
	  ((free = R_find_free_register_in_bank (reg_bank,
						 reserved_resource,
						 vreg)) != -1))
	{
	  vreg->rclass = vreg_rclass;
	  return (free);
	}

      if (free == -2)
	{
	  L_punt ("No register bank defined for registers of type: %s\n",
		  BANK_NAME (vreg->type));
	}
    }
  return (-1);
}

int
R_find_rotating_register (R_Reg * vreg, Set reserved_resource, int final)
{
  int vreg_rclass;
  R_Physical_Bank *reg_bank;
  int free = -2;

#if 0
  fprintf (stdout, "*\nvreg %d\n*\n", vreg->index);
#endif

  reg_bank = O_locate_rot_reg_bank (L_fn, vreg);

  if (reg_bank->defined &&
      ((free
	=
	R_find_free_register_in_bank (reg_bank, reserved_resource,
				      vreg)) != -1))
    {
      return (free);
    }

  /* MCM We should never get here. */
  L_punt ("R_find_rotating_register: "
	  "Unable to allocate register in specified bank.");

  if ((vreg->rclass == R_CALLER) ? (vreg->callee_benefit >= 0) :
    (vreg->caller_benefit >= 0))
    {
      /* Now try the other bank since cost is not negative */
      vreg_rclass = ((vreg->rclass == R_CALLER) ? R_CALLEE : R_CALLER);
      reg_bank = R_bank + vreg->type + vreg_rclass;
      if (reg_bank->defined &&
	  ((free
	    =
	    R_find_free_register_in_bank (reg_bank, reserved_resource,
					  vreg)) != -1))
	{
	  vreg->rclass = vreg_rclass;
	  return (free);
	}
      if (free == -2)
	{
	  L_punt ("No register bank defined for registers of type: %s\n",
		  BANK_NAME (vreg->type));
	}
    }
  return (-1);
}

/*===========================================================================
 *
 *      Func :  R_smallest_overlapping_bank()
 *      Desc :  Given a register bank type and rclass, this function determines
 *		the register bank of smallest <reg_size> that overlaps
 *      Input:  type - register bank type
 *		rclass - register bank rclass 
 *      Output: register bank type 
 *
 *      Side Effects: none
 *
 *===========================================================================*/
int
R_smallest_overlapping_bank (int type, int rclass)
{
  R_Physical_Bank *bank;

  bank = R_bank + type + rclass;

  /* This works since R_define_physical_bank ensures that */
  /* a register bank overlaps at least itself             */

  if (bank->overlap & R_OVERLAP_PREDICATE)
    return (R_PREDICATE);
  else if (bank->overlap & R_OVERLAP_INT)
    return (R_INT);
  else if (bank->overlap & R_OVERLAP_FLOAT)
    return (R_FLOAT);
  else if (bank->overlap & R_OVERLAP_DOUBLE)
    return (R_DOUBLE);
  else if (bank->overlap & R_OVERLAP_BTR)
    return (R_BTR);
  else if (bank->overlap & R_OVERLAP_QUAD)
    return (R_QUAD);

  L_punt ("R_smallest_overlapping_bank: "
	  "no register bank overlap for type: %d\n",
	  bank->overlap);
  return (0);
}

void
R_print_bank_configuration (void)
{
  int i, j, *map, count;
  R_Physical_Bank *bank;

  fprintf (stdout, "*\n*\n* Register Bank Configuration\n*\n*\n");

  for (i = 0; i < R_MAX_BANK; i++)
    {
      bank = R_bank + i;
      if (!bank->defined)
	continue;

      fprintf (stdout, "Name #%d:\n", i);
      fprintf (stdout, "\tType:\t%s\n", BANK_NAME (bank->type));
      fprintf (stdout, "\tClass:\t%s\n", CLASS_NAME (bank->rclass));
      fprintf (stdout, "\tNumber:\t%d\n", bank->num_reg);
      fprintf (stdout, "\tSize:\t%d\n", bank->reg_size);
      fprintf (stdout, "\tNumRot:\t%d\n", bank->num_rot_reg);

      /* Overlap conditions */
      fprintf (stdout, "\tOverlap:\t");
      if (bank->overlap & R_OVERLAP_PREDICATE)
	fprintf (stdout, "PREDICATE(%d) ",
		 bank->overlap_cnt[R_PREDICATE / R_TYPE_INC]);
      if (bank->overlap & R_OVERLAP_INT)
	fprintf (stdout, "INT(%d) ", bank->overlap_cnt[R_INT / R_TYPE_INC]);
      if (bank->overlap & R_OVERLAP_FLOAT)
	fprintf (stdout, "FLOAT(%d) ",
		 bank->overlap_cnt[R_FLOAT / R_TYPE_INC]);
      if (bank->overlap & R_OVERLAP_DOUBLE)
	fprintf (stdout, "DOUBLE(%d) ",
		 bank->overlap_cnt[R_DOUBLE / R_TYPE_INC]);
      if (bank->overlap & R_OVERLAP_BTR)
	fprintf (stdout, "BTR(%d) ", bank->overlap_cnt[R_BTR / R_TYPE_INC]);
      if (bank->overlap & R_OVERLAP_QUAD)
	fprintf (stdout, "QUAD(%d) ", bank->overlap_cnt[R_QUAD / R_TYPE_INC]);
      if (bank->overlap & R_OVERLAP_POINTER)
	fprintf (stdout, "POINTER(%d) ",
		 bank->overlap_cnt[R_POINTER / R_TYPE_INC]);
      fprintf (stdout, "\n");

      /* Register Maps */
      fprintf (stdout, "\tMap:\t");

      map = R_map[bank->type + bank->rclass];
      count = 0;
      for (j = 0; j < bank->num_reg * bank->reg_size; j += bank->reg_size)
	{

	  if (count == 0)
	    {
	      fprintf (stdout, "%d:\t", bank->base_index + j);
	    }

	  if (bank->rclass == R_MACRO_CALLER ||
	      bank->rclass == R_MACRO_CALLEE)
	    fprintf (stdout, "%s ", L_macro_name (map[j]));
	  else
	    fprintf (stdout, "%d ", map[j]);

	  if (count++ == 9)
	    {
	      fprintf (stdout, "\n\t\t");
	      count = 0;
	    }
	}

      if (bank->alloc_reg != NULL)
	{
	  fprintf (stdout, "\n\t\talloc:\n");
	  Stack_Print (bank->alloc_reg);
	}
      if (bank->avail_reg != NULL)
	{
	  fprintf (stdout, "\n\t\tavail:\n");
	  Stack_Print (bank->avail_reg);
	}

      fprintf (stdout, "\n\n");
    }
  fprintf (stdout, "\n");
}

/*===========================================================================
 *
 *      Func :  R_virtual_to_machine_conversion()
 *      Desc :  Determines the machine register that a live range has been
 *		allocated to.  The <base_index> field is then assigned that
 *		value.  For code generation purposes, sets of caller and
 *		callee saved registers used are also created.
 *      Input:  Set *caller 	- set of caller saved registers used 
 *		Set *callee	- set of callee saved registers used	 
 *      Output: void 
 *
 *      Side Effects:  none 
 *
 *===========================================================================*/
void
R_virtual_to_machine_conversion (void)
{
  int *phys_map;
  R_Reg *vreg;
  R_Physical_Bank *bank;
  int int_base, int_num, flt_base, flt_num, dbl_base, dbl_num, pred_base,
    pred_num;
  L_Attr *rr_attr;

  R_get_rot_regs (L_fn, &int_base, &int_num, &flt_base, &flt_num,
		  &dbl_base, &dbl_num, &pred_base, &pred_num);

  for (vreg = R_vreg; vreg != NULL; vreg = vreg->nextReg)
    {
      if (vreg->flags & R_PREALLOCATED_FLYBY &&
	  !(vreg->flags & R_REGION_CONSTRAINED))
	continue;

      /* locate physical register corresponding to base_index */
      /* assigned to the virtual register during allocation   */
      bank = R_bank + vreg->type + vreg->rclass;

      if (!bank->defined)
	L_punt ("Register allocated to non-existant register bank!");

      phys_map = R_map[vreg->type + vreg->rclass];

      vreg->phys_reg = phys_map[vreg->base_index - bank->base_index];

      *bank->used_reg = Set_add (*bank->used_reg, vreg->phys_reg);

      if (vreg->rotating == 1)
	{
	  /* This conditional converts the virtual register base ID
	     of the rotating register segments to their physical register IDs.
	     Set the base register variables to the negative physical
	     register base.  This prevents the new physical register base
	     from later matching a virtual register ID and being set again
	     to the wrong physical register. */
	  if (int_base == vreg->index)
	    int_base = -vreg->phys_reg;
	  if (flt_base == vreg->index)
	    flt_base = -vreg->phys_reg;
	  if (dbl_base == vreg->index)
	    dbl_base = -vreg->phys_reg;
	  if (pred_base == vreg->index)
	    pred_base = -vreg->phys_reg;
	}
    }

  if (R_Print_Allocation)
    {
      int count = 32;
      fprintf (stderr, "\n***  ALLOCATION RESULTS ***\n\n");

      for (vreg = R_vreg; vreg != NULL; vreg = vreg->nextReg)
	{
	  if (vreg->flags & R_PREALLOCATED_FLYBY)
	    continue;

	  if (count == 32)
	    {
	      fprintf (stderr,
		       "\nVREG\tTYPE\tRESULT\tREGISTER\tCLASS\t\tLOCATION\n");
	      fprintf (stderr,
		       "----\t----\t------\t--------\t-----\t\t--------\n");
	      count = 1;
	    }
	  if (vreg->pvreg)
	    fprintf (stderr, "%d(%d)\t%s\t",
		     vreg->index, vreg->pvreg->index, BANK_NAME (vreg->type));
	  else
	    fprintf (stderr, "%d\t%s\t", vreg->index, BANK_NAME (vreg->type));

	  if (vreg->flags & R_SPILLED)
	    fprintf (stderr, "  SP\t%d\t\t%s\t\tmem[%d]\n",
		     vreg->phys_reg, CLASS_NAME (vreg->rclass),
		     vreg->spill_loc);
	  else
	    fprintf (stderr, "  A\t%d(%d)\t\t%s\n", vreg->phys_reg,
		     vreg->base_index, CLASS_NAME (vreg->rclass));

	  count++;
	}
    }

  /* Obtain the ranges of the rotating registers, or return the empty
     set of there are none for this function. */

  if ((rr_attr = L_find_attr (L_fn->attr, "rr")))
    {
      if (int_base < 0)
	rr_attr->field[0]->value.i = -int_base;
      if (flt_base < 0)
	rr_attr->field[2]->value.i = -flt_base;
      if (dbl_base < 0)
	rr_attr->field[4]->value.i = -dbl_base;
      if (pred_base < 0)
	rr_attr->field[6]->value.i = -pred_base;
    }
}


int
R_get_rot_reg_alloc_multiple (int ctype)
{
  int rtype = R_Ltype_to_Rtype (ctype);

  R_Physical_Bank *reg_bank = R_bank + rtype + R_CALLER;

  if (reg_bank->num_rot_reg > 0)
    return reg_bank->num_rot_reg_alloc;

  reg_bank = R_bank + rtype + R_CALLEE;

  if (reg_bank->num_rot_reg > 0)
    return reg_bank->num_rot_reg_alloc;

  return -1;
}


int
R_get_rot_reg_max_alloc (int ctype)
{
  int rtype = R_Ltype_to_Rtype (ctype);

  R_Physical_Bank *reg_bank = R_bank + rtype + R_CALLER;

  if (reg_bank->num_rot_reg > 0)
    return reg_bank->num_rot_reg;

  reg_bank = R_bank + rtype + R_CALLEE;

  if (reg_bank->num_rot_reg > 0)
    return reg_bank->num_rot_reg;

  return 0;
}

