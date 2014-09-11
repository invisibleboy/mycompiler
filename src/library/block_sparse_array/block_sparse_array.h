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
 *	File:	block_sparse_array.h
 *	Author: Robert Kidd and Wen-mei Hwu
 *      This file contains declarations for routines to manage the Pcode
 *      symbol table.
 *
 *      The sparse array is implemented as a doubly linked list of
 *      blocks that are multiples of BLOCK_SIZE elements long.  It is
 *      intended to efficiently store small clusters of data at widely
 *      separated indices.
 *
 *      Block size is variable, but must be a multiple of BLOCK_SIZE.  As
 *      the array grows, adjacent blocks are merged to keep the length of
 *      the list down.
 *
 *      All access functions (get/set and relatives) take a double pointer
 *      to a BlockSparseArray structure as the argument.  If the function
 *      has to search through the list for a block, it updates the pointer
 *      to point to that block.  Subsequent accesses to elements in that
 *      block will take constant time.
 *
\*****************************************************************************/

#ifndef _LIBRARY_SPARSE_ARRAY_H_
#define _LIBRARY_SPARSE_ARRAY_H_

#include <config.h>
#include <stdio.h>

/* The minimum size of a block in the array. */
#define BLOCK_SIZE 32

/* Flags for the options bitfield. */
/* Warn if the old value in the array is non-zero when writing a new value.
 * This can help catch memory leaks in pointer arrays. */
#define BSA_WARN_ON_OVERWRITE  0x0000000000000001
/* Check that every entry in the array is zero when freeing and warn about
 * non-zero entries.  This can help catch memory leaks in pointer arrays. */
#define BSA_CHECK_NULL_ON_FREE 0x0000000000000002

/* Convert an arbitrary index and length to the values that a block
 * can use (multiples of BLOCK_SIZE). */

/* Pad index i to a block boundary. */
/* If block size is known to be a power of two, you could do something like
 *   i - (i % BLOCK_SIZE) */
#define BSA_BLOCK_INDEX(i)     (i) - ((i) % BLOCK_SIZE)

/* Pad length l from index i to a block boundary.
 * If block size is known to be a power of two, you could do something like
 *   (BLOCK_SIZE - ((i + l - 1) & (~0 >> (64-BLOCK_SIZE))) + l - 1 + \
 *      i - BSA_BLOCK_INDEX(i) */
#define BSA_BLOCK_LENGTH(i, l) (BLOCK_SIZE - (((i) + (l) - 1) % BLOCK_SIZE) + \
                                (l) - 1 + ((i) % BLOCK_SIZE))

typedef struct _BlockSparseArray
{
  int first_index;
  int length;
  long long options;
  struct _BlockSparseArray *next;
  struct _BlockSparseArray *prev;
  void **data;
}
_BlockSparseArray, *BlockSparseArray;

extern BlockSparseArray NewBlockSparseArray ();
extern void FreeBlockSparseArray (BlockSparseArray array);

extern void BlockSparseArraySet (BlockSparseArray *array, int index,
				 void *data);
extern void BlockSparseArrayClear (BlockSparseArray *array, int index);
extern void *BlockSparseArrayGet (BlockSparseArray *array, int index);

/* Returns the index of the first element in the array.  Returns -1 if there
 * are no elements. */
extern int BlockSparseArrayGetFirstIndex (BlockSparseArray *array);
/* Returns the index of the next element after the given index.  Returns -1
 * if the given index is the last element. */
extern int BlockSparseArrayGetNextIndex (BlockSparseArray *array, int index);
/* Returns the index of the first non-zero element in the array.  Returns -1
 * if there is no non-zero element. */
extern int BlockSparseArrayGetFirstNonZeroIndex (BlockSparseArray *array);
/* Returns the index of the next non-zero element after the given index.
 * Returns -1 if the given index is the last non-zero. */
extern int BlockSparseArrayGetNextNonZeroIndex (BlockSparseArray *array,
						int index);

/* Returns the first non-zero element in the array.  Updates the index argument
 * to be the index of this element.  Returns NULL (and sets index to -1) if
 * there are no non-zero elements. */
extern void *BlockSparseArrayGetFirstNonZero (BlockSparseArray *array,
					      int *index);
/* Returns the next non-zero element after the given index.  Updates the
 * index argument to be the index of this element.  Returns NULL (and sets
 * index to -1) if there are no more non-zero elements. */
extern void *BlockSparseArrayGetNextNonZero (BlockSparseArray *array,
					     int *index);

extern void BlockSparseArraySetOption (BlockSparseArray array, int option);
extern void BlockSparseArrayClrOption (BlockSparseArray array, int option);

/* Prepare the array for several set operations.  This will pre-allocate
 * a block for the needed indices.  Calling this function before a series
 * of sets to closely spaced indices will improve efficiency. */
extern void BlockSparseArrayPrepare (BlockSparseArray *array, int index,
				     int length);
/* Remove extraneous blocks (blocks that contain all NULLs).  Sets array
 * to the first block in the list. */
extern void BlockSparseArrayCompact (BlockSparseArray *array);

extern void BlockSparseArrayDump (BlockSparseArray array, FILE *out,
				  void (*print_func)(FILE *out, void *data));

#endif
