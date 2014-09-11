/**CFile***********************************************************************

  FileName    [cuddAndAbs.c]

  PackageName [cudd]

  Synopsis    [Combined AND and existential abstraction for BDDs]

  Description [External procedures included in this module:
		<ul>
		<li> Cudd_bddAndAbstract()
		</ul>
	    Internal procedures included in this module:
		<ul>
		<li> cuddBddAndAbstractRecur()
		</ul>]

  Author      [Fabio Somenzi]

  Copyright   [This file was created at the University of Colorado at
  Boulder.  The University of Colorado at Boulder makes no warranty
  about the suitability of this software for any purpose.  It is
  presented on an AS IS basis.]

******************************************************************************/

/* 10/29/02 REK Adding config.h */
#include <config.h>
#include <bdd/util.h>
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
static char rcsid[] DD_UNUSED = "$Id: cuddAndAbs.c,v 1.1.1.1 2005/04/18 17:40:17 mchu Exp $";
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

  Synopsis [Takes the AND of two BDDs and simultaneously abstracts the
  variables in cube.]

  Description [Takes the AND of two BDDs and simultaneously abstracts
  the variables in cube. The variables are existentially abstracted.
  Returns a pointer to the result is successful; NULL otherwise.
  Cudd_bddAndAbstract implements the semiring matrix multiplication
  algorithm for the boolean semiring.]

  SideEffects [None]

  SeeAlso     [Cudd_addMatrixMultiply Cudd_addTriangle Cudd_bddAnd]

******************************************************************************/
DdNode *
Cudd_bddAndAbstract(
  DdManager * manager,
  DdNode * f,
  DdNode * g,
  DdNode * cube)
{
    DdNode *res;

    do {
	manager->reordered = 0;
	res = cuddBddAndAbstractRecur(manager, f, g, cube);
    } while (manager->reordered == 1);
    return(res);

} /* end of Cudd_bddAndAbstract */


/*---------------------------------------------------------------------------*/
/* Definition of internal functions                                          */
/*---------------------------------------------------------------------------*/


/**Function********************************************************************

  Synopsis [Takes the AND of two BDDs and simultaneously abstracts the
  variables in cube.]

  Description [Takes the AND of two BDDs and simultaneously abstracts
  the variables in cube. The variables are existentially abstracted.
  Returns a pointer to the result is successful; NULL otherwise.]

  SideEffects [None]

  SeeAlso     [Cudd_bddAndAbstract]

******************************************************************************/
DdNode *
cuddBddAndAbstractRecur(
  DdManager * manager,
  DdNode * f,
  DdNode * g,
  DdNode * cube)
{
    DdNode *F, *fv, *fnv, *G, *gv, *gnv;
    DdNode *one, *zero, *r, *t, *e, *Cube;
    unsigned int topf, topg, topcube, top, index;

    one = DD_ONE(manager);
    zero = Cudd_Not(one);

    /* Terminal cases. */
    if (f == zero || g == zero || f == Cudd_Not(g)) return(zero);
    if (f == one && g == one)	return(one);

    if (cube == one) {
	return(cuddBddAndRecur(manager, f, g));
    }
    if (f == one || f == g) {
	return(cuddBddExistAbstractRecur(manager, g, cube));
    }
    if (g == one) {
	return(cuddBddExistAbstractRecur(manager, f, cube));
    }
    /* At this point f, g, and cube are not constant. */

    if (f > g) { /* Try to increase cache efficiency. */
	DdNode *tmp = f;
	f = g;
	g = tmp;
    }

    /* Here we can skip the use of cuddI, because the operands are known
    ** to be non-constant.
    */
    F = Cudd_Regular(f);
    G = Cudd_Regular(g);
    topf = manager->perm[F->index];
    topg = manager->perm[G->index];
    top = ddMin(topf, topg);
    topcube = manager->perm[cube->index];

    while (topcube < top) {
	cube = cuddT(cube);
	if (cube == one) {
	    return(cuddBddAndRecur(manager, f, g));
	}
	topcube = manager->perm[cube->index];
    }
    /* Now, topcube >= top. */

    /* Check cache. */
    if (F->ref != 1 || G->ref != 1) {
	r = cuddCacheLookup(manager, Cudd_bddAndAbstract, f, g, cube);
	if (r != NULL) {
	    return(r);
	}
    }

    if (topf == top) {
	index = F->index;
	fv = cuddT(F);
	fnv = cuddE(F);
	if (Cudd_IsComplement(f)) {
	    fv = Cudd_Not(fv);
	    fnv = Cudd_Not(fnv);
	}
    } else {
	index = G->index;
	fv = fnv = f;
    }

    if (topg == top) {
	gv = cuddT(G);
	gnv = cuddE(G);
	if (Cudd_IsComplement(g)) {
	    gv = Cudd_Not(gv);
	    gnv = Cudd_Not(gnv);
	}
    } else {
	gv = gnv = g;
    }

    if (topcube == top) {
	Cube = cuddT(cube);
    } else {
	Cube = cube;
    }

    t = cuddBddAndAbstractRecur(manager, fv, gv, Cube);
    if (t == NULL) return(NULL);

    /* Special case: 1 OR anything = 1. Hence, no need to compute
    ** the else branch if t is 1.
    */
    if (t == one && topcube == top) {
	if (F->ref != 1 || G->ref != 1)
	    cuddCacheInsert(manager, Cudd_bddAndAbstract, f, g, cube, one);
	return(one);
    }
    cuddRef(t);

    e = cuddBddAndAbstractRecur(manager, fnv, gnv, Cube);
    if (e == NULL) {
	Cudd_IterDerefBdd(manager, t);
	return(NULL);
    }
    cuddRef(e);

    if (topcube == top) {	/* abstract */
	r = cuddBddAndRecur(manager, Cudd_Not(t), Cudd_Not(e));
	if (r == NULL) {
	    Cudd_IterDerefBdd(manager, t);
	    Cudd_IterDerefBdd(manager, e);
	    return(NULL);
	}
	r = Cudd_Not(r);
	cuddRef(r);
	Cudd_IterDerefBdd(manager, t);
	Cudd_IterDerefBdd(manager, e);
	cuddDeref(r);
    } else if (t == e) {
	r = t;
	cuddDeref(t);
	cuddDeref(e);
    } else {
	if (Cudd_IsComplement(t)) {
	    r = cuddUniqueInter(manager,(int)index,Cudd_Not(t),Cudd_Not(e));
	    if (r == NULL) {
		Cudd_IterDerefBdd(manager, t);
		Cudd_IterDerefBdd(manager, e);
		return(NULL);
	    }
	    r = Cudd_Not(r);
	} else {
	    r = cuddUniqueInter(manager,(int)index,t,e);
	    if (r == NULL) {
		Cudd_IterDerefBdd(manager, t);
		Cudd_IterDerefBdd(manager, e);
		return(NULL);
	    }
	}
	cuddDeref(e);
	cuddDeref(t);
    }
    if (F->ref != 1 || G->ref != 1)
	cuddCacheInsert(manager, Cudd_bddAndAbstract, f, g, cube, r);
    return (r);

} /* end of cuddBddAndAbstractRecur */


/*---------------------------------------------------------------------------*/
/* Definition of static functions                                            */
/*---------------------------------------------------------------------------*/

