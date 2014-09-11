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
 * This file contains declarations for routines to manage the Pcode symbol
 * table.
 *
 * I'd like to enforce some separation between the symbol table
 * routines and the rest of the Pcode AST structures.  One of the
 * problems with the old Pcode library is that an innocent sounding
 * function (like NewVar) would often have side effects such as
 * fiddling with the symbol table in some way.  In the new Pcode,
 * functions to handle Pcode AST structures are kept completely
 * separate from functions to handle the symbol table.  Functions with
 * a P_ prefix can be assumed to handle the Pcode AST structures only.
 * Those with a PST_ prefix can deal with the symbol table as well.
 *
 * To implement the separation, functions that deal with the symbol table
 * are kept in separate files from those that deal with Pcode AST structures.
 * This is the reason behind the separate _symtab files.  The functions
 * in these files all have a PST_ prefix.
 */
/*****************************************************************************/

#ifndef _PCODE_SYMTAB_H_
#define _PCODE_SYMTAB_H_

#include <config.h>
#include <stdio.h>
#include "pcode.h"
#include "struct.h"

/*! When writing the list of symbol table entries, we will insert block size
 * hints (number of consecutive table entries) that the read routines can use
 * to do pre-allocation.  This constant sets the minimum number of consecutive
 * entries we must see before inserting a hint. */
#define HINT_MIN_BLOCK_SIZE 3

/* For functions that operate on the name of an ::IPSymTabEnt, this specifies
 * whether we're interested in the original source name, input file name, or
 * output file name. */
#define IP_NAME_SOURCE 0
#define IP_NAME_IN     1
#define IP_NAME_OUT    2

extern SymbolTable PST_Open (char *in_file, char *out_file, _STFlags options);
extern void PST_Close (SymbolTable table);
extern void PST_FlushFile (SymbolTable table, int file, bool write);
extern void PST_FlushEntry (SymbolTable table, Key key, bool write);

/*! Sets the order in which sources are searched for a symbol.
 *
 * \param t
 *  the symbol table.
 * \param f
 *  the first source to search.
 * \param s
 *  the second source to search.
 * \param u
 *  the third source to search.
 *
 * \sa P_SetSymbolTableSearchOrder()
 */
#define PST_SetSearchOrder(t, f, s, u) \
          (P_SetSymbolTableSearchOrder ((t), (f), (s), (u)))

extern void PST_LinkSymbolsFile (SymbolTable table, int file_key);
extern void PST_LinkSymbolsFunc (SymbolTable table, FuncDcl func_dcl);
extern void PST_LinkSymbolsStruct (SymbolTable table, StructDcl struct_dcl);
extern void PST_LinkSymbolsUnion (SymbolTable table, UnionDcl union_dcl);
extern void PST_LinkSymbolsEnum (SymbolTable table, EnumDcl enum_dcl);
extern void PST_LinkSymbolsTypeList (SymbolTable table, TypeList type_list);
extern void PST_LinkSymbolsVarList (SymbolTable table, VarList var_list);
extern void PST_LinkSymbolsStmt (SymbolTable table, Stmt stmt);
extern void PST_LinkSymbolsExpr (SymbolTable table, Expr expr);

extern void PST_RotateFile (SymbolTable table);

extern void PST_Prepare (SymbolTable table, Key key, int length);
extern Key PST_AddEntry (SymbolTable table, _EntryType type, void *entry);
extern Key PST_AddSymTabEntry (SymbolTable table, SymTabEntry entry);
extern Key PST_AddFuncDclEntry (SymbolTable table, FuncDcl func_dcl);
extern Key PST_AddTypeDclEntry (SymbolTable table, TypeDcl type_dcl,
				_EntryType type);
extern Key PST_AddVarDclEntry (SymbolTable table, VarDcl var_dcl,
			       _EntryType type);
extern Key PST_AddStructDclEntry (SymbolTable table, StructDcl struct_dcl);
extern Key PST_AddUnionDclEntry (SymbolTable table, UnionDcl union_dcl);
extern Key PST_AddEnumDclEntry (SymbolTable table, EnumDcl enum_dcl);
extern Key PST_AddAsmDclEntry (SymbolTable table, AsmDcl asm_dcl);
extern Key PST_AddStmtEntry (SymbolTable table, Stmt stmt);
extern Key PST_AddExprEntry (SymbolTable table, Expr expr);
extern Key PST_AddFieldEntry (SymbolTable table, Field field);
extern Key PST_AddEnumFieldEntry (SymbolTable table, EnumField enum_field);
extern Key PST_AddLabelEntry (SymbolTable table, Label label);

extern _EntryType PST_GetEntry (SymbolTable table, Key key, void **entry);
extern SymTabEntry PST_GetSymTabEntry (SymbolTable table, Key key);
extern SymTabEntry PST_GetSymTabEntryFromSource (SymbolTable table, Key key,
						 _STSearchOrder source);
extern SymTabEntry PST_GetSymTabEntryCopyFromSource (SymbolTable table,
						     Key key,
						     _STSearchOrder source);

/*! \brief Retrieves a SymTabEntry from memory.
 *
 * \param t
 *  the symbol table.
 * \param k
 *  the key of the entry to retrieve.
 *
 * \return
 *  The SymTabEntry.
 *
 * Retrieves a SymTabEntry from memory.
 *
 * \note \a k is referenced more than once.
 */
#define PST_GetSymTabEntryFromMem(t, k) \
          (PST_GetIPSymTabEntEntry (PST_GetFile ((t), (k).file), (k)))

extern FuncDcl PST_GetFuncDclEntry (SymbolTable table, Key key);
extern TypeDcl PST_GetTypeDclEntry (SymbolTable table, Key key);
extern VarDcl PST_GetVarDclEntry (SymbolTable table, Key key);
extern StructDcl PST_GetStructDclEntry (SymbolTable table, Key key);
extern UnionDcl PST_GetUnionDclEntry (SymbolTable table, Key key);
extern EnumDcl PST_GetEnumDclEntry (SymbolTable table, Key key);
extern AsmDcl PST_GetAsmDclEntry (SymbolTable table, Key key);
extern Stmt PST_GetStmtEntry (SymbolTable table, Key key);
extern Expr PST_GetExprEntry (SymbolTable table, Key key);
extern Field PST_GetFieldEntry (SymbolTable table, Key key);
extern EnumField PST_GetEnumFieldEntry (SymbolTable table, Key key);
extern Label PST_GetLabelEntry (SymbolTable table, Key key);

extern void PST_OrderTypeUses (SymbolTable table);
extern void PST_ResetOrder (SymbolTable table);

extern Key PST_MoveEntry (SymbolTable table, Key src_key, Key dst_key);
extern void PST_DeleteEntry (SymbolTable table, Key key);

/*! \brief Removes an entry from a symbol table.
 *
 * \param t
 *  the symbol table.
 * \param k
 *  the key of the entry to remove.
 *
 * Removes the entry with key \a key from the symbol table.
 *
 * This function removes the SymTabEntry with key \a key and the associated
 * Pcode structure and frees allocated memory.  This function leaves a hole
 * in the symbol table.
 *
 * This function is primarily a memory deallocation routine, not a symbol
 * table maintenance routine.  Because of this, a removed entry may not
 * really be removed.  If an entry is flushed to the output file, then
 * removed, subsequent calls to PST_GetSymTabEntry() will result in the
 * entry appearing in the table again.  To permanently delete a symbol,
 * see PST_DeleteEntry().
 *
 * \note \a t is referenced multiple times.
 *
 * \sa PST_DeleteEntry(), PST_AddEntry(), PST_AddSymTabEntry()
 */
#define PST_RemoveEntry(t, k) \
          (PST_RemoveSymTabEntry ((t), PST_GetSymTabEntry ((t), (k))))

extern int PST_HasFile (SymbolTable table, int key);
extern int PST_AddFile (SymbolTable table, char *source_name,
			_FileType filetype);
extern int PST_AddFileWithKey (SymbolTable table, int key, char *source_name,
			       _FileType filetype);
extern int PST_GetFileKeyByName (SymbolTable table, char *name, int which);
extern IPSymTabEnt PST_GetFileByKey (SymbolTable table, int key);

/*! \brief Sets an IPSymTabEnt in the symbol table.
 *
 * \param t
 *  the symbol table.
 * \param f
 *  the file's key.
 * \param i
 *  the file's IPSymTabEnt.
 */
#define PST_SetFile(t, f, i) (P_GetSymbolTableIPTable ((t))[(f)] = (i))

/*! \brief Gets an IPSymTabEnt from the symbol table.
 *
 * \param t
 *  the symbol table.
 * \param f
 *  the file's key
 *
 * \return
 *  The file's IPSymTabEnt.
 */
#define PST_GetFile(t, f) (P_GetSymbolTableIPTable ((t))[(f)])

/*! \brief Sets the source name for a file.
 *
 * \param t
 *  the symbol table.
 * \param f
 *  the file's key.
 * \param s
 *  the file's source file name.
 *
 * \return
 *  The file's source file name.
 */
#define PST_SetFileSourceName(t, f, s) \
          (P_SetIPSymTabEntSourceName (PST_GetFile ((t), (f)), (s)))

/*! \brief Gets the source name for a file
 *
 * \param t
 *  the symbol table.
 * \param f
 *  the file's key.
 *
 * \return
 *  the file's source file name.
 */
#define PST_GetFileSourceName(t, f) \
          (P_GetIPSymTabEntSourceName (PST_GetFile ((t), (f))))

/*! \brief Sets the input name for a file.
 *
 * \param t
 *  the symbol table.
 * \param f
 *  the file's key.
 * \param i
 *  the file's input file name.
 *
 * \return
 *  The file's input file name.
 *
 * If \a file_key is 0, sets the IP symbol table's in name (ip_table_name).
 *
 * \note \a t, \a f, and \a i are referenced more than once.
 */
#define PST_SetFileInName(t, f, i) \
          ((f) == 0 ? (P_SetSymbolTableIPTableName ((t), (i))) : \
                      (P_SetIPSymTabEntInName (PST_GetFile ((t), (f)), (i))))

/*! \brief Gets the input name for a file
 *
 * \param t
 *  the symbol table.
 * \param f
 *  the file's key.
 *
 * \return
 *  the file's input file name.
 *
 * If \a file_key is 0, gets the IP symbol table's in name (ip_table_name).
 *
 * \note \a t, and \a i are referenced more than once.
 */
#define PST_GetFileInName(t, f) \
          ((f) == 0 ? (P_GetSymbolTableIPTableName ((t))) : \
                      (P_GetIPSymTabEntInName (PST_GetFile ((t), (f)))))

/*! \brief Sets the output name for a file.
 *
 * \param t
 *  the symbol table.
 * \param f
 *  the file's key.
 * \param o
 *  the file's output file name.
 *
 * \return
 *  The file's output file name.
 *
 * If \a file_key is 0, sets the IP symbol table's out name.
 *
 * \note \a t, \a f, and \a i are referenced more than once.
 */
#define PST_SetFileOutName(t, f, o) \
          ((f) == 0 ? (P_SetSymbolTableOutName ((t), (o))) : \
                      (P_SetIPSymTabEntOutName (PST_GetFile ((t), (f)), (o))))

/*! \brief Gets the output name for a file
 *
 * \param t
 *  the symbol table.
 * \param f
 *  the file's key.
 *
 * \return
 *  the file's output file name.
 *
 * If \a file_key is 0, gets the IP symbol table's out name.
 *
 * \note \a t, and \a i are referenced more than once.
 */
#define PST_GetFileOutName(t, f) \
          ((f) == 0 ? (P_GetSymbolTableOutName ((t))) : \
                      (P_GetIPSymTabEntOutName (PST_GetFile ((t), (f)))))

/*! \brief Sets the type of a file.
 *
 * \param t
 *  the symbol table.
 * \param f
 *  the file's key.
 * \param u
 *  the file's type (FT_SOURCE or FT_HEADER).
 *
 * \return
 *  the file's type.
 */
#define PST_SetFileType(t, f, u) \
          (P_SetIPSymTabEntFileType (PST_GetFile ((t), (f)), (u)))

/*! \brief Gets the type of a file.
 *
 * \param t
 *  the symbol table.
 * \param f
 *  the file's key.
 *
 * \return
 *  The file's type (FT_SOURCE or FT_HEADER).
 */
#define PST_GetFileType(t, f) \
          (P_GetIPSymTabEntFileType (PST_GetFile ((t), (f))))

/*! \brief Sets the number of table entries for a file.
 *
 * \param t
 *  the symbol table.
 * \param f
 *  the file's key.
 * \param n
 *  the number of symbol table entries in the file.
 *
 * \return
 *  The number of symbol table entries in the file.
 */
#define PST_SetFileNumEntries(t, f, n) \
          (P_SetIPSymTabEntNumEntries (PST_GetFile ((t), (f)), (n)))

/*! \brief Gets the number of table entries for a file.
 *
 * \param t
 *  the symbol table.
 * \param f
 *  the file's key.
 *
 * \return
 *  The number of symbol table entries in the file.
 */
#define PST_GetFileNumEntries(t, f) \
          (P_GetIPSymTabEntNumEntries (PST_GetFile ((t), (f))))

/*! \brief Sets the offset of a file.
 *
 * \param t
 *  the symbol table.
 * \param f
 *  the file's key.
 * \param o
 *  the file's offset in the Pcode file.
 *
 * \return
 *  the file's offset in the Pcode file
 */
#define PST_SetFileOffset(t, f, o) \
          (P_SetIPSymTabEntOffset (PST_GetFile ((t), (f)), (o)))

/*! \brief Gets the offset of a file.
 *
 * \param t
 *  the symbol table.
 * \param f
 *  the file's key.
 *
 * \return
 *  The file's offset in the Pcode file..
 */
#define PST_GetFileOffset(t, f) \
          (P_GetIPSymTabEntOffset (PST_GetFile ((t), (f))))

/*! \brief Sets the single file symbol table for a file.
 *
 * \param t
 *  the symbol table.
 * \param f
 *  the file's key.
 * \param u
 *  the single file symbol table.
 *
 * \return
 *  The single file symbol table.
 */
#define PST_SetFileTable(t, f, u) \
          (P_SetIPSymTabEntTable (PST_GetFile ((t), (f)), (u)))

/*! \brief Gets the single file symbol table for a file.
 *
 * \param t
 *  the symbol table.
 * \param f
 *  the file's key.
 *
 * \return
 *  The single file symbol table for a file.
 */
#define PST_GetFileTable(t, f) (P_GetIPSymTabEntTable (PST_GetFile ((t), (f))))

extern FILE *PST_SetFileFile (SymbolTable table, int file_key, FILE *f);
extern FILE *PST_GetFileFile (SymbolTable table, int file_key);
extern _FileStatus PST_SetFileInFileStatus (SymbolTable table, int file_key,
					    _FileStatus f);
extern _FileStatus PST_GetFileInFileStatus (SymbolTable table, int file_key);
extern _FileStatus PST_SetFileOutFileStatus (SymbolTable table, int file_key,
					     _FileStatus f);
extern _FileStatus PST_GetFileOutFileStatus (SymbolTable table, int file_key);

/*! \brief Sets flags for a file.
 *
 * \param t
 *  the symbol table.
 * \param f
 *  the file's key.
 * \param g
 *  the flags to set.
 *
 * \return
 *  The new value of the flags field.
 */
#define PST_SetFileFlags(t, f, g) \
          (P_SetIPSymTabEntFlags (PST_GetFile ((t), (f)), (g)))

/*! \brief Gets the flags for a file.
 *
 * \param t
 *  the symbol table.
 * \param f
 *  the file's key.
 *
 * \return
 *  The value of the flags field.
 */
#define PST_GetFileFlags(t, f) \
          (P_GetIPSymTabEntFlags (PST_GetFile ((t), (f))))

/*! \brief Tests flags for a file.
 *
 * \param t
 *  the symbol table.
 * \param f
 *  the file's key.
 * \param g
 *  the flags to test.
 *
 * \return
 *  Non-zero, if the flags are set.  Otherwise, returns 0.
 */
#define PST_TstFileFlags(t, f, g) \
          (P_TstIPSymTabEntFlags (PST_GetFile ((t), (f)), (g)))

/*! \brief Clears flags for a file.
 *
 * \param t
 *  the symbol table.
 * \param f
 *  the file's key.
 * \param g
 *  the flags to clear.
 *
 * \return
 *  The new value of the flags field.
 */
#define PST_ClrFileFlags(t, f, g) \
          (P_ClrIPSymTabEntFlags (PST_GetFile ((t), (f)), (g)))

/*! \brief Sets the pragma for a file.
 *
 * \param t
 *  the symbol table.
 * \param f
 *  the file's key.
 * \param p
 *  the pragma.
 *
 * \return
 *  The file's new pragma.
 */
#define PST_SetFilePragma(t, f, p) \
          (P_SetIPSymTabEntPragma (PST_GetFile ((t), (f)), (p)))

/*! \brief Gets the pragma for a file.
 *
 * \param t
 *  the symbol table.
 * \param f
 *  the file's key.
 *
 * \return
 *  The file's pragma.
 */
#define PST_GetFilePragma(t, f) \
          (P_GetIPSymTabEntPragma (PST_GetFile ((t), (f))))

/*! \brief Sets the file's extension field for a module.
 *
 * \param t
 *  the symbol table.
 * \param f
 *  the file's key.
 * \param e
 *  the extension field.
 *
 * \return
 *  The file's new extension field.
 */
#define PST_SetFileExtM(t, f, e) \
          (P_SetIPSymTabEntExtM (PST_GetFile ((t), (f)), (e)))

/*! \brief Gets the file's extension field for module.
 *
 * \param t
 *  the symbol table.
 * \param f
 *  the file's key.
 *
 * \return
 *  The file's new extension field.
 */
#define PST_GetFileExtM(t, f) (P_GetIPSymTabEntExtM (PST_GetFile ((t), (f))))

/*! \brief Sets the file's extension field for a library.
 *
 * \param t
 *  the symbol table.
 * \param f
 *  the file's key.
 * \param i
 *  the library's index in the ext array.
 * \param e
 *  the extension field.
 *
 * \return
 *  The file's new extension field.
 */
#define PST_SetFileExtL(t, f, i, e) \
          (P_SetIPSymTabEntExtL (PST_GetFile ((t), (f)), (i), (e)))

/*! \brief Gets the file's extension field for library.
 *
 * \param t
 *  the symbol table.
 * \param f
 *  the file's key.
 * \param i
 *  the library's index in the ext array.
 *
 * \return
 *  The file's new extension field.
 */
#define PST_GetFileExtL(t, f, i) \
          (P_GetIPSymTabEntExtL (PST_GetFile ((t), (f)), (i)))

extern Key PST_AddNewScope (SymbolTable table, Key key);
extern void PST_AddEntryToScope (SymbolTable table, Key scope_key,
				 Key entry_key);
extern void PST_RemoveEntryFromScope (SymbolTable table, Key scope_key,
				      Key entry_key);
extern Scope PST_GetScope (SymbolTable table, Key key);
extern Key PST_GetParentScope (SymbolTable table, Key key);

extern Key PST_GetFileEntryFromMem (SymbolTable table, int file_key);
extern Key PST_GetFileEntryFromMemNext (SymbolTable table, Key last);
extern Key PST_GetFileEntryByType (SymbolTable table, int file_key,
				   _EntryType type);
extern Key PST_GetFileEntryByTypeNext (SymbolTable table, Key last,
				       _EntryType type);
extern Key PST_GetTableEntryFromMem (SymbolTable table);
extern Key PST_GetTableEntryFromMemNext (SymbolTable table, Key last);
extern Key PST_GetTableEntryByType (SymbolTable table, _EntryType type);
extern Key PST_GetTableEntryByTypeNext (SymbolTable table, Key last,
					_EntryType type);

extern ScopeEntry PST_GetScopeEntryByType (SymbolTable table, Key scope_key,
					   _EntryType type);
extern ScopeEntry PST_GetScopeEntryByTypeNext (SymbolTable table,
					       ScopeEntry last,
					       _EntryType type);
extern ScopeEntry PST_GetScopeEntryByTypeR (SymbolTable table, Key scope_key,
					    _EntryType type);
extern ScopeEntry PST_GetScopeEntryByTypeRNext (SymbolTable table,
						ScopeEntry last,
						_EntryType type);

extern Key PST_ScopeFindByName (SymbolTable table, Key scope_key, char *name,
				_EntryType type);
extern Key PST_ScopeFindByNameNext (SymbolTable table, Key scope_key, Key last,
				    _EntryType type);
extern Key PST_ScopeFindByNameR (SymbolTable table, Key scope_key, char *name,
				 _EntryType type);
extern Key PST_ScopeFindByNameRNext (SymbolTable table, Key scope_key,
				     Key last, _EntryType type);
extern Key PST_ScopeFindTypeByNameBasicType (SymbolTable table, Key scope_key,
					     char *name, _BasicType bt);
extern Key PST_ScopeFindTypeByNameBasicTypeR (SymbolTable table, Key scope_key,
					      char *name, _BasicType bt);

extern bool PST_ScopeContainsKey (SymbolTable table, Key scope_key, Key key);

extern int PST_FuncDclNextExprID (FuncDcl func);
extern int PST_ScopeNextExprID (SymbolTable table, Key scope_key);

#if 0
extern Key PST_GetFirstKey (SymbolTable table);
extern Key PST_GetNextKey (SymbolTable table, Key key);
#endif

#if 0
extern Key PST_ScopeFindLabelByValue (SymbolTable table, Key scope_key,
				      char *value);
#endif

extern void PST_DumpSymbolTableOrder (SymbolTable table, int file);
extern void P_DumpSymbolTable (FILE *out, SymbolTable table);
extern void P_DumpIPSymTabEnt (FILE *out, IPSymTabEnt ip_entry);
extern void P_DumpSymTabEntry (FILE *out, SymTabEntry entry);

extern void PST_ClearEntry (SymbolTable table, Key key);
extern ScopeEntry PST_FlattenScope (SymbolTable table, Scope scope);
extern Key PST_GetScopeFromEntryKey (SymbolTable table, Key k);
extern Key PST_GetGlobalScope (SymbolTable table);
extern Key PST_GetFileScope (SymbolTable table, int file);

extern void PST_SetNewKey (SymbolTable table, Key old, Key new,
			   KeyList *clean);
extern Key PST_GetNewKey (SymbolTable table, Key old);
extern void PST_CleanEntries (SymbolTable table, KeyList *clean);

/* IPSymTabEnt functions.  These function operate on a single file from
 * the symbol table. */
extern void PST_PrepareIPSymTabEntTable (IPSymTabEnt ipste, int index, int n);
extern Key PST_AddIPSymTabEntEntry (IPSymTabEnt ipste, SymTabEntry entry);
extern SymTabEntry PST_GetIPSymTabEntEntry (IPSymTabEnt ipste, Key key);

#endif

