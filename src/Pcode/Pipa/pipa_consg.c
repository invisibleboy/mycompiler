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
 *
 *      File:    pipa_consg.c
 *      Author:  Erik Nystrom
 *      Copyright (c) 2003  Erik Nystrom, Wen-mei Hwu
 *              and The Board of Trustees of the University of Illinois.
 *              All rights reserved.
 *      The University of Illinois software License Agreement
 *      specifies the terms and conditions for redistribution.
\*****************************************************************************/

#include "pipa_consg.h"
#include "pipa_graph.h"
#include "pipa_options.h"
#include "pipa_print_graph.h"

void
check_node(IPA_cgraph_node_t *node)
{
  IPA_cgraph_t *cgraph;
  IPA_cgraph_node_t *tmp_node;
  int fail = 0;

  if (!node)
    return;

  assert(node->data.var_id > 0);
  assert(node->data.version > 0);
  assert(node->data.version < 100000);
  assert(node->data.var_size > 0);
#if 0
  assert(node->data.var_size < 1024 * 1024);
#endif
  assert(node->data.in_k_cycle < 512 * 1024);
  assert(node->data.mod > 0 && node->data.mod <= node->data.var_size);
  assert(node->data.syminfo != NULL);
#if 0
#if LP64_ARCHITECTURE
  assert((long)node->data.syminfo > 100000);
#else
  assert((int)node->data.syminfo > 100000);
#endif
#endif

  if (IPA_FLAG_ISSET (node->flags, IPA_CG_NODE_FLAGS_HEAP))
    assert(node->data.var_size == IPA_max_type_size &&
	   IPA_FLAG_ISSET (node->flags, IPA_CG_NODE_FLAGS_NOLOCAL));
  if (IPA_FLAG_ISSET (node->flags, IPA_CG_NODE_FLAGS_GLOBAL))
    assert(IPA_FLAG_ISSET (node->flags, IPA_CG_NODE_FLAGS_NOCNTXT) &&
	   IPA_FLAG_ISSET (node->flags, IPA_CG_NODE_FLAGS_NOLOCAL));
  
  if (IPA_FLAG_ISSET (node->flags, (IPA_CG_NODE_FLAGS_HEAP|IPA_CG_NODE_FLAGS_STACK)))
    assert(node->data.var_size == IPA_max_type_size);

  if (node->rep_parent != node)
    {
      IPA_cgraph_edge_list_t *elist;

      for (elist=node->first_list; elist; elist=elist->nxt_list)
	{
	  if (elist->in && IPA_htab_size(elist->in) != 0)
	    fail = 3;
	  if (elist->out && IPA_htab_size(elist->out) != 0)
	    fail = 4;
	}
    }

#if 0
#if UNIFY
  if (IPA_gcon_option == IPA_GCON_ANDERSEN)
#endif
    {
      cgraph = node->cgraph;
      for (tmp_node = node; tmp_node; 
	   tmp_node = tmp_node->rep_child)
	{
	  assert(tmp_node->cgraph == cgraph);
	}
    }
#endif

  if (fail)
    {
      IPA_cg_node_print(stdout, node, IPA_PRINT_ASCI);
      assert(0);
    }
}

/*****************************************************************************
 * Find node
 *****************************************************************************/
IPA_cgraph_node_t *
IPA_consg_find_node (IPA_cgraph_t * cgraph,
		     int var_id, int version)
{
  IPA_cgraph_node_data_t ndata;
  IPA_cgraph_node_t *node;

  ndata.var_id = var_id;
  ndata.version = version;
  node = IPA_cg_node_find (cgraph, &ndata);

  DEBUG(check_node(node););

  return node;
}


/*****************************************************************************
 * Make a new node with var_id and offset.
 *****************************************************************************/

#define DEBUG_NEWNODE 0

IPA_cgraph_node_t *
IPA_consg_ensure_node_d (IPA_cgraph_t * cgraph,
                         IPA_cgraph_node_data_t * ndata,
			 int flags)
{
  IPA_cgraph_node_t *node;

  node = IPA_cg_node_add (cgraph, ndata);

  if (CG_NODE_ISNEW(node))
    {
      IPA_FLAG_SET(node->flags, flags);
      IPA_FLAG_CLR(node->flags, IPA_CG_NODE_FLAGS_GENERIC);
      node->sample_delay2 = 102;
#if DEBUG_NEWNODE
      printf ("New Node {%d,%d,%d} \n", 
	      node->data.var_id, 
	      node->data.version,
	      node->data.var_size);
#endif
    }
  else
    {
#if 0
      /* The MUSTMATCH bits in the node should subsume those being
	 requested */
      assert( ((~(node->flags & IPA_CG_NODE_FLAGS_MUSTMATCH)) & 
	       (flags & IPA_CG_NODE_FLAGS_MUSTMATCH)) == 0 );
#endif
    }

  DEBUG(check_node(node););
  return node;
}
 

/* WARNING!!! - THIS ROUTINE SHOULD ONLY BE USED TO CREATE A NEW
   NODE NOT TO COPY AN EXISTING NODE - USE IPA_consg_ensure_node_d FOR
   NODE COPYING.
*/
IPA_cgraph_node_t *
IPA_consg_ensure_node (IPA_cgraph_t * cgraph, 
		       int var_id, int version, int var_size,
		       IPA_symbol_info_t *syminfo,
		       int flags)
{
  IPA_cgraph_node_data_t ndata;
  IPA_cgraph_node_t *node;

#if 0 /* EMN_TEST */
  if (IPA_FLAG_ISSET (flags, IPA_CG_NODE_FLAGS_HEAP) ||
      IPA_FLAG_ISSET (flags, IPA_CG_NODE_FLAGS_STACK))
    {
      var_size = IPA_max_type_size;
    }
#endif

  ndata.var_id = var_id;
  ndata.version = version;
  ndata.var_size = var_size;
  ndata.syminfo = syminfo;
  ndata.mod = var_size;
  ndata.in_k_cycle = 0;
  node = IPA_cg_node_add (cgraph, &ndata);

  if (CG_NODE_ISNEW(node))
    {
      /* node->data.var_size = var_size;
	 node->data.syminfo = syminfo; */
      IPA_FLAG_SET(node->flags, flags);
      IPA_FLAG_CLR(node->flags, IPA_CG_NODE_FLAGS_GENERIC);
      node->sample_delay2 = 102;

#if DEBUG_NEWNODE
      printf ("New Node {%d,%d,%d} \n", 
	      node->data.var_id, 
	      node->data.version,
	      node->data.var_size);
#endif
    }
  else
    {
      /* The MUSTMATCH bits in the node should subsume those being
	 requested */
      assert( ((~(node->flags & IPA_CG_NODE_FLAGS_MUSTMATCH)) & 
	       (flags & IPA_CG_NODE_FLAGS_MUSTMATCH)) == 0 );
    }

  DEBUG(check_node(node););
  return node;
}

/*****************************************************************************
 * Create a new version of a node
 *****************************************************************************/
#define DEBUG_VERSION 0

IPA_cgraph_node_t *
IPA_consg_node_new_version (IPA_prog_info_t   *info,
                            IPA_cgraph_node_t *node, 
                            IPA_cgraph_t      *dst_cg,
			    IPA_cgraph_t      *orig_cg,
			    HashTable          ver_htab)
{
  IPA_cgraph_node_data_t new_data;
  IPA_cgraph_node_t *new_node;

  new_data = node->data;
  
  /* In general, versioning will only be done if a versioning table is provided
   * Do not version globals, params, returns, or nocntxt vars 
   */
  if ( ver_htab && 
       IPA_FLAG_ISCLR (node->flags, (IPA_CG_NODE_FLAGS_GLOBAL |
				     IPA_CG_NODE_FLAGS_NOCNTXT |
				     IPA_CG_NODE_FLAGS_PARAM |
				     IPA_CG_NODE_FLAGS_RETURN)) )
    {
      int new_version;
      int key;

      /* Has this var already been mapped to a particular version *
       * This _needs_ to take into account the previous version 
       *   since there may be many versions of the same var in a func
       */
      assert(new_data.version < 16000);
      assert(new_data.var_id < 255000);
      key = ((new_data.version << 18) | new_data.var_id);
      new_version = (int) (long) HashTable_find_or_null (ver_htab, key);

      if (!new_version)
	{
	  /* Never versioned this particular var,version before. 
	     Find the next version number */
	  assert(node->data.syminfo->max_version > 0);
	  node->data.syminfo->max_version ++;

	  new_version = node->data.syminfo->max_version;
	  HashTable_insert (ver_htab, key, (void *) (long) new_version);
#if DEBUG_VERSION
	  if (IPA_FLAG_ISSET(node->flags, IPA_CG_NODE_FLAGS_HEAP))
	    {
	      printf("Creating %d: %d -> version %d of %s :: ",
		     new_data.var_id, new_data.version, 
		     new_version, node->data.syminfo->symbol_name);
	      if (IPA_FLAG_ISSET(node->flags, IPA_CG_NODE_FLAGS_HEAP))
		printf("HEAP");
	      printf("\n");
	    }
#endif
	}
#if DEBUG_VERSION
      else
	{
	  printf("Reusing version %d: %d -> %d of %s :: ",
		 new_data.var_id, new_data.version, new_version,
		 node->data.syminfo->symbol_name);
	  if (IPA_FLAG_ISSET(node->flags, IPA_CG_NODE_FLAGS_HEAP))
	    printf("HEAP");
	  printf("\n");
	}
#endif

      /* Create new version of requested node */
      new_data.version = new_version;
      new_node = IPA_consg_ensure_node_d (dst_cg, &new_data, node->flags);


      /* HEAP/ESCLOCALS need the addition of special up/down edges */
      if (CG_NODE_ISNEW(new_node) && 
	  (IPA_FLAG_ISSET(node->flags, IPA_CG_NODE_FLAGS_HEAP) ||
	   IPA_FLAG_ISSET(node->flags, IPA_CG_NODE_FLAGS_ESCLOCAL))
#if HS_CI
          && (IPA_context_option != IPA_CONTEXT_INSENSITIVE)
#endif
	  )
	{
	  IPA_cgraph_node_t *orig_node;
	  IPA_cgraph_node_t *cur_node;

	  /* It is possible for HEAP/ESCLOCAL to be merged inside of
	   *  the summaries (this reduces the explosion of HEAP cloning)
	   */
	  for (cur_node=node; cur_node; cur_node = cur_node->rep_child)
	    {
	      /* Get the original, non-summary consg node to which
		 the new version will be connected */
	      orig_node = IPA_consg_find_node(orig_cg, 
					      cur_node->data.var_id, 
					      cur_node->data.version);
	      assert(orig_node);

	      /* There is an off chance that this orig_node has been merged
	       */
	      orig_node = IPA_cg_node_get_rep(orig_node);
	      
	      /* Add the up/dn edges */
	      IPA_consg_ensure_edge (ASSIGN, orig_node, new_node,
				     0, IPA_max_type_size, 0,
				     (IPA_CG_EDGE_FLAGS_UP |
				      IPA_CG_EDGE_FLAGS_EXPLICIT));
#if 0	      
#if UNIFY
	      /* With UNIFY on, you can only do 1 generation without error */
	      assert(IPA_gcon_option == IPA_GCON_ANDERSEN ||
		     IPA_cloning_option == IPA_NO_HEAP_CLONING || 
		     IPA_cloning_gen == 0);
#endif
#endif
		IPA_consg_ensure_edge (ASSIGN, new_node, orig_node,
				       0, IPA_max_type_size, 0,
				       (IPA_CG_EDGE_FLAGS_DN |
					IPA_CG_EDGE_FLAGS_EXPLICIT));
	      
	    }
	  
	  /* Track the generations and allow early termination of
	   *   versioning path for heap variables
	   * (need to re-find orig_node based on node)
	   */
	  orig_node = IPA_consg_find_node(orig_cg, 
					  node->data.var_id, 
					  node->data.version);
	  new_node->generation = orig_node->generation + 1;
	  new_node->from_version = orig_node->data.version;
#if 0
	  printf("VERSION: %d  [%s] %d <- [%s] %d [%p]\n",
		 new_node->data.var_id, 
		 dst_cg->data.fninfo->func_name,
		 new_node->data.version,
		 orig_cg->data.fninfo->func_name,
		 orig_node->data.version,
		 new_node);
#endif
	  assert(new_node->data.version > orig_node->data.version);
	  assert(orig_node->from_version < new_node->from_version);

	  if (IPA_FLAG_ISSET(node->flags, IPA_CG_NODE_FLAGS_HEAP))
	    {
	      if (new_node->generation > IPA_cloning_gen)
		{
		  IPA_FLAG_SET(new_node->flags, IPA_CG_NODE_FLAGS_NOCNTXT);
#if 0
		  printf("LAST HEAP GEN %d\n",new_node->generation);
#endif
		}
#if 0
	      else
		{
		  printf("HEAP GEN %d\n",new_node->generation);
		}
#endif
	    }
#if 0
	  else
	    {
	      printf("ESCLOCAL GEN %d\n",new_node->generation);
	    }
#endif
	}
    }
  else 
    {
      /* Just find/create without versioning */
      new_node = IPA_consg_ensure_node_d (dst_cg, &node->data, node->flags);

    }

  return new_node;
}

/*****************************************************************************
 * Manage the edge qualifiers
 *****************************************************************************/

typedef enum ceo_dtype {HZ=0, DN=1, UP=2, GB=3, MAXD=4, NA=-1, DNE=-2} ceo_dtype;
typedef enum ceo_etype {DA=0, AD=1, AS=2, MAXE=3} ceo_etype;
typedef enum ceo_vtype {SP=0, NS=1, MAXV=2} ceo_vtype;

static ceo_dtype qualtab[MAXV][MAXE][MAXD][MAXD] = 
/*                    u = &w */
  { /* Spec */
    {
      { /* u *= v */
	/*        HZ  DN  UP  GB  */   
	/* HZ */ {HZ, UP, DNE,GB},
	/* DN */ {DNE,DNE,DNE,DNE},
	/* UP */ {DNE,DNE,DNE,DNE},
	/* GB */ {DNE,DNE,DNE,DNE}
      },
      { /* v = *u */
	/*        HZ  DN  UP  GB  */ 
	/* HZ */ {HZ, DN, DNE,GB},
	/* DN */ {DNE,DNE,DNE,DNE},
	/* UP */ {DNE,DNE,DNE,DNE},
	/* GB */ {DNE,DNE,DNE,DNE}
      },
      { /* v = u */
	/*        HZ  DN  UP  GB  */ 
	/* HZ */ {HZ, DN, DNE,GB},
	/* DN */ {DN, DN, DNE,DN},
	/* UP */ {NA, NA, DNE,GB},
	/* GB */ {GB, GB, DNE,GB}
      }
    },
    /* No-spec */
    { 
      { /* u *= v */
	/*        HZ  DN  UP  GB  */   
	/* HZ */ {GB, GB, DNE,GB},
	/* DN */ {DNE,DNE,DNE,DNE},
	/* UP */ {DNE,DNE,DNE,DNE},
	/* GB */ {DNE,DNE,DNE,DNE}
      },
      { /* v = *u */
	/*        HZ  DN  UP  GB  */ 
	/* HZ */ {GB, GB, DNE,GB},
	/* DN */ {DNE,DNE,DNE,DNE},
	/* UP */ {DNE,DNE,DNE,DNE},
	/* GB */ {DNE,DNE,DNE,DNE}
      },
      { /* v = u */
	/*        HZ  DN  UP  GB  */ 
	/* HZ */ {GB, GB, DNE,GB},
	/* DN */ {DN, DN, DNE,DN},
	/* UP */ {GB, GB, DNE,GB},
	/* GB */ {GB, GB, DNE,GB}
      }
    }
  };

static int
lookup_qualtab(ceo_vtype vt, ceo_etype e2_e, ceo_dtype e2_d, ceo_dtype e1_d)
{
  int edgeflag;
  ceo_dtype new_d;
  
  new_d = qualtab[vt][e2_e][e2_d][e1_d];
  switch (new_d)
    {
    case HZ: edgeflag = IPA_CG_EDGE_FLAGS_HZ; break;
    case DN: edgeflag = IPA_CG_EDGE_FLAGS_DN; break;
    case UP: edgeflag = IPA_CG_EDGE_FLAGS_UP; break;
    case GB: edgeflag = IPA_CG_EDGE_FLAGS_HZ | IPA_CG_EDGE_FLAGS_GBL; break;
    case NA: edgeflag = 0; break;
    case DNE:
      assert(0);
    default:
      assert(0);
    }

  return edgeflag;
}

#define DEBUG_CEO 0

int
IPA_consg_calc_edge_origin (IPA_cgraph_edge_t * edge1,
			    IPA_cgraph_edge_t * edge2)
{
  ceo_dtype e1_d;
  ceo_etype e2_e;
  ceo_vtype vt;
  int new_edgeflag = 0;

#if DEBUG_CEO
  printf("%10.10s.%2d %10.10s.%2d %10.10s.%2d %10.10s.%2d :: ",
	 edge1->src_elist->node->data.syminfo->symbol_name,
	 edge1->src_elist->node->data.version,
	 edge1->dst_elist->node->data.syminfo->symbol_name,
	 edge1->dst_elist->node->data.version,
	 edge2->src_elist->node->data.syminfo->symbol_name,
	 edge2->src_elist->node->data.version,
	 edge2->dst_elist->node->data.syminfo->symbol_name,
	 edge2->dst_elist->node->data.version);

  printf("AA %8s + ", 
	 IPA_cg_edge_flag_name(edge1));
  printf("%9.9s ", 
	 edge_types[edge2->src_elist->edge_type]);

  if (IPA_FLAG_ISSET(edge2->flags, IPA_CG_EDGE_FLAGS_GBL))
    printf(" GBL ");
  if (IPA_FLAG_ISSET(edge2->flags, IPA_CG_EDGE_FLAGS_HZ))
    printf(" HZ  ");
  if (IPA_FLAG_ISSET(edge2->flags, IPA_CG_EDGE_FLAGS_DN))
    printf(" DN  ");
  if (IPA_FLAG_ISSET(edge2->flags, IPA_CG_EDGE_FLAGS_UP))
    printf(" UP  ");
  printf(" => ");
#endif

  /* edge 1 is always an addr edge */
  assert(edge1->src_elist->edge_type == ASSIGN_ADDR);


  /* Get central variable info */
  if (IPA_FLAG_ISSET(edge1->dst_elist->node->flags, IPA_CG_NODE_FLAGS_NOCNTXT))
    vt = NS;
  else
    vt = SP;


  /* What kind of edge is the non-address edge */
  switch (edge2->src_elist->edge_type)
    {
    case ASSIGN      : e2_e = AS; break;
    case SKEW        : e2_e = AS; break;
    case ASSIGN_DEREF: e2_e = AD; break;
    case DEREF_ASSIGN: e2_e = DA; break;
    default:
       assert(0);
   }

  
  /* What qualifier is on the addr edge */
  if (IPA_FLAG_ISSET(edge1->flags, IPA_CG_EDGE_FLAGS_GBL))
    e1_d = GB;
  else if (IPA_FLAG_ISSET(edge1->flags, IPA_CG_EDGE_FLAGS_HZ))
    e1_d = HZ;
  else if (IPA_FLAG_ISSET(edge1->flags, IPA_CG_EDGE_FLAGS_DN))
    e1_d = DN;
  else
    assert(0);


  /* What qualifier is on the non-addr edge 
   *   This goes ahead and computes the new edge type
   *   It also handles the case where the edge is both UP and DN
   */
  if (IPA_FLAG_ISSET(edge2->flags, IPA_CG_EDGE_FLAGS_GBL))
    {
      new_edgeflag = lookup_qualtab(vt,e2_e,GB,e1_d);
    }
  else if (IPA_FLAG_ISSET(edge2->flags, IPA_CG_EDGE_FLAGS_HZ))
    {
      new_edgeflag = lookup_qualtab(vt,e2_e,HZ,e1_d);
    }
  else if (IPA_FLAG_ISSET(edge2->flags, IPA_CG_EDGE_FLAGS_DN) && 
	   IPA_FLAG_ISSET(edge2->flags, IPA_CG_EDGE_FLAGS_UP))
    {
      int new_edgeflag1 = lookup_qualtab(vt,e2_e,DN,e1_d);
      int new_edgeflag2 = lookup_qualtab(vt,e2_e,UP,e1_d);

      /* Pick result from UP edge unless it is a DNE case */
      if (new_edgeflag2 == 0)
	new_edgeflag = new_edgeflag1;
      else
	new_edgeflag = new_edgeflag2;	
    }
  else if (IPA_FLAG_ISSET(edge2->flags, IPA_CG_EDGE_FLAGS_DN))
    {
      new_edgeflag = lookup_qualtab(vt,e2_e,DN,e1_d);
    }
  else if (IPA_FLAG_ISSET(edge2->flags, IPA_CG_EDGE_FLAGS_UP))
    {
      new_edgeflag = lookup_qualtab(vt,e2_e,UP,e1_d);
    }
  else
    assert(0);


#if DEBUG_CEO
  if (IPA_FLAG_ISSET(new_edgeflag, IPA_CG_EDGE_FLAGS_GBL))
    printf(" GBL ");
  if (IPA_FLAG_ISSET(new_edgeflag, IPA_CG_EDGE_FLAGS_HZ))
    printf(" HZ ");
  if (IPA_FLAG_ISSET(new_edgeflag, IPA_CG_EDGE_FLAGS_DN))
    printf(" DN ");
  if (IPA_FLAG_ISSET(new_edgeflag, IPA_CG_EDGE_FLAGS_UP))
    printf(" UP ");
  if (new_edgeflag == 0)
    printf(" N/A ");
  printf("\n");
#endif


  /* No edge generated?
   */
  if (new_edgeflag == 0)
    return 0;

  /* Otherwise, this is a derived edge */
  IPA_FLAG_SET(new_edgeflag, IPA_CG_EDGE_FLAGS_IMPLICIT);

  return new_edgeflag;
}


/*****************************************************************************
 * Make a new constraint edge with src, tgt, type.
 *****************************************************************************/

void
check_edge(IPA_cgraph_edge_t *edge)
{
  int fcnt;
  IPA_cgraph_edgelist_e edge_type;

  assert(edge->src_elist->edge_type == 
	 edge->dst_elist->edge_type);
  edge_type = edge->src_elist->edge_type;

  if (edge_type == ASSIGN_DEREF || edge_type == DEREF_ASSIGN)
    {
      IPA_cgraph_node_t *src_node = edge->src_elist->node;
      IPA_cgraph_node_t *dst_node = edge->dst_elist->node;
      
      assert(IPA_FLAG_ISSET(edge->flags, IPA_CG_EDGE_FLAGS_HZ));
      if (IPA_FLAG_ISCLR(src_node->flags, IPA_CG_NODE_FLAGS_SUMMARY))
	{
	  assert(IPA_FLAG_ISCLR(src_node->flags, IPA_CG_NODE_FLAGS_RETURN));
	  assert(IPA_FLAG_ISCLR(dst_node->flags, IPA_CG_NODE_FLAGS_RETURN));
	  assert(IPA_FLAG_ISCLR(src_node->flags, IPA_CG_NODE_FLAGS_PARAM));
	  assert(IPA_FLAG_ISCLR(dst_node->flags, IPA_CG_NODE_FLAGS_PARAM));
	}
    }

  if (IPA_field_option == IPA_FIELD_INDEPENDENT ||
      IPA_FLAG_ISSET (edge->src_elist->node->flags, IPA_CG_NODE_FLAGS_NOFIELD))
    assert(edge->data.source_offset == 0);
  if (IPA_field_option == IPA_FIELD_INDEPENDENT ||
      IPA_FLAG_ISSET (edge->dst_elist->node->flags, IPA_CG_NODE_FLAGS_NOFIELD))
    assert(edge->data.target_offset == 0);
  
  assert(edge->data.source_stride <= 1);
  if (edge->data.source_stride == 1)
    assert(edge->data.source_offset == 0);
  assert(edge->data.target_stride <= 1);
  if (edge->data.target_stride == 1)
    assert(edge->data.target_offset == 0);

  fcnt = 0;
  if (IPA_FLAG_ISSET(edge->flags, IPA_CG_EDGE_FLAGS_HZ))
    fcnt++;
  if (IPA_FLAG_ISSET(edge->flags, IPA_CG_EDGE_FLAGS_DN))
    fcnt++;
  if (IPA_FLAG_ISSET(edge->flags, IPA_CG_EDGE_FLAGS_UP))
    fcnt++;
  assert(fcnt > 0);

  fcnt = 0;
  if (IPA_FLAG_ISSET(edge->flags, IPA_CG_EDGE_FLAGS_EXPLICIT))
    fcnt++;
  if (IPA_FLAG_ISSET(edge->flags, IPA_CG_EDGE_FLAGS_IMPLICIT))
    fcnt++;
  assert(fcnt == 1);
}

static int
skip_ensure_edge(IPA_cgraph_node_t * src_node,
		 IPA_cgraph_node_t * dst_node,
		 IPA_cgraph_edgelist_e edge_type,
		 IPA_cgraph_edge_data_t * edata)
{
  /* Redundant loop */
  if ((edge_type == ASSIGN) &&
      (src_node == dst_node) && 
      (edata->target_offset == edata->source_offset))
    return 1;

  /* Function ptr can't be a target or a non-assign_addr src */
  if (IPA_FLAG_ISSET(dst_node->flags, IPA_CG_NODE_FLAGS_FUNC))
    return 1;
  if (IPA_FLAG_ISSET(src_node->flags, IPA_CG_NODE_FLAGS_FUNC) &&
      (edge_type != ASSIGN_ADDR))
    {
      /* printf("SKIP FN\n"); */
      return 1;
    }

  /* String ptr can't be a target or a non-assign_addr src */
  if (IPA_FLAG_ISSET(dst_node->data.syminfo->kind, IPA_VAR_KIND_STRING))
    return 1;
  if (IPA_FLAG_ISSET(src_node->data.syminfo->kind, IPA_VAR_KIND_STRING) &&
      (edge_type != ASSIGN_ADDR))
    {
      /* printf("SKIP STR\n"); */
      return 1;
    }

  /* Assignments less than pointer size pointless */
  if (edge_type == ASSIGN && edata->assign_size < IPA_POINTER_SIZE)
    return 1;

  return 0;
}

extern int apply_rn_cnt;
extern int apply_f_rn_cnt;
void
IPA_consg_apply_edge_flags(IPA_cgraph_edge_t *edge,
			   int new_flags)
{
  int renew = 0;

  /* Priority: GBL -> HZ -> UP/DN
   */
  if (!IPA_FLAG_ISSET(edge->flags, IPA_CG_EDGE_FLAGS_GBL))
    {
      if (!IPA_FLAG_ISSET(edge->flags, IPA_CG_EDGE_FLAGS_HZ))
	{
	  /* UP/DN 
	   * - anything new (HZ/GBL/U/D) forces a renew
	   */
	  if ( ((~edge->flags) & new_flags & (IPA_CG_EDGE_FLAGS_HZ|
					      IPA_CG_EDGE_FLAGS_GBL|
					      IPA_CG_EDGE_FLAGS_UD)) != 0)
	    {
	      renew = 1;
	    }
	  else if ( ((~edge->flags) & new_flags) )
	    {
	      apply_f_rn_cnt++;
	    }
	}
      else
	{
	  /* HZ 
	   * - only a GBL forces a renew
	   */
	  if (IPA_FLAG_ISSET(new_flags, IPA_CG_EDGE_FLAGS_GBL))
	    {
	      renew = 1;
	    }
	}
    }

  /* Set the new flags 
   */
  edge->flags |= new_flags;

  /* Renew the edge
   */
  if (renew)
    {
      CG_EDGE_SETNEW(edge);
      apply_rn_cnt++;
    }

  /* Priority: EXP -> IMP 
   */
  if (IPA_FLAG_ISSET(edge->flags,IPA_CG_EDGE_FLAGS_EXPLICIT) &&
      IPA_FLAG_ISSET(edge->flags,IPA_CG_EDGE_FLAGS_IMPLICIT))
    {
      IPA_FLAG_CLR(edge->flags,IPA_CG_EDGE_FLAGS_IMPLICIT);
    }
}

IPA_cgraph_edge_t *
IPA_consg_ensure_edge_d (IPA_cgraph_edgelist_e edge_type,
			 IPA_cgraph_node_t * src_node,
                         IPA_cgraph_node_t * dst_node,
                         IPA_cgraph_edge_data_t * edata, 
			 int edge_origin)
{
  IPA_cgraph_edge_t *edge;

  if (skip_ensure_edge(src_node, dst_node, edge_type, edata))
    return NULL;

  edge = IPA_cg_edge_add (src_node, dst_node, edge_type, edata);

  if (CG_EDGE_ISNEW(edge))
    {
      IPA_FLAG_SET (edge->flags, edge_origin);
      IPA_FLAG_CLR (edge->flags, (IPA_CG_EDGE_FLAGS_PROCESSED|
				  IPA_CG_EDGE_FLAGS_CALLG_PROCESSED|
				  IPA_CG_EDGE_FLAGS_GENERICALL));
#if 0
      if (src_node->data.var_id == 5327 && 
	  src_node->data.version == 58)
	{
	  printf("NEW [%p] %s",edge,IPA_cg_edge_flag_name(edge));
	  IPA_cg_node_print(stdout,edge->dst_elist->node, IPA_PRINT_ASCI);
	  printf(" <- ");
	  IPA_cg_node_print(stdout,edge->src_elist->node, IPA_PRINT_ASCI);
	  printf("\n");
	}
#endif

      DEBUG(check_node(edge->src_elist->node););
      DEBUG(check_node(edge->dst_elist->node););
    }
  else
    {
      IPA_consg_apply_edge_flags(edge, edge_origin);
    }
  DEBUG(check_edge(edge););

  return edge;
}

IPA_cgraph_edge_t *
IPA_consg_ensure_edge (IPA_cgraph_edgelist_e edge_type,
                       IPA_cgraph_node_t * src_node,
                       IPA_cgraph_node_t * dst_node,
		       unsigned int t_offset,
		       unsigned int size,
		       unsigned int s_offset,
                       int edge_origin)
{
  IPA_cgraph_edge_t *edge;
  IPA_cgraph_edge_data_t edata;

  edata.assign_size = size;
  edata.target_stride = 0;
  edata.target_offset = t_offset;
  edata.source_stride = 0;
  edata.source_offset = s_offset;
 
  if (skip_ensure_edge(src_node, dst_node, edge_type, &edata))
    return NULL;

  edge = IPA_cg_edge_add (src_node, dst_node, edge_type, &edata);

  if (CG_EDGE_ISNEW(edge))
    {
      IPA_FLAG_SET (edge->flags, edge_origin);
      IPA_FLAG_CLR (edge->flags, (IPA_CG_EDGE_FLAGS_PROCESSED|
				  IPA_CG_EDGE_FLAGS_CALLG_PROCESSED|
				  IPA_CG_EDGE_FLAGS_GENERICALL));
#if 0
      if (src_node->data.var_id == 5327 && 
	  src_node->data.version == 58)
	{
	  printf("NEW [%p] %s",edge, IPA_cg_edge_flag_name(edge));
	  IPA_cg_node_print(stdout,edge->dst_elist->node, IPA_PRINT_ASCI);
	  printf(" <- ");
	  IPA_cg_node_print(stdout,edge->src_elist->node, IPA_PRINT_ASCI);
	  printf("\n");
	}
#endif
      DEBUG(check_node(edge->src_elist->node););
      DEBUG(check_node(edge->dst_elist->node););
    }
  else
    {
      IPA_consg_apply_edge_flags(edge, edge_origin);
    }
  DEBUG(check_edge(edge););

  return edge;
}


/*****************************************************************************
 * List out the edges
 *****************************************************************************/

List
IPA_consg_build_listof_edges (List edge_list, IPA_cgraph_t * consg)
{
  IPA_cgraph_node_t      *node;
  IPA_cgraph_edge_t      *edge;
  IPA_cgraph_edge_list_t *elist;
  IPA_HTAB_ITER eiter;
  IPA_HTAB_ITER niter;

  /* Look at each node */
  IPA_HTAB_START(niter, consg->nodes);
  IPA_HTAB_LOOP(niter)
    {
      node = (IPA_cgraph_node_t *)IPA_HTAB_CUR(niter);

      /* Add all EXPLICIT or local incoming edges 
       */
      for (elist = node->first_list; elist; elist = elist->nxt_list)
	{
	  IPA_HTAB_START(eiter, elist->in);
	  IPA_HTAB_LOOP(eiter)
	    {
	      edge = (IPA_cgraph_edge_t *)IPA_HTAB_CUR(eiter);

	      /* The direct inclusion of lib.c makes this necessary
		 because a GLOBAL graph edge may point into them */
	      if (!edge->src_elist->node->data.syminfo->fninfo->has_been_called ||
		  !edge->dst_elist->node->data.syminfo->fninfo->has_been_called)
		continue;
#if 1
	      if (IPA_FLAG_ISSET(edge->flags, IPA_CG_EDGE_FLAGS_EXPLICIT) ||
		  (edge->src_elist->node->cgraph == edge->dst_elist->node->cgraph))
#endif
		{
		  
		  edge_list = List_insert_last (edge_list, edge);
		}
	    }
#if 0
	  IPA_HTAB_START(eiter, elist->out);
	  IPA_HTAB_LOOP(eiter)
	    {
	      edge = (IPA_cgraph_edge_t *)IPA_HTAB_CUR(eiter);
	      edge_list = List_insert_last (edge_list, edge);
	    }
#endif
	}
    }

  return edge_list;
}

List
IPA_consg_build_listof_new_edges (List edge_list, IPA_funcsymbol_info_t *fninfo)
{
  IPA_callg_node_t       *callg_node;
  IPA_cgraph_t           *consg;
  IPA_cgraph_node_t      *node;
  IPA_cgraph_edge_t      *edge;
  IPA_cgraph_edge_list_t *elist;
  IPA_HTAB_ITER eiter;
  IPA_HTAB_ITER niter;
  List clist = NULL;
  
  if (IPA_csrec_option == IPA_CSREC_FULL)
    {
      /* edges are spread across all members of SCC */
      for (callg_node = fninfo->call_node; callg_node; callg_node = callg_node->rep_child)
	{
	  if (!callg_node->fninfo->consg)
	    continue;
	  clist = List_insert_last(clist, callg_node->fninfo->consg);
	}
    }
  else
    {
      clist = List_insert_last(clist, fninfo->consg);
    }
  
  List_start(clist);
  while ((consg = List_next(clist)))
    {
      /* Look at each node */
      IPA_HTAB_START(niter, consg->nodes);
      IPA_HTAB_LOOP(niter)
	{
	  node = (IPA_cgraph_node_t *)IPA_HTAB_CUR(niter);
	  
	  for (elist = node->first_list; elist; elist = elist->nxt_list)
	    {
	      IPA_HTAB_START(eiter, elist->in);
	      IPA_HTAB_LOOP(eiter)
		{
		  edge = (IPA_cgraph_edge_t *)IPA_HTAB_CUR(eiter);
		  
		  /* The direct inclusion of lib.c makes this necessary
		     because a GLOBAL graph edge may point into them */
		  if (!edge->src_elist->node->data.syminfo->fninfo->has_been_called ||
		      !edge->dst_elist->node->data.syminfo->fninfo->has_been_called)
		    continue;
		  
		  if (IPA_FLAG_ISSET(edge->flags, IPA_CG_EDGE_FLAGS_PROCESSED))
		    continue;
		  
		  edge_list = List_insert_last (edge_list, edge);
		}
	      
	      IPA_HTAB_START(eiter, elist->out);
	      IPA_HTAB_LOOP(eiter)
		{
		  edge = (IPA_cgraph_edge_t *)IPA_HTAB_CUR(eiter);
		  
		  /* The direct inclusion of lib.c makes this necessary
		     because a GLOBAL graph edge may point into them */
		  if (!edge->src_elist->node->data.syminfo->fninfo->has_been_called ||
		      !edge->dst_elist->node->data.syminfo->fninfo->has_been_called)
		    continue;
		  
		  if (IPA_FLAG_ISSET(edge->flags, IPA_CG_EDGE_FLAGS_PROCESSED))
		    continue;
		  
		  edge_list = List_insert_last (edge_list, edge);
		}
	    }
	}
    }
  
  List_reset(clist);
  return edge_list;
}


/*****************************************************************************
 * Delete all implicit edges
 *****************************************************************************/

void
IPA_consg_delete_implicit_edges (IPA_cgraph_t * consg)
{
  IPA_cgraph_node_t      *node;
  IPA_cgraph_edge_t      *edge;
  IPA_cgraph_edge_list_t *elist;
  IPA_HTAB_ITER eiter;
  IPA_HTAB_ITER niter;
  int del = 0;

  /* Look at each node */
  IPA_HTAB_START(niter, consg->nodes);
  IPA_HTAB_LOOP(niter)
    {
      node = (IPA_cgraph_node_t *)IPA_HTAB_CUR(niter);
      
      for (elist = node->first_list; elist; elist = elist->nxt_list)
	{
	  IPA_HTAB_START(eiter, elist->out);
	  IPA_HTAB_LOOP(eiter)
	    {
	      edge = (IPA_cgraph_edge_t *)IPA_HTAB_CUR(eiter);

	      if (IPA_FLAG_ISSET (edge->flags, (IPA_CG_EDGE_FLAGS_EXPLICIT |
						IPA_CG_EDGE_FLAGS_HZ)))
		continue;

	      IPA_cg_edge_delete (edge);
	      del++;
	    }
	  IPA_HTAB_START(eiter, elist->in);
	  IPA_HTAB_LOOP(eiter)
	    {
	      edge = (IPA_cgraph_edge_t *)IPA_HTAB_CUR(eiter);

	      if (IPA_FLAG_ISSET (edge->flags, (IPA_CG_EDGE_FLAGS_EXPLICIT |
						IPA_CG_EDGE_FLAGS_HZ)))
		continue;

	      IPA_cg_edge_delete (edge);
	      del++;
	    }
	}
    }

  DEBUG_IPA(2, printf ("Deleted %d implicit edges\n", del););
}



/*****************************************************************************
 * Formal-Actual Linking
 *****************************************************************************/
static IPA_cgraph_node_t *
get_actual(IPA_cgraph_t *actual_cg, int actual_id, int actual_ver)
{
  IPA_cgraph_node_t *actual_node;

  assert (actual_ver != UNDEF_SUBSCR);
  
  actual_node = IPA_consg_find_node (actual_cg, actual_id, actual_ver);
  if (actual_node)
    actual_node = IPA_cg_node_get_rep(actual_node);

  return actual_node;
}

static IPA_cgraph_node_t *
get_formal(IPA_cgraph_t *formal_cg, int formal_id, int formal_ver)
{
  IPA_cgraph_node_t *formal_node;

  assert (formal_ver != UNDEF_SUBSCR);

  formal_node = IPA_consg_find_node (formal_cg, formal_id, formal_ver);
  if (formal_node)
    assert(formal_node->rep_parent == formal_node);

  return formal_node;
}

static int
IPA_consg_link_formalactual(IPA_cgraph_t *actual_cg, int actual_id,
			    IPA_cgraph_t *formal_cg, int formal_id)
{
  IPA_cgraph_node_t *actual_node;
  IPA_cgraph_node_t *formal_node;
  int fml_isellipse = 0;

  actual_node = get_actual(actual_cg, actual_id, 1);
  formal_node = get_formal(formal_cg, formal_id, 1);

  if (formal_node &&
      IPA_FLAG_ISSET(formal_node->flags, IPA_CG_NODE_FLAGS_ELLIPSE))
    fml_isellipse = 1;

  if (!formal_node || !actual_node)
    return fml_isellipse;

  IPA_cg_link_nodes (formal_cg, formal_node,
		     actual_cg, actual_node);

#if 0
  printf("P-ACT : ");
  IPA_cg_node_print(stdout, actual_node);
  printf("\n");
  printf("P-FML : ");
  IPA_cg_node_print(stdout, formal_node);
  printf("\n");
#endif  

  return fml_isellipse;
}

static void
IPA_consg_link_params (IPA_cgraph_t * caller_cng,
                       IPA_interface_t * caller_iface,
                       IPA_cgraph_t * callee_cng,
                       IPA_interface_t * callee_iface)
{
  int actual_id;
  int formal_id = 0;
  int i, formal_bound, actual_bound;
  int ellipse_reached = 0;

  /* Link nodes from caller's actual params to
   *  callee's formal params
   */
  actual_bound = IPA_interface_get_num_params (caller_iface);
  formal_bound = IPA_interface_get_num_params (callee_iface);
  for (i = 0; i < actual_bound; i++)
    {
      actual_id = IPA_interface_get_param_id (caller_iface, i);
      if (!actual_id)
        continue;
      if (!ellipse_reached)
	{
	  if (i >= formal_bound)
	    {
	      /* There are more parameters than found in formal
		 parameter list */
	      break;
	    }
	  formal_id = IPA_interface_get_param_id (callee_iface, i);
	}

      ellipse_reached = 
	IPA_consg_link_formalactual(caller_cng, actual_id,
				    callee_cng, formal_id);
    }

  /* Link appropriate returns */
  actual_id = IPA_interface_get_ret_id (caller_iface);
  formal_id = IPA_interface_get_ret_id (callee_iface);
  if (formal_id && actual_id)
    {
      IPA_consg_link_formalactual(caller_cng, actual_id,
				  callee_cng, formal_id);
    }
}


/*****************************************************************************
 * Formal-Actual Assignments
 *****************************************************************************/

void
IPA_consg_assign_params (IPA_cgraph_t * caller_cng,
                         IPA_interface_t * caller_iface,
			 IPA_cgraph_t * callee_cng,
                         IPA_interface_t * callee_iface,
			 int filter_edge_flags,
			 int set_edge_flags)
{
  int actual_id;
  int formal_id = 0;
  int i, formal_bound, actual_bound;
  int ellipse_reached = 0;

  /* Add assignment from  actual params in caller to
   *   formal params of the callee
   */
  actual_bound = IPA_interface_get_num_params (caller_iface);
  formal_bound = IPA_interface_get_num_params (callee_iface);
  for (i = 0; i < actual_bound; i++)
    {
      IPA_cgraph_edge_list_t *elist;
      IPA_cgraph_node_t *actual_node;
      IPA_cgraph_node_t *formal_node;
      IPA_cgraph_edge_t *edge;
      IPA_HTAB_ITER eiter;
      
      actual_id = IPA_interface_get_param_id (caller_iface, i);
      if (!actual_id)
	continue;
      if (!ellipse_reached)
	{
	  if (i >= formal_bound)
	    {
	      /* There are more parameters than found in formal
		 parameter list */
	      break;
	    }
	  formal_id = IPA_interface_get_param_id (callee_iface, i);
	}
      actual_node = get_actual(caller_cng, actual_id, 1);
      formal_node = get_formal(callee_cng, formal_id, 1);

      if (formal_node &&
	  IPA_FLAG_ISSET(formal_node->flags, IPA_CG_NODE_FLAGS_ELLIPSE))
	ellipse_reached = 1;

      if (!actual_node || !formal_node)
	continue;

#if 0
      printf("ACT,FORM : ");
      IPA_cg_node_print(stdout, actual_node);
      IPA_cg_node_print(stdout, formal_node);
      printf("\n");
#endif

      /* PARAMS: Add all out edges of formal node to actual_node */
      for (elist = formal_node->first_list; elist;
	   elist = elist->nxt_list)
	{
	  assert(elist->edge_type == ASSIGN);
	  
	  IPA_HTAB_START(eiter, elist->out);
	  IPA_HTAB_LOOP(eiter)
	    {
	      edge = (IPA_cgraph_edge_t *)IPA_HTAB_CUR(eiter);
	      
	      /* Only do matching flagged edges */
	      if (!IPA_FLAG_ISSET(edge->flags, filter_edge_flags))
		continue;
	      
	      IPA_consg_ensure_edge_d (elist->edge_type, 
				       actual_node, 
				       edge->dst_elist->node,
				       &edge->data,
				       set_edge_flags);
	    }
	} /* elist */
    }
}

void
IPA_consg_assign_return (IPA_cgraph_t * caller_cng,
                         IPA_interface_t * caller_iface,
			 IPA_cgraph_t * callee_cng,
                         IPA_interface_t * callee_iface,
			 int filter_edge_flags,
			 int set_edge_flags)
{
  IPA_cgraph_node_t *actual_node;
  IPA_cgraph_node_t *formal_node;
  IPA_cgraph_edge_list_t *elist;
  IPA_cgraph_edge_t *edge;
  IPA_HTAB_ITER eiter;
  int actual_id;
  int formal_id;

  /* Add assignment from formal return to actual return */
  actual_id = IPA_interface_get_ret_id (caller_iface);
  formal_id = IPA_interface_get_ret_id (callee_iface);
  /* May not have a return */
  if (!formal_id || !actual_id)
    return;

  actual_node = get_actual(caller_cng, actual_id, caller_iface->version);
  formal_node = get_formal(callee_cng, formal_id, callee_iface->version);
  /* May not use its return */
  if (!actual_node || !formal_node)
    return;

  /* RETURNS: Add all in edges of formal node to actual_node */
  for (elist = formal_node->first_list; elist;
       elist = elist->nxt_list)
    {
      assert(elist->edge_type == ASSIGN ||
	     elist->edge_type == ASSIGN_ADDR);
      if (elist->edge_type != ASSIGN)
	continue;
      
      IPA_HTAB_START(eiter, elist->in);
      IPA_HTAB_LOOP(eiter)
	{
	  edge = (IPA_cgraph_edge_t *)IPA_HTAB_CUR(eiter);
	  
	  /* Only do matching flagged edges */
	  if (!IPA_FLAG_ISSET(edge->flags, filter_edge_flags))
	    continue;
	  
	  IPA_consg_ensure_edge_d (elist->edge_type,
				   edge->src_elist->node, 
				   actual_node,
				   &edge->data,
				   set_edge_flags);
	}
    } /* elist */
}


/*****************************************************************************
 *
 *****************************************************************************/
void
IPA_consg_make_node_ci(IPA_prog_info_t * info, 
		       IPA_cgraph_node_t *node)
{
  if (IPA_FLAG_ISSET(node->flags, (IPA_CG_NODE_FLAGS_PARAM |
				   IPA_CG_NODE_FLAGS_RETURN)))
    return;

  IPA_FLAG_SET(node->flags, IPA_CG_NODE_FLAGS_NOCNTXT);
}

void
IPA_consg_make_cg_ci(IPA_prog_info_t * info, 
		     IPA_cgraph_t * cng)
{
  IPA_HTAB_ITER niter;
  IPA_cgraph_node_t *node;

  printf("FORCING CI\n");
  cng->data.fninfo->forced_ci = 1;
  IPA_HTAB_START(niter, cng->nodes);
  IPA_HTAB_LOOP(niter)
    {
      node = (IPA_cgraph_node_t *)IPA_HTAB_CUR(niter);
      IPA_consg_make_node_ci(info, node);
    }
}

/*****************************************************************************
 * Given a consg graph and callsite list, mark all callee control nodes
 *****************************************************************************/

static void
IPA_make_edges_gbl(IPA_cgraph_node_t *node)
{
  IPA_HTAB_ITER eiter;
  IPA_cgraph_edge_t      *edge;
  IPA_cgraph_edge_list_t *elist;
  
  for (elist = node->first_list; elist; elist = elist->nxt_list)
    {
      if (elist->edge_type != ASSIGN_ADDR)
	continue;

      IPA_HTAB_START(eiter, elist->out);
      IPA_HTAB_LOOP(eiter)
	{
	  edge = (IPA_cgraph_edge_t *)IPA_HTAB_CUR(eiter);
	  if (!IPA_FLAG_ISSET(edge->flags, IPA_CG_EDGE_FLAGS_EXPLICIT))
	    continue;
	  IPA_FLAG_SET (edge->flags, IPA_CG_EDGE_FLAGS_GBL);

#if 0 
	  printf("Fixing Explicit GBL flag\n");
	  IPA_cg_node_print(stdout,edge->dst_elist->node, IPA_PRINT_ASCI);
	  printf(" <- %d,%d,%d -",
		 edge->data.target_offset, 
		 edge->data.assign_size,
		 edge->data.source_offset);
	  IPA_cg_node_print(stdout,edge->src_elist->node, IPA_PRINT_ASCI);
	  printf("\n");
#endif
	}
    }

  return;
}

void
IPA_consg_set_nofield(IPA_cgraph_node_t *node)
{
  IPA_FLAG_SET (node->flags, IPA_CG_NODE_FLAGS_NOFIELD);
  node->data.mod = 1;
}

static void
IPA_consg_set_node_syminfo(IPA_cgraph_node_t *node)
{
  int kind = node->data.syminfo->kind;

  if (IPA_FLAG_ISSET (kind, IPA_VAR_KIND_TEMP))
    IPA_FLAG_SET (node->flags, IPA_CG_NODE_FLAGS_TEMP);

  if (IPA_FLAG_ISSET (kind, (IPA_VAR_KIND_GLOBAL)))
    {
      IPA_FLAG_SET (node->flags, IPA_CG_NODE_FLAGS_GLOBAL);
      /* Not context sensitive and can escape through returns */
      IPA_FLAG_SET (node->flags, IPA_CG_NODE_FLAGS_NOCNTXT);      
      IPA_FLAG_SET (node->flags, IPA_CG_NODE_FLAGS_NOLOCAL);      
    }

  if (IPA_FLAG_ISSET (kind, (IPA_VAR_KIND_PARAM)))
    IPA_FLAG_SET (node->flags, IPA_CG_NODE_FLAGS_PARAM);

  if (IPA_FLAG_ISSET (kind, (IPA_VAR_KIND_RETURN)))
    IPA_FLAG_SET (node->flags, IPA_CG_NODE_FLAGS_RETURN);

  if (IPA_FLAG_ISSET (kind, (IPA_VAR_KIND_FUNC)))
    {
      IPA_FLAG_SET (node->flags, IPA_CG_NODE_FLAGS_FUNC);
      IPA_consg_set_nofield(node);
    }

  if (IPA_FLAG_ISSET (kind, (IPA_VAR_KIND_HEAP)))
    {
      IPA_FLAG_SET (node->flags, IPA_CG_NODE_FLAGS_HEAP);

      IPA_FLAG_SET (node->flags, IPA_CG_NODE_FLAGS_NOLOCAL);
      if (IPA_cloning_option == IPA_NO_HEAP_CLONING)
	{
	  IPA_FLAG_SET (node->flags, IPA_CG_NODE_FLAGS_NOCNTXT);
	}
    }

  if (IPA_FLAG_ISSET (kind, (IPA_VAR_KIND_STACK)))
    {
      IPA_FLAG_SET (node->flags, IPA_CG_NODE_FLAGS_STACK);
    }
}

static void
IPA_consg_setup_node(IPA_cgraph_node_t *node)
{
  /* Set basic node flags
   */
  IPA_consg_set_node_syminfo(node);
  
  /* Get edges from NOCNTXT nodes into the correct starting state
   */
  if (IPA_FLAG_ISSET (node->flags, IPA_CG_NODE_FLAGS_NOCNTXT))
    IPA_make_edges_gbl(node);
}


/* WARNING!!! - THIS SHOULD ONLY BE CALLED BEFORE ANY ANALYSIS TAKES PLACE
 *   TO SETUP INITIAL VALUES
 */
void
IPA_consg_setup_nodes (IPA_prog_info_t * info, 
		       IPA_funcsymbol_info_t * fninfo,
		       IPA_cgraph_t * cng)
{
  IPA_cgraph_node_t *node;
  IPA_HTAB_ITER niter;
  IPA_callsite_t *cs;
  
  /* Mark CALLEE control nodes
   */
  assert (fninfo);

  List_start (fninfo->callsites);
  while ((cs = List_next (fninfo->callsites)))
    {
      int id;
      
      if (!cs->indirect)
	continue;
      
      id = cs->callee.cnode_id;
      node = IPA_consg_find_node (cng, id, 1);
      assert (node);
      
      IPA_FLAG_SET (node->flags, IPA_CG_NODE_FLAGS_CALLEE);
    }

  IPA_HTAB_START(niter, cng->nodes);
  IPA_HTAB_LOOP(niter)
    {
      node = (IPA_cgraph_node_t *)IPA_HTAB_CUR(niter);

      IPA_consg_setup_node(node);

      /* Heap vars have special size requirements
       */
      if (IPA_FLAG_ISSET (node->flags, IPA_CG_NODE_FLAGS_HEAP) ||
	  IPA_FLAG_ISSET (node->flags, IPA_CG_NODE_FLAGS_STACK))
	{
	  node->data.var_size = IPA_max_type_size;
	  node->data.mod = IPA_max_type_size;
	}
    }
}


void
IPA_consg_setup_nodes_list (IPA_prog_info_t * info, 
			    List mod_list)
{
  IPA_cgraph_node_t *node;

  List_start(mod_list);
  while ((node = List_next(mod_list)))
    {
      IPA_consg_setup_node(node);
    }
}


/*****************************************************************************
 * Apply graph for constraint graphs
 *****************************************************************************/

static List
add_all_linked(List m, IPA_cgraph_node_t *node)
{	  
  IPA_cgraph_link_t *l;
  for (l=node->lk;l;l=l->nxt_l)
    {
      assert(node->lk->to);

      assert(!IPA_FLAG_ISSET (l->to->flags, IPA_CG_NODE_FLAGS_SUMMARY));
      m = List_insert_last(m, l->to);
    }
  return m;
}

static List
IPA_consg_apply_graph (IPA_prog_info_t * info,
                       IPA_cgraph_t * cg_d, IPA_cgraph_t * cg_s,
                       HashTable cs_ver_htab,
		       IPA_cgraph_t *callee_cg,
		       int allow_unlinked_parret)
{
  IPA_cgraph_node_t *node;
  IPA_cgraph_node_t *d_node;
  IPA_HTAB_ITER niter;
  List mod_list = NULL;

  /* The first step is to find any nodes that exist in cg_s
   * that do not have a link to anything in cg_d. For each
   * of these, a node in cg_d needs to be created and linked
   * to the one in cg_s 
   */
  IPA_HTAB_START(niter, cg_s->nodes);
  IPA_HTAB_LOOP(niter)
    {
      node = (IPA_cgraph_node_t *)IPA_HTAB_CUR(niter);
      /* These may get unified inside summaries. The children are
       *   needed to allow versioning to get the UP/DOWN correct,
       *   nothing more.
       */
      if (node->rep_parent != node)
	{
	  assert(IPA_FLAG_ISSET (node->flags, (IPA_CG_NODE_FLAGS_HEAP|
					       IPA_CG_NODE_FLAGS_ESCLOCAL)));
	  continue;
	}

      if (node->lk)
	{
	  mod_list = add_all_linked(mod_list, node);
	  continue;
	}

      if (!allow_unlinked_parret)
	{
	  if (IPA_FLAG_ISSET (node->flags, IPA_CG_NODE_FLAGS_PARAM))
	    {
	      fprintf(info->errfile,"No link for param id%d\n",
		      node->data.var_id);
	      continue;
	    }
	  else if (IPA_FLAG_ISSET (node->flags, IPA_CG_NODE_FLAGS_RETURN))
	    {
	      fprintf(info->errfile,"No link for return id%d\n",
		      node->data.var_id);
	      continue;
	    }
	}

      if ((IPA_csrec_option == IPA_CSREC_FULL) &&
	  (d_node = IPA_consg_find_node (cg_d, 
					 node->data.var_id, 1)))
	{
	  /* !This should only occur in CSREC_FULL mode!
	   *  Look for original version of node and use it instead
	   */
	  printf("REC_CS:       found orig node\n");
	}
      else
	d_node = IPA_consg_node_new_version (info, node,
					     cg_d, callee_cg,
					     cs_ver_htab);

      /* Its possible that a different summary for a different routine
	 in the same SCC has already created this node */
      if (CG_NODE_ISNEW(d_node))
	{
	  mod_list = List_insert_last(mod_list, d_node);
#if 0
	  printf("MODLIST Added: %p %d.%d \n", 
		 d_node,
		 d_node->data.var_id, d_node->data.version);
#endif
	}

      /* Some heap nodes may have been left behind if the new summary
       *   still had their ancestors. These might have been merged.
       *   Get the parent.
       */
      d_node = IPA_cg_node_get_rep(d_node);
      
      IPA_cg_link_nodes (cg_s, node, cg_d, d_node);
    }
  
  /* At this point, there should be a node in cg_d for every
   *  node in cg_s. Now we just have to connect them 
   */
  IPA_cg_apply_graph (cg_d, cg_s);

  return mod_list;
}


/*****************************************************************************
 * Delete selected/all summary nodes from a constraint graph
 *****************************************************************************/

static int 
ancestor_in_newlist(IPA_cgraph_node_t *node, List new_heaplist)
{
  IPA_cgraph_node_t *hpnode;

  List_start(new_heaplist);
  while ((hpnode = List_next(new_heaplist)))
    {
      if (node->from_version == hpnode->data.version)
	return 1;
    }

  return 0;
}

void
IPA_consg_delete_summary_nodes2(IPA_callg_edge_t *callee_edge, List new_heaplist)
{
  IPA_cgraph_node_t      *node;
  int count = 0;
  int heap = 0;

  DEBUG_IPA(3, printf("Delete summary nodes in [%s] for [%s]\n",
		      callee_edge->caller->fninfo->func_name,
		      callee_edge->callee->fninfo->func_name););

  /* This call edge has no longer been accounted for */
  callee_edge->summary_incorporated = 0;

  List_start(callee_edge->sum_nodes);
  while ((node = List_next(callee_edge->sum_nodes)))
    {
      assert(IPA_FLAG_ISSET(node->flags, IPA_CG_NODE_FLAGS_SUMMARY));
	  
      if (IPA_FLAG_ISSET(node->flags, IPA_CG_NODE_FLAGS_HEAP) &&
	  ancestor_in_newlist(node, new_heaplist))
	continue;

      if (IPA_FLAG_ISSET(node->flags, IPA_CG_NODE_FLAGS_HEAP))
	heap++;
      count++;
#if 0      
      printf("DELSUM %p %d.%d\n", 
	     node,
	     node->data.var_id, node->data.version);
#endif
      IPA_cg_node_delete(node);
      callee_edge->sum_nodes = List_delete_current(callee_edge->sum_nodes);
    }

#if 0
  List_reset(callee_edge->sum_nodes);
  callee_edge->sum_nodes = NULL;
#endif

#if 0
  if (count + heap > 0)
    {
      printf("Delete summary %d %d nodes in [%s] for [%s]\n",
	     count, heap,
	     callee_edge->caller->fninfo->func_name,
	     callee_edge->callee->fninfo->func_name);
      if (callee_edge->caller->fninfo->consg)
	printf("   After del %d\n", IPA_htab_size(callee_edge->caller->fninfo->consg->nodes));
    }
#endif
}


void
IPA_consg_delete_all_summary_nodes2 (IPA_funcsymbol_info_t *fninfo)
{
  IPA_callg_edge_t *callee_edge;
  IPA_callg_node_t *callg_node;

  callg_node = fninfo->call_node;
  assert(callg_node->rep_parent == callg_node);
  
#if 0
  printf("SUMDELETE ALL\n");
#endif

  /* For each func in SCC */
  for (; callg_node; callg_node = callg_node->rep_child)
    {
      /* For each callee of the func */
      List_start(callg_node->callee_edges);
      while ((callee_edge = List_next(callg_node->callee_edges)))
	{
	  IPA_consg_delete_summary_nodes2(callee_edge, NULL);
	}
    }

#if 0
  if (fninfo->consg)
    {
      IPA_cgraph_node_t      *node;
      IPA_HTAB_ITER niter;
      
      /* Look at each node */
      IPA_HTAB_START(niter, fninfo->consg->nodes);
      IPA_HTAB_LOOP(niter)
	{
	  node = (IPA_cgraph_node_t *)IPA_HTAB_CUR(niter);
	  assert(!IPA_FLAG_ISSET(node->flags, IPA_CG_NODE_FLAGS_SUMMARY));
	}
    }
#endif
}


/*****************************************************************************
 * Attribute the added summary nodes to a particular call graph edge
 *****************************************************************************/

static void
IPA_consg_assign_summary_nodes2(List mod_list,
				IPA_callg_edge_t *callee_edge)
{
  IPA_cgraph_node_t      *node;
  int count = 0;
  
  List_start(mod_list);
  while ((node = List_next(mod_list)))
    {
      if (!IPA_FLAG_ISSET(node->flags, IPA_CG_NODE_FLAGS_SUMMARY))
	continue;
      count++;

      if (List_member(callee_edge->sum_nodes, node))
	continue;
      callee_edge->sum_nodes = List_insert_last(callee_edge->sum_nodes, node);
    }

#if 0
  if (count > 0)
    printf("ASSIGNED %d\n",count);
#endif
}


/*****************************************************************************
 * Given a summary graph and interfaces, apply the summary
 *****************************************************************************/

void
IPA_consg_apply_summary2 (IPA_prog_info_t * info,
			  IPA_cgraph_t * dst_cng,
			  IPA_interface_t * caller_iface,
			  IPA_cgraph_t * summary_cng,
			  IPA_interface_t * callee_iface,
			  HashTable cs_ver_htab,
			  IPA_cgraph_t * callee_cng,
			  IPA_callg_edge_t *callee_edge)
{ 
  List mod_list;

  /* The summary for this call edge has been incorporated */
  callee_edge->summary_incorporated = 1;

  assert (caller_iface && callee_iface);
  IPA_consg_link_params (dst_cng, caller_iface,
			 summary_cng, callee_iface);

  mod_list = IPA_consg_apply_graph (info, dst_cng, summary_cng, 
				    cs_ver_htab, callee_cng, 0);

  IPA_cg_clear_links (summary_cng);

  IPA_consg_assign_summary_nodes2(mod_list, callee_edge);
  IPA_consg_setup_nodes_list(info, mod_list);
  List_reset(mod_list);
}



/*****************************************************************************
 * More checking
 *****************************************************************************/

void
IPA_consg_check_graph(IPA_prog_info_t * info, IPA_cgraph_t * consg)
{
  IPA_cgraph_node_t *node;
  IPA_cgraph_edge_t *edge;
  IPA_cgraph_edge_list_t *elist;
  IPA_symbol_info_t *sym;
  IPA_HTAB_ITER niter;
  IPA_HTAB_ITER eiter;

  IPA_HTAB_START(niter, consg->nodes);
  IPA_HTAB_LOOP(niter)
    {
      node = (IPA_cgraph_node_t *)IPA_HTAB_CUR(niter);
      check_node(node);
      
      for (elist = node->first_list; elist; elist = elist->nxt_list)
	{
	  
	  IPA_HTAB_START(eiter, elist->out);
	  IPA_HTAB_LOOP(eiter)
	    {
	      edge = (IPA_cgraph_edge_t *)IPA_HTAB_CUR(eiter);
	      check_edge(edge);
	    } /* OUT */
	  
	  IPA_HTAB_START(eiter, elist->in);
	  IPA_HTAB_LOOP(eiter)
	    {
	      edge = (IPA_cgraph_edge_t *)IPA_HTAB_CUR(eiter);
	      check_edge(edge);
	    } /* IN */
	}

      /* Check against symbol table */
      sym = IPA_symbol_find_by_id (info, node->data.var_id);
      assert(sym);
      assert(sym == node->data.syminfo);

      if (IPA_FLAG_ISSET (sym->kind, (IPA_VAR_KIND_GLOBAL)))
	assert(IPA_FLAG_ISSET (node->flags, IPA_CG_NODE_FLAGS_GLOBAL));
      
      if (IPA_FLAG_ISSET (sym->kind, (IPA_VAR_KIND_PARAM)))
	assert(IPA_FLAG_ISSET (node->flags, IPA_CG_NODE_FLAGS_PARAM));
      
      if (IPA_FLAG_ISSET (sym->kind, (IPA_VAR_KIND_RETURN)))
	assert(IPA_FLAG_ISSET (node->flags, IPA_CG_NODE_FLAGS_RETURN));
      
      if (IPA_FLAG_ISSET (sym->kind, (IPA_VAR_KIND_FUNC)))
	assert(IPA_FLAG_ISSET (node->flags, IPA_CG_NODE_FLAGS_FUNC));
      
      if (IPA_FLAG_ISSET (sym->kind, (IPA_VAR_KIND_HEAP)))
	{
	  assert(IPA_FLAG_ISSET (node->flags, IPA_CG_NODE_FLAGS_HEAP));
	}

      if (IPA_FLAG_ISSET (node->flags, IPA_CG_NODE_FLAGS_SUMMARY))
	{
#if 0
	  IPA_summary_info_t     *sum_info;
	  int found = 0;
	  IPA_cgraph_node_t      *snode;

	  assert(node->summary_consg != consg);

	  for (sum_info = consg->data.fninfo->sum_info; 
	       sum_info;
	       sum_info = sum_info->nxt)
	    {
	      if (sum_info->fninfo->consg != node->summary_consg)
		continue;

	      List_start(sum_info->sum_nodes);
	      while ((snode = List_next(sum_info->sum_nodes)))
		{
		  if (snode == node)
		    found++;
		}
	    }
	  assert(found == 1);
#else
#if 0
	  IPA_callg_node_t *callg_node;
	  IPA_callg_edge_t *callee_edge;
	  int found = 0;

	  callg_node = consg->data.fninfo->call_node;
	  for (; callg_node; callg_node = callg_node->rep_child)
	    {
	      /* For each callee of the func */
	      List_start(callg_node->callee_edges);
	      while ((callee_edge = List_next(callg_node->callee_edges)))
		{
		  if (List_member(callee_edge->sum_nodes, node))
		    found++;
		}
	    }
	  assert(found == 1);
#endif
#endif
	}

    } 
}



