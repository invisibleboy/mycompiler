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
 *	File :	ml_x86.c 
 *	Desc :	Machine dependent specification.  
 *	Date :	Feb, 1992
 *	Auth :  Richard Hank, Wen-mei Hwu
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
#ifdef M_X86_FOR_HCODE
#include <Hcode/h_ccode.h>
#endif
#include <library/i_error.h>
#include <Lcode/l_main.h>
#include "m_spec.h"
#include "m_x86.h"

/*--------------------------------------------------------------------------*/
#define M_X86_SIZE_VOID		0
#define M_X86_SIZE_BIT		1
#define M_X86_SIZE_CHAR		8
#define M_X86_SIZE_SHORT	16
#define M_X86_SIZE_INT		32
#define M_X86_SIZE_LONG		32
#define M_X86_SIZE_FLOAT	32
#define M_X86_SIZE_DOUBLE	64
#define M_X86_SIZE_POINTER	32
#define M_X86_SIZE_UNION	-1
#define M_X86_SIZE_STRUCT	-1
#define M_X86_SIZE_BLOCK	-1
#define M_X86_SIZE_MAX	 	64

#define M_X86_ALIGN_VOID	-1
#define M_X86_ALIGN_BIT		1
#define M_X86_ALIGN_CHAR	8
#define M_X86_ALIGN_SHORT	16
#define M_X86_ALIGN_INT		32
#define M_X86_ALIGN_LONG	32
#define M_X86_ALIGN_FLOAT	32
#define M_X86_ALIGN_DOUBLE	32
#define M_X86_ALIGN_POINTER	32
#define M_X86_ALIGN_UNION	-1	/* depends on the field */
#define M_X86_ALIGN_STRUCT	-1
#define M_X86_ALIGN_BLOCK	-1
#define M_X86_ALIGN_MAX		64


/*--------------------------------------------------------------------------*/
#define M_X86_MAX_FNVAR_REG 		0
#define M_X86_SMALL_STRUCT_MAX 		04
#define MIN_PARAM_SIZE 	  		(16 * 0)

/* incoming and outgoing parameters */
#define M_X86_INT_BASE			0
#define M_X86_FLT_BASE			4

#define M_X86_RET_I32			0	/* return in EAX */
#define M_X86_RET_I64			0	/* return in EAX and EDX */
#define M_X86_RET_ST			0	/* no special pointer */
#define M_X86_RET_F			1	/* return in EAX */
#define M_X86_RET_F2			1	/* return in EAX */
#define M_X86_VAR_PTR	       		"$LV"

/*--------------------------------------------------------------------------*/
/*
 *	The following functions are for documentation only.
 *	The function overhead is too great for actual usage.
 */
void
M_x86_void (M_Type type)
{
  type->type = M_TYPE_VOID;
  type->unsign = 1;
  type->align = M_X86_ALIGN_VOID;
  type->size = M_X86_SIZE_VOID;
  type->nbits = 0;
}

void
M_x86_bit_long (M_Type type, int n)
{
  type->type = M_TYPE_BIT_LONG;
  type->unsign = 0;
  type->align = M_X86_ALIGN_BIT;
  type->size = n * M_X86_SIZE_BIT;
  type->nbits = n * M_X86_SIZE_BIT;
  M_assert ((n <= 32),
	    "M_bit_long: do not allow bit field of more than 32 bits");
}

void
M_x86_bit_int (M_Type type, int n)
{
  type->type = M_TYPE_BIT_INT;
  type->unsign = 0;
  type->align = M_X86_ALIGN_BIT;
  type->size = n * M_X86_SIZE_BIT;
  type->nbits = n * M_X86_SIZE_BIT;
  M_assert ((n <= 32),
	    "M_bit_int: do not allow bit field of more than 32 bits");
}

void
M_x86_bit_short (M_Type type, int n)
{
  type->type = M_TYPE_BIT_SHORT;
  type->unsign = 0;
  type->align = M_X86_ALIGN_BIT;
  type->size = n * M_X86_SIZE_BIT;
  type->nbits = n * M_X86_SIZE_BIT;
  M_assert ((n <= 16),
	    "M_bit_long: do not allow bit field of more than 16 bits");
}

void
M_x86_bit_char (M_Type type, int n)
{
  type->type = M_TYPE_BIT_CHAR;
  type->unsign = 0;
  type->align = M_X86_ALIGN_BIT;
  type->size = n * M_X86_SIZE_BIT;
  type->nbits = n * M_X86_SIZE_BIT;
  M_assert ((n <= 8),
	    "M_bit_char: do not allow bit field of more than 8 bits");
}

void
M_x86_float (M_Type type, int unsign)
{
  type->type = M_TYPE_FLOAT;
  type->unsign = unsign;
  type->align = M_X86_ALIGN_FLOAT;
  type->size = M_X86_SIZE_FLOAT;
  type->nbits = M_X86_SIZE_FLOAT;
}

void
M_x86_double (M_Type type, int unsign)
{
  type->type = M_TYPE_DOUBLE;
  type->unsign = unsign;
  type->align = M_X86_ALIGN_DOUBLE;
  type->size = M_X86_SIZE_DOUBLE;
  type->nbits = M_X86_SIZE_DOUBLE;
}

void
M_x86_pointer (M_Type type)
{
  type->type = M_TYPE_POINTER;
  type->unsign = 1;
  type->align = M_X86_ALIGN_POINTER;
  type->size = M_X86_SIZE_POINTER;
  type->nbits = M_X86_SIZE_POINTER;
}

/*--------------------------------------------------------------------------*/
int
M_x86_eval_type (M_Type type, M_Type ntype)
{
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
M_x86_eval_type2 (M_Type type, M_Type ntype)
{
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
M_x86_call_type (M_Type type, M_Type ntype)
{
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
M_x86_call_type2 (M_Type type, M_Type ntype)
{
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
M_x86_array_layout (M_Type type, int *offset)
{
  *offset = 0;			/* assume first element is aligned */
}

int
M_x86_array_align (M_Type type)
{
  return type->align;
}

int
M_x86_array_size (M_Type type, int dim)
{
  int mod, size, align;

  size = type->size;
  align = type->align;
  mod = size % align;
  if (mod != 0)
    size += (align - mod);

  return (size * dim);
}

/*--------------------------------------------------------------------------*/
void
M_x86_union_layout (int n, _M_Type * type, int *offset, int *bit_offset)
{
  int i;
  for (i = 0; i < n; i++)
    {				/* assume the union is aligned */
      offset[i] = 0;
      bit_offset[i] = 0;
    }
}

int
M_x86_union_align (int n, _M_Type * type)
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
  if (max < M_X86_ALIGN_CHAR)
    max = M_X86_ALIGN_CHAR;

  return max;
}

int
M_x86_union_size (int n, _M_Type * type)
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
  if (max_align < M_X86_ALIGN_CHAR)
    max_align = M_X86_ALIGN_CHAR;

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
M_x86_struct_layout (int n, _M_Type * type, int *base, int *bit_offset)
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
	  word_off = struct_off % M_X86_SIZE_INT;

	  /* if element doesn't fit in word, 
	     then its offset is beginning of the next word */
	  if ((word_off + size) > M_X86_SIZE_INT)
	    struct_off += (M_X86_SIZE_INT - word_off);

	  /* if element doesn't fit in aligned unit size, then successively
	     increase unit size until it does */
	  if (type[i].type == M_TYPE_BIT_CHAR)
	    {
	      bf_unit_size = M_X86_SIZE_CHAR;
	      bf_unit_off = struct_off % bf_unit_size;
#if 1
	      /* use for standard bitfields - JEM */
	      if ((bf_unit_off + size) > M_X86_SIZE_CHAR)
		{
		  type[i].type = M_TYPE_BIT_SHORT;
		  bf_unit_size = M_X86_SIZE_SHORT;
		}
#endif
	    }
	  if (type[i].type == M_TYPE_BIT_SHORT)
	    {
	      bf_unit_size = M_X86_SIZE_SHORT;
	      bf_unit_off = struct_off % bf_unit_size;
#if 1
	      /* use for standard bitfields - JEM */
	      if ((bf_unit_off + size) > M_X86_SIZE_SHORT)
		{
		  type[i].type = M_TYPE_BIT_LONG;
		  bf_unit_size = M_X86_SIZE_LONG;
		}
#endif
	    }
	  if (type[i].type == M_TYPE_BIT_LONG)
	    {
	      bf_unit_size = M_X86_SIZE_LONG;
	      bf_unit_off = struct_off % bf_unit_size;
	      if ((bf_unit_off + size) > M_X86_SIZE_LONG)
		{
		  /* problem:  can't increase size any more */
		  I_punt("M_x86_struct_layout:  "
			 "bitfield element does not fit in one word");
		}
	    }



#if 0
	  /* use for faster (non-standard) bitfields? - JEM */

	  /* adjust the element alignment to the unit size */
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
M_x86_struct_align (int n, _M_Type * type)
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
  if (max < M_X86_ALIGN_CHAR)
    max = M_X86_ALIGN_CHAR;
  return max;
}


int
M_x86_struct_size (int n, _M_Type * type, int struct_align)
{
  int i, struct_off;
  int mod, size, align, max_align, word_off;


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
	  word_off = struct_off % M_X86_SIZE_INT;

	  if ((word_off + size) > M_X86_SIZE_INT)
	    {
	      /* if element doesn't fit in word, 
	         then its offset is beginning of the next word */
	      struct_off += (M_X86_SIZE_INT - word_off);
	    }

	}


      /* adjust offset to required alignment */
      mod = struct_off % align;
      if (mod != 0)
	struct_off += (align - mod);

      struct_off += size;
    }


  /* account for alignment of entire struct */
  if (max_align < M_X86_ALIGN_CHAR)
    max_align = M_X86_ALIGN_CHAR;	/* align to at least byte boundary */
  mod = struct_off % max_align;	/* enforce max. alignment */
  if (mod != 0)
    struct_off += (max_align - mod);


  return struct_off;
}





int
M_x86_layout_fnvar (List param_list, char **base_macro, int *pcount,
		     int purpose)
{
  M_Param param;
  M_Type type;
  long int *offset;
  int *mode;
  int *reg;
  int *paddr;
  int *su_sreg;
  int *su_ereg;
  int cnt, i, size;

  cnt = List_size (param_list);

  type = alloca (cnt * sizeof (_M_Type));
  offset = alloca (cnt * sizeof (long int));
  mode = alloca (cnt * sizeof (int));
  reg = alloca (cnt * sizeof (int));
  paddr= alloca (cnt * sizeof (int));
  su_sreg = alloca (cnt * sizeof (int));
  su_ereg = alloca (cnt * sizeof (int));

  List_start (param_list);
  i = 0;
  while ((param = (M_Param) List_next (param_list)))
    {
      type[i++] = param->mtype;
    }

  size = M_x86_fnvar_layout (cnt, type, offset, mode, reg, paddr,
			     base_macro, su_sreg, su_ereg, pcount,
			     0, purpose);

  List_start (param_list);
  i = 0;
  while ((param = (M_Param) List_next (param_list)))
    {
      param->offset = offset[i];
      param->mode = mode[i];
      param->reg = reg[i];
      param->paddr = paddr[i];
      param->su_sreg = su_sreg[i];
      param->su_ereg = su_ereg[i];
      i++;
    }

    return size;
}


/*  this variable is used to determine the first time fnvar_layout is 
    called.  It is initialized to 0, then set to one the first time 
    through the function                                             */
static int have_reversed = 0;

/*--------------------------------------------------------------------------*/
int
M_x86_fnvar_layout (int n, _M_Type * type, long int *offset, int *mode,
		    int *reg, int *paddr, char **macro,
		    int *su_sreg, int *su_ereg,
		    int *pcount, int is_st, int purpose)
					/* need to return structure */
{
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

  max_align = M_X86_ALIGN_MAX;
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
	  if (rg < M_X86_MAX_FNVAR_REG)
	    {
	      mode[i] = M_THRU_REGISTER;
	      reg[i] = (rg)++ + M_X86_INT_BASE;
	    }
	  else
	    {
	      mode[i] = M_THRU_MEMORY;
	      reg[i] = -1;
	    }
	  break;

	case M_TYPE_FLOAT:
	  if (rg < M_X86_MAX_FNVAR_REG)
	    {
	      mode[i] = M_THRU_REGISTER;
	      reg[i] = (rg)++ + M_X86_FLT_BASE;
	    }
	  else
	    {
	      mode[i] = M_THRU_MEMORY;
	      reg[i] = -1;
	    }
	  break;

	case M_TYPE_DOUBLE:
	  if (rg < M_X86_MAX_FNVAR_REG)
	    {
	      if (rg == 0 || rg == 2)
		{
		  mode[i] = M_THRU_REGISTER;
		  reg[i] = rg + 1 + M_X86_FLT_BASE;
		  rg += 2;
		}
	      else if (rg == 1)
		{
		  mode[i] = M_THRU_REGISTER;
		  reg[i] = 3 + M_X86_FLT_BASE;
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

      /* The X86 convention is for the FP to contain a pointer to the 
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
	      size = M_X86_SIZE_INT;
	      align = M_X86_ALIGN_INT;
	    }
	}
      if (align >= M_X86_SMALL_STRUCT_MAX && type[i].type != M_TYPE_DOUBLE)
	/* anything larger than a 64-bit structure is passed */
	/* indirectly thru memory                            */
	align = M_X86_ALIGN_INT;
      else if (align < M_X86_ALIGN_INT)
	/* anything smaller that 32-bits is passed as 32-bits */
	align = M_X86_ALIGN_INT;

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
	   (tp == M_TYPE_STRUCT)) && (size <= M_X86_SMALL_STRUCT_MAX))
	{

	  align = M_X86_ALIGN_MAX;	/* must align to a double boundry */

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
	   (tp == M_TYPE_STRUCT)) && (size > M_X86_SMALL_STRUCT_MAX))
	{

	  align = M_X86_ALIGN_MAX;	/* must align to a double boundry */

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
}

/*--------------------------------------------------------------------------*/
int
M_x86_lvar_layout (int n, _M_Type * type, long int *offset, char **base_macro)
{
  int i, max_align, off;
  int size, align, mod, tp;
  /*
   *  the LOCAL section must be max. aligned initially
   */
  max_align = M_X86_ALIGN_MAX;
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
  *base_macro = M_X86_VAR_PTR;
  return off;			/* the total size needed */
}

/*--------------------------------------------------------------------------*/
int
M_x86_no_short_int (void)
{
  return (M_X86_SIZE_SHORT == M_X86_SIZE_INT);
}
/*--------------------------------------------------------------------------*/
void
M_x86_cb_label_name (char *fn, int cb, char *line, int len)
{
  sprintf (line, "cb%d%s", cb, fn);
}
/*--------------------------------------------------------------------------*/
int
M_x86_is_cb_label (char *label, char *fn, int *cb)
{
  return (sscanf (label, "cb%d%s", cb, fn) == 2);
}
/*--------------------------------------------------------------------------*/
void
M_x86_jumptbl_label_name (char *fn, int tbl_id, char *line, int len)
{
  sprintf (line, "%s%s%d", fn, M_JUMPTBL_BASE_NAME, tbl_id);
}
/*--------------------------------------------------------------------------*/
/* Format for x86 is: %sM_JUMPTBL_BASE_NAME%d, where %s is the func name    */
int
M_x86_is_jumptbl_label (char *label, char *fn, int *tbl_id)
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
M_x86_structure_pointer (int purpose)
{
  return M_X86_RET_ST;
}
/*--------------------------------------------------------------------------*/
int
M_x86_return_register (int type, int purpose)
{
  switch (type)
    {
    case M_TYPE_INT:
      return M_X86_RET_I32;
    case M_TYPE_LONG:
      return M_X86_RET_I32;
    case M_TYPE_FLOAT:
      return M_X86_RET_F;
    case M_TYPE_DOUBLE:
      return M_X86_RET_F;
    default:
      return M_X86_RET_I32;
    }
}
/*--------------------------------------------------------------------------*/
/*
char *M_x86_fn_label_name(label)
*/
char *
M_x86_fn_label_name (char *label, int (*is_func) (char *is_func_label))
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
M_x86_fn_name_from_label (char *label)
{
  if (!strncmp (label, "_$fn", 4))
    return (label + 4);
  else
    return (label);
}



/*--------------------------------------------------------------------------*/
int
M_x86_fragile_macro (int macro_value)
{
  switch (M_model)
    {
    case M_386:
    case M_486:
    case M_KRYPTON:
    case M_PENTIUM:
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
      M_assert (0, "M_x86_fragile_macro:  Illegal model specified!");
      return (0);
    }
}
/*--------------------------------------------------------------------------*/
int
M_x86_subroutine_call (int opc)
{
  switch (M_model)
    {
    case M_386:
    case M_486:
    case M_KRYPTON:
    case M_PENTIUM:
      return ((opc == Lop_JSR) || (opc == Lop_JSR_FS));
    default:
      M_assert (0, "M_x86_subroutine_call:  Illegal model specified!");
      return (0);
    }
}
/*--------------------------------------------------------------------------*/
/*
 * Declare code generator specific macro registers to the front end parser.
 */
void
M_define_macros_x86 (STRING_Symbol_Table * sym_tbl)
{
  M_add_symbol (sym_tbl, "fr0", LX86_MAC_FZERO);
  /* 1 if leaf function, 0 if non-leaf */
  M_add_symbol (sym_tbl, "$leaf", LX86_MAC_LEAF);
  /* total alloc requirements */
  M_add_symbol (sym_tbl, "$alloc_size", LX86_MAC_ALLOC);
  /* number of integer and float callee saved registers used */
  M_add_symbol (sym_tbl, "$callee_i_regs", LX86_MAC_CALLEE_I);
  M_add_symbol (sym_tbl, "$callee_f_regs", LX86_MAC_CALLEE_F);
  M_add_symbol (sym_tbl, "$stack", LX86_MAC_STACK);
  M_add_symbol (sym_tbl, "$fpstack", LX86_MAC_FPSTACK);
  M_add_symbol (sym_tbl, "$flags", LX86_MAC_FLAGS);
  M_add_symbol (sym_tbl, "$eax", LX86_MAC_EAX);
  M_add_symbol (sym_tbl, "$ebx", LX86_MAC_EBX);
  M_add_symbol (sym_tbl, "$ecx", LX86_MAC_ECX);
  M_add_symbol (sym_tbl, "$edx", LX86_MAC_EDX);
  M_add_symbol (sym_tbl, "$esi", LX86_MAC_ESI);
  M_add_symbol (sym_tbl, "$edi", LX86_MAC_EDI);
  M_add_symbol (sym_tbl, "$ebp", LX86_MAC_EBP);
  M_add_symbol (sym_tbl, "$esp", LX86_MAC_ESP);
  M_add_symbol (sym_tbl, "$st0", LX86_MAC_ST0);
  M_add_symbol (sym_tbl, "$st1", LX86_MAC_ST1);
  M_add_symbol (sym_tbl, "$st2", LX86_MAC_ST2);
  M_add_symbol (sym_tbl, "$st3", LX86_MAC_ST3);
  M_add_symbol (sym_tbl, "$st4", LX86_MAC_ST4);
  M_add_symbol (sym_tbl, "$st5", LX86_MAC_ST5);
  M_add_symbol (sym_tbl, "$st6", LX86_MAC_ST6);
  M_add_symbol (sym_tbl, "$st7", LX86_MAC_ST7);
}

char *
M_get_macro_name_x86 (int id)
{
  switch (id)
    {
    case LX86_MAC_LEAF:
      return "$leaf";
    case LX86_MAC_ALLOC:
      return "$alloc_size";
    case LX86_MAC_CALLEE_I:
      return "$callee_i_regs";
    case LX86_MAC_CALLEE_F:
      return "$callee_f_regs";
    case LX86_MAC_EAX:
      return "$eax";
    case LX86_MAC_EBX:
      return "$ebx";
    case LX86_MAC_ECX:
      return "$ecx";
    case LX86_MAC_EDX:
      return "$edx";
    case LX86_MAC_ESI:
      return "$esi";
    case LX86_MAC_EDI:
      return "$edi";
    case LX86_MAC_EBP:
      return "$ebp";
    case LX86_MAC_ESP:
      return "$esp";
    case LX86_MAC_STACK:
      return "$stack";
    case LX86_MAC_FLAGS:
      return "$flags";
    case LX86_MAC_FPSTACK:
      return "$fpstack";
    case LX86_MAC_ST0:
      return "$st0";
    case LX86_MAC_ST1:
      return "$st1";
    case LX86_MAC_ST2:
      return "$st2";
    case LX86_MAC_ST3:
      return "$st3";
    case LX86_MAC_ST4:
      return "$st4";
    case LX86_MAC_ST5:
      return "$st5";
    case LX86_MAC_ST6:
      return "$st6";
    case LX86_MAC_ST7:
      return "$st7";

    default:
      return "?";
    }
}

/* Added to support Lmix tools -JCG 11/21/95 */
void
M_define_opcode_name_x86 (STRING_Symbol_Table * sym_tbl)
{
  M_add_symbol (sym_tbl, X86opcode_FPUSH_I, LX86op_FPUSH_I);
  M_add_symbol (sym_tbl, X86opcode_FPOP_I, LX86op_FPOP_I);
  M_add_symbol (sym_tbl, X86opcode_PUSH, LX86op_PUSH);
  M_add_symbol (sym_tbl, X86opcode_POP, LX86op_POP);
  M_add_symbol (sym_tbl, X86opcode_NEGATE, LX86op_NEGATE);
  M_add_symbol (sym_tbl, X86opcode_FXCH, LX86op_FXCH);
  M_add_symbol (sym_tbl, X86opcode_SUB_F, LX86op_SUB_F);
  M_add_symbol (sym_tbl, X86opcode_SUB_F2, LX86op_SUB_F2);
  M_add_symbol (sym_tbl, X86opcode_SUBR_F, LX86op_SUBR_F);
  M_add_symbol (sym_tbl, X86opcode_SUBR_F2, LX86op_SUBR_F2);
  M_add_symbol (sym_tbl, X86opcode_DIV_F, LX86op_DIV_F);
  M_add_symbol (sym_tbl, X86opcode_DIV_F2, LX86op_DIV_F2);
  M_add_symbol (sym_tbl, X86opcode_DIVR_F, LX86op_DIVR_F);
  M_add_symbol (sym_tbl, X86opcode_DIVR_F2, LX86op_DIVR_F2);
  M_add_symbol (sym_tbl, X86opcode_FSTCW, LX86op_FSTCW);
  M_add_symbol (sym_tbl, X86opcode_FLDCW, LX86op_FLDCW);
  M_add_symbol (sym_tbl, X86opcode_LEA, LX86op_LEA);
  M_add_symbol (sym_tbl, X86opcode_TEST, LX86op_TEST);
  M_add_symbol (sym_tbl, X86opcode_INC, LX86op_INC);
  M_add_symbol (sym_tbl, X86opcode_DEC, LX86op_DEC);
  M_add_symbol (sym_tbl, X86opcode_EQ_FMEM, LX86op_EQ_FMEM);
  M_add_symbol (sym_tbl, X86opcode_EQ_F2MEM, LX86op_EQ_F2MEM);
  M_add_symbol (sym_tbl, X86opcode_NE_FMEM, LX86op_NE_FMEM);
  M_add_symbol (sym_tbl, X86opcode_NE_F2MEM, LX86op_NE_F2MEM);
  M_add_symbol (sym_tbl, X86opcode_GT_FMEM, LX86op_GT_FMEM);
  M_add_symbol (sym_tbl, X86opcode_GT_F2MEM, LX86op_GT_F2MEM);
  M_add_symbol (sym_tbl, X86opcode_GE_FMEM, LX86op_GE_FMEM);
  M_add_symbol (sym_tbl, X86opcode_GE_F2MEM, LX86op_GE_F2MEM);
  M_add_symbol (sym_tbl, X86opcode_LT_FMEM, LX86op_LT_FMEM);
  M_add_symbol (sym_tbl, X86opcode_LT_F2MEM, LX86op_LT_F2MEM);
  M_add_symbol (sym_tbl, X86opcode_LE_FMEM, LX86op_LE_FMEM);
  M_add_symbol (sym_tbl, X86opcode_LE_F2MEM, LX86op_LE_F2MEM);
  M_add_symbol (sym_tbl, X86opcode_ADD_FMEM, LX86op_ADD_FMEM);
  M_add_symbol (sym_tbl, X86opcode_ADD_F2MEM, LX86op_ADD_F2MEM);
  M_add_symbol (sym_tbl, X86opcode_SUB_FMEM, LX86op_SUB_FMEM);
  M_add_symbol (sym_tbl, X86opcode_SUB_F2MEM, LX86op_SUB_F2MEM);
  M_add_symbol (sym_tbl, X86opcode_SUBR_FMEM, LX86op_SUBR_FMEM);
  M_add_symbol (sym_tbl, X86opcode_SUBR_F2MEM, LX86op_SUBR_F2MEM);
  M_add_symbol (sym_tbl, X86opcode_MUL_FMEM, LX86op_MUL_FMEM);
  M_add_symbol (sym_tbl, X86opcode_MUL_F2MEM, LX86op_MUL_F2MEM);
  M_add_symbol (sym_tbl, X86opcode_DIV_FMEM, LX86op_DIV_FMEM);
  M_add_symbol (sym_tbl, X86opcode_DIV_F2MEM, LX86op_DIV_F2MEM);
  M_add_symbol (sym_tbl, X86opcode_DIVR_FMEM, LX86op_DIVR_FMEM);
  M_add_symbol (sym_tbl, X86opcode_DIVR_F2MEM, LX86op_DIVR_F2MEM);
  M_add_symbol (sym_tbl, X86opcode_CISC_TO_REG, LX86op_CISC_TO_REG);
  M_add_symbol (sym_tbl, X86opcode_CISC_TO_REG_TEST, LX86op_CISC_TO_REG_TEST);
  M_add_symbol (sym_tbl, X86opcode_CISC_BRANCH, LX86op_CISC_BRANCH);
  M_add_symbol (sym_tbl, X86opcode_CISC_TO_MEM, LX86op_CISC_TO_MEM);
  M_add_symbol (sym_tbl, X86opcode_CISC_TO_MEM_INC, LX86op_CISC_TO_MEM_INC);
  M_add_symbol (sym_tbl, X86opcode_CISC_TO_MEM_DEC, LX86op_CISC_TO_MEM_DEC);
  M_add_symbol (sym_tbl, X86opcode_CISC_TO_MEM_NEGATE,
		LX86op_CISC_TO_MEM_NEGATE);
  M_add_symbol (sym_tbl, X86opcode_CISC_TO_REG_CMP, LX86op_CISC_TO_REG_CMP);
  M_add_symbol (sym_tbl, X86opcode_MOVXL, LX86op_MOVXL);
  M_add_symbol (sym_tbl, X86opcode_INT_LOAD, LX86op_INT_LOAD);
  M_add_symbol (sym_tbl, X86opcode_INT_STORE, LX86op_INT_STORE);
  M_add_symbol (sym_tbl, X86opcode_FLOAT_LOAD, LX86op_FLOAT_LOAD);
  M_add_symbol (sym_tbl, X86opcode_FLOAT_STORE, LX86op_FLOAT_STORE);
  M_add_symbol (sym_tbl, X86opcode_TRACE_JSR, LX86op_TRACE_JSR);
  M_add_symbol (sym_tbl, X86opcode_INT_SUB_WITH_BORROW,
		LX86op_INT_SUB_WITH_BORROW);
  M_add_symbol (sym_tbl, X86opcode_MOVS, LX86op_MOVS);
  M_add_symbol (sym_tbl, X86opcode_SAHF, LX86op_SAHF);
  M_add_symbol (sym_tbl, X86opcode_CDQ, LX86op_CDQ);
  M_add_symbol (sym_tbl, X86opcode_FSTSW, LX86op_FSTSW);
  M_add_symbol (sym_tbl, X86opcode_FLDZ, LX86op_FLDZ);
  M_add_symbol (sym_tbl, X86opcode_FLD1, LX86op_FLD1);
  M_add_symbol (sym_tbl, X86opcode_FLD_ST, LX86op_FLD_ST);
  M_add_symbol (sym_tbl, X86opcode_FSTP, LX86op_FSTP);
  M_add_symbol (sym_tbl, X86opcode_FP2INT_BRANCH, LX86op_FP2INT_BRANCH);
}

char *
M_get_opcode_name_x86 (int id)
{
  switch (id)
    {
    case LX86op_FPUSH_I:
      return (X86opcode_FPUSH_I);
    case LX86op_FPOP_I:
      return (X86opcode_FPOP_I);
    case LX86op_PUSH:
      return (X86opcode_PUSH);
    case LX86op_POP:
      return (X86opcode_POP);
    case LX86op_NEGATE:
      return (X86opcode_NEGATE);
    case LX86op_FXCH:
      return (X86opcode_FXCH);
    case LX86op_SUB_F:
      return (X86opcode_SUB_F);
    case LX86op_SUB_F2:
      return (X86opcode_SUB_F2);
    case LX86op_SUBR_F:
      return (X86opcode_SUBR_F);
    case LX86op_SUBR_F2:
      return (X86opcode_SUBR_F2);
    case LX86op_DIV_F:
      return (X86opcode_DIV_F);
    case LX86op_DIV_F2:
      return (X86opcode_DIV_F2);
    case LX86op_DIVR_F:
      return (X86opcode_DIVR_F);
    case LX86op_DIVR_F2:
      return (X86opcode_DIVR_F2);
    case LX86op_FSTCW:
      return (X86opcode_FSTCW);
    case LX86op_FLDCW:
      return (X86opcode_FLDCW);
    case LX86op_LEA:
      return (X86opcode_LEA);
    case LX86op_TEST:
      return (X86opcode_TEST);
    case LX86op_INC:
      return (X86opcode_INC);
    case LX86op_DEC:
      return (X86opcode_DEC);
    case LX86op_EQ_FMEM:
      return (X86opcode_EQ_FMEM);
    case LX86op_EQ_F2MEM:
      return (X86opcode_EQ_F2MEM);
    case LX86op_NE_FMEM:
      return (X86opcode_NE_FMEM);
    case LX86op_NE_F2MEM:
      return (X86opcode_NE_F2MEM);
    case LX86op_GT_FMEM:
      return (X86opcode_GT_FMEM);
    case LX86op_GT_F2MEM:
      return (X86opcode_GT_F2MEM);
    case LX86op_GE_FMEM:
      return (X86opcode_GE_FMEM);
    case LX86op_GE_F2MEM:
      return (X86opcode_GE_F2MEM);
    case LX86op_LT_FMEM:
      return (X86opcode_LT_FMEM);
    case LX86op_LT_F2MEM:
      return (X86opcode_LT_F2MEM);
    case LX86op_LE_FMEM:
      return (X86opcode_LE_FMEM);
    case LX86op_LE_F2MEM:
      return (X86opcode_LE_F2MEM);
    case LX86op_ADD_FMEM:
      return (X86opcode_ADD_FMEM);
    case LX86op_ADD_F2MEM:
      return (X86opcode_ADD_F2MEM);
    case LX86op_SUB_FMEM:
      return (X86opcode_SUB_FMEM);
    case LX86op_SUB_F2MEM:
      return (X86opcode_SUB_F2MEM);
    case LX86op_SUBR_FMEM:
      return (X86opcode_SUBR_FMEM);
    case LX86op_SUBR_F2MEM:
      return (X86opcode_SUBR_F2MEM);
    case LX86op_MUL_FMEM:
      return (X86opcode_MUL_FMEM);
    case LX86op_MUL_F2MEM:
      return (X86opcode_MUL_F2MEM);
    case LX86op_DIV_FMEM:
      return (X86opcode_DIV_FMEM);
    case LX86op_DIV_F2MEM:
      return (X86opcode_DIV_F2MEM);
    case LX86op_DIVR_FMEM:
      return (X86opcode_DIVR_FMEM);
    case LX86op_DIVR_F2MEM:
      return (X86opcode_DIVR_F2MEM);
    case LX86op_CISC_TO_REG:
      return (X86opcode_CISC_TO_REG);
    case LX86op_CISC_TO_REG_TEST:
      return (X86opcode_CISC_TO_REG_TEST);
    case LX86op_CISC_BRANCH:
      return (X86opcode_CISC_BRANCH);
    case LX86op_CISC_TO_MEM:
      return (X86opcode_CISC_TO_MEM);
    case LX86op_CISC_TO_MEM_INC:
      return (X86opcode_CISC_TO_MEM_INC);
    case LX86op_CISC_TO_MEM_DEC:
      return (X86opcode_CISC_TO_MEM_DEC);
    case LX86op_CISC_TO_MEM_NEGATE:
      return (X86opcode_CISC_TO_MEM_NEGATE);
    case LX86op_CISC_TO_REG_CMP:
      return (X86opcode_CISC_TO_REG_CMP);
    case LX86op_MOVXL:
      return (X86opcode_MOVXL);
    case LX86op_INT_LOAD:
      return (X86opcode_INT_LOAD);
    case LX86op_INT_STORE:
      return (X86opcode_INT_STORE);
    case LX86op_FLOAT_LOAD:
      return (X86opcode_FLOAT_LOAD);
    case LX86op_FLOAT_STORE:
      return (X86opcode_FLOAT_STORE);
    case LX86op_TRACE_JSR:
      return (X86opcode_TRACE_JSR);
    case LX86op_INT_SUB_WITH_BORROW:
      return (X86opcode_INT_SUB_WITH_BORROW);
    case LX86op_MOVS:
      return (X86opcode_MOVS);
    case LX86op_SAHF:
      return (X86opcode_SAHF);
    case LX86op_CDQ:
      return (X86opcode_CDQ);
    case LX86op_FSTSW:
      return (X86opcode_FSTSW);
    case LX86op_FLDZ:
      return (X86opcode_FLDZ);
    case LX86op_FLD1:
      return (X86opcode_FLD1);
    case LX86op_FLD_ST:
      return (X86opcode_FLD_ST);
    case LX86op_FSTP:
      return (X86opcode_FSTP);
    case LX86op_FP2INT_BRANCH:
      return (X86opcode_FP2INT_BRANCH);

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
M_oper_supported_in_arch_x86 (int opc)
{
  switch (opc)
    {

    case Lop_NOR:
    case Lop_NAND:
    case Lop_NXOR:
    case Lop_OR_NOT:
    case Lop_AND_NOT:

    case Lop_MUL_ADD:
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
M_num_oper_required_for_x86 (L_Oper * oper, char *name)
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
M_is_stack_operand_x86 (L_Operand * operand)
{
  if (L_is_macro (operand) &&
      (operand->value.mac == LX86_MAC_ESP ||
       operand->value.mac == L_MAC_IP ||
       operand->value.mac == L_MAC_OP || operand->value.mac == L_MAC_LV))
    return (1);

  return (0);
}

int
M_is_unsafe_macro_x86 (L_Operand * operand)
{
  if ((L_is_macro (operand)) && (operand->value.mac == L_MAC_P1))
    return (1);

  if ((L_is_macro (operand)) && (operand->value.mac == L_MAC_P0))
    return (1);

  return (0);
}


int
M_operand_type_x86 (L_Operand * operand)
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
      if (operand->value.mac == LX86_MAC_FPSTACK)
	{
	  return (MDES_OPERAND_fstk);
	}
      else if (operand->value.mac == LX86_MAC_STACK)
	{
	  return (MDES_OPERAND_istk);
	}
      else if (operand->value.mac == LX86_MAC_FLAGS)
	{
/* fprintf(stderr,"$flag\n"); */
	  return (MDES_OPERAND_flag);
	}
      /* no break on purpose */

    case L_OPERAND_REGISTER:
/*	    if (L_is_ctype_flt(operand)) {
	        return(MDES_OPERAND_f);
	    } else if (L_is_ctype_dbl(operand)) {
	        return(MDES_OPERAND_f2);
	    } else
*/
      return (MDES_OPERAND_REG);


    case L_OPERAND_CB:
    case L_OPERAND_LABEL:
    case L_OPERAND_FLOAT:
    case L_OPERAND_DOUBLE:
    case L_OPERAND_STRING:
      return (MDES_OPERAND_Label);

    default:
      M_assert (0, "M_operand_type_x86: Unknown type");
    }
  return (0);
}

int
M_conflicting_operands_x86 (L_Operand * operand, L_Operand ** conflict_array,
			    int len, int prepass)
{
  if (L_is_ctype_dbl (operand))
    {
      /* Set up conflicting double register */
      conflict_array[0] = L_copy_operand (operand);

      /* Set up conflicting float register */
      conflict_array[1] = L_copy_operand (operand);
      conflict_array[1]->ctype = L_CTYPE_FLOAT;

      conflict_array[2] = L_copy_operand (operand);
      conflict_array[2]->ctype = L_CTYPE_FLOAT;
      conflict_array[2]->value.r = operand->value.r + 1;
      return (3);
    }
  else if (L_is_ctype_flt (operand))
    {
      /* Set up conflicting double register */
      conflict_array[0] = L_copy_operand (operand);
      conflict_array[0]->ctype = L_CTYPE_DOUBLE;
      if (operand->value.r % 2)
	conflict_array[0]->value.r = operand->value.r - 1;
      else
	conflict_array[0]->value.r = operand->value.r;

      /* Set up conflicting float register */
      conflict_array[1] = L_copy_operand (operand);
      return (2);
    }
  else
    {
      conflict_array[0] = L_copy_operand (operand);
      return (1);
    }
}


void
M_get_memory_operands_x86 (int *first, int *number, int proc_opc)
{
  switch (proc_opc)
    {
    case LX86op_EQ_FMEM:
    case LX86op_EQ_F2MEM:
    case LX86op_NE_FMEM:
    case LX86op_NE_F2MEM:
    case LX86op_GT_FMEM:
    case LX86op_GT_F2MEM:
    case LX86op_GE_FMEM:
    case LX86op_GE_F2MEM:
    case LX86op_LT_FMEM:
    case LX86op_LT_F2MEM:
    case LX86op_LE_FMEM:
    case LX86op_LE_F2MEM:
    case LX86op_ADD_FMEM:
    case LX86op_ADD_F2MEM:
    case LX86op_SUB_FMEM:
    case LX86op_SUB_F2MEM:
    case LX86op_SUBR_FMEM:
    case LX86op_SUBR_F2MEM:
    case LX86op_MUL_FMEM:
    case LX86op_MUL_F2MEM:
    case LX86op_DIV_FMEM:
    case LX86op_DIV_F2MEM:
    case LX86op_DIVR_FMEM:
    case LX86op_DIVR_F2MEM:
    case LX86op_FPUSH_I:
    case LX86op_FPOP_I:

      *first = 1;
      *number = 4;
      break;

    case LX86op_MOVXL:
    case LX86op_INT_LOAD:
    case LX86op_INT_STORE:
    case LX86op_FLOAT_LOAD:
    case LX86op_FLOAT_STORE:

    case LX86op_FLDCW:
    case LX86op_FSTCW:
    case LX86op_PUSH:
    case LX86op_POP:

    case LX86op_CISC_TO_REG:
    case LX86op_CISC_TO_REG_TEST:
    case LX86op_CISC_TO_REG_CMP:	/* BS */
    case LX86op_CISC_TO_MEM:
    case LX86op_CISC_TO_MEM_INC:
    case LX86op_CISC_TO_MEM_DEC:
    case LX86op_CISC_TO_MEM_NEGATE:
      *first = 1;
      *number = 4;
      break;

    case LX86op_CISC_BRANCH:
      *first = 0;
      *number = 4;
      break;

    default:
    case Lop_LD_I:
    case Lop_LD_C:
    case Lop_LD_UC:
    case Lop_LD_C2:
    case Lop_LD_UC2:
    case Lop_LD_F:
    case Lop_LD_F2:
    case Lop_ST_I:
    case Lop_ST_C:
    case Lop_ST_C2:
    case Lop_ST_F:
    case Lop_ST_F2:
      *first = 0;
      *number = 2;
    }
}

int
M_memory_access_size_x86 (L_Oper * op)
{

  switch (op->proc_opc)
    {
      /* Proc_opc for sign extended loads of byte or word -DML */
    case LX86op_MOVXL:
      if ((op->opc == Lop_LD_UC2) || (op->opc == Lop_LD_C2))
	return (M_type_size (M_TYPE_SHORT) / M_SIZE_CHAR);
      else if ((op->opc == Lop_LD_UC) || (op->opc == Lop_LD_C))
	return 1;
      else
	I_punt
	  ("M_memory_access_size_x86 (op %i): "
	   "unexpected opc %i for Lop_MOVXL\n",
	   op->id, op->opc);


    case LX86op_FPUSH_I:
    case LX86op_FPOP_I:
    case LX86op_PUSH:
    case LX86op_INT_LOAD:
    case LX86op_INT_STORE:
    case LX86op_CISC_TO_REG:
    case LX86op_CISC_TO_REG_TEST:
    case LX86op_CISC_TO_MEM:
    case LX86op_CISC_TO_MEM_INC:
    case LX86op_CISC_TO_MEM_DEC:
    case LX86op_CISC_TO_REG_CMP:
    case LX86op_CISC_BRANCH:
    case LX86op_CISC_TO_MEM_NEGATE:

/*
	if (attr=L_find_attr(oper->attr, "short_opr") != NULL))
	    if (attr->field[0]->value.i == 1)
		return (M_SIZE_CHAR)
	else	
*/
      return (M_type_size (M_TYPE_INT) / M_SIZE_CHAR);

    case LX86op_EQ_FMEM:
    case LX86op_NE_FMEM:
    case LX86op_GT_FMEM:
    case LX86op_GE_FMEM:
    case LX86op_LT_FMEM:
    case LX86op_LE_FMEM:
    case LX86op_ADD_FMEM:
    case LX86op_SUB_FMEM:
    case LX86op_SUBR_FMEM:
    case LX86op_MUL_FMEM:
    case LX86op_DIV_FMEM:
    case LX86op_DIVR_FMEM:
    case LX86op_FSTCW:
    case LX86op_FLDCW:
      return (M_type_size (M_TYPE_FLOAT) / M_SIZE_CHAR);

    case LX86op_EQ_F2MEM:
    case LX86op_NE_F2MEM:
    case LX86op_GT_F2MEM:
    case LX86op_GE_F2MEM:
    case LX86op_LT_F2MEM:
    case LX86op_LE_F2MEM:
    case LX86op_ADD_F2MEM:
    case LX86op_SUB_F2MEM:
    case LX86op_SUBR_F2MEM:
    case LX86op_MUL_F2MEM:
    case LX86op_DIV_F2MEM:
    case LX86op_DIVR_F2MEM:
      return (M_type_size (M_TYPE_DOUBLE) / M_SIZE_CHAR);

    case LX86op_FLOAT_LOAD:
    case LX86op_FLOAT_STORE:
      switch (op->opc)
	{
	case Lop_LD_F:
	case Lop_ST_F:
	  return (M_type_size (M_TYPE_FLOAT) / M_SIZE_CHAR);
	case Lop_LD_F2:
	case Lop_ST_F2:
	  return (M_type_size (M_TYPE_DOUBLE) / M_SIZE_CHAR);
	default:
	  M_assert (0, "M_memory_access_size_x86: illegal float ld/st");
	}


    default:
      M_assert (0, "M_memory_access_size_x86: illegal proc_opc");
      return (0);
    }
}


int
M_get_data_type_x86 (L_Oper * op)
{

  switch (op->proc_opc)
    {

      /* Proc_opc for sign extended loads of byte or word -DML */
    case LX86op_MOVXL:
      if ((op->opc == Lop_LD_UC2) || (op->opc == Lop_LD_C2))
	return (L_MEMORY_ACCESS_CHAR2);
      else if ((op->opc == Lop_LD_UC) || (op->opc == Lop_LD_C))
	return (L_MEMORY_ACCESS_CHAR);
      else
	I_punt
	  ("M_get_data_type_x86 (op %i): unexpected opc %i for Lop_MOVXL\n",
	   op->id, op->opc);

    case LX86op_FPUSH_I:
    case LX86op_FPOP_I:
    case LX86op_PUSH:
/* added by wfd  */
    case LX86op_INT_LOAD:
    case LX86op_INT_STORE:
    case LX86op_CISC_TO_REG:
    case LX86op_CISC_TO_REG_TEST:
    case LX86op_CISC_BRANCH:
    case LX86op_CISC_TO_MEM:
    case LX86op_CISC_TO_MEM_INC:
    case LX86op_CISC_TO_MEM_DEC:
    case LX86op_CISC_TO_MEM_NEGATE:
    case LX86op_CISC_TO_REG_CMP:
/*3-14-95*/
      return (L_MEMORY_ACCESS_INT);

    case LX86op_EQ_FMEM:
    case LX86op_NE_FMEM:
    case LX86op_GT_FMEM:
    case LX86op_GE_FMEM:
    case LX86op_LT_FMEM:
    case LX86op_LE_FMEM:
    case LX86op_ADD_FMEM:
    case LX86op_SUB_FMEM:
    case LX86op_SUBR_FMEM:
    case LX86op_MUL_FMEM:
    case LX86op_DIV_FMEM:
    case LX86op_DIVR_FMEM:
    case LX86op_FSTCW:
    case LX86op_FLDCW:
/* added by wfd  */
    case LX86op_FLOAT_LOAD:
    case LX86op_FLOAT_STORE:
/*3-14-95*/
      return (L_MEMORY_ACCESS_FLOAT);

    case LX86op_EQ_F2MEM:
    case LX86op_NE_F2MEM:
    case LX86op_GT_F2MEM:
    case LX86op_GE_F2MEM:
    case LX86op_LT_F2MEM:
    case LX86op_LE_F2MEM:
    case LX86op_ADD_F2MEM:
    case LX86op_SUB_F2MEM:
    case LX86op_SUBR_F2MEM:
    case LX86op_MUL_F2MEM:
    case LX86op_DIV_F2MEM:
    case LX86op_DIVR_F2MEM:
      return (L_MEMORY_ACCESS_DOUBLE);

    default:
      M_assert (0, "M_get_data_types_x86: unexpected proc_opc");
    }

  return (L_CTYPE_INT);
}

int
M_num_registers_x86 (int ctype)
{
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
M_is_implicit_memory_op_x86 (L_Oper * oper)
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
    case LX86op_CISC_TO_REG:
    case LX86op_CISC_TO_REG_TEST:
    case LX86op_CISC_TO_MEM:
    case LX86op_CISC_TO_MEM_INC:
    case LX86op_CISC_TO_MEM_DEC:
    case LX86op_CISC_TO_REG_CMP:
    case LX86op_CISC_TO_MEM_NEGATE:

    case LX86op_POP:
      /* Pops used to be annotated by moves - so if the pop is
         not an explicit LOAD / STORE instruction, signal that
         it generates an implicit memory address */
      return 1;

    case LX86op_EQ_FMEM:
    case LX86op_EQ_F2MEM:
    case LX86op_NE_FMEM:
    case LX86op_NE_F2MEM:
    case LX86op_GT_FMEM:
    case LX86op_GT_F2MEM:
    case LX86op_GE_FMEM:
    case LX86op_GE_F2MEM:
    case LX86op_LT_FMEM:
    case LX86op_LT_F2MEM:
    case LX86op_LE_FMEM:
    case LX86op_LE_F2MEM:

    case LX86op_ADD_FMEM:
    case LX86op_ADD_F2MEM:
    case LX86op_SUB_FMEM:
    case LX86op_SUB_F2MEM:
    case LX86op_SUBR_FMEM:
    case LX86op_SUBR_F2MEM:
    case LX86op_MUL_FMEM:
    case LX86op_MUL_F2MEM:
    case LX86op_DIV_FMEM:
    case LX86op_DIVR_FMEM:
    case LX86op_DIV_F2MEM:
    case LX86op_DIVR_F2MEM:
      return 1;

    };


  return 0;
}
