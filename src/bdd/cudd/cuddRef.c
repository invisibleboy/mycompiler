/**CFile***********************************************************************

  FileName    [cuddRef.c]

  PackageName [cudd]

  Synopsis    [Functions that manipulate the reference counts.]

  Description [External procedures included in this module:
		    <ul>
		    <li> Cudd_Ref()
		    <li> Cudd_RecursiveDeref()
		    <li> Cudd_IterDerefBdd()
		    <li> Cudd_RecursiveDerefZdd()
		    <li> Cudd_Deref()
		    <li> Cudd_CheckZeroRef()
		    </ul>
	       Internal procedures included in this module:
		    <ul>
		    <li> cuddReclaim()
		    <li> cuddReclaimZdd()
		    </ul>
	      ]

  SeeAlso     []

  Author      [Fabio Somenzi]

  Copyright [ This file was created at the University of Colorado at
  Boulder.  The University of Colorado at Boulder makes no warranty
  about the suitability of this software for any purpose.  It is
  presented on an AS IS basis.]

******************************************************************************/

/* 10/29/02 REK Adding config.h */
#include <config.h>
#include    <bdd/util.h>
#include    <bdd/cuddInt.h>

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
static char rcsid[] DD_UNUSED = "$Id: cuddRef.c,v 1.1.1.1 2005/04/18 17:40:17 mchu Exp $";
#endif

/*---------------------------------------------------------------------------*/
/* Macro declarations                                                        */
/*---------------------------------------------------------------------------*/


/**AutomaticStart*************************************************************/

/*---------------------------------------------------------------------------*/
/* Static function prototypes                                                */
/*---------------------------------------------------------------------------*/

/**AutomaticEnd***************************************************************/


/*---------------------------------------------------------------------------*/
/* Definition of exported functions                                          */
/*---------------------------------------------------------------------------*/

/**Function********************************************************************

  Synopsis [Increases the reference count of a node, if it is not
  saturated.]

  Description []

  SideEffects [None]

  SeeAlso     [Cudd_RecursiveDeref Cudd_Deref]

******************************************************************************/
void
Cudd_Ref(
  DdNode * n)
{

    n = Cudd_Regular(n);

    cuddSatInc(n->ref);

} /* end of Cudd_Ref */


/**Function********************************************************************

  Synopsis    [Decreases the reference count of node n.]

  Description [Decreases the reference count of node n. If n dies,
  recursively decreases the reference counts of its children.  It is
  used to dispose of a DD that is no longer needed.]

  SideEffects [None]

  SeeAlso     [Cudd_Deref Cudd_Ref Cudd_RecursiveDerefZdd]

******************************************************************************/
void
Cudd_RecursiveDeref(
  DdManager * table,
  DdNode * n)
{
    DdNode *N;
    int ord;
#ifdef __osf__
#pragma pointer_size save
#pragma pointer_size short
#endif
    DdNode **stack = table->stack;
#ifdef __osf__
#pragma pointer_size restore
#endif
    int SP = 1;

    N = Cudd_Regular(n);

    do {
#ifdef DD_DEBUG
	assert(N->ref != 0);
#endif

	if (N->ref == 1) {
	    N->ref = 0;
	    table->dead++;
#ifdef DD_STATS
	    table->nodesDropped++;
#endif
	    if (cuddIsConstant(N)) {
		table->constants.dead++;
		N = stack[--SP];
	    } else {
		ord = table->perm[N->index];
		stack[SP++] = Cudd_Regular(cuddE(N));
		table->subtables[ord].dead++;
		N = cuddT(N);
	    }
	} else {
	    cuddSatDec(N->ref);
	    N = stack[--SP];
	}
    } while (SP != 0);

} /* end of Cudd_RecursiveDeref */


/**Function********************************************************************

  Synopsis    [Decreases the reference count of BDD node n.]

  Description [Decreases the reference count of node n. If n dies,
  recursively decreases the reference counts of its children.  It is
  used to dispose of a BDD that is no longer needed. It is more
  efficient than Cudd_RecursiveDeref, but it cannot be used on
  ADDs. The greater efficiency comes from being able to assume that no
  constant node will ever die as a result of a call to this
  procedure.]

  SideEffects [None]

  SeeAlso     [Cudd_RecursiveDeref]

******************************************************************************/
void
Cudd_IterDerefBdd(
  DdManager * table,
  DdNode * n)
{
    DdNode *N;
    int ord;
#ifdef __osf__
#pragma pointer_size save
#pragma pointer_size short
#endif
    DdNode **stack = table->stack;
#ifdef __osf__
#pragma pointer_size restore
#endif
    int SP = 1;

    N = Cudd_Regular(n);

    do {
#ifdef DD_DEBUG
	assert(N->ref != 0);
#endif

	if (N->ref == 1) {
#ifdef DD_DEBUG
	    assert(!cuddIsConstant(N));
#endif
	    N->ref = 0;
	    table->dead++;
#ifdef DD_STATS
	    table->nodesDropped++;
#endif
	    ord = table->perm[N->index];
	    stack[SP++] = Cudd_Regular(cuddE(N));
	    table->subtables[ord].dead++;
	    N = cuddT(N);
	} else {
	    cuddSatDec(N->ref);
	    N = stack[--SP];
	}
    } while (SP != 0);

} /* end of Cudd_IterDerefBdd */


/**Function********************************************************************

  Synopsis    [Decreases the reference count of ZDD node n.]

  Description [Decreases the reference count of ZDD node n. If n dies,
  recursively decreases the reference counts of its children.  It is
  used to dispose of a ZDD that is no longer needed.]

  SideEffects [None]

  SeeAlso     [Cudd_Deref Cudd_Ref Cudd_RecursiveDeref]

******************************************************************************/
void
Cudd_RecursiveDerefZdd(
  DdManager * table,
  DdNode * n)
{
    DdNode *N;
    int ord;
#ifdef __osf__
#pragma pointer_size save
#pragma pointer_size short
#endif
    DdNode **stack = table->stack;
#ifdef __osf__
#pragma pointer_size restore
#endif
    int SP = 1;

    N = n;

    do {
#ifdef DD_DEBUG
	assert(N->ref != 0);
#endif

	cuddSatDec(N->ref);
    
	if (N->ref == 0) {
	    table->deadZ++;
#ifdef DD_STATS
	    table->nodesDropped++;
#endif
#ifdef DD_DEBUG
	    assert(!cuddIsConstant(N));
#endif
	    ord = table->permZ[N->index];
	    stack[SP++] = cuddE(N);
	    table->subtableZ[ord].dead++;
	    N = cuddT(N);
	} else {
	    N = stack[--SP];
	}
    } while (SP != 0);

} /* end of Cudd_RecursiveDerefZdd */


/**Function********************************************************************

  Synopsis    [Decreases the reference count of node.]

  Description [Decreases the reference count of node. It is primarily
  used in recursive procedures to decrease the ref count of a result
  node before returning it. This accomplishes the goal of removing the
  protection applied by a previous Cudd_Ref.]

  SideEffects [None]

  SeeAlso     [Cudd_RecursiveDeref Cudd_RecursiveDerefZdd Cudd_Ref]

******************************************************************************/
void
Cudd_Deref(
  DdNode * node)
{
    node = Cudd_Regular(node);
    cuddSatDec(node->ref);

} /* end of Cudd_Deref */


/**Function********************************************************************

  Synopsis [Checks the unique table for nodes with non-zero reference
  counts.]

  Description [Checks the unique table for nodes with non-zero
  reference counts. It is normally called before Cudd_Quit to make sure
  that there are no memory leaks due to missing Cudd_RecursiveDeref's.
  Takes into account that reference counts may saturate and that the
  basic constants and the projection functions are referenced by the
  manager.  Returns the number of nodes with non-zero reference count.
  (Except for the cases mentioned above.)]

  SideEffects [None]

  SeeAlso     []

******************************************************************************/
int
Cudd_CheckZeroRef(
  DdManager * manager)
{
    int size;
    int i, j;
    int remain;	/* the expected number of remaining references to one */
#ifdef __osf__
#pragma pointer_size save
#pragma pointer_size short
#endif
    DdNode **nodelist;
    DdNode *node;
    DdSubtable *subtable;
#ifdef __osf__
#pragma pointer_size restore
#endif
    int count = 0;
    int index;

    /* First look at the BDD/ADD subtables. */
    remain = 1; /* reference from the manager */
    size = manager->size;
    remain += 2 * size;	/* reference from the BDD projection functions */

    for (i = 0; i < size; i++) {
	subtable = &(manager->subtables[i]);
	nodelist = subtable->nodelist;
	for (j = 0; (unsigned) j < subtable->slots; j++) {
	    node = nodelist[j];
	    while (node != NULL) {
		if (node->ref != 0 && node->ref != DD_MAXREF) {
		    index = (int) node->index;
		    if (node != manager->vars[index]) {
			count++;
		    } else {
			if (node->ref != 1) {
			    count++;
			}
		    }
		}
		node = node->next;
	    }
	}
    }

    /* Then look at the ZDD subtables. */
    size = manager->sizeZ;
    if (size) /* references from ZDD universe */
	remain += 2;

    for (i = 0; i < size; i++) {
	subtable = &(manager->subtableZ[i]);
	nodelist = subtable->nodelist;
	for (j = 0; (unsigned) j < subtable->slots; j++) {
	    node = nodelist[j];
	    while (node != NULL) {
		if (node->ref != 0 && node->ref != DD_MAXREF) {
		    index = (int) node->index;
		    if (node == manager->univ[manager->permZ[index]]) {
			if (node->ref > 2) {
			    count++;
			}
		    } else {
			count++;
		    }
		}
		node = node->next;
	    }
	}
    }

    /* Now examine the constant table. Plusinfinity, minusinfinity, and
    ** zero are referenced by the manager. One is referenced by the
    ** manager, by the ZDD universe, and by all projection functions.
    ** All other nodes should have no references.
    */
    nodelist = manager->constants.nodelist;
    for (j = 0; (unsigned) j < manager->constants.slots; j++) {
	node = nodelist[j];
	while (node != NULL) {
	    if (node->ref != 0 && node->ref != DD_MAXREF) {
		if (node == manager->one) {
		    if ((int) node->ref != remain) {
			count++;
		    }
		} else if (node == manager->zero ||
		node == manager->plusinfinity ||
		node == manager->minusinfinity) {
		    if (node->ref != 1) {
			count++;
		    }
		} else {
		    count++;
		}
	    }
	    node = node->next;
	}
    }
    return(count);

} /* end of Cudd_CheckZeroRef */


/*---------------------------------------------------------------------------*/
/* Definition of internal functions                                          */
/*---------------------------------------------------------------------------*/


/**Function********************************************************************

  Synopsis    [Brings children of a dead node back.]

  Description []

  SideEffects [None]

  SeeAlso     [cuddReclaimZdd]

******************************************************************************/
void
cuddReclaim(
  DdManager * table,
  DdNode * n)
{
    DdNode *N;
    int ord;
#ifdef __osf__
#pragma pointer_size save
#pragma pointer_size short
#endif
    DdNode **stack = table->stack;
#ifdef __osf__
#pragma pointer_size restore
#endif
    int SP = 1;
    double initialDead = table->dead;

    N = Cudd_Regular(n);

#ifdef DD_DEBUG
    assert(N->ref == 0);
#endif

    do {
	if (N->ref == 0) {
	    N->ref = 1;
	    table->dead--;
	    if (cuddIsConstant(N)) {
		table->constants.dead--;
		N = stack[--SP];
	    } else {
		ord = table->perm[N->index];
		stack[SP++] = Cudd_Regular(cuddE(N));
		table->subtables[ord].dead--;
		N = cuddT(N);
	    }
	} else {
	    cuddSatInc(N->ref);
	    N = stack[--SP];
	}
    } while (SP != 0);

    N = Cudd_Regular(n);
    cuddSatDec(N->ref);
    table->reclaimed += initialDead - table->dead;

} /* end of cuddReclaim */


/**Function********************************************************************

  Synopsis    [Brings children of a dead ZDD node back.]

  Description []

  SideEffects [None]

  SeeAlso     [cuddReclaim]

******************************************************************************/
void
cuddReclaimZdd(
  DdManager * table,
  DdNode * n)
{
    DdNode *N;
    int ord;
#ifdef __osf__
#pragma pointer_size save
#pragma pointer_size short
#endif
    DdNode **stack = table->stack;
#ifdef __osf__
#pragma pointer_size restore
#endif
    int SP = 1;

    N = n;

#ifdef DD_DEBUG
    assert(N->ref == 0);
#endif

    do {
	cuddSatInc(N->ref);

	if (N->ref == 1) {
	    table->deadZ--;
	    table->reclaimed++;
#ifdef DD_DEBUG
	    assert(!cuddIsConstant(N));
#endif
	    ord = table->permZ[N->index];
	    stack[SP++] = cuddE(N);
	    table->subtableZ[ord].dead--;
	    N = cuddT(N);
	} else {
	    N = stack[--SP];
	}
    } while (SP != 0);

    cuddSatDec(n->ref);

} /* end of cuddReclaimZdd */


/*---------------------------------------------------------------------------*/
/* Definition of static functions                                            */
/*---------------------------------------------------------------------------*/
