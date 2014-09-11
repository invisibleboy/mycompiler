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
 *      File :          l_check.c
 *      Description :   Check data structures.
 *      Author: Po-hua Chang, Wen-mei Hwu
 *      Creation Date:  June 1990
 *      Revised : Scott A. Mahlke, February 1993.
 *              revise to account for new Lcode data structures
 *
\*****************************************************************************/
/* 10/29/02 REK Adding config.h */
#include <config.h>
#include <Lcode/l_main.h>

#define ERR     stderr

void
L_check_expr (L_Expr * expr)
{
  if (expr == NULL)
    {
      return;
    }
  switch (expr->type)
    {
    case L_EXPR_INT:
    case L_EXPR_FLOAT:
    case L_EXPR_DOUBLE:
      break;
    case L_EXPR_LABEL:
      if (expr->value.l == NULL)
        {
          L_punt ("L_check_expr failed: bad label");
        }
      break;
    case L_EXPR_STRING:
      if (expr->value.s == NULL)
        {
          L_punt ("L_check_expr failed: bad string");
        }
      break;
    case L_EXPR_ADD:
    case L_EXPR_SUB:
    case L_EXPR_MUL:
    case L_EXPR_DIV:
      if ((expr->A == NULL) || (expr->B == NULL))
        {
          L_punt ("L_check_expr failed: bad subexpressions");
        }
      L_check_expr (expr->A);
      L_check_expr (expr->B);
      break;
    case L_EXPR_NEG:
    case L_EXPR_COM:
      if (expr->A == NULL)
        {
          L_punt ("L_check_expr failed: bad subexpressions");
        }
      L_check_expr (expr->A);
      break;
    default:
      L_punt ("L_check_expr failed: illegal type");
    }
}

void
L_check_ldcltr (L_Dcltr * dcltr)
{
  if (dcltr == NULL)
    return;

  switch (dcltr->method)
    {
    case L_D_ARRY:
    case L_D_PTR:
    case L_D_FUNC:
      if (dcltr->index != NULL)
        L_check_expr (dcltr->index);
      L_check_ldcltr (dcltr->next);
      break;
    default:
      L_punt ("L_check_ldcltr: unknown method");
      break;
    }
}

void
L_check_ltype (L_Type * ltype)
{
  unsigned int all_flags;

  if (ltype == NULL)
    return;

  all_flags = (L_DATA_CONST | L_DATA_VOLATILE | L_DATA_NOALIAS |
               L_DATA_REGISTER | L_DATA_AUTO | L_DATA_STATIC |
               L_DATA_EXTERN | L_DATA_GLOBAL | L_DATA_PARAMETER |
               L_DATA_VOID | L_DATA_CHAR | L_DATA_SHORT |
               L_DATA_INT | L_DATA_LONG | L_DATA_LONGLONG | L_DATA_FLOAT |
               L_DATA_DOUBLE | L_DATA_SIGNED | L_DATA_UNSIGNED |
               L_DATA_STRUCT | L_DATA_UNION | L_DATA_ENUM | L_DATA_VARARG);
  if ((ltype->type & ~all_flags) != 0)
    L_punt ("L_check_ltype: Illegal bit set in type flags");

  L_check_ldcltr (ltype->dcltr);
}

void
L_check_data (L_Data * data)
{
  int n;
  L_Expr *label, *expr;
  if (data == NULL)
    {
      L_punt ("L_check_data failed: nil argument");
    }
  switch (data->type)
    {
    case L_INPUT_MS:
      n = data->N;
      if ((n < L_MS_TEXT) || (n > L_MS_SYNC))
        {
          L_punt ("L_check_data failed: (ms) unknown segment");
        }
      break;
    case L_INPUT_VOID:
    case L_INPUT_GLOBAL:
      label = data->address;
      if (label == NULL)
        {
          L_punt ("L_check_data failed: (%s) missing label",
                  L_lcode_name (data->type));
        }
      if (label->type != L_EXPR_LABEL)
        {
          L_punt ("L_check_data failed: (%s) illegal label field",
                  L_lcode_name (data->type));
        }
      L_check_expr (label);
      L_check_ltype (data->h_type);
      break;
    case L_INPUT_BYTE:
    case L_INPUT_WORD:
    case L_INPUT_LONG:
    case L_INPUT_LONGLONG:
    case L_INPUT_FLOAT:
    case L_INPUT_DOUBLE:
    case L_INPUT_ALIGN:
    case L_INPUT_ELEMENT_SIZE:
      n = data->N;
      if (n < 0)
        {
          L_warn ("L_check_data failed: (%s) N can not be negative",
                  L_lcode_name (data->type));
        }
      label = data->address;
      expr = data->value;
      if (label == NULL)
        {
          L_punt ("L_check_data failed: (%s) missing label",
                  L_lcode_name (data->type));
        }
      if (label->type != L_EXPR_LABEL)
        {
          L_punt ("L_check_data failed: (%s) illegal label field",
                  L_lcode_name (data->type));
        }
      L_check_expr (label);
      L_check_expr (expr);
      break;
    case L_INPUT_ASCII:
    case L_INPUT_ASCIZ:
      label = data->address;
      expr = data->value;
      if ((label == NULL) || (expr == NULL))
        {
          L_punt ("L_check_data failed: (%s) missing fields",
                  L_lcode_name (data->type));
        }
      if (label->type != L_EXPR_LABEL)
        {
          L_punt ("L_check_data failed: (%s) illegal label field",
                  L_lcode_name (data->type));
        }
      L_check_expr (label);
      L_check_expr (expr);
      break;
    case L_INPUT_SKIP:
    case L_INPUT_RESERVE:
      if (data->N <= 0)
        {
          L_punt ("L_check_data failed: (%s) N must be positive",
                  L_lcode_name (data->type));
        }
      break;
    case L_INPUT_WB:
    case L_INPUT_WW:
    case L_INPUT_WI:
    case L_INPUT_WQ:
    case L_INPUT_WF:
    case L_INPUT_WF2:
    case L_INPUT_WS:
      label = data->address;
      expr = data->value;
      if ((label == NULL) || (expr == NULL))
        {
          L_punt ("L_check_data failed: (%s) missing fields",
                  L_lcode_name (data->type));
        }
      L_check_expr (label);
      L_check_expr (expr);
      break;
    case L_INPUT_DEF_STRUCT:
    case L_INPUT_DEF_UNION:
    case L_INPUT_DEF_ENUM:
      label = data->address;
      if (label == NULL)
        L_punt ("L_check_data failed: (%s) missing address",
                L_lcode_name (data->type));
      L_check_expr (label);
      break;
    case L_INPUT_FIELD:
      label = data->address;
      if (label == NULL)
        L_punt ("L_check_data failed: (%s) missing address",
                L_lcode_name (data->type));
      L_check_expr (label);
      if (data->h_type == NULL)
        L_warn ("L_check_data: (%s) h_type is not set",
                L_lcode_name (data->type));
      L_check_ltype (data->h_type);
      break;
    case L_INPUT_ENUMERATOR:
      label = data->address;
      if (label == NULL)
        L_punt ("L_check_data failed: (%s) missing address",
                L_lcode_name (data->type));
      L_check_expr (label);
      expr = data->value;
      L_check_expr (expr);
      break;
    default:
      L_punt ("L_check_data failed: illegal type");
    }
}

void
L_check_datalist (L_Datalist * datalist)
{
  L_Datalist_Element *element;

  if (datalist == NULL)
    return;

  if (datalist->first_element == NULL)
    if (datalist->last_element != NULL)
      L_punt ("L_check_datalist failed: first element is 0, but last is not");
  if (datalist->last_element == NULL)
    if (datalist->first_element != NULL)
      L_punt ("L_check_datalist failed: last element is 0, but first is not");

  for (element = datalist->first_element; element != NULL;
       element = element->next_element)
    {
      if (element->data == NULL)
        L_punt ("L_check_datalist failed: missing data in datalist");
      L_check_data (element->data);
    }
}

void
L_check_operand (L_Operand * op)
{
  if (op == NULL)
    {
      return;
    }
  switch (op->type)
    {
    case L_OPERAND_VOID:
    case L_OPERAND_CB:
      break;
    case L_OPERAND_IMMED:
      if ((!L_is_ctype_integer (op)) && (!L_is_ctype_flt (op))
          && (!L_is_ctype_dbl (op)))
        L_punt ("L_check_operand failed: illegal ctype (i)");
      break;
    case L_OPERAND_STRING:
      if (op->value.s == NULL)
        L_punt ("L_check_operand failed: (s) missing argument");
      break;
    case L_OPERAND_MACRO:
      if (!strcmp (L_macro_name (op->value.mac), "?"))
        L_punt ("L_check_operand failed: (mac) bad index");
      break;
    case L_OPERAND_REGISTER:
      if ((op->value.r < 0) || (op->value.r > L_fn->max_reg_id))
        L_punt ("L_check_operand failed: (r) bad index");
      break;
    case L_OPERAND_RREGISTER:
      if ((op->value.rr < 0) || (op->value.rr > L_fn->max_reg_id))
        L_punt ("L_check_operand failed: (rr) bad index");
      break;
    case L_OPERAND_EVR:
      if ((op->value.evr.num < 0) || (op->value.evr.num > L_fn->max_reg_id))
        L_punt ("L_check_operand failed: (evr) bad index");
      break;
    case L_OPERAND_LABEL:
      if (op->value.l == NULL)
        L_punt ("L_check_operand failed: (l) missing argument");
      break;
    default:
      L_punt ("L_check_operand failed: illegal type");
    }
}

void
L_check_oper (L_Oper * oper)
{
  int i;
  L_Attr *attr;
  if (oper == NULL)
    {
      L_punt ("L_check_oper failed: oper is NULL");
    }
  if ((oper->id < 0) || (oper->id > L_fn->max_oper_id))
    {
      L_punt ("L_check_oper: illegal id");
    }
  if (oper->opcode == NULL)
    {
      L_punt ("L_check_oper failed: missing opcode");
    }
  if (oper->opc < 0)
    {
      L_punt ("L_check_oper failed: illegal opc");
    }
  if (oper->weight < -ZERO_EQUIVALENT)
    {
      L_warn ("L_check_oper: negative oper weight");
    }
  for (i = 0; i < L_max_dest_operand; i++)
    {
      L_check_operand (oper->dest[i]);
    }
  for (i = 0; i < L_max_src_operand; i++)
    {
      L_check_operand (oper->src[i]);
    }
  for (i = 0; i < L_max_pred_operand; i++)
    {
      L_check_operand (oper->pred[i]);
    }
  for (attr = oper->attr; attr != NULL; attr = attr->next_attr)
    {
      if (attr->name == NULL)
        L_punt ("L_check_oper failed: missing attribute name");
    }
  if ((L_ia64) && oper->pred[0] && M_cannot_predicate (oper))
    {
      L_print_oper (stderr, oper);
      L_punt ("L_check_oper: QP on instruction which is not"
              "allowed to be predicated\n");
    }
}

void
L_check_entry_block (L_Cb * cb)
{
  L_Oper *oper, *ptr;
  int opc;
  if (cb == NULL)
    {
      L_punt ("L_check_entry_block failed: cb is NULL");
    }
  if (cb->src_flow != NULL)
    {
      fprintf (stderr, "# cb %d\n", cb->id);
      L_punt ("L_check_entry_block: must not have src flow");
    }
  for (oper = cb->first_op; oper != 0; oper = oper->next_op)
    {
      opc = oper->opc;
      if (opc == Lop_PROLOGUE)
        break;
    }
  if (oper == NULL)
    {
      fprintf (stderr, "# cb %d\n", cb->id);
      L_punt ("L_check_entry_block: must contain (prologue)");
    }
  for (ptr = oper->prev_op; ptr != 0; ptr = ptr->prev_op)
    {
      opc = ptr->opc;
      if (opc != Lop_DEFINE)
        {
          /*L_print_oper(stderr, ptr); */
          break;
        }
    }
  if (ptr != NULL)
    {
      fprintf (stderr, "# cb %d\n", cb->id);
      L_punt
        ("L_check_entry_block: only (define) can appear before (prologue)");
    }
}

/*
 *      Important! - This function calls L_has_fallthru_cb() which
 *      requires the Pred graph be setup for hyperblocks and this 
 *      function does not do the setup itself, so you must call
 *      PG_setup_pred_graph() before calling this function.
 */
void
L_check_cb (L_Func * fn, L_Cb * cb)
{
  L_Oper *oper, *next;
  L_Flow *flow;
  L_Attr *attr;
  double weight;
  if (cb == NULL)
    {
      L_punt ("L_check_cb failed: cb is NULL");
    }
  /* REH 9/24/95 - allow cb with id 0 
     if ((cb->id <= 0) || (cb->id > L_fn->max_cb_id)) {
   */
  if ((cb->id < 0) || (cb->id > L_fn->max_cb_id))
    {
      L_punt ("L_check_cb failed: cb %d : illegal id", cb->id);
    }
  if (cb->weight < -ZERO_EQUIVALENT)
    {
      L_warn ("L_check_cb falied: cb %d : weight is negative", cb->id);
    }
  if (cb->weight2 < -ZERO_EQUIVALENT)
    {
      L_warn ("L_check_cb failed: cb %d : static weight is negative", cb->id);
    }
  if ((cb->first_op == NULL) && (cb->last_op != NULL))
    {
      L_punt ("L_check_cb failed: cb %d : bad operation link", cb->id);
    }
  if ((cb->first_op != NULL) && (cb->last_op == NULL))
    {
      L_punt ("L_check_cb failed: cb %d : bad operation link", cb->id);
    }
  if ((cb->src_flow == NULL) && (cb->dest_flow != NULL) &&
      !L_EXTRACT_BIT_VAL (cb->flags, L_CB_PROLOGUE))
    L_warn ("L_check_cb: cb %d is dead code (no incoming src arcs)", cb->id);
  /* SAM 7-97: cb which violates LC semantics is likely not to pass checker */
  if (L_EXTRACT_BIT_VAL (cb->flags, L_CB_VIOLATES_LC_SEMANTICS))
    return;

    /** check opers **/
  for (oper = cb->first_op; oper != NULL; oper = next)
    {
        /** check link **/
      next = oper->next_op;
      if ((next == NULL) && (oper != cb->last_op))
        L_punt ("L_check_cb failed: bad (last) operation link in cb %d\n",
                cb->id);
      if (oper == cb->first_op)
        {
          if (oper->prev_op != NULL)
            L_punt ("L_check_cb failed: bad (previous) operation link");
        }
      else
        {
          if (oper->prev_op == NULL)
            L_punt ("L_check_cb failed: bad (previous) operation link");
        }
      if (oper == cb->last_op)
        {
          if (oper->next_op != NULL)
            L_punt ("L_check_cb failed: bad (next) operation link");
        }
      else
        {
          if (oper->next_op == NULL)
            L_punt ("L_check_cb failed: bad (next) operation link");
        }
      if (oper->prev_op != NULL)
        {
          if (oper != oper->prev_op->next_op)
            L_punt ("L_check_cb failed: bad (next) operation link");
        }
        /** check operation **/
      L_check_oper (oper);
    }

    /** check control flow **/
    /** check src_flow prev and next links **/
  for (flow = cb->src_flow; flow != NULL; flow = flow->next_flow)
    {
      if (flow == cb->src_flow)
        {
          if (flow->prev_flow != NULL)
            L_punt ("L_check_cb failed: bad (previous) src flow link");
        }
      else
        {
          if (flow->prev_flow == NULL)
            L_punt ("L_check_cb failed: bad (previous) src flow link");
        }
      if (flow->prev_flow != NULL)
        {
          if (flow != flow->prev_flow->next_flow)
            L_punt ("L_check_cb failed: bad (next) src flow link");
        }
    }
    /** check dest_flow prev and next links **/
  for (flow = cb->dest_flow; flow != NULL; flow = flow->next_flow)
    {
      if (flow == cb->dest_flow)
        {
          if (flow->prev_flow != NULL)
            {
              fprintf (stderr, "cb %d, first flow prev ptr not NULL", cb->id);
              L_punt ("L_check_cb failed: bad (previous) dest flow link");
            }
        }
      else
        {
          if (flow->prev_flow == NULL)
            {
              fprintf (stderr, "cb %d, middle flow prev ptr is NULL", cb->id);
              L_punt ("L_check_cb failed: bad (previous) dest flow link");
            }
        }
      if (flow->prev_flow != NULL)
        {
          if (flow != flow->prev_flow->next_flow)
            L_punt ("L_check_cb failed: bad (next) dest flow link");
        }
    }
    /** check src flow weights and for matching dest flows **/
  weight = 0.0;
  for (flow = cb->src_flow; flow != NULL; flow = flow->next_flow)
    {
      L_Cb *src;
      L_Flow *ptr;
      if (flow->dst_cb != cb)
        L_punt ("L_check_cb failed: bad src flow arc");
      if (flow->weight < -ZERO_EQUIVALENT)
        {
          fprintf (ERR, "# negative profile weight (src) %d (%f)\n",
                   cb->id, flow->weight);
          L_warn ("L_check_cb failed: neg flow weight");
        }
      weight += flow->weight;
      src = flow->src_cb;
      for (ptr = src->dest_flow; ptr != NULL; ptr = ptr->next_flow)
        {
          if ((ptr->cc == flow->cc) &&
              (ptr->weight >= flow->weight - ZERO_EQUIVALENT) &&
              (ptr->weight <= flow->weight + ZERO_EQUIVALENT) &&
              (ptr->src_cb == flow->src_cb) && (ptr->dst_cb == flow->dst_cb))
            break;
        }
      if (ptr == NULL)
        {
          fprintf (stderr,
                   "Cb %d src flow: cc %d src_cb %d dst_cb %d weight %f\n",
                   cb->id, flow->cc, flow->src_cb->id, flow->dst_cb->id,
                   flow->weight);
          fprintf (stderr, "has no matching destination flow on cb %d\n",
                   src->id);
          for (ptr = src->dest_flow; ptr != NULL; ptr = ptr->next_flow)
            {
              fprintf (stderr, "\t(flow %d %d %f)\n", ptr->cc,
                       ptr->dst_cb->id, ptr->weight);
            }
          L_punt ("L_check_cb failed: missing dst flow arc");
        }
    }
  if (cb != fn->first_cb)
    {
      /* no check for the entry block */
      if ((weight - cb->weight > .001) || (weight - cb->weight < -.001))
        {
          if (L_verbose_check)
            {
              fprintf (ERR,
                       "# inconsistent profile weight (src): "
                       "fn %s cb %d (%f)\n",
                       fn->name, cb->id, weight - cb->weight);
            }
        }
    }
    /** check dest flow weights and for matching src flows **/
  weight = 0.0;
  for (flow = cb->dest_flow; flow != NULL; flow = flow->next_flow)
    {
      L_Cb *dst;
      L_Flow *ptr;
      if (flow->src_cb != cb)
        {
          fprintf (ERR, "# bad source id (%s, %d)\n", fn->name, cb->id);
          L_punt ("L_check_cb failed: bad dst flow arc");
        }
      if (flow->weight < -ZERO_EQUIVALENT)
        {
          fprintf (ERR, "# negative profile weight (dst) %s %d (%f)\n",
                   fn->name, cb->id, flow->weight);
          L_warn ("L_check_cb: negative flow weight");
        }
      weight += flow->weight;
      dst = flow->dst_cb;
      for (ptr = dst->src_flow; ptr != NULL; ptr = ptr->next_flow)
        {
          if ((ptr->cc == flow->cc) &&
              (ptr->weight >= flow->weight - ZERO_EQUIVALENT) &&
              (ptr->weight <= flow->weight + ZERO_EQUIVALENT) &&
              (ptr->src_cb == flow->src_cb) && (ptr->dst_cb == flow->dst_cb))
            break;
        }
      if (ptr == NULL)
        {
          fprintf (stderr, "Cb %d dst flow: cc %d dst_cb %d weight %f\n",
                   cb->id, flow->cc, flow->dst_cb->id, flow->weight);
          fprintf (stderr, "has no matching source flow on cb %d\n", dst->id);
          for (ptr = dst->src_flow; ptr != NULL; ptr = ptr->next_flow)
            {
              fprintf (stderr, "\t(flow %d %d %f)\n", ptr->cc,
                       ptr->src_cb->id, ptr->weight);
            }
          L_punt ("L_check_cb failed: missing src flow arc");
        }
    }
  if (cb->last_op != NULL)
    {
      int opc;
      opc = cb->last_op->opc;
      if ((opc == Lop_RTS) || (opc == Lop_RTS_FS))
        {
          /* ignore the check for rts block */
          weight = cb->weight;
        }
    }
  if ((weight - cb->weight > ZERO_EQUIVALENT) ||
      (weight - cb->weight < -ZERO_EQUIVALENT))
    {
      if (L_verbose_check)
        fprintf (ERR,
                 "# inconsistent profile weight (dst): fn %s cb %d (%f)\n",
                 fn->name, cb->id, weight - cb->weight);
    }

    /** check that flow arcs match branches */
  flow = cb->dest_flow;
  for (oper = cb->first_op; oper != cb->last_op; oper = oper->next_op)
    {
      if (!L_is_control_oper (oper))
        continue;
      if (L_cond_branch_opcode (oper))
        {
          if (flow == NULL)
            L_punt ("L_check_cb: no matching flow for cond br %d", oper->id);
          if (flow->dst_cb != oper->src[2]->value.cb)
            L_punt ("L_check_cb: flow and cond branch %d dont match",
                    oper->id);
        }
      else if (L_uncond_branch_opcode (oper) && L_is_predicated (oper))
        {
          if (flow == NULL)
            L_punt ("L_check_cb: no matching flow for predicated jump %d",
                    oper->id);
          if (flow->dst_cb != oper->src[0]->value.cb)
            L_punt ("L_check_cb: flow and predicated jump %d dont match",
                    oper->id);
        }
      else if (L_check_branch_opcode (oper))
        {
          if (flow == NULL)
            L_punt ("L_check_cb: no matching flow for check %d", oper->id);

          if (flow->dst_cb != oper->src[1]->value.cb)
            L_punt ("L_check_cb: flow and check %d dont match", oper->id);
        }
      else
        {
          L_print_cb (stderr, fn, cb);
          L_punt ("L_check_cb: illegal branch %d in middle of cb", oper->id);
        }
      flow = flow->next_flow;
    }

  /* Omit the last branch test for Frpized HB because its conservative, 
     SAM 2-98 */
  attr = L_find_attr (cb->attr, "hb_frp");
  if (attr == NULL)
    {
      /* last branch */
      if ((L_cond_branch (cb->last_op))
          || (L_check_branch_opcode (cb->last_op)))
        {
          if (flow == NULL)
            L_punt ("L_check_cb: no matching flow for cond br %d", oper->id);
          if (flow->dst_cb != L_find_branch_dest (oper))
            L_punt ("L_check_cb: flow and cond br %d dont match", oper->id);
          flow = flow->next_flow;
          if (L_has_fallthru_to_next_cb (cb))
            {
              if (flow == NULL)
                L_punt ("L_check_cb: no fallthru flow arc in cb %d", cb->id);
              if (flow->dst_cb != cb->next_cb)
                L_punt ("L_check_cb: cb %d flow and fallthru cb dont match",
                        cb->id);
              if (flow->next_flow != NULL)
                L_punt ("L_check_cb: too many flows cb %d", cb->id);
            }
          else
            {
              if (flow != NULL)
                L_punt ("L_check_cb: too many flows cb %d", cb->id);
            }
        }
      else if (L_uncond_branch (cb->last_op))
        {
          if (flow == NULL)
            L_punt ("L_check_cb: no matching flow for jmp %d", oper->id);
          if (flow->dst_cb != oper->src[0]->value.cb)
            L_punt ("L_check_cb: flow and jmp %d dont match", oper->id);
          flow = flow->next_flow;
          if (L_has_fallthru_to_next_cb (cb))
            {
              if (flow == NULL)
                L_punt ("L_check_cb: missing fallthru flow (cb %d)", cb->id);
              if (flow->dst_cb != cb->next_cb)
                L_punt ("L_check_cb: cb %d flow and fallthru cb dont match",
                        cb->id);
            }
          else
            {
              if (flow != NULL)
                L_punt
                  ("L_check_cb: fallthru flow but no fallthru path (cb %d)",
                   cb->id);
            }

        }
      else if (L_register_branch_opcode (cb->last_op))
        {
          if (flow == NULL)
            L_punt ("L_check_cb: no matching flow for jrg %d", oper->id);
        }
      else if (!L_subroutine_return_opcode (oper))
        {
          if (flow == NULL)
            L_punt ("L_check_cb: no fallthru flow arc in cb %d", cb->id);
          if (flow->dst_cb != cb->next_cb)
            L_punt ("L_check_cb: cb %d flow and fallthru cb dont match",
                    cb->id);
          if (flow->next_flow != NULL)
            L_punt ("L_check_cb: cb %d contains an extra dest flow", cb->id);
        }
    }

    /** check attributes **/
  for (attr = cb->attr; attr != NULL; attr = attr->next_attr)
    {
      if (attr->name == NULL)
        L_punt ("L_check_cb failed: missing attribute name");
    }
}

void
L_check_flow (L_Func * fn)
{
  int *defined, i;
  L_Cb *cb;

  defined = (int *) Lcode_malloc (sizeof (int) * (fn->max_cb_id + 1));

  for (i = 0; i < fn->max_cb_id; i++)
    {
      defined[i] = 0;
    }

  for (cb = fn->first_cb; cb != NULL; cb = cb->next_cb)
    {
      defined[cb->id] = 1;
    }

  for (cb = fn->first_cb; cb != NULL; cb = cb->next_cb)
    {
      L_Flow *flow;
      for (flow = cb->dest_flow; flow != NULL; flow = flow->next_flow)
        {
          L_Cb *src, *dest;
          src = flow->src_cb;
          dest = flow->dst_cb;
          if (defined[src->id] == 0)
            L_punt ("L_check_flow: bad src arc");
          if (defined[dest->id] == 0)
            L_punt ("L_check_flow: bad dest arc, Cb %d -> Cb %d",
                    src->id, dest->id);
          if (src != cb)
            L_punt ("L_check_flow: bad dest arc");
        }
    }

  Lcode_free (defined);
}

void
L_check_func (L_Func * fn)
{
  L_Cb *cb, *next;
  L_Attr *attr;
  if (L_verbose_check)
    {
      fprintf (ERR, "/* L_check_func %s */ \n", fn->name);
    }
  if (fn == NULL)
    L_punt ("L_check_func failed: nil argument");
  if (fn->name == NULL)
    L_punt ("L_check_func failed: no name");
  if (fn->weight < -ZERO_EQUIVALENT)
    {
      L_warn ("L_check_func failed: negative function weight");
    }
  if (fn->n_cb <= 0)
    {
      fprintf (ERR, "fn->n_cb = %d\n", fn->n_cb);
      L_punt ("L_check_func failed: illegal n_cb");
    }
  if (fn->n_oper <= 0)
    {
      fprintf (ERR, "fn->n_oper = %d\n", fn->n_oper);
      L_punt ("L_check_func failed: illegal n_oper");
    }
  if (fn->max_cb_id <= 0)
    {
      fprintf (ERR, "fn->max_cb_id = %d\n", fn->max_cb_id);
      L_punt ("L_check_func failed: illegal max_cb_id");
    }
  if (fn->max_oper_id <= 0)
    {
      fprintf (ERR, "fn->max_oper_id = %d\n", fn->max_oper_id);
      L_punt ("L_check_func failed: illegal max_oper_id");
    }
  if (fn->s_local < 0)
    {
      L_punt ("L_check_func failed: illegal s_local");
    }
  if (fn->s_param < 0)
    {
      L_punt ("L_check_func failed: illegal s_param");
    }
  if (fn->s_swap < 0)
    {
      L_punt ("L_check_func failed: illegal s_swap");
    }

  /* Call added, L_check_cb needs it. SAM 11-97 */
  PG_setup_pred_graph (fn);

    /** check cb's **/
  L_check_entry_block (fn->first_cb);
  for (cb = fn->first_cb; cb != NULL; cb = next)
    {
        /** check link **/
      next = cb->next_cb;
      if ((next == NULL) && (cb != fn->last_cb))
        {
          L_punt ("L_check_func failed: bad (last) cb link");
        }
      if (cb == fn->first_cb)
        {
          if (cb->prev_cb != NULL)
            {
              fprintf (stderr, "First cb %d has cb %d as its previous cb!\n",
                       cb->id, cb->prev_cb->id);
              L_punt ("L_check_func failed: bad (previous) cb link");
            }
        }
      else
        {
          if (cb->prev_cb == NULL)
            {
              fprintf (stderr, "Cb %d has a NULL previous cb link!\n",
                       cb->id);
              L_punt ("L_check_func failed: bad (previous) cb link");
            }
        }
      if (cb == fn->last_cb)
        {
          if (cb->next_cb != NULL)
            L_punt ("L_check_func failed: bad (next) cb link");
        }
      else
        {
          if (cb->next_cb == NULL)
            L_punt ("L_check_func failed: bad (next) cb link");
        }
      if (cb->prev_cb != NULL)
        {
          if (cb != cb->prev_cb->next_cb)
            L_punt ("L_check_func failed: bad (next) cb link");
        }
        /** check block **/
      L_check_cb (fn, cb);
    }

    /** check attributes **/
  for (attr = fn->attr; attr != NULL; attr = attr->next_attr)
    {
      if (attr->name == NULL)
        L_punt ("L_check_func failed: missing attribute name");
    }

    /** check flows **/
  L_check_flow (fn);

}
