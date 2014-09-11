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
/* 9/30/02 Robert Kidd
 * This file defines a type and macros to work on a 64 bit bitvector.
 * This replaces Intel's kapi/bv64 stuff.
 */

#ifndef _LTAHOE_BITVEC_H
#define _LTAHOE_BITVEC_H

/* 10/29/02 REK Adding config.h */
#include <config.h>
#include <library/i_types.h>

typedef ITuint64 LT_bit_vector;

#define LT_ALL_ONES  0xffffffffffffffff

/* Sets the o low order bits to 1. */
#define LT_ONES_SET_LO(o)    (LT_ALL_ONES >> (64-(o)))

/* Sets the o high order bits to 1. */
#define LT_ONES_SET_HI(o)    (LT_ALL_ONES << (64-(o)))

/* Sets the o low order bits to 0 (effectively the same as ONES_SET_HI). */
#define LT_ZEROS_SET_LO(o)   (LT_ALL_ONES << (o))

/* Sets the o high order bits to 0 (effectively the same as ONES_SET_LO). */
#define LT_ZEROS_SET_HI(o)   (LT_ALL_ONES >> (o))

/* Returns bits p through p+l shifted so that bit p becomes bit 0. */
#define LT_EXTRACT(v, p, l)  (((v) >> (p)) & LT_ONES_SET_LO (l))

/* Sets a single bit in the vector. */
#define LT_SET_BIT(b)        (1 << (b))

#endif
