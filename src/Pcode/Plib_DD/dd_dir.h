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

#ifndef DD_DIR_H
#define DD_DIR_H

#include <config.h>
#include <Pcode/impact_global.h>

typedef uint32 dddirection;

/* unknown distance, value used in dddist field of ddnode */
#define ddunknown	(uint32)0x80000000

/* directions: */
#define ddlt		(dddirection)0x1
#define ddeq		(dddirection)0x2
#define ddgt		(dddirection)0x4
#define ddall		(dddirection)0x7
#define ddrr		(dddirection)0x8	/* not used */
#define ddne		(dddirection)0xd
#define ddanydir	(dddirection)0xf	/* not used */
#define ddallnone	(dddirection)0
#define ddcovers        (dddirection)0x80000000	/* not used */
#define ddkilled        (dddirection)0x40000000	/* not used */
#define ddrefined       (dddirection)0x20000000	/* not used */
#define ddisCovered     (dddirection)0x10000000	/* not used */
#define ddAbsorbed      (dddirection)0x08000000
#define ddterminates    (dddirection)0x04000000	/* not used */
#define ddisTerminated  (dddirection)0x02000000	/* not used */
#define dddirBits       (dddirection)0x00777777

/* test to see if a dd has been killed by a kill or cover */
#define ddisDead(d)             ((d) & (ddkilled | ddisCovered | ddAbsorbed | ddisTerminated))

/* shift a direction 'd' to appropriate position for nest 'n' */
#define dddirnest(d,n)		((d)<<(((n)-1)*4))

/* test if direction vector 'dv' has direction 'd' set at nest 'n' */
#define dddirtest(dv,d,n)	((dv)&dddirnest(d,n))

/* return direction vector except for direction n */
#define ddallBut(dv,n)		((dv)&(dddirBits & ~dddirnest(ddall,n)))

/* set direction 'd' at nest 'dv' for nest 'n' */
#define dddirset(dv,d,n)	(dv|=dddirnest(d,n))

/* reset all directions at nest 'n' in 'dv' except for 'd' */
#define dddironly(dv,d,n)	(dv=(((dv)&~dddirnest(ddanydir,n))|((dv)&dddirnest(d,n))))

/* reset all directions at nest 'n' in 'dv', then set 'd' */
#define dddirsetonly(dv,d,n)	(dv=(((dv)&~dddirnest(ddanydir,n))|(dddirnest(d,n))))

/* reset direction 'd' at nest 'n' in 'dv' */
#define dddirreset(dv,d,n)	(dv&=(~dddirnest(d,n)))

/* reset all directions at nest 'n' in 'dv' */
#define dddirresetalldir(dv,n)	(dv&=(~dddirnest(ddanydir,n)))

/* extract direction vector element at nest 'n' from 'dv' */
#define ddextract1(dv,n)	(((dv)>>(((n)-1)*4))&0xF)

/* test direction 'd' in extracted direction vector element 'dv' */
#define ddtest1(dv,d)		((dv)&(d))

/* reset direction 'd' in extracted direction vector element 'dv' */
#define ddreset1(dv,d)		(dv&=(~(d)))

/* set direction 'd' in extracted direction vector element 'dv' */
#define ddset1(dv,d)		(dv|=(d))

/* filter all direction vector elements with direction 'd' set in 'dv' */
#define ddfilter(dv,d)		(((dv)&((d)|(d<<4)|(d<<8)|(d<<12)|(d<<16)|(d<<20)))/d)
/* #define ddfilter(dv,d)		(((dv)&((d)|(d<<4)|(d<<8)|(d<<12)|(d<<16)|(d<<20)|(d<<24)))/d) */

/* set all filtered direction vector elements to direction 'd' */
#define ddsetfilter(dv,f,d)	(dv|=((f)*(d)))

#endif
