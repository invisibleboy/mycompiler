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
 *      File :          constant.h
 *      Description :   Adjust constants for sign and zero extension
 *      Creation Date : September 1997
 *      Author :        Robert McGowan
 *
 *==========================================================================*/

#ifndef CONSTANT_H_
#define CONSTANT_H_

/* 10/29/02 REK Adding config.h */
#include <config.h>

#ifdef IT64BIT
#define LLONG_SIGN_BIT          ULLCONST(0x8000000000000000)
#define UNSIGNED_LLONG_MAX      ULLCONST(0xFFFFFFFFFFFFFFFF)
#define SIGNED_LLONG_POS_MAX    LLCONST(0x7FFFFFFFFFFFFFFF)
#define SIGNED_LLONG_NEG_MAX    LLONG_SIGN_BIT
#endif

#define INT_SIGN_BIT          0x80000000
#define UNSIGNED_INT_MAX      0xFFFFFFFF
#define SIGNED_INT_POS_MAX    0x7FFFFFFF
#define SIGNED_INT_NEG_MAX    INT_SIGN_BIT

#define SHORT_SIGN_BIT        0x8000
#define UNSIGNED_SHORT_MAX    0xFFFF
#define SIGNED_SHORT_POS_MAX  0x7FFF
#define SIGNED_SHORT_NEG_MAX  SHORT_SIGN_BIT

#define CHAR_SIGN_BIT         0x80
#define UNSIGNED_CHAR_MAX     0xFF
#define SIGNED_CHAR_POS_MAX   0x7F
#define SIGNED_CHAR_NEG_MAX   CHAR_SIGN_BIT

extern L_Operand *L_copy_immed_operand (unsigned char new_ctype,
                                        unsigned char old_ctype,
                                        L_Operand * from_operand);

#endif
