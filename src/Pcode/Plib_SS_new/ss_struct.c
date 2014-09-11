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
/*===========================================================================
 *
 *      File :          ss_struct.c
 *      Description :   Pcode SSA Module
 *      Creation Date : October 13, 2004
 *      Author :        James Player
 *
 * This file contains structure manipulation functions used in the Pcode
 * SSA representation.
 * 
 *===========================================================================*/
#include <Pcode/struct.h>
#include "ss_ssa2.h"


PSS_BaseTbl
PSS_BaseTbl_Insert (PSS_BaseTbl table, PSS_Base entry)
{
  if (!table)
    {
      table = ALLOCATE (_PSS_BaseTbl);
      table->first     = entry;
      table->last      = entry;
      table->num_valid = 0;
    }
  else if (!table->last || !table->first)
    {
      table->first     = entry;
      table->last      = entry;
      table->num_valid = 0;
    }
  else
    {
      table->last->next = entry;
      table->last = entry;
    }
  
  table->num_valid++;
    
  return table;
}

PSS_BaseTbl
PSS_BaseTbl_Free (PSS_BaseTbl table)
{
  PSS_Base entry, next;
  
  if (!table) return NULL;
  
  for (entry = table->first; entry; entry = next)
    {
      next = entry->next;
      PSS_Base_Free (entry);
    }

  DISPOSE (table);

  return NULL;
}

PSS_Base
PSS_Base_New (VarDcl vdcl, PSS_Def def)
{
  PSS_DefType undef_type;
  PSS_Def newDef;
  PSS_Base base = ALLOCATE (_PSS_Base);

  base->vdcl         = vdcl;
  base->addr_taken   = 0;
  base->assign_count = 0;
  base->def_count    = 0;
  base->def_bbs      = Set_new ();
  base->def_var_stk  = New_Stack ();
  base->defs         = NULL;
  base->next         = NULL;
  base->ext          = NULL;
  
  /* Push the zero def onto the stack to indicate the bottom */
  SET_DEF_TYPE (undef_type, (vdcl->qualifier & VQ_PARAMETER) ? PARAM : UNDEF);
  newDef = PSS_Def_New (NULL, NULL, UNUSED);

  PSS_AddDefToBase (newDef, base);
  Push_Top (base->def_var_stk, newDef);

  if (def)
    {
      base->def_bbs = Set_add (base->def_bbs, def->bb->ID);
      PSS_AddDefToBase (def, base);
    }

  return base;
}

PSS_Base
PSS_Base_Free (PSS_Base base)
{
  PSS_Def def;

  PS_SetSSABaseEntry (base->vdcl, NULL);
  
  for (def = base->defs; def; def = def->next)
    PSS_Def_Free (def);

  base->def_var_stk = Free_Stack (base->def_var_stk);
  base->def_bbs     = Set_dispose (base->def_bbs);

  DISPOSE (base);

  return NULL;
}

PSS_Use
PSS_Use_New (Expr var, PC_Block bb)
{
  PSS_Use u = ALLOCATE (_PSS_Use);
  
  assert (var->opcode == OP_var);

  u->var = var;
  u->bb  = bb;

  u->next = NULL;
  u->ext  = NULL;

  return u;
}

PSS_Use
PSS_Use_Free (PSS_Use u)
{
  DISPOSE (u);

  return NULL;
}

PSS_Def
PSS_Def_New (Expr var, PC_Block bb, PSS_DefType type)
{ 
  PSS_Def d = ALLOCATE (_PSS_Def);

  assert (UNINITIALIZED_TYPE (type) || var->opcode == OP_var);
        
  d->subscr = UNINITIALIZED_TYPE (type) ? 0 : UNDEF_SUBSCR;
  d->name   = UNDEF_SUBSCR;
  d->var    = var;
  d->bb     = bb;
  d->type   = type;

  d->uses = NULL;
  d->next = NULL;
  d->prev = NULL;
  d->ext  = NULL;

  return d;
}

PSS_Def
PSS_Def_Free (PSS_Def d)
{
  PSS_Use u, next;

  for (u = d->uses; u; u = next)
    {
      next = u->next;
      u = PSS_Use_Free (u);
    }

  if (d->ext)
    P_warn ("PSS_Def_Free: Freeing struct with non-NULL ext field.");
  
  DISPOSE (d);

  return NULL;
}

void
PSS_AddUseToDef (Expr var, PC_Block bb)
{
  PSS_Use use;
  PSS_Def def;

  if (!var) return;
  assert (var->opcode == OP_var);
  assert (var->value.var.ssa);
  
  use = PSS_Use_New (var, bb);
  def = var->value.var.ssa;

  use->next = def->uses;
  def->uses = use;
}

void
PSS_RemoveUseFromDef (Expr var)
{
  PSS_Use use, prev;

  if (!var) return;
  
  use = var->value.var.ssa->uses;
 
  if (!use) return;

  if (use->var == var)
    {
      var->value.var.ssa->uses = use->next;
      use = PSS_Use_Free (use);
    }
  
  else
    {
      prev = use;
      while ((use = use->next))
	{
	  if (use->var == var)
	    {
	      prev->next = use->next;
	      use = PSS_Use_Free (use);
	      break;
	    }
	    
	  prev = use;
	}
    }
}

void 
PSS_AddDefToBase (PSS_Def def, PSS_Base base)
{
  if (!def) return;
 
  if (base->defs)
    {
      base->defs->prev = def;
      def->next = base->defs;
    }
  
  base->defs = def;
  
  if (!UNINITIALIZED_TYPE(def->type))
    {
      base->def_count++;
      base->def_bbs = Set_add (base->def_bbs, def->bb->ID);
    }
}

void
PSS_RemoveDefFromBase (PSS_Def rm_def, PSS_Base base)
{
  PSS_Def def;

  for (def = base->defs; def; def = def->next)
    if (def->var == rm_def->var)
      {
	base->def_count--;

	if (def->next)
	  def->next->prev = def->prev;
	
	if (def->prev == NULL)
	  base->defs = def->next;
	else
	  def->prev->next = def->next;
	    
	break;
      }

  rm_def = PSS_Def_Free (rm_def);
}
