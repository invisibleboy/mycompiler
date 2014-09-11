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
/*
 *
 *  File:  lgp_rel_main.c
 *
 *  Description:
 *    Scan global data and create table of attributes
 * 
 *  Author: Dan Connors
 *
 *  Total rewrite: John Sias -- get rid of file-splitting 20020228
 */

#include <config.h>
#include <Lcode/l_main.h>
#include <library/string_symbol.h>

#define LG_SHORT            0
#define LG_LONG             1
#define LG_MAX_SHORT_SPACE  (4 * 1024 * 1024)


/* Global Pointer Operand Table */

STRING_Symbol_Table *LG_symbol_table = NULL;
STRING_Symbol_Table *LG_function_table = NULL;
ITuintmax LG_short_allocated;
ITuintmax LG_long_allocated;


void
LG_init ()
{
  LG_symbol_table = STRING_new_symbol_table ("LG_symbol_table",
					     8192);
  LG_function_table = STRING_new_symbol_table ("LG_function_table",
					       8192);
}


static int
LG_string_length (char *s)
{
  int len = 0;
  for (; *s != '\0'; s++)
    {
      len++;
      if (*s == '\\')
	{
	  if (('\0' != *(s + 1)) && ('0' <= *(s + 1)) && (*(s + 1) <= '9') &&
	      ('\0' != *(s + 2)) && ('0' <= *(s + 2)) && (*(s + 2) <= '9') &&
	      ('\0' != *(s + 3)) && ('0' <= *(s + 3)) && (*(s + 3) <= '9'))
	    {
	      s += 3;
	    }
	  else
	    s++;
	}
    }
  return len - 1;		/* - two quotes + \0 */
}


static int
LG_data_size (int type, L_Data *data)
{
  int size = data->N;
  switch (type)
    {
    case L_INPUT_BYTE:
      size *= 1;
      break;

    case L_INPUT_WORD:
      size *= 4;
      break;

    case L_INPUT_FLOAT:
      size *= 4;
      break;

    case L_INPUT_LONG:
      size *= M_type_size (M_TYPE_LONG) / 8;
      break;

    case L_INPUT_LONGLONG:
      size *= 8;
      break;

    case L_INPUT_DOUBLE:
      size *= 8;
      break;

    case L_INPUT_RESERVE:
      break;

    default:
      fprintf (stderr, "LG_data_size: unknown gp rel size\n");
      break;
    }
  return size;
}


static void
LG_register_function (char *label)
{
  if (STRING_find_symbol (LG_function_table, label))
    return;

  STRING_add_symbol (LG_function_table, label, (void *)(long) 1);
  return;
}


static void
LG_register_label (int type, L_Data * data, char *label, int section_type)
{
  int size, loc = 0;

  if (STRING_find_symbol (LG_symbol_table, label))
    return;

  if (data->value && (data->value->type == L_EXPR_STRING))
    size = LG_string_length (data->value->value.s);
  else
    size = LG_data_size (type, data);

  switch (section_type)
    {
    case L_MS_DATA:
    case L_MS_DATA1:
    case L_MS_RODATA:
    case L_MS_BSS:
      loc = LG_LONG;
      LG_long_allocated += size;
      LG_short_allocated += 8;
      break;

    case L_MS_SDATA:
    case L_MS_SDATA1:
    case L_MS_SBSS:
      loc = LG_SHORT;
      LG_short_allocated += size;
      break;

    default:
      L_punt ("LG_register_label: Unhandled section type %d", section_type);
    }

  STRING_add_symbol (LG_symbol_table, label, (void *)(long) loc);
  return;
}


static void
LG_scan_data (L_Data *data, int section_type)
{
  L_Expr *addr;
  char *label_name;

  if (!(addr = data->address))
    L_punt ("LG_scan_data: bad address", L_ERR_INTERNAL);
  
  if (L_token_type == L_INPUT_GLOBAL)
    {
      L_delete_data (L_data);
      L_get_input ();
      data = L_data;

      switch (L_token_type)
	{
	case L_INPUT_LONG:
	case L_INPUT_LONGLONG:
	case L_INPUT_BYTE:
	case L_INPUT_WORD:
	case L_INPUT_FLOAT:
	case L_INPUT_DOUBLE:

	  if (!(addr = data->address))
	    L_punt ("LG_scan_data: bad address", L_ERR_INTERNAL);
	  LG_register_label (L_token_type, data, data->address->value.l, section_type);
	  break;

	case L_INPUT_ALIGN:
	  label_name = strdup (data->address->value.l);

	  L_delete_data (L_data);
	  L_get_input ();

	  if (L_data->type == L_INPUT_ELEMENT_SIZE)
	    {
	      L_delete_data (L_data);
	      L_get_input ();
	    }
      
	  if (L_data->type != L_INPUT_RESERVE)
	    L_punt ("P_process_declaration: Bad data declaration");
      
	  LG_register_label (L_token_type, L_data, label_name, section_type);
	  free (label_name);
	  break;

	default:
	  L_punt ("LG_scan_data: Unexpected input token %d", L_token_type);
	  break;
	}
    }
  else if (L_token_type == L_INPUT_RESERVE)
    {
      L_delete_data (L_data);
      L_get_input ();
      data = L_data;
      switch (L_token_type)
	{
	case L_INPUT_WB:
	case L_INPUT_WW:
	case L_INPUT_WI:
	case L_INPUT_WQ:
	case L_INPUT_WF:
	case L_INPUT_WF2:
	case L_INPUT_WS:
	  break;

	default:
	  L_punt ("LG_scan_data: Unexpected input token");
	}
    }
  else
    {
      L_punt ("LG_scan_data: illegal data type %d", data->type,
	      L_ERR_INTERNAL);
    }
  return;
}


static void
LG_annotate_gp_rel_opers (L_Func * fn)
{
  L_Cb *cb;

  for (cb = fn->first_cb; cb; cb = cb->next_cb)
    {
      L_Oper *oper;
      for (oper = cb->first_op; oper; oper = oper->next_op)
	{
	  int i;
	  for (i = 0; i < L_max_src_operand; i++)
	    {
	      if (L_is_label (oper->src[i]))
		{
		  STRING_Symbol *sym;
		  if ((sym = STRING_find_symbol (LG_symbol_table,
						 oper->src[i]->value.l)))
		    {
		      switch ((int)(long) sym->data)
			{
			case LG_SHORT:
			  L_assign_type_LOCAL_GP_label (oper->src[i]);
			  break;
			case LG_LONG:
			  L_assign_type_GLOBAL_GP_label (oper->src[i]);
			  break;
			default:
			  L_punt ("Lgp_rel: bad classification %s --> %d",
				  sym->name, (int)(long)sym->data);
			}
		    }
		  else
		    {
		      /* Must be from a different load module */
		      L_assign_type_label (oper->src[i]);
		    }
		}
	      else if (L_is_string (oper->src[i]))
		{
		  L_punt ("Lgp_rel: String operands are deprecated.");
		}
	    }

	  if (L_subroutine_call_opcode (oper) &&
	      L_is_label (oper->src[0]) &&
	      STRING_find_symbol (LG_function_table,
				  oper->src[0]->value.l + 4))
	    {
	      L_Attr *attr = L_new_attr ("LINKLOCAL", 0);
	      oper->attr = L_concat_attr (oper->attr, attr);
	    }
	}
    }

  return;
}


static void
LG_phase_one_process (void)
{
  static int LG_current_section = 0;

  switch (L_token_type)
    {
    case L_INPUT_MS:
      LG_current_section = L_data->N;
      L_delete_data (L_data);
      break;

    case L_INPUT_GLOBAL:
    case L_INPUT_RESERVE:
      if (LG_current_section != L_MS_TEXT)
	LG_scan_data (L_data, LG_current_section);
      L_delete_data (L_data);
      break;

    case L_INPUT_EOF:
    case L_INPUT_VOID:
    case L_INPUT_BYTE:
    case L_INPUT_WORD:
    case L_INPUT_LONG:
    case L_INPUT_LONGLONG:
    case L_INPUT_FLOAT:
    case L_INPUT_DOUBLE:
    case L_INPUT_ALIGN:
    case L_INPUT_ASCII:
    case L_INPUT_ASCIZ:
    case L_INPUT_ELEMENT_SIZE:
    case L_INPUT_WB:
    case L_INPUT_WW:
    case L_INPUT_WI:
    case L_INPUT_WQ:
    case L_INPUT_WF:
    case L_INPUT_WF2:
    case L_INPUT_WS:
    case L_INPUT_DEF_STRUCT:
    case L_INPUT_DEF_UNION:
    case L_INPUT_DEF_ENUM:
    case L_INPUT_FIELD:
    case L_INPUT_ENUMERATOR:
    case L_INPUT_EVENT_LIST:
    case L_INPUT_RESULT_LIST:
      L_delete_data (L_data);
      break;

    case L_INPUT_FUNCTION:
      LG_register_function (L_fn->name);
      L_delete_func (L_fn);
      break;
    default:
      L_punt ("LG_phase_one_process: illegal token");
    }
}

static void
LG_phase_two_process (void)
{
  switch (L_token_type)
    {
    case L_INPUT_MS:

      L_print_data (L_OUT, L_data);
      L_delete_data (L_data);
      break;

    case L_INPUT_GLOBAL:
    case L_INPUT_RESERVE:
    case L_INPUT_EOF:
    case L_INPUT_VOID:
    case L_INPUT_BYTE:
    case L_INPUT_WORD:
    case L_INPUT_LONG:
    case L_INPUT_LONGLONG:
    case L_INPUT_FLOAT:
    case L_INPUT_DOUBLE:
    case L_INPUT_ALIGN:
    case L_INPUT_ASCII:
    case L_INPUT_ASCIZ:
    case L_INPUT_ELEMENT_SIZE:
    case L_INPUT_WB:
    case L_INPUT_WW:
    case L_INPUT_WI:
    case L_INPUT_WQ:
    case L_INPUT_WF:
    case L_INPUT_WF2:
    case L_INPUT_WS:
    case L_INPUT_DEF_STRUCT:
    case L_INPUT_DEF_UNION:
    case L_INPUT_DEF_ENUM:
    case L_INPUT_FIELD:
    case L_INPUT_ENUMERATOR:
    case L_INPUT_EVENT_LIST:
    case L_INPUT_RESULT_LIST:
      L_print_data (L_OUT, L_data);
      L_delete_data (L_data);
      break;

    case L_INPUT_FUNCTION:
      LG_annotate_gp_rel_opers (L_fn);
      L_print_func (L_OUT, L_fn);
      L_delete_func (L_fn);
      break;

    default:
      L_punt ("process_input: illegal token");
    }
}


void
LG_deinit ()
{
  STRING_delete_symbol_table (LG_symbol_table, NULL);
  STRING_delete_symbol_table (LG_function_table, NULL);
  LG_symbol_table = NULL;
  LG_function_table = NULL;
}


void
L_gen_code (Parm_Macro_List * command_line_macro_list)
{
  if ((L_file_processing_model != L_FILE_MODEL_LIST) &&
      (L_file_processing_model != L_FILE_MODEL_EXTENSION))
    L_punt ("Lgp_rel: Use file list or extension");


  /* Initialize */

  LG_init ();
  L_create_filelist ();

  /* Phase 1: read data file, if not phase2, make quick gp file */

  List_start (L_filelist);
  while ((L_file = List_next (L_filelist)))
    {
      L_input_file = L_file->input_file;
      L_open_input_file (L_input_file);

      while (L_get_input () != L_INPUT_EOF)
	LG_phase_one_process ();

      L_close_input_file (L_input_file);
    }

  if (LG_short_allocated > LG_MAX_SHORT_SPACE)
    L_punt ("Lgp_rel: short data too big!");

  /* Phase 2: process ops of all functions */
  
  List_start (L_filelist);
  while ((L_file = List_next (L_filelist)))
    {
      L_input_file = L_file->input_file;
      L_open_input_file (L_input_file);
      L_output_file = L_file->output_file;
      L_OUT = L_open_output_file (L_output_file);	
      L_generation_info_printed = 0;

      while (L_get_input () != L_INPUT_EOF)
	LG_phase_two_process ();

      L_close_input_file (L_input_file);	  
      L_close_output_file (L_OUT);
    }

  L_delete_filelist ();
  LG_deinit ();

  return;
}
