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
 *  File:  s_object.h
 *
 *  Description:  Object tracing support
 *
 *  Creation Date :  October 2000
 *
 *  Author:  John W. Sias
 *
 *  Revisions:
 *      HCH: Significant changes 11/13-21/2000
 *
 *      Copyright (c) 2000 John W. Sias, Wen-mei Hwu and 
 *                         The Board of Trustees of the University of Illinois.
 *                         All rights reserved.
 *      The University of Illinois software License Agreement
 *      specifies the terms and conditions for redistribution.
\*****************************************************************************/


#ifndef _LSIM_S_OBJECT_H_
#define _LSIM_S_OBJECT_H_

#include <config.h>
#include <library/set.h>

/* This is used for single-loop tracing (see note in s_main.c) */
#define DEFAULT_LOOP_ID -2

/* Define bounds after which runs must be combined */
#define MAX_RRUNS_OKAY 35
#define MAX_WRUNS_OKAY 35

#define PAGESIZE 4096
#define PTSIZE 1024
#define PTBITS 10

#define SPTMASK 0xFFC00000L
#define PPTMASK 0x003FF000L
#define PAGMASK 0x00000FFFL

#define S_is_asynch_tr(tracewd) ((tracewd)==L_TRACE_ASYNCH)

extern int num_rd_collapses;
extern int num_wt_collapses;
extern char *latest_fn;

/*
 * OBJECT DATABASE
 */

typedef struct _S_Obj_Context
{
  int loop_id;
  int instance;
  int iter;
}
S_Obj_Context;

#define SOBJ_UNINIT    0
#define SOBJ_WRITING   1
#define SOBJ_READING   2
#define SOBJ_READWRITE 4

typedef struct _S_Obj_RW_Summ
{
  Set writers;
  Set readers;
}
S_Obj_RW_Summ;

/* For S_Obj_Acc_Run type */
#define RUN_UNINIT 0        
#define RUN_RANDOM 1        /* Two of these should be marked in run->type */
#define RUN_STRIDE 2
#define RUN_READ   4
#define RUN_WRITE  8	    /* Beware: used for S_Obj_Acc_Run status too */

/* For S_Obj_Acc_Run status */
/* use RUN_UNINIT 0 for status initialization */
#define READS_INIT_INST 1   /*read run initialized by writes in same instance*/
#define READS_INIT_OBJ  2   /*read run initialized by writes in same object*/
#define READS_PARTIAL  4
/* RUN_WRITE fits in here */
#define READS_NONE 16

/* For S_Obj_Acc_Run direction */
/* use RUN_UNINIT 0 for initialization */
#define FORWARD 1
#define BACKWARD 2

/* For finding read run initialization source */
#define UNINIT  0
#define INSTANCE  1
#define OBJECT 2

/* For new_runs_ok, single */
#define NO  0
#define YES 1

typedef struct _S_Instance_Runs
{
  S_Obj_Context context;
  int num_rruns;
  int num_wruns;
  List runs;
  int new_rruns_ok;
  int new_wruns_ok;
}
S_Instance_Runs;

typedef struct _S_Obj_Acc_Run
{
  int status;
  int type;
  int size;                /* size (in Bytes) of individual accesses to run */
  int coverageB;           /* total Bytes covered by this run
			      (not necessarily loff-foff+1 for RANDOM runs) */
  int foff, loff;
  int stride;
  int direction; /* RUN_UNINIT, FORWARD, or BACKWARD */
  int single;    /* Note that this indicates the access is to
		    just one location within the object; 
		    the same access could have occurred an arbitrary
		    number of times */

  Set iters;
}
S_Obj_Acc_Run;

typedef struct _S_Obj_Des
{
  int type;
  int addr;
  int size;
  int status;
  char *name;

  struct _S_Obj_Des *prev;
  struct _S_Obj_Des *next;

  /* Data collection facilities */

  S_Obj_RW_Summ *rw_summ;

  List instances;

  int writes;			/* Total number writes to this object */
  int reads;			/* Total number reads to this object */

}
S_Obj_Des;

typedef struct
{
  int type;
  int loc;
  int size;
  char * name;
}
S_obj_desc;

typedef struct _S_Obj_Page_Des
{
  int entry;
  S_Obj_Des *fobj;
}
S_Obj_Page_Des;

typedef struct _S_Obj_Pri_Page_Tbl
{
  int entry;
  S_Obj_Page_Des *pdes[PTSIZE];
}
S_Obj_Pri_Page_Tbl;

typedef struct _S_Obj_Sec_Page_Tbl
{
  int entry;
  S_Obj_Pri_Page_Tbl *ppt[PTSIZE];
}
S_Obj_Sec_Page_Tbl;

/* Function declarations */
extern void S_trace_globals (Pnode * pnode);
void S_clear_stack (void);
void S_clear_pages (void);
void S_init_page_table (void);
S_Obj_Des *S_insert_object (int type, int start, int size, char * name);
S_Obj_Des *S_remove_object_by_ref (S_Obj_Des * obj);
void S_init_object_trace (void);
void S_read_obj_trace (Pnode * pnode);
void S_print_object_report (void);
void S_close_object_trace (void);

#endif
