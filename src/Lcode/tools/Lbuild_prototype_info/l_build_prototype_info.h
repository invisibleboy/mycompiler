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
 #      File:   l_build_prototype_info.c
 #
 #      Header file for l_build_prototype_info.c
 # 
 #      Created by John C. Gyllenhaal, Wen-mei Hwu - 3/99
\*****************************************************************************/

#ifndef L_BUILD_PROTOTYPE_INFO_H
#define L_BUILD_PROTOTYPE_INFO_H

#include <config.h>

/* Need for prototypes */
#include <stdio.h>
#include <Lcode/l_main.h>

/* Bit-field flags for L_deduce_prototypes */
#define DP_SILENT                     0x00000001
#define DP_USE_KNOWN_VARARGS_INFO     0x00000002

/* For ease of writing this thing, it is assumed that there is a maximum
 * prototype size.  The front end assumes 8k (as of 5/22/98), 
 * so TYPE_BUF_SIZE should be at least 8k.  This code will punt if
 * this is not large enough.
 */
#define TYPE_BUF_SIZE	8100

/* Make up a CTYPE_STRUCT since there is no L_CTYPE_STRUCT! */
#define CTYPE_STRUCT 0xff

/* See l_build_prototype_info.c for usage information */
extern void L_init_call_info ();
extern void L_delete_call_info ();
extern void L_collect_call_info (L_Func * L_fn);
extern void L_deduce_prototypes (STRING_Symbol_Table * func_labels_used,
                                 int flags);
extern void L_print_hcode_prototypes (FILE * out);
extern void L_print_C_prototypes (FILE * out, int ansi_c_format,
                                  int print_deduced_parms);
extern void L_convert_type_to_C_format (char *formatted_buf, char *raw_buf,
                                        char *param_name);
/* Useful parser of info strings */
extern void L_get_next_param_type (char *buf, char **info_ptr);
extern int L_convert_type_to_ctype (char *raw_buf);
extern void L_get_call_info (L_Func * fn, L_Oper * op, L_Attr * attr_list,
			     char *return_type_buf, char *parameter_type_buf,
			     int buf_size);

#endif
