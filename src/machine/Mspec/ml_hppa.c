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
 *	File :	ml_hppa.c 
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
#include <stdio.h>
#ifdef M_HPPA_FOR_HCODE
#include <Hcode/h_ccode.h>
#endif
#include <Lcode/l_main.h>
#include "m_hppa.h"

/*--------------------------------------------------------------------------*/
#define M_HPPA_SIZE_VOID	0
#define M_HPPA_SIZE_BIT		1
#define M_HPPA_SIZE_CHAR	8
#define M_HPPA_SIZE_SHORT	16
#define M_HPPA_SIZE_INT		32
#define M_HPPA_SIZE_LONG	32
#define M_HPPA_SIZE_FLOAT	32
#define M_HPPA_SIZE_DOUBLE	64
#define M_HPPA_SIZE_POINTER	32
#define M_HPPA_SIZE_UNION	-1
#define M_HPPA_SIZE_STRUCT	-1
#define M_HPPA_SIZE_BLOCK	-1
#define M_HPPA_SIZE_MAX	 	64

#define M_HPPA_ALIGN_VOID	-1
#define M_HPPA_ALIGN_BIT	1
#define M_HPPA_ALIGN_CHAR	8
#define M_HPPA_ALIGN_SHORT	16
#define M_HPPA_ALIGN_INT	32
#define M_HPPA_ALIGN_LONG	32
#define M_HPPA_ALIGN_FLOAT	32
#define M_HPPA_ALIGN_DOUBLE	64
#define M_HPPA_ALIGN_POINTER	32
#define M_HPPA_ALIGN_UNION	-1	/* depends on the field */
#define M_HPPA_ALIGN_STRUCT	-1
#define M_HPPA_ALIGN_BLOCK	-1
#define M_HPPA_ALIGN_MAX	64

Set Set_hppa_fragile_macro = NULL;

/*--------------------------------------------------------------------------*/
#define M_HPPA_MAX_FNVAR_REG 		4
#define M_HPPA_SMALL_STRUCT_MAX 	64
#define MIN_PARAM_SIZE 	  		(16 * 8)

/* incoming and outgoing parameters */
#define M_HPPA_INT_BASE		0
#define M_HPPA_FLT_BASE		4

#define M_HPPA_RET_I32		15	/* gr28 */
#define M_HPPA_RET_I64		15	/* gr28 & gr29 */
#define M_HPPA_RET_ST		15	/* gr28 */
#define M_HPPA_RET_F		4	/* fr04 */


/*--------------------------------------------------------------------------*/
/*
 *	The following functions are for documentation only.
 *	The function overhead is too great for actual usage.
 */
void
M_hppa_void (M_Type type)
{
  type->type = M_TYPE_VOID;
  type->unsign = 1;
  type->align = M_HPPA_ALIGN_VOID;
  type->size = M_HPPA_SIZE_VOID;
  type->nbits = 0;
}

void
M_hppa_bit_long (M_Type type, int n)
{
  type->type = M_TYPE_BIT_LONG;
  type->unsign = 1;
  type->align = M_HPPA_ALIGN_BIT;
  type->size = n * M_HPPA_SIZE_BIT;
  type->nbits = n * M_HPPA_SIZE_BIT;
  M_assert ((n <= 32),
	    "M_bit_long: do not allow bit field of more than 32 bits");
}

void
M_hppa_bit_int (M_Type type, int n)
{
  type->type = M_TYPE_BIT_INT;
  type->unsign = 1;
  type->align = M_HPPA_ALIGN_BIT;
  type->size = n * M_HPPA_SIZE_BIT;
  type->nbits = n * M_HPPA_SIZE_BIT;
  M_assert ((n <= 32),
	    "M_bit_int: do not allow bit field of more than 32 bits");
}

void
M_hppa_bit_short (M_Type type, int n)
{
  type->type = M_TYPE_BIT_SHORT;
  type->unsign = 1;
  type->align = M_HPPA_ALIGN_BIT;
  type->size = n * M_HPPA_SIZE_BIT;
  type->nbits = n * M_HPPA_SIZE_BIT;
  M_assert ((n <= 16),
	    "M_bit_long: do not allow bit field of more than 16 bits");
}

void
M_hppa_bit_char (M_Type type, int n)
{
  type->type = M_TYPE_BIT_CHAR;
  type->unsign = 1;
  type->align = M_HPPA_ALIGN_BIT;
  type->size = n * M_HPPA_SIZE_BIT;
  type->nbits = n * M_HPPA_SIZE_BIT;
  M_assert ((n <= 8),
	    "M_bit_char: do not allow bit field of more than 8 bits");
}

void
M_hppa_float (M_Type type, int unsign)
{
  type->type = M_TYPE_FLOAT;
  type->unsign = unsign;
  type->align = M_HPPA_ALIGN_FLOAT;
  type->size = M_HPPA_SIZE_FLOAT;
  type->nbits = M_HPPA_SIZE_FLOAT;
}

void
M_hppa_double (M_Type type, int unsign)
{
  type->type = M_TYPE_DOUBLE;
  type->unsign = unsign;
  type->align = M_HPPA_ALIGN_DOUBLE;
  type->size = M_HPPA_SIZE_DOUBLE;
  type->nbits = M_HPPA_SIZE_DOUBLE;
}

void
M_hppa_pointer (M_Type type)
{
  type->type = M_TYPE_POINTER;
  type->unsign = 1;
  type->align = M_HPPA_ALIGN_POINTER;
  type->size = M_HPPA_SIZE_POINTER;
  type->nbits = M_HPPA_SIZE_POINTER;
}

/*--------------------------------------------------------------------------*/
int
M_hppa_eval_type (M_Type type, M_Type ntype)
{
  switch (type->type)
    {
    case M_TYPE_VOID:
      M_void (ntype);
      return (-1);		/* can not be evaluated */
    case M_TYPE_BIT_CHAR:
    case M_TYPE_CHAR:
    case M_TYPE_SHORT:
    case M_TYPE_BLOCK:
    case M_TYPE_INT:
    case M_TYPE_BIT_LONG:
    case M_TYPE_LONG:
    case M_TYPE_POINTER:
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
M_hppa_eval_type2 (M_Type type, M_Type ntype)
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
M_hppa_call_type (M_Type type, M_Type ntype)
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
    case M_TYPE_FLOAT:
#if 0
      /* commented out, because passing floats doesn't appear to work with
         library functions such as printf, although nowhere does it say that
         I have to use doubles                                            */
      M_float (ntype, type->unsign);
      return (M_TYPE_FLOAT);
#endif
      /* BCC - 8/5/96
       * Pcode has inserted all the necessary castings. So don't insert
       * promotions here
       */
      M_float (ntype, type->unsign);
      return (M_TYPE_FLOAT);
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
M_hppa_call_type2 (M_Type type, M_Type ntype)
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
#if 0
      /* commented out, because passing floats doesn't appear to work with
         library functions such as printf, although nowhere does it say that
         I have to use doubles                                            */
      M_float (ntype, type->unsign);
      return (M_TYPE_FLOAT);
#endif
      /* BCC - 8/5/96
       * Pcode has inserted all the necessary castings. So don't insert
       * promotions here
       */
      M_float (ntype, type->unsign);
      return (M_TYPE_FLOAT);
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
M_hppa_array_layout (M_Type type, int *offset)
{
  *offset = 0;			/* assume first element is aligned */
}

int
M_hppa_array_align (M_Type type)
{
  return type->align;
}

int
M_hppa_array_size (M_Type type, int dim)
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
M_hppa_union_layout (int n, _M_Type * type, int *offset, int *bit_offset)
{
  int i;
  for (i = 0; i < n; i++)
    {				/* assume the union is aligned */
      offset[i] = 0;
      bit_offset[i] = 0;
    }
}

int
M_hppa_union_align (int n, _M_Type * type)
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
  if (max < M_HPPA_ALIGN_CHAR)
    max = M_HPPA_ALIGN_CHAR;

  return max;
}

int
M_hppa_union_size (int n, _M_Type * type)
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
  if (max_align < M_HPPA_ALIGN_CHAR)
    max_align = M_HPPA_ALIGN_CHAR;

  /* need to increment to the max. align for future array extension */
  i = max_size % max_align;
  if (i != 0)
    max_size += (max_align - i);

  return max_size;
}

/*--------------------------------------------------------------------------*/
/* NOTE: the bit_offset array is never used by Hcode, so I ignore it.       */
/*--------------------------------------------------------------------------*/
void
M_hppa_struct_layout (int n, _M_Type * type, int *base, int *bit_offset)
{
  int i, offset;
  int mod, size, align, mod_word, mod_type;

  offset = 0;			/* assume initially aligned */
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
      mod_word = offset % M_HPPA_SIZE_INT;
      if (type[i].type == M_TYPE_BIT_CHAR)
	{
	  if ((mod_word + size) > M_HPPA_SIZE_INT)
	    offset += (M_HPPA_SIZE_INT - mod_word);
	  else
	    {
	      mod_type = offset % M_HPPA_SIZE_CHAR;
	      if ((mod_type + size) > M_HPPA_SIZE_CHAR)
		{
		  /* fprintf(stdout,"Changing bit char to bit short\n"); */
		  type[i].type = M_TYPE_BIT_SHORT;
		}
	    }
	}
      if (type[i].type == M_TYPE_BIT_SHORT)
	{
	  if ((mod_word + size) > M_HPPA_SIZE_INT)
	    offset += (M_HPPA_SIZE_INT - mod_word);
	  else
	    {
	      mod_type = offset % M_HPPA_SIZE_SHORT;
	      if ((mod_type + size) > M_HPPA_SIZE_SHORT)
		{
		  /* fprintf(stdout,"Changing bit short to bit long\n"); */
		  type[i].type = M_TYPE_BIT_LONG;
		}
	    }
	}
      else if (type[i].type == M_TYPE_BIT_LONG)
	{
	  if ((mod_word + size) > M_HPPA_SIZE_INT)
	    offset += (M_HPPA_SIZE_INT - mod_word);
	}

      mod = offset % align;	/* align to what the field */
      if (mod != 0)		/* needs to start from */
	offset += (align - mod);

      if (type[i].type == M_TYPE_BIT_CHAR)
	{
	  int mod = offset % M_HPPA_SIZE_INT;

	  bit_offset[i] = offset - mod + (M_HPPA_SIZE_INT - mod - size);
	  base[i] = offset & (~(M_HPPA_SIZE_CHAR - 1));
#if 0
	  fprintf (stderr, "type = %d, base = %d, bit_offset = %d\n",
		   type[i].type, base[i], bit_offset[i]);
#endif
	}
      else if (type[i].type == M_TYPE_BIT_SHORT)
	{
	  int mod = offset % M_HPPA_SIZE_INT;

	  bit_offset[i] = offset - mod + (M_HPPA_SIZE_INT - mod - size);
	  base[i] = offset & (~(M_HPPA_SIZE_SHORT - 1));
#if 0
	  fprintf (stderr, "type = %d, base = %d, bit_offset = %d\n",
		   type[i].type, base[i], bit_offset[i]);
#endif
	}
      else if (type[i].type == M_TYPE_BIT_LONG)
	{
	  int mod = offset % M_HPPA_SIZE_INT;

	  bit_offset[i] = offset - mod + (M_HPPA_SIZE_INT - mod - size);
	  base[i] = offset & (~(M_HPPA_SIZE_LONG - 1));
#if 0
	  fprintf (stderr, "type = %d, base = %d, bit_offset = %d\n",
		   type[i].type, base[i], bit_offset[i]);
#endif
	}
      else
	{
	  base[i] = offset;
	  bit_offset[i] = 0;
#if 0
	  fprintf (stderr, "type = %d, base = %d, bit_offset = %d\n",
		   type[i].type, base[i], bit_offset[i]);
#endif

	}

      offset += size;		/* allocate space */
    }
}

int
M_hppa_struct_align (int n, _M_Type * type)
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
  if (max < M_HPPA_ALIGN_CHAR)
    max = M_HPPA_ALIGN_CHAR;
  return max;
}

int
M_hppa_struct_size (int n, _M_Type * type, int struct_align)
{
  int i, offset;
  int mod, size, align, max_align, mod_word;
  offset = 0;			/* assume initially aligned */
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
      mod_word = offset % M_HPPA_SIZE_INT;
      if (type[i].type == M_TYPE_BIT_CHAR)
	{
	  if ((mod_word + size) > M_HPPA_SIZE_INT)
	    offset += (M_HPPA_SIZE_INT - mod_word);
	  if (M_HPPA_ALIGN_CHAR > max_align)
	    max_align = M_HPPA_ALIGN_CHAR;
	}
      else if (type[i].type == M_TYPE_BIT_SHORT)
	{
	  if ((mod_word + size) > M_HPPA_SIZE_INT)
	    offset += (M_HPPA_SIZE_INT - mod_word);
	  if (M_HPPA_ALIGN_SHORT > max_align)
	    max_align = M_HPPA_ALIGN_SHORT;
	}
      else if (type[i].type == M_TYPE_BIT_LONG)
	{
	  if ((mod_word + size) > M_HPPA_SIZE_INT)
	    offset += (M_HPPA_SIZE_INT - mod_word);
	  if (M_HPPA_ALIGN_LONG > max_align)
	    max_align = M_HPPA_ALIGN_LONG;
	}
      mod = offset % align;	/* align to what the field */
      if (mod != 0)		/* needs to start from */
	offset += (align - mod);

      offset += size;
    }
  /*
   * align to at least byte boundary.
   */
  if (max_align < M_HPPA_ALIGN_CHAR)
    max_align = M_HPPA_ALIGN_CHAR;
  /* enforce max. alignment */
  mod = offset % max_align;
  if (mod != 0)
    offset += (max_align - mod);
  return offset;
}


int
M_hppa_layout_fnvar (List param_list, char **base_macro, int *pcount,
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

  size = M_hppa_fnvar_layout (cnt, type, offset, mode, reg, paddr,
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

/*--------------------------------------------------------------------------*/
int
M_hppa_fnvar_layout (int n, _M_Type * type, long int *offset, int *mode,
		     int *reg, int *paddr, char **macro,
		     int *su_sreg, int *su_ereg,
		     int *pcount, int is_st, int purpose)
					/* need to return structure */
{
  int i, max_align, off, rg;
  int size, align, mod, tp;


  switch (purpose)
    {
    case M_GET_FNVAR:
      *macro = "$IP";
      break;
    case M_PUT_FNVAR:
      *macro = "$OP";
      break;
    case M_DONT_CARE_FNVAR:
    default:
      M_assert (0, "M_fnvar_layout: unknown purpose");
    }

  max_align = M_HPPA_ALIGN_MAX;
  rg = 0;
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
	  if (rg < M_HPPA_MAX_FNVAR_REG)
	    {
	      mode[i] = M_THRU_REGISTER;
	      reg[i] = (rg)++ + M_HPPA_INT_BASE;
	    }
	  else
	    {
	      mode[i] = M_THRU_MEMORY;
	      reg[i] = -1;
	    }
	  break;

	case M_TYPE_FLOAT:
	  if (rg < M_HPPA_MAX_FNVAR_REG)
	    {
	      mode[i] = M_THRU_REGISTER;
	      reg[i] = (rg)++ + M_HPPA_FLT_BASE;
	    }
	  else
	    {
	      mode[i] = M_THRU_MEMORY;
	      reg[i] = -1;
	    }
	  break;

	case M_TYPE_DOUBLE:
	  if (rg < M_HPPA_MAX_FNVAR_REG)
	    {
	      if (rg == 0 || rg == 2)
		{
		  mode[i] = M_THRU_REGISTER;
		  reg[i] = rg + 1 + M_HPPA_FLT_BASE;
		  rg += 2;
		}
	      else if (rg == 1)
		{
		  mode[i] = M_THRU_REGISTER;
		  reg[i] = 3 + M_HPPA_FLT_BASE;
		  rg += 3;
		}
	      else
		{
		  mode[i] = M_THRU_MEMORY;
		  reg[i] = -1;
		  rg += 3;	/* Rest of parms go in memory -JCG 11/16/98 */
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
	  if (rg < M_HPPA_MAX_FNVAR_REG)
	    {
	      printf ("hppa: M_INDIRECT_THRU_REGISTER\n");
	      mode[i] = M_INDIRECT_THRU_REGISTER;
	      reg[i] = (rg)++ + M_HPPA_INT_BASE;
	    }
	  else
	    {
	      if (size <= M_HPPA_SMALL_STRUCT_MAX)
		{
		  printf ("hppa: M_THRU_MEMORY\n");
		  mode[i] = M_THRU_MEMORY;
		}
	      else
		{
		  printf ("hppa:  M_INDIRECT_THRU_MEMORY\n");
		  mode[i] = M_INDIRECT_THRU_MEMORY;
		}
	      reg[i] = -1;
	    }
	  break;
#if 0
/* IMPACT does not allow passing of structures via registers, therefore */
/* where structures are concerned, HP's calling convention is ignored   */
	  size = type[i].size;
	  if (size <= M_HPPA_SIZE_INT)
	    {
	      if (rg < M_HPPA_MAX_FNVAR_REG)
		{
		  mode[i] = M_THRU_REGISTER;
		  reg[i] = (rg)++ + M_HPPA_INT_BASE;
		}
	      else
		{
		  mode[i] = M_THRU_MEMORY;
		  reg[i] = -1;
		}
	    }
	  else if (size <= M_HPPA_SMALL_STRUCT_MAX)
	    {
	      if ((rg == 0) || (rg == 2))
		{
		  mode[i] = M_THRU_REGISTER;
		  reg[i] = rg + M_HPPA_INT_BASE;
		  rg += 2;
		}
	      else
		{
		  mode[i] = M_THRU_MEMORY;
		  reg[i] = -1;
		}
	      /*
	         else
	         {
	         if (rg < M_HPPA_MAX_FNVAR_REG)
	         {
	         mode[i] = M_INDIRECT_THRU_REGISTER;
	         reg[i] = (rg)++ + M_HPPA_INT_BASE;
	         }
	         else
	         {
	         mode[i] = M_INDIRECT_THRU_MEMORY;
	         reg[i] = -1;
	         }
	         }
	       */
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
	  /* make sure correct size and alignment is used for struct/union */
	  /* passed indirectly thru registers                              */
	  /* if IMPACT allowed structures to be passed via registers, this */
	  /* would not have to be done                                     */
	  if (mode[i] == M_INDIRECT_THRU_REGISTER)
	    {
	      size = M_HPPA_SIZE_INT;
	      align = M_HPPA_ALIGN_INT;
	    }
	}
      if (align >= M_HPPA_SMALL_STRUCT_MAX && type[i].type != M_TYPE_DOUBLE)
	/* anything larger than a 64-bit structure is passed */
	/* indirectly thru memory                            */
	align = M_HPPA_ALIGN_INT;
      else if (align < M_HPPA_ALIGN_INT)
	/* anything smaller that 32-bits is passed as 32-bits */
	align = M_HPPA_ALIGN_INT;

      mod = off % align;

      /* place the offset pointer to the boundary of the appropriate */
      /* data size                                                   */
      if (mod != 0)
	off += (align - mod);

      /* now increment the offset to point to the actual location    */
      /* for this parameter                                          */
      off += size;

      offset[i] = -off;
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
	   (tp == M_TYPE_STRUCT)) && (size <= M_HPPA_SMALL_STRUCT_MAX))
	{

	  align = M_HPPA_ALIGN_MAX;	/* must align to a double boundry */

	  mod = off % align;

	  if (mod != 0)
	    off += (align - mod);

	  off += size;

	  printf ("1 size %d off %d", size, off);
	  paddr[i] = -off;
	  printf ("1 paddr[%d] = %d\n", i, paddr[i]);
	}
    }

  /* now large ones */
  for (i = 0; i < n; i++)
    {
      tp = type[i].type;

      size = type[i].size;

      if (((tp == M_TYPE_UNION) ||
	   (tp == M_TYPE_STRUCT)) && (size > M_HPPA_SMALL_STRUCT_MAX))
	{

	  align = M_HPPA_ALIGN_MAX;	/* must align to a double boundry */

	  mod = off % align;

	  if (mod != 0)
	    off += (align - mod);

	  off += size;

	  printf ("2 size %d off %d", size, off);
	  paddr[i] = -off;
	  printf ("2 paddr[%d] = %d\n", i, paddr[i]);
	}
    }

  *pcount = rg;
  return off;			/* the total size needed */
}

/*--------------------------------------------------------------------------*/
int
M_hppa_lvar_layout (int n, _M_Type * type, long int *offset,
		    char **base_macro)
{
  int i, max_align, off;
  int size, align, mod, tp;
  /*
   *  the LOCAL section must be max. aligned initially
   */
  max_align = M_HPPA_ALIGN_MAX;
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
  return off;			/* the total size needed */
}

/*--------------------------------------------------------------------------*/
int
M_hppa_no_short_int (void)
{
  return (M_HPPA_SIZE_SHORT == M_HPPA_SIZE_INT);
}
/*--------------------------------------------------------------------------*/
void
M_hppa_cb_label_name (char *fn, int cb, char *line, int len)
{
  sprintf (line, "cb%d%s", cb, fn);
}
/*--------------------------------------------------------------------------*/
int
M_hppa_is_cb_label (char *label, char *fn, int *cb)
{
  return (sscanf (label, "cb%d%s", cb, fn) == 2);
}
/*--------------------------------------------------------------------------*/
void
M_hppa_jumptbl_label_name (char *fn, int tbl_id, char *line, int len)
{
  sprintf (line, "_$%s%s%d", fn, M_JUMPTBL_BASE_NAME, tbl_id);
}
/*--------------------------------------------------------------------------*/
/* Format for hppa is: _$%sM_JUMPTBL_BASE_NAME%d, where %s is the func name */
int
M_hppa_is_jumptbl_label (char *label, char *fn, int *tbl_id)
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
/*--------------------------------------------------------------------------*/
int
M_hppa_structure_pointer (int purpose)
{
  return M_HPPA_RET_ST;
}
/*--------------------------------------------------------------------------*/
int
M_hppa_return_register (int type, int purpose)
{
  switch (type)
    {
    case M_TYPE_INT:
      return M_HPPA_RET_I32;
    case M_TYPE_LONG:
      return M_HPPA_RET_I32;
    case M_TYPE_FLOAT:
      return M_HPPA_RET_F;
    case M_TYPE_DOUBLE:
      return M_HPPA_RET_F;
    default:
      return M_HPPA_RET_I32;
    }
}
/*--------------------------------------------------------------------------*/
/*
char *M_hppa_fn_label_name(label)
*/
char *
M_hppa_fn_label_name (char *label, int (*is_func) (char *is_func_label))
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
M_hppa_fn_name_from_label (char *label)
{
  if (!strncmp (label, "_$fn", 4))
    return (label + 4);
  else
    return (label);
}

/*--------------------------------------------------------------------------*/
int
M_hppa_fragile_macro (int macro_value)
{
  switch (M_model)
    {
    case M_HP_PA_1_0:
    case M_HP_PA_1_1:
    case M_HP_PA_7100:
    case M_HP_PA_X:
    case M_HP_PLAYDOH_LCODE:
    case M_HP_PLAYDOH_MCODE:
      switch (macro_value)
	{
	case HPPA_MAC_ZERO:
	case HPPA_MAC_FZERO:
	case HPPA_MAC_DP:	/* added JCG 6/28/95 */
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
      M_assert (0, "M_hppa_fragile_macro:  Illegal model specified!");
      return (0);
    }
}
/*--------------------------------------------------------------------------*/
Set M_hppa_fragile_macro_set ()
{
  switch (M_model)
    {
    case M_HP_PA_1_0:
    case M_HP_PA_1_1:
    case M_HP_PA_7100:
    case M_HP_PA_X:
    case M_HP_PLAYDOH_LCODE:
    case M_HP_PLAYDOH_MCODE:
      {
	if (Set_hppa_fragile_macro)
	  {
	    return Set_hppa_fragile_macro;
	  }
	Set_hppa_fragile_macro = Set_add (Set_hppa_fragile_macro, L_MAC_P0);
	Set_hppa_fragile_macro = Set_add (Set_hppa_fragile_macro, L_MAC_P1);
	Set_hppa_fragile_macro = Set_add (Set_hppa_fragile_macro, L_MAC_P2);
	Set_hppa_fragile_macro = Set_add (Set_hppa_fragile_macro, L_MAC_P3);
	Set_hppa_fragile_macro = Set_add (Set_hppa_fragile_macro, L_MAC_P4);
	Set_hppa_fragile_macro = Set_add (Set_hppa_fragile_macro, L_MAC_P5);
	Set_hppa_fragile_macro = Set_add (Set_hppa_fragile_macro, L_MAC_P6);
	Set_hppa_fragile_macro = Set_add (Set_hppa_fragile_macro, L_MAC_P7);
	Set_hppa_fragile_macro = Set_add (Set_hppa_fragile_macro, L_MAC_P8);
	Set_hppa_fragile_macro = Set_add (Set_hppa_fragile_macro, L_MAC_P9);
	Set_hppa_fragile_macro = Set_add (Set_hppa_fragile_macro, L_MAC_P10);
	Set_hppa_fragile_macro = Set_add (Set_hppa_fragile_macro, L_MAC_P11);
	Set_hppa_fragile_macro = Set_add (Set_hppa_fragile_macro, L_MAC_P13);
	Set_hppa_fragile_macro = Set_add (Set_hppa_fragile_macro, L_MAC_P14);
	Set_hppa_fragile_macro = Set_add (Set_hppa_fragile_macro, L_MAC_P15);
	Set_hppa_fragile_macro = Set_add (Set_hppa_fragile_macro, L_MAC_P16);
	Set_hppa_fragile_macro = Set_add (Set_hppa_fragile_macro, L_MAC_P17);
	Set_hppa_fragile_macro = Set_add (Set_hppa_fragile_macro, L_MAC_P18);
	Set_hppa_fragile_macro = Set_add (Set_hppa_fragile_macro, L_MAC_P19);
	Set_hppa_fragile_macro = Set_add (Set_hppa_fragile_macro, L_MAC_P20);
	Set_hppa_fragile_macro = Set_add (Set_hppa_fragile_macro, L_MAC_P21);
	Set_hppa_fragile_macro = Set_add (Set_hppa_fragile_macro, L_MAC_P22);
	Set_hppa_fragile_macro = Set_add (Set_hppa_fragile_macro, L_MAC_P23);
	Set_hppa_fragile_macro = Set_add (Set_hppa_fragile_macro, L_MAC_P24);
	Set_hppa_fragile_macro = Set_add (Set_hppa_fragile_macro, L_MAC_P25);
	Set_hppa_fragile_macro = Set_add (Set_hppa_fragile_macro, L_MAC_P26);
	Set_hppa_fragile_macro = Set_add (Set_hppa_fragile_macro, L_MAC_P27);
	Set_hppa_fragile_macro = Set_add (Set_hppa_fragile_macro, L_MAC_P28);
	Set_hppa_fragile_macro = Set_add (Set_hppa_fragile_macro, L_MAC_P29);
	Set_hppa_fragile_macro = Set_add (Set_hppa_fragile_macro, L_MAC_P30);
	Set_hppa_fragile_macro = Set_add (Set_hppa_fragile_macro, L_MAC_P31);
	Set_hppa_fragile_macro = Set_add (Set_hppa_fragile_macro, L_MAC_P32);
	Set_hppa_fragile_macro = Set_add (Set_hppa_fragile_macro, L_MAC_P33);
	Set_hppa_fragile_macro = Set_add (Set_hppa_fragile_macro, L_MAC_P34);
	Set_hppa_fragile_macro = Set_add (Set_hppa_fragile_macro, L_MAC_P35);
	Set_hppa_fragile_macro = Set_add (Set_hppa_fragile_macro, L_MAC_P36);
	Set_hppa_fragile_macro = Set_add (Set_hppa_fragile_macro, L_MAC_P37);
	Set_hppa_fragile_macro = Set_add (Set_hppa_fragile_macro, L_MAC_P38);
	Set_hppa_fragile_macro = Set_add (Set_hppa_fragile_macro, L_MAC_P39);
	Set_hppa_fragile_macro = Set_add (Set_hppa_fragile_macro, L_MAC_P40);
	Set_hppa_fragile_macro = Set_add (Set_hppa_fragile_macro, L_MAC_P41);
	Set_hppa_fragile_macro = Set_add (Set_hppa_fragile_macro, L_MAC_P42);
	Set_hppa_fragile_macro = Set_add (Set_hppa_fragile_macro, L_MAC_P43);
	Set_hppa_fragile_macro = Set_add (Set_hppa_fragile_macro, L_MAC_P44);
	Set_hppa_fragile_macro = Set_add (Set_hppa_fragile_macro, L_MAC_P45);
	Set_hppa_fragile_macro = Set_add (Set_hppa_fragile_macro, L_MAC_P46);
	Set_hppa_fragile_macro = Set_add (Set_hppa_fragile_macro, L_MAC_P47);
	Set_hppa_fragile_macro = Set_add (Set_hppa_fragile_macro, L_MAC_P48);
	Set_hppa_fragile_macro = Set_add (Set_hppa_fragile_macro, L_MAC_P49);
	Set_hppa_fragile_macro = Set_add (Set_hppa_fragile_macro, L_MAC_P50);
	Set_hppa_fragile_macro = Set_add (Set_hppa_fragile_macro, L_MAC_P51);
	Set_hppa_fragile_macro = Set_add (Set_hppa_fragile_macro, L_MAC_P52);
	Set_hppa_fragile_macro = Set_add (Set_hppa_fragile_macro, L_MAC_P53);
	Set_hppa_fragile_macro = Set_add (Set_hppa_fragile_macro, L_MAC_P54);
	Set_hppa_fragile_macro = Set_add (Set_hppa_fragile_macro, L_MAC_P55);
	Set_hppa_fragile_macro = Set_add (Set_hppa_fragile_macro, L_MAC_P56);
	Set_hppa_fragile_macro = Set_add (Set_hppa_fragile_macro, L_MAC_P57);
	Set_hppa_fragile_macro = Set_add (Set_hppa_fragile_macro, L_MAC_P58);
	Set_hppa_fragile_macro = Set_add (Set_hppa_fragile_macro, L_MAC_P59);
	Set_hppa_fragile_macro = Set_add (Set_hppa_fragile_macro, L_MAC_P60);
	Set_hppa_fragile_macro = Set_add (Set_hppa_fragile_macro, L_MAC_P61);
	Set_hppa_fragile_macro = Set_add (Set_hppa_fragile_macro, L_MAC_P62);
	Set_hppa_fragile_macro = Set_add (Set_hppa_fragile_macro, L_MAC_P63);
	Set_hppa_fragile_macro = Set_add (Set_hppa_fragile_macro, L_MAC_P64);
	Set_hppa_fragile_macro = Set_add (Set_hppa_fragile_macro, L_MAC_RS);
	Set_hppa_fragile_macro =
	  Set_add (Set_hppa_fragile_macro, L_MAC_RET_TYPE);
	Set_hppa_fragile_macro =
	  Set_add (Set_hppa_fragile_macro, L_MAC_TR_PTR);
	Set_hppa_fragile_macro =
	  Set_add (Set_hppa_fragile_macro, L_MAC_TR_MARK);
	Set_hppa_fragile_macro =
	  Set_add (Set_hppa_fragile_macro, L_MAC_TR_TEMP);
	Set_hppa_fragile_macro =
	  Set_add (Set_hppa_fragile_macro, L_MAC_PRED_ALL);
	Set_hppa_fragile_macro =
	  Set_add (Set_hppa_fragile_macro, L_MAC_SAFE_MEM);
	Set_hppa_fragile_macro =
	  Set_add (Set_hppa_fragile_macro, L_MAC_TM_TYPE);
	Set_hppa_fragile_macro =
	  Set_add (Set_hppa_fragile_macro, HPPA_MAC_TEMPREG);
	Set_hppa_fragile_macro =
	  Set_add (Set_hppa_fragile_macro, HPPA_MAC_RETADDR);
	Set_hppa_fragile_macro =
	  Set_add (Set_hppa_fragile_macro, HPPA_MAC_MILLI_RET_VALUE);
	Set_hppa_fragile_macro =
	  Set_add (Set_hppa_fragile_macro, HPPA_MAC_MILLI_RETADDR);
	Set_hppa_fragile_macro =
	  Set_add (Set_hppa_fragile_macro, HPPA_MAC_LEAF);
	Set_hppa_fragile_macro =
	  Set_add (Set_hppa_fragile_macro, HPPA_MAC_ALLOC);
	Set_hppa_fragile_macro =
	  Set_add (Set_hppa_fragile_macro, HPPA_MAC_CALLEE_I);
	Set_hppa_fragile_macro =
	  Set_add (Set_hppa_fragile_macro, HPPA_MAC_CALLEE_F);
	Set_hppa_fragile_macro =
	  Set_add (Set_hppa_fragile_macro, HPPA_MAC_CONV_LOC);
	Set_hppa_fragile_macro =
	  Set_add (Set_hppa_fragile_macro, HPPA_MAC_CONV_OFF);
	Set_hppa_fragile_macro =
	  Set_add (Set_hppa_fragile_macro, HPPA_MAC_SAR);
	Set_hppa_fragile_macro =
	  Set_add (Set_hppa_fragile_macro, HPPA_MAC_TRUE_SP);
	Set_hppa_fragile_macro =
	  Set_add (Set_hppa_fragile_macro, HPPA_MAC_DYNCALL);
	Set_hppa_fragile_macro =
	  Set_add (Set_hppa_fragile_macro, HPPA_MAC_FLOAT_CBIT);
	Set_hppa_fragile_macro =
	  Set_add (Set_hppa_fragile_macro, HPPA_MAC_SWAP_PTR);
	Set_hppa_fragile_macro =
	  Set_add (Set_hppa_fragile_macro, HPPA_MAC_SR3);
	Set_hppa_fragile_macro =
	  Set_add (Set_hppa_fragile_macro, HPPA_MAC_SR5);
	/* These do not need to be analyzed:
	   Set_hppa_fragile_macro = 
	   Set_add(Set_hppa_fragile_macro, HPPA_MAC_PLAYDOH_GPR);
	   Set_hppa_fragile_macro = 
	   Set_add(Set_hppa_fragile_macro, HPPA_MAC_PLAYDOH_GPM);
	   Set_hppa_fragile_macro = 
	   Set_add(Set_hppa_fragile_macro, HPPA_MAC_PLAYDOH_SRC0);
	   Set_hppa_fragile_macro = 
	   Set_add(Set_hppa_fragile_macro, HPPA_MAC_PLAYDOH_SRC1);
	   Set_hppa_fragile_macro = 
	   Set_add(Set_hppa_fragile_macro, HPPA_MAC_PLAYDOH_SRC2);
	   Set_hppa_fragile_macro = 
	   Set_add(Set_hppa_fragile_macro, HPPA_MAC_PLAYDOH_SRC3);
	   Set_hppa_fragile_macro = 
	   Set_add(Set_hppa_fragile_macro, HPPA_MAC_PLAYDOH_PRED0);
	   Set_hppa_fragile_macro = 
	   Set_add(Set_hppa_fragile_macro, HPPA_MAC_PLAYDOH_PRED1);
	   Set_hppa_fragile_macro = 
	   Set_add(Set_hppa_fragile_macro, HPPA_MAC_PLAYDOH_PSRC0);
	   Set_hppa_fragile_macro = 
	   Set_add(Set_hppa_fragile_macro, HPPA_MAC_PLAYDOH_PDST0);
	   Set_hppa_fragile_macro = 
	   Set_add(Set_hppa_fragile_macro, HPPA_MAC_PLAYDOH_PDST1);
	   Set_hppa_fragile_macro = 
	   Set_add(Set_hppa_fragile_macro, HPPA_MAC_PLAYDOH_DEST0);
	   Set_hppa_fragile_macro = 
	   Set_add(Set_hppa_fragile_macro, HPPA_MAC_PLAYDOH_DEST1);
	   Set_hppa_fragile_macro = 
	   Set_add(Set_hppa_fragile_macro, HPPA_MAC_PLAYDOH_BRTARGET);
	   Set_hppa_fragile_macro = 
	   Set_add(Set_hppa_fragile_macro, HPPA_MAC_PLAYDOH_BRTMP);
	   Set_hppa_fragile_macro = 
	   Set_add(Set_hppa_fragile_macro, HPPA_MAC_PLAYDOH_FSRC0);
	   Set_hppa_fragile_macro = 
	   Set_add(Set_hppa_fragile_macro, HPPA_MAC_PLAYDOH_FSRC1);
	   Set_hppa_fragile_macro = 
	   Set_add(Set_hppa_fragile_macro, HPPA_MAC_PLAYDOH_FSRC2);
	   Set_hppa_fragile_macro = 
	   Set_add(Set_hppa_fragile_macro, HPPA_MAC_PLAYDOH_FDEST0);
	   Set_hppa_fragile_macro = 
	   Set_add(Set_hppa_fragile_macro, HPPA_MAC_PLAYDOH_FONE);
	   Set_hppa_fragile_macro = 
	   Set_add(Set_hppa_fragile_macro, HPPA_MAC_PLAYDOH_PRED_FALSE);
	   Set_hppa_fragile_macro = 
	   Set_add(Set_hppa_fragile_macro, HPPA_MAC_PLAYDOH_PRED_TRUE);
	   Set_hppa_fragile_macro = 
	   Set_add(Set_hppa_fragile_macro, HPPA_MAC_PLAYDOH_LC);
	   Set_hppa_fragile_macro = 
	   Set_add(Set_hppa_fragile_macro, HPPA_MAC_PLAYDOH_ESC);
	   Set_hppa_fragile_macro = 
	   Set_add(Set_hppa_fragile_macro, HPPA_MAC_PLAYDOH_RRB);
	   Set_hppa_fragile_macro = 
	   Set_add(Set_hppa_fragile_macro, HPPA_MAC_PLAYDOH_PRED_ALL_ROT);
	   Set_hppa_fragile_macro = 
	   Set_add(Set_hppa_fragile_macro, HPPA_MAC_PLAYDOH_PRED_ALL_STATIC);
	   Set_hppa_fragile_macro = 
	   Set_add(Set_hppa_fragile_macro, HPPA_MAC_PLAYDOH_PV_0);
	   Set_hppa_fragile_macro = 
	   Set_add(Set_hppa_fragile_macro, HPPA_MAC_PLAYDOH_PV_1);
	   Set_hppa_fragile_macro = 
	   Set_add(Set_hppa_fragile_macro, HPPA_MAC_PLAYDOH_PV_2);
	   Set_hppa_fragile_macro = 
	   Set_add(Set_hppa_fragile_macro, HPPA_MAC_PLAYDOH_PV_3);
	   Set_hppa_fragile_macro = 
	   Set_add(Set_hppa_fragile_macro, HPPA_MAC_PLAYDOH_PV_4);
	   Set_hppa_fragile_macro = 
	   Set_add(Set_hppa_fragile_macro, HPPA_MAC_PLAYDOH_PV_5);
	   Set_hppa_fragile_macro =  
	   Set_add(Set_hppa_fragile_macro, HPPA_MAC_PLAYDOH_PV_6);
	   Set_hppa_fragile_macro = 
	   Set_add(Set_hppa_fragile_macro, HPPA_MAC_PLAYDOH_PV_7);
	 */
	return Set_hppa_fragile_macro;
      }
    default:
      M_assert (0, "M_hppa_dataflow_macro:  Illegal model specified!");
      return (0);
    }
}

/*--------------------------------------------------------------------------*/
int
M_hppa_dataflow_macro (int id)
{
  switch (M_model)
    {
    case M_HP_PA_1_0:
    case M_HP_PA_1_1:
    case M_HP_PA_7100:
    case M_HP_PA_X:
    case M_HP_PLAYDOH_LCODE:
    case M_HP_PLAYDOH_MCODE:
      if (id >= HPPA_MAC_PLAYDOH_GPR && id <= HPPA_MAC_LAST)
	return 0;
      else
	return ((id >= L_MAC_P0 && id <= L_MAC_P64) || (id >= L_MAC_LAST));
    default:
      M_assert (0, "M_hppa_dataflow_macro:  Illegal model specified!");
      return (0);
    }
}

/*--------------------------------------------------------------------------*/
int
M_hppa_subroutine_call (int opc)
{
  switch (M_model)
    {
    case M_HP_PA_1_0:
      return ((opc == Lop_JSR) || (opc == Lop_JSR_FS) ||
	      (opc == Lop_MUL) || (opc == Lop_MUL_U) ||
	      (opc == Lop_DIV) || (opc == Lop_DIV_U) ||
	      (opc == Lop_REM) || (opc == Lop_REM_U));
    case M_HP_PA_1_1:
    case M_HP_PA_7100:
    case M_HP_PA_X:
    case M_HP_PLAYDOH_LCODE:
    case M_HP_PLAYDOH_MCODE:
      return ((opc == Lop_JSR)
	      || (opc ==
		  Lop_JSR_FS)
	      || (opc ==
		  Lop_MUL)
	      || (opc ==
		  Lop_MUL_U)
	      || (opc ==
		  Lop_DIV)
	      || (opc == Lop_DIV_U) || (opc == Lop_REM) || (opc == Lop_REM_U));
    default:
      M_assert (0, "M_hppa_subroutine_call:  Illegal model specified!");
      return (0);
    }
}
/*--------------------------------------------------------------------------*/
/*
 * Declare code generator specific macro registers to the front end parser.
 */
void
M_define_macros_hppa (STRING_Symbol_Table * sym_tbl)
{
  M_add_symbol (sym_tbl, "fr0", HPPA_MAC_FZERO);
  M_add_symbol (sym_tbl, "gr0", HPPA_MAC_ZERO);
  M_add_symbol (sym_tbl, "gr1", HPPA_MAC_TEMPREG);
  M_add_symbol (sym_tbl, "gr2", HPPA_MAC_RETADDR);
  M_add_symbol (sym_tbl, "gr3", HPPA_MAC_SWAP_PTR);
  M_add_symbol (sym_tbl, "gr22", HPPA_MAC_DYNCALL);
  M_add_symbol (sym_tbl, "gr27", HPPA_MAC_DP);
  M_add_symbol (sym_tbl, "gr29", HPPA_MAC_MILLI_RET_VALUE);
  M_add_symbol (sym_tbl, "gr31", HPPA_MAC_MILLI_RETADDR);
  M_add_symbol (sym_tbl, "sar", HPPA_MAC_SAR);
  M_add_symbol (sym_tbl, "$true_sp", HPPA_MAC_TRUE_SP);
  M_add_symbol (sym_tbl, "fcbit", HPPA_MAC_FLOAT_CBIT);
  M_add_symbol (sym_tbl, "sr3", HPPA_MAC_SR3);
  M_add_symbol (sym_tbl, "sr5", HPPA_MAC_SR5);
  /* 1 if leaf function, 0 if non-leaf */
  M_add_symbol (sym_tbl, "$leaf", HPPA_MAC_LEAF);
  /* total alloc requirements */
  M_add_symbol (sym_tbl, "$alloc_size", HPPA_MAC_ALLOC);
  /* number of integer and float callee saved registers used */
  M_add_symbol (sym_tbl, "$callee_i_regs", HPPA_MAC_CALLEE_I);
  M_add_symbol (sym_tbl, "$callee_f_regs", HPPA_MAC_CALLEE_F);
  M_add_symbol (sym_tbl, "gpr", HPPA_MAC_PLAYDOH_GPR);
  M_add_symbol (sym_tbl, "gpm", HPPA_MAC_PLAYDOH_GPM);
  M_add_symbol (sym_tbl, "src0", HPPA_MAC_PLAYDOH_SRC0);
  M_add_symbol (sym_tbl, "src1", HPPA_MAC_PLAYDOH_SRC1);
  M_add_symbol (sym_tbl, "src2", HPPA_MAC_PLAYDOH_SRC2);
  M_add_symbol (sym_tbl, "src3", HPPA_MAC_PLAYDOH_SRC3);
  M_add_symbol (sym_tbl, "pred0", HPPA_MAC_PLAYDOH_PRED0);
  M_add_symbol (sym_tbl, "pred1", HPPA_MAC_PLAYDOH_PRED1);
  M_add_symbol (sym_tbl, "psrc0", HPPA_MAC_PLAYDOH_PSRC0);
  M_add_symbol (sym_tbl, "pdst0", HPPA_MAC_PLAYDOH_PDST0);
  M_add_symbol (sym_tbl, "pdst1", HPPA_MAC_PLAYDOH_PDST1);
  M_add_symbol (sym_tbl, "dest0", HPPA_MAC_PLAYDOH_DEST0);
  M_add_symbol (sym_tbl, "dest1", HPPA_MAC_PLAYDOH_DEST1);
  M_add_symbol (sym_tbl, "brtarget", HPPA_MAC_PLAYDOH_BRTARGET);
  M_add_symbol (sym_tbl, "brtmp", HPPA_MAC_PLAYDOH_BRTMP);
  M_add_symbol (sym_tbl, "fpsrc0", HPPA_MAC_PLAYDOH_FSRC0);
  M_add_symbol (sym_tbl, "fpsrc1", HPPA_MAC_PLAYDOH_FSRC1);
  M_add_symbol (sym_tbl, "fpsrc2", HPPA_MAC_PLAYDOH_FSRC2);
  M_add_symbol (sym_tbl, "fpdest0", HPPA_MAC_PLAYDOH_FDEST0);
  M_add_symbol (sym_tbl, "fr1", HPPA_MAC_PLAYDOH_FONE);
  M_add_symbol (sym_tbl, "pr0", HPPA_MAC_PLAYDOH_PRED_FALSE);
  M_add_symbol (sym_tbl, "pr1", HPPA_MAC_PLAYDOH_PRED_TRUE);
  M_add_symbol (sym_tbl, "lc", HPPA_MAC_PLAYDOH_LC);
  M_add_symbol (sym_tbl, "esc", HPPA_MAC_PLAYDOH_ESC);
  M_add_symbol (sym_tbl, "rrb", HPPA_MAC_PLAYDOH_RRB);
  M_add_symbol (sym_tbl, "$pred_all_rot", HPPA_MAC_PLAYDOH_PRED_ALL_ROT);
  M_add_symbol (sym_tbl, "$pred_all_static",
		HPPA_MAC_PLAYDOH_PRED_ALL_STATIC);
  M_add_symbol (sym_tbl, "pv_0", HPPA_MAC_PLAYDOH_PV_0);
  M_add_symbol (sym_tbl, "pv_1", HPPA_MAC_PLAYDOH_PV_1);
  M_add_symbol (sym_tbl, "pv_2", HPPA_MAC_PLAYDOH_PV_2);
  M_add_symbol (sym_tbl, "pv_3", HPPA_MAC_PLAYDOH_PV_3);
  M_add_symbol (sym_tbl, "pv_4", HPPA_MAC_PLAYDOH_PV_4);
  M_add_symbol (sym_tbl, "pv_5", HPPA_MAC_PLAYDOH_PV_5);
  M_add_symbol (sym_tbl, "pv_6", HPPA_MAC_PLAYDOH_PV_6);
  M_add_symbol (sym_tbl, "pv_7", HPPA_MAC_PLAYDOH_PV_7);
}

char *
M_get_macro_name_hppa (int id)
{
  switch (id)
    {
    case HPPA_MAC_ZERO:
      return "gr0";
    case HPPA_MAC_TEMPREG:
      return "gr1";
    case HPPA_MAC_RETADDR:
      return "gr2";
    case HPPA_MAC_SWAP_PTR:
      return "gr3";
    case HPPA_MAC_DP:
      return "gr27";
    case HPPA_MAC_MILLI_RET_VALUE:
      return "gr29";
    case HPPA_MAC_MILLI_RETADDR:
      return "gr31";
    case HPPA_MAC_LEAF:
      return "$leaf";
    case HPPA_MAC_ALLOC:
      return "$alloc_size";
    case HPPA_MAC_SAR:
      return "sar";
    case HPPA_MAC_CALLEE_I:
      return "$callee_i_regs";
    case HPPA_MAC_CALLEE_F:
      return "$callee_f_regs";
    case HPPA_MAC_FZERO:
      return "fr0";
    case HPPA_MAC_TRUE_SP:
      return "$true_sp";
    case HPPA_MAC_DYNCALL:
      return "gr22";
    case HPPA_MAC_FLOAT_CBIT:
      return "fcbit";
    case HPPA_MAC_SR3:
      return "sr3";
    case HPPA_MAC_SR5:
      return "sr5";

    case HPPA_MAC_PLAYDOH_GPR:
      return "gpr";
    case HPPA_MAC_PLAYDOH_GPM:
      return "gpm";
    case HPPA_MAC_PLAYDOH_SRC0:
      return "src0";
    case HPPA_MAC_PLAYDOH_SRC1:
      return "src1";
    case HPPA_MAC_PLAYDOH_SRC2:
      return "src2";
    case HPPA_MAC_PLAYDOH_SRC3:
      return "src3";
    case HPPA_MAC_PLAYDOH_PRED0:
      return "pred0";
    case HPPA_MAC_PLAYDOH_PRED1:
      return "pred1";
    case HPPA_MAC_PLAYDOH_PSRC0:
      return "psrc0";
    case HPPA_MAC_PLAYDOH_PDST0:
      return "pdst0";
    case HPPA_MAC_PLAYDOH_PDST1:
      return "pdst1";
    case HPPA_MAC_PLAYDOH_DEST0:
      return "dest0";
    case HPPA_MAC_PLAYDOH_DEST1:
      return "dest1";
    case HPPA_MAC_PLAYDOH_BRTARGET:
      return "brtarget";
    case HPPA_MAC_PLAYDOH_BRTMP:
      return "brtmp";
    case HPPA_MAC_PLAYDOH_FSRC0:
      return "fpsrc0";
    case HPPA_MAC_PLAYDOH_FSRC1:
      return "fpsrc1";
    case HPPA_MAC_PLAYDOH_FSRC2:
      return "fpsrc2";
    case HPPA_MAC_PLAYDOH_FDEST0:
      return "fpdest0";

    case HPPA_MAC_PLAYDOH_FONE:
      return "fr1";
    case HPPA_MAC_PLAYDOH_PRED_FALSE:
      return "pr0";
    case HPPA_MAC_PLAYDOH_PRED_TRUE:
      return "pr1";
    case HPPA_MAC_PLAYDOH_LC:
      return "lc";
    case HPPA_MAC_PLAYDOH_ESC:
      return "esc";
    case HPPA_MAC_PLAYDOH_RRB:
      return "rrb";
    case HPPA_MAC_PLAYDOH_PRED_ALL_ROT:
      return "$pred_all_rot";
    case HPPA_MAC_PLAYDOH_PRED_ALL_STATIC:
      return "$pred_all_static";
    case HPPA_MAC_PLAYDOH_PV_0:
      return "pv_0";
    case HPPA_MAC_PLAYDOH_PV_1:
      return "pv_1";
    case HPPA_MAC_PLAYDOH_PV_2:
      return "pv_2";
    case HPPA_MAC_PLAYDOH_PV_3:
      return "pv_3";
    case HPPA_MAC_PLAYDOH_PV_4:
      return "pv_4";
    case HPPA_MAC_PLAYDOH_PV_5:
      return "pv_5";
    case HPPA_MAC_PLAYDOH_PV_6:
      return "pv_6";
    case HPPA_MAC_PLAYDOH_PV_7:
      return "pv_7";

    default:
      return "?";
    }
}

void
M_define_opcode_name_hppa (STRING_Symbol_Table * sym_tbl)
{
  M_add_symbol (sym_tbl, PAopcode_ADDIL, LHPPAop_ADDIL);
  M_add_symbol (sym_tbl, PAopcode_LDO, LHPPAop_LDO);
  M_add_symbol (sym_tbl, PAopcode_FMPYADD, LHPPAop_FMPYADD);
  M_add_symbol (sym_tbl, PAopcode_FMPYSUB, LHPPAop_FMPYSUB);
  M_add_symbol (sym_tbl, PAopcode_MTSP, LHPPAop_MTSP);
  M_add_symbol (sym_tbl, PAopcode_ZVDEP, LHPPAop_ZVDEP);
  M_add_symbol (sym_tbl, PAopcode_VSHD, LHPPAop_VSHD);
  M_add_symbol (sym_tbl, PAopcode_VEXTRS, LHPPAop_VEXTRS);
  M_add_symbol (sym_tbl, PAopcode_COMB_EQ_FWD, LHPPAop_COMB_EQ_FWD);
  M_add_symbol (sym_tbl, PAopcode_COMB_NE_FWD, LHPPAop_COMB_NE_FWD);
  M_add_symbol (sym_tbl, PAopcode_COMB_GT_FWD, LHPPAop_COMB_GT_FWD);
  M_add_symbol (sym_tbl, PAopcode_COMB_GE_FWD, LHPPAop_COMB_GE_FWD);
  M_add_symbol (sym_tbl, PAopcode_COMB_LT_FWD, LHPPAop_COMB_LT_FWD);
  M_add_symbol (sym_tbl, PAopcode_COMB_LE_FWD, LHPPAop_COMB_LE_FWD);
  M_add_symbol (sym_tbl, PAopcode_COMB_GT_U_FWD, LHPPAop_COMB_GT_U_FWD);
  M_add_symbol (sym_tbl, PAopcode_COMB_GE_U_FWD, LHPPAop_COMB_GE_U_FWD);
  M_add_symbol (sym_tbl, PAopcode_COMB_LT_U_FWD, LHPPAop_COMB_LT_U_FWD);
  M_add_symbol (sym_tbl, PAopcode_COMB_LE_U_FWD, LHPPAop_COMB_LE_U_FWD);
  M_add_symbol (sym_tbl, PAopcode_COMIB_EQ_FWD, LHPPAop_COMIB_EQ_FWD);
  M_add_symbol (sym_tbl, PAopcode_COMIB_NE_FWD, LHPPAop_COMIB_NE_FWD);
  M_add_symbol (sym_tbl, PAopcode_COMIB_GT_FWD, LHPPAop_COMIB_GT_FWD);
  M_add_symbol (sym_tbl, PAopcode_COMIB_GE_FWD, LHPPAop_COMIB_GE_FWD);
  M_add_symbol (sym_tbl, PAopcode_COMIB_LT_FWD, LHPPAop_COMIB_LT_FWD);
  M_add_symbol (sym_tbl, PAopcode_COMIB_LE_FWD, LHPPAop_COMIB_LE_FWD);
  M_add_symbol (sym_tbl, PAopcode_COMIB_GT_U_FWD, LHPPAop_COMIB_GT_U_FWD);
  M_add_symbol (sym_tbl, PAopcode_COMIB_GE_U_FWD, LHPPAop_COMIB_GE_U_FWD);
  M_add_symbol (sym_tbl, PAopcode_COMIB_LT_U_FWD, LHPPAop_COMIB_LT_U_FWD);
  M_add_symbol (sym_tbl, PAopcode_COMIB_LE_U_FWD, LHPPAop_COMIB_LE_U_FWD);
  M_add_symbol (sym_tbl, PAopcode_COMB_EQ_BWD, LHPPAop_COMB_EQ_BWD);
  M_add_symbol (sym_tbl, PAopcode_COMB_NE_BWD, LHPPAop_COMB_NE_BWD);
  M_add_symbol (sym_tbl, PAopcode_COMB_GT_BWD, LHPPAop_COMB_GT_BWD);
  M_add_symbol (sym_tbl, PAopcode_COMB_GE_BWD, LHPPAop_COMB_GE_BWD);
  M_add_symbol (sym_tbl, PAopcode_COMB_LT_BWD, LHPPAop_COMB_LT_BWD);
  M_add_symbol (sym_tbl, PAopcode_COMB_LE_BWD, LHPPAop_COMB_LE_BWD);
  M_add_symbol (sym_tbl, PAopcode_COMB_GT_U_BWD, LHPPAop_COMB_GT_U_BWD);
  M_add_symbol (sym_tbl, PAopcode_COMB_GE_U_BWD, LHPPAop_COMB_GE_U_BWD);
  M_add_symbol (sym_tbl, PAopcode_COMB_LT_U_BWD, LHPPAop_COMB_LT_U_BWD);
  M_add_symbol (sym_tbl, PAopcode_COMB_LE_U_BWD, LHPPAop_COMB_LE_U_BWD);
  M_add_symbol (sym_tbl, PAopcode_COMIB_EQ_BWD, LHPPAop_COMIB_EQ_BWD);
  M_add_symbol (sym_tbl, PAopcode_COMIB_NE_BWD, LHPPAop_COMIB_NE_BWD);
  M_add_symbol (sym_tbl, PAopcode_COMIB_GT_BWD, LHPPAop_COMIB_GT_BWD);
  M_add_symbol (sym_tbl, PAopcode_COMIB_GE_BWD, LHPPAop_COMIB_GE_BWD);
  M_add_symbol (sym_tbl, PAopcode_COMIB_LT_BWD, LHPPAop_COMIB_LT_BWD);
  M_add_symbol (sym_tbl, PAopcode_COMIB_LE_BWD, LHPPAop_COMIB_LE_BWD);
  M_add_symbol (sym_tbl, PAopcode_COMIB_GT_U_BWD, LHPPAop_COMIB_GT_U_BWD);
  M_add_symbol (sym_tbl, PAopcode_COMIB_GE_U_BWD, LHPPAop_COMIB_GE_U_BWD);
  M_add_symbol (sym_tbl, PAopcode_COMIB_LT_U_BWD, LHPPAop_COMIB_LT_U_BWD);
  M_add_symbol (sym_tbl, PAopcode_COMIB_LE_U_BWD, LHPPAop_COMIB_LE_U_BWD);
  M_add_symbol (sym_tbl, PAopcode_BB_0_FWD, LHPPAop_BB_0_FWD);
  M_add_symbol (sym_tbl, PAopcode_BB_1_FWD, LHPPAop_BB_1_FWD);
  M_add_symbol (sym_tbl, PAopcode_BB_0_BWD, LHPPAop_BB_0_BWD);
  M_add_symbol (sym_tbl, PAopcode_BB_1_BWD, LHPPAop_BB_1_BWD);
  M_add_symbol (sym_tbl, PAopcode_LDIL, LHPPAop_LDIL);
  M_add_symbol (sym_tbl, PAopcode_ADDIB_LT_FWD, LHPPAop_ADDIB_LT_FWD);
  M_add_symbol (sym_tbl, PAopcode_ADDIB_LT_BWD, LHPPAop_ADDIB_LT_BWD);
  M_add_symbol (sym_tbl, PAopcode_EXTRU, LHPPAop_EXTRU);
  M_add_symbol (sym_tbl, PAopcode_DEPI, LHPPAop_DEPI);
  M_add_symbol (sym_tbl, PAopcode_JSR_DYNCALL, LHPPAop_JSR_DYNCALL);

  M_add_symbol (sym_tbl, PAopcode_PRED_MOV, LHPPAop_PRED_MOV);
  M_add_symbol (sym_tbl, PAopcode_ZVDEPI, LHPPAop_ZVDEPI);

  M_add_symbol (sym_tbl, PAopcode_LD_UC_SV1, LHPPAop_LD_UC_SV1);
  M_add_symbol (sym_tbl, PAopcode_LD_UC_SC1, LHPPAop_LD_UC_SC1);
  M_add_symbol (sym_tbl, PAopcode_LD_UC_SC2, LHPPAop_LD_UC_SC2);
  M_add_symbol (sym_tbl, PAopcode_LD_UC_SC3, LHPPAop_LD_UC_SC3);

  M_add_symbol (sym_tbl, PAopcode_LD_PRE_UC_SV1, LHPPAop_LD_PRE_UC_SV1);
  M_add_symbol (sym_tbl, PAopcode_LD_PRE_UC_SC1, LHPPAop_LD_PRE_UC_SC1);
  M_add_symbol (sym_tbl, PAopcode_LD_PRE_UC_SC2, LHPPAop_LD_PRE_UC_SC2);
  M_add_symbol (sym_tbl, PAopcode_LD_PRE_UC_SC3, LHPPAop_LD_PRE_UC_SC3);

  M_add_symbol (sym_tbl, PAopcode_LD_POST_UC_SV1, LHPPAop_LD_POST_UC_SV1);
  M_add_symbol (sym_tbl, PAopcode_LD_POST_UC_SC1, LHPPAop_LD_POST_UC_SC1);
  M_add_symbol (sym_tbl, PAopcode_LD_POST_UC_SC2, LHPPAop_LD_POST_UC_SC2);
  M_add_symbol (sym_tbl, PAopcode_LD_POST_UC_SC3, LHPPAop_LD_POST_UC_SC3);
  M_add_symbol (sym_tbl, PAopcode_LD_C_SV1, LHPPAop_LD_C_SV1);
  M_add_symbol (sym_tbl, PAopcode_LD_C_SC1, LHPPAop_LD_C_SC1);
  M_add_symbol (sym_tbl, PAopcode_LD_C_SC2, LHPPAop_LD_C_SC2);
  M_add_symbol (sym_tbl, PAopcode_LD_C_SC3, LHPPAop_LD_C_SC3);

  M_add_symbol (sym_tbl, PAopcode_LD_PRE_C_SV1, LHPPAop_LD_PRE_C_SV1);
  M_add_symbol (sym_tbl, PAopcode_LD_PRE_C_SC1, LHPPAop_LD_PRE_C_SC1);
  M_add_symbol (sym_tbl, PAopcode_LD_PRE_C_SC2, LHPPAop_LD_PRE_C_SC2);
  M_add_symbol (sym_tbl, PAopcode_LD_PRE_C_SC3, LHPPAop_LD_PRE_C_SC3);

  M_add_symbol (sym_tbl, PAopcode_LD_POST_C_SV1, LHPPAop_LD_POST_C_SV1);
  M_add_symbol (sym_tbl, PAopcode_LD_POST_C_SC1, LHPPAop_LD_POST_C_SC1);
  M_add_symbol (sym_tbl, PAopcode_LD_POST_C_SC2, LHPPAop_LD_POST_C_SC2);
  M_add_symbol (sym_tbl, PAopcode_LD_POST_C_SC3, LHPPAop_LD_POST_C_SC3);


  M_add_symbol (sym_tbl, PAopcode_LD_UC2_SV1, LHPPAop_LD_UC2_SV1);
  M_add_symbol (sym_tbl, PAopcode_LD_UC2_SC1, LHPPAop_LD_UC2_SC1);
  M_add_symbol (sym_tbl, PAopcode_LD_UC2_SC2, LHPPAop_LD_UC2_SC2);
  M_add_symbol (sym_tbl, PAopcode_LD_UC2_SC3, LHPPAop_LD_UC2_SC3);

  M_add_symbol (sym_tbl, PAopcode_LD_PRE_UC2_SV1, LHPPAop_LD_PRE_UC2_SV1);
  M_add_symbol (sym_tbl, PAopcode_LD_PRE_UC2_SC1, LHPPAop_LD_PRE_UC2_SC1);
  M_add_symbol (sym_tbl, PAopcode_LD_PRE_UC2_SC2, LHPPAop_LD_PRE_UC2_SC2);
  M_add_symbol (sym_tbl, PAopcode_LD_PRE_UC2_SC3, LHPPAop_LD_PRE_UC2_SC3);

  M_add_symbol (sym_tbl, PAopcode_LD_POST_UC2_SV1, LHPPAop_LD_POST_UC2_SV1);
  M_add_symbol (sym_tbl, PAopcode_LD_POST_UC2_SC1, LHPPAop_LD_POST_UC2_SC1);
  M_add_symbol (sym_tbl, PAopcode_LD_POST_UC2_SC2, LHPPAop_LD_POST_UC2_SC2);
  M_add_symbol (sym_tbl, PAopcode_LD_POST_UC2_SC3, LHPPAop_LD_POST_UC2_SC3);

  M_add_symbol (sym_tbl, PAopcode_LD_C2_SV1, LHPPAop_LD_C2_SV1);
  M_add_symbol (sym_tbl, PAopcode_LD_C2_SC1, LHPPAop_LD_C2_SC1);
  M_add_symbol (sym_tbl, PAopcode_LD_C2_SC2, LHPPAop_LD_C2_SC2);
  M_add_symbol (sym_tbl, PAopcode_LD_C2_SC3, LHPPAop_LD_C2_SC3);

  M_add_symbol (sym_tbl, PAopcode_LD_PRE_C2_SV1, LHPPAop_LD_PRE_C2_SV1);
  M_add_symbol (sym_tbl, PAopcode_LD_PRE_C2_SC1, LHPPAop_LD_PRE_C2_SC1);
  M_add_symbol (sym_tbl, PAopcode_LD_PRE_C2_SC2, LHPPAop_LD_PRE_C2_SC2);
  M_add_symbol (sym_tbl, PAopcode_LD_PRE_C2_SC3, LHPPAop_LD_PRE_C2_SC3);

  M_add_symbol (sym_tbl, PAopcode_LD_POST_C2_SV1, LHPPAop_LD_POST_C2_SV1);
  M_add_symbol (sym_tbl, PAopcode_LD_POST_C2_SC1, LHPPAop_LD_POST_C2_SC1);
  M_add_symbol (sym_tbl, PAopcode_LD_POST_C2_SC2, LHPPAop_LD_POST_C2_SC2);
  M_add_symbol (sym_tbl, PAopcode_LD_POST_C2_SC3, LHPPAop_LD_POST_C2_SC3);

  M_add_symbol (sym_tbl, PAopcode_LD_I_SV1, LHPPAop_LD_I_SV1);
  M_add_symbol (sym_tbl, PAopcode_LD_I_SC1, LHPPAop_LD_I_SC1);
  M_add_symbol (sym_tbl, PAopcode_LD_I_SC2, LHPPAop_LD_I_SC2);
  M_add_symbol (sym_tbl, PAopcode_LD_I_SC3, LHPPAop_LD_I_SC3);

  M_add_symbol (sym_tbl, PAopcode_LD_PRE_I_SV1, LHPPAop_LD_PRE_I_SV1);
  M_add_symbol (sym_tbl, PAopcode_LD_PRE_I_SC1, LHPPAop_LD_PRE_I_SC1);
  M_add_symbol (sym_tbl, PAopcode_LD_PRE_I_SC2, LHPPAop_LD_PRE_I_SC2);
  M_add_symbol (sym_tbl, PAopcode_LD_PRE_I_SC3, LHPPAop_LD_PRE_I_SC3);

  M_add_symbol (sym_tbl, PAopcode_LD_POST_I_SV1, LHPPAop_LD_POST_I_SV1);
  M_add_symbol (sym_tbl, PAopcode_LD_POST_I_SC1, LHPPAop_LD_POST_I_SC1);
  M_add_symbol (sym_tbl, PAopcode_LD_POST_I_SC2, LHPPAop_LD_POST_I_SC2);
  M_add_symbol (sym_tbl, PAopcode_LD_POST_I_SC3, LHPPAop_LD_POST_I_SC3);

  M_add_symbol (sym_tbl, PAopcode_LD_F_SV1, LHPPAop_LD_F_SV1);
  M_add_symbol (sym_tbl, PAopcode_LD_F_SC1, LHPPAop_LD_F_SC1);
  M_add_symbol (sym_tbl, PAopcode_LD_F_SC2, LHPPAop_LD_F_SC2);
  M_add_symbol (sym_tbl, PAopcode_LD_F_SC3, LHPPAop_LD_F_SC3);

  M_add_symbol (sym_tbl, PAopcode_LD_PRE_F_SV1, LHPPAop_LD_PRE_F_SV1);
  M_add_symbol (sym_tbl, PAopcode_LD_PRE_F_SC1, LHPPAop_LD_PRE_F_SC1);
  M_add_symbol (sym_tbl, PAopcode_LD_PRE_F_SC2, LHPPAop_LD_PRE_F_SC2);
  M_add_symbol (sym_tbl, PAopcode_LD_PRE_F_SC3, LHPPAop_LD_PRE_F_SC3);

  M_add_symbol (sym_tbl, PAopcode_LD_POST_F_SV1, LHPPAop_LD_POST_F_SV1);
  M_add_symbol (sym_tbl, PAopcode_LD_POST_F_SC1, LHPPAop_LD_POST_F_SC1);
  M_add_symbol (sym_tbl, PAopcode_LD_POST_F_SC2, LHPPAop_LD_POST_F_SC2);
  M_add_symbol (sym_tbl, PAopcode_LD_POST_F_SC3, LHPPAop_LD_POST_F_SC3);

  M_add_symbol (sym_tbl, PAopcode_LD_F2_SV1, LHPPAop_LD_F2_SV1);
  M_add_symbol (sym_tbl, PAopcode_LD_F2_SC1, LHPPAop_LD_F2_SC1);
  M_add_symbol (sym_tbl, PAopcode_LD_F2_SC2, LHPPAop_LD_F2_SC2);
  M_add_symbol (sym_tbl, PAopcode_LD_F2_SC3, LHPPAop_LD_F2_SC3);

  M_add_symbol (sym_tbl, PAopcode_LD_PRE_F2_SV1, LHPPAop_LD_PRE_F2_SV1);
  M_add_symbol (sym_tbl, PAopcode_LD_PRE_F2_SC1, LHPPAop_LD_PRE_F2_SC1);
  M_add_symbol (sym_tbl, PAopcode_LD_PRE_F2_SC2, LHPPAop_LD_PRE_F2_SC2);
  M_add_symbol (sym_tbl, PAopcode_LD_PRE_F2_SC3, LHPPAop_LD_PRE_F2_SC3);

  M_add_symbol (sym_tbl, PAopcode_LD_POST_F2_SV1, LHPPAop_LD_POST_F2_SV1);
  M_add_symbol (sym_tbl, PAopcode_LD_POST_F2_SC1, LHPPAop_LD_POST_F2_SC1);
  M_add_symbol (sym_tbl, PAopcode_LD_POST_F2_SC2, LHPPAop_LD_POST_F2_SC2);
  M_add_symbol (sym_tbl, PAopcode_LD_POST_F2_SC3, LHPPAop_LD_POST_F2_SC3);

}

char *
M_get_opcode_name_hppa (int id)
{
  switch (id)
    {
    case LHPPAop_ADDIL:
      return (PAopcode_ADDIL);
    case LHPPAop_LDO:
      return (PAopcode_LDO);
    case LHPPAop_FMPYADD:
      return (PAopcode_FMPYADD);
    case LHPPAop_FMPYSUB:
      return (PAopcode_FMPYSUB);
    case LHPPAop_MTSP:
      return (PAopcode_MTSP);
    case LHPPAop_ZVDEP:
      return (PAopcode_ZVDEP);
    case LHPPAop_VSHD:
      return (PAopcode_VSHD);
    case LHPPAop_VEXTRS:
      return (PAopcode_VEXTRS);
    case LHPPAop_COMB_EQ_FWD:
      return (PAopcode_COMB_EQ_FWD);
    case LHPPAop_COMB_NE_FWD:
      return (PAopcode_COMB_NE_FWD);
    case LHPPAop_COMB_GT_FWD:
      return (PAopcode_COMB_GT_FWD);
    case LHPPAop_COMB_GE_FWD:
      return (PAopcode_COMB_GE_FWD);
    case LHPPAop_COMB_LT_FWD:
      return (PAopcode_COMB_LT_FWD);
    case LHPPAop_COMB_LE_FWD:
      return (PAopcode_COMB_LE_FWD);
    case LHPPAop_COMB_GT_U_FWD:
      return (PAopcode_COMB_GT_U_FWD);
    case LHPPAop_COMB_GE_U_FWD:
      return (PAopcode_COMB_GE_U_FWD);
    case LHPPAop_COMB_LT_U_FWD:
      return (PAopcode_COMB_LT_U_FWD);
    case LHPPAop_COMB_LE_U_FWD:
      return (PAopcode_COMB_LE_U_FWD);
    case LHPPAop_COMIB_EQ_FWD:
      return (PAopcode_COMIB_EQ_FWD);
    case LHPPAop_COMIB_NE_FWD:
      return (PAopcode_COMIB_NE_FWD);
    case LHPPAop_COMIB_GT_FWD:
      return (PAopcode_COMIB_GT_FWD);
    case LHPPAop_COMIB_GE_FWD:
      return (PAopcode_COMIB_GE_FWD);
    case LHPPAop_COMIB_LT_FWD:
      return (PAopcode_COMIB_LT_FWD);
    case LHPPAop_COMIB_LE_FWD:
      return (PAopcode_COMIB_LE_FWD);
    case LHPPAop_COMIB_GT_U_FWD:
      return (PAopcode_COMIB_GT_U_FWD);
    case LHPPAop_COMIB_GE_U_FWD:
      return (PAopcode_COMIB_GE_U_FWD);
    case LHPPAop_COMIB_LT_U_FWD:
      return (PAopcode_COMIB_LT_U_FWD);
    case LHPPAop_COMIB_LE_U_FWD:
      return (PAopcode_COMIB_LE_U_FWD);
    case LHPPAop_COMB_EQ_BWD:
      return (PAopcode_COMB_EQ_BWD);
    case LHPPAop_COMB_NE_BWD:
      return (PAopcode_COMB_NE_BWD);
    case LHPPAop_COMB_GT_BWD:
      return (PAopcode_COMB_GT_BWD);
    case LHPPAop_COMB_GE_BWD:
      return (PAopcode_COMB_GE_BWD);
    case LHPPAop_COMB_LT_BWD:
      return (PAopcode_COMB_LT_BWD);
    case LHPPAop_COMB_LE_BWD:
      return (PAopcode_COMB_LE_BWD);
    case LHPPAop_COMB_GT_U_BWD:
      return (PAopcode_COMB_GT_U_BWD);
    case LHPPAop_COMB_GE_U_BWD:
      return (PAopcode_COMB_GE_U_BWD);
    case LHPPAop_COMB_LT_U_BWD:
      return (PAopcode_COMB_LT_U_BWD);
    case LHPPAop_COMB_LE_U_BWD:
      return (PAopcode_COMB_LE_U_BWD);
    case LHPPAop_COMIB_EQ_BWD:
      return (PAopcode_COMIB_EQ_BWD);
    case LHPPAop_COMIB_NE_BWD:
      return (PAopcode_COMIB_NE_BWD);
    case LHPPAop_COMIB_GT_BWD:
      return (PAopcode_COMIB_GT_BWD);
    case LHPPAop_COMIB_GE_BWD:
      return (PAopcode_COMIB_GE_BWD);
    case LHPPAop_COMIB_LT_BWD:
      return (PAopcode_COMIB_LT_BWD);
    case LHPPAop_COMIB_LE_BWD:
      return (PAopcode_COMIB_LE_BWD);
    case LHPPAop_COMIB_GT_U_BWD:
      return (PAopcode_COMIB_GT_U_BWD);
    case LHPPAop_COMIB_GE_U_BWD:
      return (PAopcode_COMIB_GE_U_BWD);
    case LHPPAop_COMIB_LT_U_BWD:
      return (PAopcode_COMIB_LT_U_BWD);
    case LHPPAop_COMIB_LE_U_BWD:
      return (PAopcode_COMIB_LE_U_BWD);
    case LHPPAop_BB_0_FWD:
      return (PAopcode_BB_0_FWD);
    case LHPPAop_BB_1_FWD:
      return (PAopcode_BB_1_FWD);
    case LHPPAop_BB_0_BWD:
      return (PAopcode_BB_0_BWD);
    case LHPPAop_BB_1_BWD:
      return (PAopcode_BB_1_BWD);
    case LHPPAop_LDIL:
      return (PAopcode_LDIL);
    case LHPPAop_ADDIB_LT_FWD:
      return (PAopcode_ADDIB_LT_FWD);
    case LHPPAop_ADDIB_LT_BWD:
      return (PAopcode_ADDIB_LT_BWD);
    case LHPPAop_EXTRU:
      return (PAopcode_EXTRU);
    case LHPPAop_DEPI:
      return (PAopcode_DEPI);
    case LHPPAop_JSR_DYNCALL:
      return (PAopcode_JSR_DYNCALL);

    case LHPPAop_PRED_MOV:
      return (PAopcode_PRED_MOV);
    case LHPPAop_ZVDEPI:
      return (PAopcode_ZVDEPI);

    case LHPPAop_LD_UC_SV1:
      return (PAopcode_LD_UC_SV1);
    case LHPPAop_LD_UC_SC1:
      return (PAopcode_LD_UC_SC1);
    case LHPPAop_LD_UC_SC2:
      return (PAopcode_LD_UC_SC2);
    case LHPPAop_LD_UC_SC3:
      return (PAopcode_LD_UC_SC3);

    case LHPPAop_LD_PRE_UC_SV1:
      return (PAopcode_LD_PRE_UC_SV1);
    case LHPPAop_LD_PRE_UC_SC1:
      return (PAopcode_LD_PRE_UC_SC1);
    case LHPPAop_LD_PRE_UC_SC2:
      return (PAopcode_LD_PRE_UC_SC2);
    case LHPPAop_LD_PRE_UC_SC3:
      return (PAopcode_LD_PRE_UC_SC3);

    case LHPPAop_LD_POST_UC_SV1:
      return (PAopcode_LD_POST_UC_SV1);
    case LHPPAop_LD_POST_UC_SC1:
      return (PAopcode_LD_POST_UC_SC1);
    case LHPPAop_LD_POST_UC_SC2:
      return (PAopcode_LD_POST_UC_SC2);
    case LHPPAop_LD_POST_UC_SC3:
      return (PAopcode_LD_POST_UC_SC3);

    case LHPPAop_LD_C_SV1:
      return (PAopcode_LD_C_SV1);
    case LHPPAop_LD_C_SC1:
      return (PAopcode_LD_C_SC1);
    case LHPPAop_LD_C_SC2:
      return (PAopcode_LD_C_SC2);
    case LHPPAop_LD_C_SC3:
      return (PAopcode_LD_C_SC3);

    case LHPPAop_LD_PRE_C_SV1:
      return (PAopcode_LD_PRE_C_SV1);
    case LHPPAop_LD_PRE_C_SC1:
      return (PAopcode_LD_PRE_C_SC1);
    case LHPPAop_LD_PRE_C_SC2:
      return (PAopcode_LD_PRE_C_SC2);
    case LHPPAop_LD_PRE_C_SC3:
      return (PAopcode_LD_PRE_C_SC3);

    case LHPPAop_LD_POST_C_SV1:
      return (PAopcode_LD_POST_C_SV1);
    case LHPPAop_LD_POST_C_SC1:
      return (PAopcode_LD_POST_C_SC1);
    case LHPPAop_LD_POST_C_SC2:
      return (PAopcode_LD_POST_C_SC2);
    case LHPPAop_LD_POST_C_SC3:
      return (PAopcode_LD_POST_C_SC3);

    case LHPPAop_LD_UC2_SV1:
      return (PAopcode_LD_UC2_SV1);
    case LHPPAop_LD_UC2_SC1:
      return (PAopcode_LD_UC2_SC1);
    case LHPPAop_LD_UC2_SC2:
      return (PAopcode_LD_UC2_SC2);
    case LHPPAop_LD_UC2_SC3:
      return (PAopcode_LD_UC2_SC3);

    case LHPPAop_LD_PRE_UC2_SV1:
      return (PAopcode_LD_PRE_UC2_SV1);
    case LHPPAop_LD_PRE_UC2_SC1:
      return (PAopcode_LD_PRE_UC2_SC1);
    case LHPPAop_LD_PRE_UC2_SC2:
      return (PAopcode_LD_PRE_UC2_SC2);
    case LHPPAop_LD_PRE_UC2_SC3:
      return (PAopcode_LD_PRE_UC2_SC3);

    case LHPPAop_LD_POST_UC2_SV1:
      return (PAopcode_LD_POST_UC2_SV1);
    case LHPPAop_LD_POST_UC2_SC1:
      return (PAopcode_LD_POST_UC2_SC1);
    case LHPPAop_LD_POST_UC2_SC2:
      return (PAopcode_LD_POST_UC2_SC2);
    case LHPPAop_LD_POST_UC2_SC3:
      return (PAopcode_LD_POST_UC2_SC3);

    case LHPPAop_LD_C2_SV1:
      return (PAopcode_LD_C2_SV1);
    case LHPPAop_LD_C2_SC1:
      return (PAopcode_LD_C2_SC1);
    case LHPPAop_LD_C2_SC2:
      return (PAopcode_LD_C2_SC2);
    case LHPPAop_LD_C2_SC3:
      return (PAopcode_LD_C2_SC3);

    case LHPPAop_LD_PRE_C2_SV1:
      return (PAopcode_LD_PRE_C2_SV1);
    case LHPPAop_LD_PRE_C2_SC1:
      return (PAopcode_LD_PRE_C2_SC1);
    case LHPPAop_LD_PRE_C2_SC2:
      return (PAopcode_LD_PRE_C2_SC2);
    case LHPPAop_LD_PRE_C2_SC3:
      return (PAopcode_LD_PRE_C2_SC3);

    case LHPPAop_LD_POST_C2_SV1:
      return (PAopcode_LD_POST_C2_SV1);
    case LHPPAop_LD_POST_C2_SC1:
      return (PAopcode_LD_POST_C2_SC1);
    case LHPPAop_LD_POST_C2_SC2:
      return (PAopcode_LD_POST_C2_SC2);
    case LHPPAop_LD_POST_C2_SC3:
      return (PAopcode_LD_POST_C2_SC3);

    case LHPPAop_LD_I_SV1:
      return (PAopcode_LD_I_SV1);
    case LHPPAop_LD_I_SC1:
      return (PAopcode_LD_I_SC1);
    case LHPPAop_LD_I_SC2:
      return (PAopcode_LD_I_SC2);
    case LHPPAop_LD_I_SC3:
      return (PAopcode_LD_I_SC3);

    case LHPPAop_LD_PRE_I_SV1:
      return (PAopcode_LD_PRE_I_SV1);
    case LHPPAop_LD_PRE_I_SC1:
      return (PAopcode_LD_PRE_I_SC1);
    case LHPPAop_LD_PRE_I_SC2:
      return (PAopcode_LD_PRE_I_SC2);
    case LHPPAop_LD_PRE_I_SC3:
      return (PAopcode_LD_PRE_I_SC3);

    case LHPPAop_LD_POST_I_SV1:
      return (PAopcode_LD_POST_I_SV1);
    case LHPPAop_LD_POST_I_SC1:
      return (PAopcode_LD_POST_I_SC1);
    case LHPPAop_LD_POST_I_SC2:
      return (PAopcode_LD_POST_I_SC2);
    case LHPPAop_LD_POST_I_SC3:
      return (PAopcode_LD_POST_I_SC3);

    case LHPPAop_LD_F_SV1:
      return (PAopcode_LD_F_SV1);
    case LHPPAop_LD_F_SC1:
      return (PAopcode_LD_F_SC1);
    case LHPPAop_LD_F_SC2:
      return (PAopcode_LD_F_SC2);
    case LHPPAop_LD_F_SC3:
      return (PAopcode_LD_F_SC3);

    case LHPPAop_LD_PRE_F_SV1:
      return (PAopcode_LD_PRE_F_SV1);
    case LHPPAop_LD_PRE_F_SC1:
      return (PAopcode_LD_PRE_F_SC1);
    case LHPPAop_LD_PRE_F_SC2:
      return (PAopcode_LD_PRE_F_SC2);
    case LHPPAop_LD_PRE_F_SC3:
      return (PAopcode_LD_PRE_F_SC3);

    case LHPPAop_LD_POST_F_SV1:
      return (PAopcode_LD_POST_F_SV1);
    case LHPPAop_LD_POST_F_SC1:
      return (PAopcode_LD_POST_F_SC1);
    case LHPPAop_LD_POST_F_SC2:
      return (PAopcode_LD_POST_F_SC2);
    case LHPPAop_LD_POST_F_SC3:
      return (PAopcode_LD_POST_F_SC3);

    case LHPPAop_LD_F2_SV1:
      return (PAopcode_LD_F2_SV1);
    case LHPPAop_LD_F2_SC1:
      return (PAopcode_LD_F2_SC1);
    case LHPPAop_LD_F2_SC2:
      return (PAopcode_LD_F2_SC2);
    case LHPPAop_LD_F2_SC3:
      return (PAopcode_LD_F2_SC3);

    case LHPPAop_LD_PRE_F2_SV1:
      return (PAopcode_LD_PRE_F2_SV1);
    case LHPPAop_LD_PRE_F2_SC1:
      return (PAopcode_LD_PRE_F2_SC1);
    case LHPPAop_LD_PRE_F2_SC2:
      return (PAopcode_LD_PRE_F2_SC2);
    case LHPPAop_LD_PRE_F2_SC3:
      return (PAopcode_LD_PRE_F2_SC3);

    case LHPPAop_LD_POST_F2_SV1:
      return (PAopcode_LD_POST_F2_SV1);
    case LHPPAop_LD_POST_F2_SC1:
      return (PAopcode_LD_POST_F2_SC1);
    case LHPPAop_LD_POST_F2_SC2:
      return (PAopcode_LD_POST_F2_SC2);
    case LHPPAop_LD_POST_F2_SC3:
      return (PAopcode_LD_POST_F2_SC3);

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
M_oper_supported_in_arch_hppa (int opc)
{
  switch (opc)
    {

    case Lop_NOR:
    case Lop_NAND:
    case Lop_NXOR:
    case Lop_OR_NOT:

    case Lop_MUL_ADD:
    case Lop_MUL_ADD_U:
    case Lop_MUL_SUB:
    case Lop_MUL_SUB_U:
    case Lop_MUL_SUB_REV:
    case Lop_MUL_SUB_REV_U:
    case Lop_MUL_SUB_F:
    case Lop_MUL_SUB_REV_F:
    case Lop_MUL_SUB_F2:
    case Lop_MUL_SUB_REV_F2:
      return (0);

    case Lop_MUL_ADD_F:
    case Lop_MUL_ADD_F2:
      if (M_model == M_HP_PA_X)
	return (1);
      else
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
M_num_oper_required_for_hppa (L_Oper * oper, char *name)
{
#define	has_label_operand(a)	((a->src[0]->type == L_OPERAND_LABEL)||\
    				 (a->src[1]->type == L_OPERAND_LABEL))

#define indexed_memory_op(a)	((a->src[0]->type == L_OPERAND_REGISTER)&&\
    				 (a->src[1]->type == L_OPERAND_REGISTER))

#define non_zero_offset(a)	((!((a->src[1]->type==L_OPERAND_IMMED)&&\
                                   ((a->src[1]->ctype&0x30)==0x00)))||\
    				 (a->src[1]->value.i != 0))

#define short_int_inc(a,b)	((a->src[b]->type==L_OPERAND_IMMED)&&\
                                 ((a->src[b]->ctype&0x30)==0x00)&&\
    				 (a->src[b]->value.i >= -0x10) &&\
    				 (a->src[b]->value.i < 0x10))
#define long_pos_int_inc(a,b)	((a->src[b]->type==L_OPERAND_IMMED)&&\
                                 ((a->src[b]->ctype&0x30)==0x00)&&\
    				 (a->src[b]->value.i > 0) && \
    				 (a->src[b]->value.i < 0x2000))
#define long_neg_int_inc(a,b)	((a->src[b]->type==L_OPERAND_IMMED)&&\
                                 ((a->src[b]->ctype&0x30)==0x00)&&\
    				 (a->src[b]->value.i >= -0x2000) && \
    				 (a->src[b]->value.i < 0))
#define register_inc(a,b)	 (a->src[b]->type == L_OPERAND_REGISTER)

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
      if (has_label_operand (oper))
	return (2);
      else
	return (1);

    case Lop_ST_F:
    case Lop_ST_F2:
      if (has_label_operand (oper))
	return (2);
      else
	return (1);

    case Lop_LD_F:
    case Lop_LD_F2:
      if (has_label_operand (oper))
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

int
M_is_stack_operand_hppa (L_Operand * operand)
{
  if (L_is_macro (operand) &&
      (operand->value.mac == L_MAC_SP ||
       operand->value.mac == L_MAC_FP ||
       operand->value.mac == HPPA_MAC_TRUE_SP ||
       operand->value.mac == L_MAC_SAFE_MEM ||
       operand->value.mac == L_MAC_P12 ||
       operand->value.mac == L_MAC_IP ||
       operand->value.mac == L_MAC_OP ||
       operand->value.mac == L_MAC_RS || operand->value.mac == L_MAC_LV))
    return (1);

  return (0);
}

int
M_is_unsafe_macro_hppa (L_Operand * operand)
{
  if (!L_is_macro (operand))
    return (0);

  switch (operand->value.mac)
    {
    case HPPA_MAC_FZERO:
    case HPPA_MAC_SAR:
    case HPPA_MAC_FLOAT_CBIT:
    case HPPA_MAC_SR3:
    case HPPA_MAC_SR5:
      return (1);
    default:
      return (0);
    }
}

int
M_operand_type_hppa (L_Operand * operand)
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
      return (MDES_OPERAND_REG);

    case L_OPERAND_CB:
    case L_OPERAND_LABEL:
    case L_OPERAND_FLOAT:
    case L_OPERAND_DOUBLE:
    case L_OPERAND_STRING:
      return (MDES_OPERAND_Label);

    default:
      M_assert (0, "M_operand_type_hppa: Unknown type");
      return (0);
    }
}

int
M_conflicting_operands_hppa (L_Operand * operand,
			     L_Operand ** conflict_array, int len,
			     int prepass)
{
  int right = 0, left = 0;

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
      if (operand->value.r - MAX_INT_NAME > 0)
	{			/* floating point register */
	  if (operand->value.r - MAX_INT_NAME - 32 > 0)
	    {
	      /* right side of a floating point register */
	      right = operand->value.r;
	      left = operand->value.r - 32;
	    }
	  else
	    {
	      /* left side of a floating point register */
	      left = operand->value.r;
	      right = operand->value.r + 32;
	    }
	}
      else
	{			/* integer register */
	  conflict_array[0] = L_copy_operand (operand);
	  return (1);
	}
    }
  else
    M_assert (0, "Lhppa_conflicting_operands: unsupported operand type");

  if (L_is_ctype_dbl (operand))
    {
      /* Set up conflicting double register */
      conflict_array[0] = L_copy_operand (operand);
      conflict_array[0]->value.r = left;

      /* Set up conflicting float register */
      conflict_array[1] = L_copy_operand (operand);
      conflict_array[1]->ctype = L_CTYPE_FLOAT;
      conflict_array[1]->value.r = left;

      conflict_array[2] = L_copy_operand (operand);
      conflict_array[2]->ctype = L_CTYPE_FLOAT;
      conflict_array[2]->value.r = right;
      return (3);
    }
  else if (L_is_ctype_flt (operand))
    {
      /* Set up conflicting double register */
      conflict_array[0] = L_copy_operand (operand);
      conflict_array[0]->ctype = L_CTYPE_DOUBLE;
      conflict_array[0]->value.r = left;

      /* Set up conflicting double register */
      conflict_array[1] = L_copy_operand (operand);
      return (2);
    }
  else
    {
      M_assert (0, "Lhppa_conflicting_operands: unsupported operand type");
      return (0);
    }
}

int
M_num_registers_hppa (int ctype)
{

  if (M_model == M_HP_PA_1_0)
    {
      M_assert (0, "M_num_registers_hppa: Rick need to add PA_1_0 :)");
    }
  else if (M_model == M_HP_PA_1_1)
    {
      M_assert (0, "M_num_registers_hppa: Rick need to add PA_1_1 :)");
    }
  else if ((M_model == M_HP_PA_7100) || (M_model == M_HP_PA_X))
    {
      /* Dont know if these numbers are correct, just guesses.. */
      switch (ctype)
	{
	case L_CTYPE_INT:
	  return (35);		/* over estimate here a bit */
	case L_CTYPE_FLOAT:
	  return (64);
	case L_CTYPE_DOUBLE:
	  return (32);
	default:
	  return (0);
	}
    }
  else
    {
      M_assert (0, "M_num_registers_hppa: unsupported model");
    }
  return (0);
}
