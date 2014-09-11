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
 *
 *  File:  hmdes.h
 *
 *  Description:
 *    Header file for hmdes reader.
 *
 *  Creation Date :  April, 1993 
 *
 *  Authors:  John C. Gyllenhaal, Roger Bringmann, Wen-mei Hwu
 *
\*****************************************************************************/

#ifndef HMDES_H
#define HMDES_H

/* 10/29/02 REK Adding config.h */
#include <config.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <library/l_alloc_new.h>
#include <ctype.h>
#include <stdarg.h>

#define MITEM_SIZE	401	/* Max size of parsed item */
#define MLINE_SIZE	4001	/* Max size of hmdes line */

#define MAX_HEADER_FILES	50	/* Max header file that 
					   can be scanned */
#define MSYMBOL_TABLE_SIZE 	256	/* must be power of 2 */
#define MAX_MASK_SIZE		50

/* Processor model.  Must be identical to lmdes.h declaration */
#define MDES_SUPERSCALAR	0
#define MDES_VLIW		1

/* Res_Node flags */
#define MDES_OVERLAPPING_REQUEST	0x00000001

typedef struct msymbol_st
{
  char *name;
  void *ptr;			/* Void pointer to this symbol's data */
  struct msymbol_st *next_hash;
  struct msymbol_st *next_linear;
  struct msymbol_st *prev_linear;
}
Msymbol;

typedef struct msymbol_table_st
{
  char *name;
  int hash_mask;
  Msymbol **hash_table;
  Msymbol *head;
  Msymbol *tail;
  int entry_count;
}
Msymbol_Table;

typedef struct header_file_st
{
  char *name;
  Msymbol_Table *table;
}
Header_File;

/*
 * Register file declaration
 */
typedef struct hmdes_reg_file_st
{
  int id;
  int external_id;
  char *name;
  int static_regs;		/* Number of static registers */
  int rotating_regs;		/* Number of rotating registers */
  int width;			/* Width of each register (in bits) */
}
Hmdes_Reg_File;

/*
 * IO_Node
 * 
 * For building the set linked list
 */
typedef struct hmdes_io_node_st
{
  Hmdes_Reg_File *reg_file;
  struct hmdes_io_node_st *next;
}
Hmdes_IO_Node;

/*
 * IO_Sets
 */
typedef struct hmdes_io_set_st
{
  int id;
  int external_id;
  char *name;
  Hmdes_IO_Node *head;		/* Points to head of IO_Nodes */
  Hmdes_IO_Node *tail;		/* Points to tail of IO_Nodes */
  int size;			/* Number of items in set */
}
Hmdes_IO_Set;

/*
 * IO_Items
 */
typedef struct hmdes_io_item_st
{
  int id;
  char *name;
  Hmdes_IO_Set **src;
  Hmdes_IO_Set **dest;
  Hmdes_IO_Set **pred;
}
Hmdes_IO_Item;

/*
 * Resource node for building resource subscripts
 */
typedef struct hmdes_res_sub_st
{
  struct hmdes_resource_st *resource;
  int id;
  int subscript;
  struct hmdes_res_sub_st *next;
}
Hmdes_Res_Sub;

/*
 * Resources 
 */
typedef struct hmdes_resource_st
{
  char *name;
  Hmdes_Res_Sub *head;
  Hmdes_Res_Sub *tail;
  int num_subscripts;
}
Hmdes_Resource;

typedef struct hmdes_res_option_st
{
  Hmdes_Resource *resource;
  Hmdes_Res_Sub *subscript;
  struct hmdes_res_option_st *next;
}
Hmdes_Res_Option;

/* 
 * Res_Node
 *
 * Used for building resource lists
 */
typedef struct hmdes_res_node_st
{
  int flags;
  int start_usage;
  int end_usage;
  Hmdes_Res_Option *head;
  Hmdes_Res_Option *tail;
  int num_options;
  int num_slots;
  struct hmdes_res_node_st *next;
}
Hmdes_Res_Node;

/*
 * ResTables
 */
typedef struct hmdes_res_list_st
{
  int id;
  char *name;
  Hmdes_Res_Node *head;		/* Points to head of linked list */
  Hmdes_Res_Node *tail;
  Hmdes_Res_Node *slot_options;
  int size;			/* Number of resources in list minus slots */
}
Hmdes_Res_List;

/*
 * Latencies
 */
typedef struct hmdes_latency_st
{
  int id;
  char *name;
  int exception;
  int *sync_src;		/* These all point to arrays */
  int *src;
  int *sync_dest;
  int *dest;
  int *pred;
}
Hmdes_Latency;

/*
 * Class_Node
 *
 * Used for making operation class linked list
 */
typedef struct hmdes_class_node_st
{
  Hmdes_IO_Item *io_item;
  Hmdes_Res_List *res_list;
  Hmdes_Latency *latency;
  struct hmdes_class_node_st *next;
}
Hmdes_Class_Node;

/*
 * Operation Classes
 */
typedef struct hmdes_operation_class_st
{
  char *name;
  Hmdes_Class_Node *head;	/* Head of linked list */
  Hmdes_Class_Node *tail;
  int size;
}
Hmdes_Operation_Class;

/* 
 * Mdes_Flag
 *
 * Used for building linked list of flags specified.
 */
typedef struct hmdes_flag_st
{
  char *name;
  int value;
  struct hmdes_flag_st *next;
}
Hmdes_Flag;

typedef struct hmdes_flag_list_st
{
  Hmdes_Flag *head;
  int num_flags;
  int bit_version;
}
Hmdes_Flag_List;
/*
 * Operation_Node
 *
 * Used for making linked list for Operations
 */
typedef struct hmdes_operation_node_st
{
  char *asm_name;
  Hmdes_Operation_Class *class;
  Hmdes_Flag_List mdes_flags;
  struct hmdes_operation_node_st *next;
}
Hmdes_Operation_Node;

/*
 * Operations
 */
typedef struct hmdes_operation_st
{
  int id;
  char *name;
  Hmdes_Operation_Node *head;	/* Head of linked list */
  Hmdes_Operation_Node *tail;
  int size;
  Hmdes_Flag_List op_flags;
}
Hmdes_Operation;

typedef struct hmdes_st
{
  char *file_name;
  int processor_model;
  int max_src_operands;
  int max_src_syncs;
  int max_dest_operands;
  int max_dest_syncs;
  int max_pred_operands;
  int max_slot;
  Msymbol_Table *defines_command;
  Msymbol_Table *defines_environ;
  Msymbol_Table *defines_internal;
  Msymbol_Table *reg_file;
  Msymbol_Table *IO_sets;
  Msymbol_Table *IO_items;
  int num_resources;
  Msymbol_Table *resources;
  Msymbol_Table *res_lists;
  Msymbol_Table *latencies;
  Msymbol_Table *op_class;
  Msymbol_Table *operations;
  Hmdes_IO_Set *null_set;
}
Hmdes;

typedef struct mparse_item_st
{
  char buf[MITEM_SIZE];
  struct mparse_item_st *next;
}
Mparse_Item;

typedef struct mparse_info_st
{
  char *file_name;
  char section_name[100];
  FILE *in;
  int line;
  Mparse_Item *head;
  Mparse_Item *tail;
  int queue_size;
  Hmdes *hmdes;
}
Mparse_Info;

/* Parameters */
extern int verbose;

/* Prototypes */
extern Msymbol_Table *Hread_header_file (char *file_name);
extern void Mdie_on_fatal_errors (Mparse_Info * pinfo);
extern void Hbuild_environment_defines (Msymbol_Table * env_tab, char **envp);
extern void Hbuild_command_line_defines (Msymbol_Table * cl_tab, char **argv);
extern Hmdes *create_hmdes (char *hmdes_file_name, char **argv, char **envp);
extern void hmdes_read_section (Mparse_Info * pinfo, char *name,
				void (*func) ());
extern void hmdes_read_define (Mparse_Info * pinfo, char *sname);
extern int Mis_name_next (Mparse_Info * pinfo);
extern int Mis_next (Mparse_Info * pinfo, char *str);
extern void Mread_name (Mparse_Info * pinfo, char *name, char *desc);
extern void Mread_symbol (Mparse_Info * pinfo, Msymbol_Table * table,
			  void **ptr, char *desc);
extern void Mread_IO_item_symbol (Mparse_Info * pinfo, Msymbol_Table * table,
				  void **ptr, char *terminator);
extern void Mread_match (Mparse_Info * pinfo, char *str);
extern int mscan_set (Mparse_Info * pinfo, char *name);
extern int mread_proc_model (Mparse_Info * pinfo, char *name, int *ptr);
extern int mread_define_var (Mparse_Info * pinfo, char *name, int *ptr);
extern void Malloc_struct (void **ptr, int size);
extern void Malloc_name (char **ptr, char *name);
extern int Mread_int (Mparse_Info * pinfo, int *ptr);
extern Msymbol_Table *Mcreate_symbol_table (char *name, int size);
extern void *Mfind_symbol (Msymbol_Table * table, char *name);
extern void Minsert_unique_symbol (Mparse_Info * pinfo, Msymbol_Table * table,
				   char *name, void *ptr);
extern void Minsert_symbol (Msymbol_Table * table, char *name, void *ptr);
extern void Mprint_symbol_table (FILE * out, Msymbol_Table * table);
extern int Mhash_name (char *name, int mask);
extern int find_header_value (Mparse_Info * pinfo, char *name, int *ptr);
extern void hmdes_read_register_files (Mparse_Info * pinfo);
extern void Madd_IO_node (Hmdes_IO_Set * IO_set, Hmdes_Reg_File * reg_file);
extern void hmdes_read_IO_sets (Mparse_Info * pinfo);
extern void hmdes_read_IO_items (Mparse_Info * pinfo);
extern void hmdes_add_res_subscript (Mparse_Info * pinfo,
				     Hmdes_Resource * resource, int subscript,
				     int id);
extern void hmdes_read_resources (Mparse_Info * pinfo);
extern void Madd_res_option (Mparse_Info * pinfo, Hmdes_Res_Node * node,
			     char *name, int subscript);
extern void hmdes_read_restables (Mparse_Info * pinfo);
extern void Mread_latency (Mparse_Info * pinfo, int *ptr);
extern void hmdes_read_latencies (Mparse_Info * pinfo);
extern Hmdes_Operation_Class *hmdes_read_class_def (Mparse_Info * pinfo,
						    char *class_name);
extern void hmdes_read_operation_class (Mparse_Info * pinfo);
extern void Mfree_flag_list (Hmdes_Flag_List * flag_list);
extern int Mmatch_flag_lists (Hmdes_Flag_List * list1,
			      Hmdes_Flag_List * list2);
extern int valid_flag_value (Mparse_Info * pinfo, Hmdes_Flag * flag);
extern void Mread_flag (Mparse_Info * pinfo, Hmdes_Flag_List * flag_list,
			char *flag_prefix, char *desc);
extern void hmdes_read_operations (Mparse_Info * pinfo);
extern void Mprint_op (FILE * out, Hmdes * hmdes, char *name);
extern int hmdes_section_end (Mparse_Info * pinfo);
extern int Mpeek_ahead (Mparse_Info * pinfo, int dist, char *buf);
extern int Mget_next (Mparse_Info * pinfo, char *buf);
extern void L_preprocess_line (Mparse_Info * pinfo, char *raw_line, char *buf,
			       int max);
extern int Misspace (char ch);
extern char *Mstrip (char *buf);
extern Mparse_Info *create_mparse_info (char *name, Hmdes * hmdes);
extern void Mparse_warn (Mparse_Info * pinfo, char *fmt, ...);
extern void Mparse_fatal (Mparse_Info * pinfo, char *fmt, ...);
extern void Mparse_error (Mparse_Info * pinfo, char *fmt, ...);
extern void H_punt (char *fmt, ...);
extern int Mmatch (char *s1, char *s2);
extern void associate_reg_files (Hmdes * hmdes);
extern void associate_opcodes (Hmdes * hmdes);
extern void clear_mask (int *mask, int size);
extern void set_mask_bit (int *mask, unsigned int bit);
extern void print_mask (FILE * out, int *mask, int size);
extern void write_res_node (FILE * out, int mask_size,
			    Hmdes_Res_Node * res_node);
extern void write_lmdes (Hmdes * hmdes, FILE * out);

#endif
