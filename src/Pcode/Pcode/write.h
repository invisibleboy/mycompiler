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
 * \brief Declarations for routines to write Pcode to a file.
 *
 * \author Robert Kidd, Wen-mei Hwu
 *
 * This file contains declarations for routines to write Pcode to a file.
 *
 * For each rule in the grammar (parse.y), there should be a function
 * in this file to write that structure to disk.  The naming
 * convention is P_write_<rule name>.  Each function should take a
 * FILE * (the output file) as the first argument and an int
 * indicating the indent level as the last.  The appropriate data type
 * for the rule is passed as the second argument.
 */
/*****************************************************************************/

#ifndef _PCODE_WRITE_H_
#define _PCODE_WRITE_H_

#include <config.h>
#include <stdio.h>
#include "pcode.h"

/*! We'll aim for lines with LINE_LENGTH characters. */
#define LINE_LENGTH 80

/* An array to map opcode values to tokens. */
static const long op_to_token[OP_last];

/* The function to call to write Pcode. */
extern int P_write (FILE *out, Dcl dcl);
extern int P_write_dcllist (FILE *out, DclList dcl_list);

/* Functions to write non-terminals. */
extern int P_write_dcl (FILE *out, Dcl dcl, int indent, int *lines);
extern int P_write_alignment (FILE *out, int alignment, int indent,
			      int *lines);
extern int P_write_asm_clobbers (FILE *out, Expr asm_clobbers, int indent,
				 int *lines);
extern int P_write_asm_dcl (FILE *out, AsmDcl asm_dcl, int indent, int *lines);
extern int P_write_asm_operands (FILE *out, Expr asm_operands, int indent,
				 int *lines);
extern int P_write_asm_stmt (FILE *out, AsmStmt asm_stmt, int indent,
			     int *lines);
extern int P_write_basic_type (FILE *out, _BasicType basic_type, int indent,
			       int *lines);
extern int P_write_basic_type_list (FILE *out, _BasicType basic_type_list,
				    int indent, int *lines);
extern int P_write_comp_stmt (FILE *out, Stmt comp_stmt, int indent,
			      int *lines);
extern int P_write_comp_stmt_container (FILE *out, Stmt comp_stmt_container,
					int indent, int *lines);
extern int P_write_entry_type (FILE *out, _EntryType entry_type, int indent,
			       int *lines);
extern int P_write_enum_field (FILE *out, EnumField enum_field, int indent,
			       int *lines);
extern int P_write_enum_field_list (FILE *out, EnumField enum_field_list,
				    int indent, int *lines);
extern int P_write_expr (FILE *out, Expr expr, int indent, int *lines);
extern int P_write_expr_core (FILE *out, Expr expr_core, int indent,
			      int *lines);
extern int P_write_expr_list (FILE *out, Expr expr_list, int indent,
			      int *lines);
extern int P_write_expr_list_container (FILE *out, Expr expr_list_container,
					int indent, int *lines);
extern int P_write_extension (FILE *out, char *extension, int indent,
			      int *lines);
extern int P_write_field (FILE *out, Field field, int indent, int *lines);
extern int P_write_field_list (FILE *out, Field field_list, int indent,
			       int *lines);
extern int P_write_func_dcl (FILE *out, FuncDcl func_dcl, int indent,
			     int *lines);
extern int P_write_identifier (FILE *out, Identifier identifier, int indent,
			       int *lines);
extern int P_write_in_name (FILE *out, char *in_name, int indent, int *lines);
extern int P_write_include (FILE *out, char *include, int indent, int *lines);
extern int P_write_initializer (FILE *out, Init initializer, int indent,
				int *lines);
extern int P_write_initializer_list (FILE *out, Init initializer_list,
				     int indent, int *lines);
extern int P_write_initializer_list_container (FILE *out,
					       Init initializer_list_container,
					       int indent, int *lines);
extern int P_write_ipste_flags (FILE *out, _IPSTEFlags ipste_flags, int indent,
				int *lines);
extern int P_write_ipste_flags_list (FILE *out, _IPSTEFlags ipste_flags_list,
				     int indent, int *lines);
extern int P_write_ip_sym_tab_ent (FILE *out, IPSymTabEnt ip_sym_tab_enty,
				   int indent, int *lines);
extern int P_write_key (FILE *out, Key key, int indent, int *lives);
extern int P_write_label (FILE *out, Label label, int indent, int *lines);
extern int P_write_label_list (FILE *out, Label label_list, int indent,
			       int *lines);
extern int P_write_named_basic_type (FILE *out, _BasicType named_basic_type,
				     int indent, int *lines);
extern int P_write_out_name (FILE *out, char *out_name, int indent,
			     int *lines);
extern int P_write_param (FILE *out, Param param, int indent, int *lines);
extern int P_write_param_list (FILE *out, Param param_list, int indent,
			       int *lines);
extern int P_write_parloop_index (FILE *out, Expr parloop_index, int indent,
				  int *lines);
extern int P_write_position (FILE *out, Position position, int indent,
			     int *lines);
extern int P_write_pragma (FILE *out, Pragma pragma, int indent, int *lines);
extern int P_write_pragma_list (FILE *out, Pragma pragma_list, int indent,
				int *lines);
extern int P_write_prof_st (FILE *out, ProfST prof_st, int indent, int *lines);
extern int P_write_prof_st_list (FILE *out, ProfST prof_st_list, int indent,
				 int *lines);
extern int P_write_profile (FILE *out, double profile, int indent, int *lines);
extern int P_write_pstmt (FILE *out, Pstmt pstmt, int indent, int *lines);
extern int P_write_size (FILE *out, int size, int indent, int *lines);
extern int P_write_scope (FILE *out, Scope scope, int indent, int *lines);
extern int P_write_scope_entry (FILE *out, ScopeEntry scope_entry, int indent,
				int *lines);
extern int P_write_shadow (FILE *out, int shadow, int indent, int *lines);
extern int P_write_stmt (FILE *out, Stmt stmt, int indent, int *lines);
extern int P_write_stmt_core (FILE *out, Stmt stmt_core, int indent,
			      int *lines);
extern int P_write_stmt_list (FILE *out, Stmt stmt_list, int indent,
			      int *lines);
extern int P_write_stmt_list_container (FILE *out, Stmt stmt_list_container,
					 int indent, int *lines);
extern int P_write_struct_qual (FILE *out, _StructQual struct_qual, int indent,
				int *lines);
extern int P_write_struct_qual_list (FILE *out, _StructQual struct_qual_list,
				     int indent, int *lines);
extern int P_write_symbol_table (FILE *out, SymbolTable symbol_table,
				 int indent, int *lines);
extern int P_write_symbol_table_entry (FILE *out,
				       SymTabEntry symbol_table_entry,
				       int indent, int *lines);
extern int P_write_symbol_table_flags (FILE *out, _STFlags symbol_table_flags,
				       int indent, int *lines);
extern int P_write_symbol_table_flags_list (FILE *out,
					    _STFlags symbol_table_flags_list,
					    int indent, int *lines);
extern int P_write_type_dcl (FILE *out, Dcl type_dcl, int indent, int *lines);
extern int P_write_type_definition (FILE *out, Dcl type_definition,
				    int indent, int *lines);
extern int P_write_type_list (FILE *out, TypeList type_list, int indent,
			      int *lines);
extern int P_write_type_list_container (FILE *out,
					TypeList type_list_container,
					int indent, int *lines);
extern int P_write_type_qual (FILE *out, _TypeQual type_qual, int indent,
			      int *lines);
extern int P_write_type_qual_list (FILE *out, _TypeQual type_qual_list,
				   int indent, int *lines);
extern int P_write_type_spec (FILE *out, TypeDcl type_spec, int indent,
			      int *lines);
extern int P_write_var_dcl (FILE *out, VarDcl var_dcl, int indent, int *lines);
extern int P_write_var_dcl_list (FILE *out, VarList var_dcl_list, int indent,
				 int *lines);
extern int P_write_var_dcl_list_container (FILE *out,
					   VarList var_dcl_list_container,
					   int indent, int *lines);
extern int P_write_var_qual (FILE *out, _VarQual var_qual, int indent,
			     int *lines);
extern int P_write_var_qual_list (FILE *out, _VarQual var_qual_list,
				  int indent, int *lines);

/* Functions to write terminals. */
extern int P_write_token (FILE *out, long token, int indent);
extern int P_write_I_LIT (FILE *out, long i_lit, int indent);
extern int P_write_F_LIT (FILE *out, double f_lit, int indent);
extern int P_write_C_LIT (FILE *out, char c_lit, int indent);
extern int P_write_ST_LIT (FILE *out, char *st_lit, int indent);
extern int P_write_opcode (FILE *out, _Opcode opcode, int indent);

/* Utility functions. */
extern int P_write_offset (FILE *out, int *offset, int indent);

extern int P_write_space (FILE *out, int count);
extern int P_write_newline (FILE *out, int count);
extern int P_write_newline_indent (FILE *out, int indent);
extern int P_write_comment (FILE *out, char *comment, int indent);
extern int P_write_size_comment (FILE *out, int indent);

extern char *P_Asmmod2String (int modifiers);

#endif
