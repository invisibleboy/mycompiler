/* IMPACT Public Release (www.crhc.uiuc.edu/IMPACT)            Version 2.00  */
/* IMPACT Trimaran Release (www.trimaran.org)                  Feb. 22, 1999 */
/*****************************************************************************\
 * LICENSE AGREEMENT NOTICE
 * 
 * IT IS A BREACH OF THIS LICENSE AGREEMENT TO REMOVE THIS NOTICE FROM
 * THE FILE OR SOFTWARE, OR ANY MODIFIED VERSIONS OF THIS FILE OR
 * SOFTWARE OR DERIVATIVE WORKS.
 * 
 * ------------------------------
 * 
 * Copyright Notices/Identification of Licensor(s) of 
 * Original Software in the File
 * 
 * Copyright 1990-1999 The Board of Trustees of the University of Illinois
 * For commercial license rights, contact: Research and Technology
 * Management Office, University of Illinois at Urbana-Champaign; 
 * FAX: 217-244-3716, or email: rtmo@uiuc.edu
 * 
 * All rights reserved by the foregoing, respectively.
 * 
 * ------------------------------
 * 	
 * Copyright Notices/Identification of Subsequent Licensor(s)/Contributors 
 * of Derivative Works
 * 
 * Copyright  <Year> <Owner>
 * <Optional:  For commercial license rights, contact:_____________________>
 * 
 * 
 * All rights reserved by the foregoing, respectively.
 * 
 * ------------------------------
 * 
 * The code contained in this file, including both binary and source 
 * [if released by the owner(s)] (hereafter, Software) is subject to
 * copyright by the respective Licensor(s) and ownership remains with
 * such Licensor(s).  The Licensor(s) of the original Software remain
 * free to license their respective proprietary Software for other
 * purposes that are independent and separate from this file, without
 * obligation to any party.
 * 
 * Licensor(s) grant(s) you (hereafter, Licensee) a license to use the
 * Software for academic, research and internal business purposes only,
 * without a fee.  "Internal business purposes" means that Licensee may
 * install, use and execute the Software for the purpose of designing and
 * evaluating products.  Licensee may submit proposals for research
 * support, and receive funding from private and Government sponsors for
 * continued development, support and maintenance of the Software for the
 * purposes permitted herein.
 * 
 * Licensee may also disclose results obtained by executing the Software,
 * as well as algorithms embodied therein.  Licensee may redistribute the
 * Software to third parties provided that the copyright notices and this
 * License Agreement Notice statement are reproduced on all copies and
 * that no charge is associated with such copies.  No patent or other
 * intellectual property license is granted or implied by this Agreement,
 * and this Agreement does not license any acts except those expressly
 * recited.
 * 
 * Licensee may modify the Software to make derivative works (as defined
 * in Section 101 of Title 17, U.S. Code) (hereafter, Derivative Works),
 * as necessary for its own academic, research and internal business
 * purposes.  Title to copyrights and other proprietary rights in
 * Derivative Works created by Licensee shall be owned by Licensee
 * subject, however, to the underlying ownership interest(s) of the
 * Licensor(s) in the copyrights and other proprietary rights in the
 * original Software.  All the same rights and licenses granted herein
 * and all other terms and conditions contained in this Agreement
 * pertaining to the Software shall continue to apply to any parts of the
 * Software included in Derivative Works.  Licensee's Derivative Work
 * should clearly notify users that it is a modified version and not the
 * original Software distributed by the Licensor(s).
 * 
 * If Licensee wants to make its Derivative Works available to other
 * parties, such distribution will be governed by the terms and
 * conditions of this License Agreement.  Licensee shall not modify this
 * License Agreement, except that Licensee shall clearly identify the
 * contribution of its Derivative Work to this file by adding an
 * additional copyright notice to the other copyright notices listed
 * above, to be added below the line "Copyright Notices/Identification of
 * Subsequent Licensor(s)/Contributors of Derivative Works."  A party who
 * is not an owner of such Derivative Work within the meaning of
 * U.S. Copyright Law (i.e., the original author, or the employer of the
 * author if "work of hire") shall not modify this License Agreement or
 * add such party's name to the copyright notices above.
 * 
 * Each party who contributes Software or makes a Derivative Work to this
 * file (hereafter, Contributed Code) represents to each Licensor and to
 * other Licensees for its own Contributed Code that:
 * 
 * (a) Such Contributed Code does not violate (or cause the Software to
 * violate) the laws of the United States, including the export control
 * laws of the United States, or the laws of any other jurisdiction.
 * 
 * (b) The contributing party has all legal right and authority to make
 * such Contributed Code available and to grant the rights and licenses
 * contained in this License Agreement without violation or conflict with
 * any law.
 * 
 * (c) To the best of the contributing party's knowledge and belief,
 * the Contributed Code does not infringe upon any proprietary rights or
 * intellectual property rights of any third party.
 * 
 * LICENSOR(S) MAKE(S) NO REPRESENTATIONS ABOUT THE SUITABILITY OF THE
 * SOFTWARE OR DERIVATIVE WORKS FOR ANY PURPOSE.  IT IS PROVIDED "AS IS"
 * WITHOUT EXPRESS OR IMPLIED WARRANTY, INCLUDING BUT NOT LIMITED TO THE
 * MERCHANTABILITY, USE OR FITNESS FOR ANY PARTICULAR PURPOSE AND ANY
 * WARRANTY AGAINST INFRINGEMENT OF ANY INTELLECTUAL PROPERTY RIGHTS.
 * LICENSOR(S) SHALL NOT BE LIABLE FOR ANY DAMAGES SUFFERED BY THE USERS
 * OF THE SOFTWARE OR DERIVATIVE WORKS.
 * 
 * Any Licensee wishing to make commercial use of the Software or
 * Derivative Works should contact each and every Licensor to negotiate
 * an appropriate license for such commercial use, and written permission
 * of all Licensors will be required for such a commercial license.
 * Commercial use includes (1) integration of all or part of the source
 * code into a product for sale by or on behalf of Licensee to third
 * parties, or (2) distribution of the Software or Derivative Works to
 * third parties that need it to utilize a commercial product sold or
 * licensed by or on behalf of Licensee.
 * 
 * By using or copying this Contributed Code, Licensee agrees to abide by
 * the copyright law and all other applicable laws of the U.S., and the
 * terms of this License Agreement.  Any individual Licensor shall have
 * the right to terminate this license immediately by written notice upon
 * Licensee's breach of, or non-compliance with, any of its terms.
 * Licensee may be held legally responsible for any copyright
 * infringement that is caused or encouraged by Licensee's failure to
 * abide by the terms of this License Agreement.  
\*****************************************************************************/
/******************************************************************************\
 *
 *  File:  m_arm.h
 *
 *  Description: Defines symbols used for reference in Mspec and code
 * 		generator.
 *
 *  Creation Date : January 2004
 *
 *  Author:  Nate Clark
 *  Modified: from Richard E. Hank's and Wen-mei Hwu's code.
 *
 *  Revisions:
 *
 *
\******************************************************************************/
#ifndef M_ARM_H
#define M_ARM_H

#include <machine/m_spec.h>

/*
 * Declarations for processor models
 */
enum 
{
    M_ARM_1_01 = 1		
};

enum {
    ARM_MAC_LEAF=100,       /* 1 if leaf function, 0 if not leaf function*/
    ARM_MAC_ALLOC,          /* total alloc space requirements            */
    ARM_MAC_CALLEE_I,
    ARM_MAC_TRUE_SP,
    ARM_MAC_RETADDR,
    ARM_MAC_FZERO,
    ARM_MAC_FONE,

    ARM_MAC_LAST		/* Please not to put anything after this. :) */
};

/* maximum values for immediate operands */
#define INT_2EXP13              0x2000
#define INT_2EXP10              0x400
#define INT_2EXP4               0x10

#define MAX_INT_NAME            22

#define FIELD_5(a)      ((((a) >= -INT_2EXP4)&&((a) < INT_2EXP4))?1:0)
#define FIELD_11(a)     ((((a) >= -INT_2EXP10)&&((a) < INT_2EXP10))?1:0)
#define FIELD_14(a)     ((((a) >= -INT_2EXP13)&&((a) < INT_2EXP13))?1:0)



#ifdef __cplusplus
extern "C" {
#endif

extern int M_arm_type_size ( int mtype );
extern int M_arm_type_align ( int mtype );
extern void M_arm_void ( M_Type type );
extern void M_arm_bit_long ( M_Type type, int n );
extern void M_arm_bit_int ( M_Type type, int n );
extern void M_arm_bit_short ( M_Type type, int n );
extern void M_arm_bit_char ( M_Type type, int n );
extern void M_arm_char ( M_Type type, int unsign );
extern void M_arm_short ( M_Type type, int unsign );
extern void M_arm_int ( M_Type type, int unsign );
extern void M_arm_long ( M_Type type, int unsign );
extern void M_arm_float ( M_Type type, int unsign );
extern void M_arm_double ( M_Type type, int unsign );
extern void M_arm_pointer ( M_Type type );
extern int M_arm_eval_type ( M_Type type, M_Type ntype );
extern int M_arm_eval_type2 ( M_Type type, M_Type ntype );
extern int M_arm_call_type ( M_Type type, M_Type ntype );
extern int M_arm_call_type2 ( M_Type type, M_Type ntype );
extern void M_arm_array_layout ( M_Type type, int *offset );
extern int M_arm_array_align ( M_Type type );
extern int M_arm_array_size ( M_Type type, int dim );
extern void M_arm_union_layout ( int n, _M_Type *type, int *offset,
			int *bit_offset );
extern int M_arm_union_align ( int n, _M_Type *type );
extern int M_arm_union_size ( int n, _M_Type *type );
extern void M_arm_struct_layout ( int n, _M_Type *type, int *base,
			int *bit_offset );
extern int M_arm_struct_align ( int n, _M_Type *type );
extern int M_arm_struct_size ( int n, _M_Type *type, int struct_align );
extern int M_arm_layout_fnvar (List param_list, char **base_macro,
			       int *pcount, int purpose, int needs_st);
extern int M_arm_fnvar_layout (int n, _M_Type * type, long int *offset,
                               int *mode, int *reg, int *paddr, char **macro,
                               int *su_sreg, int *su_ereg, int *pcount,
                               int is_st, int purpose);
extern int M_arm_lvar_layout ( int n, _M_Type *type, long int *offset,
			char **base_macro );
extern int M_arm_no_short_int ( void );
extern int M_arm_layout_order ( void );
extern void M_arm_cb_label_name ( char *fn, int cb, char *line, int len );
extern int M_arm_is_cb_label ( char *label, char *fn, int *cb );
extern void M_arm_jumptbl_label_name(char *fn, int tbl_id, char *line, int len);
extern int M_arm_is_jumptbl_label(char *label, char *fn, int *tbl_id);
extern int M_arm_structure_pointer ( int purpose );
extern int M_arm_return_register ( int type, int purpose );
extern char *M_arm_fn_label_name ( char *label,
		int (*is_func) ( char * is_func_label) );
extern char *M_arm_fn_name_from_label ( char *label );
extern void M_set_model_arm ( char *model_name );
extern int M_arm_fragile_macro ( int macro_value );
extern Set M_arm_fragile_macro_set();
extern int M_arm_dataflow_macro(int id);
extern int M_arm_subroutine_call ( int opc );
extern void M_define_macros_arm ( STRING_Symbol_Table * sym_tbl );
extern char *M_get_macro_name_arm ( int id );
extern void M_define_opcode_name_arm ( STRING_Symbol_Table * sym_tbl );
extern char *M_get_opcode_name_arm ( int id );
extern int M_oper_supported_in_arch_arm ( int opc );
extern int M_num_oper_required_for_arm ( L_Oper *oper, char *name );
extern int M_is_stack_operand_arm ( L_Operand *operand );
extern int M_is_unsafe_macro_arm ( L_Operand *operand );
extern int M_operand_type_arm ( L_Operand *operand );
extern int M_conflicting_operands_arm ( L_Operand *operand,
		L_Operand **conflict_array, int len, int prepass );
extern int M_num_registers_arm ( int ctype );

#ifdef __cplusplus
}
#endif

#endif
