/**CFile***********************************************************************

  FileName    [cuddZddFuncs.c]

  PackageName [cudd]

  Synopsis    [Functions to manipulate covers represented as ZDDs.]

  Description [External procedures included in this module:
		    <ul>
		    <li> Cudd_zddProduct();
		    <li> Cudd_zddUnateProduct();
		    <li> Cudd_zddWeakDiv();
		    <li> Cudd_zddWeakDivF();
		    <li> Cudd_zddDivide();
		    <li> Cudd_zddDivideF();
		    </ul>
	       Internal procedures included in this module:
		    <ul>
		    <li> cuddZddProduct();
		    <li> cuddZddUnateProduct();
		    <li> cuddZddWeakDiv();
		    <li> cuddZddWeakDivF();
		    <li> cuddZddDivide();
		    <li> cuddZddDivideF();
		    <li> cuddZddGetCofactors3()
		    <li> cuddZddGetCofactors2()
		    </ul>
	       Static procedures included in this module:
		    <ul>
		    </ul>
	      ]

  SeeAlso     []

  Author      [In-Ho Moon]

  Copyright [ This file was created at the University of Colorado at
  Boulder.  The University of Colorado at Boulder makes no warranty
  about the suitability of this software for any purpose.  It is
  presented on an AS IS basis.]

******************************************************************************/

/* 10/29/02 REK Adding config.h */
#include <config.h>
#include	<bdd/util.h>
#include	<bdd/cuddInt.h>

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
static char rcsid[] DD_UNUSED = "$Id: cuddZddFuncs.c,v 1.1.1.1 2005/04/18 17:40:17 mchu Exp $";
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

  Synopsis    [Computes the product of two covers represented by ZDDs.]

  Description [Computes the product of two covers represented by
  ZDDs. The result is also a ZDD. Returns a pointer to the result if
  successful; NULL otherwise.  The covers on which Cudd_zddProduct
  operates use two ZDD variables for each function variable (one ZDD
  variable for each literal of the variable). Those two ZDD variables
  should be adjacent in the order.]

  SideEffects [None]

  SeeAlso     [Cudd_zddUnateProduct]

******************************************************************************/
DdNode	*
Cudd_zddProduct(
  DdManager * dd,
  DdNode * f,
  DdNode * g)
{
    DdNode	*res;

    do {
	dd->reordered = 0;
	res = cuddZddProduct(dd, f, g);
    } while (dd->reordered == 1);
    return(res);

} /* end of Cudd_zddProduct */


/**Function********************************************************************

  Synopsis [Computes the product of two unate covers.]

  Description [Computes the product of two unate covers represented as
  ZDDs. Unate covers use one ZDD variable for each BDD
  variable. Returns a pointer to the result if successful; NULL
  otherwise.]

  SideEffects [None]

  SeeAlso     [Cudd_zddProduct]

******************************************************************************/
DdNode	*
Cudd_zddUnateProduct(
  DdManager * dd,
  DdNode * f,
  DdNode * g)
{
    DdNode	*res;

    do {
	dd->reordered = 0;
	res = cuddZddUnateProduct(dd, f, g);
    } while (dd->reordered == 1);
    return(res);

} /* end of Cudd_zddUnateProduct */


/**Function********************************************************************

  Synopsis    [Applies weak division to two covers.]

  Description [Applies weak division to two ZDDs representing two
  covers. Returns a pointer to the ZDD representing the result if
  successful; NULL otherwise. The result of weak division depends on
  the variable order. The covers on which Cudd_zddWeakDiv operates use
  two ZDD variables for each function variable (one ZDD variable for
  each literal of the variable). Those two ZDD variables should be
  adjacent in the order.]

  SideEffects [None]

  SeeAlso     [Cudd_zddDivide]

******************************************************************************/
DdNode	*
Cudd_zddWeakDiv(
  DdManager * dd,
  DdNode * f,
  DdNode * g)
{
    DdNode	*res;

    do {
	dd->reordered = 0;
	res = cuddZddWeakDiv(dd, f, g);
    } while (dd->reordered == 1);
    return(res);

} /* end of Cudd_zddWeakDiv */


/**Function********************************************************************

  Synopsis    [Computes the quotient of two unate covers.]

  Description [Computes the quotient of two unate covers represented
  by ZDDs.  Unate covers use one ZDD variable for each BDD
  variable. Returns a pointer to the resulting ZDD if successful; NULL
  otherwise.]

  SideEffects [None]

  SeeAlso     [Cudd_zddWeakDiv]

******************************************************************************/
DdNode	*
Cudd_zddDivide(
  DdManager * dd,
  DdNode * f,
  DdNode * g)
{
    DdNode	*res;

    do {
	dd->reordered = 0;
	res = cuddZddDivide(dd, f, g);
    } while (dd->reordered == 1);
    return(res);

} /* end of Cudd_zddDivide */


/**Function********************************************************************

  Synopsis    [Modified version of Cudd_zddWeakDiv.]

  Description [Modified version of Cudd_zddWeakDiv. This function may
  disappear in future releases.]

  SideEffects [None]

  SeeAlso     [Cudd_zddWeakDiv]

******************************************************************************/
DdNode	*
Cudd_zddWeakDivF(
  DdManager * dd,
  DdNode * f,
  DdNode * g)
{
    DdNode	*res;

    do {
	dd->reordered = 0;
	res = cuddZddWeakDivF(dd, f, g);
    } while (dd->reordered == 1);
    return(res);

} /* end of Cudd_zddWeakDivF */


/**Function********************************************************************

  Synopsis    [Modified version of Cudd_zddDivide.]

  Description [Modified version of Cudd_zddDivide. This function may
  disappear in future releases.]

  SideEffects [None]

  SeeAlso     []

******************************************************************************/
DdNode	*
Cudd_zddDivideF(
  DdManager * dd,
  DdNode * f,
  DdNode * g)
{
    DdNode	*res;

    do {
	dd->reordered = 0;
	res = cuddZddDivideF(dd, f, g);
    } while (dd->reordered == 1);
    return(res);

} /* end of Cudd_zddDivideF */


/*---------------------------------------------------------------------------*/
/* Definition of internal functions                                          */
/*---------------------------------------------------------------------------*/


/**Function********************************************************************

  Synopsis [Performs the recursive step of Cudd_zddProduct.]

  Description []

  SideEffects [None]

  SeeAlso     [Cudd_zddProduct]

******************************************************************************/
DdNode	*
cuddZddProduct(
  DdManager * dd,
  DdNode * f,
  DdNode * g)
{
    int		v, top_f, top_g;
    DdNode	*tmp, *term1, *term2, *term3;
    DdNode	*f0, *f1, *fd, *g0, *g1, *gd;
    DdNode	*R0, *R1, *Rd, *N0, *N1;
    DdNode	*r;
    DdNode	*one = DD_ONE(dd);
    DdNode	*zero = DD_ZERO(dd);
    int		flag;

    if (f == zero || g == zero)
        return(zero);
    if (f == one)
        return(g);
    if (g == one)
        return(f);

    top_f = dd->permZ[Cudd_Regular(f)->index];
    top_g = dd->permZ[Cudd_Regular(g)->index];

    if (top_f > top_g)
	return(cuddZddProduct(dd, g, f));

    /* Check cache */
    r = cuddCacheLookup2Zdd(dd, cuddZddProduct, f, g);
    if (r)
	return(r);

    v = Cudd_Regular(f)->index;	/* either yi or zi */
    flag = cuddZddGetCofactors3(dd, f, v, &f1, &f0, &fd);
    if (flag == 1)
	return(NULL);
    Cudd_Ref(f1);
    Cudd_Ref(f0);
    Cudd_Ref(fd);
    flag = cuddZddGetCofactors3(dd, g, v, &g1, &g0, &gd);
    if (flag == 1) {
	Cudd_RecursiveDerefZdd(dd, f1);
	Cudd_RecursiveDerefZdd(dd, f0);
	Cudd_RecursiveDerefZdd(dd, fd);
	return(NULL);
    }
    Cudd_Ref(g1);
    Cudd_Ref(g0);
    Cudd_Ref(gd);
    v = (v >> 1) << 1;

    Rd = cuddZddProduct(dd, fd, gd);
    if (Rd == NULL) {
	Cudd_RecursiveDerefZdd(dd, f1);
	Cudd_RecursiveDerefZdd(dd, f0);
	Cudd_RecursiveDerefZdd(dd, fd);
	Cudd_RecursiveDerefZdd(dd, g1);
	Cudd_RecursiveDerefZdd(dd, g0);
	Cudd_RecursiveDerefZdd(dd, gd);
	return(NULL);
    }
    Cudd_Ref(Rd);

    term1 = cuddZddProduct(dd, f0, g0);
    if (term1 == NULL) {
	Cudd_RecursiveDerefZdd(dd, f1);
	Cudd_RecursiveDerefZdd(dd, f0);
	Cudd_RecursiveDerefZdd(dd, fd);
	Cudd_RecursiveDerefZdd(dd, g1);
	Cudd_RecursiveDerefZdd(dd, g0);
	Cudd_RecursiveDerefZdd(dd, gd);
	Cudd_RecursiveDerefZdd(dd, Rd);
	return(NULL);
    }
    Cudd_Ref(term1);
    term2 = cuddZddProduct(dd, f0, gd);
    if (term2 == NULL) {
	Cudd_RecursiveDerefZdd(dd, f1);
	Cudd_RecursiveDerefZdd(dd, f0);
	Cudd_RecursiveDerefZdd(dd, fd);
	Cudd_RecursiveDerefZdd(dd, g1);
	Cudd_RecursiveDerefZdd(dd, g0);
	Cudd_RecursiveDerefZdd(dd, gd);
	Cudd_RecursiveDerefZdd(dd, Rd);
	Cudd_RecursiveDerefZdd(dd, term1);
	return(NULL);
    }
    Cudd_Ref(term2);
    term3 = cuddZddProduct(dd, fd, g0);
    if (term3 == NULL) {
	Cudd_RecursiveDerefZdd(dd, f1);
	Cudd_RecursiveDerefZdd(dd, f0);
	Cudd_RecursiveDerefZdd(dd, fd);
	Cudd_RecursiveDerefZdd(dd, g1);
	Cudd_RecursiveDerefZdd(dd, g0);
	Cudd_RecursiveDerefZdd(dd, gd);
	Cudd_RecursiveDerefZdd(dd, Rd);
	Cudd_RecursiveDerefZdd(dd, term1);
	Cudd_RecursiveDerefZdd(dd, term2);
	return(NULL);
    }
    Cudd_Ref(term3);
    Cudd_RecursiveDerefZdd(dd, f0);
    Cudd_RecursiveDerefZdd(dd, g0);
    tmp = cuddZddUnion(dd, term1, term2);
    if (tmp == NULL) {
	Cudd_RecursiveDerefZdd(dd, f1);
	Cudd_RecursiveDerefZdd(dd, fd);
	Cudd_RecursiveDerefZdd(dd, g1);
	Cudd_RecursiveDerefZdd(dd, gd);
	Cudd_RecursiveDerefZdd(dd, Rd);
	Cudd_RecursiveDerefZdd(dd, term1);
	Cudd_RecursiveDerefZdd(dd, term2);
	Cudd_RecursiveDerefZdd(dd, term3);
	return(NULL);
    }
    Cudd_Ref(tmp);
    Cudd_RecursiveDerefZdd(dd, term1);
    Cudd_RecursiveDerefZdd(dd, term2);
    R0 = cuddZddUnion(dd, tmp, term3);
    if (R0 == NULL) {
	Cudd_RecursiveDerefZdd(dd, f1);
	Cudd_RecursiveDerefZdd(dd, fd);
	Cudd_RecursiveDerefZdd(dd, g1);
	Cudd_RecursiveDerefZdd(dd, gd);
	Cudd_RecursiveDerefZdd(dd, Rd);
	Cudd_RecursiveDerefZdd(dd, term3);
	Cudd_RecursiveDerefZdd(dd, tmp);
	return(NULL);
    }
    Cudd_Ref(R0);
    Cudd_RecursiveDerefZdd(dd, tmp);
    Cudd_RecursiveDerefZdd(dd, term3);
    N0 = cuddZddGetNode(dd, v + 1, R0, Rd); /* v = zi */
    if (N0 == NULL) {
	Cudd_RecursiveDerefZdd(dd, f1);
	Cudd_RecursiveDerefZdd(dd, fd);
	Cudd_RecursiveDerefZdd(dd, g1);
	Cudd_RecursiveDerefZdd(dd, gd);
	Cudd_RecursiveDerefZdd(dd, Rd);
	Cudd_RecursiveDerefZdd(dd, R0);
	return(NULL);
    }
    Cudd_Ref(N0);
    Cudd_RecursiveDerefZdd(dd, R0);
    Cudd_RecursiveDerefZdd(dd, Rd);

    term1 = cuddZddProduct(dd, f1, g1);
    if (term1 == NULL) {
	Cudd_RecursiveDerefZdd(dd, f1);
	Cudd_RecursiveDerefZdd(dd, fd);
	Cudd_RecursiveDerefZdd(dd, g1);
	Cudd_RecursiveDerefZdd(dd, gd);
	Cudd_RecursiveDerefZdd(dd, N0);
	return(NULL);
    }
    Cudd_Ref(term1);
    term2 = cuddZddProduct(dd, f1, gd);
    if (term2 == NULL) {
	Cudd_RecursiveDerefZdd(dd, f1);
	Cudd_RecursiveDerefZdd(dd, fd);
	Cudd_RecursiveDerefZdd(dd, g1);
	Cudd_RecursiveDerefZdd(dd, gd);
	Cudd_RecursiveDerefZdd(dd, N0);
	Cudd_RecursiveDerefZdd(dd, term1);
	return(NULL);
    }
    Cudd_Ref(term2);
    term3 = cuddZddProduct(dd, fd, g1);
    if (term3 == NULL) {
	Cudd_RecursiveDerefZdd(dd, f1);
	Cudd_RecursiveDerefZdd(dd, fd);
	Cudd_RecursiveDerefZdd(dd, g1);
	Cudd_RecursiveDerefZdd(dd, gd);
	Cudd_RecursiveDerefZdd(dd, N0);
	Cudd_RecursiveDerefZdd(dd, term1);
	Cudd_RecursiveDerefZdd(dd, term2);
	return(NULL);
    }
    Cudd_Ref(term3);
    Cudd_RecursiveDerefZdd(dd, f1);
    Cudd_RecursiveDerefZdd(dd, g1);
    Cudd_RecursiveDerefZdd(dd, fd);
    Cudd_RecursiveDerefZdd(dd, gd);
    tmp = cuddZddUnion(dd, term1, term2);
    if (tmp == NULL) {
	Cudd_RecursiveDerefZdd(dd, N0);
	Cudd_RecursiveDerefZdd(dd, term1);
	Cudd_RecursiveDerefZdd(dd, term2);
	Cudd_RecursiveDerefZdd(dd, term3);
	return(NULL);
    }
    Cudd_Ref(tmp);
    Cudd_RecursiveDerefZdd(dd, term1);
    Cudd_RecursiveDerefZdd(dd, term2);
    R1 = cuddZddUnion(dd, tmp, term3);
    if (R1 == NULL) {
	Cudd_RecursiveDerefZdd(dd, N0);
	Cudd_RecursiveDerefZdd(dd, term3);
	Cudd_RecursiveDerefZdd(dd, tmp);
	return(NULL);
    }
    Cudd_Ref(R1);
    Cudd_RecursiveDerefZdd(dd, tmp);
    Cudd_RecursiveDerefZdd(dd, term3);
    N1 = cuddZddGetNode(dd, v, R1, N0); /* v = yi */
    if (N1 == NULL) {
	Cudd_RecursiveDerefZdd(dd, N0);
	Cudd_RecursiveDerefZdd(dd, R1);
	return(NULL);
    }
    Cudd_Ref(N1);
    Cudd_RecursiveDerefZdd(dd, R1);
    Cudd_RecursiveDerefZdd(dd, N0);

    cuddCacheInsert2(dd, cuddZddProduct, f, g, N1);
    Cudd_Deref(N1);
    return(N1);

} /* end of cuddZddProduct */


/**Function********************************************************************

  Synopsis    [Performs the recursive step of Cudd_zddUnateProduct.]

  Description []

  SideEffects [None]

  SeeAlso     [Cudd_zddUnateProduct]

******************************************************************************/
DdNode	*
cuddZddUnateProduct(
  DdManager * dd,
  DdNode * f,
  DdNode * g)
{
    int		v, top_f, top_g;
    DdNode	*term1, *term2, *term3, *term4;
    DdNode	*sum1, *sum2;
    DdNode	*f0, *f1, *g0, *g1;
    DdNode	*r;
    DdNode	*one = DD_ONE(dd);
    DdNode	*zero = DD_ZERO(dd);
    int		flag;

    if (f == zero || g == zero)
        return(zero);
    if (f == one)
        return(g);
    if (g == one)
        return(f);

    top_f = dd->permZ[Cudd_Regular(f)->index];
    top_g = dd->permZ[Cudd_Regular(g)->index];

    if (top_f > top_g)
	return(cuddZddUnateProduct(dd, g, f));

    /* Check cache */
    r = cuddCacheLookup2Zdd(dd, cuddZddUnateProduct, f, g);
    if (r)
	return(r);

    v = Cudd_Regular(f)->index;	/* either yi or zi */
    flag = cuddZddGetCofactors2(dd, f, v, &f1, &f0);
    if (flag == 1)
	return(NULL);
    Cudd_Ref(f1);
    Cudd_Ref(f0);
    flag = cuddZddGetCofactors2(dd, g, v, &g1, &g0);
    if (flag == 1) {
	Cudd_RecursiveDerefZdd(dd, f1);
	Cudd_RecursiveDerefZdd(dd, f0);
	return(NULL);
    }
    Cudd_Ref(g1);
    Cudd_Ref(g0);

    term1 = cuddZddUnateProduct(dd, f1, g1);
    if (term1 == NULL) {
	Cudd_RecursiveDerefZdd(dd, f1);
	Cudd_RecursiveDerefZdd(dd, f0);
	Cudd_RecursiveDerefZdd(dd, g1);
	Cudd_RecursiveDerefZdd(dd, g0);
	return(NULL);
    }
    Cudd_Ref(term1);
    term2 = cuddZddUnateProduct(dd, f1, g0);
    if (term2 == NULL) {
	Cudd_RecursiveDerefZdd(dd, f1);
	Cudd_RecursiveDerefZdd(dd, f0);
	Cudd_RecursiveDerefZdd(dd, g1);
	Cudd_RecursiveDerefZdd(dd, g0);
	Cudd_RecursiveDerefZdd(dd, term1);
	return(NULL);
    }
    Cudd_Ref(term2);
    term3 = cuddZddUnateProduct(dd, f0, g1);
    if (term3 == NULL) {
	Cudd_RecursiveDerefZdd(dd, f1);
	Cudd_RecursiveDerefZdd(dd, f0);
	Cudd_RecursiveDerefZdd(dd, g1);
	Cudd_RecursiveDerefZdd(dd, g0);
	Cudd_RecursiveDerefZdd(dd, term1);
	Cudd_RecursiveDerefZdd(dd, term2);
	return(NULL);
    }
    Cudd_Ref(term3);
    term4 = cuddZddUnateProduct(dd, f0, g0);
    if (term4 == NULL) {
	Cudd_RecursiveDerefZdd(dd, f1);
	Cudd_RecursiveDerefZdd(dd, f0);
	Cudd_RecursiveDerefZdd(dd, g1);
	Cudd_RecursiveDerefZdd(dd, g0);
	Cudd_RecursiveDerefZdd(dd, term1);
	Cudd_RecursiveDerefZdd(dd, term2);
	Cudd_RecursiveDerefZdd(dd, term3);
	return(NULL);
    }
    Cudd_Ref(term4);
    Cudd_RecursiveDerefZdd(dd, f1);
    Cudd_RecursiveDerefZdd(dd, f0);
    Cudd_RecursiveDerefZdd(dd, g1);
    Cudd_RecursiveDerefZdd(dd, g0);
    sum1 = cuddZddUnion(dd, term1, term2);
    if (sum1 == NULL) {
	Cudd_RecursiveDerefZdd(dd, term1);
	Cudd_RecursiveDerefZdd(dd, term2);
	Cudd_RecursiveDerefZdd(dd, term3);
	Cudd_RecursiveDerefZdd(dd, term4);
	return(NULL);
    }
    Cudd_Ref(sum1);
    Cudd_RecursiveDerefZdd(dd, term1);
    Cudd_RecursiveDerefZdd(dd, term2);
    sum2 = cuddZddUnion(dd, sum1, term3);
    if (sum2 == NULL) {
	Cudd_RecursiveDerefZdd(dd, term3);
	Cudd_RecursiveDerefZdd(dd, term4);
	Cudd_RecursiveDerefZdd(dd, sum1);
	return(NULL);
    }
    Cudd_Ref(sum2);
    Cudd_RecursiveDerefZdd(dd, sum1);
    Cudd_RecursiveDerefZdd(dd, term3);
    r = cuddZddGetNode(dd, v, sum2, term4);
    if (r == NULL) {
	Cudd_RecursiveDerefZdd(dd, term4);
	Cudd_RecursiveDerefZdd(dd, sum2);
	return(NULL);
    }
    Cudd_Ref(r);
    Cudd_RecursiveDerefZdd(dd, sum2);
    Cudd_RecursiveDerefZdd(dd, term4);

    cuddCacheInsert2(dd, cuddZddUnateProduct, f, g, r);
    Cudd_Deref(r);
    return(r);

} /* end of cuddZddUnateProduct */


/**Function********************************************************************

  Synopsis    [Performs the recursive step of Cudd_zddWeakDiv.]

  Description []

  SideEffects [None]

  SeeAlso     [Cudd_zddWeakDiv]

******************************************************************************/
DdNode	*
cuddZddWeakDiv(
  DdManager * dd,
  DdNode * f,
  DdNode * g)
{
    int		v;
    DdNode	*one = DD_ONE(dd);
    DdNode	*zero = DD_ZERO(dd);
    DdNode	*f0, *f1, *fd, *g0, *g1, *gd;
    DdNode	*q, *tmp;
    DdNode	*r;
    int		flag;

    if (g == one)
	return(f);
    if (f == zero || f == one)
	return(zero);
    if (f == g)
	return(one);

    /* Check cache. */
    r = cuddCacheLookup2Zdd(dd, cuddZddWeakDiv, f, g);
    if (r)
	return(r);

    v = Cudd_Regular(g)->index;

    flag = cuddZddGetCofactors3(dd, f, v, &f1, &f0, &fd);
    if (flag == 1)
	return(NULL);
    Cudd_Ref(f1);
    Cudd_Ref(f0);
    Cudd_Ref(fd);
    flag = cuddZddGetCofactors3(dd, g, v, &g1, &g0, &gd);
    if (flag == 1) {
	Cudd_RecursiveDerefZdd(dd, f1);
	Cudd_RecursiveDerefZdd(dd, f0);
	Cudd_RecursiveDerefZdd(dd, fd);
	return(NULL);
    }
    Cudd_Ref(g1);
    Cudd_Ref(g0);
    Cudd_Ref(gd);

    q = g;

    if (g0 != zero) {
	q = cuddZddWeakDiv(dd, f0, g0);
	if (q == NULL) {
	    Cudd_RecursiveDerefZdd(dd, f1);
	    Cudd_RecursiveDerefZdd(dd, f0);
	    Cudd_RecursiveDerefZdd(dd, fd);
	    Cudd_RecursiveDerefZdd(dd, g1);
	    Cudd_RecursiveDerefZdd(dd, g0);
	    Cudd_RecursiveDerefZdd(dd, gd);
	    return(NULL);
	}
	Cudd_Ref(q);
    }
    else
	Cudd_Ref(q);
    Cudd_RecursiveDerefZdd(dd, f0);
    Cudd_RecursiveDerefZdd(dd, g0);

    if (q == zero) {
	Cudd_RecursiveDerefZdd(dd, f1);
	Cudd_RecursiveDerefZdd(dd, g1);
	Cudd_RecursiveDerefZdd(dd, fd);
	Cudd_RecursiveDerefZdd(dd, gd);
	cuddCacheInsert2(dd, cuddZddWeakDiv, f, g, zero);
	Cudd_Deref(q);
	return(zero);
    }

    if (g1 != zero) {
	Cudd_RecursiveDerefZdd(dd, q);
	tmp = cuddZddWeakDiv(dd, f1, g1);
	if (tmp == NULL) {
	    Cudd_RecursiveDerefZdd(dd, f1);
	    Cudd_RecursiveDerefZdd(dd, g1);
	    Cudd_RecursiveDerefZdd(dd, fd);
	    Cudd_RecursiveDerefZdd(dd, gd);
	    return(NULL);
	}
	Cudd_Ref(tmp);
	Cudd_RecursiveDerefZdd(dd, f1);
	Cudd_RecursiveDerefZdd(dd, g1);
	if (q == g)
	    q = tmp;
	else {
	    q = cuddZddIntersect(dd, q, tmp);
	    if (q == NULL) {
		Cudd_RecursiveDerefZdd(dd, fd);
		Cudd_RecursiveDerefZdd(dd, gd);
		return(NULL);
	    }
	    Cudd_Ref(q);
	    Cudd_RecursiveDerefZdd(dd, tmp);
	}
    }
    else {
	Cudd_RecursiveDerefZdd(dd, f1);
	Cudd_RecursiveDerefZdd(dd, g1);
    }

    if (q == zero) {
	Cudd_RecursiveDerefZdd(dd, fd);
	Cudd_RecursiveDerefZdd(dd, gd);
	cuddCacheInsert2(dd, cuddZddWeakDiv, f, g, zero);
	Cudd_Deref(q);
	return(zero);
    }

    if (gd != zero) {
	Cudd_RecursiveDerefZdd(dd, q);
	tmp = cuddZddWeakDiv(dd, fd, gd);
	if (tmp == NULL) {
	    Cudd_RecursiveDerefZdd(dd, fd);
	    Cudd_RecursiveDerefZdd(dd, gd);
	    return(NULL);
	}
	Cudd_Ref(tmp);
	Cudd_RecursiveDerefZdd(dd, fd);
	Cudd_RecursiveDerefZdd(dd, gd);
	if (q == g)
	    q = tmp;
	else {
	    q = cuddZddIntersect(dd, q, tmp);
	    if (q == NULL) {
		Cudd_RecursiveDerefZdd(dd, tmp);
		return(NULL);
	    }
	    Cudd_Ref(q);
	    Cudd_RecursiveDerefZdd(dd, tmp);
	}
    }
    else {
	Cudd_RecursiveDerefZdd(dd, fd);
	Cudd_RecursiveDerefZdd(dd, gd);
    }

    cuddCacheInsert2(dd, cuddZddWeakDiv, f, g, q);
    Cudd_Deref(q);
    return(q);

} /* end of cuddZddWeakDiv */


/**Function********************************************************************

  Synopsis    [Performs the recursive step of Cudd_zddWeakDivF.]

  Description []

  SideEffects [None]

  SeeAlso     [Cudd_zddWeakDivF]

******************************************************************************/
DdNode	*
cuddZddWeakDivF(
  DdManager * dd,
  DdNode * f,
  DdNode * g)
{
    int		v, top_f, top_g, vf, vg;
    DdNode	*one = DD_ONE(dd);
    DdNode	*zero = DD_ZERO(dd);
    DdNode	*f0, *f1, *fd, *g0, *g1, *gd;
    DdNode	*q, *tmp;
    DdNode	*r;
    DdNode	*term1, *term0, *termd;
    int		flag;

    if (g == one)
	return(f);
    if (f == zero || f == one)
	return(zero);
    if (f == g)
	return(one);

    /* Check cache. */
    r = cuddCacheLookup2Zdd(dd, cuddZddWeakDivF, f, g);
    if (r)
	return(r);

    top_f = dd->permZ[Cudd_Regular(f)->index];
    top_g = dd->permZ[Cudd_Regular(g)->index];
    vf = top_f >> 1;
    vg = top_g >> 1;
    v = ddMin(top_f, top_g);

    if (v == top_f && vf < vg) {
	v = Cudd_Regular(f)->index;
	flag = cuddZddGetCofactors3(dd, f, v, &f1, &f0, &fd);
	if (flag == 1)
	    return(NULL);
	Cudd_Ref(f1);
	Cudd_Ref(f0);
	Cudd_Ref(fd);

	term1 = cuddZddWeakDivF(dd, f1, g);
	if (term1 == NULL) {
	    Cudd_RecursiveDerefZdd(dd, f1);
	    Cudd_RecursiveDerefZdd(dd, f0);
	    Cudd_RecursiveDerefZdd(dd, fd);
	    return(NULL);
	}
	Cudd_Ref(term1);
	Cudd_RecursiveDerefZdd(dd, f1);
	term0 = cuddZddWeakDivF(dd, f0, g);
	if (term0 == NULL) {
	    Cudd_RecursiveDerefZdd(dd, f0);
	    Cudd_RecursiveDerefZdd(dd, fd);
	    Cudd_RecursiveDerefZdd(dd, term1);
	    return(NULL);
	}
	Cudd_Ref(term0);
	Cudd_RecursiveDerefZdd(dd, f0);
	termd = cuddZddWeakDivF(dd, fd, g);
	if (termd == NULL) {
	    Cudd_RecursiveDerefZdd(dd, fd);
	    Cudd_RecursiveDerefZdd(dd, term1);
	    Cudd_RecursiveDerefZdd(dd, term0);
	    return(NULL);
	}
	Cudd_Ref(termd);
	Cudd_RecursiveDerefZdd(dd, fd);

	tmp = cuddZddGetNode(dd, v + 1, term0, termd);
	if (tmp == NULL) {
	    Cudd_RecursiveDerefZdd(dd, term1);
	    Cudd_RecursiveDerefZdd(dd, term0);
	    Cudd_RecursiveDerefZdd(dd, termd);
	    return(NULL);
	}
	Cudd_Ref(tmp);
	Cudd_RecursiveDerefZdd(dd, term0);
	Cudd_RecursiveDerefZdd(dd, termd);
	q = cuddZddGetNode(dd, v, term1, tmp);
	if (q == NULL) {
	    Cudd_RecursiveDerefZdd(dd, term1);
	    Cudd_RecursiveDerefZdd(dd, tmp);
	    return(NULL);
	}
	Cudd_Ref(q);
	Cudd_RecursiveDerefZdd(dd, term1);
	Cudd_RecursiveDerefZdd(dd, tmp);

	cuddCacheInsert2(dd, cuddZddWeakDivF, f, g, q);
	Cudd_Deref(q);
	return(q);
    }

    if (v == top_f)
	v = Cudd_Regular(f)->index;
    else
	v = Cudd_Regular(g)->index;

    flag = cuddZddGetCofactors3(dd, f, v, &f1, &f0, &fd);
    if (flag == 1)
	return(NULL);
    Cudd_Ref(f1);
    Cudd_Ref(f0);
    Cudd_Ref(fd);
    flag = cuddZddGetCofactors3(dd, g, v, &g1, &g0, &gd);
    if (flag == 1) {
	Cudd_RecursiveDerefZdd(dd, f1);
	Cudd_RecursiveDerefZdd(dd, f0);
	Cudd_RecursiveDerefZdd(dd, fd);
	return(NULL);
    }
    Cudd_Ref(g1);
    Cudd_Ref(g0);
    Cudd_Ref(gd);

    q = g;

    if (g0 != zero) {
	q = cuddZddWeakDivF(dd, f0, g0);
	if (q == NULL) {
	    Cudd_RecursiveDerefZdd(dd, f1);
	    Cudd_RecursiveDerefZdd(dd, f0);
	    Cudd_RecursiveDerefZdd(dd, fd);
	    Cudd_RecursiveDerefZdd(dd, g1);
	    Cudd_RecursiveDerefZdd(dd, g0);
	    Cudd_RecursiveDerefZdd(dd, gd);
	    return(NULL);
	}
	Cudd_Ref(q);
    }
    else
	Cudd_Ref(q);
    Cudd_RecursiveDerefZdd(dd, f0);
    Cudd_RecursiveDerefZdd(dd, g0);

    if (q == zero) {
	Cudd_RecursiveDerefZdd(dd, f1);
	Cudd_RecursiveDerefZdd(dd, g1);
	Cudd_RecursiveDerefZdd(dd, fd);
	Cudd_RecursiveDerefZdd(dd, gd);
	cuddCacheInsert2(dd, cuddZddWeakDivF, f, g, zero);
	Cudd_Deref(q);
	return(zero);
    }

    if (g1 != zero) {
	Cudd_RecursiveDerefZdd(dd, q);
	tmp = cuddZddWeakDivF(dd, f1, g1);
	if (tmp == NULL) {
	    Cudd_RecursiveDerefZdd(dd, f1);
	    Cudd_RecursiveDerefZdd(dd, g1);
	    Cudd_RecursiveDerefZdd(dd, fd);
	    Cudd_RecursiveDerefZdd(dd, gd);
	    return(NULL);
	}
	Cudd_Ref(tmp);
	Cudd_RecursiveDerefZdd(dd, f1);
	Cudd_RecursiveDerefZdd(dd, g1);
	if (q == g)
	    q = tmp;
	else {
	    q = cuddZddIntersect(dd, q, tmp);
	    if (q == NULL) {
		Cudd_RecursiveDerefZdd(dd, fd);
		Cudd_RecursiveDerefZdd(dd, gd);
		return(NULL);
	    }
	    Cudd_Ref(q);
	    Cudd_RecursiveDerefZdd(dd, tmp);
	}
    }
    else {
	Cudd_RecursiveDerefZdd(dd, f1);
	Cudd_RecursiveDerefZdd(dd, g1);
    }

    if (q == zero) {
	Cudd_RecursiveDerefZdd(dd, fd);
	Cudd_RecursiveDerefZdd(dd, gd);
	cuddCacheInsert2(dd, cuddZddWeakDivF, f, g, zero);
	Cudd_Deref(q);
	return(zero);
    }

    if (gd != zero) {
	Cudd_RecursiveDerefZdd(dd, q);
	tmp = cuddZddWeakDivF(dd, fd, gd);
	if (tmp == NULL) {
	    Cudd_RecursiveDerefZdd(dd, fd);
	    Cudd_RecursiveDerefZdd(dd, gd);
	    return(NULL);
	}
	Cudd_Ref(tmp);
	Cudd_RecursiveDerefZdd(dd, fd);
	Cudd_RecursiveDerefZdd(dd, gd);
	if (q == g)
	    q = tmp;
	else {
	    q = cuddZddIntersect(dd, q, tmp);
	    if (q == NULL) {
		Cudd_RecursiveDerefZdd(dd, tmp);
		return(NULL);
	    }
	    Cudd_Ref(q);
	    Cudd_RecursiveDerefZdd(dd, tmp);
	}
    }
    else {
	Cudd_RecursiveDerefZdd(dd, fd);
	Cudd_RecursiveDerefZdd(dd, gd);
    }

    cuddCacheInsert2(dd, cuddZddWeakDivF, f, g, q);
    Cudd_Deref(q);
    return(q);

} /* end of cuddZddWeakDivF */


/**Function********************************************************************

  Synopsis    [Performs the recursive step of Cudd_zddDivide.]

  Description []

  SideEffects [None]

  SeeAlso     [Cudd_zddDivide]

******************************************************************************/
DdNode	*
cuddZddDivide(
  DdManager * dd,
  DdNode * f,
  DdNode * g)
{
    int		v;
    DdNode	*one = DD_ONE(dd);
    DdNode	*zero = DD_ZERO(dd);
    DdNode	*f0, *f1, *g0, *g1;
    DdNode	*q, *r, *tmp;
    int		flag;

    if (g == one)
	return(f);
    if (f == zero || f == one)
	return(zero);
    if (f == g)
	return(one);

    /* Check cache. */
    r = cuddCacheLookup2Zdd(dd, cuddZddDivide, f, g);
    if (r)
	return(r);

    v = Cudd_Regular(g)->index;

    flag = cuddZddGetCofactors2(dd, f, v, &f1, &f0);
    if (flag == 1)
	return(NULL);
    Cudd_Ref(f1);
    Cudd_Ref(f0);
    flag = cuddZddGetCofactors2(dd, g, v, &g1, &g0);	/* g1 != zero */
    if (flag == 1) {
	Cudd_RecursiveDerefZdd(dd, f1);
	Cudd_RecursiveDerefZdd(dd, f0);
	return(NULL);
    }
    Cudd_Ref(g1);
    Cudd_Ref(g0);

    r = cuddZddDivide(dd, f1, g1);
    if (r == NULL) {
	Cudd_RecursiveDerefZdd(dd, f1);
	Cudd_RecursiveDerefZdd(dd, f0);
	Cudd_RecursiveDerefZdd(dd, g1);
	Cudd_RecursiveDerefZdd(dd, g0);
	return(NULL);
    }
    Cudd_Ref(r);

    if (r != zero && g0 != zero) {
	tmp = r;
	q = cuddZddDivide(dd, f0, g0);
	if (q == NULL) {
	    Cudd_RecursiveDerefZdd(dd, f1);
	    Cudd_RecursiveDerefZdd(dd, f0);
	    Cudd_RecursiveDerefZdd(dd, g1);
	    Cudd_RecursiveDerefZdd(dd, g0);
	    return(NULL);
	}
	Cudd_Ref(q);
	r = cuddZddIntersect(dd, r, q);
	if (r == NULL) {
	    Cudd_RecursiveDerefZdd(dd, f1);
	    Cudd_RecursiveDerefZdd(dd, f0);
	    Cudd_RecursiveDerefZdd(dd, g1);
	    Cudd_RecursiveDerefZdd(dd, g0);
	    Cudd_RecursiveDerefZdd(dd, q);
	    return(NULL);
	}
	Cudd_Ref(r);
	Cudd_RecursiveDerefZdd(dd, q);
	Cudd_RecursiveDerefZdd(dd, tmp);
    }

    Cudd_RecursiveDerefZdd(dd, f1);
    Cudd_RecursiveDerefZdd(dd, f0);
    Cudd_RecursiveDerefZdd(dd, g1);
    Cudd_RecursiveDerefZdd(dd, g0);
    
    cuddCacheInsert2(dd, cuddZddDivide, f, g, r);
    Cudd_Deref(r);
    return(r);

} /* end of cuddZddDivide */


/**Function********************************************************************

  Synopsis    [Performs the recursive step of Cudd_zddDivideF.]

  Description []

  SideEffects [None]

  SeeAlso     [Cudd_zddDivideF]

******************************************************************************/
DdNode	*
cuddZddDivideF(
  DdManager * dd,
  DdNode * f,
  DdNode * g)
{
    int		v;
    DdNode	*one = DD_ONE(dd);
    DdNode	*zero = DD_ZERO(dd);
    DdNode	*f0, *f1, *g0, *g1;
    DdNode	*q, *r, *tmp;
    int		flag;

    if (g == one)
	return(f);
    if (f == zero || f == one)
	return(zero);
    if (f == g)
	return(one);

    /* Check cache. */
    r = cuddCacheLookup2Zdd(dd, cuddZddDivideF, f, g);
    if (r)
	return(r);

    v = Cudd_Regular(g)->index;

    flag = cuddZddGetCofactors2(dd, f, v, &f1, &f0);
    if (flag == 1)
	return(NULL);
    Cudd_Ref(f1);
    Cudd_Ref(f0);
    flag = cuddZddGetCofactors2(dd, g, v, &g1, &g0);	/* g1 != zero */
    if (flag == 1) {
	Cudd_RecursiveDerefZdd(dd, f1);
	Cudd_RecursiveDerefZdd(dd, f0);
	return(NULL);
    }
    Cudd_Ref(g1);
    Cudd_Ref(g0);

    r = cuddZddDivideF(dd, f1, g1);
    if (r == NULL) {
	Cudd_RecursiveDerefZdd(dd, f1);
	Cudd_RecursiveDerefZdd(dd, f0);
	Cudd_RecursiveDerefZdd(dd, g1);
	Cudd_RecursiveDerefZdd(dd, g0);
	return(NULL);
    }
    Cudd_Ref(r);

    if (r != zero && g0 != zero) {
	tmp = r;
	q = cuddZddDivideF(dd, f0, g0);
	if (q == NULL) {
	    Cudd_RecursiveDerefZdd(dd, f1);
	    Cudd_RecursiveDerefZdd(dd, f0);
	    Cudd_RecursiveDerefZdd(dd, g1);
	    Cudd_RecursiveDerefZdd(dd, g0);
	    return(NULL);
	}
	Cudd_Ref(q);
	r = cuddZddIntersect(dd, r, q);
	if (r == NULL) {
	    Cudd_RecursiveDerefZdd(dd, f1);
	    Cudd_RecursiveDerefZdd(dd, f0);
	    Cudd_RecursiveDerefZdd(dd, g1);
	    Cudd_RecursiveDerefZdd(dd, g0);
	    Cudd_RecursiveDerefZdd(dd, q);
	    return(NULL);
	}
	Cudd_Ref(r);
	Cudd_RecursiveDerefZdd(dd, q);
	Cudd_RecursiveDerefZdd(dd, tmp);
    }

    Cudd_RecursiveDerefZdd(dd, f1);
    Cudd_RecursiveDerefZdd(dd, f0);
    Cudd_RecursiveDerefZdd(dd, g1);
    Cudd_RecursiveDerefZdd(dd, g0);
    
    cuddCacheInsert2(dd, cuddZddDivideF, f, g, r);
    Cudd_Deref(r);
    return(r);

} /* end of cuddZddDivideF */


/**Function********************************************************************

  Synopsis    [Computes the three-way decomposition of f w.r.t. v.]

  Description [Computes the three-way decomposition of function f (represented
  by a ZDD) wit respect to variable v.]

  SideEffects [The results are returned in f1, f0, and fd.]

  SeeAlso     [cuddZddGetCofactors2]

******************************************************************************/
int
cuddZddGetCofactors3(
  DdManager * dd,
  DdNode * f,
  int  v,
  DdNode ** f1,
  DdNode ** f0,
  DdNode ** fd)
{
    DdNode	*pc, *nc;
    DdNode	*zero = DD_ZERO(dd);
    int		top, hv, ht, nv;
    int		level;

    top = dd->permZ[Cudd_Regular(f)->index];
    level = dd->permZ[v];
    hv = level >> 1;
    ht = top >> 1;

    if (hv < ht) {
	*f1 = zero;
	*f0 = zero;
	*fd = f;
    }
    else {
	nv = (v >> 1) << 1;
	pc = cuddZddSubset1(dd, f, nv);
	if (pc == NULL)
	    return(1);
	Cudd_Ref(pc);
	nc = cuddZddSubset0(dd, f, nv);
	if (nc == NULL) {
	    Cudd_RecursiveDerefZdd(dd, pc);
	    return(1);
	}
	Cudd_Ref(nc);
	*f1 = cuddZddSubset0(dd, pc, nv + 1);
	if (*f1 == NULL) {
	    Cudd_RecursiveDerefZdd(dd, pc);
	    Cudd_RecursiveDerefZdd(dd, nc);
	    return(1);
	}
	Cudd_Ref(*f1);
	*f0 = cuddZddSubset1(dd, nc, nv + 1);
	if (*f0 == NULL) {
	    Cudd_RecursiveDerefZdd(dd, pc);
	    Cudd_RecursiveDerefZdd(dd, nc);
	    Cudd_RecursiveDerefZdd(dd, *f1);
	    return(1);
	}
	Cudd_Ref(*f0);
	*fd = cuddZddSubset0(dd, nc, nv + 1);
	if (*fd == NULL) {
	    Cudd_RecursiveDerefZdd(dd, pc);
	    Cudd_RecursiveDerefZdd(dd, nc);
	    Cudd_RecursiveDerefZdd(dd, *f1);
	    Cudd_RecursiveDerefZdd(dd, *f0);
	    return(1);
	}
	Cudd_Ref(*fd);
	Cudd_RecursiveDerefZdd(dd, pc);
	Cudd_RecursiveDerefZdd(dd, nc);
	Cudd_Deref(*f1);
	Cudd_Deref(*f0);
	Cudd_Deref(*fd);
    }
    return(0);

} /* end of cuddZddGetCofactors3 */


/**Function********************************************************************

  Synopsis    [Computes the two-way decomposition of f w.r.t. v.]

  Description []

  SideEffects [The results are returned in f1 and f0.]

  SeeAlso     [cuddZddGetCofactors3]

******************************************************************************/
int
cuddZddGetCofactors2(
  DdManager * dd,
  DdNode * f,
  int  v,
  DdNode ** f1,
  DdNode ** f0)
{
    *f1 = cuddZddSubset1(dd, f, v);
    if (*f1 == NULL)
	return(1);
    *f0 = cuddZddSubset0(dd, f, v);
    if (*f0 == NULL) {
	Cudd_RecursiveDerefZdd(dd, *f1);
	return(1);
    }
    return(0);

} /* end of cuddZddGetCofactors2 */


