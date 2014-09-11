/*************************************************************************\
 *
 *  File:  lhpl_pd_phase3.h 
 *
 *  Description:
 *    Include file for Print module of HPL_PD architecture
 *
 *  Creation Date :  August 1993
 *
 *  Author:  Scott A. Mahlke
 *
\************************************************************************/
#ifndef LHPL_PD_PHASE3_H
#define LHPL_PD_PHASE3_H

#ifdef __cplusplus
extern "C" {
#endif

/*
 *	lhpl_pd_phase3_func.c
 */
extern void P_init ( void );
extern void P_process_func ( L_Func *fn );

/*
 *	lhpl_pd_phase3_data.c
 */
extern void P_process_data ( FILE *F, L_Data *data );

#ifdef __cplusplus
}
#endif

#endif
