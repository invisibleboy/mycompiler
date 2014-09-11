/************************************************************************\
 *
 *  File:  lhpl_pd_phase3_func.c 
 *
 *  Description:
 *    Print function for HPL_PD architecture
 *
 *  Creation Date :  August 1993
 *
 *  Author:  Scott A. Mahlke
 *
\************************************************************************/
#include "lhpl_pd_main.h"

/*
 *	Just a dummy routines currently for HPL_PD architecture!!
 */

void P_init(void)
{
}

void P_process_func(L_Func *fn)
{
    L_print_func(L_OUT, L_fn);
}
