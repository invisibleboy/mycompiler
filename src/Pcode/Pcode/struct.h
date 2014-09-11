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
 * \author David August, Nancy Warter, Grant Haab, Krishna Subramanian
 * and Wen-mei Hwu
 *
 * Modified from code written by: Po-hua Chang
 *
 * Copyright (c) 1991 Nancy Warter, Grant Haab, Krishna Subramanian,
 * Po-hua Chang, Wen-mei Hwu, and The Board of Trustees of the University
 * of Illinois.
 * All rights reserved.
 *
 * This file declares all basic functions that operate on the internal
 * Pcode data structures.
 */
/*****************************************************************************/

#ifndef _PCODE_STRUCT_H_
#define _PCODE_STRUCT_H_

#include <config.h>
#include <stdio.h>
#include <library/i_types.h>
#include <library/i_list.h>
#include <library/i_hash.h>
#include "pcode.h"
#include "impact_global.h"
#include "parms.h"

/*-------------------------------------------------------------*/
/* Contents of struct.c                                        */
/*-------------------------------------------------------------*/
/*   Memory Allocation Functions
 *   Functions to allocate new data structures
 *   Functions to remove data structures
 *   Functions to access data in structures
 *   Copy functions
 */

/* Memory Allocation Functions */

extern void *P_alloc (int size);
extern void P_free (void *pointer, int size);
#define ALLOCATE(x)	(x *) P_alloc (sizeof(x))
#define DISPOSE(x)      P_free((void *)x, sizeof(*x))

/* Defines to indicate the direction a function walks a doubly linked list. */
#define FORWARD 0
#define REVERSE 1

/* Functions to allocate new data structures */

extern Key *P_NewKeyP (void);
extern Key *P_NewKeyPWithKey (Key k);
extern Dcl P_NewDcl (void);
extern Dcl P_NewDclWithDclType (void *dcl, _DclType type);
extern FuncDcl P_NewFuncDcl (void);
extern TypeDcl P_NewTypeDcl (void);
extern TypeDcl P_NewTypeDclWithBasicType (_BasicType basic_type);
extern KeyList P_NewKeyList (void);
extern KeyList P_NewKeyListWithKey (Key k);

/*! \brief Allocates a new Param.
 *
 * \return A pointer to the new Param.
 *
 * Allocates a new Param or aborts program if allocation fails.
 * Initializes the new Param to zeros.
 *
 * \sa #P_NewParamWithKey(), #P_FreeParam(), #P_RemoveParamNode(),
 * #P_RemoveParam()
 */
#define P_NewParam() ((Param)P_NewKeyList ())

/*! \brief Allocates a new Param and assigns a key.
 *
 * \param k
 *  the key for the new Param.
 *
 * \return A pointer to the new Param.
 *
 * Allocates a new Param or aborts program if allocation fails.
 * Initializes the new Param to zeros and sets Param.key to \a k.
 *
 * \sa #P_NewParam(), #P_FreeParam(), #P_RemoveParamNode(), #P_RemoveParam()
 */
#define P_NewParamWithKey(k) ((Param)P_NewKeyListWithKey ((k)))

extern VarDcl P_NewVarDcl (void);
extern Init P_NewInit (void);
extern StructDcl P_NewStructDcl (void);
extern UnionDcl P_NewUnionDcl (void);
extern Field P_NewField (void);
extern EnumDcl P_NewEnumDcl (void);
extern EnumField P_NewEnumField (void);
extern Stmt P_NewStmt (void);
extern Stmt P_NewStmtWithType (_StmtType t);
extern Stmt P_NewGotoStmt (Label l);
extern Stmt P_NewExprStmt (Expr e);
extern Label P_NewLabel (void);
extern Compound P_NewCompound (void);
extern IfStmt P_NewIfStmt (void);
extern SwitchStmt P_NewSwitchStmt (void);
extern Pstmt P_NewPstmt (void);
extern Advance P_NewAdvance (void);
extern Await P_NewAwait (void);
extern Mutex P_NewMutex (void);
extern Cobegin P_NewCobegin (void);
extern BodyStmt P_NewBodyStmt (void);
extern EpilogueStmt P_NewEpilogueStmt (void);
extern ParLoop P_NewParLoop (void);
extern SerLoop P_NewSerLoop (void);
extern AsmStmt P_NewAsmStmt (void);
extern Asmoprd P_NewAsmoprd (void);
extern Expr P_NewExpr (void);
extern Expr P_NewExprWithOpcode (_Opcode o);
extern Expr P_NewStringExpr (char *s);
extern Expr P_NewIntExpr (ITintmax i);
extern Expr P_NewUIntExpr (ITuintmax i);
extern Expr P_NewFloatExpr (double f);
extern Expr P_NewDoubleExpr (double d);
extern Pragma P_NewPragma (void);
extern Pragma P_NewPragmaWithSpecExpr (char *s, Expr e);
extern void P_AddStmtPragma (Pragma *prag, char *spec, Expr expr);
extern Position P_NewPosition (void);
extern Identifier P_NewIdentifier (void);
extern ProfFN P_NewProfFN (void);
extern ProfCS P_NewProfCS (void);
extern ProfBB P_NewProfBB (void);
extern ProfArc P_NewProfArc (void);
extern ProfST P_NewProfST (void);
extern ProfST P_NewProfST_w_wt (double wt);
extern ProfEXPR P_NewProfEXPR (void);
extern ProfEXPR P_NewProfEXPR_w_wt (double wt);
extern Shadow P_NewShadow (void);
extern Shadow P_NewShadowWithExprID (Shadow slist, Expr expr, int id);
extern AsmDcl P_NewAsmDcl (void);
extern Scope P_NewScope (void);
extern Scope P_NewScopeWithKey (Key k);

/*! \brief Allocates a new ScopeEntry.
 *
 * \return A pointer to the new ScopeEntry.
 *
 * Allocates a new ScopeEntry or aborts program if allocation fails.
 * Initializes the new ScopeEntry to zeros.
 *
 * \sa #P_NewScopeEntryWithKey(), #P_FreeScopeEntry(),
 * #P_RemoveScopeEntryNode(), #P_RemoveScopeEntry()
 */
#define P_NewScopeEntry() ((ScopeEntry)P_NewKeyList ())

/*! \brief Allocates a new ScopeEntry and assigns a key.
 *
 * \param k
 *  the key for the new ScopeEntry.
 *
 * \return A pointer to the new ScopeEntry.
 *
 * Allocates a new ScopeEntry or aborts program if allocation fails.
 * Initializes the new ScopeEntry to zeros and sets ScopeEntry.key to \a k.
 *
 * \sa #P_NewScopeEntry(), #P_FreeScopeEntry(), #P_RemoveScopeEntryNode(),
 * #P_RemoveScopeEntry()
 */
#define P_NewScopeEntryWithKey(k) ((ScopeEntry)P_NewKeyListWithKey ((k)))

extern SymTabEntry P_NewSymTabEntry (void);
extern IPSymTabEnt P_NewIPSymTabEnt (void);
extern SymbolTable P_NewSymbolTable (void);
extern KeyMap P_NewKeyMap (void);

/* Functions to deallocate data structures */
/* The P_Free* functions simply free the given structure.  They do nothing to
 * any data structures inside the structure.  The P_Remove* function free
 * the given structure and all sub structures. */
extern List P_RemoveList (List l, void *(*free_data)(void *));

/*! \brief Frees a DclList list and all sub structures.
 *
 * \param d
 *  the DclList list to free.
 *
 * \return A null DclList pointer.
 *
 * Frees a DclList list and all sub structures.
 *
 * \sa P_RemoveList()
 */
#define P_RemoveDclList(d) \
          ((DclList)P_FreeList ((List)(d), (void *(*)(void *))P_RemoveDcl))

extern Key *P_FreeKeyP (Key *k);

/*! \brief Removes a Key struct.
 *
 * \param k
 *  the Key pointer to free.
 *
 * \return
 *  A null Key pointer.
 *
 * \note This macro is provided only for consistency with the P_Free/P_Remove
 *       scheme.
 * \note Keys are typically statically allocated, so this function is
 *       not usually needed.
 *
 * \sa P_NewKeyP(), P_NewKeyPWithKey(), P_FreeKeyP()
 */
#define P_RemoveKeyP(k) (P_FreeKeyP ((k)))

extern Dcl P_FreeDcl (Dcl d);
extern Dcl P_RemoveDcl (Dcl d);
extern FuncDcl P_FreeFuncDcl (FuncDcl f);
extern FuncDcl P_RemoveFuncDcl (FuncDcl f);
extern TypeDcl P_FreeTypeDcl (TypeDcl t);
extern TypeDcl P_RemoveTypeDcl (TypeDcl t);

/*! \brief Frees a TypeList list and all sub structures.
 *
 * \param t
 *  the TypeList list to free.
 *
 * \return A null TypeList pointer.
 *
 * Frees a TypeList list and all sub structures.
 *
 * \sa P_RemoveList()
 */
#define P_RemoveTypeList(t) \
          ((TypeList)P_RemoveList ((List)(t), \
				   (void *(*)(void *))P_RemoveTypeDcl))

extern KeyList P_FreeKeyList (KeyList l);
extern KeyList P_RemoveKeyListNode (KeyList l);
extern KeyList P_RemoveKeyList (KeyList l);

/*! \brief Frees a Param struct.
 *
 * \param p
 *  the Param struct to free.
 *
 * \return A null Param pointer.
 *
 * Frees a Param without freeing sub structures.  This function frees
 * a single list node, not the entire list.
 *
 * If the global \a check_fields_on_free is true, function will print a warning
 * for any non-null pointer field.
 *
 * \sa #P_RemoveParamNode(), #P_RemoveParam(), #P_NewParam(),
 * #P_NewParamWithKey()
 */
#define P_FreeParam(p) ((Param)P_FreeVarList ((KeyList)(p)))

/*! \brief Frees a Param node and all sub structures.
 *
 * \param p
 *  the Param node to free.
 *
 * \return A null Param pointer.
 *
 * Fres a Param and all sub structures.  This function frees a single
 * list node, not the entire list..
 *
 * If the global \a check_fields_on_free is true, function will print a
 * warning for any non-null pointer field.
 *
 * \sa #P_FreeParam(), #P_RemoveParam(), #P_NewParam(), #P_NewParamWithKey()
 */
#define P_RemoveParamNode(p) ((Param)P_RemoveKeyListNode ((KeyList)(p)))

/*! \brief Frees a Param list and all sub structures.
 *
 * \param p
 *  the Param list to free
 *
 * \return A null Param pointer.
 *
 * Frees a Param list and all sub structures.
 *
 * \sa #P_FreeParam(), #P_RemoveParamNode(), #P_NewParam(),
 * #P_NewParamWithKey()
 */
#define P_RemoveParam(p) ((Param)P_RemoveKeyList ((KeyList)(p)))

/*! \brief Frees a VarList list and all sub structures.
 *
 * \param l
 *  the VarList list to free.
 *
 * \return A null VarList pointer.
 *
 * Frees a VarList list and all sub structures.
 *
 * \sa P_RemoveList()
 */
#define P_RemoveVarList(l) \
          ((VarList)P_RemoveList ((List)(l), \
                                  (void *(*)(void *))P_RemoveVarDcl))

extern VarDcl P_FreeVarDcl (VarDcl v);
extern VarDcl P_RemoveVarDcl (VarDcl v);
extern Init P_FreeInit (Init i);
extern Init P_RemoveInitNode (Init i);
extern Init P_RemoveInit (Init i);
extern StructDcl P_FreeStructDcl (StructDcl s);
extern StructDcl P_RemoveStructDcl (StructDcl s);
extern UnionDcl P_FreeUnionDcl (UnionDcl u);
extern UnionDcl P_RemoveUnionDcl (UnionDcl u);
extern Field P_FreeField (Field f);
extern Field P_RemoveFieldNode (Field f);
extern Field P_RemoveField (Field f);
extern EnumDcl P_FreeEnumDcl (EnumDcl e);
extern EnumDcl P_RemoveEnumDcl (EnumDcl e);
extern EnumField P_FreeEnumField (EnumField f);
extern EnumField P_RemoveEnumFieldNode (EnumField f);
extern EnumField P_RemoveEnumField (EnumField f);
extern Stmt P_FreeStmt (Stmt s);
extern Stmt P_RemoveStmtNode (Stmt s);
extern Stmt P_RemoveStmt (Stmt s);
extern Label P_FreeLabel (Label l);
extern Label P_RemoveLabelNode (Label l);
extern Label P_RemoveLabel (Label l);
extern Compound P_FreeCompound (Compound c);
extern Compound P_RemoveCompound (Compound c);
extern IfStmt P_FreeIfStmt (IfStmt i);
extern IfStmt P_RemoveIfStmt (IfStmt i);
extern SwitchStmt P_FreeSwitchStmt (SwitchStmt s);
extern SwitchStmt P_RemoveSwitchStmt (SwitchStmt s);
extern Pstmt P_FreePstmt (Pstmt p);
extern Pstmt P_RemovePstmt (Pstmt p);
extern Advance P_FreeAdvance (Advance a);
extern Advance P_RemoveAdvance (Advance a);
extern Await P_FreeAwait (Await a);
extern Await P_RemoveAwait (Await a);
extern Mutex P_FreeMutex (Mutex m);
extern Mutex P_RemoveMutex (Mutex m);
extern Cobegin P_FreeCobegin (Cobegin c);
extern Cobegin P_RemoveCobegin (Cobegin c);
extern BodyStmt P_FreeBodyStmt (BodyStmt b);
extern BodyStmt P_RemoveBodyStmt (BodyStmt b);
extern EpilogueStmt P_FreeEpilogueStmt (EpilogueStmt e);
extern EpilogueStmt P_RemoveEpilogueStmt (EpilogueStmt e);
extern ParLoop P_FreeParLoop (ParLoop p);
extern ParLoop P_RemoveParLoopNode (ParLoop p);
extern ParLoop P_RemoveParLoop (ParLoop p);
extern SerLoop P_FreeSerLoop (SerLoop s);
extern SerLoop P_RemoveSerLoop (SerLoop s);
extern AsmStmt P_FreeAsmStmt (AsmStmt a);
extern AsmStmt P_RemoveAsmStmt (AsmStmt a);
extern Asmoprd P_FreeAsmoprd (Asmoprd a);
extern Asmoprd P_RemoveAsmoprd (Asmoprd a);
extern Expr P_FreeExpr (Expr e);
extern Expr P_RemoveExprNode (Expr e);
extern Expr P_RemoveExpr (Expr e);	/* REMOVES EXPRESSION AND ALL SIBLING,
					 * NEXT AND OPERAND EXPRESSIONS */
extern Pragma P_FreePragma (Pragma p);
extern Pragma P_RemovePragmaNode (Pragma p);
extern Pragma P_RemovePragma (Pragma p);
extern Position P_FreePosition (Position p);
extern Position P_RemovePosition (Position p);
extern Identifier P_FreeIdentifier (Identifier i);
extern Identifier P_RemoveIdentifier (Identifier i);
extern ProfFN P_FreeProfFN (ProfFN p);
extern ProfFN P_RemoveProfFN (ProfFN p);
extern ProfCS P_FreeProfCS (ProfCS p);
extern ProfCS P_RemoveProfCSNode (ProfCS p);
extern ProfCS P_RemoveProfCS (ProfCS p);
extern ProfBB P_FreeProfBB (ProfBB p);
extern ProfBB P_RemoveProfBB (ProfBB p);
extern ProfArc P_FreeProfArc (ProfArc p);
extern ProfArc P_RemoveProfArcNode (ProfArc p);
extern ProfArc P_RemoveProfArc (ProfArc p);
extern ProfST P_FreeProfST (ProfST p);
extern ProfST P_RemoveProfSTNode (ProfST p);
extern ProfST P_RemoveProfST (ProfST p);
extern ProfEXPR P_FreeProfEXPR (ProfEXPR p);
extern ProfEXPR P_RemoveProfEXPRNode (ProfEXPR p);
extern ProfEXPR P_RemoveProfEXPR (ProfEXPR p);
extern Shadow P_FreeShadow (Shadow s);
extern Shadow P_RemoveShadow (Shadow s);
extern AsmDcl P_FreeAsmDcl (AsmDcl a);
extern AsmDcl P_RemoveAsmDcl (AsmDcl a);
extern Scope P_FreeScope (Scope s);
extern Scope P_RemoveScope (Scope s);

/*! \brief Frees a ScopeEntry struct.
 *
 * \param s
 *  the ScopeEntry struct to free.
 *
 * \return A null ScopeEntry pointer.
 *
 * Frees a ScopeEntry without freeing sub structures.  This function frees
 * a single list node, not the entire list.
 *
 * If the global \a check_fields_on_free is true, function will print a warning
 * for any non-null pointer field.
 *
 * \sa #P_RemoveScopeEntryNode(), #P_RemoveScopeEntry(), #P_NewScopeEntry(),
 * #P_NewScopeEntryWithKey()
 */
#define P_FreeScopeEntry(s) ((ScopeEntry)P_FreeVarList ((KeyList)(s)))

/*! \brief Frees a ScopeEntry node and all sub structures.
 *
 * \param s
 *  the ScopeEntry node to free.
 *
 * \return A null ScopeEntry pointer.
 *
 * Fres a ScopeEntry and all sub structures.  This function frees a single
 * list node, not the entire list..
 *
 * If the global \a check_fields_on_free is true, function will print a
 * warning for any non-null pointer field.
 *
 * \sa #P_FreeScopeEntry(), #P_RemoveScopeEntry(), #P_NewScopeEntry(),
 * #P_NewScopeEntryWithKey()
 */
#define P_RemoveScopeEntryNode(s) \
          ((ScopeEntry)P_RemoveKeyListNode ((KeyList)(s)))

/*! \brief Frees a ScopeEntry list and all sub structures.
 *
 * \param s
 *  the ScopeEntry list to free
 *
 * \return A null ScopeEntry pointer.
 *
 * Frees a ScopeEntry list and all sub structures.
 *
 * \sa #P_FreeScopeEntry(), #P_RemoveScopeEntryNode(), #P_NewScopeEntry(),
 * #P_NewScopeEntryWithKey()
 */
#define P_RemoveScopeEntry(s) ((ScopeEntry)P_RemoveKeyList ((KeyList)(s)))

extern Key P_NewKey (void);
extern SymTabEntry P_FreeSymTabEntry (SymTabEntry s);
extern SymTabEntry P_RemoveSymTabEntry (SymTabEntry s);
extern IPSymTabEnt P_FreeIPSymTabEnt (IPSymTabEnt i);
extern IPSymTabEnt P_RemoveIPSymTabEnt (IPSymTabEnt i);
extern SymbolTable P_FreeSymbolTable (SymbolTable s);
extern SymbolTable P_RemoveSymbolTable (SymbolTable s);
extern KeyMap P_FreeKeyMap (KeyMap k);
extern KeyMap P_RemoveKeyMap (KeyMap k);

#if 0
extern void P_RemoveDepInfo (DepInfo dep);
#endif
extern bool P_RemoveStmtPragma (Stmt stmt, Pragma pragma);
extern bool P_RemoveExprPragma (Expr expr, Pragma pragma);

/* Access functions */
/*! \addtogroup KeyPAF Key Pointer access functions */
/* @{ */
extern ITintmax P_Key2Long (Key k);
extern Key P_Long2Key (ITintmax l);
/* @} */

/*! \addtogroup DclAF Dcl access functions
 *
 * Access the fields of the Dcl struct. */
/* @{ */
#define P_SetDclType(d, t) ((d)->type = (t))
#define P_GetDclType(d) ((d)->type)
#define P_SetDclFuncDcl(d, f) ((d)->ptr.funcDcl = (f))
#define P_GetDclFuncDcl(d) ((d)->ptr.funcDcl)
#define P_SetDclTypeDcl(d, t) ((d)->ptr.typeDcl = (t))
#define P_GetDclTypeDcl(d) ((d)->ptr.typeDcl)
#define P_SetDclVarDcl(d, v) ((d)->ptr.varDcl = (v))
#define P_GetDclVarDcl(d) ((d)->ptr.varDcl)
#define P_SetDclStructDcl(d, s) ((d)->ptr.structDcl = (s))
#define P_GetDclStructDcl(d) ((d)->ptr.structDcl)
#define P_SetDclUnionDcl(d, u) ((d)->ptr.unionDcl = (u))
#define P_GetDclUnionDcl(d) ((d)->ptr.unionDcl)
#define P_SetDclEnumDcl(d, e) ((d)->ptr.enumDcl = (e))
#define P_GetDclEnumDcl(d) ((d)->ptr.enumDcl)
#define P_SetDclAsmDcl(d, a) ((d)->ptr.asmDcl = (a))
#define P_GetDclAsmDcl(d) ((d)->ptr.asmDcl)
#define P_SetDclInclude(d, i) ((d)->ptr.include = (i))
#define P_GetDclInclude(d) ((d)->ptr.include)
#define P_SetDclSymbolTable(d, s) ((d)->ptr.symbolTable = (s))
#define P_GetDclSymbolTable(d) ((d)->ptr.symbolTable)
#define P_SetDclIPSymTabEnt(d, i) ((d)->ptr.ipSymTabEnt = (i))
#define P_GetDclIPSymTabEnt(d) ((d)->ptr.ipSymTabEnt)
#define P_SetDclSymTabEntry(d, s) ((d)->ptr.symTabEntry = (s))
#define P_GetDclSymTabEntry(d) ((d)->ptr.symTabEntry)

extern Pragma P_SetDclPragma (Dcl d, Pragma p);
extern Pragma P_GetDclPragma (Dcl d);
extern Position P_SetDclPosition (Dcl d, Position p);
extern Position P_GetDclPosition (Dcl d);
extern Key P_SetDclKey (Dcl d, Key k);
extern Key P_GetDclKey (Dcl d);
/* @} */

/*! \addtogroup FuncDclAF FuncDcl access functions
 *
 * Access the fields of the FuncDcl struct. */
/* @{ */
#define P_SetFuncDclName(f, n) ((f)->name = (n))
#define P_GetFuncDclName(f) ((f)->name)
#define P_SetFuncDclKey(f, k) ((f)->key = (k))
#define P_GetFuncDclKey(f) ((f)->key)
#define P_SetFuncDclType(f, k) ((f)->type = (k))
#define P_GetFuncDclType(f) ((f)->type)
#define P_SetFuncDclLineno(f, l) ((f)->lineno = (l))
#define P_GetFuncDclLineno(f) ((f)->lineno)
#define P_SetFuncDclColno(f, c) ((f)->colno = (c))
#define P_GetFuncDclColno(f) ((f)->colno)
/*! Sets a bit in the FuncDcl.qualifier field. */
#define P_SetFuncDclQualifier(f, q) ((f)->qualifier |= (q))
#define P_GetFuncDclQualifier(f) ((f)->qualifier)
/*! Tests a bit pattern in the FuncDcl.qualifier field. */
#define P_TstFuncDclQualifier(f, q) (((f)->qualifier & (q)) == (q))
/*! Clears a bit in the FuncDcl.qualifier field. */
#define P_ClrFuncDclQualifier(f, q) ((f)->qualifier &= ~(q))
#define P_SetFuncDclFilename(f, g) ((f)->filename = (g))
#define P_GetFuncDclFilename(f) ((f)->filename)
#define P_SetFuncDclParam(f, p) ((f)->param = (p))
#define P_GetFuncDclParam(f) ((f)->param)

extern Stmt P_SetFuncDclStmt (FuncDcl f, Stmt s);

#define P_GetFuncDclStmt(f) ((f)->stmt)
#define P_SetFuncDclPragma(f, p) ((f)->pragma = (p))
#define P_GetFuncDclPragma(f) ((f)->pragma)
#define P_SetFuncDclProfile(f, p) ((f)->profile = (p))
#define P_GetFuncDclProfile(f) ((f)->profile)
#define P_SetFuncDclParLoop(f, p) ((f)->par_loop = (p))
#define P_GetFuncDclParLoop(f) ((f)->par_loop)
#define P_SetFuncDclLocal(f, l) ((f)->local = (l))
#define P_GetFuncDclLocal(f) ((f)->local)
#define P_SetFuncDclShadow(f, s) ((f)->shadow = (s))
#define P_GetFuncDclShadow(f) ((f)->shadow)
#define P_SetFuncDclMaxExprID(f, m) ((f)->max_expr_id = (m))
#define P_GetFuncDclMaxExprID(f) ((f)->max_expr_id)
/*! Sets the FuncDcl.ext field for a module */
#define P_SetFuncDclExtM(f, e) ((f)->ext[0] = (e))
/*! Gets the FuncDcl.ext field for a module */
#define P_GetFuncDclExtM(f) ((f)->ext[0])
/*! Sets the FuncDcl.ext field for a library
 *
 * \param f
 *  the FuncDcl.
 * \param i
 *  the library's index in the ext array.
 * \param e
 *  the new value of the ext field.
 *
 * \return
 *  The new value of the ext field.
 */
#define P_SetFuncDclExtL(f, i, e) ((f)->ext[(i)] = (e))
/*! Gets the FuncDcl.ext field for a library
 *
 * \param f
 *  the FuncDcl.
 * \param i
 *  the library's index in the ext array.
 *
 * \return
 *  The value of the ext field.
 */
#define P_GetFuncDclExtL(f, i) ((f)->ext[(i)])

extern Position P_SetFuncDclPosition (FuncDcl f, Position p);
extern Position P_GetFuncDclPosition (FuncDcl f);
extern Identifier P_SetFuncDclIdentifier (FuncDcl f, Identifier i);
extern Identifier P_GetFuncDclIdentifier (FuncDcl f);
/* @} */

/*! \addtogroup TypeDclAF TypeDcl access functions
 *
 * Access the fields of the TypeDcl struct.  The return value of any
 * P_SetTypeDcl* function is the new value of that field. */
/* @{ */
#define P_SetTypeDclBasicType(t, b) ((t)->basic_type = (b))
#define P_GetTypeDclBasicType(t) ((t)->basic_type)
/*! Sets a bit in the TypeDcl.qualifier field. */
#define P_SetTypeDclQualifier(t, q) ((t)->qualifier |= (q))
#define P_GetTypeDclQualifier(t) ((t)->qualifier)
/*! Tests a bit pattern in the TypeDcl.qualifier field. */
#define P_TstTypeDclQualifier(t, q) (((t)->qualifier & (q)) == (q))
/*! Clears a bit in the TypeDcl.qualifier field. */
#define P_ClrTypeDclQualifier(t, q) ((t)->qualifier &= ~(q))
#define P_SetTypeDclKey(t, k) ((t)->key = (k))
#define P_GetTypeDclKey(t) ((t)->key)
#define P_SetTypeDclType(t, k) ((t)->type = (k))
#define P_GetTypeDclType(t) ((t)->type)
#define P_SetTypeDclName(t, n) ((t)->name = (n))
#define P_GetTypeDclName(t) ((t)->name)
#define P_SetTypeDclArraySize(t, a) ((t)->details.array_size = (a))
#define P_GetTypeDclArraySize(t) ((t)->details.array_size)
#define P_SetTypeDclParam(t, p) ((t)->details.param = (p))
#define P_GetTypeDclParam(t) ((t)->details.param)
#define P_SetTypeDclSize(t, s) ((t)->size = (s))
#define P_GetTypeDclSize(t) ((t)->size)
#define P_SetTypeDclAlignment(t, a) ((t)->alignment = (a))
#define P_GetTypeDclAlignment(t) ((t)->alignment)
#define P_SetTypeDclLineno(t, l) ((t)->lineno = (l))
#define P_GetTypeDclLineno(t) ((t)->lineno)
#define P_SetTypeDclColno(t, c) ((t)->colno = (c))
#define P_GetTypeDclColno(t) ((t)->colno)
#define P_IncTypeDclRefs(t) ((t)->refs >= 0 ? ((t)->refs)++ : (t)->refs)
#define P_DecTypeDclRefs(t) ((t)->refs > 0 ? ((t)->refs)-- : (t)->refs)
#define P_SetTypeDclRefs(t, r) ((t)->refs = (r))
#define P_GetTypeDclRefs(t) ((t)->refs)
#define P_SetTypeDclFilename(t, f) ((t)->filename = (f))
#define P_GetTypeDclFilename(t) ((t)->filename)
#define P_SetTypeDclPragma(t, p) ((t)->pragma = (p))
#define P_GetTypeDclPragma(t) ((t)->pragma)
/*! Sets the TypeDcl.ext field for a module */
#define P_SetTypeDclExtM(t, e) ((t)->ext[0] = (e))
/*! Gets the TypeDcl.ext field for a module */
#define P_GetTypeDclExtM(t) ((t)->ext[0])
/*! Sets the TypeDcl.ext field for a library
 *
 * \param t
 *  the TypeDcl.
 * \param i
 *  the library's index in the ext array.
 * \param e
 *  the new value of the ext field.
 *
 * \return
 *  The new value of the ext field.
 */
#define P_SetTypeDclExtL(t, i, e) ((t)->ext[(i)] = (e))
/*! Gets the TypeDcl.ext field for a library
 *
 * \param t
 *  the TypeDcl.
 * \param i
 *  the library's index in the ext array.
 *
 * \return
 *  The value of the ext field.
 */
#define P_GetTypeDclExtL(t, i) ((t)->ext[(i)])

extern Position P_SetTypeDclPosition (TypeDcl t, Position p);
extern Position P_GetTypeDclPosition (TypeDcl t);
extern Identifier P_SetTypeDclIdentifier (TypeDcl t, Identifier i);
extern Identifier P_GetTypeDclIdentifier (TypeDcl t);
/* @} */

/*! \addtogroup KeyListAF KeyList access functions
 *
 * Access the fields of the KeyList struct.  The return value of any
 * P_SetKeyList* function is the new value of that field. */
/* @{ */
#define P_SetKeyListKey(l, k) ((l)->key = (k))
#define P_GetKeyListKey(l) ((l)->key)
#define P_SetKeyListNext(l, n) ((l)->next = (n))
#define P_GetKeyListNext(l) ((l)->next)
#define P_SetKeyListLast(l, m) ((l)->last = (m))
#define P_GetKeyListLast(l) ((l)->last)

extern KeyList P_AppendKeyListNext (KeyList l, KeyList n);
extern KeyList P_MergeKeyListNext (KeyList l, KeyList n);
extern KeyList P_DeleteKeyListNext (KeyList l, Key k);
/* @} */

/*! \addtogroup ParamAF Param access functions
 *
 * Access the fields of the Param struct.  The return value of any
 * P_SetParam* function is the new value of that field. */
/* @{ */
#define P_SetParamKey(p, k) (P_SetKeyListKey ((p), (k)))
#define P_GetParamKey(p) (P_GetKeyListKey ((p)))
#define P_SetParamNext(p, n) (P_SetKeyListNext ((p), (n)))
#define P_AppendParamNext(p, n) \
          ((Param)P_AppendKeyListNext ((KeyList)(p), (KeyList)(n)))
#define P_GetParamNext(p) (P_GetKeyListNext ((p)))
/* @} */

/*! \addtogroup VarDclAF VarDcl access functions
 *
 * Access the fields of the VarDcl struct.  The return value of any
 * P_SetVarDcl* function is the new value of that field. */
/* @{ */
#define P_SetVarDclName(v, n) ((v)->name = (n))
#define P_GetVarDclName(v) ((v)->name)
#define P_SetVarDclKey(v, k) ((v)->key = (k))
#define P_GetVarDclKey(v) ((v)->key)
#define P_SetVarDclType(v, t) ((v)->type = (t))
#define P_GetVarDclType(v) ((v)->type)
#define P_GetVarDclInit(v) ((v)->init)
#define P_SetVarDclLineno(v, l) ((v)->lineno = (l))
#define P_GetVarDclLineno(v) ((v)->lineno)
#define P_SetVarDclColno(v, c) ((v)->colno = (c))
#define P_GetVarDclColno(v) ((v)->colno)
#define P_SetVarDclAlign(v, a) ((v)->align = (a))
#define P_GetVarDclAlign(v) ((v)->align)
/*! Sets a bit in the VarDcl.qualifier field. */
#define P_SetVarDclQualifier(v, q) ((v)->qualifier |= (q))
#define P_GetVarDclQualifier(v) ((v)->qualifier)
/*! Tests a bit pattern in the VarDcl.qualifier field. */
#define P_TstVarDclQualifier(v, q) (((v)->qualifier & (q)) == (q))
/*! Clears a bit in the VarDcl.qualifier field. */
#define P_ClrVarDclQualifier(v, q) ((v)->qualifier &= ~(q))
#define P_SetVarDclFilename(v, f) ((v)->filename = (f))
#define P_GetVarDclFilename(v) ((v)->filename)
#define P_SetVarDclPragma(v, p) ((v)->pragma = (p))
#define P_GetVarDclPragma(v) ((v)->pragma)
/*! Sets the VarDcl.ext field for a module */
#define P_SetVarDclExtM(v, e) ((v)->ext[0] = (e))
/*! Gets the VarDcl.ext field for a module */
#define P_GetVarDclExtM(v) ((v)->ext[0])
/*! Sets the VarDcl.ext field for a library
 *
 * \param v
 *  the VarDcl.
 * \param i
 *  the library's index in the ext array.
 * \param e
 *  the new value of the ext field.
 *
 * \return
 *  The new value of the ext field.
 */
#define P_SetVarDclExtL(v, i, e) ((v)->ext[(i)] = (e))
/*! Gets the VarDcl.ext field for a library
 *
 * \param v
 *  the VarDcl.
 * \param i
 *  the library's index in the ext array.
 *
 * \return
 *  The value of the ext field.
 */
#define P_GetVarDclExtL(v, i) ((v)->ext[(i)])

extern Init P_SetVarDclInit (VarDcl v, Init i);
extern Position P_SetVarDclPosition (VarDcl v, Position p);
extern Position P_GetVarDclPosition (VarDcl v);
extern Identifier P_SetVarDclIdentifier (VarDcl v, Identifier i);
extern Identifier P_GetVarDclIdentifier (VarDcl v);
/* @} */

/*! \addtogroup InitAF Init access functions
 *
 * Access the fields of the Init struct.  The return value of any
 * P_SetInit* function is the new value of that field. */
/* @{ */
#define P_SetInitExpr(i, e) ((i)->expr = (e))
#define P_GetInitExpr(i) ((i)->expr)
#define P_SetInitSet(i, s) ((i)->set = (s))
#define P_GetInitSet(i) ((i)->set)
#define P_SetInitNext(i, n) ((i)->next = (n))
#define P_GetInitNext(i) ((i)->next)
#define P_SetInitPragma(i, p) ((i)->pragma = (p))
#define P_GetInitPragma(i) ((i)->pragma)
/*! Sets the Init.ext field for a module */
#define P_SetInitExtM(i, e) ((i)->ext[0] = (e))
/*! Gets the Init.ext field for a module */
#define P_GetInitExtM(i) ((i)->ext[0])
/*! Sets the Init.ext field for a library
 *
 * \param i
 *  the Init.
 * \param j
 *  the library's index in the ext array.
 * \param e
 *  the new value of the ext field.
 *
 * \return
 *  The new value of the ext field.
 */
#define P_SetInitExtL(i, j, e) ((i)->ext[(j)] = (e))
/*! Gets the Init.ext field for a library
 *
 * \param i
 *  the Init.
 * \param j
 *  the library's index in the ext array.
 *
 * \return
 *  The value of the ext field.
 */
#define P_GetInitExtL(i, j) ((i)->ext[(j)])

extern Init P_AppendInitNext (Init i, Init n);
extern VarDcl P_SetInitParentVar (Init i, VarDcl v);
/* @} */

/*! \addtogroup StructDclAF StructDcl access functions
 *
 * Access the fields of the StructDcl struct.  The return value of any
 * P_SetStructDcl* function is the new value of that field. */
/* @{ */
#define P_SetStructDclName(s, n) ((s)->name = (n))
#define P_GetStructDclName(s) ((s)->name)
#define P_SetStructDclKey(s, k) ((s)->key = (k))
#define P_GetStructDclKey(s) ((s)->key)
/*! Sets a bit in the StructDcl.qualifier field. */
#define P_SetStructDclQualifier(s, q) ((s)->qualifier |= (q))
#define P_GetStructDclQualifier(s) ((s)->qualifier)
/*! Tests a bit pattern in the StructDcl.qualifier field. */
#define P_TstStructDclQualifier(s, q) (((s)->qualifier & (q)) == (q))
/*! Clears a bit in the StructDcl.qualifier field. */
#define P_ClrStructDclQualifier(s, q) ((s)->qualifier &= ~(q))
#define P_GetStructDclFields(s) ((s)->fields)
#define P_SetStructDclLineno(s, l) ((s)->lineno = (l))
#define P_GetStructDclLineno(s) ((s)->lineno)
#define P_SetStructDclColno(s, c) ((s)->colno = (c))
#define P_GetStructDclColno(s) ((s)->colno)
#define P_SetStructDclFilename(s, f) ((s)->filename = (f))
#define P_GetStructDclFilename(s) ((s)->filename)
#define P_SetStructDclSize(s, t) ((s)->size = (t))
#define P_GetStructDclSize(s) ((s)->size)
#define P_SetStructDclAlign(s, a) ((s)->align = (a))
#define P_GetStructDclAlign(s) ((s)->align)
#define P_SetStructDclGroup(s, g) ((s)->group = (g))
#define P_GetStructDclGroup(s) ((s)->group)
#define P_SetStructDclPragma(s, p) ((s)->pragma = (p))
#define P_GetStructDclPragma(s) ((s)->pragma)
/*! Sets the StructDcl.ext field for a module */
#define P_SetStructDclExtM(s, e) ((s)->ext[0] = (e))
/*! Gets the StructDcl.ext field for a module */
#define P_GetStructDclExtM(s) ((s)->ext[0])
/*! Sets the StructDcl.ext field for a library
 *
 * \param s
 *  the StructDcl.
 * \param i
 *  the library's index in the ext array.
 * \param e
 *  the new value of the ext field.
 *
 * \return
 *  The new value of the ext field.
 */
#define P_SetStructDclExtL(s, i, e) ((s)->ext[(i)] = (e))
/*! Gets the StructDcl.ext field for a library
 *
 * \param s
 *  the StructDcl.
 * \param i
 *  the library's index in the ext array.
 *
 * \return
 *  The value of the ext field.
 */
#define P_GetStructDclExtL(s, i) ((s)->ext[(i)])

extern Field P_SetStructDclFields (StructDcl s, Field f);
extern Field P_AppendStructDclFields (StructDcl s, Field f);
extern Position P_SetStructDclPosition (StructDcl s, Position p);
extern Position P_GetStructDclPosition (StructDcl s);
extern Identifier P_SetStructDclIdentifier (StructDcl s, Identifier i);
extern Identifier P_GetStructDclIdentifier (StructDcl s);
/* @} */

/*! \addtogroup UnionDclAF UnionDcl access functions
 *
 * Access the fields of the UnionDcl struct.  The return value of any
 * P_SetUnionDcl* function is the new value of that field. */
/* @{ */
#define P_SetUnionDclName(u, n) ((u)->name = (n))
#define P_GetUnionDclName(u) ((u)->name)
#define P_SetUnionDclKey(u, k) ((u)->key = (k))
#define P_GetUnionDclKey(u) ((u)->key)
/*! Sets a bit in the UnionDcl.qualifier field. */
#define P_SetUnionDclQualifier(u, q) ((u)->qualifier |= (q))
#define P_GetUnionDclQualifier(u) ((u)->qualifier)
/*! Tests a bit pattern in the UnionDcl.qualifier field. */
#define P_TstUnionDclQualifier(u, q) (((u)->qualifier & (q)) == (q))
/*! Clears a bit in the UnionDcl.qualifier field. */
#define P_ClrUnionDclQualifier(u, q) ((u)->qualifier &= ~(q))
#define P_GetUnionDclFields(u) ((u)->fields)
#define P_SetUnionDclLineno(u, l) ((u)->lineno = (l))
#define P_GetUnionDclLineno(u) ((u)->lineno)
#define P_SetUnionDclColno(u, c) ((u)->colno = (c))
#define P_GetUnionDclColno(u) ((u)->colno)
#define P_SetUnionDclFilename(u, f) ((u)->filename = (f))
#define P_GetUnionDclFilename(u) ((u)->filename)
#define P_SetUnionDclSize(u, s) ((u)->size = (s))
#define P_GetUnionDclSize(u) ((u)->size)
#define P_SetUnionDclAlign(u, a) ((u)->align = (a))
#define P_GetUnionDclAlign(u) ((u)->align)
#define P_SetUnionDclGroup(u, g) ((u)->group = (g))
#define P_GetUnionDclGroup(u) ((u)->group)
#define P_SetUnionDclPragma(u, p) ((u)->pragma = (p))
#define P_GetUnionDclPragma(u) ((u)->pragma)
/*! Sets the UnionDcl.ext field for a module */
#define P_SetUnionDclExtM(u, e) ((u)->ext[0] = (e))
/*! Gets the UnionDcl.ext field for a module */
#define P_GetUnionDclExtM(u) ((u)->ext[0])
/*! Sets the UnionDcl.ext field for a library
 *
 * \param u
 *  the UnionDcl.
 * \param i
 *  the library's index in the ext array.
 * \param e
 *  the new value of the ext field.
 *
 * \return
 *  The new value of the ext field.
 */
#define P_SetUnionDclExtL(u, i, e) ((u)->ext[(i)] = (e))
/*! Gets the UnionDcl.ext field for a library
 *
 * \param u
 *  the UnionDcl.
 * \param i
 *  the library's index in the ext array.
 *
 * \return
 *  The value of the ext field.
 */
#define P_GetUnionDclExtL(u, i) ((u)->ext[(i)])

extern Field P_SetUnionDclFields (UnionDcl u, Field f);
extern Field P_AppendUnionDclFields (UnionDcl u, Field f);
extern Position P_SetUnionDclPosition (UnionDcl u, Position p);
extern Position P_GetUnionDclPosition (UnionDcl u);
extern Identifier P_SetUnionDclIdentifier (UnionDcl u, Identifier i);
extern Identifier P_GetUnionDclIdentifier (UnionDcl u);
/* @} */

/*! \addtogroup FieldAF Field access functions
 *
 * Access the fields of the Field struct.  The return value of any
 * P_SetField* function is the new value of that field. */
/* @{ */
#define P_SetFieldName(f, n) ((f)->name = (n))
#define P_GetFieldName(f) ((f)->name)
#define P_SetFieldKey(f, k) ((f)->key = (k))
#define P_GetFieldKey(f) ((f)->key)
#define P_SetFieldParentKey(f, k) ((f)->parent_key = (k))
#define P_GetFieldParentKey(f) ((f)->parent_key)
#define P_SetFieldType(f, t) ((f)->type = (t))
#define P_GetFieldType(f) ((f)->type)
#define P_SetFieldIsBitField(f, b) ((f)->is_bit_field = (b))
#define P_GetFieldIsBitField(f) ((f)->is_bit_field)
#define P_SetFieldBitSize(f, b) ((f)->bit_size = (b))
#define P_GetFieldBitSize(f) ((f)->bit_size)
#define P_SetFieldBitOffsetRemainder(f, b) ((f)->bit_offset_remainder = (b))
#define P_GetFieldBitOffsetRemainder(f) ((f)->bit_offset_remainder)
#define P_SetFieldNext(f, n) ((f)->next = (n))
#define P_GetFieldNext(f) ((f)->next)
#define P_SetFieldOffset(f, o) ((f)->offset = (o))
#define P_GetFieldOffset(f) ((f)->offset)
#define P_SetFieldPragma(f, p) ((f)->pragma = (p))
#define P_GetFieldPragma(f) ((f)->pragma)
/*! Sets the Field.ext field for a module */
#define P_SetFieldExtM(f, e) ((f)->ext[0] = (e))
/*! Gets the Field.ext field for a module */
#define P_GetFieldExtM(f) ((f)->ext[0])
/*! Sets the Field.ext field for a library
 *
 * \param f
 *  the Field.
 * \param i
 *  the library's index in the ext array.
 * \param e
 *  the new value of the ext field.
 *
 * \return
 *  The new value of the ext field.
 */
#define P_SetFieldExtL(f, i, e) ((f)->ext[(i)] = (e))
/*! Gets the Field.ext field for a library
 *
 * \param f
 *  the Field.
 * \param i
 *  the library's index in the ext array.
 *
 * \return
 *  The value of the ext field.
 */
#define P_GetFieldExtL(f, i) ((f)->ext[(i)])

extern Key P_SetFieldParentKeyAll (Field f, Key k);
extern Field P_AppendFieldNext (Field f, Field n);
extern Identifier P_SetFieldIdentifier (Field f, Identifier i);
extern Identifier P_GetFieldIdentifier (Field f);
/* @} */

/*! \addtogroup EnumDclAF EnumDcl access functions
 *
 * Access the fields of the EnumDcl struct.  The return value of any
 * P_SetEnumDcl* function is the new value of that field. */
/* @{ */
#define P_SetEnumDclName(e, n) ((e)->name = (n))
#define P_GetEnumDclName(e) ((e)->name)
#define P_SetEnumDclKey(e, k) ((e)->key = (k))
#define P_GetEnumDclKey(e) ((e)->key)
#define P_SetEnumDclFields(e, f) ((e)->fields = (f))
#define P_GetEnumDclFields(e) ((e)->fields)
#define P_SetEnumDclLineno(e, l) ((e)->lineno = (l))
#define P_GetEnumDclLineno(e) ((e)->lineno)
#define P_SetEnumDclColno(e, c) ((e)->colno = (c))
#define P_GetEnumDclColno(e) ((e)->colno)
#define P_SetEnumDclFilename(e, f) ((e)->filename = (f))
#define P_GetEnumDclFilename(e) ((e)->filename)
#define P_SetEnumDclPragma(e, p) ((e)->pragma = (p))
#define P_GetEnumDclPragma(e) ((e)->pragma)
/*! Sets the EnumDcl.ext field for a module */
#define P_SetEnumDclExtM(e, f) ((e)->ext[0] = (f))
/*! Gets the EnumDcl.ext field for a module */
#define P_GetEnumDclExtM(e) ((e)->ext[0])
/*! Sets the EnumDcl.ext field for a library
 *
 * \param e
 *  the EnumDcl.
 * \param i
 *  the library's index in the ext array.
 * \param f
 *  the new value of the ext field.
 *
 * \return
 *  The new value of the ext field.
 */
#define P_SetEnumDclExtL(e, i, f) ((e)->ext[(i)] = (f))
/*! Gets the EnumDcl.ext field for a library
 *
 * \param e
 *  the EnumDcl.
 * \param i
 *  the library's index in the ext array.
 *
 * \return
 *  The value of the ext field.
 */
#define P_GetEnumDclExtL(e, i) ((e)->ext[(i)])

extern Position P_SetEnumDclPosition (EnumDcl e, Position p);
extern Position P_GetEnumDclPosition (EnumDcl e);
extern Identifier P_SetEnumDclIdentifier (EnumDcl e, Identifier i);
extern Identifier P_GetEnumDclIdentifier (EnumDcl e);
/* @} */

/*! \addtogroup EnumFieldAF EnumField access functions
 *
 * Access the fields of the EnumField struct.  The return value of any
 * P_SetEnumField* function is the new value of that field. */
/* @{ */
#define P_SetEnumFieldName(e, n) ((e)->name = (n))
#define P_GetEnumFieldName(e) ((e)->name)
#define P_SetEnumFieldKey(e, k) ((e)->key = (k))
#define P_GetEnumFieldKey(e) ((e)->key)
#define P_SetEnumFieldValue(e, v) ((e)->value = (v))
#define P_GetEnumFieldValue(e) ((e)->value)
#define P_SetEnumFieldNext(e, n) ((e)->next = (n))
#define P_GetEnumFieldNext(e) ((e)->next)

extern EnumField P_AppendEnumFieldNext (EnumField e, EnumField n);
extern Identifier P_SetEnumFieldIdentifier (EnumField e, Identifier i);
extern Identifier P_GetEnumFieldIdentifier (EnumField e);
/* @} */

/*! \addtogroup StmtAF Stmt access functions
 *
 * Access the fields of the Stmt struct.  The return value of any
 * P_SetStmt* function is the new value of that field. */
/* @{ */
#define P_SetStmtType(s, t) ((s)->type = (t))
#define P_GetStmtType(s) ((s)->type)
#define P_SetStmtKey(s, k) ((s)->key = (k))
#define P_GetStmtKey(s) ((s)->key)
#define P_SetStmtStatus(s, t) ((s)->status = (t))
#define P_GetStmtStatus(s) ((s)->status)
#define P_SetStmtLineno(s, l) ((s)->lineno = (l))
#define P_GetStmtLineno(s) ((s)->lineno)
#define P_SetStmtColno(s, c) ((s)->colno = (c))
#define P_GetStmtColno(s) ((s)->colno)
#define P_SetStmtArtificial(s, a) ((s)->artificial = (a))
#define P_GetStmtArtificial(s) ((s)->artificial)
#define P_SetStmtForoverlap(s, f) ((s)->foroverlap = (f))
#define P_GetStmtForoverlap(s) ((s)->foroverlap)
#define P_SetStmtFilename(s, f) ((s)->filename = (f))
#define P_GetStmtFilename(s) ((s)->filename)
#define P_SetStmtProfile(s, p) ((s)->profile = (p))
#define P_GetStmtProfile(s) ((s)->profile)
#define P_SetStmtPragma(s, p) ((s)->pragma = (p))
#define P_GetStmtPragma(s) ((s)->pragma)
#define P_SetStmtShadow(s, t) ((s)->shadow = (t))
#define P_GetStmtShadow(s) ((s)->shadow)
#define P_GetStmtLexPrev(s) ((s)->lex_prev)
#define P_GetStmtLexNext(s) ((s)->lex_next)
#define P_GetStmtLabels(s) ((s)->labels)
#define P_GetStmtRet(s) ((s)->stmtstruct.ret)
#define P_SetStmtLabelVal(s, l) ((s)->stmtstruct.label.val = (l))
#define P_GetStmtLabelVal(s) ((s)->stmtstruct.label.val)
#define P_SetStmtLabelKey(s, k) ((s)->stmtstruct.label.key = (k))
#define P_GetStmtLabelKey(s) ((s)->stmtstruct.label.key)
#define P_GetStmtCompound(s) ((s)->stmtstruct.compound)
#define P_GetStmtIfStmt(s) ((s)->stmtstruct.ifstmt)
#define P_GetStmtSwitchStmt(s) ((s)->stmtstruct.switchstmt)
#define P_GetStmtPstmt(s) ((s)->stmtstruct.pstmt)
#define P_SetStmtAdvance(s, a) ((s)->stmtstruct.advance = (a))
#define P_GetStmtAdvance(s) ((s)->stmtstruct.advance)
#define P_SetStmtAwait(s, a) ((s)->stmtstruct.await = (a))
#define P_GetStmtAwait(s) ((s)->stmtstruct.await)
#define P_GetStmtMutex(s) ((s)->stmtstruct.mutex)
#define P_GetStmtCobegin(s) ((s)->stmtstruct.cobegin)
#define P_GetStmtParLoop(s) ((s)->stmtstruct.parloop)
#define P_GetStmtSerLoop(s) ((s)->stmtstruct.serloop)
#define P_GetStmtExpr(s) ((s)->stmtstruct.expr)
#define P_GetStmtBodyStmt(s) ((s)->stmtstruct.bodystmt)
#define P_GetStmtEpilogueStmt(s) ((s)->stmtstruct.epiloguestmt)
#define P_GetStmtAsmStmt(s) ((s)->stmtstruct.asmstmt)
#define P_SetStmtParentStmt(s, p) ((s)->parent = (p))
#define P_SetStmtParentStmtAll(s, p) \
          (P_SetStmtParentStmtDir ((s), (p), FORWARD))
#define P_GetStmtParentStmt(s) ((s)->parent)
#define P_SetStmtParentFunc(s, p) ((s)->parent_func = (p))
#define P_SetStmtParentFuncAll(s, p) \
          (P_SetStmtParentFuncDir ((s), (p), FORWARD))
#define P_GetStmtParentFunc(s) ((s)->parent_func)
#define P_SetStmtParentExpr(s, p) ((s)->parent_expr = (p))
#define P_SetStmtParentExprAll(s, p) \
          (P_SetStmtParentExprDir ((s), (p), FORWARD))
#define P_GetStmtParentExpr(s) ((s)->parent_expr)
/*! Sets the Stmt.ext field for a module */
#define P_SetStmtExtM(s, e) ((s)->ext[0] = (e))
/*! Gets the Stmt.ext field for a module */
#define P_GetStmtExtM(s) ((s)->ext[0])
/*! Sets the Stmt.ext field for a library
 *
 * \param s
 *  the Stmt.
 * \param i
 *  the library's index in the ext array.
 * \param e
 *  the new value of the ext field.
 *
 * \return
 *  The new value of the ext field.
 */
#define P_SetStmtExtL(s, i, e) ((s)->ext[(i)] = (e))
/*! Gets the Stmt.ext field for a library
 *
 * \param s
 *  the Stmt.
 * \param i
 *  the library's index in the ext array.
 *
 * \return
 *  The value of the ext field.
 */
#define P_GetStmtExtL(s, i) ((s)->ext[(i)])

extern Stmt P_SetStmtLexPrev (Stmt s, Stmt l);
extern Stmt P_SetStmtLexNext (Stmt s, Stmt l);
extern Stmt P_AppendStmtLexNext (Stmt s, Stmt n);
extern Label P_SetStmtLabels (Stmt s, Label l);
extern Label P_AppendStmtLabels (Stmt s, Label l);
extern Expr P_SetStmtRet (Stmt s, Expr r);
extern Compound P_SetStmtCompound (Stmt s, Compound c);
extern IfStmt P_SetStmtIfStmt (Stmt s, IfStmt i);
extern SwitchStmt P_SetStmtSwitchStmt (Stmt s, SwitchStmt t);
extern Pstmt P_SetStmtPstmt (Stmt s, Pstmt p);
extern Mutex P_SetStmtMutex (Stmt s, Mutex m);
extern Cobegin P_SetStmtCobegin (Stmt s, Cobegin c);
extern ParLoop P_SetStmtParLoop (Stmt s, ParLoop p);
extern SerLoop P_SetStmtSerLoop (Stmt s, SerLoop t);
extern Expr P_SetStmtExpr (Stmt s, Expr e);
extern BodyStmt P_SetStmtBodyStmt (Stmt s, BodyStmt b);
extern EpilogueStmt P_SetStmtEpilogueStmt (Stmt s, EpilogueStmt e);
extern AsmStmt P_SetStmtAsmStmt (Stmt s, AsmStmt a);
extern Stmt P_SetStmtParentStmtDir (Stmt s, Stmt p, int direction);
extern FuncDcl P_SetStmtParentFuncDir (Stmt s, FuncDcl p, int direction);
extern Expr P_SetStmtParentExprDir (Stmt s, Expr p, int direction);
extern Position P_SetStmtPosition (Stmt s, Position p);
extern Position P_GetStmtPosition (Stmt s);
extern Identifier P_SetStmtGotoIdentifier (Stmt s, Identifier i);
extern Identifier P_GetStmtGotoIdentifier (Stmt s);
extern void P_AppendStmtLocalVar (Stmt s, VarDcl v);
/* @} */

/* Functions to manage a Stmt list. */
extern void P_StmtUpdate (Stmt s, Stmt old, Stmt new);
extern void P_StmtUpdateParents (Stmt s, Stmt new);
extern void P_StmtInsertExprAfter (Stmt s, Expr e);
extern void P_StmtInsertExprBefore (Stmt s, Expr e);
extern void P_StmtInsertExprBeforeLabel (Stmt s, Expr e);
extern void P_StmtInsertStmtAfter (Stmt s, Stmt new);
extern void P_StmtInsertStmtBefore (Stmt s, Stmt new);
extern void P_StmtInsertStmtBeforeLabel (Stmt s, Stmt new);
extern void P_StmtAddLabel (Stmt s, Label l);
extern void P_StmtAddLabelAfter (Stmt s, Label l);
extern Label P_StmtExtractLabels (Stmt s);
extern Stmt P_StmtRemoveStmt (Stmt s);
extern HashTable P_StmtBuildExprMap (Stmt orig, Stmt copy);

/*! \addtogroup LabelAF Label access functions
 *
 * Access the fields of the Label struct.  The return value of any
 * P_SetLabel* function is the new value of that field. */
/* @{ */
#define P_SetLabelVal(l, v) ((l)->val = (v))
#define P_GetLabelVal(l) ((l)->val)
#define P_SetLabelKey(l, k) ((l)->key = (k))
#define P_GetLabelKey(l) ((l)->key)
#define P_SetLabelType(l, t) ((l)->type = (t))
#define P_GetLabelType(l) ((l)->type)
#define P_SetLabelExpression(l, e) ((l)->data.expression = (e))
#define P_GetLabelExpression(l) ((l)->data.expression)
#define P_SetLabelParentStmtAll(l, p) \
          (P_SetLabelParentStmtDir ((l), (p), FORWARD))
#define P_GetLabelNext(l) ((l)->next)
#define P_GetLabelPrev(l) ((l)->prev)

extern Stmt P_SetLabelParentStmt (Label l, Stmt p);
extern Stmt P_SetLabelParentStmtDir (Label l, Stmt p, int direction);
extern Stmt P_GetLabelParentStmt (Label l);
extern Label P_SetLabelNext (Label l, Label n);
extern Label P_AppendLabelNext (Label l, Label n);
extern Label P_SetLabelPrev (Label l, Label p);
extern Identifier P_SetLabelIdentifier (Label l, Identifier i);
extern Identifier P_GetLabelIdentifier (Label l);
/* @} */

/*! \addtogroup CompoundAF Compound access functions
 *
 * Access the fields of the Compound struct.  The return value of any
 * P_SetCompound* function is the new value of that field. */
/* @{ */
#define P_SetCompoundTypeList(c, t) ((c)->type_list = (t))
#define P_GetCompoundTypeList(c) ((c)->type_list)
#define P_SetCompoundVarList(c, v) ((c)->var_list = (v))
#define P_GetCompoundVarList(c) ((c)->var_list)
#define P_GetCompoundStmtList(c) ((c)->stmt_list)
#define P_SetCompoundUniqueVarID(c, u) ((c)->unique_var_id = (u))
#define P_GetCompoundUniqueVarID(c) ((c)->unique_var_id)
#define P_SetCompoundParentStmt(c, p) ((c)->parent = (p))
#define P_GetCompoundParentStmt(c) ((c)->parent)

extern Stmt P_SetCompoundStmtList (Compound c, Stmt s);
extern Stmt P_SetCompoundParentStmtAll (Compound c, Stmt p);
extern char *P_CompoundNewIdentifier (Stmt s, char *tag);
/* @} */

/*! \addtogroup IfStmtAF IfStmt access functions
 *
 * Access the fields of the IfStmt struct.  The return value of any
 * P_SetIfStmt* function is the new value of that field. */
/* @{ */
#define P_GetIfStmtCondExpr(i) ((i)->cond_expr)
#define P_GetIfStmtThenBlock(i) ((i)->then_block)
#define P_GetIfStmtElseBlock(i) ((i)->else_block)
#define P_SetIfStmtParentStmt(i, p) ((i)->parent = (p))
#define P_GetIfStmtParentStmt(i) ((i)->parent)

extern Expr P_SetIfStmtCondExpr (IfStmt i, Expr e);
extern Stmt P_SetIfStmtThenBlock (IfStmt i, Stmt t);
extern Stmt P_SetIfStmtElseBlock (IfStmt i, Stmt e);
extern Stmt P_SetIfStmtParentStmtAll (IfStmt i, Stmt p);
/* @} */

/*! \addtogroup SwitchStmtAF SwitchStmt access functions
 *
 * Access the fields of the SwitchStmt struct.  The return value of any
 * P_SetSwitchStmt* function is the new value of that field. */
/* @{ */
#define P_GetSwitchStmtExpression(s) ((s)->expression)
#define P_GetSwitchStmtSwitchBody(s) ((s)->switchbody)
#define P_SetSwitchStmtParentStmt(s, p) ((s)->parent = (p))
#define P_GetSwitchStmtParentStmt(s) ((s)->parent)

extern Expr P_SetSwitchStmtExpression (SwitchStmt s, Expr e);
extern Stmt P_SetSwitchStmtSwitchBody (SwitchStmt s, Stmt t);
extern Stmt P_SetSwitchStmtParentStmtAll (SwitchStmt s, Stmt p);
/* @} */

/*! \addtogroup PstmtAF Pstmt access functions
 *
 * Access the fields of the Pstmt struct.  The return value of any
 * P_SetPstmt* function is the new value of that field. */
/* @{ */
#define P_GetPstmtStmt(p) ((p)->stmt)
#define P_SetPstmtLineno(p, l) ((p)->lineno = (l))
#define P_GetPstmtLineno(p) ((p)->lineno)
#define P_SetPstmtColno(p, c) ((p)->colno = (c))
#define P_GetPstmtColno(p) ((p)->colno)
#define P_SetPstmtFilename(p, f) ((p)->filename = (f))
#define P_GetPstmtFilename(p) ((p)->filename)
#define P_SetPstmtPragma(p, q) ((p)->pragma = (q))
#define P_GetPstmtPragma(p) ((p)->pragma)
#define P_SetPstmtParentStmt(p, q) ((p)->parent = (q))
#define P_GetPstmtParentStmt(p) ((p)->parent)
/*! Sets the Pstmt.ext field for a module */
#define P_SetPstmtExtM(p, e) ((p)->ext[0] = (e))
/*! Gets the Pstmt.ext field for a module */
#define P_GetPstmtExtM(p) ((p)->ext[0])
/*! Sets the Pstmt.ext field for a library
 *
 * \param p
 *  the Pstmt.
 * \param i
 *  the library's index in the ext array.
 * \param e
 *  the new value of the ext field.
 *
 * \return
 *  The new value of the ext field.
 */
#define P_SetPstmtExtL(p, i, e) ((p)->ext[(i)] = (e))
/*! Gets the Pstmt.ext field for a library
 *
 * \param p
 *  the Pstmt.
 * \param i
 *  the library's index in the ext array.
 *
 * \return
 *  The value of the ext field.
 */
#define P_GetPstmtExtL(p, i) ((p)->ext[(i)])

extern Stmt P_SetPstmtStmt (Pstmt p, Stmt s);
extern Stmt P_SetPstmtParentStmtAll (Pstmt p, Stmt q);
extern Position P_SetPstmtPosition (Pstmt p, Position position);
extern Position P_GetPstmtPosition (Pstmt p);
/* @} */

/*! \addtogroup AdvanceAF Advance access functions
 *
 * Access the fields of the Advance struct.  The return value of any
 * P_SetAdvance* function is the new value of that field. */
/* @{ */
#define P_SetAdvanceMarker(a, m) ((a)->marker = (m))
#define P_GetAdvanceMarker(a) ((a)->marker)
/* @} */

/*! \addtogroup AwaitAF Await access functions
 *
 * Access the fields of the Await struct.  The return value of any
 * P_SetAwait* function is the new value of that field. */
/* @{ */
#define P_SetAwaitMarker(a, m) ((a)->marker = (m))
#define P_GetAwaitMarker(a) ((a)->marker)
#define P_SetAwaitDistance(a, d) ((a)->distance = (d))
#define P_GetAwaitDistance(a) ((a)->distance)
/* @} */

/*! \addtogroup MutexAF Mutex access functions
 *
 * Access the fields of the Mutex struct.  The return value of any
 * P_SetMutex* function is the new value of that field. */
/* @{ */
#define P_GetMutexExpression(m) ((m)->expression)
#define P_GetMutexStatement(m) ((m)->statement)
#define P_SetMutexParentStmt(m, p) ((m)->parent = (p))
#define P_GetMutexParentStmt(m) ((m)->parent)

extern Expr P_SetMutexExpression (Mutex m, Expr e);
extern Stmt P_SetMutexStatement (Mutex m, Stmt s);
extern Stmt P_SetMutexParentStmtAll (Mutex m, Stmt p);
/* @} */

/*! \addtogroup CobeginAF Cobegin access functions
 *
 * Access the fields of the Cobegin struct.  The return value of any
 * P_SetCobegin* function is the new value of that field. */
/* @{ */
#define P_GetCobeginStatements(c) ((c)->statements)
#define P_SetCobeginParentStmt(c, p) ((c)->parent = (p))
#define P_GetCobeginParentStmt(c) ((c)->parent)

extern Stmt P_SetCobeginStatements (Cobegin c, Stmt s);
extern Stmt P_SetCobeginParentStmtAll (Cobegin c, Stmt p);
/* @} */

/*! \addtogroup BodyStmtAF BodyStmt access functions
 *
 * Access the fields of the BodyStmt struct.  The return value of any
 * P_SetBodyStmt* function is the new value of that field. */
/* @{ */
#define P_GetBodyStmtStatement(b) ((b)->statement)
#define P_SetBodyStmtParentStmt(b, p) ((b)->parent = (p))
#define P_GetBodyStmtParentStmt(b) ((b)->parent)

extern Stmt P_SetBodyStmtStatement (BodyStmt b, Stmt s);
extern Stmt P_SetBodyStmtParentStmtAll (BodyStmt b, Stmt p);
/* @} */

/*! \addtogroup EpilogueStmtAF EpilogueStmt access functions
 *
 * Access the fields of the EpilogueStmt struct.  The return value of any
 * P_SetEpilogueStmt* function is the new value of that field. */
/* @{ */
#define P_GetEpilogueStmtStatement(e) ((e)->statement)
#define P_SetEpilogueStmtParentStmt(e, p) ((e)->parent = (p))
#define P_GetEpilogueStmtParentStmt(e) ((e)->parent)

extern Stmt P_SetEpilogueStmtStatement (EpilogueStmt e, Stmt s);
extern Stmt P_SetEpilogueStmtParentStmtAll (EpilogueStmt e, Stmt s);
/* @} */

/*! \addtogroup ParLoopAF ParLoop access functions
 *
 * Access the fields of the ParLoop struct.  The return value of any
 * P_SetParLoop* function is the new value of that field. */
/* @{ */
#define P_SetParLoopLoopType(p, l) ((p)->loop_type = (l))
#define P_GetParLoopLoopType(p) ((p)->loop_type)
#define P_GetParLoopPstmt(p) ((p)->pstmt)
#define P_GetParLoopIterationVar(p) ((p)->iteration_var)
#define P_SetParLoopSibling(p, s) ((p)->sibling = (s))
#define P_GetParLoopSibling(p) ((p)->sibling)
#define P_GetParLoopInitValue(p) ((p)->init_value)
#define P_GetParLoopFinalValue(p) ((p)->final_value)
#define P_GetParLoopIncrValue(p) ((p)->incr_value)
#define P_GetParLoopChild(p) ((p)->child)
#define P_SetParLoopParentStmt(p, q) ((p)->parent = (q))
#define P_GetParLoopParentStmt(p) ((p)->parent)
#define P_SetParLoopDepth(p, d) ((p)->depth = (d))
#define P_GetParLoopDepth(p) ((p)->depth)

extern Pstmt P_SetParLoopPstmt (ParLoop p, Pstmt q);
extern Expr P_SetParLoopIterationVar (ParLoop p, Expr i);
extern Expr P_SetParLoopInitValue (ParLoop p, Expr i);
extern Expr P_SetParLoopFinalValue (ParLoop p, Expr f);
extern Expr P_SetParLoopIncrValue (ParLoop p, Expr i);
extern Stmt P_SetParLoopChild (ParLoop p, Stmt c);
extern Stmt P_SetParLoopParentStmtAll (ParLoop p, Stmt parent);

/*! Returns the Stmt inside the ParLoop's Pstmt.
 *
 * \note Replaces the Parloop_Stmts_Prologue_Stmt() function from old Pcode. */
#define P_GetParLoopPrologueStmt(p) (P_GetPstmtStmt (P_GetParLoopPstmt ((p))))
extern Stmt P_GetParLoopBodyStmt (ParLoop p);
extern Stmt P_GetParLoopFirstEpilogueStmt (ParLoop p);
/* @} */

/*! \addtogroup SerLoopAF SerLoop access functions
 *
 * Access the fields of the SerLoop struct.  The return value of any
 * P_SetSerLoop* function is the new value of that field. */
/* @{ */
#define P_SetSerLoopLoopType(s, l) ((s)->loop_type = (l))
#define P_GetSerLoopLoopType(s) ((s)->loop_type)
#define P_GetSerLoopLoopBody(s) ((s)->loop_body)
#define P_GetSerLoopCondExpr(s) ((s)->cond_expr)
#define P_GetSerLoopInitExpr(s) ((s)->init_expr)
#define P_GetSerLoopIterExpr(s) ((s)->iter_expr)
#define P_SetSerLoopParentStmt(s, p) ((s)->parent = (p))
#define P_GetSerLoopParentStmt(s) ((s)->parent)

extern Stmt P_SetSerLoopLoopBody (SerLoop s, Stmt b);
extern Expr P_SetSerLoopCondExpr (SerLoop s, Expr c);
extern Expr P_SetSerLoopInitExpr (SerLoop s, Expr i);
extern Expr P_SetSerLoopIterExpr (SerLoop s, Expr i);
extern Stmt P_SetSerLoopParentStmtAll (SerLoop s, Stmt p);
/* @} */

/*! \addtogroup AsmStmtAF AsmStmt access functions
 *
 * Access the fields of the AsmStmt struct.  The return value of any
 * P_SetAsmStmt* function is the new value of that field. */
/* @{ */
#define P_SetAsmStmtIsVolatile(a, v) ((a)->is_volatile = (v))
#define P_GetAsmStmtIsVolatile(a) ((a)->is_volatile)
#define P_GetAsmStmtAsmClobbers(a) ((a)->asm_clobbers)
#define P_GetAsmStmtAsmString(a) ((a)->asm_string)
#define P_GetAsmStmtAsmOperands(a) ((a)->asm_operands)
#define P_SetAsmStmtParentStmt(a, p) ((a)->parent = (p))
#define P_GetAsmStmtParentStmt(a) ((a)->parent)

extern Expr P_SetAsmStmtAsmClobbers (AsmStmt a, Expr c);
extern Expr P_SetAsmStmtAsmString (AsmStmt a, Expr s);
extern Expr P_SetAsmStmtAsmOperands (AsmStmt a, Expr o);
extern Stmt P_SetAsmStmtParentStmtAll (AsmStmt a, Stmt p);
/* @} */

/*! \addtogroup AsmoprdAF Asmoprd access functions
 *
 * Access the fields of the Asmoprd struct.  The return value of any
 * P_SetAsmoprd* function is the new value of that field. */
/* @{ */
#define P_SetAsmoprdModifiers(a, m) ((a)->modifiers = (m))
#define P_GetAsmoprdModifiers(a) ((a)->modifiers)
#define P_SetAsmoprdConstraints(a, c) ((a)->constraints = (c))
#define P_GetAsmoprdConstraints(a) ((a)->constraints)
/* @} */

/*! \addtogroup ExprAF Expr access functions
 *
 * Access the fields of the Expr struct.  The return value of any P_SetExpr*
 * function is the new value of that field. */
/* @{ */
#define P_SetExprID(e, i) ((e)->id = (i))
#define P_GetExprID(e) ((e)->id)
#define P_SetExprStatus(e, s) ((e)->status = (s))
#define P_GetExprStatus(e) ((e)->status)
#define P_SetExprKey(e, k) ((e)->key = (k))
#define P_GetExprKey(e) ((e)->key)
#define P_SetExprOpcode(e, o) ((e)->opcode = (o))
#define P_GetExprOpcode(e) ((e)->opcode)
/*! Sets a bit in the Expr.flags field. */
#define P_SetExprFlags(e, f) ((e)->flags |= (f))
#define P_GetExprFlags(e) ((e)->flags)
/*! Tests a bit pattern in the Expr.flags field. */
#define P_TstExprFlags(e, f) (((e)->flags & (f)) == (f))
/*! Clears a bit in the Expr.flags field. */
#define P_ClrExprFlags(e, f) ((e)->flags &= ~(f))
#define P_SetExprType(e, t) ((e)->type = (t))
#define P_GetExprType(e) ((e)->type)
#define P_SetExprScalar(e, s) ((e)->value.scalar = (s))
#define P_GetExprScalar(e) ((e)->value.scalar)
#define P_SetExprUScalar(e, u) ((e)->value.uscalar = (u))
#define P_GetExprUScalar(e) ((e)->value.uscalar)
#define P_SetExprReal(e, r) ((e)->value.real = (r))
#define P_GetExprReal(e) ((e)->value.real)
#define P_SetExprString(e, s) ((e)->value.string = (s))
#define P_GetExprString(e) ((e)->value.string)
#define P_SetExprVarName(e, n) ((e)->value.var.name = (n))
#define P_GetExprVarName(e) ((e)->value.var.name)
#define P_SetExprVarKey(e, k) ((e)->value.var.key = (k))
#define P_GetExprVarKey(e) ((e)->value.var.key)
#define P_SetExprVType(e, t) ((e)->value.type = (t))
#define P_GetExprVType(e) ((e)->value.type)
#define P_GetExprStmt(e) ((e)->value.stmt)
#define P_SetExprAsmoprd(e, a) ((e)->value.asmoprd = (a))
#define P_GetExprAsmoprd(e) ((e)->value.asmoprd)
#define P_GetExprSibling(e) ((e)->sibling)
#define P_GetExprOperands(e) ((e)->operands)
#define P_SetExprParentExpr(e, p) ((e)->parentexpr = (p))
#define P_GetExprParentExpr(e) ((e)->parentexpr)
#define P_SetExprParentStmt(e, p) ((e)->parentstmt = (p))
#define P_GetExprParentStmt(e) ((e)->parentstmt)
#define P_SetExprParentVar(e, p) ((e)->parentvar = (p))
#define P_GetExprParentVar(e) ((e)->parentvar)
#define P_GetExprNext(e) ((e)->next)
#define P_GetExprPrevious(e) ((e)->previous)
#define P_SetExprBBNext(e, b) ((e)->bb_next = (b))
#define P_GetExprBBNext(e) ((e)->bb_next)
#define P_SetExprPragma(e, p) ((e)->pragma = (p))
#define P_GetExprPragma(e) ((e)->pragma)
#define P_SetExprProfile(e, p) ((e)->profile = (p))
#define P_GetExprProfile(e) ((e)->profile)
/*! Sets the Expr.ext field for a module */
#define P_SetExprExtM(e, f) ((e)->ext[0] = (f))
/*! Gets the Expr.ext field for a module */
#define P_GetExprExtM(e) ((e)->ext[0])
/*! Sets the Expr.ext field for a library
 *
 * \param e
 *  the Expr.
 * \param i
 *  the library's index in the ext array.
 * \param f
 *  the new value of the ext field.
 *
 * \return
 *  The new value of the ext field.
 */
#define P_SetExprExtL(e, i, f) ((e)->ext[(i)] = (f))
/*! Gets the Expr.ext field for a library
 *
 * \param e
 *  the Expr.
 * \param i
 *  the library's index in the ext array.
 *
 * \return
 *  The value of the ext field.
 */
#define P_GetExprExtL(e, i) ((e)->ext[(i)])

extern Stmt P_SetExprStmt (Expr e, Stmt s);
extern Expr P_SetExprSibling (Expr e, Expr s);
extern Expr P_SetExprOperands (Expr e, Expr o);
extern Expr P_AppendExprOperands (Expr e, Expr o);
extern Expr P_SetExprParentExprAll (Expr e, Expr p);
extern Stmt P_SetExprParentStmtAll (Expr e, Stmt p);
extern VarDcl P_SetExprParentVarAll (Expr e, VarDcl v);
extern Expr P_SetExprNext (Expr e, Expr n);
extern Expr P_AppendExprNext (Expr e, Expr n);
extern Expr P_SetExprPrevious (Expr e, Expr p);
extern Identifier P_SetExprVarIdentifier (Expr e, Identifier i);
extern Identifier P_GetExprVarIdentifier (Expr e);
/* @} */

/* Functions to manage an Expr. */
extern Stmt P_ExprParentStmt (Expr expr);
extern void P_ExprSwap (Expr *a, Expr *b);
extern HashTable P_ExprBuildExprMap (Expr orig, Expr copy);

/*! \addtogroup PragmaAF Pragma access functions
 *
 * Access the fields of the Pragma struct.  The return value of any
 * P_SetPragma* function is the new value of that field. */
/* @{ */
#define P_SetPragmaSpecifier(p, s) ((p)->specifier = (s))
#define P_GetPragmaSpecifier(p) ((p)->specifier)
#define P_SetPragmaExpr(p, e) ((p)->expr = (e))
#define P_GetPragmaExpr(p) ((p)->expr)
#define P_SetPragmaLineno(p, l) ((p)->lineno = (l))
#define P_GetPragmaLineno(p) ((p)->lineno)
#define P_SetPragmaColno(p, c) ((p)->colno = (c))
#define P_GetPragmaColno(p) ((p)->colno)
#define P_SetPragmaFilename(p, f) ((p)->filename = (f))
#define P_GetPragmaFilename(p) ((p)->filename)
#define P_SetPragmaNext(p, n) ((p)->next = (n))
#define P_GetPragmaNext(p) ((p)->next)

extern Pragma P_AppendPragmaNext (Pragma p, Pragma n);
extern Pragma P_DeletePragmaNext (Pragma p, Pragma d);
extern Position P_SetPragmaPosition (Pragma p, Position q);
extern Position P_GetPragmaPosition (Pragma p);
/* @} */

/*! \addtogroup PositionAF Position access functions
 *
 * Access the fields of the Position struct.  The return value of any
 * P_SetPosition* function is the new value of that field. */
/* @{ */
#define P_SetPositionLineno(p, l) ((p)->lineno = (l))
#define P_GetPositionLineno(p) ((p)->lineno)
#define P_SetPositionColno(p, c) ((p)->colno = (c))
#define P_GetPositionColno(p) ((p)->colno)
#define P_SetPositionFilename(p, f) ((p)->filename = (f))
#define P_GetPositionFilename(p) ((p)->filename)
/* @} */

/*! \addtogroup IdentifierAF Identifier access functions
 *
 * Access the fields of the Identifier struct.  The return value of any
 * P_SetIdentifier* function is the new value of that field. */
/* @{ */
#define P_SetIdentifierName(i, n) ((i)->name = (n))
#define P_GetIdentifierName(i) ((i)->name)
#define P_SetIdentifierKey(i, k) ((i)->key = (k))
#define P_GetIdentifierKey(i) ((i)->key)
/* @} */

/*! \addtogroup ProfFNAF ProfFN access functions
 *
 * Access the fields of the ProfFN struct.  The return value of any
 * P_SetProfFN* function is the new value of that field. */
/* @{ */
#define P_SetProfFNID(p, i) ((p)->fn_id = (i))
#define P_GetProfFNID(p) ((p)->fn_id)
#define P_SetProfFNCount(p, c) ((p)->count = (c))
#define P_GetProfFNCount(p) ((p)->count)
#define P_SetProfFNCalls(p, c) ((p)->calls = (c))
#define P_GetProfFNCalls(p) ((p)->calls)
/* @} */

/*! \addtogroup ProfCSAF ProfCS access functions
 *
 * Access the fields of the ProfCS struct.  The return value of any
 * P_SetProfCS* function is the new value of that field. */
/* @{ */
#define P_SetProfCSCallSiteID(p, i) ((p)->call_site_id = (i))
#define P_GetProfCSCallSiteID(p) ((p)->call_site_id)
#define P_SetProfCSCalleeID(p, i) ((p)->callee_id = (i))
#define P_GetProfCSCalleeID(p) ((p)->callee_id)
#define P_SetProfCSWeight(p, w) ((p)->weight = (w))
#define P_GetProfCSWeight(p) ((p)->weight)
#define P_SetProfCSNext(p, n) ((p)->next = (n))
#define P_GetProfCSNext(p) ((p)->next)

extern ProfCS P_AppendProfCSNext (ProfCS p, ProfCS n);
/* @} */

/*! \addtogroup ProfBBAF ProfBB access functions
 *
 * Access the fields of the ProfBB struct.  The return value of any
 * P_SetProfBB* function is the new value of that field. */
/* @{ */
#define P_SetProfBBWeight(p, w) ((p)->weight = (w))
#define P_GetProfBBWeight(p) ((p)->weight)
#define P_SetProfBBDestination(p, d) ((p)->destination = (d))
#define P_GetProfBBDestination(p) ((p)->destination)
/* @} */

/*! \addtogroup ProfArcAF ProfArc access functions
 *
 * Access the fields of the ProfArc struct.  The return value of any
 * P_SetProfArc* function is the new value of that field. */
/* @{ */
#define P_SetProfArcBBID(p, i) ((p)->bb_id = (i))
#define P_GetProfArcBBID(p) ((p)->bb_id)
#define P_SetProfArcCondition(p, c) ((p)->condition = (c))
#define P_GetProfArcCondition(p) ((p)->condition)
#define P_SetProfArcWeight(p, w) ((p)->weight = (w))
#define P_GetProfArcWeight(p) ((p)->weight)
#define P_SetProfArcNext(p, n) ((p)->next = (n))
#define P_GetProfArcNext(p) ((p)->next)

extern ProfArc P_AppendProfArcNext (ProfArc p, ProfArc n);
/* @} */

/*! \addtogroup ProfSTAF ProfST access functions
 *
 * Access the fields of the ProfST struct.  The return value of any
 * P_SetProfST* function is the new value of that field. */
/* @{ */
#define P_SetProfSTCount(p, c) ((p)->count = (c))
#define P_GetProfSTCount(p) ((p)->count)
#define P_SetProfSTNext(p, n) ((p)->next = (n))
#define P_GetProfSTNext(p) ((p)->next)

extern ProfST P_AppendProfSTNext (ProfST p, ProfST n);
/* @} */

/*! \addtogroup ProfEXPRAF ProfEXPR access functions
 *
 * Access the fields of the ProfEXPR struct.  The return value of any
 * P_SetProfEXPR* function is the new value of that field. */
/* @{ */
#define P_SetProfEXPRCount(p, c) ((p)->count = (c))
#define P_GetProfEXPRCount(p) ((p)->count)
#define P_SetProfEXPRNext(p, n) ((p)->next = (n))
#define P_GetProfEXPRNext(p) ((p)->next)

extern ProfEXPR P_AppendProfEXPRNext (ProfEXPR p, ProfEXPR n);
/* @} */

/*! \addtogroup ShadowAF Shadow access functions
 *
 * Access the fields of the Shadow struct.  The return value of any
 * P_SetShadow* function is the new value of that field. */
/* @{ */
#define P_SetShadowParamID(s, p) ((s)->param_id = (p))
#define P_GetShadowParamID(s) ((s)->param_id)
#define P_SetShadowExpr(s, e) ((s)->expr = (e))
#define P_GetShadowExpr(s) ((s)->expr)
#define P_SetShadowNext(s, n) ((s)->next = (n))
#define P_GetShadowNext(s) ((s)->next)

extern Shadow P_AppendShadowNext (Shadow s, Shadow n);
/* @} */

/*! \addtogroup AsmDclAF AsmDcl access functions
 *
 * Access the fields of the AsmDcl struct.  The return value of any
 * P_SetAsmDcl* function is the new value of that field. */
/* @{ */
#define P_SetAsmDclIsVolatile(a, v) ((a)->is_volatile = (v))
#define P_GetAsmDclIsVolatile(a) ((a)->is_volatile)
#define P_SetAsmDclAsmClobbers(a, c) ((a)->asm_clobbers = (c))
#define P_GetAsmDclAsmClobbers(a) ((a)->asm_clobbers)
#define P_SetAsmDclAsmString(a, s) ((a)->asm_string = (s))
#define P_GetAsmDclAsmString(a) ((a)->asm_string)
#define P_SetAsmDclAsmOperands(a, o) ((a)->asm_operands = (o))
#define P_GetAsmDclAsmOperands(a) ((a)->asm_operands)
#define P_SetAsmDclKey(a, k) ((a)->key = (k))
#define P_GetAsmDclKey(a) ((a)->key)
#define P_SetAsmDclLineno(a, l) ((a)->lineno = (l))
#define P_GetAsmDclLineno(a) ((a)->lineno)
#define P_SetAsmDclColno(a, c) ((a)->colno = (c))
#define P_GetAsmDclColno(a) ((a)->colno)
#define P_SetAsmDclFilename(a, f) ((a)->filename = (f))
#define P_GetAsmDclFilename(a) ((a)->filename)
#define P_SetAsmDclPragma(a, p) ((a)->pragma = (p))
#define P_GetAsmDclPragma(a) ((a)->pragma)
/*! Sets the AsmDcl.ext field for a module */
#define P_SetAsmDclExtM(a, e) ((a)->ext[0] = (e))
/*! Gets the AsmDcl.ext field for a module */
#define P_GetAsmDclExtM(a) ((a)->ext[0])
/*! Sets the AsmDcl.ext field for a library
 *
 * \param a
 *  the AsmDcl.
 * \param i
 *  the library's index in the ext array.
 * \param e
 *  the new value of the ext field.
 *
 * \return
 *  The new value of the ext field.
 */
#define P_SetAsmDclExtL(a, i, e) ((a)->ext[(i)] = (e))
/*! Gets the AsmDcl.ext field for a library
 *
 * \param a
 *  the AsmDcl.
 * \param i
 *  the library's index in the ext array.
 *
 * \return
 *  The value of the ext field.
 */
#define P_GetAsmDclExtL(a, i) ((a)->ext[(i)])

extern Position P_SetAsmDclPosition (AsmDcl a, Position p);
extern Position P_GetAsmDclPosition (AsmDcl a);
/* @} */

/*! \addtogroup ScopeAF Scope access functions
 *
 * Access the fields of the Scope struct.  The return value of any
 * P_SetScope* function is the new value of that field. */
/* @{ */
#define P_SetScopeKey(s, k) ((s)->key = (k))
#define P_GetScopeKey(s) ((s)->key)
#define P_SetScopeScopeEntry(s, t) ((s)->scope_entry = (t))
#define P_GetScopeScopeEntry(s) ((s)->scope_entry)

extern Key P_AppendScopeScopeEntry (Scope s, Key k);
/* @} */

/*! \addtogroup ScopeEntryAF ScopeEntry access functions
 *
 * Access the fields of the ScopeEntry struct.  The return value of any
 * P_SetScopeEntry* function is the new value of that field. */
/* @{ */
#define P_SetScopeEntryKey(s, k) (P_SetKeyListKey ((s), (k)))
#define P_GetScopeEntryKey(s) (P_GetKeyListKey ((s)))
#define P_SetScopeEntryNext(s, n) (P_SetKeyListNext ((s), (n)))
#define P_GetScopeEntryNext(s) (P_GetKeyListNext ((s)))
#define P_AppendScopeEntryNext(s, n) \
          ((ScopeEntry)P_AppendKeyListNext ((KeyList)(s), (KeyList)(n)))
#define P_RemoveScopeEntryNextByKey(s, k) \
          ((ScopeEntry)P_DeleteKeyListNext ((KeyList)(s), k))
/* @} */

/*! \addtogroup SymTabEntryAF SymTabEntry access functions
 *
 * Access the fields of the SymTabEntry struct.  The return value of any
 * P_SetSymTabEntry* function is the new value of that field. */
/* @{ */
#define P_SetSymTabEntryKey(s, k) ((s)->key = (k))
#define P_GetSymTabEntryKey(s) ((s)->key)
#define P_SetSymTabEntryName(s, n) ((s)->name = (n))
#define P_GetSymTabEntryName(s) ((s)->name)
#define P_SetSymTabEntryScopeKey(s, k) ((s)->scope_key = (k))
#define P_GetSymTabEntryScopeKey(s) ((s)->scope_key)
#define P_SetSymTabEntryType(s, t) ((s)->type = (t))
#define P_GetSymTabEntryType(s) ((s)->type)
#define P_SetSymTabEntryFuncDcl(s, f) ((s)->entry.func_dcl = (f))
#define P_GetSymTabEntryFuncDcl(s) ((s)->entry.func_dcl)
#define P_SetSymTabEntryTypeDcl(s, t) ((s)->entry.type_dcl = (t))
#define P_GetSymTabEntryTypeDcl(s) ((s)->entry.type_dcl)
#define P_SetSymTabEntryVarDcl(s, v) ((s)->entry.var_dcl = (v))
#define P_GetSymTabEntryVarDcl(s) ((s)->entry.var_dcl)
#define P_SetSymTabEntryStructDcl(s, t) ((s)->entry.struct_dcl = (t))
#define P_GetSymTabEntryStructDcl(s) ((s)->entry.struct_dcl)
#define P_SetSymTabEntryUnionDcl(s, u) ((s)->entry.union_dcl = (u))
#define P_GetSymTabEntryUnionDcl(s) ((s)->entry.union_dcl)
#define P_SetSymTabEntryEnumDcl(s, e) ((s)->entry.enum_dcl = (e))
#define P_GetSymTabEntryEnumDcl(s) ((s)->entry.enum_dcl)
#define P_SetSymTabEntryAsmDcl(s, a) ((s)->entry.asm_dcl = (a))
#define P_GetSymTabEntryAsmDcl(s) ((s)->entry.asm_dcl)
#define P_SetSymTabEntryStmt(s, t) ((s)->entry.stmt = (t))
#define P_GetSymTabEntryStmt(s) ((s)->entry.stmt)
#define P_SetSymTabEntryExpr(s, e) ((s)->entry.expr = (e))
#define P_GetSymTabEntryExpr(s) ((s)->entry.expr)
#define P_SetSymTabEntryField(s, f) ((s)->entry.field = (f))
#define P_GetSymTabEntryField(s) ((s)->entry.field)
#define P_SetSymTabEntryEnumField(s, e) ((s)->entry.enum_field = (e))
#define P_GetSymTabEntryEnumField(s) ((s)->entry.enum_field)
#define P_SetSymTabEntryLabel(s, l) ((s)->entry.label = (l))
#define P_GetSymTabEntryLabel(s) ((s)->entry.label)
#define P_SetSymTabEntryBlockStart(s, b) ((s)->entry.block.start = (b))
#define P_GetSymTabEntryBlockStart(s) ((s)->entry.block.start)
#define P_SetSymTabEntryBlockSize(s, b) ((s)->entry.block.size = (b))
#define P_GetSymTabEntryBlockSize(s) ((s)->entry.block.size)
#define P_SetSymTabEntryScope(s, t) ((s)->scope = (t))
#define P_GetSymTabEntryScope(s) ((s)->scope)
#define P_SetSymTabEntryOffset(s, o) ((s)->offset = (o))
#define P_GetSymTabEntryOffset(s) ((s)->offset)
#define P_SetSymTabEntryNext(s, n) ((s)->next = (n))
#define P_GetSymTabEntryNext(s) ((s)->next)
#define P_SetSymTabEntryPrev(s, p) ((s)->prev = (p))
#define P_GetSymTabEntryPrev(s) ((s)->prev)
/*! Sets a bit in the SymTabEntry.flags field. */
#define P_SetSymTabEntryFlags(s, f) ((s)->flags |= (f))
#define P_GetSymTabEntryFlags(s) ((s)->flags)
/*! Tests a bit pattern in the SymTabEntry.flags field. */
#define P_TstSymTabEntryFlags(s, f) (((s)->flags & (f)) == (f))
/*! Clears a bit in the SymTabEntry.flags field. */
#define P_ClrSymTabEntryFlags(s, f) ((s)->flags &= ~(f))
#define P_SetSymTabEntryPragma(s, p) ((s)->pragma = (p))
#define P_GetSymTabEntryPragma(s) ((s)->pragma)
/*! Sets the SymTabEntry.ext field for a module */
#define P_SetSymTabEntryExtM(s, e) ((s)->ext[0] = (e))
/*! Gets the SymTabEntry.ext field for a module */
#define P_GetSymTabEntryExtM(s) ((s)->ext[0])
/*! Sets the SymTabEntry.ext field for a library
 *
 * \param s
 *  the SymTabEntry.
 * \param i
 *  the library's index in the ext array.
 * \param e
 *  the new value of the ext field.
 *
 * \return
 *  The new value of the ext field.
 */
#define P_SetSymTabEntryExtL(s, i, e) ((s)->ext[(i)] = (e))
/*! Gets the SymTabEntry.ext field for a library
 *
 * \param s
 *  the SymTabEntry.
 * \param i
 *  the library's index in the ext array.
 *
 * \return
 *  The value of the ext field.
 */
#define P_GetSymTabEntryExtL(s, i) ((s)->ext[(i)])

extern Identifier P_SetSymTabEntryIdentifier (SymTabEntry s, Identifier i);
extern Identifier P_GetSymTabEntryIdentifier (SymTabEntry s);
extern Field P_SetSymTabEntryFields (SymTabEntry s, Field f);
extern Field P_GetSymTabEntryFields (SymTabEntry s);
/* @} */

/*! \addtogroup IPSymTabEntAF IPSymTabEnt access functions
 *
 * Access the fields of the IPSymTabEnt struct.  The return value of any
 * P_SetIPSymTabEnt* function is the new value of that field. */
/* @{ */
#define P_SetIPSymTabEntSourceName(i, n) ((i)->source_name = (n))
#define P_GetIPSymTabEntSourceName(i) ((i)->source_name)
#define P_SetIPSymTabEntInName(i, n) ((i)->in_name = (n))
#define P_GetIPSymTabEntInName(i) ((i)->in_name)
#define P_SetIPSymTabEntOutName(i, n) ((i)->out_name = (n))
#define P_GetIPSymTabEntOutName(i) ((i)->out_name)
#define P_SetIPSymTabEntFileType(i, f) ((i)->file_type = (f))
#define P_GetIPSymTabEntFileType(i) ((i)->file_type)
#define P_SetIPSymTabEntKey(i, k) ((i)->key = (k))
#define P_GetIPSymTabEntKey(i) ((i)->key)
#define P_SetIPSymTabEntNumEntries(i, n) ((i)->num_entries = (n))
#define P_GetIPSymTabEntNumEntries(i) ((i)->num_entries)
#define P_SetIPSymTabEntOffset(i, o) ((i)->offset = (o))
#define P_GetIPSymTabEntOffset(i) ((i)->offset)
#define P_SetIPSymTabEntTable(i, t) ((i)->table = (t))
#define P_GetIPSymTabEntTable(i) ((i)->table)
#define P_SetIPSymTabEntFile(i, f) ((i)->file = (f))
#define P_GetIPSymTabEntFile(i) ((i)->file)
#define P_SetIPSymTabEntInFileStatus(i, f) ((i)->in_file_status = (f))
#define P_GetIPSymTabEntInFileStatus(i) ((i)->in_file_status)
#define P_SetIPSymTabEntOutFileStatus(i, f) ((i)->out_file_status = (f))
#define P_GetIPSymTabEntOutFileStatus(i) ((i)->out_file_status)
/*! Sets a bit in the IPSymTabEnt.flags field. */
#define P_SetIPSymTabEntFlags(i, f) ((i)->flags |= (f))
#define P_GetIPSymTabEntFlags(i) ((i)->flags)
/*! Tests a bit pattern in the IPSymTabEnt.flags field. */
#define P_TstIPSymTabEntFlags(i, f) (((i)->flags & (f)) == (f))
/*! Clears a bit in the IPSymTabEnt.flags field. */
#define P_ClrIPSymTabEntFlags(i, f) ((i)->flags &= ~(f))
#define P_SetIPSymTabEntPragma(i, p) ((i)->pragma = (p))
#define P_GetIPSymTabEntPragma(i) ((i)->pragma)
/*! Sets the IPSymTabEnt.ext field for a module */
#define P_SetIPSymTabEntExtM(i, e) ((i)->ext[0] = (e))
/*! Gets the IPSymTabEnt.ext field for a module */
#define P_GetIPSymTabEntExtM(i) ((i)->ext[0])
/*! Sets the IPSymTabEnt.ext field for a library
 *
 * \param i
 *  the IPSymTabEnt.
 * \param j
 *  the library's index in the ext array.
 * \param e
 *  the new value of the ext field.
 *
 * \return
 *  The new value of the ext field.
 */
#define P_SetIPSymTabEntExtL(i, j ,e) ((i)->ext[(j)] = (e))

/*! Gets the IPSymTabEnt.ext field for a library
 *
 * \param i
 *  the IPSymTabEnt.
 * \param j
 *  the library's index in the ext array.
 *
 * \return
 *  The value of the ext field.
 */
#define P_GetIPSymTabEntExtL(i, j) ((i)->ext[(j)])
/* @} */

/*! \addtogroup SymbolTableAF SymbolTable access functions
 *
 * Access the fields of the SymbolTable struct.  The return value of any
 * P_SetSymbolTable* function is the new value of that field. */
/* @{ */
#define P_SetSymbolTableIPTableName(s, i) ((s)->ip_table_name = (i))
#define P_GetSymbolTableIPTableName(s) ((s)->ip_table_name)
#define P_SetSymbolTableOutName(s, o) ((s)->out_name = (o))
#define P_GetSymbolTableOutName(s) ((s)->out_name)
#define P_SetSymbolTableModifiableFile(s, m) ((s)->modifiable_file = (m))
#define P_GetSymbolTableModifiableFile(s) ((s)->modifiable_file)
#define P_SetSymbolTableNumFiles(s, n) ((s)->num_files = (n))
#define P_GetSymbolTableNumFiles(s) ((s)->num_files)
#define P_SetSymbolTableIPTable(s, i) ((s)->ip_table = (i))
#define P_GetSymbolTableIPTable(s) ((s)->ip_table)
/*! Gets the ith symbol source. */
#define P_GetSymbolTableSearchOrder(s, i) ((s)->search_order[(i)])
/*! Sets a bit in the SymbolTable.flags field. */
#define P_SetSymbolTableFlags(s, f) ((s)->flags |= (f))
#define P_GetSymbolTableFlags(s) ((s)->flags)
/*! Tests a bit pattern in the SymbolTable.flags field. */
#define P_TstSymbolTableFlags(s, f) (((s)->flags & (f)) == (f))
/*! Clears a bit in the SymbolTable.flags field. */
#define P_ClrSymbolTableFlags(s, f) ((s)->flags &= ~(f))
#define P_SetSymbolTableFile(s, f) ((s)->file = (f))
#define P_GetSymbolTableFile(s) ((s)->file)
#define P_SetSymbolTableInFileStatus(s, f) ((s)->in_file_status = (f))
#define P_GetSymbolTableInFileStatus(s) ((s)->in_file_status)
#define P_SetSymbolTableOutFileStatus(s, f) ((s)->out_file_status = (f))
#define P_GetSymbolTableOutFileStatus(s) ((s)->out_file_status)

extern char *P_SetSymbolTableOutExt (SymbolTable s, char *o);
extern void P_SetSymbolTableSearchOrder (SymbolTable s, _STSearchOrder first,
					 _STSearchOrder second,
					 _STSearchOrder third);
/* @} */

/*! \addtogroup KeyMapAF KeyMap access functions
 *
 * Access the fields of the KeyMap struct.  The return value of any
 * P_SetKeyMap* function is the new value of that field. */
/* @{ */
#define P_SetKeyMapNewKey(k, n) ((k)->new_key = (n))
#define P_GetKeyMapNewKey(k) ((k)->new_key)
#if 0
#define P_SetKeyMapUsers(k, u) ((k)->users = (u))
#define P_GetKeyMapUsers(k) ((k)->users)
#endif
/* @} */

/* Functions to copy data structures */

extern List P_CopyList (List l, void *(*copy_data)(void *));

/*! \brief Copies a Key Pointer.
 *
 * \param k
 *  the Key pointer to copy.
 *
 * \return
 *  A pointer to a copy of \a k.
 *
 * \note Keys are typically statically allocated, so this function is
 *       not usually needed.
 */
#define P_CopyKeyP(k) (P_NewKeyPWithKey (*(k)))

/*! \brief Copies a DclList.
 *
 * \param d
 *  the DclList to copy.
 *
 * \return A copy of the DclList.
 *
 * Copies a DclList.
 */
#define P_CopyDclList(d) \
          ((DclList)P_CopyList ((List)d, (void *(*)(void *))P_CopyDcl))

extern Dcl P_CopyDcl (Dcl d);
extern FuncDcl P_CopyFuncDcl (FuncDcl f);
extern TypeDcl P_CopyTypeDcl (TypeDcl t);
extern KeyList P_CopyKeyListNode (KeyList l);
extern KeyList P_CopyKeyList (KeyList l);

/*! \brief Copies a Param.
 *
 * \param p
 *  the Param to copy.
 *
 * \return A copy of the Param.
 *
 * Copies a Param.
 *
 * All Params in the list are copied.
 */
#define P_CopyParam(p) ((Param)P_CopyKeyList ((KeyList)(p)))

/*! \brief Copies a TypeList.
 *
 * \param t
 *  the TypeList to copy.
 *
 * \return A copy of the TypeList.
 *
 * Copies a TypeList.
 *
 * All TypeLists in the list are copied.
 */
#define P_CopyTypeList(t) \
          ((TypeList)P_CopyList ((List)t, (void *(*)(void *))P_CopyTypeDcl))

/*! \brief Copies a VarList.
 *
 * \param l
 *  the VarList to copy.
 *
 * \return A copy of the VarList.
 *
 * Copies a VarList.
 *
 * All VarLists in the list are copied.
 */
#define P_CopyVarList(l) \
          ((VarList)P_CopyList ((List)l, (void *(*)(void *))P_CopyVarDcl))

extern VarDcl P_CopyVarDcl (VarDcl v);
extern Init P_CopyInit (Init i);
extern StructDcl P_CopyStructDcl (StructDcl s);
extern UnionDcl P_CopyUnionDcl (UnionDcl u);
extern Field P_CopyField (Field f);
extern EnumDcl P_CopyEnumDcl (EnumDcl e);
extern EnumField P_CopyEnumField (EnumField f);
extern Stmt P_CopyStmtNode (Stmt s);
extern Stmt P_CopyStmt (Stmt s);
extern Label P_CopyLabel (Label l);
extern Compound P_CopyCompound (Compound c);
extern IfStmt P_CopyIfStmt (IfStmt i);
extern SwitchStmt P_CopySwitchStmt (SwitchStmt s);
extern Pstmt P_CopyPstmt (Pstmt p);
extern Advance P_CopyAdvance (Advance a);
extern Await P_CopyAwait (Await a);
extern Mutex P_CopyMutex (Mutex m);
extern Cobegin P_CopyCobegin (Cobegin c);
extern BodyStmt P_CopyBodyStmt (BodyStmt b);
extern EpilogueStmt P_CopyEpilogueStmt (EpilogueStmt e);
extern ParLoop P_CopyParLoop (ParLoop p);
extern SerLoop P_CopySerLoop (SerLoop s);
extern AsmStmt P_CopyAsmStmt (AsmStmt a);
extern Asmoprd P_CopyAsmoprd (Asmoprd a);
extern Expr P_CopyExprNode (Expr e);
extern Expr P_CopyExpr (Expr e);
extern Expr P_CopyExprList (Expr e);
extern Pragma P_CopyPragmaNode (Pragma p);
extern Pragma P_CopyPragma (Pragma p);
extern Position P_CopyPosition (Position p);
extern Identifier P_CopyIdentifier (Identifier i);
extern ProfFN P_CopyProfFN (ProfFN p);
extern ProfCS P_CopyProfCS (ProfCS p);
extern ProfBB P_CopyProfBB (ProfBB p);
extern ProfArc P_CopyProfArc (ProfArc p);
extern ProfST P_CopyProfST (ProfST p);
extern ProfEXPR P_CopyProfEXPR (ProfEXPR p);
extern Shadow P_CopyShadow (Shadow s);
extern AsmDcl P_CopyAsmDcl (AsmDcl a);
extern Scope P_CopyScope (Scope s);

/*! \brief Copies a ScopeEntry.
 *
 * \param s
 *  the ScopeEntry to copy.
 *
 * \return A copy of the ScopeEntry
 *
 * Copies a single ScopeEntry node.  Does not copy the next field.
 */
#define P_CopyScopeEntryNode(s) ((ScopeEntry)P_CopyKeyListNode ((KeyList)(s)))

/*! \brief Copies a ScopeEntry.
 *
 * \param s
 *  the ScopeEntry to copy.
 *
 * \return A copy of the ScopeEntry.
 *
 * Copies a ScopeEntry list.
 */
#define P_CopyScopeEntry(s) ((ScopeEntry)P_CopyKeyList ((KeyList)(s)))

extern SymTabEntry P_CopySymTabEntry (SymTabEntry s);
extern IPSymTabEnt P_ShCopyIPSymTabEnt (IPSymTabEnt i);
extern void P_CopySearchOrder (SearchOrder dest, SearchOrder src);
extern SymbolTable P_ShCopySymbolTable (SymbolTable s);

#define P_GetMemDep(e) (P_memdep_t)P_GetExprExtM(e)
extern P_memdep_core_t P_new_memdep_core();
extern void P_free_memdep_core(P_memdep_core_t dep);
extern Extension P_alloc_memdep(void);
extern Extension P_free_memdep(Extension e);
extern char *P_write_memdep(char *sig, Extension e);
extern void P_read_memdep(Extension e, char *sig, char *raw);
extern Extension P_copy_memdep (Extension orig_e);
extern int P_add_expr_memdep(Expr e, P_memdep_core_t dep);

/*! Define the order of processing for the P_*ApplyFunc() functions. */
#define P_PREORDER  0
#define P_POSTORDER 1

extern void P_StmtApplyFunc (Stmt s, void (*fs)(Stmt, void *),
			     void (*fe)(Expr, void *), int pre_or_post,
			     void *data);
extern void P_ExprApplyFunc (Expr e, void (*fs)(Stmt, void *),
			     void (*fe)(Expr, void *), int pre_or_post,
			     void *data);
extern void P_InitApplyFunc (Init i, void (*fs)(Stmt, void *),
			     void (*fe)(Expr, void *), int pre_or_post,
			     void *data);

/*! \brief Applies a function to a Stmt in preorder.
 *
 * \param s
 *  the Stmt.
 * \param f
 *  the function to apply to statements.
 * \param g
 *  the function to apply to expressions.
 * \param d
 *  user defined data passed as the second argument to \a f and \a g.
 *
 * \a f is a pointer to a function with the prototype
 * void f (Stmt s, void *data)
 * \a g is a pointer to a function with the prototype
 * void g (Expr e, void *data)
 *
 * \sa P_StmtApplyFunc().
 */
#define P_StmtApplyPre(s, f, g, d) \
          (P_StmtApplyFunc ((s), (f), (g), P_PREORDER, (d)))
#define P_StmtApply(s, f, g, d) (P_StmtApplyPre ((s), (f), (g), (d)))

/*! \brief Applies a function to an Expr in preorder.
 *
 * \param e
 *  the Expr.
 * \param f
 *  the function to apply to statements.
 * \param g
 *  the function to apply to expressions.
 * \param d
 *  user defined data passed as the second argument to \a f and \a g.
 *
 * \a f is a pointer to a function with the prototype
 * void f (Stmt s, void *data)
 * \a g is a pointer to a function with the prototype
 * void g (Expr e, void *data)
 *
 * \sa P_ExprApplyFunc().
 */
#define P_ExprApplyPre(e, f, g, d) \
          (P_ExprApplyFunc ((e), (f), (g), P_PREORDER, (d)))
#define P_ExprApply(e, f, g, d) (P_ExprApplyPre ((e), (f), (g), (d)))

/*! \brief Applies a function to an Init in preorder.
 *
 * \param i
 *  the Init.
 * \param f
 *  the function to apply to statements.
 * \param g
 *  the function to apply to exprssions.
 * \param d
 *  user defined data passed as the second argument to \a f and \a g.
 *
 * \a f is a pointer to a function with the prototype
 * void f (Stmt s, void *data)
 * \a g is a pointer to a function with the prototype
 * void g (Expr e, void *data)
 *
 * \sa P_InitApplyFunc().
 */
#define P_InitApplyPre(i, f, g, d) \
          (P_InitApplyFunc ((i), (f), (g), P_PREORDER, (d)))
#define P_InitApply(i, f, g, d) (P_InitApplyPre ((i), (f), (g), (d)))

/*! \brief Applies a function to a Stmt in postorder.
 *
 * \param s
 *  the Stmt.
 * \param f
 *  the function to apply to statements.
 * \param g
 *  the function to apply to expressions.
 * \param d
 *  user defined data passed as the second argument to \a f and \a g.
 *
 * \a f is a pointer to a function with the prototype
 * void f (Stmt s, void *data)
 * \a g is a pointer to a function with the prototype
 * void g (Expr e, void *data)
 *
 * \sa P_StmtApplyFunc().
 */
#define P_StmtApplyPost(s, f, g, d) \
          (P_StmtApplyFunc ((s), (f), (g), P_POSTORDER, (d)))

/*! \brief Applies a function to an Expr in postorder.
 *
 * \param e
 *  the Expr.
 * \param f
 *  the function to apply to statements.
 * \param g
 *  the function to apply to expressions.
 * \param d
 *  user defined data passed as the second argument to \a f and \a g.
 *
 * \a f is a pointer to a function with the prototype
 * void f (Stmt s, void *data)
 * \a g is a pointer to a function with the prototype
 * void g (Expr e, void *data)
 *
 * \sa P_ExprApplyFunc().
 */
#define P_ExprApplyPost(e, f, g, d) \
          (P_ExprApplyFunc ((e), (f), (g), P_POSTORDER, (d)))

/*! \brief Applies a function to an Init in postorder.
 *
 * \param i
 *  the Init.
 * \param f
 *  the function to apply to statements.
 * \param g
 *  the function to apply to exprssions.
 * \param d
 *  user defined data passed as the second argument to \a f and \a g.
 *
 * \a f is a pointer to a function with the prototype
 * void f (Stmt s, void *data)
 * \a g is a pointer to a function with the prototype
 * void g (Expr e, void *data)
 *
 * \sa P_InitApplyFunc().
 */
#define P_InitApplyPost(i, f, g, d) \
          (P_InitApplyFunc ((i), (f), (g), P_POSTORDER, (d)))

#endif
