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
 * \brief Functions to operate on internal Pcode data structures.
 *
 * \author David August, Nancy Warter, Grant Haab, Krishna Subramanian,
 * and Wen-mei Hwu
 *
 * Modified from code written by Po-hua Chang
 *
 * Copyright (c) 1991 David August, Nancy Warter, Grant Haab, Krishna
 * Subramanian, Po-hua Chang, Wen-mei Hwu, and The Board of Trustees
 * of the University of Illinois.
 * All rights reserved.
 *
 * This file contains all basic functions that operate on the internal Pcode
 * data structures.
 *
 * Groups of functions:
 * \li Memory allocation functions
 * \li Functions to allocate new data structures
 * \li Functions to remove data structures from memory
 * \li Functions to access data in structures
 * \li Copy functions
 */
/*****************************************************************************/

#include <config.h>
#include <stdio.h>
#include <malloc.h>
#include <string.h>
#include <library/i_types.h>
#include <library/i_list.h>
#include <library/i_hash.h>
#include "pcode.h"
#include "struct.h"
#include "struct_symtab.h"
#include "io_util.h"
#include "query.h"
#include "extension.h"
#include "symtab.h"
#include "parloop.h"
#ifdef WIN32
#include <string.h>
#define strdup _strdup
#endif

#undef DEBUG_MEMORY_ALLOC
#undef CHECK_MEMORY_ALLOC

#undef TEST_VARLIST


/*-------------------------------------------------------------*/
/* Memory Allocation Functions                                 */
/*-------------------------------------------------------------*/

/*! The total amount of memory allocated. */
static int total_alloc = 0;

static void append_expr_id (Expr expr, void *data);

/*! \brief Prints the memory used by Pcode.
 *
 * A debugging function to print the amount of memory used for Pcode
 * structures. */
void
P_print_alloc (void)
{
  fprintf (Ferr, "heap space management (%d)\n", total_alloc);
  fflush (Ferr);
}

/*! \brief Allocates and initializes memory
 *
 * \param size
 *  The number of bytes to allocate.
 *
 * \return A pointer to the new block.
 *
 * Allocates a block of \a size bytes.  Functions aborts program if allocation
 * fails.  If allocation succeeds, memory is initialized to 0 and a pointer
 * is returned to caller.
 *
 * \sa P_free()
 */ 
void *
P_alloc (int size)
{
  void *new;

  new = malloc (size);
  if (new == NULL)
    P_punt ("struct.c:P_alloc:%d out of memory space", __LINE__);
  memset (new, 0, size);
  total_alloc += size;

  return (new);
}

/*! \brief Frees memory
 *
 * \param pointer
 *  a pointer to memory allocated with P_alloc().
 * \param size
 *  the size of the memory block
 *
 * Writes zeros to the block and frees it.
 *
 * \sa P_alloc()
 */
void
P_free (void *pointer, int size)
{
  total_alloc -= size;
  if (pointer)
    {
      memset (pointer, 0, size);
      free (pointer);
    }
}

/*-------------------------------------------------------------*/
/* Functions to Allocate New Data Structures                   */
/*-------------------------------------------------------------*/

/*! \brief Allocates a new Key on the heap.
 *
 * \return A pointer to the new Key.
 *
 * Allocates a new Key or aborts program if allocation fails.
 * Initializes the new Key to zeros.
 *
 * \note Keys are typically statically allocated, so this function
 *       is not usually needed.
 *
 * \sa P_NewKeyPWithKey(), #P_RemoveKeyP(), P_FreeKeyP()
 */
Key *
P_NewKeyP (void)
{
  Key *new;

  /* ALLOCATE initailzes the block to 0 and will punt if the malloc fails. */
  new = ALLOCATE (Key);

  return (new);
}

/*! \brief Allocates a new Key on the heap, taking its value from another Key.
 *
 * \param k
 *  the Key to copy.
 *
 * \return
 *  A pointer to a copy of \a k on the heap.
 *
 * \note Keys are typically statically allocated, so this function
 *       is not usually needed.
 *
 * \sa P_NewKeyP(), #P_RemoveKeyP(), P_FreeKeyP()
 */
Key *
P_NewKeyPWithKey (Key k)
{
  Key *new = P_NewKeyP ();

  new->file = k.file;
  new->sym = k.sym;

  return (new);
}

/*! \brief Allocates a new Dcl.
 *
 * \return A pointer to the new Dcl.
 *
 * Allocates a new Dcl or aborts program if allocation fails.
 * Initializes the new Dcl to zeros.
 *
 * \sa P_NewDclWithDclType(), P_FreeDcl(), P_RemoveDcl()
 */
Dcl
P_NewDcl (void)
{
  Dcl new;

  /* ALLOCATE initializes the block to 0 and will punt if the malloc fails. */
  new = ALLOCATE (_Dcl);

  P_SetDclType (new, -1);

  return (new);
}

/* \brief Allocates a new Dcl and inserts a declaration.
 *
 * \param dcl
 *  A declaration to be inserted into the Dcl.
 * \param type
 *  The type of declaration to be inserted.
 *
 * \return A pointer to the new Dcl.
 *
 * Allocates a new Dcl or aborts program if allocation fails.
 * Initializes the new Dcl to zeros and sets the appropriate pointer in the
 * ::Dcl.ptr union to point to dcl.
 *
 * \sa P_NewDcl(), P_FreeDcl(), P_RemoveDcl()
 */
Dcl
P_NewDclWithDclType (void *dcl, _DclType type)
{
  Dcl new = P_NewDcl ();

  P_SetDclType (new, type);

  switch (type)
    {
    case TT_VAR:
      P_SetDclVarDcl (new, (VarDcl)dcl);
      break;
    case TT_STRUCT:
      P_SetDclStructDcl (new, (StructDcl)dcl);
      break;
    case TT_UNION:
      P_SetDclUnionDcl (new, (UnionDcl)dcl);
      break;
    case TT_ENUM:
      P_SetDclEnumDcl (new, (EnumDcl)dcl);
      break;
    case TT_FUNC:
      P_SetDclFuncDcl (new, (FuncDcl)dcl);
      break;
    case TT_TYPE:
      P_SetDclTypeDcl (new, (TypeDcl)dcl);
      break;
    case TT_ASM:
      P_SetDclAsmDcl (new, (AsmDcl)dcl);
      break;
    case TT_INCLUDE:
      P_SetDclInclude (new, (char *)dcl);
      break;
    case TT_SYMBOLTABLE:
      P_SetDclSymbolTable (new, (SymbolTable)dcl);
      break;
    case TT_IPSYMTABENT:
      P_SetDclIPSymTabEnt (new, (IPSymTabEnt)dcl);
      break;
    case TT_SYMTABENTRY:
      P_SetDclSymTabEntry (new, (SymTabEntry)dcl);
      break;
    default:
      P_punt ("struct.c:P_NewDclWithDclType:%d unknown type %d", __LINE__,
	      type);
    }

  return (new);
}

/*! \brief Allocates a new FuncDcl.
 *
 * \return A pointer to the new FuncDcl.
 *
 * Allocates a new FuncDcl or aborts program if allocation fails.
 * Initializes the new FuncDcl to zeros.
 *
 * \sa P_FreeFuncDcl(), P_RemoveFuncDcl()
 */
FuncDcl
P_NewFuncDcl (void)
{
  FuncDcl new;
  int i;

  /* ALLOCATE initializes the block to 0 and will punt if the malloc fails. */
  new = ALLOCATE (_FuncDcl);

  new->name = strdup ("???");
#if 0
  new->depend = P_NewFuncDepend ();
#endif

  if (Handlers[ES_FUNC])
    {
      new->ext = malloc (sizeof (Extension) * NumExtensions[ES_FUNC]);

      if (new->ext == NULL)
	{
	  P_punt ("struct.c:P_NewFuncDcl:%d Could not allocate ext field",
		  __LINE__ - 1);
	}

      for (i = 0; i < NumExtensions[ES_FUNC]; i++)
	{
	  if (Handlers[ES_FUNC][i].alloc && \
	      (Handlers[ES_FUNC][i].options & HO_MANUAL_ALLOC) == 0)
	    new->ext[i] = Handlers[ES_FUNC][i].alloc ();
	  else
	    new->ext[i] = NULL;
	    
	}
    }
      
  return (new);
}

/*! \brief Allocates a new TypeDcl.
 *
 * \return A pointer to the new TypeDcl.
 *
 * Allocates a new TypeDcl or aborts program if allocation fails.
 * Initializes the new TypeDcl to zeros.
 *
 * \sa P_NewTypeDclWithBasicType(), P_FreeTypeDcl(), P_RemoveTypeDcl()
 */
TypeDcl
P_NewTypeDcl (void)
{
  TypeDcl new;
  int i;

  /* ALLOCATE initializes the block to 0 and will punt if the malloc fails. */
  new = ALLOCATE (_TypeDcl);

  if (Handlers[ES_TYPE])
    {
      new->ext = malloc (sizeof (Extension) * NumExtensions[ES_TYPE]);

      if (new->ext == NULL)
	{
	  P_punt ("struct.c:P_NewTypeDcl:%d Could not allocate ext field",
		  __LINE__ - 1);
	}

      for (i = 0; i < NumExtensions[ES_TYPE]; i++)
	{
	  if (Handlers[ES_TYPE][i].alloc && \
	      (Handlers[ES_TYPE][i].options & HO_MANUAL_ALLOC) == 0)
	    new->ext[i] = Handlers[ES_TYPE][i].alloc ();
	  else
	    new->ext[i] = NULL;
	}
    }

  return (new);
}

/*! \brief Allocates a new TypeDcl and sets TypeDcl.basic_type.
 *
 * \param basic_type
 *  the basic type to set.
 *
 * \return A pointer to the new TypeDcl.
 *
 * Allocates a new TypeDcl or aborts program if allocation fails.
 * Initializes the new TypeDcl to zeros and sets ::TypeDcl.basic_type to the
 * basic_type.
 *
 * \sa P_NewTypeDcl(), P_FreeTypeDcl(), P_RemoveTypeDcl()
 */
TypeDcl
P_NewTypeDclWithBasicType (_BasicType basic_type)
{
  TypeDcl new = P_NewTypeDcl ();

  P_SetTypeDclBasicType (new, basic_type);

  return (new);
}

/*! \brief Allocates a new KeyList.
 *
 * \return A pointer to the new KeyList.
 *
 * Allocates a new KeyList or aborts program if allocation fails.
 * Initializes the new KeyList to zeros.
 *
 * \sa P_NewKeyListWithKey(), P_FreeKeyList(), P_RemoveKeyListNode(),
 * P_RemoveKeyList() */
KeyList
P_NewKeyList (void)
{
  KeyList new;

  /* ALLOCATE initializes the block to 0 and will punt if the malloc fails. */
  new = ALLOCATE (_KeyList);

  return (new);
}

/*! \brief Allocates a new KeyList and sets KeyList.key.
 *
 * \param k
 *  the key to set.
 *
 * \return A pointer to the new KeyList.
 *
 * Allocates a new KeyList or aborts program if allocation fails.
 * Initializes the new KeyList to zeros and sets ::KeyList.key to the
 * key.
 *
 * \sa P_NewKeyList(), P_FreeKeyList(), P_RemoveKeyListNode(),
 * P_RemoveKeyList() */
KeyList
P_NewKeyListWithKey (Key k)
{
  KeyList new = P_NewKeyList();

  P_SetKeyListKey (new, k);

  return (new);
}

/*! \brief Allocates a new VarDcl.
 *
 * \return A pointer to the new VarDcl.
 *
 * Allocates a new VarDcl or aborts program if allocation fails.
 * Initializes the new VarDcl to zeros.
 *
 * \sa P_FreeVarDcl(), P_RemoveVarDcl()
 */
VarDcl
P_NewVarDcl (void)
{
  VarDcl new;
  int i;

  /* ALLOCATE initializes the block to 0 and will punt if the malloc fails. */
  new = ALLOCATE (_VarDcl);

  if (Handlers[ES_VAR])
    {
      new->ext = malloc (sizeof (Extension) * NumExtensions[ES_VAR]);

      if (new->ext == NULL)
	{
	  P_punt ("struct.c:P_NewVarDcl:%d Could not allocate ext field",
		  __LINE__ - 1);
	}

      for (i = 0; i < NumExtensions[ES_VAR]; i++)
	{
	  if (Handlers[ES_VAR][i].alloc && \
	      (Handlers[ES_VAR][i].options & HO_MANUAL_ALLOC) == 0)
	    new->ext[i] = Handlers[ES_VAR][i].alloc ();
	  else
	    new->ext[i] = NULL;
	}
    }

  return (new);
}

/*! \brief Allocates a new Init.
 *
 * \return A pointer to the new Init.
 *
 * Allocates a new Init or aborts program if allocation fails.
 * Initializes the new Init to zeros.
 *
 * \sa P_FreeInit(), P_RemoveInitNode(), P_RemoveInit()
 */
Init
P_NewInit (void)
{
  Init new;
  int i;

  /* ALLOCATE initializes the block to 0 and will punt if the malloc fails. */
  new = ALLOCATE (_Init);

  if (Handlers[ES_INIT])
    {
      new->ext = malloc (sizeof (Extension) * NumExtensions[ES_INIT]);

      if (new->ext == NULL)
	{
	  P_punt ("struct.c:P_NewInit:%d Could not allocate ext field",
		  __LINE__ - 1);
	}

      for (i = 0; i < NumExtensions[ES_INIT]; i++)
	{
	  if (Handlers[ES_INIT][i].alloc && \
	      (Handlers[ES_INIT][i].options & HO_MANUAL_ALLOC) == 0)
	    new->ext[i] = Handlers[ES_INIT][i].alloc ();
	  else
	    new->ext[i] = NULL;
	}
    }

  return (new);
}

/*! \brief Allocates a new StructDcl.
 *
 * \return A pointer to the new StructDcl.
 *
 * Allocates a new StructDcl or aborts program if allocation fails.
 * Initializes the new StructDcl to zeros.
 *
 * \sa P_FreeStructDcl(), P_RemoveStructDcl()
 */
StructDcl
P_NewStructDcl (void)
{
  StructDcl new;
  int i;

  /* ALLOCATE initializes the block to 0 and will punt if the malloc fails. */
  new = ALLOCATE (_StructDcl);

  if (Handlers[ES_STRUCT])
    {
      new->ext = malloc (sizeof (Extension) * NumExtensions[ES_STRUCT]);

      if (new->ext == NULL)
	{
	  P_punt ("struct.c:P_NewStructDcl:%d Could not allocate ext field",
		  __LINE__ - 1);
	}

      for (i = 0; i < NumExtensions[ES_STRUCT]; i++)
	{
	  if (Handlers[ES_STRUCT][i].alloc && \
	      (Handlers[ES_STRUCT][i].options & HO_MANUAL_ALLOC) == 0)
	    new->ext[i] = Handlers[ES_STRUCT][i].alloc ();
	  else
	    new->ext[i] = NULL;
	}
    }

  return (new);
}

/*! \brief Allocates a new UnionDcl.
 *
 * \return A pointer to the new UnionDcl.
 *
 * Allocates a new UnionDcl or aborts program if allocation fails.
 * Initializes the new UnionDcl to zeros.
 *
 * \sa P_FreeUnionDcl(), P_RemoveUnionDcl()
 */
UnionDcl
P_NewUnionDcl (void)
{
  UnionDcl new;
  int i;

  /* ALLOCATE initializes the block to 0 and will punt if the malloc fails. */
  new = ALLOCATE (_UnionDcl);

  if (Handlers[ES_UNION])
    {
      new->ext = malloc (sizeof (Extension) * NumExtensions[ES_UNION]);

      if (new->ext == NULL)
	{
	  P_punt ("struct.c:P_NewUnionDcl:%d Could not allocate ext field",
		  __LINE__ - 1);
	}

      for (i = 0; i < NumExtensions[ES_UNION]; i++)
	{
	  if (Handlers[ES_UNION][i].alloc && \
	      (Handlers[ES_UNION][i].options & HO_MANUAL_ALLOC) == 0)
	    new->ext[i] = Handlers[ES_UNION][i].alloc ();
	  else
	    new->ext[i] = NULL;
	}
    }

  return (new);
}

/*! \brief Allocates a new Field.
 *
 * \return A pointer to the new Field.
 *
 * Allocates a new Field or aborts program if allocation fails.
 * Initializes the new Field to zeros.
 *
 * \sa P_FreeField(), P_RemoveFieldNode(), P_RemoveField()
 */
Field
P_NewField (void)
{
  Field new;
  int i;

  /* ALLOCATE initializes the block to 0 and will punt if the malloc fails. */
  new = ALLOCATE (_Field);

  if (Handlers[ES_FIELD])
    {
      new->ext = malloc (sizeof (Extension) * NumExtensions[ES_FIELD]);

      if (new->ext == NULL)
	{
	  P_punt ("struct.c:P_NewField:%d Could not allocate ext field",
		  __LINE__ - 1);
	}

      for (i = 0; i < NumExtensions[ES_FIELD]; i++)
	{
	  if (Handlers[ES_FIELD][i].alloc && \
	      (Handlers[ES_FIELD][i].options & HO_MANUAL_ALLOC) == 0)
	    new->ext[i] = Handlers[ES_FIELD][i].alloc ();
	  else
	    new->ext[i] = NULL;
	}
    }

  return (new);
}

/*! \brief Allocates a new EnumDcl.
 *
 * \return A pointer to the new EnumDcl.
 *
 * Allocates a new EnumDcl or aborts program if allocation fails.
 * Initializes the new EnumDcl to zeros.
 *
 * \sa P_FreeEnumDcl(), P_RemoveEnumDcl()
 */
EnumDcl
P_NewEnumDcl (void)
{
  EnumDcl new;
  int i;

  /* ALLOCATE initializes the block to 0 and will punt if the malloc fails. */
  new = ALLOCATE (_EnumDcl);

  if (Handlers[ES_ENUM])
    {
      new->ext = malloc (sizeof (Extension) * NumExtensions[ES_ENUM]);

      if (new->ext == NULL)
	{
	  P_punt ("struct.c:P_NewEnumDcl:%d Could not allocate ext field",
		  __LINE__ - 1);
	}

      for (i = 0; i < NumExtensions[ES_ENUM]; i++)
	{
	  if (Handlers[ES_ENUM][i].alloc && \
	      (Handlers[ES_ENUM][i].options & HO_MANUAL_ALLOC) == 0)
	    new->ext[i] = Handlers[ES_ENUM][i].alloc ();
	  else
	    new->ext[i] = NULL;
	}
    }

  return (new);
}

/*! \brief Allocates a new EnumField.
 *
 * \return A pointer to the new EnumField.
 *
 * Allocates a new EnumField or aborts program if allocation fails.
 * Initializes the new EnumField to zeros.
 *
 * \sa P_FreeEnumField(), P_RemoveEnumFieldNode(), P_RemoveEnumField()
 */
EnumField
P_NewEnumField (void)
{
  EnumField new;

  /* ALLOCATE initializes the block to 0 and will punt if the malloc fails. */
  new = ALLOCATE (_EnumField);

  return (new);
}

/*! \brief Allocates a new Stmt.
 *
 * \return A pointer to the new Stmt.
 *
 * Allocates a new Stmt or aborts program if allocation fails.
 * Initializes the new Stmt to zeros.
 *
 * \sa P_NewStmtWithType(), P_NewGotoStmt(), P_NewExprStmt(),
 * P_FreeStmt(), P_RemoveStmtNode(), P_RemoveStmt() */
Stmt
P_NewStmt (void)
{
  Stmt new;
  int i;

  /* ALLOCATE initializes the block to 0 and will punt if the malloc fails. */
  new = ALLOCATE (_Stmt);

  if (Handlers[ES_STMT])
    {
      new->ext = malloc (sizeof (Extension) * NumExtensions[ES_STMT]);

      if (new->ext == NULL)
	{
	  P_punt ("struct.c:P_NewStmt:%d Could not allocate ext field",
		  __LINE__ - 1);
	}

      for (i = 0; i < NumExtensions[ES_STMT]; i++)
	{
	  if (Handlers[ES_STMT][i].alloc && \
	      (Handlers[ES_STMT][i].options & HO_MANUAL_ALLOC) == 0)
	    new->ext[i] = Handlers[ES_STMT][i].alloc ();
	  else
	    new->ext[i] = NULL;
	}
    }

  return (new);
}

/*! \brief Allocates a new Stmt and sets the Stmt.type field.
 *
 * \param t
 *  the type of the new stmt.
 *
 * \return
 *  A pointer to a new Stmt of type \a t.
 *
 * Allocates a new Stmt or aborts program if allocation fails.  Initializes
 * the new Stmt to zeros and sets ::Stmt.type to \a t.
 *
 * \sa P_NewStmt(), P_NewGotoStmt(), P_NewExprStmt(), P_FreeStmt(),
 * P_RemoveStmtNode(), P_RemoveStmt() */
Stmt
P_NewStmtWithType (_StmtType t)
{
  Stmt new = P_NewStmt ();

  P_SetStmtType (new, t);

  return (new);
}

/*! \brief Allocates a new goto statement.
 *
 * \param l
 *  the target label of the goto.
 *
 * \return
 *  A pointer to the goto statement.
 *
 * Allocates a new Stmt and sets it up as a goto with \a l as its target.
 *
 * \sa P_NewStmt(), P_NewStmtWithType(), P_NewExprStmt(),
 * P_FreeStmt(), P_RemoveStmtNode(), P_RemoveStmt() */
Stmt
P_NewGotoStmt (Label l)
{
  Stmt new = P_NewStmtWithType (ST_GOTO);

  if (P_GetLabelVal (l))
    P_SetStmtLabelVal (new, strdup (P_GetLabelVal (l)));
  P_SetStmtLabelKey (new, P_GetLabelKey (l));

  return (new);
}

/*! \brief Allocates a new expression statement.
 *
 * \param e
 *  the expression contained within the statement.
 *
 * \return
 *  A pointer to the expression statement.
 *
 * Allocates a new Stmt and sets it up as an expression statement with
 * \a e as its expression.
 *
 * \sa P_NewStmt(), P_NewStmtWithType(), P_NewGotoStmt(),
 * P_FreeStmt(), P_RemoveStmtNode(), P_RemoveStmt() */
Stmt
P_NewExprStmt (Expr e)
{
  Stmt new = P_NewStmtWithType (ST_EXPR);

  P_SetStmtExpr (new, e);

  return (new);
}

/*! \brief Allocates a new Label.
 *
 * \return A pointer to the new Label.
 *
 * Allocates a new Label or aborts program if allocation fails.
 * Initializes the new Label to zeros.
 *
 * \sa P_FreeLabel(), P_RemoveLabelNode(), P_RemoveLabel()
 */
Label
P_NewLabel (void)
{
  Label new;

  /* ALLOCATE initializes the block to 0 and will punt if the malloc fails. */
  new = ALLOCATE (_Label);

  return (new);
}

/*! \brief Allocates a new Compound.
 *
 * \return A pointer to the new Compound.
 *
 * Allocates a new Compound or aborts program if allocation fails.
 * Initializes the new Compound to zeros.
 *
 * \sa P_FreeCompound(), P_RemoveCompound()
 */
Compound
P_NewCompound (void)
{
  Compound new;

  /* ALLOCATE initializes the block to 0 and will punt if the malloc fails. */
  new = ALLOCATE (_Compound);

  return (new);
}

/*! \brief Allocates a new IfStmt.
 *
 * \return A pointer to the new IfStmt.
 *
 * Allocates a new IfStmt or aborts program if allocation fails.
 * Initializes the new IfStmt to zeros.
 *
 * \sa P_FreeIfStmt(), P_RemoveIfStmt()
 */
IfStmt
P_NewIfStmt (void)
{
  IfStmt new;

  /* ALLOCATE initializes the block to 0 and will punt if the malloc fails. */
  new = ALLOCATE (_IfStmt);

  return (new);
}

/*! \brief Allocates a new SwitchStmt.
 *
 * \return A pointer to the new Init.
 *
 * Allocates a new SwitchStmt or aborts program if allocation fails.
 * Initializes the new SwitchStmt to zeros.
 *
 * \sa P_FreeSwitchStmt(), P_RemoveSwitchStmt()
 */
SwitchStmt
P_NewSwitchStmt (void)
{
  SwitchStmt new;

  /* ALLOCATE initializes the block to 0 and will punt if the malloc fails. */
  new = ALLOCATE (_SwitchStmt);

  return (new);
}

/*! \brief Allocates a new Pstmt.
 *
 * \return A pointer to the new Pstmt.
 *
 * Allocates a new Pstmt or aborts program if allocation fails.
 * Initializes the new Pstmt to zeros.
 *
 * \sa P_FreePstmt(), P_RemovePstmt()
 */
Pstmt
P_NewPstmt (void)
{
  Pstmt new;
  int i;

  /* ALLOCATE initializes the block to 0 and will punt if the malloc fails. */
  new = ALLOCATE (_Pstmt);

  if (Handlers[ES_PSTMT])
    {
      new->ext = malloc (sizeof (Extension) * NumExtensions[ES_PSTMT]);

      if (new->ext == NULL)
	{
	  P_punt ("struct.c:P_NewPstmt:%d Could not allocate ext field",
		  __LINE__ - 1);
	}

      for (i = 0; i < NumExtensions[ES_PSTMT]; i++)
	{
	  if (Handlers[ES_PSTMT][i].alloc && \
	      (Handlers[ES_PSTMT][i].options & HO_MANUAL_ALLOC) == 0)
	    new->ext[i] = Handlers[ES_PSTMT][i].alloc ();
	  else
	    new->ext[i] = NULL;
	}
    }

  return (new);
}

/*! \brief Allocates a new Advance.
 *
 * \return A pointer to the new Advance.
 *
 * Allocates a new Advance or aborts program if allocation fails.
 * Initializes the new Advance to zeros.
 *
 * \sa P_FreeAdvance(), P_RemoveAdvance()
 */
Advance
P_NewAdvance (void)
{
  Advance new;

  /* ALLOCATE initializes the block to 0 and will punt if the malloc fails. */
  new = ALLOCATE (_Advance);

  return (new);
}

/*! \brief Allocates a new Await.
 *
 * \return A pointer to the new Await.
 *
 * Allocates a new Await or aborts program if allocation fails.
 * Initializes the new Await to zeros.
 *
 * \sa P_FreeAwait(), P_RemoveAwait()
 */
Await
P_NewAwait (void)
{
  Await new;

  /* ALLOCATE initializes the block to 0 and will punt if the malloc fails. */
  new = ALLOCATE (_Await);

  return (new);
}

/*! \brief Allocates a new Mutex.
 *
 * \return A pointer to the new Mutex.
 *
 * Allocates a new Mutex or aborts program if allocation fails.
 * Initializes the new Mutex to zeros.
 *
 * \sa P_FreeMutex(), P_RemoveMutex()
 */
Mutex
P_NewMutex (void)
{
  Mutex new;

  /* ALLOCATE initializes the block to 0 and will punt if the malloc fails. */
  new = ALLOCATE (_Mutex);

  return (new);
}

/*! \brief Allocates a new Cobegin.
 *
 * \return A pointer to the new Cobegin.
 *
 * Allocates a new Cobegin or aborts program if allocation fails.
 * Initializes the new Cobegin to zeros.
 *
 * \sa P_FreeCobegin(), P_RemoveCobegin()
 */
Cobegin
P_NewCobegin (void)
{
  Cobegin new;

  /* ALLOCATE initializes the block to 0 and will punt if the malloc fails. */
  new = ALLOCATE (_Cobegin);

  return (new);
}

/*! \brief Allocates a new BodyStmt.
 *
 * \return A pointer to the new BodyStmt.
 *
 * Allocates a new BodyStmt or aborts program if allocation fails.
 * Initializes the new BodyStmt to zeros.
 *
 * \sa P_FreeBodyStmt(), P_RemoveBodyStmt()
 */
BodyStmt
P_NewBodyStmt (void)
{
  BodyStmt new;

  /* ALLOCATE initializes the block to 0 and will punt if the malloc fails. */
  new = ALLOCATE (_BodyStmt);

  return (new);
}

/*! \brief Allocates a new EpilogueStmt.
 *
 * \return A pointer to the new EpilogueStmt.
 *
 * Allocates a new EpilogueStmt or aborts program if allocation fails.
 * Initializes the new EpilogueStmt to zeros.
 *
 * \sa P_FreeEpilogueStmt, P_RemoveEpilogueStmt()
 */
EpilogueStmt
P_NewEpilogueStmt (void)
{
  EpilogueStmt new;

  /* ALLOCATE initializes the block to 0 and will punt if the malloc fails. */
  new = ALLOCATE (_EpilogueStmt);

  return (new);
}

/*! \brief Allocates a new ParLoop.
 *
 * \return A pointer to the new ParLoop.
 *
 * Allocates a new ParLoop or aborts program if allocation fails.
 * Initializes the new ParLoop to zeros.
 *
 * \sa P_FreeParLoop(), P_RemoveParLoopNode(), P_RemoveParLoop()
 */
ParLoop
P_NewParLoop (void)
{
  ParLoop new;

  /* ALLOCATE initializes the block to 0 and will punt if the malloc fails. */
  new = ALLOCATE (_ParLoop);

  return (new);
}

/*! \brief Allocates a new SerLoop.
 *
 * \return A pointer to the new SerLoop.
 *
 * Allocates a new SerLoop or aborts program if allocation fails.
 * Initializes the new SerLoop to zeros.
 *
 * \sa P_FreeSerLoop(), P_RemoveSerLoop()
 */
SerLoop
P_NewSerLoop (void)
{
  SerLoop (new);

  /* ALLOCATE initializes the block to 0 and will punt if the malloc fails. */
  new = ALLOCATE (_SerLoop);

  return (new);
}

/*! \brief Allocates a new AsmStmt.
 *
 * \return A pointer to the new AsmStmt.
 *
 * Allocates a new AsmStmt or aborts program if allocation fails.
 * Initializes the new AsmStmt to zeros.
 *
 * \sa P_FreeAsmStmt(), P_RemoveAsmStmt()
 */
AsmStmt
P_NewAsmStmt (void)
{
  AsmStmt new;

  /* ALLOCATE initializes the block to 0 and will punt if the malloc fails. */
  new = ALLOCATE (_AsmStmt);

  return (new);
}

/*! \brief Allocates a new Asmoprd.
 *
 * \return A pointer to the new Asmoprd.
 *
 * Allocates a new Asmoprd or aborts program if allocation fails.
 * Initializes the new Asmoprd to zeros.
 *
 * \sa P_FreeAsmoprd(), P_RemoveAsmoprd()
 */
Asmoprd
P_NewAsmoprd ()
{
  Asmoprd new;

  /* ALLOCATE initializes the block to 0 and will punt if the malloc fails. */
  new = ALLOCATE (_Asmoprd);

  return (new);
}

/*! \brief Allocates a new Expr.
 *
 * \return A pointer to the new Expr.
 *
 * Allocates a new Expr or aborts program if allocation fails.
 * Initializes the new Expr to zeros.
 *
 * \note Callers using P_NewExpr() from the old Pcode library should now use
 * P_NewExprWithOpcode().
 *
 * \sa P_NewExprWithOpcode(), P_NewStringExpr(), P_NewIntExpr(),
 * P_NewUIntExpr(), P_NewFloatExpr(), P_NewDoubleExpr(), P_FreeExpr(),
 * P_RemoveExprNode(), P_RemoveExpr() */
Expr
P_NewExpr ()
{
  Expr new;
  int i;

  /* ALLOCATE initializes the block to 0 and will punt if the malloc fails. */
  new = ALLOCATE (_Expr);

  if (Handlers[ES_EXPR])
    {
      new->ext = malloc (sizeof (Extension) * NumExtensions[ES_EXPR]);

      if (new->ext == NULL)
	{
	  P_punt ("struct.c:P_NewExpr:%d Could not allocate ext field",
		  __LINE__ - 1);
	}

      for (i = 0; i < NumExtensions[ES_EXPR]; i++)
	{
	  if (Handlers[ES_EXPR][i].alloc && \
	      (Handlers[ES_EXPR][i].options & HO_MANUAL_ALLOC) == 0)
	    new->ext[i] = Handlers[ES_EXPR][i].alloc ();
	  else
	    new->ext[i] = NULL;
	}
    }

  return (new);
}

/*! \brief Allocates a new Expr and sets the opcode field.
 *
 * \param o
 *  The opcode for the new Expr.
 *
 * \return A pointer to the new Expr.
 *
 * Allocates a new Expr or aborts program if allocation fails.
 * Initializes the new Expr to zeros and sets ::Expr.opcode to \a o.
 *
 * \note Callers using P_NewExpr() from the old Pcode library should now use
 * this function.
 *
 * \sa P_NewExpr(), P_NewStringExpr(), P_NewIntExpr() ,
 * P_NewFloatExpr(), P_NewDoubleExpr(), P_FreeExpr(),
 * P_RemoveExprNode(), P_RemoveExpr()
 * */
Expr
P_NewExprWithOpcode (_Opcode o)
{
  Expr new = P_NewExpr ();

  P_SetExprOpcode (new, o);

  return (new);
}

/*! \brief Allocates a new Expr for a string literal.
 *
 * \param s
 *  The string value for the Expr.
 *
 * \return A pointer to the new Expr.
 *
 * Allocates a new Expr or aborts program if allocation fails.
 * Initializes the new Expr to zeros and sets ::Expr.value.string to
 * the string.
 *
 * \note This function copies the string passed as \a s.  The caller must
 * free its copy after the call.
 *
 * \sa P_NewExpr(), P_NewExprWithOpcode(), P_NewIntExpr(),
 * P_NewUIntExpr(), P_NewFloatExpr(), P_NewDoubleExpr(), P_FreeExpr()
 * P_RemoveExprNode(), P_RemoveExpr() */
Expr
P_NewStringExpr (char *s)
{
  Expr new = P_NewExprWithOpcode (OP_string);

  P_SetExprString (new, strdup (s));

  return (new);
}

/* \brief Allocates a new Expr for an integer literal.
 *
 * \param i
 *  The integer value for the Expr.
 *
 * \return A pointer to the new Expr.
 *
 * Allocates a new Expr or aborts program if allocation fails.
 * Initializes the new Expr to zeros and sets ::Expr.value.scalar to
 * the integer.
 *
 * \sa P_NewExpr(), P_NewExprWithOpcode(), P_NewStringExpr(),
 * P_NewUIntExpr(), P_NewFloatExpr(), P_NewDoubleExpr(), P_FreeExpr(),
 * P_RemoveExprNode(), P_RemoveExpr() */
Expr
P_NewIntExpr (ITintmax i)
{
  Expr new = P_NewExprWithOpcode (OP_int);

  P_SetExprScalar (new, i);

  return (new);
}

/* \brief Allocates a new Expr for an unsigned integer literal.
 *
 * \param i
 *  The integer value for the Expr.
 *
 * \return A pointer to the new Expr.
 *
 * Allocates a new Expr or aborts program if allocation fails.
 * Initializes the new Expr to zeros and sets ::Expr.value.uscalar to
 * the integer.
 *
 * \sa P_NewExpr(), P_NewExprWithOpcode(), P_NewStringExpr(),
 * P_NewIntExpr(), P_NewFloatExpr(), P_NewDoubleExpr(), P_FreeExpr(),
 * P_RemoveExprNode(), P_RemoveExpr() */
Expr
P_NewUIntExpr (ITuintmax i)
{
  Expr new = P_NewExprWithOpcode (OP_int);

  P_SetExprFlags (new, EF_UNSIGNED);
  P_SetExprUScalar (new, i);

  return (new);
}

/*! \brief Allocates a new Expr for a float literal.
 *
 * \param f
 *  The float value for the Expr.
 *
 * \return A pointer to the new Expr.
 *
 * Allocates a new Expr or aborts program if allocation fails.
 * Initializes the new Expr to zeros and sets ::Expr.value.real to
 * the float.
 *
 * \sa P_NewExpr(), P_NewExprWithOpcode(), P_NewStringExpr(),
 * P_NewIntExpr(), P_NewUIntExpr(), P_NewDoubleExpr(), P_FreeExpr(),
 * P_RemoveExprNode(), P_RemoveExpr()
 * */
Expr
P_NewFloatExpr (double f)
{
  Expr new = P_NewExprWithOpcode (OP_float);

  P_SetExprReal (new, f);

  return (new);
}

/*! \brief Allocates a new Expr for a double literal.
 *
 * \param d
 *  The double value for the Expr.
 *
 * \return A pointer to the new Expr.
 *
 * Allocates a new Expr or aborts program if allocation fails.
 * Initializes the new Expr to zeros and sets ::Expr.value.real to
 * the double.
 *
 * \sa P_NewExpr(), P_NewExprWithOpcode(), P_NewStringExpr(),
 * P_NewIntExpr(), P_NewUIntExpr(), P_NewFloatExpr(), P_FreeExpr(),
 * P_RemoveExprNode(), P_RemoveExpr()
 * */
Expr
P_NewDoubleExpr (double d)
{
  Expr new = P_NewExprWithOpcode (OP_double);

  P_SetExprReal (new, d);

  return (new);
}

/*! \brief Returns the parent statement for an Expr.
 *
 * \param expr
 *  the Expr to inspect.
 *
 * \return
 *  The parent statement for the Expr.
 *
 * Walks up the Expr tree to find the parent statement.
 */
Stmt
P_ExprParentStmt (Expr expr)
{
  while (expr && P_GetExprParentStmt (expr) == NULL)
    expr = P_GetExprParentExpr (expr);

  if (expr == NULL)
    P_punt ("struct.c:P_ExprParentStmt:%d Expr parent is NULL", __LINE__);

  return (P_GetExprParentStmt (expr));
}

/*! \brief Swaps two expressions.
 *
 * \param a
 *  the first expression to swap.
 * \param b
 *  the second expression to swap.
 *
 * Swaps expressions \a a and \a b so that \a b takes \a a's place in
 * the expression tree and vice versa.  Expressions below \a a or \a b
 * (operands) are swapped with their parent.
 *
 * This function swaps the contents of the two Exprs.  Any pointers
 * pointing to \a a will still hold \a a's address, but \a a will contain
 * the contents of \a b.
 *
 * This function also swaps the addresses stored in *a and *b.  The result
 * of this function is that a takes b's place, and vice versa, and the
 * caller's variables that hold a and b still refer to their original content.
 *
 * If \a a is an OP_var and \a b is an OP_int, after calling this function,
 * the OP_var will be in the OP_int's position in the parse tree and vice
 * versa.  \a a will still refer to the OP_var and \a b will still refer
 * to the OP_int.
 *
 * \note This function moves \a a and \a b in the expression tree, so
 *       use caution when using this function inside a loop.
 */
void
P_ExprSwap (Expr *a, Expr *b)
{
  _Expr tmp;
  Expr tmp_p;

  tmp.id = (*a)->id;
  tmp.status = (*a)->status;
  tmp.key = (*a)->key;
  tmp.opcode = (*a)->opcode;
  tmp.flags = (*a)->flags;
  tmp.type = (*a)->type;

  switch ((*a)->opcode)
    {
    case OP_int:
      if ((*a)->flags & EF_UNSIGNED)
	tmp.value.uscalar = (*a)->value.uscalar;
      else
	tmp.value.scalar = (*a)->value.scalar;
      break;
    case OP_real:
    case OP_float:
    case OP_double:
      tmp.value.real = (*a)->value.real;
      break;
    case OP_char:
    case OP_string:
      tmp.value.string = (*a)->value.string;
      break;
    case OP_var:
    case OP_dot:
    case OP_arrow:
      tmp.value.var.name = (*a)->value.var.name;
      tmp.value.var.key = (*a)->value.var.key;
      break;
    case OP_type_size:
      tmp.value.type = (*a)->value.type;
      break;
    case OP_stmt_expr:
      tmp.value.stmt = (*a)->value.stmt;
      break;
    case OP_asm_oprd:
      tmp.value.asmoprd = (*a)->value.asmoprd;
      break;
    default:
      break;
    }
  
  /* (*a)->sibling, parentexpr, parentstmt, next, previous aren't swapped, as 
   * they locate a in the expression tree. */
  tmp.operands = (*a)->operands;
  tmp.pragma = (*a)->pragma;

  (*a)->id = (*b)->id;
  (*a)->status = (*b)->status;
  (*a)->key = (*b)->key;
  (*a)->opcode = (*b)->opcode;
  (*a)->flags = (*b)->flags;
  (*a)->type = (*b)->type;

  switch ((*b)->opcode)
    {
    case OP_int:
      if ((*b)->flags & EF_UNSIGNED)
	(*a)->value.uscalar = (*b)->value.uscalar;
      else
	(*a)->value.scalar = (*b)->value.scalar;
      break;
    case OP_real:
    case OP_float:
    case OP_double:
      (*a)->value.real = (*b)->value.real;
      break;
    case OP_char:
    case OP_string:
      (*a)->value.string = (*b)->value.string;
      break;
    case OP_var:
    case OP_dot:
    case OP_arrow:
      (*a)->value.var.name = (*b)->value.var.name;
      (*a)->value.var.key = (*b)->value.var.key;
      break;
    case OP_type_size:
      (*a)->value.type = (*b)->value.type;
      break;
    case OP_stmt_expr:
      (*a)->value.stmt = (*b)->value.stmt;
      break;
    case OP_asm_oprd:
      (*a)->value.asmoprd = (*b)->value.asmoprd;
      break;
    default:
      break;
    }
  
  (*a)->operands = (*b)->operands;
  (*a)->pragma = (*b)->pragma;

  (*b)->id = tmp.id;
  (*b)->status = tmp.status;
  (*b)->key = tmp.key;
  (*b)->opcode = tmp.opcode;
  (*b)->flags = tmp.flags;
  (*b)->type = tmp.type;

  switch (tmp.opcode)
    {
    case OP_int:
      if (tmp.flags & EF_UNSIGNED)
	(*b)->value.uscalar = tmp.value.uscalar;
      else
	(*b)->value.scalar = tmp.value.scalar;
      break;
    case OP_real:
    case OP_float:
    case OP_double:
      (*b)->value.real = tmp.value.real;
      break;
    case OP_char:
    case OP_string:
      (*b)->value.string = tmp.value.string;
      break;
    case OP_var:
    case OP_dot:
    case OP_arrow:
      (*b)->value.var.name = tmp.value.var.name;
      (*b)->value.var.key = tmp.value.var.key;
      break;
    case OP_type_size:
      (*b)->value.type = tmp.value.type;
      break;
    case OP_stmt_expr:
      (*b)->value.stmt = tmp.value.stmt;
      break;
    case OP_asm_oprd:
      (*b)->value.asmoprd = tmp.value.asmoprd;
      break;
    default:
      break;
    }

  (*b)->operands = tmp.operands;
  (*b)->pragma = tmp.pragma;

  tmp_p = *a;
  *a = *b;
  *b = tmp_p;

  return;
}

/*! \brief Generates a mapping of Expression IDs between two expressions.
 *
 * \param orig
 *  the original expression.
 * \param copy
 *  the copy of \a orig.
 *
 * \return
 *  A mapping of Expr IDs in \a orig to the corresponding Expr IDs in \a copy.
 *
 * The mapping is returned in a HashTable, where Expr IDs in \a orig key
 * the corresponding ID in \a copy.
 */
HashTable
P_ExprBuildExprMap (Expr orig, Expr copy)
{
  HashTable result = HashTable_create (256);
  List orig_ids = NULL, copy_ids = NULL;
  int orig_id, copy_id;

  P_ExprApply (orig, NULL, append_expr_id, &orig_ids);
  P_ExprApply (copy, NULL, append_expr_id, &copy_ids);

  /* Add -1 to the end of both lists so we know when we see the last ID.
   * (0 is a valid Expr ID). */
  orig_ids = List_insert_last (orig_ids, (void *)-1);
  copy_ids = List_insert_last (copy_ids, (void *)-1);

  List_start (orig_ids);
  List_start (copy_ids);
  /* Make sure we pull the next list before doing the logical comparison
   * to make sure that the short-circuit doesn't prevent us from pulling
   * the second list. */
#ifdef LP64_ARCHITECTURE
  while (orig_id = (int)((long)List_next (orig_ids)), \
	 copy_id = (int)((long)List_next (copy_ids)), \
	 orig_id != -1 && copy_id != -1)
    HashTable_insert (result, orig_id, (void *)(long)copy_id);
#else
  while (orig_id = (int)List_next (orig_ids), \
	 copy_id = (int)List_next (copy_ids), \
	 orig_id != -1 && copy_id != -1)
    HashTable_insert (result, orig_id, (void *)copy_id);
#endif

  if ((orig_id != -1) ^ (copy_id != -1))
    P_punt ("struct.c:P_ExprBuildExprMap:%d Expr trees differ", __LINE__);
    
  return (result);
}

/*! \brief Allocates a new Pragma.
 *
 * \return A pointer to the new Pragma.
 *
 * Allocates a new Pragma or aborts program if allocation fails.
 * Initializes the new Pragma to zeros.
 *
 * \note Callers using P_NewPragma() from the old Pcode library should now use
 * P_NewPragmaWithSpecExpr().
 *
 * \sa P_NewPragmaWithSpecExpr(), P_FreePragma(), P_RemovePragmaNode(),
 * P_RemovePragma()
 */
Pragma
P_NewPragma ()
{
  Pragma new;

  /* ALLOCATE initializes the block to 0 and will punt if the malloc fails. */
  new = ALLOCATE (__Pragma);

  return (new);
}

/*! \brief Allocates a new Pragma and sets the specifier and expression.
 *
 * \param s
 *  the specifier (pragma identifier string).
 * \param e
 *  the expression (pragma value).
 *
 * \return A pointer to the new Pragma.
 *
 * Allocates a new Pragma or aborts program if allocation fails.
 * Initializes the new Pragma to zeros and sets ::Pragma.specifier
 * to the specifier and ::Pragma.expr to the expression.
 *
 * \note Callers using P_NewPragma() from the old Pcode library should now use
 * P_NewPragmaWithSpecExpr().
 *
 * \note This function copies the string \a s.  The caller must free its
 * copy after calling this function.
 *
 * \sa P_NewPragma(), P_FreePragma, P_RemovePragmaNode(), P_RemovePragma()
 */
Pragma
P_NewPragmaWithSpecExpr (char *s, Expr e)
{
  Pragma new = P_NewPragma ();

  P_SetPragmaSpecifier (new, strdup (s));
  P_SetPragmaExpr (new, e);

  return (new);
}

void
P_AddStmtPragma (Pragma *prag, char *spec, Expr expr)
{
  Pragma new = P_NewPragmaWithSpecExpr (spec, expr);
  new->next = *prag;
  *prag = new;
}

/*! \brief Allocates a new Position.
 *
 * \return A pointer to the new Position.
 *
 * Allocates a new Position or aborts program if allocation fails.
 * Initializes the new Position to zeros.
 *
 * \sa P_FreePosition(), P_RemovePosition()
 */
Position
P_NewPosition ()
{
  Position new;

  /* ALLOCATE initializes the block to 0 and will punt if the malloc fails. */
  new = ALLOCATE (_Position);

  return (new);
}

/*! \brief Allocates a new Identifier.
 *
 * \return A pointer to the new Identirfier.
 *
 * Allocates a new Identifier or aborts program if allocation fails.
 * Initializes the new Identifier to zeros.
 *
 * \sa P_FreeIdentifer(), P_RemoveIdentifier()
 */
Identifier
P_NewIdentifier ()
{
  Identifier new;

  /* ALLOCATE initializes the block to 0 and will punt if the malloc fails. */
  new = ALLOCATE (_Identifier);

  /* Set the key to an invalid value. */
  new->key.file = -1;
  new->key.sym = -1;

  return (new);
}

/* Routine to create a new profile statement (SK) */

/* LCW - Allocate space for a function profile - 12/23/95 */
/*! \brief Allocates a new ProfFN.
 *
 * \return A pointer to the new ProfFN.
 *
 * Allocates a new ProfFN or aborts program if allocation fails.
 * Initializes the new ProfFN to zeros.
 *
 * \sa P_FreeProfFN(), P_RemoveProfFN()
 */
ProfFN
P_NewProfFN ()
{
  ProfFN new;

  /* ALLOCATE initializes the block to 0 and will punt if the malloc fails. */
  new = ALLOCATE (_ProfFN);
  new->count = 0.0;
  return (new);
}

/* LCW - copy the following routines from Hcode - 12/23/95 */
/*! \brief Allocates a new ProfCS.
 *
 * \return A pointer to the new ProfCS.
 *
 * Allocates a new ProfCS or aborts program if allocation fails.
 * Initializes the new ProfCS to zeros.
 *
 * \sa P_FreeProfCS(), P_RemoveProfCSNode(), P_RemoveProfCS()
 */
ProfCS
P_NewProfCS ()
{
  ProfCS new;

  /* ALLOCATE initializes the block to 0 and will punt if the malloc fails. */
  new = ALLOCATE (_ProfCS);

  return (new);
}

/* LCW - Allocate space for a basic block profile - 12/23/95 */
/*! \brief Allocates a new ProfBB.
 *
 * \return A pointer to the new ProfBB.
 *
 * Allocates a new ProfBB or aborts program if allocation fails.
 * Initializes the new ProfBB to zeros.
 *
 * \sa P_FreeProfBB(), P_RemoveProfBB()
 */
ProfBB
P_NewProfBB ()
{
  ProfBB new;

  /* ALLOCATE initializes the block to 0 and will punt if the malloc fails. */
  new = ALLOCATE (_ProfBB);

  return (new);
}

/*! \brief Allocates a new ProfArc.
 *
 * \return A pointer to the new ProfArc.
 *
 * Allocates a new ProfArc or aborts program if allocation fails.
 * Initializes the new ProfArc to zeros.
 *
 * \sa P_FreeProfArc(), P_RemoveProfArcNode(), P_RemoveProfArc()
 */
ProfArc
P_NewProfArc ()
{
  ProfArc new;

  /* ALLOCATE initializes the block to 0 and will punt if the malloc fails. */
  new = ALLOCATE (_ProfArc);

  return (new);
}

/* LCW - modified for the new profile structure - 10/25/95 */
/*! \brief Allocates a new ProfST.
 *
 * \return A pointer to the new ProfST.
 *
 * Allocates a new ProfST or aborts program if allocation fails.
 * Initializes the new ProfST to zeros.
 *
 * \sa P_FreeProfST(), P_RemoveProfSTNode(), P_RemoveProfST()
 */
ProfST
P_NewProfST ()
{
  ProfST new;

  /* ALLOCATE initializes the block to 0 and will punt if the malloc fails. */
  new = ALLOCATE (_ProfST);
  new->count = 0.0;
  return (new);
}

ProfST
P_NewProfST_w_wt (double wt)
{
  ProfST new;

  /* ALLOCATE initializes the block to 0 and will punt if the malloc fails. */
  new = ALLOCATE (_ProfST);
  new->count = wt;
  return (new);
}

/*! \brief Allocates a new ProfEXPR.
 *
 * \return A pointer to the new ProfEXPR.
 *
 * Allocates a new ProfEXPR or aborts program if allocation fails.
 * Initializes the new ProfEXPR to zeros.
 *
 * \sa P_FreeProfEXPR(), P_RemoveProfEXPRNode(), P_RemoveProfEXPR()
 */
ProfEXPR
P_NewProfEXPR ()
{
  ProfEXPR new;

  /* ALLOCATE initializes the block to 0 and will punt if the malloc fails. */
  new = ALLOCATE (_ProfEXPR);
  new->count = 0.0;
  return (new);
}

ProfEXPR
P_NewProfEXPR_w_wt (double wt)
{
  ProfEXPR new;

  /* ALLOCATE initializes the block to 0 and will punt if the malloc fails. */
  new = ALLOCATE (_ProfEXPR);
  new->count = wt;  
  return (new);
}

/*! \brief Allocates a new Shadow.
 *
 * \return A pointer to the new Shadow.
 *
 * Allocates a new Shadow or aborts program if allocation fails.
 * Initializes the new Shadow to zeros.
 *
 * \sa P_FreeShadow(), P_RemoveShadow()
 */
Shadow
P_NewShadow ()
{
  Shadow new;

  /* ALLOCATE initializes the block to 0 and will punt if the malloc fails. */
  new = ALLOCATE (_Shadow);

  return (new);
}

/*! \brief Allocates a new Shadow.
 *
 * *** Added for quick port of Pflatten ***
 */
Shadow
P_NewShadowWithExprID (Shadow slist, Expr expr, int id)
{
  Shadow new_shadow;
  Shadow tmp;

  new_shadow = calloc(1,sizeof(_Shadow));
  new_shadow->param_id = id;
  new_shadow->expr = expr;

  if (slist == NULL)
    slist = new_shadow;
  else if (slist->param_id > id)
    {
      new_shadow->next = slist;
      slist = new_shadow;
    }
  else
    {
      tmp = slist;
      while (tmp->next && (id > tmp->next->param_id))
        tmp = tmp->next;

      new_shadow->next = tmp->next;
      tmp->next = new_shadow;
    }

  return slist;
}

/*! \brief Allocates a new AsmDcl.
 *
 * \return A pointer to the new AsmDcl.
 *
 * Allocates a new AsmDcl or aborts program if allocation fails.
 * Initializes the new AsmDcl to zeros.
 *
 * \sa P_FreeAsmDcl(), P_RemoveAsmDcl()
 */
AsmDcl
P_NewAsmDcl ()
{
  AsmDcl new;
  int i;

  /* ALLOCATE initializes the block to 0 and will punt if the malloc fails. */
  new = ALLOCATE (_AsmDcl);

  if (Handlers[ES_ASM])
    {
      new->ext = malloc (sizeof (Extension) * NumExtensions[ES_ASM]);

      if (new->ext == NULL)
	{
	  P_punt ("struct.c:P_NewAsmDcl:%d Could not allocate ext field",
		  __LINE__ - 1);
	}

      for (i = 0; i < NumExtensions[ES_ASM]; i++)
	{
	  if (Handlers[ES_ASM][i].alloc && \
	      (Handlers[ES_ASM][i].options & HO_MANUAL_ALLOC) == 0)
	    new->ext[i] = Handlers[ES_ASM][i].alloc ();
	  else
	    new->ext[i] = NULL;
	}
    }

  return (new);
}

/*! \brief Allocates a new Scope.
 *
 * \return A pointer to the new Scope.
 *
 * Allocates a new Scope or aborts program if allocation fails.
 * Initializes the new Scope to zeros.
 *
 * \sa P_NewScopeWithKey(), P_FreeScope(), P_RemoveScope()
 */
Scope
P_NewScope ()
{
  Scope new;

  /* ALLOCATE initializes the block to 0 and will punt if the malloc fails. */
  new = ALLOCATE (_Scope);

  return (new);
}

/*! \brief Allocates a new Scope and sets the keys.
 *
 * \param key
 *  the key for the new Scope.
 *
 * \return A pointer to the new Scope.
 *
 * Allocates a new Scope or aborts program if allocation fails.
 * Initializes the new Scope to zeros and sets ::Scope.file_key and
 * ::Scope.key to the keys.
 *
 * \sa P_NewScope(), P_FreeScope(), P_RemoveScope()
 */
Scope
P_NewScopeWithKey (Key key)
{
  Scope new = P_NewScope ();

  P_SetScopeKey (new, key);

  return (new);
}

/*! \brief Initailizes a new Key.
 *
 * \return A Key with both fields initialized to 0.
 *
 * \note This function returns a Key, not a pointer to a Key allocated
 * on the heap.
 */
Key
P_NewKey ()
{
  Key new = Invalid_Key;

  return (new);
}

/*! \brief Allocates a new SymTabEntry.
 *
 * \return A pointer to the new SymTabEntry.
 *
 * Allocates a new SymTabEntry or aborts program if allocation fails.
 * Initializes the new SymTabEntry to zeros.
 *
 * \sa P_FreeSymTabEntry(), P_RemoveSymTabEntry()
 */
SymTabEntry
P_NewSymTabEntry ()
{
  SymTabEntry new;
  int i;

  /* ALLOCATE initializes the block to 0 and will punt if the malloc fails. */
  new = ALLOCATE (_SymTabEntry);

  if (Handlers[ES_SYMTABENTRY])
    {
      new->ext = malloc (sizeof (Extension) * NumExtensions[ES_SYMTABENTRY]);

      if (new->ext == NULL)
	{
	  P_punt ("struct.c:P_NewSymTabEntry:%d Could not allocate ext field",
		  __LINE__ - 1);
	}

      for (i = 0; i < NumExtensions[ES_SYMTABENTRY]; i++)
	{
	  if (Handlers[ES_SYMTABENTRY][i].alloc && \
	      (Handlers[ES_SYMTABENTRY][i].options & HO_MANUAL_ALLOC) == 0)
	    new->ext[i] = Handlers[ES_SYMTABENTRY][i].alloc ();
	  else
	    new->ext[i] = NULL;
	}
    }

  return (new);
}

/*! \brief Allocates a new IPSymTabEnt.
 *
 * \return A pointer to the new IPSymTabEnt.
 *
 * Allocates a new IPSymTabEnt or aborts program if allocation fails.
 * Initializes the new IPSymTabEnt to zeros.
 *
 * \sa P_FreeIPSymTabEnt(), P_RemoveIPSymTabEnt()
 */
IPSymTabEnt
P_NewIPSymTabEnt ()
{
  IPSymTabEnt new;
  int i;

  /* ALLOCATE initializes the block to 0 and will punt if the malloc fails. */
  new = ALLOCATE (_IPSymTabEnt);

  /* Set up the default file status. */
  new->in_file_status = FS_CLOSED;
  new->out_file_status = FS_NOT_AVAIL;

  if (Handlers[ES_IPSYMTABENT])
    {
      new->ext = malloc (sizeof (Extension) * NumExtensions[ES_IPSYMTABENT]);

      if (new->ext == NULL)
	{
	  P_punt ("struct.c:P_NewIPSymTabEnt:%d Could not allocate ext field",
		  __LINE__ - 1);
	}

      for (i = 0; i < NumExtensions[ES_IPSYMTABENT]; i++)
	{
	  if (Handlers[ES_IPSYMTABENT][i].alloc && \
	      (Handlers[ES_IPSYMTABENT][i].options & HO_MANUAL_ALLOC) == 0)
	    new->ext[i] = Handlers[ES_IPSYMTABENT][i].alloc ();
	  else
	    new->ext[i] = NULL;
	}
    }

  return (new);
}

/*! \brief Allocates a new SymbolTable.
 *
 * \return A pointer to the new SymbolTable.
 *
 * Allocates a new SymbolTable or aborts program if allocation fails.
 * Initializes the new SymbolTable to zeros.
 *
 * \sa P_FreeSymbolTable(), P_RemoveSymbolTable()
 */
SymbolTable
P_NewSymbolTable ()
{
  SymbolTable new;

  /* ALLOCATE initializes the block to 0 and will punt if the malloc fails. */
  new = ALLOCATE (_SymbolTable);

  /* Set up the default file status. */
  new->in_file_status = FS_CLOSED;
  new->out_file_status = FS_NOT_AVAIL;

  /* Set up the default search order. */
  new->search_order[0] = SO_MEM;
  new->search_order[1] = SO_OUT;
  new->search_order[2] = SO_IN;

  return (new);
}

/*! \brief Allocates a new KeyMap.
 *
 * \return A pointer to the new KeyMap.
 *
 * Allocates a new KeyMap or aborts program if allocation fails.
 * Initializes the new KeyMap to zeros.
 *
 * \sa P_FreeKeyMap(), P_RemoveKeyMap()
 */
KeyMap
P_NewKeyMap ()
{
  KeyMap new;

  /* ALLOCATE initializes the block to 0 and will punt if the malloc fails. */
  new = ALLOCATE (_KeyMap);

  return (new);
}

#if 0
/* BCC - 7/3/96 */
StructUnionPoolElem
P_NewStructUnionPoolElem (_TypeClassQual type)
{
  StructUnionPoolElem new;

  /* ALLOCATE initializes the block to 0 and will punt if the malloc fails. */
  new = ALLOCATE (_StructUnionPoolElem);

  new->type = type;

  return (new);
}
#endif

/* Defines a basic type. */
/* Calls to P_NewBasicType should be replaced with:
 * P_NewType
 * P_SetTypeType */
#if 0
Type
P_NewBasicType (int type)
{
  Type new = NewType ();

  new->type = type;
  return new;
}
#endif

/*-------------------------------------------------------------*/
/* Functions to Deallocate Data Structures                     */
/*-------------------------------------------------------------*/

/*! \brief Frees a List and all sub structures.
 *
 * \param l
 *  the List to free.
 * \param free_data
 *  a pointer to a function to free the data field.
 *
 * \return A null List pointer.
 *
 * Frees a List and all sub structures.
 */
List
P_RemoveList (List l, void *(*free_data)(void *))
{
  void *ptr;

  for (List_start (l), ptr = List_next (l); ptr; ptr = List_next (l))
    if (free_data)
      free_data (ptr);
  l = List_reset (l);

  return (l);
}

/*! \brief Frees a Key struct.
 *
 * \param k
 *  the Key pointer to free.
 *
 * \return
 *  A null Key pointer.
 *
 * \note Keys are typically statically allocated, so this function is
 *       not usually needed.
 *
 * \sa P_NewKeyP(), P_NewKeyPWithKey(), #P_RemoveKeyP()
 */
Key *
P_FreeKeyP (Key *k)
{
  if (k)
    {
      DISPOSE (k);
      k = NULL;
    }

  return (k);
}

/*! \brief Frees a Dcl struct.
 *
 * \param d
 *  the Dcl struct to free.
 *
 * \return A null Dcl pointer.
 *
 * Frees a Dcl struct without freeing sub structures.
 *
 * If the global \a check_fields_on_free is true, function will print a
 * warning for any non-null pointer field.
 *
 * \sa P_RemoveDcl(), P_NewDcl(), P_NewDclWithDclType()
 */
Dcl
P_FreeDcl (Dcl d)
{
  if (d)
    {
      if (check_fields_on_free)
	{
	  switch (P_GetDclType (d))
	    {
	    case TT_FUNC:
	      if (P_GetDclFuncDcl (d) != NULL)
		P_warn ("struct.c:P_FreeDcl:%d MEMORY LEAK freeing Dcl with "
			"non-null\nfuncDcl", __LINE__ - 1);
	      break;
	    case TT_TYPE:
	      if (P_GetDclTypeDcl (d) != NULL)
		P_warn ("struct.c:P_FreeDcl:%d MEMORY LEAK freeing Dcl with "
			"non-null\ntypeDcl", __LINE__ - 1);
	      break;
	    case TT_VAR:
	      if (P_GetDclVarDcl (d) != NULL)
		P_warn ("struct.c:P_FreeDcl:%d MEMORY LEAK freeing Dcl with "
			"non-null\nvarDcl", __LINE__ - 1);
	      break;
	    case TT_STRUCT:
	      if (P_GetDclStructDcl (d) != NULL)
		P_warn ("struct.c:P_FreeDcl:%d MEMORY LEAK freeing Dcl with "
			"non-null\nstructDcl", __LINE__ - 1);
	      break;
	    case TT_UNION:
	      if (P_GetDclUnionDcl (d) != NULL)
		P_warn ("struct.c:P_FreeDcl:%d MEMORY LEAK freeing Dcl with "
			"non-null\nunionDcl", __LINE__ - 1);
	      break;
	    case TT_ENUM:
	      if (P_GetDclEnumDcl (d) != NULL)
		P_warn ("struct.c:P_FreeDcl:%d MEMORY LEAK freeing Dcl with "
			"non-null\nenumDcl", __LINE__ - 1);
	      break;
	    case TT_ASM:
	      if (P_GetDclAsmDcl (d) != NULL)
		P_warn ("struct.c:P_FreeDcl:%d MEMORY LEAK freeing Dcl with "
			"non-null\nasmDcl", __LINE__ - 1);
	      break;
	    case TT_INCLUDE:
	      if (P_GetDclInclude (d) != NULL)
		P_warn ("struct.c:P_FreeDcl:%d MEMORY LEAK freeing Dcl with "
			"non-null\ninclude", __LINE__ - 1);
	      break;
	    case TT_SYMBOLTABLE:
	      if (P_GetDclSymbolTable (d) != NULL)
		P_warn ("struct.c:P_FreeDcl:%d MEMORY LEAK freeing Dcl with "
			"non-null\nsymbolTable", __LINE__ - 1);
	      break;
	    case TT_IPSYMTABENT:
	      if (P_GetDclIPSymTabEnt (d) != NULL)
		P_warn ("struct.c:P_FreeDcl:%d MEMORY LEAK freeing Dcl with "
			"non-null\nipSymTabEnt", __LINE__ - 1);
	      break;
	    case TT_SYMTABENTRY:
	      if (P_GetDclSymTabEntry (d) != NULL)
		P_warn ("struct.c:P_FreeDcl:%d MEMORY LEAK freeing Dcl with "
			"non-null\nsymTabEntry", __LINE__ - 1);
	      break;
	    default:
	      P_punt ("struct.c:P_FreeDcl:%d unknown type %d", __LINE__,
		      P_GetDclType (d));
	    }
	}

      DISPOSE (d);
      d = NULL;
    }

  return (d);
}

/*! \brief Frees a Dcl and all sub structures.
 *
 * \param d
 *  the Dcl to free.
 *
 * \return A null Dcl pointer.
 *
 * Frees a Dcl and all sub structures.
 *
 * \sa P_FreeDcl(), P_NewDcl(), P_NewDclWithDclType()
 */
Dcl
P_RemoveDcl (Dcl d)
{
  if (d)
    {
      switch (P_GetDclType (d))
	{
	case TT_FUNC:
	  P_SetDclFuncDcl (d, P_RemoveFuncDcl (P_GetDclFuncDcl (d)));
	  break;
	case TT_TYPE:
	  P_SetDclTypeDcl (d, P_RemoveTypeDcl (P_GetDclTypeDcl (d)));
	  break;
	case TT_VAR:
	  P_SetDclVarDcl (d, P_RemoveVarDcl (P_GetDclVarDcl (d)));
	  break;
	case TT_STRUCT:
	  P_SetDclStructDcl (d, P_RemoveStructDcl (P_GetDclStructDcl (d)));
	  break;
	case TT_UNION:
	  P_SetDclUnionDcl (d, P_RemoveUnionDcl (P_GetDclUnionDcl (d)));
	  break;
	case TT_ENUM:
	  P_SetDclEnumDcl (d, P_RemoveEnumDcl (P_GetDclEnumDcl (d)));
	  break;
	case TT_ASM:
	  P_SetDclAsmDcl (d, P_RemoveAsmDcl (P_GetDclAsmDcl (d)));
	  break;
	case TT_INCLUDE:
	  if (P_GetDclInclude (d))
	    {
	      free (P_GetDclInclude (d));
	      P_SetDclInclude (d, NULL);
	    }
	  break;
	case TT_SYMBOLTABLE:
	  P_SetDclSymbolTable (d,
			       P_RemoveSymbolTable (P_GetDclSymbolTable (d)));
	  break;
	case TT_IPSYMTABENT:
	  P_SetDclIPSymTabEnt (d,
			       P_RemoveIPSymTabEnt (P_GetDclIPSymTabEnt (d)));
	  break;
	case TT_SYMTABENTRY:
	  P_SetDclSymTabEntry (d,
			       P_RemoveSymTabEntry (P_GetDclSymTabEntry (d)));
	  break;
	default:
	  P_punt ("struct.c:P_RemoveDcl:%d unknown type %d", __LINE__,
		  P_GetDclType (d));
	}

      d = P_FreeDcl (d);
    }

  return (d);
}

/*! \brief Frees a FuncDcl struct.
 *
 * \param f
 *  the FuncDcl to free.
 *
 * \return A null FuncDcl pointer.
 *
 * Frees a FuncDcl struct without freeing sub structures.
 *
 * If the global \a check_fields_on_free is true, function will print a
 * warning for any non-null pointer field.
 *
 * \note This function does not check the FuncDcl.shadow field.  All Shadows
 *       are linked to each other, the FuncDcl holds a pointer to the head
 *       of the list, and each shadowed statement has a pointer to its
 *       shadow.  The result is that there are potentially three copies of
 *       the pointer to a given Shadow.  Shadows are therefore freed when
 *       the the shadowed Stmt is freed.
 *
 * \sa P_RemoveFuncDcl(), P_NewFuncDcl()
 */
FuncDcl
P_FreeFuncDcl (FuncDcl f)
{
  if (f)
    {
      if (check_fields_on_free)
	{
	  if (f->name != NULL)
	    P_warn ("struct.c:P_FreeFuncDcl:%d MEMORY LEAK freeing FuncDcl "
		    "with non-null name", __LINE__ - 1);
	  if (f->filename != NULL)
	    P_warn ("struct.c:P_FreeFuncDcl:%d MEMORY LEAK freeing FuncDcl "
		    "with non-null\nfilename", __LINE__ - 1);
	  if (f->param != NULL)
	    P_warn ("struct.c:P_FreeFuncDcl:%d MEMORY LEAK freeing FuncDcl "
		    "with non-null param", __LINE__ - 1);
	  if (f->stmt != NULL)
	    P_warn ("struct.c:P_FreeFuncDcl:%d MEMORY LEAK freeing FuncDcl "
		    "with non-null stmt", __LINE__ - 1);
	  if (f->pragma != NULL)
	    P_warn ("struct.c:P_FreeFuncDcl:%d MEMORY LEAK freeing FuncDcl "
		    "with non-null\npragma", __LINE__ - 1);
	  if (f->profile != NULL)
	    P_warn ("struct.c:P_FreeFuncDcl:%d MEMORY LEAK freeing FuncDcl "
		    "with non-null\nprofile", __LINE__ - 1);
	  if (f->par_loop != NULL)
	    P_warn ("struct.c:P_FreeFuncDcl:%d MEMORY LEAK freeing FuncDcl "
		    "with non-null\npar_loop", __LINE__ - 1);
	  if (f->flow != NULL)
	    P_warn ("struct.c:P_FreeFuncDcl:%d MEMORY LEAK freeing FuncDcl "
		    "with non-null flow", __LINE__ - 1);
	  if (f->depend != NULL)
	    P_warn ("struct.c:P_FreeFuncDcl:%d MEMORY LEAK freeing FuncDcl "
		    "with non-null\ndepend", __LINE__ - 1);
	  if (f->local != NULL)
	    P_warn ("struct.c:P_FreeFuncDcl:%d MEMORY LEAK freeing FuncDcl "
		    "with non-null local", __LINE__ - 1);
	  if (f->ext != NULL)
	    P_warn ("struct.c:P_FreeFuncDcl:%d MEMORY LEAK freeing FuncDcl "
		    "with non-null ext", __LINE__ - 1);
	}

      DISPOSE (f);
      f = NULL;
    }

  return (f);
}

/*! \brief Frees a FuncDcl and all sub structures.
 *
 * \param f
 *  the FuncDcl to free.
 *
 * \return A null FuncDcl pointer.
 *
 * Frees a FuncDcl and all sub structures.
 *
 * \note Applications should typically use PST_RemoveFuncDcl() instead.
 * \note This function does not free the FuncDcl.shadow field.  All Shadows
 *       are linked to each other, the FuncDcl holds a pointer to the head
 *       of the list, and each shadowed statement has a pointer to its
 *       shadow.  The result is that there are potentially three copies of
 *       the pointer to a given Shadow.  Shadows are therefore freed when
 *       the the shadowed Stmt is freed.
 *
 * \sa PST_RemoveFuncDcl(), P_FreeFuncDcl(), P_NewFuncDcl()
 */
FuncDcl
P_RemoveFuncDcl (FuncDcl f)
{
  int i;

  if (f)
    {
      if (f->name)
	{
	  free (f->name);
	  f->name = NULL;
	}
      if (f->filename)
	{
	  free (f->filename);
	  f->filename = NULL;
	}
      f->param = P_RemoveVarList (f->param);
      f->stmt = P_RemoveStmt (f->stmt);
      f->pragma = P_RemovePragma (f->pragma);
      f->profile = P_RemoveProfFN (f->profile);
#if 0
      f->par_loop = P_RemoveStmt (f->par_loop);
      f->flow = P_RemoveFuncFlow (f->flow);
      f->depend = P_RemoveFuncDepend (f->depend);
#endif
      f->local = P_RemoveVarList (f->local);

      if (Handlers[ES_FUNC])
	{
	  for (i = 0; i < NumExtensions[ES_FUNC]; i++)
	    {
	      if (f->ext[i])
		f->ext[i] = Handlers[ES_FUNC][i].free (f->ext[i]);
	    }

	  free (f->ext);
	  f->ext = NULL;
	}

      f = P_FreeFuncDcl (f);
    }
					            
  return (f);
}

/*! \brief Frees a TypeDcl struct.
 *
 * \param t
 *  the TypeDcl struct to free.
 *
 * \return A null TypeDcl pointer.
 *
 * Frees a TypeDcl struct without touching sub structures.
 *
 * If the global \a check_fields_on_free is true, function will print a
 * warning for any non-null pointer field.
 *
 * \sa P_RemoveTypeDcl(), P_NewTypeDcl(), P_NewTypeDclWithBasicType()
 */
TypeDcl
P_FreeTypeDcl (TypeDcl t)
{
  if (t)
    {
      if (check_fields_on_free)
	{
	  if (t->name != NULL)
	    P_warn ("struct.c:P_FreeTypeDcl:%d MEMORY LEAK freeing TypeDcl "
		    "with non-null\nname", __LINE__ - 1);
	  if ((t->basic_type & BT_FUNC) && t->details.param != NULL)
	    P_warn ("struct.c:P_FreeTypeDcl:%d MEMORY LEAK freeing TypeDcl "
		    "with non-null\ndetails.param", __LINE__ - 1);
	  if ((t->basic_type & BT_ARRAY) && t->details.array_size != NULL)
	    P_warn ("struct.c:P_FreeTypeDcl:%d MEMORY LEAK freeing TypeDcl "
		    "with non-null\ndetails.array_size", __LINE__ - 1);
	}

      DISPOSE (t);
      t = NULL;
    }

  return (t);
}

/*! \brief Frees a TypeDcl and all sub structures.
 *
 * \param t
 *  the TypeDcl to free.
 *
 * \return A null TypeDcl pointer.
 *
 * Frees a TypeDcl and all sub structures.
 *
 * \note Applications should typically use PST_RemoveTypeDcl() instead.
 *
 * \sa PST_RemoveTypeDcl(), P_FreeTypeDcl(), P_NewTypeDcl(),
 * P_NewTypeDclWithBasicType() */
TypeDcl
P_RemoveTypeDcl (TypeDcl t)
{
  int i;

  if (t)
    {
      if (t->name)
	{
	  free (t->name);
	  t->name = NULL;
	}
      if (t->basic_type & BT_FUNC)
	t->details.param = P_RemoveParam (t->details.param);
      if (t->basic_type & BT_ARRAY)
	t->details.array_size = P_RemoveExpr (t->details.array_size);
      if (t->filename)
	{
	  free (t->filename);
	  t->filename = NULL;
	}
      t->pragma = P_RemovePragma (t->pragma);

      if (Handlers[ES_TYPE])
	{
	  for (i = 0; i < NumExtensions[ES_TYPE]; i++)
	    {
	      if (t->ext[i])
		t->ext[i] = Handlers[ES_TYPE][i].free (t->ext[i]);
	    }
	  
	  free (t->ext);
	  t->ext = NULL;
	}

      t = P_FreeTypeDcl (t);
    }

  return (t);
}

/*! \brief Frees a KeyList struct.
 *
 * \param l
 *  the KeyList struct to free.
 *
 * \return A null KeyList pointer.
 *
 * Frees a KeyList without freeing sub structures.  This function frees a
 * single list node, not the entire list.
 *
 * If the global \a check_fields_on_free is true, function will print a
 * warning for any non-null pointer field.
 *
 * \sa P_RemoveKeyListNode(), P_RemoveKeyList(), P_NewKeyList(),
 * P_NewKeyListWithKey() */
KeyList
P_FreeKeyList (KeyList l)
{
  if (l)
    {
      if (check_fields_on_free)
	{
	  if (l->next != NULL)
	    P_warn ("struct.c:P_FreeKeyList:%d MEMORY LEAK freeing KeyList "
		    "with\nnon-null next", __LINE__ - 1);
	}

      DISPOSE (l);
      l = NULL;
    }

  return (l);
}

/*! \brief Frees a KeyList node and all sub structures.
 *
 * \param l
 *  the KeyList node to free.
 *
 * \return A null KeyList pointer.
 *
 * Frees a KeyList and all sub structures.  This function frees
 * a single list node, not the entire list.
 *
 * It is the user's responsibility to repair any next pointers that may
 * be destroyed by this function.
 *
 * \sa P_FreeKeyList(), P_RemoveKeyList(), P_NewKeyList(),
 * P_NewKeyListWithKey() */
KeyList
P_RemoveKeyListNode (KeyList l)
{
  if (l)
    {
      l->next = NULL;

      l = P_FreeKeyList (l);
    }

  return (l);
}

/*! \brief Frees a KeyList list and all sub structures.
 *
 * \param l
 *  the KeyList list to free.
 *
 * \return A null KeyList pointer.
 *
 * Frees a KeyList list and all sub structures.
 *
 * \sa P_FreeKeyList(), P_RemoveKeyListNode(), P_NewKeyList(),
 * P_NewKeyListWithKey() */
KeyList
P_RemoveKeyList (KeyList l)
{
  KeyList next;

  while (l)
    {
      next = l->next;
      l = P_RemoveKeyListNode (l);
      l = next;
    }

  return (l);
}

/*! \brief Frees a VarDcl struct.
 *
 * \param v
 *  the VarDcl to free.
 *
 * \return A null VarDcl pointer.
 *
 * Frees a VarDcl struct without freeing sub structures.
 *
 * If the global \a check_fields_on_free is true, function will print a
 * warning for any non-null pointer field.
 *
 * \sa P_RemoveVarDcl(), P_NewVarDcl()
 */
VarDcl
P_FreeVarDcl (VarDcl v)
{
  if (v)
    {
      if (check_fields_on_free)
	{
	  if (v->name != NULL)
	    P_warn ("struct.c:P_FreeVarDcl:%d MEMORY LEAK freeing VarDcl with "
		    "non-null name", __LINE__ - 1);
	  if (v->init != NULL)
	    P_warn ("struct.c:P_FreeVarDcl:%d MEMORY LEAK freeing VarDcl with "
		    "non-null init", __LINE__ - 1);
	  if (v->filename != NULL)
	    P_warn ("struct.c:P_FreeVarDcl:%d MEMORY LEAK freeing VarDcl with "
		    "non-null\nfilename", __LINE__ - 1);
	  if (v->pragma != NULL)
	    P_warn ("struct.c:P_FreeVarDcl:%d MEMORY LEAK freeing VarDcl with "
		    "non-null pragma", __LINE__ - 1);
	  if (v->ext != NULL)
	    P_warn ("struct.c:P_FreeVarDcl:%d MEMORY LEAK freeing VarDcl with "
		    "non-null ext", __LINE__ - 1);
	}

      DISPOSE (v);
      v = NULL;
    }

  return (v);
}

/*! \brief Frees a VarDcl and all sub structures.
 *
 * \param v
 *  the VarDcl to free.
 *
 * \return A null VarDcl pointer.
 *
 * Frees a VarDcl and all sub structures.
 *
 * \note Applications should typically use PST_RemoveVarDcl() instead.n
 *
 * \sa PST_RemoveVarDcl(), P_FreeVarDcl(), P_NewVarDcl()
 */
VarDcl
P_RemoveVarDcl (VarDcl v)
{
  int i;

  if (v)
    {
      if (v->name)
	{
	  free (v->name);
	  v->name = NULL;
	}
      v->init = P_RemoveInit (v->init);
      if (v->filename)
	{
	  free (v->filename);
	  v->filename = NULL;
	}
      v->pragma = P_RemovePragma (v->pragma);

      if (Handlers[ES_VAR])
	{
	  for (i = 0; i < NumExtensions[ES_VAR]; i++)
	    {
	      if (v->ext[i])
		v->ext[i] = Handlers[ES_VAR][i].free (v->ext[i]);
	    }
	  
	  free (v->ext);
	  v->ext = NULL;
	}

      v = P_FreeVarDcl (v);
    }

  return (v);
}

/*! \brief Frees an Init struct.
 *
 * \param i
 *  the Init struct to free.
 *
 * \return A null Init pointer.
 *
 * Frees an Init struct without freeing sub structures.  This function
 * frees a single list node, not the entire list.
 *
 * If the global \a check_fields_on_free is true, function will print a
 * warning for any non-null pointer field.
 *
 * \sa P_RemoveInitNode(), P_RemoveInit(), P_NewInit()
 */
Init
P_FreeInit (Init i)
{
  if (i)
    {
      if (check_fields_on_free)
	{
	  if (i->expr != NULL)
	    P_warn ("struct.c:P_FreeInit:%d MEMORY LEAK freeing Init with "
		    "non-null expr", __LINE__ - 1);
	  if (i->set != NULL)
	    P_warn ("struct.c:P_FreeInit:%d MEMORY LEAK freeing Init with "
		    "non-null set", __LINE__ - 1);
	  if (i->next != NULL)
	    P_warn ("struct.c:P_FreeInit:%d MEMORY LEAK freeing Init with "
		    "non-null next", __LINE__ - 1);
	  if (i->ext != NULL)
	    P_warn ("struct.c:P_FreeInit:%d MEMORY LEAK freeing Init with "
		    "non-null ext", __LINE__ - 1);
	}

      DISPOSE (i);
      i = NULL;
    }

  return (i);
}

/*! \brief Frees an Init node and all sub structures.
 *
 * \param i
 *  the Init node to free.
 *
 * \return A null Init pointer.
 *
 * Frees an Init node and all sub structures.  This function frees
 * a single list node, not the entire list.
 *
 * It is the user's responsibility to repair any list pointers that may be
 * destroyed by this function.
 *
 * \sa P_FreeInit(), P_RemoveInit(), P_NewInit()
 */
Init
P_RemoveInitNode (Init i)
{
  int j;

  if (i)
    {
      i->expr = P_RemoveExpr (i->expr);
      i->set = P_RemoveInit (i->set);
      i->next = NULL;

      if (Handlers[ES_INIT])
	{
	  for (j = 0; j < NumExtensions[ES_INIT]; j++)
	    {
	      if (i->ext[j])
		i->ext[j] = Handlers[ES_INIT][j].free (i->ext[j]);
	    }
	  
	  free (i->ext);
	  i->ext = NULL;
	}

      i = P_FreeInit (i);
    }

  return (i);
}

/*! \brief Frees an Init list and all sub structures.
 *
 * \param i
 *  the Init list to free.
 *
 * \return A null Init pointer.
 *
 * Frees an Init list and all sub structures.
 *
 * \sa P_FreeInit(), P_RemoveInitNode(), P_NewInit()
 */
Init
P_RemoveInit (Init i)
{
  Init next;

  while (i)
    {
      next = i->next;
      i = P_RemoveInitNode (i);
      i = next;
    }

  return (i);
}

/*! \brief Frees a StructDcl struct.
 *
 * \param s
 *  the StructDcl to free.
 *
 * \return A null StructDcl pointer.
 *
 * Frees a StructDcl struct without freeing sub structures.
 *
 * If the global \a check_fields_on_free is true, function will print a
 * warning for any non-null pointer field.
 *
 * \sa P_RemoveStructDcl(), P_NewStructDcl()
 */
StructDcl
P_FreeStructDcl (StructDcl s)
{
  if (s)
    {
      if (check_fields_on_free)
	{
	  if (s->name != NULL)
	    P_warn ("struct.c:P_FreeStructDcl:%d MEMORY LEAK freeing "
		    "StructDcl with\nnon-null name", __LINE__ - 1);
	  if (s->fields != NULL)
	    P_warn ("struct.c:P_FreeStructDcl:%d MEMORY LEAK freeing "
		    "StructDcl with\nnon-null fields", __LINE__ - 1);
	  if (s->filename != NULL)
	    P_warn ("struct.c:P_FreeStructDcl:%d MEMORY LEAK freeing "
		    "StructDcl with\nnon-null filename", __LINE__ - 1);
	  if (s->pragma != NULL)
	    P_warn ("struct.c:P_FreeStructDcl:%d MEMORY LEAK freeing "
		    "StructDcl with\nnon-null pragma", __LINE__ - 1);
	  if (s->ext != NULL)
	    P_warn ("struct.c:P_FreeStructDcl:%d MEMORY LEAK freeing "
		    "StructDcl with\nnon-null ext", __LINE__ - 1);
	}
	  
      DISPOSE (s);
      s = NULL;
    }

  return (s);
}

/*! \brief Frees a StructDcl and all sub structures.
 *
 * \param s
 *  the StructDcl to free.
 *
 * \return A null StructDcl pointer.
 *
 * Frees a StructDcl and all sub structures.
 *
 * \sa P_FreeStructDcl(), P_NewStructDcl()
 */
StructDcl
P_RemoveStructDcl (StructDcl s)
{
  int i;

  if (s)
    {
      if (s->name)
	{
	  free (s->name);
	  s->name = NULL;
	}
      s->fields = P_RemoveField (s->fields);
      if (s->filename)
	{
	  free (s->filename);
	  s->filename = NULL;
	}
      s->pragma = P_RemovePragma (s->pragma);

      if (Handlers[ES_STRUCT])
	{
	  for (i = 0; i < NumExtensions[ES_STRUCT]; i++)
	    {
	      if (s->ext[i])
		s->ext[i] = Handlers[ES_STRUCT][i].free (s->ext[i]);
	    }
	  
	  free (s->ext);
	  s->ext = NULL;
	}

      s = P_FreeStructDcl (s);
    }

  return (s);
}

/*! \brief Frees a UnionDcl struct.
 *
 * \param u
 *  the UnionDcl to free.
 *
 * \return A null UnionDcl pointer.
 *
 * Frees a UnionDcl struct without freeing sub structures.
 *
 * If the global \a check_fields_on_free is true, function will print a
 * warning for any non-null pointer field.
 *
 * \sa P_RemoveUnionDcl(), P_NewUnionDcl()
 */
UnionDcl
P_FreeUnionDcl (UnionDcl u)
{
  if (u)
    {
      if (check_fields_on_free)
	{
	  if (u->name != NULL)
	    P_warn ("struct.c:P_FreeUnionDcl:%d MEMORY LEAK freeing UnionDcl "
		    "with non-null\nname", __LINE__ - 1);
	  if (u->fields != NULL)
	    P_warn ("struct.c:P_FreeUnionDcl:%d MEMORY LEAK freeing UnionDcl "
		    "with non-null\nfields", __LINE__ - 1);
	  if (u->filename != NULL)
	    P_warn ("struct.c:P_FreeUnionDcl:%d MEMORY LEAK freeing UnionDcl "
		    "with non-null\nfilename", __LINE__ - 1);
	  if (u->pragma != NULL)
	    P_warn ("struct.c:P_FreeUnionDcl:%d MEMORY LEAK freeing UnionDcl "
		    "with\nnon-null pragma", __LINE__ - 1);
	  if (u->ext != NULL)
	    P_warn ("struct.c:P_FreeUnionDcl:%d MEMORY LEAK freeing UnionDcl "
		    "with non-null\next", __LINE__ - 1);
	}

      DISPOSE (u);
      u = NULL;
    }

  return (u);
}

/*! \brief Frees a UnionDcl and all sub structures.
 *
 * \param u
 *  the UnionDcl to free.
 *
 * \return A null UnionDcl pointer.
 *
 * Frees a UnionDcl and all sub structures.
 *
 * \sa P_FreeUnionDcl(), P_NewUnionDcl()
 */
UnionDcl
P_RemoveUnionDcl (UnionDcl u)
{
  int i;

  if (u)
    {
      if (u->name)
	{
	  free (u->name);
	  u->name = NULL;
	}
      u->fields = P_RemoveField (u->fields);
      if (u->filename)
	{
	  free (u->filename);
	  u->filename = NULL;
	}
      u->pragma = P_RemovePragma (u->pragma);

      if (Handlers[ES_UNION])
	{
	  for (i = 0; i < NumExtensions[ES_UNION]; i++)
	    {
	      if (u->ext[i])
		u->ext[i] = Handlers[ES_UNION][i].free (u->ext[i]);
	    }
	  
	  free (u->ext);
	  u->ext = NULL;
	}

      u = P_FreeUnionDcl (u);
    }

  return (u);
}

/*! \brief Frees a Field struct.
 *
 * \param f
 *  the Field struct to free.
 *
 * \return A null Field pointer.
 *
 * Frees a Field struct without freeing sub structures.  This function
 * frees a single list node, not the entire list.
 *
 * If the global \a check_fields_on_free is true, function will print a
 * warning for any non-null pointer field.
 *
 * \sa P_RemoveFieldNode(), P_RemoveField(), P_NewField()
 */
Field
P_FreeField (Field f)
{
  if (f)
    {
      if (check_fields_on_free)
	{
	  if (f->name != NULL)
	    P_warn ("struct.c:P_FreeField:%d MEMORY LEAK freeing Field with "
		    "non-null name", __LINE__ - 1);
	  if (f->next != NULL)
	    P_warn ("struct.c:P_FreeField:%d MEMORY LEAK freeing Field with "
		    "non-null next", __LINE__ - 1);
	  if (f->ext != NULL)
	    P_warn ("struct.c:P_FreeField:%d MEMORY LEAK freeing Field with "
		    "non-null ext", __LINE__ - 1);
	}

      DISPOSE (f);
      f = NULL;
    }

  return (f);
}

/*! \brief Frees a Field node and all sub structures.
 *
 * \param f
 *  the Field node to free.
 *
 * \return A null Field pointer.
 *
 * Frees a Field node and all sub structures.  This function frees
 * a single list node, not the entire list.
 *
 * It is the user's responsibility to repair any list pointers that may be
 * destroyed by this function.
 *
 * \sa P_FreeField(), P_RemoveField(), P_NewField()
 */
Field
P_RemoveFieldNode (Field f)
{
  int i;
  
  if (f)
    {
      if (f->name)
	{
	  free (f->name);
	  f->name = NULL;
	}
      f->next = NULL;

      if (Handlers[ES_FIELD])
	{
	  for (i = 0; i < NumExtensions[ES_FIELD]; i++)
	    {
	      if (f->ext[i])
		f->ext[i] = Handlers[ES_FIELD][i].free (f->ext[i]);
	    }
	  
	  free (f->ext);
	  f->ext = NULL;
	}

      f = P_FreeField (f);
    }

  return (f);
}

/*! \brief Frees a Field list and all sub structures.
 *
 * \param f
 *  the Field list to free.
 *
 * \return A null Field pointer.
 *
 * Frees a Field list and all sub structures.
 *
 * \note Applications should typically use PST_RemoveField() instead.
 *
 * \sa PST_RemoveField(), P_FreeField(), P_RemoveFieldNode(),
 * P_NewField() */
Field
P_RemoveField (Field f)
{
  Field next;

  while (f)
    {
      next = f->next;
      f = P_RemoveFieldNode (f);
      f = next;
    }

  return (f);
}

/*! \brief Frees an EnumDcl struct.
 *
 * \param e
 *  the EnumDcl to free.
 *
 * \return A null EnumDcl pointer.
 *
 * Frees an EnumDcl struct without freeing sub structures.
 *
 * If the global \a check_fields_on_free is true, function will print a
 * warning for any non-null pointer field.
 *
 * \sa P_RemoveEnumDcl(), P_NewEnumDcl()
 */
EnumDcl
P_FreeEnumDcl (EnumDcl e)
{
  if (e)
    {
      if (check_fields_on_free)
	{
	  if (e->name != NULL)
	    P_warn ("struct.c:P_FreeEnumDcl:%d MEMORY LEAK freeing EnumDcl "
		    "with non-null name", __LINE__ - 1);
	  if (e->fields != NULL)
	    P_warn ("struct.c:P_FreeEnumDcl:%d MEMORY LEAK freeing EnumDcl "
		    "with non-null\nfields", __LINE__ - 1);
	  if (e->filename != NULL)
	    P_warn ("struct.c:P_FreeEnumDcl:%d MEMORY LEAK freeing EnumDcl "
		    "with non-null\nfilename", __LINE__ - 1);
	  if (e->pragma != NULL)
	    P_warn ("struct.c:P_FreeEnumDcl:%d MEMORY LEAK freeing EnumDcl "
		    "with non-null\npragma", __LINE__ - 1);
	  if (e->ext != NULL)
	    P_warn ("struct.c:P_FreeEnumDcl:%d MEMORY LEAK freeing EnumDcl "
		    "with non-null ext", __LINE__ - 1);
	}

      DISPOSE (e);
      e = NULL;
    }

  return (e);
}

/*! \brief Frees an EnumDcl and all sub structures.
 *
 * \param e
 *  the EnumDcl to free.
 *
 * \return A null EnumDcl pointer.
 *
 * Frees an EnumDcl and all sub structures.
 *
 * \sa P_FreeEnumDcl(), P_NewEnumDcl()
 */
EnumDcl
P_RemoveEnumDcl (EnumDcl e)
{
  int i;

  if (e)
    {
      if (e->name)
	{
	  free (e->name);
	  e->name = NULL;
	}
      e->fields = P_RemoveEnumField (e->fields);
      if (e->filename)
	{
	  free (e->filename);
	  e->filename = NULL;
	}
      e->pragma = P_RemovePragma (e->pragma);

      if (Handlers[ES_ENUM])
	{
	  for (i = 0; i < NumExtensions[ES_ENUM]; i++)
	    {
	      if (e->ext[i])
		e->ext[i] = Handlers[ES_ENUM][i].free (e->ext[i]);
	    }
	  
	  free (e->ext);
	  e->ext = NULL;
	}

      e = P_FreeEnumDcl (e);
    }

  return (e);
}

/*! \brief Frees an EnumField struct.
 *
 * \param f
 *  the EnumField struct to free.
 *
 * \return A null EnumField pointer.
 *
 * Frees an EnumField struct without freeing sub structures.  This function
 * frees a single list node, not the entire list.
 *
 * If the global \a check_fields_on_free is true, function will print a
 * warning for any non-null pointer field.
 *
 * \sa P_RemoveEnumFieldNode(), P_RemoveEnumField(), P_NewEnumField()
 */
EnumField
P_FreeEnumField (EnumField f)
{
  if (f)
    {
      if (check_fields_on_free)
	{
	  if (f->name != NULL)
	    P_warn ("struct.c:P_FreeEnumField:%d MEMORY LEAK freeing "
		    "EnumField with non-null\nname", __LINE__ - 1);
	  if (f->value != NULL)
	    P_warn ("struct.c:P_FreeEnumField:%d MEMORY LEAK freeing "
		    "EnumField with non-null\nvalue", __LINE__ - 1);
	  if (f->next != NULL)
	    P_warn ("struct.c:P_FreeEnumField:%d MEMORY LEAK freeing "
		    "EnumField with non-null\nnext", __LINE__ - 1);
	}

      DISPOSE (f);
      f = NULL;
    }

  return (f);
}

/*! \brief Frees an EnumField node and all sub structures.
 *
 * \param f
 *  the EnumField node to free.
 *
 * \return A null EnumField pointer.
 *
 * Frees an EnumField node and all sub structures.  This function frees
 * a single list node, not the entire list.
 *
 * It is the user's responsibility to repair any list pointers that may be
 * destroyed by this function.
 *
 * \sa P_FreeEnumField(), P_RemoveEnumField(), P_NewEnumField()
 */
EnumField
P_RemoveEnumFieldNode (EnumField f)
{
  if (f)
    {
      if (f->name)
	{
	  free (f->name);
	  f->name = NULL;
	}
      f->value = P_RemoveExpr (f->value);
      f->next = NULL;

      f = P_FreeEnumField (f);
    }

  return (f);
}

/*! \brief Frees an EnumField list and all sub structures.
 *
 * \param f
 *  the EnumField list to free.
 *
 * \return A null EnumField pointer.
 *
 * Frees an EnumField list and all sub structures.
 *
 * \sa P_FreeEnumField(), P_RemoveEnumFieldNode(), P_NewEnumField()
 */
EnumField
P_RemoveEnumField (EnumField f)
{
  EnumField next;

  while (f)
    {
      next = f->next;
      f = P_RemoveEnumFieldNode (f);
      f = next;
    }

  return (f);
}

/*! \brief Frees a Stmt struct.
 *
 * \param s
 *  the Stmt struct to free.
 *
 * \return A null Stmt pointer.
 *
 * Frees a Stmt struct without freeing sub structures.  This function
 * frees a single list node, not the entire list.
 *
 * If the global \a check_fields_on_free is true, function will print a
 * warning for any non-null pointer field.
 *
 * \sa P_RemoveStmtNode(), P_RemoveStmt(), P_NewStmt(),
 * P_NewStmtWithType(), P_NewGotoStmt(), P_NewExprStmt() */
Stmt
P_FreeStmt (Stmt s)
{
  if (s)
    {
      if (check_fields_on_free)
	{
	  if (s->filename != NULL)
	    P_warn ("struct.c:P_FreeStmt:%d MEMORY LEAK removing Stmt with "
		    "non-null\nfilename", __LINE__ - 1);
	  if (s->profile != NULL)
	    P_warn ("struct.c:P_FreeStmt:%d MEMORY LEAK removing Stmt with "
		    "non-null\nprofile", __LINE__ - 1);
	  if (s->pragma != NULL)
	    P_warn ("struct.c:P_FreeStmt:%d MEMORY LEAK removing Stmt with "
		    "non-null\npragma", __LINE__ - 1);
	  if (s->shadow != NULL)
	    P_warn ("struct.c:P_FreeStmt:%d MEMORY LEAK removing Stmt with "
		    "non-null\nshadow", __LINE__ - 1);
	  if (s->lex_next != NULL)
	    P_warn ("struct.c:P_FreeStmt:%d MEMORY LEAK removing Stmt with "
		    "non-null\nlex_next", __LINE__ - 1);
	  if (s->labels != NULL)
	    P_warn ("struct.c:P_FreeStmt:%d MEMORY LEAK removing Stmt with "
		    "non-null\nlabels", __LINE__ - 1);
	  switch (s->type)
	    {
	    case ST_RETURN:
	      if (s->stmtstruct.ret != NULL)
		P_warn ("struct.c:P_FreeStmt:%d MEMORY LEAK removing Stmt "
			"with non-null\nstmtstruct.ret", __LINE__ - 1);
	      break;
	    case ST_GOTO:
	      if (s->stmtstruct.label.val != NULL)
		P_warn ("struct.c:P_FreeStmt:%d MEMORY LEAK removing Stmt "
			"with non-null\nstmtstruct.label.val", __LINE__ - 1);
	      break;
	    case ST_COMPOUND:
	      if (s->stmtstruct.compound != NULL)
		P_warn ("struct.c:P_FreeStmt:%d MEMORY LEAK removing Stmt "
			"with non-null\nstmtstruct.compound", __LINE__ - 1);
	      break;
	    case ST_IF:
	      if (s->stmtstruct.ifstmt != NULL)
		P_warn ("struct.c:P_FreeStmt:%d MEMORY LEAK removing Stmt "
			"with non-null\nstmtstruct.ifstmt", __LINE__ - 1);
	      break;
	    case ST_SWITCH:
	      if (s->stmtstruct.switchstmt != NULL)
		P_warn ("struct.c:P_FreeStmt:%d MEMORY LEAK removing Stmt "
			"with non-null\nstmtstruct.switchstmt", __LINE__ - 1);
	      break;
	    case ST_PSTMT:
	      if (s->stmtstruct.pstmt != NULL)
		P_warn ("struct.c:P_FreeStmt:%d MEMORY LEAK removing Stmt "
			"with non-null\nstmtstruct.pstmt", __LINE__ - 1);
	      break;
	    case ST_ADVANCE:
	      if (s->stmtstruct.advance != NULL)
		P_warn ("struct.c:P_FreeStmt:%d MEMORY LEAK removing Stmt "
			"with non-null\nstmtstruct.advance", __LINE__ - 1);
	      break;
	    case ST_AWAIT:
	      if (s->stmtstruct.await != NULL)
		P_warn ("struct.c:P_FreeStmt:%d MEMORY LEAK removing Stmt "
			"with non-null\nstmtstruct.await", __LINE__ - 1);
	      break;
	    case ST_MUTEX:
	      if (s->stmtstruct.mutex != NULL)
		P_warn ("struct.c:P_FreeStmt:%d MEMORY LEAK removing Stmt "
			"with non-null\nstmtstruct.mutex", __LINE__ - 1);
	      break;
	    case ST_COBEGIN:
	      if (s->stmtstruct.cobegin != NULL)
		P_warn ("struct.c:P_FreeStmt:%d MEMORY LEAK removing Stmt "
			"with non-null\nstmtstruct.cobegin", __LINE__ - 1);
	      break;
	    case ST_PARLOOP:
	      if (s->stmtstruct.parloop != NULL)
		P_warn ("struct.c:P_FreeStmt:%d MEMORY LEAK removing Stmt "
			"with non-null\nstmtstruct.parloop", __LINE__ - 1);
	      break;
	    case ST_SERLOOP:
	      if (s->stmtstruct.serloop != NULL)
		P_warn ("struct.c:P_FreeStmt:%d MEMORY LEAK removing Stmt "
			"with non-null\nstmtstruct.serloop", __LINE__ - 1);
	      break;
	    case ST_EXPR:
	      if (s->stmtstruct.expr != NULL)
		P_warn ("struct.c:P_FreeStmt:%d MEMORY LEAK removing Stmt "
			"with non-null\nstmtstruct.expr", __LINE__ - 1);
	      break;
	    case ST_BODY:
	      if (s->stmtstruct.bodystmt!= NULL)
		P_warn ("struct.c:P_FreeStmt:%d MEMORY LEAK removing Stmt "
			"with non-null\nstmtstruct.bodystmt", __LINE__ - 1);
	      break;
	    case ST_EPILOGUE:
	      if (s->stmtstruct.epiloguestmt != NULL)
		P_warn ("struct.c:P_FreeStmt:%d MEMORY LEAK removing Stmt "
			"with non-null\nstmtstruct.epiloguestmt",
			__LINE__ - 2);
	      break;
	    case ST_ASM:
	      if (s->stmtstruct.asmstmt != NULL)
		P_warn ("struct.c:P_FreeStmt:%d MEMORY LEAK removing Stmt "
			"with non-null\nstmtstruct.asmstmt", __LINE__ - 1);
	      break;
	    default:
	      break;
	    }
	  
	  if (s->flow != NULL)
	    P_warn ("struct.c:P_FreeStmt:%d MEMORY LEAK removing Stmt with "
		    "non-null\nflow", __LINE__ - 1);
	  if (s->ext != NULL)
	    P_warn ("struct.c:P_FreeStmt:%d MEMORY LEAK removing Stmt with "
		    "non-null ext", __LINE__ - 1);
	}

      DISPOSE (s);
      s = NULL;
    }

  return (s);
}

/*! \brief Frees a Stmt node and all sub structures.
 *
 * \param s
 *  the Stmt node to free.
 *
 * \return A null Stmt pointer.
 *
 * Frees a Stmt node and all sub structures.  This function frees
 * a single list node, not the entire list.
 *
 * It is the user's responsibility to repair any list pointers that may be
 * destroyed by this function.
 *
 * \sa P_FreeStmt(), P_RemoveStmt(), P_NewStmt(), P_NewStmtWithType(),
 * P_NewGotoStmt(), P_NewExprStmt() */
Stmt
P_RemoveStmtNode (Stmt s)
{
  int i;

  if (s)
    {
      if (s->filename)
	{
	  free (s->filename);
	  s->filename = NULL;
	}
      s->profile = P_RemoveProfST (s->profile);
      s->pragma = P_RemovePragma (s->pragma);
      s->shadow = P_RemoveShadow (s->shadow);
      s->labels = P_RemoveLabel (s->labels);
      switch (s->type)
	{
	case ST_NOOP:
	case ST_CONT:
	case ST_BREAK:
	  break;
	case ST_RETURN:
	  s->stmtstruct.ret = P_RemoveExpr (s->stmtstruct.ret);
	  break;
	case ST_GOTO:
	  if (s->stmtstruct.label.val)
	    {
	      free (s->stmtstruct.label.val);
	      s->stmtstruct.label.val = NULL;
	    }
	  break;
	case ST_COMPOUND:
	  s->stmtstruct.compound = P_RemoveCompound (s->stmtstruct.compound);
	  break;
	case ST_IF:
	  s->stmtstruct.ifstmt = P_RemoveIfStmt (s->stmtstruct.ifstmt);
	  break;
	case ST_SWITCH:
	  s->stmtstruct.switchstmt = \
	    P_RemoveSwitchStmt (s->stmtstruct.switchstmt);
	  break;
	case ST_PSTMT:
	  s->stmtstruct.pstmt = P_RemovePstmt (s->stmtstruct.pstmt);
	  break;
	case ST_ADVANCE:
	  s->stmtstruct.advance = P_RemoveAdvance (s->stmtstruct.advance);
	  break;
	case ST_AWAIT:
	  s->stmtstruct.await = P_RemoveAwait (s->stmtstruct.await);
	  break;
	case ST_MUTEX:
	  s->stmtstruct.mutex = P_RemoveMutex (s->stmtstruct.mutex);
	  break;
	case ST_COBEGIN:
	  s->stmtstruct.cobegin = P_RemoveCobegin (s->stmtstruct.cobegin);
	  break;
	case ST_PARLOOP:
	  s->stmtstruct.parloop = P_RemoveParLoop (s->stmtstruct.parloop);
	  break;
	case ST_SERLOOP:
	  s->stmtstruct.serloop = P_RemoveSerLoop (s->stmtstruct.serloop);
	  break;
	case ST_EXPR:
	  s->stmtstruct.expr = P_RemoveExpr (s->stmtstruct.expr);
	  break;
	case ST_BODY:
	  s->stmtstruct.bodystmt = P_RemoveBodyStmt (s->stmtstruct.bodystmt);
	  break;
	case ST_EPILOGUE:
	  s->stmtstruct.epiloguestmt = \
	    P_RemoveEpilogueStmt (s->stmtstruct.epiloguestmt);
	  break;
	case ST_ASM:
	  s->stmtstruct.asmstmt = P_RemoveAsmStmt (s->stmtstruct.asmstmt);
	  break;
	default:
	  P_punt ("struct.c:P_RemoveStmtNode:%d unknown stmt type %d",
		  __LINE__ - 1, s->type);
	}
#if 0
      s->flow = P_RemoveStmtFlow (s->flow);
#endif
      s->lex_next = NULL;

      if (Handlers[ES_STMT])
	{
	  for (i = 0; i < NumExtensions[ES_STMT]; i++)
	    {
	      if (s->ext[i])
		s->ext[i] = Handlers[ES_STMT][i].free (s->ext[i]);
	    }
	  
	  free (s->ext);
	  s->ext = NULL;
	}

      s = P_FreeStmt (s);
    }

  return (s);
}

/*! \brief Frees a Stmt list and all sub structures.
 *
 * \param s
 *  the Stmt list to free.
 *
 * \return A null Stmt pointer.
 *
 * Frees a Stmt list and all sub structures.
 *
 * \sa P_FreeStmt(), P_RemoveStmtNode(), P_NewStmt(),
 * P_NewStmtWithType(), P_NewGotoStmt(), P_NewExprStmt() */
Stmt
P_RemoveStmt (Stmt s)
{
  if (s)
    {
      s->lex_next = P_RemoveStmt (s->lex_next);

      s = P_RemoveStmtNode (s);
    }

  return (s);
}

/*! \brief Frees a Label struct.
 *
 * \param l
 *  the Label struct to free.
 *
 * \return A null Label pointer.
 *
 * Frees a Label struct without freeing sub structures.  This function
 * frees a single list node, not the entire list.
 *
 * If the global \a check_fields_on_free is true, function will print a
 * warning for any non-null pointer field.
 *
 * \sa P_RemoveLabelNode(), P_RemoveLabel(), P_NewLabel()
 */
Label
P_FreeLabel (Label l)
{
  if (l)
    {
      if (check_fields_on_free)
	{
	  if (l->val != NULL)
	    P_warn ("struct.c:P_FreeLabel:%d MEMORY LEAK freeing Label with "
		    "non-null val", __LINE__ - 1);
	  if (l->type == LB_CASE && l->data.expression != NULL)
	    P_warn ("struct.c:P_FreeLabel:%d MEMORY LEAK freeing Label with "
		    "non-null\ndata.expression", __LINE__ - 1);
	  if (l->next != NULL)
	    P_warn ("struct.c:P_FreeLabel:%d MEMORY LEAK freeing Label with "
		    "non-null next", __LINE__ - 1);
	}

      DISPOSE (l);
      l = NULL;
    }

  return (l);
}

/*! \brief Frees a Label node and all sub structures.
 *
 * \param l
 *  the Label node to free.
 *
 * \return A null Label pointer.
 *
 * Frees a Label node and all sub structures.  This function frees
 * a single list node, not the entire list.
 *
 * It is the user's responsibility to repair any list pointers that may be
 * destroyed by this function.
 *
 * \sa P_FreeLabel(), P_RemoveLabel(), P_NewLabel()
 */
Label
P_RemoveLabelNode (Label l)
{
  if (l)
    {
      if (l->val)
	{
	  free (l->val);
	  l->val = NULL;
	}
      if (l->type == LB_CASE)
	l->data.expression = P_RemoveExpr (l->data.expression);
      l->next = NULL;

      l = P_FreeLabel (l);
    }

  return (l);
}

/*! \brief Frees a Label list and all sub structures.
 *
 * \param l
 *  the Label list to free.
 *
 * \return A null Label pointer.
 *
 * Frees a Label list and all sub structures.
 *
 * \sa P_FreeLabel(), P_RemoveLabelNode(), P_NewLabel()
 */
Label
P_RemoveLabel (Label l)
{
  Label next;

  while (l)
    {
      next = l->next;
      l = P_RemoveLabelNode (l);
      l = next;
    }

  return (l);
}

/*! \brief Frees a Compound struct.
 *
 * \param c
 *  the Compound struct to free.
 *
 * \return A null Compound pointer.
 *
 * Frees a Compound struct without touching sub structures.
 *
 * If the global \a check_fields_on_free is true, function will print a
 * warning for any non-null pointer field.
 *
 * \sa P_RemoveCompound(), P_NewCompound()
 */
Compound
P_FreeCompound (Compound c)
{
  if (c)
    {
      if (check_fields_on_free)
	{
	  if (c->type_list != NULL)
	    P_warn ("struct.c:P_FreeCompound:%d MEMORY LEAK freeing Compound "
		    "with non-null\ntype_list", __LINE__ - 1);
	  if (c->var_list != NULL)
	    P_warn ("struct.c:P_FreeCompound:%d MEMORY LEAK freeing Compound "
		    "with non-null\nvar_list", __LINE__ - 1);
	  if (c->stmt_list != NULL)
	    P_warn ("struct.c:P_FreeCompound:%d MEMORY LEAK freeing Compound "
		    "with non-null\nstmt_list", __LINE__ - 1);
	}

      DISPOSE (c);
      c = NULL;
    }

  return (c);
}

/*! \brief Frees a Compound and all sub structures.
 *
 * \param c
 *  the Compound to free.
 *
 * \return A null Compound pointer.
 *
 * Frees a Compound and all sub structures.
 *
 * \sa P_FreeCompound(), P_NewCompound()
 */
Compound
P_RemoveCompound (Compound c)
{
  if (c)
    {
      c->type_list = P_RemoveTypeList (c->type_list);
      c->var_list = P_RemoveVarList (c->var_list);
      c->stmt_list = P_RemoveStmt (c->stmt_list);

      c = P_FreeCompound (c);
    }

  return (c);
}

/*! \brief Frees an IfStmt struct.
 *
 * \param i
 *  the IfStmt struct to free.
 *
 * \return A null IfStmt pointer.
 *
 * Frees an IfStmt struct without touching sub structures.
 *
 * If the global \a check_fields_on_free is true, function will print a
 * warning for any non-null pointer field.
 *
 * \sa P_RemoveIfStmt(), P_NewIfStmt()
 */
IfStmt
P_FreeIfStmt (IfStmt i)
{
  if (i)
    {
      if (check_fields_on_free)
	{
	  if (i->cond_expr != NULL)
	    P_warn ("struct.c:P_FreeIfStmt:%d MEMORY LEAK freeing IfStmt with "
		    "non-null\ncond_expr", __LINE__ - 1);
	  if (i->then_block != NULL)
	    P_warn ("struct.c:P_FreeIfStmt:%d MEMORY LEAK freeing IfStmt with "
		    "non-null\nthen_block", __LINE__ - 1);
	  if (i->else_block != NULL)
	    P_warn ("struct.c:P_FreeIfStmt:%d MEMORY LEAK freeing IfStmt with "
		    "non-null\nelse_block", __LINE__ - 1);
	}

      DISPOSE (i);
      i = NULL;
    }

  return (i);
}

/*! \brief Frees an IfStmt and all sub structures.
 *
 * \param i
 *  the IfStmt to free.
 *
 * \return A null IfStmt pointer.
 *
 * Frees an IfStmt and all sub structures.
 *
 * \sa P_FreeIfStmt(), P_NewIfStmt()
 */
IfStmt
P_RemoveIfStmt (IfStmt i)
{
  if (i)
    {
      i->cond_expr = P_RemoveExpr (i->cond_expr);
      i->then_block = P_RemoveStmt (i->then_block);
      i->else_block = P_RemoveStmt (i->else_block);

      i = P_FreeIfStmt (i);
    }

  return (i);
}

/*! \brief Frees a SwitchStmt struct.
 *
 * \param s
 *  the SwitchStmt struct to free.
 *
 * \return A null SwitchStmt pointer.
 *
 * Frees a SwitchStmt struct without touching sub structures.
 *
 * If the global \a check_fields_on_free is true, function will print a
 * warning for any non-null pointer field.
 *
 * \sa P_RemoveSwitchStmt(), P_NewSwitchStmt()
 */
SwitchStmt
P_FreeSwitchStmt (SwitchStmt s)
{
  if (s)
    {
      if (check_fields_on_free)
	{
	  if (s->expression != NULL)
	    P_warn ("struct.c:P_FreeSwitchStmt:%d MEMORY LEAK freeing "
		    "SwitchStmt with\nnon-null expression", __LINE__ - 1);
	  if (s->switchbody != NULL)
	    P_warn ("struct.c:P_FreeSwitchStmt:%d MEMORY LEAK freeing "
		    "SwitchStmt with\nnon-null switchbody", __LINE__ - 1);
	}

      DISPOSE (s);
      s = NULL;
    }

  return (s);
}

/*! \brief Frees a SwitchStmt and all sub structures.
 *
 * \param s
 *  the SwitchStmt to free.
 *
 * \return A null SwitchStmt pointer.
 *
 * Frees a SwitchStmt and all sub structures.
 *
 * \sa P_FreeSwitchStmt(), P_NewSwitchStmt()
 */
SwitchStmt
P_RemoveSwitchStmt (SwitchStmt s)
{
  if (s)
    {
      s->expression = P_RemoveExpr (s->expression);
      s->switchbody = P_RemoveStmt (s->switchbody);

      s = P_FreeSwitchStmt (s);
    }

  return (s);
}

/*! \brief Frees a Pstmt struct.
 *
 * \param p
 *  the Pstmt struct to free.
 *
 * \return A null Pstmt pointer.
 *
 * Frees a Pstmt struct without touching sub structures.
 *
 * If the global \a check_fields_on_free is true, function will print a
 * warning for any non-null pointer field.
 *
 * \sa P_RemovePstmt(), P_NewPstmt()
 */
Pstmt
P_FreePstmt (Pstmt p)
{
  if (p)
    {
      if (check_fields_on_free)
	{
	  if (p->stmt != NULL)
	    P_warn ("struct.c:P_FreePstmt:%d MEMORY LEAK freeing Pstmt with "
		    "non-null stmt", __LINE__ - 1);
	  if (p->filename != NULL)
	    P_warn ("struct.c:P_FreePstmt:%d MEMORY LEAK freeing Pstmt with "
		    "non-null filename", __LINE__ - 1);
	  if (p->ext != NULL)
	    P_warn ("struct.c:P_FreePstmt:%d MEMORY LEAK freeing Pstmt with "
		    "non-null ext", __LINE__ - 1);
	}
	  
      DISPOSE (p);
      p = NULL;
    }

  return (p);
}

/*! \brief Frees a Pstmt and all sub structures.
 *
 * \param p
 *  the Pstmt to free.
 *
 * \return A null Pstmt pointer.
 *
 * Frees a Pstmt and all sub structures.
 *
 * \sa P_FreePstmt(), P_NewPstmt()
 */
Pstmt
P_RemovePstmt (Pstmt p)
{
  int i;

  if (p)
    {
      p->stmt = P_RemoveStmt (p->stmt);
      if (p->filename)
	{
	  free (p->filename);
	  p->filename = NULL;
	}

      if (Handlers[ES_PSTMT])
	{
	  for (i = 0; i < NumExtensions[ES_PSTMT]; i++)
	    {
	      if (p->ext[i])
		p->ext[i] = Handlers[ES_PSTMT][i].free (p->ext[i]);
	    }
	  
	  free (p->ext);
	  p->ext = NULL;
	}

      p = P_FreePstmt (p);
    }

  return (p);
}
      
/*! \brief Frees an Advance struct.
 *
 * \param a
 *  the Advance struct to free.
 *
 * \return A null Advance pointer.
 *
 * Frees an Advance struct without touching sub structures.
 *
 * \sa P_RemoveAdvance(), P_NewAdvance()
 */
Advance
P_FreeAdvance (Advance a)
{
  if (a)
    {
      DISPOSE (a);
      a = NULL;
    }

  return (a);
}

/*! \brief Frees an Advance and all sub structures.
 *
 * \param a
 *  the Advance to free.
 *
 * \return A null Advance pointer.
 *
 * Frees an Advance and all sub structures.
 *
 * \sa P_FreeAdvance(), P_NewAdvance()
 */
Advance
P_RemoveAdvance (Advance a)
{
  if (a)
    {
      a = P_FreeAdvance (a);
    }

  return (a);
}

/*! \brief Frees an Await struct.
 *
 * \param a
 *  the Await struct to free.
 *
 * \return A null Await pointer.
 *
 * Frees an Await struct without touching sub structures.
 *
 * \sa P_RemoveAwait(), P_NewAwait()
 */
Await
P_FreeAwait (Await a)
{
  if (a)
    {
      DISPOSE (a);
      a = NULL;
    }

  return (a);
}

/*! \brief Frees an Await and all sub structures.
 *
 * \param a
 *  the Await to free.
 *
 * \return A null Await pointer.
 *
 * Frees an Await and all sub structures.
 *
 * \sa P_FreeAwait(), P_NewAwait()
 */
Await
P_RemoveAwait (Await a)
{
  if (a)
    {
      a = P_FreeAwait (a);
    }

  return (a);
}

/*! \brief Frees a Mutex struct.
 *
 * \param m
 *  the Mutex struct to free.
 *
 * \return A null Mutex pointer.
 *
 * Frees a Mutex struct without touching sub structures.
 *
 * If the global \a check_fields_on_free is true, function will print a
 * warning for any non-null pointer field.
 *
 * \sa P_RemoveMutex(), P_NewMutex()
 */
Mutex
P_FreeMutex (Mutex m)
{
  if (m)
    {
      if (check_fields_on_free)
	{
	  if (m->expression != NULL)
	    P_warn ("struct.c:P_FreeMutex:%d MEMORY LEAK freeing Mutex with "
		    "non-null\nexpression", __LINE__ - 1);
	  if (m->statement != NULL)
	    P_warn ("struct.c:P_FreeMutex:%d MEMORY LEAK freeing Mutex with "
		    "non-null\nstatement", __LINE__ - 1);
	}

      DISPOSE (m);
      m = NULL;
    }

  return (m);
}

/*! \brief Frees a Mutex and all sub structures.
 *
 * \param m
 *  the Mutex to free.
 *
 * \return A null Mutex pointer.
 *
 * Frees a Mutex and all sub structures.
 *
 * \sa P_FreeMutex(), P_NewMutex()
 */
Mutex
P_RemoveMutex (Mutex m)
{
  if (m)
    {
      m->expression = P_RemoveExpr (m->expression);
      m->statement = P_RemoveStmt (m->statement);

      m = P_FreeMutex (m);
    }

  return (m);
}

/*! \brief Frees a Cobegin struct.
 *
 * \param c
 *  the Cobegin struct to free.
 *
 * \return A null Cobegin pointer.
 *
 * Frees a Cobegin struct without touching sub structures.
 *
 * If the global \a check_fields_on_free is true, function will print a
 * warning for any non-null pointer field.
 *
 * \sa P_RemoveCobegin(), P_NewCobegin()
 */
Cobegin
P_FreeCobegin (Cobegin c)
{
  if (c)
    {
      if (check_fields_on_free)
	{
	  if (c->statements != NULL)
	    P_warn ("struct.c:P_FreeCobegin:%d MEMORY LEAK freeing Cobegin "
		    "with non-null\nstatements", __LINE__ - 1);
	}

      DISPOSE (c);
      c = NULL;
    }

  return (c);
}

/*! \brief Frees a Cobegin and all sub structures.
 *
 * \param c
 *  the Cobegin to free.
 *
 * \return A null Cobegin pointer.
 *
 * Frees a Cobegin and all sub structures.
 *
 * \sa P_FreeCobegin(), P_NewCobegin()
 */
Cobegin
P_RemoveCobegin (Cobegin c)
{
  if (c)
    {
      c->statements = P_RemoveStmt (c->statements);

      c = P_FreeCobegin (c);
    }
  
  return (c);
}

/*! \brief Frees a BodyStmt struct.
 *
 * \param b
 *  the BodyStmt struct to free.
 *
 * \return A null BodyStmt pointer.
 *
 * Frees a BodyStmt struct without touching sub structures.
 *
 * If the global \a check_fields_on_free is true, function will print a
 * warning for any non-null pointer field.
 *
 * \sa P_RemoveBodyStmt(), P_NewBodyStmt()
 */
BodyStmt
P_FreeBodyStmt (BodyStmt b)
{
  if (b)
    {
      if (check_fields_on_free)
	{
	  if (b->statement != NULL)
	    P_warn ("struct.c:P_FreeBodyStmt:%d MEMORY LEAK freeing BodyStmt "
		    "with non-null\nstatement", __LINE__ - 1);
	}

      DISPOSE (b);
      b = NULL;
    }

  return (b);
}

/*! \brief Frees a BodyStmt and all sub structures.
 *
 * \param b
 *  the BodyStmt to free.
 *
 * \return A null BodyStmt pointer.
 *
 * Frees a BodyStmt and all sub structures.
 *
 * \sa P_FreeBodyStmt(), P_NewBodyStmt()
 */
BodyStmt
P_RemoveBodyStmt (BodyStmt b)
{
  if (b)
    {
      b->statement = P_RemoveStmt (b->statement);

      b = P_FreeBodyStmt (b);
    }

  return (b);
}

/*! \brief Frees an EpilogueStmt struct.
 *
 * \param e
 *  the EpilogueStmt struct to free.
 *
 * \return A null EpilogueStmt pointer.
 *
 * Frees an EpilogueStmt struct without touching sub structures.
 *
 * If the global \a check_fields_on_free is true, function will print a
 * warning for any non-null pointer field.
 *
 * \sa P_RemoveEpilogueStmt(), P_NewEpilogueStmt()
 */
EpilogueStmt
P_FreeEpilogueStmt (EpilogueStmt e)
{
  if (e)
    {
      if (check_fields_on_free)
	{
	  if (e->statement != NULL)
	    P_warn ("struct.c:P_FreeEpilogueStmt:%d MEMORY LEAK freeing "
		    "EpilogueStmt with\nnon-null statement", __LINE__ - 1);
	}

      DISPOSE (e);
      e = NULL;
    }

  return (e);
}

/*! \brief Frees an EpilogueStmt and all sub structures.
 *
 * \param e
 *  the EpilogueStmt to free.
 *
 * \return A null EpilogueStmt pointer.
 *
 * Frees an EpilogueStmt and all sub structures.
 *
 * \sa P_FreeEpilogueStmt(), P_NewEpilogueStmt()
 */
EpilogueStmt
P_RemoveEpilogueStmt (EpilogueStmt e)
{
  if (e)
    {
      e->statement = P_RemoveStmt (e->statement);

      e = P_FreeEpilogueStmt (e);
    }

  return (e);
}

/*! \brief Frees a ParLoop struct.
 *
 * \param p
 *  the ParLoop struct to free.
 *
 * \return A null ParLoop pointer.
 *
 * Frees a ParLoop struct without freeing sub structures.  This function
 * frees a single list node, not the entire list.
 *
 * If the global \a check_fields_on_free is true, function will print a
 * warning for any non-null pointer field.
 *
 * \sa P_RemoveParLoopNode(), P_RemoveParLoop(), P_NewParLoop()
 */
ParLoop
P_FreeParLoop (ParLoop p)
{
  if (p)
    {
      if (check_fields_on_free)
	{
	  if (p->pstmt != NULL)
	    P_warn ("struct.c:P_FreeParLoop:%d MEMORY LEAK freeing ParLoop "
		    "with non-null\npstmt", __LINE__ - 1);
	  if (p->iteration_var != NULL)
	    P_warn ("struct.c:P_FreeParLoop:%d MEMORY LEAK freeing ParLoop "
		    "with non-null\niteration_var", __LINE__ - 1);
	  if (p->sibling != NULL)
	    P_warn ("struct.c:P_FreeParLoop:%d MEMORY LEAK freeing ParLoop "
		    "with non-null\nsibling", __LINE__ - 1);
	  if (p->init_value != NULL)
	    P_warn ("struct.c:P_FreeParLoop:%d MEMORY LEAK freeing ParLoop "
		    "with non-null\ninit_value", __LINE__ - 1);
	  if (p->final_value != NULL)
	    P_warn ("struct.c:P_FreeParLoop:%d MEMORY LEAK freeing ParLoop "
		    "with non-null\nfinal_value", __LINE__ - 1);
	  if (p->incr_value != NULL)
	    P_warn ("struct.c:P_FreeParLoop:%d MEMORY LEAK freeing ParLoop "
		    "with non-null\nincr_value", __LINE__ - 1);
	  if (p->child != NULL)
	    P_warn ("struct.c:P_FreeParLoop:%d MEMORY LEAK freeing ParLoop "
		    "with non-null\nchild", __LINE__ - 1);
	}
      
      DISPOSE (p);
      p = NULL;
    }

  return (p);
}

/*! \brief Frees a ParLoop node and all sub structures.
 *
 * \param p
 *  the ParLoop node to free.
 *
 * \return A null ParLoop pointer.
 *
 * Frees a ParLoop node and all sub structures.  This function frees
 * a single list node, not the entire list.
 *
 * It is the user's responsibility to repair any list pointers that may be
 * destroyed by this function.
 *
 * \sa P_FreeParLoop(), P_RemoveParLoop(), P_NewParLoop()
 */
ParLoop
P_RemoveParLoopNode (ParLoop p)
{
  if (p)
    {
      p->pstmt = P_RemovePstmt (p->pstmt);
      p->iteration_var = P_RemoveExpr (p->iteration_var);
      p->sibling = NULL;
      p->init_value = P_RemoveExpr (p->init_value);
      p->final_value = P_RemoveExpr (p->final_value);
      p->incr_value = P_RemoveExpr (p->incr_value);
      p->child = P_RemoveStmt (p->child);

      p = P_FreeParLoop (p);
    }

  return (p);
}

/*! \brief Frees a ParLoop list and all sub structures.
 *
 * \param p
 *  the ParLoop list to free.
 *
 * \return A null ParLoop pointer.
 *
 * Frees a ParLoop list and all sub structures.
 *
 * \sa P_FreeParLoop(), P_RemoveParLoopNode(), P_NewParLoop()
 */
ParLoop
P_RemoveParLoop (ParLoop p)
{
  if (p)
    {
#if 0
      p->sibling = P_RemoveParLoop (p->sibling);
#endif

      p = P_RemoveParLoopNode (p);
    }

  return (p);
}

/*! \brief Frees a SerLoop struct.
 *
 * \param s
 *  the SerLoop struct to free.
 *
 * \return A null SerLoop pointer.
 *
 * Frees a SerLoop struct without touching sub structures.
 *
 * If the global \a check_fields_on_free is true, function will print a
 * warning for any non-null pointer field.
 *
 * \sa P_RemoveSerLoop(), P_NewSerLoop()
 */
SerLoop
P_FreeSerLoop (SerLoop s)
{
  if (s)
    {
      if (check_fields_on_free)
	{
	  if (s->loop_body != NULL)
	    P_warn ("struct.c:P_FreeSerLoop:%d MEMORY LEAK freeing SerLoop "
		    "with non-null\nloop_body", __LINE__ - 1);
	  if (s->cond_expr != NULL)
	    P_warn ("struct.c:P_FreeSerLoop:%d MEMORY LEAK freeing SerLoop "
		    "with non-null\ncond_expr", __LINE__ - 1);
	  if (s->init_expr != NULL)
	    P_warn ("struct.c:P_FreeSerLoop:%d MEMORY LEAK freeing SerLoop "
		    "with non-null\ninit_expr", __LINE__ - 1);
	  if (s->iter_expr != NULL)
	    P_warn ("struct.c:P_FreeSerLoop:%d MEMORY LEAK freeing SerLoop "
		    "with non-null\niter_expr", __LINE__ - 1);
	}

      DISPOSE (s);
      s = NULL;
    }

  return (s);
}

/*! \brief Frees a SerLoop and all sub structures.
 *
 * \param s
 *  the SerLoop to free.
 *
 * \return A null SerLoop pointer.
 *
 * Frees a SerLoop and all sub structures.
 *
 * \sa P_FreeSerLoop(), P_NewSerLoop()
 */
SerLoop
P_RemoveSerLoop (SerLoop s)
{
  if (s)
    {
      s->loop_body = P_RemoveStmt (s->loop_body);
      s->cond_expr = P_RemoveExpr (s->cond_expr);
      s->init_expr = P_RemoveExpr (s->init_expr);
      s->iter_expr = P_RemoveExpr (s->iter_expr);

      s = P_FreeSerLoop (s);
    }

  return (s);
}

/*! \brief Frees an AsmStmt struct.
 *
 * \param a
 *  the AsmStmt struct to free.
 *
 * \return A null AsmStmt pointer.
 *
 * Frees an AsmStmt struct without touching sub structures.
 *
 * If the global \a check_fields_on_free is true, function will print a
 * warning for any non-null pointer field.
 *
 * \sa P_RemoveAsmStmt(), P_NewAsmStmt()
 */
AsmStmt
P_FreeAsmStmt (AsmStmt a)
{
  if (a)
    {
      if (check_fields_on_free)
	{
	  if (a->asm_clobbers != NULL)
	    P_warn ("struct.c:P_FreeAsmStmt:%d MEMORY LEAK freeing AsmStmt "
		    "with non-null\nclobbers", __LINE__ - 1);
	  if (a->asm_string != NULL)
	    P_warn ("struct.c:P_FreeAsmStmt:%d MEMORY LEAK freeing AsmStmt "
		    "with non-null\nstring", __LINE__ - 1);
	  if (a->asm_operands != NULL)
	    P_warn ("struct.c:P_FreeAsmStmt:%d MEMORY LEAK freeing AsmStmt "
		    "with non-null\noperands", __LINE__ - 1);
	}

      DISPOSE (a);
      a = NULL;
    }

  return (a);
}

/*! \brief Frees an AsmStmt and all sub structures.
 *
 * \param a
 *  the AsmStmt to free.
 *
 * \return A null AsmStmt pointer.
 *
 * Frees an AsmStmt and all sub structures.
 *
 * \sa P_FreeAsmStmt(), P_NewAsmStmt()
 */
AsmStmt
P_RemoveAsmStmt (AsmStmt a)
{
  if (a)
    {
      a->asm_clobbers = P_RemoveExpr (a->asm_clobbers);
      a->asm_string = P_RemoveExpr (a->asm_string);
      a->asm_operands = P_RemoveExpr (a->asm_operands);

      a = P_FreeAsmStmt (a);
    }

  return (a);
}

/*! \brief Frees an Asmoprd struct.
 *
 * \param a
 *  the Asmoprd struct to free.
 *
 * \return A null Asmoprd pointer.
 *
 * Frees an Asmoprd struct without touching sub structures.
 *
 * If the global \a check_fields_on_free is true, function will print a
 * warning for any non-null pointer field.
 *
 * \sa P_RemoveAsmoprd(), P_NewAsmoprd()
 */
Asmoprd
P_FreeAsmoprd (Asmoprd a)
{
  if (a)
    {
      if (check_fields_on_free)
	{
	  if (a->constraints != NULL)
	    P_warn ("struct.c:P_FreeAsmoprd:%d MEMORY LEAK freeing Asmoprd "
		    "with non-null\nconstraints", __LINE__ - 1);
	}

      DISPOSE (a);
      a = NULL;
    }

  return (a);
}

/*! \brief Frees an Asmoprd and all sub structures.
 *
 * \param a
 *  the Asmoprd to free.
 *
 * \return A null Asmoprd pointer.
 *
 * Frees an Asmoprd and all sub structures.
 *
 * \sa P_FreeAsmoprd(), P_NewAsmoprd()
 */
Asmoprd
P_RemoveAsmoprd (Asmoprd a)
{
  if (a)
    {
      if (a->constraints)
	{
	  free (a->constraints);
	  a->constraints = NULL;
	}

      a = P_FreeAsmoprd (a);
    }

  return (a);
}
	 
/*! \brief Frees an Expr struct.
 *
 * \param e
 *  the Expr struct to free.
 *
 * \return A null Expr pointer.
 *
 * Frees an Expr struct without freeing sub structures.  This function
 * frees a single list node, not the entire list.
 *
 * If the global \a check_fields_on_free is true, function will print a
 * warning for any non-null pointer field.
 *
 * \sa P_RemoveExprNode(), P_RemoveExpr(), P_NewExpr(),
 * P_NewExprWithOpcode(), P_NewStringExpr(), P_NewIntExpr(),
 * P_NewUIntExpr(), P_NewFloatExpr, P_NewDoubleExpr() */
Expr
P_FreeExpr (Expr e)
{
  if (e)
    {
      if (check_fields_on_free)
	{
	  switch (e->opcode)
	    {
	    case OP_char:
	    case OP_string:
	      if (e->value.string != NULL)
		P_warn ("struct.c:P_FreeExpr:%d MEMORY LEAK freeing Expr "
			"with non-null\nvalue.string", __LINE__ - 1);
	      break;
	    case OP_var:
	      if (e->value.var.name != NULL)
		P_warn ("struct.c:P_FreeExpr:%d MEMORY LEAK freeing Expr "
			"with non-null\nvalue.var.name", __LINE__ - 1);
	      break;
	    case OP_stmt_expr:
	      if (e->value.stmt != NULL)
		P_warn ("struct.c:P_FreeExpr:%d MEMORY LEAK freeing Expr "
			"with non-null\nvalue.stmt", __LINE__ - 1);
	      break;
	    case OP_asm_oprd:
	      if (e->value.asmoprd != NULL)
		P_warn ("struct.c:P_FreeExpr:%d MEMORY LEAK freeing Expr "
			"with non-null\nvalue.asmoprd", __LINE__ - 1);
	      break;
	    default:
	      break;
	    }
	  if (e->sibling != NULL)
	    P_warn ("struct.c:P_FreeExpr:%d MEMORY LEAK freeing Expr with "
		    "non-null sibling", __LINE__ - 1);
	  if (e->operands != NULL)
	    P_warn ("struct.c:P_FreeExpr:%d MEMORY LEAK freeing Expr with "
		    "non-null operands", __LINE__ - 1);
	  if (e->next != NULL)
	    P_warn ("struct.c:P_FreeExpr:%d MEMORY LEAK freeing Expr with "
		    "non-null next", __LINE__ - 1);
	  if (e->bb_next != NULL)
	    P_warn ("struct.c:P_FreeExpr:%d MEMORY LEAK freeing Expr with "
		    "non-null bb_next", __LINE__ - 1);
	  if (e->pragma != NULL)
	    P_warn ("struct.c:P_FreeExpr:%d MEMORY LEAK freeing Expr with "
		    "non-null pragma", __LINE__ - 1);
	  if (e->profile != NULL)
	    P_warn ("struct.c:P_FreeExpr:%d MEMORY LEAK freeing Expr with "
		    "non-null profile", __LINE__ - 1);
	  if (e->ext != NULL)
	    P_warn ("struct.c:P_FreeExpr:%d MEMORY LEAK freeing Expr with "
		    "non-null ext", __LINE__ - 1);
	}

      DISPOSE (e);
      e = NULL;
    }

  return (e);
}

/*! \brief Frees an Expr node and all sub structures.
 *
 * \param e
 *  the Expr node to free.
 *
 * \return A null Expr pointer.
 *
 * Frees an Expr node and all sub structures.  This function frees
 * a single list node, not the entire list.
 *
 * It is the user's responsibility to repair any list pointers that may be
 * destroyed by this function.
 *
 * \sa P_FreeExpr(), P_RemoveExpr(), P_NewExpr(),
 * P_NewExprWithOpcode(), P_NewStringExpr(), P_NewIntExpr(),
 * P_NewUIntExpr(), P_NewFloatExpr(), P_NewDoubleExpr() */
Expr
P_RemoveExprNode (Expr e)
{
  int i;

  if (e)
    {
      switch (e->opcode)
	{
	case OP_char:
	case OP_string:
	  if (e->value.string)
	    {
	      free (e->value.string);
	      e->value.string = NULL;
	    }
	  break;
	case OP_var:
	case OP_dot:
	case OP_arrow:
	  if (e->value.var.name)
	    {
	      free (e->value.var.name);
	      e->value.var.name = NULL;
	    }
	  break;
	case OP_stmt_expr:
	  e->value.stmt = P_RemoveStmt (e->value.stmt);
	  break;
	case OP_asm_oprd:
	  e->value.asmoprd = P_RemoveAsmoprd (e->value.asmoprd);
	  break;
	default:
	  break;
	}
      e->sibling = NULL;
      e->operands = P_RemoveExpr (e->operands);
      e->next = NULL;
      e->bb_next = P_RemoveExpr (e->bb_next);
      e->pragma = P_RemovePragma (e->pragma);
      e->profile = P_RemoveProfEXPR (e->profile);
      
      if (Handlers[ES_EXPR])
	{
	  for (i = 0; i < NumExtensions[ES_EXPR]; i++)
	    {
	      if (e->ext[i])
		e->ext[i] = Handlers[ES_EXPR][i].free (e->ext[i]);
	    }
	  
	  free (e->ext);
	  e->ext = NULL;
	}

      e = P_FreeExpr (e);
    }
  
  return (e);
}

/*! \brief Frees an Expr list and all sub structures.
 *
 * \param e
 *  the Expr list to free.
 *
 * \return A null Expr pointer.
 *
 * Frees an Expr list and all sub structures.
 *
 * \note Applications should typically use PST_RemoveExpr() instead.
 *
 * \sa PST_RemoveExpr(), P_FreeExpr(), P_RemoveExprNode(),
 * P_NewExpr(), P_NewExprWithOpcode(), P_NewStringExpr(),
 * P_NewIntExpr(), P_NewUIntExpr(), P_NewFloatExpr(),
 * P_NewDoubleExpr() */
Expr
P_RemoveExpr (Expr e)
{
  if (e)
    {
      e->sibling = P_RemoveExpr (e->sibling);
      e->next = P_RemoveExpr (e->next);

      e = P_RemoveExprNode (e);
    }

  return (e);
}

/*! \brief Frees a Pragma struct.
 *
 * \param p
 *  the Pragma struct to free.
 *
 * \return A null Pragma pointer.
 *
 * Frees a Pragma struct without freeing sub structures.  This function
 * frees a single list node, not the entire list.
 *
 * If the global \a check_fields_on_free is true, function will print a
 * warning for any non-null pointer field.
 *
 * \sa P_RemovePragmaNode(), P_RemovePragma(), P_NewPragma(), 
 * P_NewPragmaWithSpecExpr()
 */
Pragma
P_FreePragma (Pragma p)
{
  if (p)
    {
      if (check_fields_on_free)
	{
	  if (p->specifier != NULL)
	    P_warn ("struct.c:P_FreePragma:%d MEMORY LEAK freeing Pragma "
		    "with non-null\nspecifier", __LINE__ - 1);
	  if (p->expr != NULL)
	    P_warn ("struct.c:P_FreePragma:%d MEMORY LEAK freeing Pragma "
		    "with non-null\nexpr", __LINE__ - 1);
	  if (p->filename != NULL)
	    P_warn ("struct.c:P_FreePragma:%d MEMORY LEAK freeing Pragma "
		    "with non-null\nfilename", __LINE__ - 1);
#if 0
	  if (p->dep_info != NULL)
	    P_warn ("struct.c:P_FreePragma:%d MEMORY LEAK freeing Pragma "
		    "with non-null\ndep_info", __LINE__ - 1);
#endif
	  if (p->next != NULL)
	    P_warn ("struct.c:P_FreePragma:%d MEMORY LEAK freeing Pragma "
		    "with non-null\nnext", __LINE__ - 1);
	}

      DISPOSE (p);
      p = NULL;
    }

  return (p);
}

/*! \brief Frees a Pragma node and all sub structures.
 *
 * \param p
 *  the Pragma node to free.
 *
 * \return A null Pragma pointer.
 *
 * Frees a Pragma node and all sub structures.  This function frees
 * a single list node, not the entire list.
 *
 * It is the user's responsibility to repair any list pointers that may be
 * destroyed by this function.
 *
 * \sa P_FreePragma(), P_RemovePragma(), P_NewPragma(),
 * P_NewPragmaWithSpecExpr()
 */
Pragma
P_RemovePragmaNode (Pragma p)
{
  if (p)
    {
      if (p->specifier)
	{
	  free (p->specifier);
	  p->specifier = NULL;
	}
      p->expr = P_RemoveExpr (p->expr);
      if (p->filename)
	{
	  free (p->filename);
	  p->filename = NULL;
	}
#if 0
      p->dep_info = P_RemoveDepInfo (p->dep_info);
#endif
      p->next = NULL;

      p = P_FreePragma (p);
    }

  return (p);
}

/*! \brief Frees a Pragma list and all sub structures.
 *
 * \param p
 *  the Pragma list to free.
 *
 * \return A null Pragma pointer.
 *
 * Frees a Pragma list and all sub structures.
 *
 * \sa P_FreePragma(), P_RemovePragmaNode(), P_NewPragma(),
 * P_NewPragmaWithSpecExpr()
 */
Pragma
P_RemovePragma (Pragma p)
{
  Pragma next;

  while (p)
    {
      next = p->next;
      p = P_RemovePragmaNode (p);
      p = next;
    }

  return (p);
}

/*! \brief Frees a Position struct.
 *
 * \param p
 *  the Position struct to free.
 *
 * \return A null Position pointer.
 *
 * Frees a Position struct without touching sub structures.
 *
 * If the global \a check_fields_on_free is true, function will print a
 * warning for any non-null pointer field.
 *
 * \sa P_RemovePosition(), P_NewPosition()
 */
Position
P_FreePosition (Position p)
{
  if (p)
    {
      if (check_fields_on_free)
	{
	  if (p->filename != NULL)
	    P_warn ("struct.c:P_FreePosition:%d MEMORY LEAK freeing Position with non-null\nfilename");
	}

      DISPOSE (p);
      p = NULL;
    }

  return (p);
}

/*! \brief Frees a Position and all sub structures.
 *
 * \param p
 *  the Position to free.
 *
 * \return A null Position pointer.
 *
 * Frees a Position and all sub structures.
 *
 * \sa P_FreePosition(), P_NewPosition()
 */
Position
P_RemovePosition (Position p)
{
  if (p)
    {
      if (p->filename)
	{
	  free (p->filename);
	  p->filename = NULL;
	}

      p = P_FreePosition (p);
    }

  return (p);
}

/*! \brief Frees an Identifier struct.
 *
 * \param i
 *  the Identifier to free.
 *
 * \return A null Identifier pointer.
 *
 * Frees an Identifier struct without touching sub structures.
 *
 * If the global \a check_fields_on_free is true, function will print a
 * warning for any non-null pointer field.
 *
 * \sa P_RemoveIdentifer(), P_NewIdentifier()
 */
Identifier
P_FreeIdentifier (Identifier i)
{
  if (i)
    {
      if (check_fields_on_free)
	{
	  if (i->name != NULL)
	    P_warn ("struct.c:P_FreeIdentifier:%d MEMORY LEAK freeing "
		    "Identifier with\nnon-null name", __LINE__ - 1);
	}

      DISPOSE (i);
      i = NULL;
    }

  return (i);
}

/*! \brief Frees an Identifier and all sub structures.
 *
 * \param i
 *  the Identifier to free.
 *
 * \return A null Identifier pointer.
 *
 * Frees an Identifier and all sub structures.
 *
 * \sa P_FreeIdentifier(), P_NewIdentifier()
 */
Identifier
P_RemoveIdentifier (Identifier i)
{
  if (i)
    {
      if (i->name)
	{
	  free (i->name);
	  i->name = NULL;
	}

      i = P_FreeIdentifier (i);
    }

  return (i);
}

/*! \brief Frees a ProfFN struct.
 *
 * \param p
 *  the ProfFN struct to free.
 *
 * \return A null ProfFN pointer.
 *
 * Frees a ProfFN struct without touching sub structures.
 *
 * If the global \a check_fields_on_free is true, function will print a
 * warning for any non-null pointer field.
 *
 * \sa P_RemoveProfFN(), P_NewProfFN()
 */
ProfFN
P_FreeProfFN (ProfFN p)
{
  if (p)
    {
      if (check_fields_on_free)
	{
	  if (p->calls != NULL)
	    P_warn ("struct.c:P_FreeProfFN:%d MEMORY LEAK freeing ProfFn "
		    "with\nnon-null calls", __LINE__ - 1);
	}

      DISPOSE (p);
      p = NULL;
    }

  return (p);
}

/*! \brief Frees a ProfFN and all sub structures.
 *
 * \param p
 *  the ProfFN to free.
 *
 * \return A null ProfFN pointer.
 *
 * Frees a ProfFN and all sub structures.
 *
 * \sa P_FreeProfFN(), P_NewProfFN()
 */
ProfFN
P_RemoveProfFN (ProfFN p)
{
  if (p)
    {
      p->calls = P_RemoveProfCS (p->calls);

      p = P_FreeProfFN (p);
    }

  return (p);
}

/*! \brief Frees a ProfCS struct.
 *
 * \param p
 *  the ProfCS struct to free.
 *
 * \return A null ProfCS pointer.
 *
 * Frees a ProfCS struct without freeing sub structures.  This function
 * frees a single list node, not the entire list.
 *
 * If the global \a check_fields_on_free is true, function will print a
 * warning for any non-null pointer field.
 *
 * \sa P_RemoveProfCSNode(), P_RemoveProfCS(), P_NewProfCS()
 */
ProfCS
P_FreeProfCS (ProfCS p)
{
  if (p)
    {
      if (check_fields_on_free)
	{
	  if (p->next != NULL)
	    P_warn ("struct.c:P_FreeProfCS:%d MEMORY LEAK freeing ProfCS "
		    "with non-null\nnext", __LINE__ - 1);
	}

      DISPOSE (p);
      p = NULL;
    }

  return (p);
}

/*! \brief Frees a ProfCS node and all sub structures.
 *
 * \param p
 *  the ProfCS node to free.
 *
 * \return A null ProfCS pointer.
 *
 * Frees a ProfCS node and all sub structures.  This function frees
 * a single list node, not the entire list.
 *
 * It is the user's responsibility to repair any list pointers that may be
 * destroyed by this function.
 *
 * \sa P_FreeProfCS(), P_RemoveProfCS(), P_NewProfCS()
 */
ProfCS
P_RemoveProfCSNode (ProfCS p)
{
  if (p)
    {
      p->next = NULL;

      p = P_FreeProfCS (p);
    }

  return (p);
}

/*! \brief Frees a ProfCS list and all sub structures.
 *
 * \param p
 *  the ProfCS list to free.
 *
 * \return A null ProfCS pointer.
 *
 * Frees a ProfCS list and all sub structures.
 *
 * \sa P_FreeProfCS(), P_RemoveProfCSNode(), P_NewProfCS()
 */
ProfCS
P_RemoveProfCS (ProfCS p)
{
  ProfCS next;

  while (p)
    {
      next = p->next;
      p = P_RemoveProfCSNode (p);
      p = next;
    }

  return (p);
}

/*! \brief Frees a ProfBB struct.
 *
 * \param p
 *  the ProfBB struct to free.
 *
 * \return A null ProfBB pointer.
 *
 * Frees a ProfBB struct without touching sub structures.
 *
 * If the global \a check_fields_on_free is true, function will print a
 * warning for any non-null pointer field.
 *
 * \sa P_RemoveProfBB(), P_NewProfBB()
 */
ProfBB
P_FreeProfBB (ProfBB p)
{
  if (p)
    {
      if (check_fields_on_free)
	{
	  if (p->destination != NULL)
	    P_warn ("struct.c:P_FreeProfBB:%d MEMORY LEAK freeing ProfBB "
		    "with non-null\ndestination", __LINE__ - 1);
	}

      DISPOSE (p);
      p = NULL;
    }

  return (p);
}

/*! \brief Frees a ProfBB and all sub structures.
 *
 * \param p
 *  the ProfBB to free.
 *
 * \return A null ProfBB pointer.
 *
 * Frees a ProfBB and all sub structures.
 *
 * \sa P_FreeProfBB(), P_NewProfBB()
 */
ProfBB
P_RemoveProfBB (ProfBB p)
{
  if (p)
    {
      p->destination = P_RemoveProfArc (p->destination);

      p = P_FreeProfBB (p);
    }

  return (p);
}

/*! \brief Frees a ProfArc struct.
 *
 * \param p
 *  the ProfArc struct to free.
 *
 * \return A null ProfArc pointer.
 *
 * Frees a ProfArc struct without freeing sub structures.  This function
 * frees a single list node, not the entire list.
 *
 * If the global \a check_fields_on_free is true, function will print a
 * warning for any non-null pointer field.
 *
 * \sa P_RemoveProfArcNode(), P_RemoveProfArc(), P_NewProfArc()
 */
ProfArc
P_FreeProfArc (ProfArc p)
{
  if (p)
    {
      if (check_fields_on_free)
	{
	  if (p->next != NULL)
	    P_warn ("struct.c:P_FreeProfArc:%d MEMORY LEAK freeing ProfArc "
		    "with\nnon-null next", __LINE__ - 1);
	}

      DISPOSE (p);
      p = NULL;
    }
  
  return (p);
}

/*! \brief Frees a ProfArc node and all sub structures.
 *
 * \param p
 *  the ProfArc node to free.
 *
 * \return A null ProfArc pointer.
 *
 * Frees a ProfArc node and all sub structures.  This function frees
 * a single list node, not the entire list.
 *
 * It is the user's responsibility to repair any list pointers that may be
 * destroyed by this function.
 *
 * \sa P_FreeProfArc(), P_RemoveProfArc(), P_NewProfArc()
 */
ProfArc
P_RemoveProfArcNode (ProfArc p)
{
  if (p)
    {
      p->next = NULL;

      p = P_FreeProfArc (p);
    }

  return (p);
}

/*! \brief Frees a ProfArc list and all sub structures.
 *
 * \param p
 *  the ProfArc list to free.
 *
 * \return A null ProfArc pointer.
 *
 * Frees a ProfArc list and all sub structures.
 *
 * \sa P_FreeProfArc(), P_RemoveProfArcNode(), P_NewProfArc()
 */
ProfArc
P_RemoveProfArc (ProfArc p)
{
  ProfArc next;

  while (p)
    {
      next = p->next;
      p = P_RemoveProfArcNode (p);
      p = next;
    }

  return (p);
}

/*! \brief Frees a ProfST struct.
 *
 * \param p
 *  the ProfST struct to free.
 *
 * \return A null ProfST pointer.
 *
 * Frees a ProfST struct without freeing sub structures.  This function
 * frees a single list node, not the entire list.
 *
 * If the global \a check_fields_on_free is true, function will print a
 * warning for any non-null pointer field.
 *
 * \sa P_RemoveProfSTNode(), P_RemoveProfST(), P_NewProfST()
 */
ProfST
P_FreeProfST (ProfST p)
{
  if (p)
    {
      if (check_fields_on_free)
	{
	  if (p->next)
	    P_warn ("struct.c:P_FreeProfST:%d MEMORY LEAK freeing ProfST "
		    "with non-null\nnext", __LINE__ - 1);
	}

      DISPOSE (p);
      p = NULL;
    }

  return (p);
}

/*! \brief Frees a ProfST node and all sub structures.
 *
 * \param p
 *  the ProfSt node to free.
 *
 * \return A null ProfST pointer.
 *
 * Frees a ProfST node and all sub structures.  This function frees
 * a single list node, not the entire list.
 *
 * It is the user's responsibility to repair any list pointers that may be
 * destroyed by this function.
 *
 * \sa P_FreeProfST(), P_RemoveProfST(), P_NewProfST()
 */
ProfST
P_RemoveProfSTNode (ProfST p)
{
  if (p)
    {
      p->next = NULL;

      p = P_FreeProfST (p);
    }

  return (p);
}

/*! \brief Frees a ProfST list and all sub structures.
 *
 * \param p
 *  the ProfST list to free.
 *
 * \return A null ProfST pointer.
 *
 * Frees a ProfST list and all sub structures.
 *
 * \sa P_FreeProfST(), P_RemoveProfSTNode(), P_NewProfST()
 */
ProfST
P_RemoveProfST (ProfST p)
{
  ProfST next;

  while (p)
    {
      next = p->next;
      p = P_RemoveProfSTNode (p);
      p = next;
    }

  return (p);
}
	  
/*! \brief Frees a ProfEXPR struct.
 *
 * \param p
 *  the ProfEXPR struct to free.
 *
 * \return A null ProfEXPR pointer.
 *
 * Frees a ProfEXPR struct without freeing sub structures.  This function
 * frees a single list node, not the entire list.
 *
 * If the global \a check_fields_on_free is true, function will print a
 * warning for any non-null pointer field.
 *
 * \sa P_RemoveProfEXPRNode(), P_RemoveProfEXPR(), P_NewProfEXPR()
 */
ProfEXPR
P_FreeProfEXPR (ProfEXPR p)
{
  if (p)
    {
      if (check_fields_on_free)
	{
	  if (p->next != NULL)
	    P_warn ("struct.c:P_FreeProfEXPR:%d MEMORY LEAK freeing ProfEXPR "
		    "with\nnon-null next", __LINE__ - 1);
	}

      DISPOSE (p);
      p = NULL;
    }

  return (p);
}

/*! \brief Frees a ProfEXPR node and all sub structures.
 *
 * \param p
 *  the ProfEXPR node to free.
 *
 * \return A null ProfEXPR pointer.
 *
 * Frees a ProfEXPR node and all sub structures.  This function frees
 * a single list node, not the entire list.
 *
 * It is the user's responsibility to repair any list pointers that may be
 * destroyed by this function.
 *
 * \sa P_FreeProfEXPR(), P_RemoveProfEXPR(), P_NewProfEXPR()
 */
ProfEXPR
P_RemoveProfEXPRNode (ProfEXPR p)
{
  if (p)
    {
      p->next = NULL;

      p = P_FreeProfEXPR (p);
    }

  return (p);
}

/*! \brief Frees a ProfEXPR list and all sub structures.
 *
 * \param p
 *  the ProfEXPR list to free.
 *
 * \return A null ProfEXPR pointer.
 *
 * Frees a ProfEXPR list and all sub structures.
 *
 * \sa P_FreeProfEXPR(), P_RemoveProfEXPRNode(), P_NewProfEXPR()
 */
ProfEXPR
P_RemoveProfEXPR (ProfEXPR p)
{
  ProfEXPR next;

  while (p)
    {
      next = p->next;
      p = P_RemoveProfEXPRNode (p);
      p = next;
    }

  return (p);
}

/*! \brief Frees a Shadow struct.
 *
 * \param s
 *  the Shadow struct to free.
 *
 * \return A null Shadow pointer.
 *
 * Frees a Shadow struct without freeing sub structures.  This function
 * frees a single list node, not the entire list.
 *
 * If the global \a check_fields_on_free is true, function will print a
 * warning for any non-null pointer field.
 *
 * \note This function does not check the Shadow.next field.  All Shadows
 *       are linked to each other, the FuncDcl holds a pointer to the head
 *       of the list, and each shadowed statement has a pointer to its
 *       shadow.  The result is that there are potentially three copies of
 *       the pointer to a given Shadow.  Shadows are therefore freed when
 *       the the shadowed Stmt is freed.
 *
 * \sa P_RemoveShadow(), P_NewShadow()
 */
Shadow
P_FreeShadow (Shadow s)
{
  if (s)
    {
      if (check_fields_on_free)
	{
	  if (s->expr != NULL)
	    P_warn ("struct.c:P_FreeShadow:%d MEMORY LEAK freeing Shadow "
		    "with non-null\nexpr", __LINE__ - 1);
	}

      DISPOSE (s);
      s = NULL;
    }

  return (s);
}

/*! \brief Frees a Shadow node and all sub structures.
 *
 * \param s
 *  the Shadow node to free.
 *
 * \return A null Shadow pointer.
 *
 * Frees a Shadow node and all sub structures.  This function frees
 * a single list node, not the entire list.
 *
 * It is the user's responsibility to repair any list pointers that may be
 * destroyed by this function.
 *
 * \note This function does not free the Shadow.next field.  All Shadows
 *       are linked to each other, the FuncDcl holds a pointer to the head
 *       of the list, and each shadowed statement has a pointer to its
 *       shadow.  The result is that there are potentially three copies of
 *       the pointer to a given Shadow.  Shadows are therefore freed when
 *       the the shadowed Stmt is freed.
 *
 * \sa P_FreeShadow(), P_RemoveShadow(), P_NewShadow()
 */
Shadow
P_RemoveShadow (Shadow s)
{
  if (s)
    {
      /* The shadow expr pointer points to a statement's expr.  The
       * expr will be freed when the statement is freed. */
      s->expr = NULL;

      s = P_FreeShadow (s);
    }

  return (s);
}

/*! \brief Frees a AsmDcl struct.
 *
 * \param a
 *  the AsmDcl to free.
 *
 * \return A null AsmDcl pointer.
 *
 * Frees a AsmDcl struct without freeing sub structures.
 *
 * If the global \a check_fields_on_free is true, function will print a
 * warning for any non-null pointer field.
 *
 * \sa P_RemoveAsmDcl(), P_NewAsmDcl()
 */
AsmDcl
P_FreeAsmDcl (AsmDcl a)
{
  if (a)
    {
      if (check_fields_on_free)
	{
	  if (a->asm_clobbers)
	    P_warn ("struct.c:P_FreeAsmDcl:%d MEMORY LEAK freeing AsmDcl "
		    "with non-null\nasm_clobbers", __LINE__ - 1);
	  if (a->asm_string)
	    P_warn ("struct.c:P_FreeAsmDcl:%d MEMORY LEAK freeing AsmDcl "
		    "with non-null\nasm_string", __LINE__ - 1);
	  if (a->asm_operands)
	    P_warn ("string.c:P_FreeAsmDcl:%d MEMORY LEAK freeing AsmDcl "
		    "with non-null\nasm_operands", __LINE__ - 1);
	  if (a->filename)
	    P_warn ("string.c:P_FreeAsmDcl:%d MEMORY LEAK freeing AsmDcl "
		    "with non-null\nfilename", __LINE__ - 1);
	  if (a->ext)
	    P_warn ("string.c:P_FreeAsmDcl:%d MEMORY LEAK freeing AsmDcl "
		    "with non-null\next", __LINE__ - 1);
	}

      DISPOSE (a);
      a = NULL;
    }

  return (a);
}

/*! \brief Frees a AsmDcl and all sub structures.
 *
 * \param a
 *  the AsmDcl to free.
 *
 * \return A null AsmDcl pointer.
 *
 * Frees a AsmDcl and all sub structures.
 *
 * \sa P_FreeAsmDcl(), P_NewAsmDcl()
 */
AsmDcl
P_RemoveAsmDcl (AsmDcl a)
{
  int i;

  if (a)
    {
      a->asm_clobbers = P_RemoveExpr (a->asm_clobbers);
      a->asm_string = P_RemoveExpr (a->asm_string);
      a->asm_operands = P_RemoveExpr (a->asm_operands);
      if (a->filename)
	{
	  free (a->filename);
	  a->filename = NULL;
	}

      if (Handlers[ES_ASM])
	{
	  for (i = 0; i < NumExtensions[ES_ASM]; i++)
	    {
	      if (a->ext[i])
		a->ext[i] = Handlers[ES_ASM][i].free (a->ext[i]);
	    }
	  
	  free (a->ext);
	  a->ext = NULL;
	}

      a = P_FreeAsmDcl (a);
    }

  return (a);
}

/*! \brief Frees a Scope struct.
 *
 * \param s
 *  the Scope struct to free.
 *
 * \return A null Scope pointer.
 *
 * Frees a Scope struct without touching sub structures.
 *
 * If the global \a check_fields_on_free is true, function will print a
 * warning for any non-null pointer field.
 *
 * \sa P_RemoveScope(), P_NewScope(), P_NewScopeWithKey()
 */
Scope
P_FreeScope (Scope s)
{
  if (s)
    {
      if (check_fields_on_free)
	{
	  if (s->scope_entry != NULL)
	    P_warn ("struct.c:P_FreeScope:%d MEMORY LEAK freeing Scope "
		    "with non-null\nscope-var", __LINE__ - 1);
	}

      DISPOSE (s);
      s = NULL;
    }

  return (s);
}

/*! \brief Frees a Scope and all sub structures.
 *
 * \param s
 *  the Scope to free.
 *
 * \return A null Scope pointer.
 *
 * Frees a Scope and all sub structures.
 *
 * \sa P_FreeScope(), P_NewScope(), P_NewScopeWithKey()
 */
Scope
P_RemoveScope (Scope s)
{
  if (s)
    {
      s->scope_entry = P_RemoveScopeEntry (s->scope_entry);

      s = P_FreeScope (s);
    }

  return (s);
}

/*! \brief Frees a SymTabEntry struct.
 *
 * \param s
 *  the SymTabEntry struct to free.
 *
 * \return A null SymTabEntry pointer.
 *
 * Frees a SymTabEntry struct without touching sub structures.
 *
 * If the global \a check_fields_on_free is true, function will print a
 * warning for any non-null pointer field.
 *
 * \sa P_RemoveSymTabEntry(), P_NewSymTabEntry()
 */
SymTabEntry
P_FreeSymTabEntry (SymTabEntry s)
{
  if (s)
    {
      if (check_fields_on_free)
	{
	  if (P_GetSymTabEntryName (s) != NULL)
	    P_warn ("struct.c:P_FreeSymTabEntry:%d MEMORY LEAK freeing "
		    "SymTabEntry with\nnon-null name", __LINE__ - 1);

	  switch (P_GetSymTabEntryType (s))
	    {
	    case ET_FUNC:
	      if (P_GetSymTabEntryFuncDcl (s) != NULL)
		P_warn ("struct.c:P_FreeSymTabEntry:%d MEMORY LEAK freeing "
			"SymTabEntry with\nnon-null func_dcl", __LINE__ - 1);
	      break;
	    case ET_TYPE_GLOBAL:
	      if (P_GetSymTabEntryTypeDcl (s) != NULL)
		P_warn ("struct.c:P_FreeSymTabEntry:%d MEMORY LEAK freeing "
			"SymTabEntry with\nnon-null type", __LINE__ - 1);
	      break;
	    case ET_VAR_GLOBAL:
	      if (P_GetSymTabEntryVarDcl (s) != NULL)
		P_warn ("struct.c:P_FreeSymTabEntry:%d MEMORY LEAK freeing "
			"SymTabEntry with\nnon-null var_dcl", __LINE__ - 1);
	      break;
	    case ET_STRUCT:
	      if (P_GetSymTabEntryStructDcl (s) != NULL)
		P_warn ("struct.c:P_FreeSymTabEntry:%d MEMORY LEAK freeing "
			"SymTabEntry with\nnon-null struct_dcl", __LINE__ - 1);
	      break;
	    case ET_UNION:
	      if (P_GetSymTabEntryUnionDcl (s) != NULL)
		P_warn ("struct.c:P_FreeSymTabEntry:%d MEMORY LEAK freeing "
			"SymTabEntry with\nnon-null union_dcl", __LINE__ - 1);
	      break;
	    case ET_ENUM:
	      if (P_GetSymTabEntryEnumDcl (s) != NULL)
		P_warn ("struct.c:P_FreeSymTabEntry:%d MEMORY LEAK freeing "
			"SymTabEntry with\nnon-null enum_dcl", __LINE__ - 1);
	      break;
	    case ET_ASM:
	      if (P_GetSymTabEntryAsmDcl (s) != NULL)
		P_warn ("struct.c:P_FreeSymTabEntry:%d MEMORY LEAK freeing "
			"SymTabEntry with\nnon-null asm_dcl", __LINE__ - 1);
	      break;
	    case ET_TYPE_LOCAL:
	    case ET_VAR_LOCAL:
	    case ET_STMT:
	    case ET_EXPR:
	    case ET_FIELD:
	    case ET_ENUMFIELD:
	    case ET_LABEL:
	    case ET_SCOPE:
	    case ET_BLOCK:
	      break;
	    default:
	      P_punt ("struct.c:P_FreeSymTabEntry:%d Invalid entry type %d",
		      __LINE__ - 1, P_GetSymTabEntryType (s));
	    }

	  if (P_GetSymTabEntryScope (s) != NULL)
	    P_warn ("struct.c:P_FreeSymTabEntry:%d MEMORY LEAK freeing "
		    "SymTabEntry with\nnon-null expr", __LINE__ - 1);
	}

      DISPOSE (s);
      s = NULL;
    }

  return (s);
}

/*! \brief Frees a SymTabEntry and all sub structures.
 *
 * \param s
 *  the SymTabEntry to free.
 *
 * \return A null SymTabEntry pointer.
 *
 * Frees a SymTabEntry and all sub structures.
 *
 * \sa P_FreeSymTabEntry(), P_NewSymTabEntry()
 */
SymTabEntry
P_RemoveSymTabEntry (SymTabEntry s)
{
  int i;

  if (s)
    {
      if (P_GetSymTabEntryName (s))
	{
	  free (P_GetSymTabEntryName (s));
	  P_SetSymTabEntryName (s, NULL);
	}

      switch (P_GetSymTabEntryType (s))
	{
	case ET_FUNC:
	  s->entry.func_dcl = P_RemoveFuncDcl (s->entry.func_dcl);
	  break;
	case ET_TYPE_GLOBAL:
	  s->entry.type_dcl = P_RemoveTypeDcl (s->entry.type_dcl);
	  break;
	case ET_VAR_GLOBAL:
	  s->entry.var_dcl = P_RemoveVarDcl (s->entry.var_dcl);
	  break;
	case ET_STRUCT:
	  s->entry.struct_dcl = P_RemoveStructDcl (s->entry.struct_dcl);
	  break;
	case ET_UNION:
	  s->entry.union_dcl = P_RemoveUnionDcl (s->entry.union_dcl);
	  break;
	case ET_ENUM:
	  s->entry.enum_dcl = P_RemoveEnumDcl (s->entry.enum_dcl);
	  break;
	case ET_ASM:
	  s->entry.asm_dcl = P_RemoveAsmDcl (s->entry.asm_dcl);
	  break;
	case ET_TYPE_LOCAL:
	case ET_VAR_LOCAL:
	case ET_STMT:
	case ET_EXPR:
	case ET_FIELD:
	case ET_ENUMFIELD:
	case ET_LABEL:
	case ET_SCOPE:
	case ET_BLOCK:
	  /* Fields and EnumFields should be freed when their struct, union,
	   * or enum is freed. */
	  break;
	default:
	  P_punt ("struct.c:P_RemoveSymTabEntry:%d Invalid entry type %d",
		  __LINE__ - 1, P_GetSymTabEntryType (s));
	}

      s->scope = P_RemoveScope (s->scope);

      if (Handlers[ES_SYMTABENTRY])
	{
	  for (i = 0; i < NumExtensions[ES_SYMTABENTRY]; i++)
	    {
	      if (s->ext[i])
		s->ext[i] = Handlers[ES_SYMTABENTRY][i].free (s->ext[i]);
	    }
	  
	  free (s->ext);
	  s->ext = NULL;
	}

      s = P_FreeSymTabEntry (s);
    }

  return (s);
}

/*! \brief Frees an IPSymTabEnt struct.
 *
 * \param i
 *  the IPSymTabEnt struct to free.
 *
 * \return A null IPSymTabEnt pointer.
 *
 * Frees an IPSymTabEnt struct without touching sub structures.
 *
 * If the global \a check_fields_on_free is true, function will print a
 * warning for any non-null pointer field.
 *
 * \sa P_RemoveIPSymTabEnt(), P_NewIPSymTabEnt()
 */
IPSymTabEnt
P_FreeIPSymTabEnt (IPSymTabEnt i)
{
  if (i)
    {
      if (check_fields_on_free)
	{
	  if (i->source_name != NULL)
	    P_warn ("struct.c:P_FreeIPSymTabEnt:%d MEMORY LEAK freeing "
		    "IPSymTabEnt with\nnon-null source_name", __LINE__ - 1);
	  if (i->in_name != NULL)
	    P_warn ("struct.c:P_FreeIPSymTabEnt:%d MEMORY LEAK freeing "
		    "IPSymTabEnt with\nnon-null in_name", __LINE__ - 1);
	  if (i->out_name != NULL)
	    P_warn ("struct.c:P_FreeIPSymTabEnt:%d MEMORY LEAK freeing "
		    "IPSymTabEnt with\nnon-null out_name", __LINE__ - 1);
	  if (i->table != NULL)
	    P_warn ("struct.c:P_FreeIPSymTabEnt:%d MEMORY LEAK freeing "
		    "IPSymTabEnt with\nnon-null file_table", __LINE__ - 1);
	  if (i->file != NULL)
	    P_warn ("struct.c:P_FreeIPSymTabEnt:%d MEMORY LEAK freeing "
		    "IPSymTabEnt with\nnon-null file", __LINE__ - 1);
	}

      DISPOSE (i);
      i = NULL;
    }

  return (i);
}

/*! \brief Frees an IPSymTabEnt and all sub structures.
 *
 * \param i
 *  the IPSymTabEnt to free.
 *
 * \return A null IPSymTabEnt pointer.
 *
 * Frees an IPSymTabEnt and all sub structures.
 *
 * \sa P_FreeIPSymTabEnt(), P_NewIPSymTabEnt()
 */
IPSymTabEnt
P_RemoveIPSymTabEnt (IPSymTabEnt i)
{
  int index, j;
  SymTabEntry e;

  if (i)
    {  
      if (i->source_name)
	{
	  free (i->source_name);
	  i->source_name = NULL;
	}
      if (i->in_name)
	{
	  free (i->in_name);
	  i->in_name = NULL;
	}
      if (i->out_name)
	{
	  free (i->out_name);
	  i->out_name = NULL;
	}

      if (i->table)
	{
	  for (e = (SymTabEntry)BlockSparseArrayGetFirstNonZero (&(i->table),
								 &index);
	       e;
	       e = (SymTabEntry)BlockSparseArrayGetNextNonZero (&(i->table),
								&index))
	    {
	      e = P_RemoveSymTabEntry (e);
	      BlockSparseArrayClear (&(i->table), index);
	    }

	  FreeBlockSparseArray (i->table);
	  i->table = NULL;
	}

      if ((i->in_file_status != FS_CLOSED && \
	   i->in_file_status != FS_NOT_AVAIL) && i->file)
	{
	  P_file_close (i->file);
	  i->file = NULL;
	  i->in_file_status = FS_CLOSED;
	}

      if (Handlers[ES_IPSYMTABENT])
	{
	  for (j = 0; j < NumExtensions[ES_IPSYMTABENT]; j++)
	    {
	      if (i->ext[j])
		i->ext[j] = Handlers[ES_IPSYMTABENT][j].free (i->ext[j]);
	    }
	  
	  free (i->ext);
	  i->ext = NULL;
	}

      i = P_FreeIPSymTabEnt (i);
    }

  return (i);
}

/*! \brief Frees a SymbolTable struct.
 *
 * \param s
 *  the SymbolTable struct to free.
 *
 * \return A null SymbolTable pointer.
 *
 * Frees a SymbolTable struct without touching sub structures.
 *
 * If the global \a check_fields_on_free is true, function will print a
 * warning for any non-null pointer field.
 *
 * \sa P_RemoveSymbolTable(), P_NewSymbolTable()
 */
SymbolTable
P_FreeSymbolTable (SymbolTable s)
{
  if (s)
    {
      if (check_fields_on_free)
	{
	  if (s->ip_table_name != NULL)
	    P_warn ("struct.c:P_FreeSymbolTable:%d MEMORY LEAK freeing "
		    "SymbolTable with\nnon-null ip_table_name", __LINE__ - 1);
	  if (s->out_name != NULL)
	    P_warn ("struct.c:P_FreeSymbolTable:%d MEMORY LEAK freeing "
		    "SymbolTable with\nnon-null out_name", __LINE__ - 1);
	  if (s->ip_table != NULL)
	    P_warn ("struct.c:P_FreeSymbolTable:%d MEMORY LEAK freeing "
		    "SymbolTable with\nnon-null ip_table", __LINE__ - 1);
	  if (s->file != NULL)
	    P_warn ("struct.c:P_FreeSymbolTable:%d MEMORY LEAK freeing "
		    "SymbolTable with\nnon-null file", __LINE__ - 1);
	}
	  
      DISPOSE (s);
      s = NULL;
    }

  return (s);
}

/*! \brief Frees a SymbolTable and all sub structures.
 *
 * \param s
 *  the SymbolTable to free.
 *
 * \return A null SymbolTable pointer.
 *
 * Frees a SymbolTable and all sub structures.
 *
 * \sa P_FreeSymbolTable(), P_NewSymbolTable()
 */
SymbolTable
P_RemoveSymbolTable (SymbolTable s)
{
  int i;

#if 0
  /* We're freeing this table, so silence warnings about table consistency.
   */
  check_table_consistency_on_free = FALSE;
#endif

  if (s)
    {
      if (s->ip_table_name)
	{
	  free (s->ip_table_name);
	  s->ip_table_name = NULL;
	}
      if (s->out_name)
	{
	  free (s->out_name);
	  s->out_name = NULL;
	}
      
      if (s->ip_table)
	{
	  for (i = 1; i <= s->num_files; i++)
	    s->ip_table[i] = P_RemoveIPSymTabEnt (s->ip_table[i]);

	  free (s->ip_table);
	  s->ip_table = NULL;
	}
      
      if ((s->in_file_status != FS_CLOSED && \
	   s->in_file_status != FS_NOT_AVAIL) && s->file)
	{
	  P_file_close (s->file);
	  s->file = NULL;
	  s->in_file_status = FS_CLOSED;
	}

      s = P_FreeSymbolTable (s);
    }

#if 0
  check_table_consistency_on_free = TRUE;
#endif

  return (s);
}

/*! \brief Frees a KeyMap struct.
 *
 * \param k
 *  the KeyMap struct to free.
 *
 * \return A null KeyMap pointer.
 *
 * Frees a KeyMap struct without touching sub structures.
 *
 * If the global \a check_fields_on_free is true, function will print a
 * warning for any non-null pointer field.
 *
 * \sa P_RemoveKeyMap(), P_NewKeyMap()
 */
KeyMap
P_FreeKeyMap (KeyMap k)
{
  if (k)
    {
      DISPOSE (k);
      k = NULL;
    }
  
  return (k);
}

/*! \brief Frees a KeyMap and all sub structures.
 *
 * \param k
 *  the KeyMap to free.
 *
 * \return A null KeyMap pointer.
 *
 * Frees a KeyMap and all sub structures.
 *
 * \sa P_FreeKeyMap(), P_NewKeyMap(0
 */
KeyMap
P_RemoveKeyMap (KeyMap k)
{
  if (k)
    k = P_FreeKeyMap (k);

  return (k);
}


#if 0
/* CWL - 01/06/02 */
static void
P_RemoveDepInfo (DepInfo dep)
{
  DepInfo last_dep;

  while (dep)
    {
      last_dep = dep;
      dep = dep->next;
      DISPOSE (last_dep);
    }
}
#endif

/* if (pragma==0) remove all pragmas.
 * else only remove a single pragma if found.
 */
bool
P_RemoveStmtPragma (Stmt stmt, Pragma pragma)
{
  Pragma ptr;
  if (pragma == 0)
    {
      P_RemovePragma (stmt->pragma);
      stmt->pragma = 0;
      return TRUE;
    }
  for (ptr = stmt->pragma; ptr != 0; ptr = ptr->next)
    if (ptr == pragma)
      break;
  if (ptr == 0)
    return FALSE;		/* not found */
  ptr = stmt->pragma;
  if (pragma == ptr)
    {
      stmt->pragma = pragma->next;
    }
  else
    {
      while (ptr->next != pragma)
	ptr = ptr->next;
      ptr->next = pragma->next;
    }
  pragma->next = 0;
  P_RemovePragma (pragma);
  return TRUE;
}

/* if (pragma==0) remove all pragmas.
 * else only remove a single pragma if found.
 */
bool
P_RemoveExprPragma (Expr expr, Pragma pragma)
{
  Pragma ptr;
  if (pragma == 0)
    {
      P_RemovePragma (expr->pragma);
      expr->pragma = 0;
      return TRUE;
    }
  for (ptr = expr->pragma; ptr != 0; ptr = ptr->next)
    if (ptr == pragma)
      break;
  if (ptr == 0)
    return FALSE;		/* not found */
  ptr = expr->pragma;
  if (pragma == ptr)
    {
      expr->pragma = pragma->next;
    }
  else
    {
      while (ptr->next != pragma)
	ptr = ptr->next;
      ptr->next = pragma->next;
    }
  pragma->next = 0;
  P_RemovePragma (pragma);
  return TRUE;
}

/*!  \brief Converts a Key to an unsigned long.
 *
 * \param k
 *  the key.
 *
 * \return
 *  \a k encoded as an unsigned 64 bit word.
 *
 * \sa P_Long2Key()
 */
ITintmax
P_Key2Long (Key k)
{
  ITintmax result;

  result = ((ITintmax)k.file << 32) + (ITintmax)k.sym;

  return (result);
}

/*! \brief Converts an unsigned long to a Key.
 *
 * \param l
 *  the long.
 *
 * \return
 *  \a l as a Kye.
 *
 * \sa P_Key2Long()
 */
Key
P_Long2Key (ITintmax l)
{
  Key result;

  result.file = (l & ((ITintmax)0xffffffff << 32)) >> 32;
  result.sym = l & 0xffffffff;

  return result;
}

/* define when use profile info */

#if 0
void
P_RemoveStructUnionPoolElem (StructUnionPoolElem s)
{
  if (s)
    {
      switch (s->type)
	{
	case TY_STRUCT:
	  P_RemoveStructDcl (s->ptr.st);
	  break;
	case TY_UNION:
	  P_RemoveUnionDcl (s->ptr.un);
	  break;
	}

      P_RemoveStructUnionPoolElem (s->next);

      DISPOSE (s);
    }
}
#endif
  

/* Access functions */
/* Dcl access functions. */
/*! \brief Sets the Pragma field in a ::Dcl.
 *
 * \param d
 *  the ::Dcl.
 * \param p
 *  the Pragma to set.
 *
 * \return
 *  The new value of the pragma field (\a p).
 *
 * Sets the Pragma field of the appropriate sub structure of the ::Dcl.
 */
Pragma
P_SetDclPragma (Dcl d, Pragma p)
{
  if (d)
    {
      switch (P_GetDclType (d))
	{
	case TT_FUNC:
	case TT_ASM:
	case TT_INCLUDE:
	case TT_SYMBOLTABLE:
	case TT_IPSYMTABENT:
	case TT_SYMTABENTRY:
	  return (p);
	case TT_TYPE:
	  P_SetTypeDclPragma (P_GetDclTypeDcl (d), p);
	  break;
	case TT_VAR:
	  P_SetVarDclPragma (P_GetDclVarDcl (d), p);
	  break;
	case TT_STRUCT:
	  P_SetStructDclPragma (P_GetDclStructDcl (d), p);
	  break;
	case TT_UNION:
	  P_SetUnionDclPragma (P_GetDclUnionDcl (d), p);
	  break;
	case TT_ENUM:
	  P_SetEnumDclPragma (P_GetDclEnumDcl (d), p);
	  break;
	default:
	  P_punt ("struct.c:P_SetDclPragma:%d Unknown dcl type %d", __LINE__,
		  P_GetDclType (d));
	}
    }
  else
    P_punt ("struct.c:P_SetDclPragma:%d struct Dcl (d) is NULL", __LINE__);

  return (p);
}

/*! \brief Gets the Pragma field from a ::Dcl.
 *
 * \param d
 *  the ::Dcl.
 *
 * \return
 *  The value of the Pragma field of the Dcl.
 *
 * Returns the value of the Param field from the appropriate sub structure
 * of the ::Dcl.
 */
Pragma
P_GetDclPragma (Dcl d)
{
  if (d)
    {
      switch (P_GetDclType (d))
	{
	case TT_FUNC:
	case TT_ASM:
	case TT_INCLUDE:
	case TT_SYMBOLTABLE:
	case TT_IPSYMTABENT:
	case TT_SYMTABENTRY:
	  return (NULL);
	case TT_TYPE:
	  return (P_GetTypeDclPragma (P_GetDclTypeDcl (d)));
	case TT_VAR:
	  return (P_GetVarDclPragma (P_GetDclVarDcl (d)));
	case TT_STRUCT:
	  return (P_GetStructDclPragma (P_GetDclStructDcl (d)));
	case TT_UNION:
	  return (P_GetUnionDclPragma (P_GetDclUnionDcl (d)));
	case TT_ENUM:
	  return (P_GetEnumDclPragma (P_GetDclEnumDcl (d)));
	default:
	  P_punt ("struct.c:P_GetDclPragma:%d Unknown dcl type %d", __LINE__,
		  P_GetDclType (d));
	}
    }
  else
    {
      P_punt ("struct.c:P_GetDclPragma:%d struct Dcl (d) is NULL", __LINE__);
    }

  /* Should never reach this point. */
  return (NULL);
}

/*! \brief Sets the position of a ::Dcl.
 *
 * \param d
 *  the ::Dcl.
 * \param p
 *  the ::Position to set.
 *
 * \return
 *  The new Position (\a p).
 *
 * Sets the position of the appropriate sub structure of Dcl.
 *
 * \note The values from the ::Position struct are copied, so the caller is
 * responsible for freeing the ::Position passed as \a p.
 */
Position
P_SetDclPosition (Dcl d, Position p)
{
  if (d)
    {
      switch (P_GetDclType (d))
	{
	case TT_FUNC:
	  P_SetFuncDclPosition (P_GetDclFuncDcl (d), p);
	  break;
	case TT_TYPE:
	  P_SetTypeDclPosition (P_GetDclTypeDcl (d), p);
	  break;
	case TT_VAR:
	  P_SetVarDclPosition (P_GetDclVarDcl (d), p);
	  break;
	case TT_STRUCT:
	  P_SetStructDclPosition (P_GetDclStructDcl (d), p);
	  break;
	case TT_UNION:
	  P_SetUnionDclPosition (P_GetDclUnionDcl (d), p);
	  break;
	case TT_ENUM:
	  P_SetEnumDclPosition (P_GetDclEnumDcl (d), p);
	  break;
	case TT_ASM:
	  P_SetAsmDclPosition (P_GetDclAsmDcl (d), p);
	  break;
	case TT_INCLUDE:
	case TT_SYMBOLTABLE:
	case TT_IPSYMTABENT:
	case TT_SYMTABENTRY:
	  break;
	default:
	  P_punt ("struct.c:P_SetDclPosition:%d Unknown dcl type %d", __LINE__,
		  P_GetDclType (d));
	}
    }
  else
    P_punt ("struct.c:P_SetDclPosition:%d struct Dcl (d) is NULL", __LINE__);

  return (p);
}

/*! \brief Gets the position of a ::Dcl.
 *
 * \param d
 *  the ::Dcl.
 *
 * \return
 *  A pointer to a ::Position struct for the ::Dcl.
 *
 * Returns the position of the appropriate sub structure of Dcl.
 *
 * \note This function allocates a new ::Position and returns a pointer.  It
 * is the caller's responsibility to free the ::Position using
 * P_RemovePosition().
 *
 * \sa P_RemovePosition() */
Position
P_GetDclPosition (Dcl d)
{
  if (d)
    {
      switch (P_GetDclType (d))
	{
	case TT_FUNC:
	  return (P_GetFuncDclPosition (P_GetDclFuncDcl (d)));
	case TT_TYPE:
	  return (P_GetTypeDclPosition (P_GetDclTypeDcl (d)));
	case TT_VAR:
	  return (P_GetVarDclPosition (P_GetDclVarDcl (d)));
	case TT_STRUCT:
	  return (P_GetStructDclPosition (P_GetDclStructDcl (d)));
	case TT_UNION:
	  return (P_GetUnionDclPosition (P_GetDclUnionDcl (d)));
	case TT_ENUM:
	  return (P_GetEnumDclPosition (P_GetDclEnumDcl (d)));
	case TT_ASM:
	  return (P_GetAsmDclPosition (P_GetDclAsmDcl (d)));
	  break;
	case TT_INCLUDE:
	case TT_SYMBOLTABLE:
	case TT_IPSYMTABENT:
	case TT_SYMTABENTRY:
	  break;
	default:
	  P_punt ("struct.c:P_GetDclPosition:%d Unknown dcl type %d", __LINE__,
		  P_GetDclType (d));
	}
    }
  else
    P_punt ("struct.c:P_GetDclPosition:%d struct Dcl (d) is NULL", __LINE__);

  return (NULL);
}

/*! \brief Sets the key for a ::Dcl.
 *
 * \param d
 *  the ::Dcl.
 * \param k
 *  the ::Key to set.
 *
 * \return
 *  The new Key (\a k).
 *
 * Sets the key of the appropriate sub structure of Dcl.
 */
Key
P_SetDclKey (Dcl d, Key k)
{
  if (d)
    {
      switch (P_GetDclType (d))
	{
	case TT_FUNC:
	  P_SetFuncDclKey (P_GetDclFuncDcl (d), k);
	  break;
	case TT_TYPE:
	  P_SetTypeDclKey (P_GetDclTypeDcl (d), k);
	  break;
	case TT_VAR:
	  P_SetVarDclKey (P_GetDclVarDcl (d), k);
	  break;
	case TT_STRUCT:
	  P_SetStructDclKey (P_GetDclStructDcl (d), k);
	  break;
	case TT_UNION:
	  P_SetUnionDclKey (P_GetDclUnionDcl (d), k);
	  break;
	case TT_ENUM:
	  P_SetEnumDclKey (P_GetDclEnumDcl (d), k);
	  break;
	case TT_ASM:
	  P_SetAsmDclKey (P_GetDclAsmDcl (d), k);
	  break;
	case TT_INCLUDE:
	case TT_SYMBOLTABLE:
	case TT_IPSYMTABENT:
	case TT_SYMTABENTRY:
	  break;
	default:
	  P_punt ("struct.c:P_SetDclKey:%d Unknown dcl type %d", __LINE__,
		  P_GetDclType (d));
	}
    }
  else
    P_punt ("struct.c:P_SetDclKey:%d struct Dcl (d) is NULL", __LINE__);

  return (k);
}

/*! \brief Gets the key for a ::Dcl.
 *
 * \param d
 *  the ::Dcl.
 *
 * \return
 *  The key for the ::Dcl.
 *
 * Returns the key for the appropriate sub structure of Dcl.
 */
Key
P_GetDclKey (Dcl d)
{
  if (d)
    {
      switch (P_GetDclType (d))
	{
	case TT_FUNC:
	  return (P_GetFuncDclKey (P_GetDclFuncDcl (d)));
	case TT_TYPE:
	  return (P_GetTypeDclKey (P_GetDclTypeDcl (d)));
	case TT_VAR:
	  return (P_GetVarDclKey (P_GetDclVarDcl (d)));
	case TT_STRUCT:
	  return (P_GetStructDclKey (P_GetDclStructDcl (d)));
	case TT_UNION:
	  return (P_GetUnionDclKey (P_GetDclUnionDcl (d)));
	case TT_ENUM:
	  return (P_GetEnumDclKey (P_GetDclEnumDcl (d)));
	case TT_ASM:
	  return (P_GetAsmDclKey (P_GetDclAsmDcl (d)));
	  break;
	case TT_INCLUDE:
	case TT_SYMBOLTABLE:
	case TT_IPSYMTABENT:
	case TT_SYMTABENTRY:
	  break;
	default:
	  P_punt ("struct.c:P_GetDclKey:%d Unknown dcl type %d", __LINE__,
		  P_GetDclType (d));
	}
    }
  else
    P_punt ("struct.c:P_GetDclKey:%d struct Dcl (d) is NULL", __LINE__);

  return (Invalid_Key);
}

/* FuncDcl access functions. */
/*! \brief Sets the FuncDcl.stmt field.
 *
 * \param f
 *  the ::FuncDcl.
 * \param s
 *  the new value of the FuncDcl.stmt field.
 *
 * \return
 *  The new value of the FuncDcl.stmt field.
 *
 * Sets the FuncDcl.stmt field to \a s.  Sets the parent_func field of \a s
 * to \a f.
 */
Stmt
P_SetFuncDclStmt (FuncDcl f, Stmt s)
{
  if (f)
    {
      if (f->stmt && P_GetStmtParentFunc (f->stmt) == f)
	P_SetStmtParentFunc (f->stmt, NULL);

      f->stmt = s;

      if (f->stmt)
	{
	  P_SetStmtParentStmt (f->stmt, NULL);
	  P_SetStmtParentFunc (f->stmt, f);
	  P_SetStmtParentExpr (f->stmt, NULL);
	}
    }
  else
    P_punt ("struct.c:P_SetFuncDclStmt:%d FuncDcl (f) is NULL", __LINE__);
  
  return (s);
}

/*! \brief Sets the position of a ::FuncDcl.
 *
 * \param f
 *  the ::FuncDcl.
 * \param p
 *  the new position of the ::FuncDcl.
 *
 * \return
 *  The new position of the ::FuncDcl (\a p).
 *
 * \note The values from the ::Position struct are copied, so the caller is
 * responsible for freeing the ::Position passed as \a p.
 */
Position
P_SetFuncDclPosition (FuncDcl f, Position p)
{
  char *filename;

  if (f)
    {
      P_SetFuncDclLineno (f, P_GetPositionLineno (p));
      P_SetFuncDclColno (f, P_GetPositionColno (p));
      if ((filename = P_GetPositionFilename (p)))
	P_SetFuncDclFilename (f, strdup (filename));
    }
  else
    P_punt ("struct.c:P_SetFuncDclPosition:%d struct FuncDcl (f) is NULL",
	    __LINE__ - 1);

  return (p);
}

/*! \brief Gets the position of a ::FuncDcl.
 *
 * \param f
 *  the ::FuncDcl.
 *
 * \return
 *  A pointer to a ::Position struct for the ::FuncDcl.
 *
 * \note This function allocates a new ::Position and returns a pointer.  It
 * is the caller's responsibility to free the ::Position using
 * P_RemovePosition().
 *
 * \sa P_RemovePosition() */
Position
P_GetFuncDclPosition (FuncDcl f)
{
  Position result;
  char *filename;

  if (f)
    {
      result = P_NewPosition ();

      P_SetPositionLineno (result, P_GetFuncDclLineno (f));
      P_SetPositionColno (result, P_GetFuncDclColno (f));
      if ((filename = P_GetFuncDclFilename (f)))
	P_SetPositionFilename (result, strdup (filename));

      return (result);
    }
  else
    P_punt ("struct.c:P_GetFuncDclPosition:%d struct FuncDcl (f) is NULL",
	    __LINE__ - 1);

  /* Should never reach this point. */
  return (NULL);
}

/*! \brief Sets the identifier (key and name) of a ::FuncDcl.
 *
 * \param f
 *  the ::FuncDcl.
 * \param i
 *  the new identifier.
 *
 * \return
 *  The new identifier (\a i).
 *
 * \note The values from the ::Identifier struct are copied, so the caller is
 * responsible for freeing the ::Identifier passed as \a i.
 */
Identifier
P_SetFuncDclIdentifier (FuncDcl f, Identifier i)
{
  char *name;

  if (f)
    {
      if (f->name)
	free (f->name);
      if ((name = P_GetIdentifierName (i)))
	P_SetFuncDclName (f, strdup (name));
      else
	P_SetFuncDclName (f, NULL);

      P_SetFuncDclKey (f, P_GetIdentifierKey (i));
    }

  return (i);
}

/*! \brief Gets the identifier (name and key) from a ::FuncDcl.
 *
 * \param f
 *  the ::FuncDcl.
 *
 * \return
 *  A pointer to an ::Identifier struct for the ::FuncDcl.
 *
 * \note This function allocates a new ::Identifier and returns a pointer.
 * It is the caller's responsibility to free the ::Identifier using
 * P_RemoveIdentifier().
 *
 * \sa P_RemoveIdentifier() */
Identifier
P_GetFuncDclIdentifier (FuncDcl f)
{
  Identifier result;
  char *name;

  if (f)
    {
      result = P_NewIdentifier ();

      P_SetIdentifierKey (result, P_GetFuncDclKey (f));
      if ((name = P_GetFuncDclName (f)))
	P_SetIdentifierName (result, strdup (name));

      return (result);
    }
  else
    P_punt ("struct.c:P_GetFuncDclIdentifier:%d struct FuncDcl (f) is NULL",
	    __LINE__ - 1);

  /* Should never reach this point. */
  return (NULL);
}

/* TypeDcl access functions. */
/*! \brief Sets the position of a ::TypeDcl.
 *
 * \param t
 *  the TypeDcl.
 * \param p
 *  the new position of the ::TypeDcl.
 *
 * \return
 *  the new position of the ::TypeDcl (\a p).
 *
 * \note The values from the ::Position struct are copied, so the caller is
 * responsible for freeing the ::Position passed as \a p.
 */
Position
P_SetTypeDclPosition (TypeDcl t, Position p)
{
  char *f;

  if (t)
    {
      P_SetTypeDclLineno (t, P_GetPositionLineno (p));
      P_SetTypeDclColno (t, P_GetPositionColno (p));
      if ((f = P_GetPositionFilename (p)))
	P_SetTypeDclFilename (t, strdup (f));
    }
  else
    P_punt ("struct.c:P_SetTypeDclPosition:%d struct TypeDcl (t) is NULL",
	    __LINE__ - 1);

  return (p);
}

/* \brief Gets the position of a ::TypeDcl.
 *
 * \param t
 *  the ::TypeDcl.
 *
 * \return
 *  A pointer to a ::Position struct for the ::TypeDcl.
 *
 * \note This function allocates a new ::Position and returns a pointer.  It
 * is the caller's responsiblity to free the ::Position using
 * P_RemovePosition().
 *
 * \sa P_RemovePosition() */
Position
P_GetTypeDclPosition (TypeDcl t)
{
  Position result;
  char *f;

  if (t)
    {
      result = P_NewPosition ();
      
      P_SetPositionLineno (result, P_GetTypeDclLineno (t));
      P_SetPositionColno (result, P_GetTypeDclColno (t));
      if ((f = P_GetTypeDclFilename (t)))
	P_SetPositionFilename (result, strdup (f));

      return (result);
    }
  else
    P_punt ("struct.c:P_GetTypeDclPosition:%d struct TypeDcl (t) is NULL",
	    __LINE__ - 1);

  /* Should never reach this point. */
  return (NULL);
}

/*! \brief Sets the identifier (key and name) of a ::TypeDcl.
 *
 * \param t
 *  the ::TypeDcl.
 * \param i
 *  the new identifier.
 *
 * \return
 *  the new identifier (\a i).
 *
 * \note The values from the ::Identifier struct are copied, so the caller is
 * responsible for freeing the ::Identifier passed as \a i.
 */
Identifier
P_SetTypeDclIdentifier (TypeDcl t, Identifier i)
{
  char *name;

  if (t)
    {
      if (t->name)
	free (t->name);
      if ((name = P_GetIdentifierName (i)))
	P_SetTypeDclName (t, strdup (name));
      else
	P_SetTypeDclName (t, NULL);

      P_SetTypeDclKey (t, P_GetIdentifierKey (i));
    }

  return (i);
}

/*! \brief Gets the identifier (name and key) from a ::TypeDcl.
 *
 * \param t
 *  the ::TypeDcl.
 *
 * \return
 *  A pointer to an ::Identifier struct for the ::TypeDcl.
 *
 * \note This function alloates a new ::Identifier and returns a pointer.
 * It is the caller's responsibility to free the ::Identifier using
 * P_RemoveIdentifier().
 *
 * \sa P_RemoveIdentifier() */
Identifier
P_GetTypeDclIdentifier (TypeDcl t)
{
  Identifier result;
  char *name;

  if (t)
    {
      result = P_NewIdentifier ();

      P_SetIdentifierKey (result, P_GetTypeDclKey (t));
      if ((name = P_GetTypeDclName (t)))
	P_SetIdentifierName (result, strdup (name));

      return (result);
    }
  else
    P_punt ("struct.c:P_GetTypeDclIdentifier:%d struct TypeDcl (t) is NULL",
	    __LINE__ - 1);

  /* Should never reach this point. */
  return (NULL);
}

/* KeyList access functions. */
/*! \brief Appends a ::KeyList to a ::KeyList list.
 *
 * \param l
 *  the head of the list to append.
 * \param n
 *  the ::KeyList to append to the list.
 *
 * \return
 *  A pointer to the head of the list.
 *
 * Appends ::KeyList \a n to the tail of the list headed by \a l.  \a l may
 * be null, in which case \a n is returned as the head of the list.
 */
KeyList
P_AppendKeyListNext (KeyList l, KeyList n)
{
  if (l)
    l->last->next = n;
  else
    l = n;

  /* Update the last pointer. */
  l->last = n;

  return (l);
}

/*! \brief Merges two KeyLists.
 *
 * \param l
 *  the head of the first list to merge.
 * \param n
 *  the head of the second list to merge.
 *
 * \return
 *  A pointer to the head of the list.
 *
 * Merges list \a n into \a l and returns the head of the list.  \a l may
 * be null, in which case \a n is returned as the head of the list.
 *
 * Only keys that do not exist in \a l are copied from \a n.  KeyList nodes
 * that are not merged are freed by this function.  The caller must free
 * the returned KeyList.
 */
KeyList
P_MergeKeyListNext (KeyList l, KeyList n)
{
  KeyList t = NULL;

  if (l == NULL)
    return (n);

  while (n)
    {
      t = P_GetKeyListNext (n);
      P_SetKeyListNext (n, NULL);

      if (P_FindKeyListKey (l, P_GetKeyListKey (n)) == NULL)
	{
	  l->last->next = n;
	  l->last = n;
	}
      else
	{
	  n = P_FreeKeyList (n);
	}

      n = t;
    }

  return (l);
}

/*! \brief Deletes a Key from a KeyList.
 *
 * \param l
 *  the KeyList to delete from.
 * \param k
 *  the Key to delete from \a l.
 *
 * \return
 *  the new head of the list (\a l or \a l->next) after deletion.
 */
KeyList
P_DeleteKeyListNext (KeyList l, Key k)
{
  KeyList t, p, result, last = NULL;

  result = l;
  p = NULL;
  if (l)
    last = l->last;

  for (t = l; t; p = t, t = P_GetKeyListNext (t))
    {
      if (P_MatchKey (k, P_GetKeyListKey (t)))
	{
	  if (p == NULL)
	    result = P_GetKeyListNext (t);
	  else
	    P_SetKeyListNext (p, P_GetKeyListNext (t));

	  if (t == last)
	    last = p;

	  P_SetKeyListNext (t, NULL);
	  t = P_RemoveKeyList (t);
	  break;
	}
    }

  /* Reset the last pointer in case we removed the head. */
  if (result)
    result->last = last;

  return (result);
}

/* VarDcl access functions. */
/*! \brief Sets the Init for a ::VarDcl.
 *
 * \param v
 *  the ::VarDcl.
 * \param i
 *  the new initializer for the ::VarDcl.
 *
 * \return
 *  The new initializer for the ::VarDcl (\a i).
 *
 * Sets the Expr.parentvar field in the Exprs in \a i to \a v.
 */
Init
P_SetVarDclInit (VarDcl v, Init i)
{
  if (i)
    P_SetInitParentVar (i, v);
  v->init = i;

  return (i);
}

/*! \brief Sets the position of a ::VarDcl.
 *
 * \param v
 *  the ::VarDcl.
 * \param p
 *  the new position of the ::VarDcl.
 *
 * \return
 *  The new position of the ::VarDcl (\a p).
 *
 * \note The values from the ::Position struct are copied, so the caller is
 * responsible for freeing the ::Position passed as \a p.
 */
Position
P_SetVarDclPosition (VarDcl v, Position p)
{
  char *f;

  if (v)
    {
      P_SetVarDclLineno (v, P_GetPositionLineno (p));
      P_SetVarDclColno (v, P_GetPositionColno (p));
      if ((f = P_GetPositionFilename (p)))
	P_SetVarDclFilename (v, strdup (f));
    }
  else
    P_punt ("struct.c:P_SetVarDclPosition:%d struct VarDcl (v) is NULL",
	    __LINE__ - 1);

  return (p);
}

/*! \brief Gets the position of a ::VarDcl.
 *
 * \param v
 *  the ::VarDcl.
 *
 * \return
 *  A pointer to a ::Position struct for the ::VarDcl.
 *
 * \note This function allocates a new ::Position and returns a pointer.  It
 * is the caller's responsibility to free the ::Position using
 * P_RemovePosition().
 *
 * \sa P_RemovePosition() */
Position
P_GetVarDclPosition (VarDcl v)
{
  Position result;
  char *f;

  if (v)
    {
      result = P_NewPosition ();

      P_SetPositionLineno (result, P_GetVarDclLineno (v));
      P_SetPositionColno (result, P_GetVarDclColno (v));
      if ((f = P_GetVarDclFilename (v)))
	P_SetPositionFilename (result, strdup (f));

      return (result);
    }
  else
    P_punt ("struct.c:P_GetVarDclPosition:%d struct VarDcl (v) is NULL",
	    __LINE__ - 1);

  /* Should never reach this point. */
  return (NULL);
}

/*! \brief Sets the identifier (key and name) of a ::VarDcl.
 *
 * \param v
 *  the ::VarDcl.
 * \param i
 *  the new identifier.
 *
 * \return
 *  the new identifier (\a i).
 *
 * \note The values from the ::Identifier struct are copied, so the caller is
 * responsible for freeing the ::Identifier passed as \a i.
 */
Identifier
P_SetVarDclIdentifier (VarDcl v, Identifier i)
{
  char *name;

  if (v)
    {
      if (v->name)
	free (v->name);
      if ((name = P_GetIdentifierName (i)))
	P_SetVarDclName (v, strdup (name));
      else
	P_SetVarDclName (v, NULL);

      P_SetVarDclKey (v, P_GetIdentifierKey (i));
    }

  return (i);
}

/*! \brief Gets the identifier (name and key) from a ::VarDcl.
 *
 * \param v
 *  the ::VarDcl.
 *
 * \return
 *  A pointer to an ::Identifier struct for the ::VarDcl.
 *
 * \note The function allocates a new ::Identifier and returns a
 * pointer.  It is the caller's responsibility to free the
 * ::Identifier using P_RemoveIdentifier().
 *
 * \sa P_RemoveIdentifier() */
Identifier
P_GetVarDclIdentifier (VarDcl v)
{
  Identifier result;
  char *name;

  if (v)
    {
      result = P_NewIdentifier ();

      P_SetIdentifierKey (result, P_GetVarDclKey (v));
      if ((name = P_GetVarDclName (v)))
	P_SetIdentifierName (result, strdup (name));

      return (result);
    }
  else
    P_punt ("struct.c:P_GetVarDclIdentifier:%d struct VarDcl (v) is NULL",
	    __LINE__ - 1);

  /* Should never reach this point. */
  return (NULL);
}

/* Init access functions. */
/*! \brief Appends an ::Init to an ::Init list.
 *
 * \param i
 *  the head of the list to append.
 * \param n
 *  the ::Init to append to the list.
 *
 * \return
 *  A pointer to the head of the list.
 *
 * Appends ::Init \a n to the tail of the list headed by \a i.  \a i may
 * be null, in which case \a n is returned as the head of the list.
 */
Init
P_AppendInitNext (Init i, Init n)
{
  Init t = i;

  if (i)
    {
      while (t->next)
	t = t->next;

      t->next = n;
    }
  else
    i = n;

  return (i);
}

/*! \brief Sets the parentvar field for all Exprs in an Init.
 *
 * \param i
 *  the init.
 * \param v
 *  the new parent VarDcl for \a i.
 *
 * \return
 *  The new parent VarDcl for \a i (\a v).
 */
VarDcl
P_SetInitParentVar (Init i, VarDcl v)
{
  Expr e;
  Init j;

  if ((e = P_GetInitExpr (i)))
    P_SetExprParentVar (e, v);
  if ((j = P_GetInitSet (i)))
    P_SetInitParentVar (j, v);
  if ((j = P_GetInitNext (i)))
    P_SetInitParentVar (j, v);

  return (v);
}

/* StructDcl access functions. */
/*! \brief Sets the StructDcl.fields field.
 *
 * \param s
 *  the ::StructDcl.
 * \param f
 *  the new value of the StructDcl.fields field.
 *
 * \return
 *  The new value of the StructDcl.fields field (\a f).
 *
 * Sets the StructDcl.fields field.  If StructDcl.key is defined, copies
 * the StructDcl's key to the Field.parent_key field.
 *
 * \note This function sets the StructDcl.fields field explicitly.  To
 *       maintain a Field list, see P_AppendStructDclFields().
 */
Field
P_SetStructDclFields (StructDcl s, Field f)
{
  if (s)
    {
      if (s->fields)
	P_SetFieldParentKeyAll (s->fields, Invalid_Key);

      s->fields = f;

      if (s->fields && P_ValidKey (s->key))
	P_SetFieldParentKeyAll (s->fields, s->key);
    }
  else
    P_punt ("struct.c:P_SetStructDclFields:%d struct StructDcl (s) is NULL",
	    __LINE__ - 1);

  return (f);
}

/*! \brief Appends a Field to the StructDcl.fields field.
 *
 * \param s
 *  the ::StructDcl.
 * \param f
 *  the field to append to the StructDcl.fields field.
 *
 * \return
 *  The head of the StructDcl.field list.
 *
 * Appends \a f to the \a s.fields list.  If StructDcl.key is defined, copies
 * the StructDcl's key to the Field.parent_key field.
 *
 * \note This function maintains a Field list.  To set the StructDcl.fields
 *       field explicitly, see P_SetStructDclFields().
 */
Field
P_AppendStructDclFields (StructDcl s, Field f)
{
  Field result = NULL;

  if (s)
    {
      if (f && P_ValidKey (s->key))
	P_SetFieldParentKeyAll (f, s->key);

      s->fields = P_AppendFieldNext (s->fields, f);

      result = s->fields;
    }
  else
    P_punt ("struct.c:P_AppendStructDclFields:%d struct StructDcl (s) is NULL",
	    __LINE__ - 1);

  return (result);
}

/*! \brief Sets the position of a ::StructDcl.
 *
 * \param s
 *  the ::StructDcl.
 * \param p
 *  the new position of the ::StructDcl.
 *
 * \return
 *  The new position of the ::StructDcl (\a p).
 *
 * \note The values from the ::Position struct are copied, so the caller is
 * responsible for freeing the ::Position passed as \a p.
 */
Position
P_SetStructDclPosition (StructDcl s, Position p)
{
  char *f;

  if (s)
    {
      P_SetStructDclLineno (s, P_GetPositionLineno (p));
      P_SetStructDclColno (s, P_GetPositionColno (p));
      if ((f = P_GetPositionFilename (p)))
	P_SetStructDclFilename (s, strdup (f));
    }
  else
    P_punt ("struct.c:P_SetStructDclPosition:%d struct StructDcl (s) is NULL",
	    __LINE__ - 1);

  return (p);
}

/*! \brief Gets the position of a ::StructDcl.
 *
 * \param s
 *  the ::StructDcl.
 *
 * \return
 *  A pointer to a ::Position struct for the ::StructDcl.
 *
 * \note This function allocates a new ::Position and returns a pointer.  It
 * is the caller's responsibility to free the ::Position using
 * P_RemovePosition().
 *
 * \sa P_RemovePosition() */
Position
P_GetStructDclPosition (StructDcl s)
{
  Position result;
  char *f;

  if (s)
    {
      result = P_NewPosition ();

      P_SetPositionLineno (result, P_GetStructDclLineno (s));
      P_SetPositionColno (result, P_GetStructDclColno (s));
      if ((f = P_GetStructDclFilename (s)))
	P_SetPositionFilename (result, strdup (f));

      return (result);
    }
  else
    P_punt ("struct.c:P_GetStructDclPosition:%d struct StructDcl (s) is NULL",
	    __LINE__ - 1);

  /* Should never reach this point. */
  return (NULL);
}

/*! \brief Sets the identifier (key and name) of a ::StructDcl.
 *
 * \param s
 *  the ::StructDcl.
 * \param i
 *  the new identifier.
 *
 * \return
 *  The new identifier (\a i).
 *
 * \note The values from the ::Identifier struct are copied, so the caller is
 * responsible for freeing the ::Identifier passed as \a i.
 */
Identifier
P_SetStructDclIdentifier (StructDcl s, Identifier i)
{
  char *name;

  if (s)
    {
      if (s->name)
	free (s->name);
      if ((name = P_GetIdentifierName (i)))
	P_SetStructDclName (s, strdup (name));
      else
	P_SetStructDclName (s, NULL);

      P_SetStructDclKey (s, P_GetIdentifierKey (i));
    }

  return (i);
}

/*! \brief Gets the identifier (name and key) from a ::StructDcl.
 *
 * \param s
 *  the ::StructDcl.
 *
 * \return
 *  A pointer to an ::Identifier struct for the ::StructDcl.
 *
 * \note This function allocates a new ::Identifier and returns a pointer.
 * It is the caller's repsonsibility to free the ::Identifier using
 * P_RemoveIdentifier().
 *
 * \sa P_RemoveIdentifier() */
Identifier
P_GetStructDclIdentifier (StructDcl s)
{
  Identifier result;
  char *name;

  if (s)
    {
      result = P_NewIdentifier ();

      P_SetIdentifierKey (result, P_GetStructDclKey (s));
      if ((name = P_GetStructDclName (s)))
	P_SetIdentifierName (result, strdup (name));

      return (result);
    }
  else
    P_punt ("struct.c:P_GetStructDclIdentifier:%d struct StructDcl (s) is "
	    "NULL", __LINE__ - 1);

  /* Should never reach this point. */
  return (NULL);
}

/* UnionDcl access functions. */
/*! \brief Sets the UnionDcl.fields field.
 *
 * \param u
 *  the ::UnionDcl.
 * \param f
 *  the new value of the UnionDcl.fields field.
 *
 * \return
 *  The new value of the UnionDcl.fields field (\a f).
 *
 * Sets the UnionDcl.fields fields.  If the UnionDcl.key field is defined,
 * sets the Field.parent_key field on \a f.
 *
 * \note This function explicitly sets the UnionDcl.fields field.  To
 *       maintain a field list, see P_AppendUnionDclFields().
 */
Field
P_SetUnionDclFields (UnionDcl u, Field f)
{
  if (u)
    {
      if (u->fields)
	P_SetFieldParentKeyAll (u->fields, Invalid_Key);

      u->fields = f;

      if (u->fields && P_ValidKey (u->key))
	P_SetFieldParentKeyAll (u->fields, u->key);
    }
  else
    P_punt ("struct.c:P_SetUnionDclFields:%d struct UnionDcl (u) is NULL",
	    __LINE__ - 1);

  return (f);
}

/*! \brief Appends a field to the UnionDcl.fields list.
 *
 * \param u
 *  the ::UnionDcl.
 * \param f
 *  the field to append to the UnionDcl.fields list.
 *
 * \return
 *  The head of the UnionDcl.field list.
 *
 * Appends \a f to the \a u.fields list.  If \a u.key is set, the UnionDcl's
 * key is copied to \a f.
 *
 * \note This function maintains the a Field list.  To set UnionDcl.fields
 *       explicitly, see P_SetUnionDclFields().
 */
Field
P_AppendUnionDclFields (UnionDcl u, Field f)
{
  Field result = NULL;

  if (u)
    {
      if (f && P_ValidKey (u->key))
	P_SetFieldParentKeyAll (f, u->key);

      u->fields = P_AppendFieldNext (u->fields, f);

      result = u->fields;
    }
  else
    P_punt ("struct.c:P_AppendUnionDclFields:%d struct UnionDcl (u) is NULL",
	    __LINE__ - 1);

  return (result);
}

/*! \brief Sets the position of an ::UnionDcl.
 *
 * \param u
 *  the ::UnionDcl.
 * \param p
 *  the new position of the ::UnionDcl.
 *
 * \return
 *  the new position of the ::UnionDcl (\a p).
 *
 * \note The values from the ::Position struct are copied, so the caller is
 * responsible for freeing the ::Position passed as \a p.
 */
Position
P_SetUnionDclPosition (UnionDcl u, Position p)
{
  char *f;

  if (u)
    {
      P_SetUnionDclLineno (u, P_GetPositionLineno (p));
      P_SetUnionDclColno (u, P_GetPositionColno (p));
      if ((f = P_GetPositionFilename (p)))
	P_SetUnionDclFilename (u, strdup (f));
    }
  else
    P_punt ("struct.c:P_SetUnionDclPosition:%d struct UnionDcl (u) is NULL",
	    __LINE__ - 1);

  return (p);
}

/*! \brief Gets the position of an ::UnionDcl.
 *
 * \param u
 *  the ::UnionDcl.
 *
 * \return
 *  A pointer to a ::Position struct for the ::UnionDcl.
 *
 * \note This function allocates a new ::Position and returns a pointer.  It
 * is the caller's responsibility to free the ::Position using
 * P_RemovePosition().
 *
 * \sa P_RemovePosition() */
Position
P_GetUnionDclPosition (UnionDcl u)
{
  Position result;
  char *f;

  if (u)
    {
      result = P_NewPosition ();

      P_SetPositionLineno (result, P_GetUnionDclLineno (u));
      P_SetPositionColno (result, P_GetUnionDclColno (u));
      if ((f = P_GetUnionDclFilename (u)))
	P_SetPositionFilename (result, strdup (f));

      return (result);
    }
  else
    P_punt ("struct.c:P_GetUnionDclPosition:%d struct UnionDcl (u) is NULL",
	    __LINE__ - 1);

  /* Should never reach this point. */
  return (NULL);
}

/*! \brief Sets the identifier (key and name) of a ::UnionDcl.
 *
 * \param u
 *  the ::UnionDcl
 * \param i
 *  the new identifier.
 *
 * \return
 *  The new identifier (\a i).
 *
 * \note The values from the ::Identifier struct are copied, so the caller is
 * responsible for freeing the ::Identifier passed as \a i.
 */
Identifier
P_SetUnionDclIdentifier (UnionDcl u, Identifier i)
{
  char *name;

  if (u)
    {
      if (u->name)
	free (u->name);
      if ((name = P_GetIdentifierName (i)))
	P_SetUnionDclName (u, strdup (name));
      else
	P_SetUnionDclName (u, NULL);

      P_SetUnionDclKey (u, P_GetIdentifierKey (i));
    }

  return (i);
}

/*! \brief Gets the identifier (name and key) from a ::UnionDcl.
 *
 * \param u
 *  the ::UnionDcl.
 *
 * \return
 *  A pointer to an ::Identifier struct for the ::UnionDcl.
 *
 * \note This function allocates a new ::Identifier and returns a pointer.
 * It is the caller's repsonsibility to free the ::Identifier using
 * P_RemoveIdentifier().
 *
 * \sa P_RemoveIdentifier() */
Identifier
P_GetUnionDclIdentifier (UnionDcl u)
{
  Identifier result;
  char *name;

  if (u)
    {
      result = P_NewIdentifier ();

      P_SetIdentifierKey (result, P_GetUnionDclKey (u));
      if ((name = P_GetUnionDclName (u)))
	P_SetIdentifierName (result, strdup (name));

      return (result);
    }
  else
    P_punt ("struct.c:P_GetUnionDclIdentifier:%d struct UnionDcl (u) is NULL",
	    __LINE__ - 1);

  /* Should never reach this point. */
  return (NULL);
}

/* Field access functions. */
/*! \brief Sets the Field.parent_key field for all fields in a list.
 *
 * \param f
 *  the ::Field.
 * \param k
 *  the new value of the Field.parent_key field.
 *
 * \return
 *  The new value of the Field.parent_key field (\a k).
 *
 * \note This function sets the Field.parent_key field for all fields in a
 *       list.  To set the field for a single field, see
 *       #P_SetFieldParentKey().
 */
Key
P_SetFieldParentKeyAll (Field f, Key k)
{
  if (f)
    {
      while (f)
	{
	  f->parent_key = k;

	  f = f->next;
	}
    }
  else
    P_punt ("struct.c:P_SetFieldParentKeyAll:%d struct Field (f) is NULL",
	    __LINE__ - 1);

  return (k);
}

/*! \brief Appends a ::Field to a ::Field list.
 *
 * \param f
 *  the head of the list to append.
 * \param n
 *  the ::Field to append to the list.
 *
 * \return
 *  A pointer to the head of the list.
 *
 * Appends ::Field \a n to the tail of the list headed by \a f.  \a f may
 * be null, in which case \a n is returned as the head of the list.
 */
Field
P_AppendFieldNext (Field f, Field n)
{
  Field t = f;

  if (f)
    {
      while (t->next)
	t = t->next;

      t->next = n;
    }
  else
    f = n;

  return (f);
}

/*! \brief Sets the identifier (key and name) of a ::Field.
 *
 * \param f
 *  the ::Field.
 * \param i
 *  the new identifier.
 *
 * \return
 *  The new identifier (\a i).
 *
 * \note The values from the ::Identifier struct are copied, so the caller is
 * responsible for freeing the ::Identifier passed as \a i.
 */
Identifier
P_SetFieldIdentifier (Field f, Identifier i)
{
  char *name;

  if (f)
    {
      if (f->name)
	free (f->name);
      if ((name = P_GetIdentifierName (i)))
	P_SetFieldName (f, strdup (name));
      else
	P_SetFieldName (f, NULL);

      P_SetFieldKey (f, P_GetIdentifierKey (i));
    }
  else
    P_punt ("struct.c:P_SetFieldIdentifier:%d struct Field (f) is NULL",
	    __LINE__ - 1);

  return (i);
}

/*! \brief Gets the identifier (name and key) from a ::Field.
 *
 * \param f
 *  the ::Field.
 *
 * \return
 *  A pointer to an ::Identifier struct for the ::Field.
 *
 * \note This function allocates a new ::Identifier and returns a pointer.
 * It is the caller's repsonsibility to free the ::Identifier using
 * P_RemoveIdentifier().
 *
 * \sa P_SetFieldIdentifier() */
Identifier
P_GetFieldIdentifier (Field f)
{
  Identifier result;
  char *name;

  if (f)
    {
      result = P_NewIdentifier ();

      P_SetIdentifierKey (result, P_GetFieldKey (f));
      if ((name = P_GetFieldName (f)))
	P_SetIdentifierName (result, strdup (name));

      return (result);
    }
  else
    P_punt ("struct.c:P_GetFieldIdentifier:%d struct Field (f) is NULL",
	    __LINE__ - 1);

  /* Should never reach this point. */
  return (NULL);
}

/* EnumDcl access functions. */
/*! \brief Sets the position of an ::EnumDcl.
 *
 * \param e
 *  the ::EnumDcl.
 * \param p
 *  the new position of the ::EnumDcl.
 *
 * \return
 *  The new position of the ::EnumDcl (\a p).
 *
 * \note The values from the ::Position struct are copied, so the caller is
 * responsible for freeing the ::Position passed as \a p.
 */
Position
P_SetEnumDclPosition (EnumDcl e, Position p)
{
  char *f;

  if (e)
    {
      P_SetEnumDclLineno (e, P_GetPositionLineno (p));
      P_SetEnumDclColno (e, P_GetPositionColno (p));
      if ((f = P_GetPositionFilename (p)))
	P_SetEnumDclFilename (e, strdup (f));
    }
  else
    P_punt ("struct.c:P_SetEnumDclPosition:%d struct EnumDcl (e) is NULL",
	    __LINE__ - 1);

  return (p);
}

/*! \brief Gets the position of an ::EnumDcl.
 *
 * \param e
 *  the ::EnumDcl.
 *
 * \return
 *  A pointer to a ::Position struct for the ::EnumDcl.
 *
 * \note This function allocates a new ::Position and returns a pointer.  It
 * is the caller's responsibility to free the ::Position using
 * P_RemovePosition().
 *
 * \sa P_RemovePosition() */
Position
P_GetEnumDclPosition (EnumDcl e)
{
  Position result;
  char *f;

  if (e)
    {
      result = P_NewPosition ();

      P_SetPositionLineno (result, P_GetEnumDclLineno (e));
      P_SetPositionColno (result, P_GetEnumDclColno (e));
      if ((f = P_GetEnumDclFilename (e)))
	P_SetPositionFilename (result, strdup (f));

      return (result);
    }
  else
    P_punt ("struct.c:P_GetEnumDclPosition:%d struct EnumDcl (e) is NULL",
	    __LINE__ - 1);

  /* Should never reach this point. */
  return (NULL);
}

/*! \brief Sets the identifier (key and name) of an ::EnumDcl.
 *
 * \param e
 *  the ::EnumDcl.
 * \param i
 *  the new identifier.
 *
 * \return
 *  The new identifier (\a i).
 *
 * \note The values from the ::Identifier struct are copied, so the caller is
 * responsible for freeing the ::Identifier passed as \a i.
 */
Identifier
P_SetEnumDclIdentifier (EnumDcl e, Identifier i)
{
  char *name;

  if (e)
    {
      if (e->name)
	free (e->name);
      if ((name = P_GetIdentifierName (i)))
	P_SetEnumDclName (e, strdup (name));
      else
	P_SetEnumDclName (e, NULL);

      P_SetEnumDclKey (e, P_GetIdentifierKey (i));
    }
  else
    P_punt ("struct.c:P_SetEnumDclIdentifier:%d struct EnumDcl (e) is NULL",
	    __LINE__ - 1);

  return (i);
}

/*! \brief Gets the identifier (name and key) from an ::EnumDcl.
 *
 * \param e
 *  the ::EnumDcl.
 *
 * \return
 *  A pointer to an ::Identifier struct for the ::EnumDcl.
 *
 * \note This function allocates a new ::Identifier and returns a pointer.
 * It is the caller's repsonsibility to free the ::Identifier using
 * P_RemoveIdentifier().
 *
 * \sa P_RemoveIdentifier() */
Identifier
P_GetEnumDclIdentifier (EnumDcl e)
{
  Identifier result;
  char *name;

  if (e)
    {
      result = P_NewIdentifier ();

      P_SetIdentifierKey (result, P_GetEnumDclKey (e));
      if ((name = P_GetEnumDclName (e)))
	P_SetIdentifierName (result, strdup (name));

      return (result);
    }
  else
    P_punt ("struct.c:P_GetEnumDclIdentifier:%d struct EnumDcl (e) is NULL",
	    __LINE__ - 1);

  /* Should never reach this point. */
  return (NULL);
}

/* EnumField access functions. */
/*! \brief Appends an ::EnumField to a ::EnumField list.
 *
 * \param e
 *  the head of the list to append.
 * \param n
 *  the ::EnumField to append to the list.
 *
 * \return
 *  A pointer to the head of the list.
 *
 * Appends ::EnumField \a n to the tail of the list headed by \a e.  \a e may
 * be null, in which case \a n is returned as the head of the list.
 */
EnumField
P_AppendEnumFieldNext (EnumField e, EnumField n)
{
  EnumField t = e;

  if (e)
    {
      while (t->next)
	t = t->next;

      t->next = n;
    }
  else
    e = n;

  return (e);
}

/*! \brief Sets the identifier (key and name) of an ::EnumField.
 *
 * \param e
 *  the ::EnumField.
 * \param i
 *  the new identifier.
 *
 * \return
 *  The new identifier (\a i).
 *
 * \note The values from the ::Identifier struct are copied, so the caller is
 * responsible for freeing the ::Identifier passed as \a i.
 */
Identifier
P_SetEnumFieldIdentifier (EnumField e, Identifier i)
{
  char *name;

  if (e)
    {
      if (e->name)
	free (e->name);
      if ((name = P_GetIdentifierName (i)))
	P_SetEnumFieldName (e, strdup (name));
      else
	P_SetEnumFieldName (e, NULL);

      P_SetEnumFieldKey (e, P_GetIdentifierKey (i));
    }
  else
    P_punt ("struct.c:P_SetEnumFieldIdentifier:%d struct EnumField (e) is "
	    "NULL", __LINE__ - 1);

  return (i);
}

/*! \brief Gets the identifier (name and key) from an ::EnumField.
 *
 * \param e
 *  the ::EnumField.
 *
 * \return
 *  A pointer to an ::Identifier struct for the ::EnumField.
 *
 * \note This function allocates a new ::Identifier and returns a pointer.
 * It is the caller's repsonsibility to free the ::Identifier using
 * P_RemoveIdentifier().
 */
Identifier
P_GetEnumFieldIdentifier (EnumField e)
{
  Identifier result;
  char *name;

  if (e)
    {
      result = P_NewIdentifier ();

      P_SetIdentifierKey (result, P_GetEnumFieldKey (e));
      if ((name = P_GetEnumFieldName (e)))
	P_SetIdentifierName (result, strdup (name));

      return (result);
    }
  else
    P_punt ("struct.c:P_GetEnumFieldIdentifier:%d struct EnumField (e) is "
	    "NULL", __LINE__ - 1);

  /* Should never reach this point. */
  return (NULL);
}

/* Stmt access functions. */
/*! \brief Sets the Stmt.lex_prev field.
 *
 * \param s
 *  the ::Stmt.
 * \param l
 *  the new value of the Stmt.lex_prev field.
 *
 * \return
 *  The new value of the Stmt.lex_prev field (\a l).
 *
 * Sets the Stmt.lex_prev field and copies \a s's parent pointers to \a l.
 * Sets \a l.lex_next to point to \a s.
 */
Stmt
P_SetStmtLexPrev (Stmt s, Stmt l)
{
  if (s)
    {
      /* This function can't assume that it can set the old lex_prev's
       * parent fields to NULL, since it is still valid for them
       * to refer to s's parent. */

      s->lex_prev = l;

      if (s->lex_prev)
	{
	  s->lex_prev->lex_next = s;

	  if (s->parent)
	    {
	      P_SetStmtParentStmtDir (s->lex_prev, s->parent, REVERSE);
	      P_SetStmtParentFuncDir (s->lex_prev, NULL, REVERSE);
	      P_SetStmtParentExprDir (s->lex_prev, NULL, REVERSE);
	    }
	  if (s->parent_func)
	    {
	      P_SetStmtParentStmtDir (s->lex_prev, NULL, REVERSE);
	      P_SetStmtParentFuncDir (s->lex_prev, s->parent_func, REVERSE);
	      P_SetStmtParentExprDir (s->lex_prev, NULL, REVERSE);
	    }
	  if (s->parent_expr)
	    {
	      P_SetStmtParentStmtDir (s->lex_prev, NULL, REVERSE);
	      P_SetStmtParentFuncDir (s->lex_prev, NULL, REVERSE);
	      P_SetStmtParentExprDir (s->lex_prev, s->parent_expr, REVERSE);
	    }
	}
    }
  else
    P_punt ("struct.c:P_SetStmtLexPrev:%d struct Stmt (s) is NULL", __LINE__);

  return (l);
}

/*! \brief Sets the Stmt.lex_next field.
 *
 * \param s
 *  the ::Stmt.
 * \param l
 *  the new value of the Stmt.lex_next field.
 *
 * \return
 *  The new value of the Stmt.lex_next field (\a l).
 *
 * Sets \a s.lex_next to \a l.  Copies \a s's parent pointers to \a l.
 * Sets \a l->lex_prev to point to \a s.
 *
 * \note This function explicitly sets the Stmt.lex_next field.  To maintain
 *       a Stmt list, see P_AppendStmtLexNext().
 */
Stmt
P_SetStmtLexNext (Stmt s, Stmt l)
{
  if (s)
    {
      /* This function can't assume that it can set the old lex_next's
       * parent field to NULL, since it is still valid for them to refer
       * to s's parent. */

      s->lex_next = l;

      if (s->lex_next)
	{
	  s->lex_next->lex_prev = s;

	  P_SetStmtParentStmtAll (s->lex_next, NULL);
	  P_SetStmtParentFuncAll (s->lex_next, NULL);
	  P_SetStmtParentExprAll (s->lex_next, NULL);

	  if (s->parent)
	    P_SetStmtParentStmtAll (s->lex_next, s->parent);
	  if (s->parent_func)
	    P_SetStmtParentFuncAll (s->lex_next, s->parent_func);
	  if (s->parent_expr)
	    P_SetStmtParentExprAll (s->lex_next, s->parent_expr);
	}
    }
  else
    P_punt ("struct.c:P_SetStmtLexNext:%d struct Stmt (s) is NULL", __LINE__);

  return (l);
}

/*! \brief Appends a ::Stmt to a ::Stmt lex_next list.
 *
 * \param s
 *  the head of the list to append.
 * \param n
 *  the ::Stmt to append to the lex_next list.
 *
 * \return
 *  A pointer to the head of the list.
 *
 * Appends ::Stmt \a n to the tail of the Stmt.lex_next list headed by \a s.
 * This function also sets the Stmt.lex_prev field of \a n appropriately.
 * \a s may be null, in which case \a n is returned as the head of the list.
 * Copies \a s's parent pointers to \a n.
 *
 * \note This function maintains a Stmt list.  To set the Stmt.lex_next field
 *       explicitly, see P_SetStmtLexNext().
 */
Stmt
P_AppendStmtLexNext (Stmt s, Stmt n)
{
  Stmt t = s;

  if (s)
    {
      while (t->lex_next)
	t = t->lex_next;

      t->lex_next = n;

      if (t->lex_next)
	{
	  t->lex_next->lex_prev = t;

	  P_SetStmtParentStmtAll (t->lex_next, NULL);
	  P_SetStmtParentFuncAll (t->lex_next, NULL);
	  P_SetStmtParentExprAll (t->lex_next, NULL);

	  if (s->parent)
	    P_SetStmtParentStmtAll (t->lex_next, s->parent);
	  if (s->parent_func)
	    P_SetStmtParentFuncAll (t->lex_next, s->parent_func);
	  if (s->parent_expr)
	    P_SetStmtParentExprAll (t->lex_next, s->parent_expr);
	}
    }
  else
    s = n;

  return (s);
}

/*! \brief Sets the Stmt.labels field.
 *
 * \param s
 *  the ::Stmt.
 * \param l
 *  the new value of the Stmt.labels field.
 *
 * \return
 *  The new value of the Stmt.labels field.
 *
 * If the label is of type LB_LABEL, sets the parent field to point
 * to \a s.
 *
 * \note This function explicitly sets the operand field of \a e.
 *       To maintain a list of labels, use P_AppendStmtLabels().
 */
Label
P_SetStmtLabels (Stmt s, Label l)
{
  if (s)
    {
      if (s->labels && P_GetLabelParentStmt (s->labels) == s)
	P_SetLabelParentStmtAll (s->labels, NULL);

      s->labels = l;

      if (s->labels)
	P_SetLabelParentStmtAll (s->labels, s);
    }
  else
    P_punt ("struct.c:P_SetStmtLabels:%d struct Stmt (s) is NULL", __LINE__);

  return (l);
}

/*! \brief Appends a ::Label to the Stmt.labels field.
 *
 * \param s
 *  the ::Stmt.
 * \param l
 *  the label to append to the list.
 *
 * \return
 *  The head of the Stmt.labels field.
 *
 * Appends \a l to \a s.labels.  sets the parent field of \a l to point
 * to \a s.
 */
Label
P_AppendStmtLabels (Stmt s, Label l)
{
  Label last;

  if (s)
    {
      if ((last = s->labels))
	{
	  while (P_GetLabelNext (last))
	    last = P_GetLabelNext (last);

	  P_SetLabelNext (last, l);
	}
      else
	s->labels = l;

      if (l)
	P_SetLabelParentStmtAll (l, s);
    }
  else
    P_punt ("struct.c:P_AppendStmtLabels:%d struct Stmt (s) is NULL",
	    __LINE__ - 1);

  return (s->labels);
}

/*! \brief Sets the Stmt.stmtstruct.ret field.
 *
 * \param s
 *  the ::Stmt.
 * \param r
 *  the new value of the Stmt.stmtstruct.ret field.
 *
 * \return
 *  The new value of the Stmt.stmtstruct.ret field (\a r).
 *
 * Sets the Stmt.stmtstruct.ret field to \a r.  Sets \a r's parentstmt
 * field to point to \a s.
 */
Expr
P_SetStmtRet (Stmt s, Expr r)
{
  if (s && (s->type == ST_RETURN || r == NULL))
    {
      if (s->stmtstruct.ret && P_GetExprParentStmt (s->stmtstruct.ret) == s)
	P_SetExprParentStmtAll (s->stmtstruct.ret, NULL);

      s->stmtstruct.ret = r;

      if (s->stmtstruct.ret)
	{
	  P_SetExprParentStmtAll (s->stmtstruct.ret, s);
	  P_SetExprParentExprAll (s->stmtstruct.ret, NULL);
	  P_SetExprParentVarAll (s->stmtstruct.ret, NULL);
	}
    }
  else if (s)
    P_punt ("struct.c:P_SetStmtRet:%d requires ST_RETURN, received %d",
	    __LINE__ - 1, s->type);
  else
    P_punt ("struct.c:P_SetStmtRet:%d struct Stmt (s) is NULL", __LINE__);

  return (r);
}

/*! \brief Sets the Stmt.stmtstruct.compound field.
 * \param s
 *  the ::Stmt.
 * \param c
 *  the new value of the Stmt.stmtstruct.compound field.
 *
 * \return
 *  The new value of the Stmt.stmtstruct.compound field.
 *
 * Sets the Stmt.stmtstruct.compound field.  Sets the parent statement
 * of the compound to \a s.
 */
Compound
P_SetStmtCompound (Stmt s, Compound c)
{
  if (s && (s->type == ST_COMPOUND || c == NULL))
    {
      if (s->stmtstruct.compound && \
	  P_GetCompoundParentStmt (s->stmtstruct.compound) == s)
	P_SetCompoundParentStmtAll (s->stmtstruct.compound, NULL);

      s->stmtstruct.compound = c;

      if (s->stmtstruct.compound)
	P_SetCompoundParentStmtAll (s->stmtstruct.compound, s);
    }
  else if (s)
    P_punt ("struct.c:P_SetStmtCompound:%d requires ST_COMPOUND, received %d",
	    __LINE__ - 1, s->type);
  else
    P_punt ("struct.c:P_SetStmtCompound:%d struct Stmt (s) is NULL", __LINE__);

  return (c);
}

/*! \brief Sets the Stmt.stmtstruct.ifstmt field.
 *
 * \param s
 *  the ::Stmt.
 * \param i
 *  the new value of the Stmt.stmtstruct.ifstmt field.
 *
 * \return
 *  The new value of the Stmt.stmtstruct.ifstmt field.
 *
 * Sets the Stmt.stmtstruct.compound field.  Sets the parent statement
 * of the IfStmt to \a s.
 */
IfStmt
P_SetStmtIfStmt (Stmt s, IfStmt i)
{
  if (s && (s->type == ST_IF || i == NULL))
    {
      if (s->stmtstruct.ifstmt && \
	  P_GetIfStmtParentStmt (s->stmtstruct.ifstmt) == s)
	P_SetIfStmtParentStmtAll (s->stmtstruct.ifstmt, NULL);

      s->stmtstruct.ifstmt = i;

      if (s->stmtstruct.ifstmt)
	P_SetIfStmtParentStmtAll (s->stmtstruct.ifstmt, s);
    }
  else if (s)
    P_punt ("struct.c:P_SetStmtIfStmt:%d requires ST_IF, received %d",
	    __LINE__ - 1, s->type);
  else
    P_punt ("struct.c:P_SetStmtIfStmt:%d struct Stmt (s) is NULL", __LINE__);

  return (i);
}

/*! \brief Sets the Stmt.stmtstruct.switchstmt field.
 *
 * \param s
 *  the ::Stmt.
 * \param t
 *  the new value of the Stmt.stmtstruct.switchstmt field.
 *
 * \return
 *  The new value of the Stmt.stmtstruct.switchstmt field.
 *
 * Sets the Stmt.stmtstruct.switchstmt field.  Sets the parent statement
 * of the switch statement to \a s.
 */
SwitchStmt
P_SetStmtSwitchStmt (Stmt s, SwitchStmt t)
{
  if (s && (s->type == ST_SWITCH || t == NULL))
    {
      if (s->stmtstruct.switchstmt && \
	  P_GetSwitchStmtParentStmt (s->stmtstruct.switchstmt) == s)
	P_SetSwitchStmtParentStmtAll (s->stmtstruct.switchstmt, NULL);

      s->stmtstruct.switchstmt = t;

      if (s->stmtstruct.switchstmt)
	P_SetSwitchStmtParentStmtAll (s->stmtstruct.switchstmt, s);
    }
  else if (s)
    P_punt ("struct.c:P_SetStmtSwitchStmt:%d requires ST_SWITCH, received %d",
	    __LINE__ - 1, s->type);
  else
    P_punt ("struct.c:P_SetStmtSwitchStmt:%d struct Stmt (s) is NULL",
	    __LINE__ - 1);
  
  return (t);
}

/*! \brief Sets the Stmt.stmtstruct.pstmt field.
 *
 * \param s
 *  the ::Stmt.
 * \param p
 *  the new value of the Stmt.stmtstruct.pstmt field.
 *
 * \return
 *  The new value of the Stmt.stmtstruct.pstmt field.
 *
 * Sets the Stmt.stmtstruct.pstmt field.  Sets the parent statement
 * of the pstmt to \a s.
 */
Pstmt
P_SetStmtPstmt (Stmt s, Pstmt p)
{
  if (s && (s->type == ST_PSTMT || p == NULL))
    {
      if (s->stmtstruct.pstmt && \
	  P_GetPstmtParentStmt (s->stmtstruct.pstmt) == s)
	P_SetPstmtParentStmtAll (s->stmtstruct.pstmt, NULL);

      s->stmtstruct.pstmt = p;

      if (s->stmtstruct.pstmt)
	P_SetPstmtParentStmtAll (s->stmtstruct.pstmt, s);
    }
  else if (s)
    P_punt ("struct.c:P_SetStmtPstmt:%d requires ST_PSTMT, received %d",
	    __LINE__ - 1, s->type);
  else
    P_punt ("struct.c:P_SetStmtPstmt:%d struct Stmt (s) is NULL", __LINE__);

  return (p);
}

/*! \brief Sets the Stmt.stmtstruct.mutex field.
 *
 * \param s
 *  the ::Stmt.
 * \param m
 *  the new value of the Stmt.stmtstruct.mutex field.
 *
 * \return
 *  The new value of the Stmt.stmtstruct.mutex field.
 *
 * Sets the Stmt.stmtstruct.mutex field.  Sets the parent statement
 * of the mutex to \a s.
 */
Mutex
P_SetStmtMutex (Stmt s, Mutex m)
{
  if (s && (s->type == ST_MUTEX || m == NULL))
    {
      if (s->stmtstruct.mutex && \
	  P_GetMutexParentStmt (s->stmtstruct.mutex) == s)
	P_SetMutexParentStmtAll (s->stmtstruct.mutex, NULL);

      s->stmtstruct.mutex = m;

      if (s->stmtstruct.mutex)
	P_SetMutexParentStmtAll (s->stmtstruct.mutex, s);
    }
  else if (s)
    P_punt ("struct.c:P_SetStmtMutex:%d requires ST_MUTEX, received %d",
	    __LINE__ - 1, s->type);
  else
    P_punt ("struct.c:P_SetStmtMutex:%d struct Stmt (s) is NULL", __LINE__);

  return (m);
}

/*! \brief Sets the Stmt.stmtstruct.cobegin field.
 *
 * \param s
 *  the ::Stmt.
 * \param c
 *  the new value of the Stmt.stmtstruct.cobegin field.
 *
 * \return
 *  The new value of the Stmt.stmtstruct.cobegin field.
 *
 * Sets the stmt.stmtstruct.cobegin field.  Sets the parent statement
 * of the cobegin to \a s.
 */
Cobegin
P_SetStmtCobegin (Stmt s, Cobegin c)
{
  if (s && (s->type == ST_COBEGIN || c == NULL))
    {
      if (s->stmtstruct.cobegin && \
	  P_GetCobeginParentStmt (s->stmtstruct.cobegin) == s)
	P_SetCobeginParentStmtAll (s->stmtstruct.cobegin, NULL);

      s->stmtstruct.cobegin = c;

      if (s->stmtstruct.cobegin)
	P_SetCobeginParentStmtAll (s->stmtstruct.cobegin, s);
    }
  else if (s)
    P_punt ("struct.c:P_SetStmtCobegin:%d requires ST_COBEGIN, received %d",
	    __LINE__ - 1, s->type);
  else
    P_punt ("struct.c:P_SetStmtCobegin:%d struct Stmt (s) is NULL", __LINE__);

  return (c);
}

/*! \brief Sets the Stmt.stmtstruct.parloop field.
 *
 * \param s
 *  the ::Stmt.
 * \param p
 *  the new value of the Stmt.stmtstruct.parloop field.
 *
 * \return
 *  The new value of the Stmt.stmtstruct.parloop field.
 *
 * Sets the Stmt.stmtstruct.parloop field.  Sets the parent statement
 * of the parloop to \a s.
 */
ParLoop
P_SetStmtParLoop (Stmt s, ParLoop p)
{
  if (s && (s->type == ST_PARLOOP || p == NULL))
    {
      if (s->stmtstruct.parloop && \
	  P_GetParLoopParentStmt (s->stmtstruct.parloop) == s)
	P_SetParLoopParentStmtAll (s->stmtstruct.parloop, NULL);

      s->stmtstruct.parloop = p;

      if (s->stmtstruct.parloop)
	P_SetParLoopParentStmtAll (s->stmtstruct.parloop, s);
    }
  else if (s)
    P_punt ("struct.c:P_SetStmtParLoop:%d requires ST_PARLOOP, received %d",
	    __LINE__ - 1, s->type);
  else
    P_punt ("struct.c:P_SetStmtParLoop:%d struct Stmt (s) is NULL", __LINE__);

  return (p);
}

/*! \brief Sets the Stmt.stmtstruct.serloop field.
 *
 * \param s
 *  the ::Stmt.
 * \param t
 *  the new value of the Stmt.stmtstruct.serloop field.
 *
 * \return
 *  The new value of the Stmt.stmtstruct.serloop field.
 *
 * Sets the Stmt.stmtstruct.serloop field.  Sets the parent statement
 * of the serloop to \a s.
 */
SerLoop
P_SetStmtSerLoop (Stmt s, SerLoop t)
{
  if (s && (s->type == ST_SERLOOP || t == NULL))
    {
      if (s->stmtstruct.serloop && \
	  P_GetSerLoopParentStmt (s->stmtstruct.serloop) == s)
	P_SetSerLoopParentStmtAll (s->stmtstruct.serloop, NULL);

      s->stmtstruct.serloop = t;

      if (s->stmtstruct.serloop)
	P_SetSerLoopParentStmtAll (s->stmtstruct.serloop, s);
    }
  else if (s)
    P_punt ("struct.c:P_SetStmtSerLoop:%d requires ST_SERLOOP, received %d",
	    __LINE__ - 1, s->type);
  else
    P_punt ("struct.c:P_SetStmtSerLoop:%d struct Stmt (s) is NULL", __LINE__);

  return (t);
}

/*! \brief Sets the Stmt.stmtstruct.expr field.
 *
 * \param s
 *  the ::Stmt.
 * \param e
 *  the new value of the Stmt.stmtstruct.expr field.
 *
 * \return
 *  The new value of the Stmt.stmtstruct.expr field.
 *
 * Sets the Stmt.stmtstruct.expr field.  Sets the parent statement
 * of the expr to \a s.
 */
Expr
P_SetStmtExpr (Stmt s, Expr e)
{
  if (s && (s->type == ST_EXPR || e == NULL))
    {
      if (s->stmtstruct.expr && \
	  P_GetExprParentStmt (s->stmtstruct.expr) != NULL)
	P_SetExprParentStmtAll (s->stmtstruct.expr, NULL);

      s->stmtstruct.expr = e;

      if (s->stmtstruct.expr)
	{
	  P_SetExprParentStmtAll (s->stmtstruct.expr, s);
	  P_SetExprParentExprAll (s->stmtstruct.expr, NULL);
	}
    }
  else if (s)
    P_punt ("struct.c:P_SetStmtExpr:%d requires ST_EXPR, received %d",
	    __LINE__ - 1, s->type);
  else
    P_punt ("struct.c:P_SetStmtExpr:%d struct Stmt (s) is NULL", __LINE__);

  return (e);
}

/*! \brief Sets the Stmt.stmtstruct.bodystmt field.
 *
 * \param s
 *  the ::Stmt.
 * \param b
 *  the new value of the Stmt.stmtstruct.bodystmt field.
 *
 * \return
 *  The new value of the Stmt.stmtstruct.bodystmt field.
 *
 * Sets the Stmt.stmtstruct.bodystmt field.  Sets the parent statement
 * of the bodystmt to \a s.
 */
BodyStmt
P_SetStmtBodyStmt (Stmt s, BodyStmt b)
{
  if (s && (s->type == ST_BODY || b == NULL))
    {
      if (s->stmtstruct.bodystmt && \
	  P_GetBodyStmtParentStmt (s->stmtstruct.bodystmt) == s)
	P_SetBodyStmtParentStmtAll (s->stmtstruct.bodystmt, NULL);

      s->stmtstruct.bodystmt = b;

      if (s->stmtstruct.bodystmt)
	P_SetBodyStmtParentStmtAll (s->stmtstruct.bodystmt, s);
    }
  else if (s)
    P_punt ("struct.c:P_SetStmtBodyStmt:%d requires ST_BODY, received %d",
	    __LINE__ - 1, s->type);
  else
    P_punt ("struct.c:P_SetStmtBodyStmt:%d struct Stmt (s) is NULL", __LINE__);

  return (b);
}

/*! \brief Sets the Stmt.stmtstruct.epiloguestmt field.
 *
 * \param s
 *  the ::Stmt.
 * \param e
 *  the new value of the Stmt.stmtstruct.epiloguestmt field.
 *
 * \return
 *  The new value of the Stmt.stmtstruct.epiloguestmt field.
 *
 * Sets the stmt.stmtstruct.epiloguestmt field.  Sets the parent statement
 * of the epiloguestmt to \a s.
 */
EpilogueStmt
P_SetStmtEpilogueStmt (Stmt s, EpilogueStmt e)
{
  if (s && (s->type == ST_EPILOGUE || e == NULL))
    {
      if (s->stmtstruct.epiloguestmt && \
	  P_GetEpilogueStmtParentStmt (s->stmtstruct.epiloguestmt) == s)
	P_SetEpilogueStmtParentStmtAll (s->stmtstruct.epiloguestmt, NULL);

      s->stmtstruct.epiloguestmt = e;

      if (s->stmtstruct.epiloguestmt)
	P_SetEpilogueStmtParentStmtAll (s->stmtstruct.epiloguestmt, s);
    }
  else if (s)
    P_punt ("struct.c:P_SetStmtEpilogueStmt:%d requires ST_EPILOGUE, received "
	    "%d", __LINE__ - 1, s->type);
  else
    P_punt ("struct.c:P_SetStmtEpilogueStmt:%d struct Stmt (s) is NULL",
	    __LINE__ - 1);

  return (e);
}

/*! \brief Sets the Stmt.stmtstruct.asmstmt field.
 *
 * \param s
 *  the ::Stmt.
 * \param a
 *  the new value of the Stmt.stmtstruct.asmstmt field.
 *
 * \return
 *  The new value of the Stmt.stmtstruct.asmstmt field.
 *
 * Sets the Stmt.stmtstruct.asmstmt field.  Sets the parent statement
 * of the asmstmt to \a s.
 */
AsmStmt
P_SetStmtAsmStmt (Stmt s, AsmStmt a)
{
  if (s && (s->type == ST_ASM || a == NULL))
    {
      if (s->stmtstruct.asmstmt && \
	  P_GetAsmStmtParentStmt (s->stmtstruct.asmstmt) == s)
	P_SetAsmStmtParentStmtAll (s->stmtstruct.asmstmt, NULL);

      s->stmtstruct.asmstmt = a;

      if (s->stmtstruct.asmstmt)
	P_SetAsmStmtParentStmtAll (s->stmtstruct.asmstmt, s);
    }
  else if (s)
    P_punt ("struct.c:P_SetStmtAsmStmt:%d requires ST_ASM, received %d",
	    __LINE__ - 1, s->type);
  else
    P_punt ("struct.c:P_SetStmtAsmStmt:%d struct Stmt (s) is NULL", __LINE__);

  return (a);
}

/*! \brief Sets the Stmt.parent field in all statements in a list.
 *
 * \param s
 *  the head of the list to set.
 * \param p
 *  the new value of the Stmt.parent field.
 * \param direction
 *  the direction the list should be traversed.  FORWARD (via lex_next) or
 *  REVERSE (via lex_prev).
 *
 * \return
 *  The new value of the Stmt.parent field (\a p).
 */
Stmt
P_SetStmtParentStmtDir (Stmt s, Stmt p, int direction)
{
  Stmt cur_stmt;

  if (s)
    {
      switch (direction)
	{
	case FORWARD:
	  for (cur_stmt = s; cur_stmt; cur_stmt = cur_stmt->lex_next)
	    P_SetStmtParentStmt (cur_stmt, p);
	  break;

	case REVERSE:
	  for (cur_stmt = s; cur_stmt; cur_stmt = cur_stmt->lex_prev)
	    P_SetStmtParentStmt (cur_stmt, p);
	  break;

	default:
	  P_punt ("struct.c:P_SetStmtParentStmtDir:%d invalid direction %d",
		  __LINE__ - 1, direction);
	}
    }
  else
    P_punt ("struct.c:P_SetStmtParentStmtDir:%d struct Stmt (s) is NULL",
	    __LINE__ - 1);

  return (p);
}

/*! \brief Sets the Stmt.parent_func field in all statements in a list.
 *
 * \param s
 *  the head of the list to set.
 * \param p
 *  the new value of the Stmt.parent_func field.
 * \param direction
 *  the direction the list should be traversed.  FORWARD (via lex_next) or
 *  REVERSE (via lex_prev).
 *
 * \return
 *  The new value of the Stmt.parent_func field (\a p).
 */
FuncDcl
P_SetStmtParentFuncDir (Stmt s, FuncDcl p, int direction)
{
  Stmt cur_stmt;

  if (s)
    {
      switch (direction)
	{
	case FORWARD:
	  for (cur_stmt = s; cur_stmt; cur_stmt = cur_stmt->lex_next)
	    P_SetStmtParentFunc (cur_stmt, p);
	  break;

	case REVERSE:
	  for (cur_stmt = s; cur_stmt; cur_stmt = cur_stmt->lex_prev)
	    P_SetStmtParentFunc (cur_stmt, p);
	  break;

	default:
	  P_punt ("struct.c:P_SetStmtParentFuncDir:%d invalid direction %d",
		  __LINE__ - 1, direction);
	}
    }
  else
    P_punt ("struct.c:P_SetStmtParentFuncDir:%d struct Stmt (s) is NULL",
	    __LINE__ - 1);

  return (p);
}

/*! \brief Sets the Stmt.parent_expr field in all statements in a list.
 *
 * \param s
 *  the head of the list to set.
 * \param p
 *  the new value of the Stmt.parent_expr field.
 * \param direction
 *  the direction the list should be traversed.  FORWARD (via lex_next) or
 *  REVERSE (via lex_prev).
 *
 * \return
 *  The new value of the Stmt.parent_expr field (\a p).
 */
Expr
P_SetStmtParentExprDir (Stmt s, Expr p, int direction)
{
  Stmt cur_stmt;

  if (s)
    {
      switch (direction)
	{
	case FORWARD:
	  for (cur_stmt = s; cur_stmt; cur_stmt = cur_stmt->lex_next)
	    P_SetStmtParentExpr (cur_stmt, p);
	  break;

	case REVERSE:
	  for (cur_stmt = s; cur_stmt; cur_stmt = cur_stmt->lex_prev)
	    P_SetStmtParentExpr (cur_stmt, p);
	  break;

	default:
	  P_punt ("struct.c:P_SetStmtParentExprDir:%d invalid direction %d",
		  __LINE__ - 1, direction);
	}
    }
  else
    P_punt ("struct.c:P_SetStmtParentExprDir:%d struct Stmt (s) is NULL",
	    __LINE__ - 1);

  return (p);
}

/*! \brief Sets the position of a ::Stmt.
 *
 * \param s
 *  the ::Stmt.
 * \param p
 *  the ::Position to set.
 *
 * \return
 *  The new position of the ::Stmt (\a p).
 *
 * \note The values from the ::Position struct are copied, so the caller is
 * responsible for freeing the ::Position passed as \a p.
 */
Position
P_SetStmtPosition (Stmt s, Position p)
{
  char *f;

  if (s)
    {
      P_SetStmtLineno (s, P_GetPositionLineno (p));
      P_SetStmtColno (s, P_GetPositionColno (p));
      if ((f = P_GetPositionFilename (p)))
	{
	  if (P_GetStmtFilename (s))
	    free (P_GetStmtFilename (s));

	  P_SetStmtFilename (s, strdup (f));
	}
    }
  else
    P_punt ("struct.c:P_SetStmtPosition:%d struct Stmt (s) is NULL", __LINE__);

  return (p);
}

/*! \brief Gets the position of a ::Stmt.
 *
 * \param s
 *  the ::Stmt.
 *
 * \return
 *  A pointer to a ::Position struct for the ::Stmt.
 *
 * \note This function allocates a new ::Position and returns a pointer.  It
 * is the caller's responsibility to free the ::Position using
 * P_RemovePosition().
 *
 * \sa P_RemovePosition() */
Position
P_GetStmtPosition (Stmt s)
{
  Position result;
  char *f;

  if (s)
    {
      result = P_NewPosition ();

      P_SetPositionLineno (result, P_GetStmtLineno (s));
      P_SetPositionColno (result, P_GetStmtColno (s));
      if ((f = P_GetStmtFilename (s)))
	P_SetPositionFilename (result, strdup (f));

      return (result);
    }
  else
    P_punt ("struct.c:P_GetStmtPosition:%d struct Stmt (s) is NULL", __LINE__);

  /* Should never reach this point. */
  return (NULL);
}

/*! \brief Sets the identifier (key and label value) of a goto ::Stmt.
 *
 * \param s
 *  the ::Stmt.
 * \param i
 *  the new identifier.
 *
 * \return
 *  The new identifier (\a i);
 *
 * \note The values from the ::Identifier struct are copied, so the
 * caller is responsible for freeing the ::Identifier passed as \a i.
 */
Identifier
P_SetStmtGotoIdentifier (Stmt s, Identifier i)
{
  char *val;

  if (s && P_GetStmtType (s) == ST_GOTO)
    {
      if (s->stmtstruct.label.val)
	free (s->stmtstruct.label.val);
      if ((val = P_GetIdentifierName (i)))
	P_SetStmtLabelVal (s, strdup (val));
      else
	P_SetStmtLabelVal (s, NULL);

      P_SetStmtLabelKey (s, P_GetIdentifierKey (i));
    }

  return (i);

}

/*! \brief Gets the identifier (label val and key) from a goto ::Stmt.
 *
 * \param s
 *  the ::Stmt.
 *
 * \return
 *  A pointer to an ::Identifier struct for the goto ::stmt.  Returns
 *  null if the ::Stmt is not a goto.
 *
 * \note This function allocates a new ::Identifier and returns a pointer.
 * It is the caller's responsibility to free the ::Identifier using
 * P_RemoveIdentifier().
 *
 * \sa P_RemoveIdentifier() */
Identifier
P_GetStmtGotoIdentifier (Stmt s)
{
  Identifier result;
  char *val;

  if (s)
    {
      if (P_GetStmtType (s) == ST_GOTO)
	{
	  result = P_NewIdentifier ();

	  P_SetIdentifierKey (result, P_GetStmtLabelKey (s));
	  if ((val = P_GetStmtLabelVal (s)))
	    P_SetIdentifierName (result, strdup (val));

	  return (result);
	}
      else
	{
	  return (NULL);
	}
    }
  else
    P_punt ("struct.c:P_GetStmtGotoIdentifier:%d struct Stmt (s) is NULL",
	    __LINE__ - 1);

  /* Should never reach this point. */
  return (NULL);
}

/*! \brief Appends a VarDcl to a Stmt's local var list.
 *
 * \param s
 *  the Stmt.
 * \param v
 *  the VarDcl to append.
 *
 * Appends VarDcl \a v to Stmt \a s's local var list.  \a s must be a
 * compound stmt.
 */
void
P_AppendStmtLocalVar (Stmt s, VarDcl v)
{
  Compound c;

  if (P_GetStmtType (s) != ST_COMPOUND)
    P_punt ("struct.c:P_AppendStmtLocalVar:%d Stmt must be ST_COMPOUND",
	    __LINE__ - 1);

  c = P_GetStmtCompound (s);

  P_SetCompoundVarList (c, List_insert_last (P_GetCompoundVarList (c), v));

  return;
}

/*! \brief Updates statement to refer to a new statement.
 *
 * \param s
 *  the statement to update.
 * \param old
 *  the old child statement of \a s.
 * \param new
 *  the new child statement of \a s.
 *
 * If any of \a s's child statements refer to \a old, they are changed 
 * to refer to \a new.
 *
 * \sa P_StmtUpdateParents(), P_StmtInsertExprAfter(),
 * P_StmtInsertExprBefore(), P_StmtInsertStmtAfter(),
 * P_StmtInsertStmtBefore(), P_StmtAddLabel(), P_StmtAddLabelAfter(),
 * P_StmtRemoveStmt() */
void
P_StmtUpdate (Stmt s, Stmt old, Stmt new)
{
  switch (P_GetStmtType (s))
    {
    case ST_NOOP:
    case ST_CONT:
    case ST_BREAK:
    case ST_RETURN:
    case ST_GOTO:
    case ST_ADVANCE:
    case ST_AWAIT:
    case ST_ASM:
      break;
      
    case ST_COMPOUND:
      {
	Compound c = P_GetStmtCompound (s);

	if (P_GetCompoundStmtList (c) == old)
	  P_SetCompoundStmtList (c, new);
      }
      break;

    case ST_IF:
      {
	IfStmt i = P_GetStmtIfStmt (s);

	if (P_GetIfStmtThenBlock (i) == old)
	  P_SetIfStmtThenBlock (i, new);
	else if (P_GetIfStmtElseBlock (i) == old)
	  P_SetIfStmtElseBlock (i, new);
      }
      break;

    case ST_SWITCH:
      {
	SwitchStmt t = P_GetStmtSwitchStmt (s);
	
	if (P_GetSwitchStmtSwitchBody (t) == old)
	  P_SetSwitchStmtSwitchBody (t, new);
      }
      break;

    case ST_PSTMT:
      {
	Pstmt p = P_GetStmtPstmt (s);

	if (P_GetPstmtStmt (p) == old)
	  P_SetPstmtStmt (p, new);
      }
      break;

    case ST_MUTEX:
      {
	Mutex m = P_GetStmtMutex (s);

	if (P_GetMutexStatement (m) == old)
	  P_SetMutexStatement (m, new);
      }
      break;

    case ST_COBEGIN:
      {
	Cobegin c = P_GetStmtCobegin (s);

	if (P_GetCobeginStatements (c) == old)
	  P_SetCobeginStatements (c, new);
      }
      break;
      
    case ST_PARLOOP:
      {
	ParLoop p = P_GetStmtParLoop (s);
	
	if (P_GetPstmtStmt (P_GetParLoopPstmt (p)) == old)
	  P_SetPstmtStmt (P_GetParLoopPstmt (p), new);
      }
      break;
	  
    case ST_SERLOOP:
      {
	SerLoop t = P_GetStmtSerLoop (s);

	if (P_GetSerLoopLoopBody (t) == old)
	  P_SetSerLoopLoopBody (t, new);
      }
      break;

    case ST_EXPR:
      {
	Expr e = P_GetStmtExpr (s);
	
	if (P_GetExprOpcode (e) != OP_stmt_expr)
	  P_punt ("struct.c:P_StmtUpdate:%d stmt is ST_EXPR, but expr is "
		  "not OP_stmt_expr", __LINE__ - 1);
	
	if (P_GetExprStmt (e) == old)
	  P_SetExprStmt (e, new);
      }
      break;

    case ST_BODY:
      {
	BodyStmt b = P_GetStmtBodyStmt (s);
	
	if (P_GetBodyStmtStatement (b) == old)
	  P_SetBodyStmtStatement (b, new);
      }
      break;

    case ST_EPILOGUE:
      {
	EpilogueStmt e = P_GetStmtEpilogueStmt (s);
	
	if (P_GetEpilogueStmtStatement (e) == old)
	  P_SetEpilogueStmtStatement (e, new);
      }
      break;

    default:
      P_punt ("struct.c:P_StmtUpdate:%d Unknown stmt type %d", __LINE__,
	      P_GetStmtType (s));
    }

  return;
}

/*! \brief Updates statement parents to refer to a new statement.
 *
 * \param s
 *  the old statement.
 * \param new
 *  the new statement.
 *
 * If any of the parents of \a s refer to \a s, they are updated to
 * refer to \a new.
 *
 * \sa P_StmtInsertExprAfter(), P_StmtInsertExprBefore(),
 * P_StmtInsertStmtAfter(), P_StmtInsertStmtBefore(),
 * P_StmtAddLabel(), P_StmtAddLabelAfter(), P_StmtRemoveStmt() */
void
P_StmtUpdateParents (Stmt s, Stmt new)
{
  P_StmtUpdate (P_GetStmtParentStmt (s), s, new);
  
  return;
}

/*! \brief Inserts an expression into a statement list.
 *
 * \param s
 *  the stmt to insert the expression after.
 * \param e
 *  the expression to insert.
 *
 * Inserts a new expression stmt containing \a e into \a s's statement list
 * after \a s.
 *
 * \sa P_StmtUpdate(), P_StmtUpdateParents(),
 * P_StmtInsertExprBefore(), P_StmtInsertExprBeforeLabel()
 * P_StmtInsertStmtAfter(), P_StmtInsertStmtBefore(),
 * P_StmtInsertStmtBeforeLabel(), P_StmtAddLabel(),
 * P_StmtAddLabelAfter(), P_StmtRemoveStmt() */
void
P_StmtInsertExprAfter (Stmt s, Expr e)
{
  Stmt new = P_NewStmtWithType (ST_EXPR);

  P_SetStmtExpr (new, e);

  P_StmtInsertStmtAfter (s, new);

  return;
}

/*! \brief Inserts an expression into a statement list.
 *
 * \param s
 *  the stmt to insert the expression after.
 * \param e
 *  the expression to insert.
 *
 * Inserts a new expression stmt containing \a e into \a s's statement list
 * before \a s.  Label attached to \a s are moved to the new stmt.
 *
 * before:     LABEL: s;
 *
 * after:      LABEL: stmt containing e;
 *                    s;
 *
 * \sa P_StmtUpdate(), P_StmtUpdateParents(), P_StmtInsertExprAfter(),
 * P_StmtInsertExprBeforeLabel(), P_StmtInsertStmtAfter(),
 * P_StmtInsertStmtBefore(), P_StmtInsertStmtBeforeLabel(),
 * P_StmtAddLabel(), P_StmtAddLabelAfter(), P_StmtRemoveStmt() */
void
P_StmtInsertExprBefore (Stmt s, Expr e)
{
  Stmt new = P_NewStmtWithType (ST_EXPR);

  P_SetStmtExpr (new, e);

  P_StmtInsertStmtBefore (s, new);

  return;
}

/*! \brief Inserts an expression into a statement list.
 *
 * \param s
 *  the stmt to insert the expression after.
 * \param e
 *  the expression to insert.
 *
 * Inserts a new expression stmt containing \a e into \a s's statement list
 * before \a s.  Labels on \a s remain on \a s.
 *
 * before:     LABEL: s;
 *
 * after:             stmt containing e;
 *             LABEL: s;
 *
 * \sa P_StmtUpdate(), P_StmtUpdateParents(), P_StmtInsertExprAfter(),
 * P_StmtInsertExprBefore(), P_StmtInsertStmtAfter(),
 * P_StmtInsertStmtBefore(), P_StmtInsertStmtBeforeLabel(),
 * P_StmtAddLabel(), P_StmtAddLabelAfter(), P_StmtRemoveStmt() */
void
P_StmtInsertExprBeforeLabel (Stmt s, Expr e)
{
  Stmt new = P_NewStmtWithType (ST_EXPR);

  P_SetStmtExpr (new, e);

  P_StmtInsertStmtBeforeLabel (s, new);

  return;
}

/*! \brief Inserts a statement into a statement list.
 *
 * \param s
 *  the stmt to insert the statement after.
 * \param new
 *  the statement to insert.
 *
 * Inserts statement \a new into \a s's statement list after \a s.
 *
 * \sa P_StmtUpdate(), P_StmtUpdateParents(), P_StmtInsertExprAfter(),
 * P_StmtInsertExprBefore(), P_StmtInsertExprBeforeLabel(),
 * P_StmtInsertStmtBefore(), P_StmtInsertStmtBeforeLabel(),
 * P_StmtAddLabel(), P_StmtAddLabelAfter(), P_StmtRemoveStmt() */
void
P_StmtInsertStmtAfter (Stmt s, Stmt new)
{
  if (s->lex_next)
    s->lex_next->lex_prev = new;
  new->lex_next = s->lex_next;
  new->lex_prev = s;
  s->lex_next = new;

  P_SetStmtParentStmt (new, P_GetStmtParentStmt (s));
  P_SetStmtParentFunc (new, P_GetStmtParentFunc (s));
  P_SetStmtParentExpr (new, P_GetStmtParentExpr (s));

  return;
}

/*! \brief Inserts a statement into a statement list.
 *
 * \param s
 *  the stmt to insert the statement before.
 * \param new
 *  the statement ot insert.
 *
 * Inserts statement \a new into \a s's statement list before \a s.  Any
 * labels on \a s are moved to \a new.
 *
 * before:     LABEL: s;
 *
 * after:      LABEL: new;
 *                    s;
 *
 * \sa P_StmtUpdate(), P_StmtUpdateParents(), P_StmtInsertExprAfter(),
 * P_StmtInsertExprBefore(), P_StmtInsertExprBeforeLabel(),
 * P_StmtInsertStmtAfter(), P_StmtInsertStmtBeforeLabel(),
 * P_StmtAddLabel(), P_StmtAddLabelAfter(), P_StmtRemoveStmt() */
void
P_StmtInsertStmtBefore (Stmt s, Stmt new)
{
  if (s->lex_prev)
    s->lex_prev->lex_next = new;
  new->lex_prev = s->lex_prev;
  new->lex_next = s;
  s->lex_prev = new;

  /* Update any parent stmts to refer to new instead of s. */
  P_StmtUpdateParents (s, new);

  P_SetStmtParentStmt (new, P_GetStmtParentStmt (s));
  P_SetStmtParentFunc (new, P_GetStmtParentFunc (s));
  P_SetStmtParentExpr (new, P_GetStmtParentExpr (s));

  P_SetStmtLabels (new, P_StmtExtractLabels (s));

  return;
}

/*! \brief Inserts a statement into a statement list.
 *
 * \param s
 *  the stmt to insert the statement before.
 * \param new
 *  the statement ot insert.
 *
 * Inserts statement \a new into \a s's statement list before \a s.  Any
 * labels on \a s stay on \a s.
 *
 * before:     LABEL: s;
 *
 * after:             new;
 *             LABEL: s;
 *
 * \sa P_StmtUpdate(), P_StmtUpdateParents(), P_StmtInsertExprAfter(),
 * P_StmtInsertExprBefore(), P_StmtInsertExprBeforeLabel(),
 * P_StmtInsertStmtAfter(), P_StmtInsertStmtBeforeLabel(),
 * P_StmtAddLabel(), P_StmtAddLabelAfter(), P_StmtRemoveStmt() */
void
P_StmtInsertStmtBeforeLabel (Stmt s, Stmt new)
{
  if (s->lex_prev)
    s->lex_prev->lex_next = new;
  new->lex_prev = s->lex_prev;
  new->lex_next = s;
  s->lex_prev = new;

  /* Update any parent stmts to refer to new instead of s. */
  P_StmtUpdateParents (s, new);

  P_SetStmtParentStmt (new, P_GetStmtParentStmt (s));
  P_SetStmtParentFunc (new, P_GetStmtParentFunc (s));
  P_SetStmtParentExpr (new, P_GetStmtParentExpr (s));

  return;
}

/*! \brief Adds a label to a statement.
 *
 * \param s
 *  the statement to receive the label.
 * \param l
 *  the label to add to the statement.
 *
 * \sa P_StmtUpdate(), P_StmtUpdateParents(), P_StmtInsertExprAfter(),
 * P_StmtInsertExprBefore(), P_StmtInsertStmtAfter(),
 * P_StmtInsertStmtBefore(), P_StmtAddLabelAfter(), P_StmtRemoveStmt() */
void
P_StmtAddLabel (Stmt s, Label l)
{
  P_SetStmtLabels (s, P_AppendLabelNext (P_GetStmtLabels (s), l));

  return;
}

/*! \brief Adds a label after a statement.
 *
 * \param s
 *  the statement after which to insert the label.
 * \param l
 *  the label to add to the statement.
 *
 * Adds a new nop statement following \a s and attaches \a l.
 *
 * \sa P_StmtUpdate(), P_StmtUpdateParents(), P_StmtInsertExprAfter(),
 * P_StmtInsertExprBefore(), P_StmtInsertStmtAfter(),
 * P_StmtInsertStmtBefore(), P_StmtAddLabel(), P_StmtRemoveStmt() */
void
P_StmtAddLabelAfter (Stmt s, Label l)
{
  Stmt target;
  if (!(target = P_GetStmtLexNext (s)))
    {
      P_StmtInsertStmtAfter (s, P_NewStmtWithType (ST_NOOP));
      target = P_GetStmtLexNext (s);
    }

  P_StmtAddLabel (target, l);

  return;
}

/*! \brief Extract labels from a statement.
 *
 * \param s
 *  the Stmt to inspect.
 *
 * \return
 *  a list of labels extracted from the statement.
 *
 * This function extracts all labels on \a s and all nested statements
 * up to the first non-noop.  Labels are removed from the statements and
 * returned in a list.
 * 
 * When we flatten an expression, we have to propagate the label associated 
 * with the old expression to the new one. For example:
 *
 * L1: a(b());		==> L1: t1 = b();
 *				a(t1);
 * But sometimes the label is hidden under the first level statement, 
 *
 * { L1: a(b()); }	==> L1: t1 = b();
 *				{ a(t1); }
 * Or even complicate:
 *
 * L1 : { L2: a(b()); }	==> L1: L2: t1 = b();
 *				{ a(t1); }
 */
Label
P_StmtExtractLabels (Stmt s)
{
  Label labels = NULL;

  if (s)
    {
      labels = P_GetStmtLabels (s);
      P_SetStmtLabels (s, NULL);

      if (P_GetStmtType (s) == ST_COMPOUND)
	{
	  Compound c = P_GetStmtCompound (s);
	  labels = P_AppendLabelNext \
	             (labels, P_StmtExtractLabels (P_GetCompoundStmtList (c)));
	}
    }

  return (labels);
}

/*! \brief Removes a statement from a statement list.
 *
 * \param s
 *  the stmt to remove from the list.
 *
 * \return
 *  The new head of the stmt list.
 *
 * Removes stmt \a s from the statement list.  Sets \a s->lex_next to null
 * so \a s can be freed.  This function does not free \a s.  For that,
 * see P_RemoveStmt().
 *
 * \sa P_StmtInsertExprAfter(), P_StmtInsertExprBefore(),
 * P_StmtInsertStmtAfter(), P_StmtInsertStmtBefore(), P_RemoveStmt()
 */
Stmt
P_StmtRemoveStmt (Stmt s)
{
  Stmt prev = P_GetStmtLexPrev (s);
  Stmt next = P_GetStmtLexNext (s);
  Stmt result;

  if (prev)
    P_SetStmtLexNext (prev, next);
  if (next)
    P_SetStmtLexPrev (next, prev);

  P_SetStmtLexNext (s, NULL);

  /* If s was the first stmt in the list, update any parents to refer
   * to the second stmt. */
  if (prev == NULL)
    {
      P_StmtUpdateParents (s, next);
      result = next;
    }
  else /* Find the first stmt in the list. */
    {
      for (result = prev; P_GetStmtLexPrev (result);
	   result = P_GetStmtLexPrev (result));
    }

  return (result);
}

/*! \brief Used to generate a flat list of Expr IDs from a Stmt
 *
 * \param expr
 *  the expr to inspect
 * \param data
 *  a List onto which \a expr's id will be appended.
 */
static void
append_expr_id (Expr expr, void *data)
{
  List *id_list = (List *)data;

#ifdef LP64_ARCHITECTURE
  *id_list = List_insert_last (*id_list, (void *)((long)P_GetExprID (expr)));
#else
  *id_list = List_insert_last (*id_list, (void *)((int)P_GetExprID (expr)));
#endif

  return;
}

/*! \brief Generates a mapping of Expression IDs between two statements.
 *
 * \param orig
 *  the original statement.
 * \param copy
 *  the copy of \a orig.
 *
 * \return
 *  A mapping of Expr IDs in \a orig to the corresponding Expr IDs in \a copy.
 *
 * The mapping is returned in a HashTable, where Expr IDs in \a orig key
 * the corresponding ID in \a copy.
 */
HashTable
P_StmtBuildExprMap (Stmt orig, Stmt copy)
{
  HashTable result = HashTable_create (256);
  List orig_ids = NULL, copy_ids = NULL;
  int orig_id, copy_id;

  P_StmtApply (orig, NULL, append_expr_id, &orig_ids);
  P_StmtApply (copy, NULL, append_expr_id, &copy_ids);

  /* Add -1 to the end of both lists so we know when we see the last ID.
   * (0 is a valid Expr ID). */
  orig_ids = List_insert_last (orig_ids, (void *)-1);
  copy_ids = List_insert_last (copy_ids, (void *)-1);

  List_start (orig_ids);
  List_start (copy_ids);
  /* Make sure we pull the next list before doing the logical comparison
   * to make sure that the short-circuit doesn't prevent us from pulling
   * the second list. */
#ifdef LP64_ARCHITECTURE
  while (orig_id = (int)((long)List_next (orig_ids)), \
	 copy_id = (int)((long)List_next (copy_ids)), \
	 orig_id != -1 && copy_id != -1)
    HashTable_insert (result, orig_id, (void *)(long)copy_id);
#else
  while (orig_id = (int)List_next (orig_ids), \
	 copy_id = (int)List_next (copy_ids), \
	 orig_id != -1 && copy_id != -1)
    HashTable_insert (result, orig_id, (void *)copy_id);
#endif

  if ((orig_id != -1) ^ (copy_id != -1))
    P_punt ("struct.c:P_StmtBuildExprMap:%d Expr trees differ", __LINE__);
    
  return (result);
}

/* Label access functions. */
/*! \brief Sets the Label.data.parent field.
 *
 * \param l
 *  the ::Label.
 * \param p
 *  the new value of the Label.data.parent field (\a p).
 *
 * \return
 *  The new value of the Label.data.parent field.
 *
 * If the label's type is LB_LABEL, sets the Label.data.parent field.
 */
Stmt
P_SetLabelParentStmt (Label l, Stmt p)
{
  if (l)
    {
      if (l->type == LB_LABEL)
	l->data.parent = p;
      else
	{
	  if (l->data.expression)
	    l->data.expression->parentstmt = p;
	}
    }
  else
    P_punt ("struct.c:P_SetLabelParentStmt:%d struct Label (l) is NULL",
	    __LINE__ - 1);

  return (p);
}

/*! \brief Sets the parent statement all ::Labels in a list.
 *
 * \param l
 *  the ::Label.
 * \param p
 *  the new parent statement for the label.
 * \param direction
 *  the direction the list should be traversed.  FORWARD (via next) or
 *  REVERSE (via prev).
 *
 * \return
 *  The new parent statement for the label (\a p).
 *
 * Walks the label list headed by \a l and sets the parent field for any
 * labels of type LB_LABEL.
 */
Stmt
P_SetLabelParentStmtDir (Label l, Stmt p, int direction)
{
  Label cur_label;

  if (l)
    {
      switch (direction)
	{
	case FORWARD:
	  for (cur_label = l; cur_label; cur_label = cur_label->next)
	    P_SetLabelParentStmt (cur_label, p);
	  break;

	case REVERSE:
	  for (cur_label = l; cur_label; cur_label = cur_label->prev)
	    P_SetLabelParentStmt (cur_label, p);
	  break;

	default:
	  P_punt ("struct.c:P_SetLabelParentStmtDir:%d invalid direction %d",
		  __LINE__ - 1);
	}
    }
  else
    P_punt ("struct.c:P_SetLabelParentStmtDir:%d struct Label (l) is NULL",
	    __LINE__ - 1);

  return (p);
}

/*! \brief Gets the parent statement for a ::Label.
 *
 * \param l
 *  the ::Label.
 *
 * \return
 *  The parent statement for the label.
 *
 * Walks the label list headed by \a l and return the parent field for the
 * first label of type LB_LABEL, or NULL, if none exists.
 */
Stmt
P_GetLabelParentStmt (Label l)
{
  Label cur_label;
  Stmt result = NULL;

  if (l)
    {
      for (cur_label = l; cur_label; cur_label = cur_label->next)
	if (l->type == LB_LABEL)
	  {
	    result = l->data.parent;
	    break;
	  }
      if (result == NULL)
	{
	  for (cur_label = l; cur_label; cur_label = cur_label->prev)
	    if (l->type == LB_LABEL)
	      {
		result = l->data.parent;
		break;
	      }
	}
    }
  else
    P_punt ("struct.c:P_GetLabelParentStmt:%d struct Label (l) is NULL",
	    __LINE__ - 1);

  return (result);
}

/*! \brief Sets the Label.next field.
 *
 * \param l
 *  the ::Label.
 * \param n
 *  the new value of the Label.next field.
 *
 * \return
 *  The new value of the Label.next field (\a n).
 *
 * Sets \a n as \a l.next.  If \a n is of type LB_LABEL, sets its
 * data.parent field to the parent of \a n (if known).
 *
 * \note This function sets the Label.next field explicitly.  To maintain
 *       a Label list, see P_AppendLabelNext().
 */
Label
P_SetLabelNext (Label l, Label n)
{
  Stmt parent = NULL;

  if (l)
    {
      /* This function can't assume that it can set the old next's parent
       * field to NULL, since it is still valid for it to refer to l's
       * parent. */

      l->next = n;

      if (l->next && (parent = P_GetLabelParentStmt (l)))
	P_SetLabelParentStmtAll (l->next, parent);
    }
  else
    P_punt ("struct.c:P_SetLabelNext:%d struct Label (l) is NULL", __LINE__);

  return (n);
}

/*! \brief Appends a ::Label to a ::Label list.
 *
 * \param l
 *  the head of the list to append.
 * \param n
 *  the ::Label to append to the list.
 *
 * \return
 *  A pointer to the head of the list.
 *
 * Appends ::Label \a n to the tail of the list headed by \a l.  This
 * function also sets the Label.prev field of \a n appropriately.  \a l may
 * be null, in which case \a n is returned as the head of the list.
 * If \a n is of type LB_LABEL, sets its data.parent field to the parent
 * of \a n (if known).
 *
 * \note This function maintains a Label list.  To set the Label.next
 *       field explicitly, see P_SetLabelNext().
 */
Label
P_AppendLabelNext (Label l, Label n)
{
  Label t = l;
  Stmt parent = NULL;

  if (l)
    {
      while (t->next)
	t = t->next;

      t->next = n;

      if (t->next)
	{
	  t->next->prev = t;

	  if ((parent = P_GetLabelParentStmt (l)))
	    P_SetLabelParentStmtAll (t->next, parent);
	}
    }
  else
    l = n;

  return (l);
}

/*! \brief Sets the Label.prev field.
 *
 * \param l
 *  the ::Label.
 * \param p
 *  the new value of the Label.prev field.
 *
 * \return
 *  The new value of the Label.prev field (\a p).
 *
 * Sets \a p as \a l.prev.  If \a p is of type LB_LABEL, sets its
 * data.parent field to the parent of \a n (if known).
 */
Label
P_SetLabelPrev (Label l, Label p)
{
  Stmt parent = NULL;

  if (l)
    {
      /* This function can't assume that it can set prev's parent field
       * to NULL, since it is still valid for it to refer to l's parent. */

      l->prev = p;

      if (l->prev && (parent = P_GetLabelParentStmt (l)))
	  P_SetLabelParentStmtDir (l->prev, parent, REVERSE);
    }
  else
    P_punt ("struct.c:P_SetLabelPrev:%d struct Label (l) is NULL", __LINE__);

  return (p);
}

/*! \brief Sets the identifier (key and name) of a ::Label.
 *
 * \param l
 *  the ::Label.
 * \param i
 *  the new identifier.
 *
 * \return
 *  The new identifier.
 *
 * \note The values from the ::Identifier struct are copied, so the caller is
 * responsible for freeing the ::Identifier passed as \a i.
 */
Identifier
P_SetLabelIdentifier (Label l, Identifier i)
{
  char *val;

  if (l)
    {
      if (l->val)
	free (l->val);
      if ((val = P_GetIdentifierName (i)))
	P_SetLabelVal (l, strdup (val));
      else
	P_SetLabelVal (l, NULL);

      P_SetLabelKey (l, P_GetIdentifierKey (i));
    }

  return (i);
}

/*! \brief Gets the identifier (name and key) from a ::Label.
 *
 * \param l
 *  the ::Label.
 *
 * \return
 *  A pointer to an ::Identifier struct for the ::Label.
 *
 * \note This function allocates a new ::Identifier and returns a pointer.
 * It is the caller's responsibility to free the ::Identifier using
 * P_RemoveIdentifier().
 *
 * \sa P_RemoveIdentifier() */
Identifier
P_GetLabelIdentifier (Label l)
{
  Identifier result;
  char *val;

  if (l)
    {
      result = P_NewIdentifier ();

      P_SetIdentifierKey (result, P_GetLabelKey (l));
      if ((val = P_GetLabelVal (l)))
	P_SetIdentifierName (result, strdup (val));

      return (result);
    }
  else
    P_punt ("struct.c:P_GetLabelIdentifier:%d struct Label (l) is NULL",
	    __LINE__ - 1);

  /* Should never reach this point. */
  return (NULL);
}

/* Compound access functions. */
/*! \brief Sets the Compound.stmt_list field.
 *
 * \param c
 *  the ::Compound.
 * \param s
 *  the new value of the Compound.stmt_list field.
 *
 * \return
 *  The new value of the Compound.stmt_list field.
 *
 * Sets the Compound.stmt_list field.  If Compound.parent is defined, copies
 * the Compound's parent statement to \a s.
 */
Stmt
P_SetCompoundStmtList (Compound c, Stmt s)
{
  if (c)
    {
      /* This function can't assume that it can set the old stmt_list's
       * parent field to be NULL, since it is still valid for it to
       * refer to c's parent. */

      c->stmt_list = s;

      if (c->stmt_list)
	{
	  P_SetStmtParentStmtAll (c->stmt_list, c->parent);
	  P_SetStmtParentFuncAll (c->stmt_list, NULL);
	  P_SetStmtParentExprAll (c->stmt_list, NULL);
	}
    }
  else
    P_punt ("struct.c:P_SetCompoundStmtList:%d struct Compound (c) is NULL",
	    __LINE__ - 1);

  return (s);
}

/*! \brief Sets the parent statement for the ::Compound.
 *
 * \param c
 *  the ::Compound.
 * \param p
 *  the new parent statement.
 *
 * \return
 *  The new parent statement (\a p).
 *
 * Sets the parent statement for all statements in the Compound.stmt_list
 * field.
 *
 * \note This function sets the parent statement for the compound and all
 *       statements in Compound.stmt_list.  To set the parent statement
 *       for the compound only, see #P_SetCompoundParent().
 */
Stmt
P_SetCompoundParentStmtAll (Compound c, Stmt p)
{
  if (c)
    {
      c->parent = p;

      if (c->stmt_list)
	{
	  P_SetStmtParentStmtAll (c->stmt_list, p);
	  P_SetStmtParentFuncAll (c->stmt_list, NULL);
	  P_SetStmtParentExprAll (c->stmt_list, NULL);
	}
    }
  else
    P_punt ("struct.c:P_SetCompoundParentStmtAll:%d struct Compound (c) is "
	    "NULL", __LINE__ - 1);

  return (p);
}

/*! \brief Returns an identifier for a symbol local to a compound.
 *
 * \param s
 *  the ST_COMPOUND statement under which the identifier will be defined.
 * \param tag
 *  (optional) a short tag that can be embedded in the identifier to identify
 *  the code that created it.
 *
 * \return
 *  An identifier of the form PT_(tag)_(scope)_(serial).
 *
 * The caller is responsible for freeing the returned string.
 */
char *
P_CompoundNewIdentifier (Stmt s, char *tag)
{
  Compound c = P_GetStmtCompound (s);
  Key scope = P_GetStmtKey (s);
  char result[P_MAX_IDENTIFIER_LEN];

  result[0] = '\0';

  if (tag)
    snprintf (result, P_MAX_IDENTIFIER_LEN, "PT_%s_%d_%d_%d", tag, scope.file,
	      scope.sym, P_GetCompoundUniqueVarID (c)++);
  else
    snprintf (result, P_MAX_IDENTIFIER_LEN, "PT__%d_%d_%d", scope.file,
	      scope.sym, P_GetCompoundUniqueVarID (c)++);

  return (strdup (result));
}

/* IfStmt access functions. */
/*! \brief Sets the IfStmt.cond_expr field.
 *
 * \param i
 *  the ::IfStmt.
 * \param c
 *  the new value of the IfStmt.cond_expr field.
 *
 * \return
 *  The new value of the IfStmt.cond_expr field.
 *
 * Sets the IfStmt.cond_expr field.  If IfStmt.parent is set, sets
 * \a c's parent statement.
 */
Expr
P_SetIfStmtCondExpr (IfStmt i, Expr c)
{
  if (i)
    {
      /* This function can't assume that it can set the old cond_expr's
       * parent field to NULL, since it is still valid for it to
       * refer to i's parent. */

      i->cond_expr = c;

      if (i->cond_expr)
	{
	  P_SetExprParentStmtAll (i->cond_expr, i->parent);
	  P_SetExprParentExprAll (i->cond_expr, NULL);
	}
    }
  else
    P_punt ("struct.c:P_SetIfStmtCondExpr:%d struct IfStmt (i) is NULL",
	    __LINE__ - 1);

  return (c);
}

/*! \brief Sets the IfStmt.then_block field.
 *
 * \param i
 *  the ::IfStmt.
 * \param t
 *  the new value of the IfStmt.then_block field.
 *
 * \return
 *  The new value of the IfStmt.then_block field.
 *
 * Sets the IfStmt.then_block field.  If IfStmt.parent is set, sets
 * \a t's parent statement.
 */
Stmt
P_SetIfStmtThenBlock (IfStmt i, Stmt t)
{
  if (i)
    {
      /* This function can't assume that it can set the old then_block's
       * parent field to NULL, since it is still valid for it to
       * refer to i's parent. */

      i->then_block = t;

      if (i->then_block)
	{
	  P_SetStmtParentStmtAll (i->then_block, i->parent);
	  P_SetStmtParentFuncAll (i->then_block, NULL);
	  P_SetStmtParentExprAll (i->then_block, NULL);
	}
    }
  else
    P_punt ("struct.c:P_SetIfStmtThenBlock:%d struct IfStmt (i) is NULL",
	    __LINE__ - 1);

  return (t);
}


/*! \brief Sets the IfStmt.else_block field.
 *
 * \param i
 *  the ::IfStmt.
 * \param e
 *  the new value of the IfStmt.else_block field.
 *
 * \return
 *  The new value of the IfStmt.else_block field.
 *
 * Sets the IfStmt.else_block field.  If IfStmt.parent is set, sets
 * \a e's parent statement.
 */
Stmt
P_SetIfStmtElseBlock (IfStmt i, Stmt e)
{
  if (i)
    {
      /* This function can't assume that it can set the old else_block's
       * parent field to NULL, since it is still valid for it to
       * refer to i's parent. */

      i->else_block = e;

      if (i->else_block)
	{
	  P_SetStmtParentStmtAll (i->else_block, i->parent);
	  P_SetStmtParentFuncAll (i->else_block, NULL);
	  P_SetStmtParentExprAll (i->else_block, NULL);
	}
    }
  else
    P_punt ("struct.c:P_SetIfStmtElseBlock:%d struct IfStmt (i) is NULL",
	    __LINE__ - 1);

  return (e);
}

/*! \brief Sets the parent statement for an ::IfStmt.
 *
 * \param i
 *  the ::IfStmt.
 * \param p
 *  the new parent statement.
 *
 * \return
 *  The new parent statement (\a p);
 *
 * Sets the parent statement for all statements in IfStmt.then_block and
 * IfStmt.else_block and for all expressions in IfStmt.cond_expr.
 *
 * \note This function sets the parent field for the IfStmt and all
 *       sub-structures.  To set the IfStmt.parent field only, see
 *       #P_SetIfStmtParentStmt().
 */
Stmt
P_SetIfStmtParentStmtAll (IfStmt i, Stmt p)
{
  if (i)
    {
      i->parent = p;

      if (i->cond_expr)
	{
	  P_SetExprParentStmtAll (i->cond_expr, p);
	  P_SetExprParentExprAll (i->cond_expr, NULL);
	}
      if (i->then_block)
	{
	  P_SetStmtParentStmtAll (i->then_block, p);
	  P_SetStmtParentFuncAll (i->then_block, NULL);
	  P_SetStmtParentExprAll (i->then_block, NULL);
	}
      if (i->else_block)
	{
	  P_SetStmtParentStmtAll (i->else_block, p);
	  P_SetStmtParentFuncAll (i->else_block, NULL);
	  P_SetStmtParentExprAll (i->else_block, NULL);
	}
    }
  else
    P_punt ("struct.c:P_SetIfStmtParentStmtAll:%d struct IfStmt (i) is NULL",
	    __LINE__ - 1);

  return (p);
}

/* SwitchStmt access functions. */
/*! \brief Sets the SwitchStmt.expression field.
 *
 * \param s
 *  the ::SwitchStmt.
 * \param e
 *  the new value of the SwitchStmt.expression field.
 *
 * \return
 *  The new value of the SwitchStmt.expression field (\a e).
 *
 * Sets the SwitchStmt.expression.  If the SwitchStmt.parent field is
 * set, sets the parent field of \a e.
 */
Expr
P_SetSwitchStmtExpression (SwitchStmt s, Expr e)
{
  if (s)
    {
      /* This function can't assume that it can set the old expression's
       * parent field to NULL, since it is still valid for it to
       * refer to s's parent. */

      s->expression = e;

      if (s->expression)
	{
	  P_SetExprParentStmtAll (s->expression, s->parent);
	  P_SetExprParentExprAll (s->expression, NULL);
	}
    }
  else
    P_punt ("struct.c:P_SetSwitchStmtExpression:%d struct SwitchStmt (s) is "
	    "NULL", __LINE__ - 1);

  return (e);
}

/*!\ brief Sets the SwitchStmt.switchbody field.
 *
 * \param s
 *  the ::SwitchStmt.
 * \param t
 *  the new value of the SwitchStmt.switchbody field.
 *
 * \return
 *  The new value of the SwitchStmt.switchbody field (\a t).
 *
 * Sets the SwitchStmt.switchbody.  If the SwitchStmt.parent field is
 * set, sets the parent field of \a t.
 */
Stmt
P_SetSwitchStmtSwitchBody (SwitchStmt s, Stmt t)
{
  if (s)
    {
      /* This function can't assume that it can set the old switchbody's
       * parent field to NULL, since it is still valid for it to
       * refer to s's parent. */

      s->switchbody = t;

      if (s->switchbody)
	{
	  P_SetStmtParentStmtAll (s->switchbody, s->parent);
	  P_SetStmtParentFuncAll (s->switchbody, NULL);
	  P_SetStmtParentExprAll (s->switchbody, NULL);
	}
    }
  else
    P_punt ("struct.c:P_SetSwitchStmtSwitchBody:%d struct SwitchStmt (s) is "
	    "NULL", __LINE__ - 1);

  return (t);
}

/*! \brief Sets the parent statement for a ::SwitchStmt.
 *
 * \param s
 *  the ::SwitchStmt.
 * \param p
 *  the new parent statement.
 *
 * \return
 *  The new parent statement (\a p).
 *
 * Sets the parent statement for all statements in SwitchStmt.switchbody and
 * for all expressions in SwitchStmt.expression.
 *
 * \note This function sets the parent statement for all sub-structures
 *       of \a s.  To set only the SwitchStmt.parent field, see
 *       #P_SetSwitchStmtParentStmt().
 */
Stmt
P_SetSwitchStmtParentStmtAll (SwitchStmt s, Stmt p)
{
  if (s)
    {
      s->parent = p;

      if (s->expression)
	{
	  P_SetExprParentStmtAll (s->expression, p);
	  P_SetExprParentExprAll (s->expression, NULL);
	}
      if (s->switchbody)
	{
	  P_SetStmtParentStmtAll (s->switchbody, p);
	  P_SetStmtParentFuncAll (s->switchbody, NULL);
	  P_SetStmtParentExprAll (s->switchbody, NULL);
	}
    }
  else
    P_punt \
      ("struct.c:P_SetSwitchStmtParentStmtAll:%d struct SwitchStmt (s) is "
       "NULL", __LINE__ - 1);

  return (p);
}

/* Pstmt access functions. */
/*! \brief Sets the Pstmt.stmt field.
 *
 * \param p
 *  the ::Pstmt.
 * \param s
 *  the new value of the Pstmt.stmt field.
 *
 * \return
 *  The new value of the Pstmt.stmt field (\a s).
 *
 * Sets the Pstmt.stmt field.  If the Pstmt.parent field is set, 
 * set's \a s's parent field.
 */
Stmt
P_SetPstmtStmt (Pstmt p, Stmt s)
{
  if (p)
    {
      /* This function can't assume that it can set the old stmt's
       * parent field to NULL, since it is still valid for it to
       * refer to p's parent. */

      p->stmt = s;

      if (p->stmt)
	{
	  P_SetStmtParentStmtAll (p->stmt, p->parent);
	  P_SetStmtParentFuncAll (p->stmt, NULL);
	  P_SetStmtParentExprAll (p->stmt, NULL);
	}
    }
  else
    P_punt ("struct.c:P_SetPstmtStmt:%d struct Pstmt (p) is NULL", __LINE__);

  return (s);
}

/*! \brief Sets the parent statement of a ::Pstmt.
 *
 * \param p
 *  the ::Pstmt.
 * \param q
 *  the new parent statement for the ::Pstmt.
 *
 * \return
 *  The new parent statement for the ::Pstmt (\a q).
 *
 * Sets the parent statement of all statements in the Pstmt.stmt field.
 *
 * \note This function sets the parent statement of all sub-structures
 *       of \a p.  To set the Pstmt.parent field only, see
 *       P_SetPstmtParentStmt().
 */
Stmt
P_SetPstmtParentStmtAll (Pstmt p, Stmt q)
{
  if (p)
    {
      p->parent = q;

      if (p->stmt)
	{
	  P_SetStmtParentStmtAll (p->stmt, q);
	  P_SetStmtParentFuncAll (p->stmt, NULL);
	  P_SetStmtParentExprAll (p->stmt, NULL);
	}
    }
  else
    P_punt ("struct.c:P_SetPstmtParentStmtAll:%d struct Pstmt (p) is NULL",
	    __LINE__ - 1);

  return (q);
}

/*! \brief Sets the position of a ::Pstmt.
 *
 * \param p
 *  the ::Pstmt.
 * \param position
 *  the new position of the ::Pstmt.
 *
 * \return
 *  Phe new position of the ::Pstmt (\a position).
 *
 * \note The values from the ::Position struct are copied, so the caller is
 * responsible for freeing the ::Position passed as \a p.
 */
Position
P_SetPstmtPosition (Pstmt p, Position position)
{
  char *f;

  if (p)
    {
      P_SetPstmtLineno (p, P_GetPositionLineno (position));
      P_SetPstmtColno (p, P_GetPositionColno (position));
      if ((f = P_GetPositionFilename (position)))
	P_SetPstmtFilename (p, strdup (f));
    }
  else
    P_punt ("struct.c:P_SetPstmtPosition:%d struct Pstmt (p) is NULL",
	    __LINE__ - 1);

  return (position);
}

/*! \brief Gets the position of a ::Pstmt.
 *
 * \param p
 *  the ::Pstmt.
 *
 * \return
 *  A pointer to a ::Position struct for the ::Pstmt.
 *
 * \note This function allocates a new ::Position and returns a pointer.  It
 * is the caller's responsibility to free the ::Position using
 * P_RemovePosition().
 *
 * \sa P_RemovePosition()
 */
Position
P_GetPstmtPosition (Pstmt p)
{
  Position result;
  char *f;

  if (p)
    {
      result = P_NewPosition ();

      P_SetPositionLineno (result, P_GetPstmtLineno (p));
      P_SetPositionColno (result, P_GetPstmtColno (p));
      if ((f = P_GetPstmtFilename (p)))
	P_SetPositionFilename (result, strdup (f));

      return (result);
    }
  else
    P_punt ("struct.c:P_GetPstmtPosition:%d struct Pstmt (p) is NULL",
	    __LINE__ - 1);

  /* Should never reach this point. */
  return (NULL);
}

/* Mutex access functions. */
/*! \brief Sets the Mutex.expression field.
 *
 * \param m
 *  the ::Mutex.
 * \param e
 *  the new value of the Mutex.expression field.
 *
 * \return
 *  The new value of the Mutex.expression field.
 */
Expr
P_SetMutexExpression (Mutex m, Expr e)
{
  if (m)
    {
      /* This function can't assume that it can set the old expression's
       * parent field to NULL, since it is still valid for it to
       * refer to m's parent. */

      m->expression = e;

      if (m->expression)
	{
	  P_SetExprParentStmtAll (m->expression, m->parent);
	  P_SetExprParentExprAll (m->expression, NULL);
	}
    }
  else
    P_punt ("struct.c:P_SetMutexExpression:%d struct Mutex (m) is NULL",
	    __LINE__ - 1);

  return (e);
}

/*! \brief Sets the Mutex.statement field.
 *
 * \param m
 *  the ::Mutex.
 * \param s
 *  the new value of the Mutex.statment field.
 *
 * \return
 *  The new value of the Mutex.statment field (\a s).
 */
Stmt
P_SetMutexStatement (Mutex m, Stmt s)
{
  if (m)
    {
      /* This function can't assume that it can set the old statement's
       * parent field to NULL, since it is still valid for it to
       * refer to m's parent. */

      m->statement = s;

      if (m->statement)
	{
	  P_SetStmtParentStmtAll (m->statement, m->parent);
	  P_SetStmtParentFuncAll (m->statement, NULL);
	  P_SetStmtParentExprAll (m->statement, NULL);
	}
    }
  else
    P_punt ("struct.c:P_SetMutexStatement:%d struct Mutex (m) is NULL",
	    __LINE__ - 1);

  return (s);
}

/*! \brief Sets the parent statement of a ::Mutex.
 *
 * \param m
 *  the ::Mutex.
 * \param p
 *  the new parent statement.
 *
 * \return
 *  The new parent statement (\a p).
 *
 * Sets the parent statement of all statements in the Mutex.statement field
 * and all expressions in the Mutex.expression field.
 *
 * \note This function sets the parent statement of all sub-structures
 *       of the Mutex.  To set the Mutex.statement field only, see
 *       #P_SetMutexParentStmt().
 */
Stmt
P_SetMutexParentStmtAll (Mutex m, Stmt p)
{
  if (m)
    {
      m->parent = p;

      if (m->expression)
	{
	  P_SetExprParentStmtAll (m->expression, p);
	  P_SetExprParentExprAll (m->expression, NULL);
	}
      if (m->statement)
	{
	  P_SetStmtParentStmtAll (m->statement, p);
	  P_SetStmtParentFuncAll (m->statement, NULL);
	  P_SetStmtParentExprAll (m->statement, NULL);
	}
    }
  else
    P_punt ("struct.c:P_SetMutexParentStmtAll:%d struct Mutex (m) is NULL",
	    __LINE__ - 1);

  return (p);
}

/* Cobegin access functions. */
/*! \brief Sets the Cobegin.statements field.
 *
 * \param c
 *  the ::Cobegin.
 * \param s
 *  the new value of the Cobegin.statements field.
 *
 * \return
 *  The new value of the Cobegin.statements field (\a s).
 */
Stmt
P_SetCobeginStatements (Cobegin c, Stmt s)
{
  if (c)
    {
      /* This function can't assume that it can set the old statements's
       * parent field to NULL, since it is still valid for it to
       * refer to c's parent. */

      c->statements = s;

      if (c->statements)
	{
	  P_SetStmtParentStmtAll (c->statements, c->parent);
	  P_SetStmtParentFuncAll (c->statements, NULL);
	  P_SetStmtParentExprAll (c->statements, NULL);
	}
    }
  else
    P_punt ("struct.c:P_SetCobeginStatements:%d struct Cobegin (c) is NULL",
	    __LINE__ - 1);

  return (s);
}

/*! \brief Sets the parent statement of a ::Cobegin.
 *
 * \param c
 *  the ::Cobegin.
 * \param p
 *  the new parent statement for the ::Cobegin.
 *
 * \return
 *  The new parent statement for the ::Cobegin.
 *
 * Sets the parent statement of all statements in the Cobegin.statements
 * field.
 *
 * \note This function sets the parent statement of all sub-structures.  To
 *       set the Cobegin.parent field only, see #P_SetCobeginParentStmt().
 */
Stmt
P_SetCobeginParentStmtAll (Cobegin c, Stmt p)
{
  if (c)
    {
      c->parent = p;

      if (c->statements)
	{
	  P_SetStmtParentStmtAll (c->statements, p);
	  P_SetStmtParentFuncAll (c->statements, NULL);
	  P_SetStmtParentExprAll (c->statements, NULL);
	}
    }
  else
    P_punt ("struct.c:P_SetCobeginParentStmtAll:%d struct Cobegin (c) is NULL",
	    __LINE__ - 1);

  return (p);
}

/* BodyStmt access functions. */
/*! \brief Sets the BodyStmt.statement field.
 *
 * \param b
 *  the ::BodyStmt.
 * \param s
 *  the new value of the BodyStmt.statement field.
 *
 * \return
 *  The new value of the BodyStmt.statement field (\a s).
 */
Stmt
P_SetBodyStmtStatement (BodyStmt b, Stmt s)
{
  if (b)
    {
      /* This function can't assume that it can set the old statement's
       * parent field to NULL, since it is still valid for it to
       * refer to b's parent. */

      b->statement = s;

      if (b->statement)
	{
	  P_SetStmtParentStmtAll (b->statement, b->parent);
	  P_SetStmtParentFuncAll (b->statement, NULL);
	  P_SetStmtParentExprAll (b->statement, NULL);
	}
    }
  else
    P_punt ("struct.c:P_SetBodyStmtStatement:%d struct BodyStmt (b) is NULL",
	    __LINE__ - 1);

  return (s);
}

/*! \brief Sets the parent statement of a ::BodyStmt.
 *
 * \param b
 *  the ::BodyStmt.
 * \param p
 *  the new parent statement for the ::BodyStmt.
 *
 * \return
 *  The new parent statement for the ::BodyStmt (\a p).
 *
 * Sets the parent statement of all statement in the BodyStmt.statement
 * field.
 *
 * \note This function sets the parent statement for all sub-structures
 *       of \a b.  To set the BodyStmt.parent field only, see
 *       #P_SetBodyStmtParentStmt().
 */
Stmt
P_SetBodyStmtParentStmtAll (BodyStmt b, Stmt p)
{
  if (b)
    {
      b->parent = p;

      if (b->statement)
	{
	  P_SetStmtParentStmtAll (b->statement, p);
	  P_SetStmtParentFuncAll (b->statement, NULL);
	  P_SetStmtParentExprAll (b->statement, NULL);
	}
    }
  else
    P_punt ("struct.c:P_SetBodyStmtParentStmtAll:%d struct BodyStmt (b) is "
	    "NULL", __LINE__ - 1);

  return (p);
}

/* EpilogueStmt access functions. */
/*! \brief Sets the EpilogueStmt.statement field.
 *
 * \param e
 *  the ::EpilogueStmt.
 * \param s
 *  the new value of the EpilogueStmt.statement field.
 *
 * \return
 *  The new value of the EpilogueStmt.statement field (\a s).
 */
Stmt
P_SetEpilogueStmtStatement (EpilogueStmt e, Stmt s)
{
  if (e)
    {
      /* This function can't assume that it can set the old statement's
       * parent field to NULL, since it is still valid for it to
       * refer to e's parent. */

      e->statement = s;

      if (e->statement)
	{
	  P_SetStmtParentStmtAll (e->statement, e->parent);
	  P_SetStmtParentFuncAll (e->statement, NULL);
	  P_SetStmtParentExprAll (e->statement, NULL);
	}
    }
  else
    P_punt ("struct.c:P_SetEpilogueStmtStatement:%d struct EpilogueStmt (e) "
	    "is NULL", __LINE__ - 1);

  return (s);
}

/*! \brief Sets the parent statement of a ::EpilogueStmt.
 *
 * \param e
 *  the ::EpilogueStmt.
 * \param p
 *  the new parent statement for the ::EpilogueStmt.
 *
 * \return
 *  The new parent statement for the ::EpilogueStmt (\a p).
 *
 * Sets the parent statement of all statement in the EpilogueStmt.statement
 * field.
 *
 * \note This function sets the parent statement for all sub-structures
 *       of \a e.  To set the EpilogueStmt.parent field only, see
 *       #P_SetEpilogueStmtParentStmt().
 */
Stmt
P_SetEpilogueStmtParentStmtAll (EpilogueStmt e, Stmt p)
{
  if (e)
    {
      e->parent = p;

      if (e->statement)
	{
	  P_SetStmtParentStmtAll (e->statement, p);
	  P_SetStmtParentFuncAll (e->statement, NULL);
	  P_SetStmtParentExprAll (e->statement, NULL);
	}
    }
  else
    P_punt ("struct.c:P_SetEpilogueStmtParentStmtAll:%d struct EpilogueStmt "
	    "(e) is NULL", __LINE__ - 1);

  return (p);
}

/* ParLoop access functions. */
/*! \brief Sets the ParLoop.pstmt field.
 *
 * \param p
 *  the ::ParLoop.
 * \param q
 *  the new value of the ParLoop.pstmt field.
 *
 * \return
 *  the new value of the ParLoop.pstmt field (\a q).
 */
Pstmt
P_SetParLoopPstmt (ParLoop p, Pstmt q)
{
  if (p)
    {
      /* This function can't assume that it can set the old parloop's
       * parent field to NULL, since it is still valid for it to
       * refer to p's parent. */

      p->pstmt = q;

      if (p->pstmt)
	P_SetPstmtParentStmtAll (p->pstmt, p->parent);
    }
  else
    P_punt ("struct.c:P_SetParLoopPstmt:%d struct ParLoop (p) is NULL",
	    __LINE__ - 1);

  return (q);
}

/*! \brief Sets the ParLoop.iteration_var field.
 *
 * \param p
 *  the ::ParLoop.
 * \param i
 *  the new value of the ParLoop.iteration_var field.
 *
 * \return
 *  The new value of the ParLoop.iteration_var field (\a i).
 */
Expr
P_SetParLoopIterationVar (ParLoop p, Expr i)
{
  if (p)
    {
      /* This function can't assume that it can set the old iteration_var's
       * parent field to NULL, since it is still valid for it to
       * refer to p's parent. */

      p->iteration_var = i;

      if (p->iteration_var)
	{
	  P_SetExprParentStmtAll (p->iteration_var, p->parent);
	  P_SetExprParentExprAll (p->iteration_var, NULL);
	}
    }
  else
    P_punt ("struct.c:P_SetParLoopIterationVar:%d struct ParLoop (p) is NULL",
	    __LINE__ - 1);

  return (i);
}

/*! \brief Sets the ParLoop.init_value field.
 *
 * \param p
 *  the ::ParLoop.
 * \param i
 *  the new value of the ParLoop.init_value field.
 *
 * \return
 *  The new value of the ParLoop.init_value field (\a i).
 */
Expr
P_SetParLoopInitValue (ParLoop p, Expr i)
{
  if (p)
    {
      /* This function can't assume that it can set the old init_value's
       * parent field to NULL, since it is still valid for it to
       * refer to p's parent. */

      p->init_value = i;

      if (p->init_value)
	{
	  P_SetExprParentStmtAll (p->init_value, p->parent);
	  P_SetExprParentExprAll (p->init_value, NULL);
	}
    }
  else
    P_punt ("struct.c:P_SetParLoopInitValue:%d struct ParLoop (p) is NULL",
	    __LINE__ - 1);

  return (i);
}

/*! \brief Sets the ParLoop.final_value field.
 *
 * \param p
 *  the ::ParLoop.
 * \param f
 *  the new value of the ParLoop.final_value field.
 *
 * \return
 *  The new value of the ParLoop.final_value field (\a f).
 */
Expr
P_SetParLoopFinalValue (ParLoop p, Expr f)
{
  if (p)
    {
      /* This function can't assume that it can set the old final_value's
       * parent field to NULL, since it is still valid for it to
       * refer to p's parent. */

      p->final_value = f;

      if (p->final_value)
	{
	  P_SetExprParentStmtAll (p->final_value, p->parent);
	  P_SetExprParentExprAll (p->final_value, NULL);
	}
    }
  else
    P_punt ("struct.c:P_SetParLoopFinalValue:%d struct ParLoop (p) is NULL",
	    __LINE__ - 1);

  return (f);
}

/*! \brief Sets the ParLoop.incr_value field.
 *
 * \param p
 *  the ::ParLoop.
 * \param i
 *  the new value of the ParLoop.incr_value field.
 *
 * \return
 *  The new value of the ParLoop.incr_value field (\a i).
 */
Expr
P_SetParLoopIncrValue (ParLoop p, Expr i)
{
  if (p)
    {
      /* This function can't assume that it can set the old incr_value's
       * parent field to NULL, since it is still valid for it to
       * refer to p's parent. */

      p->incr_value = i;

      if (p->incr_value)
	{
	  P_SetExprParentStmtAll (p->incr_value, p->parent);
	  P_SetExprParentExprAll (p->incr_value, NULL);
	}
    }
  else
    P_punt ("struct.c:P_SetParLoopIncrValue:%d struct ParLoop (p) is NULL",
	    __LINE__ - 1);

  return (i);
}

/*! \brief Sets the ParLoop.child field.
 *
 * \param p
 *  the ::ParLoop.
 * \param c
 *  the new value of the ParLoop.child field.
 *
 * \return
 *  the new value of the ParLoop.child field (\a c).
 */
Stmt
P_SetParLoopChild (ParLoop p, Stmt c)
{
  if (p)
    {
      /* This function can't assume that it can set the old child's
       * parent field to NULL, since it is still valid for it to
       * refer to p's parent. */

      p->child = c;

      if (p->child)
	{
	  P_SetStmtParentStmtAll (p->child, p->parent);
	  P_SetStmtParentFuncAll (p->child, NULL);
	  P_SetStmtParentExprAll (p->child, NULL);
	}
    }
  else
    P_punt ("struct.c:P_SetParLoopChild:%d struct ParLoop (p) is NULL",
	    __LINE__ - 1);

  return (c);
}

/*! \brief Sets the parent statement of a ::ParLoop.
 *
 * \param p
 *  the ::ParLoop.
 * \param parent
 *  the new parent statement for the ::ParLoop.
 *
 * \return
 *  The new parent statement for the ::ParLoop (\a parent).
 *
 * Sets the parent statement of all statements in the ParLoop.Pstmt,
 * ParLoop.sibling, and ParLoop.child fields.  Sets the parent statement of
 * all expressions in the ParLoop.iteration_var, ParLoop.init_value,
 * ParLoop.final_value, and ParLoop.incr_value fields.
 *
 * \note This function sets the parent statement for all sub-structures.
 *       To set the parent statement for \a p only, see
 *       #P_SetParLoopParentStmt().
 */
Stmt
P_SetParLoopParentStmtAll (ParLoop p, Stmt parent)
{
  if (p)
    {
      p->parent = parent;
      
      if (p->pstmt)
	P_SetPstmtParentStmtAll (p->pstmt, parent);
      if (p->iteration_var)
	{
	  P_SetExprParentStmtAll (p->iteration_var, parent);
	  P_SetExprParentExprAll (p->iteration_var, NULL);
	}
      if (p->sibling)
	{
	  P_SetStmtParentStmtAll (p->sibling, parent);
	  P_SetStmtParentExprAll (p->sibling, NULL);
	}
      if (p->init_value)
	{
	  P_SetExprParentStmtAll (p->init_value, parent);
	  P_SetExprParentExprAll (p->init_value, NULL);
	}
      if (p->final_value)
	{
	  P_SetExprParentStmtAll (p->final_value, parent);
	  P_SetExprParentExprAll (p->final_value, NULL);
	}
      if (p->incr_value)
	{
	  P_SetExprParentStmtAll (p->incr_value, parent);
	  P_SetExprParentExprAll (p->incr_value, NULL);
	}
      if (p->child)
	{
	  P_SetStmtParentStmtAll (p->child, parent);
	  P_SetStmtParentFuncAll (p->child, NULL);
	  P_SetStmtParentExprAll (p->child, NULL);
	}
    }
  else
    P_punt ("struct.c:P_SetParLoopParentStmtAll:%d struct ParLoop (p) is NULL",
	    __LINE__ - 1);

  return (parent);
}

/*! \brief Returns the BodyStmt in the ParLoop's Pstmt.
 *
 * \param p
 *  the ParLoop.
 *
 * \return
 *  A pointer to the ParLoop's BodyStmt.
 *
 * \note This function replaces Parloop_Stmts_Body_Stmt() in the old Pcode.
 */
Stmt
P_GetParLoopBodyStmt (ParLoop p)
{
  Stmt s;

  s = P_GetParLoopPrologueStmt (p);

  if (P_GetStmtType (s) != ST_COMPOUND)
    P_punt ("struct.c:P_GetParLoopBodyStmt:%d ParLoop Stmt must be compound");

  s = P_GetCompoundStmtList (P_GetStmtCompound (s));

  while (s)
    {
      if (P_GetStmtType (s) == ST_BODY)
	return (s);

      s = P_GetStmtLexNext (s);
    }

  return (NULL);
}

/*! \brief Returns the first EpilogueStmt in the ParLoop's Pstmt.
 *
 * \param p
 *  the ParLoop.
 *
 * \return
 *  A pointer to the ParLoop's first EpilogueStmt.
 *
 * \note This function replaces Parloop_Stmts_First_Epilogue_Stmt() in the
 *       old Pcode.
 */
Stmt
P_GetParLoopFirstEpilogueStmt (ParLoop p)
{
  Stmt s;

  s = P_GetParLoopPrologueStmt (p);

  if (P_GetStmtType (s) != ST_COMPOUND)
    P_punt ("struct.c:P_GetParLoopFirstEpilogueStmt:%d ParLoop Stmt must be "
	    "compound");

  s = P_GetCompoundStmtList (P_GetStmtCompound (s));

  while (s)
    {
      if (P_GetStmtType (s) == ST_EPILOGUE)
	return (s);

      s = P_GetStmtLexNext (s);
    }

  return (NULL);
}

/* SerLoop access functions. */
/*! \brief Sets the SerLoop.loop_body field.
 *
 * \param s
 *  the ::SerLoop.
 * \param b
 *  the new value of the SerLoop.loop_body field.
 *
 * \return
 *  The new value of the SerLoop.loop_body field (\a b).
 */
Stmt
P_SetSerLoopLoopBody (SerLoop s, Stmt b)
{
  if (s)
    {
      /* This function can't assume that it can set the old loop_body's
       * parent field to NULL, since it is still valid for it to
       * refer to s's parent. */

      s->loop_body = b;

      if (s->loop_body)
	{
	  P_SetStmtParentStmtAll (s->loop_body, s->parent);
	  P_SetStmtParentFuncAll (s->loop_body, NULL);
	  P_SetStmtParentExprAll (s->loop_body, NULL);
	}
    }
  else
    P_punt ("struct.c:P_SetSerLoopLoopBody:%d struct SerLoop (s) is NULL",
	    __LINE__ - 1);

  return (b);
}

/*! \brief Sets the SerLoop.cond_expr field.
 *
 * \param s
 *  the ::SerLoop.
 * \param c
 *  the new value of the SerLoop.cond_expr field.
 *
 * \return
 *  the new value of the SerLoop.cond_expr field (\a c).
 */
Expr
P_SetSerLoopCondExpr (SerLoop s, Expr c)
{
  if (s)
    {
      /* This function can't assume that it can set the old cond_expr's
       * parent field to NULL, since it is still valid for it to
       * refer to s's parent. */

      s->cond_expr = c;

      if (s->cond_expr)
	{
	  P_SetExprParentStmtAll (s->cond_expr, s->parent);
	  P_SetExprParentExprAll (s->cond_expr, NULL);
	}
    }
  else
    P_punt ("struct.c:P_SetSerLoopCondExpr:%d struct SerLoop (s) is NULL",
	    __LINE__ - 1);

  return (c);
}

/*! \brief Sets the SerLoop.init_expr field.
 *
 * \param s
 *  the ::SerLoop.
 * \param i
 *  the new value of the SerLoop.init_expr field.
 *
 * \return
 *  The new value of the SerLoop.init_expr field (\a i).
 */
Expr
P_SetSerLoopInitExpr (SerLoop s, Expr i)
{
  if (s)
    {
      /* This function can't assume that it can set the old init_expr's
       * parent field to NULL, since it is still valid for it to
       * refer to s's parent. */

      s->init_expr = i;

      if (s->init_expr)
	{
	  P_SetExprParentStmtAll (s->init_expr, s->parent);
	  P_SetExprParentExprAll (s->init_expr, NULL);
	}
    }
  else
    P_punt ("struct.c:P_SetSerLoopInitExpr:%d struct SerLoop (s) is NULL",
	    __LINE__ - 1);

  return (i);
}

/*! \brief Sets the SerLoop.iter_expr field.
 *
 * \param s
 *  the ::SerLoop.
 * \param i
 *  the new value of the SerLoop.iter_expr field.
 *
 * \return
 *  The new value of the SerLoop.iter_expr field (\a i).
 */
Expr
P_SetSerLoopIterExpr (SerLoop s, Expr i)
{
  if (s)
    {
      /* This function can't assume that it can set the old iter_expr's
       * parent field to NULL, since it is still valid for it to
       * refer to s's parent. */

      s->iter_expr = i;

      if (s->iter_expr)
	{
	  P_SetExprParentStmtAll (s->iter_expr, s->parent);
	  P_SetExprParentExprAll (s->iter_expr, NULL);
	}
    }
  else
    P_punt ("struct.c:P_SetSerLoopIterExpr:%d struct SerLoop (s) is NULL",
	    __LINE__ - 1);

  return (i);
}
/*! \brief Sets the parent statement of a ::SerLoop
 *
 * \param s
 *  the ::SerLoop
 * \param p
 *  the new parent statement.
 *
 * \return
 *  the new parent statement (\a p).
 *
 * Sets the parent statement of all statements in SerLoop.loop_body and of
 * all expressions in the SerLoop.cond_expr, SerLoop.init_expr, and
 * SerLoop.iter_expr.
 *
 * \note This function sets the parent statement of sub-structures of \a s.
 *       To set only the SerLoop.parent field, see #P_SetSerLoopParentStmt().
 */
Stmt
P_SetSerLoopParentStmtAll (SerLoop s, Stmt p)
{
  if (s)
    {
      s->parent = p;

      if (s->loop_body)
	{
	  P_SetStmtParentStmtAll (s->loop_body, p);
	  P_SetStmtParentFuncAll (s->loop_body, NULL);
	  P_SetStmtParentExprAll (s->loop_body, NULL);
	}
      if (s->cond_expr)
	{
	  P_SetExprParentStmtAll (s->cond_expr, p);
	  P_SetExprParentExprAll (s->cond_expr, NULL);
	}
      if (s->init_expr)
	{
	  P_SetExprParentStmtAll (s->init_expr, p);
	  P_SetExprParentExprAll (s->init_expr, NULL);
	}
      if (s->iter_expr)
	{
	  P_SetExprParentStmtAll (s->iter_expr, p);
	  P_SetExprParentExprAll (s->iter_expr, NULL);
	}
    }
  else
    P_punt ("struct.c:P_SetSerLoopParentExprAll:%d struct SerLoop (s) is NULL",
	    __LINE__ - 1);

  return (p);
}

/* AsmStmt access functions. */
/*! \brief Sets the AsmStmt.asm_clobbers field.
 *
 * \param a
 *  the ::AsmStmt.
 * \param c
 *  the new value of the AsmStmt.asm_clobbers field.
 *
 * \return
 *  The new value of the AsmStmt.asm_clobbers field (\a c).
 */
Expr
P_SetAsmStmtAsmClobbers (AsmStmt a, Expr c)
{
  if (a)
    {
      /* This function can't assume that it can set the old asm_clobbers's
       * parent field to NULL, since it is still valid for it to
       * refer to a's parent. */

      a->asm_clobbers = c;

      if (a->asm_clobbers)
	{
	  P_SetExprParentStmtAll (a->asm_clobbers, a->parent);
	  P_SetExprParentExprAll (a->asm_clobbers, NULL);
	}
    }
  else
    P_punt ("struct.c:P_SetAsmStmtAsmClobbers:%d struct AsmStmt (a) is NULL",
	    __LINE__ - 1);

  return (c);
}

/*! \brief Sets the AsmStmt.asm_string field.
 *
 * \param a
 *  the ::AsmStmt.
 * \param s
 *  the new value of the AsmStmt.asm_string field.
 *
 * \return
 *  The new value of the AsmStmt.asm_string field (\a s).
 */
Expr
P_SetAsmStmtAsmString (AsmStmt a, Expr s)
{
  if (a)
    {
      /* This function can't assume that it can set the old asm_string's
       * parent field to NULL, since it is still valid for it to
       * refer to a's parent. */

      a->asm_string = s;

      if (a->asm_string)
	{
	  P_SetExprParentStmtAll (a->asm_string, a->parent);
	  P_SetExprParentExprAll (a->asm_string, NULL);
	}
    }
  else
    P_punt ("struct.c:P_SetAsmStmtAsmString:%d struct AsmStmt (a) is NULL",
	    __LINE__ - 1);

  return (s);
}

/*! \brief Sets the AsmStmt.asm_operands field.
 *
 * \param a
 *  the ::AsmStmt.
 * \param o
 *  the new value of the AsmStmt.asm_operands field.
 *
 * \return
 *  The new value of the AsmStmt.asm_operands field (\a o).
 */
Expr
P_SetAsmStmtAsmOperands (AsmStmt a, Expr o)
{
  if (a)
    {
      /* This function can't assume that it can set the old asm_operands's
       * parent field to NULL, since it is still valid for it to
       * refer to a's parent. */

      a->asm_operands = o;

      if (a->asm_operands)
	{
	  P_SetExprParentStmtAll (a->asm_operands, a->parent);
	  P_SetExprParentExprAll (a->asm_operands, NULL);
	}
    }
  else
    P_punt ("struct.c:P_SetAsmStmtAsmOperands:%d struct AsmStmt (a) is NULL",
	    __LINE__ - 1);

  return (o);
}

/*! \brief Sets the parent statement of an ::AsmStmt.
 *
 * \param a
 *  the ::AsmStmt.
 * \param p
 *  the new parent statement.
 *
 * \return
 *  the new parent statement (\a p).
 *
 * Sets the parent statement of all expressions in the AsmStmt.asm_clobbers,
 * AsmStmt.asm_string, and AsmStmt.asm_operands fields.
 *
 * \note This function sets the parent statement of all sub-structures.
 *       To set the AsmStmt.parent field only, see P_SetAsmStmtParentStmt().
 */
Stmt
P_SetAsmStmtParentStmtAll (AsmStmt a, Stmt p)
{
  if (a)
    {
      a->parent = p;

      if (a->asm_clobbers)
	{
	  P_SetExprParentStmtAll (a->asm_clobbers, p);
	  P_SetExprParentExprAll (a->asm_clobbers, NULL);
	}
      if (a->asm_string)
	{
	  P_SetExprParentStmtAll (a->asm_string, p);
	  P_SetExprParentExprAll (a->asm_string, NULL);
	}
      if (a->asm_operands)
	{
	  P_SetExprParentStmtAll (a->asm_operands, p);
	  P_SetExprParentExprAll (a->asm_operands, NULL);
	}
    }
  else
    P_punt ("struct.c:P_SetAsmStmtParentStmtAll:%d struct AsmStmt (a) is NULL",
	    __LINE__ - 1);

  return (p);
}

/* Expr access functions. */
/*! \brief Sets the Expr.value.stmt field.
 *
 * \param e
 *  the ::Expr.
 * \param s
 *  the new value of the Expr.value.stmt field.
 *
 * \return
 *  The new value of the Expr.value.stmt field (\a s).
 *
 * Sets \a s as \a e.value.stmt.  Sets \a s's parent expr to \a e.
 */
Stmt
P_SetExprStmt (Expr e, Stmt s)
{
  if (e)
    {
      if (e->value.stmt && P_GetStmtParentExpr (e->value.stmt) == e)
	P_SetStmtParentExprAll (e->value.stmt, NULL);

      e->value.stmt = s;

      if (e->value.stmt)
	{
	  P_SetStmtParentStmtAll (e->value.stmt, NULL);
	  P_SetStmtParentFuncAll (e->value.stmt, NULL);
	  P_SetStmtParentExprAll (e->value.stmt, e);
	}
    }
  else
    P_punt ("struct.c:P_SetExprStmt:%d struct Expr (e) is NULL", __LINE__);

  return (s);
}

/*! \brief Sets the Expr.sibling field.
 *
 * \param e
 *  the ::Expr.
 * \param s
 *  the new value of the Expr.sibling field.
 *
 * \return
 *  The new value of the Expr.sibling field (\a s).
 *
 * Sets the \a s as \a e.sibling.  Copies \a e's parent pointers to \a s.
 */
Expr
P_SetExprSibling (Expr e, Expr s)
{
  if (e)
    {
      /* This function can't assume that it can set the old sibling's
       * parent fields to NULL, since it is still valid for them to
       * refer to e's parent. */

      e->sibling = s;

      if (e->sibling)
	{
	  P_SetExprParentExprAll (e->sibling, e->parentexpr);
	  P_SetExprParentStmtAll (e->sibling, e->parentstmt);
	}
    }
  else
    P_punt ("struct.c:P_SetExprSibling:%d struct Expr (e) is NULL", __LINE__);

  return (s);
}

/*! \brief Sets the Expr.operands field.
 *
 * \param e
 *  the ::Expr.
 * \param o
 *  the new value of the Expr.operands field.
 *
 * \return
 *  The new value of the Expr.operands field.
 *
 * Sets \a o as the operand field of \a e.  Sets \a e as the parent
 * expr of \a o.
 *
 * \note This function explicitly sets the operands field of \a e.
 *       To maintain a list of operands, use P_AppendExprOperands().
 */
Expr
P_SetExprOperands (Expr e, Expr o)
{
  if (e)
    {
      if (e->operands && P_GetExprParentExpr (e->operands) == e)
	P_SetExprParentExprAll (e->operands, NULL);

      e->operands = o;

      if (e->operands)
	{
	  P_SetExprParentStmtAll (e->operands, e->parentstmt);
	  P_SetExprParentExprAll (e->operands, e);
	}
    }
  else
    {
      P_punt ("struct.c:P_SetExprOperands:%d struct Expr (e) is NULL",
	      __LINE__ - 1);
    }
  
  return (e);
}

/*! \brief Appends an ::Expr to an Expr.operands list.
 *
 * \param e
 *  the head of the list to append.
 * \param o
 *  the ::Expr to append to the Expr.operand list.
 *
 * \return
 *  A pointer to the head of the list.
 *
 * Appends ::Expr \a o to the tail of the Expr.sibling chain on the
 * Expr.operands field of ::Expr \a e.  \a e.operands may be null, in which
 * case it is set to \a o.  \a o's parent expr is set to \a e.  The function
 * returns a pointer to \a e.
 *
 * \note This function maintains an operand list.  To explicitly set
 *       Expr.operands, see P_SetExprOperands().
 */
Expr
P_AppendExprOperands (Expr e, Expr o)
{
  Expr last;

  if (e)
    {
      if ((last = e->operands))
	{
	  while (last->sibling)
	    last = last->sibling;

	  last->sibling = o;
	}
      else
	e->operands = o;

      if (o)
	{
	  P_SetExprParentStmtAll (o, e->parentstmt);
	  P_SetExprParentExprAll (o, e);
	}
    }
  else
    P_punt ("struct.c:P_AppendExprOperands:%d struct Expr (e) is NULL",
	    __LINE__ - 1);

  return (e);
}

/*! \brief Sets the Expr.parentexpr field for siblings and next Exprs.
 *
 * \param e
 *  the ::Expr.
 * \param p
 *  the new value of the Expr.parentexpr field.
 *
 * \return
 *  The new value of the Expr.parentexpr field (\a p).
 *
 * Sets the Expr.parentexpr field for sibling Exprs and all Exprs
 * in the Expr.next field.
 *
 * \note This function sets the parentexpr field for all Exprs in the sibling
 *       and next lists.  To set the field on \a e only, see
 *       #P_SetExprParentExpr().
 */
Expr
P_SetExprParentExprAll (Expr e, Expr p)
{
  /* Done */
  if (!e)
    return(p);

  /* Only update subtree if need be */
  if (e->parentexpr == p)
    return (p);

  e->parentexpr = p;

  P_SetExprParentExprAll (e->operands, e);
  P_SetExprParentExprAll (e->sibling, p);
  P_SetExprParentExprAll (e->next, p);

  return (p);
}

/*! \brief Sets the Expr.parentstmt field for siblings and next Exprs..
 *
 * \param e
 *  the ::Expr.
 * \param p
 *  the new value of the Expr.parentstmt field.
 *
 * \return
 *  The new value of the Expr.parentstmt field (\a p).
 *
 * Sets the Expr.parentstmt field for sibling Exprs and all Exprs
 * in the Expr.next field.
 *
 * \note This function sets the parentstmt field for all Exprs in the sibling
 *       and next lists.  To set the field on \a e only, see
 *       #P_SetExprParentStmt().
 */
Stmt
P_SetExprParentStmtAll (Expr e, Stmt p)
{
  /* Done */
  if (!e)
    return (p);

  /* Only update subtree if need be */
  if (e->parentstmt == p)
    return (p);

  e->parentstmt = p;

  /* Update all subtrees */
  P_SetExprParentStmtAll (e->operands, p);
  P_SetExprParentStmtAll (e->sibling, p);
  P_SetExprParentStmtAll (e->next, p);
  
  return (p);
}

/*! \brief Sets the Expr.parentvar field for siblings and next Exprs..
 *
 * \param e
 *  the ::Expr.
 * \param v
 *  the new value of the Expr.parentvar field.
 *
 * \return
 *  The new value of the Expr.parentvar field (\a v).
 *
 * Sets the Expr.parentvar field for sibling Exprs and all Exprs
 * in the Expr.next field.
 *
 * \note This function sets the parentvar field for all Exprs in the sibling
 *       and next lists.  To set the field on \a e only, see
 *       #P_SetExprParentVar().
 */
VarDcl
P_SetExprParentVarAll (Expr e, VarDcl v)
{
  /* Done */
  if (!e)
    return (v);

  /* Only update subtree if need be */
  if (e->parentvar == v)
    return (v);

  e->parentvar = v;

  /* Update all subtrees */
  P_SetExprParentVarAll (e->operands, v);
  P_SetExprParentVarAll (e->sibling, v);
  P_SetExprParentVarAll (e->next, v);
  
  return (v);
}

/*! \brief Sets the Expr.next field.
 *
 * \param e
 *  the ::Expr.
 * \param n
 *  the new value of the Expr.next field.
 *
 * \return
 *  The new value of the Expr.next field (\a n).
 *
 * Sets \a n as \a e.next.  Copied \a e's parent pointers to \a n.
 *
 * \note This function sets Expr.next explicitly.  To maintain a Expr
 *       list, see P_AppendExprNext().
 */
Expr
P_SetExprNext (Expr e, Expr n)
{
  if (e)
    {
      /* This function can't assume that it can set the old next's
       * parent fields to NULL, since it is still valid for them to
       * refer to e's parent. */

      e->next = n;

      if (e->next)
	{
	  e->next->previous = e;

	  P_SetExprParentExprAll (e->next, e->parentexpr);
	  P_SetExprParentStmtAll (e->next, e->parentstmt);
	  P_SetExprParentVarAll (e->next, e->parentvar);
	}
    }
  else
    P_punt ("struct.c:P_SetExprNext:%d struct Expr (e) is NULL", __LINE__);

  return (n);
}

/*! \brief Appends an ::Expr to an ::Expr list.
 *
 * \param e
 *  the head of the list to append.
 * \param n
 *  the ::Expr to append to the list.
 *
 * \return
 *  A pointer to the head of the list.
 *
 * Appends ::Expr \a n to the tail of the list headed by \a e.  This
 * function also sets the Expr.previous field of \a n appropriately.  \a e
 * may be null, in which case \a n is returned as the head of the list.
 *
 * \note This function maintain an Expr list.  To set Expr.next explicitly,
 *       see P_SetExprNext().
 */
Expr
P_AppendExprNext (Expr e, Expr n)
{
  Expr t = e;
  
  if (e)
    {
      while (t->next)
	t = t->next;

      t->next = n;

      if (t->next)
	{
	  t->next->previous = t;

	  P_SetExprParentExprAll (t->next, e->parentexpr);
	  P_SetExprParentStmtAll (t->next, e->parentstmt);
	  P_SetExprParentVarAll (t->next, e->parentvar);
	}
    }
  else
    e = n;

  return (e);
}

/*! \brief Sets the Expr.previous field.
 *
 * \param e
 *  the ::Expr.
 * \param p
 *  the new value of the Expr.previous field.
 *
 * \return
 *  The new value of the Expr.previous field (\a p).
 *
 * This function sets \a p as \a e.prev and copies \a e's parent pointers
 * to \a p.
 */
Expr
P_SetExprPrevious (Expr e, Expr p)
{
  if (e)
    {
      /* This function can't assume that it can set the old previous's
       * parent fields to NULL, since it is still valid for them to
       * refer to e's parent. */

      e->previous = p;

      if (e->previous)
	{
	  e->previous->next = e;

	  P_SetExprParentExprAll (e, p->parentexpr);
	  P_SetExprParentStmtAll (e, p->parentstmt);
	  P_SetExprParentVarAll (e, p->parentvar);
	}
    }
  else
    P_punt ("struct.c:P_SetExprPrev:%d struct Expr (e) is NULL", __LINE__);

  return (p);
}

/*! \brief Sets the identifier (key and name) of a variable in an ::Expr.
 *
 * \param e
 *  the ::Expr.
 * \param i
 *  the new identifier.
 *
 * \return
 *  The new identifier (\a i).
 *
 * \note The values from the ::Identifier struct are copied, so the caller is
 * responsible for freeing the ::Identifier passed as \a i.
 */
Identifier
P_SetExprVarIdentifier (Expr e, Identifier i)
{
  char *name;

  if (e)
    {
      if (P_GetExprVarName (e))
	free (P_GetExprVarName (e));
      if ((name = P_GetIdentifierName (i)))
	P_SetExprVarName (e, strdup (name));
      else
	P_SetExprVarName (e, NULL);

      P_SetExprVarKey (e, P_GetIdentifierKey (i));
    }

  return (i);
}

/*! \brief Gets the identifier (name and key) from a variable in an ::Expr.
 *
 * \param e
 *  the ::Expr.
 *
 * \return
 *  A pointer to an ::Identifier struct for the ::Expr.
 *
 * \note This function allocates a new ::Identifier and returns a pointer.
 * It is the caller's responsibility to free the ::Identifier using
 * P_RemoveIdentifier().
 *
 * \sa P_RemoveIdentifier() */
Identifier
P_GetExprVarIdentifier (Expr e)
{
  Identifier result;
  char *name;

  if (e)
    {
      result = P_NewIdentifier ();

      P_SetIdentifierKey (result, P_GetExprVarKey (e));
      if ((name = P_GetExprVarName (e)))
	P_SetIdentifierName (result, strdup (name));

      return (result);
    }
  else
    P_punt ("struct.c:P_GetExprVarIdentifier:%d struct Expr (e) is NULL",
	    __LINE__ - 1);

  /* Should never reach this point. */
  return (NULL);
}

/* Pragma access functions. */
/*! \brief Appends a ::Pragma to a ::Pragma list.
 *
 * \param p
 *  the head of the list to append.
 * \param n
 *  the ::Pragma to append to the list.
 *
 * \return
 *  A pointer to the head of the list.
 *
 * Appends ::Pragma \a n to the tail of the list headed by \a p.  \a p may
 * be null, in which case \a n is returned as the head of the list.
 *
 * \note This function maintains a Pragma list.  To set the Pragma.next field
 *       explicitly, see #P_SetPragmaNext().
 */
Pragma
P_AppendPragmaNext (Pragma p, Pragma n)
{
  Pragma t = p;

  if (p)
    {
      while (t->next)
	t = t->next;

      t->next = n;
    }
  else
    p = n;

  return (p);
}

/*! \brief Deletes a ::Pragma from a ::Pragma list.
 *
 * \param p
 *  the head of the list to delete.
 * \param d
 *  the ::Pragma to delete from the list.
 *
 * \return
 *  A pointer to the head of the list.
 *
 * Deletes ::Pragma \a d from the list headed by \a p.  This function does
 * not free \a d.
 *
 * \note This function maintains a Pragma list.  To se the Pragma.next field
 *       explicitly, see #P_SetPragmaNext().
 */
Pragma
P_DeletePragmaNext (Pragma p, Pragma d)
{
  Pragma t = p, l = NULL;

  for (t = p; t; l = t, t = t->next)
    if (t == d)
      {
	if (l)
	  l->next = t->next;
	else
	  p = t->next;
	
	t->next = NULL;
	break;
      }

  return (p);
}

/*! \brief Sets the position of a ::Pragma.
 *
 * \param p
 *  the ::Pragma.
 * \param q
 *  the new position of the ::Pragma.
 *
 * \return
 *  The new position of the ::Pragma (\a q).
 *
 * \note The valus from the ::Position struct are copied, so the caller is
 * responsible for freeing the ::Position passed as \a p.
 */
Position
P_SetPragmaPosition (Pragma p, Position q)
{
  char *filename;

  if (p)
    {
      P_SetPragmaLineno (p, P_GetPositionLineno (q));
      P_SetPragmaColno (p, P_GetPositionColno (q));
      if ((filename = P_GetPositionFilename (q)))
	P_SetPragmaFilename (p, strdup (filename));
    }
  else
    {
      P_punt ("struct.c:P_SetPragmaPosition:%d struct Pragma (p) is NULL",
	      __LINE__ - 1);
    }

  return (q);
}

/*! \brief Gets the position of a ::Pragma.
 *
 * \param p
 *  the ::Pragma.
 *
 * \return
 *  A pointer to a ::Position struct for the ::Pragma.
 *
 * \note This function allocates a new ::Position and returns a pointer.  It
 * is the caller's responsibility to free the ::Position using
 * P_RemovePosition()
 *
 * \sa P_RemovePosition() */
Position
P_GetPragmaPosition (Pragma p)
{
  Position result = NULL;
  char *filename;

  if (p)
    {
      result = P_NewPosition ();

      P_SetPositionLineno (result, P_GetPragmaLineno (p));
      P_SetPositionColno (result, P_GetPragmaColno (p));
      if ((filename = P_GetPragmaFilename (p)))
	P_SetPragmaFilename (result, strdup (filename));
    }
  else
    {
      P_punt ("struct.c:P_GetPragmaPosition:%d struct Pragma (p) is NULL",
	      __LINE__ - 1);
    }

  return (result);
}

/* ProfCS access functions. */
/*! \brief Appends a ::ProfCS to a ::ProfCS list.
 *
 * \param p
 *  the head of the list to append.
 * \param n
 *  the ::ProfCS to append to the list.
 *
 * \return
 *  A pointer to the head of the list.
 *
 * Appends ::ProfCS \a n to the tail of the list headed by \a p.  \a p may
 * be null, in which case \a n is returned as the head of the list.
 *
 * \note This function maintains a ProfCS list.  To set the ProfCS.next
 *       field explicitly, see #P_SetProfCSNext().
 */
ProfCS
P_AppendProfCSNext (ProfCS p, ProfCS n)
{
  ProfCS t = p;

  if (p)
    {
      while (t->next)
	t = t->next;

      t->next = n;
    }
  else
    p = n;

  return (p);
}

/* ProfArc access functions. */
/*! \brief Appends a ::ProfArc to a ::ProfArc list.
 *
 * \param p
 *  the head of the list to append.
 * \param n
 *  the ::ProfArc to append to the list.
 *
 * \return
 *  A pointer to the head of the list.
 *
 * Appends ::ProfArc \a n to the tail of the list headed by \a p.  \a p may
 * be null, in which case \a n is returned as the head of the list.
 *
 * \note This function maintains a ProfArc list.  To set the ProfArc.next
 *       field explictly, see P_SetProfArcNext().
 */
ProfArc
P_AppendProfArcNext (ProfArc p, ProfArc n)
{
  ProfArc t = p;

  if (p)
    {
      while (t->next)
	t = t->next;

      t->next = n;
    }
  else
    p = n;

  return (p);
}

/* ProfST access functions. */
/*! \brief Appends a ::ProfST to a ::ProfST list.
 *
 * \param p
 *  the head of the list to append.
 * \param n
 *  the ::ProfST to append to the list.
 *
 * \return
 *  A pointer to the head of the list.
 *
 * Appends ::ProfST \a n to the tail of the list headed by \a p.  \a p may
 * be null, in which case \a n is returned as the head of the list.
 *
 * \note This function maintains a ProfST list.  To set the ProfST.next field
 *       explicitly, see #P_SetProfSTNext().
 */
ProfST
P_AppendProfSTNext (ProfST p, ProfST n)
{
  ProfST t = p;

  if (p)
    {
      while (t->next)
	t = t->next;

      t->next = n;
    }
  else
    p = n;

  return (p);
}

/* ProfEXPR access functions. */
/*! \brief Appends a ::ProfEXPR to a ::ProfEXPR list.
 *
 * \param p
 *  the head of the list to append.
 * \param n
 *  the ::ProfEXPR to append to the list.
 *
 * \return
 *  A pointer to the head of the list.
 *
 * Appends ::ProfEXPR \a n to the tail of the list headed by \a p.  \a s may
 * be null, in which case \a n is returned as the head of the list.
 *
 * \note This function maintains a ProfEXPR list.  To set the ProfEXPR.next
 *       field explicitly, see #P_SetProfEXPRNext().
 */
ProfEXPR
P_AppendProfEXPRNext (ProfEXPR p, ProfEXPR n)
{
  ProfEXPR t = p;

  if (p)
    {
      while (t->next)
	t = t->next;

      t->next = n;
    }
  else
    p = n;

  return (p);
}

/* Shadow access functions. */
/*! \brief Appends a ::Shadow to a ::Shadow list.
 *
 * \param s
 *  the head of the list to append.
 * \param n
 *  the ::Shadow to append to the list.
 *
 * \return
 *  A pointer to the head of the list.
 *
 * Appends ::Shadow \a n to the tail of the list headed by \a s.  \a s may
 * be null, in which case \a n is returned as the head of the list.
 *
 * \note This function maintains a Shadow list.  To set the Shadow.next
 *       field explicitly, see #P_SetShadowNext().
 */
Shadow
P_AppendShadowNext (Shadow s, Shadow n)
{
  Shadow t = s;

  if (s)
    {
      while (t->next)
	t = t->next;

      t->next = n;
    }
  else
    s = n;

  return (s);
}

/* AsmDcl access functions. */
/*! \brief Sets the postiion of an ::AsmStmt.
 *
 * \param a
 *  the ::AsmStmt.
 * \param p
 *  the new position of the ::AsmStmt.
 * 
 * \return
 *  The new position of the ::AsmStmt (\a p).
 *
 * \note The values from the ::Position struct are copied, so the caller is
 * responsible for freeing the ::Position passed as \a p.
 */
Position
P_SetAsmDclPosition (AsmDcl a, Position p)
{
  char *f;

  if (a)
    {
      P_SetAsmDclLineno (a, P_GetPositionLineno (p));
      P_SetAsmDclColno (a, P_GetPositionColno (p));
      if ((f = P_GetPositionFilename (p)))
	P_SetAsmDclFilename (a, strdup (f));
    }
  else
    P_punt ("struct.c:P_SetAsmDclPosition:%d struct AsmDcl (a) is NULL",
	    __LINE__ - 1);

  return (p);
}

/*! \brief Gets the position of an ::AsmStmt.
 *
 * \param a
 *  the ::AsmStmt.
 *
 * \return
 *  A pointer to a ::Position struct for the ::AsmStmt.
 *
 * \note This function allocates a new ::Position and returns a pointer.  It
 * is the caller's responsibility to free the ::Position using
 * P_RemovePosition().
 *
 * \sa P_RemovePosition() */
Position
P_GetAsmDclPosition (AsmDcl a)
{
  Position result;
  char *f;

  if (a)
    {
      result = P_NewPosition ();

      P_SetPositionLineno (result, P_GetAsmDclLineno (a));
      P_SetPositionColno (result, P_GetAsmDclColno (a));
      if ((f = P_GetAsmDclFilename (a)))
	P_SetPositionFilename (result, strdup (f));

      return (result);
    }
  else
    P_punt ("struct.c:P_GetAsmDclPosition:%d struct AsmDcl (a) is NULL",
	    __LINE__ - 1);

  /* Should never reach this point. */
  return (NULL);
}

/* Scope access functions. */
/*! \brief Appends key to the Scope.scope_entry list.
 *
 * \param s
 *  the ::Scope.
 * \param k
 *  the ::Key to append to the list.
 *
 * \return
 *  The ::Key to append to the list (\a k).
 */
Key
P_AppendScopeScopeEntry (Scope s, Key k)
{
  if (s)
    s->scope_entry = P_AppendScopeEntryNext (s->scope_entry,
					     P_NewScopeEntryWithKey (k));
  else
    P_punt ("struct.c:P_AppendScopeScopeEntry:%d struct Scope (s) is NULL",
	    __LINE__ - 1);

  return (k);
}

/* SymTabEntry access functions. */
/*! \brief Sets the identifier (key and name) of a ::SymTabEntry.
 *
 * \param s
 *  the ::SymTabEntry.
 * \param i
 *  the new identifier.
 *
 * \return
 *  The new identifier (\a i).
 *
 * \note The values from the ::Identifier struct are copied, so the caller is
 * responsible for freeing the ::Identifier passed as \a i.
 */
Identifier
P_SetSymTabEntryIdentifier (SymTabEntry s, Identifier i)
{
  char *name;

  if (s)
    {
      if (s->name)
	free (s->name);
      if ((name = P_GetIdentifierName (i)))
	P_SetSymTabEntryName (s, strdup (name));
      else
	P_SetSymTabEntryName (s, NULL);

      P_SetSymTabEntryKey (s, P_GetIdentifierKey (i));
    }

  return (i);
}

/*! \brief Gets the identifier (name and key) from a ::SymTabEntry.
 *
 * \param s
 *  the ::SymTabEntry.
 *
 * \return
 *  A pointer to an ::Identifier struct for the ::SymTabEntry.
 *
 * \note This function allocates a new ::Identifier and returns a pointer.
 * It is the caller's responsibility to free the ::Identifier using
 * P_RemoveIdentifier().
 *
 * \sa P_RemoveIdentifier() */
Identifier
P_GetSymTabEntryIdentifier (SymTabEntry s)
{
  Identifier result;
  char *name;

  if (s)
    {
      result = P_NewIdentifier ();

      P_SetIdentifierKey (result, P_GetSymTabEntryKey (s));
      if ((name = P_GetSymTabEntryName (s)))
	P_SetIdentifierName (result, strdup (name));

      return (result);
    }
  else
    P_punt ("struct.c:P_GetSymTabEntryIdentifier:%d struct SymTabEntry (s) is "
	    "NULL", __LINE__ - 1);

  /* Should never reach this point. */
  return (NULL);
}

/*! \brief Sets the fields field for the referenced StructDcl or UnionDcl.
 *
 * \param s
 *  the ::SymTabEntry.
 * \param f
 *  the new value of the struct or union's fields field.
 *
 * \return
 *  The new value of the struct or union's fields field (\a f).
 *
 * Sets the fields field for the StructDcl or UnionDcl referenced by \a s.
 * This function assumes \a s holds a struct or union, and punts if this
 * is not the case.
 */
Field
P_SetSymTabEntryFields (SymTabEntry s, Field f)
{
  _EntryType e = P_GetSymTabEntryType (s);

  if (e == ET_STRUCT)
    P_SetStructDclFields (P_GetSymTabEntryStructDcl (s), f);
  else if (e == ET_UNION)
    P_SetUnionDclFields (P_GetSymTabEntryUnionDcl (s), f);
  else
    P_punt ("struct.c:P_SetSymTabEntryFields:%d SymTabEntry must be ET_STRUCT "
	    "or ET_UNION\nnot 0x%x", __LINE__ - 1, e);

  return (f);
}

/*! \brief Gets the fields field for the referenced StructDcl or UnionDcl.
 *
 * \param s
 *  the ::SymTabEntry.
 *
 * \return
 *  The value of the struct or union's fields field (\a f).
 *
 * Gets the fields field for the StructDcl or UnionDcl referenced by \a s.
 * This function assumes \a s holds a struct or union, and punts if this
 * is not the case.
 */
Field
P_GetSymTabEntryFields (SymTabEntry s)
{
  _EntryType e = P_GetSymTabEntryType (s);
  Field f = NULL;

  if (e == ET_STRUCT)
    f = P_GetStructDclFields (P_GetSymTabEntryStructDcl (s));
  else if (e == ET_UNION)
    f = P_GetUnionDclFields (P_GetSymTabEntryUnionDcl (s));
  else
    P_punt ("struct.c:P_GetSymTabEntryFields:%d SymTabEntry must be ET_STRUCT "
	    "or ET_UNION\nnot 0x%x", __LINE__ - 1, e);

  return (f);
}

/* SymbolTable access functions. */
/*! \brief Sets the output file extension for the symbol table.
 *
 * \param s
 *  the SymbolTable.
 * \param o
 *  the new output file extension.
 *
 * \return
 *  The new output file extension (\a o)
 *
 * Sets the output file extension for the symbol table.  Sets up the output
 * file name for each file in the table.
 *
 * \note The character string passed as \a o is not copied, so it must not
 *       be freed by the caller.
 */
char *
P_SetSymbolTableOutExt (SymbolTable s, char *o)
{
  int i, base_len, len;

  /* Set the IP table's out name. */
  /* Allocate space for the symbol table's new filename.  This is the length
   * of the base name, extension, '.', and '\0'. */
  base_len = (int)((long)(strrchr (s->ip_table_name, '.') - s->ip_table_name));
  len = base_len + strlen (o) + 2;

  if (!(s->out_name = malloc (len)))
    P_punt ("struct.c:P_SetSymbolTableOutExt:%d Could not alloc out_name",
	    __LINE__ - 1);

  s->out_name[0] = '\0';
  strncat (s->out_name, s->ip_table_name, base_len);
  strcat (s->out_name, ".");
  strcat (s->out_name, o);

  /* Set the out name for each source file. */
  for (i = 1; i <= P_GetSymbolTableNumFiles (s); i++)
    {
      IPSymTabEnt file = s->ip_table[i];
      
      if (P_GetIPSymTabEntFileType (file) == FT_HEADER)
	continue;
      
      if (P_GetIPSymTabEntOutName (file) != NULL)
	{
	  free (P_GetIPSymTabEntOutName (file));
	  P_SetIPSymTabEntOutName (file, NULL);
	}

      base_len = (int)((long)(strrchr (P_GetIPSymTabEntInName (file), '.') - \
			      P_GetIPSymTabEntInName (file)));
      len = base_len + strlen (o) + 2;

      if (!(P_SetIPSymTabEntOutName (file, malloc (len))))
	P_punt ("struct.c:P_SetSymbolTableOutExt:%d Could not allocate "
		"out_name for file %d", __LINE__ - 1, i);
	  
      P_GetIPSymTabEntOutName (file)[0] = '\0';
      strncat (P_GetIPSymTabEntOutName (file), P_GetIPSymTabEntInName (file),
	       base_len);
      strcat (P_GetIPSymTabEntOutName (file), ".");
      strcat (P_GetIPSymTabEntOutName (file), o);
    }

  return (o);
}

/*! \brief Sets the order in which sources are inspected for a symbol.
 *
 * \param s
 *  the symbol table.
 * \param first, second, third
 *  the first, second, and third sources to inspect.
 *
 * Sets the order in which symbol table functions search sources for a
 * requested symbol.  The default order is:
 *
 * SO_MEM, SO_OUT, SO_IN
 *
 * This order attempts to return the newest version of a symbol by first
 * searching memory, then the output file (in case the symbol was modified
 * and flushed), then the input file.
 */
void
P_SetSymbolTableSearchOrder (SymbolTable s, _STSearchOrder first,
			     _STSearchOrder second, _STSearchOrder third)
{
  s->search_order[0] = first;
  s->search_order[1] = second;
  s->search_order[2] = third;

  return;
}

/*-------------------------------------------------------------*/
/* Functions to copy data structures                           */
/*-------------------------------------------------------------*/

/*! \brief Copies a List.
 *
 * \param l
 *  the List to copy.
 * \param copy_data
 *  A function to copy the data field.
 * 
 * \return A copy of the List.
 *
 * Copies a List.
 */
List
P_CopyList (List l, void *(*copy_data)(void *))
{
  List new = NULL;
  void *ptr;

  for (List_start (l), ptr = List_next (l); ptr; ptr = List_next (l))
    {
      if (copy_data)
	new = List_insert_last (new, copy_data (ptr));
      else
	new = List_insert_last (new, ptr);
    }

  return (new);
}

/*! \brief Copies a Dcl.
 *
 * \param d
 *  the Dcl to copy.
 *
 * \return A copy of the Dcl.
 *
 * Copies a Dcl.
 *
 * \note Modules should typically use PST_CopyDcl() instead of this function.
 *
 * \sa PST_CopyDcl()
 */
Dcl
P_CopyDcl (Dcl d)
{
  Dcl new = NULL;

  if (d)
    {
      new = P_NewDcl ();

      P_SetDclType (new, P_GetDclType (d));
      switch (P_GetDclType (d))
	{
	case TT_FUNC:
	  P_SetDclFuncDcl (new, P_CopyFuncDcl (P_GetDclFuncDcl (d)));
	  break;
	case TT_TYPE:
	  P_SetDclTypeDcl (new, P_CopyTypeDcl (P_GetDclTypeDcl (d)));
	  break;
	case TT_VAR:
	  P_SetDclVarDcl (new, P_CopyVarDcl (P_GetDclVarDcl (d)));
	  break;
	case TT_STRUCT:
	  P_SetDclStructDcl (new, P_CopyStructDcl (P_GetDclStructDcl (d)));
	  break;
	case TT_UNION:
	  P_SetDclUnionDcl (new, P_CopyUnionDcl (P_GetDclUnionDcl (d)));
	  break;
	case TT_ENUM:
	  P_SetDclEnumDcl (new, P_CopyEnumDcl (P_GetDclEnumDcl (d)));
	  break;
	case TT_ASM:
	  P_SetDclAsmDcl (new, P_CopyAsmDcl (P_GetDclAsmDcl (d)));
	  break;
	case TT_INCLUDE:
	  if (P_GetDclInclude (d))
	    P_SetDclInclude (new, strdup (P_GetDclInclude (d)));
	  break;
	case TT_SYMBOLTABLE:
	case TT_IPSYMTABENT:
	case TT_SYMTABENTRY:
	  P_warn ("struct.c:P_CopyDcl:%d trying to copy symbol table",
		  __LINE__ - 1);
	  break;
	default:
	  P_punt ("struct.c:P_CopyDcl:%d unknown Dcl type %d", __LINE__,
		  P_GetDclType (d));
	}
    }

  return (new);
}

/*! \brief Copies a FuncDcl.
 *
 * \param f
 *  the FuncDcl to copy.
 *
 * \return A copy of the FuncDcl.
 *
 * Copies a FuncDcl.
 *
 * It is the user's responsibility to copy the ext field.
 *
 * \note Modules should typically use PST_CopyFuncDcl() instead of this
 *       function.
 *
 * \sa PST_CopyFuncDcl()
 */
FuncDcl
P_CopyFuncDcl (FuncDcl f)
{
  FuncDcl new = NULL;
  CopyHandler copy;
  int i;

  if (f)
    {
      new = P_NewFuncDcl ();
      
      if (P_GetFuncDclName (f))
	{
	  if (P_GetFuncDclName (new))
	    free (P_GetFuncDclName (new));

	  P_SetFuncDclName (new, strdup (P_GetFuncDclName (f)));
	}

      P_SetFuncDclKey (new, P_GetFuncDclKey (f));
      P_SetFuncDclType (new, P_GetFuncDclType (f));
      P_SetFuncDclLineno (new, P_GetFuncDclLineno (f));
      P_SetFuncDclColno (new, P_GetFuncDclColno (f));
      P_SetFuncDclQualifier (new, P_GetFuncDclQualifier (f));
      if (P_GetFuncDclFilename (f))
	P_SetFuncDclFilename (new, strdup (P_GetFuncDclFilename (f)));

      P_SetFuncDclParam (new, P_CopyVarList (P_GetFuncDclParam (f)));
      P_SetFuncDclStmt (new, P_CopyStmt (P_GetFuncDclStmt (f)));
      P_SetFuncDclPragma (new, P_CopyPragma (P_GetFuncDclPragma (f)));
      P_SetFuncDclProfile (new, P_CopyProfFN (P_GetFuncDclProfile (f)));

#if 0
      new->par_loop = P_CopyStmt (f->par_loop);
      new->flow = P_CopyFuncFlow (f->flow);
      new->depend = P_CopyFuncDepend (f->depend);
      new->local = P_CopyVarList (f->local);
#endif

      P_SetFuncDclShadow (new, P_CopyShadow (P_GetFuncDclShadow (f)));
      P_SetFuncDclMaxExprID (new, P_GetFuncDclMaxExprID (f));

      if (Handlers[ES_FUNC])
	{
	  for (i = 0; i < NumExtensions[ES_FUNC]; i++)
	    {
	      if (P_GetFuncDclExtL (f, i) && \
		  (copy = Handlers[ES_FUNC][i].copy))
		{
		  if (P_GetFuncDclExtL (new, i))
		    {
		      FreeHandler ext_free = Handlers[ES_FUNC][i].free;
		      P_SetFuncDclExtL (new, i,
					(*ext_free) (P_GetFuncDclExtL (new,
								       i)));
		    }

		  P_SetFuncDclExtL (new, i, (*copy) (P_GetFuncDclExtL (f, i)));
		}
	    }
	}
    }

  return (new);
}

/*! \brief Copies a TypeDcl.
 *
 * \param t
 *  the TypeDcl to copy.
 *
 * \return A copy of the TypeDcl.
 *
 * Copies a TypeDcl.
 *
 * \note The reference count is not copied.  The reference count of the
 *       returned type is 0.
 * \note Modules should typically use PST_CopyTypeDcl() instead of this
 *       function.
 *
 * \sa PST_CopyTypeDcl()
 */
TypeDcl
P_CopyTypeDcl (TypeDcl t)
{
  TypeDcl new = NULL;
  CopyHandler copy;
  int i;
  _BasicType bt;

  if (t)
    {
      bt = P_GetTypeDclBasicType (t);

      new = P_NewTypeDcl ();

      P_SetTypeDclBasicType (new, bt);
      P_SetTypeDclQualifier (new, P_GetTypeDclQualifier (t));
      P_SetTypeDclKey (new, P_GetTypeDclKey (t));
      P_SetTypeDclType (new, P_GetTypeDclType (t));
      if (P_GetTypeDclName (t))
	P_SetTypeDclName (new, strdup (P_GetTypeDclName (t)));

      if (bt & BT_ARRAY)
	P_SetTypeDclArraySize (new,
			       P_CopyExprList (P_GetTypeDclArraySize (t)));
      if (bt & BT_FUNC)
	P_SetTypeDclParam (new, P_CopyParam (P_GetTypeDclParam (t)));

      P_SetTypeDclSize (new, P_GetTypeDclSize (t));
      P_SetTypeDclAlignment (new, P_GetTypeDclAlignment (t));
      P_SetTypeDclLineno (new, P_GetTypeDclLineno (t));
      P_SetTypeDclColno (new, P_GetTypeDclColno (t));
      if (P_GetTypeDclFilename (t))
	P_SetTypeDclFilename (new, strdup (P_GetTypeDclFilename (t)));
      P_SetTypeDclPragma (new, P_CopyPragma (P_GetTypeDclPragma (t)));

      if (Handlers[ES_TYPE])
	{
	  for (i = 0; i < NumExtensions[ES_TYPE]; i++)
	    {
	      if (P_GetTypeDclExtL (t, i) && \
		  (copy = Handlers[ES_TYPE][i].copy))
		{
		  if (P_GetTypeDclExtL (new, i))
		    {
		      FreeHandler ext_free = Handlers[ES_TYPE][i].free;
		      P_SetTypeDclExtL (new, i,
					(*ext_free) (P_GetTypeDclExtL (new,
								       i)));
		    }

		  P_SetTypeDclExtL (new, i, (*copy) (P_GetTypeDclExtL (t, i)));
		}
	    }
	}
    }

  return (new);
}

/*! \brief Copies a KeyList.
 *
 * \param l
 *  the KeyList to copy.
 *
 * \return A copy of the KeyList.
 *
 * Copies a single KeyList node.  Does not copy the next or last fields.
 */
KeyList
P_CopyKeyListNode (KeyList l)
{
  KeyList new = NULL;

  if (l)
    {
      new = P_NewKeyList ();

      new->key = l->key;
    }

  return (new);
}

/*! \brief Copies a KeyList.
 *
 * \param l
 *  the KeyList to copy.
 *
 * \return A copy of the KeyList.
 *
 * Copies a KeyList.
 *
 * All KeyLists in the list are copied.
 */
KeyList
P_CopyKeyList (KeyList l)
{
  KeyList new = NULL, l_nxt, new_nxt;

  if (l)
    {
      new = P_CopyKeyListNode (l);

      for (new_nxt = new, l_nxt = l->next; l_nxt;
	   new_nxt = new_nxt->next, l_nxt = l_nxt->next)
	new_nxt->next = P_CopyKeyListNode (l_nxt);

      /* Find the end of the list copy to set as new->last. */
      new->last = new_nxt;
    }

  return (new);
}

/*! \brief Copies a VarDcl.
 *
 * \param v
 *  the VarDcl to copy.
 *
 * \return A copy of the VarDcl.
 *
 * Copies a VarDcl.
 *
 * It is the user's responsibility to copy the ext field.
 *
 * \note Modules should typically use PST_CopyVarDcl() instead of this
 *       function.
 *
 * \sa PST_CopyVarDcl()
 */
VarDcl
P_CopyVarDcl (VarDcl v)
{
  VarDcl new = NULL;
  CopyHandler copy;
  int i;

  if (v)
    {
      new = P_NewVarDcl ();

      if (P_GetVarDclName (v))
	P_SetVarDclName (new, strdup (P_GetVarDclName (v)));
      P_SetVarDclKey (new, P_GetVarDclKey (v));
      P_SetVarDclType (new, P_GetVarDclType (v));
      P_SetVarDclInit (new, P_CopyInit (P_GetVarDclInit (v)));
      P_SetVarDclLineno (new, P_GetVarDclLineno (v));
      P_SetVarDclColno (new, P_GetVarDclColno (v));
      P_SetVarDclAlign (new, P_GetVarDclAlign (v));
      P_SetVarDclQualifier (new, P_GetVarDclQualifier (v));
      if (P_GetVarDclFilename (v))
	P_SetVarDclFilename (new, strdup (P_GetVarDclFilename (v)));
      P_SetVarDclPragma (new, P_CopyPragma (P_GetVarDclPragma (v)));

      if (Handlers[ES_VAR])
	{
	  for (i = 0; i < NumExtensions[ES_VAR]; i++)
	    {
	      if (P_GetVarDclExtL (v, i) && (copy = Handlers[ES_VAR][i].copy))
		{
		  if (P_GetVarDclExtL (new, i))
		    {
		      FreeHandler ext_free = Handlers[ES_VAR][i].free;
		      P_SetVarDclExtL (new, i,
				       (*ext_free) (P_GetVarDclExtL (new, i)));
		    }

		  P_SetVarDclExtL (new, i, (*copy) (P_GetVarDclExtL (v, i)));
		}
	    }
	}
    }

  return (new);
}

/*! \brief Copies an Init.
 *
 * \param i
 *  the Init to copy.
 *
 * \return A copy of the Init.
 *
 * Copies an Init.
 *
 * All Inits in the list are copied.
 */
Init
P_CopyInit (Init i)
{
  Init new = NULL;
  CopyHandler copy;
  int j;

  if (i)
    {
      new = P_NewInit ();

      new->expr = P_CopyExprList (i->expr);
      new->set = P_CopyInit (i->set);
      new->next = P_CopyInit (i->next);

      if (Handlers[ES_INIT])
	{
	  for (j = 0; j < NumExtensions[ES_INIT]; j++)
	    {
	      if (i->ext[j] && (copy = Handlers[ES_INIT][j].copy))
		{
		  if (new->ext[j])
		    {
		      FreeHandler ext_free = Handlers[ES_INIT][j].free;
		      new->ext[j] = (*ext_free) (new->ext[j]);
		    }

		  new->ext[j] = (*copy) (i->ext[j]);
		}
	    }
	}
    }

  return (new);
}

/*! \brief Copies a StructDcl.
 *
 * \param s
 *  the StructDcl to copy.
 *
 * \return A copy of the StructDcl.
 *
 * Copies a StructDcl.  The StructDcl.next and prev fields are set to
 * Invalid_Key.
 *
 * \note Modules should typically use PST_CopyStructDcl() instead of this
 *       function.
 *
 * \sa PST_CopyStructDcl()
 */
StructDcl
P_CopyStructDcl (StructDcl s)
{
  StructDcl new = NULL;
  CopyHandler copy;
  int i;

  if (s)
    {
      new = P_NewStructDcl ();

      if (s->name)
	new->name = strdup (s->name);
      new->key = s->key;
      new->qualifier = s->qualifier;
      new->fields = P_CopyField (s->fields);
      new->lineno = s->lineno;
      new->colno = s->colno;
      if (s->filename)
	new->filename = strdup (s->filename);
      new->size = s->size;
      new->align = s->align;
      new->group = s->group;
      new->pragma = P_CopyPragma (s->pragma);

      if (Handlers[ES_STRUCT])
	{
	  for (i = 0; i < NumExtensions[ES_STRUCT]; i++)
	    {
	      if (s->ext[i] && (copy = Handlers[ES_STRUCT][i].copy))
		{
		  if (new->ext[i])
		    {
		      FreeHandler ext_free = Handlers[ES_STRUCT][i].free;
		      new->ext[i] = (*ext_free) (new->ext[i]);
		    }

		  new->ext[i] = (*copy) (s->ext[i]);
		}
	    }
	}
    }

  return (new);
}

/*! \brief Copies a UnionDcl.
 *
 * \param u
 *  the UnionDcl to copy.
 *
 * \return A copy of the UnionDcl.
 *
 * Copies a UnionDcl.  The UnionDcl.next and prev fields are set to
 * Invalid_Key.
 *
 * \note Modules should typically use PST_CopyUnionDcl() instead of this
 *       function.
 *
 * \sa PST_CopyUnionDcl()
 */
UnionDcl
P_CopyUnionDcl (UnionDcl u)
{
  UnionDcl new = NULL;
  CopyHandler copy;
  int i;

  if (u)
    {
      new = P_NewUnionDcl ();

      if (u->name)
	new->name = strdup (u->name);
      new->key = u->key;
      new->qualifier = u->qualifier;
      new->fields = P_CopyField (u->fields);
      new->lineno = u->lineno;
      new->colno = u->colno;
      if (u->filename)
	new->filename = strdup (u->filename);
      new->size = u->size;
      new->align = u->align;
      new->group = u->group;
      new->pragma = P_CopyPragma (u->pragma);
      
      if (Handlers[ES_UNION])
	{
	  for (i = 0; i < NumExtensions[ES_UNION]; i++)
	    {
	      if (u->ext[i] && (copy = Handlers[ES_UNION][i].copy))
		{
		  if (new->ext[i])
		    {
		      FreeHandler ext_free = Handlers[ES_UNION][i].free;
		      new->ext[i] = (*ext_free) (new->ext[i]);
		    }

		  new->ext[i] = (*copy) (u->ext[i]);
		}
	    }
	}
    }

  return (new);
}

/*! \brief Copies a Field.
 *
 * \param f
 *  the Field to copy.
 *
 * \return A copy of the Field.
 *
 * Copies a Field.
 *
 * All Fields in the list are copied.
 *
 * \note Modules should typically use PST_CopyField() instead of this
 *       function.
 *
 * \sa PST_CopyField()
 */
Field
P_CopyField (Field f)
{
  Field new = NULL;
  CopyHandler copy;
  int i;

  if (f)
    {
      new = P_NewField ();

      if (f->name)
	new->name = strdup (f->name);
      new->key = f->key;
      new->parent_key = f->parent_key;
      new->type = f->type;
      new->is_bit_field = f->is_bit_field;
      new->bit_size = f->bit_size;
      new->bit_offset_remainder = f->bit_offset_remainder;
      new->next = P_CopyField (f->next);
      new->offset = f->offset;
      new->pragma = P_CopyPragma (f->pragma);

      if (Handlers[ES_FIELD])
	{
	  for (i = 0; i < NumExtensions[ES_FIELD]; i++)
	    {
	      if (f->ext[i] && (copy = Handlers[ES_FIELD][i].copy))
		{
		  if (new->ext[i])
		    {
		      FreeHandler ext_free = Handlers[ES_FIELD][i].free;
		      new->ext[i] = (*ext_free) (new->ext[i]);
		    }

		  new->ext[i] = (*copy) (f->ext[i]);
		}
	    }
	}
    }

  return (new);
}

/*! \brief Copies an EnumDcl.
 *
 * \param e
 *  the EnumDcl to copy.
 *
 * \return A copy of the EnumDcl.
 *
 * Copies an EnumDcl.
 *
 * \note Modules should typically use PST_CopyEnumDcl() instead of this
 *       function.
 *
 * \sa PST_CopyEnumDcl()
 */
EnumDcl
P_CopyEnumDcl (EnumDcl e)
{
  EnumDcl new = NULL;
  CopyHandler copy;
  int i;

  if (e)
    {
      new = P_NewEnumDcl ();

      if (e->name)
	new->name = strdup (e->name);
      new->key = e->key;
      new->fields = P_CopyEnumField (e->fields);
      new->lineno = e->lineno;
      new->colno = e->colno;
      if (e->filename)
	new->filename = strdup (e->filename);
      new->pragma = P_CopyPragma (e->pragma);

      if (Handlers[ES_ENUM])
	{
	  for (i = 0; i < NumExtensions[ES_ENUM]; i++)
	    {
	      if (e->ext[i] && (copy = Handlers[ES_ENUM][i].copy))
		{
		  if (new->ext[i])
		    {
		      FreeHandler ext_free = Handlers[ES_ENUM][i].free;
		      new->ext[i] = (*ext_free) (new->ext[i]);
		    }

		  new->ext[i] = (*copy) (e->ext[i]);
		}
	    }
	}
    }

  return (new);
}

/*! \brief Copies an EnumField.
 *
 * \param f
 *  the EnumField to copy.
 *
 * \return A copy of the EnumField.
 *
 * Copies an EnumField.
 *
 * All EnumFields in the list are copied.
 *
 * \note Modules should typically use PST_CopyEnumField() instead of this
 *       function.
 *
 * \sa PST_CopyEnumField()
 */
EnumField
P_CopyEnumField (EnumField f)
{
  EnumField new = NULL;

  if (f)
    {
      new = P_NewEnumField ();

      if (f->name)
	new->name = strdup (f->name);
      new->key = f->key;
      new->value = P_CopyExprList (f->value);
      new->next = P_CopyEnumField (f->next);
    }

  return (new);
}

/*! \brief Copies a single Stmt.
 *
 * \param s
 *  the Stmt to copy.
 *
 * \return A copy of the Stmt.
 *
 * Copies a single Stmt.
 *
 * Only the given Stmt is copied, not the entire list.
 *
 * ::Stmt.lex_prev, ::Stmt.lex_next, ::Stmt.parent, ::Stmt.parent_func,
 * and ::Stmt.parent_expr are set to null in the new Stmt.
 *
 * \note Modules should typically use PST_CopyStmtNode() instead of this
 *       function.
 *
 * \sa P_CopyStmt(), PST_CopyStmtNode()
 */
Stmt
P_CopyStmtNode (Stmt s)
{
  Stmt new = NULL;
  CopyHandler copy;
  int i;

  if (s)
    {
      new = P_NewStmt ();

      new->type = s->type;
      new->key = s->key;
      new->status = s->status;
      new->lineno = s->lineno;
      new->colno = s->colno;
      new->artificial = s->artificial;
      new->foroverlap = s->foroverlap;
      if (s->filename)
	new->filename = strdup (s->filename);
      new->profile = P_CopyProfST (s->profile);
      new->pragma = P_CopyPragma (s->pragma);

      /* Shadow is copied after the expression. */

      new->labels = P_CopyLabel (s->labels);
      if (new->labels)
	P_SetLabelParentStmtAll (new->labels, new);

      switch (s->type)
	{
	case ST_RETURN:
	  if ((new->stmtstruct.ret = P_CopyExprList (s->stmtstruct.ret)))
	    P_SetExprParentStmtAll (new->stmtstruct.ret, new);
	  break;
	case ST_GOTO:
	  if (s->stmtstruct.label.val)
	    new->stmtstruct.label.val = strdup (s->stmtstruct.label.val);
	  new->stmtstruct.label.key = s->stmtstruct.label.key;
	  break;
	case ST_COMPOUND:
	  new->stmtstruct.compound = P_CopyCompound (s->stmtstruct.compound);
	  P_SetCompoundParentStmtAll (new->stmtstruct.compound, new);
	  break;
	case ST_IF:
	  new->stmtstruct.ifstmt = P_CopyIfStmt (s->stmtstruct.ifstmt);
	  P_SetIfStmtParentStmtAll (new->stmtstruct.ifstmt, new);
	  break;
	case ST_SWITCH:
	  new->stmtstruct.switchstmt = \
	    P_CopySwitchStmt (s->stmtstruct.switchstmt);
	  P_SetSwitchStmtParentStmtAll (new->stmtstruct.switchstmt, new);
	  break;
	case ST_PSTMT:
	  new->stmtstruct.pstmt = P_CopyPstmt (s->stmtstruct.pstmt);
	  P_SetPstmtParentStmtAll (new->stmtstruct.pstmt, new);
	  break;
	case ST_ADVANCE:
	  new->stmtstruct.advance = P_CopyAdvance (s->stmtstruct.advance);
	  break;
	case ST_AWAIT:
	  new->stmtstruct.await = P_CopyAwait (s->stmtstruct.await);
	  break;
	case ST_MUTEX:
	  new->stmtstruct.mutex = P_CopyMutex (s->stmtstruct.mutex);
	  P_SetMutexParentStmtAll (new->stmtstruct.mutex, new);
	  break;
	case ST_COBEGIN:
	  new->stmtstruct.cobegin = P_CopyCobegin (s->stmtstruct.cobegin);
	  P_SetCobeginParentStmtAll (new->stmtstruct.cobegin, new);
	  break;
	case ST_PARLOOP:
	  new->stmtstruct.parloop = P_CopyParLoop (s->stmtstruct.parloop);
	  P_SetParLoopParentStmtAll (new->stmtstruct.parloop, new);
	  break;
	case ST_SERLOOP:
	  new->stmtstruct.serloop = P_CopySerLoop (s->stmtstruct.serloop);
	  P_SetSerLoopParentStmtAll (new->stmtstruct.serloop, new);
	  break;
	case ST_EXPR:
	  new->stmtstruct.expr = P_CopyExprList (s->stmtstruct.expr);
	  P_SetExprParentStmtAll (new->stmtstruct.expr, new);
	  break;
	case ST_BODY:
	  new->stmtstruct.bodystmt = P_CopyBodyStmt (s->stmtstruct.bodystmt);
	  P_SetBodyStmtParentStmtAll (new->stmtstruct.bodystmt, new);
	  break;
	case ST_EPILOGUE:
	  new->stmtstruct.epiloguestmt = \
	    P_CopyEpilogueStmt (s->stmtstruct.epiloguestmt);
	  P_SetEpilogueStmtParentStmtAll (new->stmtstruct.epiloguestmt, new);
	  break;
	case ST_ASM:
	  new->stmtstruct.asmstmt = P_CopyAsmStmt (s->stmtstruct.asmstmt);
	  P_SetAsmStmtParentStmtAll (new->stmtstruct.asmstmt, new);
	  break;
	default:
	  break;
	}

      /* Copy the shadow, if it exists. */
      if (s->shadow)
	new->shadow = P_NewShadowWithExprID (new->shadow, P_GetStmtExpr (new),
					     s->shadow->param_id);

#if 0
      new->flow = P_CopyStmtFlow (s->flow);
#endif

      if (Handlers[ES_STMT])
	{
	  for (i = 0; i < NumExtensions[ES_STMT]; i++)
	    {
	      if (s->ext[i] && (copy = Handlers[ES_STMT][i].copy))
		{
		  if (new->ext[i])
		    {
		      FreeHandler ext_free = Handlers[ES_STMT][i].free;
		      new->ext[i] = (*ext_free) (new->ext[i]);
		    }

		  new->ext[i] = (*copy) (s->ext[i]);
		}
	    }
	}
    }

  return (new);
}

/*! \brief Copies a Stmt.
 *
 * \param s
 *  the Stmt to copy.
 *
 * \return A copy of the Stmt.
 *
 * Copies a Stmt.
 *
 * The entire list (lex_next chain) is copied.
 *
 * ::Stmt.parent, ::Stmt.parent_func, and ::Stmt.parent_expr are set to null
 * in the new Stmt.
 *
 * \note Modules should typically use PST_CopyStmt() instead of this
 *       function.
 *
 * \sa P_CopyStmtNode(), PST_CopyStmt()
 */
Stmt
P_CopyStmt (Stmt s)
{
  Stmt new = NULL;

  if (s)
    {
      new = P_CopyStmtNode (s);
      new->lex_next = P_CopyStmt (s->lex_next);
      if (new->lex_next)
	new->lex_next->lex_prev = new;
    }

  return (new);
}

/*! \brief Copies a Label.
 *
 * \param l
 *  the Label to copy.
 *
 * \return A copy of the Label.
 *
 * Copies a Label.
 *
 * All Labels in the list are copied.  The parent pointer (for LB_LABEL labels)
 * is not copied.
 *
 * \note Modules should typically use PST_CopyLabel() instead of this
 *       function.
 *
 * \sa PST_CopyLabel()
 */
Label
P_CopyLabel (Label l)
{
  Label new = NULL;

  if (l)
    {
      new = P_NewLabel ();

      if (l->val)
	new->val = strdup (l->val);
      new->key = l->key;
      new->type = l->type;
      if (l->type == LB_CASE)
	new->data.expression = P_CopyExprList (l->data.expression);
      new->next = P_CopyLabel (l->next);
      if (new->next)
	new->next->prev = new;
    }

  return (new);
}

/*! \brief Copies a Compound.
 *
 * \param c
 *  the Compound to copy.
 *
 * \return A copy of the Compound.
 *
 * Copies a Compound.
 */
Compound
P_CopyCompound (Compound c)
{
  Compound new = NULL;

  if (c)
    {
      new = P_NewCompound ();
      new->type_list = P_CopyTypeList (c->type_list);
      new->var_list = P_CopyVarList (c->var_list);
      new->stmt_list = P_CopyStmt (c->stmt_list);
      new->unique_var_id = c->unique_var_id;
    }

  return (new);
}

/*! \brief Copies an IfStmt.
 *
 * \param i
 *  the IfStmt to copy.
 *
 * \return A copy of the IfStmt.
 *
 * Copies an IfStmt.
 */
IfStmt
P_CopyIfStmt (IfStmt i)
{
  IfStmt new = NULL;

  if (i)
    {
      new = P_NewIfStmt ();

      new->cond_expr = P_CopyExprList (i->cond_expr);
      new->then_block = P_CopyStmt (i->then_block);
      new->else_block = P_CopyStmt (i->else_block);
    }

  return (new);
}

/*! \brief Copies a SwitchStmt.
 *
 * \param s
 *  the SwitchStmt to copy.
 *
 * \return A copy of the SwitchStmt.
 *
 * Copies a SwitchStmt.
 */
SwitchStmt
P_CopySwitchStmt (SwitchStmt s)
{
  SwitchStmt new = NULL;

  if (s)
    {
      new = P_NewSwitchStmt ();

      new->expression = P_CopyExprList (s->expression);
      new->switchbody = P_CopyStmt (s->switchbody);
    }

  return (new);
}

/*! \brief Copies a Pstmt.
 *
 * \param p
 *  the Pstmt to copy.
 *
 * \return A copy of the Pstmt.
 *
 * Copies a Pstmt.
 *
 * It is the user's responsibility to copy the ext field.
 */
Pstmt
P_CopyPstmt (Pstmt p)
{
  Pstmt new = NULL;
  CopyHandler copy;
  int i;

  if (p)
    {
      new = P_NewPstmt ();

      new->stmt = P_CopyStmt (p->stmt);
      new->lineno = p->lineno;
      new->colno = p->colno;
      if (p->filename)
	new->filename = strdup (p->filename);

      if (Handlers[ES_PSTMT])
	{
	  for (i = 0; i < NumExtensions[ES_PSTMT]; i++)
	    {
	      if (p->ext[i] && (copy = Handlers[ES_PSTMT][i].copy))
		{
		  if (new->ext[i])
		    {
		      FreeHandler ext_free = Handlers[ES_PSTMT][i].free;
		      new->ext[i] = (*ext_free) (new->ext[i]);
		    }

		  new->ext[i] = (*copy) (p->ext[i]);
		}
	    }
	}
    }

  return (new);
}
      
/*! \brief Copies an Advance.
 *
 * \param a
 *  the Advance to copy.
 *
 * \return A copy of the Advance.
 *
 * Copies an Advance.
 */
Advance
P_CopyAdvance (Advance a)
{
  Advance new = NULL;

  if (a)
    {
      new = P_NewAdvance ();

      new->marker = a->marker;
    }

  return (new);
}

/*! \brief Copies an Await.
 *
 * \param a
 *  the Await to copy.
 *
 * \return A copy of the Await.
 *
 * Copies an Await.
 */
Await
P_CopyAwait (Await a)
{
  Await new = NULL;

  if (a)
    {
      new = P_NewAwait ();

      new->marker = a->marker;
      new->distance = a->distance;
    }

  return (new);
}

/*! \brief Copies a Mutex.
 *
 * \param m
 *  the Mutex to copy.
 *
 * \return A copy of the Mutex.
 *
 * Copies a Mutex.
 */
Mutex
P_CopyMutex (Mutex m)
{
  Mutex new = NULL;

  if (m)
    {
      new = P_NewMutex ();

      new->expression = P_CopyExprList (m->expression);
      new->statement = P_CopyStmt (m->statement);
    }

  return (new);
}

/*! \brief Copies a Cobegin.
 *
 * \param c
 *  the Cobegin to copy.
 *
 * \return A copy of the Cobegin.
 *
 * Copies a Cobegin.
 */
Cobegin
P_CopyCobegin (Cobegin c)
{
  Cobegin new = NULL;

  if (c)
    {
      new = P_NewCobegin ();

      new->statements = P_CopyStmt (c->statements);
    }

  return (new);
}

/*! \brief Copies a BodyStmt.
 *
 * \param b
 *  the BodyStmt to copy.
 *
 * \return A copy of the BodyStmt.
 *
 * Copies a BodyStmt.
 */
BodyStmt
P_CopyBodyStmt (BodyStmt b)
{
  BodyStmt new = NULL;

  if (b)
    {
      new = P_NewBodyStmt ();

      new->statement = P_CopyStmt (b->statement);
    }

  return (new);
}

/*! \brief Copies an EpilogueStmt.
 *
 * \param e
 *  the EpilogueStmt to copy.
 *
 * \return A copy of the EpilogueStmt.
 *
 * Copies an EpilogueStmt.
 */
EpilogueStmt
P_CopyEpilogueStmt (EpilogueStmt e)
{
  EpilogueStmt new = NULL;

  if (e)
    {
      new = P_NewEpilogueStmt ();

      new->statement = P_CopyStmt (e->statement);
    }

  return (new);
}

/*! \brief Copies a ParLoop.
 *
 * \param p
 *  the ParLoop to copy.
 *
 * \return A copy of the ParLoop.
 *
 * Copies a ParLoop.
 *
 * ::Parloop.parent in the new ParLoop is set to null.
 */
ParLoop
P_CopyParLoop (ParLoop p)
{
  ParLoop new = NULL;

  if (p)
    {
      new = P_NewParLoop ();

      new->loop_type = p->loop_type;
      new->pstmt = P_CopyPstmt (p->pstmt);
      new->iteration_var = P_CopyExprList (p->iteration_var);
      new->init_value = P_CopyExprList (p->init_value);
      new->final_value = P_CopyExprList (p->final_value);
      new->incr_value = P_CopyExprList (p->incr_value);
      new->child = P_CopyStmt (p->child);
      new->depth = p->depth;
    }

  return (new);
}

/*! \brief Copies a SerLoop.
 *
 * \param s
 *  the SerLoop to copy.
 *
 * \return A copy of the SerLoop.
 *
 * Copies a SerLoop.
 */
SerLoop
P_CopySerLoop (SerLoop s)
{
  SerLoop new = NULL;

  if (s)
    {
      new = P_NewSerLoop ();

      new->loop_type = s->loop_type;
      new->loop_body = P_CopyStmt (s->loop_body);
      new->cond_expr = P_CopyExprList (s->cond_expr);
      new->init_expr = P_CopyExprList (s->init_expr);
      new->iter_expr = P_CopyExprList (s->iter_expr);
    }

  return (new);
}

/*! \brief Copies an AsmStmt.
 *
 * \param a
 *  the AsmStmt to copy.
 *
 * \return A copy of the AsmStmt.
 *
 * Copies an AsmStmt.
 */
AsmStmt
P_CopyAsmStmt (AsmStmt a)
{
  AsmStmt new = NULL;

  if (a)
    {
      new = P_NewAsmStmt ();

      new->is_volatile = a->is_volatile;
      new->asm_clobbers = P_CopyExprList (a->asm_clobbers);
      new->asm_string = P_CopyExprList (a->asm_string);
      new->asm_operands = P_CopyExprList (a->asm_operands);
    }

  return (new);
}

/*! \brief Copies an Asmoprd.
 *
 * \param a
 *  the Asmoprd to copy.
 *
 * \return A copy of the Asmoprd.
 *
 * Copies an Asmoprd.
 */
Asmoprd
P_CopyAsmoprd (Asmoprd a)
{
  Asmoprd new = NULL;

  if (a)
    {
      new = P_NewAsmoprd ();

      new->modifiers = a->modifiers;
      if (a->constraints)
	new->constraints = strdup (a->constraints);
    }

  return (new);
}

/*! \brief Copies a single expr without operands, siblings, or the next list.
 *
 * \param e
 *  the Expr to copy.
 *
 * \return A copy of the Expr without operands, siblings, or the next list.
 *
 * \note Modules should typically use PST_CopyExprNode() instead of this
 *       function.
 *
 * \sa P_CopyExpr(), P_CopyExprList(), PST_CopyExprNode()
 */
Expr
P_CopyExprNode (Expr e)
{
  Expr new = NULL;
  Expr operand, sibling, next;

  if (e)
    {
      operand = P_GetExprOperands (e);
      sibling = P_GetExprSibling (e);
      next = P_GetExprNext (e);
      P_SetExprOperands (e, NULL);
      P_SetExprSibling (e, NULL);
      P_SetExprNext (e, NULL);

      new = P_CopyExpr (e);

      P_SetExprOperands (e, operand);
      P_SetExprSibling (e, sibling);
      P_SetExprNext (e, next);
    }

  return (new);
}

/*! \brief Copies an Expr.
 *
 * \param e
 *  the Expr to copy.
 *
 * \return A copy of the Expr.
 *
 * Copies an Expr.  Expr->next is not copied.
 *
 * ::Expr.bb_next is set to null in the new Expr.
 *
 * \note Modules should typically use PST_CopyExpr() instead of this function.
 *
 * \sa P_CopyExprNode(), P_CopyExprList(), PST_CopyExpr()
 */
Expr
P_CopyExpr (Expr e)
{
  Expr new = NULL;
  Expr *popd, opd;
  CopyHandler copy;
  int i;

  if (e)
    {
      new = P_NewExpr ();

      new->id = e->id;
      new->status = e->status;
      new->key = e->key;
      new->opcode = e->opcode;
      new->flags = e->flags;
      new->type = e->type;
      switch (e->opcode)
	{
	case OP_int:
	  if (P_GetExprFlags (e) & EF_UNSIGNED)
	    new->value.uscalar = e->value.uscalar;
	  else
	    new->value.scalar = e->value.scalar;
	  break;

	case OP_float:
	case OP_double:
	case OP_real:
	  new->value.real = e->value.real;
	  break;

	case OP_char:
	case OP_string:
	  if (e->value.string)
	    new->value.string = strdup (e->value.string);
	  break;

	case OP_dot:
	case OP_arrow:
	case OP_var:
	  if (e->value.var.name)
	    new->value.var.name = strdup (e->value.var.name);
	  new->value.var.key = e->value.var.key;
	  break;

	case OP_cast:
	  new->value.type = e->value.type;
	  break;

	case OP_stmt_expr:
	  P_SetExprStmt (new, P_CopyStmt (P_GetExprStmt (e)));
	  break;

	case OP_asm_oprd:
	  new->value.asmoprd = P_CopyAsmoprd (e->value.asmoprd);
	  break;
	default:
	  break;
	}

      popd = &new->operands;

      for (opd = e->operands; opd; opd = opd->sibling)
	{
	  Expr nopd;

	  *popd = nopd = P_CopyExprList (opd);
	  /* 03/10/04 REK Need to set the parentexpr for all exprs in the
	   *              list, not just nopd. */
	  P_SetExprParentExprAll (nopd, new);
	  popd = &nopd->sibling;
	}

      /* The next field isn't copied.  See P_CopyExprList(). */

      new->parentexpr = e->parentexpr;
      new->parentstmt = e->parentstmt;
      new->parentvar = e->parentvar;
      new->pragma = P_CopyPragma (e->pragma);
      new->profile = P_CopyProfEXPR (e->profile);

      if (Handlers[ES_EXPR])
	{
	  for (i = 0; i < NumExtensions[ES_EXPR]; i++)
	    {
	      if (e->ext[i] && (copy = Handlers[ES_EXPR][i].copy))
		{
		  if (new->ext[i])
		    {
		      FreeHandler ext_free = Handlers[ES_EXPR][i].free;
		      new->ext[i] = (*ext_free) (new->ext[i]);
		    }

		  new->ext[i] = (*copy) (e->ext[i]);
		}
	    }
	}
    }

  return (new);
}

/*! \brief Copies an Expr list.  This copies Expr->next.
 *
 * \param e
 *  the Expr to copy.
 *
 * \return
 *  A copy of \a e.
 *
 * The entire list is copied.
 *
 * \note Modules should typically use PST_CopyExprList() instead of this
 *       function.
 *
 * \sa P_CopyExprNode(), P_CopyExpr(), PST_CopyExprList()
 */
Expr
P_CopyExprList (Expr e)
{
  Expr new = NULL;

  if (e)
    {
      Expr nxt, nnxt, nprv;

      nprv = new = P_CopyExpr (e);

      for (nxt = e->next; nxt; nxt = nxt->next)
	{
	  nprv->next = nnxt = P_CopyExpr (nxt);
	  nnxt->previous = nprv;
	  nprv = nnxt;
	}
    }
  return (new);
}

/*! \brief Copies a single Pragma.
 *
 * \param p
 *  the Pragma to copy.
 *
 * \return A copy of the Pragma.
 *
 * Copies a single Pragma.
 *
 * Only the given Pragma is copied, not the entire list.
 *
 * \sa P_CopyPragma()
 */
Pragma
P_CopyPragmaNode (Pragma p)
{
  Pragma new = NULL;

  if (p)
    {
      new = P_NewPragma ();

      if (p->specifier)
	new->specifier = strdup (p->specifier);
      new->expr = P_CopyExprList (p->expr);
      new->lineno = p->lineno;
      new->colno = p->colno;
      if (p->filename)
	new->filename = strdup (p->filename);
#if 0
      new->dep_info = P_CopyDepInfo (p->dep_info);
#endif
    }

  return (new);
}

/*! \brief Copies a Pragma.
 *
 * \param p
 *  the Pragma to copy.
 *
 * \return A copy of the Pragma.
 *
 * Copies a Pragma.
 *
 * The entire list is copied.
 *
 * \sa P_CopyPragmaNode()
 */
Pragma
P_CopyPragma (Pragma p)
{
  Pragma new = NULL;

  if (p)
    {
      new = P_CopyPragmaNode (p);
      new->next = P_CopyPragma (p->next);
    }

  return (new);
}

/*! \brief Copies a Position.
 *
 * \param p
 *  the Position to copy.
 *
 * \return A copy of the Position.
 *
 * Copies a Position.
 */
Position
P_CopyPosition (Position p)
{
  Position new = NULL;

  if (p)
    {
      new = P_NewPosition ();

      new->lineno = p->lineno;
      new->colno = p->colno;
      if (p->filename)
	new->filename = strdup (p->filename);
    }

  return (new);
}

/*! \ brief Copies an Identifier.
 *
 * \param i
 *  the Identifier to copy.
 *
 * \return A copy of the Identifier.
 *
 * Copies an Identifier.
 */
Identifier
P_CopyIdentifier (Identifier i)
{
  Identifier new = NULL;

  if (i)
    {
      new = P_NewIdentifier ();

      if (i->name)
	new->name = strdup (i->name);
      new->key = i->key;
    }

  return (new);
}

/*! \brief Copies a ProfFN.
 *
 * \param p
 *  the ProfFN to copy.
 *
 * \return A copy of the ProfFN.
 *
 * Copies a ProfFN.
 */
ProfFN
P_CopyProfFN (ProfFN p)
{
  ProfFN new = NULL;

  if (p)
    {
      new = P_NewProfFN ();

      new->fn_id = p->fn_id;
      new->count = p->count;
      new->calls = P_CopyProfCS (p->calls);
    }

  return (new);
}

/*! \brief Copies a ProfCS.
 *
 * \param p
 *  the ProfCS to copy.
 *
 * \return A copy of the ProfCS.
 *
 * Copies a ProfCS.
 */
ProfCS
P_CopyProfCS (ProfCS p)
{
  ProfCS new = NULL;

  if (p)
    {
      new = P_NewProfCS ();

      new->call_site_id = p->call_site_id;
      new->callee_id = p->callee_id;
      new->weight = p->weight;
      new->next = P_CopyProfCS (p->next);
    }

  return (new);
}

/*! \brief Copies a ProfBB.
 *
 * \param p
 *  the ProfBB to copy.
 *
 * \return A copy of the ProfBB.
 *
 * Copies a ProfBB.
 */
ProfBB
P_CopyProfBB (ProfBB p)
{
  ProfBB new = NULL;

  if (p)
    {
      new = P_NewProfBB ();

      new->weight = p->weight;
      new->destination = P_CopyProfArc (p->destination);
    }

  return (new);
}

/*! \brief Copies a ProfArc.
 *
 * \param p
 *  the ProfArc to copy.
 *
 * \return A copy of the ProfArc.
 *
 * Copies a ProfArc.
 */
ProfArc
P_CopyProfArc (ProfArc p)
{
  ProfArc new = NULL;

  if (p)
    {
      new = P_NewProfArc ();

      new->bb_id = p->bb_id;
      new->condition = p->condition;
      new->weight = p->weight;
      new->next = P_CopyProfArc (p->next);
    }

  return (new);
}

/*! \brief Copies a ProfST.
 *
 * \param p
 *  the ProfST to copy.
 *
 * \return A copy of the ProfST.
 *
 * Copies a ProfST.
 */
ProfST
P_CopyProfST (ProfST p)
{
  ProfST new = NULL;

  if (p)
    {
      new = P_NewProfST ();

      new->count = p->count;
      new->next = P_CopyProfST (p->next);
    }

  return (new);
}

/*! \brief Copies a ProfEXPR.
 *
 * \param p
 *  the ProfEXPR to copy.
 *
 * \return A copy of the ProfEXPR.
 *
 * Copies a ProfEXPR.
 */
ProfEXPR
P_CopyProfEXPR (ProfEXPR p)
{
  ProfEXPR new = NULL;

  if (p)
    {
      new = P_NewProfEXPR ();
      
      new->count = p->count;
      new->next = P_CopyProfEXPR (p->next);
    }

  return (new);
}

/*! \brief Copies a Shadow.
 *
 * \param s
 *  the Shadow to copy.
 *
 * \return A copy of the Shadow.
 *
 * Copies a Shadow.
 *
 * \bug Shadows are tricky structures to copy.  The hold a pointer to
 *      an Expr in some other structure.  This other structure is
 *      responsible for maintaining this Expr, so the copied Shadow should
 *      point to the Expr copied by Expr's parent.  The right thing to do
 *      is probably have the Shadow's parent copy the Shadow and the
 *      Shadow's Expr simultaneously.
 */
Shadow
P_CopyShadow (Shadow s)
{
  Shadow new = NULL;

  if (s)
    {
      new = P_NewShadow ();

      new->param_id = s->param_id;
      new->expr = P_CopyExprList (s->expr);
      new->next = P_CopyShadow (s->next);
    }

  return (new);
}

/*! \brief Copies an AsmDcl.
 *
 * \param a
 *  the AsmDcl to copy.
 *
 * \return A copy of the AsmDcl.
 *
 * Copies an AsmDcl.
 *
 * It is the user's responsibility to copy the ext field.
 *
 * \note Modules should typically use PST_CopyAsmDcl() instead of this
 *       function.
 *
 * \sa P_CopyAsmDcl(), copy_asm_dcl(). */
AsmDcl
P_CopyAsmDcl (AsmDcl a)
{
  AsmDcl new = NULL;
  CopyHandler copy;
  int i;

  if (a)
    {
      new = P_NewAsmDcl ();

      new->is_volatile = a->is_volatile;
      new->asm_clobbers = P_CopyExprList (a->asm_clobbers);
      new->asm_string = P_CopyExprList (a->asm_string);
      new->asm_operands = P_CopyExprList (a->asm_operands);
      new->key = a->key;
      new->lineno = a->lineno;
      new->colno = a->colno;
      if (a->filename)
	new->filename = strdup (a->filename);

      if (Handlers[ES_ASM])
	{
	  for (i = 0; i < NumExtensions[ES_ASM]; i++)
	    {
	      if (a->ext[i] && (copy = Handlers[ES_ASM][i].copy))
		{
		  if (new->ext[i])
		    {
		      FreeHandler ext_free = Handlers[ES_ASM][i].free;
		      new->ext[i] = (*ext_free) (new->ext[i]);
		    }

		  new->ext[i] = (*copy) (a->ext[i]);
		}
	    }
	}
    }

  return (new);
}

/*! \brief Copies a Scope.
 *
 * \param s
 *  the Scope to copy.
 *
 * \return A copy of the Scope.
 */
Scope
P_CopyScope (Scope s)
{
  Scope new = NULL;

  if (s)
    {
      new = P_NewScope ();

      new->key = s->key;
      new->scope_entry = P_CopyScopeEntry (s->scope_entry);
    }

  return (new);
}

/*! \brief Copies a SymTabEntry.
 *
 * \param s
 *  the SymTabEntry to copy.
 *
 * \return A copy of the SymTabEntry.
 *
 * \note This function does not copy the SymTabEntry.entry pointer for fields,
 *       statements and expressions.  These structures are all contained in
 *       other Pcode structures and are copied with their parent structure.
 * \note This function does not copy the SymTabEntry.next field.
 */
SymTabEntry
P_CopySymTabEntry (SymTabEntry s)
{
  SymTabEntry new = NULL;
  CopyHandler copy;
  int i;

  if (s)
    {
      new = P_NewSymTabEntry ();

      new->key = s->key;
      if (s->name)
	new->name = strdup (s->name);
      new->scope_key = s->scope_key;
      new->type = s->type;

      /* Move the Pcode structure from the old entry to the new one. */
      switch (s->type)
	{
	case ET_FUNC:
	  new->entry.func_dcl = P_CopyFuncDcl (s->entry.func_dcl);
	  break;
	  
	case ET_TYPE_GLOBAL:
	  new->entry.type_dcl = P_CopyTypeDcl (s->entry.type_dcl);
	  break;
	  
	case ET_VAR_GLOBAL:
	  new->entry.var_dcl = P_CopyVarDcl (s->entry.var_dcl);
	  break;
	  
	case ET_STRUCT:
	  new->entry.struct_dcl = P_CopyStructDcl (s->entry.struct_dcl);
	  break;
	      
	case ET_UNION:
	  new->entry.union_dcl = P_CopyUnionDcl (s->entry.union_dcl);
	  break;
		  
	case ET_ENUM:
	  new->entry.enum_dcl = P_CopyEnumDcl (s->entry.enum_dcl);
	  break;
	  break;

	case ET_ASM:
	  new->entry.asm_dcl = P_CopyAsmDcl (s->entry.asm_dcl);
	  break;

	case ET_STMT:
	case ET_EXPR:
	case ET_FIELD:
	case ET_ENUMFIELD:
	case ET_TYPE_LOCAL:
	case ET_VAR_LOCAL:
	case ET_LABEL:
	case ET_SCOPE:
	case ET_BLOCK:
	  break;

	default:
	  P_punt ("struct.c:P_CopySymTabEntry:%d Unknown _EntryType %d",
		  __LINE__ - 1, s->type);
	}

      new->scope = P_CopyScope (s->scope);

      if (Handlers[ES_SYMTABENTRY])
	{
	  for (i = 0; i < NumExtensions[ES_SYMTABENTRY]; i++)
	    {
	      if (s->ext[i] && (copy = Handlers[ES_SYMTABENTRY][i].copy))
		{
		  if (new->ext[i])
		    {
		      FreeHandler ext_free = Handlers[ES_SYMTABENTRY][i].free;
		      new->ext[i] = (*ext_free) (new->ext[i]);
		    }

		  new->ext[i] = (*copy) (s->ext[i]);
		}
	    }
	}
    }

  return (new);
}

/*! \brief Shallow copies an IPSymTabEnt.
 *
 * \param i
 *  the IPSymTagEnt to copy.
 *
 * \return A shallow copy of the IPSymTabEnt.
 *
 * The file, in_file_status, and out_file_status fields are not copied.
 */
IPSymTabEnt
P_ShCopyIPSymTabEnt (IPSymTabEnt i)
{
  IPSymTabEnt new = NULL;
  CopyHandler copy;
  int j;

  if (i)
    {
      new = P_NewIPSymTabEnt();

      if (i->source_name)
	new->source_name = strdup (i->source_name);
      if (i->in_name)
	new->in_name = strdup (i->in_name);
      if (i->out_name)
	new->out_name = strdup (i->out_name);
      new->file_type = i->file_type;
      new->key = i->key;
      new->num_entries = i->num_entries;
      new->offset = i->offset;
      new->flags = i->flags;
      new->pragma = P_CopyPragma (i->pragma);

      if (Handlers[ES_IPSYMTABENT])
	{
	  for (j = 0; j < NumExtensions[ES_IPSYMTABENT]; j++)
	    {
	      if (i->ext[j] && (copy = Handlers[ES_IPSYMTABENT][j].copy))
		{
		  if (new->ext[j])
		    {
		      FreeHandler ext_free = Handlers[ES_IPSYMTABENT][j].free;
		      new->ext[j] = (*ext_free) (new->ext[j]);
		    }

		  new->ext[j] = (*copy) (i->ext[j]);
		}
	    }
	}
    }

  return (new);
}

/*! \brief Copies a SearchOrder
 *
 * \param dest
 *  the destination SearchOrder.
 * \param src
 *  the source SearchOrder.
 *
 * Copies the contents of \a src to \a dest.
 */
void
P_CopySearchOrder (SearchOrder dest, SearchOrder src)
{
  int i;

  for (i = 0; i < SO_NUM_SOURCES; i++)
    dest[i] = src[i];

  return;
}

/*! \brief Shallow copies a SymbolTable.
 *
 * \param s
 *  the SymbolTable to copy.
 *
 * \return A shallow copy of the SymbolTable.
 *
 * The file, in_file_status, and out_file_status fields are not copied.
 * The ip_table array field is allocated, but the contents are not copied.
 */
SymbolTable
P_ShCopySymbolTable (SymbolTable s)
{
  SymbolTable new = NULL;
  int i;

  if (s)
    {
      new = P_NewSymbolTable ();

      if (s->ip_table_name)
	new->ip_table_name = strdup (s->ip_table_name);
      if (s->out_name)
	new->out_name = strdup (s->out_name);
      new->modifiable_file = s->modifiable_file;
      new->num_files = s->num_files;

      /* Allocate the IPSymTabEnt array, but don't copy any of the
       * IPSymTabEnts. */
      new->ip_table = malloc (sizeof (IPSymTabEnt) * (s->num_files + 1));
      for (i = 0; i <= s->num_files; i++)
	new->ip_table[i] = NULL;

      for (i = 0; i < 3; i++)
	new->search_order[i] = s->search_order[i];
      new->flags = s->flags;
    }

  return (new);
}

P_memdep_core_t 
P_new_memdep_core()
{
  P_memdep_core_t core;
  core = calloc(1,sizeof(_P_memdep_core_t));
  return core;
}

void
P_free_memdep_core(P_memdep_core_t dep)
{
  free(dep);
}

Extension 
P_alloc_memdep(void)
{
  P_memdep_t md;
  md = (P_memdep_t)calloc(1,sizeof(_P_memdep_t));
  assert(md);
  return (Extension)md;
}

Extension 
P_free_memdep(Extension e)
{
  P_memdep_t md = (P_memdep_t)(e);
  if (md)
    {
      P_memdep_core_t dep;
      List_start(md->deps);
      while ((dep = (P_memdep_core_t)List_next(md->deps)))
	{
	  P_free_memdep_core(dep);
	}
      free(md);
    }
  return NULL;
}

char *
P_write_memdep(char *sig, Extension e)
{
  int size, o;
  char *buffer;
  P_memdep_core_t dep;

  P_memdep_t md = (P_memdep_t)(e);

  size = (List_size(md->deps) + 1) * (6+6+1+6+6);
  buffer = malloc(size);

  o = 0;
  List_start(md->deps);
  while ((dep = (P_memdep_core_t)List_next(md->deps)))
    {
      char c;
      if (dep->is_def)
	c = 'D';
      else
	c = 'U';

      o += sprintf(buffer + o, "%c%d-%d.%d,%d ", 
		   c, dep->id, dep->version, dep->offset, dep->size);
      if(o >= size) {
	printf("I'm a lazy programmer, who didn't allocate enough space for this\n"
	       "buffer, and instead of gracefully recovering, I just asserted!\n");
	assert(0);
      }
    }
  buffer[o] = 0;

  return buffer;
}

void 
P_read_memdep(Extension e, char *sig, char *raw)
{
  P_memdep_t md = (P_memdep_t)(e);
  int id, version, offset, size;
  char c;

  assert(md);
  while (sscanf(raw, "%c%d-%d.%d,%d", &c, &id, &version, &offset, &size) == 5)
    {
      P_memdep_core_t dep;
  
      dep = P_new_memdep_core();
      dep->id = id;
      dep->version = version;
      dep->offset = offset;
      dep->size = size;
      if (c == 'D')
	dep->is_def = 1;
      else if (c == 'U')
	dep->is_def = 0;
      else
	assert(0);

      md->deps = List_insert_last(md->deps, dep);
      
      /* Get to and past next blanks */
      raw = strchr(raw, ' ');
      while (*raw == ' ') 
	raw++;
    }
}

Extension 
P_copy_memdep (Extension orig_e)
{
  P_memdep_t orig_md;
  P_memdep_core_t orig_dep;
  P_memdep_t new_md;
  Extension new_e;

  orig_md = (P_memdep_t)(orig_e);
  new_e = P_alloc_memdep();
  new_md = (P_memdep_t)(new_e);

  assert(orig_md);
  List_start(orig_md->deps);
  while ((orig_dep = (P_memdep_core_t)List_next(orig_md->deps)))
    {
      P_memdep_core_t new_dep;

      new_dep = P_new_memdep_core();
      *new_dep = *orig_dep;

      new_md->deps = List_insert_last(new_md->deps, new_dep);
    }
  
  return new_e;
}

int
P_add_expr_memdep(Expr e, P_memdep_core_t dep)
{
  P_memdep_core_t cur;
  P_memdep_t md;

  assert(dep->id > 0);
  md = P_GetMemDep(e);
  assert(md);

  List_start(md->deps);
  while ((cur = (P_memdep_core_t)List_next(md->deps)))
    {
      if (dep->id == cur->id &&
	  dep->offset == cur->offset &&
          dep->is_def == cur->is_def &&
          dep->version == cur->version)
        {
	  return 0;
        }
    }

  cur = P_new_memdep_core();
  *cur = *dep;
  md->deps = List_insert_last(md->deps, cur);
  return 1;
}

/*! \brief Applies a function to nested Stmts and Exprs.
 *
 * \param i
 *  the Init to process.
 * \param fs
 *  the function to apply to statements.
 * \param fe
 *  the function to apply to expressions.
 * \param pre_or_post
 *  P_PREORDER or P_POSTORDER.  Determines whether this function works in
 *  preorder or postorder.
 * \param data
 *  user defined data that is passed as the second argument to \a fs and \a fe.
 *
 * Applies a function to nested Exprs in an initializer.
 */
void
P_InitApplyFunc (Init i, void (*fs)(Stmt, void *), void (*fe)(Expr, void *),
		 int pre_or_post, void *data)
{
  if (i)
    {
      if (i->expr)
	P_ExprApplyFunc (i->expr, fs, fe, pre_or_post, data);
      if (i->set)
	P_InitApplyFunc (i->set, fs, fe, pre_or_post, data);
      if (i->next)
	P_InitApplyFunc (i->next, fs, fe, pre_or_post, data);
    }

  return;
}

/*! \brief Applies a function to nested Stmts and Exprs.
 *
 * \param e
 *  the Expr to process.
 * \param fs
 *  the function to apply to statements.
 * \param fe
 *  the function to apply to expressions.
 * \param pre_or_post
 *  P_PREORDER or P_POSTORDER.  Determines whether this function works in
 *  preorder or postorder.
 * \param data
 *  user defined data that is passed as the second argument to \a fs and \a fe.
 *
 * Applies a function to nested Exprs.
 */
void
P_ExprApplyFunc (Expr e, void (*fs)(Stmt, void *), void (*fe)(Expr, void *),
		 int pre_or_post, void *data)
{
  while (e)
    {
      if (fe && pre_or_post == P_PREORDER)
	fe (e, data);

      if (e->next)
	P_ExprApplyFunc (e->next, fs, fe, pre_or_post, data);
      if (e->operands)
	P_ExprApplyFunc (e->operands, fs, fe, pre_or_post, data);

      if (e->opcode == OP_stmt_expr)
	P_StmtApplyFunc (e->value.stmt, fs, fe, pre_or_post, data);

      if (fe && pre_or_post == P_POSTORDER)
	fe (e, data);

      e = e->sibling;
    }

  return;
}

/*! \brief Applies a function to nested Stmts and Exprs.
 *
 * \param s
 *  the Stmt tree to process.
 * \param fs
 *  the function to apply to statements.
 * \param fe
 *  the function to apply to expressions.
 * \param pre_or_post
 *  P_PREORDER or P_POSTORDER.  Determines whether this function works in
 *  preorder or postorder.
 * \param data
 *  user defined data that is passed as the second argument to \a fs and \a fe.
 *
 * Applies a function to nested Stmts, incl. lex_next.
 */
void
P_StmtApplyFunc (Stmt s, void (*fs)(Stmt, void *), void (*fe)(Expr, void *),
		 int pre_or_post, void *data)
{
  Stmt ns;

  while (s)
    {
      ns = s->lex_next;

      if (fs && pre_or_post == P_PREORDER)
	fs (s, data);

      switch (s->type) 
	{
	case ST_RETURN:
	  P_ExprApplyFunc (s->stmtstruct.ret, fs, fe, pre_or_post, data);
	  break;
	case ST_COMPOUND:
	  {
	    Compound c = P_GetStmtCompound (s);
	    VarList vl = P_GetCompoundVarList (c);
	    VarDcl v;
	    Init i;

	    List_start (vl);
	    while ((v = (VarDcl)List_next (vl)))
	      if ((i = P_GetVarDclInit (v)))
		P_InitApplyFunc (i, fs, fe, pre_or_post, data);
	    	    
	    P_StmtApplyFunc (P_GetCompoundStmtList (c), fs, fe, pre_or_post,
			     data);
	  }
	  break;
	case ST_IF:
	  P_ExprApplyFunc (s->stmtstruct.ifstmt->cond_expr, fs, fe,
			   pre_or_post, data);
	  P_StmtApplyFunc (s->stmtstruct.ifstmt->then_block, fs, fe,
			   pre_or_post, data);
	  if (s->stmtstruct.ifstmt->else_block)
	    P_StmtApplyFunc (s->stmtstruct.ifstmt->else_block, fs, fe,
			     pre_or_post, data);
	  break;
	case ST_SWITCH:
	  P_ExprApplyFunc (s->stmtstruct.switchstmt->expression, fs, fe,
			   pre_or_post, data);
	  P_StmtApplyFunc (s->stmtstruct.switchstmt->switchbody, fs, fe,
			   pre_or_post, data);
	  break;
	case ST_PSTMT:
	  P_StmtApplyFunc (s->stmtstruct.pstmt->stmt, fs, fe, pre_or_post,
			   data);
	  break;
	case ST_MUTEX:
	  P_ExprApplyFunc (s->stmtstruct.mutex->expression, fs, fe,
			   pre_or_post, data);
	  P_StmtApplyFunc (s->stmtstruct.mutex->statement, fs, fe, pre_or_post,
			   data);
	  break;
	case ST_COBEGIN:
	  P_StmtApplyFunc (s->stmtstruct.cobegin->statements, fs, fe,
			   pre_or_post, data);
	  break;
	case ST_PARLOOP:
	  P_ExprApplyFunc (s->stmtstruct.parloop->init_value, fs, fe,
			   pre_or_post, data);
	  P_ExprApplyFunc (s->stmtstruct.parloop->final_value, fs, fe,
			   pre_or_post, data);
	  P_ExprApplyFunc (s->stmtstruct.parloop->incr_value, fs, fe,
			   pre_or_post, data);
	  P_StmtApplyFunc (Parloop_Stmts_Prologue_Stmt (s), fs, fe,
			   pre_or_post, data);
	  break;
	case ST_SERLOOP:
	  P_ExprApplyFunc (s->stmtstruct.serloop->cond_expr, fs, fe,
			   pre_or_post, data);
	  P_ExprApplyFunc (s->stmtstruct.serloop->init_expr, fs, fe,
			   pre_or_post, data);
	  P_ExprApplyFunc (s->stmtstruct.serloop->iter_expr, fs, fe,
			   pre_or_post, data);
	  P_StmtApplyFunc (s->stmtstruct.serloop->loop_body, fs, fe,
			   pre_or_post, data);
	  break;
	case ST_BODY:
	  P_StmtApplyFunc (s->stmtstruct.bodystmt->statement, fs, fe,
			   pre_or_post, data);
	  break;
	case ST_EPILOGUE:
	  P_StmtApplyFunc (s->stmtstruct.epiloguestmt->statement, fs, fe,
			   pre_or_post, data);
	  break;
	case ST_EXPR:
	  P_ExprApplyFunc (s->stmtstruct.expr, fs, fe, pre_or_post, data);
	  break;
	case ST_ASM:
	  P_ExprApplyFunc (s->stmtstruct.asmstmt->asm_clobbers, fs, fe,
			   pre_or_post, data);
	  P_ExprApplyFunc (s->stmtstruct.asmstmt->asm_string, fs, fe,
			   pre_or_post, data);
	  P_ExprApplyFunc (s->stmtstruct.asmstmt->asm_operands, fs, fe,
			   pre_or_post, data);
	  break;
#if 0
	  /* For debug */
	case ST_NOOP:
	case ST_CONT:
	case ST_BREAK:
	case ST_GOTO:
	case ST_ADVANCE:
	case ST_AWAIT:
	  break;
	default:
	  P_punt ("P_StmtApply: unhandled _StmtType");
#else
	default:
	  break;
#endif
	}

      if (fs && pre_or_post == P_POSTORDER)
	fs (s, data);

      s = ns;
    }

  return;
}
