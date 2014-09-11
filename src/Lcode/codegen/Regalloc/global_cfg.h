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
/*===========================================================================*\
 *
 *  File:  global_cfg.h
 *
 *  Description:
 *    Global program control flow graph data structure definitions
 *
 *  Creation Date :  June, 1994.
 *
 *  Author:  Richard E. Hank, Wen-mei Hwu
 *
 *
 *
 *===========================================================================*/
/*===========================================================================*/
/* 10/29/02 REK Adding config.h */
#include <config.h>
#include <Lcode/l_main.h>
#include <Lcode/l_interproc.h>

#ifndef GLOBAL_CFG_H
#define GLOBAL_CFG_H

typedef struct _Func
{
  int id;
  char *funcName;
  char *fileName;
  unsigned short numNodes;
  unsigned short maxNodeId;
  short maxLoopId;
  L_Loop *loops;
  Set loopHeaders;
  Set loopBackEdges;
  HashTable nodeHashTbl;
  double weight;
  struct _Node *cfg;
  struct _Func *nextFunction;
}
Func;

/*
 * Graph Function Access Macros
 */
#define _FuncId(x)		(x)->id
#define _FuncName(x)		(x)->funcName
#define _FuncFileName(x)	(x)->fileName
#define _FuncWeight(x)		(x)->weight
#define _FuncCFG(x)		(x)->cfg
#define _FuncNext(x)		(x)->nextFunction
#define _FuncLoops(x)		(x)->loops
#define _FuncMaxLoopId(x)	(x)->maxLoopId
#define _FuncNodeHashTbl(x)	(x)->nodeHashTbl
#define _FuncNumNodes(x)	(x)->numNodes
#define _FuncMaxNodeId(x)	(x)->maxNodeId
#define _FuncLoopHeaders(x)	(x)->loopHeaders
#define _FuncLoopBackEdges(x)	(x)->loopBackEdges

typedef struct _Node
{
  union
  {
    struct
    {
      unsigned short fnId;
      unsigned short cbId;
    }
    id;
    int nodeId;
  }
  id;
  unsigned short flags;
  unsigned short operCount;
  int srcFnId;
  int region;
  Set dominators;
  struct _Arc *incomingList;
  struct _Arc *targetList;
  struct _Node *nextNode;
  double weight;
}
Node;

/*
 * Graph Node Flags
 */
#define	NODE_PROLOGUE		0x0001
#define NODE_EPILOGUE		0x0002

#define NODE_NON_ENTRY		0x2000
#define NODE_NON_EXIT		0x4000
#define NODE_VISITED		0x8000

/*
 * Graph Node Access Macros
 */
#define _NodeFunction(x)	(x)->id.id.fnId
#define _NodeCb(x)		(x)->id.id.cbId
#define _NodeId(x)		(x)->id.nodeId
#define _NodeSrcFnId(x)		(x)->srcFnId

#define _NodeRegion(x)		(x)->region
#define _NodeWeight(x)		(x)->weight
#define _NodeFlags(x)		(x)->flags
#define _NodeNext(x)		(x)->nextNode
#define _NodeIncoming(x)	(x)->incomingList
#define _NodeBranchTargets(x)   (x)->targetList
#define _NodeDominators(x)	(x)->dominators
#define _NodeOperCount(x)	(x)->operCount

typedef struct _Arc
{
  int flags;
  int oper_id;
  double weight;
  struct _Node *src;
  struct _Node *dst;
  struct _Arc *srcNext;
  struct _Arc *dstNext;
}
Arc;

/*
 * Graph Arc Access Macros
 */
#define _ArcFlags(y)		(y)->flags
#define _ArcOperId(y)		(y)->oper_id
#define _ArcWeight(y)		(y)->weight
#define _ArcSrcNode(y)		(y)->src
#define _ArcSrcNext(y)		(y)->srcNext
#define _ArcDstNode(y)		(y)->dst
#define _ArcDstNext(y)		(y)->dstNext

/*
 * Arc Node Flags
 */
#define	ARC_BRANCH		0x0001
#define ARC_FALLTHRU		0x0002
#define ARC_JSR			0x0004
#define ARC_REGISTER_BRANCH	0x0008

/*
 * Graph Memory Management
 */

#ifdef __cplusplus
extern "C"
{
#endif

  extern L_Alloc_Pool *GraphFuncPool;
  extern L_Alloc_Pool *GraphNodePool;
  extern L_Alloc_Pool *GraphArcPool;

#ifdef __cplusplus
}
#endif


#define _newFunc()  	((Func *)L_alloc(GraphFuncPool))
#define	_newNode()	((Node *)L_alloc(GraphNodePool))
#define _newArc()	((Arc *)L_alloc(GraphArcPool))

#define _freeFunction(x)	L_free(GraphFuncPool,x)
#define _freeNode(x)		L_free(GraphNodePool,x)
#define _freeArc(x)		L_free(GraphArcPool,x)

#ifdef __cplusplus
extern "C"
{
#endif

  extern int *nodeBuf;
  extern int maxNodesPerFunc;

/*
 * Hash Tables
 */
  extern HashTable fnHashTbl;
  extern HashTable prologueHashTbl;

/*
 * Parameters
 */
  extern char *regionInputFormat;	/* Values: "file_list", "cfg" */
  extern char *regionOutputFormat;	/* Values: "cfg" or "pretty_cfg" */


/*
 *  Function Prototypes
 */
  extern Node *newNode (int cb_id, double weight);
  extern Node *sortNodes (Node * nodeList);
  extern Func *sortFunctions (Func * funcList);
  extern void addBranchTarget (int funcId, Node * node, L_Flow * flow,
			       int id);
  extern void addRegisterBranchTargets (int funcId, Node * node,
					L_Flow * first, L_Flow * last,
					int id);
  extern void addJsrTarget (int funcId, Node * node, L_Oper * oper,
			    CallGraph * callgraph);
  extern void initCFG (void);
  extern Node *buildFunctionCFG (L_Func * fn, int funcId,
				 CallGraph * callgraph);
  extern Func *buildGlobalCFG (FILE * fileList, CallGraph * callgraph);
  extern Arc *reverseTargets (Arc * arcList);
  extern void outputGlobalCFG (FILE * fp, Func * funcList);
  extern void outputPrettyGlobalCFG (FILE * fp, Func * funcList);
  extern Func *parseFunctionMap (FILE * fp);
  extern void parseFunction (FILE * fp);
  extern Func *parseCFGFile (FILE * fp);

#ifdef __cplusplus
}
#endif

#endif
