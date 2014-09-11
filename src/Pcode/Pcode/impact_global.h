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
 *	File:	impact_global.h
 *	Author:	Po-hua Chang, Grant Haab and Wen-mei Hwu
 *	Creation Date:	1988
 * 	Copyright (c) 1991 Po-hua Chang, Grant Haab, Wen-mei Hwu
 *			and The Board of Trustees of the University of Illinois.
 *			All rights reserved.
 *	The University of Illinois software License Agreement
 * 	specifies the terms and conditions for redistribution.
\*****************************************************************************/

#ifndef _PCODE_IMPACT_GLOBAL_H_
#define _PCODE_IMPACT_GLOBAL_H_

#include <config.h>
#include <stdio.h>
#include <assert.h>
#include <string.h>
#include <ctype.h>
#include <values.h>
#include <sys/types.h>
#include <stdarg.h>
#include <stdlib.h>
#include <library/i_types.h>

typedef ITint8 sint8;
typedef ITint16 sint16;
typedef ITint32 sint;
typedef ITint32 sint32;
typedef ITuint8 uint8;
typedef ITuint16 uint16;
typedef ITuint32 uint32;

#ifdef _WIN32
typedef int bool;
#else
typedef unsigned char bool;
#endif
#ifndef uint
#define	uint			unsigned int
#endif
#ifndef NIL
#define NIL 0L
#endif
#ifndef TRUE
#define TRUE 1
#endif
#ifndef FALSE
#define FALSE 0
#endif
#endif
