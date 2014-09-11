#include <Pcode/pcode.h>
#include <Pcode/struct.h>
#include <Pcode/cfg.h>
#include <Pcode/loop.h>
#include <Pcode/symtab_i.h>
#include <library/i_hash.h>
#include "ss_ssa2.h"
#include "ss_lifetime.h"

typedef struct Lifetime
{
  Set lifeSubscrs;
  int num;
  int visited;

  void *ext;
}
_Lifetime, *Lifetime;

#define LIFETIME(a) ((Lifetime)(a)->ext)

static void LoopMerge (PSS_Base base, HashTable defTbl);
static void DisjMerge (PSS_Def phiDef, HashTable defTbl);
static void PhiMerge  (PSS_Def phiDef, HashTable defTbl);

static int  ComputeLifetimes_Var (PC_Graph, PSS_Base, PSS_LifetimeSelect);
static void CombineLifetimes (Lifetime, Lifetime, HashTable);

static Lifetime newLifetime (int, void*);
static void* freeLifetime (Lifetime, HashTable);



int
PSS_ComputeLifetimes (PC_Graph cfg, PSS_BaseTbl varTbl,
		      PSS_LifetimeSelect select)
{
  PSS_Base base;
  int ltCount = 0;

  if (varTbl)
    for (base = varTbl->first; base; base = base->next)
      if (!base->addr_taken)
	if (!LT_PTR(select) || PSI_IsPointerType (base->vdcl->type))
	  ltCount += ComputeLifetimes_Var (cfg, base, select);

  return ltCount;
}


#define DEF_TABLE_SIZE 64
static int
ComputeLifetimes_Var (PC_Graph cfg, PSS_Base base, PSS_LifetimeSelect select)
{
  int ltCount;
  List ltSets = NULL;
  PSS_Def def;
  HashTable defTbl = HashTable_create (DEF_TABLE_SIZE);
  Set s;
  int ltNum = 1;

  /* Construct a hash of defs and create Lifetime structures. */
  for (def = base->defs; def; def = def->next)
    {
      if (HashTable_find_or_null (defTbl, def->subscr) != NULL)
	P_punt ("\nComputeLifetimes_Var: variable \"%s\" in function \"%s\"\n" 
		"already exists in hashtable with subscript %i.",
		base->vdcl->name, cfg->func->name, def->subscr);

      HashTable_insert (defTbl, def->subscr, def);

/* Define CAST_AS_LVALUE_OK if your compiler accepts casts as lvalues and
 * does not support statement expressions.  This does not need to be defined
 * for gcc or icc.  This must not be defined for gcc >= 4. */
#undef CAST_AS_LVALUE_OK

#ifdef CAST_AS_LVALUE_OK
      LIFETIME(def) = newLifetime (def->subscr, def->ext);
#else
      def->ext = (void*) (newLifetime (def->subscr, def->ext));
#endif      
    }

  /* Compute Lifetime info using selected method */
  switch (select)
    {
      case FULL:
      case FULL_PTR:
	/* No merging takes place with full renaming. */
	break;
	
      case LOOP:
      case LOOP_PTR:
	LoopMerge (base, defTbl);
	break;
	
      case MERGE_PHIS:
      case MERGE_PHIS_PTR:
	for (def = base->defs; def; def = def->next)
	  if (MERGE_TYPE (def->type) && !LIFETIME(def)->visited)
	    PhiMerge (def, defTbl);
	break;

      case DISJOINT_LTS:
      case DISJOINT_LTS_PTR:
	for (def = base->defs; def; def = def->next)
	  if (MERGE_TYPE (def->type) && !LIFETIME(def)->visited)
	    DisjMerge (def, defTbl);
	break;

      default:
	P_punt ("ComputeLifetimes_Var: invalid lifetime selector.");
      }
  
  /* Do something with lifetime info */
  HashTable_start (defTbl);
  while ((def = HashTable_next (defTbl)))
    if (!List_member (ltSets, LIFETIME(def)->lifeSubscrs))
      ltSets = List_insert_last (ltSets , LIFETIME(def)->lifeSubscrs);

  ltCount = List_size (ltSets);

  List_start (ltSets);
  while ((s = List_next (ltSets)))
    {
      int isTemp = 1;
      SetIterator *si = Set_iterator (s, UNDEF_SUBSCR);
    
      while (!Set_exhausted (si))
	{
	  int subscr = Set_next (si);

	  def = HashTable_find (defTbl, subscr);
	  def->name = ltNum;

	  if (!MERGE_TYPE(def->type))
	    isTemp = 0;
	}
      
      si = Set_end_iterator (si);
      ltNum++;

      if (isTemp)
	{
	  si = Set_iterator (s, UNDEF_SUBSCR);

	  while (!Set_exhausted (si))
	    {
	      int subscr = Set_next (si);

	      def = HashTable_find (defTbl, subscr);
	      SET_DEF_TYPE (def->type, PIPA_TEMP);
	    }
	  
	  si = Set_end_iterator (si);
	}
    }

  /* Cleanup */
  List_reset (ltSets);

  for (def = base->defs; def; def = def->next)
    def->ext = freeLifetime (LIFETIME(def), defTbl);

  HashTable_free (defTbl);

  return ltCount;
}


static int
SameLifetimes (PSS_Def d1, PSS_Def d2)
{
  int ret = 0;

  if (LIFETIME(d1)->lifeSubscrs == LIFETIME(d2)->lifeSubscrs)
    ret = 1;

  return ret;
}

static int
LoopContainsDef (PC_Loop loop, PSS_Def def)
{
  int ret;
  
  assert (loop != NULL);

  if (UNINITIALIZED_TYPE(def->type))
    ret = 0;
  else
    ret = Set_in (loop->body, def->bb->ID);
  
  return ret;
}


static void
LoopMerge (PSS_Base base, HashTable defTbl)
{
  PSS_Def def, muDef;
  List muList = NULL;
  
  /* Creat a list of all MU defs */
  for (def = base->defs; def; def = def->next)
    if (MU_TYPE (def->type))
      muList = List_insert_last (muList, def);

  /* For each MU def, merge it with all defs within the same loop */
  List_start (muList);
  while ((muDef = List_next (muList)))
    {
      PC_Loop loop = muDef->bb->loop;

      for (def = base->defs; def; def = def->next)
	if (LoopContainsDef (loop, def) && !SameLifetimes (muDef, def))
	  CombineLifetimes (LIFETIME(muDef), LIFETIME(def), defTbl);
    }
}


static void
DisjMerge_rec (PSS_Def def, HashTable defTbl)
{
  List rhsVars;
  VarDcl vdcl;
  Expr var;

  LIFETIME(def)->visited = 1;

  if (UNINITIALIZED_TYPE (def->type))
    return;

  rhsVars = PSS_GetSubExprByOpcode_List (def->var, OP_var);
  vdcl = PSI_GetVarDclEntry (def->var->value.var.key);

  List_start (rhsVars);
  while ((var = List_next (rhsVars)))
    if (PSI_GetVarDclEntry (var->value.var.key) == vdcl)
      {
	PSS_Def varDef = var->value.var.ssa;
	if (!LIFETIME(varDef)->visited && !SameLifetimes (def, varDef))
	  {
	    CombineLifetimes (LIFETIME(def), LIFETIME(varDef), defTbl);
	    DisjMerge_rec (varDef, defTbl);
	  }
      }

}


static void
DisjMerge (PSS_Def phiDef, HashTable defTbl)
{
  assert (MERGE_TYPE (phiDef->type));
  DisjMerge_rec (phiDef, defTbl);
}


static int
DefHasNormalUse (PSS_Def def)
{
  int ret = 0;
  PSS_Use use;

  for (use = def->uses; use; use = use->next)
    {
      Expr parent = use->var->parentexpr;

      if (parent && parent->opcode != OP_phi)
	{
	  ret = 1;
	  break;
	}
    }
  
  return ret;
}

static void
PhiMerge_rec (PSS_Def def, HashTable defTbl)
{
  List rhsVars;
  VarDcl vdcl;
  Expr var;

  LIFETIME(def)->visited = 1;

  if (UNINITIALIZED_TYPE (def->type))
    return;

  rhsVars = PSS_GetSubExprByOpcode_List (def->var, OP_var);
  vdcl = PSI_GetVarDclEntry (def->var->value.var.key);

  List_start (rhsVars);
  while ((var = List_next (rhsVars)))
    if (PSI_GetVarDclEntry (var->value.var.key) == vdcl)
      {
	PSS_Def varDef = var->value.var.ssa;
	if (!LIFETIME(varDef)->visited && !DefHasNormalUse (varDef) &&
	    !SameLifetimes (def, varDef))
	  {
	    CombineLifetimes (LIFETIME(def), LIFETIME(varDef), defTbl);
	    PhiMerge_rec (varDef, defTbl);
	  }
      }

}

static void
PhiMerge (PSS_Def phiDef, HashTable defTbl)
{
  assert (MERGE_TYPE (phiDef->type));
  PhiMerge_rec (phiDef, defTbl);
}


/*! \brief Union the lifetime sets and point all the defs covered by the
 *   new set at it.
 */
static void
CombineLifetimes (Lifetime L1, Lifetime L2, HashTable defTbl)
{
  int subscr;
  SetIterator *si;

  L1->lifeSubscrs = Set_union_acc (L1->lifeSubscrs, L2->lifeSubscrs);
  //Set_dispose (L2->lifeSubscrs);
 
  si = Set_iterator (L1->lifeSubscrs, UNDEF_SUBSCR);
  for (subscr = Set_next (si); subscr != UNDEF_SUBSCR; subscr = Set_next (si))
    {
      PSS_Def def = HashTable_find (defTbl, subscr);
      LIFETIME(def)->lifeSubscrs = L1->lifeSubscrs;
    }
  
  si = Set_end_iterator (si);
  return;
}


static Lifetime
newLifetime (int subscr, void *ext)
{
  Lifetime lt = ALLOCATE (_Lifetime);
  
  lt->lifeSubscrs = Set_new ();
  lt->lifeSubscrs = Set_add (lt->lifeSubscrs, subscr);

  lt->num = UNDEF_SUBSCR;
  lt->visited = 0;

  lt->ext = ext;

  return lt;
}

static void*
freeLifetime (Lifetime lt, HashTable defTbl)
{
  void *ext = lt->ext;

  if (lt->lifeSubscrs)
    {
      Set s = lt->lifeSubscrs;
      SetIterator *si;
      int subscr;

#if 1
      si = Set_iterator (s, UNDEF_SUBSCR);
      for (subscr = Set_next (si); subscr != UNDEF_SUBSCR;
	   subscr = Set_next (si))
	{
	  PSS_Def def = HashTable_find (defTbl, subscr);
	  LIFETIME(def)->lifeSubscrs = NULL;
	}
      Set_end_iterator (si);

#else
      int i, sz, *buf;

      if ((sz = Set_size (s)))
	{
	  buf = alloca (sz * sizeof (int));
	  Set_2array (s, buf);

	  for (i = 0; i < sz; i++)
	    {
	      PSS_Def odef = HashTable_find (defTbl, buf[i]);
	      LIFETIME(odef)->lifeSubscrs = NULL;
	    }
	}


#endif
	
      Set_dispose (s);

    }

  DISPOSE (lt);

  return ext;
}
