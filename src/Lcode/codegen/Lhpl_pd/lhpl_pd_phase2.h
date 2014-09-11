/******************************************************************************\
 *
 *  File:  lhpl_pd_phase2.h
 *
 *  Description:  Header file for phase2 of HPL_PD code generator
 *
 *  Creation Date :  August 1993
 *
 *  Author:  Scott A. Mahlke
 *
\******************************************************************************/
#ifndef LHPL_PD_PHASE2_H
#define LHPL_PD_PHASE2_H

/*
 *	HPPA stuff
 */
#define MAX_INT_NAME            22

#ifdef __cplusplus
extern "C" {
#endif

/*
 *	lplaydoh_phase2_func.c prototypes
 */
extern void O_annotate ( L_Func *fn );
extern void O_perform_init ( L_Func *fn );
extern void O_process_func ( L_Func *fn, Parm_Macro_List *command_line_macro_list );
extern void O_init ( Parm_Macro_List *command_line_macro_list );

/*
 *	lplaydoh_phase2_reg.c prototypes
 */
extern L_Oper* O_spill_reg ( int reg, int type, L_Operand *operand,
				int spill_offset,L_Operand **pred, int type_flag );
extern L_Oper* O_fill_reg ( int reg, int type, L_Operand *operand,
				int fill_offset, L_Operand **pred, int type_flag );
extern L_Oper *O_jump_oper ( int opc, L_Cb *dest_cb );
extern double R_caller_cost ( int lcode_ctype, int leaf );
extern double R_callee_cost ( int lcode_ctype, int leaf, int callee_allocated );
extern double R_spill_store_cost ( int lcode_ctype );
extern double R_spill_load_cost ( int lcode_ctype );
extern void O_register_init ( void );
extern void O_register_allocation ( L_Func *fn,
				Parm_Macro_List *command_line_macro_list );

#ifdef __cplusplus
}
#endif

#endif
