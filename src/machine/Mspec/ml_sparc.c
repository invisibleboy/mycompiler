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
 *	File:	ml_sparc.c
 *	Author:	Roland G. Ouellette, Wen-mei Hwu
 *	Creation Date:	Sept, 1990
 *      Modified : Roland G. Ouellette Mon Oct  8 14:33:41 1990
 *		   fixed negative lvar layout offsets.
 *                 modified from m_spec.c code by Pohua Paul Chang.
 *	           By Sabrina Hwu, June, 1993 for adapting to new lcode format
 *		   
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

/*===========================================================================
 *==========================================================================*/
/* 10/29/02 REK Adding config.h */
#include <config.h>
#include <Lcode/l_main.h>
#include <machine/m_spec.h>
#include <machine/m_sparc.h>

/*--------------------------------------------------------------------------*/
#define M_SPARC_SIZE_VOID	0
#define M_SPARC_SIZE_BIT	1
#define M_SPARC_SIZE_CHAR	8
#define M_SPARC_SIZE_SHORT	16
#define M_SPARC_SIZE_INT	32
#define M_SPARC_SIZE_LONG	32
#define M_SPARC_SIZE_FLOAT	32
#define M_SPARC_SIZE_DOUBLE	64
#define M_SPARC_SIZE_POINTER	32
#define M_SPARC_SIZE_UNION	-1
#define M_SPARC_SIZE_STRUCT	-1
#define M_SPARC_SIZE_BLOCK	-1
#define M_SPARC_SIZE_MAX	64

#define M_SPARC_ALIGN_VOID	-1
#define M_SPARC_ALIGN_BIT	1
#define M_SPARC_ALIGN_CHAR	8
#define M_SPARC_ALIGN_SHORT	16
#define M_SPARC_ALIGN_INT	32
#define M_SPARC_ALIGN_LONG	32
#define M_SPARC_ALIGN_FLOAT	32
#define M_SPARC_ALIGN_DOUBLE	64
#define M_SPARC_ALIGN_POINTER	32
#define M_SPARC_ALIGN_UNION	-1	/* depends on the field */
#define M_SPARC_ALIGN_STRUCT	-1
#define M_SPARC_ALIGN_BLOCK	-1
#define M_SPARC_ALIGN_MAX	64

#define M_SPARC_SMALL_STRUCT_MAX 128


/*--------------------------------------------------------------------------*/
#define M_SPARC_MAX_FNVAR_REG	6
#define M_SPARC_PARAM_FP_OFFSET (17 * 4 * 8)	/* 17 longwords in bits... */
#define MIN_PARAM_SIZE \
	((M_SPARC_MAX_FNVAR_REG * M_SPARC_SIZE_INT) + M_SPARC_PARAM_FP_OFFSET)

#define M_SPARC_IN_BASE		0
#define M_SPARC_RET_IN		0
#define M_SPARC_OUT_BASE	6
#define M_SPARC_RET_OUT		6
#define M_SPARC_RET_F		12	/* %f0 & %f1 */
/* #define M_SPARC_RET_F2	13 *//* %f0 & %f1 */
#define M_SPARC_ST		14	/* the hidden param */
/*#define M_SPARC_ST_IN		15*//* different to be safe */

/*--------------------------------------------------------------------------*/
/*
 *	The following functions are for documentation only.
 *	The function overhead is too great for actual usage.
 */
void
M_sparc_void (M_Type type)
{
  type->type = M_TYPE_VOID;
  type->unsign = 1;
  type->align = M_SPARC_ALIGN_VOID;
  type->size = M_SPARC_SIZE_VOID;
  type->nbits = 0;
}

void
M_sparc_bit_long (M_Type type, int n)
{
  type->type = M_TYPE_BIT_LONG;
  type->unsign = 0;
  type->align = M_SPARC_ALIGN_BIT;
  type->size = n * M_SPARC_SIZE_BIT;
  type->nbits = n * M_SPARC_SIZE_BIT;
  M_assert ((n <= 32),
	    "M_bit_long: do not allow bit field of more than 32 bits");
}

void
M_sparc_bit_int (M_Type type, int n)
{
  type->type = M_TYPE_BIT_INT;
  type->unsign = 0;
  type->align = M_SPARC_ALIGN_BIT;
  type->size = n * M_SPARC_SIZE_BIT;
  type->nbits = n * M_SPARC_SIZE_BIT;
  M_assert ((n <= 32),
	    "M_bit_int: do not allow bit field of more than 32 bits");
}

void
M_sparc_bit_short (M_Type type, int n)
{
  type->type = M_TYPE_BIT_SHORT;
  type->unsign = 0;
  type->align = M_SPARC_ALIGN_BIT;
  type->size = n * M_SPARC_SIZE_BIT;
  type->nbits = n * M_SPARC_SIZE_BIT;
  M_assert ((n <= 16),
	    "M_bit_long: do not allow bit field of more than 16 bits");
}

void
M_sparc_bit_char (M_Type type, int n)
{
  type->type = M_TYPE_BIT_CHAR;
  type->unsign = 0;
  type->align = M_SPARC_ALIGN_BIT;
  type->size = n * M_SPARC_SIZE_BIT;
  type->nbits = n * M_SPARC_SIZE_BIT;
  M_assert ((n <= 8),
	    "M_bit_char: do not allow bit field of more than 8 bits");
}

void
M_sparc_float (M_Type type, int unsign)
{
  type->type = M_TYPE_FLOAT;
  type->unsign = unsign;
  type->align = M_SPARC_ALIGN_FLOAT;
  type->size = M_SPARC_SIZE_FLOAT;
  type->nbits = M_SPARC_SIZE_FLOAT;
}

void
M_sparc_double (M_Type type, int unsign)
{
  type->type = M_TYPE_DOUBLE;
  type->unsign = unsign;
  type->align = M_SPARC_ALIGN_DOUBLE;
  type->size = M_SPARC_SIZE_DOUBLE;
  type->nbits = M_SPARC_SIZE_DOUBLE;
}

void
M_sparc_pointer (M_Type type)
{
  type->type = M_TYPE_POINTER;
  type->unsign = 1;
  type->align = M_SPARC_ALIGN_POINTER;
  type->size = M_SPARC_SIZE_POINTER;
  type->nbits = M_SPARC_SIZE_POINTER;
}

/*--------------------------------------------------------------------------*/

int
M_sparc_eval_type (M_Type type, M_Type ntype)
{
  switch (type->type)
    {
    case M_TYPE_VOID:
      M_void (ntype);
      return -1;		/* can not be evaluated */
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
      return M_TYPE_INT;
    case M_TYPE_FLOAT:
      M_float (ntype, type->unsign);
      return M_TYPE_FLOAT;
    case M_TYPE_DOUBLE:
      M_double (ntype, type->unsign);
      return M_TYPE_DOUBLE;
    case M_TYPE_UNION:
    case M_TYPE_STRUCT:
      *ntype = *type;
      return type->type;
    default:
      return -1;
    }
}

int
M_sparc_eval_type2 (M_Type type, M_Type ntype)
{
  switch (type->type)
    {
    case M_TYPE_VOID:
      M_void (ntype);
      return -1;		/* can not be evaluated */
    case M_TYPE_BIT_CHAR:
    case M_TYPE_CHAR:
      M_char (ntype, type->unsign);
      return M_TYPE_CHAR;
    case M_TYPE_SHORT:
      M_short (ntype, type->unsign);
      return M_TYPE_SHORT;
    case M_TYPE_BLOCK:
    case M_TYPE_INT:
      /* the starting address of array is used */
      M_int (ntype, type->unsign);
      return M_TYPE_INT;
    case M_TYPE_BIT_LONG:
    case M_TYPE_LONG:
      M_long (ntype, type->unsign);
      return M_TYPE_LONG;
    case M_TYPE_POINTER:
      M_pointer (ntype);
      return M_TYPE_POINTER;
    case M_TYPE_FLOAT:
      M_float (ntype, type->unsign);
      return M_TYPE_FLOAT;
    case M_TYPE_DOUBLE:
      M_double (ntype, type->unsign);
      return M_TYPE_DOUBLE;
    case M_TYPE_UNION:
    case M_TYPE_STRUCT:
      *ntype = *type;
      return type->type;
    default:
      return -1;
    }
}

int
M_sparc_call_type (M_Type type, M_Type ntype)
{
  switch (type->type)
    {
    case M_TYPE_VOID:
      M_void (ntype);
      return -1;		/* can not be evaluated */
    case M_TYPE_BIT_CHAR:
    case M_TYPE_CHAR:
      M_char (ntype, type->unsign);
      return M_TYPE_CHAR;
    case M_TYPE_SHORT:
      M_short (ntype, type->unsign);
      return M_TYPE_SHORT;
    case M_TYPE_BIT_LONG:
    case M_TYPE_INT:
    case M_TYPE_LONG:
    case M_TYPE_POINTER:
    case M_TYPE_BLOCK:
      /* the starting address of array is used */
      M_int (ntype, type->unsign);
      return M_TYPE_INT;
      /* BCC - don't promote float to double now - 8/5/96 */
    case M_TYPE_FLOAT:
      /* BCC - 1/15/97
       * Always promote floats to doubles because M_sparc_fnvar_layout requires
       * that and I don't know how to fix M_sparc_fnvar_layout yes.
       */
      /*
         M_float(ntype, type->unsign);
         return M_TYPE_FLOAT;
       */
    case M_TYPE_DOUBLE:
      M_double (ntype, type->unsign);
      return M_TYPE_DOUBLE;
    case M_TYPE_UNION:
    case M_TYPE_STRUCT:
      *ntype = *type;
      return type->type;
    default:
      return -1;
    }
}

int
M_sparc_call_type2 (M_Type type, M_Type ntype)
{
  switch (type->type)
    {
    case M_TYPE_VOID:
      M_void (ntype);
      return -1;		/* can not be evaluated */
    case M_TYPE_BIT_CHAR:
    case M_TYPE_CHAR:
      M_char (ntype, type->unsign);
      return M_TYPE_CHAR;
    case M_TYPE_SHORT:
      M_short (ntype, type->unsign);
      return M_TYPE_SHORT;
    case M_TYPE_BLOCK:
    case M_TYPE_INT:
      /* the starting address of array is used */
      M_int (ntype, type->unsign);
      return M_TYPE_INT;
    case M_TYPE_BIT_LONG:
    case M_TYPE_LONG:
      M_long (ntype, type->unsign);
      return M_TYPE_LONG;
    case M_TYPE_POINTER:
      M_pointer (ntype);
      return M_TYPE_POINTER;
      /* BCC - don't promote float to double now - 8/5/96 */
    case M_TYPE_FLOAT:
      /* BCC - 1/15/97
       * Always promote floats to doubles because M_sparc_fnvar_layout requires
       * that and I don't know how to fix M_sparc_fnvar_layout yes.
       */
      /*
         M_float(ntype, type->unsign);
         return M_TYPE_FLOAT;
       */
    case M_TYPE_DOUBLE:
      M_double (ntype, type->unsign);
      return M_TYPE_DOUBLE;
    case M_TYPE_UNION:
    case M_TYPE_STRUCT:
      *ntype = *type;
      return type->type;
    default:
      return -1;
    }
}

/*--------------------------------------------------------------------------*/
void
M_sparc_array_layout (M_Type type, int *offset)
{
  *offset = 0;			/* assume first element is aligned */
}

int
M_sparc_array_align (M_Type type)
{
  return type->align;
}

int
M_sparc_array_size (M_Type type, int dim)
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
M_sparc_union_layout (int n, _M_Type * type, int *offset, int *bit_offset)
{
  int i;
  for (i = 0; i < n; i++)
    {				/* assume the union is aligned */
      offset[i] = 0;
      bit_offset[i] = 0;
    }
}

int
M_sparc_union_align (int n, _M_Type * type)
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
   *    align to at least byte boundary.
   */
  if (max < M_SPARC_ALIGN_CHAR)
    max = M_SPARC_ALIGN_CHAR;
  return max;
}

int
M_sparc_union_size (int n, _M_Type * type)
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
   *    align to at least byte boundary.
   */
  if (max_align < M_SPARC_ALIGN_CHAR)
    max_align = M_SPARC_ALIGN_CHAR;
  /* need to increment to the max. align for future array extension */
  i = max_size % max_align;
  if (i != 0)
    max_size += (max_align - i);
  return max_size;
}

/*--------------------------------------------------------------------------*/
void
M_sparc_struct_layout (int n, _M_Type * type, int *base, int *bit_offset)
{
  int i, offset;
  int mod, size, align;
  int mod_word, mod_type;

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
      if ((i != 0) &&
	  (type[i].type != M_TYPE_BIT_LONG) &&
	  (type[i].type != M_TYPE_BIT_SHORT) &&
	  (type[i].type != M_TYPE_BIT_CHAR))
	{
	  if (type[i - 1].type == M_TYPE_BIT_LONG)
	    {
	      mod = offset % M_SPARC_ALIGN_INT;
	      if (mod != 0)
		offset += (M_SPARC_ALIGN_INT - mod);
	    }
	  else if (type[i - 1].type == M_TYPE_BIT_CHAR)
	    {
	      mod = offset % M_SPARC_ALIGN_CHAR;
	      if (mod != 0)
		offset += (M_SPARC_ALIGN_CHAR - mod);
	    }
	}

      mod_word = offset % M_SPARC_SIZE_INT;
      if (type[i].type == M_TYPE_BIT_CHAR)
	{
	  if ((mod_word + size) > M_SPARC_SIZE_INT)
	    offset += (M_SPARC_SIZE_INT - mod_word);
	  else
	    {
	      mod_type = offset % M_SPARC_SIZE_CHAR;
	      if ((mod_type + size) > M_SPARC_SIZE_CHAR)
		{
		  /* fprintf(stdout,"Changing bit char to bit short\n"); */
		  type[i].type = M_TYPE_BIT_SHORT;
		}
	    }
	}
      if (type[i].type == M_TYPE_BIT_SHORT)
	{
	  if ((mod_word + size) > M_SPARC_SIZE_INT)
	    offset += (M_SPARC_SIZE_INT - mod_word);
	  else
	    {
	      mod_type = offset % M_SPARC_SIZE_SHORT;
	      if ((mod_type + size) > M_SPARC_SIZE_SHORT)
		{
		  /* fprintf(stdout,"Changing bit short to bit long\n"); */
		  type[i].type = M_TYPE_BIT_LONG;
		}
	    }
	}
      else if (type[i].type == M_TYPE_BIT_LONG)
	{
	  if ((mod_word + size) > M_SPARC_SIZE_INT)
	    offset += (M_SPARC_SIZE_INT - mod_word);
	}
      mod = offset % align;	/* align to what the field */
      if (mod != 0)		/* needs to start from */
	offset += (align - mod);

      if (type[i].type == M_TYPE_BIT_CHAR)
	{
	  int mod = offset % M_SPARC_SIZE_INT;

	  bit_offset[i] = offset - mod + (M_SPARC_SIZE_INT - mod - size);
	  base[i] = offset & (~(M_SPARC_SIZE_CHAR - 1));
#if 0
	  fprintf (stderr, "type = %d, base = %d, bit_offset = %d\n",
		   type[i].type, base[i], bit_offset[i]);
#endif
	}
      else if (type[i].type == M_TYPE_BIT_SHORT)
	{
	  int mod = offset % M_SPARC_SIZE_INT;

	  bit_offset[i] = offset - mod + (M_SPARC_SIZE_INT - mod - size);
	  base[i] = offset & (~(M_SPARC_SIZE_SHORT - 1));
#if 0
	  fprintf (stderr, "type = %d, base = %d, bit_offset = %d\n",
		   type[i].type, base[i], bit_offset[i]);
#endif
	}
      else if (type[i].type == M_TYPE_BIT_LONG)
	{
	  int mod = offset % M_SPARC_SIZE_INT;

	  bit_offset[i] = offset - mod + (M_SPARC_SIZE_INT - mod - size);
	  base[i] = offset & (~(M_SPARC_SIZE_LONG - 1));
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
M_sparc_struct_align (int n, _M_Type * type)
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
   *    align to at least byte boundary.
   */
  if (max < M_SPARC_ALIGN_CHAR)
    max = M_SPARC_ALIGN_CHAR;
  return max;
}

int
M_sparc_struct_size (int n, _M_Type * type, int struct_align)
{
  int i, offset;
  int mod, size, align, max_align;
  int mod_word;
  offset = 0;			/* assume initially aligned */
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
      mod_word = offset % M_SPARC_SIZE_INT;
      if (type[i].type == M_TYPE_BIT_CHAR)
	{
	  if ((mod_word + size) > M_SPARC_SIZE_INT)
	    offset += (M_SPARC_SIZE_INT - mod_word);
	  if (M_SPARC_ALIGN_CHAR > max_align)
	    max_align = M_SPARC_ALIGN_CHAR;
	}
      else if (type[i].type == M_TYPE_BIT_SHORT)
	{
	  if ((mod_word + size) > M_SPARC_SIZE_INT)
	    offset += (M_SPARC_SIZE_INT - mod_word);
	  if (M_SPARC_ALIGN_SHORT > max_align)
	    max_align = M_SPARC_ALIGN_SHORT;
	}
      else if (type[i].type == M_TYPE_BIT_LONG)
	{
	  if ((mod_word + size) > M_SPARC_SIZE_INT)
	    offset += (M_SPARC_SIZE_INT - mod_word);
	  if (M_SPARC_ALIGN_LONG > max_align)
	    max_align = M_SPARC_ALIGN_LONG;
	}

      mod = offset % align;
      if (mod != 0)
	offset += (align - mod);
      offset += size;
    }
  /*
   *    align to at least byte boundary.
   */
  if (max_align < M_SPARC_ALIGN_CHAR)
    max_align = M_SPARC_ALIGN_CHAR;
  /* enforce max. alignment */
  mod = offset % max_align;
  if (mod != 0)
    offset += (max_align - mod);
  return offset;
}

int
M_sparc_layout_fnvar (List param_list, char **base_macro, int *pcount,
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

  size = M_sparc_fnvar_layout (cnt, type, offset, mode, reg, paddr,
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
M_sparc_fnvar_layout (int n, _M_Type * type, long int *offset, int *mode,
		      int *reg, int *paddr, char **macro,
		      int *su_sreg, int *su_ereg,
		      int *pcount, int is_st, int purpose)
					/* need to return structure */
{
  int i, max_align, off, rg, rg_base = 0;
  int size, align, mod, tp;

  switch (purpose)
    {
    case M_GET_FNVAR:
      rg_base = M_SPARC_IN_BASE;
      *macro = "$IP";
      break;
    case M_PUT_FNVAR:
      rg_base = M_SPARC_OUT_BASE;
      *macro = "$OP";
      break;
    case M_DONT_CARE_FNVAR:
    default:
      /*
         rg_base = 1000000;
         *macro = "NoBaseMacro";
       */
      M_assert (0, "M_fnvar_layout: unknown purpose");

    }

  max_align = M_SPARC_ALIGN_MAX;
  rg = 0;
  off = M_SPARC_PARAM_FP_OFFSET;

  for (i = 0; i < n; i++)
    {
      tp = type[i].type;
      switch (tp)
	{
	case M_TYPE_CHAR:
	case M_TYPE_SHORT:
	case M_TYPE_INT:
	  if (rg < M_SPARC_MAX_FNVAR_REG)
	    {
	      mode[i] = M_THRU_REGISTER;
	      reg[i] = rg + rg_base;
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
	  if (rg < M_SPARC_MAX_FNVAR_REG)
	    {
	      mode[i] = M_INDIRECT_THRU_REGISTER;
	      reg[i] = rg + rg_base;
	      rg++;
	    }
	  else
	    {
	      mode[i] = M_INDIRECT_THRU_MEMORY;
	      reg[i] = -1;
	    }
	  break;

	case M_TYPE_DOUBLE:
	  if (rg < M_SPARC_MAX_FNVAR_REG)
	    {
	      mode[i] = M_THRU_REGISTER;
	      reg[i] = rg + rg_base;
	      rg += 2;
	    }
	  else
	    {
	      mode[i] = M_THRU_MEMORY;
	      reg[i] = -1;
	    }
	  break;

	default:
	  M_assert (0, "M_fnvar_layout: argument is not promoted");
	}

      if ((tp == M_TYPE_UNION) || (tp == M_TYPE_STRUCT))
	{
	  /* for the pointer to the structure */
	  size = M_SPARC_SIZE_POINTER;
	  align = M_SPARC_ALIGN_POINTER;
	}
      else
	{
	  size = type[i].size;
	  align = type[i].align;
	  align = (align <= 32) ? align : 32;	/* doubles not aligned 
						   -- AARGH */
	}

      mod = off % align;
      if (mod != 0)
	off += (align - mod);

      /* since we're big endian... */
      if (size < M_SPARC_SIZE_INT)
	off += (M_SPARC_SIZE_INT - size);

      offset[i] = off;
      off += size;
    }

  /* The param section must be at least certain size.
   * This is the backing store for the parameter regs. */
  if (off < MIN_PARAM_SIZE)
    off = MIN_PARAM_SIZE;

  /* the param section must be max. aligned */
  mod = off % max_align;
  if (mod != 0)
    off += (max_align - mod);

  /* now for the body of the structures... small ones first. */
  for (i = 0; i < n; i++)
    {
      tp = type[i].type;

      size = type[i].size;

      if (((tp == M_TYPE_UNION) ||
	   (tp == M_TYPE_STRUCT)) && (size <= M_SPARC_SMALL_STRUCT_MAX))
	{

	  align = M_SPARC_ALIGN_MAX;	/* must align to a double boundry */

	  mod = off % align;
	  if (mod != 0)
	    off += (align - mod);

	  paddr[i] = off;
	  off += size;
	}
    }

  /* now large ones */
  for (i = 0; i < n; i++)
    {
      tp = type[i].type;

      size = type[i].size;

      if (((tp == M_TYPE_UNION) ||
	   (tp == M_TYPE_STRUCT)) && (size > M_SPARC_SMALL_STRUCT_MAX))
	{

	  align = M_SPARC_ALIGN_MAX;	/* must align to a double boundry */

	  mod = off % align;
	  if (mod != 0)
	    off += (align - mod);

	  paddr[i] = off;
	  off += size;
	}
    }

  /* the param section must be max. aligned */
  mod = off % max_align;
  if (mod != 0)
    off += (max_align - mod);

  return off;			/* the total size needed */
}

/*--------------------------------------------------------------------------*/
int
M_sparc_lvar_layout (int n, _M_Type * type, long int *offset,
		     char **base_macro)
{
  int i, max_align, off;
  int size, align, mod, tp;
  /*
   *    the LOCAL section must be max. aligned initially
   */
  max_align = M_SPARC_ALIGN_MAX;
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
	off -= (align + mod);
      off -= size;
      offset[i] = off;
    }
  /* the local section must be max. aligned */
  mod = off % max_align;
  if (mod != 0)
    off -= (max_align + mod);
  /*
   *    Local variables are relative to $LV
   */
  *base_macro = "$LV";
  return -off;			/* the total size needed */
}

/*--------------------------------------------------------------------------*/
int
M_sparc_no_short_int (void)
{
  return (M_SPARC_SIZE_SHORT == M_SPARC_SIZE_INT);
}
/*--------------------------------------------------------------------------*/
void
M_sparc_cb_label_name (char *fn, int cb, char *line, int len)
{
  sprintf (line, "Lcb%d%s", cb, fn);
}
/*--------------------------------------------------------------------------*/
int
M_sparc_is_cb_label (char *label, char *fn, int *cb)
{
  return (sscanf (label, "Lcb%d%s", cb, fn) == 2);
}
/*--------------------------------------------------------------------------*/
void
M_sparc_jumptbl_label_name (char *fn, int tbl_id, char *line, int len)
{
  sprintf (line, "%s%s%d", fn, M_JUMPTBL_BASE_NAME, tbl_id);
}
/*--------------------------------------------------------------------------*/
/* Format for sparc is: %sM_JUMPTBL_BASE_NAME%d, where %s is the func name  */
int
M_sparc_is_jumptbl_label (char *label, char *fn, int *tbl_id)
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
M_sparc_structure_pointer (int purpose)
{
  return M_SPARC_ST;
}
/*--------------------------------------------------------------------------*/
int
M_sparc_return_register (int type, int purpose)
{
  switch (type)
    {
    case M_TYPE_FLOAT:
    case M_TYPE_DOUBLE:
      return M_SPARC_RET_F;
    default:
      switch (purpose)
	{
	case M_GET_FNVAR:
	  return M_SPARC_RET_OUT;
	case M_PUT_FNVAR:
	  return M_SPARC_RET_IN;
	case M_DONT_CARE_FNVAR:
	default:
	  return -1;
	}
      /*return M_SPARC_RET_IN; */
    }
}

/*--------------------------------------------------------------------------*/
char *
M_sparc_fn_label_name (char *label, int (*is_func) (char *is_func_label))
{
  /* REH 5/18/95 made to simply return the label */
  return (label);
#if 0
  static char fn_label[1024];

  sprintf (fn_label, "_%s", label);
  return (fn_label);
#endif
}

char *
M_sparc_fn_name_from_label (char *label)
{
  /* REH 5/18/95 made to simply return the label */
  return (label);
#if 0
  return (label + 1);
#endif
}

/*--------------------------------------------------------------------------*/
/*
 *  Declare code generator specific macro registers to the front end parser.
 */
void
M_define_macros_sparc (STRING_Symbol_Table * sym_tbl)
{

  M_add_symbol (sym_tbl, "g0", L_SPARC_MAC_ZERO);
  M_add_symbol (sym_tbl, "g1", L_SPARC_MAC_G1);
  M_add_symbol (sym_tbl, "icc", L_SPARC_MAC_ICC);
  M_add_symbol (sym_tbl, "icc0", L_SPARC_MAC_ICC0);
  M_add_symbol (sym_tbl, "icc1", L_SPARC_MAC_ICC1);
  M_add_symbol (sym_tbl, "icc2", L_SPARC_MAC_ICC2);
  M_add_symbol (sym_tbl, "icc3", L_SPARC_MAC_ICC3);
  M_add_symbol (sym_tbl, "icc4", L_SPARC_MAC_ICC4);
  M_add_symbol (sym_tbl, "icc5", L_SPARC_MAC_ICC5);
  M_add_symbol (sym_tbl, "icc6", L_SPARC_MAC_ICC6);
  M_add_symbol (sym_tbl, "icc7", L_SPARC_MAC_ICC7);
  M_add_symbol (sym_tbl, "icc8", L_SPARC_MAC_ICC8);
  M_add_symbol (sym_tbl, "icc9", L_SPARC_MAC_ICC9);
  M_add_symbol (sym_tbl, "icc10", L_SPARC_MAC_ICC10);
  M_add_symbol (sym_tbl, "icc11", L_SPARC_MAC_ICC11);
  M_add_symbol (sym_tbl, "icc12", L_SPARC_MAC_ICC12);
  M_add_symbol (sym_tbl, "icc13", L_SPARC_MAC_ICC13);
  M_add_symbol (sym_tbl, "icc14", L_SPARC_MAC_ICC14);
  M_add_symbol (sym_tbl, "icc15", L_SPARC_MAC_ICC15);
  /* 1 if leaf function, 0 if non-leaf */
  M_add_symbol (sym_tbl, "$leaf", L_SPARC_MAC_LEAF);
  /* number of callee save registers used */
  M_add_symbol (sym_tbl, "$callee_regs", L_SPARC_MAC_CALLEE);
  /* number of caller save registers used */
  M_add_symbol (sym_tbl, "$caller_regs", L_SPARC_MAC_CALLER);
  /* total alloc requirements */
  M_add_symbol (sym_tbl, "$alloc_size", L_SPARC_MAC_ALLOC);
  M_add_symbol (sym_tbl, "pred_base", L_SPARC_MAC_PRED_BASE);
  M_add_symbol (sym_tbl, "fcc", L_SPARC_MAC_FCC);
  M_add_symbol (sym_tbl, "fcc0", L_SPARC_MAC_FCC0);
  M_add_symbol (sym_tbl, "fcc1", L_SPARC_MAC_FCC1);
  M_add_symbol (sym_tbl, "fcc2", L_SPARC_MAC_FCC2);
  M_add_symbol (sym_tbl, "fcc3", L_SPARC_MAC_FCC3);
  M_add_symbol (sym_tbl, "fcc4", L_SPARC_MAC_FCC4);
  M_add_symbol (sym_tbl, "fcc5", L_SPARC_MAC_FCC5);
  M_add_symbol (sym_tbl, "fcc6", L_SPARC_MAC_FCC6);
  M_add_symbol (sym_tbl, "fcc7", L_SPARC_MAC_FCC7);
  M_add_symbol (sym_tbl, "fcc8", L_SPARC_MAC_FCC8);
  M_add_symbol (sym_tbl, "fcc9", L_SPARC_MAC_FCC9);
  M_add_symbol (sym_tbl, "fcc10", L_SPARC_MAC_FCC10);
  M_add_symbol (sym_tbl, "fcc11", L_SPARC_MAC_FCC11);
  M_add_symbol (sym_tbl, "fcc12", L_SPARC_MAC_FCC12);
  M_add_symbol (sym_tbl, "fcc13", L_SPARC_MAC_FCC13);
  M_add_symbol (sym_tbl, "fcc14", L_SPARC_MAC_FCC14);
  M_add_symbol (sym_tbl, "fcc15", L_SPARC_MAC_FCC15);
  M_add_symbol (sym_tbl, "o7", L_SPARC_MAC_RETADDR);
  M_add_symbol (sym_tbl, "i7", L_SPARC_MAC_NONLEAF_RETADDR);
  M_add_symbol (sym_tbl, "C_set", L_SPARC_MAC_CARRY_SET);
  M_add_symbol (sym_tbl, "C_clear", L_SPARC_MAC_CARRY_CLEAR);
  M_add_symbol (sym_tbl, "N_set", L_SPARC_MAC_NEG_SET);
  M_add_symbol (sym_tbl, "N_clear", L_SPARC_MAC_NEG_CLEAR);
  M_add_symbol (sym_tbl, "V_set", L_SPARC_MAC_OVERFLOW_SET);
  M_add_symbol (sym_tbl, "V_clear", L_SPARC_MAC_OVERFLOW_CLEAR);
  M_add_symbol (sym_tbl, "fsr", L_SPARC_MAC_FSR);
  M_add_symbol (sym_tbl, "y", L_SPARC_MAC_Y);

}
/*--------------------------------------------------------------------------*/
int
M_sparc_fragile_macro (int macro_value)
{
  switch (M_model)
    {
    case M_SPARC_ELC:
    case M_SPARC_VIKING:
      switch (macro_value)
	{
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
    default:
      M_assert (0, "M_sparc_fragile_macro:  Illegal model specified!");
      return (0);
    }
}

/*--------------------------------------------------------------------------*/
Set M_sparc_fragile_macro_set ()
{
  switch (M_model)
    {
    case M_SPARC_ELC:
    case M_SPARC_VIKING:
      return NULL;
    default:
      M_assert (0, "M_sparc_fragile_macro_set:  Illegal model specified!");
      return (0);
    }

}

/*--------------------------------------------------------------------------*/
int
M_sparc_dataflow_macro (int id)
{
  switch (M_model)
    {
    case M_SPARC_ELC:
    case M_SPARC_VIKING:
      return ((id >= L_MAC_P0 && id <= L_MAC_P64) || (id >= L_MAC_LAST));
    default:
      M_assert (0, "M_sparc_dataflow_macro:  Illegal model specified!");
      return (0);
    }
}


/*--------------------------------------------------------------------------*/
int
M_sparc_subroutine_call (int opc)
{
  switch (M_model)
    {
    case M_SPARC_ELC:
    case M_SPARC_VIKING:
      return ((opc == Lop_JSR) || (opc == Lop_JSR_FS) ||
	      (opc == Lop_REM) || (opc == Lop_REM_U));
    default:
      M_assert (0, "M_sparc_subroutine_call:  Illegal model specified!");
      return (0);
    }
}

/*--------------------------------------------------------------------------*/
char *
M_get_macro_name_sparc (int id)
{
  switch (id)
    {

    case L_SPARC_MAC_ZERO:
      return "g0";
    case L_SPARC_MAC_G1:
      return "g1";
    case L_SPARC_MAC_ICC:
      return "icc";
    case L_SPARC_MAC_ICC0:
      return "icc0";
    case L_SPARC_MAC_ICC1:
      return "icc1";
    case L_SPARC_MAC_ICC2:
      return "icc2";
    case L_SPARC_MAC_ICC3:
      return "icc3";
    case L_SPARC_MAC_ICC4:
      return "icc4";
    case L_SPARC_MAC_ICC5:
      return "icc5";
    case L_SPARC_MAC_ICC6:
      return "icc6";
    case L_SPARC_MAC_ICC7:
      return "icc7";
    case L_SPARC_MAC_ICC8:
      return "icc8";
    case L_SPARC_MAC_ICC9:
      return "icc9";
    case L_SPARC_MAC_ICC10:
      return "icc10";
    case L_SPARC_MAC_ICC11:
      return "icc11";
    case L_SPARC_MAC_ICC12:
      return "icc12";
    case L_SPARC_MAC_ICC13:
      return "icc13";
    case L_SPARC_MAC_ICC14:
      return "icc14";
    case L_SPARC_MAC_ICC15:
      return "icc15";
    case L_SPARC_MAC_LEAF:
      return "$leaf";
    case L_SPARC_MAC_CALLEE:
      return "$callee_regs";
    case L_SPARC_MAC_CALLER:
      return "$caller_regs";
    case L_SPARC_MAC_ALLOC:
      return "$alloc_size";
    case L_SPARC_MAC_PRED_BASE:
      return "pred_base";
    case L_SPARC_MAC_FCC:
      return "fcc";
    case L_SPARC_MAC_FCC0:
      return "fcc0";
    case L_SPARC_MAC_FCC1:
      return "fcc1";
    case L_SPARC_MAC_FCC2:
      return "fcc2";
    case L_SPARC_MAC_FCC3:
      return "fcc3";
    case L_SPARC_MAC_FCC4:
      return "fcc4";
    case L_SPARC_MAC_FCC5:
      return "fcc5";
    case L_SPARC_MAC_FCC6:
      return "fcc6";
    case L_SPARC_MAC_FCC7:
      return "fcc7";
    case L_SPARC_MAC_FCC8:
      return "fcc8";
    case L_SPARC_MAC_FCC9:
      return "fcc9";
    case L_SPARC_MAC_FCC10:
      return "fcc10";
    case L_SPARC_MAC_FCC11:
      return "fcc11";
    case L_SPARC_MAC_FCC12:
      return "fcc12";
    case L_SPARC_MAC_FCC13:
      return "fcc13";
    case L_SPARC_MAC_FCC14:
      return "fcc14";
    case L_SPARC_MAC_FCC15:
      return "fcc15";
    case L_SPARC_MAC_RETADDR:
      return "o7";
    case L_SPARC_MAC_NONLEAF_RETADDR:
      return "i7";
    case L_SPARC_MAC_CARRY_SET:
      return "C_set";
    case L_SPARC_MAC_CARRY_CLEAR:
      return "C_clear";
    case L_SPARC_MAC_NEG_SET:
      return "N_set";
    case L_SPARC_MAC_NEG_CLEAR:
      return "N_clear";
    case L_SPARC_MAC_OVERFLOW_SET:
      return "V_set";
    case L_SPARC_MAC_OVERFLOW_CLEAR:
      return "V_clear";
    case L_SPARC_MAC_FSR:
      return "fsr";
    case L_SPARC_MAC_Y:
      return "y";

    default:
      return "?";
    }
}
/*--------------------------------------------------------------------------*/
/*
 * Return true (1) if the instruction is supported in the hardware of the
 * processor.  Return false (0) otherwise.
 *
 * Basically, those are the instructions that are splitted into multiple
 * instructions in phase1 of the Sparc code generator and the optimizer
 * will avoid generating them.
 */
int
M_oper_supported_in_arch_sparc (int opc)
{
  switch (opc)
    {

    case Lop_NOR:
    case Lop_NAND:
    case Lop_AND_NOT:
    case Lop_OR_NOT:

    case Lop_MUL_ADD:
    case Lop_MUL_SUB:
    case Lop_MUL_ADD_F:
    case Lop_MUL_SUB_F:
    case Lop_MUL_ADD_F2:
    case Lop_MUL_SUB_F2:
    case Lop_MUL_SUB_REV:
    case Lop_MUL_SUB_REV_U:
    case Lop_MUL_SUB_REV_F:
    case Lop_MUL_SUB_REV_F2:
      return (0);

    default:
      return (1);
    }
}

/*--------------------------------------------------------------------------*/
int
M_is_stack_operand_sparc (L_Operand * operand)
{
  if (L_is_macro (operand) &&
      (operand->value.mac == L_MAC_SP ||
       operand->value.mac == L_MAC_FP ||
       operand->value.mac == L_MAC_LV ||
       operand->value.mac == L_MAC_IP ||
       operand->value.mac == L_MAC_OP || operand->value.mac == L_MAC_RS))
    return (1);

  return (0);
}

int
M_is_unsafe_macro_sparc (L_Operand * operand)
{
  if (!L_is_macro (operand))
    return (0);

  if ((operand->value.mac >= L_SPARC_MAC_ICC &&
       operand->value.mac <= L_SPARC_MAC_ICC15) ||
      (operand->value.mac >= L_SPARC_MAC_FCC &&
       operand->value.mac <= L_SPARC_MAC_FCC15))
    return (1);

  switch (operand->value.mac)
    {
    case L_MAC_P0:
    case L_MAC_P1:
    case L_MAC_P2:
    case L_MAC_P3:
    case L_MAC_P4:
    case L_MAC_P5:
    case L_MAC_P6:
    case L_MAC_P7:
    case L_MAC_P8:
    case L_MAC_P9:
    case L_MAC_P10:
    case L_MAC_P11:
      if ((L_is_ctype_flt (operand)) || (L_is_ctype_dbl (operand)))
	return (1);
      else
	return (0);

    case L_MAC_P14:
      return (1);

    default:
      return (0);
    }
}

/*--------------------------------------------------------------------------*/
/*
 * Returns the number of machine instructions required to implement the
 * specified oper in the best case.  It is assumed that this is for
 * supported instructions.  A call to M_oper_supported_in_arch should be
 * made for abnormal instructions.
 *
 * Sparc v8 does not support any pre/post increment loads or stores
 * instructions.
 */
int
M_num_oper_required_for_sparc (L_Oper * oper, char *name)
{
  switch (oper->opc)
    {
    case Lop_LD_PRE_UC:
    case Lop_LD_PRE_C:
    case Lop_LD_PRE_UC2:
    case Lop_LD_PRE_C2:
    case Lop_LD_PRE_I:

    case Lop_ST_PRE_C:
    case Lop_ST_PRE_C2:
    case Lop_ST_PRE_I:

    case Lop_LD_POST_UC:
    case Lop_LD_POST_C:
    case Lop_LD_POST_UC2:
    case Lop_LD_POST_C2:
    case Lop_LD_POST_I:

    case Lop_ST_POST_C:
    case Lop_ST_POST_C2:
    case Lop_ST_POST_I:

    case Lop_LD_PRE_F:
    case Lop_LD_PRE_F2:

    case Lop_ST_PRE_F:
    case Lop_ST_PRE_F2:

    case Lop_LD_POST_F:
    case Lop_LD_POST_F2:

    case Lop_ST_POST_F:
    case Lop_ST_POST_F2:
      return (2);

    default:
      return (1);
    }
}
/*--------------------------------------------------------------------------*/

int
M_operand_type_sparc (L_Operand * operand)
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
      M_assert (0, "M_operand_type_sparc: Unknown type");
      return (0);
    }
}

/*--------------------------------------------------------------------------*/

/* Change this before we attempt to perform scheduling */

int
M_conflicting_operands_sparc (L_Operand * operand,
			      L_Operand ** conflict_array, int len,
			      int prepass)
{
  int right = 0, left = 0, mac_pair;

  if (prepass && (!L_is_macro (operand)))
    {
      conflict_array[0] = L_copy_operand (operand);
      return (1);
    }

  if (L_is_macro (operand))
    {
      switch (operand->value.mac)
	{
	case L_MAC_P0:
	case L_MAC_P1:
	case L_MAC_P2:
	case L_MAC_P3:
	case L_MAC_P4:
	case L_MAC_P5:
	case L_MAC_P6:
	case L_MAC_P7:
	case L_MAC_P8:
	case L_MAC_P9:
	case L_MAC_P10:
	case L_MAC_P11:
	  if (operand->value.mac & 0x1)
	    {			/* if odd macro */
	      if (L_is_ctype_dbl (operand))
		{
		  I_punt ("Mac %d: This macro shouldn't be type double!",
			  operand->value.mac);
		  return (-1);	/* I_punt doesn't return */
		}
	      else
		mac_pair = operand->value.mac - 1;
	    }
	  else			/* if even macro */
	    mac_pair = operand->value.mac;

	  /* Conflicts with its int type */
	  conflict_array[0] = L_copy_operand (operand);
	  conflict_array[0]->ctype = L_CTYPE_INT;

	  /* Conflicts with its float type */
	  conflict_array[1] = L_copy_operand (operand);
	  conflict_array[1]->ctype = L_CTYPE_FLOAT;

	  /* Conflicts with its double type */
	  conflict_array[2] = L_copy_operand (operand);
	  conflict_array[2]->ctype = L_CTYPE_DOUBLE;
	  conflict_array[2]->value.mac = mac_pair;

	  /* Assuming only even macros have type double.
	   * Ex: (mac P0 f2) also conflicts with (mac P1 f) and (mac P1 i)
	   *     (mac P2 f2)  "     "        "   (mac P3 f) and (mac P3 i)
	   */
	  if (L_is_ctype_dbl (operand))
	    {
	      conflict_array[3] = L_copy_operand (operand);
	      conflict_array[3]->ctype = L_CTYPE_FLOAT;
	      conflict_array[3]->value.mac = operand->value.mac + 1;

	      conflict_array[4] = L_copy_operand (operand);
	      conflict_array[4]->ctype = L_CTYPE_INT;
	      conflict_array[4]->value.mac = operand->value.mac + 1;
	      return (5);
	    }
	  else
	    return (3);
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
      else if (L_is_ctype_flt (operand) || L_is_ctype_dbl (operand))
	{
	  if (operand->value.r & 0x1)
	    {
	      /* right side of a floating point register */
	      left = operand->value.r - 1;
	      right = operand->value.r;
	    }
	  else
	    {
	      /* left side of a floating point register */
	      left = operand->value.r;
	      right = operand->value.r + 1;
	    }
	}
      else
	{
	  M_assert (0, "Limpact_conflicting_operands: illegal reg number");
	}
    }
  else
    M_assert (0, "Lsparc_conflicting_operands: unsupported operand type");

  if (L_is_ctype_dbl (operand))
    {
      /* Set up conflicting double register */
      conflict_array[0] = L_copy_operand (operand);
      conflict_array[0]->value.r = left;

      /* Set up conflicting float register */
      conflict_array[1] = L_copy_operand (operand);
      conflict_array[1]->value.r = left;
      conflict_array[1]->ctype = L_CTYPE_FLOAT;

      conflict_array[2] = L_copy_operand (operand);
      conflict_array[2]->value.r = right;
      conflict_array[2]->ctype = L_CTYPE_FLOAT;
      return (3);
    }
  else if (L_is_ctype_flt (operand))
    {
      /* Set up conflicting double register */
      conflict_array[0] = L_copy_operand (operand);
      conflict_array[0]->ctype = L_CTYPE_DOUBLE;
      conflict_array[0]->value.r = left;

      /* Set up conflicting float register */
      conflict_array[1] = L_copy_operand (operand);
      return (2);
    }
  else
    {
      M_assert (0, "M_conflicting_operands_sparc: unsupported operand type");
      return (0);
    }
}

int
M_num_registers_sparc (int ctype)
{

  if ((M_model == M_SPARC_ELC) || (M_model == M_SPARC_VIKING))
    {
      /* Dont know if these numbers are correct, just guesses.. */
      switch (ctype)
	{
	case L_CTYPE_INT:
	  return (32);
	case L_CTYPE_FLOAT:
	  return (32);
	case L_CTYPE_DOUBLE:
	  return (16);
	default:
	  return (0);
	}
    }
  else
    {
      M_assert (0, "M_num_registers_sparc: unsupported model");
      return (0);
    }
}
