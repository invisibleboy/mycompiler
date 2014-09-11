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
 *	File:	ml_spec.c
 *	Author:	Pohua Chang, Wen-mei Hwu
 *	Creation Date:	June, 1990
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
#include <machine/m_hppa.h>
#include <machine/m_impact.h>
#include <machine/m_hpl_pd.h>
#include <machine/m_sparc.h>
#include <machine/m_x86.h>
#include <machine/m_ti.h>
#include <machine/m_sh.h>
#include <machine/m_bx86.h>
#include <machine/m_tahoe.h>
#include <machine/m_starcore.h>
#include <machine/m_arm.h>
#include <machine/m_wims.h>

#define TEST_SIZE

/*
 *	Global variables
 */
extern int M_arch;
extern int M_model;

/*
 *	The following functions are for documentation only.
 *	The function overhead is too great for actual usage.
 */
void
M_void (M_Type type)
{
  /* Use layout database (if specified) to determine info -ITI/JCG 2/99 */
  if (M_use_layout_database)
    {
      type->type = M_TYPE_VOID;
      type->unsign = 1;
      type->align = -1;		/* No database read necessary, 
				   IMPACT convention */
      type->size = 0;		/* No database read necessary, 
				   IMPACT convention */
      type->nbits = 0;		/* No database read necessary, 
				   IMPACT convention */
    }
  /* Otherwise use orig hard-coded method to determine info -ITI(JCG) 2/99 */
  else
    {
      switch (M_arch)
	{
	case M_IMPACT:
	  M_impact_void (type);
	  break;
	case M_TAHOE:
	  M_tahoe_void (type);
	  break;
	case M_SPARC:
	  M_sparc_void (type);
	  break;
	case M_HPPA:
	  M_hppa_void (type);
	  break;
	case M_X86:
	  M_x86_void (type);
	  break;
	case M_PLAYDOH:
	  M_playdoh_void (type);
	  break;
	case M_TI:
	  M_ti_void (type);
	  break;
	case M_SH:
	  M_sh_void (type);
	  break;
	case M_BX86:
	  M_bx86_void (type);
	  break;
	case M_STARCORE:
	  M_starcore_void (type);
	  break;
	case M_ARM:
	  M_arm_void (type);
	  break;
	case M_WIMS:
	  M_wims_void (type);
	  break;
	default:
	  M_assert (0, "M_void: illegal M_arch type");
	}
    }
  type->flags = 0;
}

void
M_bit_llong (M_Type type, int n, int unsign)
{
  /* Use layout database (if specified) to determine info -ITI(JCG) 2/99 */
  if (M_use_layout_database)
    {
      type->type = M_TYPE_BIT_LLONG;
      type->unsign = (unsign != 0);
      type->align = 1;		/* No database read necessary, 
				   IMPACT convention */
      type->size = n;		/* No database read necessary, 
				   IMPACT convention */
      type->nbits = n;		/* No database read necessary, 
				   IMPACT convention */
      if (n > M_read_database_i ("_HT__base_types", "longlong", "size"))
	{
	  I_punt ("M_bit_long: bit field (%i) more than %i bits",
		  n, M_read_database_i ("_HT__base_types", "longlong",
					"size"));
	}
    }
  /* Otherwise use orig hard-coded method to determine info -ITI(JCG) 2/99 */
  else
    {
      switch (M_arch)
	{
	case M_TAHOE:
	  M_tahoe_bit_llong (type, n);
	  break;
	default:
	  M_assert (0, "M_bit_llong: illegal M_arch type");
	}
    }
  type->flags = 0;
}

/* Added 'unsign' parameter, phase in use -ITI(JCG) 2/99 */
void
M_bit_long (M_Type type, int n, int unsign)
{
  /* Use layout database (if specified) to determine info -ITI(JCG) 2/99 */
  if (M_use_layout_database)
    {
      type->type = M_TYPE_BIT_LONG;
      type->unsign = (unsign != 0);
      type->align = 1;		/* No database read necessary, 
				   IMPACT convention */
      type->size = n;		/* No database read necessary, 
				   IMPACT convention */
      type->nbits = n;		/* No database read necessary, 
				   IMPACT convention */
      if (n > M_read_database_i ("_HT__base_types", "long", "size"))
	{
	  I_punt ("M_bit_long: bit field (%i) more than %i bits",
		  n, M_read_database_i ("_HT__base_types", "long", "size"));
	}
    }
  /* Otherwise use orig hard-coded method to determine info -ITI(JCG) 2/99 */
  else
    {
      switch (M_arch)
	{
	case M_IMPACT:
	  M_impact_bit_long (type, n);
	  break;
	case M_TAHOE:
	  M_tahoe_bit_long (type, n);
	  break;
	case M_SPARC:
	  M_sparc_bit_long (type, n);
	  break;
	case M_HPPA:
	  M_hppa_bit_long (type, n);
	  break;
	case M_X86:
	  M_x86_bit_long (type, n);
	  break;
	case M_PLAYDOH:
	  M_playdoh_bit_long (type, n);
	  break;
	case M_TI:
	  M_ti_bit_long (type, n);
	  break;
	case M_SH:
	  M_sh_bit_long (type, n);
	  break;
	case M_BX86:
	  M_bx86_bit_long (type, n);
	  break;
	case M_STARCORE:
	  M_starcore_bit_long (type, n);
	  break;
	case M_ARM:
	  M_arm_bit_long (type, n);
	  break;
	case M_WIMS:
	  M_wims_bit_long (type, n);
          break;
	default:
	  M_assert (0, "M_bit_long: illegal M_arch type");
	}
    }
  type->flags = 0;
}


void
M_bit_int (M_Type type, int n, int unsign)
{
  /* Use layout database (if specified) to determine info -ITI(JCG) 2/99 */
  if (M_use_layout_database)
    {
      type->type = M_TYPE_BIT_INT;
      type->unsign = (unsign != 0);
      type->align = 1;		/* No database read necessary, 
				   IMPACT convention */
      type->size = n;		/* No database read necessary, 
				   IMPACT convention */
      type->nbits = n;		/* No database read necessary, 
				   IMPACT convention */
      if (n > M_read_database_i ("_HT__base_types", "int", "size"))
	{
	  I_punt ("M_bit_int: bit field (%i) more than %i bits",
		  n, M_read_database_i ("_HT__base_types", "int", "size"));
	}
    }
  /* Otherwise use orig hard-coded method to determine info -ITI(JCG) 2/99 */
  else
    {
      switch (M_arch)
	{
	case M_IMPACT:
	  M_impact_bit_int (type, n);
	  break;
	case M_TAHOE:
	  M_tahoe_bit_int (type, n);
	  break;
	case M_SPARC:
	  M_sparc_bit_int (type, n);
	  break;
	case M_HPPA:
	  M_hppa_bit_int (type, n);
	  break;
	case M_X86:
	  M_x86_bit_int (type, n);
	  break;
	case M_PLAYDOH:
	  M_playdoh_bit_int (type, n);
	  break;
	case M_TI:
	  M_ti_bit_int (type, n);
	  break;
	case M_SH:
	  M_sh_bit_int (type, n);
	  break;
	case M_BX86:
	  M_bx86_bit_int (type, n);
	  break;
	case M_STARCORE:
	  M_starcore_bit_int (type, n);
	  break;
	case M_ARM:
	  M_arm_bit_int (type, n);
	  break;
	case M_WIMS:
	  M_wims_bit_int (type, n);
          break;
	default:
	  M_assert (0, "M_bit_int: illegal M_arch type");
	}
    }
  type->flags = 0;
}

/* Added 'unsign' parameter, phase in use -ITI(JCG) 2/99 */
void
M_bit_short (M_Type type, int n, int unsign)
{
  /* Use layout database (if specified) to determine info -ITI(JCG) 2/99 */
  if (M_use_layout_database)
    {
      type->type = M_TYPE_BIT_SHORT;
      type->unsign = (unsign != 0);
      type->align = 1;		/* No database read necessary, 
				   IMPACT convention */
      type->size = n;		/* No database read necessary, 
				   IMPACT convention */
      type->nbits = n;		/* No database read necessary, 
				   IMPACT convention */
      if (n > M_read_database_i ("_HT__base_types", "short", "size"))
	{
	  I_punt ("M_bit_short: bit field (%i) more than %i bits",
		  n, M_read_database_i ("_HT__base_types", "short", "size"));
	}
    }
  /* Otherwise use orig hard-coded method to determine info -ITI(JCG) 2/99 */
  else
    {
      switch (M_arch)
	{
	case M_IMPACT:
	  M_impact_bit_short (type, n);
	  break;
	case M_TAHOE:
	  M_tahoe_bit_short (type, n);
	  break;
	case M_SPARC:
	  M_sparc_bit_short (type, n);
	  break;
	case M_HPPA:
	  M_hppa_bit_short (type, n);
	  break;
	case M_X86:
	  M_x86_bit_short (type, n);
	  break;
	case M_PLAYDOH:
	  M_playdoh_bit_short (type, n);
	  break;
	case M_TI:
	  M_ti_bit_short (type, n);
	  break;
	case M_SH:
	  M_sh_bit_short (type, n);
	  break;
	case M_BX86:
	  M_bx86_bit_short (type, n);
	  break;
	case M_STARCORE:
	  M_starcore_bit_short (type, n);
	  break;
	case M_ARM:
	  M_arm_bit_short (type, n);
	  break;
	case M_WIMS:
	  M_wims_bit_short (type, n);
	  break;  
	default:
	  M_assert (0, "M_bit_short: illegal M_arch type");
	}
    }
  type->flags = 0;
}

/* Added 'unsign' parameter, phase in use -ITI(JCG) 2/99 */
void
M_bit_char (M_Type type, int n, int unsign)
{
  /* Use layout database (if specified) to determine info -ITI(JCG) 2/99 */
  if (M_use_layout_database)
    {
      type->type = M_TYPE_BIT_CHAR;
      type->unsign = (unsign != 0);
      type->align = 1;		/* No database read necessary, 
				   IMPACT convention */
      type->size = n;		/* No database read necessary, 
				   IMPACT convention */
      type->nbits = n;		/* No database read necessary, 
				   IMPACT convention */
      if (n > M_read_database_i ("_HT__base_types", "char", "size"))
	{
	  I_punt ("M_bit_char: bit field (%i) more than %i bits",
		  n, M_read_database_i ("_HT__base_types", "char", "size"));
	}
    }
  /* Otherwise use orig hard-coded method to determine info -ITI(JCG) 2/99 */
  else
    {
      switch (M_arch)
	{
	case M_IMPACT:
	  M_impact_bit_char (type, n);
	  break;
	case M_TAHOE:
	  M_tahoe_bit_char (type, n);
	  break;
	case M_SPARC:
	  M_sparc_bit_char (type, n);
	  break;
	case M_HPPA:
	  M_hppa_bit_char (type, n);
	  break;
	case M_X86:
	  M_x86_bit_char (type, n);
	  break;
	case M_PLAYDOH:
	  M_playdoh_bit_char (type, n);
	  break;
	case M_TI:
	  M_ti_bit_char (type, n);
	  break;
	case M_SH:
	  M_sh_bit_char (type, n);
	  break;
	case M_BX86:
	  M_bx86_bit_char (type, n);
	  break;
	case M_STARCORE:
	  M_starcore_bit_char (type, n);
	  break;
	case M_ARM:
	  M_arm_bit_char (type, n);
	  break;
	case M_WIMS:
	  M_wims_bit_char (type, n);
	  break;
	default:
	  M_assert (0, "M_bit_char: illegal M_arch type");
	}
    }
  type->flags = 0;
}

void
M_float (M_Type type, int unsign)
{
  /* Use layout database (if specified) to determine info -ITI(JCG) 2/99 */
  if (M_use_layout_database)
    {
      type->type = M_TYPE_FLOAT;
      type->unsign = (unsign != 0);
      type->align = M_read_database_i ("_HT__base_types", "float", "align");
      type->size = M_read_database_i ("_HT__base_types", "float", "size");
      type->nbits = M_read_database_i ("_HT__base_types", "float", "size");
    }
  /* Otherwise use orig hard-coded method to determine info -ITI(JCG) 2/99 */
  else
    {
      switch (M_arch)
	{
	case M_IMPACT:
	  M_impact_float (type, unsign);
	  break;
	case M_TAHOE:
	  M_tahoe_float (type, unsign);
	  break;
	case M_SPARC:
	  M_sparc_float (type, unsign);
	  break;
	case M_HPPA:
	  M_hppa_float (type, unsign);
	  break;
	case M_X86:
	  M_x86_float (type, unsign);
	  break;
	case M_PLAYDOH:
	  M_playdoh_float (type, unsign);
	  break;
	case M_TI:
	  M_ti_float (type, unsign);
	  break;
	case M_SH:
	  M_sh_float (type, unsign);
	  break;
	case M_BX86:
	  M_bx86_float (type, unsign);
	  break;
	case M_STARCORE:
	  M_starcore_float (type, unsign);
	  break;
	case M_ARM:
	  M_arm_float (type, unsign);
	  break;
	case M_WIMS:
	  M_wims_float (type, unsign);
 	  break;
	default:
	  M_assert (0, "M_float: illegal M_arch type");
	}
    }
  type->flags = 0;
}

void
M_double (M_Type type, int unsign)
{
  /* Use layout database (if specified) to determine info -ITI(JCG) 2/99 */
  if (M_use_layout_database)
    {
      type->type = M_TYPE_DOUBLE;
      type->unsign = (unsign != 0);
      type->align = M_read_database_i ("_HT__base_types", "double", "align");
      type->size = M_read_database_i ("_HT__base_types", "double", "size");
      type->nbits = M_read_database_i ("_HT__base_types", "double", "size");
    }
  /* Otherwise use orig hard-coded method to determine info -ITI(JCG) 2/99 */
  else
    {
      switch (M_arch)
	{
	case M_IMPACT:
	  M_impact_double (type, unsign);
	  break;
	case M_TAHOE:
	  M_tahoe_double (type, unsign);
	  break;
	case M_SPARC:
	  M_sparc_double (type, unsign);
	  break;
	case M_HPPA:
	  M_hppa_double (type, unsign);
	  break;
	case M_X86:
	  M_x86_double (type, unsign);
	  break;
	case M_PLAYDOH:
	  M_playdoh_double (type, unsign);
	  break;
	case M_TI:
	  M_ti_double (type, unsign);
	  break;
	case M_SH:
	  M_sh_double (type, unsign);
	  break;
	case M_BX86:
	  M_bx86_double (type, unsign);
	  break;
	case M_STARCORE:
	  M_starcore_double (type, unsign);
	  break;
	case M_ARM:
	  M_arm_double (type, unsign);
	  break;
	case M_WIMS:
	  M_wims_double (type, unsign);
	  break;
	default:
	  M_assert (0, "M_double: illegal M_arch type");
	}
    }
  type->flags = 0;
}

void
M_pointer (M_Type type)
{
  /* Use layout database (if specified) to determine info -ITI(JCG) 2/99 */
  if (M_use_layout_database)
    {
      type->type = M_TYPE_POINTER;
      type->unsign = 1;
      type->align = M_read_database_i ("_HT__base_types", "void *", "align");
      type->size = M_read_database_i ("_HT__base_types", "void *", "size");
      type->nbits = M_read_database_i ("_HT__base_types", "void *", "size");
    }
  /* Otherwise use orig hard-coded method to determine info -ITI(JCG) 2/99 */
  else
    {
      switch (M_arch)
	{
	case M_IMPACT:
	  M_impact_pointer (type);
	  break;
	case M_TAHOE:
	  M_tahoe_pointer (type);
	  break;
	case M_SPARC:
	  M_sparc_pointer (type);
	  break;
	case M_HPPA:
	  M_hppa_pointer (type);
	  break;
	case M_X86:
	  M_x86_pointer (type);
	  break;
	case M_PLAYDOH:
	  M_playdoh_pointer (type);
	  break;
	case M_TI:
	  M_ti_pointer (type);
	  break;
	case M_SH:
	  M_sh_pointer (type);
	  break;
	case M_BX86:
	  M_bx86_pointer (type);
	  break;
	case M_STARCORE:
	  M_starcore_pointer (type);
	  break;
	case M_ARM:
	  M_arm_pointer (type);
	  break;
	case M_WIMS:
	  M_wims_pointer (type);
	  break;
	default:
	  M_assert (0, "M_pointer: illegal M_arch type");
	}
    }
  type->flags = 0;
}

void
M_union (M_Type type, int align, int size)
{
  type->type = M_TYPE_UNION;
  type->unsign = 1;
  type->align = align;
  type->size = size;
  type->nbits = size;
#ifdef TEST_SIZE
  if (size < 0)
    M_assert (0, "M_union: size cannot be negative");
  if (align < 0)
    M_assert (0, "M_union: align cannot be negative");
#endif
  type->flags = 0;
}

void
M_struct (M_Type type, int align, int size)
{
  type->type = M_TYPE_STRUCT;
  type->unsign = 1;
  type->align = align;
  type->size = size;
  type->nbits = size;
#ifdef TEST_SIZE
  if (size < 0)
    M_assert (0, "M_struct: size cannot be negative");
  if (align < 0)
    M_assert (0, "M_struct: align cannot be negative");
#endif
  type->flags = 0;
}

void
M_block (M_Type type, int align, int size)
{
  type->type = M_TYPE_BLOCK;
  type->unsign = 1;
  type->align = align;
  type->size = size;
  type->nbits = size;
#ifdef TEST_SIZE
  if (type->size < 0)
    M_assert (0, "M_block: size cannot be negative");
  if (type->align < 0)
    M_assert (0, "M_block: align cannot be negative");
#endif
}

int
M_eval_type (M_Type type, M_Type ntype)
{
  int native_mtype = M_native_int_register_mtype ();

  /* Use layout database (if specified) to determine info -ITI(JCG) 2/99 */
  if (M_use_layout_database)
    {
      /* Based on hppa version, which seems wierd.  However, since
       * M_eval_type2 does what I expect it to, is seems unwise to change
       * this code. -ITI (JCG) 2/99 */
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
	case M_TYPE_LLONG:
	case M_TYPE_POINTER:
	case M_TYPE_BLOCK:	/* the starting address of array is used */
	  /* The promoted size is machine dependent. MCM 7/00 */
	  if (native_mtype == M_TYPE_INT)
	    M_int (ntype, type->unsign);
	  else if (native_mtype == M_TYPE_LLONG)
	    M_llong (ntype, type->unsign);
	  else
	    I_punt ("M_eval_type: Invalid native machine mtype %d\n",
		    native_mtype);
	  return (ntype->type);
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
	  return (-1);
	}
    }
  /* Otherwise use orig hard-coded method to determine info -ITI(JCG) 2/99 */
  else
    {
      switch (M_arch)
	{
	case M_IMPACT:
	  return M_impact_eval_type (type, ntype);
	case M_TAHOE:
	  return M_tahoe_eval_type (type, ntype);
	case M_SPARC:
	  return M_sparc_eval_type (type, ntype);
	case M_HPPA:
	  return M_hppa_eval_type (type, ntype);
	case M_X86:
	  return M_x86_eval_type (type, ntype);
	case M_PLAYDOH:
	  return M_playdoh_eval_type (type, ntype);
	case M_TI:
	  return M_ti_eval_type (type, ntype);
	case M_SH:
	  return M_sh_eval_type (type, ntype);
	case M_BX86:
	  return M_bx86_eval_type (type, ntype);
	case M_STARCORE:
	  return M_starcore_eval_type (type, ntype);
	case M_ARM:
	  return M_arm_eval_type (type, ntype);
	case M_WIMS:
	  return M_wims_eval_type (type, ntype);
	default:
	  M_assert (0, "M_eval_type: illegal M_arch type");
	  return -1;
	}
    }
}

int
M_eval_type2 (M_Type type, M_Type ntype)
{
  /* Use layout database (if specified) to determine info -ITI(JCG) 2/99 */
  if (M_use_layout_database)
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
	case M_TYPE_BIT_SHORT:	/* -ITI(JCG) 2/99 */
	case M_TYPE_SHORT:
	  M_short (ntype, type->unsign);
	  return M_TYPE_SHORT;
	case M_TYPE_BLOCK:	/* the starting address of array is used */
	case M_TYPE_INT:
	  M_int (ntype, type->unsign);
	  return M_TYPE_INT;
	case M_TYPE_BIT_LONG:
	case M_TYPE_LONG:
	  M_long (ntype, type->unsign);
	  return M_TYPE_LONG;
	case M_TYPE_BIT_LLONG:
	case M_TYPE_LLONG:
	  M_llong (ntype, type->unsign);
	  return M_TYPE_LLONG;
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
	default:		/* -ITI(JCG) 2/99 */
	  return (-1);
	}
    }
  /* Otherwise use orig hard-coded method to determine info -ITI(JCG) 2/99 */
  else
    {
      switch (M_arch)
	{
	case M_IMPACT:
	  return M_impact_eval_type2 (type, ntype);
	case M_TAHOE:
	  return M_tahoe_eval_type2 (type, ntype);
	case M_SPARC:
	  return M_sparc_eval_type2 (type, ntype);
	case M_HPPA:
	  return M_hppa_eval_type2 (type, ntype);
	case M_X86:
	  return M_x86_eval_type2 (type, ntype);
	case M_PLAYDOH:
	  return M_playdoh_eval_type2 (type, ntype);
	case M_TI:
	  return M_ti_eval_type2 (type, ntype);
	case M_SH:
	  return M_sh_eval_type2 (type, ntype);
	case M_BX86:
	  return M_bx86_eval_type2 (type, ntype);
	case M_STARCORE:
	  return M_starcore_eval_type2 (type, ntype);
	case M_ARM:
	  return M_arm_eval_type2 (type, ntype);
	case M_WIMS:
	  return M_wims_eval_type2 (type, ntype);
	default:
	  M_assert (0, "M_eval_type2: illegal M_arch type");
	  return -1;
	}
    }
}

int
M_call_type (M_Type type, M_Type ntype)
{
  int native_mtype = M_native_int_register_mtype ();

  /* Use layout database (if specified) to determine info -ITI(JCG) 2/99 */
  /* Used when parameter ctypes are not propagated, rather the parmeters
     are promoted to machine word size. */
  if (M_use_layout_database)
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
	case M_TYPE_LLONG:
	case M_TYPE_POINTER:
	case M_TYPE_BLOCK:	/* the starting address of array is used */
	  /* The promoted size is machine dependent. MCM 7/00 */
	  if (native_mtype == M_TYPE_INT)
	    M_int (ntype, type->unsign);
	  else if (native_mtype == M_TYPE_LLONG)
	    M_llong (ntype, type->unsign);
	  else
	    I_punt ("M_call_type: Invalid native machine mtype %d\n",
		    native_mtype);
	  return (ntype->type);
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
	  return type->type;
	default:
	  return (-1);
	}
    }
  /* Otherwise use orig hard-coded method to determine info -ITI(JCG) 2/99 */
  else
    {
      switch (M_arch)
	{
	case M_IMPACT:
	  return M_impact_call_type (type, ntype);
	case M_TAHOE:
	  return M_tahoe_call_type (type, ntype);
	case M_SPARC:
	  return M_sparc_call_type (type, ntype);
	case M_HPPA:
	  return M_hppa_call_type (type, ntype);
	case M_X86:
	  return M_x86_call_type (type, ntype);
	case M_PLAYDOH:
	  return M_playdoh_call_type (type, ntype);
	case M_TI:
	  return M_ti_call_type (type, ntype);
	case M_SH:
	  return M_sh_call_type (type, ntype);
	case M_STARCORE:
	  return M_starcore_call_type (type, ntype);
	case M_ARM:
	  return M_arm_call_type (type, ntype);
	case M_WIMS:
	  return M_wims_call_type (type, ntype);
	default:
	  M_assert (0, "M_call_type: illegal M_arch type");
	  return -1;
	}
    }
}

int
M_call_type2 (M_Type type, M_Type ntype)
{
  /* Use layout database (if specified) to determine info -ITI(JCG) 2/99 */
  if (M_use_layout_database)
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
	case M_TYPE_BIT_SHORT:	/* ITI(JCG) 2/99 */
	case M_TYPE_SHORT:
	  M_short (ntype, type->unsign);
	  return M_TYPE_SHORT;
	case M_TYPE_INT:
	  M_int (ntype, type->unsign);
	  return (ntype->type);
	case M_TYPE_BIT_LONG:
	case M_TYPE_LONG:
	  M_long (ntype, type->unsign);
	  return M_TYPE_LONG;
	case M_TYPE_BIT_LLONG:
	case M_TYPE_LLONG:
	  M_llong (ntype, type->unsign);
	  return M_TYPE_LLONG;
	case M_TYPE_BLOCK:	/* the starting address of array is used */
	case M_TYPE_POINTER:
	  M_pointer (ntype);
	  return M_TYPE_POINTER;
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
	  return type->type;
	default:
	  return (-1);
	}
    }
  /* Otherwise use orig hard-coded method to determine info -ITI(JCG) 2/99 */
  else
    {
      switch (M_arch)
	{
	case M_IMPACT:
	  return M_impact_call_type2 (type, ntype);
	case M_TAHOE:
	  return M_tahoe_call_type2 (type, ntype);
	case M_SPARC:
	  return M_sparc_call_type2 (type, ntype);
	case M_HPPA:
	  return M_hppa_call_type2 (type, ntype);
	case M_X86:
	  return M_x86_call_type2 (type, ntype);
	case M_PLAYDOH:
	  return M_playdoh_call_type2 (type, ntype);
	case M_TI:
	  return M_ti_call_type2 (type, ntype);
	case M_SH:
	  return M_sh_call_type2 (type, ntype);
	case M_STARCORE:
	  return M_starcore_call_type2 (type, ntype);
	case M_ARM:
	  return M_arm_call_type2 (type, ntype);
	case M_WIMS:
	  return M_wims_call_type2 (type, ntype);
	default:
	  M_assert (0, "M_call_type2: illegal M_arch type");
	  return -1;
	}
    }
}

void
M_array_layout (M_Type type, int *offset)
{
  /* Use layout database (if specified) to determine info -ITI(JCG) 2/99 */
  if (M_use_layout_database)
    {
      *offset = 0;		/* assume first element is aligned */
    }
  /* Otherwise use orig hard-coded method to determine info -ITI(JCG) 2/99 */
  else
    {
      switch (M_arch)
	{
	case M_IMPACT:
	  M_impact_array_layout (type, offset);
	  break;
	case M_TAHOE:
	  M_tahoe_array_layout (type, offset);
	  break;
	case M_SPARC:
	  M_sparc_array_layout (type, offset);
	  break;
	case M_HPPA:
	  M_hppa_array_layout (type, offset);
	  break;
	case M_X86:
	  M_x86_array_layout (type, offset);
	  break;
	case M_PLAYDOH:
	  M_playdoh_array_layout (type, offset);
	  break;
	case M_TI:
	  M_ti_array_layout (type, offset);
	  break;
	case M_SH:
	  M_sh_array_layout (type, offset);
	  break;
	case M_STARCORE:
	  M_starcore_array_layout (type, offset);
	  break;
	case M_ARM:
	  M_arm_array_layout (type, offset);
	  break;
	case M_WIMS:
	  M_wims_array_layout (type, offset);
	default:
	  M_assert (0, "M_array_layout: illegal M_arch type");
	}
    }
}

int
M_array_align (M_Type type)
{
  /* Use layout database (if specified) to determine info -ITI(JCG) 2/99 */
  if (M_use_layout_database)
    {
      return type->align;
    }
  /* Otherwise use orig hard-coded method to determine info -ITI(JCG) 2/99 */
  else
    {
      switch (M_arch)
	{
	case M_IMPACT:
	  return M_impact_array_align (type);
	case M_TAHOE:
	  return M_tahoe_array_align (type);
	case M_SPARC:
	  return M_sparc_array_align (type);
	case M_HPPA:
	  return M_hppa_array_align (type);
	case M_X86:
	  return M_x86_array_align (type);
	case M_PLAYDOH:
	  return M_playdoh_array_align (type);
	case M_TI:
	  return M_ti_array_align (type);
	case M_SH:
	  return M_sh_array_align (type);
	case M_STARCORE:
	  return M_starcore_array_align (type);
	case M_ARM:
	  return M_arm_array_align (type);
	case M_WIMS:
	  return M_wims_array_align (type);
	default:
	  M_assert (0, "M_array_align: illegal M_arch type");
	  return -1;
	}
    }
}

int
M_array_size (M_Type type, int dim)
{
  /* Use layout database (if specified) to determine info -ITI(JCG) 2/99 */
  if (M_use_layout_database)
    {
      /* This seems to be a pretty safe way to calculate it -ITI(JCG) 2/99 */
      int mod, size, align;
      size = type->size;
      align = type->align;
      mod = size % align;
      if (mod != 0)
	size += (align - mod);
      return (size * dim);
    }
  /* Otherwise use orig hard-coded method to determine info -ITI(JCG) 2/99 */
  else
    {
      switch (M_arch)
	{
	case M_IMPACT:
	  return M_impact_array_size (type, dim);
	case M_TAHOE:
	  return M_tahoe_array_size (type, dim);
	case M_SPARC:
	  return M_sparc_array_size (type, dim);
	case M_HPPA:
	  return M_hppa_array_size (type, dim);
	case M_X86:
	  return M_x86_array_size (type, dim);
	case M_PLAYDOH:
	  return M_playdoh_array_size (type, dim);
	case M_TI:
	  return M_ti_array_size (type, dim);
	case M_SH:
	  return M_sh_array_size (type, dim);
	case M_STARCORE:
	  return M_starcore_array_size (type, dim);
	case M_ARM:
	  return M_arm_array_size (type, dim);
	case M_WIMS:
	  return M_wims_array_size (type, dim);
	default:
	  M_assert (0, "M_array_size: illegal M_arch type");
	  return -1;
	}
    }
}

/* Pass union & field names so can use layout database -ITI(JCG) 2/99 */
void
M_union_layout (int n, _M_Type * type, int *offset, int *bit_offset,
		char *union_name, char *field_name[])
{
  int i;
  /* Use layout database (if specified) to determine info -ITI(JCG) 2/99 */
  if (M_use_layout_database)
    {
      for (i = 0; i < n; i++)
	{
	  offset[i] = M_read_database_i (union_name, field_name[i], "offset");
	  bit_offset[i] = 0;	/* IMPACT convention for non-bit-fields */
	}
    }
  /* Otherwise use orig hard-coded method to determine info -ITI(JCG) 2/99 */
  else
    {
      switch (M_arch)
	{
	case M_IMPACT:
	  M_impact_union_layout (n, type, offset, bit_offset);
	  break;
	case M_TAHOE:
	  M_tahoe_union_layout (n, type, offset, bit_offset);
	  break;
	case M_SPARC:
	  M_sparc_union_layout (n, type, offset, bit_offset);
	  break;
	case M_HPPA:
	  M_hppa_union_layout (n, type, offset, bit_offset);
	  break;
	case M_X86:
	  M_x86_union_layout (n, type, offset, bit_offset);
	  break;
	case M_PLAYDOH:
	  M_playdoh_union_layout (n, type, offset, bit_offset);
	  break;
	case M_TI:
	  M_ti_union_layout (n, type, offset, bit_offset);
	  break;
	case M_SH:
	  M_sh_union_layout (n, type, offset, bit_offset);
	  break;
	case M_STARCORE:
	  M_starcore_union_layout (n, type, offset, bit_offset);
	  break;
	case M_ARM:
	  M_arm_union_layout (n, type, offset, bit_offset);
	  break;
	case M_WIMS:
	  M_wims_union_layout (n, type, offset, bit_offset);
	  break;
	default:
	  M_assert (0, "M_union_layout: illegal M_arch type");
	}
    }
}

/* Pass union name so can use layout database -ITI(JCG) 2/99 */
int
M_union_align (int n, _M_Type * type, char *union_name)
{
  /* Use layout database (if specified) to determine info -ITI(JCG) 2/99 */
  if (M_use_layout_database)
    {
      return (M_read_database_i ("_HT__user_types", union_name, "align"));
    }
  /* Otherwise use orig hard-coded method to determine info -ITI(JCG) 2/99 */
  else
    {
      switch (M_arch)
	{
	case M_IMPACT:
	  return M_impact_union_align (n, type);
	case M_TAHOE:
	  return M_tahoe_union_align (n, type);
	case M_SPARC:
	  return M_sparc_union_align (n, type);
	case M_HPPA:
	  return M_hppa_union_align (n, type);
	case M_X86:
	  return M_x86_union_align (n, type);
	case M_PLAYDOH:
	  return M_playdoh_union_align (n, type);
	case M_TI:
	  return M_ti_union_align (n, type);
	case M_SH:
	  return M_sh_union_align (n, type);
	case M_STARCORE:
	  return M_starcore_union_align (n, type);
	case M_ARM:
	  return M_arm_union_align (n, type);
	case M_WIMS:
	  return M_wims_union_align (n, type);
	default:
	  M_assert (0, "M_union_align: illegal M_arch type");
	  return -1;
	}
    }
}

/* Pass union name so can use layout database -ITI(JCG) 2/99 */
int
M_union_size (int n, _M_Type * type, char *union_name)
{
  /* Use layout database (if specified) to determine info -ITI(JCG) 2/99 */
  if (M_use_layout_database)
    {
      return (M_read_database_i ("_HT__user_types", union_name, "size"));
    }
  /* Otherwise use orig hard-coded method to determine info -ITI(JCG) 2/99 */
  else
    {
      switch (M_arch)
	{
	case M_IMPACT:
	  return M_impact_union_size (n, type);
	case M_TAHOE:
	  return M_tahoe_union_size (n, type);
	case M_SPARC:
	  return M_sparc_union_size (n, type);
	case M_HPPA:
	  return M_hppa_union_size (n, type);
	case M_X86:
	  return M_x86_union_size (n, type);
	case M_PLAYDOH:
	  return M_playdoh_union_size (n, type);
	case M_TI:
	  return M_ti_union_size (n, type);
	case M_SH:
	  return M_sh_union_size (n, type);
	case M_STARCORE:
	  return M_starcore_union_size (n, type);
	case M_ARM:
	  return M_arm_union_size (n, type);
	case M_WIMS:
	  return M_wims_union_size (n, type);
	default:
	  M_assert (0, "M_union_size: illegal M_arch type");
	  return -1;
	}
    }
}

/* Pass struct & field names so can use layout database -ITI(JCG) 2/99 */
void
M_struct_layout (int n, _M_Type * type, int *base, int *bit_offset,
		 char *struct_name, char *field_name[])
{
  /* Use layout database (if specified) to determine info -ITI(JCG) 2/99 */
  if (M_use_layout_database)
    {
      int i, offset;
      int size, mod_word, mod_type;
      int short_size, char_size, int_size, long_size;
#if 0
      /* Debug */
      fprintf (stderr, "\nLayout for struct %s:\n", struct_name);
#endif

      /* Get sizes of the types for bit-field calculation (if any) */
      char_size = M_read_database_i ("_HT__base_types", "char", "size");
      short_size = M_read_database_i ("_HT__base_types", "short", "size");
      int_size = M_read_database_i ("_HT__base_types", "int", "size");
      long_size = M_read_database_i ("_HT__base_types", "long", "size");

      offset = 0;		/* assume initially aligned */
      for (i = 0; i < n; i++)
	{
	  /* Handle bit-fields in a special way */
	  if ((type[i].type == M_TYPE_BIT_CHAR) ||
	      (type[i].type == M_TYPE_BIT_SHORT) ||
	      (type[i].type == M_TYPE_BIT_INT) ||
	      (type[i].type == M_TYPE_BIT_LONG))
	    {
	      /* Get bit-field size */
	      size = type[i].size;

	      /* Get host-compiler's offset for bit field */
	      offset = M_read_database_i (struct_name, field_name[i],
					  "offset");

	      /* Promote the type of bit field so that only one
	       * load/store is needed (i.e., if 5 bits but crosses
	       * char boundary, load/store it as a short)
	       */
	      if (type[i].type == M_TYPE_BIT_CHAR)
		{
		  mod_type = offset % char_size;
		  if ((mod_type + size) > char_size)
		    {
		      type[i].type = M_TYPE_BIT_SHORT;
		    }
		}
	      if (type[i].type == M_TYPE_BIT_SHORT)
		{
		  mod_type = offset % short_size;
		  if ((mod_type + size) > short_size)
		    {
		      type[i].type = M_TYPE_BIT_INT;
		    }
		}
	      if (type[i].type == M_TYPE_BIT_INT)
		{
		  mod_type = offset % int_size;
		  if ((mod_type + size) > int_size)
		    {
		      type[i].type = M_TYPE_BIT_LONG;
		    }
		}

	      /* IMPACT can't handle bit-fields that cross word boundaries */
	      mod_word = offset % long_size;
	      if ((mod_word + size) > long_size)
		{
		  I_punt ("\n"
			  "M_struct_layout: "
			  "IMPACT does not support bit fields \n"
			  "                 that cross word boundaries!"
			  "\n"
			  "     Database: %s\n"
			  "    Built for: %s\n"
			  "    Structure: \"%s\"\n"
			  "        Field: %s\n"
			  "         Size: %i\n"
			  "       Offset: %i\n",
			  M_layout_database_name,
			  M_layout_database_desc (),
			  struct_name, field_name[i], size, offset);
		}

	      /* IMPACT requires bit_offset to be from the rightmost
	         p             * bit of the surrounding longword.  Thus, we do
	         * this tricky calculation to give the wierd bit_offset.
	         *
	         *         bit field representation.
	         *         |<-------- M_TYPE_LONG --------->|
	         *         +--------------------------------+
	         *         |00000000111111000000000000000000| <--- bit_mask
	         *         +--------------------------------+
	         *  offset |<------>|    |<---------------->| bit_offset
	         *
	         *      To read a bit field (for a long bit field),
	         *      1. data = fetch long word
	         *      2. data = data & bit_mask
	         *      3. data >> bit_offset (logical shift)
	         *
	         * For char and short bit fields, HtoL adjusts the bit_offset
	         * appropriately for that size.  It even works. :) 
	       */
	      mod_word = offset % long_size;
	      bit_offset[i] = offset - mod_word +
		(long_size - mod_word - size);

	      /* Figure out base offset for bit-field based on type.
	       * Use offset, not the funky bit_offset calculated above
	       */
	      if (type[i].type == M_TYPE_BIT_CHAR)
		{
		  base[i] = offset & (~(char_size - 1));

		  /* Compensate for little endian ordering of
		   * bytes -ITI(JCG) 6/99
		   */
		  if (M_read_database_i ("_HT__info",
					 "little_endian", "value") == 1)
		    {
		      if ((offset & 31) >= 24)
			base[i] -= 24;
		      else if ((offset & 31) >= 16)
			base[i] -= 8;
		      else if ((offset & 31) >= 8)
			base[i] += 8;
		      else
			base[i] += 24;
		    }
		}
	      else if (type[i].type == M_TYPE_BIT_SHORT)
		{
		  base[i] = offset & (~(short_size - 1));

		  /* Compensate for little endian ordering of
		   * shorts -ITI(JCG) 6/99
		   */
		  if (M_read_database_i ("_HT__info",
					 "little_endian", "value") == 1)
		    {
		      if ((offset & 31) >= 16)
			base[i] -= 16;
		      else
			base[i] += 16;
		    }
		}
	      else if (type[i].type == M_TYPE_BIT_INT)
		{
		  base[i] = offset & (~(int_size - 1));
		}
	      else if (type[i].type == M_TYPE_BIT_LONG)
		{
		  base[i] = offset & (~(long_size - 1));
		}
	      else
		{
		  I_punt ("M_struct_layout: Unexpected field type %i",
			  type[i].type);
		}
	    }

	  /* Otherwise, place how compiler indicates */
	  else
	    {
	      base[i] = M_read_database_i (struct_name, field_name[i],
					   "offset");
	      bit_offset[i] = 0;	/* IMPACT convention for 
					   non-bit-fields */
	    }

#if 0
	  /* Debug */
	  fprintf (stderr,
		   "  %8s  base: %5i   bit_offset: %5i  size: %i  type:%i\n",
		   field_name[i], base[i], bit_offset[i], type[i].size,
		   type[i].type);
#endif
	}
    }
  /* Otherwise use orig hard-coded method to determine info -ITI(JCG) 2/99 */
  else
    {
      switch (M_arch)
	{
	case M_IMPACT:
	  M_impact_struct_layout (n, type, base, bit_offset);
	  break;
	case M_TAHOE:
	  M_tahoe_struct_layout (n, type, base, bit_offset);
	  break;
	case M_SPARC:
	  M_sparc_struct_layout (n, type, base, bit_offset);
	  break;
	case M_HPPA:
	  M_hppa_struct_layout (n, type, base, bit_offset);
	  break;
	case M_X86:
	  M_x86_struct_layout (n, type, base, bit_offset);
	  break;
	case M_PLAYDOH:
	  M_playdoh_struct_layout (n, type, base, bit_offset);
	  break;
	case M_TI:
	  M_ti_struct_layout (n, type, base, bit_offset);
	  break;
	case M_SH:
	  M_sh_struct_layout (n, type, base, bit_offset);
	  break;
	case M_STARCORE:
	  M_starcore_struct_layout (n, type, base, bit_offset);
	  break;
	case M_ARM:
	  M_arm_struct_layout (n, type, base, bit_offset);
	  break;
	case M_WIMS:
	  M_wims_struct_layout (n, type, base, bit_offset);
	  break;
	default:
	  M_assert (0, "M_struct_layout: illegal M_arch type");
	}
    }
}

/* Pass struct name so can use layout database -ITI(JCG) 2/99 */
int
M_struct_align (int n, _M_Type * type, char *struct_name)
{
  /* Use layout database (if specified) to determine info -ITI(JCG) 2/99 */
  if (M_use_layout_database)
    {
      return (M_read_database_i ("_HT__user_types", struct_name, "align"));
    }
  /* Otherwise use orig hard-coded method to determine info -ITI(JCG) 2/99 */
  else
    {
      switch (M_arch)
	{
	case M_IMPACT:
	  return M_impact_struct_align (n, type);
	case M_TAHOE:
	  return M_tahoe_struct_align (n, type);
	case M_SPARC:
	  return M_sparc_struct_align (n, type);
	case M_HPPA:
	  return M_hppa_struct_align (n, type);
	case M_X86:
	  return M_x86_struct_align (n, type);
	case M_PLAYDOH:
	  return M_playdoh_struct_align (n, type);
	case M_TI:
	  return M_ti_struct_align (n, type);
	case M_SH:
	  return M_sh_struct_align (n, type);
	case M_STARCORE:
	  return M_starcore_struct_align (n, type);
	case M_ARM:
	  return M_arm_struct_align (n, type);
	case M_WIMS:
	  return M_wims_struct_align (n, type);
	default:
	  M_assert (0, "M_struct_align: illegal M_arch type");
	  return -1;
	}
    }
}

/* Pass struct name so can use layout database -ITI(JCG) 2/99 */
int
M_struct_size (int n, _M_Type * type, int struct_align, char *struct_name)
{
  /* Use layout database (if specified) to determine info -ITI(JCG) 2/99 */
  if (M_use_layout_database)
    {
      return (M_read_database_i ("_HT__user_types", struct_name, "size"));
    }
  /* Otherwise use orig hard-coded method to determine info -ITI(JCG) 2/99 */
  else
    {
      switch (M_arch)
	{
	case M_IMPACT:
	  return M_impact_struct_size (n, type, struct_align);
	case M_TAHOE:
	  return M_tahoe_struct_size (n, type, struct_align);
	case M_SPARC:
	  return M_sparc_struct_size (n, type, struct_align);
	case M_HPPA:
	  return M_hppa_struct_size (n, type, struct_align);
	case M_X86:
	  return M_x86_struct_size (n, type, struct_align);
	case M_PLAYDOH:
	  return M_playdoh_struct_size (n, type, struct_align);
	case M_TI:
	  return M_ti_struct_size (n, type, struct_align);
	case M_SH:
	  return M_sh_struct_size (n, type, struct_align);
	case M_STARCORE:
	  return M_starcore_struct_size (n, type, struct_align);
	case M_ARM:
	  return M_arm_struct_size (n, type, struct_align);
	case M_WIMS:
	  return M_wims_struct_size (n, type, struct_align);
	default:
	  M_assert (0, "M_struct_size: illegal M_arch type");
	  return -1;
	}
    }
}

int
M_layout_fnvar (List param_list, char **base_macro, int *pcount, int purpose,
		int needs_ST)
{
  switch (M_arch)
    {
    case M_IMPACT:
      return M_impact_layout_fnvar (param_list, base_macro, pcount, purpose);
    case M_TAHOE:
      return M_tahoe_layout_fnvar (param_list, base_macro, pcount, purpose);
    case M_SPARC:
      return M_sparc_layout_fnvar (param_list, base_macro, pcount, purpose);
    case M_HPPA:
      return M_hppa_layout_fnvar (param_list, base_macro, pcount, purpose);
    case M_X86:
      return M_x86_layout_fnvar (param_list, base_macro, pcount, purpose);
    case M_PLAYDOH:
      return M_playdoh_layout_fnvar (param_list, base_macro, pcount, purpose);
    case M_TI:
      return M_ti_layout_fnvar (param_list, base_macro, pcount, purpose);
    case M_SH:
      return M_sh_layout_fnvar (param_list, base_macro, pcount, purpose);
    case M_STARCORE:
      return M_starcore_layout_fnvar (param_list, base_macro, pcount, purpose);
    case M_ARM:
      return M_arm_layout_fnvar (param_list, base_macro, pcount, purpose,
				 needs_ST);
    case M_WIMS:
      return M_wims_layout_fnvar (param_list, base_macro, pcount, purpose);
    default:
      M_assert (0, "M_layout_fnvar: illegal M_arch type");
      return -1;
    }
}


int
M_default_layout_retvar (M_Param param, int purpose)
{
  if (param->mtype.type & (M_TYPE_STRUCT | M_TYPE_UNION))
    param->mode = M_INDIRECT_THRU_REGISTER;
  else
    param->mode = M_THRU_REGISTER;
  return (param->mode == M_INDIRECT_THRU_REGISTER);
}


int
M_layout_retvar (M_Param param, int purpose)
{
  switch (M_arch)
    {
    case M_IMPACT:
      return M_default_layout_retvar (param, purpose);
    case M_TAHOE:
      return M_tahoe_layout_retvar (param, purpose);
    case M_SPARC:
      return M_default_layout_retvar (param, purpose);
    case M_HPPA:
      return M_default_layout_retvar (param, purpose);
    case M_X86:
      return M_default_layout_retvar (param, purpose);
    case M_PLAYDOH:
      return M_default_layout_retvar (param, purpose);
    case M_TI:
      return M_default_layout_retvar (param, purpose);
    case M_SH:
      return M_default_layout_retvar (param, purpose);
    case M_STARCORE:
      return M_default_layout_retvar (param, purpose);
    case M_WIMS:
      return M_wims_layout_retvar (param, purpose);
    case M_ARM:
      return M_arm_layout_retvar (param, purpose);
    default:
      M_assert (0, "Layout_retvar: illegal M_arch type");
      return -1;
    }
}


int
M_fnvar_layout (int n, _M_Type * type, long int *offset, int *mode, int *reg,
		int *paddr, char **base_macro, int *su_sreg, int *su_ereg,
		int *pcount, int need_ST, int purpose)
/*
 *	n = number of arguments to be passed for this jsr.
 *	type[] = data types of arguments.
 *	offset[] = offsets from $SP or $FP, where the arguments are placed.
 *	mode[] = 1) M_THRU_REGISTER: pass thru register
 *		 2) M_THRU_MEMORY: pass thru memory
 *		 3) M_INDIRECT_THRU_REGISTER: pass thru memory, allocate space
 *			elsewhere, but pass a pointer thru register
 *		 4) M_INDIRECT_THRU_MEMORY: pass thru memory, allocate
 *			space elsewhere, and pass a pointer thru memory
 *	reg[] = the macro register number when pass-thru register.
 *	paddr[] = the starting address of INDIRECT mode.
 *	base_macro = $SP or $FP
 *      su_sreg = first reg used for multi-reg param
 *      su_ereg = last reg used for multi-reg param
 *      pcount = number of parameter regs created
 *	need_ST = does the function return a structure?
 *	purpose = for calling or for receiving arguments?
 */
			/* need to return structure */
{
  switch (M_arch)
    {
    case M_IMPACT:
      return M_impact_fnvar_layout (n, type, offset, mode, reg, paddr,
				    base_macro, su_sreg, su_ereg,
				    pcount, need_ST, purpose);
    case M_TAHOE:
      return M_tahoe_fnvar_layout (n, type, offset, mode, reg, paddr,
				   base_macro, su_sreg, su_ereg,
				   pcount, need_ST, purpose);
    case M_SPARC:
      return M_sparc_fnvar_layout (n, type, offset, mode, reg, paddr,
				   base_macro, su_sreg, su_ereg,
				   pcount, need_ST, purpose);
    case M_HPPA:
      return M_hppa_fnvar_layout (n, type, offset, mode, reg, paddr,
				  base_macro, su_sreg, su_ereg,
				  pcount, need_ST, purpose);
    case M_X86:
      return M_x86_fnvar_layout (n, type, offset, mode, reg, paddr,
				 base_macro, su_sreg, su_ereg,
				 pcount, need_ST, purpose);
    case M_PLAYDOH:
      return M_playdoh_fnvar_layout (n, type, offset, mode, reg, paddr,
				     base_macro, su_sreg, su_ereg,
				     pcount, need_ST, purpose);
    case M_TI:
      return M_ti_fnvar_layout (n, type, offset, mode, reg, paddr,
				base_macro, su_sreg, su_ereg,
				pcount, need_ST, purpose);
    case M_SH:
      return M_sh_fnvar_layout (n, type, offset, mode, reg, paddr,
				base_macro, su_sreg, su_ereg,
				pcount, need_ST, purpose);
    case M_STARCORE:
      return M_starcore_fnvar_layout (n, type, offset, mode, reg, paddr,
				      base_macro, su_sreg, su_ereg,
				      pcount, need_ST, purpose);
    case M_ARM:
      return M_arm_fnvar_layout (n, type, offset, mode, reg, paddr,
				 base_macro, su_sreg, su_ereg,
				 pcount, need_ST, purpose);
    case M_WIMS:
      return M_wims_fnvar_layout (n, type, offset, mode, reg, paddr,
				  base_macro, su_sreg, su_ereg,
				  pcount, need_ST, purpose);
    default:
      M_assert (0, "M_fnvar_layout: illegal M_arch type");
      return -1;
    }
}

int
M_lvar_layout (int n, _M_Type * type, long int *offset, char **base_macro)
{
  switch (M_arch)
    {
    case M_IMPACT:
      return M_impact_lvar_layout (n, type, offset, base_macro);
    case M_TAHOE:
      return M_tahoe_lvar_layout (n, type, offset, base_macro);
    case M_SPARC:
      return M_sparc_lvar_layout (n, type, offset, base_macro);
    case M_HPPA:
      return M_hppa_lvar_layout (n, type, offset, base_macro);
    case M_X86:
      return M_x86_lvar_layout (n, type, offset, base_macro);
    case M_PLAYDOH:
      return M_playdoh_lvar_layout (n, type, offset, base_macro);
    case M_TI:
      return M_ti_lvar_layout (n, type, offset, base_macro);
    case M_SH:
      return M_sh_lvar_layout (n, type, offset, base_macro);
    case M_STARCORE:
      return M_starcore_lvar_layout (n, type, offset, base_macro);
    case M_ARM:
      return M_arm_lvar_layout (n, type, offset, base_macro);
    case M_WIMS:
      return M_wims_lvar_layout (n, type, offset, base_macro);
    default:
      M_assert (0, "M_lvar_layout: illegal M_arch type");
      return -1;
    }
}

int
M_native_int_register_ctype (void)
{
  switch (M_arch)
    {
    case M_TAHOE:
      return L_CTYPE_LLONG;
    case M_IMPACT:
    case M_SPARC:
    case M_HPPA:
    case M_X86:
    case M_PLAYDOH:
    case M_TI:
    case M_SH:
    case M_STARCORE:
    case M_ARM:
    case M_WIMS:
      return L_CTYPE_INT;
    default:
      M_assert (0, "M_native_int_register_ctype: illegal M_arch type");
      return -1;
    }

}

int
M_native_int_register_mtype (void)
{
  switch (M_arch)
    {
    case M_TAHOE:
      return M_TYPE_LLONG;
    case M_IMPACT:
    case M_SPARC:
    case M_HPPA:
    case M_X86:
    case M_PLAYDOH:
    case M_TI:
    case M_SH:
    case M_STARCORE:
    case M_ARM:
    case M_WIMS:
      return M_TYPE_INT;
    default:
      M_assert (0, "M_native_int_register_mtype: illegal M_arch type");
      return -1;
    }

}

int
M_no_short_int (void)
{
  /* Use layout database (if specified) to determine info -ITI(JCG) 2/99 */
  if (M_use_layout_database)
    {
      return (M_read_database_i ("_HT__base_types", "short", "size") ==
	      M_read_database_i ("_HT__base_types", "int", "size"));
    }
  /* Otherwise use orig hard-coded method to determine info -ITI(JCG) 2/99 */
  else
    {
      switch (M_arch)
	{
	case M_IMPACT:
	  return M_impact_no_short_int ();
	case M_TAHOE:
	  return M_tahoe_no_short_int ();
	case M_SPARC:
	  return M_sparc_no_short_int ();
	case M_HPPA:
	  return M_hppa_no_short_int ();
	case M_X86:
	  return M_x86_no_short_int ();
	case M_PLAYDOH:
	  return M_playdoh_no_short_int ();
	case M_TI:
	  return M_ti_no_short_int ();
	case M_SH:
	  return M_sh_no_short_int ();
	case M_STARCORE:
	  return M_starcore_no_short_int ();
	case M_ARM:
	  return M_arm_no_short_int ();
	case M_WIMS:
	  return M_wims_no_short_int ();
	default:
	  M_assert (0, "M_no_short_int: illegal M_arch type");
	  return -1;
	}
    }
}

int
M_compatible_type (M_Type type1, M_Type type2)
{
  _M_Type t1, t2;
  M_eval_type (type1, &t1);
  M_eval_type (type2, &t2);
  /*
   *  sign does not matter.
   */
  return (t1.type == t2.type);
}

void
M_cb_label_name (char *fn, int cb, char *line, int len)
{
  switch (M_arch)
    {
    case M_IMPACT:
      M_impact_cb_label_name (fn, cb, line, len);
      break;
    case M_TAHOE:
      M_tahoe_cb_label_name (fn, cb, line, len);
      break;
    case M_SPARC:
      M_sparc_cb_label_name (fn, cb, line, len);
      break;
    case M_HPPA:
      M_hppa_cb_label_name (fn, cb, line, len);
      break;
    case M_X86:
      M_x86_cb_label_name (fn, cb, line, len);
      break;
    case M_PLAYDOH:
      M_playdoh_cb_label_name (fn, cb, line, len);
      break;
    case M_TI:
      M_ti_cb_label_name (fn, cb, line, len);
      break;
    case M_SH:
      M_sh_cb_label_name (fn, cb, line, len);
      break;
    case M_BX86:
      M_bx86_cb_label_name (fn, cb, line, len);
      break;
    case M_STARCORE:
      M_starcore_cb_label_name (fn, cb, line, len);
      break;
    case M_ARM:
      M_arm_cb_label_name (fn, cb, line, len);
      break;
    case M_WIMS:
      M_wims_cb_label_name (fn, cb, line, len);
      break;
    default:
      M_assert (0, "M_cb_label_name: illegal M_arch type");
    }
}

int
M_is_cb_label (char *label, char *fn, int *cb)
{
  switch (M_arch)
    {
    case M_IMPACT:
      return M_impact_is_cb_label (label, fn, cb);
    case M_TAHOE:
      return M_tahoe_is_cb_label (label, fn, cb);
    case M_SPARC:
      return M_sparc_is_cb_label (label, fn, cb);
    case M_HPPA:
      return M_hppa_is_cb_label (label, fn, cb);
    case M_X86:
      return M_x86_is_cb_label (label, fn, cb);
    case M_PLAYDOH:
      return M_playdoh_is_cb_label (label, fn, cb);
    case M_TI:
      return M_ti_is_cb_label (label, fn, cb);
    case M_SH:
      return M_sh_is_cb_label (label, fn, cb);
    case M_BX86:
      return M_bx86_is_cb_label (label, fn, cb);
    case M_STARCORE:
      return M_starcore_is_cb_label (label, fn, cb);
    case M_ARM:
      return M_arm_is_cb_label (label, fn, cb);
    case M_WIMS:
      return M_wims_is_cb_label (label, fn, cb);
    default:
      M_assert (0, "M_is_cb_label: illegal M_arch type");
      return 0;
    }
}

void
M_jumptbl_label_name (char *fn, int tbl_id, char *line, int len)
{
  switch (M_arch)
    {
    case M_IMPACT:
      M_impact_jumptbl_label_name (fn, tbl_id, line, len);
      break;
    case M_TAHOE:
      M_tahoe_jumptbl_label_name (fn, tbl_id, line, len);
      break;
    case M_SPARC:
      M_sparc_jumptbl_label_name (fn, tbl_id, line, len);
      break;
    case M_HPPA:
      M_hppa_jumptbl_label_name (fn, tbl_id, line, len);
      break;
    case M_X86:
      M_x86_jumptbl_label_name (fn, tbl_id, line, len);
      break;
    case M_PLAYDOH:
      M_playdoh_jumptbl_label_name (fn, tbl_id, line, len);
      break;
    case M_TI:
      M_ti_jumptbl_label_name (fn, tbl_id, line, len);
      break;
    case M_SH:
      M_sh_jumptbl_label_name (fn, tbl_id, line, len);
      break;
    case M_BX86:
      M_bx86_jumptbl_label_name (fn, tbl_id, line, len);
      break;
    case M_STARCORE:
      M_starcore_jumptbl_label_name (fn, tbl_id, line, len);
      break;
    case M_ARM:
      M_arm_jumptbl_label_name (fn, tbl_id, line, len);
      break;
    case M_WIMS:
      M_wims_jumptbl_label_name (fn, tbl_id, line, len);
      break;  
    default:
      M_assert (0, "M_jumptbl_label_name: illegal M_arch type");
    }
}

int
M_is_jumptbl_label (char *label, char *fn, int *tbl_id)
{
  switch (M_arch)
    {
    case M_IMPACT:
      return M_impact_is_jumptbl_label (label, fn, tbl_id);
    case M_TAHOE:
      return M_tahoe_is_jumptbl_label (label, fn, tbl_id);
    case M_SPARC:
      return M_sparc_is_jumptbl_label (label, fn, tbl_id);
    case M_HPPA:
      return M_hppa_is_jumptbl_label (label, fn, tbl_id);
    case M_X86:
      return M_x86_is_jumptbl_label (label, fn, tbl_id);
    case M_PLAYDOH:
      return M_playdoh_is_jumptbl_label (label, fn, tbl_id);
    case M_TI:
      return M_ti_is_jumptbl_label (label, fn, tbl_id);
    case M_SH:
      return M_sh_is_jumptbl_label (label, fn, tbl_id);
    case M_BX86:
      return M_bx86_is_jumptbl_label (label, fn, tbl_id);
    case M_STARCORE:
      return M_starcore_is_jumptbl_label (label, fn, tbl_id);
    case M_ARM:
      return M_arm_is_jumptbl_label (label, fn, tbl_id);
    case M_WIMS:
      return M_wims_is_jumptbl_label (label, fn, tbl_id);
    default:
      M_assert (0, "M_is_jumptbl_label: illegal M_arch type");
      return 0;
    }
}

char *
M_fn_label_name (char *label, int (*is_func) (char *is_func_label))
{
  switch (M_arch)
    {
    case M_IMPACT:
      return (char *) M_impact_fn_label_name (label, is_func);
    case M_TAHOE:
      return (char *) M_tahoe_fn_label_name (label, is_func);
    case M_SPARC:
      return (char *) M_sparc_fn_label_name (label, is_func);
    case M_HPPA:
      return (char *) M_hppa_fn_label_name (label, is_func);
    case M_X86:
      return (char *) M_x86_fn_label_name (label, is_func);
    case M_PLAYDOH:
      return (char *) M_playdoh_fn_label_name (label, is_func);
    case M_TI:
      return (label);
    case M_SH:
      return (label);
    case M_BX86:
      return (char *) M_bx86_fn_label_name (label, is_func);
    case M_STARCORE:
      return (char *) M_starcore_fn_label_name (label, is_func);
    case M_ARM:
      return (char *) M_arm_fn_label_name (label, is_func);
    case M_WIMS:
      return (char *) M_wims_fn_label_name (label, is_func);
    default:
      M_assert (0, "M_fn_label_name: illegal M_arch type");
      return 0;
    }
}

char *
M_fn_name_from_label (char *label)
{
  switch (M_arch)
    {
    case M_IMPACT:
      return (char *) M_impact_fn_name_from_label (label);
    case M_TAHOE:
      return (char *) M_tahoe_fn_name_from_label (label);
    case M_SPARC:
      return (char *) M_sparc_fn_name_from_label (label);
    case M_HPPA:
      return (char *) M_hppa_fn_name_from_label (label);
    case M_X86:
      return (char *) M_x86_fn_name_from_label (label);
    case M_PLAYDOH:
      return (char *) M_playdoh_fn_name_from_label (label);
    case M_TI:
      return (label);
    case M_SH:
      return (label);
    case M_BX86:
      return (char *) M_bx86_fn_name_from_label (label);
    case M_STARCORE:
      return (char *) M_starcore_fn_name_from_label (label);
    case M_ARM:
      return (char *) M_arm_fn_name_from_label (label);
    case M_WIMS:
      return (label);
    default:
      M_assert (0, "M_fn_name_from_label: illegal M_arch type");
      return 0;
    }
}

int
M_structure_pointer (int purpose)
{
  switch (M_arch)
    {
    case M_IMPACT:
      return M_impact_structure_pointer (purpose);
    case M_TAHOE:
      return M_tahoe_structure_pointer (purpose);
    case M_SPARC:
      return M_sparc_structure_pointer (purpose);
    case M_HPPA:
      return M_hppa_structure_pointer (purpose);
    case M_X86:
      return M_x86_structure_pointer (purpose);
    case M_PLAYDOH:
      return M_playdoh_structure_pointer (purpose);
    case M_TI:
      return M_ti_structure_pointer (purpose);
    case M_SH:
      return M_sh_structure_pointer (purpose);
    case M_STARCORE:
      return M_starcore_structure_pointer (purpose);
    case M_ARM:
      return M_arm_structure_pointer (purpose);
    case M_WIMS:
      return M_wims_structure_pointer (purpose);
    default:
      M_assert (0, "M_structure_pointer: illegal M_arch type");
      return 0;
    }
}

int 
M_return_value_thru_stack()
{
    switch (M_arch) {
      case M_IMPACT:
        return 0;
      case M_TAHOE:
        return 0;
      case M_SPARC:
        return 0;
      case M_HPPA:
        return 0;
      case M_X86:
        return 0;
      case M_PLAYDOH:
        return 0;
      case M_TI:
        return 0;
      case M_SH:
        return 0;
      case M_STARCORE:
        return 0;
      case M_ARM:
        return 0;
      case M_WIMS:
        return 1;
      default:
        M_assert(0, "illegal M_arch type");
        return 0;
    }
    return 0;
}

int
M_return_value_offset()
{
    int off;
    switch (M_arch) {
      case M_IMPACT:
        off = 0;
        break;
      case M_TAHOE:
        off = 0;
        break;
      case M_SPARC:
        off = 0;
        break;
      case M_HPPA:
        off = 0;
        break;
      case M_X86:
        off = 0;
        break;
      case M_PLAYDOH:
        off = 0;
        break;
      case M_TI:
        off = 0;
        break;
      case M_SH:
        off = 0;
        break;
      case M_STARCORE:
        off = 0;
        break;
      case M_ARM:
        off = 0;
        break;
      case M_WIMS:
        off = 0;
        break;
      default:
        M_assert(0, "illegal M_arch type");
        return 0;
    }
    // if stack grows from low to high
    // set off to size of the return value (M_'arch'_SIZE_MAX) above
    // and do off = -off before returning
    return off;
}

int
M_return_register (int type, int purpose)
{
  switch (M_arch)
    {
    case M_IMPACT:
      return M_impact_return_register (type, purpose);
    case M_TAHOE:
      return M_tahoe_return_register (type, purpose);
    case M_SPARC:
      return M_sparc_return_register (type, purpose);
    case M_HPPA:
      return M_hppa_return_register (type, purpose);
    case M_X86:
      return M_x86_return_register (type, purpose);
    case M_PLAYDOH:
      return M_playdoh_return_register (type, purpose);
    case M_TI:
      return M_ti_return_register (type, purpose);
    case M_SH:
      return M_sh_return_register (type, purpose);
    case M_STARCORE:
      return M_starcore_return_register (type, purpose);
    case M_ARM:
      return M_arm_return_register (type, purpose);
    case M_WIMS:
      return M_wims_return_register (type, purpose);
    default:
      M_assert (0, "M_return_register: illegal M_arch type");
      return 0;
    }
}

L_Operand *
M_return_epilogue_cntr_register ()
{
  switch (M_arch)
    {
    case M_IMPACT:
      return NULL;
    case M_TAHOE:
      return M_tahoe_epilogue_cntr_register ();
    case M_STARCORE:
      return M_starcore_epilogue_cntr_register ();
    case M_SPARC:
    case M_HPPA:
    case M_X86:
    case M_PLAYDOH:
    case M_TI:
    case M_SH:
    case M_ARM:
    case M_WIMS:
      M_assert (0,
		"M_return_epilogue_cntr_register: unsupported M_arch type");
      return NULL;
    default:
      M_assert (0, "M_return_epilogue_cntr_register: illegal M_arch type");
      return NULL;
    }
}

L_Operand *
M_return_loop_cntr_register ()
{
  switch (M_arch)
    {
    case M_IMPACT:
      return NULL;
    case M_TAHOE:
      return M_tahoe_loop_cntr_register ();
    case M_STARCORE:
      return M_starcore_loop_cntr_register ();
    case M_SPARC:
    case M_HPPA:
    case M_X86:
    case M_PLAYDOH:
    case M_TI:
    case M_SH:
    case M_ARM:
    case M_WIMS:
      M_assert (0, "M_return_loop_cntr_register: unsupported M_arch type");
      return NULL;
    default:
      M_assert (0, "M_return_loop_cntr_register: illegal M_arch type");
      return NULL;
    }
}

int
M_fragile_macro (int macro_value)
{
  switch (M_arch)
    {
    case M_IMPACT:
      return M_impact_fragile_macro (macro_value);
    case M_TAHOE:
      return M_tahoe_fragile_macro (macro_value);
    case M_SPARC:
      return M_sparc_fragile_macro (macro_value);
    case M_HPPA:
      return M_hppa_fragile_macro (macro_value);
    case M_X86:
      return M_x86_fragile_macro (macro_value);
    case M_PLAYDOH:
      return M_playdoh_fragile_macro (macro_value);
    case M_TI:
      return M_ti_fragile_macro (macro_value);
    case M_SH:
      return M_sh_fragile_macro (macro_value);
    case M_BX86:
      return M_bx86_fragile_macro (macro_value);
    case M_STARCORE:
      return M_starcore_fragile_macro (macro_value);
    case M_ARM:
      return M_arm_fragile_macro (macro_value);
    case M_WIMS:
      return M_wims_fragile_macro (macro_value);
    default:
      M_assert (0, "M_fragile_macro: illegal M_arch type");
      return 0;
    }
}


/****************************************************************************
 *
 * routine: M_extra_pred_define_opcode()
 * purpose: Determine if the proc_opc opcode generates a predicate.
 *          Only machine specific predicate generating opcodes above and
 *          beyond the normal pred defines, clears, and stores need to be
 *          listed.
 * input: 
 * output:
 * returns:
 * modified:
 * note:
 *-------------------------------------------------------------------------*/

int
M_extra_pred_define_opcode (int proc_opc)
{
  switch (M_arch)
    {
    case M_TAHOE:
      return (M_tahoe_extra_pred_define_opcode (proc_opc));
    case M_STARCORE:
      return (M_starcore_extra_pred_define_opcode (proc_opc));
    case M_IMPACT:
      return (0);
    case M_HPPA:
      return (0);
    case M_PLAYDOH:
      return (0);
    default:
      M_assert (0, "illegal M_arch type");
      return (0);
    }
}


int
M_extra_pred_define_type1 (L_Oper * oper)
{
  switch (M_arch)
    {
    case M_TAHOE:
      return M_tahoe_extra_pred_define_type1 (oper);
    case M_IMPACT:
      return (0);
    case M_HPPA:
      return (0);
    case M_PLAYDOH:
      return (0);
    default:
      M_assert (0, "M_extra_pred_define_type1: illegal M_arch type");
      return 0;
    }
}

int
M_extra_pred_define_type2 (L_Oper * oper)
{
  switch (M_arch)
    {
    case M_TAHOE:
      return M_tahoe_extra_pred_define_type2 (oper);
    case M_IMPACT:
      return (0);
    case M_HPPA:
      return (0);
    case M_PLAYDOH:
      return (0);
    default:
      M_assert (0, "M_extra_pred_define_type2: illegal M_arch type");
      return 0;
    }
}

Set
M_fragile_macro_set ()
{
  switch (M_arch)
    {
    case M_IMPACT:
      return M_impact_fragile_macro_set ();
    case M_TAHOE:
      return M_tahoe_fragile_macro_set ();
    case M_HPPA:
      return M_hppa_fragile_macro_set ();
    case M_SPARC:
      return M_sparc_fragile_macro_set ();
    case M_PLAYDOH:
      return M_playdoh_fragile_macro_set ();
    case M_X86:
    case M_TI:
    case M_BX86:
    case M_ARM:
    case M_WIMS:
      return NULL;
    case M_SH:
      return M_sh_fragile_macro_set ();
    case M_STARCORE:
      return M_starcore_fragile_macro_set ();
    default:
      M_assert (0, "M_dataflow_macro: illegal M_arch type");
    }
  return NULL;
}

int
M_dataflow_macro (int id)
{
  switch (M_arch)
    {
    case M_IMPACT:
      return M_impact_dataflow_macro (id);
    case M_TAHOE:
      return M_tahoe_dataflow_macro (id);
    case M_HPPA:
      return M_hppa_dataflow_macro (id);
    case M_SPARC:
      return M_sparc_dataflow_macro (id);
    case M_X86:
    case M_PLAYDOH:
    case M_TI:
    case M_ARM:
    case M_WIMS:
      return ((id >= L_MAC_P0 && id <= L_MAC_P64) || (id >= L_MAC_LAST));
    case M_SH:
      return M_sh_dataflow_macro (id);
    case M_STARCORE:
      return M_starcore_dataflow_macro (id);
    default:
      M_assert (0, "M_dataflow_macro: illegal M_arch type");
    }
  return 1;
}

int
M_zero_macro (L_Operand *opd)
{
  int mac;
  if (!opd ||
      !L_is_macro(opd))
    return 0;

  mac = opd->value.mac;

  switch (M_arch)
    {
    case M_TAHOE:
      if (mac == TAHOE_MAC_ZERO)
	return 1;
      break;

    default:
      break;
    }
  return 0;
}

int
M_subroutine_call (int opc)
{
  switch (M_arch)
    {
    case M_IMPACT:
      return M_impact_subroutine_call (opc);
    case M_TAHOE:
      return M_tahoe_subroutine_call (opc);
    case M_SPARC:
      return M_sparc_subroutine_call (opc);
    case M_HPPA:
      return M_hppa_subroutine_call (opc);
    case M_X86:
      return M_x86_subroutine_call (opc);
    case M_PLAYDOH:
      return M_playdoh_subroutine_call (opc);
    case M_TI:
      return M_ti_subroutine_call (opc);
    case M_SH:
      return M_sh_subroutine_call (opc);
    case M_STARCORE:
      return M_starcore_subroutine_call (opc);
    case M_ARM:
      return M_arm_subroutine_call (opc);
    case M_WIMS:
      return M_wims_subroutine_call (opc);
    default:
      M_assert (0, "M_subroutine_call: illegal M_arch type");
      return 0;
    }
}

void
M_define_macros (STRING_Symbol_Table * sym_tbl)
{
  switch (M_arch)
    {
    case M_IMPACT:
      M_define_macros_impact (sym_tbl);
      break;
    case M_TAHOE:
      M_define_macros_tahoe (sym_tbl);
      break;
    case M_SPARC:
      M_define_macros_sparc (sym_tbl);
      break;
    case M_HPPA:
      M_define_macros_hppa (sym_tbl);
      break;
    case M_X86:
      M_define_macros_x86 (sym_tbl);
      break;
    case M_PLAYDOH:
      M_define_macros_playdoh (sym_tbl);
      break;
    case M_TI:
      M_define_macros_ti (sym_tbl);
      break;
    case M_SH:
      M_define_macros_sh (sym_tbl);
      break;
    case M_BX86:
      M_define_macros_bx86 (sym_tbl);
      break;
    case M_STARCORE:
      M_define_macros_starcore (sym_tbl);
      break;
    case M_ARM:
      M_define_macros_arm (sym_tbl);
      break;
    case M_WIMS:
      M_define_macros_wims (sym_tbl);
      break;
    default:
      M_assert (0, "M_define_macros: illegal M_arch type");
    }
}

/* Return strings for architecture/model specific macros */
char *
M_get_macro_name (int id)
{
  switch (M_arch)
    {
    case M_IMPACT:
      return M_get_macro_name_impact (id);
    case M_TAHOE:
      return M_get_macro_name_tahoe (id);
    case M_SPARC:
      return M_get_macro_name_sparc (id);
    case M_HPPA:
      return M_get_macro_name_hppa (id);
    case M_X86:
      return M_get_macro_name_x86 (id);
    case M_PLAYDOH:
      return M_get_macro_name_playdoh (id);
    case M_TI:
      return M_get_macro_name_ti (id);
    case M_SH:
      return M_get_macro_name_sh (id);
    case M_BX86:
      return M_get_macro_name_bx86 (id);
    case M_STARCORE:
      return M_get_macro_name_starcore (id);
    case M_ARM:
      return M_get_macro_name_arm (id);
    case M_WIMS:
      return M_get_macro_name_wims (id);
    default:
      M_assert (0, "M_get_macro_name: illegal M_arch type");
      return (char *) 0;
    }
}

void
M_define_opcode_name (STRING_Symbol_Table * sym_tbl)
{
  switch (M_arch)
    {
    case M_IMPACT:
#if 0
      M_define_opcode_name_impact (sym_tbl);
#endif
      break;
    case M_TAHOE:

      break;
    case M_SPARC:
#if 0
      M_define_opcode_name_sparc (sym_tbl);
#endif
      break;
    case M_HPPA:
      M_define_opcode_name_hppa (sym_tbl);
      break;
    case M_X86:
      M_define_opcode_name_x86 (sym_tbl);
      break;
    case M_PLAYDOH:
      M_define_opcode_name_playdoh (sym_tbl);
      break;
    case M_TI:
      M_define_opcode_name_ti (sym_tbl);
      break;
    case M_SH:
#if 0
      M_define_opcode_name_sh (sym_tbl);
#endif
      break;
    case M_BX86:
      M_define_opcode_name_bx86 (sym_tbl);
      break;
    case M_STARCORE:
      M_define_opcode_name_starcore (sym_tbl);
      break;
    case M_ARM:
      M_define_opcode_name_arm (sym_tbl);
      break;
    case M_WIMS:
      M_define_opcode_name_wims (sym_tbl);
      break;
    default:
      M_assert (0, "M_define_opcode_name: illegal M_arch type");
    }
}

/* Return strings for architecture/model specific opcodes */
char *
M_get_opcode_name (int id)
{
  switch (M_arch)
    {
    case M_TAHOE:
      /*        return M_get_opcode_name_tahoe(id); */
    case M_IMPACT:
      return ("?");  /* Currently only supports Lcode opcodes -JCG 4/99 */
#if 0
    case M_SPARC:
      return M_get_opcode_name_sparc (id);
#endif
    case M_HPPA:
      return M_get_opcode_name_hppa (id);
    case M_X86:
      return M_get_opcode_name_x86 (id);
    case M_PLAYDOH:
      return M_get_opcode_name_playdoh (id);
    case M_TI:
      return M_get_opcode_name_ti (id);
#if 0
    case M_SH:
      return M_get_opcode_name_sh (id);
#endif
    case M_BX86:
      return M_get_opcode_name_bx86 (id);
    case M_STARCORE:
      return M_get_opcode_name_starcore (id);
    case M_ARM:
      return M_get_opcode_name_arm (id);
    case M_WIMS:
      return M_get_opcode_name_wims (id);
    default:
      M_assert (0, "M_get_opcode_name: illegal M_arch type");
      return (char *) 0;
    }
}

/*
 * Return true (1) if the instruction is supported in the hardware of the
 * processor.  Return false (0) otherwise.
 */
int
M_oper_supported_in_arch (int opc)
{
  switch (M_arch)
    {
    case M_IMPACT:
      return M_oper_supported_in_arch_impact (opc);
    case M_TAHOE:
      return M_oper_supported_in_arch_tahoe (opc);
    case M_SPARC:
      return M_oper_supported_in_arch_sparc (opc);
    case M_HPPA:
      return M_oper_supported_in_arch_hppa (opc);
    case M_X86:
      return M_oper_supported_in_arch_x86 (opc);
    case M_PLAYDOH:
      return M_oper_supported_in_arch_playdoh (opc);
    case M_TI:
      return M_oper_supported_in_arch_ti (opc);
    case M_SH:
      return M_oper_supported_in_arch_sh (opc);
    case M_BX86:
      return M_oper_supported_in_arch_bx86 (opc);
    case M_STARCORE:
      return M_oper_supported_in_arch_starcore (opc);
    case M_ARM:
      return M_oper_supported_in_arch_arm (opc);
    case M_WIMS:
      return M_oper_supported_in_arch_wims (opc);
    default:
      M_assert (0, "M_oper_supported_in_arch: illegal M_arch type");
      return 0;
    }
}

/*
 * Returns the number of machine instructions required to implement the
 * specified oper in the best case.  It is assumed that this is for 
 * supported instructions.  A call to M_oper_supported_in_arch should be
 * made for abnormal instructions.
 */
int
M_num_oper_required_for (L_Oper * oper, char *name)
{
  switch (M_arch)
    {
    case M_IMPACT:
      return M_num_oper_required_for_impact (oper, name);
    case M_TAHOE:
      return M_num_oper_required_for_tahoe (oper, name);
    case M_SPARC:
      return M_num_oper_required_for_sparc (oper, name);
    case M_HPPA:
      return M_num_oper_required_for_hppa (oper, name);
    case M_X86:
      return M_num_oper_required_for_x86 (oper, name);
    case M_PLAYDOH:
      return M_num_oper_required_for_playdoh (oper, name);
    case M_TI:
      return M_num_oper_required_for_ti (oper, name);
    case M_SH:
      return M_num_oper_required_for_sh (oper, name);
    case M_BX86:
      return M_num_oper_required_for_bx86 (oper, name);
    case M_STARCORE:
      return M_num_oper_required_for_starcore (oper, name);
    case M_ARM:
      return M_num_oper_required_for_arm (oper, name);
    case M_WIMS:
      return M_num_oper_required_for_wims (oper, name);
    default:
      M_assert (0, "M_num_oper_required_for: illegal M_arch type");
      return 0;
    }
}

int
M_is_stack_operand (L_Operand * operand)
{
  if (operand == NULL)
    return 0;

  switch (M_arch)
    {
    case M_IMPACT:
      return M_is_stack_operand_impact (operand);
    case M_TAHOE:
      return M_is_stack_operand_tahoe (operand);
    case M_SPARC:
      return M_is_stack_operand_sparc (operand);
    case M_HPPA:
      return M_is_stack_operand_hppa (operand);
    case M_X86:
      return M_is_stack_operand_x86 (operand);
    case M_PLAYDOH:
      return M_is_stack_operand_playdoh (operand);
    case M_TI:
      return M_is_stack_operand_ti (operand);
    case M_SH:
      return M_is_stack_operand_sh (operand);
    case M_BX86:
      return M_is_stack_operand_bx86 (operand);
    case M_STARCORE:
      return M_is_stack_operand_starcore (operand);
    case M_ARM:
      return M_is_stack_operand_arm (operand);
    case M_WIMS:
      return M_is_stack_operand_wims (operand);
    default:
      M_assert (0, "M_is_stack_operand: illegal M_arch type");
      return 0;
    }
}

int
M_is_unsafe_macro (L_Operand * operand)
{
  if (operand == NULL)
    return 0;

  switch (M_arch)
    {
    case M_IMPACT:
      return M_is_unsafe_macro_impact (operand);
    case M_TAHOE:
      return M_is_unsafe_macro_tahoe (operand);
    case M_SPARC:
      return M_is_unsafe_macro_sparc (operand);
    case M_HPPA:
      return M_is_unsafe_macro_hppa (operand);
    case M_X86:
      return M_is_unsafe_macro_x86 (operand);
    case M_PLAYDOH:
      return M_is_unsafe_macro_playdoh (operand);
    case M_TI:
      return M_is_unsafe_macro_ti (operand);
    case M_SH:
      return M_is_unsafe_macro_sh (operand);
    case M_BX86:
      return M_is_unsafe_macro_bx86 (operand);
    case M_STARCORE:
      return M_is_unsafe_macro_starcore (operand);
    case M_ARM:
      return M_is_unsafe_macro_arm (operand);
    case M_WIMS:
      return M_is_unsafe_macro_wims (operand);
    default:
      M_assert (0, "M_is_unsafe_macro: illegal M_arch type");
      return 0;
    }
}

IFPTR1
M_mdes_operand_type (void)
{
  switch (M_arch)
    {
    case M_IMPACT:
      return M_operand_type_impact;
    case M_TAHOE:
      return M_operand_type_tahoe;
    case M_HPPA:
      return M_operand_type_hppa;
    case M_PLAYDOH:
      return M_operand_type_playdoh;
    case M_SPARC:
      return M_operand_type_sparc;
    case M_X86:
      return M_operand_type_x86;
    case M_TI:
      return M_operand_type_ti;
    case M_SH:
      return M_operand_type_sh;
    case M_BX86:
      return M_operand_type_bx86;
    case M_STARCORE:
      return M_operand_type_starcore;
    case M_ARM:
      return M_operand_type_arm;
    case M_WIMS:
      return M_operand_type_wims;
    default:
      M_assert (0, "M_mdes_operand_type: illegal M_arch type");
      return (NULL);
    }
}

IFPTR2
M_dep_conflicting_operands (void)
{
  switch (M_arch)
    {
    case M_IMPACT:
      return M_conflicting_operands_impact;
    case M_TAHOE:
      return M_conflicting_operands_tahoe;
    case M_HPPA:
      return M_conflicting_operands_hppa;
    case M_PLAYDOH:
      return M_conflicting_operands_playdoh;
    case M_X86:
      return M_conflicting_operands_x86;
    case M_SPARC:
      return M_conflicting_operands_sparc;
    case M_TI:
      return M_conflicting_operands_ti;
    case M_SH:
      return M_conflicting_operands_sh;
    case M_BX86:
      return M_conflicting_operands_bx86;
    case M_STARCORE:
      return M_conflicting_operands_starcore;
    case M_ARM:
      return M_conflicting_operands_arm;
    case M_WIMS:
      return M_conflicting_operands_wims;
    default:
      M_assert (0, "M_dep_conflicting_operands: illegal M_arch type");
      return (NULL);
    }
}

/*
 *	For L_independent_memory_ops, for now this is hardwired for all archs
 *	right now until Dave G mods his stuff.
 *		first = location of first address operand
 *		number = number of address operands
 *		proc_opc = processor specific opcode
 */

void
M_get_memory_operands (int *first, int *number, int proc_opc)
{
  switch (M_arch)
    {
    case M_IMPACT:
    case M_HPPA:
    case M_PLAYDOH:
    case M_SPARC:
    case M_TI:
    case M_SH:
    case M_ARM:
    case M_WIMS:
      *first = 0;
      *number = 2;
      break;
    case M_TAHOE:
      M_get_memory_operands_tahoe (first, number, proc_opc);
      break;
    case M_X86:
      M_get_memory_operands_x86 (first, number, proc_opc);
      break;
    case M_BX86:
      M_get_memory_operands_bx86 (first, number, proc_opc);
      break;
    case M_STARCORE:
      M_get_memory_operands_starcore (first, number, proc_opc);
      break;
    default:
      *first = 0;
      *number = 2;
    }
}

int
M_num_registers (int ctype)
{
  switch (M_arch)
    {
    case M_IMPACT:
      return M_num_registers_impact (ctype);
    case M_TAHOE:
      return M_num_registers_tahoe (ctype);
    case M_SPARC:
      return M_num_registers_sparc (ctype);
    case M_HPPA:
      return M_num_registers_hppa (ctype);
    case M_X86:
      return M_num_registers_x86 (ctype);
    case M_PLAYDOH:
      return M_num_registers_playdoh (ctype);
    case M_TI:
      return M_num_registers_ti (ctype);
    case M_SH:
      return M_num_registers_sh (ctype);
    case M_BX86:
      return M_num_registers_bx86 (ctype);
    case M_STARCORE:
      return M_num_registers_starcore (ctype);
    case M_ARM:
      return M_num_registers_arm (ctype);
    case M_WIMS:
      return M_num_registers_wims (ctype);
    default:
      M_assert (0, "M_num_registers: illegal M_arch type");
      return 0;
    }
}


int
M_memory_access_size (L_Oper * op)
{
  if (op == NULL)
    M_assert (0, "M_memory_access_size: op is NULL");

  switch (M_arch)
    {
    case M_X86:
      return (M_memory_access_size_x86 (op));
    case M_BX86:
      return (M_memory_access_size_bx86 (op));
    case M_STARCORE:
      return (M_memory_access_size_starcore (op));
    case M_IMPACT:
    case M_TAHOE:
    case M_SPARC:
    case M_HPPA:
    case M_PLAYDOH:
    case M_TI:
    case M_SH:
    case M_ARM:
    case M_WIMS:
    default:
      M_assert (0, "M_memory_access_size: illegal opcode");
      return (0);
    }
}


int
M_get_data_type (L_Oper * op)
{
  if (op == NULL)
    M_assert (0, "M_get_data_type: op is NULL");

  switch (M_arch)
    {
    case M_X86:
      return (M_get_data_type_x86 (op));
    case M_BX86:
      return (M_get_data_type_bx86 (op));
    case M_STARCORE:
      return (M_get_data_type_starcore (op));
    case M_IMPACT:
    case M_TAHOE:
    case M_SPARC:
    case M_HPPA:
    case M_PLAYDOH:
    case M_TI:
    case M_SH:
    case M_ARM:
    case M_WIMS:
    default:
      M_assert (0, "M_get_data_type: illegal opcode");
      return (0);
    }
}

int
M_scaled_addressing_avail (void)
{
  switch (M_arch)
    {
    case M_X86:
    case M_BX86:
      return (1);
    case M_IMPACT:
    case M_TAHOE:
    case M_SPARC:
    case M_HPPA:
    case M_PLAYDOH:
    case M_TI:
    case M_SH:
    case M_STARCORE:
    case M_ARM:
    case M_WIMS:
    default:
      return (0);
    }
}

int
M_is_implicit_memory_op (L_Oper * oper)
{

  switch (M_arch)
    {
    case M_X86:
      return (M_is_implicit_memory_op_x86 (oper));
    case M_BX86:
      return (M_is_implicit_memory_op_bx86 (oper));
    case M_STARCORE:
      return (M_is_implicit_memory_op_starcore (oper));
    case M_IMPACT:
    case M_TAHOE:
    case M_SPARC:
    case M_HPPA:
    case M_PLAYDOH:
    case M_TI:
    case M_SH:
    case M_ARM:
    case M_WIMS:
    default:
      return (0);
    }
}

int
M_opc_from_proc_opc (int proc_opc)
{
  switch (M_arch)
    {
    case M_X86:
    case M_IMPACT:
    case M_SPARC:
    case M_HPPA:
    case M_TI:
    case M_SH:
    case M_BX86:
    case M_ARM:
    case M_WIMS:
      M_assert (0, "M_opc_from_proc_opc: Illegal architecture!");
      return (0);
    case M_TAHOE:
      return (M_opc_from_proc_opc_tahoe (proc_opc));
    case M_PLAYDOH:
      return (M_opc_from_proc_opc_playdoh (proc_opc));
    case M_STARCORE:
      return (M_opc_from_proc_opc_starcore (proc_opc));
    default:
      M_assert (0, "M_opc_from_proc_opc: Illegal architecture!");
      return (0);
    }
}

/* IA64 sias 20000517 */

int
M_cannot_predicate (L_Oper * oper)
{

  switch (M_arch)
    {
    case M_TAHOE:
      return (M_cannot_predicate_tahoe (oper));
    case M_IMPACT:
    case M_PLAYDOH:
    case M_ARM:
      return (0);
    default:
      return (1);
    }
}

/* Return a bit vector that represents the src operands to try and coalesce.
 * 01 - src[0], 10 - src[1], 11 - src[0] & src[1]
 */
int
M_coalescing_oper (L_Oper *oper)
{
  switch (M_arch)
    {
    case M_STARCORE:
      return (M_starcore_coalescing_oper (oper));
    default:
      return 0;
    }
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



