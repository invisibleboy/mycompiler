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
 *	File:	block_sparse_array.c
 *	Author: Robert Kidd and Wen-mei Hwu
 *      This file contains definitions for routines to manage the Pcode
 *      symbol table.
 *
 *      The sparse array is implemented as a doubly linked list of
 *      blocks that are multiples of BLOCK_SIZE (defined in sparse_array.h)
 *      elements long.  It is indended to efficiently store small clusters
 *      of data at widely separated indices.
 *
 *      Block size is variable, but must be a multiple of BLOCK_SIZE.  As
 *      the array grows, adjacent blocks are merged to keep the length of
 *      the list down.
 *
\*****************************************************************************/

#include <config.h>
#include <stdlib.h>
#include <malloc.h>
#include <string.h>
#include <stdio.h>
#include <library/i_error.h>
#include "block_sparse_array.h"

static void AllocateData (BlockSparseArray block, int index, int length);
static BlockSparseArray ExtractBlock (BlockSparseArray block, int index,
				      int length);
static void MergeBlocks (BlockSparseArray *array, int index, int length);
static int GetBlock (BlockSparseArray *array, int index);
static void GetOrAddBlock (BlockSparseArray *array, int index, int length);

BlockSparseArray
NewBlockSparseArray ()
{
  BlockSparseArray new;

  new = malloc (sizeof (_BlockSparseArray));

  if (new == NULL)
    I_punt ("block_sparse_array.c:NewBlockSparseArray:%d Could not allocate BlockSparseArray", __LINE__);

  memset (new, 0, sizeof (_BlockSparseArray));

  return (new);
}

void
FreeBlockSparseArray (BlockSparseArray array)
{
  int i;

  /* Find the first block in the list. */
  while (array->prev)
    array = array->prev;

  if (array->data != NULL)
    {
      if (array->options & BSA_CHECK_NULL_ON_FREE)
	for (i = 0; i < array->length; i++)
	  if (array->data[i] != NULL)
	    I_warn ("block_sparse_array.c:FreeBlockSparseArray:%d\n"
		    "data[%d] (array[%d]) is non-null", __LINE__, i,
		    array->first_index + i);

      free (array->data);
      array->data = NULL;
    }

  if (array->next)
    {
      array->next->prev = NULL;

      FreeBlockSparseArray (array->next);

      array->next = NULL;
    }

  free (array);
}

/* Insert a new element into the array.  This function updates the array
 * pointer to point to the block containing the inserted element. */
void
BlockSparseArraySet (BlockSparseArray *array, int index, void *data)
{
  if (array == NULL)
    I_punt ("block_sparse_array.c:BlockSparseArraySet:%d array is NULL",
	    __LINE__);

  GetOrAddBlock (array, index, 1);

  /* Insert the element into the array. */
  if (((*array)->options & BSA_WARN_ON_OVERWRITE) && \
      (*array)->data[index - (*array)->first_index] != NULL)
    I_warn ("block_sparse_array.c:BlockSparseArraySet:%d\n"
	    "overwriting non-null value at index %d (block %d, index %d)",
	    __LINE__, index, (*array)->first_index,
	    index - (*array)->first_index);

  (*array)->data[index - (*array)->first_index] = data;

  return;
}

/* Sets an array element to null.  Does not warn if a non-null entry is
 * being overwritten. */
void
BlockSparseArrayClear (BlockSparseArray *array, int index)
{
  if (array == NULL)
    I_punt ("block_sparse_array.c:BlockSparseArrayClear:%d array is NULL",
	    __LINE__);

  GetOrAddBlock (array, index, 1);

  (*array)->data[index - (*array)->first_index] = NULL;

  return;
}

/* Get an element from the array.  This function updates the array pointer
 * to point to the block containing the accessed element. */
void *
BlockSparseArrayGet (BlockSparseArray *array, int index)
{
  /* Find the block that should contain the index. */
  if (GetBlock (array, index) == 0)
    return (NULL);
  else
    return ((*array)->data[index - (*array)->first_index]);
}

/* Returns the index of the first element in the array.  Returns -1 if there
 * are no elements.  This function updates the array pointer to point to the
 * block containing the first element. */
int
BlockSparseArrayGetFirstIndex (BlockSparseArray *array)
{
  /* Find the first block in the list. */
  while ((*array)->prev)
    (*array) = (*array)->prev;

  if ((*array)->length == 0)
    return (-1);
  else
    return ((*array)->first_index);
}

/* Returns the index of the next element after the given index.  Returns -1
 * if the given index is the last element.  This function updates the array
 * pointer to point to the block containing the returned index. */
int
BlockSparseArrayGetNextIndex (BlockSparseArray *array, int index)
{
  /* Find the block containing the given index. */
  if (GetBlock (array, index) == 0)
    {
      /* End of array */
      return (-1);
    }
  else
    {
      /* If this is the last index in this block, return the first index
       * in the next block. */
      if ((*array)->first_index + (*array)->length == index + 1)
	{
	  if ((*array)->next)
	    {
	      (*array) = (*array)->next;
	      return ((*array)->first_index);
	    }
	  else
	    {
	      return (-1);
	    }
	}
      else
	{
	  return (index + 1);
	}
    }
}

/* Returns the index of the first non-zero element in the array.  Returns -1
 * if there is no non-zero element.  This function updates the array pointer
 * to point to the block containing the first non-zero element. */
int
BlockSparseArrayGetFirstNonZeroIndex (BlockSparseArray *array)
{
  int i;
  BlockSparseArray temp = *array;

  /* Find the first block in the list. */
  while (temp->prev)
    temp = temp->prev;

  while (temp)
    {
      for (i = 0; i < temp->length; i++)
	if (temp->data[i] != NULL)
	  {
	    *array = temp;
	    return (temp->first_index + i);
	  }

      temp = temp->next;
    }

  return (-1);
}

/* Returns the index of the next non-zero element after the given index.
 * Returns -1 if the given index is the last non-zero.  This function updates
 * the array pointer to point to the block containing the returned element. */
int
BlockSparseArrayGetNextNonZeroIndex (BlockSparseArray *array, int index)
{
  int i;
  BlockSparseArray temp;

  /* Find the block containing the given index. */
  if (GetBlock (array, index) == 0)
    {
      /* End of array */
      return (-1);
    }
  else
    {
      temp = *array;

      /* Search the rest of the current block for a non-zero. */
      for (i = index - temp->first_index; i < temp->length; i++)
	if (temp->data[i] != NULL)
	  {
	    *array = temp;
	    return (temp->first_index + i);
	  }

      /* Search the rest of the array for a non-zero. */
      temp = temp->next;

      while (temp)
	{
	  for (i = 0; i < temp->length; i++)
	    if (temp->data[i] != NULL)
	      {
		*array = temp;
		return (temp->first_index + i);
	      }

	  temp = temp->next;
	}
    }

  return (-1);
}

/* Returns the first non-zero element in the array.  Updates the index argument
 * to be the index of this element.  Returns NULL (and sets index to -1) if
 * there are no non-zero elements.  This function updates the array pointer
 * to point to the block containing the returned element. */
void *
BlockSparseArrayGetFirstNonZero (BlockSparseArray *array, int *index)
{
  int i;
  BlockSparseArray temp = *array;

  /* Find the first block in the list. */
  while (temp->prev)
    temp = temp->prev;

  while (temp)
    {
      for (i = 0; i < temp->length; i++)
	if (temp->data[i] != NULL)
	  {
	    *array = temp;
	    *index = temp->first_index + i;
	    return (temp->data[i]);
	  }

      temp = temp->next;
    }

  *index = -1;
  return (NULL);
}

/* Returns the next non-zero element after the given index.  Updates the
 * index argument to be the index of this element.  Returns NULL (and sets
 * index to -1) if there are no more non-zero elements.  This function updates
 * the array pointer to point to the block containing the returned element. */
void *
BlockSparseArrayGetNextNonZero (BlockSparseArray *array, int *index)
{
  int i;
  BlockSparseArray temp;

  /* Find the block containing the given index. */
  if (GetBlock (array, *index) == 0)
    {
      /* End of array */
      *index = -1;
      return (NULL);
    }
  else
    {
      temp = *array;

      /* Search the rest of the current block for a non-zero. */
      for (i = (*index - temp->first_index) + 1; i < temp->length; i++)
	if (temp->data[i] != NULL)
	  {
	    *array = temp;
	    *index = temp->first_index + i;
	    return (temp->data[i]);
	  }

      /* Search the rest of the array for a non-zero. */
      temp = temp->next;

      while (temp)
	{
	  for (i = 0; i < temp->length; i++)
	    if (temp->data[i] != NULL)
	      {
		*array = temp;
		*index = temp->first_index + i;
		return (temp->data[i]);
	      }

	  temp = temp->next;
	}
    }

  *index = -1;
  return (NULL);
}

void
BlockSparseArraySetOption (BlockSparseArray array, int option)
{
  /* Find the first block in the list. */
  while (array->prev)
    array = array->prev;

  while (array)
    {
      array->options |= option;
      array = array->next;
    }
}

void
BlockSparseArrayClrOption (BlockSparseArray array, int option)
{
  /* Find the first block in the list. */
  while (array->prev)
    array = array->prev;

  while (array)
    {
      array->options &= ~option;
      array = array->next;
    }
}

/* Prepare the array for several set operations.  This will pre-allocate
 * a block for the needed indices.  Calling this function before a series
 * of sets to closely spaced indices will improve efficiency.  This function
 * updates the array pointer to point to the prepared block. */
void
BlockSparseArrayPrepare (BlockSparseArray *array, int index, int length)
{
  GetOrAddBlock (array, index, length);

  return;
}

/* Removes blocks that contain all zeros from the array.  This function
 * returns a pointer to the first block in the list. */
void
BlockSparseArrayCompact (BlockSparseArray *array)
{
  BlockSparseArray temp = *array;
  BlockSparseArray tail = NULL;
  BlockSparseArray new, next;
  int i;
  int index = 0, length = 0;
  int index_found;
  int free_node;
  
  while (temp)
    {
      next = temp->next;

      index_found = 0;
      free_node = 0;

      /* Walk through the list node to discover blocks of non-zeros. */
      for (i = 0; i < temp->length; i++)
	{
	  if (temp->data[i] != NULL)
	    {
	      if (index_found == 0)
		{
		  /* This is the first non-zero in a block.  Set index
		   * to the closest block boundary and set length
		   * to one block. */
		  index = BSA_BLOCK_INDEX (temp->first_index + i);
		  length = BLOCK_SIZE;
		  index_found = 1;
		}
	      else
		{
		  /* Three possibilities:
		   * (Note: block refers to the one we're discovering inside
		   *  the list node, not the list node itself)
		   * 1. The next non-zero is in the known block (index, length)
		   *    as the last.
		   *   -Do nothing.
		   * 2. The next non-zero is in the next block (the one
		   *    starting at index+length).
		   *   -Add the next block (of length BLOCK_SIZE) to the known
		   *    block.
		   * 3. The next non-zero is at least one block after the known
		   *    block.
		   *   -Create a new block from the known block, reset index
		   *    and length, continue scanning for non-zeros. */
		  if (index + length <= temp->first_index + i)
		    {
		      if (index + length + BLOCK_SIZE > temp->first_index + i)
			{
			  /* non-zero is in the next block. */
			  length += BLOCK_SIZE;
			}
		      else
			{
			  new = ExtractBlock (temp, index, length);

			  if (tail)
			    {
			      tail->next = new;
			      new->prev = tail;
			    }
			  else
			    {
			      *array = new;
			    }
			  tail = new;

			  /* If ExtractBlock allocated a new node, we can
			   * free temp when we're done. */
			  if (new != temp)
			    free_node = 1;

			  /* Start a new block at the current non-zero. */
			  index = BSA_BLOCK_INDEX (temp->first_index + i);
			  length = BLOCK_SIZE;
			  index_found = 1;
			}
		    }
		}
	    }
	}
      
      /* Copy the last data from the block. */
      if (index_found == 1)
	{
	  new = ExtractBlock (temp, index, length);

	  if (tail)
	    {
	      tail->next = new;
	      new->prev = tail;
	    }
	  else
	    {
	      *array = new;
	    }
	  tail = new;

	  if (new != temp)
	    free_node = 1;
	}

      /* Free the node if we copied part of it inside the for loop or there
       * were no non-zero elements found. */
      if (free_node || index_found == 0)
	{
	  temp->prev = NULL;
	  temp->next = NULL;
	  BlockSparseArrayClrOption (temp, BSA_CHECK_NULL_ON_FREE);
	  FreeBlockSparseArray (temp);
	}

      temp = next;
    }

  return;
}

void
BlockSparseArrayDump (BlockSparseArray array, FILE *out,
		      void (*print_func)(FILE *out, void *data))
{
  int i, j;

  if (array)
    {
      /* Find the first block in the list. */
      while (array->prev)
	array = array->prev;
      
      for (i = 0; array; i++, array = array->next)
	{
#if 0
	  printf ("Block %d, first_index %d, length %d\n", i,
		  array->first_index, array->length);
#endif
	  for (j = 0; j < array->length; j++)
	    {
#if 0
	      printf ("Array element %d (block element %d) = ",
		      array->first_index + j, j);
#endif
	      
	      print_func (out, array->data[j]);
	      
#if 0
	      printf ("\n");
#endif
	    }
	}
    }
  else
    {
#if 0
      printf ("array is null\n");
#endif
    }
}

/* Allocates space for a new block of elements.  Sets the data, first_index,
 * and length fields in the BlockSparseArray structure.  The block allocated
 * by this function will be a multiple of BLOCK_SIZE in length and start
 * at an index which is a multiple of BLOCK_SIZE. */
static void
AllocateData (BlockSparseArray block, int index, int length)
{
  block->first_index = BSA_BLOCK_INDEX (index);
  block->length = BSA_BLOCK_LENGTH (index, length);
  block->data = malloc (sizeof (void *) * block->length);
  if (block->data == NULL)
    I_punt ("block_sparse_array.c:AllocateData:%d Could not allocate memory",
	    __LINE__);

  memset (block->data, 0, sizeof (void *) * block->length);
}

/* Extracts elements from block starting at index with the given length.
 * If index and length match those of the block, block is returned.
 * Otherwise, a new block is allocated and filled with data copied
 * from block.
 * Note: The data copied from block is exactly from index (inclusive) to
 * index + length (exclusive).  The actual block allocated may be padded
 * at the beginning or end. */
static BlockSparseArray
ExtractBlock (BlockSparseArray block, int index, int length)
{
  BlockSparseArray new;

  if (index == block->first_index && length == block->length)
    {
      new = block;
    }
  else
    {
      new = NewBlockSparseArray ();
      new->options = block->options;
      AllocateData (new, index, length);
      memcpy (&(new->data[index - new->first_index]),
	      &(block->data[index - block->first_index]),
	      length * sizeof (void *));
    }

  return (new);
}

/* Merges several blocks into a single large block.  Any elements between
 * index and (index + length - 1), plus those needed to pad index and
 * length to block boundaries, are merged.  The smaller blocks are freed
 * and the array list repaired.  This function updates the array pointer to
 * point to the newly allocated block. */
static void
MergeBlocks (BlockSparseArray *array, int index, int length)
{
  BlockSparseArray first = NULL;
  BlockSparseArray temp = NULL, last = NULL, new = NULL;
  int new_index = 0, new_length = 0;
  int last_index = 0, last_length = 0;

  /* Find the block where the first element should be found. */
  if (GetBlock (array, index) == 0)
    {
      /* If the first block in the merge set doesn't exist yet, find
       * the first block that does exist. */
      if (GetBlock (array, index + BLOCK_SIZE) == 0)
	return;
    }      

  first = *array;

  /* first is the first block in the merge set.  Find the last block in the
   * set. */
  last_index = first->first_index;
  last_length = first->length;
  last = first->next;

  while (last && (last->first_index < index + length))
    {
      last_index = last->first_index;
      last_length = last->length;
      last = last->next;
    }

  /* Note that last is the first block after the merge set, but last_index
   * and last_length are the index and length of the last block in the
   * merge set (ie, last->prev).  This lets us get the last index and length
   * even if last is NULL. */

  new_index = index;
  new_length = length;

  /* The first block may extend before the merge range, so adjust the
   * range to include the entire first block. */
  if (first->first_index <= index)
    {
      new_index = first->first_index;
      new_length = length + (index - new_index);
    }

  /* The last block may extend past the merge range, so adjust the range
   * to include the entire last block. */
  if (last_index + last_length > index + length)
    {
      new_length += (last_index + last_length) - (index + length);
    }

  new = NewBlockSparseArray ();
  if (*array)
    new->options = (*array)->options;
  AllocateData (new, new_index, new_length);

  /* Copy data from blocks in the merge range. */
  if (first)
    {
      new->prev = first->prev;
      new->next = last;

      temp = first;
      while (temp != last)
	{
	  BlockSparseArray next = temp->next;

	  memcpy (&(new->data[temp->first_index - new->first_index]),
		  temp->data, temp->length * sizeof (void *));

	  temp->prev = NULL;
	  temp->next = NULL;
	  BlockSparseArrayClrOption (temp, BSA_CHECK_NULL_ON_FREE);
	  FreeBlockSparseArray (temp);

	  temp = next;
	}

      if (new->next)
	new->next->prev = new;
      if (new->prev)
	new->prev->next = new;
    }

  *array = new;
  return;
}

/* Updates array to point to the block where the given index should
 * be found.  Function returns 1 if the block was found and 0 if the
 * block doesn't exist. */
static int
GetBlock (BlockSparseArray *array, int index)
{
  BlockSparseArray temp = *array;

  /* If the index is in the current block (array), simply return array.
   * Otherwise, find the block and return a pointer. */
  if (temp->first_index > index || \
      temp->first_index + temp->length <= index)
    {
      if (temp->first_index + temp->length <= index)
	{
	  while ((temp->first_index + temp->length <= index) && temp->next)
	    temp = temp->next;
	  
	  if (temp->first_index > index || \
	      ((temp->first_index + temp->length <= index) && \
	       temp->next == NULL))
	    return (0);
	}
      else
	{
	  while (temp->prev && \
		 (temp->prev->first_index + temp->prev->length > index))
	    temp = temp->prev;
	  
	  if ((temp->first_index > index && \
	       (temp->prev == NULL || \
		temp->prev->first_index + temp->prev->length <= index)) || \
	      (temp->first_index + temp->length <= index))
	    return (0);
	}
    }

  *array = temp;
  return (1);
}

/* Updates array to point to the block where the given index should
 * be found.  Adds a new block with enough room for length elements
 * if needed. */
static void
GetOrAddBlock (BlockSparseArray *array, int index, int length)
{
  BlockSparseArray temp;
  BlockSparseArray next = NULL, prev = NULL, new = NULL;
  int initialize_struct_only = 0, merge_blocks = 0;

  /* Check if the block will fit in the current block. */
  if (((*array)->first_index <= index) && \
      ((*array)->first_index + (*array)->length) >= (index + length))
    {
      return;
    }
  else
    {
      temp = *array;

      /* Find the block where the first element should be found. */
      if (temp->first_index + temp->length <= index)
	while ((temp->first_index + temp->length <= index) && temp->next)
	  temp = temp->next;
      else
	while (temp->prev && \
	       (temp->prev->first_index + temp->prev->length > index))
	  temp = temp->prev;
    }

  if (temp->first_index <= index && (temp->first_index + temp->length > index))
    {
      if ((temp->first_index + temp->length) >= (index + length))
	{
	  /* The requested block fits in this block. */
	  *array = temp;
	  return;
	}
      else
	{
	  /* The requested block extends past this block. */
	  MergeBlocks (array, index, length);
	  return;
	}
    }
  else if (temp->first_index > index)
    {
      /* Need new block before temp. */
      if (temp->first_index < index + length)
	{
	  /* The requested block extends into the next existing block. */
	  merge_blocks = 1;
	}

      next = temp;
      prev = temp->prev;
    }
  else if ((temp->first_index + temp->length) <= index && temp->next == NULL)
    {
      /* Need new block after temp. */
      /* Note that this will also catch a new, empty BlockSparseArray
       * structure (first_index=0, length=0).  In that case, we don't add a
       * new node or merge.  We just initialize the empty structure below. */
      if (temp->length == 0)
	{
	  initialize_struct_only = 1;
	}
      else
	{
	  next = temp->next;
	  prev = temp;
	}
    }
  else
    {
      I_punt ("block_sparse_array.c:GetOrAddBlock:%d Cannot find block for index %d", __LINE__);
    }

  /* If this block is adjacent to the previous or next blocks, merge the
   * blocks. */
  if (prev && (prev->first_index + prev->length == BSA_BLOCK_INDEX (index)))
    {
      length = prev->length + BSA_BLOCK_LENGTH (index, length);
      index = prev->first_index;
      merge_blocks = 1;
    }
  if (next && (next->first_index == \
	       BSA_BLOCK_INDEX (index) + BSA_BLOCK_LENGTH (index, length)))
    {
      length = BSA_BLOCK_LENGTH (index, length) + next->length;
      index = BSA_BLOCK_INDEX (index);
      merge_blocks = 1;
    }

  /* If we're doing any merging, MergeBlocks will take care of the
   * initialization. */
  if (merge_blocks)
    {
      MergeBlocks (array, index, length);
      return;
    }

  if (initialize_struct_only == 0)
    {
      new = NewBlockSparseArray ();

      /* Copy the options field from array. */
      if (*array)
	new->options = (*array)->options;
    }
  else
    {
      /* Empty block. */
      new = temp;
    }

  /* Allocates a new data block and sets new's first_index and length
   * fields appropriately. */
  AllocateData (new, index, length);

  if (initialize_struct_only == 0)
    {
      new->next = next;
      new->prev = prev;

      if (prev)
	prev->next = new;
      if (next)
	next->prev = new;
    }

  *array = new;
  return;
}







