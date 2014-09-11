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
 *      File: l_mve.h
 *      Description: Structures and external function declarations for modulo
 *                   variable expansion
 *      Creation Date: January, 1994
 *      Author: Daniel Lavery
 *
 *      Revision Date: Summer 1999
 *      Authors: IMPACT Technologies: Matthew Merten and John Gyllenhaal
\*****************************************************************************/

#ifndef L_MVE_H
#define L_MVE_H

/* 10/29/02 REK Adding config.h */
#include <config.h>
#include <Lcode/sm.h>

/* memory allocation pools for the above structures and arrays of
   pointers to them */
extern L_Alloc_Pool *Lpipe_src_mve_pool;
extern L_Alloc_Pool *Lpipe_pred_mve_pool;
extern L_Alloc_Pool *Lpipe_dest_mve_pool;
extern L_Alloc_Pool *Lpipe_dest_lr_pool;
extern L_Alloc_Pool *Lpipe_MVEInfo_pool;

extern void Lpipe_free_mve_info ();

extern int Lpipe_analyze_lr (SM_Cb *, int);

extern int Lpipe_rreg_transform (SM_Cb *);
extern void Lpipe_fix_live_in_rot (SM_Cb *);
extern void Lpipe_fix_live_out_rot (SM_Cb *, SM_Oper *);

extern void Lpipe_mve_transform (SM_Cb *, int);
extern void Lpipe_fix_live_in (SM_Cb *);
extern void Lpipe_fix_live_out (SM_Cb *, int);
extern int Lpipe_find_name (SM_Cb *, Lpipe_LRInfo *, Lpipe_MVEInfo *);
extern int Lpipe_get_pro_reg (SM_Cb *sm_cb, Lpipe_LRInfo *lr_info, 
			      Lpipe_MVEInfo *mve_info, int stg_ofst);
#endif
