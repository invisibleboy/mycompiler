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
 *
 *  File:  l_ru_interface.h
 *
 *  Description:
 *    Header file for Lcode interface to RU manager
 *
 *  Creation Date : May 1993
 *
 *  Authors : Scott Mahlke, John Gyllenhaal
 *
 *  Revision 1.1.1.1  1995/08/30 16:49:03  david
 *  Import of IMPACT source
 *
 * Revision 1.1  1994/01/19  18:49:26  roger
 * Initial revision
 *
 *
\*****************************************************************************/
#ifndef L_RU_INTERFACE_H
#define L_RU_INTERFACE_H

/* 10/29/02 REK Adding config.h */
#include <config.h>

/*
 *	External functions
 */

extern RU_Info* L_create_ru_info_oper (L_Oper *oper);
	/* (L_Oper *oper) */

extern void L_create_ru_info_cb (L_Cb *cb);
	/* (L_Cb *cb) */

extern void L_create_ru_info_fn (L_Func *fn);
	/* (L_Func *fn) */

extern void L_delete_ru_info_oper (RU_Info *ru_info);
	/* (L_Oper *oper) */

extern void L_delete_ru_info_cb (L_Cb *cb);
	/* (L_Cb *cb) */
	
extern void L_delete_ru_info_fn (L_Func *fn);
	/* (L_Func *fn) */

#endif
