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
 * \brief Functions to manage Plink's structures.
 *
 * \author Robert Kidd and Wen-mei Hwu.
 *
 * This file contains structure definitions and function prototypes to
 * manage Plink's data structures.
 */

#ifndef _PLINK_DATA_H_
#define _PLINK_DATA_H_

#include <config.h>
#include <library/i_list.h>
#include <library/i_hashl.h>
#include <Pcode/pcode.h>
#include <Pcode/struct.h>

#define PLINK_HASH_SIZE 128

/*! Flags to set on the Plink_SymTabEntryExt struct. */
typedef enum _Plink_SymTabEntryFlags
{
  PLF_MERGING     = 0x00000001, /*!< The entry is being merged. */
  PLF_MERGED      = 0x00000002, /*!< The entry has been merged. */
  PLF_COPIED      = 0x00000004, /*!< The entry has been copied to IP Table. */
  PLF_LINKED      = 0x00000008, /*!< The entry has been linked to another
				 * entry. */
  PLF_UPDATING    = 0x00000010, /*!< The entry is being updated. */

  /* Flags that indicate what the SymTabEntryExt.key field refers to. */
  PLF_LT_SOURCE   = 0x00000020, /*!< The key refers to the SymTabEntry's
				 * Source Table entry.  This will be set
				 * on entries in IP Table. */
  PLF_LT_IP       = 0x00000040, /*!< The key refers to the SymTabEntry's
				 * IP Table entry.  This will be set
				 * on entries in Source Table. */
  PLF_LT_LINK     = 0x00000080, /*!< The key links this SymTabEntry to another.
				 * The linked entry should be used instead
				 * of this one. */

  PLF_TO_GLOBAL   = 0x00000100, /*!< This entry should be copied directly
				 * to the global scope. */

  PLF_LONELY_ST   = 0x00000200, /*!< For some reason, certain structs
				 * may not have corresponding BT_STRUCT
				 * TypeDcls in C++.  This module currently
				 * prints a warning and tags the struct. */
}
_Plink_SymTabEntryFlags;

/*! Flags to set on the Plink_FuncDclExt struct. */
typedef enum _Plink_FuncDclFlags
{
  PL_FD_MST_FLTN  = 0x00000001, /*!< Set when a validation function inserts
				 *   a compound expression that will require
				 *   flattening. */
  PL_FD_FLATTENED = 0x00000002, /*!< Set when a function is flattened.
				 *   This can probably be replaced by a
				 *   reference to Pflatten's extension data
				 *   once that is converted. */
}
_Plink_FuncDclFlags;

/*! Values for the type of an input file. */
typedef enum _Plink_InputType
{
  PL_INPUT_OBJECT,              /*!< An object file. */
  PL_INPUT_LIBRARY,             /*!< A static library. */
}
_Plink_InputType;

/*! A mask to match the link type flags. */
#define PLF_LT_MASK (PLF_LT_SOURCE | PLF_LT_IP | PLF_LT_LINK)

/*! A struct to map Source Table keys to IP Table keys.
 *
 * This struct is used to link table entries between source files and to map
 * Source Table keys to IP Table keys.
 *
 * If file is 0, the key field holds a complete key for the IP Table, which
 * is this entry's new key.
 *
 * If file is not 0, file is a source file's key in IP Table.  key is
 * a key into this source file, which is the entry to which this entry is
 * linked.
 */
typedef struct Plink_Key
{
  int file;                     /*!< the source file's key. */
  Key key;                      /*!< the SymTabEntry's new key. */
}
_Plink_Key, *Plink_Key;

/*! A struct to hold names of input files and libraries. */
typedef struct _Plink_InputFile
{
  _Plink_InputType type;
  char *name;
} _Plink_InputFile, *Plink_InputFile;

/*! The struct attached to each SymTabEntry. */
typedef struct Plink_SymTabEntryExt
{
  _Plink_Key key;                /*!< Maps to another entry in the table. */
  _Plink_SymTabEntryFlags flags; /*!< SymTabEntry status flags. */
  int src_file;                  /*!< The key for the source file that holds
				  * this SymTabEntry.  If 0, SymTabEntry is
				  * in IP Table. */
  List users;                    /*!< A list of Plink_Keys that reference
				  * this SymTabEntry. */
}
_Plink_SymTabEntryExt, *Plink_SymTabEntryExt;

/*! A struct to hold a Source Table in the IPSymTabEnt.ext field. */
typedef struct Plink_IPSymTabEntExt
{
  SymbolTable source_table;     /*!< the source file's Source Table. */
}
_Plink_IPSymTabEntExt, *Plink_IPSymTabEntExt;

/*! A struct to hold a signature on a TypeDcl. */
typedef struct Plink_TypeDclExt
{
  char *sig;                    /*!< The type signature. */
  Key multi_type;               /*!< The key in ip_table of a BT_UNION TypeDcl
				 *   holding this and similar TypeDcls. */
}
_Plink_TypeDclExt, *Plink_TypeDclExt;
/*! A struct to hold a signature on a StructDcl. */
typedef struct Plink_StructDclExt
{
  char *sig;                    /*!< The struct signature. */
  Key multi_type;               /*!< The key in ip_table of a BT_UNION TypeDcl
				 *   holding pointers to this and other
				 *   structs with the same name. */
}
_Plink_StructDclExt, *Plink_StructDclExt;

/*! A struct to hold a signature on a UnionDcl. */
typedef struct Plink_UnionDclExt
{
  char *sig;                    /*!< The union signature. */
  Key multi_type;               /*!< The key in ip_table of a BT_UNION TypeDcl
				 *   holding pointers to this and other
				 *   unions with the same name. */
  HashLTable multi_hash;        /*!< If this is a union constructed by
				 *   Plink to hold pointers to structs
				 *   with the same name, this bit will
				 *   be set. */
}
_Plink_UnionDclExt, *Plink_UnionDclExt;

/*! A struct to hold Plink attributes for a FuncDcl. */
typedef struct Plink_FuncDclExt
{
  _Plink_FuncDclFlags flags;    /*!< FuncDcl status flags. */
}
_Plink_FuncDclExt, *Plink_FuncDclExt;

/*! A struct to map external symbol names to symbol table keys. */
typedef struct Plink_EIData
{
  char *name;                   /*!< the symbol name. */
  _Plink_Key def;               /*!< the key of this extern's definition. */
  List users;                   /*!< a list of Plink_Keys of table entries
				 * that reference this extern.  Once the
				 * definition's key is known, these must
				 * be linked to it. */
}
_Plink_EIData, *Plink_EIData;

/*! A struct to map one string to another. */
typedef struct Plink_StringMap
{
  char *lhs;                    /*!< the left side of the mapping. */
  char *rhs;                    /*!< the right side of the mapping. */
}
_Plink_StringMap, *Plink_StringMap;

/*! ExternInfo is a list of Plink_EIData structs. */
typedef List ExternInfo;

extern Plink_Key Plink_AllocKey (void);
extern Plink_Key Plink_AllocKeyWithFileKey (int f, Key k);
extern Plink_Key Plink_FreeKey (Plink_Key p);
extern Plink_InputFile Plink_AllocInputFile (void);
extern Plink_InputFile Plink_AllocInputFileWithTypeName (_Plink_InputType type,
							 char *name);
extern Plink_InputFile Plink_FreeInputFile (Plink_InputFile p);
extern Plink_SymTabEntryExt Plink_AllocSymTabEntryExt (void);
extern Plink_SymTabEntryExt Plink_FreeSymTabEntryExt (Plink_SymTabEntryExt p);
extern Plink_IPSymTabEntExt Plink_AllocIPSymTabEntExt (void);
extern Plink_IPSymTabEntExt Plink_FreeIPSymTabEntExt (Plink_IPSymTabEntExt p);
extern Plink_TypeDclExt Plink_AllocTypeDclExt (void);
extern Plink_TypeDclExt Plink_FreeTypeDclExt (Plink_TypeDclExt p);
extern Plink_TypeDclExt Plink_CopyTypeDclExt (Plink_TypeDclExt p);
extern char *Plink_WriteTypeDclExt (char *sig, Plink_TypeDclExt p);
extern Plink_StructDclExt Plink_AllocStructDclExt (void);
extern Plink_StructDclExt Plink_FreeStructDclExt (Plink_StructDclExt p);
extern Plink_StructDclExt Plink_CopyStructDclExt (Plink_StructDclExt p);
extern char *Plink_WriteStructDclExt (char *sig, Plink_StructDclExt p);
extern Plink_UnionDclExt Plink_AllocUnionDclExt (void);
extern Plink_UnionDclExt Plink_FreeUnionDclExt (Plink_UnionDclExt p);
extern Plink_UnionDclExt Plink_CopyUnionDclExt (Plink_UnionDclExt p);
extern char *Plink_WriteUnionDclExt (char *sig, Plink_UnionDclExt p);
extern Plink_FuncDclExt Plink_AllocFuncDclExt (void);
extern Plink_FuncDclExt Plink_FreeFuncDclExt (Plink_FuncDclExt p);
extern Plink_FuncDclExt Plink_CopyFuncDclExt (Plink_FuncDclExt p);
extern Plink_EIData Plink_AllocEIData (void);
extern Plink_EIData Plink_FreeEIData (Plink_EIData p);
extern Plink_StringMap Plink_AllocStringMap (void);
extern Plink_StringMap Plink_FreeStringMap (Plink_StringMap p);

/*! \brief Sets the Plink_Key.file field.
 *
 * \param p
 *  the Plink_Key.
 * \param f
 *  the new value of the Plink_Key.file field.
 */
#define Plink_SetKeyFile(p, f) ((p)->file = (f))

/*! \brief Gets the Plink_Key.file field.
 *
 * \param p
 *  the Plink_Key.
 *
 * \return
 *  The value of the Plink_Key.file field.
 */
#define Plink_GetKeyFile(p) ((p)->file)

/*! \brief Sets the Plink_Key.key field.
 *
 * \param p
 *  the Plink_Key.
 * \param k
 *  the new value of the Plink_Key.key field.
 */
#define Plink_SetKeyKey(p, k) ((p)->key = (k))

/*! \brief Gets the Plink_Key.key field.
 *
 * \param p
 *  the Plink_Key.
 *
 * \return
 *  The value of the Plink_Key.key field.
 */
#define Plink_GetKeyKey(p) ((p)->key)

/*! \brief Sets the SymTabEntry.(Plink_SymTabEntryExt).key.file field.
 *
 * \param e
 *  the SymTabEntry.
 * \param f
 *  the new value of the file field.
 */
#define Plink_SetSymTabEntryFile(e, f) \
          (((Plink_SymTabEntryExt)P_GetSymTabEntryExtM (e))->key.file = (f))

/*! \brief Gets the SymTabEntry.(Plink_SymTabEntryExt).key.file field.
 *
 * \param e
 *  the SymTabEntry.
 *
 * \return
 *  The value of the file field.
 */
#define Plink_GetSymTabEntryFile(e) \
          (((Plink_SymTabEntryExt)P_GetSymTabEntryExtM (e))->key.file)

/*! \brief Sets the SymTabEntry.(Plink_SymTabEntryExt).key.key field.
 *
 * \param e
 *  the SymTabEntry.
 * \param k
 *  the new value of the key field.
 */
#define Plink_SetSymTabEntryKey(e, k) \
          (((Plink_SymTabEntryExt)P_GetSymTabEntryExtM (e))->key.key = (k))

/*! \brief Gets the SymTabEntry.(Plink_SymTabEntryExt).key.key field.
 *
 * \param e
 *  the SymTabEntry.
 *
 * \return
 *  The value of the key field.
 */
#define Plink_GetSymTabEntryKey(e) \
          (((Plink_SymTabEntryExt)P_GetSymTabEntryExtM (e))->key.key)

/*! \brief Sets a flag for a SymTabEntry.
 *
 * \param e
 *  the SymTabEntry.
 * \param f
 *  the flag to set.
 */
#define Plink_SetSymTabEntryFlags(e, f) \
          (((Plink_SymTabEntryExt)P_GetSymTabEntryExtM ((e)))->flags |= (f))

/*! \brief Gets the flags for a SymTabEntry.
 *
 * \param e
 *  the SymTabEntry.
 *
 * \return
 *  The flags for the SymTabEntry.
 */
#define Plink_GetSymTabEntryFlags(e) \
          (((Plink_SymTabEntryExt)P_GetSymTabEntryExtM ((e)))->flags)

/*! \brief Clears a flag for a SymTabEntry.
 *
 * \param e
 *  the SymTabEntry.
 * \param f
 *  the flag to clear.
 */
#define Plink_ClrSymTabEntryFlags(e, f) \
          (((Plink_SymTabEntryExt)P_GetSymTabEntryExtM ((e)))->flags &= ~(f))

/*! \brief Sets the source file for a SymTabEntry.
 *
 * \param e
 *  the SymTabEntry.
 * \param s
 *  the new value of the src_file field.
 */
#define Plink_SetSymTabEntrySrcFile(e, s) \
          (((Plink_SymTabEntryExt)P_GetSymTabEntryExtM ((e)))->src_file |= (s))

/*! \brief Gets the source file for a SymTabEntry.
 *
 * \param e
 *  the SymTabEntry.
 *
 * \return
 *  The value of the src_file field for the SymTabEntry.
 */
#define Plink_GetSymTabEntrySrcFile(e) \
          (((Plink_SymTabEntryExt)P_GetSymTabEntryExtM ((e)))->src_file)

/*! \brief Sets the SymTabEntryExt.users field.
 *
 * \param e
 *  the SymTabEntry
 * \param u
 *  the new value of the users field.
 */
#define Plink_SetSymTabEntryUsers(e, u) \
          (((Plink_SymTabEntryExt)P_GetSymTabEntryExtM ((e)))->users = (u))

extern void Plink_AppendSymTabEntryUsers (SymTabEntry e, int f, Key k);
extern void Plink_DeleteSymTabEntryUsers (SymTabEntry e, int f, Key k);

/*! \brief Gets the SymTabEntryExt.users field.
 *
 * \param e
 *  the SymTabEntry
 *
 * \return
 *  The value of the SymTabEntryExt.users field.
 */
#define Plink_GetSymTabEntryUsers(e) \
          (((Plink_SymTabEntryExt)P_GetSymTabEntryExtM ((e)))->users)

/*! \brief Sets the Plink_IPSymTabEntExt.source_table field.
 *
 * \param i
 *  the interprocedural symbol table.
 * \param k
 *  the source file's key in \a i.
 * \param s
 *  the new value of the Plink_IPSymTabEntExt.source_table field.
 */
#define Plink_SetSourceTable(i, k, s) \
          (((Plink_IPSymTabEntExt)P_GetIPSymTabEntExtM \
                                    (PST_GetFile ((i), \
						  (k))))->source_table = (s))

/*! \brief Gets the Plink_IPSymTabEntExt.source_table field.
 *
 * \param i
 *  the interprocedural symbol table.
 * \param k
 *  the source file's key in \a i.
 *
 * \return
 *  The value of the Plink_IPSymTabEntExt.source_table field.
 */
#define Plink_GetSourceTable(i, k) \
          (((Plink_IPSymTabEntExt)P_GetIPSymTabEntExtM \
                                    (PST_GetFile ((i), (k))))->source_table)

/*! \brief Sets the signature for a TypeDcl.
 *
 * \param t
 *  the TypeDcl.
 * \param s
 *  the new value of the TypeDcl.(Plink_TypeDclExt).sig field.
 */
#define Plink_SetTypeDclSignature(t, s) \
          (((Plink_TypeDclExt)P_GetTypeDclExtM ((t)))->sig = (s))

/*! \brief Gets the signature for a TypeDcl.
 *
 * \param t
 *  the TypeDcl.
 *
 * \return
 *  The value of the TypeDcl.(Plink_TypeDclExt).sig field.
 */
#define Plink_GetTypeDclSignature(t) \
          (((Plink_TypeDclExt)P_GetTypeDclExtM ((t)))->sig)

/*! \brief Sets the multi_type for a TypeDcl.
 *
 * \param t
 *  the TypeDcl.
 * \param m
 *  the new value of the TypeDcl.(Plink_TypeDclExt).multi_type field.
 */
#define Plink_SetTypeDclMultiType(t, m) \
          (((Plink_TypeDclExt)P_GetTypeDclExtM ((t)))->multi_type = (m))

/*! \brief Gets the multi_type for a TypeDcl.
 *
 * \param t
 *  the TypeDcl.
 *
 * \return
 *  The value of the TypeDcl.(Plink_TypeDclExt).multi_type field.
 */
#define Plink_GetTypeDclMultiType(t) \
          (((Plink_TypeDclExt)P_GetTypeDclExtM ((t)))->multi_type)

extern Key Plink_GetTypeMultiType (SymbolTable ip_table, Key k);

/*! \brief Sets the signature for a StructDcl.
 *
 * \param s
 *  the StructDcl.
 * \param t
 *  the new value of the StructDcl.(Plink_StructDclExt).sig field.
 */
#define Plink_SetStructDclSignature(s, t) \
          (((Plink_StructDclExt)P_GetStructDclExtM ((s)))->sig = (t))

/*! \brief Gets the signature for a StructDcl.
 *
 * \param s
 *  the StructDcl.
 *
 * \return
 *  The value of the StructDcl.(Plink_StructDclExt).sig field.
 */
#define Plink_GetStructDclSignature(s) \
          (((Plink_StructDclExt)P_GetStructDclExtM ((s)))->sig)

/*! \brief Sets the multi_type for a StructDcl.
 *
 * \param s
 *  the StructDcl.
 * \param m
 *  the new value of the StructDcl.(Plink_StructDclExt).multi_type field.
 */
#define Plink_SetStructDclMultiType(s, m) \
          (((Plink_StructDclExt)P_GetStructDclExtM ((s)))->multi_type = (m))

/*! \brief Gets the multi_type for a StructDcl.
 *
 * \param s
 *  the StructDcl.
 *
 * \return
 *  The value of the StructDcl.(Plink_StructDclExt).multi_type field.
 */
#define Plink_GetStructDclMultiType(s) \
          (((Plink_StructDclExt)P_GetStructDclExtM ((s)))->multi_type)

/*! \brief Sets the signature for a UnionDcl.
 *
 * \param u
 *  the UnionDcl.
 * \param s
 *  the new value of the UnionDcl.(Plink_UnionDclExt).sig field.
 */
#define Plink_SetUnionDclSignature(u, s) \
          (((Plink_UnionDclExt)P_GetUnionDclExtM ((u)))->sig = (s))

/*! \brief Gets the signature for a UnionDcl.
 *
 * \param u
 *  the UnionDcl.
 *
 * \return
 *  The value of the UnionDcl.(Plink_UnionDclExt).sig field.
 */
#define Plink_GetUnionDclSignature(u) \
          (((Plink_UnionDclExt )P_GetUnionDclExtM ((u)))->sig)

/*! \brief Sets the multi_hash for a UnionDcl.
 *
 * \param u
 *  the UnionDcl.
 * \param m
 *  the new value of the UnionDcl.(Plink_UnionDclExt).multi_hash field.
 */
#define Plink_SetUnionDclMultiHash(u, m) \
          (((Plink_UnionDclExt)P_GetUnionDclExtM ((u)))->multi_hash = (m))

/*! \brief Gets the multi_hash for a UnionDcl.
 *
 * \param u
 *  the UnionDcl.
 *
 * \return
 *  The value of the UnionDcl.(Plink_UnionDclExt).multi_hash field.
 */
#define Plink_GetUnionDclMultiHash(u) \
          (((Plink_UnionDclExt )P_GetUnionDclExtM ((u)))->multi_hash)

/*! \brief Sets the multi_type for a UnionDcl.
 *
 * \param u
 *  the UnionDcl.
 * \param m
 *  the new value of the UnionDcl.(Plink_UnionDclExt).multi_type field.
 */
#define Plink_SetUnionDclMultiType(u, m) \
          (((Plink_UnionDclExt)P_GetUnionDclExtM ((u)))->multi_type = (m))

/*! \brief Gets the multi_type for a UnionDcl.
 *
 * \param u
 *  the UnionDcl.
 *
 * \return
 *  The value of the UnionDcl.(Plink_UnionDclExt).multi_type field.
 */
#define Plink_GetUnionDclMultiType(u) \
          (((Plink_UnionDclExt)P_GetUnionDclExtM ((u)))->multi_type)

/*! \brief Sets a flag for a FuncDcl.
 *
 * \param f
 *  the FuncDcl.
 * \param g
 *  the flag to set.
 */
#define Plink_SetFuncDclFlags(f, g) \
          (((Plink_FuncDclExt)P_GetFuncDclExtM ((f)))->flags |= (g))

/*! \brief Gets the flags for a FuncDcl.
 *
 * \param f
 *  the FuncDcl.
 *
 * \return
 *  The flags for the FuncDcl.
 */
#define Plink_GetFuncDclFlags(f) \
          (((Plink_FuncDclExt)P_GetFuncDclExtM ((f)))->flags)

/*! \brief Clears a flag for a FuncDcl.
 *
 * \param f
 *  the FuncDcl.
 * \param g
 *  the flag to clear.
 */
#define Plink_ClrFuncDclFlags(f, g) \
          (((Plink_FuncDclExt)P_GetFuncDclExtM ((f)))->flags &= ~(g))

/*! \brief Test a flag for a FuncDcl.
 *
 * \param f
 *  the FuncDcl.
 * \param g
 *  the flag to test.
 *
 * \return
 *  Returns non-zero if \a g is set.  Otherwise, returns 0.
 */
#define Plink_TstFuncDclFlags(f, g) \
          ((((Plink_FuncDclExt)P_GetFuncDclExtM ((f)))->flags & (g)) == (g))

/*! \brief Frees an ExternInfo list and all sub structures.
 *
 * \param e
 *  the ExternInfo list to free.
 *
 * \return A null ExternInfo pointer
 *
 * Frees an ExternInfo list and all sub structures.
 */
#define Plink_RemoveExternInfo(e) \
          ((ExternInfo)P_RemoveList ((List)(d), \
                                     (void *(*)(void *))Plink_free_ei_data))

/*! \brief Sets the \a name field of a Plink_EIData struct.
 *
 * \param e
 *  the Plink_EIData struct.
 * \param n
 *  the new name.
 */
#define Plink_SetEIDataName(e, n) (((Plink_EIData)(e))->name = (n))

/*! \brief Gets the \a name field of a Plink_EIData struct.
 *
 * \param e
 *  the Plink_EIData struct.
 *
 * \return
 *  The value of the name field.
 */
#define Plink_GetEIDataName(e) (((Plink_EIData)(e))->name)

/*! \brief Sets the \a def.file field of a Plink_EIData struct.
 *
 * \param e
 *  the Plink_EIData struct.
 * \param f
 *  the new file.
 */
#define Plink_SetEIDataFile(e, f) (((Plink_EIData)(e))->def.file = (f))

/*! \brief Gets the \a def.file field of a Plink_EIData struct.
 *
 * \param e
 *  the Plink_EIData struct.
 *
 * \return
 *  The value of the def.lfile field.
 */
#define Plink_GetEIDataFile(e) (((Plink_EIData)(e))->def.file)

/*! \brief Sets the \a def.key field of a Plink_EIData struct.
 *
 * \param e
 *  the Plink_EIData struct.
 * \param k
 *  the new def.key.
 */
#define Plink_SetEIDataKey(e, k) (((Plink_EIData)(e))->def.key = (k))

/*! \brief Gets the \a def.key field of a Plink_EIData struct.
 *
 * \param e
 *  the Plink_EIData struct.
 *
 * \return
 *  The value of the def.key field.
 */
#define Plink_GetEIDataKey(e) (((Plink_EIData)(e))->def.key)

/*! \brief Appends a Plink_Key to the \a users field of a Plink_EIData struct.
 *
 * \param e
 *  the Plink_EIData struct.
 * \param f
 *  the file for the new Plink_Key.
 * \param k
 *  the key for the new Plink_Key.
 */
#define Plink_AppendEIDataUsers(e, f, k) \
          (((Plink_EIData)(e))->users = \
            List_insert_last (((Plink_EIData)(e))->users, \
                              Plink_AllocKeyWithFileKey ((f), (k))))

/*! \brief Gets the \a users field of a Plink_EIData struct.
 *
 * \param e
 *  the Plink_EIData struct.
 *
 * \return
 *  The value of the users field.
 *
 * \note This macro returns a ::List, not a ::Plink_Key.  Use List_start()
 * and List_next() to get the Plink_Keys.
 */
#define Plink_GetEIDataUsers(e) (((Plink_EIData)(e))->users)

extern void Plink_SetLink (SymbolTable ip_table, int o_file, Key o_key,
			   int t_file, Key t_key,
			   _Plink_SymTabEntryFlags type);
extern _Plink_SymTabEntryFlags Plink_GetLinkType (SymbolTable ip_table,
						  int o_file, Key o_key);
extern int Plink_GetLinkFile (SymbolTable ip_table, int o_file, Key o_key);
extern Key Plink_GetLinkKey (SymbolTable ip_table, int o_file, Key o_key);

extern Plink_EIData Plink_FindExtern (char *name);
extern Plink_EIData Plink_GetFirstExtern ();
extern Plink_EIData Plink_GetNextExtern ();
extern Plink_EIData Plink_GetFirstUndefinedExtern ();
extern Plink_EIData Plink_GetNextUndefinedExtern ();
extern void Plink_CleanupExterns ();
extern List Plink_AddMapping (List map, char *lhs, char *rhs);
extern List Plink_DelMapping (List map, char *lhs, char *rhs);
extern bool Plink_FindMapping (List map, char **lhs, char **rhs);
extern bool Plink_FindMappingNext (List map, char **lhs, char **rhs);

extern SymTabEntry Plink_GetSymTabEntry (SymbolTable ip_table, int file,
					 Key key);

extern Key Plink_GetIPKey (SymbolTable ip_table, int src_file_key,
			   Key src_entry_key);

extern SymTabEntry Plink_GetTypeDclType (SymbolTable ip_table,
					 SymTabEntry type_entry);
extern SymTabEntry Plink_GetTypeDclParam (SymbolTable ip_table,
					  SymTabEntry type_entry,
					  Param *param);
extern SymTabEntry Plink_GetStructDclField (SymbolTable ip_table,
					    SymTabEntry struct_entry,
					    Field *field);
extern SymTabEntry Plink_GetUnionDclField (SymbolTable ip_table,
					   SymTabEntry union_entry,
					   Field *field);
extern SymTabEntry Plink_GetFieldType (SymbolTable ip_table,
				       SymTabEntry field_entry);

extern SymTabEntry Plink_FollowLinks (SymbolTable ip_table,
				      SymTabEntry src_entry);

#endif
