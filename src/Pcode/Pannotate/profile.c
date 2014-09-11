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
 *      File:   profile.c
 *      Author: Teresa Johnson and Wen-mei Hwu
 *      Copyright (c) 1995 Teresa Johnson, Wen-mei Hwu
 *                      and The Board of Trustees of the University of Illinois.
 *                      All rights reserved.
 *      The University of Illinois software License Agreement
 *      specifies the terms and conditions for redistribution.
\*****************************************************************************/

#include <config.h>
#include "annotate.h"
#include <library/string_symbol.h>

#define AM_VERSION 3

#if 0
static void
Create_Dep_ExprPragmas_Expr (Expr expr)
{
  Expr sib;

  /*if (expr == 0) Punt("Create_Dep_ExprPragmas_Expr : nil input"); */
  if (expr == 0)
    return;

  while (expr)
    {
      if (expr->acc && expr->acc->dep_prag_num &&
	  ((!assume_C_semantics && expr->acc->var_expr == expr) ||
	   (assume_C_semantics && expr->acc->acc_expr == expr)))
	{
	  AddExprPragma (expr, "\"DEP\"",
			 NewIntExpr (expr->acc->dep_prag_num));
	}
      if (expr->acc2 && expr->acc2->dep_prag_num &&
	  ((!assume_C_semantics && expr->acc2->var_expr == expr) ||
	   (assume_C_semantics && expr->acc2->acc_expr == expr)))
	{
	  AddExprPragma (expr, "\"DEP\"",
			 NewIntExpr (expr->acc2->dep_prag_num));
	}
      for (sib = expr->operands; sib; sib = sib->sibling)
	{
	  Create_Dep_ExprPragmas_Expr (sib);
	}
      expr = expr->next;
    }
}

static void
Create_Dep_ExprPragmas_Stmt (Stmt stmt)
{
  /*if (stmt == 0) Punt("Create_Dep_ExprPragmas_Stmt : nil input"); */
  if (stmt == 0)
    return;

  while (stmt)
    {
      switch (stmt->type)
	{
	case ST_RETURN:
	  Create_Dep_ExprPragmas_Expr (stmt->stmtstruct.ret);
	  break;
	case ST_COMPOUND:
	  Create_Dep_ExprPragmas_Stmt (stmt->stmtstruct.compound->stmt_list);
	  break;
	case ST_IF:
	  Create_Dep_ExprPragmas_Expr (stmt->stmtstruct.ifstmt->cond_expr);
	  Create_Dep_ExprPragmas_Stmt (stmt->stmtstruct.ifstmt->then_block);
	  Create_Dep_ExprPragmas_Stmt (stmt->stmtstruct.ifstmt->else_block);
	  break;
	case ST_SWITCH:
	  Create_Dep_ExprPragmas_Expr (stmt->stmtstruct.switchstmt->
				       expression);
	  Create_Dep_ExprPragmas_Stmt (stmt->stmtstruct.switchstmt->
				       switchbody);
	  break;
	case ST_PSTMT:
	  Create_Dep_ExprPragmas_Stmt (stmt->stmtstruct.pstmt->stmt);
	  break;
	case ST_MUTEX:
	  Create_Dep_ExprPragmas_Expr (stmt->stmtstruct.mutex->expression);
	  Create_Dep_ExprPragmas_Stmt (stmt->stmtstruct.mutex->statement);
	  break;
	case ST_COBEGIN:
	  Create_Dep_ExprPragmas_Stmt (stmt->stmtstruct.cobegin->statements);
	  break;
	case ST_PARLOOP:
	  Create_Dep_ExprPragmas_Expr (stmt->stmtstruct.parloop->
				       iteration_var);
	  Create_Dep_ExprPragmas_Expr (stmt->stmtstruct.parloop->init_value);
	  Create_Dep_ExprPragmas_Expr (stmt->stmtstruct.parloop->final_value);
	  Create_Dep_ExprPragmas_Expr (stmt->stmtstruct.parloop->incr_value);
	  Create_Dep_ExprPragmas_Stmt (stmt->stmtstruct.parloop->pstmt->stmt);
	  break;
	case ST_SERLOOP:
	  Create_Dep_ExprPragmas_Stmt (stmt->stmtstruct.serloop->loop_body);
	  Create_Dep_ExprPragmas_Expr (stmt->stmtstruct.serloop->cond_expr);
	  Create_Dep_ExprPragmas_Expr (stmt->stmtstruct.serloop->init_expr);
	  Create_Dep_ExprPragmas_Expr (stmt->stmtstruct.serloop->iter_expr);
	  break;
	case ST_EXPR:
	  Create_Dep_ExprPragmas_Expr (stmt->stmtstruct.expr);
	  break;
	case ST_BODY:
	  Create_Dep_ExprPragmas_Stmt (stmt->stmtstruct.bodystmt->statement);
	  break;
	case ST_EPILOGUE:
	  Create_Dep_ExprPragmas_Stmt (stmt->stmtstruct.epiloguestmt->
				       statement);
	  break;
	default:
	  break;
	}
      stmt = stmt->lex_next;
    }
}

void
Create_Dep_ExprPragmas (FuncDcl fn)
{
  if (fn == 0)
    I_punt ("Create_Dep_ExprPragmas : nil input");

  if (DEMO_OUTPUT)
    DD_Print_Entire_Access_Table (Flog, fn);
  Create_Dep_ExprPragmas_Stmt (fn->stmt);
}


P_Attr *
P_new_attr (int id, char *name, int max_field)
{
  P_Attr *attr;
  int i;

  attr = (P_Attr *) ALLOCATE (P_Attr);
  attr->op_id = id;
  attr->name = name;
  attr->max_field = max_field;
  attr->next_attr = NIL;
  attr->next_attr2 = NIL;
  attr->field =
    (P_Attr_Field **) malloc (sizeof (P_Attr_Field *) * max_field);
  for (i = 0; i < max_field; i++)
    attr->field[i] = ALLOCATE (P_Attr_Field);
  attr->rank = 0;
  return attr;
}

void
P_set_int_attr_field (P_Attr * attr, int field_num, int value)
{
  if (field_num > attr->max_field)
    I_punt ("Tried to set non-existent field in attribute");
  attr->field[field_num]->type = P_INT;
  attr->field[field_num]->value.i = value;
}

void
P_set_float_attr_field (P_Attr * attr, int field_num, float value)
{
  if (field_num > attr->max_field)
    I_punt ("Tried to set non-existent field in attribute");
  attr->field[field_num]->type = P_FLOAT;
  attr->field[field_num]->value.f = value;
}

void
P_set_string_attr_field (P_Attr * attr, int field_num, char *value)
{
  if (field_num > attr->max_field)
    I_punt ("Tried to set non-existent field in attribute");
  attr->field[field_num]->type = P_STRING;
  attr->field[field_num]->value.s = value;
}

void
P_set_label_attr_field (P_Attr * attr, int field_num, char *value)
{
  if (field_num > attr->max_field)
    I_punt ("Tried to set non-existent field in attribute");
  attr->field[field_num]->type = P_LABEL;
  attr->field[field_num]->value.l = value;
}

P_Attr *
P_find_attr (P_Attr * attr, char *name)
{
  char *attr_name;
  P_Attr *ptr;

  if (name == NULL)
    I_punt ("P_find_attr: name is NULL");

  for (ptr = attr; ptr != NULL; ptr = ptr->next_attr)
    {
      attr_name = ptr->name;
      if (*attr_name != *name)	/* quick check of first letter */
	continue;
      if (!strcmp (attr_name, name))
	return (ptr);
    }

  return (NULL);
}

static void Pannotate_line_numbers_Stmt (Stmt stmt);
static void Pannotate_Stmt (Stmt stmt);
static void Pannotate_line_numbers_Expr (Expr expr);
static void Pannotate_Expr (Expr expr);
static void Pannotate_read_annot (char *fn_name, int i);
static void Pannotate_read_func (int version, int num_fn_attr,
				 int num_cb_attr, int num_op_attr);
static void Pannotate_read_index (void);

void
Pannotate_Init (void)
{
  /* init the hash table to hold the index file info and read into it */
  P_init_string_hash ();
  Pannotate_read_index ();
}

void
Pannotate_Finish (void)
{
  P_delete_string_hash ();
}

void
Pannotate_Func (FuncDcl fn)
{
  int n, N;

  N = P_get_num_positions (fn->name);

  for (n = 0; n < N; n++)
    {
      /* init the hash tables to hold the attributes */
      P_init_hash ();

      /* read the Attribute Manager file into the hash tables, etc. */
      Pannotate_read_annot (fn->name, n);

      /* Combine like attrs */
      P_combine_attrs ();

      /* associate line numbers with attributes */
      Pannotate_line_numbers_Stmt (fn->stmt);

      DD_Build_Access_Table_Func (fn);

      /* Annotate all statements */
      Pannotate_Stmt (fn->stmt);

      /* delete all the hash tables and attribute structures that were used */
      P_delete_hash ();

    }
}

static void
Pannotate_line_numbers_Stmt (Stmt stmt)
{
  if (stmt == 0)
    return;

  while (stmt)
    {
      switch (stmt->type)
	{
	case ST_RETURN:
	  Pannotate_line_numbers_Expr (stmt->stmtstruct.ret);
	  break;
	case ST_COMPOUND:
	  Pannotate_line_numbers_Stmt (stmt->stmtstruct.compound->stmt_list);
	  break;
	case ST_IF:
	  Pannotate_line_numbers_Expr (stmt->stmtstruct.ifstmt->cond_expr);
	  Pannotate_line_numbers_Stmt (stmt->stmtstruct.ifstmt->then_block);
	  Pannotate_line_numbers_Stmt (stmt->stmtstruct.ifstmt->else_block);
	  break;
	case ST_SWITCH:
	  Pannotate_line_numbers_Expr (stmt->stmtstruct.switchstmt->
				       expression);
	  Pannotate_line_numbers_Stmt (stmt->stmtstruct.switchstmt->
				       switchbody);
	  break;
	case ST_PSTMT:
	  Pannotate_line_numbers_Stmt (stmt->stmtstruct.pstmt->stmt);
	  break;
	case ST_MUTEX:
	  Pannotate_line_numbers_Expr (stmt->stmtstruct.mutex->expression);
	  Pannotate_line_numbers_Stmt (stmt->stmtstruct.mutex->statement);
	  break;
	case ST_COBEGIN:
	  Pannotate_line_numbers_Stmt (stmt->stmtstruct.cobegin->statements);
	  break;
	case ST_PARLOOP:
	  Pannotate_line_numbers_Expr (stmt->stmtstruct.parloop->
				       iteration_var);
	  Pannotate_line_numbers_Expr (stmt->stmtstruct.parloop->init_value);
	  Pannotate_line_numbers_Expr (stmt->stmtstruct.parloop->final_value);
	  Pannotate_line_numbers_Expr (stmt->stmtstruct.parloop->incr_value);
	  Pannotate_line_numbers_Stmt (stmt->stmtstruct.parloop->pstmt->stmt);
	  break;
	case ST_SERLOOP:
	  Pannotate_line_numbers_Stmt (stmt->stmtstruct.serloop->loop_body);
	  Pannotate_line_numbers_Expr (stmt->stmtstruct.serloop->cond_expr);
	  Pannotate_line_numbers_Expr (stmt->stmtstruct.serloop->init_expr);
	  Pannotate_line_numbers_Expr (stmt->stmtstruct.serloop->iter_expr);
	  break;
	case ST_EXPR:
	  Pannotate_line_numbers_Expr (stmt->stmtstruct.expr);
	  break;
	case ST_BODY:
	  Pannotate_line_numbers_Stmt (stmt->stmtstruct.bodystmt->statement);
	  break;
	case ST_EPILOGUE:
	  Pannotate_line_numbers_Stmt (stmt->stmtstruct.epiloguestmt->
				       statement);
	  break;
	default:
	  break;
	}
      stmt = stmt->lex_next;
    }
}

static void
Pannotate_line_numbers_Expr (Expr expr)
{
  Pragma pragma;
  P_Attr *this, *attr_list;
  P_attr_hash_node *node;
  Expr sib;

  if (expr == 0)
    return;

  while (expr)
    {
      /* get the list of attributes specified in the Attribute
         Manager file for this Expression by looking at all of its Pragmas */
      for (pragma = expr->pragma; pragma; pragma = pragma->next)
	{
	  if (strcmp ("\"DEP\"", pragma->specifier))
	    continue;
	  if (pragma->expr->opcode != OP_int)
	    I_punt ("invalid DEP Pragma expr");
	  attr_list = P_get_attr (pragma->expr->value.scalar);

	  /* for each op attribute that was in the Attribute Manager
	     file, set the line number of the corresponding op node */
	  for (this = attr_list; this; this = this->next_attr)
	    {
	      node = P_get_hash_node_by_op (this->op_id);
	      node->lineno = expr->parentstmt->lineno;
	    }
	}
      for (sib = expr->operands; sib; sib = sib->sibling)
	{
	  Pannotate_line_numbers_Expr (sib);
	}
      expr = expr->next;
    }
}

static void
Pannotate_Stmt (Stmt stmt)
{
  if (stmt == 0)
    return;

  while (stmt)
    {
      switch (stmt->type)
	{
	case ST_RETURN:
	  Pannotate_Expr (stmt->stmtstruct.ret);
	  break;
	case ST_COMPOUND:
	  Pannotate_Stmt (stmt->stmtstruct.compound->stmt_list);
	  break;
	case ST_IF:
	  Pannotate_Expr (stmt->stmtstruct.ifstmt->cond_expr);
	  Pannotate_Stmt (stmt->stmtstruct.ifstmt->then_block);
	  Pannotate_Stmt (stmt->stmtstruct.ifstmt->else_block);
	  break;
	case ST_SWITCH:
	  Pannotate_Expr (stmt->stmtstruct.switchstmt->expression);
	  Pannotate_Stmt (stmt->stmtstruct.switchstmt->switchbody);
	  break;
	case ST_PSTMT:
	  Pannotate_Stmt (stmt->stmtstruct.pstmt->stmt);
	  break;
	case ST_MUTEX:
	  Pannotate_Expr (stmt->stmtstruct.mutex->expression);
	  Pannotate_Stmt (stmt->stmtstruct.mutex->statement);
	  break;
	case ST_COBEGIN:
	  Pannotate_Stmt (stmt->stmtstruct.cobegin->statements);
	  break;
	case ST_PARLOOP:
	  Pannotate_Expr (stmt->stmtstruct.parloop->iteration_var);
	  Pannotate_Expr (stmt->stmtstruct.parloop->init_value);
	  Pannotate_Expr (stmt->stmtstruct.parloop->final_value);
	  Pannotate_Expr (stmt->stmtstruct.parloop->incr_value);
	  Pannotate_Stmt (stmt->stmtstruct.parloop->pstmt->stmt);
	  break;
	case ST_SERLOOP:
	  Pannotate_Stmt (stmt->stmtstruct.serloop->loop_body);
	  Pannotate_Expr (stmt->stmtstruct.serloop->cond_expr);
	  Pannotate_Expr (stmt->stmtstruct.serloop->init_expr);
	  Pannotate_Expr (stmt->stmtstruct.serloop->iter_expr);
	  break;
	case ST_EXPR:
	  Pannotate_Expr (stmt->stmtstruct.expr);
	  break;
	case ST_BODY:
	  Pannotate_Stmt (stmt->stmtstruct.bodystmt->statement);
	  break;
	case ST_EPILOGUE:
	  Pannotate_Stmt (stmt->stmtstruct.epiloguestmt->statement);
	  break;
	default:
	  break;
	}
      stmt = stmt->lex_next;
    }
}

static void
Pannotate_Expr (Expr expr)
{
  Pragma pragma, prag;
  P_Attr *this, *attr_list;
  P_attr_hash_node *node;
  char name[20];
  char null_string[10];
  Expr sib, new_expr, prag_expr;
  int i, num;

  if (expr == 0)
    return;

  sprintf (null_string, "\"NULL\"");

  while (expr)
    {
      /* get the list of attributes specified in the Attribute
         Manager file for this Expression by looking at all of its Pragmas */
      for (pragma = expr->pragma; pragma; pragma = pragma->next)
	{
	  if (strcmp ("\"DEP\"", pragma->specifier))
	    continue;
	  if (pragma->expr->opcode != OP_int)
	    I_punt ("invalid DEP Pragma expr");
	  attr_list = P_get_attr (pragma->expr->value.scalar);

	  /* for each op attribute that was in the Attribute Manager
	     file, insert it into this Pcode expr pragma list */
	  for (this = attr_list; this; this = this->next_attr)
	    {
	      sprintf (name, "\"%s\"", this->name);
	      prag = FindExprPragma (expr, name);

#if 0
	      /* Need to take out all old prags on a prepass */
	      if (prag)
		RemoveExprPragma(expr,prag);
#endif

	      fprintf (Fpcode_position, "%12d", expr->parentstmt->lineno);
	      if (expr->acc)
		fprintf (Fpcode_position, " %s", expr->acc->text);
	      else if (expr->acc2)
		fprintf (Fpcode_position, " %s", expr->acc2->text);
	      else
		fprintf (Fpcode_position, " ??");
	      fprintf (Fpcode_position, " %s %d", name, this->max_field);
	      prag_expr = 0;
	      for (i = 0; i < this->max_field; i++)
		{
		  if (!this->field[i])
		    {
		      new_expr = NewStringExpr (strdup (null_string));
		      fprintf (Fpcode_position, " NULL");
		    }
		  else
		    {
		      switch (this->field[i]->type)
			{
			case P_INT:
			  if (!(strcmp (this->name, "mdp") ||
				strncmp (this->name, "dep_", 4)) && !i)
			    {
			      if (this->field[i]->value.i == -1 ||
				  !(node =
				    P_get_hash_node_by_op (this->field[i]->
							   value.i)))
				{
				  num = -1;
				  new_expr = NewIntExpr (num);
				  fprintf (Fpcode_position, " %12d", num);
				}
			      else
				{
				  new_expr = NewIntExpr (node->lineno);
				  fprintf (Fpcode_position, " %12d",
					   node->lineno);
				}
			    }
			  else
			    {
			      new_expr = NewIntExpr (this->field[i]->value.i);
			      fprintf (Fpcode_position, " %12d",
				       this->field[i]->value.i);
			    }
			  break;
			case P_FLOAT:
			  new_expr = NewRealExpr (this->field[i]->value.f);
			  fprintf (Fpcode_position, " %12f",
				   this->field[i]->value.f);
			  break;
			case P_STRING:
			  new_expr =
			    NewStringExpr (strdup (this->field[i]->value.s));
			  fprintf (Fpcode_position, " %12s",
				   this->field[i]->value.s);
			  break;
			case P_LABEL:
			  new_expr =
			    NewStringExpr (strdup (this->field[i]->value.l));
			  fprintf (Fpcode_position, " %12s",
				   this->field[i]->value.l);
			  break;
			}
		    }
		  if (!prag_expr)
		    prag_expr = new_expr;
		  else
		    AddNextOperand (prag_expr, new_expr);
		}

	      AddExprPragma (expr, strdup (name), prag_expr);

	      if (this->rank)
		fprintf (Fpcode_position, " %d", this->rank);
	      fprintf (Fpcode_position, "\n");
	    }
	}
      for (sib = expr->operands; sib; sib = sib->sibling)
	Pannotate_Expr (sib);
      expr = expr->next;
    }
}

static void
Pannotate_read_annot (char *fn_name, int i)
{
  int version, num_fn_attr, num_cb_attr, num_op_attr;
  char name[500];
  float weight;

  rewind (Fannot);
  fscanf (Fannot, "# Attribute Manager file --- Version %d", &version);
  if (version > AM_VERSION)
    I_punt ("Version %d of annot file not supported yet", version);
  fseek (Fannot, P_get_position (fn_name, i), 0);
  fscanf (Fannot, "begin _%s %d %d %d %f\n", name, &num_fn_attr,
	  &num_cb_attr, &num_op_attr, &weight);
  if (strcmp (fn_name, name))
    I_punt ("Function not found in Attribute Manager file");
  Pannotate_read_func (version, num_fn_attr, num_cb_attr, num_op_attr);
}

static void
Pannotate_read_func (int version, int num_fn_attr, int num_cb_attr,
		     int num_op_attr)
{
  int i, err, op_id, id, num_fields, field_num, rank;
  char type, name[500], fn_name[500], line[500];
  long value_i;
  float value_f;
  char value_s[500];
  P_Attr *attr;

  /* Currently can't pass back function attrs */
  for (i = 0; i < num_fn_attr; i++)
    {
      fgets (line, 500, Fannot);
    }

  /* Currently can't pass back cb attrs */
  for (i = 0; i < num_cb_attr; i++)
    {
      fgets (line, 500, Fannot);
    }

  for (i = 0; i < num_op_attr; i++)
    {
      err = fscanf (Fannot, "%d %s", &op_id, name);
      if (err < 2)
	I_punt ("Incorrect format of annot file (op_id name)");
      if (version == 1)
	{
	  attr = P_new_attr (op_id, strdup (name), 1);
	  err = fscanf (Fannot, " %ld", &value_i);
	  if (err < 1)
	    I_punt ("Incorrect format of annot file (value ver 1)");
	  P_set_int_attr_field (attr, 0, value_i);
	}
      else			/* version == 2 or 3 */
	{
	  if (version == 3)
	    {
	      err = fscanf (Fannot, " %d", &num_fields);
	      if (err != 1)
		I_punt
		  ("Incorrect format of annot file: num fields not specified");
	    }
	  else
	    num_fields = 1;
	  attr = P_new_attr (op_id, strdup (name), num_fields);
	  for (field_num = 0; field_num < num_fields; field_num++)
	    {
	      err = fscanf (Fannot, " %c", &type);
	      if (err != 1)
		I_punt ("Incorrect format of annot file (missing attr type)");
	      switch (type)
		{
		case 'i':
		  err = fscanf (Fannot, " %ld", &value_i);
		  P_set_int_attr_field (attr, field_num, value_i);
		  break;
		case 'f':
		  err = fscanf (Fannot, " %f", &value_f);
		  P_set_float_attr_field (attr, field_num, value_f);
		  break;
		case 's':
		  err = fscanf (Fannot, " %s", value_s);
		  P_set_string_attr_field (attr, field_num, value_s);
		  break;
		case 'l':
		  err = fscanf (Fannot, " %s", value_s);
		  P_set_label_attr_field (attr, field_num, value_s);
		  break;
		case 'N':
		  err = fscanf (Fannot, "ULL");
		  attr->field[field_num] = 0;
		  err = 1;
		  break;
		default:
		  I_punt ("Attribute type '%c' not known (id %d)\n",
			  type, op_id);
		  break;
		}
	      if (err < 1)
		I_punt ("Incorrect format of annot file (missing val)");
	    }
	}

      fgets (line, 500, Fannot);

      err = sscanf (line, " %d Rank %d\n", &id, &rank);
      if (err > 0)
	{
	  P_hash_insert_attr (attr, id);
	  if (err > 1)
	    {
	      attr->rank = rank;
	    }
	}
    }
  fscanf (Fannot, "end _%s\n\n", fn_name);
}

static void
Pannotate_read_index (void)
{
  int err, version, n;
  long position;
  char name[500];

  err =
    fscanf (Fannot_index, "# Attribute Manager index file --- Version %d\n",
	    &version);
  if (err < 1)
    I_punt ("Incorrect format of index file");
  while ((err = fscanf (Fannot_index, "_%s %d", name, &n)) != -1)
    {
      if (err < 2)
	I_punt ("Incorrect format of index file");
      for (; n; n--)
	{
	  err = fscanf (Fannot_index, " 0x%lx", &position);
	  if (err < 1)
	    I_punt ("Incorrect format of index file");
	  P_insert_position (name, position);
	}
      fscanf (Fannot_index, "\n");
    }
}
#endif
