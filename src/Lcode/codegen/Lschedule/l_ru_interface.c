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
/*****************************************************************************\
 *
 *  File:  l_ru_interface.c
 *
 *  Description:
 *    Lcode interface to RU manager for building RU_Info structures
 *
 *  Creation Date : May 1993
 *
 *  Authors : Scott Mahlke, John Gyllenhaal
 *
 *  Revision 1.1.1.1  1995/08/30 16:49:03  david
 *  Import of IMPACT source
 *
 * Revision 1.1  1994/01/19  18:49:25  roger
 * Initial revision
 *
 *
\*****************************************************************************/
/* 10/29/02 REK Adding config.h */
#include <config.h>
#include "l_schedule.h"
#include "RU_manager.h"
#include "l_ru_interface.h"

/*
 *	Assume RU_set_max_pred() called before this!!!!!!!!
 */

RU_Info* L_create_ru_info_oper (oper)
L_Oper *oper;
{
    int i, *pred;
    RU_Info *ru_info = NULL;

    /* check if oper is to be ignored by the scheduler */
    if (op_flag_set(oper->proc_opc, OP_FLAG_IGNORE))
	return (ru_info);

    /* create array of predicate registers */
    pred = RU_pred_alloc();

    /* Only permit one predicate for now - RAB */
    for (i=0; i< 1 /* L_max_pred_operand */; i++) {
        if (oper->pred[i]==NULL)
            pred[i] = 0;
        else if (L_is_register(oper->pred[i]))
            pred[i] = oper->pred[i]->value.r;
        else
            L_punt("L_initialize_ru_info: illegal pred operand");
    }

    /* create info structure */
    ru_info = RU_info_create(oper, pred);
    ru_info->proc_opc = oper->proc_opc;

    return ru_info;
}

#if 0
void L_create_ru_info_cb (cb)
L_Cb *cb;
{
    L_Oper *oper;

    for (oper=cb->first_op; oper!=NULL; oper=oper->next_op) {
	L_create_ru_info_oper(oper);
    }
}

void L_create_ru_info_fn (fn)
L_Func *fn;
{
    L_Cb *cb;

    for (cb=fn->first_cb; cb!=NULL; cb=cb->next_cb) {
	L_create_ru_info_cb(cb);
    }
}
#endif

void L_delete_ru_info_oper (ru_info)
RU_Info *ru_info;
{
    if (ru_info!=NULL) RU_info_delete(ru_info);
}

#if 0
void L_delete_ru_info_cb (cb)
L_Cb *cb;
{
    L_Oper *oper;

    for (oper=cb->first_op; oper!=NULL; oper=oper->next_op) {
	L_delete_ru_info_oper(oper);
    }
}

void L_delete_ru_info_fn (fn)
L_Func *fn;
{
    L_Cb *cb;

    for (cb=fn->first_cb; cb!=NULL; cb=cb->next_cb) {
	L_delete_ru_info_cb(cb);
    }
}
#endif
