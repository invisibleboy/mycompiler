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
/*! \file
 * \author Robert Kidd and Wen-mei Hwu
 *
 * This file contains functions to manage data structures needed by Pflatten.
 */

#include <config.h>
#include <Pcode/pcode.h>
#include <Pcode/struct.h>
#include "data.h"

int PF_Indices[ES_LAST];

/*! \brief Allocates a PF_func_data struct.
 *
 * \return
 *  A pointer to a new PF_func_data.
 *
 * Allocates a new PF_func_data or aborts program if allocation fails.
 *
 * \sa PF_free_func_data()
 */
PF_func_data
PF_alloc_func_data (void)
{
  PF_func_data new;

  /* ALLOCATE initializes the block to 0 and will punt if the malloc fails. */
  new = ALLOCATE (_PF_func_data);

  return (new);
}

/*! \brief Frees a PF_func_data struct.
 *
 * \param d
 *  the PF_func_data struct to free.
 *
 * \return
 *  A null PF_func_data pointer.
 *
 * \sa PF_alloc_func_data()
 */
PF_func_data
PF_free_func_data (PF_func_data d)
{
  if (d)
    {
      DISPOSE (d);
      d = NULL;
    }

  return (d);
}

/*! \brief Allocates a PF_expr_data struct.
 *
 * \return
 *  A pointer to a new PF_expr_data.
 *
 * Allocates a new PF_expr_data or aborts program if allocation fails.
 *
 * \sa PF_free_expr_data(), PF_copy_expr_data()
 */
PF_expr_data
PF_alloc_expr_data (void)
{
  PF_expr_data new;

  /* ALLOCATE initializes the block to 0 and will punt if the malloc fails. */
  new = ALLOCATE (_PF_expr_data);

  return (new);
}

/*! \brief Frees a PF_expr_data struct.
 *
 * \param d
 *  the PF_expr_data struct to free.
 *
 * \return
 *  A null PF_expr_data pointer.
 *
 * \sa PF_alloc_expr_data(), PF_copy_expr_data()
 */
PF_expr_data
PF_free_expr_data (PF_expr_data d)
{
  if (d)
    {
      DISPOSE (d);
      d = NULL;
    }

  return (d);
}

/*! \brief Copies a PF_expr_data struct.
 *
 * \param d
 *  the PF_expr_data struct to copy.
 *
 * \return
 *  A copy of the PF_expr_data struct.
 *
 * \sa PF_alloc_expr_data(), PF_free_expr_data()
 */
PF_expr_data
PF_copy_expr_data (PF_expr_data d)
{
  PF_expr_data new = NULL;

  if (d)
    {
      new = PF_alloc_expr_data ();

      new->flags = d->flags;
    }

  return (new);
}

/*! \brief Allocates a new PF_stmt_data struct.
 *
 * \return
 *  A pointer to a new PF_stmt_data.
 *
 * Allocates a new PF_stmt_data or aborts program if allocation fails.
 *
 * \sa PF_free_stmt_data()
 */
PF_stmt_data
PF_alloc_stmt_data (void)
{
  PF_stmt_data new;

  /* ALLOCATE initializes the block to 0 and will punt if the malloc fails. */
  new = ALLOCATE (_PF_stmt_data);

  return (new);
}

/*! \brief Frees a PF_stmt_data struct.
 *
 * \param d
 *  the PF_stmt_data struct to free.
 *
 * \return
 *  A null PF_stmt_data pointer.
 *
 * \sa PF_alloc_stmt_data()
 */
PF_stmt_data
PF_free_stmt_data (PF_stmt_data d)
{
  if (d)
    {
      DISPOSE (d);
      d = NULL;
    }

  return (d);
}
