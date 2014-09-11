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
 *      File: l_mii.h
 *      Description: external declarations for MII routines
 *      Creation Date: October, 1994
 *      Author: Daniel Lavery
 *
 *      Revision Date: Summer 1999
 *      Authors: IMPACT Technologies: Matthew Merten and John Gyllenhaal
\*****************************************************************************/

#ifndef L_MII_H
#define L_MII_H

/* 10/29/02 REK Adding config.h */
#include <config.h>
#include <Lcode/sm.h>

typedef struct _Softpipe_MinII
{
  int res_MinII;		/* Resource constrained minimum II */
  int rec_MinII;		/* Recurrence constrained minimum II */
  int MinII;			/* Min II, which is max of 
				   resource and recurrence */
  float eff_MinII;		/* Effective MinII for unrolled loops */
}
Softpipe_MinII;

#define NEGATIVE_INFINITY -32768	/* initialization value for 
					   MinDist array */
#define MAX_RESOURCES_PER_OPTION 256

extern Softpipe_MinII *Lpipe_determine_mii (SM_Cb *, int);
extern int Lpipe_build_MinDist (SM_Cb *, int);
extern int Lpipe_get_MinDist (int, int);
extern int Lpipe_get_DepHeight();
extern void Lpipe_free_MinDist();
extern void Lpipe_free_op_array();

#endif
