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
 *	File:	 parse.y
 *      Authors: Robert Kidd and Wen-mei Hwu
 * 	Copyright (c) 2003 Robert Kidd, Wen-mei Hwu
 *		 and The Board of Trustees of the University of Illinois.
 *		 All rights reserved.
 *      License Agreement specifies the terms and conditions for 
 *      redistribution.
 *****************************************************************************/

#include <config.h>
#include <library/i_list.h>
#include <Pcode/pcode.h>
#include <Pcode/struct.h>
#include <Pcode/perror.h>
#include <Pcode/util.h>
#include <Pcode/symtab.h>
#include <Pcode/extension.h>
#include <Pcode/parse_prefix.h>
#include <Pcode/query.h>

#define YYPARSE_VERBOSE 1

int last_column;
int last_line;
char *yytext;

void yyerror (char *s);
extern int yylex (void);
%}

%union
{
  char c_val;
  char *st_val;
  long i_val;
  double f_val;

  _BasicType _basictype;
  _EntryType _entrytype;
  _FileType _filetype;
  _IPSTEFlags _ipsteflags;
  _TypeQual _typequal;
  _VarQual _varqual;
  _STFlags _stflags;
  _StructQual _structqual;

  AsmDcl _asmdcl;
  AsmStmt _asmstmt;
  Dcl _dcl;
  EnumField _enumfield;
  Expr _expr;
  Field _field;
  FuncDcl _funcdcl;
  Identifier _identifier;
  Init _init;
  IPSymTabEnt _ipsymtabent;
  Key _key;
  Label _label;
  Param _param;
  Position _position;
  Pragma _pragma;
  ProfST _profst;
  Pstmt _pstmt;
  Scope _scope;
  ScopeEntry _scopeentry;
  Stmt _stmt;
  SymbolTable _symboltable;
  SymTabEntry _symtabentry;
  TypeDcl _typedcl;
  TypeList _typelist;
  VarDcl _vardcl;
  VarList _varlist;
}

/* List of keywords in alphabetical order. */
/* ADD ADDR ADVANCE ALIGNMENT AND ARRAY ARROW ASM ASMOPRD ASSIGN AUTO
 * AWAIT A_ADD A_AND A_DIV A_LSHFT A_MOD A_MUL A_OR A_RSHFT A_SUB
 * A_XOR BITFIELD BLOCK BREAK CALL CASE CAST CDECL CHAR CLOBBERS
 * COBEGIN COMDAT COMMON COMPEXPR COMPSTMT CONJ CONST CONSTRUCTOR
 * CONTINUE DEF DEFAULT DEFINED DESTRUCTOR DIM DISJ DIV DO DOACROSS
 * DOALL DOSERIAL DOSUPER DOT DOUBLE ELLIPSIS ELSE EMPTY END ENUM
 * ENUMFIELD EQ ERROR EXPLICIT_ALIGNMENT EXPR EXPRSIZE EXTERN FASTCALL
 * FIELD FINAL FLOAT FOR FUNC FUNCTION GE GLOBAL GOTO GT GVAR HEADER
 * ID IF IMPLICIT IN INC INCLUDE INCOMPLETE INDEX INDR INIT INT INV
 * IP_TABLE KEY LABEL LE LINKED LINKMULTI LONG LONGDOUBLE LONGLONG LSHFT LT
 * LTYPE MOD MUL MUTEX NE NEG NOT NOT_AVAIL NULL NUM_ENTRIES OFFSET
 * OLD_PARAM OPERANDS OR OUT PARAM PARAMETER PARENT PHI POINTER POS
 * POSTDEC POSTINC PRAGMA PREDEC PREINC PROFILE PSTMT QUEST REFS
 * REGISTER RETURN RSHFT SCOPE SIGNED SIZE SHADOW SHORT SOURCE STATIC
 * STDCALL STMT STMTEXPR STRING STRUCT SUB SWITCH SYM SYMBOL_TABLE
 * SYNC THEN TYPE TYPEDEF TYPESIZE UNION UNNAMED UNSIGNED VAR VARARG
 * VOID VOLATILE WEAK WHILE XOR */

%token <i_val> CONST VOLATILE AUTO STATIC EXTERN REGISTER GLOBAL
%token <i_val> SIGNED UNSIGNED
%token <i_val> VOID CHAR SHORT INT LONG LONGLONG FLOAT STRING
%token <i_val> DOUBLE LONGDOUBLE
%token <i_val> CDECL STDCALL FASTCALL
%token <i_val> DOT ARROW CAST EXPRSIZE TYPESIZE QUEST DISJ CONJ COMPEXPR
%token <i_val> ASSIGN OR XOR AND EQ NE LT LE GE GT RSHFT LSHFT ADD SUB
%token <i_val> MUL DIV MOD NEG NOT INV PREINC PREDEC POSTINC POSTDEC A_ADD
%token <i_val> A_SUB A_MUL A_DIV A_MOD A_RSHFT A_LSHFT A_AND A_OR A_XOR
%token <i_val> INDR ADDR INDEX CALL ASMOPRD STMTEXPR
%token <i_val> DEF TYPEDEF STRUCT UNION ENUM ERROR
%token <i_val> SIZE ALIGNMENT POS PROFILE SHADOW PRAGMA
%token <i_val> BREAK CONTINUE KW_NULL COMPSTMT DO WHILE FOR IF THEN ELSE
%token <i_val> SWITCH RETURN GOTO PSTMT ADVANCE AWAIT
%token <i_val> DOSERIAL INIT FINAL INC DOALL DOACROSS DOSUPER
%token <i_val> MUTEX COBEGIN ASM OPERANDS CLOBBERS
%token <i_val> ID ARRAY FUNCTION POINTER SYNC PARAMETER PARAM LABEL
%token <i_val> CASE DEFAULT VARARG BITFIELD VAR REAL TYPE SYM SCOPE
%token <i_val> IP_TABLE SYMBOL_TABLE NUM_ENTRIES BLOCK EXPR STMT FUNC OFFSET
%token <i_val> EXPLICIT_ALIGNMENT ENUMFIELD FIELD KEY DIM HEADER SOURCE
%token <i_val> IN OUT INCLUDE PARENT GVAR LTYPE REFS EMPTY INCOMPLETE IMPLICIT
%token <i_val> COMMON DEFINED WEAK COMDAT CONSTRUCTOR DESTRUCTOR UNNAMED
%token <i_val> END LINKED NOT_AVAIL ELLIPSIS OLD_PARAM PHI LINKMULTI
%token <i_val> I_LIT
%token <f_val> F_LIT
%token <c_val> C_LIT
%token <st_val> ST_LIT

%type <_dcl> dcl
%type <i_val> alignment
%type <_expr> asm_clobbers
%type <_asmdcl> asm_dcl
%type <_expr> asm_operands
%type <_asmstmt> asm_stmt
%type <_basictype> basic_type
%type <_basictype> basic_type_list
%type <_stmt> comp_stmt
%type <_entrytype> entry_type
%type <_enumfield> enum_field
%type <_enumfield> enum_field_list
%type <_expr> expr
%type <_expr> expr_container
%type <_expr> expr_core
%type <_expr> expr_list
%type <_expr> expr_list_container
%type <_field> field
%type <_field> field_list
%type <_filetype> file_type
%type <_funcdcl> func_dcl
%type <_identifier> identifier
%type <st_val> in_name
%type <st_val> include
%type <_init> initializer
%type <_init> initializer_list
%type <_init> initializer_list_container
%type <_ipsteflags> ipste_flags
%type <_ipsteflags> ipste_flags_list
%type <_ipsymtabent> ip_sym_tab_ent
%type <_key> key
%type <_label> label
%type <_label> label_list
%type <_basictype> named_basic_type
%type <st_val> out_name
%type <_param> param
%type <_param> param_list
%type <_expr> parloop_index
%type <_position> position
%type <_pragma> pragma
%type <_pragma> pragma_list
%type <_profst> prof_st
%type <_profst> prof_st_list
%type <f_val> profile
%type <_pstmt> pstmt
%type <_scope> scope
%type <_scopeentry> scope_entry
%type <i_val> size
%type <i_val> shadow
%type <_stmt> stmt
%type <_stmt> stmt_core
%type <_stmt> stmt_list
%type <_stmt> stmt_list_container
%type <_structqual> struct_qual
%type <_structqual> struct_qual_list
%type <_symboltable> symbol_table
%type <_symtabentry> symbol_table_entry
%type <_stflags> symbol_table_flags
%type <_stflags> symbol_table_flags_list
%type <_dcl> type_dcl
%type <_dcl> type_definition
%type <_typelist> type_list
%type <_typelist> type_list_container
%type <_typequal> type_qual
%type <_typequal> type_qual_list
%type <_typedcl> type_spec
%type <_vardcl> var_dcl
%type <_varqual> var_qual
%type <_varqual> var_qual_list
%type <_varlist> var_dcl_list
%type <_varlist> var_dcl_list_container

%start dcl /* Start symbol */

%%
dcl: /* empty */
		{
			$$ = NULL;
			P_Input = $$;
			YYACCEPT;
		}
	| func_dcl
		{
			$$ = P_NewDcl ();
			P_SetDclType ($$, TT_FUNC);
			P_SetDclFuncDcl ($$, $1);
			P_Input = $$;
			YYACCEPT;
		}
	| var_dcl
		{
			$$ = P_NewDcl ();
			P_SetDclType ($$, TT_VAR);
			P_SetDclVarDcl ($$, $1);
			P_Input = $$;
			YYACCEPT;
		}
	| asm_dcl
		{
			$$ = P_NewDcl ();
			P_SetDclType ($$, TT_ASM);
			P_SetDclAsmDcl ($$, $1);
			P_Input = $$;
			YYACCEPT;
		}
	| include
		{
			$$ = P_NewDcl ();
			P_SetDclType ($$, TT_INCLUDE);
			P_SetDclInclude ($$, $1);
			P_Input = $$;
			YYACCEPT;
		}
	| type_dcl
		{
			$$ = $1;
			P_Input = $$;
			YYACCEPT;
		}
	| symbol_table
		{
			$$ = P_NewDcl ();
			P_SetDclType ($$, TT_SYMBOLTABLE);
			P_SetDclSymbolTable ($$, $1);
			P_Input = $$;
			YYACCEPT;
		}
	| ip_sym_tab_ent
		{
			$$ = P_NewDcl ();
			P_SetDclType ($$, TT_IPSYMTABENT);
			P_SetDclIPSymTabEnt ($$, $1);
			P_Input = $$;
			YYACCEPT;
		}
	| symbol_table_entry
		{
			$$ = P_NewDcl ();
			P_SetDclType ($$, TT_SYMTABENTRY);
			P_SetDclSymTabEntry ($$, $1);
			P_Input = $$;
			YYACCEPT;
		}
			
;

alignment: /* empty */
		{
			$$ = 0;
		}
	| ALIGNMENT I_LIT
		{
			$$ = $2;
		}
;

asm_clobbers: /* empty */
		{
			$$ = NULL;
		}
	| CLOBBERS expr_list_container
		{
			$$ = $2;
		}
;

asm_dcl: '(' asm_stmt key pragma_list position ')'
		{
			$$ = P_NewAsmDcl ();
			P_SetAsmDclIsVolatile ($$,
					       P_GetAsmStmtIsVolatile ($2));
			P_SetAsmDclAsmClobbers ($$,
						P_GetAsmStmtAsmClobbers ($2));
			P_SetAsmDclAsmString ($$, P_GetAsmStmtAsmString ($2));
			P_SetAsmDclAsmOperands ($$,
						P_GetAsmStmtAsmOperands ($2));

			$2 = P_RemoveAsmStmt ($2);

			P_SetAsmDclKey ($$, $3);
			P_SetAsmDclPragma ($$, $4);

			if ($5)
			{
				P_SetAsmDclPosition ($$, $5);
				$5 = P_RemovePosition ($5);
			}

			if (Handlers[ES_ASM])
				P_ExtRead (ES_ASM, (void *)$$);
		}
;

asm_operands: /* empty */
		{
			$$ = NULL;
		}
	| OPERANDS expr_list_container
		{
			$$ = $2;
		}
;

asm_stmt: ASM expr asm_operands asm_clobbers
		{
			$$ = P_NewAsmStmt ();
			P_SetAsmStmtAsmString ($$, $2);
			P_SetAsmStmtAsmOperands ($$, $3);
			P_SetAsmStmtAsmClobbers ($$, $4);
		}
	| ASM VOLATILE expr asm_operands asm_clobbers
		{
			$$ = P_NewAsmStmt ();
			P_SetAsmStmtIsVolatile ($$, 1);
			P_SetAsmStmtAsmString ($$, $3);
			P_SetAsmStmtAsmOperands ($$, $4);
			P_SetAsmStmtAsmClobbers ($$, $5);
		}
;

basic_type: VOID
		{
			$$ = BT_VOID;
		}
	| CHAR
		{
			$$ = BT_CHAR;
		}
	| SHORT
		{
			$$ = BT_SHORT;
		}
	| INT
		{
			$$ = BT_INT;
		}
	| LONG
		{
			$$ = BT_LONG;
		}
	| LONGLONG
		{
			$$ = BT_LONGLONG;
		}
	| FLOAT
		{
			$$ = BT_FLOAT;
		}
	| DOUBLE
		{
			$$ = BT_DOUBLE;
		}
	| LONGDOUBLE
		{
			$$ = BT_LONGDOUBLE;
		}
	| UNSIGNED
		{
			$$ = BT_UNSIGNED;
		}
	| VARARG
		{
			$$ = BT_VARARG;
		}
	| BITFIELD
		{
			$$ = BT_BIT_FIELD;
		}
;

basic_type_list: /* empty */
		{
			$$ = 0;
		}
	| basic_type_list basic_type
		{
			$$ = $1 | $2;
		}
;

comp_stmt: COMPSTMT I_LIT type_list_container var_dcl_list_container
           stmt_list_container
		{
			$$ = P_NewStmt ();
			P_SetStmtType ($$, ST_COMPOUND);
			P_SetStmtCompound ($$, P_NewCompound ());
			P_SetCompoundUniqueVarID (P_GetStmtCompound ($$), $2);
			P_SetCompoundTypeList (P_GetStmtCompound ($$), $3);
			P_SetCompoundVarList (P_GetStmtCompound ($$), $4);
			P_SetCompoundStmtList (P_GetStmtCompound ($$), $5);
		}
;

entry_type: FUNC
		{
			$$ = ET_FUNC;
		}
	| LTYPE
		{
			$$ = ET_TYPE_LOCAL;
		}
	| TYPE
		{
			$$ = ET_TYPE_GLOBAL;
		}
	| VAR
		{
			$$ = ET_VAR_LOCAL;
		}
	| GVAR
		{
			$$ = ET_VAR_GLOBAL;
		}
	| STRUCT
		{
			$$ = ET_STRUCT;
		}
	| UNION
		{
			$$ = ET_UNION;
		}
	| ENUM
		{
			$$ = ET_ENUM;
		}
	| ASM
		{
			$$ = ET_ASM;
		}
	| STMT
		{
			$$ = ET_STMT;
		}
	| EXPR
		{	
			$$ = ET_EXPR;
		}
	| FIELD
		{
			$$ = ET_FIELD;
		}
	| ENUMFIELD
		{
			$$ = ET_ENUMFIELD;
		}
	| LABEL
		{
			$$ = ET_LABEL;
		}
	| SCOPE
		{
			$$ = ET_SCOPE;
		}
;

enum_field: '(' identifier expr ')'
		{
			$$ = P_NewEnumField ();
			P_SetEnumFieldIdentifier ($$, $2);
			$2 = P_RemoveIdentifier ($2);
			P_SetEnumFieldValue ($$, $3);
		}
	| '(' identifier ')'
		{
			$$ = P_NewEnumField ();
			P_SetEnumFieldIdentifier ($$, $2);
			$2 = P_RemoveIdentifier ($2);
		}
;

enum_field_list: /* empty */
		{
			$$ = NULL;
		}
	| enum_field_list enum_field
		{
			$$ = P_AppendEnumFieldNext ($1, $2);
		}
;

expr: '(' I_LIT expr_core pragma_list profile ')'
		{
			P_SetExprID ($3, $2);
			P_SetExprPragma ($3, $4);
			if ($5 > 0)
			{
				P_SetExprProfile ($3, P_NewProfEXPR ());
				P_SetProfEXPRCount (P_GetExprProfile ($3), $5);
			}
			$$ = $3;

			if (Handlers[ES_EXPR])
				P_ExtRead (ES_EXPR, (void *)$$);
		}
;

expr_container: '(' ')'
		{
			$$ = NULL;
		}	
	| '(' expr ')'
		{
			$$ = $2;
		}
;

expr_core: VAR identifier
		{
			$$ = P_NewExpr ();
			P_SetExprOpcode ($$, OP_var);
			P_SetExprVarIdentifier ($$, $2);
			$2 = P_RemoveIdentifier ($2);
		}
	| INT I_LIT key
		{
			$$ = P_NewExpr ();
			P_SetExprOpcode ($$, OP_int);
			P_SetExprScalar ($$, $2);
			P_SetExprType ($$, $3);
		}
	| SIGNED INT I_LIT key
		{
			$$ = P_NewExpr ();
			P_SetExprOpcode ($$, OP_int);
			P_SetExprScalar ($$, $3);
			P_SetExprType ($$, $4);
		}
	| UNSIGNED INT I_LIT key
		{
			$$ = P_NewExpr ();
			P_SetExprOpcode ($$, OP_int);
			P_SetExprFlags ($$, EF_UNSIGNED);
			P_SetExprUScalar ($$, $3);
			P_SetExprType ($$, $4);
		}
	| REAL F_LIT key
		{
			$$ = P_NewExpr ();
			P_SetExprOpcode ($$, OP_real);
			P_SetExprReal ($$, $2);
			P_SetExprType ($$, $3);
		}
	| FLOAT F_LIT key
		{
			$$ = P_NewExpr ();
			P_SetExprOpcode ($$, OP_float);
			P_SetExprReal ($$, $2);
			P_SetExprType ($$, $3);
		}
	| DOUBLE F_LIT key
		{
			$$ = P_NewExpr ();
			P_SetExprOpcode ($$, OP_double);
			P_SetExprReal ($$, $2);
			P_SetExprType ($$, $3);
		}
	| CHAR C_LIT key
		{
			$$ = P_NewExpr ();
			P_SetExprOpcode ($$, OP_char);
			P_SetExprString ($$, strdup (&$2));
			P_SetExprType ($$, $3);
		}
	| STRING ST_LIT key
		{
			$$ = P_NewExpr ();
			P_SetExprOpcode ($$, OP_string);
			P_SetExprString ($$, P_DQString2String ($2));
			free ($2);
			P_SetExprType ($$, $3);
		}
	| DOT expr identifier
		{
			$$ = P_NewExpr ();
			P_SetExprOpcode ($$, OP_dot);
			P_AppendExprOperands ($$, $2);
			P_SetExprVarIdentifier ($$, $3);
			$3 = P_RemoveIdentifier ($3);
		}
	| ARROW expr identifier
		{
			$$ = P_NewExpr ();
			P_SetExprOpcode ($$, OP_arrow);
			P_AppendExprOperands ($$, $2);
			P_SetExprVarIdentifier ($$, $3);
			$3 = P_RemoveIdentifier ($3);
		}
	| CAST key expr
		{
			$$ = P_NewExpr ();
			P_SetExprOpcode ($$, OP_cast);
			P_SetExprType ($$, $2);
			P_AppendExprOperands ($$, $3);
		}
	| EXPRSIZE expr
		{
			$$ = P_NewExpr ();
			P_SetExprOpcode ($$, OP_expr_size);
#if 0
			P_AppendExprOperands ($$, $2);
#endif
		}
	| TYPESIZE key
		{
			$$ = P_NewExpr ();
			P_SetExprOpcode ($$, OP_type_size);
#if 0
			P_SetExprVType ($$, $2);
#endif
		}
	| QUEST expr expr expr
		{
			$$ = P_NewExpr ();
			P_SetExprOpcode ($$, OP_quest);
			P_AppendExprOperands ($$, $2);
			P_AppendExprOperands ($$, $3);
			P_AppendExprOperands ($$, $4);
		}
	| DISJ expr expr
		{
			$$ = P_NewExpr ();
			P_SetExprOpcode ($$, OP_disj);
			P_AppendExprOperands ($$, $2);
			P_AppendExprOperands ($$, $3);
		}
	| CONJ expr expr
		{
			$$ = P_NewExpr ();
			P_SetExprOpcode ($$, OP_conj);
			P_AppendExprOperands ($$, $2);
			P_AppendExprOperands ($$, $3);
		}
	| COMPEXPR expr_list_container
		{
			$$ = P_NewExpr ();
			P_SetExprOpcode ($$, OP_compexpr);
			P_SetExprOperands ($$, $2);
		}
	| ASSIGN expr expr
		{
			$$ = P_NewExpr ();
			P_SetExprOpcode ($$, OP_assign);
			P_AppendExprOperands ($$, $2);
			P_AppendExprOperands ($$, $3);
		}
	| OR expr expr
		{
			$$ = P_NewExpr ();
			P_SetExprOpcode ($$, OP_or);
			P_AppendExprOperands ($$, $2);
			P_AppendExprOperands ($$, $3);
		}
	| XOR expr expr
		{
			$$ = P_NewExpr ();	
			P_SetExprOpcode ($$, OP_xor);
			P_AppendExprOperands ($$, $2);
			P_AppendExprOperands ($$, $3);
		}
	| AND expr expr
		{
			$$ = P_NewExpr ();	
			P_SetExprOpcode ($$, OP_and);
			P_AppendExprOperands ($$, $2);
			P_AppendExprOperands ($$, $3);
		}
	| EQ expr expr
		{
			$$ = P_NewExpr ();	
			P_SetExprOpcode ($$, OP_eq);
			P_AppendExprOperands ($$, $2);
			P_AppendExprOperands ($$, $3);
		}
	| NE expr expr
		{
			$$ = P_NewExpr ();	
			P_SetExprOpcode ($$, OP_ne);
			P_AppendExprOperands ($$, $2);
			P_AppendExprOperands ($$, $3);
		}
	| LT expr expr
		{
			$$ = P_NewExpr ();	
			P_SetExprOpcode ($$, OP_lt);
			P_AppendExprOperands ($$, $2);
			P_AppendExprOperands ($$, $3);
		}
	| LE expr expr
		{
			$$ = P_NewExpr ();	
			P_SetExprOpcode ($$, OP_le);
			P_AppendExprOperands ($$, $2);
			P_AppendExprOperands ($$, $3);
		}
	| GE expr expr
		{
			$$ = P_NewExpr ();	
			P_SetExprOpcode ($$, OP_ge);
			P_AppendExprOperands ($$, $2);
			P_AppendExprOperands ($$, $3);
		}
	| GT expr expr
		{
			$$ = P_NewExpr ();	
			P_SetExprOpcode ($$, OP_gt);
			P_AppendExprOperands ($$, $2);
			P_AppendExprOperands ($$, $3);
		}
	| RSHFT expr expr
		{
			$$ = P_NewExpr ();	
			P_SetExprOpcode ($$, OP_rshft);
			P_AppendExprOperands ($$, $2);
			P_AppendExprOperands ($$, $3);
		}
	| LSHFT expr expr
		{
			$$ = P_NewExpr ();	
			P_SetExprOpcode ($$, OP_lshft);
			P_AppendExprOperands ($$, $2);
			P_AppendExprOperands ($$, $3);
		}
	| ADD expr expr
		{
			$$ = P_NewExpr ();	
			P_SetExprOpcode ($$, OP_add);
			P_AppendExprOperands ($$, $2);
			P_AppendExprOperands ($$, $3);
		}
	| SUB expr expr
		{
			$$ = P_NewExpr ();	
			P_SetExprOpcode ($$, OP_sub);
			P_AppendExprOperands ($$, $2);
			P_AppendExprOperands ($$, $3);
		}
	| MUL expr expr
		{
			$$ = P_NewExpr ();	
			P_SetExprOpcode ($$, OP_mul);
			P_AppendExprOperands ($$, $2);
			P_AppendExprOperands ($$, $3);
		}
	| DIV expr expr
		{
			$$ = P_NewExpr ();	
			P_SetExprOpcode ($$, OP_div);
			P_AppendExprOperands ($$, $2);
			P_AppendExprOperands ($$, $3);
		}
	| MOD expr expr
		{
			$$ = P_NewExpr ();	
			P_SetExprOpcode ($$, OP_mod);
			P_AppendExprOperands ($$, $2);
			P_AppendExprOperands ($$, $3);
		}
	| NEG expr
		{
			$$ = P_NewExpr ();	
			P_SetExprOpcode ($$, OP_neg);
			P_AppendExprOperands ($$, $2);
		}
	| NOT expr
		{
			$$ = P_NewExpr ();	
			P_SetExprOpcode ($$, OP_not);
			P_AppendExprOperands ($$, $2);
		}
	| INV expr
		{
			$$ = P_NewExpr ();	
			P_SetExprOpcode ($$, OP_inv);
			P_AppendExprOperands ($$, $2);
		}
	| PREINC expr
		{
			$$ = P_NewExpr ();	
			P_SetExprOpcode ($$, OP_preinc);
			P_AppendExprOperands ($$, $2);
		}
	| PREDEC expr
		{
			$$ = P_NewExpr ();	
			P_SetExprOpcode ($$, OP_predec);
			P_AppendExprOperands ($$, $2);
		}
	| POSTINC expr
		{
			$$ = P_NewExpr ();	
			P_SetExprOpcode ($$, OP_postinc);
			P_AppendExprOperands ($$, $2);
		}
	| POSTDEC expr
		{
			$$ = P_NewExpr ();	
			P_SetExprOpcode ($$, OP_postdec);
			P_AppendExprOperands ($$, $2);
		}
	| A_ADD expr expr
		{
			$$ = P_NewExpr ();	
			P_SetExprOpcode ($$, OP_Aadd);
			P_AppendExprOperands ($$, $2);
			P_AppendExprOperands ($$, $3);
		}
	| A_SUB expr expr
		{
			$$ = P_NewExpr ();	
			P_SetExprOpcode ($$, OP_Asub);
			P_AppendExprOperands ($$, $2);
			P_AppendExprOperands ($$, $3);
		}
	| A_MUL expr expr
		{
			$$ = P_NewExpr ();	
			P_SetExprOpcode ($$, OP_Amul);
			P_AppendExprOperands ($$, $2);
			P_AppendExprOperands ($$, $3);
		}
	| A_DIV expr expr
		{
			$$ = P_NewExpr ();	
			P_SetExprOpcode ($$, OP_Adiv);
			P_AppendExprOperands ($$, $2);
			P_AppendExprOperands ($$, $3);
		}
	| A_MOD expr expr
		{
			$$ = P_NewExpr ();	
			P_SetExprOpcode ($$, OP_Amod);
			P_AppendExprOperands ($$, $2);
			P_AppendExprOperands ($$, $3);
		}
	| A_LSHFT expr expr
		{
			$$ = P_NewExpr ();	
			P_SetExprOpcode ($$, OP_Alshft);
			P_AppendExprOperands ($$, $2);
			P_AppendExprOperands ($$, $3);
		}
	| A_RSHFT expr expr
		{
			$$ = P_NewExpr ();	
			P_SetExprOpcode ($$, OP_Arshft);
			P_AppendExprOperands ($$, $2);
			P_AppendExprOperands ($$, $3);
		}
	| A_AND expr expr
		{
			$$ = P_NewExpr ();	
			P_SetExprOpcode ($$, OP_Aand);
			P_AppendExprOperands ($$, $2);
			P_AppendExprOperands ($$, $3);
		}
	| A_OR expr expr
		{
			$$ = P_NewExpr ();	
			P_SetExprOpcode ($$, OP_Aor);
			P_AppendExprOperands ($$, $2);
			P_AppendExprOperands ($$, $3);
		}
	| A_XOR expr expr
		{
			$$ = P_NewExpr ();	
			P_SetExprOpcode ($$, OP_Axor);
			P_AppendExprOperands ($$, $2);
			P_AppendExprOperands ($$, $3);
		}
	| INDR expr
		{
			$$ = P_NewExpr ();	
			P_SetExprOpcode ($$, OP_indr);
			P_AppendExprOperands ($$, $2);
		}
	| ADDR expr
		{
			$$ = P_NewExpr ();	
			P_SetExprOpcode ($$, OP_addr);
			P_AppendExprOperands ($$, $2);
		}
	| INDEX expr expr_list_container
		{
			$$ = P_NewExpr ();
			P_SetExprOpcode ($$, OP_index);
			P_AppendExprOperands ($$, $2);
			P_AppendExprOperands ($$, $3);
		}
	| CALL expr expr_list_container
		{
			$$ = P_NewExpr ();	
			P_SetExprOpcode ($$, OP_call);
			P_AppendExprOperands ($$, $2);
			P_AppendExprOperands ($$, $3);
		}
	| ASMOPRD ST_LIT ST_LIT
		{
			int flag, i;

			$$ = P_NewExpr ();	
			P_SetExprOpcode ($$, OP_asm_oprd);
			P_SetExprAsmoprd ($$, P_NewAsmoprd ());
			
			flag = 0;
			for (i = 0; $2[i]; i++)
			{
				switch ($2[i])
				{
				case '+':
					flag |= 0x03;
					break;
				case '=':
					flag |= 0x02;
					break;
				case '&':
					flag |= 0x04;
					break;
				case '%':
					flag |= 0x08;
					break;
				case '*':
					flag |= 0x10;
					break;
				case '#':
					flag |= 0x20;
					break;
				case '?':
					flag |= 0x40;
					break;
				case '!':
					flag |= 0x80;
					break;
				case '"':
					break;
				default:
					assert (0);
				}
			}
			if (flag == 0)
				flag = 0x01;

			P_SetAsmoprdModifiers (P_GetExprAsmoprd ($$), flag);
			P_SetAsmoprdConstraints (P_GetExprAsmoprd ($$), 
						 P_DQString2String ($3));
			free ($2);
			free ($3);
		}
	| STMTEXPR stmt
		{
			$$ = P_NewExpr ();
			P_SetExprOpcode ($$, OP_stmt_expr);
			P_SetExprStmt ($$, $2);
		}
	| KW_NULL       /* NULL keyword - prevent conflict with pointer NULL */
		{
			$$ = P_NewExpr ();
			P_SetExprOpcode ($$, OP_null);
		}
	| SYNC 
		{
			$$ = P_NewExpr ();
			P_SetExprOpcode ($$, OP_sync);
		}
	| PHI expr expr_list_container
		{
			$$ = P_NewExpr ();	
			P_SetExprOpcode ($$, OP_phi);
			P_AppendExprOperands ($$, $2);
			P_AppendExprOperands ($$, $3);
		} 
;

expr_list: /* empty */
		{
			$$ = NULL;
		}
	| expr_list expr
		{
			$$ = P_AppendExprNext ($1, $2);
		}
;

expr_list_container: '(' expr_list ')'
		{
			Expr temp_expr, prev_temp_expr;

			$$ = $2;

			if ($$)
			{
				prev_temp_expr = $$;
				temp_expr = $$->next;

				while (temp_expr)
				{
					temp_expr->previous = prev_temp_expr;

					prev_temp_expr = temp_expr;
					temp_expr = temp_expr->next;
				}
			}
		}
;

field: '(' identifier TYPE key OFFSET I_LIT PARENT key pragma_list ')'
		{
			$$ = P_NewField ();
			P_SetFieldIdentifier ($$, $2);
			$2 = P_RemoveIdentifier ($2);
			P_SetFieldType ($$, $4);
			P_SetFieldIsBitField($$, 0);
			P_SetFieldOffset ($$, $6);
			P_SetFieldParentKey ($$, $8);
			P_SetFieldPragma ($$, $9);
		}
	/* A bitfield */
	| '(' identifier TYPE key BITFIELD I_LIT I_LIT OFFSET I_LIT PARENT key
              pragma_list ')'
		{
			$$ = P_NewField ();
			P_SetFieldIdentifier ($$, $2);
			$2 = P_RemoveIdentifier ($2);
			P_SetFieldType ($$, $4);
			P_SetFieldIsBitField($$, 1);
			P_SetFieldBitSize ($$, $6);
			P_SetFieldBitOffsetRemainder ($$, $7);
			P_SetFieldOffset ($$, $9);
			P_SetFieldParentKey ($$, $11);
			P_SetFieldPragma ($$, $12);
		}
;

field_list: /* empty */
		{
			$$ = NULL;
		}
	| field_list field
		{
			$$ = P_AppendFieldNext ($1, $2);
		}
;

file_type: SOURCE
		{
			$$ = FT_SOURCE;
		}
	| HEADER
		{
			$$ = FT_HEADER;
		}
;

func_dcl: '(' FUNCTION identifier TYPE key var_qual_list I_LIT pragma_list
	      var_dcl_list_container stmt position ')'
		{
			$$ = P_NewFuncDcl ();
			P_SetFuncDclIdentifier ($$, $3);
			$3 = P_RemoveIdentifier ($3);
			P_SetFuncDclType ($$, $5);
			P_SetFuncDclQualifier ($$, $6);
			P_SetFuncDclMaxExprID ($$, $7);
			P_SetFuncDclPragma ($$, $8);
			P_SetFuncDclParam ($$, $9);
			P_SetFuncDclStmt ($$, $10);
			if ($11)
			{
				P_SetFuncDclPosition ($$, $11);
				$11 = P_RemovePosition ($11);
			}

			if (Handlers[ES_FUNC])
				P_ExtRead (ES_FUNC, (void *)$$);
		}
	| '(' FUNCTION identifier TYPE key var_qual_list I_LIT pragma_list
	      position ')'
		{
			$$ = P_NewFuncDcl ();
			P_SetFuncDclIdentifier ($$, $3);
			$3 = P_RemoveIdentifier ($3);
			P_SetFuncDclType ($$, $5);
			P_SetFuncDclQualifier ($$, $6);
			P_SetFuncDclMaxExprID ($$, $7);
			P_SetFuncDclPragma ($$, $8);
			if ($9)
			{
				P_SetFuncDclPosition ($$, $9);
				$9 = P_RemovePosition ($9);
			}

			if (Handlers[ES_FUNC])
				P_ExtRead (ES_FUNC, (void *)$$);
		}
;

identifier: ID key
		{
			$$ = P_NewIdentifier ();
			P_SetIdentifierKey ($$, $2);
		}
	| ID ST_LIT key
		{
			$$ = P_NewIdentifier ();
			P_SetIdentifierName ($$, P_DQString2String ($2));
			free ($2);
			P_SetIdentifierKey ($$, $3);
		}
;

in_name: /* empty */
		{
			$$ = NULL;
		}
	| IN ST_LIT
		{
			$$ = P_DQString2String ($2);
			free ($2);
		}
;

include: '(' INCLUDE ST_LIT ')'
		{
			$$ = P_DQString2String ($3);
			free ($3);
		}
;

initializer: expr pragma_list
		{
			$$ = P_NewInit ();
			P_SetInitExpr ($$, $1);
			P_SetInitPragma ($$, $2);

			if (Handlers[ES_INIT])
				P_ExtRead (ES_INIT, (void *)$$);
		}
	| initializer_list_container pragma_list
		{
			$$ = P_NewInit ();
			P_SetInitSet ($$, $1);
			P_SetInitPragma ($$, $2);

			if (Handlers[ES_INIT])
				P_ExtRead (ES_INIT, (void *)$$);
		}
;

initializer_list: /* empty */
		{
			$$ = NULL;
		}
	| initializer_list initializer
		{
			$$ = P_AppendInitNext ($1, $2);
		}
;

initializer_list_container: '(' initializer_list ')'
		{
			$$ = $2;
		}
;

/* Only IPSTEF_NOT_AVAIL is preserved at the moment. */
ipste_flags: NOT_AVAIL
		{
			$$ = IPSTEF_NOT_AVAIL;
		}
;

ipste_flags_list: /* empty */
		{
			$$ = 0;
		}
	| ipste_flags_list ipste_flags
		{
			$$ = $1 | $2;
		}
;

ip_sym_tab_ent: '(' IP_TABLE ST_LIT I_LIT file_type in_name out_name
		    ipste_flags_list NUM_ENTRIES I_LIT OFFSET I_LIT
                    pragma_list ')'
		{
			$$ = P_NewIPSymTabEnt ();
			P_SetIPSymTabEntSourceName ($$,
						    P_DQString2String ($3));
			free ($3);
			P_SetIPSymTabEntKey ($$, $4);
			P_SetIPSymTabEntFileType ($$, $5);
			P_SetIPSymTabEntInName ($$, $6);
			P_SetIPSymTabEntOutName ($$, $7);
			P_SetIPSymTabEntFlags ($$, $8);
			P_SetIPSymTabEntNumEntries ($$, $10);
			P_SetIPSymTabEntOffset ($$, $12);
			P_SetIPSymTabEntPragma ($$, $13);

			if (P_TstIPSymTabEntFlags ($$, IPSTEF_NOT_AVAIL))
			  P_SetIPSymTabEntInFileStatus ($$, FS_NOT_AVAIL);

			if (Handlers[ES_IPSYMTABENT])
				P_ExtRead (ES_IPSYMTABENT, (void *)$$);
		}
;

key: '(' I_LIT I_LIT ')'
		{
			$$.file = $2;
			$$.sym = $3;
		}
;

label: '(' LABEL identifier ')'
		{
			$$ = P_NewLabel ();
			P_SetLabelType ($$, LB_LABEL);
			P_SetLabelIdentifier ($$, $3);
			$3 = P_RemoveIdentifier ($3);
		}
	| '(' CASE expr ')'
		{
			$$ = P_NewLabel ();
			P_SetLabelType ($$, LB_CASE);
			P_SetLabelExpression ($$, $3);
		}
	| '(' DEFAULT ')'
		{
			$$ = P_NewLabel ();
			P_SetLabelType ($$, LB_DEFAULT);
		}
;

label_list: /* empty */
		{
			$$ = NULL;
		}
	| label_list label
		{
			$$ = P_AppendLabelNext ($1, $2);
		}
;

named_basic_type: STRUCT
		{
			$$ = BT_STRUCT;
		}
	| UNION
		{
			$$ = BT_UNION;
		}
	| ENUM
		{
			$$ = BT_ENUM;
		}
;

out_name: /* empty */
		{
			$$ = NULL;
		}
	| OUT ST_LIT
		{
		   	$$ = P_DQString2String ($2);
			free ($2);
		}
;

param: '(' PARAM TYPE key ')'
		{
			$$ = P_NewParam ();
			P_SetParamKey ($$, $4);
		}
;

param_list: /* empty */
		{
			$$ = NULL;
		}
	| param_list param
		{
			$$ = P_AppendParamNext ($1, $2);
		}
;

parloop_index: identifier
		{
			$$ = P_NewExpr ();
			P_SetExprOpcode ($$, OP_var);
			P_SetExprVarIdentifier ($$, $1);
			$1 = P_RemoveIdentifier ($1);
		}
;

/* Note: Any rule that gets a position pointer is responsible for freeing
 * the pointer. */
position: /* empty */
		{
			$$ = NULL;
		}
	| POS ST_LIT I_LIT I_LIT
		{
			$$ = P_NewPosition ();
			P_SetPositionFilename ($$, P_DQString2String ($2));
			free ($2);
			P_SetPositionLineno ($$, $3);
			P_SetPositionColno ($$, $4);
		}
;

pragma: PRAGMA ST_LIT expr_list_container
		{
			$$ = P_NewPragma ();
			P_SetPragmaSpecifier ($$, P_DQString2String ($2));
			free ($2);
			P_SetPragmaExpr ($$, $3);
		}
;

pragma_list: /* empty */
		{
			$$ = NULL;
		}
	| pragma_list pragma
		{
			$$ = P_AppendPragmaNext ($1, $2);
		}
;

prof_st: /* empty */
		{
			$$ = NULL;
		}
	| PROFILE prof_st_list
		{
			$$ = $2;
		}
;

prof_st_list: /* empty */
		{
			$$ = NULL;
		}
	| F_LIT prof_st_list
		{
			ProfST new = P_NewProfST ();

			P_SetProfSTCount (new, $1);
			P_SetProfSTNext (new, $2);
			$$ = new;
		}
;

profile: /* empty */
		{
			$$ = 0.0;
		}
	| PROFILE F_LIT
		{
			$$ = $2;
		}
;

pstmt: PSTMT position pragma_list stmt
		{
			$$ = P_NewPstmt ();
			if ($2)
			{
				P_SetPstmtPosition ($$, $2);
				$2 = P_RemovePosition ($2);
			}
			P_SetPstmtPragma ($$, $3);
			P_SetPstmtStmt ($$, $4);

			if (Handlers[ES_PSTMT])
				P_ExtRead (ES_PSTMT, (void *)$$);
		}
;

scope: SCOPE key scope_entry
		{
			$$ = P_NewScope ();
			P_SetScopeKey ($$, $2);
			P_SetScopeScopeEntry ($$, $3);
		}
;

scope_entry: /* empty */
		{
			$$ = NULL;
		}
	| scope_entry key
		{
			$$ = \
			  P_AppendScopeEntryNext ($1,
						  P_NewScopeEntryWithKey ($2));
		}
;

size: /* empty */
		{
			$$ = 0;
		}
	| SIZE I_LIT
		{
			$$ = $2;
		}
;

shadow: /* empty */
		{
			$$ = -1;
		}
	| SHADOW I_LIT
		{
			$$ = $2;
		}
;

stmt: '(' label_list KEY key stmt_core pragma_list shadow position prof_st ')'
		{
			$$ = $5;
			P_SetStmtLabels ($$, $2);
			P_SetStmtKey ($$, $4);
			P_SetStmtPragma ($$, $6);

			if ($7 != -1)
			{
				P_SetStmtShadow \
				  ($$, 
				   P_NewShadowWithExprID \
				     (P_GetStmtShadow ($$), P_GetStmtExpr ($$),
				      $7));
			}

			if ($8)
			{
				P_SetStmtPosition ($$, $8);
				$8 = P_RemovePosition ($8);
			}

			if ($9)
				P_SetStmtProfile ($$, $9);

			if (Handlers[ES_STMT])
				P_ExtRead (ES_STMT, (void *)$$);
		}
;

stmt_core: expr
		{
			$$ = P_NewStmt ();
			P_SetStmtType ($$, ST_EXPR);
			P_SetStmtExpr ($$, $1);
		}
	| comp_stmt
		{
			$$ = $1;
		}
	| DO stmt WHILE expr
		{
			$$ = P_NewStmt ();
			P_SetStmtType ($$, ST_SERLOOP);
			P_SetStmtSerLoop ($$, P_NewSerLoop ());
			P_SetSerLoopLoopType (P_GetStmtSerLoop ($$), LT_DO);
			P_SetSerLoopLoopBody (P_GetStmtSerLoop ($$), $2);
			P_SetSerLoopCondExpr (P_GetStmtSerLoop ($$), $4);
		}
	| WHILE expr DO stmt
		{
			$$ = P_NewStmt ();
			P_SetStmtType ($$, ST_SERLOOP);
			P_SetStmtSerLoop ($$, P_NewSerLoop ());
			P_SetSerLoopLoopType (P_GetStmtSerLoop ($$), LT_WHILE);
			P_SetSerLoopCondExpr (P_GetStmtSerLoop ($$), $2);
			P_SetSerLoopLoopBody (P_GetStmtSerLoop ($$), $4);
		}
	| FOR expr_container expr_container expr_container DO stmt
		{
			$$ = P_NewStmt ();
			P_SetStmtType ($$, ST_SERLOOP);
			P_SetStmtSerLoop ($$, P_NewSerLoop ());
			P_SetSerLoopLoopType (P_GetStmtSerLoop ($$), LT_FOR);
			P_SetSerLoopInitExpr (P_GetStmtSerLoop ($$), $2);
			P_SetSerLoopCondExpr (P_GetStmtSerLoop ($$), $3);
			P_SetSerLoopIterExpr (P_GetStmtSerLoop ($$), $4);
			P_SetSerLoopLoopBody (P_GetStmtSerLoop ($$), $6);
		}
	| IF expr THEN stmt
		{
			$$ = P_NewStmt ();
			P_SetStmtType ($$, ST_IF);
			P_SetStmtIfStmt ($$, P_NewIfStmt ());
			P_SetIfStmtCondExpr (P_GetStmtIfStmt ($$), $2);
			P_SetIfStmtThenBlock (P_GetStmtIfStmt ($$), $4);
		}
	| IF expr THEN stmt ELSE stmt
		{
			$$ = P_NewStmt ();
			P_SetStmtType ($$, ST_IF);
			P_SetStmtIfStmt ($$, P_NewIfStmt ());
			P_SetIfStmtCondExpr (P_GetStmtIfStmt ($$), $2);
			P_SetIfStmtThenBlock (P_GetStmtIfStmt ($$), $4);
			P_SetIfStmtElseBlock (P_GetStmtIfStmt ($$), $6);
		}
	| SWITCH expr stmt
		{
			$$ = P_NewStmt ();
			P_SetStmtType ($$, ST_SWITCH);
			P_SetStmtSwitchStmt ($$, P_NewSwitchStmt ());
			P_SetSwitchStmtExpression (P_GetStmtSwitchStmt ($$),
						   $2);
			P_SetSwitchStmtSwitchBody (P_GetStmtSwitchStmt ($$),
						   $3);
		}
	| RETURN
		{
			$$ = P_NewStmt ();
			P_SetStmtType ($$, ST_RETURN);
		}
	| RETURN expr
		{
			$$ = P_NewStmt ();
			P_SetStmtType ($$, ST_RETURN);
			P_SetStmtRet ($$, $2);
		}
	| GOTO identifier
		{
			$$ = P_NewStmt ();
			P_SetStmtType ($$, ST_GOTO);
			P_SetStmtGotoIdentifier ($$, $2);
			$2 = P_RemoveIdentifier ($2);
		}
	| pstmt		
		{
			$$ = P_NewStmt ();
			P_SetStmtType ($$, ST_PSTMT);
			P_SetStmtPstmt ($$, $1);
		}
	| ADVANCE I_LIT
		{
			$$ = P_NewStmt ();
			P_SetStmtType ($$, ST_ADVANCE);
			P_SetStmtAdvance ($$, P_NewAdvance ());
			P_SetAdvanceMarker (P_GetStmtAdvance ($$), $2);
		}
	| AWAIT I_LIT I_LIT
		{
			$$ = P_NewStmt ();
			P_SetStmtType ($$, ST_AWAIT);
			P_SetStmtAwait ($$, P_NewAwait ());
			P_SetAwaitMarker (P_GetStmtAwait ($$), $2);
			P_SetAwaitDistance (P_GetStmtAwait ($$), $3);
		}
	| DOSERIAL INDEX parloop_index INIT expr FINAL expr INC expr pstmt
		{
			$$ = P_NewStmt ();
			P_SetStmtType ($$, ST_PARLOOP);
			P_SetStmtParLoop ($$, P_NewParLoop ());
			P_SetParLoopLoopType (P_GetStmtParLoop ($$),
					      LT_DOSERIAL);
			P_SetParLoopIterationVar (P_GetStmtParLoop ($$), $3);
			P_SetParLoopInitValue (P_GetStmtParLoop ($$), $5);
			P_SetParLoopFinalValue (P_GetStmtParLoop ($$), $7);
			P_SetParLoopIncrValue (P_GetStmtParLoop ($$), $9);
			P_SetParLoopPstmt (P_GetStmtParLoop ($$), $10);
		}
	| DOALL INDEX parloop_index INIT expr FINAL expr INC expr pstmt
		{
			$$ = P_NewStmt ();
			P_SetStmtType ($$, ST_PARLOOP);
			P_SetStmtParLoop ($$, P_NewParLoop ());
			P_SetParLoopLoopType (P_GetStmtParLoop ($$), LT_DOALL);
			P_SetParLoopIterationVar (P_GetStmtParLoop ($$), $3);
			P_SetParLoopInitValue (P_GetStmtParLoop ($$), $5);
			P_SetParLoopFinalValue (P_GetStmtParLoop ($$), $7);
			P_SetParLoopIncrValue (P_GetStmtParLoop ($$), $9);
			P_SetParLoopPstmt (P_GetStmtParLoop ($$), $10);
		}
	| DOACROSS INDEX parloop_index INIT expr FINAL expr INC expr pstmt
		{
			$$ = P_NewStmt ();
			P_SetStmtType ($$, ST_PARLOOP);
			P_SetStmtParLoop ($$, P_NewParLoop ());
			P_SetParLoopLoopType (P_GetStmtParLoop ($$),
					      LT_DOACROSS);
			P_SetParLoopIterationVar (P_GetStmtParLoop ($$), $3);
			P_SetParLoopInitValue (P_GetStmtParLoop ($$), $5);
			P_SetParLoopFinalValue (P_GetStmtParLoop ($$), $7);
			P_SetParLoopIncrValue (P_GetStmtParLoop ($$), $9);
			P_SetParLoopPstmt (P_GetStmtParLoop ($$), $10);
		}
	| DOSUPER INDEX parloop_index INIT expr FINAL expr INC expr pstmt
		{
			$$ = P_NewStmt ();
			P_SetStmtType ($$, ST_PARLOOP);
			P_SetStmtParLoop ($$, P_NewParLoop ());
			P_SetParLoopLoopType (P_GetStmtParLoop ($$),
					      LT_DOSUPER);
			P_SetParLoopIterationVar (P_GetStmtParLoop ($$), $3);
			P_SetParLoopInitValue (P_GetStmtParLoop ($$), $5);
			P_SetParLoopFinalValue (P_GetStmtParLoop ($$), $7);
			P_SetParLoopIncrValue (P_GetStmtParLoop ($$), $9);
			P_SetParLoopPstmt (P_GetStmtParLoop ($$), $10);
		}
	| MUTEX expr stmt
		{
			$$ = P_NewStmt ();
			P_SetStmtType ($$, ST_MUTEX);
			P_SetStmtMutex ($$, P_NewMutex ());
			P_SetMutexExpression (P_GetStmtMutex ($$), $2);
			P_SetMutexStatement (P_GetStmtMutex ($$), $3);
		}
	| COBEGIN stmt
		{
			$$ = P_NewStmt ();
			P_SetStmtType ($$, ST_COBEGIN);
			P_SetStmtCobegin ($$, P_NewCobegin ());
			P_SetCobeginStatements (P_GetStmtCobegin ($$), $2);
		}
	| asm_stmt
		{
			$$ = P_NewStmt ();
			P_SetStmtType ($$, ST_ASM);
			P_SetStmtAsmStmt ($$, $1);
		}
	| BREAK
		{
			$$ = P_NewStmt ();
			P_SetStmtType ($$, ST_BREAK);
		}
	| CONTINUE
		{
			$$ = P_NewStmt ();
			P_SetStmtType ($$, ST_CONT);
		}
	| KW_NULL       /* NULL keyword - prevent conflict with pointer NULL */
		{
			$$ = P_NewStmt ();
			P_SetStmtType ($$, ST_NOOP);
		}
;

stmt_list: /* empty */
		{
			$$ = NULL;
		}
	| stmt_list stmt
		{
			$$ = P_AppendStmtLexNext ($1, $2);
		}
;

stmt_list_container: '(' stmt_list ')'
		{
			$$ = $2;
		}
;

/* SQ_COMPARING is not preserved between modules. */
struct_qual: EMPTY
		{
			$$ = SQ_EMPTY;
		}
	| INCOMPLETE
		{
			$$ = SQ_INCOMPLETE;
		}
	| UNNAMED
		{
			$$ = SQ_UNNAMED;
		}
	| LINKMULTI
		{
			$$ = SQ_LINKMULTI;
		}
;

struct_qual_list: /* empty */
		{
			$$ = 0;
		}
	| struct_qual_list struct_qual
		{
			$$ = $1 | $2;
		}
;

symbol_table: '(' SYMBOL_TABLE NUM_ENTRIES I_LIT symbol_table_flags_list ')'
		{
			$$ = P_NewSymbolTable ();
			P_SetSymbolTableNumFiles ($$, $4);
			P_SetSymbolTableIPTable \
			  ($$, malloc (sizeof (_IPSymTabEnt *) * ($4 + 1)));
			P_SetSymbolTableFlags ($$, $5);
		}
	| '(' SYMBOL_TABLE END ')'
		{
			$$ = NULL;
		}
;

symbol_table_entry: '(' SYM END ')'
		{
			$$ = NULL;
		}
	| '(' SYM BLOCK key I_LIT ')'
		{
			$$ = P_NewSymTabEntry ();
			P_SetSymTabEntryType ($$, ET_BLOCK);
			P_SetSymTabEntryBlockStart ($$, $4);
			P_SetSymTabEntryBlockSize ($$, $5);
		}
	| '(' SYM identifier key entry_type OFFSET I_LIT pragma_list ')'
		{
			$$ = P_NewSymTabEntry ();
			P_SetSymTabEntryIdentifier ($$, $3);
			$3 = P_RemoveIdentifier ($3);
			P_SetSymTabEntryScopeKey ($$, $4);
			P_SetSymTabEntryType ($$, $5);
			P_SetSymTabEntryOffset ($$, $7);
			P_SetSymTabEntryPragma ($$, $8);

			if (Handlers[ES_SYMTABENTRY])
				P_ExtRead (ES_SYMTABENTRY, (void *)$$);
		}
	| '(' SYM identifier key entry_type scope OFFSET I_LIT pragma_list ')'
		{
			$$ = P_NewSymTabEntry ();
			P_SetSymTabEntryIdentifier ($$, $3);
			$3 = P_RemoveIdentifier ($3);
			P_SetSymTabEntryScopeKey ($$, $4);
			P_SetSymTabEntryType ($$, $5);
			P_SetSymTabEntryScope ($$, $6);
			P_SetSymTabEntryOffset ($$, $8);
			P_SetSymTabEntryPragma ($$, $9);

			if (Handlers[ES_SYMTABENTRY])
				P_ExtRead (ES_SYMTABENTRY, (void *)$$);
		}
;

/* Only STF_LINKED is preserved at the moment. */
symbol_table_flags: LINKED
		{
			$$ = STF_LINKED;
		}
;

symbol_table_flags_list: /* empty */
		{
			$$ = 0;
		}
	| symbol_table_flags_list symbol_table_flags
		{
			$$ = $1 | $2;
		}
;

type_dcl: '(' DEF type_definition ')'
		{
			$$ = $3;
		}
;

type_definition: '(' TYPEDEF identifier type_qual_list alignment REFS I_LIT key
                     pragma_list position ')'
		{
			$$ = P_NewDcl ();
			P_SetDclType ($$, TT_TYPE);
			P_SetDclTypeDcl ($$, P_NewTypeDcl ());
			P_SetTypeDclIdentifier (P_GetDclTypeDcl ($$), $3);
			$3 = P_RemoveIdentifier ($3);
			P_SetTypeDclQualifier (P_GetDclTypeDcl ($$), $4);
			P_SetTypeDclAlignment (P_GetDclTypeDcl ($$), $5);
			P_SetTypeDclRefs (P_GetDclTypeDcl ($$), $7);
			P_SetTypeDclBasicType (P_GetDclTypeDcl ($$),
					       BT_TYPEDEF_E);
			P_SetTypeDclType (P_GetDclTypeDcl ($$), $8);
			P_SetTypeDclPragma (P_GetDclTypeDcl ($$), $9);
			if ($10)
			{
				P_SetTypeDclPosition (P_GetDclTypeDcl ($$),
						      $10);
				$10 = P_RemovePosition ($10);
			}

			if (Handlers[ES_TYPE])
				P_ExtRead (ES_TYPE,
					   (void *)P_GetDclTypeDcl ($$));
		}
	| '(' TYPE type_spec ')'
		{
			$$ = P_NewDcl ();
			P_SetDclType ($$, TT_TYPE);
			P_SetDclTypeDcl ($$, $3);
		}
	| '(' STRUCT identifier field_list struct_qual_list pragma_list
	      position ')'
		{
			$$ = P_NewDcl ();
			P_SetDclType ($$, TT_STRUCT);
			P_SetDclStructDcl ($$, P_NewStructDcl ());
			P_SetStructDclIdentifier (P_GetDclStructDcl ($$), $3);
			$3 = P_RemoveIdentifier ($3);
			P_SetStructDclFields (P_GetDclStructDcl ($$), $4);
			P_SetStructDclQualifier (P_GetDclStructDcl ($$), $5);
			P_SetStructDclPragma (P_GetDclStructDcl ($$), $6);
			if ($7)
			{
				P_SetStructDclPosition (P_GetDclStructDcl ($$),
							$7);
				$7 = P_RemovePosition ($7);
			}

			if (Handlers[ES_STRUCT])
				P_ExtRead (ES_STRUCT,
					   (void *)P_GetDclStructDcl ($$));
		}
	| '(' UNION identifier field_list struct_qual_list pragma_list
	      position ')'
		{
			$$ = P_NewDcl ();
			P_SetDclType ($$, TT_UNION);
			P_SetDclUnionDcl ($$, P_NewUnionDcl ());
			P_SetUnionDclIdentifier (P_GetDclUnionDcl ($$), $3);
			$3 = P_RemoveIdentifier ($3);
			P_SetUnionDclFields (P_GetDclUnionDcl ($$), $4);
			P_SetUnionDclQualifier (P_GetDclUnionDcl ($$), $5);
			P_SetUnionDclPragma (P_GetDclUnionDcl ($$), $6);
			if ($7)
			{
				P_SetUnionDclPosition (P_GetDclUnionDcl ($$),
						       $7);
				$7 = P_RemovePosition ($7);
			}

			if (Handlers[ES_UNION])
				P_ExtRead (ES_UNION,
					   (void *)P_GetDclUnionDcl ($$));
		}
	| '(' ENUM identifier enum_field_list pragma_list position ')'
		{
			$$ = P_NewDcl ();
			P_SetDclType ($$, TT_ENUM);
			P_SetDclEnumDcl ($$, P_NewEnumDcl ());
			P_SetEnumDclIdentifier (P_GetDclEnumDcl ($$), $3);
			$3 = P_RemoveIdentifier ($3);
			P_SetEnumDclFields (P_GetDclEnumDcl ($$), $4);
			P_SetEnumDclPragma (P_GetDclEnumDcl ($$), $5);
			if ($6)
			{
				P_SetEnumDclPosition (P_GetDclEnumDcl ($$),
						      $6);
				$6 = P_RemovePosition ($6);
			}

			if (Handlers[ES_ENUM])
				P_ExtRead (ES_ENUM,
					   (void *)P_GetDclEnumDcl ($$));
		}
;

type_list: /* empty */
		{
			$$ = NULL;
		}
	| type_list type_spec
		{
		        $$ = List_insert_last ($1, $2);
		}
;

type_list_container: '(' type_list ')'
		{
			$$ = $2;
		}
;

type_qual: CONST                /* qualifiers */
		{
			$$ = TY_CONST;
		}
	| VOLATILE
		{
			$$ = TY_VOLATILE;
		}
	| SYNC
		{
			$$ = TY_SYNC;
		}
	| IMPLICIT
		{
			$$ = TY_IMPLICIT;
		}
	| DEFAULT
		{
			$$ = TY_DEFAULT;
		}
	| EXPLICIT_ALIGNMENT
		{
			$$ = TY_EXP_ALIGN;
		}
	| UNNAMED
		{
			$$ = TY_UNNAMED;
		}
;

type_qual_list: /* empty */
		{	
			$$ = 0;
		}
	| type_qual_list type_qual
		{
			$$ = $1 | $2;
		}
;

type_spec: '(' KEY key type_qual_list basic_type_list size alignment REFS I_LIT
               pragma_list position ')'
		{
			$$ = P_NewTypeDcl ();
			P_SetTypeDclKey ($$, $3);
			P_SetTypeDclQualifier ($$, $4);
			P_SetTypeDclBasicType ($$, $5);
			P_SetTypeDclSize ($$, $6);
			P_SetTypeDclAlignment ($$, $7);
			P_SetTypeDclRefs ($$, $9);
			P_SetTypeDclPragma ($$, $10);
			if ($11)
			{
				P_SetTypeDclPosition ($$, $11);
				$11 = P_RemovePosition ($11);
			}

			if (Handlers[ES_TYPE])
				P_ExtRead (ES_TYPE, (void *)$$);
		}
	| '(' named_basic_type identifier type_qual_list TYPE key size
              alignment REFS I_LIT pragma_list position ')'
		{
			$$ = P_NewTypeDcl ();
			P_SetTypeDclBasicType ($$, $2);
			P_SetTypeDclIdentifier ($$, $3);
			$3 = P_RemoveIdentifier ($3);
			P_SetTypeDclQualifier ($$, $4);
			P_SetTypeDclType ($$, $6);
			P_SetTypeDclSize ($$, $7);
			P_SetTypeDclAlignment ($$, $8);
			P_SetTypeDclRefs ($$, $10);
			P_SetTypeDclPragma ($$, $11);
			if ($12)
			{
				P_SetTypeDclPosition ($$, $12);
				$12 = P_RemovePosition ($12);
			}

			if (Handlers[ES_TYPE])
				P_ExtRead (ES_TYPE, (void *)$$);
		}
	| '(' KEY key type_qual_list ARRAY TYPE key DIM expr_container size
	      alignment REFS I_LIT pragma_list position ')'
		{
			$$ = P_NewTypeDcl ();
			P_SetTypeDclKey ($$, $3);
			P_SetTypeDclQualifier ($$, $4);
			P_SetTypeDclBasicType ($$, BT_ARRAY);
			P_SetTypeDclType ($$, $7);
			P_SetTypeDclArraySize ($$, $9);
			P_SetTypeDclSize ($$, $10);
			P_SetTypeDclAlignment ($$, $11);
			P_SetTypeDclRefs ($$, $13);
			P_SetTypeDclPragma ($$, $14);
			if ($15)
			{
				P_SetTypeDclPosition ($$, $15);
				$15 = P_RemovePosition ($15);
			}

			if (Handlers[ES_TYPE])
				P_ExtRead (ES_TYPE, (void *)$$);
		}
	| '(' KEY key type_qual_list FUNCTION ST_LIT RETURN TYPE key
	      param_list size alignment REFS I_LIT pragma_list position ')'
		{
			$$ = P_NewTypeDcl ();
			P_SetTypeDclKey ($$, $3);
			P_SetTypeDclQualifier ($$, $4);
			P_SetTypeDclBasicType ($$, BT_FUNC);
			P_SetTypeDclName ($$, P_DQString2String ($6));
			free ($6);
			P_SetTypeDclType ($$, $9);
			P_SetTypeDclParam ($$, $10);
			P_SetTypeDclSize ($$, $11);
			P_SetTypeDclAlignment ($$, $12);
			P_SetTypeDclRefs ($$, $14);
			P_SetTypeDclPragma ($$, $15);
			if ($16)
			{
				P_SetTypeDclPosition ($$, $16);
				$16 = P_RemovePosition ($16);
			}

			if (Handlers[ES_TYPE])
				P_ExtRead (ES_TYPE, (void *)$$);
		}
	| '(' KEY key type_qual_list POINTER TYPE key size alignment REFS I_LIT
	      pragma_list position ')'
		{
			$$ = P_NewTypeDcl ();
			P_SetTypeDclKey ($$, $3);
			P_SetTypeDclQualifier ($$, $4);
			P_SetTypeDclBasicType ($$, BT_POINTER);
			P_SetTypeDclType ($$, $7);
			P_SetTypeDclSize ($$, $8);
			P_SetTypeDclAlignment ($$, $9);
			P_SetTypeDclRefs ($$, $11);
			P_SetTypeDclPragma ($$, $12);
			if ($13)
			{
				P_SetTypeDclPosition ($$, $13);
				$13 = P_RemovePosition ($13);
			}

			if (Handlers[ES_TYPE])
				P_ExtRead (ES_TYPE, (void *)$$);
		}
	| '(' KEY key type_qual_list TYPEDEF TYPE key alignment REFS I_LIT
	      pragma_list position ')'
		{
			$$ = P_NewTypeDcl ();
			P_SetTypeDclKey ($$, $3);
			P_SetTypeDclQualifier ($$, $4);
			P_SetTypeDclBasicType ($$, BT_TYPEDEF_I);
			P_SetTypeDclType ($$, $7);
			P_SetTypeDclAlignment ($$, $8);
			P_SetTypeDclRefs ($$, $10);
			P_SetTypeDclPragma ($$, $11);
			if ($12)
			{
				P_SetTypeDclPosition ($$, $12);
				$12 = P_RemovePosition ($12);
			}

			if (Handlers[ES_TYPE])
				P_ExtRead (ES_TYPE, (void *)$$);
		}
;

var_dcl: '(' identifier TYPE key var_qual_list pragma_list position ')'
		 {
			$$ = P_NewVarDcl ();
			P_SetVarDclIdentifier ($$, $2);
			$2 = P_RemoveIdentifier ($2);
			P_SetVarDclType ($$, $4);
			P_SetVarDclQualifier ($$, $5);
			P_SetVarDclPragma ($$, $6);
			if ($7)
			{
				P_SetVarDclPosition ($$, $7);
				$7 = P_RemovePosition ($7);
			}

			if (Handlers[ES_VAR])
				P_ExtRead (ES_VAR, (void *)$$);
		}
	| '(' identifier TYPE key var_qual_list initializer_list_container
              pragma_list position ')'
		{
			$$ = P_NewVarDcl ();
			P_SetVarDclIdentifier ($$, $2);
			$2 = P_RemoveIdentifier ($2);
			P_SetVarDclType ($$, $4);
			P_SetVarDclQualifier ($$, $5);
			P_SetVarDclInit ($$, $6);
			P_SetVarDclPragma ($$, $7);
			if ($8)
			{
				P_SetVarDclPosition ($$, $8);
				$8 = P_RemovePosition ($8);
			}

			if (Handlers[ES_VAR])
				P_ExtRead (ES_VAR, (void *)$$);
		}
;

var_dcl_list: /* empty */
		{
			$$ = NULL;
		}
	| var_dcl_list var_dcl
		{
			$$ = List_insert_last ($1, $2);
		}
;

var_dcl_list_container: '(' var_dcl_list ')'
		{
			$$ = $2;
		}
;

var_qual: DEFINED
		{
			$$ = VQ_DEFINED;
		}			
	| COMMON
		{
			$$ = VQ_COMMON;
		}
	| REGISTER              /* class */
		{
			$$ = VQ_REGISTER;
		}
	| AUTO
		{
			$$ = VQ_AUTO;
		}
	| STATIC
		{
			$$ = VQ_STATIC;
		}
	| EXTERN
		{
			$$ = VQ_EXTERN;
		}
	| GLOBAL
		{
			$$ = VQ_GLOBAL;
		}
	| PARAMETER
		{
			$$ = VQ_PARAMETER;
		}
	| IMPLICIT
		{
			$$ = VQ_IMPLICIT;
		}
	| CDECL                 /* old dcltr qualifier */
		{
			$$ = VQ_CDECL;
		}
	| STDCALL
		{
			$$ = VQ_STDCALL;
		}
	| FASTCALL
		{
			$$ = VQ_FASTCALL;
		}
	| WEAK
		{
			$$ = VQ_WEAK;
		}
	| COMDAT
		{
			$$ = VQ_COMDAT;
		}
	| CONSTRUCTOR
		{
			$$ = VQ_CONSTRUCTOR;
		}
	| DESTRUCTOR
		{
			$$ = VQ_DESTRUCTOR;
		}
	| ELLIPSIS
		{
			$$ = VQ_APP_ELLIPSIS;
		}
	| OLD_PARAM
		{
			$$ = VQ_OLD_PARAM;
		}
;

var_qual_list: /* empty */
		{
			$$ = 0;
		}
	| var_qual_list var_qual
		{
			$$ = $1 | $2;
		}
;
%%

void
yyerror (char *s)
{
	P_warn ("parse.y: %s on line %d, col %d before %s", s, last_line,
	        last_column, yytext);

	return;
}
