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
/*****************************************************************************
 *
 *  File:  s_object.c
 *
 *  Description:  Object tracing support
 *
 *  Creation Date :  October 2000
 *
 *  Authors:  John W. Sias, Hillery Hunter
 *
 *  Revisions:
 *      HCH: Add single-direction stride formation 11/7/01
 *      HCH: Trace malloc/calloc objects 11/5/01
 *      HCH: Overhaul and re-write run formation 10/01
 *      HCH: Cut output by aprox. 0.5 3/6/01
 *      HCH: Output format modified 1/22/01
 *      HCH: Revisions/bug fixes 12/27/00
 *      HCH: Significant changes 11/13-21/00
 *
 *      Copyright (c) 2000 John W. Sias, Wen-mei Hwu and 
 *                         The Board of Trustees of the University of Illinois.
 *                         All rights reserved.
 *      The University of Illinois software License Agreement
 *      specifies the terms and conditions for redistribution.
\*****************************************************************************/

#include <config.h>
#include "s_main.h"
#include "s_object.h"

#undef PRINT_INDIVIDUAL_RWS 
#undef ONE_D_STRIDES 

#undef RECORD_RUN_ITERS

FILE *S_OBJ_OUT = NULL;
char *S_obj_out_file = "OBJTRACE";

static ITintmax S_traced_lp_writes = 0;
static ITintmax S_traced_lp_reads = 0;
static ITintmax S_missed_lp_writes = 0;
static ITintmax S_missed_lp_reads = 0;
static ITintmax S_traced_writes = 0;
static ITintmax S_traced_reads = 0;
static ITintmax S_missed_writes = 0;
static ITintmax S_missed_reads = 0;

static int last_lp_id;
int num_rd_collapses = 0;
int num_wt_collapses = 0;

static List S_program_stack = NULL;

#define USE_OA_CACHE 

#ifdef USE_OA_CACHE

#define OA_CACHESIZE 64

static S_Obj_Des * ObjCacheArray[OA_CACHESIZE];

#endif

static void
S_stack_push (S_Obj_Des * obj)
{
  S_program_stack = List_insert_first (S_program_stack, obj);
  return;
}

static S_Obj_Des *
S_stack_pop (void)
{
  S_Obj_Des *obj;
  obj = List_first (S_program_stack);
  S_program_stack = List_delete_current (S_program_stack);
  return obj;
}

/*
 * OBJECT TRACE INTERFACE
 */

void
S_init_object_trace (void)
{
  S_OBJ_OUT = fopen (S_obj_out_file, "w");

  S_init_page_table ();

  if (!S_OBJ_OUT)
    S_punt ("S_init_object_trace: Cannot open output file");

#ifdef USE_OA_CACHE

  {
    int i;

    /* Invalidate matching entries */

    for (i = 0; i < OA_CACHESIZE; i++)
      ObjCacheArray[i] = NULL;
  }

#endif
}

void
S_close_object_trace (void)
{
  fclose (S_OBJ_OUT);
}

void
S_dump_obj_des (S_Obj_Des * obj)
{
  char *tstr = NULL;

  if (obj->status == SOBJ_UNINIT)
    return;

  if (obj->type & L_TRACE_OBJ_GLOB)
    tstr = "G";
  else if (obj->type & L_TRACE_OBJ_STAK)
    tstr = "S";
  else if (obj->type & L_TRACE_OBJ_HEAP)
    tstr = "H";
  else
    S_punt ("S_dump_obj_des: bad type");
  
  fprintf (S_OBJ_OUT, "%s %08X %8d %s\n", 
	   tstr, obj->addr, obj->size, obj->name);

  if (Set_size (obj->rw_summ->readers) != 0)
    Set_print (S_OBJ_OUT, "r lps: ", obj->rw_summ->readers);
  if (Set_size (obj->rw_summ->writers) != 0)
    Set_print (S_OBJ_OUT, "w lps: ", obj->rw_summ->writers);
}

void
S_print_object_record (S_Obj_Des * obj)
{
  char *stat = NULL;

  switch (obj->status)
    {
    case SOBJ_UNINIT:
      stat = "UNIT";
      return;
    case SOBJ_WRITING:
      fprintf (S_OBJ_OUT, "%i ", obj->writes);
      stat = "WRIT";
      break;
    case SOBJ_READING:
      fprintf (S_OBJ_OUT, "%i ", obj->reads);
      stat = "READ";
      break;
    case SOBJ_READWRITE:
      fprintf (S_OBJ_OUT, "%i Rs %i Ws ", obj->reads, obj->writes);
      stat = " ";
      break;
    }
  fprintf (S_OBJ_OUT, "%s ", stat);

  return;
}

void
S_print_object_runs (S_Obj_Des * obj)
{

  S_Instance_Runs *inst;

  S_Obj_Acc_Run *run;
  char *type_summ1 = NULL, *type_summ2 = NULL, *stat_char = NULL;

  List_start (obj->instances);
  while ((inst = (S_Instance_Runs *) List_next (obj->instances)))
    {
      fprintf (S_OBJ_OUT, "L %i I %i\n", inst->context.loop_id,
	       inst->context.instance);

      List_start (inst->runs);
      while ((run = (S_Obj_Acc_Run *) List_next (inst->runs)))
	{
	  /* Runs should only be marked either read or write, not both */
	  if ((run->type & RUN_READ) && !(run->type & RUN_WRITE))
	    type_summ1 = "R";
	  else if (!(run->type & RUN_READ) && (run->type & RUN_WRITE))
	    type_summ1 = "W";
	  else
	    S_punt
	      ("S_print_object_runs: object with bad read/write type marking");

	  if (run->type & RUN_RANDOM)
	    type_summ2 = "Ra";
	  else if (run->type & RUN_STRIDE && !(run->single))
	    type_summ2 = "St";
	  else if (run->type & RUN_STRIDE && (run->single))
	    type_summ2 = "Si";
	  else
	    S_punt
	      ("S_print_object_runs: invalid object random/stride type\n");

	  fprintf (S_OBJ_OUT, "%s %s f %i l %i ", type_summ1,
		   type_summ2, run->foff, run->loff);

	  switch (run->status)
	    {
	    case READS_INIT_OBJ:
	      stat_char = "Ob";
	      break;
	    case READS_INIT_INST:
	      stat_char = "In";
	      break;
	    case READS_NONE:
	      stat_char = "No";
	      break;
	    case RUN_WRITE:
	      stat_char = "N/A";
	      break;
	    case READS_PARTIAL:
	      stat_char = "Pa";
	      break;
	    default:
	      S_punt ("s_object.c error: run status not initialized\n");
	    }
	  fprintf (S_OBJ_OUT, "%s ", stat_char);

	  fprintf (S_OBJ_OUT, "%i ", run->size);

	  fprintf (S_OBJ_OUT, "%i\n", run->coverageB);

#ifdef RECORD_RUN_ITERS
	  if (Set_size (run->iters) == 0)
	    S_punt("S_print_object_runs: iter info not marked on run");
	  Set_print(S_OBJ_OUT, "iters", run->iters);
#endif
	}
    }
  return;
}

void
S_read_obj_trace (Pnode * pnode)
{
  /* 10/25/04 REK Commenting out unused variable to quiet compiler warning. */
#if 0
  int tracewd;
#endif
  S_Obj_Des *od;
  S_obj_desc obj;

  while (S_peek_trace_word (pnode) == L_TRACE_ASYNCH)
    {
      S_get_trace_word (pnode);
      obj.type = S_get_trace_word (pnode);
      obj.loc = S_get_trace_word (pnode);
      obj.size = S_get_trace_word (pnode);
      obj.name = NULL;

      switch (obj.type)
	{
	case L_TRACE_OBJ_STAK:
	  obj.name = latest_fn_name;
	  if (obj.loc != 0)
	    {
	      /* New stack frame */
	      od = S_insert_object (obj.type, obj.loc, obj.size, obj.name);
	      S_stack_push (od);
	    }
	  else
	    {
	      od = S_stack_pop ();
	      if (!od)
		{
		  S_punt ("S_read_obj_trace: Pop of empty stack");
		}
	      if ((od->status) != SOBJ_UNINIT)
		{
		  if (S_trace_loop_id == DEFAULT_LOOP_ID)
		    {
		      S_print_object_record (od);
		      fprintf (S_OBJ_OUT, "X");
		      S_dump_obj_des (od);
		      S_print_object_runs (od);
		    }
		}
	      S_remove_object_by_ref (od);
	    }
	  break;
	case L_TRACE_OBJ_HEAP:
	  if(obj.size != 0)
	    {
	      S_insert_object (obj.type, obj.loc, obj.size, obj.name);
	    }
	  /* FIX: add handling of FREES; right now, if malloc'ed objects
	   * overlap, they'll be caught by an S_insert_near S_punt */
	  /*	  else 
	    {
	      od = S_find_heap_obj(obj);
	      if((od->status) != SOBJ_UNINIT)
		{
		  S_print_object_record (od);
		  fprintf (S_OBJ_OUT, "X");
		  S_dump_obj_des (od);
		  S_print_object_runs (od);
 		}
	      S_remove_object_by_ref (od);
	      } */
	  break; 
	case L_TRACE_OBJ_GLOB:
	  S_insert_object (obj.type, obj.loc, obj.size, obj.name);
	  break;
	default:
	  S_punt ("S_read_obj_trace: Bad object type");
	}
    }
  return;
}

/*
 * OBJECT DATABASE
 */

static S_Obj_Sec_Page_Tbl *spt;
static S_Obj_Des *firstobj;

void
S_init_page_table (void)
{
  firstobj = NULL;
  spt = malloc (sizeof (S_Obj_Sec_Page_Tbl));
  if (!spt)
    S_punt ("Unable to create secondary page table");
  memset (spt, 0, sizeof (S_Obj_Sec_Page_Tbl));
}

void
S_free_ppt (S_Obj_Pri_Page_Tbl * ppt)
{
  int i;
  for (i = 0; i < PTSIZE; i++)
    if (ppt->pdes[i])
      free (ppt->pdes[i]);

  free (ppt);
}

void
S_free_mem_map (void)
{
  int i;
  for (i = 0; i < PTSIZE; i++)
    if (spt->ppt[i])
      S_free_ppt (spt->ppt[i]);

  free (spt);

  firstobj = NULL;
  spt = NULL;
}

void
S_attach_object (S_Obj_Des * obj, S_Obj_Page_Des * page)
{
  if (!page->fobj)
    page->fobj = obj;
  return;
}

void
S_insert_near (S_Obj_Des * obj, S_Obj_Des * near_obj)
{
  int start, end;
  S_Obj_Des *lt = NULL, *rt = NULL;
  start = obj->addr;
  end = obj->addr + obj->size - 1;

  if (!near_obj)
    {
      if (firstobj)
	fprintf (stderr, "Warning: defaulting to firstobj\n");

      near_obj = firstobj;
    }

  if (near_obj)
    {
      if (near_obj->addr <= obj->addr)
	{
	  rt = near_obj->next;
	  lt = near_obj;
	  while (rt && rt->addr <= end)
	    {
	      lt = rt;
	      rt = rt->next;
	    }
	}
      else
	{
	  rt = near_obj;
	  lt = near_obj->prev;
	  while (lt && (lt->addr + lt->size - 1) >= start)
	    {
	      rt = lt;
	      lt = lt->prev;
	    }
	}
    }
  if ((lt && ((lt->addr + lt->size - 1) >= start)) ||
      (rt && (rt->addr <= end)))
    {
      if (lt && (lt->addr == obj->addr) && (lt->size == obj->size))
	;   /*  fprintf(S_OBJ_OUT, "DUPL %08X %8d\n", obj->addr, obj->size); */
      else if (rt && (rt->addr == obj->addr) && (rt->size == obj->size))
	;   /*  fprintf(S_OBJ_OUT, "DUPL %08X %8d\n", obj->addr, obj->size); */

      else
	  S_punt ("S_insert_near: Tried to insert an overlapping object"); 

      free (obj->rw_summ);
      free (obj);

#ifdef USE_OA_CACHE

      {
	int i;
	
	/* Invalidate matching entries */

	for (i = 0; i < OA_CACHESIZE; i++)
	  if (ObjCacheArray[i] == obj)
	    ObjCacheArray[i] = NULL;
      }

#endif

      obj = NULL;
    }
  else
    {
      obj->prev = lt;
      obj->next = rt;
      if (lt)
	lt->next = obj;
      else
	firstobj = obj;
      if (rt)
	rt->prev = obj;
    }
  return;
}

S_Obj_Page_Des *
S_new_pdes (int i)
{
  S_Obj_Page_Des *pdes;

  pdes = malloc (sizeof (S_Obj_Page_Des));
  pdes->entry = i;
  pdes->fobj = NULL;
  return pdes;
}

S_Obj_Pri_Page_Tbl *
S_new_ppt (int i)
{
  S_Obj_Pri_Page_Tbl *ppt;

  ppt = malloc (sizeof (S_Obj_Pri_Page_Tbl));
  if (!ppt)
    S_punt ("Unable to create primary page table");
  memset (ppt, 0, sizeof (S_Obj_Pri_Page_Tbl));
  ppt->entry = i;
  return ppt;
}

static int
find_pdes_near (S_Obj_Pri_Page_Tbl * ppt, S_Obj_Des * obj, int order)
{
  int ppte = -1, i;

  if (order < 0)
    {
      for (i = (PTSIZE - 1); i >= 0; i--)
	if (ppt->pdes[i] && ppt->pdes[i]->fobj && ppt->pdes[i]->fobj != obj)
	  {
	    ppte = i;
	    break;
	  }
    }
  else
    {
      for (i = 0; i < PTSIZE; i++)
	if (ppt->pdes[i] && ppt->pdes[i]->fobj && ppt->pdes[i]->fobj != obj)
	  {
	    ppte = i;
	    break;
	  }
    }
  if ((ppte >= PTSIZE) || (ppte < 0))
    ppte = -1;
  return ppte;
}

S_Obj_Des *
S_insert_object (int type, int start, int size, char * name)
{
  int end;
  int sptf, sptl, pptf, pptl;
  /* page table entries leading to first attempted insertion pt */
  int spte = -1, ppte = -1;
  int i, j;
  S_Obj_Pri_Page_Tbl *ppt = NULL;
  S_Obj_Des *obj, *srch;
  S_Obj_Page_Des *pdes;
  S_Obj_RW_Summ *new_summ;

  if (size <= 0)
    S_punt ("S_insert_object: size <= 0");

  end = start + size - 1;

  /* Create a new object descriptor */

  obj = malloc (sizeof (S_Obj_Des));
  if (!obj)
    S_punt ("S_insert_object: could not malloc object descriptor");

  obj->type = type;
  obj->addr = start;
  obj->size = size;
  obj->status = SOBJ_UNINIT;
  obj->name = name;
  obj->writes = 0;
  obj->reads = 0;
  obj->instances = NULL;

  new_summ = malloc (sizeof (S_Obj_RW_Summ));
  new_summ->writers = NULL;
  new_summ->readers = NULL;
  obj->rw_summ = new_summ;

  /* Insert into the page table */

  sptf = (start & SPTMASK) >> 22;
  sptl = (end & SPTMASK) >> 22;

  for (i = sptf; i <= sptl; i++)
    {
      if (!(ppt = spt->ppt[i]))
	ppt = spt->ppt[i] = S_new_ppt (i);
      else if (spte == -1)
	spte = i;

      if (i == sptf)
	pptf = (start & PPTMASK) >> 12;
      else
	pptf = 0;
      if (i == sptl)
	pptl = (end & PPTMASK) >> 12;
      else
	pptl = PTSIZE - 1;
      for (j = pptf; j <= pptl; j++)
	{
	  pdes = ppt->pdes[j];
	  if (!pdes)
	    pdes = ppt->pdes[j] = S_new_pdes (j);
	  else if (ppte == -1)
	    {
	      spte = i;
	      ppte = j;
	    }
	  S_attach_object (obj, pdes);
	}
    }

  /* Find list insertion pt for object */

  sptf = spte;

  /* Need to search spte for nearby non-empty ppt */
  for (j = 0; j < (PTSIZE - 1); j++)
    {
      if (((sptf - j) >= 0) && spt->ppt[sptf - j])
	{
	  spte = sptf - j;
	  ppt = spt->ppt[spte];
	  if ((ppte = find_pdes_near (ppt, obj, -1)) != -1)
	    break;
	}

      if (((sptf + j) < PTSIZE) && spt->ppt[sptf + j])
	{
	  spte = sptf + j;
	  ppt = spt->ppt[spte];
	  if ((ppte = find_pdes_near (ppt, obj, 1)) != -1)
	    break;
	}
    }

  if (ppte != -1)
    srch = ppt->pdes[ppte]->fobj;
  else
    srch = NULL;

  /* search from srch to find right insertion place */

  S_insert_near (obj, srch);

  return obj;
}

void
S_find_object (S_Obj_Des **pobj, int addr)
{
  int spte, ppte;
  S_Obj_Des *iter, *obj;
  S_Obj_Pri_Page_Tbl *ppt;
  S_Obj_Page_Des *pdes;

  obj = *pobj;

  if (obj && (addr >= obj->addr) && (addr < (obj->addr + obj->size)))
    return;

  obj = NULL;

  spte = (addr & SPTMASK) >> 22;

  ppt = spt->ppt[spte];
  if (ppt)
    {
      ppte = (addr & PPTMASK) >> 12;

      pdes = ppt->pdes[ppte];

      if (pdes)
	{
	  iter = pdes->fobj;

	  if (iter->addr > addr)
	    {
	      while (iter && (iter->addr > addr))
		iter = iter->prev;
	    }
	  else
	    {
	      while (iter && ((iter->addr + iter->size - 1) < addr))
		iter = iter->next;
	    }

	  if (iter &&
	      (addr >= iter->addr) && (addr < iter->addr + iter->size))
	    obj = iter;
	}
    }

  if (obj && ((addr < obj->addr) || (addr >= obj->addr + obj->size)))
    S_punt ("S_find_object is broken");

  *pobj = obj;

  return;
}

S_Obj_Des *
S_remove_object (int type, int start)
{
  S_Obj_Des *obj = NULL;
  S_find_object (&obj, start);

  if (!obj)
    S_punt ("Attempt to remove a non-existent object");

  if (obj->addr != start)
    S_punt ("S_remove_object: Attempt to remove "
	    "a partially overlapping object");

  obj = S_remove_object_by_ref (obj);
  return obj;
}

S_Obj_Des *
S_remove_object_by_ref (S_Obj_Des * obj)
{
  int start, end;
  int sptf, sptl, pptf, pptl;
  /* 10/25/04 REK Commenting out unused variables to quiet compiler warnings.
   */
#if 0
  int spte = -1, ppte = -1;
#endif
  int i, j;
  S_Obj_Pri_Page_Tbl *ppt;
  S_Obj_Page_Des *pdes;

  if (!obj)
    S_punt ("Attempt to remove a non-existent object");

  start = obj->addr;

  /* iterate over affected pages, removing references as necessary. */

  end = start + obj->size - 1;

  sptf = (start & SPTMASK) >> 22;
  sptl = (end & SPTMASK) >> 22;

  for (i = sptf; i <= sptl; i++)
    {
      if (!(ppt = spt->ppt[i]))
	S_punt ("Expected to find a non-empty secondary page");

      pptf = (i == sptf) ? (start & PPTMASK) >> 12 : 0;
      pptl = (i == sptl) ? (end & PPTMASK) >> 12 : PTSIZE - 1;

      for (j = pptf; j <= pptl; j++)
	{
	  pdes = ppt->pdes[j];
	  if (!pdes)
	    S_punt ("Expected to find a non-empty primary page");
	  if (pdes->fobj == obj)
	    {
	      unsigned int pstart, pend;

	      /* Move fobj to a valid object in the page
	       * if one exists, or delete the page 
	       */

	      pstart = (i << 22) + (j << 12);
	      pend = pstart + 0xFFF;
	      if (obj->prev && ((obj->prev->addr +
				 obj->prev->size - 1) >= pstart))
		pdes->fobj = obj->prev;
	      else if (obj->next && obj->next->addr <= pend)
		pdes->fobj = obj;
	      else
		{
		  /* Page is now empty */
		  ppt->pdes[j] = NULL;
		  free (pdes);
		}
	    }
	}
    }

  /* fix up object list */

  if (obj->prev)
    obj->prev->next = obj->next;
  else
    firstobj = obj->next;

  if (obj->next)
    obj->next->prev = obj->prev;

  free (obj->rw_summ);
  free (obj);

#ifdef USE_OA_CACHE

  {
    int i;

    /* Invalidate matching entries */

    for (i = 0; i < OA_CACHESIZE; i++)
      if (ObjCacheArray[i] == obj)
	ObjCacheArray[i] = NULL;
  }

#endif

  return NULL;
}

void 
S_add_new_inst(S_Obj_Des * obj, Sint * sint)
{
  S_Instance_Runs *inst;

  inst = malloc (sizeof (S_Instance_Runs));
  inst->context.loop_id = sint->oper->loop->loop_id;
  inst->context.instance = sint->oper->loop->instance;
  inst->runs = NULL;
  inst->num_rruns = 0;
  inst->num_wruns = 0;
  inst->new_rruns_ok = YES;
  inst->new_wruns_ok = YES;
  obj->instances = List_insert_last (obj->instances, inst);
  
  return;
}

void
S_add_new_run(S_Instance_Runs * inst, 
	      int access_type, int offset, int access_size)
{
  S_Obj_Acc_Run *run;

  run = malloc (sizeof (S_Obj_Acc_Run));
  run->type = RUN_UNINIT;
  run->type |= RUN_STRIDE;    /* Assume stride access until striding fails */
  run->single = YES;
  run->type |= access_type;
  run->foff = offset;
  run->loff = offset + access_size - 1;
  run->stride = 1;	     /* Note: default stride currently 1 access size */
  run->size = access_size;
  run->coverageB = access_size;
  run->status = RUN_UNINIT;
  run->iters = NULL;
  run->direction = RUN_UNINIT;
  
  inst->runs = List_insert_last (inst->runs, run);
  if (access_type == RUN_READ)
      inst->num_rruns++;
  else if (access_type == RUN_WRITE)
      inst->num_wruns++;
  else
    S_punt ("S_add_new_run: invalid access_type parameter\n");

  return;
}

/* FIX: stride currently only allowed to be 1 access size
 *   -> "RUN_STRIDE" is really contiguous access 
 *   -> Strides are also required to consist of accesses of same size */
int
S_is_stride_access(Sint * sint, S_Obj_Acc_Run * run, int offset)
{
  int diff1, diff2, temp;

  if (sint->access_size == run->size)
    {
      /* distance access occured spacially after current run */
      diff1 = offset - run->loff;  
      /* distance access occured spacially before current run */
      diff2 = run->foff - offset;  
      temp = run->stride * run->size;
      /* Found run now if type match and stride found */
#ifdef ONE_D_STRIDES
      if (run->direction == RUN_UNINIT)
	{
	  if (diff1 == 1)
	    {
	      run->direction = FORWARD;
	      return YES;
	    }
	  else if (diff2 == temp)
	    {
	      run->direction = BACKWARD;
	      return YES;
	    }
	}
      else if ( ((run->direction == FORWARD) && (diff1 == 1)) ||
	   ((run->direction == BACKWARD) && (diff2 == temp)) )
	return YES;
#else      
      if ((diff1 == 1)             /* check for forward stride */
	  || (diff2 == temp))      /* check for backward stride */
	return YES;
#endif
    }
  return NO;
}

int 
S_find_writer (S_Obj_Des * obj, int offset, int access_size)
{
  int is_last_instance = YES;  
  int wrun_found = NO;
  int init_type = UNINIT;

  S_Instance_Runs *inst = NULL;
  S_Obj_Acc_Run *wrun = NULL;

  List_start (obj->instances);
  
  while ((inst = (S_Instance_Runs *) List_prev (obj->instances)))
    {
      List_start (inst->runs);
      while ((wrun = (S_Obj_Acc_Run *) List_prev (inst->runs)))
	{
	  /* FIX: should this be ((wrun->type & RUN_WRITE)&&
	   * (wrun->status != RUN_RANDOM)) so that merged 
	   * write runs do not appear as writers of regions they 
	   * don't touch? */
	  if (wrun->type & RUN_WRITE)
	    {
	      if (((offset + access_size - 1) <=  (wrun->loff))  &&
		  (offset >= wrun->foff))
		{
		  if(is_last_instance == YES)
		    init_type = INSTANCE;
		  else
		    init_type = OBJECT;
		  wrun_found = YES;
		  break;
		}
	    }
	}
      if(wrun_found)
	break;
      is_last_instance = NO;
    }
  return init_type;
}

void
S_set_new_run_status (int init_type, S_Obj_Acc_Run * run)
{
  if (init_type)
    {
      switch (init_type)
	{
	case OBJECT:
	  switch (run->status)
	    {
	    case READS_INIT_INST:
	    case RUN_UNINIT:
	      run->status = READS_INIT_OBJ;
	      break;
	    case READS_NONE:
	      run->status = READS_PARTIAL;
	      break;
	    default:    /* READS_INIT_OBJ, READS_PARTIAL stay same */
	      break;
	    }
	  break;
	case INSTANCE:
	  switch (run->status)
	    {
	    case READS_NONE:
	      run->status = READS_PARTIAL;
	      break;
	    case RUN_UNINIT:
	      run->status = READS_INIT_INST;
	      break;
	    default: /* READS_INIT_INST, READS_INIT_OBJ, READS_PARTIAL */
	      break;
	    }
	  break;
	default:
	  S_punt("S_set_new_run_status: init_type invalid.");
	  break;
	}
    }
  /*If no init_type, there's no previous write covering current read */
  else
    {
      switch (run->status)
	{
	case RUN_UNINIT:
	  run->status = READS_NONE;
	  break;
	case READS_INIT_INST:
	case READS_INIT_OBJ:
	  run->status = READS_PARTIAL;
	  break;
	default:
	  break;
	}
    }
  return;
}

/* Remember that S_record_object_write_run parallels this function; 
 * make sure to make changes there if appropriate when this function
 * is modified */
void
S_record_object_read_run (Sint * sint, S_Obj_Des * obj)
{
  S_Instance_Runs *inst = NULL;
  S_Obj_Acc_Run *run = NULL;

  int offset; 
  int access_size;
  int init_type1 = UNINIT;
  int init_type2 = UNINIT;
  int stay_single = NO;

  offset = sint->trace.mem_addr - obj->addr;
  access_size = sint->access_size;
  
  /* Set last loop id for next call to S_record_object_[read,write]_run */
  last_lp_id = sint->oper->loop->loop_id;
  
  /* Get last instance in object list; if it doesn't match by    
   * id, loop instance or id of last loop, force creation 
   * of a new instance record  */
  inst = (S_Instance_Runs *) List_last (obj->instances);
  if (!inst 
      || (sint->oper->loop->instance != inst->context.instance) 
      || (sint->oper->loop->loop_id != inst->context.loop_id) 
      || (sint->oper->loop->loop_id != last_lp_id))
    {
      S_add_new_inst(obj, sint);
      inst = (S_Instance_Runs *) List_last(obj->instances);
      S_add_new_run(inst, RUN_READ, offset, access_size);
      run = (S_Obj_Acc_Run *) List_last(inst->runs);
    }
  /* FIX: should still save largest existing runs, and collapse excess
     runs into one random run rather than merging all of them */
  /* If no new runs allowed, collapse runs; fix foff/loff, return */
  else if (!(inst->new_rruns_ok))
    {
      /* Collapse runs */
      int max_loff;
      int min_foff;
      int num_removed = 0;
      int num_init_obj = 0;
      int num_init_inst = 0;
      int num_partial = 0;
      int num_none = 0;
      int tot_coverageB = 0;
      
      /* Set offsets at opposite extremes of object */
      min_foff = obj->size - 1;
      max_loff = 0;
      
      List_start (inst->runs);
      while ((run = (S_Obj_Acc_Run *) List_next (inst->runs)))
	{
	  if (!(run->type & RUN_READ))
	    continue;
	  if (run->foff < min_foff)
	    min_foff = run->foff;
	  if (run->loff > max_loff)
	    max_loff = run->loff;

	  switch(run->status)
	    {
	    case READS_INIT_OBJ:
	      num_init_obj++;
	      break;
	    case READS_INIT_INST:
	      num_init_inst++;
	      break;
	    case READS_PARTIAL:
	      num_partial++;
	      break;
	    case READS_NONE:
	      num_none++;
	      break;
	    default:
	      S_punt("S_record_object_read_run: invalid run->status");
	    }
	  
	  tot_coverageB += run->coverageB;
	  
	  if (num_removed != (inst->num_rruns - 1))
	    {
	      inst->runs = List_remove (inst->runs, run);
	      num_removed++;
	    }
	  
	  if (inst->runs == NULL)
	    S_punt ("inst->runs is NULL");

	}
      
      inst->num_rruns -= num_removed;
      
      /* add max and min info to only remaining run of its type */
      List_start (inst->runs);
      while ((run = (S_Obj_Acc_Run *) List_next (inst->runs)))
	{
	  if (run->type & RUN_READ)
	    break;
	}
      run->foff = min_foff;
      run->loff = max_loff;
      
      /* Total run status is only INIT if all collapsed runs were also INIT */
      if ((num_partial == 0) && (num_none == 0))
	{
	  if (num_init_obj != 0)
	    run->status = READS_INIT_OBJ;
	  else
	    run->status = READS_INIT_INST;
	}
      /* If at least 1 run was PARTIAL, collapsed run is now PARTIAL */
      else if (num_partial != 0)
	{
	  run->status = READS_PARTIAL;
	}
      else
	{
	  run->status = READS_NONE;
	}
      if ((offset + access_size - 1) > run->loff)
	run->loff = offset + access_size - 1;
      if (offset < run->foff)
	run->foff = offset;
#ifdef RECORD_RUN_ITERS      
      run->iters = Set_add (run->iters, sint->oper->loop->iter);	  
#endif
      init_type1 = S_find_writer (obj, offset, access_size);
      S_set_new_run_status (init_type1, run);
      
      if (run->single)
	run->single = NO;

      run->type |= RUN_RANDOM;

      run->coverageB = tot_coverageB;

      return;
    }
  else
    {
      List_start (inst->runs);
      while ((run = (S_Obj_Acc_Run *) List_next (inst->runs)))
	{
	  if(run->type & RUN_READ)
	    {
	      /* Split these cases so matching accesses keep single=YES */
	      if (S_is_stride_access(sint, run, offset))
		{
		  if(!stay_single) 
		    run->single = NO;
		  break;
		} 
	      else if ((run->foff == offset) &&
		       (run->loff == (offset + access_size - 1)))
		{
		  /* NOTE: run->single should perhaps be NO here */
		  break;
		}
	    }
	}

      if (!run)
	{
	  S_add_new_run(inst, RUN_READ, offset, access_size);
	  run = (S_Obj_Acc_Run *) List_last(inst->runs);
	}
      else 
	{
	  run->coverageB += access_size;
	}

      if ((offset + access_size - 1) > run->loff)
	run->loff = offset + access_size - 1;
      
      if (offset < run->foff)
	run->foff = offset;
    }

#ifdef RECORD_RUN_ITERS      
  run->iters = Set_add (run->iters, sint->oper->loop->iter);
#endif

  init_type2 = S_find_writer (obj, offset, access_size);
  S_set_new_run_status (init_type2, run);
  
  if (inst->num_rruns >= MAX_RRUNS_OKAY)
    {
      /* 10/25/04 REK Commenting out unused variable to quiet compiler
       *          warning. */
#if 0
      S_Obj_Acc_Run *run2;
#endif
      List_start (inst->runs);
      /*      while ((run2 = (S_Obj_Acc_Run *) List_next (inst->runs))
	     && (run2->type & RUN_READ))
	     run2->type |= RUN_RANDOM; */
      inst->new_rruns_ok = NO;

      /* DEBUG */
      num_rd_collapses++;
    }

  return;
}

void
S_record_object_write_run (Sint * sint, S_Obj_Des * obj)
{

  S_Instance_Runs *inst = NULL;
  S_Obj_Acc_Run *run = NULL;

  int offset; 
  int access_size;
  /* 10/25/04 REK Commenting out unused variables to quiet compiler warnings.
   */
#if 0
  int init_type1 = UNINIT;
  int init_type2 = UNINIT;
#endif
  int stay_single = NO;

  offset = sint->trace.mem_addr - obj->addr;
  access_size = sint->access_size;
  
  /* Set last loop id for next call to S_record_object_[read,write]_run */
  last_lp_id = sint->oper->loop->loop_id;
  
  /* Get last instance in object list; if it doesn't match by    
   * id, loop instance or id of last loop, force creation 
   * of a new instance record  */
  inst = (S_Instance_Runs *) List_last (obj->instances);
  if (!inst 
      || (sint->oper->loop->instance != inst->context.instance) 
      || (sint->oper->loop->loop_id != inst->context.loop_id) 
      || (sint->oper->loop->loop_id != last_lp_id))
    {
      S_add_new_inst(obj, sint);
      inst = (S_Instance_Runs *) List_last(obj->instances);
      S_add_new_run(inst, RUN_WRITE, offset, access_size);
      run = (S_Obj_Acc_Run *) List_last(inst->runs);
    }
  /* FIX: should still save largest existing runs, and collapse excess
     runs into one random run rather than merging all of them */
  /* If no new runs allowed, collapse runs; fix foff/loff, return */
  else if (!(inst->new_wruns_ok))
    {
      /* Collapse runs */
      int max_loff;
      int min_foff;
      int num_removed = 0;
      /* 10/25/04 REK Commenting out unused variables to quiet compiler
       *              warnings. */
#if 0
      int num_init_obj = 0;
      int num_init_inst = 0;
      int num_partial = 0;
      int num_none = 0;
#endif
      int tot_coverageB = 0;

      /* Set offsets at opposite extremes of object */
      min_foff = obj->size - 1;
      max_loff = 0;
      
      List_start (inst->runs);
      while ((run = (S_Obj_Acc_Run *) List_next (inst->runs)))
	{
	  if (!(run->type & RUN_WRITE))
	    continue;
	  if (run->foff < min_foff)
	    min_foff = run->foff;
	  if (run->loff > max_loff)
	    max_loff = run->loff;
	  
	  tot_coverageB += run->coverageB;

	  if (num_removed != (inst->num_wruns - 1))
	    {
	      inst->runs = List_remove (inst->runs, run);
	      num_removed++;
	    }
	  
	  if (inst->runs == NULL)
	    S_punt ("inst->runs is NULL");
	}
      
      inst->num_wruns -= num_removed;
      
      /* add max and min info to only remaining run of its type */
      List_start (inst->runs);
      while ((run = (S_Obj_Acc_Run *) List_next (inst->runs)))
	{
	  if (run->type & RUN_WRITE)
	    break;
	}
      run->foff = min_foff;
      run->loff = max_loff;
      
      if ((offset + access_size - 1) > run->loff)
	run->loff = offset + access_size - 1;
      if (offset < run->foff)
	run->foff = offset;

#ifdef RECORD_RUN_ITERS      
      run->iters = Set_add (run->iters, sint->oper->loop->iter);
#endif

      run->status = RUN_WRITE;
      
      if (run->single)
	run->single = NO;
      
      run->type |= RUN_RANDOM;

      run->coverageB = tot_coverageB;

      return;
    }
  else
    {
      List_start (inst->runs);
      while ((run = (S_Obj_Acc_Run *) List_next (inst->runs)))
	{
	  if(run->type & RUN_WRITE)
	    {
	      /* Split these cases so matching accesses keep single=YES */
	      if (S_is_stride_access(sint, run, offset))
		{
		  if(!stay_single) 
		    run->single = NO;
		  break;
		} 
	      else if ((run->foff == offset) &&
		       (run->loff == (offset + access_size - 1)))
		{
		  break;
		}
	    }
	}

      if (!run)
	{
	  S_add_new_run(inst, RUN_WRITE, offset, access_size);
	  run = (S_Obj_Acc_Run *) List_last(inst->runs);
	}
      else 
	{
	  run->coverageB += access_size;
	}
      
      if ((offset + access_size - 1) > run->loff)
	run->loff = offset + access_size - 1;
      
      if (offset < run->foff)
	run->foff = offset;
    }

#ifdef RECORD_RUN_ITERS      
  run->iters = Set_add (run->iters, sint->oper->loop->iter);
#endif

  run->status = RUN_WRITE;
  
  if (inst->num_wruns >= MAX_WRUNS_OKAY)
    {
      /* 10/25/04 REK Commenting out unused variable to quiet compiler
       *          warning. */
#if 0
      S_Obj_Acc_Run *run2;
#endif
      List_start (inst->runs);
      /*      while ((run2 = (S_Obj_Acc_Run *) List_next (inst->runs))
	     && (run2->type & RUN_WRITE))
	     run2->type |= RUN_RANDOM; */
      inst->new_wruns_ok = NO;

      /* DEBUG */
      num_wt_collapses++;
    }

  return;
}

void
S_dump_load (Sint * sint)
{
  S_Obj_Des *obj;
  S_Loop *loop = sint->oper->loop;

#ifdef USE_OA_CACHE
  obj = ObjCacheArray[sint->oper->pc % OA_CACHESIZE];
#endif

  S_find_object (&obj, sint->trace.mem_addr);

#ifdef USE_OA_CACHE
  ObjCacheArray[sint->oper->pc % OA_CACHESIZE] = obj;
#endif

  if (obj)
    {
      switch (obj->status)
	{
	case SOBJ_READING:
	case SOBJ_READWRITE:
	  break;
	case SOBJ_UNINIT:
	  obj->status = SOBJ_READING;
	  break;
	case SOBJ_WRITING:
	  obj->status = SOBJ_READWRITE;
	  break;
	default:
	  S_punt ("Illegal object status");
	}

      S_traced_reads++;
      obj->reads++;
      if (loop)
	{
	  S_traced_lp_reads++;
	  obj->rw_summ->readers =
	    Set_add (obj->rw_summ->readers, loop->loop_id);
	  S_record_object_read_run (sint, obj);
	}
    }
  else
    {
      S_missed_reads++;
      if (loop)
	{
	  fprintf (S_OBJ_OUT, "DEBUG: MISSED READ %08X %8d %s %i %i %i %i %i\n",
		   sint->trace.mem_addr,
		   sint->access_size,
		   sint->fn->name, 
		   loop->loop_id, loop->instance, loop->iter,
		   sint->oper->lcode_id, sint->oper->opc);
	S_missed_lp_reads++;
	}

    }

  /* S_trace_loop_id is DEFAULT_LOOP_ID if full tracing is to be done */
  if((loop) && (S_trace_loop_id != DEFAULT_LOOP_ID) 
     && (loop->loop_id == S_trace_loop_id))
    {
      fprintf (S_OBJ_OUT, "R %08X %8d %i %i\n",
	       sint->trace.mem_addr,
	       sint->access_size,
	       loop->instance, loop->iter);
    }

#ifdef PRINT_INDIVIDUAL_RWS
  if (!loop)
    fprintf (S_OBJ_OUT, "R %08X %8d %s\n",
	     sint->trace.mem_addr, sint->access_size, sint->fn->name);
  else
    fprintf (S_OBJ_OUT, "R %08X %8d %s %i %i %i op %i\n",
	     sint->trace.mem_addr,
	     sint->access_size,
	     sint->fn->name, loop->loop_id, loop->instance, loop->iter,
	     sint->oper->lcode_id);
#endif
  return;
}

void
S_dump_store (Sint * sint)
{
  S_Obj_Des *obj;
  S_Loop *loop = sint->oper->loop;

#ifdef USE_OA_CACHE
  obj = ObjCacheArray[sint->oper->pc % OA_CACHESIZE];
#endif

  S_find_object (&obj, sint->trace.mem_addr);

#ifdef USE_OA_CACHE
  ObjCacheArray[sint->oper->pc % OA_CACHESIZE] = obj;
#endif

  if (obj)
    {
      switch (obj->status)
	{
	case SOBJ_WRITING:
	case SOBJ_READWRITE:
	  break;
	case SOBJ_UNINIT:
	  obj->status = SOBJ_WRITING;
	  break;
	case SOBJ_READING:
	  obj->status = SOBJ_READWRITE;
	  break;
	default:
	  S_punt ("Illegal object status");
	}
      S_traced_writes++;
      obj->writes++;

      if (loop)
	{
	  S_traced_lp_writes++;
	  obj->rw_summ->writers =
	    Set_add (obj->rw_summ->writers, loop->loop_id);
	  S_record_object_write_run (sint, obj);
	}
    }
  else
    {
      S_missed_writes++;
      if (loop)
	{
	  S_missed_lp_writes++;
	  /* DEBUG */
	  fprintf (S_OBJ_OUT, "DEBUG: MISSED WRITE %08X %8d %s %i %i %i %i %i\n",
		   sint->trace.mem_addr,
		   sint->access_size,
		   sint->fn->name, 
		   loop->loop_id, loop->instance, loop->iter,
		   sint->oper->lcode_id, sint->oper->opc);
		   
	}
    }

  /* S_trace_loop_id is initialized to DEFAULT_LOOP_ID */
  if((loop) && (S_trace_loop_id != DEFAULT_LOOP_ID) 
     && (loop->loop_id == S_trace_loop_id))
    {
      fprintf (S_OBJ_OUT, "W %08X %8d %i %i\n",
	       sint->trace.mem_addr,
	       sint->access_size,
	       loop->instance, loop->iter);
    }

#ifdef PRINT_INDIVIDUAL_RWS
  if (!loop)
    fprintf (S_OBJ_OUT, "W %08X %8d %s\n",
	     sint->trace.mem_addr, sint->access_size, sint->fn->name);
  else
    fprintf (S_OBJ_OUT, "W %08X %8d %s %i %i %i, op %i\n",
	     sint->trace.mem_addr,
	     sint->access_size,
	     sint->fn->name, loop->loop_id, loop->instance, loop->iter,
	     sint->oper->lcode_id);
#endif
  return;
}

void
S_clear_stack (void)
{
  S_Obj_Des *od;

  od = S_stack_pop ();
  while (od)
    {
      if (od->status != SOBJ_UNINIT)
	{
	  S_print_object_record (od);
	  fprintf (S_OBJ_OUT, "X");
	  S_dump_obj_des (od);
	  S_print_object_runs (od);
	}
      S_remove_object_by_ref (od);
      od = S_stack_pop ();
    }
  return;
}

void
S_clear_pages (void)
{
  S_Obj_Des *od, *nod;

  od = firstobj;
  while (od)
    {
      nod = od->next;
      if (od->status != SOBJ_UNINIT)
	{
	  S_print_object_record (od);
	  fprintf (S_OBJ_OUT, "X");
	  S_dump_obj_des (od);
	  S_print_object_runs (od);
	}
      S_remove_object_by_ref (od);
      od = nod;
    }
  return;
}

void
S_print_object_report (void)
{
  /* NOTE: standard traced info represents the number of lds/sts which
   * "hit" in objects, irregardless of whether the access comes from
   * a loop. FROM LOOPS info counts loop lds/sts which hit in objects,
   * though the loop nesting level is not taken into account, so this 
   * is an upper bound on percent of program memory behavior attributable
   * to loops.
   */

  fprintf (S_OBJ_OUT, "OBJTR_SUMMARY\n");
  fprintf (S_OBJ_OUT, "READS: " ITintmaxformat " / "
	   ITintmaxformat " TRACED\n", S_traced_reads,
	   S_traced_reads + S_missed_reads);
  fprintf (S_OBJ_OUT, "WRITES: " ITintmaxformat " / "
	   ITintmaxformat " TRACED\n", S_traced_writes,
	   S_traced_writes + S_missed_writes);
  fprintf (S_OBJ_OUT, "FROM LOOPS - READS: " ITintmaxformat " / "
	   ITintmaxformat " TRACED\n", S_traced_lp_reads,
	   S_traced_reads + S_missed_reads);
  fprintf (S_OBJ_OUT, "FROM LOOPS - WRITES: " ITintmaxformat " / "
	   ITintmaxformat " TRACED\n", S_traced_lp_writes,
	   S_traced_writes + S_missed_writes);
  fprintf (S_OBJ_OUT, "FROM LOOPS - READS MISSED: " ITintmaxformat " / "
	   ITintmaxformat "\n", S_missed_lp_reads,
	   S_traced_lp_reads + S_missed_lp_reads);
  fprintf (S_OBJ_OUT, "FROM LOOPS - WRITES MISSED: " ITintmaxformat " / "
	   ITintmaxformat "\n", S_missed_lp_writes,
	   S_traced_lp_writes + S_missed_lp_writes);
  return;
}
