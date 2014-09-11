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
 *      File:   s_hash.c
 *      Author: Teresa Johnson
 *      Creation Date:  1995
 *      Copyright (c) 1995 Teresa Johnson, Wen-mei Hwu and
 *                         The Board of Trustees of the University of Illinois.
 *                         All rights reserved.
 *      The University of Illinois software License Agreement
 *      specifies the terms and conditions for redistribution.
\*****************************************************************************/

#include <config.h>

#ifndef lint
#define lint
static char copyright[] =
"@(#) Copyright (c) 1995 Teresa Johnson, Wen-mei Hwu and The Board of \n\
 Trustees of the University of Illinois. All rights reserved.\n";
#endif

#include <Lcode/l_main.h>
#include "s_profile.h"
#include "s_mem_profile.h"
#include "s_hash.h"
#include "s_main.h"

/* Hash Tables */
S_hash_node *pdc_data_info_hash[MEMDEP_DATA_HASH_SIZE];
S_hash_node *data_info_hash[MEMDEP_DATA_HASH_SIZE];
S_hash_node *load_info_hash[MEMDEP_ACCESS_HASH_SIZE];
S_hash_node *store_info_hash[MEMDEP_ACCESS_HASH_SIZE];

/* Queues */
S_hash_node *data_queue;
S_hash_node *load_queue;
S_hash_node *store_queue;
S_hash_node *data_last;
S_hash_node *load_last;
S_hash_node *store_last;
int num_in_data_hash;
int num_in_load_hash;
int num_in_store_hash;
int num_load_alias;
int num_store_alias;
int aliases_in_file;

/* PDC Queues */
S_hash_node *pdc_data_queue;
S_hash_node *pdc_data_last;
int num_in_pdc_data_hash;

/* L_Alloc_Pool's */
L_Alloc_Pool *S_hash_pool = NULL;

extern L_Alloc_Pool    *S_MemDep_Data_Info_pool;
extern L_Alloc_Pool    *S_MemDep_Access_Info_pool;
extern L_Alloc_Pool    *S_MemDep_Alias_Info_pool;
extern L_Alloc_Pool    *S_Pdc_MemDep_Data_Info_pool;

extern S_Profile_Info *prof_info;
extern FILE *tmp_file;

extern int S_clear_addrs_on_overflow;

void S_delete_data_info(S_MemDep_Data_Info *info);
void S_delete_access_info(S_MemDep_Access_Info *info,int *num_alias);
void S_delete_pdc_data_info(S_Pdc_MemDep_Data_Info *info);

void S_init_hash(void)
{
	int i;

        if (!S_hash_pool)
           S_hash_pool = 
                L_create_alloc_pool("S_hash_pool",sizeof(S_hash_node),1);
	for (i=0;i<MEMDEP_DATA_HASH_SIZE;i++) data_info_hash[i] = NULL;
	for (i=0;i<MEMDEP_ACCESS_HASH_SIZE;i++) load_info_hash[i] = NULL;
	for (i=0;i<MEMDEP_ACCESS_HASH_SIZE;i++) store_info_hash[i] = NULL;
	num_in_data_hash = 0;
	num_in_load_hash = 0;
	num_in_store_hash = 0;
	num_load_alias = 0;
	num_store_alias = 0;
	data_last = load_last = store_last = NULL;
	data_queue = load_queue = store_queue = NULL;
}

void S_init_pdc_hash(void)
{
        int i;

        if (!S_hash_pool)
           S_hash_pool =
                L_create_alloc_pool("S_hash_pool",sizeof(S_hash_node),1);

        for (i=0;i<MEMDEP_DATA_HASH_SIZE;i++) pdc_data_info_hash[i] = NULL;

        num_in_pdc_data_hash = 0;

        pdc_data_last =  NULL;
        pdc_data_queue = NULL;
}


S_hash_node *S_hash_insert_data_info(int key)
{
	int i = (key >> 2) & (MEMDEP_DATA_HASH_SIZE-1);
	S_hash_node *this,*prev;

	if (S_clear_addrs_on_overflow && num_in_data_hash > MEMDEP_MAX_DATA)
	    S_hash_remove_oldest_data_info();
	if (!data_info_hash[i])
	{
	    data_info_hash[i] = (S_hash_node *) L_alloc(S_hash_pool);
	    this = data_info_hash[i];
	    prev = NULL;
	}
	else if (!S_clear_addrs_on_overflow)
	{
	    this = data_info_hash[i];
	    prev = NULL;
	    num_in_data_hash--;
	}
	else
	{
	    this = data_info_hash[i];
	    while (this->next) this = this->next;
	    this->next = (S_hash_node *) L_alloc(S_hash_pool);
	    prev = this;
	    this = this->next;
	}
	this->key = key;
	this->ptr = NULL;
	this->next = NULL;
	this->prev = prev;
	this->Qprev = NULL;
	this->Qnext = data_queue;
	if (data_queue) data_queue->Qprev = this;
	data_queue = this;
	if (!data_last) data_last = this;
	num_in_data_hash++;
	return this;
}

S_hash_node *S_hash_insert_pdc_data_info(int key)
{
        int i = (key >> 2) & (MEMDEP_DATA_HASH_SIZE-1);
        S_hash_node *this,*prev;

        if (S_clear_addrs_on_overflow && num_in_pdc_data_hash > MEMDEP_MAX_DATA)
            S_hash_remove_oldest_pdc_data_info();
        if (!pdc_data_info_hash[i])
        {
            pdc_data_info_hash[i] = (S_hash_node *) L_alloc(S_hash_pool);
            this = pdc_data_info_hash[i];
            prev = NULL;
        }
        else if (!S_clear_addrs_on_overflow)
        {
            this = pdc_data_info_hash[i];
            prev = NULL;
            num_in_pdc_data_hash--;
        }
        else
        {
            this = pdc_data_info_hash[i];
            while (this->next) this = this->next;
            this->next = (S_hash_node *) L_alloc(S_hash_pool);
            prev = this;
            this = this->next;
        }
        this->key = key;
        this->ptr = NULL;
        this->next = NULL;
        this->prev = prev;
        this->Qprev = NULL;
        this->Qnext = pdc_data_queue;
        if (pdc_data_queue) pdc_data_queue->Qprev = this;
        pdc_data_queue = this;
        if (!pdc_data_last) pdc_data_last = this;
        num_in_pdc_data_hash++;
        return this;
}


S_hash_node *S_hash_insert_load_info(int key)
{
	int i = key & (MEMDEP_ACCESS_HASH_SIZE-1);
	S_hash_node *this,*prev;

	while ((num_in_load_hash > MEMDEP_MAX_LOAD) ||
			(num_load_alias > MEMDEP_MAX_ALIAS))
		S_hash_remove_oldest_load_info();
	if (!load_info_hash[i])
	{
	   load_info_hash[i] = (S_hash_node *) L_alloc(S_hash_pool);
	   this = load_info_hash[i];
	   prev = NULL;
	}
	else
	{
	   this = load_info_hash[i];
	   while (this->next) this = this->next;
	   this->next = (S_hash_node *) L_alloc(S_hash_pool);
	   prev = this;
	   this = this->next;
	}
	this->key = key;
	this->ptr = NULL;
	this->next = NULL;
	this->prev = prev;
	this->Qprev = NULL;
	this->Qnext = load_queue;
	if (load_queue) load_queue->Qprev = this;
	load_queue = this;
	if (!load_last)
		load_last = this;
	num_in_load_hash++;
	return this;
}

S_hash_node *S_hash_insert_store_info(int key)
{
	int i = key & (MEMDEP_ACCESS_HASH_SIZE-1);
	S_hash_node *this,*prev;

	while ((num_in_store_hash > MEMDEP_MAX_STORE) ||
			(num_store_alias > MEMDEP_MAX_ALIAS))
		S_hash_remove_oldest_store_info();
	if (!store_info_hash[i])
	{
	   store_info_hash[i] = (S_hash_node *) L_alloc(S_hash_pool);
	   this = store_info_hash[i];
	   prev = NULL;
	}
	else
	{
	   this = store_info_hash[i];
	   while (this->next) this = this->next;
	   this->next = (S_hash_node *) L_alloc(S_hash_pool);
	   prev = this;
	   this = this->next;
	}
	this->key = key;
	this->ptr = NULL;
	this->next = NULL;
	this->prev = prev;
	this->Qprev = NULL;
	this->Qnext = store_queue;
	if (store_queue) store_queue->Qprev = this;
	store_queue = this;
	if (!store_last)
		store_last = this;
	num_in_store_hash++;
	return this;
}

void S_delete_hash(void)
{
	int i;
	S_hash_node *this,*next;

	for (i=0;i<MEMDEP_DATA_HASH_SIZE;i++)
	{
	   this = data_info_hash[i];
	   while (this)
	   {
	      next = this->next;
	      S_delete_data_info((S_MemDep_Data_Info *)this->ptr);
	      L_free(S_hash_pool,this);
	      this = next;
	   }
	   data_info_hash[i] = NULL;
	}
	for (i=0;i<MEMDEP_ACCESS_HASH_SIZE;i++)
	{
	   this = load_info_hash[i];
	   while (this)
	   {
	      next = this->next;
	      S_delete_access_info((S_MemDep_Access_Info *)this->ptr,
			&num_load_alias);
	      L_free(S_hash_pool,this);
	      this = next;
	   }
	   load_info_hash[i] = NULL;
	   this = store_info_hash[i];
	   while (this)
	   {
	      next = this->next;
	      S_delete_access_info((S_MemDep_Access_Info *)this->ptr,
			&num_store_alias);
	      L_free(S_hash_pool,this);
	      this = next;
	   }
	   store_info_hash[i] = NULL;
	}
	num_in_data_hash = num_in_load_hash = num_in_store_hash = 0;
}

void S_delete_pdc_hash(void)
{
        int i;
        S_hash_node *this,*next;
        for (i=0;i<MEMDEP_DATA_HASH_SIZE;i++)
        {
           this = pdc_data_info_hash[i];
           while (this)
           {
              next = this->next;
              S_delete_pdc_data_info((S_Pdc_MemDep_Data_Info *)this->ptr);
              L_free(S_hash_pool,this);
              this = next;
           }
           pdc_data_info_hash[i] = NULL;
        }
        num_in_pdc_data_hash = 0;
}

S_hash_node *S_hash_find_data_info(int key)
{
	S_hash_node *node;
	int i;

	i = (key >> 2) & (MEMDEP_DATA_HASH_SIZE-1);
	node = data_info_hash[i];
	if (!S_clear_addrs_on_overflow) return node;
	while (node)
	{
	   if (node->key == key) break;
	   node = node->next;
	}
	if (!node) return NULL;
	if (node == data_queue) return node;
	if (node->Qprev) node->Qprev->Qnext = node->Qnext;
	if (node->Qnext) node->Qnext->Qprev = node->Qprev;
	if (node == data_last)
	   data_last = data_last->Qprev;
	node->Qprev = NULL;
	node->Qnext = data_queue;
	data_queue->Qprev = node;
	data_queue = node;
	return node;
}

S_hash_node *S_hash_find_pdc_data_info(int key)
{
        S_hash_node *node;
        int i;

        i = (key >> 2) & (MEMDEP_DATA_HASH_SIZE-1);
        node = pdc_data_info_hash[i];
        if (!S_clear_addrs_on_overflow) return node;
        while (node)
        {
           if (node->key == key) break;
           node = node->next;
        }
        if (!node) return NULL;
        if (node == pdc_data_queue) return node;
        if (node->Qprev) node->Qprev->Qnext = node->Qnext;
        if (node->Qnext) node->Qnext->Qprev = node->Qprev;
        if (node == pdc_data_last)
           pdc_data_last = pdc_data_last->Qprev;
        node->Qprev = NULL;
        node->Qnext = pdc_data_queue;
        pdc_data_queue->Qprev = node;
        pdc_data_queue = node;
        return node;
}

S_hash_node *S_hash_find_load_info(int key)
{
	S_hash_node *node;
	int i;

	i = key & (MEMDEP_ACCESS_HASH_SIZE-1);	
	node = load_info_hash[i];
	while (node)
	{
	   if (node->key == key) break;
	   node = node->next;
	}
	if (!node) return NULL;
	if (node == load_queue) return node;
	if (node->Qprev) node->Qprev->Qnext = node->Qnext;
	if (node->Qnext) node->Qnext->Qprev = node->Qprev;
	if (node == load_last)
	   load_last = load_last->Qprev;
	node->Qprev = NULL;
	node->Qnext = load_queue;
	load_queue->Qprev = node;
	load_queue = node;
	return node;
}

S_hash_node *S_hash_find_store_info(int key)
{
	S_hash_node *node;
	int i;

	i = key & (MEMDEP_ACCESS_HASH_SIZE-1);	
	node = store_info_hash[i];
	while (node)
	{
	   if (node->key == key) break;
	   node = node->next;
	}
	if (!node) return NULL;
	if (node == store_queue) return node;
	if (node->Qprev) node->Qprev->Qnext = node->Qnext;
	if (node->Qnext) node->Qnext->Qprev = node->Qprev;
	if (node == store_last)
	   store_last = store_last->Qprev;
	node->Qprev = NULL;
	node->Qnext = store_queue;
	store_queue->Qprev = node;
	store_queue = node;
	return node;
}

void S_hash_remove_oldest_data_info()
{
        /* 10/25/04 REK Commenting out unused variable to quiet compiler
	 *              warning. */
#if 0
        S_hash_node *node;
#endif
	int i;

	if (data_last->prev) data_last->prev->next = data_last->next;
        else
	{
	    i = (data_last->key >> 2) & (MEMDEP_DATA_HASH_SIZE-1);
	    data_info_hash[i] = data_last->next;
	}
	if (data_last->next) data_last->next->prev = data_last->prev;
	if (data_last->Qprev)
	{
	  data_last = data_last->Qprev;
	  S_delete_data_info((S_MemDep_Data_Info *)data_last->Qnext->ptr);
	  L_free(S_hash_pool,data_last->Qnext);
	  data_last->Qnext = NULL;
	}
	else
	{
	  S_delete_data_info((S_MemDep_Data_Info *)data_last->ptr);
	  L_free(S_hash_pool,data_last);
	  data_last = NULL;
	}
	num_in_data_hash--;
}


void S_hash_remove_oldest_pdc_data_info()
{
        /* 10/25/04 REK Commenting out unused variable to quiet compiler
	 *              warning. */
#if 0
        S_hash_node *node;
#endif
        int i;

        if (pdc_data_last->prev) pdc_data_last->prev->next = pdc_data_last->next;
        else
        {
            i = (pdc_data_last->key >> 2) & (MEMDEP_DATA_HASH_SIZE-1);
            pdc_data_info_hash[i] = pdc_data_last->next;
        }
        if (pdc_data_last->next) pdc_data_last->next->prev = pdc_data_last->prev;
        if (pdc_data_last->Qprev)
        {
          pdc_data_last = pdc_data_last->Qprev;
          S_delete_pdc_data_info((S_Pdc_MemDep_Data_Info *)pdc_data_last->Qnext->ptr);
          L_free(S_hash_pool,pdc_data_last->Qnext);
          pdc_data_last->Qnext = NULL;
        }
        else
        {
          S_delete_pdc_data_info((S_Pdc_MemDep_Data_Info *)pdc_data_last->ptr);
          L_free(S_hash_pool,pdc_data_last);
          pdc_data_last = NULL;
        }
        num_in_pdc_data_hash--;
}


void S_hash_remove_oldest_load_info()
{
        /* 10/25/04 REK Commenting out unused variable to quiet compiler
	 *              warning. */
#if 0
        S_hash_node *node;
#endif

	if (load_last->prev) load_last->prev->next = load_last->next;
        else load_info_hash[load_last->key & (MEMDEP_ACCESS_HASH_SIZE-1)] 
					= load_last->next;
	if (load_last->next) load_last->next->prev = load_last->prev;
	if (load_last->Qprev)
	{
	  load_last = load_last->Qprev;
	  S_delete_access_info((S_MemDep_Access_Info *)load_last->Qnext->ptr,
		&num_load_alias);
	  L_free(S_hash_pool,load_last->Qnext);
	  load_last->Qnext = NULL;
	}
	else
	{
	  S_delete_access_info((S_MemDep_Access_Info *)load_last->ptr,
		&num_load_alias);
	  L_free(S_hash_pool,load_last);
	  load_last = NULL;
	}
	num_in_load_hash--;
}

void S_hash_remove_oldest_store_info()
{
        /* 10/25/04 REK Commenting out unused variable to quiet compiler
	 *              warning. */
#if 0
        S_hash_node *node;
#endif

	if (store_last->prev) store_last->prev->next = store_last->next;
        else store_info_hash[store_last->key & (MEMDEP_ACCESS_HASH_SIZE-1)] 
					= store_last->next;
	if (store_last->next) store_last->next->prev = store_last->prev;
	if (store_last->Qprev)
	{
	  store_last = store_last->Qprev;
	  S_delete_access_info((S_MemDep_Access_Info *)store_last->Qnext->ptr,
		&num_store_alias);
	  L_free(S_hash_pool,store_last->Qnext);
	  store_last->Qnext = NULL;
	}
	else
	{
	  S_delete_access_info((S_MemDep_Access_Info *)store_last->ptr,
		&num_store_alias);
	  L_free(S_hash_pool,store_last);
	  store_last = NULL;
	}
	num_in_store_hash--;
}

void S_delete_data_info(S_MemDep_Data_Info *info)
{
	if (!info) return;
	L_free(S_MemDep_Data_Info_pool,info);
}

void S_delete_pdc_data_info(S_Pdc_MemDep_Data_Info *info)
{
        if (!info) return;
        L_free(S_Pdc_MemDep_Data_Info_pool,info);
}

void S_delete_access_info(S_MemDep_Access_Info *info,int *num_alias)
{
        /* 10/25/04 REK Commenting out unused variables to quiet compiler
	 *              warnings. */
#if 0
	S_MemDep_Alias_Info *next,*this;
#endif

	if (!info) return;
	S_print_alias_info(info->alias_info,info->pc);
	S_delete_alias_info(info->alias_info,num_alias);
	info->alias_info = NULL;
	L_free(S_MemDep_Access_Info_pool,info);
}

void S_delete_alias_info(S_MemDep_Alias_Info *info,int *num_alias)
{
	S_MemDep_Alias_Info *next,*this;

	for (this=info;this;this=next)
	{
	   next=this->next;
	   L_free(S_MemDep_Alias_Info_pool,this);
	   if (num_alias) (*num_alias)--;
	}
}

void S_print_alias_info(S_MemDep_Alias_Info *info,int pc)
{
        S_MemDep_Alias_Info *this;

	for (this=info;this;this=this->next)
	{
	   fprintf(tmp_file,"%15ld %8d %8d %8d\n",
		(long)oper_tab[pc]->cb->fn,
		pc,this->source_pc,this->alias_times);
	   prof_info[pc].num_aliases++;
	   aliases_in_file++;
	}
}
