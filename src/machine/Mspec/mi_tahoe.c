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
 *  	  File :	mi_tahoe.c 
 * Description : 	Machine dependent specification.  
 *     Authors :        Dan Connors and Jim Pierce
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
#include  "m_spec.h"
#include  "m_tahoe.h"
#include <library/i_error.h>
#include <library/i_list.h>

/*--------------------------------------------------------------------------*/

/*=========================================================================*/
/*
 *    specific data sizes
 */
/*=========================================================================*/

int M_tahoe_size_long = 64;
int M_tahoe_size_pointer = 64;
int M_tahoe_align_long = 64;
int M_tahoe_align_pointer = 64;

#define M_TAHOE_SIZE_VOID       0
#define M_TAHOE_SIZE_BIT        1
#define M_TAHOE_SIZE_CHAR       8
#define M_TAHOE_SIZE_SHORT      16
#define M_TAHOE_SIZE_INT        32
#define M_TAHOE_SIZE_LONG       M_tahoe_size_long
#define M_TAHOE_SIZE_POINTER    M_tahoe_size_pointer
#define M_TAHOE_SIZE_LLONG      64
#define M_TAHOE_SIZE_FLOAT      32
#define M_TAHOE_SIZE_DOUBLE     64
#define M_TAHOE_SIZE_MAX        64

#define M_TAHOE_ALIGN_VOID      -1
#define M_TAHOE_ALIGN_BIT       1
#define M_TAHOE_ALIGN_CHAR      8
#define M_TAHOE_ALIGN_SHORT     16
#define M_TAHOE_ALIGN_INT       32
#define M_TAHOE_ALIGN_LONG      M_tahoe_align_long
#define M_TAHOE_ALIGN_POINTER   M_tahoe_align_pointer
#define M_TAHOE_ALIGN_LLONG     64
#define M_TAHOE_ALIGN_FLOAT     32
#define M_TAHOE_ALIGN_DOUBLE    64
#define M_TAHOE_ALIGN_MAX       64


int
M_tahoe_type_size (int mtype)
{
  switch (mtype)
    {
    case M_TYPE_VOID:
      return M_TAHOE_SIZE_VOID;
    case M_TYPE_BIT_CHAR:
    case M_TYPE_BIT_SHORT:
    case M_TYPE_BIT_LONG:
    case M_TYPE_BIT_LLONG:
      return M_TAHOE_SIZE_BIT;
    case M_TYPE_CHAR:
      return M_TAHOE_SIZE_CHAR;
    case M_TYPE_SHORT:
      return M_TAHOE_SIZE_SHORT;
    case M_TYPE_INT:
      return M_TAHOE_SIZE_INT;
    case M_TYPE_LONG:
      return M_TAHOE_SIZE_LONG;
    case M_TYPE_LLONG:
      return M_TAHOE_SIZE_LLONG;
    case M_TYPE_FLOAT:
      return M_TAHOE_SIZE_FLOAT;
    case M_TYPE_DOUBLE:
      return M_TAHOE_SIZE_DOUBLE;
    case M_TYPE_POINTER:
      return M_TAHOE_SIZE_POINTER;
    case M_TYPE_BLOCK:
    case M_TYPE_UNION:
    case M_TYPE_STRUCT:
    default:
      return -1;
    }
}


int
M_tahoe_type_align (int mtype)
{
  switch (mtype)
    {
    case M_TYPE_VOID:
      return M_TAHOE_ALIGN_VOID;
    case M_TYPE_BIT_LONG:
      return M_TAHOE_ALIGN_BIT;
    case M_TYPE_BIT_SHORT:
      return M_TAHOE_ALIGN_BIT;
    case M_TYPE_BIT_CHAR:
      return M_TAHOE_ALIGN_BIT;
    case M_TYPE_CHAR:
      return M_TAHOE_ALIGN_CHAR;
    case M_TYPE_SHORT:
      return M_TAHOE_ALIGN_SHORT;
    case M_TYPE_INT:
      return M_TAHOE_ALIGN_INT;
    case M_TYPE_LONG:
      return M_TAHOE_ALIGN_LONG;
    case M_TYPE_LLONG:
      return M_TAHOE_ALIGN_LLONG;
    case M_TYPE_FLOAT:
      return M_TAHOE_ALIGN_FLOAT;
    case M_TYPE_DOUBLE:
      return M_TAHOE_ALIGN_DOUBLE;
    case M_TYPE_POINTER:
      return M_TAHOE_ALIGN_POINTER;
    case M_TYPE_BLOCK:
    case M_TYPE_UNION:
    case M_TYPE_STRUCT:
    default:
      return -1;
    }
}

/*--------------------------------------------------------------------------*/
/*
 *    The following functions are for documentation only.
 *      The function overhead is too great for actual usage.
 */
void
M_tahoe_void (M_Type type)
{
  type->type = M_TYPE_VOID;
  type->unsign = 1;
  type->align = M_TAHOE_ALIGN_VOID;
  type->size = M_TAHOE_SIZE_VOID;
  type->nbits = 0;
}

void
M_tahoe_bit_llong (M_Type type, int n)
{
  type->type = M_TYPE_BIT_LLONG;
  type->unsign = 1;
  type->align = M_TAHOE_ALIGN_BIT;
  type->size = n * M_TAHOE_SIZE_BIT;
  type->nbits = n * M_TAHOE_SIZE_BIT;
  M_assert ((n <= 64),
	    "M_bit_long: do not allow bit field of more than 64 bits");
}

void
M_tahoe_bit_long (M_Type type, int n)
{
  type->type = M_TYPE_BIT_LONG;
  type->unsign = 1;
  type->align = M_TAHOE_ALIGN_BIT;
  type->size = n * M_TAHOE_SIZE_BIT;
  type->nbits = n * M_TAHOE_SIZE_BIT;
  M_assert ((n <= 64),
	    "M_bit_long: do not allow bit field of more than 64 bits");
}

void
M_tahoe_bit_int (M_Type type, int n)
{
  type->type = M_TYPE_BIT_INT;
  type->unsign = 1;
  type->align = M_TAHOE_ALIGN_BIT;
  type->size = n * M_TAHOE_SIZE_BIT;
  type->nbits = n * M_TAHOE_SIZE_BIT;
  M_assert ((n <= 32),
	    "M_bit_int: do not allow bit field of more than 32 bits");
}

void
M_tahoe_bit_short (M_Type type, int n)
{
  type->type = M_TYPE_BIT_SHORT;
  type->unsign = 1;
  type->align = M_TAHOE_ALIGN_BIT;
  type->size = n * M_TAHOE_SIZE_BIT;
  type->nbits = n * M_TAHOE_SIZE_BIT;
  M_assert ((n <= 16),
	    "M_bit_short: do not allow bit field of more than 16 bits");
}


void
M_tahoe_bit_char (M_Type type, int n)
{
  type->type = M_TYPE_BIT_CHAR;
  type->unsign = 1;
  type->align = M_TAHOE_ALIGN_BIT;
  type->size = n * M_TAHOE_SIZE_BIT;
  type->nbits = n * M_TAHOE_SIZE_BIT;
  M_assert ((n <= 8),
	    "M_bit_char: do not allow bit field of more than 8 bits");
}

void
M_tahoe_char (M_Type type, int unsign)
{
  type->type = M_TYPE_CHAR;
  type->unsign = unsign;
  type->align = M_TAHOE_ALIGN_CHAR;
  type->size = M_TAHOE_SIZE_CHAR;
  type->nbits = M_TAHOE_SIZE_CHAR;
}

void
M_tahoe_short (M_Type type, int unsign)
{
  type->type = M_TYPE_SHORT;
  type->unsign = unsign;
  type->align = M_TAHOE_ALIGN_SHORT;
  type->size = M_TAHOE_SIZE_SHORT;
  type->nbits = M_TAHOE_SIZE_SHORT;
}

void
M_tahoe_int (M_Type type, int unsign)
{
  type->type = M_TYPE_INT;
  type->unsign = unsign;
  type->align = M_TAHOE_ALIGN_INT;
  type->size = M_TAHOE_SIZE_INT;
  type->nbits = M_TAHOE_SIZE_INT;
}

void
M_tahoe_llong (M_Type type, int unsign)
{
  type->type = M_TYPE_LLONG;
  type->unsign = unsign;
  type->align = M_TAHOE_ALIGN_LLONG;
  type->size = M_TAHOE_SIZE_LLONG;
  type->nbits = M_TAHOE_SIZE_LLONG;
}

void
M_tahoe_long (M_Type type, int unsign)
{
  type->type = M_TYPE_LONG;
  type->unsign = unsign;
  type->align = M_TAHOE_ALIGN_LONG;
  type->size = M_TAHOE_SIZE_LONG;
  type->nbits = M_TAHOE_SIZE_LONG;
}

void
M_tahoe_float (M_Type type, int unsign)
{
  type->type = M_TYPE_FLOAT;
  type->unsign = unsign;
  type->align = M_TAHOE_ALIGN_FLOAT;
  type->size = M_TAHOE_SIZE_FLOAT;
  type->nbits = M_TAHOE_SIZE_FLOAT;
}

void
M_tahoe_double (M_Type type, int unsign)
{
  type->type = M_TYPE_DOUBLE;
  type->unsign = unsign;
  type->align = M_TAHOE_ALIGN_DOUBLE;
  type->size = M_TAHOE_SIZE_DOUBLE;
  type->nbits = M_TAHOE_SIZE_DOUBLE;
}

void
M_tahoe_pointer (M_Type type)
{
  type->type = M_TYPE_POINTER;
  type->unsign = 1;
  type->align = M_TAHOE_ALIGN_POINTER;
  type->size = M_TAHOE_SIZE_POINTER;
  type->nbits = M_TAHOE_SIZE_POINTER;
}

/*--------------------------------------------------------------------------*/

int
M_tahoe_eval_type (M_Type type, M_Type ntype)
{
  int rtype = type->type;

  switch (rtype)
    {
    case M_TYPE_VOID:
      M_tahoe_void (ntype);
      rtype = -1;
      break;
    case M_TYPE_BIT_LONG:
    case M_TYPE_BIT_CHAR:
    case M_TYPE_CHAR:
    case M_TYPE_SHORT:
    case M_TYPE_INT:
    case M_TYPE_BLOCK:
      /* the starting address of array is used */
      M_tahoe_int (ntype, type->unsign);
      rtype = M_TYPE_INT;
      break;
    case M_TYPE_LONG:
      if (M_TAHOE_SIZE_LONG == 32)
	{
	  M_tahoe_int (ntype, type->unsign);
	  rtype = M_TYPE_INT;
	}
      else if (M_TAHOE_SIZE_LONG == 64)
	{
	  M_tahoe_llong (ntype, type->unsign);
	  rtype = M_TYPE_LLONG;
	}
      else
	{
	  I_punt("M_tahoe_eval_type: "
		 "M_TAHOE_SIZE_LONG not 32 or 64 bits: %d.",
		 M_TAHOE_SIZE_LONG);
	}
      break;
    case M_TYPE_POINTER:
      if (M_TAHOE_SIZE_POINTER == 32)
	{
	  M_tahoe_int (ntype, type->unsign);
	  rtype = M_TYPE_INT;
	}
      else if (M_TAHOE_SIZE_POINTER == 64)
	{
	  M_tahoe_llong (ntype, type->unsign);
	  rtype = M_TYPE_LLONG;
	}
      else
	{
	  I_punt("M_tahoe_eval_type: "
		 "M_TAHOE_SIZE_POINTER not 32 or 64 bits: %d.",
		 M_TAHOE_SIZE_POINTER);
	}
      break;
    case M_TYPE_LLONG:
      M_tahoe_llong (ntype, type->unsign);
      break;
    case M_TYPE_FLOAT:
      M_tahoe_float (ntype, type->unsign);
      break;
    case M_TYPE_DOUBLE:
      M_tahoe_double (ntype, type->unsign);
      break;
    case M_TYPE_UNION:
    case M_TYPE_STRUCT:
      *ntype = *type;
      break;
    default:
      rtype = -1;
    }
  return rtype;
}

int
M_tahoe_eval_type2 (M_Type type, M_Type ntype)
{
  switch (type->type)
    {
    case M_TYPE_VOID:
      M_tahoe_void (ntype);
      return (-1);		/* can not be evaluated */
    case M_TYPE_BIT_CHAR:
    case M_TYPE_CHAR:
      M_tahoe_char (ntype, type->unsign);
      return (M_TYPE_CHAR);
    case M_TYPE_SHORT:
      M_tahoe_short (ntype, type->unsign);
      return (M_TYPE_SHORT);
    case M_TYPE_INT:
    case M_TYPE_BLOCK:
      M_tahoe_int (ntype, type->unsign);
      return (M_TYPE_INT);
    case M_TYPE_BIT_LONG:
    case M_TYPE_LONG:
      M_tahoe_long (ntype, type->unsign);
      return (M_TYPE_LONG);
    case M_TYPE_BIT_LLONG:
    case M_TYPE_LLONG:
      M_tahoe_llong (ntype, type->unsign);
      return (M_TYPE_INT);
    case M_TYPE_POINTER:
      M_tahoe_pointer (ntype);
      return (M_TYPE_POINTER);
    case M_TYPE_FLOAT:
      M_tahoe_float (ntype, type->unsign);
      return (M_TYPE_FLOAT);
    case M_TYPE_DOUBLE:
      M_tahoe_double (ntype, type->unsign);
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
M_tahoe_call_type (M_Type type, M_Type ntype)
{
  switch (type->type)
    {
    case M_TYPE_VOID:
      M_tahoe_void (ntype);
      return (-1);		/* can not be evaluated */
    case M_TYPE_BIT_CHAR:
    case M_TYPE_BIT_LONG:
    case M_TYPE_BIT_LLONG:
    case M_TYPE_CHAR:
    case M_TYPE_SHORT:
    case M_TYPE_INT:
    case M_TYPE_LONG:
    case M_TYPE_LLONG:
    case M_TYPE_POINTER:
    case M_TYPE_BLOCK:
      /* the starting address of array is used */
      M_tahoe_llong (ntype, type->unsign);
      return (M_TYPE_LLONG);
    case M_TYPE_FLOAT:
    case M_TYPE_DOUBLE:
      M_tahoe_double (ntype, type->unsign);
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
M_tahoe_call_type2 (M_Type type, M_Type ntype)
{
  switch (type->type)
    {
    case M_TYPE_VOID:
      M_tahoe_void (ntype);
      return (-1);		/* can not be evaluated */
    case M_TYPE_BIT_CHAR:
    case M_TYPE_CHAR:
      M_tahoe_char (ntype, type->unsign);
      return (M_TYPE_CHAR);
    case M_TYPE_SHORT:
      M_tahoe_short (ntype, type->unsign);
      return (M_TYPE_SHORT);
    case M_TYPE_INT:
    case M_TYPE_BLOCK:
      /* the starting address of array is used */
      M_tahoe_int (ntype, type->unsign);
      return (M_TYPE_INT);
    case M_TYPE_BIT_LONG:
    case M_TYPE_LONG:
      M_tahoe_long (ntype, type->unsign);
      return (M_TYPE_LONG);
    case M_TYPE_BIT_LLONG:
    case M_TYPE_LLONG:
      M_tahoe_llong (ntype, type->unsign);
      return (M_TYPE_LLONG);
    case M_TYPE_POINTER:
      M_tahoe_pointer (ntype);
      return (M_TYPE_POINTER);
    case M_TYPE_FLOAT:
      M_tahoe_float (ntype, type->unsign);
      return (M_TYPE_FLOAT);
    case M_TYPE_DOUBLE:
      M_tahoe_double (ntype, type->unsign);
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
M_tahoe_array_layout (M_Type type, int *offset)
{
  *offset = 0;		/* assume first element is aligned */
  return;
}


int
M_tahoe_array_align (M_Type type)
{
  return type->align;
}


int
M_tahoe_array_size (M_Type type, int dim)
{
  int mod, size, align;

  size = type->size;
  align = type->align;
  if ((mod = size % align))
    size += (align - mod);

  return (size * dim);
}

/*--------------------------------------------------------------------------*/

void
M_tahoe_union_layout (int n, _M_Type * type, int *offset, int *bit_offset)
{
  int i;

  for (i = 0; i < n; i++)
    {			/* assume the union is aligned */
      offset[i] = 0;
      bit_offset[i] = 0;
    }
  return;
}

int
M_tahoe_union_align (int n, _M_Type * type)
{
  int i, max;

  max = 0;
  for (i = 0; i < n; i++)
    {
      int aln = type[i].align;
      if (aln > max)
	max = aln;
    }

  /* minimum alignment is byte boundary */

  if (max < M_TAHOE_ALIGN_CHAR)
    max = M_TAHOE_ALIGN_CHAR;
  
  return max;
}

int
M_tahoe_union_size (int n, _M_Type * type)
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

  /* minimum alignment is byte boundary */

  if (max_align < M_TAHOE_ALIGN_CHAR)
    max_align = M_TAHOE_ALIGN_CHAR;

  /* need to increment to the max. align for future array extension */

  if ((i = max_size % max_align))
    max_size += (max_align - i);
  
  return max_size;
}

/*--------------------------------------------------------------------------*/

int
M_tahoe_layout_order (void)
{
  int endianness;

  switch (M_swarch) {
  case M_IPF_LIN_LP64:
  case M_IPF_WIN_P64:
    endianness = M_LITTLE_ENDIAN;
    break;
  case M_IPF_HPUX_LP64:
  case M_IPF_HPUX_ILP32:
    endianness = M_BIG_ENDIAN;
    break;
  default:
    endianness = -1;
    M_assert (0, "M_tahoe_layout_order: Invalid swarch");
  }
  return endianness;
}


/*
 * struct_layout
 *
 *    INPUTS:   n          - the number of elements in the structure
 *              type       - an array of _M_Type structures for each element
 *                           in the structure
 *
 *    OUTPUTS:  base       - an array in which to store the offset into the
 *                           struct which should be addressed to load the
 *                           element.
 *              bit_offset - an array in which to store the bit offset of the
 *                           element from the base
 */
void
M_tahoe_struct_layout (int n, _M_Type * type, int *base, int *bit_offset)
{
  int i;			/* index for arrays */
  int offset;			/* the current offset into the struct */
  int mod;			/* */
  int size;			/* the size of the current element */
  int align;			/* the aligment requirement of the
				 * current element */
  int word_off;			/* the offset into the current word */
  int bf_unit_size = 0;		/* the unit size of the bitfield
				 * (eg. char, short, long) */
  int bf_unit_off;		/* the offset into current bitfield
				 * unit size */

  int mod_word, mod_type;

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

      if (M_tahoe_layout_order () == M_BIG_ENDIAN)
	{
	  mod_word = offset % M_TAHOE_SIZE_INT;
	  if (type[i].type == M_TYPE_BIT_CHAR)
	    {
	      if ((mod_word + size) > M_TAHOE_SIZE_INT)
		offset += (M_TAHOE_SIZE_INT - mod_word);
	      else
		{
		  mod_type = offset % M_TAHOE_SIZE_CHAR;
		  if ((mod_type + size) > M_TAHOE_SIZE_CHAR)
		    {
		      /*
		       * fprintf(stdout,"Changing bit char to bit short\n");
		       */
		      type[i].type = M_TYPE_BIT_SHORT;
		    }
		}
	    }
	  if (type[i].type == M_TYPE_BIT_SHORT)
	    {
	      if ((mod_word + size) > M_TAHOE_SIZE_INT)
		offset += (M_TAHOE_SIZE_INT - mod_word);
	      else
		{
		  mod_type = offset % M_TAHOE_SIZE_SHORT;
		  if ((mod_type + size) > M_TAHOE_SIZE_SHORT)
		    {
		      /*
		       * fprintf(stdout,"Changing bit short to bit long\n");
		       */
		      type[i].type = M_TYPE_BIT_LONG;
		    }
		}
	    }
	  else if (type[i].type == M_TYPE_BIT_LONG)
	    {
	      if ((mod_word + size) > M_TAHOE_SIZE_INT)
		offset += (M_TAHOE_SIZE_INT - mod_word);
	    }
	  
	  mod = offset % align;	/* align to what the field */
	  if (mod != 0)	/* needs to start from */
	    offset += (align - mod);
	  
	  if (type[i].type == M_TYPE_BIT_CHAR)
	    {
	      int mod = offset % M_TAHOE_SIZE_INT;
	      
	      bit_offset[i] =
		offset - mod + (M_TAHOE_SIZE_INT - mod - size);
	      base[i] = offset & (~(M_TAHOE_SIZE_CHAR - 1));
	    }
	  else if (type[i].type == M_TYPE_BIT_SHORT)
	    {
	      int mod = offset % M_TAHOE_SIZE_INT;

	      bit_offset[i] =
		offset - mod + (M_TAHOE_SIZE_INT - mod - size);
	      base[i] = offset & (~(M_TAHOE_SIZE_SHORT - 1));
	    }
	  else if (type[i].type == M_TYPE_BIT_LONG)
	    {
	      int mod = offset % M_TAHOE_SIZE_INT;
	      
	      bit_offset[i] =
		offset - mod + (M_TAHOE_SIZE_INT - mod - size);
	      base[i] = offset & (~(M_TAHOE_SIZE_LONG - 1));
	    }
	  else
	    {
	      base[i] = offset;
	      bit_offset[i] = 0;
	    }
	  
	  offset += size;	/* allocate space */
	  
	  
	}
      else if (M_tahoe_layout_order () == M_LITTLE_ENDIAN)
	{
	  
	  /*
	   *  need to treat bit fields specially:
	   *      - keep them in a word when possible
	   *      - increase the unit size if necessary
	   */
	  if (type[i].type == M_TYPE_BIT_CHAR ||
	      type[i].type == M_TYPE_BIT_SHORT ||
	      type[i].type == M_TYPE_BIT_LONG)
	    {
	      word_off = offset % M_TAHOE_SIZE_INT;
	      
	      /* if element doesn't fit in word,
	       * then its offset is beginning of the next word */
	      if ((word_off + size) > M_TAHOE_SIZE_INT)
		offset += (M_TAHOE_SIZE_INT - word_off);
	      
	      /* if element doesn't fit in aligned unit size, 
		 then successively
		 * increase unit size until it does */
	      if (type[i].type == M_TYPE_BIT_CHAR)
		{
		  bf_unit_size = M_TAHOE_SIZE_CHAR;
		  bf_unit_off = offset % bf_unit_size;
		  
		  /* use for standard bitfields */
		  if ((bf_unit_off + size) > M_TAHOE_SIZE_CHAR)
		    {
		      type[i].type = M_TYPE_BIT_SHORT;
		      bf_unit_size = M_TAHOE_SIZE_SHORT;
		    }
		}
	      if (type[i].type == M_TYPE_BIT_SHORT)
		{
		  bf_unit_size = M_TAHOE_SIZE_SHORT;
		  bf_unit_off = offset % bf_unit_size;
		  
		  /* use for standard bitfields */
		  if ((bf_unit_off + size) > M_TAHOE_SIZE_SHORT)
		    {
		      type[i].type = M_TYPE_BIT_LONG;
		      bf_unit_size = M_TAHOE_SIZE_LONG;
		    }
		  
		}
	      if (type[i].type == M_TYPE_BIT_LONG)
		{
		  bf_unit_size = M_TAHOE_SIZE_LONG;
		  bf_unit_off = offset % bf_unit_size;
		  if ((bf_unit_off + size) > M_TAHOE_SIZE_LONG)
		    {
		      /* problem:  can't increase size any more */
		      M_assert (0,
				"M_tahoe_struct_layout:  bitfield "
				"element does not fit in one word");
		    }
		}
	      
	    }		/* end special treatment of bitfields */
	  
	  
	  /* adjust offset to required alignment */
	  mod = offset % align;
	  if (mod != 0)
	    offset += (align - mod);
	  

	  /* calculate the base (the offset into the struct which should
	   * be addressed to load the element) */
	  /* calculate the bit_offset (the bit offset from the base of the
	   * element) */
	  switch (type[i].type)
	    {
	    case M_TYPE_BIT_CHAR:
	    case M_TYPE_BIT_SHORT:
	    case M_TYPE_BIT_LONG:
	      base[i] = offset & (~(bf_unit_size - 1));   /*mask off 
							    low bits */
	      bit_offset[i] = offset - base[i];
#if 0
	      fprintf (stderr,
		       "bitfields:  type = %d, base = %d, bit_offset = %d"
		       "\n", type[i].type, base[i], bit_offset[i]);
#endif
	      break;
	    default:
	      bit_offset[i] = 0;
	      base[i] = offset;
#if 0
	      fprintf (stderr, "type = %d, base = %d, bit_offset = %d\n",
		       type[i].type, base[i], bit_offset[i]);
#endif
	    }
	  /* adjust offset to allocate space for current element */
	  offset += size;
	}
      else
	{
	  M_assert (0, "M_tahoe_struct_layout: endian ordering not set");
	}
      
    }
  return;
}

int
M_tahoe_struct_align (int n, _M_Type * type)
{
  int i, max;

  max = 0;
  for (i = 0; i < n; i++)
    {
      int aln = type[i].align;
      if (aln > max)
	max = aln;
    }

  /* minimum alignment is byte boundary */
  if (max < M_TAHOE_ALIGN_CHAR)
    max = M_TAHOE_ALIGN_CHAR;
  return max;
}


int
M_tahoe_struct_size (int n, _M_Type * type, int struct_align)
{

  int i, offset;
  int mod, size, align, max_align, mod_word;
  int word_off;

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
      
      /* Endian Choice: Big or small */
      if (M_tahoe_layout_order () == M_BIG_ENDIAN)
	{
	  
	  mod_word = offset % M_TAHOE_SIZE_INT;
	  if (type[i].type == M_TYPE_BIT_CHAR)
	    {
	      if ((mod_word + size) > M_TAHOE_SIZE_INT)
		offset += (M_TAHOE_SIZE_INT - mod_word);
	      if (M_TAHOE_ALIGN_CHAR > max_align)
		max_align = M_TAHOE_ALIGN_CHAR;
	    }
	  else if (type[i].type == M_TYPE_BIT_SHORT)
	    {
	      if ((mod_word + size) > M_TAHOE_SIZE_INT)
		offset += (M_TAHOE_SIZE_INT - mod_word);
	      if (M_TAHOE_ALIGN_SHORT > max_align)
		max_align = M_TAHOE_ALIGN_SHORT;
	    }
	  else if (type[i].type == M_TYPE_BIT_LONG)
	    {
	      if ((mod_word + size) > M_TAHOE_SIZE_INT)
		offset += (M_TAHOE_SIZE_INT - mod_word);
	      if (M_TAHOE_ALIGN_LONG > max_align)
		max_align = M_TAHOE_ALIGN_LONG;
	    }
	  
	}
      else if (M_tahoe_layout_order () == M_LITTLE_ENDIAN)
	{
	  
	  if (type[i].type == M_TYPE_BIT_CHAR ||
	      type[i].type == M_TYPE_BIT_SHORT ||
	      type[i].type == M_TYPE_BIT_LONG)
	    {
	      word_off = offset % M_TAHOE_SIZE_INT;
	      
	      if ((word_off + size) > M_TAHOE_SIZE_INT)
		{
		  /* if element doesn't fit in word,
		   * then its offset is beginning of the next word */
		  offset += (M_TAHOE_SIZE_INT - word_off);
		}
	      
	    }
	  
	}
      else
	{
	  M_assert (0, "M_tahoe_struct_size: endian ordering not set");
	}
      
      mod = offset % align;	/* align to what the field */
      if (mod != 0)		/* needs to start from */
	offset += (align - mod);
      
      offset += size;
	}
  
  /* minimum alignment is byte boundary */
  if (max_align < M_TAHOE_ALIGN_CHAR)
    max_align = M_TAHOE_ALIGN_CHAR;
  /* enforce max. alignment */
  mod = offset % max_align;
  if (mod != 0)
    offset += (max_align - mod);
  return offset;
}

/*--------------------------------------------------------------------------*/

int
M_tahoe_no_short_int (void)
{
  return (M_TAHOE_SIZE_SHORT == M_TAHOE_SIZE_INT);
}

/*--------------------------------------------------------------------------*/

void
M_tahoe_cb_label_name (char *fn, int cb, char *line, int len)
{
  sprintf (line, "cb%d%s", cb, fn);
}

/*--------------------------------------------------------------------------*/

int
M_tahoe_is_cb_label (char *label, char *fn, int *cb)
{
  return (sscanf (label, "cb%d%s", cb, fn) == 2);
}

/*--------------------------------------------------------------------------*/

void
M_tahoe_jumptbl_label_name (char *fn, int tbl_id, char *line, int len)
{
  sprintf (line, "%s%s%d", fn, M_JUMPTBL_BASE_NAME, tbl_id);
  return;
}

/*--------------------------------------------------------------------------*/

int
M_tahoe_is_jumptbl_label (char *label, char *fn, int *tbl_id)
{
  /* Format for v1.0 is: %sM_JUMPTBL_BASE_NAME%d, where %s is the func 
   * name 
   */
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
M_tahoe_structure_pointer (int purpose)
{
  return M_TAHOE_RET_INT_BASE;
}

/*--------------------------------------------------------------------------*/

int
M_tahoe_return_register (int type, int purpose)
{
  switch (type)
    {
    case M_TYPE_INT:
      return M_TAHOE_RET_INT_BASE;
    case M_TYPE_LONG:
      return M_TAHOE_RET_INT_BASE;
    case M_TYPE_LLONG:
      return M_TAHOE_RET_INT_BASE;
    case M_TYPE_FLOAT:
      return M_TAHOE_RET_FLT_BASE;
    case M_TYPE_DOUBLE:
      return M_TAHOE_RET_FLT_BASE;
    default:
      return M_TAHOE_RET_INT_BASE;
    }
}

/*--------------------------------------------------------------------------*/

char *
M_tahoe_fn_label_name (char *label, int (*is_func) (char *is_func_label))
{
  static char fn_label[64];
  
  if ((*is_func) (label))
    {
      sprintf (fn_label, "$fn_%s", label);
      return (fn_label);
    }
  else
    {
      return (label);
    }
}

char *
M_tahoe_fn_name_from_label (char *label)
{
  if (!strncmp (label, "_$fn", 4))
    return (label + 4);
  else
    return (label);
}

/*--------------------------------------------------------------------------*/

void
M_set_model_tahoe (char *model_name)
{
  if (!strcasecmp (model_name, "itanium"))
    {
      M_model = M_IPF_ITANIUM;                  /* 6/2001 */
    }
  else if (!strcasecmp (model_name, "mckinley"))
    {
      M_model = M_IPF_MCKINLEY;                 /* ?/2002 */
    }
  else if (!strcasecmp (model_name, "madison"))
    {
      M_model = M_IPF_MADISON;                  /* ?/2003 */
    }
  else if (!strcasecmp (model_name, "deerfield"))
    {
      M_model = M_IPF_DEERFIELD;                /* ?/2004 */
    }
  else if (!strcasecmp (model_name, "mendocito"))
    {
      M_model = M_IPF_MENDOCITO;                /* ?/2005 */
    }
  else if (!strcasecmp (model_name, "chivano"))
    {
      M_model = M_IPF_CHIVANO;                  /* ?/2006 */
    }
  else
    {
      fprintf (stderr, "illegal model_name : %s\n", model_name);
      fprintf (stderr, "Use one of the following: ITANIUM, MCKINLEY\n");
      M_assert (0, "M_set_model_tahoe: illegal model name specified");
    }

  M_use_layout_database = 1;
  return;
}

void
M_set_swarch_tahoe (char *model_name)
{
  if (!strcasecmp (model_name, "linux"))
    {
      M_swarch = M_IPF_LIN_LP64;
      M_tahoe_size_long = 64;
      M_tahoe_size_pointer = 64;
      M_tahoe_align_long = 64;
      M_tahoe_align_pointer = 64;
    }
  else if (!strcasecmp (model_name, "hpux64"))
    {
      M_swarch = M_IPF_HPUX_LP64;
      M_tahoe_size_long = 64;
      M_tahoe_size_pointer = 64;
      M_tahoe_align_long = 64;
      M_tahoe_align_pointer = 64;
    }
  else if (!strcasecmp (model_name, "hpux32"))
    {
      M_swarch = M_IPF_HPUX_ILP32;
      M_tahoe_size_long = 32;
      M_tahoe_size_pointer = 32;
      M_tahoe_align_long = 32;
      M_tahoe_align_pointer = 32;
    }
  else if (!strcasecmp (model_name, "win64"))
    {
      M_swarch = M_IPF_WIN_P64;
      M_tahoe_size_long = 32;
      M_tahoe_size_pointer = 64;
      M_tahoe_align_long = 32;
      M_tahoe_align_pointer = 64;
    }
  else
    {
      fprintf (stderr, "illegal swarch: %s\n", model_name);
      fprintf (stderr, "Use one of the following: LINUX, WIN64, HPUX64, HPUX32\n");
      M_assert (0, "M_set_model_tahoe: illegal model name specified");
    }
  return;
}

/*--------------------------------------------------------------------------*/

/* align_bit = bits, offset = bytes */
static int
M_align_offset(int offset, int align_bit)
{
  int mod;
  int align = align_bit / 8;

  if ((align_bit % 8) != 0)
    I_punt("M_align_offset: alignment not multiple of 8 = %d\n",
	   align_bit);

  mod = offset % align;
  if (mod != 0)
    offset += (align - mod);  

  return offset;
}


/*
 * M_tahoe_layout_fnvar
 * ----------------------------------------------------------------------
 * Lay out function incoming / outgoing parameters according to 
 * IA-64 Software Conventions and Runtime Architecture Guide
 * Intel Doc. No. 245256-001, August, 1999.
 *
 * EXCEPTION: Homogeneous floating-point aggregates (HFA) are
 *            handled as an ordinary aggregate type.
 */
int
M_tahoe_layout_fnvar (List param_list, char **macro, int *pcount, int purpose)
{
  M_Param param;
  int current_offset;
  int int_reg_base;
  int param_position, fp_param_position;
  int extra_space;

  /* Ltahoe stack frame, parameter portion.
   *
   *              |   ...    |
   *              +----------+
   *              |    P9    |
   *              +----------+ <-- IP + 24
   *              |    P8    |
   *              +----------+ <-- IP + 16
   *              | SCRATCH1 |
   *              +----------+ <-- IP + 8
   *  CALLER      | SCRATCH0 |
   *  ------      +----------+ <-- IP (PSP)
   *  CALLEE      |   ...    |
   */

  switch (purpose)
    {
    case M_GET_FNVAR:
      *macro = "$IP";
      int_reg_base = M_TAHOE_IN_INT_BASE;
      break;
    case M_PUT_FNVAR:
      *macro = "$OP";
      int_reg_base = M_TAHOE_OUT_INT_BASE;
      break;
    case M_RET_FNVAR:
      M_assert (0, "M_layout_fnvar: M_RET_FNVAR unsupported");
      *macro = "$P16";
      int_reg_base = M_TAHOE_RET_INT_BASE;
      break;
    case M_DONT_CARE_FNVAR:
    default:
      M_assert (0, "M_layout_fnvar: unknown purpose");
      return (-1);
    }

  param_position = 0;
  fp_param_position = 0;
  current_offset = SCRATCH_SPACE_SIZE;

  List_start (param_list);
  while ((param = (M_Param) List_next (param_list)))
    {
      int tp, size, align, is_hfa;
      tp = param->mtype.type;
      size = param->mtype.size;
      align = param->mtype.align;

      /* initialize */

      param->offset = param->paddr = param->su_sreg = param->su_ereg = 0;
      
      switch (tp)
	{
	case M_TYPE_CHAR:
	case M_TYPE_SHORT:
	case M_TYPE_INT:
	case M_TYPE_LONG:
	case M_TYPE_POINTER:
	case M_TYPE_LLONG:
	  if (param_position < M_TAHOE_MAX_FNVAR_INT_REG)
	    {
	      param->mode = M_THRU_REGISTER;
	      param->reg = param_position + int_reg_base;
	      param->offset = 0;
	      param_position++;
	    }
	  else
	    {
	      param->mode = M_THRU_MEMORY;
	      param->reg = -1;
	      current_offset = M_align_offset(current_offset, align);
	      param->offset = current_offset;
	      current_offset += (M_TAHOE_SIZE_LLONG / 8);
	    }
	  break;

	case M_TYPE_FLOAT:
	case M_TYPE_DOUBLE:
	  if (param_position < M_TAHOE_MAX_FNVAR_FLT_REG)
	    {
	      param->mode = M_THRU_REGISTER;
	      param->reg = fp_param_position + M_TAHOE_FLT_BASE;
	      param->offset = 0;
	      fp_param_position++;
	      param_position++;
	    }
	  else
	    {
	      /* Verify alignment */
	      current_offset = M_align_offset(current_offset, align);
	      
	      param->mode = M_THRU_MEMORY;
	      param->reg = -1;
	      param->offset = current_offset;
	      current_offset += (M_TAHOE_SIZE_DOUBLE / 8);
	    }
	  break;

	case M_TYPE_UNION:
	case M_TYPE_STRUCT:

	  is_hfa = param->mtype.flags & M_TYFL_HFA;

	  if (is_hfa)
	    I_warn ("Setting up HFA fnvar");

	  /*printf("size %d (%d bytes) align %d\n",size,size/8,align); */

	  if (param_position < M_TAHOE_MAX_FNVAR_INT_REG)
	    {
	      if (align == 128)
		{
		  /* 128 bit alignment means use next even register */
		  if (param_position & 0x1)
		    param_position++;
		}
	      else
		{
		  /* use next register */
		  M_assert ((align <= 64),
			    "M_tahoe_layout_fnvar: Unexpected alignment\n");
		}
	    }

	  if (param_position < M_TAHOE_MAX_FNVAR_INT_REG)
	    {
	      int regs_needed;

	      param->mode = M_THRU_REGISTER;
	      param->reg = -1;
	      param->su_sreg = param_position + int_reg_base;
	      param->paddr = param_position;

	      regs_needed = (size / M_TAHOE_SIZE_LLONG);
	      if (size % M_TAHOE_SIZE_LLONG)
		regs_needed++;

	      if (regs_needed + param_position < M_TAHOE_MAX_FNVAR_INT_REG)
		{
		  /* Struct fits entirely into param regs */
		  param_position += regs_needed;
		  param->offset = -1;
		  param->su_ereg = param_position + int_reg_base - 1;
		}
	      else
		{
		  /* Struct fills remaining regs and overlaps into memory */
		  regs_needed = regs_needed - (M_TAHOE_MAX_FNVAR_INT_REG
					       - param_position);
		  param_position = M_TAHOE_MAX_FNVAR_INT_REG;
		  extra_space = M_TAHOE_SIZE_LLONG * regs_needed;
		  param->offset = current_offset;
		  param->su_ereg = param_position + int_reg_base - 1;
		  current_offset += extra_space;
		}

	      if (is_hfa)
		{
		  ;
		}
	    }
	  else
	    {
	      param->mode = M_THRU_MEMORY;
	      param->reg = -1;
	      param->su_sreg = -1;
	      param->su_ereg = -1;
	      param->paddr = -1;

	      /* Verify alignment */
	      current_offset = M_align_offset(current_offset, align);
	      
	      param->offset = current_offset;
	      current_offset += size / 8;
	    }
	  break;

	default:
	  M_assert (0, "M_layout_fnvar: argument is not promoted");
	}
    }

  {
    int marker = M_TAHOE_MAX_FNVAR_INT_REG - param_position;

    List_start (param_list);
    while ((param = (M_Param) List_next (param_list)))
      {
	if ((param->paddr >= 0) && 
	    ((param->mtype.type == M_TYPE_UNION) ||
	     (param->mtype.type == M_TYPE_STRUCT)))
	  param->paddr += marker;
      }
  }

  *pcount = param_position;
  /* size necessary for parameters */
  return (current_offset - SCRATCH_SPACE_SIZE);	
}

/*
 * M_tahoe_fnvar_layout
 * ----------------------------------------------------------------------
 * Lay out function incoming / outgoing parameters according to 
 * IA-64 Software Conventions and Runtime Architecture Guide
 * Intel Doc. No. 245256-001, August, 1999.
 *
 * EXCEPTION: Homogeneous floating-point aggregates (HFA) are
 *            handled as an ordinary aggregate type.
 */

int
M_tahoe_fnvar_layout (int n, _M_Type * type, long int *offset, int *mode,
		      int *reg, int *paddr, char **macro,
		      int *su_sreg, int *su_ereg,
		      int *pcount, int is_st, int purpose)
{
  int i, current_offset;
  int int_reg_base;
  int param_position, fp_param_position;
  int extra_space;
  int endoffset[128];

  /* Ltahoe stack frame, parameter portion.
   *
   *              |   ...    |
   *              +----------+
   *              |    P9    |
   *              +----------+ <-- IP + 24
   *              |    P8    |
   *              +----------+ <-- IP + 16
   *              | SCRATCH1 |
   *              +----------+ <-- IP + 8
   *  CALLER      | SCRATCH0 |
   *  ------      +----------+ <-- IP (PSP)
   *  CALLEE      |   ...    |
   */

  switch (purpose)
    {
    case M_GET_FNVAR:
      *macro = "$IP";
      int_reg_base = M_TAHOE_IN_INT_BASE;
      break;
    case M_PUT_FNVAR:
      *macro = "$OP";
      int_reg_base = M_TAHOE_OUT_INT_BASE;
      break;
    case M_DONT_CARE_FNVAR:
    default:
      M_assert (0, "M_fnvar_layout: unknown purpose");
      return (-1);
    }

  param_position = 0;
  fp_param_position = 0;
  current_offset = SCRATCH_SPACE_SIZE;

  for (i = 0; i < n; i++)
    {
      int tp, size, align, is_hfa;
      tp = type[i].type;
      size = type[i].size;
      align = type[i].align;
      
      switch (tp)
	{
	case M_TYPE_CHAR:
	case M_TYPE_SHORT:
	case M_TYPE_INT:
	case M_TYPE_LONG:
	case M_TYPE_POINTER:
	case M_TYPE_LLONG:
	  if (param_position < M_TAHOE_MAX_FNVAR_INT_REG)
	    {
	      mode[i] = M_THRU_REGISTER;
	      reg[i] = param_position + int_reg_base;
	      offset[i] = 0;
	      param_position++;
	    }
	  else
	    {
	      mode[i] = M_THRU_MEMORY;
	      reg[i] = -1;
	      current_offset = M_align_offset(current_offset, align);
	      offset[i] = current_offset;
	      current_offset += (M_TAHOE_SIZE_LLONG / 8);
	    }
	  break;

	case M_TYPE_FLOAT:
	case M_TYPE_DOUBLE:
	  if (param_position < M_TAHOE_MAX_FNVAR_FLT_REG)
	    {
	      mode[i] = M_THRU_REGISTER;
	      reg[i] = fp_param_position + M_TAHOE_FLT_BASE;
	      offset[i] = 0;
	      fp_param_position++;
	      param_position++;
	    }
	  else
	    {
	      /* Verify alignment */
	      current_offset = M_align_offset(current_offset, align);
	      
	      mode[i] = M_THRU_MEMORY;
	      reg[i] = -1;
	      offset[i] = current_offset;
	      current_offset += (M_TAHOE_SIZE_DOUBLE / 8);
	    }
	  break;

	case M_TYPE_UNION:
	case M_TYPE_STRUCT:

	  is_hfa = type[i].flags & M_TYFL_HFA;

	  if (is_hfa)
	    I_warn ("Setting up HFA fnvar");

	  endoffset[i] = -1;
	  /*printf("size %d (%d bytes) align %d\n",size,size/8,align); */

	  if (param_position < M_TAHOE_MAX_FNVAR_INT_REG)
	    {
	      if (align == 128)
		{
		  /* 128 bit alignment means use next even register */
		  if (param_position & 0x1)
		    param_position++;
		}
	      else
		{
		  /* use next register */
		  M_assert ((align <= 64),
			    "M_tahoe_fnvar_layout: Unexpected alignment\n");
		}
	    }

	  if (param_position < M_TAHOE_MAX_FNVAR_INT_REG)
	    {
	      int regs_needed;

	      mode[i] = M_THRU_REGISTER;
	      reg[i] = -1;
	      su_sreg[i] = param_position + int_reg_base;
	      paddr[i] = param_position;

	      regs_needed = (size / M_TAHOE_SIZE_LLONG);
	      if (size % M_TAHOE_SIZE_LLONG)
		regs_needed++;

	      if (regs_needed + param_position < M_TAHOE_MAX_FNVAR_INT_REG)
		{
		  /* Struct fits entirely into param regs */
		  param_position += regs_needed;
		  offset[i] = -1;
		  su_ereg[i] = param_position + int_reg_base - 1;
		}
	      else
		{
		  /* Struct fills remaining regs and overlaps into memory */
		  regs_needed = regs_needed - (M_TAHOE_MAX_FNVAR_INT_REG
					       - param_position);
		  param_position = M_TAHOE_MAX_FNVAR_INT_REG;
		  extra_space = M_TAHOE_SIZE_LLONG * regs_needed;
		  offset[i] = current_offset;
		  su_ereg[i] = param_position + int_reg_base - 1;
		  current_offset += extra_space;
		  endoffset[i] = current_offset;
		}

	      if (is_hfa)
		{
		  ;
		}
	    }
	  else
	    {
	      /* printf("M_tahoe_fnvar_layout: Struct/Union "
		        "through memory\n"); */
	      mode[i] = M_THRU_MEMORY;
	      reg[i] = -1;
	      su_sreg[i] = -1;
	      su_ereg[i] = -1;
	      paddr[i] = -1;

	      /* Verify alignment */
	      current_offset = M_align_offset(current_offset, align);
	      
	      offset[i] = current_offset;
	      current_offset += size / 8;
	      endoffset[i] = current_offset;
	    }
	  break;

	default:
	  M_assert (0, "M_fnvar_layout: argument is not promoted");
	}
    }

  {
    int marker;

    marker = M_TAHOE_MAX_FNVAR_INT_REG - param_position;
    for (i = (n - 1); i >= 0; i--)
      {
	if ((type[i].type == M_TYPE_UNION || 
	     type[i].type == M_TYPE_STRUCT) &&
	    paddr[i] >= 0)
	  {
	    paddr[i] = paddr[i] + marker;
	  }
      }
  }

  *pcount = param_position;
  /* size necessary for parameters */
  return (current_offset - SCRATCH_SPACE_SIZE);	
}


int
M_tahoe_layout_retvar (M_Param param, int purpose)
{
  int tp;

  tp = param->mtype.type;

  /* initialize */
  
  param->offset = param->paddr = param->su_sreg = param->su_ereg = 0;
      
  switch (tp)
    {
    case M_TYPE_CHAR:
    case M_TYPE_SHORT:
    case M_TYPE_INT:
    case M_TYPE_LONG:
    case M_TYPE_POINTER:
    case M_TYPE_LLONG:
    case M_TYPE_FLOAT:
    case M_TYPE_DOUBLE:
      param->mode = M_THRU_REGISTER;
      param->reg = M_tahoe_return_register (tp, 0);
      param->offset = 0;
      break;

    case M_TYPE_UNION:
    case M_TYPE_STRUCT:
      {
	int is_hfa = param->mtype.flags & M_TYFL_HFA,
	  size = param->mtype.size;

	if (is_hfa)
	  I_warn ("Setting up HFA fn return");

	if (size <= 256)
	  {
	    int regs_needed = (size / M_TAHOE_SIZE_LLONG);

	    if (size % M_TAHOE_SIZE_LLONG)
	      regs_needed++;

	    param->mode = M_THRU_REGISTER;
	    param->reg = -1;
	    param->su_sreg = M_TAHOE_RET_INT_BASE;
	    param->su_ereg = M_TAHOE_RET_INT_BASE + regs_needed - 1;
	    param->paddr = 0;
	    param->offset = -1;
	  }
	else
	  {
	    param->mode = M_INDIRECT_THRU_REGISTER;
	    param->reg = M_TAHOE_RET_INT_BASE;
	    param->su_sreg = -1;
	    param->su_ereg = -1;
	    param->paddr = -1;
	    param->offset = 0;
	  }
      }
      break;

    default:
      M_assert (0, "M_layout_fnvar: argument is not promoted");
    }

  return (param->mode == M_INDIRECT_THRU_REGISTER);
}


/* local space size will be a multiple of 16 so that the size won't
 * have to be changed in phase2.  Phase1 vararg needs to know the
 * exact size of the local variable space. */

int
M_tahoe_fnvar_to_lvar (_M_Type type, long int *offset,
		       char **base_macro, int local_space)
{
  int current_offset, total_local_space;
  int size, align, mod;

  total_local_space = 0;
  current_offset = local_space;

  /* First local variable cannot have zero offset else
   * wouldn't be able to increase offset in Ltahoe-phase2 */
  if (current_offset == 0)
    {
      current_offset = type.align / 8;
      total_local_space += type.align / 8;
    }

  *base_macro = "$LV";

  switch (type.type)
    {
    case M_TYPE_CHAR:
    case M_TYPE_SHORT:
    case M_TYPE_INT:
    case M_TYPE_LONG:
    case M_TYPE_LLONG:
    case M_TYPE_POINTER:
    case M_TYPE_FLOAT:
    case M_TYPE_DOUBLE:
      size = type.size / 8;
      align = type.align / 8;
      mod = current_offset % align;
      if (mod != 0)
	current_offset += (align - mod);
      *offset = current_offset;
      total_local_space = current_offset + size;

#if 0
      fprintf (stderr,
	       "MCM PARM (bytes): s %d, a %d, m %d, l %d, co %d, t %d\n",
	       size, align, mod, local_space, *offset, total_local_space);
#endif

      break;

    default:
      M_assert (0, "M_tahoe_fnvar_to_lvar: Unrecognized fnvar type");
    }

  return ADDR_ALIGN (total_local_space, 16);
}


/* local space size will be a multiple of 16 so that the size won't
 * have to be changed in phase2.  Phase1 vararg needs to know the
 * exact size of the local variable space. */

/*--------------------------------------------------------------------------*/
int
M_tahoe_lvar_layout (int n, _M_Type * type, long int *offset,
		     char **base_macro)
{
  int i, off;
  int size, align, mod, tp;

  /*
   * the LOCAL section must be max. aligned initially
   */

  /* Note:  JEP 12/20/95

   * The first local variable cannot have a zero offset from the beginning
   * of the local variables.  This is because the true offset from sp is
   * adjusted in O_annotate of phase2 but the non-zero offset stack offset
   * address is converted to an add and zero offset in phase1.
   * Ex.  8($LV) -> add $r1 = 4,$LV, 0($r1) 
   * If the initial offset is 0, the add instruction is not created and a 
   * later offset adjustment cannot be made. */

  if (n > 0)
    off = type[0].align / 8;	/* first local can't have zero offset */
  else
    off = 0;

  for (i = 0; i < n; i++)
    {
      tp = type[i].type;
      if (tp == M_TYPE_BIT_LONG ||
	  tp == M_TYPE_BIT_INT ||
	  tp == M_TYPE_BIT_CHAR)
	M_assert (0, "M_lvar_layout: bit field not allowed");

      size = type[i].size / 8;
      align = type[i].align / 8;

#if 1
      if (type[i].type == M_TYPE_UNION || type[i].type == M_TYPE_STRUCT)
	{
	  /* For struct parameter passing to work
	     nicely, structs MUST be at least
	     8 byte aligned. */
	  if (align < 8)
	    align = 8;
	}
#endif

      mod = off % align;
      if (mod != 0)
	off += (align - mod);
      offset[i] = off;
      off += size;
      /*
         printf("size %d, align %d, offset %d, endoff %d\n",
         size, align, offset[i], off);
       */
    }

  /*
   * Local variables are relative to $LV
   */
  *base_macro = "$LV";

  return ADDR_ALIGN (off, 16);	/* the total size needed, mult. of 16 bytes */
}
