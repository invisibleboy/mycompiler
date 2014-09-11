/**CFile***********************************************************************

  FileName    [cuddSign.c]

  PackageName [cudd]

  Synopsis    [Computation of signatures]

  Description [External procedures included in this module:
		    <ul>
		    <li> Cudd_CofMinterm();
		    </ul>
		Static procedures included in this module:
		    <ul>
		    <li> ddCofMintermAux()
		    </ul>
		    ]

  Author      [Fabio Somenzi]

  Copyright   [This file was created at the University of Colorado at
  Boulder.  The University of Colorado at Boulder makes no warranty
  about the suitability of this software for any purpose.  It is
  presented on an AS IS basis.]

******************************************************************************/

/* 10/29/02 REK Adding config.h */
#include <config.h>
#include <bdd/util.h>
#include <bdd/st.h>
#include <bdd/cuddInt.h>


/*---------------------------------------------------------------------------*/
/* Constant declarations                                                     */
/*---------------------------------------------------------------------------*/


/*---------------------------------------------------------------------------*/
/* Stucture declarations                                                     */
/*---------------------------------------------------------------------------*/


/*---------------------------------------------------------------------------*/
/* Type declarations                                                         */
/*---------------------------------------------------------------------------*/


/*---------------------------------------------------------------------------*/
/* Variable declarations                                                     */
/*---------------------------------------------------------------------------*/

#ifndef lint
static char rcsid[] DD_UNUSED = "$Id: cuddSign.c,v 1.1.1.1 2005/04/18 17:40:17 mchu Exp $";
#endif

static int    size;

#ifdef DD_STATS
static int num_calls;	/* should equal 2n-1 (n is the # of nodes) */
static int table_mem;
#endif


/*---------------------------------------------------------------------------*/
/* Macro declarations                                                        */
/*---------------------------------------------------------------------------*/


/**AutomaticStart*************************************************************/

/*---------------------------------------------------------------------------*/
/* Static function prototypes                                                */
/*---------------------------------------------------------------------------*/

static double * ddCofMintermAux ARGS((DdManager *dd, DdNode *node, st_table *table));

/**AutomaticEnd***************************************************************/


/*---------------------------------------------------------------------------*/
/* Definition of exported functions                                          */
/*---------------------------------------------------------------------------*/

/**Function********************************************************************

  Synopsis [Computes the fraction of minterms in the on-set of all the
  positive cofactors of a BDD or ADD.]

  Description [Computes the fraction of minterms in the on-set of all
  the positive cofactors of DD. Returns the pointer to an array of
  doubles if successful; NULL otherwise. The array hs as many
  positions as there are BDD variables in the manager plus one. The
  last position of the array contains the fraction of the minterms in
  the ON-set of the function represented by the BDD or ADD. The other
  positions of the array hold the variable signatures.]

  SideEffects [None]

******************************************************************************/
double *
Cudd_CofMinterm(
  DdManager * dd,
  DdNode * node)
{
    st_table	*table;
    double	*values;
    double	*result = NULL;
    int		i, firstLevel;

#ifdef DD_STATS
    long startTime;
    startTime = util_cpu_time();
    num_calls = 0;
    table_mem = sizeof(st_table);
#endif

    table = st_init_table(st_ptrcmp, st_ptrhash);
    if (table == NULL) {
	(void) fprintf(stdout,"out-of-memory, couldn't measure DD cofactors.\n");
	return(NULL);
    }
    size = dd->size;
    values = ddCofMintermAux(dd, node, table);
    if (values != NULL) {
	result = ALLOC(double,size + 1);
	if (result != NULL) {
#ifdef DD_STATS
	    table_mem += (size + 1) * sizeof(double);
#endif
	    if (Cudd_IsConstant(node))
		firstLevel = 1;
	    else
		firstLevel = cuddI(dd,Cudd_Regular(node)->index);
	    for (i = 0; i < size; i++) {
		if (i >= cuddI(dd,Cudd_Regular(node)->index)) {
		    result[dd->invperm[i]] = values[i - firstLevel];
		} else {
		    result[dd->invperm[i]] = values[size - firstLevel];
		}
	    }
	    result[size] = values[size - firstLevel];
	} else {
	    dd->errorCode = CUDD_MEMORY_OUT;
	}
    }

#ifdef DD_STATS
    table_mem += table->num_bins * sizeof(st_table_entry *);
#endif
    if (Cudd_Regular(node)->ref == 1) FREE(values);
    st_foreach(table, cuddStCountfree, NULL);
    st_free_table(table);
#ifdef DD_STATS
    (void) fprintf(stdout,"Number of calls: %d\tTable memory: %d bytes\n",
   		  num_calls, table_mem);
    (void) fprintf(stdout,"Time to compute measures: %s\n",
		  util_print_time(util_cpu_time() - startTime));
#endif
    if (result == NULL) {
	(void) fprintf(stdout,"out-of-memory, couldn't measure DD cofactors.\n");
    }
    return(result);

} /* end of Cudd_CofMinterm */


/*---------------------------------------------------------------------------*/
/* Definition of internal functions                                          */
/*---------------------------------------------------------------------------*/


/*---------------------------------------------------------------------------*/
/* Definition of static functions                                            */
/*---------------------------------------------------------------------------*/


/**Function********************************************************************

  Synopsis    [Recursive Step for Cudd_CofMinterm function.]

  Description [Traverses the DD node and computes the fraction of
  minterms in the on-set of all positive cofactors simultaneously.
  It allocates an array with two more entries than there are
  variables below the one labeling the node.  One extra entry (the
  first in the array) is for the variable labeling the node. The other
  entry (the last one in the array) holds the fraction of minterms of
  the function rooted at node.  Each other entry holds the value for
  one cofactor. The array is put in a symbol table, to avoid repeated
  computation, and its address is returned by the procedure, for use
  by the caller.  Returns a pointer to the array of cofactor measures.]

  SideEffects [None]

  SeeAlso     []

******************************************************************************/
static double *
ddCofMintermAux(
  DdManager * dd,
  DdNode * node,
  st_table * table)
{
    DdNode	*N;		/* regular version of node */
    DdNode	*Nv, *Nnv;
    double	*values;
    double	*valuesT, *valuesE;
    int		i;
    int		localSize, localSizeT, localSizeE;
    double	vT, vE;

#ifdef DD_STATS
    num_calls++;
#endif

    if (st_lookup(table, (char *) node, (char **) &values)) {
	return(values);
    }

    N = Cudd_Regular(node);
    if (cuddIsConstant(N)) {
	localSize = 1;
    } else {
	localSize = size - cuddI(dd,N->index) + 1;
    }
    values = ALLOC(double, localSize);
    if (values == NULL) {
	dd->errorCode = CUDD_MEMORY_OUT;
	return(NULL);
    }

    if (cuddIsConstant(N)) {
	if (node == DD_ZERO(dd) || node == Cudd_Not(DD_ONE(dd))) {
	    values[0] = 0.0;
	} else {
	    values[0] = 1.0;
	}
    } else {
	Nv = Cudd_NotCond(cuddT(N),N!=node);
	Nnv = Cudd_NotCond(cuddE(N),N!=node);

	valuesT = ddCofMintermAux(dd, Nv, table);
	if (valuesT == NULL) return(NULL);
	valuesE = ddCofMintermAux(dd, Nnv, table);
	if (valuesE == NULL) return(NULL);

	if (Cudd_IsConstant(Nv)) {
	    localSizeT = 1;
	} else {
	    localSizeT = size - cuddI(dd,Cudd_Regular(Nv)->index) + 1;
	}
	if (Cudd_IsConstant(Nnv)) {
	    localSizeE = 1;
	} else {
	    localSizeE = size - cuddI(dd,Cudd_Regular(Nnv)->index) + 1;
	}
	values[0] = valuesT[localSizeT - 1];
	for (i = 1; i < localSize; i++) {
	    if (i >= cuddI(dd,Cudd_Regular(Nv)->index) - cuddI(dd,N->index)) {
		vT = valuesT[i - cuddI(dd,Cudd_Regular(Nv)->index) +
			    cuddI(dd,N->index)];
	    } else {
		vT = valuesT[localSizeT - 1];
	    }
	    if (i >= cuddI(dd,Cudd_Regular(Nnv)->index) - cuddI(dd,N->index)) {
		vE = valuesE[i - cuddI(dd,Cudd_Regular(Nnv)->index) +
			    cuddI(dd,N->index)];
	    } else {
		vE = valuesE[localSizeE - 1];
	    }
	    values[i] = (vT + vE) / 2.0;
	}
	if (Cudd_Regular(Nv)->ref == 1) FREE(valuesT);
	if (Cudd_Regular(Nnv)->ref == 1) FREE(valuesE);
    }

    if (N->ref > 1) {
	if (st_add_direct(table, (char *) node, (char *) values) == ST_OUT_OF_MEM) {
	    FREE(values);
	    return(NULL);
	}
#ifdef DD_STATS
	table_mem += localSize * sizeof(double) + sizeof(st_table_entry);
#endif
    }
    return(values);

} /* end of ddCofMintermAux */
