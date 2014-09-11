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
 *
 *      File :          region.h
 *      Description :   Region structure definitions
 *      Creation Date : January, 1994
 *      Author :        Richard Hank, Wen-mei Hwu
 *
 *
 *===========================================================================*/
/* 10/29/02 REK Adding config.h */
#include <config.h>
#include <Lcode/l_main.h>
#include "global_cfg.h"

#ifndef	REGION_H
#define REGION_H

typedef struct region_member
{
  union
  {
    Node *node;
    L_Cb *cb;
  }
  member;
  struct region_member *nextMember;
}
RegionMember;

#define _RegionMemberNode(a)	(a)->member.node
#define _RegionMemberCb(a)	(a)->member.cb

typedef struct region_alloc
{
  unsigned short vregId;
  unsigned char rclass;
  unsigned char flags;
  short baseIndex;
  short physReg;
  short spillLocation;
  struct region_alloc *nextAlloc;
}
RegionAlloc;

#define REGION_ALLOCATED_FIRST	0x01
#define REGION_SPILLED_FIRST	0x02

#define _RegionAllocVregId(a)		(a)->vregId
#define _RegionAllocBaseIndex(a)	(a)->baseIndex
#define _RegionAllocPhysReg(a)		(a)->physReg
#define _RegionAllocClass(a)		(a)->rclass
#define _RegionAllocFlags(a)		(a)->flags
#define _RegionAllocSpillLocation(a)	(a)->spillLocation
#define _RegionAllocNext(a)		(a)->nextAlloc

typedef struct region
{
  int id;
  int classId;
  int classId1;
  char *rclass;
  unsigned char flags;
  double weight;
  Set members;
  Node *seed;
  RegionMember *memberList;
  RegionMember *enterList;
  RegionMember *exitList;

  /* Allocation result of flyby virtual registers */
  RegionAlloc *allocList;
  HashTable regionAllocHashTbl;

  struct region *nextRegion;
}
Region;

#define REGION_ALLOCATED	0x1

#define _RegionFlags(a)		(a)->flags

#define inRegionCb(region,cb)	(Set_in((region)->members,(cb)->id))

#if 0
enum
{
  SINGLE_BLOCK,			/* region containing a single block */
  SUPERBLOCK,			/* region containing a path with no
                                   side entrances */
  TRACE,			/* region containing a path with side
                                   entrances */
  HYPERBLOCK,			/* natural hyperblocks */
  HYPERBLOCK_TRACE,

  SINGLE_BLOCK_LOOP,		/* region containing a single block loop */
  SUPERBLOCK_LOOP,		/* region containing a superblock loop */
  TRACE_LOOP,			/* region containing a trace loop */
  HYPERBLOCK_LOOP,		/* region containing a hyperblock loop */
  HYPERBLOCK_TRACE_LOOP,

  MULTIPLE_ENTRY,

  UNCLASSIFIED,			/* region contents are unknown at this time */
  ZERO_WEIGHT,			/* region blocks are never executed */
  FUNCTION_COUNT,		/* number of functions */

  REGION_CLASSES		/* number of region classes */
};
#endif

enum
{
  ACYCLIC,
  CYCLIC,
  INSTRS_FUNCTION,
  BLOCKS_FUNCTION,
  INSTRS_REGION,
  BLOCKS_REGION,
  SINGLE_BLOCK,
  SINGLE_PATH,
  MULTIPLE_PATH,
  SINGLE_BLOCK_LOOP,
  SINGLE_PATH_LOOP,
  MULTIPLE_PATH_LOOP,

  MULTIPLE_ENTRY,

  UNCLASSIFIED,			/* region contents are unknown at this time */
  ZERO_WEIGHT,			/* region blocks are never executed */
  FUNCTION_COUNT,		/* number of functions */

  REGION_CLASSES
};

enum
{
  SINGLE_BLOCK_NO_EPI,
  SINGLE_PATH_NO_EPI,
  MULTIPLE_PATH_NO_EPI,
  SINGLE_PATH_EPI,
  MULTIPLE_PATH_EPI,
  NON_PROLOGUE,
  PROLOGUE_REGION_CLASSES
};

#ifdef __cplusplus
extern "C"
{
#endif

  extern L_Alloc_Pool *Region_pool;
  extern L_Alloc_Pool *RegionMember_pool;
  extern L_Alloc_Pool *RegionAlloc_pool;

  extern HashTable regionNodeHashTbl;
  extern char *RegionClass[];
  extern char *PrologueRegionClass[];

  extern int RegionStatistics[];
  extern FILE *RegionStatFile;

  extern Set loopBackEdges;
  extern Set loopHeaders;

  extern char *regionInputFormat;
  extern char *regionCFGFormat;
  extern int regionOutputCFG;
  extern int regionSelectRegions;
  extern int regionSelectDebug;
  extern int printRegions;
  extern int printRegionJSR;
  extern int printStatistics;

  extern int writeRegions;

/*
 * Function Prototypes
 */
  extern Region *newRegion (void);
  extern void freeRegion (Region * region);
  extern int inRegion (Region * region, Node * node);
  extern void AddMember (Region * region, Node * node);
  extern L_Loop *loopFromHeader (Func * func, Node * node);
  extern L_Loop *loopFromBackEdge (Func * func, Node * node);
  extern Node *bestSuccessor (Node * node, Node * seed, Set limit);
  extern Node *bestPredecessor (Node * node, Node * seed);
  extern Region *regionFormation (Func * funcList);
  extern double RegionWeight (Region * region);
  extern void regionSelection (Func * funcList);
  extern int isRegionEntry (Region * region, Node * node);
  extern L_Loop *regionLoop (Region * region);
  extern int regionSuperblock (Region * region, L_Loop * loop);
  extern int regionTrace (Region * region, L_Loop * loop);
  extern int regionHyperblock (Region * region, L_Loop * loop);
  extern int regionHyperblockTrace (Region * region, L_Loop * loop);
  extern int regionMultipleEntry (Region * region, L_Loop * loop);
  extern int classifyRegion (Region * region);
  extern void PrintRegion (Region * region);
  extern void initRegionStatistics (void);
  extern void printRegionStatistics (void);
  extern int breakup (char *buf, char **fields);
  extern void readRegionStatistics (void);
  extern void writeRegionStatistics (void);
  extern void calcRegionStats (Region * regions);

  extern void R_disassemble_regions (L_Func * fn);

#ifdef __cplusplus
}
#endif

#endif
