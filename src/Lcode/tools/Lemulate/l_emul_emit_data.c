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
 *      File: l_emul_data.c
 *      Authors: IMPACT Technologies, Inc. (Jake Janovetz)
 *      Creation Date:  June 1999
 *
 *      This file performs intrinsic emulation support for Lemulate.
 *      Intrinsic operations get converted to function calls which
 *      perform the equivalent function.  The intrinsic emulation 
 *      functions are external and are linked with the emulation code.
 *
\*****************************************************************************/

#include <config.h>
#include <Lcode/l_emul.h>
#include "l_emul_decl.h"
#include "l_emul_emit_builtin.h"
#include "l_emul_emit_data.h"
#include "l_emul_emit_op.h"
#include "l_emul_emit_operand.h"
#include "l_emul_intrinsic.h"
#include "l_emul_trace.h"
#include "l_emul_util.h"

#ifdef INTRINSICS
#include "l_emul_intrinsic.h"
#endif

/* Returns 1 if token_type specifies initialization data, 0 otherwise */
int
C_is_init_data (int token_type)
{
  int is_init_data;

  switch (token_type)
    {
    case L_INPUT_WB:
    case L_INPUT_WW:
    case L_INPUT_WI:
    case L_INPUT_WQ:
    case L_INPUT_WF:
    case L_INPUT_WF2:
    case L_INPUT_WS:
      is_init_data = 1;
      break;

    default:
      is_init_data = 0;
      break;
    }

  return (is_init_data);
}

/* Emits comments, include, the init flag variable, and starts
 * the initialization function definition.
 *
 * If there are no data initialization routines, prints
 * out NULL macro and does not emit anything else.
 */
void
C_emit_data_init_function (FILE * init_out, FILE * extern_out,
			   Mem_Usage * mem_usage)
{
  STRING_Symbol *symbol;
  char *init_routine_name;

  /* Print out header for init_out */
  fprintf (init_out,
	   "int main (argc, argv)\n"
	   "   int argc;\n" "   char **argv;\n" "{\n" "   int rv;\n");

  /* Go through each initialization routine created and print
   * out call to each one.
   */
  for (symbol = mem_usage->init_routines_created->head_symbol;
       symbol != NULL; symbol = symbol->next_symbol)
    {
      /* Get init routine name for ease of use */
      init_routine_name = symbol->name;

      /* Print out call to this routine in data init function */
      fprintf (init_out,
	       "\n"
	       "%s{extern void %s%s();\n"
	       "%s%s%s();}\n",
	       C_indent, C_prefix, init_routine_name,
	       C_indent, C_prefix, init_routine_name);
    }
  fprintf (init_out, "   rv = %sreal_main(argc,argv);\n"
	   "return rv;\n" "}\n", C_prefix);
}

/* Emit the C version of each Lcode data declaration.  If the data
 * is GLOBAL, also emit an 'extern' declaration to the extern_out
 * file for use by other C files.  For initialized arrays, structures,
 * and unions, generates C functions to intializes them.  The name of
 * these functions are added to mem_usage->init_routines_created and
 * the calls to them will be emitted in the last file processed.
 * Structure and Union definitions are written to 'struct_out'.
 *
 * If jump_tbl_data is not NULL, L_INPUT_RESERVE will pull the initialization
 * data from this datalist, instead of from the input file.  It will
 * also update the jump_tabls pointer appropriately.
 */
void
C_emit_data (FILE * out, FILE * extern_out, FILE * struct_out,
	     L_Data * data, Mem_Usage * mem_usage,
	     L_Datalist_Element ** jump_tbl_data)
{
  int peeked_type;
  char *struct_name;
  InitInfo init_info;
  char trace_routine_name[TYPE_BUF_SIZE];

  /* Handle each type of data appropriately */
  switch (data->type)
    {
      /* Specifies the alignment requirements for the label. 
       * Used by C_emit_reserve_data()
       */
    case L_INPUT_ALIGN:
      C_set_label_alignment (mem_usage, C_get_data_label (data->address),
			     data->N);

      /* Sanity check, this type of data should not have value */
      C_verify_no_data_value (data);
      break;

      /* Specifies that the specified label has global scope */
    case L_INPUT_GLOBAL:
      C_make_label_global (mem_usage, C_get_data_label (data->address));

      /* If declaration information is piggybacked on global specifier,
       * read in declaration for label.
       */
      if (data->h_type != NULL)
	{
	  C_set_label_decl_cast (mem_usage, C_get_data_label (data->address),
				 data->h_type);
	}
      mem_usage->object_id = data->id;

      /* Sanity check, this type of data should not have value */
      C_verify_no_data_value (data);
      break;

      /* Declares memory for array, structure, or union.  Will consume
       * initialization data specified immediately after this token,
       * if present.
       */
    case L_INPUT_RESERVE:
      /* Emit declaration of variable of type structure, union, or array */
      init_info = C_emit_reserve_data (out, extern_out, struct_out,
				  data, mem_usage, jump_tbl_data);

      /* SER 20040509: Addition for HCH MICRO 04 */
      if (C_insert_probes && C_trace_mem_addrs && C_custom_profiling &&
	  mem_usage->object_id != 0 && mem_usage->object_id != -1)
	{
	  if (!mem_usage->data_label_name)
	    L_punt ("C_emit_data / trace_objects: trying to trace "
		    "an unnamed object");
	  sprintf (trace_routine_name, "trace_glob_%i_%s", ++C_unique_id,
		   mem_usage->data_label_name);

	  /* Add to table of initialization routines create */
	  STRING_add_symbol (mem_usage->init_routines_created,
			     trace_routine_name, NULL);
	  fprintf (out, "void %s%s()\n" "{\n", C_prefix, trace_routine_name);
	  fprintf (out, "   _EM_put_trace(L_TRACE_ASYNCH);\n");
	  fprintf (out, "   _EM_put_trace3(L_TRACE_OBJ_GLOB,%d,&(%s));\n",
		   mem_usage->object_id, mem_usage->data_label_name);
	  fprintf (out, "   _EM_put_trace2(%d, %d);\n}\n", data->N,
		   mem_usage->data_label_element_size);
	  mem_usage->object_id = -10;
	}
      /* End of addition. */

      if (C_trace_objects)
	{
	  char trace_routine_name[TYPE_BUF_SIZE];

	  if (!mem_usage->data_label_name)
	    L_punt ("C_emit_data / trace_objects: trying to trace "
		    "an unnamed object");

	  sprintf (trace_routine_name, "trace_glob_%i_%s", ++C_unique_id,
		   mem_usage->data_label_name);

	  /* Add to table of initialization routines create */
	  STRING_add_symbol (mem_usage->init_routines_created,
			     trace_routine_name, NULL);

	  /* Print out C function wrapper for initialization */
	  fprintf (out, "void %s%s()\n" "{\n", C_prefix, trace_routine_name);
	  fprintf (out, "   _EM_put_trace(L_TRACE_ASYNCH);\n");
	  fprintf (out, "   _EM_put_trace3(L_TRACE_OBJ_GLOB,"
		   "&%s,%d);\n", mem_usage->data_label_name, init_info.data_size);
	  fprintf (out, "   _EM_put_trace(%i);\n", (init_info.data_size == init_info.tot_inits));
	  fprintf (out, "}\n");
	}
      break;

      /* Specifies memory section to put next data.  Leave to 
       * host compiler to place data and code in the sections it chooses.
       */
    case L_INPUT_MS:
      /* It doesn't appear that any special handling is needed for
       * "data", "bss", or "text".  Punt if hit some other section.
       */
      if ((strcmp (L_ms_name (data->N), "data") != 0) &&
	  (strcmp (L_ms_name (data->N), "sdata") != 0) &&
	  (strcmp (L_ms_name (data->N), "rodata") != 0) &&
	  (strcmp (L_ms_name (data->N), "bss") != 0) &&
	  (strcmp (L_ms_name (data->N), "sbss") != 0) &&
	  (strcmp (L_ms_name (data->N), "text") != 0))
	{
	  L_punt ("C_emit_data: Unhandled MS section '%s'!",
		  L_ms_name (data->N));
	}

      /* Sanity check, this type of data should not have value */
      C_verify_no_data_value (data);
      break;

      /* Sets array element size, used by C_emit_reserved_data() */
    case L_INPUT_ELEMENT_SIZE:
      C_set_label_element_size (mem_usage, C_get_data_label (data->address),
				data->N);
      break;

      /* Initialization data for arrays and structures.  Should not see
       * here since handled by C_emit_reserved_data()
       */
    case L_INPUT_WB:
    case L_INPUT_WW:
    case L_INPUT_WI:
    case L_INPUT_WQ:
    case L_INPUT_WF:
    case L_INPUT_WF2:
    case L_INPUT_WS:
      L_print_data (stderr, data);
      L_punt ("C_emit_data: type '%i' should have been handled by "
	      "C_emit_reserved_data()!", data->type);
      break;

      /* Character variable declaration */
    case L_INPUT_BYTE:
      C_emit_base_type (out, extern_out, data, mem_usage, "char");
      break;

      /* Short variable declaration */
    case L_INPUT_WORD:
      C_emit_base_type (out, extern_out, data, mem_usage, "short");
      break;

      /* Int (assumed to be the same as long) variable declaration */
    case L_INPUT_LONG:
      C_emit_base_type (out, extern_out, data, mem_usage, "int");
      break;

      /* Longlong variable declaration */
    case L_INPUT_LONGLONG:
      C_emit_base_type (out, extern_out, data, mem_usage, "longlong");
      break;

      /* Float variable declaration */
    case L_INPUT_FLOAT:
      C_emit_base_type (out, extern_out, data, mem_usage, "float");
      break;

      /* Double variable declaration */
    case L_INPUT_DOUBLE:
      C_emit_base_type (out, extern_out, data, mem_usage, "double");
      break;

      /* Emit struct/union/emum definition if have not already defined
       * it.
       */
    case L_INPUT_DEF_STRUCT:
    case L_INPUT_DEF_UNION:
    case L_INPUT_DEF_ENUM:
      {
	STRING_Symbol *sym;
	/* Get the name of the struct/union/enum 
	 * Use raw label because the data->address does not get '_' prepended.
	 */
	struct_name = C_get_raw_data_label (data->address);

	/* If first time encountered struct/union/enum, define
	 * it in struct_out.
	 */
	if (!(sym = STRING_find_symbol (mem_usage->struct_names_defined, 
					struct_name)) ||
	    (!sym->data && 
	     (((peeked_type = C_peek_input (jump_tbl_data)) == L_INPUT_FIELD) ||
	      (peeked_type == L_INPUT_ENUMERATOR))))
	  {
	    int def = 0;

	    /* Emit appropriate declaration to struct_out */
	    if (data->type == L_INPUT_DEF_STRUCT)
	      {
		/* Emit struct definition (including fields in struct) */
		def = C_emit_def_struct_union (struct_out, data, mem_usage, 0,
					       jump_tbl_data);
	      }
	    else if (data->type == L_INPUT_DEF_UNION)
	      {
		/* Emit union definition (including fields in union) */
		def = C_emit_def_struct_union (struct_out, data, mem_usage, 1,
					       jump_tbl_data);
	      }
	    else if (data->type == L_INPUT_DEF_ENUM)
	      {
		/* Emit enum definition, with enumerators and their values */
		/* Normal path through IMPACT removes these, this is
		 * primarily for documentation. -ITI/JCG 4/99
		 */
		def = C_emit_def_enum (struct_out, data, mem_usage, 
				       jump_tbl_data);
	      }
	    else
	      {
		L_punt ("C_emit_data: unexpectly got type '%i'!", data->type);
	      }

	    /* No, add name to table */

	    if (!sym)
	      {
#if LP64_ARCHITECTURE
		STRING_add_symbol (mem_usage->struct_names_defined,
				   struct_name, (void *)((long)def));
#else
		STRING_add_symbol (mem_usage->struct_names_defined,
				   struct_name, (void *) def);
#endif
	      }      
	    else
	      {
#if LP64_ARCHITECTURE
		sym->data = (void *)((long) def);
#else
		sym->data = (void *) def;
#endif
	      }
	  }
	/* Otherwise, skip over the initialization data */
	else
	  {
	    while (
		   ((peeked_type = C_peek_input (jump_tbl_data)) ==
		    L_INPUT_FIELD) || (peeked_type == L_INPUT_ENUMERATOR))
	      {
		/* Get this field/enumerator (instead of just peeking at it) */
		C_get_input (jump_tbl_data, mem_usage);
		
		/* Free the data we just read in (if jump_tbl_data == NULL).
		 * Otherwise, this data (from a jump table) will be freed when
		 * the function is freed.
		 */
		if (jump_tbl_data == NULL)
		  {
		    L_delete_data (C_data);
		  }
	      }
	  }
      }
      break;

    case L_INPUT_FIELD:
      L_punt ("C_emit_data: type '%i' should have been handled by "
	      "C_emit_def_struct_union()!", data->type);
      break;

    case L_INPUT_ENUMERATOR:
      L_punt ("C_emit_data: type '%i' should have been handled by "
	      "C_emit_def_enum()!", data->type);
      break;

    default:
      L_print_data (stderr, data);
      L_punt ("C_emit_data: Unknown type %i!\n", data->type);
      break;
    }

}

/* Emit the function's hash tables in a form that can be
 * faithfully emulated in C.
 */
void
C_emit_fn_jump_tables (FILE * out, FILE * extern_out,
		       FILE * struct_out, L_Func * fn, Mem_Usage * mem_usage)
{
  L_Datalist_Element *jumptbl_element, *jumptbl_data_ptr;
  L_Data *data;
  L_Oper *op;
  L_Cb *cb;
  INT_Symbol_Table *cb_id_table;
  INT_Symbol *cb_id_symbol;
  Heap *cb_id_heap;
  char func_buf[10000];
  int cb_id, token_type;
  char *cb_label;
  int index;

  /* Emit comment on what we are doing */
  fprintf (out,
	   "\n"
	   "/* Emulate cb labels (jump targets) in C using #defines */\n");

  /* Make int symbol table to hold cb id's referenced */
  cb_id_table = INT_new_symbol_table ("cb_id", 0);

  /* Scan thru all the hash tables, looking for cb references */
  for (jumptbl_element = fn->jump_tbls->first_element; jumptbl_element;
       jumptbl_element = jumptbl_element->next_element)
    {
      /* Get data for ease of use */
      data = jumptbl_element->data;

      /* Look for hash table initialization data items */
      if ((data->type == L_INPUT_WI || data->type == L_INPUT_WQ) && 
	  (data->value->type == L_EXPR_LABEL))
	{
	  /* Sanity check, make sure will not exceed buffer */
	  if (strlen (data->value->value.l) >= sizeof (func_buf))
	    L_punt ("C_emit_fn_jump_tables: label too long (%i):\n"
		    "%s",
		    strlen (data->value->value.l), data->value->value.l);

	  /* Get cb id using Mspec call */
	  if (!M_is_cb_label (data->value->value.l, func_buf, &cb_id))
	    L_punt ("C_emit_fn_jump_tables: cb label expected!");

	  /* If id not already in table, add it */
	  if (INT_find_symbol (cb_id_table, cb_id) == NULL)
	    INT_add_symbol (cb_id_table, cb_id, data->value->value.l);
	}
    }

  /* Sanity check, better have found at least one cb id! */
  if (cb_id_table->head_symbol == NULL)
    L_punt ("C_emit_fn_jump_tables: At least one cb label expected!");

  /* Sort cb_ids for readability using heap library */
  cb_id_heap = Heap_Create (HEAP_MIN);

  /* Add every cb id found to the heap */
  for (cb_id_symbol = cb_id_table->head_symbol; cb_id_symbol != NULL;
       cb_id_symbol = cb_id_symbol->next_symbol)
    {
      /* Get cb id and label for ease of use */
      cb_id = cb_id_symbol->value;
      cb_label = (char *) cb_id_symbol->data;

      /* Add cb_id to heap */
      Heap_Insert (cb_id_heap, (void *) cb_label, (double) cb_id);
    }

  /* Emit case for every cb id found in hash tables for this function */
  while ((cb_label = (char *) Heap_ExtractTop (cb_id_heap)) != ((int) NULL))
    {
      /* Get cb id again using Mspec call */
      if (!M_is_cb_label (cb_label, func_buf, &cb_id))
	L_punt ("C_emit_fn_jump_tables: cb label expected!");
      fprintf (out, "#define %s%s  %i\n", C_prefix, cb_label, cb_id);
    }

  /* Free heap, nothing to free (flagged with NULL) */
  cb_id_heap = Heap_Dispose (cb_id_heap, NULL);

  /* Scan function for "unexpected" cb labels.  Some of the hash table
   * optimizations when optimizing 126.gcc create comparisions to
   * cb labels that are no longer in the hash table (due to cb renaming).
   * To make 126.gcc work anyway, look for these labels and add it to
   * the table. -6/99
   */
  for (cb = fn->first_cb; cb != NULL; cb = cb->next_cb)
    {
      for (op = cb->first_op; op != NULL; op = op->next_op)
	{
	  /* Don't want to add any extra labels.  Only JUMP ops
	   * should have cb operands in src[0] and src[1].  All others
	   * should be due to hash table optimizations.
	   */
	  if ((op->opc == Lop_JUMP) || (op->opc == Lop_JUMP_FS))
	    continue;

	  /* Only look in src[0] and src[1], since don't want
	   * to get all the targets of conditional branches.
	   */
	  for (index = 0; index <= 1; index++)
	    {
	      /* Skip non-CB operands */
	      if ((op->src[index] == NULL) ||
		  (op->src[index]->type != L_OPERAND_CB))
		continue;

	      /* Get cb id of label */
	      cb_id = op->src[index]->value.cb->id;

	      /* If id not already in table, skip it */
	      if (INT_find_symbol (cb_id_table, cb_id) != NULL)
		continue;

	      /* Add to table to prevent duplicates */
	      INT_add_symbol (cb_id_table, cb_id, "(extra!)");

	      /* Print out the extra define */
	      fprintf (out, "/* Added for op %i (not in hash table!) */\n",
		       op->id);
	      fprintf (out, "#define ");
	      C_emit_operand (out, fn, op->src[index]);
	      fprintf (out, "  %i\n", cb_id);
	    }
	}
    }

  /* Emit comment on what we are doing */
  fprintf (out, "\n" "/* Jump table(s) for function '%s' */\n", fn->name);

  /* Process jump table data as if it was read from input stream.
   * Enhanced C_get_input() and C_peek_input() to support this.
   */
  jumptbl_data_ptr = fn->jump_tbls->first_element;

  while ((token_type = C_get_input (&jumptbl_data_ptr, mem_usage)) !=
	 L_INPUT_EOF)
    {
      if (token_type == L_INPUT_FUNCTION)
	{
	  L_punt ("C_emit_fn_jump_tables: Function data not expected!");
	}
      else
	{
	  /* Get jump table data from global variable for clarity */
	  data = C_data;

	  /* Emit data in C, read initialization data, etc. from
	   * the jump table data list, not the input stream!
	   */
	  C_emit_data (out, extern_out, struct_out,
		       data, mem_usage, &jumptbl_data_ptr);

	  /* Don't free data!  Will be done when function freed! */
	}
    }

  /* Free the cb_id table, no data to delete, so use NULL for free routine */
  INT_delete_symbol_table (cb_id_table, NULL);
}

/* Emit data declarations for the 'reserve' data type.  This type is
 * used to allocate arrays, structures, and unions.  Although the fields of
 * the structure/union doesn't matter, it must be of the specified
 * size and have the specified alignment.
 *
 * If declaration information is present in Lcode (and we are using it):
 *   Use decl info to declare the label the same was as it was in the
 *   source program.  
 *
 * Otherwise:
 *   For all cases, it will use a union of a char array (to yield the proper
 *   size) and a basic type (to yield the proper alignment).
 *
 * If the array, struct, etc. is initialized, this routine will
 * also emit a C function that initializes the specified sections
 * of the memory (that is called at startup).
 *
 * This routine assumes sizeof(char) is 1 byte and that one of the basic 
 * types (short, int, etc.) will yield the requested alignment.
 *
 * If jump_tbl_data is not NULL, will pull the initialization
 * data from this datalist, instead of from the input file.  It will
 * also update the jump_tabls pointer appropriately.
 */

InitInfo
C_emit_reserve_data (FILE * out, FILE * extern_out,
		     FILE * struct_out,
		     L_Data * data, Mem_Usage * mem_usage,
		     L_Datalist_Element ** jump_tbl_data)
{
  char decl_buf[TYPE_BUF_SIZE], *actual_decl;
  char init_routine_name[TYPE_BUF_SIZE];
  L_Data *init_data;
  char *data_label, *union_name;
  int data_align, data_global;
  char *cast_type = NULL;
  int token_type;
  int num_inits;
  int size_of_init;
  /* 10/25/04 REK Commenting out unused variable to quiet compiler warning. */
#if 0
  int tot_inits;
#endif

  InitInfo init_info;
  
  /* Get the label that we are reserving memory for */
  if ((data_label = C_get_label_name (mem_usage)) == NULL)
    L_punt ("C_emit_reserve_data: NULL label name during RESERVE!");

  /* Sanity check, this type of data should not have value */
  C_verify_no_data_value (data);

  /* Generate a new unique id for our struct name and initialization code */
  C_unique_id++;

  /* Get the size being reserved */
  init_info.data_size = data->N;

  /* Sanity check */
  if (init_info.data_size < 1)
    {
      L_punt ("C_emit_reserve_data: Invalid data size '%i' during RESERVE!",
	      init_info.data_size);
    }

  /* Get requested data alignment */
  data_align = C_get_label_alignment (mem_usage, data_label);

  /* If no align info, assume alignment of 1 byte */
  if (data_align < 1)
    {
      data_align = 1;
    }

  /* If not global, start declaration by printing out 'static' */
  data_global = C_is_label_global (mem_usage, data_label);
  if (!data_global)
    {
      /* Record that we have locally defined this data label */
      C_add_mem_usage (mem_usage->file_scope_data_labels_defined, data_label);
    }
  else
    {
      /* Record that we have globally defined this data label */
      C_add_mem_usage (mem_usage->program_scope_data_labels_defined,
		       data_label);
    }

  /* Get the actual declaration for this label, if info present */
  actual_decl = C_get_label_decl (mem_usage, data_label);

  /* If have actual declaration info, use it */
  if (actual_decl != NULL)
    {
      strcpy (decl_buf, actual_decl);
    }

  /* Otherwise, create a valid but inexact declaration for it */
  else
    {
      /* Use a union to enforce size and alignment requirements.
       * Use helper function to create the appropriate union and return
       * the name to use to reference this union.
       */
      union_name = C_generate_union_name (struct_out, init_info.data_size, data_align,
					  mem_usage);


      /* Declare union of the returned type to get desired 
       * characteristics.
       */
      if (data_global)
	{
	  sprintf (decl_buf, "union %s %s", union_name, data_label);
	}
      else
	{
	  sprintf (decl_buf, "static union %s %s", union_name, data_label);
	}

    }

#ifdef IT64BIT
  if (data_align >= 16)
    {
      /* Special alignment */
      fprintf (out, "%s __attribute__ ((aligned (%d)));\n",
	       decl_buf, data_align);
    }
  else
#endif
    {
      /* Normal GCC supported alignment */
      /* Print out declaration */
      fprintf (out, "%s;\n", decl_buf);
    }

  /* If globally visible, add extern to header file. */
  if (data_global && extern_out)
    {
      fprintf (extern_out, "extern %s;\n", decl_buf);
    }


  /* Peek ahead at data stream to see if have initialization data. */
  token_type = C_peek_input (jump_tbl_data);

  /* Some versions of GCC have a bug that causes -O compile time of
     long initializations to blow up.  We will keep these functions
     small so that this does not happen */
  num_inits = 0;
  size_of_init = 0;
  init_info.tot_inits = 0;

  /* If present, generate appropriate C code to initialize structure */
  /* Print out intialization data, as long as matches initial
   * initializer token found.
   */
  while (C_is_init_data (C_peek_input (jump_tbl_data)))
    {
      /* Check to see if we are at the beginning of an initialization function */ 
      if (size_of_init == 0)
	{
	  /* Create init_routine_name from unique id, num of inits and variable name */
	  sprintf (init_routine_name, "init_%i_%i_%s", C_unique_id, num_inits, data_label);
	  
	  /* Add to table of initialization routines create */
	  STRING_add_symbol (mem_usage->init_routines_created,
			     init_routine_name, NULL);
	  
	  /* Print out C function wrapper for initialization */
	  fprintf (out, "void %s%s()\n" "{\n", C_prefix, init_routine_name);
	}
	  
      /* Get this initializer (instead of just peeking at it) */
      token_type = C_get_input (jump_tbl_data, mem_usage);
      
      /* Get initializer data */
      init_data = C_data;
      
      /* Determine pointer type needed in cast */
      switch (token_type)
	{
	case L_INPUT_WB:
	  cast_type = "char *";
	  init_info.tot_inits += sizeof (char);
	  break;
	  
	case L_INPUT_WW:
	  cast_type = "short *";
	  init_info.tot_inits += sizeof (short);
	  break;
	  
	case L_INPUT_WI:
	  cast_type = "int *";
	  init_info.tot_inits += sizeof (int);
	  break;
	  
	case L_INPUT_WQ:
	  cast_type = "longlong *";
	  init_info.tot_inits += sizeof (long long);
	  break;
	  
	case L_INPUT_WF:
	  cast_type = "float *";
	  init_info.tot_inits += sizeof (float);
	  break;
	  
	case L_INPUT_WF2:
	  cast_type = "double *";
	  init_info.tot_inits += sizeof (double);
	  break;
	  
	case L_INPUT_WS:
	  cast_type = NULL;	/* Need special handling of strings */
	  break;
	  
	default:
	  L_punt ("C_emit_reserve_data: Unexpected init token (%i)!",
		  token_type);
	}
      
      /* Handle everything but strings thru assignment */
      if (token_type != L_INPUT_WS)
	{
	  /* Print out dereference and cast to appropriate type */
	  fprintf (out, "%s*((%s)", C_indent, cast_type);
	  
	  /* Print out address expression */
	  C_emit_expr (out, init_data->address);
	  
	  /* Finish dereference and add = */
	  fprintf (out, ") = ");
	  
	  /* Print out initializiation */
	  C_emit_expr (out, init_data->value);
	  
	  /* Finish statement */
	  fprintf (out, ";\n");
	}
      /* For strings, use strcpy to intialize section of memory */
      else
	{
	  /* Print out strcpy and cast to char * */
	  fprintf (out, "%sstrcpy((char *)", C_indent);
	  
	  /* Print out address expression */
	  C_emit_expr (out, init_data->address);
	  
	  /* Print out comma and cast of data to char * */
	  fprintf (out, ", (char *)");
	  
	  /* Print out initializiation string */
	  C_emit_expr (out, init_data->value);
	  
	  /* Finish statement */
	  fprintf (out, ");\n");
	}

      /* Free the data we just read in (if jump_tbl_data == NULL).
       * Otherwise, this data (from a jump table) will be freed when 
       * the function is freed.
       */
      if (jump_tbl_data == NULL)
	{
	  L_delete_data (init_data);
	}
      if (size_of_init >= 128)
	{
	  /* 128 was chosen arbitrarily to keep the size
	     reasonably small.  We are now done with this
	     initilization function and must start the next */

	  /* Print out closing "}" */
	  fprintf (out, "}\n"); 

	  num_inits++;
	  size_of_init = 0;	
	}
      else
	{	  
	  if (!(C_is_init_data (C_peek_input (jump_tbl_data))))
	    {
	      /* Print out closing "}" */
	      fprintf (out, "}\n"); 
	    }
	  size_of_init++;
	}
    }
  
  return init_info;
}

/* Returns a pointer to the true C name of the passed data label.  
 * Do not free the returned pointer!
 * Punts if not a data label!
 */
char *
C_get_data_label (L_Expr * expr)
{
  char *label;

  /* Make sure truely a label! */
  if (expr->type != L_EXPR_LABEL)
    {
      L_punt ("C_get_data_label: expr type (%i) not L_EXPR_LABEL!",
	      expr->type);
    }

  /* Get true C name */
  label = C_true_name (expr->value.l);

  /* Return true C name */
  return (label);
}

/* Returns a pointer to the raw name of the passed data label.  
 * Do not free the returned pointer!
 * Punts if not a data label!
 */
char *
C_get_raw_data_label (L_Expr * expr)
{
  /* Make sure truely a label! */
  if (expr->type != L_EXPR_LABEL)
    L_punt ("C_get_data_label: expr type (%i) not L_EXPR_LABEL!", expr->type);

  /* Return raw label name */
  return (expr->value.l);
}

/* Returns a pointer to the passed data string.  
 * Do not free the returned pointer!
 * Punts if not a data string!
 */
char *
C_get_data_string (L_Expr * expr)
{
  /* Make sure truely a string! */
  if (expr->type != L_EXPR_STRING)
    L_punt ("C_get_data_string: expr type (%i) not L_EXPR_STRING!",
	    expr->type);

  /* Return string */
  return (expr->value.s);
}

/* Returns the int value of the passed expr.
 * Punts if not an int!
 */
int
C_get_data_int (L_Expr * expr)
{
  /* Make sure truely an int! */
  if (expr->type != L_EXPR_INT)
    L_punt ("C_get_data_string: expr type (%i) not L_EXPR_INT!", expr->type);

  /* Return int value */
  return ITicast (expr->value.i);
}

/* Emits the C code for a Lcode Data expression, uses recursion
 * to handle math.
 */
void
C_emit_expr (FILE * out, L_Expr * expr)
{
  /* Sanity check */
  if (expr == NULL)
    {
      L_punt ("C_emit_expr: Error NULL expression!");
    }

  /* Handle each expression type appropriately */
  switch (expr->type)
    {
    case L_EXPR_INT:
#if defined(X86LIN_SOURCE) || defined(_HPUX_SOURCE)
      /* RDB/MCM  HP's cc and x86lin gcc have problems with LL constants */
      if (expr->value.i <= ITMAXS32)
	fprintf(out, "(" "%d" "L" ")", (int)expr->value.i);
      else
	fprintf (out, "(" ITintmaxformat ITintmaxsuffix ")", expr->value.i);
#else
      fprintf (out, "(" ITintmaxformat ITintmaxsuffix ")", expr->value.i);
#endif
      break;

    case L_EXPR_FLOAT:
      fprintf (out, "((float)%.8e)", expr->value.f);
      break;

    case L_EXPR_DOUBLE:
      fprintf (out, "((double)%.16e)", expr->value.f2);
      break;

    case L_EXPR_LABEL:
      /* If cb label in hash table, add C_prefix to name and
       * don't add &, since will be #defined to a constant.
       */
      if ((expr->type == L_EXPR_LABEL) && C_is_cb_label (expr->value.l))
	{
	  fprintf (out, "((%s)%s%s)", C_native_machine_ctype_str,
		   C_prefix, expr->value.l);
	}
      /* Otherwise, get address of true name */
      else
	{
	  fprintf (out, "((%s)&%s)", C_native_machine_ctype_str,
		   C_get_data_label (expr));
	}
      break;

    case L_EXPR_STRING:
      fprintf (out, "((%s)%s)", C_native_machine_ctype_str, expr->value.s);
      break;

      /* A + B (int oper) */
    case L_EXPR_ADD:
      fprintf (out, "(");
      C_emit_expr (out, expr->A);
      fprintf (out, " + ");
      C_emit_expr (out, expr->B);
      fprintf (out, ")");
      break;

      /* A - B (int oper) */
    case L_EXPR_SUB:
      fprintf (out, "(");
      C_emit_expr (out, expr->A);
      fprintf (out, " - ");
      C_emit_expr (out, expr->B);
      fprintf (out, ")");
      break;

      /* A * B (int oper) */
    case L_EXPR_MUL:
      fprintf (out, "(");
      C_emit_expr (out, expr->A);
      fprintf (out, " * ");
      C_emit_expr (out, expr->B);
      fprintf (out, ")");
      break;

      /* A / B (int oper) */
    case L_EXPR_DIV:
      fprintf (out, "(");
      C_emit_expr (out, expr->A);
      fprintf (out, " / ");
      C_emit_expr (out, expr->B);
      fprintf (out, ")");
      break;

      /* - A (int oper) */
    case L_EXPR_NEG:
      fprintf (out, "(-");
      C_emit_expr (out, expr->A);
      fprintf (out, ")");
      break;

      /* ~ A (int oper) */
    case L_EXPR_COM:
      fprintf (out, "(~");
      C_emit_expr (out, expr->A);
      fprintf (out, ")");
      break;

    default:
      L_punt ("C_emit_expr: Unpexpected expr type %i!", expr->type);
      break;
    }
}

/* Emits declaration (and optional initialization) for Lcode base
 * types.  If declaration information is present (and being used),
 * this routine will use that information.  Otherwise, it will
 * use the default_type_name to declare the variable.
 */
void
C_emit_base_type (FILE * out, FILE * extern_out, L_Data * data,
		  Mem_Usage * mem_usage, char *default_type_name)
{
  char decl_buf[TYPE_BUF_SIZE];
  char *actual_decl, *actual_cast;

  int size;
  int initialized = FALSE;

  /* Get actual declaration and cast, if info present */
  actual_decl = C_get_label_decl (mem_usage,
				  C_get_data_label (data->address));
  actual_cast = C_get_label_cast (mem_usage,
				  C_get_data_label (data->address));

  /* If have actual declaration, use it */
  if (actual_decl != NULL)
    {
      strcpy (decl_buf, actual_decl);
    }
  /* Otherwise, create a valid (but possibly inexact) declaration
   * for this variable (i.e. int i instead of char *i).
   */
  else
    {
      /* If static, add 'static' to decl */
      if (!C_is_label_global (mem_usage, C_get_data_label (data->address)))
	{
	  sprintf (decl_buf, "static %s %s",
		   default_type_name, C_get_data_label (data->address));
	}
      /* Otherwise, don't add anything */
      else
	{
	  sprintf (decl_buf, "%s %s",
		   default_type_name, C_get_data_label (data->address));
	}
    }

  /* If static, record that we have locally defined this data label */
  if (!C_is_label_global (mem_usage, C_get_data_label (data->address)))
    {
      C_add_mem_usage (mem_usage->file_scope_data_labels_defined,
		       C_get_data_label (data->address));
    }
  /* Otherwise, emit external definition */
  else
    {
      /* Emit external definition */
      if (extern_out)
	fprintf (extern_out, "extern %s;\n", decl_buf);

      /* Record that we have globally defined this data label */
      C_add_mem_usage (mem_usage->program_scope_data_labels_defined,
		       C_get_data_label (data->address));
    }

  /* Emit the declaration buffer */
  fprintf (out, "%s", decl_buf);

  /* If have initialization value, emit that */
  if (data->value != NULL)
    {
      initialized = TRUE;

      fprintf (out, " = ");

      /* If have actual cast info, wrap around initialization value */
      if (actual_cast != NULL)
	{
	  fprintf (out, "((%s)", actual_cast);
	}

      C_emit_expr (out, data->value);

      /* Finish cast, if present */
      if (actual_cast != NULL)
	{
	  fprintf (out, ")");
	}
    }

  /* Terminate declaration */
  fprintf (out, ";\n");

  /* SER 20040509: Addition for HCH MICRO 04 */
  if (C_insert_probes && C_trace_mem_addrs && C_custom_profiling &&
      mem_usage->object_id != 0 && mem_usage->object_id != -1)
    {
      char trace_routine_name[TYPE_BUF_SIZE];
      if (!mem_usage->data_label_name)
	L_punt ("C_emit_data / trace_objects: trying to trace "
		"an unnamed object");
      sprintf (trace_routine_name, "trace_glob_%i_%s", ++C_unique_id,
	       mem_usage->data_label_name);
      /* Add to table of initialization routines create */
      STRING_add_symbol (mem_usage->init_routines_created,
			 trace_routine_name, NULL);
      fprintf (out, "void %s%s()\n" "{\n", C_prefix, trace_routine_name);
      fprintf (out, "   _EM_put_trace(L_TRACE_ASYNCH);\n");
      fprintf (out, "   _EM_put_trace3(L_TRACE_OBJ_GLOB,%d,&%s);\n",
	       mem_usage->object_id, mem_usage->data_label_name);
      mem_usage->object_id = -10;
      if (actual_cast == NULL || strstr (actual_cast, "*"))
	size = sizeof (void *);
      else if (!strcmp (actual_cast, "char "))
	size = sizeof (char);
      else if (!strcmp (actual_cast, "unsigned char "))
	size = sizeof (unsigned char);
      else if (!strcmp (actual_cast, "short "))
	size = sizeof (short);
      else if (!strcmp (actual_cast, "unsigned short "))
	size = sizeof (unsigned short);
      else if (!strcmp (actual_cast, "int "))
	size = sizeof (int);
      else if (!strcmp (actual_cast, "unsigned int "))
	size = sizeof (unsigned int);
      else if (!strcmp (actual_cast, "long "))
	size = sizeof (long);
      else if (!strcmp (actual_cast, "unsigned long "))
	size = sizeof (unsigned long);
      else if (!strcmp (actual_cast, "long long"))
	size = sizeof (long long);
      else if (!strcmp (actual_cast, "float "))
	size = sizeof (float);
      else if (!strcmp (actual_cast, "double "))
	size = sizeof (double);
      else 
	{
	  L_punt("C_emit_base_type/trace objects: unaccounted cast type %s", actual_cast);
	  return;
	}
      fprintf (out, "   _EM_put_trace2(%d, %d);\n}\n", size, size);
    }
  /* End of addition. */

  /* Emit global objects for items without a reserve in Lcode */
  if (C_trace_objects)
    {
      char trace_routine_name[TYPE_BUF_SIZE];
      
      if (!mem_usage->data_label_name)
	L_punt ("C_emit_base_type / trace_objects: trying to trace "
		"an unnamed object");
      
      sprintf (trace_routine_name, "trace_glob_%i_%s", ++C_unique_id,
	       mem_usage->data_label_name);

      /* Add to table of initialization routines create */
      STRING_add_symbol (mem_usage->init_routines_created,
			 trace_routine_name, NULL);

      if (strstr (actual_cast, "*"))
	size = sizeof (void *);
      else if (!strcmp (actual_cast, "char "))
	size = sizeof (char);
      else if (!strcmp (actual_cast, "unsigned char "))
	size = sizeof (unsigned char);
      else if (!strcmp (actual_cast, "short "))
	size = sizeof (short);
      else if (!strcmp (actual_cast, "unsigned short "))
	size = sizeof (unsigned short);
      else if (!strcmp (actual_cast, "int "))
	size = sizeof (int);
      else if (!strcmp (actual_cast, "unsigned int "))
	size = sizeof (unsigned int);
      else if (!strcmp (actual_cast, "long "))
	size = sizeof (long);
      else if (!strcmp (actual_cast, "unsigned long "))
	size = sizeof (unsigned long);
      else if (!strcmp (actual_cast, "long long"))
	size = sizeof (long long);
      else if (!strcmp (actual_cast, "float "))
	size = sizeof (float);
      else if (!strcmp (actual_cast, "double "))
	size = sizeof (double);
      else 
	{
	  L_punt("C_emit_base_type/trace objects: unaccounted cast type %s", actual_cast);
	  return;
	}

      /* Print out C function wrapper for initialization */
      fprintf (out, "void %s%s()\n" "{\n", C_prefix, trace_routine_name);
      fprintf (out, "   _EM_put_trace(L_TRACE_ASYNCH);\n");
      fprintf (out, "   _EM_put_trace3(L_TRACE_OBJ_GLOB,"
	       "&%s,%d);\n", mem_usage->data_label_name,size);
      fprintf (out, "   _EM_put_trace(%i);\n", initialized);
      fprintf (out, "}\n");      
    }  
}

/* Sanity check.  Punts if data->value != NULL with error message.  */
void
C_verify_no_data_value (L_Data * data)
{
  if (data->value != NULL)
    {
      fprintf (stderr, "\n" "Data: type:%i  N:%i\n", data->type, data->N);
      if (data->address != NULL)
	{
	  fprintf (stderr, "Address Expr: '");
	  C_emit_expr (stderr, data->address);
	  fprintf (stderr, "'\n");
	}
      if (data->value != NULL)
	{
	  fprintf (stderr, "Value Expr: '");
	  C_emit_expr (stderr, data->value);
	  fprintf (stderr, "'\n");
	}
      L_punt ("C_verify_no_data_value: Unexpected data value present!");
    }
}
