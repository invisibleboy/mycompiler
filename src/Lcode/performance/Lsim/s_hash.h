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
 *      File:   s_hash.h
 *      Author: Teresa Johnson
 *      Creation Date:  1995
 *      Copyright (c) 1995 Teresa Johnson, Wen-mei Hwu and
 *                         The Board of Trustees of the University of Illinois.
 *                         All rights reserved.
 *      The University of Illinois software License Agreement
 *      specifies the terms and conditions for redistribution.
\*****************************************************************************/

#ifndef _LSIM_S_HASH_H_
#define _LSIM_S_HASH_H_

#include <config.h>

/* Total mem size (in bytes) used for hash tables will be:
 * 	152*MEMDEP_DATA_HASH_SIZE + 176*MEMDEP_ACCESS_HASH_SIZE
 * as long as the formulas to calc MEMDEP_MAX_* are not changed.
 */
#define MEMDEP_DATA_HASH_SIZE 1024*128
#define MEMDEP_ACCESS_HASH_SIZE 1024*16
#define MEMDEP_MAX_DATA 4*MEMDEP_DATA_HASH_SIZE
#define MEMDEP_MAX_LOAD 2*MEMDEP_ACCESS_HASH_SIZE
#define MEMDEP_MAX_STORE 2*MEMDEP_ACCESS_HASH_SIZE
#define MEMDEP_MAX_ALIAS 8*MEMDEP_ACCESS_HASH_SIZE

typedef struct S_hash_node
{
	int key;
	void *ptr;
	struct S_hash_node *next,*prev;
	struct S_hash_node *Qnext,*Qprev;
} S_hash_node;

extern int aliases_in_file;

void S_init_hash(void);
S_hash_node *S_hash_insert_data_info(int key);
S_hash_node *S_hash_insert_load_info(int key);
S_hash_node *S_hash_insert_store_info(int key);
void S_delete_hash(void);
S_hash_node *S_hash_find_data_info(int key);
S_hash_node *S_hash_find_load_info(int key);
S_hash_node *S_hash_find_store_info(int key);
void S_hash_remove_oldest_data_info();
void S_hash_remove_oldest_load_info();
void S_hash_remove_oldest_store_info();
void S_delete_alias_info(S_MemDep_Alias_Info *info,int *num_alias);
void S_print_alias_info(S_MemDep_Alias_Info *info,int pc);

/* PDC data info hash functions */
void S_init_pdc_hash(void);
void S_delete_pdc_hash(void);
void S_hash_remove_oldest_pdc_data_info();
S_hash_node *S_hash_insert_pdc_data_info(int key);
S_hash_node *S_hash_find_pdc_data_info(int key);

#endif
