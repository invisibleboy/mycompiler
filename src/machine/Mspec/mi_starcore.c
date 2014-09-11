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
 *  	  File :	mi_starcore.c 
 * Description : 	Machine dependent specification.  
 *     Authors :        Christopher Shannon
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
#include  "m_starcore.h"
#include <library/i_error.h>

/* model compiling for, internal to this Mspec only */
int M_EM_SC_model = -1;
int stack_only = 0;
char *starcore_fn_name = NULL;

/*--------------------------------------------------------------------------*/

int
M_starcore_type_size (int mtype)
{

  if (EM_SC_MODEL_OK (M_EM_SC_model))
    {

      switch (mtype)
	{
	case M_TYPE_VOID:
	  return M_STARCORE_SIZE_VOID;
	case M_TYPE_BIT_CHAR:
	case M_TYPE_BIT_SHORT:
	case M_TYPE_BIT_LONG:
	  return M_STARCORE_SIZE_BIT;
	case M_TYPE_CHAR:
	  return M_STARCORE_SIZE_CHAR;
	case M_TYPE_SHORT:
	  return M_STARCORE_SIZE_SHORT;
	case M_TYPE_INT:
	  return M_STARCORE_SIZE_INT;
	case M_TYPE_LONG:
	  return M_STARCORE_SIZE_LONG;
	case M_TYPE_FLOAT:
	  return M_STARCORE_SIZE_FLOAT;
	case M_TYPE_DOUBLE:
	  return M_STARCORE_SIZE_DOUBLE;
	case M_TYPE_POINTER:
	  return M_STARCORE_SIZE_POINTER;
	case M_TYPE_UNION:
	  return M_STARCORE_SIZE_UNION;
	case M_TYPE_STRUCT:
	  return M_STARCORE_SIZE_STRUCT;
	case M_TYPE_BLOCK:
	  return M_STARCORE_SIZE_BLOCK;
	default:
	  return -1;
	}
    }
  else
    {
      M_assert (0, "M_starcore_type_size: illegal machine model");
      return (0);
    }
}


int
M_starcore_type_align (int mtype)
{

  if (EM_SC_MODEL_OK (M_EM_SC_model))
    {
      switch (mtype)
	{
	case M_TYPE_VOID:
	  return M_STARCORE_ALIGN_VOID;
	case M_TYPE_BIT_LONG:
	  return M_STARCORE_ALIGN_BIT;
	case M_TYPE_BIT_SHORT:
	  return M_STARCORE_ALIGN_BIT;
	case M_TYPE_BIT_CHAR:
	  return M_STARCORE_ALIGN_BIT;
	case M_TYPE_CHAR:
	  return M_STARCORE_ALIGN_CHAR;
	case M_TYPE_SHORT:
	  return M_STARCORE_ALIGN_SHORT;
	case M_TYPE_INT:
	  return M_STARCORE_ALIGN_INT;
	case M_TYPE_LONG:
	  return M_STARCORE_ALIGN_LONG;
	case M_TYPE_FLOAT:
	  return M_STARCORE_ALIGN_FLOAT;
	case M_TYPE_DOUBLE:
	  return M_STARCORE_ALIGN_DOUBLE;
	case M_TYPE_POINTER:
	  return M_STARCORE_ALIGN_POINTER;
	case M_TYPE_UNION:
	  return M_STARCORE_ALIGN_UNION;
	case M_TYPE_STRUCT:
	  return M_STARCORE_ALIGN_STRUCT;
	case M_TYPE_BLOCK:
	  return M_STARCORE_ALIGN_BLOCK;
	default:
	  return -1;
	}
    }
  else
    {
      M_assert (0, "M_starcore_type_align: illegal machine model");
      return (0);
    }
}

/*--------------------------------------------------------------------------*/
/*
 *    The following functions are for documentation only.
 *      The function overhead is too great for actual usage.
 */
void
M_starcore_void (M_Type type)
{

  if (EM_SC_MODEL_OK (M_EM_SC_model))
    {
      type->type = M_TYPE_VOID;
      type->unsign = 1;
      type->align = M_STARCORE_ALIGN_VOID;
      type->size = M_STARCORE_SIZE_VOID;
      type->nbits = 0;
    }
  else
    {
      M_assert (0, "M_starcore_void: illegal machine model");
    }
}

void
M_starcore_bit_long (M_Type type, int n)
{

  if (EM_SC_MODEL_OK (M_EM_SC_model))
    {
      type->type = M_TYPE_BIT_LONG;
      type->unsign = 1;
      type->align = M_STARCORE_ALIGN_BIT;
      type->size = n * M_STARCORE_SIZE_BIT;
      type->nbits = n * M_STARCORE_SIZE_BIT;
      M_assert ((n <= 32),
		"M_bit_long: do not allow bit field of more than 32 bits");
    }
  else
    {
      M_assert (03, "M_starcore_bit_long: illegal machine model");
    }
}

void
M_starcore_bit_int (M_Type type, int n)
{

  if (EM_SC_MODEL_OK (M_EM_SC_model))
    {
      type->type = M_TYPE_BIT_INT;
      type->unsign = 1;
      type->align = M_STARCORE_ALIGN_BIT;
      type->size = n * M_STARCORE_SIZE_BIT;
      type->nbits = n * M_STARCORE_SIZE_BIT;
      M_assert ((n <= 32),
		"M_bit_int: do not allow bit field of more than 32 bits");
    }
  else
    {
      M_assert (03, "M_starcore_bit_int: illegal machine model");
    }
}

void
M_starcore_bit_short (M_Type type, int n)
{

  if (EM_SC_MODEL_OK (M_EM_SC_model))
    {
      type->type = M_TYPE_BIT_SHORT;
      type->unsign = 1;
      type->align = M_STARCORE_ALIGN_BIT;
      type->size = n * M_STARCORE_SIZE_BIT;
      type->nbits = n * M_STARCORE_SIZE_BIT;
      M_assert ((n <= 16),
		"M_bit_short: do not allow bit field of more than 16 bits");
    }
  else
    {
      M_assert (0, "M_starcore_bit_short: illegal machine model");
    }
}


void
M_starcore_bit_char (M_Type type, int n)
{

  if (EM_SC_MODEL_OK (M_EM_SC_model))
    {
      type->type = M_TYPE_BIT_CHAR;
      type->unsign = 1;
      type->align = M_STARCORE_ALIGN_BIT;
      type->size = n * M_STARCORE_SIZE_BIT;
      type->nbits = n * M_STARCORE_SIZE_BIT;
      M_assert ((n <= 8),
		"M_bit_char: do not allow bit field of more than 8 bits");
    }
  else
    {
      M_assert (0, "M_starcore_bit_char: illegal machine model");
    }
}

void
M_starcore_char (M_Type type, int unsign)
{

  if (EM_SC_MODEL_OK (M_EM_SC_model))
    {
      type->type = M_TYPE_CHAR;
      type->unsign = unsign;
      type->align = M_STARCORE_ALIGN_CHAR;
      type->size = M_STARCORE_SIZE_CHAR;
      type->nbits = M_STARCORE_SIZE_CHAR;
    }
  else
    {
      M_assert (0, "M_starcore_char: illegal machine model");
    }
}

void
M_starcore_short (M_Type type, int unsign)
{

  if (EM_SC_MODEL_OK (M_EM_SC_model))
    {
      type->type = M_TYPE_SHORT;
      type->unsign = unsign;
      type->align = M_STARCORE_ALIGN_SHORT;
      type->size = M_STARCORE_SIZE_SHORT;
      type->nbits = M_STARCORE_SIZE_SHORT;
    }
  else
    {
      M_assert (0, "M_starcore_short: illegal machine model");
    }
}

void
M_starcore_int (M_Type type, int unsign)
{

  if (EM_SC_MODEL_OK (M_EM_SC_model))
    {
      type->type = M_TYPE_INT;
      type->unsign = unsign;
      type->align = M_STARCORE_ALIGN_INT;
      type->size = M_STARCORE_SIZE_INT;
      type->nbits = M_STARCORE_SIZE_INT;
    }
  else
    {
      M_assert (0, "M_starcore_int: illegal machine model");
    }
}

void
M_starcore_long (M_Type type, int unsign)
{

  if (EM_SC_MODEL_OK (M_EM_SC_model))
    {
      type->type = M_TYPE_LONG;
      type->unsign = unsign;
      type->align = M_STARCORE_ALIGN_LONG;
      type->size = M_STARCORE_SIZE_LONG;
      type->nbits = M_STARCORE_SIZE_LONG;
    }
  else
    {
      M_assert (0, "M_starcore_long: illegal machine model");
    }
}

void
M_starcore_float (M_Type type, int unsign)
{

  if (EM_SC_MODEL_OK (M_EM_SC_model))
    {
      type->type = M_TYPE_FLOAT;
      type->unsign = unsign;
      type->align = M_STARCORE_ALIGN_FLOAT;
      type->size = M_STARCORE_SIZE_FLOAT;
      type->nbits = M_STARCORE_SIZE_FLOAT;
    }
  else
    {
      M_assert (0, "M_starcore_float: illegal machine model");
    }
}

void
M_starcore_double (M_Type type, int unsign)
{

  if (EM_SC_MODEL_OK (M_EM_SC_model))
    {
      type->type = M_TYPE_DOUBLE;
      type->unsign = unsign;
      type->align = M_STARCORE_ALIGN_DOUBLE;
      type->size = M_STARCORE_SIZE_DOUBLE;
      type->nbits = M_STARCORE_SIZE_DOUBLE;
    }
  else
    {
      M_assert (0, "M_starcore_double: illegal machine model");
    }
}

void
M_starcore_pointer (M_Type type)
{

  if (EM_SC_MODEL_OK (M_EM_SC_model))
    {
      type->type = M_TYPE_POINTER;
      type->unsign = 1;
      type->align = M_STARCORE_ALIGN_POINTER;
      type->size = M_STARCORE_SIZE_POINTER;
      type->nbits = M_STARCORE_SIZE_POINTER;
    }
  else
    {
      M_assert (0, "M_starcore_pointer: illegal machine model");
    }
}

/*--------------------------------------------------------------------------*/

int
M_starcore_eval_type (M_Type type, M_Type ntype)
{

  if (EM_SC_MODEL_OK (M_EM_SC_model))
    {
      switch (type->type)
	{
	case M_TYPE_VOID:
	  M_starcore_void (ntype);
	  return (-1);		/* can not be evaluated */
	case M_TYPE_BIT_LONG:
	case M_TYPE_BIT_CHAR:
	case M_TYPE_CHAR:
	case M_TYPE_SHORT:
	case M_TYPE_INT:
	case M_TYPE_LONG:
	  M_starcore_int (ntype, type->unsign);
	  return (M_TYPE_INT);
	case M_TYPE_POINTER:
	case M_TYPE_BLOCK:
	  /* the starting address of array is used */
	  M_starcore_pointer (ntype);
	  return (M_TYPE_POINTER);
	case M_TYPE_FLOAT:
	  /*L_punt ("M_starcore_eval_type: found M_TYPE_FLOAT");*/
	  M_starcore_float (ntype, type->unsign);
	  return (M_TYPE_FLOAT);
	case M_TYPE_DOUBLE:
	  /*L_punt ("M_starcore_eval_type: found M_TYPE_DOUBLE");*/
	  M_starcore_double (ntype, type->unsign);
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
      M_assert (0, "M_starcore_eval_type: illegal machine model");
      return (0);
    }
}

int
M_starcore_eval_type2 (M_Type type, M_Type ntype)
{

  if (EM_SC_MODEL_OK (M_EM_SC_model))
    {
      switch (type->type)
	{
	case M_TYPE_VOID:
	  M_starcore_void (ntype);
	  return (-1);		/* can not be evaluated */
	case M_TYPE_BIT_CHAR:
	case M_TYPE_CHAR:
	  M_starcore_char (ntype, type->unsign);
	  return (M_TYPE_CHAR);
	case M_TYPE_SHORT:
	  M_starcore_short (ntype, type->unsign);
	  return (M_TYPE_SHORT);
	case M_TYPE_INT:
	case M_TYPE_BLOCK:
	  M_starcore_int (ntype, type->unsign);
	  return (M_TYPE_INT);
	case M_TYPE_BIT_LONG:
	case M_TYPE_LONG:
	  M_starcore_long (ntype, type->unsign);
	  return (M_TYPE_LONG);
	case M_TYPE_POINTER:
	  M_starcore_pointer (ntype);
	  return (M_TYPE_POINTER);
	case M_TYPE_FLOAT:
	  M_starcore_float (ntype, type->unsign);
	  return (M_TYPE_FLOAT);
	case M_TYPE_DOUBLE:
	  M_starcore_double (ntype, type->unsign);
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
      M_assert (0, "M_starcore_eval_type2: illegal machine model");
      return (0);
    }
}

int
M_starcore_call_type (M_Type type, M_Type ntype)
{

  if (EM_SC_MODEL_OK (M_EM_SC_model))
    {
      switch (type->type)
	{
	case M_TYPE_VOID:
	  M_starcore_void (ntype);
	  return (-1);		/* can not be evaluated */
	case M_TYPE_BIT_CHAR:
	case M_TYPE_BIT_LONG:
	case M_TYPE_CHAR:
	case M_TYPE_SHORT:
	case M_TYPE_INT:
	case M_TYPE_LONG:
	  M_starcore_int (ntype, type->unsign);
	  return (M_TYPE_INT);
	case M_TYPE_POINTER:
	case M_TYPE_BLOCK:
	  /* the starting address of array is used */
	  M_starcore_pointer (ntype);
	  return (M_TYPE_POINTER);
	case M_TYPE_FLOAT:
	case M_TYPE_DOUBLE:
	  M_starcore_double (ntype, type->unsign);
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
      M_assert (0, "M_starcore_call_type: illegal machine model");
      return (0);
    }
}

int
M_starcore_call_type2 (M_Type type, M_Type ntype)
{

  if (EM_SC_MODEL_OK (M_EM_SC_model))
    {
      switch (type->type)
	{
	case M_TYPE_VOID:
	  M_starcore_void (ntype);
	  return (-1);		/* can not be evaluated */
	case M_TYPE_BIT_CHAR:
	case M_TYPE_CHAR:
	  M_starcore_char (ntype, type->unsign);
	  return (M_TYPE_CHAR);
	case M_TYPE_SHORT:
	  M_starcore_short (ntype, type->unsign);
	  return (M_TYPE_SHORT);
	case M_TYPE_INT:
	case M_TYPE_BLOCK:
	  /* the starting address of array is used */
	  M_starcore_int (ntype, type->unsign);
	  return (M_TYPE_INT);
	case M_TYPE_BIT_LONG:
	case M_TYPE_LONG:
	  M_starcore_long (ntype, type->unsign);
	  return (M_TYPE_LONG);
	case M_TYPE_POINTER:
	  M_starcore_pointer (ntype);
	  return (M_TYPE_POINTER);
	case M_TYPE_FLOAT:
	  M_starcore_float (ntype, type->unsign);
	  return (M_TYPE_FLOAT);
	case M_TYPE_DOUBLE:
	  M_starcore_double (ntype, type->unsign);
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
      M_assert (0, "M_starcore_call_type2: illegal machine model");
      return (0);
    }
}

/*--------------------------------------------------------------------------*/

void
M_starcore_array_layout (M_Type type, int *offset)
{

  if (EM_SC_MODEL_OK (M_EM_SC_model))
    {
      *offset = 0;		/* assume first element is aligned */
    }
  else
    {
      M_assert (0, "M_starcore_array_layout: illegal machine model");
    }
}


int
M_starcore_array_align (M_Type type)
{

  if (EM_SC_MODEL_OK (M_EM_SC_model))
    {
      return type->align;
    }
  else
    {
      M_assert (0, "M_starcore_array_align: illegal machine model");
      return (0);
    }
}


int
M_starcore_array_size (M_Type type, int dim)
{
  int mod, size, align;

  if (EM_SC_MODEL_OK (M_EM_SC_model))
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
      M_assert (0, "M_starcore_array_size: illegal machine model");
      return (0);
    }

}

/*--------------------------------------------------------------------------*/

void
M_starcore_union_layout (int n, _M_Type * type, int *offset, int *bit_offset)
{
  int i;

  if (EM_SC_MODEL_OK (M_EM_SC_model))
    {
      for (i = 0; i < n; i++)
	{			/* assume the union is aligned */
	  offset[i] = 0;
	  bit_offset[i] = 0;
	}
    }
  else
    {
      M_assert (0, "M_starcore_union_layout: illegal machine model");
    }
}

int
M_starcore_union_align (int n, _M_Type * type)
{

  if (EM_SC_MODEL_OK (M_EM_SC_model))
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
      if (max < M_STARCORE_ALIGN_CHAR)
	max = M_STARCORE_ALIGN_CHAR;

      return max;
    }
  else
    {
      M_assert (0, "M_starcore_union_align: illegal machine model");
      return (0);
    }
}

int
M_starcore_union_size (int n, _M_Type * type)
{
  int i, aln, max_size, max_align;

  if (EM_SC_MODEL_OK (M_EM_SC_model))
    {
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
      if (max_align < M_STARCORE_ALIGN_CHAR)
	max_align = M_STARCORE_ALIGN_CHAR;

      /* need to increment to the max. align for future array extension */
      i = max_size % max_align;
      if (i != 0)
	max_size += (max_align - i);

      return max_size;
    }
  else
    {
      M_assert (0, "M_starcore_union_size: illegal machine model");
      return (0);
    }
}

/*--------------------------------------------------------------------------*/

int
M_starcore_layout_order (void)
{

  if (EM_SC_MODEL_OK (M_EM_SC_model))
    {
      return M_LITTLE_ENDIAN;
      /* return M_BIG_ENDIAN; */
    }
  else
    {
      M_assert (0, "M_starcore_layout_order: illegal machine model");
      return (0);
    }
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
M_starcore_struct_layout (int n, _M_Type *type, int *base, int *bit_offset)
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

  if (EM_SC_MODEL_OK (M_EM_SC_model))
    {
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
	  if (M_starcore_layout_order () == M_BIG_ENDIAN)
	    {
	      mod_word = offset % M_STARCORE_SIZE_INT;
	      if (type[i].type == M_TYPE_BIT_CHAR)
		{
		  if ((mod_word + size) > M_STARCORE_SIZE_INT)
		    offset += (M_STARCORE_SIZE_INT - mod_word);
		  else
		    {
		      mod_type = offset % M_STARCORE_SIZE_CHAR;
		      if ((mod_type + size) > M_STARCORE_SIZE_CHAR)
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
		  if ((mod_word + size) > M_STARCORE_SIZE_INT)
		    offset += (M_STARCORE_SIZE_INT - mod_word);
		  else
		    {
		      mod_type = offset % M_STARCORE_SIZE_SHORT;
		      if ((mod_type + size) > M_STARCORE_SIZE_SHORT)
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
		  if ((mod_word + size) > M_STARCORE_SIZE_INT)
		    offset += (M_STARCORE_SIZE_INT - mod_word);
		}

	      mod = offset % align;	/* align to what the field */
	      if (mod != 0)	        /* needs to start from */
		offset += (align - mod);

	      if (type[i].type == M_TYPE_BIT_CHAR)
		{
		  int mod = offset % M_STARCORE_SIZE_INT;

		  bit_offset[i] =
		    offset - mod + (M_STARCORE_SIZE_INT - mod - size);
		  base[i] = offset & (~(M_STARCORE_SIZE_CHAR - 1));
		}
	      else if (type[i].type == M_TYPE_BIT_SHORT)
		{
		  int mod = offset % M_STARCORE_SIZE_INT;

		  bit_offset[i] =
		    offset - mod + (M_STARCORE_SIZE_INT - mod - size);
		  base[i] = offset & (~(M_STARCORE_SIZE_SHORT - 1));
		}
	      else if (type[i].type == M_TYPE_BIT_LONG)
		{
		  int mod = offset % M_STARCORE_SIZE_INT;

		  bit_offset[i] =
		    offset - mod + (M_STARCORE_SIZE_INT - mod - size);
		  base[i] = offset & (~(M_STARCORE_SIZE_LONG - 1));
		}
	      else
		{
		  base[i] = offset;
		  bit_offset[i] = 0;
		}

	      offset += size;	/* allocate space */


	    }
	  else if (M_starcore_layout_order () == M_LITTLE_ENDIAN)
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
		  word_off = offset % M_STARCORE_SIZE_INT;

		  /* if element doesn't fit in word,
		   * then its offset is beginning of the next word */
		  if ((word_off + size) > M_STARCORE_SIZE_INT)
		    offset += (M_STARCORE_SIZE_INT - word_off);

		  /* if element doesn't fit in aligned unit size, 
		     then successively
		   * increase unit size until it does */
		  if (type[i].type == M_TYPE_BIT_CHAR)
		    {
		      bf_unit_size = M_STARCORE_SIZE_CHAR;
		      bf_unit_off = offset % bf_unit_size;

		      /* use for standard bitfields */
		      if ((bf_unit_off + size) > M_STARCORE_SIZE_CHAR)
			{
			  type[i].type = M_TYPE_BIT_SHORT;
			  bf_unit_size = M_STARCORE_SIZE_SHORT;
			}
		    }
		  if (type[i].type == M_TYPE_BIT_SHORT)
		    {
		      bf_unit_size = M_STARCORE_SIZE_SHORT;
		      bf_unit_off = offset % bf_unit_size;

		      /* use for standard bitfields */
		      if ((bf_unit_off + size) > M_STARCORE_SIZE_SHORT)
			{
			  type[i].type = M_TYPE_BIT_LONG;
			  bf_unit_size = M_STARCORE_SIZE_LONG;
			}

		    }
		  if (type[i].type == M_TYPE_BIT_LONG)
		    {
		      bf_unit_size = M_STARCORE_SIZE_LONG;
		      bf_unit_off = offset % bf_unit_size;
		      if ((bf_unit_off + size) > M_STARCORE_SIZE_LONG)
			{
			  /* problem:  can't increase size any more */
			  M_assert (0,
				    "M_starcore_struct_layout:  bitfield "
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
	      M_assert(0, "M_starcore_struct_layout: endian ordering not set");
	    }

	}

    }
  else
    {
      M_assert (0, "M_starcore_struct_layout: illegal machine model");
    }
}

int
M_starcore_struct_align (int n, _M_Type *type)
{
  int i, max;

  if (EM_SC_MODEL_OK (M_EM_SC_model))
    {
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
      if (max < M_STARCORE_ALIGN_CHAR)
	max = M_STARCORE_ALIGN_CHAR;
      return max;
    }
  else
    {
      M_assert (0, "M_starcore_struct_align: illegal machine model");
      return (0);
    }
}


int
M_starcore_struct_size (int n, _M_Type * type, int struct_align)
{

  int i, offset;
  int mod, size, align, max_align, mod_word;
  int word_off;

  if (EM_SC_MODEL_OK (M_EM_SC_model))
    {
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
	  if (M_starcore_layout_order () == M_BIG_ENDIAN)
	    {

	      mod_word = offset % M_STARCORE_SIZE_INT;
	      if (type[i].type == M_TYPE_BIT_CHAR)
		{
		  if ((mod_word + size) > M_STARCORE_SIZE_INT)
		    offset += (M_STARCORE_SIZE_INT - mod_word);
		  if (M_STARCORE_ALIGN_CHAR > max_align)
		    max_align = M_STARCORE_ALIGN_CHAR;
		}
	      else if (type[i].type == M_TYPE_BIT_SHORT)
		{
		  if ((mod_word + size) > M_STARCORE_SIZE_INT)
		    offset += (M_STARCORE_SIZE_INT - mod_word);
		  if (M_STARCORE_ALIGN_SHORT > max_align)
		    max_align = M_STARCORE_ALIGN_SHORT;
		}
	      else if (type[i].type == M_TYPE_BIT_LONG)
		{
		  if ((mod_word + size) > M_STARCORE_SIZE_INT)
		    offset += (M_STARCORE_SIZE_INT - mod_word);
		  if (M_STARCORE_ALIGN_LONG > max_align)
		    max_align = M_STARCORE_ALIGN_LONG;
		}

	    }
	  else if (M_starcore_layout_order () == M_LITTLE_ENDIAN)
	    {

	      if (type[i].type == M_TYPE_BIT_CHAR ||
		  type[i].type == M_TYPE_BIT_SHORT ||
		  type[i].type == M_TYPE_BIT_LONG)
		{
		  word_off = offset % M_STARCORE_SIZE_INT;

		  if ((word_off + size) > M_STARCORE_SIZE_INT)
		    {
		      /* if element doesn't fit in word,
		       * then its offset is beginning of the next word */
		      offset += (M_STARCORE_SIZE_INT - word_off);
		    }

		}

	    }
	  else
	    {
	      M_assert (0, "M_starcore_struct_size: endian ordering not set");
	    }

	  mod = offset % align;	/* align to what the field */
	  if (mod != 0)		/* needs to start from */
	    offset += (align - mod);

	  offset += size;
	}
      /*
       *      align to at least byte boundary.
       */
      if (max_align < M_STARCORE_ALIGN_CHAR)
	max_align = M_STARCORE_ALIGN_CHAR;
      /* enforce max. alignment */
      mod = offset % max_align;
      if (mod != 0)
	offset += (max_align - mod);
      return offset;
    }
  else
    {
      M_assert (0, "M_starcore_struct_size: illegal machine model");
      return (0);
    }

}

/* Returns the position in the functions argument list that the const char*
 * expression is, and thus where varargs begins.  If the function isn't a
 * varargs function, a value of INT_MAX is returned
 */
static int
M_is_vararg_func (char *str)
{
  if (!str)
    return INT_MAX;

  if (!strcmp (str, "printf"))
    return 0;
  if (!strcmp (str, "scanf"))
    return 0;
  if (!strcmp (str, "vprintf"))
    return 0;
  if (!strcmp (str, "fprintf"))
    return 1;
  if (!strcmp (str, "sprintf"))
    return 1;
  if (!strcmp (str, "vfprintf"))
    return 1;
  if (!strcmp (str, "vsprintf"))
    return 1;
  if (!strcmp (str, "fscanf"))
    return 1;
  if (!strcmp (str, "sscanf"))
    return 1;

  return INT_MAX;
}

/*--------------------------------------------------------------------------*/

/* align_bit = bits, offset = bytes */
static int
M_align_offset (int offset, int align_bit)
{
  int mod;
  int align = align_bit;

  if ((align_bit % 8) != 0)
    I_punt ("M_align_offset: alignment not multiple of 8 = %d\n",
	   align_bit);

  mod = offset % align;
  if (mod != 0)
    offset += (align - mod);

  return offset;
}

int
M_starcore_layout_fnvar (List param_list, char **base_macro, int *pcount,
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

  size = M_starcore_fnvar_layout (cnt, type, offset, mode, reg, paddr,
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


int
M_starcore_fnvar_layout (int n, _M_Type *type, long int *offset, int *mode,
			 int *reg, int *paddr, char **macro, int *su_sreg, 
			 int *su_ereg, int *pcount, int is_st, int purpose)
{
  int i, max_align, current_offset, vararg_func, size, align, tp,
    int_reg_base, param_position, marker, endoffset[128],
    int_reg_size, addr_reg_size, addr_reg_base;

  int_reg_size = M_read_database_i ("_HT__base_types", "int", "size");
  addr_reg_size = M_read_database_i ("_HT__base_types", "int", "size");

  vararg_func = M_is_vararg_func (starcore_fn_name);

  switch (purpose)
    {
    case M_GET_FNVAR:
      *macro = "$IP";
      int_reg_base = M_STARCORE_IN_INT_BASE;
      addr_reg_base = M_STARCORE_IN_ADDR_BASE;
      break;
    case M_PUT_FNVAR:
      *macro = "$OP";
      int_reg_base = M_STARCORE_OUT_INT_BASE;
      addr_reg_base = M_STARCORE_OUT_ADDR_BASE;
      break;
    case M_DONT_CARE_FNVAR:
    default:
      M_assert (0, "M_fnvar_layout: unknown purpose");
      return (-1);
    }

  max_align = M_STARCORE_ALIGN_MAX;
  param_position = 0;
  current_offset = 0;

  for (i = 0; i < n; i++)
    {
      tp = type[i].type;
      size = type[i].size;
      align = type[i].align;

      switch (tp)
	{
	case M_TYPE_CHAR:
	case M_TYPE_SHORT:
	case M_TYPE_INT:
	case M_TYPE_LONG:
	case M_TYPE_FLOAT:
	case M_TYPE_DOUBLE:
	  if ((param_position < M_STARCORE_MAX_FNVAR_INT_REG) && 
	      (i < vararg_func))
	    {
	      mode[i] = M_THRU_REGISTER;
	      reg[i] = param_position + int_reg_base;
	      offset[i] = 0;
	      param_position++;
	    }
	  else
	    {
	      /* Verify alignment */
	      current_offset = M_align_offset (current_offset, align);

	      mode[i] = M_THRU_MEMORY;
	      reg[i] = -1;
	      current_offset += int_reg_size;
	      offset[i] = -current_offset;
	    }
	  break;

	case M_TYPE_POINTER:
	  if ((param_position < M_STARCORE_MAX_FNVAR_ADDR_REG) &&
	      (i < vararg_func))
	    {
	      mode[i] = M_THRU_REGISTER;
	      reg[i] = param_position + addr_reg_base;
	      offset[i] = 0;
	      param_position++;
	    }
	  else
	    {
	      /* Verify alignment */
	      current_offset = M_align_offset(current_offset, align);

	      mode[i] = M_THRU_MEMORY;
	      reg[i] = -1;
	      current_offset += addr_reg_size;
	      offset[i] = -current_offset;
	    }
	  break;

	case M_TYPE_UNION:
	case M_TYPE_STRUCT:
	  endoffset[i] = -1;

	  mode[i] = M_THRU_MEMORY;
	  reg[i] = -1;
	  su_sreg[i] = -1;
	  su_ereg[i] = -1;
	  paddr[i] = -1;

	  /* Verify alignment */
	  current_offset = M_align_offset(current_offset, align);

	  current_offset += size;
	  offset[i] = -current_offset;
	  endoffset[i] = current_offset;
	  break;

	default:
	  M_assert (0, "M_fnvar_layout: argument is not promoted");
	}
    }

  marker = M_STARCORE_MAX_FNVAR_INT_REG - param_position;
  for (i = (n - 1); i >= 0; i--)
    {
      if ((type[i].type == M_TYPE_UNION || type[i].type == M_TYPE_STRUCT) &&
	  (paddr[i] >= 0))
	{
	  paddr[i] = paddr[i] + marker;
	}
    }

  *pcount = param_position;
  return (current_offset);	/* size necessary for parameters */
}

/* local space size will be a multiple of 16 so that the size won't
 * have to be changed in phase2.  Phase1 vararg needs to know the
 * exact size of the local variable space. */

int
M_starcore_fnvar_to_lvar (_M_Type type, long int *offset,
			  char **base_macro, int local_space)
{
  int current_offset, total_local_space;
  int size, align;

  total_local_space = 0;
  current_offset = local_space * 8;

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
      size = type.size;
      align = type.align;
      current_offset = M_align_offset (current_offset, align);
      current_offset += size;
      *offset = -(current_offset / 8);
      break;

    default:
      M_assert (0, "M_starcore_fnvar_to_lvar: Unrecognized fnvar type");
    }

  return (current_offset / 8);
}

/*--------------------------------------------------------------------------*/
int
M_starcore_lvar_layout (int n, _M_Type * type, long int *offset,
			char **base_macro)
{
  int i, off;
  int size, align, mod, tp;

  off = 0;

  for (i = 0; i < n; i++)
    {
      tp = type[i].type;
      if (tp == M_TYPE_BIT_LONG)
	{
	  M_assert (0, "M_lvar_layout: bit field not allowed");
	}
      if (tp == M_TYPE_BIT_INT)
	{
	  M_assert (0, "M_lvar_layout: bit field not allowed");
	}
      if (tp == M_TYPE_BIT_CHAR)
	{
	  M_assert (0, "M_lvar_layout: bit field not allowed");
	}

      size = type[i].size;
      align = type[i].align;

      if (type[i].type == M_TYPE_UNION || type[i].type == M_TYPE_STRUCT)
	{
	  /* For struct parameter passing to work
	     nicely, structs MUST be at least
	     4 byte aligned. */
	  if (align < 32)
	    align = 32;
	}

      mod = off % align;
      if (mod != 0)
	off += (align - mod);
      off += size;
      offset[i] = -off;
    }

  /*
   * Local variables are relative to $LV
   */
  *base_macro = "$LV";

  return off;
}

/*--------------------------------------------------------------------------*/

int
M_starcore_no_short_int (void)
{

  if (EM_SC_MODEL_OK (M_EM_SC_model))
    {
      return (M_STARCORE_SIZE_SHORT == M_STARCORE_SIZE_INT);
    }
  else
    {
      M_assert (0, "M_starcore_no_short_int: illegal machine model");
      return (0);
    }
}
/*--------------------------------------------------------------------------*/

void
M_starcore_cb_label_name (char *fn, int cb, char *line, int len)
{

  if (EM_SC_MODEL_OK (M_EM_SC_model))
    {
      sprintf (line, "cb%d%s", cb, fn);
    }
  else
    {
      M_assert (0, "M_starcore_cb_label_name: illegal machine model");
    }
}
/*--------------------------------------------------------------------------*/

int
M_starcore_is_cb_label (char *label, char *fn, int *cb)
{

  if (EM_SC_MODEL_OK (M_EM_SC_model))
    {
      return (sscanf (label, "cb%d%s", cb, fn) == 2);
    }
  else
    {
      M_assert (0, "M_starcore_is_cb_label: illegal machine model");
      return (0);
    }
}

/*--------------------------------------------------------------------------*/

void
M_starcore_jumptbl_label_name (char *fn, int tbl_id, char *line, int len)
{

  if (EM_SC_MODEL_OK (M_EM_SC_model))
    {
      sprintf (line, "%s%s%d", fn, M_JUMPTBL_BASE_NAME, tbl_id);
    }
  else
    {
      M_assert (0, "M_starcore_cb_label_name: illegal machine model");
    }
}

/*--------------------------------------------------------------------------*/

int
M_starcore_is_jumptbl_label (char *label, char *fn, int *tbl_id)
{

  if (EM_SC_MODEL_OK (M_EM_SC_model))
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
  else
    {
      M_assert (0, "M_starcore_is_jumptbl_label: illegal machine model");
      return (0);
    }
}

/*--------------------------------------------------------------------------*/

int
M_starcore_structure_pointer (int purpose)
{

  if (EM_SC_MODEL_OK (M_EM_SC_model))
    {
      return M_STARCORE_LCODE_RET_ST;
    }
  else
    {
      M_assert (0, "M_starcore_structure_pointer: illegal machine model");
      return (0);
    }
}

/*--------------------------------------------------------------------------*/

int
M_starcore_return_register (int type, int purpose)
{

  if (EM_SC_MODEL_OK (M_EM_SC_model))
    {
      switch (type)
	{
	case M_TYPE_INT:
	case M_TYPE_LONG:
	case M_TYPE_FLOAT:
	case M_TYPE_DOUBLE:
	  return M_STARCORE_LCODE_RET_INT;
	case M_TYPE_POINTER:
	  return M_STARCORE_LCODE_RET_ADDR;
	case M_TYPE_STRUCT:
	  return M_STARCORE_LCODE_RET_ST;
	default:
	  return M_STARCORE_LCODE_RET_INT;
	}
    }
  else
    {
      M_assert (0, "M_starcore_return_register: illegal machine model");
      return (0);
    }
}

/*--------------------------------------------------------------------------*/

char *
M_starcore_fn_label_name (char *label, int (*is_func) (char *is_func_label))
{

  if (EM_SC_MODEL_OK (M_EM_SC_model))
    {
      static char fn_label[64];

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
      M_assert (0, "M_starcore_fn_label_name: illegal machine model");
      return (0);
    }
}

char *
M_starcore_fn_name_from_label (char *label)
{

  if (EM_SC_MODEL_OK (M_EM_SC_model))
    {
      if (!strncmp (label, "_$fn", 4))
	return (label + 4);
      else
	return (label);
    }
  else
    {
      M_assert (0, "M_starcore_fn_name_from_label: illegal machine model");
      return (0);
    }
}

/*--------------------------------------------------------------------------*/

void
M_set_model_starcore (char *model_name)
{

  if (!strcasecmp (model_name, "SC140"))
    {
      M_model = M_EM_SC140;
      M_EM_SC_model = M_EM_SC140;
    }
  else
    {
      fprintf (stderr, "illegal model_name : %s\n", model_name);
      fprintf (stderr, "Only currently supporting SC140\n");
      M_assert (0, "M_set_model_starcore: illegal model name specified");
    }

  M_use_layout_database = 0;

}

