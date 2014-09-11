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
/*===========================================================================
 *
 *      File :          r_regalloc.c
 *      Description :   Register allocation interface functions 
 *      Creation Date : July 12, 1991
 *      Author :        Richard Hank
 *
 * Revision 4.0  95/01/10  17:33:26  17:33:26  hank (Richard E. Hank)
 * Region-based Allocation:  This version incorporates the preliminary work
 *   on region based compilation into the register allocation process.
 *   
 *   1)  Region-based allocation is done if the parameter 
 *       "region_based_allocation" is set.
 *   2)  Function-based allocation is done if the function contains
 *       no region markers or the above flag is not set.
 * 
 * Implementation changes:
 *   1) The virtual register array has finally been eliminated and replaced
 *      by a somewhat more dynamic linked list.
 *   2) The register allocator no longer returns a structure since the
 *      registers used in each bank are returned through pointers passed to
 *      it via R_define_physical_bank
 *   3) All spill code related functions have been moved to r_regspill.c
 * 
 * Revision 3.0  94/03/16  20:58:17  20:58:17  hank (Richard E. Hank)
 * Several Changes:
 * 1) A destination virtual register of an instruction may now be allocated  
 *    to the same register as a source virtual register  - exactly how this
 *    will interact with pruning is not yet determined.
 * 2) The register bank description functions have been moved to r_regbank.c.
 *    Changes to the register bank description are described there.
 * 3) Spilled live ranges are now split at the cb and instruction level
 *    in extreme register pressure situations.  (i.e. x86)
 *    The code has been moved to a separate file.
 * 4) Several PA-RISC specific and MCB hacks are present.
 * 
 * Revision 2.0  93/06/14  21:55:17  21:55:17  hank (Richard E. Hank)
 * "This version contains the following major revisions:
 *   1) Eliminated the need for explicit spill registers.  Spill register
 *      bank definitions are allowed, but they are ignored.
 *   2) The interference graph is pruned to remove unconstrained virtual
 *      registers prior to coloring
 *   3) The actual allocation algorithm repeats the coloring process until
 *      all virtual registers have been assigned a registers, this was
 *      necessitated by the elimination of explicit spill registers.
 *      -> Results in a much improved allocation
 *   4) Spill code insertion has been further optimized by eliminating
 *      unecessary spill stores whenever possible."
 * 
 * Revision 1.1  93/04/19  12:56:56  12:56:56  hank (Richard E. Hank)
 * Initial revision
 * 
 *      Copyright (c) 1991,1992,1993,1994  
 *                         Richard Hank, Wen-mei Hwu and The Board of
 *			   Trustees of the University of Illinois. 
 *			   All rights reserved.
 *      The University of Illinois software License Agreement
 *      specifies the terms and conditions for redistribution.
 *===========================================================================*/

/* 10/29/02 REK Adding config.h */
#include <config.h>
#include "r_regalloc.h"
#include "region.h"
#include <machine/m_hppa.h>
#include <Lcode/l_opti.h>
#include <Lcode/l_disjvreg.h>
#include <library/i_list.h>

/*==================================
 *REGISTER ALLOCATOR GLOBAL POINTERS
 *==================================*/

/*==============================================================
 * Register Allocation Configuration Parameters: Default Values 
 *==============================================================*/
int R_Init = 1;
int R_Register_Allocation = 1;
int R_Spill_Everything = 0;
int R_Macro_Allocation = 1;
int R_Utilize_Profile_Info = 1;
int R_Prevent_MCB_Preload_Spills = 0;
int R_Same_Cycle_Anti_Deps_Interference = 1;

int R_Prune_Interference_Graph = 0;
int R_Region_Based_Allocation = 0;
int R_Round_Robin_Allocation = 0;

int R_Invariant_Vreg_Priorities = 0;
int R_Schedule_Invariant_Alloc = 0;

int R_Print_Parm_Configuration = 0;
int R_Print_Bank_Configuration = 0;

int R_Print_Macroregisters = 0;
int R_Print_Live_Ranges = 0;
int R_Print_Interference_Graph = 0;
int R_Print_Class_Selection = 0;
int R_Print_Coloring_Stats = 0;
int R_Print_Allocation = 0;
int R_Print_Allocation_Stats = 0;

int R_Print_Dead_Code_Results = 0;

int R_Print_Virtual_Register_Function = 0;
int R_Allow_Dead_Code_Elimination = 1;

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
int R_Minimize_Spill_Fill = 0;

/*=============================================
 * Register Allocation Memory Allocation Pools
 *=============================================*/
L_Alloc_Pool *R_alloc_virtual_register = NULL;
L_Alloc_Pool *R_alloc_interference_arc = NULL;

/* This HP-PA hack must be generalized */
Set R_pa_set = 0;

double ld_wgt, st_wgt, mv_wgt;
int ld_cnt, st_cnt, mv_cnt;

R_Physical_Bank *R_bank = 0;	/* register bank array pointer  */
int **R_map;			/* register map array pointer   */
int R_global_overlap[R_MAX_BANK];
int R_max_overlap_size[R_MAX_BANK];

int R_n_vreg = 0;
R_Reg *R_vreg = NULL;		/* virtual register list        */

HashTable R_vregHashTbl;	/* virtual register hash table  */
HashTable R_regionAllocMap;	/* region allocation mapping table */

HashTable flowHashTbl;

int *R_buf = NULL;		/* global buffer */
int *R_buf2 = NULL;

void **R_pbuf = NULL;		/* global buffer */
void **R_pbuf2 = NULL;

int R_n_oper;

Set R_oper_set = NULL;
List R_jsr_list = NULL;         /* List of jsr opers. Opers are kept in order
				 * of execution. */
Set R_extend_set = NULL;	/* Misc ops with extended dependences */
L_Oper *R_rts = NULL;		/* function rts                 */

int pred_spill_stack;
int int_spill_stack;
int fp_spill_stack;

int *R_register_contents;

R_Macro *R_avail_macro;
R_Macro *R_jsr_def_macro;

int R_n_avail_macro;
int R_n_jsr_def_macro;

double caller_callee_threshold = 0.0;

double total_instruction_weight = 0.0;
double total_spill_weight = 0.0;
double total_region_spill_weight = 0.0;
double total_caller_weight = 0.0;

L_Region *R_Region;

/*========================================
 *REGISTER ALLOC. INTERNAL FUNCTIONS
 *========================================*/

/*===========================================================================
 *
 *      Func :	R_read_parm_regalloc()    
 *      Desc :  Reads option file and loads register allocation parameters 
 *		Sets the following globals:
 *		    R_Register_Allocation = 1/0  (1=default)
 *		    R_Macro_Allocation = 1/0	 (1=default)
 *		    R_Constant_Preloading = 1/0  (0=default)
 *		    R_Loop_Removal = 1/0	 (0=default)
 *		    R_Live_Range_Splitting = 1/0 (0=default)
 *	Input:  Parm_Parse_Info *ppi; 
 *	Output: none
 *
 *	Side Effects: as described above 
 *
 *===========================================================================*/
static void
R_read_parm_regalloc (Parm_Parse_Info * ppi)
{
  L_read_parm_b (ppi, "spill_everything", &R_Spill_Everything);
  L_read_parm_b (ppi, "region_based_allocation", &R_Region_Based_Allocation);
  L_read_parm_b (ppi, "prune_interference_graph",
		 &R_Prune_Interference_Graph);
  L_read_parm_b (ppi, "?round_robin_allocation", &R_Round_Robin_Allocation);
  L_read_parm_b (ppi, "schedule_invariant_allocation",
		 &R_Schedule_Invariant_Alloc);
  L_read_parm_b (ppi, "macro_allocation", &R_Macro_Allocation);
  L_read_parm_b (ppi, "utilize_profile_info", &R_Utilize_Profile_Info);
  L_read_parm_b (ppi, "prevent_MCB_preload_spills",
		 &R_Prevent_MCB_Preload_Spills);
  L_read_parm_b (ppi, "same_cycle_anti_dep_interference",
		 &R_Same_Cycle_Anti_Deps_Interference);
  L_read_parm_b (ppi, "print_macroregisters", &R_Print_Macroregisters);
  L_read_parm_b (ppi, "print_live_ranges", &R_Print_Live_Ranges);
  L_read_parm_b (ppi, "print_interference_graph",
		 &R_Print_Interference_Graph);
  L_read_parm_b (ppi, "print_class_selection", &R_Print_Class_Selection);
  L_read_parm_b (ppi, "print_allocation", &R_Print_Allocation);
  L_read_parm_b (ppi, "print_coloring_stats", &R_Print_Coloring_Stats);
  L_read_parm_b (ppi, "print_allocation_stats", &R_Print_Allocation_Stats);
  L_read_parm_b (ppi, "print_parm_configuration",
		 &R_Print_Parm_Configuration);
  L_read_parm_b (ppi, "print_bank_configuration",
		 &R_Print_Bank_Configuration);
  L_read_parm_b (ppi, "print_dead_code_results", &R_Print_Dead_Code_Results);

  L_read_parm_b (ppi, "print_virtual_register_function",
		 &R_Print_Virtual_Register_Function);
  L_read_parm_b (ppi, "allow_dead_code_elimination",
		 &R_Allow_Dead_Code_Elimination);
  L_read_parm_b (ppi, "minimize_spill_fill", &R_Minimize_Spill_Fill);

  if (R_Spill_Everything)
    R_Register_Allocation = 0;

  if (R_Spill_Everything || !R_Register_Allocation)
    R_Macro_Allocation = 0;

  if (R_Schedule_Invariant_Alloc)
    R_Invariant_Vreg_Priorities = 1;

  if (M_arch == M_X86 && !R_Macro_Allocation)
    {
      L_warn ("RA (x86): Macro Allocation defaulted to 'yes'\n");
      R_Macro_Allocation = 1;
    }

  return;
}

#define ON_OFF(parm)	((parm)?"on":"off")

void
R_print_parm_configuration (void)
{
  fprintf (stdout, "*\n*\n* Register Allocator Configuration\n*\n*\n");

  fprintf (stdout, "spill_everything:	       %s\n",
	   ON_OFF (R_Spill_Everything));
  fprintf (stdout, "macro_allocation:          %s\n",
	   ON_OFF (R_Macro_Allocation));
  fprintf (stdout, "utilize_profile_info:      %s\n",
	   ON_OFF (R_Utilize_Profile_Info));
  fprintf (stdout, "region_based_allocation:   %s\n",
	   ON_OFF (R_Region_Based_Allocation));
  fprintf (stdout, "prune_interference_graph:  %s\n",
	   ON_OFF (R_Prune_Interference_Graph));
  fprintf (stdout, "\n");
  fprintf (stdout, "print_parm_configuration:  %s\n",
	   ON_OFF (R_Print_Parm_Configuration));
  fprintf (stdout, "print_bank_configuration:  %s\n",
	   ON_OFF (R_Print_Bank_Configuration));
  fprintf (stdout, "print_live_ranges:         %s\n",
	   ON_OFF (R_Print_Live_Ranges));
  fprintf (stdout, "print_class_selection:     %s\n",
	   ON_OFF (R_Print_Class_Selection));
  fprintf (stdout, "print_coloring_stats:      %s\n",
	   ON_OFF (R_Print_Coloring_Stats));
  fprintf (stdout, "print_allocation:          %s\n",
	   ON_OFF (R_Print_Allocation));
  fprintf (stdout, "print_allocation_stats:    %s\n",
	   ON_OFF (R_Print_Allocation));

  fprintf (stdout, "\n\n");

  return;
}


static R_Reg *
R_new_reg (int vreg_id, int type, double weight, int def, int rclass)
{
  R_Reg *new_vreg;

  new_vreg = (R_Reg *) L_alloc (R_alloc_virtual_register);
  new_vreg->flags = R_CURRENT_VREG;
  new_vreg->nextReg = R_vreg;
  R_vreg = new_vreg;

  HashTable_insert (R_vregHashTbl, vreg_id, new_vreg);
  R_n_vreg += 1;

  new_vreg->ref_weight = weight;
  new_vreg->def_weight = def ? weight : 0;
  
  new_vreg->rclass = rclass;
  new_vreg->type = type;
  new_vreg->size = 0;
  new_vreg->index = vreg_id;
  new_vreg->base_index = -1;
  new_vreg->spill_loc = -1;
  new_vreg->ref_instr = NULL;
  new_vreg->live_range = NULL;
  new_vreg->def_instr = NULL;
  new_vreg->ref_cbs = NULL;
  new_vreg->pvreg = NULL;
  new_vreg->instr_weight = 0;
  new_vreg->priority = 0.0;
  new_vreg->interfere = NULL;
  new_vreg->illegal_reg = NULL;
  new_vreg->constraints = NULL;
  new_vreg->rotating = 0;
  new_vreg->nth_rot_reg = -1;
  new_vreg->caller_benefit = 0;
  new_vreg->callee_benefit = 0;
  
  new_vreg->intf_vreg = NULL;

  return new_vreg;
}

/*===========================================================================
 *
 *      Func : 	R_add_register()  
 *      Desc :  Defines a new virtual register 
 *      Input:  int vreg_id 	- virtual register number
 *		int type	- virtual register type
 *		double weight	- instruction execution frequency
 *      Output: none
 *
 *      Side Effects:  allocation of new R_Reg structure 
 *
 *===========================================================================*/
R_Reg *
R_add_register (int vreg_id, int type, double weight, int def)
{
  R_Reg *new_vreg;

  if ((new_vreg = VREG (vreg_id)))
    {
      /* Found existing vreg -- check for type match and update weights */

      if (new_vreg->type != R_Ltype_to_Rtype (type))
	{
	  /* Special case for Playdoh here - SAM 11-97 */
	  if ((M_model == M_HP_PLAYDOH_MCODE)
	      && ((new_vreg->type == R_FLOAT)
		  || (new_vreg->type == R_DOUBLE)))
	    new_vreg->type = R_DOUBLE;
	  else
	    L_punt ("R_add_register: Virtual register type conflict\n"
		    "\t vreg %d occurs first with type %s\n"
		    "\t and subsequently with type %s.\n",
		    new_vreg->index, BANK_NAME (new_vreg->type),
		    BANK_NAME (R_Ltype_to_Rtype (type)));
	}

      new_vreg->ref_weight += weight;
      if (def)
	new_vreg->def_weight += weight;    
    }
  else
    {  
      /* Create a new vreg */      

      switch (type)
	{
	case L_CTYPE_PREDICATE:
	  type = R_PREDICATE;
	  break;
	case L_CTYPE_INT:
	case L_CTYPE_LLONG:
	  type = R_INT;
	  break;
	case L_CTYPE_FLOAT:
	  type = R_FLOAT;
	  break;
	case L_CTYPE_DOUBLE:
	  type = R_DOUBLE;
	  break;
	case L_CTYPE_BTR:
	  type = R_BTR;
	  break;
	case L_CTYPE_POINTER:
	  type = R_POINTER;
	  break;
	default:
	  L_punt ("RA: invalid virtual register type: %d", type);
	  break;
	}

      new_vreg = R_new_reg (vreg_id, type, weight, def, R_PREFER_SPILL);
    }

  return (new_vreg);
}

/*===========================================================================
 *
 *      Func :	R_find_free_vreg()   
 *      Desc :  Returns a new virtual register number 
 *      Input:  none 
 *      Output: int
 *
 *      Side Effects: increments L_fn->max_reg_id
 *
 *===========================================================================*/
int
R_find_free_vreg (void)
{
  int new_vreg = ++(L_fn->max_reg_id);

  if (new_vreg == 0)
    new_vreg = ++(L_fn->max_reg_id);

  return (new_vreg);
}

/*===========================================================================
 *
 *      Func :	R_spill_loc()   
 *      Desc :  Returns a new swap space offset 
 *      Input:  none 
 *      Output: int
 *
 *      Side Effects: none
 *
 *===========================================================================*/
int
R_spill_loc (int type)
{
  int loc = -1;

  if (M_arch != M_TAHOE)
    {
      switch (type)
	{
	case R_PREDICATE:
	case R_INT:
	case R_FLOAT:
	case R_BTR:
	case R_POINTER:
	  loc = int_spill_stack;
	  int_spill_stack += 4;
	  fp_spill_stack = int_spill_stack;
	  break;
	case R_DOUBLE:
	  /* ensure double boundary in register swap space */
	  if ((int_spill_stack % 8) != 0)
	    int_spill_stack += (8 - (int_spill_stack % 8));
	  loc = int_spill_stack;
	  int_spill_stack += 8;
	  fp_spill_stack = int_spill_stack;
	  break;
	default:
	  L_punt ("RA (R_spill_loc): invalid virtual register type: %d", type);
	}
    }
  else
    {
      switch (type)
	{
	case R_INT:
	case R_BTR:
	  loc = int_spill_stack;
	  int_spill_stack += 8;
	  break;
	case R_FLOAT:
	case R_DOUBLE:
	  /* ensure 16 byte boundary in register swap space */
	  if ((fp_spill_stack % 16) != 0)
	    fp_spill_stack += (16 - (fp_spill_stack % 16));
	  loc = fp_spill_stack;
	  fp_spill_stack += 16;
	  break;
	case R_PREDICATE:
	  /* L_punt ("Unable to spill predicate register.\n"); */
	  /* EMN */
	  loc = pred_spill_stack;
	  pred_spill_stack += 1;
	  break; 
	default:
	  L_punt ("RA (R_spill_loc): invalid virtual register type: %d", type);
	}
    }
  return (loc);
}

/*===========================================================================
 *
 *      Func : 	R_add_macro_register  
 *      Desc :  Defines the virtual register for a macro register available
 *		to the register allocator. The virtual register is treated
 *		as if it has already been allocated to this macro.
 *      Input:  int vreg_id	- macro virtual register id
 *		int mac		- macro register
 *		int type	- macro register type
 *		int rclass	- macro register bank (CALLER/CALLEE)	 
 *      Output: none
 *
 *      Side Effects: allocation of new R_Reg structure
 *
 *===========================================================================*/
R_Reg *
R_add_macro_register (int vreg_id, int mac, int type, int rclass)
{
  R_Physical_Bank *bank;
  R_Reg *new_vreg;
  int i, *map, found = 0;
  int size = 0, base_index = 0;

  new_vreg = VREG (vreg_id);
  if (new_vreg)
    return (new_vreg);

  /* locate the macro register in the appropriate bank that this */
  /* virtual register has already been "allocated" to            */

  if ((rclass != R_MACRO_CALLER) &&
      (rclass != R_MACRO_CALLEE))
    L_punt ("RA: Unidentified rclass");

  bank = R_bank + type + rclass;
  if (bank->defined)
    {
      map = R_map[type + rclass];
      for (i = 0; i < bank->num_reg; i++)
	{
	  if (mac == map[i * bank->reg_size])
	    {
	      size = bank->reg_size;
	      base_index =
		bank->base_index + i * bank->reg_size;
	      found = 1;
	      
	      R_pa_set = Set_add (R_pa_set, base_index);

	      break;
	    }
	}
    }

  if (!found)
    L_punt ("RA: invalid macro %d (%s)", mac, L_macro_name (mac));

  new_vreg = R_new_reg (vreg_id, type, 0.0, 0, rclass);

  new_vreg->flags |= R_PREALLOCATED_MACRO;

  new_vreg->size = size;
  new_vreg->base_index = base_index;

  new_vreg->caller_benefit = -1;
  new_vreg->callee_benefit = -1;

  return (new_vreg);
}

/*===========================================================================
 *
 *      Func :	R_determine_available_macros()   
 *      Desc :  Scans for R_MACRO_CALLER and R_MACRO_CALLEE register
 *		banks to determine what macro registers are available
 *		if any. 
 *      Input:  none 
 *      Output: none
 *
 *      Side Effects:  	allocates and defines globals: <R_avail_macro_array>,
 *			<R_avail_macro_vreg_id>,<R_jsr_def_macros>,
 *			<R_jsr_def_vreg_id>,<R_n_avail_macro>, and
 *			<R_n_jsr_def_macro>
 *
 *===========================================================================*/

void
R_determine_available_macros (void)
{
  int i, j, type, vreg_id;
  int *map, n_macro, mac_reg;
  R_Physical_Bank *bank;

  R_n_avail_macro = 0;
  R_n_jsr_def_macro = 0;

  if (!R_Macro_Allocation)
    return;

  n_macro = 0;
  /* determine a ceiling on the number of available macro registers */
  /* for memory allocation purposes                                 */
  for (i = 0; i < R_NUM_TYPES; i++)
    {
      type = i * R_TYPE_INC;
      bank = R_bank + type + R_MACRO_CALLER;
      if (bank->defined)
	n_macro += bank->num_reg * bank->reg_size;
      bank = R_bank + type + R_MACRO_CALLEE;
      if (bank->defined)
	n_macro += bank->num_reg * bank->reg_size;
    }
  if (n_macro == 0)
    return;

  R_avail_macro = (R_Macro *) malloc (sizeof (R_Macro) * n_macro);
  R_jsr_def_macro = (R_Macro *) malloc (sizeof (R_Macro) * n_macro);

  for (i = 0; i < R_NUM_TYPES; i++)
    {
      type = i * R_TYPE_INC;
      bank = R_bank + type + R_MACRO_CALLER;
      if (bank->defined)
	{
	  map = R_map[type + R_MACRO_CALLER];
	  for (j = 0; j < bank->num_reg; j++)
	    {
	      mac_reg = map[j * bank->reg_size];

	      vreg_id = R_find_free_vreg ();
	      R_avail_macro[R_n_avail_macro].id = mac_reg;
	      R_avail_macro[R_n_avail_macro].type = type;
	      R_avail_macro[R_n_avail_macro++].vreg_id = vreg_id;

	      /* define macros defined by a jsr */
	      R_jsr_def_macro[R_n_jsr_def_macro].id = mac_reg;
	      R_jsr_def_macro[R_n_jsr_def_macro].type = type;
	      R_jsr_def_macro[R_n_jsr_def_macro++].vreg_id = vreg_id;

	      R_add_macro_register (vreg_id, mac_reg, type, R_MACRO_CALLER);
	    }
	}
      bank = R_bank + type + R_MACRO_CALLEE;
      if (bank->defined)
	{
	  map = R_map[type + R_MACRO_CALLEE];
	  for (j = 0; j < bank->num_reg; j++)
	    {
	      mac_reg = map[j * bank->reg_size];

	      vreg_id = R_find_free_vreg ();
	      R_avail_macro[R_n_avail_macro].id = mac_reg;
	      R_avail_macro[R_n_avail_macro].type = type;
	      R_avail_macro[R_n_avail_macro++].vreg_id = vreg_id;

	      R_add_macro_register (vreg_id, mac_reg, type, R_MACRO_CALLEE);
	    }
	}
    }
  if (R_Print_Macroregisters)
    {
      fprintf (stdout, "Available Macroregisters:\n");
      for (i = 0; i < R_n_avail_macro; i++)
	fprintf (stdout, "\t(mac %s %s) assigned virtual register %d\n",
		 L_macro_name (R_avail_macro[i].id),
		 L_ctype_name (R_conv_type_to_Ltype (R_avail_macro[i].type)),
		 R_avail_macro[i].vreg_id);
      fprintf (stdout, "JSR Defined Macroregisters:\n");
      for (i = 0; i < R_n_jsr_def_macro; i++)
	fprintf (stdout, "\t(mac %s %s) assigned virtual register %d\n",
		 L_macro_name (R_avail_macro[i].id),
		 L_ctype_name (R_conv_type_to_Ltype (R_avail_macro[i].type)),
		 R_avail_macro[i].vreg_id);
    }
  return;
}

Set
R_process_illegal_register_attr (L_Attr * attr)
{
  int i, j, found;
  L_Operand *preg;
  int type, *map, size;
  R_Physical_Bank *bank;
  Set constraints = NULL;
  R_Reg *vreg_ptr = VREG (attr->field[0]->value.r);

  if (vreg_ptr == NULL)
    {
      fprintf (stderr, "Illegal operand in ill_reg attribute:");
      L_print_operand (stdout, attr->field[0], 0);
      L_punt ("R_process_illegal_register_attr: Illegal operand.");
    }

  for (i = 1; i < attr->max_field; i++)
    {
      preg = attr->field[i];
      type = R_Ltype_to_Rtype (L_return_old_ctype (preg));

      found = 0;
      if (L_is_register (preg))
	{
	  bank = R_bank + type + R_CALLER;
	  if (bank->defined)
	    {
	      map = R_map[type + R_CALLER];
	      size = bank->reg_size;
	      for (j = 0; j < bank->num_reg; j++)
		{
		  if (map[j * size] == preg->value.r)
		    {
		      constraints = Set_add (constraints,
					     bank->base_index +
					     j * bank->reg_size);
		      found = 1;
		      break;
		    }
		}
	    }
	  if (found)
	    continue;

	  bank = R_bank + type + R_CALLEE;
	  if (bank->defined)
	    {
	      map = R_map[type + R_CALLEE];
	      size = bank->reg_size;
	      for (j = 0; j < bank->num_reg; j++)
		{
		  if (map[j * size] == preg->value.r)
		    {
		      constraints = Set_add (constraints,
					     bank->base_index +
					     j * bank->reg_size);
		      found = 1;
		      break;
		    }
		}
	    }
	  if (!found)
	    L_punt ("Regalloc: unknown register in ill_reg attribute: %d!!\n",
		    preg->value.r);
	}
      else if (L_is_macro (preg))
	{
	  bank = R_bank + type + R_MACRO_CALLER;
	  if (bank->defined)
	    {
	      map = R_map[type + R_MACRO_CALLER];
	      size = bank->reg_size;
	      for (j = 0; j < bank->num_reg; j++)
		{
		  if (map[j * size] == preg->value.mac)
		    {
		      constraints = Set_add (constraints,
					     bank->base_index +
					     j * bank->reg_size);
		      found = 1;
		      break;
		    }
		}
	    }
	  if (found)
	    continue;

	  bank = R_bank + type + R_MACRO_CALLEE;
	  if (bank->defined)
	    {
	      map = R_map[type + R_MACRO_CALLEE];
	      size = bank->reg_size;
	      for (j = 0; j < bank->num_reg; j++)
		{
		  if (map[j * size] == preg->value.mac)
		    {
		      constraints = Set_add (constraints,
					     bank->base_index +
					     j * bank->reg_size);
		      found = 1;
		      break;
		    }
		}
	    }
	  if (!found)
	    L_punt ("Regalloc: unknown macro in ill_reg attribute: %d!!\n",
		    preg->value.mac);
	}
      else
	L_punt ("Regalloc: illegal operand type in ill_reg attribute!!\n");
    }
  vreg_ptr->illegal_reg = Set_union (vreg_ptr->illegal_reg, constraints);
  Set_dispose (constraints);
  return (NULL);
}

/* Use isl attribute instead of old scheduler structures to get
 * operation issue time. -JCG 6/99
 */
int
R_issue_time (L_Oper * oper)
{
  L_Attr *attr;

  /* Handle case where prepass scheduling is not done */
  if ((attr = L_find_attr (oper->attr, "isl")) ||
      (attr = L_find_attr (oper->attr, "cycle")))
    {
      /* Sanity check, make sure first field is an int */
      if ((attr->field[0] == NULL) || !L_is_int_constant (attr->field[0]))
	L_punt ("R_issue_time: isl field[0] not an int!");

      return (attr->field[0]->value.i);
    }

  return (-1);
}

/* Build a set with all of the registers that require rotation for 
 * this function.
 * Use the following functions to access:   
 * Set_dispose(R_rot_reg);
 * Set_in(R_rot_reg, reg_id) 
 */
Set
R_build_rotating_reg_set (L_Func * fn)
{
  Set R_rot_reg = NULL;
  int int_rot_reg_base;
  int int_rot_reg_num;
  int flt_rot_reg_base;
  int flt_rot_reg_num;
  int dbl_rot_reg_base;
  int dbl_rot_reg_num;
  int pred_rot_reg_base;
  int pred_rot_reg_num;
  int i;

  R_get_rot_regs (fn, &int_rot_reg_base, &int_rot_reg_num,
		  &flt_rot_reg_base, &flt_rot_reg_num,
		  &dbl_rot_reg_base, &dbl_rot_reg_num,
		  &pred_rot_reg_base, &pred_rot_reg_num);

  if (int_rot_reg_num == 0 &&
      flt_rot_reg_num == 0 && dbl_rot_reg_num == 0 && pred_rot_reg_num == 0)
    {
      return NULL;
    }

  for (i = int_rot_reg_base; i < (int_rot_reg_base + int_rot_reg_num); i++)
    R_rot_reg = Set_add (R_rot_reg, i);

  for (i = flt_rot_reg_base; i < (flt_rot_reg_base + flt_rot_reg_num); i++)
    R_rot_reg = Set_add (R_rot_reg, i);

  for (i = dbl_rot_reg_base; i < (dbl_rot_reg_base + dbl_rot_reg_num); i++)
    R_rot_reg = Set_add (R_rot_reg, i);

  for (i = pred_rot_reg_base; i < (pred_rot_reg_base + pred_rot_reg_num); i++)
    R_rot_reg = Set_add (R_rot_reg, i);

  return R_rot_reg;
}

void
R_get_rot_regs (L_Func * fn, int *int_base, int *int_num,
		int *flt_base, int *flt_num,
		int *dbl_base, int *dbl_num, int *pred_base, int *pred_num)
{
  L_Attr *rr_attr = NULL;

  /* Check for erroneous pointer to the function */
  if (fn == NULL)
    L_punt ("R_get_rotating_reg_set: NULL function input.");

  /* Find the attribute that contains the rotating register numbers. */
  rr_attr = L_find_attr (fn->attr, "rr");

  /* Obtain the ranges of the rotating registers, or return the empty
     set of there are none for this function. */
  if (rr_attr != NULL)
    {
      *int_base = rr_attr->field[0]->value.i;
      *int_num = rr_attr->field[1]->value.i;
      *flt_base = rr_attr->field[2]->value.i;
      *flt_num = rr_attr->field[3]->value.i;
      *dbl_base = rr_attr->field[4]->value.i;
      *dbl_num = rr_attr->field[5]->value.i;
      *pred_base = rr_attr->field[6]->value.i;
      *pred_num = rr_attr->field[7]->value.i;
    }
  else
    {
      int start_reg = (((++fn->max_reg_id) + 100) / 100) * 100;
      
      *int_base = start_reg;
      if (M_arch == M_TAHOE)
	*flt_base = *int_base + 96;
      else
	*flt_base = *int_base +
	  R_get_rot_reg_max_alloc (M_native_int_register_ctype());
#if 0
      *dbl_base = *flt_base +
	R_get_rot_reg_max_alloc (L_CTYPE_FLOAT);
#else
      *dbl_base = *flt_base + 0;
#endif

      *pred_base = *dbl_base +
	R_get_rot_reg_max_alloc (L_CTYPE_DOUBLE);
      fn->max_reg_id = *pred_base +
	R_get_rot_reg_max_alloc (L_CTYPE_PREDICATE);

      *int_num = 0;
      *flt_num = 0;
      *dbl_num = 0;
      *pred_num = 0;

      rr_attr = L_new_attr ("rr", 8);
      L_set_int_attr_field (rr_attr, 0, *int_base);
      L_set_int_attr_field (rr_attr, 1, 0);
      L_set_int_attr_field (rr_attr, 2, *flt_base);
      if (M_arch != M_TAHOE)
	L_set_int_attr_field (rr_attr, 3, 0);
      else
#if 0
	L_set_int_attr_field (rr_attr, 3, 
			      R_get_rot_reg_max_alloc (L_CTYPE_FLOAT));      
#else
	L_set_int_attr_field (rr_attr, 3, 0);
#endif
      L_set_int_attr_field (rr_attr, 4, *dbl_base);
      if (M_arch != M_TAHOE)
	L_set_int_attr_field (rr_attr, 5, 0);
      else
	L_set_int_attr_field (rr_attr, 5, 
			      R_get_rot_reg_max_alloc (L_CTYPE_DOUBLE));
      L_set_int_attr_field (rr_attr, 6, *pred_base);
      if (M_arch != M_TAHOE)
	L_set_int_attr_field (rr_attr, 7, 0);
      else
	L_set_int_attr_field (rr_attr, 7, 
			      R_get_rot_reg_max_alloc (L_CTYPE_PREDICATE));
      fn->attr = L_concat_attr (fn->attr, rr_attr);
    }

  return;
}

/*===========================================================================
 *
 *      Func :	R_process_cb()   
 *      Desc :  Defines an R_Instr structure for every instruction within
 *		the control block to be used for dataflow analysis. 
 *	 	The operands of each instruction are examined for virtual
 *		registers or macros contained in <R_avail_macro_array>	
 *      Input:  L_Func *fn	- current function
 *		L_Cb *cb	- current control block 
 *      Output: none
 *
 *      Side Effects: allocates and defines an R_Instr structure for 
 *		      each instruction  
 *
 *===========================================================================*/

#define R_MATCHING_MACRO(l_opd, r_mac) (((l_opd)->value.mac == (r_mac).id) && \
    (R_Ltype_to_Rtype (L_return_old_ctype ((l_opd))) == (r_mac).type))


void
R_process_cb (L_Cb * cb)
{
  L_Oper *oper;
  L_Operand *opd;
  int i, j, this_cycle = -1, new_cycle;
  int *buffer = R_buf;
  L_Attr *attr;
  Set cb_vregs = NULL, cb_oper_set = NULL, defined_this_cycle = NULL;

  int hyperblock = 
    L_EXTRACT_BIT_VAL (L_fn->flags, L_FUNC_CC_IN_PREDICATE_REGS) | 
    L_EXTRACT_BIT_VAL (cb->flags, L_CB_HYPERBLOCK);
  
  /* if local_conflict, all live ranges def'd/used in a given CB conflict. */
  int local_conflict = R_Schedule_Invariant_Alloc;

  /* if same_cycle_conflict, zero-cycle anti-dependences cause live ranges
   * to conflict. */
  int same_cycle_conflict = R_Same_Cycle_Anti_Deps_Interference &&
    (M_arch != M_X86);

  /* Scan each instruction in the cb, adding the use and def information
     to the set of live ranges. */
  for (oper = cb->first_op; oper; oper = oper->next_op)
    {
      int mac, vreg = 0;
      R_Reg *vreg_ptr = NULL;

      R_oper_set = Set_add (R_oper_set, oper->id);

      /* build a set of oper id's for cb */
      if (local_conflict)
	cb_oper_set = Set_add (cb_oper_set, oper->id);

      if (same_cycle_conflict && 
	  ((new_cycle = R_issue_time (oper)) != -1) &&
	  (new_cycle != this_cycle))
	{
	  defined_this_cycle = Set_dispose (defined_this_cycle);
	  this_cycle = new_cycle;
	}

      /* guard predicate */

      if ((opd = oper->pred[0]) && L_is_variable (opd))
	{
	  if (L_is_reg (opd))
	    {
	      vreg = opd->value.r;
	      vreg_ptr = R_add_register (vreg, L_return_old_ctype (opd), 
					 oper->weight, 0);
	    }
	  else if (L_is_macro (opd))
	    {
	      mac = opd->value.mac;
	      vreg = -1 - mac;
	      
	      for (j = 0; j < R_n_avail_macro; j++)
		if (R_MATCHING_MACRO(opd, R_avail_macro[j]))
		  {
		    vreg = R_avail_macro[j].vreg_id;			
		    vreg_ptr = VREG (vreg);

		    if (!(vreg_ptr->flags & R_CURRENT_VREG))
		      vreg_ptr->flags |= R_CURRENT_VREG;		

		    L_assign_type_general_register (opd);
		    opd->value.r = vreg;

		    break;
		  }
	    }

	  L_add_src_operand_reg (oper, vreg, FALSE, TRUE);

	  if (vreg >= 0)
	    {
	      vreg_ptr->ref_instr = Set_add (vreg_ptr->ref_instr, oper->id);

	      if (M_model == M_HP_PLAYDOH_MCODE)
		vreg_ptr->ref_cbs = Set_add (vreg_ptr->ref_cbs, cb->id);

	      if (local_conflict)
		cb_vregs = Set_add (cb_vregs, vreg);
	    }
	}

      /* src operands */

      for (i = 0; i < L_max_src_operand; i++)
	{
	  if (!(opd = oper->src[i]))
	    continue;
	  if (L_is_reg (opd))
	    {
	      vreg = opd->value.r;
	      vreg_ptr = R_add_register (vreg, L_return_old_ctype (opd), 
					 oper->weight, 0);
	    }
	  else if (L_is_macro (opd))
	    {
	      mac = opd->value.mac;
	      vreg = -1 - mac;
	      for (j = 0; j < R_n_avail_macro; j++)
		{
		  if (R_MATCHING_MACRO(opd, R_avail_macro[j]))
		    {
		      vreg = R_avail_macro[j].vreg_id;
		      vreg_ptr = VREG (vreg);

		      if (!(vreg_ptr->flags & R_CURRENT_VREG))
			vreg_ptr->flags |= R_CURRENT_VREG;

		      L_assign_type_general_register (opd);
		      opd->value.r = vreg;

		      break;
		    }
		}
	    }
	  else
	    {
	      continue;
	    }

	  L_add_src_operand_reg (oper, vreg, FALSE, FALSE);

	  if (vreg >= 0)
	    {
	      vreg_ptr->ref_instr =
		Set_add (vreg_ptr->ref_instr, oper->id);

	      /* we need a general way to do this */
	      if (M_arch == M_HPPA && oper->opc == Lop_MUL_F &&
		  L_find_attr (oper->attr, "reg_special"))
		vreg_ptr->flags |= R_PA_HACK;

	      if (M_model == M_HP_PLAYDOH_MCODE)
		vreg_ptr->ref_cbs = Set_add (vreg_ptr->ref_cbs, cb->id);

	      if (local_conflict)
		cb_vregs = Set_add (cb_vregs, vreg);
	    }
	}

      /* EMN: Extended source support */

      if ((oper->opc == Lop_CHECK) && (attr = L_find_attr (oper->attr, "src")))
	for (i = 0; i < attr->max_field; i++)
	  {
	    if (!(opd = attr->field[i]))
	      continue;

	    if (L_is_reg (opd))
	      {
		vreg = opd->value.r;
		vreg_ptr = R_add_register (vreg, L_return_old_ctype (opd),
					   oper->weight, 0);
	      }
	    else if (L_is_macro (opd))
	      {
		printf ("regalloc.c : here\n");
		mac = opd->value.mac;
		vreg = -1 - mac;

		for (j = 0; j < R_n_avail_macro; j++)
		  if (R_MATCHING_MACRO(opd, R_avail_macro[j]))
		    {
		      vreg = R_avail_macro[j].vreg_id;
		      vreg_ptr = VREG (vreg);
		      
		      if (!(vreg_ptr->flags & R_CURRENT_VREG))
			vreg_ptr->flags |= R_CURRENT_VREG;

		      L_assign_type_general_register (opd);
		      opd->value.r = vreg;
		      break;
		    }
	      }
	    else
	      {
		continue;
	      }

	    L_add_src_operand_reg (oper, vreg, FALSE, FALSE);

	    if (vreg >= 0)
	      {
		vreg_ptr->ref_instr =
		  Set_add (vreg_ptr->ref_instr, oper->id);

		/* we need a general way to do this */
		if (M_arch == M_HPPA && oper->opc == Lop_MUL_F &&
		    L_find_attr (oper->attr, "reg_special"))
		  vreg_ptr->flags |= R_PA_HACK;

		if (M_model == M_HP_PLAYDOH_MCODE)
		  vreg_ptr->ref_cbs = Set_add (vreg_ptr->ref_cbs, cb->id);

		if (local_conflict)
		  cb_vregs = Set_add (cb_vregs, vreg);
	      }
	  }

      /* dest operands */

      for (i = 0; i < L_max_dest_operand; i++)
	{
	  int trans = FALSE, uncond = FALSE;
		
	  if (!(opd = oper->dest[i]))
	    continue;

	  if (L_is_reg (opd))
	    {
	      vreg = opd->value.r;
	      vreg_ptr = R_add_register (vreg, L_return_old_ctype (opd), 
					 oper->weight, 1);
	    }
	  else if (L_is_macro (opd))
	    {
	      mac = opd->value.mac;
	      vreg = -1 - mac;

	      for (j = 0; j < R_n_avail_macro; j++)
		if (R_MATCHING_MACRO(opd, R_avail_macro[j]))
		  {
		    vreg = R_avail_macro[j].vreg_id;
		    vreg_ptr = VREG (vreg);

		    if (!(vreg_ptr->flags & R_CURRENT_VREG))
		      vreg_ptr->flags |= R_CURRENT_VREG;

		    L_assign_type_general_register (opd);
		    opd->value.r = vreg;
		    break;
		  }
	    }
	  else
	    {
	      continue;
	    }

	  if (L_is_ctype_predicate (opd))
	    {
	      if (L_is_update_predicate_ptype (opd->ptype))
		trans = TRUE;
	      if (L_is_uncond_predicate_ptype (opd->ptype))
		uncond = TRUE;
	    }

	  L_add_dest_operand_reg(oper, vreg, trans, uncond);

	  if (vreg >= 0)
	    {
	      vreg_ptr->ref_instr = Set_add (vreg_ptr->ref_instr, oper->id);
	      vreg_ptr->def_instr = Set_add (vreg_ptr->def_instr, oper->id);

	      /*
	       * The defining instruction must be in the live range
	       * within a hyperblock
	       */
	      if (hyperblock)
		{
		  vreg_ptr->live_range =
		    Set_add (VREG (vreg)->live_range, oper->id);
		}

	      /* SAM 3-97 - Remember referencing for all regs w/
		 playdoh emul */
	      if (M_model == M_HP_PLAYDOH_MCODE)
		vreg_ptr->ref_cbs = Set_add (vreg_ptr->ref_cbs, cb->id);

	      /*
	       * MCB Hack
	       */
	      if (R_Prevent_MCB_Preload_Spills &&
		  (L_find_attr (oper->attr, "MCB") != NULL) &&
		  op_flag_set (oper->proc_opc, OP_FLAG_LOAD))
		vreg_ptr->flags |= R_MCB_PRELOAD;

	      /*
	       * PA Hack
	       */
	      if (M_arch == M_HPPA)
		if (oper->opc == Lop_DIV_F2 || oper->opc == Lop_DIV_F ||
		    oper->opc == Lop_SQRT_F2 || oper->opc == Lop_SQRT_F)
		  vreg_ptr->live_range =
		    Set_add (vreg_ptr->live_range, oper->id);

	      /*
	       * CMOV Hack:
	       * If the opcode is a cmov, add the destination operand
	       * as a source for dataflow analysis.
	       */
	      if (L_cmov_opcode (oper))
		L_add_src_operand_reg (oper, vreg, FALSE, FALSE);

	      /*
	       * Region Hack
	       * -> The vreg defined by the last instruction in an exit cb
	       *    of a region will have may have an empty LR, so as a quick
	       *    fix add a last oper to the LR of the vreg, always.
	       */
	      if (R_Region_Based_Allocation)
		vreg_ptr->live_range =
		  Set_add (vreg_ptr->live_range, oper->id);

	      if (!oper->next_op)
		vreg_ptr->live_range =
		  Set_add (vreg_ptr->live_range, oper->id);

	      /*
	       * Add previous instructions at the same issue time to the 
	       * live range, subsquent instructions at the same issue time
	       * will be placed in the live range automatically, unless
	       * you can issue flow-dep instructions in the same cycle causing
	       * the live range do be defined, used and dead in the same cycle
	       */

	      if (same_cycle_conflict && this_cycle != -1)
		{
		  L_Oper *prev = oper;

		  vreg_ptr->intf_vreg = Set_union_acc (vreg_ptr->intf_vreg,
						       defined_this_cycle);
		  defined_this_cycle = Set_add (defined_this_cycle, vreg);

		  while ((prev = prev->prev_op) &&
			 (R_issue_time (prev) == this_cycle))
		    {
		      vreg_ptr->live_range =
			Set_add (vreg_ptr->live_range, prev->id);
		    }
		}

	      if ((M_arch == M_HPPA) && 
		  (oper->opc == Lop_MUL_F) &&
		  L_find_attr (oper->attr, "reg_special"))
		vreg_ptr->flags |= R_PA_HACK;

	      if (local_conflict)
		cb_vregs = Set_add (cb_vregs, vreg);
	    }
	}

      if (op_flag_set (oper->proc_opc, OP_FLAG_JSR))
        R_jsr_list = List_insert_last (R_jsr_list, oper);

      if (oper->opc == Lop_CHECK)
	R_extend_set = Set_add (R_extend_set, oper->id);

      /*
       * Constraint Hack - added for x86 :)
       */
      attr = oper->attr;
      while ((attr = L_find_attr (attr, "ill_reg")) != NULL)
	{
	  R_process_illegal_register_attr (attr);
	  attr = attr->next_attr;
	}

      if (op_flag_set (oper->proc_opc, OP_FLAG_RTS))
	{
	  R_rts = oper;
	}
    }

  if (defined_this_cycle)
    defined_this_cycle = Set_dispose (defined_this_cycle);

  /*
   * In effort make the allocation invariant to small scheduling
   * changes, the lifetime of a vreg is set to contain every operation
   * within a cb in which it is referenced.  It will also contain every
   * operation in a cb wherein it is live, this will be handled
   * during live range construction.
   */

  if (local_conflict)
    {
      int n_vreg = Set_2array (cb_vregs, buffer);
      for (i = 0; i < n_vreg; i++)
	{
	  R_Reg *vreg = VREG (buffer[i]);
	  vreg->intf_vreg = Set_union_acc (vreg->intf_vreg, cb_vregs);

	  vreg->live_range = Set_union_acc (vreg->live_range, cb_oper_set);
	}
      Set_dispose (cb_vregs);
      Set_dispose (cb_oper_set);
    }
  return;
}


void
R_process_cb_for_spilling (L_Func * fn, L_Cb * cb)
{
  L_Oper *oper;
  L_Operand *opd;

  for (oper = cb->first_op; oper != NULL; oper = oper->next_op)
    {
      int i;
      R_Reg *vreg_ptr;
      L_Attr *attr;

      /* dest operands */

      for (i = 0; i < L_max_dest_operand; i++)
	if ((opd = oper->dest[i]) && L_is_reg (opd))
	  vreg_ptr = R_add_register (opd->value.r, L_return_old_ctype (opd),
				     oper->weight, 1);

      /* src operands */

      for (i = 0; i < L_max_src_operand; i++)
	if ((opd = oper->src[i]) && L_is_reg (opd))
	  vreg_ptr = R_add_register (opd->value.r, L_return_old_ctype (opd),
				     oper->weight, 0);

      /* guard predicate */
      
      if ((opd = oper->pred[0]) && L_is_reg (opd))
	vreg_ptr = R_add_register (opd->value.r, L_return_old_ctype (opd),
				   oper->weight, 0);

      /* EMN: Extended source support */

      if ((attr = L_find_attr (oper->attr, "src")) && oper->opc == Lop_CHECK)
	for (i = 0; i < attr->max_field; i++)
	  if ((opd = attr->field[i]) && L_is_reg (opd))
	    vreg_ptr = R_add_register (opd->value.r, L_return_old_ctype (opd),
				       oper->weight, 0);
    }

  return;
}

R_Reg *
R_reset_vreg_list (R_Reg * vregList)
{
  R_Reg *vreg, *next, *newList = NULL;

  for (vreg = vregList; vreg != NULL; vreg = next)
    {
      next = vreg->nextReg;

      if (vreg->flags & R_PREALLOCATED_MACRO)
	{
	  vreg->nextReg = newList;
	  newList = vreg;
	}
      else
	{
	  vreg->flags = 0;
	  vreg->size = 0;
	  vreg->rclass = R_PREFER_SPILL;
	  vreg->base_index = -1;
	  vreg->nextReg = NULL;
	  vreg->live_range = NULL;
	  if (vreg->ref_instr != vreg->live_range)
	    {
	      Set_dispose (vreg->def_instr);
	      Set_dispose (vreg->ref_cbs);
	      Set_dispose (vreg->ref_instr);
	      Set_dispose (vreg->live_range);
	    }
	  else
	    {
	      Set_dispose (vreg->live_range);
	    }
	}
      vreg->spill_loc = -1;

      vreg->ref_instr = NULL;
      vreg->def_instr = NULL;
      vreg->ref_cbs = NULL;
      vreg->pvreg = NULL;
      vreg->ref_weight = 0.0;
      vreg->def_weight = 0.0;
      vreg->instr_weight = 0;
      vreg->priority = 0.0;
      vreg->interfere = NULL;
      vreg->rotating = 0;
      vreg->nth_rot_reg = -1;
      vreg->caller_benefit = -1;
      vreg->callee_benefit = -1;
    }
  return (newList);
}

/*===========================================================================
 *
 *      Func :	R_register_saving_convention_selection()
 *      Desc :  From the execution frequency of the function and the
 *		number of subroutine calls contained within a given
 *		live range, this function determines whether a caller or
 *		callee saved register is more beneficial.
 *      Input:  L_Func *fn	- current function
 *      Output: none
 *
 *      Side Effects: allocates and defines an R_Instr structure for 
 *		      each instruction  
 *
 *===========================================================================*/
void
R_register_saving_convention_selection (L_Func * fn, L_Region * region)
{
  int i, j, leaf, index;
  double spill_weight;
  int *pref_alloc, *pref_alloc1, *pref_alloc2;
  int callee_allocated1 = 0;
  double caller_benefit, callee_benefit, *callee_benefit2 = NULL;
  double total_benefit1 = 0.0, total_benefit2 = 0.0;
  R_Reg *vreg;
  L_Oper *cur_jsr;

  double callee_cost[R_NUM_TYPES];
  double caller_cost[R_NUM_TYPES];
  double spill_load_cost[R_NUM_TYPES];
  double spill_store_cost[R_NUM_TYPES];

  if (R_Region_Based_Allocation)
    {
      if (L_find_attr (region->attr, "leaf") != NULL)
	leaf = 1;
      else
	leaf = 0;
    }
  else
    {
      leaf = (List_size (R_jsr_list) == 0);
    }

  /*
   * Initialize caller/callee/spill cost function data
   * from the code generator
   */
  caller_cost[0] = R_caller_cost (L_CTYPE_PREDICATE, leaf);
  caller_cost[1] = R_caller_cost (L_CTYPE_INT, leaf);
  caller_cost[2] = R_caller_cost (L_CTYPE_FLOAT, leaf);
  caller_cost[3] = R_caller_cost (L_CTYPE_DOUBLE, leaf);
  caller_cost[4] = R_caller_cost (L_CTYPE_BTR, leaf);
  caller_cost[5] = R_caller_cost (L_CTYPE_POINTER, leaf);

  callee_cost[0] = R_callee_cost (L_CTYPE_PREDICATE, leaf, 0);
  callee_cost[1] = R_callee_cost (L_CTYPE_INT, leaf, 0);
  callee_cost[2] = R_callee_cost (L_CTYPE_FLOAT, leaf, 0);
  callee_cost[3] = R_callee_cost (L_CTYPE_DOUBLE, leaf, 0);
  callee_cost[4] = R_callee_cost (L_CTYPE_BTR, leaf, 0);
  callee_cost[5] = R_callee_cost (L_CTYPE_POINTER, leaf, 0);

#if 0
  sub_callee_cost[0] = R_callee_cost (L_CTYPE_PREDICATE, leaf, 1);
  sub_callee_cost[1] = R_callee_cost (L_CTYPE_INT, leaf, 1);
  sub_callee_cost[2] = R_callee_cost (L_CTYPE_FLOAT, leaf, 1);
  sub_callee_cost[3] = R_callee_cost (L_CTYPE_DOUBLE, leaf, 1);
#endif

  spill_load_cost[0] = R_spill_load_cost (L_CTYPE_PREDICATE);
  spill_load_cost[1] = R_spill_load_cost (L_CTYPE_INT);
  spill_load_cost[2] = R_spill_load_cost (L_CTYPE_FLOAT);
  spill_load_cost[3] = R_spill_load_cost (L_CTYPE_DOUBLE);
  spill_load_cost[4] = R_spill_load_cost (L_CTYPE_BTR);
  spill_load_cost[5] = R_spill_load_cost (L_CTYPE_POINTER);

  spill_store_cost[0] = R_spill_store_cost (L_CTYPE_PREDICATE);
  spill_store_cost[1] = R_spill_store_cost (L_CTYPE_INT);
  spill_store_cost[2] = R_spill_store_cost (L_CTYPE_FLOAT);
  spill_store_cost[3] = R_spill_store_cost (L_CTYPE_DOUBLE);
  spill_store_cost[4] = R_spill_store_cost (L_CTYPE_BTR);
  spill_store_cost[5] = R_spill_store_cost (L_CTYPE_POINTER);

  if (R_Print_Class_Selection)
    {
      fprintf (stderr, "\n*** Register Bank Class Selection ***\n\n");
      fprintf (stderr, "(function weight+1) ----> %f\n", (fn->weight + 1.0));
      fprintf (stderr, "Store Cost:\n");
      fprintf (stderr, "\tPredicate --> %.5f\n", spill_store_cost[0]);
      fprintf (stderr, "\tInt       --> %.5f\n", spill_store_cost[1]);
      fprintf (stderr, "\tFloat     --> %.5f\n", spill_store_cost[2]);
      fprintf (stderr, "\tDouble    --> %.5f\n", spill_store_cost[3]);
      fprintf (stderr, "\tBtr       --> %.5f\n", spill_store_cost[4]);
      fprintf (stderr, "\tPointer   --> %.5f\n", spill_store_cost[5]);
      fprintf (stderr, "\nLoad Cost:\n");
      fprintf (stderr, "\tPredicate --> %.5f\n", spill_load_cost[0]);
      fprintf (stderr, "\tInt       --> %.5f\n", spill_load_cost[1]);
      fprintf (stderr, "\tFloat     --> %.5f\n", spill_load_cost[2]);
      fprintf (stderr, "\tDouble    --> %.5f\n", spill_load_cost[3]);
      fprintf (stderr, "\tBtr       --> %.5f\n", spill_load_cost[4]);
      fprintf (stderr, "\tPointer   --> %.5f\n", spill_load_cost[5]);
      fprintf (stderr, "\nCaller Cost:\n");
      fprintf (stderr, "\tPredicate --> %.5f\n", caller_cost[0]);
      fprintf (stderr, "\tInt       --> %.5f\n", caller_cost[1]);
      fprintf (stderr, "\tFloat     --> %.5f\n", caller_cost[2]);
      fprintf (stderr, "\tDouble    --> %.5f\n", caller_cost[3]);
      fprintf (stderr, "\tBtr       --> %.5f\n", caller_cost[4]);
      fprintf (stderr, "\tPointer   --> %.5f\n", caller_cost[5]);
      fprintf (stderr, "\nCallee Cost:\n");
      fprintf (stderr, "\tPredicate --> %.5f\n", callee_cost[0]);
      fprintf (stderr, "\tInt       --> %.5f\n", callee_cost[1]);
      fprintf (stderr, "\tFloat     --> %.5f\n", callee_cost[2]);
      fprintf (stderr, "\tDouble    --> %.5f\n", callee_cost[3]);
      fprintf (stderr, "\tBtr       --> %.5f\n", callee_cost[4]);
      fprintf (stderr, "\tPointer   --> %.5f\n", callee_cost[5]);
#if 0
      fprintf (stderr, "\nSubsequent Callee Cost:\n");
      fprintf (stderr, "\tPredicate --> %.5f\n", sub_callee_cost[0]);
      fprintf (stderr, "\tInt       --> %.5f\n", sub_callee_cost[1]);
      fprintf (stderr, "\tFloat     --> %.5f\n", sub_callee_cost[2]);
      fprintf (stderr, "\tDouble    --> %.5f\n", sub_callee_cost[3]);
#endif
      fprintf (stderr, "\nSubroutine Calls:\n");

      List_start (R_jsr_list);
      while ((cur_jsr = (L_Oper *)List_next (R_jsr_list)))
	fprintf (stderr, "\t%d\n", cur_jsr->id);
    }

  pref_alloc1 = (int *) malloc (sizeof (int) * (fn->max_reg_id + 1));
  pref_alloc2 = (int *) malloc (sizeof (int) * (fn->max_reg_id + 1));

  i = 0;
  total_benefit1 = 0.0;
  total_benefit2 = 0.0;

  for (vreg = R_vreg; vreg != NULL; vreg = vreg->nextReg)
    {
      double jsr_cost = 0.0;
      double ref_weight = -1.0;

      if (vreg->flags & R_PREALLOCATED_MACRO)
	continue;

      List_start (R_jsr_list);
      while ((cur_jsr = (L_Oper *)List_next (R_jsr_list)))
	{
	  /* JEP 9/96, Bob McGowan 10/97
	     A branch register which used as a jsr src is not live over the
	     call.  Therefore a caller save register can be used.
	     If the branch register is live-in on the op following the call,
	     then a callee register should be used. */
	  if (vreg->type == R_BTR)
	    {
	      if (Set_in (vreg->live_range, cur_jsr->id))
		{
		  L_Oper *jsr_next_oper;

		  jsr_next_oper = cur_jsr->next_op;

		  if ((jsr_next_oper == NULL) ||
		      Set_in (vreg->live_range, jsr_next_oper->id))
		    {
		      jsr_cost += cur_jsr->weight;
		      vreg->flags |= R_CONTAINS_JSR;
		    }
		}
	    }
	  else if (Set_in (vreg->live_range, cur_jsr->id))
	    {
	      jsr_cost += cur_jsr->weight;
	      vreg->flags |= R_CONTAINS_JSR;
	    }
	}
      if (vreg->flags & R_CONTAINS_JSR)
	{
	  if (vreg->flags & R_PREALLOCATED_FLYBY)
	    {
	      L_Region_Regmap *regmap;

	      regmap = L_find_region_regmap (region, vreg->index);
	      if (regmap)
		{
		  ref_weight = regmap->ref_weight;
		  if (regmap->jsr_weight > jsr_cost)
		    jsr_cost = regmap->jsr_weight;
		  if (regmap->phys_reg != -1)
		    {
		      if (regmap->type == R_CALLER ||
			  regmap->type == R_MACRO_CALLER)
			{
			  /*
			     fprintf(stdout,"FOUND FLYBY WITH JSR\n");
			   */
			  vreg->flags &= ~(R_PREALLOCATED_FLYBY);
			  vreg->flags |= R_REGION_CONSTRAINED;
			}
		    }
		}
	    }
	  else
	    {
	      L_Region_Regmap *regmap;

	      regmap = L_find_region_regmap (region, vreg->index);
	      if (regmap)
		ref_weight = regmap->ref_weight;
	      if (regmap && (regmap->jsr_weight > jsr_cost))
		jsr_cost = regmap->jsr_weight;
	    }

	}
      else
	{
	  L_Region_Regmap *regmap;

	  regmap = L_find_region_regmap (region, vreg->index);
	  if (regmap)
	    ref_weight = regmap->ref_weight;
	  if (regmap && (regmap->jsr_weight > jsr_cost))
	    jsr_cost = regmap->jsr_weight;
	}

      switch (vreg->type)
	{
	case R_PREDICATE:
	  index = 0;
	  break;
	case R_INT:
	  index = 1;
	  break;
	case R_FLOAT:
	  index = 2;
	  break;
	case R_DOUBLE:
	  index = 3;
	  break;
	case R_BTR:
	  index = (M_arch == M_TAHOE) ? 4 : 1;
	  break;
	case R_POINTER:
	  index = (M_arch == M_STARCORE) ? 5 :1;
	  break;
	default:
	  index = -1;
	  L_punt ("Regalloc: Undefined register type index");
	  break;
	}

      if (ref_weight < 0.0)
	spill_weight = (vreg->def_weight + 1) * spill_store_cost[index] +
	  (vreg->ref_weight - vreg->def_weight + 1) * spill_load_cost[index];
      else
	spill_weight = ref_weight * spill_store_cost[index];

      /*
         rec_weight = R_determine_reconcile_cost(region,vreg,
         spill_store_cost+index,
         spill_load_cost+index);

       */

      /*
       * Determine benefit of allocation to caller/callee saved registers
       */

      caller_benefit = spill_weight /*+ rec_weight */  -
	((jsr_cost + 0.5) * caller_cost[index]);
      callee_benefit = spill_weight /*+ rec_weight */  -
	((fn->weight + 1.0) * callee_cost[index]);

      if (R_Print_Class_Selection)
	{
	  fprintf (stderr, "VREG #%d reg_wgt = %f jsr_cost = %f",
		   vreg->index, vreg->ref_weight, jsr_cost);
	  fprintf (stderr, " caller_benefit = %f callee_benefit = %f",
		   caller_benefit, callee_benefit);

	  fprintf (stderr, "\n");
	}

      /* Assign virtual register caller save benefit */

      /* Caller/callee selection preferences are preformed twice: The
	 first time, <caller_benefit> and <callee_benefit> are used and
	 if a callee register is chosen, <callee_benefit2> is used
	 thereafter.
	 
	 To handle the case where <callee_benefit> may be much lower than
	 <callee_benefit2>, a better allocation may be achieved if we
	 "force" a register to be preferred callee.  Thus the selections
	 are made using <caller_benefit> and <callee_benefit2>, with a
	 small correction to <total_benefit2> to account for the one
	 callee register with benefit <callee_benefit>.
	 
	 The preference selection with the largest benefit is chosen. */

      /* Assign virtual register callee save benefit */

      if (caller_benefit >= 0.0)
	{
	  if (callee_benefit >= 0.0)
	    {
	      if (caller_benefit - callee_benefit >= caller_callee_threshold)
		{
		  vreg->caller_benefit = 1;
		  vreg->callee_benefit = 0;
		  pref_alloc1[i] = R_CALLER;
		  total_benefit1 += caller_benefit;
		}
	      else
		{
		  vreg->caller_benefit = 0;
		  vreg->callee_benefit = 1;
		  pref_alloc1[i] = R_CALLEE;
		  total_benefit1 += callee_benefit;
		  callee_allocated1 = 1;
		}
	    }
	  else
	    {
	      vreg->caller_benefit = 1;
	      vreg->callee_benefit = -1;
	      pref_alloc1[i] = R_CALLER;
	      total_benefit1 += caller_benefit;
	    }
	}
      else if (callee_benefit >= 0.0)
	{
	  vreg->caller_benefit = -1;
	  vreg->callee_benefit = 1;
	  pref_alloc1[i] = R_CALLEE;
	  total_benefit1 += callee_benefit;
	  callee_allocated1 = 1;
	}
      else
	{
	  vreg->caller_benefit = -1;
	  vreg->callee_benefit = -1;
	  pref_alloc1[i] = R_PREFER_SPILL;
	}
      /* increment virtual register count */
      i += 1;
    }

  pref_alloc = pref_alloc1;


  if (R_Print_Class_Selection)
    fprintf (stderr, "\nAllocation benefit: %f\n", total_benefit1);

  i = 0;
  for (vreg = R_vreg; vreg != NULL; vreg = vreg->nextReg)
    {
      if (vreg->flags & R_PREALLOCATED_MACRO)
	continue;

      vreg->rclass = pref_alloc[i];

      i += 1;
      if (R_Print_Class_Selection)
	{
	  fprintf (stderr, "VREG #%d", vreg->index);

	  switch (vreg->rclass)
	    {
	    case R_CALLER:
	      fprintf (stderr, " prefer-caller\n");
	      break;
	    case R_CALLEE:
	      fprintf (stderr, " prefer-callee\n");
	      break;
	    default:
	      fprintf (stderr, " prefer to spill\n");
	      break;
	    }
	}
    }

  /* SAM 3-97 - Hack added for Playdoh emulation.  The problem is with
     emulion of scheduled code , function calls to Impact_Update() are
     inserted in phase3 (after allocation), thus any caller registers
     spanning these funct calls will not be saved/restored properly.
     Here any vregs that prefer caller regs are changed to callee or
     spill to prevent the problem.  I wasn't sure how to tell if the
     liverange spanned a call to Impact_Update(), the check if the
     ref_cbs > 1 seemed like a reasonable approximation. */

  if ((M_model == M_HP_PLAYDOH_MCODE) &&
      (L_EXTRACT_BIT_VAL (fn->flags, L_FUNC_SCHEDULED)))
    {
      for (vreg = R_vreg; vreg != NULL; vreg = vreg->nextReg)
	{
	  if ((vreg->rclass == R_CALLER) && (Set_size (vreg->ref_cbs) > 1))
	    {
	      vreg->caller_benefit = -1;
	      vreg->rclass = (vreg->callee_benefit > -1) ? R_CALLEE : R_SPILL;
	    }
	}
    }

  free (pref_alloc1);
  free (pref_alloc2);
  if (callee_benefit2)
    free (callee_benefit2);

  return;
}


void
R_determine_reserved_registers (Set * reserved_resource,
				Set * constrained_resource,
				int *constrained_array, R_Reg * vreg)
{
  int i, j;
  unsigned int k;
  int *neighbor = R_buf;
  R_Arc *arc;
  Set rres = NULL, cres = NULL;
  L_Region_Regmap *regmap;

  /* find simultaineously live "live ranges" */

  for (arc = vreg->interfere, i = 0; arc; arc = arc->next)
    neighbor[i++] = arc->lr->index;

  /* and the virtual registers assigned to them if any */
  for (j = 0; j < i; j++)
    {
      R_Reg *tvreg = VREG (neighbor[j]);
      if (tvreg->base_index != -1)
	{
	  for (k = 0; k < tvreg->size; k++)
	    rres = Set_add (rres, tvreg->base_index + k);
	}
      if (tvreg->flags & R_REGION_CONSTRAINED)
	{
	  regmap = L_find_region_regmap (R_Region, tvreg->index);
	  cres = Set_add (cres, regmap->phys_reg);
	}
    }
  /* Handle PA-RISC restrictions on single precision 5-ops */
  if (vreg->flags & R_PA_HACK)
    rres = Set_union_acc (rres, R_pa_set);

  /* Handle illegal registers */
  rres = Set_union_acc (rres, vreg->illegal_reg);

  /* 
   * Handle constraints imposed by previously allocated regions 
   */
  if (!(vreg->flags & R_SPILLED))
    {
      regmap = L_find_region_regmap (R_Region, vreg->index);
      if (regmap != NULL)
	rres = Set_union_acc (rres, regmap->occupied);
    }
  *reserved_resource = rres;
  *constrained_resource = cres;
}

/*===========================================================================
 *
 *      Func :  R_conv_type_to_Ltype()
 *      Desc :  Converts a register allocator specific type (R_INT, etc.) to
 *		its equivalent Lcode type (L_CTYPE_INT,etc.)
 *      Input:  int type	- regalloc specific type 
 *      Output: Lcode type 
 *
 *      Side Effects: none 
 *
 *===========================================================================*/
int
R_conv_type_to_Ltype (int type)
{
  switch (type)
    {
    case R_INT:
      return (M_native_int_register_ctype ());
    case R_FLOAT:
      return (L_CTYPE_FLOAT);
    case R_DOUBLE:
      return (L_CTYPE_DOUBLE);
    case R_PREDICATE:
      return (L_CTYPE_PREDICATE);
    case R_BTR:
      return (L_CTYPE_BTR);
    case R_POINTER:
      return (L_CTYPE_POINTER);
    case R_QUAD:
    default:
      L_punt ("RA (R_conv_type_to_Ltype): invalid bank type");
      return (0);
    }
}

int
R_conv_rclass_to_Lclass (int rclass)
{
  switch (rclass)
    {
    case R_MACRO_CALLER:
    case R_MACRO_CALLEE:
      return (L_OPERAND_MACRO);
    case R_CALLER:
    case R_CALLEE:
      return (L_OPERAND_REGISTER);
    default:
      L_punt ("RA (R_conv_rclass_to_Lclass): invalid register class");
      return (0);
    }
}

int
R_Ltype_to_Rtype (int type)
{
  switch (type)
    {
    case L_CTYPE_LLONG:
    case L_CTYPE_INT:
      return (R_INT);
    case L_CTYPE_FLOAT:
      return (R_FLOAT);
    case L_CTYPE_DOUBLE:
      return (R_DOUBLE);
    case L_CTYPE_PREDICATE:
      return (R_PREDICATE);
    case L_CTYPE_BTR:
      return (R_BTR);
    case L_CTYPE_POINTER:
      return (R_POINTER);
    default:
      L_punt ("RA (R_Ltype_to_Rtype): invalid ctype");
      return (0);
    }
}


char *
BANK_NAME (int type)
{
  switch (type)
    {
    case R_INT:
      return ("INT");
    case R_PREDICATE:
      return ("PREDICATE");
    case R_FLOAT:
      return ("FLOAT");
    case R_DOUBLE:
      return ("DOUBLE");
    case R_QUAD:
      return ("QUAD");
    case R_BTR:
      return ("BTR");
    case R_POINTER:
      return ("POINTER");
    default:
      return ("??");
    }
}

char *
CLASS_NAME (int rclass)
{
  switch (rclass)
    {
    case R_CALLER:
      return ("CALLER");
    case R_CALLEE:
      return ("CALLEE");
    case R_MACRO_CALLER:
      return ("CALLER MACRO");
    case R_MACRO_CALLEE:
      return ("CALLEE MACRO");
    case R_SPILL:
      return ("SPILL");
    default:
      return ("??");
    }
}

void
R_sort_vreg_list (void)
{
  Heap *sortHeap = Heap_Create (HEAP_MAX);
  R_Reg *vreg;
  int int_base, int_num, flt_base, flt_num, dbl_base, dbl_num, pred_base,
    pred_num;
  int nth_rot_reg;

  R_get_rot_regs (L_fn, &int_base, &int_num, &flt_base, &flt_num,
		  &dbl_base, &dbl_num, &pred_base, &pred_num);

  for (vreg = R_vreg; vreg != NULL; vreg = vreg->nextReg)
    {
      Heap_Insert (sortHeap, vreg, (double) vreg->index);

      if (vreg->type == R_INT)
	{
	  nth_rot_reg = vreg->index - int_base;
	  if (nth_rot_reg >= 0 && nth_rot_reg < int_num)
	    {
	      vreg->rotating = 1;
	      vreg->nth_rot_reg = nth_rot_reg;
	    }
	}
      else if (vreg->type == R_FLOAT)
	{
	  nth_rot_reg = vreg->index - flt_base;
	  if (nth_rot_reg >= 0 && nth_rot_reg < flt_num)
	    {
	      vreg->rotating = 1;
	      vreg->nth_rot_reg = nth_rot_reg;
	    }
	}
      else if (vreg->type == R_DOUBLE)
	{
	  nth_rot_reg = vreg->index - dbl_base;
	  if (nth_rot_reg >= 0 && nth_rot_reg < dbl_num)
	    {
	      vreg->rotating = 1;
	      vreg->nth_rot_reg = nth_rot_reg;
	    }
	}
      else if (vreg->type == R_PREDICATE)
	{
	  nth_rot_reg = vreg->index - pred_base;
	  if (nth_rot_reg >= 0 && nth_rot_reg < pred_num)
	    {
	      vreg->rotating = 1;
	      vreg->nth_rot_reg = nth_rot_reg;
	    }
	}
    }

  R_vreg = NULL;

  while ((vreg = (R_Reg *) Heap_ExtractTop (sortHeap)) != NULL)
    {
      vreg->nextReg = R_vreg;
      R_vreg = vreg;
    }

  Heap_Dispose (sortHeap, NULL);

  return;
}

int
R_init_pa_set (void)
{
  int float_caller_base, float_callee_base;

  R_pa_set = NULL;

  float_caller_base = R_bank[R_FLOAT + R_CALLER].base_index;
  float_callee_base = R_bank[R_FLOAT + R_CALLEE].base_index;

  /* CALLER SAVES */
  R_pa_set = Set_add (R_pa_set, float_caller_base + 0);	/* 8L */
  R_pa_set = Set_add (R_pa_set, float_caller_base + 1);	/* 8R */
  R_pa_set = Set_add (R_pa_set, float_caller_base + 2);	/* 9L */
  R_pa_set = Set_add (R_pa_set, float_caller_base + 3);	/* 9R */
  R_pa_set = Set_add (R_pa_set, float_caller_base + 4);	/*10L */
  R_pa_set = Set_add (R_pa_set, float_caller_base + 5);	/*10R */
  R_pa_set = Set_add (R_pa_set, float_caller_base + 6);	/*11L */
  R_pa_set = Set_add (R_pa_set, float_caller_base + 7);	/*11R */

  /* CALLEE SAVES */
  R_pa_set = Set_add (R_pa_set, float_callee_base + 0);	/*12L */
  R_pa_set = Set_add (R_pa_set, float_callee_base + 1);	/*12R */
  R_pa_set = Set_add (R_pa_set, float_callee_base + 2);	/*13L */
  R_pa_set = Set_add (R_pa_set, float_callee_base + 3);	/*13R */
  R_pa_set = Set_add (R_pa_set, float_callee_base + 4);	/*14L */
  R_pa_set = Set_add (R_pa_set, float_callee_base + 5);	/*14R */
  R_pa_set = Set_add (R_pa_set, float_callee_base + 6);	/*15L */
  R_pa_set = Set_add (R_pa_set, float_callee_base + 7);	/*15R */

  return (0);
}

/*===========================================================================
 *
 *      Func :  R_compute_dataflow_info() 
 *      Desc :  Dataflow information is constructed in two phases.
 *		Phase 1 defines the dataflow graph from the <R_instr> 
 *		information and then reaching definition analysis is 
 *		performed.
 *		Phase 2 scans for jsr instructions and each jsr is made to
 *		define every available caller saved macro register and the
 *		previously determined reaching definitions are used to 
 *		force every jsr to use only those macro registers whose
 *		definitions reach the jsr instruction. 
 *      Input:  none
 *      Output: none 
 *
 *      Side Effects:  calculates dataflow information 
 *
 *===========================================================================*/
void
R_compute_dataflow_info (L_Func * fn)
{
  L_Oper *oper;
  L_Attr *attr;
  R_Reg *vreg;
  int j, n_extend, vreg_id;
  int macro, macro_type;

  int *extend_opers = R_buf2;

  n_extend = Set_2array (R_extend_set, extend_opers);

  /* Handle macroregisters used and defined by each subroutine call */
  List_start (R_jsr_list);
  while ((oper = (L_Oper *)List_next (R_jsr_list)))
    {
      if (oper->opc == Lop_JSR_ND)
	continue;

      for (j = 0; j < R_n_jsr_def_macro; j++)
	{
	  L_add_dest_operand_reg (oper, R_jsr_def_macro[j].vreg_id, FALSE,
				  FALSE);

	  vreg = VREG(R_jsr_def_macro[j].vreg_id);

	  vreg->def_instr = Set_add (vreg->def_instr, 
				     oper->id);
	}

      if ((attr = L_find_attr (oper->attr, "tr")))
	{
	  int k;
	  for (k = 0; k < attr->max_field; k++)
	    {
	      macro = attr->field[k]->value.mac;
	      macro_type =
		R_Ltype_to_Rtype (L_return_old_ctype (attr->field[k]));

	      vreg_id = -1 - macro;

	      for (j = 0; j < R_n_jsr_def_macro; j++)
		if (R_jsr_def_macro[j].id == macro &&
		    R_jsr_def_macro[j].type == macro_type)
		  {
		    vreg_id = R_jsr_def_macro[j].vreg_id;
		    break;
		  }

	      L_add_src_operand_reg (oper, vreg_id, FALSE, FALSE);
	    }
	}
    }

  /* Handle macroregisters used by the rts */
  if (R_rts && (attr = L_find_attr (R_rts->attr, "tr")))
    {
      int k;
      for (k = 0; k < attr->max_field; k++)
	{
	  macro = attr->field[k]->value.mac;
	  macro_type =
	    R_Ltype_to_Rtype (L_return_old_ctype (attr->field[k]));

	  vreg_id = -1 - macro;

	  for (j = 0; j < R_n_avail_macro; j++)
	    if (R_avail_macro[j].id == macro &&
		R_avail_macro[j].type == macro_type)
	      {
		vreg_id = R_avail_macro[j].vreg_id;
		break;
	      }
	  
	  L_add_src_operand_reg (R_rts, vreg_id, FALSE, FALSE);
	}
    }

  /* 
   * Perform Live Variable Analysis 
   * ----------------------------------------------------------------------
   * Use special critical variable mode for intereference graph construction.
   * An unconditional predicate definition must conflict with all live
   * predicates, not simply those live when its guard is TRUE.
   */

#if 0
  L_dataflow_analysis (CRITICAL_VARIABLE | INTERFERENCE);
#else
  L_dataflow_analysis (LIVE_VARIABLE | INTERFERENCE);
#endif

}


/*===========================================================================
 *
 *      Func :  R_print_live_ranges()
 *      Desc :  Dumps the live ranges to a file
 *      Input:  none 
 *      Output: none
 *
 *      Side Effects: none
 *
 *===========================================================================*/
void
R_print_live_ranges (FILE * file)
{
  R_Reg *vreg;
  R_Arc *arc;

  fprintf (file, "\n*** Function Live Ranges ***\n\n");
  for (vreg = R_vreg; vreg != NULL; vreg = vreg->nextReg)
    {
      fprintf (file,
	       "Live Range #%d : class %d, type %d, rot %d, nth rot %d, "
	       "%s\n\tLive insts: ",
	       vreg->index, vreg->rclass, vreg->type, vreg->rotating,
	       vreg->nth_rot_reg,
	       ((vreg->flags & R_CONSTANT) ? "(Constant)" : ""));
      Set_print (file, "", vreg->live_range);
      fprintf (file, "\tInterfering vregs: ");
      arc = vreg->interfere;
      while (arc)
	{
	  fprintf (file, "%d ", arc->lr->index);
	  arc = arc->next;
	}
      fprintf (file, "\n");
    }
}


/*===========================================================================
 *
 *      Func :  R_compute_live_ranges()
 *      Desc :  Determines the live ranges of each virtual register by
 *		examining the computed live variable information 
 *      Input:  none 
 *      Output: none
 *
 *      Side Effects: defines <live_range> and <instr_weight> fields of
 *		      each defined R_Reg structure 
 *
 *===========================================================================*/
void
R_compute_live_ranges (L_Func * fn, L_Region * region)
{
  int i, j, k, num, num_operand;
  int *live_in = R_buf;
  int n_oper, *oper_array = R_buf2;
  Set in, out;
  Set delete_oper = NULL;
  int *delete_oper_array = R_buf;
  int *def_oper;
  /* MDES flags of opers that are unsafe to delete */
  int unsafe_to_delete = OP_FLAG_CBR | OP_FLAG_JMP | OP_FLAG_RTS |
    OP_FLAG_JSR | OP_FLAG_SYNC | OP_FLAG_CHK |
    OP_FLAG_STORE | OP_FLAG_IGNORE | OP_FLAG_MEMCOPY;

  R_Reg *vreg;

  n_oper = Set_2array (R_oper_set, oper_array);

  def_oper = (int *) malloc (sizeof (int) * n_oper);

  for (i = 0; i < n_oper; i++)
    {
      int live_dest = 0, mac_dest = 0, reg_dest = 0;
      int oper_id = oper_array[i];
      L_Oper *oper = L_oper_hash_tbl_find_oper (fn->oper_hash_tbl, oper_id);
      L_Cb *cb = L_oper_hash_tbl_find_cb (fn->oper_hash_tbl, oper_id);

      /*
       * Perform global dead code removal to avoid live range
       * and interference graph problems caused by unused
       * virtual register definitions
       */

      out = (Set) L_get_oper_OUT_set (cb, oper, BOTH_PATHS);

      num_operand = L_max_dest_operand;
      for (j = 0; j < num_operand; j++)
	{
	  L_Operand *dest;
	  if (!(dest = oper->dest[j]))
	    continue;

	  vreg = VREG (dest->value.r);

	  /*
	   * REH 3/13/95 Correction to generate proper 
	   * macro lifetime in the presence of dead code.
	   */

	  if (L_is_reg (dest))
	    {
	      if (vreg->rclass == R_MACRO_CALLER ||
		  vreg->rclass == R_MACRO_CALLEE)
		{
		  mac_dest++;
		}
	      else
		{
		  reg_dest = 1;

		  if (Set_in (out, dest->value.r))
		    {
		      live_dest++;
		    }
		  else if (L_general_pred_comparison_opcode (oper) &&
			   L_is_ctype_predicate (dest))
		    {
		      if (R_Print_Dead_Code_Results)
			{
			  fprintf (stderr, "RA eliding pred dest %d\n", j);
			  L_print_oper (stdout, oper);
			}
		      L_delete_operand (dest);
		      oper->dest[j] = NULL;
		    }
		}
	    }
	  else if (L_is_macro (dest) &&
		   (!L_is_ctype_predicate (dest) ||
		    M_dataflow_macro (dest->value.mac)))
	    {
	      mac_dest++;
	    }
	}

      if (reg_dest && !live_dest)
	{
	  /* If none of the virtual register destinations were live out,
	   * the oper may be safe to delete.  If it is not safe to
	   * delete, add the oper to the dest vreg live ranges to
	   * guarantee a conflict-free allocation.
	   */

	  if (R_Allow_Dead_Code_Elimination && !mac_dest &&
	      !op_flag_set (oper->proc_opc, unsafe_to_delete))
	    {
	      if (R_Print_Dead_Code_Results)
		{
		  fprintf (stdout, "RA: eliding dead code oper\n");
		  L_print_oper (stdout, oper);
		}
	      /* Remember oper for deleting oper later */
	      delete_oper = Set_add (delete_oper, oper->id);
	    }
	  else
	    {
	      for (k = 0; k < num_operand; k++)
		{
		  L_Operand *dest;
		  if (!(dest = oper->dest[k]))
		    continue;
		  
		  if (L_is_reg (dest))
		    {
		      R_Reg *tmp_vreg = VREG (dest->value.r);
		      tmp_vreg->live_range =
			Set_add (tmp_vreg->live_range, oper->id);
		    }
		}
	    }
	}

      /* end of global dead code removal */

      in = L_get_oper_IN_set (oper);
      num = Set_2array (in, live_in);

      for (k = 0; k < num; k++)
	{
	  if (live_in[k] < 0)
	    continue;
	  
	  vreg = VREG (live_in[k]);
	  vreg->live_range = (Set) Set_add (vreg->live_range, oper->id);
	  vreg->instr_weight += 1;
	}
    }

  free (def_oper);

  /* Now, delete the opers selected as dead code */

  if (R_Allow_Dead_Code_Elimination)
    {
      n_oper = Set_2array (delete_oper, delete_oper_array);
      for (i = 0; i < n_oper; i++)
	{
	  int oper_id = delete_oper_array[i];
	  L_Oper *oper = L_oper_hash_tbl_find_oper (fn->oper_hash_tbl, 
						    oper_id);
	  L_Cb *cb = L_oper_hash_tbl_find_cb (fn->oper_hash_tbl, oper_id);

	  /* 20020829 SZU
	   * Nullify for TAHOE arch only
	   */
	  if ((!L_EXTRACT_BIT_VAL (cb->flags, L_CB_SOFTPIPE) &&
	       !L_find_attr (cb->attr, "kernel")) || (M_arch != M_TAHOE))
	    L_delete_oper (cb, oper);
	  else
	    L_nullify_operation (oper);
	}
      Set_dispose (delete_oper);
    }

  if (R_Print_Live_Ranges)
    R_print_live_ranges (stdout);
}

void
R_print_interference_graph (void)
{
  int count, num_int, type1, type2;
  R_Arc *arc;
  R_Reg *vreg;

  fprintf (stderr, "*\n* Virtual Register Interference Graph\n*\n");

  for (vreg = R_vreg; vreg != NULL; vreg = vreg->nextReg)
    {
      type1 = R_smallest_overlapping_bank (vreg->type, R_CALLER);

      num_int = 0;
      for (arc = vreg->interfere; arc != NULL; arc = arc->next)
	{
	  type2 = R_smallest_overlapping_bank (arc->lr->type, R_CALLER);
	  if (type1 == type2)
	    num_int += 1;
	}

      fprintf (stderr, "VReg #%d (%s), %d interferences:\n",
	       vreg->index, BANK_NAME (vreg->type), num_int);

      count = 0;
      fprintf (stderr, "\t\t");
      for (arc = vreg->interfere; arc != NULL; arc = arc->next)
	{
	  type2 = R_smallest_overlapping_bank (arc->lr->type, R_CALLER);
	  if (type1 == type2)
	    {
	      fprintf (stderr, "%d ", arc->lr->index);
	      if (count++ == 9)
		{
		  fprintf (stderr, "\n\t\t");
		  count = 0;
		}
	    }
	}
      fprintf (stderr, "\n\n");
    }
}

void
R_add_interference_arc (R_Reg * from, R_Reg * to)
{
  R_Arc *new_arc;

  if (!from || !to)
    L_punt ("R_add_interference_arc: NULL R_Reg");

  new_arc = (R_Arc *) L_alloc (R_alloc_interference_arc);
  new_arc->lr = from;
  new_arc->next = to->interfere;
  to->interfere = new_arc;
}


/*===========================================================================
 *
 *      Func :  R_construct_interference_graph()
 *      Desc :  Creates an interfernce graph node for each virtual register
 *		with weight <ref_weight>^3/<instr_weight>.  An arc is 
 *		created between two nodes if it is determined that there
 *		live ranges overlap(intersect). 
 *      Input:  none 
 *      Output: 
 *
 *      Side Effects: adds interference arcs to <interfere> field of the
 *		      R_Reg structure
 *
 *===========================================================================*/
void
R_construct_interference_graph (L_Func *fn, L_Region * region)
{
  R_Reg *r1, *r2;
  int use_invariant_priorities = R_Invariant_Vreg_Priorities;
  int *buf = NULL, bufsz = 0, sz, j;

  /*
   * Construct interference arcs
   *
   * This is done by comparing each pair of virtual registers to
   * determine if there is an intersection between their respective 
   * live ranges, if so two interference arcs are created.
   */

  for (r1 = R_vreg; r1; r1 = r1->nextReg)
    {
      Set intf = NULL;

      /* Calculate virtual register priority */
      if (!use_invariant_priorities)
	{
	  if (r1->instr_weight != 0)
	    r1->priority =
	      (r1->ref_weight * r1->ref_weight * r1->ref_weight) /
	      (double) r1->instr_weight;
	  else
	    r1->priority = 0.0;
	}
      else
	{
	  r1->priority = (double) r1->index;
	}

      sz = Set_size (r1->def_instr);

      if (sz > bufsz)
	{
	  bufsz = sz < 16 ? 32 : (2 * sz);
	  if (buf)
	    free (buf);
	  buf = Lcode_malloc (bufsz * sizeof (int));
	}

      /* r1 interferes with r2 iff a definition of r1 occurs
       * at a point where r2 is live, or a definition of r2 occurs
       * where r1 is live.
       */

      Set_2array (r1->def_instr, buf);

      for (j = 0; j < sz; j++)
	{
	  L_Oper *oper;
	  L_Cb *cb;

	  if (!(oper = L_oper_hash_tbl_find_oper (fn->oper_hash_tbl, buf[j])))
	    continue;

	  cb = L_oper_hash_tbl_find_cb (fn->oper_hash_tbl, buf[j]);

	  intf = Set_union_acc (intf, 
				L_get_oper_OUT_set (cb, oper, BOTH_PATHS));
	}

      /* Add precomputed interferences */

      intf = Set_union_acc (intf, r1->intf_vreg);

      /* Draw arcs */

      for (r2 = R_vreg; r2; r2 = r2->nextReg)
	{
	  if (r1 == r2)
	    continue;

	  if (Set_in (intf, r2->index) && !R_find_interference (r1, r2))
	    {
	      R_add_interference_arc (r2, r1);
	      R_add_interference_arc (r1, r2);
	    }
	}

      intf = Set_dispose (intf);
    }

  if (buf)
    free (buf);

  if (R_Print_Interference_Graph)
    {
      DB_spit_func (fn, "INTFG");
      R_print_interference_graph ();
    }

  return;
}

/*===========================================================================
 *
 *      Func :  R_remove_interference()
 *      Desc :  Removes the interference arcs between two live ranges in
 *              the interference graph.
 *      Input:  vreg1, vreg2 - two no longer interfering live ranges 
 *      Output: 
 *
 *      Side Effects: none
 *
 *===========================================================================*/
void
R_remove_interference (R_Reg * vreg1, R_Reg * vreg2)
{
  R_Arc *arc, *prev_arc;

  for (arc = prev_arc = vreg1->interfere; arc != NULL; arc = arc->next)
    {
      if (arc->lr != vreg2)
	{
	  prev_arc = arc;
	  continue;
	}

      if (arc != prev_arc)
	prev_arc->next = arc->next;
      else			/* prev_arc = arc */
	vreg1->interfere = arc->next;

      L_free (R_alloc_interference_arc, arc);
      break;
    }
}

R_Arc *
R_find_interference (R_Reg * vreg1, R_Reg * vreg2)
{
  R_Arc *arc;

  for (arc = vreg1->interfere; arc != NULL; arc = arc->next)
    {
      if (arc->lr == vreg2)
	return (arc);
    }
  return (NULL);
}

/*===========================================================================
 *
 *      Func :  R_prune_interference_graph()
 *      Desc :  Removes live ranges from the interference graph whose number
 *              of interferences is less than the number of available registers
 *              of that type.  A live range is removed only of both caller and
 *              callee saved benefits are > 0.0.
 *      Input:  
 *      Output: 
 *
 *      Side Effects: reduction of the interference graph
 *
 *===========================================================================*/
Stack *unconstrained_vreg = NULL;

static void
R_prune_interference_graph (void)
{
  int cnt = 0, change;
  R_Reg *vreg;
  R_Arc *arc;
  int num;
  /*
     unconstrained_vreg = New_Stack();
   */
#if 0
  fprintf (stdout, "Pruning interference graph.\n");
#endif
  for (vreg = R_vreg; vreg; vreg = vreg->nextReg)
    vreg->flags = vreg->flags & (~R_UNCONSTRAINED);

  while (((long int) (vreg = (R_Reg *) Pop (unconstrained_vreg))) != -1);

  change = 1;
  while (change)
    {
      change = 0;
      for (vreg = R_vreg; vreg != NULL; vreg = vreg->nextReg)
	{

	  if (vreg->flags & R_UNCONSTRAINED)
	    continue;

	  /* my HP-PA hack */
	  if (vreg->flags & R_PA_HACK)
	    continue;

	  if (vreg->rclass == R_MACRO_CALLER ||
	      vreg->rclass == R_MACRO_CALLEE)
	    continue;

	  if (vreg->caller_benefit < 0 || vreg->callee_benefit < 0)
	    continue;

	  cnt = 0;

	  /* Account for the ill_reg's a vreg cannot be allocated to */
	  cnt += Set_size (vreg->illegal_reg);

	  for (arc = vreg->interfere; arc != NULL; arc = arc->next)
	    {
	      if (arc->lr->flags & R_UNCONSTRAINED)
		continue;

	      if (R_global_overlap[vreg->type] &&
		  R_global_overlap[arc->lr->type])
		cnt += R_max_overlap_size[vreg->type];
	    }
	  num = R_bank[vreg->type + R_CALLER].num_reg *
	    R_bank[vreg->type + R_CALLER].reg_size +
	    R_bank[vreg->type + R_CALLEE].num_reg *
	    R_bank[vreg->type + R_CALLEE].reg_size +
	    R_bank[vreg->type + R_MACRO_CALLER].num_reg *
	    R_bank[vreg->type + R_MACRO_CALLER].reg_size +
	    R_bank[vreg->type + R_MACRO_CALLEE].num_reg *
	    R_bank[vreg->type + R_MACRO_CALLEE].reg_size;

	  if (cnt < num)
	    {
	      change = 1;
	      vreg->flags |= R_UNCONSTRAINED;

	      Push_Top (unconstrained_vreg, vreg);
	    }
	}
    }
}

/*===========================================================================
 *
 *      Func :  R_allocate_unconstrained_virtual_registers()
 *      Desc :  Assigns physical registers to unconstrained virtual registers
 *              in the opposite order in which they were removed from the 
 *              interference graph.  If there is no available error, then a
 *              small, but major internal error has occurred.
 *      Input:  
 *      Output: 
 *
 *      Side Effects:  
 *
 *===========================================================================*/
static int
R_allocate_unconstrained_virtual_registers (void)
{
  int free_register, *neighbor;
  R_Reg *vreg;
  Set reserved_resource;
  Set constrained_resource;
  Set tmp_reserved;
  int *constrained_array;

  neighbor = (int *) malloc (R_n_vreg * sizeof (int));
  constrained_array = (int *) malloc (sizeof (int) * R_n_vreg);

  while (((long int) (vreg = (R_Reg *) Pop (unconstrained_vreg))) != -1)
    {
      reserved_resource = NULL;
      constrained_resource = NULL;
      R_determine_reserved_registers (&reserved_resource,
				      &constrained_resource,
				      constrained_array, vreg);

      tmp_reserved = Set_union (reserved_resource, constrained_resource);
      free_register = R_find_free_register (vreg, tmp_reserved, 1);

      if (free_register != -1)
	{
	  /* live range is allocatable */
	  vreg->base_index = free_register;
	  vreg->size = (R_bank + vreg->type + vreg->rclass)->reg_size;
	}
      else
	L_punt ("RA: No free register for unconstrained vreg!!!!\n");

      reserved_resource = Set_dispose (reserved_resource);
    }

  /*
     Free_Stack(unconstrained_vreg);
   */

  free (neighbor);
  free (constrained_array);

  return (0);
}

/*===========================================================================
 *
 *      Func :  R_allocate_virtual_registers()
 *      Desc :  The heart of the register allocator.  Performs the allocation
 *              for all constrained virtual regs, using a hybrid combination
 *              of Chaitin's allocator and Chow's allocator. If you want to
 *              know how it works, ask Rick.
 *      Input:  
 *      Output: 
 *
 *      Side Effects:  
 *
 *===========================================================================*/
void
R_allocate_virtual_registers (L_Func * fn)
{
  int uncolored, pass;
  int free_register;
  void *tmp;
  Heap *heap;
  Stack *shadow_stack;
  R_Reg *vreg;
  Set reserved_resource;
  Set constrained_resource;
  Set tmp_reserved;
  int *constrained_array;

  constrained_array = (int *) malloc (sizeof (int) * R_n_vreg);

  heap = Heap_Create (HEAP_MAX);
  shadow_stack = New_Stack ();
  unconstrained_vreg = New_Stack ();

  /* Scan and allocate the rotating registers. */

  for (vreg = R_vreg; vreg != NULL; vreg = vreg->nextReg)
    {
      if (vreg->rotating != 1)
	continue;

      /* Allocate these rotating registers outright. */

      reserved_resource = NULL;
      constrained_resource = NULL;
      R_determine_reserved_registers (&reserved_resource,
				      &constrained_resource,
				      constrained_array, vreg);

      tmp_reserved = Set_union (reserved_resource, constrained_resource);
      free_register = R_find_rotating_register (vreg, tmp_reserved, 1);

      if (R_Print_Coloring_Stats)
	{
	  fprintf (stderr, "Coloring(R,C) %d\t-> %d\n", vreg->index,
		   free_register);
	  fprintf (stderr, "\tWgt %f\n", vreg->priority);
	  Set_print (stderr, "\tRR", reserved_resource);
	  Set_print (stderr, "\tCR", constrained_resource);
	}

      Set_dispose (tmp_reserved);

      reserved_resource = Set_dispose (reserved_resource);
      constrained_resource = Set_dispose (constrained_resource);

      if (free_register != -1)
	{
	  /* live range is allocatable */
	  vreg->base_index = free_register;
	  vreg->size = (R_bank + vreg->type + vreg->rclass)->reg_size;
	}
      else
	{
	  L_punt ("R_allocate_virtual_registers: "
		  "Unable to color a rotating register.");
	}
    }

  /* Scan and color remaining registers */

  for (vreg = R_vreg; vreg != NULL; vreg = vreg->nextReg)
    {
      if (vreg->rotating == 1)
	continue;

      if (vreg->flags & R_PREALLOCATED_MACRO)
	continue;

      if (vreg->flags & R_PREALLOCATED_FLYBY &&
	  !(vreg->flags & R_REGION_CONSTRAINED))
	continue;

#if 0
      if ( vreg->flags & R_UNCONSTRAINED )
	continue;
#endif

      Heap_Insert (heap, vreg, vreg->priority);
    }

  if (R_Print_Coloring_Stats)
    fprintf (stderr, "*** GRAPH COLORING STATISTICS ***\n\n");

  uncolored = 1;
  pass = 1;
  while (uncolored)
    {
      if (R_Print_Coloring_Stats)
	fprintf (stderr, "Coloring Pass %d\n", pass);

      if (R_Prune_Interference_Graph)
	R_prune_interference_graph ();

      uncolored = 0;

      tmp = Heap_Top (heap);
      if (tmp)
	Push_Top (shadow_stack, tmp);

      vreg = (R_Reg *) Heap_ExtractTop (heap);

      while (vreg != NULL)
	{
	  if (vreg->flags & R_UNCONSTRAINED)
	    {
	      tmp = Heap_Top (heap);
	      if (tmp)
		Push_Top (shadow_stack, tmp);

	      vreg = (R_Reg *) Heap_ExtractTop (heap);
	      continue;
	    }

	  reserved_resource = NULL;
	  constrained_resource = NULL;
	  R_determine_reserved_registers (&reserved_resource,
					  &constrained_resource,
					  constrained_array, vreg);

	  tmp_reserved = Set_union (reserved_resource, constrained_resource);
	  free_register = R_find_free_register (vreg, tmp_reserved, 1);

	  if (R_Print_Coloring_Stats)
	    {
	      fprintf (stderr, "Coloring(R,C) %d\t-> %d\n", vreg->index,
		       free_register);
	      fprintf (stderr, "\tWgt %f\n", vreg->priority);
	      Set_print (stderr, "\tRR", reserved_resource);
	      Set_print (stderr, "\tCR", constrained_resource);
	    }

	  Set_dispose (tmp_reserved);

	  reserved_resource = Set_dispose (reserved_resource);
	  constrained_resource = Set_dispose (constrained_resource);

	  if (free_register != -1)
	    {
	      /* live range is allocatable */
	      vreg->base_index = free_register;
	      vreg->size = (R_bank + vreg->type + vreg->rclass)->reg_size;

	      /* additional code is required to allocate the constant
                 live range */
	    }
	  else
	    {
	      uncolored += 1;

	      if (R_Print_Coloring_Stats)
		fprintf (stderr, "Virtual register uncolored: %d\n",
			 vreg->index);

	      R_handle_unallocatable_vreg (fn, vreg, shadow_stack);
	    }

	  tmp = Heap_Top (heap);
	  if (tmp)
	    Push_Top (shadow_stack, tmp);

	  vreg = (R_Reg *) Heap_ExtractTop (heap);
	}

      if (uncolored)
	{
	  R_Reg *tmp_vreg;
	  while (((long int) (tmp_vreg = (R_Reg *) Pop (shadow_stack))) != -1)
	    {
	      tmp_vreg->base_index = -1;

	      if (tmp_vreg->caller_benefit >= tmp_vreg->callee_benefit)
		tmp_vreg->rclass = R_CALLER;
	      else
		tmp_vreg->rclass = R_CALLEE;

	      if (!((tmp_vreg->flags & R_PREALLOCATED_FLYBY) &&
		    (tmp_vreg->flags & R_SPILLED)))
		Heap_Insert (heap, tmp_vreg, tmp_vreg->priority);

	    }
	  if (R_Print_Coloring_Stats)
	    fprintf (stderr, "\tUncolored registers: %d\n", uncolored);

	  pass += 1;

	  /* reset the register bank stacks */
	  R_reset_register_stacks ();
	}
    }
  R_allocate_unconstrained_virtual_registers ();

  Heap_Dispose (heap, NULL);
  Free_Stack (shadow_stack);
  Free_Stack (unconstrained_vreg);
  free (constrained_array);
}

/*===========================================================================
 *
 *      Func :  R_spill_all_virtual_registers()
 *      Desc :  Assigns a physical register to all virtual registers. Since
 *              their live ranges consist only of defs and uses, they are
 *              all being spilled.
 *      Input:  
 *      Output: 
 *
 *
 *===========================================================================*/
static void
R_spill_all_virtual_registers (L_Func * fn, L_Region * region)
{
  int i, l, free_register, *phys_map;
  L_Cb *cb;
  L_Oper *oper, *spill_instr, *next_op;
  R_Physical_Bank *bank;
  R_Reg *vreg;
  Set reserved_resource, src_vregs;
  Set pred_vregs;
  Set pred_resource;
  L_Attr *attr;

  pred_vregs = NULL;
  for (cb = fn->first_cb; cb != NULL; cb = cb->next_cb)
    {
      pred_resource = NULL;
      pred_vregs = NULL;

      for (oper = cb->first_op; oper != NULL; oper = next_op)
	{
	  next_op = oper->next_op;
	  /*
	   * All other registers are live only during an operation
	   */

	  reserved_resource = Set_union (0, pred_resource);

	  src_vregs = NULL;

	  /* guard predicate */

	  {
	    L_Operand *pred;
	    if ((pred = oper->pred[0]) &&
		L_is_reg (pred))
	      {
		vreg = VREG (pred->value.r);
		if (Set_in (pred_vregs, vreg->index))
		  {
		    free_register = vreg->base_index;
		  }
		else
		  {
		    vreg->rclass = R_CALLER;
		    free_register =
		      R_find_free_register (vreg, reserved_resource, 1);
		    pred_vregs = Set_add (pred_vregs, vreg->index);
		  }
		
		if (free_register != -1)
		  {
		    vreg->base_index = free_register;
		    vreg->size =
		      (R_bank + vreg->type + vreg->rclass)->reg_size;
		    
		    /*
		     * Place register in the reserved resource set
		     */
		    for (l = 0; l < vreg->size; l++)
		      {
			reserved_resource =
			  (Set) Set_add (reserved_resource,
					 vreg->base_index + l);
			pred_resource =
			  Set_add (pred_resource, vreg->base_index + l);
		      }
		    /*
		     * Assign a spill location if necessary 
		     */
		    if (vreg->spill_loc == -1)
		      vreg->spill_loc = R_spill_loc (vreg->type);
		    
		    /*
		     * Determine the physical register assigned
		     */
		    bank = R_bank + vreg->type + vreg->rclass;
		    
		    if (!bank->defined)
		      L_punt ("Register allocated to non-existant "
			      "register bank!");
		    
		    phys_map = R_map[vreg->type + vreg->rclass];
		    vreg->phys_reg =
		      phys_map[vreg->base_index - bank->base_index];
		    *bank->used_reg =
		      Set_add (*bank->used_reg, vreg->phys_reg);

		    /* Insert the fill instructions prior to the
		     * current oper */
		    spill_instr = 
		      (L_Oper *) O_fill_reg (vreg->phys_reg,
					     R_conv_rclass_to_Lclass
					     (vreg->rclass),
					     pred,
					     vreg->spill_loc,
					     NULL,
					     R_SPILL_CODE);
		      
		    R_insert_spill_code_before (cb, oper, spill_instr);

		    /*
		     * Replace the virtual register with a physical register
		     */
		    pred->value.r = vreg->phys_reg;
		    
		    /* save virtual predicate of jsr */
		    if (L_general_subroutine_call_opcode (oper))
		      {
			L_Attr *new_attr = L_new_attr ("vpred", 1);
			L_set_int_attr_field (new_attr, 0, pred->value.r);
			oper->attr = L_concat_attr (oper->attr, new_attr);
		      }
		  }
		else
		  {
		    L_punt ("RA: No free register for spilling all vreg!\n");
		  }
	      }
	  }

	  /* src operands */

	  for (i = 0; i < L_max_src_operand; i++)
	    {
	      L_Operand *src;
	      if ((src = oper->src[i]) &&
		  L_is_reg (src))
		{
		  src_vregs = Set_add (src_vregs, src->value.r);

		  vreg = VREG (src->value.r);
		  /* Probing of predicates results in move predicate
                     to register operation */
		  if (Set_in (pred_vregs, vreg->index))
		    {
		      free_register = vreg->base_index;
		    }
		  else
		    {
		      vreg->rclass = R_CALLER;
		      free_register =
			R_find_free_register (vreg, reserved_resource, 1);
		      if (L_is_ctype_predicate (src))
			pred_vregs = Set_add (pred_vregs, src->value.r);
		    }

		  if (free_register != -1)
		    {
		      vreg->base_index = free_register;
		      vreg->size =
			(R_bank + vreg->type + vreg->rclass)->reg_size;

		      /*
		       * Place register in the reserved resource set
		       */
		      for (l = 0; l < vreg->size; l++)
			{
			  reserved_resource =
			    (Set) Set_add (reserved_resource,
					   vreg->base_index + l);
			}
		      /*
		       * Assign a spill location if necessary 
		       */
		      if (vreg->spill_loc == -1)
			vreg->spill_loc = R_spill_loc (vreg->type);

		      /*
		       * Determine the physical register assigned
		       */
		      bank = R_bank + vreg->type + vreg->rclass;

		      if (!bank->defined)
			L_punt ("Register allocated to non-existant "
				"register bank!");

		      phys_map = R_map[vreg->type + vreg->rclass];
		      vreg->phys_reg =
			phys_map[vreg->base_index - bank->base_index];
		      *bank->used_reg =
			Set_add (*bank->used_reg, vreg->phys_reg);

		      /*
		       * Insert the fill instructions prior to the current oper
		       */
		      spill_instr = 
			(L_Oper *) O_fill_reg (vreg->phys_reg,
					       R_conv_rclass_to_Lclass
					       (vreg->rclass),
					       src,
					       vreg->spill_loc,
					       oper->pred,
					       R_SPILL_CODE);
		      
		      R_insert_spill_code_before (cb, oper, spill_instr);

		      /*
		       * Replace the virtual register with a physical register
		       */
		      src->value.r = vreg->phys_reg;
		    }
		  else
		    L_punt("RA: No free register for spilling all vreg!\n");
		}
	    }

	  /* EMN: Extended source support */

	  if ((attr = L_find_attr (oper->attr, "src")) &&
	      oper->opc == Lop_CHECK)
	    {
	      for (i = 0; i < attr->max_field; i++)
		{
		  L_Operand *src;
		  if (!(src = attr->field[i]))
		    continue;
		  if (L_is_reg (src))
		    {
		      src_vregs = Set_add (src_vregs, src->value.r);

		      vreg = VREG (src->value.r);
		      /* Probing of predicates results in move
                         predicate to register operation */
		      if (Set_in (pred_vregs, vreg->index))
			{
			  free_register = vreg->base_index;
			}
		      else
			{
			  vreg->rclass = R_CALLER;
			  free_register =
			    R_find_free_register (vreg, reserved_resource, 1);
			  if (L_is_ctype_predicate (src))
			    pred_vregs = Set_add (pred_vregs, src->value.r);
			}

		      if (free_register != -1)
			{
			  vreg->base_index = free_register;
			  vreg->size =
			    (R_bank + vreg->type + vreg->rclass)->reg_size;

			  /*
			   * Place register in the reserved resource set
			   */
			  for (l = 0; l < vreg->size; l++)
			    {
			      reserved_resource =
				(Set) Set_add (reserved_resource,
					       vreg->base_index + l);
			    }
			  /*
			   * Assign a spill location if necessary 
			   */
			  if (vreg->spill_loc == -1)
			    vreg->spill_loc = R_spill_loc (vreg->type);

			  /*
			   * Determine the physical register assigned
			   */
			  bank = R_bank + vreg->type + vreg->rclass;

			  if (!bank->defined)
			    L_punt ("Register allocated to "
				    "non-existant register bank!");

			  phys_map = R_map[vreg->type + vreg->rclass];
			  vreg->phys_reg =
			    phys_map[vreg->base_index - bank->base_index];
			  *bank->used_reg =
			    Set_add (*bank->used_reg, vreg->phys_reg);

			  /* Insert the fill instructions prior to the
			   * current oper */
			  spill_instr = 
			    (L_Oper *) O_fill_reg (vreg->phys_reg,
						   R_conv_rclass_to_Lclass
						   (vreg->rclass),
						   src,
						   vreg->
						   spill_loc,
						   oper->pred,
						   R_SPILL_CODE);
			  
			  R_insert_spill_code_before (cb, oper, spill_instr);

			  /* Replace the virtual register with a
			   * physical register */
			  src->value.r = vreg->phys_reg;
			}
		      else
			L_punt("Register Allocation: "
			       "No free register for spilling all vreg!!!!\n");
		    }
		}
	    }

	  /* dest operands */

	  for (i = 0; i < L_max_dest_operand; i++)
	    {
	      L_Operand *dest;
	      if ((dest = oper->dest[i]) &&
		  L_is_reg (dest))
		{
		  vreg = VREG (dest->value.r);

		  if (Set_in (src_vregs, dest->value.r) ||
		      Set_in (pred_vregs, dest->value.r))
		    {
		      free_register = vreg->base_index;
		    }
		  else
		    {
		      vreg->rclass = R_CALLER;
		      free_register =
			R_find_free_register (vreg, reserved_resource, 1);
		    }

		  if (L_is_ctype_predicate (dest))
		    {
		      pred_vregs = Set_add (pred_vregs, dest->value.r);
		    }

		  if (free_register != -1)
		    {
		      vreg->base_index = free_register;
		      vreg->size =
			(R_bank + vreg->type + vreg->rclass)->reg_size;

		      /*
		       * Place register in the reserved resource set
		       */
		      for (l = 0; l < vreg->size; l++)
			{
			  reserved_resource =
			    Set_add (reserved_resource, vreg->base_index + l);
			  if (L_is_ctype_predicate (dest))
			    pred_resource =
			      Set_add (pred_resource, vreg->base_index + l);
			}
		      /*
		       * Assign a spill location if necessary 
		       */
		      if (vreg->spill_loc == -1)
			vreg->spill_loc = R_spill_loc (vreg->type);

		      /*
		       * Determine the physical register assigned
		       */
		      bank = R_bank + vreg->type + vreg->rclass;

		      if (!bank->defined)
			L_punt ("Register allocated to non-existant "
				"register bank!");

		      phys_map = R_map[vreg->type + vreg->rclass];
		      vreg->phys_reg =
			phys_map[vreg->base_index - bank->base_index];
		      *bank->used_reg =
			Set_add (*bank->used_reg, vreg->phys_reg);

		      /*
		       * Insert the spill instructions after to the current oper
		       */
		      spill_instr = 
			(L_Oper *) O_spill_reg (vreg->phys_reg,
						R_conv_rclass_to_Lclass
						(vreg->rclass),
						dest,
						vreg->spill_loc,
						oper->pred,
						R_SPILL_CODE);
		      
		      R_insert_spill_code_after (cb, oper, spill_instr);

		      /* When spilling all, need to fill some
                         predicate destination before define */
		      if (L_is_update_predicate_ptype_operand (dest))
			{
			  spill_instr = 
			    (L_Oper *) O_fill_reg (vreg->phys_reg,
						   R_conv_rclass_to_Lclass
						   (vreg->rclass),
						   dest,
						   vreg->
						   spill_loc,
						   oper->pred,
						   R_SPILL_CODE);
			  
			  R_insert_spill_code_before (cb, oper, spill_instr);
			}

		      /* Replace the virtual register with the
		       * physical register */
		      dest->value.r = vreg->phys_reg;
		    }
		  else
		    L_punt ("RA: No free register for spilling all vreg!\n");
		}
	    }

	  Set_dispose (src_vregs);
	  Set_dispose (reserved_resource);
	}

      Set_dispose (pred_resource);
      Set_dispose (pred_vregs);
    }
}

static void
R_regalloc_memory_init (L_Func * fn)
{
  int i, tmp;

  /* initialize spill stack counter */
  pred_spill_stack = 0;
  int_spill_stack = 0;
  fp_spill_stack = 0;

  R_oper_set = NULL;

  /* initialize original function oper count */
  R_n_oper = fn->max_oper_id;

  R_buf =
    (int *) malloc (sizeof (int) *
		    (R_MAX (fn->max_reg_id, fn->max_oper_id) + 100));
  R_buf2 =
    (int *) malloc (sizeof (int) *
		    (R_MAX (fn->max_reg_id, fn->max_oper_id) + 100));

  R_pbuf =
    (void **) malloc (sizeof (void *) *
		      (R_MAX (fn->max_reg_id, fn->max_oper_id) + 100));
  R_pbuf2 =
    (void **) malloc (sizeof (void *) *
		      (R_MAX (fn->max_reg_id, fn->max_oper_id) + 100));

  if (R_alloc_virtual_register == NULL)
    {
      R_alloc_virtual_register =
	L_create_alloc_pool ("R_Reg", sizeof (R_Reg), 256);
      R_alloc_interference_arc =
	L_create_alloc_pool ("R_Arc", sizeof (R_Arc), 512);
    }

  /* Determine Hash Table Size/Mask */
  tmp = fn->max_reg_id + 100;
  for (i = 31; i >= 0; i--)
    if (tmp >> i != 0)
      break;

  R_vregHashTbl = HashTable_create (1 << (i + 1));
  R_regionAllocMap = HashTable_create (1 << (i + 1));
  flowHashTbl = HashTable_create (R_n_oper);
}

static void
R_regalloc_memory_cleanup (void)
{
  R_Reg *vreg, *nextReg;

  /*
   * Tear down the interference graph and free up virtual register structures
   */
  for (vreg = R_vreg; vreg != NULL; vreg = nextReg)
    {
      R_Arc *arc, *tmp;
      nextReg = vreg->nextReg;

      /*
         * In disposing of these two sets care must be taken,
         * since the <live_range> set may be set to the <ref_instr>
         * set during the allocation process.
         * It would be bad to dispose of the same set twice
       */
      if (vreg->ref_instr != vreg->live_range)
	Set_dispose (vreg->ref_instr);

      Set_dispose (vreg->live_range);
      Set_dispose (vreg->def_instr);
      Set_dispose (vreg->ref_cbs);
      Set_dispose (vreg->intf_vreg);
      
      /* Free the interference arcs for this node */
      for (arc = vreg->interfere; arc != NULL; arc = tmp)
	{
	  tmp = arc->next;
	  L_free (R_alloc_interference_arc, arc);
	}

      /* free virtual register structure */
      L_free (R_alloc_virtual_register, vreg);
    }

  L_free_alloc_pool (R_alloc_interference_arc);
  R_alloc_interference_arc = NULL;
  L_free_alloc_pool (R_alloc_virtual_register);
  R_alloc_virtual_register = NULL;

  R_vreg = NULL;

  HashTable_free (R_vregHashTbl);
  HashTable_free (R_regionAllocMap);
  HashTable_free (flowHashTbl);

  free (R_buf);
  free (R_buf2);

  free (R_pbuf);
  free (R_pbuf2);

  if (R_Macro_Allocation)
    {
      if (R_avail_macro)
	free (R_avail_macro);
      if (R_jsr_def_macro)
	free (R_jsr_def_macro);
    }
  R_jsr_list = List_reset (R_jsr_list);
  R_oper_set = Set_dispose (R_oper_set);
}

/*===========================================================================
 *
 *      Func :  R_register_allocation()
 *      Desc :  The register allocation driver routine.
 *      Input:  L_Func *fn - function to allocate
 *      Output: The function allocated to the target processor, the registers
 *		used are returned via pointers provided with the register
 *		bank definitions and the size of the required swap space.
 *
 *===========================================================================*/

int
R_register_allocation (L_Func * fn, Parm_Macro_List * command_line_macro_list)
{
  int ispace, fspace, pspace;

  R_register_allocation_sep (fn, command_line_macro_list, 
			     &ispace, &fspace, &pspace);
  return (ispace + fspace + pspace);
}

int
R_register_allocation_sep (L_Func * fn,
			   Parm_Macro_List * command_line_macro_list,
			   int *int_swap_space_size, int *fp_swap_space_size,
			   int *pred_swap_space_size)
{
  int pred_jsr_swap_space = 0;
  int int_jsr_swap_space = 0;
  int fp_jsr_swap_space = 0;
  L_Cb *cur_cb;

  L_Region *region;
  L_Region_Member *member;
  L_Time regalloc_time;
  L_Attr *attr;
  void *start, *end;

  double total, spill;
  double total_callee_weight = 0.0;

  total_instruction_weight = 0.0;
  total_spill_weight = 0.0;
  total_region_spill_weight = 0.0;
  total_caller_weight = 0.0;

  L_init_time (&regalloc_time);
  L_start_time (&regalloc_time);

#ifndef __WIN32__
  start = sbrk (0);
#else
/* ADA 5/29/96: There is no easy way to emulate sbrk() on Win95/NT */
  start = 0;
#endif

  if (R_Init)
    {
      R_regalloc_version ();
      /* Renamed 'Regalloc' to 'Lregalloc' -JCG 5/26/98 */
      L_load_parameters_aliased (L_parm_file, command_line_macro_list,
				 "(Lregalloc", "(Regalloc",
				 R_read_parm_regalloc);
      R_Init = 0;
    }

  if (R_Print_Parm_Configuration)
    R_print_parm_configuration ();

  R_analyze_bank_overlap ();

  if (R_Print_Bank_Configuration)
    R_print_bank_configuration ();

  if (!R_Spill_Everything)
    {
      if (!R_Region_Based_Allocation)
	{
	  Set R_rot_reg = R_build_rotating_reg_set (fn);
	  L_rename_disjoint_virtual_registers (fn, R_rot_reg);
	  Set_dispose (R_rot_reg);
	}

      L_setup_dataflow_no_operands (fn);

      if (!R_Utilize_Profile_Info)
	{
	  /* If we are not to use profile information, then compute
	   * oper weights statically by using loop nesting level
	   * information.  */
	  L_dataflow_analysis (DOMINATOR_CB);
	  L_loop_detection (fn, 0);
	  L_compute_static_cb_weight (fn);
	}

      L_compute_oper_weight (fn, !R_Utilize_Profile_Info,
			     R_Utilize_Profile_Info);
    }

  R_regalloc_memory_init (fn);

  /*
   *  Determine available macro registers 
   */
  if (M_arch == M_HPPA)
    R_init_pa_set ();
  
  R_determine_available_macros ();
  
  R_Region = region = R_assemble_region (fn);
  R_unmap_physical_registers (region);

  /* initialize spill stack counter */
  
  R_init_spill_stack (region, &fp_spill_stack, &int_spill_stack, 
		      &pred_spill_stack);

  if (!R_Spill_Everything)
    {
      /* 
       *  0. set up oper, live range structures
       */
      for (cur_cb = fn->first_cb; cur_cb != NULL; cur_cb = cur_cb->next_cb)
	{
	  if (L_EXTRACT_BIT_VAL (cur_cb->flags, L_CB_ENTRANCE_BOUNDARY) &&
	      L_EXTRACT_BIT_VAL (cur_cb->flags, L_CB_PROLOGUE))
	    continue;
	  R_process_cb (cur_cb);
	}

      /* 
       *  1. compute the dataflow information
       */
      R_compute_dataflow_info (fn);

      /*
       *  2. determine flyby live ranges within the region
       */
      R_determine_region_flybys (region);

      R_sort_vreg_list ();

      /*
       *  3. determine allocation constraints
       */
      R_determine_allocation_constraints (region);

      /* 
       *     debugging step for looking at function's live
       *     ranges,interference graph 
       */
      if (R_Print_Virtual_Register_Function)
	DB_spit_func (fn, "Virtual_func");

      /*
       *  4. compute virtual register live ranges 
       */
      R_compute_live_ranges (fn, region);

      /*  
       *  5. generate caller/callee saved weights and bank preferences
       *     for each virtual register 
       */
      R_register_saving_convention_selection (fn, region);

      /*
       *  6. construct virtual register interference graph
       */
      R_construct_interference_graph (fn, region);

      /*
       *  7. perform register allocation
       *     sort all nodes in the interference graph and
       *     allocate from the most important to the least
       *     important
       */
      R_allocate_virtual_registers (fn);
      L_save_virtual_pred_reg_numbers_in_attr (fn);

      /*
       *  8. convert virtual register numbers to machine register numbers
       */
      R_virtual_to_machine_conversion ();

      /*
       *  9. insert necessary spill code
       */
      R_insert_spill_fill_code (region);

      /*
       * 10. insert code to save caller-saved registers  
       */
      R_insert_jsr_spill_fill_code (fn, &int_jsr_swap_space,
				    &fp_jsr_swap_space,
				    &pred_jsr_swap_space);
      pred_spill_stack += pred_jsr_swap_space;
      int_spill_stack += int_jsr_swap_space;
      fp_spill_stack += fp_jsr_swap_space;

      /*
       * 11. reconcile the allocated region 
       */
      R_update_alloc_state (region);
      R_update_region_spill_stack (region);
      R_reconcile_allocated_region (region);
      R_insert_reconciliation_code (fn, region);
    }
  else
    {
      for (member = region->member_cbs;
	   member != NULL; member = member->next_member)
	R_process_cb_for_spilling (fn, member->cb);

      L_save_virtual_pred_reg_numbers_in_attr (fn);
      R_spill_all_virtual_registers (fn, region);
    }

  if (R_Print_Allocation_Stats)
    {
      Set callee_int, callee_dbl;

      callee_int = ((R_bank[R_CALLER + R_INT].used_reg != NULL) ?
		    *(R_bank[R_CALLER + R_INT].used_reg) : NULL);
      callee_dbl = ((R_bank[R_CALLER + R_DOUBLE].used_reg != NULL) ?
		    *(R_bank[R_CALLER + R_DOUBLE].used_reg) : NULL);
      total_callee_weight = (fn->weight * 2.0 *
			     Set_size (callee_int) + Set_size (callee_dbl));

      total = total_instruction_weight + total_spill_weight +
	total_caller_weight + total_callee_weight + total_region_spill_weight;
      spill = total - total_instruction_weight;

      fprintf (stdout, "%f\t%f\t%f\t%f\t%f\n", total,
	       total_spill_weight,
	       total_caller_weight,
	       total_callee_weight, total_region_spill_weight);
    }

  R_regalloc_memory_cleanup ();

  fn->flags = L_SET_BIT_FLAG (fn->flags, L_FUNC_REGISTER_ALLOCATED);

  L_stop_time (&regalloc_time);

#ifndef __WIN32__
  end = sbrk (0);
#else
/* ADA 5/29/96: There is no easy way to emulate sbrk() on Win95/NT */
  end = 0;
#endif

  attr = L_new_attr ("reg_time", 1);
  L_set_double_attr_field (attr, 0, L_final_time (&regalloc_time));
  fn->attr = L_concat_attr (fn->attr, attr);

  attr = L_new_attr ("reg_mem_usage", 1);
  L_set_int_attr_field (attr, 0, (long int) end - (long int) start);
  fn->attr = L_concat_attr (fn->attr, attr);

  *pred_swap_space_size = pred_spill_stack;
  *int_swap_space_size = int_spill_stack;
  *fp_swap_space_size = fp_spill_stack;

  {
    L_Cb *cb;
    L_Oper *op;
    int i;

    for (cb = fn->first_cb; cb; cb = cb->next_cb)
      for (op = cb->first_op; op; op = op->next_op)
	for (i = 0; i < L_max_dest_operand; i++)
	  {
	    if (op->dest[i] && L_is_reg (op->dest[i])
		&& op->dest[i]->value.r > fn->max_reg_id)
	      fn->max_reg_id = op->dest[i]->value.r;
	  }
  }

  return (pred_spill_stack + int_spill_stack + fp_spill_stack);
}
