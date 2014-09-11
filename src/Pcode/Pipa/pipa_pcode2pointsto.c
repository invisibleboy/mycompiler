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


/*****************************************************************************\
 *      File:    pipa_pcode2pointsto.c
 *      Author:  Erik Nystrom
 *      Copyright (c) 2003  Erik Nystrom, Wen-mei Hwu
 *              and The Board of Trustees of the University of Illinois.
 *              All rights reserved.
 *      The University of Illinois software License Agreement
 *      specifies the terms and conditions for redistribution.
\*****************************************************************************/

#include "pipa_pcode2pointsto.h"
#include <Pcode/query_symtab.h>
#include <Pcode/reduce_symtab.h>
#include <Pcode/ss_ssa2.h>
#include <Pcode/ss_lifetime.h>



/* PRAGMAS USEABLE BY PIPA 
 *
 * pipa_ignore      - ignore the following statement
 * pipa_heap_alloc  - marks the return of a heap allocation routine 
 *                    + forces a model where the summary generated
 *                      acts just like malloc (returns a fresh heap location)
 * IN PROGRESS:
 *
 * pipa_heap_free   - marks a heap deallocation routine
 *                    + (for now) forces a model where this routine has 
 *                      no summary at all
 *
 * pipa_blockcopy   - marks a routine block copy routine like memcpy
 *
 */

static int IsVarargParam (VarDcl param);
extern IPA_prog_info_t *Globals_ProgInfo;

#define DEBUG_IPA_LEVEL    0

Expr
IPA_find_var_expr(Expr expr)
{
  while (expr && expr->opcode != OP_var)
    expr = expr->operands;

  return expr;
}

int
IPA_bcg_formalparam(IPA_prog_info_t * info, 
		    buildcg_t *l_bcg,
		    Key param_type_key,
		    int pcnt, int isvararg)
{
  buildcg_t *r_bcg;
  IPA_symbol_info_t *sym;
  char name[256];

  /* Add symbol */
  sprintf (name, "%s_%d", IPA_FORMAL_PARAM_PREFIX, pcnt);
  sym = IPA_Pcode_Add_Sym (info, name, IPA_symbol_tmpvarkey(),
			   param_type_key,
			   IPA_VARKIND_PARAM);
      
  /* Add assignment */
  r_bcg = IPA_buildcg_start(info, info->cur_fninfo, sym->id, 1, B_BUILD);
  if (isvararg)
    IPA_FLAG_SET(r_bcg->node->flags, IPA_CG_NODE_FLAGS_ELLIPSE);
  IPA_bcg_assign(info, l_bcg, r_bcg, 
		 IPA_Pcode_sizeof(info, param_type_key));
  IPA_buildcg_free (r_bcg);
      
  /* Add id of fake formal to param list */
  IPA_interface_append_param_id (info->cur_fninfo->iface, sym->id);

  return sym->id;
}

void 
IPA_bcg_actualparam(IPA_prog_info_t * info, 
		    IPA_funcsymbol_info_t *fninfo,
		    IPA_callsite_t *cs,
		    buildcg_t *param_bcg,
		    Key param_type_key,
		    int pcnt)
{ 
  buildcg_t *l_bcg;
  IPA_symbol_info_t *sym;
  char name[256];
  static int callsite_id = 1;

  /* Add symbol */
  sprintf (name, "%s_%d_cid_%d", "PARAM_IN", pcnt, callsite_id);
  sym = IPA_Pcode_Add_Sym (info, name, 
			   IPA_symbol_tmpvarkey(), 
			   IPA_symbol_tmptypekey(),
			   IPA_VARKIND_LOCAL);

  /* Add assignment */
  l_bcg = IPA_buildcg_start(info, info->cur_fninfo, sym->id, 1, B_BUILD);
  if (param_bcg)
    {
      IPA_bcg_assign(info, l_bcg, param_bcg,
		     IPA_Pcode_sizeof(info, param_type_key));
    }
  IPA_buildcg_free (l_bcg);

  /* Add id to callsite */
  IPA_interface_set_param_id (cs->iface, pcnt, sym->id);  

  /* Incr */
  callsite_id++;
}

/* info->cur_fninfo, */

void
IPA_bcg_formalreturn(IPA_prog_info_t * info, 
		     IPA_funcsymbol_info_t *fninfo,
		     Expr return_expr,
		     int is_heap)
{
  IPA_symbol_info_t *sym;
  buildcg_t *l_bcg;
  buildcg_t *r_bcg;
  Expr var_expr;
  static int rtmp = 1;
  int size, id;
  char name[256];

  if (!is_heap)
    {
      /* Use return expr for rval */
      var_expr = IPA_find_var_expr(return_expr);
      if (!var_expr)
	return;
      if (!(r_bcg = IPA_BuildEqns_For_Expr (info, return_expr, var_expr)))
	return;

      size =  IPA_Pcode_sizeof (info, IPA_ExprType(info, return_expr));
    }
  else
    {
      strcpy (name, IPA_ALLOC_VAR_PREFIX);
      strcat (name, fninfo->func_name);
      sym = IPA_Pcode_Add_Sym (info, name, IPA_symbol_tmpvarkey(),
			       IPA_ExprType(info, return_expr), 
			       IPA_VARKIND_HEAP);

      r_bcg = IPA_buildcg_start(info, info->cur_fninfo, sym->id, 1, B_BUILD);
      IPA_bcg_addrof(info, r_bcg);

      size = IPA_POINTER_SIZE;
    }

  if ( is_heap ||
       return_expr->opcode != OP_var)
    {
      /* Create temporary */
      sprintf (name, "%s%d", "RETTMP_", rtmp++);
      sym = IPA_Pcode_Add_Sym (info, name, IPA_symbol_tmpvarkey(),
			       IPA_ExprType(info, return_expr),
			       IPA_VARKIND_LOCAL);

      /* Assignment to temporary */
      l_bcg =  IPA_buildcg_start(info, info->cur_fninfo, sym->id, 1, B_BUILD);
      IPA_bcg_assign(info, l_bcg, r_bcg, size);

      IPA_buildcg_free (r_bcg);
      r_bcg = l_bcg;
      l_bcg = NULL;
    }


  /* Find/Create formal return variable */
  if ((id = IPA_interface_get_ret_id(fninfo->iface)) == 0)
    {
      sym = IPA_Pcode_Add_Sym (info, IPA_ALLOC_RETURN, IPA_symbol_tmpvarkey(),
			       IPA_ExprType(info, return_expr),
			       IPA_VARKIND_RETURN);
      sym->kind = IPA_VARKIND_RETURN;
    }
  else
    {
      sym = IPA_symbol_find_by_id(info, id);
    }
  
  /* Assignment to formal return */
  l_bcg =  IPA_buildcg_start(info, info->cur_fninfo, sym->id, 1, B_BUILD);
  IPA_bcg_assign(info, l_bcg, r_bcg, size);
  IPA_buildcg_free (r_bcg);
  IPA_buildcg_free (l_bcg);

  /* Update interface */
  IPA_interface_set_ret_id (fninfo->iface, sym->id, 1);
}

void 
IPA_bcg_actualreturn(IPA_prog_info_t * info, 
		     IPA_funcsymbol_info_t *fninfo,
		     char *name, int version, Key key,
		     Key type_key,
		     IPA_callsite_t *cs)
{
  IPA_symbol_info_t *sym;
  static int cnt = 1;

  assert(IPA_interface_get_ret_id(cs->iface) == 0);

  /* Get id for actual return variable */
  sym = IPA_Pcode_Add_Sym (info, name, key, type_key,
			   IPA_VARKIND_LOCAL);

  /* Add id to callsite */
  IPA_interface_set_ret_id (cs->iface, sym->id, version);

  /* This simulates the effect of alloca() which is
     not a real function */
  if (!cs->indirect &&
      (strcmp(cs->callee.dir.name, "alloca") == 0 ||
       strcmp(cs->callee.dir.name, "__builtin_alloca") == 0))
    {
      char name[256];
      Key type;
      buildcg_t *r_bcg;
      buildcg_t *l_bcg;

      l_bcg =  IPA_buildcg_start(info, info->cur_fninfo, sym->id, 1, B_BUILD);

      strcpy (name, IPA_ALLOC_VAR_PREFIX);
      strcat (name, "alloca");

      type = PST_FindPointerToType (info->symboltable, 
				    PST_FindBasicType (info->symboltable,
						       BT_VOID));
      sym = IPA_Pcode_Add_Sym (info, name, IPA_symbol_tmpvarkey(),
			       type, IPA_VARKIND_STACK);
      printf("[%d:%10.10s] ALLOCA %d %s\n",cnt++,fninfo->func_name,sym->id,name);
      
      r_bcg = IPA_buildcg_start(info, info->cur_fninfo, sym->id, 1, B_BUILD);
      IPA_bcg_addrof(info, r_bcg);

      IPA_bcg_assign(info, l_bcg, r_bcg, IPA_POINTER_SIZE);
      IPA_buildcg_free (r_bcg);
      IPA_buildcg_free (l_bcg);      
    }
}

IPA_callsite_t *
IPA_bcg_direct_call(IPA_prog_info_t * info, 
		    IPA_funcsymbol_info_t *fninfo,
		    char *callee_name, Key callee_key)
{
  IPA_callsite_t *cs;

  /* Callsite */
  cs = IPA_callsite_new_prog (info, fninfo,
			      callee_name, callee_key);
  return cs;
}

IPA_callsite_t *
IPA_bcg_indirect_call(IPA_prog_info_t * info, 
		      IPA_funcsymbol_info_t *fninfo,
		      buildcg_t *callee_bcg,
		      Key call_type_key)
{
  IPA_callsite_t *cs;
  buildcg_t *l_bcg;
  IPA_symbol_info_t *sym;
  char name[256];
  Key dummy = {-1,-1};

  /* Callsite */
  cs = IPA_callsite_new_prog (info, fninfo, 
			      NULL, dummy);

  /* Create fake callee node */
  sprintf (name, "%s_%d", IPA_IND_CALLEE_PREFIX, cs->cs_id);
  sym = IPA_Pcode_Add_Sym (info, name, 
			   IPA_symbol_tmpvarkey(), 
			   IPA_symbol_tmptypekey(),
			   IPA_VARKIND_LOCAL);
  l_bcg = IPA_buildcg_start(info, info->cur_fninfo, sym->id, 1, B_BUILD);

  IPA_bcg_assign(info, l_bcg, callee_bcg, IPA_POINTER_SIZE);
  
  cs->callee.cnode_id = l_bcg->node->data.var_id;
  IPA_buildcg_free (l_bcg);

  return cs;
}
		    


/****************************************************************
 * Pcode helper functions
 ****************************************************************/
#if 0
Key ipa_scope_key;
#endif

Key
IPA_ExprType(IPA_prog_info_t *info, Expr expr)
{
#if 0
  /* THIS IS TEMPORARY CODE TO GET THROUGH AN ERROR */
  Expr tmp_expr = expr;
  while (tmp_expr)
    {
      Key bad_key = {-1,-1};
      if (tmp_expr->opcode == OP_var &&
	  strcmp(tmp_expr->value.var.name, "__builtin_va_alist") == 0)
	return bad_key;
      tmp_expr = tmp_expr->operands;
    }
#endif

#if 0
  /* This is a hack to get around a temporary symtab problem
   */
  if (expr->parentstmt)
    scope_key = PST_GetExprScope (info->symboltable, expr);
  else
    scope_key = ipa_scope_key;
#endif

  return PST_ExprType(info->symboltable, expr);
}

/* Function dcltr only */
int
IPA_Pcode_IsFunctionType(IPA_prog_info_t *info, Key type_key)
{
  return PST_IsFunctionType(info->symboltable, type_key);
}

/* One or more ptrs */
int
IPA_Pcode_IsPointerType(IPA_prog_info_t *info, Key type_key)
{
  return PST_IsPointerType(info->symboltable, type_key);
}

/* One or more array dcltrs but no pointers */
int
IPA_Pcode_IsArrayType(IPA_prog_info_t *info, Key type_key)
{
  return PST_IsArrayType(info->symboltable, type_key);
}

/* Structure dcltr only */
int
IPA_Pcode_IsStructureType(IPA_prog_info_t *info, Key type_key)
{
  return PST_IsStructureType(info->symboltable, type_key);
}

int
IPA_Pcode_IsContentPointerType(IPA_prog_info_t *info, Key type_key)
{
  if (PST_IsArrayType(info->symboltable, type_key))
    return IPA_Pcode_IsContentPointerType(info, 
					  PST_GetTypeType(info->symboltable, 
							  type_key));
  else
    return PST_IsPointerType(info->symboltable, type_key);
}


/* One or more array dcltrs but no pointers ending in structure */
int
IPA_Pcode_IsStructureArrayType(IPA_prog_info_t *info, Key type_key)
{
  do 
    {
      if (!PST_IsArrayType(info->symboltable, type_key))
	return 0;
      
      type_key = PST_GetTypeType(info->symboltable, type_key);

      if (PST_IsStructureType(info->symboltable, type_key))
	return 1;
    }
  while (1);
  
  return 0;
}

int 
IPA_Pcode_sizeof (IPA_prog_info_t *info,
		  Key type_key)
{
  int size;

  /* Temporary variable type created for the consg
     but unknown to real symbol table */
  if (type_key.file == IPA_TEMPTYPE_FILEID)
    return IPA_POINTER_SIZE;

  assert(P_ValidKey(type_key));

  /* This is ok as long as IPA_Pcode_sizeof is never
   * used for struct layout (which shouldn't occur)
   *
   * The goal is to return the size of the underlying object
   *   past any array portions. This isn't a default 
   *   pointer-sized situation because we want a struct array
   *   to end up with a node the size of the struct.
   */
  while (PST_IsArrayType(info->symboltable, type_key))
    {
      type_key = PST_GetTypeType(info->symboltable, type_key); 
    }
  
  if (PST_IsFunctionType(info->symboltable, type_key))
    size = IPA_POINTER_SIZE;
  else
    size = PST_GetTypeSize(info->symboltable, type_key);

  if (size <= 0)
    {
#if 0
      _Dcl dcl;
      TypeDcl tdcl;

      fprintf(info->errfile,"### TYPE SIZE MUST BE > 0 forcing to %d\n",
	      IPA_POINTER_SIZE);

      tdcl = PST_GetTypeDclEntry(info->symboltable, type_key);
      dcl.type = TT_TYPE;
      dcl.ptr.typeDcl = tdcl;
      P_write_type_dcl(info->errfile, &dcl, 5, NULL);
      fprintf(info->errfile,"\n");
#endif
      return IPA_POINTER_SIZE;
    }
  
  return size;
}

/* Same as above function, except it returns the size of arrays rather than size
   of array elements. */
int 
IPA_Pcode_sizeof_array (IPA_prog_info_t *info,
                        Key type_key)
{
  int size;

  /* Temporary variable type created for the consg
     but unknown to real symbol table */
  if (type_key.file == IPA_TEMPTYPE_FILEID)
    return IPA_POINTER_SIZE;

  assert(P_ValidKey(type_key));

  if (PST_IsFunctionType(info->symboltable, type_key))
    size = IPA_POINTER_SIZE;
  else
    size = PST_GetTypeSize(info->symboltable, type_key);

  if (size <= 0)
    {
      return IPA_POINTER_SIZE;
    }

  return size;
}

Field
IPA_Pcode_get_field (IPA_prog_info_t *info,
		     Key type_key, char *field_name)
{
  Field field;

  if (PST_GetTypeBasicType(info->symboltable, type_key) & BT_STRUCT)
    {
      StructDcl st = PST_GetStructDclEntry (info->symboltable, 
					    PST_GetTypeType(info->symboltable,
							    type_key));
      field = P_GetStructDclFields (st);
    }
  else if (PST_GetTypeBasicType(info->symboltable, type_key) & BT_UNION)
    {
      UnionDcl un = PST_GetUnionDclEntry (info->symboltable, 
					  PST_GetTypeType(info->symboltable,
							  type_key));
      field = P_GetUnionDclFields (un);
    }
  else
    assert(0);

  if (!field_name)
    return field;

  for (; field; field = P_GetFieldNext (field))
    {
      if (strcmp (P_GetFieldName (field), field_name) == 0)
        {
          return field;
        }
    }

  I_punt ("IPA_Pcode_get_field: %s not found\n", field_name);
  return NULL;
}

int
IPA_Expr_Has_Var_Involved (Expr expr)
{
  int sib, child;

  if (expr == 0)
    return 0;
  switch (expr->opcode)
    {
    case OP_int:
    case OP_enum:
    case OP_real:
    case OP_char:
      return 0;
    case OP_string:
    case OP_var:
      return 1;
    default:
      break;
    }
  child = IPA_Expr_Has_Var_Involved (expr->operands);
  sib = IPA_Expr_Has_Var_Involved (expr->sibling);
  return child + sib;
}


int
IPA_Expr_Has_Indr (Expr expr)
{
  Expr ptr;

  if (expr->opcode == OP_indr ||
      expr->opcode == OP_index)
    return 1;

  for (ptr = expr->operands; ptr != 0; ptr = ptr->sibling)
    {
      if (IPA_Expr_Has_Indr(ptr))
	return 1;
    }

  return 0;
}


/****************************************************************
 * Pointsto helper functions
 ****************************************************************/

#if 0
void
IPA_Pcode_Get_Offsets (IPA_prog_info_t * info,
                       Type type, IPA_var_type_t *var_type,
		       int start_offset)
{
  StructDcl st;
  UnionDcl un;
  Field field;

  if (!(type->type & (TY_STRUCT | TY_UNION)))
    return;

  if ((st = FindStructNoScopeId (type->struct_name)))
    {
      field = st->fields;
    }
  else if ((un = FindUnionNoScopeId (type->struct_name)))
    {
      field = un->fields;
    }
  else
    I_punt ("IPA_Pcode_Get_Offsets: unexpected case\n");

  for (; field; field = field->next)
    {
      if (IsStructureType (field->type) || IsStructureArrayType (field->type))
        {
          IPA_Pcode_Get_Offsets (info, field->type, var_type,
                                 start_offset + field->offset);
        }
      else
        {
	  IPA_var_type_add_offset(var_type, 
				  (start_offset + field->offset));
        }
    }

  return;
}
#endif

IPA_symbol_info_t *
IPA_Pcode_Get_Sym (IPA_prog_info_t * info, Key sym_key)
{
  return IPA_symbol_find (info, sym_key);
}

IPA_symbol_info_t *
IPA_Pcode_Add_Sym (IPA_prog_info_t * info,
		   char *sym_name, Key sym_key,
		   Key type_key, int flags)
{
  IPA_symbol_info_t *sym;

  sym = IPA_symbol_add (info, info->cur_fninfo, 
			sym_name, sym_key,
			flags, type_key);

  return sym;
}



/****************************************************************
 *
 * These functions are create new call sites
 *
 ****************************************************************/

void
IPA_Pcode_Add_CallSite (IPA_prog_info_t * info, Expr call_expr)
{
  IPA_callsite_t *cs = NULL;
  IPA_funcsymbol_info_t *fninfo;
  buildcg_t *bcg;
  Expr param = NULL;
  Expr r_var_expr = NULL;
  Expr ret_expr = NULL;
  int i;

  fninfo = info->cur_fninfo;
  if (!fninfo)
    I_punt ("IPA_Add_CallSite: fn not found\n");

  /* Build the callee equation
   *  This eqn simply resolves to the possible callees at
   *   this particular callsite. It should be ok to build this
   *   before all of the local accesses have been located since
   *   function symbols should be global.
   */
  r_var_expr = call_expr;
  while (r_var_expr->opcode != OP_var)
    r_var_expr = r_var_expr->operands;

  if (call_expr->operands->opcode == OP_var)
    {
      if (!strcmp("__builtin_stdarg_start", r_var_expr->value.var.name) ||
	  !strcmp("__builtin_va_start", r_var_expr->value.var.name))
	{
	  Expr va_list_expr;
	  buildcg_t *ellipse_bcg;
	  buildcg_t *va_bcg;

	  /* This is a special case for varargs on IA64 
	   *   Find the first parameter, this is the va_list variable
	   *   Assign the address of the ellipse var to this var
	   */
	  printf("   - Found __builtin_stdarg_start, connecting ellipse to va_list \n");
	  /* ELLIPSE */
	  assert(info->cur_fninfo->ellipse_id > 0);
	  ellipse_bcg = IPA_buildcg_start(info, info->cur_fninfo, 
					  info->cur_fninfo->ellipse_id,
					  1, B_BUILD);
	  IPA_bcg_addrof(info, ellipse_bcg);
	  
	  /* VA */
	  va_list_expr = call_expr->operands->sibling;

	  assert(va_list_expr->opcode == OP_var);
	  va_bcg = IPA_BuildEqns_For_Expr (info, va_list_expr, va_list_expr);

	  /* ASSIGNMENT */
	  IPA_bcg_assign(info, va_bcg, ellipse_bcg, 
			 IPA_Pcode_sizeof (info, 
					   IPA_ExprType(info, va_list_expr)));

	  IPA_buildcg_free(va_bcg);
	  IPA_buildcg_free(ellipse_bcg);

	  return;
	}
      else 
	{
	  cs = IPA_bcg_direct_call(info, fninfo,
				   r_var_expr->value.var.name,
				   r_var_expr->value.var.key);
	}
    }
  else
    {
      bcg = IPA_BuildEqns_For_Expr (info, call_expr, r_var_expr);
      IPA_bcg_addrof(info, bcg);

      cs = IPA_bcg_indirect_call(info, fninfo, bcg, 
				 IPA_ExprType(info, call_expr->operands));

      IPA_buildcg_free (bcg);
    }
  cs->call_expr = call_expr;

  /* Build the parameter interface
   */
  for (param = call_expr->operands->sibling, i = 0; param;
       param = param->next, i++)
    {
      /* Build the parameter interface
       */
      r_var_expr = param;
      while (r_var_expr && r_var_expr->opcode != OP_var)
        r_var_expr = r_var_expr->operands;

      if (!r_var_expr)
	{
	  IPA_bcg_actualparam(info, fninfo, cs,
			      NULL, IPA_ExprType(info, param), i);
	}
      else
	{
	  bcg = IPA_BuildEqns_For_Expr (info, param, r_var_expr);
	  IPA_bcg_actualparam(info, fninfo, cs,
			      bcg, IPA_ExprType(info, param), i);
	  IPA_buildcg_free(bcg);
	}
   }

  /* Build return equation
   */
  ret_expr = call_expr;
  while (ret_expr)
    {
      if (ret_expr->opcode == OP_assign)
        break;
      ret_expr = ret_expr->parentexpr;
    }

  if (ret_expr)
    {
      ret_expr = ret_expr->operands;
      r_var_expr = ret_expr;
      while (r_var_expr->opcode != OP_var)
        r_var_expr = r_var_expr->operands;
     
	{
	  int version = r_var_expr->value.var.ssa ? 
	    r_var_expr->value.var.ssa->name : 1;
	  
	  IPA_bcg_actualreturn(info, fninfo, 
			       r_var_expr->value.var.name,
			       version, r_var_expr->value.var.key,
			       IPA_ExprType(info, r_var_expr),
			       cs);
	}
    }

}


/****************************************************************
 *
 * These functions are used to find all of the assignments and
 *   to add in the root points to equations
 *
 ****************************************************************/

int
IPA_LHS_All_Literals (Expr expr)
{
  switch (expr->opcode)
    {
    case OP_cast:
    case OP_compexpr:
      return IPA_LHS_All_Literals (expr->operands);
    case OP_char:
    case OP_int:
      return 1;
    default:
      return 0;
    }
  I_punt ("DD_LHS_All_Literals: internal error\n");
}

Expr
IPA_Find_Significant_Parent_As_First_Operand (Expr child_expr)
{
  Expr par_expr;

  assert (child_expr);

  /* top level expr */
  if (child_expr->parentexpr == NULL)
    return 0;

  /* right-descendant */
  if (child_expr->parentexpr->operands != child_expr &&
      !IPA_LHS_All_Literals (child_expr->parentexpr->operands))
    return 0;

  par_expr = child_expr->parentexpr;
  do
    {
      switch (par_expr->opcode)
        {
          /* useless operators */
#if 0
        case OP_cast:
#endif
        case OP_compexpr:
          par_expr = par_expr->parentexpr;
          break;
        default:
          return par_expr;
        }
    }
  while (par_expr);
  return 0;
}

void
IPA_Expr_Addr_Eqn (IPA_prog_info_t * info, Expr expr, buildcg_t *bcg)
{
  IPA_bcg_addrof(info, bcg);
}

void
IPA_Expr_Deref_Eqn (IPA_prog_info_t * info, Expr expr, buildcg_t *bcg)
{
  IPA_bcg_deref(info, bcg);
}


void
IPA_Expr_Field_Eqn (IPA_prog_info_t * info, Expr expr, buildcg_t *bcg)
{
  Field field;

  field = IPA_Pcode_get_field (info, IPA_ExprType(info, expr->operands), 
				expr->value.string);

  IPA_bcg_offset(info, bcg, field->offset);
}


void
IPA_Expr_Arrow_Eqn (IPA_prog_info_t * info, Expr expr, buildcg_t *bcg)
{
  Field field;
  Key type_key;

  /* Incorporate dereference */
  IPA_bcg_deref(info, bcg);

  /* Should look  like BT_POINTER then BT_STRUCTURE */
  type_key = IPA_ExprType(info, expr->operands);
  assert(IPA_Pcode_IsPointerType(info, type_key));

  /* Now use structure type and field name to get info */
  type_key = PST_GetTypeType(info->symboltable, type_key);
  field = IPA_Pcode_get_field (info, type_key,
			       expr->value.string);

  /* Incorporate offset */
  IPA_bcg_offset(info, bcg, field->offset);
}

void
IPA_Expr_Handle_Addition (IPA_prog_info_t * info, Expr expr, buildcg_t *bcg)
{

  Expr offset_expr;
  int offset_val;
  int multiplier;
  
  if (IPA_field_safety < IPA_SAFETY_LEVEL1)
    return;

  offset_expr = PST_ReduceExpr(info->symboltable, 
			       expr->operands->sibling);

  /* Compute the multiplier */
  if (PST_IsPointerTypeExpr (info->symboltable, expr->operands) ||
      PST_IsArrayTypeExpr (info->symboltable, expr->operands))
    {
      Key type = PST_ExprType(info->symboltable, expr->operands);
      multiplier = PST_GetTypeSize(info->symboltable,
				  PST_GetTypeType(info->symboltable,
						  type));
    }
  else if (PST_IsFunctionTypeExpr (info->symboltable, expr->operands))
    {
      multiplier = IPA_POINTER_SIZE;
      assert(0);
    }
  else
    {
      Key type = PST_ExprType(info->symboltable, expr->operands);
      if (PST_GetTypeSize(info->symboltable, type) < IPA_POINTER_SIZE)
	return;
      multiplier = 1;
    }

  if (P_IsIntegralExpr(offset_expr))
    {
      offset_val = P_IntegralExprValue(offset_expr);
      IPA_bcg_add(info, bcg, multiplier * offset_val);
#if 0
      printf("L1OFF = %d\n", multiplier * offset_val);
#endif
    }
  else if (IPA_field_safety >= IPA_SAFETY_LEVEL2)
    {
      if (bcg->mode == B_BUILD)
	{
	  /* Reduce expr to realize the correct node */
	  IPA_bcg_reduce(info, bcg);
	  if (bcg->node->data.var_size <= IPA_POINTER_SIZE ||
	      bcg->offset == 0)
	    {
	      bcg->node->data.in_k_cycle = multiplier;
	    }
	  else
	    printf("L2MULT = %d offset %d [%s] \n", 
		   multiplier,
		   bcg->offset,
		   bcg->node->data.syminfo->symbol_name);
	}
#if 0
      printf("L2MULT = %d offset %d [%s]  ", 
	     multiplier,
	     bcg->offset,
	     bcg->node->data.syminfo->symbol_name);
      if (PST_IsArrayTypeExpr (info->symboltable, expr))
	printf("ARRAY");
      if (PST_IsPointerTypeExpr (info->symboltable, expr))
	printf("POINTER");
      printf("\n");
      DB_Pcode_Expr(expr); printf("\n");
      /*print_bcg(bcg);
	DB_Pcode_Expr(expr); printf("\n");
	printf("-----------\n");*/
#endif
    }

#if 0
  printf(" + "); print_bcg(bcg);
#endif
}

static void
IPA_set_nofield(buildcg_t *bcg)
{
  if (IPA_field_safety < IPA_SAFETY_LEVEL1)
    return;

#if 0
  if (!IPA_FLAG_ISSET(bcg->node->flags, IPA_CG_NODE_FLAGS_NOFIELD))
    {
      printf("NOFIELD %s %d \n",
	     bcg->node->data.syminfo->symbol_name,
	     bcg->node->data.var_id);
    }
#endif
  IPA_consg_set_nofield(bcg->node); 
}


int
IPA_is_ptr_invalidating (int opcode)
{
  switch (opcode)
    {
    case OP_div:
    case OP_mul:
    case OP_mod:
    case OP_neg:
    case OP_or:
    case OP_xor:
    case OP_and:
    case OP_rshft:
    case OP_lshft:
    case OP_inv:
    case OP_disj:
    case OP_conj:
    case OP_eq:
    case OP_ne:
    case OP_lt:
    case OP_le:
    case OP_ge:
    case OP_gt:
    case OP_not:
      return 1;
      break;
    default:
      break;
    }
  return 0;
}



buildcg_t *
IPA_BuildEqns_For_Expr (IPA_prog_info_t * info,
                        Expr root_expr, Expr var_expr)
{
  IPA_symbol_info_t *sym;
  Expr par_expr = NULL;
  Expr cur_expr = NULL;
  Expr prv_expr = NULL;
  buildcg_t *bcg = NULL;
  int check_for_array;

  assert (var_expr->opcode == OP_var ||
	  var_expr->opcode == OP_call ||
	  var_expr->opcode == OP_string);;
  par_expr = var_expr;


  do
    {
      prv_expr = cur_expr;
      cur_expr = par_expr;
      par_expr = IPA_Find_Significant_Parent_As_First_Operand (cur_expr);

#if 0
      printf("[%s:%d] : ", op_to_value[cur_expr->opcode],cur_expr->id);
      if (bcg)
	print_bcg(bcg);
      printf(" -> ");
#endif

      cur_expr->flags |= EF_VISITED;
      check_for_array = 0;
      switch (cur_expr->opcode)
        {
	case OP_string:
	  {
	    char name[256];
	    Key type;

#if 0
	    printf("SSTR %s\n", cur_expr->value.string);
#endif

	    assert(256 > sprintf (name, "SSTR_%.25s",
				  cur_expr->value.string));
	    type = PST_FindPointerToType (info->symboltable, 
					  PST_FindBasicType (info->symboltable,
							     BT_CHAR));
	    sym = IPA_Pcode_Add_Sym (info, name, IPA_symbol_tmpvarkey(),
				     type, (IPA_VARKIND_GLOBAL | 
				            IPA_VAR_KIND_STRING));
	    assert(sym->id > 0);
	    
	    bcg = IPA_buildcg_start(info, info->cur_fninfo, sym->id,
				    1, B_BUILD);
	    IPA_Expr_Addr_Eqn (info, cur_expr, bcg);
	    IPA_set_nofield(bcg);
	  }
	  break;

        case OP_var:
          /* VAR  ************************************************** */

	  /* THIS NEEDS TO KNOW WHAT VARIABLE SUBSCRIPT IT WANTS - JWP */
	  sym = IPA_Pcode_Add_Sym (info, 
				   cur_expr->value.var.name,
				   cur_expr->value.var.key,
				   IPA_ExprType(info, cur_expr),
				   IPA_VARKIND_LOCAL);
	  assert(sym->id > 0);

	    {
	      PSS_Def def = cur_expr->value.var.ssa;
	      int subscr = def ? def->name : 1;
	      
	      bcg = IPA_buildcg_start(info, info->cur_fninfo, sym->id,
				      subscr, B_BUILD);

	      if (def && PIPA_TEMP_TYPE (def->type))
		{
		  assert (bcg->node);
		  bcg->node->flags |= IPA_CG_NODE_FLAGS_TEMP;
		}
	    }
#if 0
	  printf("VAR %s\n", cur_expr->value.var.name);
#endif

#if SAFEOFFSET
	  if (sym->kind & IPA_VARKIND_FUNC)
	    {
	      /* No field sensitivity for functions */
	      IPA_set_nofield(bcg);
	    }
#endif

	  if (IPA_Pcode_IsFunctionType(info, IPA_ExprType(info, cur_expr)) &&
	      !IPA_Pcode_IsPointerType(info, IPA_ExprType(info, cur_expr)))
	    {
	      IPA_Expr_Addr_Eqn (info, cur_expr, bcg);
	    }
	  else
	    {
	      check_for_array = 1;
	    }
          break;

        case OP_addr:
          /* ADDR  ************************************************* */
          IPA_Expr_Addr_Eqn (info, cur_expr, bcg);
          check_for_array = 1;
          break;

        case OP_indr:
          /* DEREF ************************************************* */
          IPA_Expr_Deref_Eqn (info, cur_expr, bcg);
          check_for_array = 1;
          break;

        case OP_dot:
          /* OFFSET ************************************************* */
          IPA_Expr_Field_Eqn (info, cur_expr, bcg);
          check_for_array = 1;
          break;

        case OP_arrow:
          /* ARROW ************************************************* */
	  IPA_Expr_Arrow_Eqn (info, cur_expr, bcg);
          check_for_array = 1;
          break;

        case OP_index:
          /* Currently, arrays are treated as one big chunk, so
           *   the index is not processed 
           */
#if SAFEOFFSET
	  IPA_Expr_Handle_Addition(info, cur_expr, bcg);
#endif

          /* DEREF ************************************************* */
          IPA_Expr_Deref_Eqn (info, cur_expr, bcg);
          check_for_array = 1;
          break;

        case OP_add:
        case OP_sub:
	  /* Check for offset from addr of last fixed param
	     of a vararg routine */
	  if (bcg->node->data.var_id == info->cur_fninfo->lastfixed_id &&
	      bcg->status == B_ADDR)
	    {
	      int version = bcg->node->data.version;
	      
/* 	      printf("   - Found lastfixed offset, reseting to ellipse \n"); */
	      /* Need to "restart" at the ellipse var */
	      IPA_buildcg_free(bcg);
	      bcg = IPA_buildcg_start(info, info->cur_fninfo, 
				      info->cur_fninfo->ellipse_id, version, B_BUILD);
	      IPA_bcg_addrof(info, bcg);
	    }
#if SAFEOFFSET
	  else 
	    {
	      IPA_Expr_Handle_Addition(info, cur_expr, bcg);
	    }
#endif
          break;
	  
        case OP_call:
          /* Calls should have already been handled, so do nothing */
          break;

        case OP_cast:
          break;
        case OP_assign:
          break;
        case OP_compexpr:
          break;

	case OP_phi:
	  /* JWP - Added PHI consideration.  Assuming nothing needs to be done. */
	  break;

        default:
          /* Does the opcode invalidate any kind of pointer passing,
           * - stop forming this side of the equation and return 0
           */
          if (IPA_is_ptr_invalidating (cur_expr->opcode))
	    {
	      IPA_buildcg_kill(bcg);
	      return NULL;
	    }

          /* The rest should not be encountered */
          I_punt ("IPA_Expr_Build_Eqn: unexpected opcode %d\n",
                  cur_expr->opcode);
          break;
        }

      /* Arrays (i.e. int myarray[0]) are special.
       *  The value of myarray and &myarray are the same
       *   while assignment to myarray is invalid. myarray 
       *   simply represents the start of an allocated segment.
       *  To make all of this work, arrays are always preceeded
       *   by a dereference. 
       */
      if (cur_expr && check_for_array &&
          (IPA_Pcode_IsArrayType (info, IPA_ExprType(info, cur_expr)) ||
           IPA_Pcode_IsStructureArrayType (info, IPA_ExprType(info, cur_expr))))
        {
          IPA_Expr_Addr_Eqn (info, cur_expr, bcg);
	  bcg->isarray = 1;

#if SAFEOFFSET
	  if ((var_expr == cur_expr) &&
	      !IPA_Pcode_IsStructureArrayType(info, IPA_ExprType(info, cur_expr)))
	    {
	      /* This named var is an array. Turn it into a FI
		 node. Two problems:
		 1) This may be conservative for arrays whose
		    writes and uses are to fixed indices
		 2) This will preserve FS-ness of heap arrays
                    despite variablity in indices
                 This is done to preserve unification opportunties
                   for arrays of func pointers where the uses
                   are completely ambiguous */
	      IPA_set_nofield(bcg);
	    }
#endif
        }

#if 0
      print_bcg(bcg);
      printf("\n");
#endif

    }
  while (par_expr && cur_expr != root_expr);

  return bcg;
}

void
IPA_Add_Root_Points_To_Relations (IPA_prog_info_t * info, Expr expr)
{
  Expr lhs, rhs;
  Expr r_check;
  Expr l_var_expr, r_var_expr;
  IPA_funcsymbol_info_t *fninfo;
  int size;

  fninfo = info->cur_fninfo;
  if (!fninfo)
    I_punt ("IPA_Add_Root_Points_To_Relations: fn not found\n");
  lhs = expr->operands;
  rhs = expr->operands->sibling;

#if 0
  P_write_expr(stdout, expr, 10, NULL);
#endif

  switch (IPA_allow_pcode_expr)
    {
    case IPA_ALLOW_PTR_AGGR:
      if (!IPA_Pcode_IsPointerType (info, IPA_ExprType(info, lhs)) && 
	  !IPA_Pcode_IsPointerType (info, IPA_ExprType(info, rhs)) &&
	  !IPA_Pcode_IsArrayType (info, IPA_ExprType(info, rhs)) && 
	  !IPA_Pcode_IsStructureType (info, IPA_ExprType(info, rhs)) &&
	  !IPA_Pcode_IsFunctionType (info, IPA_ExprType(info, rhs)))
	return;
    case IPA_ALLOW_PTR_AGGR_INDIR:
      if (!IPA_Pcode_IsPointerType (info, IPA_ExprType(info, lhs)) && 
	  !IPA_Pcode_IsPointerType (info, IPA_ExprType(info, rhs)) &&
	  !IPA_Pcode_IsArrayType (info, IPA_ExprType(info, rhs)) && 
	  !IPA_Pcode_IsStructureType (info, IPA_ExprType(info, rhs)) &&
	  !IPA_Pcode_IsFunctionType (info, IPA_ExprType(info, rhs)) &&
	  !IPA_Expr_Has_Indr(lhs) && !IPA_Expr_Has_Indr(rhs))
	return;
    case IPA_ALLOW_ALL:
      break;
    default:
      assert(0);
      break;
    }

  /* PUT IN SHORT CIRCUIT HERE - JWP */
  if (rhs->opcode == OP_phi)
    {
      buildcg_t *l_bcg = NULL;
      buildcg_t *r_bcg = NULL;
      Expr phiOperand;
      int lsub;

      lsub = lhs->value.var.ssa->name;
      l_bcg = IPA_BuildEqns_For_Expr (info, lhs, lhs);

      assert (l_bcg);

      /* Generate an assignment edge from the lhs to each PHI operand */
      for (phiOperand = P_GetExprOperands (rhs); phiOperand;
	   phiOperand = P_GetExprNext (phiOperand))
	{
	  assert (phiOperand->opcode == OP_var);

	  if (phiOperand->value.var.ssa->name == lsub) continue;
	  
	  r_bcg = IPA_BuildEqns_For_Expr (info, rhs, phiOperand);

	  assert (r_bcg);

	  IPA_bcg_assign(info, l_bcg, r_bcg,
			 IPA_Pcode_sizeof (info, IPA_ExprType(info, rhs)));
	  
	  IPA_buildcg_free (r_bcg);
	}

      IPA_buildcg_free (l_bcg);
      
      return;
    }
  
  
  /*
   * The ideal rhs is:
   * (1) p = q = r; we want to have
   *     p = r and q = r
   * (2) p = (q, r); we want to have
   *     p = r;
   * (3) p = (int *) (q, r); we want to have
   *     p = r;
   * In other words, we want the lowest- and right-most operand as the right
   * hand side element.
   */
  r_check = rhs;
  while (rhs->opcode == OP_assign ||
         rhs->opcode == OP_compexpr || rhs->opcode == OP_cast)
    {
      switch (rhs->opcode)
        {
        case OP_assign:
          rhs = rhs->operands->sibling;
          break;
        case OP_compexpr:
          rhs = rhs->operands;
          while (rhs->next)
            rhs = rhs->next;
          break;
        case OP_cast:
          rhs = rhs->operands;
          break;
        default:
          break;
        }
    }

  if (!IPA_Expr_Has_Var_Involved (rhs))
    return;
  rhs = r_check;

  while (lhs->opcode == OP_compexpr)
    lhs = lhs->operands;

  assert (lhs && rhs);

  /* Find the central variable/call for lval
   */
  l_var_expr = lhs;
  while (l_var_expr && l_var_expr->opcode != OP_var)
    l_var_expr = l_var_expr->operands;
  assert (l_var_expr);

  /* Find the central variable/call for rval
   */
  r_var_expr = rhs;
  while (r_var_expr && 
	 r_var_expr->opcode != OP_var &&
         r_var_expr->opcode != OP_call &&
	 r_var_expr->opcode != OP_string)
    {
      /* Check for opcodes that invalidate a pointer assignment
       */
      if (IPA_is_ptr_invalidating (r_var_expr->opcode))
        return;

      switch (r_var_expr->opcode)
        {
        case OP_assign:
          r_var_expr = r_var_expr->operands->sibling;
          break;
        case OP_compexpr:
          r_var_expr = r_var_expr->operands;
          while (r_var_expr->next)
            r_var_expr = r_var_expr->next;
          break;
        default:
          r_var_expr = r_var_expr->operands;
          break;
        }
    }


  /* Determine the size of the assignment
   */
  size = IPA_Pcode_sizeof (info, IPA_ExprType(info, lhs));
#if 1
  if (size < IPA_POINTER_SIZE)
    P_warn("size less than %d\n",IPA_POINTER_SIZE);
#else
  assert(size >= IPA_POINTER_SIZE);
#endif
  if (size < IPA_POINTER_SIZE)
    size = IPA_POINTER_SIZE;

  if (r_var_expr)
    {
      buildcg_t *l_bcg = NULL;
      buildcg_t *r_bcg = NULL;

      DEBUG_IPA (2, printf ("RValue Var/Call %s\n",
			    r_var_expr->value.var.name););

#if 0
      printf("## ASSIGN ");
      DB_Pcode_Expr(lhs);
      printf(" = ");
      DB_Pcode_Expr(rhs);
      printf("\n");
#endif

      if (r_var_expr->opcode != OP_call)
        {
	  /* USE THE FOLLOWING FOR THE LOOP - JWP */
          if ((l_bcg = IPA_BuildEqns_For_Expr (info, lhs, l_var_expr)) &&
              (r_bcg = IPA_BuildEqns_For_Expr (info, rhs, r_var_expr)))
            {
              if (r_var_expr->opcode != OP_var &&
		  r_var_expr->opcode != OP_string)
                I_punt
                  ("IPA_Add_Root_Points_To_Relations: Unexpected opcode\n");

#if 0
              /* At this point we have a standard assignment of 
               *   a rhs to a lhs 
               */
	      printf("## ASSIGN ");
	      DB_Pcode_Expr(lhs);
	      printf(" %d = %d ",
		     l_bcg->node->data.var_id,
		     r_bcg->node->data.var_id);
	      DB_Pcode_Expr(rhs);
	      printf("\n");
#endif
	      IPA_bcg_assign(info, l_bcg, r_bcg,
			     IPA_Pcode_sizeof (info, IPA_ExprType(info, rhs)));
            }
	  IPA_buildcg_free(l_bcg);
	  IPA_buildcg_free(r_bcg);
        }
    }
#if 0
  else
    {
      I_warn ("IPA_Add_Root_Points_To_Relations: RValue Var CONSTANT\n");
      if (rhs)
	P_write_expr(stdout, rhs, 10, NULL);
    }
#endif
}


static List
IPA_Find_All_Expr_In_Exprs (IPA_prog_info_t * info, Expr expr,
			    List list)
{
  if (!expr)
    return list;

  if (IPA_exclude_zero_profile)
    {
      if (expr && expr->profile)
	{
	  if (expr->profile->count == 0)
	    {
	      /*printf("Skipping expr %d\n", expr->id);*/
	      return list;
	    }
	}
    }
  /*printf("expr %d\n", expr->id);*/
	  
  for (; expr; expr = expr->next)
    {
      /* Create the equation for the assigment
       */
      list = IPA_Find_All_Expr_In_Exprs (info, expr->operands, list);
      list = IPA_Find_All_Expr_In_Exprs (info, expr->sibling, list);

      switch (expr->opcode)
	{
	case OP_stmt_expr:
	  assert(0);
	  break;
	default:
	  list = List_insert_last(list, expr);
	  break;
	}
    }

  return list;
}

List
IPA_Find_All_Expr_In_Stmts (IPA_prog_info_t *info, Stmt stmt,
			    List list, buildcg_mode_t mode)
{
  while (stmt)
    {
      if (P_FindPragma (stmt->pragma, "pipa_ignore"))
	{
	  printf("FOUND PIPA IGNORE PRAGMA - SKIPPING\n");
	  stmt = stmt->lex_next;
	  continue;
	}

      if (P_FindPragma (stmt->pragma, "unreachable"))
	{
	  printf("FOUND UNREACABLE PRAGMA - SKIPPING\n");
	  stmt = stmt->lex_next;
	  continue;
	}

      if (IPA_exclude_zero_profile)
	{
	  if (stmt && stmt->profile)
	    {
	      if (stmt->profile->count == 0)
		{
		  /*printf("Skipping stmt\n");*/
		  stmt = stmt->lex_next;
		  continue;
		}
	    }
	}
 
      switch (stmt->type)
        {
        case ST_NOOP:
        case ST_CONT:
        case ST_BREAK:
        case ST_GOTO:
        case ST_ADVANCE:
        case ST_AWAIT:
          break;
        case ST_RETURN:
	  list = IPA_Find_All_Expr_In_Exprs (info, stmt->stmtstruct.ret, 
				      list);
          break;
        case ST_COMPOUND:
          /* Look through the local vars for any special types
           *  (e.g. statically sized arrays)
           */
	  if (mode == B_BUILD)
	    {
	      VarDcl varl;
	      List_start(stmt->stmtstruct.compound->var_list);
	      while ((varl = List_next(stmt->stmtstruct.compound->var_list)))
		{
		  Key scope_key;
		  scope_key = PST_GetStmtScope(info->symboltable, 
					       stmt);
		  IPA_BuildEqns_For_Var (info, varl, scope_key);
		}
	    }

          list = IPA_Find_All_Expr_In_Stmts (info, stmt->stmtstruct.compound->stmt_list, 
					     list, mode);
          break;
        case ST_IF:
          list = IPA_Find_All_Expr_In_Exprs (info, stmt->stmtstruct.ifstmt->cond_expr,
					     list);
          list = IPA_Find_All_Expr_In_Stmts (info, stmt->stmtstruct.ifstmt->then_block, 
					     list, mode);
          if (stmt->stmtstruct.ifstmt->else_block)
            list = IPA_Find_All_Expr_In_Stmts (info, stmt->stmtstruct.ifstmt->else_block,
					       list, mode);
          break;
        case ST_SWITCH:
          list = IPA_Find_All_Expr_In_Exprs (info, stmt->stmtstruct.switchstmt->expression,
					     list);
          list = IPA_Find_All_Expr_In_Stmts (info,
				      stmt->stmtstruct.switchstmt->switchbody, 
					     list, mode);
          break;
        case ST_SERLOOP:
	  {
	    SerLoop serloop;
	    serloop = stmt->stmtstruct.serloop;
	    if (serloop->init_expr)
	      list = IPA_Find_All_Expr_In_Exprs (info, serloop->init_expr, list);
	    if (serloop->cond_expr)
	      list = IPA_Find_All_Expr_In_Exprs (info, serloop->cond_expr, list);
	    if (serloop->iter_expr)
	      list = IPA_Find_All_Expr_In_Exprs (info, serloop->iter_expr, list);
	    list = IPA_Find_All_Expr_In_Stmts (info, serloop->loop_body, list, mode);
	  }
          break;
        case ST_EXPR:
          list = IPA_Find_All_Expr_In_Exprs (info, stmt->stmtstruct.expr, list);
          break;
        case ST_PARLOOP:
          {
            ParLoop p = P_GetStmtParLoop (stmt);
            Pstmt pstmt = P_GetParLoopPstmt (p);
            list = IPA_Find_All_Expr_In_Stmts (info, P_GetPstmtStmt (pstmt), list, mode);
          }
          break;
        case ST_BODY:
	  {
	    BodyStmt b = P_GetStmtBodyStmt (stmt);
	    list = IPA_Find_All_Expr_In_Stmts (info, P_GetBodyStmtStatement (b), list, mode);
	  }
	  break;
        case ST_EPILOGUE:
          list = IPA_Find_All_Expr_In_Stmts (info,
					     stmt->stmtstruct.epiloguestmt->statement,
					     list, mode);
          break;
	case ST_ASM:
	  /* 20040622SZU
	   * Temporary fix.
	   * Pflatten not handling assembly statements, leaving unexpected
	   * STMTEXPR later on.
	   * Should fix Pflatten eventually.
	   */
#if 0
          list = IPA_Find_All_Expr_In_Exprs (info,
				      stmt->stmtstruct.asmstmt->asm_string, list);
          list = IPA_Find_All_Expr_In_Exprs (info,
				      stmt->stmtstruct.asmstmt->asm_operands, list);
#endif
	  break;
        default:
          I_punt
            ("IPA_Find_All_Expr_In_Stmts: Invalid statement type");
        }
      stmt = stmt->lex_next;
    }
  return list;
}



List
IPA_Find_All_Expr_In_CFG (IPA_prog_info_t *info, PC_Graph cfg,
			  buildcg_mode_t mode)
{
  PC_Block bb;
  List list = NULL;

  for (bb = cfg->first_bb; bb; bb = bb->next)
    {
      _PC_ExprIter ei;
      Expr expr;
      
      for (expr = PC_ExprIterFirst (bb, &ei, 1); expr;
	   expr = PC_ExprIterNext (&ei, 1))
	{
	  if (expr->parentstmt &&
	      P_FindPragma (expr->parentstmt->pragma, "pipa_ignore"))
	    {
	      printf("FOUND PIPA IGNORE PRAGMA - SKIPPING\n");
	      continue;
	    }
	  
	  list = IPA_Find_All_Expr_In_Exprs(info, expr, list);
	}

    }

  if (mode == B_BUILD)
    IPA_Find_Special_Local_Vars_in_Stmts (info, cfg->func->stmt);


  return list;

}

void
IPA_Find_Special_Local_Vars_in_Stmts (IPA_prog_info_t *info, Stmt stmt)
{
  
  while (stmt)
    {
 
      switch (stmt->type)
        {
        case ST_NOOP:
        case ST_CONT:
        case ST_BREAK:
        case ST_GOTO:
        case ST_ADVANCE:
        case ST_AWAIT:
          break;
        case ST_RETURN:
          break;
        case ST_COMPOUND:
          /* Look through the local vars for any special types
           *  (e.g. statically sized arrays)
           */
	    {
	      VarDcl varl;
	      List_start(stmt->stmtstruct.compound->var_list);
	      while ((varl = List_next(stmt->stmtstruct.compound->var_list)))
		{
		  Key scope_key;
		  scope_key = PST_GetStmtScope(info->symboltable, 
					       stmt);
		  IPA_BuildEqns_For_Var (info, varl, scope_key);
		}
	    }

	  IPA_Find_Special_Local_Vars_in_Stmts (info, stmt->stmtstruct.compound->stmt_list);
          break;
        case ST_IF:
	  IPA_Find_Special_Local_Vars_in_Stmts (info, stmt->stmtstruct.ifstmt->then_block); 
          if (stmt->stmtstruct.ifstmt->else_block)
	    IPA_Find_Special_Local_Vars_in_Stmts (info,
						  stmt->stmtstruct.ifstmt->else_block);
          break;
        case ST_SWITCH:
	  IPA_Find_Special_Local_Vars_in_Stmts (info,
						stmt->stmtstruct.switchstmt->switchbody); 
          break;
        case ST_SERLOOP:
	  {
	    SerLoop serloop;
	    serloop = stmt->stmtstruct.serloop;
	    IPA_Find_Special_Local_Vars_in_Stmts (info, serloop->loop_body);
	  }
          break;
        case ST_EXPR:
          break;
        case ST_PARLOOP:
          {
            ParLoop p = P_GetStmtParLoop (stmt);
            Pstmt pstmt = P_GetParLoopPstmt (p);
            IPA_Find_Special_Local_Vars_in_Stmts (info, P_GetPstmtStmt (pstmt));
          }
          break;
        case ST_BODY:
	  {
	    BodyStmt b = P_GetStmtBodyStmt (stmt);
	    IPA_Find_Special_Local_Vars_in_Stmts (info, P_GetBodyStmtStatement (b));
	  }
	  break;
        case ST_EPILOGUE:
          IPA_Find_Special_Local_Vars_in_Stmts (info,
						stmt->stmtstruct.epiloguestmt->statement);
          break;
	case ST_ASM:
	  break;
        default:
          I_punt
            ("IPA_Find_Special_Local_Vars_in_Stmts: Invalid statement type");
        }
      stmt = stmt->lex_next;
    }

  return;
}


#if 0

void
IPA_Find_All_Assignments_In_Exprs (IPA_prog_info_t * info, Expr expr)
{
  for (; expr; expr = expr->next)
    {
      if (IPA_exclude_zero_profile)
	{
	  if (expr && expr->profile && expr->profile->count == 0)
	    continue;
	}

      IPA_Find_All_Assignments_In_Exprs (info, expr->operands);
      IPA_Find_All_Assignments_In_Exprs (info, expr->sibling);

      
      if (expr->opcode == OP_call)
        {
          IPA_Pcode_Add_CallSite (info, expr);
        }
      else if (expr->opcode == OP_assign)
        {
	  IPA_Add_Root_Points_To_Relations (info, expr);
        }
      else if ((expr->flags & EF_VISITED) == 0 &&
	       (expr->opcode == OP_indr ||
		expr->opcode == OP_arrow))
	{
	  Expr var_expr = expr;
	  buildcg_t *bcg;
	  while (var_expr->opcode != OP_var)
	    var_expr = var_expr->operands;

	  bcg = IPA_BuildEqns_For_Expr (info, expr, var_expr);
	  IPA_buildcg_free (bcg);	  
	}
    }
}

void
IPA_Find_All_Assignments_In_Stmts (IPA_prog_info_t * info, Stmt stmt)
{
  SerLoop serloop;
  VarDcl varl;

  while (stmt)
    {
      switch (stmt->type)
        {
        case ST_NOOP:
        case ST_CONT:
        case ST_BREAK:
        case ST_GOTO:
        case ST_ADVANCE:
        case ST_AWAIT:
          break;
        case ST_RETURN:
          if (stmt->stmtstruct.ret)
	    {
	      /* Allocation routines need to be handled in a special manner
	       */
	      if (!strcmp (info->cur_fninfo->func_name, "calloc") ||
		  !strcmp (info->cur_fninfo->func_name, "malloc") ||
		  !strcmp (info->cur_fninfo->func_name, "valloc"))
		{
		  IPA_bcg_formalreturn(info, info->cur_fninfo, 
				       stmt->stmtstruct.ret, 1);
		}
	      else
		{
		  IPA_bcg_formalreturn(info, info->cur_fninfo, 
				       stmt->stmtstruct.ret, 0);
		}

	      /* Process the rest of return (if anything) */
	      IPA_Find_All_Assignments_In_Exprs (info, stmt->stmtstruct.ret);
	    }
          break;
        case ST_COMPOUND:
          /* Look through the local vars for any special types
           *  (e.g. statically sized arrays)
           */
	  List_start(stmt->stmtstruct.compound->var_list);
	  while ((varl = List_next(stmt->stmtstruct.compound->var_list)))
	    {
	      Key scope_key;
	      scope_key = PST_GetStmtScope(info->symboltable, 
					   stmt);
              IPA_BuildEqns_For_Var (info, varl, scope_key);
            }

          IPA_Find_All_Assignments_In_Stmts (info, stmt->stmtstruct.compound->
                                             stmt_list);
          break;
        case ST_IF:
          IPA_Find_All_Assignments_In_Exprs (info, stmt->stmtstruct.ifstmt->
                                             cond_expr);
          IPA_Find_All_Assignments_In_Stmts (info, stmt->stmtstruct.ifstmt->
                                             then_block);
          if (stmt->stmtstruct.ifstmt->else_block)
            IPA_Find_All_Assignments_In_Stmts (info, stmt->stmtstruct.ifstmt->
                                               else_block);
          break;
        case ST_SWITCH:
          IPA_Find_All_Assignments_In_Exprs (info,
                                             stmt->stmtstruct.switchstmt->
                                             expression);
          IPA_Find_All_Assignments_In_Stmts (info,
                                             stmt->stmtstruct.switchstmt->
                                             switchbody);
          break;
        case ST_SERLOOP:
          serloop = stmt->stmtstruct.serloop;
          if (serloop->init_expr)
            IPA_Find_All_Assignments_In_Exprs (info, serloop->init_expr);
          if (serloop->cond_expr)
            IPA_Find_All_Assignments_In_Exprs (info, serloop->cond_expr);
          if (serloop->iter_expr)
            IPA_Find_All_Assignments_In_Exprs (info, serloop->iter_expr);
          IPA_Find_All_Assignments_In_Stmts (info, serloop->loop_body);
          break;
        case ST_EXPR:
          IPA_Find_All_Assignments_In_Exprs (info, stmt->stmtstruct.expr);
          break;
        case ST_PARLOOP:
          {
            ParLoop p = P_GetStmtParLoop (stmt);
            Pstmt pstmt = P_GetParLoopPstmt (p);
            IPA_Find_All_Assignments_In_Stmts (info, P_GetPstmtStmt (pstmt));
          }
          break;
        case ST_BODY:
	  {
	    BodyStmt b = P_GetStmtBodyStmt (stmt);
	    IPA_Find_All_Assignments_In_Stmts (info, P_GetBodyStmtStatement (b));
	  }
          return;
        case ST_EPILOGUE:
          IPA_Find_All_Assignments_In_Stmts (info,
                                             stmt->stmtstruct.epiloguestmt->
                                             statement);
          break;
	case ST_ASM:
	  assert(0);
          IPA_Find_All_Assignments_In_Exprs (info,
					     stmt->stmtstruct.asmstmt->asm_string);
          IPA_Find_All_Assignments_In_Exprs (info,
					     stmt->stmtstruct.asmstmt->asm_operands);
	  break;
        default:
          I_punt
            ("IPA_Find_All_Assignments_In_Stmts: Invalid statement type");
        }
      stmt = stmt->lex_next;
    }
}

#endif

/****************************************************************
 *
 *  This function adds the parameter information for this function
 *
 ****************************************************************/

void
IPA_Find_All_Params (IPA_prog_info_t * info, FuncDcl func)
{
  VarDcl param;
  buildcg_t *l_bcg;
  IPA_symbol_info_t *sym;
  int pcnt = 0;
  int fid = -1;
  int aid = -1;

  List_start(func->param);
  while ((param = List_next(func->param)))
    {
      if (!(sym = IPA_symbol_find (info, param->key)))
        {
          /* This can occur when a parameter is completely unused and therefore
           *  no access added it to the symbol table
           */
          sym = IPA_Pcode_Add_Sym (info, param->name, param->key,
				   param->type,
				   IPA_VARKIND_LOCAL);
        }

      /* Add in an equation fake_param = real_param 
       *   and then use the fake one in the interface
       */
      l_bcg = IPA_buildcg_start(info, info->cur_fninfo, sym->id, 1, B_BUILD);
      IPA_FLAG_SET(l_bcg->node->flags, IPA_CG_NODE_FLAGS_REALPARAM);

      if (IsVarargParam(param))
	{
#if 0
	  /* JWS: may not be available in old-style varargs.
	   * Looks like taking this assert out should be OK.
	   */
	  assert(aid != -1);
#endif
	  info->cur_fninfo->lastfixed_id = aid;
	  info->cur_fninfo->is_vararg = 1;
	  DEBUG_IPA(3, printf("VARARG FIXED %d\n", aid););
	}

      fid = IPA_bcg_formalparam(info, l_bcg, param->type, pcnt, IsVarargParam(param));
      aid = l_bcg->node->data.var_id;

      if (IsVarargParam(param))
	{
	  info->cur_fninfo->ellipse_id = aid;
	  DEBUG_IPA(3, printf("VARARG ELLIPSE %d\n", aid););
	}

      /* This provides an object for the argv and
       *   env parameters to main()
       */
      if (strcmp(func->name, "main") == 0)
	{
	  buildcg_t *r_bcg = NULL;

	  if (pcnt == 1)
	    {
	      sym = IPA_Pcode_Add_Sym (info, "MAIN_ARGV_INPUT",
				       IPA_symbol_tmpvarkey(), 
				       IPA_symbol_tmptypekey(),
				       IPA_VARKIND_LOCAL);
	      r_bcg = IPA_buildcg_start(info, info->cur_fninfo, 
					sym->id, 1, B_BUILD);
	      IPA_bcg_addrof(info, r_bcg);
      	    }
	  else if (pcnt == 2)
	    {
	      sym = IPA_Pcode_Add_Sym (info, "MAIN_ENV_INPUT",
				       IPA_symbol_tmpvarkey(), 
				       IPA_symbol_tmptypekey(),
				       IPA_VARKIND_LOCAL);
	      r_bcg = IPA_buildcg_start(info, info->cur_fninfo, 
					sym->id, 1, B_BUILD);
	      IPA_bcg_addrof(info, r_bcg);
	    }

	  if (r_bcg)
	    {
	      IPA_bcg_assign(info, l_bcg, r_bcg, IPA_POINTER_SIZE);
	      IPA_buildcg_free (r_bcg);
	    }
	}

      IPA_buildcg_free (l_bcg);
      pcnt++;
    }
}


/****************************************************************
 *
 *  This function handles static variable initialization
 *
 ****************************************************************/

void
IPA_eqn_from_exprs (IPA_prog_info_t * info, Expr lhs, Expr rhs, int deref_rhs)
{
  Expr l_var_expr, r_var_expr;
  buildcg_t *l_bcg = NULL;
  buildcg_t *r_bcg = NULL;

  /* Find lhs variable */
  l_var_expr = lhs;
  while (l_var_expr->opcode != OP_var)
    l_var_expr = l_var_expr->operands;
  if (!l_var_expr)
    I_punt ("lhs variable expr not found\n");

  /* Find rhs variable */
  r_var_expr = rhs;
  while (r_var_expr->opcode != OP_var &&
	 r_var_expr->opcode != OP_string)
    r_var_expr = r_var_expr->operands;
  if (!r_var_expr)
    I_punt ("rhs variable expr not found\n");

  if ((l_bcg = IPA_BuildEqns_For_Expr (info, lhs, l_var_expr)) &&
      (r_bcg = IPA_BuildEqns_For_Expr (info, rhs, r_var_expr)))
    {
      IPA_bcg_assign(info, l_bcg, r_bcg, 
		     IPA_Pcode_sizeof(info, IPA_ExprType(info, rhs)));
    }
  IPA_buildcg_free (l_bcg);
  IPA_buildcg_free (r_bcg);
}

void
IPA_follow_lhs_rhs (IPA_prog_info_t * info,
		    Expr lhs_expr,
                    Init rhs)
{
  Expr new_expr = NULL;
  Init set = NULL;
  Key lhs_type_key;

  /* Nothing left to initialize */
  if (!rhs)
    return;

  lhs_type_key = IPA_ExprType(info, lhs_expr);

  if (IPA_Pcode_IsArrayType(info, lhs_type_key))
    {
      DEBUG_IPA (2, printf ("Init Array \n"););

      /* For each initializer for array indices */
      for (set = rhs->set; set; set = set->next)
        {
	  /* (LHS) INDEX  = (RHS) SET   */
          new_expr = P_NewExprWithOpcode (OP_index);

	  P_SetExprOperands(new_expr, P_CopyExpr (lhs_expr));
	  new_expr->operands->parentexpr = new_expr;

          IPA_follow_lhs_rhs (info, new_expr, set);
	  
          P_RemoveExpr (new_expr);
        }
    }
  else if (PST_GetTypeBasicType(info->symboltable, 
				lhs_type_key) & BT_STRUCT)
    {
      StructDcl st;
      Field field;
      SymTabEntry entry;
      Key lhs_typetype;

      lhs_typetype = PST_GetTypeType(info->symboltable, lhs_type_key);
      entry = PST_GetSymTabEntry (info->symboltable, lhs_typetype);
      st = P_GetSymTabEntryStructDcl(entry);

      assert (st);
      DEBUG_IPA (2, printf ("Init Struct [%s]\n", st->name););

      for (field = st->fields, set = rhs->set;
           field && set; field = field->next, set = set->next)
        {
	  /* (LHS) DOT FIELD = (RHS) SET */
          DEBUG_IPA (2, printf ("  Init Field [%s - %d:%d]\n",
                                field->name, field->offset, 
				IPA_Pcode_sizeof(info, field->type)););

          new_expr = P_NewExprWithOpcode (OP_dot);
	  P_SetExprOperands(new_expr, P_CopyExpr (lhs_expr));
	  new_expr->operands->parentexpr = new_expr;
          new_expr->value.string = strdup(field->name);

          IPA_follow_lhs_rhs (info, new_expr, set);

          P_RemoveExpr (new_expr);
        }
    }
  else if (PST_GetTypeBasicType(info->symboltable, 
				lhs_type_key) & BT_UNION)
    {
      UnionDcl un;
      Field field;
      SymTabEntry entry;
      Key lhs_typetype;

      lhs_typetype = PST_GetTypeType(info->symboltable, lhs_type_key);
      entry = PST_GetSymTabEntry (info->symboltable, lhs_typetype);
      un = P_GetSymTabEntryUnionDcl(entry);
      assert (un);
      DEBUG_IPA (2, printf ("Init Union [%s]\n", un->name););

      /* Can initialize only the first member of union */
      set = rhs->set;
      field = un->fields;
      if (set)
	{
	  DEBUG_IPA (2, printf ("  Init Un Field [%s - %d:%d]\n",
				field->name, field->offset, 
				IPA_Pcode_sizeof(info, field->type)););

          new_expr = P_NewExprWithOpcode (OP_dot);
	  P_SetExprOperands(new_expr, P_CopyExpr (lhs_expr));
	  new_expr->operands->parentexpr = new_expr;
          new_expr->value.string = strdup(field->name);

          IPA_follow_lhs_rhs (info, new_expr, set);

          P_RemoveExpr (new_expr);	  
	}
    }
  else
    {
      if (!rhs->expr)
        I_punt ("IPA_follow_lhs_rhs: No expression\n");

      if (IPA_Pcode_IsContentPointerType (info, lhs_type_key) &&
          IPA_Expr_Has_Var_Involved (rhs->expr))
        {
          DEBUG_IPA (2, printf ("  Pointer init\n"););
          IPA_eqn_from_exprs (info, lhs_expr, rhs->expr, 1);
        }
    }
}

void
IPA_BuildEqns_For_Var (IPA_prog_info_t * info, VarDcl var, Key scope_key)
{
  Expr new_expr = NULL;

#if 0
  ipa_scope_key = scope_key;
#endif

  DEBUG_IPA (2, printf("EQN FOR VAR: %s\n",var->name););
  new_expr = P_NewExprWithOpcode (OP_var);
  P_SetExprVarName (new_expr, strdup(var->name));
  P_SetExprVarKey (new_expr, var->key);
  new_expr->type = var->type;

  /* Build the initialization equations */
  IPA_follow_lhs_rhs (info, new_expr, var->init);

  P_RemoveExpr (new_expr);
}

/****************************************************************
 *
 *  This is the driver function
 *
 ****************************************************************/

void
DB_Pcode_Expr(Expr expr)
{
  P_write_expr (stdout, expr, 0, NULL);
  fflush(stdout);
}

static int
IsVarargParam (VarDcl param)
{
  /* In ANSI-C, "..." is a vararg parameter */
  if (!strcmp (param->name, "__builtin_impact_ellipsis") ||
      !strcmp (param->name, "__builtin_va_alist"))
    return 1;
  return 0;
}

#if 0
static int
IsVarargCallee (FuncDcl fn)
{
  VarDcl param;

  List_start(fn->param);
  while ((param = List_next(fn->param)))
    {
      if (IsVarargParam (param))
	return 1;
    }
  return 0;
}
#endif

void
IPA_BuildEqns_For_Func (FuncDcl func, IPA_prog_info_t * info)
{
  char *buffer = NULL;
  char *filename = NULL;
  IPA_symbol_info_t *sym;
  buildcg_t *bcg;

/*   printf(" FUNC [%s]\n", func->name); */
 
  /* A symbol and node should exist for evey GLOBAL */
  sym = IPA_symbol_add (info, info->globals, func->name,
			func->key, IPA_VARKIND_FUNC,
			func->type);
  bcg = IPA_buildcg_start(info, info->globals, sym->id, 1, B_BUILD);
  IPA_buildcg_free (bcg);
  
  /* If the function is not defined and linked in 
     then don't add it at all
   */
  if (!func->stmt)
    {
/*       printf(" FUNC DNE [%s]\n", func->name); */
      return;
    }

  /*
   * Add in the symbol for the function
   */
  if (strcmp ("main", func->name) == 0)
    {
      /* Build callsite for main */
      IPA_callsite_new_prog (info,
			     info->globals,
			     func->name, func->key);
    }

  {
    /* JWS 20040717: Add callsites for C++ constructor functions */

    Pragma p;

    for (p = func->pragma; (p = P_FindPragma (p, "Cattr")); 
	 p = P_GetPragmaNext (p))
      {
	if (!strcmp (P_GetExprString (P_GetPragmaExpr (p)),
		     "__constructor__"))
	  break;
      }

    if (p)
      {
/* 	printf ("Entering function %s() as a constructor.\n", */
/* 		func->name); */
	IPA_callsite_new_prog (info,
			       info->globals,
			       func->name, func->key);
      }
  }

  if (func->filename != NULL)
    {
      buffer = strdup (func->filename);
      buffer[strlen (buffer) - 1] = 0;
      filename = strdup (buffer + 1);
    }
  else
    {
      filename = strdup("unknown");
    }

#if 0
  IPA_symbol_add (info, info->globals, func->name,
		  func->key, IPA_VARKIND_FUNC,
		  func->type);
#endif
  
  info->cur_fninfo = IPA_funcsymbol_add (info, func->key, 
					 filename, func->name);
  if (info->in_library && !info->cur_fninfo)
    return;
  info->cur_fninfo->consg = IPA_cg_cgraph_new(info->cur_fninfo);
  info->cur_fninfo->func_key = func->key;
  free (buffer);
  free (filename);

  if (info->in_library)
    info->cur_fninfo->from_library = 1;
  else
    info->cur_fninfo->from_library = 0;    


  if (P_FindPragma (func->pragma, "pipa_heap_alloc"))
    {
/*       printf("[%s] heap_alloc\n",func->name); */
      info->cur_fninfo->is_heap_alloc = 1;
    }
  if (P_FindPragma (func->pragma, "pipa_heap_free"))
    {
/*       printf("[%s] heap_free\n",func->name); */
      info->cur_fninfo->is_heap_free = 1;
    }
  if (P_FindPragma (func->pragma, "pipa_blockcopy"))
    {
/*       printf("[%s] blockcopy\n",func->name); */
      info->cur_fninfo->is_blockcopy = 1;
    }
#if 0
  if (strstr(func->name,"_error_exit_") ||
      strstr(func->name,"_out_of_memory_"))
   {
      printf("[%s] NoExit\n",func->name);
      info->cur_fninfo->is_noexit = 1;
      /* Normal calls to noexit routines are largely ignored
	 so add a top-level call to facilitate topological sorting */
      IPA_callsite_new_prog (info,
			     info->globals,
			     func->name, func->key);
    }
#endif

#if 0
  /* moved to find all params */
  if (IsVarargCallee(func))
    info->cur_fninfo->is_vararg = 1;
#endif

  /* Find accesses and form bulk of equations
   */
  IPA_Find_All_Params (info, func);
#if 0
  IPA_Find_All_Assignments_In_Stmts (info, func->stmt);
#endif

  {
    List exprlist = NULL;
    Expr expr;

    /* Read expressions from the AST if no flow-sensitivity is specified. */
    if (IPA_flow_sensitive_type == IPA_FLOW_NONE)
      exprlist = IPA_Find_All_Expr_In_Stmts (info, func->stmt, NULL, B_BUILD);
    
    /* If flow sensitivity is specified, read expressions from the CFG. */
    else
      {
	PC_Graph cfg;
	PSS_BaseTbl LocalVars;

	/* Generate the control-flow graph */
	cfg = PC_Function (func, 0, 0);

	/* Generate SSA from the CFG */
	if ((LocalVars = PSS_ComputeSSA (cfg)))
	  {
	    if (IPA_flow_sensitive_type == IPA_FLOW_FULL)
	      PSS_ComputeLifetimes (cfg, LocalVars, FULL);
	    else if (IPA_flow_sensitive_type == IPA_FLOW_LOOP)
	      PSS_ComputeLifetimes (cfg, LocalVars, LOOP);
	    else if (IPA_flow_sensitive_type == IPA_FLOW_PHIS)
	      PSS_ComputeLifetimes (cfg, LocalVars, MERGE_PHIS);
	    else if (IPA_flow_sensitive_type == IPA_FLOW_DJLT)
	      PSS_ComputeLifetimes (cfg, LocalVars, DISJOINT_LTS);
	    else
	      I_punt ("IPA_BuildEqns_For_Func: Invalid flow-sensitivity type."
		      " [%d]",
		      IPA_flow_sensitive_type);

	    PSS_NormalizeSubscrs (LocalVars, 1);
	    
	    exprlist = IPA_Find_All_Expr_In_CFG (info, cfg, B_BUILD);
	  }
	else
	  exprlist =
	    IPA_Find_All_Expr_In_Stmts (info, func->stmt, NULL, B_BUILD);
      }
    
    List_start(exprlist);
    while ((expr = List_next(exprlist)))
      {
	expr->flags &= (~EF_VISITED);
      }

    List_start(exprlist);
    while ((expr = List_next(exprlist)))
      {
	DEBUG_IPA(3, printf("EXPR %d",expr->id);
		  if (expr->opcode == OP_var)
		     printf (" VAR %s",expr->value.var.name);
		  printf("\n"););

	if (expr->parentexpr == NULL &&
	    expr->parentstmt && expr->parentstmt->type == ST_RETURN)
	  {
	    /* Attach return path */
	    if (!strcmp (info->cur_fninfo->func_name, "calloc") ||
		!strcmp (info->cur_fninfo->func_name, "malloc") ||
		!strcmp (info->cur_fninfo->func_name, "valloc") ||
		info->cur_fninfo->is_heap_alloc)
	      {
		/* Allocation routines need to be handled in 
		 *  a special manner
		 */
/* 		printf("STMT return in %s is proto-HEAP\n", info->cur_fninfo->func_name); */
		IPA_bcg_formalreturn(info, info->cur_fninfo, 
				     expr, 1);
	      }
	    else
	      {
		IPA_bcg_formalreturn(info, info->cur_fninfo, 
				     expr, 0);
	      }
	  }

	if (expr->opcode == OP_call)
	  {
	    DEBUG_IPA(3, printf("   CALLSITE %d\n",expr->id););
	    IPA_Pcode_Add_CallSite (info, expr);
	  }
	else if (expr->opcode == OP_assign)
	  {
	    DEBUG_IPA(3, printf("   ROOT %d\n",expr->id););
	    IPA_Add_Root_Points_To_Relations (info, expr);
	  }
	else if ((expr->flags & EF_VISITED) == 0 &&
		 (expr->opcode == OP_indr ||
		  expr->opcode == OP_arrow ||
		  expr->opcode == OP_index))
	  {
	    Expr var_expr = expr;
	    buildcg_t *bcg;

	    DEBUG_IPA(3, printf("   NO ASSIGN EXPR %d\n",expr->id););

	    while (var_expr && var_expr->opcode != OP_var)
	      var_expr = var_expr->operands;
	    
	    if (var_expr)
	      {
		bcg = IPA_BuildEqns_For_Expr (info, expr, var_expr);
		IPA_buildcg_free (bcg);	  
	      }
	  }
      }
    
    List_reset(exprlist);
  }
}

