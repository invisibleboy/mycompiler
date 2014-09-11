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
/* Author: John C. Gyllenhaal, Wen-mei Hwu                                   */
#include <config.h>
#include <stdio.h>
#include "l_encode_interface.h"

#define OPERAND_HASH_SIZE	128

typedef struct eoperand
{
  int		id;	/* Internal id of Eoperand */
  unsigned int	flags;  /* Is it a register */
  unsigned int	hash_val;
  L_Operand		*operand;
  struct eoperand	*next_linear;
  struct eoperand	*next_hash;
} Eoperand;

typedef struct eop
{
  int		index;		/* Internal id of Eop */
  int		flags;		/* Flags to be passed to Lsim */
  int		playdoh_flags;	/* playdoh flags to be passed to Lsim */
  int		id;		/* Lcode id */
  int		opc;
  int		proc_opc;
  int		cycle;
  int		slot;
  int		latency[3];
  int		dep_id;		/* Dependence id (sync arch) */
  int		br_target;	/* branch target (cb id), 0 if none */
  int           loop_id;        /* HCH: id of function loop containing the op */
  short	        *eoperand;	/* Array of eoperands ids (dest, src, pred) */
  struct eop	*next;
} Eop;

typedef struct ecb
{
  int		id;		/* Lcode id */
  int		first_op;	/* index of first op in cb */
  L_Attr*       prehead_cbs;    
  struct ecb	*next;
} Ecb;

typedef struct efn
{
  char          *name;
  char	        *asm_name;
  Eop		*head_eop;
  Eop		*tail_eop;
  int		eop_count;
  Eoperand	*eoperand_hash[OPERAND_HASH_SIZE];
  Eoperand	*head_eoperand;
  Eoperand	*tail_eoperand;
  int		eoperand_count;	   /* Number of unique eoperands */
  Ecb		*head_ecb;
  Ecb		*tail_ecb;
  int		ecb_count;
  int		sched_info_avail;  /* 1 if sched info avail, 0 otherwise*/
} Efn;

