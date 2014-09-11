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
 *	File:	ml_impact.c
 *	Author:	Scott A. Mahlke, Wen-mei Hwu
 *	Creation Date:	September 1993
 *
 * Revised by IMPACT Technologies Inc. (John Gyllenhaal) to support 
 * the 'Lcode' model for the 'IMPACT' architecture. -2/99
\*****************************************************************************/

/*****************************************************************************\
 * NOTICE OF CONVENTION                                                      *
 * ------------------------------------------------------------------------- *
 * Mspec links to Pcode, Hcode, and Lcode modules.  In order to allow this   *
 * to take place without requiring front-end modules to link to liblcode.a,  *
 * Mspec code is divided into two classes as follows:                        *
 *  - mi_*.c must not depend on linkage to liblcode.a                        *
 *  - ml_*.c may depend on linkage to liblcode.a                             *
\*****************************************************************************/

/* 10/29/02 REK Adding config.h */
#include <config.h>
#include <Lcode/l_main.h>
#include <machine/m_spec.h>
#include <machine/m_impact.h>
#include <machine/m_hppa.h>
#include <machine/m_sparc.h>
#include <library/i_list.h>

#define JWS_NEW_STACK_MODEL

/*
 *	The way this is set up now is the following:
 *	If my target is ver1 of impact, i use the functions
 *	defined in here, otherwise I call the functions defined
 *	in M_hppa.c since I am using HP macros, HP calling convention,
 *	HP data layout, HP stack layout, etc.
 *
 */

#define M_IMPACT_LCODE_SIZE_VOID        0
#define M_IMPACT_LCODE_SIZE_BIT         1
#define M_IMPACT_LCODE_SIZE_CHAR        8
#define M_IMPACT_LCODE_SIZE_SHORT       16
#define M_IMPACT_LCODE_SIZE_INT         32
#define M_IMPACT_LCODE_SIZE_LONG        32
#define M_IMPACT_LCODE_SIZE_FLOAT       32
#define M_IMPACT_LCODE_SIZE_DOUBLE      64
#define M_IMPACT_LCODE_SIZE_POINTER     32
#define M_IMPACT_LCODE_SIZE_UNION       -1
#define M_IMPACT_LCODE_SIZE_STRUCT      -1
#define M_IMPACT_LCODE_SIZE_BLOCK       -1
#define M_IMPACT_LCODE_SIZE_MAX         64

#define M_IMPACT_LCODE_ALIGN_VOID       -1
#define M_IMPACT_LCODE_ALIGN_BIT        1
#define M_IMPACT_LCODE_ALIGN_CHAR       8
#define M_IMPACT_LCODE_ALIGN_SHORT      16
#define M_IMPACT_LCODE_ALIGN_INT        32
#define M_IMPACT_LCODE_ALIGN_LONG       32
#define M_IMPACT_LCODE_ALIGN_FLOAT      32
#define M_IMPACT_LCODE_ALIGN_DOUBLE     64
#define M_IMPACT_LCODE_ALIGN_POINTER    32
#define M_IMPACT_LCODE_ALIGN_UNION      -1	/* depends on the field */
#define M_IMPACT_LCODE_ALIGN_STRUCT     -1
#define M_IMPACT_LCODE_ALIGN_BLOCK      -1
#define M_IMPACT_LCODE_ALIGN_MAX        64


#define M_IMPACT_SIZE_VOID	0
#define M_IMPACT_SIZE_BIT	1
#define M_IMPACT_SIZE_CHAR	8
#define M_IMPACT_SIZE_SHORT	16
#define M_IMPACT_SIZE_INT	32
#define M_IMPACT_SIZE_LONG	32
#define M_IMPACT_SIZE_FLOAT	32
#define M_IMPACT_SIZE_DOUBLE	64
#define M_IMPACT_SIZE_POINTER	32
#define M_IMPACT_SIZE_UNION	-1
#define M_IMPACT_SIZE_STRUCT	-1
#define M_IMPACT_SIZE_BLOCK	-1
#define M_IMPACT_SIZE_MAX	64

#define M_IMPACT_ALIGN_VOID	-1
#define M_IMPACT_ALIGN_BIT	1
#define M_IMPACT_ALIGN_CHAR	8
#define M_IMPACT_ALIGN_SHORT	16
#define M_IMPACT_ALIGN_INT	32
#define M_IMPACT_ALIGN_LONG	32
#define M_IMPACT_ALIGN_FLOAT	32
#define M_IMPACT_ALIGN_DOUBLE	64
#define M_IMPACT_ALIGN_POINTER	32
#define M_IMPACT_ALIGN_UNION	-1	/* depends on the field */
#define M_IMPACT_ALIGN_STRUCT	-1
#define M_IMPACT_ALIGN_BLOCK	-1
#define M_IMPACT_ALIGN_MAX	64

/* model compiling for, internal to this Mspec only */
extern int M_impact_model;

Set Set_impact_fragile_macro = NULL;

#define M_IMPACT_MAX_FNVAR_REG	4
#define MIN_PARAM_SIZE	(M_IMPACT_MAX_FNVAR_REG * M_IMPACT_SIZE_INT)

#define M_IMPACT_R0_I		0	/* $P0 */
#define M_IMPACT_R0_F		1	/* $P1 */
#define M_IMPACT_R0_F2		2	/* $P2 */
#define M_IMPACT_ST		0	/* $P0 */
#define M_IMPACT_ARG_I		4
#define M_IMPACT_ARG_F		(M_IMPACT_ARG_I + M_IMPACT_MAX_FNVAR_REG)
#define M_IMPACT_ARG_F2		(M_IMPACT_ARG_F + M_IMPACT_MAX_FNVAR_REG)

/* Model IMPACT/Lcode after the passing conventions used for HPPA,
 * at least initially. -ITI (JCG) 2/99 
 */
#define M_IMPACT_LCODE_MAX_FNVAR_REG            4
#define M_IMPACT_LCODE_SMALL_STRUCT_MAX         64
#define M_IMPACT_LCODE_MIN_PARAM_SIZE           (16 * 8)

/* incoming and outgoing parameters */
#define M_IMPACT_LCODE_INT_BASE         0
#define M_IMPACT_LCODE_FLT_BASE         4

#define M_IMPACT_LCODE_RET_I32          15	/* $P15 */
#define M_IMPACT_LCODE_RET_I64          15	/* $P15 */
#define M_IMPACT_LCODE_RET_ST           15	/* $P15 */
#define M_IMPACT_LCODE_RET_F            4	/* $P4  */

/*
 *	The following functions are for documentation only.
 *	The function overhead is too great for actual usage.
 */
void
M_impact_void (M_Type type)
{
  if (M_impact_model == M_IM_LCODE)
    {
      type->type = M_TYPE_VOID;
      type->unsign = 1;
      type->align = M_IMPACT_LCODE_ALIGN_VOID;
      type->size = M_IMPACT_LCODE_SIZE_VOID;
      type->nbits = 0;
    }
  else if (M_impact_model == M_IM_VER_1)
    {
      type->type = M_TYPE_VOID;
      type->unsign = 1;
      type->align = M_IMPACT_ALIGN_VOID;
      type->size = M_IMPACT_SIZE_VOID;
      type->nbits = 0;
    }
  else if ((M_impact_model == M_IM_HP_LCODE) ||
	   (M_impact_model == M_IM_HP_MCODE))
    {
      M_hppa_void (type);
    }
  else if ((M_impact_model == M_IM_SPARC_LCODE) ||
	   (M_impact_model == M_IM_SPARC_MCODE))
    {
      M_sparc_void (type);
    }
  else
    {
      M_assert (0, "M_impact_void: illegal machine model");
    }
}

void
M_impact_bit_long (M_Type type, int n)
{
  if (M_impact_model == M_IM_LCODE)
    {
      type->type = M_TYPE_BIT_LONG;
      type->unsign = 1;
      type->align = M_IMPACT_LCODE_ALIGN_BIT;
      type->size = n * M_IMPACT_LCODE_SIZE_BIT;
      type->nbits = n * M_IMPACT_LCODE_SIZE_BIT;
      M_assert ((n <= 32),
		"M_bit_long: do not allow bit field of more than 32 bits");
    }
  else if (M_impact_model == M_IM_VER_1)
    {
      type->type = M_TYPE_BIT_LONG;
      type->unsign = 0;
      type->align = M_IMPACT_ALIGN_BIT;
      type->size = n * M_IMPACT_SIZE_BIT;
      type->nbits = n * M_IMPACT_SIZE_BIT;
      M_assert ((n <= 32),
		"M_bit_long: do not allow bit field of more than 32 bits");
    }
  else if ((M_impact_model == M_IM_HP_LCODE) ||
	   (M_impact_model == M_IM_HP_MCODE))
    {
      M_hppa_bit_long (type, n);
    }
  else if ((M_impact_model == M_IM_SPARC_LCODE) ||
	   (M_impact_model == M_IM_SPARC_MCODE))
    {
      M_sparc_bit_long (type, n);
    }
  else
    {
      M_assert (0, "M_impact_bit_long: illegal machine model");
    }
}

void
M_impact_bit_int (M_Type type, int n)
{
  if (M_impact_model == M_IM_LCODE)
    {
      type->type = M_TYPE_BIT_INT;
      type->unsign = 1;
      type->align = M_IMPACT_LCODE_ALIGN_BIT;
      type->size = n * M_IMPACT_LCODE_SIZE_BIT;
      type->nbits = n * M_IMPACT_LCODE_SIZE_BIT;
      M_assert ((n <= 32),
		"M_bit_int: do not allow bit field of more than 32 bits");
    }
  else if (M_impact_model == M_IM_VER_1)
    {
      type->type = M_TYPE_BIT_INT;
      type->unsign = 0;
      type->align = M_IMPACT_ALIGN_BIT;
      type->size = n * M_IMPACT_SIZE_BIT;
      type->nbits = n * M_IMPACT_SIZE_BIT;
      M_assert ((n <= 32),
		"M_bit_int: do not allow bit field of more than 32 bits");
    }
  else if ((M_impact_model == M_IM_HP_LCODE) ||
	   (M_impact_model == M_IM_HP_MCODE))
    {
      M_hppa_bit_int (type, n);
    }
  else if ((M_impact_model == M_IM_SPARC_LCODE) ||
	   (M_impact_model == M_IM_SPARC_MCODE))
    {
      M_sparc_bit_int (type, n);
    }
  else
    {
      M_assert (0, "M_impact_bit_int: illegal machine model");
    }
}

void
M_impact_bit_short (M_Type type, int n)
{
  if (M_impact_model == M_IM_LCODE)
    {
      type->type = M_TYPE_BIT_SHORT;
      type->unsign = 1;
      type->align = M_IMPACT_LCODE_ALIGN_BIT;
      type->size = n * M_IMPACT_LCODE_SIZE_BIT;
      type->nbits = n * M_IMPACT_LCODE_SIZE_BIT;
      M_assert ((n <= 16),
		"M_bit_long: do not allow bit field of more than 16 bits");
    }
  else if (M_impact_model == M_IM_VER_1)
    {
      type->type = M_TYPE_BIT_SHORT;
      type->unsign = 0;
      type->align = M_IMPACT_ALIGN_BIT;
      type->size = n * M_IMPACT_SIZE_BIT;
      type->nbits = n * M_IMPACT_SIZE_BIT;
      M_assert ((n <= 16),
		"M_bit_long: do not allow bit field of more than 16 bits");
    }
  else if ((M_impact_model == M_IM_HP_LCODE) ||
	   (M_impact_model == M_IM_HP_MCODE))
    {
      M_hppa_bit_short (type, n);
    }
  else if ((M_impact_model == M_IM_SPARC_LCODE) ||
	   (M_impact_model == M_IM_SPARC_MCODE))
    {
      M_sparc_bit_short (type, n);
    }
  else
    {
      M_assert (0, "M_impact_bit_short: illegal machine model");
    }
}

void
M_impact_bit_char (M_Type type, int n)
{
  if (M_impact_model == M_IM_LCODE)
    {
      type->type = M_TYPE_BIT_CHAR;
      type->unsign = 1;
      type->align = M_IMPACT_LCODE_ALIGN_BIT;
      type->size = n * M_IMPACT_LCODE_SIZE_BIT;
      type->nbits = n * M_IMPACT_LCODE_SIZE_BIT;
      M_assert ((n <= 8),
		"M_bit_char: do not allow bit field of more than 8 bits");
    }
  else if (M_impact_model == M_IM_VER_1)
    {
      type->type = M_TYPE_BIT_CHAR;
      type->unsign = 0;
      type->align = M_IMPACT_ALIGN_BIT;
      type->size = n * M_IMPACT_SIZE_BIT;
      type->nbits = n * M_IMPACT_SIZE_BIT;
      M_assert ((n <= 8),
		"M_bit_char: do not allow bit field of more than 8 bits");
    }
  else if ((M_impact_model == M_IM_HP_LCODE) ||
	   (M_impact_model == M_IM_HP_MCODE))
    {
      M_hppa_bit_char (type, n);
    }
  else if ((M_impact_model == M_IM_SPARC_LCODE) ||
	   (M_impact_model == M_IM_SPARC_MCODE))
    {
      M_sparc_bit_char (type, n);
    }
  else
    {
      M_assert (0, "M_impact_bit_char: illegal machine model");
    }
}

void
M_impact_float (M_Type type, int unsign)
{
  if (M_impact_model == M_IM_LCODE)
    {
      type->type = M_TYPE_FLOAT;
      type->unsign = (unsign != 0);
      type->align = M_IMPACT_LCODE_ALIGN_FLOAT;
      type->size = M_IMPACT_LCODE_SIZE_FLOAT;
      type->nbits = M_IMPACT_LCODE_SIZE_FLOAT;
    }
  else if (M_impact_model == M_IM_VER_1)
    {
      type->type = M_TYPE_FLOAT;
      type->unsign = (unsign != 0);
      type->align = M_IMPACT_ALIGN_FLOAT;
      type->size = M_IMPACT_SIZE_FLOAT;
      type->nbits = M_IMPACT_SIZE_FLOAT;
    }
  else if ((M_impact_model == M_IM_HP_LCODE) ||
	   (M_impact_model == M_IM_HP_MCODE))
    {
      M_hppa_float (type, unsign);
    }
  else if ((M_impact_model == M_IM_SPARC_LCODE) ||
	   (M_impact_model == M_IM_SPARC_MCODE))
    {
      M_sparc_float (type, unsign);
    }
  else
    {
      M_assert (0, "M_impact_float: illegal machine model");
    }
}

void
M_impact_double (M_Type type, int unsign)
{
  if (M_impact_model == M_IM_LCODE)
    {
      type->type = M_TYPE_DOUBLE;
      type->unsign = (unsign != 0);
      type->align = M_IMPACT_LCODE_ALIGN_DOUBLE;
      type->size = M_IMPACT_LCODE_SIZE_DOUBLE;
      type->nbits = M_IMPACT_LCODE_SIZE_DOUBLE;
    }
  else if (M_impact_model == M_IM_VER_1)
    {
      type->type = M_TYPE_DOUBLE;
      type->unsign = (unsign != 0);
      type->align = M_IMPACT_ALIGN_DOUBLE;
      type->size = M_IMPACT_SIZE_DOUBLE;
      type->nbits = M_IMPACT_SIZE_DOUBLE;
    }
  else if ((M_impact_model == M_IM_HP_LCODE) ||
	   (M_impact_model == M_IM_HP_MCODE))
    {
      M_hppa_double (type, unsign);
    }
  else if ((M_impact_model == M_IM_SPARC_LCODE) ||
	   (M_impact_model == M_IM_SPARC_MCODE))
    {
      M_sparc_double (type, unsign);
    }
  else
    {
      M_assert (0, "M_impact_double: illegal machine model");
    }
}

void
M_impact_pointer (M_Type type)
{
  if (M_impact_model == M_IM_LCODE)
    {
      type->type = M_TYPE_POINTER;
      type->unsign = 1;
      type->align = M_IMPACT_LCODE_ALIGN_POINTER;
      type->size = M_IMPACT_LCODE_SIZE_POINTER;
      type->nbits = M_IMPACT_LCODE_SIZE_POINTER;
    }
  else if (M_impact_model == M_IM_VER_1)
    {
      type->type = M_TYPE_POINTER;
      type->unsign = 1;
      type->align = M_IMPACT_ALIGN_POINTER;
      type->size = M_IMPACT_SIZE_POINTER;
      type->nbits = M_IMPACT_SIZE_POINTER;
    }
  else if ((M_impact_model == M_IM_HP_LCODE) ||
	   (M_impact_model == M_IM_HP_MCODE))
    {
      M_hppa_pointer (type);
    }
  else if ((M_impact_model == M_IM_SPARC_LCODE) ||
	   (M_impact_model == M_IM_SPARC_MCODE))
    {
      M_sparc_pointer (type);
    }
  else
    {
      M_assert (0, "M_impact_pointer: illegal machine model");
    }
}

int
M_impact_eval_type (M_Type type, M_Type ntype)
{
  if (M_impact_model == M_IM_LCODE)
    {
      switch (type->type)
	{
	case M_TYPE_VOID:
	  M_impact_void (ntype);
	  return -1;		/* can not be evaluated */
	case M_TYPE_BIT_LONG:
	case M_TYPE_BIT_CHAR:
	case M_TYPE_CHAR:
	case M_TYPE_SHORT:
	case M_TYPE_INT:
	case M_TYPE_LONG:
	case M_TYPE_POINTER:
	case M_TYPE_BLOCK:	/* the starting address of array is used */
	  M_impact_int (ntype, type->unsign);
	  return M_TYPE_INT;
	case M_TYPE_FLOAT:
	  M_impact_float (ntype, type->unsign);
	  return M_TYPE_FLOAT;
	case M_TYPE_DOUBLE:
	  M_impact_double (ntype, type->unsign);
	  return M_TYPE_DOUBLE;
	case M_TYPE_UNION:
	case M_TYPE_STRUCT:
	  *ntype = *type;
	  return type->type;
	default:
	  return (-1);
	}
    }
  else if (M_impact_model == M_IM_VER_1)
    {
      switch (type->type)
	{
	case M_TYPE_VOID:
	  M_impact_void (ntype);
	  return -1;		/* can not be evaluated */
	case M_TYPE_BIT_LONG:
	case M_TYPE_BIT_CHAR:
	case M_TYPE_CHAR:
	case M_TYPE_SHORT:
	case M_TYPE_INT:
	case M_TYPE_LONG:
	case M_TYPE_POINTER:
	case M_TYPE_BLOCK:	/* the starting address of array is used */
	  M_impact_int (ntype, type->unsign);
	  return M_TYPE_INT;
	case M_TYPE_FLOAT:
	  M_impact_float (ntype, type->unsign);
	  return M_TYPE_FLOAT;
	case M_TYPE_DOUBLE:
	  M_impact_double (ntype, type->unsign);
	  return M_TYPE_DOUBLE;
	case M_TYPE_UNION:
	case M_TYPE_STRUCT:
	  *ntype = *type;
	  return type->type;
	}
    }
  else if ((M_impact_model == M_IM_HP_LCODE) ||
	   (M_impact_model == M_IM_HP_MCODE))
    {
      return (M_hppa_eval_type (type, ntype));
    }
  else if ((M_impact_model == M_IM_SPARC_LCODE) ||
	   (M_impact_model == M_IM_SPARC_MCODE))
    {
      return (M_sparc_eval_type (type, ntype));
    }
  else
    {
      M_assert (0, "M_impact_eval_type: illegal machine model");
    }
  return (0);
}

int
M_impact_eval_type2 (M_Type type, M_Type ntype)
{
  if (M_impact_model == M_IM_LCODE)
    {
      switch (type->type)
	{
	case M_TYPE_VOID:
	  M_impact_void (ntype);
	  return -1;		/* can not be evaluated */
	case M_TYPE_BIT_CHAR:
	case M_TYPE_CHAR:
	  M_impact_char (ntype, type->unsign);
	  return M_TYPE_CHAR;
	case M_TYPE_SHORT:
	  M_impact_short (ntype, type->unsign);
	  return M_TYPE_SHORT;
	case M_TYPE_BLOCK:	/* the starting address of array is used */
	case M_TYPE_INT:
	  M_impact_int (ntype, type->unsign);
	  return M_TYPE_INT;
	case M_TYPE_BIT_LONG:
	case M_TYPE_LONG:
	  M_impact_long (ntype, type->unsign);
	  return M_TYPE_LONG;
	case M_TYPE_POINTER:
	  M_impact_pointer (ntype);
	  return M_TYPE_POINTER;
	case M_TYPE_FLOAT:
	  M_impact_float (ntype, type->unsign);
	  return M_TYPE_FLOAT;
	case M_TYPE_DOUBLE:
	  M_impact_double (ntype, type->unsign);
	  return M_TYPE_DOUBLE;
	case M_TYPE_UNION:
	case M_TYPE_STRUCT:
	  *ntype = *type;
	  return type->type;
	}
    }
  else if (M_impact_model == M_IM_VER_1)
    {
      switch (type->type)
	{
	case M_TYPE_VOID:
	  M_impact_void (ntype);
	  return -1;		/* can not be evaluated */
	case M_TYPE_BIT_CHAR:
	case M_TYPE_CHAR:
	  M_impact_char (ntype, type->unsign);
	  return M_TYPE_CHAR;
	case M_TYPE_SHORT:
	  M_impact_short (ntype, type->unsign);
	  return M_TYPE_SHORT;
	case M_TYPE_BLOCK:	/* the starting address of array is used */
	case M_TYPE_INT:
	  M_impact_int (ntype, type->unsign);
	  return M_TYPE_INT;
	case M_TYPE_BIT_LONG:
	case M_TYPE_LONG:
	  M_impact_long (ntype, type->unsign);
	  return M_TYPE_LONG;
	case M_TYPE_POINTER:
	  M_impact_pointer (ntype);
	  return M_TYPE_POINTER;
	case M_TYPE_FLOAT:
	  M_impact_float (ntype, type->unsign);
	  return M_TYPE_FLOAT;
	case M_TYPE_DOUBLE:
	  M_impact_double (ntype, type->unsign);
	  return M_TYPE_DOUBLE;
	case M_TYPE_UNION:
	case M_TYPE_STRUCT:
	  *ntype = *type;
	  return type->type;
	}
    }
  else if ((M_impact_model == M_IM_HP_LCODE) ||
	   (M_impact_model == M_IM_HP_MCODE))
    {
      return (M_hppa_eval_type (type, ntype));
    }
  else if ((M_impact_model == M_IM_SPARC_LCODE) ||
	   (M_impact_model == M_IM_SPARC_MCODE))
    {
      return (M_sparc_eval_type (type, ntype));
    }
  else
    {
      M_assert (0, "M_impact_eval_type: illegal machine model");
    }
  return (0);
}

int
M_impact_call_type (M_Type type, M_Type ntype)
{
  if (M_impact_model == M_IM_LCODE)
    {
      switch (type->type)
	{
	case M_TYPE_VOID:
	  M_impact_void (ntype);
	  return -1;		/* can not be evaluated */
	case M_TYPE_BIT_LONG:
	case M_TYPE_BIT_CHAR:
	case M_TYPE_CHAR:
	case M_TYPE_SHORT:
	case M_TYPE_INT:
	case M_TYPE_LONG:
	case M_TYPE_POINTER:
	case M_TYPE_BLOCK:	/* the starting address of array is used */
	  M_impact_int (ntype, type->unsign);
	  return M_TYPE_INT;
	  /* BCC - don't promote float to double now - 8/5/96 */
	case M_TYPE_FLOAT:
	  M_impact_float (ntype, type->unsign);
	  return M_TYPE_FLOAT;
	case M_TYPE_DOUBLE:
	  M_impact_double (ntype, type->unsign);
	  return M_TYPE_DOUBLE;
	case M_TYPE_UNION:
	case M_TYPE_STRUCT:
	  *ntype = *type;
	  return type->type;
	default:
	  return (-1);
	}
    }
  else if (M_impact_model == M_IM_VER_1)
    {
      switch (type->type)
	{
	case M_TYPE_VOID:
	  M_impact_void (ntype);
	  return -1;		/* can not be evaluated */
	case M_TYPE_BIT_LONG:
	case M_TYPE_BIT_CHAR:
	case M_TYPE_CHAR:
	case M_TYPE_SHORT:
	case M_TYPE_INT:
	case M_TYPE_LONG:
	case M_TYPE_POINTER:
	case M_TYPE_BLOCK:	/* the starting address of array is used */
	  M_impact_int (ntype, type->unsign);
	  return M_TYPE_INT;
	  /* BCC - don't promote float to double now - 8/5/96 */
	case M_TYPE_FLOAT:
	  M_impact_float (ntype, type->unsign);
	  return M_TYPE_FLOAT;
	case M_TYPE_DOUBLE:
	  M_impact_double (ntype, type->unsign);
	  return M_TYPE_DOUBLE;
	case M_TYPE_UNION:
	case M_TYPE_STRUCT:
	  *ntype = *type;
	  return type->type;
	}
    }
  else if ((M_impact_model == M_IM_HP_LCODE) ||
	   (M_impact_model == M_IM_HP_MCODE))
    {
      return (M_hppa_call_type (type, ntype));
    }
  else if ((M_impact_model == M_IM_SPARC_LCODE) ||
	   (M_impact_model == M_IM_SPARC_MCODE))
    {
      return (M_sparc_call_type (type, ntype));
    }
  else
    {
      M_assert (0, "M_impact_call_type: illegal machine model");
    }
  return (0);
}

int
M_impact_call_type2 (M_Type type, M_Type ntype)
{
  if (M_impact_model == M_IM_LCODE)
    {
      switch (type->type)
	{
	case M_TYPE_VOID:
	  M_impact_void (ntype);
	  return -1;		/* can not be evaluated */
	case M_TYPE_BIT_CHAR:
	case M_TYPE_CHAR:
	  M_impact_char (ntype, type->unsign);
	  return M_TYPE_CHAR;
	case M_TYPE_SHORT:
	  M_impact_short (ntype, type->unsign);
	  return M_TYPE_SHORT;
	case M_TYPE_BLOCK:	/* the starting address of array is used */
	case M_TYPE_INT:
	  M_impact_int (ntype, type->unsign);
	  return M_TYPE_INT;
	case M_TYPE_BIT_LONG:
	case M_TYPE_LONG:
	  M_impact_long (ntype, type->unsign);
	  return M_TYPE_LONG;
	case M_TYPE_POINTER:
	  M_impact_pointer (ntype);
	  return M_TYPE_POINTER;
	  /* BCC - don't promote float to double now - 8/5/96 */
	case M_TYPE_FLOAT:
	  M_impact_float (ntype, type->unsign);
	  return M_TYPE_FLOAT;
	case M_TYPE_DOUBLE:
	  M_impact_double (ntype, type->unsign);
	  return M_TYPE_DOUBLE;
	case M_TYPE_UNION:
	case M_TYPE_STRUCT:
	  *ntype = *type;
	  return type->type;
	default:
	  return (-1);
	}
    }
  else if (M_impact_model == M_IM_VER_1)
    {
      switch (type->type)
	{
	case M_TYPE_VOID:
	  M_impact_void (ntype);
	  return -1;		/* can not be evaluated */
	case M_TYPE_BIT_CHAR:
	case M_TYPE_CHAR:
	  M_impact_char (ntype, type->unsign);
	  return M_TYPE_CHAR;
	case M_TYPE_SHORT:
	  M_impact_short (ntype, type->unsign);
	  return M_TYPE_SHORT;
	case M_TYPE_BLOCK:	/* the starting address of array is used */
	case M_TYPE_INT:
	  M_impact_int (ntype, type->unsign);
	  return M_TYPE_INT;
	case M_TYPE_BIT_LONG:
	case M_TYPE_LONG:
	  M_impact_long (ntype, type->unsign);
	  return M_TYPE_LONG;
	case M_TYPE_POINTER:
	  M_impact_pointer (ntype);
	  return M_TYPE_POINTER;
	  /* BCC - don't promote float to double now - 8/5/96 */
	case M_TYPE_FLOAT:
	  M_impact_float (ntype, type->unsign);
	  return M_TYPE_FLOAT;
	case M_TYPE_DOUBLE:
	  M_impact_double (ntype, type->unsign);
	  return M_TYPE_DOUBLE;
	case M_TYPE_UNION:
	case M_TYPE_STRUCT:
	  *ntype = *type;
	  return type->type;
	}
    }
  else if ((M_impact_model == M_IM_HP_LCODE) ||
	   (M_impact_model == M_IM_HP_MCODE))
    {
      return (M_hppa_call_type (type, ntype));
    }
  else if ((M_impact_model == M_IM_SPARC_LCODE) ||
	   (M_impact_model == M_IM_SPARC_MCODE))
    {
      return (M_sparc_call_type (type, ntype));
    }
  else
    {
      M_assert (0, "M_impact_call_type: illegal machine model");
    }
  return (0);
}

void
M_impact_array_layout (M_Type type, int *offset)
{
  if (M_impact_model == M_IM_LCODE)
    {
      *offset = 0;		/* assume first element is aligned */
    }
  else if (M_impact_model == M_IM_VER_1)
    {
      *offset = 0;		/* assume first element is aligned */
    }
  else if ((M_impact_model == M_IM_HP_LCODE) ||
	   (M_impact_model == M_IM_HP_MCODE))
    {
      M_hppa_array_layout (type, offset);
    }
  else if ((M_impact_model == M_IM_SPARC_LCODE) ||
	   (M_impact_model == M_IM_SPARC_MCODE))
    {
      M_sparc_array_layout (type, offset);
    }
  else
    {
      M_assert (0, "M_impact_array_layout: illegal machine model");
    }
}

int
M_impact_array_align (M_Type type)
{
  if (M_impact_model == M_IM_LCODE)
    {
      return type->align;
    }
  else if (M_impact_model == M_IM_VER_1)
    {
      return type->align;
    }
  else if ((M_impact_model == M_IM_HP_LCODE) ||
	   (M_impact_model == M_IM_HP_MCODE))
    {
      return (M_hppa_array_align (type));
    }
  else if ((M_impact_model == M_IM_SPARC_LCODE) ||
	   (M_impact_model == M_IM_SPARC_MCODE))
    {
      return (M_sparc_array_align (type));
    }
  else
    {
      M_assert (0, "M_impact_array_align: illegal machine model");
      return (0);
    }
}

int
M_impact_array_size (M_Type type, int dim)
{
  if (M_impact_model == M_IM_LCODE)
    {
      int mod, size, align;
      size = type->size;
      align = type->align;
      mod = size % align;
      if (mod != 0)
	size += (align - mod);
      return (size * dim);
    }
  else if (M_impact_model == M_IM_VER_1)
    {
      int mod, size, align;
      size = type->size;
      align = type->align;
      mod = size % align;
      if (mod != 0)
	size += (align - mod);
      return (size * dim);
    }
  else if ((M_impact_model == M_IM_HP_LCODE) ||
	   (M_impact_model == M_IM_HP_MCODE))
    {
      return (M_hppa_array_size (type, dim));
    }
  else if ((M_impact_model == M_IM_SPARC_LCODE) ||
	   (M_impact_model == M_IM_SPARC_MCODE))
    {
      return (M_sparc_array_size (type, dim));
    }
  else
    {
      M_assert (0, "M_impact_array_size: illegal machine model");
      return (0);
    }
}

void
M_impact_union_layout (int n, _M_Type * type, int *offset, int *bit_offset)
{
  if (M_impact_model == M_IM_LCODE)
    {
      int i;
      for (i = 0; i < n; i++)
	{			/* assume the union is aligned */
	  offset[i] = 0;
	  bit_offset[i] = 0;
	}
    }
  else if (M_impact_model == M_IM_VER_1)
    {
      int i;
      for (i = 0; i < n; i++)
	{			/* assume the union is aligned */
	  offset[i] = 0;
	  bit_offset[i] = 0;
	}
    }
  else if ((M_impact_model == M_IM_HP_LCODE) ||
	   (M_impact_model == M_IM_HP_MCODE))
    {
      M_hppa_union_layout (n, type, offset, bit_offset);
    }
  else if ((M_impact_model == M_IM_SPARC_LCODE) ||
	   (M_impact_model == M_IM_SPARC_MCODE))
    {
      M_sparc_union_layout (n, type, offset, bit_offset);
    }
  else
    {
      M_assert (0, "M_impact_union_layout: illegal machine model");
    }
}

int
M_impact_union_align (int n, _M_Type * type)
{
  if (M_impact_model == M_IM_LCODE)
    {
      int i, max;
      max = 0;
      for (i = 0; i < n; i++)
	{
	  int aln = type[i].align;
	  if (aln > max)
	    max = aln;
	}
      /*
       *  align to at least byte boundary.
       */
      if (max < M_IMPACT_LCODE_ALIGN_CHAR)
	max = M_IMPACT_LCODE_ALIGN_CHAR;
      return max;
    }
  else if (M_impact_model == M_IM_VER_1)
    {
      int i, max;
      max = 0;
      for (i = 0; i < n; i++)
	{
	  int aln = type[i].align;
	  if (aln > max)
	    max = aln;
	}
      /*
       *  align to at least integer boundary.
       */
      if (max < M_IMPACT_ALIGN_INT)
	max = M_IMPACT_ALIGN_INT;
      return max;
    }
  else if ((M_impact_model == M_IM_HP_LCODE) ||
	   (M_impact_model == M_IM_HP_MCODE))
    {
      return (M_hppa_union_align (n, type));
    }
  else if ((M_impact_model == M_IM_SPARC_LCODE) ||
	   (M_impact_model == M_IM_SPARC_MCODE))
    {
      return (M_sparc_union_align (n, type));
    }
  else
    {
      M_assert (0, "M_impact_union_align: illegal machine model");
      return (0);
    }
}

int
M_impact_union_size (int n, _M_Type * type)
{
  if (M_impact_model == M_IM_LCODE)
    {
      int i, aln, max_size, max_align;
      max_size = 0;
      max_align = 0;
      for (i = 0; i < n; i++)
	{
	  int size;
	  size = type[i].size;
	  if (size > max_size)
	    max_size = size;
	  aln = type[i].align;
	  if (aln > max_align)
	    max_align = aln;
	}
      /*
       *  align to at least byte boundary.
       */
      if (max_align < M_IMPACT_LCODE_ALIGN_CHAR)
	max_align = M_IMPACT_LCODE_ALIGN_CHAR;
      /* need to increment to the max. align for future array extension */
      i = max_size % max_align;
      if (i != 0)
	max_size += (max_align - i);
      return max_size;
    }
  else if (M_impact_model == M_IM_VER_1)
    {
      int i, aln, max_size, max_align;
      max_size = 0;
      max_align = 0;
      for (i = 0; i < n; i++)
	{
	  int size;
	  size = type[i].size;
	  if (size > max_size)
	    max_size = size;
	  aln = type[i].align;
	  if (aln > max_align)
	    max_align = aln;
	}
      /*
       *  align to at least integer boundary.
       */
      if (max_align < M_IMPACT_ALIGN_INT)
	max_align = M_IMPACT_ALIGN_INT;
      /* need to increment to the max. align for future array extension */
      i = max_size % max_align;
      if (i != 0)
	max_size += (max_align - i);
      return max_size;
    }
  else if ((M_impact_model == M_IM_HP_LCODE) ||
	   (M_impact_model == M_IM_HP_MCODE))
    {
      return (M_hppa_union_size (n, type));
    }
  else if ((M_impact_model == M_IM_SPARC_LCODE) ||
	   (M_impact_model == M_IM_SPARC_MCODE))
    {
      return (M_sparc_union_size (n, type));
    }
  else
    {
      M_assert (0, "M_impact_union_size: illegal machine model");
      return (0);
    }
}

void
M_impact_struct_layout (int n, _M_Type * type, int *base, int *bit_offset)
{
  if (M_impact_model == M_IM_LCODE)
    {
      int i, offset;
      int mod, size, align, mod_word, mod_type;

      offset = 0;		/* assume initially aligned */
      for (i = 0; i < n; i++)
	{
	  size = type[i].size;
	  align = type[i].align;
	  M_assert ((size != 0) && (align != 0),
		    "M_struct_layout: void is not allowed in structure");
	  /*
	   *  need to treat bit fields specially.
	   *  keep them in a word when possible.
	   */
	  mod_word = offset % M_IMPACT_LCODE_SIZE_INT;
	  if (type[i].type == M_TYPE_BIT_CHAR)
	    {
	      if ((mod_word + size) > M_IMPACT_LCODE_SIZE_INT)
		offset += (M_IMPACT_LCODE_SIZE_INT - mod_word);
	      else
		{
		  mod_type = offset % M_IMPACT_LCODE_SIZE_CHAR;
		  if ((mod_type + size) > M_IMPACT_LCODE_SIZE_CHAR)
		    {
		      type[i].type = M_TYPE_BIT_SHORT;
		    }
		}
	    }
	  if (type[i].type == M_TYPE_BIT_SHORT)
	    {
	      if ((mod_word + size) > M_IMPACT_LCODE_SIZE_INT)
		offset += (M_IMPACT_LCODE_SIZE_INT - mod_word);
	      else
		{
		  mod_type = offset % M_IMPACT_LCODE_SIZE_SHORT;
		  if ((mod_type + size) > M_IMPACT_LCODE_SIZE_SHORT)
		    {
		      type[i].type = M_TYPE_BIT_LONG;
		    }
		}
	    }
	  else if (type[i].type == M_TYPE_BIT_LONG)
	    {
	      if ((mod_word + size) > M_IMPACT_LCODE_SIZE_INT)
		offset += (M_IMPACT_LCODE_SIZE_INT - mod_word);
	    }
	  mod = offset % align;	/* align to what the field */
	  if (mod != 0)		/* needs to start from */
	    offset += (align - mod);

	  if (type[i].type == M_TYPE_BIT_CHAR)
	    {
	      int mod = offset % M_IMPACT_LCODE_SIZE_INT;

	      bit_offset[i] = offset - mod +
		(M_IMPACT_LCODE_SIZE_INT - mod - size);
	      base[i] = offset & (~(M_IMPACT_LCODE_SIZE_CHAR - 1));
	    }
	  else if (type[i].type == M_TYPE_BIT_SHORT)
	    {
	      int mod = offset % M_IMPACT_LCODE_SIZE_INT;

	      bit_offset[i] = offset - mod +
		(M_IMPACT_LCODE_SIZE_INT - mod - size);
	      base[i] = offset & (~(M_IMPACT_LCODE_SIZE_SHORT - 1));

	    }
	  else if (type[i].type == M_TYPE_BIT_LONG)
	    {
	      int mod = offset % M_IMPACT_LCODE_SIZE_INT;

	      bit_offset[i] = offset - mod +
		(M_IMPACT_LCODE_SIZE_INT - mod - size);
	      base[i] = offset & (~(M_IMPACT_LCODE_SIZE_LONG - 1));
	    }
	  else
	    {
	      base[i] = offset;
	      bit_offset[i] = 0;
	    }

	  offset += size;	/* allocate space */
	}
    }
  else if (M_impact_model == M_IM_VER_1)
    {
      int i, offset;
      int mod, size, align;
      offset = 0;		/* assume initially aligned */
      for (i = 0; i < n; i++)
	{
	  size = type[i].size;
	  align = type[i].align;
	  M_assert ((size != 0) && (align != 0),
		    "M_struct_layout: void is not allowed in structure");
	  /*
	   *  need to treat bit fields specially.
	   *  keep them in a word when possible.
	   */
	  if ((i != 0) && (type[i].type != M_TYPE_BIT_LONG) &&
	      (type[i].type != M_TYPE_BIT_CHAR))
	    {
	      if (type[i - 1].type == M_TYPE_BIT_LONG)
		{
		  mod = offset % M_IMPACT_ALIGN_INT;
		  if (mod != 0)
		    offset += (M_IMPACT_ALIGN_INT - mod);
		}
	      else if (type[i - 1].type == M_TYPE_BIT_CHAR)
		{
		  mod = offset % M_IMPACT_ALIGN_INT;
		  if (mod != 0)
		    offset += (M_IMPACT_ALIGN_INT - mod);
		}
	    }
	  if ((type[i].type == M_TYPE_BIT_LONG) ||
	      (type[i].type == M_TYPE_BIT_CHAR))
	    {
	      int mod1;
	      mod1 = offset % M_IMPACT_SIZE_INT;
	      if ((mod1 + size) > M_IMPACT_SIZE_INT)
		{
		  /* need to go to the next word */
		  offset += (M_IMPACT_SIZE_INT - mod1);
		}
	    }
	  mod = offset % align;	/* align to what the field */
	  if (mod != 0)		/* needs to start from */
	    offset += (align - mod);
	  base[i] = offset;
	  if (type[i].type == M_TYPE_BIT_LONG)
	    {
	      bit_offset[i] = offset % M_IMPACT_SIZE_LONG;
	    }
	  else if (type[i].type == M_TYPE_BIT_CHAR)
	    {
	      bit_offset[i] = offset % M_IMPACT_SIZE_CHAR;
	    }
	  else
	    {
	      bit_offset[i] = 0;
	    }
	  offset += size;	/* allocate space */
	}
    }
  else if ((M_impact_model == M_IM_HP_LCODE) ||
	   (M_impact_model == M_IM_HP_MCODE))
    {
      M_hppa_struct_layout (n, type, base, bit_offset);
    }
  else if ((M_impact_model == M_IM_SPARC_LCODE) ||
	   (M_impact_model == M_IM_SPARC_MCODE))
    {
      M_sparc_struct_layout (n, type, base, bit_offset);
    }
  else
    {
      M_assert (0, "M_impact_struct_layout: illegal machine model");
    }
}

int
M_impact_struct_align (int n, _M_Type * type)
{
  if (M_impact_model == M_IM_LCODE)
    {
      int i, max;
      max = 0;
      for (i = 0; i < n; i++)
	{
	  int aln = type[i].align;
	  if (aln > max)
	    max = aln;
	}
      /*
       *  align to at least byte boundary.
       */
      if (max < M_IMPACT_LCODE_ALIGN_CHAR)
	max = M_IMPACT_LCODE_ALIGN_CHAR;
      return max;
    }
  else if (M_impact_model == M_IM_VER_1)
    {
      int i, max;
      max = 0;
      for (i = 0; i < n; i++)
	{
	  int aln = type[i].align;
	  if (aln > max)
	    max = aln;
	}
      /*
       *  align to at least integer boundary.
       */
      if (max < M_IMPACT_ALIGN_INT)
	max = M_IMPACT_ALIGN_INT;
      return max;
    }
  else if ((M_impact_model == M_IM_HP_LCODE) ||
	   (M_impact_model == M_IM_HP_MCODE))
    {
      return (M_hppa_struct_align (n, type));
    }
  else if ((M_impact_model == M_IM_SPARC_LCODE) ||
	   (M_impact_model == M_IM_SPARC_MCODE))
    {
      return (M_sparc_struct_align (n, type));
    }
  else
    {
      M_assert (0, "M_impact_struct_align: illegal machine model");
      return (0);
    }
}

int
M_impact_struct_size (int n, _M_Type * type, int struct_align)
{
  if (M_impact_model == M_IM_LCODE)
    {
      int i, offset;
      int mod, size, align, max_align, mod_word;
      offset = 0;		/* assume initially aligned */
      max_align = struct_align;
      for (i = 0; i < n; i++)
	{
	  size = type[i].size;
	  align = type[i].align;
	  M_assert ((size != 0) && (align != 0),
		    "M_struct_layout: void is not allowed in structure");
	  if (align > max_align)
	    max_align = align;
	  /*
	   *  need to treat bit fields specially.
	   *  keep them in a word when possible.
	   */
	  mod_word = offset % M_IMPACT_LCODE_SIZE_INT;
	  if (type[i].type == M_TYPE_BIT_CHAR)
	    {
	      if ((mod_word + size) > M_IMPACT_LCODE_SIZE_INT)
		offset += (M_IMPACT_LCODE_SIZE_INT - mod_word);
	      if (M_IMPACT_LCODE_ALIGN_CHAR > max_align)
		max_align = M_IMPACT_LCODE_ALIGN_CHAR;
	    }
	  else if (type[i].type == M_TYPE_BIT_SHORT)
	    {
	      if ((mod_word + size) > M_IMPACT_LCODE_SIZE_INT)
		offset += (M_IMPACT_LCODE_SIZE_INT - mod_word);
	      if (M_IMPACT_LCODE_ALIGN_SHORT > max_align)
		max_align = M_IMPACT_LCODE_ALIGN_SHORT;
	    }
	  else if (type[i].type == M_TYPE_BIT_LONG)
	    {
	      if ((mod_word + size) > M_IMPACT_LCODE_SIZE_INT)
		offset += (M_IMPACT_LCODE_SIZE_INT - mod_word);
	      if (M_IMPACT_LCODE_ALIGN_LONG > max_align)
		max_align = M_IMPACT_LCODE_ALIGN_LONG;
	    }
	  mod = offset % align;	/* align to what the field */
	  if (mod != 0)		/* needs to start from */
	    offset += (align - mod);

	  offset += size;
	}
      /*
       * align to at least byte boundary.
       */
      if (max_align < M_IMPACT_LCODE_ALIGN_CHAR)
	max_align = M_IMPACT_LCODE_ALIGN_CHAR;
      /* enforce max. alignment */
      mod = offset % max_align;
      if (mod != 0)
	offset += (max_align - mod);
      return offset;
    }
  else if (M_impact_model == M_IM_VER_1)
    {
      int i, offset;
      int mod, size, align, max_align;
      offset = 0;		/* assume initially aligned */
      max_align = 0;
      for (i = 0; i < n; i++)
	{
	  size = type[i].size;
	  align = type[i].align;
	  M_assert ((size != 0) && (align != 0),
		    "M_struct_layout: void is not allowed in structure");
	  if (align > max_align)
	    max_align = align;
	  /*
	   *  need to treat bit fields specially.
	   *  keep them in a word when possible.
	   */
	  if ((i != 0) && (type[i].type != M_TYPE_BIT_LONG) &&
	      (type[i].type != M_TYPE_BIT_CHAR))
	    {
	      if (type[i - 1].type == M_TYPE_BIT_LONG)
		{
		  mod = offset % M_IMPACT_ALIGN_INT;
		  if (mod != 0)
		    offset += (M_IMPACT_ALIGN_INT - mod);
		}
	      else if (type[i - 1].type == M_TYPE_BIT_CHAR)
		{
		  mod = offset % M_IMPACT_ALIGN_INT;
		  if (mod != 0)
		    offset += (M_IMPACT_ALIGN_INT - mod);
		}
	    }
	  if ((type[i].type == M_TYPE_BIT_LONG)
	      || (type[i].type == M_TYPE_BIT_CHAR))
	    {
	      int mod1;
	      mod1 = offset % M_IMPACT_SIZE_INT;
	      if ((mod1 + size) > M_IMPACT_SIZE_INT)
		{
		  /* need to go to the next word */
		  offset += (M_IMPACT_SIZE_INT - mod1);
		}
	    }
	  mod = offset % align;
	  if (mod != 0)
	    offset += (align - mod);
	  offset += size;
	}
      /*
       *  align to at least integer boundary.
       */
      if (max_align < M_IMPACT_ALIGN_INT)
	max_align = M_IMPACT_ALIGN_INT;
      /* enforce max. alignment */
      mod = offset % max_align;
      if (mod != 0)
	offset += (max_align - mod);
      return offset;
    }
  else if ((M_impact_model == M_IM_HP_LCODE) ||
	   (M_impact_model == M_IM_HP_MCODE))
    {
      return (M_hppa_struct_size (n, type, struct_align));
    }
  else if ((M_impact_model == M_IM_SPARC_LCODE) ||
	   (M_impact_model == M_IM_SPARC_MCODE))
    {
      return (M_sparc_struct_size (n, type, struct_align));
    }
  else
    {
      M_assert (0, "M_impact_struct_size: illegal machine model");
      return (0);
    }
}

int
M_impact_layout_fnvar (List param_list, char **base_macro,
		       int *pcount, int purpose)
{
  M_Param param;

  if (M_impact_model == M_IM_LCODE)
    {
      int max_align, off;
      int int_rg;
      int fp_rg;
      int size, align, mod, tp;


      switch (purpose)
	{
	case M_GET_FNVAR:
	  *base_macro = "$IP";
	  break;
	case M_PUT_FNVAR:
	  *base_macro = "$OP";
	  break;
	case M_DONT_CARE_FNVAR:
	default:
	  M_assert (0, "M_fnvar_layout: unknown purpose");
	}

      max_align = M_IMPACT_LCODE_ALIGN_MAX;
      fp_rg = 0;
      int_rg = 0;
      off = 0;

      List_start (param_list);
      while ((param = (M_Param)List_next (param_list)))
	{
	  tp = param->mtype.type;

	  switch (tp)
	    {
	    case M_TYPE_CHAR:
	    case M_TYPE_SHORT:
	    case M_TYPE_INT:
	    case M_TYPE_LONG:
	    case M_TYPE_POINTER:
	      if (int_rg < M_IMPACT_LCODE_MAX_FNVAR_REG)
		{
		  param->mode = M_THRU_REGISTER;
		  param->reg = (int_rg)++ + M_IMPACT_LCODE_INT_BASE;
		}
	      else
		{
		  param->mode = M_THRU_MEMORY;
		  param->reg = -1;
		}
	      break;

	    case M_TYPE_FLOAT:
	      if (int_rg < M_IMPACT_LCODE_MAX_FNVAR_REG)
		{
		  param->mode = M_THRU_REGISTER;
		  param->reg = (fp_rg) + 1 + M_IMPACT_LCODE_FLT_BASE;
		  int_rg += 1;
		  fp_rg  += 1;
		}
	      else
		{
		  param->mode = M_THRU_MEMORY;
		  param->reg = -1;
		}
	      break;

	    case M_TYPE_DOUBLE:
	      if (int_rg < M_IMPACT_LCODE_MAX_FNVAR_REG)
		{
		  param->mode = M_THRU_REGISTER;
		  param->reg = fp_rg + 1 + M_IMPACT_LCODE_FLT_BASE;
		  int_rg += 2;
		  fp_rg  += 1;
		}
	      else
		{
		  param->mode = M_THRU_MEMORY;
		  param->reg = -1;
		}
	      break;

	    case M_TYPE_UNION:
	    case M_TYPE_STRUCT:
	      size = param->mtype.size;
	      if (int_rg < M_IMPACT_LCODE_MAX_FNVAR_REG)
		{
		  param->mode = M_INDIRECT_THRU_REGISTER;
		  param->reg = (int_rg)++ + M_IMPACT_LCODE_INT_BASE;
		}
	      else
		{
		  if (size <= M_IMPACT_LCODE_SMALL_STRUCT_MAX)
		    param->mode = M_THRU_MEMORY;
		  else
		    param->mode = M_INDIRECT_THRU_MEMORY;
		  param->reg = -1;
		}
	      break;
	    default:
	      M_assert (0, "M_fnvar_layout: argument is not promoted");
	    }
	  /* note, the method of calculating offsets may seem strange, but */
	  /* the PA convention has the stack growing towards high memory   */
	  /* and the parameters are referenced back from the $sp, with the */
	  /* first parameter being closest to the stack pointer            */
	  size = param->mtype.size;
	  align = param->mtype.align;

	  if (param->mtype.type == M_TYPE_UNION || 
	      param->mtype.type == M_TYPE_STRUCT)
	    {
	      /* make sure correct size and alignment is used for 
	       * struct/union passed indirectly thru registers
	       * if IMPACT allowed structures to be passed via registers, 
	       * this would not have to be done                           
	       */
	      if (param->mode == M_INDIRECT_THRU_REGISTER)
		{
		  size = M_IMPACT_LCODE_SIZE_INT;
		  align = M_IMPACT_LCODE_ALIGN_INT;
		}
	    }
	  if (align >= M_IMPACT_LCODE_SMALL_STRUCT_MAX &&
	      param->mtype.type != M_TYPE_DOUBLE)
	    /* anything larger than a 64-bit structure is passed */
	    /* indirectly thru memory                            */
	    align = M_IMPACT_LCODE_ALIGN_INT;
	  else if (align < M_IMPACT_LCODE_ALIGN_INT)
	    /* anything smaller that 32-bits is passed as 32-bits */
	    align = M_IMPACT_LCODE_ALIGN_INT;

	  mod = off % align;

	  /* place the offset pointer to the boundary of the appropriate */
	  /* data size                                                   */
	  if (mod != 0)
	    off += (align - mod);

#ifndef JWS_NEW_STACK_MODEL
	  /* now increment the offset to point to the actual location    */
	  /* for this parameter                                          */
	  off += size;
	  param->offset = -off;
#else
	  param->offset = off;
	  off += size;
#endif
	}


      /* The param section must be at least certain size.
       * This is the backing store for the parameter regs. */
      if (off < MIN_PARAM_SIZE)
	off = MIN_PARAM_SIZE;

      /* now for the body of the structures... small ones first. */

      List_start (param_list);
      while ((param = (M_Param)List_next (param_list)))
	{
	  tp = param->mtype.type;
	  size = param->mtype.size;

	  if (((tp == M_TYPE_UNION) ||
	       (tp == M_TYPE_STRUCT)) &&
	      (size <= M_IMPACT_LCODE_SMALL_STRUCT_MAX))
	    {

	      /* must align to a double boundry */
	      align = M_IMPACT_LCODE_ALIGN_MAX;

	      mod = off % align;

	      if (mod != 0)
		off += (align - mod);

#ifndef JWS_NEW_STACK_MODEL
	      off += size;
	      param->paddr = -off;
#else
	      param->paddr = off;
	      off += size;
#endif
	    }
	}
      /* now large ones */
      List_start (param_list);
      while ((param = (M_Param)List_next (param_list)))
	{
	  tp = param->mtype.type;

	  size = param->mtype.size;

	  if (((tp == M_TYPE_UNION) ||
	       (tp == M_TYPE_STRUCT)) &&
	      (size > M_IMPACT_LCODE_SMALL_STRUCT_MAX))
	    {

	      /* must align to a double boundry */
	      align = M_IMPACT_LCODE_ALIGN_MAX;

	      mod = off % align;

	      if (mod != 0)
		off += (align - mod);
#ifndef JWS_NEW_STACK_MODEL
	      off += size;
	      param->paddr = -off;
#else
	      param->paddr = off;
	      off += size;
#endif
	    }
	}

      *pcount = int_rg;
      return off;		/* the total size needed */
    }
  else if (M_impact_model == M_IM_VER_1)
    {
      int max_align, off, rg;
      int size, align, mod, tp;
      /*
       *  Formal parameters are specified relative to $FP or $SP.
       */
      if (purpose == M_GET_FNVAR)
	{
	  /* should be $_arg (for IMPACT, $_arg = $FP) */
	  *base_macro = "$FP";	/* incoming arguments */
	}
      else if (purpose == M_PUT_FNVAR)
	{
	  /* should be $_param (for IMPACT, $_param = $SP) */
	  *base_macro = "$SP";	/* outgoing arguments */
	}
      else
	{
	  /* the caller does not need the base_macro */
	  *base_macro = "???";
	  M_assert (0, "M_fnvar_layout: unknown purpose");
	}
      /* 
       *  the PARAM section must be max. aligned initially 
       */
      max_align = M_IMPACT_ALIGN_MAX;
      /*
       *  pass the first M_IMPACT_MAX_FNVAR_REG 
       *  scalar variables thru registers.
       */
      rg = 0;
      off = 0;
      List_start (param_list);
      while ((param = (M_Param)List_next (param_list)))
	{
	  tp = param->mtype.type;
	  switch (tp)
	    {
	    case M_TYPE_INT:
	    case M_TYPE_DOUBLE:
	      if (rg < M_IMPACT_MAX_FNVAR_REG)
		{
		  param->mode = M_THRU_REGISTER;
		  switch (tp)
		    {
		    case M_TYPE_FLOAT:
		      param->reg = M_IMPACT_ARG_F + rg;
		      break;
		    case M_TYPE_DOUBLE:
		      param->reg = M_IMPACT_ARG_F2 + rg;
		      break;
		    default:
		      param->reg = M_IMPACT_ARG_I + rg;
		      break;
		    }
		  rg++;
		}
	      else
		{
		  param->mode = M_THRU_MEMORY;
		  param->reg = -1;
		}
	      break;
	    case M_TYPE_UNION:
	    case M_TYPE_STRUCT:
	      param->mode = M_THRU_MEMORY;
	      param->reg = -1;
	      break;
	    default:
	      M_assert (0, "M_fnvar_layout: argument is not promoted");
	    }
	  size = param->mtype.size;
	  align = param->mtype.align;
	  mod = off % align;
	  if (mod != 0)
	    off += (align - mod);
	  param->offset = off;
	  off += size;
	}
      /* the param section must be at least certain size */
      if (off < MIN_PARAM_SIZE)
	off = MIN_PARAM_SIZE;
      /* the param section must be max. aligned */
      mod = off % max_align;
      if (mod != 0)
	off += (max_align - mod);
      *pcount = rg;
      return off;		/* the total size needed */
    }
  else if ((M_impact_model == M_IM_HP_LCODE) ||
	   (M_impact_model == M_IM_HP_MCODE))
    {
      return M_hppa_layout_fnvar (param_list, base_macro, pcount, purpose);
    }
  else if ((M_impact_model == M_IM_SPARC_LCODE) ||
	   (M_impact_model == M_IM_SPARC_MCODE))
    {
      return M_sparc_layout_fnvar (param_list, base_macro, pcount, purpose);
    }
  else
    {
      M_assert (0, "M_impact_fnvar_layout: illegal machine model");
      return (0);
    }
}

int
M_impact_fnvar_layout (int n, _M_Type * type, long int *offset, int *mode,
		       int *reg, int *paddr, char **base_macro,
		       int *su_sreg, int *su_ereg,
		       int *pcount, int need_ST, int purpose)
				      /* need to return structure */
{
  if (M_impact_model == M_IM_LCODE)
    {
      int i, max_align, off;
      int int_rg;
      int fp_rg;
      int size, align, mod, tp;


      switch (purpose)
	{
	case M_GET_FNVAR:
	  *base_macro = "$IP";
	  break;
	case M_PUT_FNVAR:
	  *base_macro = "$OP";
	  break;
	case M_DONT_CARE_FNVAR:
	default:
	  M_assert (0, "M_fnvar_layout: unknown purpose");
	}

      max_align = M_IMPACT_LCODE_ALIGN_MAX;
      fp_rg = 0;
      int_rg = 0;
      off = 0;

      for (i = 0; i < n; i++)
	{
	  tp = type[i].type;
	  switch (tp)
	    {
	    case M_TYPE_CHAR:
	    case M_TYPE_SHORT:
	    case M_TYPE_INT:
	    case M_TYPE_LONG:
	    case M_TYPE_POINTER:
	      if (int_rg < M_IMPACT_LCODE_MAX_FNVAR_REG)
		{
		  mode[i] = M_THRU_REGISTER;
		  reg[i] = (int_rg)++ + M_IMPACT_LCODE_INT_BASE;
		}
	      else
		{
		  mode[i] = M_THRU_MEMORY;
		  reg[i] = -1;
		}
	      break;

	    case M_TYPE_FLOAT:
	      if (int_rg < M_IMPACT_LCODE_MAX_FNVAR_REG)
		{
		  mode[i] = M_THRU_REGISTER;
		  reg[i] = (fp_rg) + 1 + M_IMPACT_LCODE_FLT_BASE;
		  int_rg += 1;
		  fp_rg  += 1;
		}
	      else
		{
		  mode[i] = M_THRU_MEMORY;
		  reg[i] = -1;
		}
	      break;

	    case M_TYPE_DOUBLE:
	      if (int_rg < M_IMPACT_LCODE_MAX_FNVAR_REG)
		{
		  mode[i] = M_THRU_REGISTER;
		  reg[i] = fp_rg + 1 + M_IMPACT_LCODE_FLT_BASE;
		  int_rg += 2;
		  fp_rg  += 1;
#if 0
		  if (rg == 0 || rg == 2)
		    {
		      mode[i] = M_THRU_REGISTER;
		      reg[i] = rg + 1 + M_IMPACT_LCODE_FLT_BASE;
		      rg += 2;
		    }
		  else if (rg == 1)
		    {
		      mode[i] = M_THRU_REGISTER;
		      reg[i] = 3 + M_IMPACT_LCODE_FLT_BASE;
		      rg += 3;
		    }
		  else
		    {
		      mode[i] = M_THRU_MEMORY;
		      reg[i] = -1;
		    }
#endif
		}
	      else
		{
		  mode[i] = M_THRU_MEMORY;
		  reg[i] = -1;
		}
	      break;

	    case M_TYPE_UNION:
	    case M_TYPE_STRUCT:
	      size = type[i].size;
	      if (int_rg < M_IMPACT_LCODE_MAX_FNVAR_REG)
		{
		  mode[i] = M_INDIRECT_THRU_REGISTER;
		  reg[i] = (int_rg)++ + M_IMPACT_LCODE_INT_BASE;
		}
	      else
		{
		  if (size <= M_IMPACT_LCODE_SMALL_STRUCT_MAX)
		    mode[i] = M_THRU_MEMORY;
		  else
		    mode[i] = M_INDIRECT_THRU_MEMORY;
		  reg[i] = -1;
		}
	      break;

#if 0
	      /* IMPACT does not allow passing of structures via registers,
	       * therefore where structures are concerned, HP's calling 
	       * convention is ignored   
	       */
	      size = type[i].size;
	      if (size <= M_IMPACT_LCODE_SIZE_INT)
		{
		  if (rg < M_IMPACT_LCODE_MAX_FNVAR_REG)
		    {
		      mode[i] = M_THRU_REGISTER;
		      reg[i] = (rg)++ + M_IMPACT_LCODE_INT_BASE;
		    }
		  else
		    {
		      mode[i] = M_THRU_MEMORY;
		      reg[i] = -1;
		    }
		}
	      else if (size <= M_IMPACT_LCODE_SMALL_STRUCT_MAX)
		{
		  if ((rg == 0) || (rg == 2))
		    {
		      mode[i] = M_THRU_REGISTER;
		      reg[i] = rg + M_IMPACT_LCODE_INT_BASE;
		      rg += 2;
		    }
		  else
		    {
		      mode[i] = M_THRU_MEMORY;
		      reg[i] = -1;
		    }
		}
	      else
		{
		  if (rg < M_IMPACT_LCODE_MAX_FNVAR_REG)
		    {
		      mode[i] = M_INDIRECT_THRU_REGISTER;
		      reg[i] = (rg)++ + M_IMPACT_LCODE_INT_BASE;
		    }
		  else
		    {
		      mode[i] = M_INDIRECT_THRU_MEMORY;
		      reg[i] = -1;
		    }
		}
	      break;
#endif
	    default:
	      M_assert (0, "M_fnvar_layout: argument is not promoted");
	    }
	  /* note, the method of calculating offsets may seem strange, but */
	  /* the PA convention has the stack growing towards high memory   */
	  /* and the parameters are referenced back from the $sp, with the */
	  /* first parameter being closest to the stack pointer            */
	  size = type[i].size;
	  align = type[i].align;

	  if (type[i].type == M_TYPE_UNION || type[i].type == M_TYPE_STRUCT)
	    {
	      /* make sure correct size and alignment is used for 
	       * struct/union passed indirectly thru registers
	       * if IMPACT allowed structures to be passed via registers, 
	       * this would not have to be done                           
	       */
	      if (mode[i] == M_INDIRECT_THRU_REGISTER)
		{
		  size = M_IMPACT_LCODE_SIZE_INT;
		  align = M_IMPACT_LCODE_ALIGN_INT;
		}
	    }
	  if (align >= M_IMPACT_LCODE_SMALL_STRUCT_MAX &&
	      type[i].type != M_TYPE_DOUBLE)
	    /* anything larger than a 64-bit structure is passed */
	    /* indirectly thru memory                            */
	    align = M_IMPACT_LCODE_ALIGN_INT;
	  else if (align < M_IMPACT_LCODE_ALIGN_INT)
	    /* anything smaller that 32-bits is passed as 32-bits */
	    align = M_IMPACT_LCODE_ALIGN_INT;

	  mod = off % align;

	  /* place the offset pointer to the boundary of the appropriate */
	  /* data size                                                   */
	  if (mod != 0)
	    off += (align - mod);

#ifndef JWS_NEW_STACK_MODEL
	  /* now increment the offset to point to the actual location    */
	  /* for this parameter                                          */
	  off += size;
	  offset[i] = -off;
#else
	  offset[i] = off;
	  off += size;
#endif
	}


      /* The param section must be at least certain size.
       * This is the backing store for the parameter regs. */
      if (off < MIN_PARAM_SIZE)
	off = MIN_PARAM_SIZE;

      /* now for the body of the structures... small ones first. */
      for (i = 0; i < n; i++)
	{
	  tp = type[i].type;

	  size = type[i].size;

	  if (((tp == M_TYPE_UNION) ||
	       (tp == M_TYPE_STRUCT)) &&
	      (size <= M_IMPACT_LCODE_SMALL_STRUCT_MAX))
	    {

	      /* must align to a double boundry */
	      align = M_IMPACT_LCODE_ALIGN_MAX;

	      mod = off % align;

	      if (mod != 0)
		off += (align - mod);

#ifndef JWS_NEW_STACK_MODEL
	      off += size;
	      paddr[i] = -off;
#else
	      paddr[i] = off;
	      off += size;
#endif
	    }
	}
      /* now large ones */
      for (i = 0; i < n; i++)
	{
	  tp = type[i].type;

	  size = type[i].size;

	  if (((tp == M_TYPE_UNION) ||
	       (tp == M_TYPE_STRUCT)) &&
	      (size > M_IMPACT_LCODE_SMALL_STRUCT_MAX))
	    {

	      /* must align to a double boundry */
	      align = M_IMPACT_LCODE_ALIGN_MAX;

	      mod = off % align;

	      if (mod != 0)
		off += (align - mod);
#ifndef JWS_NEW_STACK_MODEL
	      off += size;
	      paddr[i] = -off;
#else
	      paddr[i] = off;
	      off += size;
#endif
	    }
	}

      *pcount = int_rg;
      return off;		/* the total size needed */
    }
  else if (M_impact_model == M_IM_VER_1)
    {
      int i, max_align, off, rg;
      int size, align, mod, tp;
      /*
       *  Formal parameters are specified relative to $FP or $SP.
       */
      if (purpose == M_GET_FNVAR)
	{
	  /* should be $_arg (for IMPACT, $_arg = $FP) */
	  *base_macro = "$FP";	/* incoming arguments */
	}
      else if (purpose == M_PUT_FNVAR)
	{
	  /* should be $_param (for IMPACT, $_param = $SP) */
	  *base_macro = "$SP";	/* outgoing arguments */
	}
      else
	{
	  /* the caller does not need the base_macro */
	  *base_macro = "???";
	  M_assert (0, "M_fnvar_layout: unknown purpose");
	}
      /* 
       *  the PARAM section must be max. aligned initially 
       */
      max_align = M_IMPACT_ALIGN_MAX;
      /*
       *  pass the first M_IMPACT_MAX_FNVAR_REG 
       *  scalar variables thru registers.
       */
      rg = 0;
      off = 0;
      for (i = 0; i < n; i++)
	{
	  tp = type[i].type;
	  switch (tp)
	    {
	    case M_TYPE_INT:
	    case M_TYPE_DOUBLE:
	      if (rg < M_IMPACT_MAX_FNVAR_REG)
		{
		  mode[i] = M_THRU_REGISTER;
		  switch (tp)
		    {
		    case M_TYPE_FLOAT:
		      reg[i] = M_IMPACT_ARG_F + rg;
		      break;
		    case M_TYPE_DOUBLE:
		      reg[i] = M_IMPACT_ARG_F2 + rg;
		      break;
		    default:
		      reg[i] = M_IMPACT_ARG_I + rg;
		      break;
		    }
		  rg++;
		}
	      else
		{
		  mode[i] = M_THRU_MEMORY;
		  reg[i] = -1;
		}
	      break;
	    case M_TYPE_UNION:
	    case M_TYPE_STRUCT:
	      mode[i] = M_THRU_MEMORY;
	      reg[i] = -1;
	      break;
	    default:
	      M_assert (0, "M_fnvar_layout: argument is not promoted");
	    }
	  size = type[i].size;
	  align = type[i].align;
	  mod = off % align;
	  if (mod != 0)
	    off += (align - mod);
	  offset[i] = off;
	  off += size;
	}
      /* the param section must be at least certain size */
      if (off < MIN_PARAM_SIZE)
	off = MIN_PARAM_SIZE;
      /* the param section must be max. aligned */
      mod = off % max_align;
      if (mod != 0)
	off += (max_align - mod);
      *pcount = rg;
      return off;		/* the total size needed */
    }
  else if ((M_impact_model == M_IM_HP_LCODE) ||
	   (M_impact_model == M_IM_HP_MCODE))
    {
      return (M_hppa_fnvar_layout (n, type, offset, mode, reg, paddr,
				   base_macro, su_sreg, su_ereg, pcount,
				   need_ST, purpose));
    }
  else if ((M_impact_model == M_IM_SPARC_LCODE) ||
	   (M_impact_model == M_IM_SPARC_MCODE))
    {
      return (M_sparc_fnvar_layout (n, type, offset, mode, reg, paddr,
				    base_macro, su_sreg, su_ereg, pcount,
				    need_ST, purpose));
    }
  else
    {
      M_assert (0, "M_impact_fnvar_layout: illegal machine model");
      return (0);
    }
}

int
M_impact_lvar_layout (int n, _M_Type * type, long int *offset,
		      char **base_macro)
{
  if (M_impact_model == M_IM_LCODE)
    {
      int i, max_align, off;
      int size, align, mod, tp;
      /*
       *  the LOCAL section must be max. aligned initially
       */
      max_align = M_IMPACT_LCODE_ALIGN_MAX;
      off = 0;
      for (i = 0; i < n; i++)
	{
	  tp = type[i].type;
	  if (tp == M_TYPE_BIT_LONG)
	    {
	      M_assert (0, "M_lvar_layout: bit field not allowed");
	    }
	  if (tp == M_TYPE_BIT_CHAR)
	    {
	      M_assert (0, "M_lvar_layout: bit field not allowed");
	    }
	  size = type[i].size;
	  align = type[i].align;
	  mod = off % align;
	  if (mod != 0)
	    off += (align - mod);

	  off += size;
	  offset[i] = -off;
	}

      /* the local section must be max. aligned */
      mod = off % max_align;
      if (mod != 0)
	off += (max_align - mod);
      /*
       *  Local variables are relative to $LV
       */
      *base_macro = "$LV";
      return off;		/* the total size needed */
    }
  else if (M_impact_model == M_IM_VER_1)
    {
      int i, max_align, off;
      int size, align, mod, tp;
      /*
       *  Local variables are relative to $_local
       */
      /* 
       *  the LOCAL section must be max. aligned initially 
       */
      max_align = M_IMPACT_ALIGN_MAX;
      off = 0;
      for (i = 0; i < n; i++)
	{
	  tp = type[i].type;
	  if (tp == M_TYPE_BIT_LONG)
	    {
	      M_assert (0, "M_lvar_layout: bit field not allowed");
	    }
	  if (tp == M_TYPE_BIT_CHAR)
	    {
	      M_assert (0, "M_lvar_layout: bit field not allowed");
	    }
	  size = type[i].size;
	  align = type[i].align;
	  mod = off % align;
	  if (mod != 0)
	    off += (align - mod);
	  offset[i] = off;
	  off += size;
	}
      /* the local section must be max. aligned */
      mod = off % max_align;
      if (mod != 0)
	off += (max_align - mod);
      /*
       *      I will use negative offsets and use $FP as the
       *      base address.
       *              $_local = $FP - $local.
       */
      *base_macro = "$FP";
      for (i = 0; i < n; i++)
	offset[i] = offset[i] - off;
      return off;		/* the total size needed */
    }
  else if ((M_impact_model == M_IM_HP_LCODE) ||
	   (M_impact_model == M_IM_HP_MCODE))
    {
      return (M_hppa_lvar_layout (n, type, offset, base_macro));
    }
  else if ((M_impact_model == M_IM_SPARC_LCODE) ||
	   (M_impact_model == M_IM_SPARC_MCODE))
    {
      return (M_sparc_lvar_layout (n, type, offset, base_macro));
    }
  else
    {
      M_assert (0, "M_impact_lvar_layout: illegal machine model");
      return (0);
    }

}

int
M_impact_no_short_int (void)
{
  if (M_impact_model == M_IM_LCODE)
    {
      return (M_IMPACT_LCODE_SIZE_SHORT == M_IMPACT_LCODE_SIZE_INT);
    }
  else if (M_impact_model == M_IM_VER_1)
    {
      return (M_IMPACT_SIZE_SHORT == M_IMPACT_SIZE_INT);
    }
  else if ((M_impact_model == M_IM_HP_LCODE) ||
	   (M_impact_model == M_IM_HP_MCODE))
    {
      return (M_hppa_no_short_int ());
    }
  else if ((M_impact_model == M_IM_SPARC_LCODE) ||
	   (M_impact_model == M_IM_SPARC_MCODE))
    {
      return (M_sparc_no_short_int ());
    }
  else
    {
      M_assert (0, "M_impact_no_short_int: illegal machine model");
      return (0);
    }
}

void
M_impact_cb_label_name (char *fn, int cb, char *line, int len)
{
  if (M_impact_model == M_IM_LCODE)
    {
      sprintf (line, "cb%d%s", cb, fn);
    }
  else if (M_impact_model == M_IM_VER_1)
    {
      sprintf (line, "cb%d%s", cb, fn);
    }
  else if ((M_impact_model == M_IM_HP_LCODE) ||
	   (M_impact_model == M_IM_HP_MCODE))
    {
      M_hppa_cb_label_name (fn, cb, line, len);
    }
  else if ((M_impact_model == M_IM_SPARC_LCODE) ||
	   (M_impact_model == M_IM_SPARC_MCODE))
    {
      M_sparc_cb_label_name (fn, cb, line, len);
    }
  else
    {
      M_assert (0, "M_impact_cb_label_name: illegal machine model");
    }
}

int
M_impact_is_cb_label (char *label, char *fn, int *cb)
{
  if (M_impact_model == M_IM_LCODE)
    {
      return (sscanf (label, "cb%d%s", cb, fn) == 2);
    }
  else if (M_impact_model == M_IM_VER_1)
    {
      return (sscanf (label, "cb%d%s", cb, fn) == 2);
    }
  else if ((M_impact_model == M_IM_HP_LCODE) ||
	   (M_impact_model == M_IM_HP_MCODE))
    {
      return (M_hppa_is_cb_label (label, fn, cb));
    }
  else if ((M_impact_model == M_IM_SPARC_LCODE) ||
	   (M_impact_model == M_IM_SPARC_MCODE))
    {
      return (M_sparc_is_cb_label (label, fn, cb));
    }
  else
    {
      M_assert (0, "M_impact_is_cb_label: illegal machine model");
      return (0);
    }
}

void
M_impact_jumptbl_label_name (char *fn, int tbl_id, char *line, int len)
{
  if (M_impact_model == M_IM_LCODE)
    {
      sprintf (line, "_$%s%s%d", fn, M_JUMPTBL_BASE_NAME, tbl_id);
    }
  else if (M_impact_model == M_IM_VER_1)
    {
      sprintf (line, "%s%s%d", fn, M_JUMPTBL_BASE_NAME, tbl_id);
    }
  else if ((M_impact_model == M_IM_HP_LCODE) ||
	   (M_impact_model == M_IM_HP_MCODE))
    {
      M_hppa_jumptbl_label_name (fn, tbl_id, line, len);
    }
  else if ((M_impact_model == M_IM_SPARC_LCODE) ||
	   (M_impact_model == M_IM_SPARC_MCODE))
    {
      M_sparc_jumptbl_label_name (fn, tbl_id, line, len);
    }
  else
    {
      M_assert (0, "M_impact_jumptbl_label_name: illegal machine model");
    }
}

int
M_impact_is_jumptbl_label (char *label, char *fn, int *tbl_id)
{
  if (M_impact_model == M_IM_LCODE)
    {
      char *ptr;
      int label_len, fn_len, base_len;

      /* Check the prefix */
      if (strncmp (label, "_$", 2))
	return (0);

      /* Some length checks, to make sure we dont step outside array */
      label_len = strlen (label);
      fn_len = strlen (fn);
      base_len = strlen (M_JUMPTBL_BASE_NAME);
      if (label_len <= (2 + fn_len + base_len))
	return (0);

      /* Check that fn is correct */
      ptr = label;
      ptr += 2;
      if (strncmp (ptr, fn, fn_len))
	return (0);

      /* Check that jumptbl base name is correct */
      ptr += fn_len;
      if (strncmp (ptr, M_JUMPTBL_BASE_NAME, base_len))
	return (0);

      /* Get the id */
      ptr += base_len;
      return (sscanf (ptr, "%d", tbl_id) == 1);
    }
  else if (M_impact_model == M_IM_VER_1)
    {
      /* Format for v1.0: %sM_JUMPTBL_BASE_NAME%d, where %s is the func name */
      char *ptr;
      int label_len, fn_len, base_len;

      /* Some length checks, to make sure we dont step outside array */
      label_len = strlen (label);
      fn_len = strlen (fn);
      base_len = strlen (M_JUMPTBL_BASE_NAME);
      if (label_len <= (fn_len + base_len))
	return (0);

      /* Check that fn is correct */
      ptr = label;
      if (strncmp (ptr, fn, fn_len))
	return (0);

      /* Check that jumptbl base name is correct */
      ptr += fn_len;
      if (strncmp (ptr, M_JUMPTBL_BASE_NAME, base_len))
	return (0);

      /* Get the id */
      ptr += base_len;
      return (sscanf (ptr, "%d", tbl_id) == 1);
    }
  else if ((M_impact_model == M_IM_HP_LCODE) ||
	   (M_impact_model == M_IM_HP_MCODE))
    {
      return (M_hppa_is_jumptbl_label (label, fn, tbl_id));
    }
  else if ((M_impact_model == M_IM_SPARC_LCODE) ||
	   (M_impact_model == M_IM_SPARC_MCODE))
    {
      return (M_sparc_is_jumptbl_label (label, fn, tbl_id));
    }
  else
    {
      M_assert (0, "M_impact_is_jumptbl_label: illegal machine model");
      return (0);
    }
}

int
M_impact_structure_pointer (int purpose)
{
  if (M_impact_model == M_IM_LCODE)
    {
      return M_IMPACT_LCODE_RET_ST;
    }
  else if (M_impact_model == M_IM_VER_1)
    {
      return M_IMPACT_ST;
    }
  else if ((M_impact_model == M_IM_HP_LCODE) ||
	   (M_impact_model == M_IM_HP_MCODE))
    {
      return (M_hppa_structure_pointer (purpose));
    }
  else if ((M_impact_model == M_IM_SPARC_LCODE) ||
	   (M_impact_model == M_IM_SPARC_MCODE))
    {
      return (M_sparc_structure_pointer (purpose));
    }
  else
    {
      M_assert (0, "M_impact_structure_pointer: illegal machine model");
      return (0);
    }
}

int
M_impact_return_register (int type, int purpose)
{
  if (M_impact_model == M_IM_LCODE)
    {
      switch (type)
	{
	case M_TYPE_INT:
	  return M_IMPACT_LCODE_RET_I32;
	case M_TYPE_LONG:
	  return M_IMPACT_LCODE_RET_I32;
	case M_TYPE_FLOAT:
	  return M_IMPACT_LCODE_RET_F;
	case M_TYPE_DOUBLE:
	  return M_IMPACT_LCODE_RET_F;
	default:
	  return M_IMPACT_LCODE_RET_I32;
	}
    }
  else if (M_impact_model == M_IM_VER_1)
    {
      switch (type)
	{
	case M_TYPE_FLOAT:
	  return M_IMPACT_R0_F;
	case M_TYPE_DOUBLE:
	  return M_IMPACT_R0_F2;
	default:
	  return M_IMPACT_R0_I;
	}
    }
  else if ((M_impact_model == M_IM_HP_LCODE) ||
	   (M_impact_model == M_IM_HP_MCODE))
    {
      return (M_hppa_return_register (type, purpose));
    }
  else if ((M_impact_model == M_IM_SPARC_LCODE) ||
	   (M_impact_model == M_IM_SPARC_MCODE))
    {
      return (M_sparc_return_register (type, purpose));
    }
  else
    {
      M_assert (0, "M_impact_return_register: illegal machine model");
      return (0);
    }
}

/* Added to support Lcode model -ITI (JCG) 2/99 */
char *
M_impact_fn_label_name (char *label, int (*is_func) (char *is_func_label))
{
  static char fn_label[1024];

  if (M_impact_model == M_IM_LCODE)
    {
      if ((*is_func) (label))
	{
	  sprintf (fn_label, "$fn_%s", label);
	  return (fn_label);
	}
      else
	return (label);
    }
  else
    {
      return (label);
    }

}

char *
M_impact_fn_name_from_label (char *label)
{
  if (M_impact_model == M_IM_LCODE)
    {
      if (!strncmp (label, "_$fn", 4))
	return (label + 4);
      else
	return (label);
    }
  else
    {
      return (label);
    }
}

int
M_impact_fragile_macro (int macro_value)
{
  if (M_impact_model == M_IM_LCODE)
    {
      switch (macro_value)
	{
	case L_MAC_P12:
	case L_MAC_SP:
	case L_MAC_FP:
	case L_MAC_LV:		/* added SAM 10-94 */
	case L_MAC_IP:		/* added SAM 10-94 */
	case L_MAC_OP:		/* added SAM 10-94 */
	case L_MAC_LOCAL_SIZE:
	case L_MAC_PARAM_SIZE:
	case L_MAC_SWAP_SIZE:
	  return 0;
	default:
	  return (1);
	}
    }
  else if (M_impact_model == M_IM_VER_1)
    {
      switch (macro_value)
	{
	case L_MAC_SP:
	case L_MAC_FP:
	case L_MAC_LOCAL_SIZE:
	case L_MAC_PARAM_SIZE:
	case L_MAC_SWAP_SIZE:
	  return 0;
	default:
	  return 1;
	}
    }
  else if ((M_impact_model == M_IM_HP_LCODE) ||
	   (M_impact_model == M_IM_HP_MCODE))
    {
      return (M_hppa_fragile_macro (macro_value));
    }
  else if ((M_impact_model == M_IM_SPARC_LCODE) ||
	   (M_impact_model == M_IM_SPARC_MCODE))
    {
      return (M_sparc_fragile_macro (macro_value));
    }
  else
    {
      M_assert (0, "M_impact_return_register: illegal machine model");
      return (0);
    }
}

Set M_impact_fragile_macro_set ()
{
  if (M_impact_model == M_IM_LCODE)
    {
      if (Set_impact_fragile_macro)
	{
	  return Set_impact_fragile_macro;
	}
      Set_impact_fragile_macro = Set_add (Set_impact_fragile_macro, L_MAC_P0);
      Set_impact_fragile_macro = Set_add (Set_impact_fragile_macro, L_MAC_P1);
      Set_impact_fragile_macro = Set_add (Set_impact_fragile_macro, L_MAC_P2);
      Set_impact_fragile_macro = Set_add (Set_impact_fragile_macro, L_MAC_P3);
      Set_impact_fragile_macro = Set_add (Set_impact_fragile_macro, L_MAC_P4);
      Set_impact_fragile_macro = Set_add (Set_impact_fragile_macro, L_MAC_P5);
      Set_impact_fragile_macro = Set_add (Set_impact_fragile_macro, L_MAC_P6);
      Set_impact_fragile_macro = Set_add (Set_impact_fragile_macro, L_MAC_P7);
      Set_impact_fragile_macro = Set_add (Set_impact_fragile_macro, L_MAC_P8);
      Set_impact_fragile_macro = Set_add (Set_impact_fragile_macro, L_MAC_P9);
      Set_impact_fragile_macro =
	Set_add (Set_impact_fragile_macro, L_MAC_P10);
      Set_impact_fragile_macro =
	Set_add (Set_impact_fragile_macro, L_MAC_P11);
      Set_impact_fragile_macro =
	Set_add (Set_impact_fragile_macro, L_MAC_P13);
      Set_impact_fragile_macro =
	Set_add (Set_impact_fragile_macro, L_MAC_P14);
      Set_impact_fragile_macro =
	Set_add (Set_impact_fragile_macro, L_MAC_P15);
      Set_impact_fragile_macro =
	Set_add (Set_impact_fragile_macro, L_MAC_P16);
      Set_impact_fragile_macro =
	Set_add (Set_impact_fragile_macro, L_MAC_P17);
      Set_impact_fragile_macro =
	Set_add (Set_impact_fragile_macro, L_MAC_P18);
      Set_impact_fragile_macro =
	Set_add (Set_impact_fragile_macro, L_MAC_P19);
      Set_impact_fragile_macro =
	Set_add (Set_impact_fragile_macro, L_MAC_P20);
      Set_impact_fragile_macro =
	Set_add (Set_impact_fragile_macro, L_MAC_P21);
      Set_impact_fragile_macro =
	Set_add (Set_impact_fragile_macro, L_MAC_P22);
      Set_impact_fragile_macro =
	Set_add (Set_impact_fragile_macro, L_MAC_P23);
      Set_impact_fragile_macro =
	Set_add (Set_impact_fragile_macro, L_MAC_P24);
      Set_impact_fragile_macro =
	Set_add (Set_impact_fragile_macro, L_MAC_P25);
      Set_impact_fragile_macro =
	Set_add (Set_impact_fragile_macro, L_MAC_P26);
      Set_impact_fragile_macro =
	Set_add (Set_impact_fragile_macro, L_MAC_P27);
      Set_impact_fragile_macro =
	Set_add (Set_impact_fragile_macro, L_MAC_P28);
      Set_impact_fragile_macro =
	Set_add (Set_impact_fragile_macro, L_MAC_P29);
      Set_impact_fragile_macro =
	Set_add (Set_impact_fragile_macro, L_MAC_P30);
      Set_impact_fragile_macro =
	Set_add (Set_impact_fragile_macro, L_MAC_P31);
      Set_impact_fragile_macro =
	Set_add (Set_impact_fragile_macro, L_MAC_P32);
      Set_impact_fragile_macro =
	Set_add (Set_impact_fragile_macro, L_MAC_P33);
      Set_impact_fragile_macro =
	Set_add (Set_impact_fragile_macro, L_MAC_P34);
      Set_impact_fragile_macro =
	Set_add (Set_impact_fragile_macro, L_MAC_P35);
      Set_impact_fragile_macro =
	Set_add (Set_impact_fragile_macro, L_MAC_P36);
      Set_impact_fragile_macro =
	Set_add (Set_impact_fragile_macro, L_MAC_P37);
      Set_impact_fragile_macro =
	Set_add (Set_impact_fragile_macro, L_MAC_P38);
      Set_impact_fragile_macro =
	Set_add (Set_impact_fragile_macro, L_MAC_P39);
      Set_impact_fragile_macro =
	Set_add (Set_impact_fragile_macro, L_MAC_P40);
      Set_impact_fragile_macro =
	Set_add (Set_impact_fragile_macro, L_MAC_P41);
      Set_impact_fragile_macro =
	Set_add (Set_impact_fragile_macro, L_MAC_P42);
      Set_impact_fragile_macro =
	Set_add (Set_impact_fragile_macro, L_MAC_P43);
      Set_impact_fragile_macro =
	Set_add (Set_impact_fragile_macro, L_MAC_P44);
      Set_impact_fragile_macro =
	Set_add (Set_impact_fragile_macro, L_MAC_P45);
      Set_impact_fragile_macro =
	Set_add (Set_impact_fragile_macro, L_MAC_P46);
      Set_impact_fragile_macro =
	Set_add (Set_impact_fragile_macro, L_MAC_P47);
      Set_impact_fragile_macro =
	Set_add (Set_impact_fragile_macro, L_MAC_P48);
      Set_impact_fragile_macro =
	Set_add (Set_impact_fragile_macro, L_MAC_P49);
      Set_impact_fragile_macro =
	Set_add (Set_impact_fragile_macro, L_MAC_P50);
      Set_impact_fragile_macro =
	Set_add (Set_impact_fragile_macro, L_MAC_P51);
      Set_impact_fragile_macro =
	Set_add (Set_impact_fragile_macro, L_MAC_P52);
      Set_impact_fragile_macro =
	Set_add (Set_impact_fragile_macro, L_MAC_P53);
      Set_impact_fragile_macro =
	Set_add (Set_impact_fragile_macro, L_MAC_P54);
      Set_impact_fragile_macro =
	Set_add (Set_impact_fragile_macro, L_MAC_P55);
      Set_impact_fragile_macro =
	Set_add (Set_impact_fragile_macro, L_MAC_P56);
      Set_impact_fragile_macro =
	Set_add (Set_impact_fragile_macro, L_MAC_P57);
      Set_impact_fragile_macro =
	Set_add (Set_impact_fragile_macro, L_MAC_P58);
      Set_impact_fragile_macro =
	Set_add (Set_impact_fragile_macro, L_MAC_P59);
      Set_impact_fragile_macro =
	Set_add (Set_impact_fragile_macro, L_MAC_P60);
      Set_impact_fragile_macro =
	Set_add (Set_impact_fragile_macro, L_MAC_P61);
      Set_impact_fragile_macro =
	Set_add (Set_impact_fragile_macro, L_MAC_P62);
      Set_impact_fragile_macro =
	Set_add (Set_impact_fragile_macro, L_MAC_P63);
      Set_impact_fragile_macro =
	Set_add (Set_impact_fragile_macro, L_MAC_P64);
      Set_impact_fragile_macro = Set_add (Set_impact_fragile_macro, L_MAC_RS);
      Set_impact_fragile_macro = Set_add (Set_impact_fragile_macro,
					  L_MAC_RET_TYPE);
      Set_impact_fragile_macro = Set_add (Set_impact_fragile_macro,
					  L_MAC_TR_PTR);
      Set_impact_fragile_macro = Set_add (Set_impact_fragile_macro,
					  L_MAC_TR_MARK);
      Set_impact_fragile_macro = Set_add (Set_impact_fragile_macro,
					  L_MAC_TR_TEMP);
      Set_impact_fragile_macro = Set_add (Set_impact_fragile_macro,
					  L_MAC_PRED_ALL);
      Set_impact_fragile_macro = Set_add (Set_impact_fragile_macro,
					  L_MAC_SAFE_MEM);
      Set_impact_fragile_macro = Set_add (Set_impact_fragile_macro,
					  L_MAC_TM_TYPE);
      return Set_impact_fragile_macro;
    }
  else if (M_impact_model == M_IM_VER_1)
    {
      if (Set_impact_fragile_macro)
	{
	  return Set_impact_fragile_macro;
	}
      Set_impact_fragile_macro = Set_add (Set_impact_fragile_macro, L_MAC_P0);
      Set_impact_fragile_macro = Set_add (Set_impact_fragile_macro, L_MAC_P1);
      Set_impact_fragile_macro = Set_add (Set_impact_fragile_macro, L_MAC_P2);
      Set_impact_fragile_macro = Set_add (Set_impact_fragile_macro, L_MAC_P3);
      Set_impact_fragile_macro = Set_add (Set_impact_fragile_macro, L_MAC_P4);
      Set_impact_fragile_macro = Set_add (Set_impact_fragile_macro, L_MAC_P5);
      Set_impact_fragile_macro = Set_add (Set_impact_fragile_macro, L_MAC_P6);
      Set_impact_fragile_macro = Set_add (Set_impact_fragile_macro, L_MAC_P7);
      Set_impact_fragile_macro = Set_add (Set_impact_fragile_macro, L_MAC_P8);
      Set_impact_fragile_macro = Set_add (Set_impact_fragile_macro, L_MAC_P9);
      Set_impact_fragile_macro =
	Set_add (Set_impact_fragile_macro, L_MAC_P10);
      Set_impact_fragile_macro =
	Set_add (Set_impact_fragile_macro, L_MAC_P11);
      Set_impact_fragile_macro =
	Set_add (Set_impact_fragile_macro, L_MAC_P12);
      Set_impact_fragile_macro =
	Set_add (Set_impact_fragile_macro, L_MAC_P13);
      Set_impact_fragile_macro =
	Set_add (Set_impact_fragile_macro, L_MAC_P14);
      Set_impact_fragile_macro =
	Set_add (Set_impact_fragile_macro, L_MAC_P15);

      Set_impact_fragile_macro =
	Set_add (Set_impact_fragile_macro, L_MAC_TR_PTR);
      Set_impact_fragile_macro =
	Set_add (Set_impact_fragile_macro, L_MAC_TR_MARK);
      Set_impact_fragile_macro =
	Set_add (Set_impact_fragile_macro, L_MAC_TR_TEMP);

      Set_impact_fragile_macro =
	Set_add (Set_impact_fragile_macro, L_MAC_SAFE_MEM);

      return Set_impact_fragile_macro;
    }
  else if ((M_impact_model == M_IM_HP_LCODE) ||
	   (M_impact_model == M_IM_HP_MCODE))
    {
      return (M_hppa_fragile_macro_set ());
    }
  else if ((M_impact_model == M_IM_SPARC_LCODE) ||
	   (M_impact_model == M_IM_SPARC_MCODE))
    {
      return (M_sparc_fragile_macro_set ());
    }
  else
    {
      M_assert (0, "M_impact_fragile_macro_set: illegal machine model");
      return (0);
    }
}

int
M_impact_dataflow_macro (int id)
{
  if (M_impact_model == M_IM_LCODE)
    {
      return ((id >= L_MAC_P0 && id <= L_MAC_P64) || (id == L_MAC_SP) ||
	      (id == L_MAC_RETADDR) || (id >= L_MAC_LAST));
    }
  else if (M_impact_model == M_IM_VER_1)
    {
      return ((id >= L_MAC_P0 && id <= L_MAC_P64) || (id >= L_MAC_LAST));
    }
  else if ((M_impact_model == M_IM_HP_LCODE) ||
	   (M_impact_model == M_IM_HP_MCODE))
    {
      return (M_hppa_dataflow_macro (id));
    }
  else if ((M_impact_model == M_IM_SPARC_LCODE) ||
	   (M_impact_model == M_IM_SPARC_MCODE))
    {
      return (M_sparc_dataflow_macro (id));
    }
  else
    {
      M_assert (0, "M_impact_dataflow_macro: illegal machine model");
      return (0);
    }
}


/*
 *	This is an instruction set query, so M_IM_HP does not call HP
 *	Mspec function!!!
 *      Really??  Talk with me before changing it back - DIA
 *      HP-LCODE case modified to return true only for JSR, 
 *                                                     JSR_FS. - JWS 3/29/98
 */
int
M_impact_subroutine_call (int opc)
{
  if ((M_impact_model == M_IM_LCODE))
    {
      /* Intentionally different than HP's. Assume have div unit, etc.
       * -ITI (JCT) 2/99 
       */
      return ((opc == Lop_JSR) || (opc == Lop_JSR_FS));
    }
  else if ((M_impact_model == M_IM_VER_1))
    {
      return ((opc == Lop_JSR) || (opc == Lop_JSR_FS));
    }
  else if (M_impact_model == M_IM_HP_MCODE)
    {
      return (M_hppa_subroutine_call (opc));
    }
  else if (M_impact_model == M_IM_HP_LCODE)
    {
      return ((opc == Lop_JSR) || (opc == Lop_JSR_FS));
    }
  else if ((M_impact_model == M_IM_SPARC_MCODE) ||
	   (M_impact_model == M_IM_SPARC_LCODE))
    {
      return (M_sparc_subroutine_call (opc));
    }
  else
    {
      M_assert (0, "M_impact_subroutine_call: illegal machine model");
      return (0);
    }
}

/*
 *	Impact model assumed to fully support Lcode opcodes
 *	This is an instruction set query, so M_IM_HP does not call HP
 *      Mspec function!!!
 */
int
M_oper_supported_in_arch_impact (int opc)
{
  if (M_impact_model == M_IM_LCODE)
    {
      /* Different from HP, assume full operation support -ITI (JCT) 2/99 */
      return 1;
    }
  else if ((M_impact_model == M_IM_VER_1) ||
	   (M_impact_model == M_IM_HP_LCODE) ||
	   (M_impact_model == M_IM_SPARC_LCODE))
    {
      return 1;
    }
  else if ((M_impact_model == M_IM_HP_MCODE))
    {
      return (M_oper_supported_in_arch_hppa (opc));
    }
  else if ((M_impact_model == M_IM_SPARC_MCODE))
    {
      return (M_oper_supported_in_arch_sparc (opc));
    }
  else
    {
      M_assert (0, "M_oper_supported_in_arch_impact: illegal machine model");
      return (0);
    }
}

/*
 *      Impact model assumed to fully support Lcode opcodes
 *      This is an instruction set query, so M_IM_HP does not call HP
 *      Mspec function!!!
 */
int
M_num_oper_required_for_impact (L_Oper * oper, char *name)
{
  if (M_impact_model == M_IM_LCODE)
    {
      /* Use HPPA's biases to move labels into register's verses 
       * encouraging labels to be copy-propagated everywhere (which
       * happens if always return 1. 
       * May need to do further tweaking later. -ITI (JCG) 2/99
       */
#define has_label_operand(a)    ((a->src[0]->type == L_OPERAND_LABEL)||\
                                 (a->src[1]->type == L_OPERAND_LABEL))

#define indexed_memory_op(a)    ((a->src[0]->type == L_OPERAND_REGISTER)&&\
                                 (a->src[1]->type == L_OPERAND_REGISTER))

#define non_zero_offset(a)      ((!((a->src[1]->type==L_OPERAND_IMMED)&&\
                                   ((a->src[1]->ctype&0x30)==0x00)))||\
                                 (a->src[1]->value.i != 0))

#define short_int_inc(a,b)      ((a->src[b]->type==L_OPERAND_IMMED)&&\
                                 ((a->src[b]->ctype&0x30)==0x00)&&\
                                 (a->src[b]->value.i >= -0x10) &&\
                                 (a->src[b]->value.i < 0x10))
#define long_pos_int_inc(a,b)   ((a->src[b]->type==L_OPERAND_IMMED)&&\
                                 ((a->src[b]->ctype&0x30)==0x00)&&\
                                 (a->src[b]->value.i > 0) && \
                                 (a->src[b]->value.i < 0x2000))
#define long_neg_int_inc(a,b)   ((a->src[b]->type==L_OPERAND_IMMED)&&\
                                 ((a->src[b]->ctype&0x30)==0x00)&&\
                                 (a->src[b]->value.i >= -0x2000) && \
                                 (a->src[b]->value.i < 0))
#define register_inc(a,b)        (a->src[b]->type == L_OPERAND_REGISTER)
      switch (oper->opc)
	{
	case Lop_ST_C:
	case Lop_ST_C2:
	case Lop_ST_I:
	  if (indexed_memory_op (oper) || has_label_operand (oper))
	    return (2);
	  else
	    return (1);

	case Lop_LD_UC:
	case Lop_LD_C:
	case Lop_LD_UC2:
	case Lop_LD_C2:
	case Lop_LD_I:
	  if (indexed_memory_op (oper) || has_label_operand (oper))
	    return (2);
	  else
	    return (1);

	case Lop_ST_F:
	case Lop_ST_F2:
	  if (indexed_memory_op (oper) || has_label_operand (oper))
	    return (2);
	  else
	    return (1);

	case Lop_LD_F:
	case Lop_LD_F2:
	  if (indexed_memory_op (oper) || has_label_operand (oper))
	    return (2);
	  else
	    return (1);

	case Lop_LD_PRE_UC:
	case Lop_LD_PRE_C:
	case Lop_LD_PRE_UC2:
	case Lop_LD_PRE_C2:
	case Lop_LD_PRE_I:
	  if (indexed_memory_op (oper) || non_zero_offset (oper))
	    return (2);
	  else if (short_int_inc (oper, 2) || long_neg_int_inc (oper, 2))
	    return (1);
	  else
	    return (2);

	case Lop_ST_PRE_C:
	case Lop_ST_PRE_C2:
	case Lop_ST_PRE_I:
	  if (indexed_memory_op (oper) || non_zero_offset (oper))
	    return (2);
	  else if (short_int_inc (oper, 3) || long_neg_int_inc (oper, 3))
	    return (1);
	  else
	    return (2);

	case Lop_LD_POST_UC:
	case Lop_LD_POST_C:
	case Lop_LD_POST_UC2:
	case Lop_LD_POST_C2:
	case Lop_LD_POST_I:
	  if (non_zero_offset (oper))
	    return (2);
	  else if (short_int_inc (oper, 2) ||
		   long_pos_int_inc (oper, 2) || register_inc (oper, 2))
	    return (1);
	  else
	    return (2);

	case Lop_ST_POST_C:
	case Lop_ST_POST_C2:
	case Lop_ST_POST_I:
	  if (indexed_memory_op (oper) || non_zero_offset (oper))
	    return (2);
	  else if (short_int_inc (oper, 3) || long_pos_int_inc (oper, 3))
	    return (1);
	  else
	    return (2);

	case Lop_LD_PRE_F:
	case Lop_LD_PRE_F2:
	  if (indexed_memory_op (oper) || non_zero_offset (oper))
	    return (2);
	  else if (short_int_inc (oper, 2))
	    return (1);
	  else
	    return (2);

	case Lop_ST_PRE_F:
	case Lop_ST_PRE_F2:
	  if (indexed_memory_op (oper) || non_zero_offset (oper))
	    return (2);
	  else if (short_int_inc (oper, 3))
	    return (1);
	  else
	    return (2);

	case Lop_LD_POST_F:
	case Lop_LD_POST_F2:
	  if (non_zero_offset (oper))
	    return (2);
	  else if (short_int_inc (oper, 2) || register_inc (oper, 2))
	    return (1);
	  else
	    return (2);

	case Lop_ST_POST_F:
	case Lop_ST_POST_F2:
	  if (non_zero_offset (oper))
	    return (2);
	  else if (short_int_inc (oper, 3) || register_inc (oper, 3))
	    return (1);
	  else
	    return (2);

	case Lop_PREF_LD:
	  if (has_label_operand (oper))
	    return (2);
	  else
	    return (1);

	default:
	  return (1);
	}
    }
  else if ((M_impact_model == M_IM_VER_1) ||
	   (M_impact_model == M_IM_HP_LCODE) ||
	   (M_impact_model == M_IM_SPARC_LCODE))
    {
      return 1;
    }
  else if ((M_impact_model == M_IM_HP_MCODE))
    {
      return (M_num_oper_required_for_hppa (oper, name));
    }
  else if ((M_impact_model == M_IM_SPARC_MCODE))
    {
      return (M_num_oper_required_for_sparc (oper, name));
    }
  else
    {
      M_assert (0, "M_num_oper_required_for_impact: illegal machine model");
      return (0);
    }
}

int
M_is_stack_operand_impact (L_Operand * operand)
{
  if (M_impact_model == M_IM_LCODE)
    {
      if (L_is_macro (operand) &&
	  (operand->value.mac == L_MAC_SP ||
	   operand->value.mac == L_MAC_FP ||
	   operand->value.mac == L_MAC_SAFE_MEM ||
	   operand->value.mac == L_MAC_P12 ||
	   operand->value.mac == L_MAC_IP ||
	   operand->value.mac == L_MAC_OP ||
	   operand->value.mac == L_MAC_RS || operand->value.mac == L_MAC_LV))
	{
	  return (1);
	}
      else
	{
	  return (0);
	}
    }
  else if (M_impact_model == M_IM_VER_1)
    {
      if (L_is_macro (operand) &&
	  (operand->value.mac == L_MAC_SP || operand->value.mac == L_MAC_FP))
	return (1);
      return (0);
    }
  else if ((M_impact_model == M_IM_HP_LCODE) ||
	   (M_impact_model == M_IM_HP_MCODE))
    {
      return (M_is_stack_operand_hppa (operand));
    }
  else if ((M_impact_model == M_IM_SPARC_LCODE) ||
	   (M_impact_model == M_IM_SPARC_MCODE))
    {
      return (M_is_stack_operand_sparc (operand));
    }
  else
    {
      M_assert (0, "M_is_stack_operand_impact: illegal machine model");
      return (0);
    }
}

int
M_is_unsafe_macro_impact (L_Operand * operand)
{
  if (M_impact_model == M_IM_LCODE)
    {
      return 0;
    }
  else if (M_impact_model == M_IM_VER_1)
    {
      return 0;
    }
  else if ((M_impact_model == M_IM_HP_LCODE) ||
	   (M_impact_model == M_IM_HP_MCODE))
    {
      return (M_is_unsafe_macro_hppa (operand));
    }
  else if ((M_impact_model == M_IM_SPARC_LCODE) ||
	   (M_impact_model == M_IM_SPARC_MCODE))
    {
      return (M_is_unsafe_macro_sparc (operand));
    }
  else
    {
      M_assert (0, "M_is_unsafe_macro_impact: illegal machine model");
      return (0);
    }
}

static int
M_operand_type_impact_lcode (L_Operand * operand)
{
  /* If NULL operand pointer, then return MDES_OPERAND_NULL */
  if (operand == NULL)
    return (MDES_OPERAND_NULL);

  switch (L_operand_case_type (operand))
    {
    case L_OPERAND_INT:
    case L_OPERAND_FLOAT:
    case L_OPERAND_DOUBLE:
      return (MDES_OPERAND_Lit);

    case L_OPERAND_MACRO:
    case L_OPERAND_REGISTER:
      if (L_is_ctype_predicate (operand))
	return (MDES_OPERAND_p);
      else
	return (MDES_OPERAND_REG);

    case L_OPERAND_CB:
    case L_OPERAND_LABEL:
    case L_OPERAND_STRING:
      return (MDES_OPERAND_Label);

    default:
      M_assert (0, "M_operand_type_impact_lcode: Unknown type");
      return (0);
    }
}

static int
M_operand_type_impact_hp_lcode (L_Operand * operand)
{
  /* If NULL operand pointer, then return MDES_OPERAND_NULL */
  if (operand == NULL)
    return (MDES_OPERAND_NULL);

  switch (L_operand_case_type (operand))
    {
    case L_OPERAND_INT:
      if (FIELD_5 (operand->value.i))
	return (MDES_OPERAND_Lit5);
      else if (FIELD_11 (operand->value.i))
	return (MDES_OPERAND_Lit11);
      else if (FIELD_14 (operand->value.i))
	return (MDES_OPERAND_Lit14);
      else
	return (MDES_OPERAND_Lit21);

    case L_OPERAND_MACRO:
    case L_OPERAND_REGISTER:
      if (L_is_ctype_predicate (operand))
	return (MDES_OPERAND_p);
      else
	return (MDES_OPERAND_REG);

    case L_OPERAND_CB:
    case L_OPERAND_LABEL:
    case L_OPERAND_FLOAT:
    case L_OPERAND_DOUBLE:
    case L_OPERAND_STRING:
      return (MDES_OPERAND_Label);

    default:
      M_assert (0, "M_operand_type_impact_hp_lcode: Unknown type");
      return (0);
    }
}

static int
M_operand_type_impact_sparc_lcode (L_Operand * operand)
{
  /* If NULL operand pointer, then return MDES_OPERAND_NULL */
  if (operand == NULL)
    return (MDES_OPERAND_NULL);

  switch (L_operand_case_type (operand))
    {
    case L_OPERAND_INT:
      return (MDES_OPERAND_Lit);

    case L_OPERAND_MACRO:
      if (operand->value.mac >= L_SPARC_MAC_ICC &&
	  operand->value.mac <= L_SPARC_MAC_ICC15)
	return (MDES_OPERAND_sparc_icc);
      else if (operand->value.mac >= L_SPARC_MAC_FCC &&
	       operand->value.mac <= L_SPARC_MAC_FCC15)
	return (MDES_OPERAND_sparc_fcc);
      else
	return (MDES_OPERAND_REG);

    case L_OPERAND_REGISTER:
      return (MDES_OPERAND_REG);

    case L_OPERAND_CB:
    case L_OPERAND_LABEL:
    case L_OPERAND_FLOAT:
    case L_OPERAND_DOUBLE:
    case L_OPERAND_STRING:
      return (MDES_OPERAND_Label);

    default:
      M_assert (0, "M_operand_type_impact_sparc_lcode: Unknown type");
      return (0);
    }

}

int
M_operand_type_impact (L_Operand * operand)
{
  if (M_impact_model == M_IM_LCODE)
    {
      return M_operand_type_impact_lcode (operand);
    }
  else if ((M_impact_model == M_IM_VER_1) ||
	   (M_impact_model == M_IM_HP_LCODE) ||
	   (M_impact_model == M_IM_SPARC_LCODE))
    {
      return M_operand_type_impact_lcode (operand);
    }
  else if ((M_impact_model == M_IM_HP_MCODE))
    {
      return M_operand_type_impact_hp_lcode (operand);
    }
  else if ((M_impact_model == M_IM_SPARC_MCODE))
    {
      return M_operand_type_impact_sparc_lcode (operand);
    }
  else
    {
      M_assert (0, "M_operand_type_impact: illegal machine model");
      return (0);
    }
}


/* Mechanism for Limpact to specify the offset between the left float
 * register numer and the double register number.  Typically, this
 * offset should be set to num_flt_callee_reg in Limpact. -JCG 6/99
 */
int M_Limpact_dbl_offset = 0;

/* Conflicting operand functions for impact ver1 and hppa models */
/*
 *      Structure of assumed register file for IMPACT machine, so this
 *	is what is used after register allocation for the IMPACT machine
 *
 *      int registers
 *              1
 *              ...
 *              Limpact_num_int_reg (required to be an even #)
 *
 *      flt/dbl registers (double == consecective float regs)
 *      (double reg number is left float + M_Limpact_dbl_offset -JCG 6/99)
 *              Limpact_num_int_reg+1   Limpact_num_int_reg+2
 *              ...                     ...
 *              Limpact_num_flt_reg+M_Limpact_dbl_offset-1   
 *              Limpact_num_flt_reg+M_Limpact_dbl_offset
 *      Note: Doubles use only odd register numbers after allocation! -JCG 6/99
 *
 *      prd registrs
 *              Limpact_num_flt_reg
 *              ...
 *              Limpact_num_prd_reg
 *
 *      Return registers:
 *              P0 (int), P2 (dbl) (no flt and prd parameter passing regs)
 *              --> actually P1 is flt, but should never see!!!
 *
 *      Parameter passing
 *              P4-P7 (int), P12-P15(dbl) 
 *              (no flt and prd parameter passing regs)
 *              --> actually P8-P11 is flt, but should never see!!!
 *
 *      Stack ptr, Frame ptr
 *              SP, FP
 *
 *      Note for now, handle all macros so can run Limpact on all Lcodes
 *      for any target machine.
 */

 
static int
M_conflicting_operands_impact_lcode (L_Operand * operand,
				     L_Operand ** conflict_array, int len,
				     int prepass)
{
  int i = 0;

  /* Use this case if no register allocation has been done on the code!! */
  if (prepass)
    {
      if (L_is_reg (operand))
	{
	  conflict_array[i++] = L_copy_operand (operand);
	}
      else if (L_is_macro (operand))
	{
	  conflict_array[i++] = L_copy_operand (operand);
	}
      else
	{
	  M_assert (0,
		    "Limpact_conflicting_operands: illegal operand type "
		    "(no regalloc)");
	}
    }

  /* else register allocation has been done */
  else if (L_is_reg (operand))
    {
      /* int register */
      if (L_is_ctype_integer (operand))
	{
	  conflict_array[i++] = L_copy_operand (operand);
	}
      /* Made match Limpact's register bank definitions -JCG 6/99 */
      else if (L_is_ctype_dbl (operand))
	{
	  /* Sanity check, double register operand number must be odd
	   * after register allocation!
	   */
	  if ((operand->value.r & 0x1) == 0)
	    {
	      M_assert (0,
			"Limpact_conflicting_operands: "
			"illegal even double regsiter after regalloc!");
	    }

	  /* Set up conflicting double register */
	  conflict_array[i++] = L_copy_operand (operand);

	  /* Set up conflicting float registers. */
	  /* Left float reg # + M_Limpact_dbl_offset  = double reg # */
	  conflict_array[i] = L_copy_operand (operand);
	  conflict_array[i]->ctype = L_CTYPE_FLOAT;
	  conflict_array[i++]->value.r =
	    operand->value.r - M_Limpact_dbl_offset;

	  /* Right float reg # + M_Limpact_dbl_offset - 1 = double reg # */
	  conflict_array[i] = L_copy_operand (operand);
	  conflict_array[i]->ctype = L_CTYPE_FLOAT;
	  conflict_array[i++]->value.r =
	    (operand->value.r - M_Limpact_dbl_offset) + 1;

#if 0
	  /* Debug */
	  fprintf (stderr, "For operand: ");
	  L_print_operand (stderr, operand, 1);
	  fprintf (stderr, "\nConflict array: ");
	  L_print_operand (stderr, conflict_array[0], 1);
	  L_print_operand (stderr, conflict_array[1], 1);
	  L_print_operand (stderr, conflict_array[2], 1);
	  fprintf (stderr, "\n\n");
#endif
	}
      /* Made match Limpact's register bank definitions -JCG 6/99 */
      else if (L_is_ctype_flt (operand))
	{
	  /* If left register */
	  if (operand->value.r & 0x1)
	    {
	      /* Set up conflicting float register */
	      conflict_array[i++] = L_copy_operand (operand);

	      /* Set up conflicting double register */
	      /* Left float reg # + M_Limpact_dbl_offset  = double reg # */
	      conflict_array[i] = L_copy_operand (operand);
	      conflict_array[i]->ctype = L_CTYPE_DOUBLE;
	      conflict_array[i++]->value.r =
		operand->value.r + M_Limpact_dbl_offset;

#if 0
	      /* Debug */
	      fprintf (stderr, "For operand: ");
	      L_print_operand (stderr, operand, 1);
	      fprintf (stderr, "\nConflict array: ");
	      L_print_operand (stderr, conflict_array[0], 1);
	      L_print_operand (stderr, conflict_array[1], 1);
	      fprintf (stderr, "\n\n");
#endif
	    }
	  else
	    {
	      /* Set up conflicting float register */
	      conflict_array[i++] = L_copy_operand (operand);

	      /* Set up conflicting double register */
	      /* Right float reg # + M_Limpact_dbl_offset - 1 =double reg # */
	      conflict_array[i] = L_copy_operand (operand);
	      conflict_array[i]->ctype = L_CTYPE_DOUBLE;
	      conflict_array[i++]->value.r =
		operand->value.r + M_Limpact_dbl_offset - 1;

#if 0
	      /* Debug */
	      fprintf (stderr, "For operand: ");
	      L_print_operand (stderr, operand, 1);
	      fprintf (stderr, "\nConflict array: ");
	      L_print_operand (stderr, conflict_array[0], 1);
	      L_print_operand (stderr, conflict_array[1], 1);
	      fprintf (stderr, "\n\n");
#endif
	    }
	}
#if 1
      else if (L_MAC_PRED_ALL)
	{
	  int j, num_preds, pred_base;

	  conflict_array[i++] =
	    L_new_macro_operand (L_MAC_PRED_ALL, L_CTYPE_PREDICATE,
				 L_PTYPE_NULL);

	  num_preds = Mspec_num_prd_callee_reg;
	  pred_base = (Mspec_num_int_caller_reg +
		       Mspec_num_int_callee_reg + 
		       Mspec_num_flt_caller_reg*2 +
		       Mspec_num_flt_callee_reg*2);
	  for (j = 0; j < num_preds; j++)
	    {
	      conflict_array[i++] =
		L_new_register_operand (pred_base + j,
					L_CTYPE_PREDICATE, 
					L_PTYPE_NULL);
	    }
	}
#endif
      /* pred register */
      else if (L_is_ctype_predicate (operand))
	{
	  conflict_array[i++] = L_copy_operand (operand);
#if 1
	  conflict_array[i++] = L_new_macro_operand (L_MAC_PRED_ALL, 
						     L_CTYPE_PREDICATE,
						     L_PTYPE_NULL);
#endif
	}
      else
	{
	  M_assert (0, "Limpact_conflicting_operands: illegal reg number");
	}
    }
  else if (L_is_macro (operand))
    {
      conflict_array[i++] = L_copy_operand (operand);
    }
  else
    {
      printf("%d %d\n",i,len);
      M_assert (0, "M_conflicting_operands_impact_lcode: unsupported operand\n");
    }

  if (i > len)
    {
      printf("%d %d\n",i,len);
      M_assert (0,"M_conflicting_operands_impact_lcode: too many conflicts\n");
    }

  return (i);
}


/*
 *	This file is essentially a combination of the impact_lcode version
 *	and the hppa version (from m_hppa.c) for the Lhppa macros
 */
static int
M_conflicting_operands_impact_hp_lcode (L_Operand * operand,
					L_Operand ** conflict_array, int len,
					int prepass)
{
  int right, left;

  if (prepass && (!L_is_macro (operand)))
    {
      conflict_array[0] = L_copy_operand (operand);
      return (1);
    }
  if (L_is_macro (operand))
    {
      switch (operand->value.mac)
	{
	case L_MAC_SP:
	case HPPA_MAC_TRUE_SP:
	case L_MAC_SAFE_MEM:
	  conflict_array[0] =
	    L_new_macro_operand (L_MAC_SP, L_CTYPE_INT, L_PTYPE_NULL);
	  conflict_array[1] =
	    L_new_macro_operand (HPPA_MAC_TRUE_SP, L_CTYPE_INT, L_PTYPE_NULL);
	  conflict_array[2] =
	    L_new_macro_operand (L_MAC_SAFE_MEM, L_CTYPE_INT, L_PTYPE_NULL);
	  return (3);
	case L_MAC_P4:
	case L_MAC_P5:
	case L_MAC_P6:
	case L_MAC_P7:
	  left = operand->value.mac;
	  right = left + 4;
	  break;
	case L_MAC_P8:
	case L_MAC_P9:
	case L_MAC_P10:
	case L_MAC_P11:
	  right = operand->value.mac;
	  left = right - 4;
	  break;
	default:
	  conflict_array[0] = L_copy_operand (operand);
	  return (1);
	}
    }
  else if (L_is_reg (operand))
    {
      /* int register */
      if (L_is_ctype_integer (operand))
	{
	  conflict_array[0] = L_copy_operand (operand);
	  return (1);
	}
      /* Made match Limpact's register bank definitions -JCG 6/99 */
      else if (L_is_ctype_dbl (operand))
	{
	  /* Sanity check, double register operand number must be odd
	   * after register allocation!
	   */
	  if ((operand->value.r & 0x1) == 0)
	    {
	      M_assert (0,
			"Limpact_conflicting_operands: "
			"illegal even double regsiter after regalloc!");
	      return (0);
	    }

	  /* Set up conflicting double register */
	  conflict_array[0] = L_copy_operand (operand);

	  /* Set up conflicting float registers. */
	  /* Left float reg # + M_Limpact_dbl_offset  = double reg # */
	  conflict_array[1] = L_copy_operand (operand);
	  conflict_array[1]->ctype = L_CTYPE_FLOAT;
	  conflict_array[1]->value.r =
	    operand->value.r - M_Limpact_dbl_offset;

	  /* Right float reg # + M_Limpact_dbl_offset - 1 = double reg # */
	  conflict_array[2] = L_copy_operand (operand);
	  conflict_array[2]->ctype = L_CTYPE_FLOAT;
	  conflict_array[2]->value.r =
	    (operand->value.r - M_Limpact_dbl_offset) + 1;

#if 0
	  /* Debug */
	  fprintf (stderr, "For operand: ");
	  L_print_operand (stderr, operand, 1);
	  fprintf (stderr, "\nConflict array: ");
	  L_print_operand (stderr, conflict_array[0], 1);
	  L_print_operand (stderr, conflict_array[1], 1);
	  L_print_operand (stderr, conflict_array[2], 1);
	  fprintf (stderr, "\n\n");
#endif
	  return (3);
	}
      /* Made match Limpact's register bank definitions -JCG 6/99 */
      else if (L_is_ctype_flt (operand))
	{
	  /* If left register */
	  if (operand->value.r & 0x1)
	    {
	      /* Set up conflicting float register */
	      conflict_array[0] = L_copy_operand (operand);

	      /* Set up conflicting double register */
	      /* Left float reg # + M_Limpact_dbl_offset  = double reg # */
	      conflict_array[1] = L_copy_operand (operand);
	      conflict_array[1]->ctype = L_CTYPE_DOUBLE;
	      conflict_array[1]->value.r =
		operand->value.r + M_Limpact_dbl_offset;

#if 0
	      /* Debug */
	      fprintf (stderr, "For operand: ");
	      L_print_operand (stderr, operand, 1);
	      fprintf (stderr, "\nConflict array: ");
	      L_print_operand (stderr, conflict_array[0], 1);
	      L_print_operand (stderr, conflict_array[1], 1);
	      fprintf (stderr, "\n\n");
#endif

	      return (2);
	    }
	  else
	    {
	      /* Set up conflicting float register */
	      conflict_array[0] = L_copy_operand (operand);

	      /* Set up conflicting double register */
	      /* Right float reg # + M_Limpact_dbl_offset - 1 =double reg # */
	      conflict_array[1] = L_copy_operand (operand);
	      conflict_array[1]->ctype = L_CTYPE_DOUBLE;
	      conflict_array[1]->value.r =
		operand->value.r + M_Limpact_dbl_offset - 1;

#if 0
	      /* Debug */
	      fprintf (stderr, "For operand: ");
	      L_print_operand (stderr, operand, 1);
	      fprintf (stderr, "\nConflict array: ");
	      L_print_operand (stderr, conflict_array[0], 1);
	      L_print_operand (stderr, conflict_array[1], 1);
	      fprintf (stderr, "\n\n");
#endif

	      return (2);
	    }
	}
      /* pred register */
      else if (L_is_ctype_predicate (operand))
	{
	  conflict_array[0] = L_copy_operand (operand);
	  return (1);
	}
      else
	{
	  M_assert (0, "Limpact_conflicting_operands: illegal reg number");
	  return (0);
	}
    }
  else
    {
      M_assert (0, "Limpact_conflicting_operands: unsupported operand type");
      return (0);
    }

  M_assert (0,
	    "M_conflicting_operands_impact_hp_lcode: "
	    "unsupported operand type");
  return (0);

}

/*
 *	This file is essentially a combination of the impact_lcode version
 *	and the sparc version (from m_sparc.c) for the Lsparc macros
 */
static int
M_conflicting_operands_impact_sparc_lcode (L_Operand * operand,
					   L_Operand ** conflict_array,
					   int len, int prepass)
{
  if (prepass && (!L_is_macro (operand)))
    {
      conflict_array[0] = L_copy_operand (operand);
      return (1);
    }
  if (L_is_macro (operand))
    {
      switch (operand->value.mac)
	{
	default:
	  conflict_array[0] = L_copy_operand (operand);
	  return (1);
	}
    }
  else if (L_is_reg (operand))
    {
      /* int register */
      if (L_is_ctype_integer (operand))
	{
	  conflict_array[0] = L_copy_operand (operand);
	  return (1);
	}
      /* Made match Limpact's register bank definitions -JCG 6/99 */
      else if (L_is_ctype_dbl (operand))
	{
	  /* Sanity check, double register operand number must be odd
	   * after register allocation!
	   */
	  if ((operand->value.r & 0x1) == 0)
	    {
	      M_assert (0,
			"Limpact_conflicting_operands: "
			"illegal even double regsiter after regalloc!");
	      return (0);
	    }

	  /* Set up conflicting double register */
	  conflict_array[0] = L_copy_operand (operand);

	  /* Set up conflicting float registers. */
	  /* Left float reg # + M_Limpact_dbl_offset  = double reg # */
	  conflict_array[1] = L_copy_operand (operand);
	  conflict_array[1]->ctype = L_CTYPE_FLOAT;
	  conflict_array[1]->value.r =
	    operand->value.r - M_Limpact_dbl_offset;

	  /* Right float reg # + M_Limpact_dbl_offset - 1 = double reg # */
	  conflict_array[2] = L_copy_operand (operand);
	  conflict_array[2]->ctype = L_CTYPE_FLOAT;
	  conflict_array[2]->value.r =
	    (operand->value.r - M_Limpact_dbl_offset) + 1;

#if 0
	  /* Debug */
	  fprintf (stderr, "For operand: ");
	  L_print_operand (stderr, operand, 1);
	  fprintf (stderr, "\nConflict array: ");
	  L_print_operand (stderr, conflict_array[0], 1);
	  L_print_operand (stderr, conflict_array[1], 1);
	  L_print_operand (stderr, conflict_array[2], 1);
	  fprintf (stderr, "\n\n");
#endif
	  return (3);
	}
      /* Made match Limpact's register bank definitions -JCG 6/99 */
      else if (L_is_ctype_flt (operand))
	{
	  /* If left register */
	  if (operand->value.r & 0x1)
	    {
	      /* Set up conflicting float register */
	      conflict_array[0] = L_copy_operand (operand);

	      /* Set up conflicting double register */
	      /* Left float reg # + M_Limpact_dbl_offset  = double reg # */
	      conflict_array[1] = L_copy_operand (operand);
	      conflict_array[1]->ctype = L_CTYPE_DOUBLE;
	      conflict_array[1]->value.r =
		operand->value.r + M_Limpact_dbl_offset;

#if 0
	      /* Debug */
	      fprintf (stderr, "For operand: ");
	      L_print_operand (stderr, operand, 1);
	      fprintf (stderr, "\nConflict array: ");
	      L_print_operand (stderr, conflict_array[0], 1);
	      L_print_operand (stderr, conflict_array[1], 1);
	      fprintf (stderr, "\n\n");
#endif

	      return (2);
	    }
	  else
	    {
	      /* Set up conflicting float register */
	      conflict_array[0] = L_copy_operand (operand);

	      /* Set up conflicting double register */
	      /* Right float reg # + M_Limpact_dbl_offset - 1 =double reg # */
	      conflict_array[1] = L_copy_operand (operand);
	      conflict_array[1]->ctype = L_CTYPE_DOUBLE;
	      conflict_array[1]->value.r =
		operand->value.r + M_Limpact_dbl_offset - 1;

#if 0
	      /* Debug */
	      fprintf (stderr, "For operand: ");
	      L_print_operand (stderr, operand, 1);
	      fprintf (stderr, "\nConflict array: ");
	      L_print_operand (stderr, conflict_array[0], 1);
	      L_print_operand (stderr, conflict_array[1], 1);
	      fprintf (stderr, "\n\n");
#endif

	      return (2);
	    }
	}
      /* pred register */
      else if (L_is_ctype_predicate (operand))
	{
	  conflict_array[0] = L_copy_operand (operand);
	  return (1);
	}
      else
	{
	  M_assert (0,
		    "M_conflicting_operands_impact_sparc_lcode: "
		    "illegal reg number");
	  return (0);
	}
    }

  M_assert (0,
	    "M_conflicting_operands_impact_sparc_lcode: "
	    "unsupported operand type");
  return (0);
}

int
M_conflicting_operands_impact (L_Operand * operand,
			       L_Operand ** conflict_array, int len,
			       int prepass)
{
  if (M_impact_model == M_IM_LCODE)
    {
      return M_conflicting_operands_impact_lcode (operand,
						  conflict_array, len,
						  prepass);
    }
  else if (M_impact_model == M_IM_VER_1)
    {
      return M_conflicting_operands_impact_lcode (operand,
						  conflict_array, len,
						  prepass);
    }
  else if ((M_impact_model == M_IM_HP_LCODE) ||
	   (M_impact_model == M_IM_HP_MCODE))
    {
      return M_conflicting_operands_impact_hp_lcode (operand,
						     conflict_array, len,
						     prepass);
    }
  else if ((M_impact_model == M_IM_SPARC_LCODE) ||
	   (M_impact_model == M_IM_SPARC_MCODE))
    {
      return M_conflicting_operands_impact_sparc_lcode (operand,
							conflict_array, len,
							prepass);
    }
  else
    {
      M_assert (0, "M_conflicting_operands_impact: illegal machine model");
      return (0);
    }

}

void
M_define_macros_impact (STRING_Symbol_Table * sym_tbl)
{
  if (M_impact_model == M_IM_LCODE)
    {
      return;
    }
  else if (M_impact_model == M_IM_VER_1)
    {
      return;
    }
  else if ((M_impact_model == M_IM_HP_LCODE) ||
	   (M_impact_model == M_IM_HP_MCODE))
    {
      M_define_macros_hppa (sym_tbl);
    }
  else if ((M_impact_model == M_IM_SPARC_LCODE) ||
	   (M_impact_model == M_IM_SPARC_MCODE))
    {
      M_define_macros_sparc (sym_tbl);
    }
  else
    {
      M_assert (0, "M_define_macros_impact: illegal machine model");
    }
}

char *
M_get_macro_name_impact (int id)
{

  if (M_impact_model == M_IM_LCODE)
    {
      return "?";
    }
  else if (M_impact_model == M_IM_VER_1)
    {
      return "?";
    }
  else if ((M_impact_model == M_IM_HP_LCODE) ||
	   (M_impact_model == M_IM_HP_MCODE))
    {
      return (M_get_macro_name_hppa (id));
    }
  else if ((M_impact_model == M_IM_SPARC_LCODE) ||
	   (M_impact_model == M_IM_SPARC_MCODE))
    {
      return (M_get_macro_name_sparc (id));
    }
  else
    {
      M_assert (0, "M_get_macro_name_impact: illegal machine model");
      return (NULL);
    }
}

int
M_num_registers_impact (int ctype)
{
  switch (ctype)
    {
    case L_CTYPE_INT:
      return (64);
    case L_CTYPE_FLOAT:
      return (64);
    case L_CTYPE_DOUBLE:
      return (32);
    case L_CTYPE_PREDICATE:
      return (64);
    default:
      return (0);
    }
}
