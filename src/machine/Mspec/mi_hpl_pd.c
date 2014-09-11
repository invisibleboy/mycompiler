/*****************************************************************************\
 *
 *		      Illinois Open Source License
 *                     University of Illinois/NCSA
 *                         Open Source License
 *
 * Copyright (c) 2002, The University of Illinois at Urbana-Champaign.
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
 *	File :	ml_hpl_pd.c  (renamed from ml_playdoh.c -JCG 7/14/98)
 *	Desc :	Machine dependent specification.  
 *	Date :	August 1993
 *	Auth :  Scott A. Mahlke, Wen-mei Hwu
 *
 *==========================================================================*/

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
#include <machine/m_hpl_pd.h>
#include <machine/m_hppa.h>
#include <machine/m_sparc.h>

/*
 *  5-20-95
 *	Playdoh currently supports 3 models:
 *	1. v1-hp - Playdoh instruction set; with HP calling convention,
 *		parameter passing macros, data layout, stack layout, etc.
 *		Thus you can feed HP Lcode to playdoh but NOT HP Mcode (ie,
 *		code run thru phase1 of Lhppa).
 *	2. v1-sun - Playdoh instruction set; with Sparc calling convention,
 *		parameter passing macros, data layout, stack layout, etc.
 *		Thus you can feed Sparc Lcode to playdoh but 
 *              NOT Sparc Mcode (ie, code run thru phase1 of Lsparc).
 *	3. v1.0 - This is purely configurable to any sort of 
 *              calling convention, data layout, stack layout that you desire.
 *              Currently it it is set up identical to HP-PA because we don't  
 *              have much originality.  Also the emulator operates on an HP 
 *              or Sparc, so if you ever want the code to run, you best use 
 *              one of those 2 platforms.
 *	If you have questions talk to Rick or Scott.
 *
 *  12-04
 *      Only the last model (v1.0) is currently supported
 */

/* model compiling for, internal to this Mspec only */
int M_playdoh_model;

/*--------------------------------------------------------------------------*/
#define M_PLAYDOH_SIZE_VOID	0
#define M_PLAYDOH_SIZE_BIT	1
#define M_PLAYDOH_SIZE_CHAR	8
#define M_PLAYDOH_SIZE_SHORT	16
#define M_PLAYDOH_SIZE_INT	32
#define M_PLAYDOH_SIZE_LONG	32
#define M_PLAYDOH_SIZE_FLOAT	32
#define M_PLAYDOH_SIZE_DOUBLE	64
#define M_PLAYDOH_SIZE_POINTER	32
#define M_PLAYDOH_SIZE_LLONG    64
#define M_PLAYDOH_SIZE_UNION	-1
#define M_PLAYDOH_SIZE_STRUCT	-1
#define M_PLAYDOH_SIZE_BLOCK	-1
#define M_PLAYDOH_SIZE_MAX 	64

#define M_PLAYDOH_ALIGN_VOID	-1
#define M_PLAYDOH_ALIGN_BIT	1
#define M_PLAYDOH_ALIGN_CHAR	8
#define M_PLAYDOH_ALIGN_SHORT	16
#define M_PLAYDOH_ALIGN_INT	32
#define M_PLAYDOH_ALIGN_LONG	32
#define M_PLAYDOH_ALIGN_FLOAT	32
#define M_PLAYDOH_ALIGN_DOUBLE	64
#define M_PLAYDOH_ALIGN_POINTER	32
#define M_PLAYDOH_ALIGN_LLONG   64
#define M_PLAYDOH_ALIGN_UNION	-1	/* depends on the field */
#define M_PLAYDOH_ALIGN_STRUCT	-1
#define M_PLAYDOH_ALIGN_BLOCK	-1
#define M_PLAYDOH_ALIGN_MAX	64

int
M_playdoh_type_size (int mtype)
{
  if (M_playdoh_model == M_PLAYDOH_V1)
    {
      switch (mtype)
	{
	case M_TYPE_VOID:
	  return M_PLAYDOH_SIZE_VOID;
	case M_TYPE_BIT_LONG:
	case M_TYPE_BIT_LLONG:
	case M_TYPE_BIT_CHAR:
	case M_TYPE_BIT_SHORT:
	case M_TYPE_BIT_INT:
	  return M_PLAYDOH_SIZE_BIT;
	case M_TYPE_CHAR:
	  return M_PLAYDOH_SIZE_CHAR;
	case M_TYPE_SHORT:
	  return M_PLAYDOH_SIZE_SHORT;
	case M_TYPE_INT:
	  return M_PLAYDOH_SIZE_INT;
	case M_TYPE_LONG:
	  return M_PLAYDOH_SIZE_LONG;
	case M_TYPE_LLONG:
	  return M_PLAYDOH_SIZE_LLONG;
	case M_TYPE_FLOAT:
	  return M_PLAYDOH_SIZE_FLOAT;
	case M_TYPE_DOUBLE:
	  return M_PLAYDOH_SIZE_DOUBLE;
	case M_TYPE_POINTER:
	  return M_PLAYDOH_SIZE_POINTER;
	case M_TYPE_UNION:
	  return M_PLAYDOH_SIZE_UNION;
	case M_TYPE_STRUCT:
	  return M_PLAYDOH_SIZE_STRUCT;
	case M_TYPE_BLOCK:
	  return M_PLAYDOH_SIZE_BLOCK;
	default:
	  return -1;
	}
    }
  else
    {
      M_assert (0, "M_playdoh_type_size: illegal machine model");
      return (0);
    }

}

int
M_playdoh_type_align (int mtype)
{
  if (M_playdoh_model == M_PLAYDOH_V1)
    {
      switch (mtype)
	{
	case M_TYPE_VOID:
	  return M_PLAYDOH_ALIGN_VOID;
	case M_TYPE_BIT_LONG:
	  return M_PLAYDOH_ALIGN_BIT;
	case M_TYPE_BIT_SHORT:
	  return M_PLAYDOH_ALIGN_BIT;
	case M_TYPE_BIT_CHAR:
	  return M_PLAYDOH_ALIGN_BIT;
	case M_TYPE_CHAR:
	  return M_PLAYDOH_ALIGN_CHAR;
	case M_TYPE_SHORT:
	  return M_PLAYDOH_ALIGN_SHORT;
	case M_TYPE_INT:
	  return M_PLAYDOH_ALIGN_INT;
	case M_TYPE_LONG:
	  return M_PLAYDOH_ALIGN_LONG;
	case M_TYPE_LLONG:
	  return M_PLAYDOH_ALIGN_LLONG;
	case M_TYPE_FLOAT:
	  return M_PLAYDOH_ALIGN_FLOAT;
	case M_TYPE_DOUBLE:
	  return M_PLAYDOH_ALIGN_DOUBLE;
	case M_TYPE_POINTER:
	  return M_PLAYDOH_ALIGN_POINTER;
	case M_TYPE_UNION:
	  return M_PLAYDOH_ALIGN_UNION;
	case M_TYPE_STRUCT:
	  return M_PLAYDOH_ALIGN_STRUCT;
	case M_TYPE_BLOCK:
	  return M_PLAYDOH_ALIGN_BLOCK;
	default:
	  return -1;
	}
    }
  else
    {
      M_assert (0, "M_playdoh_type_align: illegal machine model");
      return (0);
    }
}

/*
 *	The following functions are for documentation only.
 *	The function overhead is too great for actual usage.
 */
void
M_playdoh_bit_llong (M_Type type, int n)
{
  type->type = M_TYPE_BIT_LLONG;
  type->unsign = 1;
  type->align = M_PLAYDOH_ALIGN_BIT;
  type->size = n * M_PLAYDOH_SIZE_BIT;
  type->nbits = n * M_PLAYDOH_SIZE_BIT;
  M_assert ((n <= 64),
	    "M_bit_long: do not allow bit field of more than 64 bits");
}

void
M_playdoh_char (M_Type type, int unsign)
{
  if (M_playdoh_model == M_PLAYDOH_V1)
    {
      type->type = M_TYPE_CHAR;
      type->unsign = (unsign != 0);
      type->align = M_PLAYDOH_ALIGN_CHAR;
      type->size = M_PLAYDOH_SIZE_CHAR;
      type->nbits = M_PLAYDOH_SIZE_CHAR;
    }
  else
    {
      M_assert (0, "M_playdoh_char: illegal machine model");
    }
}

void
M_playdoh_short (M_Type type, int unsign)
{
  if (M_playdoh_model == M_PLAYDOH_V1)
    {
      type->type = M_TYPE_SHORT;
      type->unsign = (unsign !=0);
      type->align = M_PLAYDOH_ALIGN_SHORT;
      type->size = M_PLAYDOH_SIZE_SHORT;
      type->nbits = M_PLAYDOH_SIZE_SHORT;
    }
  else
    {
      M_assert (0, "M_playdoh_short: illegal machine model");
    }
}

void
M_playdoh_int (M_Type type, int unsign)
{
  if (M_playdoh_model == M_PLAYDOH_V1)
    {
      type->type = M_TYPE_INT;
      type->unsign = (unsign != 0);
      type->align = M_PLAYDOH_ALIGN_INT;
      type->size = M_PLAYDOH_SIZE_INT;
      type->nbits = M_PLAYDOH_SIZE_INT;
    }
  else
    {
      M_assert (0, "M_playdoh_int: illegal machine model");
    }
}

void
M_playdoh_long (M_Type type, int unsign)
{
  if (M_playdoh_model == M_PLAYDOH_V1)
    {
      type->type = M_TYPE_LONG;
      type->unsign = (unsign != 0);
      type->align = M_PLAYDOH_ALIGN_LONG;
      type->size = M_PLAYDOH_SIZE_LONG;
      type->nbits = M_PLAYDOH_SIZE_LONG;
    }
  else
    {
      M_assert (0, "M_playdoh_long: illegal machine model");
    }
}


void
M_playdoh_llong (M_Type type, int unsign)
{
  type->type = M_TYPE_LLONG;
  type->unsign = unsign;
  type->align = M_PLAYDOH_ALIGN_LLONG;
  type->size = M_PLAYDOH_SIZE_LLONG;
  type->nbits = M_PLAYDOH_SIZE_LLONG;
}


int
M_playdoh_layout_order (void)
{
  if (M_playdoh_model == M_PLAYDOH_V1)
    {
      if (M_read_database_i ("_HT__info", "little_endian", "value") == 1)
	{
	  return M_LITTLE_ENDIAN;
	}
      else
	{
	  return M_BIG_ENDIAN;
	}

      // Made consistent with Lcode model for impact architecture
      //      return M_BIG_ENDIAN;
    }
  else
    {
      M_assert (0, "M_playdoh_layout_order: illegal machine model");
      return (0);
    }
}

void
M_set_model_playdoh (char *model_name)
{
  if (!strcasecmp (model_name, "V1.0") ||
      !strcasecmp (model_name, "V1.1"))
    {
      M_model = M_PLAYDOH_V1;
      M_playdoh_model = M_PLAYDOH_V1;

      // Made consistent with Lcode model for impact architecture
      /* Use the layout database to answer as many queries as possible */
      M_use_layout_database = 0;
    }
  else
    {
      fprintf (stderr, "illegal model_name : %s\n", model_name);
      fprintf (stderr,
	       "Use one of the following: V1.0, V1.1, V1-HP, V1-SUN\n");
      M_assert (0, "M_set_model_impact: illegal model name specified");
    }
}
