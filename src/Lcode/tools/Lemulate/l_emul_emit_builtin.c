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
 *      File: l_emul_builtin.c (version 2)
 *      Authors: IMPACT Technologies, Inc. (John C. Gyllenhaal)
 *      Creation Date:  March 1999
 * 
 *      This is a complete reengineering and rewrite of version 1 of
 *      of Lemulate, which was written by Qudus Olaniran, Dan Connors,
 *      IMPACT Technologies, Inc, and Wen-mei Hwu.  This new version is
 *      optimized for portability, performance, and hopefully clarity.
 *
\*****************************************************************************/

#include <config.h>
#include <Lcode/l_emul.h>
#include "l_emul_decl.h"
#include "l_emul_emit_builtin.h"
#include "l_emul_emit_data.h"
#include "l_emul_emit_op.h"
#include "l_emul_emit_operand.h"
#include "l_emul_intrinsic.h"
#include "l_emul_trace.h"
#include "l_emul_util.h"

#ifdef INTRINSICS
#include "l_emul_intrinsic.h"
#endif

/* Returns 1 if 'name' is the name of a builtin function */
int
C_is_builtin_function (char *name)
{
  char *true_name;

  /* Get true name for label */
  true_name = C_true_name (name);

  /* Return 1 if starts with '__builtin_' */
  if ((true_name[0] == '_') &&
      (true_name[1] == '_') &&
      (true_name[2] == 'b') &&
      (true_name[3] == 'u') &&
      (true_name[4] == 'i') &&
      (true_name[5] == 'l') &&
      (true_name[6] == 't') &&
      (true_name[7] == 'i') && (true_name[8] == 'n') && (true_name[9] == '_'))
    {
      return (1);
    }

  /* Otherwise, assume not a builtin function */
  return (0);
}

int
C_get_builtin_parms (char *name, L_Oper * op, char all_parm_type_buf[])
{
  char return_type_buf[TYPE_BUF_SIZE];
  char parm_type_buf[TYPE_BUF_SIZE];
  char *parse_ptr;
  int count;

  /* Get the return type and parameter specifiers from the jsr call_info
   * attribute.  Use smallest buffers for these, since buf size is checked.
   */
  L_get_call_info (NULL, op, op->attr, return_type_buf, all_parm_type_buf,
		   sizeof (return_type_buf));

  /* Go through and handle each parameter */
  parse_ptr = all_parm_type_buf;
  count = 0;
  while (*parse_ptr != 0)
    {
      /* Get the next parameter type */
      L_get_next_param_type (parm_type_buf, &parse_ptr);

      /* Increment parameter count */
      count++;
    }
  return count;
}

void
C_check_builtin_parms (char *name, L_Oper * op, int num_expected)
{
  char all_parm_type_buf[TYPE_BUF_SIZE];
  /* Make sure has the expected number of parameters */
  if (C_get_builtin_parms (name, op, all_parm_type_buf) != num_expected)
    {
      L_print_oper (stderr, op);
      L_punt ("%s: Exactly %i parameter(s) expected!",
	      "C_check_builtin_parms", num_expected);
    }
}

/* Emit the special code HP cc expects for __builtin_va_start().
 *
 * This builtin function *violates* C semantics and *modifies*
 * the first parameter's value, even though it is passed by value.
 *
 * To fix this, Ben (BCC) and I (JCG) developed a patch in Pcode 
 * that puts it into legal C sematics (parm0 is also assigned the
 * return value) and the JSR is marked with use_ret_as_parm0.
 *
 * This code requires this Pcode patch and for the use_ret_as_parm0
 * attribute to be present!
 * 
 * The second parameter must be the address of the last function parameter,
 * not an address into our Lcode stack.  This probably will always work.
 *
 * Thus this code should transform (which will cause HP cc to panic):
 *    mac_P15_i = __builtin_va_start(mac_P0_i, mac_P1_i); 
 *
 * into what HP cc expects:
 *    __builtin_va_start(mac_P15, (prefix)px);
 *
 * ART - 
 * To support Win32, which does not internally support __builtin_va_start,
 * provide enough parameters to the function so that __builtin_va_start
 * can be emulated, but use the address of the real parameter rather
 * then the emulated parameter so that the function receives a pointer
 * to the real C stack, not the emulated stack.
 *
 * Thus this code should transform:
 *    mac_P15_i = __builtin_va_start(mac_P0_i, mac_P1_i, mac_P2_i, mac_P3_i); 
 *
 * into
 *    __builtin_va_start(mac_P15, (prefix)px, mac_P2_i, mac_P3_i);
 */
void
C_emit_builtin_va_start_op (FILE * out, L_Func * fn, L_Oper * op)
{
  char return_type_buf[TYPE_BUF_SIZE];

  char caller_parm_type_buf[TYPE_BUF_SIZE];
  char callee_parm_type_buf[TYPE_BUF_SIZE];
  char parm_type_buf[TYPE_BUF_SIZE];
  char *parse_ptr;
  L_Attr *ret_attr;
  L_Operand *parm0;
  int parm_index;
  int num_parms = 0;

  num_parms =
    C_get_builtin_parms ("C_emit_builtin_va_start_op", op,
			 callee_parm_type_buf);

  if (num_parms != 2 && num_parms != 4)
    {
      L_print_oper (stderr, op);
      L_punt
	("C_emit_builtin_va_start_op: Either 2 o 4 parameter(s) expected!");
    }

  /* Make sure have "use_ret_as_parm0" attribute!  This fixup
   * assumes the Pcode patch has been done!
   */
  if (L_find_attr (op->attr, "use_ret_as_parm0") == NULL)
    {
      fprintf (stderr, "For func %s:\n", fn->name);
      L_print_oper (stderr, op);
      L_punt ("C_emit_builtin_va_start_op: use_ret_as_parm0 attr expected!");
    }

  /* Determine the return register used (will use as parm0) */
  if ((ret_attr = L_find_attr (op->attr, "ret")) == NULL)
    {
      fprintf (stderr, "For func %s:\n", fn->name);
      L_print_oper (stderr, op);
      L_punt ("C_emit_builtin_va_start_op: ret attr expected!");
    }

  /* Make sure have normal or macro register! */
  if ((!L_is_macro (ret_attr->field[0])) && !L_is_reg (ret_attr->field[0]))
    {
      fprintf (stderr, "For func %s:\n", fn->name);
      L_print_oper (stderr, op);
      L_punt ("C_emit_builtin_va_start_op: ret_attr[0] expected to be "
	      "register!");
    }

  /* Get return macro register that we will emit as the first parm */
  parm0 = ret_attr->field[0];

  /* Determine how many parameters there are before vararg */
  parm_index = 0;

  /* Get the return type and parameter specifiers from the function
   * attribute.  Use smallest buffers for these, since buf size is checked.
   */
  L_get_call_info (fn, NULL, fn->attr, return_type_buf, caller_parm_type_buf,
		   sizeof (return_type_buf));

  /* Go through and count each parameter before varargs.
   * If user used varargs.h instead of stdargs.h, there
   * will not be a 'varargs' flag, just take last argument.
   */
  parse_ptr = caller_parm_type_buf;
  parm_index = 0;
  while (*parse_ptr != 0)
    {
      /* Get the next parameter type */
      L_get_next_param_type (parm_type_buf, &parse_ptr);

      /* If vararg, we want the last parameter */
      if (strcmp (parm_type_buf, "vararg") == 0)
	break;

      parm_index++;
    }

  /* 
   *  Should be ready to emit this hacked call now! 
   */
  /* Emit builtin function name */
  fprintf (out, "%s (", C_true_name (op->src[0]->value.l));

  /* Emit new parm0 operand */
  C_emit_operand (out, fn, parm0);

  /* Emit ',' and address of last parameter in function */
  fprintf (out, ", &%sp%i", C_prefix, parm_index - 1);

  /* Print the remaining parms */
  if (num_parms > 2)
    {
      /* Emit the address of the variable that is supposed
         to be returned by the builtin function as the third
         argument so that it can be set inside the function */
      fprintf (out, ", &");
      C_emit_operand (out, fn, parm0);

      if (num_parms > 3)
	{
	  fprintf (out, ", ");

	  /* Skip the first 3 params */
	  parse_ptr = callee_parm_type_buf;
	  for (parm_index = 3; parm_index > 0; --parm_index)
	    {
	      /* Get the next parameter type */
	      L_get_next_param_type (parm_type_buf, &parse_ptr);
	    }
	  C_emit_jsr_parms (out, fn, op, parse_ptr, 3);
	}
    }

  /* Emit closing string */
  fprintf (out, "); /* Patched! */");

  /* Done */
}

/*  

op 16 jsr [] [(l_g_abs ___builtin_stdarg_start)]
__builtin_stdarg_start (((void **) _EM_mac_P8_i), ((int ) _EM_mac_P9_i));

    (op 16 jsr [] [(l_g_abs ___builtin_stdarg_start)] <(tr (mac $P8 ll)(mac $P9 ll))(ret (mac $P16 ll))(param_size (i 0))(call_info (s_l_abs "int%void+PP%int"))>)

*/

void
C_emit_builtin_stdarg_start_op (FILE * out, L_Func * fn, L_Oper * op)
{
  char return_type_buf[TYPE_BUF_SIZE];
  char caller_parm_type_buf[TYPE_BUF_SIZE];
  char callee_parm_type_buf[TYPE_BUF_SIZE];
  char parm_type_buf[TYPE_BUF_SIZE];
  char *parse_ptr;
  L_Attr *tr_attr;
  L_Operand *parm_reg;
  int parm_index;
  int num_parms = 0;

  num_parms =
    C_get_builtin_parms ("C_emit_builtin_stdarg_start_op", op,
			 callee_parm_type_buf);

  if (num_parms > 2 && num_parms != 4)
    {
      L_print_oper (stderr, op);
      L_punt ("C_emit_builtin_stdarg_start_op: "
	      "Either 1, 2 o 4 parameter(s) expected!");
    }


  if ((tr_attr = L_find_attr (op->attr, "tr")) != NULL)
    {

      /* Emit builtin function name */
      fprintf (out, "%s (", C_true_name (op->src[0]->value.l));

      fprintf (out, "(void *)");

      parm_reg = tr_attr->field[0];
      C_emit_operand (out, fn, parm_reg);
      if (num_parms != 1)
	{
	  fprintf (out, ", ");

	  /* This second parameter is suppose to be the last
	     named parameter of the varargs call.  So cycle through
	     the varargs functions' parameters counting.  Producing
	     the parameter register to which that last named parameter
	     is propagated seems to work, but a compiler warning is
	     produced and I don't know how stable that is.  MCM 7/2000 */

	  /*
	     parm_reg = tr_attr->field[1];
	     C_emit_operand (out, fn, parm_reg);
	   */

	  /* Get the return type and parameter specifiers from the
	   * function attribute.  Use smallest buffers for these,
	   * since buf size is checked. 
	   */

	  L_get_call_info (fn, NULL, fn->attr, return_type_buf,
			   caller_parm_type_buf, sizeof (return_type_buf));

	  /* Go through and count each parameter before varargs.
	   * If user used varargs.h instead of stdargs.h, there
	   * will not be a 'varargs' flag, just take last argument.
	   */
	  parse_ptr = caller_parm_type_buf;
	  parm_index = 0;
	  while (*parse_ptr != 0)
	    {
	      /* Get the next parameter type */
	      L_get_next_param_type (parm_type_buf, &parse_ptr);

	      /* If vararg, we want the last parameter */
	      if (strcmp (parm_type_buf, "vararg") == 0)
		break;

	      parm_index++;
	    }

	  /* Emit ',' and address of last parameter in function */
	  fprintf (out, "%sp%i", C_prefix, parm_index - 1);
	}

      /* Emit closing string */
      fprintf (out, "); /* Patched! */");

      /* Done */
    }
  else
    L_punt ("stdarg_start: no tr attribute on builtin_stdarg_start_op!\n");
}


/* Emit the special code gcc expects for __builtin_next_arg().
 *
 * This builtin function *violates* C semantics and reads
 * the address of the first parameter even though it looks
 * like it is passed by value.
 *
 * The first parameter must be the address of the last function parameter,
 * not an address into our Lcode stack.  This probably will always work.
 *
 * Thus this code should transform:
 *    mac_P15_i = (int) __builtin_va_start(mac_P0_i); 
 *
 * into what gcc expects:
 *    mac_P15_i = (int) __builtin_va_start(&_EM_p0);
 * 
 */
void
C_emit_builtin_next_arg_op (FILE * out, L_Func * fn, L_Oper * op)
{
  char return_type_buf[TYPE_BUF_SIZE];
  char all_parm_type_buf[TYPE_BUF_SIZE];
  char parm_type_buf[TYPE_BUF_SIZE];
  char *parse_ptr;
  L_Attr *ret_attr;
  L_Operand *ret_reg;
  int parm_index, ret_type;

  /* Make sure have exactly 1 parameters */
  C_check_builtin_parms ("C_emit_builtin_next_arg_op", op, 1);

  /* Has the return register been specified thru a ret attribute? */
  if ((ret_attr = L_find_attr (op->attr, "ret")) != NULL)
    {
      /* Get return register from first field */
      ret_reg = ret_attr->field[0];

      /* Sanity check, better be a register or macro */
      if (!L_is_reg (ret_reg) && !L_is_macro (ret_reg))
	{
	  L_print_oper (stderr, op);
	  L_punt ("C_emit_builtin_next_arg_op: Unexpected 'ret' attribute "
		  "contents!");
	}
    }

  /* Otherwise, set return_reg to NULL to flag better be void */
  else
    {
      ret_reg = NULL;
    }


  /* Determine how many parameters there are before vararg */
  parm_index = 0;

  /* Get the return type from op attribute to set up return value register */
  L_get_call_info (fn, NULL, op->attr, return_type_buf, all_parm_type_buf,
		   sizeof (return_type_buf));

  /* Convert return type into Lcode ctype (or CTYPE_STRUCT, if required). */
  ret_type = L_convert_type_to_ctype (return_type_buf);

  /* Handle each CTYPE appropriately */
  switch (ret_type)
    {
    case L_CTYPE_INT:
      if (C_native_machine_ctype == L_CTYPE_INT)
	{
	  /* Expect explicit return register specifier */
	  if (ret_reg != NULL)
	    {
	      C_emit_operand (out, fn, ret_reg);
	    }
	  /* Otherwise, punt */
	  else
	    {
	      L_print_oper (stderr, op);
	      L_punt ("C_emit_builtin_next_arg_op: Expect int 'ret' attr!");
	    }
	  fprintf (out, " = (int) ");
	}
      else
	L_punt ("C_emit_builtin_next_arg_op: Unexpect int 'ret' attr!");
      break;

    case L_CTYPE_LLONG:
      if (C_native_machine_ctype == L_CTYPE_LLONG)
	{
	  /* Expect explicit return register specifier */
	  if (ret_reg != NULL)
	    {
	      C_emit_operand (out, fn, ret_reg);
	    }
	  /* Otherwise, punt */
	  else
	    {
	      L_print_oper (stderr, op);
	      L_punt
		("C_emit_builtin_next_arg_op: Expect longlong 'ret' attr!");
	    }
	  fprintf (out, " = (longlong) ");
	}
      else
	L_punt ("C_emit_builtin_next_arg_op: Unexpect longlong 'ret' attr!");
      break;

    case L_CTYPE_VOID:
      /* Do nothing */
      break;

    default:
      L_punt ("C_emit_builtin_next_arg_op: unexpected return type %i!",
	      ret_type);
    }


  /* Get the return type and parameter specifiers from the function
   * attribute.  Use smallest buffers for these, since buf size is checked.
   */
  L_get_call_info (fn, NULL, fn->attr, return_type_buf, all_parm_type_buf,
		   sizeof (return_type_buf));

  /* Go through and count each parameter before varargs */
  parse_ptr = all_parm_type_buf;
  parm_index = 0;
  while (*parse_ptr != 0)
    {
      /* Get the next parameter type */
      L_get_next_param_type (parm_type_buf, &parse_ptr);

      /* If vararg, no more parameters need to be set up for this func */
      if (!strcmp (parm_type_buf, "vararg"))
	break;

      parm_index++;
    }

  /* 
   *  Should be ready to emit this hacked call now! 
   */
  fprintf (out, "%s (&%sp%i); /* Patched! */",
	   C_true_name (op->src[0]->value.l), C_prefix, parm_index - 1);

  /* Done */
}

/* This function figures out what to emit for the builtin functions
 * the host compiler uses.  For varargs functions, some special
 * processing is usually required.
 */
void
C_emit_builtin_jsr_op (FILE * out, L_Func * fn, L_Oper * op)
{
  char *builtin_name;

  /* Verify proper number of operands are present */
  C_check_operands ("C_emit_builtin_jsr_op", op, 0, 1);

  /* For builtin functions, require src[0] to be a label. */
  if (!L_is_label (op->src[0]))
    {
      L_print_oper (stderr, op);
      L_punt ("C_emit_builtin_jsr_op: Expect src[0] to be a label!");
    }

  /* Pick the function to handle the builtin function based on
   * the name.
   */
  builtin_name = C_true_name (op->src[0]->value.l);

  /* Detect HP cc's va_start builtin function */
  if (strcmp (builtin_name, "__builtin_va_start") == 0)
    {
      /* Emit special code HP cc expects */
      C_emit_builtin_va_start_op (out, fn, op);
    }

  /* Detect one of gcc's va_start builtin functions */
  else if (strcmp (builtin_name, "__builtin_saveregs") == 0)
    {
      /* Normal handling fine, use C_emit_normal_jsr */
      C_emit_normal_jsr_op (out, fn, op);
    }

  /* Detect one of gcc's va_start builtin functions */
  else if (strcmp (builtin_name, "__builtin_next_arg") == 0)
    {
      /* Emit special code gcc expects */
      C_emit_builtin_next_arg_op (out, fn, op);
    }

  /* Detect one of cc's alloca builtin functions */
  else if (strcmp (builtin_name, "__builtin_alloca") == 0)
    {
      /* Normal handling fine, use C_emit_normal_jsr */
      C_emit_normal_jsr_op (out, fn, op);
    }

  /* Detect one of gcc's va_start builtin functions */
  else if (strcmp (builtin_name, "__builtin_stdarg_start") == 0 ||
	   strcmp (builtin_name, "__builtin_varargs_start") == 0)
    {
      /* Normal handling fine, use C_emit_normal_jsr */
      C_emit_builtin_stdarg_start_op (out, fn, op);
    }

  /* Otherwise, we don't know how to handle this */
  else
    {
      L_print_oper (stderr, op);
      L_warn ("C_emit_builtin_jsr_op: Unknown builtin function!");
      C_emit_normal_jsr_op (out, fn, op);
    }
}
