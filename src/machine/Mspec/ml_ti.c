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
 *	File :	ml_ti.c 
 *	Desc :	Machine dependent specification.  
 *	Date :	December 1995
 *	Auth :  Dan Connors and Sabrina Hwu
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
#ifdef M_TI_FOR_HCODE
#include <Hcode/h_ccode.h>
#endif
#include <Lcode/l_main.h>
#include "m_ti.h"

extern int use_standard_function_call_model;

/*--------------------------------------------------------------------------*/
#define M_TI_SIZE_VOID		0
#define M_TI_SIZE_BIT		1
#define M_TI_SIZE_CHAR		32	/* 32-bit word addressable */
#define M_TI_SIZE_SHORT		32	/* 32-bit word addressable */
#define M_TI_SIZE_INT		32
#define M_TI_SIZE_LONG		32
#define M_TI_SIZE_FLOAT		32
#define M_TI_SIZE_DOUBLE	32	/* TI doesn't have double */
#define M_TI_SIZE_POINTER	32
#define M_TI_SIZE_UNION		-1
#define M_TI_SIZE_STRUCT	-1
#define M_TI_SIZE_BLOCK		-1
#define M_TI_SIZE_MAX	 	64

#define M_TI_ALIGN_VOID		-1
#define M_TI_ALIGN_BIT		1
#define M_TI_ALIGN_CHAR		32	/* 32-bit word address able */
#define M_TI_ALIGN_SHORT	32	/* 32-bit word address able */
#define M_TI_ALIGN_INT		32
#define M_TI_ALIGN_LONG		32
#define M_TI_ALIGN_FLOAT	32
#define M_TI_ALIGN_DOUBLE	32	/* TI doesn't have double */
#define M_TI_ALIGN_POINTER	32
#define M_TI_ALIGN_UNION	-1	/* depends on the field */
#define M_TI_ALIGN_STRUCT	-1
#define M_TI_ALIGN_BLOCK	-1
#define M_TI_ALIGN_MAX		64



/*--------------------------------------------------------------------------*/

#define M_TI_MAX_FNVAR_REG 		6
#define M_TI_SMALL_STRUCT_MAX 		64
#define MIN_PARAM_SIZE 	  		(16 * 8)

#define M_TI_NUM_FLT_PARM_REGS 2	/* 2 float parameters   */
#define M_TI_NUM_INT_PARM_REGS 4	/* 4 Int parameters   */

#define M_TI_INT_BASE		0	/* 6 max integer parameters      */
					/* 2 have a priority to be float */
					/* P0,P1,P2,P3,P4,P5             */


/* 6 registers are used to pass parameters */
/* AR2,R2,R3,RC,RS,RE */
/* R2 and R3 have floating point priority */



#define M_TI_FLT_BASE	        4	/* 2 float parameters   */
					/* P4, P5               */

/* All return values are placed in P6, which is float/int */
#define M_TI_RET_F	        6	/* return in R0 */
#define M_TI_RET_I32		6	/* return in R0 */
#define M_TI_RET_I64		6	/* return in R0 */
#define M_TI_RET_ST		6	/* return in R0 */


/*--------------------------------------------------------------------------*/
/*
 *	The following functions are for documentation only.
 *	The function overhead is too great for actual usage.
 */
void
M_ti_void (M_Type type)
{
  type->type = M_TYPE_VOID;
  type->unsign = 1;
  type->align = M_TI_ALIGN_VOID;
  type->size = M_TI_SIZE_VOID;
  type->nbits = 0;
}

void
M_ti_bit_long (M_Type type, int n)
{
  type->type = M_TYPE_BIT_LONG;
  type->unsign = 1;
  type->align = M_TI_ALIGN_BIT;
  type->size = n * M_TI_SIZE_BIT;
  type->nbits = n * M_TI_SIZE_BIT;
  M_assert ((n <= 32),
	    "M_bit_long: do not allow bit field of more than 32 bits");
}

void
M_ti_bit_int (M_Type type, int n)
{
  type->type = M_TYPE_BIT_INT;
  type->unsign = 1;
  type->align = M_TI_ALIGN_BIT;
  type->size = n * M_TI_SIZE_BIT;
  type->nbits = n * M_TI_SIZE_BIT;
  M_assert ((n <= 32),
	    "M_bit_int: do not allow bit field of more than 32 bits");
}

void
M_ti_bit_short (M_Type type, int n)
{
  type->type = M_TYPE_BIT_SHORT;
  type->unsign = 1;
  type->align = M_TI_ALIGN_BIT;
  type->size = n * M_TI_SIZE_BIT;
  type->nbits = n * M_TI_SIZE_BIT;
  M_assert ((n <= 16),
	    "M_bit_long: do not allow bit field of more than 16 bits");
}

void
M_ti_bit_char (M_Type type, int n)
{
  type->type = M_TYPE_BIT_CHAR;
  type->unsign = 1;
  type->align = M_TI_ALIGN_BIT;
  type->size = n * M_TI_SIZE_BIT;
  type->nbits = n * M_TI_SIZE_BIT;

  /*
     M_assert ((n<=8), "M_bit_char: "
                       "do not allow bit field of more than 8 bits");
   */
}

void
M_ti_float (M_Type type, int unsign)
{
  type->type = M_TYPE_FLOAT;
  type->unsign = unsign;
  type->align = M_TI_ALIGN_FLOAT;
  type->size = M_TI_SIZE_FLOAT;
  type->nbits = M_TI_SIZE_FLOAT;
}

void
M_ti_double (M_Type type, int unsign)
{
  type->type = M_TYPE_DOUBLE;
  type->unsign = unsign;
  type->align = M_TI_ALIGN_DOUBLE;
  type->size = M_TI_SIZE_DOUBLE;
  type->nbits = M_TI_SIZE_DOUBLE;
}

void
M_ti_pointer (M_Type type)
{
  type->type = M_TYPE_POINTER;
  type->unsign = 1;
  type->align = M_TI_ALIGN_POINTER;
  type->size = M_TI_SIZE_POINTER;
  type->nbits = M_TI_SIZE_POINTER;
}

/*--------------------------------------------------------------------------*/
int
M_ti_eval_type (M_Type type, M_Type ntype)
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
M_ti_eval_type2 (M_Type type, M_Type ntype)
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
M_ti_call_type (M_Type type, M_Type ntype)
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
#if 0
/* commented out, because passing floats doesn't appear to work with
   library functions such as printf, although nowhere does it say that
   I have to use doubles						*/
      M_float (ntype, type->unsign);
      return (M_TYPE_FLOAT);
#endif
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
M_ti_call_type2 (M_Type type, M_Type ntype)
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
#if 0
/* commented out, because passing floats doesn't appear to work with
   library functions such as printf, although nowhere does it say that
   I have to use doubles						*/
      M_float (ntype, type->unsign);
      return (M_TYPE_FLOAT);
#endif
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
M_ti_array_layout (M_Type type, int *offset)
{
  *offset = 0;			/* assume first element is aligned */
}

int
M_ti_array_align (M_Type type)
{
  return type->align;
}

int
M_ti_array_size (M_Type type, int dim)
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
M_ti_union_layout (int n, _M_Type * type, int *offset, int *bit_offset)
{
  int i;
  for (i = 0; i < n; i++)
    {				/* assume the union is aligned */
      offset[i] = 0;
      bit_offset[i] = 0;
    }
}

int
M_ti_union_align (int n, _M_Type * type)
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
  if (max < M_TI_ALIGN_CHAR)
    max = M_TI_ALIGN_CHAR;

  return max;
}

int
M_ti_union_size (int n, _M_Type * type)
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
  if (max_align < M_TI_ALIGN_CHAR)
    max_align = M_TI_ALIGN_CHAR;

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
M_ti_struct_layout (int n, _M_Type * type, int *base, int *bit_offset)
{
  int i, offset;
  int mod, size, align;

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
      if (type[i].type == M_TYPE_BIT_LONG ||
	  type[i].type == M_TYPE_BIT_SHORT || type[i].type == M_TYPE_BIT_CHAR)
	{
	  int mod1 = offset % M_TI_SIZE_INT;
	  if ((mod1 + size) > M_TI_SIZE_INT)
	    offset += (M_TI_SIZE_INT - mod1);
	}

      mod = offset % align;	/* align to what the field */
      if (mod != 0)		/* needs to start from */
	offset += (align - mod);

      if (type[i].type == M_TYPE_BIT_LONG ||
	  type[i].type == M_TYPE_BIT_SHORT || type[i].type == M_TYPE_BIT_CHAR)
	{
	  int mod = offset % M_TI_SIZE_INT;
/*
	    base[i] = offset - mod + (M_TI_SIZE_INT - mod - size);
*/
	  bit_offset[i] = offset - mod + (M_TI_SIZE_INT - mod - size);
	  base[i] = offset;
	}
      else
	{
	  base[i] = offset;
	  bit_offset[i] = 0;
	}

      offset += size;		/* allocate space */
    }
}

int
M_ti_struct_align (int n, _M_Type * type)
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
  if (max < M_TI_ALIGN_CHAR)
    max = M_TI_ALIGN_CHAR;
  return max;
}

int
M_ti_struct_size (int n, _M_Type * type, int struct_align)
{
  int i, offset;
  int mod, size, align, max_align;
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
      if (type[i].type == M_TYPE_BIT_LONG || type[i].type == M_TYPE_BIT_CHAR)
	{
	  int mod1 = offset % M_TI_SIZE_INT;
	  if ((mod1 + size) > M_TI_SIZE_INT)
	    offset += (M_TI_SIZE_INT - mod1);
	}

      mod = offset % align;	/* align to what the field */
      if (mod != 0)		/* needs to start from */
	offset += (align - mod);

      offset += size;
    }
  /*
   * align to at least byte boundary.
   */
  if (max_align < M_TI_ALIGN_CHAR)
    max_align = M_TI_ALIGN_CHAR;
  /* enforce max. alignment */
  mod = offset % max_align;
  if (mod != 0)
    offset += (max_align - mod);
  return offset;
}


int
M_ti_layout_fnvar (List param_list, char **base_macro, int *pcount,
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

  size = M_ti_fnvar_layout (cnt, type, offset, mode, reg, paddr,
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
M_ti_fnvar_layout (int n, _M_Type * type, long int *offset, int *mode,
		   int *reg, int *paddr, char **macro,
		   int *su_sreg, int *su_ereg,
		   int *pcount, int is_st, int purpose)
					/* need to return structure */
{
  int i, max_align, off, int_rg, flt_rg;
  int size, align, mod, tp;
  int num_flt_params;

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

  max_align = M_TI_ALIGN_MAX;
  int_rg = 0;			/* Initialize number of parameters used yet */
  flt_rg = 0;			/* Initialize number of parameters used yet */
  num_flt_params = 0;
  off = 0;

  /* Scan the function and determine the number of float parameters    */
  /* Floats have priority for registers P4 and P5, but if there aren't */
  /* that many floats, then integer parameters will take them          */

  for (i = 0; i < n; i++)
    {
      tp = type[i].type;
      switch (tp)
	{
	case M_TYPE_FLOAT:
	case M_TYPE_DOUBLE:
	  num_flt_params++;
	}
    }

  for (i = 0; i < n; i++)
    {
      tp = type[i].type;

      if (use_standard_function_call_model)
	{
	  mode[i] = M_THRU_MEMORY;
	  reg[i] = -1;
	}
      else
	{
	  switch (tp)
	    {
	    case M_TYPE_CHAR:
	    case M_TYPE_SHORT:
	    case M_TYPE_INT:
	    case M_TYPE_LONG:
	    case M_TYPE_POINTER:

	      switch (num_flt_params)
		{
		case 0:
		  /* Occupy P0,P1,P2,P3,P4,P5 */
		  if (int_rg < M_TI_MAX_FNVAR_REG)
		    {
		      mode[i] = M_THRU_REGISTER;
		      reg[i] = (int_rg)++ + M_TI_INT_BASE;
		    }
		  else
		    {
		      mode[i] = M_THRU_MEMORY;
		      reg[i] = -1;
		    }
		  break;

		case 1:
		  /* Occupy P0,P1,P2,P3,P5  not P4 */
		  if (int_rg < M_TI_NUM_INT_PARM_REGS + 1)
		    {
		      mode[i] = M_THRU_REGISTER;

		      if (int_rg < M_TI_NUM_INT_PARM_REGS)
			reg[i] = (int_rg)++ + M_TI_INT_BASE;
		      else
			reg[i] = (int_rg)++ + M_TI_INT_BASE + 1;
		    }
		  else
		    {
		      mode[i] = M_THRU_MEMORY;
		      reg[i] = -1;
		    }
		  break;

		default:	/* 2 or greater float parameters */
		  /* Occupy P0,P1,P2,P3   */
		  if (int_rg < M_TI_NUM_INT_PARM_REGS)
		    {
		      mode[i] = M_THRU_REGISTER;
		      reg[i] = (int_rg)++ + M_TI_INT_BASE;
		    }
		  else
		    {
		      mode[i] = M_THRU_MEMORY;
		      reg[i] = -1;
		    }
		  break;
		}

	      break;

	    case M_TYPE_FLOAT:
	    case M_TYPE_DOUBLE:
	      if (flt_rg < M_TI_MAX_FNVAR_REG)
		{
		  mode[i] = M_THRU_REGISTER;
		  reg[i] = (flt_rg)++ + M_TI_FLT_BASE;
		}
	      else
		{
		  mode[i] = M_THRU_MEMORY;
		  reg[i] = -1;
		}
	      break;

	    case M_TYPE_UNION:
	    case M_TYPE_STRUCT:
	      /* Not done for TI yet */
	      size = type[i].size;
	      if (int_rg < M_TI_MAX_FNVAR_REG)
		{
		  mode[i] = M_INDIRECT_THRU_REGISTER;
		  reg[i] = (int_rg)++ + M_TI_INT_BASE;
		}
	      else
		{
		  if (size <= M_TI_SMALL_STRUCT_MAX)
		    mode[i] = M_THRU_MEMORY;
		  else
		    mode[i] = M_INDIRECT_THRU_MEMORY;
		  reg[i] = -1;
		}
	      break;
	    default:
	      M_assert (0, "M_fnvar_layout: argument is not promoted");
	    }
	}

      /* The TI convention has the stack growing towards high memory   */
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
	      size = M_TI_SIZE_INT;
	      align = M_TI_ALIGN_INT;
	    }
	}
      if (align >= M_TI_SMALL_STRUCT_MAX && type[i].type != M_TYPE_DOUBLE)
	/* anything larger than a 64-bit structure is passed */
	/* indirectly thru memory                            */
	align = M_TI_ALIGN_INT;
      else if (align < M_TI_ALIGN_INT)
	/* anything smaller that 32-bits is passed as 32-bits */
	align = M_TI_ALIGN_INT;

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
	   (tp == M_TYPE_STRUCT)) && (size <= M_TI_SMALL_STRUCT_MAX))
	{

	  align = M_TI_ALIGN_MAX;	/* must align to a double boundry */

	  mod = off % align;

	  if (mod != 0)
	    off += (align - mod);

	  off += size;

	  paddr[i] = -off;
	}
    }

  /* now large ones */
  for (i = 0; i < n; i++)
    {
      tp = type[i].type;

      size = type[i].size;

      if (((tp == M_TYPE_UNION) ||
	   (tp == M_TYPE_STRUCT)) && (size > M_TI_SMALL_STRUCT_MAX))
	{

	  align = M_TI_ALIGN_MAX;	/* must align to a double boundry */

	  mod = off % align;

	  if (mod != 0)
	    off += (align - mod);

	  off += size;

	  paddr[i] = -off;
	}
    }

  *pcount = flt_rg + int_rg;
  return off;			/* the total size needed */
}

/*--------------------------------------------------------------------------*/
int
M_ti_lvar_layout (int n, _M_Type * type, long int *offset, char **base_macro)
{
  int i, max_align, off;
  int size, align, mod, tp;
  /*
   *  the LOCAL section must be max. aligned initially
   */
  max_align = M_TI_ALIGN_MAX;
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
M_ti_no_short_int (void)
{
  return (M_TI_SIZE_SHORT == M_TI_SIZE_INT);
}
/*--------------------------------------------------------------------------*/
void
M_ti_cb_label_name (char *fn, int cb, char *line, int len)
{
  sprintf (line, "cb%d%s", cb, fn);
}
/*--------------------------------------------------------------------------*/
int
M_ti_is_cb_label (char *label, char *fn, int *cb)
{
  return (sscanf (label, "cb%d%s", cb, fn) == 2);
}
/*--------------------------------------------------------------------------*/
void
M_ti_jumptbl_label_name (char *fn, int tbl_id, char *line, int len)
{
  sprintf (line, "%s%s%d", fn, M_JUMPTBL_BASE_NAME, tbl_id);
}
/*--------------------------------------------------------------------------*/
/* Format for ti is: %sM_JUMPTBL_BASE_NAME%d, where %s is the func name */
int
M_ti_is_jumptbl_label (char *label, char *fn, int *tbl_id)
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
M_ti_structure_pointer (int purpose)
{
  return M_TI_RET_ST;
}
/*--------------------------------------------------------------------------*/
int
M_ti_return_register (int type, int purpose)
{
  switch (type)
    {
    case M_TYPE_INT:
      return M_TI_RET_I32;
    case M_TYPE_LONG:
      return M_TI_RET_I32;
    case M_TYPE_FLOAT:
      return M_TI_RET_F;
    case M_TYPE_DOUBLE:
      return M_TI_RET_F;
    default:
      return M_TI_RET_I32;
    }
}
/*--------------------------------------------------------------------------*/
/*
char *M_ti_fn_label_name(label)
*/
char *
M_ti_fn_label_name (char *label, int (*is_func) (char *is_func_label))
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
M_ti_fn_name_from_label (char *label)
{
  if (!strncmp (label, "_$fn", 4))
    return (label + 4);
  else
    return (label);
}

/*--------------------------------------------------------------------------*/
int
M_ti_fragile_macro (int macro_value)
{
  switch (M_model)
    {
    case M_TI_1:

      switch (macro_value)
	{
	case L_MAC_SP:
	case L_MAC_FP:
	case L_MAC_LV:
	case L_MAC_IP:
	case L_MAC_OP:
	case TI_MAC_ST:
	case L_MAC_LOCAL_SIZE:
	case L_MAC_PARAM_SIZE:
	case L_MAC_SWAP_SIZE:
	  return 0;
	default:
	  return 1;
	}
    default:
      M_assert (0, "M_ti_fragile_macro:  Illegal model specified!");
      return (0);
    }
}
/*--------------------------------------------------------------------------*/
int
M_ti_subroutine_call (int opc)
{
  switch (M_model)
    {
    case M_TI_1:
      return ((opc == Lop_JSR) || (opc == Lop_JSR_FS) ||
	      (opc == Lop_DIV) || (opc == Lop_DIV_U) ||
	      (opc == Lop_REM) || (opc == Lop_REM_U) ||
	      (opc == Lop_DIV_F) || (opc == Lop_DIV_F2));
    default:
      M_assert (0, "M_ti_subroutine_call:  Illegal model specified!");
      return (0);
    }
}
/*--------------------------------------------------------------------------*/
/*
 * Declare code generator specific macro registers to the front end parser.
 */
void
M_define_macros_ti (STRING_Symbol_Table * sym_tbl)
{
  M_add_symbol (sym_tbl, "r0", TI_MAC_COMPUTATION_REG0);
  M_add_symbol (sym_tbl, "r1", TI_MAC_COMPUTATION_REG1);
  M_add_symbol (sym_tbl, "r2", TI_MAC_COMPUTATION_REG2);
  M_add_symbol (sym_tbl, "r3", TI_MAC_COMPUTATION_REG3);

  M_add_symbol (sym_tbl, "a7", TI_MAC_AUXILIARY_REG7);

  M_add_symbol (sym_tbl, "ir0", TI_MAC_INDEX_REG0);
  M_add_symbol (sym_tbl, "ir1", TI_MAC_INDEX_REG1);

  /* repeat block registers */
  M_add_symbol (sym_tbl, "rs", TI_MAC_RS);
  M_add_symbol (sym_tbl, "re", TI_MAC_RE);
  M_add_symbol (sym_tbl, "rc", TI_MAC_RC);

  M_add_symbol (sym_tbl, "st", TI_MAC_ST);
  M_add_symbol (sym_tbl, "bk", TI_MAC_BK);

  M_add_symbol (sym_tbl, "dp", TI_MAC_DP);
  M_add_symbol (sym_tbl, "$true_sp", TI_MAC_TRUE_SP);

  /* 1 if leaf function, 0 if non-leaf */
  M_add_symbol (sym_tbl, "$leaf", TI_MAC_LEAF);

  /* total alloc requirements */
  M_add_symbol (sym_tbl, "$alloc_size", TI_MAC_ALLOC);

  /* number of integer and float callee saved registers used */
  M_add_symbol (sym_tbl, "$callee_i_regs", TI_MAC_CALLEE_I);
  M_add_symbol (sym_tbl, "$callee_f_regs", TI_MAC_CALLEE_F);
}

char *
M_get_macro_name_ti (int id)
{
  switch (id)
    {
    case TI_MAC_COMPUTATION_REG0:
      return "r0";
    case TI_MAC_COMPUTATION_REG1:
      return "r1";
    case TI_MAC_COMPUTATION_REG2:
      return "r2";
    case TI_MAC_COMPUTATION_REG3:
      return "r3";

    case TI_MAC_AUXILIARY_REG7:
      return "a7";

    case TI_MAC_INDEX_REG0:
      return "ir0";
    case TI_MAC_INDEX_REG1:
      return "ir1";
    case TI_MAC_RS:
      return "rs";
    case TI_MAC_RE:
      return "re";
    case TI_MAC_RC:
      return "rc";

    case TI_MAC_ST:
      return "st";
    case TI_MAC_BK:
      return "bk";

    case TI_MAC_DP:
      return "dp";
    case TI_MAC_LEAF:
      return "$leaf";
    case TI_MAC_ALLOC:
      return "$alloc_size";
    case TI_MAC_CALLEE_I:
      return "$callee_i_regs";
    case TI_MAC_CALLEE_F:
      return "$callee_f_regs";
    case TI_MAC_TRUE_SP:
      return "$true_sp";

    default:
      return "?";
    }
}

void
M_define_opcode_name_ti (STRING_Symbol_Table * sym_tbl)
{

  M_add_symbol (sym_tbl, TIopcode_ALU_DIRECT, LTIop_ALU_DIRECT);
  M_add_symbol (sym_tbl, TIopcode_ALU_INDIRECT, LTIop_ALU_INDIRECT);

  M_add_symbol (sym_tbl, TIopcode_ALU_REG_INDIRECT, LTIop_ALU_REG_INDIRECT);
  M_add_symbol (sym_tbl, TIopcode_ALU_INDIRECT_REG, LTIop_ALU_INDIRECT_REG);
  M_add_symbol (sym_tbl, TIopcode_ALU_INDIRECT_INDIRECT,
		LTIop_ALU_INDIRECT_INDIRECT);
  M_add_symbol (sym_tbl, TIopcode_ALU_REG_DIRECT, LTIop_ALU_REG_DIRECT);

  M_add_symbol (sym_tbl, TIopcode_BS, LTIop_BS);
  M_add_symbol (sym_tbl, TIopcode_BD, LTIop_BD);
  M_add_symbol (sym_tbl, TIopcode_BR, LTIop_BR);
  M_add_symbol (sym_tbl, TIopcode_BRD, LTIop_BRD);
  M_add_symbol (sym_tbl, TIopcode_BU, LTIop_BU);
  M_add_symbol (sym_tbl, TIopcode_BUD, LTIop_BUD);
  M_add_symbol (sym_tbl, TIopcode_DB, LTIop_DB);
  M_add_symbol (sym_tbl, TIopcode_DBD, LTIop_DBD);
  M_add_symbol (sym_tbl, TIopcode_CALL, LTIop_CALL);
  M_add_symbol (sym_tbl, TIopcode_RETS, LTIop_RETS);
  M_add_symbol (sym_tbl, TIopcode_REPEAT_BLOCK, LTIop_REPEAT_BLOCK);
  M_add_symbol (sym_tbl, TIopcode_REPEAT_INSTRUCTION,
		LTIop_REPEAT_INSTRUCTION);
  M_add_symbol (sym_tbl, TIopcode_POP, LTIop_POP);
  M_add_symbol (sym_tbl, TIopcode_POPF, LTIop_POPF);
  M_add_symbol (sym_tbl, TIopcode_PUSH, LTIop_PUSH);
  M_add_symbol (sym_tbl, TIopcode_PUSHF, LTIop_PUSHF);
  M_add_symbol (sym_tbl, TIopcode_SUBRI, LTIop_SUBRI);
  M_add_symbol (sym_tbl, TIopcode_SUBRF, LTIop_SUBRF);
  M_add_symbol (sym_tbl, TIopcode_CALLU, LTIop_CALLU);

  M_add_symbol (sym_tbl, TIopcode_PAR_STORE, LTIop_PAR_STORE);
  M_add_symbol (sym_tbl, TIopcode_PAR_LOAD, LTIop_PAR_LOAD);
  M_add_symbol (sym_tbl, TIopcode_PAR_ONE_OPERAND, LTIop_PAR_ONE_OPERAND);
  M_add_symbol (sym_tbl, TIopcode_PAR_TWO_OPERAND, LTIop_PAR_TWO_OPERAND);
}

char *
M_get_opcode_name_ti (int id)
{
  switch (id)
    {

    case LTIop_ALU_DIRECT:
      return (TIopcode_ALU_DIRECT);
    case LTIop_ALU_INDIRECT:
      return (TIopcode_ALU_INDIRECT);

    case LTIop_ALU_REG_INDIRECT:
      return (TIopcode_ALU_REG_INDIRECT);
    case LTIop_ALU_INDIRECT_REG:
      return (TIopcode_ALU_INDIRECT_REG);
    case LTIop_ALU_INDIRECT_INDIRECT:
      return (TIopcode_ALU_INDIRECT_INDIRECT);
    case LTIop_ALU_REG_DIRECT:
      return (TIopcode_ALU_REG_DIRECT);

    case LTIop_BS:
      return (TIopcode_BS);
    case LTIop_BD:
      return (TIopcode_BD);
    case LTIop_BR:
      return (TIopcode_BR);
    case LTIop_BRD:
      return (TIopcode_BRD);
    case LTIop_BU:
      return (TIopcode_BU);
    case LTIop_BUD:
      return (TIopcode_BUD);
    case LTIop_DB:
      return (TIopcode_DB);
    case LTIop_DBD:
      return (TIopcode_DBD);
    case LTIop_CALL:
      return (TIopcode_CALL);
    case LTIop_RETS:
      return (TIopcode_RETS);
    case LTIop_REPEAT_BLOCK:
      return (TIopcode_REPEAT_BLOCK);
    case LTIop_REPEAT_INSTRUCTION:
      return (TIopcode_REPEAT_INSTRUCTION);
    case LTIop_POP:
      return (TIopcode_POP);
    case LTIop_POPF:
      return (TIopcode_POPF);
    case LTIop_PUSH:
      return (TIopcode_PUSH);
    case LTIop_PUSHF:
      return (TIopcode_PUSHF);
    case LTIop_SUBRI:
      return (TIopcode_SUBRI);
    case LTIop_SUBRF:
      return (TIopcode_SUBRF);
    case LTIop_CALLU:
      return (TIopcode_CALLU);

    case LTIop_PAR_STORE:
      return (TIopcode_PAR_STORE);
    case LTIop_PAR_LOAD:
      return (TIopcode_PAR_LOAD);
    case LTIop_PAR_ONE_OPERAND:
      return (TIopcode_PAR_ONE_OPERAND);
    case LTIop_PAR_TWO_OPERAND:
      return (TIopcode_PAR_TWO_OPERAND);

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
M_oper_supported_in_arch_ti (int opc)
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
M_num_oper_required_for_ti (L_Oper * oper, char *name)
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
      return (1);

    case Lop_LD_UC:
    case Lop_LD_C:
    case Lop_LD_UC2:
    case Lop_LD_C2:
    case Lop_LD_I:
      return (1);

    case Lop_ST_F:
    case Lop_ST_F2:
      return (1);

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
M_is_stack_operand_ti (L_Operand * operand)
{
  if (L_is_macro (operand) &&
      (operand->value.mac == L_MAC_SP ||
       operand->value.mac == L_MAC_FP ||
       operand->value.mac == TI_MAC_TRUE_SP ||
       operand->value.mac == L_MAC_SAFE_MEM ||
       operand->value.mac == L_MAC_IP ||
       operand->value.mac == L_MAC_OP ||
       operand->value.mac == L_MAC_RS || operand->value.mac == L_MAC_LV))
    return (1);

  return (0);
}

int
M_is_unsafe_macro_ti (L_Operand * operand)
{
  if (!L_is_macro (operand))
    return (0);

  switch (operand->value.mac)
    {
    case TI_MAC_COMPUTATION_REG0:
    case TI_MAC_COMPUTATION_REG1:
    case L_MAC_SP:
    case L_MAC_FP:
    case L_MAC_LV:
    case L_MAC_IP:
    case L_MAC_OP:
    case TI_MAC_ST:
    case L_MAC_LOCAL_SIZE:
    case L_MAC_PARAM_SIZE:
    case L_MAC_SWAP_SIZE:
      return 1;
    default:
      return 0;
    }
}

int
M_operand_type_ti (L_Operand * operand)
{
  /* If NULL operand pointer, then return MDES_OPERAND_NULL */
  if (operand == NULL)
    return (MDES_OPERAND_NULL);

  switch (L_operand_case_type (operand))
    {
    case L_OPERAND_INT:
      if (FIELD_16 (operand->value.i))
	return (MDES_OPERAND_Lit16);
      else
	return (MDES_OPERAND_Lit24);

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
      M_assert (0, "M_operand_type_ti: Unknown type");
      return (0);
    }
}

int
M_conflicting_operands_ti (L_Operand * operand, L_Operand ** conflict_array,
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
	case L_MAC_SP:
	case TI_MAC_TRUE_SP:
	case L_MAC_SAFE_MEM:
	  conflict_array[0] =
	    L_new_macro_operand (L_MAC_SP, L_CTYPE_INT, L_PTYPE_NULL);
	  conflict_array[1] =
	    L_new_macro_operand (TI_MAC_TRUE_SP, L_CTYPE_INT, L_PTYPE_NULL);
	  conflict_array[2] =
	    L_new_macro_operand (L_MAC_SAFE_MEM, L_CTYPE_INT, L_PTYPE_NULL);
	  return (3);
	case TI_MAC_COMPUTATION_REG0:
	  conflict_array[0] =
	    L_new_macro_operand (TI_MAC_COMPUTATION_REG0, L_CTYPE_INT,
				 L_PTYPE_NULL);
	  conflict_array[1] =
	    L_new_macro_operand (TI_MAC_COMPUTATION_REG0, L_CTYPE_FLOAT,
				 L_PTYPE_NULL);
	  return (2);
	case TI_MAC_COMPUTATION_REG1:
	  conflict_array[0] =
	    L_new_macro_operand (TI_MAC_COMPUTATION_REG1, L_CTYPE_INT,
				 L_PTYPE_NULL);
	  conflict_array[1] =
	    L_new_macro_operand (TI_MAC_COMPUTATION_REG1, L_CTYPE_FLOAT,
				 L_PTYPE_NULL);
	  return (2);
	case TI_MAC_COMPUTATION_REG2:
	  conflict_array[0] =
	    L_new_macro_operand (TI_MAC_COMPUTATION_REG2, L_CTYPE_INT,
				 L_PTYPE_NULL);
	  conflict_array[1] =
	    L_new_macro_operand (TI_MAC_COMPUTATION_REG2, L_CTYPE_FLOAT,
				 L_PTYPE_NULL);
	  return (2);
	case TI_MAC_COMPUTATION_REG3:
	  conflict_array[0] =
	    L_new_macro_operand (TI_MAC_COMPUTATION_REG3, L_CTYPE_INT,
				 L_PTYPE_NULL);
	  conflict_array[1] =
	    L_new_macro_operand (TI_MAC_COMPUTATION_REG3, L_CTYPE_FLOAT,
				 L_PTYPE_NULL);
	  return (2);

	default:
	  conflict_array[0] = L_copy_operand (operand);
	  return (1);
	}
    }
  else if (L_is_reg (operand))
    {
      conflict_array[0] = L_copy_operand (operand);
      return (1);
    }
  else
    M_assert (0, "Lti_conflicting_operands: unsupported operand type");
  return 1;
}

int
M_num_registers_ti (int ctype)
{
  /* These numbers are estimates */
  if (M_model == M_TI_1)
    {
      switch (ctype)
	{
	case L_CTYPE_INT:
	  return (16);
	case L_CTYPE_FLOAT:
	  return (8);
	case L_CTYPE_DOUBLE:
	  return (8);
	default:
	  return (0);
	}
    }
  else
    {
      M_assert (0, "M_num_registers_ti: unsupported model");
    }
  return (0);
}
