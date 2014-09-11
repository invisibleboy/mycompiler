/*****************************************************************************\
 *
 *                    Illinois Open Source License
 *                     University of Illinois/NCSA
 *                         Open Source License
 *
 * Copyright (c) 2004, The University of Illinois at Urbana-Champaign.
 * All rights reserved.
 *
 * Developed by:
 *
 *              IMPACT Research Group
 *
 *              University of Illinois at Urbana-Champaign
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
 *      File :          l_super_load_annotate.c
 *      Description :   Append each load with it final superscalar oper id, 
 *			from this point on, its unique load oper id attribute  
 *      Author :        Geoff Kent 
 *      Date :          March 2003
 *
 *==========================================================================*/

#include <config.h>
#include "l_opti.h"



void
L_tag_load(L_Func *fn)
{

   L_Cb *cb;
   L_Oper *oper;
   L_Attr *attr;

   int load_prev_tag = 0; /*boolean that detects if load has already been 
			    tagged */

   for (cb = fn->first_cb; cb; cb = cb->next_cb)
      {
         for (oper = cb->first_op; oper; oper = oper->next_op)
            {
               if (L_load_opcode(oper))
                  {

		     for (attr = oper->attr; attr ; attr = attr->next_attr)
                     {
                       if (!strcmp(attr->name, "load_id"))
			{
			//load has already been tagged earlier
			load_prev_tag = 1;
			break;
			}
                     } 
                     if (load_prev_tag == 1)
		        {
			   load_prev_tag = 0;
			   continue;
		        }	
                     else
                        {
		           attr = L_new_attr ("load_id", 1);
                           L_set_int_attr_field (attr, 0, oper->id);
                           L_concat_attr (oper->attr, attr);
			}
                  }
            }
      }
}
