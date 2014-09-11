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
/*         Author :     Richard Hank, Wen-mei Hwu                            */
#ifndef R_REGPROTO_H
#define R_REGPROTO_H

/* 10/29/02 REK Adding config.h */
#include <config.h>
#include "r_regalloc.h"

#ifdef __cplusplus
extern "C"
{
#endif

  extern char *BANK_NAME (int type);
  extern char *CLASS_NAME (int rclass);
  extern R_Reg *R_add_macro_register (int vreg_id, int mac, int type,
				      int rclass);
  extern R_Reg *R_add_register (int vreg_id, int type, double weight,
				int def);
  extern void R_allocate_virtual_registers (L_Func * fn);
  extern void R_compute_dataflow_info (L_Func * fn);
  extern void R_compute_live_ranges (L_Func * fn, L_Region * region);
  extern void R_construct_interference_graph (L_Func *fn, L_Region * region);
  extern int R_conv_rclass_to_Lclass (int rclass);
  extern int R_conv_type_to_Ltype (int type);
  extern void R_determine_available_macros (void);
  extern void R_determine_reserved_registers (Set * reserved_resource,
					      Set * constrained_resource,
					      int *constrained_array,
					      R_Reg * vreg);
  extern int R_find_free_vreg (void);
  extern R_Arc *R_find_interference (R_Reg * vreg1, R_Reg * vreg2);
  extern Set R_find_reserved_resource (Set reserved_resource, int neighbor[],
				       int num_neighbor);
  extern int R_init_pa_set (void);
  extern int R_Ltype_to_Rtype (int type);
  extern void R_print_interference_graph (void);
  extern void R_print_parm_configuration (void);
  extern void R_process_cb (L_Cb * cb);
  extern void R_process_cb_for_spilling (L_Func * fn, L_Cb * cb);
  extern Set R_process_illegal_register_attr (L_Attr * attr);
  extern int R_register_allocation_sep (L_Func * fn,
					Parm_Macro_List *
					command_line_macro_list,
					int *int_swap_space_size,
					int *fp_swap_space_size,
					int *pred_swap_space_size);
  extern int R_register_allocation (L_Func * fn,
				    Parm_Macro_List *
				    command_line_macro_list);
  extern void R_register_saving_convention_selection (L_Func * fn,
						      L_Region * region);
  extern void R_remove_interference (R_Reg * vreg1, R_Reg * vreg2);
  extern R_Reg *R_reset_vreg_list (R_Reg * vregList);
  extern void R_sort_vreg_list (void);
  extern int R_spill_loc (int type);
  extern void R_analyze_bank_overlap (void);
  extern void R_define_physical_bank_with_rot (int rclass, int type,
					       int num_reg, int reg_size,
					       int overlap, int *reg_array,
					       Set * used, int num_rot_reg,
					       int first_reg_rot,
					       int num_rot_reg_alloc);
  extern void R_define_physical_bank (int rclass, int type, int num_reg,
				      int reg_size, int overlap,
				      int *reg_array, Set * used);
  extern int R_find_free_register (R_Reg * vreg, Set reserved_resource,
				   int final);
  extern int R_find_free_register_in_bank (R_Physical_Bank * reg_bank,
					   Set reserved_resource,
					   R_Reg * vreg);
  extern Set R_build_rotating_reg_set (L_Func *);
  extern void R_get_rot_regs (L_Func *, int *, int *, int *, int *, int *,
			      int *, int *, int *);
  extern int R_get_rot_reg_alloc_multiple (int);
  extern int R_get_rot_reg_max_alloc (int);
  extern void R_print_bank_configuration (void);
  extern void R_reset_register_stacks (void);
  extern int R_smallest_overlapping_bank (int type, int rclass);
  extern void R_virtual_to_machine_conversion (void);
  extern void R_add_load (L_Region * region, L_Cb * fromCb, L_Cb * toCb,
			  R_Reg * vreg, L_Region_Regmap * regmap);
  extern void R_add_store (L_Region * region, L_Cb * fromCb, L_Cb * toCb,
			   R_Reg * vreg, L_Region_Regmap * regmap);
  extern L_Region *R_assemble_region (L_Func * fn);
  extern void R_disassemble_region (L_Func * fn);
  extern void R_determine_allocation_constraints (L_Region * region);
  extern double R_determine_reconcile_cost (L_Region * region, R_Reg * vreg,
					    double *store_cost,
					    double *load_cost);
  extern void R_determine_region_flybys (L_Region * region);
  extern int R_find_base_from_phys_reg (int phys_reg, int type, int rclass);
  extern void R_init_flow_hash_tbl (L_Func * fn);
  extern int R_init_spill_stack (L_Region * region, int *fp_spill_stack,
				 int *int_spill_stack, int *pred_spill_stack);
  extern void R_insert_reconciliation_code (L_Func * fn, L_Region * region);
  extern void R_reconcile_allocated_region (L_Region * region);
  extern void R_unmap_physical_registers (L_Region * region);
  extern void R_update_alloc_state (L_Region * region);
  extern void R_update_region_spill_stack (L_Region * region);
  extern int R_number_of_registers (int type, int caller_save, int macro);
  extern int R_register_overlap (int type1, int type2);
  extern int R_size_of_register (int type);
  extern void R_handle_unallocatable_vreg (L_Func * fn, R_Reg * vreg,
					   Stack * vreg_stack);
  extern int R_insert_jsr_spill_fill_code (L_Func * fn,
					   int *int_jsr_swap_space,
					   int *fp_jsr_swap_space,
					   int *pred_jsr_swap_space);
  extern int R_insert_spill_code_after (L_Cb * cb, L_Oper * oper,
					L_Oper * spill);
  extern int R_insert_spill_code_before (L_Cb * cb, L_Oper * oper,
					 L_Oper * spill);

  extern void R_insert_spill_fill_code (L_Region * region);

  extern void R_regalloc_version (void);

  extern void R_disassemble_regions (L_Func * fn);

  extern void R_print_live_ranges (FILE *);
  extern int R_find_rotating_register (R_Reg *, Set, int);

#ifdef __cplusplus
}
#endif

#endif
