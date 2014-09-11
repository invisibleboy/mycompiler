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
 *	File:	pl_func.c
 *	Author:	Po-hua Chang, Wen-mei Hwu
 *	Creation Date:	June 1990
 *      Modified by: Nancy J. Warter
 *	Modified by: Roger A. Bringmann  2/8/93
 *	    Modified to produce new Lcode parenthesization/format
 *	 Revised: Dave Gallagher, Scott Mahlke - 6/94
 *              Build Lcode structure rather than just printing out text file
 *      Revised by: Ben-Chung Cheng - June 1995
 *              Change M_SIZE_INT, M_SIZE_CHAR to P_INT_SIZE, P_CHAR_SIZE
 *              Those should be determined in runtime, not compile time
 *	Revised by: Chien-Wei Li - 12/2001 for P-to-L.
 *		change H_CHAR_SIZE to P_CHAR_SIZE
 *		       H_INT_SIZE to P_INT_SIZE
\*****************************************************************************/
#include <config.h>
#include <assert.h>
#include <alloca.h>
#include "pl_main.h"
#include <library/c_basic.h>
#include <library/c_symbol.h>
#include <machine/m_spec.h>
#include <machine/m_tahoe.h>
#include <string.h>
#include <Pcode/cfg.h>
#include <Pcode/query_symtab.h>
#include <Pcode/symtab_i.h>

#undef DEBUG_PCODE

#undef TEST_GEN_EXPR
#undef DEBUG_LAYOUT
#undef DEBUG_FLOW
#undef DEBUG_HASH

L_Datalist *L_datalist = NULL;
L_Datalist *L_hash_datalist = NULL;
L_Datalist *L_string_datalist = NULL;

static void look_for_addr (Expr expr);
static int is_allocatable_type (Key type);
static int compute_actual_space (FuncDcl fn);
static void compute_actual_space2 (Expr expr);
static char *new_hash_table (char *fn);
static char *cb_label_name (char *fn, int cb);

static void PLI_gen_if (L_Cb * src_cb, L_Cb * dst_cb, Expr expr,
			Pragma pragma, int inv);
static int PLI_gen_return (L_Cb * cb, Expr expr, Key return_type,
			   int return_addr_reg);
static void PLI_gen_expr (L_Cb * cb, Expr expr);
static List PL_LinkLocalDataDcl (FuncDcl);

/* LCW - put local variables and parameters information in function 
 * attribute - 5/20/96 
 */
static void gen_local_info_attribute ();
static void gen_local_type_attribute (VarDcl var);

/*-------------------------------------------------------------------------*/
/*
 *  There is a problem with a function that returns structures (union/struct).
 *  We require the space to be allocated by the caller before function call,
 *  and pass the address through the $R register.
 *	e.g.
 *		(alloc ($R) (size align))
 *		(jsr ....)
 *  After the function call, it is assumed that $R is unchanged, and
 *  points to the return data.
 *	e.g.
 *		(jsr ....)
 *		(mov (r.new) ($R))
 *  Now the data is safe to be operated upon until the next call.
 *  Usually the caller copies the data to a variable immediately.
 *
 *  The callee (that returns structure) should do the following:
 *  	e.g.
 *		(prologue ....)
 *		(mov (r.new) ($R))
 *  So it does not need to worry about $R being clobbered when it does
 *  function calls. If there is no function call in the callee body,
 *  the code optimizer will automatically propagate $R to the return block.
 */
/*
 *  Be careful about the type of variables.
 *  Because we automatically promote function arguments,
 *  when a formal parameter is referred, its "param_mtype"
 *  should be used (!!! not "param->type").
 *  For local variables, they are not promoted. Therefore,
 *  we can used either "local_mtype" or local->type.
 *  For consistency, we will use "mtype" at all times.
 *  For global variables, the usual var->type can be
 *  converted by PL_pcode2lcode_type().
 */
/*-------------------------------------------------------------------------*/

#define MAX_VAR ((PL_MAX_LOCAL_VAR>PL_MAX_PARAM_VAR) ? \
	PL_MAX_LOCAL_VAR:PL_MAX_PARAM_VAR)


Dyn_str_t *
PL_dstr_new (int init_size)
{
  Dyn_str_t *buf = calloc (sizeof (Dyn_str_t), 1);

  buf->maxsize = init_size;
  buf->size = 1;
  buf->str = (char *) malloc (buf->maxsize);
  buf->str[0] = 0;

  return buf;
}


void 
PL_dstr_free(Dyn_str_t *buf)
{
  free(buf->str);
  free(buf);
}


void 
PL_dstr_strcat(Dyn_str_t *buf, char *str)
{
  int   buffer_len;

  if (!buf)
    L_punt("LI_dstr_strcat: buf is NULL\n");

  buffer_len = strlen(str);
  if ((buffer_len + buf->size) > (buf->maxsize-1))
    {
#if 0
      printf("Resizing buffer[%d], bp[%d] bpsize[%d] bpmax[%d]\n",
             buffer_len, strlen(buf_ptr),
             buf_ptr_size, buf_ptr_maxsize);
      printf("%c %c\n",
             *(char*)(buf_ptr+buf_ptr_size),
             *(char*)(buf_ptr+buf_ptr_size+1));
#endif
      while ((buf->maxsize-1) < (buffer_len + buf->size))
        {
          buf->maxsize *= 2;
        }
#if 0
      printf("Maxsize [%d]\n", buf_ptr_maxsize);
#endif

      buf->str = realloc(buf->str, buf->maxsize);
    }

  strcat(buf->str, str);
  buf->size += buffer_len;
  buf->str[buf->size] = 0;
}


void
PL_dstr_sprintf(Dyn_str_t *buf, const char *format, ...)
{
  va_list args;

  va_start (args, format);

  /* Increase size until there is no more truncation */
  while (buf->maxsize == vsnprintf (buf->str, buf->maxsize, format, args))
    {
      buf->maxsize *= 2;
      buf->str = realloc(buf->str, buf->maxsize);     
    }

  va_end (args);
  
  buf->size = strlen(buf->str);
}


char *
PL_dstr2str(Dyn_str_t *buf)
{
  char *buffer;

  buffer = malloc((sizeof(char) * buf->size) + 1);

  strncpy(buffer, buf->str, buf->size);

  assert(buf->str[buf->size] == 0);
  assert(strlen(buffer) < buf->size);

  return buffer;
}


void
PL_dstr_trunc(Dyn_str_t *buf, int num)
{
  assert(num <= buf->size);
  buf->size -= num;
  buf->str[buf->size] = 0;  
}


void 
PL_dstr_clear(Dyn_str_t *buf)
{
  buf->size = 0;
  buf->str[0] = 0;    
}


/*
 * Local Variable management
 */

typedef struct lvar_info_t
{
  VarDcl  var;
  int     index;
  int     is_param;
  int     is_lvar;
  int     is_vararg_param;

  _M_Type mtype;
  long    offset;
  char   *base_macro;
  int     register_allocatable;
  int     mode;
  int     reg_id;

  /* From formal param */
  int     reg;
  int     paddr;
  int     su_sreg;
  int     su_ereg;
  int     real_sreg;
  int     real_ereg;
  char   *tahoe_param_base_macro;
  
  struct lvar_info_t *nxt_var;
} lvar_info_t;


typedef struct lvars_t
{
  lvar_info_t *head;
  lvar_info_t *tail;
  int size;
  int r_count; /* number of param passing regs used */
} lvars_t;


static int next_reg_id;		/* virtual register counter */

static void
PL_init_reg_id ()
{
  next_reg_id = 1;
}

int
PL_next_reg_id ()
{
  return next_reg_id++;
}

static int
PL_peek_next_reg_id ()
{
  return next_reg_id;
}


Extension 
P_alloc_vartbl_entry(void)
{
  lvar_info_t *tmp; 
  tmp = calloc(1,sizeof(lvar_info_t));
  assert(tmp);
  return (Extension)tmp;
}


Extension 
P_free_vartbl_entry(Extension e)
{
  lvar_info_t *tmp = (lvar_info_t*)e;
  if (tmp)
    free(tmp);
  return NULL;
}


lvar_info_t *
PL_find_vartbl_entry(Key key)
{
  VarDcl dcl;
  lvar_info_t *tmp;
  
  dcl = PST_GetVarDclEntry(PL_symtab, key);
  tmp = P_GetVarDclExtM(dcl);
  
  return tmp;
}


static lvar_info_t *
PL_add_to_lvarlist(lvars_t *vars, VarDcl var)
{
  lvar_info_t *tmp;
 
  /* These have already been created */
  tmp = PL_find_vartbl_entry(var->key);
  assert(tmp);
  assert(tmp->nxt_var == NULL); 
  assert(tmp->var == NULL); 

  if (!vars->head)
    vars->head = tmp;
  if (vars->tail)
    {
      vars->tail->nxt_var = tmp;
      tmp->index = vars->tail->index + 1;
    }

  vars->tail = tmp;

  assert(vars->size == tmp->index);
  vars->size++;

  tmp->var = var;

  return tmp;
}


int
PL_find_local_var (Key key, M_Type mtype, int *in_reg, int *reg_id,
		   char **base_macro, int *offset)
{
  lvar_info_t *vinfo;

  vinfo = PL_find_vartbl_entry(key);
  
  if (vinfo->var == NULL)
    return 0;
  if (!vinfo->is_lvar)
    return 0;

  if (mtype != 0)
    *mtype = vinfo->mtype;
  if (in_reg != 0)
    *in_reg = (vinfo->mode == M_THRU_REGISTER);
  if (reg_id != 0)
    *reg_id = vinfo->reg_id;
  if (base_macro != 0)
    *base_macro = vinfo->base_macro;
  if (offset != 0)
    *offset = vinfo->offset;

  return 1;
}


int
PL_find_param_var (Key key, M_Type mtype, int *in_reg, int *reg_id,
		   char **base_macro, int *offset, int *sreg, int *ereg)
{
  lvar_info_t *vinfo;

  vinfo = PL_find_vartbl_entry(key);
  
  if (vinfo->var == NULL)
    return 0;
  if (!vinfo->is_param)
    return 0;

  if (mtype)
    *mtype = vinfo->mtype;
  if (reg_id)
    *reg_id = vinfo->reg_id;
  if (sreg)
    *sreg = vinfo->real_sreg;
  if (ereg)
    *ereg = vinfo->real_ereg;
  if (base_macro)
    *base_macro = (M_arch == M_TAHOE) ? 
      vinfo->tahoe_param_base_macro : vinfo->base_macro;
  
  if (in_reg)
    {
      switch (vinfo->mode)
	{
	case M_THRU_REGISTER:
	  *in_reg = 1;
	  break;
	case M_THRU_MEMORY:
	case M_INDIRECT_THRU_REGISTER:
	case M_INDIRECT_THRU_MEMORY:
	  *in_reg = 0;
	  break;
	default:
	  P_punt ("illegal mode returned by M_fnvar_layout");
	}
    }
  if (offset)
    {
      switch (vinfo->mode)
	{
	case M_THRU_REGISTER:
	case M_THRU_MEMORY:
	  *offset = vinfo->offset;
	  break;
	case M_INDIRECT_THRU_REGISTER:
	case M_INDIRECT_THRU_MEMORY:
	  *offset = vinfo->paddr;
	  break;
	default:
	  P_punt ("illegal mode returned by M_fnvar_layout");
	}
    }

  return 1;
}


static void
find_allocatable (lvars_t *vars)
{
  lvar_info_t *vinfo;

  /*
   *  first assume everything is register allocatable.
   *  except can not allocate a structure to a register.
   *  1. union, 2. struct, 3. array, 4. function
   */

  for (vinfo = vars->head; vinfo; vinfo = vinfo->nxt_var)
    vinfo->register_allocatable = is_allocatable_type (vinfo->var->type);

  /*
   *  delete those that are pointer accessable.
   */

  PC_ApplyToExprs (look_for_addr);

  return;
}


/* CWL - 12/17/00 for P-to-L.
 * The Pcode "Expr" structure is different from the Hcode "Expr".
 * An expr tree is like:
 *
 * switch (expr->opcode)
 *   case OP_var:      	{expr->value.var_name} 
 *   case OP_enum:     	undefined 
 *   case OP_error:    	undefined
 *   case OP_int:      	{expr->value.uscalar} if TY_UNSIGNED
 *                  	{expr->value.scalar}  if TY_SIGNED 
 *   case OP_real | OP_float | OP_double:	
 * 			{expr->value.real} 
 *   case OP_char | OP_string:
 *   			{expr->value.string} 
 *   case OP_dot:      	{expr->operands} {"."} {expr->value.string} 
 *   case OP_arrow:    	{expr->operands} {"->"} {expr->value.string} 
 *   case OP_cast:     	{"("} {expr->type} {")"} {expr->operands} 
 *   case OP_expr_size:	undefined
 *   case OP_type_size:	undefined
 *   case OP_neg:      	{"-"} {expr->operands}
 *   case OP_not:      	{"!"} {expr->operands} 
 *   case OP_inv:      	{"~"} {expr->operands}
 *   case OP_preinc:   	{"++"} {expr->operands}
 *   case OP_predec:   	{"--"} {expr->operands} 
 *   case OP_indr:     	{"*"} {expr->operands} 
 *   case OP_addr:     	{"&"} {expr->operands}
 *   case OP_postinc:  	{expr->operands} {"++"}
 *   case OP_postdec:  	{expr->operands} {"--"}
 *   case OP_quest:    	{expr->operands} {"?"}
 *                     	   {expr->operands->sibling} {":"}
 *                     	   {expr->operands->sibling->sibling}
 *   case OP_compexpr: 	{"("} {expr->operands} (maybe another OP_compexpr)
 *                     	   {","} {expr->operands->next} (maybe empty) {")"} 
 *   case OP_index:    	{expr->operands} {"["} {expr->operands->sibling} {"]"} 
 *   case OP_call:     	{expr->operands} {"("} 
 *                         {expr->operands->sibling} {","}
 *                         {expr->operands->sibling->next} {","}
 *                         {expr->operands->sibling->next->next} ... {")"} 
 *   case OP_disj:   	{expr->operands} {"||"} {expr->operands->sibling} 
 *   case OP_conj:   	{expr->operands} {"&&"} {expr->operands->sibling} 
 *   case OP_assign: 	{expr->operands} {"="} {expr->operands->sibling} 
 *   case OP_or:     	{expr->operands} {"|"} {expr->operands->sibling}
 *   case OP_xor:    	{expr->operands} {"^"} {expr->operands->sibling}
 *   case OP_and:    	{expr->operands} {"&"} {expr->operands->sibling}
 *   case OP_eq:     	{expr->operands} {"=="} {expr->operands->sibling}
 *   case OP_ne:     	{expr->operands} {"!="} {expr->operands->sibling}
 *   case OP_lt:     	{expr->operands} {"<"} {expr->operands->sibling}
 *   case OP_le:     	{expr->operands} {"<="} {expr->operands->sibling}
 *   case OP_ge:     	{expr->operands} {">="} {expr->operands->sibling}
 *   case OP_gt:     	{expr->operands} {">"} {expr->operands->sibling}
 *   case OP_rshft:  	{expr->operands} {">>"} {expr->operands->sibling}
 *   case OP_lshft:  	{expr->operands} {"<<"} {expr->operands->sibling}
 *   case OP_add:    	{expr->operands} {"+"} {expr->operands->sibling}
 *   case OP_sub:    	{expr->operands} {"-"} {expr->operands->sibling}
 *   case OP_mul:    	{expr->operands} {"*"} {expr->operands->sibling}
 *   case OP_div:    	{expr->operands} {"/"} {expr->operands->sibling}
 *   case OP_mod:    	{expr->operands} {"%"} {expr->operands->sibling}
 *   case OP_Aadd:   	{expr->operands} {"+="} {expr->operands->sibling}
 *   case OP_Asub:   	{expr->operands} {"-="} {expr->operands->sibling}
 *   case OP_Amul:   	{expr->operands} {"*="} {expr->operands->sibling}
 *   case OP_Adiv:   	{expr->operands} {"/="} {expr->operands->sibling}
 *   case OP_Amod:   	{expr->operands} {"%="} {expr->operands->sibling}
 *   case OP_Arshft: 	{expr->operands} {">>="} {expr->operands->sibling}
 *   case OP_Alshft: 	{expr->operands} {"<<="} {expr->operands->sibling}
 *   case OP_Aand:   	{expr->operands} {"&="} {expr->operands->sibling}
 *   case OP_Aor:    	{expr->operands} {"|="} {expr->operands->sibling}
 *   case OP_Axor:   	{expr->operands} {"^="} {expr->operands->sibling}
 */

static void
look_for_addr (Expr expr)
{
  Expr op, opS, opN;

  if (!expr || !(op = expr->operands))
    return;

  for (opS = expr->operands; opS; opS = opS->sibling)
    {
      look_for_addr (opS);
      for (opN = opS->next; opN; opN = opN->next)
	look_for_addr (opN);
    }

  if ((expr->opcode == OP_addr) && (op->opcode == OP_var))
    {
      lvar_info_t *vinfo;
      vinfo = PL_find_vartbl_entry(op->value.var.key);
      vinfo->register_allocatable = 0;
    }

  return;
}


static int
is_allocatable_type (Key type)
{
  /* do not move volatile objects to register */
  if (PST_GetTypeQualifier(PL_symtab, type) & TY_VOLATILE)
    return 0;
  
  if (PST_GetTypeBasicType(PL_symtab, type) & (BT_ARRAY |
					       BT_FUNC |
					       BT_STRUCTURE))
    return 0;

  return 1;
}


/*
 *	do not allocate space to those that are register
 *	allocatable. the space will be automatically
 *	allocated if spilling occurs.
 */

static int
PL_layout_local (lvars_t *vars)
{
  _M_Type temp_mtype[MAX_VAR];
  char *temp_base_macro = "???";
  long temp_offset[MAX_VAR];
  int n_local = 0, local_space = 0, num = 0;
  lvar_info_t *vinfo;

  n_local = vars->size;
  if (n_local == 0)
    return 0;
  
  num = 0;
  for (vinfo = vars->head; vinfo; vinfo = vinfo->nxt_var)
    {
      assert(num < MAX_VAR);
      if (!vinfo->register_allocatable)
	{
	  temp_mtype[num] = vinfo->mtype;
	  num++;
	}
    }

  local_space = M_lvar_layout (num, temp_mtype, temp_offset, 
			       &temp_base_macro);
  
  if (M_arch != M_TAHOE)
    {
      int i;
      for (i = 0; i < num; i++)
	temp_offset[i] /= P_CHAR_SIZE;	/* convert to byte */

      local_space /= P_CHAR_SIZE;
    }

  num = 0;
  for (vinfo = vars->head; vinfo; vinfo = vinfo->nxt_var)
    {
      vinfo->base_macro = temp_base_macro;
      if (vinfo->register_allocatable)
	{
	  vinfo->reg_id = PL_next_reg_id ();
	  vinfo->mode = M_THRU_REGISTER;
	  vinfo->offset = -1;
	}
      else
	{
	  vinfo->reg_id = -1;
	  vinfo->mode = M_THRU_MEMORY;
	  vinfo->offset = temp_offset[num];
 	  num++;
	}
    }
  
  return local_space;
}


/*
 *	the allocation decision is not done here, because
 *	the caller has no idea what's pointer accessable
 *	or not. so we will let the caller make the decision,
 *	and make necessary repair.
 */
static int
PL_layout_formal (lvars_t *vars, int returns_ST)
{
  char *base_macro = "???";
  int count = 0, param_space, n_param = vars->size;
  lvar_info_t *vinfo;
  M_Param mp, cmp;
  List lp = NULL;
  int ctr = 0;

  if (n_param == 0)
    return 0;
 
  mp = alloca (n_param * sizeof (_M_Param));

  /* Initialize arrays to known values to prevent random numbers
   * from being put into new attributes -ITI/JCG 3/99
   */
  for (vinfo = vars->head, cmp = mp; vinfo; vinfo = vinfo->nxt_var, cmp++)
    {
      vinfo->reg = -1;
      cmp->mtype = vinfo->mtype;
      cmp->mode = vinfo->mode;
      cmp->reg = -1;
      cmp->offset = -1;
      cmp->paddr = 0;
      cmp->su_sreg = -1;
      cmp->su_ereg = -1;
      lp = List_insert_last (lp, (void *)cmp);
    }

  /*
   *    Prepare for recieving incoming parameters.
   */

  param_space =
    M_layout_fnvar (lp, &base_macro, &count, M_GET_FNVAR, returns_ST);

  for (vinfo = vars->head, cmp = mp; vinfo; vinfo = vinfo->nxt_var, cmp++, ctr++)
    {
      vinfo->reg = cmp->reg;
      vinfo->mode = cmp->mode;
      vinfo->base_macro = base_macro;
      vinfo->offset = cmp->offset;
      vinfo->paddr = cmp->paddr;
      vinfo->su_sreg = cmp->su_sreg;
      vinfo->su_ereg = cmp->su_ereg;
      vinfo->real_sreg = -1;
      vinfo->real_ereg = -1;      
      if (M_arch == M_TAHOE)
	{
	  vinfo->tahoe_param_base_macro = base_macro;
	}
      else
	{
	  vinfo->offset /= P_CHAR_SIZE;	/* convert to byte */
	  vinfo->paddr /= P_CHAR_SIZE;	/* convert to byte */
	}

    }

  vars->r_count = count;

  if (M_arch != M_TAHOE)
    param_space /= P_CHAR_SIZE;

  lp = List_reset (lp);

  return param_space;
}


void
PL_update_local_size_oper (L_Cb * prologue_cb, int size)
{
  L_Oper *oper;

  for (oper = prologue_cb->first_op; oper; oper = oper->next_op)
    {
      if ((oper->opc == Lop_DEFINE) &&
	  (oper->dest[0]->type == L_OPERAND_MACRO) &&
	  (oper->dest[0]->value.mac == L_MAC_LOCAL_SIZE))
	{
	  oper->src[0]->value.i = (ITintmax) size;
	  break;
	}
    }

  if (oper == NULL)
    P_punt ("PL_update_local_size_oper: Couldn't find local size oper");
  return;
}

static int tmprec_actual_space = 0;

static int
compute_actual_space (FuncDcl fn)
{
  char *temp_base_macro = "???";
  _M_Type temp_mtype[MAX_VAR];
  int temp_mode[MAX_VAR];
  int temp_reg[MAX_VAR];
  long temp_offset[MAX_VAR];
  int temp_paddr[MAX_VAR];
  int temp_su_sreg[MAX_VAR];
  int temp_su_ereg[MAX_VAR];
  int temp_count;
  int size;
  int actual_space = 0;

  /*
   *  for Sparc code generator, even if the function does not
   *  make function call, we still need to allocate some
   *  minimum parameter region (for OS registers saves)
   *
   *  Because of the 0, the parameters are dummys
   */
  size = M_fnvar_layout (0, temp_mtype, temp_offset,
			 temp_mode, temp_reg, temp_paddr,
			 &temp_base_macro,
			 temp_su_sreg, temp_su_ereg,
			 &temp_count, 0, M_PUT_FNVAR);
  if (M_arch != M_TAHOE)
    size /= P_CHAR_SIZE;
  if (size > actual_space)
    actual_space = size;

  if (PL_is_aggr_type (fn->type, NULL))
    {
      /* If a structure is returned by this function, account for the parameter
       * size needed by the internal block copy routine, IMPACT_block_mov. */
      _M_Type type[4];
      M_int (type + 0, 1);
      M_int (type + 1, 1);
      M_int (type + 2, 0);
      M_int (type + 3, 0);

      size = M_fnvar_layout (4, type, temp_offset, temp_mode, temp_reg,
			     temp_paddr, &temp_base_macro,
			     temp_su_sreg, temp_su_ereg,
			     &temp_count, 0, M_PUT_FNVAR);
      printf ("Func[%s] Returning a struct bit size %d\n", fn->name, size);
      size /= P_CHAR_SIZE;
      if (size > actual_space)
	actual_space = size;
    }

  /*
   *  check for actual calls.
   */

  
  tmprec_actual_space = actual_space;
  PC_ApplyToExprs (compute_actual_space2);
  actual_space = tmprec_actual_space;

  return actual_space;
}

static void
compute_actual_space2 (Expr expr)
{
  int num, size, need_ST;
  Expr op;

  if (!expr || !expr->operands)
    return;

  compute_actual_space2 (expr->operands);
  if (expr->opcode == OP_compexpr)
    {				/* assume at least one operands */
      /*  
       * {"("} {expr->operands} {","} {expr->operands->next} {")"} 
       */
      Expr e;

      for (e = expr->operands->next; e; e = e->next)
	compute_actual_space2 (e);
    }
  else if (expr->operands->sibling)
    {
      compute_actual_space2 (expr->operands->sibling);

      if (expr->opcode == OP_quest)
	{
	  /* 
	   * {expr->operands} {"?"} {expr->operands->sibling} {":"}
	   *                        {expr->operands->sibling->sibling} 
	   */
	  compute_actual_space2 (expr->operands->sibling->sibling);
	}
      else if (expr->opcode == OP_call)
	{
	  /* 
	   * {expr->operands} {"("} {expr->operands->sibling}
	   *                  {","} {expr->operands->sibling->next}
	   *                  {","} {expr->operands->sibling->next->next}
	   *                  ... {")"} 
	   */
	  for (op = expr->operands->sibling->next; op; op = op->next)
	    compute_actual_space2 (op);
	}
    }

  if (expr->opcode == OP_call)
    {
      char *temp_base_macro = "???";
      _M_Type temp_mtype[MAX_VAR];
      int temp_mode[MAX_VAR];
      int temp_reg[MAX_VAR];
      long temp_offset[MAX_VAR];
      int temp_paddr[MAX_VAR];
      int temp_su_sreg[MAX_VAR];
      int temp_su_ereg[MAX_VAR];
      int temp_count;

      for (op = expr->operands->sibling, num = 0; op; op = op->next, num++)
	{
	  PL_pcode2lcode_type (PST_ExprType(PL_symtab, op), 
			       temp_mtype + num, 1);
	}
      need_ST = PL_is_aggr_type (PST_ExprType(PL_symtab, expr), NULL);

      size = M_fnvar_layout (num, temp_mtype, temp_offset,
			     temp_mode, temp_reg, temp_paddr,
			     &temp_base_macro, temp_su_sreg, temp_su_ereg,
			     &temp_count, need_ST, M_PUT_FNVAR);


      if (M_arch != M_TAHOE)
	size /= P_CHAR_SIZE;
      if (size > tmprec_actual_space)
	tmprec_actual_space = size;
    }
}


/*-------------------------------------------------------------------------*/
/* LCW - put local variables and parameters information in function 
 * attribute - 5/20/96 */
static void
gen_local_info_attribute (lvars_t *lvars, lvars_t *pvars)
{
  char str_buffer[256];
  L_Attr *new_attr;
  lvar_info_t *vinfo;

  for (vinfo = lvars->head; vinfo; vinfo = vinfo->nxt_var)
    {
      /* generate variable name */
      new_attr = L_new_attr ("LVAR_NAME", 1);
      sprintf (str_buffer, "\"%s\"", vinfo->var->name);
      L_set_string_attr_field (new_attr, 0, str_buffer);
      L_fn->attr = L_concat_attr (L_fn->attr, new_attr);

      /* generate type infomation */
      gen_local_type_attribute (vinfo->var);

      /* generate actual location */
      new_attr = L_new_attr ("LVAR_LOC", 2);
      if (vinfo->mode == M_THRU_REGISTER)
	{
	  L_set_string_attr_field (new_attr, 0, "\"r\"");
	  L_set_int_attr_field (new_attr, 1, vinfo->reg_id);
	}
      else
	{			/* M_THRU_MEMORY */
	  L_set_string_attr_field (new_attr, 0, "\"m\"");
	  L_set_int_attr_field (new_attr, 1, vinfo->offset);
	}
      L_fn->attr = L_concat_attr (L_fn->attr, new_attr);
    }

  for (vinfo = pvars->head; vinfo; vinfo = vinfo->nxt_var)
    {
      /* generate parameter name */
      new_attr = L_new_attr ("LVAR_NAME", 1);
      sprintf (str_buffer, "\"%s\"", vinfo->var->name);
      L_set_string_attr_field (new_attr, 0, str_buffer);
      L_fn->attr = L_concat_attr (L_fn->attr, new_attr);

      /* generate type infomation */
      gen_local_type_attribute (vinfo->var);

      /* generate actual location */
      new_attr = L_new_attr ("LVAR_LOC", 2);
      L_set_string_attr_field (new_attr, 0, "\"m\"");
      L_set_int_attr_field (new_attr, 1, vinfo->offset);
      L_fn->attr = L_concat_attr (L_fn->attr, new_attr);
    }
}


/* LCW - generate function attribute for local variable type - 5/20/96 */
static void
gen_local_type_attribute (VarDcl var)
{
  char st_name[256];
  int qual, tqual, basic;
  L_Attr *new_attr;
  Key type;

  type = var->type;
  qual = P_GetVarDclQualifier(var);
  tqual = PST_GetTypeQualifier(PL_symtab, type);
  basic = PST_GetTypeBasicType(PL_symtab, type);

  if (qual & VQ_REGISTER)
    {
      new_attr = L_new_attr ("LVAR_TYPE", 1);
      L_set_string_attr_field (new_attr, 0, "\"register\"");
      L_fn->attr = L_concat_attr (L_fn->attr, new_attr);
    }
  if (qual & VQ_STATIC)
    {
      new_attr = L_new_attr ("LVAR_TYPE", 1);
      L_set_string_attr_field (new_attr, 0, "\"static\"");
      L_fn->attr = L_concat_attr (L_fn->attr, new_attr);
    }
  if (qual & VQ_EXTERN)
    {				/* put error message here? */
      new_attr = L_new_attr ("LVAR_TYPE", 1);
      L_set_string_attr_field (new_attr, 0, "\"extern\"");
      L_fn->attr = L_concat_attr (L_fn->attr, new_attr);
    }
  if (qual & VQ_AUTO)
    {
      new_attr = L_new_attr ("LVAR_TYPE", 1);
      L_set_string_attr_field (new_attr, 0, "\"auto\"");
      L_fn->attr = L_concat_attr (L_fn->attr, new_attr);
    }
  if (qual & VQ_GLOBAL)
    {				/* put error message here? */
      new_attr = L_new_attr ("LVAR_TYPE", 1);
      L_set_string_attr_field (new_attr, 0, "\"global\"");
      L_fn->attr = L_concat_attr (L_fn->attr, new_attr);
    }
  if (qual & VQ_PARAMETER)
    {
      new_attr = L_new_attr ("LVAR_TYPE", 1);
      L_set_string_attr_field (new_attr, 0, "\"parameter\"");
      L_fn->attr = L_concat_attr (L_fn->attr, new_attr);
    }





  if (tqual & TY_CONST)
    {
      new_attr = L_new_attr ("LVAR_TYPE", 1);
      L_set_string_attr_field (new_attr, 0, "\"const\"");
      L_fn->attr = L_concat_attr (L_fn->attr, new_attr);
    }
  if (tqual & TY_VOLATILE)
    {
      new_attr = L_new_attr ("LVAR_TYPE", 1);
      L_set_string_attr_field (new_attr, 0, "\"volatile\"");
      L_fn->attr = L_concat_attr (L_fn->attr, new_attr);
    }
#if 0
  if (basic & BT_SIGNED)
    {
      new_attr = L_new_attr ("LVAR_TYPE", 1);
      L_set_string_attr_field (new_attr, 0, "\"signed\"");
      L_fn->attr = L_concat_attr (L_fn->attr, new_attr);
    }
#endif
  if (basic & BT_UNSIGNED)
    {
      new_attr = L_new_attr ("LVAR_TYPE", 1);
      L_set_string_attr_field (new_attr, 0, "\"unsigned\"");
      L_fn->attr = L_concat_attr (L_fn->attr, new_attr);
    }
  if (basic & BT_VOID)
    {
      new_attr = L_new_attr ("LVAR_TYPE", 1);
      L_set_string_attr_field (new_attr, 0, "\"void\"");
      L_fn->attr = L_concat_attr (L_fn->attr, new_attr);
    }
  if (basic & BT_SHORT)
    {
      new_attr = L_new_attr ("LVAR_TYPE", 1);
      L_set_string_attr_field (new_attr, 0, "\"short\"");
      L_fn->attr = L_concat_attr (L_fn->attr, new_attr);
    }
  if (basic & BT_LONG)
    {
      new_attr = L_new_attr ("LVAR_TYPE", 1);
      L_set_string_attr_field (new_attr, 0, "\"long\"");
      L_fn->attr = L_concat_attr (L_fn->attr, new_attr);
    }
  if (basic & BT_CHAR)
    {
      new_attr = L_new_attr ("LVAR_TYPE", 1);
      L_set_string_attr_field (new_attr, 0, "\"char\"");
      L_fn->attr = L_concat_attr (L_fn->attr, new_attr);
    }
  if (basic & BT_INT)
    {
      new_attr = L_new_attr ("LVAR_TYPE", 1);
      L_set_string_attr_field (new_attr, 0, "\"int\"");
      L_fn->attr = L_concat_attr (L_fn->attr, new_attr);
    }
  if (basic & BT_FLOAT)
    {
      new_attr = L_new_attr ("LVAR_TYPE", 1);
      L_set_string_attr_field (new_attr, 0, "\"float\"");
      L_fn->attr = L_concat_attr (L_fn->attr, new_attr);
    }
  if (basic & BT_DOUBLE)
    {
      new_attr = L_new_attr ("LVAR_TYPE", 1);
      L_set_string_attr_field (new_attr, 0, "\"double\"");
      L_fn->attr = L_concat_attr (L_fn->attr, new_attr);
    }
  if (basic & BT_LONGDOUBLE)
    {
      new_attr = L_new_attr ("LVAR_TYPE", 1);
      L_set_string_attr_field (new_attr, 0, "\"long double\"");
      L_fn->attr = L_concat_attr (L_fn->attr, new_attr);
    }
  if (basic & BT_STRUCT)
    {
      new_attr = L_new_attr ("LVAR_TYPE", 1);
      sprintf (st_name, "\"struct %s\"", 
	       P_GetStructDclName(PST_GetStructDclEntry(PL_symtab, 
							PST_GetTypeType(PL_symtab, type))));
      L_set_string_attr_field (new_attr, 0, st_name);
      L_fn->attr = L_concat_attr (L_fn->attr, new_attr);
    }
  if (basic & BT_UNION)
    {
      new_attr = L_new_attr ("LVAR_TYPE", 1);
      sprintf (st_name, "\"union %s\"", 
	       P_GetUnionDclName(PST_GetUnionDclEntry(PL_symtab, 
						      PST_GetTypeType(PL_symtab, type))));
      L_set_string_attr_field (new_attr, 0, st_name);
      L_fn->attr = L_concat_attr (L_fn->attr, new_attr);
    }
  if (basic & BT_ENUM)
    {
      new_attr = L_new_attr ("LVAR_TYPE", 1);
      sprintf (st_name, "\"enum %s\"", 
	       P_GetEnumDclName(PST_GetEnumDclEntry(PL_symtab, 
						    PST_GetTypeType(PL_symtab, type))));
      L_set_string_attr_field (new_attr, 0, st_name);
      L_fn->attr = L_concat_attr (L_fn->attr, new_attr);
    }
  if (basic & BT_VARARG)
    {
      new_attr = L_new_attr ("LVAR_TYPE", 1);
      L_set_string_attr_field (new_attr, 0, "\"vararg\"");
      L_fn->attr = L_concat_attr (L_fn->attr, new_attr);
    }

  while ((PST_GetTypeBasicType(PL_symtab, type) & (BT_POINTER|BT_ARRAY|BT_FUNC)) != 0)
    {
      new_attr = L_new_attr ("LVAR_DCLTR", 1);
      if (PST_GetTypeBasicType(PL_symtab, type) & BT_ARRAY)
	L_set_string_attr_field (new_attr, 0, "\"a\"");
      else if (PST_GetTypeBasicType(PL_symtab, type) & BT_POINTER)
	L_set_string_attr_field (new_attr, 0, "\"p\"");
      else if (PST_GetTypeBasicType(PL_symtab, type) & BT_FUNC)
	L_set_string_attr_field (new_attr, 0, "\"f\"");
      else
	P_punt
	  ("gen_local_type_attribute: illegal declarator (access pattern)");
	
      L_fn->attr = L_concat_attr (L_fn->attr, new_attr);
      type = PST_GetTypeType(PL_symtab, type);
    }
}


/*-------------------------------------------------------------------------*/
/* SAM 8-96, modified to be Mspec driven */
static int next_hash_tbl_id = 0;

static char *
new_hash_table (char *fn)
{
  char line[1024], name[1024];

  sprintf (name, "%s", fn);	/* prefix _ already in Lcode name */
  M_jumptbl_label_name (name, next_hash_tbl_id, line, 1024);
  next_hash_tbl_id++;

  return C_findstr (line);
}


static char *
cb_label_name (char *fn, int cb)
{
  char name[512], line[512];

  sprintf (name, "_%s", fn);	/* prefix with _ */
  M_cb_label_name (name, cb, line, 512);

  return C_findstr (line);
}


/*-------------------------------------------------------------------------*/
/*
 *	Convert string to upper case, convert -'s to _'s
 */
void
PL_convert_to_upper (char *str)
{
  int i, len;

  len = strlen (str);
  for (i = 0; i < len; i++)
    {
      if (str[i] == '-')
	str[i] = '_';
      else
	str[i] = toupper (str[i]);
    }
}


static L_Oper *
PL_define_incoming_reg (L_Cb *prologue_cb, int mac, int ctype)
{
  L_Oper *new_oper;

  new_oper = PL_new_loper (NULL, Lop_DEFINE);
  new_oper->dest[0] = L_new_macro_operand (mac, ctype, L_PTYPE_NULL);
  L_insert_oper_after (prologue_cb, prologue_cb->last_op,
		       new_oper);
  return new_oper;
}


static void
PL_spill_vararg_reg_params (FuncDcl fn, L_Cb *prologue_cb, 
			    lvars_t *vars, int first_va, int thru_reg)
{
  int i, ip_offset, vcnt, st_opc = 0;
  int incr;
  Expr shadow_expr;

  vcnt = thru_reg - (vars->size - 1); /* Don't count the ellipsis */
      
  if (vcnt <= 0)
    return;

  incr = PL_MType_Size(PL_native_int_reg_mtype) / 8;

  if (incr == 4)
    st_opc = Lop_ST_I;
  else if (incr == 8)
    st_opc = Lop_ST_Q;
  else
    P_punt ("PL_spill_vararg_reg_params: Unsupported store size %d", incr);

  ip_offset = (M_arch == M_TAHOE) ? (incr * (2 - vcnt)) : 
    (M_arch == M_ARM) ? (incr * (-vcnt)) :
    (incr * first_va);

  shadow_expr = PL_get_shadow (fn, first_va);

  for (i = first_va; i < thru_reg; i++)
    {
      L_Oper *store_op;
      L_Attr *attr;

      store_op = PL_new_loper (shadow_expr, st_opc);
      store_op->src[0] = L_new_macro_operand (L_MAC_IP,
					      PL_native_int_reg_ctype,
					      L_PTYPE_NULL);
      store_op->src[1] = L_new_gen_int_operand (ip_offset);
      store_op->src[2] = L_new_macro_operand (L_MAC_P0 + i,
					      PL_native_int_reg_ctype,
					      L_PTYPE_NULL);

      store_op->flags = L_SET_BIT_FLAG (store_op->flags, L_OPER_SAFE_PEI);
      store_op->flags = L_SET_BIT_FLAG (store_op->flags,
					L_OPER_STACK_REFERENCE);

      attr = L_new_attr (STACK_ATTR_NAME, 2);
      attr->field[0] = L_new_macro_operand (L_MAC_IP,
					    PL_native_int_reg_ctype,
					    L_PTYPE_NULL);
      attr->field[1] = L_new_gen_int_operand (ip_offset);
      store_op->attr = L_concat_attr (store_op->attr, attr);

      attr = L_new_attr ("VIPSPILL", 0);
      store_op->attr = L_concat_attr (store_op->attr, attr);
      L_insert_oper_after (prologue_cb, prologue_cb->last_op, 
			   store_op);

      ip_offset += incr;
    }
  return;
}


L_Flow *
PL_connect_cbs (L_Cb * src_cb, L_Cb * dst_cb, int cc, double weight)
{
  L_Flow *fl;

  fl = L_new_flow (cc, src_cb, dst_cb, weight);
  src_cb->dest_flow = L_concat_flow (src_cb->dest_flow, fl);
  dst_cb->src_flow = L_concat_flow (dst_cb->src_flow,
				    L_copy_single_flow (fl));
  return fl;
}


L_Oper *
PL_add_jump (L_Cb * src_cb, L_Cb * dst_cb)
{
  L_Oper *jmp;

  jmp = PL_new_loper (NULL, Lop_JUMP);
  jmp->src[0] = L_new_cb_operand (dst_cb);
  L_insert_oper_after (src_cb, src_cb->last_op, jmp);
  return jmp;
}


void
PL_terminate_goto (PC_Block src_bb)
{
  PC_Flow pfl;
  L_Cb *src_cb, *dst_cb;
  int ft;

  if (!(pfl = src_bb->s_flow) || pfl->s_next_flow || src_bb->cond)
    P_punt ("PtoL: Malformed GOTO");

  src_cb = (L_Cb *) src_bb->ext;
  dst_cb = (L_Cb *) pfl->dest_bb->ext;

  ft = (dst_cb == src_cb->next_cb);

  PL_connect_cbs (src_cb, dst_cb, !ft, pfl->weight);

  if (!ft)
    PL_add_jump (src_cb, dst_cb);

  return;
}


void
PL_terminate_if (PC_Block src_bb)
{
  int val, ft0, ft1;
  PC_Flow pfl0, pfl1 = NULL;
  L_Cb *src_cb, *dst_cb0, *dst_cb1;

  if (!(pfl0 = src_bb->s_flow) ||
      !(pfl1 = pfl0->s_next_flow) || pfl1->s_next_flow || !src_bb->cond)
    P_punt ("PtoL: Malformed IF (structural)");

  src_cb = (L_Cb *) src_bb->ext;

  val = (int) P_IntegralExprValue (pfl0->flow_cond);

  if (val != 0)
    {
      pfl0 = pfl1;
      pfl1 = src_bb->s_flow;
    }

  if (P_IntegralExprValue (pfl1->flow_cond) != 1)
    P_punt ("PtoL: Malformed IF (cc)");

  dst_cb0 = (L_Cb *) pfl0->dest_bb->ext;
  dst_cb1 = (L_Cb *) pfl1->dest_bb->ext;

  ft0 = (src_cb->next_cb == dst_cb0);
  ft1 = (src_cb->next_cb == dst_cb1);

  /* if (x) then DST1 else DST0 */

  if (ft0 || !ft1)
    {
      /* br (x) DST1; [jump DST0] */
      PL_connect_cbs (src_cb, dst_cb1, 1, pfl1->weight);
#if 0
      PLI_gen_if (src_cb, dst_cb1, src_bb->cond, src_bb->pragma, 0);
#else
      /* TM: NULL for pragma coz we don't want to transfer this to Lcode. */
      PLI_gen_if (src_cb, dst_cb1, src_bb->cond, NULL, 0);
#endif
      PL_connect_cbs (src_cb, dst_cb0, 0, pfl0->weight);
      if (!ft0)
	PL_add_jump (src_cb, dst_cb0);
    }
  else
    {
      /* br (!x) DST0; */
      PL_connect_cbs (src_cb, dst_cb0, 1, pfl0->weight);
#if 0
      PLI_gen_if (src_cb, dst_cb0, src_bb->cond, src_bb->pragma, 1);
#else
      /* TM: Pass NULL for pragma coz we don't want to transfer this to Lcode. */
      PLI_gen_if (src_cb, dst_cb0, src_bb->cond, NULL, 1);
#endif
      PL_connect_cbs (src_cb, dst_cb1, 0, pfl1->weight);
    }

  return;
}


typedef struct _PL_Case
{
  int idx;
  ITintmax val;
  L_Cb *dst;
  double wgt;
}
PL_Case;


static int switch_id = 0;


static void
PLI_gen_switch_br_seq (L_Cb * src_cb, PL_Case *cases, int n_cases,
		       PL_Case * dflt, PL_Ret cc)
{
  int i;
  L_Oper *new_oper;

  for (i = 0; i < n_cases; i++)
    {
      new_oper = PL_new_loper (NULL, Lop_BR);
      L_set_compare (new_oper, L_CTYPE_INT, Lcmp_COM_EQ);
      new_oper->src[0] = PL_gen_operand (&(cc->op1));

      new_oper->src[1] = L_propagate_sign_size_ctype_info ?
	L_new_int_operand (cases[i].val, new_oper->src[0]->ctype) :
	L_new_gen_int_operand (cases[i].val);

      new_oper->src[2] = L_new_cb_operand (cases[i].dst);
      L_insert_oper_after (src_cb, src_cb->last_op, new_oper);

      /* Add a "SWITCH" attribute to these branches, so the branch
       * predictor will know that these branches are associated with a
       * switch statement.
       *
       * The SWITCH attribute is now used in Lsuperscalar switch
       * optimization. -- JWS
       */
      if (PL_generate_static_branch_attrs)
	{
	  L_Attr *attr = L_new_attr ("SWITCH", 1);
	  L_set_int_attr_field (attr, 0, switch_id);
	  new_oper->attr = L_concat_attr (new_oper->attr, attr);
	}
    }

  switch_id++;

  new_oper = PL_new_loper (NULL, Lop_JUMP);
  L_insert_oper_after (src_cb, src_cb->last_op, new_oper);
  new_oper->src[0] = L_new_cb_operand (dflt->dst);

  return;
}


static void
PLI_gen_switch_hash (L_Cb * src_cb, PL_Case *cases, int n_cases,
		     PL_Case * dflt, PL_Ret cc, Expr cond_expr,
		     ITintmax min_val, ITintmax max_val)
{
  char *hash_name;
  L_Oper *new_oper;
  L_Data *new_data;
  L_Expr *new_expr;
  _PL_Operand Oindex, Omin, Odest;
  int i, ld_opc, ptype = 0, esize, entries;

  hash_name = new_hash_table (L_fn->name);

  /** generate target address **/

  PL_new_int (&Omin, min_val, 0);
  PL_new_register (&Oindex, PL_next_reg_id (), M_TYPE_POINTER, 0);

  PL_gen_sub (src_cb, &Oindex, &(cc->op1), &Omin, 0, NULL);
  PL_new_int (&Omin, P_POINTER_SIZE / P_CHAR_SIZE, 0);
  Odest = Oindex;
  PL_gen_mul (src_cb, &Odest, &Oindex, &Omin, NULL, 0, NULL);
  Oindex = Odest;

  /** check lower bound **/

  new_oper = PL_new_loper (NULL, Lop_BR);
  L_set_compare (new_oper, L_CTYPE_INT, Lcmp_COM_LT);
  new_oper->src[0] = PL_gen_operand (&(cc->op1));
  new_oper->src[1] = L_new_gen_int_operand (min_val);
  new_oper->src[2] = L_new_cb_operand (dflt->dst);
  L_insert_oper_after (src_cb, src_cb->last_op, new_oper);

  /** check upper bound **/

  new_oper = PL_new_loper (NULL, Lop_BR);
  L_set_compare (new_oper, L_CTYPE_INT, Lcmp_COM_GT);
  new_oper->src[0] = PL_gen_operand (&(cc->op1));
  new_oper->src[1] = L_new_gen_int_operand (max_val);
  new_oper->src[2] = L_new_cb_operand (dflt->dst);
  L_insert_oper_after (src_cb, src_cb->last_op, new_oper);

  /** fetch target address */

  Odest = Oindex;

  switch (P_POINTER_SIZE)
    {
    case 64:
      ld_opc = Lop_LD_Q;
      ptype = L_INPUT_WQ;
      break;
    case 32:
    case 16:
      ld_opc = Lop_LD_I;
      ptype = L_INPUT_WI;
      break;
    default:
      ld_opc = 0;
      P_punt ("Invalid P_POINTER_SIZE");
    }

  new_oper = PL_new_loper ((void *) (-1), ld_opc);
  new_oper->dest[0] = PL_gen_operand (&Odest);
  new_oper->src[0] = L_new_gen_label_operand (hash_name);
  new_oper->src[1] = PL_gen_operand (&Oindex);
  L_insert_oper_after (src_cb, src_cb->last_op, new_oper);

  /** generate a jump **/
  new_oper = PL_new_loper (NULL, Lop_JUMP_RG);

  new_oper->src[0] = PL_gen_operand (&Odest);
  new_oper->src[1] = PL_gen_operand (&(cc->op1));
  L_insert_oper_after (src_cb, src_cb->last_op, new_oper);

  /* Generate hash table */

  esize = P_POINTER_SIZE / P_CHAR_SIZE;

  PL_invalidate_last_ms();
  PL_ms (L_hash_datalist, "data");

  new_data = L_new_data (L_INPUT_ALIGN);
  new_data->N = esize;
  new_expr = L_new_expr (L_EXPR_STRING);
  new_expr->value.s = C_findstr (hash_name);
  new_data->address = new_expr;

  entries = max_val - min_val + 1;

  L_concat_datalist_element (L_hash_datalist,
			     L_new_datalist_element (new_data));

  new_data = L_new_data (L_INPUT_RESERVE);
  new_data->N = entries * esize;

  L_concat_datalist_element (L_hash_datalist,
			     L_new_datalist_element (new_data));


  /* Fill in the table with appropriate targets.  If no
   * CC record exists for a particular entry, fill in the
   * default target.
   */

  for (i = min_val; i <= max_val; i++)
    {
      int k;
      L_Expr *addr, *tgt;
      char *lbl;

      for (k = 0; k < n_cases; k++)
	if (cases[k].val == i)
	  break;

      if (k != n_cases)
	lbl = cb_label_name (L_fn->name, cases[k].dst->id);
      else
	lbl = cb_label_name (L_fn->name, dflt->dst->id);

      tgt = L_new_expr_label_no_underscore (lbl);
      addr = L_new_addr_no_underscore (hash_name, (i - min_val) * esize);

      new_data = L_new_data_w (ptype, addr, tgt);

      L_concat_datalist_element (L_hash_datalist,
				 L_new_datalist_element (new_data));
    }

  PL_invalidate_last_ms();

  return;
}


static int
PL_compare_case (const void *a, const void *b)
{
  PL_Case *ca = (PL_Case *) a, *cb = (PL_Case *) b;

  if (ca->wgt < cb->wgt)
    return 1;
  else if ((ca->wgt > cb->wgt) || (a < b))	/* stability */
    return -1;
  else
    return 1;
}

void
PL_terminate_switch (PC_Block src_bb)
{
  PC_Flow fl;
  Expr cond_expr = src_bb->cond;
  int i, n_cases, n_dflt, use_hash;
  PL_Case *cases;
  PL_Case dflt;
  L_Cb *src_cb = (L_Cb *) src_bb->ext;
  ITintmax min_val = 0x1FFFFFFF, max_val = -0x1FFFFFFF;
  double total_wgt = 0.0, br_seq_cost = 0.0;
  _PL_Ret cc;

  n_cases = n_dflt = 0;

  for (fl = src_bb->s_flow; fl; fl = fl->s_next_flow)
    {
      if (IsDefaultExpr (fl->flow_cond))
	n_dflt++;
      else
	n_cases++;
    }

  if (n_dflt != 1)
    P_punt ("PtoL PL_terminate_switch(): Switch w/ <> 1 default case");

  cases = alloca (n_cases * sizeof (PL_Case));

  for (i = 0, fl = src_bb->s_flow; fl; fl = fl->s_next_flow)
    {
      if (IsDefaultExpr (fl->flow_cond))
	{
	  /* default case goes in 0 */
	  dflt.val = 0;
	  dflt.dst = (L_Cb *) fl->dest_bb->ext;
	  dflt.wgt = fl->weight;
	  dflt.idx = -1;
	}
      else
	{
	  cases[i].val = P_IntegralExprValue (fl->flow_cond);
	  cases[i].dst = (L_Cb *) fl->dest_bb->ext;
	  cases[i].wgt = fl->weight;
	  cases[i].idx = i;
	  i++;
	}
      total_wgt += fl->weight;
    }

  /* Sort arcs according to profile weight */

  qsort (cases, n_cases, sizeof (PL_Case), PL_compare_case);  

#if 0
  fprintf (stderr, "SWITCH STMT\n");
  for (i = 0; i < n_cases; i++)
    {
      fprintf (stderr, "CASE[%d]: cb %d wt %f\n", cases[i].idx,
	       cases[i].dst->id, cases[i].wgt);
    }

  fprintf (stderr, "DFLT: cb %d wt %f\n", dflt.dst->id, dflt.wgt);
#endif

  /* Compute cost of branch sequence and bounds */

  for (i = 0; i < n_cases; i++)
    {
      br_seq_cost += (i + 1) * cases[i].wgt;
      if (cases[i].val < min_val)
	min_val = cases[i].val;
      if (cases[i].val > max_val)
	max_val = cases[i].val;
    }

  br_seq_cost = (total_wgt <= 0.0) ? 1.0e100 : (br_seq_cost / total_wgt);

  /* Decide whether or not to implement as hashed (indirect) branch */

  if (PL_generate_hashing_branches && (n_cases >= PL_MIN_HASH_JUMP_CASE) &&
      ((src_bb->weight >= PL_MIN_HASH_JUMP_WEIGHT) ||
       PL_ignore_hash_profile_weight) &&
      ((br_seq_cost > PL_MAX_BR_SEQUENCE_WEIGHT) ||
       PL_ignore_hash_br_seq_weight) &&
      ((max_val - min_val) <= PL_MAX_HASH_JUMP_SIZE))
    {
      use_hash = 1;
    }
  else
    {
      use_hash = 0;
    }

  /* Hook up flows */

  if (use_hash)
    {
      /* Flows for bounds-checking branches */
      PL_connect_cbs (src_cb, dflt.dst, 1, 0.0);
      PL_connect_cbs (src_cb, dflt.dst, 1, 0.0);
    }

  for (i = 0; i < n_cases; i++)
    {
      PL_connect_cbs (src_cb, cases[i].dst, cases[i].val, cases[i].wgt);
    }

  PL_connect_cbs (src_cb, dflt.dst, DEFAULT_VALUE, dflt.wgt);

  /* Generate branches */

  PLI_gen_data (src_cb, cond_expr, &cc);
  PLI_simplify (src_cb, &cc);

  if (!use_hash)
    {
      PLI_gen_switch_br_seq (src_cb, cases, n_cases, &dflt, &cc);
    }
  else
    {
      PLI_gen_switch_hash (src_cb, cases, n_cases, &dflt, &cc,
			   cond_expr, min_val, max_val);
    }

  return;
}


void
PL_terminate_return (PC_Block src_bb, L_Cb * epilogue_cb)
{
  L_Cb *src_cb;
  int ft;
  double wt = src_bb->weight;
  src_cb = (L_Cb *) src_bb->ext;

  ft = (epilogue_cb == src_cb->next_cb);

  PL_connect_cbs (src_cb, epilogue_cb, !ft, wt);
  epilogue_cb->weight += wt;

  if (!ft)
    PL_add_jump (src_cb, epilogue_cb);

  return;
}

void append_select_all_attr(L_Attr **attr, Pragma p)
{
  Pragma temp = p;
  int flag = 0;
  while(temp) {
    //if(0 == strncmp(temp->specifier, "LOOP", 4)) { /*Use this condition if you want to if-convert everything */
    if(0 == strncmp(temp->specifier, "innerloop", 9)) {
      flag = 1;
      break;
    }
    temp = temp->next;
  }
  if(flag == 1) {
    printf("Flag was 1\n");
    if(!(*attr)) {
      *attr = L_new_attr("HB_select_all", 0);
    }
    else {
      L_concat_attr(*attr, L_new_attr("HB_select_all", 0));
    }
  }
}

void PL_translate_longlong(L_Func *);

/*
 *	1. need to put a prologue immediately after the function header.
 *	2. need to put an epilogue immediately after the most important
 *		rts block.
 */
void
PL_gen_func (FuncDcl fn)
{
  PC_Block bb;
  L_Cb *prologue_cb, *first_cb, *epilogue_cb;
  Expr return_expr;

  /*** temporary variables ***/
  int j, r, ctype, offset, tsize;
  char temp_name[5000], *temp_ptr;
  char *func_name;

  L_Oper *new_oper;
  L_Attr *new_attr;

  int tr_count = 0;
  int tmo_count = 0;

  int old_style_param;		/* BCC - 8/23/96 */
  int stk_size_lv, stk_size_ip, param_space;
  int max_thru_reg = 0;
  int first_param_reg_spill = 0;
  int vararg_param = 0, first_vararg = 0;

  Key return_type;
  int rv_is_aggr, rv_is_indir_aggr = 0;
  int rv_is_longlong = 0;
  int return_addr_reg;		/* the address register of caller space */
  int actual_space;

  _M_Param rv_param;

  /* IA64 support JWS 20000517 */

  int tmcount = L_TM_START_VALUE;
  
  L_Attr *ipspill_attr = NULL;

  List LocalVarList = NULL;
  VarDcl var = NULL;

  lvars_t local_vars = {NULL, NULL, 0, 0};
  lvars_t formal_vars = {NULL, NULL, 0, 0};
  lvar_info_t *vinfo = NULL;

  // mchu - create hash table
  if(PL_annotate_bitwidths)
    operandHash = HashTable_create (4096);

  /* CWL - 12/24/00 for P-to-L.
   * Link all local variables together for later processing.
   */

  LocalVarList = PL_LinkLocalDataDcl (fn);
  
  if (!fn || !fn->name)
    P_punt ("PL_gen_func : nil or nameless input");

  func_name = PL_fmt_var_name(fn->name, fn->key);

  /*  
   * Generate CF information if not done yet.
   */

  PC_Function (fn, 1, PL_normalize_loops ? PC_NORM_LOOPS : 0);

  /*
   *  Clear all information from the previous function.
   */

  PL_init_reg_id ();
  next_hash_tbl_id = 0;		/* SAM 8-96 */
  /*
   *  Generate global (static) variables hidden in the function.
   */
  List_start(LocalVarList);
  while ((var = List_next(LocalVarList)))
    {
      if (P_GetVarDclQualifier(var) & VQ_STATIC)
	PL_gen_var (L_datalist, var);
    }
  /*
   *  Generate function header.
   */

  PL_invalidate_last_ms();
  PL_ms (L_datalist, "text");

  /* Generate return type
   */
  assert(PST_GetTypeBasicType(PL_symtab, fn->type) & BT_FUNC);
  return_type = PST_GetTypeType(PL_symtab, fn->type);

  sprintf (temp_name, "_%s", func_name);
  temp_ptr = C_findstr (temp_name);

  {
    L_Data *new_data = L_new_data (L_INPUT_GLOBAL);

    new_data->address = L_new_expr (L_EXPR_LABEL);
    new_data->address->value.l = temp_ptr;
    /* LCW - insert L_Type in L_Data structure - 4/25/96 */
    if (PL_emit_source_info)
      new_data->h_type = L_gen_type (return_type);
    L_concat_datalist_element (L_datalist,
			       L_new_datalist_element (new_data));
  }

  rv_is_aggr = PL_is_aggr_type (return_type, &return_type);

  rv_is_longlong = PST_IsLongLongType(PL_symtab, return_type);

  PL_pcode2lcode_type (return_type, &(rv_param.mtype), 0);

  if (rv_is_aggr)
    {
      M_layout_retvar (&rv_param, M_GET_FNVAR);
      rv_is_indir_aggr = !PL_gen_compliant_struct_return ||
	(rv_param.mode != M_THRU_REGISTER);
    }

  /* CWL - 11/28/00
   * In ProfFN of Pcode, there is a "count" field, but no "weight" field.
   * In ProfFN of Hcode, there is a "weight" field. 
   * Another difference between Pcode and Hcode is the "profile" field
   * in FuncDcl. In Pcode, is a pointer, but in Hcode it is not.
   */

  L_fn =
    L_new_func (temp_ptr,
		(fn->stmt->profile ? fn->stmt->profile->count : 0.0));

  L_fn->attr =
    L_concat_attr (L_fn->attr, PL_gen_attr_from_pragma (fn->pragma));

  /* CWL - 01/08/02 for P-to-L.
   * change var->next to var->local_next.
   * may combine this loop with PL_LinkLocalDataDcl.
   */

  /* Generate types for local variables
   */
  List_start(LocalVarList);
  while ((var = List_next(LocalVarList)))
    {
      if (!(P_GetVarDclQualifier(var) & (VQ_STATIC | VQ_EXTERN)))
	{
	  lvar_info_t *vinfo;

	  vinfo = PL_add_to_lvarlist(&local_vars, var);
	  vinfo->is_lvar = 1;
	  PL_pcode2lcode_type (var->type, &vinfo->mtype, 0);
	}
    }

  old_style_param = P_FindPragma (fn->pragma, "\"old_style_param\"") ? 1 : 0;

  List_start(fn->param);
  while ((var = List_next(fn->param)))
    {
      lvar_info_t *vinfo;
      _M_Type mtype;

      vinfo = PL_add_to_lvarlist(&formal_vars, var);
      vinfo->is_param = 1;
      PL_pcode2lcode_type (var->type, &vinfo->mtype, 1);

      /* 
       * BCC - 8/23/96
       * for functions using old-style parameters, floats are still passed 
       * via doubles
       */

      if (old_style_param && mtype.type == M_TYPE_FLOAT)
	M_double (&vinfo->mtype, vinfo->mtype.unsign);
    }

  /*
   *  Determine which local variables can be safely allocated
   *  to registers.
   */

  find_allocatable (&local_vars);
  find_allocatable (&formal_vars);

  /*
   * JWS: Allocate blocks, including function prologue 
   */

  /* Create prologue */

  {
    double wt;

    if (fn->stmt && fn->stmt->profile)
      wt = fn->stmt->profile->count;
    else if (fn->profile)
      wt = fn->profile->count;
    else
      wt = 0.0;

    prologue_cb = L_create_cb (wt);
  }

  L_insert_cb_after (L_fn, L_fn->last_cb, prologue_cb);

  /* Create body blocks */

  for (bb = PC_cfg->first_bb; bb; bb = bb->next)
    {
      L_Cb *cb = L_create_cb (bb->weight);
      L_insert_cb_after (L_fn, L_fn->last_cb, cb);
      bb->ext = (void *) cb;
    }

  first_cb = (L_Cb *) PC_cfg->first_bb->ext;

  /* Create and set up epilogue */

  epilogue_cb = L_create_cb (0.0);
  L_insert_cb_after (L_fn, L_fn->last_cb, epilogue_cb);
  new_oper = PL_new_loper (NULL, Lop_EPILOGUE);
  L_insert_oper_after (epilogue_cb, epilogue_cb->last_op, new_oper);

  new_oper = PL_new_loper (NULL, Lop_RTS);
  L_insert_oper_after (epilogue_cb, epilogue_cb->last_op, new_oper);

  if (!M_return_value_thru_stack()) 
    {

      if (rv_is_aggr && !rv_is_indir_aggr)
        {
          /* RVFILL is handled by PLI_gen_return */
          int i, nreg = rv_param.su_ereg - rv_param.su_sreg + 1;
      
          new_oper->attr = L_new_attr ("tr", nreg); /* same attr string as jsr */

          for (i = 0; i < nreg; i++)
	    {
	      L_set_macro_attr_field (new_oper->attr, i, 
                                      L_MAC_P0 + rv_param.su_sreg + i,
		  		      PL_native_int_reg_ctype,
				      L_PTYPE_NULL);
	    }
        }
      else if (!PST_IsVoidType(PL_symtab, return_type))
        {
          int mtype_type = PL_key_get_mtype(return_type);
          int rid = L_MAC_P0 + M_return_register (mtype_type, M_GET_FNVAR);
      
          new_oper->attr = L_new_attr ("tr", 1); /* same attr string as jsr */
          L_set_macro_attr_field (new_oper->attr, 0, rid,
			          PL_key_get_regctype (return_type),
			          L_PTYPE_NULL);
        }
    }

  /* Set up prologue */

  prologue_cb->dest_flow = L_new_flow (0, prologue_cb, first_cb,
				       prologue_cb->weight);
  first_cb->src_flow = L_copy_flow (prologue_cb->dest_flow);

  {
    L_Attr *tr_attr = NULL;
    L_Attr *tro_attr = NULL;
    L_Attr *trse_attr = NULL;
    L_Attr *tm_attr = NULL;
    L_Attr *tmo_attr = NULL;
    L_Attr *tms_attr = NULL;
    L_Attr *tmso_attr = NULL;
    L_Attr *trsof_attr = NULL;

    /*
     *  Decide how to layout the local variables and formal parameters.
     */
    {
      int returns_ST = 0;
      /* zzz
       * If ARM functions return a union/struct, the pointer to the
       * return value is passed in through the first param
       * register. This causes the other arguments to get bumped up, and
       * we'll need 4 bytes of extra stack space.
       */
      if(M_arch == M_ARM)
	if(rv_is_aggr)
	  returns_ST = 1;

      param_space = PL_layout_formal (&formal_vars, returns_ST);
    }

    {
      L_Attr *parm_attr = L_new_attr ("formal_param_size", 0);
      L_set_int_attr_field (parm_attr, 0, param_space);
      L_fn->attr = L_concat_attr (L_fn->attr, parm_attr);
    }

    stk_size_lv = PL_layout_local (&local_vars);

    switch (M_arch)
      {
      case M_IMPACT:
	max_thru_reg = 4;
	break;
      case M_PLAYDOH:
	max_thru_reg = 4;
	break;
      case M_ARM:
	max_thru_reg = 4;
	break;
      case M_TAHOE:
	max_thru_reg = 8;
	break;
      default:
	max_thru_reg = 0;
	break;
      }

    first_param_reg_spill = max_thru_reg;

    /*
     * ipspill attribute -- JWS 20010302
     * ----------------------------------------------------------------------
     * "bit" vector for eight register-bound input parameters 
     * 1 = register must be spilled to stack (vararg or structure)
     * 0 = register need not be spilled, and space does not need to be
     *     reserved
     */

    ipspill_attr = L_new_attr ("ipspill", max_thru_reg);
    for (j = 0; j < max_thru_reg; j++)
      L_set_int_attr_field (ipspill_attr, j, 0);

    {
      /* For a vararg function, annotate position of first
       * vararg argument and mark parameters passed through registers
       * for spill.
       */
      lvar_info_t *varg_vinfo = NULL;
      
      first_vararg = 0;
      List_start(fn->param);
      while ((var = List_next(fn->param)))
	{
	  if (!strcmp (var->name, "__builtin_va_alist") ||
	      !strcmp (var->name, "__builtin_impact_ellipsis"))
	    {
	      vararg_param = 1;
	      varg_vinfo = PL_find_vartbl_entry(var->key);
	      varg_vinfo->is_vararg_param = 1;
	      break;
	    }
	  first_vararg++;
	}
      
      if (vararg_param)
	{
	  /* Add VARARG attribute.  VARARG constant
	   * indicates the location of the first vararg parameter.
	   */
	  
	  new_attr = L_new_attr ("VARARG", 1);
	  L_set_int_attr_field (new_attr, 0,
				(M_arch == M_TAHOE) ? first_vararg :
				(M_arch == M_ARM) ? first_vararg * 4 - 16 :
				varg_vinfo->offset);
	  L_fn->attr = L_concat_attr (L_fn->attr, new_attr);
	  
	  if (!tr_attr)
	    {
	      tr_attr = L_new_attr ("tr", 0);
	      L_fn->attr = L_concat_attr (L_fn->attr, tr_attr);
	      tro_attr = L_new_attr ("tro", 0);
	      L_fn->attr = L_concat_attr (L_fn->attr, tro_attr);
	    }
	  
	  /* Mark register-passed varargs for spilling */
	  for (j = first_vararg; j < max_thru_reg; j++)
	    {
	      if (j < first_param_reg_spill)
		first_param_reg_spill = j;
	      
	      L_set_int_attr_field (ipspill_attr, j, 1);
	      
	      L_set_macro_attr_field (tr_attr, j,
				      (L_MAC_P0 + j),
				      PL_native_int_reg_ctype, L_PTYPE_NULL);
	    }

	  formal_vars.r_count = max_thru_reg;
	}

      new_attr = L_new_attr ("ip", 0);
      L_fn->attr = L_concat_attr (L_fn->attr, new_attr);
      L_set_int_attr_field (new_attr, 0, formal_vars.r_count);
    }

    /*
     *  Find maximum space that we need for pushing actual parameters.
     */

    actual_space = compute_actual_space (fn);

    /*
     * LCW - put local variables and parameters information in function
     * attribute - 5/20/96
     */

    if (PL_emit_source_info)
      gen_local_info_attribute (&local_vars, &formal_vars);

    /*
     *  Print parameter types.
     */

    for (vinfo = formal_vars.head; vinfo; vinfo = vinfo->nxt_var)
      {
	if (vinfo->is_vararg_param)
	  break;

	switch (vinfo->mode)
	  {
	  case M_THRU_REGISTER:
	    ctype =
	      PL_parm_ctype (vinfo->mtype.type, vinfo->mtype.unsign);

#ifndef PL_GEN_FLOAT_OPERANDS
	    if (ctype == L_CTYPE_FLOAT)
	      ctype = L_CTYPE_DOUBLE;
#endif

	    /* tr = thru register parameter */
	    /* tro = thru register offset for cases where parameter
	       thru-register must be written back to memory */

	    if (!tr_attr)
	      {
		tr_attr = L_new_attr ("tr", 0);
		L_fn->attr = L_concat_attr (L_fn->attr, tr_attr);
		tro_attr = L_new_attr ("tro", 0);
		L_fn->attr = L_concat_attr (L_fn->attr, tro_attr);
	      }

	    if (vinfo->mtype.type == M_TYPE_UNION ||
		vinfo->mtype.type == M_TYPE_STRUCT)
	      {
		/* Set thru register range start end (trse) */
		if (!trse_attr)
		  {
		    trse_attr = L_new_attr ("trse", 0);
		    L_fn->attr = L_concat_attr (L_fn->attr, trse_attr);
		  }
		L_set_int_attr_field (trse_attr, 2 * vinfo->index,
				      vinfo->su_sreg);
		L_set_int_attr_field (trse_attr, 2 * vinfo->index + 1, 
				      vinfo->su_ereg);

		tsize = PL_MType_Size(PL_native_int_reg_mtype) / 8;

		for (r = vinfo->su_sreg; r <= vinfo->su_ereg; r++)
		  {
		    PL_define_incoming_reg (prologue_cb, L_MAC_P0 + r, ctype);

		    L_set_macro_attr_field (tr_attr, tr_count, L_MAC_P0 + r,
					    ctype, L_PTYPE_NULL);
		    tr_count++;
		  }

		if(M_arch == M_TAHOE) {
		  if (vinfo->paddr < 6) {
		    stk_size_lv += (6 - vinfo->paddr) * tsize;
		  }
		} else if(M_arch == M_ARM) {
		  stk_size_lv += tsize * (vinfo->su_ereg - vinfo->su_sreg + 1);
		}
		

		/* This is the offset from which the copied struct
		   will start (relative to IP) */
		if (!tmso_attr)
		  {
		    tmso_attr = L_new_attr ("tmso", 0);
		    L_fn->attr = L_concat_attr (L_fn->attr, tmso_attr);
		  }
		if(M_arch == M_TAHOE)
		  L_set_int_attr_field (tmso_attr, vinfo->index,
					(vinfo->paddr - 6) * tsize);
		else if(M_arch == M_ARM)
		  L_set_int_attr_field (tmso_attr, vinfo->index,
					tsize * (vinfo->su_sreg) - 16);
		else
		  P_punt(" copying struct from param regs with unknown arch!\n");

		/* Mark register-passed portions as requiring spill */

		for (j = vinfo->su_sreg; j <= vinfo->su_ereg; j++)
		  {
		    if (j < first_param_reg_spill)
		      first_param_reg_spill = j;
		    L_set_int_attr_field (ipspill_attr, j, 1);
		  }

		/* This is the start and end offset that the struct
		   will occupy once copied into the stack */
		if (!trsof_attr)
		  {
		    trsof_attr = L_new_attr ("trsof", 0);
		    L_fn->attr = L_concat_attr (L_fn->attr, trsof_attr);
		  }
		if (M_arch == M_TAHOE) {
		  if ((vinfo->paddr - 6) * tsize < -48)
		    P_punt ("trsof smaller than -48\n");
		}

		if (M_arch == M_TAHOE) {
		  L_set_int_attr_field (trsof_attr, 2 * vinfo->index,
					(vinfo->paddr - 6) * tsize);
		  L_set_int_attr_field (trsof_attr, 2 * vinfo->index + 1,
					((vinfo->paddr - 6) * tsize +
					 vinfo->mtype.size / 8));
		} else if (M_arch == M_ARM) {
		  L_set_int_attr_field (trsof_attr, 2 * vinfo->index,
					(tsize * vinfo->su_sreg) - 16);
		  L_set_int_attr_field (trsof_attr, 2 * vinfo->index + 1,
					((tsize * vinfo->su_sreg) - 16 +
					 (vinfo->mtype.size / 8)));
		} else
		  P_punt(" copying struct from param regs with unknown arch!\n");

	      }
	    else
	      {
		PL_define_incoming_reg (prologue_cb, L_MAC_P0 + vinfo->reg,
					ctype);

		L_set_macro_attr_field (tr_attr, tr_count,
					(L_MAC_P0 + vinfo->reg), ctype,
					L_PTYPE_NULL);
		L_set_int_attr_field (tro_attr, tr_count, vinfo->offset);
		tr_count++;
	      }
	    break;

	  case M_INDIRECT_THRU_REGISTER:
	    /* I believe this case is used only for passing copies
	     * of structures to the function.  The address of the space
	     * the structure is passing in the register but then the
	     * structure is actually put on the stack after all the
	     * parameters (which is how it is accessed by Lcode).  The 
	     * thru-register part (which has the address) doesn't 
	     * seem to be actually used by Lcode, but is very useful 
	     * for Lemulate to find the copy of the structure. -ITI/JCG 3/99
	     */

	    PL_define_incoming_reg (prologue_cb, L_MAC_P0 + vinfo->reg,
				    L_CTYPE_INT);

	    /* Add explicit attribute to function to specify where
	     * indirect thru-register parameters are expected.  
	     * Simplifies Lemulate's job of emulating this code. -ITI/JCG 3/99
	     * 
	     * Added tro attribute update for consistency, but I am
	     * not sure if it is truely needed here (but better
	     * safe then sorry). -ITI/JCG 3/99
	     */
	    if (!tr_attr)
	      {
		tr_attr = L_new_attr ("tr", 0);
		L_fn->attr = L_concat_attr (L_fn->attr, tr_attr);
		tro_attr = L_new_attr ("tro", 0);
		L_fn->attr = L_concat_attr (L_fn->attr, tro_attr);
	      }
	    L_set_macro_attr_field (tr_attr, tr_count,
				    (L_MAC_P0 + vinfo->reg), L_CTYPE_INT,
				    L_PTYPE_NULL);
	    L_set_int_attr_field (tro_attr, tr_count, vinfo->offset);

	    /* Add explicit attribute to function to specify where
	     * the copy of the structure is place on the stack 
	     * (relative to the IP macro). -ITI/JCG 3/99 
	     */
	    if (!tmso_attr)
	      {
		tmso_attr = L_new_attr ("tmso", 0);
		L_fn->attr = L_concat_attr (L_fn->attr, tmso_attr);
	      }

	    L_set_int_attr_field (tmso_attr, vinfo->index, vinfo->paddr);

	    tr_count++;
	    break;

	  case M_THRU_MEMORY:
	    {
	      ctype = PL_key_get_parmctype (vinfo->var->type);
	      
	      new_oper = PL_new_loper (NULL, Lop_DEFINE);
	      new_oper->src[0] = L_new_macro_operand (L_MAC_IP,
						      L_CTYPE_INT,
						      L_PTYPE_NULL);
	      new_oper->src[1] = L_new_gen_int_operand (vinfo->offset);
	      new_oper->dest[0] = L_new_macro_operand (L_MAC_TM_TYPE,
						       ctype, L_PTYPE_NULL);
	      L_insert_oper_after (prologue_cb, prologue_cb->last_op, 
				   new_oper);
	      
	      new_oper->attr = L_new_attr ("tm", 1);
	      L_set_int_attr_field (new_oper->attr, 0, tmcount);
	      
	      /* Add explicit attribute to function to specify where
	       * thru-memory parameters are expected.  Simplifies Lemulate's
	       * job of emulating this code. -ITI/JCG 3/99
	       */
	      if (!tmo_attr)
		{
		  tm_attr = L_new_attr ("tm", 0);
		  L_fn->attr = L_concat_attr (L_fn->attr, tm_attr);
		  tmo_attr = L_new_attr ("tmo", 0);
		  L_fn->attr = L_concat_attr (L_fn->attr, tmo_attr);
		  tms_attr = L_new_attr ("tms", 0);
		  L_fn->attr = L_concat_attr (L_fn->attr, tms_attr);
		}
	      /* Use tmo_count as index, since tm_count has base 
	       * offset (of 300) added in.
	       */
	      L_set_int_attr_field (tm_attr, tmo_count, tmcount);
	      L_set_int_attr_field (tmo_attr, tmo_count, vinfo->offset);
	      L_set_int_attr_field (tms_attr, tmo_count,
				    vinfo->mtype.size / 8);
	      tmcount++;
	      tmo_count++;
	    }
	    break;

	    /* Added, since just dropped on the floor before -ITI/JCG 3/99 */
	  case M_INDIRECT_THRU_MEMORY:
	    /* This case is used only for passing copies of structures to 
	     * the function.  The address of the space the structure is 
	     * passing in thru memory where a pointer parameter would be
	     * placed but then the structure is actually put on the stack 
	     * after all the parameters (which is how it is accessed by 
	     * Lcode).  The thru-memory part (which has the address) 
	     * doesn't seem to be actually used by Lcode, but is very 
	     * useful for Lemulate to find the copy of the
	     * structure. -ITI/JCG 3/99
	     */
	    new_oper = PL_new_loper (NULL, Lop_DEFINE);
	    new_oper->src[0] = L_new_macro_operand (L_MAC_IP,
						    L_CTYPE_INT,
						    L_PTYPE_NULL);
	    new_oper->src[1] = L_new_gen_int_operand (vinfo->offset);
	    new_oper->dest[0] = L_new_macro_operand (L_MAC_TM_TYPE,
						     L_CTYPE_INT,
						     L_PTYPE_NULL);
	    new_oper->attr = L_new_attr ("tm", 1);
	    L_set_int_attr_field (new_oper->attr, 0, tmcount);
	    L_insert_oper_after (prologue_cb, prologue_cb->last_op, new_oper);

	    /* Add explicit attribute to function to specify where
	     * thru-memory parameters are expected.  Simplifies Lemulate's
	     * job of emulating this code. -ITI/JCG 3/99
	     */
	    if (!tmo_attr)
	      {
		tm_attr = L_new_attr ("tm", 0);
		L_fn->attr = L_concat_attr (L_fn->attr, tm_attr);
		tmo_attr = L_new_attr ("tmo", 0);
		L_fn->attr = L_concat_attr (L_fn->attr, tmo_attr);
		tms_attr = L_new_attr ("tms", 0);
		L_fn->attr = L_concat_attr (L_fn->attr, tms_attr);
	      }
	    /* Add explicit attribute to function to specify where
	     * the copy of the structure is place on the stack 
	     * (relative to the IP macro). -ITI/JCG 3/99 
	     */
	    if (!tmso_attr)
	      {
		tmso_attr = L_new_attr ("tmso", 0);
		L_fn->attr = L_concat_attr (L_fn->attr, tmso_attr);
	      }
	    L_set_int_attr_field (tmso_attr, vinfo->index, vinfo->paddr);

	    /* Use tmo_count as index, since tm_count has base 
	     * offset (of 300) added in.
	     */
	    L_set_int_attr_field (tm_attr, tmo_count, tmcount);
	    L_set_int_attr_field (tmo_attr, tmo_count, vinfo->offset);
	    L_set_int_attr_field (tms_attr, tmo_count,
				  vinfo->mtype.size / 8);
	    tmcount++;
	    tmo_count++;
	    break;

	  default:
	    break;
	  }
      }

    if (vararg_param)
      {
	int i;
	for (i = first_vararg; i < max_thru_reg; i++)
	  PL_define_incoming_reg (prologue_cb, L_MAC_P0 + i,
				  PL_native_int_reg_ctype);
      }

    if (ipspill_attr)
      L_fn->attr = L_concat_attr (L_fn->attr, ipspill_attr);

    {
      L_Attr *attr = L_new_attr ("stk_size_ip", 1);

      stk_size_ip = (max_thru_reg - first_param_reg_spill) *
	(PL_MType_Size(PL_native_int_reg_mtype) / 8);

      L_set_int_attr_field (attr, 0, stk_size_ip);
      L_fn->attr = L_concat_attr (L_fn->attr, attr);
    }
  }

  /*
   * return value
   * ----------------------------------------------------------------------
   */

  if (!M_return_value_thru_stack()) 
    {
      if (rv_is_indir_aggr)
	{
	  PL_define_incoming_reg (prologue_cb,
				  L_MAC_P0 + M_return_register (M_TYPE_STRUCT,
								M_PUT_FNVAR),
				  L_CTYPE_INT);

	  PL_define_incoming_reg (prologue_cb, L_MAC_RET_TYPE, L_CTYPE_INT);
	}
      else if (!PST_IsVoidType(PL_symtab, return_type))
	{
	  ctype = PL_key_get_parmctype(return_type);
#ifndef PL_GEN_FLOAT_OPERANDS
	  if (ctype == L_CTYPE_FLOAT)
	    ctype = L_CTYPE_DOUBLE;
#endif
	  
	  PL_define_incoming_reg (prologue_cb, L_MAC_RET_TYPE, ctype);
	}
    }
  /*
   *  Print define statements.
   */

  new_oper = PL_define_incoming_reg (prologue_cb, L_MAC_IP,
				     PL_native_int_reg_ctype);
  new_oper = PL_define_incoming_reg (prologue_cb, L_MAC_LV,
				     PL_native_int_reg_ctype);
  new_oper = PL_define_incoming_reg (prologue_cb, L_MAC_OP,
				     PL_native_int_reg_ctype);

  new_oper = PL_define_incoming_reg (prologue_cb, L_MAC_LOCAL_SIZE,
				     L_CTYPE_INT);
  new_oper->src[0] = L_new_gen_int_operand (stk_size_lv);

  new_oper = PL_define_incoming_reg (prologue_cb, L_MAC_PARAM_SIZE,
				     L_CTYPE_INT);
  new_oper->src[0] = L_new_gen_int_operand (actual_space);

  new_oper = PL_new_loper (NULL, Lop_PROLOGUE);
  L_insert_oper_after (prologue_cb, prologue_cb->last_op, new_oper);

  tmcount = L_TM_START_VALUE;

  /*
   *  Fix parameters.
   */

  for (vinfo = formal_vars.head; vinfo; vinfo = vinfo->nxt_var)
    {
      Expr shadow_expr;
      _PL_Operand dest, src1, src2, src3;
      
      if (vinfo->is_vararg_param)
	break;

      shadow_expr = PL_get_shadow (fn, vinfo->index);

#if EMN_DEBUG_SHADOW
      if (shadow_expr)
	printf ("SHADOW EXPR\n");
#endif

      if (vinfo->register_allocatable)
	{
	  vinfo->reg_id = PL_next_reg_id ();
	  switch (vinfo->mode)
	    {
	    case M_THRU_REGISTER:
	      {
		/*** r <- $Rreg ***/
		_M_Type mtype;
		char line[64];
		PL_pcode2lcode_type (vinfo->var->type, &mtype, 1);
		PL_new_register (&dest, vinfo->reg_id, mtype.type,
				 mtype.unsign);
		sprintf (line, "$P%d", vinfo->reg);
		PL_new_macro (&src1, line, vinfo->mtype.type,
			      vinfo->mtype.unsign);

		if ((PL_native_int_size > P_INT_SIZE) &&
		    (mtype.type == M_TYPE_INT))
		  PL_gen_cast (prologue_cb, &dest, &src1,
			       vinfo->mtype.unsign ? Lop_ZXT_I : Lop_SXT_I);
		else
		  PL_gen_mov (prologue_cb, &dest, &src1, 0);

		/*
		 *  type may have been changed.
		 */
		if (!PL_MType_Compatible(vinfo->mtype.type, mtype.type))
		  {
		    printf ("here\n");
		    vinfo->mtype = mtype;
		  }
		break;
	      }
	    case M_THRU_MEMORY:
	      {
		/*** r <- mem[vinfo->base_macro + offset] ***/
		_M_Type mtype;
		L_Attr *new_attr;
		/*
		 *  load into a register.
		 */
		PL_new_register (&dest, vinfo->reg_id,
				 vinfo->mtype.type, vinfo->mtype.unsign);

		PL_new_macro (&src1, (M_arch != M_TAHOE) ? 
			      vinfo->base_macro :
			      vinfo->tahoe_param_base_macro,
			      M_TYPE_POINTER, 0);
		PL_new_int (&src2, vinfo->offset, 0);

		new_attr = L_new_attr ("tm", 1);
		L_set_int_attr_field (new_attr, 0, tmcount++);
		PL_gen_load (prologue_cb, shadow_expr, &dest, &src1, &src2,
			     vinfo->mtype.unsign, new_attr);
		/*
		 *  change type if necessary.
		 */
		PL_pcode2lcode_type (vinfo->var->type, &mtype, 1);
		if (!PL_MType_Compatible(vinfo->mtype.type, mtype.type))
		  {
		    src1 = dest;
		    vinfo->reg_id = PL_next_reg_id ();
		    PL_new_register (&dest, vinfo->reg_id, mtype.type,
				     mtype.unsign);
		    PL_gen_mov (prologue_cb, &dest, &src1, 0);
		    vinfo->mtype = mtype;
		  }
		break;
	      }
	    case M_INDIRECT_THRU_REGISTER:
	    case M_INDIRECT_THRU_MEMORY:
	      P_punt ("do not allocate structures to registers");
	      break;
	    default:
	      P_punt ("illegal mode returned from M_fnvar_layout");
	    }
	  vinfo->mode = M_THRU_REGISTER;
	}
      else
	{			/* !vinfo->register_allocatable) */
	  _M_Type mtype;
	  int base = 0;

	  vinfo->reg_id = -1;
	  switch (vinfo->mode)
	    {
	    case M_THRU_REGISTER:
	      {
		/*** mem[vinfo->base_macro + offset] <- $Preg ***/
		char line[64];
		if (((M_arch == M_TAHOE) || (M_arch == M_ARM)) &&
		    (vinfo->mtype.type == M_TYPE_UNION ||
		     vinfo->mtype.type == M_TYPE_STRUCT))
		  {
		    int reg_id;
		    int is_unsigned = PST_IsUnsignedType(PL_symtab,
							  vinfo->var->type);

		    /* This loop generates moves from the params macros into
		       real registers. It also stores the real start and
		       end registers (as opposed to the start and end
		       macro regs */

		    vinfo->real_sreg = reg_id = PL_peek_next_reg_id ();
		    for (r = vinfo->su_sreg; r <= vinfo->su_ereg; r++)
		      {
			sprintf (line, "$P%d", r);
			PL_new_macro (&src1, line, PL_native_int_reg_mtype,
				      vinfo->mtype.unsign);
			PL_new_register (&src3, (reg_id = PL_next_reg_id ()),
					 PL_native_int_reg_mtype,
					 is_unsigned);
			PL_gen_mov (prologue_cb, &src3, &src1, 0);
		      }
		    vinfo->real_ereg = reg_id;

		    /* This generates the starting location of the
		       struct or rest of the struct */
		    tsize = PL_MType_Size(PL_native_int_reg_mtype) / 8;

		    /* Generate the stores from IP directly */
		    offset = 0;
		    if (M_arch == M_TAHOE)
		      base = (vinfo->paddr - 6) * tsize;
		    else if (M_arch  == M_ARM)
		      base = (tsize * vinfo->su_sreg) - 16;

		    PL_new_macro (&src1, "$IP", M_TYPE_POINTER, 
				  is_unsigned);

		    /* Do the appropriate copy of the struct param to
		       a local location */
		    for (r = vinfo->real_sreg; r <= vinfo->real_ereg; r++)
		      {
			PL_new_register (&src3, r, PL_native_int_reg_mtype,
					 is_unsigned);
			PL_new_int (&src2, base + offset, 0);
			PL_gen_store (prologue_cb, shadow_expr,
				      &src1, &src2, &src3,
				      src3.data_type,
				      L_new_attr ("IPSPILL", 0));
			offset += tsize;
		      }

		    /* Convert to THRU MEMORY, since it is now */
		    vinfo->mode = M_THRU_MEMORY;
		    vinfo->offset = base;
		  }
		else
		  {
		    sprintf (line, "$P%d", vinfo->reg);

		    PL_new_macro (&src3, line, vinfo->mtype.type,
				  vinfo->mtype.unsign);

		    /*
		     *  change type if necessary.
		     */
		    PL_pcode2lcode_type (vinfo->var->type, &mtype, 1);
		    if (!PL_MType_Compatible(vinfo->mtype.type, mtype.type))
		      {
			src1 = src3;
			PL_new_register (&src3, PL_next_reg_id (), mtype.type,
					 mtype.unsign);
			PL_gen_mov (prologue_cb, &src3, &src1, 0);
			vinfo->mtype = mtype;
		      }
		    if (M_arch != M_TAHOE)
		      {
			PL_new_macro (&src1, vinfo->base_macro,
				      M_TYPE_POINTER, 0);
		      }
		    else
		      {
			stk_size_lv = M_tahoe_fnvar_to_lvar (vinfo->mtype,
							     &vinfo->offset,
							     &vinfo->tahoe_param_base_macro,
							     stk_size_lv);
			PL_new_macro (&src1, vinfo->tahoe_param_base_macro,
				      M_TYPE_POINTER, 0);
		      }
		    
		    /*
		     * The reg assigned could be an int param, flt
		     * param, or double param, but in ARM these are
		     * all the same registers, so we need to set up
		     * the offset differently based on which param
		     * we're dealing with. This code is a big hack
		     * that shows very poor knowledge of software
		     * engineering :)
		     */
		    if (M_arch == M_ARM) {
		      /* int params */
		      if(vinfo->reg < 4)
			vinfo->offset = (4 * vinfo->reg) - 16;
		      /* float params */
		      else if(vinfo->reg > 4 && vinfo->reg < 9)
			vinfo->offset = (4 * (vinfo->reg - 5)) - 16;
		      /* double params */
		      else if(vinfo->reg >= 9)
			vinfo->offset = (4 * (vinfo->reg - 9)) - 16;
		    }

		    PL_new_int (&src2, vinfo->offset, 0);
#if 0
		    printf ("Storing parameter %s to %s + %d\n",
			    line, src1.value.mac, (int) src2.value.i);
#endif
		    PL_gen_store (prologue_cb, shadow_expr,
				  &src1, &src2, &src3,
				  vinfo->mtype.type,
				  L_new_attr ("IPSPILL", 0));
		    /*
		     *  change to memory.
		     */
		    vinfo->mode = M_THRU_MEMORY;
		  }
		break;
	      }
	    case M_THRU_MEMORY:
	      {
		/*
		 *  change type if necessary.
		 */
		PL_pcode2lcode_type (vinfo->var->type, &mtype, 1);
		if (!PL_MType_Compatible(vinfo->mtype.type, mtype.type))
		  {
		    /*
		     *  load into a register.
		     */
		    PL_new_register (&dest, PL_next_reg_id (),
				     vinfo->mtype.type,
				     vinfo->mtype.unsign);

		    PL_new_macro (&src1, (M_arch != M_TAHOE) ? 
				  vinfo->base_macro :
				  vinfo->tahoe_param_base_macro,
				  M_TYPE_POINTER, 0);

		    PL_new_int (&src2, vinfo->offset, 0);
		    PL_gen_load (prologue_cb, shadow_expr, &dest, &src1,
				 &src2, vinfo->mtype.unsign, 0);
		    /*
		     *  change type.
		     */
		    src3 = dest;
		    PL_new_register (&dest, PL_next_reg_id (), mtype.type,
				     mtype.unsign);
		    PL_gen_mov (prologue_cb, &dest, &src3, 0);
		    vinfo->mtype = mtype;
		    src3 = dest;
		    /*
		     *  store back to memory.
		     */

		    PL_gen_store (prologue_cb, shadow_expr,
				  &src1, &src2, &src3,
				  vinfo->mtype.type,
				  L_new_attr ("IPSPILL", 0));
		  }
		break;
	      }
	    case M_INDIRECT_THRU_REGISTER:
	    case M_INDIRECT_THRU_MEMORY:
	      {
		/*
		 *  keep in memory.
		 */
		break;
	      }
	    default:
	      P_punt ("illegal parameter mode returned by M_fnvar_layout");
	    }
	}

#if EMN_DEBUG_SHADOW
      if (shadow_expr)
	printf ("---------\n");
#endif
    }

  if (vararg_param)
    {
      for (vinfo = formal_vars.head; vinfo; vinfo = vinfo->nxt_var)
	{
	  if (vinfo->is_vararg_param)
	    break;
	}

      vinfo->mode = M_THRU_MEMORY;
      PL_spill_vararg_reg_params (fn, prologue_cb, 
				  &formal_vars, 
				  vinfo->index, 
				  max_thru_reg);
    }

  /*
   *  Handle special case of returning a structure.
   */

  if (rv_is_indir_aggr)
    {
      /*
       *  Need to save the address of the caller allocated space,
       *  which is assumed to be passed in through $Pn.
       */
      _PL_Operand src1, src2, dest;
      char line[64];
      return_addr_reg = PL_next_reg_id ();
      PL_new_register (&dest, return_addr_reg, M_TYPE_POINTER, 0);
      sprintf (line, "$P%d", M_structure_pointer (M_GET_FNVAR));
      PL_new_macro (&src1, line, M_TYPE_POINTER, 0);
      if (!M_return_value_thru_stack())
	{
	  PL_gen_mov (prologue_cb, &dest, &src1, 0);
	}
      else
        {
          PL_new_macro (&src1, "$IP", M_TYPE_POINTER, 0);
          PL_new_int(&src2, M_return_value_offset(), 0);
          PL_gen_load(prologue_cb, (void *)-1, &dest, &src1, &src2, M_TYPE_POINTER, 0);
        }
    }
  else
    {
      return_addr_reg = -1;
    }

#ifdef DEBUG_LAYOUT
  {
    int i;
    printf ("### actual parameter space = (%d bytes) \n", actual_space);
    printf ("### local variable space = (%d bytes) (%s) \n",
	    stk_size_lv, local_base_macro);

    for (i = 0; i < n_local; i++)
      printf ("\t%s : offset=%d, mode=%d, reg=%d\n",
	      local[i]->name, local_offset[i], local_mode[i], local_reg_id[i]);

    printf ("### formal space = (%d bytes) (%s) \n",
	    param_space, param_base_macro);
    for (i = 0; i < n_param; i++)
      printf ("\t%s : offset=%d, mode=%d, reg=%d\n",
	      param[i]->name, param_offset[i], param_mode[i], param_reg_id[i]);
  }
#endif

  /*
   *  Generate function body.
   */

  for (bb = PC_cfg->first_bb; bb; bb = bb->next)
    {
      PC_PStmt ps;
      L_Cb *new_cb = (L_Cb *) bb->ext;

      /*
       * I. Generate block attributes.
       */

      if (bb->pragma) {
	new_cb->attr = PL_gen_attr_from_pragma (bb->pragma);
        append_select_all_attr(&(new_cb->attr), bb->pragma);
      }

      /*
       * II. Generate block statements
       */

      for (ps = bb->first_ps; ps; ps = ps->succ)
	{
	  Expr expr;
	  if (ps->type == PC_T_Probe || !ps->data.stmt || 
	      ps->data.stmt->shadow)
	    continue;

	  switch (ps->data.stmt->type)
	    {
	    case ST_EXPR:
	      expr = ps->data.stmt->stmtstruct.expr;
	      PLI_gen_expr (new_cb, expr);
	      break;
	    case ST_RETURN:
	      expr = ps->data.stmt->stmtstruct.expr;
	      if (expr && expr->opcode != OP_null)
		{
		  if (PLI_gen_return (new_cb, expr,
				      return_type, return_addr_reg))
		    return_expr = expr;
		}
	      break;
	    case ST_NOOP:
	      break;
	    default:
	      P_warn ("PtoL: Unanticipated stmt type %d", ps->data.stmt->type);
	    }
	}

      /*
       * III. Implement block-ending control flow
       */

      switch (bb->cont_type)
	{
	case -1:
	case CNT_ENTRY:
	  P_warn ("PtoL: Unexpected cont type %d, ignoring", bb->cont_type);
	  break;
	case CNT_EXIT:
	  break;
	case CNT_GOTO:
	case CNT_BREAK:
	  PL_terminate_goto (bb);
	  break;
	case CNT_IF:
	  PL_terminate_if (bb);
	  break;
	case CNT_RETURN:
	  PL_terminate_return (bb, epilogue_cb);
	  break;
	case CNT_SWITCH:
	  PL_terminate_switch (bb);
	  break;
	default:
	  P_punt ("PtoL: Unhandled bb control type %d", bb->cont_type);
	}
    }

  if (M_arch == M_TAHOE)
    PL_update_local_size_oper (prologue_cb, stk_size_lv);

  /* L_print_func(stdout, L_fn); */
  /* KVM : At this point, the operands have correct ctypes. If
   * the operands were of long long type, do translations to 
   * emulate the behaviour using native int type.
   */
  PL_translate_longlong(L_fn);


  /* KVM : Change the int ctypes back to PL_native_int_reg_ctype.
   */

  {
    L_Cb *cb;
    L_Oper *op;
    L_Attr *ptr;

    for (cb = L_fn->first_cb; cb; cb = cb->next_cb) {
      for (op = cb->first_op; op; op = op->next_op) {
        int i;

        for(i=0; i<L_max_dest_operand; i++) {
          if(op->dest[i] == NULL) break;
          if(L_is_ctype_void(op->dest[i]) || L_is_ctype_integer(op->dest[i])) {
            op->dest[i]->ctype = PL_native_int_reg_ctype;
          }
        }
        for(i=0; i<L_max_src_operand; i++) {
          if(op->src[i] == NULL) break;
          if(L_is_ctype_void(op->src[i]) || L_is_ctype_integer(op->src[i])) {
            op->src[i]->ctype = PL_native_int_reg_ctype;
          }
        }
        ptr = op->attr;
        while(ptr) {
          for(i=0; i<ptr->max_field; i++) {
            L_Operand *operand = ptr->field[i];
            if(L_is_ctype_void(operand) || L_is_ctype_integer(operand))
              operand->ctype = PL_native_int_reg_ctype;
          }
          ptr = ptr->next_attr;
        }
      }
    }
  }

  if (M_arch == M_TAHOE)
    PL_update_local_size_oper (prologue_cb, stk_size_lv);

  // mchu - bitwidth scan
  if (PL_annotate_bitwidths) {
    L_Cb *cb;
    L_Oper *op;
    int operand;
    int bitwidth;
    int src_bitwidth;
    int dest_bitwidth;

    L_Attr *attr = NULL;
    int field = 0;

    for (cb = L_fn->first_cb; cb; cb = cb->next_cb) {
      for (op = cb->first_op; op; op = op->next_op) {
        if(!L_sign_or_zero_extend_opcode(op))
          continue;

        if(!L_is_reg(op->src[0]) || !L_is_reg(op->dest[0]) )
          continue;

        src_bitwidth = (int) HashTable_find_or_null (operandHash, (int) op->src[0]->value.r);
        dest_bitwidth = (int) HashTable_find_or_null (operandHash, (int) op->dest[0]->value.r);

        if(src_bitwidth < dest_bitwidth) {
          HashTable_remove(operandHash, (int) op->dest[0]->value.r);
          HashTable_insert(operandHash, (int) op->dest[0]->value.r, (int*) src_bitwidth);
        }
        else if(dest_bitwidth < src_bitwidth) {
          HashTable_remove(operandHash, (int) op->src[0]->value.r);
          HashTable_insert(operandHash, (int) op->src[0]->value.r, (int*) dest_bitwidth);
        }

      }      
    }

    // mchu - insert L_Oper attribute for bitwidths
  
    if (!(attr = L_find_attr (L_fn->attr, "bitwidth"))) {
      attr = L_new_attr ("bitwidth", 0);
      L_fn->attr = L_concat_attr (L_fn->attr, attr);
      field = 0;
    }

    HashTable_start (operandHash);

    while ((bitwidth = (int) HashTable_next (operandHash))) {
      operand = (int) HashTable_key(operandHash);

      L_set_register_attr_field (attr, field++, operand, L_CTYPE_INT, 0);  
      L_set_int_attr_field (attr, field++, bitwidth);
    }
    HashTable_reset (operandHash);

    //  L_print_func(stdout, L_fn);
    // mchu - reset for next func
    HashTable_reset(operandHash);
  }
  return;
}

HashTable llong_operand_hash;
int num_native_ints = M_SIZE_LLONG/M_SIZE_INT;

void PL_llong_codegen_load(L_Cb *cb, L_Oper *op)
{
  int i;
  L_Operand *old_dest;
  L_Operand **new_dests;

  L_Operand *base;
  L_Operand **offsets;

  old_dest = op->dest[0];
  new_dests = (L_Operand **)malloc(sizeof(L_Operand *) * num_native_ints);

  /* KVM : TBD : Assuming a little endian machine. */
  for(i=0; i<num_native_ints; i++) {
    new_dests[i] = L_new_register_operand(PL_next_reg_id(), PL_native_int_reg_ctype, 1);
  }

  if(HashTable_find_or_null(llong_operand_hash, old_dest->value.r)) {
    P_punt("old_dest already in hash table\n");
  }
  HashTable_insert(llong_operand_hash, old_dest->value.r, (void *)new_dests);

  base = op->src[0];
  offsets = (L_Operand **)malloc(sizeof(L_Operand *) * num_native_ints);

  if(L_is_ctype_llong(op->src[1])) {
    L_Operand **temp_offsets;
    L_warn("op %d : offset is long long!\n", op->id);
    temp_offsets = (L_Operand **)HashTable_find(llong_operand_hash, op->src[1]->value.r);
    offsets[0] = L_copy_operand(temp_offsets[0]);
  }
  else {
    offsets[0] = L_copy_operand(op->src[1]);
  }

  for(i=1; i<num_native_ints; i++) {
    int next_offset = i*(PL_MType_Size(M_TYPE_INT)/PL_MType_Size(M_TYPE_CHAR));
    L_Oper *new_add = L_create_new_op(Lop_ADD);
    L_Operand *temp_int = L_new_int_operand(next_offset, PL_native_int_reg_ctype);
    offsets[i] = L_new_register_operand(PL_next_reg_id(), PL_native_int_reg_ctype, 1);
    new_add->dest[0] = offsets[i];
    new_add->src[0] = L_copy_operand(offsets[0]);
    new_add->src[1] = temp_int;
    L_insert_oper_before(cb, op, new_add);
    offsets[i] = L_copy_operand(offsets[i]);
  }

  for(i=0; i<num_native_ints; i++) {
    L_Oper *new_load = L_create_new_op(Lop_LD_I);
    L_Attr *stack_attr;

    new_load->dest[0] = new_dests[i];
    new_load->src[0] = L_copy_operand(base);
    new_load->src[1] = offsets[i];

    new_load->flags = op->flags;
    new_load->attr = L_copy_attr(op->attr);

    if(i > 0) {
      stack_attr = L_find_attr(new_load->attr, STACK_ATTR_NAME);
      if(stack_attr) {
        stack_attr->field[1]->value.i += i*PL_MType_Size(M_TYPE_INT)/PL_MType_Size(M_TYPE_CHAR);
      }
    }

    L_insert_oper_before(cb, op, new_load);
  }

  L_delete_oper(cb, op);
}

void PL_llong_codegen_store(L_Cb *cb, L_Oper *op)
{
  L_Operand *val = op->src[2];
  int i;

  L_Attr *stack_attr;

  L_Operand *base;
  L_Operand *old_offset;
  L_Operand **new_offsets;

  base = op->src[0];
  old_offset = op->src[1];

  if(L_is_ctype_llong(op->src[1])) {
    L_punt("op %d offset is longlong!\n", op->id);
  }

  new_offsets = (L_Operand **)malloc(sizeof(L_Operand *) * num_native_ints);

  new_offsets[0] = L_copy_operand(old_offset);

  //if(!L_is_numeric_constant(new_offsets[0])) {
  //  P_punt("PL_llong_codegen_store : Only supporting constant offsets.");
  //}

  for(i=1; i<num_native_ints; i++) {
    int next_offset = i*(PL_MType_Size(M_TYPE_INT)/PL_MType_Size(M_TYPE_CHAR));
    L_Oper *new_add = L_create_new_op(Lop_ADD);
    L_Operand *temp_int = L_new_int_operand(next_offset, PL_native_int_reg_ctype);
    new_offsets[i] = L_new_register_operand(PL_next_reg_id(), PL_native_int_reg_ctype, 1);
    new_add->dest[0] = new_offsets[i];
    new_add->src[0] = L_copy_operand(new_offsets[0]);
    new_add->src[1] = temp_int;
    L_insert_oper_before(cb, op, new_add);
    new_offsets[i] = L_copy_operand(new_offsets[i]);
  }

  if(L_is_register(val)) {
    L_Operand **vals;
    /* KVM : TBD A llong register is stored to memory before it is being
             defined. Defining a new register..
    */
    if(HashTable_find_or_null(llong_operand_hash, op->src[2]->value.r)) {
      vals = (L_Operand **)HashTable_find(llong_operand_hash, op->src[2]->value.r);
    }
    else {
      vals = (L_Operand **)malloc(sizeof(L_Operand *) * num_native_ints);
      for(i=0; i<num_native_ints; i++) {
        vals[i] = L_new_register_operand(PL_next_reg_id(), PL_native_int_reg_ctype, 1);
      }
      HashTable_insert(llong_operand_hash, op->src[2]->value.r, (void *)vals);
    }
    for(i=0; i<num_native_ints; i++) {
      L_Oper *new_op = L_create_new_op(Lop_ST_I);
      new_op->src[0] = L_copy_operand(base);
      new_op->src[1] = new_offsets[i];
      new_op->src[2] = L_copy_operand(vals[i]);
      new_op->flags = op->flags;
      new_op->attr = L_copy_attr(op->attr);

      if(i > 0) {
        stack_attr = L_find_attr(new_op->attr, STACK_ATTR_NAME);
        if(stack_attr) {
          stack_attr->field[1]->value.i += i*PL_MType_Size(M_TYPE_INT)/PL_MType_Size(M_TYPE_CHAR);
        }
      }

      L_insert_oper_before(cb, op, new_op);
    }
    L_delete_oper(cb, op);
  }
  else if(L_is_constant(val)) {
    ITintmax value = op->src[2]->value.i;
    ITintmax temp = value;
    ITintmax mask = 0xFFFFFFFF;
    for(i=0; i<num_native_ints; i++) {
      L_Oper *new_op = L_create_new_op(Lop_ST_I);
      new_op->src[0] = L_copy_operand(base);
      new_op->src[1] = new_offsets[i];
      new_op->src[2] = L_new_int_operand(temp & mask, PL_native_int_reg_ctype);
      temp = temp >> PL_MType_Size(M_TYPE_INT);
      new_op->flags = op->flags;
      new_op->attr = L_copy_attr(op->attr);

      if(i > 0) {
        stack_attr = L_find_attr(new_op->attr, STACK_ATTR_NAME);
        if(stack_attr) {
          stack_attr->field[1]->value.i += i*PL_MType_Size(M_TYPE_INT)/PL_MType_Size(M_TYPE_CHAR);
        }
      }

      L_insert_oper_before(cb, op, new_op);
    }
    L_delete_oper(cb, op);
  }
}

void PL_llong_codegen_add_sub(L_Cb *cb, L_Oper *op)
{
  L_Operand **src1, **src2;
  L_Oper *temp_op = NULL;
  int is_unsigned;

  L_Operand **new_dest;
  L_Operand *temp_carry = 0;

  int i;

  if(L_is_constant(op->src[0])) {
    union {long long x; struct {int lo, hi;} parts;} buf;
    buf.x = op->src[0]->value.i;
    src1 = (L_Operand **)malloc(sizeof(L_Operand *) * num_native_ints);
    for(i=0; i<num_native_ints; i++) {
      L_Oper *new_op = L_create_new_op(Lop_MOV);
      src1[i] = L_new_register_operand(PL_next_reg_id(), PL_native_int_reg_ctype, 1);
      new_op->dest[0] = src1[i];
      /* KVM : TBD : handling only 32-bit machine here. */
      if(i == 0)
        new_op->src[0] = L_new_gen_int_operand(buf.parts.lo);
      else
        new_op->src[0] = L_new_gen_int_operand(buf.parts.hi);
      L_insert_oper_before(cb, op, new_op);
    }
  }
  else {
    src1 = (L_Operand **)HashTable_find(llong_operand_hash, op->src[0]->value.r);
  }
  if(L_is_constant(op->src[1])) {
    union {long long x; struct {int lo, hi;} parts;} buf;
    buf.x = op->src[1]->value.i;
    src2 = (L_Operand **)malloc(sizeof(L_Operand *) * num_native_ints);
    for(i=0; i<num_native_ints; i++) {
      L_Oper *new_op = L_create_new_op(Lop_MOV);
      src2[i] = L_new_register_operand(PL_next_reg_id(), PL_native_int_reg_ctype, 1);
      new_op->dest[0] = src2[i];
      /* KVM : TBD : handling only 32-bit machine here. */
      if(i == 0)
        new_op->src[0] = L_new_gen_int_operand(buf.parts.lo);
      else
        new_op->src[0] = L_new_gen_int_operand(buf.parts.hi);
      L_insert_oper_before(cb, op, new_op);
    }
  }
  else {
    src2 = (L_Operand **)HashTable_find(llong_operand_hash, op->src[1]->value.r);
  }

  new_dest = (L_Operand **)malloc(sizeof(L_Operand *) * num_native_ints);

  if(L_is_ctype_ullong(op->src[0]) && L_is_ctype_ullong(op->src[1]))
    is_unsigned = 1;
  else if(L_is_ctype_llong(op->src[0]) && L_is_ctype_llong(op->src[1]))
    is_unsigned = 0;
  else if(L_is_constant(op->src[0])) {
    if(L_is_ctype_llong(op->src[1])) is_unsigned = 0;
    else is_unsigned = 1;
  }
  else P_punt("PL_llong_codegen_add_sub : Cannot add two incompatible types.");

  for(i=0; i<num_native_ints; i++) {
    if(L_add_opcode(op)) {
      if(is_unsigned)
        temp_op = L_create_new_op(Lop_ADD_CARRY_U);
      else
        temp_op = L_create_new_op(Lop_ADD_CARRY);
    }
    else if(L_sub_opcode(op)) {
      if(is_unsigned)
        temp_op = L_create_new_op(Lop_SUB_CARRY_U);
      else
        temp_op = L_create_new_op(Lop_SUB_CARRY);
    }
    new_dest[i] = L_new_register_operand(PL_next_reg_id(), PL_native_int_reg_ctype, 1);

    temp_op->src[0] = L_copy_operand(src1[i]);
    temp_op->src[1] = L_copy_operand(src2[i]);
    if(i == 0)
      temp_carry = L_new_int_operand(0, PL_native_int_reg_ctype);
    else 
      temp_carry = L_copy_operand(temp_carry);
    temp_op->src[2] = temp_carry;

    temp_op->dest[0] = new_dest[i];
    temp_carry = L_new_register_operand(PL_next_reg_id(), PL_native_int_reg_ctype, 1);
    temp_op->dest[1] = temp_carry;

    temp_op->flags = op->flags;
    temp_op->attr = L_copy_attr(op->attr);
    if(i == 0) {
      temp_op->attr = L_concat_attr(temp_op->attr, L_new_attr("do_not_constant_fold", 0));
    }
    L_insert_oper_before(cb, op, temp_op);
  }

  if(HashTable_find_or_null(llong_operand_hash, op->dest[0]->value.r)) {
    P_punt("old_dest already in hash table\n");
  }

  HashTable_insert(llong_operand_hash, op->dest[0]->value.r, (void *)new_dest);
  L_delete_oper(cb, op);
}

void PL_llong_codegen_logic(L_Cb *cb, L_Oper *op)
{
  L_Operand **src1, **src2;
  L_Oper *temp_op;

  L_Operand **new_dest;

  int i;

  if(L_is_constant(op->src[0])) {
    union {long long x; struct {int lo, hi;} parts;} buf;
    buf.x = op->src[0]->value.i;
    src1 = (L_Operand **)malloc(sizeof(L_Operand *) * num_native_ints);
    for(i=0; i<num_native_ints; i++) {
      L_Oper *new_op = L_create_new_op(Lop_MOV);
      src1[i] = L_new_register_operand(PL_next_reg_id(), PL_native_int_reg_ctype, 1);
      new_op->dest[0] = src1[i];
      /* KVM : TBD : handling only 32-bit machine here. */
      if(i == 0)
        new_op->src[0] = L_new_gen_int_operand(buf.parts.lo);
      else
        new_op->src[0] = L_new_gen_int_operand(buf.parts.hi);
      L_insert_oper_before(cb, op, new_op);
    }
  }
  else {
    if(HashTable_find_or_null(llong_operand_hash, op->src[0]->value.r)) {
      src1 = (L_Operand **)HashTable_find(llong_operand_hash, op->src[0]->value.r);
    }
    else {
      src1 = (L_Operand **)malloc(sizeof(L_Operand *) * num_native_ints);
      for(i=0; i<num_native_ints; i++) {
        src1[i] = L_new_register_operand(PL_next_reg_id(), PL_native_int_reg_ctype, 1);
      }
      HashTable_insert(llong_operand_hash, op->src[0]->value.r, (void *)src1);
    }
  }

  if(L_is_register(op->src[1]))
     src2 = (L_Operand **)HashTable_find(llong_operand_hash, op->src[1]->value.r);
  else {
    ITintmax value = op->src[1]->value.i;
    ITintmax temp = value;
    ITintmax mask = 0xFFFFFFFF;

    src2 = (L_Operand **)malloc(sizeof(L_Operand *) * num_native_ints);
    for(i=0; i<num_native_ints; i++) {
      temp_op = L_create_new_op(Lop_MOV);
      src2[i] = L_new_register_operand(PL_next_reg_id(), PL_native_int_reg_ctype, 1);
      temp_op->dest[0] = src2[i];
      temp_op->src[0] = L_new_gen_int_operand(temp & mask);
      temp = temp >> PL_MType_Size(M_TYPE_INT);
      L_insert_oper_before(cb, op, temp_op);
    }
  }

  new_dest = (L_Operand **)malloc(sizeof(L_Operand *) * num_native_ints);

  for(i=0; i<num_native_ints; i++) {
    temp_op = L_create_new_op(op->opc);

    new_dest[i] = L_new_register_operand(PL_next_reg_id(), PL_native_int_reg_ctype, 1);

    temp_op->src[0] = L_copy_operand(src1[i]);
    temp_op->src[1] = L_copy_operand(src2[i]);
    temp_op->dest[0] = new_dest[i];

    temp_op->flags = op->flags;
    temp_op->attr = L_copy_attr(op->attr);

    L_insert_oper_before(cb, op, temp_op);
  }
 
 if(HashTable_find_or_null(llong_operand_hash, op->dest[0]->value.r)) {
    P_punt("old_dest already in hash table\n");
  }

  HashTable_insert(llong_operand_hash, op->dest[0]->value.r, (void *)new_dest);
  L_delete_oper(cb, op);
}

void PL_llong_codegen_sxt(L_Cb *cb, L_Oper *op)
{
  L_Operand *old_dest;
  L_Operand **new_dests;
  L_Operand *temp_dest;
  L_Oper *new_op;
  int i;

  old_dest = op->dest[0];
  new_dests = (L_Operand **)malloc(sizeof(L_Operand *) * num_native_ints);

  for(i=0; i<num_native_ints; i++) {
    new_dests[i] = L_new_register_operand(PL_next_reg_id(), PL_native_int_reg_ctype, 1);
  }

  if(HashTable_find_or_null(llong_operand_hash, old_dest->value.r)) {
    P_punt("old_dest already in hash table\n");
  }

  HashTable_insert(llong_operand_hash, old_dest->value.r, (void *)new_dests);

  if(L_is_ctype_llong(op->src[0])) {
    L_Operand **old_src = (L_Operand **)HashTable_find(llong_operand_hash, op->src[0]->value.r);
    if(!L_is_ctype_llong(op->dest[0])) {
      op->src[0] = L_copy_operand(old_src[0]);
      return;
    }
    for(i=0; i<num_native_ints; i++) {
      new_op = L_create_new_op(Lop_MOV);
      new_op->dest[0] = new_dests[i];
      new_op->src[0] = L_copy_operand(old_src[i]);
      new_op->flags = op->flags;
      new_op->attr = L_copy_attr(op->attr);
      L_insert_oper_before(cb, op, new_op);
    }
    L_delete_oper(cb, op);
    return;
  }

  if(op->opc == Lop_SXT_C)
    new_op = L_create_new_op(Lop_SXT_C);
  else if(op->opc == Lop_SXT_C2)
    new_op = L_create_new_op(Lop_SXT_C2);
  else if(op->opc == Lop_SXT_I)
    new_op = L_create_new_op(Lop_MOV);
  else {
    P_punt("PL_llong_codegen_sxt : Unknown sxt opcode");
    new_op = 0;
  }

  new_op->dest[0] = new_dests[0];
  new_op->src[0] = L_copy_operand(op->src[0]);
  new_op->flags = op->flags;
  new_op->attr = L_copy_attr(op->attr);
  L_insert_oper_before(cb, op, new_op);

  temp_dest = L_new_register_operand(PL_next_reg_id(), PL_native_int_reg_ctype, 1);
  new_op = L_create_new_op(Lop_ASR);
  new_op->dest[0] = temp_dest;
  new_op->src[0] = L_copy_operand(new_dests[0]);
  new_op->src[1] = L_new_int_operand(31, PL_native_int_reg_ctype);
  L_insert_oper_before(cb, op, new_op);

  for(i=1; i<num_native_ints; i++) {
    new_op = L_create_new_op(Lop_MOV);
    new_op->dest[0] = new_dests[i];
    new_op->src[0] = L_copy_operand(temp_dest);
    new_op->flags = op->flags;
    new_op->attr = L_copy_attr(op->attr);
    L_insert_oper_before(cb, op, new_op);
  }

  L_delete_oper(cb, op);
}

void PL_llong_genadd_for_mul(L_Cb *cb, L_Oper *op, L_Operand **dest, L_Operand **src)
{
  L_Oper *temp_op;
  int i;
  int is_unsigned;

  L_Operand *temp_carry = 0;

  if(L_is_ctype_ullong(op->src[0]))// && L_is_ctype_ullong(op->src[1]))
    is_unsigned = 1;
  else if(L_is_ctype_llong(op->src[0]))// && L_is_ctype_llong(op->src[1]))
    is_unsigned = 0;
  else P_punt("PL_llong_genadd_for_mul : Cannot handle type.");

  for(i=0; i<num_native_ints; i++) {
    if(is_unsigned)
      temp_op = L_create_new_op(Lop_ADD_CARRY_U);
    else
      temp_op = L_create_new_op(Lop_ADD_CARRY);

    temp_op->src[0] = L_copy_operand(dest[i]);
    temp_op->src[1] = L_copy_operand(src[i]);
    if(i == 0)
      temp_carry = L_new_int_operand(0, PL_native_int_reg_ctype);
    else 
      temp_carry = L_copy_operand(temp_carry);
    temp_op->src[2] = temp_carry;

    temp_op->dest[0] = L_copy_operand(dest[i]);
    temp_carry = L_new_register_operand(PL_next_reg_id(), PL_native_int_reg_ctype, 1);
    temp_op->dest[1] = temp_carry;

    temp_op->flags = op->flags;
    temp_op->attr = L_copy_attr(op->attr);
    if(i == 0) {
      temp_op->attr = L_concat_attr(temp_op->attr, L_new_attr("do_not_constant_fold", 0));
    }
    L_insert_oper_before(cb, op, temp_op);
  }
}

void PL_llong_codegen_mul(L_Cb *cb, L_Oper *op)
{
  L_Operand *old_dest;
  L_Operand **a;
  L_Operand **b = NULL;
  L_Operand ***c;
  L_Operand ***d;
  L_Operand **new_dest;
  L_Operand **temp_src;
  L_Operand *zero_operand;

  int i, j, k;

  a = (L_Operand **)HashTable_find(llong_operand_hash, op->src[0]->value.r);
  if(L_is_register(op->src[1]))
    b = (L_Operand **)HashTable_find(llong_operand_hash, op->src[1]->value.r);
  else if(L_is_constant(op->src[1])) {
    union {long long x; struct {int lo, hi;} parts;} buf;
    buf.x = op->src[1]->value.i;
    b = (L_Operand **)malloc(sizeof(L_Operand *) * num_native_ints);
    for(i=0; i<num_native_ints; i++) {
      L_Oper *new_op = L_create_new_op(Lop_MOV);
      b[i] = L_new_register_operand(PL_next_reg_id(), PL_native_int_reg_ctype, 1);
      new_op->dest[0] = b[i];
      /* KVM : TBD : handling only 32-bit machine here. */
      if(i == 0)
        new_op->src[0] = L_new_gen_int_operand(buf.parts.lo);
      else
        new_op->src[0] = L_new_gen_int_operand(buf.parts.hi);
      L_insert_oper_before(cb, op, new_op);

    }
  }

  c = (L_Operand ***)malloc(sizeof(L_Operand **) * num_native_ints * num_native_ints);
  d = (L_Operand ***)malloc(sizeof(L_Operand **) * num_native_ints * num_native_ints);

  for(i=0; i<num_native_ints; i++) {
    c[i] = (L_Operand **)malloc(sizeof(L_Operand *) * num_native_ints);
    d[i] = (L_Operand **)malloc(sizeof(L_Operand *) * num_native_ints);
  }

  old_dest = op->dest[0];
  new_dest = (L_Operand **)malloc(sizeof(L_Operand *) * num_native_ints);

  if(HashTable_find_or_null(llong_operand_hash, old_dest->value.r)) {
    P_punt("old_dest already in hash table\n");
  }

  HashTable_insert(llong_operand_hash, old_dest->value.r, (void *)new_dest);

  for(i=0; i<num_native_ints; i++) {
    L_Oper *init_op;
    init_op = L_create_new_op(Lop_MOV);
    new_dest[i] = L_new_register_operand(PL_next_reg_id(), PL_native_int_reg_ctype, 1);
    init_op->dest[0] = new_dest[i];
    init_op->src[0] = L_new_int_operand(0, PL_native_int_reg_ctype);
    L_insert_oper_before(cb, op, init_op);
  }

  /* KVM :
   *   an a(n-1) a(n-2) .... a0
   *   bn b(n-1) b(n-2) .... b0
   *  --------------------------
   *                      b0an  ....  b0a2  b0a1  b0a0
   *              b1an b1a(n-1) ....  b1a2  b1a0
   *           ......
   *           ......
   * bnan bna(n-1) ..... bna0
   */

  temp_src = (L_Operand **)malloc(sizeof(L_Operand *) * num_native_ints);
  zero_operand = L_new_int_operand(0, PL_native_int_reg_ctype);

  for(i=0; i<num_native_ints; i++) {
    int flag = 0;
    for(j=0; j<num_native_ints; j++) {
      L_Oper *new_op;
      int opc;
      int num_shift;

      num_shift = i+j;
      if(num_shift >= num_native_ints) {
        flag = 1;
        break;
      }

      if(L_is_ctype_ullong(op->src[0])) opc = Lop_MUL_WIDE;
      else opc = Lop_MUL_WIDE_U;

      new_op = L_create_new_op(opc);
      c[i][j] = L_new_register_operand(PL_next_reg_id(), PL_native_int_reg_ctype, 1);
      d[i][j] = L_new_register_operand(PL_next_reg_id(), PL_native_int_reg_ctype, 1);
      new_op->dest[0] = d[i][j];
      new_op->dest[1] = c[i][j];
      new_op->src[0] = L_copy_operand(b[i]);
      new_op->src[1] = L_copy_operand(a[j]);
      new_op->flags = op->flags;
      new_op->attr = L_copy_attr(op->attr);
      L_insert_oper_before(cb, op, new_op);

      for(k=0; k<num_native_ints; k++) {
        if(k == num_shift)
          temp_src[k] = c[i][j];
        else if(k == num_shift + 1)
          temp_src[k] = d[i][j];
        else
          temp_src[k] = zero_operand;
      }
      PL_llong_genadd_for_mul(cb, op, new_dest, temp_src);
    }
    if(flag == 1) break;
  }

  L_delete_oper(cb, op);
}

void PL_llong_codegen_move(L_Cb *cb, L_Oper *op)
{
  L_Operand **a;
  int present = 0;
  int i;

  if(L_is_macro(op->dest[0])) {
    L_Operand **new_dest;

    if(L_is_register(op->src[0])) {
      a = (L_Operand **)HashTable_find(llong_operand_hash, op->src[0]->value.r);

      new_dest = (L_Operand **)malloc(sizeof(L_Operand *) * num_native_ints);

      for(i=0; i<num_native_ints; i++) {
        L_Oper *new_op;
        new_op = L_create_new_op(Lop_MOV);
        if(i == 0)
          new_dest[i] = L_copy_operand(op->dest[0]);
        else
          new_dest[i] = L_new_macro_operand(op->dest[0]->value.mac + i, PL_native_int_reg_ctype, 0);
        new_op->dest[0] = new_dest[i];
        new_op->src[0] = L_copy_operand(a[i]);
        L_insert_oper_before(cb, op, new_op);
      }
      L_delete_oper(cb, op);
    }
    else if(L_is_constant(op->src[0])) {
      union {long long x; struct {int lo, hi;} parts;} buf;
      buf.x = op->src[0]->value.i;
      new_dest = (L_Operand **)malloc(sizeof(L_Operand *) * num_native_ints);
      for(i=0; i<num_native_ints; i++) {
        L_Oper *new_op = L_create_new_op(Lop_MOV);
        if(i == 0)
          new_dest[i] = L_copy_operand(op->dest[0]);
        else
          new_dest[i] = L_new_macro_operand(op->dest[0]->value.mac + i, PL_native_int_reg_ctype, 0);
        new_op->dest[0] = new_dest[i];
        /* KVM : TBD : handling only 32-bit machine here. */
        if(i == 0)
          new_op->src[0] = L_new_gen_int_operand(buf.parts.lo);
        else
          new_op->src[0] = L_new_gen_int_operand(buf.parts.hi);
        L_insert_oper_before(cb, op, new_op);
      }
      L_delete_oper(cb, op);
    }
    else {
      P_punt("PL_llong_codegen_move : Not handling this case.");
    }
  }
  else {
    if(L_is_register(op->src[0]))
      present = HashTable_member(llong_operand_hash, op->src[0]->value.r);
    else
      present = 0;

    if(present) {
      L_Operand **new_dest;
      new_dest = (L_Operand **)malloc(sizeof(L_Operand *) * num_native_ints);

      a = (L_Operand **)HashTable_find(llong_operand_hash, op->src[0]->value.r);

      for(i=0; i<num_native_ints; i++) {
        L_Oper *new_op = L_create_new_op(Lop_MOV);
        new_dest[i] = L_new_register_operand(PL_next_reg_id(), PL_native_int_reg_ctype, 1);
        new_op->dest[0] = new_dest[i];
        new_op->src[0] = L_copy_operand(a[i]);
        L_insert_oper_before(cb, op, new_op);
      }
      if(HashTable_find_or_null(llong_operand_hash, op->dest[0]->value.r)) {
        L_Operand **old_dest = (L_Operand **)HashTable_find(llong_operand_hash, op->dest[0]->value.r);
        for(i=0; i<num_native_ints; i++) {
          L_Oper *new_op = L_create_new_op(Lop_MOV);
          new_op->dest[0] = L_copy_operand(old_dest[i]);
          new_op->src[0] = L_copy_operand(new_dest[i]);
          L_insert_oper_before(cb, op, new_op);
        }
      }
      else 
        HashTable_insert(llong_operand_hash, op->dest[0]->value.r, (void *)new_dest);
      L_delete_oper(cb, op);

    }
    else {
      /* KVM : Handle assignments of the form r1 <- mac $Pi */
      L_Operand **new_dest;

      if(L_is_macro(op->src[0])) {
        new_dest = (L_Operand **)malloc(sizeof(L_Operand *) * num_native_ints);
        for(i=0; i<num_native_ints; i++) {
          L_Oper *new_op = L_create_new_op(Lop_MOV);
          new_dest[i] = L_new_register_operand(PL_next_reg_id(), PL_native_int_reg_ctype, 1);
          new_op->dest[0] = new_dest[i];
          if(i == 0)
            new_op->src[0] = L_copy_operand(op->src[0]);
          else
            new_op->src[0] = L_new_macro_operand(op->src[0]->value.mac + i, PL_native_int_reg_ctype, 0);
          L_insert_oper_before(cb, op, new_op);
        }
        if(HashTable_find_or_null(llong_operand_hash, op->dest[0]->value.r)) {
          L_Operand **old_dest = (L_Operand **)HashTable_find(llong_operand_hash, op->dest[0]->value.r);
          for(i=0; i<num_native_ints; i++) {
            L_Oper *new_op = L_create_new_op(Lop_MOV);
            new_op->dest[0] = L_copy_operand(old_dest[i]);
            new_op->src[0] = L_copy_operand(new_dest[i]);
            L_insert_oper_before(cb, op, new_op);
          }
        }
        else
          HashTable_insert(llong_operand_hash, op->dest[0]->value.r, (void *)new_dest);
        L_delete_oper(cb, op);
      }
      else if(L_is_constant(op->src[0])) {
        union {long long x; struct {int lo, hi;} parts;} buf;
        buf.x = op->src[0]->value.i;
        new_dest = (L_Operand **)malloc(sizeof(L_Operand *) * num_native_ints);
        for(i=0; i<num_native_ints; i++) {
          L_Oper *new_op = L_create_new_op(Lop_MOV);
          new_dest[i] = L_new_register_operand(PL_next_reg_id(), PL_native_int_reg_ctype, 1);
          new_op->dest[0] = new_dest[i];
          /* KVM : TBD : handling only 32-bit machine here. */
          if(i == 0)
            new_op->src[0] = L_new_gen_int_operand(buf.parts.lo);
          else
            new_op->src[0] = L_new_gen_int_operand(buf.parts.hi);
          L_insert_oper_before(cb, op, new_op);
        }
        if(HashTable_find_or_null(llong_operand_hash, op->dest[0]->value.r)) {
          L_Operand **old_dest = (L_Operand **)HashTable_find(llong_operand_hash, op->dest[0]->value.r);
          for(i=0; i<num_native_ints; i++) {
            L_Oper *new_op = L_create_new_op(Lop_MOV);
            new_op->dest[0] = L_copy_operand(old_dest[i]);
            new_op->src[0] = L_copy_operand(new_dest[i]);
            L_insert_oper_before(cb, op, new_op);
          }
        }
        else 
          HashTable_insert(llong_operand_hash, op->dest[0]->value.r, (void *)new_dest);
        L_delete_oper(cb, op);

      }
      else if(L_is_register(op->dest[0]) && L_is_register(op->src[0])) {
        L_Operand **src1 = (L_Operand **)HashTable_find(llong_operand_hash, op->src[0]->value.r);
        new_dest = (L_Operand **)malloc(sizeof(L_Operand *) * num_native_ints);

        for(i=0; i<num_native_ints; i++) {
          L_Oper *new_op = L_create_new_op(Lop_MOV);
          new_dest[i] = L_new_register_operand(PL_next_reg_id(), PL_native_int_reg_ctype, 1);
          new_op->dest[0] = new_dest[i];
          new_op->src[0] = L_copy_operand(src1[i]);
          L_insert_oper_before(cb, op, new_op);
        }
        if(HashTable_find_or_null(llong_operand_hash, op->dest[0]->value.r)) {
          L_Operand **old_dest = (L_Operand **)HashTable_find(llong_operand_hash, op->dest[0]->value.r);
          for(i=0; i<num_native_ints; i++) {
            L_Oper *new_op = L_create_new_op(Lop_MOV);
            new_op->dest[0] = L_copy_operand(old_dest[i]);
            new_op->src[0] = L_copy_operand(new_dest[i]);
            L_insert_oper_before(cb, op, new_op);
          }
        }
        else
          HashTable_insert(llong_operand_hash, op->dest[0]->value.r, (void *)new_dest);
        L_delete_oper(cb, op);

      }
      else {
        P_punt("PL_llong_codegen_move : Cannot handle this case.");
      }
    }
  }
}

void PL_llong_codegen_define(L_Cb *cb, L_Oper *op)
{
  int i;
  L_Oper *new_op;

  if(L_is_macro(op->dest[0])) {
    if(op->dest[0]->value.mac != L_MAC_RET_TYPE && op->dest[0]->value.mac != L_MAC_TM_TYPE) {
      if(!L_is_ctype_llong(op->dest[0]))
        P_punt("PL_llong_codegen_define : Unexpected type.");
      for(i=1; i<num_native_ints; i++) {
        new_op = L_create_new_op(Lop_DEFINE);
        new_op->dest[0] = L_new_macro_operand(op->dest[0]->value.mac + i, PL_native_int_reg_ctype, 0);
        new_op->attr = L_copy_attr(op->attr);
        L_insert_oper_after(cb, op, new_op);
      }
    }
  }
}

void DEBUGLL(char *OPCODE)
{
  //printf("LLONG PL_llong_codegen %s\n", OPCODE);
}

void PL_llong_codegen_jsr(L_Cb *cb, L_Oper *op)
{
  L_Attr *attr;
  L_Operand *new_op;

  int i, j;
  int num_llong_params = 0;
  int orig_max;

  attr = L_find_attr(op->attr, "tr");
  if(attr) {
    for(i=0; i<attr->max_field; i++) {
      if(L_is_ctype_llong(attr->field[i])) num_llong_params++;
    }
    if(num_llong_params == 0) {
      return;
    }

    DEBUGLL("jsr");

    j = 0;

    orig_max = attr->max_field;
    for(i=0; i<orig_max; i++) {
      if(L_is_ctype_llong(attr->field[j])) {
        new_op = L_copy_operand(attr->field[j]);
        new_op->value.mac = attr->field[j]->value.mac + 1;
        L_insert_attr_field(attr, new_op, j);
        j++;
      }
      j++;
    }
  }
}

void PL_llong_codegen_lsr(L_Func *l_fn, L_Cb *cb, L_Oper *op)
{
  L_Operand **new_dest;

  L_Operand **src1;
  int i;

  L_Operand *shift_amount = NULL;
  L_Oper *temp_op;
  L_Operand *val, *minuslo, *minushi, *temp1, *temp2;
  L_Operand *lo, *hi, *sa;
  L_Operand *yhl, *ylh;

  src1 = (L_Operand **)HashTable_find(llong_operand_hash, op->src[0]->value.r);

  if(L_is_register(op->src[1])) {
    shift_amount = op->src[1];
  }
  else if(L_is_constant(op->src[1])) {
    temp_op = L_create_new_op(Lop_MOV);
    shift_amount = L_new_register_operand(PL_next_reg_id(), PL_native_int_reg_ctype, 1);
    temp_op->dest[0] = shift_amount;
    temp_op->src[0] = L_new_gen_int_operand(op->src[1]->value.i);
    L_insert_oper_before(cb, op, temp_op);
  }
  new_dest = (L_Operand **)malloc(sizeof(L_Operand *) * num_native_ints);
  for(i=0; i<num_native_ints; i++) {
    new_dest[i] = L_new_register_operand(PL_next_reg_id(), PL_native_int_reg_ctype, 1);
  }

  /* val <- shift_amount >> 5 */
  temp_op = L_create_new_op(Lop_LSR);
  val = L_new_register_operand(PL_next_reg_id(), PL_native_int_reg_ctype, 1);
  temp_op->dest[0] = val;
  temp_op->src[0] = L_copy_operand(shift_amount);
  temp_op->src[1] = L_new_gen_int_operand(5);
  L_insert_oper_before(cb, op, temp_op);

  /* minuslo <- -src1[0] */
  temp_op = L_create_new_op(Lop_SUB);
  minuslo = L_new_register_operand(PL_next_reg_id(), PL_native_int_reg_ctype, 1);
  temp_op->dest[0] = minuslo;
  temp_op->src[0] = L_new_gen_int_operand(0);
  temp_op->src[1] = L_copy_operand(src1[0]);
  L_insert_oper_before(cb, op, temp_op);

  /* minushi <- -src1[1] */
  temp_op = L_create_new_op(Lop_SUB);
  minushi = L_new_register_operand(PL_next_reg_id(), PL_native_int_reg_ctype, 1);
  temp_op->dest[0] = minushi;
  temp_op->src[0] = L_new_gen_int_operand(0);
  temp_op->src[1] = L_copy_operand(src1[1]);
  L_insert_oper_before(cb, op, temp_op);

  /* temp1 <- minuslo + src1[1] */
  temp_op = L_create_new_op(Lop_ADD);
  temp1 = L_new_register_operand(PL_next_reg_id(), PL_native_int_reg_ctype, 1);
  temp_op->dest[0] = temp1;
  temp_op->src[0] = L_copy_operand(minuslo);
  temp_op->src[1] = L_copy_operand(src1[1]);
  L_insert_oper_before(cb, op, temp_op);

  /* temp2 <- val * temp1 */
  temp_op = L_create_new_op(Lop_MUL);
  temp2 = L_new_register_operand(PL_next_reg_id(), PL_native_int_reg_ctype, 1);
  temp_op->dest[0] = temp2;
  temp_op->src[0] = L_copy_operand(val);
  temp_op->src[1] = L_copy_operand(temp1);
  L_insert_oper_before(cb, op, temp_op);

  /* lo <- src1[0] + temp2 */
  temp_op = L_create_new_op(Lop_ADD);
  lo = L_new_register_operand(PL_next_reg_id(), PL_native_int_reg_ctype, 1);
  temp_op->dest[0] = lo;
  temp_op->src[0] = L_copy_operand(src1[0]);
  temp_op->src[1] = L_copy_operand(temp2);
  L_insert_oper_before(cb, op, temp_op);

  /* temp1 <- val * minushi */
  temp_op = L_create_new_op(Lop_MUL);
  temp1 = L_new_register_operand(PL_next_reg_id(), PL_native_int_reg_ctype, 1);
  temp_op->dest[0] = temp1;
  temp_op->src[0] = L_copy_operand(val);
  temp_op->src[1] = L_copy_operand(minushi);
  L_insert_oper_before(cb, op, temp_op);

  /* hi <- src1[1] + temp1 */
  temp_op = L_create_new_op(Lop_ADD);
  hi = L_new_register_operand(PL_next_reg_id(), PL_native_int_reg_ctype, 1);
  temp_op->dest[0] = hi;
  temp_op->src[0] = L_copy_operand(src1[1]);
  temp_op->src[1] = L_copy_operand(temp1);
  L_insert_oper_before(cb, op, temp_op);

  /* temp1 <- val * (-32) */
  temp_op = L_create_new_op(Lop_MUL);
  temp1 = L_new_register_operand(PL_next_reg_id(), PL_native_int_reg_ctype, 1);
  temp_op->dest[0] = temp1;
  temp_op->src[0] = L_copy_operand(val);
  temp_op->src[1] = L_new_gen_int_operand(-32);
  L_insert_oper_before(cb, op, temp_op);

  /* sa <- shift_amount + temp1 */
  temp_op = L_create_new_op(Lop_ADD);
  sa = L_new_register_operand(PL_next_reg_id(), PL_native_int_reg_ctype, 1);
  temp_op->dest[0] = sa;
  temp_op->src[0] = L_copy_operand(shift_amount);
  temp_op->src[1] = L_copy_operand(temp1);
  L_insert_oper_before(cb, op, temp_op);

  /* temp1 <- 32 - sa */
  temp_op = L_create_new_op(Lop_SUB);
  temp1 = L_new_register_operand(PL_next_reg_id(), PL_native_int_reg_ctype, 1);
  temp_op->dest[0] = temp1;
  temp_op->src[0] = L_new_gen_int_operand(32);
  temp_op->src[1] = L_copy_operand(sa);
  L_insert_oper_before(cb, op, temp_op);

  /* yhl <- hi << temp1 */
  temp_op = L_create_new_op(Lop_LSL);
  yhl = L_new_register_operand(PL_next_reg_id(), PL_native_int_reg_ctype, 1);
  temp_op->dest[0] = yhl;
  temp_op->src[0] = L_copy_operand(hi);
  temp_op->src[1] = L_copy_operand(temp1);
  L_insert_oper_before(cb, op, temp_op);

  /* ylh <- lo >> sa */
  temp_op = L_create_new_op(Lop_LSR);
  ylh = L_new_register_operand(PL_next_reg_id(), PL_native_int_reg_ctype, 1);
  temp_op->dest[0] = ylh;
  temp_op->src[0] = L_copy_operand(lo);
  temp_op->src[1] = L_copy_operand(sa);
  L_insert_oper_before(cb, op, temp_op);

  /* new_dest[1] <- hi >> sa */
  temp_op = L_create_new_op(Lop_LSR);
  temp_op->dest[0] = new_dest[1];
  temp_op->src[0] = L_copy_operand(hi);
  temp_op->src[1] = L_copy_operand(sa);
  L_insert_oper_before(cb, op, temp_op);

  /* new_dest[0] <- yhl + ylh */
  temp_op = L_create_new_op(Lop_ADD);
  temp_op->dest[0] = new_dest[0];
  temp_op->src[0] = L_copy_operand(yhl);
  temp_op->src[1] = L_copy_operand(ylh);
  L_insert_oper_before(cb, op, temp_op);

  if(HashTable_find_or_null(llong_operand_hash, op->dest[0]->value.r)) {
    P_punt("old_dest already in hash table\n");
  }

  HashTable_insert(llong_operand_hash, op->dest[0]->value.r, (void *)new_dest);
  L_delete_oper(cb, op);
}

void PL_llong_codegen_asr(L_Func *l_fn, L_Cb *cb, L_Oper *op)
{
  L_Operand **new_dest;

  L_Operand **src1;
  int i;

  L_Operand *shift_amount = NULL;
  L_Oper *temp_op;
  L_Operand *val, *minuslo, *temp1, *temp2, *temp3, *temp4, *temp5, *temp;
  L_Operand *lo, *hi, *sa;
  L_Operand *yhl, *ylh;
  L_Operand *anshi, *anslo;

  src1 = (L_Operand **)HashTable_find(llong_operand_hash, op->src[0]->value.r);

  if(L_is_register(op->src[1])) {
    shift_amount = op->src[1];
  }
  else if(L_is_constant(op->src[1])) {
    temp_op = L_create_new_op(Lop_MOV);
    shift_amount = L_new_register_operand(PL_next_reg_id(), PL_native_int_reg_ctype, 1);
    temp_op->dest[0] = shift_amount;
    temp_op->src[0] = L_new_gen_int_operand(op->src[1]->value.i);
    L_insert_oper_before(cb, op, temp_op);
  }
  new_dest = (L_Operand **)malloc(sizeof(L_Operand *) * num_native_ints);
  for(i=0; i<num_native_ints; i++) {
    new_dest[i] = L_new_register_operand(PL_next_reg_id(), PL_native_int_reg_ctype, 1);
  }

#define INSERTOP(opcode, opdest, src1, src2) \
  temp_op = L_create_new_op(opcode); \
  temp_op->dest[0] = opdest; \
  temp_op->src[0] = src1; \
  temp_op->src[1] = src2; \
  L_insert_oper_before(cb, op, temp_op);

  lo = src1[0];
  hi = src1[1];

  val = L_new_register_operand(PL_next_reg_id(), PL_native_int_reg_ctype, 1);
  INSERTOP(Lop_LSR, val, L_copy_operand(shift_amount), L_new_gen_int_operand(5));

  minuslo = L_new_register_operand(PL_next_reg_id(), PL_native_int_reg_ctype, 1);
  INSERTOP(Lop_SUB, minuslo, L_new_gen_int_operand(0), L_copy_operand(lo));

  temp1 = L_new_register_operand(PL_next_reg_id(), PL_native_int_reg_ctype, 1);
  INSERTOP(Lop_ADD, temp1, L_copy_operand(minuslo), L_copy_operand(hi));

  temp = temp1;
  temp1 = L_new_register_operand(PL_next_reg_id(), PL_native_int_reg_ctype, 1);
  INSERTOP(Lop_MUL, temp1, L_copy_operand(val), L_copy_operand(temp));

  temp = temp1;
  temp1 = L_new_register_operand(PL_next_reg_id(), PL_native_int_reg_ctype, 1);
  INSERTOP(Lop_ADD, temp1, L_copy_operand(temp), L_copy_operand(lo));

  temp2 = L_new_register_operand(PL_next_reg_id(), PL_native_int_reg_ctype, 1);
  INSERTOP(Lop_LSR, temp2, L_copy_operand(hi), L_new_gen_int_operand(31));

  temp5 = L_new_register_operand(PL_next_reg_id(), PL_native_int_reg_ctype, 1);
  INSERTOP(Lop_MUL, temp5, L_copy_operand(temp2), L_new_gen_int_operand(-1));

  temp = temp5;
  temp5 = L_new_register_operand(PL_next_reg_id(), PL_native_int_reg_ctype, 1);
  INSERTOP(Lop_SUB, temp5, L_copy_operand(temp), L_copy_operand(hi));

  temp = temp5;
  temp5 = L_new_register_operand(PL_next_reg_id(), PL_native_int_reg_ctype, 1);
  INSERTOP(Lop_MUL, temp5, L_copy_operand(val), L_copy_operand(temp));

  temp2 = L_new_register_operand(PL_next_reg_id(), PL_native_int_reg_ctype, 1);
  INSERTOP(Lop_ADD, temp2, L_copy_operand(temp5), L_copy_operand(hi));

  sa = L_new_register_operand(PL_next_reg_id(), PL_native_int_reg_ctype, 1);
  INSERTOP(Lop_MUL, sa, L_copy_operand(val), L_new_gen_int_operand(-32));

  temp = sa;
  sa = L_new_register_operand(PL_next_reg_id(), PL_native_int_reg_ctype, 1);
  INSERTOP(Lop_ADD, sa, L_copy_operand(temp), L_copy_operand(shift_amount));

  yhl = L_new_register_operand(PL_next_reg_id(), PL_native_int_reg_ctype, 1);
  INSERTOP(Lop_SUB, yhl, L_new_gen_int_operand(32), L_copy_operand(sa));

  temp = yhl;
  yhl = L_new_register_operand(PL_next_reg_id(), PL_native_int_reg_ctype, 1);
  INSERTOP(Lop_LSL, yhl, L_copy_operand(temp2), L_copy_operand(temp));

  ylh = L_new_register_operand(PL_next_reg_id(), PL_native_int_reg_ctype, 1);
  INSERTOP(Lop_LSR, ylh, L_copy_operand(temp1), L_copy_operand(sa));

  temp3 = L_new_register_operand(PL_next_reg_id(), PL_native_int_reg_ctype, 1);
  INSERTOP(Lop_LSR, temp3, L_copy_operand(temp2), L_new_gen_int_operand(31));

  temp = temp3;
  temp3 = L_new_register_operand(PL_next_reg_id(), PL_native_int_reg_ctype, 1);
  INSERTOP(Lop_SUB, temp3, L_new_gen_int_operand(0), L_copy_operand(temp));

  temp4 = L_new_register_operand(PL_next_reg_id(), PL_native_int_reg_ctype, 1);
  INSERTOP(Lop_SUB, temp4, L_new_gen_int_operand(32), L_copy_operand(sa));

  temp = temp3;
  temp3 = L_new_register_operand(PL_next_reg_id(), PL_native_int_reg_ctype, 1);
  INSERTOP(Lop_LSL, temp3, L_copy_operand(temp), L_copy_operand(temp4));

  anshi = L_new_register_operand(PL_next_reg_id(), PL_native_int_reg_ctype, 1);
  INSERTOP(Lop_LSR, anshi, L_copy_operand(temp2), L_copy_operand(sa));

  temp = anshi;
  anshi = L_new_register_operand(PL_next_reg_id(), PL_native_int_reg_ctype, 1);
  INSERTOP(Lop_ADD, anshi, L_copy_operand(temp), L_copy_operand(temp3));

  anslo = L_new_register_operand(PL_next_reg_id(), PL_native_int_reg_ctype, 1);
  INSERTOP(Lop_ADD, anslo, L_copy_operand(yhl), L_copy_operand(ylh));

  new_dest[0] = anslo;
  new_dest[1] = anshi;

#undef INSERTOP

  if(HashTable_find_or_null(llong_operand_hash, op->dest[0]->value.r)) {
    P_punt("old_dest already in hash table\n");
  }

  HashTable_insert(llong_operand_hash, op->dest[0]->value.r, (void *)new_dest);
  L_delete_oper(cb, op);
}
 

void PL_llong_codegen_lsl(L_Func *l_fn, L_Cb *cb, L_Oper *op)
{
  L_Operand **new_dest;

  L_Operand **src1;
  int i;

  L_Operand *shift_amount = NULL;
  L_Oper *temp_op;
  L_Operand *val, *minuslo, *minushi, *temp1, *temp2, *temp;
  L_Operand *lo, *hi, *sa;
  L_Operand *yhl, *ylh;
  L_Operand *anshi, *anslo;

  if(L_is_constant(op->src[0])) {
    union {long long x; struct {int lo, hi;} parts;} buf;
    buf.x = op->src[0]->value.i;
    src1 = (L_Operand **)malloc(sizeof(L_Operand *) * num_native_ints);
    for(i=0; i<num_native_ints; i++) {
      L_Oper *new_op = L_create_new_op(Lop_MOV);
      src1[i] = L_new_register_operand(PL_next_reg_id(), PL_native_int_reg_ctype, 1);
      new_op->dest[0] = src1[i];
      /* KVM : TBD : handling only 32-bit machine here. */
      if(i == 0)
        new_op->src[0] = L_new_gen_int_operand(buf.parts.lo);
      else
        new_op->src[0] = L_new_gen_int_operand(buf.parts.hi);
      L_insert_oper_before(cb, op, new_op);
    }
  }
  else {
    src1 = (L_Operand **)HashTable_find(llong_operand_hash, op->src[0]->value.r);
  }

  if(L_is_register(op->src[1])) {
    shift_amount = op->src[1];
  }
  else if(L_is_constant(op->src[1])) {
    temp_op = L_create_new_op(Lop_MOV);
    shift_amount = L_new_register_operand(PL_next_reg_id(), PL_native_int_reg_ctype, 1);
    temp_op->dest[0] = shift_amount;
    temp_op->src[0] = L_new_gen_int_operand(op->src[1]->value.i);
    L_insert_oper_before(cb, op, temp_op);
  }
  new_dest = (L_Operand **)malloc(sizeof(L_Operand *) * num_native_ints);
  for(i=0; i<num_native_ints; i++) {
    new_dest[i] = L_new_register_operand(PL_next_reg_id(), PL_native_int_reg_ctype, 1);
  }

#define INSERTOP(opcode, opdest, src1, src2) \
  temp_op = L_create_new_op(opcode); \
  temp_op->dest[0] = opdest; \
  temp_op->src[0] = src1; \
  temp_op->src[1] = src2; \
  L_insert_oper_before(cb, op, temp_op);

  lo = src1[0];
  hi = src1[1];

  val = L_new_register_operand(PL_next_reg_id(), PL_native_int_reg_ctype, 1);
  INSERTOP(Lop_LSR, val, L_copy_operand(shift_amount), L_new_gen_int_operand(5));

  minushi = L_new_register_operand(PL_next_reg_id(), PL_native_int_reg_ctype, 1);
  INSERTOP(Lop_SUB, minushi, L_new_gen_int_operand(0), L_copy_operand(hi));

  temp1 = L_new_register_operand(PL_next_reg_id(), PL_native_int_reg_ctype, 1);
  INSERTOP(Lop_ADD, temp1, L_copy_operand(minushi), L_copy_operand(lo));

  temp = temp1;
  temp1 = L_new_register_operand(PL_next_reg_id(), PL_native_int_reg_ctype, 1);
  INSERTOP(Lop_MUL, temp1, L_copy_operand(val), L_copy_operand(temp));

  temp = temp1;
  temp1 = L_new_register_operand(PL_next_reg_id(), PL_native_int_reg_ctype, 1);
  INSERTOP(Lop_ADD, temp1, L_copy_operand(temp), L_copy_operand(hi));

  minuslo = L_new_register_operand(PL_next_reg_id(), PL_native_int_reg_ctype, 1);
  INSERTOP(Lop_SUB, minuslo, L_new_gen_int_operand(0), L_copy_operand(lo));

  temp2 = L_new_register_operand(PL_next_reg_id(), PL_native_int_reg_ctype, 1);
  INSERTOP(Lop_MUL, temp2, L_copy_operand(val), L_copy_operand(minuslo));

  temp = temp2;
  temp2 = L_new_register_operand(PL_next_reg_id(), PL_native_int_reg_ctype, 1);
  INSERTOP(Lop_ADD, temp2, L_copy_operand(temp), L_copy_operand(lo));

  sa = L_new_register_operand(PL_next_reg_id(), PL_native_int_reg_ctype, 1);
  INSERTOP(Lop_MUL, sa, L_copy_operand(val), L_new_gen_int_operand(-32));

  temp = sa;
  sa = L_new_register_operand(PL_next_reg_id(), PL_native_int_reg_ctype, 1);
  INSERTOP(Lop_ADD, sa, L_copy_operand(temp), L_copy_operand(shift_amount));

  yhl = L_new_register_operand(PL_next_reg_id(), PL_native_int_reg_ctype, 1);
  INSERTOP(Lop_LSL, yhl, L_copy_operand(temp1), L_copy_operand(sa));

  ylh = L_new_register_operand(PL_next_reg_id(), PL_native_int_reg_ctype, 1);
  INSERTOP(Lop_SUB, ylh, L_new_gen_int_operand(32), L_copy_operand(sa));

  temp = ylh;
  ylh = L_new_register_operand(PL_next_reg_id(), PL_native_int_reg_ctype, 1);
  INSERTOP(Lop_LSR, ylh, L_copy_operand(temp2), L_copy_operand(temp));

  anshi = L_new_register_operand(PL_next_reg_id(), PL_native_int_reg_ctype, 1);
  INSERTOP(Lop_ADD, anshi, L_copy_operand(yhl), L_copy_operand(ylh));

  anslo = L_new_register_operand(PL_next_reg_id(), PL_native_int_reg_ctype, 1);
  INSERTOP(Lop_LSL, anslo, L_copy_operand(temp2), L_copy_operand(sa));

  new_dest[0] = anslo;
  new_dest[1] = anshi;

#undef INSERTOP
  if(HashTable_find_or_null(llong_operand_hash, op->dest[0]->value.r)) {
    P_punt("old_dest already in hash table\n");
  }

  HashTable_insert(llong_operand_hash, op->dest[0]->value.r, (void *)new_dest);
  L_delete_oper(cb, op);
}

void PL_llong_codegen_branch(L_Cb *cb, L_Oper *op)
{
  L_Oper *new_rcmp;
  L_Oper *temp_op;

  L_Operand **preds;
  L_Operand **src1, **src2;
  L_Operand *temp_dest = NULL;
  int i;

  src1 = (L_Operand **)HashTable_find(llong_operand_hash, op->src[0]->value.r);
  if(L_is_constant(op->src[1])) {
    union {long long x; struct {int lo, hi;} parts;} buf;
    buf.x = op->src[1]->value.i;
    src2 = (L_Operand **)malloc(sizeof(L_Operand *) * num_native_ints);
    for(i=0; i<num_native_ints; i++) {
      L_Oper *new_op = L_create_new_op(Lop_MOV);
      src2[i] = L_new_register_operand(PL_next_reg_id(), PL_native_int_reg_ctype, 1);
      new_op->dest[0] = src2[i];
      /* KVM : TBD : handling only 32-bit machine here. */
      if(i == 0)
        new_op->src[0] = L_new_gen_int_operand(buf.parts.lo);
      else
        new_op->src[0] = L_new_gen_int_operand(buf.parts.hi);
      L_insert_oper_before(cb, op, new_op);
    }
  }
  else {
    src2 = (L_Operand **)HashTable_find(llong_operand_hash, op->src[1]->value.r);
  }

  preds = (L_Operand **)malloc(sizeof(L_Operand *) * num_native_ints);
  for(i=0; i<num_native_ints; i++) {
    preds[i] = L_new_register_operand(PL_next_reg_id(), PL_native_int_reg_ctype, 1);
  }

  for(i=num_native_ints - 1; i>=0; i--) {
    new_rcmp = L_create_new_op(Lop_RCMP);
    new_rcmp->dest[0] = L_new_register_operand(PL_next_reg_id(), PL_native_int_reg_ctype, 1);
    preds[i] = new_rcmp->dest[0];
    new_rcmp->src[0] = L_copy_operand(src1[i]);
    new_rcmp->src[1] = L_copy_operand(src2[i]);
    L_copy_compare(new_rcmp, op);
    new_rcmp->com[0] = PL_native_int_reg_ctype;
    L_insert_oper_before(cb, op, new_rcmp);
  }

  if(L_int_bne_branch_opcode(op) || L_int_beq_branch_opcode(op)) {
    for(i=0; i<num_native_ints; i++) {
      if(i == 0) {
        temp_op = L_create_new_op(Lop_MOV);
        temp_op->dest[0] = L_new_register_operand(PL_next_reg_id(), PL_native_int_reg_ctype, 1);
        temp_dest = temp_op->dest[0];
        temp_op->src[0] = L_copy_operand(preds[i]);
        L_insert_oper_before(cb, op, temp_op);
      }
      else {
        if(L_int_bne_branch_opcode(op))
          temp_op = L_create_new_op(Lop_OR);
        else 
          temp_op = L_create_new_op(Lop_AND);
        temp_op->dest[0] = L_copy_operand(temp_dest);
        temp_op->src[0] = L_copy_operand(temp_dest);
        temp_op->src[1] = L_copy_operand(preds[i]);
        L_insert_oper_before(cb, op, temp_op);
      }
    }
  }
  else if(L_int_blt_branch_opcode(op)) {
    if(L_ctype_is_signed(op->com[0])) {
      L_Oper *eq_cmp, *lt_cmp;
      L_Oper *and_op, *or_op;
      L_Operand *eq, *lt;
      L_Operand *t1, *t2;

      eq_cmp = L_create_new_op(Lop_RCMP);
      L_copy_compare(eq_cmp, op);
      eq_cmp->com[1] = Lcmp_COM_EQ;
      eq = L_new_register_operand(PL_next_reg_id(), PL_native_int_reg_ctype, 1);
      eq_cmp->dest[0] = eq;
      eq_cmp->src[0] = L_copy_operand(src1[1]);
      eq_cmp->src[1] = L_copy_operand(src2[1]);
      L_insert_oper_before(cb, op, eq_cmp);

      lt_cmp = L_create_new_op(Lop_RCMP);
      L_copy_compare(lt_cmp, op);
      lt_cmp->com[0] = L_ctype_unsigned_version(PL_native_int_reg_ctype);
      lt = L_new_register_operand(PL_next_reg_id(), PL_native_int_reg_ctype, 1);
      lt_cmp->dest[0] = lt;
      lt_cmp->src[0] = L_copy_operand(src1[0]);
      lt_cmp->src[1] = L_copy_operand(src2[0]);
      L_insert_oper_before(cb, op, lt_cmp);

      and_op = L_create_new_op(Lop_AND);
      t1 = L_new_register_operand(PL_next_reg_id(), PL_native_int_reg_ctype, 1);
      and_op->dest[0] = t1;
      and_op->src[0] = L_copy_operand(eq);
      and_op->src[1] = L_copy_operand(lt);
      L_insert_oper_before(cb, op, and_op);

      or_op = L_create_new_op(Lop_OR);
      t2 = L_new_register_operand(PL_next_reg_id(), PL_native_int_reg_ctype, 1);
      or_op->dest[0] = t2;
      or_op->src[0] = L_copy_operand(preds[1]);
      or_op->src[1] = L_copy_operand(t1);
      L_insert_oper_before(cb, op, or_op);
      temp_dest = t2;
    }
    else {
      for(i=0; i<num_native_ints; i++) {
        if(i == 0) {
          temp_op = L_create_new_op(Lop_MOV);
          temp_op->dest[0] = L_new_register_operand(PL_next_reg_id(), PL_native_int_reg_ctype, 1);
          temp_dest = temp_op->dest[0];
          temp_op->src[0] = L_copy_operand(preds[i]);
          L_insert_oper_before(cb, op, temp_op);
        }
        else {
          temp_op = L_create_new_op(Lop_AND);
          temp_op->dest[0] = L_copy_operand(temp_dest);
          temp_op->src[0] = L_copy_operand(temp_dest);
          temp_op->src[1] = L_copy_operand(preds[i]);
          L_insert_oper_before(cb, op, temp_op);
        }
      }
    }
  }

  else if(L_int_bgt_branch_opcode(op)) {
    for(i=0; i<num_native_ints; i++) {
      if(i == 0) {
        temp_op = L_create_new_op(Lop_MOV);
        temp_op->dest[0] = L_new_register_operand(PL_next_reg_id(), PL_native_int_reg_ctype, 1);
        temp_dest = temp_op->dest[0];
        temp_op->src[0] = L_copy_operand(preds[i]);
        L_insert_oper_before(cb, op, temp_op);
      }
      else {
        temp_op = L_create_new_op(Lop_AND);
        temp_op->dest[0] = L_copy_operand(temp_dest);
        temp_op->src[0] = L_copy_operand(temp_dest);
        temp_op->src[1] = L_copy_operand(preds[i]);
        L_insert_oper_before(cb, op, temp_op);
      }
    }
  }
  else if(L_int_bge_branch_opcode(op)) {
    for(i=0; i<num_native_ints; i++) {
      if(i == 0) {
        temp_op = L_create_new_op(Lop_MOV);
        temp_op->dest[0] = L_new_register_operand(PL_next_reg_id(), PL_native_int_reg_ctype, 1);
        temp_dest = temp_op->dest[0];
        temp_op->src[0] = L_copy_operand(preds[i]);
        L_insert_oper_before(cb, op, temp_op);
      }
      else {
        temp_op = L_create_new_op(Lop_AND);
        temp_op->dest[0] = L_copy_operand(temp_dest);
        temp_op->src[0] = L_copy_operand(temp_dest);
        temp_op->src[1] = L_copy_operand(preds[i]);
        L_insert_oper_before(cb, op, temp_op);
      }
    }
  }

  else {
    L_punt("Not handling long long branch for op %d\n", op->id);
  }
  temp_op = L_create_new_op(Lop_BR);
  temp_op->src[0] = L_copy_operand(temp_dest);
  temp_op->src[1] = L_new_int_operand(1, PL_native_int_reg_ctype);
  temp_op->src[2] = L_copy_operand(op->src[2]);
  L_set_compare(temp_op, PL_native_int_reg_ctype, Lcmp_COM_EQ);
  L_insert_oper_before(cb, op, temp_op);

  L_delete_oper(cb, op);
}

void PL_llong_codegen_zxt(L_Cb *cb, L_Oper *op)
{
  L_Operand **new_dest;
  L_Oper *temp_op;

  L_Operand **src1;
  int i;

  if(L_is_ctype_llong(op->src[0])) {
    if(!L_is_ctype_llong(op->dest[0])) {
      src1 = (L_Operand **)HashTable_find(llong_operand_hash, op->src[0]->value.r);
      op->src[0] = L_copy_operand(src1[0]);
      return;
    }
  }

  if(L_is_ctype_llong(op->src[0])) {
    src1 = (L_Operand **)HashTable_find(llong_operand_hash, op->src[0]->value.r);
    new_dest = (L_Operand **)malloc(sizeof(L_Operand *) * num_native_ints);
    for(i=0; i<num_native_ints; i++) {
      new_dest[i] = L_new_register_operand(PL_next_reg_id(), PL_native_int_reg_ctype, 1);
      temp_op = L_create_new_op(Lop_MOV);
      temp_op->dest[0] = new_dest[i];
      temp_op->src[0] = L_copy_operand(src1[i]);
      L_insert_oper_before(cb, op, temp_op);
    }

    if(HashTable_find_or_null(llong_operand_hash, op->dest[0]->value.r)) {
      for(i=0; i<num_native_ints; i++) {
        L_Operand **temp_dest = (L_Operand **)HashTable_find(llong_operand_hash, op->dest[0]->value.r);
        temp_op = L_create_new_op(Lop_MOV);
        temp_op->dest[0] = L_copy_operand(temp_dest[i]);
        temp_op->src[0] = L_copy_operand(new_dest[i]);
        L_insert_oper_before(cb, op, temp_op);
      }
    }
    else {
      HashTable_insert(llong_operand_hash, op->dest[0]->value.r, (void *)new_dest);
    }
    L_delete_oper(cb, op);
  }
  else if(L_is_ctype_integer(op->src[0])) {
    new_dest = (L_Operand **)malloc(sizeof(L_Operand *) * num_native_ints);
    for(i=0; i<num_native_ints; i++) {
      new_dest[i] = L_new_register_operand(PL_next_reg_id(), PL_native_int_reg_ctype, 1);
      temp_op = L_create_new_op(Lop_MOV);
      temp_op->dest[0] = new_dest[i];
      if(i == 0) {
        temp_op->src[0] = L_copy_operand(op->src[0]);
      }
      else {
        temp_op->src[0] = L_new_int_operand(0, PL_native_int_reg_ctype);
      }
      L_insert_oper_before(cb, op, temp_op);
    }

    if(HashTable_find_or_null(llong_operand_hash, op->dest[0]->value.r)) {
      P_punt("old_dest already in hash table\n");
    }

    HashTable_insert(llong_operand_hash, op->dest[0]->value.r, (void *)new_dest);
    L_delete_oper(cb, op);
  }

}

void PL_llong_codegen_rcmp(L_Cb *cb, L_Oper *op)
{
  L_Oper *new_rcmp;
  L_Oper *temp_op;

  L_Operand **preds;
  L_Operand **src1, **src2;
  L_Operand *temp_dest = NULL;
  int i;

  src1 = (L_Operand **)HashTable_find(llong_operand_hash, op->src[0]->value.r);
  if(L_is_constant(op->src[1])) {
    union {long long x; struct {int lo, hi;} parts;} buf;
    buf.x = op->src[1]->value.i;
    src2 = (L_Operand **)malloc(sizeof(L_Operand *) * num_native_ints);
    for(i=0; i<num_native_ints; i++) {
      L_Oper *new_op = L_create_new_op(Lop_MOV);
      src2[i] = L_new_register_operand(PL_next_reg_id(), PL_native_int_reg_ctype, 1);
      new_op->dest[0] = src2[i];
      /* KVM : TBD : handling only 32-bit machine here. */
      if(i == 0)
        new_op->src[0] = L_new_gen_int_operand(buf.parts.lo);
      else
        new_op->src[0] = L_new_gen_int_operand(buf.parts.hi);
      L_insert_oper_before(cb, op, new_op);
    }
  }
  else {
    src2 = (L_Operand **)HashTable_find(llong_operand_hash, op->src[1]->value.r);
  }

  preds = (L_Operand **)malloc(sizeof(L_Operand *) * num_native_ints);
  for(i=0; i<num_native_ints; i++) {
    preds[i] = L_new_register_operand(PL_next_reg_id(), PL_native_int_reg_ctype, 1);
  }

  for(i=num_native_ints - 1; i>=0; i--) {
    new_rcmp = L_create_new_op(Lop_RCMP);
    new_rcmp->dest[0] = L_new_register_operand(PL_next_reg_id(), PL_native_int_reg_ctype, 1);
    preds[i] = new_rcmp->dest[0];
    new_rcmp->src[0] = L_copy_operand(src1[i]);
    new_rcmp->src[1] = L_copy_operand(src2[i]);
    L_copy_compare(new_rcmp, op);
    new_rcmp->com[0] = PL_native_int_reg_ctype;
    L_insert_oper_before(cb, op, new_rcmp);
  }

  if(L_int_ne_rcmp_opcode(op) || L_int_eq_rcmp_opcode(op)) {
    for(i=0; i<num_native_ints; i++) {
      if(i == 0) {
        temp_op = L_create_new_op(Lop_MOV);
        temp_op->dest[0] = L_copy_operand(op->dest[0]);
        temp_dest = temp_op->dest[0];
        temp_op->src[0] = L_copy_operand(preds[i]);
        L_insert_oper_before(cb, op, temp_op);
      }
      else {
        if(L_int_ne_rcmp_opcode(op))
          temp_op = L_create_new_op(Lop_OR);
        else 
          temp_op = L_create_new_op(Lop_AND);
        temp_op->dest[0] = L_copy_operand(temp_dest);
        temp_op->src[0] = L_copy_operand(temp_dest);
        temp_op->src[1] = L_copy_operand(preds[i]);
        L_insert_oper_before(cb, op, temp_op);
      }
    }
  }

  L_delete_oper(cb, op);
}

void PL_llong_codegen_fn(L_Func *fn)
{
  L_Attr *attr;
  L_Attr *new_attr;

  int i, j;
  int num_llong_params = 0;

  attr = L_find_attr(fn->attr, "tr");
  if(attr) {
    for(i=0; i<attr->max_field; i++) {
      if(L_is_ctype_llong(attr->field[i])) num_llong_params++;
    }
    if(num_llong_params == 0) return;

    new_attr = L_new_attr("tr", attr->max_field+num_llong_params);

    j = 0;

    for(i=0; i<attr->max_field; i++) {
      L_set_macro_attr_field (new_attr, j, attr->field[i]->value.mac, PL_native_int_reg_ctype, L_PTYPE_NULL);
      j++;
      if(L_is_ctype_llong(attr->field[i])) {
        L_set_macro_attr_field (new_attr, j, attr->field[i]->value.mac + 1, PL_native_int_reg_ctype, L_PTYPE_NULL);
        j++;
      }
    }

    fn->attr = L_delete_attr(fn->attr, attr);
    L_concat_attr(fn->attr, new_attr);
  }
}

void PL_llong_codegen_fix_load_offset(L_Cb *cb, L_Oper *op)
{
  L_Operand **new_offset;

  new_offset = (L_Operand **)HashTable_find(llong_operand_hash, op->src[1]->value.r);
  op->src[1] = L_copy_operand(new_offset[0]);
}

void PL_llong_codegen(L_Func *l_fn, L_Cb *cb, L_Oper *op)
{
  if(L_int_load_opcode(op)) {
    if(L_is_ctype_llong(op->dest[0])) {
      DEBUGLL("load");
      PL_llong_codegen_load(cb, op);
    }
    else if(L_is_ctype_llong(op->src[1])) {
      PL_llong_codegen_fix_load_offset(cb, op);
    }
  }
  else if(L_int_store_opcode(op)) {
    if(L_is_ctype_llong(op->src[2])) {
      DEBUGLL("store");
       PL_llong_codegen_store(cb, op);
    }
    else if(L_is_ctype_llong(op->src[0]) || L_is_ctype_llong(op->src[1])) {
      L_punt("op %d Not handling llong offsets for store\n", op->id);
    }
  }
  else if(L_add_opcode(op) || L_sub_opcode(op)) {
    DEBUGLL("add_sub");
    PL_llong_codegen_add_sub(cb, op);
  }
  else if(L_sign_extend_opcode(op)) {
    DEBUGLL("sxt");
    PL_llong_codegen_sxt(cb, op);
  }
  else if(L_int_mul_opcode(op)) {
    DEBUGLL("mul");
    PL_llong_codegen_mul(cb, op);
  }
  else if(L_int_move_opcode(op)) {
    DEBUGLL("mov");
    PL_llong_codegen_move(cb, op);
  }
  else if(op->opc == Lop_DEFINE) {
    DEBUGLL("def");
    PL_llong_codegen_define(cb, op);
  }
  else if(L_subroutine_call_opcode(op)) {
    PL_llong_codegen_jsr(cb, op);
  }
  else if(op->opc == Lop_LSR) {
    DEBUGLL("lsr");
    PL_llong_codegen_lsr(l_fn, cb, op);
  }
  else if(op->opc == Lop_ASR) {
    DEBUGLL("asr");
    PL_llong_codegen_asr(l_fn, cb, op);
  }
  else if(op->opc == Lop_LSL) {
    DEBUGLL("lsl");
    PL_llong_codegen_lsl(l_fn, cb, op);
  }
  else if(L_logic_opcode(op)) {
    DEBUGLL("logic");
    PL_llong_codegen_logic(cb, op);
  }
  else if(L_int_cond_branch_opcode(op)) {
    DEBUGLL("branch");
    PL_llong_codegen_branch(cb, op);
  }
  else if(L_zero_extend_opcode(op)) {
    DEBUGLL("zxt");
    PL_llong_codegen_zxt(cb, op);
  }
  else if(L_int_comparison_opcode(op)) {
    DEBUGLL("cmp");
    PL_llong_codegen_rcmp(cb, op);
  }
  else {
    P_punt("PL_llong_codegen : TBD : No translation for op %d (%s)\n", op->id, op->opcode);
  }
}

void PL_translate_longlong(L_Func *l_fn)
{
  L_Cb *cb;
  L_Oper *op;
  int i;

  /*
  char str[1024];

  FILE *fp;

  sprintf(str, "%s.before", l_fn->name);
  printf("LLONG Translating function %s\n", l_fn->name);
  fp = fopen(str, "w");
  L_print_func(fp, l_fn);
  fclose(fp);
  */

  PL_llong_codegen_fn(l_fn);

  llong_operand_hash = HashTable_create(127);

  for (cb = l_fn->first_cb; cb; cb = cb->next_cb) {
    for (op = cb->first_op; op;) {
      int flag = 0;

      for(i=0; i<L_max_dest_operand; i++) {
        if(L_is_ctype_llong(op->dest[i])) {
          flag = 1;
          break;
        }
      }
      for(i=0; i<L_max_src_operand; i++) {
        if(L_is_ctype_llong(op->src[i])) {
          flag = 1;
          break;
        }
      }

      if(flag == 1 || L_subroutine_call_opcode(op)) {
        L_Oper *temp_op = op->next_op;
        PL_llong_codegen(l_fn, cb, op);
        op = temp_op;
      }
      else {
        op = op->next_op;
      }
    }
  }

  HashTable_free(llong_operand_hash);

  /*
  sprintf(str, "%s.after", l_fn->name);
  fp = fopen(str, "w");
  L_print_func(fp, l_fn);
  fclose(fp);
  */
}

int
PL_uses_pointer_operand (Expr expr)
{
  int ret = 0;

  if (!expr)
    ret = 0;
  else
    switch (expr->opcode)
      {
	/* constants never a ptr */
      case OP_enum:
      case OP_int:
      case OP_float:
      case OP_double:
      case OP_char:
      case OP_string:
      case OP_expr_size:
      case OP_type_size:
	ret = 0;
	break;
	/* pointer if ret type is ptr */

      case OP_var:
      case OP_dot:
      case OP_arrow:
      case OP_cast:
      case OP_indr:
      case OP_addr:
      case OP_index:
      case OP_compexpr:
      case OP_assign:
      case OP_or:
      case OP_xor:
      case OP_and:
      case OP_rshft:
      case OP_lshft:
      case OP_add:
      case OP_sub:
      case OP_mul:
      case OP_div:
      case OP_mod:
      case OP_Aadd:
      case OP_Asub:
      case OP_Amul:
      case OP_Adiv:
      case OP_Amod:
      case OP_Arshft:
      case OP_Alshft:
      case OP_Aand:
      case OP_Aor:
      case OP_Axor:
      case OP_neg:
      case OP_not:
      case OP_inv:
      case OP_preinc:
      case OP_predec:
      case OP_postinc:
      case OP_postdec:
      case OP_call:
	ret = PST_IsPointerType (PL_symtab, PST_ExprType(PL_symtab, expr));
	break;

	/* illegal OP's in flattened Hcode */
      case OP_quest:
      case OP_disj:
      case OP_conj:
	P_punt ("PL_uses_pointer_operand: Hcode should be flattened");
	break;

	/* These always ret int, so check operands */
      case OP_eq:
      case OP_ne:
      case OP_lt:
      case OP_le:
      case OP_ge:
      case OP_gt:
	ret = (PL_uses_pointer_operand (expr->operands) ||
	       PL_uses_pointer_operand (expr->operands->sibling));
	break;

      default:
	P_punt ("PL_uses_pointer_operand: illegal opcode");
      }
  return (ret);
}


L_Attr *
PL_gen_pointer_attr (Expr expr)
{
  L_Attr *attr = NULL;

  if (PL_uses_pointer_operand (expr))
    attr = PL_gen_attr ("ptr", 0);

  return (attr);
}


L_Attr *
PL_gen_if_attr (Pragma pragma, Expr expr)
{
  L_Attr *attr;

  attr = PL_gen_attr_from_pragma (pragma);
  attr = L_concat_attr (attr, PL_gen_pointer_attr (expr));

  /* LCW -- concatnate the pragma of the condition expression of a 
   * if-statement to the attributes of this if-statement for line no,
   * file name and scope information - 8/9/95 
   */

  /* TM: Don't process expr->pragma coz we don't want to transfer this to Lcode. */
#if 0
  if (expr->pragma)
    attr = L_concat_attr (attr, PL_gen_attr_from_pragma (expr->pragma));
#endif

  return (attr);
}


static void
PLI_gen_if (L_Cb * src_cb, L_Cb * dst_cb, Expr expr, Pragma pragma, int inv)
{
  ITuint8 com = 0;
  int uns = 0;
  PL_Operand hcop1, hcop2;
  _PL_Ret cc;
  /* L_Oper *new_oper; */
  L_Attr *new_attr;

  PLI_gen_data (src_cb, expr, &cc);

  hcop1 = &(cc.op1);
  hcop2 = &(cc.op2);

  switch (cc.type)
    {
    case PL_RET_NONE:
      break;
    case PL_RET_SIMPLE:
      com = inv ? Lcmp_COM_EQ : Lcmp_COM_NE;
      uns = 0;
      hcop2 = NULL;
      break;
    case PL_RET_EQ:
      com = inv ? Lcmp_COM_NE : Lcmp_COM_EQ;
      uns = 0;
      break;
    case PL_RET_NE:
      com = inv ? Lcmp_COM_EQ : Lcmp_COM_NE;
      uns = 0;
      break;
    case PL_RET_GT:
      com = inv ? Lcmp_COM_LE : Lcmp_COM_GT;
      uns = 0;
      break;
    case PL_RET_GE:
      com = inv ? Lcmp_COM_LT : Lcmp_COM_GE;
      uns = 0;
      break;
    case PL_RET_LT:
      com = inv ? Lcmp_COM_GE : Lcmp_COM_LT;
      uns = 0;
      break;
    case PL_RET_LE:
      com = inv ? Lcmp_COM_GT : Lcmp_COM_LE;
      uns = 0;
      break;
    case PL_RET_GT_U:
      com = inv ? Lcmp_COM_LE : Lcmp_COM_GT;
      uns = 1;
      break;
    case PL_RET_GE_U:
      com = inv ? Lcmp_COM_LT : Lcmp_COM_GE;
      uns = 1;
      break;
    case PL_RET_LT_U:
      com = inv ? Lcmp_COM_GE : Lcmp_COM_LT;
      uns = 1;
      break;
    case PL_RET_LE_U:
      com = inv ? Lcmp_COM_GT : Lcmp_COM_LE;
      uns = 1;
      break;
    default:
      PLI_simplify (src_cb, &cc);
      com = inv ? Lcmp_COM_EQ : Lcmp_COM_NE;
      uns = 0;
      hcop1 = &(cc.op1);
      hcop2 = NULL;
      break;
    }

  new_attr = PL_gen_if_attr (pragma, expr);

  PL_gen_cbr (src_cb, dst_cb, hcop1, hcop2, com, uns, new_attr);
  return;
}


static int
PLI_gen_return (L_Cb * cb, Expr expr, Key return_type, int return_addr_reg)
{
  _PL_Ret cc;
  _M_Type mtype;
  _PL_Operand dest, src, src1, src2;
  char line[64];
  int rv_is_aggr, rv_is_indir_aggr = 0;
  int rv_is_longlong = 0;

  PL_pcode2lcode_type (return_type, &mtype, 0);
  
  if (!expr || PST_IsVoidType(PL_symtab, return_type))
    return 0;

  rv_is_aggr = PL_is_aggr_type (return_type, &return_type);

  rv_is_longlong = PST_IsLongLongType(PL_symtab, return_type);

  if (rv_is_aggr)
    {
      _M_Param rv_param;
      PL_pcode2lcode_type (return_type, &(rv_param.mtype), 0);
      M_layout_retvar (&rv_param, M_GET_FNVAR);
      rv_is_indir_aggr = !PL_gen_compliant_struct_return ||
	(rv_param.mode != M_THRU_REGISTER);

      if (rv_is_indir_aggr)
	{
	  /* Indirect thru return register */
	  int type_size, type_align, bt;

	  bt = PST_GetTypeBasicType (PL_symtab, return_type);
	  type_size = PL_key_get_size (return_type);
	  type_align = PL_key_get_align (return_type);

	  if (bt & BT_UNION)
	    P_warn ("Function %s() returns a union\n", L_fn->name);

	  if (M_arch == M_TAHOE)
	    {
	      if (PL_is_HFA_type (return_type))
		P_warn ("Non-SCRAG-compliant HFA struct return in %s()",
			L_fn->name);
	      else if (PL_key_get_size (return_type) <= 32)
		P_warn ("Non-SCRAG-compliant small struct return in %s()",
			L_fn->name);
	    }

	  PLI_gen_addr (cb, expr, &cc);
	  PLI_simplify (cb, &cc);
	  PL_new_register (&dest, return_addr_reg, M_TYPE_POINTER, 0);
	  PL_gen_block_mov (cb, expr, &dest, 0, &(cc.op1), 0, 
			    type_size, type_align, 0, 0, 0, 1);
	  src = dest;
	  sprintf (line, "$P%d", M_return_register (M_TYPE_POINTER, 
						    M_PUT_FNVAR));
	  PL_new_macro (&dest, line, M_TYPE_POINTER, 0);
	  if (!M_return_value_thru_stack())
	    {
	      PL_gen_mov (cb, &dest, &src, 0);
	    }
	  else
	    {
	      // PL_new_macro(&src1, "$IP", M_TYPE_POINTER, 0);
	      // PL_new_int(&src2, M_return_value_offset(), 0);
	      // PL_gen_store(cb, expr, &src1, &src2, &(src), M_TYPE_POINTER, 0);
	    }
	}
      else
	{
	  /* Aggregate passed through return registers */
	  _PL_Operand opd_ofst, opd_val;
	  int tsize = PL_MType_Size(PL_native_int_reg_mtype) / 8,
	    offset = 0, r;

	  PLI_gen_data (cb, expr, &cc);
	  PLI_simplify (cb, &cc);

	  for (r = rv_param.su_sreg; r <= rv_param.su_ereg ; r++)
	    {
	      sprintf (line, "$P%d", M_return_register (M_TYPE_INT, 
							M_PUT_FNVAR));
	      PL_new_macro (&opd_val, line, PL_native_int_reg_mtype, 0);
	      PL_new_int (&opd_ofst, offset, 0);
	      PL_gen_load (cb, expr, &opd_val, &(cc.op1), &opd_ofst, 
			   dest.data_type, L_new_attr ("RVFILL", 0));
	      offset += tsize;
	    }
	}
    }
  else
    {
      /* "Normal" Thru-register */

      int mtype_type, is_unsigned;

      mtype_type = PL_key_get_mtype (return_type);
      is_unsigned = PST_IsUnsignedType(PL_symtab, return_type);

      PLI_gen_data (cb, expr, &cc);
      PLI_simplify (cb, &cc);

      sprintf (line, "$P%d", M_return_register (mtype_type, M_PUT_FNVAR));
      PL_new_macro (&dest, line, mtype_type, is_unsigned);
      if (!M_return_value_thru_stack() && !rv_is_longlong)
        {
	  PL_gen_mov (cb, &dest, &(cc.op1), 0);
        }
      else if(rv_is_longlong) {
          PL_new_macro(&src1, "$IP", M_TYPE_LLONG, 0);
          PL_new_int(&src2, M_return_value_offset(), 0);
          PL_gen_store(cb, expr, &src1, &src2, &(cc.op1), mtype.type, 0);
      }
      else
        {
          PL_new_macro(&src1, "$IP", M_TYPE_POINTER, 0);
          PL_new_int(&src2, M_return_value_offset(), 0);
          PL_gen_store(cb, expr, &src1, &src2, &(cc.op1), mtype.type, 0);
        }
    }

  return 1;
}


static void
PLI_gen_expr (L_Cb * cb, Expr expr)
{
  _PL_Ret cc;

  if (!expr)
    return;

  PLI_gen_data (cb, expr, &cc);
  PLI_simplify (cb, &cc);
}


static void
PL_changelabel (Expr e, void *data)
{
  if (e->opcode == OP_var)
    {
      Key k;
      char *n;
      VarDcl v;
      
      n = e->value.var.name;
      k = e->value.var.key;
      v = PSI_GetVarDclEntry (k);

      if (v && strcmp (n, v->name))
	{
	  fprintf (stderr, "FIXED ONE: %s -> %s\n", n, v->name);
	  e->value.var.name = strdup (v->name);
	}
    }
}


static List
PL_LinkLocalDataDcl (FuncDcl fn)
{
  int et_types;
  Key scope_key;
  Key key;
  List list = NULL;

  et_types = (ET_VAR_LOCAL);
  scope_key = PST_GetStmtScope(PL_symtab, fn->stmt);

  for (key = PST_GetFileEntryByType (PL_symtab, scope_key.file, et_types);
       P_ValidKey (key);
       key = PST_GetFileEntryByTypeNext (PL_symtab, key, et_types))
    
    {
      SymTabEntry entry;

      if (!PST_ScopeContainsKey(PL_symtab, scope_key, key))
	continue;

      entry = PST_GetSymTabEntry (PL_symtab, key);
      
      switch (P_GetSymTabEntryType (entry))
	{
	case ET_VAR_LOCAL:
	  {
	    VarDcl var_dcl = P_GetSymTabEntryVarDcl (entry);

            /* HCH 5/31/04: globalize local variables for object scheduling */
	    Key base_type;
            _BasicType bt;
	    int mtype_type;
	    _VarQual qual;

            base_type = PST_GetBaseType (PL_symtab, var_dcl->type);
            bt = PST_GetTypeBasicType(PL_symtab, base_type);
            mtype_type = PL_key_get_mtype(var_dcl->type);

            if ( PL_globalize_lvars &&
		 (!P_FindPragma (fn->pragma, "\"RECURSIVE\"") && 
		  ((mtype_type != M_TYPE_POINTER) &&
		   (PST_IsStructureType (PL_symtab, var_dcl->type) ||
		    PST_IsArrayType(PL_symtab, var_dcl->type)))) )
              {
                fprintf (stderr, "HCH: CONVERTING LV %s TO GLOB\n",
			 var_dcl->name);

		qual = P_GetVarDclQualifier(var_dcl);
		P_ClrVarDclQualifier(var_dcl, qual); 
		P_SetVarDclQualifier(var_dcl, VQ_GLOBAL);
		
                if (var_dcl->pragma)
                  {
                    Pragma prag;
                    prag = P_GetVarDclPragma(var_dcl);
                    if (P_FindPragma (prag, "OBJID"))
		      {
			char new_name[1024];

			strcpy (new_name,var_dcl->name);
			strcat (new_name, "_");
			strcat (new_name, fn->name);
			sprintf (new_name, "%s_%d_%d", new_name, 
				 key.file, key.sym);
			P_SetVarDclName (var_dcl, strdup (new_name));
		      }
                  }

                /* HCH: ripped from do_pcode.c L407: case ET_VAR_GLOBAL */
                /* Create and dump global data definitions */
		PL_gen_lcode_var (L_datalist, var_dcl);
              }
            else if (!(P_GetVarDclQualifier(var_dcl) & VQ_PARAMETER))
	      {
		list = List_insert_last(list, var_dcl);
	      }
	  }
	  break;
	default:
	  break;
	}
    }
  
  if ( PL_globalize_lvars && (!P_FindPragma (fn->pragma, "\"RECURSIVE\"")) )
    P_StmtApply (fn->stmt, NULL, PL_changelabel, NULL);
  
  return list;
}
