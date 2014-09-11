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
 *      File :          l_safe.c
 *      Description :   Simple safety analysis
 *           Complex interprocedural analysis can be found in Lsafe directory.
 *           This is just the very simplest stuff that should be performed
 *           by everyone!
 *      Author :        Scott Mahlke, Roger Bringmann, Wen-mei Hwu
 *      Date :          December 1994
 *
 *==========================================================================*/
/* 10/29/02 REK Adding config.h */
#include <config.h>
#include <Lcode/l_main.h>

/*===========================================================================*/
/*
 *      Simple side effect and Synchronization function tables
 */
/*===========================================================================*/

/*
 *      Functions assumed to have no effect on the memory, therefore loads
 *      and stores can be optimized across these functions.
 */
static char *L_side_effect_free_func_table[] = {

  /* selected libc functions */
/*
        "_printf",
        "_fprintf",
        "_putc",
        "_fputc",
        "_fputs",
        "_putchar",
        "_puts",
        "_fflush",
        "_ftell",
        "_fwrite",
        "_fgetpos",
        "_ferror",
        "_clearerr",
        "_feof",
        "_fclose",
*/
  "_strlen",
  "_strcmp",
  "_strncmp",
  "_perror",
  "_isalnum",
  "_isalpha",
  "_iscntrl",
  "_isdigit",
  "_isgraph",
  "_islower",
  "_isprint",
  "_ispunct",
  "_isspace",
  "_isupper",
  "_isxdigit",
  "_tolower",
  "_toupper",
  "_atoi",
  "_atof",
  "_atol",
  "_strchr",
  "_strrchr",
  "_strstr",
  "_strcasecmp",
  "_strncasecmp",
  "_index",
  "_rindex",
  "_strpbrk",
  "_strspn",
  "_strcspn",
  "_rand",
  "_srand",
  "_memchr",
  "_memcmp",
  "setjmp",

  /* in case the user did not use return */
  "_exit",

  /* selected libm functions */
  "_exp",
  "_exp2",
  "_exp10",
  "_log",
  "_log2",
  "_log10",
  "_pow",
  "_sqrt",
  "_sin",
  "_cos",
  "_tan",
  "_asin",
  "_acos",
  "_atan",
  "_atan2",
  "_sinh",
  "_cosh",
  "_tanh",
  "_asinh",
  "_acosh",
  "_atanh",
  "_abs",
  "_labs",
  "_fabs",
  "_cabs",
  "_floor",
  "_ceil",
  "_system",

  /* curses lib functions, under test still :P */
  "_printw",
  "_wprintw",
  "_addstr",
  "_waddstr",
  "_wmove",
  "_printw",
  "_wclrtoeol",
  "_wstandout",
  "_wstandend",
  "_waddch",

  /* last entry of array to mark termination */
  NULL
};

/*
 *      No code motion across these ops, also no optimizations across these
 *      ops which would create a live register
 */
static char *L_synchronization_func_table[] = {

  /* Known subroutine calls that should be treated like sync ops */
  "_setjmp",
  "__setjmp",
  "_sigsetjmp",
  "___sigsetjmp",
  "_longjmp",
  "_siglongjmp",
  "___siglongjmp",
  "_pthread_create",
  "__pthread_create",

  /* last entry of array to mark termination */
  NULL
};

/*===========================================================================*/
/*
 *      Simple side effect free function call analysis
 */
/*===========================================================================*/

/* Sometimes you don't have an oper for checking - RAB */
int
L_name_in_side_effect_free_func_table (char *name)
{
  char *label;
  int i;

  if (name == NULL)
    return 0;

  label = M_fn_name_from_label (name);

  for (i = 0;; i++)
    {
      if (L_side_effect_free_func_table[i] == NULL)
        break;
      if (!strcmp (L_side_effect_free_func_table[i], label))
        return 1;
    }
  return 0;
}

int
L_op_in_side_effect_free_func_table (L_Oper * op)
{
  if (op == NULL)
    return 0;
  if (!L_is_label (op->src[0]))
    return 0;

  if (!strncmp (op->src[0]->value.l, "_$fn", 4))
    {
      return (L_name_in_side_effect_free_func_table
              (op->src[0]->value.l + 4));
    }
  else
    {
      return (L_name_in_side_effect_free_func_table (op->src[0]->value.l));
    }
}

int
L_side_effect_free_sub_call (L_Oper * oper)
{
  if (oper == NULL)
    return 0;

  return (L_EXTRACT_BIT_VAL (oper->flags, L_OPER_SIDE_EFFECT_FREE));
}

void
L_find_side_effect_free_sub_calls (L_Func * fn)
{
  L_Cb *cb;
  L_Oper *op;

  for (cb = fn->first_cb; cb != NULL; cb = cb->next_cb)
    {
      for (op = cb->first_op; op != NULL; op = op->next_op)
        {
          if (!L_subroutine_call_opcode (op))
            continue;
          if (!L_op_in_side_effect_free_func_table (op))
            continue;
          op->flags = L_SET_BIT_FLAG (op->flags, L_OPER_SIDE_EFFECT_FREE);
        }
    }
}

/*===========================================================================*/
/*
 *      Simple synchronization function call analysis
 */
/*===========================================================================*/

int
L_name_in_synchronization_func_table (char *name)
{
  char *label;
  int i;

  if (name == NULL)
    return 0;

  label = M_fn_name_from_label (name);

  for (i = 0;; i++)
    {
      if (L_synchronization_func_table[i] == NULL)
        break;
      if (!strcmp (L_synchronization_func_table[i], label))
        return 1;
    }
  return 0;
}

int
L_op_in_synchronization_func_table (L_Oper * op)
{
  if (op == NULL)
    return 0;
  if (!L_is_label (op->src[0]))
    return 0;

  return (L_name_in_synchronization_func_table (op->src[0]->value.l));
}

int
L_synchronization_sub_call (L_Oper * oper)
{
  if (oper == NULL)
    return 0;

  return (L_EXTRACT_BIT_VAL (oper->flags, L_OPER_SYNC));
}

void
L_find_synchronization_sub_calls (L_Func * fn)
{
  L_Cb *cb;
  L_Oper *op;

  for (cb = fn->first_cb; cb != NULL; cb = cb->next_cb)
    {
      for (op = cb->first_op; op != NULL; op = op->next_op)
        {
          if (!L_subroutine_call_opcode (op))
            continue;
          if (!L_op_in_synchronization_func_table (op))
            continue;
          op->flags = L_SET_BIT_FLAG (op->flags, L_OPER_SYNC);
        }
    }
}

/*===========================================================================*/
/*
 *      Simple safety for speculation analysis
 */
/*===========================================================================*/

/*
 *      Returns a boolean TRUE if the instruction is a potentially
 *      excepting instruction.
 *
 *      This should later interface to the mdes, just too much of a burden now.
 *      So we will go with our usual assumption, loads, stores, int divs, int
 *      rems, and all floating point instrs are potentially excepting.
 */
int
L_is_pei (L_Oper * oper)
{
  if (oper == NULL)
    return 0;

  if (L_general_load_opcode (oper))
    return 1;
  if (L_general_store_opcode (oper))
    return 1;
  if (L_check_opcode (oper))
    return 1;
  if (L_int_div_opcode (oper))
    return 1;
  if (L_int_rem_opcode (oper))
    return 1;
  if (L_flt_arithmetic_opcode (oper))
    return 1;
  if (L_dbl_arithmetic_opcode (oper))
    return 1;
  if (L_flt_comparison_opcode (oper))
    return 1;
  if (L_dbl_comparison_opcode (oper))
    return 1;
  if (L_intrinsic_opcode (oper) &&
      !L_intrinsic_is_opti_enabled (oper, "SafeSpeculation"))
    return 1;
  return 0;
}

int
L_is_pe_expression (L_Expression *expression)
{
  if (expression == NULL)
    return 0;

  if (L_general_load_opcode (expression))
    return 1;
  if (L_general_store_opcode (expression))
    return 1;
  if (L_check_opcode (expression))
    return 1;
  if (L_int_div_opcode (expression))
    return 1;
  if (L_int_rem_opcode (expression))
    return 1;
  if (L_flt_arithmetic_opcode (expression))
    return 1;
  if (L_dbl_arithmetic_opcode (expression))
    return 1;
  if (L_flt_comparison_opcode (expression))
    return 1;
  if (L_dbl_comparison_opcode (expression))
    return 1;
  /* Not necessarily true, but at least safe. */
  if (L_intrinsic_opcode (expression))
    return 1;
  return 0;
}

/*
 * 1) load instruction with immediate offset from a label is always safe.
 * 2) load instruction based upon the stack pointer of frame pointer
 * 3) divide and remainder instructions with non-zero divisor.
 */

int
L_is_trivially_safe (L_Oper * oper)
{
  if (!L_is_pei (oper))
    return 1;

  if (L_EXTRACT_BIT_VAL (oper->flags, L_OPER_SAFE_PEI))
    return 1;

  if (L_general_load_opcode (oper) &&
      L_EXTRACT_BIT_VAL (oper->flags, L_OPER_SPILL_CODE))
    return 1;

  if ((L_general_load_opcode (oper)) || (L_general_store_opcode (oper)))
    {
      if ((L_is_int_constant (oper->src[0]) && L_is_label (oper->src[1])))
        return 1;
      else if ((L_is_int_constant (oper->src[1]) &&
                L_is_label (oper->src[0])))
        return 1;
      else if (M_is_stack_operand (oper->src[0]) &&
               L_is_int_constant (oper->src[1]))
        return 1;
      else if (M_is_stack_operand (oper->src[1]) &&
               L_is_int_constant (oper->src[0]))
        return 1;
    }
  else if (L_int_div_opcode (oper) &&
           L_is_int_constant (oper->src[1]) && !L_is_int_zero (oper->src[1]))
    return 1;
  else if (L_int_rem_opcode (oper) &&
           L_is_int_constant (oper->src[1]) && !L_is_int_zero (oper->src[1]))
    return 1;

  return 0;
}

/*
 *      Oper is safe if it is either never excepting or it has been
 *      marked with an F (safe) flag.
 */
int
L_safe_for_speculation (L_Oper * oper)
{
  if (!L_is_pei (oper))
    return (1);
  else if (L_EXTRACT_BIT_VAL (oper->flags, L_OPER_SAFE_PEI))
    return (1);
  else
    return (0);
}

void
L_mark_safe_instructions (L_Func * fn)
{
  L_Cb *cb;
  L_Oper *oper;

  for (cb = fn->first_cb; cb != NULL; cb = cb->next_cb)
    {
      for (oper = cb->first_op; oper != NULL; oper = oper->next_op)
        {
          if (!L_is_pei (oper))
            continue;
          if (!L_is_trivially_safe (oper))
            continue;
#if 0
          /* DIA - promoted load is unsafe because its source may not be 
             set. */
          if (L_EXTRACT_BIT_VAL (oper->flags, L_OPER_PROMOTED))
            continue;
#endif
          oper->flags = L_SET_BIT_FLAG (oper->flags, L_OPER_SAFE_PEI);
          oper->flags = L_CLR_BIT_FLAG (oper->flags, L_OPER_MASK_PE);
        }
    }
}
