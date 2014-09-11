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
 *      File :          l_callgraph.h
 *      Description :   Header file for call graph manipulation in Lcode.
 *      Original : Brian Deitrich, Wen-mei Hwu 1997 (adapted from Roger 
 *                 Bringmann's work)
 *
\*****************************************************************************/

#ifndef _l_callgraph
#define _l_callgraph

/* 10/29/02 REK Adding config.h */
#include <config.h>
#include <Lcode/l_code.h>

/* Data structures to enable a depth-first search of the callgraph */
enum L_DFS_color
{ WHITE, GRAY, BLACK };
typedef struct L_DFS_Info
{
  enum L_DFS_color color;
  int start;
  int end;
  struct L_CG_Node *prev_node;
  enum L_DFS_color bw_color;
}
L_DFS_Info;

/* types of jsrs -- NORMAL_JSR =         jsr has a label operand.
 *                  RESOLVED_IND_JSR =   jsr has a register operand.  CALLNAME
 *                                       attribute is present and shows only
 *                                       one target.
 *                  UNRESOLVED_IND_JSR = jsr has a register operand.  CALLNAME
 *                                       attribute shows multiple targets.
 * If no CALLNAME attribute is present, no arcs are built.  Arcs going to all
 * nodes are assumed in this case. */
#define UNDEFINED_JSR     -1
#define NORMAL_JSR         0
#define RESOLVED_IND_JSR   1
#define UNRESOLVED_IND_JSR 2


#define ARC_BACKEDGE       0x0001

typedef struct L_CG_Arc
{
  int id;                       /* unique id */
  unsigned int flags;           /* bit field available for 
                                   specific applications */
  int level;                    /* For staging analysis */
  int jsr_id;                   /* id of src node jsr call */
  int type;                     /* one of the types of jsr described
                                 * above. */
  double weight;                /* frequency of src node jsr call */

  struct L_CG_Node *src_node;   /* caller node */
  struct L_CG_Arc *prev_dst_arc;        /* used to access other arcs which
                                         * exit the src node */
  struct L_CG_Arc *next_dst_arc;

  struct L_CG_Node *dst_node;   /* callee node */
  struct L_CG_Arc *prev_src_arc;        /* used to access other arcs which
                                         * call the dst node */
  struct L_CG_Arc *next_src_arc;

  void *ext;                    /* generic pointer for specific
                                 * applications */
}
L_CG_Arc;

typedef struct L_CG_Node
{
  int id;                       /* unique id  */
  int flags;                    /* Flags */
  char *func_name;              /* function name */
  char *filename;               /* name of file that function is in */
  int func_file_num;            /* the ordered number of the function
                                 * in the file.  used when multiple
                                 * functions are contained in the same
                                 * file.  it allows the correct
                                 * function to be obtained in the case
                                 * when multiple functions are found 
                                 * in the same file. */
  L_Func *fn;                   /* place for the function pointer to
                                 * be located if the function is in
                                 * memory. */

  L_CG_Arc *first_src_arc;      /* arc to first node calling this
                                 * node */
  L_CG_Arc *last_src_arc;       /* arc to last node calling this
                                 * node */

  L_CG_Arc *first_dst_arc;      /* arc to first node called by this
                                 * node */
  L_CG_Arc *last_dst_arc;       /* arc to last node called by this
                                 * node */

  struct L_CG_Node *prev_node;  /* used for node traversal */
  struct L_CG_Node *next_node;

  L_DFS_Info *dfs;              /* used to help traverse callgraph via
                                 * DFS search */
  void *ext;                    /* generic pointer for specific
                                 * applications */
}
L_CG_Node;

typedef struct L_CallGraph
{
  L_CG_Node *first_node;        /* first node in node list */
  L_CG_Node *last_node;         /* last node in node list */
}
L_CallGraph;

extern L_CG_Arc *L_CG_copy_arc (L_CG_Arc *);
extern void L_CG_delete_only_arc (L_CG_Arc *);
extern void L_CG_delete_arc (L_CG_Arc *);
extern void L_CG_delete_all_dst_arcs (L_CG_Arc *);
extern void L_CG_delete_all_src_arcs (L_CG_Arc *);
extern void L_CG_remove_arc_from_src_and_dst (L_CG_Arc *);
extern void L_CG_add_arc (L_CG_Node *, L_Oper *, L_Operand *, L_CallGraph *);
extern void L_CG_print_arc (L_CG_Arc *);
extern L_CG_Node *L_CG_new_node (char *);
extern void L_CG_add_node (L_CG_Node *, L_CallGraph *);
extern void L_CG_delete_node (L_CG_Node * node);
extern void L_CG_delete_node_not_func (L_CG_Node * node);
extern L_CG_Node *L_CG_find_node (char *, L_CallGraph *);
extern void L_CG_print_node (L_CG_Node *);
extern L_Func *L_read_node_func (L_CG_Node *);
extern void L_delete_node_func (L_CG_Node *);
extern L_CallGraph *L_CG_new_callgraph (void);
extern void L_CG_callgraph_delete (L_CallGraph *);
extern void L_CG_callgraph_delete_not_func (L_CallGraph * callgraph);
extern L_CallGraph *L_CG_callgraph_build (List filelist,
					 void (*user_init_func) (L_CG_Node *));
extern void L_CG_print_callgraph (L_CallGraph *);
extern void L_CG_print_davinci_callgraph (L_CallGraph *);

#endif
