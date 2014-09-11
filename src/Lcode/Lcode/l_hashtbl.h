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
 *      File :          l_hashtbl.h
 *      Description :   Repair has table entry when changing cb labels.
 *      Creation Date : October, 1990
 *      Author :        Pohua Chang, Scott Mahlke, Wen-mei Hwu
 *
 *==========================================================================*/
#ifndef L_HASHTBL_H
#define L_HASHTBL_H

/* 10/29/02 REK Adding config.h */
#include <config.h>

typedef struct impact_map
{
  int old_cb;
  int new_cb;
}
L_Hash_Tbl_Map;

#ifdef __cplusplus
extern "C"
{
#endif

  extern char *L_hash_tbl_current_fn;
  extern int L_hash_tbl_n_change;
  extern int L_hash_tbl_n_allocated;
  extern int L_hash_tbl_allocated;
  extern L_Hash_Tbl_Map *L_hash_tbl_change;

  extern void L_define_fn_name (char *);
  extern int L_change_cb_id (int, int);
  extern void L_repair_hashtbl (L_Data *);

#ifdef __cplusplus
}
#endif


/*=======================================================================*/
/*
 *      New jump table (hash table) routines.  Added SAM 7-96
 */
/*=======================================================================*/

#define L_JUMPTBL_DEFAULT_CC            2147483647
#define L_JUMPTBL_MAX_ID                99999   /* This is just for sanity,
                                                   check, no global array */
#define L_JUMPTBL_OLDSTYLE_BASE_NAME    "hash_" /* Int name convention */
#define L_JUMPTBL_FUNC_ATTR             "jump_tbls"   /* Atttached to funcs */
#define L_JUMPTBL_OP_ATTR               "tbl_name"    /* Attached to jump_rg */
#define L_JUMPTBL_RENAMED_STRING        "\"renamed\""

/* Jump table status flags (field of L_Func) */
#define L_JUMPTBL_MODIFIED_CONTROL      0x00000001
#define L_JUMPTBL_NEW_TABLES            0x00000002
#define L_JUMPTBL_RENAMED_TABLES        0x00000004

#ifdef __cplusplus
extern "C"
{
#endif

  extern int L_extract_jump_table_id (char *name);
  extern int L_valid_jump_table_name (char *name);
  extern char *L_construct_jump_table_name (int id);
  extern int L_oldstyle_extract_jump_table_id (char *name);
  extern int L_oldstyle_valid_jump_table_name (char *name);

  extern int L_func_has_jump_table_info (L_Func * fn);
  extern int L_num_jump_tables (L_Func * fn);
  extern int L_func_has_jump_tables (L_Func * fn);
  extern int L_func_needs_jump_table_renaming (L_Func * fn);
  extern char *L_jump_table_name (L_Oper * jrg);
  extern int L_new_jump_table_id (L_Func * fn);
  extern void L_install_jumptbl_op_attr (L_Oper * oper, char *tbl_name);
  extern void L_install_jumptbl_func_attr (L_Func * fn, int num_tbls,
                                           int max_id);
  extern void L_update_func_attr_for_new_jump_table (L_Func * fn, int new_id);
  extern void L_update_func_attr_for_deleted_jump_table (L_Func * fn);
  extern void L_update_func_attr_for_renamed_tables (L_Func * fn);

  extern void L_set_func_modified_jump_table_flag (L_Func * fn);
  extern void L_set_func_new_jump_table_flag (L_Func * fn);
  extern void L_set_func_renamed_jump_table_flag (L_Func * fn);
  extern int L_jump_tables_have_changes (L_Func * fn);

  extern char *L_create_renamed_label (char *name);
  extern void L_rename_label_operand (L_Operand * operand);
  extern void L_rename_jump_table_labels (L_Func * fn);

  extern L_Operand *L_extract_jump_table_name_operand (L_Oper * op);
  extern Set L_get_reaching_defs (L_Operand * operand, L_Oper * oper,
                                  int *df_done);
  extern int L_find_jrg_label_for_DEF (Set * op_visited, Set * op_set,
                                       L_Oper * oper, int *df_done,
                                       int *name_found);
  extern int L_find_jrg_label_for_USE (Set * op_visited, Set * op_set,
                                       L_Operand * operand, L_Oper * oper,
                                       int *df_done, int *name_found);
  extern Set L_find_jump_table_address_ops (L_Cb * cb, L_Oper * jrg,
                                            int *df_done);
  extern void L_setup_jump_table_info (L_Func * fn);

  extern int L_safe_to_make_jump_tbl_names_unique (L_Func * fn, int *df_done);
  extern void L_update_op_for_new_jump_table (L_Oper * oper, char *old_name,
                                              char *new_name);
  extern int L_make_new_jump_table (L_Func * fn, L_Cb * cb, L_Oper * jrg,
                                    int *df_done);
  extern void L_make_all_jump_tables_unique (L_Func * fn);

  extern L_Datalist *L_generate_jump_table_for_op (L_Func * fn, L_Oper * jrg);
  extern void L_regenerate_all_jump_tables (L_Func * fn);

#ifdef __cplusplus
}
#endif


#endif
