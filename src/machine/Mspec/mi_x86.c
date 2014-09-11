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
 *	File :	mi_x86.c 
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

int
M_x86_type_size (int mtype)
{
  switch (mtype)
    {
    case M_TYPE_VOID:
      return M_X86_SIZE_VOID;
    case M_TYPE_BIT_LONG:
      return M_X86_SIZE_BIT;
    case M_TYPE_BIT_CHAR:
      return M_X86_SIZE_BIT;
    case M_TYPE_CHAR:
      return M_X86_SIZE_CHAR;
    case M_TYPE_SHORT:
      return M_X86_SIZE_SHORT;
    case M_TYPE_INT:
      return M_X86_SIZE_INT;
    case M_TYPE_LONG:
      return M_X86_SIZE_LONG;
    case M_TYPE_FLOAT:
      return M_X86_SIZE_FLOAT;
    case M_TYPE_DOUBLE:
      return M_X86_SIZE_DOUBLE;
    case M_TYPE_POINTER:
      return M_X86_SIZE_POINTER;
    case M_TYPE_UNION:
      return M_X86_SIZE_UNION;
    case M_TYPE_STRUCT:
      return M_X86_SIZE_STRUCT;
    case M_TYPE_BLOCK:
      return M_X86_SIZE_BLOCK;
    default:
      return -1;
    }
}

int
M_x86_type_align (int mtype)
{
  switch (mtype)
    {
    case M_TYPE_VOID:
      return M_X86_ALIGN_VOID;
    case M_TYPE_BIT_LONG:
      return M_X86_ALIGN_BIT;
    case M_TYPE_BIT_CHAR:
      return M_X86_ALIGN_BIT;
    case M_TYPE_CHAR:
      return M_X86_ALIGN_CHAR;
    case M_TYPE_SHORT:
      return M_X86_ALIGN_SHORT;
    case M_TYPE_INT:
      return M_X86_ALIGN_INT;
    case M_TYPE_LONG:
      return M_X86_ALIGN_LONG;
    case M_TYPE_FLOAT:
      return M_X86_ALIGN_FLOAT;
    case M_TYPE_DOUBLE:
      return M_X86_ALIGN_DOUBLE;
    case M_TYPE_POINTER:
      return M_X86_ALIGN_POINTER;
    case M_TYPE_UNION:
      return M_X86_ALIGN_UNION;
    case M_TYPE_STRUCT:
      return M_X86_ALIGN_STRUCT;
    case M_TYPE_BLOCK:
      return M_X86_ALIGN_BLOCK;
    default:
      return -1;
    }
}

void
M_x86_char (M_Type type, int unsign)
{
  type->type = M_TYPE_CHAR;
  type->unsign = unsign;
  type->align = M_X86_ALIGN_CHAR;
  type->size = M_X86_SIZE_CHAR;
  type->nbits = M_X86_SIZE_CHAR;
}

void
M_x86_short (M_Type type, int unsign)
{
  type->type = M_TYPE_SHORT;
  type->unsign = unsign;
  type->align = M_X86_ALIGN_SHORT;
  type->size = M_X86_SIZE_SHORT;
  type->nbits = M_X86_SIZE_SHORT;
}

void
M_x86_int (M_Type type, int unsign)
{
  type->type = M_TYPE_INT;
  type->unsign = unsign;
  type->align = M_X86_ALIGN_INT;
  type->size = M_X86_SIZE_INT;
  type->nbits = M_X86_SIZE_INT;
}

void
M_x86_long (M_Type type, int unsign)
{
  type->type = M_TYPE_LONG;
  type->unsign = unsign;
  type->align = M_X86_ALIGN_LONG;
  type->size = M_X86_SIZE_LONG;
  type->nbits = M_X86_SIZE_LONG;
}

/*--------------------------------------------------------------------------*/
int
M_x86_layout_order (void)
{
  return M_LITTLE_ENDIAN;
}


void
M_set_model_x86 (char *model_name)
{
  if (!strcasecmp (model_name, "386"))
    M_model = M_386;
  else if (!strcasecmp (model_name, "486"))
    M_model = M_486;
  else if (!strcasecmp (model_name, "Pentium"))
    M_model = M_PENTIUM;
  else if (!strcasecmp (model_name, "Krypton") ||
	   !strcasecmp (model_name, "K5"))
    M_model = M_KRYPTON;
  else
    M_assert (0, "M_set_model_x86:  Illegal model specified!");
}
