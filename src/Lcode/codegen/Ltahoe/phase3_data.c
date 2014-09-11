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
 * phase3_data.c                                                             *
 * ------------------------------------------------------------------------- *
 * Assembly generation for data structures                                   *
 *                                                                           *
 * AUTHORS: D.A. Connors, J.W. Sias                                          *
 *****************************************************************************/
/* 08/21/02 REK Fixing a bug in P_process_data where the section header for
 *              the first section is not printed.
 */

/* 10/29/02 REK Adding config.h */
#include <config.h>
#include "ltahoe_main.h"

#include <ctype.h>
#include "phase3.h"
#include "phase1_func.h"

typedef struct _P_Init_Data
{
  int type;			/* Type of data */
  enum
  { CLASS_NONE, CLASS_INT, CLASS_STRING_POINTER,
    CLASS_LABEL_POINTER, CLASS_FUNCTION_POINTER,
    CLASS_LABEL_POINTER_PLUS_OFFSET
  }
  class;
  union
  {
    char *string;		/* string or label  */
    ITintmax i;			/* integer constant */
    float f;			/* float constant   */
    double f2;			/* double constant  */
  }
  value;
  int offset;
}
P_Init_Data;

typedef struct _P_Data_Section
{
  int type;
  int global;
  int align;
  char *label;
  char *string;
  int element_size;
  int reserve;
  List init_data;		/* P_Init_Data * */
}
P_Data_Section;


static P_Init_Data *
P_create_initial_data (int type)
{
  P_Init_Data *data;

  if ((data = (P_Init_Data *) malloc (sizeof (P_Init_Data))) == NULL)
    L_punt ("P_create_initial_data: Not enough space");

  data->value.string = NULL;
  data->offset = 0;
  data->type = (char) type;
  return data;
}


static void
P_init_data_section (P_Data_Section * section, char *label,
		     int type, int global)
{
  section->type = type;
  section->global = global;
  section->align = 0;
  section->element_size = 0;
  section->reserve = 0;
  section->init_data = NULL;
  section->label = strdup (label + 1);	/* remove beginning underscore */
  return;
}


static void
P_clean_data_section (P_Data_Section * section)
{
  P_Init_Data *pi;

  section->type = 0;
  section->align = 0;
  section->element_size = 0;
  section->reserve = 0;

  if (section->label)
    free (section->label);
  section->label = NULL;

  List_start (section->init_data);
  while ((pi = (P_Init_Data *) List_first (section->init_data)))
    {
      if ((pi->type == L_INPUT_WS) ||
	  ((pi->type == L_INPUT_WQ) && (pi->class != CLASS_INT) &&
	   pi->value.string))
	free (pi->value.string);
      free (pi);
      section->init_data = List_delete_current (section->init_data);
    }

  return;
}


static int
P_string_length (char *s)
{
  /* ias string escape characters
   * ------------------------------------------------------------
   * \ddd, d an octal digit
   * \Xdd. d a hex digit
   * \' \" \b \t \n \f \r \\
   */
  int len = 0;
  for (; *s != '\0'; s++)
    {
      len++;
      if (*s == '\\')
	{
	  if (*(s + 1) && isdigit (*(s + 1)) &&
	      *(s + 2) && isdigit (*(s + 2)) &&
	      *(s + 3) && isdigit (*(s + 3)))
	    s += 3;
	  else if (*(s + 1) && (*(s + 1) == 'X') &&
		   *(s + 2) && isxdigit (*(s + 2)) &&
		   *(s + 3) && isxdigit (*(s + 3)))
	    s += 3;
	  else
	    s++;
	}
    }
  return len - 1;		/* - two quotes + \0 */
}


static int
P_process_expr (L_Expr * expr)
{
  int val = 0;

  if (expr == NULL)
    L_punt ("P_process_expr failed: nil expr or outfile");

  switch (expr->type)
    {
    case L_EXPR_INT:
      val = expr->value.i;
      break;

    case L_EXPR_LABEL:
      if (!expr->value.l)
	L_punt ("P_process_expr: bad label");
      val = 0;
      break;

    case L_EXPR_ADD:
      val = P_process_expr (expr->B);
      break;

    case L_EXPR_FLOAT:
    case L_EXPR_DOUBLE:
    case L_EXPR_STRING:
    case L_EXPR_SUB:
    case L_EXPR_MUL:
    case L_EXPR_DIV:
    case L_EXPR_NEG:
    case L_EXPR_COM:
    default:
      L_punt ("P_process_expr failed: illegal type");
    }
  return val;
}

/*
 * Add a label to an init_data structure, and enter the
 * label into the global symtab.
 */
static void
P_add_label (P_Init_Data *init_data, L_Expr *value)
{
  if (!value->value.l)
    L_punt ("P_add_label: bad label");

  if (init_data->value.string)
    L_punt ("P_add_label: already allocated label");

  /* Determine if label is a function label */

  if (!strncmp (value->value.l, "_$fn", 4))
    {
      /* Strip off the front five chars */
      init_data->value.string =
	strdup (value->value.l + 5);
      init_data->class = CLASS_FUNCTION_POINTER;
      P_symtab_add_label (init_data->value.string, 1);
    }
  else
    {
      init_data->value.string =
	strdup (value->value.l + 1);
      init_data->class = CLASS_LABEL_POINTER;
      P_symtab_add_label (init_data->value.string, 0);
    }
  return;
}


static void
P_process_expr_add (P_Init_Data * init_data, L_Expr * expr)
{
  L_Expr *left, *right, *mul_left, *mul_right;
  int val;

  if (!expr)
    L_punt ("P_process_expr_add failed: NULL expr");

  /* Process left side */
  left = expr->A;

  switch (left->type)
    {
    case L_EXPR_ADD:
      P_process_expr_add (init_data, left);
      break;

    case L_EXPR_INT:
      val = left->value.i;
      init_data->offset += val;
      break;

    case L_EXPR_LABEL:
      P_add_label (init_data, left);
      break;

    case L_EXPR_MUL:
      mul_left = left->A;
      mul_right = left->B;

      if ((mul_left->type != L_EXPR_INT) || (mul_right->type != L_EXPR_INT))
	L_punt ("P_process_expr_add: ");

      init_data->offset = mul_left->value.i * mul_right->value.i;
      break;

    default:
      L_punt ("P_process_expr_add: unsupported expression type");
    }

  /* Process left side */
  right = expr->B;

  /* Process right side */
  switch (right->type)
    {
    case L_EXPR_ADD:
      P_process_expr_add (init_data, right);
      break;

    case L_EXPR_INT:
      val = right->value.i;
      init_data->offset += val;
      break;

    case L_EXPR_LABEL:
      P_add_label (init_data, right);
      break;

    case L_EXPR_MUL:
      mul_left = right->A;
      mul_right = right->B;

      if ((mul_left->type != L_EXPR_INT) || (mul_right->type != L_EXPR_INT))
	L_punt ("P_process_expr_add: ");

      init_data->offset = mul_left->value.i * mul_right->value.i;
      break;

    default:
      L_punt ("P_process_expr_add: unsupported expression type");
    }
  return;
}


/*----------------------------------------------------------------------*/


static int
P_next_data (void)
{
  L_delete_data (L_data);
  L_get_input ();
  return L_token_type;
}


static int
P_process_data_resv (P_Data_Section * section, int section_type)
{
  P_Init_Data *init_data = NULL;
  int length;

  int offset = 0;
  int last_data_size;
  int last_data_offset;
  int reserved = 0, allocated, difference;

  int token_type;

  token_type = P_next_data ();

  if (L_data->type == L_INPUT_ELEMENT_SIZE)
    {
      section->element_size = L_data->N;

      token_type = P_next_data ();

      if (L_data->type != L_INPUT_RESERVE)
	L_punt ("P_process_data_resv: Bad data declaration");

      reserved = L_data->N;

      /* Pad to a multiple of object alignment */

      reserved += reserved % section->align;
    }
  else if (L_data->type == L_INPUT_RESERVE)
    {
      reserved = L_data->N;
    }
  else
    {
      L_punt ("P_process_data_resv: Bad data declaration");
      return 0;
    }

  section->reserve = reserved;

  offset = last_data_size = last_data_offset = difference = allocated = 0;

  /* Now cycle through until reading a new data segment */
  /* Note: reserved data may be more than initialized   */

  while ((token_type = P_next_data ()) &&
	 (token_type >= L_INPUT_WB) && (token_type <= L_INPUT_WS))
    {
      /* Check to see if the next token is still part of this section */

      /* Process the expression for label offsets */
      offset = P_process_expr (L_data->address);

      difference = (offset - (last_data_offset + last_data_size));

      if (difference > 0)
	{
	  init_data = P_create_initial_data (L_INPUT_SKIP);
	  init_data->value.i = difference;
	  section->init_data = List_insert_last (section->init_data,
						 (void *) init_data);
	  allocated += difference;
	}
      else if (difference != 0)
	{
	  L_punt ("P_process_data_resv: problem");
	}

      init_data = NULL;

      switch (token_type)
	{
	case L_INPUT_WB:
	  init_data = P_create_initial_data (L_INPUT_WB);
	  init_data->value.i = L_data->value->value.i;
	  last_data_size = 1;
	  break;
	case L_INPUT_WW:
	  init_data = P_create_initial_data (L_INPUT_WW);
	  init_data->value.i = L_data->value->value.i;
	  last_data_size = 2;
	  break;
	case L_INPUT_WI:
	  init_data = P_create_initial_data (L_INPUT_WI);

	  if (L_data->value->type == L_EXPR_INT)
	    {
	      init_data->value.i = L_data->value->value.i;
	      init_data->class = CLASS_INT;
	    }
	  else if (L_data->value->type == L_EXPR_ADD)
	    {
	      /* Pointer could point to label plus offset */
	      P_process_expr_add (init_data, L_data->value);

	      /* Allocate string space */
	      init_data->class = CLASS_LABEL_POINTER_PLUS_OFFSET;
	    }
	  else
	    {
	      L_punt ("P_process_data_resv: WI type problem");
	    }
	  last_data_size = 4;
	  break;
	case L_INPUT_WQ:
	  init_data = P_create_initial_data (L_INPUT_WQ);

	  if (L_data->value->type == L_EXPR_INT)
	    {
	      init_data->value.i = L_data->value->value.i;
	      init_data->class = CLASS_INT;
	    }
	  else if (L_data->value->type == L_EXPR_STRING)
	    {
	      L_punt ("P_process_data_resv: string literals are deprecated");
	    }
	  else if (L_data->value->type == L_EXPR_LABEL)
	    {
	      /* Determine if label is a function label */
	      if (!strncmp (L_data->value->value.l, "_$fn", 4))
		{
		  /* Strip off the front five chars */

		  init_data->value.string =
		    strdup (L_data->value->value.l + 5);
		  init_data->class = CLASS_FUNCTION_POINTER;
		  P_symtab_add_label (init_data->value.string, 1);
		}
	      else
		{
		  init_data->value.string =
		    strdup (L_data->value->value.l + 1);
		  init_data->class = CLASS_LABEL_POINTER;
		  P_symtab_add_label (init_data->value.string, 0);
		}
	    }
	  else if (L_data->value->type == L_EXPR_ADD)
	    {
	      /* Pointer could point to label plus offset */
	      P_process_expr_add (init_data, L_data->value);

	      /* Allocate string space */
	      init_data->class = CLASS_LABEL_POINTER_PLUS_OFFSET;
	    }
	  else
	    {
	      L_punt ("P_process_data_resv: WQ type problem");
	    }
	  last_data_size = 8;
	  break;
	case L_INPUT_WF:
	  init_data = P_create_initial_data (L_INPUT_WF);
	  init_data->value.f = L_data->value->value.f;
	  last_data_size = 4;
	  break;
	case L_INPUT_WF2:
	  init_data = P_create_initial_data (L_INPUT_WF2);
	  init_data->value.f2 = L_data->value->value.f2;
	  last_data_size = 8;
	  break;
	case L_INPUT_WS:
	  init_data = P_create_initial_data (L_INPUT_WS);
	  init_data->value.string = strdup (L_data->value->value.s);
	  length = P_string_length (L_data->value->value.s);
	  last_data_size = length;
	  break;
	default:
	  return token_type;
	}

      /* Keep track of the space allocated */
      allocated += last_data_size;

      /* Partially keep track of the last item */
      last_data_offset = offset;

      /* Put the initial data into the section structure */
      /* Must be in the same order */

      if (init_data)
	section->init_data = List_insert_last (section->init_data,
					       (void *) init_data);
    }

  /* Need to check if the reserved space is the same size as created */
  /* Pad the end of the data section */

  /* If nothing was allocated, there is no need to add a skip -- a
   * "common" reservation directive will be generated
   */

  if (allocated)
    {
      difference = (reserved - allocated);

      if (difference > 0)
	{
	  init_data = P_create_initial_data (L_INPUT_SKIP);
	  init_data->value.i = difference;
	  section->init_data = List_insert_last (section->init_data,
						 (void *) init_data);
	}
      else if (difference < 0)
	{
	  L_punt ("P_process_data_resv: final padding problem");
	}
    }

  return token_type;
}


static int
P_bss_skippable_token_type (int token_type)
{
  int skip;

  switch (L_token_type)
    {
    case L_INPUT_EOF:
    case L_INPUT_MS:
    case L_INPUT_GLOBAL:
      skip = 0;
      break;
    case L_INPUT_DEF_STRUCT:
    case L_INPUT_DEF_UNION:
    case L_INPUT_DEF_ENUM:
    case L_INPUT_FIELD:
    case L_INPUT_ENUMERATOR:
      skip = 1;
      break;
    default:
      skip = -1;
      L_punt ("P_bss_skippable_token_type: unexpected input token, %d",
	      token_type);
    }
  return skip;
}


void
P_print_section_title (int section_type)
{
  fprintf (L_OUT, "\t.section .%s\n", L_ms_name (section_type));
  return;
}


static void
P_print_initialized_data (P_Data_Section * ptr)
{
  char *label;
  int reserve_size;
  int align;
  P_Init_Data *init_data;
  union CONVERT convert;

  label = ptr->label;

  /* May need to pad on word boundries */

  reserve_size = ptr->reserve;
  align = ptr->align;

  fprintf (L_OUT, "\t.align %d\n", align);
  /* 12/02/02 REK Write the global line in a form for ias or as. */
  if (!ptr->global)
    fprintf (L_OUT, "%s:\n", label);
  /* ias recognizes a label defined with two colons as a global.  GNU as
   * does not recognize the double colon syntax and requires a .global 
   * definition and a single colon. */
  else if (Ltahoe_output_for_ias)
    fprintf (L_OUT, "%s::\n", label);
  else
    fprintf (L_OUT, "\t.global %s#\n%s:\n", label, label);

  List_start (ptr->init_data);
  while ((init_data = (P_Init_Data *) List_next (ptr->init_data)))
    {
      switch (init_data->type)
	{
	case L_INPUT_WI:
	  if (init_data->class == CLASS_INT)
	    fprintf (L_OUT, "\tdata4.ua %lld\t// s32\n", init_data->value.i);
	  else
	    L_punt ("P_print_initialized_data: problem with WI");
	  break;

	case L_INPUT_WW:
	  fprintf (L_OUT, "\tdata2 %lld\t // i16\n", init_data->value.i);
	  break;

	case L_INPUT_WB:
	  fprintf (L_OUT, "\tdata1 %lld\t// i8\n", init_data->value.i);
	  break;

	case L_INPUT_WQ:
	  if (init_data->class == CLASS_INT)
	    fprintf (L_OUT, "\tdata8.ua %lld\t// i64\n", init_data->value.i);
	  else if (init_data->class == CLASS_STRING_POINTER)
	    fprintf (L_OUT, "\tdata8.ua __1STRINGPACKET_%lld#\t// p64\n",
		     init_data->value.i);
	  else if (init_data->class == CLASS_LABEL_POINTER)
	    fprintf (L_OUT, "\tdata8 %s#\t// p64\n", init_data->value.string);
	  else if (init_data->class == CLASS_FUNCTION_POINTER)
	    fprintf (L_OUT, "\tdata8.ua @fptr(%s#)\t// p64\n",
		     init_data->value.string);
	  else if (init_data->class == CLASS_LABEL_POINTER_PLUS_OFFSET)
	    fprintf (L_OUT, "\tdata8 %s# + %d\t// p64\n",
		     init_data->value.string, init_data->offset);
	  else
	    L_punt ("P_print_initialized_data: problem with WQ");
	  break;

	case L_INPUT_WF:
	  convert.sgl = init_data->value.f;
	  fprintf (L_OUT, "\tdata4 0x%08x\t//xf32\n",
#if defined(X86LIN_SOURCE) || defined(WIN32) || defined(IA64LIN_SOURCE)
		   convert.integer.lo
#elif defined(_SOLARIS_SOURCE) || defined(_HPUX_SOURCE)
		   convert.integer.hi
#else
#error Unsupported host platform
#endif
	    );
	  break;

	case L_INPUT_WF2:
	  convert.dbl = init_data->value.f2;
	  fprintf (L_OUT, "\tdata4 0x%08x,0x%08x\t//xf32\n",
		   convert.integer.lo, convert.integer.hi);
	  break;

	case L_INPUT_WS:
	  fprintf (L_OUT, "\tstringz %s\n", init_data->value.string);
	  break;

	case L_INPUT_SKIP:
	  fprintf (L_OUT, "\t.skip " ITintmaxformat "\n", init_data->value.i);
	  break;

	default:
	  L_punt ("P_print_initalized_data: illegal data section");
	}
    }
}


static void
P_print_reserve_data (P_Data_Section * ptr)
{
  fprintf (L_OUT, "\t.common\t%s#,%d,%d\n",
	   ptr->label, ptr->reserve, ptr->align);
  return;
}


/*
 * P_process_data_decl
 * ----------------------------------------------------------------------
 * Assumes an L_INPUT_MS has been read
 * Returns only after an ms label has been read, or after EOF.
 * Returns the type of the pending token.
 */

static int
P_process_data_decl (int section_type, int section_pend)
{
  L_Expr *data_addr;
  P_Data_Section section;
  int token_type = L_token_type;

  while ((token_type != L_INPUT_EOF) && (token_type != L_INPUT_MS))
    {
      if (L_data->type != L_INPUT_GLOBAL)
	{
	  if (section_type != L_MS_BSS)
	    L_punt ("P_process_data_decl: skippables permitted in BSS only");

	  do
	    token_type = P_next_data ();
	  while (P_bss_skippable_token_type (token_type));
	}
      else
	{
	  /* L_INPUT_GLOBAL: Create a labeled data object */

	  data_addr = L_data->address;

	  if (section_type != L_MS_TEXT)
	    {
	      P_symtab_add_def (data_addr->value.l + 1, 0);

	      P_init_data_section (&section, data_addr->value.l,
				   section_type, 1);

	      if (section_pend)
		{
		  /* A section change is pending, so we need to
		   * print the section header
		   */
		  P_print_section_title (section_type);
		  section_pend = 0;
		}

	      token_type = P_next_data ();
	      if (token_type == L_INPUT_EOF)
		L_punt ("P_process_data_decl: unexpected EOF");

	      if ((section_type == L_MS_BSS) && (L_data->value))
		L_punt ("P_process_data_decl: BSS is not initialized");

	      if (L_data->type == L_INPUT_ALIGN)
		{
		  section.align = L_data->N;

		  if (section.align & (section.align - 1))
		    L_punt ("Alignment value is not a power of 2");

		  token_type = P_process_data_resv (&section, section_type);
		}
	      else
		{
		  P_Init_Data *init_data = NULL;

		  switch (L_data->type)
		    {
		    case L_INPUT_LONG:
		    case L_INPUT_LONGLONG:
		      if (L_data->value)
			{
			  init_data = P_create_initial_data (L_INPUT_WQ);
			  if (L_data->value->type == L_EXPR_INT)
			    {
			      init_data->value.i = L_data->value->value.i;
			      init_data->class = CLASS_INT;
			    }
			  else if (L_data->value->type == L_EXPR_STRING)
			    {
			      L_punt ("P_process_data_decl: "
				      "string literals are deprecated");
			    }
			  else if (L_data->value->type == L_EXPR_LABEL)
			    {
			      P_add_label (init_data, L_data->value);
			    }
			  else if (L_data->value->type == L_EXPR_ADD)
			    {
			      /* Pointer could point to label plus offset */
			      P_process_expr_add (init_data, L_data->value);

			      /* Allocate string space */
			      init_data->class =
				CLASS_LABEL_POINTER_PLUS_OFFSET;
			    }
			  else
			    {
			      L_punt ("P_process_data_decl: "
				      "unknown data value type %d",
				      L_data->value->type);
			    }
			}

		      section.align = 8;
		      section.reserve = 8;
		      section.element_size = 8;
		      break;

		    case L_INPUT_WORD:
		      if (L_data->value)
			{
			  init_data = P_create_initial_data (L_INPUT_WI);
			  init_data->value.i = L_data->value->value.i;
			  init_data->class = CLASS_INT;
			  if (L_data->value->type == L_EXPR_STRING)
			    L_punt ("handle this case");
			}

		      section.align = 4;
		      section.reserve = 2;
		      section.element_size = 2;
		      break;

		    case L_INPUT_BYTE:
		      if (L_data->value)
			{
			  init_data = P_create_initial_data (L_INPUT_WB);
			  init_data->value.i = L_data->value->value.i;
			}

		      section.align = 4;
		      section.reserve = 1;
		      section.element_size = 1;
		      break;

		    case L_INPUT_FLOAT:
		      if (L_data->value)
			{
			  init_data = P_create_initial_data (L_INPUT_WF);
			  init_data->value.f = L_data->value->value.f;
			}

		      section.align = 4;
		      section.reserve = 4;
		      section.element_size = 4;
		      break;

		    case L_INPUT_DOUBLE:
		      if (L_data->value)
			{
			  init_data = P_create_initial_data (L_INPUT_WF2);
			  init_data->value.f2 = L_data->value->value.f2;
			}

		      section.align = 8;
		      section.reserve = 8;
		      section.element_size = 8;
		      break;

		    default:
		      L_punt ("P_process_data_decl: unexpected token, %d",
			      L_data->type);
		    }

		  if (init_data)
		    section.init_data = List_insert_last (section.init_data,
							  (void *) init_data);

		  token_type = P_next_data ();
		}

	      if (section.init_data)
		P_print_initialized_data (&section);
	      else
		P_print_reserve_data (&section);

	      P_clean_data_section (&section);
	    }
	  else
	    {
	      /* TEXT SECTION -- this token should be a global label
	       * immediately preceding a function -- stop! */
	      L_delete_data (L_data);
	      break;
	    }
	}
    }
  return token_type;
}


/*
 * P_process_data
 * ----------------------------------------------------------------------
 * Process a sequence of data section declarations.
 * Assumes L_INPUT_MS is in L_data
 */
/* 08/21/02 REK Fixing a bug there the section header is not printed for the
 *              first section processed.
 */

static int P_section_curr = -1, P_section_pend = 0;

void
P_clear_current_ms (void)
{
  P_section_curr = -1;
  return;
}

void
P_process_data (FILE * F_OUT, L_Data * data)
{
  int token_type = L_token_type;

  while (token_type != L_INPUT_EOF)
    {
      if (L_data->type == L_INPUT_MS)
	{
	  if (L_data->N != P_section_curr)
	    P_section_pend = 1;
	  P_section_curr = L_data->N;

	  token_type = P_next_data ();
	}
      else
	{
	  token_type = P_process_data_decl (P_section_curr, P_section_pend);

	  if (P_section_curr == L_MS_TEXT)
	    break;
	}
    }

  return;
}
