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
/*============================================================================
 *
 *      File :          r_regalloc.h
 *      Description :   Contains all structure definitions required by
 *                      the register allocator
 *      Creation Date : July 10, 1991
 *      Author :        Richard Hank, Wen-mei Hwu
 *
 * Revision 4.0  95/01/10  17:35:19  17:35:19  hank (Richard E. Hank)
 * - Changes consistent with register allocator revisions.
 *   ( see comments for r_regalloc.c version 4.0 )
 *
 * Revision 3.0  94/03/16  20:59:00  20:59:00  hank (Richard E. Hank)
 * - Changes consistent with register allocator revisions.
 *  ( see comments for r_regalloc.c version 3.0 )"
 *
 * Revision 2.0  93/06/14  21:58:17  21:58:17  hank (Richard E. Hank)
 * "- Changes consistent with register allocator revisions.
 *    ( see comments for r_regalloc.c version 2.0 )"
 *
 * Revision 1.1  93/04/19  12:57:01  12:57:01  hank (Richard E. Hank)
 * Initial revision
 *
 *===========================================================================*/

#ifndef R_REGALLOC_H
#define R_REGALLOC_H

/* 10/29/02 REK Adding config.h */
#include <config.h>
#include <Lcode/l_main.h>
#include <library/stack.h>
#include <library/heap.h>
#include <library/set.h>
#include <library/i_list.h>
#include <machine/lmdes.h>
#include <Lcode/l_region.h>

/*==========================================================================*
 * REGISTER ALLOCATOR USER INTERFACE FUNCTIONS,STRUCTURES, AND INSTRUCTIONS *
 *==========================================================================*/

/*
 * Register Bank Definition:
 *
 *     The register allocator allocates registers from a linear
 *     virtual register file, with the registers numbered from
 *     R[0..N].  Mapped onto this virtual register file is a series of
 *     register banks, defined by the user which represent the
 *     physical register configuration of the target processor.
 *
 *     Each register bank is uniquely defined by two paramters:
 *	  1) CLASS	- either caller or callee saved
 *        2) TYPE	- data type ( i.e. int, float, double, or quad )
 *
 *     In addition to the above parameters there are 3 more parameters, which
 *     describe the mapping of the banks into the virtual register file.
 *	  3) NUM_REG    - number of registers in the bank
 *	  4) REG_SIZE	- number of registers in the virtual file required by
 *			  each register in the bank
 *        5) OVERLAP    - describes the overlap conditions for example
 *                        float and double register files typically overlap.
 *
 *      Also, so that the register allocator may return actual machine
 *  	register numbers to the code generator, an array of integers
 *  	called a register * map is also required for each bank.
 *
 *      Finally, the register allocator requires a Set * so that it
 *      may return the set of registers used from each register bank.
 *
 *	The following is an example of one way to define the register
 *	configuration of the MIPS processor to the register allocator:
 *
 *      class, type, base_index, num_reg, reg_size, reg_map
 *
 * R_define_physical_bank( R_CALLER, R_INT, 8, 1, R_OVERLAP_INT, 
 *                          caller_int_map, &clri_set);
 * R_define_physical_bank( R_CALLER, R_FLOAT, 4, 1, 
 *                         R_OVERLAP_FLOAT|R_OVERLAP_DOUBLE, 
 *                          caller_flt_map, &clrf_set);
 * R_define_physical_bank( R_CALLER, R_DOUBLE, 4, 1, 
 *                         R_OVERLAP_FLOAT|R_OVERLAP_DOUBLE, 
 *                          caller_flt_map, &clrf_set );
 * R_define_physical_bank( R_CALLEE, R_INT, 9, 1, R_OVERLAP_INT,
 *                          callee_int_map, &clei_set);
 * R_define_physical_bank( R_CALLEE, R_FLOAT, 6, 1, 
 *                         R_OVERLAP_FLOAT|R_OVERLAP_DOUBLE,
 *                          callee_flt_map, &clef_set);
 * R_define_physical_bank( R_CALLEE, R_DOUBLE, 6, 1, 
 *                         R_OVERLAP_FLOAT|R_OVERLAP_DOUBLE,
 *                          callee_flt_map, &clef_set );
 *
 *      NOTE: It is highly recommended that the register maps provided to *
 *	       the register allocator have the property that registers
 *	       in non-overlapping banks have unique numbers, so that
 *	       the register usage information returned by the register
 *	       allocator * will be correct.  * If this is not done,
 *	       register allocation will be done properly, but it will
 *	       make creating your prologue much more difficult
 *	
 *      Register Maps:
 *	int caller_int_map[] = { 8, 9, 10, 11, 12, 13, 14, 15, 16 };
 *      int callee_int_map[] = { 16, 17, 18, 19, 20, 21, 22, 23, 30 };
 *	int caller_flt_map[] = { 8, 10, 16, 18 };
 *	int callee_flt_map[] = { 20, 22, 24, 26, 28, 30 }; 
 *         ( as per the above note, the flt maps should have each number
 *           incremented by an amount which makes them unique from the 
 *	     integer banks )
 * 
 *	Macroregister Banks:
 *	
 *	Macroregister banks are defined exactly as above, using the
 *	bank class * names: R_MACRO_CALLER and R_MACRO_CALLEE.  There
 *	is one exception, the * register map array for macroregisters
 *	must contain the macro register * name rather that the
 *	processor register name.
 *
 *      Example:
 *      int caller_macro_map[] = { L_MAC_P0, L_MAC_P1, L_MAC_P2, L_MAC_P3 }; */

/*============================
 *  REGISTER BANK DEFINITION  
 *============================*/
#define R_NUM_CLASS	5
#define R_TYPE_INC	R_NUM_CLASS
#define R_NUM_TYPES	7	/* number of bank data types */
#define R_MAX_BANK 	(R_NUM_TYPES*R_NUM_CLASS)

typedef struct R_Physical_Bank
{
  short defined;
  short rclass;			/* rclass may be: R_CALLER, R_CALLEE      */
  short type;			/* type: R_INT, R_FLOAT, etc.             */
  short num_reg;		/* max number of registers in bank        */
  short num_rot_reg;		/* number of rotating registers in bank   */
  short rot_reg_ofs;		/* offs. into reg map of the 
				   start of rot regs                      */
  short num_rot_reg_alloc;	/* multiple of rot regs that must 
				   be allocated                           */
  short reg_size;		/* size of registers in bank              */
  short overlap;
  short global_overlap;
  int base_index;		/* offset into register map               */
  Stack *avail_reg;
  Stack *alloc_reg;
  int alloc_init;
  int mask;			/* member register mask */
  int overlap_cnt[R_NUM_TYPES];

  Set *used_reg;

  int max;			/* internally defined fields */
  short res_inc;
}
R_Physical_Bank;

#ifdef __cplusplus
extern "C"
{
#endif

  extern int R_global_overlap[];
  extern int R_max_overlap_size[];

#ifdef __cplusplus
}
#endif

/* 
 *  Register Bank Class 
 */
#define R_CALLER  	   0	/* caller saved register bank            */
#define R_CALLEE           1	/* callee saved register bank            */
#define R_SPILL		   2	/* spill register bank                   */
#define R_MACRO_CALLER	   3	/* caller saved macro register bank      */
#define R_MACRO_CALLEE     4	/* callee saved macro register bank      */

#define R_PREFER_SPILL	-1

/*
 * Register Bank Type 
 */
#define R_PREDICATE     0
#define R_INT		5
#define R_FLOAT  	10
#define R_DOUBLE	15
#define R_BTR		20
#define R_QUAD	        25
#define R_POINTER       30

/*
 * Register Bank Overlap
 */
#define R_OVERLAP_PREDICATE	0x01
#define R_OVERLAP_INT		0x02
#define R_OVERLAP_FLOAT		0x04
#define	R_OVERLAP_DOUBLE	0x08
#define R_OVERLAP_BTR		0x10
#define R_OVERLAP_QUAD		0x20
#define R_OVERLAP_POINTER       0x40

/*============================
 *  REGISTER ALLOCATION  
 *============================*/
typedef struct R_Alloc_Info
{
  int n_caller_used;		/* number caller registers used        */
  int n_callee_used;		/* number callee registers used        */
  Set callee_used;		/* callee registers used               */
  Set caller_used;		/* caller registers used               */
  int swap_size;		/* total swap space size =             */
  /*       spilled live range + swap reg */
  int swap_offset;		/* loc in swap space for callee swap   */
}
R_Alloc_Info;


#define R_SPILL_CODE		0
#define R_JSR_SAVE_CODE		1
#define R_CALLEE_SAVE_CODE	2

/* Virtual Register Flags */
#define R_UNCONSTRAINED		0x1
#define R_CONSTANT		0x2
#define	R_SPILLED		0x4
#define R_CB_SPLIT		0x8
#define R_INSTR_SPLIT		0x10
#define R_PREALLOCATED_MACRO	0x20
#define R_PA_HACK		0x40
#define R_MCB_PRELOAD		0x80

#define R_CONTAINS_JSR		0x100

#define R_LIVE_OUTSIDE_REGION	0x1000
#define R_PREALLOCATED_FLYBY	0x2000
#define R_REGION_CONSTRAINED	0x4000
#define R_CURRENT_VREG		0x8000

typedef struct R_Reg {
  int index;			/* virtual register number        */
  unsigned short flags;		/* ????  */
  char rclass;			/* virtual register class         */
  unsigned char type;		/* virtual register type          */
  unsigned char size;		/* number of bank registers req'd */

  char caller_benefit;		/* positive if beneficial to use
				 * caller-saved register          */
  char callee_benefit;		/* positive if beneficial to use
				 * callee-saved register          */

  short base_index;		/* offset into register bank      */
  short phys_reg;
  int spill_loc;		/* spill location of live range   */

  Set def_instr;
  Set ref_instr;		/* vreg referencing instructions  */
  Set live_range;		/* instructions w/in live range   */
  Set live_range_uncond;	/* instructions w/in live range on
				 * TRUE predicate (for pred regs.)*/
  Set ref_cbs;

  int rotating;
  int nth_rot_reg;

  Set illegal_reg;
  Set constraints;

  struct R_Reg *pvreg;

  int instr_weight;		/* += 1 each instr in live range  */
  double ref_weight;		/* += instr wgt, for each instr
				 * referencing virtual register   */
  double def_weight;		/* += instr wgt, for each instr
				 * defining virtual registers     */

  /* interference graph stuff */

  double priority;		/* vreg coloring priority         */
  struct R_Arc *interfere;	/* interfering virtual registers  */

  struct R_Reg *nextReg;

  Set intf_vreg;

} R_Reg;

typedef struct R_Arc {
  struct R_Reg *lr;
  struct R_Arc *next;
} R_Arc;

typedef struct R_Macro {
  int id;
  int type;
  int vreg_id;
} R_Macro;


typedef struct _reconcile_block {
  L_Cb *location;
  L_Cb *fromCb;
  L_Cb *toCb;
  L_Flow *flow;
  L_Oper *branch;
  L_Oper *stores;
  Set store_loc;
  L_Oper *loads;
  Set load_loc;
  struct _reconcile_block *nextRCB;
} RCB;

/*===========================================================*/
/* Defines for accessing operation,control block and flow    */
/* data structures, if they are changed from their current   */
/* array structure, alteration of these defines will still   */
/* permit the register allocator to function properly	     */
/*===========================================================*/

#define VREG(a)			((R_Reg *) \
				 HashTable_find_or_null(R_vregHashTbl,(a)))

/*==========================================================*/
#define IMPACT_ABS(a)		((a>0)?a:-a)

#define CALLOC(a,b)		(((b)==0)?NULL:(a *)calloc((b),sizeof(a)))
#define MALLOC(a,b)		(((b)==0)?NULL:(a *)malloc((b)*sizeof(a)))
#define FREE(a)			{ if ( a != NULL ) \
    					free(a); \
    				  a = NULL; \
    				}

#define R_OPERAND_REG(a)       ((L_is_reg(&a)) ? a.value.r : -1 )
#define R_OPERAND_MACRO(a)     ((L_is_macro(&a)) ? a.value.mac : -1 )
#define R_OPERAND_TYPE(a)      ((int)L_return_old_ctype(&a))

#define R_MAX(a,b)		(((a)>(b))?(a):(b))
#define R_MIN(a,b)		(((a)>(b))?(b):(a))

/*===================================
 *REGISTER ALLOCATOR GLOBAL VARIABLES
 *===================================*/

#ifdef __cplusplus
extern "C"
{
#endif

/* Register Allocation Configuration Parameters */
  extern int R_Utilize_Profile_Info;

  extern L_Alloc_Pool *R_alloc_virtual_register;
  extern L_Alloc_Pool *R_alloc_interference_arc;
  extern L_Alloc_Pool *R_RegionAlloc_pool;

  extern int *R_register_contents;

  extern double ld_wgt, st_wgt, mv_wgt;
  extern int ld_cnt, st_cnt, mv_cnt;

  extern R_Physical_Bank *R_bank;	/* register bank array pointer  */
  extern int **R_map;		/* register map array pointer   */

  extern R_Reg *R_vreg;		/* virtual register list        */

  extern HashTable R_vregHashTbl;
  extern HashTable R_regionAllocMap;
  extern HashTable flowHashTbl;

  extern int *R_buf;
  extern int *R_buf2;

  extern void **R_pbuf;
  extern void **R_pbuf2;

  extern int R_n_oper;

  extern Set R_oper_set;
  extern List R_jsr_list;       /* List of jsr opers in the order they appear
				 * in a function.
				 * This was originally a set (R_jsr_set).
				 * However, nothing used it as a set.  All
				 * uses converted it to an array and iterated
				 * through the elements.  Changing this to
				 * a list lets us preserve the order of jsrs.
				 */
  extern L_Oper *R_rts;

  extern int pred_spill_stack;	/* current location in pred spill frame */
  extern int int_spill_stack;	/* current location in int spill frame */
  extern int fp_spill_stack;	/* current location in fp spill frame */

  extern R_Macro *R_avail_macro;
  extern R_Macro *R_jsr_def_macro;
  extern int R_n_avail_macro;
  extern int R_n_jsr_def_macro;

  extern int R_Sort_List;

  extern double total_instruction_weight;
  extern double total_spill_weight;
  extern double total_region_spill_weight;
  extern double total_caller_weight;

  extern L_Region *R_Region;

/*==============================================================
 * Register Allocation Configuration Parameters: Default Values 
 *==============================================================*/
  extern int R_Init;
  extern int R_Register_Allocation;
  extern int R_Macro_Allocation;
  extern int R_Prevent_MCB_Preload_Spills;
  extern int R_Same_Cycle_Anti_Deps_Interference;

  extern int R_Region_Based_Allocation;
  extern int R_Round_Robin_Allocation;

  extern int R_Prextern_Parm_Configuration;
  extern int R_Prextern_Bank_Configuration;

  extern int R_Invariant_Vreg_Priorities;

  extern int R_Print_Macroregisters;
  extern int R_Print_Live_Ranges;
  extern int R_Print_Interference_Graph;
  extern int R_Print_Class_Selection;
  extern int R_Print_Coloring_Stats;
  extern int R_Print_Allocation;
  extern int R_Print_Allocation_Stats;

  extern int R_Print_Dead_Code_Results;

  extern int R_Print_Virtual_Register_Function;
  extern int R_Allow_Dead_Code_Elimination;

  /* -Fminimize_spill_fill=(yes|no)
   * If 0, R_insert_jsr_spill_fill_code() spills and fills every live register
   * around each jsr.  If 1, we try to do things more intelligently.  If
   * a register is live across two jsrs, but is not used between the jsrs
   * (and neither jsr is predicated and there is no branch between jsrs), we
   * eliminiate the useless fill and spill of the register between the jsrs.
   *
   * spill r1;           spill r1;
   * spill r2;           spill r2;
   * jsr foo1            jsr foo1;
   * fill r1;            fill r1;
   * fill r2;      =>    r3 = r1++;
   * r3 = r1++;          spill r1
   * spill r1;           jsr foo2;
   * spill r2;           fill r1;
   * jsr foo2;           fill r2;
   * fill r1;
   * fill r2;
   *
   * This is a big win in straight line code with lots of jsrs, such as you
   * see when compiling C++. */
  extern int R_Minimize_Spill_Fill;

/*========================================= 
 * REGISTER ALLOCATOR FUNCTION DECLARATIONS 
 *=========================================*/
/* These functions are called by the register allocator and are
   written by the user to return the correct cost given the paramters
   passed */

  extern double R_callee_cost (int lcode_ctype, int leaf,
			       int callee_allocated);
  extern double R_caller_cost (int lcode_ctype, int leaf);
  extern double R_spill_load_cost (int lcode_ctype);
  extern double R_spill_store_cost (int lcode_ctype);

  extern L_Oper *O_spill_reg (int reg, int type, L_Operand * operand,
			      int spill_offset, L_Operand ** pred,
			      int type_flag);
  extern L_Oper *O_fill_reg (int reg, int type, L_Operand * operand,
			     int fill_offset, L_Operand ** pred,
			     int type_flag);
  extern L_Oper *O_move_reg (int dest_reg, int src_reg, int reg_type);
  extern L_Oper *O_jump_oper (int opc, L_Cb * dst);
  extern R_Physical_Bank *O_locate_rot_reg_bank (L_Func * fn, R_Reg * vreg);

  extern int R_Ltype_to_Rtype (int type);
  extern char *BANK_NAME (int type);
  extern char *CLASS_NAME (int rclass);
  extern int R_conv_type_to_Ltype (int type);

  extern void R_add_interference_arc (R_Reg *from, R_Reg *to);

#ifdef __cplusplus
}
#endif


/*
 * Function Prototypes
 */
#include "r_regproto.h"

#endif
