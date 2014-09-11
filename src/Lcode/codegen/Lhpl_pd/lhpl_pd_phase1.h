/*************************************************************************\
 *
 *  File:  lhpl_pd_phase1.h
 *  
 *  Description:
 *	    Additional hpl_pd opcodes
 *
 *  Creation Date :  August 1993
 *
 *  Author:  Scott A. Mahlke
 *
\************************************************************************/
#ifndef LHPL_PD_PHASE1_H
#define LHPL_PD_PHASE1_H

/*
 * List of floating point literals in the current function 
 */

typedef struct float_list {
    int type;
    union {
	double   f;
	double f2;
    } value;
} FLOAT_LIST;

#define MAX_FLOATS		100


/*
 *	Cache hierarchy mgmt defines 
 */

#define LPLAYDOH_CACHE_SPECIFIER_NAME	"chs"
#define LPLAYDOH_CACHE_SPECIFIER_V1	0
#define LPLAYDOH_CACHE_SPECIFIER_C1	1
#define LPLAYDOH_CACHE_SPECIFIER_C2	2
#define LPLAYDOH_CACHE_SPECIFIER_C3	3

#ifdef __cplusplus
extern "C" {
#endif

extern L_Cb	*mcb;

extern void L_print_fp_constant_data ( void );
extern void L_process_func ( L_Func *fn );
extern void L_init ( Parm_Macro_List *command_line_macro_list );

#ifdef __cplusplus
}
#endif

#endif
