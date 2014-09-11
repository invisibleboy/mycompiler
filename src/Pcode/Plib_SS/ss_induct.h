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


#ifndef __PLIB_SS_INDUCT_H__
#define __PLIB_SS_INDUCT_H__


/************************************************************************************************/

#include <library/llist.h>
#include <library/string_symbol.h>
#include <Pcode/pcode.h>
#include <Pcode/reduce.h>
#include <Pcode/struct.h>
#include <Pcode/cfg.h>
#include <Pcode/loop.h>

extern int funcdcl_ext_vgraph;

/*******************************************************************/

#define LPTR_next(l)		((l)->next)
#define LPTR_ptr(l)		((l)->ptr)

/*******************************************************************/

typedef enum {
   DUMP_SUCC_VNODE = 1
} DumpVnodeFormat;

typedef struct _Alpha_var_id {
  Key key;
  char *name;
  PC_Loop loop;
  int tag;
}
_Alpha_var_id, *Alpha_var_id;

#define Alpha_var_ids_key(v)		((v)->key)
#define Alpha_var_ids_cast(v)           ((Alpha_var_id)(v))
#define Alpha_var_ids_name(v)           (v->name)
#define Alpha_var_ids_tag(v)            (v->tag)
#define Alpha_var_ids_loop(v)           (v->loop)
#if 0
#define Alpha_var_ids_loop_no(v)        (Get_Loop_parloop_depth(Alpha_var_ids_loop(v)))
#endif
#define Alpha_var_ids_loop_no(v)        (Get_Loop_nesting_level(Alpha_var_ids_loop(v)))
#define Alpha_var_ids_const_p(v)        (Alpha_var_ids_loop(v) == NULL)
#define Alpha_var_ids_is_index(v)       (Alpha_var_ids_loop(v) != NULL)
#define Alpha_var_ids_index_p(v)        (Alpha_var_ids_is_index(v))

/************************************************************************************************/
/*
 * _P_Value: record the possible (symbolic) values of a Pcode expression,
 * with the following encoding of (loop, init_value, incr_value):
 *
 * NULL: yet to find the actual value, i.e. we don't know whether the value will be KNOWN or UNKNOWN.
 * UNKNOWN_VALUE: we cannot find the value either because it's too complicated, or because we are too lazy.
 * <vid->is_index == 0, init.of_invariant, NULL>: (symbolic) constant
 * <vid->is_index == 1, init.of_variant, incr>: induction value
 */

typedef struct _P_Value {

  struct _Alpha_var_id *vid;
  /*
   * initial value
   */
  union {
    Expr of_const;              /* (symbolic) constant */
    struct _P_Value *of_induct; /* init value of induction */
  } init;

  /*
   * increment value
   */
  Expr induct_incr;
}
_P_Value, *P_Value;

_P_Value _UNKNOWN_VALUE;

#define UNKNOWN_VALUE                   (&_UNKNOWN_VALUE)
#define Value_is_known(v)               ((v) != UNKNOWN_VALUE)
#define Value_is_unknown(v)             ((v) == UNKNOWN_VALUE)
#define Value_vid(v)                    ((v)->vid)
#define Value_is_induct(v)              (Value_vid(v) && Alpha_var_ids_index_p(Value_vid(v)))
#define Value_is_const(v)               ((Value_vid(v) == NULL) || Alpha_var_ids_const_p(Value_vid(v)))
#define Value_induct_init(v)            ((v)->init.of_induct)
#define Value_induct_incr(v)            ((v)->induct_incr)
#define Value_const(v)                  ((v)->init.of_const)
#define Value_is_integer(v)             (Value_is_const(v) && (Value_const(v)->opcode == OP_int))
#define Value_integer(v)                (Value_const(v)->value.scalar)
#define Value_is_zero(v)                (Value_is_integer(v) && (Value_integer(v) == 0))
#define Value_is_one(v)                 (Value_is_integer(v) && (Value_integer(v) == 1))

/************************************************************************************************/

typedef struct _P_Vnode {
  int id;
  struct _P_Vgraph *vgraph;     /* the Vgraph this node resides */
  struct _P_Scc *scc;           /* the SCC this node belongs to */              /* to get STEP */
  int status;                   /* for finding Scc and other value graph algorithms */
  int num_succ;                 /* number of successors */
  Lptr succ;                    /* list of successor V_Node's */
  struct _P_Vnode *scc_prev;    /* predecessor node in Scc */
  Expr expr;                    /* corresponding Pcode expression */
  PC_Block bb;     		/* the basic block this node belongs to */
  struct _P_Vnode *next;        /* connect all V_Node */
}
_P_Vnode, *P_Vnode;

#define Vnode_id(v)             ((v)->id)
#define Vnode_vgraph(v)		((v)->vgraph)
#define Vnode_scc(v)            ((v)->scc)
#define Vnode_status(v)         ((v)->status)
#define Vnode_num_succ(v)       ((v)->num_succ)
#define Vnode_first_succ(v)    	((v)->succ)
#define Vnode_scc_prev(v)       ((v)->scc_prev)
#define Vnode_expr(v)           ((v)->expr)
#define Vnode_bb(v)             ((v)->bb)
#define Vnode_next(v)           ((v)->next)
#define Vnode_end_of_succ(s)    ((s) == NULL)
#define Vnode_scc_id(v)         (Scc_id(Vnode_scc(v)))
#define Vnode_next_succ(s)      LPTR_next(s)
#define Vnode_succ_vnode(s)     LPTR_ptr(s)
#define Vnode_succ1_vnode(v)    (Vnode_succ_vnode(Vnode_first_succ(v)))
#define Vnode_succ2_vnode(v)    (Vnode_succ_vnode(Vnode_next_succ(Vnode_first_succ(v))))

/************************************************************************************************/

/*
 * acc_name:
 *   all scc's of the same loop and with the same increment are treated as one induction variable
 *   and have the same acc_name. The acc_name will be created using the scc_id.
 *   scc's which are not induction variables, will have their original acc_name as acc_name.
 */

typedef struct _P_Scc {
  int id;
  int flag;			/* general purpose flag */
  int num_comp;                 /* number of components in Scc */
  Lptr comp;                    /* link list of components ptr to Vnode */
  struct _P_Scc *next;          /* next Scc */
  struct _P_Scc *prev;		/* prev Scc */ 
}
_P_Scc, *P_Scc;

#define Scc_id(s)               ((s)->id)
#define Scc_flag(s)		((s)->flag)
#define Scc_num_comp(s)         ((s)->num_comp)
#define Scc_next(s)             ((s)->next)
#define Scc_prev(s)		((s)->prev)
#define Scc_comp(s)       	((s)->comp)
#define Scc_end_of_comp(c)      ((c) == NULL)
#define Scc_next_comp(c)        LPTR_next(c)
#define Scc_comp_vnode(c)       LPTR_ptr(c)

#define NewSccComp(v)           (NewLptr(v))

/************************************************************************************************/

typedef struct _P_Vgraph
{
  struct _PC_Graph *cfg;           /* the CFG this vgraph belongs to */
  struct _P_Vnode *first_vnode; /* first Vnode in this Vgraph */
  struct _P_Vnode *last_vnode;  /* last Vnode in this Vgraph */
  int num_vnode;                /* number of Vnodes in this Vgraph */
  struct _P_Scc *scc_list;      /* the list of Scc in this Vgraph */
  int num_scc;                  /* number of Scc in this Vgraph */
}
_P_Vgraph, *P_Vgraph;

#define Vgraph_cfg(g)			((g)->cfg)
#define Vgraph_first_vnode(g)           ((g)->first_vnode)
#define Vgraph_last_vnode(g)		((g)->last_vnode)
#define Vgraph_num_vnode(g)             ((g)->num_vnode)
#define Vgraph_num_scc(g)		((g)->num_scc)
#define Vgraph_expr2vnode(g)            ((g)->expr2vnode)
#define Vgraph_expr2value(g)		((g)->expr2value)
#define Vgraph_first_scc(g)             ((g)->scc_list)
#define Vgraph_next_scc(s)              (Scc_next(s))
#define Vgraph_end_of_scc(s)            ((s) == NULL)

/************************************************************************************************/

#define INDVAR_FORMAT			"$%d_"

/************************************************************************************************/

extern P_Vgraph P_SS_BuildVgraph (PC_Graph cfg);
extern P_Vgraph Get_CFG_vgraph (PC_Graph cfg);
extern void P_SS_FreeVgraph (P_Vgraph vgraph);
extern void P_SS_FindSccInVgraph (P_Vgraph vgraph);
extern void P_SS_DumpScc (FILE *out_file, P_Scc scc_list, int dump_vnode);
extern void P_SS_FindValuesInCFG (PC_Graph cfg);
extern void P_SS_FreeValue(void * value);
extern PC_Loop P_SS_VnodeOuterMostEnclosingParloop (P_Vnode vnode);
extern PC_Loop P_SS_ExprOuterMostEnclosingParloop (P_Vgraph vgraph, Expr expr);
extern void P_SS_SetExprToVid (Expr expr, Alpha_var_id vid);
extern void P_SS_DumpVgraph (FILE *out_file, P_Vgraph vgraph);
extern void P_SS_DumpValue (FILE *out_file, P_Value value);
extern STRING_Symbol *P_SS_FindStringSymbol(char *name);
extern void P_SS_DeleteVarIdTable();

extern void P_CF_Dump_All_BB (FILE *out_file, PC_Graph cfg, char *title);

#endif /* __PLIB_SS_induct__ */
