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
 *      File:   l_hash.h
 *      Author: Teresa Johnson
 *      Creation Date:  1994
 *      Copyright (c) 1994 Teresa Johnson, Wen-mei Hwu and
 *                         The Board of Trustees of the University of Illinois.
 *                         All rights reserved.
 *      The University of Illinois software License Agreement
 *      specifies the terms and conditions for redistribution.
\*****************************************************************************/

#include <config.h>

#define AN_HASH_SIZE 1024

typedef struct L_hash_node
{
	int id;
	L_Attr *attr;
	struct L_hash_node *next;
} L_hash_node;

void L_init_hash(void);
void L_hash_insert_op_attr(L_Attr *attr,int id);
void L_hash_insert_cb_attr(L_Attr *attr,int id);
void L_delete_hash(void);
L_Attr *L_get_cb_attr(int id);
L_Attr *L_get_op_attr(int id);
L_hash_node *L_get_cb_hash_node(int i);
L_hash_node *L_get_op_hash_node(int i);

typedef struct L_position_node
{
	long position;
	struct L_position_node *next;
} L_position_node;

typedef struct L_string_hash_node
{
	char *name;
	int n;
	L_position_node *position;
	struct L_string_hash_node *next;
} L_string_hash_node;

void L_init_string_hash(void);
void L_insert_position(char *name,long position);
L_position_node *L_insert_pos(L_position_node *positions,long position);
int L_get_num_positions(char *name);
long L_get_position(char *name,int i);
void L_delete_string_hash(void);
int L_string_hash(char *str);
L_string_hash_node *L_get_string_hash(int i);
