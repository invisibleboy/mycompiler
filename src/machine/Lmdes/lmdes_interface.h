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
/******************************************************************************\
 *  File:  lmdes_interface.h
 *
 *  Description:
 *    Header file for Lcode interface functions to lmdes
 *
 *  Creation Date :  June, 1993
 *
 *  Authors:  John C. Gyllenhaal, Wen-mei Hwu
 *
\******************************************************************************/
#ifndef LMDES_INTERFACE_H
#define LMDES_INTERFACE_H
/* 10/29/02 REK Adding config.h */
#include <config.h>
#include "lmdes.h"

/*
 * Mcode interface functions to mdes.
 */

#ifdef __cplusplus
extern "C"
{
#endif

/* Prototypes */
  void L_build_oper_mdes_info (L_Oper * op);	/* (L_Oper *op) */
  void L_build_cb_mdes_info (L_Cb * cb);	/* (L_Cb *cb) */

  void L_free_oper_mdes_info (L_Oper * op);	/* (L_Oper *op) */
  void L_free_cb_mdes_info (L_Cb * cb);	/* (L_Cb *cb) */

#ifdef __cplusplus
}
#endif

#endif
