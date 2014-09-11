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
 *      File: l_emul_util.h
 *      Authors: IMPACT Technologies, Inc. (Jake Janovetz)
 *      Creation Date:  June 1999
 *
 *      This file performs intrinsic emulation support for Lemulate.
 *      Intrinsic operations get converted to function calls which
 *      perform the equivalent function.  The intrinsic emulatiointrinsicn 
 *      functions are external and are linked with the emulation code.
 *
\*****************************************************************************/

#ifndef __LEM_UTIL_H__
#define __LEM_UTIL_H__

#include <config.h>
#include <Lcode/l_emul.h>

extern void L_read_parm_Lemulate (Parm_Parse_Info * ppi);
extern int C_is_multiple_of_eight (int num);
extern void C_make_multiple_of_eight (int *int_ptr);
extern char *C_layout_database_desc ();
extern int C_read_database_i (char *section_name, char *entry_name,
			      char *field_name);
extern char *C_read_database_s (char *section_name, char *entry_name,
				char *field_name);

extern int C_get_input (L_Datalist_Element ** element_ptr,
			Mem_Usage * mem_usage);
extern int C_peek_input (L_Datalist_Element ** element_ptr);
extern void C_write_brand (char *file_name, char *value, char *old_value);
extern void C_get_call_info (L_Func * fn, L_Oper * op, L_Attr * attr_list,
			     char *return_type_buf, char *parameter_type_buf,
			     int buf_size);
extern char *C_get_align_C_type (int desired_align);
extern int C_is_at_EOF (FILE * in);
extern int C_get_next_input_name (FILE * in, Mbuf * input_name,
				  Mbuf * output_name, Mbuf * header_name);

#endif /* __LEM_UTIL_H__ */
