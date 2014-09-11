/*****************************************************************************\
 *
 *                    Illinois Open Source License
 *                     University of Illinois/NCSA
 *                         Open Source License
 *
 * Copyright (c) 2004, The University of Illinois at Urbana-Champaign.
 * All rights reserved.
 *
 * Developed by:
 *
 *              IMPACT Research Group
 *
 *              University of Illinois at Urbana-Champaign
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

#include <config.h>
#include "pl_main.h"

typedef struct polym_t
{
  int  offset;
  List fieldlist;
} polym_t;


static List
Find_PolyMorphic (Field field, List poly_list, int offset)
{
  polym_t *pm;
  
  for (; field; field = field->next)
    {
      if (!PST_IsStructureType(PL_symtab, field->type))
	{
	  /* Look at base fields */
	  List_start(poly_list);
	  while ((pm = List_next(poly_list)))
	    if (offset + field->offset == pm->offset)
	      break;

	  if (!pm)
	    {
	      pm = calloc(1, sizeof(polym_t));
	      pm->offset = field->offset;
	      poly_list = List_insert_last(poly_list, pm);
	    }

	  pm->fieldlist = List_insert_last(pm->fieldlist, field);
	}
      else
	{
	  if (PST_GetTypeBasicType(PL_symtab, field->type) & BT_STRUCT)
	    {
	      StructDcl st = NULL;
	      st = PST_GetStructDclEntry(PL_symtab, 
					 PST_GetTypeType(PL_symtab, 
							 field->type));
	      poly_list = Find_PolyMorphic (st->fields, poly_list, 
					    offset + field->offset);
	    }
	  else if (PST_GetTypeBasicType(PL_symtab, field->type) & BT_UNION)
	    {
	      UnionDcl un = NULL;
	      un = PST_GetUnionDclEntry(PL_symtab, 
					PST_GetTypeType(PL_symtab, 
							field->type));
	      poly_list = Find_PolyMorphic (un->fields, poly_list, 
					    offset + field->offset);
	    }
	  else
	    assert(0);
	}
    }

  return poly_list;
}


static polym_t *
Search_PolyMorphic (List poly_list, Field search_field)
{
  polym_t *pm;
  Field field;

  List_start(poly_list);
  while ((pm = List_next(poly_list)))
    {
      if (List_size(pm->fieldlist) == 1)
	continue;
#if 0
      printf("Polymorphic field at offset %d [%d]\n",
	     pm->offset, List_size(pm->fieldlist));
#endif
      List_start(pm->fieldlist);
      while ((field = List_next(pm->fieldlist)))
	{
	  if (field == search_field)
	    goto FOUND_POLY;
	}
    }

 FOUND_POLY:
  return pm;
}


static void
Del_PolyMorphic (List poly_list)
{
  polym_t *pm;

  List_start(poly_list);
  while ((pm = List_next(poly_list)))
    {
      List_reset(pm->fieldlist);
      free(pm);
    }
  List_reset(poly_list);
}


int
P_look_for_polymorphic(Expr expr)
{
  Expr un_expr, ptr;
  List poly_list;
  Field acc_field, field;
  polym_t *pm;
  UnionDcl un;
  Key un_type;
  int found_union = 0, mixed = 0;
  
  /* See if the access is to a field in a union 
   */
  return 0;

  if (expr->opcode != OP_dot &&
      expr->opcode != OP_arrow)
    return 0;

  for (ptr = expr; (ptr->opcode == OP_dot || ptr->opcode == OP_arrow); 
       ptr = ptr->operands)
    {
      Key type = PST_ExprType(PL_symtab, ptr);
      Key dtype = PST_DereferenceType(PL_symtab, type);

      if (ptr->opcode == OP_dot && 
	  (PST_GetTypeBasicType(PL_symtab, type) & BT_UNION))
	{
	  found_union = 1;
	  un_expr = ptr;
	  un_type = type;
	  break;
	}
      if (ptr->opcode == OP_arrow && 
	  (PST_GetTypeBasicType(PL_symtab, dtype) & BT_UNION))
	{
	  found_union = 1;
	  un_expr = ptr;
	  un_type = dtype;
	  break;
	}
    }

  /* Casts may need to be added to the loop above
     for acceptable opcodes */
  assert(ptr->opcode != OP_cast);
  if (!found_union)
    return 0;

  /* Get the field info for the field in question 
   */
  un = PST_GetUnionDclEntry(PL_symtab, 
			    PST_GetTypeType(PL_symtab, un_type));
  field = un->fields;
  
  acc_field = NULL;
  for (; field; field = field->next)
    {
      /* Expr contains the field name in question */
      if (!strcmp(expr->value.var.name, field->name))
	{
	  acc_field = field;
	  break;
	}
    }
  assert(acc_field);

  if (!PST_IsPointerType(PL_symtab, acc_field->type))
    return 0;

  /* Using the highest union in the type, determine if this
   *   field is polymorphic 
   */
  poly_list = Find_PolyMorphic(field, NULL, 0);

  if ((pm = Search_PolyMorphic(poly_list, acc_field)))
    {
      List_start(pm->fieldlist);
      while ((field = List_next(pm->fieldlist)))
	{
	  if (!PST_IsPointerType(PL_symtab, field->type))
	    {
	      mixed = 1;
	      break;
	    }
	}
    }

  Del_PolyMorphic (poly_list);

  return mixed;
}

