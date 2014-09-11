/**CFile***********************************************************************

  FileName    [cuddTable.c]

  PackageName [cudd]

  Synopsis    [Unique table management functions.]

  Description [External procedures included in this module:
		<ul>
		<li> Cudd_Prime()
		</ul>
	Internal procedures included in this module:
		<ul>
		<li> cuddAllocNode()
		<li> cuddInitTable()
		<li> cuddFreeTable()
		<li> cuddGarbageCollect()
		<li> cuddGarbageCollectZdd()
		<li> cuddZddGetNode()
		<li> cuddZddGetNodeIVO()
		<li> cuddUniqueInter()
		<li> cuddUniqueInterZdd()
		<li> cuddUniqueConst()
		<li> cuddInsertSubtables()
		<li> cuddDestroySubtables()
		<li> cuddResizeTableZdd()
		</ul>
	Static procedures included in this module:
		<ul>
		<li> ddRehash()
		<li> ddRehashZdd()
		<li> ddResizeTable()
		<li> cuddFindParent()
		<li> cuddOrderedInsert()
		<li> cuddOrderedThread()
		<li> cuddRotateLeft()
		<li> cuddRotateRight()
		<li> cuddDoRebalance()
		</ul>]

  SeeAlso     []

  Author      [Fabio Somenzi]

  Copyright   [This file was created at the University of Colorado at
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

#ifndef DD_UNSORTED_FREE_LIST
/* Constants for red/black trees. */
#define DD_STACK_SIZE 128
#define DD_RED   0
#define DD_BLACK 1
#define DD_PAGE_SIZE 8192
#define DD_PAGE_MASK ~(DD_PAGE_SIZE - 1)
#define DD_INSERT_COMPARE(x,y) \
	(((unsigned long) (x) & DD_PAGE_MASK) - \
	 ((unsigned long) (y) & DD_PAGE_MASK))
#endif

/*---------------------------------------------------------------------------*/
/* Stucture declarations                                                     */
/*---------------------------------------------------------------------------*/

/* This is a hack for when CUDD_VALUE_TYPE is double */
typedef union hack {
    CUDD_VALUE_TYPE value;
    unsigned int bits[2];
} hack;

/*---------------------------------------------------------------------------*/
/* Type declarations                                                         */
/*---------------------------------------------------------------------------*/

/*---------------------------------------------------------------------------*/
/* Variable declarations                                                     */
/*---------------------------------------------------------------------------*/

#ifndef lint
static char rcsid[] DD_UNUSED = "$Id: cuddTable.c,v 1.1.1.1 2005/04/18 17:40:17 mchu Exp $";
#endif

/*---------------------------------------------------------------------------*/
/* Macro declarations                                                        */
/*---------------------------------------------------------------------------*/


#ifndef DD_UNSORTED_FREE_LIST
/* Macros for red/black trees. */
#define DD_COLOR(p)  ((p)->index)
#define DD_IS_BLACK(p) ((p)->index == DD_BLACK)
#define DD_IS_RED(p) ((p)->index == DD_RED)
#define DD_LEFT(p) cuddT(p)
#define DD_RIGHT(p) cuddE(p)
#define DD_NEXT(p) ((p)->next)
#endif


/**AutomaticStart*************************************************************/

/*---------------------------------------------------------------------------*/
/* Static function prototypes                                                */
/*---------------------------------------------------------------------------*/

static void ddRehash ARGS((DdManager *unique, int i));
static void ddRehashZdd ARGS((DdManager *unique, int i));
static int ddResizeTable ARGS((DdManager *unique, int index));
static int cuddFindParent ARGS((DdManager *table, DdNode *node));
DD_INLINE static void ddFixLimits ARGS((DdManager *unique));
#ifdef __osf__
#pragma pointer_size save
#pragma pointer_size short
#endif
static void cuddOrderedInsert ARGS((DdNode **root, DdNode *node));
static DdNode * cuddOrderedThread ARGS((DdNode *root, DdNode *list));
static void cuddRotateLeft ARGS((DdNode **nodeP));
static void cuddRotateRight ARGS((DdNode **nodeP));
static void cuddDoRebalance ARGS((DdNode ***stack, int stackN));
#ifdef __osf__
#pragma pointer_size restore
#endif
static void ddPatchTree ARGS((DdManager *dd, MtrNode *treenode));

/**AutomaticEnd***************************************************************/


/*---------------------------------------------------------------------------*/
/* Definition of exported functions                                          */
/*---------------------------------------------------------------------------*/


/**Function********************************************************************

  Synopsis    [Returns the next prime &gt;= p.]

  Description []

  SideEffects [None]

******************************************************************************/
unsigned int
Cudd_Prime(
  unsigned int  p)
{
    int i,pn;

    p--;
    do {
        p++;
        if (p&1) {
	    pn = 1;
	    i = 3;
	    while ((unsigned) (i * i) <= p) {
		if (p % i == 0) {
		    pn = 0;
		    break;
		}
		i += 2;
	    }
	} else {
	    pn = 0;
	}
    } while (!pn);
    return(p);

} /* end of Cudd_Prime */


/*---------------------------------------------------------------------------*/
/* Definition of internal functions                                          */
/*---------------------------------------------------------------------------*/


/**Function********************************************************************

  Synopsis    [Fast storage allocation for DdNodes in the table.]

  Description [Fast storage allocation for DdNodes in the table. The
  first 4 bytes of a chunk contain a pointer to the next block; the
  rest contains DD_MEM_CHUNK spaces for DdNodes.  Returns a pointer to
  a new node if successful; NULL is memory is full.]

  SideEffects [None]

  SeeAlso     [cuddDynamicAllocNode]

******************************************************************************/
DdNode *
cuddAllocNode(
  DdManager * unique)
{
    int i;
#ifdef __osf__
#pragma pointer_size save
#pragma pointer_size short
#endif
    DdNode **mem,*list,*node;

    if (unique->nextFree == NULL) {	/* memory is all full */
        mem = (DdNode **) ALLOC(DdNode,DD_MEM_CHUNK + 1);
#ifdef __osf__
#pragma pointer_size restore
#endif
	if (mem == NULL) {
	    if (cuddGarbageCollect(unique,1) == 0) {
		unique->errorCode = CUDD_MEMORY_OUT;
#ifdef DD_VERBOSE
	        (void) fprintf(stderr,"cuddAllocNode: out of memory");
		(void) fprintf(stderr,"Memory in use = %ld\n", unique->memused);
#endif
	        return(NULL);
	    }
	} else {
	    unsigned long offset;
	    unique->memused += (DD_MEM_CHUNK + 1) * sizeof(DdNode);
	    mem[0] = (DdNode *) unique->memoryList;
	    unique->memoryList = mem;

	    /* Here we rely on the fact that the size of a DdNode is a
	    ** power of 2 and a multiple of the size of a pointer.
	    ** If we align one node, all the others will be aligned
	    ** as well. */
	    offset = (unsigned long) mem & (sizeof(DdNode) - 1);
	    mem += (sizeof(DdNode) - offset) / sizeof(DdNodePtr);
#ifdef DD_DEBUG
	    assert(((unsigned long) mem & (sizeof(DdNode) - 1)) == 0);
#endif
	    list = (DdNode *) mem;

	    i = 1;
	    do {
		list[i - 1].next = &list[i];
	    } while (++i < DD_MEM_CHUNK);

	    list[DD_MEM_CHUNK-1].next = NULL;

	    unique->nextFree = &list[0];
	}
    }
    unique->allocated++;
    node = unique->nextFree;
    unique->nextFree = node->next;
    return(node);

} /* end of cuddAllocNode */


/**Function********************************************************************

  Synopsis    [Creates and initializes the unique table.]

  Description [Creates and initializes the unique table. Returns a pointer
  to the table if successful; NULL otherwise.]

  SideEffects [None]

  SeeAlso     [Cudd_Init cuddFreeTable]

******************************************************************************/
DdManager *
cuddInitTable(
  unsigned int numVars  /* Initial number of BDD variables (and subtables) */,
  unsigned int numVarsZ /* Initial number of ZDD variables (and subtables) */,
  unsigned int numSlots /* Initial size of the BDD subtables */)
{
    DdManager	*unique = ALLOC(DdManager,1);
    int		i, j;
#ifdef __osf__
#pragma pointer_size save
#pragma pointer_size short
#endif
    DdNode	**nodelist;
#ifdef __osf__
#pragma pointer_size restore
#endif
    unsigned int slots;
    int shift;

    if (unique == NULL) {
	return(NULL);
    }
    unique->epsilon = DD_EPSILON;
    unique->maxGrowth = DD_MAX_REORDER_GROWTH;
    unique->size = numVars;
    unique->sizeZ = numVarsZ;
    unique->maxSize = ddMax(DD_DEFAULT_RESIZE, numVars);
    unique->maxSizeZ = ddMax(DD_DEFAULT_RESIZE, numVarsZ);

    /* Adjust the requested number of slots to a power of 2. */
    slots = CUDD_UNIQUE_SLOTS;
    while (slots < numSlots) {
	slots <<= 1;
    }
    shift = sizeof(int) * 8 - cuddComputeFloorLog2(slots);

    unique->slots = (numVars + 1) * slots;
    unique->keys = 0;
    unique->keysZ = 0;
    unique->dead = 0;
    unique->deadZ = 0;
    unique->gcPercent = DD_GC_PERCENT_HI;
    unique->minDead = (DD_GC_PERCENT_HI * unique->slots) / 100;
    Cudd_SetLooseUpTo(unique,0);
    unique->gcEnabled = 1;
    unique->allocated = 0;
    unique->reclaimed = 0;
    unique->subtables = ALLOC(DdSubtable,unique->maxSize);
    if (unique->subtables == NULL) {
	unique->errorCode = CUDD_MEMORY_OUT;
	return(0);
    }
    unique->subtableZ = ALLOC(DdSubtable,unique->maxSizeZ);
    if (unique->subtableZ == NULL) {
	unique->errorCode = CUDD_MEMORY_OUT;
	return(0);
    }
    unique->perm = ALLOC(int,unique->maxSize);
    if (unique->perm == NULL) {
	unique->errorCode = CUDD_MEMORY_OUT;
	return(0);
    }
    unique->invperm = ALLOC(int,unique->maxSize);
    if (unique->invperm == NULL) {
	unique->errorCode = CUDD_MEMORY_OUT;
	return(0);
    }
    unique->permZ = ALLOC(int,unique->maxSizeZ);
    if (unique->permZ == NULL) {
	unique->errorCode = CUDD_MEMORY_OUT;
	return(0);
    }
    unique->invpermZ = ALLOC(int,unique->maxSizeZ);
    if (unique->invpermZ == NULL) {
	unique->errorCode = CUDD_MEMORY_OUT;
	return(0);
    }
#ifdef __osf__
#pragma pointer_size save
#pragma pointer_size short
#endif
    unique->stack = ALLOC(DdNode *,ddMax(unique->maxSize,unique->maxSizeZ)+1);
#ifdef __osf__
#pragma pointer_size restore
#endif
    if (unique->stack == NULL) {
	unique->errorCode = CUDD_MEMORY_OUT;
	return(0);
    }
    unique->stack[0] = NULL; /* to suppress harmless UMR */

    for (i = 0; (unsigned) i < numVars; i++) {
	unique->subtables[i].slots = slots;
	unique->subtables[i].shift = shift;
	unique->subtables[i].keys = 0;
	unique->subtables[i].dead = 0;
	unique->subtables[i].maxKeys = slots * DD_MAX_SUBTABLE_DENSITY;
#ifdef __osf__
#pragma pointer_size save
#pragma pointer_size short
#endif
	nodelist = unique->subtables[i].nodelist = ALLOC(DdNode *,slots);
#ifdef __osf__
#pragma pointer_size restore
#endif
	if (nodelist == NULL) {
	    unique->errorCode = CUDD_MEMORY_OUT;
	    return(NULL);
	}
	for (j = 0; (unsigned) j < slots; j++) {
	    nodelist[j] = NULL;
	}
	unique->perm[i] = i;
	unique->invperm[i] = i;
    }
    for (i = 0; (unsigned) i < numVarsZ; i++) {
	unique->subtableZ[i].slots = slots;
	unique->subtableZ[i].shift = shift;
	unique->subtableZ[i].keys = 0;
	unique->subtableZ[i].dead = 0;
	unique->subtableZ[i].maxKeys = slots * DD_MAX_SUBTABLE_DENSITY;
#ifdef __osf__
#pragma pointer_size save
#pragma pointer_size short
#endif
	nodelist = unique->subtableZ[i].nodelist = ALLOC(DdNode *,slots);
#ifdef __osf__
#pragma pointer_size restore
#endif
	if (nodelist == NULL) {
	    unique->errorCode = CUDD_MEMORY_OUT;
	    return(NULL);
	}
	for (j = 0; (unsigned) j < slots; j++) {
	    nodelist[j] = NULL;
	}
	unique->permZ[i] = i;
	unique->invpermZ[i] = i;
    }
    unique->constants.slots = slots;
    unique->constants.shift = shift;
    unique->constants.keys = 0;
    unique->constants.dead = 0;
    unique->constants.maxKeys = slots * DD_MAX_SUBTABLE_DENSITY;
#ifdef __osf__
#pragma pointer_size save
#pragma pointer_size short
#endif
    nodelist = unique->constants.nodelist = ALLOC(DdNode *,slots);
#ifdef __osf__
#pragma pointer_size restore
#endif
    if (nodelist == NULL) {
	unique->errorCode = CUDD_MEMORY_OUT;
	return(NULL);
    }
    for (j = 0; (unsigned) j < slots; j++) {
	nodelist[j] = NULL;
    }

    unique->memoryList = NULL;
    unique->nextFree = NULL;

    unique->memused = sizeof(DdManager) + (unique->maxSize + unique->maxSizeZ)
	* (sizeof(DdSubtable) + 2 * sizeof(int)) + (numVars + 1) *
	slots * sizeof(DdNode *) + (ddMax(unique->maxSize,unique->maxSizeZ) +
				    1) * sizeof(DdNode *);

    /* Initialize fields concerned with automatic dynamic reordering */
    unique->reorderings = 0;
    unique->autoDyn = 0;	/* initially disabled */
    unique->autoDynZ = 0;	/* initially disabled */
    unique->realign = 0;	/* initially disabled */
    unique->realignZ = 0;	/* initially disabled */
    unique->reordered = 0;
    unique->autoMethod = CUDD_REORDER_SIFT;
    unique->autoMethodZ = CUDD_REORDER_SIFT;
    unique->nextDyn = DD_FIRST_REORDER;
    unique->countDead = ~0;
    unique->siftMaxVar = DD_SIFT_MAX_VAR;
    unique->siftMaxSwap = DD_SIFT_MAX_SWAPS;
    unique->tree = NULL;
    unique->treeZ = NULL;
    unique->groupcheck = CUDD_GROUP_CHECK7;
    unique->recomb = DD_DEFAULT_RECOMB;
    unique->symmviolation = 0;
    unique->arcviolation = 0;
    unique->populationSize = 0;
    unique->numberXovers = 0;
    unique->linear = NULL;
    unique->linearSize = 0;
#ifdef __osf__
#pragma pointer_size save
#pragma pointer_size short
#endif
    unique->univ = (DdNode **)NULL;
#ifdef __osf__
#pragma pointer_size restore
#endif
    unique->localCaches = NULL;
    unique->preGCHook = NULL;
    unique->postGCHook = NULL;
    unique->preReorderingHook = NULL;
    unique->postReorderingHook = NULL;
    unique->errorCode = CUDD_NO_ERROR;

    /* Initialize statistical counters. */
    unique->garbageCollections = 0;
    unique->GCTime = 0;
    unique->reordTime = 0;
#ifdef DD_UNIQUE_PROFILE
    unique->uniqueLookUps = 0;
    unique->uniqueLinks = 0;
#endif

    return(unique);

} /* end of cuddInitTable */


/**Function********************************************************************

  Synopsis    [Frees the resources associated to a unique table.]

  Description []

  SideEffects [None]

  SeeAlso     [cuddInitTable]

******************************************************************************/
void
cuddFreeTable(
  DdManager * unique)
{
#ifdef __osf__
#pragma pointer_size save
#pragma pointer_size short
#endif
    DdNode **next;
    DdNode **memlist = unique->memoryList;
    int i;

    if (unique->univ != NULL) cuddZddFreeUniv(unique);
    while (memlist != NULL) {
        next = (DdNode **) memlist[0];	/* link to next block */
#ifdef __osf__
#pragma pointer_size restore
#endif
	FREE(memlist);
	memlist = next;
    }
    unique->nextFree = NULL;
    unique->memoryList = NULL;

    for (i = 0; i < unique->size; i++) {
	FREE(unique->subtables[i].nodelist);
    }
    for (i = 0; i < unique->sizeZ; i++) {
	FREE(unique->subtableZ[i].nodelist);
    }
    FREE(unique->constants.nodelist);
    FREE(unique->subtables);
    FREE(unique->subtableZ);
    FREE(unique->cache);
    FREE(unique->perm);
    FREE(unique->permZ);
    FREE(unique->invperm);
    FREE(unique->invpermZ);
    FREE(unique->vars);
    FREE(unique->stack);
    if (unique->tree != NULL) Mtr_FreeTree(unique->tree);
    if (unique->treeZ != NULL) Mtr_FreeTree(unique->treeZ);
    if (unique->linear != NULL) FREE(unique->linear);
    while (unique->preGCHook != NULL)
	Cudd_RemoveHook(unique,unique->preGCHook->f,CUDD_PRE_GC_HOOK);
    while (unique->postGCHook != NULL)
	Cudd_RemoveHook(unique,unique->postGCHook->f,CUDD_POST_GC_HOOK);
    while (unique->preReorderingHook != NULL)
	Cudd_RemoveHook(unique,unique->preReorderingHook->f,
			CUDD_PRE_REORDERING_HOOK);
    while (unique->postReorderingHook != NULL)
	Cudd_RemoveHook(unique,unique->postReorderingHook->f,
			CUDD_POST_REORDERING_HOOK);
    FREE(unique);

} /* end of cuddFreeTable */


/**Function********************************************************************

  Synopsis    [Performs garbage collection on a unique table.]

  Description [Performs garbage collection on a unique table.
  If clearCache is 0, the cache is not cleared. This should only be
  specified if the cache has been cleared right before calling
  cuddGarbageCollect. (As in the case of dynamic reordering.)
  Returns the total number of deleted nodes.]

  SideEffects [None]

  SeeAlso     [cuddGarbageCollectZdd]

******************************************************************************/
int
cuddGarbageCollect(
  DdManager * unique,
  int  clearCache)
{
    DdHook	*hook;
    DdCache	*cache = unique->cache;
#ifdef __osf__
#pragma pointer_size save
#pragma pointer_size short
#endif
    DdNode	**nodelist;
#ifdef __osf__
#pragma pointer_size restore
#endif
    int		i, j, deleted, totalDeleted;
    DdCache	*c;
    DdNode	*node,*last,*next;
    int		slots;
    long	localTime;
#ifndef DD_UNSORTED_FREE_LIST
#ifdef __osf__
#pragma pointer_size save
#pragma pointer_size short
#endif
    DdNode	*tree;
#ifdef __osf__
#pragma pointer_size restore
#endif
#endif

    if (unique->dead == 0) {
        return(0);
    }

    hook = unique->preGCHook;
    while (hook != NULL) {
	int res = (hook->f)(unique,NULL);
	if (res == 0) return(0);
	hook = hook->next;
    }

    localTime = util_cpu_time();

    /* If many nodes are being reclaimed, we want to resize the tables
    ** more aggressively, to reduce the frequency of garbage collection.
    */
    if (2.0 * unique->reclaimed > unique->allocated &&
	unique->slots <= unique->looseUpTo) {
	unique->minDead = (DD_GC_PERCENT_HI * unique->slots) / 100;
#ifdef DD_VERBOSE
	if (unique->gcPercent == DD_GC_PERCENT_LO) {
	    (void) fprintf(stderr,"GC percentage = %d\t", DD_GC_PERCENT_HI);
	    (void) fprintf(stderr,"minDead = %d\n", unique->minDead);
	}
#endif
	unique->gcPercent = DD_GC_PERCENT_HI;
    }

    unique->garbageCollections++;
#ifdef DD_VERBOSE
    (void) fprintf(stderr,"garbage collecting (%d dead out of %d)...",
    		   unique->dead,unique->keys);
#endif

    /* Remove references to garbage collected nodes from the cache. */
    if (clearCache) {
	slots = unique->cacheSlots;
	for (i = 0; i < slots; i++) {
	    c = &cache[i];
	    if (c->data != NULL) {
		if (Cudd_Regular(c->f)->ref == 0 ||
		Cudd_Regular(c->g)->ref == 0 ||
		Cudd_Regular(c->h)->ref == 0 ||
		(c->data != DD_NON_CONSTANT &&
		Cudd_Regular(c->data)->ref == 0)) {
		    c->data = NULL;
		    unique->cachedeletions++;
		}
	    }
	}
	cuddLocalCacheClearDead(unique);
    }

    /* Now return dead nodes to free list. Count them for sanity check. */
    totalDeleted = 0;
#ifndef DD_UNSORTED_FREE_LIST
    tree = NULL;
#endif

    for (i = 0; i < unique->size; i++) {
	if (unique->subtables[i].dead == 0) continue;
	nodelist = unique->subtables[i].nodelist;

	deleted = 0;
	slots = unique->subtables[i].slots;
	for (j = 0; j < slots; j++) {
	    node = nodelist[j];
	    last = NULL;
	    while (node != NULL) {
		next = node->next;
		if (node->ref == 0) {
		    deleted++;
#ifndef DD_UNSORTED_FREE_LIST
#ifdef __osf__
#pragma pointer_size save
#pragma pointer_size short
#endif
		    cuddOrderedInsert(&tree,node);
#ifdef DD_STATS
		    unique->nodesFreed++;
#endif
#ifdef __osf__
#pragma pointer_size restore
#endif
#else
		    cuddDeallocNode(unique,node);
#endif
		    if (last != NULL) {
			last->next = next;
		    } else {
			nodelist[j] = next;
		    }
		} else {
		    last = node;
		}	    
		node = next;
	    }
	}
	if ((unsigned) deleted != unique->subtables[i].dead) {
	    (void) fprintf(stderr,"cuddGarbageCollect: problem in table %d\n",i);
	    fail("cuddGarbageCollect: dead count != deleted");
	}
	totalDeleted += deleted;
	unique->subtables[i].keys -= deleted;
	unique->subtables[i].dead = 0;
    }
    if (unique->constants.dead != 0) {
	nodelist = unique->constants.nodelist;
	deleted = 0;
	slots = unique->constants.slots;
	for (j = 0; j < slots; j++) {
	    node = nodelist[j];
	    last = NULL;
	    while (node != NULL) {
		next = node->next;
		if (node->ref == 0) {
		    deleted++;
#ifndef DD_UNSORTED_FREE_LIST
#ifdef __osf__
#pragma pointer_size save
#pragma pointer_size short
#endif
		    cuddOrderedInsert(&tree,node);
#ifdef DD_STATS
		    unique->nodesFreed++;
#endif
#ifdef __osf__
#pragma pointer_size restore
#endif
#else
		    cuddDeallocNode(unique,node);
#endif
		    if (last != NULL) {
			last->next = next;
		    } else {
			nodelist[j] = next;
		    }
		} else {
		    last = node;
		}	    
		node = next;
	    }
	}
	if ((unsigned) deleted != unique->constants.dead) {
	    (void) fprintf(stderr,"cuddGarbageCollect: problem in constants\n");
	    fail("cuddGarbageCollect: dead count != deleted");
	}
	totalDeleted += deleted;
	unique->constants.keys -= deleted;
	unique->constants.dead = 0;
    }
    if ((unsigned) totalDeleted != unique->dead) {
	fail("cuddGarbageCollect: dead count != deleted");
    }
    unique->keys -= totalDeleted;
    unique->dead = 0;

#ifndef DD_UNSORTED_FREE_LIST
    unique->nextFree = cuddOrderedThread(tree,unique->nextFree);
#endif

    unique->GCTime += util_cpu_time() - localTime;

    hook = unique->postGCHook;
    while (hook != NULL) {
	int res = (hook->f)(unique,NULL);
	if (res == 0) return(0);
	hook = hook->next;
    }

#ifdef DD_VERBOSE
    (void) fprintf(stderr," done\n");
#endif

    return(totalDeleted);

} /* end of cuddGarbageCollect */


/**Function********************************************************************

  Synopsis    [Performs garbage collection on a ZDD unique table.]

  Description [Performs garbage collection on a ZDD unique table.
  If clearCache is 0, the cache is not cleared. This should only be
  specified if the cache has been cleared right before calling
  cuddGarbageCollectZdd. (As in the case of dynamic reordering.)
  Returns the total number of deleted nodes.]

  SideEffects [None]

  SeeAlso     [cuddGarbageCollect]

******************************************************************************/
int
cuddGarbageCollectZdd(
  DdManager * unique,
  int  clearCache)
{
    DdCache	*cache = unique->cache;
#ifdef __osf__
#pragma pointer_size save
#pragma pointer_size short
#endif
    DdNode	**nodelist;
#ifdef __osf__
#pragma pointer_size restore
#endif
    int		i, j, deleted, totalDeleted;
    DdCache	*c;
    DdNode	*node,*last,*next;
    int		slots;
    long	localTime;
#ifndef DD_UNSORTED_FREE_LIST
#ifdef __osf__
#pragma pointer_size save
#pragma pointer_size short
#endif
    DdNode	*tree;
#ifdef __osf__
#pragma pointer_size restore
#endif
#endif

    if (unique->deadZ == 0) {
        return(0);
    }

    localTime = util_cpu_time();

    /* If many nodes are being reclaimed, we want to resize the tables
    ** more aggressively, to reduce the frequency of garbage collection.
    */
    if (2.0 * unique->reclaimed > unique->allocated &&
	unique->slots <= unique->looseUpTo) {
	unique->minDead = (DD_GC_PERCENT_HI * unique->slots) / 100;
#ifdef DD_VERBOSE
	if (unique->gcPercent == DD_GC_PERCENT_LO) {
	    (void) fprintf(stderr,"GC percentage = %d\t", DD_GC_PERCENT_HI);
	    (void) fprintf(stderr,"minDead = %d\n", unique->minDead);
	}
#endif
	unique->gcPercent = DD_GC_PERCENT_HI;
    }

    unique->garbageCollections++;
#ifdef DD_VERBOSE
    (void) fprintf(stderr,"garbage collecting (%d dead out of %d)...",
    		   unique->deadZ,unique->keysZ);
#endif

    /* Remove references to garbage collected nodes from the cache. */
    if (clearCache) {
	slots = unique->cacheSlots;
	for (i = 0; i < slots; i++) {
	    c = &cache[i];
	    if (c->data != NULL) {
		if (Cudd_Regular(c->f)->ref == 0 ||
		Cudd_Regular(c->g)->ref == 0 ||
		Cudd_Regular(c->h)->ref == 0 ||
		(c->data != DD_NON_CONSTANT &&
		Cudd_Regular(c->data)->ref == 0)) {
		    c->data = NULL;
		    unique->cachedeletions++;
		}
	    }
	}
    }

    /* Now return dead nodes to free list. Count them for sanity check. */
    totalDeleted = 0;
#ifndef DD_UNSORTED_FREE_LIST
    tree = NULL;
#endif

    for (i = 0; i < unique->sizeZ; i++) {
	if (unique->subtableZ[i].dead == 0) continue;
	nodelist = unique->subtableZ[i].nodelist;

	deleted = 0;
	slots = unique->subtableZ[i].slots;
	for (j = 0; j < slots; j++) {
	    node = nodelist[j];
	    last = NULL;
	    while (node != NULL) {
		next = node->next;
		if (node->ref == 0) {
		    deleted++;
#ifndef DD_UNSORTED_FREE_LIST
#ifdef __osf__
#pragma pointer_size save
#pragma pointer_size short
#endif
		    cuddOrderedInsert(&tree,node);
#ifdef DD_STATS
		    unique->nodesFreed++;
#endif
#ifdef __osf__
#pragma pointer_size restore
#endif
#else
		    cuddDeallocNode(unique,node);
#endif
		    if (last != NULL) {
			last->next = next;
		    } else {
			nodelist[j] = next;
		    }
		} else {
		    last = node;
		}	    
		node = next;
	    }
	}
	if ((unsigned) deleted != unique->subtableZ[i].dead) {
	    (void) fprintf(stderr,"cuddGarbageCollectZdd: problem in table %d\n",i);
	    fail("cuddGarbageCollectZdd: dead count != deleted");
	}
	totalDeleted += deleted;
	unique->subtableZ[i].keys -= deleted;
	unique->subtableZ[i].dead = 0;
    }

    /* No need to examine the constant table for ZDDs.
    ** If we did we should be careful not to count whatever dead
    ** nodes we found there among the dead ZDD nodes. */
    if ((unsigned) totalDeleted != unique->deadZ) {
	fail("cuddGarbageCollectZdd: dead count != deleted");
    }
    unique->keysZ -= totalDeleted;
    unique->deadZ = 0;

#ifndef DD_UNSORTED_FREE_LIST
    unique->nextFree = cuddOrderedThread(tree,unique->nextFree);
#endif

    unique->GCTime += util_cpu_time() - localTime;

#ifdef DD_VERBOSE
    (void) fprintf(stderr," done\n");
#endif

    return(totalDeleted);

} /* end of cuddGarbageCollectZdd */


/**Function********************************************************************

  Synopsis [Wrapper for cuddUniqueInterZdd.]

  Description [Wrapper for cuddUniqueInterZdd, which applies the ZDD
  reduction rule. Returns a pointer to the result node under normal
  conditions; NULL if reordering occurred or memory was exhausted.]

  SideEffects [None]

  SeeAlso     [cuddUniqueInterZdd]

******************************************************************************/
DdNode *
cuddZddGetNode(
  DdManager * zdd,
  int  id,
  DdNode * T,
  DdNode * E)
{
    DdNode	*node;

    if (T == DD_ZERO(zdd))
	return(E);
    node = cuddUniqueInterZdd(zdd, id, T, E);
    return(node);

} /* end of cuddZddGetNode */


/**Function********************************************************************

  Synopsis [Wrapper for cuddUniqueInterZdd that is independent of variable
  ordering.]

  Description [Wrapper for cuddUniqueInterZdd that is independent of
  variable ordering (IVO). This function does not require parameter
  index to precede the indices of the top nodes of g and h in the
  variable order.  Returns a pointer to the result node under normal
  conditions; NULL if reordering occurred or memory was exhausted.]

  SideEffects [None]

  SeeAlso     [cuddZddGetNode cuddZddIsop]

******************************************************************************/
DdNode *
cuddZddGetNodeIVO(
  DdManager * dd,
  int  index,
  DdNode * g,
  DdNode * h)
{
    DdNode	*f, *r, *t;
    DdNode	*zdd_one = DD_ONE(dd);
    DdNode	*zdd_zero = DD_ZERO(dd);

    f = cuddUniqueInterZdd(dd, index, zdd_one, zdd_zero);
    if (f == NULL) {
	return(NULL);
    }
    cuddRef(f);
    t = cuddZddProduct(dd, f, g);
    if (t == NULL) {
	Cudd_RecursiveDerefZdd(dd, f);
	return(NULL);
    }
    cuddRef(t);
    Cudd_RecursiveDerefZdd(dd, f);
    r = cuddZddUnion(dd, t, h);
    if (r == NULL) {
	Cudd_RecursiveDerefZdd(dd, t);
	return(NULL);
    }
    cuddRef(r);
    Cudd_RecursiveDerefZdd(dd, t);

    cuddDeref(r);
    return(r);

} /* end of cuddZddGetNodeIVO */


/**Function********************************************************************

  Synopsis    [Checks the unique table for the existence of an internal node.]

  Description [Checks the unique table for the existence of an internal
  node. If it does not exist, it creates a new one.  Does not
  modify the reference count of whatever is returned.  A newly created
  internal node comes back with a reference count 0.  For a newly
  created node, increments the reference counts of what T and E point
  to.  Returns a pointer to the new node if successful; NULL if memory
  is exhausted or if reordering took place.]

  SideEffects [None]

  SeeAlso     [cuddUniqueInterZdd]

******************************************************************************/
DdNode *
cuddUniqueInter(
  DdManager * unique,
  int  index,
  DdNode * T,
  DdNode * E)
{
    int pos;
    unsigned int level;
    int retval;
#ifdef __osf__
#pragma pointer_size save
#pragma pointer_size short
#endif
    DdNode **nodelist;
    DdNode *looking;
    DdSubtable *subtable;
#ifdef __osf__
#pragma pointer_size restore
#endif

#ifdef DD_UNIQUE_PROFILE
    unique->uniqueLookUps++;
#endif

    if (index >= unique->size) {
	if (!ddResizeTable(unique,index)) return(NULL);
    }

    level = unique->perm[index];
    subtable = &(unique->subtables[level]);

#ifdef DD_DEBUG
    assert(level < (unsigned) cuddI(unique,T->index));
    assert(level < (unsigned) cuddI(unique,Cudd_Regular(E)->index));
#endif

    pos = ddHash(T, E, subtable->shift);
    nodelist = subtable->nodelist;
    looking = nodelist[pos];

    while (looking != NULL) {
        if (cuddT(looking) == T && cuddE(looking) == E) {
	    if (looking->ref == 0) {
		cuddReclaim(unique,looking);
	    }
	    return(looking);
	}
	looking = looking->next;
#ifdef DD_UNIQUE_PROFILE
	unique->uniqueLinks++;
#endif
    }

    /* countDead is 0 if deads should be counted and ~0 if they should not. */
    if (unique->autoDyn &&
    unique->keys - (unique->dead & unique->countDead) >= unique->nextDyn) {
#ifdef DD_DEBUG
	retval = Cudd_DebugCheck(unique);
	if (retval != 0) return(NULL);
	retval = Cudd_CheckKeys(unique);
	if (retval != 0) return(NULL);
#endif
	retval = Cudd_ReduceHeap(unique,unique->autoMethod,10); /* 10 = whatever */
	if (retval == 0) unique->reordered = 2;
#ifdef DD_DEBUG
	retval = Cudd_DebugCheck(unique);
	if (retval != 0) unique->reordered = 2;
	retval = Cudd_CheckKeys(unique);
	if (retval != 0) unique->reordered = 2;
#endif
	return(NULL);
    }

    if (subtable->keys > subtable->maxKeys) {
        if (unique->gcEnabled &&
	    ((unique->dead > unique->minDead) ||
	    ((unique->dead > unique->minDead / 2) &&
	    (subtable->dead > subtable->keys * 0.95)))) { /* too many dead */
	    (void) cuddGarbageCollect(unique,1);
	} else {
	    ddRehash(unique,(int)level);
	    pos = ddHash(T, E, subtable->shift);
	    nodelist = subtable->nodelist;
	}
    }

    unique->keys++;
    subtable->keys++;

    /* Ref T & E before calling cuddAllocNode, because it may garbage
    ** collect, and then T & E would be lost.
    */
    cuddRef(T);
    cuddRef(E);
    looking = cuddAllocNode(unique);
    if (looking == NULL) return(NULL);
    looking->ref = 0;
    looking->index = index;
    cuddT(looking) = T;
    cuddE(looking) = E;
    looking->next = nodelist[pos];
    nodelist[pos] = looking;

    return(looking);

} /* end of cuddUniqueInter */


/**Function********************************************************************

  Synopsis    [Checks the unique table for the existence of an internal
  ZDD node.]

  Description [Checks the unique table for the existence of an internal
  ZDD node. If it does not exist, it creates a new one.  Does not
  modify the reference count of whatever is returned.  A newly created
  internal node comes back with a reference count 0.  For a newly
  created node, increments the reference counts of what T and E point
  to.  Returns a pointer to the new node if successful; NULL if memory
  is exhausted or if reordering took place.]

  SideEffects [None]

  SeeAlso     [cuddUniqueInter]

******************************************************************************/
DdNode *
cuddUniqueInterZdd(
  DdManager * unique,
  int  index,
  DdNode * T,
  DdNode * E)
{
    int pos;
    unsigned int level;
    int retval;
#ifdef __osf__
#pragma pointer_size save
#pragma pointer_size short
#endif
    DdNode **nodelist;
    DdNode *looking;
    DdSubtable *subtable;
#ifdef __osf__
#pragma pointer_size restore
#endif

#ifdef DD_UNIQUE_PROFILE
    unique->uniqueLookUps++;
#endif

    if (index >= unique->sizeZ) {
	if (!cuddResizeTableZdd(unique,index)) return(NULL);
    }

    level = unique->permZ[index];
    subtable = &(unique->subtableZ[level]);

#ifdef DD_DEBUG
    assert(level < (unsigned) cuddIZ(unique,T->index));
    assert(level < (unsigned) cuddIZ(unique,Cudd_Regular(E)->index));
#endif

    if (subtable->keys > subtable->maxKeys) {
        if (unique->gcEnabled && ((unique->deadZ > unique->minDead) ||
	(10 * subtable->dead > 9 * subtable->keys))) { 	/* too many dead */
	    (void) cuddGarbageCollectZdd(unique,1);
	} else {
	    ddRehashZdd(unique,(int)level);
	}
    }

    pos = ddHash(T, E, subtable->shift);
    nodelist = subtable->nodelist;
    looking = nodelist[pos];

    while (looking != NULL) {
        if (cuddT(looking) == T && cuddE(looking) == E) {
	    if (looking->ref == 0) {
		cuddReclaimZdd(unique,looking);
	    }
	    return(looking);
	}
	looking = looking->next;
#ifdef DD_UNIQUE_PROFILE
	unique->uniqueLinks++;
#endif
    }

    /* countDead is 0 if deads should be counted and ~0 if they should not. */
    if (unique->autoDynZ &&
    unique->keysZ - (unique->deadZ & unique->countDead) >= unique->nextDyn) {
#ifdef DD_DEBUG
	retval = Cudd_DebugCheck(unique);
	if (retval != 0) return(NULL);
	retval = Cudd_CheckKeys(unique);
	if (retval != 0) return(NULL);
#endif
	retval = Cudd_zddReduceHeap(unique,unique->autoMethodZ,10); /* 10 = whatever */
	if (retval == 0) unique->reordered = 2;
#ifdef DD_DEBUG
	retval = Cudd_DebugCheck(unique);
	if (retval != 0) unique->reordered = 2;
	retval = Cudd_CheckKeys(unique);
	if (retval != 0) unique->reordered = 2;
#endif
	return(NULL);
    }

    unique->keysZ++;
    subtable->keys++;

    /* Ref T & E before calling cuddAllocNode, because it may garbage
    ** collect, and then T & E would be lost.
    */
    cuddRef(T);
    cuddRef(E);
    looking = cuddAllocNode(unique);
    if (looking == NULL) return(NULL);
    looking->ref = 0;
    looking->index = index;
    cuddT(looking) = T;
    cuddE(looking) = E;
    looking->next = nodelist[pos];
    nodelist[pos] = looking;

    return(looking);

} /* end of cuddUniqueInterZdd */


/**Function********************************************************************

  Synopsis    [Checks the unique table for the existence of a constant node.]

  Description [Checks the unique table for the existence of a constant node.
  If it does not exist, it creates a new one.  Does not
  modify the reference count of whatever is returned.  A newly created
  internal node comes back with a reference count 0.  Returns a
  pointer to the new node.]

  SideEffects [None]

******************************************************************************/
DdNode *
cuddUniqueConst(
  DdManager * unique,
  CUDD_VALUE_TYPE  value)
{
    int pos;
#ifdef __osf__
#pragma pointer_size save
#pragma pointer_size short
#endif
    DdNode **nodelist;
#ifdef __osf__
#pragma pointer_size restore
#endif
    DdNode *looking;
    hack split;

#ifdef DD_UNIQUE_PROFILE
    unique->uniqueLookUps++;
#endif

    if (unique->constants.keys > unique->constants.maxKeys) {
        if (unique->gcEnabled && ((unique->dead > unique->minDead) ||
	(10 * unique->constants.dead > 9 * unique->constants.keys))) { 	/* too many dead */
	    (void) cuddGarbageCollect(unique,1);
	} else {
	    ddRehash(unique,CUDD_MAXINDEX);
	}
    }

    cuddAdjust(value); /* for the case of crippled infinities */

    if (ddAbs(value) < unique->epsilon) {
	value = 0.0;
    }
    split.value = value;

    pos = ddHash(split.bits[0], split.bits[1], unique->constants.shift);
    nodelist = unique->constants.nodelist;
    looking = nodelist[pos];

    /* Here we compare values both for equality and for difference less
     * than epsilon. The first comparison is required when values are
     * infinite, since Infinity - Infinity is NaN and NaN < X is 0 for
     * every X.
     */
    while (looking != NULL) {
        if (looking->type.value == value ||
	ddEqualVal(looking->type.value,value,unique->epsilon)) {
	    if (looking->ref == 0) {
		cuddReclaim(unique,looking);
	    }
	    return(looking);
	}
	looking = looking->next;
#ifdef DD_UNIQUE_PROFILE
	unique->uniqueLinks++;
#endif
    }

    unique->keys++;
    unique->constants.keys++;

    looking = cuddAllocNode(unique);
    looking->ref = 0;
    looking->index = CUDD_MAXINDEX;
    looking->type.value = value;
    looking->next = nodelist[pos];
    nodelist[pos] = looking;

    return(looking);

} /* end of cuddUniqueConst */


/**Function********************************************************************

  Synopsis [Inserts n new subtables in a unique table at level.]

  Description [Inserts n new subtables in a unique table at level.
  The number n should be positive, and level should be an existing level.
  Returns 1 if successful; 0 otherwise.]

  SideEffects [None]

  SeeAlso     [cuddDestroySubtables]

******************************************************************************/
int
cuddInsertSubtables(
  DdManager * unique,
  int  n,
  int  level)
{
#ifdef __osf__
#pragma pointer_size save
#pragma pointer_size short
#endif
    DdSubtable *newsubtables;
    DdNode **newnodelist;
    DdNode **newvars;
#ifdef __osf__
#pragma pointer_size restore
#endif
    int oldsize,newsize;
    int i,j,index,reorderSave;
    int numSlots = CUDD_UNIQUE_SLOTS;
    int *newperm, *newinvperm;
    DdNode *one, *zero;

#ifdef DD_DEBUG
    assert(n > 0 && level < unique->size);
#endif

    oldsize = unique->size;
    /* Easy case: there is still room in the current table. */
    if (oldsize + n <= unique->maxSize) {
	/* Shift the tables at and below level. */
	for (i = oldsize - 1; i >= level; i--) {
	    unique->subtables[i+n].slots    = unique->subtables[i].slots;
	    unique->subtables[i+n].shift    = unique->subtables[i].shift;
	    unique->subtables[i+n].keys     = unique->subtables[i].keys;
	    unique->subtables[i+n].maxKeys  = unique->subtables[i].maxKeys;
	    unique->subtables[i+n].dead     = unique->subtables[i].dead;
	    unique->subtables[i+n].nodelist = unique->subtables[i].nodelist;
	    index                           = unique->invperm[i];
	    unique->invperm[i+n]            = index;
	    unique->perm[index]            += n;
	}
	/* Create new subtables. */
	for (i = 0; i < n; i++) {
	    unique->subtables[level+i].slots = numSlots;
	    unique->subtables[level+i].shift = sizeof(int) * 8 -
		cuddComputeFloorLog2(numSlots);
	    unique->subtables[level+i].keys = 0;
	    unique->subtables[level+i].maxKeys = numSlots * DD_MAX_SUBTABLE_DENSITY;
	    unique->subtables[level+i].dead = 0;
	    unique->perm[oldsize+i] = level + i;
	    unique->invperm[level+i] = oldsize + i;
#ifdef __osf__
#pragma pointer_size save
#pragma pointer_size short
#endif
	    newnodelist = unique->subtables[level+i].nodelist = ALLOC(DdNode *, numSlots);
#ifdef __osf__
#pragma pointer_size restore
#endif
	    if (newnodelist == NULL) {
		unique->errorCode = CUDD_MEMORY_OUT;
		return(0);
	    }
	    for (j = 0; j < numSlots; j++) {
		newnodelist[j] = NULL;
	    }
	}
    } else {
	/* The current table is too small: we need to allocate a new,
	** larger one; move all old subtables, and initialize the new
	** subtables.
	*/
	newsize = oldsize + n + DD_DEFAULT_RESIZE;
#ifdef DD_VERBOSE
	(void) fprintf(stderr,"Increasing the table size from %d to %d\n",
	    unique->maxSize, newsize);
#endif
	newsubtables = ALLOC(DdSubtable,newsize);
	if (newsubtables == NULL) {
	    unique->errorCode = CUDD_MEMORY_OUT;
	    return(0);
	}
#ifdef __osf__
#pragma pointer_size save
#pragma pointer_size short
#endif
	newvars = ALLOC(DdNode *,newsize);
#ifdef __osf__
#pragma pointer_size restore
#endif
	if (newvars == NULL) {
	    unique->errorCode = CUDD_MEMORY_OUT;
	    return(0);
	}
	newperm = ALLOC(int,newsize);
	if (newperm == NULL) {
	    unique->errorCode = CUDD_MEMORY_OUT;
	    return(0);
	}
	newinvperm = ALLOC(int,newsize);
	if (newinvperm == NULL) {
	    unique->errorCode = CUDD_MEMORY_OUT;
	    return(0);
	}
	unique->memused += (newsize - unique->maxSize) * ((numSlots+1) *
	    sizeof(DdNode *) + 2 * sizeof(int) + sizeof(DdSubtable));
	for (i = 0; i < level; i++) {
	    newsubtables[i].slots = unique->subtables[i].slots;
	    newsubtables[i].shift = unique->subtables[i].shift;
	    newsubtables[i].keys = unique->subtables[i].keys;
	    newsubtables[i].maxKeys = unique->subtables[i].maxKeys;
	    newsubtables[i].dead = unique->subtables[i].dead;
	    newsubtables[i].nodelist = unique->subtables[i].nodelist;
	    newvars[i] = unique->vars[i];
	    newperm[i] = unique->perm[i];
	    newinvperm[i] = unique->invperm[i];
	}
	for (i = level; i < oldsize; i++) {
	    newperm[i] = unique->perm[i];
	}
	for (i = level; i < level + n; i++) {
	    newsubtables[i].slots = numSlots;
	    newsubtables[i].shift = sizeof(int) * 8 -
		cuddComputeFloorLog2(numSlots);
	    newsubtables[i].keys = 0;
	    newsubtables[i].maxKeys = numSlots * DD_MAX_SUBTABLE_DENSITY;
	    newsubtables[i].dead = 0;
	    newperm[oldsize + i - level] = i;
	    newinvperm[i] = oldsize + i - level;
#ifdef __osf__
#pragma pointer_size save
#pragma pointer_size short
#endif
	    newnodelist = newsubtables[i].nodelist = ALLOC(DdNode *, numSlots);
#ifdef __osf__
#pragma pointer_size restore
#endif
	    if (newnodelist == NULL) {
		unique->errorCode = CUDD_MEMORY_OUT;
		return(0);
	    }
	    for (j = 0; j < numSlots; j++) {
		newnodelist[j] = NULL;
	    }
	}
	for (i = level; i < oldsize; i++) {
	    newsubtables[i+n].slots    = unique->subtables[i].slots;
	    newsubtables[i+n].shift    = unique->subtables[i].shift;
	    newsubtables[i+n].keys     = unique->subtables[i].keys;
	    newsubtables[i+n].maxKeys  = unique->subtables[i].maxKeys;
	    newsubtables[i+n].dead     = unique->subtables[i].dead;
	    newsubtables[i+n].nodelist = unique->subtables[i].nodelist;
	    newvars[i]                 = unique->vars[i];
	    index                      = unique->invperm[i];
	    newinvperm[i+n]            = index;
	    newperm[index]            += n;
	}
	FREE(unique->subtables);
	unique->subtables = newsubtables;
	unique->maxSize = newsize;
	FREE(unique->vars);
	unique->vars = newvars;
	FREE(unique->perm);
	unique->perm = newperm;
	FREE(unique->invperm);
	unique->invperm = newinvperm;
	if (newsize > unique->maxSizeZ) {
	    FREE(unique->stack);
#ifdef __osf__
#pragma pointer_size save
#pragma pointer_size short
#endif
	    unique->stack = ALLOC(DdNode *,newsize + 1);
#ifdef __osf__
#pragma pointer_size restore
#endif
	    if (unique->stack == NULL) {
		unique->errorCode = CUDD_MEMORY_OUT;
		return(0);
	    }
	    unique->stack[0] = NULL; /* to suppress harmless UMR */
	    unique->memused +=
		(newsize - ddMax(unique->maxSize,unique->maxSizeZ))
		* sizeof(DdNode *);
	}

    }
    unique->slots += n * numSlots;
    ddFixLimits(unique);
    unique->size += n;

    /* Now that the table is in a coherent state, create the new
    ** projection functions. We need to temporarily disable reordering,
    ** because we cannot reorder without projection functions in place.
    **/
    one = unique->one;
    zero = Cudd_Not(one);

    reorderSave = unique->autoDyn;
    unique->autoDyn = 0;
    for (i = oldsize; i < oldsize + n; i++) {
	unique->vars[i] = cuddUniqueInter(unique,i,one,zero);
	if (unique->vars[i] == NULL) {
	    unique->autoDyn = reorderSave;
	    return(0);
	}
	cuddRef(unique->vars[i]);
    }
    if (unique->tree != NULL) {
	unique->tree->size += n;
	unique->tree->index = unique->invperm[0];
	ddPatchTree(unique,unique->tree);
    }
    unique->autoDyn = reorderSave;

    return(1);

} /* end of cuddInsertSubtables */


/**Function********************************************************************

  Synopsis [Destroys the n most recently created subtables in a unique table.]

  Description [Destroys the n most recently created subtables in a unique
  table.  n should be positive. The subtables should not contain any live
  nodes, except the (isolated) projection function. The projection
  functions are freed.  Returns 1 if successful; 0 otherwise.]

  SideEffects [None]

  SeeAlso     [cuddInsertSubtables]

******************************************************************************/
int
cuddDestroySubtables(
  DdManager * unique,
  int  n)
{
#ifdef __osf__
#pragma pointer_size save
#pragma pointer_size short
#endif
    DdSubtable *subtables;
    DdNode **nodelist;
    DdNode **vars;
#ifdef __osf__
#pragma pointer_size restore
#endif
    int firstIndex, lastIndex;
    int index, level, newlevel;
    int lowestLevel;
    int shift;
    int found;

    /* Sanity check and set up. */
    if (n <= 0) return(0);

    subtables = unique->subtables;
    vars = unique->vars;
    firstIndex = unique->size - n;
    lastIndex  = unique->size - 1;

    /* Check for nodes labeled by the variables being destroyed
    ** that may still be in use.  It is allowed to destroy a variable
    ** only if there are no such nodes. Also, find the lowest level
    ** among the variables being destroyed. This will make further
    ** processing more efficient.
    */
    lowestLevel = unique->size;
    for (index = firstIndex; index < lastIndex; index++) {
	level = unique->perm[index];
	if (level < lowestLevel) lowestLevel = level;
	nodelist = subtables[level].nodelist;
	if (subtables[level].keys - subtables[level].dead != 1) return(0);
	/* The projection function should be isolated. If the ref count
	** is 1, everything is OK. If the ref count is saturated, then
	** we need to make sure that there are no nodes pointing to it.
	** As for the external references, we assume the application is
	** responsible for them.
	*/
	if (vars[index]->ref != 1) {
	    if (vars[index]->ref != DD_MAXREF) return(0);
	    found = cuddFindParent(unique,vars[index]);
	    if (found) {
		return(0);
	    } else {
		vars[index]->ref = 1;
	    }
	}
	Cudd_RecursiveDeref(unique,vars[index]);
    }

    /* Collect garbage, because we cannot afford having dead nodes pointing
    ** to the dead nodes in the subtables being destroyed.
    */
    (void) cuddGarbageCollect(unique,1);

    /* Here we know we can destroy our subtables. */
    for (index = firstIndex; index < lastIndex; index++) {
	level = unique->perm[index];
	nodelist = subtables[level].nodelist;
#ifdef DD_DEBUG
	assert(subtables[level].keys == 0);
#endif
	FREE(nodelist);
	unique->memused -= subtables[level].slots;
	unique->slots -= subtables[level].slots;
	unique->dead -= unique[level].dead;
	unique->keys--;
    }

    /* Here all subtables to be destroyed have their keys field == 0 and
    ** their hash tables have been freed.
    ** We now scan the subtables from level lowestLevel + 1 to level size - 1,
    ** shifting the subtables as required. We keep a running count of
    ** how many subtables have been moved, so that we know by how many
    ** positions each subtable should be shifted.
    */
    shift = 1;
    for (level = lowestLevel + 1; level < unique->size; level++) {
	if (subtables[level].keys == 0) {
	    shift++;
	    continue;
	}
	newlevel = level - shift;
	subtables[newlevel].slots = subtables[level].slots;
	subtables[newlevel].shift = subtables[level].shift;
	subtables[newlevel].keys = subtables[level].keys;
	subtables[newlevel].maxKeys = subtables[level].maxKeys;
	subtables[newlevel].dead = subtables[level].dead;
	subtables[newlevel].nodelist = subtables[level].nodelist;
	unique->invperm[newlevel]  = unique->invperm[level];
    }

    unique->size -= n;

    return(1);

} /* end of cuddDestroySubtables */


/**Function********************************************************************

  Synopsis [Increases the number of ZDD subtables in a unique table so
  that it meets or exceeds index.]

  Description [Increases the number of ZDD subtables in a unique table so
  that it meets or exceeds index.  When new ZDD variables are created, it
  is possible to preserve the functions unchanged, or it is possible to
  preserve the covers unchanged, but not both. cuddResizeTableZdd preserves
  the covers.  Returns 1 if successful; 0 otherwise.]

  SideEffects [None]

  SeeAlso     [ddResizeTable]

******************************************************************************/
int
cuddResizeTableZdd(
  DdManager * unique,
  int  index)
{
#ifdef __osf__
#pragma pointer_size save
#pragma pointer_size short
#endif
    DdSubtable *newsubtables;
    DdNode **newnodelist;
#ifdef __osf__
#pragma pointer_size restore
#endif
    int oldsize,newsize;
    int i,j,reorderSave;
    int numSlots = CUDD_UNIQUE_SLOTS;
    int *newperm, *newinvperm;
    DdNode *one, *zero;

    oldsize = unique->sizeZ;
    /* Easy case: there is still room in the current table. */
    if (index < unique->maxSizeZ) {
	for (i = oldsize; i <= index; i++) {
	    unique->subtableZ[i].slots = numSlots;
	    unique->subtableZ[i].shift = sizeof(int) * 8 -
		cuddComputeFloorLog2(numSlots);
	    unique->subtableZ[i].keys = 0;
	    unique->subtableZ[i].maxKeys = numSlots * DD_MAX_SUBTABLE_DENSITY;
	    unique->subtableZ[i].dead = 0;
	    unique->permZ[i] = i;
	    unique->invpermZ[i] = i;
#ifdef __osf__
#pragma pointer_size save
#pragma pointer_size short
#endif
	    newnodelist = unique->subtableZ[i].nodelist = ALLOC(DdNode *, numSlots);
#ifdef __osf__
#pragma pointer_size restore
#endif
	    if (newnodelist == NULL) {
		unique->errorCode = CUDD_MEMORY_OUT;
		return(0);
	    }
	    for (j = 0; j < numSlots; j++) {
		newnodelist[j] = NULL;
	    }
	}
    } else {
	/* The current table is too small: we need to allocate a new,
	** larger one; move all old subtables, and initialize the new
	** subtables up to index included.
	*/
	newsize = index + DD_DEFAULT_RESIZE;
#ifdef DD_VERBOSE
	(void) fprintf(stderr,"Increasing the ZDD table size from %d to %d\n",
	    unique->maxSizeZ, newsize);
#endif
	newsubtables = ALLOC(DdSubtable,newsize);
	if (newsubtables == NULL) {
	    unique->errorCode = CUDD_MEMORY_OUT;
	    return(0);
	}
	newperm = ALLOC(int,newsize);
	if (newperm == NULL) {
	    unique->errorCode = CUDD_MEMORY_OUT;
	    return(0);
	}
	newinvperm = ALLOC(int,newsize);
	if (newinvperm == NULL) {
	    unique->errorCode = CUDD_MEMORY_OUT;
	    return(0);
	}
	unique->memused += (newsize - unique->maxSizeZ) * ((numSlots+1) *
	    sizeof(DdNode *) + 2 * sizeof(int) + sizeof(DdSubtable));
	if (newsize > unique->maxSize) {
	    FREE(unique->stack);
#ifdef __osf__
#pragma pointer_size save
#pragma pointer_size short
#endif
	    unique->stack = ALLOC(DdNode *,newsize + 1);
#ifdef __osf__
#pragma pointer_size restore
#endif
	    if (unique->stack == NULL) {
		unique->errorCode = CUDD_MEMORY_OUT;
		return(0);
	    }
	    unique->stack[0] = NULL; /* to suppress harmless UMR */
	    unique->memused +=
		(newsize - ddMax(unique->maxSize,unique->maxSizeZ))
		* sizeof(DdNode *);
	}
	for (i = 0; i < oldsize; i++) {
	    newsubtables[i].slots = unique->subtableZ[i].slots;
	    newsubtables[i].shift = unique->subtableZ[i].shift;
	    newsubtables[i].keys = unique->subtableZ[i].keys;
	    newsubtables[i].maxKeys = unique->subtableZ[i].maxKeys;
	    newsubtables[i].dead = unique->subtableZ[i].dead;
	    newsubtables[i].nodelist = unique->subtableZ[i].nodelist;
	    newperm[i] = unique->permZ[i];
	    newinvperm[i] = unique->invpermZ[i];
	}
	for (i = oldsize; i <= index; i++) {
	    newsubtables[i].slots = numSlots;
	    newsubtables[i].shift = sizeof(int) * 8 -
		cuddComputeFloorLog2(numSlots);
	    newsubtables[i].keys = 0;
	    newsubtables[i].maxKeys = numSlots * DD_MAX_SUBTABLE_DENSITY;
	    newsubtables[i].dead = 0;
	    newperm[i] = i;
	    newinvperm[i] = i;
#ifdef __osf__
#pragma pointer_size save
#pragma pointer_size short
#endif
	    newnodelist = newsubtables[i].nodelist = ALLOC(DdNode *, numSlots);
#ifdef __osf__
#pragma pointer_size restore
#endif
	    if (newnodelist == NULL) {
		unique->errorCode = CUDD_MEMORY_OUT;
		return(0);
	    }
	    for (j = 0; j < numSlots; j++) {
		newnodelist[j] = NULL;
	    }
	}
	FREE(unique->subtableZ);
	unique->subtableZ = newsubtables;
	unique->maxSizeZ = newsize;
	FREE(unique->permZ);
	unique->permZ = newperm;
	FREE(unique->invpermZ);
	unique->invpermZ = newinvperm;
    }
    unique->slots += (index + 1 - unique->sizeZ) * numSlots;
    ddFixLimits(unique);
    unique->sizeZ = index + 1;

    /* Now that the table is in a coherent state, update the ZDD
    ** universe. We need to temporarily disable reordering,
    ** because we cannot reorder without universe in place.
    */
    one = unique->one;
    zero = unique->zero;

    reorderSave = unique->autoDynZ;
    unique->autoDynZ = 0;
    cuddZddFreeUniv(unique);
    if (!cuddZddInitUniv(unique)) {
	unique->autoDynZ = reorderSave;
	return(0);
    }
    unique->autoDynZ = reorderSave;

    return(1);

} /* end of cuddResizeTableZdd */


/*---------------------------------------------------------------------------*/
/* Definition of static functions                                            */
/*---------------------------------------------------------------------------*/

/**Function********************************************************************

  Synopsis    [Rehashes a unique subtable.]

  Description []

  SideEffects [None]

  SeeAlso     [ddRehashZdd]

******************************************************************************/
static void
ddRehash(
  DdManager * unique,
  int  i)
{
    unsigned int slots, oldslots;
    int shift, oldshift;
    int j, pos;
#ifdef __osf__
#pragma pointer_size save
#pragma pointer_size short
#endif
    DdNode **nodelist, **oldnodelist;
#ifdef __osf__
#pragma pointer_size restore
#endif
    DdNode *node, *next;
    hack split;
    extern void (*MMoutOfMemory)(long);
    void (*saveHandler)(long);

    if (2.0 * unique->reclaimed <= unique->allocated ||
	unique->slots > unique->looseUpTo) {
	unique->minDead = (DD_GC_PERCENT_LO * unique->slots) / 100;
#ifdef DD_VERBOSE
	if (unique->gcPercent == DD_GC_PERCENT_HI) {
	    (void) fprintf(stderr,"GC percentage = %d\t", DD_GC_PERCENT_LO);
	    (void) fprintf(stderr,"minDead = %d\n", unique->minDead);
	}
#endif
	unique->gcPercent = DD_GC_PERCENT_LO;
    }

    if (i != CUDD_MAXINDEX) {
	oldslots = unique->subtables[i].slots;
	oldshift = unique->subtables[i].shift;
	oldnodelist = unique->subtables[i].nodelist;

	/* Compute the new size of the subtable. Normally, we just
	** double.  However, after reordering, a table may be severely
	** overloaded. Therefore, we iterate. */
	slots = oldslots;
	shift = oldshift;
	do {
	    slots <<= 1;
	    shift--;
	} while (slots * DD_MAX_SUBTABLE_DENSITY <
		 2 * unique->subtables[i].keys);

	saveHandler = MMoutOfMemory;
	MMoutOfMemory = Cudd_OutOfMem;
#ifdef __osf__
#pragma pointer_size save
#pragma pointer_size short
#endif
	nodelist = ALLOC(DdNode *, slots);
#ifdef __osf__
#pragma pointer_size restore
#endif
	MMoutOfMemory = saveHandler;
	if (nodelist == NULL) {
	    int j;
	    (void) fprintf(stderr,
			   "Unable to resize subtable %d for lack of memory\n",
			   i);
	    /* Prevent frequent resizing attempts. */
	    cuddGarbageCollect(unique,1);
	    for (j = 0; j < unique->size; j++) {
		unique->subtables[j].maxKeys <<= 1;
	    }
	    unique->constants.maxKeys <<= 1;
	    return;
	}
	unique->subtables[i].nodelist = nodelist;
	unique->subtables[i].slots = slots;
	unique->subtables[i].shift = shift;
	unique->subtables[i].maxKeys = slots * DD_MAX_SUBTABLE_DENSITY;

	for (j = 0; (unsigned) j < slots; j++) {
	    nodelist[j] = NULL;
	}
	for (j = 0; (unsigned) j < oldslots; j++) {
	    node = oldnodelist[j];
	    while (node != NULL) {
		next = node->next;
		pos = ddHash(cuddT(node), cuddE(node), shift);
		node->next = nodelist[pos];
		nodelist[pos] = node;
		node = next;
	    }
	}
	FREE(oldnodelist);

#ifdef DD_VERBOSE
	(void) fprintf(stderr,"rehashing layer %d: keys %d dead %d new size %d\n",
	i,unique->subtables[i].keys,unique->subtables[i].dead,slots);
#endif
    } else {
	oldslots = unique->constants.slots;
	oldshift = unique->constants.shift;
	oldnodelist = unique->constants.nodelist;

	/* The constant subtable is never subjected to reordering.
	** Therefore, when it is resized, it is because it has just
	** reached the maximum load. We can safely just double the size,
	** with no need for the loop we use for the other tables.
	*/
	slots = oldslots << 1;
	shift = oldshift - 1;
	saveHandler = MMoutOfMemory;
	MMoutOfMemory = Cudd_OutOfMem;
#ifdef __osf__
#pragma pointer_size save
#pragma pointer_size short
#endif
	nodelist = ALLOC(DdNode *, slots);
#ifdef __osf__
#pragma pointer_size restore
#endif
	MMoutOfMemory = saveHandler;
	if (nodelist == NULL) {
	    int j;
	    (void) fprintf(stderr,
			   "Unable to resize constant subtable for lack of memory\n");
	    cuddGarbageCollect(unique,1);
	    for (j = 0; j < unique->size; j++) {
		unique->subtables[j].maxKeys <<= 1;
	    }
	    unique->constants.maxKeys <<= 1;
	    return;
	}
	unique->constants.slots = slots;
	unique->constants.shift = shift;
	unique->constants.maxKeys = slots * DD_MAX_SUBTABLE_DENSITY;
	unique->constants.nodelist = nodelist;
	for (j = 0; (unsigned) j < slots; j++) {
	    nodelist[j] = NULL;
	}
	for (j = 0; (unsigned) j < oldslots; j++) {
	    node = oldnodelist[j];
	    while (node != NULL) {
		next = node->next;
		split.value = cuddV(node);
		pos = ddHash(split.bits[0], split.bits[1], shift);
		node->next = nodelist[pos];
		nodelist[pos] = node;
		node = next;
	    }
	}
	FREE(oldnodelist);

#ifdef DD_VERBOSE
	(void) fprintf(stderr,"rehashing constants: keys %d dead %d new size %d\n",
	unique->constants.keys,unique->constants.dead,slots);
#endif
    }

    /* Update global data */

    unique->memused += (slots - oldslots) * sizeof(DdNode *);
    unique->slots += (slots - oldslots);
    ddFixLimits(unique);

} /* end of ddRehash */


/**Function********************************************************************

  Synopsis    [Rehashes a ZDD unique subtable.]

  Description []

  SideEffects [None]

  SeeAlso     [ddRehash]

******************************************************************************/
static void
ddRehashZdd(
  DdManager * unique,
  int  i)
{
    unsigned int slots, oldslots;
    int shift, oldshift;
    int j, pos;
#ifdef __osf__
#pragma pointer_size save
#pragma pointer_size short
#endif
    DdNode **nodelist, **oldnodelist;
#ifdef __osf__
#pragma pointer_size restore
#endif
    DdNode *node, *next;
    extern void (*MMoutOfMemory)(long);
    void (*saveHandler)(long);

    if (2.0 * unique->reclaimed <= unique->allocated ||
	unique->slots > unique->looseUpTo) {
	unique->minDead = (DD_GC_PERCENT_LO * unique->slots) / 100;
#ifdef DD_VERBOSE
	if (unique->gcPercent == DD_GC_PERCENT_HI) {
	    (void) fprintf(stderr,"GC percentage = %d\t", DD_GC_PERCENT_LO);
	    (void) fprintf(stderr,"minDead = %d\n", unique->minDead);
	}
#endif
	unique->gcPercent = DD_GC_PERCENT_LO;
    }

    assert(i != CUDD_MAXINDEX);
    oldslots = unique->subtableZ[i].slots;
    oldshift = unique->subtableZ[i].shift;
    oldnodelist = unique->subtableZ[i].nodelist;

    /* Compute the new size of the subtable. Normally, we just
    ** double.  However, after reordering, a table may be severely
    ** overloaded. Therefore, we iterate. */
    slots = oldslots;
    shift = oldshift;
    do {
	slots <<= 1;
	shift--;
    } while (slots * DD_MAX_SUBTABLE_DENSITY < 2 * unique->subtableZ[i].keys);

    saveHandler = MMoutOfMemory;
    MMoutOfMemory = Cudd_OutOfMem;
#ifdef __osf__
#pragma pointer_size save
#pragma pointer_size short
#endif
    nodelist = ALLOC(DdNode *, slots);
#ifdef __osf__
#pragma pointer_size restore
#endif
    MMoutOfMemory = saveHandler;
    if (nodelist == NULL) {
	int j;
	(void) fprintf(stderr,
		       "Unable to resize ZDD subtable %d for lack of memory.\n",
		       i);
	cuddGarbageCollectZdd(unique,1);
	for (j = 0; j < unique->sizeZ; j++) {
	    unique->subtableZ[j].maxKeys <<= 1;
	}
	return;
    }
    unique->subtableZ[i].nodelist = nodelist;
    unique->subtableZ[i].slots = slots;
    unique->subtableZ[i].shift = shift;
    unique->subtableZ[i].maxKeys = slots * DD_MAX_SUBTABLE_DENSITY;
    for (j = 0; (unsigned) j < slots; j++) {
	nodelist[j] = NULL;
    }
    for (j = 0; (unsigned) j < oldslots; j++) {
	node = oldnodelist[j];
	while (node != NULL) {
	    next = node->next;
	    pos = ddHash(cuddT(node), cuddE(node), shift);
	    node->next = nodelist[pos];
	    nodelist[pos] = node;
	    node = next;
	}
    }
    FREE(oldnodelist);

#ifdef DD_VERBOSE
    (void) fprintf(stderr,"rehashing layer %d: keys %d dead %d new size %d\n",
		   i,unique->subtableZ[i].keys,unique->subtableZ[i].dead,slots);
#endif

    /* Update global data. */
    unique->memused += (slots - oldslots) * sizeof(DdNode *);
    unique->slots += (slots - oldslots);
    ddFixLimits(unique);

} /* end of ddRehashZdd */


/**Function********************************************************************

  Synopsis [Increases the number of subtables in a unique table so
  that it meets or exceeds index.]

  Description [Increases the number of subtables in a unique table so
  that it meets or exceeds index. Returns 1 if successful; 0 otherwise.]

  SideEffects [None]

  SeeAlso     [cuddResizeTableZdd]

******************************************************************************/
static int
ddResizeTable(
  DdManager * unique,
  int  index)
{
#ifdef __osf__
#pragma pointer_size save
#pragma pointer_size short
#endif
    DdSubtable *newsubtables;
    DdNode **newnodelist;
    DdNode **newvars;
#ifdef __osf__
#pragma pointer_size restore
#endif
    int oldsize,newsize;
    int i,j,reorderSave;
    int numSlots = CUDD_UNIQUE_SLOTS;
    int *newperm, *newinvperm;
    DdNode *one, *zero;

    oldsize = unique->size;
    /* Easy case: there is still room in the current table. */
    if (index < unique->maxSize) {
	for (i = oldsize; i <= index; i++) {
	    unique->subtables[i].slots = numSlots;
	    unique->subtables[i].shift = sizeof(int) * 8 -
		cuddComputeFloorLog2(numSlots);
	    unique->subtables[i].keys = 0;
	    unique->subtables[i].maxKeys = numSlots * DD_MAX_SUBTABLE_DENSITY;
	    unique->subtables[i].dead = 0;
	    unique->perm[i] = i;
	    unique->invperm[i] = i;
#ifdef __osf__
#pragma pointer_size save
#pragma pointer_size short
#endif
	    newnodelist = unique->subtables[i].nodelist = ALLOC(DdNode *, numSlots);
#ifdef __osf__
#pragma pointer_size restore
#endif
	    if (newnodelist == NULL) {
		unique->errorCode = CUDD_MEMORY_OUT;
		return(0);
	    }
	    for (j = 0; j < numSlots; j++) {
		newnodelist[j] = NULL;
	    }
	}
    } else {
	/* The current table is too small: we need to allocate a new,
	** larger one; move all old subtables, and initialize the new
	** subtables up to index included.
	*/
	newsize = index + DD_DEFAULT_RESIZE;
#ifdef DD_VERBOSE
	(void) fprintf(stderr,"Increasing the table size from %d to %d\n",
	    unique->maxSize, newsize);
#endif
	newsubtables = ALLOC(DdSubtable,newsize);
	if (newsubtables == NULL) {
	    unique->errorCode = CUDD_MEMORY_OUT;
	    return(0);
	}
#ifdef __osf__
#pragma pointer_size save
#pragma pointer_size short
#endif
	newvars = ALLOC(DdNode *,newsize);
#ifdef __osf__
#pragma pointer_size restore
#endif
	if (newvars == NULL) {
	    unique->errorCode = CUDD_MEMORY_OUT;
	    return(0);
	}
	newperm = ALLOC(int,newsize);
	if (newperm == NULL) {
	    unique->errorCode = CUDD_MEMORY_OUT;
	    return(0);
	}
	newinvperm = ALLOC(int,newsize);
	if (newinvperm == NULL) {
	    unique->errorCode = CUDD_MEMORY_OUT;
	    return(0);
	}
	unique->memused += (newsize - unique->maxSize) * ((numSlots+1) *
	    sizeof(DdNode *) + 2 * sizeof(int) + sizeof(DdSubtable));
	if (newsize > unique->maxSizeZ) {
	    FREE(unique->stack);
#ifdef __osf__
#pragma pointer_size save
#pragma pointer_size short
#endif
	    unique->stack = ALLOC(DdNode *,newsize + 1);
#ifdef __osf__
#pragma pointer_size restore
#endif
	    if (unique->stack == NULL) {
		unique->errorCode = CUDD_MEMORY_OUT;
		return(0);
	    }
	    unique->stack[0] = NULL; /* to suppress harmless UMR */
	    unique->memused +=
		(newsize - ddMax(unique->maxSize,unique->maxSizeZ))
		* sizeof(DdNode *);
	}
	for (i = 0; i < oldsize; i++) {
	    newsubtables[i].slots = unique->subtables[i].slots;
	    newsubtables[i].shift = unique->subtables[i].shift;
	    newsubtables[i].keys = unique->subtables[i].keys;
	    newsubtables[i].maxKeys = unique->subtables[i].maxKeys;
	    newsubtables[i].dead = unique->subtables[i].dead;
	    newsubtables[i].nodelist = unique->subtables[i].nodelist;
	    newvars[i] = unique->vars[i];
	    newperm[i] = unique->perm[i];
	    newinvperm[i] = unique->invperm[i];
	}
	for (i = oldsize; i <= index; i++) {
	    newsubtables[i].slots = numSlots;
	    newsubtables[i].shift = sizeof(int) * 8 -
		cuddComputeFloorLog2(numSlots);
	    newsubtables[i].keys = 0;
	    newsubtables[i].maxKeys = numSlots * DD_MAX_SUBTABLE_DENSITY;
	    newsubtables[i].dead = 0;
	    newperm[i] = i;
	    newinvperm[i] = i;
#ifdef __osf__
#pragma pointer_size save
#pragma pointer_size short
#endif
	    newnodelist = newsubtables[i].nodelist = ALLOC(DdNode *, numSlots);
#ifdef __osf__
#pragma pointer_size restore
#endif
	    if (newnodelist == NULL) {
		unique->errorCode = CUDD_MEMORY_OUT;
		return(0);
	    }
	    for (j = 0; j < numSlots; j++) {
		newnodelist[j] = NULL;
	    }
	}
	FREE(unique->subtables);
	unique->subtables = newsubtables;
	unique->maxSize = newsize;
	FREE(unique->vars);
	unique->vars = newvars;
	FREE(unique->perm);
	unique->perm = newperm;
	FREE(unique->invperm);
	unique->invperm = newinvperm;
    }
    unique->slots += (index + 1 - unique->size) * numSlots;
    ddFixLimits(unique);
    unique->size = index + 1;

    /* Now that the table is in a coherent state, create the new
    ** projection functions. We need to temporarily disable reordering,
    ** because we cannot reorder without projection functions in place.
    **/
    one = unique->one;
    zero = Cudd_Not(one);

    reorderSave = unique->autoDyn;
    unique->autoDyn = 0;
    for (i = oldsize; i <= index; i++) {
	unique->vars[i] = cuddUniqueInter(unique,i,one,zero);
	if (unique->vars[i] == NULL) {
	    unique->autoDyn = reorderSave;
	    return(0);
	}
	cuddRef(unique->vars[i]);
    }
    unique->autoDyn = reorderSave;

    return(1);

} /* end of ddResizeTable */


/**Function********************************************************************

  Synopsis    [Searches the subtables above node for a parent.]

  Description [Searches the subtables above node for a parent. Returns 1
  as soon as one parent is found. Returns 0 is the search is fruitless.]

  SideEffects [None]

  SeeAlso     []

******************************************************************************/
static int
cuddFindParent(
  DdManager * table,
  DdNode * node)
{
    int         i,j;
    int		slots;
#ifdef __osf__
#pragma pointer_size save
#pragma pointer_size short
#endif
    DdNode	**nodelist,*f;
#ifdef __osf__
#pragma pointer_size restore
#endif
	
    for (i = cuddI(table,node->index) - 1; i >= 0; i--) {
	nodelist = table->subtables[i].nodelist;
	slots = table->subtables[i].slots;

	for (j = 0; j < slots; j++) {
	    f = nodelist[j];
	    while (f != NULL) {
		if (cuddT(f) == node || Cudd_Regular(cuddE(f)) == node) {
		    return(1);
		}
		f = f->next;
	    }
	}
    }

    return(0);

} /* end of cuddFindParent */


/**Function********************************************************************

  Synopsis    [Adjusts the values of table limits.]

  Description [Adjusts the values of table fields controlling the.
  sizes of subtables and computed table. If the computed table is too small
  according to the new values, it is resized.]

  SideEffects [Modifies manager fields. May resize computed table.]

  SeeAlso     []

******************************************************************************/
DD_INLINE
static void
ddFixLimits(
  DdManager *unique)
{
    unique->minDead = (unique->gcPercent * unique->slots) / 100;
    unique->maxCache = ddMin(unique->maxCacheHard,2*unique->slots);
    unique->minCache = ddMax(unique->minCache,unique->slots/2);
    if (unique->cacheSlots < ddMin(unique->minCache,unique->maxCache))
	cuddCacheResize(unique);
    return;

} /* end of ddFixLimits */


#ifndef DD_UNSORTED_FREE_LIST
#ifdef __osf__
#pragma pointer_size save
#pragma pointer_size short
#endif
/**Function********************************************************************

  Synopsis    [Inserts a DdNode in a red/black search tree.]

  Description [Inserts a DdNode in a red/black search tree. Nodes from
  the same "page" (defined by DD_PAGE_MASK) are linked in a LIFO list.]

  SideEffects [None]

  SeeAlso     [cuddOrderedThread]

******************************************************************************/
static void
cuddOrderedInsert(
  DdNode ** root,
  DdNode * node)
{
    DdNode *scan;
    DdNode **scanP;
    DdNode **stack[DD_STACK_SIZE];
    int stackN = 0;

    scanP = root;
    while ((scan = *scanP) != NULL) {
	stack[stackN++] = scanP;
	if (DD_INSERT_COMPARE(node, scan) == 0) { /* add to page list */
	    DD_NEXT(node) = DD_NEXT(scan);
	    DD_NEXT(scan) = node;
	    return;
	}
	scanP = (node < scan) ? &DD_LEFT(scan) : &DD_RIGHT(scan);
    }
    DD_RIGHT(node) = DD_LEFT(node) = DD_NEXT(node) = NULL;
    DD_COLOR(node) = DD_RED;
    *scanP = node;
    stack[stackN] = &node;
    cuddDoRebalance(stack,stackN);

} /* end of cuddOrderedInsert */


/**Function********************************************************************

  Synopsis    [Threads all the nodes of a search tree into a linear list.]

  Description [Threads all the nodes of a search tree into a linear
  list. For each node of the search tree, the "left" child, if non-null, has
  a lower address than its parent, and the "right" child, if non-null, has a
  higher address than its parent.
  The list is sorted in order of increasing addresses. The search
  tree is destroyed as a result of this operation. The last element of
  the linear list is made to point to the address passed in list. Each
  node if the search tree is a linearly-linked list of nodes from the
  same memory page (as defined in DD_PAGE_MASK). When a node is added to
  the linear list, all the elements of the linked list are added.]

  SideEffects [The search tree is destroyed as a result of this operation.]

  SeeAlso     [cuddOrderedInsert]

******************************************************************************/
static DdNode *
cuddOrderedThread(
  DdNode * root,
  DdNode * list)
{
    DdNode *current, *next, *prev, *end;

    current = root;
    /* The first word in the node is used to implement a stack that holds
    ** the nodes from the root of the tree to the current node. Here we
    ** put the root of the tree at the bottom of the stack.
    */
    *((DdNode **) current) = NULL;

    while (current != NULL) {
	if (DD_RIGHT(current) != NULL) {
	    /* If possible, we follow the "right" link. Eventually we'll
	    ** find the node with the largest address in the current tree.
	    ** In this phase we use the first word of a node to implemen
	    ** a stack of the nodes on the path from the root to "current".
	    ** Also, we disconnect the "right" pointers to indicate that
	    ** we have already followed them.
	    */
	    next = DD_RIGHT(current);
	    DD_RIGHT(current) = NULL;
	    *((DdNode **)next) = current;
	    current = next;
	} else {
	    /* We can't proceed along the "right" links any further.
	    ** Hence "current" is the largest element in the current tree.
	    ** We make this node the new head of "list". (Repeating this
	    ** operation until the tree is empty yields the desired linear
	    ** threading of all nodes.)
	    */
	    prev = *((DdNode **) current); /* save prev node on stack in prev */
	    /* Traverse the linked list of current until the end. */
	    for (end = current; DD_NEXT(end) != NULL; end = DD_NEXT(end));
	    DD_NEXT(end) = list; /* attach "list" at end and make */
	    list = current;   /* "current" the new head of "list" */
	    /* Now, if current has a "left" child, we push it on the stack.
	    ** Otherwise, we just continue with the parent of "current".
	    */
	    if (DD_LEFT(current) != NULL) {
		next = DD_LEFT(current);
		*((DdNode **) next) = prev;
		current = next;
	    } else {
		current = prev;
	    }
	}
    }

    return(list);

} /* end of cuddOrderedThread */


/**Function********************************************************************

  Synopsis    [Performs the left rotation for red/black trees.]

  Description []

  SideEffects [None]

  SeeAlso     [cuddRotateRight]

******************************************************************************/
DD_INLINE
static void
cuddRotateLeft(
  DdNode ** nodeP)
{
    DdNode *newRoot;
    DdNode *oldRoot = *nodeP;

    *nodeP = newRoot = DD_RIGHT(oldRoot);
    DD_RIGHT(oldRoot) = DD_LEFT(newRoot);
    DD_LEFT(newRoot) = oldRoot;

} /* end of cuddRotateLeft */


/**Function********************************************************************

  Synopsis    [Performs the right rotation for red/black trees.]

  Description []

  SideEffects [None]

  SeeAlso     [cuddRotateLeft]

******************************************************************************/
DD_INLINE
static void
cuddRotateRight(
  DdNode ** nodeP)
{
    DdNode *newRoot;
    DdNode *oldRoot = *nodeP;

    *nodeP = newRoot = DD_LEFT(oldRoot);
    DD_LEFT(oldRoot) = DD_RIGHT(newRoot);
    DD_RIGHT(newRoot) = oldRoot;

} /* end of cuddRotateRight */


/**Function********************************************************************

  Synopsis    [Rebalances a red/black tree.]

  Description []

  SideEffects [None]

  SeeAlso     []

******************************************************************************/
static void
cuddDoRebalance(
  DdNode *** stack,
  int  stackN)
{
    DdNode **xP, *x, *y, **parentP, *parent, **grandpaP, *grandpa;

    xP = stack[stackN];
    x = *xP;
    /* Work our way back up, re-balancing the tree. */
    while (--stackN >= 0) {
	parentP = stack[stackN];
	parent = *parentP;
	if (DD_IS_BLACK(parent)) break;
	/* Since the root is black, here a non-null grandparent exists. */
	grandpaP = stack[stackN-1];
	grandpa = *grandpaP;
	if (parent == DD_LEFT(grandpa)) {
	    y = DD_RIGHT(grandpa);
	    if (y != NULL && DD_IS_RED(y)) {
		DD_COLOR(parent) = DD_BLACK;
		DD_COLOR(y) = DD_BLACK;
		DD_COLOR(grandpa) = DD_RED;
		x = grandpa;
		stackN--;
	    } else {
		if (x == DD_RIGHT(parent)) {
		    cuddRotateLeft(parentP);
		    DD_COLOR(x) = DD_BLACK;
		} else {
		    DD_COLOR(parent) = DD_BLACK;
		}
		DD_COLOR(grandpa) = DD_RED;
		cuddRotateRight(grandpaP);
		break;
	    }
	} else {
	    y = DD_LEFT(grandpa);
	    if (y != NULL && DD_IS_RED(y)) {
		DD_COLOR(parent) = DD_BLACK;
		DD_COLOR(y) = DD_BLACK;
		DD_COLOR(grandpa) = DD_RED;
		x = grandpa;
		stackN--;
	    } else {
		if (x == DD_LEFT(parent)) {
		    cuddRotateRight(parentP);
		    DD_COLOR(x) = DD_BLACK;
		} else {
		    DD_COLOR(parent) = DD_BLACK;
		}
		DD_COLOR(grandpa) = DD_RED;
		cuddRotateLeft(grandpaP);
	    }
	}
    }
    DD_COLOR(*(stack[0])) = DD_BLACK;

} /* end of cuddDoRebalance */
#ifdef __osf__
#pragma pointer_size restore
#endif
#endif


/**Function********************************************************************

  Synopsis    [Fixes a variable tree after the insertion of new subtables.]

  Description [Fixes a variable tree after the insertion of new subtables.
  After such an insertion, the low fields of the tree below the insertion
  point are inconsistent.]

  SideEffects [None]

  SeeAlso     []

******************************************************************************/
static void
ddPatchTree(
  DdManager *dd,
  MtrNode *treenode)
{
    MtrNode *auxnode = treenode;

    while (auxnode != NULL) {
	auxnode->low = dd->perm[auxnode->index];
	if (auxnode->child != NULL) {
	    ddPatchTree(dd, auxnode->child);
	}
	auxnode = auxnode->younger;
    }

    return;

} /* end of ddPatchTree */
