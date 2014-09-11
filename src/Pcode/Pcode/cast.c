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
/*! \file
 * \brief Functions to cast expressions.
 *
 * \author David August, Nancy Warter, and Wen-mei Hwu
 *
 * Modified from code written by: Po-hua Chang
 *
 * Copyright (c) 1991 David August, Nancy Warter, Po-hua Chnag, Wen-mei Hwu
 * and The Board of Trustees of the University of Illinois.
 * All rights reserved.
 *
 * The University of Illinois software License Agreement specifies the terms
 * and conditions for redistribution.
 *
 * NJW - this file provides both functions to determine the type
 * of a variable and also the cast of an expression.  I'm not sure
 * we want to reduce or simplify expressions in pcode (unless specifically
 * requested by a transformation) since this will all be done in 
 * hcode.  However, some of these functions are useful and should be
 * usable as is.
 *
 * Structure assignment is allowed.
 * e.g. struct _buf x, y;
 * 	x = y; 		is allowed.
 * 	x = \&y;        is also allowed.
 * When the LHS of an assignment is a structure, it copies
 * sizeof(structure) bytes from the memory location specified
 * by the RHS operand.
 *
 * A function may return a structure. It actually returns
 * a pointer to the structure (struct/union/array).
 * When structure (struct/union), array, and function are
 * passed to another function as parameters, their memory
 * location (pointers to them) are passed instead.
 *
 * The constant literals can actually be modified by
 * the user (bad practice). Therefore, it may be wrong
 * to assume they are all constants. For example, some
 * programs actually rewrite the string content (string I/O).
 * When a constant string is passed as parameter, it may be
 * changed. In this version of the C compiler, it is
 * assumed that modifying constants causes unpredictable
 * effect. This assumption allows the compiler more freedom
 * to perform code optimization.
 * As a direct consequence, the types of all constant expressions
 * are marked as TY_CONST.
 *
 * integral types = [short, long, unsigned, signed, int, char, enum]
 * arithmetic types = integral types + [float, double]
 * pointer types = [pointer, 0-dimension array]
 * fundamental types = arithmetic types + pointer types
 * structure types = [union, struct]
 * array type = [N-dimension array, N is defined]
 * function type = [func]
 *
 * ** pointer and array are inter-changeable.
 */
/*****************************************************************************/

#include <config.h>
#include "pcode.h"
#include "cast.h"
