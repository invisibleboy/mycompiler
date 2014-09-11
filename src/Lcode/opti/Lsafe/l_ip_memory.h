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
 *  File: l_ip_memory.h
 *
 *  Description:  
 *
 *  Creation Date :  July, 1994
 *
 *  Author:  Roger A. Bringmann and Wen-mei Hwu
 *
 *  Copyright (c) 1994 Roger A. Bringmann , Wen-mei Hwu and The Board of
 *		  Trustees of the University of Illinois. 
 *		  All rights reserved.
 *
 *  The University of Illinois software License Agreement
 *  specifies the terms and conditions for redistribution.
 *
\*****************************************************************************/

#ifndef L_IP_MEMORY
#define L_IP_MEMORY

/* 10/29/02 REK Adding config.h */
#include <config.h>

typedef struct Memory_Cell {
    int			flag;
    L_Operand		*src0;
    L_Operand		*src1;

    Value		*first_value;

    struct Memory_Cell	*next_cell;
} Memory_Cell;

#define RUNTIME_DEF     0
#define LOAD_TIME_DEF   1

/*
 * These defines are used to specify the type of hash table, size of
 * hash table and hash table mask used for local and global memory
 * accesses.
 */
#define		PROGRAM				0
#define		PROGRAM_HASH_TABLE_SIZE		1024	
#define		PROGRAM_HASH_TABLE_MASK		PROGRAM_HASH_TABLE_SIZE-1

#define		FUNCTION			1
#define		FUNCTION_HASH_TABLE_SIZE	64
#define		FUNCTION_HASH_TABLE_MASK	FUNCTION_HASH_TABLE_SIZE-1

#define		HASH				2
#define		HASH_HASH_TABLE_SIZE		256
#define		HASH_HASH_TABLE_MASK		HASH_HASH_TABLE_SIZE-1

typedef struct Memory {
	int		num_entries;

	int		hash_mask;
	Memory_Cell	**hash_table;
} Memory;

#endif
