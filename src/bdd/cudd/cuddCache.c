/**CFile***********************************************************************

  FileName    [cuddCache.c]

  PackageName [cudd]

  Synopsis    [Functions for cache insertion and lookup.]

  Description [Internal procedures included in this module:
		<ul>
		<li> cuddInitCache()
		<li> cuddCacheInsert()
		<li> cuddCacheInsert2()
		<li> cuddCacheLookup()
		<li> cuddCacheLookupZdd()
		<li> cuddCacheLookup2()
		<li> cuddCacheLookup2Zdd()
		<li> cuddConstantLookup()
		<li> cuddCacheProfile()
		<li> cuddCacheResize()
		<li> cuddCacheFlush()
		<li> cuddComputeFloorLog2()
		</ul>
	    Static procedures included in this module:
		<ul>
		</ul> ]

  SeeAlso     []

  Author      [Fabio Somenzi]

  Copyright [ This file was created at the University of Colorado at
  Boulder.  The University of Colorado at Boulder makes no warranty
  about the suitability of this software for any purpose.  It is
  presented on an AS IS basis.]

******************************************************************************/
/* 10/04/02 REK Optimizing cuddInitCache a little. */

/* 10/29/02 REK Adding config.h */
#include <config.h>
#include <string.h>
#include    <bdd/util.h>
#include    <bdd/cuddInt.h>

/*---------------------------------------------------------------------------*/
/* Constant declarations                                                     */
/*---------------------------------------------------------------------------*/

#ifdef DD_CACHE_PROFILE
#define DD_HYSTO_BINS 8
#endif

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
static char rcsid[] DD_UNUSED = "$Id: cuddCache.c,v 1.1.1.1 2005/04/18 17:40:17 mchu Exp $";
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

/*---------------------------------------------------------------------------*/
/* Definition of internal functions                                          */
/*---------------------------------------------------------------------------*/


/**Function********************************************************************

  Synopsis    [Initializes the computed table.]

  Description [Initializes the computed table. It is called by
  Cudd_Init. Returns 1 in case of success; 0 otherwise.]

  SideEffects [None]

  SeeAlso     [Cudd_Init]

******************************************************************************/
/* 10/04/02 REK Changing the for loop to a call to memset.  This gives a huge
 *              speedup on Itanium. */
int
cuddInitCache(
  DdManager * unique /* unique table */,
  unsigned int cacheSize /* initial size of the cache */,
  unsigned int maxCacheSize /* cache size beyond which no resizing occurs */)
{
#if 0
    int i;
#endif
    unsigned int logSize;

    logSize = cuddComputeFloorLog2(ddMax(cacheSize,unique->slots/2));
    cacheSize = 1 << logSize;
    unique->cache = ALLOC(DdCache,cacheSize);
    if (unique->cache == NULL) {
	unique->errorCode = CUDD_MEMORY_OUT;
	return(0);
    }
    unique->cacheSlots = cacheSize;
    unique->cacheShift = sizeof(int) * 8 - logSize;
    unique->minCache = unique->cacheSlots;
    unique->maxCacheHard = maxCacheSize;
    /* We ignore cacheSize in initializing unique->maxCache because
    ** both quantities are monotonic increasing. Hence it is as if
    ** we took the maximum of maxCache and cacheSize. */
    unique->maxCache = ddMin(maxCacheSize,unique->slots);
    Cudd_SetMinHit(unique,DD_MIN_HIT);
    /* Initialize to avoid division by 0 and immediate resizing. */
    unique->cacheMisses = (double) (int) (cacheSize * unique->minHit + 1);
    unique->cacheHits = 0;
    unique->memused += cacheSize * sizeof(DdCache);
    unique->totCachehits = 0;
    /* The sum of cacheMisses and totCacheMisses is always correct,
    ** even though cacheMisses is larger than it should for the reasons
    ** explained above. */
    unique->totCacheMisses = -unique->cacheMisses;
    unique->cachecollisions = 0;
    unique->cacheinserts = 0;
    unique->cachedeletions = 0;

    /* Initialize the cache */
    /* 10/04/02 REK Simply call memset instead of initializing the struct
     *              manually. */
    memset (unique->cache, '\0', cacheSize * sizeof (DdCache));

#if 0
    for (i = 0; (unsigned) i < cacheSize; i++) { 
      unique->cache[i].data = NULL; 
#ifdef DD_CACHE_PROFILE
      unique->cache[i].count = 0; 
#endif
    } 
#endif
    
    return(1);

} /* end of cuddInitCache */


/**Function********************************************************************

  Synopsis    [Inserts a result in the cache.]

  Description []

  SideEffects [None]

  SeeAlso     [cuddCacheInsert2 cuddCacheInsert1]

******************************************************************************/
void
cuddCacheInsert(
  DdManager * table,
  DdNode * (*op)(DdManager *, DdNode *, DdNode *, DdNode *),
  DdNode * f,
  DdNode * g,
  DdNode * h,
  DdNode * data)
{
    int posn;
    register DdCache *entry;

    posn = ddCHash(op,f,g,h,table->cacheShift);
    entry = &table->cache[posn];

    if (entry->data != NULL) {
        table->cachecollisions++;
    }
    table->cacheinserts++;

    entry->op = (DdNode *(*)()) op;
    entry->f = f;
    entry->g = g;
    entry->h = h;
    entry->data = data;
#ifdef DD_CACHE_PROFILE
    entry->count++;
#endif

} /* end of cuddCacheInsert */


/**Function********************************************************************

  Synopsis    [Inserts a result in the cache for a function with two
  operands.]

  Description []

  SideEffects [None]

  SeeAlso     [cuddCacheInsert cuddCacheInsert1]

******************************************************************************/
void
cuddCacheInsert2(
  DdManager * table,
  DdNode * (*op)(DdManager *, DdNode *, DdNode *),
  DdNode * f,
  DdNode * g,
  DdNode * data)
{
    int posn;
    register DdCache *entry;

    posn = ddCHash2(op,f,g,table->cacheShift);
    entry = &table->cache[posn];

    if (entry->data != NULL) {
        table->cachecollisions++;
    }
    table->cacheinserts++;

    entry->op = (DdNode *(*)()) op;
    entry->f = f;
    entry->g = g;
    entry->h = g;
    entry->data = data;
#ifdef DD_CACHE_PROFILE
    entry->count++;
#endif

} /* end of cuddCacheInsert2 */


/**Function********************************************************************

  Synopsis    [Inserts a result in the cache for a function with two
  operands.]

  Description []

  SideEffects [None]

  SeeAlso     [cuddCacheInsert cuddCacheInsert2]

******************************************************************************/
void
cuddCacheInsert1(
  DdManager * table,
  DdNode * (*op)(DdManager *, DdNode *),
  DdNode * f,
  DdNode * data)
{
    int posn;
    register DdCache *entry;

    posn = ddCHash2(op,f,f,table->cacheShift);
    entry = &table->cache[posn];

    if (entry->data != NULL) {
        table->cachecollisions++;
    }
    table->cacheinserts++;

    entry->op = (DdNode *(*)()) op;
    entry->f = f;
    entry->g = f;
    entry->h = f;
    entry->data = data;
#ifdef DD_CACHE_PROFILE
    entry->count++;
#endif

} /* end of cuddCacheInsert1 */


/**Function********************************************************************

  Synopsis    [Looks up in the cache for the result of op applied to f,
  g, and h.]

  Description [Returns the result if found; it returns NULL if no
  result is found.]

  SideEffects [None]

  SeeAlso     [cuddCacheLookup2 cuddCacheLookup1]

******************************************************************************/
DdNode *
cuddCacheLookup(
  DdManager * table,
  DdNode * (*op)(DdManager *, DdNode *, DdNode *, DdNode *),
  DdNode * f,
  DdNode * g,
  DdNode * h)
{
    int posn;
    register DdCache *en,*cache;
    DdNode *data;

    cache = table->cache;
#ifdef DD_DEBUG
    if (cache == NULL) {
        return(NULL);
    }
#endif

    posn = ddCHash(op,f,g,h,table->cacheShift);
    en = &cache[posn];
    if (en->data != NULL && en->f==f && en->g==g && en->h==h && en->op==(DdNode *(*)())op) {
	data = Cudd_Regular(en->data);
	table->cacheHits++;
	if (data->ref == 0) {
	    cuddReclaim(table,data);
	}
	return(en->data);
    }

    /* Cache miss: decide whether to resize. */
    table->cacheMisses++;

    if (table->cacheSlots < table->maxCache &&
	table->cacheHits > table->cacheMisses * table->minHit) {
	cuddCacheResize(table);
    }

    return(NULL);

} /* end of cuddCacheLookup */


/**Function********************************************************************

  Synopsis    [Looks up in the cache for the result of op applied to f,
  g, and h.]

  Description [Returns the result if found; it returns NULL if no
  result is found.]

  SideEffects [None]

  SeeAlso     [cuddCacheLookup2Zdd cuddCacheLookup1Zdd]

******************************************************************************/
DdNode *
cuddCacheLookupZdd(
  DdManager * table,
  DdNode * (*op)(DdManager *, DdNode *, DdNode *, DdNode *),
  DdNode * f,
  DdNode * g,
  DdNode * h)
{
    int posn;
    register DdCache *en,*cache;
    DdNode *data;

    cache = table->cache;
#ifdef DD_DEBUG
    if (cache == NULL) {
        return(NULL);
    }
#endif

    posn = ddCHash(op,f,g,h,table->cacheShift);
    en = &cache[posn];
    if (en->data != NULL && en->f==f && en->g==g && en->h==h && en->op==(DdNode *(*)())op) {
	data = Cudd_Regular(en->data);
	table->cacheHits++;
	if (data->ref == 0) {
	    cuddReclaimZdd(table,data);
	}
	return(en->data);
    }

    /* Cache miss: decide whether to resize. */
    table->cacheMisses++;

    if (table->cacheSlots < table->maxCache &&
	table->cacheHits > table->cacheMisses * table->minHit) {
	cuddCacheResize(table);
    }

    return(NULL);

} /* end of cuddCacheLookupZdd */


/**Function********************************************************************

  Synopsis    [Looks up in the cache for the result of op applied to f
  and g.]

  Description [Returns the result if found; it returns NULL if no
  result is found.]

  SideEffects [None]

  SeeAlso     [cuddCacheLookup cuddCacheLookup1]

******************************************************************************/
DdNode *
cuddCacheLookup2(
  DdManager * table,
  DdNode * (*op)(DdManager *, DdNode *, DdNode *),
  DdNode * f,
  DdNode * g)
{
    int posn;
    register DdCache *en,*cache;
    DdNode *data;

    cache = table->cache;
#ifdef DD_DEBUG
    if (cache == NULL) {
        return(NULL);
    }
#endif

    posn = ddCHash2(op,f,g,table->cacheShift);
    en = &cache[posn];
    if (en->data != NULL && en->f==f && en->g==g && en->op==(DdNode *(*)())op) {
	data = Cudd_Regular(en->data);
	table->cacheHits++;
	if (data->ref == 0) {
	    cuddReclaim(table,data);
	}
	return(en->data);
    }

    /* Cache miss: decide whether to resize. */
    table->cacheMisses++;

    if (table->cacheSlots < table->maxCache &&
	table->cacheHits > table->cacheMisses * table->minHit) {
	cuddCacheResize(table);
    }

    return(NULL);

} /* end of cuddCacheLookup2 */


/**Function********************************************************************

  Synopsis [Looks up in the cache for the result of op applied to f.]

  Description [Returns the result if found; it returns NULL if no
  result is found.]

  SideEffects [None]

  SeeAlso     [cuddCacheLookup cuddCacheLookup2]

******************************************************************************/
DdNode *
cuddCacheLookup1(
  DdManager * table,
  DdNode * (*op)(DdManager *, DdNode *),
  DdNode * f)
{
    int posn;
    register DdCache *en,*cache;
    DdNode *data;

    cache = table->cache;
#ifdef DD_DEBUG
    if (cache == NULL) {
        return(NULL);
    }
#endif

    posn = ddCHash2(op,f,f,table->cacheShift);
    en = &cache[posn];
    if (en->data != NULL && en->f==f && en->op==(DdNode *(*)())op) {
	data = Cudd_Regular(en->data);
	table->cacheHits++;
	if (data->ref == 0) {
	    cuddReclaim(table,data);
	}
	return(en->data);
    }

    /* Cache miss: decide whether to resize. */
    table->cacheMisses++;

    if (table->cacheSlots < table->maxCache &&
	table->cacheHits > table->cacheMisses * table->minHit) {
	cuddCacheResize(table);
    }

    return(NULL);

} /* end of cuddCacheLookup1 */


/**Function********************************************************************

  Synopsis [Looks up in the cache for the result of op applied to f
  and g.]

  Description [Returns the result if found; it returns NULL if no
  result is found.]

  SideEffects [None]

  SeeAlso     [cuddCacheLookupZdd cuddCacheLookup1Zdd]

******************************************************************************/
DdNode *
cuddCacheLookup2Zdd(
  DdManager * table,
  DdNode * (*op)(DdManager *, DdNode *, DdNode *),
  DdNode * f,
  DdNode * g)
{
    int posn;
    register DdCache *en,*cache;
    DdNode *data;

    cache = table->cache;
#ifdef DD_DEBUG
    if (cache == NULL) {
        return(NULL);
    }
#endif

    posn = ddCHash2(op,f,g,table->cacheShift);
    en = &cache[posn];
    if (en->data != NULL && en->f==f && en->g==g && en->op==(DdNode *(*)())op) {
	data = Cudd_Regular(en->data);
	table->cacheHits++;
	if (data->ref == 0) {
	    cuddReclaimZdd(table,data);
	}
	return(en->data);
    }

    /* Cache miss: decide whether to resize. */
    table->cacheMisses++;

    if (table->cacheSlots < table->maxCache &&
	table->cacheHits > table->cacheMisses * table->minHit) {
	cuddCacheResize(table);
    }

    return(NULL);

} /* end of cuddCacheLookup2Zdd */


/**Function********************************************************************

  Synopsis [Looks up in the cache for the result of op applied to f.]

  Description [Returns the result if found; it returns NULL if no
  result is found.]

  SideEffects [None]

  SeeAlso     [cuddCacheLookupZdd cuddCacheLookup2Zdd]

******************************************************************************/
DdNode *
cuddCacheLookup1Zdd(
  DdManager * table,
  DdNode * (*op)(DdManager *, DdNode *),
  DdNode * f)
{
    int posn;
    register DdCache *en,*cache;
    DdNode *data;

    cache = table->cache;
#ifdef DD_DEBUG
    if (cache == NULL) {
        return(NULL);
    }
#endif

    posn = ddCHash2(op,f,f,table->cacheShift);
    en = &cache[posn];
    if (en->data != NULL && en->f==f && en->op==(DdNode *(*)())op) {
	data = Cudd_Regular(en->data);
	table->cacheHits++;
	if (data->ref == 0) {
	    cuddReclaimZdd(table,data);
	}
	return(en->data);
    }

    /* Cache miss: decide whether to resize. */
    table->cacheMisses++;

    if (table->cacheSlots < table->maxCache &&
	table->cacheHits > table->cacheMisses * table->minHit) {
	cuddCacheResize(table);
    }

    return(NULL);

} /* end of cuddCacheLookup1Zdd */


/**Function********************************************************************

  Synopsis [Looks up in the cache for the result of op applied to f,
  g, and h.]

  Description [Looks up in the cache for the result of op applied to f,
  g, and h. Assumes that the calling procedure (e.g.,
  Cudd_bddIteConstant) is only interested in whether the result is
  constant or not. Returns the result if found (possibly
  DD_NON_CONSTANT); otherwise it returns NULL.]

  SideEffects [None]

  SeeAlso     [cuddCacheLookup]

******************************************************************************/
DdNode *
cuddConstantLookup(
  DdManager * table,
  DdNode * (*op)(DdManager *, DdNode *, DdNode *, DdNode *),
  DdNode * f,
  DdNode * g,
  DdNode * h)
{
    int posn;
    register DdCache *en,*cache;

    cache = table->cache;
#ifdef DD_DEBUG
    if (cache == NULL) {
        return(NULL);
    }
#endif
    posn = ddCHash(op,f,g,h,table->cacheShift);
    en = &cache[posn];

    /* We do not reclaim here because the result should not be
     * referenced, but only tested for being a constant.
     */
    if (en->f == f && en->g == g && en->h == h && en->data != NULL &&
    en->op == (DdNode *(*)()) op) {
	table->cacheHits++;
        return(en->data);
    }

    /* Cache miss: decide whether to resize. */
    table->cacheMisses++;

    if (table->cacheSlots < table->maxCache &&
	table->cacheHits > table->cacheMisses * table->minHit) {
	cuddCacheResize(table);
    }

    return(NULL);

} /* end of cuddConstantLookup */


/**Function********************************************************************

  Synopsis    [Computes and prints a profile of the cache usage.]

  Description [Computes and prints a profile of the cache usage.
  Returns 1 if successful; 0 otherwise.]

  SideEffects [None]

  SeeAlso     []

******************************************************************************/
int
cuddCacheProfile(
  DdManager * table,
  FILE * fp)
{
    DdCache *cache = table->cache;
    int slots = table->cacheSlots;
    int nzeroes = 0;
    int i, retval;

#ifdef DD_CACHE_PROFILE
    double count, mean, meansq, stddev, expected;
    long max, min;
    int imax, imin;
    long *hystogramQ, *hystogramR; /* histograms by quotient and remainder */
    int nbins = DD_HYSTO_BINS;
    int bin;
    long thiscount;
    double totalcount;

    meansq = mean = expected = 0.0;
    max = min = (long) cache[0].count;
    imax = imin = 0;
    totalcount = 0.0;

    hystogramQ = ALLOC(long, nbins);
    if (hystogramQ == NULL) {
	table->errorCode = CUDD_MEMORY_OUT;
	return(0);
    }
    hystogramR = ALLOC(long, nbins);
    if (hystogramR == NULL) {
	FREE(hystogramQ);
	table->errorCode = CUDD_MEMORY_OUT;
	return(0);
    }
    for (i = 0; i < nbins; i++) {
	hystogramQ[i] = 0;
	hystogramR[i] = 0;
    }

    for (i = 0; i < slots; i++) {
	thiscount = (long) cache[i].count;
	if (thiscount > max) {
	    max = thiscount;
	    imax = i;
	}
	if (thiscount < min) {
	    min = thiscount;
	    imin = i;
	}
	if (thiscount == 0) {
	    nzeroes++;
	}
	count = (double) thiscount;
	mean += count;
	meansq += count * count;
	totalcount += count;
	expected += count * (double) i;
	bin = (i * nbins) / slots;
	hystogramQ[bin] += thiscount;
	bin = i % nbins;
	hystogramR[bin] += thiscount;
    }
    mean /= (double) slots;
    meansq /= (double) slots;
    stddev = sqrt(meansq - mean*mean);

    retval = fprintf(fp,"Cache stats: slots = %d average = %g ", slots, mean);
    if (retval == EOF) return(0);
    retval = fprintf(fp,"standard deviation = %g\n", stddev);
    if (retval == EOF) return(0);
    retval = fprintf(fp,"Cache max accesses = %ld for slot %d\n", max, imax);
    if (retval == EOF) return(0);
    retval = fprintf(fp,"Cache min accesses = %ld for slot %d\n", min, imin);
    if (retval == EOF) return(0);
    retval = fprintf(fp,"Cache used slots = %.2f%%\n",
		     100.0 - (double) nzeroes * 100.0 / (double) slots);
    if (retval == EOF) return(0);

    if (totalcount > 0) {
	expected /= totalcount;
	retval = fprintf(fp,"Cache access hystogram for %d bins", nbins);
	if (retval == EOF) return(0);
	retval = fprintf(fp," (expected bin value = %g)\nBy quotient:",
			 expected);
	if (retval == EOF) return(0);
	for (i = nbins - 1; i>=0; i--) {
	    retval = fprintf(fp," %ld", hystogramQ[i]);
	    if (retval == EOF) return(0);
	}
	retval = fprintf(fp,"\nBy residue: ");
	if (retval == EOF) return(0);
	for (i = nbins - 1; i>=0; i--) {
	    retval = fprintf(fp," %ld", hystogramR[i]);
	    if (retval == EOF) return(0);
	}
	retval = fprintf(fp,"\n");
	if (retval == EOF) return(0);
    }

    FREE(hystogramQ);
    FREE(hystogramR);
#else
    for (i = 0; i < slots; i++) {
	nzeroes += cache[i].data == NULL;
    }
    retval = fprintf(fp,"Cache used slots = %.2f%%\n",
		     100.0 - (double) nzeroes * 100.0 / (double) slots);
    if (retval == EOF) return(0);
#endif
    return(1);

} /* end of cuddCacheProfile */


/**Function********************************************************************

  Synopsis    [Resizes the cache.]

  Description []

  SideEffects [None]

  SeeAlso     []

******************************************************************************/
void
cuddCacheResize(
  DdManager * table)
{
    DdCache *cache, *oldcache, *entry, *old;
    int i;
    int posn, shift;
    unsigned int slots, oldslots;
    double offset;
    extern void (*MMoutOfMemory)(long);
    void (*saveHandler)(long);

    oldcache = table->cache;
    oldslots = table->cacheSlots;
    slots = table->cacheSlots = oldslots << 1;

#ifdef DD_VERBOSE
    (void) fprintf(stderr,"Resizing the cache from %d to %d entries\n",
		oldslots, slots);
    (void) fprintf(stderr,"\thits = %g\tmisses = %g\thit ratio = %5.3f\n",
		table->cacheHits, table->cacheMisses,
		table->cacheHits / (table->cacheHits + table->cacheMisses));
#endif

    saveHandler = MMoutOfMemory;
    MMoutOfMemory = Cudd_OutOfMem;
    table->cache = cache = ALLOC(DdCache,slots);
    MMoutOfMemory = saveHandler;
    /* If we fail to allocate the new table we just give up. */
    if (cache == NULL) {
#ifdef DD_VERBOSE
	(void) fprintf(stderr,"Resizing failed. Giving up.\n");
#endif
	table->cacheSlots = oldslots;
	table->cache = oldcache;
	/* Do not try to resize again. */
	table->maxCacheHard = table->maxCache = oldslots - 1;
	return;
    }
    shift = --(table->cacheShift);
    table->memused += (slots - oldslots) * sizeof(DdCache);

    /* Clear new cache. */
    for (i = 0; (unsigned) i < slots; i++) {
	cache[i].data = NULL;
#ifdef DD_CACHE_PROFILE
	cache[i].count = 0;
#endif
    }

    /* Copy from old cache to new one. */
    for (i = 0; (unsigned) i < oldslots; i++) {
	old = &oldcache[i];
	if (old->data != NULL) {
	    if (old->g == old->h) { /* probably 2-operand operation */
		posn = ddCHash2(old->op,old->f,old->g,shift);
	    } else {
		posn = ddCHash(old->op,old->f,old->g,old->h,shift);
	    }
	    entry = &cache[posn];
	    entry->op = old->op;
	    entry->f = old->f;
	    entry->g = old->g;
	    entry->h = old->h;
	    entry->data = old->data;	
	}
    }

    FREE(oldcache);

    /* Reinitialize measurements so as to avoid division by 0 and
    ** immediate resizing.
    */
    offset = (double) (int) (slots * table->minHit + 1);
    table->totCacheMisses += table->cacheMisses - offset;
    table->cacheMisses = offset;
    table->totCachehits += table->cacheHits;
    table->cacheHits = 0;

} /* end of cuddCacheResize */


/**Function********************************************************************

  Synopsis    [Flushes the cache.]

  Description []

  SideEffects [None]

  SeeAlso     []

******************************************************************************/
void
cuddCacheFlush(
  DdManager * table)
{
    int i, slots;
    DdCache *cache;

    slots = table->cacheSlots;
    cache = table->cache;
    for (i = 0; i < slots; i++) {
	table->cachedeletions += cache[i].data != NULL;
	cache[i].data = NULL;
    }

    return;

} /* end of cuddCacheFlush */


/**Function********************************************************************

  Synopsis    [Returns the floor of the logarithm to the base 2.]

  Description [Returns the floor of the logarithm to the base 2.
  The input value is assumed to be greater than 0.]

  SideEffects [None]

  SeeAlso     []

******************************************************************************/
int
cuddComputeFloorLog2(
  unsigned int value)
{
    int floorLog = 0;
#ifdef DD_DEBUG
    assert(value > 0);
#endif
    while (value > 1) {
	floorLog++;
	value >>= 1;
    }
    return(floorLog);

} /* end of cuddComputeFloorLog2 */

/*---------------------------------------------------------------------------*/
/* Definition of static functions                                            */
/*---------------------------------------------------------------------------*/
