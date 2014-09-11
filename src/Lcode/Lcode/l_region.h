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
 *      File:  l_region.h
 *      Author: Richard E. Hank, Wen-mei Hwu
 *      Creation Date:  April 1995
 *
\*****************************************************************************/
#ifndef L_REGION_H
#define L_REGION_H

/* 10/29/02 REK Adding config.h */
#include <config.h>

/*
 *      L_Region_Member
 */
typedef struct L_Region_Member
{
  L_Cb *cb;                     /* ptr to member cb */
  struct L_Region_Member *next_member;
}
L_Region_Member;

/*
 *      L_Region_Boundary
 */
typedef struct L_Region_Boundary
{
  L_Cb *ecb;                    /* ptr to entry/exit cb.  
                                   The ecb is the cb where 
                                   control flow enters the 
                                   region */
  L_Cb *bcb;                    /* ptr boundary cb.  
                                   The boundary cb is the cb 
                                   that transfers flow of 
                                   control into the region or 
                                   is the target of exiting 
                                   control flow. */
  Set live_in;
  Set live_out;
  struct L_Region_Boundary *next_boundary;
}
L_Region_Boundary;

#define L_REGION_VREG_ALLOC     0x01
#define L_REGION_VREG_SPILL     0x02
#define L_REGION_VREG_IGNORE    0x04

/*
 *      L_Region_Regcon
 */
typedef struct L_Region_Regcon
{
  L_Cb *boundary_cb;
  int flags;
  int type;
  int phys_reg;
}
L_Region_Regcon;

/*
 *      L_Region_Regmap
 */
typedef struct L_Region_Regmap
{
  int vreg_id;                  /* virtual register id */
  int ctype;                    /* ctype :) */
  int flags;                    /* global flags, alloc/spilled first */
  Set occupied;                 /* unavailable registers */
  double jsr_weight;
  double ref_weight;
  int spill_loc;                /* stack location for spilling */
  int type;                     /* type REGISTER/MACRO */
  int phys_reg;                 /* physical register assigned */
  INT_Symbol_Table *constraints;
}
L_Region_Regmap;

/*
 *      L_Region
 */
typedef struct L_Region
{
  int id;                       /* region id */
  int flags;                    /* bit field flags */
  L_Region_Member *member_cbs;  /* list of region members */
  Set member_set;               /* set of region member ids */
  L_Region_Boundary *entry_cbs; /* entry points to the region */
  L_Region_Boundary *exit_cbs;  /* exit points from the region */
  INT_Symbol_Table *regmap;     /* exposed vreg regalloc info  */
  double weight;
  L_Attr *attr;                 /* additional attributes */
  struct L_Region *prev_region; /* previous region in list */
  struct L_Region *next_region; /* next region in list */
}
L_Region;

/*
 *      Region hash table entry
 */

typedef struct L_Region_Hash_Entry
{
  int id;                       /* hash key (oper id) */
  L_Region *region;             /* region ptr */
  struct L_Region_Hash_Entry *prev_region_hash;
  struct L_Region_Hash_Entry *next_region_hash;
}
L_Region_Hash_Entry;

/*========================================================================*/
/*
 *      L_Region flags
 */
/*========================================================================*/

/* printed char */
#define L_REGION_PRESENCE                           0x00000001  /* -    */
#define L_REGION_COMPILATION_COMPLETE               0x00000002  /* C    */
#define L_REGION_HAS_JRG                            0x00000004  /* -    */
#define L_REGION_AGGRESSIVE_OPTIMIZATION            0x00000008  /* -    */
#define L_REGION_PROLOGUE                           0x00000010  /* P    */
#define L_REGION_FUNCTION                           0x00000020  /* -    */
#define L_REGION_REGISTER_ALLOCATED                 0x00000040  /* R    */
#define L_REGION_EPILOGUE                           0x00000080  /* E    */

/* These bits should be set for the above func flags that do not print */
#define L_REGION_PRINTED        (L_REGION_COMPILATION_COMPLETE |\
                                 L_REGION_PROLOGUE |\
                                 L_REGION_EPILOGUE)
#define L_REGION_HIDDEN_FLAGS   ~(L_REGION_PRINTED)

/* Region flag explanation, printed character in Lcode is in () before
 *      flag name.  A "(-)" indicates the flag is not printed.
 *
 *   (-) L_REGION_PRESENCE - region is already present in the Lcode, 
 *                           or has already been printed to the Lcode file.
 *   (C) L_REGION_COMPILATION_COMPLETE - compilation of region is complete
 *
 *   (-) L_REGION_HAS_JRG - region contains a jump_rg operation.
 *
 *   (-) L_REGION_PROLOGUE - region contains the function prologue.
 *
 *   (-) L_REGION_FUNCTION - region contains the entire function
 *
 *   (R) L_REGION_REGISTER_ALLOCATED - the region has been register allocated
 */

#ifdef __cplusplus
extern "C"
{
#endif

  extern L_Region_Hash_Entry **L_global_region_hash_tbl;
  extern int L_print_all_regions;
  extern L_Region *L_last_printed_region;

#include <Lcode/l_io.h>

  extern L_Region_Regmap *L_find_region_regmap (L_Region * region,
                                                int vreg_id);
  extern L_Region_Regcon *L_find_regcon_for_cb (L_Region_Regmap * regmap,
                                                L_Cb * cb);
  extern void L_add_cb_to_region (L_Region * region, L_Cb * cb);
  extern L_Region_Boundary *L_add_entry_cb_to_region (L_Region * region,
                                                      L_Cb * ecb, L_Cb * bcb);
  extern L_Region_Boundary *L_add_exit_cb_to_region (L_Region * region,
                                                     L_Cb * ecb, L_Cb * bcb);
  extern void L_add_regcon_to_regmap (L_Region_Regmap * regmap,
                                      L_Region_Regcon * regcon);
  extern void L_add_regmap_to_region (L_Region * region,
                                      L_Region_Regmap * regmap);
  extern int L_cb_in_region (L_Region * region, L_Cb * cb);
  extern void L_change_region_entry_boundary_cb (L_Region * region,
                                                 L_Cb * ecb, L_Cb * old_bcb,
                                                 L_Cb * new_bcb);
  extern void L_change_region_exit_boundary_cb (L_Region * region, L_Cb * ecb,
                                                L_Cb * old_bcb,
                                                L_Cb * new_bcb);
  extern void L_change_region_exit_cb (L_Region * region, L_Cb * old_ecb,
                                       L_Cb * new_ecb, L_Cb * bcb);
  extern void L_delete_all_entry_cbs (L_Region * region);
  extern void L_delete_all_exit_cbs (L_Region * region);
  extern void L_delete_all_region (L_Region * list,
                                   L_Region_Hash_Entry ** region_hash_tbl);
  extern void L_delete_region (L_Region * region);
  extern void L_delete_region_boundary (L_Region_Boundary * bndry);
  extern void L_delete_region_member (L_Region_Member * mbr);
  extern void L_delete_region_regcon (L_Region_Regcon * regcon);
  extern void L_delete_region_regmap (L_Region_Regmap * regmap);
  extern Set L_get_region_entry_IN_set (L_Region * region, L_Cb * bcb,
                                        L_Cb * ecb);
  extern Set L_get_region_exit_OUT_set (L_Region * region, L_Cb * bcb);
  extern Set L_get_region_member_set (L_Region * region);
  extern int L_get_region_seed (L_Region * region);
  extern void L_insert_region_after (L_Func * fn, L_Region * after_region,
                                     L_Region * region);
  extern int L_is_region_entry (L_Region * region, L_Cb * cb);
  extern int L_is_region_exit (L_Region * region, L_Cb * cb);
  extern L_Region *L_new_region (int id);
  extern L_Region_Boundary *L_new_region_boundary (L_Cb * ecb, L_Cb * bcb);
  extern L_Region *L_new_region_in_func (L_Func * fn, int id);
  extern L_Region_Member *L_new_region_member (L_Cb * cb);
  extern L_Region_Regcon *L_new_region_regcon (L_Cb * bcb, int flags,
                                               int type, int phys_reg);
  extern L_Region_Regmap *L_new_region_regmap (int vreg_id);
  extern int L_num_region_entry_cbs (L_Region * region);
  extern void L_print_region (FILE * F, L_Func * fn, L_Region * region);
  extern void L_print_region_end (FILE * F, L_Region * region);
  extern void L_print_region_regmap (FILE * F, L_Region_Regmap * regmap);
  extern void L_print_region_set (FILE * F, Set set, int decode);
  extern L_Region *L_read_region (L_Func * fn, L_Input_Buf * input_buf);
  extern void L_read_region_boundary_conditions (L_Func * fn,
                                                 L_Region * region,
                                                 L_Input_Buf * input_buf);
  extern L_Region *L_read_region_end (L_Func * fn, L_Input_Buf * input_buf);
  extern void L_read_region_flags (L_Region * region,
                                   L_Input_Buf * input_buf);
  extern void L_read_region_regmap (L_Func * fn, L_Region * region,
                                    L_Input_Buf * input_buf);
  extern void L_read_region_reg_constraint (L_Func * fn,
                                            L_Region_Regmap * regmap,
                                            L_Input_Buf * input_buf);
  extern Set L_read_region_set (L_Input_Buf * input_buf, int encode);
  extern L_Oper *L_region_boundary_insert_point (L_Cb * cb);
  extern int L_region_empty (L_Region * region);
  extern L_Region_Hash_Entry **L_region_hash_tbl_create (void);
  extern void L_region_hash_tbl_delete (L_Region_Hash_Entry **
                                        region_hash_tbl, L_Region * region);
  extern void L_region_hash_tbl_delete_all (L_Region_Hash_Entry **
                                            region_hash_tbl);
  extern L_Region *L_region_hash_tbl_find (L_Region_Hash_Entry **
                                           region_hash_tbl, int region_id);
  extern void L_region_hash_tbl_insert (L_Region_Hash_Entry **
                                        region_hash_tbl, L_Region * region);
  extern void L_remove_cb_from_region (L_Region * region, L_Cb * cb);
  extern void L_remove_region_entry_cb (L_Region * region, L_Cb * ecb,
                                        L_Cb * bcb, int punt);
  extern void L_set_region_seed (L_Region * region, int cb_id);



#ifdef __cplusplus
}
#endif

#endif
