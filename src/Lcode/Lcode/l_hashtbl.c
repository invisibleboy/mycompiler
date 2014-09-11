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
/*===========================================================================
 *      File :          l_hashtbl.c
 *      Description :   Repair has table entry when changing cb labels.
 *      Creation Date : October, 1990
 *      Author :        Pohua Chang, Scott Mahlke, Wen-mei Hwu
 *
 *==========================================================================*/
/* 10/29/02 REK Adding config.h */
#include <config.h>
#include <Lcode/l_main.h>

#define ERR     stderr
#undef DEBUG

/*=======================================================================*/
/*
 *      Global variables
 */
/*=======================================================================*/

char *L_hash_tbl_current_fn = "???";
int L_hash_tbl_n_change;
int L_hash_tbl_n_allocated;
int L_hash_tbl_allocated;
L_Hash_Tbl_Map *L_hash_tbl_change = NULL;

/*=======================================================================*/
/*
 *      Functions to remap hash table jump targets
 */
/*=======================================================================*/

void
L_define_fn_name (char *name)
{
  if (L_debug_hash_table_repair)
    printf ("L_define_fn_name: define fn %s\n", name);
  L_hash_tbl_current_fn = name;
  L_hash_tbl_n_change = 0;
  L_hash_tbl_allocated = 0;
  L_hash_tbl_n_allocated = 0;
  if (L_hash_tbl_change != NULL)
    {
      free (L_hash_tbl_change);
      L_hash_tbl_change = NULL;
    }
}

static void
L_realloc_change ()
{
  int i, t_allocated;
  L_Hash_Tbl_Map *t_change;

  /* allocate new structure */
  t_allocated = L_hash_tbl_n_allocated * 2;
  t_change =
    (L_Hash_Tbl_Map *) malloc (sizeof (L_Hash_Tbl_Map) * t_allocated);

  /* copy old entries */
  for (i = 0; i < L_hash_tbl_n_allocated; i++)
    {
      t_change[i].old_cb = L_hash_tbl_change[i].old_cb;
      t_change[i].new_cb = L_hash_tbl_change[i].new_cb;
    }

  /* free old structure */
  free (L_hash_tbl_change);

  /* reset global vars */
  L_hash_tbl_n_allocated = t_allocated;
  L_hash_tbl_change = t_change;
}

/****************************************************************************
 *
 * routine: L_change_cb_id()
 * purpose: Change the cb id in the hash table
 * returns: 1 if a change was made, 0 otherwise.
 * modified: 10/16/96 - Bob McGowan - added the return value.
 * note:
 *-------------------------------------------------------------------------*/
int
L_change_cb_id (int old_id, int new_id)
{
  int i;
  int change = 0;

  if (L_debug_hash_table_repair)
    printf ("L_change_cb_id:\told %d\tnew %d\n", old_id, new_id);
  if (old_id == new_id)
    return change;

  /* SAM for compatibility with old routines */
  L_set_func_modified_jump_table_flag (L_fn);

  if (L_region_hash_table_management == 0)
    {
      if (!L_hash_tbl_allocated)
        {
          L_hash_tbl_n_allocated = L_fn->n_cb;
          L_hash_tbl_change =
            (struct impact_map *) malloc (sizeof (struct impact_map) *
                                   L_hash_tbl_n_allocated);
          L_hash_tbl_allocated = 1;
        }
      /*
       *        redirection.
       */
      for (i = 0; i < L_hash_tbl_n_change; i++)
        if (old_id == L_hash_tbl_change[i].new_cb)
          L_hash_tbl_change[i].new_cb = new_id;
      /*
       *        new setting.
       */
      for (i = 0; i < L_hash_tbl_n_change; i++)
        if (old_id == L_hash_tbl_change[i].old_cb)
          break;
      if (i == L_hash_tbl_n_change)
        {
          i = L_hash_tbl_n_change++;
          if (i >= L_hash_tbl_n_allocated)
            L_realloc_change ();
          L_hash_tbl_change[i].old_cb = old_id;
          L_hash_tbl_change[i].new_cb = new_id;
        }
      else
        {
          L_hash_tbl_change[i].new_cb = new_id;
        }
    }
  else
    {
      /* Presumably this module was called from the region manager,  */
      /* so that any hash table changes will be reflected by placing */
      /* attributes on the function.  The region manager will ensure */
      /* that the hash tbl is updated properly.                */
      L_Attr *attr = L_new_attr ("hash_tbl_updt", 2);
      L_set_int_attr_field (attr, 0, old_id);
      L_set_int_attr_field (attr, 1, new_id);

      L_fn->attr = L_concat_attr (L_fn->attr, attr);
      change = 1;
    }
  return change;
}

static void
L_repair_label (L_Expr * expr)
{
  if (expr == NULL)
    return;
  L_repair_label (expr->A);
  L_repair_label (expr->B);
  if (expr->type == L_EXPR_LABEL)
    {
      char *label;
      char fn[512], line[512];
      int cb_id;
      label = expr->value.l;
      if (M_is_cb_label (label, fn, &cb_id))
        {
          int new_id, i;
          /*
           *  Lcode->Lopti cb translation.
           */
          if (strcmp (fn, L_hash_tbl_current_fn))
            {
              fprintf (ERR, "[%s, %d]\n", fn, cb_id);
              L_punt ("L_repair_label: "
                      "hash table def must immediately follow function");
            }
          new_id = cb_id;
          /*
           *  See if another translation is necessary.
           */
          for (i = 0; i < L_hash_tbl_n_change; i++)
            {
              if (new_id == L_hash_tbl_change[i].old_cb)
                {
                  new_id = L_hash_tbl_change[i].new_cb;
                  break;
                }
            }
          if (L_debug_hash_table_repair)
            {
              fprintf (ERR, "# label: fn=%s, cb=%d, bb=%d\n", fn, cb_id,
                       new_id);
            }
          /*
           *  Update the name.
           */
          if (new_id != cb_id)
            {
              M_cb_label_name (fn, new_id, line, 512);
              expr->value.l = L_add_string (L_string_table, line);
            }
        }
    }
}

/*
 *      Rename all cb labels
 */
void
L_repair_hashtbl (L_Data * data)
{
  if (data == NULL)
    return;
  if (data->address != NULL)
    L_repair_label (data->address);
  if (data->value != NULL)
    L_repair_label (data->value);
}

/* SAM 7-96 additions */
/*=======================================================================*/
/*=======================================================================*/
/*
 *      New jump table (or hash table, what ever you want to call them)
 *      manipulation routines.
 */
/*=======================================================================*/
/*=======================================================================*/

/*--------------------------------------------------------------------------*/
/*
 *      Basic jump tbl name manipulation routines
 */

int
L_extract_jump_table_id (char *name)
{
  int tbl_id;

  if (!M_is_jumptbl_label (name, L_fn->name, &tbl_id))
    L_punt ("L_extract_jump_table_id: illegal name %s", name);
  return (tbl_id);
}

/* Return 1 if name seems like the name of a valid jump table */
int
L_valid_jump_table_name (char *name)
{
  int tbl_id;

  if (!M_is_jumptbl_label (name, L_fn->name, &tbl_id))
    return (0);

  if (tbl_id < 0)
    return (0);
  if (tbl_id > L_JUMPTBL_MAX_ID)
    return (0);

  return (1);
}

char *
L_construct_jump_table_name (int id)
{
  char tmp[1024], *name;

  M_jumptbl_label_name (L_fn->name, id, tmp, 1024);
  name = L_add_string (L_string_table, tmp);

  return (name);
}

/* Format of a jump tbl name is L_JUMPTBL_OLDSTYLE_BASE_NAME%d, %d is the id */
int
L_oldstyle_extract_jump_table_id (char *name)
{
  int base_len, id;
  char *ptr;

  base_len = strlen (L_JUMPTBL_OLDSTYLE_BASE_NAME);

  if (strlen (name) <= base_len)
    L_punt ("L_extract_jump_table_id: illegal name %s", name);

  ptr = name;
  ptr += base_len;

  id = atoi (ptr);
  return (id);
}

/* Return 1 if name seems like the name of a valid jump table */
int
L_oldstyle_valid_jump_table_name (char *name)
{
  int tbl_id;

  /* Base name should be L_JUMPTBL_OLDSTYLE_BASE_NAME */
  if (strncmp (name, L_JUMPTBL_OLDSTYLE_BASE_NAME,
               strlen (L_JUMPTBL_OLDSTYLE_BASE_NAME)))
    return (0);

  /* tbl id should be non-negative and not larger than L_JUMPTBL_MAX_ID */
  tbl_id = L_oldstyle_extract_jump_table_id (name);

  if (tbl_id < 0)
    return (0);
  if (tbl_id > L_JUMPTBL_MAX_ID)
    return (0);

  return (1);
}


/*--------------------------------------------------------------------------*/
/*
 *      Jump tbl attr manipulation functions
 *      These should only be called after jump tbl info is setup!
 */

/* Function has jump table info installed (ie setup has been run) */
int
L_func_has_jump_table_info (L_Func * fn)
{
  L_Attr *attr;

  attr = L_find_attr (fn->attr, L_JUMPTBL_FUNC_ATTR);

  return (attr != NULL);
}

/* Ret the number of jump tables the func has */
int
L_num_jump_tables (L_Func * fn)
{
  L_Attr *attr;

  attr = L_find_attr (fn->attr, L_JUMPTBL_FUNC_ATTR);

  if (attr != NULL)
    return ((int) attr->field[0]->value.i);

  /* If its NULL, we have an Lcode file which the jump table info is 
     not setup */
  else
    {
      L_punt ("L_num_jump_tables: jump table info is not set up!!");
      return (-1);
    }
}

/* Ret 1 if the func has any jump tables (and hence jrgs) */
int
L_func_has_jump_tables (L_Func * fn)
{
  return (L_num_jump_tables (fn) > 0);
}

int
L_func_needs_jump_table_renaming (L_Func * fn)
{
  L_Attr *attr;

  /* if no setup done, then need renaming */
  if (!(attr = L_find_attr (fn->attr, L_JUMPTBL_FUNC_ATTR)))
    return (1);
  /* if setup done, but old-style names still in place */
  if (!L_find_string_attr_field (attr, L_JUMPTBL_RENAMED_STRING))
    return (1);
  else
    return (0);
}

/* Extract jump table name from the attribute of a jrg */
char *
L_jump_table_name (L_Oper * jrg)
{
  L_Attr *attr;

  attr = L_find_attr (jrg->attr, L_JUMPTBL_OP_ATTR);

  if (attr == NULL)
    return (NULL);
  else
    return (attr->field[0]->value.l);
}

/* Get the next unused jump table id */
int
L_new_jump_table_id (L_Func * fn)
{
  L_Attr *attr;
  int max_id;

  attr = L_find_attr (fn->attr, L_JUMPTBL_FUNC_ATTR);
  if (attr == NULL)
    L_punt ("L_new_jump_table_id: jump tbl info not set up correctly!");

  max_id = (int) attr->field[1]->value.i;

  return (max_id + 1);
}

/* Insert/Replace L_JUMPTBL_OP_ATTR onto the oper */
void
L_install_jumptbl_op_attr (L_Oper * oper, char *tbl_name)
{
  L_Attr *attr;

  attr = L_find_attr (oper->attr, L_JUMPTBL_OP_ATTR);

  if (attr != NULL)
    {
      L_set_label_attr_field (attr, 0, tbl_name);
    }
  else
    {
      attr = L_new_attr (L_JUMPTBL_OP_ATTR, 1);
      L_set_label_attr_field (attr, 0, tbl_name);
      oper->attr = L_concat_attr (oper->attr, attr);
    }
}

/* Insert/Replace L_JUMPTBL_FUNC_ATTR onto the func */
void
L_install_jumptbl_func_attr (L_Func * fn, int num_tbls, int max_id)
{
  L_Attr *attr;

  attr = L_find_attr (fn->attr, L_JUMPTBL_FUNC_ATTR);

  /* Renaming done automatically, so always put in attr.  Only older LC
     files will have an attr w/o L_JUMPTBL_RENAMED_STRING set */
  if (attr != NULL)
    {
      L_set_int_attr_field (attr, 0, num_tbls);
      L_set_int_attr_field (attr, 1, max_id);
      L_set_string_attr_field (attr, 2, L_JUMPTBL_RENAMED_STRING);
    }
  else
    {
      attr = L_new_attr (L_JUMPTBL_FUNC_ATTR, 2);
      L_set_int_attr_field (attr, 0, num_tbls);
      L_set_int_attr_field (attr, 1, max_id);
      L_set_string_attr_field (attr, 2, L_JUMPTBL_RENAMED_STRING);
      fn->attr = L_concat_attr (fn->attr, attr);
    }


}

/* For a L_JUMPTBL_FUNC_ATTR, increment num_tbls and reset max_id (if necc) 
   to account for a new jump table */
void
L_update_func_attr_for_new_jump_table (L_Func * fn, int new_id)
{
  L_Attr *attr;

  attr = L_find_attr (fn->attr, L_JUMPTBL_FUNC_ATTR);
  if (attr == NULL)
    L_punt ("L_update_func_attr_for_new_jump_table: "
            "jump tbl info not set up correctly!");

  /* update number of tbls */
  attr->field[0]->value.i++;

  /* update max_id */
  if (new_id > (int) attr->field[1]->value.i)
    attr->field[1]->value.i = (ITintmax) new_id;
}

void
L_update_func_attr_for_deleted_jump_table (L_Func * fn)
{
  L_Attr *attr;

  attr = L_find_attr (fn->attr, L_JUMPTBL_FUNC_ATTR);
  if (attr == NULL)
    L_punt ("L_update_func_attr_for_deleted_jump_table: "
            "jump tbl info not set up correctly!");

  /* update number of tbls */
  if (attr->field[0]->value.i-- == 0)
    L_punt ("L_update_func_attr_for_deleted_jump_table: "
            "number of tables cannot be negative");
}

void
L_update_func_attr_for_renamed_tables (L_Func * fn)
{
  L_Attr *attr;

  attr = L_find_attr (fn->attr, L_JUMPTBL_FUNC_ATTR);
  /* if attr not there, renamed flag will be put in when the attr is created */
  if (attr == NULL)
    return;

  L_set_string_attr_field (attr, 2, L_JUMPTBL_RENAMED_STRING);
}

/*--------------------------------------------------------------------------*/
/*
 *      Marking jumptbls as requiring modification when printing
 */

void
L_set_func_modified_jump_table_flag (L_Func * fn)
{
  fn->jump_tbl_flags = L_SET_BIT_FLAG (fn->jump_tbl_flags,
                                       L_JUMPTBL_MODIFIED_CONTROL);
}

void
L_set_func_new_jump_table_flag (L_Func * fn)
{
  fn->jump_tbl_flags = L_SET_BIT_FLAG (fn->jump_tbl_flags,
                                       L_JUMPTBL_NEW_TABLES);
}

void
L_set_func_renamed_jump_table_flag (L_Func * fn)
{
  fn->jump_tbl_flags = L_SET_BIT_FLAG (fn->jump_tbl_flags,
                                       L_JUMPTBL_RENAMED_TABLES);
}

int
L_jump_tables_have_changes (L_Func * fn)
{
  return (L_EXTRACT_BIT_VAL (fn->jump_tbl_flags,
                             L_JUMPTBL_MODIFIED_CONTROL |
                             L_JUMPTBL_NEW_TABLES |
                             L_JUMPTBL_RENAMED_TABLES));
}


/*--------------------------------------------------------------------------*/
/*
 *      Jump table renaming.  This is for compatibility of old Lcode files
 *      with the new jump table name format.
 */

char *
L_create_renamed_label (char *name)
{
  int id;
  char *new_name;

  id = L_oldstyle_extract_jump_table_id (name);
  new_name = L_construct_jump_table_name (id);
  return (new_name);
}

void
L_rename_label_operand (L_Operand * operand)
{
  operand->value.l = L_create_renamed_label (operand->value.l);
}

void
L_rename_jump_table_labels (L_Func * fn)
{
  int i, renamed, jrg_found;
  char *tbl_name, *new_name;
  L_Cb *cb;
  L_Oper *oper;
  L_Operand *src;
  L_Attr *attr;


  /* Handle the special case up front where there is a jumptbl func attr,
     but no tables.  Still need to set the "renamed" flag for these. */
  attr = L_find_attr (fn->attr, L_JUMPTBL_FUNC_ATTR);
  if ((attr != NULL) && (attr->field[0]->value.i == 0))
    {
      if (!L_find_string_attr_field (attr, L_JUMPTBL_RENAMED_STRING))
        L_update_func_attr_for_renamed_tables (fn);
      else
        L_warn ("L_rename_jump_table_labels: func unnecessarily called");
      return;

    }


  /* Either setup not run or there are jump tables, so scan code and rename
     oldstyle jump table labels to new style */
  renamed = 0;
  jrg_found = 0;
  for (cb = fn->first_cb; cb != NULL; cb = cb->next_cb)
    {
      for (oper = cb->first_op; oper != NULL; oper = oper->next_op)
        {
          /* Rename label src operands */
          for (i = 0; i < L_max_src_operand; i++)
            {
              src = oper->src[i];
              if (!L_is_label (src))
                continue;
              if (L_oldstyle_valid_jump_table_name (src->value.l))
                {
                  L_rename_label_operand (src);
                  renamed = 1;
                }
            }
          /* Rename label attr on load ops */
          if (L_general_load_opcode (oper) &&
              L_EXTRACT_BIT_VAL (oper->flags, L_OPER_LABEL_REFERENCE))
            {
              attr = L_find_attr (oper->attr, "label");
              if (attr &&
                  L_oldstyle_valid_jump_table_name (attr->field[0]->value.l))
                {
                  L_rename_label_operand (attr->field[0]);
                  renamed = 1;
                }
            }
          /* Rename tbl_name attr on jump_rg ops */
          else if (L_register_branch_opcode (oper))
            {
              jrg_found = 1;
              tbl_name = L_jump_table_name (oper);
              if (tbl_name && L_oldstyle_valid_jump_table_name (tbl_name))
                {
                  new_name = L_create_renamed_label (tbl_name);
                  L_install_jumptbl_op_attr (oper, new_name);
                  renamed = 1;
                }
            }
        }
    }

  /* Simple sanity check */
  if ((renamed) && (!jrg_found))
    L_punt
      ("L_rename_jump_table_labels: renaming occured but no jrg's in funct");

  /* Set the flag so datalist is regen'd at print time to account for rename */
  if (renamed)
    {
      L_set_func_renamed_jump_table_flag (fn);
      L_update_func_attr_for_renamed_tables (fn);
    }
}


/*---------------------------------------------------------------------------*/
/*
 *    Set up jump table info for a function.  Info is maintained as attributes
 *    on each jump_rg and 1 on the function.
 */

/* Get the jumptbl name from the load/address-calc op */
L_Operand *
L_extract_jump_table_name_operand (L_Oper * op)
{
  int i;
  L_Operand *src, *tbl_name;

  if (op == NULL)
    return (NULL);

  tbl_name = NULL;
  for (i = 0; i < L_max_src_operand; i++)
    {
      src = op->src[i];
      if (L_is_label (src) && L_valid_jump_table_name (src->value.l))
        {
          tbl_name = src;
          break;
        }
    }

  return (tbl_name);
}

Set
L_get_reaching_defs (L_Operand * operand, L_Oper * oper, int *df_done)
{
  L_Oper *def;
  L_Cb *cb;
  Set rdefs = NULL;

  /* Look in the current block */
  def = L_prev_def (operand, oper);

  if (def != NULL)
    {
      rdefs = Set_add (rdefs, def->id);
    }
  else
    {
      if (!*df_done)
        {
          *df_done = 1;
          L_do_flow_analysis (L_fn, REACHING_DEFINITION);
        }
      cb = L_oper_hash_tbl_find_cb (L_fn->oper_hash_tbl, oper->id);
      rdefs = L_get_cb_RIN_defining_opers (cb, operand);
    }

  return (rdefs);
}

int
L_find_jrg_label_for_DEF (Set * op_visited, Set * op_set, L_Oper * oper,
                          int *df_done, int *name_found)
{
  L_Operand *name_operand, *src;
  int j, found = 0;

  name_operand = L_extract_jump_table_name_operand (oper);
  if (name_operand != NULL)
    {
      *op_set = Set_add (*op_set, oper->id);
      *op_visited = Set_add (*op_visited, oper->id);
      *name_found = 1;
      return (1);
    }
  else if (Set_in (*op_visited, oper->id))
    {
      /* Here we err on the aggressive side and return 1 indicating found
         when a cycle is detected in the use-def chains */
      return (1);
    }
  else if (L_general_move_opcode (oper) ||
           L_general_load_opcode (oper) || L_int_arithmetic_opcode (oper))
    {
      for (j = 0; j < L_max_src_operand; j++)
        {
          src = oper->src[j];
          if (!L_is_variable (src))
            continue;
          *op_visited = Set_add (*op_visited, oper->id);
          found = L_find_jrg_label_for_USE (op_visited, op_set, src, oper,
                                            df_done, name_found);
          if (found)
            {
              *op_set = Set_add (*op_set, oper->id);
              return (1);
            }
        }
    }

  *op_visited = Set_add (*op_visited, oper->id);
  return (0);
}

int
L_find_jrg_label_for_USE (Set * op_visited, Set * op_set, L_Operand * operand,
                          L_Oper * oper, int *df_done, int *name_found)
{
  L_Oper *def;
  int i, num_defs, *def_buf, found = 1;
  Set def_set;

  def_set = L_get_reaching_defs (operand, oper, df_done);
  num_defs = Set_size (def_set);
  if (num_defs == 0)
    return (0);
  def_buf = (int *) Lcode_malloc (sizeof (int) * num_defs);
  (void) Set_2array (def_set, def_buf);

  for (i = 0; i < num_defs; i++)
    {
      def = L_oper_hash_tbl_find_oper (L_fn->oper_hash_tbl, def_buf[i]);
      found &= L_find_jrg_label_for_DEF (op_visited, op_set, def, df_done,
                                         name_found);
      *op_visited = Set_add (*op_visited, oper->id);
      if (found)
        *op_set = Set_add (*op_set, def->id);
    }

  Set_dispose (def_set);
  Lcode_free (def_buf);

  return (found);
}

/* Return the ld which defines src[0].  If the tbl_name is explicitly given
   as an operand, then just return the ld.  If not, get the relevant addr calc
   ops which give then tbl name.  Note that this routine may not be as general
   as it needs to be, but my thinking is jrgs are regular in structure so no
   sense in going overboard here. */
Set
L_find_jump_table_address_ops (L_Cb * cb, L_Oper * jrg, int *df_done)
{
  Set address_ops = NULL;
  Set visited_ops = NULL;
  int name_found = 0;
  int found;

  address_ops = NULL;
  if (!L_is_variable (jrg->src[0]))
    L_punt ("L_find_jump_table_address_ops: illegal src[0] for jrg %d",
            jrg->id);

  /* `name_found' is used to indicate that atleast 1 reference to a jump table
     was encountered.  Whereas, `found' indicates along all paths leading to
     the jrg, jrg->src[0] has a jump_table name reaching.  Note that in the
     case of loops, `found' is aggressively assumed in the cycle, so 
     `name_found' is used to ensure there is an out-of-loop value containing a 
     hash table name which reaches the jrg */
  found =
    L_find_jrg_label_for_USE (&visited_ops, &address_ops, jrg->src[0], jrg,
                              df_done, &name_found);

  if (!name_found)
    L_punt ("L_find_jump_table_address_ops: no jump table names for jrg %d",
            jrg->id);
  if (!found)
    L_punt
      ("L_find_jump_table_address_ops: name_operand not found for jrg %d",
       jrg->id);

  return (address_ops);
}

/* Add attributes to each jsr and to the function to hold relevant jump tbl
   info for the function */
void
L_setup_jump_table_info (L_Func * fn)
{
  L_Cb *cb;
  L_Oper *oper, *def;
  L_Operand *name_operand;
  int num_jrg, tbl_id, df_done, num_tbls, max_id, i, *buf, num_address_ops;
  Set tbl_set, address_ops;

  num_jrg = 0;
  max_id = -1;
  tbl_set = NULL;
  df_done = 0;

  for (cb = fn->first_cb; cb != NULL; cb = cb->next_cb)
    {
      for (oper = cb->first_op; oper != NULL; oper = oper->next_op)
        {
          if (!L_register_branch_opcode (oper))
            continue;
          num_jrg++;

          /* Now get the jump table name from chain of flow dep ops */
          address_ops = L_find_jump_table_address_ops (cb, oper, &df_done);
          num_address_ops = Set_size (address_ops);
          buf = (int *) Lcode_malloc (sizeof (int) * num_address_ops);
          (void) Set_2array (address_ops, buf);

          name_operand = NULL;
          for (i = 0; i < num_address_ops; i++)
            {
              def = L_oper_hash_tbl_find_oper (fn->oper_hash_tbl, buf[i]);
              name_operand = L_extract_jump_table_name_operand (def);
              if (name_operand != NULL)
                break;
            }

          if (name_operand == NULL)
            L_punt ("L_setup_jump_table_info: name_operand not found");
          Lcode_free (buf);
          address_ops = Set_dispose (address_ops);

          tbl_id = L_extract_jump_table_id (name_operand->value.l);
#ifdef DEBUG
          if (Set_in (tbl_set, tbl_id))
            L_warn ("L_setup_jump_table_info: multiple jrgs use table %s",
                    name_operand->value.l);
#endif
          tbl_set = Set_add (tbl_set, tbl_id);
          if (tbl_id > max_id)
            max_id = tbl_id;

          /* Install attribute on the jrg */
          L_install_jumptbl_op_attr (oper, name_operand->value.l);
        }
    }

  num_tbls = Set_size (tbl_set);

  L_install_jumptbl_func_attr (fn, num_tbls, max_id);

  Set_dispose (tbl_set);
}

/*--------------------------------------------------------------------------*/
/*
 *      Creation of new jump tbls when replicating a jrg
 *      The user is assumed to have duplicated the jump tbl load
 *      and any associated addr calculations before calling this.
 */

int
L_safe_to_make_jump_tbl_names_unique (L_Func * fn, int *df_done)
{
  L_Cb *cb;
  L_Oper *oper;
  Set all_address_ops, address_ops;

  all_address_ops = NULL;

  for (cb = fn->first_cb; cb != NULL; cb = cb->next_cb)
    {
      for (oper = cb->first_op; oper != NULL; oper = oper->next_op)
        {
          if (!L_register_branch_opcode (oper))
            continue;
          address_ops = L_find_jump_table_address_ops (cb, oper, df_done);
          if (!Set_intersect_empty (all_address_ops, address_ops))
            return (0);
          all_address_ops = Set_union_acc (all_address_ops, address_ops);
          Set_dispose (address_ops);
        }
    }

  Set_dispose (all_address_ops);
  return (1);
}

/* Modify relevant label operands/attrs to account a new jump table name */
void
L_update_op_for_new_jump_table (L_Oper * oper, char *old_name, char *new_name)
{
  int i;
  L_Operand *src, *field;
  L_Attr *attr;

  /* Check the source operands */
  for (i = 0; i < L_max_src_operand; i++)
    {
      src = oper->src[i];
      if (L_is_label (src) && L_valid_jump_table_name (src->value.l))
        {
          if (strcmp (src->value.l, old_name))
            L_punt ("L_update_op_for_new_jump_table: "
                    "oper %d references unknown jump tbl", oper->id);
          L_delete_operand (src);
          oper->src[i] = L_new_gen_label_operand (new_name);
        }
    }

  /* Check if there are any label attrs referencing old_name */
  if (L_EXTRACT_BIT_VAL (oper->flags, L_OPER_LABEL_REFERENCE))
    {
      attr = L_find_attr (oper->attr, "label");
      field = attr->field[0];
      if (field->type == L_OPERAND_LABEL
          && L_valid_jump_table_name (field->value.l))
        {
          if (strcmp (field->value.l, old_name))
            L_punt ("L_update_op_for_new_jump_table: "
                    "oper %d references unknown jump tbl in a label attr",
                    oper->id);
          L_delete_operand (field);
          attr->field[0] = L_new_gen_label_operand (new_name);
        }
    }
}

/* Rename the table referenced by jrg to a new unique table */
int
L_make_new_jump_table (L_Func * fn, L_Cb * cb, L_Oper * jrg, int *df_done)
{
  Set address_ops;
  int *buf, i, num_address_ops, new_id;
  char *old_name, *new_name;
  L_Oper *op;

  old_name = L_jump_table_name (jrg);
  new_id = L_new_jump_table_id (fn);
  new_name = L_construct_jump_table_name (new_id);

  /* Update relevant attrs (func, and jrg) */
  L_update_func_attr_for_new_jump_table (fn, new_id);
  L_install_jumptbl_op_attr (jrg, new_name);

  address_ops = L_find_jump_table_address_ops (cb, jrg, df_done);
  num_address_ops = Set_size (address_ops);
  buf = (int *) Lcode_malloc (sizeof (int) * num_address_ops);
  (void) Set_2array (address_ops, buf);

  for (i = 0; i < num_address_ops; i++)
    {
      op = L_oper_hash_tbl_find_oper (fn->oper_hash_tbl, buf[i]);
      L_update_op_for_new_jump_table (op, old_name, new_name);
    }

  /* Record as modified */
  L_set_func_new_jump_table_flag (fn);

  Set_dispose (address_ops);
  Lcode_free (buf);

  return (new_id);
}

/* Make all jrgs in the program reference unique tables, no sharing.  This is
   the default config of all Lcode functions */
void
L_make_all_jump_tables_unique (L_Func * fn)
{
  L_Cb *cb;
  L_Oper *oper;
  char *tbl_name;
  int tbl_id, df_done, new_id;
  Set tbls_processed;

  tbls_processed = NULL;
  df_done = 0;

  if (!L_safe_to_make_jump_tbl_names_unique (fn, &df_done))
    {
      L_warn ("L_make_all_jump_tables_unique: not safe to rename, NOT DONE!");
      return;
    }

  for (cb = fn->first_cb; cb != NULL; cb = cb->next_cb)
    {
      for (oper = cb->first_op; oper != NULL; oper = oper->next_op)
        {
          if (!L_register_branch_opcode (oper))
            continue;

          /* Check if this jump tbl has already been done */
          tbl_name = L_jump_table_name (oper);
          tbl_id = L_extract_jump_table_id (tbl_name);
          if (Set_in (tbls_processed, tbl_id))
            {
              new_id = L_make_new_jump_table (fn, cb, oper, &df_done);
              tbls_processed = Set_add (tbls_processed, new_id);
            }
          else
            {
              tbls_processed = Set_add (tbls_processed, tbl_id);
            }
        }
    }

  Set_dispose (tbls_processed);
}

/*--------------------------------------------------------------------------*/
/*
 *      Jump table construction routines
 */

/* Build a jump tbl (a datalist) for a given jrg from the flow list */
L_Datalist *
L_generate_jump_table_for_op (L_Func * fn, L_Oper * jrg)
{
  L_Flow *flow, *ptr;
  L_Cb *cb;
  int i, align, num_entries, num_bytes, *cb_id_vals, default_cb_id,
    min_cc, max_cc, cc;
  char *tbl_name, tmp[1024];
  L_Datalist *datalist;
  L_Datalist_Element *element;
  L_Data *data;
  L_Expr *expr, *expr2, *exprA, *exprB;
  int ptype = 0;

  switch (M_type_size (M_TYPE_POINTER))
    {
    case 64:
      ptype = L_INPUT_WQ;
      break;
    case 32:
      ptype = L_INPUT_WI;
      break;
    default:
      L_punt ("L_generate_jump_table_for_op: bad pointer size");
    }

  if (!L_register_branch_opcode (jrg))
    L_punt ("L_generate_jump_table_for_op: op %d is not a jrg", jrg->id);

  cb = L_oper_hash_tbl_find_cb (fn->oper_hash_tbl, jrg->id);
  flow = L_find_flow_for_branch (cb, jrg);
  if (flow == NULL)
    L_punt ("L_generate_jump_table_for_op: jrg %d has no target flows",
            jrg->id);

  datalist = L_new_datalist ();

  min_cc = L_JUMPTBL_DEFAULT_CC;
  max_cc = -L_JUMPTBL_DEFAULT_CC;
  default_cb_id = -1;
  for (ptr = flow; ptr != NULL; ptr = ptr->next_flow)
    {
      cc = ptr->cc;
      if (cc == L_JUMPTBL_DEFAULT_CC)
        {
          default_cb_id = ptr->dst_cb->id;
          continue;
        }
      if (cc < min_cc)
        min_cc = cc;
      if (cc > max_cc)
        max_cc = cc;
    }

  if (default_cb_id == -1)
    L_punt ("L_generate_jump_table_for_op: jrg %d is missing default flow",
            jrg->id);

  /* Extract general jump tbl info */
  num_entries = max_cc - min_cc + 1;
  align = M_type_align (M_TYPE_POINTER) / M_type_size (M_TYPE_CHAR);
  num_bytes = num_entries * align;
  tbl_name = L_jump_table_name (jrg);

  /* Create a vector of target cb's for each cc val between min/max */
  cb_id_vals = (int *) Lcode_malloc (sizeof (int) * num_entries);
  for (i = 0; i < num_entries; i++)
    {
      cb_id_vals[i] = default_cb_id;
    }
  for (ptr = flow; ptr != NULL; ptr = ptr->next_flow)
    {
      cc = ptr->cc;
      if (cc == L_JUMPTBL_DEFAULT_CC)
        continue;
      cb_id_vals[cc - min_cc] = ptr->dst_cb->id;
    }

    /*** Create the list of data items ***/

  /* First we need an align data, such as (align 4 hash_0) */
  expr = L_new_expr (L_EXPR_LABEL);
  expr->value.l = L_add_string (L_string_table, tbl_name);
  data = L_new_data (L_INPUT_ALIGN);
  data->N = align;
  data->address = expr;
  element = L_new_datalist_element (data);
  L_concat_datalist_element (datalist, element);

  /* Second data is a reserve, such as (reserve 116) */
  data = L_new_data (L_INPUT_RESERVE);
  data->N = num_bytes;
  element = L_new_datalist_element (data);
  L_concat_datalist_element (datalist, element);

  /* Now for the actual table entries, each table entry is a (wi ...) data */
  for (i = 0; i < num_entries; i++)
    {
      /* Create the address expr for the wi data, generally its an ADD expr,
         but for i=0, an immediate addr can be used */
      if (i == 0)
        {
          expr = L_new_expr (L_EXPR_LABEL);
          expr->value.l = L_add_string (L_string_table, tbl_name);
        }
      else
        {
          exprA = L_new_expr (L_EXPR_LABEL);
          exprA->value.l = L_add_string (L_string_table, tbl_name);
          exprB = L_new_expr (L_EXPR_INT);
          exprB->value.i = (ITintmax) (i * align);
          expr = L_new_expr (L_EXPR_ADD);
          expr->A = exprA;
          expr->B = exprB;
        }

      /* Create the data expression  for the wi data */
      expr2 = L_new_expr (L_EXPR_LABEL);
      M_cb_label_name (fn->name, cb_id_vals[i], &tmp[0], 0);
      expr2->value.l = L_add_string (L_string_table, tmp);

      /* Create the actual wi data item */
      data = L_new_data (ptype);
      data->address = expr;
      data->value = expr2;
      element = L_new_datalist_element (data);
      L_concat_datalist_element (datalist, element);
    }

  Lcode_free (cb_id_vals);

  return (datalist);
}

/* Regenerate all jump tables from scratch for a function */
void
L_regenerate_all_jump_tables (L_Func * fn)
{
  L_Datalist *new_jump_tbls, *datalist;
  L_Datalist_Element *element;
  L_Data *data;
  L_Cb *cb;
  L_Oper *oper, *tmp_op, **op_buf;
  Set tbls_processed;
  char *tbl_name;
  int tbl_id, tmp_id, *id_buf, index, num_tbls, i, j;
  int max_tbl_id;

  /* Nuke the old list */
  L_delete_datalist (fn->jump_tbls);
  fn->jump_tbls = NULL;

  num_tbls = L_num_jump_tables (fn);
  if (num_tbls <= 0)
    return;

  op_buf = (L_Oper **) Lcode_malloc (sizeof (L_Oper *) * num_tbls);
  id_buf = (int *) Lcode_malloc (sizeof (int) * num_tbls);

  /* Find the jrg's and put them in the above 2 bufs. */
  tbls_processed = NULL;
  index = 0;
  /* reset the number of tables */
  num_tbls = 0;
  max_tbl_id = 0;
  for (cb = fn->first_cb; cb != NULL; cb = cb->next_cb)
    {
      for (oper = cb->first_op; oper != NULL; oper = oper->next_op)
        {
          if (!L_register_branch_opcode (oper))
            continue;

          /* Check if this jump tbl has already been done */
          tbl_name = L_jump_table_name (oper);
          tbl_id = L_extract_jump_table_id (tbl_name);
          if (Set_in (tbls_processed, tbl_id))
            continue;

          tbls_processed = Set_add (tbls_processed, tbl_id);
          op_buf[index] = oper;
          id_buf[index] = tbl_id;
          index++;
          num_tbls++;
          if (tbl_id > max_tbl_id)
            max_tbl_id = tbl_id;
        }
    }

  /* Reset the function's hash table attributes */
  L_install_jumptbl_func_attr (fn, num_tbls, max_tbl_id);

  /* Now sort the id_buf into ascending id's: this is just so we print
     out tables in order of increasing id (not necc, but helps with debug) */
  for (i = 0; i < num_tbls; i++)
    {
      for (j = i + 1; j < num_tbls; j++)
        {
          if (id_buf[j] < id_buf[i])
            {
              tmp_id = id_buf[j];
              tmp_op = op_buf[j];
              id_buf[j] = id_buf[i];
              op_buf[j] = op_buf[i];
              id_buf[i] = tmp_id;
              op_buf[i] = tmp_op;
            }
        }
    }

  /* Now, make the list of data's that represent the tables */
  new_jump_tbls = L_new_datalist ();

  /* First put a (ms data) data item on the list */
  data = L_new_data (L_INPUT_MS);
  data->N = L_MS_DATA;
  element = L_new_datalist_element (data);
  L_concat_datalist_element (new_jump_tbls, element);

  /* Second, for each table, generate the data items */
  for (i = 0; i < num_tbls; i++)
    {
      datalist = L_generate_jump_table_for_op (fn, op_buf[i]);
      L_merge_datalists (new_jump_tbls, datalist);
      L_delete_datalist (datalist);
    }

  fn->jump_tbls = new_jump_tbls;

  /* Free up tmp space */
  Lcode_free (op_buf);
  Lcode_free (id_buf);
  Set_dispose (tbls_processed);
}
