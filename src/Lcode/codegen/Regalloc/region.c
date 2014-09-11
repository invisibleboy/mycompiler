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
/*===========================================================================*\
 *
 *  File:  region.c
 *
 *  Description:
 *
 *
 *  Creation Date :  June, 1994.
 *
 *  Author:  Richard E. Hank, Wen-mei Hwu
 *
 *
 *===========================================================================*/
/*===========================================================================*/
/* 10/29/02 REK Adding config.h */
#include <config.h>
#include <library/heap.h>
#include <library/stack.h>
#include "global_cfg.h"
#include "region.h"

L_Alloc_Pool *Region_pool = NULL;
L_Alloc_Pool *RegionMember_pool = NULL;
L_Alloc_Pool *RegionAlloc_pool = NULL;
HashTable regionNodeHashTbl;	/* Find a region from a node */

void
initRegion (void)
{
  if (Region_pool == NULL)
    Region_pool = L_create_alloc_pool ("Region", sizeof (Region), 32);
  if (RegionMember_pool == NULL)
    RegionMember_pool = L_create_alloc_pool ("RegionMember",
					     sizeof (RegionMember), 256);
  if (RegionAlloc_pool == NULL)
    RegionAlloc_pool =
      L_create_alloc_pool ("RegionAlloc", sizeof (RegionAlloc), 128);

  if (regionNodeHashTbl == NULL)
    regionNodeHashTbl = HashTable_create (512);
}

Region *
newRegion (void)
{
  Region *region = (Region *) L_alloc (Region_pool);

  region->id = -1;
  region->flags = 0;
  region->rclass = NULL;
  region->seed = NULL;
  region->memberList = NULL;
  region->enterList = NULL;
  region->exitList = NULL;
  region->members = NULL;
  region->nextRegion = NULL;
  region->weight = 0.0;

  region->allocList = NULL;
  region->regionAllocHashTbl = HashTable_create (128);

  return (region);
}

void
freeRegion (Region * region)
{
  RegionMember *mbr, *next;

  for (mbr = region->memberList; mbr != NULL; mbr = next)
    {
      next = mbr->nextMember;

      L_free (RegionMember_pool, mbr);
    }
  Set_dispose (region->members);

  L_free (Region_pool, region);
}

int
inRegion (Region * region, Node * node)
{
  return (Set_in (region->members, _NodeCb (node)));
}

void
AddMember (Region * region, Node * node)
{
  RegionMember *new_rm = (RegionMember *) L_alloc (RegionMember_pool);

  /* Add cb to region member linked list */
  _RegionMemberNode (new_rm) = node;
  if (region->memberList != NULL)
    new_rm->nextMember = region->memberList;
  else
    new_rm->nextMember = NULL;

  region->memberList = new_rm;

  /* Update search tables */
  HashTable_insert (regionNodeHashTbl, _NodeId (node), region);

  /* Add cb to region member set */
  region->members = Set_add (region->members, _NodeCb (node));
}

void
addRegionMemberCb (Region * region, L_Cb * cb)
{
  RegionMember *new_rm = (RegionMember *) L_alloc (RegionMember_pool);

  /* Add cb to region member linked list */
  new_rm->member.cb = cb;
  if (region->memberList != NULL)
    new_rm->nextMember = region->memberList;
  else
    new_rm->nextMember = NULL;

  region->memberList = new_rm;

  /* Update search tables */
  HashTable_insert (regionNodeHashTbl, cb->id, region);

  /* Add cb to region member set */
  region->members = Set_add (region->members, cb->id);
}

void
printRegionCbs (Region * region)
{
  RegionMember *member;

  fprintf (stdout, "********\n*Region %d:\n*\n", region->id);
  fprintf (stdout, "*\tWeight: %f\n", region->weight);
  fprintf (stdout, "*\tBlocks:\n");

  for (member = region->memberList; member != NULL;
       member = member->nextMember)
    {
      fprintf (stdout, "*\t\t%d\n", _RegionMemberCb (member)->id);
    }
  fprintf (stdout, "*\n********\n");
}
