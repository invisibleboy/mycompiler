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
 *      File:   md_compiler.h
 *
 *      Description: The header file for md_compiler.h
 *
 *      Creation Date:  Feb. 1995
 *
 *      Authors: John C. Gyllenhaal and Wen-mei Hwu
 *
 *      Revisions:
 *
\*****************************************************************************/

/* 10/29/02 REK Adding config.h */
#include <config.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>
#define MAXARGS 100
#include <string.h>
#include <ctype.h>

#if 0
/* Test out debug macros */
#define MD_DEBUG_MACROS
#endif

#include <library/mbuf.h>
#include <library/mfile.h>
#include <library/md.h>

/* Since the concrete syntax is not set, use defines for delimiters 
 * Must use quoted strings for delimiter tokens
 */
#define SECTION_START	"{"
#define SECTION_END	"}"

#define FIELD_START	"("
#define FIELD_END	")"

#define ENTRY_START	"("
#define ENTRY_END	")"

#define LINK_START	"("
#define LINK_END	")"

#define CREATE_SECTION	"CREATE"

#define TERMINATOR	";"
#define KLEENE_STAR	"*"
#define OR_MARKER	"|"

#define REPLACE_FIELD	"!"
#define CONCAT_FIELD	"||"


/* File pointer, used for reading MD file */
typedef struct Fptr
{
  Mptr *mptr;
  Mptr *checkpoint;		/* Checkpoint for error messages */
  Mbuf *source_name;		/* Source file name before preprocessing */
  int source_line;		/* Line number in unpreprocessed code */
}
Fptr;


/* Prototypes */
extern int Fpeekc (Fptr * fptr);
extern int Fgetc (Fptr * fptr);
extern void Fadvance_to_next_token (Fptr * fptr);
extern int is_md_ident (char *ident);

extern void Ferror (Fptr * fptr, char *fmt, ...);
extern void vFerror (Fptr * fptr, char *fmt, va_list args);
extern void L_punt (char *fmt, ...);

extern MD *build_md (FILE * in, char *input_file_name);


extern int check_md_for_ambiguous_links (FILE * out, MD * md);

extern void build_field_decl (MD * md, MD_Section * section, Fptr * fptr,
			      Mbuf * mbuf);
extern void build_entry (MD_Section * section, Fptr * fptr, Mbuf * mbuf);
extern void build_element (MD_Field * field, MD_Element_Req * element_req,
			   int element_index, Fptr * fptr, Mbuf * mbuf);
