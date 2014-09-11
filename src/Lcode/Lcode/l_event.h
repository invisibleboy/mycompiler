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
 *      File:  l_event.h
 *      Author: Richard E. Hank, Wen-mei Hwu
 *      Creation Date:  April 1995
 *
\*****************************************************************************/
#ifndef L_EVENT_H
#define L_EVENT_H

/* 10/29/02 REK Adding config.h */
#include <config.h>

/*==========================================================================*/
/*
 *      Lcode event - data structures
 */
/*==========================================================================*/

/*
 * L_Event_Map
 */
typedef struct L_Event_Map
{
  int event_id;
  int flags;
  char *command;
  char *arch;
  char *model;
  void *action;
}
L_Event_Map;

/* 
 *      L_Event
 */

typedef struct L_Event
{
  int event_num;
  struct L_Event_Map *event_map;
  char *command;
  L_Attr *arguments;
  L_Attr *attr;
  struct L_Event *prev_event;
  struct L_Event *next_event;
}
L_Event;

#ifdef __cplusplus
extern "C"
{
#endif

  extern L_Event *L_event_list; /* current active event list */
  extern L_Event *L_result_list;        /* current active result list */

  extern L_Event_Map *L_new_event_map (int event_id, char *command);

  extern L_Event *L_new_event (int event_num, char *event);
  extern void L_delete_event (L_Event * event);
  extern void L_delete_event_list (L_Event * event_list);
  extern L_Event *L_add_event_to_front (L_Event * event_list,
                                        L_Event * new_event);
  extern L_Event *L_concat_event (L_Event * event_list, L_Event * new_event);


#include <Lcode/l_io.h>

  extern L_Event *L_read_event (L_Input_Buf * input_buf);
  extern L_Event *L_read_event_list (L_Input_Buf * input_buf);
  extern void L_print_event (FILE * F, L_Event * event);
  extern void L_print_event_list (FILE * F, L_Event * eventlist);
  extern void L_print_result_list (FILE * F, L_Event * eventlist);

#ifdef __cplusplus
}
#endif


#endif
