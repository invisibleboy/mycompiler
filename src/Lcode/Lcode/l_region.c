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
 *      File:  l_region.c
 *      Author: Richard E. Hank, Wen-mei Hwu
 *      Creation Date:  April 1995
 *
\*****************************************************************************/

/* 10/29/02 REK Adding config.h */
#include <config.h>
#include <library/set.h>
#include <Lcode/l_main.h>
#include <Lcode/l_region.h>

int L_print_all_regions = 0;
L_Region *L_last_printed_region = NULL;

L_Region_Hash_Entry **L_global_region_hash_tbl = NULL;

L_Region_Boundary *
L_new_region_boundary (L_Cb * ecb, L_Cb * bcb)
{
  L_Region_Boundary *bndry =
    (L_Region_Boundary *) L_alloc (L_alloc_region_boundary);

  bndry->ecb = ecb;
  bndry->bcb = bcb;
  bndry->live_in = NULL;
  bndry->live_out = NULL;
  bndry->next_boundary = NULL;

  return (bndry);
}

void
L_delete_region_boundary (L_Region_Boundary * bndry)
{
  bndry->ecb = NULL;
  bndry->bcb = NULL;

  if (bndry->live_in)
    bndry->live_in = Set_dispose (bndry->live_in);
  if (bndry->live_out)
    bndry->live_out = Set_dispose (bndry->live_out);

  bndry->next_boundary = NULL;

  L_free (L_alloc_region_boundary, bndry);
}

L_Region_Member *
L_new_region_member (L_Cb * cb)
{
  L_Region_Member *member =
    (L_Region_Member *) L_alloc (L_alloc_region_member);

  /* 
   * Region member initialization
   * Note:  The "struct Node *" field is not 
   *        initialized by Lcode.
   */
  member->cb = cb;
  member->next_member = NULL;

  return (member);
}

void
L_delete_region_member (L_Region_Member * mbr)
{
  mbr->cb = NULL;
  mbr->next_member = NULL;

  L_free (L_alloc_region_member, mbr);
}

void
L_add_cb_to_region (L_Region * region, L_Cb * cb)
{
  Set member_set;
  L_Region_Member *member;
  L_Attr *attr;

  if (cb != NULL)
    {
      member_set = region->member_set;
      if (!Set_in (member_set, cb->id))
        {

          region->member_set = Set_add (member_set, cb->id);

          member = L_new_region_member (cb);
          member->next_member = region->member_cbs;
          region->member_cbs = member;

          if ((attr = L_find_attr (cb->attr, "region")) != NULL)
            {
              if ((int) attr->field[0]->value.i != region->id)
                {
                  L_punt ("L_add_cb_to_region: "
                          "Can't add cb %d to region %d it belongs to region "
                          "%d\n", cb->id, region->id,
                          (int) attr->field[0]->value.i);
                }
            }
          else
            {
              attr = L_new_attr ("region", 1);
              L_set_int_attr_field (attr, 0, region->id);
              cb->attr = L_concat_attr (cb->attr, attr);
            }

          cb->region = region;
        }
    }
}

void
L_remove_cb_from_region (L_Region * region, L_Cb * cb)
{
  Set member_set;
  L_Attr *attr;
  L_Region_Member *member, *prev;

  if (cb != NULL)
    {
      member_set = region->member_set;
      if (Set_in (member_set, cb->id))
        {

          region->member_set = Set_delete (member_set, cb->id);

          prev = NULL;
          member = region->member_cbs;
          while (member->cb != cb)
            {
              prev = member;
              member = member->next_member;
            }
          if (member)
            {
              if (prev)
                prev->next_member = member->next_member;
              else
                region->member_cbs = member->next_member;
              L_delete_region_member (member);
            }

          if ((attr = L_find_attr (cb->attr, "region")) != NULL)
            {
              cb->attr = L_delete_attr (cb->attr, attr);
            }

          cb->region = NULL;
        }
    }
}

int
L_cb_in_region (L_Region * region, L_Cb * cb)
{
  if (cb->region != NULL && cb->region == region)
    return (1);
  return (0);
  /*
     return(Set_in(region->member_set,cb->id));
   */
}

L_Region_Boundary *
L_add_entry_cb_to_region (L_Region * region, L_Cb * ecb, L_Cb * bcb)
{
  L_Region_Boundary *prev = NULL;
  L_Region_Boundary *bndry = region->entry_cbs;

  /* Make sure this entry and boundary cb doesn't already exist */
  for (; bndry != NULL; bndry = bndry->next_boundary)
    {

      prev = bndry;

      if (bndry->ecb == ecb && bndry->bcb == bcb)
        return (bndry);
    }

  /* create the new boundary cb and add it to the entry_cb list */
  bndry = L_new_region_boundary (ecb, bcb);

  if (prev == NULL)
    region->entry_cbs = bndry;
  else
    prev->next_boundary = bndry;
  bndry->next_boundary = NULL;
/*
    bndry->next_boundary = region->entry_cbs;
    region->entry_cbs = bndry;
    */

  return (bndry);
}

void
L_remove_region_entry_cb (L_Region * region, L_Cb * ecb, L_Cb * bcb, int punt)
{
  L_Region_Boundary *bndry, *prev;

  prev = NULL;
  for (bndry = region->entry_cbs; bndry != NULL; bndry = bndry->next_boundary)
    {

      /* If we find it, remove it. */
      if (bndry->ecb == ecb && bndry->bcb == bcb)
        {
          if (prev == NULL)
            region->entry_cbs = bndry->next_boundary;
          else
            prev->next_boundary = bndry->next_boundary;
          L_delete_region_boundary (bndry);
          return;
        }

      if (prev == NULL)
        prev = region->entry_cbs;
      else
        prev = prev->next_boundary;
    }
  if (punt)
    {
      /*
         * Try to determine what the problem is!
       */
      if (!L_cb_in_region (region, ecb))
        L_punt
          ("L_remove_region_entry_cb: Cb %d is not a member of region %d\n",
           ecb->id, region->id);
      else
        {
          for (bndry = region->entry_cbs;
               bndry != NULL; bndry = bndry->next_boundary)
            {
              fprintf (stderr, "\tEntrance pair: %d -> %d\n",
                       ((bndry->bcb) ? bndry->bcb->id : -1), bndry->ecb->id);
            }
          L_punt ("L_remove_region_entry_cb: "
                  "Invalid entrance boundary pair %d -> %d for region %d.\n",
                  bcb->id, ecb->id, region->id);
        }
    }
}

int
L_is_region_entry_transition (L_Region * region, L_Cb * ecb, L_Cb * bcb)
{
  L_Region_Boundary *bndry = region->entry_cbs;

  /* Make sure that the transition old_ecb -> bcb actually exists */
  for (; bndry != NULL; bndry = bndry->next_boundary)
    {
      if (bndry->ecb == ecb && bndry->bcb == bcb)
        {
          return (1);
        }
    }
  return (0);
}

void
L_change_region_entry_cb (L_Region * region, L_Cb * old_ecb,
                          L_Cb * new_ecb, L_Cb * bcb)
{
  L_Region_Boundary *bndry = region->entry_cbs;
#if 0
  fprintf (stderr, "Change (%d) ENTRY CB %d -> %d  : %d -> %d\n",
           region->id, bcb->id, ((old_ecb) ? old_ecb->id : -1),
           bcb->id, ((new_ecb) ? new_ecb->id : -1));
#endif
  /* Make sure that the transition old_ecb -> bcb actually exists */
  for (; bndry != NULL; bndry = bndry->next_boundary)
    {
      if (bndry->ecb == old_ecb && bndry->bcb == bcb)
        {
          /* found it so we can change it. */
          bndry->ecb = new_ecb;
          return;
        }
    }
  L_punt ("L_change_region_entry_cb: Cb %d does not branch to entry cb %d\n",
          bcb->id, old_ecb->id);
}


void
L_change_region_entry_boundary_cb (L_Region * region, L_Cb * ecb,
                                   L_Cb * old_bcb, L_Cb * new_bcb)
{
  L_Region_Boundary *bndry = region->entry_cbs;
#if 0
  fprintf (stderr, "Change (%d) ENTRY Boundary CB %d -> %d  : %d -> %d\n",
           region->id,
           old_bcb->id, ((ecb) ? ecb->id : -1),
           new_bcb->id, ((ecb) ? ecb->id : -1));
#endif
  /* Make sure that this entry cb truly branches to the old_bcb */
  for (; bndry != NULL; bndry = bndry->next_boundary)
    {
      if (bndry->ecb == ecb && bndry->bcb == old_bcb)
        {
          /* found it so we can change it. */
          bndry->bcb = new_bcb;
          return;
        }
    }
  /*
   * Error diagnosis
   */
  for (bndry = region->entry_cbs; bndry != NULL; bndry = bndry->next_boundary)
    {
      if (bndry->ecb == ecb)
        fprintf (stderr, "Entry cb %d <-  cb %d\n",
                 bndry->ecb->id, bndry->bcb->id);
    }
  L_punt
    ("L_change_region_entry_boundary_cb: Cb %d does not branch to cb %d of\
region\n", old_bcb->id, ecb->id, region->id);
}



void
L_delete_all_entry_cbs (L_Region * region)
{
  L_Region_Boundary *bndry, *next;

  for (bndry = region->entry_cbs; bndry != NULL; bndry = next)
    {
      next = bndry->next_boundary;

      L_delete_region_boundary (bndry);
    }
  region->entry_cbs = NULL;
}

int
L_num_region_entry_cbs (L_Region * region)
{
  int n_entry;
  L_Region_Boundary *bndry;
  Set entry = NULL;

  for (bndry = region->entry_cbs; bndry != NULL; bndry = bndry->next_boundary)
    {
      entry = Set_add (entry, bndry->ecb->id);
    }
  n_entry = Set_size (entry);

  Set_dispose (entry);
  return (n_entry);
}

int
L_is_region_entry (L_Region * region, L_Cb * cb)
{
  L_Region_Boundary *bndry = region->entry_cbs;

  for (; bndry != NULL; bndry = bndry->next_boundary)
    {
      if (bndry->ecb == cb)
        return (1);
    }
  return (0);
}

Set
L_get_region_member_set (L_Region * region)
{
  return (region->member_set);
}

int
L_region_empty (L_Region * region)
{
  return (Set_empty (region->member_set));
}

Set
L_get_region_entry_IN_set (L_Region * region, L_Cb * bcb, L_Cb * ecb)
{
  L_Region_Boundary *bndry = region->entry_cbs;

  for (; bndry != NULL; bndry = bndry->next_boundary)
    {
      if (bndry->bcb == bcb && bndry->ecb == ecb)
        {
          return (bndry->live_in);
        }
    }
  for (bndry = region->entry_cbs; bndry != NULL; bndry = bndry->next_boundary)
    {
      fprintf (stderr, "\tEntry pair: %d -> %d\n",
               ((bndry->bcb) ? bndry->bcb->id : -1), bndry->ecb->id);
    }
  L_punt ("L_get_region_entry_IN_set: "
          "Cb %d -> %d is not a boundary cb of region %d\n",
          bcb->id, ecb->id, region->id);
  return (Set_INVALID);
}

L_Region_Boundary *
L_add_exit_cb_to_region (L_Region * region, L_Cb * ecb, L_Cb * bcb)
{
  L_Region_Boundary *prev = NULL;
  L_Region_Boundary *bndry = region->exit_cbs;

  /* Make sure this exit and boundary cb doesn't already exist */
  for (; bndry != NULL; bndry = bndry->next_boundary)
    {
      prev = bndry;

      if (bndry->ecb == ecb && bndry->bcb == bcb)
        return (bndry);
    }
  /* create the new boundary cb and add it to the exit_cb list */
  bndry = L_new_region_boundary (ecb, bcb);

  if (prev == NULL)
    region->exit_cbs = bndry;
  else
    prev->next_boundary = bndry;
  bndry->next_boundary = NULL;

  return (bndry);
}

void
L_change_region_exit_boundary_cb (L_Region * region, L_Cb * ecb,
                                  L_Cb * old_bcb, L_Cb * new_bcb)
{
  L_Region_Boundary *bndry = region->exit_cbs;
#if 0
  fprintf (stderr, "Change (%d) EXIT Boundary CB %d -> %d  : %d -> %d\n",
           region->id, ecb->id, ((old_bcb) ? old_bcb->id : -1),
           ecb->id, ((new_bcb) ? new_bcb->id : -1));
#endif
  /* Make sure that this exit cb truly branches to the old_bcb */
  for (; bndry != NULL; bndry = bndry->next_boundary)
    {
      if (bndry->ecb == ecb && bndry->bcb == old_bcb)
        {
          /* found it so we can change it. */
          bndry->bcb = new_bcb;
          return;
        }
    }
  /*
   * Error diagnosis
   */
  for (bndry = region->exit_cbs; bndry != NULL; bndry = bndry->next_boundary)
    {
      if (bndry->ecb == ecb)
        fprintf (stderr, "Exit cb %d branches to cb %d\n",
                 bndry->ecb->id, bndry->bcb->id);
    }
  L_punt
    ("L_change_region_exit_boundary_cb: Exit cb %d of region %d does not \
branch to cb %d\n", ecb->id, region->id, old_bcb->id);
}

int
L_is_region_exit_transition (L_Region * region, L_Cb * ecb, L_Cb * bcb)
{
  L_Region_Boundary *bndry = region->exit_cbs;

  /* Make sure that the transition ecb -> bcb actually exists */
  for (; bndry != NULL; bndry = bndry->next_boundary)
    {
      if (bndry->ecb == ecb && bndry->bcb == bcb)
        {
          return (1);
        }
    }
  return (0);
}

void
L_change_region_exit_cb (L_Region * region, L_Cb * old_ecb,
                         L_Cb * new_ecb, L_Cb * bcb)
{
  L_Region_Boundary *bndry = region->exit_cbs;
#if 0
  fprintf (stderr, "Change (%d) EXIT CB %d -> %d  : %d -> %d\n",
           region->id, ((old_ecb) ? old_ecb->id : -1), ((bcb) ? bcb->id : -1),
           ((new_ecb) ? new_ecb->id : -1), ((bcb) ? bcb->id : -1));
#endif
  /* Make sure that the transition old_ecb -> bcb actually exists */
  for (; bndry != NULL; bndry = bndry->next_boundary)
    {
      if (bndry->ecb == old_ecb && bndry->bcb == bcb)
        {
          /* found it so we can change it. */
          bndry->ecb = new_ecb;
          return;
        }
    }
  L_punt ("L_change_region_exit_cb: Exit cb %d does not branch to cb %d\n",
          old_ecb->id, bcb->id);
}

void
L_remove_region_exit_cb (L_Region * region, L_Cb * ecb, L_Cb * bcb)
{
  L_Region_Boundary *bndry, *prev;

  prev = NULL;
  for (bndry = region->exit_cbs; bndry != NULL; bndry = bndry->next_boundary)
    {

      /* If we find it, remove it. */
      if (bndry->ecb == ecb && bndry->bcb == bcb)
        {
          if (prev == NULL)
            region->exit_cbs = bndry->next_boundary;
          else
            prev->next_boundary = bndry->next_boundary;
          L_delete_region_boundary (bndry);
          return;
        }

      if (prev == NULL)
        prev = region->exit_cbs;
      else
        prev = prev->next_boundary;
    }
  /*
   * Try to determine what the problem is!
   */
  if (!L_cb_in_region (region, ecb))
    L_punt ("L_remove_region_exit_cb: Cb %d is not a member of region %d\n",
            ecb->id, region->id);
  else
    {
      for (bndry = region->exit_cbs;
           bndry != NULL; bndry = bndry->next_boundary)
        {
          fprintf (stderr, "\tExit pair: %d -> %d\n",
                   ((bndry->bcb) ? bndry->bcb->id : -1), bndry->ecb->id);
        }
      L_punt
        ("L_remove_region_exit_cb: Invalid entrance boundary pair %d -> %d \
for region %d.\n", bcb->id, ecb->id, region->id);
    }

}

void
L_delete_all_exit_cbs (L_Region * region)
{
  L_Region_Boundary *bndry, *next;

  for (bndry = region->exit_cbs; bndry != NULL; bndry = next)
    {
      next = bndry->next_boundary;

      L_delete_region_boundary (bndry);
    }
  region->exit_cbs = NULL;
}

int
L_num_region_exit_cbs (L_Region * region)
{
  int n_exit;
  L_Region_Boundary *bndry;
  Set exit = NULL;

  for (bndry = region->exit_cbs; bndry != NULL; bndry = bndry->next_boundary)
    {
      exit = Set_add (exit, bndry->ecb->id);
    }
  n_exit = Set_size (exit);

  Set_dispose (exit);
  return (n_exit);
}

int
L_is_region_exit (L_Region * region, L_Cb * cb)
{
  L_Region_Boundary *bndry = region->exit_cbs;

  for (; bndry != NULL; bndry = bndry->next_boundary)
    {
      if (bndry->ecb == cb)
        return (1);
    }
  return (0);
}

Set
L_get_region_exit_OUT_set (L_Region * region, L_Cb * bcb)
{
  L_Region_Boundary *bndry = region->exit_cbs;

  /* Doesn't matter which exit cb is branching to bcb, they */
  /* all have the same live out.                            */
  for (; bndry != NULL; bndry = bndry->next_boundary)
    {
      if (bndry->bcb == bcb)
        return (bndry->live_out);
    }
  return ((Set) - 1);
}

L_Region *
L_new_region (int id)
{
  L_Region *region = (L_Region *) L_alloc (L_alloc_region);

  /* 
   * Region structure initialization
   */
  region->id = id;
  region->flags = 0;
  region->member_cbs = NULL;
  region->member_set = NULL;
  region->entry_cbs = NULL;
  region->exit_cbs = NULL;
  region->regmap = INT_new_symbol_table ("Regmap", 16);
  region->attr = NULL;
  region->prev_region = NULL;
  region->next_region = NULL;
  region->weight = 0.0;

  return (region);
}

void
L_delete_region (L_Region * region)
{
  L_Region_Member *mbr, *next_mbr;
  L_Region_Boundary *bndry, *next_bndry;

  /*
   * Delete the region boundary information
   */
  for (bndry = region->entry_cbs; bndry != NULL; bndry = next_bndry)
    {
      next_bndry = bndry->next_boundary;

      L_delete_region_boundary (bndry);
    }
  for (bndry = region->exit_cbs; bndry != NULL; bndry = next_bndry)
    {
      next_bndry = bndry->next_boundary;

      L_delete_region_boundary (bndry);
    }
  /* 
   * Delete the region member information
   */
  for (mbr = region->member_cbs; mbr != NULL; mbr = next_mbr)
    {
      next_mbr = mbr->next_member;

      L_delete_region_member (mbr);
    }
  region->member_set = Set_dispose (region->member_set);

  /*
   * Delete region register map information
   */
  INT_delete_symbol_table (region->regmap,
                           (void (*)(void *)) L_delete_region_regmap);

  /*
   *  Delete the attributes
   */
  if (region->attr != NULL)
    L_delete_all_attr (region->attr);
  region->attr = NULL;

  region->prev_region = NULL;
  region->next_region = NULL;

  L_free (L_alloc_region, region);
}

L_Region *
L_new_region_in_func (L_Func * fn, int id)
{
  L_Region *region = L_new_region (id);

  L_region_hash_tbl_insert (fn->region_hash_tbl, region);

  return (region);
}

void
L_set_region_seed (L_Region * region, int cb_id)
{
  L_Attr *attr = L_new_attr ("seed", 1);
  L_set_int_attr_field (attr, 0, cb_id);

  region->attr = L_concat_attr (region->attr, attr);
}

int
L_get_region_seed (L_Region * region)
{
  L_Attr *attr = L_find_attr (region->attr, "seed");

  return ((int) attr->field[0]->value.i);
}

void
L_insert_region_after (L_Func * fn, L_Region * after_region,
                       L_Region * region)
{
  region->prev_region = after_region;
  region->next_region =
    ((after_region != NULL) ? after_region->next_region : NULL);

  if (region->prev_region != NULL)
    {
      region->prev_region->next_region = region;
    }
  else
    {
      fn->first_region = region;
    }

  if (region->next_region != NULL)
    {
      region->next_region->prev_region = region;
    }
  else
    {
      fn->last_region = region;
    }
}

void
L_delete_all_region (L_Region * list, L_Region_Hash_Entry ** region_hash_tbl)
{
  L_Region *region, *next;

  for (region = list; region != NULL; region = next)
    {
      next = region->next_region;

      L_delete_region (region);
    }
  /* delete all hash tbl entries */
  L_region_hash_tbl_delete_all (region_hash_tbl);
}

L_Oper *
L_region_boundary_insert_point (L_Cb * cb)
{
  L_Oper *op;

  if (!L_EXTRACT_BIT_VAL (cb->flags, L_CB_BOUNDARY))
    {
      L_punt ("L_region_boundary_insert_point: cb%d is not a boundary!\n",
              cb->id);
    }

  for (op = cb->first_op; op != NULL; op = op->next_op)
    {
      if (op->opc == Lop_BOUNDARY)
        return (op);
    }
  L_punt ("L_region_boundary_insert_point: cb%d contains no boundary op!\n",
          cb->id);
  return (NULL);
}

/*========================================================================*/
/*
 *      REGION Regmap functions
 */
/*========================================================================*/
L_Region_Regmap *
L_new_region_regmap (int vreg_id)
{
  L_Region_Regmap *regmap =
    (L_Region_Regmap *) L_alloc (L_alloc_region_regmap);

  regmap->vreg_id = vreg_id;
  regmap->ctype = -1;
  regmap->flags = 0;
  regmap->occupied = NULL;
  regmap->jsr_weight = 0.0;
  regmap->ref_weight = 0.0;
  regmap->spill_loc = -1;
  regmap->type = -1;
  regmap->phys_reg = -1;
  regmap->constraints = INT_new_symbol_table ("Regcon", 16);

  return (regmap);
}

void
L_delete_region_regmap (L_Region_Regmap * regmap)
{
  Set_dispose (regmap->occupied);

  INT_delete_symbol_table (regmap->constraints,
                           (void (*)(void *)) L_delete_region_regcon);

  L_free (L_alloc_region_regmap, regmap);
}

void
L_add_regmap_to_region (L_Region * region, L_Region_Regmap * regmap)
{
  INT_add_symbol (region->regmap, regmap->vreg_id, regmap);
}

L_Region_Regmap *
L_find_region_regmap (L_Region * region, int vreg_id)
{
  L_Region_Regmap *regmap;

  regmap = (L_Region_Regmap *) INT_find_symbol_data (region->regmap, vreg_id);

  return (regmap);
}

L_Region_Regcon *
L_new_region_regcon (L_Cb * bcb, int flags, int type, int phys_reg)
{
  L_Region_Regcon *regcon =
    (L_Region_Regcon *) L_alloc (L_alloc_region_regcon);

  regcon->boundary_cb = bcb;
  regcon->flags = flags;
  regcon->type = type;
  regcon->phys_reg = phys_reg;

  return (regcon);
}

void
L_delete_region_regcon (L_Region_Regcon * regcon)
{
  L_free (L_alloc_region_regcon, regcon);
}

void
L_add_regcon_to_regmap (L_Region_Regmap * regmap, L_Region_Regcon * regcon)
{
  INT_add_symbol (regmap->constraints, regcon->boundary_cb->id, regcon);
}

L_Region_Regcon *
L_find_regcon_for_cb (L_Region_Regmap * regmap, L_Cb * cb)
{
  L_Region_Regcon *regcon;

  regcon =
    (L_Region_Regcon *) INT_find_symbol_data (regmap->constraints, cb->id);

  return (regcon);
}

/*========================================================================*/
/*
 *      REGION Hash Table Functions
 */
/*========================================================================*/

/*
 *      L_Region_Hash_Entry
 */

static L_Region_Hash_Entry *
L_new_region_hash_entry (int id, L_Region * region)
{
  L_Region_Hash_Entry *entry;

  entry = (L_Region_Hash_Entry *) L_alloc (L_alloc_region_hash_entry);
  entry->id = id;
  entry->region = region;
  entry->prev_region_hash = NULL;
  entry->next_region_hash = NULL;

  return (entry);
}

static L_Region_Hash_Entry *
L_delete_region_hash_entry (L_Region_Hash_Entry * list,
                            L_Region_Hash_Entry * entry)
{
  L_Region_Hash_Entry *first, *prev, *next;

  /* Unlink entry and free it */
  first = list;
  if (entry == first)
    first = entry->next_region_hash;

  prev = entry->prev_region_hash;
  next = entry->prev_region_hash;
  if (prev != NULL)
    prev->next_region_hash = next;
  if (next != NULL)
    next->prev_region_hash = prev;

  entry->prev_region_hash = NULL;
  entry->next_region_hash = NULL;

  L_free (L_alloc_region_hash_entry, entry);

  return (first);
}

static void
L_delete_all_region_hash_entry (L_Region_Hash_Entry * list)
{
  L_Region_Hash_Entry *ptr, *next;

  for (ptr = list; ptr != NULL; ptr = next)
    {
      next = ptr->next_region_hash;
      ptr->prev_region_hash = NULL;
      ptr->next_region_hash = NULL;
      L_free (L_alloc_region_hash_entry, ptr);
    }
}

static L_Region_Hash_Entry *
L_find_region_hash_entry (L_Region_Hash_Entry * list, int id)
{
  L_Region_Hash_Entry *ptr;

  for (ptr = list; ptr != NULL; ptr = ptr->next_region_hash)
    {
      if (ptr->id == id)
        return (ptr);
    }

  return (NULL);
}

L_Region_Hash_Entry **
L_region_hash_tbl_create ()
{
  int i;
  L_Region_Hash_Entry **region_hash_tbl;

  region_hash_tbl =
    (L_Region_Hash_Entry **) L_alloc (L_alloc_region_hash_tbl);

  for (i = 0; i < L_region_hash_tbl_size; i++)
    {
      region_hash_tbl[i] = NULL;
    }

  return (region_hash_tbl);
}

void
L_region_hash_tbl_insert (L_Region_Hash_Entry ** region_hash_tbl,
                          L_Region * region)
{
  int hash_id;
  L_Region_Hash_Entry *entry;

  hash_id = region->id & (L_region_hash_tbl_size - 1);
  entry = L_new_region_hash_entry (region->id, region);
  entry->next_region_hash = region_hash_tbl[hash_id];
  if (region_hash_tbl[hash_id] != NULL)
    region_hash_tbl[hash_id]->prev_region_hash = entry;
  region_hash_tbl[hash_id] = entry;
}

void
L_region_hash_tbl_delete (L_Region_Hash_Entry ** region_hash_tbl,
                          L_Region * region)
{
  int hash_id;
  L_Region_Hash_Entry *entry;

  hash_id = region->id & (L_region_hash_tbl_size - 1);

  /* find the appropriate entry */
  entry = L_find_region_hash_entry (region_hash_tbl[hash_id], region->id);
  if (entry == NULL)
    L_punt ("L_region_hash_tbl_delete: entry for region not found");

  region_hash_tbl[hash_id] =
    L_delete_region_hash_entry (region_hash_tbl[hash_id], entry);
}

void
L_region_hash_tbl_delete_all (L_Region_Hash_Entry ** region_hash_tbl)
{
  int i;

  for (i = 0; i < L_region_hash_tbl_size; i++)
    {
      if (region_hash_tbl[i] != NULL)
        {
          L_delete_all_region_hash_entry (region_hash_tbl[i]);
          region_hash_tbl[i] = NULL;
        }
    }
}

L_Region *
L_region_hash_tbl_find (L_Region_Hash_Entry ** region_hash_tbl, int region_id)
{
  int hash_id;
  L_Region_Hash_Entry *entry;

  hash_id = region_id & (L_region_hash_tbl_size - 1);
  entry = L_find_region_hash_entry (region_hash_tbl[hash_id], region_id);
  if (entry)
    return (entry->region);
  else
    return (NULL);
}

/*============================================================
 *
 *  Region input/output routines
 *
 *============================================================*/
void
L_read_region_flags (L_Region * region, L_Input_Buf * input_buf)
{
  char *flags;
  int i, num_flags;

  /* Assumes the token next token will be the flag string */
  flags = L_get_next_lcode_token (input_buf);
  num_flags = strlen (flags);

  for (i = 0; i < num_flags; i++)
    {
      switch (flags[i])
        {
          /* Rememeber to include lower and upper case! */
        case 'C':
        case 'c':
          region->flags = L_SET_BIT_FLAG (region->flags,
                                          L_REGION_COMPILATION_COMPLETE);
          break;
        case 'E':
        case 'e':
          region->flags = L_SET_BIT_FLAG (region->flags, L_REGION_EPILOGUE);
          break;
        case 'P':
        case 'p':
          region->flags = L_SET_BIT_FLAG (region->flags, L_REGION_PROLOGUE);
          break;
        default:
          L_punt ("L_read_region_flags: invalid flag in region %d of <%c>",
                  region->id, flags[i]);
        }
    }
  L_get_next_lcode_token_expecting (input_buf, ">", "L_read_fn_flags");
}

Set
L_read_region_set (L_Input_Buf * input_buf, int encode)
{
  int ival;
  char ch, *token;
  Set set = NULL;

  /*
     L_get_next_lcode_token_expecting(input_buf,"(","L_read_region_set"); 
   */
  ch = L_peek_next_char (input_buf);

  while (ch != ')')
    {
      token = L_get_next_lcode_token (input_buf);

      if (L_strtol (token, NULL, &ival) == 0)
        {
          /* since we got an invalid integer assume it is a macro name */
          if ((ival = L_macro_id (token)) == -1)
            {
              L_print_buf_with_arrow (stderr, input_buf);
              L_punt ("L_read_region_set: set contains invalid macro");
            }

          if (encode)
            set = Set_add (set, L_MAC_INDEX (ival));
          else
            set = Set_add (set, ival);
        }
      else
        {                       /* add the valid int to the set */

          if (encode)
            set = Set_add (set, L_REG_INDEX (ival));
          else
            set = Set_add (set, ival);
        }

      ch = L_peek_next_char (input_buf);
    }
  L_get_next_lcode_token_expecting (input_buf, ")", "L_read_region_set");

  return (set);
}

void
L_read_region_reg_constraint (L_Func * fn, L_Region_Regmap * regmap,
                              L_Input_Buf * input_buf)
{
  int cb_id, type, phys_reg, flags;
  char ch, *token, *ty;
  L_Cb *cb;
  L_Region_Regcon *regcon;

  L_get_next_lcode_token_expecting (input_buf, "cb", "L_read_region_regmap");

  token = L_get_next_lcode_token (input_buf);
  if (L_strtol (token, NULL, &cb_id) == 0)
    {
      L_print_buf_with_arrow (stderr, input_buf);
      L_punt ("L_read_region_reg_constraint: "
              "The cb id is not a valid integer value.");
    }
  cb = L_cb_hash_tbl_find_and_alloc (fn->cb_hash_tbl, cb_id);

  flags = 0;
  type = 0;
  phys_reg = -1;

  ch = L_peek_next_char (input_buf);
  switch (ch)
    {
    case 'r':                   /* physical register - macro/register */
      ty = L_get_next_lcode_token (input_buf);
      type = L_operand_id (ty);
      token = L_get_next_lcode_token (input_buf);
      L_strtol (token, NULL, &phys_reg);
      flags = L_REGION_VREG_ALLOC;
      break;
    case 'm':
      ty = L_get_next_lcode_token (input_buf);
      type = L_operand_id (ty);
      token = L_get_next_lcode_token (input_buf);
      phys_reg = L_macro_id (token);
      flags = L_REGION_VREG_ALLOC;
      break;
    case 's':                   /* spilled */
      token = L_get_next_lcode_token (input_buf);
      flags = L_REGION_VREG_SPILL;
      break;
    case 'i':
      token = L_get_next_lcode_token (input_buf);
      flags = L_REGION_VREG_IGNORE;
      break;
    }
  L_get_next_lcode_token_expecting (input_buf, ")", "L_read_region_regmap");

  regcon = L_new_region_regcon (cb, flags, type, phys_reg);
  L_add_regcon_to_regmap (regmap, regcon);
}

void
L_read_regmap_flags (L_Region_Regmap * regmap, L_Input_Buf * input_buf)
{
  char *flags;
  int i, num_flags;

  /* Assumes the token next token will be the flag string */
  flags = L_get_next_lcode_token (input_buf);
  num_flags = strlen (flags);

  for (i = 0; i < num_flags; i++)
    {
      switch (flags[i])
        {
          /* Rememeber to include lower and upper case! */
        case 'A':
        case 'a':
          regmap->flags = L_SET_BIT_FLAG (regmap->flags, L_REGION_VREG_ALLOC);
          break;
        case 'S':
        case 's':
          regmap->flags = L_SET_BIT_FLAG (regmap->flags, L_REGION_VREG_SPILL);
          break;
        default:
          L_punt ("L_read_regmap_flags: invalid flag in regmap of <%c>",
                  flags[i]);
        }
    }
  L_get_next_lcode_token_expecting (input_buf, ">", "L_read_regmap_flags");
}

void
L_read_region_regmap (L_Func * fn, L_Region * region, L_Input_Buf * input_buf)
{
  char ch, *token, *ty;
  L_Operand *opd;
  L_Region_Regmap *regmap;

  L_get_next_lcode_token_expecting (input_buf, "regmap",
                                    "L_read_region_regmap");

  ch = L_peek_next_char (input_buf);
  while (ch != '}')
    {
      L_get_next_lcode_token_expecting (input_buf, "[",
                                        "L_read_region_regmap");

      opd = L_read_operand (fn, input_buf);
      regmap = L_new_region_regmap (opd->value.r);
      regmap->ctype = L_return_old_ctype (opd);
      L_delete_operand (opd);

      ch = L_peek_next_char (input_buf);
      if (ch == '<')
        {
          L_get_next_lcode_token (input_buf);   /* Throw away '<' */
          ch = L_peek_next_char (input_buf);

          /* Read in region flags */
          L_read_regmap_flags (regmap, input_buf);

          ch = L_peek_next_char (input_buf);
        }

      ch = L_peek_next_char (input_buf);
      while (ch != ']')
        {
          /* 
           * Get the other information associated with this vreg
           */
          L_get_next_lcode_token_expecting (input_buf, "(",
                                            "L_read_region_regmap");

          ch = L_peek_next_char (input_buf);
          switch (ch)
            {
            case 'c':           /* cb - boundary cb constraint */
              L_read_region_reg_constraint (fn, regmap, input_buf);
              break;
            case 'g':
              L_get_next_lcode_token_expecting (input_buf, "gref",
                                                "L_read_region_regmap");
              token = L_get_next_lcode_token (input_buf);
              L_strtod (token, NULL, &regmap->ref_weight);
              L_get_next_lcode_token_expecting (input_buf, ")",
                                                "L_read_region_regmap");
              break;
            case 'j':           /* jsr - global jsr weight for vreg */
              L_get_next_lcode_token_expecting (input_buf, "jsr",
                                                "L_read_region_regmap");
              token = L_get_next_lcode_token (input_buf);
              L_strtod (token, NULL, &regmap->jsr_weight);
              L_get_next_lcode_token_expecting (input_buf, ")",
                                                "L_read_region_regmap");
              break;
            case 'o':           /* occ - occupied registers */
              L_get_next_lcode_token_expecting (input_buf, "occ",
                                                "L_read_region_regmap");
              regmap->occupied = L_read_region_set (input_buf, 0);
              break;
            case 'r':           /* physical register - macro/register */
              ty = L_get_next_lcode_token (input_buf);
              regmap->type = L_operand_id (ty);
              token = L_get_next_lcode_token (input_buf);
              L_strtol (token, NULL, &regmap->phys_reg);
              L_get_next_lcode_token_expecting (input_buf, ")",
                                                "L_read_region_regmap");
              break;
            case 'm':
              ty = L_get_next_lcode_token (input_buf);
              regmap->type = L_operand_id (ty);
              token = L_get_next_lcode_token (input_buf);
              regmap->phys_reg = L_macro_id (token);
              L_get_next_lcode_token_expecting (input_buf, ")",
                                                "L_read_region_regmap");
              break;
            case 's':           /* st - stack location */
              L_get_next_lcode_token_expecting (input_buf, "st",
                                                "L_read_region_regmap");
              token = L_get_next_lcode_token (input_buf);
              L_strtol (token, NULL, &regmap->spill_loc);
              L_get_next_lcode_token_expecting (input_buf, ")",
                                                "L_read_region_regmap");
              break;
            }
          ch = L_peek_next_char (input_buf);
        }

      L_add_regmap_to_region (region, regmap);

      L_get_next_lcode_token_expecting (input_buf, "]",
                                        "L_read_region_regmap");

      ch = L_peek_next_char (input_buf);
    }
  L_get_next_lcode_token_expecting (input_buf, "}", "L_read_region_regmap");
}

void
L_read_region_boundary_conditions (L_Func * fn, L_Region * region,
                                   L_Input_Buf * input_buf)
{
  int boundary_type, bndry_info;
  char ch, *token;
  L_Cb *ecb, *bcb;
  L_Operand *opd;
  L_Region_Boundary *bndry;

  /*
   * Read boundary cb type 
   */
  token = L_get_next_lcode_token (input_buf);
  boundary_type = L_lcode_id (token);
  if (!(boundary_type == L_INPUT_REGION_ENTRY ||
        boundary_type == L_INPUT_REGION_EXIT))
    {
      L_print_buf_with_arrow (stderr, input_buf);
      L_punt
        ("L_read_region_boundary_conditions: Unknown region boundary item.");
    }

  /*
   * Read entry/exit cb - formatted as an operand: (cb x)
   */
  opd = L_read_operand (fn, input_buf);
  ecb = opd->value.cb;
  L_delete_operand (opd);

  ch = L_peek_next_char (input_buf);
  while (ch != '}')
    {
      L_get_next_lcode_token (input_buf);       /* throw away '[' */

      /* If the boundary condition info is not empty. */
      ch = L_peek_next_char (input_buf);
      if (ch != ']')
        {
          /* 
           * Read boundary cb - formatted as an operand: (cb x)
           */
          opd = L_read_operand (fn, input_buf);
          bcb = opd->value.cb;
          L_delete_operand (opd);
        }
      else
        bcb = NULL;


      if (boundary_type == L_INPUT_REGION_ENTRY)
        bndry = L_add_entry_cb_to_region (region, ecb, bcb);
      else
        bndry = L_add_exit_cb_to_region (region, ecb, bcb);

      while (ch != ']')
        {
          L_get_next_lcode_token (input_buf);   /* throw away '(' */

          token = L_get_next_lcode_token (input_buf);
          bndry_info = L_lcode_id (token);

          switch (bndry_info)
            {
            case L_INPUT_REGION_LIVE_IN:
              bndry->live_in = L_read_region_set (input_buf, 1);
              break;
            case L_INPUT_REGION_LIVE_OUT:
              bndry->live_out = L_read_region_set (input_buf, 1);
              break;
            default:
              L_print_buf_with_arrow (stderr, input_buf);
              L_punt("L_read_region_boundary_conditions: "
                     "Unknown boundary condition info");
            }

          ch = L_peek_next_char (input_buf);
        }

      L_get_next_lcode_token (input_buf);       /* throw away ']' */
      ch = L_peek_next_char (input_buf);
    }
  L_get_next_lcode_token_expecting (input_buf, "}",
                                    "L_read_region_boundary_conditions");
}

L_Region *
L_read_region (L_Func * fn, L_Input_Buf * input_buf)
{
  char ch, *token;
  int id;
  L_Attr *attr;
  L_Region *region;

  /*
   * Read region id
   */
  token = L_get_next_lcode_token (input_buf);
  if (L_strtol (token, NULL, &id) == 0)
    {
      L_print_buf_with_arrow (stderr, input_buf);
      L_punt ("L_read_region:  The region id is not an integer value.");
    }

  if (id < 0)
    {
      L_print_buf_with_arrow (stderr, input_buf);
      L_punt ("L_read_region: "
              "The id of the current region is negative, this is illegal.");
    }

  /* find if already allocated region, else allocate new one */
  if ((region = L_region_hash_tbl_find (fn->region_hash_tbl, id)) == NULL)
    {
      region = L_new_region_in_func (fn, id);
      L_insert_region_after (fn, fn->last_region, region);
    }

  ch = L_peek_next_char (input_buf);
  if (ch == '<')
    {
      L_get_next_lcode_token (input_buf);       /* Throw away '<' */
      ch = L_peek_next_char (input_buf);

      /* Read in region flags */
      L_read_region_flags (region, input_buf);

      ch = L_peek_next_char (input_buf);
    }
  if (ch == '{')
    {
      /* 
       * Read boundary condition information
       */
      while (ch != '<' && ch != ')')
        {
          L_get_next_lcode_token_expecting (input_buf, "{", "L_read_region");

          ch = L_peek_next_char (input_buf);
          switch (ch)
            {
            case 'e':
              L_read_region_boundary_conditions (fn, region, input_buf);
              break;
            case 'r':
              L_read_region_regmap (fn, region, input_buf);
              break;
            }

          ch = L_peek_next_char (input_buf);
        }
    }
  if (ch == '<')
    {
      L_get_next_lcode_token (input_buf);       /* Throw away '<' */
      ch = L_peek_next_char (input_buf);
      while (ch != '>')
        {
          attr = L_read_attr (NULL, input_buf);
          region->attr = L_concat_attr (region->attr, attr);

          ch = L_peek_next_char (input_buf);
        }
      L_get_next_lcode_token (input_buf);       /* Throw away '>' */
    }

  /* Region the terminating token */
  L_get_next_lcode_token_expecting (input_buf, ")", "L_read_region");

  return (region);
}

L_Region *
L_read_region_end (L_Func * fn, L_Input_Buf * input_buf)
{
  char *token;
  int id;

  /*
   * Read region id
   */
  token = L_get_next_lcode_token (input_buf);
  if (L_strtol (token, NULL, &id) == 0)
    {
      L_print_buf_with_arrow (stderr, input_buf);
      L_punt ("L_read_region_end:  The region id is not an integer value.");
    }

  /* Region the terminating token */
  L_get_next_lcode_token_expecting (input_buf, ")", "L_read_region");

  return (NULL);
}

void
L_print_region_set (FILE * F, Set set, int decode)
{
  int n_entry, *entry_buf;
  int i;

  if ((n_entry = Set_size (set)) != 0)
    {
      entry_buf = (int *) malloc (n_entry * sizeof (int));
      Set_2array (set, entry_buf);

      for (i = 0; i < n_entry; i++)
        {
          if (decode)
            {
              if (L_IS_MAPPED_REG (entry_buf[i]))
                fprintf (F, "%d ", L_UNMAP_REG (entry_buf[i]));
              else              /* is a mapped macro */
                fprintf (F, "%s ", L_macro_name (L_UNMAP_MAC (entry_buf[i])));
            }
          else
            {
              fprintf (F, "%d ", entry_buf[i]);
            }
        }
      free (entry_buf);
    }
  fputc (')', F);
}

void
L_print_region_regcon (FILE * F, L_Region_Regcon * regcon)
{
  fputs ("(", F);

  fprintf (F, "cb %d ", regcon->boundary_cb->id);
  if (regcon->flags & L_REGION_VREG_SPILL)
    fputs ("s", F);
  else if (regcon->flags & L_REGION_VREG_IGNORE)
    fputs ("i", F);
  else if (regcon->type == L_OPERAND_REGISTER)
    fprintf (F, "r %d", regcon->phys_reg);
  else if (regcon->type == L_OPERAND_MACRO)
    fprintf (F, "mac %s", L_macro_name (regcon->phys_reg));

  fputs (")", F);
}

void
L_print_region_regmap (FILE * F, L_Region_Regmap * regmap)
{
  int flags;
  INT_Symbol *sym;

  fputs ("\n     [", F);

  fprintf (F, "(r %d %s) ", regmap->vreg_id, L_ctype_name (regmap->ctype));

  /*
   *  Print flags - flags are optional
   *
   *  Remember to clear out any flags that should not print! 
   */
  flags = regmap->flags;
  if (flags != 0)
    {
      fprintf (F, "<");
      if (L_EXTRACT_BIT_VAL (flags, L_REGION_VREG_ALLOC))
        fprintf (F, "A");
      if (L_EXTRACT_BIT_VAL (flags, L_REGION_VREG_SPILL))
        fprintf (F, "S");
      fprintf (F, "> ");
    }

  if (regmap->occupied != NULL && !Set_empty (regmap->occupied))
    {
      fputs ("(occ ", F);
      L_print_region_set (F, regmap->occupied, 0);
    }

  fprintf (F, "(st %d)", regmap->spill_loc);

  if (regmap->type == L_OPERAND_MACRO)
    fprintf (F, "(mac %s)", L_macro_name (regmap->phys_reg));
  else
    fprintf (F, "(r %d)", regmap->phys_reg);

  fprintf (F, "(jsr %g)", regmap->jsr_weight);
  fprintf (F, "(gref %g)", regmap->ref_weight);
  /*
   * Print the constraints on this virtual register.  They are stored
   * in a hash table, so traverse the list of entries.
   */
  for (sym = regmap->constraints->head_symbol;
       sym != NULL; sym = sym->next_symbol)
    L_print_region_regcon (F, (L_Region_Regcon *) sym->data);

  fputs ("]", F);
}

void
L_print_region (FILE * F, L_Func * fn, L_Region * region)
{
  int flags;
  L_Attr *attr;
  L_Region_Boundary *bndry;
  INT_Symbol *sym;

  /* All of the region specific information should be output only the      */
  /* first time the region is encountered.  For all subsequent occurences  */
  /* The region specifier is emitted if necessary.                         */


  if (region == NULL)
    return;

  /* When called from L_print_func, only print the necessary information */
  /* otherwise print everything, each time this function is called.      */
  if (L_print_all_regions == 1)
    {
      /* All of the region specific information should be output only the    */
      /* first time the region is encountered.  For all subsequent           */
      /* occurences the region specifier is emitted if necessary.            */
      if (L_last_printed_region == region)
        return;

      L_last_printed_region = region;

      fprintf (F, " (region %d", region->id);
      if (L_EXTRACT_BIT_VAL (region->flags, L_REGION_PRESENCE))
        {
          /* The region has already been emitted once, so it is sufficient */
          /* to emit the id of the region.                             */
          fprintf (F, ")\n");
          return;
        }
      /* Indicate that the region has been emitted */
      region->flags = L_SET_BIT_FLAG (region->flags, L_REGION_PRESENCE);
    }
  else
    fprintf (F, " (region %d", region->id);

  /*
   *  Print flags - flags are optional
   *
   *  Remember to clear out any flags that should not print!
   */
  flags = region->flags;
  flags = L_CLR_BIT_FLAG (flags, L_REGION_HIDDEN_FLAGS);
  if (flags != 0)
    {
      fprintf (F, " <");
      if (L_EXTRACT_BIT_VAL (flags, L_REGION_COMPILATION_COMPLETE))
        fprintf (F, "C");
      if (L_EXTRACT_BIT_VAL (flags, L_REGION_EPILOGUE))
        fprintf (F, "E");
      if (L_EXTRACT_BIT_VAL (flags, L_REGION_PROLOGUE))
        fprintf (F, "P");
      fprintf (F, ">");
    }
  /*
   *  Print the entry cb's
   */
  for (bndry = region->entry_cbs; bndry != NULL; bndry = bndry->next_boundary)
    {
      fputs ("\n   {entry (cb ", F);

      if (L_cb_hash_tbl_find (fn->cb_hash_tbl, bndry->ecb->id))
        fprintf (F, "%d) [", bndry->ecb->id);
      else
        fputs ("-1) [", F);

      if (bndry->bcb != NULL)
        {
          /*
           * Print boundary cb information
           */
          if (L_cb_hash_tbl_find (fn->cb_hash_tbl, bndry->bcb->id))
            fprintf (F, "(cb %d)", bndry->bcb->id);
          else
            fputs ("(cb -1)", F);

          /* 
           * Print live_in information 
           */
          fputs ("(live_in ", F);
          L_print_region_set (F, bndry->live_in, 1);
        }
      fputs ("]", F);
      fputs ("}", F);
    }
  /*
   *  Print the exit cb's
   */
  for (bndry = region->exit_cbs; bndry != NULL; bndry = bndry->next_boundary)
    {
      fputs ("\n   {exit (cb ", F);
      if (L_cb_hash_tbl_find (fn->cb_hash_tbl, bndry->ecb->id))
        fprintf (F, "%d) [", bndry->ecb->id);
      else
        fputs ("-1) [", F);

      if (bndry->bcb != NULL)
        {

          /*
           * Print boundary cb information
           */
          if (L_cb_hash_tbl_find (fn->cb_hash_tbl, bndry->bcb->id))
            fprintf (F, "(cb %d)", bndry->bcb->id);
          else
            fputs ("(cb -1)", F);

          /* 
           * Print live_out information 
           */
          fputs ("(live_out ", F);
          L_print_region_set (F, bndry->live_out, 1);
        }
      fputs ("]", F);
      fputs ("}", F);
    }
  /*
   *  Print the region regmap information. They are stored
   * in a hash table, so traverse the list of entries.
   */
  if ((sym = region->regmap->head_symbol) != NULL)
    {
      fputs ("\n   {regmap", F);
      for (sym = region->regmap->head_symbol;
           sym != NULL; sym = sym->next_symbol)
        {
          L_print_region_regmap (F, (L_Region_Regmap *) sym->data);
        }
      fputs ("\n   }", F);
    }
  /*
   *  Print the attributes
   */
  if (region->attr != NULL)
    {
      int cnt = 1;
      fprintf (F, "\n\t<");
      for (attr = region->attr; attr != NULL; attr = attr->next_attr)
        {
          L_print_attr (F, attr);
          cnt += 1;
          if (cnt > 2 && attr->next_attr != NULL)
            {
              fprintf (F, "\n\t ");
              cnt = 1;
            }
        }
      fprintf (F, ">");
    }
  fprintf (F, ")\n");
}

void
L_print_region_end (FILE * F, L_Region * region)
{
  fprintf (F, " (region_end %d)\n", region->id);
}
