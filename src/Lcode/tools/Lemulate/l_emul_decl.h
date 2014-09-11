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
 *      File: l_emul_decl.h
 *      Authors: IMPACT Technologies, Inc. (Jake Janovetz)
 *      Creation Date:  June 1999
 *
 *      This file performs intrinsic emulation support for Lemulate.
 *      Intrinsic operations get converted to function calls which
 *      perform the equivalent function.  The intrinsic emulatiointrinsicn 
 *      functions are external and are linked with the emulation code.
 *
\*****************************************************************************/

#ifndef __LEM_DECL_H__
#define __LEM_DECL_H__

#include <config.h>
#include <Lcode/l_emul.h>

extern Reg_Usage *C_new_reg_usage (char *name);
extern void C_delete_reg_usage (Reg_Usage * reg_usage);
extern void C_add_reg_usage (Reg_Usage * reg_usage, L_Operand * operand);
extern void C_add_reg_id_usage (Reg_Usage * reg_usage, int reg_id, int ctype);
extern void C_add_fn_reg_usage (L_Func * fn, Reg_Usage * fn_scope_reg_usage,
				Reg_Usage * global_reg_usage);
extern Mem_Usage *C_new_mem_usage (char *name);
extern void C_init_file_scope_mem_usage (Mem_Usage * mem_usage);
extern void C_delete_mem_usage (Mem_Usage * mem_usage);
extern void C_reset_data_label_if_necessary (Mem_Usage * mem_usage,
					     char *label);
extern char *C_get_label_name (Mem_Usage * mem_usage);
extern void C_make_label_global (Mem_Usage * mem_usage, char *label);
extern int C_is_label_global (Mem_Usage * mem_usage, char *label);
extern void C_set_label_decl_cast (Mem_Usage * mem_usage, char *label,
				   L_Type * h_type);
extern char *C_get_label_decl (Mem_Usage * mem_usage, char *label);
extern char *C_get_label_cast (Mem_Usage * mem_usage, char *label);
extern void C_set_label_alignment (Mem_Usage * mem_usage, char *label,
				   int align);
extern int C_get_label_alignment (Mem_Usage * mem_usage, char *label);
extern void C_set_label_element_size (Mem_Usage * mem_usage, char *label,
				      int size);
extern int C_get_label_element_size (Mem_Usage * mem_usage, char *label);
extern void C_add_mem_usage (STRING_Symbol_Table * table, char *label);
extern void C_add_fn_mem_usage (L_Func * fn, Mem_Usage * mem_usage);
extern void C_add_data_mem_usage (L_Data * data, Mem_Usage * mem_usage);
extern int C_deduce_program_scope_labels (Mem_Usage * mem_usage);
extern void C_emit_undefined_label_extern (FILE * extern_out,
					   Mem_Usage * mem_usage);
extern void C_emit_reg_array_decl (FILE * out, INT_Symbol_Table * table,
				   char *line_prefix, char *var_prefix);
extern void C_emit_specific_reg_decls (FILE * out, INT_Symbol_Table * table,
				       char *line_prefix, char *var_prefix,
				       char *var_postfix, int is_macro);
extern void C_emit_reg_decls (FILE * out, Reg_Usage * reg_usage,
			      char *passed_line_prefix);
extern void C_emit_fn_declaration (FILE * out, L_Func * fn,
				   Mem_Usage * mem_usage);
extern STRING_Symbol_Table *C_build_string_map (L_Func * fn, int start_id);
extern void C_emit_string_decls (FILE * out,
				 STRING_Symbol_Table * string_map);


extern char *C_generate_union_name (FILE * extern_out, int data_size,
				    int data_align, Mem_Usage * mem_usage);
extern void C_dclptr_to_C_string (char *dest_buf, L_Dcltr * dclptr,
				  char *incoming_buf);
extern void C_emit_field_decl (FILE * out, L_Data * data);
extern int C_emit_def_struct_union (FILE * out, L_Data * def_data,
				    Mem_Usage * mem_usage, int is_union,
				    L_Datalist_Element ** jump_tbl_data);
extern int C_emit_def_enum (FILE * out, L_Data * def_data,
			    Mem_Usage * mem_usage,
			    L_Datalist_Element ** jump_tbl_data);
extern void C_emit_non_trapping_load_prototypes (FILE * out);
extern void C_emit_typedefs (FILE * out);
extern void C_emit_database_struct_union_def (FILE * struct_out,
					      char *struct_name,
					      int is_union);
extern void C_emit_database_struct_union_defs (FILE * struct_out,
					       Mem_Usage * mem_usage);

#endif /* __LEM_DECL_H__ */
