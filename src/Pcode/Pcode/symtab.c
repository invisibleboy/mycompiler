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
 * \brief Routines to manage the Pcode symbol table.
 *
 * \author Robert Kidd, Wen-mei Hwu
 *
 * This file contains definitions for routines to manage the Pcode
 * symbol table.
 *
 * The symbol table is a sparse array implemented as a doubly linked
 * list of chunks of a fixed size.
 */
/*****************************************************************************/

#include <config.h>
#include <stdio.h>
#include <sys/file.h>
#include <sys/types.h>
#include <unistd.h>
#include <library/i_list.h>
#include <library/block_sparse_array.h>
#include "pcode.h"
#include "struct.h"
#include "symtab.h"
#include "struct_symtab.h"
#include "write.h"
#include "write_symtab.h"
#include "read.h"
#include "read_symtab.h"
#include "io_util.h"
#include "query.h"
#include "query_symtab.h"

static void link_symbols_stmt (Stmt s, void *data);
static void link_symbols_expr (Expr e, void *data);
static bool must_read (SymTabEntry e);
static Key get_file_entry (SymbolTable table, int file_key);
static Key get_file_entry_next (SymbolTable table, Key last);
static Key get_table_entry (SymbolTable table);
static Key get_table_entry_next (SymbolTable table, Key last);
static KeyList order_types (SymbolTable table, Key key, KeyList list,
			    KeyList parent);
static KeyList find_parent (SymbolTable table, KeyList list, Type type);
static bool can_break_recursion (SymbolTable table, Type type);
static void tag_entry_for_flush (SymbolTable table, Key key);
static void tag_stmt_for_flush (SymbolTable table, Stmt stmt);
static void tag_expr_for_flush (SymbolTable table, Expr expr);
static void tag_init_for_flush (SymbolTable table, Init init);

/*! \brief Opens a symbol table.
 *
 * \param in_file
 *  the name of the file to open.
 * \param out_file
 *  the name of the output file.
 * \param options
 *  symbol table options.
 *
 * \return
 *  The symbol table, or NULL if the table can't be opened.
 *
 * Opens \a in_file and reads a symbol table.  The key of the source file that
 * can be modified is returned in \a table.modifiable_file.  If any source
 * file can be modified (\a filename is an interprocedural symbol table),
 * \a table.modifiable_file is set to 0.
 *
 * \a in_file corresponds to the Pcode -i argument, and \a out_file to -o.
 * If \a in_file is a single source file, \a out_file will be assumed
 * to be the complete output filename.  If \a in_file is an interprocedural
 * symbol table, \a out_file is assumed to be the output file extension.
 *
 * If \a in_file can't be opened, returns NULL.
 *
 * \sa PST_Close()
 */
SymbolTable
PST_Open (char *in_file, char *out_file, _STFlags options)
{
  FILE *in;
  SymbolTable table = NULL;
  int file_key;

  if ((in = P_file_open_r (in_file)))
    {
      table = PST_ReadFile (in, &file_key);

      P_SetSymbolTableFlags (table, options);

      if (file_key == 0)
	{
	  P_SetSymbolTableIPTableName (table, strdup (in_file));
	  
	  P_SetSymbolTableFile (table, in);
	  P_SetSymbolTableInFileStatus (table, FS_READ_PERM);
	  
	  /* Set up the output filenames. */
	  if (out_file && strcmp (out_file, "stdout") != 0)
	    P_SetSymbolTableOutExt (table, out_file);
	}
      else
	{
	  PST_SetFileFile (table, file_key, in);
	  PST_SetFileInFileStatus (table, file_key, FS_READ_PERM);
	  if (out_file && strcmp (out_file, "stdout") != 0)
	    PST_SetFileOutName (table, file_key, strdup (out_file));
	}
    }

  return (table);
}

/*! \brief Closes a symbol table.
 *
 * \param table
 *  the symbol table.
 *
 * Closes the symbol table.  If the table option STF_READ_ONLY is not set,
 * writes all files.  This function frees \a table.
 *
 * \note This function uses PST_GetSymTabEntry() to get symbols to write.
 *       It resets the search order to the default so that the latest
 *       version of the symbol is written.
 *
 * \sa PST_Open()
 */
void
PST_Close (SymbolTable table)
{
  int file_key = P_GetSymbolTableModifiableFile (table);
  int i;

  /* Set the search order so the latest version of a symbol is written. */
  PST_SetSearchOrder (table, SO_MEM, SO_OUT, SO_IN);

#if 0
  if (!(P_GetSymbolTableFlags (table) & STF_READ_ONLY))
    PST_RotateFile (table);
#endif
      
  if (file_key == 0)
    {
      if (!(P_GetSymbolTableFlags (table) & STF_READ_ONLY))
	PST_WriteFile (table, 0);

      for (i = 1; i <= P_GetSymbolTableNumFiles (table); i++)
	{
	  if (PST_GetFileType (table, i) == FT_SOURCE)
	    {
	      if (!(P_GetSymbolTableFlags (table) & STF_READ_ONLY))
		PST_WriteFile (table, i);

	      if (PST_GetFileInFileStatus (table, i) == FS_READ_PERM)
		{
		  P_file_close (PST_GetFileFile (table, i));
		  PST_SetFileFile (table, i, NULL);
		  PST_SetFileInFileStatus (table, i, FS_CLOSED);
		}
	    }
	}

      if (P_GetSymbolTableInFileStatus (table) == FS_READ_PERM)
	{
	  P_file_close (P_GetSymbolTableFile (table));
	  P_SetSymbolTableFile (table, NULL);
	  P_SetSymbolTableInFileStatus (table, FS_CLOSED);
	}
    }
  else
    {
      if (!(P_GetSymbolTableFlags (table) & STF_READ_ONLY))
	PST_WriteFile (table, file_key);

      if (PST_GetFileInFileStatus (table, file_key) == FS_READ_PERM)
	{
	  P_file_close (PST_GetFileFile (table, file_key));
	  PST_SetFileFile (table, file_key, NULL);
	  PST_SetFileInFileStatus (table, file_key, FS_CLOSED);
	}
    }

  table = PST_RemoveSymbolTable (table);

  return;
}

/*! \brief Flushes a file from the symbol table.
 *
 * \param table
 *  the symbol table.
 * \param file
 *  the file's key.
 * \param write
 *  If TRUE, writes the file to disk before flushing.
 *
 * \sa PST_FlushEntry()
 *
 * Flushes file \a file from memory.  If \a write is true, writes the file
 * to disk first.
 *
 * This function writes all symbols in memory to the output file.  If a
 * symbol doesn't exist in memory, it is read from the output file, or
 * input file if it doesn't exist in output.
 */
void
PST_FlushFile (SymbolTable table, int file, bool write)
{
  Key key;
  int num_entries;

  if (write)
    {
      for (key = PST_GetFileEntryByType (table, file, ET_ANY);
	   P_ValidKey (key);
	   key = PST_GetFileEntryByTypeNext (table, key, ET_ANY))
	tag_entry_for_flush (table, key);

      PST_UpdateFile (table, file);
    }

  /* Put the file into its initial state.  Since the initial state for
   * the IP symbol table and headers is to have all symbols in memory,
   * this only makes sense for source files. */
  if (PST_GetFileType (table, file) == FT_SOURCE)
    {
      /* The repeated calls to PST_RemoveEntry() will reduce num_entries to 0.
       * We need a valid num_entries after exiting this function, so preserve
       * it here and restore it before exiting.  This preserves the range of
       * valid keys.  If a key is added after the flush, it is added outside
       * the range of previously flushed keys, avoiding a collision. */
      num_entries = PST_GetFileNumEntries (table, file);

      /* We'll remove the entry inside the loop, so we can just repeatedly
       * get the first entry in the file.
       *
       * Note: P_ValidKey() is a macro that evaluates its argument twice,
       * so its argument should not be a function call. */
      while (key = PST_GetFileEntryByType (table, file, ET_ANY),
	     P_ValidKey (key))
	PST_RemoveEntry (table, key);

      FreeBlockSparseArray (PST_GetFileTable (table, file));
      PST_SetFileTable (table, file, NULL);

      if (PST_GetFileInFileStatus (table, file) == FS_READ_PERM)
	{
	  P_file_close (PST_GetFileFile (table, file));
	  PST_SetFileFile (table, file, NULL);
	  PST_SetFileInFileStatus (table, file, FS_CLOSED);
	}

      /* Restore the number of entries in this file. */
      PST_SetFileNumEntries (table, file, num_entries);
    }

  return;
}

/*! \brief Flushes an entry from the symbol table.
 *
 * \param table
 *  the symbol table
 * \param key
 *  the entry's key.
 * \param write
 *  If TRUE, writes the entry to disk before flushing.
 *
 * Flushes a SymTabEntry from the symbol table.  All SymTabEntries below
 * the given entry are flushed as well.
 */
void
PST_FlushEntry (SymbolTable table, Key key, bool write)
{
  SymTabEntry entry;
  KeyList flush_list = NULL, kl;
  Key k;

  tag_entry_for_flush (table, key);

  if (write)
    PST_UpdateFile (table, key.file);

  /* Build a list of keys of symbols to free.  We will be modifying the
   * symbol table, so extract the list first so we don't try to iterate
   * and modify at the same time. */
  for (k = PST_GetFileEntryByType (table, key.file, ET_ANY); P_ValidKey (k);
       k = PST_GetFileEntryByTypeNext (table, k, ET_ANY))
    {
      entry = PST_GetSymTabEntry (table, k);

      if (P_TstSymTabEntryFlags (entry, STE_FLUSH_ME))
	flush_list = P_AppendKeyListNext (flush_list, P_NewKeyListWithKey (k));
    }
  
  for (kl = flush_list; kl; kl = P_GetKeyListNext (flush_list))
    {
      entry = PST_GetSymTabEntry (table, P_GetKeyListKey (kl));

      /* Use the P_Remove functions to remove the Pcode structure but
       * leave the SymTabEntry in place. */
      if (P_TstSymTabEntryFlags (entry, STE_FLUSH_ME))
	{
	  P_ClrSymTabEntryFlags (entry, STE_FLUSH_ME);

	  switch (P_GetSymTabEntryType (entry))
	    {
	    case ET_FUNC:
	      {
		FuncDcl f = P_GetSymTabEntryFuncDcl (entry);
		Scope s = P_GetSymTabEntryScope (entry);
		f = P_RemoveFuncDcl (f);
		s = P_RemoveScope (s);
		P_SetSymTabEntryFuncDcl (entry, f);
		P_SetSymTabEntryScope (entry, s);
	      }
	      break;
	    case ET_TYPE_GLOBAL:
	      {
		TypeDcl t = P_GetSymTabEntryTypeDcl (entry);
		t = P_RemoveTypeDcl (t);
		P_SetSymTabEntryTypeDcl (entry, t);
	      }
	      break;
	    case ET_TYPE_LOCAL:
	      P_SetSymTabEntryTypeDcl (entry, NULL);
	      break;
	    case ET_VAR_GLOBAL:
	      {
		VarDcl v = P_GetSymTabEntryVarDcl (entry);
		v = P_RemoveVarDcl (v);
		P_SetSymTabEntryVarDcl (entry, v);
	      }
	      break;
	    case ET_VAR_LOCAL:
	      P_SetSymTabEntryVarDcl (entry, NULL);
	      break;
	    case ET_STRUCT:
	      {
		StructDcl s = P_GetSymTabEntryStructDcl (entry);
		s = P_RemoveStructDcl (s);
		P_SetSymTabEntryStructDcl (entry, s);
	      }
	      break;
	    case ET_UNION:
	      {
		UnionDcl u = P_GetSymTabEntryUnionDcl (entry);
		u = P_RemoveUnionDcl (u);
		P_SetSymTabEntryUnionDcl (entry, u);
	      }
	      break;
	    case ET_FIELD:
	      P_SetSymTabEntryField (entry, NULL);
	      break;
	    case ET_ENUM:
	      {
		EnumDcl e = P_GetSymTabEntryEnumDcl (entry);
		e = P_RemoveEnumDcl (e);
		P_SetSymTabEntryEnumDcl (entry, e);
	      }
	      break;
	    case ET_ENUMFIELD:
	      P_SetSymTabEntryEnumField (entry, NULL);
	      break;
	    case ET_SCOPE:
	      {
		Scope s = P_GetSymTabEntryScope (entry);
		s = P_RemoveScope (s);
		P_SetSymTabEntryScope (entry, s);
	      }
	      break;
	    case ET_ASM:
	      {
		AsmDcl a = P_GetSymTabEntryAsmDcl (entry);
		a = P_RemoveAsmDcl (a);
		P_SetSymTabEntryAsmDcl (entry, a);
	      }
	      break;
	    case ET_STMT:
	      {
		Scope sc = P_GetSymTabEntryScope (entry);
		sc = P_RemoveScope (sc);
		P_SetSymTabEntryScope (entry, sc);
		/* The stmt will be removed with its parent function. */
		P_SetSymTabEntryStmt (entry, NULL);
	      }
	      break;
	    case ET_EXPR:
	      /* The Expr will be removed with its parent stmt. */
	      P_SetSymTabEntryExpr (entry, NULL);
	      break;
	    case ET_LABEL:
	      /* The Label will be removed with its parent stmt. */
	      P_SetSymTabEntryLabel (entry, NULL);
	      break;
	    default:
	      break;
	    }
	}
    }

  return;
}

/*! \brief Finds symbol table entries in a file.
 *
 * \param table
 *  the symbol table.
 * \param file
 *  the key of the file to search.
 *
 * This function finds symbol table entries in a file and stores a pointer
 * to the entry in the entry's SymTabEntry.
 */
void
PST_LinkSymbolsFile (SymbolTable table, int file)
{
  SymTabEntry entry;
  Key key;
  int i;

  for (i = 1; i <= table->ip_table[file]->num_entries; i++)
    {
      key.file = file;
      key.sym = i;

      entry = PST_GetSymTabEntry (table, key);

      if (entry == NULL)
	continue;

      switch (P_GetSymTabEntryType (entry))
	{
	case ET_FUNC:
	  PST_LinkSymbolsFunc (table, P_GetSymTabEntryFuncDcl (entry));
	  break;

	case ET_STRUCT:
	  PST_LinkSymbolsStruct (table, P_GetSymTabEntryStructDcl (entry));
	  break;

	case ET_UNION:
	  PST_LinkSymbolsUnion (table, P_GetSymTabEntryUnionDcl (entry));
	  break;

	case ET_ENUM:
	  PST_LinkSymbolsEnum (table, P_GetSymTabEntryEnumDcl (entry));
	  break;

	case ET_STMT:
	  PST_LinkSymbolsStmt (table, P_GetSymTabEntryStmt (entry));
	  break;

	case ET_EXPR:
	  PST_LinkSymbolsExpr (table, P_GetSymTabEntryExpr (entry));
	  break;

	case ET_TYPE_LOCAL:
	case ET_TYPE_GLOBAL:
	case ET_VAR_LOCAL:
	case ET_VAR_GLOBAL:
	case ET_ASM:
	case ET_FIELD:
	case ET_ENUMFIELD:
	case ET_LABEL:
	case ET_SCOPE:
	case ET_BLOCK:
	  break;

	default:
	  P_punt ("symtab.c:PST_LinkSymbolsFile:%d Unknown SymTabEntry type "
		  "%d", __LINE__ - 1, P_GetSymTabEntryType (entry));
	}
    }

  return;
}

/*! \brief Finds symbol table entries in a FuncDcl.
 *
 * \param table
 *  the symbol table.
 * \param func_dcl
 *  the FuncDcl to search.
 *
 * This function finds symbol table entries in a FuncDcl and stores a
 * pointer to the entry in the entry's SymTabEntry.
 */
void
PST_LinkSymbolsFunc (SymbolTable table, FuncDcl func_dcl)
{
  PST_LinkSymbolsVarList (table, P_GetFuncDclParam (func_dcl));
  PST_LinkSymbolsStmt (table, P_GetFuncDclStmt (func_dcl));

  return;
}

/*! \brief Finds symbol table entries in a StructDcl.
 *
 * \param table
 *  the symbol table.
 * \param struct_dcl
 *  the StructDcl to search.
 *
 * This function finds symbol table entries in a StructDcl and stores a
 * pointer to the entry in the entry's SymTabEntry.
 */
void
PST_LinkSymbolsStruct (SymbolTable table, StructDcl struct_dcl)
{
  SymTabEntry entry;
  Field f;

  for (f = P_GetStructDclFields (struct_dcl); f; f = P_GetFieldNext (f))
    {
      if ((entry = PST_GetSymTabEntryFromMem (table, P_GetFieldKey (f))))
	P_SetSymTabEntryField (entry, f);
    }

  return;
}

/*! \brief Finds symbol table entries in a UnionDcl.
 *
 * \param table
 *  the symbol table.
 * \param union_dcl
 *  the UnionDcl to search.
 *
 * This function finds symbol table entries in a UnionDcl and stores a
 * pointer to the entry in the entry's SymTabEntry.
 */
void
PST_LinkSymbolsUnion (SymbolTable table, UnionDcl union_dcl)
{
  SymTabEntry entry;
  Field f;

  for (f = P_GetUnionDclFields (union_dcl); f; f = P_GetFieldNext (f))
    {
      if ((entry = PST_GetSymTabEntryFromMem (table, P_GetFieldKey (f))))
	P_SetSymTabEntryField (entry, f);
    }

  return;
}

/*! \brief Finds symbol table entries in an EnumDcl.
 *
 * \param table
 *  the symbol table.
 * \param enum_dcl
 *  the EnumDcl to search.
 *
 * This function finds symbol table entries in an EnumDcl and stores a
 * pointer to the entry in the entry's SymTabEntry.
 */
void
PST_LinkSymbolsEnum (SymbolTable table, EnumDcl enum_dcl)
{
  SymTabEntry entry;
  EnumField e;

  for (e = P_GetEnumDclFields (enum_dcl); e; e = P_GetEnumFieldNext (e))
    {
      if ((entry = PST_GetSymTabEntryFromMem (table, P_GetEnumFieldKey (e))))
	P_SetSymTabEntryEnumField (entry, e);
    }

  return;
}

/*! \brief Finds symbol table entries in a TypeList.
 *
 * \param table
 *  the symbol table.
 * \param type_list
 *  the TypeList to search.
 *
 * This function finds symbol table entries in a TypeList and stores a
 * pointer to the entry in the entry's SymTabEntry.
 */
void
PST_LinkSymbolsTypeList (SymbolTable table, TypeList type_list)
{
  SymTabEntry entry;
  TypeDcl type_dcl;

  for (List_start (type_list), type_dcl = (TypeDcl)List_next (type_list);
       type_dcl; type_dcl = (TypeDcl)List_next (type_list))
    {
      if ((entry = PST_GetSymTabEntryFromMem (table,
					      P_GetTypeDclKey (type_dcl))))
	P_SetSymTabEntryTypeDcl (entry, type_dcl);
    }

  return;
}

/*! \brief Finds symbol table entries in a VarList.
 *
 * \param table
 *  the symbol table.
 * \param var_list
 *  the VarList to search.
 *
 * This function finds symbol table entries in a VarList and stores a
 * pointer to the entry in the entry's SymTabEntry.
 */
void
PST_LinkSymbolsVarList (SymbolTable table, VarList var_list)
{
  SymTabEntry entry;
  VarDcl var_dcl;

  for (List_start (var_list), var_dcl = (VarDcl)List_next (var_list); var_dcl;
       var_dcl = (VarDcl)List_next (var_list))
    {
      if ((entry = PST_GetSymTabEntryFromMem (table,
					      P_GetVarDclKey (var_dcl))))
	P_SetSymTabEntryVarDcl (entry, var_dcl);
    }
  
  return;
}

/*! \brief Finds symbol table entries in a Stmt.
 *
 * \param table
 *  the symbol table.
 * \param stmt
 *  the Stmt to search.
 *
 * This function finds symbol table entries in a Stmt and stores a
 * pointer to the entry in the entry's SymTabEntry.
 */
void
PST_LinkSymbolsStmt (SymbolTable table, Stmt stmt)
{
  P_StmtApply (stmt, link_symbols_stmt, link_symbols_expr, table);
  return;
}

/*! \brief Links symbol table entries in a Stmt.
 *
 * \param s
 *  the statement to inspect.
 * \param data
 *  used to pass in the symbol table.
 *
 * Links any symbols in \a s to their SymTabEntries in the table.  This
 * function must be applied with P_StmtApplyFunc().
 */
static void
link_symbols_stmt (Stmt s, void *data)
{
  SymbolTable table = (SymbolTable)data;
  SymTabEntry e;
  Key k;

  if (s)
    {
      k = P_GetStmtKey (s);
      
      if (P_ValidKey (k) && (e = PST_GetSymTabEntryFromMem (table, k)))
	P_SetSymTabEntryStmt (e, s);

      switch (P_GetStmtType (s))
	{
	case ST_NOOP:
	  {
	    Label label_list = P_GetStmtLabels (s);
	    
	    while (label_list)
	      {
		k = P_GetLabelKey (label_list);
		
		if (P_ValidKey (k) && \
		    (e = PST_GetSymTabEntryFromMem (table, k)))
		  P_SetSymTabEntryLabel (e, label_list);
		
		label_list = P_GetLabelNext (label_list);
	      }
	  }
	  break;
	case ST_COMPOUND:
	  {
	    Compound c = P_GetStmtCompound (s);
	    
	    PST_LinkSymbolsTypeList (table, P_GetCompoundTypeList (c));
	    PST_LinkSymbolsVarList (table, P_GetCompoundVarList (c));
	  }
	  break;
	default:
	  break;
	}
    }

  return;
}

/*! \brief Finds symbol table entries in an Expr.
 *
 * \param table
 *  the symbol table.
 * \param expr
 *  the Expr to search.
 *
 * This function finds symbol table entries in an Expr and stores a
 * pointer to the entry in the entry's SymTabEntry.
 */
void
PST_LinkSymbolsExpr (SymbolTable table, Expr expr)
{
  P_ExprApply (expr, link_symbols_stmt, link_symbols_expr, table);
  return;
}

/*! \brief Links symbol table entries in an Expr.
 *
 * \param e
 *  the expression to inspect.
 * \param data
 *  used to pass in the symbol table.
 *
 * Links any symbols in \a e to their SymTabEntries in the table.  This
 * function must be applied with P_ExprApplyFunc().
 */
static void
link_symbols_expr (Expr e, void *data)
{
  SymbolTable table = (SymbolTable)data;
  SymTabEntry entry;
  Key k;

  if (e)
    {
      k = P_GetExprKey (e);

      if (P_ValidKey (k) && (entry = PST_GetSymTabEntryFromMem (table, k)))
	P_SetSymTabEntryExpr (entry, e);
    }

  return;
}

/*! \brief Rotates the out_file field to the in_file field.
 *
 * \param table
 *  the symbol table.
 */
void
PST_RotateFile (SymbolTable table)
{
  int i;

  for (i = 1; i <= P_GetSymbolTableNumFiles (table); i++)
    {
      if (PST_GetFileOutName (table, i))
	{
	  if (PST_GetFileInName (table, i))
	    free (PST_GetFileInName (table, i));

	  PST_SetFileInName (table, i, PST_GetFileOutName (table, i));
	  PST_SetFileOutName (table, i, NULL);

	  /* If the out file status is FS_NOT_AVAIL, set the IPSTEF_NOT_AVAIL
	   * flag so the status will propagate to the next stage. */
	  if (PST_GetFileOutFileStatus (table, i) == FS_NOT_AVAIL)
	    PST_SetFileFlags (table, i, IPSTEF_NOT_AVAIL);
	}
    }

  if (P_GetSymbolTableOutName (table))
    {
      if (P_GetSymbolTableIPTableName (table))
	free (P_GetSymbolTableIPTableName (table));

      P_SetSymbolTableIPTableName (table, P_GetSymbolTableOutName (table));
      P_SetSymbolTableOutName (table, NULL);
    }

  return;
}

/*! \brief Prepares the symbol table for several insertions
 *
 * \param table
 *  the symbol table.
 * \param key
 *  the first key in the block.
 * \param length
 *  the block length.
 *
 * Allocates space in the symbol table for several symbols.
 */
void
PST_Prepare (SymbolTable table, Key key, int length)
{
  if (P_ValidKey (key))
    PST_PrepareIPSymTabEntTable (PST_GetFile (table, key.file), key.sym,
				 length);

  return;
}

/*! \brief Adds a table entry to a symbol table.
 *
 * \param table
 *  the symbol table.
 * \param type
 *  the type of entry to add.
 * \param entry
 *  the table entry to add.
 *
 * \return
 *  The key for the newly inserted symbol.
 *
 * Inserts the table entry into the symbol table.  \a entry must have
 * a valid file key (key.file).  The symbol is inserted in that file's
 * table.  If \a entry has a valid symbol key (key.sym), that key is
 * used when inserting.  If \a entry does not have a valid symbol key
 * (key.sym == 0), the entry is inserted as the last entry in the
 * table.  In either case, the symbol's key is returned.
 *
 * \sa PST_RemoveEntry(), PST_AddSymTabEntry(),
 * PST_AddFuncDclEntry(), PST_AddTypeDclEntry(),
 * PST_AddVarDclEntry(), PST_AddStructDclEntry(),
 * PST_AddUnionDclEntry(), PST_AddEnumDclEntry(),
 * PST_AddAsmDclEntry(), PST_AddStmtEntry(),
 * PST_AddExprEntry(), PST_AddFieldEntry(),
 * PST_AddEnumFieldEntry(), PST_AddLabelEntry(),
 * PST_GetEntry(), PST_GetSymTabEntry(),
 * PST_GetFuncDclEntry(), PST_GetTypeDclEntry(),
 * PST_GetVarDclEntry(), PST_GetStructDclEntry(),
 * PST_GetUnionDclEntry(), PST_GetEnumDclEntry(),
 * PST_GetAsmDclEntry(), PST_GetStmtEntry(),
 * PST_GetExprEntry(), PST_GetFieldEntry(),
 * PST_GetEnumFieldEntry(), PST_GetLabelEntry() */
Key
PST_AddEntry (SymbolTable table, _EntryType type, void *entry)
{
  switch (type)
    {
    case ET_FUNC:
      return (PST_AddFuncDclEntry (table, (FuncDcl)entry));
      break;

    case ET_TYPE_LOCAL:
      return (PST_AddTypeDclEntry (table, (TypeDcl)entry, ET_TYPE_LOCAL));
      break;

    case ET_TYPE_GLOBAL:
      return (PST_AddTypeDclEntry (table, (TypeDcl)entry, ET_TYPE_GLOBAL));
      break;

    case ET_VAR_LOCAL:
      return (PST_AddVarDclEntry (table, (VarDcl)entry, ET_VAR_LOCAL));
      break;

    case ET_VAR_GLOBAL:
      return (PST_AddVarDclEntry (table, (VarDcl)entry, ET_VAR_GLOBAL));
      break;

    case ET_STRUCT:
      return (PST_AddStructDclEntry (table, (StructDcl)entry));
      break;

    case ET_UNION:
      return (PST_AddUnionDclEntry (table, (UnionDcl)entry));
      break;

    case ET_ENUM:
      return (PST_AddEnumDclEntry (table, (EnumDcl)entry));
      break;

    case ET_ASM:
      return (PST_AddAsmDclEntry (table, (AsmDcl)entry));
      break;

    case ET_STMT:
      return (PST_AddStmtEntry (table, (Stmt)entry));
      break;

    case ET_EXPR:
      return (PST_AddExprEntry (table, (Expr)entry));
      break;

    case ET_FIELD:
      return (PST_AddFieldEntry (table, (Field)entry));
      break;

    case ET_ENUMFIELD:
      return (PST_AddEnumFieldEntry (table, (EnumField)entry));
      break;

    case ET_LABEL:
      return (PST_AddLabelEntry (table, (Label)entry));
      break;

    default:
      P_punt ("symtab.c:PST_AddEntry:%d Unknown entry type %d", __LINE__,
	      type);
    }

  /* Should never reach this point. */
  return (Invalid_Key);
}

/*! brief Adds a SymTabEntry to a symbol table.
 *
 * \param table
 *  the symbol table.
 * \param entry
 *  the symbol table entry to add.
 *
 * \return
 *  The key for the newly inserted symbol.
 *
 * Inserts a SymTabEntry structure into the symbol table.  \a entry
 * must have a valid file key (key.file).  The symbol is inserted in
 * that file's table.  If \a entry has a valid symbol key (key.sym),
 * that key is used when inserting.  If \a entry does not have a valid
 * symbol key (key.sym == 0), the entry is inserted as the last entry
 * in the table.  In either case, the symbol's key is returned.
 *
 * \sa PST_RemoveEntry(), P_NewSymTabEntry(), PST_AddEntry(),
 * PST_AddFuncDclEntry(), PST_AddTypeDclEntry(),
 * PST_AddVarDclEntry(), PST_AddStructDclEntry(),
 * PST_AddUnionDclEntry(), PST_AddEnumDclEntry(),
 * PST_AddAsmDclEntry(), PST_AddStmtEntry(),
 * PST_AddExprEntry(), PST_AddFieldEntry(),
 * PST_AddEnumFieldEntry(), PST_AddLabelEntry(),
 * PST_GetEntry(), PST_GetSymTabEntry(),
 * PST_GetFuncDclEntry(), PST_GetTypeDclEntry(),
 * PST_GetVarDclEntry(), PST_GetStructDclEntry(),
 * PST_GetUnionDclEntry(), PST_GetEnumDclEntry(),
 * PST_GetAsmDclEntry(), PST_GetStmtEntry(),
 * PST_GetExprEntry(), PST_GetFieldEntry(),
 * PST_GetEnumFieldEntry(), PST_GetLabelEntry() */
Key
PST_AddSymTabEntry (SymbolTable table, SymTabEntry entry)
{
  Key key = P_GetSymTabEntryKey (entry);

  /* Always add ET_BLOCK entries to allocate memory. */
  if (P_GetSymTabEntryType (entry) == ET_BLOCK)
    {
      key = PST_AddIPSymTabEntEntry \
	      (PST_GetFile (table, P_GetSymTabEntryBlockStart (entry).file),
	       entry);
    }
  /* Determine if the SymTabEntry's file is already in the SymbolTable. */
  else if (key.file == 0)
    {
      P_punt ("symtab.c:P_AddSymTabEntry:%d entry has invalid file key",
	      __LINE__ - 1);
    }
  else
    {
      if (PST_HasFile (table, key.file) == 0)
	{
	  /* If no space in the table has been allocated for the file,
	   * allocate space now. */
	  PST_AddFileWithKey (table, key.file, NULL, FT_SOURCE);
	}

      key = PST_AddIPSymTabEntEntry (PST_GetFile (table, key.file), entry);
    }

  return (key);
}

/*! \brief Adds a table entry for a FuncDcl.
 *
 * \param table
 *  the symbol table.
 * \param func_dcl
 *  the FuncDcl to add.
 *
 * \return
 *  The key for the newly inserted FuncDcl.
 *
 * Inserts the FuncDcl into the symbol table.  \a entry must have a
 * valid file key (key.file).  The symbol is inserted in that file's
 * table.  If \a entry has a valid symbol key (key.sym), that key is
 * used when inserting.  If \a entry does not have a valid symbol key
 * (key.sym == 0), the entry is inserted as the last entry in the
 * table.  In either case, the symbol's key is returned.
 *
 * \sa P_NewSymbolTable(), PST_AddSymTabEntry(),
 * PST_AddEntry(), PST_AddTypeDclEntry(),
 * PST_AddVarDclEntry(), PST_AddStructDclEntry(),
 * PST_AddUnionDclEntry(), PST_AddEnumDclEntry(),
 * PST_AddAsmDclEntry(), PST_AddStmtEntry(),
 * PST_AddExprEntry(), PST_AddFieldEntry(),
 * PST_AddEnumFieldEntry(), PST_AddLabelEntry(),
 * PST_GetEntry(), PST_GetSymTabEntry(),
 * PST_GetFuncDclEntry(), PST_GetTypeDclEntry(),
 * PST_GetVarDclEntry(), PST_GetStructDclEntry(),
 * PST_GetUnionDclEntry(), PST_GetEnumDclEntry(),
 * PST_GetAsmDclEntry(), PST_GetStmtEntry(),
 * PST_GetExprEntry(), PST_GetFieldEntry(),
 * PST_GetEnumFieldEntry(), PST_GetLabelEntry() */
Key
PST_AddFuncDclEntry (SymbolTable table, FuncDcl func_dcl)
{
  SymTabEntry new_entry = P_NewSymTabEntry ();
  char *name;

  P_SetSymTabEntryType (new_entry, ET_FUNC);
  P_SetSymTabEntryKey (new_entry, P_GetFuncDclKey (func_dcl));
  if ((name = P_GetFuncDclName (func_dcl)))
    P_SetSymTabEntryName (new_entry, strdup (name));
  P_SetSymTabEntryFuncDcl (new_entry, func_dcl);

  /* PST_AddSymTabEntry may update the key (if key.sym == 0),
   * so update func_dcl->key. */
  P_SetFuncDclKey (func_dcl, PST_AddSymTabEntry (table, new_entry));

  return (P_GetFuncDclKey (func_dcl));
}

/*! \brief Adds a table entry for a TypeDcl.
 *
 * \param table
 *  the symbol table.
 * \param type_dcl
 *  the TypeDcl to add.
 * \param type
 *  ET_TYPE_LOCAL or ET_TYPE_GLOBAL
 *
 * \return
 *  The key for the newly inserted TypeDcl.
 *
 * Inserts the TypeDcl into the symbol table.  \a entry must have a valid
 * file key (key.file).  The symbol is inserted in that file's table.
 * If \a entry has a valid symbol key (key.sym), that key is used when
 * inserting.  If \a entry does not have a valid symbol key (key.sym
 * == 0), the entry is inserted as the last entry in the table.  In
 * either case, the symbol's key is returned.
 *
 * \sa P_NewSymbolTable(), PST_AddSymTabEntry(),
 * PST_AddEntry(), PST_AddFuncDclEntry(),
 * PST_AddVarDclEntry(), PST_AddStructDclEntry(),
 * PST_AddUnionDclEntry(), PST_AddEnumDclEntry(),
 * PST_AddAsmDclEntry(), PST_AddStmtEntry(),
 * PST_AddExprEntry(), PST_AddFieldEntry(),
 * PST_AddEnumFieldEntry(), PST_AddLabelEntry(),
 * PST_GetEntry(), PST_GetSymTabEntry(),
 * PST_GetFuncDclEntry(), PST_GetTypeDclEntry(),
 * PST_GetVarDclEntry(), PST_GetStructDclEntry(),
 * PST_GetUnionDclEntry(), PST_GetEnumDclEntry(),
 * PST_GetAsmDclEntry(), PST_GetStmtEntry(),
 * PST_GetExprEntry(), PST_GetFieldEntry(),
 * PST_GetEnumFieldEntry(), PST_GetLabelEntry() */
Key
PST_AddTypeDclEntry (SymbolTable table, TypeDcl type_dcl, _EntryType type)
{
  SymTabEntry new_entry = P_NewSymTabEntry ();
  char *name;

  P_SetSymTabEntryType (new_entry, type);
  P_SetSymTabEntryKey (new_entry, P_GetTypeDclKey (type_dcl));
  if ((name = P_GetTypeDclName (type_dcl)))
    P_SetSymTabEntryName (new_entry, strdup (name));
  P_SetSymTabEntryTypeDcl (new_entry, type_dcl);

  /* PST_AddSymTabEntry may update the key (if key.sym == 0),
   * so update type_dcl->key. */
  P_SetTypeDclKey (type_dcl, PST_AddSymTabEntry (table, new_entry));

  return (P_GetTypeDclKey (type_dcl));
}

/*! \brief Adds a table entry for a VarDcl.
 *
 * \param table
 *  the symbol table.
 * \param var_dcl
 *  the VarDcl to add.
 * \param type
 *  ET_VAR_LOCAL or ET_VAR_GLOBAL
 *
 * \return
 *  The key for the newly inserted VarDcl.
 *
 * Inserts the VarDcl into the symbol table.  \a entry must have a
 * valid file key (key.file).  The symbol is inserted in that file's
 * table.  If \a entry has a valid symbol key (key.sym), that key is
 * used when inserting.  If \a entry does not have a valid symbol key
 * (key.sym == 0), the entry is inserted as the last entry in the
 * table.  In either case, the symbol's key is returned.
 *
 * \sa P_NewSymbolTable(), PST_AddSymTabEntry(),
 * PST_AddEntry(), PST_AddFuncDclEntry(),
 * PST_AddTypeDclEntry(), PST_AddStructDclEntry(),
 * PST_AddUnionDclEntry(), PST_AddEnumDclEntry(),
 * PST_AddAsmDclEntry(), PST_AddStmtEntry(),
 * PST_AddExprEntry(), PST_AddFieldEntry(),
 * PST_AddEnumFieldEntry(), PST_AddLabelEntry(),
 * PST_GetEntry(), PST_GetSymTabEntry(),
 * PST_GetFuncDclEntry(), PST_GetTypeDclEntry(),
 * PST_GetVarDclEntry(), PST_GetStructDclEntry(),
 * PST_GetUnionDclEntry(), PST_GetEnumDclEntry(),
 * PST_GetAsmDclEntry(), PST_GetStmtEntry(),
 * PST_GetExprEntry(), PST_GetFieldEntry(),
 * PST_GetEnumFieldEntry(), PST_GetLabelEntry() */
Key
PST_AddVarDclEntry (SymbolTable table, VarDcl var_dcl, _EntryType type)
{
  SymTabEntry new_entry = P_NewSymTabEntry ();
  char *name;

  if (!(type & ET_VAR))
    P_punt ("symtab.c:PST_AddVarDclEntry:%d Unknown type 0x%x", __LINE__,
	    type);

  P_SetSymTabEntryType (new_entry, type);
  P_SetSymTabEntryKey (new_entry, P_GetVarDclKey (var_dcl));
  if ((name = P_GetVarDclName (var_dcl)))
    P_SetSymTabEntryName (new_entry, strdup (name));
  P_SetSymTabEntryVarDcl (new_entry, var_dcl);

  /* PST_AddSymTabEntry may update the key (if key.sym == 0),
   * so update var_dcl->key. */
  P_SetVarDclKey (var_dcl, PST_AddSymTabEntry (table, new_entry));

  return (P_GetVarDclKey (var_dcl));
}

/*! \brief Adds a table entry for a StructDcl.
 *
 * \param table
 *  the symbol table.
 * \param struct_dcl
 *  the StructDcl to add.
 *
 * \return
 *  The key for the newly inserted StructDcl.
 *
 * Inserts the StructDcl into the symbol table.  \a entry must have a
 * valid file key (key.file).  The symbol is inserted in that file's
 * table.  If \a entry has a valid symbol key (key.sym), that key is
 * used when inserting.  If \a entry does not have a valid symbol key
 * (key.sym == 0), the entry is inserted as the last entry in the
 * table.  In either case, the symbol's key is returned.
 *
 * \sa P_NewSymbolTable(), PST_AddSymTabEntry(),
 * PST_AddEntry(), PST_AddFuncDclEntry(),
 * PST_AddTypeDclEntry(), PST_AddVarDclEntry(),
 * PST_AddUnionDclEntry(), PST_AddEnumDclEntry(),
 * PST_AddAsmDclEntry(), PST_AddStmtEntry(),
 * PST_AddExprEntry(), PST_AddFieldEntry(),
 * PST_AddEnumFieldEntry(), PST_AddLabelEntry(),
 * PST_GetEntry(), PST_GetSymTabEntry(),
 * PST_GetFuncDclEntry(), PST_GetTypeDclEntry(),
 * PST_GetVarDclEntry(), PST_GetStructDclEntry(),
 * PST_GetUnionDclEntry(), PST_GetEnumDclEntry(),
 * PST_GetAsmDclEntry(), PST_GetStmtEntry(),
 * PST_GetExprEntry(), PST_GetFieldEntry(),
 * PST_GetEnumFieldEntry(), PST_GetLabelEntry() */
Key
PST_AddStructDclEntry (SymbolTable table, StructDcl struct_dcl)
{
  SymTabEntry new_entry = P_NewSymTabEntry ();
  char *name;

  P_SetSymTabEntryType (new_entry, ET_STRUCT);
  P_SetSymTabEntryKey (new_entry, P_GetStructDclKey (struct_dcl));
  if ((name = P_GetStructDclName (struct_dcl)))
    P_SetSymTabEntryName (new_entry, strdup (name));
  P_SetSymTabEntryStructDcl (new_entry, struct_dcl);

  /* PST_AddSymTabEntry may update the key (if key.sym == 0),
   * so update struct_dcl->key. */
  P_SetStructDclKey (struct_dcl, PST_AddSymTabEntry (table, new_entry));

  return (P_GetStructDclKey (struct_dcl));
}

/*! \brief Adds a table entry for a UnionDcl.
 *
 * \param table
 *  the symbol table.
 * \param union_dcl
 *  the UnionDcl to add.
 *
 * \return
 *  The key for the newly inserted UnionDcl.
 *
 * Inserts the UnionDcl into the symbol table.  \a entry must have a
 * valid file key (key.file).  The symbol is inserted in that file's
 * table.  If \a entry has a valid symbol key (key.sym), that key is
 * used when inserting.  If \a entry does not have a valid symbol key
 * (key.sym == 0), the entry is inserted as the last entry in the
 * table.  In either case, the symbol's key is returned.
 *
 * \sa P_NewSymbolTable(), PST_AddSymTabEntry(),
 * PST_AddEntry(), PST_AddFuncDclEntry(),
 * PST_AddTypeDclEntry(), PST_AddVarDclEntry(),
 * PST_AddStructDclEntry(), PST_AddEnumDclEntry(),
 * PST_AddAsmDclEntry(), PST_AddStmtEntry(),
 * PST_AddExprEntry(), PST_AddFieldEntry(),
 * PST_AddEnumFieldEntry(), PST_AddLabelEntry(),
 * PST_GetEntry(), PST_GetSymTabEntry(),
 * PST_GetFuncDclEntry(), PST_GetTypeDclEntry(),
 * PST_GetVarDclEntry(), PST_GetStructDclEntry(),
 * PST_GetUnionDclEntry(), PST_GetEnumDclEntry(),
 * PST_GetAsmDclEntry(), PST_GetStmtEntry(),
 * PST_GetExprEntry(), PST_GetFieldEntry(),
 * PST_GetEnumFieldEntry(), PST_GetLabelEntry() */
Key
PST_AddUnionDclEntry (SymbolTable table, UnionDcl union_dcl)
{
  SymTabEntry new_entry = P_NewSymTabEntry ();
  char *name;

  P_SetSymTabEntryType (new_entry, ET_UNION);
  P_SetSymTabEntryKey (new_entry, P_GetUnionDclKey (union_dcl));
  if ((name = P_GetUnionDclName (union_dcl)))
    P_SetSymTabEntryName (new_entry, strdup (name));
  P_SetSymTabEntryUnionDcl (new_entry, union_dcl);

  /* PST_AddSymTabEntry may update the key (if key.sym == 0),
   * so update union_dcl->key. */
  P_SetUnionDclKey (union_dcl, PST_AddSymTabEntry (table, new_entry));

  return (P_GetUnionDclKey (union_dcl));
}

/*! \brief Adds a table entry for an EnumDcl.
 *
 * \param table
 *  the symbol table.
 * \param enum_dcl
 *  the EnumDcl to add.
 *
 * \return
 *  The key for the newly inserted EnumDcl.
 *
 * Inserts the EnumDcl into the symbol table.  \a entry must have a
 * valid file key (key.file).  The symbol is inserted in that file's
 * table.  If \a entry has a valid symbol key (key.sym), that key is
 * used when inserting.  If \a entry does not have a valid symbol key
 * (key.sym == 0), the entry is inserted as the last entry in the
 * table.  In either case, the symbol's key is returned.
 *
 * \sa P_NewSymbolTable(), PST_AddSymTabEntry(),
 * PST_AddEntry(), PST_AddFuncDclEntry(),
 * PST_AddTypeDclEntry(), PST_AddVarDclEntry(),
 * PST_AddStructDclEntry(), PST_AddUnionDclEntry(),
 * PST_AddAsmDclEntry(), PST_AddStmtEntry(),
 * PST_AddExprEntry(), PST_AddFieldEntry(),
 * PST_AddEnumFieldEntry(), PST_AddLabelEntry(),
 * PST_GetEntry(), PST_GetSymTabEntry(),
 * PST_GetFuncDclEntry(), PST_GetTypeDclEntry(),
 * PST_GetVarDclEntry(), PST_GetStructDclEntry(),
 * PST_GetUnionDclEntry(), PST_GetEnumDclEntry(),
 * PST_GetAsmDclEntry(), PST_GetStmtEntry(),
 * PST_GetExprEntry(), PST_GetFieldEntry(),
 * PST_GetEnumFieldEntry(), PST_GetLabelEntry() */
Key
PST_AddEnumDclEntry (SymbolTable table, EnumDcl enum_dcl)
{
  SymTabEntry new_entry = P_NewSymTabEntry ();
  char *name;

  P_SetSymTabEntryType (new_entry, ET_ENUM);
  P_SetSymTabEntryKey (new_entry, P_GetEnumDclKey (enum_dcl));
  if ((name = P_GetEnumDclName (enum_dcl)))
    P_SetSymTabEntryName (new_entry, strdup (name));
  P_SetSymTabEntryEnumDcl (new_entry, enum_dcl);

  /* PST_AddSymTabEntry may update the key (if key.sym == 0),
   * so update enum_dcl->key. */
  P_SetEnumDclKey (enum_dcl, PST_AddSymTabEntry (table, new_entry));

  return (P_GetEnumDclKey (enum_dcl));
}

/*! \brief Adds a table entry for an AsmDcl.
 *
 * \param table
 *  the symbol table.
 * \param asm_dcl
 *  the AsmDcl to add.
 *
 * \return
 *  The key for the newly inserted AsmDcl.
 *
 * Inserts the AsmDcl into the symbol table.  \a entry must have a
 * valid file key (key.file).  The symbol is inserted in that file's
 * table.  If \a entry has a valid symbol key (key.sym), that key is
 * used when inserting.  If \a entry does not have a valid symbol key
 * (key.sym == 0), the entry is inserted as the last entry in the
 * table.  In either case, the symbol's key is returned.
 *
 * \sa P_NewSymbolTable(), PST_AddSymTabEntry(),
 * PST_AddEntry(), PST_AddFuncDclEntry(),
 * PST_AddTypeDclEntry(), PST_AddVarDclEntry(),
 * PST_AddStructDclEntry(), PST_AddUnionDclEntry(),
 * PST_AddEnumDclEntry(), PST_AddStmtEntry(),
 * PST_AddExprEntry(), PST_AddFieldEntry(),
 * PST_AddEnumFieldEntry(), PST_AddLabelEntry(),
 * PST_GetEntry(), PST_GetSymTabEntry(),
 * PST_GetFuncDclEntry(), PST_GetTypeDclEntry(),
 * PST_GetVarDclEntry(), PST_GetStructDclEntry(),
 * PST_GetUnionDclEntry(), PST_GetEnumDclEntry(),
 * PST_GetAsmDclEntry(), PST_GetStmtEntry(),
 * PST_GetExprEntry(), PST_GetFieldEntry(),
 * PST_GetEnumFieldEntry(), PST_GetLabelEntry() */
Key
PST_AddAsmDclEntry (SymbolTable table, AsmDcl asm_dcl)
{
  SymTabEntry new_entry = P_NewSymTabEntry ();

  P_SetSymTabEntryType (new_entry, ET_ASM);
  P_SetSymTabEntryKey (new_entry, P_GetAsmDclKey (asm_dcl));
  P_SetSymTabEntryAsmDcl (new_entry, asm_dcl);

  /* PST_AddSymTabEntry may update the key (if key.sym == 0),
   * so update asm_dcl->key. */
  P_SetAsmDclKey (asm_dcl, PST_AddSymTabEntry (table, new_entry));

  return (P_GetAsmDclKey (asm_dcl));
}

/*! \brief Adds a table entry for a Stmt.
 *
 * \param table
 *  the symbol table.
 * \param stmt
 *  the Stmt to add.
 *
 * \return
 *  The key for the newly inserted Stmt.
 *
 * Inserts the Stmt into the symbol table.  \a entry must have a valid
 * file key (key.file).  The symbol is inserted in that file's table.
 * If \a entry has a valid symbol key (key.sym), that key is used when
 * inserting.  If \a entry does not have a valid symbol key (key.sym
 * == 0), the entry is inserted as the last entry in the table.  In
 * either case, the symbol's key is returned.
 *
 * \sa P_NewSymbolTable(), PST_AddSymTabEntry(),
 * PST_AddEntry(), PST_AddFuncDclEntry(),
 * PST_AddTypeDclEntry(), PST_AddVarDclEntry(),
 * PST_AddStructDclEntry(), PST_AddUnionDclEntry(),
 * PST_AddEnumDclEntry(), PST_AddAsmDclEntry(),
 * PST_AddExprEntry(), PST_AddFieldEntry(),
 * PST_AddEnumFieldEntry(), PST_AddLabelEntry(),
 * PST_GetEntry(), PST_GetSymTabEntry(),
 * PST_GetFuncDclEntry(), PST_GetTypeDclEntry(),
 * PST_GetVarDclEntry(), PST_GetStructDclEntry(),
 * PST_GetUnionDclEntry(), PST_GetEnumDclEntry(),
 * PST_GetAsmDclEntry(), PST_GetStmtEntry(),
 * PST_GetExprEntry(), PST_GetFieldEntry(),
 * PST_GetEnumFieldEntry(), PST_GetLabelEntry() */
Key
PST_AddStmtEntry (SymbolTable table, Stmt stmt)
{
  SymTabEntry new_entry = P_NewSymTabEntry ();

  P_SetSymTabEntryType (new_entry, ET_STMT);
  P_SetSymTabEntryKey (new_entry, P_GetStmtKey (stmt));
  P_SetSymTabEntryStmt (new_entry, stmt);

  /* PST_AddSymTabEntry may update the key (if key.sym == 0),
   * so update stmt->key. */
  P_SetStmtKey (stmt, PST_AddSymTabEntry (table, new_entry));

  return (P_GetStmtKey (stmt));
}

/*! \brief Adds a table entry for an Expr.
 *
 * \param table
 *  the symbol table.
 * \param expr
 *  the Expr to add.
 *
 * \return
 *  The key for the newly inserted Expr.
 *
 * Inserts the Expr into the symbol table.  \a entry must have a valid
 * file key (key.file).  The symbol is inserted in that file's table.
 * If \a entry has a valid symbol key (key.sym), that key is used when
 * inserting.  If \a entry does not have a valid symbol key (key.sym
 * == 0), the entry is inserted as the last entry in the table.  In
 * either case, the symbol's key is returned.
 *
 * \sa P_NewSymbolTable(), PST_AddSymTabEntry(),
 * PST_AddEntry(), PST_AddFuncDclEntry(),
 * PST_AddTypeDclEntry(), PST_AddVarDclEntry(),
 * PST_AddStructDclEntry(), PST_AddUnionDclEntry(),
 * PST_AddEnumDclEntry(), PST_AddAsmDclEntry(),
 * PST_AddStmtEntry(), PST_AddFieldEntry(),
 * PST_AddEnumFieldEntry(), PST_AddLabelEntry(),
 * PST_GetEntry(), PST_GetSymTabEntry(),
 * PST_GetFuncDclEntry(), PST_GetTypeDclEntry(),
 * PST_GetVarDclEntry(), PST_GetStructDclEntry(),
 * PST_GetUnionDclEntry(), PST_GetEnumDclEntry(),
 * PST_GetAsmDclEntry(), PST_GetStmtEntry(),
 * PST_GetExprEntry(), PST_GetFieldEntry(),
 * PST_GetEnumFieldEntry(), PST_GetLabelEntry() */
Key
PST_AddExprEntry (SymbolTable table, Expr expr)
{
  SymTabEntry new_entry = P_NewSymTabEntry ();

  P_SetSymTabEntryType (new_entry, ET_EXPR);
  P_SetSymTabEntryKey (new_entry, P_GetExprKey (expr));
  P_SetSymTabEntryExpr (new_entry, expr);

  /* PST_AddSymTabEntry may update the key (if key.sym == 0),
   * so update expr->key. */
  P_SetExprKey (expr, PST_AddSymTabEntry (table, new_entry));

  return (P_GetExprKey (expr));
}

/*! \brief Adds a table entry for a Field.
 *
 * \param table
 *  the symbol table.
 * \param field
 *  the Field to add.
 *
 * \return
 *  The key for the newly inserted Field.
 *
 * Inserts the Field into the symbol table.  \a entry must have a
 * valid file key (key.file).  The symbol is inserted in that file's
 * table.  If \a entry has a valid symbol key (key.sym), that key is
 * used when inserting.  If \a entry does not have a valid symbol key
 * (key.sym == 0), the entry is inserted as the last entry in the
 * table.  In either case, the symbol's key is returned.
 *
 * \sa P_NewSymbolTable(), PST_AddSymTabEntry(),
 * PST_AddEntry(), PST_AddFuncDclEntry(),
 * PST_AddTypeDclEntry(), PST_AddVarDclEntry(),
 * PST_AddStructDclEntry(), PST_AddUnionDclEntry(),
 * PST_AddEnumDclEntry(), PST_AddAsmDclEntry(),
 * PST_AddStmtEntry(), PST_AddExprEntry(),
 * PST_AddEnumFieldEntry(), PST_AddLabelEntry(),
 * PST_GetEntry(), PST_GetSymTabEntry(),
 * PST_GetFuncDclEntry(), PST_GetTypeDclEntry(),
 * PST_GetVarDclEntry(), PST_GetStructDclEntry(),
 * PST_GetUnionDclEntry(), PST_GetEnumDclEntry(),
 * PST_GetAsmDclEntry(), PST_GetStmtEntry(),
 * PST_GetExprEntry(), PST_GetFieldEntry(),
 * PST_GetEnumFieldEntry(), PST_GetLabelEntry() */
Key
PST_AddFieldEntry (SymbolTable table, Field field)
{
  SymTabEntry new_entry = P_NewSymTabEntry ();
  char *name;

  P_SetSymTabEntryType (new_entry, ET_FIELD);
  P_SetSymTabEntryKey (new_entry, P_GetFieldKey (field));
  if ((name = P_GetFieldName (field)))
    P_SetSymTabEntryName (new_entry, strdup (name));
  P_SetSymTabEntryField (new_entry, field);

  /* PST_AddSymTabEntry may update the key (if key.sym == 0),
   * so update field->key. */
  P_SetFieldKey (field, PST_AddSymTabEntry (table, new_entry));

  return (P_GetFieldKey (field));
}

/*! \brief Adds a table entry for an EnumField.
 *
 * \param table
 *  the symbol table.
 * \param enum_field
 *  the EnumField to add.
 *
 * \return
 *  The key for the newly inserted EnumField.
 *
 * Inserts the EnumField into the symbol table.  \a entry must have a
 * valid file key (key.file).  The symbol is inserted in that file's
 * table.  If \a entry has a valid symbol key (key.sym), that key is
 * used when inserting.  If \a entry does not have a valid symbol key
 * (key.sym == 0), the entry is inserted as the last entry in the
 * table.  In either case, the symbol's key is returned.
 *
 * \sa P_NewSymbolTable(), PST_AddSymTabEntry(),
 * PST_AddEntry(), PST_AddFuncDclEntry(),
 * PST_AddTypeDclEntry(), PST_AddVarDclEntry(),
 * PST_AddStructDclEntry(), PST_AddUnionDclEntry(),
 * PST_AddEnumDclEntry(), PST_AddAsmDclEntry(),
 * PST_AddStmtEntry(), PST_AddExprEntry(),
 * PST_AddFieldEntry(), PST_AddLabelEntry(),
 * PST_GetEntry(), PST_GetSymTabEntry(),
 * PST_GetFuncDclEntry(), PST_GetTypeDclEntry(),
 * PST_GetVarDclEntry(), PST_GetStructDclEntry(),
 * PST_GetUnionDclEntry(), PST_GetEnumDclEntry(),
 * PST_GetAsmDclEntry(), PST_GetStmtEntry(),
 * PST_GetExprEntry(), PST_GetFieldEntry(),
 * PST_GetEnumFieldEntry(), PST_GetLabelEntry() */
Key
PST_AddEnumFieldEntry (SymbolTable table, EnumField enum_field)
{
  SymTabEntry new_entry = P_NewSymTabEntry ();
  char *name;
  

  P_SetSymTabEntryType (new_entry, ET_ENUMFIELD);
  P_SetSymTabEntryKey (new_entry, P_GetEnumFieldKey (enum_field));
  if ((name = P_GetEnumFieldName (enum_field)))
    P_SetSymTabEntryName (new_entry, strdup (name));
  P_SetSymTabEntryEnumField (new_entry, enum_field);

  /* PST_AddSymTabEntry may update the key (if key.sym == 0),
   * so update field->key. */
  P_SetEnumFieldKey (enum_field, PST_AddSymTabEntry (table, new_entry));

  return (P_GetEnumFieldKey (enum_field));
}

/*! \brief Adds a table entry for a Label.
 *
 * \param table
 *  the symbol table.
 * \param label
 *  the Label to add.
 *
 * \return
 *  The key for the newly inserted EnumField.
 *
 * Inserts the Label into the symbol table.  \a entry must have a
 * valid file key (key.file).  The symbol is inserted in that file's
 * table.  If \a entry has a valid symbol key (key.sym), that key is
 * used when inserting.  If \a entry does not have a valid symbol key
 * (key.sym == 0), the entry is inserted as the last entry in the
 * table.  In either case, the symbol's key is returned.
 *
 * \sa P_NewSymbolTable(), PST_AddSymTabEntry(),
 * PST_AddEntry(), PST_AddFuncDclEntry(),
 * PST_AddTypeDclEntry(), PST_AddVarDclEntry(),
 * PST_AddStructDclEntry(), PST_AddUnionDclEntry(),
 * PST_AddEnumDclEntry(), PST_AddAsmDclEntry(),
 * PST_AddStmtEntry(), PST_AddExprEntry(),
 * PST_AddFieldEntry(), PST_AddEnumFieldEntry(),
 * PST_GetEntry(), PST_GetSymTabEntry(),
 * PST_GetFuncDclEntry(), PST_GetTypeDclEntry(),
 * PST_GetVarDclEntry(), PST_GetStructDclEntry(),
 * PST_GetUnionDclEntry(), PST_GetEnumDclEntry(),
 * PST_GetAsmDclEntry(), PST_GetStmtEntry(),
 * PST_GetExprEntry(), PST_GetFieldEntry(),
 * PST_GetEnumFieldEntry(), PST_GetLabelEntry() */
Key
PST_AddLabelEntry (SymbolTable table, Label label)
{
  SymTabEntry new_entry = P_NewSymTabEntry ();

  P_SetSymTabEntryType (new_entry, ET_LABEL);
  P_SetSymTabEntryKey (new_entry, P_GetLabelKey (label));
  P_SetSymTabEntryLabel (new_entry, label);

  /* PST_AddSymTabEntry may update the key (if key.sym == 0),
   * so update field->key. */
  P_SetLabelKey (label, PST_AddSymTabEntry (table, new_entry));

  return (P_GetLabelKey (label));
}

/*! \brief Retrieves a table entry from a symbol table.
 *
 * \param table
 *  the symbol table
 * \param key
 *  the key of the entry to retrieve.
 * \param entry
 *  will be updated to point to the entry.
 *
 * \return
 *  The type of the entry returned in \a entry.
 *
 * Retrieves a table entry from the symbol table and returns a pointer
 * in \a *entry.  The type of the entry is returned to the caller.  \n
 * *entry must not be freed by the caller.
 *
 * \sa P_NewSymbolTable(), PST_AddSymTabEntry(),
 * PST_AddEntry(), PST_AddFuncDclEntry(),
 * PST_AddTypeDclEntry(), PST_AddVarDclEntry(),
 * PST_AddStructDclEntry(), PST_AddUnionDclEntry(),
 * PST_AddEnumDclEntry(), PST_AddAsmDclEntry(),
 * PST_AddStmtEntry(), PST_AddExprEntry(),
 * PST_AddFieldEntry(), PST_AddEnumFieldEntry(),
 * PST_AddLabelEntry(), PST_GetSymTabEntry(),
 * PST_GetFuncDclEntry(), PST_GetTypeDclEntry(),
 * PST_GetVarDclEntry(), PST_GetStructDclEntry(),
 * PST_GetUnionDclEntry(), PST_GetEnumDclEntry(),
 * PST_GetAsmDclEntry(), PST_GetStmtEntry(),
 * PST_GetExprEntry(), PST_GetFieldEntry(),
 * PST_GetEnumFieldEntry(), PST_GetLabelEntry() */
_EntryType
PST_GetEntry (SymbolTable table, Key key, void **entry)
{
  SymTabEntry target;

  if ((target = PST_GetSymTabEntry (table, key)))
    {
      switch (P_GetSymTabEntryType (target))
	{
	case ET_FUNC:
	  *entry = (void *)(P_GetSymTabEntryFuncDcl (target));
	  break;

	case ET_TYPE_LOCAL:
	case ET_TYPE_GLOBAL:
	  *entry = (void *)(P_GetSymTabEntryTypeDcl (target));
	  break;

	case ET_VAR_LOCAL:
	case ET_VAR_GLOBAL:
	  *entry = (void *)(P_GetSymTabEntryVarDcl (target));
	  break;

	case ET_STRUCT:
	  *entry = (void *)(P_GetSymTabEntryStructDcl (target));
	  break;

	case ET_UNION:
	  *entry = (void *)(P_GetSymTabEntryUnionDcl (target));
	  break;

	case ET_ENUM:
	  *entry = (void *)(P_GetSymTabEntryEnumDcl (target));
	  break;

	case ET_ASM:
	  *entry = (void *)(P_GetSymTabEntryAsmDcl (target));
	  break;

	case ET_STMT:
	  *entry = (void *)(P_GetSymTabEntryStmt (target));
	  break;

	case ET_EXPR:
	  *entry = (void *)(P_GetSymTabEntryExpr (target));
	  break;

	case ET_FIELD:
	  *entry = (void *)(P_GetSymTabEntryField (target));
	  break;

	case ET_ENUMFIELD:
	  *entry = (void *)(P_GetSymTabEntryEnumField (target));
	  break;

	case ET_LABEL:
	  *entry = (void *)(P_GetSymTabEntryLabel (target));
	  break;

	default:
	  P_punt ("symtab.c:PST_GetEntry:%d Unknown entry type %d", __LINE__,
		  P_GetSymTabEntryType (target));
	}
    }
  else
    {
#if 0
      P_punt ("symtab.c:PST_GetEntry:%d No entry for key (%d, %d)", __LINE__,
	      key.file, key.sym);
#endif
      *entry = NULL;
      return (ET_NONE);
    }

  return (P_GetSymTabEntryType (target));
}

/*! \brief Retrieves a table entry from a symbol table.
 *
 * \param table
 *  the symbol table.
 * \param key
 *  the desired table entry's key.
 *
 * \return
 *   A pointer to the SymTabEntry.  This pointer must not be freed
 *   by the user.  If the requested key does not exist, returns null.
 *
 * Retrieves a ::SymTabEntry with the given key from the table.  The table
 * in memory and files on disk are searched in the order specified
 * in \a table.search_order.
 *
 * \sa PST_GetSymTabEntryFromSource(), PST_GetSymTabEntryCopyFromSource() */
SymTabEntry
PST_GetSymTabEntry (SymbolTable table, Key key)
{
  SymTabEntry result = NULL;
  int i;

  if (P_ValidKey (key) && \
      key.file <= P_GetSymbolTableNumFiles (table) && \
      PST_GetFile (table, key.file) && \
      key.sym <= PST_GetFileNumEntries (table, key.file))
    {   
      for (i = 0; result == NULL && i < SO_NUM_SOURCES; i++)
	result = \
	  PST_GetSymTabEntryFromSource (table, key,
					P_GetSymbolTableSearchOrder (table,
								     i));
    }

  return (result);
}

/*! \brief Retrieves a table entry from a symbol table
 *
 * \param table
 *  the symbol table.
 * \param key
 *  the desired table entry's key.
 * \param source
 *  the source from which to read the key.
 *
 * \return
 *  A pointer to the SymTabEntry.  This pointer must not be freed
 *  by the user.
 *
 * Retrieves a ::SymTabEntry with the given key from the table.  This function
 * reads from the source specified in \a source.
 *
 * \sa PST_GetSymTabEntry(), PST_GetSymTabEntryCopyFromSource()
 */
SymTabEntry
PST_GetSymTabEntryFromSource (SymbolTable table, Key key,
			      _STSearchOrder source)
{
  SymTabEntry result = NULL;

  switch (source)
    {
    case SO_MEM:
      result = PST_GetSymTabEntryFromMem (table, key);

      /* If the SymTabEntry is in memory, but the referenced Pcode structure
       * is not, return NULL.  This entry will be read when this function
       * is called with a file source. */
      if (result && must_read (result))
	result = NULL;
      break;
    case SO_IN:
      result = PST_ReadSymbolFromIn (table, key);
      break;
    case SO_OUT:
      result = PST_ReadSymbolFromOut (table, key);
      break;
    default:
      break;
    }

  return (result);
}

/*! \brief Retrieves a table entry from a symbol table and makes a copy.
 *
 * \param table
 *  the symbol table.
 * \param key
 *  the desired table entry's key.
 * \param source
 *  the source from which to read the key.
 *
 * \return
 *  A pointer to the SymTabEntry.  This pointer must not be freed by the user.
 *
 * Retrieves a ::SymTabEntry with the given key from the table.  This function
 * makes and inserts a copy of the symbol into the table in memory and
 * returns a pointer to the copy.  The keys in the copy are updated to
 * ensure uniqueness.
 *
 * \sa PST_GetSymTabEntry(), PST_GetSymTabEntryFromSource()
 */
SymTabEntry
PST_GetSymTabEntryCopyFromSource (SymbolTable table, Key key,
				  _STSearchOrder source)
{
  SymTabEntry result = NULL;

  switch (source)
    {
    case SO_MEM:
      result = \
	PST_CopySymTabEntryToTableScope (table, Invalid_Key, table,
					 PST_GetSymTabEntry (table, key),
					 FALSE);
      break;
    case SO_IN:
      result = PST_ReadSymbolCopyFromIn (table, key);
      break;
    case SO_OUT:
      result = PST_ReadSymbolCopyFromOut (table, key);
      break;
    default:
      break;
    }

  return (result);
}

/*! \brief Retrieves a FuncDcl from a symbol table.
 *
 * \param table
 *  the symbol table.
 * \param key
 *  the desired FuncDcl's key.
 *
 * \return
 *  A pointer to the FuncDcl.
 *
 * Retrieves a FuncDcl from a symbol table and returns a pointer.  The
 * caller must not free the returned pointer.  This function will
 * return null if the requested symbol is not a FuncDcl.
 *
 * \sa P_NewSymbolTable(), PST_AddSymTabEntry(),
 * PST_AddEntry(), PST_AddFuncDclEntry(),
 * PST_AddTypeDclEntry(), PST_AddVarDclEntry(),
 * PST_AddStructDclEntry(), PST_AddUnionDclEntry(),
 * PST_AddEnumDclEntry(), PST_AddAsmDclEntry(),
 * PST_AddStmtEntry(), PST_AddExprEntry(),
 * PST_AddFieldEntry(), PST_AddEnumFieldEntry(),
 * PST_AddLabelEntry(), PST_GetEntry(),
 * PST_GetTypeDclEntry(), PST_GetVarDclEntry(),
 * PST_GetStructDclEntry(), PST_GetUnionDclEntry(),
 * PST_GetEnumDclEntry(), PST_GetAsmDclEntry(),
 * PST_GetStmtEntry(), PST_GetExprEntry(),
 * PST_GetFieldEntry(), PST_GetEnumFieldEntry(),
 * PST_GetLabelEntry() */
FuncDcl
PST_GetFuncDclEntry (SymbolTable table, Key key)
{
  _EntryType type;
  FuncDcl func_dcl;

  type = PST_GetEntry (table, key, (void **)&func_dcl);

  if (type == ET_FUNC)
    return (func_dcl);
  else
    return (NULL);
}

/*! \brief Retrieves a TypeDcl from a symbol table.
 *
 * \param table
 *  the symbol table.
 * \param key
 *  the desired TypeDcl's key.
 *
 * \return
 *  A pointer to the TypeDcl.
 *
 * Retrieves a TypeDcl from a symbol table and returns a pointer.  The
 * caller must not free the returned pointer.  This function will
 * return null if the requested symbol is not a TypeDcl.
 *
 * \sa P_NewSymbolTable(), PST_AddSymTabEntry(),
 * PST_AddEntry(), PST_AddFuncDclEntry(),
 * PST_AddTypeDclEntry(), PST_AddVarDclEntry(),
 * PST_AddStructDclEntry(), PST_AddUnionDclEntry(),
 * PST_AddEnumDclEntry(), PST_AddAsmDclEntry(),
 * PST_AddStmtEntry(), PST_AddExprEntry(),
 * PST_AddFieldEntry(), PST_AddEnumFieldEntry(),
 * PST_AddLabelEntry(), PST_GetEntry(),
 * PST_GetFuncDclEntry(), PST_GetVarDclEntry(),
 * PST_GetStructDclEntry(), PST_GetUnionDclEntry(),
 * PST_GetEnumDclEntry(), PST_GetAsmDclEntry(),
 * PST_GetStmtEntry(), PST_GetExprEntry(),
 * PST_GetFieldEntry(), PST_GetEnumFieldEntry(),
 * PST_GetLabelEntry() */
TypeDcl
PST_GetTypeDclEntry (SymbolTable table, Key key)
{
  _EntryType entry_type;
  TypeDcl type_dcl;

  entry_type = PST_GetEntry (table, key, (void **)&type_dcl);

  if (entry_type & ET_TYPE)
    return (type_dcl);
  else
    return (NULL);
}

/*! \brief Retrieves a VarDcl from a symbol table.
 *
 * \param table
 *  the symbol table.
 * \param key
 *  the desired VarDcl's key.
 *
 * \return
 *  A pointer to the VarDcl.
 *
 * Retrieves a VarDcl from a symbol table and returns a pointer.  The
 * caller must not free the returned pointer.  This function will
 * return null if the requested symbol is not a VarDcl.
 *
 * \sa P_NewSymbolTable(), PST_AddSymTabEntry(),
 * PST_AddEntry(), PST_AddFuncDclEntry(),
 * PST_AddTypeDclEntry(), PST_AddVarDclEntry(),
 * PST_AddStructDclEntry(), PST_AddUnionDclEntry(),
 * PST_AddEnumDclEntry(), PST_AddAsmDclEntry(),
 * PST_AddStmtEntry(), PST_AddExprEntry(),
 * PST_AddFieldEntry(), PST_AddEnumFieldEntry(),
 * PST_AddLabelEntry(), PST_GetEntry(),
 * PST_GetFuncDclEntry(), PST_GetTypeDclEntry(),
 * PST_GetStructDclEntry(), PST_GetUnionDclEntry(),
 * PST_GetEnumDclEntry(), PST_GetAsmDclEntry(),
 * PST_GetStmtEntry(), PST_GetExprEntry(),
 * PST_GetFieldEntry(), PST_GetEnumFieldEntry(),
 * PST_GetLabelEntry() */
VarDcl
PST_GetVarDclEntry (SymbolTable table, Key key)
{
  _EntryType type;
  VarDcl var_dcl;

  type = PST_GetEntry (table, key, (void **)&var_dcl);

  if (type & ET_VAR)
    return (var_dcl);
  else
    return (NULL);
}

/*! \brief Retrieves a StructDcl from a symbol table.
 *
 * \param table
 *  the symbol table.
 * \param key
 *  the desired StructDcl's key.
 *
 * \return
 *  A pointer to the StructDcl.
 *
 * Retrieves a StructDcl from a symbol table and returns a pointer.
 * The caller must not free the returned pointer.  This function will
 * return null if the requested symbol is not a StructDcl.
 *
 * \sa P_NewSymbolTable(), PST_AddSymTabEntry(),
 * PST_AddEntry(), PST_AddFuncDclEntry(),
 * PST_AddTypeDclEntry(), PST_AddVarDclEntry(),
 * PST_AddStructDclEntry(), PST_AddUnionDclEntry(),
 * PST_AddEnumDclEntry(), PST_AddAsmDclEntry(),
 * PST_AddStmtEntry(), PST_AddExprEntry(),
 * PST_AddFieldEntry(), PST_AddEnumFieldEntry(),
 * PST_AddLabelEntry(), PST_GetEntry(),
 * PST_GetFuncDclEntry(), PST_GetTypeDclEntry(),
 * PST_GetVarDclEntry(), PST_GetUnionDclEntry(),
 * PST_GetEnumDclEntry(), PST_GetAsmDclEntry(),
 * PST_GetStmtEntry(), PST_GetExprEntry(),
 * PST_GetFieldEntry(), PST_GetEnumFieldEntry(),
 * PST_GetLabelEntry() */
StructDcl
PST_GetStructDclEntry (SymbolTable table, Key key)
{
  _EntryType type;
  StructDcl struct_dcl;

  type = PST_GetEntry (table, key, (void **)&struct_dcl);

  if (type == ET_STRUCT)
    return (struct_dcl);
  else
    return (NULL);
}

/*! \brief Retrieves a UnionDcl from a symbol table.
 *
 * \param table
 *  the symbol table.
 * \param key
 *  the desired UnionDcl's key.
 *
 * \return
 *  A pointer to the UnionDcl.
 *
 * Retrieves a UnionDcl from a symbol table and returns a pointer.
 * The caller must not free the returned pointer.  This function will
 * return null if the requested symbol is not a UnionDcl.
 *
 * \sa P_NewSymbolTable(), PST_AddSymTabEntry(),
 * PST_AddEntry(), PST_AddFuncDclEntry(),
 * PST_AddTypeDclEntry(), PST_AddVarDclEntry(),
 * PST_AddStructDclEntry(), PST_AddUnionDclEntry(),
 * PST_AddEnumDclEntry(), PST_AddAsmDclEntry(),
 * PST_AddStmtEntry(), PST_AddExprEntry(),
 * PST_AddFieldEntry(), PST_AddEnumFieldEntry(),
 * PST_AddLabelEntry(), PST_GetEntry(),
 * PST_GetFuncDclEntry(), PST_GetTypeDclEntry(),
 * PST_GetVarDclEntry(), PST_GetStructDclEntry(),
 * PST_GetEnumDclEntry(), PST_GetAsmDclEntry(),
 * PST_GetStmtEntry(), PST_GetExprEntry(),
 * PST_GetFieldEntry(), PST_GetEnumFieldEntry(),
 * PST_GetLabelEntry() */
UnionDcl
PST_GetUnionDclEntry (SymbolTable table, Key key)
{
  _EntryType type;
  UnionDcl union_dcl;

  type = PST_GetEntry (table, key, (void **)&union_dcl);

  if (type == ET_UNION)
    return (union_dcl);
  else
    return (NULL);
}

/*! \brief Retrieves an EnumDcl from a symbol table.
 *
 * \param table
 *  the symbol table.
 * \param key
 *  the desired EnumDcl's key.
 *
 * \return
 *  A pointer to the EnumDcl.
 *
 * Retrieves an EnumDcl from a symbol table and returns a pointer.
 * The caller must not free the returned pointer.  This function will
 * return null if the requested symbol is not an EnumDcl.
 *
 * \sa P_NewSymbolTable(), PST_AddSymTabEntry(),
 * PST_AddEntry(), PST_AddFuncDclEntry(),
 * PST_AddTypeDclEntry(), PST_AddVarDclEntry(),
 * PST_AddStructDclEntry(), PST_AddUnionDclEntry(),
 * PST_AddEnumDclEntry(), PST_AddAsmDclEntry(),
 * PST_AddStmtEntry(), PST_AddExprEntry(),
 * PST_AddFieldEntry(), PST_AddEnumFieldEntry(),
 * PST_AddLabelEntry(), PST_GetEntry(),
 * PST_GetFuncDclEntry(), PST_GetTypeDclEntry(),
 * PST_GetVarDclEntry(), PST_GetStructDclEntry(),
 * PST_GetUnionDclEntry(), PST_GetAsmDclEntry(),
 * PST_GetStmtEntry(), PST_GetExprEntry(),
 * PST_GetFieldEntry(), PST_GetEnumFieldEntry(),
 * PST_GetLabelEntry() */
EnumDcl
PST_GetEnumDclEntry (SymbolTable table, Key key)
{
  _EntryType type;
  EnumDcl enum_dcl;

  type = PST_GetEntry (table, key, (void **)&enum_dcl);

  if (type == ET_ENUM)
    return (enum_dcl);
  else
    return (NULL);
}

/*! \brief Retrieves an AsmDcl from a symbol table.
 *
 * \param table
 *  the symbol table.
 * \param key
 *  the desired AsmDcl's key.
 *
 * \return
 *  A pointer to the AsmDcl.
 *
 * Retrieves an AsmDcl from a symbol table and returns a pointer.  The
 * caller must not free the returned pointer.  This function will
 * return null if the requested symbol is not an AsmDcl.
 *
 * \sa P_NewSymbolTable(), PST_AddSymTabEntry(),
 * PST_AddEntry(), PST_AddFuncDclEntry(),
 * PST_AddTypeDclEntry(), PST_AddVarDclEntry(),
 * PST_AddStructDclEntry(), PST_AddUnionDclEntry(),
 * PST_AddEnumDclEntry(), PST_AddAsmDclEntry(),
 * PST_AddStmtEntry(), PST_AddExprEntry(),
 * PST_AddFieldEntry(), PST_AddEnumFieldEntry(),
 * PST_AddLabelEntry(), PST_GetEntry(),
 * PST_GetFuncDclEntry(), PST_GetTypeDclEntry(),
 * PST_GetVarDclEntry(), PST_GetStructDclEntry(),
 * PST_GetUnionDclEntry(), PST_GetEnumDclEntry(),
 * PST_GetStmtEntry(), PST_GetExprEntry(),
 * PST_GetFieldEntry(), PST_GetEnumFieldEntry(),
 * PST_GetLabelEntry() */
AsmDcl
PST_GetAsmDclEntry (SymbolTable table, Key key)
{
  _EntryType type;
  AsmDcl asm_dcl;

  type = PST_GetEntry (table, key, (void **)&asm_dcl);

  if (type == ET_ASM)
    return (asm_dcl);
  else
    return (NULL);
}

/*! \brief Retrieves a Stmt from a symbol table.
 *
 * \param table
 *  the symbol table.
 * \param key
 *  the desired Stmt's key.
 *
 * \return
 *  A pointer to the Stmt.
 *
 * Retrieves a Stmt from a symbol table and returns a pointer.  The
 * caller must not free the returned pointer.  This function will
 * return null if the requested symbol is not a Stmt.
 *
 * \sa P_NewSymbolTable(), PST_AddSymTabEntry(),
 * PST_AddEntry(), PST_AddFuncDclEntry(),
 * PST_AddTypeDclEntry(), PST_AddVarDclEntry(),
 * PST_AddStructDclEntry(), PST_AddUnionDclEntry(),
 * PST_AddEnumDclEntry(), PST_AddAsmDclEntry(),
 * PST_AddStmtEntry(), PST_AddExprEntry(),
 * PST_AddFieldEntry(), PST_AddEnumFieldEntry(),
 * PST_AddLabelEntry(), PST_GetEntry(),
 * PST_GetFuncDclEntry(), PST_GetTypeDclEntry(),
 * PST_GetVarDclEntry(), PST_GetStructDclEntry(),
 * PST_GetEnumDclEntry(), PST_GetAsmDclEntry(),
 * PST_GetExprEntry(), PST_GetFieldEntry(),
 * PST_GetEnumFieldEntry(), PST_GetLabelEntry() */
Stmt
PST_GetStmtEntry (SymbolTable table, Key key)
{
  _EntryType type;
  Stmt stmt;

  type = PST_GetEntry (table, key, (void **)&stmt);

  if (type == ET_STMT)
    return (stmt);
  else
    return (NULL);
}

/*! \brief Retrieves an Expr from a symbol table.
 *
 * \param table
 *  the symbol table.
 * \param key
 *  the desired Expr's key.
 *
 * \return
 *  A pointer to the Expr.
 *
 * Retrieves an Expr from a symbol table and returns a pointer.  The
 * caller must not free the returned pointer.  This function will
 * return null if the requested symbol is not an Expr.
 *
 * \sa P_NewSymbolTable(), PST_AddSymTabEntry(),
 * PST_AddEntry(), PST_AddFuncDclEntry(),
 * PST_AddTypeDclEntry(), PST_AddVarDclEntry(),
 * PST_AddStructDclEntry(), PST_AddUnionDclEntry(),
 * PST_AddEnumDclEntry(), PST_AddAsmDclEntry(),
 * PST_AddStmtEntry(), PST_AddExprEntry(),
 * PST_AddFieldEntry(), PST_AddEnumFieldEntry(),
 * PST_AddLabelEntry(), PST_GetEntry(),
 * PST_GetFuncDclEntry(), PST_GetTypeDclEntry(),
 * PST_GetVarDclEntry(), PST_GetStructDclEntry(),
 * PST_GetUnionDclEntry(), PST_GetEnumDclEntry(),
 * PST_GetAsmDclEntry(), PST_GetStmtEntry(),
 * PST_GetFieldEntry(), PST_GetEnumFieldEntry(),
 * PST_GetLabelEntry() */
Expr
PST_GetExprEntry (SymbolTable table, Key key)
{
  _EntryType type;
  Expr expr;

  type = PST_GetEntry (table, key, (void **)&expr);

  if (type == ET_EXPR)
    return (expr);
  else
    return (NULL);
}

/*! \brief Retrieves a Field from a symbol table.
 *
 * \param table
 *  the symbol table.
 * \param key
 *  the desired Field's key.
 *
 * \return
 *  A pointer to the Field.
 *
 * Retrieves a Field from a symbol table and returns a pointer.  The
 * caller must not free the returned pointer.  This function will
 * return null if the requested symbol is not a Field.
 *
 * \sa P_NewSymbolTable(), PST_AddSymTabEntry(),
 * PST_AddEntry(), PST_AddFuncDclEntry(),
 * PST_AddTypeDclEntry(), PST_AddVarDclEntry(),
 * PST_AddStructDclEntry(), PST_AddUnionDclEntry(),
 * PST_AddEnumDclEntry(), PST_AddAsmDclEntry(),
 * PST_AddStmtEntry(), PST_AddExprEntry(),
 * PST_AddFieldEntry(), PST_AddEnumFieldEntry(),
 * PST_AddLabelEntry(), PST_GetEntry(),
 * PST_GetFuncDclEntry(), PST_GetTypeDclEntry(),
 * PST_GetVarDclEntry(), PST_GetStructDclEntry(),
 * PST_GetUnionDclEntry(), PST_GetEnumDclEntry(),
 * PST_GetAsmDclEntry(), PST_GetStmtEntry(),
 * PST_GetExprEntry(), PST_GetEnumFieldEntry(),
 * PST_GetLabelEntry() */
Field
PST_GetFieldEntry (SymbolTable table, Key key)
{
  _EntryType type;
  Field field;

  type = PST_GetEntry (table, key, (void **)&field);

  if (type == ET_FIELD)
    return (field);
  else
    return (NULL);
}

/*! \brief Retrieves an EnumField from a symbol table.
 *
 * \param table
 *  the symbol table.
 * \param key
 *  the desired EnumField's key.
 *
 * \return
 *  A pointer to the EnumField.
 *
 * Retrieves an EnumField from a symbol table and returns a pointer.
 * The caller must not free the returned pointer.  This function will
 * return null if the requested symbol is not an EnumField.
 *
 * \sa P_NewSymbolTable(), PST_AddSymTabEntry(),
 * PST_AddEntry(), PST_AddFuncDclEntry(),
 * PST_AddTypeDclEntry(), PST_AddVarDclEntry(),
 * PST_AddStructDclEntry(), PST_AddUnionDclEntry(),
 * PST_AddEnumDclEntry(), PST_AddAsmDclEntry(),
 * PST_AddStmtEntry(), PST_AddExprEntry(),
 * PST_AddFieldEntry(), PST_AddEnumFieldEntry(),
 * PST_AddLabelEntry(), PST_GetEntry(),
 * PST_GetFuncDclEntry(), PST_GetTypeDclEntry(),
 * PST_GetVarDclEntry(), PST_GetStructDclEntry(),
 * PST_GetUnionDclEntry(), PST_GetEnumDclEntry(),
 * PST_GetAsmDclEntry(), PST_GetStmtEntry(),
 * PST_GetExprEntry(), PST_GetFieldEntry(),
 * PST_GetLabelEntry() */
EnumField
PST_GetEnumFieldEntry (SymbolTable table, Key key)
{
  _EntryType type;
  EnumField enum_field;

  type = PST_GetEntry (table, key, (void **)&enum_field);

  if (type == ET_ENUMFIELD)
    return (enum_field);
  else
    return (NULL);
}

/*! \brief Retrieves a Label from a symbol table.
 *
 * \param table
 *  the symbol table.
 * \param key
 *  the desired Label's key.
 *
 * \return
 *  A pointer to the Label.
 *
 * Retrieves a Label from a symbol table and returns a pointer.  The
 * caller must not free the returned pointer.  This function will
 * return null if the requested symbol is not a Label.
 *
 * \sa P_NewSymbolTable(), PST_AddSymTabEntry(),
 * PST_AddEntry(), PST_AddFuncDclEntry(),
 * PST_AddTypeDclEntry(), PST_AddVarDclEntry(),
 * PST_AddStructDclEntry(), PST_AddUnionDclEntry(),
 * PST_AddEnumDclEntry(), PST_AddAsmDclEntry(),
 * PST_AddStmtEntry(), PST_AddExprEntry(),
 * PST_AddFieldEntry(), PST_AddEnumFieldEntry(),
 * PST_AddLabelEntry(), PST_GetEntry(),
 * PST_GetFuncDclEntry(), PST_GetTypeDclEntry(),
 * PST_GetVarDclEntry(), PST_GetStructDclEntry(),
 * PST_GetUnionDclEntry(), PST_GetEnumDclEntry(),
 * PST_GetAsmDclEntry(), PST_GetStmtEntry(),
 * PST_GetExprEntry(), PST_GetFieldEntry(),
 * PST_GetEnumFieldEntry() */
Label
PST_GetLabelEntry (SymbolTable table, Key key)
{
  _EntryType type;
  Label label;

  type = PST_GetEntry (table, key, (void **)&label);

  if (type == ET_LABEL)
    return (label);
  else
    return (NULL);
}

/*! \brief Links all entries in the table together through next and prev.
 *
 * \param table
 *  the symbol table.
 *
 * Links all entries in the table together through the next and prev fields.
 * This will determine the order in which entries are processed when using
 * the PST_Get*EntryByType[Next] functions.
 *
 * This function sorts the entries so that all types (except pointer types)
 * are defined before they are used.  Pointer types can be defined after
 * use, if necessary to break recursive references.
 *
 * \sa PST_GetFileEntryByType(), PST_GetFileEntryByTypeNext(),
 * PST_GetTableEntryByType(), PST_GetTableEntryByTypeNext()
 */
void
PST_OrderTypeUses (SymbolTable table)
{
  Key k;
  SymTabEntry entry, last = NULL;
  KeyList i, list = NULL;

  PST_ResetOrder (table);

  for (k = PST_GetTableEntryByType (table, ET_ANY); P_ValidKey (k);
       k = PST_GetTableEntryByTypeNext (table, k, ET_ANY))
    {
      entry = PST_GetSymTabEntry (table, k);

      switch (P_GetSymTabEntryType (entry))
	{
	case ET_TYPE_GLOBAL:
	case ET_TYPE_LOCAL:
	case ET_STRUCT:
	case ET_UNION:
	  list = order_types (table, k, list, NULL);
	  break;
	default:
	  list = P_AppendKeyListNext (list, P_NewKeyListWithKey (k));
	  break;
	}
    }

  for (i = list; i; i = P_GetKeyListNext (i))
    {
      entry = PST_GetSymTabEntry (table, P_GetKeyListKey (i));

      if (last)
	{
	  P_SetSymTabEntryNext (last, P_GetSymTabEntryKey (entry));
	  P_SetSymTabEntryPrev (entry, P_GetSymTabEntryKey (last));
	}

      last = entry;
    }

  list = P_RemoveKeyList (list);

  return;
}

/*! \brief Clears the next and prev fields in all SymTabEntries.
 *
 * \param table
 *  the symbol table.
 *
 * Clears the next and prev fields in all SymTabEntries.  This resets
 * the order to the default.
 */
void
PST_ResetOrder (SymbolTable table)
{
  Key k;
  SymTabEntry entry;

  for (k.file = 1; k.file <= table->num_files; k.file++)
    {
      for (k.sym = 1; k.sym <= table->ip_table[k.file]->num_entries; k.sym++)
	{
	  if ((entry = PST_GetSymTabEntry (table, k)))
	    {
	      P_SetSymTabEntryNext (entry, Invalid_Key);
	      P_SetSymTabEntryPrev (entry, Invalid_Key);
	    }
	}
    }

  return;
}

/*! \brief Moves a symbol to a different scope.
 *
 * \param table
 *  the symbol table.
 * \param src_key
 *  the key of the symbol to move.
 * \param dst_scope_key
 *  the destination scope key for the symbol.
 *
 * \return
 *  The new key of the symbol.
 *
 * This function moves a symbol to scope \a dst_scope_key.
 */
Key
PST_MoveEntry (SymbolTable table, Key src_key, Key dst_scope_key)
{
  Key result = Invalid_Key;
  Key scope_key;
  Key new_entry_key = Invalid_Key;
  SymTabEntry entry;

  /* HP's compiler isn't happy with initializing new_entry_key to
   * {dst_scope_key.file, 0}. */
  new_entry_key.file = dst_scope_key.file;

  if ((entry = PST_GetSymTabEntry (table, src_key)))
    {
      /* If the entry is in a scope, remove its old key. */
      scope_key = P_GetSymTabEntryScopeKey (entry);
	
      if (P_ValidKey (scope_key))
	PST_RemoveEntryFromScope (table, scope_key, src_key);

      P_SetSymTabEntryKey (entry, new_entry_key);

      result = PST_AddSymTabEntry (table, entry);

      /* If the entry has a scope, update that scope's key. */
      if (P_GetSymTabEntryScope (entry))
	{
	  Scope scope = P_GetSymTabEntryScope (entry);
	  ScopeEntry se;
	  
	  P_SetScopeKey (scope, result);
	  
	  /* Update all ScopeEntries. */
	  for (se = P_GetScopeScopeEntry (scope); se;
	       se = P_GetScopeEntryNext (se))
	    {
	      PST_AddEntryToScope (table, result, P_GetScopeEntryKey (se));
	    }
	}

      /* Insert the symbol in its new scope. */
      PST_AddEntryToScope (table, dst_scope_key, result);
      
      PST_ClearEntry (table, src_key);
    }

  return (result);
}

/*! \brief Deletes an entry from the symbol table.
 *
 * \param table
 *  the symbol table.
 * \param key
 *  the key of the entry to delete.
 *
 * Deletes entry with key \a key from \a table.  The SymTabEntry and
 * referenced Pcode structure are freed.  A SymTabEntry will remain in
 * the table after calling this function.  This entry contains only the
 * key and the STE_DELETED flag.  This entry is used to remove the deleted
 * entry from the output file if the output file already exists.
 *
 * This function is primarily for symbol table maintenance, not memory
 * deallocation.  Calling this function on a symbol will permanently delete
 * the symbol.  In contrast, calling PST_RemoveEntry() will free the
 * memory allocated for a SymTabEntry.  However, subsequent calls to
 * PST_GetSymTabEntry() will read the symbol into the table again.
 *
 * This function differs from #PST_RemoveEntry() in that it leaves this
 * flagged entry in the table.  #PST_RemoveEntry() frees the entry and
 * leaves a hole.
 *
 * \sa PST_RemoveEntry(), PST_AddSymTabEntry()
 */
void
PST_DeleteEntry (SymbolTable table, Key key)
{
  SymTabEntry deleted = P_NewSymTabEntry ();

  PST_RemoveEntry (table, key);

  P_SetSymTabEntryKey (deleted, key);
  P_SetSymTabEntryFlags (deleted, STE_DELETED);

  PST_AddSymTabEntry (table, deleted);

  return;
}

/*! \brief Checks the symbol table for the existence of a file.
 *
 * \param table
 *  the symbol table.
 * \param key
 *  the key for the file to check.
 *
 * \return
 *  If the file exists in the table, returns 1.  Otherwise returns 0.
 *
 * \sa PST_AddFile(), PST_AddFileWithKey(), PST_GetFileKeyByName(),
 * PST_GetOrAddFileKeyByName()
 */
int
PST_HasFile (SymbolTable table, int key)
{
  if (key > 0 && key <= table->num_files && table->ip_table[key] != NULL)
    return (1);

  return (0);
}

/*!\ brief Adds a file to the symbol table.
 *
 * \param table
 *  the symbol table.
 * \param source_name
 *  the name of the new file's original source code.
 * \param filetype
 *  the type of the new file (FT_SOURCE or FT_HEADER).
 *
 * \return
 *  The file's key.
 *
 * This function adds a new ::IPSymTabEnt to the symbol table for a file
 * with the given name.  If an ::IPSymTabEnt already exists for the file,
 * this function returns the file's key.
 *
 * \sa PST_HasFile(), PST_AddFile(), PST_GetFileKeyByName()
 */
int
PST_AddFile (SymbolTable table, char *source_name, _FileType filetype)
{
  int i, key;

  /* Try to find the file in the table. */
  for (i = 1; i <= table->num_files; i++)
    if (strcmp (source_name, table->ip_table[i]->source_name) == 0)
      return (i);

  /* Add a new IPSymTabEnt for the file.  The key is the first position after
   * the end of the file array. */
  key = PST_AddFileWithKey (table, table->num_files + 1, source_name,
			    filetype);

  return (key);
}

/*! \brief Adds a file to the symbol table.
 *
 * \param table
 *  the symbol table.
 * \param key
 *  the key for the new file.
 * \param source_name
 *  the name of the new file's original source code.  Pass NULL if the name
 *  is unknown.
 * \param filetype
 *  the type of the new file.
 *
 * \return
 *  The file's key.
 *
 * This function adds a new ::IPSymTabEnt to the symbol table for the
 * file with the given key.  If an ::IPSymTabEnt already exists for
 * the given key with the same name as the new file, this function
 * does nothing If an ::IPSymTabEnt already exists for the given key
 * with a different name, this function prints a warning and
 * overwrites the old ::IPSymTabEnt.
 *
 * If the key is past the end of the ::SymbolTable's ip_table array,
 * the ip_table array is realloc()ed.  This isn't very efficient, but
 * it is a very rare case.  After Plink, all files will be known, so
 * the array can be appropriately sized from the start.
 *
 * \note The string passed in for name is copied, so the caller must
 * free its copy separately.
 *
 * \sa PST_HasFile(), PST_AddFile(), PST_GetFileKeyByName()
 */
int
PST_AddFileWithKey (SymbolTable table, int key, char *source_name,
		    _FileType filetype)
{
  int i;

  if (key > table->num_files)
    {
      table->ip_table = realloc (table->ip_table, 
				 (key + 1) * sizeof (_IPSymTabEnt *));
      /* Set all new ip_table entries to null. */
      /* Index 0 is always null. */
      table->ip_table[0] = NULL;
      for (i = table->num_files + 1; i <= key; i++)
	table->ip_table[i] = NULL;

      table->num_files = key;
    }

  if (table->ip_table[key] == NULL)
    {
      /* Allocate a new IPSymTabEnt for the file. */
      table->ip_table[key] = P_NewIPSymTabEnt ();
      table->ip_table[key]->key = key;
      if (source_name)
	table->ip_table[key]->source_name = strdup (source_name);
      table->ip_table[key]->file_type = filetype;
    }
  else
    {
      /* If there's already an entry, the name of the entry and the
       * new file must match. */
      if (!((table->ip_table[key]->source_name && source_name && \
	     strcmp (table->ip_table[key]->source_name, source_name) == 0) || \
	    (table->ip_table[key]->source_name == NULL && \
	     source_name == NULL)))
	{
	  P_warn ("symtab.c:PST_AddFileWithKey:%d file with key %d "
		  "already exists\nold file %s, new file %s", __LINE__ - 1,
		  key, table->ip_table[key]->source_name, source_name);
	}
    }

  return (key);
}

/*! \brief Gets a file's key.
 *
 * \param table
 *  the symbol table.
 * \param name
 *  the file's name.
 * \param which
 *  determines which file name we're searching for: IP_NAME_SOURCE,
 *  IP_NAME_IN, or , IP_NAME_OUT.
 *
 * \return
 *  The file's key.
 *
 * Finds the file in the symbol table based on its name and returns
 * the file's key.  If the file doesn't exist, returns 0.
 *
 * \sa PST_HasFile(), PST_AddFile(), PST_AddFileWithKey()
 */
int
PST_GetFileKeyByName (SymbolTable table, char *name, int which)
{
  int i;

  for (i = 1; i <= table->num_files; i++)
    {
      switch (which)
	{
	case IP_NAME_SOURCE:
	  if (strcmp (name, table->ip_table[i]->source_name) == 0)
	    return (i);
	  break;
	case IP_NAME_IN:
	  if (strcmp (name, table->ip_table[i]->in_name) == 0)
	    return (i);
	  break;
	case IP_NAME_OUT:
	  if (strcmp (name, table->ip_table[i]->out_name) == 0)
	    return (i);
	  break;
	default:
	  P_punt ("symtab.c:PST_GetFileKeyByName:%d Unknown file type %d",
		  __LINE__ - 1, which);
	}
    }

  /* The file doesn't exist in the table. */
  return (0);
}

/*! \brief Gets a file's IPSymTabEnt.
 *
 * \param table
 *  the symbol table.
 * \param key
 *  the file's key.
 *
 * \return
 *  A pointer to the IPSymTabEnt for the file.  The user must not free this
 *  pointer.
 *
 * \sa PST_HasFile(), PST_AddFile(), PST_AddFileWithKey(),
 * PST_GetFileKeyByName()
 */
IPSymTabEnt
PST_GetFileByKey (SymbolTable table, int key)
{
  IPSymTabEnt result = NULL;

  if (key <= table->num_files)
    result = table->ip_table[key];
  else
    P_punt ("symtab.c:PST_GetFileByKey:%d invalid key %d", __LINE__, key);

  return (result);
}

/*! \brief Sets the file handle for a file.
 *
 * \param table
 *  the symbol table.
 * \param file_key
 *  the file's key.
 * \param f
 *  the file handle to set.
 *
 * \return
 *  The new file handle (\a f).
 *
 * Sets the file handle for a file.  If the file is embedded in the symbol
 * table, the SymbolTable.file field is set.
 */
FILE *
PST_SetFileFile (SymbolTable table, int file_key, FILE *f)
{
  if (file_key == 0 || (PST_GetFileFlags (table, file_key) & IPSTEF_EMBEDDED))
    P_SetSymbolTableFile (table, f);
  else
    P_SetIPSymTabEntFile (PST_GetFile (table, file_key), f);

  return (f);
}

/*! \brief Gets the file handle for a file.
 *
 * \param table
 *  the symbol table.
 * \param file_key
 *  the file's key.
 *
 * \return
 *  The file's file handle (\a f).
 *
 * Gets the file handle for a file.  If the file is embedded in the symbol
 * table, the SymbolTable.file field is returned.
 */
FILE *
PST_GetFileFile (SymbolTable table, int file_key)
{
  if (file_key == 0 || (PST_GetFileFlags (table, file_key) & IPSTEF_EMBEDDED))
    return (P_GetSymbolTableFile (table));
  else
    return (P_GetIPSymTabEntFile (PST_GetFile (table, file_key)));
}

/*! \brief Sets the input file status for a file.
 *
 * \param table
 *  the symbol table.
 * \param file_key
 *  the file's key.
 * \param f
 *  the file status to set.
 *
 * \return
 *  The new input file status (\a f).
 *
 * Sets the input file status for a file.  If the file is embedded in the
 * symbol table, the SymbolTable.in_file_status field is set.
 */
_FileStatus
PST_SetFileInFileStatus (SymbolTable table, int file_key, _FileStatus f)
{
  if (file_key == 0 || (PST_GetFileFlags (table, file_key) & IPSTEF_EMBEDDED))
    P_SetSymbolTableInFileStatus (table, f);
  else
    P_SetIPSymTabEntInFileStatus (PST_GetFile (table, file_key), f);

  return (f);
}

/*! \brief Gets the input file status for a file.
 *
 * \param table
 *  the symbol table.
 * \param file_key
 *  the file's key.
 *
 * \return
 *  The file's input file status (\a f).
 *
 * Gets the input file status for a file.  If the file is embedded in the
 * symbol table, the SymbolTable.in_file_status field is returned.
 */
_FileStatus
PST_GetFileInFileStatus (SymbolTable table, int file_key)
{
  if (file_key == 0 || (PST_GetFileFlags (table, file_key) & IPSTEF_EMBEDDED))
    return (P_GetSymbolTableInFileStatus (table));
  else
    return (P_GetIPSymTabEntInFileStatus (PST_GetFile (table, file_key)));
}

/*! \brief Sets the output file status for a file.
 *
 * \param table
 *  the symbol table.
 * \param file_key
 *  the file's key.
 * \param f
 *  the output file status to set.
 *
 * \return
 *  The new output file status (\a f).
 *
 * Sets the output file status for a file.  If the file is embedded in the
 * symbol table, the SymbolTable.out_file_status field is set.
 */
_FileStatus
PST_SetFileOutFileStatus (SymbolTable table, int file_key, _FileStatus f)
{
  if (file_key == 0 || (PST_GetFileFlags (table, file_key) & IPSTEF_EMBEDDED))
    P_SetSymbolTableOutFileStatus (table, f);
  else
    P_SetIPSymTabEntOutFileStatus (PST_GetFile (table, file_key), f);

  return (f);
}

/*! \brief Gets the output file status for a file.
 *
 * \param table
 *  the symbol table.
 * \param file_key
 *  the file's key.
 *
 * \return
 *  The file's output file status (\a f).
 *
 * Gets the output file status for a file.  If the file is embedded in the
 * symbol table, the SymbolTable.out_file_status field is returned.
 */
_FileStatus
PST_GetFileOutFileStatus (SymbolTable table, int file_key)
{
  if (file_key == 0 || (PST_GetFileFlags (table, file_key) & IPSTEF_EMBEDDED))
    return (P_GetSymbolTableOutFileStatus (table));
  else
    return (P_GetIPSymTabEntOutFileStatus (PST_GetFile (table, file_key)));
}

/*! \brief Adds a Scope to the symbol table.
 *
 * \param table
 *  the symbol table.
 * \param key
 *  the scope's key.
 *
 * \return
 *  The key of the newly inserted scope.
 *
 * Adds a Scope to the symbol table.  \a key must have a valid file
 * key (key.file).  If \a key has a valid symbol key (key.sym), the
 * scope will be inserted with that key.  If there is already a table
 * entry with that key, a scope will be added to that entry.  If \n
 * key does not have a valid symbol key (key.sym == 0), the scope
 * will be inserted at the end of the table.  In any case, the key for
 * the new scope is returned to the caller.
 *
 * \sa PST_GetScope() */
Key
PST_AddNewScope (SymbolTable table, Key key)
{
  SymTabEntry scope_table_entry;

  if (key.file == 0)
    {
      P_punt ("symtab.c:PST_AddNewScope:%d key.file is invalid", __LINE__);
    }
  else
    {
      /* If a table entry already exists for this key, add a scope. */
      if ((key.sym > 0) && \
	  (scope_table_entry = PST_GetSymTabEntry (table, key)))
	{
	  if (scope_table_entry->scope == NULL)
	    P_SetSymTabEntryScope (scope_table_entry, P_NewScopeWithKey (key));
	}
      else
	{
	  scope_table_entry = P_NewSymTabEntry ();

	  P_SetSymTabEntryKey (scope_table_entry, key);
	  P_SetSymTabEntryType (scope_table_entry, ET_SCOPE);

	  /* PST_AddSymTabEntry may update the key (if key.sym == 0),
	   * so update the key. */
	  key = PST_AddSymTabEntry (table, scope_table_entry);
#if 0
	  P_SetSymTabEntryScopeKey (scope_table_entry, key);
#endif
	  P_SetSymTabEntryScope (scope_table_entry, P_NewScopeWithKey (key));
	}
    }

  return (key);
}

/*! \brief Adds a table entry to a scope
 *
 * \param table
 *  the symbol table.
 * \param scope_key
 *  the scope to recieve the entry.
 * \param entry_key
 *  the entry to add to the scope.
 *
 * \sa PST_RemoveEntryFromScope()
 *
 * This function adds a table entry to a scope.  If that entry already
 * exists in another scope, it is removed from the other one.
 */
void
PST_AddEntryToScope (SymbolTable table, Key scope_key, Key entry_key)
{
  Scope scope;
  SymTabEntry table_entry;

  if ((table_entry = PST_GetSymTabEntry (table, entry_key)))
    if (P_ValidKey (P_GetSymTabEntryScopeKey (table_entry)))
      PST_RemoveEntryFromScope (table, P_GetSymTabEntryScopeKey (table_entry),
				entry_key);

  scope = PST_GetScope (table, scope_key);

  P_AppendScopeScopeEntry (scope, entry_key);

  /* We just added a table entry to a scope.  Now set the scope field in that
   * table entry. */
  if (table_entry)
    P_SetSymTabEntryScopeKey (table_entry, scope_key);

  return;
}

/*! \brief Removes a table entry from a scope
 *
 * \param table
 *  the symbol table.
 * \param scope_key
 *  the scope containing the entry.
 * \param entry_key
 *  the entry to remove from the scope.
 *
 * Removes \a entry_key from the scope's ScopeEntry list.  If the SymTabEntry
 * corresponding to \a entry_key has \a scope_key in its scope_key field,
 * this field is set to {0, 0}.
 *
 * \sa PST_AddEntryToScope()
 */
void
PST_RemoveEntryFromScope (SymbolTable table, Key scope_key, Key entry_key)
{
  Scope scope;
  SymTabEntry entry;
  
  if ((scope = PST_GetScope (table, scope_key)) && \
      P_FindScopeEntryKey (P_GetScopeScopeEntry (scope), entry_key))
    {
      P_SetScopeScopeEntry \
	(scope,
	 P_RemoveScopeEntryNextByKey (P_GetScopeScopeEntry (scope),
				      entry_key));

      entry = PST_GetSymTabEntry (table, entry_key);
	  
      if (entry && P_MatchKey (P_GetSymTabEntryScopeKey (entry), scope_key))
	P_SetSymTabEntryScopeKey (entry, Invalid_Key);
    }
      
  return;
}

/*! \brief Returns the scope from its key.
 *
 * \param table
 *  the symbol table.
 * \param key
 *  the desired scope's key.
 *
 * \return
 *  The Scope stored in the table entry.  This pointer must not be freed
 *  by the caller.
 *
 * Returns the Scope stored in the table entry with the given key.  If
 * no scope exists, returns null.
 *
 * \sa PST_AddNewScope(), PST_GetParentScope() */
Scope
PST_GetScope (SymbolTable table, Key key)
{
  Scope result = NULL;
  SymTabEntry table_entry;

  if ((table_entry = PST_GetSymTabEntry (table, key)))
    result = table_entry->scope;

  return (result);
}

/*! \brief Returns the scope under which a symbol is defined.
 *
 * \param table
 *  the symbol table
 * \param key
 *  the symbol's key.
 *
 * \return
 *  The key for the scope under which the symbol is defined.
 *
 * \sa PST_GetScope()
 */
Key
PST_GetParentScope (SymbolTable table, Key key)
{
  SymTabEntry entry;
  Key result = Invalid_Key;

  if ((entry = PST_GetSymTabEntry (table, key)))
    result = P_GetSymTabEntryScopeKey (entry);

  return (result);
}

/*! \brief Returns the key of the first entry in a file.
 *
 * \param table
 *  the symbol table.
 * \param file_key
 *  the file's key.
 *
 * \return
 *  The key of the first symbol in \a file.
 *
 * \sa get_file_entry_next()
 */
static Key
get_file_entry (SymbolTable table, int file_key)
{
  Key key, result = Invalid_Key;
  SymTabEntry entry;

  key.file = file_key;
  for (key.sym = 1; key.sym <= PST_GetFileNumEntries (table, key.file);
       key.sym++)
    {
      if ((entry = PST_GetSymTabEntry (table, key)))
	{
	  result = key;
	  goto done;
	}
    }

 done:
  return (result);
}

/*! \brief Returns the key of the next entry in a file.
 *
 * \param table
 *  the symbol table.
 * \param last
 *  the key of the previous entry in the table.
 *
 * \return
 *  the key of the entry following \a last in the current order.
 *
 * \sa get_file_entry()
 */
static Key
get_file_entry_next (SymbolTable table, Key last)
{
  Key result = Invalid_Key;

  for (result = get_table_entry_next (table, last);
       P_ValidKey (result) && result.file != last.file;
       result = get_table_entry_next (table, result));

  return (result);
}

/*! \brief Returns the first entry in memory defined under a file.
 *
 * \param table
 *  the symbol table.
 * \param file_key
 *  the file's key.
 *
 * \return
 *  The key of the first entry already in memory defined under file
 *  \a file_key.
 *
 * This function returns the key of the first SymTabEntry already in memory.
 *
 * \sa PST_GetFileEntryFromMemNext()
 */
Key
PST_GetFileEntryFromMem (SymbolTable table, int file_key)
{
  Key key, result = Invalid_Key;
  SymTabEntry entry;

  key.file = file_key;
  for (key.sym = 1; key.sym <= PST_GetFileNumEntries (table, key.file);
       key.sym++)
    {
      if ((entry = PST_GetSymTabEntryFromMem (table, key)))
	{
	  result = key;
	  goto done;
	}
    }

 done:
  return (result);
}

/*! \brief Returns the next entry in memory defined under a file.
 *
 * \param table
 *  the symbol table.
 * \param last
 *  the key returned by the last call to PST_GetFileEntryFromMem() or
 *  PST_GetFileEntryFromMemNext()
 *
 * \return
 *  The key of the next entry already in memory defined under file
 *  \a file_key.
 *
 * This function returns the key of the next SymTabEntry already in memory.
 *
 * \sa PST_GetFileEntryFromMem()
 */
Key
PST_GetFileEntryFromMemNext (SymbolTable table, Key last)
{
  Key key, result = Invalid_Key;
  SymTabEntry entry;

  key.file = last.file;
  for (key.sym = last.sym + 1;
       key.sym <= PST_GetFileNumEntries (table, key.file); key.sym++)
    {
      if ((entry = PST_GetSymTabEntryFromMem (table, key)))
	{
	  result = key;
	  goto done;
	}
    }

 done:
  return (result);
}

/*! \brief Returns the first entry of a type defined under a file.
 *
 * \param table
 *  the symbol table.
 * \param file_key
 *  the file's key.
 * \param type
 *  the entry type to find.
 *
 * \return
 *  The key for the first entry of type \a type defined under file \a file_key.
 *  If no entry of type \a type exists, returns an invalid key ({0, 0}).
 *  This pointer must not be freed by the caller.
 *
 * \sa PST_GetFileEntryByTypeNext(), PST_GetScopeEntryByType(),
 * PST_GetScopeEntryByTypeNext()
 */
Key
PST_GetFileEntryByType (SymbolTable table, int file_key, _EntryType type)
{
  SymTabEntry entry;
  Key key, result = Invalid_Key;

  for (key = get_file_entry (table, file_key); P_ValidKey (key);
       key = get_file_entry_next (table, key))
    {
      entry = PST_GetSymTabEntry (table, key);

      if (P_GetSymTabEntryType (entry) & type)
	{
	  result = key;
	  goto done;
	}
    }

 done:
  return (result);
}

/*! \brief Returns the next entry of a type defined under a file.
 *
 * \param table
 *  the symbol table.
 * \param last
 *  the key returned by the last call to PST_GetFileEntryByType() or
 *  PST_GetFileEntryByTypeNext().
 * \param type
 *  the entry type to find.
 *
 * \return
 *  The key for the next entry of type \a type defined under file \a file_key.
 *  If no entry of type \a type exists, returns an invalid key ({0, 0}).
 *  This pointer must not be freed by the caller.
 *
 * \sa PST_GetFileEntryByType(), PST_GetScopeEntryByType(),
 * PST_GetScopeEntryByTypeNext()
 */
Key
PST_GetFileEntryByTypeNext (SymbolTable table, Key last, _EntryType type)
{
  SymTabEntry entry;
  Key key, result = Invalid_Key;

  for (key = get_file_entry_next (table, last); P_ValidKey (key);
       key = get_file_entry_next (table, key))
    {
      entry = PST_GetSymTabEntry (table, key);

      if (P_GetSymTabEntryType (entry) & type)
	{
	  result = key;
	  goto done;
	}
    }

 done:
  return (result);
}

/*! \brief Returns the key of the first entry in a symbol table.
 *
 * \param table
 *  the symbol table.
 *
 * \return
 *  the key of the frist entry in the table.
 *
 * \sa get_table_entry_next()
 */
static Key
get_table_entry (SymbolTable table)
{
  Key key, result = Invalid_Key;
  SymTabEntry entry;

  /* Find the first entry in the table. */
  for (key.file = 1; key.file <= P_GetSymbolTableNumFiles (table); key.file++)
    {
      for (key.sym = 1; key.sym <= PST_GetFileNumEntries (table, key.file);
	   key.sym++)
	{
	  if ((entry = PST_GetSymTabEntry (table, key)))
	    {
	      result = key;
	      goto done;
	    }
	}
    }

 done:
  return (result);
}

/*! \brief Returns the key of the next entry in a symbol table.
 *
 * \param table
 *  the symbol table.
 * \param last
 *  the key of the previous entry in the table.
 *
 * \return
 *  the key of the entry following \a last in the current order.
 *
 * \sa get_table_entry()
 */
static Key
get_table_entry_next (SymbolTable table, Key last)
{
  Key key, result = Invalid_Key;
  SymTabEntry entry;

  /* If we can't find the last symbol, increment the symbol key to find
   * the next symbol which is defined. */
  entry = PST_GetSymTabEntry (table, last);

  if (entry && P_ValidKey (P_GetSymTabEntryNext (entry)))
    {   
      result = P_GetSymTabEntryNext (entry);
    }
  else
    {
      for (key.file = last.file; key.file <= P_GetSymbolTableNumFiles (table);
	   key.file++)
	{
	  /* When we're finishing last's file, start with sym last + 1.
	   * For the rest of the files, start with sym 1. */
	  for (key.sym = key.file == last.file ? last.sym + 1 : 1;
	       key.sym <= PST_GetFileNumEntries (table, key.file); key.sym++)
	    {
	      if ((entry = PST_GetSymTabEntry (table, key)))
		{
		  result = key;
		  goto done;
		}
	    }
	}
    }

 done:
  return (result);
}

/*! \brief Returns the first entry in memory.
 *
 * \param table
 *  the symbol table.
 *
 * \return
 *  The key of the first entry already in memory.
 *
 * \sa PST_GetTableEntryFromMemNext()
 */
Key
PST_GetTableEntryFromMem (SymbolTable table)
{
  Key key, result = Invalid_Key;
  SymTabEntry entry;

  for (key.file = 1; key.file <= P_GetSymbolTableNumFiles (table); key.file++)
    {
      for (key.sym = 1; key.sym <= PST_GetFileNumEntries (table, key.file);
	   key.sym++)
	{
	  if ((entry = PST_GetSymTabEntryFromMem (table, key)))
	    {
	      result = key;
	      goto done;
	    }
	}
    }

 done:
  return (result);
}

/*! \brief Returns the next entry in memory.
 *
 * \param table
 *  the symbol table.
 * \param last
 *  the key returned by the last call to PST_GetTableEntryFromMem() or
 *  PST_GetTableEntryFromMemNext()
 *
 * \return
 *  The key of the next entry already in memory.
 *
 * \sa PST_GetTableEntryFromMem()
 */
Key
PST_GetTableEntryFromMemNext (SymbolTable table, Key last)
{
  Key key, result = Invalid_Key;
  SymTabEntry entry;

  for (key.file = last.file; key.file <= P_GetSymbolTableNumFiles (table);
       key.file++)
    {
      /* When we're finishing last's file, start with sym last + 1.
       * For the rest of te files, start with sym 1. */
      for (key.sym = key.file == last.file ? last.sym + 1 : 1;
	   key.sym <= PST_GetFileNumEntries (table, key.file); key.sym++)
	{
	  if ((entry = PST_GetSymTabEntryFromMem (table, key)))
	    {
	      result = key;
	      goto done;
	    }
	}
    }

 done:
  return (result);
}

/*! \brief Returns the first entry of a type in a symbol table.
 *
 * \param table
 *  the symbol table.
 * \param type
 *  the type of symbol to return.
 *
 * \return
 *  The key of the first entry of type \a type in \a table.
 *
 * \sa PST_GetTableEntryByTypeNext()
 */
Key
PST_GetTableEntryByType (SymbolTable table, _EntryType type)
{
  SymTabEntry entry;
  Key key, result = Invalid_Key;

  for (key = get_table_entry (table); P_ValidKey (key);
       key = get_table_entry_next (table, key))
    {
      entry = PST_GetSymTabEntry (table, key);

      if (P_GetSymTabEntryType (entry) & type)
	{
	  result = key;
	  goto done;
	}
    }

 done:
  return (result);
}

/*! \brief Returns the next entry of a type in a symbol table.
 *
 * \param table
 *  the symbol table.
 * \param last
 *  the key of the last entry returned.
 * \param type
 *  the type of symbol to return.
 *
 * \return
 *  The key of the next entry of type \a type in \a table.
 *
 * \note This function must be able to retrieve the SymTabEntry with key
 *       \a last.  Care must be taken if it is necessary to remove
 *       \a last.
 *
 * \sa PST_GetTableEntryByType()
 */
Key
PST_GetTableEntryByTypeNext (SymbolTable table, Key last, _EntryType type)
{
  SymTabEntry entry;
  Key key, result = Invalid_Key;

  for (key = get_table_entry_next (table, last); P_ValidKey (key);
       key = get_table_entry_next (table, key))
    {
      entry = PST_GetSymTabEntry (table, key);

      if (P_GetSymTabEntryType (entry) & type)
	{
	  result = key;
	  goto done;
	}
    }

 done:
  return (result);
}

/*! \brief Returns the first entry of a type defined under a scope.
 *
 * \param table
 *  the symbol table.
 * \param scope_key
 *  the scope's key.
 * \param type
 *  the entry type to find.
 *
 * \return
 *  A ScopeEntry for the first entry of type \a type defined under the ::Scope.
 *  This pointer must not be freed by the caller.
 *
 * Returns a ScopeEntry for the first entry of type \a type defined under a
 * ::Scope.  The ScopeEntry structure contains the key for the symbol.
 * If no symbol is found, returns null.
 *
 * \sa PST_GetScope(), PST_GetScopeEntryByTypeNext(), PST_GetFileEntryByType(),
 * PST_GetFileEntryByTypeNext()
 */
ScopeEntry
PST_GetScopeEntryByType (SymbolTable table, Key scope_key, _EntryType type)
{
  Scope scope;
  ScopeEntry se = NULL;
  SymTabEntry table_entry;

  if ((scope = PST_GetScope (table, scope_key)))
    {
      for (se = P_GetScopeScopeEntry (scope); se;
	   se = P_GetScopeEntryNext (se))
	{
	  if ((table_entry = PST_GetSymTabEntry (table, se->key)))
	    {
	      if (table_entry != NULL && \
		  (P_GetSymTabEntryType (table_entry) & type))
		break;
	      else if (table_entry == NULL) /* Shouldn't happen */
		P_warn ("symtab.c:PST_GetScopeEntryByType:%d\nScope (%d, %d) "
			"has null entry. key=(%d, %d)", __LINE__ - 1,
			scope_key.file, scope_key.sym, se->key.file,
			se->key.sym);
	    }
	}
    }

  return (se);
}

/*! \brief Returns the next entry of a type defined under a scope.
 *
 * \param table
 *  the symbol table.
 * \param last
 *  a ScopeEntry from the scope to search.
 * \param type
 *  the entry type to find.
 *
 * \return
 *  A ScopeEntry for the next entry of type \a type defined under a ::Scope.
 *  This pointer must not be freed by the caller.
 *
 * Returns a ScopeEntry for the next entry of type \a type after the one
 * represented by \a last.  If the ScopeEntry refers to the last entry
 * defined in the ::Scope, this function returns null.
 *
 * \sa PST_GetScope(), PST_GetScopeEntryByType(), PST_GetFileEntryByType(),
 * PST_GetFileEntryByTypeNext()
 */
ScopeEntry
PST_GetScopeEntryByTypeNext (SymbolTable table, ScopeEntry last,
			     _EntryType type)
{
  SymTabEntry table_entry;
  
  if (last)
    {
      while ((last = P_GetScopeEntryNext (last)))
	{
	  if ((table_entry = PST_GetSymTabEntry (table,
						 P_GetSymTabEntryKey (last))))
	    {
	      if (table_entry != NULL && \
		  (P_GetSymTabEntryType (table_entry) & type))
		break;
	      else if (table_entry == NULL) /* Shouldn't happen */
		P_warn ("symtab.c:PST_GetScopeEntryByTypeNext:%d\nScope has "
			"null entry: key=(%d, %d)", __LINE__ - 1,
			last->key.file, last->key.sym);
	    }
	}
    }

  return (last);
}

/*! \brief Returns the first entry of a type defined in a scope (recursive).
 *
 * \param table
 *  the symbol table.
 * \param scope_key
 *  the scope's key.
 * \param type
 *  the entry type to find.
 *
 * \return
 *  A ScopeEntry for the first entry of type \a type defined in the ::Scope.
 *  This pointer must not be freed by the caller.
 *
 * Returns a ScopeEntry for the first entry of type \a type defined in
 * a \a scope_key and \a scope_entry's parents.  The ScopeEntry structure
 * contains the key for the symbol. If no symbol is found, returns null. 
 *
 * \sa PST_GetScopeEntryByTypeRNext()
 */
ScopeEntry
PST_GetScopeEntryByTypeR (SymbolTable table, Key scope_key, _EntryType type)
{
  ScopeEntry result = NULL;

  while (P_ValidKey (scope_key) && result == NULL)
    {
      result = PST_GetScopeEntryByType (table, scope_key, type);
      scope_key = PST_GetScopeFromEntryKey (table, scope_key);
    }

  return (result);
}

/*! \brief Returns the next entyr of a type defined in a scope (recursive).
 *
 * \param table
 *  the symbol table.
 * \param last
 *  a ScopeEntry from the scope to search.
 * \param type
 *  the entry type to find.
 *
 * \return
 *  A ScopeEntry for the next entry of type \a type defined in \a last's
 *  scope and parent scopes.  This pointer must not be freed by the caller.
 *
 * Returns a ScopeEntry for the next entry of type \a type after the one
 * represented by \a last.  If the ScopeEntry refers to the last entry
 * defined in \a last's scope or parent scopes, this function returns null.
 *
 * \sa PST_GetScopeEntryByTypeR()
 */
ScopeEntry
PST_GetScopeEntryByTypeRNext (SymbolTable table, ScopeEntry last,
			      _EntryType type)
{
  ScopeEntry result = NULL;
  Key parent_scope;

  if (!(result = PST_GetScopeEntryByTypeNext (table, last, type)))
    {
      /* No more entries in last's scope, so try last's scope's parent. */
      /* Get last's scope. */
      parent_scope = PST_GetScopeFromEntryKey (table,
					       P_GetScopeEntryKey (last));
      /* Get last's scope's parent. */
      parent_scope = PST_GetScopeFromEntryKey (table, parent_scope);
      result = PST_GetScopeEntryByTypeR (table, parent_scope, type);
    }

  return (result);
}

/*! \brief Finds the key for a table entry given the entry name.
 *
 * \param table
 *  the symbol table.
 * \param scope_key
 *  the scope to search.
 * \param name
 *  the name to find.
 * \param type
 *  the type of entry to find.
 *
 * \return
 *  The entry's key.
 *
 * Searches only the scope \a scope_key for an entry with entry type
 * \a type and name \a name.  If found, returns the entry's key.
 * Otherwise, returns the invalid key ({0, 0}).
 *
 * \sa PST_ScopeFindByNameNext(), PST_ScopeFindByNameR(),
 * PST_ScopeFindByNameRNext()  */
Key
PST_ScopeFindByName (SymbolTable table, Key scope_key, char *name,
		     _EntryType type)
{
  ScopeEntry se;
  Key result = Invalid_Key;
  SymTabEntry entry;

  for (se = PST_GetScopeEntryByType (table, scope_key, type); se;
       se = PST_GetScopeEntryByTypeNext (table, se, type))
    {
      entry = PST_GetSymTabEntry (table, P_GetScopeEntryKey (se));

      if (entry != NULL && P_GetSymTabEntryName (entry) && \
	  strcmp (P_GetSymTabEntryName (entry), name) == 0)
	{
	  break;
	}
      else if (entry == NULL) /* Shouldn't happen */
	{
	  P_punt ("symtab.c:PST_ScopeFindByName:%d\nScope (%d, %d) "
		  " has null entry: key=(%d, %d)", __LINE__ - 1,
		  scope_key.file, scope_key.sym, se->key.file, se->key.sym);
	}
    }

  if (se)
    result = se->key;

  return (result);
}

/*! \brief Finds the key for the next table entry given the entry name.
 *
 * \param table
 *  the symbol table.
 * \param scope_key
 *  the scope to search.
 * \param last
 *  the key of the last entry with the desired name.
 * \param type
 *  the type of entry to find.
 *
 * \return
 *  The entry's key.
 *
 * Searches only the scope \a scope_key for the next entry with
 * entry type \a type and the same name as entry \a last.  If found,
 * returns the entry's key. Otherwise, returns the invalid key ({0, 0}).
 *
 * Structs have both and ET_TYPE and ET_STRUCT entry.  The ET_TYPE
 * entry carries the struct's name, but it's also possible to have a
 * typedef with the same name to a different type, so there can easily
 * be more than one ET_TYPE with the same name.
 *
 * \sa PST_ScopeFindByName(), PST_ScopeFindByNameR(),
 * PST_ScopeFindByNameRNext()  */
Key
PST_ScopeFindByNameNext (SymbolTable table, Key scope_key, Key last,
			 _EntryType type)
{
  ScopeEntry se;
  Key result = Invalid_Key;
  SymTabEntry entry;
  char *name;
  bool last_found = FALSE;

  name = P_GetTypeDclName (PST_GetTypeDclEntry (table, last));

  /* Walking through the entire list to pick up in the middle is somewhat
   * ugly, but then, so is walking the entire table and strcmp()ing entry
   * names.  Fortunately, this function shouldn't be needed after Psymtab. */
  for (se = PST_GetScopeEntryByType (table, scope_key, type); se;
       se = PST_GetScopeEntryByTypeNext (table, se, type))
    {
      if (!last_found)
	{
	  if (P_MatchKey (se->key, last))
	    last_found = TRUE;

	  continue;
	}

      entry = PST_GetSymTabEntry (table, P_GetScopeEntryKey (se));

      if (entry != NULL && P_GetSymTabEntryName (entry) && \
	  strcmp (P_GetSymTabEntryName (entry), name) == 0)
	{
	  break;
	}
      else if (entry == NULL) /* Shouldn't happen */
	{
	  P_punt ("symtab.c:PST_ScopeFindByName:%d\nScope (%d, %d) "
		  " has null entry: key=(%d, %d)", __LINE__ - 1,
		  scope_key.file, scope_key.sym, se->key.file, se->key.sym);
	}
    }

  if (se)
    result = se->key;

  return (result);
}

/*! \brief Finds the key for the table entry given the entry name.
 *
 * \param table
 *  the symbol table.
 * \param scope_key
 *  the scope in which the variable is referenced.
 * \param name
 *  the name to find
 * \param type
 *  the type of entry to find.
 *
 * Searches up the scope stack recusively for the closest definition
 * of an entry with the given name and type.  If found, returns the
 * entry's key.  Otherwise, returns an invalid key ({0, 0}).
 *
 * \sa PST_ScopeFindByNameRNext(), PST_ScopeFindByName(),
 * PST_ScopeFindByNameNext() */
Key
PST_ScopeFindByNameR (SymbolTable table, Key scope_key, char *name,
		      _EntryType type)
{
  Key current_scope_key;
  Key result = Invalid_Key;
  
  /* Search up the scope stack for the var. */
  for (current_scope_key = scope_key;
       P_ValidKey (current_scope_key) && !P_ValidKey (result);
       current_scope_key = PST_GetScopeFromEntryKey (table, current_scope_key))
    result = PST_ScopeFindByName (table, current_scope_key, name, type);

  return (result);
}

/*! \brief Finds the key for the next table entry given the entry name.
 *
 * \param table
 *  the symbol table.
 * \param scope_key
 *  the scope in which the variable is referenced.
 * \param last
 *  the key of the last entry with the desired name.
 * \param type
 *  the type of entry to find.
 *
 * Searches up the scope stack recursively for the next entry with
 * entry type \a type and the same name as entry \a last.  If found,
 * returns the entry's key.  Otherwise, returns an invalid key ({0, 0}).
 *
 * Structs have both and ET_TYPE and ET_STRUCT entry.  The ET_TYPE
 * entry carries the struct's name, but it's also possible to have a
 * typedef with the same name to a different type, so there can easily
 * be more than one ET_TYPE with the same name.
 *
 * \sa PST_ScopeFindByNameR(), PST_ScopeFindByName(),
 * PST_ScopeFindByNameNext()  */
Key
PST_ScopeFindByNameRNext (SymbolTable table, Key scope_key, Key last,
			  _EntryType type)
{
  Key current_scope_key;
  Key result = Invalid_Key;
  char *name;
  
  name = P_GetTypeDclName (PST_GetTypeDclEntry (table, last));

  /* Finish searching the current scope. */
  result = PST_ScopeFindByNameNext (table, current_scope_key, last, type);

  /* Move up the scope stack if that didn't find anything. */
  if (!P_ValidKey (result))
    {
      while ((current_scope_key = \
	      PST_GetScopeFromEntryKey (table, current_scope_key),
	      P_ValidKey (current_scope_key)) && !P_ValidKey (result))
	result = PST_ScopeFindByName (table, current_scope_key, name, type);
    }

  return (result);
}

/*! \brief Finds a type in a scope with a given name and basic type.
 *
 * \param table
 *  the symbol table.
 * \param scope_key
 *  the scope to inspect.
 * \param name
 *  the desired type's name.
 * \param bt
 *  the desired type's basic type.
 *
 * \return
 *  The type's key, or Invalid_Key if the type is not found.
 *
 * Searches only the scope \a scope_key for a type with name \a name and
 * basic type \a bt.
 *
 * \sa PST_ScopeFindTypeByNameBasicTypeR()
 */
Key
PST_ScopeFindTypeByNameBasicType (SymbolTable table, Key scope_key, char *name,
				  _BasicType bt)
{
  Key k = Invalid_Key;

  for (k = PST_ScopeFindByName (table, scope_key, name, ET_TYPE);
       P_ValidKey (k) && \
	 !(P_GetTypeDclBasicType (PST_GetTypeDclEntry (table, k)) & bt);
       k = PST_ScopeFindByNameNext (table, scope_key, k, ET_TYPE));

  return (k);
}

/*! \brief Finds a type in a scope with a given name and basic type.
 *
 * \param table
 *  the symbol table.
 * \param scope_key
 *  the scope to inspect.
 * \param name
 *  the desired type's name.
 * \param bt
 *  the desired type's basic type.
 *
 * \return
 *  The type's key, or Invalid_Key if the type is not found.
 *
 * Searches up the scope stack for the closest definition of a type with
 * name \a name and basic type \a bt.
 *
 * \sa PST_ScopeFindTypeByNameBasicType()
 */
Key
PST_ScopeFindTypeByNameBasicTypeR (SymbolTable table, Key scope_key,
				   char *name, _BasicType bt)
{
  Key k = Invalid_Key;

  for (k = PST_ScopeFindByNameR (table, scope_key, name, ET_TYPE);
       P_ValidKey (k) && \
	 !(P_GetTypeDclBasicType (PST_GetTypeDclEntry (table, k)) & bt);
       k = PST_ScopeFindByNameRNext (table, scope_key, k, ET_TYPE));

  return (k);
}

/*! \brief Determines if a key is defined under a scope.
 *
 * \param table
 *  the symbol table.
 * \param scope_key
 *  the scope to inspect.
 * \param key
 *  the key to inspect.
 *
 * \return
 *  If \a key is defined under \a scope, returns TRUE.  Otherwise, returns
 *  FALSE.
 *
 * This function determines if \a key is defined somewhere under \a scope.
 * \a key can be buried several scopes below \a scope.
 */
bool
PST_ScopeContainsKey (SymbolTable table, Key scope_key, Key key)
{
  Key cur_scope;

  for (cur_scope = PST_GetParentScope (table, key); P_ValidKey (cur_scope);
       cur_scope = PST_GetParentScope (table, cur_scope))
    {
      if (P_MatchKey (scope_key, cur_scope))
	return (TRUE);
    }

  return (FALSE);
}

/*! \brief Returns the next available Expr ID for a FuncDcl
 *
 * \param func
 *   The FuncDcl for the function we want the next id from 
 *
 * \return
 *  The next unused Expr ID.
 * 
 * Return FuncDcl->max_expr_id + 1.  Increments FuncDcl->max_expr_id.
 *
 * \sa PST_ScopeNextExprID()
 */
int
PST_FuncDclNextExprID(FuncDcl func)
{
  int result;
  
  result = P_GetFuncDclMaxExprID (func) + 1;
  P_SetFuncDclMaxExprID (func, result);

  return result;
}

/*! \brief Returns the next available Expr ID for a scope.
 *
 * \param table
 *  the symbol table.
 * \param scope_key
 *  the scope under which the Expr is defined.
 *
 * \return
 *  The next unused Expr ID.
 *
 * Finds the enclosing FuncDcl for the scope and returns
 * FuncDcl->max_expr_id + 1.  Increments FuncDcl->max_expr_id.
 *
 * \sa PST_ScopeUpdateExprIDs(), PST_ScopeCopyExpr(),
 * PST_ScopeCopyExprNode(), PST_ScopeCopyExprList(),
 * #PST_UpdateExprIDs(), #PST_CopyExpr(), #PST_CopyExprNode(),
 * #PST_CopyExprList() */
int
PST_ScopeNextExprID (SymbolTable table, Key scope_key)
{
  FuncDcl func;
  Key func_key;
  int result = 0;

  func_key = PST_ScopeFindFuncScope (table, scope_key);

  if (P_ValidKey (func_key))
    {
      func = PST_GetFuncDclEntry (table, func_key);

      result = PST_FuncDclNextExprID(func);
    }

  return (result);
}

#if 0
/*! \brief Returns the first key in a symbol table.
 *
 * \param table
 *  the symbol table.
 *
 * \return
 *  The first key defined in the table.
 */
Key
PST_GetFirstKey (SymbolTable table)
{
  Key result = Invalid_Key;
  int i;

  for (i = 1; i < table->num_files; i++)
    {
      if (table->ip_table[i] != NULL)
	{
	  result.file = i;
	  result.sym = 1;
	  break;
	}
    }

  return (result);
}
#endif

/*! \brief Prints the keys of the symbol table according to the current order.
 *
 * \param table
 *  the symbol table.
 * \param file
 *  the file to dump (0 for the entire table).
 *
 * Prints the keys of a file or the entire table to stderr in the current
 * order.  This is intended for debugging ordering functions.
 */
void
PST_DumpSymbolTableOrder (SymbolTable table, int file)
{
  Key k;

  if (file == 0)
    for (k = PST_GetTableEntryByType (table, ET_ANY); P_ValidKey (k);
	 k = PST_GetTableEntryByTypeNext (table, k, ET_ANY))
      fprintf (stderr, "(%d, %d)\n", k.file, k.sym);
  else
    for (k = PST_GetFileEntryByType (table, file, ET_ANY); P_ValidKey (k);
	 k = PST_GetFileEntryByTypeNext (table, k, ET_ANY))
      fprintf (stderr, "(%d, %d)\n", k.file, k.sym);

  return;
}

/*! \brief Prints the contents of a symbol table.
 *
 * \param out
 *  the file to write.
 * \param table
 *  the symbol table.
 *
 * Prints the contents of a symbol table.
 */
void
P_DumpSymbolTable (FILE *out, SymbolTable table)
{
  int i;

  if (table)
    {
      if (table->ip_table_name)
	fprintf (out, "ip_table_name = %s\n", table->ip_table_name);
      fprintf (out, "num_files = %d\n", table->num_files);
  
      for (i = 1; i <= table->num_files; i++)
	{
	  fprintf (out, "IPSymTabEnt %d:\n", i);
	  P_DumpIPSymTabEnt (out, table->ip_table[i]);
	}
    }
  else
    {
      fprintf (out, "table is null\n");
    }
}

/*! \brief Prints the contents of an IPSymTabEnt.
 *
 * \param out
 *  the file to write.
 * \param ip_entry
 *  the IPSymTabEnt.
 *
 * Prints the contents of an IPSymTabEnt.
 */
void
P_DumpIPSymTabEnt (FILE *out, IPSymTabEnt ip_entry)
{
  if (ip_entry)
    {
      if (ip_entry->source_name)
	fprintf (out, "source_name = %s\n", ip_entry->source_name);
      if (ip_entry->in_name)
	fprintf (out, "in_name = %s\n", ip_entry->in_name);
      if (ip_entry->out_name)
	fprintf (out, "out_name = %s\n", ip_entry->out_name);
      fprintf (out, "key = %d\n", ip_entry->key);
      fprintf (out, "num_entries = %d\n", ip_entry->num_entries);
      
      BlockSparseArrayDump (ip_entry->table, out,
			    (void (*)(FILE *, void *))P_DumpSymTabEntry);
    }
  else
    {
      fprintf (out, "ip_entry is null\n");
    }
}

/*! \brief Prints the contents of a SymTabEntry
 *
 * \param out
 *  the file to write.
 * \param entry
 *  the SymTabEntry.
 *
 * Prints the contents of a SymTabEntry.
 */
void
P_DumpSymTabEntry (FILE *out, SymTabEntry entry)
{
  if (entry)
    {
      fprintf (out, "key = {%d, %d}\n", entry->key.file, entry->key.sym);
      if (entry->name)
	fprintf (out, "name = %s\n", entry->name);
      fprintf (out, "scope_key = {%d, %d}\n", entry->scope_key.file,
	       entry->scope_key.sym);
      switch (P_GetSymTabEntryType (entry))
	{
	case ET_FUNC:
	  fprintf (out, "FuncDcl = 0x%lx\n",
		   (long int)P_GetSymTabEntryFuncDcl (entry));
	  break;
	case ET_TYPE_LOCAL:
	case ET_TYPE_GLOBAL:
	  fprintf (out, "TypeDcl = 0x%lx\n", 
		   (long int)P_GetSymTabEntryTypeDcl (entry));
	  break;
	case ET_VAR_LOCAL:
	case ET_VAR_GLOBAL:
	  fprintf (out, "VarDcl = 0x%lx\n",
		   (long int)P_GetSymTabEntryVarDcl (entry));
	  break;
	case ET_STRUCT:
	  fprintf (out, "StructDcl = 0x%lx\n",
		   (long int)P_GetSymTabEntryStructDcl (entry));
	  break;
	case ET_UNION:
	  fprintf (out, "UnionDcl = 0x%lx\n",
		   (long int)P_GetSymTabEntryUnionDcl (entry));
	  break;
	case ET_ENUM:
	  fprintf (out, "EnumDcl = 0x%lx\n",
		   (long int)P_GetSymTabEntryEnumDcl (entry));
	  break;
	case ET_ASM:
	  fprintf (out, "AsmDcl = 0x%lx\n",
		   (long int)P_GetSymTabEntryAsmDcl (entry));
	  break;
	case ET_STMT:
	  fprintf (out, "Stmt = 0x%lx\n",
		   (long int)P_GetSymTabEntryStmt (entry));
	  break;
	case ET_EXPR:
	  fprintf (out, "Expr = 0x%lx\n", 
		   (long int)P_GetSymTabEntryExpr (entry));
	  break;
	case ET_FIELD:
	  fprintf (out, "Field = 0x%lx\n", 
		   (long int)P_GetSymTabEntryField (entry));
	  break;
	case ET_ENUMFIELD:
	  fprintf (out, "EnumField = 0x%lx\n",
		   (long int)P_GetSymTabEntryEnumField (entry));
	  break;
	case ET_LABEL:
	  fprintf (out, "Label = 0x%lx\n",
		   (long int)P_GetSymTabEntryLabel (entry));
	  break;
	case ET_BLOCK:
	  fprintf (out, "BlockSize = %d\n", P_GetSymTabEntryBlockSize (entry));
	  break;
	case ET_SCOPE:
	  fprintf (out, "Scope\n");
	  break;
	default:
	  fprintf (out, "Unknown entry type %d\n",
		   P_GetSymTabEntryType (entry));
	  break;
	}
      
      if (entry->scope)
	{
	  ScopeEntry se;
	  
	  fprintf (out, "Scope: key = {%d, %d}\n", entry->scope->key.file,
		   entry->scope->key.sym);
	  
	  for (se = entry->scope->scope_entry; se; se = se->next)
	    {
	      fprintf (out, "ScopeEntry: key = {%d, %d}\n", se->key.file,
		       se->key.sym);
	    }
	}
      
      fprintf (out, "offset = %d\n", entry->offset);
    }
  else
    {
      fprintf (out, "entry is null\n");
    }
}
   
/*! \brief Determines if a SymTabEntry needs to be read from disk.
 *
 * \param e
 *  the SymTabEntry
 *
 * \return
 *  If the SymTabEntry needs to be read from disk, returns TRUE.  Otherwise,
 *  returns FALSE.
 */
static bool
must_read (SymTabEntry e)
{
  if (P_GetSymTabEntryOffset (e) == 0)
    return (FALSE);

  switch (P_GetSymTabEntryType (e))
    {
    case ET_FUNC:
      return (P_GetSymTabEntryFuncDcl (e) == NULL);
    case ET_TYPE_GLOBAL:
      return (P_GetSymTabEntryTypeDcl (e) == NULL);
    case ET_VAR_GLOBAL:
      return (P_GetSymTabEntryVarDcl (e) == NULL);
    case ET_STRUCT:
      return (P_GetSymTabEntryStructDcl (e) == NULL);
    case ET_UNION:
      return (P_GetSymTabEntryUnionDcl (e) == NULL);
    case ET_FIELD:
      return (P_GetSymTabEntryField (e) == NULL);
    case ET_ENUM:
      return (P_GetSymTabEntryEnumDcl (e) == NULL);
    case ET_SCOPE:
      return (P_GetSymTabEntryScope (e) == NULL);
#if 0
      return ((P_GetSymTabEntryOffset (e) != 0) && !in_mem (e));
#endif
    default:
      break;
    }

  return (FALSE);
}

/*! \brief Clears an entry in the symbol table.
 *
 * \param table
 *  the symbol table.
 * \param key
 *  the key of the entry to clear.
 *
 * This function clears the SymTabEntry pointer corresponding to \a key in
 * \a table.
 */
void
PST_ClearEntry (SymbolTable table, Key key)
{
  int i;

  if (P_ValidKey (key) && PST_HasFile (table, key.file))
    {
      /* If we're removing the last entry in the file, reset num_entries. */
      if (table->ip_table[key.file]->num_entries == key.sym)
	{
	  for (i = table->ip_table[key.file]->num_entries - 1; i >= 1; i--)
	    {
	      if (BlockSparseArrayGet (&(table->ip_table[key.file]->table), i))
		break;
	    }

	  table->ip_table[key.file]->num_entries = i;
	}

      BlockSparseArrayClear (&(table->ip_table[key.file]->table), key.sym);
    }

  return;
}

/*! \brief Flattens a tree of scopes to a single scope.
 *
 * \param table
 *  the symbol table.
 * \param scope
 *  the root of the scope tree to flatten.
 *
 * \return
 *  A ScopeEntry list of all table entries referenced in all scopes under
 *  \a scope.
 *
 * Flattens a tree of scopes to a single scope and returns a pointer
 * to a ScopeEntry list.  The tree is flattened using a preorder traversal.
 * It is the caller's responsibility to free
 * this list using P_RemoveScopeEntry().
 *
 * \sa P_RemoveScopeEntry()
 */
ScopeEntry
PST_FlattenScope (SymbolTable table, Scope scope)
{
  ScopeEntry result = NULL, new = NULL, last = NULL;
  ScopeEntry se;
  SymTabEntry entry;
  Scope entry_scope;

  for (se = P_GetScopeScopeEntry (scope); se; se = P_GetScopeEntryNext (se))
    {
      /* We're doing a preorder traversal, so we want to copy the list
       * node by node. */
      new = P_CopyScopeEntryNode (se);

      if (result == NULL)
	{
	  result = new;
	}
      else
	{
	  P_SetScopeEntryNext (last, new);
	}

      last = new;

      /* If this scope entry is a scope, add all entries from it. */
      if ((entry = PST_GetSymTabEntry (table, last->key)))
	{
	  if ((entry_scope = P_GetSymTabEntryScope (entry)))
	    {
	      P_SetScopeEntryNext (last,
				   PST_FlattenScope (table, entry_scope));
	      
	      while (last->next)
		last = last->next;
	    }
	}
    }

  return (result);
}

/*! \brief Finds the scope enclosing a symbol key.
 *
 * \param table
 *  the symbol table.
 * \param k
 *  the symbol key.
 *
 * \return
 *  The key of the scope under which \a k is defined.
 */
Key
PST_GetScopeFromEntryKey (SymbolTable table, Key k)
{
  SymTabEntry e = PST_GetSymTabEntry (table, k);
  Key scope_key = P_GetSymTabEntryScopeKey (e);
  Key file_scope = PST_GetFileScope (table, k.file);
  Key global_scope = PST_GetGlobalScope (table);

  /* If this is the top level scope in a file, return the global scope as its
   * scope. */
  if (!P_ValidKey (scope_key) && P_MatchKey (k, file_scope) && \
      !P_MatchKey (k, global_scope))
    scope_key = PST_GetGlobalScope (table);

  return (scope_key);
}  

/*! \brief Returns the key of the global scope.
 *
 * \param table
 *  the symbol table.
 *
 * \return
 *  The key of the global scope.
 *
 * Returns the key of the top level scope.
 *
 * \sa PST_GetFileScope(), #PST_GetFuncDclScope(),
 * #PST_GetTypeDclScope(), #PST_GetVarDclScope(),
 * #PST_GetStructDclScope(), #PST_GetUnionDclScope(),
 * PST_GetStmtScope(), PST_GetExprScope() */
Key
PST_GetGlobalScope (SymbolTable table)
{
  Key global_scope = {1, 1};

  return (global_scope);
}

/*! \brief Returns the key of a file's top level scope.
 *
 * \param table
 *  the symbol table.
 * \param file
 *  the file's key.
 *
 * \return
 *  The key of the file's scope.
 *
 * Returns the key of the file's  top level scope.
 *
 * \sa PST_GetGlobalScope(), #PST_GetFuncDclScope(),
 * #PST_GetTypeDclScope(), #PST_GetVarDclScope(),
 * #PST_GetStructDclScope(), #PST_GetUnionDclScope(),
 * PST_GetStmtScope(), PST_GetExprScope() */
Key
PST_GetFileScope (SymbolTable table, int file)
{
  Key file_scope;

  /* HP's compiler isn't happy with initializing file_scope to
   * {file, 1}. */
  file_scope.file = file;
  file_scope.sym = 1;

  return (file_scope);
}

/*! \brief Annotates a SymTabEntry with its new key.
 *
 * \param table
 *  the symbol table.
 * \param old
 *  the old key.
 * \param new
 *  the new key.
 * \param clean
 *  a list of keys of symbol table entries that have been tagged with
 *  a KeyMap structure.
 *
 * \return
 *  A list of keys of symbol table entries that have been tagged with a
 *  KeyMap structure is returned as \a clean.
 *
 * Stores the new key for a SymTabEntry in the entry's extension field.
 * If the extension field is null, allocates a new KeyMap structure to save
 * the key.
 *
 * \note The Pcode library allocates space in the SymTabEntry's extension
 *       early in main().  It sets the HO_MANUAL_ALLOC option so that the
 *       KeyMap is not allocated until we need it.
 *
 * \sa PST_GetNewKey()
 */
void
PST_SetNewKey (SymbolTable table, Key old, Key new, KeyList *clean)
{
  SymTabEntry entry = NULL;
  KeyMap map = NULL;

  if ((entry = PST_GetSymTabEntry (table, old)))
    {
      if ((map = P_GetSymTabEntryExtL (entry, Indices[ES_SYMTABENTRY])))
	{
	  P_punt ("symtab.c:PST_SetNewKey:%d Entry (%d, %d) already has a new "
		  "key (%d, %d)", __LINE__ - 1, old.file, old.sym,
		  P_GetKeyMapNewKey (map).file, P_GetKeyMapNewKey (map).sym);
	}
      else
	{
	  /* There are no known users of this key.  Allocate a new KeyMap with
	   * the new key. */
	  map = P_NewKeyMap ();
	  P_SetKeyMapNewKey (map, new);
	  P_SetSymTabEntryExtL (entry, Indices[ES_SYMTABENTRY], map);

	  /* Add this entry's old key to the list of entries to clean. */
	  if (clean)
	    *clean = P_AppendKeyListNext (*clean, P_NewKeyListWithKey (old));
	}
    }

  return;
}

/*! \brief Retrives the new key for a SymTabEntry.
 *
 * \param table
 *  the symbol table.
 * \param old
 *  the SymTabEntry's old key.
 *
 * \return
 *  The SymTabEntry's new key, or Invalid_Key, if the new key doesn't exist.
 */
Key
PST_GetNewKey (SymbolTable table, Key old)
{
  SymTabEntry entry = NULL;
  KeyMap map = NULL;
  Key result = Invalid_Key;

  if ((entry = PST_GetSymTabEntry (table, old)))
    {
      if ((map = P_GetSymTabEntryExtL (entry, Indices[ES_SYMTABENTRY])))
	{
	  result = P_GetKeyMapNewKey (map);
	}
    }

  return (result);
}

/*! \brief Removes KeyMap structures from SymTabEntries.
 *
 * \param table
 *  the symbol table.
 * \param clean
 *  a list of keys of symbol table entries that are tagged with KeyMap
 *  structures.
 *
 * Frees the KeyMap structure attached to each SymTabEntry.
 *
 * This function frees the KeyList passed as clean. */
void
PST_CleanEntries (SymbolTable table, KeyList *clean)
{
  SymTabEntry entry;
  KeyList l;
  KeyMap map;

  for (l = *clean; l; l = P_GetKeyListNext (l))
    {
      entry = PST_GetSymTabEntry (table, P_GetKeyListKey (l));

      map = P_GetSymTabEntryExtL (entry, Indices[ES_SYMTABENTRY]);
      map = P_RemoveKeyMap (map);
      P_SetSymTabEntryExtL (entry, Indices[ES_SYMTABENTRY], map);
    }

  *clean = P_RemoveKeyList (*clean);

  return;
}

/* IPSymTabEnt functions. */
/*! \brief Pre-allocates space in the IPSymTabEnt's table.
 *
 * \param ipste
 *  the IPSymTabEnt.
 * \param index
 *  the starting index of the block to pre-allocate.
 * \param n
 *  the number of entries to pre-allocate.
 *
 * Pre-allocates \a n entries starting at \a index in the IPSymTabEnt.
 */
void
PST_PrepareIPSymTabEntTable (IPSymTabEnt ipste, int index, int n)
{
  if (P_GetIPSymTabEntTable (ipste) == NULL)
    {
      P_SetIPSymTabEntTable (ipste, NewBlockSparseArray ());
      BlockSparseArraySetOption (P_GetIPSymTabEntTable (ipste), BSA_OPTIONS);
    }

  BlockSparseArrayPrepare (&P_GetIPSymTabEntTable (ipste), index, n);

  return;
}

/*! \brief Sets an entry in the IPSymTabEnt.
 *
 * \param ipste
 *  the IPSymTabEnt.
 * \param entry
 *  the entry to add.
 *
 * \return
 *  The entry's new key.
 *
 * Adds \a entry to \a ipste's table.  If \a entry.key.sym is 0, \a key.sym
 * is set to the next available key in \a ipste and the new key is returned.
 * If \a entry.key.sym is not 0, \a key is returned.
 *
 * If \a entry is of type ET_BLOCK, this function calls
 * PST_PrepareIPSymTabEntTable() to pre-allocate space.  In this case,
 * it returns Invalid_Key.
 */
Key
PST_AddIPSymTabEntEntry (IPSymTabEnt ipste, SymTabEntry entry)
{
  Key key = P_GetSymTabEntryKey (entry);

  if (P_GetSymTabEntryType (entry) == ET_BLOCK)
    {
      PST_PrepareIPSymTabEntTable (ipste,
				   P_GetSymTabEntryBlockStart (entry).sym,
				   P_GetSymTabEntryBlockSize (entry));
      entry = P_RemoveSymTabEntry (entry);
      return (Invalid_Key);
    }

  if (key.file != P_GetIPSymTabEntKey (ipste))
    P_punt ("symtab.c:PST_AddIPSymTabEntEntry:%d Attempting to insert key "
	    "(%d, %d) in wrong IP Table (%d)", __LINE__ - 1, key.file, key.sym,
	    P_GetIPSymTabEntKey (ipste));

  if (key.sym == 0)
    {
      P_SetIPSymTabEntNumEntries (ipste,
				  P_GetIPSymTabEntNumEntries (ipste) + 1);
      key.sym = P_GetIPSymTabEntNumEntries (ipste);
    }
  else if (key.sym > P_GetIPSymTabEntNumEntries (ipste))
    {
      P_SetIPSymTabEntNumEntries (ipste, key.sym);
    }

  if (P_GetIPSymTabEntTable (ipste) == NULL)
    {
      P_SetIPSymTabEntTable (ipste, NewBlockSparseArray ());
      BlockSparseArraySetOption (P_GetIPSymTabEntTable (ipste), BSA_OPTIONS);
    }

  BlockSparseArraySet (&P_GetIPSymTabEntTable (ipste), key.sym, entry);

  P_SetSymTabEntryKey (entry, key);

  return (key);
}

/*! \brief Gets an entry from an IPSymTabEnt.
 *
 * \param ipste
 *  the IPSymTabEnt.
 * \param key
 *  the entry's key.
 *
 * \return
 *  the SymTabEntry keyed by \a key.
 *
 * Retrieves a SymTabEntry from \a ipste by its key.
 */
SymTabEntry
PST_GetIPSymTabEntEntry (IPSymTabEnt ipste, Key key)
{
  SymTabEntry result = NULL;

  if (key.file != P_GetIPSymTabEntKey (ipste))
    P_punt ("symtab.c:PST_GetIPSymTabEntEntry:%d Attempting to retrieve key "
	    "(%d, %d) in wrong IP Table (%d)", __LINE__ - 1, key.file, key.sym,
	    P_GetIPSymTabEntKey (ipste));

  if (P_GetIPSymTabEntTable (ipste))
    result = (SymTabEntry)BlockSparseArrayGet (&P_GetIPSymTabEntTable (ipste),
					       key.sym);

  return (result);
}

/*! \brief Orders types so that types are defined before they are referenced.
 *
 * \param table
 *  the symbol table.
 * \param key
 *  the key of the type to inspect.
 * \param list
 *  the list of keys in their final order.
 * \param parent
 *  a list of keys of parent structs and unions.
 *
 * \return
 *  The list of keys in their final order.
 *
 * If \a key is a type, struct, or union, adds all types referenced by
 * \a key to \a list, then adds \a key to the list.  This ensures that
 * types used by a type are defined earlier in the list than that type.
 *
 * Recursive types can occur through pointers to named structs or unions
 * This function keeps a list of all parent types as it digs into a type.
 * When it detects a cycle (the current key is already on the parent list),
 * it continues recursing until it finds a pointer to a named struct or
 * union.  It adds the type referencing the pointer to list first, then
 * the rest of the types as it unwinds.
 *
 * struct foo {
 *   struct bar *b;
 * }
 * struct unnamed {
 *   struct foo d;
 * }
 * struct bar {
 *   struct unnamed c;
 * }
 *
 * Assume structs are processed top down.  The naive implementation will
 * process foo, bar *, bar, unnamed, then see foo.  It will detect a loop
 * at this point and start to back out, leaving the list as: unnamed, bar,
 * bar, foo.  When this order is written in C, struct foo is referenced
 * before it is defined, and things break.
 *
 * When this function sees foo after processing unnamed, it sees that the
 * type causing the loop is not a pointer to a named struct.  It will
 * continue to recurse, inspecting foo a second time.  This time, it will
 * see the pointer to bar, and since we're looping, add foo to list and 
 * start to unwind -- only adding a type to the list if it hasn't already
 * been added.  The list after *this* function is foo, unnamed, bar, bar *.
 */
static KeyList
order_types (SymbolTable table, Key key, KeyList list, KeyList parent)
{
  SymTabEntry entry;
  Key refd_type_key;
  bool added_parent = FALSE;

  entry = PST_GetSymTabEntry (table, key);

  /* If this type has already been added to the list, so have its referenced
   * types.  We can exit now. */
  if (P_FindKeyListKey (list, key))
    return (list);

  if (!P_FindKeyListKey (parent, key))
    {
      parent = P_AppendKeyListNext (parent, P_NewKeyListWithKey (key));
      added_parent = TRUE;
    }

  switch (P_GetSymTabEntryType (entry))
    {
    case ET_TYPE_GLOBAL:
    case ET_TYPE_LOCAL:
      {
	TypeDcl t = P_GetSymTabEntryTypeDcl (entry);
	refd_type_key = P_GetTypeDclType (t);

	/* Recurse unless we're looping (refd_type_key is found in the
	 * parent list and is a type that we can use to break recursion
	 * (pointer to a struct)). */
        if (P_ValidKey (refd_type_key) && \
	    !(find_parent (table, parent, refd_type_key) && \
	      can_break_recursion (table, refd_type_key)))
	  list = order_types (table, refd_type_key, list, parent);

	if (P_IsFunctionTypeDcl (t))
	  {
	    Param p;
	    
	    for (p = P_GetTypeDclParam (t); p; p = P_GetParamNext (p))
	      {
		refd_type_key = P_GetParamKey (p);
		
		if (P_ValidKey (refd_type_key) && \
		    !(find_parent (table, parent, refd_type_key) && \
		      can_break_recursion (table, refd_type_key)))
		  list = order_types (table, refd_type_key, list, parent);
	      }
	  }
      }
      break;
    case ET_STRUCT:
    case ET_UNION:
      {
	Field f;
	
	for (f = P_GetSymTabEntryFields (entry); f; f = P_GetFieldNext (f))
	  {
	    refd_type_key = P_GetFieldType (f);
	    
	    if (P_ValidKey (refd_type_key) && \
		!(find_parent (table, parent, refd_type_key) && \
		  can_break_recursion (table, refd_type_key)))
	      list = order_types (table, refd_type_key, list, parent);
	  }
      }
      break;
    default:
      P_punt ("symtab.c:order_types:%d invalid entry type 0x%x", __LINE__,
	      P_GetSymTabEntryType (entry));
      break;
    }

  if (added_parent)
    parent = P_DeleteKeyListNext (parent, key);

  /* Add key to list (if it hasn't already been added). */
  if (!P_FindKeyListKey (list, key))
    list = P_AppendKeyListNext (list, P_NewKeyListWithKey (key));

  return (list);
}

/*! \brief Finds a type in the parent list.
 *
 * \param table
 *  the symbol table.
 * \param parent
 *  the parent list.
 * \param type
 *  the type to search for in the parent list.
 *
 * \return
 *  The KeyList from \a list containing \a type.
 *
 * This function searches \a parent for \a type.  If \a type is a typedef,
 * this function also searches \a parent for the typedef's base type.  This
 * function returns the KeyList node containing the found type.  If
 * the type is a pointer to a pointer, we also search the parent list for
 * the base pointer type.
 *
 * These rules ensure that if we see something like
 *
 * struct foo
 * {
 *   struct ***foo;
 * }
 *
 * That we don't end up with an ordering like
 * (struct **foo)
 * (struct ***foo)
 * struct foo
 * struct *foo
 *
 * Where the higher order pointers can be processed before their base pointer
 * type.
 */
static KeyList
find_parent (SymbolTable table, KeyList parent, Type type)
{
  KeyList result = NULL;
  Type reduced_type, derefd_type;

  result = P_FindKeyListKey (parent, type);
  reduced_type = PST_ReduceTypedefs (table, type);

  if (result == NULL && !P_MatchKey (type, reduced_type))
    result = P_FindKeyListKey (parent, reduced_type);

  if (result == NULL && PST_IsPointerType (table, type))
    {
      derefd_type = PST_DereferenceType (table, type);
  
      if (PST_IsPointerType (table, derefd_type))
	result = find_parent (table, parent, derefd_type);
    }

  return (result);
}

/*! \brief Determines if a type can break a recursive type
 *
 * \param table
 *  the symbol table.
 * \param type
 *  the type to inspect.
 *
 * \return
 *  If \a type can break a recursive type, returns TRUE.  Otherwise, returns
 *  FALSE.
 *
 * A recursive cycle in a type can be broken by a type whose use can come out
 * of order with its definition.  In C, this means a pointer to a named
 * struct.  If \a type is a pointer to a named struct, returns TRUE.
 */
static bool
can_break_recursion (SymbolTable table, Type type)
{
  bool result = FALSE;

  if (PST_IsPointerType (table, type))
    {
      Type derefd_type = PST_DereferencePointers (table, type);

      if (PST_IsStructureType (table, derefd_type))
	result = TRUE;
    }

  return (result);
}

/*! \brief Sets the STE_FLUSH_ME flag on a SymTabEntry.
 *
 * \param table
 *  the symbol table.
 * \param key
 *  the key of the entry to tag.
 *
 * Sets the STE_FLUSH_ME flag on \a key and all SymTabEntries under \a key.
 */
static void
tag_entry_for_flush (SymbolTable table, Key key)
{
  SymTabEntry entry;

  if ((entry = PST_GetSymTabEntry (table, key)))
    {
      switch (P_GetSymTabEntryType (entry))
	{
	case ET_FUNC:
	  {
	    FuncDcl f = P_GetSymTabEntryFuncDcl (entry);
	    VarDcl p;
	    Stmt s;

	    P_SetSymTabEntryFlags (entry, STE_FLUSH_ME);
	  
	    List_start (P_GetFuncDclParam (f));
	    while ((p = (VarDcl)List_next (P_GetFuncDclParam (f))))
	      tag_entry_for_flush (table, P_GetVarDclKey (p));

	    if ((s = P_GetFuncDclStmt (f)))
	      tag_entry_for_flush (table, P_GetStmtKey (s));
	  }
	  break;
	case ET_VAR_GLOBAL:
	case ET_VAR_LOCAL:
	  {
	    VarDcl v = P_GetSymTabEntryVarDcl (entry);

	    P_SetSymTabEntryFlags (entry, STE_FLUSH_ME);

	    tag_init_for_flush (table, P_GetVarDclInit (v));
	  }
	  break;
	case ET_TYPE_GLOBAL:
	case ET_TYPE_LOCAL:
	case ET_ASM:
	case ET_LABEL:
	case ET_SCOPE:
	  P_SetSymTabEntryFlags (entry, STE_FLUSH_ME);
	  break;
	case ET_STRUCT:
	case ET_UNION:
	  {
	    Field f;

	    P_SetSymTabEntryFlags (entry, STE_FLUSH_ME);

	    for (f = P_GetSymTabEntryFields (entry); f; f = P_GetFieldNext (f))
	      tag_entry_for_flush (table, P_GetFieldKey (f));
	  }
	  break;
	case ET_ENUM:
	  {
	    EnumField f;

	    P_SetSymTabEntryFlags (entry, STE_FLUSH_ME);

	    for (f = P_GetEnumDclFields (P_GetSymTabEntryEnumDcl (entry));
		 f; f = P_GetEnumFieldNext (f))
	      tag_entry_for_flush (table, P_GetEnumFieldKey (f));
	  }
	  break;
	case ET_STMT:
	  tag_stmt_for_flush (table, P_GetSymTabEntryStmt (entry));
	  break;
	case ET_EXPR:
	  tag_expr_for_flush (table, P_GetSymTabEntryExpr (entry));
	  break;
	default:
	  break;
	}
    }

  return;
}

/*! \brief sets the STE_FLUSH_ME flag on a Stmt.
 *
 * \param table
 *  the symbol table.
 * \param stmt
 *  the Stmt to tag.
 *
 * Sets the STE_FLUSH_ME flag on \a stmt and all SymTabEntries under \a stmt.
 */
static void
tag_stmt_for_flush (SymbolTable table, Stmt stmt)
{
  SymTabEntry entry;
  Label l;

  while (stmt)
    {
      if (P_ValidKey (P_GetStmtKey (stmt)))
	{
	  entry = PST_GetSymTabEntry (table, P_GetStmtKey (stmt));
	  P_SetSymTabEntryFlags (entry, STE_FLUSH_ME);
	}

      for (l = P_GetStmtLabels (stmt); l; l = P_GetLabelNext (l))
	tag_entry_for_flush (table, P_GetLabelKey (l));

      switch (P_GetStmtType (stmt))
	{
	case ST_RETURN:
	  tag_expr_for_flush (table, P_GetStmtRet (stmt));
	  break;
	case ST_COMPOUND:
	  {
	    Compound c = P_GetStmtCompound (stmt);
	    VarDcl v;
	    TypeDcl t;

	    List_start (P_GetCompoundTypeList (c));
	    while ((t = (TypeDcl)List_next (P_GetCompoundTypeList (c))))
	      tag_entry_for_flush (table, P_GetTypeDclKey (t));

	    List_start (P_GetCompoundVarList (c));
	    while ((v = (VarDcl)List_next (P_GetCompoundVarList (c))))
	      tag_entry_for_flush (table, P_GetVarDclKey (v));

	    tag_stmt_for_flush (table, P_GetCompoundStmtList (c));
	  }
	  break;
	case ST_IF:
	  {
	    IfStmt i = P_GetStmtIfStmt (stmt);

	    tag_expr_for_flush (table, P_GetIfStmtCondExpr (i));
	    tag_stmt_for_flush (table, P_GetIfStmtThenBlock (i));
	    tag_stmt_for_flush (table, P_GetIfStmtElseBlock (i));
	  }
	  break;
	case ST_SWITCH:
	  {
	    SwitchStmt s = P_GetStmtSwitchStmt (stmt);

	    tag_expr_for_flush (table, P_GetSwitchStmtExpression (s));
	    tag_stmt_for_flush (table, P_GetSwitchStmtSwitchBody (s));
	  }
	  break;
	case ST_PSTMT:
	  tag_stmt_for_flush (table, P_GetPstmtStmt (P_GetStmtPstmt (stmt)));
	  break;
	case ST_MUTEX:
	  {
	    Mutex m = P_GetStmtMutex (stmt);

	    tag_expr_for_flush (table, P_GetMutexExpression (m));
	    tag_stmt_for_flush (table, P_GetMutexStatement (m));
	  }
	  break;
	case ST_COBEGIN:
	  {
	    Cobegin c = P_GetStmtCobegin (stmt);

	    tag_stmt_for_flush (table, P_GetCobeginStatements (c));
	  }
	  break;
	case ST_PARLOOP:
	  {
	    ParLoop p = P_GetStmtParLoop (stmt);

	    tag_stmt_for_flush (table, P_GetPstmtStmt (P_GetParLoopPstmt (p)));
	    tag_expr_for_flush (table, P_GetParLoopIterationVar (p));
	    tag_expr_for_flush (table, P_GetParLoopInitValue (p));
	    tag_expr_for_flush (table, P_GetParLoopFinalValue (p));
	    tag_expr_for_flush (table, P_GetParLoopIncrValue (p));
	  }
	  break;
	case ST_SERLOOP:
	  {
	    SerLoop s = P_GetStmtSerLoop (stmt);

	    tag_stmt_for_flush (table, P_GetSerLoopLoopBody (s));
	    tag_expr_for_flush (table, P_GetSerLoopCondExpr (s));
	    tag_expr_for_flush (table, P_GetSerLoopInitExpr (s));
	    tag_expr_for_flush (table, P_GetSerLoopIterExpr (s));
	  }
	  break;
	case ST_EXPR:
	  tag_expr_for_flush (table, P_GetStmtExpr (stmt));
	  break;
	case ST_BODY:
	  {
	    BodyStmt b = P_GetStmtBodyStmt (stmt);

	    tag_stmt_for_flush (table, P_GetBodyStmtStatement (b));
	  }
	  break;
	case ST_EPILOGUE:
	  {
	    EpilogueStmt e = P_GetStmtEpilogueStmt (stmt);

	    tag_stmt_for_flush (table, P_GetEpilogueStmtStatement (e));
	  }
	  break;
	case ST_ASM:
	  {
	    AsmStmt a = P_GetStmtAsmStmt (stmt);

	    tag_expr_for_flush (table, P_GetAsmStmtAsmClobbers (a));
	    tag_expr_for_flush (table, P_GetAsmStmtAsmString (a));
	    tag_expr_for_flush (table, P_GetAsmStmtAsmOperands (a));
	  }
	  break;
	default:
	  break;
	}

      stmt = P_GetStmtLexNext (stmt);
    }

  return;
}

/*! \brief sets the STE_FLUSH_ME flag on an Expr.
 *
 * \param table
 *  the symbol table.
 * \param expr
 *  the Expr to tag.
 *
 * Sets the STE_FLUSH_ME flag on \a expr and all SymTabEntries under \a expr.
 */
static void
tag_expr_for_flush (SymbolTable table, Expr expr)
{
  SymTabEntry entry;
  Expr op;

  while (expr)
    {
      if (P_ValidKey (P_GetExprKey (expr)))
	{
	  entry = PST_GetSymTabEntry (table, P_GetExprKey (expr));
	  P_SetSymTabEntryFlags (entry, STE_FLUSH_ME);
	}

      if (P_GetExprOpcode (expr) == OP_stmt_expr)
	tag_stmt_for_flush (table, P_GetExprStmt (expr));

      for (op = P_GetExprOperands (expr); op; op = P_GetExprSibling (op))
	tag_expr_for_flush (table, op);

      expr = P_GetExprNext (expr);
    }
  
  return;
}

/*! \brief sets the STE_FLUSH_ME flag on an init.
 *
 * \param table
 *  the symbol table.
 * \param init
 *  the Init to tag.
 *
 * Sets the STE_FLUSH_ME flag for all Exprs in \a init.
 */
static void
tag_init_for_flush (SymbolTable table, Init init)
{
  if (init)
    {
      tag_expr_for_flush (table, P_GetInitExpr (init));
      tag_init_for_flush (table, P_GetInitSet (init));
      tag_init_for_flush (table, P_GetInitNext (init));
    }

  return;
}

