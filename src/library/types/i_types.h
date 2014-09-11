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
 *      File :          i_types.h
 *      Description :   IMPACT base types
 *      Creation Date : July 2000
 *      Author :        John W. Sias, Wen-mei Hwu
 *
 *==========================================================================*/
#ifndef I_TYPES_H
#define I_TYPES_H

/* 10/29/02 REK Adding config.h */
#include <config.h>

#define ITMAXU32 0xFFFFFFFFlu
#define ITMAXS32 0x7FFFFFFFl
#define ITMINS32 0x80000000l

/* Flag IT64BIT controls whether or not IMPACT is on a 64-bit host 
 * #define IT64BIT
 *
 * This should be set by the build system
 */

typedef char ITint8;
typedef unsigned char ITuint8;
typedef short ITint16;
typedef unsigned short ITuint16;
typedef int ITint32;
typedef unsigned int ITuint32;

#ifdef IT64BIT

/* 64-bit foundation types
 * ----------------------------------------------------------------------
 */

# ifdef _WIN32
typedef _int64 ITint64;
typedef unsigned _int64 ITuint64;
#   define LLCONST(a) (a ## I64)
#   define ULLCONST(a) (a ## uI64)
#   define ITintmaxformat      "%I64d"
#   define ITintmaxhexfmt      "%I64x"
#   define ITintmaxoctfmt      "%I64o"
#   define ITuintmaxformat     "%I64u"
#   define ITintmaxsuffix      "I64"
#   define ITuintmaxsuffix     "uI64"
#   define ITintmaxcast        "(_int64)"
#   define ITuintmaxcast       "(unsigned _int64)"
# else
typedef long long int ITint64;
typedef long long unsigned int ITuint64;
#   define LLCONST(a) (a ## ll)
#   define ULLCONST(a) (a ## ull)
#   define ITintmaxformat      "%lld"
#   define ITintmaxhexfmt      "%llx"
#   define ITintmaxoctfmt      "%llo"
#   define ITuintmaxformat     "%llu"
#   define ITintmaxsuffix      "LL"
#   define ITuintmaxsuffix     "ULL"
#   define ITintmaxcast        "(long long int)"
#   define ITuintmaxcast       "(unsigned long long int)"
# endif

# define ITMAXU64 ULLCONST(0xFFFFFFFFFFFFFFFF)
# define ITMAXS64  LLCONST(0x7FFFFFFFFFFFFFFF)
# define ITMINS64  LLCONST(0x8000000000000000)

# define ITINTMAX  ITMAXS64
# define ITINTMIN  ITMINS64
# define ITUINTMAX ITMAXU64
# define ITMAXBITS 64

typedef ITint64 ITintmax;
typedef ITuint64 ITuintmax;

#else

/* 32-bit foundation types
 * ----------------------------------------------------------------------
 */

typedef ITint32 ITintmax;
typedef ITuint32 ITuintmax;
# define ITintmaxformat      "%d"
# define ITintmaxhexfmt      "%x"
# define ITintmaxoctfmt      "%o"
# define ITuintmaxformat     "%u"
# define ITintmaxsuffix      "L"
# define ITuintmaxsuffix     "UL"

# define ITINTMAX  ITMAXS32
# define ITINTMIN  ITMINS32
# define ITUINTMAX ITMAXU32
# define ITMAXBITS 32
# define ITintmaxcast        "(int)"
# define ITuintmaxcast       "(unsigned int)"
#endif

extern int IT_typecheck (void);
extern int IT_punt (char *, ...);

/* Safe casting macros */

#ifdef IT64BIT

#define ITicast(v) (((v)==(ITintmax)(ITint32)(v)) ? \
                    (ITint32)(v) : \
                    IT_punt(__FILE__ ": %d with value %llx", __LINE__, v))

#define ITuicast(v) (((v)<=(ITuintmax)(ITMAXU32)) ? \
                    (ITint32)(v) : IT_punt(__FILE__ ": %d", __LINE__))
#else

#define ITicast(v) (v)

#define ITuicast(v) (v)
#endif

#endif

/* Macro for library calls */

#ifdef IT64BIT

#define ITabs(v)  (((v) < 0) ? \
                  (-(v)) : (v))

#else

#define ITabs(v)  abs(v)

#endif
