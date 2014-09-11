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
 *	File:	pcode.c
 *	Author: David August, Nancy Warter and Wen-mei Hwu
 *	Modified from code written by:	Po-hua Chang
 * 	Copyright (c) 1991 David August, Nancy Warter, Po-hua Chang, 
 *              Wen-mei Hwu
 *	       	and The Board of Trustees of the University of Illinois.
 *	       	All rights reserved.
 *	The University of Illinois software License Agreement
 * 	specifies the terms and conditions for redistribution.
\*****************************************************************************/

#include <config.h>
#include <string.h>
#include "pcode.h"
#include "struct.h"
#include "parms.h"
#include "extension.h"

#if 0
/* Pulling BT_SIGNED out of the _BasicType enum so it can be deprecated. */
const int BT_SIGNED = 0x200;
#endif

char *P_parm_file = NULL;

Dcl P_Input;

ExtHandler **Handlers = NULL;

int NumExtensions[ES_LAST];

/* An array to map opcodes to a string of their enum value. */
const char *op_to_value[OP_last] = 
{
  "",                           /*!< No opcode 0 */
  "OP_var",
  "OP_enum",
  "OP_int",
  "OP_real",
  "OP_error",
  "OP_char",
  "OP_string",
  "OP_dot",
  "OP_arrow",
  "OP_cast",
  "OP_expr_size",
  "OP_type_size",
  "OP_quest",
  "OP_disj",
  "OP_conj",
  "OP_compexpr",
  "OP_assign",
  "OP_or",
  "OP_xor",
  "OP_and",
  "OP_eq",
  "OP_ne",
  "OP_lt",
  "OP_le",
  "OP_ge",
  "OP_gt",
  "OP_rshft",
  "OP_lshft",
  "OP_add",
  "OP_sub",
  "OP_mul",
  "OP_div",
  "OP_mod",
  "OP_neg",
  "OP_not",
  "OP_inv",
  "",                           /*!< No opcode 37 */
  "OP_preinc",
  "OP_predec",
  "OP_postinc",
  "OP_postdec",
  "OP_Aadd",
  "OP_Asub",
  "OP_Amul",
  "OP_Adiv",
  "OP_Amod",
  "OP_Arshft",
  "OP_Alshft",
  "OP_Aand",
  "OP_Aor",
  "OP_Axor",
  "OP_indr",
  "OP_addr",
  "OP_index",
  "OP_call",
  "OP_float",
  "OP_double",
  "OP_null",
  "OP_sync",
  "OP_stmt_expr",
  "OP_asm_oprd"
};

/* An array to map ExtStruct enum values to human readable strings. */
const char *ExtStruct_To_String[ES_LAST] = 
{
  "FuncDcl",
  "TypeDcl",
  "VarDcl",
  "Init",
  "StructDcl",
  "UnionDcl",
  "Field",
  "EnumDcl",
  "Stmt",
  "Pstmt",
  "Expr",
  "AsmDcl",
  "SymTabEntry",
  "IPSymTabEnt"
};

const Key Invalid_Key = {0, 0};

int Indices[ES_LAST];

extern void P_def_handlers (char *prog_name, Parm_Macro_List *external_list);
extern void P_show_help (char *prog_name, Parm_Macro_List *external_list);

/*! \brief Initializes the default input and output file handles.
 */
void
P_init_io (void)
{
  Fin = stdin;
  F_input = "stdin";
  Fout = stdout;
  Fout2 = stdout;
  F_output = "stdout";
  Ferr = stderr;
  Flog = stderr;
  Fstpcode = stderr;
  Fannot = stdin;
  Fannot_index = stdin;
  Fpcode_position = stdout;

  return;
}

/*! \brief Initializes the extension handlers.
 */
void
P_init_handlers (char *prog_name, Parm_Macro_List *external_list)
{
  int i;

  for (i = 0; i < ES_LAST; i++)
    NumExtensions[i] = 0;

  Handlers = calloc (ES_LAST, sizeof (ExtHandler *));

  if (Handlers == NULL)
    P_punt ("pcode.c:P_init_handlers:%d Could not alloc Handlers", __LINE__);

  P_def_handlers (prog_name, external_list);

  /* Initialize the Pcode library extension handlers. */
  Indices[ES_SYMTABENTRY] = \
    P_ExtSetupL (ES_SYMTABENTRY, (AllocHandler)P_NewKeyMap,
		 (FreeHandler)P_RemoveKeyMap);
  P_ExtSetOptionL (ES_SYMTABENTRY, Indices[ES_SYMTABENTRY], HO_MANUAL_ALLOC);

  return;
}

/*! \brief Maps _BasicType values to human readable strings.
 *
 * \param b
 *  the _BasicType value.
 *
 * \return
 *  A human readable equivalent to \a b.
 */
char *
P_BasicTypeToString (_BasicType b)
{
  char *result = NULL;

  if (b & BT_ARITHMETIC)
    {
      if (b & BT_INTEGRAL)
	{
	  if (b & BT_UNSIGNED)
	    {
	      switch (b & BT_INTEGRAL_BASE)
		{
		case BT_CHAR:
		  result = "unsigned char";
		  break;
		case BT_SHORT:
		  result = "unsigned short";
		  break;
		case BT_INT:
		  result = "unsigned int";
		  break;
		case BT_LONG:
		  result = "unsigned long";
		  break;
		case BT_LONGLONG:
		  result = "unsigned long long";
		  break;
		default:
		  result = "unknown";
		  break;
		}
	    }
	  else
	    {
	      switch (b & BT_INTEGRAL_BASE)
		{
		case BT_CHAR:
		  result = "signed char";
		  break;
		case BT_SHORT:
		  result = "signed short";
		  break;
		case BT_INT:
		  result = "signed int";
		  break;
		case BT_LONG:
		  result = "signed long";
		  break;
		case BT_LONGLONG:
		  result = "signed long long";
		  break;
		default:
		  result = "unknown";
		  break;
		}
	    }
	}
      else if (b & BT_REAL)
	{
	  switch (b & BT_REAL)
	    {
	    case BT_FLOAT:
	      result = "float";
	      break;
	    case BT_DOUBLE:
	      result = "double";
	      break;
	    case BT_LONGDOUBLE:
	      result = "long double";
	      break;
	    default:
	      result = "unknown";
	      break;
	    }
	}
      else
	{
	  result = "unknown";
	}
    }
  else if (b & BT_STRUCTURE)
    {
      switch (b & BT_STRUCTURE)
	{
	case BT_STRUCT:
	  result = "struct";
	  break;
	case BT_UNION:
	  result = "union";
	  break;
	default:
	  result = "unknown";
	  break;
	}
    }
  else if (b & BT_VOID)
    {
      result = "void";
    }
  else if (b & BT_VARARG)
    {
      result = "vararg";
    }
  else if (b & BT_ARRAY)
    {
      result = "array";
    }
  else if (b & BT_FUNC)
    {
      result = "func";
    }
  else if (b & BT_POINTER)
    {
      result = "pointer";
    }
  else if (b & BT_TYPEDEF)
    {
      switch (b & BT_TYPEDEF)
	{
	case BT_TYPEDEF_E:
	  result = "user defined typedef";
	  break;
	case BT_TYPEDEF_I:
	  result = "compiler defined typedef";
	  break;
	default:
	  result = "unknown";
	  break;
	}
    }
  else
    {
      result = "unknown";
    }

  return (result);
}

/*! \brief Maps _EntryType values to human readable strings.
 *
 * \param e
 *  the _EntryType value.
 *
 * \return
 *  A human readable equivalent to \a e.
 */
char *
P_EntryTypeToString (_EntryType e)
{
  char *result = NULL;

  switch (e)
    {
    case ET_NONE:
      result = "none";
      break;
    case ET_FUNC:
      result = "func";
      break;
    case ET_TYPE_LOCAL:
      result = "local type";
      break;
    case ET_TYPE_GLOBAL:
      result = "global type";
      break;
    case ET_VAR_LOCAL:
      result = "local var";
      break;
    case ET_VAR_GLOBAL:
      result = "global var";
      break;
    case ET_STRUCT:
      result = "struct";
      break;
    case ET_UNION:
      result = "union";
      break;
    case ET_ENUM:
      result = "enum";
      break;
    case ET_ASM:
      result = "asm";
      break;
    case ET_STMT:
      result = "stmt";
      break;
    case ET_EXPR:
      result = "expr";
      break;
    case ET_FIELD:
      result = "struct/union field";
      break;
    case ET_ENUMFIELD:
      result = "enum field";
      break;
    case ET_LABEL:
      result = "label";
      break;
    case ET_SCOPE:
      result = "scope";
      break;
    case ET_BLOCK:
      result = "block";
      break;
    default:
      result = "unknown";
      break;
    }

  return (result);
}
