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
 *      File:   sm_mdes.h
 *      Author: John C. Gyllenhaal, Wen-mei Hwu
 *      Creation Date:  March 1996
\*****************************************************************************/

#ifndef SM_MDES_H
#define SM_MDES_H

/* 10/29/02 REK Adding config.h */
#include <config.h>
#include <library/md.h>
#include <library/set.h>

typedef struct SM_Resource
{
  char *name;
  unsigned int mask;		/* Bit mask for this resource */
  unsigned short offset;	/* Offset for this resource */
}
SM_Resource;

typedef struct SM_Usage
{
  unsigned int resources_used;	/* Bit mask of resources used */
  unsigned short map_offset;	/* Offset into resource map */
}
SM_Usage;

typedef struct SM_Option
{
  SM_Usage *first_usage;	/* Array of resource usages */
  SM_Usage *last_usage;		/* Last usage in above array */
}
SM_Option;

/* Possible names:
 * SM_Req SM_Required SM_Use SM_Used SM_Head SM_Tree SM_Node SM_Choice
 * SM_Selection
 */
typedef struct SM_Choice
{
  SM_Option *first_option;	/* First in an array of options */
  SM_Option *last_option;	/* Last option in above array */
}
SM_Choice;

typedef struct SM_Table
{
  SM_Choice *first_choice;	/* First in an array of choices */
  SM_Choice *last_choice;	/* Last choice in above array */
  SM_Choice *slot_choice;	/* Choice that sets slot used */
  unsigned short num_choices;	/* Number of choices (array size) */
  unsigned short *slot_options;	/* Depends on the slot choice */
  unsigned short max_usage_offset;	/* Max offset of any usage in table */
}
SM_Table;

/* 20030527 SZU
 * Structure for Syllable_Type
 */
typedef struct SM_Syllable
{
  char *name;
  unsigned int mask;		/* Bit mask for this syllable */
  unsigned int num_slots;	/* Number of slots syllable takes */
}
SM_Syllable;

/* 20030527 SZU
 * Structure for Port_Type
 */
typedef struct SM_Port
{
  char *name;
  unsigned int mask;		/* Bit mask for this port type */
  SM_Syllable *syllable;	/* Syllable for this port type */
}
SM_Port;

/* 20030528 SZU
 * Structure for Restrictions
 */
typedef struct SM_Restriction
{
  char *name;
  unsigned int num;		/* Number before violating this restriction */
  unsigned int mask;		/* Bit mask of ports for this restriction */
}
SM_Restriction;

/* 20030529 SZU
 * Structure for Templates
 */
typedef struct SM_Template
{
  char *name;
  unsigned int mask;		/* Bit mask of shifted syll for this template */
  int *stop_bits;		/* Array for stop bits */
  unsigned int template_mask;	/* Bit mask of this template */
}
SM_Template;

/* 20030529 SZU
 * Structure for Template_Groups
 */
typedef struct SM_TGroup
{
  char *name;
  unsigned int mask;		/* Bit mask of templates in this group */
}
SM_TGroup;

/* 20030530 SZU
 * Structure for Issue_Groups
 */
typedef struct SM_Issue
{
  char *name;
  unsigned int num_templates;	/* Number of templates in this issue group */
  unsigned int *templates;
}
SM_Issue;

/* 20030531 SZU
 * Strucutre for Dispersal_Rules
 */
typedef struct SM_DRule
{
  char *name;
  int *slot;		/* Array for slots that fall under this dispersal */
  unsigned int num_slots;	/* Number of slots; 0 means all slots */
  unsigned int port_mask;	/* HEX representation of the Port_Type */
  unsigned int syll_mask;	/* HEX representation of the Syllable_Type */
  unsigned int *rsrc;	/* Array of resource masks. Order to try dispersal */
  unsigned int num_rsrcs;	/* Number of resources to try */
  /* 20030617 SZU
   * Added for template masks if defined; 0 if not.
   */
  unsigned int templates;
}
SM_DRule;

/* 20030603 SZU
 * Intermediate linked list structure to keep track of opcodes for nop.
 */
typedef struct SM_Nop
{
  int opcode;	/* The opcode */
  int priority;	/* Priority of this nop */
  struct SM_Nop *prev_nop;
  struct SM_Nop *next_nop;
}
SM_Nop;

/* 20020723 SZU
 * From SM_Prod through SM_PCLat
 * Added to support Prod_Cons_Latency in SM
 * 20020822 SZU
 * Trying to use the Set data structure for better matching of PCLat.
 */
typedef struct SM_PCLat
{
  char *name;
  Set plat;
  int *pdest_latency_penalty;		/* Similar to Mdes_Latency */
  Set clat;
  int *csrc_latency_penalty;		/* Similar to Mdes_Latency */
}
SM_PCLat;

typedef struct SM_Mdes
{
  struct Mdes2 *mdes2;		/* What this was build from */
  MD *md_mdes;			/* The md version of the mdes */
  SM_Resource *resource;	/* Array of resources */
  unsigned int num_resources;	/* Size of resource array */
  unsigned int map_width;	/* words/cycle in map */
  unsigned int time_shift;	/* Accounts for map width */
  unsigned int max_num_choices;	/* Max across all tables */

  /*
   * Fields below used for building/freeing schedule manager's mdes.
   * In general not used directly by the schedule manager.
   */
  SM_Option *unit_array;	/* Array of units */
  unsigned int unit_array_size;	/* Size of unit array */
  SM_Choice *choice_array;	/* Array of choices */
  unsigned int choice_array_size;	/* Size of choice array */
  SM_Table *table_array;	/* Array of tables */
  unsigned int table_array_size;	/* Size of table array */
  /*
   * Fields above used for building/freeing schedule manager's mdes.
   * In general not used directly by the schedule manager.
   */

  /* 20020527 SZU
   * Fields below added for Syllable_Type
   */
  SM_Syllable *syllable_array;
  unsigned int num_syllables;
  unsigned int top_syllable;	/* Hex number of top syllable */

  /* 20020527 SZU
   * Fields below added for Port_Type
   */
  SM_Port *port_array;
  unsigned int num_ports;

  /* 20020527 SZU
   * Fields below added for Restrictions
   */
  SM_Restriction *restrict_array;
  unsigned int num_restricts;

  /* 20030529 SZU
   * Fields below added for Templates
   */
  SM_Template *template_array;
  unsigned int num_templates;
  unsigned int template_shift_var;	/* Number of bits to shift syll by */
  unsigned int slots_per_template;	/* <= max_slots, mod max_slots = 0 */

  /* 20030529 SZU
   * Fields below added for Template_Groups
   */
  SM_TGroup *tgroup_array;
  unsigned int num_tgroups;

  /* 20030530 SZU
   * Fields below added for Issue_Groups
   */
  SM_Issue *issue_array;
  unsigned int num_issues;
  unsigned int max_template_per_issue;	/* * slots_per_template <= max_slots */

  /* 20030531 SZU
   * Fields below added for Dispersal_Rules
   */
  SM_DRule *drule_array;
  unsigned int num_drules;

  /* 20030603 SZU
   * Fields below added for array of nop opcodes
   */
  int *nop_array;	/* Array of opcodes that are nop, heuristic order */
  int max_nop_index;	/* Max index element for nop_array */

  /* 20020724 SZU
   * Fields below added for Prod_Cons_Latency
   */
  SM_PCLat *pclat_array;
  unsigned int pclat_array_size;
}
SM_Mdes;

extern SM_Mdes *SM_build_mdes (struct Mdes2 *mdes2);

#endif
