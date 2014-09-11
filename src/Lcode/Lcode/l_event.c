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
 *      File:  l_event.c
 *      Author: Richard E. Hank, Wen-mei Hwu
 *      Creation Date:  April 1995
 *
\*****************************************************************************/

/* 10/29/02 REK Adding config.h */
#include <config.h>
#include <Lcode/l_main.h>
#include "l_event.h"

/*=========================================
 * L_Event_Map Routines
 *=========================================*/
L_Event_Map *
L_new_event_map (int event_id, char *command)
{
  L_Event_Map *new_map = (L_Event_Map *) L_alloc (L_alloc_event_map);

  new_map->event_id = event_id;
  new_map->flags = 0;
  new_map->command = command;
  new_map->arch = NULL;
  new_map->model = NULL;
  new_map->action = NULL;

  return (new_map);
}

/*=========================================
 * L_Event Routines
 *=========================================*/

L_Event *
L_new_event (int event_num, char *event)
{
  L_Event *new_event = (L_Event *) L_alloc (L_alloc_event);

  new_event->event_num = event_num;
  new_event->event_map = NULL;
  new_event->command = L_add_string (L_string_table, event);
  new_event->arguments = NULL;
  new_event->attr = NULL;
  new_event->prev_event = NULL;
  new_event->next_event = NULL;

  return (new_event);
}

void
L_delete_event (L_Event * event)
{
  event->event_num = -1;
  event->event_map = NULL;
  event->command = NULL;
  event->prev_event = NULL;
  event->next_event = NULL;
  /* 
   *  Delete the arguments
   */
  if (event->arguments != NULL)
    L_delete_all_attr (event->arguments);
  event->arguments = NULL;
  /*
   *  Delete the attributes
   */
  if (event->attr != NULL)
    L_delete_all_attr (event->attr);
  event->arguments = NULL;

  L_free (L_alloc_event, event);
}

void
L_delete_event_list (L_Event * event_list)
{
  L_Event *current_event;

  current_event = event_list;
  while (current_event != NULL)
    {
      L_delete_event (current_event);
      current_event = current_event->next_event;
    }
}

L_Event *
L_add_event_to_front (L_Event * event_list, L_Event * new_event)
{
  L_Event *tmp;

  if (event_list == NULL)
    return (new_event);

  if (new_event == NULL)
    return (event_list);

  for (tmp = new_event; tmp->next_event != NULL; tmp = tmp->next_event)
    ;

  tmp->next_event = event_list;
  event_list->prev_event = tmp;

  return (new_event);
}

L_Event *
L_concat_event (L_Event * event_list, L_Event * new_event)
{
  L_Event *tmp;

  if (event_list == NULL)
    return (new_event);

  if (new_event == NULL)
    return (event_list);

  for (tmp = event_list; tmp->next_event != NULL; tmp = tmp->next_event)
    ;

  tmp->next_event = new_event;
  new_event->prev_event = tmp;

  return (event_list);
}

/*============================================================
 *
 *  Eventlist input/output routines
 *
 *============================================================*/
L_Event *
L_read_event (L_Input_Buf * input_buf)
{
  char ch, *token;
  int id;
  L_Attr *attr;
  L_Event *event;

  /* Get the open paren that starts the event */
  L_get_next_lcode_token_expecting (input_buf, "(", "L_read_event");

  /* Read event id */
  token = L_get_next_lcode_token (input_buf);
  if (L_strtol (token, NULL, &id) == 0)
    {
      L_print_buf_with_arrow (stderr, input_buf);
      L_punt ("L_read_event:  The event id is not an integer value.");
    }

  /* Read the event command */
  token = L_get_next_lcode_token (input_buf);

  /* Token should not begin with a quote */
  if (token[0] == '"')
    {
      L_print_buf_with_arrow (stderr, input_buf);
      L_punt ("L_read_event: This is not a valid event command!\n");
    }
  event = L_new_event (id, token);

  /* Event arguments are formated as Lcode attributes  */
  /* Arguments are read until the character is a '<'   */
  /* or a ')'.  A '<' indicates that the arguments are */
  /* complete and the optional attributes begin.  A ')' */
  /* terminates the event.                               */

  ch = L_peek_next_char (input_buf);

  while (ch != '<' && ch != ')')
    {
      attr = L_read_attr (NULL, input_buf);
      event->arguments = L_concat_attr (event->arguments, attr);

      ch = L_peek_next_char (input_buf);
    }

  if (ch == '<')
    {
      /* Read event attributes */
      L_get_next_lcode_token (input_buf);       /* Throw away '>' */
      while (ch != '>')
        {
          attr = L_read_attr (NULL, input_buf);
          event->attr = L_concat_attr (event->attr, attr);

          ch = L_peek_next_char (input_buf);
        }
      L_get_next_lcode_token (input_buf);       /* Throw away '>' */
    }

  /* Terminate the event */
  L_get_next_lcode_token_expecting (input_buf, ")", "L_read_event");

  return (event);
}

L_Event *
L_read_event_list (L_Input_Buf * input_buf)
{
  L_Event *event_list = NULL;
  L_Event *tmp, *current_event = NULL;

  while (L_peek_next_char (input_buf) != ')')
    {
      tmp = L_read_event (input_buf);
      if (current_event == NULL)
        {
          event_list = current_event = tmp;
        }
      else
        {
          current_event->next_event = tmp;
          current_event = tmp;
        }
    }
  L_get_next_lcode_token_expecting (input_buf, ")", "L_read_event_list");

  return (event_list);
}

void
L_print_event (FILE * F, L_Event * event)
{
  L_Attr *attr;

  fprintf (F, "\t(%d %s ", event->event_num, event->command);

  /*
   *  Print the arguments
   */
  if (event->arguments != NULL)
    {
      for (attr = event->arguments; attr != NULL; attr = attr->next_attr)
        L_print_attr (F, attr);
    }

  /*
   *  Print the attributes
   */
  if (event->attr != NULL)
    {
      fprintf (F, " <");
      for (attr = event->attr; attr != NULL; attr = attr->next_attr)
        L_print_attr (F, attr);
      fprintf (F, ">");
    }
  fprintf (F, ")\n");
}

void
L_print_event_list (FILE * F, L_Event * eventlist)
{
  L_Event *current_event;

  fprintf (F, "(event_list\n");

  current_event = eventlist;
  while (current_event != NULL)
    {
      L_print_event (F, current_event);
      current_event = current_event->next_event;
    }

  fprintf (F, ")\n");
}

void
L_print_result_list (FILE * F, L_Event * eventlist)
{
  L_Event *current_event;

  fprintf (F, "(result_list\n");

  current_event = eventlist;
  while (current_event != NULL)
    {
      L_print_event (F, current_event);
      current_event = current_event->next_event;
    }

  fprintf (F, ")\n");
}
