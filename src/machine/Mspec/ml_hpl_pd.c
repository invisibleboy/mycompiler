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
 *	File :	ml_hpl_pd.c  (Renamed from ml_playdoh.c -JCG 7/14/98)
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

#define JWS_NEW_STACK_MODEL

/*
 *  5-20-95
 *	Playdoh currently supports 3 models:
 *	1. v1-hp - Playdoh instruction set; with HP calling convention,
 *		parameter passing macros, data layout, stack layout, etc.
 *		Thus you can feed HP Lcode to playdoh but NOT HP Mcode (ie,
 *		code run thru phase1 of Lhppa).
 *	2. v1-sun - Playdoh instruction set; with Sparc calling convention,
 *		parameter passing macros, data layout, stack layout, etc.
 *		Thus you can feed Sparc Lcode to playdoh but NOT Sparc Mcode 
                (ie, code run thru phase1 of Lsparc).
 *	3. v1.0 - This is purely configurable to any sort of calling 
 *              convention, data layout, stack layout that you desire.  
 *              Currently it is set up identical to HP-PA because we don't 
 *              have much originality.  Also the emulator operates on an HP 
 *              or Sparc, so if you ever want the code to run, you best use 
 *              one of those 2 platforms.
 *	If you have questions talk to Rick or Scott.
 *
 *  12-04
 *      Only the last model (v1.0) is currently supported
 */

/* model compiling for, internal to this Mspec only */
extern int M_playdoh_model;
Set Set_playdoh_fragile_macro = NULL;

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
#define M_PLAYDOH_ALIGN_DOUBLE	32
#define M_PLAYDOH_ALIGN_POINTER	32
#define M_PLAYDOH_ALIGN_UNION	-1	/* depends on the field */
#define M_PLAYDOH_ALIGN_STRUCT	-1
#define M_PLAYDOH_ALIGN_BLOCK	-1
#define M_PLAYDOH_ALIGN_MAX	64

/*--------------------------------------------------------------------------*/
#define M_PLAYDOH_MAX_FNVAR_REG 	4
#define M_PLAYDOH_SMALL_STRUCT_MAX 	64
#define MIN_PARAM_SIZE 	         (M_PLAYDOH_MAX_FNVAR_REG * M_PLAYDOH_SIZE_INT)

/* incoming and outgoing parameters */
#define M_PLAYDOH_INT_BASE		0
#define M_PLAYDOH_FLT_BASE		4

#define M_PLAYDOH_RET_I32		15	/* gr28 */
#define M_PLAYDOH_RET_I64		15	/* gr28 & gr29 */
#define M_PLAYDOH_RET_ST		15	/* gr28 */
#define M_PLAYDOH_RET_F			4	/* fr04 */

/*--------------------------------------------------------------------------*/
/*
 *	The following functions are for documentation only.
 *	The function overhead is too great for actual usage.
 */
void
M_playdoh_void (M_Type type)
{
  if (M_playdoh_model == M_PLAYDOH_V1)
    {
      type->type = M_TYPE_VOID;
      type->unsign = 1;
      type->align = M_PLAYDOH_ALIGN_VOID;
      type->size = M_PLAYDOH_SIZE_VOID;
      type->nbits = 0;
    }
  else
    {
      M_assert (0, "M_playdoh_void: illegal machine model");
    }
}

void
M_playdoh_bit_long (M_Type type, int n)
{
  if (M_playdoh_model == M_PLAYDOH_V1)
    {
      type->type = M_TYPE_BIT_LONG;
      type->unsign = 1;
      type->align = M_PLAYDOH_ALIGN_BIT;
      type->size = n * M_PLAYDOH_SIZE_BIT;
      type->nbits = n * M_PLAYDOH_SIZE_BIT;
      M_assert ((n <= 32),
		"M_bit_long: do not allow bit field of more than 32 bits");
    }
  else
    {
      M_assert (0, "M_playdoh_bit_long: illegal machine model");
    }
}

void
M_playdoh_bit_int (M_Type type, int n)
{
  if (M_playdoh_model == M_PLAYDOH_V1)
    {
      type->type = M_TYPE_BIT_INT;
      type->unsign = 1;
      type->align = M_PLAYDOH_ALIGN_BIT;
      type->size = n * M_PLAYDOH_SIZE_BIT;
      type->nbits = n * M_PLAYDOH_SIZE_BIT;
      M_assert ((n <= 32),
		"M_bit_int: do not allow bit field of more than 32 bits");
    }
  else
    {
      M_assert (0, "M_playdoh_bit_int: illegal machine model");
    }
}

void
M_playdoh_bit_short (M_Type type, int n)
{
  if (M_playdoh_model == M_PLAYDOH_V1)
    {
      type->type = M_TYPE_BIT_SHORT;
      type->unsign = 1;
      type->align = M_PLAYDOH_ALIGN_BIT;
      type->size = n * M_PLAYDOH_SIZE_BIT;
      type->nbits = n * M_PLAYDOH_SIZE_BIT;
      M_assert ((n <= 16),
		"M_bit_long: do not allow bit field of more than 16 bits");
    }
  else
    {
      M_assert (0, "M_playdoh_bit_short: illegal machine model");
    }
}

void
M_playdoh_bit_char (M_Type type, int n)
{
  if (M_playdoh_model == M_PLAYDOH_V1)
    {
      type->type = M_TYPE_BIT_CHAR;
      type->unsign = 1;
      type->align = M_PLAYDOH_ALIGN_BIT;
      type->size = n * M_PLAYDOH_SIZE_BIT;
      type->nbits = n * M_PLAYDOH_SIZE_BIT;
      M_assert ((n <= 8),
		"M_bit_char: do not allow bit field of more than 8 bits");
    }
  else
    {
      M_assert (0, "M_playdoh_bit_char: illegal machine model");
    }
}

void
M_playdoh_float (M_Type type, int unsign)
{
  if (M_playdoh_model == M_PLAYDOH_V1)
    {
      type->type = M_TYPE_FLOAT;
      type->unsign = (unsign != 0);
      type->align = M_PLAYDOH_ALIGN_FLOAT;
      type->size = M_PLAYDOH_SIZE_FLOAT;
      type->nbits = M_PLAYDOH_SIZE_FLOAT;
    }
  else
    {
      M_assert (0, "M_playdoh_float: illegal machine model");
    }
}

void
M_playdoh_double (M_Type type, int unsign)
{
  if (M_playdoh_model == M_PLAYDOH_V1)
    {
      type->type = M_TYPE_DOUBLE;
      type->unsign = (unsign != 0);
      type->align = M_PLAYDOH_ALIGN_DOUBLE;
      type->size = M_PLAYDOH_SIZE_DOUBLE;
      type->nbits = M_PLAYDOH_SIZE_DOUBLE;
    }
  else
    {
      M_assert (0, "M_playdoh_double: illegal machine model");
    }
}

void
M_playdoh_pointer (M_Type type)
{
  if (M_playdoh_model == M_PLAYDOH_V1)
    {
      type->type = M_TYPE_POINTER;
      type->unsign = 1;
      type->align = M_PLAYDOH_ALIGN_POINTER;
      type->size = M_PLAYDOH_SIZE_POINTER;
      type->nbits = M_PLAYDOH_SIZE_POINTER;
    }
  else
    {
      M_assert (0, "M_playdoh_pointer: illegal machine model");
    }
}

/*--------------------------------------------------------------------------*/
int
M_playdoh_eval_type (M_Type type, M_Type ntype)
{
  if (M_playdoh_model == M_PLAYDOH_V1)
    {
      switch (type->type)
	{
	case M_TYPE_VOID:
	  M_playdoh_void (ntype);
	  return (-1);		/* can not be evaluated */
	case M_TYPE_BIT_LONG:
	case M_TYPE_BIT_CHAR:
	case M_TYPE_CHAR:
	case M_TYPE_SHORT:
	case M_TYPE_INT:
	case M_TYPE_LONG:
	case M_TYPE_POINTER:
	case M_TYPE_BLOCK:
	  /* the starting address of array is used */
	  M_playdoh_int (ntype, type->unsign);
	  return (M_TYPE_INT);
	case M_TYPE_LLONG:
	  M_playdoh_llong (ntype, type->unsign);
	  break;
	case M_TYPE_FLOAT:
	  M_playdoh_float (ntype, type->unsign);
	  return (M_TYPE_FLOAT);
	case M_TYPE_DOUBLE:
	  M_playdoh_double (ntype, type->unsign);
	  return (M_TYPE_DOUBLE);
	case M_TYPE_UNION:
	case M_TYPE_STRUCT:
	  *ntype = *type;
	  return type->type;
	default:
	  return (-1);
	}
    }
  else
    {
      M_assert (0, "M_playdoh_eval_type: illegal machine model");
      return (0);
    }
}

int
M_playdoh_eval_type2 (M_Type type, M_Type ntype)
{
  if (M_playdoh_model == M_PLAYDOH_V1)
    {
      switch (type->type)
	{
	case M_TYPE_VOID:
	  M_playdoh_void (ntype);
	  return (-1);		/* can not be evaluated */
	case M_TYPE_BIT_CHAR:
	case M_TYPE_CHAR:
	  M_playdoh_char (ntype, type->unsign);
	  return (M_TYPE_CHAR);
	case M_TYPE_SHORT:
	  M_playdoh_short (ntype, type->unsign);
	  return (M_TYPE_SHORT);
	case M_TYPE_BLOCK:
	case M_TYPE_INT:
	  /* the starting address of array is used */
	  M_playdoh_int (ntype, type->unsign);
	  return (M_TYPE_INT);
	case M_TYPE_BIT_LONG:
	case M_TYPE_LONG:
	  M_playdoh_long (ntype, type->unsign);
	  return (M_TYPE_LONG);
	case M_TYPE_BIT_LLONG:
	case M_TYPE_LLONG:
	  M_playdoh_llong (ntype, type->unsign);
	  return (M_TYPE_INT);
	case M_TYPE_POINTER:
	  M_playdoh_pointer (ntype);
	  return (M_TYPE_POINTER);
	case M_TYPE_FLOAT:
	  M_playdoh_float (ntype, type->unsign);
	  return (M_TYPE_FLOAT);
	case M_TYPE_DOUBLE:
	  M_playdoh_double (ntype, type->unsign);
	  return (M_TYPE_DOUBLE);
	case M_TYPE_UNION:
	case M_TYPE_STRUCT:
	  *ntype = *type;
	  return type->type;
	default:
	  return (-1);
	}
    }
  else
    {
      M_assert (0, "M_playdoh_eval_type: illegal machine model");
      return (0);
    }
}

int
M_playdoh_call_type (M_Type type, M_Type ntype)
{
  if (M_playdoh_model == M_PLAYDOH_V1)
    {
      switch (type->type)
	{
	case M_TYPE_VOID:
	  M_playdoh_void (ntype);
	  return (-1);		/* can not be evaluated */
	case M_TYPE_BIT_CHAR:
	case M_TYPE_CHAR:
	case M_TYPE_SHORT:
	case M_TYPE_BIT_LONG:
	case M_TYPE_INT:
	case M_TYPE_LONG:
	case M_TYPE_POINTER:
	case M_TYPE_BLOCK:
	  /* the starting address of array is used */
	  M_playdoh_int (ntype, type->unsign);
	  return (M_TYPE_INT);
	case M_TYPE_LLONG:
	case M_TYPE_BIT_LLONG:
	  M_playdoh_llong (ntype, type->unsign);
	  return (M_TYPE_LLONG);
	  /* BCC - don't promote float to double now - 8/5/96 */
	case M_TYPE_FLOAT:
	  M_playdoh_float (ntype, type->unsign);
	  return M_TYPE_FLOAT;
	case M_TYPE_DOUBLE:
	  M_playdoh_double (ntype, type->unsign);
	  return M_TYPE_DOUBLE;
	case M_TYPE_UNION:
	case M_TYPE_STRUCT:
	  *ntype = *type;
	  return (type->type);
	default:
	  return (-1);
	}
    }
  else
    {
      M_assert (0, "M_playdoh_call_type: illegal machine model");
      return (0);
    }
}

int
M_playdoh_call_type2 (M_Type type, M_Type ntype)
{
  if (M_playdoh_model == M_PLAYDOH_V1)
    {
      switch (type->type)
	{
	case M_TYPE_VOID:
	  M_playdoh_void (ntype);
	  return (-1);		/* can not be evaluated */
	case M_TYPE_BIT_CHAR:
	case M_TYPE_CHAR:
	  M_playdoh_char (ntype, type->unsign);
	  return (M_TYPE_CHAR);
	case M_TYPE_SHORT:
	  M_playdoh_short (ntype, type->unsign);
	  return (M_TYPE_SHORT);
	case M_TYPE_BLOCK:
	case M_TYPE_INT:
	  /* the starting address of array is used */
	  M_playdoh_int (ntype, type->unsign);
	  return (M_TYPE_INT);
	case M_TYPE_BIT_LONG:
	case M_TYPE_LONG:
	  M_playdoh_long (ntype, type->unsign);
	  return (M_TYPE_LONG);
	case M_TYPE_BIT_LLONG:
	case M_TYPE_LLONG:
	  M_playdoh_llong (ntype, type->unsign);
	  return (M_TYPE_LLONG);
	case M_TYPE_POINTER:
	  M_playdoh_pointer (ntype);
	  return (M_TYPE_POINTER);
	  /* BCC - don't promote float to double now - 8/5/96 */
	case M_TYPE_FLOAT:
	  M_playdoh_float (ntype, type->unsign);
	  return M_TYPE_FLOAT;
	case M_TYPE_DOUBLE:
	  M_playdoh_double (ntype, type->unsign);
	  return M_TYPE_DOUBLE;
	case M_TYPE_UNION:
	case M_TYPE_STRUCT:
	  *ntype = *type;
	  return (type->type);
	default:
	  return (-1);
	}
    }
  else
    {
      M_assert (0, "M_playdoh_call_type: illegal machine model");
      return (0);
    }
}

/*--------------------------------------------------------------------------*/
void
M_playdoh_array_layout (M_Type type, int *offset)
{
  if (M_playdoh_model == M_PLAYDOH_V1)
    {
      *offset = 0;		/* assume first element is aligned */
    }
  else
    {
      M_assert (0, "M_playdoh_array_layout: illegal machine model");
    }
}

int
M_playdoh_array_align (M_Type type)
{
  if (M_playdoh_model == M_PLAYDOH_V1)
    {
      return type->align;
    }
  else
    {
      M_assert (0, "M_playdoh_array_align: illegal machine model");
      return (0);
    }
}

int
M_playdoh_array_size (M_Type type, int dim)
{
  int mod, size, align;

  if (M_playdoh_model == M_PLAYDOH_V1)
    {
      size = type->size;
      align = type->align;
      mod = size % align;
      if (mod != 0)
	size += (align - mod);

      return (size * dim);
    }
  else
    {
      M_assert (0, "M_playdoh_array_size: illegal machine model");
      return (0);
    }

}

/*--------------------------------------------------------------------------*/
void
M_playdoh_union_layout (int n, _M_Type * type, int *offset, int *bit_offset)
{
  int i;
  if (M_playdoh_model == M_PLAYDOH_V1)
    {
      for (i = 0; i < n; i++)
	{			/* assume the union is aligned */
	  offset[i] = 0;
	  bit_offset[i] = 0;
	}
    }
  else
    {
      M_assert (0, "M_playdoh_union_layout: illegal machine model");
    }
}

int
M_playdoh_union_align (int n, _M_Type * type)
{
  if (M_playdoh_model == M_PLAYDOH_V1)
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
       *      align to at least byte boundary.
       */
      if (max < M_PLAYDOH_ALIGN_CHAR)
	max = M_PLAYDOH_ALIGN_CHAR;

      return max;
    }
  else
    {
      M_assert (0, "M_playdoh_union_align: illegal machine model");
      return (0);
    }
}

int
M_playdoh_union_size (int n, _M_Type * type)
{
  if (M_playdoh_model == M_PLAYDOH_V1)
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
       *      align to at least byte boundary.
       */
      if (max_align < M_PLAYDOH_ALIGN_CHAR)
	max_align = M_PLAYDOH_ALIGN_CHAR;

      /* need to increment to the max. align for future array extension */
      i = max_size % max_align;
      if (i != 0)
	max_size += (max_align - i);

      return max_size;
    }
  else
    {
      M_assert (0, "M_playdoh_union_size: illegal machine model");
      return (0);
    }
}

void
M_playdoh_struct_layout (int n, _M_Type * type, int *base, int *bit_offset)
{
  if (M_playdoh_model == M_PLAYDOH_V1)
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
	  mod_word = offset % M_PLAYDOH_SIZE_INT;
	  if (type[i].type == M_TYPE_BIT_CHAR)
	    {
	      if ((mod_word + size) > M_PLAYDOH_SIZE_INT)
		offset += (M_PLAYDOH_SIZE_INT - mod_word);
	      else
		{
		  mod_type = offset % M_PLAYDOH_SIZE_CHAR;
		  if ((mod_type + size) > M_PLAYDOH_SIZE_CHAR)
		    {
		      type[i].type = M_TYPE_BIT_SHORT;
		    }
		}
	    }
	  if (type[i].type == M_TYPE_BIT_SHORT)
	    {
	      if ((mod_word + size) > M_PLAYDOH_SIZE_INT)
		offset += (M_PLAYDOH_SIZE_INT - mod_word);
	      else
		{
		  mod_type = offset % M_PLAYDOH_SIZE_SHORT;
		  if ((mod_type + size) > M_PLAYDOH_SIZE_SHORT)
		    {
		      type[i].type = M_TYPE_BIT_LONG;
		    }
		}
	    }
	  else if (type[i].type == M_TYPE_BIT_LONG)
	    {
	      if ((mod_word + size) > M_PLAYDOH_SIZE_INT)
		offset += (M_PLAYDOH_SIZE_INT - mod_word);
	    }
	  mod = offset % align;	/* align to what the field */
	  if (mod != 0)		/* needs to start from */
	    offset += (align - mod);

	  if (type[i].type == M_TYPE_BIT_CHAR)
	    {
	      int mod = offset % M_PLAYDOH_SIZE_INT;

	      bit_offset[i] = offset - mod +
		(M_PLAYDOH_SIZE_INT - mod - size);
	      base[i] = offset & (~(M_PLAYDOH_SIZE_CHAR - 1));
	    }
	  else if (type[i].type == M_TYPE_BIT_SHORT)
	    {
	      int mod = offset % M_PLAYDOH_SIZE_INT;

	      bit_offset[i] = offset - mod +
		(M_PLAYDOH_SIZE_INT - mod - size);
	      base[i] = offset & (~(M_PLAYDOH_SIZE_SHORT - 1));

	    }
	  else if (type[i].type == M_TYPE_BIT_LONG)
	    {
	      int mod = offset % M_PLAYDOH_SIZE_INT;

	      bit_offset[i] = offset - mod +
		(M_PLAYDOH_SIZE_INT - mod - size);
	      base[i] = offset & (~(M_PLAYDOH_SIZE_LONG - 1));
	    }
	  else
	    {
	      base[i] = offset;
	      bit_offset[i] = 0;
	    }

	  offset += size;	/* allocate space */
	}
    }
  else
    {
      M_assert (0, "M_playdoh_struct_layout: illegal machine model");
    }
}

int
M_playdoh_struct_align (int n, _M_Type * type)
{
  if (M_playdoh_model == M_PLAYDOH_V1)
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
       *      align to at least byte boundary.
       */
      if (max < M_PLAYDOH_ALIGN_CHAR)
	max = M_PLAYDOH_ALIGN_CHAR;
      return max;
    }
  else
    {
      M_assert (0, "M_playdoh_struct_align: illegal machine model");
      return (0);
    }
}

int
M_playdoh_struct_size (int n, _M_Type * type, int struct_align)
{
  if (M_playdoh_model == M_PLAYDOH_V1)
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
	  mod_word = offset % M_PLAYDOH_SIZE_INT;
	  if (type[i].type == M_TYPE_BIT_CHAR)
	    {
	      if ((mod_word + size) > M_PLAYDOH_SIZE_INT)
		offset += (M_PLAYDOH_SIZE_INT - mod_word);
	      if (M_PLAYDOH_ALIGN_CHAR > max_align)
		max_align = M_PLAYDOH_ALIGN_CHAR;
	    }
	  else if (type[i].type == M_TYPE_BIT_SHORT)
	    {
	      if ((mod_word + size) > M_PLAYDOH_SIZE_INT)
		offset += (M_PLAYDOH_SIZE_INT - mod_word);
	      if (M_PLAYDOH_ALIGN_SHORT > max_align)
		max_align = M_PLAYDOH_ALIGN_SHORT;
	    }
	  else if (type[i].type == M_TYPE_BIT_LONG)
	    {
	      if ((mod_word + size) > M_PLAYDOH_SIZE_INT)
		offset += (M_PLAYDOH_SIZE_INT - mod_word);
	      if (M_PLAYDOH_ALIGN_LONG > max_align)
		max_align = M_PLAYDOH_ALIGN_LONG;
	    }
	  mod = offset % align;	/* align to what the field */
	  if (mod != 0)		/* needs to start from */
	    offset += (align - mod);

	  offset += size;
	}
      /*
       * align to at least byte boundary.
       */
      if (max_align < M_PLAYDOH_ALIGN_CHAR)
	max_align = M_PLAYDOH_ALIGN_CHAR;
      /* enforce max. alignment */
      mod = offset % max_align;
      if (mod != 0)
	offset += (max_align - mod);
      return offset;
    }
  else
    {
      M_assert (0, "M_playdoh_struct_size: illegal machine model");
      return (0);
    }
}


int
M_playdoh_layout_fnvar (List param_list, char **base_macro, int *pcount,
		     int purpose)
{
  M_Param param;
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

  max_align = M_PLAYDOH_ALIGN_MAX;
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
	  if (int_rg < M_PLAYDOH_MAX_FNVAR_REG)
	    {
	      param->mode = M_THRU_REGISTER;
	      param->reg = (int_rg)++ + M_PLAYDOH_INT_BASE;
	    }
	  else
	    {
	      param->mode = M_THRU_MEMORY;
	      param->reg = -1;
	    }
	  break;

        case M_TYPE_LLONG:
          if(int_rg + 1 < M_PLAYDOH_MAX_FNVAR_REG) {
            param->mode = M_THRU_REGISTER;
            param->reg = (int_rg) + M_PLAYDOH_INT_BASE;
            int_rg += 2;
          }
          else {
            param->mode = M_THRU_MEMORY;
            param->reg = -1;
            if(int_rg < M_PLAYDOH_MAX_FNVAR_REG)
              int_rg++;
          }
          break;

	case M_TYPE_FLOAT:
	  if (int_rg < M_PLAYDOH_MAX_FNVAR_REG)
	    {
	      param->mode = M_THRU_REGISTER;
	      param->reg = (fp_rg) + 1 + M_PLAYDOH_FLT_BASE;
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
	  if (int_rg < M_PLAYDOH_MAX_FNVAR_REG)
	    {
	      param->mode = M_THRU_REGISTER;
	      param->reg = fp_rg + 1 + M_PLAYDOH_FLT_BASE;
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
	  if (int_rg < M_PLAYDOH_MAX_FNVAR_REG)
	    {
	      param->mode = M_INDIRECT_THRU_REGISTER;
	      param->reg = (int_rg)++ + M_PLAYDOH_INT_BASE;
	    }
	  else
	    {
	      if (size <= M_PLAYDOH_SMALL_STRUCT_MAX)
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
	      size = M_PLAYDOH_SIZE_INT;
	      align = M_PLAYDOH_ALIGN_INT;
	    }
	}
      if (align >= M_PLAYDOH_SMALL_STRUCT_MAX &&
	  param->mtype.type != M_TYPE_DOUBLE)
	/* anything larger than a 64-bit structure is passed */
	/* indirectly thru memory                            */
	align = M_PLAYDOH_ALIGN_INT;
      else if (align < M_PLAYDOH_ALIGN_INT)
	/* anything smaller that 32-bits is passed as 32-bits */
	align = M_PLAYDOH_ALIGN_INT;

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
	  (size <= M_PLAYDOH_SMALL_STRUCT_MAX))
	{

	  /* must align to a double boundry */
	  align = M_PLAYDOH_ALIGN_MAX;

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
	  (size > M_PLAYDOH_SMALL_STRUCT_MAX))
	{

	  /* must align to a double boundry */
	  align = M_PLAYDOH_ALIGN_MAX;

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

/*--------------------------------------------------------------------------*/
int
M_playdoh_fnvar_layout (int n, _M_Type * type, long int *offset, int *mode,
			int *reg, int *paddr, char **base_macro,
			int *su_sreg, int *su_ereg,
			int *pcount, int is_st, int purpose)
					/* need to return structure */
{
  if (M_playdoh_model == M_PLAYDOH_V1)
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

      max_align = M_PLAYDOH_ALIGN_MAX;
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
	      if (int_rg < M_PLAYDOH_MAX_FNVAR_REG)
		{
		  mode[i] = M_THRU_REGISTER;
		  reg[i] = (int_rg)++ + M_PLAYDOH_INT_BASE;
		}
	      else
		{
		  mode[i] = M_THRU_MEMORY;
		  reg[i] = -1;
		}
	      break;

            case M_TYPE_LLONG:
              if(int_rg + 1 < M_PLAYDOH_MAX_FNVAR_REG) {
                mode[i] = M_THRU_REGISTER;
                reg[i] = (int_rg) + M_PLAYDOH_INT_BASE;
                int_rg += 2;
              }
              else {
                mode[i] = M_THRU_MEMORY;
                reg[i] = -1;
                if(int_rg < M_PLAYDOH_MAX_FNVAR_REG)
                  int_rg++;
              }
              break;
	    case M_TYPE_FLOAT:
	      if (int_rg < M_PLAYDOH_MAX_FNVAR_REG)
		{
		  mode[i] = M_THRU_REGISTER;
		  reg[i] = (fp_rg) + 1 + M_PLAYDOH_FLT_BASE;
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
	      if (int_rg < M_PLAYDOH_MAX_FNVAR_REG)
		{
		  mode[i] = M_THRU_REGISTER;
		  reg[i] = fp_rg + 1 + M_PLAYDOH_FLT_BASE;
		  int_rg += 2;
		  fp_rg  += 1;
#if 0
		  if (rg == 0 || rg == 2)
		    {
		      mode[i] = M_THRU_REGISTER;
		      reg[i] = rg + 1 + M_PLAYDOH_FLT_BASE;
		      rg += 2;
		    }
		  else if (rg == 1)
		    {
		      mode[i] = M_THRU_REGISTER;
		      reg[i] = 3 + M_PLAYDOH_FLT_BASE;
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
	      if (int_rg < M_PLAYDOH_MAX_FNVAR_REG)
		{
		  mode[i] = M_INDIRECT_THRU_REGISTER;
		  reg[i] = (int_rg)++ + M_PLAYDOH_INT_BASE;
		}
	      else
		{
		  if (size <= M_PLAYDOH_SMALL_STRUCT_MAX)
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
	      if (size <= M_PLAYDOH_SIZE_INT)
		{
		  if (rg < M_PLAYDOH_MAX_FNVAR_REG)
		    {
		      mode[i] = M_THRU_REGISTER;
		      reg[i] = (rg)++ + M_PLAYDOH_INT_BASE;
		    }
		  else
		    {
		      mode[i] = M_THRU_MEMORY;
		      reg[i] = -1;
		    }
		}
	      else if (size <= M_PLAYDOH_SMALL_STRUCT_MAX)
		{
		  if ((rg == 0) || (rg == 2))
		    {
		      mode[i] = M_THRU_REGISTER;
		      reg[i] = rg + M_PLAYDOH_INT_BASE;
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
		  if (rg < M_PLAYDOH_MAX_FNVAR_REG)
		    {
		      mode[i] = M_INDIRECT_THRU_REGISTER;
		      reg[i] = (rg)++ + M_PLAYDOH_INT_BASE;
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
		  size = M_PLAYDOH_SIZE_INT;
		  align = M_PLAYDOH_ALIGN_INT;
		}
	    }
	  if (align >= M_PLAYDOH_SMALL_STRUCT_MAX &&
	      type[i].type != M_TYPE_DOUBLE)
	    /* anything larger than a 64-bit structure is passed */
	    /* indirectly thru memory                            */
	    align = M_PLAYDOH_ALIGN_INT;
	  else if (align < M_PLAYDOH_ALIGN_INT)
	    /* anything smaller that 32-bits is passed as 32-bits */
	    align = M_PLAYDOH_ALIGN_INT;

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
	      (size <= M_PLAYDOH_SMALL_STRUCT_MAX))
	    {

	      /* must align to a double boundry */
	      align = M_PLAYDOH_ALIGN_MAX;

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
	      (size > M_PLAYDOH_SMALL_STRUCT_MAX))
	    {

	      /* must align to a double boundry */
	      align = M_PLAYDOH_ALIGN_MAX;

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
  else
    {
      M_assert (0, "M_playdoh_fnvar_layout: illegal machine model");
      return (0);
    }
}

/*--------------------------------------------------------------------------*/
int
M_playdoh_lvar_layout (int n, _M_Type * type, long int *offset,
		       char **base_macro)
{
  if (M_playdoh_model == M_PLAYDOH_V1)
    {
      int i, max_align, off;
      int size, align, mod, tp;
      /*
       *      the LOCAL section must be max. aligned initially
       */
      max_align = M_PLAYDOH_ALIGN_MAX;
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
       *      Local variables are relative to $LV
       */
      *base_macro = "$LV";
      return off;		/* the total size needed */
    }
  else
    {
      M_assert (0, "M_playdoh_lvar_layout: illegal machine model");
      return (0);
    }
}

/*--------------------------------------------------------------------------*/
int
M_playdoh_no_short_int (void)
{
  if (M_playdoh_model == M_PLAYDOH_V1)
    {
      return (M_PLAYDOH_SIZE_SHORT == M_PLAYDOH_SIZE_INT);
    }
  else
    {
      M_assert (0, "M_playdoh_no_short_int: illegal machine model");
      return (0);
    }
}
/*--------------------------------------------------------------------------*/
void
M_playdoh_cb_label_name (char *fn, int cb, char *line, int len)
{
  if (M_playdoh_model == M_PLAYDOH_V1)
    {
      sprintf (line, "cb%d%s", cb, fn);
    }
  else
    {
      M_assert (0, "M_playdoh_cb_label_name: illegal machine model");
    }
}
/*--------------------------------------------------------------------------*/
int
M_playdoh_is_cb_label (char *label, char *fn, int *cb)
{
  if (M_playdoh_model == M_PLAYDOH_V1)
    {
      return (sscanf (label, "cb%d%s", cb, fn) == 2);
    }
  else
    {
      M_assert (0, "M_playdoh_is_cb_label: illegal machine model");
      return (0);
    }
}
/*--------------------------------------------------------------------------*/
void
M_playdoh_jumptbl_label_name (char *fn, int tbl_id, char *line, int len)
{
  if (M_playdoh_model == M_PLAYDOH_V1)
    {
      sprintf (line, "_$%s%s%d", fn, M_JUMPTBL_BASE_NAME, tbl_id);
    }
  else
    {
      M_assert (0, "M_playdoh_jumptbl_label_name: illegal machine model");
    }
}
/*--------------------------------------------------------------------------*/
int
M_playdoh_is_jumptbl_label (char *label, char *fn, int *tbl_id)
{
  if (M_playdoh_model == M_PLAYDOH_V1)
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
  else
    {
      M_assert (0, "M_playdoh_is_jumptbl_label: illegal machine model");
      return (0);
    }
}
/*--------------------------------------------------------------------------*/
int
M_playdoh_structure_pointer (int purpose)
{
  if (M_playdoh_model == M_PLAYDOH_V1)
    {
      return M_PLAYDOH_RET_ST;
    }
  else
    {
      M_assert (0, "M_playdoh_structure_pointer: illegal machine model");
      return (0);
    }
}
/*--------------------------------------------------------------------------*/
int
M_playdoh_return_register (int type, int purpose)
{
  if (M_playdoh_model == M_PLAYDOH_V1)
    {
      switch (type)
	{
	case M_TYPE_INT:
	  return M_PLAYDOH_RET_I32;
	case M_TYPE_LONG:
	  return M_PLAYDOH_RET_I32;
	case M_TYPE_LLONG:
	  return M_PLAYDOH_RET_I64;
	case M_TYPE_FLOAT:
	  return M_PLAYDOH_RET_F;
	case M_TYPE_DOUBLE:
	  return M_PLAYDOH_RET_F;
	default:
	  return M_PLAYDOH_RET_I32;
	}
    }
  else
    {
      M_assert (0, "M_playdoh_return_register: illegal machine model");
      return (0);
    }
}
/*--------------------------------------------------------------------------*/
char *
M_playdoh_fn_label_name (char *label, int (*is_func) (char *is_func_label))
{
  if (M_playdoh_model == M_PLAYDOH_V1)
    {
      static char fn_label[1024];

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
      M_assert (0, "M_playdoh_fn_label_name: illegal machine model");
      return (0);
    }
}

char *
M_playdoh_fn_name_from_label (char *label)
{
  if (M_playdoh_model == M_PLAYDOH_V1)
    {
      if (!strncmp (label, "_$fn", 4))
	return (label + 4);
      else
	return (label);
    }
  else
    {
      M_assert (0, "M_playdoh_fn_name_from_label: illegal machine model");
      return (0);
    }
}

/*--------------------------------------------------------------------------*/
int
M_playdoh_fragile_macro (int macro_value)
{
  if (M_playdoh_model == M_PLAYDOH_V1)
    {
      switch (macro_value)
	{
	case L_MAC_P12:
	case L_MAC_SP:
	case L_MAC_FP:
	case L_MAC_LV:
	case L_MAC_IP:
	case L_MAC_OP:
	case L_MAC_LOCAL_SIZE:
	case L_MAC_PARAM_SIZE:
	case L_MAC_SWAP_SIZE:
	  return 0;
	default:
	  return (1);
	}
    }
  else
    {
      M_assert (0, "M_playdoh_fragile_macro: illegal machine model");
      return (0);
    }
}


Set M_playdoh_fragile_macro_set ()
{
  if (M_playdoh_model == M_PLAYDOH_V1)
    {
      if (Set_playdoh_fragile_macro)
	{
	  return Set_playdoh_fragile_macro;
	}
      Set_playdoh_fragile_macro = Set_add (Set_playdoh_fragile_macro, L_MAC_P0);
      Set_playdoh_fragile_macro = Set_add (Set_playdoh_fragile_macro, L_MAC_P1);
      Set_playdoh_fragile_macro = Set_add (Set_playdoh_fragile_macro, L_MAC_P2);
      Set_playdoh_fragile_macro = Set_add (Set_playdoh_fragile_macro, L_MAC_P3);
      Set_playdoh_fragile_macro = Set_add (Set_playdoh_fragile_macro, L_MAC_P4);
      Set_playdoh_fragile_macro = Set_add (Set_playdoh_fragile_macro, L_MAC_P5);
      Set_playdoh_fragile_macro = Set_add (Set_playdoh_fragile_macro, L_MAC_P6);
      Set_playdoh_fragile_macro = Set_add (Set_playdoh_fragile_macro, L_MAC_P7);
      Set_playdoh_fragile_macro = Set_add (Set_playdoh_fragile_macro, L_MAC_P8);
      Set_playdoh_fragile_macro = Set_add (Set_playdoh_fragile_macro, L_MAC_P9);
      Set_playdoh_fragile_macro =
	Set_add (Set_playdoh_fragile_macro, L_MAC_P10);
      Set_playdoh_fragile_macro =
	Set_add (Set_playdoh_fragile_macro, L_MAC_P11);
      Set_playdoh_fragile_macro =
	Set_add (Set_playdoh_fragile_macro, L_MAC_P13);
      Set_playdoh_fragile_macro =
	Set_add (Set_playdoh_fragile_macro, L_MAC_P14);
      Set_playdoh_fragile_macro =
	Set_add (Set_playdoh_fragile_macro, L_MAC_P15);
      Set_playdoh_fragile_macro =
	Set_add (Set_playdoh_fragile_macro, L_MAC_P16);
      Set_playdoh_fragile_macro =
	Set_add (Set_playdoh_fragile_macro, L_MAC_P17);
      Set_playdoh_fragile_macro =
	Set_add (Set_playdoh_fragile_macro, L_MAC_P18);
      Set_playdoh_fragile_macro =
	Set_add (Set_playdoh_fragile_macro, L_MAC_P19);
      Set_playdoh_fragile_macro =
	Set_add (Set_playdoh_fragile_macro, L_MAC_P20);
      Set_playdoh_fragile_macro =
	Set_add (Set_playdoh_fragile_macro, L_MAC_P21);
      Set_playdoh_fragile_macro =
	Set_add (Set_playdoh_fragile_macro, L_MAC_P22);
      Set_playdoh_fragile_macro =
	Set_add (Set_playdoh_fragile_macro, L_MAC_P23);
      Set_playdoh_fragile_macro =
	Set_add (Set_playdoh_fragile_macro, L_MAC_P24);
      Set_playdoh_fragile_macro =
	Set_add (Set_playdoh_fragile_macro, L_MAC_P25);
      Set_playdoh_fragile_macro =
	Set_add (Set_playdoh_fragile_macro, L_MAC_P26);
      Set_playdoh_fragile_macro =
	Set_add (Set_playdoh_fragile_macro, L_MAC_P27);
      Set_playdoh_fragile_macro =
	Set_add (Set_playdoh_fragile_macro, L_MAC_P28);
      Set_playdoh_fragile_macro =
	Set_add (Set_playdoh_fragile_macro, L_MAC_P29);
      Set_playdoh_fragile_macro =
	Set_add (Set_playdoh_fragile_macro, L_MAC_P30);
      Set_playdoh_fragile_macro =
	Set_add (Set_playdoh_fragile_macro, L_MAC_P31);
      Set_playdoh_fragile_macro =
	Set_add (Set_playdoh_fragile_macro, L_MAC_P32);
      Set_playdoh_fragile_macro =
	Set_add (Set_playdoh_fragile_macro, L_MAC_P33);
      Set_playdoh_fragile_macro =
	Set_add (Set_playdoh_fragile_macro, L_MAC_P34);
      Set_playdoh_fragile_macro =
	Set_add (Set_playdoh_fragile_macro, L_MAC_P35);
      Set_playdoh_fragile_macro =
	Set_add (Set_playdoh_fragile_macro, L_MAC_P36);
      Set_playdoh_fragile_macro =
	Set_add (Set_playdoh_fragile_macro, L_MAC_P37);
      Set_playdoh_fragile_macro =
	Set_add (Set_playdoh_fragile_macro, L_MAC_P38);
      Set_playdoh_fragile_macro =
	Set_add (Set_playdoh_fragile_macro, L_MAC_P39);
      Set_playdoh_fragile_macro =
	Set_add (Set_playdoh_fragile_macro, L_MAC_P40);
      Set_playdoh_fragile_macro =
	Set_add (Set_playdoh_fragile_macro, L_MAC_P41);
      Set_playdoh_fragile_macro =
	Set_add (Set_playdoh_fragile_macro, L_MAC_P42);
      Set_playdoh_fragile_macro =
	Set_add (Set_playdoh_fragile_macro, L_MAC_P43);
      Set_playdoh_fragile_macro =
	Set_add (Set_playdoh_fragile_macro, L_MAC_P44);
      Set_playdoh_fragile_macro =
	Set_add (Set_playdoh_fragile_macro, L_MAC_P45);
      Set_playdoh_fragile_macro =
	Set_add (Set_playdoh_fragile_macro, L_MAC_P46);
      Set_playdoh_fragile_macro =
	Set_add (Set_playdoh_fragile_macro, L_MAC_P47);
      Set_playdoh_fragile_macro =
	Set_add (Set_playdoh_fragile_macro, L_MAC_P48);
      Set_playdoh_fragile_macro =
	Set_add (Set_playdoh_fragile_macro, L_MAC_P49);
      Set_playdoh_fragile_macro =
	Set_add (Set_playdoh_fragile_macro, L_MAC_P50);
      Set_playdoh_fragile_macro =
	Set_add (Set_playdoh_fragile_macro, L_MAC_P51);
      Set_playdoh_fragile_macro =
	Set_add (Set_playdoh_fragile_macro, L_MAC_P52);
      Set_playdoh_fragile_macro =
	Set_add (Set_playdoh_fragile_macro, L_MAC_P53);
      Set_playdoh_fragile_macro =
	Set_add (Set_playdoh_fragile_macro, L_MAC_P54);
      Set_playdoh_fragile_macro =
	Set_add (Set_playdoh_fragile_macro, L_MAC_P55);
      Set_playdoh_fragile_macro =
	Set_add (Set_playdoh_fragile_macro, L_MAC_P56);
      Set_playdoh_fragile_macro =
	Set_add (Set_playdoh_fragile_macro, L_MAC_P57);
      Set_playdoh_fragile_macro =
	Set_add (Set_playdoh_fragile_macro, L_MAC_P58);
      Set_playdoh_fragile_macro =
	Set_add (Set_playdoh_fragile_macro, L_MAC_P59);
      Set_playdoh_fragile_macro =
	Set_add (Set_playdoh_fragile_macro, L_MAC_P60);
      Set_playdoh_fragile_macro =
	Set_add (Set_playdoh_fragile_macro, L_MAC_P61);
      Set_playdoh_fragile_macro =
	Set_add (Set_playdoh_fragile_macro, L_MAC_P62);
      Set_playdoh_fragile_macro =
	Set_add (Set_playdoh_fragile_macro, L_MAC_P63);
      Set_playdoh_fragile_macro =
	Set_add (Set_playdoh_fragile_macro, L_MAC_P64);
      Set_playdoh_fragile_macro = Set_add (Set_playdoh_fragile_macro, L_MAC_RS);
      Set_playdoh_fragile_macro = Set_add (Set_playdoh_fragile_macro,
					  L_MAC_RET_TYPE);
      Set_playdoh_fragile_macro = Set_add (Set_playdoh_fragile_macro,
					  L_MAC_TR_PTR);
      Set_playdoh_fragile_macro = Set_add (Set_playdoh_fragile_macro,
					  L_MAC_TR_MARK);
      Set_playdoh_fragile_macro = Set_add (Set_playdoh_fragile_macro,
					  L_MAC_TR_TEMP);
      Set_playdoh_fragile_macro = Set_add (Set_playdoh_fragile_macro,
					  L_MAC_PRED_ALL);
      Set_playdoh_fragile_macro = Set_add (Set_playdoh_fragile_macro,
					  L_MAC_SAFE_MEM);
      Set_playdoh_fragile_macro = Set_add (Set_playdoh_fragile_macro,
					  L_MAC_TM_TYPE);
      return Set_playdoh_fragile_macro;
    }
  else
    {
      M_assert (0, "M_playdoh_fragile_macro_set: illegal machine model");
      return (0);
    }
}

int
M_playdoh_dataflow_macro (int id)
{
  if (M_playdoh_model == M_PLAYDOH_V1)
    {
      return ((id >= L_MAC_P0 && id <= L_MAC_P64) || (id == L_MAC_SP) ||
	      (id == L_MAC_RETADDR) || (id >= L_MAC_LAST));
    }
  else
    {
      M_assert (0, "M_playdoh_dataflow_macro: illegal machine model");
      return (0);
    }
}

/*--------------------------------------------------------------------------*/
int
M_playdoh_subroutine_call (int opc)
{
  if (M_playdoh_model == M_PLAYDOH_V1)
    {
      return ((opc == Lop_JSR) || (opc == Lop_JSR_FS));
    }
  else
    {
      M_assert (0, "M_playdoh_subroutine_call:  Illegal model specified!");
      return (0);
    }
}
/*--------------------------------------------------------------------------*/
/*
 * Declare code generator specific macro registers to the front end parser.
 */
void
M_define_macros_playdoh (STRING_Symbol_Table * sym_tbl)
{
  if (M_playdoh_model == M_PLAYDOH_V1)
    {
      M_add_symbol (sym_tbl, "fr0", PLAYDOH_MAC_FZERO);
      M_add_symbol (sym_tbl, "fr1", PLAYDOH_MAC_FONE);
      M_add_symbol (sym_tbl, "gr0", PLAYDOH_MAC_ZERO);
      M_add_symbol (sym_tbl, "true_sp", PLAYDOH_MAC_TRUE_SP);
      M_add_symbol (sym_tbl, "pr0", PLAYDOH_MAC_PRED_FALSE);
      M_add_symbol (sym_tbl, "pr1", PLAYDOH_MAC_PRED_TRUE);
      M_add_symbol (sym_tbl, "gr1", PLAYDOH_MAC_TEMPREG);
      M_add_symbol (sym_tbl, "gr2", PLAYDOH_MAC_RETADDR);
      M_add_symbol (sym_tbl, "lc", PLAYDOH_MAC_LC);
      M_add_symbol (sym_tbl, "esc", PLAYDOH_MAC_ESC);
      M_add_symbol (sym_tbl, "rrb", PLAYDOH_MAC_RRB);
      M_add_symbol (sym_tbl, "$pred_all_rot", PLAYDOH_MAC_PRED_ALL_ROT);
      M_add_symbol (sym_tbl, "$pred_all_static", PLAYDOH_MAC_PRED_ALL_STATIC);
      M_add_symbol (sym_tbl, "pv_0", PLAYDOH_MAC_PV_0);
      M_add_symbol (sym_tbl, "pv_1", PLAYDOH_MAC_PV_1);
      M_add_symbol (sym_tbl, "pv_2", PLAYDOH_MAC_PV_2);
      M_add_symbol (sym_tbl, "pv_3", PLAYDOH_MAC_PV_3);
      M_add_symbol (sym_tbl, "pv_4", PLAYDOH_MAC_PV_4);
      M_add_symbol (sym_tbl, "pv_5", PLAYDOH_MAC_PV_5);
      M_add_symbol (sym_tbl, "pv_6", PLAYDOH_MAC_PV_6);
      M_add_symbol (sym_tbl, "pv_7", PLAYDOH_MAC_PV_7);
    }
  else
    {
      M_assert (0, "M_define_macros_playdoh: illegal machine model");
    }
}

char *
M_get_macro_name_playdoh (int id)
{
  if (M_playdoh_model == M_PLAYDOH_V1)
    {
      switch (id)
	{
	case PLAYDOH_MAC_ZERO:
	  return "gr0";
	case PLAYDOH_MAC_TEMPREG:
	  return "gr1";
	case PLAYDOH_MAC_RETADDR:
	  return "gr2";
	case PLAYDOH_MAC_FZERO:
	  return "fr0";
	case PLAYDOH_MAC_TRUE_SP:
	  return "true_sp";
	case PLAYDOH_MAC_FONE:
	  return "fr1";
	case PLAYDOH_MAC_PRED_FALSE:
	  return "pr0";
	case PLAYDOH_MAC_PRED_TRUE:
	  return "pr1";
	case PLAYDOH_MAC_LC:
	  return "lc";
	case PLAYDOH_MAC_ESC:
	  return "esc";
	case PLAYDOH_MAC_RRB:
	  return "rrb";
	case PLAYDOH_MAC_PRED_ALL_ROT:
	  return "$pred_all_rot";
	case PLAYDOH_MAC_PRED_ALL_STATIC:
	  return "$pred_all_static";
	case PLAYDOH_MAC_PV_0:
	  return "pv_0";
	case PLAYDOH_MAC_PV_1:
	  return "pv_1";
	case PLAYDOH_MAC_PV_2:
	  return "pv_2";
	case PLAYDOH_MAC_PV_3:
	  return "pv_3";
	case PLAYDOH_MAC_PV_4:
	  return "pv_4";
	case PLAYDOH_MAC_PV_5:
	  return "pv_5";
	case PLAYDOH_MAC_PV_6:
	  return "pv_6";
	case PLAYDOH_MAC_PV_7:
	  return "pv_7";
	default:
	  return "?";
	}
    }
  else
    {
      M_assert (0, "M_get_macro_name_playdoh: illegal machine model");
      return (0);
    }
}

void
M_define_opcode_name_playdoh (STRING_Symbol_Table * sym_tbl)
{

  M_add_symbol (sym_tbl, PLAYDOHopcode_PBRR, PLAYDOHop_PBRR);
  M_add_symbol (sym_tbl, PLAYDOHopcode_PBRA, PLAYDOHop_PBRA);

#if 0
  M_add_symbol (sym_tbl, PLAYDOHopcode_EXTRSB, PLAYDOHop_EXTRSB);
  M_add_symbol (sym_tbl, PLAYDOHopcode_EXTRSH, PLAYDOHop_EXTRSH);
#endif

  M_add_symbol (sym_tbl, PLAYDOHopcode_MOVEGF_L, PLAYDOHop_MOVEGF_L);
  M_add_symbol (sym_tbl, PLAYDOHopcode_MOVEGF_U, PLAYDOHop_MOVEGF_U);
  M_add_symbol (sym_tbl, PLAYDOHopcode_MOVEFG_L, PLAYDOHop_MOVEFG_L);
  M_add_symbol (sym_tbl, PLAYDOHopcode_MOVEFG_U, PLAYDOHop_MOVEFG_U);
  M_add_symbol (sym_tbl, PLAYDOHopcode_MOVEPG, PLAYDOHop_MOVEPG);
  M_add_symbol (sym_tbl, PLAYDOHopcode_LDCM, PLAYDOHop_LDCM);

  M_add_symbol (sym_tbl, PLAYDOHopcode_BRU, PLAYDOHop_BRU);
  M_add_symbol (sym_tbl, PLAYDOHopcode_BRCT, PLAYDOHop_BRCT);
  M_add_symbol (sym_tbl, PLAYDOHopcode_BRCF, PLAYDOHop_BRCF);
  M_add_symbol (sym_tbl, PLAYDOHopcode_BRL, PLAYDOHop_BRL);
  M_add_symbol (sym_tbl, PLAYDOHopcode_BRLC, PLAYDOHop_BRLC);

  M_add_symbol (sym_tbl, PLAYDOHopcode_BRF_B_B_B, PLAYDOHop_BRF_B_B_B);
  M_add_symbol (sym_tbl, PLAYDOHopcode_BRF_B_B_F, PLAYDOHop_BRF_B_B_F);
  M_add_symbol (sym_tbl, PLAYDOHopcode_BRF_B_F_B, PLAYDOHop_BRF_B_F_B);
  M_add_symbol (sym_tbl, PLAYDOHopcode_BRF_B_F_F, PLAYDOHop_BRF_B_F_F);
  M_add_symbol (sym_tbl, PLAYDOHopcode_BRF_F_B_B, PLAYDOHop_BRF_F_B_B);
  M_add_symbol (sym_tbl, PLAYDOHopcode_BRF_F_B_F, PLAYDOHop_BRF_F_B_F);
  M_add_symbol (sym_tbl, PLAYDOHopcode_BRF_F_F_B, PLAYDOHop_BRF_F_F_B);
  M_add_symbol (sym_tbl, PLAYDOHopcode_BRF_F_F_F, PLAYDOHop_BRF_F_F_F);

  M_add_symbol (sym_tbl, PLAYDOHopcode_BRW_B_B_B, PLAYDOHop_BRW_B_B_B);
  M_add_symbol (sym_tbl, PLAYDOHopcode_BRW_B_B_F, PLAYDOHop_BRW_B_B_F);
  M_add_symbol (sym_tbl, PLAYDOHopcode_BRW_B_F_B, PLAYDOHop_BRW_B_F_B);
  M_add_symbol (sym_tbl, PLAYDOHopcode_BRW_B_F_F, PLAYDOHop_BRW_B_F_F);
  M_add_symbol (sym_tbl, PLAYDOHopcode_BRW_F_B_B, PLAYDOHop_BRW_F_B_B);
  M_add_symbol (sym_tbl, PLAYDOHopcode_BRW_F_B_F, PLAYDOHop_BRW_F_B_F);
  M_add_symbol (sym_tbl, PLAYDOHopcode_BRW_F_F_B, PLAYDOHop_BRW_F_F_B);
  M_add_symbol (sym_tbl, PLAYDOHopcode_BRW_F_F_F, PLAYDOHop_BRW_F_F_F);

  M_add_symbol (sym_tbl, PLAYDOHopcode_BRDVI, PLAYDOHop_BRDVI);
  M_add_symbol (sym_tbl, PLAYDOHopcode_BRDVF, PLAYDOHop_BRDVF);

  M_add_symbol (sym_tbl, PLAYDOHopcode_L_B_V1_V1, PLAYDOHop_L_B_V1_V1);
  M_add_symbol (sym_tbl, PLAYDOHopcode_L_B_V1_C1, PLAYDOHop_L_B_V1_C1);
  M_add_symbol (sym_tbl, PLAYDOHopcode_L_B_V1_C2, PLAYDOHop_L_B_V1_C2);
  M_add_symbol (sym_tbl, PLAYDOHopcode_L_B_V1_C3, PLAYDOHop_L_B_V1_C3);
  M_add_symbol (sym_tbl, PLAYDOHopcode_L_B_C1_V1, PLAYDOHop_L_B_C1_V1);
  M_add_symbol (sym_tbl, PLAYDOHopcode_L_B_C1_C1, PLAYDOHop_L_B_C1_C1);
  M_add_symbol (sym_tbl, PLAYDOHopcode_L_B_C1_C2, PLAYDOHop_L_B_C1_C2);
  M_add_symbol (sym_tbl, PLAYDOHopcode_L_B_C1_C3, PLAYDOHop_L_B_C1_C3);
  M_add_symbol (sym_tbl, PLAYDOHopcode_L_B_C2_V1, PLAYDOHop_L_B_C2_V1);
  M_add_symbol (sym_tbl, PLAYDOHopcode_L_B_C2_C1, PLAYDOHop_L_B_C2_C1);
  M_add_symbol (sym_tbl, PLAYDOHopcode_L_B_C2_C2, PLAYDOHop_L_B_C2_C2);
  M_add_symbol (sym_tbl, PLAYDOHopcode_L_B_C2_C3, PLAYDOHop_L_B_C2_C3);
  M_add_symbol (sym_tbl, PLAYDOHopcode_L_B_C3_V1, PLAYDOHop_L_B_C3_V1);
  M_add_symbol (sym_tbl, PLAYDOHopcode_L_B_C3_C1, PLAYDOHop_L_B_C3_C1);
  M_add_symbol (sym_tbl, PLAYDOHopcode_L_B_C3_C2, PLAYDOHop_L_B_C3_C2);
  M_add_symbol (sym_tbl, PLAYDOHopcode_L_B_C3_C3, PLAYDOHop_L_B_C3_C3);

  M_add_symbol (sym_tbl, PLAYDOHopcode_L_H_V1_V1, PLAYDOHop_L_H_V1_V1);
  M_add_symbol (sym_tbl, PLAYDOHopcode_L_H_V1_C1, PLAYDOHop_L_H_V1_C1);
  M_add_symbol (sym_tbl, PLAYDOHopcode_L_H_V1_C2, PLAYDOHop_L_H_V1_C2);
  M_add_symbol (sym_tbl, PLAYDOHopcode_L_H_V1_C3, PLAYDOHop_L_H_V1_C3);
  M_add_symbol (sym_tbl, PLAYDOHopcode_L_H_C1_V1, PLAYDOHop_L_H_C1_V1);
  M_add_symbol (sym_tbl, PLAYDOHopcode_L_H_C1_C1, PLAYDOHop_L_H_C1_C1);
  M_add_symbol (sym_tbl, PLAYDOHopcode_L_H_C1_C2, PLAYDOHop_L_H_C1_C2);
  M_add_symbol (sym_tbl, PLAYDOHopcode_L_H_C1_C3, PLAYDOHop_L_H_C1_C3);
  M_add_symbol (sym_tbl, PLAYDOHopcode_L_H_C2_V1, PLAYDOHop_L_H_C2_V1);
  M_add_symbol (sym_tbl, PLAYDOHopcode_L_H_C2_C1, PLAYDOHop_L_H_C2_C1);
  M_add_symbol (sym_tbl, PLAYDOHopcode_L_H_C2_C2, PLAYDOHop_L_H_C2_C2);
  M_add_symbol (sym_tbl, PLAYDOHopcode_L_H_C2_C3, PLAYDOHop_L_H_C2_C3);
  M_add_symbol (sym_tbl, PLAYDOHopcode_L_H_C3_V1, PLAYDOHop_L_H_C3_V1);
  M_add_symbol (sym_tbl, PLAYDOHopcode_L_H_C3_C1, PLAYDOHop_L_H_C3_C1);
  M_add_symbol (sym_tbl, PLAYDOHopcode_L_H_C3_C2, PLAYDOHop_L_H_C3_C2);
  M_add_symbol (sym_tbl, PLAYDOHopcode_L_H_C3_C3, PLAYDOHop_L_H_C3_C3);

  M_add_symbol (sym_tbl, PLAYDOHopcode_L_W_V1_V1, PLAYDOHop_L_W_V1_V1);
  M_add_symbol (sym_tbl, PLAYDOHopcode_L_W_V1_C1, PLAYDOHop_L_W_V1_C1);
  M_add_symbol (sym_tbl, PLAYDOHopcode_L_W_V1_C2, PLAYDOHop_L_W_V1_C2);
  M_add_symbol (sym_tbl, PLAYDOHopcode_L_W_V1_C3, PLAYDOHop_L_W_V1_C3);
  M_add_symbol (sym_tbl, PLAYDOHopcode_L_W_C1_V1, PLAYDOHop_L_W_C1_V1);
  M_add_symbol (sym_tbl, PLAYDOHopcode_L_W_C1_C1, PLAYDOHop_L_W_C1_C1);
  M_add_symbol (sym_tbl, PLAYDOHopcode_L_W_C1_C2, PLAYDOHop_L_W_C1_C2);
  M_add_symbol (sym_tbl, PLAYDOHopcode_L_W_C1_C3, PLAYDOHop_L_W_C1_C3);
  M_add_symbol (sym_tbl, PLAYDOHopcode_L_W_C2_V1, PLAYDOHop_L_W_C2_V1);
  M_add_symbol (sym_tbl, PLAYDOHopcode_L_W_C2_C1, PLAYDOHop_L_W_C2_C1);
  M_add_symbol (sym_tbl, PLAYDOHopcode_L_W_C2_C2, PLAYDOHop_L_W_C2_C2);
  M_add_symbol (sym_tbl, PLAYDOHopcode_L_W_C2_C3, PLAYDOHop_L_W_C2_C3);
  M_add_symbol (sym_tbl, PLAYDOHopcode_L_W_C3_V1, PLAYDOHop_L_W_C3_V1);
  M_add_symbol (sym_tbl, PLAYDOHopcode_L_W_C3_C1, PLAYDOHop_L_W_C3_C1);
  M_add_symbol (sym_tbl, PLAYDOHopcode_L_W_C3_C2, PLAYDOHop_L_W_C3_C2);
  M_add_symbol (sym_tbl, PLAYDOHopcode_L_W_C3_C3, PLAYDOHop_L_W_C3_C3);

  M_add_symbol (sym_tbl, PLAYDOHopcode_L_Q_V1_V1, PLAYDOHop_L_Q_V1_V1);
  M_add_symbol (sym_tbl, PLAYDOHopcode_L_Q_V1_C1, PLAYDOHop_L_Q_V1_C1);
  M_add_symbol (sym_tbl, PLAYDOHopcode_L_Q_V1_C2, PLAYDOHop_L_Q_V1_C2);
  M_add_symbol (sym_tbl, PLAYDOHopcode_L_Q_V1_C3, PLAYDOHop_L_Q_V1_C3);
  M_add_symbol (sym_tbl, PLAYDOHopcode_L_Q_C1_V1, PLAYDOHop_L_Q_C1_V1);
  M_add_symbol (sym_tbl, PLAYDOHopcode_L_Q_C1_C1, PLAYDOHop_L_Q_C1_C1);
  M_add_symbol (sym_tbl, PLAYDOHopcode_L_Q_C1_C2, PLAYDOHop_L_Q_C1_C2);
  M_add_symbol (sym_tbl, PLAYDOHopcode_L_Q_C1_C3, PLAYDOHop_L_Q_C1_C3);
  M_add_symbol (sym_tbl, PLAYDOHopcode_L_Q_C2_V1, PLAYDOHop_L_Q_C2_V1);
  M_add_symbol (sym_tbl, PLAYDOHopcode_L_Q_C2_C1, PLAYDOHop_L_Q_C2_C1);
  M_add_symbol (sym_tbl, PLAYDOHopcode_L_Q_C2_C2, PLAYDOHop_L_Q_C2_C2);
  M_add_symbol (sym_tbl, PLAYDOHopcode_L_Q_C2_C3, PLAYDOHop_L_Q_C2_C3);
  M_add_symbol (sym_tbl, PLAYDOHopcode_L_Q_C3_V1, PLAYDOHop_L_Q_C3_V1);
  M_add_symbol (sym_tbl, PLAYDOHopcode_L_Q_C3_C1, PLAYDOHop_L_Q_C3_C1);
  M_add_symbol (sym_tbl, PLAYDOHopcode_L_Q_C3_C2, PLAYDOHop_L_Q_C3_C2);
  M_add_symbol (sym_tbl, PLAYDOHopcode_L_Q_C3_C3, PLAYDOHop_L_Q_C3_C3);

  M_add_symbol (sym_tbl, PLAYDOHopcode_LI_B_V1_V1, PLAYDOHop_LI_B_V1_V1);
  M_add_symbol (sym_tbl, PLAYDOHopcode_LI_B_V1_C1, PLAYDOHop_LI_B_V1_C1);
  M_add_symbol (sym_tbl, PLAYDOHopcode_LI_B_V1_C2, PLAYDOHop_LI_B_V1_C2);
  M_add_symbol (sym_tbl, PLAYDOHopcode_LI_B_V1_C3, PLAYDOHop_LI_B_V1_C3);
  M_add_symbol (sym_tbl, PLAYDOHopcode_LI_B_C1_V1, PLAYDOHop_LI_B_C1_V1);
  M_add_symbol (sym_tbl, PLAYDOHopcode_LI_B_C1_C1, PLAYDOHop_LI_B_C1_C1);
  M_add_symbol (sym_tbl, PLAYDOHopcode_LI_B_C1_C2, PLAYDOHop_LI_B_C1_C2);
  M_add_symbol (sym_tbl, PLAYDOHopcode_LI_B_C1_C3, PLAYDOHop_LI_B_C1_C3);
  M_add_symbol (sym_tbl, PLAYDOHopcode_LI_B_C2_V1, PLAYDOHop_LI_B_C2_V1);
  M_add_symbol (sym_tbl, PLAYDOHopcode_LI_B_C2_C1, PLAYDOHop_LI_B_C2_C1);
  M_add_symbol (sym_tbl, PLAYDOHopcode_LI_B_C2_C2, PLAYDOHop_LI_B_C2_C2);
  M_add_symbol (sym_tbl, PLAYDOHopcode_LI_B_C2_C3, PLAYDOHop_LI_B_C2_C3);
  M_add_symbol (sym_tbl, PLAYDOHopcode_LI_B_C3_V1, PLAYDOHop_LI_B_C3_V1);
  M_add_symbol (sym_tbl, PLAYDOHopcode_LI_B_C3_C1, PLAYDOHop_LI_B_C3_C1);
  M_add_symbol (sym_tbl, PLAYDOHopcode_LI_B_C3_C2, PLAYDOHop_LI_B_C3_C2);
  M_add_symbol (sym_tbl, PLAYDOHopcode_LI_B_C3_C3, PLAYDOHop_LI_B_C3_C3);

  M_add_symbol (sym_tbl, PLAYDOHopcode_LI_H_V1_V1, PLAYDOHop_LI_H_V1_V1);
  M_add_symbol (sym_tbl, PLAYDOHopcode_LI_H_V1_C1, PLAYDOHop_LI_H_V1_C1);
  M_add_symbol (sym_tbl, PLAYDOHopcode_LI_H_V1_C2, PLAYDOHop_LI_H_V1_C2);
  M_add_symbol (sym_tbl, PLAYDOHopcode_LI_H_V1_C3, PLAYDOHop_LI_H_V1_C3);
  M_add_symbol (sym_tbl, PLAYDOHopcode_LI_H_C1_V1, PLAYDOHop_LI_H_C1_V1);
  M_add_symbol (sym_tbl, PLAYDOHopcode_LI_H_C1_C1, PLAYDOHop_LI_H_C1_C1);
  M_add_symbol (sym_tbl, PLAYDOHopcode_LI_H_C1_C2, PLAYDOHop_LI_H_C1_C2);
  M_add_symbol (sym_tbl, PLAYDOHopcode_LI_H_C1_C3, PLAYDOHop_LI_H_C1_C3);
  M_add_symbol (sym_tbl, PLAYDOHopcode_LI_H_C2_V1, PLAYDOHop_LI_H_C2_V1);
  M_add_symbol (sym_tbl, PLAYDOHopcode_LI_H_C2_C1, PLAYDOHop_LI_H_C2_C1);
  M_add_symbol (sym_tbl, PLAYDOHopcode_LI_H_C2_C2, PLAYDOHop_LI_H_C2_C2);
  M_add_symbol (sym_tbl, PLAYDOHopcode_LI_H_C2_C3, PLAYDOHop_LI_H_C2_C3);
  M_add_symbol (sym_tbl, PLAYDOHopcode_LI_H_C3_V1, PLAYDOHop_LI_H_C3_V1);
  M_add_symbol (sym_tbl, PLAYDOHopcode_LI_H_C3_C1, PLAYDOHop_LI_H_C3_C1);
  M_add_symbol (sym_tbl, PLAYDOHopcode_LI_H_C3_C2, PLAYDOHop_LI_H_C3_C2);
  M_add_symbol (sym_tbl, PLAYDOHopcode_LI_H_C3_C3, PLAYDOHop_LI_H_C3_C3);

  M_add_symbol (sym_tbl, PLAYDOHopcode_LI_W_V1_V1, PLAYDOHop_LI_W_V1_V1);
  M_add_symbol (sym_tbl, PLAYDOHopcode_LI_W_V1_C1, PLAYDOHop_LI_W_V1_C1);
  M_add_symbol (sym_tbl, PLAYDOHopcode_LI_W_V1_C2, PLAYDOHop_LI_W_V1_C2);
  M_add_symbol (sym_tbl, PLAYDOHopcode_LI_W_V1_C3, PLAYDOHop_LI_W_V1_C3);
  M_add_symbol (sym_tbl, PLAYDOHopcode_LI_W_C1_V1, PLAYDOHop_LI_W_C1_V1);
  M_add_symbol (sym_tbl, PLAYDOHopcode_LI_W_C1_C1, PLAYDOHop_LI_W_C1_C1);
  M_add_symbol (sym_tbl, PLAYDOHopcode_LI_W_C1_C2, PLAYDOHop_LI_W_C1_C2);
  M_add_symbol (sym_tbl, PLAYDOHopcode_LI_W_C1_C3, PLAYDOHop_LI_W_C1_C3);
  M_add_symbol (sym_tbl, PLAYDOHopcode_LI_W_C2_V1, PLAYDOHop_LI_W_C2_V1);
  M_add_symbol (sym_tbl, PLAYDOHopcode_LI_W_C2_C1, PLAYDOHop_LI_W_C2_C1);
  M_add_symbol (sym_tbl, PLAYDOHopcode_LI_W_C2_C2, PLAYDOHop_LI_W_C2_C2);
  M_add_symbol (sym_tbl, PLAYDOHopcode_LI_W_C2_C3, PLAYDOHop_LI_W_C2_C3);
  M_add_symbol (sym_tbl, PLAYDOHopcode_LI_W_C3_V1, PLAYDOHop_LI_W_C3_V1);
  M_add_symbol (sym_tbl, PLAYDOHopcode_LI_W_C3_C1, PLAYDOHop_LI_W_C3_C1);
  M_add_symbol (sym_tbl, PLAYDOHopcode_LI_W_C3_C2, PLAYDOHop_LI_W_C3_C2);
  M_add_symbol (sym_tbl, PLAYDOHopcode_LI_W_C3_C3, PLAYDOHop_LI_W_C3_C3);

  M_add_symbol (sym_tbl, PLAYDOHopcode_LI_Q_V1_V1, PLAYDOHop_LI_Q_V1_V1);
  M_add_symbol (sym_tbl, PLAYDOHopcode_LI_Q_V1_C1, PLAYDOHop_LI_Q_V1_C1);
  M_add_symbol (sym_tbl, PLAYDOHopcode_LI_Q_V1_C2, PLAYDOHop_LI_Q_V1_C2);
  M_add_symbol (sym_tbl, PLAYDOHopcode_LI_Q_V1_C3, PLAYDOHop_LI_Q_V1_C3);
  M_add_symbol (sym_tbl, PLAYDOHopcode_LI_Q_C1_V1, PLAYDOHop_LI_Q_C1_V1);
  M_add_symbol (sym_tbl, PLAYDOHopcode_LI_Q_C1_C1, PLAYDOHop_LI_Q_C1_C1);
  M_add_symbol (sym_tbl, PLAYDOHopcode_LI_Q_C1_C2, PLAYDOHop_LI_Q_C1_C2);
  M_add_symbol (sym_tbl, PLAYDOHopcode_LI_Q_C1_C3, PLAYDOHop_LI_Q_C1_C3);
  M_add_symbol (sym_tbl, PLAYDOHopcode_LI_Q_C2_V1, PLAYDOHop_LI_Q_C2_V1);
  M_add_symbol (sym_tbl, PLAYDOHopcode_LI_Q_C2_C1, PLAYDOHop_LI_Q_C2_C1);
  M_add_symbol (sym_tbl, PLAYDOHopcode_LI_Q_C2_C2, PLAYDOHop_LI_Q_C2_C2);
  M_add_symbol (sym_tbl, PLAYDOHopcode_LI_Q_C2_C3, PLAYDOHop_LI_Q_C2_C3);
  M_add_symbol (sym_tbl, PLAYDOHopcode_LI_Q_C3_V1, PLAYDOHop_LI_Q_C3_V1);
  M_add_symbol (sym_tbl, PLAYDOHopcode_LI_Q_C3_C1, PLAYDOHop_LI_Q_C3_C1);
  M_add_symbol (sym_tbl, PLAYDOHopcode_LI_Q_C3_C2, PLAYDOHop_LI_Q_C3_C2);
  M_add_symbol (sym_tbl, PLAYDOHopcode_LI_Q_C3_C3, PLAYDOHop_LI_Q_C3_C3);

  M_add_symbol (sym_tbl, PLAYDOHopcode_S_B_V1, PLAYDOHop_S_B_V1);
  M_add_symbol (sym_tbl, PLAYDOHopcode_S_B_C1, PLAYDOHop_S_B_C1);
  M_add_symbol (sym_tbl, PLAYDOHopcode_S_B_C2, PLAYDOHop_S_B_C2);
  M_add_symbol (sym_tbl, PLAYDOHopcode_S_B_C3, PLAYDOHop_S_B_C3);

  M_add_symbol (sym_tbl, PLAYDOHopcode_S_H_V1, PLAYDOHop_S_H_V1);
  M_add_symbol (sym_tbl, PLAYDOHopcode_S_H_C1, PLAYDOHop_S_H_C1);
  M_add_symbol (sym_tbl, PLAYDOHopcode_S_H_C2, PLAYDOHop_S_H_C2);
  M_add_symbol (sym_tbl, PLAYDOHopcode_S_H_C3, PLAYDOHop_S_H_C3);

  M_add_symbol (sym_tbl, PLAYDOHopcode_S_W_V1, PLAYDOHop_S_W_V1);
  M_add_symbol (sym_tbl, PLAYDOHopcode_S_W_C1, PLAYDOHop_S_W_C1);
  M_add_symbol (sym_tbl, PLAYDOHopcode_S_W_C2, PLAYDOHop_S_W_C2);
  M_add_symbol (sym_tbl, PLAYDOHopcode_S_W_C3, PLAYDOHop_S_W_C3);

  M_add_symbol (sym_tbl, PLAYDOHopcode_S_Q_V1, PLAYDOHop_S_Q_V1);
  M_add_symbol (sym_tbl, PLAYDOHopcode_S_Q_C1, PLAYDOHop_S_Q_C1);
  M_add_symbol (sym_tbl, PLAYDOHopcode_S_Q_C2, PLAYDOHop_S_Q_C2);
  M_add_symbol (sym_tbl, PLAYDOHopcode_S_Q_C3, PLAYDOHop_S_Q_C3);

  M_add_symbol (sym_tbl, PLAYDOHopcode_SI_B_V1, PLAYDOHop_SI_B_V1);
  M_add_symbol (sym_tbl, PLAYDOHopcode_SI_B_C1, PLAYDOHop_SI_B_C1);
  M_add_symbol (sym_tbl, PLAYDOHopcode_SI_B_C2, PLAYDOHop_SI_B_C2);
  M_add_symbol (sym_tbl, PLAYDOHopcode_SI_B_C3, PLAYDOHop_SI_B_C3);

  M_add_symbol (sym_tbl, PLAYDOHopcode_SI_H_V1, PLAYDOHop_SI_H_V1);
  M_add_symbol (sym_tbl, PLAYDOHopcode_SI_H_C1, PLAYDOHop_SI_H_C1);
  M_add_symbol (sym_tbl, PLAYDOHopcode_SI_H_C2, PLAYDOHop_SI_H_C2);
  M_add_symbol (sym_tbl, PLAYDOHopcode_SI_H_C3, PLAYDOHop_SI_H_C3);

  M_add_symbol (sym_tbl, PLAYDOHopcode_SI_W_V1, PLAYDOHop_SI_W_V1);
  M_add_symbol (sym_tbl, PLAYDOHopcode_SI_W_C1, PLAYDOHop_SI_W_C1);
  M_add_symbol (sym_tbl, PLAYDOHopcode_SI_W_C2, PLAYDOHop_SI_W_C2);
  M_add_symbol (sym_tbl, PLAYDOHopcode_SI_W_C3, PLAYDOHop_SI_W_C3);

  M_add_symbol (sym_tbl, PLAYDOHopcode_SI_Q_V1, PLAYDOHop_SI_Q_V1);
  M_add_symbol (sym_tbl, PLAYDOHopcode_SI_Q_C1, PLAYDOHop_SI_Q_C1);
  M_add_symbol (sym_tbl, PLAYDOHopcode_SI_Q_C2, PLAYDOHop_SI_Q_C2);
  M_add_symbol (sym_tbl, PLAYDOHopcode_SI_Q_C3, PLAYDOHop_SI_Q_C3);

  M_add_symbol (sym_tbl, PLAYDOHopcode_FL_S_V1_V1, PLAYDOHop_FL_S_V1_V1);
  M_add_symbol (sym_tbl, PLAYDOHopcode_FL_S_V1_C1, PLAYDOHop_FL_S_V1_C1);
  M_add_symbol (sym_tbl, PLAYDOHopcode_FL_S_V1_C2, PLAYDOHop_FL_S_V1_C2);
  M_add_symbol (sym_tbl, PLAYDOHopcode_FL_S_V1_C3, PLAYDOHop_FL_S_V1_C3);
  M_add_symbol (sym_tbl, PLAYDOHopcode_FL_S_C1_V1, PLAYDOHop_FL_S_C1_V1);
  M_add_symbol (sym_tbl, PLAYDOHopcode_FL_S_C1_C1, PLAYDOHop_FL_S_C1_C1);
  M_add_symbol (sym_tbl, PLAYDOHopcode_FL_S_C1_C2, PLAYDOHop_FL_S_C1_C2);
  M_add_symbol (sym_tbl, PLAYDOHopcode_FL_S_C1_C3, PLAYDOHop_FL_S_C1_C3);
  M_add_symbol (sym_tbl, PLAYDOHopcode_FL_S_C2_V1, PLAYDOHop_FL_S_C2_V1);
  M_add_symbol (sym_tbl, PLAYDOHopcode_FL_S_C2_C1, PLAYDOHop_FL_S_C2_C1);
  M_add_symbol (sym_tbl, PLAYDOHopcode_FL_S_C2_C2, PLAYDOHop_FL_S_C2_C2);
  M_add_symbol (sym_tbl, PLAYDOHopcode_FL_S_C2_C3, PLAYDOHop_FL_S_C2_C3);
  M_add_symbol (sym_tbl, PLAYDOHopcode_FL_S_C3_V1, PLAYDOHop_FL_S_C3_V1);
  M_add_symbol (sym_tbl, PLAYDOHopcode_FL_S_C3_C1, PLAYDOHop_FL_S_C3_C1);
  M_add_symbol (sym_tbl, PLAYDOHopcode_FL_S_C3_C2, PLAYDOHop_FL_S_C3_C2);
  M_add_symbol (sym_tbl, PLAYDOHopcode_FL_S_C3_C3, PLAYDOHop_FL_S_C3_C3);

  M_add_symbol (sym_tbl, PLAYDOHopcode_FL_D_V1_V1, PLAYDOHop_FL_D_V1_V1);
  M_add_symbol (sym_tbl, PLAYDOHopcode_FL_D_V1_C1, PLAYDOHop_FL_D_V1_C1);
  M_add_symbol (sym_tbl, PLAYDOHopcode_FL_D_V1_C2, PLAYDOHop_FL_D_V1_C2);
  M_add_symbol (sym_tbl, PLAYDOHopcode_FL_D_V1_C3, PLAYDOHop_FL_D_V1_C3);
  M_add_symbol (sym_tbl, PLAYDOHopcode_FL_D_C1_V1, PLAYDOHop_FL_D_C1_V1);
  M_add_symbol (sym_tbl, PLAYDOHopcode_FL_D_C1_C1, PLAYDOHop_FL_D_C1_C1);
  M_add_symbol (sym_tbl, PLAYDOHopcode_FL_D_C1_C2, PLAYDOHop_FL_D_C1_C2);
  M_add_symbol (sym_tbl, PLAYDOHopcode_FL_D_C1_C3, PLAYDOHop_FL_D_C1_C3);
  M_add_symbol (sym_tbl, PLAYDOHopcode_FL_D_C2_V1, PLAYDOHop_FL_D_C2_V1);
  M_add_symbol (sym_tbl, PLAYDOHopcode_FL_D_C2_C1, PLAYDOHop_FL_D_C2_C1);
  M_add_symbol (sym_tbl, PLAYDOHopcode_FL_D_C2_C2, PLAYDOHop_FL_D_C2_C2);
  M_add_symbol (sym_tbl, PLAYDOHopcode_FL_D_C2_C3, PLAYDOHop_FL_D_C2_C3);
  M_add_symbol (sym_tbl, PLAYDOHopcode_FL_D_C3_V1, PLAYDOHop_FL_D_C3_V1);
  M_add_symbol (sym_tbl, PLAYDOHopcode_FL_D_C3_C1, PLAYDOHop_FL_D_C3_C1);
  M_add_symbol (sym_tbl, PLAYDOHopcode_FL_D_C3_C2, PLAYDOHop_FL_D_C3_C2);
  M_add_symbol (sym_tbl, PLAYDOHopcode_FL_D_C3_C3, PLAYDOHop_FL_D_C3_C3);

  M_add_symbol (sym_tbl, PLAYDOHopcode_FLI_S_V1_V1, PLAYDOHop_FLI_S_V1_V1);
  M_add_symbol (sym_tbl, PLAYDOHopcode_FLI_S_V1_C1, PLAYDOHop_FLI_S_V1_C1);
  M_add_symbol (sym_tbl, PLAYDOHopcode_FLI_S_V1_C2, PLAYDOHop_FLI_S_V1_C2);
  M_add_symbol (sym_tbl, PLAYDOHopcode_FLI_S_V1_C3, PLAYDOHop_FLI_S_V1_C3);
  M_add_symbol (sym_tbl, PLAYDOHopcode_FLI_S_C1_V1, PLAYDOHop_FLI_S_C1_V1);
  M_add_symbol (sym_tbl, PLAYDOHopcode_FLI_S_C1_C1, PLAYDOHop_FLI_S_C1_C1);
  M_add_symbol (sym_tbl, PLAYDOHopcode_FLI_S_C1_C2, PLAYDOHop_FLI_S_C1_C2);
  M_add_symbol (sym_tbl, PLAYDOHopcode_FLI_S_C1_C3, PLAYDOHop_FLI_S_C1_C3);
  M_add_symbol (sym_tbl, PLAYDOHopcode_FLI_S_C2_V1, PLAYDOHop_FLI_S_C2_V1);
  M_add_symbol (sym_tbl, PLAYDOHopcode_FLI_S_C2_C1, PLAYDOHop_FLI_S_C2_C1);
  M_add_symbol (sym_tbl, PLAYDOHopcode_FLI_S_C2_C2, PLAYDOHop_FLI_S_C2_C2);
  M_add_symbol (sym_tbl, PLAYDOHopcode_FLI_S_C2_C3, PLAYDOHop_FLI_S_C2_C3);
  M_add_symbol (sym_tbl, PLAYDOHopcode_FLI_S_C3_V1, PLAYDOHop_FLI_S_C3_V1);
  M_add_symbol (sym_tbl, PLAYDOHopcode_FLI_S_C3_C1, PLAYDOHop_FLI_S_C3_C1);
  M_add_symbol (sym_tbl, PLAYDOHopcode_FLI_S_C3_C2, PLAYDOHop_FLI_S_C3_C2);
  M_add_symbol (sym_tbl, PLAYDOHopcode_FLI_S_C3_C3, PLAYDOHop_FLI_S_C3_C3);

  M_add_symbol (sym_tbl, PLAYDOHopcode_FLI_D_V1_V1, PLAYDOHop_FLI_D_V1_V1);
  M_add_symbol (sym_tbl, PLAYDOHopcode_FLI_D_V1_C1, PLAYDOHop_FLI_D_V1_C1);
  M_add_symbol (sym_tbl, PLAYDOHopcode_FLI_D_V1_C2, PLAYDOHop_FLI_D_V1_C2);
  M_add_symbol (sym_tbl, PLAYDOHopcode_FLI_D_V1_C3, PLAYDOHop_FLI_D_V1_C3);
  M_add_symbol (sym_tbl, PLAYDOHopcode_FLI_D_C1_V1, PLAYDOHop_FLI_D_C1_V1);
  M_add_symbol (sym_tbl, PLAYDOHopcode_FLI_D_C1_C1, PLAYDOHop_FLI_D_C1_C1);
  M_add_symbol (sym_tbl, PLAYDOHopcode_FLI_D_C1_C2, PLAYDOHop_FLI_D_C1_C2);
  M_add_symbol (sym_tbl, PLAYDOHopcode_FLI_D_C1_C3, PLAYDOHop_FLI_D_C1_C3);
  M_add_symbol (sym_tbl, PLAYDOHopcode_FLI_D_C2_V1, PLAYDOHop_FLI_D_C2_V1);
  M_add_symbol (sym_tbl, PLAYDOHopcode_FLI_D_C2_C1, PLAYDOHop_FLI_D_C2_C1);
  M_add_symbol (sym_tbl, PLAYDOHopcode_FLI_D_C2_C2, PLAYDOHop_FLI_D_C2_C2);
  M_add_symbol (sym_tbl, PLAYDOHopcode_FLI_D_C2_C3, PLAYDOHop_FLI_D_C2_C3);
  M_add_symbol (sym_tbl, PLAYDOHopcode_FLI_D_C3_V1, PLAYDOHop_FLI_D_C3_V1);
  M_add_symbol (sym_tbl, PLAYDOHopcode_FLI_D_C3_C1, PLAYDOHop_FLI_D_C3_C1);
  M_add_symbol (sym_tbl, PLAYDOHopcode_FLI_D_C3_C2, PLAYDOHop_FLI_D_C3_C2);
  M_add_symbol (sym_tbl, PLAYDOHopcode_FLI_D_C3_C3, PLAYDOHop_FLI_D_C3_C3);

  M_add_symbol (sym_tbl, PLAYDOHopcode_FS_S_V1, PLAYDOHop_FS_S_V1);
  M_add_symbol (sym_tbl, PLAYDOHopcode_FS_S_C1, PLAYDOHop_FS_S_C1);
  M_add_symbol (sym_tbl, PLAYDOHopcode_FS_S_C2, PLAYDOHop_FS_S_C2);
  M_add_symbol (sym_tbl, PLAYDOHopcode_FS_S_C3, PLAYDOHop_FS_S_C3);

  M_add_symbol (sym_tbl, PLAYDOHopcode_FS_D_V1, PLAYDOHop_FS_D_V1);
  M_add_symbol (sym_tbl, PLAYDOHopcode_FS_D_C1, PLAYDOHop_FS_D_C1);
  M_add_symbol (sym_tbl, PLAYDOHopcode_FS_D_C2, PLAYDOHop_FS_D_C2);
  M_add_symbol (sym_tbl, PLAYDOHopcode_FS_D_C3, PLAYDOHop_FS_D_C3);

  M_add_symbol (sym_tbl, PLAYDOHopcode_FSI_S_V1, PLAYDOHop_FSI_S_V1);
  M_add_symbol (sym_tbl, PLAYDOHopcode_FSI_S_C1, PLAYDOHop_FSI_S_C1);
  M_add_symbol (sym_tbl, PLAYDOHopcode_FSI_S_C2, PLAYDOHop_FSI_S_C2);
  M_add_symbol (sym_tbl, PLAYDOHopcode_FSI_S_C3, PLAYDOHop_FSI_S_C3);

  M_add_symbol (sym_tbl, PLAYDOHopcode_FSI_D_V1, PLAYDOHop_FSI_D_V1);
  M_add_symbol (sym_tbl, PLAYDOHopcode_FSI_D_C1, PLAYDOHop_FSI_D_C1);
  M_add_symbol (sym_tbl, PLAYDOHopcode_FSI_D_C2, PLAYDOHop_FSI_D_C2);
  M_add_symbol (sym_tbl, PLAYDOHopcode_FSI_D_C3, PLAYDOHop_FSI_D_C3);

  M_add_symbol (sym_tbl, PLAYDOHopcode_LDS_B_V1_V1, PLAYDOHop_LDS_B_V1_V1);
  M_add_symbol (sym_tbl, PLAYDOHopcode_LDS_B_V1_C1, PLAYDOHop_LDS_B_V1_C1);
  M_add_symbol (sym_tbl, PLAYDOHopcode_LDS_B_V1_C2, PLAYDOHop_LDS_B_V1_C2);
  M_add_symbol (sym_tbl, PLAYDOHopcode_LDS_B_V1_C3, PLAYDOHop_LDS_B_V1_C3);
  M_add_symbol (sym_tbl, PLAYDOHopcode_LDS_B_C1_V1, PLAYDOHop_LDS_B_C1_V1);
  M_add_symbol (sym_tbl, PLAYDOHopcode_LDS_B_C1_C1, PLAYDOHop_LDS_B_C1_C1);
  M_add_symbol (sym_tbl, PLAYDOHopcode_LDS_B_C1_C2, PLAYDOHop_LDS_B_C1_C2);
  M_add_symbol (sym_tbl, PLAYDOHopcode_LDS_B_C1_C3, PLAYDOHop_LDS_B_C1_C3);
  M_add_symbol (sym_tbl, PLAYDOHopcode_LDS_B_C2_V1, PLAYDOHop_LDS_B_C2_V1);
  M_add_symbol (sym_tbl, PLAYDOHopcode_LDS_B_C2_C1, PLAYDOHop_LDS_B_C2_C1);
  M_add_symbol (sym_tbl, PLAYDOHopcode_LDS_B_C2_C2, PLAYDOHop_LDS_B_C2_C2);
  M_add_symbol (sym_tbl, PLAYDOHopcode_LDS_B_C2_C3, PLAYDOHop_LDS_B_C2_C3);
  M_add_symbol (sym_tbl, PLAYDOHopcode_LDS_B_C3_V1, PLAYDOHop_LDS_B_C3_V1);
  M_add_symbol (sym_tbl, PLAYDOHopcode_LDS_B_C3_C1, PLAYDOHop_LDS_B_C3_C1);
  M_add_symbol (sym_tbl, PLAYDOHopcode_LDS_B_C3_C2, PLAYDOHop_LDS_B_C3_C2);
  M_add_symbol (sym_tbl, PLAYDOHopcode_LDS_B_C3_C3, PLAYDOHop_LDS_B_C3_C3);

  M_add_symbol (sym_tbl, PLAYDOHopcode_LDS_H_V1_V1, PLAYDOHop_LDS_H_V1_V1);
  M_add_symbol (sym_tbl, PLAYDOHopcode_LDS_H_V1_C1, PLAYDOHop_LDS_H_V1_C1);
  M_add_symbol (sym_tbl, PLAYDOHopcode_LDS_H_V1_C2, PLAYDOHop_LDS_H_V1_C2);
  M_add_symbol (sym_tbl, PLAYDOHopcode_LDS_H_V1_C3, PLAYDOHop_LDS_H_V1_C3);
  M_add_symbol (sym_tbl, PLAYDOHopcode_LDS_H_C1_V1, PLAYDOHop_LDS_H_C1_V1);
  M_add_symbol (sym_tbl, PLAYDOHopcode_LDS_H_C1_C1, PLAYDOHop_LDS_H_C1_C1);
  M_add_symbol (sym_tbl, PLAYDOHopcode_LDS_H_C1_C2, PLAYDOHop_LDS_H_C1_C2);
  M_add_symbol (sym_tbl, PLAYDOHopcode_LDS_H_C1_C3, PLAYDOHop_LDS_H_C1_C3);
  M_add_symbol (sym_tbl, PLAYDOHopcode_LDS_H_C2_V1, PLAYDOHop_LDS_H_C2_V1);
  M_add_symbol (sym_tbl, PLAYDOHopcode_LDS_H_C2_C1, PLAYDOHop_LDS_H_C2_C1);
  M_add_symbol (sym_tbl, PLAYDOHopcode_LDS_H_C2_C2, PLAYDOHop_LDS_H_C2_C2);
  M_add_symbol (sym_tbl, PLAYDOHopcode_LDS_H_C2_C3, PLAYDOHop_LDS_H_C2_C3);
  M_add_symbol (sym_tbl, PLAYDOHopcode_LDS_H_C3_V1, PLAYDOHop_LDS_H_C3_V1);
  M_add_symbol (sym_tbl, PLAYDOHopcode_LDS_H_C3_C1, PLAYDOHop_LDS_H_C3_C1);
  M_add_symbol (sym_tbl, PLAYDOHopcode_LDS_H_C3_C2, PLAYDOHop_LDS_H_C3_C2);
  M_add_symbol (sym_tbl, PLAYDOHopcode_LDS_H_C3_C3, PLAYDOHop_LDS_H_C3_C3);

  M_add_symbol (sym_tbl, PLAYDOHopcode_LDS_W_V1_V1, PLAYDOHop_LDS_W_V1_V1);
  M_add_symbol (sym_tbl, PLAYDOHopcode_LDS_W_V1_C1, PLAYDOHop_LDS_W_V1_C1);
  M_add_symbol (sym_tbl, PLAYDOHopcode_LDS_W_V1_C2, PLAYDOHop_LDS_W_V1_C2);
  M_add_symbol (sym_tbl, PLAYDOHopcode_LDS_W_V1_C3, PLAYDOHop_LDS_W_V1_C3);
  M_add_symbol (sym_tbl, PLAYDOHopcode_LDS_W_C1_V1, PLAYDOHop_LDS_W_C1_V1);
  M_add_symbol (sym_tbl, PLAYDOHopcode_LDS_W_C1_C1, PLAYDOHop_LDS_W_C1_C1);
  M_add_symbol (sym_tbl, PLAYDOHopcode_LDS_W_C1_C2, PLAYDOHop_LDS_W_C1_C2);
  M_add_symbol (sym_tbl, PLAYDOHopcode_LDS_W_C1_C3, PLAYDOHop_LDS_W_C1_C3);
  M_add_symbol (sym_tbl, PLAYDOHopcode_LDS_W_C2_V1, PLAYDOHop_LDS_W_C2_V1);
  M_add_symbol (sym_tbl, PLAYDOHopcode_LDS_W_C2_C1, PLAYDOHop_LDS_W_C2_C1);
  M_add_symbol (sym_tbl, PLAYDOHopcode_LDS_W_C2_C2, PLAYDOHop_LDS_W_C2_C2);
  M_add_symbol (sym_tbl, PLAYDOHopcode_LDS_W_C2_C3, PLAYDOHop_LDS_W_C2_C3);
  M_add_symbol (sym_tbl, PLAYDOHopcode_LDS_W_C3_V1, PLAYDOHop_LDS_W_C3_V1);
  M_add_symbol (sym_tbl, PLAYDOHopcode_LDS_W_C3_C1, PLAYDOHop_LDS_W_C3_C1);
  M_add_symbol (sym_tbl, PLAYDOHopcode_LDS_W_C3_C2, PLAYDOHop_LDS_W_C3_C2);
  M_add_symbol (sym_tbl, PLAYDOHopcode_LDS_W_C3_C3, PLAYDOHop_LDS_W_C3_C3);

  M_add_symbol (sym_tbl, PLAYDOHopcode_LDS_Q_V1_V1, PLAYDOHop_LDS_Q_V1_V1);
  M_add_symbol (sym_tbl, PLAYDOHopcode_LDS_Q_V1_C1, PLAYDOHop_LDS_Q_V1_C1);
  M_add_symbol (sym_tbl, PLAYDOHopcode_LDS_Q_V1_C2, PLAYDOHop_LDS_Q_V1_C2);
  M_add_symbol (sym_tbl, PLAYDOHopcode_LDS_Q_V1_C3, PLAYDOHop_LDS_Q_V1_C3);
  M_add_symbol (sym_tbl, PLAYDOHopcode_LDS_Q_C1_V1, PLAYDOHop_LDS_Q_C1_V1);
  M_add_symbol (sym_tbl, PLAYDOHopcode_LDS_Q_C1_C1, PLAYDOHop_LDS_Q_C1_C1);
  M_add_symbol (sym_tbl, PLAYDOHopcode_LDS_Q_C1_C2, PLAYDOHop_LDS_Q_C1_C2);
  M_add_symbol (sym_tbl, PLAYDOHopcode_LDS_Q_C1_C3, PLAYDOHop_LDS_Q_C1_C3);
  M_add_symbol (sym_tbl, PLAYDOHopcode_LDS_Q_C2_V1, PLAYDOHop_LDS_Q_C2_V1);
  M_add_symbol (sym_tbl, PLAYDOHopcode_LDS_Q_C2_C1, PLAYDOHop_LDS_Q_C2_C1);
  M_add_symbol (sym_tbl, PLAYDOHopcode_LDS_Q_C2_C2, PLAYDOHop_LDS_Q_C2_C2);
  M_add_symbol (sym_tbl, PLAYDOHopcode_LDS_Q_C2_C3, PLAYDOHop_LDS_Q_C2_C3);
  M_add_symbol (sym_tbl, PLAYDOHopcode_LDS_Q_C3_V1, PLAYDOHop_LDS_Q_C3_V1);
  M_add_symbol (sym_tbl, PLAYDOHopcode_LDS_Q_C3_C1, PLAYDOHop_LDS_Q_C3_C1);
  M_add_symbol (sym_tbl, PLAYDOHopcode_LDS_Q_C3_C2, PLAYDOHop_LDS_Q_C3_C2);
  M_add_symbol (sym_tbl, PLAYDOHopcode_LDS_Q_C3_C3, PLAYDOHop_LDS_Q_C3_C3);

  M_add_symbol (sym_tbl, PLAYDOHopcode_LDSI_B_V1_V1, PLAYDOHop_LDSI_B_V1_V1);
  M_add_symbol (sym_tbl, PLAYDOHopcode_LDSI_B_V1_C1, PLAYDOHop_LDSI_B_V1_C1);
  M_add_symbol (sym_tbl, PLAYDOHopcode_LDSI_B_V1_C2, PLAYDOHop_LDSI_B_V1_C2);
  M_add_symbol (sym_tbl, PLAYDOHopcode_LDSI_B_V1_C3, PLAYDOHop_LDSI_B_V1_C3);
  M_add_symbol (sym_tbl, PLAYDOHopcode_LDSI_B_C1_V1, PLAYDOHop_LDSI_B_C1_V1);
  M_add_symbol (sym_tbl, PLAYDOHopcode_LDSI_B_C1_C1, PLAYDOHop_LDSI_B_C1_C1);
  M_add_symbol (sym_tbl, PLAYDOHopcode_LDSI_B_C1_C2, PLAYDOHop_LDSI_B_C1_C2);
  M_add_symbol (sym_tbl, PLAYDOHopcode_LDSI_B_C1_C3, PLAYDOHop_LDSI_B_C1_C3);
  M_add_symbol (sym_tbl, PLAYDOHopcode_LDSI_B_C2_V1, PLAYDOHop_LDSI_B_C2_V1);
  M_add_symbol (sym_tbl, PLAYDOHopcode_LDSI_B_C2_C1, PLAYDOHop_LDSI_B_C2_C1);
  M_add_symbol (sym_tbl, PLAYDOHopcode_LDSI_B_C2_C2, PLAYDOHop_LDSI_B_C2_C2);
  M_add_symbol (sym_tbl, PLAYDOHopcode_LDSI_B_C2_C3, PLAYDOHop_LDSI_B_C2_C3);
  M_add_symbol (sym_tbl, PLAYDOHopcode_LDSI_B_C3_V1, PLAYDOHop_LDSI_B_C3_V1);
  M_add_symbol (sym_tbl, PLAYDOHopcode_LDSI_B_C3_C1, PLAYDOHop_LDSI_B_C3_C1);
  M_add_symbol (sym_tbl, PLAYDOHopcode_LDSI_B_C3_C2, PLAYDOHop_LDSI_B_C3_C2);
  M_add_symbol (sym_tbl, PLAYDOHopcode_LDSI_B_C3_C3, PLAYDOHop_LDSI_B_C3_C3);

  M_add_symbol (sym_tbl, PLAYDOHopcode_LDSI_H_V1_V1, PLAYDOHop_LDSI_H_V1_V1);
  M_add_symbol (sym_tbl, PLAYDOHopcode_LDSI_H_V1_C1, PLAYDOHop_LDSI_H_V1_C1);
  M_add_symbol (sym_tbl, PLAYDOHopcode_LDSI_H_V1_C2, PLAYDOHop_LDSI_H_V1_C2);
  M_add_symbol (sym_tbl, PLAYDOHopcode_LDSI_H_V1_C3, PLAYDOHop_LDSI_H_V1_C3);
  M_add_symbol (sym_tbl, PLAYDOHopcode_LDSI_H_C1_V1, PLAYDOHop_LDSI_H_C1_V1);
  M_add_symbol (sym_tbl, PLAYDOHopcode_LDSI_H_C1_C1, PLAYDOHop_LDSI_H_C1_C1);
  M_add_symbol (sym_tbl, PLAYDOHopcode_LDSI_H_C1_C2, PLAYDOHop_LDSI_H_C1_C2);
  M_add_symbol (sym_tbl, PLAYDOHopcode_LDSI_H_C1_C3, PLAYDOHop_LDSI_H_C1_C3);
  M_add_symbol (sym_tbl, PLAYDOHopcode_LDSI_H_C2_V1, PLAYDOHop_LDSI_H_C2_V1);
  M_add_symbol (sym_tbl, PLAYDOHopcode_LDSI_H_C2_C1, PLAYDOHop_LDSI_H_C2_C1);
  M_add_symbol (sym_tbl, PLAYDOHopcode_LDSI_H_C2_C2, PLAYDOHop_LDSI_H_C2_C2);
  M_add_symbol (sym_tbl, PLAYDOHopcode_LDSI_H_C2_C3, PLAYDOHop_LDSI_H_C2_C3);
  M_add_symbol (sym_tbl, PLAYDOHopcode_LDSI_H_C3_V1, PLAYDOHop_LDSI_H_C3_V1);
  M_add_symbol (sym_tbl, PLAYDOHopcode_LDSI_H_C3_C1, PLAYDOHop_LDSI_H_C3_C1);
  M_add_symbol (sym_tbl, PLAYDOHopcode_LDSI_H_C3_C2, PLAYDOHop_LDSI_H_C3_C2);
  M_add_symbol (sym_tbl, PLAYDOHopcode_LDSI_H_C3_C3, PLAYDOHop_LDSI_H_C3_C3);

  M_add_symbol (sym_tbl, PLAYDOHopcode_LDSI_W_V1_V1, PLAYDOHop_LDSI_W_V1_V1);
  M_add_symbol (sym_tbl, PLAYDOHopcode_LDSI_W_V1_C1, PLAYDOHop_LDSI_W_V1_C1);
  M_add_symbol (sym_tbl, PLAYDOHopcode_LDSI_W_V1_C2, PLAYDOHop_LDSI_W_V1_C2);
  M_add_symbol (sym_tbl, PLAYDOHopcode_LDSI_W_V1_C3, PLAYDOHop_LDSI_W_V1_C3);
  M_add_symbol (sym_tbl, PLAYDOHopcode_LDSI_W_C1_V1, PLAYDOHop_LDSI_W_C1_V1);
  M_add_symbol (sym_tbl, PLAYDOHopcode_LDSI_W_C1_C1, PLAYDOHop_LDSI_W_C1_C1);
  M_add_symbol (sym_tbl, PLAYDOHopcode_LDSI_W_C1_C2, PLAYDOHop_LDSI_W_C1_C2);
  M_add_symbol (sym_tbl, PLAYDOHopcode_LDSI_W_C1_C3, PLAYDOHop_LDSI_W_C1_C3);
  M_add_symbol (sym_tbl, PLAYDOHopcode_LDSI_W_C2_V1, PLAYDOHop_LDSI_W_C2_V1);
  M_add_symbol (sym_tbl, PLAYDOHopcode_LDSI_W_C2_C1, PLAYDOHop_LDSI_W_C2_C1);
  M_add_symbol (sym_tbl, PLAYDOHopcode_LDSI_W_C2_C2, PLAYDOHop_LDSI_W_C2_C2);
  M_add_symbol (sym_tbl, PLAYDOHopcode_LDSI_W_C2_C3, PLAYDOHop_LDSI_W_C2_C3);
  M_add_symbol (sym_tbl, PLAYDOHopcode_LDSI_W_C3_V1, PLAYDOHop_LDSI_W_C3_V1);
  M_add_symbol (sym_tbl, PLAYDOHopcode_LDSI_W_C3_C1, PLAYDOHop_LDSI_W_C3_C1);
  M_add_symbol (sym_tbl, PLAYDOHopcode_LDSI_W_C3_C2, PLAYDOHop_LDSI_W_C3_C2);
  M_add_symbol (sym_tbl, PLAYDOHopcode_LDSI_W_C3_C3, PLAYDOHop_LDSI_W_C3_C3);

  M_add_symbol (sym_tbl, PLAYDOHopcode_LDSI_Q_V1_V1, PLAYDOHop_LDSI_Q_V1_V1);
  M_add_symbol (sym_tbl, PLAYDOHopcode_LDSI_Q_V1_C1, PLAYDOHop_LDSI_Q_V1_C1);
  M_add_symbol (sym_tbl, PLAYDOHopcode_LDSI_Q_V1_C2, PLAYDOHop_LDSI_Q_V1_C2);
  M_add_symbol (sym_tbl, PLAYDOHopcode_LDSI_Q_V1_C3, PLAYDOHop_LDSI_Q_V1_C3);
  M_add_symbol (sym_tbl, PLAYDOHopcode_LDSI_Q_C1_V1, PLAYDOHop_LDSI_Q_C1_V1);
  M_add_symbol (sym_tbl, PLAYDOHopcode_LDSI_Q_C1_C1, PLAYDOHop_LDSI_Q_C1_C1);
  M_add_symbol (sym_tbl, PLAYDOHopcode_LDSI_Q_C1_C2, PLAYDOHop_LDSI_Q_C1_C2);
  M_add_symbol (sym_tbl, PLAYDOHopcode_LDSI_Q_C1_C3, PLAYDOHop_LDSI_Q_C1_C3);
  M_add_symbol (sym_tbl, PLAYDOHopcode_LDSI_Q_C2_V1, PLAYDOHop_LDSI_Q_C2_V1);
  M_add_symbol (sym_tbl, PLAYDOHopcode_LDSI_Q_C2_C1, PLAYDOHop_LDSI_Q_C2_C1);
  M_add_symbol (sym_tbl, PLAYDOHopcode_LDSI_Q_C2_C2, PLAYDOHop_LDSI_Q_C2_C2);
  M_add_symbol (sym_tbl, PLAYDOHopcode_LDSI_Q_C2_C3, PLAYDOHop_LDSI_Q_C2_C3);
  M_add_symbol (sym_tbl, PLAYDOHopcode_LDSI_Q_C3_V1, PLAYDOHop_LDSI_Q_C3_V1);
  M_add_symbol (sym_tbl, PLAYDOHopcode_LDSI_Q_C3_C1, PLAYDOHop_LDSI_Q_C3_C1);
  M_add_symbol (sym_tbl, PLAYDOHopcode_LDSI_Q_C3_C2, PLAYDOHop_LDSI_Q_C3_C2);
  M_add_symbol (sym_tbl, PLAYDOHopcode_LDSI_Q_C3_C3, PLAYDOHop_LDSI_Q_C3_C3);

  M_add_symbol (sym_tbl, PLAYDOHopcode_FLDS_S_V1_V1, PLAYDOHop_FLDS_S_V1_V1);
  M_add_symbol (sym_tbl, PLAYDOHopcode_FLDS_S_V1_C1, PLAYDOHop_FLDS_S_V1_C1);
  M_add_symbol (sym_tbl, PLAYDOHopcode_FLDS_S_V1_C2, PLAYDOHop_FLDS_S_V1_C2);
  M_add_symbol (sym_tbl, PLAYDOHopcode_FLDS_S_V1_C3, PLAYDOHop_FLDS_S_V1_C3);
  M_add_symbol (sym_tbl, PLAYDOHopcode_FLDS_S_C1_V1, PLAYDOHop_FLDS_S_C1_V1);
  M_add_symbol (sym_tbl, PLAYDOHopcode_FLDS_S_C1_C1, PLAYDOHop_FLDS_S_C1_C1);
  M_add_symbol (sym_tbl, PLAYDOHopcode_FLDS_S_C1_C2, PLAYDOHop_FLDS_S_C1_C2);
  M_add_symbol (sym_tbl, PLAYDOHopcode_FLDS_S_C1_C3, PLAYDOHop_FLDS_S_C1_C3);
  M_add_symbol (sym_tbl, PLAYDOHopcode_FLDS_S_C2_V1, PLAYDOHop_FLDS_S_C2_V1);
  M_add_symbol (sym_tbl, PLAYDOHopcode_FLDS_S_C2_C1, PLAYDOHop_FLDS_S_C2_C1);
  M_add_symbol (sym_tbl, PLAYDOHopcode_FLDS_S_C2_C2, PLAYDOHop_FLDS_S_C2_C2);
  M_add_symbol (sym_tbl, PLAYDOHopcode_FLDS_S_C2_C3, PLAYDOHop_FLDS_S_C2_C3);
  M_add_symbol (sym_tbl, PLAYDOHopcode_FLDS_S_C3_V1, PLAYDOHop_FLDS_S_C3_V1);
  M_add_symbol (sym_tbl, PLAYDOHopcode_FLDS_S_C3_C1, PLAYDOHop_FLDS_S_C3_C1);
  M_add_symbol (sym_tbl, PLAYDOHopcode_FLDS_S_C3_C2, PLAYDOHop_FLDS_S_C3_C2);
  M_add_symbol (sym_tbl, PLAYDOHopcode_FLDS_S_C3_C3, PLAYDOHop_FLDS_S_C3_C3);

  M_add_symbol (sym_tbl, PLAYDOHopcode_FLDS_D_V1_V1, PLAYDOHop_FLDS_D_V1_V1);
  M_add_symbol (sym_tbl, PLAYDOHopcode_FLDS_D_V1_C1, PLAYDOHop_FLDS_D_V1_C1);
  M_add_symbol (sym_tbl, PLAYDOHopcode_FLDS_D_V1_C2, PLAYDOHop_FLDS_D_V1_C2);
  M_add_symbol (sym_tbl, PLAYDOHopcode_FLDS_D_V1_C3, PLAYDOHop_FLDS_D_V1_C3);
  M_add_symbol (sym_tbl, PLAYDOHopcode_FLDS_D_C1_V1, PLAYDOHop_FLDS_D_C1_V1);
  M_add_symbol (sym_tbl, PLAYDOHopcode_FLDS_D_C1_C1, PLAYDOHop_FLDS_D_C1_C1);
  M_add_symbol (sym_tbl, PLAYDOHopcode_FLDS_D_C1_C2, PLAYDOHop_FLDS_D_C1_C2);
  M_add_symbol (sym_tbl, PLAYDOHopcode_FLDS_D_C1_C3, PLAYDOHop_FLDS_D_C1_C3);
  M_add_symbol (sym_tbl, PLAYDOHopcode_FLDS_D_C2_V1, PLAYDOHop_FLDS_D_C2_V1);
  M_add_symbol (sym_tbl, PLAYDOHopcode_FLDS_D_C2_C1, PLAYDOHop_FLDS_D_C2_C1);
  M_add_symbol (sym_tbl, PLAYDOHopcode_FLDS_D_C2_C2, PLAYDOHop_FLDS_D_C2_C2);
  M_add_symbol (sym_tbl, PLAYDOHopcode_FLDS_D_C2_C3, PLAYDOHop_FLDS_D_C2_C3);
  M_add_symbol (sym_tbl, PLAYDOHopcode_FLDS_D_C3_V1, PLAYDOHop_FLDS_D_C3_V1);
  M_add_symbol (sym_tbl, PLAYDOHopcode_FLDS_D_C3_C1, PLAYDOHop_FLDS_D_C3_C1);
  M_add_symbol (sym_tbl, PLAYDOHopcode_FLDS_D_C3_C2, PLAYDOHop_FLDS_D_C3_C2);
  M_add_symbol (sym_tbl, PLAYDOHopcode_FLDS_D_C3_C3, PLAYDOHop_FLDS_D_C3_C3);

  M_add_symbol (sym_tbl, PLAYDOHopcode_FLDSI_S_V1_V1,
		PLAYDOHop_FLDSI_S_V1_V1);
  M_add_symbol (sym_tbl, PLAYDOHopcode_FLDSI_S_V1_C1,
		PLAYDOHop_FLDSI_S_V1_C1);
  M_add_symbol (sym_tbl, PLAYDOHopcode_FLDSI_S_V1_C2,
		PLAYDOHop_FLDSI_S_V1_C2);
  M_add_symbol (sym_tbl, PLAYDOHopcode_FLDSI_S_V1_C3,
		PLAYDOHop_FLDSI_S_V1_C3);
  M_add_symbol (sym_tbl, PLAYDOHopcode_FLDSI_S_C1_V1,
		PLAYDOHop_FLDSI_S_C1_V1);
  M_add_symbol (sym_tbl, PLAYDOHopcode_FLDSI_S_C1_C1,
		PLAYDOHop_FLDSI_S_C1_C1);
  M_add_symbol (sym_tbl, PLAYDOHopcode_FLDSI_S_C1_C2,
		PLAYDOHop_FLDSI_S_C1_C2);
  M_add_symbol (sym_tbl, PLAYDOHopcode_FLDSI_S_C1_C3,
		PLAYDOHop_FLDSI_S_C1_C3);
  M_add_symbol (sym_tbl, PLAYDOHopcode_FLDSI_S_C2_V1,
		PLAYDOHop_FLDSI_S_C2_V1);
  M_add_symbol (sym_tbl, PLAYDOHopcode_FLDSI_S_C2_C1,
		PLAYDOHop_FLDSI_S_C2_C1);
  M_add_symbol (sym_tbl, PLAYDOHopcode_FLDSI_S_C2_C2,
		PLAYDOHop_FLDSI_S_C2_C2);
  M_add_symbol (sym_tbl, PLAYDOHopcode_FLDSI_S_C2_C3,
		PLAYDOHop_FLDSI_S_C2_C3);
  M_add_symbol (sym_tbl, PLAYDOHopcode_FLDSI_S_C3_V1,
		PLAYDOHop_FLDSI_S_C3_V1);
  M_add_symbol (sym_tbl, PLAYDOHopcode_FLDSI_S_C3_C1,
		PLAYDOHop_FLDSI_S_C3_C1);
  M_add_symbol (sym_tbl, PLAYDOHopcode_FLDSI_S_C3_C2,
		PLAYDOHop_FLDSI_S_C3_C2);
  M_add_symbol (sym_tbl, PLAYDOHopcode_FLDSI_S_C3_C3,
		PLAYDOHop_FLDSI_S_C3_C3);

  M_add_symbol (sym_tbl, PLAYDOHopcode_FLDSI_D_V1_V1,
		PLAYDOHop_FLDSI_D_V1_V1);
  M_add_symbol (sym_tbl, PLAYDOHopcode_FLDSI_D_V1_C1,
		PLAYDOHop_FLDSI_D_V1_C1);
  M_add_symbol (sym_tbl, PLAYDOHopcode_FLDSI_D_V1_C2,
		PLAYDOHop_FLDSI_D_V1_C2);
  M_add_symbol (sym_tbl, PLAYDOHopcode_FLDSI_D_V1_C3,
		PLAYDOHop_FLDSI_D_V1_C3);
  M_add_symbol (sym_tbl, PLAYDOHopcode_FLDSI_D_C1_V1,
		PLAYDOHop_FLDSI_D_C1_V1);
  M_add_symbol (sym_tbl, PLAYDOHopcode_FLDSI_D_C1_C1,
		PLAYDOHop_FLDSI_D_C1_C1);
  M_add_symbol (sym_tbl, PLAYDOHopcode_FLDSI_D_C1_C2,
		PLAYDOHop_FLDSI_D_C1_C2);
  M_add_symbol (sym_tbl, PLAYDOHopcode_FLDSI_D_C1_C3,
		PLAYDOHop_FLDSI_D_C1_C3);
  M_add_symbol (sym_tbl, PLAYDOHopcode_FLDSI_D_C2_V1,
		PLAYDOHop_FLDSI_D_C2_V1);
  M_add_symbol (sym_tbl, PLAYDOHopcode_FLDSI_D_C2_C1,
		PLAYDOHop_FLDSI_D_C2_C1);
  M_add_symbol (sym_tbl, PLAYDOHopcode_FLDSI_D_C2_C2,
		PLAYDOHop_FLDSI_D_C2_C2);
  M_add_symbol (sym_tbl, PLAYDOHopcode_FLDSI_D_C2_C3,
		PLAYDOHop_FLDSI_D_C2_C3);
  M_add_symbol (sym_tbl, PLAYDOHopcode_FLDSI_D_C3_V1,
		PLAYDOHop_FLDSI_D_C3_V1);
  M_add_symbol (sym_tbl, PLAYDOHopcode_FLDSI_D_C3_C1,
		PLAYDOHop_FLDSI_D_C3_C1);
  M_add_symbol (sym_tbl, PLAYDOHopcode_FLDSI_D_C3_C2,
		PLAYDOHop_FLDSI_D_C3_C2);
  M_add_symbol (sym_tbl, PLAYDOHopcode_FLDSI_D_C3_C3,
		PLAYDOHop_FLDSI_D_C3_C3);

  M_add_symbol (sym_tbl, PLAYDOHopcode_LDV_B, PLAYDOHop_LDV_B);
  M_add_symbol (sym_tbl, PLAYDOHopcode_LDV_H, PLAYDOHop_LDV_H);
  M_add_symbol (sym_tbl, PLAYDOHopcode_LDV_W, PLAYDOHop_LDV_W);
  M_add_symbol (sym_tbl, PLAYDOHopcode_LDV_Q, PLAYDOHop_LDV_Q);
  M_add_symbol (sym_tbl, PLAYDOHopcode_FLDV_S, PLAYDOHop_FLDV_S);
  M_add_symbol (sym_tbl, PLAYDOHopcode_FLDV_D, PLAYDOHop_FLDV_D);

  M_add_symbol (sym_tbl, PLAYDOHopcode_SH1ADDL, PLAYDOHop_SH1ADDL);
  M_add_symbol (sym_tbl, PLAYDOHopcode_SH2ADDL, PLAYDOHop_SH2ADDL);
  M_add_symbol (sym_tbl, PLAYDOHopcode_SH3ADDL, PLAYDOHop_SH3ADDL);

  M_add_symbol (sym_tbl, PLAYDOHopcode_FMPYADDN_S, PLAYDOHop_FMPYADDN_S);
  M_add_symbol (sym_tbl, PLAYDOHopcode_FMPYADDN_D, PLAYDOHop_FMPYADDN_D);

  M_add_symbol (sym_tbl, PLAYDOHopcode_CMPR_FALSE, PLAYDOHop_CMPR_FALSE);
  M_add_symbol (sym_tbl, PLAYDOHopcode_CMPR_TRUE, PLAYDOHop_CMPR_TRUE);
  M_add_symbol (sym_tbl, PLAYDOHopcode_CMPR_OD, PLAYDOHop_CMPR_OD);
  M_add_symbol (sym_tbl, PLAYDOHopcode_CMPR_EV, PLAYDOHop_CMPR_EV);
  M_add_symbol (sym_tbl, PLAYDOHopcode_CMPR_SV, PLAYDOHop_CMPR_SV);
  M_add_symbol (sym_tbl, PLAYDOHopcode_CMPR_NSV, PLAYDOHop_CMPR_NSV);

  M_add_symbol (sym_tbl, PLAYDOHopcode_FCMPR_S_FALSE,
		PLAYDOHop_FCMPR_S_FALSE);
  M_add_symbol (sym_tbl, PLAYDOHopcode_FCMPR_S_TRUE, PLAYDOHop_FCMPR_S_TRUE);
  M_add_symbol (sym_tbl, PLAYDOHopcode_FCMPR_D_FALSE,
		PLAYDOHop_FCMPR_D_FALSE);
  M_add_symbol (sym_tbl, PLAYDOHopcode_FCMPR_D_TRUE, PLAYDOHop_FCMPR_D_TRUE);

  M_add_symbol (sym_tbl, PLAYDOHopcode_M_NO_OP, PLAYDOHop_M_NO_OP);

  M_add_symbol (sym_tbl, PLAYDOHopcode_MOVELB, PLAYDOHop_MOVELB);
  M_add_symbol (sym_tbl, PLAYDOHopcode_MOVELBX, PLAYDOHop_MOVELBX);
  M_add_symbol (sym_tbl, PLAYDOHopcode_MOVELBS, PLAYDOHop_MOVELBS);
  M_add_symbol (sym_tbl, PLAYDOHopcode_PBRRL, PLAYDOHop_PBRRL);
  M_add_symbol (sym_tbl, PLAYDOHopcode_PBRAL, PLAYDOHop_PBRAL);
  M_add_symbol (sym_tbl, PLAYDOHopcode_PBRRLBS, PLAYDOHop_PBRRLBS);
  M_add_symbol (sym_tbl, PLAYDOHopcode_PBRALBS, PLAYDOHop_PBRALBS);
  M_add_symbol (sym_tbl, PLAYDOHopcode_MOVELG, PLAYDOHop_MOVELG);
  M_add_symbol (sym_tbl, PLAYDOHopcode_MOVELGX, PLAYDOHop_MOVELGX);
  M_add_symbol (sym_tbl, PLAYDOHopcode_MOVELGS, PLAYDOHop_MOVELGS);
  M_add_symbol (sym_tbl, PLAYDOHopcode_MOVELF, PLAYDOHop_MOVELF);
  M_add_symbol (sym_tbl, PLAYDOHopcode_MOVELFS, PLAYDOHop_MOVELFS);

  M_add_symbol (sym_tbl, PLAYDOHopcode_MOVEGC, PLAYDOHop_MOVEGC);
  M_add_symbol (sym_tbl, PLAYDOHopcode_MOVECG, PLAYDOHop_MOVECG);
  M_add_symbol (sym_tbl, PLAYDOHopcode_MOVEGG, PLAYDOHop_MOVEGG);
  M_add_symbol (sym_tbl, PLAYDOHopcode_MOVEBB, PLAYDOHop_MOVEBB);

  M_add_symbol (sym_tbl, PLAYDOHopcode_SAVE, PLAYDOHop_SAVE);
  M_add_symbol (sym_tbl, PLAYDOHopcode_RESTORE, PLAYDOHop_RESTORE);
  M_add_symbol (sym_tbl, PLAYDOHopcode_FSAVE_S, PLAYDOHop_FSAVE_S);
  M_add_symbol (sym_tbl, PLAYDOHopcode_FRESTORE_S, PLAYDOHop_FRESTORE_S);
  M_add_symbol (sym_tbl, PLAYDOHopcode_FSAVE_D, PLAYDOHop_FSAVE_D);
  M_add_symbol (sym_tbl, PLAYDOHopcode_FRESTORE_D, PLAYDOHop_FRESTORE_D);
  M_add_symbol (sym_tbl, PLAYDOHopcode_BSAVE, PLAYDOHop_BSAVE);
  M_add_symbol (sym_tbl, PLAYDOHopcode_BRESTORE, PLAYDOHop_BRESTORE);
  M_add_symbol (sym_tbl, PLAYDOHopcode_MOVEGBP, PLAYDOHop_MOVEGBP);
  M_add_symbol (sym_tbl, PLAYDOHopcode_MOVEGCM, PLAYDOHop_MOVEGCM);

  /* SLARSEN: Vector extract ops */
  M_add_symbol(sym_tbl, PLAYDOHopcode_VEXTRSB, PLAYDOHop_VEXTRSB);
  M_add_symbol(sym_tbl, PLAYDOHopcode_VEXTRSH, PLAYDOHop_VEXTRSH);

  /* SLARSEN: Vector memory ops */
  M_add_symbol(sym_tbl, PLAYDOHopcode_VL_B_C1_C1, PLAYDOHop_VL_B_C1_C1);
  M_add_symbol(sym_tbl, PLAYDOHopcode_VL_H_C1_C1, PLAYDOHop_VL_H_C1_C1);
  M_add_symbol(sym_tbl, PLAYDOHopcode_VL_W_C1_C1, PLAYDOHop_VL_W_C1_C1);
  M_add_symbol(sym_tbl, PLAYDOHopcode_VFL_S_C1_C1, PLAYDOHop_VFL_S_C1_C1);
  M_add_symbol(sym_tbl, PLAYDOHopcode_VFL_D_C1_C1, PLAYDOHop_VFL_D_C1_C1);
  M_add_symbol(sym_tbl, PLAYDOHopcode_VS_B_C1, PLAYDOHop_VS_B_C1);
  M_add_symbol(sym_tbl, PLAYDOHopcode_VS_H_C1, PLAYDOHop_VS_H_C1);
  M_add_symbol(sym_tbl, PLAYDOHopcode_VS_W_C1, PLAYDOHop_VS_W_C1);
  M_add_symbol(sym_tbl, PLAYDOHopcode_VFS_S_C1, PLAYDOHop_VFS_S_C1);
  M_add_symbol(sym_tbl, PLAYDOHopcode_VFS_D_C1, PLAYDOHop_VFS_D_C1);

  M_add_symbol(sym_tbl, PLAYDOHopcode_VLE_B_C1_C1, PLAYDOHop_VLE_B_C1_C1);
  M_add_symbol(sym_tbl, PLAYDOHopcode_VLE_H_C1_C1, PLAYDOHop_VLE_H_C1_C1);
  M_add_symbol(sym_tbl, PLAYDOHopcode_VLE_W_C1_C1, PLAYDOHop_VLE_W_C1_C1);
  M_add_symbol(sym_tbl, PLAYDOHopcode_VFLE_S_C1_C1, PLAYDOHop_VFLE_S_C1_C1);
  M_add_symbol(sym_tbl, PLAYDOHopcode_VFLE_D_C1_C1, PLAYDOHop_VFLE_D_C1_C1);
  M_add_symbol(sym_tbl, PLAYDOHopcode_VSE_B_C1, PLAYDOHop_VSE_B_C1);
  M_add_symbol(sym_tbl, PLAYDOHopcode_VSE_H_C1, PLAYDOHop_VSE_H_C1);
  M_add_symbol(sym_tbl, PLAYDOHopcode_VSE_W_C1, PLAYDOHop_VSE_W_C1);
  M_add_symbol(sym_tbl, PLAYDOHopcode_VFSE_S_C1, PLAYDOHop_VFSE_S_C1);
  M_add_symbol(sym_tbl, PLAYDOHopcode_VFSE_D_C1, PLAYDOHop_VFSE_D_C1);
}

char *
M_get_opcode_name_playdoh (int id)
{
  switch (id)
    {
    case PLAYDOHop_PBRR:
      return (PLAYDOHopcode_PBRR);
    case PLAYDOHop_PBRA:
      return (PLAYDOHopcode_PBRA);
#if 0
    case PLAYDOHop_EXTRSB:
      return (PLAYDOHopcode_EXTRSB);
    case PLAYDOHop_EXTRSH:
      return (PLAYDOHopcode_EXTRSH);
#endif

    case PLAYDOHop_MOVEGF_L:
      return (PLAYDOHopcode_MOVEGF_L);
    case PLAYDOHop_MOVEGF_U:
      return (PLAYDOHopcode_MOVEGF_U);
    case PLAYDOHop_MOVEFG_L:
      return (PLAYDOHopcode_MOVEFG_L);
    case PLAYDOHop_MOVEFG_U:
      return (PLAYDOHopcode_MOVEFG_U);
    case PLAYDOHop_MOVEPG:
      return (PLAYDOHopcode_MOVEPG);
    case PLAYDOHop_LDCM:
      return (PLAYDOHopcode_LDCM);

    case PLAYDOHop_BRU:
      return (PLAYDOHopcode_BRU);
    case PLAYDOHop_BRCT:
      return (PLAYDOHopcode_BRCT);
    case PLAYDOHop_BRCF:
      return (PLAYDOHopcode_BRCF);
    case PLAYDOHop_BRL:
      return (PLAYDOHopcode_BRL);
    case PLAYDOHop_BRLC:
      return (PLAYDOHopcode_BRLC);

    case PLAYDOHop_BRF_B_B_F:
      return (PLAYDOHopcode_BRF_B_B_F);
    case PLAYDOHop_BRF_B_F_F:
      return (PLAYDOHopcode_BRF_B_F_F);
    case PLAYDOHop_BRF_F_B_B:
      return (PLAYDOHopcode_BRF_F_B_B);
    case PLAYDOHop_BRF_F_F_B:
      return (PLAYDOHopcode_BRF_F_F_B);
    case PLAYDOHop_BRF_F_F_F:
      return (PLAYDOHopcode_BRF_F_F_F);

    case PLAYDOHop_BRW_B_B_F:
      return (PLAYDOHopcode_BRW_B_B_F);
    case PLAYDOHop_BRW_B_F_F:
      return (PLAYDOHopcode_BRW_B_F_F);
    case PLAYDOHop_BRW_F_B_B:
      return (PLAYDOHopcode_BRW_F_B_B);
    case PLAYDOHop_BRW_F_F_B:
      return (PLAYDOHopcode_BRW_F_F_B);
    case PLAYDOHop_BRW_F_F_F:
      return (PLAYDOHopcode_BRW_F_F_F);

    case PLAYDOHop_BRDVI:
      return (PLAYDOHopcode_BRDVI);
    case PLAYDOHop_BRDVF:
      return (PLAYDOHopcode_BRDVF);

    case PLAYDOHop_L_B_V1_V1:
      return (PLAYDOHopcode_L_B_V1_V1);
    case PLAYDOHop_L_B_V1_C1:
      return (PLAYDOHopcode_L_B_V1_C1);
    case PLAYDOHop_L_B_V1_C2:
      return (PLAYDOHopcode_L_B_V1_C2);
    case PLAYDOHop_L_B_V1_C3:
      return (PLAYDOHopcode_L_B_V1_C3);
    case PLAYDOHop_L_B_C1_V1:
      return (PLAYDOHopcode_L_B_C1_V1);
    case PLAYDOHop_L_B_C1_C1:
      return (PLAYDOHopcode_L_B_C1_C1);
    case PLAYDOHop_L_B_C1_C2:
      return (PLAYDOHopcode_L_B_C1_C2);
    case PLAYDOHop_L_B_C1_C3:
      return (PLAYDOHopcode_L_B_C1_C3);
    case PLAYDOHop_L_B_C2_V1:
      return (PLAYDOHopcode_L_B_C2_V1);
    case PLAYDOHop_L_B_C2_C1:
      return (PLAYDOHopcode_L_B_C2_C1);
    case PLAYDOHop_L_B_C2_C2:
      return (PLAYDOHopcode_L_B_C2_C2);
    case PLAYDOHop_L_B_C2_C3:
      return (PLAYDOHopcode_L_B_C2_C3);
    case PLAYDOHop_L_B_C3_V1:
      return (PLAYDOHopcode_L_B_C3_V1);
    case PLAYDOHop_L_B_C3_C1:
      return (PLAYDOHopcode_L_B_C3_C1);
    case PLAYDOHop_L_B_C3_C2:
      return (PLAYDOHopcode_L_B_C3_C2);
    case PLAYDOHop_L_B_C3_C3:
      return (PLAYDOHopcode_L_B_C3_C3);
            
    case PLAYDOHop_L_H_V1_V1:
      return (PLAYDOHopcode_L_H_V1_V1);
    case PLAYDOHop_L_H_V1_C1:
      return (PLAYDOHopcode_L_H_V1_C1);
    case PLAYDOHop_L_H_V1_C2:
      return (PLAYDOHopcode_L_H_V1_C2);
    case PLAYDOHop_L_H_V1_C3:
      return (PLAYDOHopcode_L_H_V1_C3);
    case PLAYDOHop_L_H_C1_V1:
      return (PLAYDOHopcode_L_H_C1_V1);
    case PLAYDOHop_L_H_C1_C1:
      return (PLAYDOHopcode_L_H_C1_C1);
    case PLAYDOHop_L_H_C1_C2:
      return (PLAYDOHopcode_L_H_C1_C2);
    case PLAYDOHop_L_H_C1_C3:
      return (PLAYDOHopcode_L_H_C1_C3);
    case PLAYDOHop_L_H_C2_V1:
      return (PLAYDOHopcode_L_H_C2_V1);
    case PLAYDOHop_L_H_C2_C1:
      return (PLAYDOHopcode_L_H_C2_C1);
    case PLAYDOHop_L_H_C2_C2:
      return (PLAYDOHopcode_L_H_C2_C2);
    case PLAYDOHop_L_H_C2_C3:
      return (PLAYDOHopcode_L_H_C2_C3);
    case PLAYDOHop_L_H_C3_V1:
      return (PLAYDOHopcode_L_H_C3_V1);
    case PLAYDOHop_L_H_C3_C1:
      return (PLAYDOHopcode_L_H_C3_C1);
    case PLAYDOHop_L_H_C3_C2:
      return (PLAYDOHopcode_L_H_C3_C2);
    case PLAYDOHop_L_H_C3_C3:
      return (PLAYDOHopcode_L_H_C3_C3);

    case PLAYDOHop_L_W_V1_V1:
      return (PLAYDOHopcode_L_W_V1_V1);
    case PLAYDOHop_L_W_V1_C1:
      return (PLAYDOHopcode_L_W_V1_C1);
    case PLAYDOHop_L_W_V1_C2:
      return (PLAYDOHopcode_L_W_V1_C2);
    case PLAYDOHop_L_W_V1_C3:
      return (PLAYDOHopcode_L_W_V1_C3);
    case PLAYDOHop_L_W_C1_V1:
      return (PLAYDOHopcode_L_W_C1_V1);
    case PLAYDOHop_L_W_C1_C1:
      return (PLAYDOHopcode_L_W_C1_C1);
    case PLAYDOHop_L_W_C1_C2:
      return (PLAYDOHopcode_L_W_C1_C2);
    case PLAYDOHop_L_W_C1_C3:
      return (PLAYDOHopcode_L_W_C1_C3);
    case PLAYDOHop_L_W_C2_V1:
      return (PLAYDOHopcode_L_W_C2_V1);
    case PLAYDOHop_L_W_C2_C1:
      return (PLAYDOHopcode_L_W_C2_C1);
    case PLAYDOHop_L_W_C2_C2:
      return (PLAYDOHopcode_L_W_C2_C2);
    case PLAYDOHop_L_W_C2_C3:
      return (PLAYDOHopcode_L_W_C2_C3);
    case PLAYDOHop_L_W_C3_V1:
      return (PLAYDOHopcode_L_W_C3_V1);
    case PLAYDOHop_L_W_C3_C1:
      return (PLAYDOHopcode_L_W_C3_C1);
    case PLAYDOHop_L_W_C3_C2:
      return (PLAYDOHopcode_L_W_C3_C2);
    case PLAYDOHop_L_W_C3_C3:
      return (PLAYDOHopcode_L_W_C3_C3);

    case PLAYDOHop_L_Q_V1_V1:
      return (PLAYDOHopcode_L_Q_V1_V1);
    case PLAYDOHop_L_Q_V1_C1:
      return (PLAYDOHopcode_L_Q_V1_C1);
    case PLAYDOHop_L_Q_V1_C2:
      return (PLAYDOHopcode_L_Q_V1_C2);
    case PLAYDOHop_L_Q_V1_C3:
      return (PLAYDOHopcode_L_Q_V1_C3);
    case PLAYDOHop_L_Q_C1_V1:
      return (PLAYDOHopcode_L_Q_C1_V1);
    case PLAYDOHop_L_Q_C1_C1:
      return (PLAYDOHopcode_L_Q_C1_C1);
    case PLAYDOHop_L_Q_C1_C2:
      return (PLAYDOHopcode_L_Q_C1_C2);
    case PLAYDOHop_L_Q_C1_C3:
      return (PLAYDOHopcode_L_Q_C1_C3);
    case PLAYDOHop_L_Q_C2_V1:
      return (PLAYDOHopcode_L_Q_C2_V1);
    case PLAYDOHop_L_Q_C2_C1:
      return (PLAYDOHopcode_L_Q_C2_C1);
    case PLAYDOHop_L_Q_C2_C2:
      return (PLAYDOHopcode_L_Q_C2_C2);
    case PLAYDOHop_L_Q_C2_C3:
      return (PLAYDOHopcode_L_Q_C2_C3);
    case PLAYDOHop_L_Q_C3_V1:
      return (PLAYDOHopcode_L_Q_C3_V1);
    case PLAYDOHop_L_Q_C3_C1:
      return (PLAYDOHopcode_L_Q_C3_C1);
    case PLAYDOHop_L_Q_C3_C2:
      return (PLAYDOHopcode_L_Q_C3_C2);
    case PLAYDOHop_L_Q_C3_C3:
      return (PLAYDOHopcode_L_Q_C3_C3);

    case PLAYDOHop_LI_B_V1_V1:
      return (PLAYDOHopcode_LI_B_V1_V1);
    case PLAYDOHop_LI_B_V1_C1:
      return (PLAYDOHopcode_LI_B_V1_C1);
    case PLAYDOHop_LI_B_V1_C2:
      return (PLAYDOHopcode_LI_B_V1_C2);
    case PLAYDOHop_LI_B_V1_C3:
      return (PLAYDOHopcode_LI_B_V1_C3);
    case PLAYDOHop_LI_B_C1_V1:
      return (PLAYDOHopcode_LI_B_C1_V1);
    case PLAYDOHop_LI_B_C1_C1:
      return (PLAYDOHopcode_LI_B_C1_C1);
    case PLAYDOHop_LI_B_C1_C2:
      return (PLAYDOHopcode_LI_B_C1_C2);
    case PLAYDOHop_LI_B_C1_C3:
      return (PLAYDOHopcode_LI_B_C1_C3);
    case PLAYDOHop_LI_B_C2_V1:
      return (PLAYDOHopcode_LI_B_C2_V1);
    case PLAYDOHop_LI_B_C2_C1:
      return (PLAYDOHopcode_LI_B_C2_C1);
    case PLAYDOHop_LI_B_C2_C2:
      return (PLAYDOHopcode_LI_B_C2_C2);
    case PLAYDOHop_LI_B_C2_C3:
      return (PLAYDOHopcode_LI_B_C2_C3);
    case PLAYDOHop_LI_B_C3_V1:
      return (PLAYDOHopcode_LI_B_C3_V1);
    case PLAYDOHop_LI_B_C3_C1:
      return (PLAYDOHopcode_LI_B_C3_C1);
    case PLAYDOHop_LI_B_C3_C2:
      return (PLAYDOHopcode_LI_B_C3_C2);
    case PLAYDOHop_LI_B_C3_C3:
      return (PLAYDOHopcode_LI_B_C3_C3);

    case PLAYDOHop_LI_H_V1_V1:
      return (PLAYDOHopcode_LI_H_V1_V1);
    case PLAYDOHop_LI_H_V1_C1:
      return (PLAYDOHopcode_LI_H_V1_C1);
    case PLAYDOHop_LI_H_V1_C2:
      return (PLAYDOHopcode_LI_H_V1_C2);
    case PLAYDOHop_LI_H_V1_C3:
      return (PLAYDOHopcode_LI_H_V1_C3);
    case PLAYDOHop_LI_H_C1_V1:
      return (PLAYDOHopcode_LI_H_C1_V1);
    case PLAYDOHop_LI_H_C1_C1:
      return (PLAYDOHopcode_LI_H_C1_C1);
    case PLAYDOHop_LI_H_C1_C2:
      return (PLAYDOHopcode_LI_H_C1_C2);
    case PLAYDOHop_LI_H_C1_C3:
      return (PLAYDOHopcode_LI_H_C1_C3);
    case PLAYDOHop_LI_H_C2_V1:
      return (PLAYDOHopcode_LI_H_C2_V1);
    case PLAYDOHop_LI_H_C2_C1:
      return (PLAYDOHopcode_LI_H_C2_C1);
    case PLAYDOHop_LI_H_C2_C2:
      return (PLAYDOHopcode_LI_H_C2_C2);
    case PLAYDOHop_LI_H_C2_C3:
      return (PLAYDOHopcode_LI_H_C2_C3);
    case PLAYDOHop_LI_H_C3_V1:
      return (PLAYDOHopcode_LI_H_C3_V1);
    case PLAYDOHop_LI_H_C3_C1:
      return (PLAYDOHopcode_LI_H_C3_C1);
    case PLAYDOHop_LI_H_C3_C2:
      return (PLAYDOHopcode_LI_H_C3_C2);
    case PLAYDOHop_LI_H_C3_C3:
      return (PLAYDOHopcode_LI_H_C3_C3);

    case PLAYDOHop_LI_W_V1_V1:
      return (PLAYDOHopcode_LI_W_V1_V1);
    case PLAYDOHop_LI_W_V1_C1:
      return (PLAYDOHopcode_LI_W_V1_C1);
    case PLAYDOHop_LI_W_V1_C2:
      return (PLAYDOHopcode_LI_W_V1_C2);
    case PLAYDOHop_LI_W_V1_C3:
      return (PLAYDOHopcode_LI_W_V1_C3);
    case PLAYDOHop_LI_W_C1_V1:
      return (PLAYDOHopcode_LI_W_C1_V1);
    case PLAYDOHop_LI_W_C1_C1:
      return (PLAYDOHopcode_LI_W_C1_C1);
    case PLAYDOHop_LI_W_C1_C2:
      return (PLAYDOHopcode_LI_W_C1_C2);
    case PLAYDOHop_LI_W_C1_C3:
      return (PLAYDOHopcode_LI_W_C1_C3);
    case PLAYDOHop_LI_W_C2_V1:
      return (PLAYDOHopcode_LI_W_C2_V1);
    case PLAYDOHop_LI_W_C2_C1:
      return (PLAYDOHopcode_LI_W_C2_C1);
    case PLAYDOHop_LI_W_C2_C2:
      return (PLAYDOHopcode_LI_W_C2_C2);
    case PLAYDOHop_LI_W_C2_C3:
      return (PLAYDOHopcode_LI_W_C2_C3);
    case PLAYDOHop_LI_W_C3_V1:
      return (PLAYDOHopcode_LI_W_C3_V1);
    case PLAYDOHop_LI_W_C3_C1:
      return (PLAYDOHopcode_LI_W_C3_C1);
    case PLAYDOHop_LI_W_C3_C2:
      return (PLAYDOHopcode_LI_W_C3_C2);
    case PLAYDOHop_LI_W_C3_C3:
      return (PLAYDOHopcode_LI_W_C3_C3);

    case PLAYDOHop_LI_Q_V1_V1:
      return (PLAYDOHopcode_LI_Q_V1_V1);
    case PLAYDOHop_LI_Q_V1_C1:
      return (PLAYDOHopcode_LI_Q_V1_C1);
    case PLAYDOHop_LI_Q_V1_C2:
      return (PLAYDOHopcode_LI_Q_V1_C2);
    case PLAYDOHop_LI_Q_V1_C3:
      return (PLAYDOHopcode_LI_Q_V1_C3);
    case PLAYDOHop_LI_Q_C1_V1:
      return (PLAYDOHopcode_LI_Q_C1_V1);
    case PLAYDOHop_LI_Q_C1_C1:
      return (PLAYDOHopcode_LI_Q_C1_C1);
    case PLAYDOHop_LI_Q_C1_C2:
      return (PLAYDOHopcode_LI_Q_C1_C2);
    case PLAYDOHop_LI_Q_C1_C3:
      return (PLAYDOHopcode_LI_Q_C1_C3);
    case PLAYDOHop_LI_Q_C2_V1:
      return (PLAYDOHopcode_LI_Q_C2_V1);
    case PLAYDOHop_LI_Q_C2_C1:
      return (PLAYDOHopcode_LI_Q_C2_C1);
    case PLAYDOHop_LI_Q_C2_C2:
      return (PLAYDOHopcode_LI_Q_C2_C2);
    case PLAYDOHop_LI_Q_C2_C3:
      return (PLAYDOHopcode_LI_Q_C2_C3);
    case PLAYDOHop_LI_Q_C3_V1:
      return (PLAYDOHopcode_LI_Q_C3_V1);
    case PLAYDOHop_LI_Q_C3_C1:
      return (PLAYDOHopcode_LI_Q_C3_C1);
    case PLAYDOHop_LI_Q_C3_C2:
      return (PLAYDOHopcode_LI_Q_C3_C2);
    case PLAYDOHop_LI_Q_C3_C3:
      return (PLAYDOHopcode_LI_Q_C3_C3);

    case PLAYDOHop_S_B_V1:
      return (PLAYDOHopcode_S_B_V1);
    case PLAYDOHop_S_B_C1:
      return (PLAYDOHopcode_S_B_C1);
    case PLAYDOHop_S_B_C2:
      return (PLAYDOHopcode_S_B_C2);
    case PLAYDOHop_S_B_C3:
      return (PLAYDOHopcode_S_B_C3);

    case PLAYDOHop_S_H_V1:
      return (PLAYDOHopcode_S_H_V1);
    case PLAYDOHop_S_H_C1:
      return (PLAYDOHopcode_S_H_C1);
    case PLAYDOHop_S_H_C2:
      return (PLAYDOHopcode_S_H_C2);
    case PLAYDOHop_S_H_C3:
      return (PLAYDOHopcode_S_H_C3);

    case PLAYDOHop_S_W_V1:
      return (PLAYDOHopcode_S_W_V1);
    case PLAYDOHop_S_W_C1:
      return (PLAYDOHopcode_S_W_C1);
    case PLAYDOHop_S_W_C2:
      return (PLAYDOHopcode_S_W_C2);
    case PLAYDOHop_S_W_C3:
      return (PLAYDOHopcode_S_W_C3);

    case PLAYDOHop_S_Q_V1:
      return (PLAYDOHopcode_S_Q_V1);
    case PLAYDOHop_S_Q_C1:
      return (PLAYDOHopcode_S_Q_C1);
    case PLAYDOHop_S_Q_C2:
      return (PLAYDOHopcode_S_Q_C2);
    case PLAYDOHop_S_Q_C3:
      return (PLAYDOHopcode_S_Q_C3);

    case PLAYDOHop_SI_B_V1:
      return (PLAYDOHopcode_SI_B_V1);
    case PLAYDOHop_SI_B_C1:
      return (PLAYDOHopcode_SI_B_C1);
    case PLAYDOHop_SI_B_C2:
      return (PLAYDOHopcode_SI_B_C2);
    case PLAYDOHop_SI_B_C3:
      return (PLAYDOHopcode_SI_B_C3);

    case PLAYDOHop_SI_H_V1:
      return (PLAYDOHopcode_SI_H_V1);
    case PLAYDOHop_SI_H_C1:
      return (PLAYDOHopcode_SI_H_C1);
    case PLAYDOHop_SI_H_C2:
      return (PLAYDOHopcode_SI_H_C2);
    case PLAYDOHop_SI_H_C3:
      return (PLAYDOHopcode_SI_H_C3);

    case PLAYDOHop_SI_W_V1:
      return (PLAYDOHopcode_SI_W_V1);
    case PLAYDOHop_SI_W_C1:
      return (PLAYDOHopcode_SI_W_C1);
    case PLAYDOHop_SI_W_C2:
      return (PLAYDOHopcode_SI_W_C2);
    case PLAYDOHop_SI_W_C3:
      return (PLAYDOHopcode_SI_W_C3);

    case PLAYDOHop_SI_Q_V1:
      return (PLAYDOHopcode_SI_Q_V1);
    case PLAYDOHop_SI_Q_C1:
      return (PLAYDOHopcode_SI_Q_C1);
    case PLAYDOHop_SI_Q_C2:
      return (PLAYDOHopcode_SI_Q_C2);
    case PLAYDOHop_SI_Q_C3:
      return (PLAYDOHopcode_SI_Q_C3);

    case PLAYDOHop_FL_S_V1_V1:
      return (PLAYDOHopcode_FL_S_V1_V1);
    case PLAYDOHop_FL_S_V1_C1:
      return (PLAYDOHopcode_FL_S_V1_C1);
    case PLAYDOHop_FL_S_V1_C2:
      return (PLAYDOHopcode_FL_S_V1_C2);
    case PLAYDOHop_FL_S_V1_C3:
      return (PLAYDOHopcode_FL_S_V1_C3);
    case PLAYDOHop_FL_S_C1_V1:
      return (PLAYDOHopcode_FL_S_C1_V1);
    case PLAYDOHop_FL_S_C1_C1:
      return (PLAYDOHopcode_FL_S_C1_C1);
    case PLAYDOHop_FL_S_C1_C2:
      return (PLAYDOHopcode_FL_S_C1_C2);
    case PLAYDOHop_FL_S_C1_C3:
      return (PLAYDOHopcode_FL_S_C1_C3);
    case PLAYDOHop_FL_S_C2_V1:
      return (PLAYDOHopcode_FL_S_C2_V1);
    case PLAYDOHop_FL_S_C2_C1:
      return (PLAYDOHopcode_FL_S_C2_C1);
    case PLAYDOHop_FL_S_C2_C2:
      return (PLAYDOHopcode_FL_S_C2_C2);
    case PLAYDOHop_FL_S_C2_C3:
      return (PLAYDOHopcode_FL_S_C2_C3);
    case PLAYDOHop_FL_S_C3_V1:
      return (PLAYDOHopcode_FL_S_C3_V1);
    case PLAYDOHop_FL_S_C3_C1:
      return (PLAYDOHopcode_FL_S_C3_C1);
    case PLAYDOHop_FL_S_C3_C2:
      return (PLAYDOHopcode_FL_S_C3_C2);
    case PLAYDOHop_FL_S_C3_C3:
      return (PLAYDOHopcode_FL_S_C3_C3);

    case PLAYDOHop_FL_D_V1_V1:
      return (PLAYDOHopcode_FL_D_V1_V1);
    case PLAYDOHop_FL_D_V1_C1:
      return (PLAYDOHopcode_FL_D_V1_C1);
    case PLAYDOHop_FL_D_V1_C2:
      return (PLAYDOHopcode_FL_D_V1_C2);
    case PLAYDOHop_FL_D_V1_C3:
      return (PLAYDOHopcode_FL_D_V1_C3);
    case PLAYDOHop_FL_D_C1_V1:
      return (PLAYDOHopcode_FL_D_C1_V1);
    case PLAYDOHop_FL_D_C1_C1:
      return (PLAYDOHopcode_FL_D_C1_C1);
    case PLAYDOHop_FL_D_C1_C2:
      return (PLAYDOHopcode_FL_D_C1_C2);
    case PLAYDOHop_FL_D_C1_C3:
      return (PLAYDOHopcode_FL_D_C1_C3);
    case PLAYDOHop_FL_D_C2_V1:
      return (PLAYDOHopcode_FL_D_C2_V1);
    case PLAYDOHop_FL_D_C2_C1:
      return (PLAYDOHopcode_FL_D_C2_C1);
    case PLAYDOHop_FL_D_C2_C2:
      return (PLAYDOHopcode_FL_D_C2_C2);
    case PLAYDOHop_FL_D_C2_C3:
      return (PLAYDOHopcode_FL_D_C2_C3);
    case PLAYDOHop_FL_D_C3_V1:
      return (PLAYDOHopcode_FL_D_C3_V1);
    case PLAYDOHop_FL_D_C3_C1:
      return (PLAYDOHopcode_FL_D_C3_C1);
    case PLAYDOHop_FL_D_C3_C2:
      return (PLAYDOHopcode_FL_D_C3_C2);
    case PLAYDOHop_FL_D_C3_C3:
      return (PLAYDOHopcode_FL_D_C3_C3);

    case PLAYDOHop_FLI_S_V1_V1:
      return (PLAYDOHopcode_FLI_S_V1_V1);
    case PLAYDOHop_FLI_S_V1_C1:
      return (PLAYDOHopcode_FLI_S_V1_C1);
    case PLAYDOHop_FLI_S_V1_C2:
      return (PLAYDOHopcode_FLI_S_V1_C2);
    case PLAYDOHop_FLI_S_V1_C3:
      return (PLAYDOHopcode_FLI_S_V1_C3);
    case PLAYDOHop_FLI_S_C1_V1:
      return (PLAYDOHopcode_FLI_S_C1_V1);
    case PLAYDOHop_FLI_S_C1_C1:
      return (PLAYDOHopcode_FLI_S_C1_C1);
    case PLAYDOHop_FLI_S_C1_C2:
      return (PLAYDOHopcode_FLI_S_C1_C2);
    case PLAYDOHop_FLI_S_C1_C3:
      return (PLAYDOHopcode_FLI_S_C1_C3);
    case PLAYDOHop_FLI_S_C2_V1:
      return (PLAYDOHopcode_FLI_S_C2_V1);
    case PLAYDOHop_FLI_S_C2_C1:
      return (PLAYDOHopcode_FLI_S_C2_C1);
    case PLAYDOHop_FLI_S_C2_C2:
      return (PLAYDOHopcode_FLI_S_C2_C2);
    case PLAYDOHop_FLI_S_C2_C3:
      return (PLAYDOHopcode_FLI_S_C2_C3);
    case PLAYDOHop_FLI_S_C3_V1:
      return (PLAYDOHopcode_FLI_S_C3_V1);
    case PLAYDOHop_FLI_S_C3_C1:
      return (PLAYDOHopcode_FLI_S_C3_C1);
    case PLAYDOHop_FLI_S_C3_C2:
      return (PLAYDOHopcode_FLI_S_C3_C2);
    case PLAYDOHop_FLI_S_C3_C3:
      return (PLAYDOHopcode_FLI_S_C3_C3);

    case PLAYDOHop_FLI_D_V1_V1:
      return (PLAYDOHopcode_FLI_D_V1_V1);
    case PLAYDOHop_FLI_D_V1_C1:
      return (PLAYDOHopcode_FLI_D_V1_C1);
    case PLAYDOHop_FLI_D_V1_C2:
      return (PLAYDOHopcode_FLI_D_V1_C2);
    case PLAYDOHop_FLI_D_V1_C3:
      return (PLAYDOHopcode_FLI_D_V1_C3);
    case PLAYDOHop_FLI_D_C1_V1:
      return (PLAYDOHopcode_FLI_D_C1_V1);
    case PLAYDOHop_FLI_D_C1_C1:
      return (PLAYDOHopcode_FLI_D_C1_C1);
    case PLAYDOHop_FLI_D_C1_C2:
      return (PLAYDOHopcode_FLI_D_C1_C2);
    case PLAYDOHop_FLI_D_C1_C3:
      return (PLAYDOHopcode_FLI_D_C1_C3);
    case PLAYDOHop_FLI_D_C2_V1:
      return (PLAYDOHopcode_FLI_D_C2_V1);
    case PLAYDOHop_FLI_D_C2_C1:
      return (PLAYDOHopcode_FLI_D_C2_C1);
    case PLAYDOHop_FLI_D_C2_C2:
      return (PLAYDOHopcode_FLI_D_C2_C2);
    case PLAYDOHop_FLI_D_C2_C3:
      return (PLAYDOHopcode_FLI_D_C2_C3);
    case PLAYDOHop_FLI_D_C3_V1:
      return (PLAYDOHopcode_FLI_D_C3_V1);
    case PLAYDOHop_FLI_D_C3_C1:
      return (PLAYDOHopcode_FLI_D_C3_C1);
    case PLAYDOHop_FLI_D_C3_C2:
      return (PLAYDOHopcode_FLI_D_C3_C2);
    case PLAYDOHop_FLI_D_C3_C3:
      return (PLAYDOHopcode_FLI_D_C3_C3);

    case PLAYDOHop_FS_S_V1:
      return (PLAYDOHopcode_FS_S_V1);
    case PLAYDOHop_FS_S_C1:
      return (PLAYDOHopcode_FS_S_C1);
    case PLAYDOHop_FS_S_C2:
      return (PLAYDOHopcode_FS_S_C2);
    case PLAYDOHop_FS_S_C3:
      return (PLAYDOHopcode_FS_S_C3);

    case PLAYDOHop_FS_D_V1:
      return (PLAYDOHopcode_FS_D_V1);
    case PLAYDOHop_FS_D_C1:
      return (PLAYDOHopcode_FS_D_C1);
    case PLAYDOHop_FS_D_C2:
      return (PLAYDOHopcode_FS_D_C2);
    case PLAYDOHop_FS_D_C3:
      return (PLAYDOHopcode_FS_D_C3);

    case PLAYDOHop_FSI_S_V1:
      return (PLAYDOHopcode_FSI_S_V1);
    case PLAYDOHop_FSI_S_C1:
      return (PLAYDOHopcode_FSI_S_C1);
    case PLAYDOHop_FSI_S_C2:
      return (PLAYDOHopcode_FSI_S_C2);
    case PLAYDOHop_FSI_S_C3:
      return (PLAYDOHopcode_FSI_S_C3);

    case PLAYDOHop_FSI_D_V1:
      return (PLAYDOHopcode_FSI_D_V1);
    case PLAYDOHop_FSI_D_C1:
      return (PLAYDOHopcode_FSI_D_C1);
    case PLAYDOHop_FSI_D_C2:
      return (PLAYDOHopcode_FSI_D_C2);
    case PLAYDOHop_FSI_D_C3:
      return (PLAYDOHopcode_FSI_D_C3);

    case PLAYDOHop_LDS_B_V1_V1:
      return (PLAYDOHopcode_LDS_B_V1_V1);
    case PLAYDOHop_LDS_B_V1_C1:
      return (PLAYDOHopcode_LDS_B_V1_C1);
    case PLAYDOHop_LDS_B_V1_C2:
      return (PLAYDOHopcode_LDS_B_V1_C2);
    case PLAYDOHop_LDS_B_V1_C3:
      return (PLAYDOHopcode_LDS_B_V1_C3);
    case PLAYDOHop_LDS_B_C1_V1:
      return (PLAYDOHopcode_LDS_B_C1_V1);
    case PLAYDOHop_LDS_B_C1_C1:
      return (PLAYDOHopcode_LDS_B_C1_C1);
    case PLAYDOHop_LDS_B_C1_C2:
      return (PLAYDOHopcode_LDS_B_C1_C2);
    case PLAYDOHop_LDS_B_C1_C3:
      return (PLAYDOHopcode_LDS_B_C1_C3);
    case PLAYDOHop_LDS_B_C2_V1:
      return (PLAYDOHopcode_LDS_B_C2_V1);
    case PLAYDOHop_LDS_B_C2_C1:
      return (PLAYDOHopcode_LDS_B_C2_C1);
    case PLAYDOHop_LDS_B_C2_C2:
      return (PLAYDOHopcode_LDS_B_C2_C2);
    case PLAYDOHop_LDS_B_C2_C3:
      return (PLAYDOHopcode_LDS_B_C2_C3);
    case PLAYDOHop_LDS_B_C3_V1:
      return (PLAYDOHopcode_LDS_B_C3_V1);
    case PLAYDOHop_LDS_B_C3_C1:
      return (PLAYDOHopcode_LDS_B_C3_C1);
    case PLAYDOHop_LDS_B_C3_C2:
      return (PLAYDOHopcode_LDS_B_C3_C2);
    case PLAYDOHop_LDS_B_C3_C3:
      return (PLAYDOHopcode_LDS_B_C3_C3);

    case PLAYDOHop_LDS_H_V1_V1:
      return (PLAYDOHopcode_LDS_H_V1_V1);
    case PLAYDOHop_LDS_H_V1_C1:
      return (PLAYDOHopcode_LDS_H_V1_C1);
    case PLAYDOHop_LDS_H_V1_C2:
      return (PLAYDOHopcode_LDS_H_V1_C2);
    case PLAYDOHop_LDS_H_V1_C3:
      return (PLAYDOHopcode_LDS_H_V1_C3);
    case PLAYDOHop_LDS_H_C1_V1:
      return (PLAYDOHopcode_LDS_H_C1_V1);
    case PLAYDOHop_LDS_H_C1_C1:
      return (PLAYDOHopcode_LDS_H_C1_C1);
    case PLAYDOHop_LDS_H_C1_C2:
      return (PLAYDOHopcode_LDS_H_C1_C2);
    case PLAYDOHop_LDS_H_C1_C3:
      return (PLAYDOHopcode_LDS_H_C1_C3);
    case PLAYDOHop_LDS_H_C2_V1:
      return (PLAYDOHopcode_LDS_H_C2_V1);
    case PLAYDOHop_LDS_H_C2_C1:
      return (PLAYDOHopcode_LDS_H_C2_C1);
    case PLAYDOHop_LDS_H_C2_C2:
      return (PLAYDOHopcode_LDS_H_C2_C2);
    case PLAYDOHop_LDS_H_C2_C3:
      return (PLAYDOHopcode_LDS_H_C2_C3);
    case PLAYDOHop_LDS_H_C3_V1:
      return (PLAYDOHopcode_LDS_H_C3_V1);
    case PLAYDOHop_LDS_H_C3_C1:
      return (PLAYDOHopcode_LDS_H_C3_C1);
    case PLAYDOHop_LDS_H_C3_C2:
      return (PLAYDOHopcode_LDS_H_C3_C2);
    case PLAYDOHop_LDS_H_C3_C3:
      return (PLAYDOHopcode_LDS_H_C3_C3);

    case PLAYDOHop_LDS_W_V1_V1:
      return (PLAYDOHopcode_LDS_W_V1_V1);
    case PLAYDOHop_LDS_W_V1_C1:
      return (PLAYDOHopcode_LDS_W_V1_C1);
    case PLAYDOHop_LDS_W_V1_C2:
      return (PLAYDOHopcode_LDS_W_V1_C2);
    case PLAYDOHop_LDS_W_V1_C3:
      return (PLAYDOHopcode_LDS_W_V1_C3);
    case PLAYDOHop_LDS_W_C1_V1:
      return (PLAYDOHopcode_LDS_W_C1_V1);
    case PLAYDOHop_LDS_W_C1_C1:
      return (PLAYDOHopcode_LDS_W_C1_C1);
    case PLAYDOHop_LDS_W_C1_C2:
      return (PLAYDOHopcode_LDS_W_C1_C2);
    case PLAYDOHop_LDS_W_C1_C3:
      return (PLAYDOHopcode_LDS_W_C1_C3);
    case PLAYDOHop_LDS_W_C2_V1:
      return (PLAYDOHopcode_LDS_W_C2_V1);
    case PLAYDOHop_LDS_W_C2_C1:
      return (PLAYDOHopcode_LDS_W_C2_C1);
    case PLAYDOHop_LDS_W_C2_C2:
      return (PLAYDOHopcode_LDS_W_C2_C2);
    case PLAYDOHop_LDS_W_C2_C3:
      return (PLAYDOHopcode_LDS_W_C2_C3);
    case PLAYDOHop_LDS_W_C3_V1:
      return (PLAYDOHopcode_LDS_W_C3_V1);
    case PLAYDOHop_LDS_W_C3_C1:
      return (PLAYDOHopcode_LDS_W_C3_C1);
    case PLAYDOHop_LDS_W_C3_C2:
      return (PLAYDOHopcode_LDS_W_C3_C2);
    case PLAYDOHop_LDS_W_C3_C3:
      return (PLAYDOHopcode_LDS_W_C3_C3);

    case PLAYDOHop_LDS_Q_V1_V1:
      return (PLAYDOHopcode_LDS_Q_V1_V1);
    case PLAYDOHop_LDS_Q_V1_C1:
      return (PLAYDOHopcode_LDS_Q_V1_C1);
    case PLAYDOHop_LDS_Q_V1_C2:
      return (PLAYDOHopcode_LDS_Q_V1_C2);
    case PLAYDOHop_LDS_Q_V1_C3:
      return (PLAYDOHopcode_LDS_Q_V1_C3);
    case PLAYDOHop_LDS_Q_C1_V1:
      return (PLAYDOHopcode_LDS_Q_C1_V1);
    case PLAYDOHop_LDS_Q_C1_C1:
      return (PLAYDOHopcode_LDS_Q_C1_C1);
    case PLAYDOHop_LDS_Q_C1_C2:
      return (PLAYDOHopcode_LDS_Q_C1_C2);
    case PLAYDOHop_LDS_Q_C1_C3:
      return (PLAYDOHopcode_LDS_Q_C1_C3);
    case PLAYDOHop_LDS_Q_C2_V1:
      return (PLAYDOHopcode_LDS_Q_C2_V1);
    case PLAYDOHop_LDS_Q_C2_C1:
      return (PLAYDOHopcode_LDS_Q_C2_C1);
    case PLAYDOHop_LDS_Q_C2_C2:
      return (PLAYDOHopcode_LDS_Q_C2_C2);
    case PLAYDOHop_LDS_Q_C2_C3:
      return (PLAYDOHopcode_LDS_Q_C2_C3);
    case PLAYDOHop_LDS_Q_C3_V1:
      return (PLAYDOHopcode_LDS_Q_C3_V1);
    case PLAYDOHop_LDS_Q_C3_C1:
      return (PLAYDOHopcode_LDS_Q_C3_C1);
    case PLAYDOHop_LDS_Q_C3_C2:
      return (PLAYDOHopcode_LDS_Q_C3_C2);
    case PLAYDOHop_LDS_Q_C3_C3:
      return (PLAYDOHopcode_LDS_Q_C3_C3);

    case PLAYDOHop_LDSI_B_V1_V1:
      return (PLAYDOHopcode_LDSI_B_V1_V1);
    case PLAYDOHop_LDSI_B_V1_C1:
      return (PLAYDOHopcode_LDSI_B_V1_C1);
    case PLAYDOHop_LDSI_B_V1_C2:
      return (PLAYDOHopcode_LDSI_B_V1_C2);
    case PLAYDOHop_LDSI_B_V1_C3:
      return (PLAYDOHopcode_LDSI_B_V1_C3);
    case PLAYDOHop_LDSI_B_C1_V1:
      return (PLAYDOHopcode_LDSI_B_C1_V1);
    case PLAYDOHop_LDSI_B_C1_C1:
      return (PLAYDOHopcode_LDSI_B_C1_C1);
    case PLAYDOHop_LDSI_B_C1_C2:
      return (PLAYDOHopcode_LDSI_B_C1_C2);
    case PLAYDOHop_LDSI_B_C1_C3:
      return (PLAYDOHopcode_LDSI_B_C1_C3);
    case PLAYDOHop_LDSI_B_C2_V1:
      return (PLAYDOHopcode_LDSI_B_C2_V1);
    case PLAYDOHop_LDSI_B_C2_C1:
      return (PLAYDOHopcode_LDSI_B_C2_C1);
    case PLAYDOHop_LDSI_B_C2_C2:
      return (PLAYDOHopcode_LDSI_B_C2_C2);
    case PLAYDOHop_LDSI_B_C2_C3:
      return (PLAYDOHopcode_LDSI_B_C2_C3);
    case PLAYDOHop_LDSI_B_C3_V1:
      return (PLAYDOHopcode_LDSI_B_C3_V1);
    case PLAYDOHop_LDSI_B_C3_C1:
      return (PLAYDOHopcode_LDSI_B_C3_C1);
    case PLAYDOHop_LDSI_B_C3_C2:
      return (PLAYDOHopcode_LDSI_B_C3_C2);
    case PLAYDOHop_LDSI_B_C3_C3:
      return (PLAYDOHopcode_LDSI_B_C3_C3);

    case PLAYDOHop_LDSI_H_V1_V1:
      return (PLAYDOHopcode_LDSI_H_V1_V1);
    case PLAYDOHop_LDSI_H_V1_C1:
      return (PLAYDOHopcode_LDSI_H_V1_C1);
    case PLAYDOHop_LDSI_H_V1_C2:
      return (PLAYDOHopcode_LDSI_H_V1_C2);
    case PLAYDOHop_LDSI_H_V1_C3:
      return (PLAYDOHopcode_LDSI_H_V1_C3);
    case PLAYDOHop_LDSI_H_C1_V1:
      return (PLAYDOHopcode_LDSI_H_C1_V1);
    case PLAYDOHop_LDSI_H_C1_C1:
      return (PLAYDOHopcode_LDSI_H_C1_C1);
    case PLAYDOHop_LDSI_H_C1_C2:
      return (PLAYDOHopcode_LDSI_H_C1_C2);
    case PLAYDOHop_LDSI_H_C1_C3:
      return (PLAYDOHopcode_LDSI_H_C1_C3);
    case PLAYDOHop_LDSI_H_C2_V1:
      return (PLAYDOHopcode_LDSI_H_C2_V1);
    case PLAYDOHop_LDSI_H_C2_C1:
      return (PLAYDOHopcode_LDSI_H_C2_C1);
    case PLAYDOHop_LDSI_H_C2_C2:
      return (PLAYDOHopcode_LDSI_H_C2_C2);
    case PLAYDOHop_LDSI_H_C2_C3:
      return (PLAYDOHopcode_LDSI_H_C2_C3);
    case PLAYDOHop_LDSI_H_C3_V1:
      return (PLAYDOHopcode_LDSI_H_C3_V1);
    case PLAYDOHop_LDSI_H_C3_C1:
      return (PLAYDOHopcode_LDSI_H_C3_C1);
    case PLAYDOHop_LDSI_H_C3_C2:
      return (PLAYDOHopcode_LDSI_H_C3_C2);
    case PLAYDOHop_LDSI_H_C3_C3:
      return (PLAYDOHopcode_LDSI_H_C3_C3);

    case PLAYDOHop_LDSI_W_V1_V1:
      return (PLAYDOHopcode_LDSI_W_V1_V1);
    case PLAYDOHop_LDSI_W_V1_C1:
      return (PLAYDOHopcode_LDSI_W_V1_C1);
    case PLAYDOHop_LDSI_W_V1_C2:
      return (PLAYDOHopcode_LDSI_W_V1_C2);
    case PLAYDOHop_LDSI_W_V1_C3:
      return (PLAYDOHopcode_LDSI_W_V1_C3);
    case PLAYDOHop_LDSI_W_C1_V1:
      return (PLAYDOHopcode_LDSI_W_C1_V1);
    case PLAYDOHop_LDSI_W_C1_C1:
      return (PLAYDOHopcode_LDSI_W_C1_C1);
    case PLAYDOHop_LDSI_W_C1_C2:
      return (PLAYDOHopcode_LDSI_W_C1_C2);
    case PLAYDOHop_LDSI_W_C1_C3:
      return (PLAYDOHopcode_LDSI_W_C1_C3);
    case PLAYDOHop_LDSI_W_C2_V1:
      return (PLAYDOHopcode_LDSI_W_C2_V1);
    case PLAYDOHop_LDSI_W_C2_C1:
      return (PLAYDOHopcode_LDSI_W_C2_C1);
    case PLAYDOHop_LDSI_W_C2_C2:
      return (PLAYDOHopcode_LDSI_W_C2_C2);
    case PLAYDOHop_LDSI_W_C2_C3:
      return (PLAYDOHopcode_LDSI_W_C2_C3);
    case PLAYDOHop_LDSI_W_C3_V1:
      return (PLAYDOHopcode_LDSI_W_C3_V1);
    case PLAYDOHop_LDSI_W_C3_C1:
      return (PLAYDOHopcode_LDSI_W_C3_C1);
    case PLAYDOHop_LDSI_W_C3_C2:
      return (PLAYDOHopcode_LDSI_W_C3_C2);
    case PLAYDOHop_LDSI_W_C3_C3:
      return (PLAYDOHopcode_LDSI_W_C3_C3);

    case PLAYDOHop_LDSI_Q_V1_V1:
      return (PLAYDOHopcode_LDSI_Q_V1_V1);
    case PLAYDOHop_LDSI_Q_V1_C1:
      return (PLAYDOHopcode_LDSI_Q_V1_C1);
    case PLAYDOHop_LDSI_Q_V1_C2:
      return (PLAYDOHopcode_LDSI_Q_V1_C2);
    case PLAYDOHop_LDSI_Q_V1_C3:
      return (PLAYDOHopcode_LDSI_Q_V1_C3);
    case PLAYDOHop_LDSI_Q_C1_V1:
      return (PLAYDOHopcode_LDSI_Q_C1_V1);
    case PLAYDOHop_LDSI_Q_C1_C1:
      return (PLAYDOHopcode_LDSI_Q_C1_C1);
    case PLAYDOHop_LDSI_Q_C1_C2:
      return (PLAYDOHopcode_LDSI_Q_C1_C2);
    case PLAYDOHop_LDSI_Q_C1_C3:
      return (PLAYDOHopcode_LDSI_Q_C1_C3);
    case PLAYDOHop_LDSI_Q_C2_V1:
      return (PLAYDOHopcode_LDSI_Q_C2_V1);
    case PLAYDOHop_LDSI_Q_C2_C1:
      return (PLAYDOHopcode_LDSI_Q_C2_C1);
    case PLAYDOHop_LDSI_Q_C2_C2:
      return (PLAYDOHopcode_LDSI_Q_C2_C2);
    case PLAYDOHop_LDSI_Q_C2_C3:
      return (PLAYDOHopcode_LDSI_Q_C2_C3);
    case PLAYDOHop_LDSI_Q_C3_V1:
      return (PLAYDOHopcode_LDSI_Q_C3_V1);
    case PLAYDOHop_LDSI_Q_C3_C1:
      return (PLAYDOHopcode_LDSI_Q_C3_C1);
    case PLAYDOHop_LDSI_Q_C3_C2:
      return (PLAYDOHopcode_LDSI_Q_C3_C2);
    case PLAYDOHop_LDSI_Q_C3_C3:
      return (PLAYDOHopcode_LDSI_Q_C3_C3);

    case PLAYDOHop_FLDS_S_V1_V1:
      return (PLAYDOHopcode_FLDS_S_V1_V1);
    case PLAYDOHop_FLDS_S_V1_C1:
      return (PLAYDOHopcode_FLDS_S_V1_C1);
    case PLAYDOHop_FLDS_S_V1_C2:
      return (PLAYDOHopcode_FLDS_S_V1_C2);
    case PLAYDOHop_FLDS_S_V1_C3:
      return (PLAYDOHopcode_FLDS_S_V1_C3);
    case PLAYDOHop_FLDS_S_C1_V1:
      return (PLAYDOHopcode_FLDS_S_C1_V1);
    case PLAYDOHop_FLDS_S_C1_C1:
      return (PLAYDOHopcode_FLDS_S_C1_C1);
    case PLAYDOHop_FLDS_S_C1_C2:
      return (PLAYDOHopcode_FLDS_S_C1_C2);
    case PLAYDOHop_FLDS_S_C1_C3:
      return (PLAYDOHopcode_FLDS_S_C1_C3);
    case PLAYDOHop_FLDS_S_C2_V1:
      return (PLAYDOHopcode_FLDS_S_C2_V1);
    case PLAYDOHop_FLDS_S_C2_C1:
      return (PLAYDOHopcode_FLDS_S_C2_C1);
    case PLAYDOHop_FLDS_S_C2_C2:
      return (PLAYDOHopcode_FLDS_S_C2_C2);
    case PLAYDOHop_FLDS_S_C2_C3:
      return (PLAYDOHopcode_FLDS_S_C2_C3);
    case PLAYDOHop_FLDS_S_C3_V1:
      return (PLAYDOHopcode_FLDS_S_C3_V1);
    case PLAYDOHop_FLDS_S_C3_C1:
      return (PLAYDOHopcode_FLDS_S_C3_C1);
    case PLAYDOHop_FLDS_S_C3_C2:
      return (PLAYDOHopcode_FLDS_S_C3_C2);
    case PLAYDOHop_FLDS_S_C3_C3:
      return (PLAYDOHopcode_FLDS_S_C3_C3);

    case PLAYDOHop_FLDS_D_V1_V1:
      return (PLAYDOHopcode_FLDS_D_V1_V1);
    case PLAYDOHop_FLDS_D_V1_C1:
      return (PLAYDOHopcode_FLDS_D_V1_C1);
    case PLAYDOHop_FLDS_D_V1_C2:
      return (PLAYDOHopcode_FLDS_D_V1_C2);
    case PLAYDOHop_FLDS_D_V1_C3:
      return (PLAYDOHopcode_FLDS_D_V1_C3);
    case PLAYDOHop_FLDS_D_C1_V1:
      return (PLAYDOHopcode_FLDS_D_C1_V1);
    case PLAYDOHop_FLDS_D_C1_C1:
      return (PLAYDOHopcode_FLDS_D_C1_C1);
    case PLAYDOHop_FLDS_D_C1_C2:
      return (PLAYDOHopcode_FLDS_D_C1_C2);
    case PLAYDOHop_FLDS_D_C1_C3:
      return (PLAYDOHopcode_FLDS_D_C1_C3);
    case PLAYDOHop_FLDS_D_C2_V1:
      return (PLAYDOHopcode_FLDS_D_C2_V1);
    case PLAYDOHop_FLDS_D_C2_C1:
      return (PLAYDOHopcode_FLDS_D_C2_C1);
    case PLAYDOHop_FLDS_D_C2_C2:
      return (PLAYDOHopcode_FLDS_D_C2_C2);
    case PLAYDOHop_FLDS_D_C2_C3:
      return (PLAYDOHopcode_FLDS_D_C2_C3);
    case PLAYDOHop_FLDS_D_C3_V1:
      return (PLAYDOHopcode_FLDS_D_C3_V1);
    case PLAYDOHop_FLDS_D_C3_C1:
      return (PLAYDOHopcode_FLDS_D_C3_C1);
    case PLAYDOHop_FLDS_D_C3_C2:
      return (PLAYDOHopcode_FLDS_D_C3_C2);
    case PLAYDOHop_FLDS_D_C3_C3:
      return (PLAYDOHopcode_FLDS_D_C3_C3);

    case PLAYDOHop_FLDSI_S_V1_V1:
      return (PLAYDOHopcode_FLDSI_S_V1_V1);
    case PLAYDOHop_FLDSI_S_V1_C1:
      return (PLAYDOHopcode_FLDSI_S_V1_C1);
    case PLAYDOHop_FLDSI_S_V1_C2:
      return (PLAYDOHopcode_FLDSI_S_V1_C2);
    case PLAYDOHop_FLDSI_S_V1_C3:
      return (PLAYDOHopcode_FLDSI_S_V1_C3);
    case PLAYDOHop_FLDSI_S_C1_V1:
      return (PLAYDOHopcode_FLDSI_S_C1_V1);
    case PLAYDOHop_FLDSI_S_C1_C1:
      return (PLAYDOHopcode_FLDSI_S_C1_C1);
    case PLAYDOHop_FLDSI_S_C1_C2:
      return (PLAYDOHopcode_FLDSI_S_C1_C2);
    case PLAYDOHop_FLDSI_S_C1_C3:
      return (PLAYDOHopcode_FLDSI_S_C1_C3);
    case PLAYDOHop_FLDSI_S_C2_V1:
      return (PLAYDOHopcode_FLDSI_S_C2_V1);
    case PLAYDOHop_FLDSI_S_C2_C1:
      return (PLAYDOHopcode_FLDSI_S_C2_C1);
    case PLAYDOHop_FLDSI_S_C2_C2:
      return (PLAYDOHopcode_FLDSI_S_C2_C2);
    case PLAYDOHop_FLDSI_S_C2_C3:
      return (PLAYDOHopcode_FLDSI_S_C2_C3);
    case PLAYDOHop_FLDSI_S_C3_V1:
      return (PLAYDOHopcode_FLDSI_S_C3_V1);
    case PLAYDOHop_FLDSI_S_C3_C1:
      return (PLAYDOHopcode_FLDSI_S_C3_C1);
    case PLAYDOHop_FLDSI_S_C3_C2:
      return (PLAYDOHopcode_FLDSI_S_C3_C2);
    case PLAYDOHop_FLDSI_S_C3_C3:
      return (PLAYDOHopcode_FLDSI_S_C3_C3);

    case PLAYDOHop_FLDSI_D_V1_V1:
      return (PLAYDOHopcode_FLDSI_D_V1_V1);
    case PLAYDOHop_FLDSI_D_V1_C1:
      return (PLAYDOHopcode_FLDSI_D_V1_C1);
    case PLAYDOHop_FLDSI_D_V1_C2:
      return (PLAYDOHopcode_FLDSI_D_V1_C2);
    case PLAYDOHop_FLDSI_D_V1_C3:
      return (PLAYDOHopcode_FLDSI_D_V1_C3);
    case PLAYDOHop_FLDSI_D_C1_V1:
      return (PLAYDOHopcode_FLDSI_D_C1_V1);
    case PLAYDOHop_FLDSI_D_C1_C1:
      return (PLAYDOHopcode_FLDSI_D_C1_C1);
    case PLAYDOHop_FLDSI_D_C1_C2:
      return (PLAYDOHopcode_FLDSI_D_C1_C2);
    case PLAYDOHop_FLDSI_D_C1_C3:
      return (PLAYDOHopcode_FLDSI_D_C1_C3);
    case PLAYDOHop_FLDSI_D_C2_V1:
      return (PLAYDOHopcode_FLDSI_D_C2_V1);
    case PLAYDOHop_FLDSI_D_C2_C1:
      return (PLAYDOHopcode_FLDSI_D_C2_C1);
    case PLAYDOHop_FLDSI_D_C2_C2:
      return (PLAYDOHopcode_FLDSI_D_C2_C2);
    case PLAYDOHop_FLDSI_D_C2_C3:
      return (PLAYDOHopcode_FLDSI_D_C2_C3);
    case PLAYDOHop_FLDSI_D_C3_V1:
      return (PLAYDOHopcode_FLDSI_D_C3_V1);
    case PLAYDOHop_FLDSI_D_C3_C1:
      return (PLAYDOHopcode_FLDSI_D_C3_C1);
    case PLAYDOHop_FLDSI_D_C3_C2:
      return (PLAYDOHopcode_FLDSI_D_C3_C2);
    case PLAYDOHop_FLDSI_D_C3_C3:
      return (PLAYDOHopcode_FLDSI_D_C3_C3);

    case PLAYDOHop_LDV_B:
      return (PLAYDOHopcode_LDV_B);
    case PLAYDOHop_LDV_H:
      return (PLAYDOHopcode_LDV_H);
    case PLAYDOHop_LDV_W:
      return (PLAYDOHopcode_LDV_W);
    case PLAYDOHop_LDV_Q:
      return (PLAYDOHopcode_LDV_Q);
    case PLAYDOHop_FLDV_S:
      return (PLAYDOHopcode_FLDV_S);
    case PLAYDOHop_FLDV_D:
      return (PLAYDOHopcode_FLDV_D);

    case PLAYDOHop_SH1ADDL:
      return (PLAYDOHopcode_SH1ADDL);
    case PLAYDOHop_SH2ADDL:
      return (PLAYDOHopcode_SH2ADDL);
    case PLAYDOHop_SH3ADDL:
      return (PLAYDOHopcode_SH3ADDL);

    case PLAYDOHop_FMPYADDN_S:
      return (PLAYDOHopcode_FMPYADDN_S);
    case PLAYDOHop_FMPYADDN_D:
      return (PLAYDOHopcode_FMPYADDN_D);

    case PLAYDOHop_CMPR_FALSE:
      return (PLAYDOHopcode_CMPR_FALSE);
    case PLAYDOHop_CMPR_TRUE:
      return (PLAYDOHopcode_CMPR_TRUE);
    case PLAYDOHop_CMPR_OD:
      return (PLAYDOHopcode_CMPR_OD);
    case PLAYDOHop_CMPR_EV:
      return (PLAYDOHopcode_CMPR_EV);

    case PLAYDOHop_FCMPR_S_FALSE:
      return (PLAYDOHopcode_FCMPR_S_FALSE);
    case PLAYDOHop_FCMPR_S_TRUE:
      return (PLAYDOHopcode_FCMPR_S_TRUE);
    case PLAYDOHop_FCMPR_D_FALSE:
      return (PLAYDOHopcode_FCMPR_D_FALSE);
    case PLAYDOHop_FCMPR_D_TRUE:
      return (PLAYDOHopcode_FCMPR_D_TRUE);

    case PLAYDOHop_M_NO_OP:
      return (PLAYDOHopcode_M_NO_OP);

    case PLAYDOHop_MOVELB:
      return (PLAYDOHopcode_MOVELB);
    case PLAYDOHop_MOVELBX:
      return (PLAYDOHopcode_MOVELBX);
    case PLAYDOHop_MOVELBS:
      return (PLAYDOHopcode_MOVELBS);
    case PLAYDOHop_PBRRL:
      return (PLAYDOHopcode_PBRRL);
    case PLAYDOHop_PBRAL:
      return (PLAYDOHopcode_PBRAL);
    case PLAYDOHop_PBRRLBS:
      return (PLAYDOHopcode_PBRRLBS);
    case PLAYDOHop_PBRALBS:
      return (PLAYDOHopcode_PBRALBS);
    case PLAYDOHop_MOVELG:
      return (PLAYDOHopcode_MOVELG);
    case PLAYDOHop_MOVELGX:
      return (PLAYDOHopcode_MOVELGX);
    case PLAYDOHop_MOVELGS:
      return (PLAYDOHopcode_MOVELGS);
    case PLAYDOHop_MOVELF:
      return (PLAYDOHopcode_MOVELF);
    case PLAYDOHop_MOVELFS:
      return (PLAYDOHopcode_MOVELFS);

    case PLAYDOHop_MOVEGC:
      return (PLAYDOHopcode_MOVEGC);
    case PLAYDOHop_MOVECG:
      return (PLAYDOHopcode_MOVECG);
    case PLAYDOHop_MOVEGG:
      return (PLAYDOHopcode_MOVEGG);
    case PLAYDOHop_MOVEBB:
      return (PLAYDOHopcode_MOVEBB);

    case PLAYDOHop_SAVE:
      return (PLAYDOHopcode_SAVE);
    case PLAYDOHop_RESTORE:
      return (PLAYDOHopcode_RESTORE);
    case PLAYDOHop_FSAVE_S:
      return (PLAYDOHopcode_FSAVE_S);
    case PLAYDOHop_FRESTORE_S:
      return (PLAYDOHopcode_FRESTORE_S);
    case PLAYDOHop_FSAVE_D:
      return (PLAYDOHopcode_FSAVE_D);
    case PLAYDOHop_FRESTORE_D:
      return (PLAYDOHopcode_FRESTORE_D);
    case PLAYDOHop_BSAVE:
      return (PLAYDOHopcode_BSAVE);
    case PLAYDOHop_BRESTORE:
      return (PLAYDOHopcode_BRESTORE);
    case PLAYDOHop_MOVEGBP:
      return (PLAYDOHopcode_MOVEGBP);
    case PLAYDOHop_MOVEGCM:
      return (PLAYDOHopcode_MOVEGCM);

      /* SLARSEN: Vector extract ops */
      case PLAYDOHop_VEXTRSB:		return (PLAYDOHopcode_VEXTRSB);
      case PLAYDOHop_VEXTRSH:		return (PLAYDOHopcode_VEXTRSH);

        /* SLARSEN: Vector memory ops */
      case PLAYDOHop_VL_B_C1_C1:	return (PLAYDOHopcode_VL_B_C1_C1);
      case PLAYDOHop_VL_H_C1_C1:	return (PLAYDOHopcode_VL_H_C1_C1);
      case PLAYDOHop_VL_W_C1_C1:	return (PLAYDOHopcode_VL_W_C1_C1);
      case PLAYDOHop_VFL_S_C1_C1:	return (PLAYDOHopcode_VFL_S_C1_C1);
      case PLAYDOHop_VFL_D_C1_C1:	return (PLAYDOHopcode_VFL_D_C1_C1);
      case PLAYDOHop_VS_B_C1:		return (PLAYDOHopcode_VS_B_C1);
      case PLAYDOHop_VS_H_C1:		return (PLAYDOHopcode_VS_H_C1);
      case PLAYDOHop_VS_W_C1:		return (PLAYDOHopcode_VS_W_C1);
      case PLAYDOHop_VFS_S_C1:	return (PLAYDOHopcode_VFS_S_C1);
      case PLAYDOHop_VFS_D_C1:	return (PLAYDOHopcode_VFS_D_C1);
	
      case PLAYDOHop_VLE_B_C1_C1:	return (PLAYDOHopcode_VLE_B_C1_C1);
      case PLAYDOHop_VLE_H_C1_C1:	return (PLAYDOHopcode_VLE_H_C1_C1);
      case PLAYDOHop_VLE_W_C1_C1:	return (PLAYDOHopcode_VLE_W_C1_C1);
      case PLAYDOHop_VFLE_S_C1_C1:	return (PLAYDOHopcode_VFLE_S_C1_C1);
      case PLAYDOHop_VFLE_D_C1_C1:	return (PLAYDOHopcode_VFLE_D_C1_C1);
      case PLAYDOHop_VSE_B_C1:	return (PLAYDOHopcode_VSE_B_C1);
      case PLAYDOHop_VSE_H_C1:	return (PLAYDOHopcode_VSE_H_C1);
      case PLAYDOHop_VSE_W_C1:	return (PLAYDOHopcode_VSE_W_C1);
      case PLAYDOHop_VFSE_S_C1:	return (PLAYDOHopcode_VFSE_S_C1);
      case PLAYDOHop_VFSE_D_C1:	return (PLAYDOHopcode_VFSE_D_C1);

    default:
      return ("?");
    }
}

/*--------------------------------------------------------------------------*/
/*
 * Return true (1) if the instruction is supported in the hardware of the
 * processor.  Return false (0) otherwise.
 */
int
M_oper_supported_in_arch_playdoh (int opc)
{
  if (M_playdoh_model == M_PLAYDOH_V1)
    {
      switch (opc)
	{
	case Lop_OR_NOT:
	case Lop_AND_NOT:
	  return (0);

	case Lop_ABS:
	case Lop_NAND:
	case Lop_NOR:
	case Lop_NXOR:
	case Lop_AND_COMPL:
	case Lop_OR_COMPL:
	case Lop_ABS_F:
	case Lop_ABS_F2:
	case Lop_MUL_ADD_F:
	case Lop_MUL_ADD_F2:
	case Lop_MUL_SUB_F:
	case Lop_MUL_SUB_F2:
	case Lop_MUL_SUB_REV_F:
	case Lop_MUL_SUB_REV_F2:
	case Lop_SQRT_F:
	case Lop_SQRT_F2:
	case Lop_MIN_F:
	case Lop_MIN_F2:
	case Lop_MAX_F:
	case Lop_MAX_F2:
	case Lop_RCP_F:
	case Lop_RCP_F2:
	  return (1);

	default:
	  return (1);
	}
    }
  else
    {
      M_assert (0, "M_oper_supported_in_arch_playdoh: illegal machine model");
      return (0);
    }
}

/*
 * Returns the number of machine instructions required to implement the
 * specified oper in the best case.  It is assumed that this is for
 * supported instructions.  A call to M_oper_supported_in_arch should be
 * made for abnormal instructions.
 */
int
M_num_oper_required_for_playdoh (L_Oper * oper, char *name)
{
#define zero_offset(a)          ((a->src[1]->type==L_OPERAND_IMMED) &&\
                                 ((a->src[1]->ctype&0x30)==0x00)&&\
    				 (a->src[1]->value.i == 0))


  if (M_playdoh_model == M_PLAYDOH_V1)
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
	case Lop_ST_Q:
	  if (indexed_memory_op (oper) || has_label_operand (oper))
	    return (2);
	  else
	    return (1);

	case Lop_LD_UC:
	case Lop_LD_C:
	case Lop_LD_UC2:
	case Lop_LD_C2:
	case Lop_LD_I:
	case Lop_LD_Q:
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

	case Lop_LD_POST_UC:
	case Lop_LD_POST_C:
	case Lop_LD_POST_UC2:
	case Lop_LD_POST_C2:
	case Lop_LD_POST_I:
	case Lop_LD_POST_Q:
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
	case Lop_ST_POST_Q:
	  if (indexed_memory_op (oper) || non_zero_offset (oper))
	    return (2);
	  else if (short_int_inc (oper, 3) || long_pos_int_inc (oper, 3))
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

	case Lop_LD_PRE_UC:
	case Lop_LD_PRE_C:
	case Lop_LD_PRE_UC2:
	case Lop_LD_PRE_C2:
	case Lop_LD_PRE_I:
	case Lop_LD_PRE_Q:
	case Lop_LD_PRE_F:
	case Lop_LD_PRE_F2:
	case Lop_ST_PRE_C:
	case Lop_ST_PRE_C2:
	case Lop_ST_PRE_I:
	case Lop_ST_PRE_Q:
	case Lop_ST_PRE_F:
	case Lop_ST_PRE_F2:
	  return (2);


	default:
	  return (1);
	}
    }
  else
    {
      M_assert (0, "M_num_oper_required_for_playdoh: illegal machine model");
      return (0);
    }
}

int
M_num_registers_playdoh (int ctype)
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

int
M_is_stack_operand_playdoh (L_Operand * operand)
{
  if (M_playdoh_model == M_PLAYDOH_V1)
    {

      if (L_is_macro (operand) &&
	  (operand->value.mac == L_MAC_SP ||
	   operand->value.mac == L_MAC_FP ||
	   operand->value.mac == L_MAC_SAFE_MEM ||
	   operand->value.mac == L_MAC_P12 ||
	   operand->value.mac == L_MAC_LV ||
	   operand->value.mac == L_MAC_RS ||
	   operand->value.mac == L_MAC_IP || operand->value.mac == L_MAC_OP))
	return (1);

      return (0);
    }
  else
    {
      M_assert (0, "M_is_stack_operand_playdoh: illegal machine model");
      return (0);
    }
}

int
M_is_unsafe_macro_playdoh (L_Operand * operand)
{
  if (M_playdoh_model == M_PLAYDOH_V1)
    {
      return (0);
    }
  else
    {
      M_assert (0, "M_is_unsafe_macro_playdoh: illegal machine model");
      return (0);
    }
}

int
M_operand_type_playdoh (L_Operand * operand)
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
      M_assert (0, "M_operand_type_playdoh: Unknown type");
      return (0);
    }
}

/* Mechanism for playdoh to specify the offset between the left float
 * register numer and the double register number.  Typically, this
 * offset should be set to num_flt_callee_reg in playdoh. -JCG 6/99
 */
int M_playdoh_dbl_offset = 0;

/* Conflicting operand functions for impact ver1 and hppa models */
/*
 *      Structure of assumed register file for IMPACT machine, so this
 *	is what is used after register allocation for the IMPACT machine
 *
 *      int registers
 *              1
 *              ...
 *              playdoh_num_int_reg (required to be an even #)
 *
 *      flt/dbl registers (double == consecective float regs)
 *      (double reg number is left float + M_playdoh_dbl_offset -JCG 6/99)
 *              playdoh_num_int_reg+1   playdoh_num_int_reg+2
 *              ...                     ...
 *              playdoh_num_flt_reg+M_playdoh_dbl_offset-1   
 *              playdoh_num_flt_reg+M_playdoh_dbl_offset
 *      Note: Doubles use only odd register numbers after allocation! -JCG 6/99
 *
 *      prd registrs
 *              playdoh_num_flt_reg
 *              ...
 *              playdoh_num_prd_reg
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
 *      Note for now, handle all macros so can run playdoh on all Lcodes
 *      for any target machine.
 */

int
M_conflicting_operands_playdoh (L_Operand * operand,
				L_Operand ** conflict_array, int len,
				int prepass)
{
  if (M_playdoh_model == M_PLAYDOH_V1) {
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
		      "playdoh_conflicting_operands: illegal operand type "
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
	/* Made match playdoh's register bank definitions -JCG 6/99 */
	else if (L_is_ctype_dbl (operand))
	  {
	    /* Sanity check, double register operand number must be odd
	     * after register allocation!
	     */
	    if ((operand->value.r & 0x1) == 0)
	      {
		M_assert (0,
			  "playdoh_conflicting_operands: "
			  "illegal even double regsiter after regalloc!");
	      }

	    /* Set up conflicting double register */
	    conflict_array[i++] = L_copy_operand (operand);

	    /* Set up conflicting float registers. */
	    /* Left float reg # + M_playdoh_dbl_offset  = double reg # */
	    conflict_array[i] = L_copy_operand (operand);
	    conflict_array[i]->ctype = L_CTYPE_FLOAT;
	    conflict_array[i++]->value.r =
	      operand->value.r - M_playdoh_dbl_offset;

	    /* Right float reg # + M_playdoh_dbl_offset - 1 = double reg # */
	    conflict_array[i] = L_copy_operand (operand);
	    conflict_array[i]->ctype = L_CTYPE_FLOAT;
	    conflict_array[i++]->value.r =
	      (operand->value.r - M_playdoh_dbl_offset) + 1;

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
	/* Made match playdoh's register bank definitions -JCG 6/99 */
	else if (L_is_ctype_flt (operand))
	  {
	    /* If left register */
	    if (operand->value.r & 0x1)
	      {
		/* Set up conflicting float register */
		conflict_array[i++] = L_copy_operand (operand);

		/* Set up conflicting double register */
		/* Left float reg # + M_playdoh_dbl_offset  = double reg # */
		conflict_array[i] = L_copy_operand (operand);
		conflict_array[i]->ctype = L_CTYPE_DOUBLE;
		conflict_array[i++]->value.r =
		  operand->value.r + M_playdoh_dbl_offset;

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
		/* Right float reg # + M_playdoh_dbl_offset - 1 =double reg # */
		conflict_array[i] = L_copy_operand (operand);
		conflict_array[i]->ctype = L_CTYPE_DOUBLE;
		conflict_array[i++]->value.r =
		  operand->value.r + M_playdoh_dbl_offset - 1;

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
	    M_assert (0, "playdoh_conflicting_operands: illegal reg number");
	  }
      }
    else if (L_is_macro (operand))
      {
	conflict_array[i++] = L_copy_operand (operand);
      }
    else
      {
	printf("%d %d\n",i,len);
	M_assert (0, "M_conflicting_operands_playdoh: unsupported operand\n");
      }

    if (i > len)
      {
	printf("%d %d\n",i,len);
	M_assert (0,"M_conflicting_operands_playdoh: too many conflicts\n");
      }

    return (i);
  }
  else
    {
      M_assert (0, "M_conflicting_operands_playdoh: illegal machine model");
      return (0);
    }
}

/*
 *	Return the opc used to represent the proc_opc in Lcode.  This
 *	function is called if the user knows how to create proc_opcs,
 *	but does not know how that opcode will eventually be stored
 *	in Lcode using the opc/proc_opc combo.
 *
 *	-1's return means I wasn't sure how we should represent them
 *	in Lcode currently, need Ricks input to agree on the correct Lop.
 */
int
M_opc_from_proc_opc_playdoh (int proc_opc)
{
  switch (proc_opc)
    {
    case PLAYDOHop_PBRR:
      return (Lop_PBR);
    case PLAYDOHop_PBRA:
      return (Lop_PBR);
#if 0
    case PLAYDOHop_EXTRSB:
      return (Lop_EXTRACT_C);
    case PLAYDOHop_EXTRSH:
      return (Lop_EXTRACT_C2);
#endif

    case PLAYDOHop_MOVEGF_L:
      return (-1);
    case PLAYDOHop_MOVEGF_U:
      return (-1);
    case PLAYDOHop_MOVEFG_L:
      return (-1);
    case PLAYDOHop_MOVEFG_U:
      return (-1);
    case PLAYDOHop_MOVEPG:
      return (Lop_MOV);
    case PLAYDOHop_LDCM:
      return (-1);

    case PLAYDOHop_BRU:
      return (Lop_JUMP);
    case PLAYDOHop_BRCT:
      return (Lop_BR);
    case PLAYDOHop_BRCF:
      return (Lop_BR);
    case PLAYDOHop_BRL:
      return (Lop_JSR);
    case PLAYDOHop_BRLC:
      return (Lop_BR);

    case PLAYDOHop_BRF_B_B_F:
      return (Lop_BR);
    case PLAYDOHop_BRF_B_F_F:
      return (Lop_BR);
    case PLAYDOHop_BRF_F_B_B:
      return (Lop_BR);
    case PLAYDOHop_BRF_F_F_B:
      return (Lop_BR);
    case PLAYDOHop_BRF_F_F_F:
      return (Lop_BR);

    case PLAYDOHop_BRW_B_B_F:
      return (Lop_BR);
    case PLAYDOHop_BRW_B_F_F:
      return (Lop_BR);
    case PLAYDOHop_BRW_F_B_B:
      return (Lop_BR);
    case PLAYDOHop_BRW_F_F_B:
      return (Lop_BR);
    case PLAYDOHop_BRW_F_F_F:
      return (Lop_BR);

      /* TLJ 8/14/96 - use the CHECK operation temporarily
       * so I can avoid creating the compensation code block
       * for the advanced load. Eventually use Lop_BEQ,
       * when there is a cb created for the BRDV to jump to.
       */
    case PLAYDOHop_BRDVI:
      return (Lop_CHECK);
    case PLAYDOHop_BRDVF:
      return (Lop_CHECK);

    case PLAYDOHop_L_B_V1_V1:
      return (Lop_LD_UC);
    case PLAYDOHop_L_B_V1_C1:
      return (Lop_LD_UC);
    case PLAYDOHop_L_B_V1_C2:
      return (Lop_LD_UC);
    case PLAYDOHop_L_B_V1_C3:
      return (Lop_LD_UC);
    case PLAYDOHop_L_B_C1_V1:
      return (Lop_LD_UC);
    case PLAYDOHop_L_B_C1_C1:
      return (Lop_LD_UC);
    case PLAYDOHop_L_B_C1_C2:
      return (Lop_LD_UC);
    case PLAYDOHop_L_B_C1_C3:
      return (Lop_LD_UC);
    case PLAYDOHop_L_B_C2_V1:
      return (Lop_LD_UC);
    case PLAYDOHop_L_B_C2_C1:
      return (Lop_LD_UC);
    case PLAYDOHop_L_B_C2_C2:
      return (Lop_LD_UC);
    case PLAYDOHop_L_B_C2_C3:
      return (Lop_LD_UC);
    case PLAYDOHop_L_B_C3_V1:
      return (Lop_LD_UC);
    case PLAYDOHop_L_B_C3_C1:
      return (Lop_LD_UC);
    case PLAYDOHop_L_B_C3_C2:
      return (Lop_LD_UC);
    case PLAYDOHop_L_B_C3_C3:
      return (Lop_LD_UC);

    case PLAYDOHop_LG_B_V1_V1:
      return (Lop_LD_UC);
    case PLAYDOHop_LG_B_V1_C1:
      return (Lop_LD_UC);
    case PLAYDOHop_LG_B_V1_C2:
      return (Lop_LD_UC);
    case PLAYDOHop_LG_B_V1_C3:
      return (Lop_LD_UC);
    case PLAYDOHop_LG_B_C1_V1:
      return (Lop_LD_UC);
    case PLAYDOHop_LG_B_C1_C1:
      return (Lop_LD_UC);
    case PLAYDOHop_LG_B_C1_C2:
      return (Lop_LD_UC);
    case PLAYDOHop_LG_B_C1_C3:
      return (Lop_LD_UC);
    case PLAYDOHop_LG_B_C2_V1:
      return (Lop_LD_UC);
    case PLAYDOHop_LG_B_C2_C1:
      return (Lop_LD_UC);
    case PLAYDOHop_LG_B_C2_C2:
      return (Lop_LD_UC);
    case PLAYDOHop_LG_B_C2_C3:
      return (Lop_LD_UC);
    case PLAYDOHop_LG_B_C3_V1:
      return (Lop_LD_UC);
    case PLAYDOHop_LG_B_C3_C1:
      return (Lop_LD_UC);
    case PLAYDOHop_LG_B_C3_C2:
      return (Lop_LD_UC);
    case PLAYDOHop_LG_B_C3_C3:
      return (Lop_LD_UC);

    case PLAYDOHop_LM_B_V1_V1:
      return (Lop_LD_UC);
    case PLAYDOHop_LM_B_V1_C1:
      return (Lop_LD_UC);
    case PLAYDOHop_LM_B_V1_C2:
      return (Lop_LD_UC);
    case PLAYDOHop_LM_B_V1_C3:
      return (Lop_LD_UC);
    case PLAYDOHop_LM_B_C1_V1:
      return (Lop_LD_UC);
    case PLAYDOHop_LM_B_C1_C1:
      return (Lop_LD_UC);
    case PLAYDOHop_LM_B_C1_C2:
      return (Lop_LD_UC);
    case PLAYDOHop_LM_B_C1_C3:
      return (Lop_LD_UC);
    case PLAYDOHop_LM_B_C2_V1:
      return (Lop_LD_UC);
    case PLAYDOHop_LM_B_C2_C1:
      return (Lop_LD_UC);
    case PLAYDOHop_LM_B_C2_C2:
      return (Lop_LD_UC);
    case PLAYDOHop_LM_B_C2_C3:
      return (Lop_LD_UC);
    case PLAYDOHop_LM_B_C3_V1:
      return (Lop_LD_UC);
    case PLAYDOHop_LM_B_C3_C1:
      return (Lop_LD_UC);
    case PLAYDOHop_LM_B_C3_C2:
      return (Lop_LD_UC);
    case PLAYDOHop_LM_B_C3_C3:
      return (Lop_LD_UC);

    case PLAYDOHop_LX_B_V1_V1:
      return (Lop_LD_C);
    case PLAYDOHop_LX_B_V1_C1:
      return (Lop_LD_C);
    case PLAYDOHop_LX_B_V1_C2:
      return (Lop_LD_C);
    case PLAYDOHop_LX_B_V1_C3:
      return (Lop_LD_C);
    case PLAYDOHop_LX_B_C1_V1:
      return (Lop_LD_C);
    case PLAYDOHop_LX_B_C1_C1:
      return (Lop_LD_C);
    case PLAYDOHop_LX_B_C1_C2:
      return (Lop_LD_C);
    case PLAYDOHop_LX_B_C1_C3:
      return (Lop_LD_C);
    case PLAYDOHop_LX_B_C2_V1:
      return (Lop_LD_C);
    case PLAYDOHop_LX_B_C2_C1:
      return (Lop_LD_C);
    case PLAYDOHop_LX_B_C2_C2:
      return (Lop_LD_C);
    case PLAYDOHop_LX_B_C2_C3:
      return (Lop_LD_C);
    case PLAYDOHop_LX_B_C3_V1:
      return (Lop_LD_C);
    case PLAYDOHop_LX_B_C3_C1:
      return (Lop_LD_C);
    case PLAYDOHop_LX_B_C3_C2:
      return (Lop_LD_C);
    case PLAYDOHop_LX_B_C3_C3:
      return (Lop_LD_C);
    
    case PLAYDOHop_LGX_B_V1_V1:
      return (Lop_LD_C);
    case PLAYDOHop_LGX_B_V1_C1:
      return (Lop_LD_C);
    case PLAYDOHop_LGX_B_V1_C2:
      return (Lop_LD_C);
    case PLAYDOHop_LGX_B_V1_C3:
      return (Lop_LD_C);
    case PLAYDOHop_LGX_B_C1_V1:
      return (Lop_LD_C);
    case PLAYDOHop_LGX_B_C1_C1:
      return (Lop_LD_C);
    case PLAYDOHop_LGX_B_C1_C2:
      return (Lop_LD_C);
    case PLAYDOHop_LGX_B_C1_C3:
      return (Lop_LD_C);
    case PLAYDOHop_LGX_B_C2_V1:
      return (Lop_LD_C);
    case PLAYDOHop_LGX_B_C2_C1:
      return (Lop_LD_C);
    case PLAYDOHop_LGX_B_C2_C2:
      return (Lop_LD_C);
    case PLAYDOHop_LGX_B_C2_C3:
      return (Lop_LD_C);
    case PLAYDOHop_LGX_B_C3_V1:
      return (Lop_LD_C);
    case PLAYDOHop_LGX_B_C3_C1:
      return (Lop_LD_C);
    case PLAYDOHop_LGX_B_C3_C2:
      return (Lop_LD_C);
    case PLAYDOHop_LGX_B_C3_C3:
      return (Lop_LD_C);

    case PLAYDOHop_LMX_B_V1_V1:
      return (Lop_LD_C);
    case PLAYDOHop_LMX_B_V1_C1:
      return (Lop_LD_C);
    case PLAYDOHop_LMX_B_V1_C2:
      return (Lop_LD_C);
    case PLAYDOHop_LMX_B_V1_C3:
      return (Lop_LD_C);
    case PLAYDOHop_LMX_B_C1_V1:
      return (Lop_LD_C);
    case PLAYDOHop_LMX_B_C1_C1:
      return (Lop_LD_C);
    case PLAYDOHop_LMX_B_C1_C2:
      return (Lop_LD_C);
    case PLAYDOHop_LMX_B_C1_C3:
      return (Lop_LD_C);
    case PLAYDOHop_LMX_B_C2_V1:
      return (Lop_LD_C);
    case PLAYDOHop_LMX_B_C2_C1:
      return (Lop_LD_C);
    case PLAYDOHop_LMX_B_C2_C2:
      return (Lop_LD_C);
    case PLAYDOHop_LMX_B_C2_C3:
      return (Lop_LD_C);
    case PLAYDOHop_LMX_B_C3_V1:
      return (Lop_LD_C);
    case PLAYDOHop_LMX_B_C3_C1:
      return (Lop_LD_C);
    case PLAYDOHop_LMX_B_C3_C2:
      return (Lop_LD_C);
    case PLAYDOHop_LMX_B_C3_C3:
      return (Lop_LD_C);

    case PLAYDOHop_L_H_V1_V1:
      return (Lop_LD_UC2);
    case PLAYDOHop_L_H_V1_C1:
      return (Lop_LD_UC2);
    case PLAYDOHop_L_H_V1_C2:
      return (Lop_LD_UC2);
    case PLAYDOHop_L_H_V1_C3:
      return (Lop_LD_UC2);
    case PLAYDOHop_L_H_C1_V1:
      return (Lop_LD_UC2);
    case PLAYDOHop_L_H_C1_C1:
      return (Lop_LD_UC2);
    case PLAYDOHop_L_H_C1_C2:
      return (Lop_LD_UC2);
    case PLAYDOHop_L_H_C1_C3:
      return (Lop_LD_UC2);
    case PLAYDOHop_L_H_C2_V1:
      return (Lop_LD_UC2);
    case PLAYDOHop_L_H_C2_C1:
      return (Lop_LD_UC2);
    case PLAYDOHop_L_H_C2_C2:
      return (Lop_LD_UC2);
    case PLAYDOHop_L_H_C2_C3:
      return (Lop_LD_UC2);
    case PLAYDOHop_L_H_C3_V1:
      return (Lop_LD_UC2);
    case PLAYDOHop_L_H_C3_C1:
      return (Lop_LD_UC2);
    case PLAYDOHop_L_H_C3_C2:
      return (Lop_LD_UC2);
    case PLAYDOHop_L_H_C3_C3:
      return (Lop_LD_UC2);

    case PLAYDOHop_LG_H_V1_V1:
      return (Lop_LD_UC2);
    case PLAYDOHop_LG_H_V1_C1:
      return (Lop_LD_UC2);
    case PLAYDOHop_LG_H_V1_C2:
      return (Lop_LD_UC2);
    case PLAYDOHop_LG_H_V1_C3:
      return (Lop_LD_UC2);
    case PLAYDOHop_LG_H_C1_V1:
      return (Lop_LD_UC2);
    case PLAYDOHop_LG_H_C1_C1:
      return (Lop_LD_UC2);
    case PLAYDOHop_LG_H_C1_C2:
      return (Lop_LD_UC2);
    case PLAYDOHop_LG_H_C1_C3:
      return (Lop_LD_UC2);
    case PLAYDOHop_LG_H_C2_V1:
      return (Lop_LD_UC2);
    case PLAYDOHop_LG_H_C2_C1:
      return (Lop_LD_UC2);
    case PLAYDOHop_LG_H_C2_C2:
      return (Lop_LD_UC2);
    case PLAYDOHop_LG_H_C2_C3:
      return (Lop_LD_UC2);
    case PLAYDOHop_LG_H_C3_V1:
      return (Lop_LD_UC2);
    case PLAYDOHop_LG_H_C3_C1:
      return (Lop_LD_UC2);
    case PLAYDOHop_LG_H_C3_C2:
      return (Lop_LD_UC2);
    case PLAYDOHop_LG_H_C3_C3:
      return (Lop_LD_UC2);

    case PLAYDOHop_LM_H_V1_V1:
      return (Lop_LD_UC2);
    case PLAYDOHop_LM_H_V1_C1:
      return (Lop_LD_UC2);
    case PLAYDOHop_LM_H_V1_C2:
      return (Lop_LD_UC2);
    case PLAYDOHop_LM_H_V1_C3:
      return (Lop_LD_UC2);
    case PLAYDOHop_LM_H_C1_V1:
      return (Lop_LD_UC2);
    case PLAYDOHop_LM_H_C1_C1:
      return (Lop_LD_UC2);
    case PLAYDOHop_LM_H_C1_C2:
      return (Lop_LD_UC2);
    case PLAYDOHop_LM_H_C1_C3:
      return (Lop_LD_UC2);
    case PLAYDOHop_LM_H_C2_V1:
      return (Lop_LD_UC2);
    case PLAYDOHop_LM_H_C2_C1:
      return (Lop_LD_UC2);
    case PLAYDOHop_LM_H_C2_C2:
      return (Lop_LD_UC2);
    case PLAYDOHop_LM_H_C2_C3:
      return (Lop_LD_UC2);
    case PLAYDOHop_LM_H_C3_V1:
      return (Lop_LD_UC2);
    case PLAYDOHop_LM_H_C3_C1:
      return (Lop_LD_UC2);
    case PLAYDOHop_LM_H_C3_C2:
      return (Lop_LD_UC2);
    case PLAYDOHop_LM_H_C3_C3:
      return (Lop_LD_UC2);

    case PLAYDOHop_LX_H_V1_V1:
      return (Lop_LD_C2);
    case PLAYDOHop_LX_H_V1_C1:
      return (Lop_LD_C2);
    case PLAYDOHop_LX_H_V1_C2:
      return (Lop_LD_C2);
    case PLAYDOHop_LX_H_V1_C3:
      return (Lop_LD_C2);
    case PLAYDOHop_LX_H_C1_V1:
      return (Lop_LD_C2);
    case PLAYDOHop_LX_H_C1_C1:
      return (Lop_LD_C2);
    case PLAYDOHop_LX_H_C1_C2:
      return (Lop_LD_C2);
    case PLAYDOHop_LX_H_C1_C3:
      return (Lop_LD_C2);
    case PLAYDOHop_LX_H_C2_V1:
      return (Lop_LD_C2);
    case PLAYDOHop_LX_H_C2_C1:
      return (Lop_LD_C2);
    case PLAYDOHop_LX_H_C2_C2:
      return (Lop_LD_C2);
    case PLAYDOHop_LX_H_C2_C3:
      return (Lop_LD_C2);
    case PLAYDOHop_LX_H_C3_V1:
      return (Lop_LD_C2);
    case PLAYDOHop_LX_H_C3_C1:
      return (Lop_LD_C2);
    case PLAYDOHop_LX_H_C3_C2:
      return (Lop_LD_C2);
    case PLAYDOHop_LX_H_C3_C3:
      return (Lop_LD_C2);

    case PLAYDOHop_LGX_H_V1_V1:
      return (Lop_LD_C2);
    case PLAYDOHop_LGX_H_V1_C1:
      return (Lop_LD_C2);
    case PLAYDOHop_LGX_H_V1_C2:
      return (Lop_LD_C2);
    case PLAYDOHop_LGX_H_V1_C3:
      return (Lop_LD_C2);
    case PLAYDOHop_LGX_H_C1_V1:
      return (Lop_LD_C2);
    case PLAYDOHop_LGX_H_C1_C1:
      return (Lop_LD_C2);
    case PLAYDOHop_LGX_H_C1_C2:
      return (Lop_LD_C2);
    case PLAYDOHop_LGX_H_C1_C3:
      return (Lop_LD_C2);
    case PLAYDOHop_LGX_H_C2_V1:
      return (Lop_LD_C2);
    case PLAYDOHop_LGX_H_C2_C1:
      return (Lop_LD_C2);
    case PLAYDOHop_LGX_H_C2_C2:
      return (Lop_LD_C2);
    case PLAYDOHop_LGX_H_C2_C3:
      return (Lop_LD_C2);
    case PLAYDOHop_LGX_H_C3_V1:
      return (Lop_LD_C2);
    case PLAYDOHop_LGX_H_C3_C1:
      return (Lop_LD_C2);
    case PLAYDOHop_LGX_H_C3_C2:
      return (Lop_LD_C2);
    case PLAYDOHop_LGX_H_C3_C3:
      return (Lop_LD_C2);
    
    case PLAYDOHop_LMX_H_V1_V1:
      return (Lop_LD_C2);
    case PLAYDOHop_LMX_H_V1_C1:
      return (Lop_LD_C2);
    case PLAYDOHop_LMX_H_V1_C2:
      return (Lop_LD_C2);
    case PLAYDOHop_LMX_H_V1_C3:
      return (Lop_LD_C2);
    case PLAYDOHop_LMX_H_C1_V1:
      return (Lop_LD_C2);
    case PLAYDOHop_LMX_H_C1_C1:
      return (Lop_LD_C2);
    case PLAYDOHop_LMX_H_C1_C2:
      return (Lop_LD_C2);
    case PLAYDOHop_LMX_H_C1_C3:
      return (Lop_LD_C2);
    case PLAYDOHop_LMX_H_C2_V1:
      return (Lop_LD_C2);
    case PLAYDOHop_LMX_H_C2_C1:
      return (Lop_LD_C2);
    case PLAYDOHop_LMX_H_C2_C2:
      return (Lop_LD_C2);
    case PLAYDOHop_LMX_H_C2_C3:
      return (Lop_LD_C2);
    case PLAYDOHop_LMX_H_C3_V1:
      return (Lop_LD_C2);
    case PLAYDOHop_LMX_H_C3_C1:
      return (Lop_LD_C2);
    case PLAYDOHop_LMX_H_C3_C2:
      return (Lop_LD_C2);
    case PLAYDOHop_LMX_H_C3_C3:
      return (Lop_LD_C2);

    case PLAYDOHop_L_W_V1_V1:
      return (Lop_LD_I);
    case PLAYDOHop_L_W_V1_C1:
      return (Lop_LD_I);
    case PLAYDOHop_L_W_V1_C2:
      return (Lop_LD_I);
    case PLAYDOHop_L_W_V1_C3:
      return (Lop_LD_I);
    case PLAYDOHop_L_W_C1_V1:
      return (Lop_LD_I);
    case PLAYDOHop_L_W_C1_C1:
      return (Lop_LD_I);
    case PLAYDOHop_L_W_C1_C2:
      return (Lop_LD_I);
    case PLAYDOHop_L_W_C1_C3:
      return (Lop_LD_I);
    case PLAYDOHop_L_W_C2_V1:
      return (Lop_LD_I);
    case PLAYDOHop_L_W_C2_C1:
      return (Lop_LD_I);
    case PLAYDOHop_L_W_C2_C2:
      return (Lop_LD_I);
    case PLAYDOHop_L_W_C2_C3:
      return (Lop_LD_I);
    case PLAYDOHop_L_W_C3_V1:
      return (Lop_LD_I);
    case PLAYDOHop_L_W_C3_C1:
      return (Lop_LD_I);
    case PLAYDOHop_L_W_C3_C2:
      return (Lop_LD_I);
    case PLAYDOHop_L_W_C3_C3:
      return (Lop_LD_I);

    case PLAYDOHop_LG_W_V1_V1:
      return (Lop_LD_I);
    case PLAYDOHop_LG_W_V1_C1:
      return (Lop_LD_I);
    case PLAYDOHop_LG_W_V1_C2:
      return (Lop_LD_I);
    case PLAYDOHop_LG_W_V1_C3:
      return (Lop_LD_I);
    case PLAYDOHop_LG_W_C1_V1:
      return (Lop_LD_I);
    case PLAYDOHop_LG_W_C1_C1:
      return (Lop_LD_I);
    case PLAYDOHop_LG_W_C1_C2:
      return (Lop_LD_I);
    case PLAYDOHop_LG_W_C1_C3:
      return (Lop_LD_I);
    case PLAYDOHop_LG_W_C2_V1:
      return (Lop_LD_I);
    case PLAYDOHop_LG_W_C2_C1:
      return (Lop_LD_I);
    case PLAYDOHop_LG_W_C2_C2:
      return (Lop_LD_I);
    case PLAYDOHop_LG_W_C2_C3:
      return (Lop_LD_I);
    case PLAYDOHop_LG_W_C3_V1:
      return (Lop_LD_I);
    case PLAYDOHop_LG_W_C3_C1:
      return (Lop_LD_I);
    case PLAYDOHop_LG_W_C3_C2:
      return (Lop_LD_I);
    case PLAYDOHop_LG_W_C3_C3:
      return (Lop_LD_I);

    case PLAYDOHop_LM_W_V1_V1:
      return (Lop_LD_I);
    case PLAYDOHop_LM_W_V1_C1:
      return (Lop_LD_I);
    case PLAYDOHop_LM_W_V1_C2:
      return (Lop_LD_I);
    case PLAYDOHop_LM_W_V1_C3:
      return (Lop_LD_I);
    case PLAYDOHop_LM_W_C1_V1:
      return (Lop_LD_I);
    case PLAYDOHop_LM_W_C1_C1:
      return (Lop_LD_I);
    case PLAYDOHop_LM_W_C1_C2:
      return (Lop_LD_I);
    case PLAYDOHop_LM_W_C1_C3:
      return (Lop_LD_I);
    case PLAYDOHop_LM_W_C2_V1:
      return (Lop_LD_I);
    case PLAYDOHop_LM_W_C2_C1:
      return (Lop_LD_I);
    case PLAYDOHop_LM_W_C2_C2:
      return (Lop_LD_I);
    case PLAYDOHop_LM_W_C2_C3:
      return (Lop_LD_I);
    case PLAYDOHop_LM_W_C3_V1:
      return (Lop_LD_I);
    case PLAYDOHop_LM_W_C3_C1:
      return (Lop_LD_I);
    case PLAYDOHop_LM_W_C3_C2:
      return (Lop_LD_I);
    case PLAYDOHop_LM_W_C3_C3:
      return (Lop_LD_I);

    case PLAYDOHop_L_Q_V1_V1:
      return (Lop_LD_Q);
    case PLAYDOHop_L_Q_V1_C1:
      return (Lop_LD_Q);
    case PLAYDOHop_L_Q_V1_C2:
      return (Lop_LD_Q);
    case PLAYDOHop_L_Q_V1_C3:
      return (Lop_LD_Q);
    case PLAYDOHop_L_Q_C1_V1:
      return (Lop_LD_Q);
    case PLAYDOHop_L_Q_C1_C1:
      return (Lop_LD_Q);
    case PLAYDOHop_L_Q_C1_C2:
      return (Lop_LD_Q);
    case PLAYDOHop_L_Q_C1_C3:
      return (Lop_LD_Q);
    case PLAYDOHop_L_Q_C2_V1:
      return (Lop_LD_Q);
    case PLAYDOHop_L_Q_C2_C1:
      return (Lop_LD_Q);
    case PLAYDOHop_L_Q_C2_C2:
      return (Lop_LD_Q);
    case PLAYDOHop_L_Q_C2_C3:
      return (Lop_LD_Q);
    case PLAYDOHop_L_Q_C3_V1:
      return (Lop_LD_Q);
    case PLAYDOHop_L_Q_C3_C1:
      return (Lop_LD_Q);
    case PLAYDOHop_L_Q_C3_C2:
      return (Lop_LD_Q);
    case PLAYDOHop_L_Q_C3_C3:
      return (Lop_LD_Q);

    case PLAYDOHop_LG_Q_V1_V1:
      return (Lop_LD_Q);
    case PLAYDOHop_LG_Q_V1_C1:
      return (Lop_LD_Q);
    case PLAYDOHop_LG_Q_V1_C2:
      return (Lop_LD_Q);
    case PLAYDOHop_LG_Q_V1_C3:
      return (Lop_LD_Q);
    case PLAYDOHop_LG_Q_C1_V1:
      return (Lop_LD_Q);
    case PLAYDOHop_LG_Q_C1_C1:
      return (Lop_LD_Q);
    case PLAYDOHop_LG_Q_C1_C2:
      return (Lop_LD_Q);
    case PLAYDOHop_LG_Q_C1_C3:
      return (Lop_LD_Q);
    case PLAYDOHop_LG_Q_C2_V1:
      return (Lop_LD_Q);
    case PLAYDOHop_LG_Q_C2_C1:
      return (Lop_LD_Q);
    case PLAYDOHop_LG_Q_C2_C2:
      return (Lop_LD_Q);
    case PLAYDOHop_LG_Q_C2_C3:
      return (Lop_LD_Q);
    case PLAYDOHop_LG_Q_C3_V1:
      return (Lop_LD_Q);
    case PLAYDOHop_LG_Q_C3_C1:
      return (Lop_LD_Q);
    case PLAYDOHop_LG_Q_C3_C2:
      return (Lop_LD_Q);
    case PLAYDOHop_LG_Q_C3_C3:
      return (Lop_LD_Q);

    case PLAYDOHop_LM_Q_V1_V1:
      return (Lop_LD_Q);
    case PLAYDOHop_LM_Q_V1_C1:
      return (Lop_LD_Q);
    case PLAYDOHop_LM_Q_V1_C2:
      return (Lop_LD_Q);
    case PLAYDOHop_LM_Q_V1_C3:
      return (Lop_LD_Q);
    case PLAYDOHop_LM_Q_C1_V1:
      return (Lop_LD_Q);
    case PLAYDOHop_LM_Q_C1_C1:
      return (Lop_LD_Q);
    case PLAYDOHop_LM_Q_C1_C2:
      return (Lop_LD_Q);
    case PLAYDOHop_LM_Q_C1_C3:
      return (Lop_LD_Q);
    case PLAYDOHop_LM_Q_C2_V1:
      return (Lop_LD_Q);
    case PLAYDOHop_LM_Q_C2_C1:
      return (Lop_LD_Q);
    case PLAYDOHop_LM_Q_C2_C2:
      return (Lop_LD_Q);
    case PLAYDOHop_LM_Q_C2_C3:
      return (Lop_LD_Q);
    case PLAYDOHop_LM_Q_C3_V1:
      return (Lop_LD_Q);
    case PLAYDOHop_LM_Q_C3_C1:
      return (Lop_LD_Q);
    case PLAYDOHop_LM_Q_C3_C2:
      return (Lop_LD_Q);
    case PLAYDOHop_LM_Q_C3_C3:
      return (Lop_LD_Q);

    case PLAYDOHop_LI_B_V1_V1:
      return (Lop_LD_POST_UC);
    case PLAYDOHop_LI_B_V1_C1:
      return (Lop_LD_POST_UC);
    case PLAYDOHop_LI_B_V1_C2:
      return (Lop_LD_POST_UC);
    case PLAYDOHop_LI_B_V1_C3:
      return (Lop_LD_POST_UC);
    case PLAYDOHop_LI_B_C1_V1:
      return (Lop_LD_POST_UC);
    case PLAYDOHop_LI_B_C1_C1:
      return (Lop_LD_POST_UC);
    case PLAYDOHop_LI_B_C1_C2:
      return (Lop_LD_POST_UC);
    case PLAYDOHop_LI_B_C1_C3:
      return (Lop_LD_POST_UC);
    case PLAYDOHop_LI_B_C2_V1:
      return (Lop_LD_POST_UC);
    case PLAYDOHop_LI_B_C2_C1:
      return (Lop_LD_POST_UC);
    case PLAYDOHop_LI_B_C2_C2:
      return (Lop_LD_POST_UC);
    case PLAYDOHop_LI_B_C2_C3:
      return (Lop_LD_POST_UC);
    case PLAYDOHop_LI_B_C3_V1:
      return (Lop_LD_POST_UC);
    case PLAYDOHop_LI_B_C3_C1:
      return (Lop_LD_POST_UC);
    case PLAYDOHop_LI_B_C3_C2:
      return (Lop_LD_POST_UC);
    case PLAYDOHop_LI_B_C3_C3:
      return (Lop_LD_POST_UC);

    case PLAYDOHop_LI_H_V1_V1:
      return (Lop_LD_POST_UC2);
    case PLAYDOHop_LI_H_V1_C1:
      return (Lop_LD_POST_UC2);
    case PLAYDOHop_LI_H_V1_C2:
      return (Lop_LD_POST_UC2);
    case PLAYDOHop_LI_H_V1_C3:
      return (Lop_LD_POST_UC2);
    case PLAYDOHop_LI_H_C1_V1:
      return (Lop_LD_POST_UC2);
    case PLAYDOHop_LI_H_C1_C1:
      return (Lop_LD_POST_UC2);
    case PLAYDOHop_LI_H_C1_C2:
      return (Lop_LD_POST_UC2);
    case PLAYDOHop_LI_H_C1_C3:
      return (Lop_LD_POST_UC2);
    case PLAYDOHop_LI_H_C2_V1:
      return (Lop_LD_POST_UC2);
    case PLAYDOHop_LI_H_C2_C1:
      return (Lop_LD_POST_UC2);
    case PLAYDOHop_LI_H_C2_C2:
      return (Lop_LD_POST_UC2);
    case PLAYDOHop_LI_H_C2_C3:
      return (Lop_LD_POST_UC2);
    case PLAYDOHop_LI_H_C3_V1:
      return (Lop_LD_POST_UC2);
    case PLAYDOHop_LI_H_C3_C1:
      return (Lop_LD_POST_UC2);
    case PLAYDOHop_LI_H_C3_C2:
      return (Lop_LD_POST_UC2);
    case PLAYDOHop_LI_H_C3_C3:
      return (Lop_LD_POST_UC2);

    case PLAYDOHop_LI_W_V1_V1:
      return (Lop_LD_POST_I);
    case PLAYDOHop_LI_W_V1_C1:
      return (Lop_LD_POST_I);
    case PLAYDOHop_LI_W_V1_C2:
      return (Lop_LD_POST_I);
    case PLAYDOHop_LI_W_V1_C3:
      return (Lop_LD_POST_I);
    case PLAYDOHop_LI_W_C1_V1:
      return (Lop_LD_POST_I);
    case PLAYDOHop_LI_W_C1_C1:
      return (Lop_LD_POST_I);
    case PLAYDOHop_LI_W_C1_C2:
      return (Lop_LD_POST_I);
    case PLAYDOHop_LI_W_C1_C3:
      return (Lop_LD_POST_I);
    case PLAYDOHop_LI_W_C2_V1:
      return (Lop_LD_POST_I);
    case PLAYDOHop_LI_W_C2_C1:
      return (Lop_LD_POST_I);
    case PLAYDOHop_LI_W_C2_C2:
      return (Lop_LD_POST_I);
    case PLAYDOHop_LI_W_C2_C3:
      return (Lop_LD_POST_I);
    case PLAYDOHop_LI_W_C3_V1:
      return (Lop_LD_POST_I);
    case PLAYDOHop_LI_W_C3_C1:
      return (Lop_LD_POST_I);
    case PLAYDOHop_LI_W_C3_C2:
      return (Lop_LD_POST_I);
    case PLAYDOHop_LI_W_C3_C3:
      return (Lop_LD_POST_I);

    case PLAYDOHop_LI_Q_V1_V1:
      return (Lop_LD_POST_Q);
    case PLAYDOHop_LI_Q_V1_C1:
      return (Lop_LD_POST_Q);
    case PLAYDOHop_LI_Q_V1_C2:
      return (Lop_LD_POST_Q);
    case PLAYDOHop_LI_Q_V1_C3:
      return (Lop_LD_POST_Q);
    case PLAYDOHop_LI_Q_C1_V1:
      return (Lop_LD_POST_Q);
    case PLAYDOHop_LI_Q_C1_C1:
      return (Lop_LD_POST_Q);
    case PLAYDOHop_LI_Q_C1_C2:
      return (Lop_LD_POST_Q);
    case PLAYDOHop_LI_Q_C1_C3:
      return (Lop_LD_POST_Q);
    case PLAYDOHop_LI_Q_C2_V1:
      return (Lop_LD_POST_Q);
    case PLAYDOHop_LI_Q_C2_C1:
      return (Lop_LD_POST_Q);
    case PLAYDOHop_LI_Q_C2_C2:
      return (Lop_LD_POST_Q);
    case PLAYDOHop_LI_Q_C2_C3:
      return (Lop_LD_POST_Q);
    case PLAYDOHop_LI_Q_C3_V1:
      return (Lop_LD_POST_Q);
    case PLAYDOHop_LI_Q_C3_C1:
      return (Lop_LD_POST_Q);
    case PLAYDOHop_LI_Q_C3_C2:
      return (Lop_LD_POST_Q);
    case PLAYDOHop_LI_Q_C3_C3:
      return (Lop_LD_POST_Q);

    case PLAYDOHop_S_B_V1:
      return (Lop_ST_C);
    case PLAYDOHop_S_B_C1:
      return (Lop_ST_C);
    case PLAYDOHop_S_B_C2:
      return (Lop_ST_C);
    case PLAYDOHop_S_B_C3:
      return (Lop_ST_C);

    case PLAYDOHop_S_H_V1:
      return (Lop_ST_C2);
    case PLAYDOHop_S_H_C1:
      return (Lop_ST_C2);
    case PLAYDOHop_S_H_C2:
      return (Lop_ST_C2);
    case PLAYDOHop_S_H_C3:
      return (Lop_ST_C2);

    case PLAYDOHop_S_W_V1:
      return (Lop_ST_I);
    case PLAYDOHop_S_W_C1:
      return (Lop_ST_I);
    case PLAYDOHop_S_W_C2:
      return (Lop_ST_I);
    case PLAYDOHop_S_W_C3:
      return (Lop_ST_I);

    case PLAYDOHop_S_Q_V1:
      return (Lop_ST_Q);
    case PLAYDOHop_S_Q_C1:
      return (Lop_ST_Q);
    case PLAYDOHop_S_Q_C2:
      return (Lop_ST_Q);
    case PLAYDOHop_S_Q_C3:
      return (Lop_ST_Q);
    
    case PLAYDOHop_SG_B_V1:
      return (Lop_ST_C);
    case PLAYDOHop_SG_B_C1:
      return (Lop_ST_C);
    case PLAYDOHop_SG_B_C2:
      return (Lop_ST_C);
    case PLAYDOHop_SG_B_C3:
      return (Lop_ST_C);

    case PLAYDOHop_SG_H_V1:
      return (Lop_ST_C2);
    case PLAYDOHop_SG_H_C1:
      return (Lop_ST_C2);
    case PLAYDOHop_SG_H_C2:
      return (Lop_ST_C2);
    case PLAYDOHop_SG_H_C3:
      return (Lop_ST_C2);

    case PLAYDOHop_SG_W_V1:
      return (Lop_ST_I);
    case PLAYDOHop_SG_W_C1:
      return (Lop_ST_I);
    case PLAYDOHop_SG_W_C2:
      return (Lop_ST_I);
    case PLAYDOHop_SG_W_C3:
      return (Lop_ST_I);

    case PLAYDOHop_SG_Q_V1:
      return (Lop_ST_Q);
    case PLAYDOHop_SG_Q_C1:
      return (Lop_ST_Q);
    case PLAYDOHop_SG_Q_C2:
      return (Lop_ST_Q);
    case PLAYDOHop_SG_Q_C3:
      return (Lop_ST_Q);

    case PLAYDOHop_SM_B_V1:
      return (Lop_ST_C);
    case PLAYDOHop_SM_B_C1:
      return (Lop_ST_C);
    case PLAYDOHop_SM_B_C2:
      return (Lop_ST_C);
    case PLAYDOHop_SM_B_C3:
      return (Lop_ST_C);

    case PLAYDOHop_SM_H_V1:
      return (Lop_ST_C2);
    case PLAYDOHop_SM_H_C1:
      return (Lop_ST_C2);
    case PLAYDOHop_SM_H_C2:
      return (Lop_ST_C2);
    case PLAYDOHop_SM_H_C3:
      return (Lop_ST_C2);

    case PLAYDOHop_SM_W_V1:
      return (Lop_ST_I);
    case PLAYDOHop_SM_W_C1:
      return (Lop_ST_I);
    case PLAYDOHop_SM_W_C2:
      return (Lop_ST_I);
    case PLAYDOHop_SM_W_C3:
      return (Lop_ST_I);

    case PLAYDOHop_SM_Q_V1:
      return (Lop_ST_Q);
    case PLAYDOHop_SM_Q_C1:
      return (Lop_ST_Q);
    case PLAYDOHop_SM_Q_C2:
      return (Lop_ST_Q);
    case PLAYDOHop_SM_Q_C3:
      return (Lop_ST_Q);

    case PLAYDOHop_SI_B_V1:
      return (Lop_ST_POST_C);
    case PLAYDOHop_SI_B_C1:
      return (Lop_ST_POST_C);
    case PLAYDOHop_SI_B_C2:
      return (Lop_ST_POST_C);
    case PLAYDOHop_SI_B_C3:
      return (Lop_ST_POST_C);

    case PLAYDOHop_SI_H_V1:
      return (Lop_ST_POST_C2);
    case PLAYDOHop_SI_H_C1:
      return (Lop_ST_POST_C2);
    case PLAYDOHop_SI_H_C2:
      return (Lop_ST_POST_C2);
    case PLAYDOHop_SI_H_C3:
      return (Lop_ST_POST_C2);

    case PLAYDOHop_SI_W_V1:
      return (Lop_ST_POST_I);
    case PLAYDOHop_SI_W_C1:
      return (Lop_ST_POST_I);
    case PLAYDOHop_SI_W_C2:
      return (Lop_ST_POST_I);
    case PLAYDOHop_SI_W_C3:
      return (Lop_ST_POST_I);

    case PLAYDOHop_SI_Q_V1:
      return (Lop_ST_POST_Q);
    case PLAYDOHop_SI_Q_C1:
      return (Lop_ST_POST_Q);
    case PLAYDOHop_SI_Q_C2:
      return (Lop_ST_POST_Q);
    case PLAYDOHop_SI_Q_C3:
      return (Lop_ST_POST_Q);

    case PLAYDOHop_FL_S_V1_V1:
      return (Lop_LD_F);
    case PLAYDOHop_FL_S_V1_C1:
      return (Lop_LD_F);
    case PLAYDOHop_FL_S_V1_C2:
      return (Lop_LD_F);
    case PLAYDOHop_FL_S_V1_C3:
      return (Lop_LD_F);
    case PLAYDOHop_FL_S_C1_V1:
      return (Lop_LD_F);
    case PLAYDOHop_FL_S_C1_C1:
      return (Lop_LD_F);
    case PLAYDOHop_FL_S_C1_C2:
      return (Lop_LD_F);
    case PLAYDOHop_FL_S_C1_C3:
      return (Lop_LD_F);
    case PLAYDOHop_FL_S_C2_V1:
      return (Lop_LD_F);
    case PLAYDOHop_FL_S_C2_C1:
      return (Lop_LD_F);
    case PLAYDOHop_FL_S_C2_C2:
      return (Lop_LD_F);
    case PLAYDOHop_FL_S_C2_C3:
      return (Lop_LD_F);
    case PLAYDOHop_FL_S_C3_V1:
      return (Lop_LD_F);
    case PLAYDOHop_FL_S_C3_C1:
      return (Lop_LD_F);
    case PLAYDOHop_FL_S_C3_C2:
      return (Lop_LD_F);
    case PLAYDOHop_FL_S_C3_C3:
      return (Lop_LD_F);

    case PLAYDOHop_FL_D_V1_V1:
      return (Lop_LD_F2);
    case PLAYDOHop_FL_D_V1_C1:
      return (Lop_LD_F2);
    case PLAYDOHop_FL_D_V1_C2:
      return (Lop_LD_F2);
    case PLAYDOHop_FL_D_V1_C3:
      return (Lop_LD_F2);
    case PLAYDOHop_FL_D_C1_V1:
      return (Lop_LD_F2);
    case PLAYDOHop_FL_D_C1_C1:
      return (Lop_LD_F2);
    case PLAYDOHop_FL_D_C1_C2:
      return (Lop_LD_F2);
    case PLAYDOHop_FL_D_C1_C3:
      return (Lop_LD_F2);
    case PLAYDOHop_FL_D_C2_V1:
      return (Lop_LD_F2);
    case PLAYDOHop_FL_D_C2_C1:
      return (Lop_LD_F2);
    case PLAYDOHop_FL_D_C2_C2:
      return (Lop_LD_F2);
    case PLAYDOHop_FL_D_C2_C3:
      return (Lop_LD_F2);
    case PLAYDOHop_FL_D_C3_V1:
      return (Lop_LD_F2);
    case PLAYDOHop_FL_D_C3_C1:
      return (Lop_LD_F2);
    case PLAYDOHop_FL_D_C3_C2:
      return (Lop_LD_F2);
    case PLAYDOHop_FL_D_C3_C3:
      return (Lop_LD_F2);

    case PLAYDOHop_FLI_S_V1_V1:
      return (Lop_LD_POST_F);
    case PLAYDOHop_FLI_S_V1_C1:
      return (Lop_LD_POST_F);
    case PLAYDOHop_FLI_S_V1_C2:
      return (Lop_LD_POST_F);
    case PLAYDOHop_FLI_S_V1_C3:
      return (Lop_LD_POST_F);
    case PLAYDOHop_FLI_S_C1_V1:
      return (Lop_LD_POST_F);
    case PLAYDOHop_FLI_S_C1_C1:
      return (Lop_LD_POST_F);
    case PLAYDOHop_FLI_S_C1_C2:
      return (Lop_LD_POST_F);
    case PLAYDOHop_FLI_S_C1_C3:
      return (Lop_LD_POST_F);
    case PLAYDOHop_FLI_S_C2_V1:
      return (Lop_LD_POST_F);
    case PLAYDOHop_FLI_S_C2_C1:
      return (Lop_LD_POST_F);
    case PLAYDOHop_FLI_S_C2_C2:
      return (Lop_LD_POST_F);
    case PLAYDOHop_FLI_S_C2_C3:
      return (Lop_LD_POST_F);
    case PLAYDOHop_FLI_S_C3_V1:
      return (Lop_LD_POST_F);
    case PLAYDOHop_FLI_S_C3_C1:
      return (Lop_LD_POST_F);
    case PLAYDOHop_FLI_S_C3_C2:
      return (Lop_LD_POST_F);
    case PLAYDOHop_FLI_S_C3_C3:
      return (Lop_LD_POST_F);

    case PLAYDOHop_FLI_D_V1_V1:
      return (Lop_LD_POST_F2);
    case PLAYDOHop_FLI_D_V1_C1:
      return (Lop_LD_POST_F2);
    case PLAYDOHop_FLI_D_V1_C2:
      return (Lop_LD_POST_F2);
    case PLAYDOHop_FLI_D_V1_C3:
      return (Lop_LD_POST_F2);
    case PLAYDOHop_FLI_D_C1_V1:
      return (Lop_LD_POST_F2);
    case PLAYDOHop_FLI_D_C1_C1:
      return (Lop_LD_POST_F2);
    case PLAYDOHop_FLI_D_C1_C2:
      return (Lop_LD_POST_F2);
    case PLAYDOHop_FLI_D_C1_C3:
      return (Lop_LD_POST_F2);
    case PLAYDOHop_FLI_D_C2_V1:
      return (Lop_LD_POST_F2);
    case PLAYDOHop_FLI_D_C2_C1:
      return (Lop_LD_POST_F2);
    case PLAYDOHop_FLI_D_C2_C2:
      return (Lop_LD_POST_F2);
    case PLAYDOHop_FLI_D_C2_C3:
      return (Lop_LD_POST_F2);
    case PLAYDOHop_FLI_D_C3_V1:
      return (Lop_LD_POST_F2);
    case PLAYDOHop_FLI_D_C3_C1:
      return (Lop_LD_POST_F2);
    case PLAYDOHop_FLI_D_C3_C2:
      return (Lop_LD_POST_F2);
    case PLAYDOHop_FLI_D_C3_C3:
      return (Lop_LD_POST_F2);

    case PLAYDOHop_FS_S_V1:
      return (Lop_ST_F);
    case PLAYDOHop_FS_S_C1:
      return (Lop_ST_F);
    case PLAYDOHop_FS_S_C2:
      return (Lop_ST_F);
    case PLAYDOHop_FS_S_C3:
      return (Lop_ST_F);

    case PLAYDOHop_FS_D_V1:
      return (Lop_ST_F2);
    case PLAYDOHop_FS_D_C1:
      return (Lop_ST_F2);
    case PLAYDOHop_FS_D_C2:
      return (Lop_ST_F2);
    case PLAYDOHop_FS_D_C3:
      return (Lop_ST_F2);

    case PLAYDOHop_FSI_S_V1:
      return (Lop_ST_POST_F);
    case PLAYDOHop_FSI_S_C1:
      return (Lop_ST_POST_F);
    case PLAYDOHop_FSI_S_C2:
      return (Lop_ST_POST_F);
    case PLAYDOHop_FSI_S_C3:
      return (Lop_ST_POST_F);

    case PLAYDOHop_FSI_D_V1:
      return (Lop_ST_POST_F2);
    case PLAYDOHop_FSI_D_C1:
      return (Lop_ST_POST_F2);
    case PLAYDOHop_FSI_D_C2:
      return (Lop_ST_POST_F2);
    case PLAYDOHop_FSI_D_C3:
      return (Lop_ST_POST_F2);

    case PLAYDOHop_LDS_B_V1_V1:
      return (Lop_LD_UC);
    case PLAYDOHop_LDS_B_V1_C1:
      return (Lop_LD_UC);
    case PLAYDOHop_LDS_B_V1_C2:
      return (Lop_LD_UC);
    case PLAYDOHop_LDS_B_V1_C3:
      return (Lop_LD_UC);
    case PLAYDOHop_LDS_B_C1_V1:
      return (Lop_LD_UC);
    case PLAYDOHop_LDS_B_C1_C1:
      return (Lop_LD_UC);
    case PLAYDOHop_LDS_B_C1_C2:
      return (Lop_LD_UC);
    case PLAYDOHop_LDS_B_C1_C3:
      return (Lop_LD_UC);
    case PLAYDOHop_LDS_B_C2_V1:
      return (Lop_LD_UC);
    case PLAYDOHop_LDS_B_C2_C1:
      return (Lop_LD_UC);
    case PLAYDOHop_LDS_B_C2_C2:
      return (Lop_LD_UC);
    case PLAYDOHop_LDS_B_C2_C3:
      return (Lop_LD_UC);
    case PLAYDOHop_LDS_B_C3_V1:
      return (Lop_LD_UC);
    case PLAYDOHop_LDS_B_C3_C1:
      return (Lop_LD_UC);
    case PLAYDOHop_LDS_B_C3_C2:
      return (Lop_LD_UC);
    case PLAYDOHop_LDS_B_C3_C3:
      return (Lop_LD_UC);

    case PLAYDOHop_LDS_H_V1_V1:
      return (Lop_LD_UC2);
    case PLAYDOHop_LDS_H_V1_C1:
      return (Lop_LD_UC2);
    case PLAYDOHop_LDS_H_V1_C2:
      return (Lop_LD_UC2);
    case PLAYDOHop_LDS_H_V1_C3:
      return (Lop_LD_UC2);
    case PLAYDOHop_LDS_H_C1_V1:
      return (Lop_LD_UC2);
    case PLAYDOHop_LDS_H_C1_C1:
      return (Lop_LD_UC2);
    case PLAYDOHop_LDS_H_C1_C2:
      return (Lop_LD_UC2);
    case PLAYDOHop_LDS_H_C1_C3:
      return (Lop_LD_UC2);
    case PLAYDOHop_LDS_H_C2_V1:
      return (Lop_LD_UC2);
    case PLAYDOHop_LDS_H_C2_C1:
      return (Lop_LD_UC2);
    case PLAYDOHop_LDS_H_C2_C2:
      return (Lop_LD_UC2);
    case PLAYDOHop_LDS_H_C2_C3:
      return (Lop_LD_UC2);
    case PLAYDOHop_LDS_H_C3_V1:
      return (Lop_LD_UC2);
    case PLAYDOHop_LDS_H_C3_C1:
      return (Lop_LD_UC2);
    case PLAYDOHop_LDS_H_C3_C2:
      return (Lop_LD_UC2);
    case PLAYDOHop_LDS_H_C3_C3:
      return (Lop_LD_UC2);

    case PLAYDOHop_LDS_W_V1_V1:
      return (Lop_LD_I);
    case PLAYDOHop_LDS_W_V1_C1:
      return (Lop_LD_I);
    case PLAYDOHop_LDS_W_V1_C2:
      return (Lop_LD_I);
    case PLAYDOHop_LDS_W_V1_C3:
      return (Lop_LD_I);
    case PLAYDOHop_LDS_W_C1_V1:
      return (Lop_LD_I);
    case PLAYDOHop_LDS_W_C1_C1:
      return (Lop_LD_I);
    case PLAYDOHop_LDS_W_C1_C2:
      return (Lop_LD_I);
    case PLAYDOHop_LDS_W_C1_C3:
      return (Lop_LD_I);
    case PLAYDOHop_LDS_W_C2_V1:
      return (Lop_LD_I);
    case PLAYDOHop_LDS_W_C2_C1:
      return (Lop_LD_I);
    case PLAYDOHop_LDS_W_C2_C2:
      return (Lop_LD_I);
    case PLAYDOHop_LDS_W_C2_C3:
      return (Lop_LD_I);
    case PLAYDOHop_LDS_W_C3_V1:
      return (Lop_LD_I);
    case PLAYDOHop_LDS_W_C3_C1:
      return (Lop_LD_I);
    case PLAYDOHop_LDS_W_C3_C2:
      return (Lop_LD_I);
    case PLAYDOHop_LDS_W_C3_C3:
      return (Lop_LD_I);

    case PLAYDOHop_LDS_Q_V1_V1:
      return (Lop_LD_Q);
    case PLAYDOHop_LDS_Q_V1_C1:
      return (Lop_LD_Q);
    case PLAYDOHop_LDS_Q_V1_C2:
      return (Lop_LD_Q);
    case PLAYDOHop_LDS_Q_V1_C3:
      return (Lop_LD_Q);
    case PLAYDOHop_LDS_Q_C1_V1:
      return (Lop_LD_Q);
    case PLAYDOHop_LDS_Q_C1_C1:
      return (Lop_LD_Q);
    case PLAYDOHop_LDS_Q_C1_C2:
      return (Lop_LD_Q);
    case PLAYDOHop_LDS_Q_C1_C3:
      return (Lop_LD_Q);
    case PLAYDOHop_LDS_Q_C2_V1:
      return (Lop_LD_Q);
    case PLAYDOHop_LDS_Q_C2_C1:
      return (Lop_LD_Q);
    case PLAYDOHop_LDS_Q_C2_C2:
      return (Lop_LD_Q);
    case PLAYDOHop_LDS_Q_C2_C3:
      return (Lop_LD_Q);
    case PLAYDOHop_LDS_Q_C3_V1:
      return (Lop_LD_Q);
    case PLAYDOHop_LDS_Q_C3_C1:
      return (Lop_LD_Q);
    case PLAYDOHop_LDS_Q_C3_C2:
      return (Lop_LD_Q);
    case PLAYDOHop_LDS_Q_C3_C3:
      return (Lop_LD_Q);

    case PLAYDOHop_LDSI_B_V1_V1:
      return (Lop_LD_POST_UC);
    case PLAYDOHop_LDSI_B_V1_C1:
      return (Lop_LD_POST_UC);
    case PLAYDOHop_LDSI_B_V1_C2:
      return (Lop_LD_POST_UC);
    case PLAYDOHop_LDSI_B_V1_C3:
      return (Lop_LD_POST_UC);
    case PLAYDOHop_LDSI_B_C1_V1:
      return (Lop_LD_POST_UC);
    case PLAYDOHop_LDSI_B_C1_C1:
      return (Lop_LD_POST_UC);
    case PLAYDOHop_LDSI_B_C1_C2:
      return (Lop_LD_POST_UC);
    case PLAYDOHop_LDSI_B_C1_C3:
      return (Lop_LD_POST_UC);
    case PLAYDOHop_LDSI_B_C2_V1:
      return (Lop_LD_POST_UC);
    case PLAYDOHop_LDSI_B_C2_C1:
      return (Lop_LD_POST_UC);
    case PLAYDOHop_LDSI_B_C2_C2:
      return (Lop_LD_POST_UC);
    case PLAYDOHop_LDSI_B_C2_C3:
      return (Lop_LD_POST_UC);
    case PLAYDOHop_LDSI_B_C3_V1:
      return (Lop_LD_POST_UC);
    case PLAYDOHop_LDSI_B_C3_C1:
      return (Lop_LD_POST_UC);
    case PLAYDOHop_LDSI_B_C3_C2:
      return (Lop_LD_POST_UC);
    case PLAYDOHop_LDSI_B_C3_C3:
      return (Lop_LD_POST_UC);

    case PLAYDOHop_LDSI_H_V1_V1:
      return (Lop_LD_POST_UC2);
    case PLAYDOHop_LDSI_H_V1_C1:
      return (Lop_LD_POST_UC2);
    case PLAYDOHop_LDSI_H_V1_C2:
      return (Lop_LD_POST_UC2);
    case PLAYDOHop_LDSI_H_V1_C3:
      return (Lop_LD_POST_UC2);
    case PLAYDOHop_LDSI_H_C1_V1:
      return (Lop_LD_POST_UC2);
    case PLAYDOHop_LDSI_H_C1_C1:
      return (Lop_LD_POST_UC2);
    case PLAYDOHop_LDSI_H_C1_C2:
      return (Lop_LD_POST_UC2);
    case PLAYDOHop_LDSI_H_C1_C3:
      return (Lop_LD_POST_UC2);
    case PLAYDOHop_LDSI_H_C2_V1:
      return (Lop_LD_POST_UC2);
    case PLAYDOHop_LDSI_H_C2_C1:
      return (Lop_LD_POST_UC2);
    case PLAYDOHop_LDSI_H_C2_C2:
      return (Lop_LD_POST_UC2);
    case PLAYDOHop_LDSI_H_C2_C3:
      return (Lop_LD_POST_UC2);
    case PLAYDOHop_LDSI_H_C3_V1:
      return (Lop_LD_POST_UC2);
    case PLAYDOHop_LDSI_H_C3_C1:
      return (Lop_LD_POST_UC2);
    case PLAYDOHop_LDSI_H_C3_C2:
      return (Lop_LD_POST_UC2);
    case PLAYDOHop_LDSI_H_C3_C3:
      return (Lop_LD_POST_UC2);

    case PLAYDOHop_LDSI_W_V1_V1:
      return (Lop_LD_POST_I);
    case PLAYDOHop_LDSI_W_V1_C1:
      return (Lop_LD_POST_I);
    case PLAYDOHop_LDSI_W_V1_C2:
      return (Lop_LD_POST_I);
    case PLAYDOHop_LDSI_W_V1_C3:
      return (Lop_LD_POST_I);
    case PLAYDOHop_LDSI_W_C1_V1:
      return (Lop_LD_POST_I);
    case PLAYDOHop_LDSI_W_C1_C1:
      return (Lop_LD_POST_I);
    case PLAYDOHop_LDSI_W_C1_C2:
      return (Lop_LD_POST_I);
    case PLAYDOHop_LDSI_W_C1_C3:
      return (Lop_LD_POST_I);
    case PLAYDOHop_LDSI_W_C2_V1:
      return (Lop_LD_POST_I);
    case PLAYDOHop_LDSI_W_C2_C1:
      return (Lop_LD_POST_I);
    case PLAYDOHop_LDSI_W_C2_C2:
      return (Lop_LD_POST_I);
    case PLAYDOHop_LDSI_W_C2_C3:
      return (Lop_LD_POST_I);
    case PLAYDOHop_LDSI_W_C3_V1:
      return (Lop_LD_POST_I);
    case PLAYDOHop_LDSI_W_C3_C1:
      return (Lop_LD_POST_I);
    case PLAYDOHop_LDSI_W_C3_C2:
      return (Lop_LD_POST_I);
    case PLAYDOHop_LDSI_W_C3_C3:
      return (Lop_LD_POST_I);

    case PLAYDOHop_LDSI_Q_V1_V1:
      return (Lop_LD_POST_Q);
    case PLAYDOHop_LDSI_Q_V1_C1:
      return (Lop_LD_POST_Q);
    case PLAYDOHop_LDSI_Q_V1_C2:
      return (Lop_LD_POST_Q);
    case PLAYDOHop_LDSI_Q_V1_C3:
      return (Lop_LD_POST_Q);
    case PLAYDOHop_LDSI_Q_C1_V1:
      return (Lop_LD_POST_Q);
    case PLAYDOHop_LDSI_Q_C1_C1:
      return (Lop_LD_POST_Q);
    case PLAYDOHop_LDSI_Q_C1_C2:
      return (Lop_LD_POST_Q);
    case PLAYDOHop_LDSI_Q_C1_C3:
      return (Lop_LD_POST_Q);
    case PLAYDOHop_LDSI_Q_C2_V1:
      return (Lop_LD_POST_Q);
    case PLAYDOHop_LDSI_Q_C2_C1:
      return (Lop_LD_POST_Q);
    case PLAYDOHop_LDSI_Q_C2_C2:
      return (Lop_LD_POST_Q);
    case PLAYDOHop_LDSI_Q_C2_C3:
      return (Lop_LD_POST_Q);
    case PLAYDOHop_LDSI_Q_C3_V1:
      return (Lop_LD_POST_Q);
    case PLAYDOHop_LDSI_Q_C3_C1:
      return (Lop_LD_POST_Q);
    case PLAYDOHop_LDSI_Q_C3_C2:
      return (Lop_LD_POST_Q);
    case PLAYDOHop_LDSI_Q_C3_C3:
      return (Lop_LD_POST_Q);

    case PLAYDOHop_FLDS_S_V1_V1:
      return (Lop_LD_F);
    case PLAYDOHop_FLDS_S_V1_C1:
      return (Lop_LD_F);
    case PLAYDOHop_FLDS_S_V1_C2:
      return (Lop_LD_F);
    case PLAYDOHop_FLDS_S_V1_C3:
      return (Lop_LD_F);
    case PLAYDOHop_FLDS_S_C1_V1:
      return (Lop_LD_F);
    case PLAYDOHop_FLDS_S_C1_C1:
      return (Lop_LD_F);
    case PLAYDOHop_FLDS_S_C1_C2:
      return (Lop_LD_F);
    case PLAYDOHop_FLDS_S_C1_C3:
      return (Lop_LD_F);
    case PLAYDOHop_FLDS_S_C2_V1:
      return (Lop_LD_F);
    case PLAYDOHop_FLDS_S_C2_C1:
      return (Lop_LD_F);
    case PLAYDOHop_FLDS_S_C2_C2:
      return (Lop_LD_F);
    case PLAYDOHop_FLDS_S_C2_C3:
      return (Lop_LD_F);
    case PLAYDOHop_FLDS_S_C3_V1:
      return (Lop_LD_F);
    case PLAYDOHop_FLDS_S_C3_C1:
      return (Lop_LD_F);
    case PLAYDOHop_FLDS_S_C3_C2:
      return (Lop_LD_F);
    case PLAYDOHop_FLDS_S_C3_C3:
      return (Lop_LD_F);

    case PLAYDOHop_FLDS_D_V1_V1:
      return (Lop_LD_F2);
    case PLAYDOHop_FLDS_D_V1_C1:
      return (Lop_LD_F2);
    case PLAYDOHop_FLDS_D_V1_C2:
      return (Lop_LD_F2);
    case PLAYDOHop_FLDS_D_V1_C3:
      return (Lop_LD_F2);
    case PLAYDOHop_FLDS_D_C1_V1:
      return (Lop_LD_F2);
    case PLAYDOHop_FLDS_D_C1_C1:
      return (Lop_LD_F2);
    case PLAYDOHop_FLDS_D_C1_C2:
      return (Lop_LD_F2);
    case PLAYDOHop_FLDS_D_C1_C3:
      return (Lop_LD_F2);
    case PLAYDOHop_FLDS_D_C2_V1:
      return (Lop_LD_F2);
    case PLAYDOHop_FLDS_D_C2_C1:
      return (Lop_LD_F2);
    case PLAYDOHop_FLDS_D_C2_C2:
      return (Lop_LD_F2);
    case PLAYDOHop_FLDS_D_C2_C3:
      return (Lop_LD_F2);
    case PLAYDOHop_FLDS_D_C3_V1:
      return (Lop_LD_F2);
    case PLAYDOHop_FLDS_D_C3_C1:
      return (Lop_LD_F2);
    case PLAYDOHop_FLDS_D_C3_C2:
      return (Lop_LD_F2);
    case PLAYDOHop_FLDS_D_C3_C3:
      return (Lop_LD_F2);

    case PLAYDOHop_FLDSI_S_V1_V1:
      return (Lop_LD_POST_F);
    case PLAYDOHop_FLDSI_S_V1_C1:
      return (Lop_LD_POST_F);
    case PLAYDOHop_FLDSI_S_V1_C2:
      return (Lop_LD_POST_F);
    case PLAYDOHop_FLDSI_S_V1_C3:
      return (Lop_LD_POST_F);
    case PLAYDOHop_FLDSI_S_C1_V1:
      return (Lop_LD_POST_F);
    case PLAYDOHop_FLDSI_S_C1_C1:
      return (Lop_LD_POST_F);
    case PLAYDOHop_FLDSI_S_C1_C2:
      return (Lop_LD_POST_F);
    case PLAYDOHop_FLDSI_S_C1_C3:
      return (Lop_LD_POST_F);
    case PLAYDOHop_FLDSI_S_C2_V1:
      return (Lop_LD_POST_F);
    case PLAYDOHop_FLDSI_S_C2_C1:
      return (Lop_LD_POST_F);
    case PLAYDOHop_FLDSI_S_C2_C2:
      return (Lop_LD_POST_F);
    case PLAYDOHop_FLDSI_S_C2_C3:
      return (Lop_LD_POST_F);
    case PLAYDOHop_FLDSI_S_C3_V1:
      return (Lop_LD_POST_F);
    case PLAYDOHop_FLDSI_S_C3_C1:
      return (Lop_LD_POST_F);
    case PLAYDOHop_FLDSI_S_C3_C2:
      return (Lop_LD_POST_F);
    case PLAYDOHop_FLDSI_S_C3_C3:
      return (Lop_LD_POST_F);

    case PLAYDOHop_FLDSI_D_V1_V1:
      return (Lop_LD_POST_F2);
    case PLAYDOHop_FLDSI_D_V1_C1:
      return (Lop_LD_POST_F2);
    case PLAYDOHop_FLDSI_D_V1_C2:
      return (Lop_LD_POST_F2);
    case PLAYDOHop_FLDSI_D_V1_C3:
      return (Lop_LD_POST_F2);
    case PLAYDOHop_FLDSI_D_C1_V1:
      return (Lop_LD_POST_F2);
    case PLAYDOHop_FLDSI_D_C1_C1:
      return (Lop_LD_POST_F2);
    case PLAYDOHop_FLDSI_D_C1_C2:
      return (Lop_LD_POST_F2);
    case PLAYDOHop_FLDSI_D_C1_C3:
      return (Lop_LD_POST_F2);
    case PLAYDOHop_FLDSI_D_C2_V1:
      return (Lop_LD_POST_F2);
    case PLAYDOHop_FLDSI_D_C2_C1:
      return (Lop_LD_POST_F2);
    case PLAYDOHop_FLDSI_D_C2_C2:
      return (Lop_LD_POST_F2);
    case PLAYDOHop_FLDSI_D_C2_C3:
      return (Lop_LD_POST_F2);
    case PLAYDOHop_FLDSI_D_C3_V1:
      return (Lop_LD_POST_F2);
    case PLAYDOHop_FLDSI_D_C3_C1:
      return (Lop_LD_POST_F2);
    case PLAYDOHop_FLDSI_D_C3_C2:
      return (Lop_LD_POST_F2);
    case PLAYDOHop_FLDSI_D_C3_C3:
      return (Lop_LD_POST_F2);

    case PLAYDOHop_LDV_B:
      return (Lop_LD_UC);
    case PLAYDOHop_LDV_H:
      return (Lop_LD_UC2);
    case PLAYDOHop_LDV_W:
      return (Lop_LD_I);
    case PLAYDOHop_LDV_Q:
      return (Lop_LD_Q);
    case PLAYDOHop_FLDV_S:
      return (Lop_LD_F);
    case PLAYDOHop_FLDV_D:
      return (Lop_LD_F2);

    case PLAYDOHop_SH1ADDL:
      return (Lop_LSL);
    case PLAYDOHop_SH2ADDL:
      return (Lop_LSL);
    case PLAYDOHop_SH3ADDL:
      return (Lop_LSL);

    case PLAYDOHop_FMPYADDN_S:
      return (Lop_MUL_ADD_F);
    case PLAYDOHop_FMPYADDN_D:
      return (Lop_MUL_ADD_F2);

    case PLAYDOHop_CMPR_FALSE:
      return (Lop_NE);
    case PLAYDOHop_CMPR_TRUE:
      return (Lop_EQ);
    case PLAYDOHop_CMPR_OD:
      return (Lop_EQ);
    case PLAYDOHop_CMPR_EV:
      return (Lop_EQ);

    case PLAYDOHop_FCMPR_S_FALSE:
      return (Lop_NE_F);
    case PLAYDOHop_FCMPR_S_TRUE:
      return (Lop_EQ_F);
    case PLAYDOHop_FCMPR_D_FALSE:
      return (Lop_NE_F2);
    case PLAYDOHop_FCMPR_D_TRUE:
      return (Lop_EQ_F2);

    case PLAYDOHop_M_NO_OP:
      return (Lop_NO_OP);

    case PLAYDOHop_MOVELB:
      return (Lop_MOV);
    case PLAYDOHop_MOVELBX:
      return (Lop_MOV);
    case PLAYDOHop_MOVELBS:
      return (Lop_MOV);
    case PLAYDOHop_PBRRL:
      return (Lop_PBR);
    case PLAYDOHop_PBRAL:
      return (Lop_PBR);
    case PLAYDOHop_PBRRLBS:
      return (Lop_PBR);
    case PLAYDOHop_PBRALBS:
      return (Lop_PBR);
    case PLAYDOHop_MOVELG:
      return (Lop_MOV);
    case PLAYDOHop_MOVELGX:
      return (Lop_MOV);
    case PLAYDOHop_MOVELGS:
      return (Lop_MOV);
    case PLAYDOHop_MOVELF:
      return (Lop_MOV_F);
    case PLAYDOHop_MOVELFS:
      return (Lop_MOV_F);

    case PLAYDOHop_MOVEGC:
      return (Lop_MOV);
    case PLAYDOHop_MOVECG:
      return (Lop_MOV);
    case PLAYDOHop_MOVEGG:
      return (Lop_MOV);
    case PLAYDOHop_MOVEBB:
      return (Lop_MOV);

    case PLAYDOHop_SAVE:
      return (Lop_ST_I);
    case PLAYDOHop_RESTORE:
      return (Lop_LD_I);
    case PLAYDOHop_FSAVE_S:
      return (Lop_ST_F);
    case PLAYDOHop_FRESTORE_S:
      return (Lop_LD_F);
    case PLAYDOHop_FSAVE_D:
      return (Lop_ST_F2);
    case PLAYDOHop_FRESTORE_D:
      return (Lop_LD_F2);
    case PLAYDOHop_BSAVE:
      return (Lop_ST_I);
    case PLAYDOHop_BRESTORE:
      return (Lop_LD_I);
    case PLAYDOHop_MOVEGBP:
      return (Lop_MOV);
    case PLAYDOHop_MOVEGCM:
      return (Lop_MOV);

      /* SLARSEN: Vector extract ops */
      case PLAYDOHop_VEXTRSB:		return (Lop_VEXTRACT_C);
      case PLAYDOHop_VEXTRSH:		return (Lop_VEXTRACT_C2);

	/* SLARSEN: Vector memory ops */
      case PLAYDOHop_VL_B_C1_C1:	return (Lop_VLD_C);
      case PLAYDOHop_VL_H_C1_C1:	return (Lop_VLD_C2);
      case PLAYDOHop_VL_W_C1_C1:	return (Lop_VLD_I);
      case PLAYDOHop_VFL_S_C1_C1:	return (Lop_VLD_F);
      case PLAYDOHop_VFL_D_C1_C1:	return (Lop_VLD_F2);
      case PLAYDOHop_VS_B_C1:		return (Lop_VST_C);
      case PLAYDOHop_VS_H_C1:		return (Lop_VST_C2);
      case PLAYDOHop_VS_W_C1:		return (Lop_VST_I);
      case PLAYDOHop_VFS_S_C1:	return (Lop_VST_F);
      case PLAYDOHop_VFS_D_C1:	return (Lop_VST_F2);

      case PLAYDOHop_VLE_B_C1_C1:	return (Lop_VLDE_C);
      case PLAYDOHop_VLE_H_C1_C1:	return (Lop_VLDE_C2);
      case PLAYDOHop_VLE_W_C1_C1:	return (Lop_VLDE_I);
      case PLAYDOHop_VFLE_S_C1_C1:	return (Lop_VLDE_F);
      case PLAYDOHop_VFLE_D_C1_C1:	return (Lop_VLDE_F2);
      case PLAYDOHop_VSE_B_C1:	return (Lop_VSTE_C);
      case PLAYDOHop_VSE_H_C1:	return (Lop_VSTE_C2);
      case PLAYDOHop_VSE_W_C1:	return (Lop_VSTE_I);
      case PLAYDOHop_VFSE_S_C1:	return (Lop_VSTE_F);
      case PLAYDOHop_VFSE_D_C1:	return (Lop_VSTE_F2);

    default:
      fprintf (stderr, "Illegal proc_opc %d\n", proc_opc);
      M_assert (0, "M_opc_from_proc_opc_playdoh: illegal proc_opc");
      return (-1);
    }
}

int M_base_displ_load_opcode(int proc_opc)
{
  switch(proc_opc) {
    case PLAYDOHop_LG_B_V1_V1:
    case PLAYDOHop_LG_B_V1_C1:
    case PLAYDOHop_LG_B_V1_C2:
    case PLAYDOHop_LG_B_V1_C3:
    case PLAYDOHop_LG_B_C1_V1:
    case PLAYDOHop_LG_B_C1_C1:
    case PLAYDOHop_LG_B_C1_C2:
    case PLAYDOHop_LG_B_C1_C3:
    case PLAYDOHop_LG_B_C2_V1:
    case PLAYDOHop_LG_B_C2_C1:
    case PLAYDOHop_LG_B_C2_C2:
    case PLAYDOHop_LG_B_C2_C3:
    case PLAYDOHop_LG_B_C3_V1:
    case PLAYDOHop_LG_B_C3_C1:
    case PLAYDOHop_LG_B_C3_C2:
    case PLAYDOHop_LG_B_C3_C3:
    case PLAYDOHop_LM_B_V1_V1:
    case PLAYDOHop_LM_B_V1_C1:
    case PLAYDOHop_LM_B_V1_C2:
    case PLAYDOHop_LM_B_V1_C3:
    case PLAYDOHop_LM_B_C1_V1:
    case PLAYDOHop_LM_B_C1_C1:
    case PLAYDOHop_LM_B_C1_C2:
    case PLAYDOHop_LM_B_C1_C3:
    case PLAYDOHop_LM_B_C2_V1:
    case PLAYDOHop_LM_B_C2_C1:
    case PLAYDOHop_LM_B_C2_C2:
    case PLAYDOHop_LM_B_C2_C3:
    case PLAYDOHop_LM_B_C3_V1:
    case PLAYDOHop_LM_B_C3_C1:
    case PLAYDOHop_LM_B_C3_C2:
    case PLAYDOHop_LM_B_C3_C3:
    case PLAYDOHop_LG_H_V1_V1:
    case PLAYDOHop_LG_H_V1_C1:
    case PLAYDOHop_LG_H_V1_C2:
    case PLAYDOHop_LG_H_V1_C3:
    case PLAYDOHop_LG_H_C1_V1:
    case PLAYDOHop_LG_H_C1_C1:
    case PLAYDOHop_LG_H_C1_C2:
    case PLAYDOHop_LG_H_C1_C3:
    case PLAYDOHop_LG_H_C2_V1:
    case PLAYDOHop_LG_H_C2_C1:
    case PLAYDOHop_LG_H_C2_C2:
    case PLAYDOHop_LG_H_C2_C3:
    case PLAYDOHop_LG_H_C3_V1:
    case PLAYDOHop_LG_H_C3_C1:
    case PLAYDOHop_LG_H_C3_C2:
    case PLAYDOHop_LG_H_C3_C3:
    case PLAYDOHop_LM_H_V1_V1:
    case PLAYDOHop_LM_H_V1_C1:
    case PLAYDOHop_LM_H_V1_C2:
    case PLAYDOHop_LM_H_V1_C3:
    case PLAYDOHop_LM_H_C1_V1:
    case PLAYDOHop_LM_H_C1_C1:
    case PLAYDOHop_LM_H_C1_C2:
    case PLAYDOHop_LM_H_C1_C3:
    case PLAYDOHop_LM_H_C2_V1:
    case PLAYDOHop_LM_H_C2_C1:
    case PLAYDOHop_LM_H_C2_C2:
    case PLAYDOHop_LM_H_C2_C3:
    case PLAYDOHop_LM_H_C3_V1:
    case PLAYDOHop_LM_H_C3_C1:
    case PLAYDOHop_LM_H_C3_C2:
    case PLAYDOHop_LM_H_C3_C3:
    case PLAYDOHop_LGX_B_V1_V1:
    case PLAYDOHop_LGX_B_V1_C1:
    case PLAYDOHop_LGX_B_V1_C2:
    case PLAYDOHop_LGX_B_V1_C3:
    case PLAYDOHop_LGX_B_C1_V1:
    case PLAYDOHop_LGX_B_C1_C1:
    case PLAYDOHop_LGX_B_C1_C2:
    case PLAYDOHop_LGX_B_C1_C3:
    case PLAYDOHop_LGX_B_C2_V1:
    case PLAYDOHop_LGX_B_C2_C1:
    case PLAYDOHop_LGX_B_C2_C2:
    case PLAYDOHop_LGX_B_C2_C3:
    case PLAYDOHop_LGX_B_C3_V1:
    case PLAYDOHop_LGX_B_C3_C1:
    case PLAYDOHop_LGX_B_C3_C2:
    case PLAYDOHop_LGX_B_C3_C3:
    case PLAYDOHop_LMX_B_V1_V1:
    case PLAYDOHop_LMX_B_V1_C1:
    case PLAYDOHop_LMX_B_V1_C2:
    case PLAYDOHop_LMX_B_V1_C3:
    case PLAYDOHop_LMX_B_C1_V1:
    case PLAYDOHop_LMX_B_C1_C1:
    case PLAYDOHop_LMX_B_C1_C2:
    case PLAYDOHop_LMX_B_C1_C3:
    case PLAYDOHop_LMX_B_C2_V1:
    case PLAYDOHop_LMX_B_C2_C1:
    case PLAYDOHop_LMX_B_C2_C2:
    case PLAYDOHop_LMX_B_C2_C3:
    case PLAYDOHop_LMX_B_C3_V1:
    case PLAYDOHop_LMX_B_C3_C1:
    case PLAYDOHop_LMX_B_C3_C2:
    case PLAYDOHop_LMX_B_C3_C3:
    case PLAYDOHop_LGX_H_V1_V1:
    case PLAYDOHop_LGX_H_V1_C1:
    case PLAYDOHop_LGX_H_V1_C2:
    case PLAYDOHop_LGX_H_V1_C3:
    case PLAYDOHop_LGX_H_C1_V1:
    case PLAYDOHop_LGX_H_C1_C1:
    case PLAYDOHop_LGX_H_C1_C2:
    case PLAYDOHop_LGX_H_C1_C3:
    case PLAYDOHop_LGX_H_C2_V1:
    case PLAYDOHop_LGX_H_C2_C1:
    case PLAYDOHop_LGX_H_C2_C2:
    case PLAYDOHop_LGX_H_C2_C3:
    case PLAYDOHop_LGX_H_C3_V1:
    case PLAYDOHop_LGX_H_C3_C1:
    case PLAYDOHop_LGX_H_C3_C2:
    case PLAYDOHop_LGX_H_C3_C3:
    case PLAYDOHop_LMX_H_V1_V1:
    case PLAYDOHop_LMX_H_V1_C1:
    case PLAYDOHop_LMX_H_V1_C2:
    case PLAYDOHop_LMX_H_V1_C3:
    case PLAYDOHop_LMX_H_C1_V1:
    case PLAYDOHop_LMX_H_C1_C1:
    case PLAYDOHop_LMX_H_C1_C2:
    case PLAYDOHop_LMX_H_C1_C3:
    case PLAYDOHop_LMX_H_C2_V1:
    case PLAYDOHop_LMX_H_C2_C1:
    case PLAYDOHop_LMX_H_C2_C2:
    case PLAYDOHop_LMX_H_C2_C3:
    case PLAYDOHop_LMX_H_C3_V1:
    case PLAYDOHop_LMX_H_C3_C1:
    case PLAYDOHop_LMX_H_C3_C2:
    case PLAYDOHop_LMX_H_C3_C3:
    case PLAYDOHop_LG_W_V1_V1:
    case PLAYDOHop_LG_W_V1_C1:
    case PLAYDOHop_LG_W_V1_C2:
    case PLAYDOHop_LG_W_V1_C3:
    case PLAYDOHop_LG_W_C1_V1:
    case PLAYDOHop_LG_W_C1_C1:
    case PLAYDOHop_LG_W_C1_C2:
    case PLAYDOHop_LG_W_C1_C3:
    case PLAYDOHop_LG_W_C2_V1:
    case PLAYDOHop_LG_W_C2_C1:
    case PLAYDOHop_LG_W_C2_C2:
    case PLAYDOHop_LG_W_C2_C3:
    case PLAYDOHop_LG_W_C3_V1:
    case PLAYDOHop_LG_W_C3_C1:
    case PLAYDOHop_LG_W_C3_C2:
    case PLAYDOHop_LG_W_C3_C3:
    case PLAYDOHop_LM_W_V1_V1:
    case PLAYDOHop_LM_W_V1_C1:
    case PLAYDOHop_LM_W_V1_C2:
    case PLAYDOHop_LM_W_V1_C3:
    case PLAYDOHop_LM_W_C1_V1:
    case PLAYDOHop_LM_W_C1_C1:
    case PLAYDOHop_LM_W_C1_C2:
    case PLAYDOHop_LM_W_C1_C3:
    case PLAYDOHop_LM_W_C2_V1:
    case PLAYDOHop_LM_W_C2_C1:
    case PLAYDOHop_LM_W_C2_C2:
    case PLAYDOHop_LM_W_C2_C3:
    case PLAYDOHop_LM_W_C3_V1:
    case PLAYDOHop_LM_W_C3_C1:
    case PLAYDOHop_LM_W_C3_C2:
    case PLAYDOHop_LM_W_C3_C3:
    case PLAYDOHop_LG_Q_V1_V1:
    case PLAYDOHop_LG_Q_V1_C1:
    case PLAYDOHop_LG_Q_V1_C2:
    case PLAYDOHop_LG_Q_V1_C3:
    case PLAYDOHop_LG_Q_C1_V1:
    case PLAYDOHop_LG_Q_C1_C1:
    case PLAYDOHop_LG_Q_C1_C2:
    case PLAYDOHop_LG_Q_C1_C3:
    case PLAYDOHop_LG_Q_C2_V1:
    case PLAYDOHop_LG_Q_C2_C1:
    case PLAYDOHop_LG_Q_C2_C2:
    case PLAYDOHop_LG_Q_C2_C3:
    case PLAYDOHop_LG_Q_C3_V1:
    case PLAYDOHop_LG_Q_C3_C1:
    case PLAYDOHop_LG_Q_C3_C2:
    case PLAYDOHop_LG_Q_C3_C3:
    case PLAYDOHop_LM_Q_V1_V1:
    case PLAYDOHop_LM_Q_V1_C1:
    case PLAYDOHop_LM_Q_V1_C2:
    case PLAYDOHop_LM_Q_V1_C3:
    case PLAYDOHop_LM_Q_C1_V1:
    case PLAYDOHop_LM_Q_C1_C1:
    case PLAYDOHop_LM_Q_C1_C2:
    case PLAYDOHop_LM_Q_C1_C3:
    case PLAYDOHop_LM_Q_C2_V1:
    case PLAYDOHop_LM_Q_C2_C1:
    case PLAYDOHop_LM_Q_C2_C2:
    case PLAYDOHop_LM_Q_C2_C3:
    case PLAYDOHop_LM_Q_C3_V1:
    case PLAYDOHop_LM_Q_C3_C1:
    case PLAYDOHop_LM_Q_C3_C2:
    case PLAYDOHop_LM_Q_C3_C3:
    case PLAYDOHop_FLG_S_V1_V1:
    case PLAYDOHop_FLG_S_V1_C1:
    case PLAYDOHop_FLG_S_V1_C2:
    case PLAYDOHop_FLG_S_V1_C3:
    case PLAYDOHop_FLG_S_C1_V1:
    case PLAYDOHop_FLG_S_C1_C1:
    case PLAYDOHop_FLG_S_C1_C2:
    case PLAYDOHop_FLG_S_C1_C3:
    case PLAYDOHop_FLG_S_C2_V1:
    case PLAYDOHop_FLG_S_C2_C1:
    case PLAYDOHop_FLG_S_C2_C2:
    case PLAYDOHop_FLG_S_C2_C3:
    case PLAYDOHop_FLG_S_C3_V1:
    case PLAYDOHop_FLG_S_C3_C1:
    case PLAYDOHop_FLG_S_C3_C2:
    case PLAYDOHop_FLG_S_C3_C3:
    case PLAYDOHop_FLG_D_V1_V1:
    case PLAYDOHop_FLG_D_V1_C1:
    case PLAYDOHop_FLG_D_V1_C2:
    case PLAYDOHop_FLG_D_V1_C3:
    case PLAYDOHop_FLG_D_C1_V1:
    case PLAYDOHop_FLG_D_C1_C1:
    case PLAYDOHop_FLG_D_C1_C2:
    case PLAYDOHop_FLG_D_C1_C3:
    case PLAYDOHop_FLG_D_C2_V1:
    case PLAYDOHop_FLG_D_C2_C1:
    case PLAYDOHop_FLG_D_C2_C2:
    case PLAYDOHop_FLG_D_C2_C3:
    case PLAYDOHop_FLG_D_C3_V1:
    case PLAYDOHop_FLG_D_C3_C1:
    case PLAYDOHop_FLG_D_C3_C2:
    case PLAYDOHop_FLG_D_C3_C3:
    case PLAYDOHop_FLM_S_V1_V1:
    case PLAYDOHop_FLM_S_V1_C1:
    case PLAYDOHop_FLM_S_V1_C2:
    case PLAYDOHop_FLM_S_V1_C3:
    case PLAYDOHop_FLM_S_C1_V1:
    case PLAYDOHop_FLM_S_C1_C1:
    case PLAYDOHop_FLM_S_C1_C2:
    case PLAYDOHop_FLM_S_C1_C3:
    case PLAYDOHop_FLM_S_C2_V1:
    case PLAYDOHop_FLM_S_C2_C1:
    case PLAYDOHop_FLM_S_C2_C2:
    case PLAYDOHop_FLM_S_C2_C3:
    case PLAYDOHop_FLM_S_C3_V1:
    case PLAYDOHop_FLM_S_C3_C1:
    case PLAYDOHop_FLM_S_C3_C2:
    case PLAYDOHop_FLM_S_C3_C3:
    case PLAYDOHop_FLM_D_V1_V1:
    case PLAYDOHop_FLM_D_V1_C1:
    case PLAYDOHop_FLM_D_V1_C2:
    case PLAYDOHop_FLM_D_V1_C3:
    case PLAYDOHop_FLM_D_C1_V1:
    case PLAYDOHop_FLM_D_C1_C1:
    case PLAYDOHop_FLM_D_C1_C2:
    case PLAYDOHop_FLM_D_C1_C3:
    case PLAYDOHop_FLM_D_C2_V1:
    case PLAYDOHop_FLM_D_C2_C1:
    case PLAYDOHop_FLM_D_C2_C2:
    case PLAYDOHop_FLM_D_C2_C3:
    case PLAYDOHop_FLM_D_C3_V1:
    case PLAYDOHop_FLM_D_C3_C1:
    case PLAYDOHop_FLM_D_C3_C2:
    case PLAYDOHop_FLM_D_C3_C3:
      return 1;
    default:
      return 0;
  }
}

int M_sign_extend_load_opcode(int proc_opc)
{
  switch(proc_opc) {
    case PLAYDOHop_LGX_B_V1_V1:
    case PLAYDOHop_LGX_B_V1_C1:
    case PLAYDOHop_LGX_B_V1_C2:
    case PLAYDOHop_LGX_B_V1_C3:
    case PLAYDOHop_LGX_B_C1_V1:
    case PLAYDOHop_LGX_B_C1_C1:
    case PLAYDOHop_LGX_B_C1_C2:
    case PLAYDOHop_LGX_B_C1_C3:
    case PLAYDOHop_LGX_B_C2_V1:
    case PLAYDOHop_LGX_B_C2_C1:
    case PLAYDOHop_LGX_B_C2_C2:
    case PLAYDOHop_LGX_B_C2_C3:
    case PLAYDOHop_LGX_B_C3_V1:
    case PLAYDOHop_LGX_B_C3_C1:
    case PLAYDOHop_LGX_B_C3_C2:
    case PLAYDOHop_LGX_B_C3_C3:
    case PLAYDOHop_LMX_B_V1_V1:
    case PLAYDOHop_LMX_B_V1_C1:
    case PLAYDOHop_LMX_B_V1_C2:
    case PLAYDOHop_LMX_B_V1_C3:
    case PLAYDOHop_LMX_B_C1_V1:
    case PLAYDOHop_LMX_B_C1_C1:
    case PLAYDOHop_LMX_B_C1_C2:
    case PLAYDOHop_LMX_B_C1_C3:
    case PLAYDOHop_LMX_B_C2_V1:
    case PLAYDOHop_LMX_B_C2_C1:
    case PLAYDOHop_LMX_B_C2_C2:
    case PLAYDOHop_LMX_B_C2_C3:
    case PLAYDOHop_LMX_B_C3_V1:
    case PLAYDOHop_LMX_B_C3_C1:
    case PLAYDOHop_LMX_B_C3_C2:
    case PLAYDOHop_LMX_B_C3_C3:

    case PLAYDOHop_LGX_H_V1_V1:
    case PLAYDOHop_LGX_H_V1_C1:
    case PLAYDOHop_LGX_H_V1_C2:
    case PLAYDOHop_LGX_H_V1_C3:
    case PLAYDOHop_LGX_H_C1_V1:
    case PLAYDOHop_LGX_H_C1_C1:
    case PLAYDOHop_LGX_H_C1_C2:
    case PLAYDOHop_LGX_H_C1_C3:
    case PLAYDOHop_LGX_H_C2_V1:
    case PLAYDOHop_LGX_H_C2_C1:
    case PLAYDOHop_LGX_H_C2_C2:
    case PLAYDOHop_LGX_H_C2_C3:
    case PLAYDOHop_LGX_H_C3_V1:
    case PLAYDOHop_LGX_H_C3_C1:
    case PLAYDOHop_LGX_H_C3_C2:
    case PLAYDOHop_LGX_H_C3_C3:
    case PLAYDOHop_LMX_H_V1_V1:
    case PLAYDOHop_LMX_H_V1_C1:
    case PLAYDOHop_LMX_H_V1_C2:
    case PLAYDOHop_LMX_H_V1_C3:
    case PLAYDOHop_LMX_H_C1_V1:
    case PLAYDOHop_LMX_H_C1_C1:
    case PLAYDOHop_LMX_H_C1_C2:
    case PLAYDOHop_LMX_H_C1_C3:
    case PLAYDOHop_LMX_H_C2_V1:
    case PLAYDOHop_LMX_H_C2_C1:
    case PLAYDOHop_LMX_H_C2_C2:
    case PLAYDOHop_LMX_H_C2_C3:
    case PLAYDOHop_LMX_H_C3_V1:
    case PLAYDOHop_LMX_H_C3_C1:
    case PLAYDOHop_LMX_H_C3_C2:
    case PLAYDOHop_LMX_H_C3_C3:

    case PLAYDOHop_LGX_W_V1_V1:
    case PLAYDOHop_LGX_W_V1_C1:
    case PLAYDOHop_LGX_W_V1_C2:
    case PLAYDOHop_LGX_W_V1_C3:
    case PLAYDOHop_LGX_W_C1_V1:
    case PLAYDOHop_LGX_W_C1_C1:
    case PLAYDOHop_LGX_W_C1_C2:
    case PLAYDOHop_LGX_W_C1_C3:
    case PLAYDOHop_LGX_W_C2_V1:
    case PLAYDOHop_LGX_W_C2_C1:
    case PLAYDOHop_LGX_W_C2_C2:
    case PLAYDOHop_LGX_W_C2_C3:
    case PLAYDOHop_LGX_W_C3_V1:
    case PLAYDOHop_LGX_W_C3_C1:
    case PLAYDOHop_LGX_W_C3_C2:
    case PLAYDOHop_LGX_W_C3_C3:
    case PLAYDOHop_LMX_W_V1_V1:
    case PLAYDOHop_LMX_W_V1_C1:
    case PLAYDOHop_LMX_W_V1_C2:
    case PLAYDOHop_LMX_W_V1_C3:
    case PLAYDOHop_LMX_W_C1_V1:
    case PLAYDOHop_LMX_W_C1_C1:
    case PLAYDOHop_LMX_W_C1_C2:
    case PLAYDOHop_LMX_W_C1_C3:
    case PLAYDOHop_LMX_W_C2_V1:
    case PLAYDOHop_LMX_W_C2_C1:
    case PLAYDOHop_LMX_W_C2_C2:
    case PLAYDOHop_LMX_W_C2_C3:
    case PLAYDOHop_LMX_W_C3_V1:
    case PLAYDOHop_LMX_W_C3_C1:
    case PLAYDOHop_LMX_W_C3_C2:
    case PLAYDOHop_LMX_W_C3_C3:

    case PLAYDOHop_LGX_Q_V1_V1:
    case PLAYDOHop_LGX_Q_V1_C1:
    case PLAYDOHop_LGX_Q_V1_C2:
    case PLAYDOHop_LGX_Q_V1_C3:
    case PLAYDOHop_LGX_Q_C1_V1:
    case PLAYDOHop_LGX_Q_C1_C1:
    case PLAYDOHop_LGX_Q_C1_C2:
    case PLAYDOHop_LGX_Q_C1_C3:
    case PLAYDOHop_LGX_Q_C2_V1:
    case PLAYDOHop_LGX_Q_C2_C1:
    case PLAYDOHop_LGX_Q_C2_C2:
    case PLAYDOHop_LGX_Q_C2_C3:
    case PLAYDOHop_LGX_Q_C3_V1:
    case PLAYDOHop_LGX_Q_C3_C1:
    case PLAYDOHop_LGX_Q_C3_C2:
    case PLAYDOHop_LGX_Q_C3_C3:
    case PLAYDOHop_LMX_Q_V1_V1:
    case PLAYDOHop_LMX_Q_V1_C1:
    case PLAYDOHop_LMX_Q_V1_C2:
    case PLAYDOHop_LMX_Q_V1_C3:
    case PLAYDOHop_LMX_Q_C1_V1:
    case PLAYDOHop_LMX_Q_C1_C1:
    case PLAYDOHop_LMX_Q_C1_C2:
    case PLAYDOHop_LMX_Q_C1_C3:
    case PLAYDOHop_LMX_Q_C2_V1:
    case PLAYDOHop_LMX_Q_C2_C1:
    case PLAYDOHop_LMX_Q_C2_C2:
    case PLAYDOHop_LMX_Q_C2_C3:
    case PLAYDOHop_LMX_Q_C3_V1:
    case PLAYDOHop_LMX_Q_C3_C1:
    case PLAYDOHop_LMX_Q_C3_C2:
    case PLAYDOHop_LMX_Q_C3_C3:
    
    case PLAYDOHop_LX_B_V1_V1:
    case PLAYDOHop_LX_B_V1_C1:
    case PLAYDOHop_LX_B_V1_C2:
    case PLAYDOHop_LX_B_V1_C3:
    case PLAYDOHop_LX_B_C1_V1:
    case PLAYDOHop_LX_B_C1_C1:
    case PLAYDOHop_LX_B_C1_C2:
    case PLAYDOHop_LX_B_C1_C3:
    case PLAYDOHop_LX_B_C2_V1:
    case PLAYDOHop_LX_B_C2_C1:
    case PLAYDOHop_LX_B_C2_C2:
    case PLAYDOHop_LX_B_C2_C3:
    case PLAYDOHop_LX_B_C3_V1:
    case PLAYDOHop_LX_B_C3_C1:
    case PLAYDOHop_LX_B_C3_C2:
    case PLAYDOHop_LX_B_C3_C3:

    case PLAYDOHop_LX_H_V1_V1:
    case PLAYDOHop_LX_H_V1_C1:
    case PLAYDOHop_LX_H_V1_C2:
    case PLAYDOHop_LX_H_V1_C3:
    case PLAYDOHop_LX_H_C1_V1:
    case PLAYDOHop_LX_H_C1_C1:
    case PLAYDOHop_LX_H_C1_C2:
    case PLAYDOHop_LX_H_C1_C3:
    case PLAYDOHop_LX_H_C2_V1:
    case PLAYDOHop_LX_H_C2_C1:
    case PLAYDOHop_LX_H_C2_C2:
    case PLAYDOHop_LX_H_C2_C3:
    case PLAYDOHop_LX_H_C3_V1:
    case PLAYDOHop_LX_H_C3_C1:
    case PLAYDOHop_LX_H_C3_C2:
    case PLAYDOHop_LX_H_C3_C3:

    case PLAYDOHop_LX_W_V1_V1:
    case PLAYDOHop_LX_W_V1_C1:
    case PLAYDOHop_LX_W_V1_C2:
    case PLAYDOHop_LX_W_V1_C3:
    case PLAYDOHop_LX_W_C1_V1:
    case PLAYDOHop_LX_W_C1_C1:
    case PLAYDOHop_LX_W_C1_C2:
    case PLAYDOHop_LX_W_C1_C3:
    case PLAYDOHop_LX_W_C2_V1:
    case PLAYDOHop_LX_W_C2_C1:
    case PLAYDOHop_LX_W_C2_C2:
    case PLAYDOHop_LX_W_C2_C3:
    case PLAYDOHop_LX_W_C3_V1:
    case PLAYDOHop_LX_W_C3_C1:
    case PLAYDOHop_LX_W_C3_C2:
    case PLAYDOHop_LX_W_C3_C3:

    case PLAYDOHop_LX_Q_V1_V1:
    case PLAYDOHop_LX_Q_V1_C1:
    case PLAYDOHop_LX_Q_V1_C2:
    case PLAYDOHop_LX_Q_V1_C3:
    case PLAYDOHop_LX_Q_C1_V1:
    case PLAYDOHop_LX_Q_C1_C1:
    case PLAYDOHop_LX_Q_C1_C2:
    case PLAYDOHop_LX_Q_C1_C3:
    case PLAYDOHop_LX_Q_C2_V1:
    case PLAYDOHop_LX_Q_C2_C1:
    case PLAYDOHop_LX_Q_C2_C2:
    case PLAYDOHop_LX_Q_C2_C3:
    case PLAYDOHop_LX_Q_C3_V1:
    case PLAYDOHop_LX_Q_C3_C1:
    case PLAYDOHop_LX_Q_C3_C2:
    case PLAYDOHop_LX_Q_C3_C3:

      return 1;
    default :
      return 0;
  }
}

int M_base_displ_store_opcode(int proc_opc)
{
  switch(proc_opc) {
    case PLAYDOHop_SG_B_V1:
    case PLAYDOHop_SG_B_C1:
    case PLAYDOHop_SG_B_C2:
    case PLAYDOHop_SG_B_C3:
    case PLAYDOHop_SG_H_V1:
    case PLAYDOHop_SG_H_C1:
    case PLAYDOHop_SG_H_C2:
    case PLAYDOHop_SG_H_C3:
    case PLAYDOHop_SG_W_V1:
    case PLAYDOHop_SG_W_C1:
    case PLAYDOHop_SG_W_C2:
    case PLAYDOHop_SG_W_C3:
    case PLAYDOHop_SG_Q_V1:
    case PLAYDOHop_SG_Q_C1:
    case PLAYDOHop_SG_Q_C2:
    case PLAYDOHop_SG_Q_C3:
    case PLAYDOHop_SM_B_V1:
    case PLAYDOHop_SM_B_C1:
    case PLAYDOHop_SM_B_C2:
    case PLAYDOHop_SM_B_C3:
    case PLAYDOHop_SM_H_V1:
    case PLAYDOHop_SM_H_C1:
    case PLAYDOHop_SM_H_C2:
    case PLAYDOHop_SM_H_C3:
    case PLAYDOHop_SM_W_V1:
    case PLAYDOHop_SM_W_C1:
    case PLAYDOHop_SM_W_C2:
    case PLAYDOHop_SM_W_C3:
    case PLAYDOHop_SM_Q_V1:
    case PLAYDOHop_SM_Q_C1:
    case PLAYDOHop_SM_Q_C2:
    case PLAYDOHop_SM_Q_C3:
    case PLAYDOHop_FSG_S_V1:
    case PLAYDOHop_FSG_S_C1:
    case PLAYDOHop_FSG_S_C2:
    case PLAYDOHop_FSG_S_C3:
    case PLAYDOHop_FSM_S_V1:
    case PLAYDOHop_FSM_S_C1:
    case PLAYDOHop_FSM_S_C2:
    case PLAYDOHop_FSM_S_C3:
    case PLAYDOHop_FSG_D_V1:
    case PLAYDOHop_FSG_D_C1:
    case PLAYDOHop_FSG_D_C2:
    case PLAYDOHop_FSG_D_C3:
    case PLAYDOHop_FSM_D_V1:
    case PLAYDOHop_FSM_D_C1:
    case PLAYDOHop_FSM_D_C2:
    case PLAYDOHop_FSM_D_C3:
      return 1;
    default:
      return 0;
  }
}
