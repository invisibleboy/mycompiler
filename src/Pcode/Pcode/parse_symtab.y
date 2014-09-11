%{

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
/*****************************************************************************
 *	File:	 parse_symtab.y
 *      Authors: Robert Kidd and Wen-mei Hwu
 * 	Copyright (c) 2003 Robert Kidd, Wen-mei Hwu
 *		 and The Board of Trustees of the University of Illinois.
 *		 All rights reserved.
 *      License Agreement specifies the terms and conditions for 
 *      redistribution.
 *****************************************************************************/

#include <config.h>
#include <stdio.h>
#include <library/i_list.h>
#include <library/block_sparse_array.h>
#include <Pcode/pcode.h>
#include <Pcode/struct.h>
#include <Pcode/symtab.h>
#include <Pcode/read_symtab.h>
#if 0
#include <Pcode/lex_symtab.h>
#endif
#include <Pcode/parse_prefix_symtab.h>

#define YYPARSE_VERBOSE 1
#define YYPARSE_PARAM ret

void yyerror (char *s);
extern int yylex (void *lvalp);
%}

%pure_parser

%union
{
  char *_includename;
  Dcl _dcl;
  List _list;
  SymTabEntry _symtabentry;
  IPSymTabEnt _ipsymtabent;
  SymbolTable _symboltable;
}

%token <_dcl> DCL
%token <_dcl> DCL_FUNC
%token <_includename> INCLUDE_NAME
%token <_ipsymtabent> IP_SYM_TAB_ENT_HEADER
%token <_ipsymtabent> IP_SYM_TAB_ENT_SOURCE
%token <_symboltable> SYMBOL_TABLE_BEGIN
%token <_symboltable> SYMBOL_TABLE_END
%token <_symtabentry> SYM_TAB_ENTRY_BLOCK
%token <_symtabentry> SYM_TAB_ENTRY_END
%token <_symtabentry> SYM_TAB_ENTRY_FUNC
%token <_symtabentry> SYM_TAB_ENTRY_OTHER
%token <_symtabentry> SYM_TAB_ENTRY_SCOPE

%type <_symboltable> dcl
%type <_symboltable> func
%type <_symboltable> pcode_file
%type <_list> ip_symbol_table_list
%type <_symboltable> ip_table
%type <_list> scope
%type <_ipsymtabent> source_body
%type <_ipsymtabent> source_file
%type <_ipsymtabent> header_file
%type <_list> sym_tab_entry_list

%start pcode_file /* Start symbol */

%%

dcl: DCL
		{
		  SymbolTable table;
		  SymTabEntry entry;
		  Key key;

		  /* We must have a table to read a DCL. */
		  if (!(table = ((SymTabParserArg *)ret)->table))
		    YYABORT;

		  switch (P_GetDclType ($1))
		    {
		    case TT_FUNC:
		      P_punt ("parse_symtab.y:dcl:%d TT_FUNC seen in DCL",
			      __LINE__ - 1);
		      break;
		    case TT_TYPE:
		      {
			TypeDcl t;

			t = P_GetDclTypeDcl ($1);
			P_SetDclTypeDcl ($1, NULL);

			key = P_GetTypeDclKey (t);
			    
			entry = PST_GetSymTabEntryFromMem (table, key);
			P_SetSymTabEntryTypeDcl (entry, t);
		      }
		      break;
		    case TT_VAR:
		      {
			VarDcl v;

			v = P_GetDclVarDcl ($1);
			P_SetDclVarDcl ($1, NULL);

			key = P_GetVarDclKey (v);
			    
			entry = PST_GetSymTabEntryFromMem (table, key);
			P_SetSymTabEntryVarDcl (entry, v);
		      }
		      break;
		    case TT_STRUCT:
		      {
			StructDcl s;

			s = P_GetDclStructDcl ($1);
			P_SetDclStructDcl ($1, NULL);

			key = P_GetStructDclKey (s);
			    
			entry = PST_GetSymTabEntryFromMem (table, key);
			P_SetSymTabEntryStructDcl (entry, s);

			/* Link fields inside the StructDcl to their
			 * SymTabEntries. */
			PST_LinkSymbolsStruct (table, s);
		      }
		      break;
		    case TT_UNION:
		      {
			UnionDcl u;

			u = P_GetDclUnionDcl ($1);
			P_SetDclUnionDcl ($1, NULL);

			key = P_GetUnionDclKey (u);

			entry = PST_GetSymTabEntryFromMem (table, key);
			P_SetSymTabEntryUnionDcl (entry, u);

			/* Link fields inside the UnionDcl to their
			 * SymTabEntries. */
			PST_LinkSymbolsUnion (table, u);
		      }
		      break;
		    case TT_ENUM:
		      {
			EnumDcl e;

			e = P_GetDclEnumDcl ($1);
			P_SetDclEnumDcl ($1, NULL);

			key = P_GetEnumDclKey (e);

			entry = PST_GetSymTabEntryFromMem (table, key);
			P_SetSymTabEntryEnumDcl (entry, e);

			/* Link fields inside the EnumDcl to their
			 * SymTabEntries. */
			PST_LinkSymbolsEnum (table, e);
		      }
		      break;
		    case TT_ASM:
		      {
			AsmDcl a;

			a = P_GetDclAsmDcl ($1);
			P_SetDclAsmDcl ($1, NULL);

			key = P_GetAsmDclKey (a);

			entry = PST_GetSymTabEntryFromMem (table, key);
			P_SetSymTabEntryAsmDcl (entry, a);
		      }
		      break;
		    default:
		      P_punt ("parse_symtab.y:header_body:%d Unhandled Dcl "
			      "type %d", __LINE__ - 1, P_GetDclType ($1));
		      break;
		    }

		  $1 = P_RemoveDcl ($1);

		  ((SymTabParserArg *)ret)->key = key;

		  $$ = table;
		}
;

func: SYM_TAB_ENTRY_FUNC sym_tab_entry_list DCL_FUNC
		{
		  SymTabEntry func_entry, entry;
		  SymbolTable table;
		  FuncDcl func_dcl;
		  Key key;

		  /* We must have a table supplied to read a FuncDcl. */
		  if (!(table = ((SymTabParserArg *)ret)->table))
		    YYABORT;

		  key = P_GetSymTabEntryKey ($1);

		  /* $1 (the SYM_TAB_ENTRY_FUNC) is a copy of the one
		   * that appears in the scope.  It can be removed now. */
		  $1 = P_RemoveSymTabEntry ($1);

		  func_entry = PST_GetSymTabEntryFromMem (table, key);

		  List_start ($2);
		  while ((entry = (SymTabEntry)List_next ($2)))
		    PST_AddSymTabEntry (table, entry);
		  $2 = P_RemoveList ($2, NULL);

		  func_dcl = P_GetDclFuncDcl ($3);
		  P_SetDclFuncDcl ($3, NULL);
		  $3 = P_RemoveDcl ($3);

		  P_SetSymTabEntryFuncDcl (func_entry, func_dcl);
		  PST_LinkSymbolsFunc (table, func_dcl);

		  ((SymTabParserArg *)ret)->key = key;

		  $$ = table;
		}
;

/* There is an unparsed dcl list after the header's sym_tab_entry_list.
 * This is not read with the file, but is read as symbols are requested. */
header_file: IP_SYM_TAB_ENT_HEADER sym_tab_entry_list SYM_TAB_ENTRY_END
             /* dcl/func list */
		{
		  IPSymTabEnt ipste;
		  SymTabEntry entry;

		  ipste = $1;
		  ((SymTabParserArg *)ret)->ipste = ipste;

		  List_start ($2);
		  while ((entry = (SymTabEntry)List_next ($2)))
		    PST_AddIPSymTabEntEntry (ipste, entry);
		  $2 = P_RemoveList ($2, NULL);

		  /* Header files are always stored in the IP symbol table's
		   * file. */
		  P_SetIPSymTabEntFlags (ipste, IPSTEF_EMBEDDED);
		  
		  $3 = P_RemoveSymTabEntry ($3);

		  $$ = ipste;
		}
;

ip_symbol_table_list: /* empty */
		{
		  $$ = NULL;
		}
	| ip_symbol_table_list IP_SYM_TAB_ENT_HEADER
		{
		  $$ = List_insert_last ($1, $2);
		}
	| ip_symbol_table_list IP_SYM_TAB_ENT_SOURCE
		{
		  $$ = List_insert_last ($1, $2);
		}
;

/* The interprocedural symbol table will be followed by the contents of any
 * header files.  Before linking, the body of a source file will follow.
 * This rule only parses the IP symbol table.  Header and source files are
 * read by PST_ReadFile. */
ip_table: SYMBOL_TABLE_BEGIN ip_symbol_table_list SYMBOL_TABLE_END
	  /* header_file list source_body */
		{
		  SymbolTable table;
		  IPSymTabEnt ipste;
		  int file_key;

		  table = $1;
		  ((SymTabParserArg *)ret)->table = table;

		  List_start ($2);
		  while ((ipste = (IPSymTabEnt)List_next ($2)))
		    {
		      file_key = P_GetIPSymTabEntKey (ipste);
		      PST_SetFile (table, file_key, ipste);
		    }
		  $2 = P_RemoveList ($2, NULL);

		  $$ = table;
		}
;

pcode_file: ip_table
		{
		  YYACCEPT;
		}
	| source_file
		{
		  YYACCEPT;
		}
	| source_body
		{
		  YYACCEPT;
		}
	| header_file
		{
		  YYACCEPT;
		}
	| func
		{
		  YYACCEPT;
		}
        | dcl
		{
		  YYACCEPT;
		}
;

/* There is an unparsed dcl list after the scope's sym_tab_entry_list.
 * this is not read with the file, but is read as symbols are requested. */
scope: SYM_TAB_ENTRY_SCOPE sym_tab_entry_list SYM_TAB_ENTRY_END /* dcl list */
		{
		  List scope = NULL;

		  scope = List_insert_last (scope, $1);
		  scope = List_append (scope, $2);

		  $3 = P_RemoveSymTabEntry ($3);

		  $$ = scope;
		}
;

source_body: IP_SYM_TAB_ENT_SOURCE scope
		{
		  IPSymTabEnt ipste;
		  SymTabEntry entry;

		  ipste = $1;
		  ((SymTabParserArg *)ret)->ipste = ipste;

		  List_start ($2);
		  while ((entry = (SymTabEntry)List_next ($2)))
		    PST_AddIPSymTabEntEntry (ipste, entry);
		  $2 = P_RemoveList ($2, NULL);

		  $$ = ipste;
		}
;

source_file: INCLUDE_NAME source_body
		{
		  ((SymTabParserArg *)ret)->include_name = $1;
		  
		  $$ = $2;
		}
;

sym_tab_entry_list: /* empty */
		{
		  $$ = NULL;
		}
	| sym_tab_entry_list SYM_TAB_ENTRY_BLOCK
		{
		  $$ = List_insert_last ($1, $2);
		}
	| sym_tab_entry_list SYM_TAB_ENTRY_FUNC
		{
		  $$ = List_insert_last ($1, $2);
		}
	| sym_tab_entry_list SYM_TAB_ENTRY_OTHER
		{
		  $$ = List_insert_last ($1, $2);
		}
	| sym_tab_entry_list SYM_TAB_ENTRY_SCOPE
		{
		  $$ = List_insert_last ($1, $2);
		}
;
%%

void
yyerror (char *s)
{
  P_warn ("parse_symtab.y: %s\n", s);
}




