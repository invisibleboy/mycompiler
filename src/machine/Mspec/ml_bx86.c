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
 *	File :	ml_bx86.c 
 *	Desc :	Machine dependent specification.  
 *      Date :  1997
 *      Auth :  Matt Merten, Mike Thiems, Wen-mei Hwu
 *
 *    Modified : modified from m_spec.c code by Pohua Paul Chang.
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
#ifdef M_BX86_FOR_HCODE
#include <Hcode/h_ccode.h>
#endif

#include <library/i_error.h>
#include <Lcode/l_main.h>
#include "m_spec.h"
#include "m_bx86.h"

/*--------------------------------------------------------------------------*/

#define M_BX86_SIZE_VOID	0
#define M_BX86_SIZE_BIT		1
#define M_BX86_SIZE_CHAR	8
#define M_BX86_SIZE_SHORT	16
#define M_BX86_SIZE_INT		32
#define M_BX86_SIZE_LONG	32
#define M_BX86_SIZE_FLOAT	32
#define M_BX86_SIZE_DOUBLE	64
#define M_BX86_SIZE_POINTER	32
#define M_BX86_SIZE_UNION	-1
#define M_BX86_SIZE_STRUCT	-1
#define M_BX86_SIZE_BLOCK	-1
#define M_BX86_SIZE_MAX	 	64

#define M_BX86_ALIGN_VOID	-1
#define M_BX86_ALIGN_BIT	1
#define M_BX86_ALIGN_CHAR	8
#define M_BX86_ALIGN_SHORT	16
#define M_BX86_ALIGN_INT	32
#define M_BX86_ALIGN_LONG	32
#define M_BX86_ALIGN_FLOAT	32
#define M_BX86_ALIGN_DOUBLE	32
#define M_BX86_ALIGN_POINTER	32
#define M_BX86_ALIGN_UNION	-1	/* depends on the field */
#define M_BX86_ALIGN_STRUCT	-1
#define M_BX86_ALIGN_BLOCK	-1
#define M_BX86_ALIGN_MAX	64



/*--------------------------------------------------------------------------*/

#define M_BX86_MAX_FNVAR_REG 		0
#define M_BX86_SMALL_STRUCT_MAX 	04
#define MIN_PARAM_SIZE 	  		(16 * 0)

/* incoming and outgoing parameters */
#define M_BX86_INT_BASE			0
#define M_BX86_FLT_BASE			4

#define M_BX86_RET_I32			0	/* return in EAX */
#define M_BX86_RET_I64			0	/* return in EAX and EDX */
#define M_BX86_RET_ST			0	/* no special pointer */
#define M_BX86_RET_F			1	/* return in EAX */
#define M_BX86_RET_F2			1	/* return in EAX */
#define M_BX86_VAR_PTR	       		"$LV"

/*--------------------------------------------------------------------------*/
/*
 *	The following functions are for documentation only.
 *	The function overhead is too great for actual usage.
 */
void
M_bx86_void (M_Type type)
{
  I_punt ("ml_bx86: Unimplemented: M_bx86_void");
  type->type = M_TYPE_VOID;
  type->unsign = 1;
  type->align = M_BX86_ALIGN_VOID;
  type->size = M_BX86_SIZE_VOID;
  type->nbits = 0;
}

void
M_bx86_bit_long (M_Type type, int n)
{
  I_punt ("ml_bx86: Unimplemented: M_bx86_bit_long");
  type->type = M_TYPE_BIT_LONG;
  type->unsign = 0;
  type->align = M_BX86_ALIGN_BIT;
  type->size = n * M_BX86_SIZE_BIT;
  type->nbits = n * M_BX86_SIZE_BIT;
  M_assert ((n <= 32),
	    "M_bit_long: do not allow bit field of more than 32 bits");
}

void
M_bx86_bit_int (M_Type type, int n)
{
  I_punt ("ml_bx86: Unimplemented: M_bx86_bit_int");
  type->type = M_TYPE_BIT_INT;
  type->unsign = 0;
  type->align = M_BX86_ALIGN_BIT;
  type->size = n * M_BX86_SIZE_BIT;
  type->nbits = n * M_BX86_SIZE_BIT;
  M_assert ((n <= 32),
	    "M_bit_int: do not allow bit field of more than 32 bits");
}

void
M_bx86_bit_short (M_Type type, int n)
{
  I_punt ("ml_bx86: Unimplemented: M_bx86_bit_short");
  type->type = M_TYPE_BIT_SHORT;
  type->unsign = 0;
  type->align = M_BX86_ALIGN_BIT;
  type->size = n * M_BX86_SIZE_BIT;
  type->nbits = n * M_BX86_SIZE_BIT;
  M_assert ((n <= 16),
	    "M_bit_long: do not allow bit field of more than 16 bits");
}

void
M_bx86_bit_char (M_Type type, int n)
{
  I_punt ("ml_bx86: Unimplemented: M_bx86_bit_char");
  type->type = M_TYPE_BIT_CHAR;
  type->unsign = 0;
  type->align = M_BX86_ALIGN_BIT;
  type->size = n * M_BX86_SIZE_BIT;
  type->nbits = n * M_BX86_SIZE_BIT;
  M_assert ((n <= 8),
	    "M_bit_char: do not allow bit field of more than 8 bits");
}

void
M_bx86_float (M_Type type, int unsign)
{
  I_punt ("ml_bx86: Unimplemented: M_bx86_float");
  type->type = M_TYPE_FLOAT;
  type->unsign = unsign;
  type->align = M_BX86_ALIGN_FLOAT;
  type->size = M_BX86_SIZE_FLOAT;
  type->nbits = M_BX86_SIZE_FLOAT;
}

void
M_bx86_double (M_Type type, int unsign)
{
  I_punt ("ml_bx86: Unimplemented: M_bx86_double");
  type->type = M_TYPE_DOUBLE;
  type->unsign = unsign;
  type->align = M_BX86_ALIGN_DOUBLE;
  type->size = M_BX86_SIZE_DOUBLE;
  type->nbits = M_BX86_SIZE_DOUBLE;
}

void
M_bx86_pointer (M_Type type)
{
  I_punt ("ml_bx86: Unimplemented: M_bx86_pointer");
  type->type = M_TYPE_POINTER;
  type->unsign = 1;
  type->align = M_BX86_ALIGN_POINTER;
  type->size = M_BX86_SIZE_POINTER;
  type->nbits = M_BX86_SIZE_POINTER;
}

/*--------------------------------------------------------------------------*/
int
M_bx86_eval_type (M_Type type, M_Type ntype)
{
  I_punt ("ml_bx86: Unimplemented: M_bx86_eval_type");
  switch (type->type)
    {
    case M_TYPE_VOID:
      M_void (ntype);
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
      M_int (ntype, type->unsign);
      return (M_TYPE_INT);
    case M_TYPE_FLOAT:
      M_float (ntype, type->unsign);
      return (M_TYPE_FLOAT);
    case M_TYPE_DOUBLE:
      M_double (ntype, type->unsign);
      return (M_TYPE_DOUBLE);
    case M_TYPE_UNION:
    case M_TYPE_STRUCT:
      *ntype = *type;
      return type->type;
    default:
      return (-1);
    }
}

int
M_bx86_eval_type2 (M_Type type, M_Type ntype)
{
  I_punt ("ml_bx86: Unimplemented: M_bx86_eval_type2");
  switch (type->type)
    {
    case M_TYPE_VOID:
      M_void (ntype);
      return (-1);		/* can not be evaluated */
    case M_TYPE_BIT_CHAR:
    case M_TYPE_CHAR:
      M_char (ntype, type->unsign);
      return (M_TYPE_CHAR);
    case M_TYPE_SHORT:
      M_short (ntype, type->unsign);
      return (M_TYPE_SHORT);
    case M_TYPE_BLOCK:
    case M_TYPE_INT:
      /* the starting address of array is used */
      M_int (ntype, type->unsign);
      return (M_TYPE_INT);
    case M_TYPE_BIT_LONG:
    case M_TYPE_LONG:
      M_long (ntype, type->unsign);
      return (M_TYPE_LONG);
    case M_TYPE_POINTER:
      M_pointer (ntype);
      return (M_TYPE_POINTER);
    case M_TYPE_FLOAT:
      M_float (ntype, type->unsign);
      return (M_TYPE_FLOAT);
    case M_TYPE_DOUBLE:
      M_double (ntype, type->unsign);
      return (M_TYPE_DOUBLE);
    case M_TYPE_UNION:
    case M_TYPE_STRUCT:
      *ntype = *type;
      return type->type;
    default:
      return (-1);
    }
}

int
M_bx86_call_type (M_Type type, M_Type ntype)
{
  I_punt ("ml_bx86: Unimplemented: M_bx86_call_type");
  switch (type->type)
    {
    case M_TYPE_VOID:
      M_void (ntype);
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
      M_int (ntype, type->unsign);
      return (M_TYPE_INT);
      /* BCC - don't promote float to double now - 8/5/96 */
    case M_TYPE_FLOAT:
      M_float (ntype, type->unsign);
      return M_TYPE_FLOAT;
    case M_TYPE_DOUBLE:
      M_double (ntype, type->unsign);
      return M_TYPE_DOUBLE;
    case M_TYPE_UNION:
    case M_TYPE_STRUCT:
      *ntype = *type;
      return (type->type);
    default:
      return (-1);
    }
}

int
M_bx86_call_type2 (M_Type type, M_Type ntype)
{
  I_punt ("ml_bx86: Unimplemented: M_bx86_call_type2");
  switch (type->type)
    {
    case M_TYPE_VOID:
      M_void (ntype);
      return (-1);		/* can not be evaluated */
    case M_TYPE_BIT_CHAR:
    case M_TYPE_CHAR:
      M_char (ntype, type->unsign);
      return (M_TYPE_CHAR);
    case M_TYPE_SHORT:
      M_short (ntype, type->unsign);
      return (M_TYPE_SHORT);
    case M_TYPE_BLOCK:
    case M_TYPE_INT:
      /* the starting address of array is used */
      M_int (ntype, type->unsign);
      return (M_TYPE_INT);
    case M_TYPE_BIT_LONG:
    case M_TYPE_LONG:
      M_long (ntype, type->unsign);
      return (M_TYPE_LONG);
    case M_TYPE_POINTER:
      M_pointer (ntype);
      return (M_TYPE_POINTER);
      /* BCC - don't promote float to double now - 8/5/96 */
    case M_TYPE_FLOAT:
      M_float (ntype, type->unsign);
      return M_TYPE_FLOAT;
    case M_TYPE_DOUBLE:
      M_double (ntype, type->unsign);
      return M_TYPE_DOUBLE;
    case M_TYPE_UNION:
    case M_TYPE_STRUCT:
      *ntype = *type;
      return (type->type);
    default:
      return (-1);
    }
}

/*--------------------------------------------------------------------------*/
void
M_bx86_array_layout (M_Type type, int *offset)
{
  I_punt ("ml_bx86: Unimplemented: M_bx86_array_layout");
  *offset = 0;			/* assume first element is aligned */
}

int
M_bx86_array_align (M_Type type)
{
  I_punt ("ml_bx86: Unimplemented: M_bx86_array_align");
  return type->align;
}

int
M_bx86_array_size (M_Type type, int dim)
{
  int mod, size, align;

  I_punt ("ml_bx86: Unimplemented: M_bx86_array_size");
  size = type->size;
  align = type->align;
  mod = size % align;
  if (mod != 0)
    size += (align - mod);

  return (size * dim);
}

/*--------------------------------------------------------------------------*/
void
M_bx86_union_layout (int n, _M_Type * type, int *offset, int *bit_offset)
{
  int i;
  I_punt ("ml_bx86: Unimplemented: M_bx86_union_layout");
  for (i = 0; i < n; i++)
    {				/* assume the union is aligned */
      offset[i] = 0;
      bit_offset[i] = 0;
    }
}

int
M_bx86_union_align (int n, _M_Type * type)
{
  int i, max;
  I_punt ("ml_bx86: Unimplemented: M_bx86_union_align");
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
  if (max < M_BX86_ALIGN_CHAR)
    max = M_BX86_ALIGN_CHAR;

  return max;
}

int
M_bx86_union_size (int n, _M_Type * type)
{
  int i, aln, max_size, max_align;

  I_punt ("ml_bx86: Unimplemented: M_bx86_union_size");
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
  if (max_align < M_BX86_ALIGN_CHAR)
    max_align = M_BX86_ALIGN_CHAR;

  /* need to increment to the max. align for future array extension */
  i = max_size % max_align;
  if (i != 0)
    max_size += (max_align - i);

  return max_size;
}


/* 
 * M_x86_struct_layout
 *
 *    INPUTS: 	n	   - the number of elements in the structure
 *		type       - an array of _M_Type structures for each element
 *			     in the structure
 *
 *    OUTPUTS:	base	   - an array in which to store the offset into the
 *			     struct which should be addressed to load the 
 *			     element. 
 *		bit_offset - an array in which to store the bit offset of the
 *			     element from the base
 */
void
M_bx86_struct_layout (int n, _M_Type * type, int *base, int *bit_offset)
{
  int i;			/* index for arrays */
  int struct_off;		/* the current offset into the struct */
  int mod;			/* */
  int size;			/* the size of the current element */
  int align;			/* the aligment requirement of the 
				   current element */
  int word_off;			/* the offset into the current word */
  int bf_unit_size = 0;		/* the unit size of the bitfield 
				   (eg. char, short, long) */
  int bf_unit_off;		/* the offset into current bitfield 
				   unit size */

  I_punt ("ml_bx86: Unimplemented: M_bx86_struct_layout");

  struct_off = 0;		/* assume initially aligned */

  for (i = 0; i < n; i++)
    {				/* for each element in the struct */
      size = type[i].size;	/* size of element */
      align = type[i].align;	/* alignment requirements of element */
      M_assert ((size != 0) && (align != 0),
		"M_struct_layout: void is not allowed in structure");

      /*
       *  need to treat bit fields specially:
       *      - keep them in a word when possible
       *      - increase the unit size if necessary
       */
      if (type[i].type == M_TYPE_BIT_CHAR ||
	  type[i].type == M_TYPE_BIT_SHORT || type[i].type == M_TYPE_BIT_LONG)
	{
	  word_off = struct_off % M_BX86_SIZE_INT;

	  /* if element doesn't fit in word, 
	     then its offset is beginning of the next word */
	  if ((word_off + size) > M_BX86_SIZE_INT)
	    struct_off += (M_BX86_SIZE_INT - word_off);

	  /* if element doesn't fit in aligned unit size, then successively
	     increase unit size until it does */
	  if (type[i].type == M_TYPE_BIT_CHAR)
	    {
	      bf_unit_size = M_BX86_SIZE_CHAR;
	      bf_unit_off = struct_off % bf_unit_size;
#if 1
	      /* use for standard bitfields - JEM */
	      if ((bf_unit_off + size) > M_BX86_SIZE_CHAR)
		{
		  type[i].type = M_TYPE_BIT_SHORT;
		  bf_unit_size = M_BX86_SIZE_SHORT;
		}
#endif
	    }
	  if (type[i].type == M_TYPE_BIT_SHORT)
	    {
	      bf_unit_size = M_BX86_SIZE_SHORT;
	      bf_unit_off = struct_off % bf_unit_size;
#if 1
	      /* use for standard bitfields - JEM */
	      if ((bf_unit_off + size) > M_BX86_SIZE_SHORT)
		{
		  type[i].type = M_TYPE_BIT_LONG;
		  bf_unit_size = M_BX86_SIZE_LONG;
		}
#endif
	    }
	  if (type[i].type == M_TYPE_BIT_LONG)
	    {
	      bf_unit_size = M_BX86_SIZE_LONG;
	      bf_unit_off = struct_off % bf_unit_size;
	      if ((bf_unit_off + size) > M_BX86_SIZE_LONG)
		{
		  /* problem:  can't increase size any more */
		  I_punt
		    ("M_bx86_struct_layout:  "
		     "bitfield element does not fit in one word");
		}
	    }



#if 0
	  /* use for faster (non-standard) bitfields? - JEM */

	  /* adjust the element alignment to the unit size  */
	  /* x86 does not require this alignment, but it will eliminate
	     the need for shift instructions */
	  if (bf_unit_size > type[i].align)
	    {
	      type[i].align = bf_unit_size;
	      align = bf_unit_size;
	    }
#endif
	}			/* end special treatment of bitfields */



      /* adjust offset to required alignment */
      mod = struct_off % align;
      if (mod != 0)
	struct_off += (align - mod);


      /* calculate the base (the offset into the struct which should
         be addressed to load the element) */
      /* calculate the bit_offset (the bit offset from the base of the 
         element) */
      switch (type[i].type)
	{
	case M_TYPE_BIT_CHAR:
	case M_TYPE_BIT_SHORT:
	case M_TYPE_BIT_LONG:
	  base[i] = struct_off & (~(bf_unit_size - 1));	/*mask off low bits */
	  bit_offset[i] = struct_off - base[i];
#if 0
	  fprintf (stderr,
		   "bitfields:  type = %d, base = %d, bit_offset = %d\n",
		   type[i].type, base[i], bit_offset[i]);
#endif
	  break;
	default:
	  bit_offset[i] = 0;
	  base[i] = struct_off;
#if 0
	  fprintf (stderr, "type = %d, base = %d, bit_offset = %d\n",
		   type[i].type, base[i], bit_offset[i]);
#endif
	}


      /* adjust offset to allocate space for current element */
      struct_off += size;
    }				/* end for loop */
}


int
M_bx86_struct_align (int n, _M_Type * type)
{
  int i, max;
  I_punt ("ml_bx86: Unimplemented: M_bx86_struct_align");
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
  if (max < M_BX86_ALIGN_CHAR)
    max = M_BX86_ALIGN_CHAR;
  return max;
}


int
M_bx86_struct_size (int n, _M_Type * type, int struct_align)
{
  int i, struct_off;
  int mod, size, align, max_align, word_off;

  I_punt ("ml_bx86: Unimplemented: M_bx86_struct_size");

  struct_off = 0;		/* assume initially aligned */
  max_align = struct_align;

  for (i = 0; i < n; i++)
    {				/* loop through each struct element */
      size = type[i].size;
      align = type[i].align;
      M_assert ((size != 0) && (align != 0),
		"M_struct_layout: void is not allowed in structure");

      /* keep track of max alignment */
      if (align > max_align)
	max_align = align;

      /*
       *  need to treat bit fields specially:
       *      - keep them in a word when possible
       */
      if (type[i].type == M_TYPE_BIT_CHAR ||
	  type[i].type == M_TYPE_BIT_SHORT || type[i].type == M_TYPE_BIT_LONG)
	{
	  word_off = struct_off % M_BX86_SIZE_INT;

	  if ((word_off + size) > M_BX86_SIZE_INT)
	    {
	      /* if element doesn't fit in word, 
	         then its offset is beginning of the next word */
	      struct_off += (M_BX86_SIZE_INT - word_off);
	    }

	}


      /* adjust offset to required alignment */
      mod = struct_off % align;
      if (mod != 0)
	struct_off += (align - mod);

      struct_off += size;
    }


  /* account for alignment of entire struct */
  if (max_align < M_BX86_ALIGN_CHAR)
    max_align = M_BX86_ALIGN_CHAR;	/* align to at least byte boundary */
  mod = struct_off % max_align;	/* enforce max. alignment */
  if (mod != 0)
    struct_off += (max_align - mod);


  return struct_off;
}

/*--------------------------------------------------------------------------*/
int
M_bx86_fnvar_layout (int n, _M_Type * type, long int *offset, int *mode,
		     int *reg, int *paddr, char **macro, int *su_sreg,
		     int *su_ereg, int *pcount, int is_st, int purpose)
					/* need to return structure */
{

  I_punt ("ml_bx86: Unimplemented: M_bx86_fnvar_layout ");
  return (-1);   /* I_punt doesn't return */
#if 0

  int i, max_align, off = 0, rg;
  int size, align, mod, tp;


  switch (purpose)
    {
    case M_GET_FNVAR:
      *macro = "$IP";
      /* return address is being pointed to be stack pointer */
      /* thus, offset to first parameter is 4 from stack pointer */
      off = 4 * 8;
      break;
    case M_PUT_FNVAR:
      *macro = "$OP";
      /* return address is being pointed to be stack pointer */
      /* thus, offset to first parameter is 4 from stack pointer */
      off = 0 * 8;
      break;
    case M_DONT_CARE_FNVAR:
    default:
      M_assert (0, "M_fnvar_layout: unknown purpose");
    }

  max_align = M_BX86_ALIGN_MAX;
  rg = 0;



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
	  if (rg < M_BX86_MAX_FNVAR_REG)
	    {
	      mode[i] = M_THRU_RBEGISTER;
	      reg[i] = (rg)++ + M_BX86_INT_BASE;
	    }
	  else
	    {
	      mode[i] = M_THRU_MEMORY;
	      reg[i] = -1;
	    }
	  break;

	case M_TYPE_FLOAT:
	  if (rg < M_BX86_MAX_FNVAR_REG)
	    {
	      mode[i] = M_THRU_REGISTER;
	      reg[i] = (rg)++ + M_BX86_FLT_BASE;
	    }
	  else
	    {
	      mode[i] = M_THRU_MEMORY;
	      reg[i] = -1;
	    }
	  break;

	case M_TYPE_DOUBLE:
	  if (rg < M_BX86_MAX_FNVAR_REG)
	    {
	      if (rg == 0 || rg == 2)
		{
		  mode[i] = M_THRU_REGISTER;
		  reg[i] = rg + 1 + M_BX86_FLT_BASE;
		  rg += 2;
		}
	      else if (rg == 1)
		{
		  mode[i] = M_THRU_REGISTER;
		  reg[i] = 3 + M_BX86_FLT_BASE;
		  rg += 3;
		}
	      else
		{
		  mode[i] = M_THRU_MEMORY;
		  reg[i] = -1;
		}
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
	  mode[i] = M_THRU_MEMORY;
	  reg[i] = -1;
	  break;

	default:
	  M_assert (0, "M_fnvar_layout: argument is not promoted");
	}

      /* The BX86 convention is for the FP to contain a pointer to the 
         previous stack frame.  Because the stack grows down, the first
         function variable is available at FP -4.  The SP is at the bottom
         of the stack, and is used for push/pop beyond the local variable
         area. */

      size = type[i].size;
      align = type[i].align;

      if (type[i].type == M_TYPE_UNION || type[i].type == M_TYPE_STRUCT)
	{
	  /* make sure correct size and alignment is used for struct/union */
	  /* passed indirectly thru registers                              */
	  /* if IMPACT allowed structures to be passed via registers, this */
	  /* would not have to be done                                     */
	  if (mode[i] == M_INDIRECT_THRU_REGISTER)
	    {
	      size = M_BX86_SIZE_INT;
	      align = M_BX86_ALIGN_INT;
	    }
	}
      if (align >= M_BX86_SMALL_STRUCT_MAX && type[i].type != M_TYPE_DOUBLE)
	/* anything larger than a 64-bit structure is passed */
	/* indirectly thru memory                            */
	align = M_BX86_ALIGN_INT;
      else if (align < M_BX86_ALIGN_INT)
	/* anything smaller that 32-bits is passed as 32-bits */
	align = M_BX86_ALIGN_INT;

      mod = off % align;

      /* place the offset pointer to the boundary of the appropriate */
      /* data size                                                   */
      if (mod != 0)
	off += (align - mod);

      /* now increment the offset to point to the actual location    */
      /* for this parameter.  */


      offset[i] = off;		/* offset is positive for X86 */
      /* note offset for first parameter is set */
      /* before adding its size                */
      off += size;
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
	   (tp == M_TYPE_STRUCT)) && (size <= M_BX86_SMALL_STRUCT_MAX))
	{

	  align = M_BX86_ALIGN_MAX;	/* must align to a double boundry */

	  mod = off % align;
/*
            if ( mod!=0 )
	        off += (align - mod);

            off += size;
*/
	  paddr[i] = -off;
	}
    }

  /* now large ones */
  for (i = 0; i < n; i++)
    {
      tp = type[i].type;

      size = type[i].size;

      if (((tp == M_TYPE_UNION) ||
	   (tp == M_TYPE_STRUCT)) && (size > M_BX86_SMALL_STRUCT_MAX))
	{

	  align = M_BX86_ALIGN_MAX;	/* must align to a double boundry */

	  mod = off % align;
/*
            if ( mod!=0 ) 
	        off += (align - mod);

	    off += size;
*/

	  paddr[i] = -off;
	}
    }


  /*  Because the X86 reverses operands, pushing them right to left,
     we need to reverse the order of operands on the stack         */
  have_reversed = 0;
/*
    if (! have_reversed)  {

        for (i=0;i<n;i++)  {
	    rev_mode[i] = mode[i];
	    rev_reg[i] = reg [i];
	    rev_offset[i] = offset[i];
	    rev_paddr[i] = paddr[i];
	    rev_type[i] = type[i];   
        }

        for (i=0;i<n;i++)  {
	    mode[n-1-i] = rev_mode[i];
	    reg[n-1-i] = rev_reg [i];
	    offset[n-1-i] = rev_offset[i];
	    paddr[n-1-i] = rev_paddr[i];
	    type[n-1-i] = rev_type[i];   
        }

        have_reversed = 1;
    }
*/

  return off;			/* the total size needed */

#endif

}

/*--------------------------------------------------------------------------*/
int
M_bx86_lvar_layout (int n, _M_Type * type, long int *offset,
		    char **base_macro)
{
  int i, max_align, off;
  int size, align, mod, tp;
  /*
   *  the LOCAL section must be max. aligned initially
   */
  max_align = M_BX86_ALIGN_MAX;
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
  *base_macro = M_BX86_VAR_PTR;
  return off;			/* the total size needed */
}

/*--------------------------------------------------------------------------*/
int
M_bx86_no_short_int (void)
{
  return (M_BX86_SIZE_SHORT == M_BX86_SIZE_INT);
}
/*--------------------------------------------------------------------------*/
void
M_bx86_cb_label_name (char *fn, int cb, char *line, int len)
{
  sprintf (line, "cb%d%s", cb, fn);
}
/*--------------------------------------------------------------------------*/
int
M_bx86_is_cb_label (char *label, char *fn, int *cb)
{
  return (sscanf (label, "cb%d%s", cb, fn) == 2);
}
/*--------------------------------------------------------------------------*/
void
M_bx86_jumptbl_label_name (char *fn, int tbl_id, char *line, int len)
{
  sprintf (line, "%s%s%d", fn, M_JUMPTBL_BASE_NAME, tbl_id);
}
/*--------------------------------------------------------------------------*/
/* Format for x86 is: %sM_JUMPTBL_BASE_NAME%d, where %s is the func name */
int
M_bx86_is_jumptbl_label (char *label, char *fn, int *tbl_id)
{
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
/*--------------------------------------------------------------------------*/
int
M_bx86_structure_pointer (int purpose)
{
  return M_BX86_RET_ST;
}
/*--------------------------------------------------------------------------*/
int
M_bx86_return_register (int type, int purpose)
{
  switch (type)
    {
    case M_TYPE_INT:
      return M_BX86_RET_I32;
    case M_TYPE_LONG:
      return M_BX86_RET_I32;
    case M_TYPE_FLOAT:
      return M_BX86_RET_F;
    case M_TYPE_DOUBLE:
      return M_BX86_RET_F;
    default:
      return M_BX86_RET_I32;
    }
}
/*--------------------------------------------------------------------------*/
/*
char *M_bx86_fn_label_name(label)
*/
char *
M_bx86_fn_label_name (char *label, int (*is_func) (char *is_func_label))
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


char *
M_bx86_fn_name_from_label (char *label)
{
  if (!strncmp (label, "_$fn", 4))
    return (label + 4);
  else
    return (label);
}



/*--------------------------------------------------------------------------*/
int
M_bx86_fragile_macro (int macro_value)
{
  switch (M_model)
    {
    case M_B_486:
    case M_B_PENTIUM:
    case M_B_PPRO:
    case M_B_PENTIUM_MMX:
    case M_B_PENTIUM_II:
    case M_B_K5:
    case M_B_K6:
    case M_B_K6_MMX:
    case M_B_K6_PLUS:

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
    default:
      M_assert (0, "M_bx86_fragile_macro:  Illegal model specified!");
      return (0);
    }
}
/*--------------------------------------------------------------------------*/
int
M_bx86_subroutine_call (int opc)
{
  switch (M_model)
    {
    case M_B_486:
    case M_B_PENTIUM:
    case M_B_PPRO:
    case M_B_PENTIUM_MMX:
    case M_B_PENTIUM_II:
    case M_B_K5:
    case M_B_K6:
    case M_B_K6_MMX:
    case M_B_K6_PLUS:
      return ((opc == Lop_JSR) || (opc == Lop_JSR_FS));
    default:
      M_assert (0, "M_bx86_subroutine_call:  Illegal model specified!");
      return (0);
    }
}
/*--------------------------------------------------------------------------*/
/*
 * Declare code generator specific macro registers to the front end parser.
 */
void
M_define_macros_bx86 (STRING_Symbol_Table * sym_tbl)
{
  /* number of integer and float callee saved registers used */
  M_add_symbol (sym_tbl, "$callee_i_regs", LBX86_MAC_CALLEE_I);
  M_add_symbol (sym_tbl, "$callee_f_regs", LBX86_MAC_CALLEE_F);

  M_add_symbol (sym_tbl, "$oszapc_flag", LBX86_MAC_OSZAPC_FLAGS);
  M_add_symbol (sym_tbl, "$c_flag", LBX86_MAC_C_FLAGS);
  M_add_symbol (sym_tbl, "$oszap_flag", LBX86_MAC_OSZAP_FLAGS);
  M_add_symbol (sym_tbl, "$d_flag", LBX86_MAC_D_FLAGS);
  M_add_symbol (sym_tbl, "$oc_flag", LBX86_MAC_OC_FLAGS);
  M_add_symbol (sym_tbl, "$szapc_flag", LBX86_MAC_SZAPC_FLAGS);
  M_add_symbol (sym_tbl, "$oszpc_flag", LBX86_MAC_OSZPC_FLAGS);
  M_add_symbol (sym_tbl, "$z_flag", LBX86_MAC_Z_FLAGS);
  M_add_symbol (sym_tbl, "$zc_flag", LBX86_MAC_ZC_FLAGS);
  M_add_symbol (sym_tbl, "$osz_flag", LBX86_MAC_OSZ_FLAGS);
  M_add_symbol (sym_tbl, "$os_flag", LBX86_MAC_OS_FLAGS);
  M_add_symbol (sym_tbl, "$o_flag", LBX86_MAC_O_FLAGS);
  M_add_symbol (sym_tbl, "$s_flag", LBX86_MAC_S_FLAGS);
  M_add_symbol (sym_tbl, "$a_flag", LBX86_MAC_A_FLAGS);
  M_add_symbol (sym_tbl, "$p_flag", LBX86_MAC_P_FLAGS);
  M_add_symbol (sym_tbl, "$all_gp_32", LBX86_MAC_ALL_GP_REG32);
  M_add_symbol (sym_tbl, "$all_gp_16", LBX86_MAC_ALL_GP_REG16);
  M_add_symbol (sym_tbl, "$eax", LBX86_MAC_EAX);
  M_add_symbol (sym_tbl, "$ax", LBX86_MAC_AX);
  M_add_symbol (sym_tbl, "$ah", LBX86_MAC_AH);
  M_add_symbol (sym_tbl, "$al", LBX86_MAC_AL);
  M_add_symbol (sym_tbl, "$eaxh", LBX86_MAC_EAXH);
  M_add_symbol (sym_tbl, "$ebx", LBX86_MAC_EBX);
  M_add_symbol (sym_tbl, "$bx", LBX86_MAC_BX);
  M_add_symbol (sym_tbl, "$bh", LBX86_MAC_BH);
  M_add_symbol (sym_tbl, "$bl", LBX86_MAC_BL);
  M_add_symbol (sym_tbl, "$ebxh", LBX86_MAC_EBXH);
  M_add_symbol (sym_tbl, "$ecx", LBX86_MAC_ECX);
  M_add_symbol (sym_tbl, "$cx", LBX86_MAC_CX);
  M_add_symbol (sym_tbl, "$ch", LBX86_MAC_CH);
  M_add_symbol (sym_tbl, "$cl", LBX86_MAC_CL);
  M_add_symbol (sym_tbl, "$ecxh", LBX86_MAC_ECXH);
  M_add_symbol (sym_tbl, "$edx", LBX86_MAC_EDX);
  M_add_symbol (sym_tbl, "$dx", LBX86_MAC_DX);
  M_add_symbol (sym_tbl, "$dh", LBX86_MAC_DH);
  M_add_symbol (sym_tbl, "$dl", LBX86_MAC_DL);
  M_add_symbol (sym_tbl, "$edxh", LBX86_MAC_EDXH);
  M_add_symbol (sym_tbl, "$esi", LBX86_MAC_ESI);
  M_add_symbol (sym_tbl, "$si", LBX86_MAC_SI);
  M_add_symbol (sym_tbl, "$esih", LBX86_MAC_ESIH);
  M_add_symbol (sym_tbl, "$edi", LBX86_MAC_EDI);
  M_add_symbol (sym_tbl, "$di", LBX86_MAC_DI);
  M_add_symbol (sym_tbl, "$edih", LBX86_MAC_EDIH);
  M_add_symbol (sym_tbl, "$ebp", LBX86_MAC_EBP);
  M_add_symbol (sym_tbl, "$bp", LBX86_MAC_BP);
  M_add_symbol (sym_tbl, "$ebph", LBX86_MAC_EBPH);
  M_add_symbol (sym_tbl, "$esp", LBX86_MAC_ESP);
  M_add_symbol (sym_tbl, "$sp", LBX86_MAC_SP);
  M_add_symbol (sym_tbl, "$esph", LBX86_MAC_ESPH);
  M_add_symbol (sym_tbl, "$cs", LBX86_MAC_CS);
  M_add_symbol (sym_tbl, "$ss", LBX86_MAC_SS);
  M_add_symbol (sym_tbl, "$ds", LBX86_MAC_DS);
  M_add_symbol (sym_tbl, "$es", LBX86_MAC_ES);
  M_add_symbol (sym_tbl, "$fs", LBX86_MAC_FS);
  M_add_symbol (sym_tbl, "$gs", LBX86_MAC_GS);
  M_add_symbol (sym_tbl, "$all_fst", LBX86_MAC_ALL_FST);
  M_add_symbol (sym_tbl, "$st0", LBX86_MAC_ST0);
  M_add_symbol (sym_tbl, "$st1", LBX86_MAC_ST1);
  M_add_symbol (sym_tbl, "$st2", LBX86_MAC_ST2);
  M_add_symbol (sym_tbl, "$st3", LBX86_MAC_ST3);
  M_add_symbol (sym_tbl, "$st4", LBX86_MAC_ST4);
  M_add_symbol (sym_tbl, "$st5", LBX86_MAC_ST5);
  M_add_symbol (sym_tbl, "$st6", LBX86_MAC_ST6);
  M_add_symbol (sym_tbl, "$st7", LBX86_MAC_ST7);
  M_add_symbol (sym_tbl, "$all_mm", LBX86_MAC_ALL_MM);
  M_add_symbol (sym_tbl, "$mm0", LBX86_MAC_MM0);
  M_add_symbol (sym_tbl, "$mm1", LBX86_MAC_MM1);
  M_add_symbol (sym_tbl, "$mm2", LBX86_MAC_MM2);
  M_add_symbol (sym_tbl, "$mm3", LBX86_MAC_MM3);
  M_add_symbol (sym_tbl, "$mm4", LBX86_MAC_MM4);
  M_add_symbol (sym_tbl, "$mm5", LBX86_MAC_MM5);
  M_add_symbol (sym_tbl, "$mm6", LBX86_MAC_MM6);
  M_add_symbol (sym_tbl, "$mm7", LBX86_MAC_MM7);
  M_add_symbol (sym_tbl, "$addr", LBX86_MAC_ADDR);
  M_add_symbol (sym_tbl, "$fpsw", LBX86_MAC_FPSW);
  M_add_symbol (sym_tbl, "$fpcw", LBX86_MAC_FPCW);
}

char *
M_get_macro_name_bx86 (int id)
{
  switch (id)
    {
    case LBX86_MAC_CALLEE_I:
      return "$callee_i_regs";
    case LBX86_MAC_CALLEE_F:
      return "$callee_f_regs";

    case LBX86_MAC_OSZAPC_FLAGS:
      return "$oszapc_flag";
    case LBX86_MAC_C_FLAGS:
      return "$c_flag";
    case LBX86_MAC_OSZAP_FLAGS:
      return "$oszap_flag";
    case LBX86_MAC_D_FLAGS:
      return "$d_flag";
    case LBX86_MAC_OC_FLAGS:
      return "$oc_flag";
    case LBX86_MAC_SZAPC_FLAGS:
      return "$szapc_flag";
    case LBX86_MAC_OSZPC_FLAGS:
      return "$oszpc_flag";
    case LBX86_MAC_Z_FLAGS:
      return "$z_flag";
    case LBX86_MAC_ZC_FLAGS:
      return "$zc_flag";
    case LBX86_MAC_OSZ_FLAGS:
      return "$osz_flag";
    case LBX86_MAC_OS_FLAGS:
      return "$os_flag";
    case LBX86_MAC_O_FLAGS:
      return "$o_flag";
    case LBX86_MAC_S_FLAGS:
      return "$s_flag";
    case LBX86_MAC_A_FLAGS:
      return "$a_flag";
    case LBX86_MAC_P_FLAGS:
      return "$p_flag";
    case LBX86_MAC_ALL_GP_REG32:
      return "$all_gp_32";
    case LBX86_MAC_ALL_GP_REG16:
      return "$all_gp_16";
    case LBX86_MAC_EAX:
      return "$eax";
    case LBX86_MAC_AX:
      return "$ax";
    case LBX86_MAC_AH:
      return "$ah";
    case LBX86_MAC_AL:
      return "$al";
    case LBX86_MAC_EAXH:
      return "$eaxh";
    case LBX86_MAC_EBX:
      return "$ebx";
    case LBX86_MAC_BX:
      return "$bx";
    case LBX86_MAC_BH:
      return "$bh";
    case LBX86_MAC_BL:
      return "$bl";
    case LBX86_MAC_EBXH:
      return "$ebxh";
    case LBX86_MAC_ECX:
      return "$ecx";
    case LBX86_MAC_CX:
      return "$cx";
    case LBX86_MAC_CH:
      return "$ch";
    case LBX86_MAC_CL:
      return "$cl";
    case LBX86_MAC_ECXH:
      return "$ecxh";
    case LBX86_MAC_EDX:
      return "$edx";
    case LBX86_MAC_DX:
      return "$dx";
    case LBX86_MAC_DH:
      return "$dh";
    case LBX86_MAC_DL:
      return "$dl";
    case LBX86_MAC_EDXH:
      return "$edxh";
    case LBX86_MAC_ESI:
      return "$esi";
    case LBX86_MAC_SI:
      return "$si";
    case LBX86_MAC_ESIH:
      return "$esih";
    case LBX86_MAC_EDI:
      return "$edi";
    case LBX86_MAC_DI:
      return "$di";
    case LBX86_MAC_EDIH:
      return "$edih";
    case LBX86_MAC_EBP:
      return "$ebp";
    case LBX86_MAC_BP:
      return "$bp";
    case LBX86_MAC_EBPH:
      return "$ebph";
    case LBX86_MAC_ESP:
      return "$esp";
    case LBX86_MAC_SP:
      return "$sp";
    case LBX86_MAC_ESPH:
      return "$esph";
    case LBX86_MAC_CS:
      return "$cs";
    case LBX86_MAC_SS:
      return "$ss";
    case LBX86_MAC_DS:
      return "$ds";
    case LBX86_MAC_ES:
      return "$es";
    case LBX86_MAC_FS:
      return "$fs";
    case LBX86_MAC_GS:
      return "$gs";
    case LBX86_MAC_ALL_FST:
      return "$all_fst";
    case LBX86_MAC_ST0:
      return "$st0";
    case LBX86_MAC_ST1:
      return "$st1";
    case LBX86_MAC_ST2:
      return "$st2";
    case LBX86_MAC_ST3:
      return "$st3";
    case LBX86_MAC_ST4:
      return "$st4";
    case LBX86_MAC_ST5:
      return "$st5";
    case LBX86_MAC_ST6:
      return "$st6";
    case LBX86_MAC_ST7:
      return "$st7";
    case LBX86_MAC_ALL_MM:
      return "$all_mm";
    case LBX86_MAC_MM0:
      return "$mm0";
    case LBX86_MAC_MM1:
      return "$mm1";
    case LBX86_MAC_MM2:
      return "$mm2";
    case LBX86_MAC_MM3:
      return "$mm3";
    case LBX86_MAC_MM4:
      return "$mm4";
    case LBX86_MAC_MM5:
      return "$mm5";
    case LBX86_MAC_MM6:
      return "$mm6";
    case LBX86_MAC_MM7:
      return "$mm7";
    case LBX86_MAC_ADDR:
      return "$addr";
    case LBX86_MAC_FPSW:
      return "$fpsw";
    case LBX86_MAC_FPCW:
      return "$fpcw";
    default:
      return "?";
    }
}

/* Added to support Lmix tools -JCG 11/21/95 */
void
M_define_opcode_name_bx86 (STRING_Symbol_Table * sym_tbl)
{

}

char *
M_get_opcode_name_bx86 (int id)
{
  switch (id)
    {
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
M_oper_supported_in_arch_bx86 (int opc)
{
  switch (opc)
    {

    case Lop_NOR:
    case Lop_NAND:
    case Lop_NXOR:
    case Lop_OR_NOT:
    case Lop_AND_NOT:

    case Lop_MUL_ADD_U:
    case Lop_MUL_ADD_F:
    case Lop_MUL_ADD_F2:
    case Lop_MUL_SUB:
    case Lop_MUL_SUB_U:
    case Lop_MUL_SUB_REV:
    case Lop_MUL_SUB_REV_U:
    case Lop_MUL_SUB_F:
    case Lop_MUL_SUB_REV_F:
    case Lop_MUL_SUB_F2:
    case Lop_MUL_SUB_REV_F2:
      return (0);

    default:
      return (1);
    }
}

/*
 * Returns the number of machine instructions required to implement the
 * specified oper in the best case.  It is assumed that this is for
 * supported instructions.  A call to M_oper_supported_in_arch should be
 * made for abnormal instructions.
 */
int
M_num_oper_required_for_bx86 (L_Oper * oper, char *name)
{

#define	has_label_operand(a)	((a->src[0]->type == L_OPERAND_LABEL)||\
    				 (a->src[1]->type == L_OPERAND_LABEL))
#define indexed_memory_op(a)	((a->src[0]->type == L_OPERAND_REGISTER)&&\
    				 (a->src[1]->type == L_OPERAND_REGISTER))

#define non_zero_offset(a)	(!((a->src[1]->type==L_OPERAND_IMMED)&&\
                                  ((a->src[1]->ctype&0x30)==0x00)) ||\
    				 (a->src[1]->value.i != 0))

#define short_int_inc(a,b)	(((a->src[1]->type==L_OPERAND_IMMED)&&\
                                  ((a->src[1]->ctype&0x30)==0x00)) &&\
    				 (a->src[b]->value.i >= -0x10) &&\
    				 (a->src[b]->value.i < 0x10))
#define long_pos_int_inc(a,b)	(((a->src[1]->type==L_OPERAND_IMMED)&&\
                                  ((a->src[1]->ctype&0x30)==0x00)) &&\
    				 (a->src[b]->value.i > 0) && \
    				 (a->src[b]->value.i < 0x2000))
#define long_neg_int_inc(a,b)	(((a->src[1]->type==L_OPERAND_IMMED)&&\
                                  ((a->src[1]->ctype&0x30)==0x00)) &&\
    				 (a->src[b]->value.i >= -0x2000) && \
    				 (a->src[b]->value.i < 0))
#define register_inc(a,b)	 (a->src[b]->type == L_OPERAND_REGISTER)

  switch (oper->opc)
    {
    case Lop_ST_C:
    case Lop_ST_C2:
    case Lop_ST_I:
    case Lop_LD_UC:
    case Lop_LD_C:
    case Lop_LD_UC2:
    case Lop_LD_C2:
    case Lop_LD_I:

    case Lop_ST_F:
    case Lop_ST_F2:

    case Lop_LD_F:
    case Lop_LD_F2:
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

    default:
      return (1);
    }
}

int
M_is_stack_operand_bx86 (L_Operand * operand)
{
  if (L_is_macro (operand) &&
      (operand->value.mac == LBX86_MAC_ESP ||
       operand->value.mac == L_MAC_IP ||
       operand->value.mac == L_MAC_OP || operand->value.mac == L_MAC_LV))
    return (1);

  return (0);
}

int
M_is_unsafe_macro_bx86 (L_Operand * operand)
{
  if ((L_is_macro (operand)) && (operand->value.mac == L_MAC_P1))
    return (1);

  if ((L_is_macro (operand)) && (operand->value.mac == L_MAC_P0))
    return (1);

  return (0);
}


int
M_operand_type_bx86 (L_Operand * operand)
{


/* fprintf(stderr,"$here\n");*/
  /* If NULL operand pointer, then return MDES_OPERAND_NULL */
  if (operand == NULL)
    return (MDES_OPERAND_NULL);

  switch (L_operand_case_type (operand))
    {
    case L_OPERAND_INT:
/* fprintf(stderr,"$lit\n"); */
      return (MDES_OPERAND_Lit);

    case L_OPERAND_MACRO:
    case L_OPERAND_REGISTER:
      return (MDES_OPERAND_REG);


    case L_OPERAND_CB:
    case L_OPERAND_LABEL:
    case L_OPERAND_FLOAT:
    case L_OPERAND_DOUBLE:
    case L_OPERAND_STRING:
      return (MDES_OPERAND_Label);

    default:
      M_assert (0, "M_operand_type_bx86: Unknown type");
    }

  return (0);
}


/*
               EFLAGS
     ------------------------
1)   (OF  SF  ZF  AF  PF  CF)
2)                        CF       (clc dest)
3)    OF  SF  ZF  AF  PF           (dec dest)
4)    OF                  CF       (ror dest)
5)        SF  ZF  AF  PF  CF       (sahf dest)
6)    OF  SF  ZF      PF  CF       (shl dest)
7)            ZF                   (loopz src) 
8)            ZF          CF       (seta src) 
9)    OF  SF  ZF                   (setg src)
10)   OF  SF                       (setge src)
11)   OF                           (for components)
12)       SF                       (for components)
13)               AF               (for components)
14)                   PF           (for components)

15)                           DF   (cld dest)

Conflicting types:
1->  1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14
2->  1, 2,    4, 5, 6,    8
3->  1,    3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14
4->  1, 2, 3, 4, 5, 6,    8, 9, 10, 11
5->  1, 2, 3, 4, 5, 6, 7, 8, 9, 10,     12, 13, 14
6->  1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12,     14
7->  1,    3,    5, 6, 7, 8, 9
8->  1, 2, 3, 4, 5, 6, 7, 8, 9
9->  1,    3, 4, 5, 6, 7, 8, 9, 10, 11, 12
10-> 1,    3, 4, 5, 6,       9, 10, 11, 12
11-> 1,    3, 4,    6,       9, 10, 11
12-> 1,    3,    5, 6,       9, 10,     12
13-> 1,    3,    5,                         13
14-> 1,    3,    5, 6,                          14

Note that case 1 conflicts with all others (except 15, only w/ itself)
*/

int
M_conflicting_operands_bx86 (L_Operand * operand, L_Operand ** conflict_array,
			     int len, int prepass)
{
  int macval;

  /* Define flags */
#define MSPEC_oszapc_flags LBX86_MAC_OSZAPC_FLAGS,L_CTYPE_VOID,L_PTYPE_NULL
#define MSPEC_c_flags LBX86_MAC_C_FLAGS,L_CTYPE_VOID,L_PTYPE_NULL
#define MSPEC_oszap_flags LBX86_MAC_OSZAP_FLAGS,L_CTYPE_VOID,L_PTYPE_NULL
#define MSPEC_oc_flags LBX86_MAC_OC_FLAGS,L_CTYPE_VOID,L_PTYPE_NULL
#define MSPEC_szapc_flags LBX86_MAC_SZAPC_FLAGS,L_CTYPE_VOID, L_PTYPE_NULL
#define MSPEC_oszpc_flags LBX86_MAC_OSZPC_FLAGS,L_CTYPE_VOID, L_PTYPE_NULL
#define MSPEC_z_flags LBX86_MAC_Z_FLAGS,L_CTYPE_VOID,L_PTYPE_NULL
#define MSPEC_zc_flags LBX86_MAC_ZC_FLAGS,L_CTYPE_VOID,L_PTYPE_NULL
#define MSPEC_osz_flags LBX86_MAC_OSZ_FLAGS,L_CTYPE_VOID, L_PTYPE_NULL
#define MSPEC_os_flags LBX86_MAC_OS_FLAGS,L_CTYPE_VOID,L_PTYPE_NULL
#define MSPEC_d_flags LBX86_MAC_D_FLAGS,L_CTYPE_VOID,L_PTYPE_NULL
#define MSPEC_o_flags LBX86_MAC_O_FLAGS,L_CTYPE_VOID,L_PTYPE_NULL
#define MSPEC_s_flags LBX86_MAC_S_FLAGS,L_CTYPE_VOID,L_PTYPE_NULL
#define MSPEC_a_flags LBX86_MAC_A_FLAGS,L_CTYPE_VOID,L_PTYPE_NULL
#define MSPEC_p_flags LBX86_MAC_P_FLAGS,L_CTYPE_VOID,L_PTYPE_NULL

  /* Define all gp registers */
#define MSPEC_gp_reg32 LBX86_MAC_ALL_GP_REG32,L_CTYPE_INT,L_PTYPE_NULL
#define MSPEC_gp_reg16 LBX86_MAC_ALL_GP_REG16,L_CTYPE_INT,L_PTYPE_NULL

  /* Define 32 bit gp registers */
#define MSPEC_eax LBX86_MAC_EAX,L_CTYPE_INT,L_PTYPE_NULL
#define MSPEC_ebx LBX86_MAC_EBX,L_CTYPE_INT,L_PTYPE_NULL
#define MSPEC_ecx LBX86_MAC_ECX,L_CTYPE_INT,L_PTYPE_NULL
#define MSPEC_edx LBX86_MAC_EDX,L_CTYPE_INT,L_PTYPE_NULL
#define MSPEC_esi LBX86_MAC_ESI,L_CTYPE_INT,L_PTYPE_NULL
#define MSPEC_edi LBX86_MAC_EDI,L_CTYPE_INT,L_PTYPE_NULL
#define MSPEC_ebp LBX86_MAC_EBP,L_CTYPE_INT,L_PTYPE_NULL
#define MSPEC_esp LBX86_MAC_ESP,L_CTYPE_INT,L_PTYPE_NULL

  /* Define 16 bit gp registers */
#define MSPEC_ax LBX86_MAC_AX,L_CTYPE_SHORT,L_PTYPE_NULL
#define MSPEC_bx LBX86_MAC_BX,L_CTYPE_SHORT,L_PTYPE_NULL
#define MSPEC_cx LBX86_MAC_CX,L_CTYPE_SHORT,L_PTYPE_NULL
#define MSPEC_dx LBX86_MAC_DX,L_CTYPE_SHORT,L_PTYPE_NULL
#define MSPEC_si LBX86_MAC_SI,L_CTYPE_SHORT,L_PTYPE_NULL
#define MSPEC_di LBX86_MAC_DI,L_CTYPE_SHORT,L_PTYPE_NULL
#define MSPEC_bp LBX86_MAC_BP,L_CTYPE_SHORT,L_PTYPE_NULL
#define MSPEC_sp LBX86_MAC_SP,L_CTYPE_SHORT,L_PTYPE_NULL
  /*  ... pseudo-registers */
#define MSPEC_eaxh LBX86_MAC_EAXH,L_CTYPE_SHORT,L_PTYPE_NULL
#define MSPEC_ebxh LBX86_MAC_EBXH,L_CTYPE_SHORT,L_PTYPE_NULL
#define MSPEC_ecxh LBX86_MAC_ECXH,L_CTYPE_SHORT,L_PTYPE_NULL
#define MSPEC_edxh LBX86_MAC_EDXH,L_CTYPE_SHORT,L_PTYPE_NULL
#define MSPEC_esih LBX86_MAC_ESIH,L_CTYPE_SHORT,L_PTYPE_NULL
#define MSPEC_edih LBX86_MAC_EDIH,L_CTYPE_SHORT,L_PTYPE_NULL
#define MSPEC_ebph LBX86_MAC_EBPH,L_CTYPE_SHORT,L_PTYPE_NULL
#define MSPEC_esph LBX86_MAC_ESPH,L_CTYPE_SHORT,L_PTYPE_NULL

  /* Define lower 8 bit gp registers */
#define MSPEC_al LBX86_MAC_AL,L_CTYPE_CHAR,L_PTYPE_NULL
#define MSPEC_bl LBX86_MAC_BL,L_CTYPE_CHAR,L_PTYPE_NULL
#define MSPEC_cl LBX86_MAC_CL,L_CTYPE_CHAR,L_PTYPE_NULL
#define MSPEC_dl LBX86_MAC_DL,L_CTYPE_CHAR,L_PTYPE_NULL

  /* Define upper 8 bit gp registers */
#define MSPEC_ah LBX86_MAC_AH,L_CTYPE_CHAR,L_PTYPE_NULL
#define MSPEC_bh LBX86_MAC_BH,L_CTYPE_CHAR,L_PTYPE_NULL
#define MSPEC_ch LBX86_MAC_CH,L_CTYPE_CHAR,L_PTYPE_NULL
#define MSPEC_dh LBX86_MAC_DH,L_CTYPE_CHAR,L_PTYPE_NULL

  /* Define 16 bit segment registers */
#define MSPEC_cs LBX86_MAC_CS,L_CTYPE_SHORT,L_PTYPE_NULL
#define MSPEC_ds LBX86_MAC_DS,L_CTYPE_SHORT,L_PTYPE_NULL
#define MSPEC_es LBX86_MAC_ES,L_CTYPE_SHORT,L_PTYPE_NULL
#define MSPEC_fs LBX86_MAC_FS,L_CTYPE_SHORT,L_PTYPE_NULL
#define MSPEC_gs LBX86_MAC_GS,L_CTYPE_SHORT,L_PTYPE_NULL
#define MSPEC_ss LBX86_MAC_SS,L_CTYPE_SHORT,L_PTYPE_NULL

  /* Define float and MMX registers */
#define MSPEC_all_fst LBX86_MAC_ALL_FST,L_CTYPE_FLOAT,L_PTYPE_NULL
#define MSPEC_st0 LBX86_MAC_ST0,L_CTYPE_FLOAT,L_PTYPE_NULL
#define MSPEC_st1 LBX86_MAC_ST1,L_CTYPE_FLOAT,L_PTYPE_NULL
#define MSPEC_st2 LBX86_MAC_ST2,L_CTYPE_FLOAT,L_PTYPE_NULL
#define MSPEC_st3 LBX86_MAC_ST3,L_CTYPE_FLOAT,L_PTYPE_NULL
#define MSPEC_st4 LBX86_MAC_ST4,L_CTYPE_FLOAT,L_PTYPE_NULL
#define MSPEC_st5 LBX86_MAC_ST5,L_CTYPE_FLOAT,L_PTYPE_NULL
#define MSPEC_st6 LBX86_MAC_ST6,L_CTYPE_FLOAT,L_PTYPE_NULL
#define MSPEC_st7 LBX86_MAC_ST7,L_CTYPE_FLOAT,L_PTYPE_NULL
#define MSPEC_all_mm LBX86_MAC_ALL_MM,L_CTYPE_LONG,L_PTYPE_NULL
#define MSPEC_mm0 LBX86_MAC_MM0,L_CTYPE_LONG,L_PTYPE_NULL
#define MSPEC_mm1 LBX86_MAC_MM1,L_CTYPE_LONG,L_PTYPE_NULL
#define MSPEC_mm2 LBX86_MAC_MM2,L_CTYPE_LONG,L_PTYPE_NULL
#define MSPEC_mm3 LBX86_MAC_MM3,L_CTYPE_LONG,L_PTYPE_NULL
#define MSPEC_mm4 LBX86_MAC_MM4,L_CTYPE_LONG,L_PTYPE_NULL
#define MSPEC_mm5 LBX86_MAC_MM5,L_CTYPE_LONG,L_PTYPE_NULL
#define MSPEC_mm6 LBX86_MAC_MM6,L_CTYPE_LONG,L_PTYPE_NULL
#define MSPEC_mm7 LBX86_MAC_MM7,L_CTYPE_LONG,L_PTYPE_NULL

#define MSPEC_addr LBX86_MAC_ADDR,L_CTYPE_VOID,L_PTYPE_NULL

#define MSPEC_fpsw LBX86_MAC_FPSW,L_CTYPE_VOID,L_PTYPE_NULL
#define MSPEC_fpcw LBX86_MAC_FPCW,L_CTYPE_VOID,L_PTYPE_NULL

  M_assert ((len >= 34), "M_conflicting_operands_bx86: "
	    "len of conflict_array is too short");

  if (L_is_macro (operand))
    {

      macval = operand->value.mac;

      switch (macval)
	{

	case LBX86_MAC_OSZAPC_FLAGS:	/*1 */
	  conflict_array[0] = L_new_macro_operand (MSPEC_oszapc_flags);	/*1 */
	  conflict_array[1] = L_new_macro_operand (MSPEC_c_flags);	/*2 */
	  conflict_array[2] = L_new_macro_operand (MSPEC_oszap_flags);	/*3 */
	  conflict_array[3] = L_new_macro_operand (MSPEC_oc_flags);	/*4 */
	  conflict_array[4] = L_new_macro_operand (MSPEC_szapc_flags);	/*5 */
	  conflict_array[5] = L_new_macro_operand (MSPEC_oszpc_flags);	/*6 */
	  conflict_array[6] = L_new_macro_operand (MSPEC_z_flags);	/*7 */
	  conflict_array[7] = L_new_macro_operand (MSPEC_zc_flags);	/*8 */
	  conflict_array[8] = L_new_macro_operand (MSPEC_osz_flags);	/*9 */
	  conflict_array[9] = L_new_macro_operand (MSPEC_os_flags);	/*10 */
	  conflict_array[10] = L_new_macro_operand (MSPEC_o_flags);	/*11 */
	  conflict_array[11] = L_new_macro_operand (MSPEC_s_flags);	/*12 */
	  conflict_array[12] = L_new_macro_operand (MSPEC_a_flags);	/*13 */
	  conflict_array[13] = L_new_macro_operand (MSPEC_p_flags);	/*14 */
	  return 14;

	case LBX86_MAC_C_FLAGS:	/*2 */
	  conflict_array[0] = L_new_macro_operand (MSPEC_oszapc_flags);	/*1 */
	  conflict_array[1] = L_new_macro_operand (MSPEC_c_flags);	/*2 */
	  conflict_array[2] = L_new_macro_operand (MSPEC_oc_flags);	/*4 */
	  conflict_array[3] = L_new_macro_operand (MSPEC_szapc_flags);	/*5 */
	  conflict_array[4] = L_new_macro_operand (MSPEC_oszpc_flags);	/*6 */
	  conflict_array[5] = L_new_macro_operand (MSPEC_zc_flags);	/*8 */
	  return 6;

	case LBX86_MAC_OSZAP_FLAGS:	/*3 */
	  conflict_array[0] = L_new_macro_operand (MSPEC_oszapc_flags);	/*1 */
	  conflict_array[1] = L_new_macro_operand (MSPEC_oszap_flags);	/*3 */
	  conflict_array[2] = L_new_macro_operand (MSPEC_oc_flags);	/*4 */
	  conflict_array[3] = L_new_macro_operand (MSPEC_szapc_flags);	/*5 */
	  conflict_array[4] = L_new_macro_operand (MSPEC_oszpc_flags);	/*6 */
	  conflict_array[5] = L_new_macro_operand (MSPEC_z_flags);	/*7 */
	  conflict_array[6] = L_new_macro_operand (MSPEC_zc_flags);	/*8 */
	  conflict_array[7] = L_new_macro_operand (MSPEC_osz_flags);	/*9 */
	  conflict_array[8] = L_new_macro_operand (MSPEC_os_flags);	/*10 */
	  conflict_array[9] = L_new_macro_operand (MSPEC_o_flags);	/*11 */
	  conflict_array[10] = L_new_macro_operand (MSPEC_s_flags);	/*12 */
	  conflict_array[11] = L_new_macro_operand (MSPEC_a_flags);	/*13 */
	  conflict_array[12] = L_new_macro_operand (MSPEC_p_flags);	/*14 */
	  return 13;

	case LBX86_MAC_OC_FLAGS:	/*4 */
	  conflict_array[0] = L_new_macro_operand (MSPEC_oszapc_flags);	/*1 */
	  conflict_array[1] = L_new_macro_operand (MSPEC_c_flags);	/*2 */
	  conflict_array[2] = L_new_macro_operand (MSPEC_oszap_flags);	/*3 */
	  conflict_array[3] = L_new_macro_operand (MSPEC_oc_flags);	/*4 */
	  conflict_array[4] = L_new_macro_operand (MSPEC_szapc_flags);	/*5 */
	  conflict_array[5] = L_new_macro_operand (MSPEC_oszpc_flags);	/*6 */
	  conflict_array[6] = L_new_macro_operand (MSPEC_zc_flags);	/*8 */
	  conflict_array[7] = L_new_macro_operand (MSPEC_osz_flags);	/*9 */
	  conflict_array[8] = L_new_macro_operand (MSPEC_os_flags);	/*10 */
	  conflict_array[9] = L_new_macro_operand (MSPEC_o_flags);	/*11 */
	  return 10;

	case LBX86_MAC_SZAPC_FLAGS:	/*5 */
	  conflict_array[0] = L_new_macro_operand (MSPEC_oszapc_flags);	/*1 */
	  conflict_array[1] = L_new_macro_operand (MSPEC_c_flags);	/*2 */
	  conflict_array[2] = L_new_macro_operand (MSPEC_oszap_flags);	/*3 */
	  conflict_array[3] = L_new_macro_operand (MSPEC_oc_flags);	/*4 */
	  conflict_array[4] = L_new_macro_operand (MSPEC_szapc_flags);	/*5 */
	  conflict_array[5] = L_new_macro_operand (MSPEC_oszpc_flags);	/*6 */
	  conflict_array[6] = L_new_macro_operand (MSPEC_z_flags);	/*7 */
	  conflict_array[7] = L_new_macro_operand (MSPEC_zc_flags);	/*8 */
	  conflict_array[8] = L_new_macro_operand (MSPEC_osz_flags);	/*9 */
	  conflict_array[9] = L_new_macro_operand (MSPEC_os_flags);	/*10 */
	  conflict_array[10] = L_new_macro_operand (MSPEC_s_flags);	/*12 */
	  conflict_array[11] = L_new_macro_operand (MSPEC_a_flags);	/*13 */
	  conflict_array[12] = L_new_macro_operand (MSPEC_p_flags);	/*14 */
	  return 13;

	case LBX86_MAC_OSZPC_FLAGS:	/*6 */
	  conflict_array[0] = L_new_macro_operand (MSPEC_oszapc_flags);	/*1 */
	  conflict_array[1] = L_new_macro_operand (MSPEC_c_flags);	/*2 */
	  conflict_array[2] = L_new_macro_operand (MSPEC_oszap_flags);	/*3 */
	  conflict_array[3] = L_new_macro_operand (MSPEC_oc_flags);	/*4 */
	  conflict_array[4] = L_new_macro_operand (MSPEC_szapc_flags);	/*5 */
	  conflict_array[5] = L_new_macro_operand (MSPEC_oszpc_flags);	/*6 */
	  conflict_array[6] = L_new_macro_operand (MSPEC_z_flags);	/*7 */
	  conflict_array[7] = L_new_macro_operand (MSPEC_zc_flags);	/*8 */
	  conflict_array[8] = L_new_macro_operand (MSPEC_osz_flags);	/*9 */
	  conflict_array[9] = L_new_macro_operand (MSPEC_os_flags);	/*10 */
	  conflict_array[10] = L_new_macro_operand (MSPEC_o_flags);	/*11 */
	  conflict_array[11] = L_new_macro_operand (MSPEC_s_flags);	/*12 */
	  conflict_array[12] = L_new_macro_operand (MSPEC_p_flags);	/*14 */
	  return 10;

	case LBX86_MAC_Z_FLAGS:	/*7 */
	  conflict_array[0] = L_new_macro_operand (MSPEC_oszapc_flags);	/*1 */
	  conflict_array[1] = L_new_macro_operand (MSPEC_oszap_flags);	/*3 */
	  conflict_array[2] = L_new_macro_operand (MSPEC_szapc_flags);	/*5 */
	  conflict_array[3] = L_new_macro_operand (MSPEC_oszpc_flags);	/*6 */
	  conflict_array[4] = L_new_macro_operand (MSPEC_z_flags);	/*7 */
	  conflict_array[5] = L_new_macro_operand (MSPEC_zc_flags);	/*8 */
	  conflict_array[6] = L_new_macro_operand (MSPEC_osz_flags);	/*9 */
	  return 7;

	case LBX86_MAC_ZC_FLAGS:	/*8 */
	  conflict_array[0] = L_new_macro_operand (MSPEC_oszapc_flags);	/*1 */
	  conflict_array[1] = L_new_macro_operand (MSPEC_c_flags);	/*2 */
	  conflict_array[2] = L_new_macro_operand (MSPEC_oszap_flags);	/*3 */
	  conflict_array[3] = L_new_macro_operand (MSPEC_oc_flags);	/*4 */
	  conflict_array[4] = L_new_macro_operand (MSPEC_szapc_flags);	/*5 */
	  conflict_array[5] = L_new_macro_operand (MSPEC_oszpc_flags);	/*6 */
	  conflict_array[6] = L_new_macro_operand (MSPEC_z_flags);	/*7 */
	  conflict_array[7] = L_new_macro_operand (MSPEC_zc_flags);	/*8 */
	  conflict_array[8] = L_new_macro_operand (MSPEC_osz_flags);	/*9 */
	  return 9;

	case LBX86_MAC_OSZ_FLAGS:	/*9 */
	  conflict_array[0] = L_new_macro_operand (MSPEC_oszapc_flags);	/*1 */
	  conflict_array[1] = L_new_macro_operand (MSPEC_oszap_flags);	/*3 */
	  conflict_array[2] = L_new_macro_operand (MSPEC_oc_flags);	/*4 */
	  conflict_array[3] = L_new_macro_operand (MSPEC_szapc_flags);	/*5 */
	  conflict_array[4] = L_new_macro_operand (MSPEC_oszpc_flags);	/*6 */
	  conflict_array[5] = L_new_macro_operand (MSPEC_z_flags);	/*7 */
	  conflict_array[6] = L_new_macro_operand (MSPEC_zc_flags);	/*8 */
	  conflict_array[7] = L_new_macro_operand (MSPEC_osz_flags);	/*9 */
	  conflict_array[8] = L_new_macro_operand (MSPEC_os_flags);	/*10 */
	  conflict_array[9] = L_new_macro_operand (MSPEC_o_flags);	/*11 */
	  conflict_array[10] = L_new_macro_operand (MSPEC_s_flags);	/*12 */
	  return 11;

	case LBX86_MAC_OS_FLAGS:	/*10 */
	  conflict_array[0] = L_new_macro_operand (MSPEC_oszapc_flags);	/*1 */
	  conflict_array[1] = L_new_macro_operand (MSPEC_oszap_flags);	/*3 */
	  conflict_array[2] = L_new_macro_operand (MSPEC_oc_flags);	/*4 */
	  conflict_array[3] = L_new_macro_operand (MSPEC_szapc_flags);	/*5 */
	  conflict_array[4] = L_new_macro_operand (MSPEC_oszpc_flags);	/*6 */
	  conflict_array[5] = L_new_macro_operand (MSPEC_osz_flags);	/*9 */
	  conflict_array[6] = L_new_macro_operand (MSPEC_os_flags);	/*10 */
	  conflict_array[7] = L_new_macro_operand (MSPEC_o_flags);	/*11 */
	  conflict_array[8] = L_new_macro_operand (MSPEC_s_flags);	/*12 */
	  return 9;

	case LBX86_MAC_O_FLAGS:	/*11 */
	  conflict_array[0] = L_new_macro_operand (MSPEC_oszapc_flags);	/*1 */
	  conflict_array[1] = L_new_macro_operand (MSPEC_oszap_flags);	/*3 */
	  conflict_array[2] = L_new_macro_operand (MSPEC_oc_flags);	/*4 */
	  conflict_array[3] = L_new_macro_operand (MSPEC_oszpc_flags);	/*6 */
	  conflict_array[4] = L_new_macro_operand (MSPEC_osz_flags);	/*9 */
	  conflict_array[5] = L_new_macro_operand (MSPEC_os_flags);	/*10 */
	  conflict_array[6] = L_new_macro_operand (MSPEC_o_flags);	/*11 */
	  return 7;

	case LBX86_MAC_S_FLAGS:	/*12 */
	  conflict_array[0] = L_new_macro_operand (MSPEC_oszapc_flags);	/*1 */
	  conflict_array[1] = L_new_macro_operand (MSPEC_oszap_flags);	/*3 */
	  conflict_array[2] = L_new_macro_operand (MSPEC_szapc_flags);	/*5 */
	  conflict_array[3] = L_new_macro_operand (MSPEC_oszpc_flags);	/*6 */
	  conflict_array[4] = L_new_macro_operand (MSPEC_osz_flags);	/*9 */
	  conflict_array[5] = L_new_macro_operand (MSPEC_os_flags);	/*10 */
	  conflict_array[6] = L_new_macro_operand (MSPEC_s_flags);	/*12 */
	  return 7;

	case LBX86_MAC_A_FLAGS:	/*13 */
	  conflict_array[0] = L_new_macro_operand (MSPEC_oszapc_flags);	/*1 */
	  conflict_array[1] = L_new_macro_operand (MSPEC_oszap_flags);	/*3 */
	  conflict_array[2] = L_new_macro_operand (MSPEC_szapc_flags);	/*5 */
	  conflict_array[3] = L_new_macro_operand (MSPEC_a_flags);	/*13 */
	  return 4;

	case LBX86_MAC_P_FLAGS:	/*14 */
	  conflict_array[0] = L_new_macro_operand (MSPEC_oszapc_flags);	/*1 */
	  conflict_array[1] = L_new_macro_operand (MSPEC_oszap_flags);	/*3 */
	  conflict_array[2] = L_new_macro_operand (MSPEC_szapc_flags);	/*5 */
	  conflict_array[3] = L_new_macro_operand (MSPEC_oszpc_flags);	/*6 */
	  conflict_array[4] = L_new_macro_operand (MSPEC_p_flags);	/*14 */
	  return 5;

	case LBX86_MAC_EAX:
	  conflict_array[0] = L_new_macro_operand (MSPEC_eax);
	  conflict_array[1] = L_new_macro_operand (MSPEC_ax);
	  conflict_array[2] = L_new_macro_operand (MSPEC_al);
	  conflict_array[3] = L_new_macro_operand (MSPEC_ah);
	  conflict_array[4] = L_new_macro_operand (MSPEC_gp_reg32);
	  conflict_array[5] = L_new_macro_operand (MSPEC_gp_reg16);
	  conflict_array[6] = L_new_macro_operand (MSPEC_eaxh);
	  return 7;

	case LBX86_MAC_AX:
	  conflict_array[0] = L_new_macro_operand (MSPEC_eax);
	  conflict_array[1] = L_new_macro_operand (MSPEC_ax);
	  conflict_array[2] = L_new_macro_operand (MSPEC_al);
	  conflict_array[3] = L_new_macro_operand (MSPEC_ah);
	  conflict_array[4] = L_new_macro_operand (MSPEC_gp_reg32);
	  conflict_array[5] = L_new_macro_operand (MSPEC_gp_reg16);
	  return 6;

	case LBX86_MAC_AL:
	  conflict_array[0] = L_new_macro_operand (MSPEC_eax);
	  conflict_array[1] = L_new_macro_operand (MSPEC_ax);
	  conflict_array[2] = L_new_macro_operand (MSPEC_al);
	  conflict_array[3] = L_new_macro_operand (MSPEC_gp_reg32);
	  conflict_array[4] = L_new_macro_operand (MSPEC_gp_reg16);
	  return 5;

	case LBX86_MAC_AH:
	  conflict_array[0] = L_new_macro_operand (MSPEC_eax);
	  conflict_array[1] = L_new_macro_operand (MSPEC_ax);
	  conflict_array[2] = L_new_macro_operand (MSPEC_ah);
	  conflict_array[3] = L_new_macro_operand (MSPEC_gp_reg32);
	  conflict_array[4] = L_new_macro_operand (MSPEC_gp_reg16);
	  return 5;

	case LBX86_MAC_EAXH:
	  conflict_array[0] = L_new_macro_operand (MSPEC_eax);
	  conflict_array[1] = L_new_macro_operand (MSPEC_eaxh);
	  conflict_array[2] = L_new_macro_operand (MSPEC_gp_reg32);
	  return 3;

	case LBX86_MAC_EBX:
	  conflict_array[0] = L_new_macro_operand (MSPEC_ebx);
	  conflict_array[1] = L_new_macro_operand (MSPEC_bx);
	  conflict_array[2] = L_new_macro_operand (MSPEC_bl);
	  conflict_array[3] = L_new_macro_operand (MSPEC_bh);
	  conflict_array[4] = L_new_macro_operand (MSPEC_gp_reg32);
	  conflict_array[5] = L_new_macro_operand (MSPEC_gp_reg16);
	  conflict_array[6] = L_new_macro_operand (MSPEC_ebxh);
	  return 7;

	case LBX86_MAC_BX:
	  conflict_array[0] = L_new_macro_operand (MSPEC_ebx);
	  conflict_array[1] = L_new_macro_operand (MSPEC_bx);
	  conflict_array[2] = L_new_macro_operand (MSPEC_bl);
	  conflict_array[3] = L_new_macro_operand (MSPEC_bh);
	  conflict_array[4] = L_new_macro_operand (MSPEC_gp_reg32);
	  conflict_array[5] = L_new_macro_operand (MSPEC_gp_reg16);
	  return 6;

	case LBX86_MAC_BL:
	  conflict_array[0] = L_new_macro_operand (MSPEC_ebx);
	  conflict_array[1] = L_new_macro_operand (MSPEC_bx);
	  conflict_array[2] = L_new_macro_operand (MSPEC_bl);
	  conflict_array[3] = L_new_macro_operand (MSPEC_gp_reg32);
	  conflict_array[4] = L_new_macro_operand (MSPEC_gp_reg16);
	  return 5;

	case LBX86_MAC_BH:
	  conflict_array[0] = L_new_macro_operand (MSPEC_ebx);
	  conflict_array[1] = L_new_macro_operand (MSPEC_bx);
	  conflict_array[2] = L_new_macro_operand (MSPEC_bh);
	  conflict_array[3] = L_new_macro_operand (MSPEC_gp_reg32);
	  conflict_array[4] = L_new_macro_operand (MSPEC_gp_reg16);
	  return 5;

	case LBX86_MAC_EBXH:
	  conflict_array[0] = L_new_macro_operand (MSPEC_ebx);
	  conflict_array[1] = L_new_macro_operand (MSPEC_ebxh);
	  conflict_array[2] = L_new_macro_operand (MSPEC_gp_reg32);
	  return 3;

	case LBX86_MAC_ECX:
	  conflict_array[0] = L_new_macro_operand (MSPEC_ecx);
	  conflict_array[1] = L_new_macro_operand (MSPEC_cx);
	  conflict_array[2] = L_new_macro_operand (MSPEC_cl);
	  conflict_array[3] = L_new_macro_operand (MSPEC_ch);
	  conflict_array[4] = L_new_macro_operand (MSPEC_gp_reg32);
	  conflict_array[5] = L_new_macro_operand (MSPEC_gp_reg16);
	  conflict_array[6] = L_new_macro_operand (MSPEC_ecxh);
	  return 7;

	case LBX86_MAC_CX:
	  conflict_array[0] = L_new_macro_operand (MSPEC_ecx);
	  conflict_array[1] = L_new_macro_operand (MSPEC_cx);
	  conflict_array[2] = L_new_macro_operand (MSPEC_cl);
	  conflict_array[3] = L_new_macro_operand (MSPEC_ch);
	  conflict_array[4] = L_new_macro_operand (MSPEC_gp_reg32);
	  conflict_array[5] = L_new_macro_operand (MSPEC_gp_reg16);
	  return 6;

	case LBX86_MAC_CL:
	  conflict_array[0] = L_new_macro_operand (MSPEC_ecx);
	  conflict_array[1] = L_new_macro_operand (MSPEC_cx);
	  conflict_array[2] = L_new_macro_operand (MSPEC_cl);
	  conflict_array[3] = L_new_macro_operand (MSPEC_gp_reg32);
	  conflict_array[4] = L_new_macro_operand (MSPEC_gp_reg16);
	  return 5;

	case LBX86_MAC_CH:
	  conflict_array[0] = L_new_macro_operand (MSPEC_ecx);
	  conflict_array[1] = L_new_macro_operand (MSPEC_cx);
	  conflict_array[2] = L_new_macro_operand (MSPEC_ch);
	  conflict_array[3] = L_new_macro_operand (MSPEC_gp_reg32);
	  conflict_array[4] = L_new_macro_operand (MSPEC_gp_reg16);
	  return 5;

	case LBX86_MAC_ECXH:
	  conflict_array[0] = L_new_macro_operand (MSPEC_ecx);
	  conflict_array[1] = L_new_macro_operand (MSPEC_ecxh);
	  conflict_array[2] = L_new_macro_operand (MSPEC_gp_reg32);
	  return 3;

	case LBX86_MAC_EDX:
	  conflict_array[0] = L_new_macro_operand (MSPEC_edx);
	  conflict_array[1] = L_new_macro_operand (MSPEC_dx);
	  conflict_array[2] = L_new_macro_operand (MSPEC_dl);
	  conflict_array[3] = L_new_macro_operand (MSPEC_dh);
	  conflict_array[4] = L_new_macro_operand (MSPEC_gp_reg32);
	  conflict_array[5] = L_new_macro_operand (MSPEC_gp_reg16);
	  conflict_array[6] = L_new_macro_operand (MSPEC_edxh);
	  return 7;

	case LBX86_MAC_DX:
	  conflict_array[0] = L_new_macro_operand (MSPEC_edx);
	  conflict_array[1] = L_new_macro_operand (MSPEC_dx);
	  conflict_array[2] = L_new_macro_operand (MSPEC_dl);
	  conflict_array[3] = L_new_macro_operand (MSPEC_dh);
	  conflict_array[4] = L_new_macro_operand (MSPEC_gp_reg32);
	  conflict_array[5] = L_new_macro_operand (MSPEC_gp_reg16);
	  return 6;

	case LBX86_MAC_DL:
	  conflict_array[0] = L_new_macro_operand (MSPEC_edx);
	  conflict_array[1] = L_new_macro_operand (MSPEC_dx);
	  conflict_array[2] = L_new_macro_operand (MSPEC_dl);
	  conflict_array[3] = L_new_macro_operand (MSPEC_gp_reg32);
	  conflict_array[4] = L_new_macro_operand (MSPEC_gp_reg16);
	  return 5;

	case LBX86_MAC_DH:
	  conflict_array[0] = L_new_macro_operand (MSPEC_edx);
	  conflict_array[1] = L_new_macro_operand (MSPEC_dx);
	  conflict_array[2] = L_new_macro_operand (MSPEC_dh);
	  conflict_array[3] = L_new_macro_operand (MSPEC_gp_reg32);
	  conflict_array[4] = L_new_macro_operand (MSPEC_gp_reg16);
	  return 5;

	case LBX86_MAC_EDXH:
	  conflict_array[0] = L_new_macro_operand (MSPEC_edx);
	  conflict_array[1] = L_new_macro_operand (MSPEC_edxh);
	  conflict_array[2] = L_new_macro_operand (MSPEC_gp_reg32);
	  return 3;

	case LBX86_MAC_ESI:
	  conflict_array[0] = L_new_macro_operand (MSPEC_esi);
	  conflict_array[1] = L_new_macro_operand (MSPEC_si);
	  conflict_array[2] = L_new_macro_operand (MSPEC_gp_reg32);
	  conflict_array[3] = L_new_macro_operand (MSPEC_gp_reg16);
	  conflict_array[4] = L_new_macro_operand (MSPEC_esih);
	  return 5;

	case LBX86_MAC_SI:
	  conflict_array[0] = L_new_macro_operand (MSPEC_esi);
	  conflict_array[1] = L_new_macro_operand (MSPEC_si);
	  conflict_array[2] = L_new_macro_operand (MSPEC_gp_reg32);
	  conflict_array[3] = L_new_macro_operand (MSPEC_gp_reg16);
	  return 4;

	case LBX86_MAC_ESIH:
	  conflict_array[0] = L_new_macro_operand (MSPEC_esi);
	  conflict_array[1] = L_new_macro_operand (MSPEC_esih);
	  conflict_array[2] = L_new_macro_operand (MSPEC_gp_reg32);
	  return 3;

	case LBX86_MAC_EDI:
	  conflict_array[0] = L_new_macro_operand (MSPEC_edi);
	  conflict_array[1] = L_new_macro_operand (MSPEC_di);
	  conflict_array[2] = L_new_macro_operand (MSPEC_gp_reg32);
	  conflict_array[3] = L_new_macro_operand (MSPEC_gp_reg16);
	  conflict_array[4] = L_new_macro_operand (MSPEC_edih);
	  return 5;

	case LBX86_MAC_DI:
	  conflict_array[0] = L_new_macro_operand (MSPEC_edi);
	  conflict_array[1] = L_new_macro_operand (MSPEC_di);
	  conflict_array[2] = L_new_macro_operand (MSPEC_gp_reg32);
	  conflict_array[3] = L_new_macro_operand (MSPEC_gp_reg16);
	  return 4;

	case LBX86_MAC_EDIH:
	  conflict_array[0] = L_new_macro_operand (MSPEC_edi);
	  conflict_array[1] = L_new_macro_operand (MSPEC_edih);
	  conflict_array[2] = L_new_macro_operand (MSPEC_gp_reg32);
	  return 3;

	case LBX86_MAC_EBP:
	  conflict_array[0] = L_new_macro_operand (MSPEC_ebp);
	  conflict_array[1] = L_new_macro_operand (MSPEC_bp);
	  conflict_array[2] = L_new_macro_operand (MSPEC_gp_reg32);
	  conflict_array[3] = L_new_macro_operand (MSPEC_gp_reg16);
	  conflict_array[4] = L_new_macro_operand (MSPEC_ebph);
	  return 5;

	case LBX86_MAC_BP:
	  conflict_array[0] = L_new_macro_operand (MSPEC_ebp);
	  conflict_array[1] = L_new_macro_operand (MSPEC_bp);
	  conflict_array[2] = L_new_macro_operand (MSPEC_gp_reg32);
	  conflict_array[3] = L_new_macro_operand (MSPEC_gp_reg16);
	  return 4;

	case LBX86_MAC_EBPH:
	  conflict_array[0] = L_new_macro_operand (MSPEC_ebp);
	  conflict_array[1] = L_new_macro_operand (MSPEC_ebph);
	  conflict_array[2] = L_new_macro_operand (MSPEC_gp_reg32);
	  return 3;

	case LBX86_MAC_ESP:
	  conflict_array[0] = L_new_macro_operand (MSPEC_esp);
	  conflict_array[1] = L_new_macro_operand (MSPEC_sp);
	  conflict_array[2] = L_new_macro_operand (MSPEC_gp_reg32);
	  conflict_array[3] = L_new_macro_operand (MSPEC_gp_reg16);
	  conflict_array[4] = L_new_macro_operand (MSPEC_esph);
	  return 5;

	case LBX86_MAC_SP:
	  conflict_array[0] = L_new_macro_operand (MSPEC_esp);
	  conflict_array[1] = L_new_macro_operand (MSPEC_sp);
	  conflict_array[2] = L_new_macro_operand (MSPEC_gp_reg32);
	  conflict_array[3] = L_new_macro_operand (MSPEC_gp_reg16);
	  return 4;

	case LBX86_MAC_ESPH:
	  conflict_array[0] = L_new_macro_operand (MSPEC_esp);
	  conflict_array[1] = L_new_macro_operand (MSPEC_esph);
	  conflict_array[2] = L_new_macro_operand (MSPEC_gp_reg32);
	  return 3;

	case LBX86_MAC_ALL_GP_REG32:
	  conflict_array[0] = L_new_macro_operand (MSPEC_eax);
	  conflict_array[1] = L_new_macro_operand (MSPEC_ebx);
	  conflict_array[2] = L_new_macro_operand (MSPEC_ecx);
	  conflict_array[3] = L_new_macro_operand (MSPEC_edx);
	  conflict_array[4] = L_new_macro_operand (MSPEC_esi);
	  conflict_array[5] = L_new_macro_operand (MSPEC_edi);
	  conflict_array[6] = L_new_macro_operand (MSPEC_ebp);
	  conflict_array[7] = L_new_macro_operand (MSPEC_esp);
	  conflict_array[8] = L_new_macro_operand (MSPEC_ax);
	  conflict_array[9] = L_new_macro_operand (MSPEC_bx);
	  conflict_array[10] = L_new_macro_operand (MSPEC_cx);
	  conflict_array[11] = L_new_macro_operand (MSPEC_dx);
	  conflict_array[12] = L_new_macro_operand (MSPEC_si);
	  conflict_array[13] = L_new_macro_operand (MSPEC_di);
	  conflict_array[14] = L_new_macro_operand (MSPEC_bp);
	  conflict_array[15] = L_new_macro_operand (MSPEC_sp);
	  conflict_array[16] = L_new_macro_operand (MSPEC_eaxh);
	  conflict_array[17] = L_new_macro_operand (MSPEC_ebxh);
	  conflict_array[18] = L_new_macro_operand (MSPEC_ecxh);
	  conflict_array[19] = L_new_macro_operand (MSPEC_edxh);
	  conflict_array[20] = L_new_macro_operand (MSPEC_esih);
	  conflict_array[21] = L_new_macro_operand (MSPEC_edih);
	  conflict_array[22] = L_new_macro_operand (MSPEC_ebph);
	  conflict_array[23] = L_new_macro_operand (MSPEC_esph);
	  conflict_array[24] = L_new_macro_operand (MSPEC_al);
	  conflict_array[25] = L_new_macro_operand (MSPEC_bl);
	  conflict_array[26] = L_new_macro_operand (MSPEC_cl);
	  conflict_array[27] = L_new_macro_operand (MSPEC_dl);
	  conflict_array[28] = L_new_macro_operand (MSPEC_ah);
	  conflict_array[29] = L_new_macro_operand (MSPEC_bh);
	  conflict_array[30] = L_new_macro_operand (MSPEC_ch);
	  conflict_array[31] = L_new_macro_operand (MSPEC_dh);
	  conflict_array[32] = L_new_macro_operand (MSPEC_gp_reg32);
	  conflict_array[33] = L_new_macro_operand (MSPEC_gp_reg16);
	  return 34;

	case LBX86_MAC_ALL_GP_REG16:
	  conflict_array[0] = L_new_macro_operand (MSPEC_eax);
	  conflict_array[1] = L_new_macro_operand (MSPEC_ebx);
	  conflict_array[2] = L_new_macro_operand (MSPEC_ecx);
	  conflict_array[3] = L_new_macro_operand (MSPEC_edx);
	  conflict_array[4] = L_new_macro_operand (MSPEC_esi);
	  conflict_array[5] = L_new_macro_operand (MSPEC_edi);
	  conflict_array[6] = L_new_macro_operand (MSPEC_ebp);
	  conflict_array[7] = L_new_macro_operand (MSPEC_esp);
	  conflict_array[8] = L_new_macro_operand (MSPEC_ax);
	  conflict_array[9] = L_new_macro_operand (MSPEC_bx);
	  conflict_array[10] = L_new_macro_operand (MSPEC_cx);
	  conflict_array[11] = L_new_macro_operand (MSPEC_dx);
	  conflict_array[12] = L_new_macro_operand (MSPEC_si);
	  conflict_array[13] = L_new_macro_operand (MSPEC_di);
	  conflict_array[14] = L_new_macro_operand (MSPEC_bp);
	  conflict_array[15] = L_new_macro_operand (MSPEC_sp);
	  conflict_array[16] = L_new_macro_operand (MSPEC_al);
	  conflict_array[17] = L_new_macro_operand (MSPEC_bl);
	  conflict_array[18] = L_new_macro_operand (MSPEC_cl);
	  conflict_array[19] = L_new_macro_operand (MSPEC_dl);
	  conflict_array[20] = L_new_macro_operand (MSPEC_ah);
	  conflict_array[21] = L_new_macro_operand (MSPEC_bh);
	  conflict_array[22] = L_new_macro_operand (MSPEC_ch);
	  conflict_array[23] = L_new_macro_operand (MSPEC_dh);
	  conflict_array[24] = L_new_macro_operand (MSPEC_gp_reg32);
	  conflict_array[25] = L_new_macro_operand (MSPEC_gp_reg16);
	  return 26;

	case LBX86_MAC_ST0:
	case LBX86_MAC_ST1:
	case LBX86_MAC_ST2:
	case LBX86_MAC_ST3:
	case LBX86_MAC_ST4:
	case LBX86_MAC_ST5:
	case LBX86_MAC_ST6:
	case LBX86_MAC_ST7:
	  conflict_array[0] = L_copy_operand (operand);
	  conflict_array[1] = L_new_macro_operand (MSPEC_all_fst);
	  return 2;

	case LBX86_MAC_ALL_FST:
	  conflict_array[0] = L_new_macro_operand (MSPEC_all_fst);
	  conflict_array[1] = L_new_macro_operand (MSPEC_st0);
	  conflict_array[2] = L_new_macro_operand (MSPEC_st1);
	  conflict_array[3] = L_new_macro_operand (MSPEC_st2);
	  conflict_array[4] = L_new_macro_operand (MSPEC_st3);
	  conflict_array[5] = L_new_macro_operand (MSPEC_st4);
	  conflict_array[6] = L_new_macro_operand (MSPEC_st5);
	  conflict_array[7] = L_new_macro_operand (MSPEC_st6);
	  conflict_array[8] = L_new_macro_operand (MSPEC_st7);
	  return 9;

	case LBX86_MAC_MM0:
	case LBX86_MAC_MM1:
	case LBX86_MAC_MM2:
	case LBX86_MAC_MM3:
	case LBX86_MAC_MM4:
	case LBX86_MAC_MM5:
	case LBX86_MAC_MM6:
	case LBX86_MAC_MM7:
	  conflict_array[0] = L_copy_operand (operand);
	  conflict_array[1] = L_new_macro_operand (MSPEC_all_mm);
	  return 2;

	case LBX86_MAC_ALL_MM:
	  conflict_array[0] = L_new_macro_operand (MSPEC_all_mm);
	  conflict_array[1] = L_new_macro_operand (MSPEC_mm0);
	  conflict_array[2] = L_new_macro_operand (MSPEC_mm1);
	  conflict_array[3] = L_new_macro_operand (MSPEC_mm2);
	  conflict_array[4] = L_new_macro_operand (MSPEC_mm3);
	  conflict_array[5] = L_new_macro_operand (MSPEC_mm4);
	  conflict_array[6] = L_new_macro_operand (MSPEC_mm5);
	  conflict_array[7] = L_new_macro_operand (MSPEC_mm6);
	  conflict_array[8] = L_new_macro_operand (MSPEC_mm7);
	  return 9;

	case LBX86_MAC_ADDR:
	  /* Since addr never shows up in any 
	     dest, the default case should be OK. */
	default:
	  conflict_array[0] = L_copy_operand (operand);
	  return 1;

	}
    }
  else
    M_assert (0, "M_conflicting_operands_bx86: unsupported operand type");
  return 1;
}


/* Stack and string modifications can break this convention. MCM */
void
M_get_memory_operands_bx86 (int *first, int *number, int proc_opc)
{

  *first = 3;
  *number = 4;
  return;
}

int
M_memory_access_size_bx86 (L_Oper * op)
{

  return LBX86_EXT (op)->mem_size;

}

int
M_get_data_type_bx86 (L_Oper * op)
{
  I_punt ("ml_bx86: Unimplemented: M_get_data_type_bx86");

  return (-1); /* I_punt doesn't return */
#if 0

  switch (op->proc_opc)
    {

      /* Proc_opc for sign extended loads of byte or word -DML */
    case LBX86op_MOVXL:
      if ((op->opc == Lop_LD_UC2) || (op->opc == Lop_LD_C2))
	return (L_MEMORY_ACCESS_CHAR2);
      else if ((op->opc == Lop_LD_UC) || (op->opc == Lop_LD_C))
	return (L_MEMORY_ACCESS_CHAR);
      else
	I_punt
	  ("M_get_data_type_x86 (op %i): unexpected opc %i for Lop_MOVXL\n",
	   op->id, op->opc);

    case LBX86op_FPUSH_I:
    case LBX86op_FPOP_I:
    case LBX86op_PUSH:
/* added by wfd  */
    case LBX86op_INT_LOAD:
    case LBX86op_INT_STORE:
    case LBX86op_CISC_TO_REG:
    case LBX86op_CISC_TO_REG_TEST:
    case LBX86op_CISC_BRANCH:
    case LBX86op_CISC_TO_MEM:
    case LBX86op_CISC_TO_MEM_INC:
    case LBX86op_CISC_TO_MEM_DEC:
    case LBX86op_CISC_TO_MEM_NEGATE:
    case LBX86op_CISC_TO_REG_CMP:
/*3-14-95*/
      return (L_MEMORY_ACCESS_INT);

    case LBX86op_EQ_FMEM:
    case LBX86op_NE_FMEM:
    case LBX86op_GT_FMEM:
    case LBX86op_GE_FMEM:
    case LBX86op_LT_FMEM:
    case LBX86op_LE_FMEM:
    case LBX86op_ADD_FMEM:
    case LBX86op_SUB_FMEM:
    case LBX86op_SUBR_FMEM:
    case LBX86op_MUL_FMEM:
    case LBX86op_DIV_FMEM:
    case LBX86op_DIVR_FMEM:
    case LBX86op_FSTCW:
    case LBX86op_FLDCW:
/* added by wfd  */
    case LBX86op_FLOAT_LOAD:
    case LBX86op_FLOAT_STORE:
/*3-14-95*/
      return (L_MEMORY_ACCESS_REAL);

    case LBX86op_EQ_F2MEM:
    case LBX86op_NE_F2MEM:
    case LBX86op_GT_F2MEM:
    case LBX86op_GE_F2MEM:
    case LBX86op_LT_F2MEM:
    case LBX86op_LE_F2MEM:
    case LBX86op_ADD_F2MEM:
    case LBX86op_SUB_F2MEM:
    case LBX86op_SUBR_F2MEM:
    case LBX86op_MUL_F2MEM:
    case LBX86op_DIV_F2MEM:
    case LBX86op_DIVR_F2MEM:
      return (L_MEMORY_ACCESS_REAL);

    default:
      M_assert (0, "M_get_data_types_x86: unexpected proc_opc");
    }

  return (L_CTYPE_INT);

#endif

}

int
M_num_registers_bx86 (int ctype)
{
  I_punt ("ml_bx86: Unimplemented: M_num_registers_bx86 ");

  switch (ctype)
    {
    case L_CTYPE_INT:
      return (7);
    case L_CTYPE_FLOAT:
    case L_CTYPE_DOUBLE:
      return (10000);		/* cheat infinite */
    default:
      return (0);
    }
}

int
M_is_implicit_memory_op_bx86 (L_Oper * oper)
/*
 * Returns 0 if the operation does not implicitly generates
 * a memory address.  Otherwise, the return value indicates
 * the location of the operands which are used to generate the
 * address: 
	(1) "Standard" 5-operand load/store format, with the
		4 operands for the address in src1, src2, src3, and src4.
		Used for CISC operations.

	(2) Implicit stack reference.  Used for JSR / RET instructions.

	(3) Floating point memory operation - memory operands in src1 and src2.
 */
{

  I_punt ("ml_bx86: Unimplemented: M_is_implicit_memory_op_bx86");
  return (-1);  /* I_punt doesn't return */
#if 0

  /* only implicit if not ld/st */
  if (L_general_load_opcode (oper) || L_general_store_opcode (oper))
    return (0);

  /* 
   * Handle instructions which always generate memory addresses 
   * but aren't loads or stores  - currently, jsr's and rets
   * push or pop a memory operand to / from the stack. 
   */
  switch (oper->opc)
    {
    case Lop_JSR:
    case Lop_JSR_FS:
    case Lop_RTS:
    case Lop_RTS_FS:
      return 2;
    };

  /* 
   * Look at proc_opc to identify instructions which
   *  generate addresses.  This includes CISC operations
   *  and "special" cases like the floating pt operations, 
   *  POP instructions, etc. 
   */
  switch (oper->proc_opc)
    {
    case LBX86op_CISC_TO_REG:
    case LBX86op_CISC_TO_REG_TEST:
    case LBX86op_CISC_TO_MEM:
    case LBX86op_CISC_TO_MEM_INC:
    case LBX86op_CISC_TO_MEM_DEC:
    case LBX86op_CISC_TO_REG_CMP:
    case LBX86op_CISC_TO_MEM_NEGATE:

    case LBX86op_POP:
      /* Pops used to be annotated by moves - so if the pop is
         not an explicit LOAD / STORE instruction, signal that
         it generates an implicit memory address */
      return 1;

    case LBX86op_EQ_FMEM:
    case LBX86op_EQ_F2MEM:
    case LBX86op_NE_FMEM:
    case LBX86op_NE_F2MEM:
    case LBX86op_GT_FMEM:
    case LBX86op_GT_F2MEM:
    case LBX86op_GE_FMEM:
    case LBX86op_GE_F2MEM:
    case LBX86op_LT_FMEM:
    case LBX86op_LT_F2MEM:
    case LBX86op_LE_FMEM:
    case LBX86op_LE_F2MEM:

    case LBX86op_ADD_FMEM:
    case LBX86op_ADD_F2MEM:
    case LBX86op_SUB_FMEM:
    case LBX86op_SUB_F2MEM:
    case LBX86op_SUBR_FMEM:
    case LBX86op_SUBR_F2MEM:
    case LBX86op_MUL_FMEM:
    case LBX86op_MUL_F2MEM:
    case LBX86op_DIV_FMEM:
    case LBX86op_DIVR_FMEM:
    case LBX86op_DIV_F2MEM:
    case LBX86op_DIVR_F2MEM:
      return 1;

    };

  return 0;
#endif

}
