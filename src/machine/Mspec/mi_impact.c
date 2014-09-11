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
 *	File:	mi_impact.c
 *	Author:	Scott A. Mahlke, Wen-mei Hwu
 *	Creation Date:	September 1993
 *  
 * Revised by IMPACT Technologies Inc. (John Gyllenhaal) to support 
 * the 'Lcode' model for the 'IMPACT' architecture. -2/99
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
#include <machine/m_impact.h>
#include <machine/m_hppa.h>
#include <machine/m_sparc.h>
/*
 *	The way this is set up now is the following:
 *	If my target is ver1 of impact, i use the functions
 *	defined in here, otherwise I call the functions defined
 *	in M_hppa.c since I am using HP macros, HP calling convention,
 *	HP data layout, HP stack layout, etc.
 *
 */

/* model compiling for, internal to this Mspec only */
int M_impact_model;

/* New portable model IMPACT/Lcode, which is based on HPPA/PA-7200 due
 * to our familiarity with it (and so our old hppa-specific tools will work)
 * -ITI (JCG) 2/99
 */
#define M_IMPACT_LCODE_SIZE_VOID        0
#define M_IMPACT_LCODE_SIZE_BIT         1
#define M_IMPACT_LCODE_SIZE_CHAR        8
#define M_IMPACT_LCODE_SIZE_SHORT       16
#define M_IMPACT_LCODE_SIZE_INT         32
#define M_IMPACT_LCODE_SIZE_LONG        32
#define M_IMPACT_LCODE_SIZE_FLOAT       32
#define M_IMPACT_LCODE_SIZE_DOUBLE      64
#define M_IMPACT_LCODE_SIZE_POINTER     32
#define M_IMPACT_LCODE_SIZE_UNION       -1
#define M_IMPACT_LCODE_SIZE_STRUCT      -1
#define M_IMPACT_LCODE_SIZE_BLOCK       -1
#define M_IMPACT_LCODE_SIZE_MAX         64

#define M_IMPACT_LCODE_ALIGN_VOID       -1
#define M_IMPACT_LCODE_ALIGN_BIT        1
#define M_IMPACT_LCODE_ALIGN_CHAR       8
#define M_IMPACT_LCODE_ALIGN_SHORT      16
#define M_IMPACT_LCODE_ALIGN_INT        32
#define M_IMPACT_LCODE_ALIGN_LONG       32
#define M_IMPACT_LCODE_ALIGN_FLOAT      32
#define M_IMPACT_LCODE_ALIGN_DOUBLE     64
#define M_IMPACT_LCODE_ALIGN_POINTER    32
#define M_IMPACT_LCODE_ALIGN_UNION      -1	/* depends on the field */
#define M_IMPACT_LCODE_ALIGN_STRUCT     -1
#define M_IMPACT_LCODE_ALIGN_BLOCK      -1
#define M_IMPACT_LCODE_ALIGN_MAX        64


#define M_IMPACT_SIZE_VOID	0
#define M_IMPACT_SIZE_BIT	1
#define M_IMPACT_SIZE_CHAR	8
#define M_IMPACT_SIZE_SHORT	16
#define M_IMPACT_SIZE_INT	32
#define M_IMPACT_SIZE_LONG	32
#define M_IMPACT_SIZE_FLOAT	32
#define M_IMPACT_SIZE_DOUBLE	64
#define M_IMPACT_SIZE_POINTER	32
#define M_IMPACT_SIZE_UNION	-1
#define M_IMPACT_SIZE_STRUCT	-1
#define M_IMPACT_SIZE_BLOCK	-1
#define M_IMPACT_SIZE_MAX	64

#define M_IMPACT_ALIGN_VOID	-1
#define M_IMPACT_ALIGN_BIT	1
#define M_IMPACT_ALIGN_CHAR	8
#define M_IMPACT_ALIGN_SHORT	16
#define M_IMPACT_ALIGN_INT	32
#define M_IMPACT_ALIGN_LONG	32
#define M_IMPACT_ALIGN_FLOAT	32
#define M_IMPACT_ALIGN_DOUBLE	64
#define M_IMPACT_ALIGN_POINTER	32
#define M_IMPACT_ALIGN_UNION	-1	/* depends on the field */
#define M_IMPACT_ALIGN_STRUCT	-1
#define M_IMPACT_ALIGN_BLOCK	-1
#define M_IMPACT_ALIGN_MAX	64

int
M_impact_type_size (int mtype)
{
  if (M_impact_model == M_IM_LCODE)
    {
      switch (mtype)
	{
	case M_TYPE_VOID:
	  return M_IMPACT_LCODE_SIZE_VOID;
	case M_TYPE_BIT_LONG:
	  return M_IMPACT_LCODE_SIZE_BIT;
	case M_TYPE_BIT_CHAR:
	  return M_IMPACT_LCODE_SIZE_BIT;
	case M_TYPE_CHAR:
	  return M_IMPACT_LCODE_SIZE_CHAR;
	case M_TYPE_SHORT:
	  return M_IMPACT_LCODE_SIZE_SHORT;
	case M_TYPE_INT:
	  return M_IMPACT_LCODE_SIZE_INT;
	case M_TYPE_LONG:
	  return M_IMPACT_LCODE_SIZE_LONG;
	case M_TYPE_FLOAT:
	  return M_IMPACT_LCODE_SIZE_FLOAT;
	case M_TYPE_DOUBLE:
	  return M_IMPACT_LCODE_SIZE_DOUBLE;
	case M_TYPE_POINTER:
	  return M_IMPACT_LCODE_SIZE_POINTER;
	case M_TYPE_UNION:
	  return M_IMPACT_LCODE_SIZE_UNION;
	case M_TYPE_STRUCT:
	  return M_IMPACT_LCODE_SIZE_STRUCT;
	case M_TYPE_BLOCK:
	  return M_IMPACT_LCODE_SIZE_BLOCK;
	default:
	  return -1;
	}
    }
  else if (M_impact_model == M_IM_VER_1)
    {
      switch (mtype)
	{
	case M_TYPE_VOID:
	  return M_IMPACT_SIZE_VOID;
	case M_TYPE_BIT_LONG:
	  return M_IMPACT_SIZE_BIT;
	case M_TYPE_BIT_CHAR:
	  return M_IMPACT_SIZE_BIT;
	case M_TYPE_CHAR:
	  return M_IMPACT_SIZE_CHAR;
	case M_TYPE_SHORT:
	  return M_IMPACT_SIZE_SHORT;
	case M_TYPE_INT:
	  return M_IMPACT_SIZE_INT;
	case M_TYPE_LONG:
	  return M_IMPACT_SIZE_LONG;
	case M_TYPE_FLOAT:
	  return M_IMPACT_SIZE_FLOAT;
	case M_TYPE_DOUBLE:
	  return M_IMPACT_SIZE_DOUBLE;
	case M_TYPE_POINTER:
	  return M_IMPACT_SIZE_POINTER;
	case M_TYPE_UNION:
	  return M_IMPACT_SIZE_UNION;
	case M_TYPE_STRUCT:
	  return M_IMPACT_SIZE_STRUCT;
	case M_TYPE_BLOCK:
	  return M_IMPACT_SIZE_BLOCK;
	default:
	  return -1;
	}
    }
  else if ((M_impact_model == M_IM_HP_LCODE) ||
	   (M_impact_model == M_IM_HP_MCODE))
    {
      return (M_hppa_type_size (mtype));
    }
  else if ((M_impact_model == M_IM_SPARC_LCODE) ||
	   (M_impact_model == M_IM_SPARC_MCODE))
    {
      return (M_sparc_type_size (mtype));
    }
  else
    {
      M_assert (0, "M_impact_type_size: illegal machine model");
      return (0);
    }
}

int
M_impact_type_align (int mtype)
{
  if (M_impact_model == M_IM_LCODE)
    {
      switch (mtype)
	{
	case M_TYPE_VOID:
	  return M_IMPACT_LCODE_ALIGN_VOID;
	case M_TYPE_BIT_LONG:
	  return M_IMPACT_LCODE_ALIGN_BIT;
	case M_TYPE_BIT_SHORT:
	  return M_IMPACT_LCODE_ALIGN_BIT;
	case M_TYPE_BIT_CHAR:
	  return M_IMPACT_LCODE_ALIGN_BIT;
	case M_TYPE_CHAR:
	  return M_IMPACT_LCODE_ALIGN_CHAR;
	case M_TYPE_SHORT:
	  return M_IMPACT_LCODE_ALIGN_SHORT;
	case M_TYPE_INT:
	  return M_IMPACT_LCODE_ALIGN_INT;
	case M_TYPE_LONG:
	  return M_IMPACT_LCODE_ALIGN_LONG;
	case M_TYPE_FLOAT:
	  return M_IMPACT_LCODE_ALIGN_FLOAT;
	case M_TYPE_DOUBLE:
	  return M_IMPACT_LCODE_ALIGN_DOUBLE;
	case M_TYPE_POINTER:
	  return M_IMPACT_LCODE_ALIGN_POINTER;
	case M_TYPE_UNION:
	  return M_IMPACT_LCODE_ALIGN_UNION;
	case M_TYPE_STRUCT:
	  return M_IMPACT_LCODE_ALIGN_STRUCT;
	case M_TYPE_BLOCK:
	  return M_IMPACT_LCODE_ALIGN_BLOCK;
	default:
	  return -1;
	}
    }
  else if (M_impact_model == M_IM_VER_1)
    {
      switch (mtype)
	{
	case M_TYPE_VOID:
	  return M_IMPACT_ALIGN_VOID;
	case M_TYPE_BIT_LONG:
	  return M_IMPACT_ALIGN_BIT;
	case M_TYPE_BIT_SHORT:
	  return M_IMPACT_ALIGN_BIT;
	case M_TYPE_BIT_CHAR:
	  return M_IMPACT_ALIGN_BIT;
	case M_TYPE_CHAR:
	  return M_IMPACT_ALIGN_CHAR;
	case M_TYPE_SHORT:
	  return M_IMPACT_ALIGN_SHORT;
	case M_TYPE_INT:
	  return M_IMPACT_ALIGN_INT;
	case M_TYPE_LONG:
	  return M_IMPACT_ALIGN_LONG;
	case M_TYPE_FLOAT:
	  return M_IMPACT_ALIGN_FLOAT;
	case M_TYPE_DOUBLE:
	  return M_IMPACT_ALIGN_DOUBLE;
	case M_TYPE_POINTER:
	  return M_IMPACT_ALIGN_POINTER;
	case M_TYPE_UNION:
	  return M_IMPACT_ALIGN_UNION;
	case M_TYPE_STRUCT:
	  return M_IMPACT_ALIGN_STRUCT;
	case M_TYPE_BLOCK:
	  return M_IMPACT_ALIGN_BLOCK;
	default:
	  return -1;
	}
    }
  else if ((M_impact_model == M_IM_HP_LCODE) ||
	   (M_impact_model == M_IM_HP_MCODE))
    {
      return (M_hppa_type_align (mtype));
    }
  else if ((M_impact_model == M_IM_SPARC_LCODE) ||
	   (M_impact_model == M_IM_SPARC_MCODE))
    {
      return (M_sparc_type_align (mtype));
    }
  else
    {
      M_assert (0, "M_impact_type_align: illegal machine model");
      return (0);
    }
}


void
M_impact_char (M_Type type, int unsign)
{
  if (M_impact_model == M_IM_LCODE)
    {
      type->type = M_TYPE_CHAR;
      type->unsign = (unsign != 0);
      type->align = M_IMPACT_LCODE_ALIGN_CHAR;
      type->size = M_IMPACT_LCODE_SIZE_CHAR;
      type->nbits = M_IMPACT_LCODE_SIZE_CHAR;
    }
  else if (M_impact_model == M_IM_VER_1)
    {
      type->type = M_TYPE_CHAR;
      type->unsign = (unsign != 0);
      type->align = M_IMPACT_ALIGN_CHAR;
      type->size = M_IMPACT_SIZE_CHAR;
      type->nbits = M_IMPACT_SIZE_CHAR;
    }
  else if ((M_impact_model == M_IM_HP_LCODE) ||
	   (M_impact_model == M_IM_HP_MCODE))
    {
      M_hppa_char (type, unsign);
    }
  else if ((M_impact_model == M_IM_SPARC_LCODE) ||
	   (M_impact_model == M_IM_SPARC_MCODE))
    {
      M_sparc_char (type, unsign);
    }
  else
    {
      M_assert (0, "M_impact_char: illegal machine model");
    }
}

void
M_impact_short (M_Type type, int unsign)
{
  if (M_impact_model == M_IM_LCODE)
    {
      type->type = M_TYPE_SHORT;
      type->unsign = (unsign != 0);
      type->align = M_IMPACT_LCODE_ALIGN_SHORT;
      type->size = M_IMPACT_LCODE_SIZE_SHORT;
      type->nbits = M_IMPACT_LCODE_SIZE_SHORT;
    }
  else if (M_impact_model == M_IM_VER_1)
    {
      type->type = M_TYPE_SHORT;
      type->unsign = (unsign != 0);
      type->align = M_IMPACT_ALIGN_SHORT;
      type->size = M_IMPACT_SIZE_SHORT;
      type->nbits = M_IMPACT_SIZE_SHORT;
    }
  else if ((M_impact_model == M_IM_HP_LCODE) ||
	   (M_impact_model == M_IM_HP_MCODE))
    {
      M_hppa_short (type, unsign);
    }
  else if ((M_impact_model == M_IM_SPARC_LCODE) ||
	   (M_impact_model == M_IM_SPARC_MCODE))
    {
      M_sparc_short (type, unsign);
    }
  else
    {
      M_assert (0, "M_impact_short: illegal machine model");
    }
}

void
M_impact_int (M_Type type, int unsign)
{
  if (M_impact_model == M_IM_LCODE)
    {
      type->type = M_TYPE_INT;
      type->unsign = (unsign != 0);
      type->align = M_IMPACT_LCODE_ALIGN_INT;
      type->size = M_IMPACT_LCODE_SIZE_INT;
      type->nbits = M_IMPACT_LCODE_SIZE_INT;
    }
  else if (M_impact_model == M_IM_VER_1)
    {
      type->type = M_TYPE_INT;
      type->unsign = (unsign != 0);
      type->align = M_IMPACT_ALIGN_INT;
      type->size = M_IMPACT_SIZE_INT;
      type->nbits = M_IMPACT_SIZE_INT;
    }
  else if ((M_impact_model == M_IM_HP_LCODE) ||
	   (M_impact_model == M_IM_HP_MCODE))
    {
      M_hppa_int (type, unsign);
    }
  else if ((M_impact_model == M_IM_SPARC_LCODE) ||
	   (M_impact_model == M_IM_SPARC_MCODE))
    {
      M_sparc_int (type, unsign);
    }
  else
    {
      M_assert (0, "M_impact_int: illegal machine model");
    }
}

void
M_impact_long (M_Type type, int unsign)
{
  if (M_impact_model == M_IM_LCODE)
    {
      type->type = M_TYPE_LONG;
      type->unsign = (unsign != 0);
      type->align = M_IMPACT_LCODE_ALIGN_LONG;
      type->size = M_IMPACT_LCODE_SIZE_LONG;
      type->nbits = M_IMPACT_LCODE_SIZE_LONG;
    }
  else if (M_impact_model == M_IM_VER_1)
    {
      type->type = M_TYPE_LONG;
      type->unsign = (unsign != 0);
      type->align = M_IMPACT_ALIGN_LONG;
      type->size = M_IMPACT_SIZE_LONG;
      type->nbits = M_IMPACT_SIZE_LONG;
    }
  else if ((M_impact_model == M_IM_HP_LCODE) ||
	   (M_impact_model == M_IM_HP_MCODE))
    {
      M_hppa_long (type, unsign);
    }
  else if ((M_impact_model == M_IM_SPARC_LCODE) ||
	   (M_impact_model == M_IM_SPARC_MCODE))
    {
      M_sparc_long (type, unsign);
    }
  else
    {
      M_assert (0, "M_impact_long: illegal machine model");
    }
}

int
M_impact_layout_order (void)
{
  if (M_impact_model == M_IM_LCODE)
    {
      /* Use host info database to determine if little or big endian.
       * -ITI(JCG) 6/99
       */
      if (M_read_database_i ("_HT__info", "little_endian", "value") == 1)
	{
	  return M_LITTLE_ENDIAN;
	}
      else
	{
	  return M_BIG_ENDIAN;
	}
    }
  else if (M_impact_model == M_IM_VER_1)
    {
      return M_LITTLE_ENDIAN;
    }
  else if ((M_impact_model == M_IM_HP_LCODE) ||
	   (M_impact_model == M_IM_HP_MCODE))
    {
      return (M_hppa_layout_order ());
    }
  else if ((M_impact_model == M_IM_SPARC_LCODE) ||
	   (M_impact_model == M_IM_SPARC_MCODE))
    {
      return (M_sparc_layout_order ());
    }
  else
    {
      M_assert (0, "M_impact_layout_order: illegal machine model");
      return (0);
    }
}

void
M_set_model_impact (char *model_name)
{
  /* The 'lcode' model is the only model supported in the public
   * release.  The rest are being phased out. -JCG 2/99
   */
  if (!strcasecmp (model_name, "lcode"))
    {
      M_impact_model = M_IM_LCODE;
      M_model = M_IM_LCODE;

      /* Use the layout database to answer as many queries as possible */
      M_use_layout_database = 1;
    }
  else if (!strcasecmp (model_name, "V1.0"))
    {
      M_impact_model = M_IM_VER_1;
      M_model = M_IM_VER_1;
    }
  else if ((!strcasecmp (model_name, "HP-LCODE")) ||
	   (!strcasecmp (model_name, "HP_LCODE")))
    {
      M_impact_model = M_IM_HP_LCODE;
      M_set_model_hppa ("PA_7100");
    }
  else if ((!strcasecmp (model_name, "HP-MCODE")) ||
	   (!strcasecmp (model_name, "HP_MCODE")))
    {
      M_impact_model = M_IM_HP_MCODE;
      M_set_model_hppa ("PA_7100");
    }
#ifndef IMPACT_EXTERNAL
  else if ((!strcasecmp (model_name, "SPARC-LCODE")) ||
	   (!strcasecmp (model_name, "SPARC_LCODE")))
    {
      M_impact_model = M_IM_SPARC_LCODE;
      /* REH 1/18/95 - If you change this model to something */
      /* else you need to alter the model checked by         */
      /* O_register_allocation in limpact_phase2_reg.c       */
      M_set_model_sparc ("VIKING");
    }
  else if ((!strcasecmp (model_name, "SPARC-MCODE")) ||
	   (!strcasecmp (model_name, "SPARC_MCODE")))
    {
      M_impact_model = M_IM_SPARC_MCODE;
      /* REH 1/18/95 - If you change this model to something */
      /* else you need to alter the model checked by         */
      /* O_register_allocation in limpact_phase2_reg.c       */
      M_set_model_sparc ("VIKING");
    }
#endif
  else
    {
      fprintf (stderr, "illegal model_name : %s\n", model_name);
      fprintf (stderr,
	       "Use one of the following: V1.0, HP-LCODE, HP-MCODE\n");
      M_assert (0, "M_set_model_impact: illegal model name specified");
    }
}
