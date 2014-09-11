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
 *      File:   lmdes2.c
 *      Author: John C. Gyllenhaal, Wen-mei Hwu
 *      Creation Date:  April 1996
\*****************************************************************************/

/* 10/29/02 REK Adding config.h */
#include <config.h>
#include <string.h>
#include <stdlib.h>
#include <stdarg.h>
#include <ctype.h>
#include <malloc.h>
#include <library/md.h>
#include <library/i_error.h>
#include "lmdes.h"
#include "lmdes2.h"
#include "mdes2.h"
#include <library/heap.h>
#include <Lcode/l_flags.h>

#define DEBUG_LMDES2 0

/* Use sm_mdes function to build new mdes */
extern int SM_add_ids_to_entries (MD_Section * section);
static void init_stats (Mdes_Stats * stats);

MD_Field *
find_parm_value (MD * md, char *parm_name)
{
  MD_Section *parm_section;
  MD_Entry *parm_entry;
  MD_Field_Decl *value_field_decl;
  MD_Field *value_field;

  if ((parm_section = MD_find_section (md, "Parameter")) == NULL)
    I_punt ("load_mdes_from_version2: Parameter section expected!");

  if ((parm_entry = MD_find_entry (parm_section, parm_name)) == NULL)
    I_punt ("load_mdes_from_version2: Parameter '%s' expected!", parm_name);

  if ((value_field_decl = MD_find_field_decl (parm_section, "value")) == NULL)
    I_punt ("load_mdes_from_version2: Field 'value' expected in '%s'!",
	    parm_section->name);

  if ((value_field = MD_find_field (parm_entry, value_field_decl)) == NULL)
    I_punt ("load_mdes_from_version2: Field '%s' expected from parm '%s'!",
	    value_field_decl->name, parm_name);

  return (value_field);
}


MD_Field *
find_field_by_name (MD_Entry * entry, char *field_name)
{
  MD_Field_Decl *field_decl;
  MD_Field *field;

  if ((field_decl = MD_find_field_decl (entry->section, field_name)) == NULL)
    I_punt ("load_mdes_from_version2: Field '%s' expected in '%s'!",
	    field_name, entry->section->name);

  field = MD_find_field (entry, field_decl);
  return (field);
}

char *
get_string_parm (MD * md, char *parm_name)
{
  MD_Field *value_field;
  char *value_string;

  value_field = find_parm_value (md, parm_name);
  value_string = MD_get_string (value_field, 0);
  return (value_string);
}

int
get_int_parm (MD * md, char *parm_name)
{
  MD_Field *value_field;
  char *value_string, *ptr, *end_ptr;
  int value;

  value_field = find_parm_value (md, parm_name);
  value_string = MD_get_string (value_field, 0);

  /* Convert string to int */

  /* Require the int to have at least 1 digit in it */
  for (ptr = value_string; *ptr != 0; ptr++)
    {
      if (isdigit (*ptr))
	break;
    }
  if (*ptr == 0)
    I_punt ("Parm '%s': invalid int value '%s'\n", value_string);

  /* Convert to long, make sure
   * conversion goes to end of string.
   */
  value = (int) strtol (value_string, &end_ptr, 0);

  if (*end_ptr != 0)
    I_punt ("Parm '%s': invalid int value '%s'\n", value_string);

  return (value);
}
/* Returns 1 s1 matches s2 (ignoring case), 0 otherwise */
static int
value_match (char *s1, char *s2)
{
  for (; *s1 != 0; s1++, s2++)
    {
      if (toupper (*s1) != toupper (*s2))
	return (0);
    }

  if (*s2 != 0)
    return (0);
  else
    return (1);
}

/* Returns 1 or 0 depending on parm value.
 * The following are equivalent (yes, y, 1, on) (no, n, 0, off).
 */
int
get_binary_parm (MD * md, char *parm_name)
{
  MD_Field *value_field;
  char *value_string;

  value_field = find_parm_value (md, parm_name);
  value_string = MD_get_string (value_field, 0);

  /* Convert to binary value */
  if (value_match (value_string, "yes") ||
      value_match (value_string, "y") ||
      value_match (value_string, "1") || value_match (value_string, "on"))
    return (1);

  if (value_match (value_string, "no") ||
      value_match (value_string, "n") ||
      value_match (value_string, "0") || value_match (value_string, "off"))
    return (0);

  I_punt ("Parm '%s': invalid binary value '%s'\n", parm_name, value_string);

  return (0);			/* To make the compiler happy */
}

int
get_num_entries (MD * md, char *section_name)
{
  MD_Section *section;
  int num_entries;

  section = MD_find_section (md, section_name);

  /* Not the best way to get the entry count */
  num_entries = section->entry_table->symbol_count;

  return (num_entries);
}

int
get_int_value (MD * md, char *section_name, char *entry_name,
	       char *field_name, int index)
{
  MD_Section *section;
  MD_Entry *entry;
  MD_Field_Decl *field_decl;
  MD_Field *field;
  int value;

  if ((section = MD_find_section (md, section_name)) == NULL)
    I_punt ("load_mdes_from_version2: %s section expected!", section_name);

  if ((entry = MD_find_entry (section, entry_name)) == NULL)
    I_punt ("load_mdes_from_version2: Entry '%s->%s' in expected!",
	    section_name, entry_name);

  if ((field_decl = MD_find_field_decl (section, field_name)) == NULL)
    I_punt ("load_mdes_from_version2: Field 'value' expected in '%s'!",
	    section->name);

  if ((field = MD_find_field (entry, field_decl)) == NULL)
    I_punt ("load_mdes_from_version2: Field '%s' expected in '%s->%s'!",
	    field_decl->name, section_name, entry_name);

  value = MD_get_int (field, index);

  return (value);
}

int
calc_total_entry_name_len (MD * md, char *section_name)
{
  MD_Section *section;
  MD_Entry *entry;
  int total_string_len, len;

  total_string_len = 0;

  if ((section = MD_find_section (md, section_name)) == NULL)
    I_punt ("load_mdes_from_version2: %s section expected!", section_name);

  /* Calculate the space needed to store all the entry names */
  for (entry = MD_first_entry (section); entry != NULL;
       entry = MD_next_entry (entry))
    {
      len = strlen (entry->name);
      /* Add 1 for terminator */
      total_string_len += len + 1;
    }

  return (total_string_len);
}

void
set_latencies (MD_Entry * entry, char *field_name, int *lat_array)
{
  MD_Entry *lat_entry;
  MD_Field *field, *time_field;
  int index1, lat;

  /* Do nothing if field not set */
  if ((field = find_field_by_name (entry, field_name)) == NULL)
    return;

  for (index1 = 0; index1 <= field->max_element_index; index1++)
    {
      lat_entry = MD_get_link (field, index1);
      if ((time_field = find_field_by_name (lat_entry, "time")) == NULL)
	I_punt ("set_latencies: %s->%s[%i]->%s time field expected!",
		entry->name, field_name, index1, lat_entry->name);

      /* First time is latency */
      lat = MD_get_int (time_field, 0);
      lat_array[index1] = lat;

      /* Sanity check */
      if (time_field->max_element_index != 0)
	I_punt ("set_latencies: %s->%s only one time may be specified!",
		time_field->entry->section->name, time_field->entry->name);
    }
}

int
calc_num_op_alts (MD_Entry * impact_op_entry)
{
  MD_Entry *op_entry, *alt_entry;
  MD_Field *op_field, *alt_field, *format_field;
  int num_alts;
  int index1, index2;

  num_alts = 0;

  /* Get the op field */
  op_field = find_field_by_name (impact_op_entry, "op");

  /* Sum up for each of this impact_op's operations */
  for (index1 = 0; index1 <= op_field->max_element_index; index1++)
    {
      /* Get the op entry */
      op_entry = MD_get_link (op_field, index1);

      /* Get the alt field */
      alt_field = find_field_by_name (op_entry, "alt");

      /* Sum up all the IO formats (each will be a different alt) for
       * each alternative.
       */
      for (index2 = 0; index2 <= alt_field->max_element_index; index2++)
	{
	  alt_entry = MD_get_link (alt_field, index2);

	  format_field = find_field_by_name (alt_entry, "format");

	  num_alts += format_field->max_element_index + 1;
	}
    }

  return (num_alts);
}

int
calc_num_alts (MD * md)
{
  MD_Section *impact_op_section, *op_section, *alt_section;
  MD_Entry *impact_op_entry, *op_entry, *alt_entry;
  MD_Field_Decl *op_field_decl, *alt_field_decl, *format_field_decl;
  MD_Field *op_field, *alt_field, *format_field;
  int num_alts;
  int index1, index2;

  num_alts = 0;

  op_section = MD_find_section (md, "Operation");
  alt_field_decl = MD_find_field_decl (op_section, "alt");

  alt_section = MD_find_section (md, "Scheduling_Alternative");
  format_field_decl = MD_find_field_decl (alt_section, "format");

  /* Sum up alts for each operation */
  impact_op_section = MD_find_section (md, "IMPACT_Operation");
  op_field_decl = MD_find_field_decl (impact_op_section, "op");
  for (impact_op_entry = MD_first_entry (impact_op_section);
       impact_op_entry != NULL;
       impact_op_entry = MD_next_entry (impact_op_entry))
    {
      /* Get the op field */
      op_field = MD_find_field (impact_op_entry, op_field_decl);

      /* Sum up for each of this impact_op's operations */
      for (index1 = 0; index1 <= op_field->max_element_index; index1++)
	{
	  /* Get the op entry */
	  op_entry = MD_get_link (op_field, index1);

	  /* Get the alt field */
	  alt_field = MD_find_field (op_entry, alt_field_decl);

	  /* Sum up all the IO formats (each will be a different alt) for
	   * each alteratnvie.
	   */
	  for (index2 = 0; index2 <= alt_field->max_element_index; index2++)
	    {
	      alt_entry = MD_get_link (alt_field, index2);

	      format_field = MD_find_field (alt_entry, format_field_decl);

	      num_alts += format_field->max_element_index + 1;
	    }
	}
    }

  return (num_alts);
}

int
calc_op_total_string_len (MD * md)
{
  MD_Section *impact_op_section, *op_section, *alt_section;
  MD_Entry *impact_op_entry, *op_entry, *alt_entry;
  MD_Field_Decl *op_field_decl, *alt_field_decl, *format_field_decl;
  MD_Field *op_field, *alt_field, *format_field;
  int total_string_len;
  int index1, index2, index3;

  total_string_len = 0;

  op_section = MD_find_section (md, "Operation");
  alt_field_decl = MD_find_field_decl (op_section, "alt");

  alt_section = MD_find_section (md, "Scheduling_Alternative");
  format_field_decl = MD_find_field_decl (alt_section, "format");

  /* Sum up alts for each operation */
  impact_op_section = MD_find_section (md, "IMPACT_Operation");
  op_field_decl = MD_find_field_decl (impact_op_section, "op");
  for (impact_op_entry = MD_first_entry (impact_op_section);
       impact_op_entry != NULL;
       impact_op_entry = MD_next_entry (impact_op_entry))
    {
      total_string_len += strlen (impact_op_entry->name) + 1;

      /* Get the op field */
      op_field = MD_find_field (impact_op_entry, op_field_decl);

      /* Sum up for each of this impact_op's operations */
      for (index1 = 0; index1 <= op_field->max_element_index; index1++)
	{
	  /* Get the op entry */
	  op_entry = MD_get_link (op_field, index1);

	  /* Get the alt field */
	  alt_field = MD_find_field (op_entry, alt_field_decl);

	  /* Sum up all the IO formats (each will be a different alt) for
	   * each alteratnvie.
	   */
	  for (index2 = 0; index2 <= alt_field->max_element_index; index2++)
	    {
	      alt_entry = MD_get_link (alt_field, index2);

	      format_field = MD_find_field (alt_entry, format_field_decl);
	      for (index3 = 0; index3 <= format_field->max_element_index;
		   index3++)
		{
		  total_string_len += strlen (op_entry->name) + 1;
		}
	    }
	}
    }

  return (total_string_len);
}

Mdes *
load_lmdes_from_version2 (char *file_name, int num_pred, int num_dest,
			  int num_src, int num_sync)
{
  MD *md;
  Mdes *mdes;
  char *processor_model;
  int struct_size;
  int data_size;
  int set_table_size;
  int alt_size;
  int buf_size;
  int *mask_ptr;
  int *array_ptr;
  Mdes_IO_Set **type_ptr;
  Mdes_Alt *alt_ptr;
  int total_string_len;
  int total_alts;
  Mdes_IO_Set *IO_set;
  Mdes_IO_Item *IO_item;
  Mdes_Resource *resource;
  Mdes_Latency *latency;
  Mdes_Alt *alt;
  Mdes_Operation *operation;
  char *name_buf_ptr;
  int IO_item_id, reslist_id, latency_id;
  int i, j;
  /* MD parsing local variables */
  Mdes_IO_Set *null_IO_set;
  MD_Section *section;
  MD_Entry *entry;
  MD_Field *field;
  int index1;
  /* 20031024 SZU */
  int num_slots, slots_per_template, max_template_per_issue;
  SM_Nop *first_nop = NULL, *last_nop = NULL, *current_nop = NULL;
  int num_nop = 0;
  int *nop_array;

  /* Malloc mdes structure and copy file name to field */
  Malloc_struct ((void **) &mdes, sizeof (Mdes));
  Malloc_name (&mdes->file_name, file_name);

  mdes->mdes2 = load_mdes2 (file_name);

  /* Read the md version of the machine description */
  md = mdes->mdes2->md_mdes;

  /* Point the mdes2 version of the mdes at the version1 version */
  mdes->mdes2->version1_mdes = mdes;

  /* Get the processor model */
  processor_model = get_string_parm (md, "processor_model");

#if DEBUG_LMDES2
  printf ("Processor model is '%s'\n", processor_model);
#endif

  if (strcmp (processor_model, "superscalar") == 0)
    mdes->processor_model = MDES_SUPERSCALAR;
  else if (strcmp (processor_model, "vliw") == 0)
    mdes->processor_model = MDES_VLIW;
  else
    I_punt ("load_lmdes_from_version2:\n"
	    " processor model expected to be superscalar or vliw not '%s'",
	    processor_model);


  mdes->number[MDES_PRED] = get_int_parm (md, "num_pred_operands");
  mdes->number[MDES_DEST] = get_int_parm (md, "num_dest_operands");
  mdes->number[MDES_SRC] = get_int_parm (md, "num_src_operands");

  /* Version2 currently allows 3 syncs to be specified */
  mdes->number[MDES_SYNC_IN] = 3;
  mdes->number[MDES_SYNC_OUT] = 3;

  /* Increase operand counts to be at least as big as those specified
   * by the parameters. This allows guarantees the mdes operand counts 
   * will be compatable with the lcode operand counts, etc.
   */
  if (mdes->number[MDES_PRED] < num_pred)
    mdes->number[MDES_PRED] = num_pred;
  if (mdes->number[MDES_DEST] < num_dest)
    mdes->number[MDES_DEST] = num_dest;
  if (mdes->number[MDES_SRC] < num_src)
    mdes->number[MDES_SRC] = num_src;
  if (mdes->number[MDES_SYNC_IN] < num_sync)
    mdes->number[MDES_SYNC_IN] = num_sync;
  if (mdes->number[MDES_SYNC_OUT] < num_sync)
    mdes->number[MDES_SYNC_OUT] = num_sync;


  /* Get the slot statisitics */
  mdes->max_slot = get_int_parm (md, "max_slot");
  mdes->num_slots = get_int_parm (md, "num_slots");

  /* Currently, max_slot better be num_slots -1 */
  if (mdes->max_slot != (mdes->num_slots - 1))
    I_punt ("load_lmdes_from_version2:\n"
	    "max_slot (%i) expected to be num_slots (%i) -1!",
	    mdes->max_slot, mdes->num_slots);

#if DEBUG_LMDES2
  printf ("Number: pred %i dest %i src %i max_slot %i num_slots %i\n",
	  mdes->number[MDES_PRED], mdes->number[MDES_DEST],
	  mdes->number[MDES_SRC], mdes->max_slot, mdes->num_slots);
#endif

  /* 20030601 SZU
   * Itanium checks.
   * Check that slots_per_template <= num_slots.
   * Check that num_slots % slots_per_template = 0.
   * Check that slots_per_template * max_template_per_issue <= num_slots
   */
  num_slots = mdes->num_slots;
  slots_per_template = mdes->mdes2->sm_mdes->slots_per_template;
  max_template_per_issue = mdes->mdes2->sm_mdes->max_template_per_issue;

#if DEBUG_LMDES2
  printf ("slots_per_template %i max_template_per_issue %i\n",
	  slots_per_template, max_template_per_issue);
#endif

  if (slots_per_template != 0)
    if ((slots_per_template > num_slots) ||
	(num_slots % slots_per_template != 0) ||
	(slots_per_template * max_template_per_issue > num_slots))
      I_punt ("load_lmdes_from_version2: Something wrong w/ relationship betw:"
	      "\nnum_slots %i slots_per_template %i max_template_per_issue %i",
	      num_slots, slots_per_template, max_template_per_issue);

  /*
   * Get total operand count, latency count, and calculate offsets into
   * arrays for pred, dest, src, etc. operands
   */
  mdes->operand_count = mdes->number[MDES_PRED] + mdes->number[MDES_DEST] +
    mdes->number[MDES_SRC];

  mdes->latency_count = mdes->operand_count + mdes->number[MDES_SYNC_IN] +
    mdes->number[MDES_SYNC_OUT];

  mdes->offset[MDES_PRED] = 0;
  mdes->offset[MDES_DEST] = mdes->offset[MDES_PRED] + mdes->number[MDES_PRED];
  mdes->offset[MDES_SRC] = mdes->offset[MDES_DEST] + mdes->number[MDES_DEST];
  mdes->offset[MDES_SYNC_IN] = mdes->offset[MDES_SRC] +
    mdes->number[MDES_SRC];
  mdes->offset[MDES_SYNC_OUT] = mdes->offset[MDES_SYNC_IN] +
    mdes->number[MDES_SYNC_IN];


  /* Set names for each operand type, for error messages */
  mdes->name[MDES_PRED] = "pred";
  mdes->name[MDES_DEST] = "dest";
  mdes->name[MDES_SRC] = "src";
  mdes->name[MDES_SYNC_IN] = "sync_in";
  mdes->name[MDES_SYNC_OUT] = "sync_out";

  /* Malloc structures to allow reverse mapping from operand_index
   * to operand_type and operand_number.
   */
  Malloc_struct ((void **) &mdes->index_type,
		 mdes->latency_count * sizeof (int));
  Malloc_struct ((void **) &mdes->index_number,
		 mdes->latency_count * sizeof (int));

  /* Init reverse map structure for MDES_PRED */
  for (i = 0; i < mdes->number[MDES_PRED]; i++)
    {
      mdes->index_type[mdes->offset[MDES_PRED] + i] = MDES_PRED;
      mdes->index_number[mdes->offset[MDES_PRED] + i] = i;
    }


  /* Init reverse map structure for MDES_DEST */
  for (i = 0; i < mdes->number[MDES_DEST]; i++)
    {
      mdes->index_type[mdes->offset[MDES_DEST] + i] = MDES_DEST;
      mdes->index_number[mdes->offset[MDES_DEST] + i] = i;
    }

  /* Init reverse map structure for MDES_SRC */
  for (i = 0; i < mdes->number[MDES_SRC]; i++)
    {
      mdes->index_type[mdes->offset[MDES_SRC] + i] = MDES_SRC;
      mdes->index_number[mdes->offset[MDES_SRC] + i] = i;
    }

  /* Init reverse map structure for MDES_SYNC_IN */
  for (i = 0; i < mdes->number[MDES_SYNC_IN]; i++)
    {
      mdes->index_type[mdes->offset[MDES_SYNC_IN] + i] = MDES_SYNC_IN;
      mdes->index_number[mdes->offset[MDES_SYNC_IN] + i] = i;
    }

  /* Init reverse map structure for MDES_SYNC_OUT */
  for (i = 0; i < mdes->number[MDES_SYNC_OUT]; i++)
    {
      mdes->index_type[mdes->offset[MDES_SYNC_OUT] + i] = MDES_SYNC_OUT;
      mdes->index_number[mdes->offset[MDES_SYNC_OUT] + i] = i;
    }

  /* Build the IO_Sets structure */
  mdes->num_IO_sets = get_num_entries (md, "Field_Type");
  mdes->IOmask_width = (mdes->num_IO_sets >> 5) + 1;
  mdes->num_reg_files = mdes->num_IO_sets;
  mdes->null_external_id = get_int_value (md, "Field_Type", "NULL", "id", 0);
  mdes->max_IO_set_id = get_int_parm (md, "max_field_type_id");
  total_string_len = calc_total_entry_name_len (md, "Field_Type");

#if DEBUG_LMDES2
  printf ("IOmask_width %i num_IO_sets %i null_external_id %i\n",
	  mdes->IOmask_width, mdes->num_IO_sets, mdes->null_external_id);
  printf ("max_IO_set_id %i total_string_len %i\n",
	  mdes->max_IO_set_id, total_string_len);
#endif

  /* Allocate array of IO_Sets */
  struct_size = mdes->num_IO_sets * sizeof (Mdes_IO_Set);
  Malloc_struct ((void **) &mdes->IO_set, struct_size);

  /* Allocate array to map external_id's to IO_Sets */
  set_table_size = (mdes->max_IO_set_id + 1) * sizeof (Mdes_IO_Set *);
  Malloc_struct ((void **) &mdes->IO_set_table, set_table_size);

  /* Allocate array of IOmasks */
  data_size = mdes->num_IO_sets * (mdes->IOmask_width * sizeof (int));
  Malloc_struct ((void **) &mask_ptr, data_size);

  /* Allocate all the name buffer space in one shot */
  Malloc_struct ((void **) &name_buf_ptr, total_string_len);

  /* Initialize IO_Set_table to NULL pointers */
  for (i = 0; i <= mdes->max_IO_set_id; i++)
    mdes->IO_set_table[i] = NULL;

  /* Read in the IO_sets */
  section = MD_find_section (md, "Field_Type");
  entry = MD_first_entry (section);
  for (i = 0; i < mdes->num_IO_sets; i++)
    {
      IO_set = &mdes->IO_set[i];
      IO_set->name = name_buf_ptr;
      IO_set->mask = mask_ptr;

      IO_set->id = i;

      /* Get the external id */
      if ((field = find_field_by_name (entry, "id")) != NULL)
	IO_set->external_id = MD_get_int (field, 0);
      else
	IO_set->external_id = -1;

      strcpy (IO_set->name, entry->name);

      /* Put in IO_Set_table */
      if (IO_set->external_id != -1)
	mdes->IO_set_table[IO_set->external_id] = IO_set;


      /* Masks are built in a post-pass! */
      /* Move name buf pointer after terminator */
      name_buf_ptr += strlen (IO_set->name) + 1;

      /* Move mask pointer after this mask */
      mask_ptr += mdes->IOmask_width;

      /* Goto next Field_Type entry */
      entry = MD_next_entry (entry);
    }

  /* Build masks for IO_sets */
  section = MD_find_section (md, "Field_Type");
  entry = MD_first_entry (section);
  for (i = 0; i < mdes->num_IO_sets; i++)
    {
      IO_set = &mdes->IO_set[i];

      /* Initialize mask */
      for (index1 = 0; index1 < mdes->IOmask_width; index1++)
	IO_set->mask[index1] = 0;

      /* Build IO_mask */
      field = find_field_by_name (entry, "compatible_ids");
      for (index1 = 0; index1 <= field->max_element_index; index1++)
	{
	  int id, external_id, index;
	  Mdes_IO_Set *target_set;

	  external_id = MD_get_int (field, index1);
	  target_set = mdes->IO_set_table[external_id];
	  id = target_set->id;
	  index = id >> 5;
	  IO_set->mask[index] |= 1 << (id & 0x1f);
	}


#if DEBUG_LMDES2
      printf ("%2i %2i %-20s", IO_set->id, IO_set->external_id, IO_set->name);
      for (index1 = 0; index1 < mdes->IOmask_width; index1++)
	printf (" %08x", IO_set->mask[index1]);
      printf ("\n");
#endif

      /* Goto next Field_Type entry */
      entry = MD_next_entry (entry);
    }

  /* Build IO_Items structures */
  mdes->num_IO_items = get_num_entries (md, "Operation_Format");
  total_string_len = calc_total_entry_name_len (md, "Operation_Format");

#if DEBUG_LMDES2
  printf ("num_IO_items %i total_string_len %i\n",
	  mdes->num_IO_items, total_string_len);
#endif


  /* Allocate array of IO_Items */
  struct_size = mdes->num_IO_items * sizeof (Mdes_IO_Item);
  Malloc_struct ((void **) &mdes->IO_item, struct_size);

  /* Allocate array of operand types */
  data_size = mdes->num_IO_items * mdes->operand_count *
    sizeof (Mdes_IO_Set *);
  Malloc_struct ((void **) &type_ptr, data_size);

  /* Allocate all the name buffer space in one shot */
  Malloc_struct ((void **) &name_buf_ptr, total_string_len);

  section = MD_find_section (md, "Operation_Format");
  entry = MD_first_entry (section);
  null_IO_set = mdes->IO_set_table[mdes->null_external_id];
  for (i = 0; i < mdes->num_IO_items; i++)
    {
      IO_item = &mdes->IO_item[i];
      IO_item->name = name_buf_ptr;
      IO_item->operand_type = type_ptr;

      IO_item->id = i;
      strcpy (IO_item->name, entry->name);

      /* Initialize all types to NULL type first */
      for (j = 0; j < mdes->operand_count; j++)
	IO_item->operand_type[j] = null_IO_set;

      /* Build types from pred, src, and dest fields of entry */
      if ((field = find_field_by_name (entry, "dest")) != NULL)
	{
	  for (index1 = 0; index1 <= field->max_element_index; index1++)
	    {
	      MD_Entry *target_type;
	      int id, index;

	      target_type = MD_get_link (field, index1);

	      /* Find the id of this target type */
	      for (id = 0; id < mdes->num_IO_sets; id++)
		{
		  if (strcmp (target_type->name, mdes->IO_set[id].name) == 0)
		    break;
		}

	      /* Sanity check */
	      if (id >= mdes->num_IO_sets)
		I_punt ("IO_type %s not found!", target_type->name);

	      index = mdes->offset[MDES_DEST] + index1;
	      IO_item->operand_type[index] = &mdes->IO_set[id];
	    }
	}

      if ((field = find_field_by_name (entry, "src")) != NULL)
	{
	  for (index1 = 0; index1 <= field->max_element_index; index1++)
	    {
	      MD_Entry *target_type;
	      int id, index;

	      target_type = MD_get_link (field, index1);

	      /* Find the id of this target type */
	      for (id = 0; id < mdes->num_IO_sets; id++)
		{
		  if (strcmp (target_type->name, mdes->IO_set[id].name) == 0)
		    break;
		}

	      /* Sanity check */
	      if (id >= mdes->num_IO_sets)
		I_punt ("IO_type %s not found!", target_type->name);

	      index = mdes->offset[MDES_SRC] + index1;
	      IO_item->operand_type[index] = &mdes->IO_set[id];
	    }
	}

      if ((field = find_field_by_name (entry, "pred")) != NULL)
	{
	  for (index1 = 0; index1 <= field->max_element_index; index1++)
	    {
	      MD_Entry *target_type;
	      int id, index;

	      target_type = MD_get_link (field, index1);

	      /* Find the id of this target type */
	      for (id = 0; id < mdes->num_IO_sets; id++)
		{
		  if (strcmp (target_type->name, mdes->IO_set[id].name) == 0)
		    break;
		}

	      /* Sanity check */
	      if (id >= mdes->num_IO_sets)
		I_punt ("IO_type %s not found!", target_type->name);

	      index = mdes->offset[MDES_PRED] + index1;
	      IO_item->operand_type[index] = &mdes->IO_set[id];
	    }
	}

#if DEBUG_LMDES2
      printf ("%3i %-15s", IO_item->id, IO_item->name);
      for (j = 0; j < mdes->operand_count; j++)
	printf (" %-2i", IO_item->operand_type[j]->id);
      printf ("\n");
#endif

      /* Move name buf pointer after terminator */
      name_buf_ptr += strlen (IO_item->name) + 1;

      /* Move type ptr to next operand type array */
      type_ptr += mdes->operand_count;

      /* Goto next Operation_Format entry */
      entry = MD_next_entry (entry);
    }

  /* Build Resource structures */
  mdes->num_resources = get_num_entries (md, "Resource");
  total_string_len = calc_total_entry_name_len (md, "Resource");

#if DEBUG_LMDES2
  printf ("Resources %i total string len %i\n", mdes->num_resources,
	  total_string_len);
#endif

  struct_size = mdes->num_resources * sizeof (Mdes_Resource);
  Malloc_struct ((void **) &mdes->resource, struct_size);

  /* Allocate all the name buffer space in one shot */
  Malloc_struct ((void **) &name_buf_ptr, total_string_len);

  section = MD_find_section (md, "Resource");
  entry = MD_first_entry (section);
  for (i = 0; i < mdes->num_resources; i++)
    {
      resource = &mdes->resource[i];
      resource->name = name_buf_ptr;

      resource->id = i;
      strcpy (resource->name, entry->name);

      /* Move name buf pointer after terminator */
      name_buf_ptr += strlen (resource->name) + 1;

#if DEBUG_LMDES2
      printf ("%-3i %s\n", resource->id, resource->name);
#endif

      /* Goto next Resource entry */
      entry = MD_next_entry (entry);
    }

  /* Not using the reslist structures in the mdes */
  mdes->num_reslists = 0;
  mdes->Rmask_width = 1;
  mdes->reslist = NULL;

#if DEBUG_LMDES2
  printf ("No Reslist structures built!\n");
#endif

  /* Build latency structures */
  mdes->num_latencies = get_num_entries (md, "Operation_Latency");
  total_string_len = calc_total_entry_name_len (md, "Operation_Latency");

#if DEBUG_LMDES2
  printf ("Latencies %i total string len %i\n", mdes->num_latencies,
	  total_string_len);
#endif

  /* Allocate array of latencies */
  struct_size = mdes->num_latencies * sizeof (Mdes_Latency);
  Malloc_struct ((void **) &mdes->latency, struct_size);

  /* Allocate all operand_latency arrays in one shot */
  data_size = mdes->num_latencies * mdes->latency_count * sizeof (int);
  Malloc_struct ((void **) &array_ptr, data_size);

  /* Allocate all the name buffer space in one shot */
  Malloc_struct ((void **) &name_buf_ptr, total_string_len);

  section = MD_find_section (md, "Operation_Latency");
  entry = MD_first_entry (section);
  for (i = 0; i < mdes->num_latencies; i++)
    {
      latency = &mdes->latency[i];
      latency->name = name_buf_ptr;
      latency->operand_latency = array_ptr;

      latency->id = i;
      strcpy (latency->name, entry->name);

      /* By default, all latencies 0 */
      for (j = 0; j < mdes->latency_count; j++)
	latency->operand_latency[j] = 0;

      set_latencies (entry, "dest",
		     &latency->operand_latency[mdes->offset[MDES_DEST]]);

      set_latencies (entry, "src",
		     &latency->operand_latency[mdes->offset[MDES_SRC]]);

      set_latencies (entry, "pred",
		     &latency->operand_latency[mdes->offset[MDES_PRED]]);


      set_latencies (entry, "mem_dest",
		     &latency->operand_latency[mdes->offset[MDES_SYNC_OUT] +
					       0]);
      set_latencies (entry, "ctrl_dest",
		     &latency->operand_latency[mdes->offset[MDES_SYNC_OUT] +
					       1]);
      set_latencies (entry, "sync_dest",
		     &latency->operand_latency[mdes->offset[MDES_SYNC_OUT] +
					       2]);

      set_latencies (entry, "mem_src",
		     &latency->operand_latency[mdes->offset[MDES_SYNC_IN] +
					       0]);
      set_latencies (entry, "ctrl_src",
		     &latency->operand_latency[mdes->offset[MDES_SYNC_IN] +
					       1]);
      set_latencies (entry, "sync_src",
		     &latency->operand_latency[mdes->offset[MDES_SYNC_IN] +
					       2]);



      /* Make latency exception to be dest[0] latency! */
      latency->exception = latency->operand_latency[mdes->offset[MDES_DEST]];

#if DEBUG_LMDES2
      printf ("%-3i %-15s %-3i", latency->id, latency->name,
	      latency->exception);
      for (j = 0; j < mdes->latency_count; j++)
	printf (" %-3i", latency->operand_latency[j]);
      printf ("\n");
#endif


      /* Move name buf pointer after terminator */
      name_buf_ptr += strlen (latency->name) + 1;

      /* Move operand latency array ptr to next array */
      array_ptr += mdes->latency_count;

      /* Goto next Operation Latency entry */
      entry = MD_next_entry (entry);
    }


  /* Load the operation's structures */
  mdes->num_operations = get_num_entries (md, "IMPACT_Operation");
  mdes->max_opcode = get_int_parm (md, "max_proc_opc");
  total_alts = calc_num_alts (md);

  /* Will put NULL in for asm names! (may change later) */
  total_string_len = calc_op_total_string_len (md);

#if DEBUG_LMDES2
  printf (" Operations %i total alts %i max_opcode %i total_string_len %i\n",
	  mdes->num_operations, total_alts, mdes->max_opcode,
	  total_string_len);
#endif


  /* Allocate array of Operations */
  struct_size = mdes->num_operations * sizeof (Mdes_Operation);
  Malloc_struct ((void **) &mdes->operation, struct_size);

  /* Allocate all alts in one shot */
  alt_size = total_alts * sizeof (Mdes_Alt);
  Malloc_struct ((void **) &alt_ptr, alt_size);

  /* Allocate op_table (indexed by opcode) */
  data_size = (mdes->max_opcode + 1) * sizeof (Mdes_Operation *);
  Malloc_struct ((void **) &mdes->op_table, data_size);

  /* Allocate all the name buffer space in one shot */
  Malloc_struct ((void **) &name_buf_ptr, total_string_len);


  /* Intialize all op_table pointers to NULL */
  for (i = 0; i <= mdes->max_opcode; i++)
    mdes->op_table[i] = NULL;

  /* Tag formats and latencies with id for easy access.
   * Reservation_Tables already have ids
   */
  SM_add_ids_to_entries (MD_find_section (md, "Operation_Format"));
  SM_add_ids_to_entries (MD_find_section (md, "Operation_Latency"));

  /* Read in operations */
  section = MD_find_section (md, "IMPACT_Operation");
  entry = MD_first_entry (section);
  for (i = 0; i < mdes->num_operations; i++)
    {
      MD_Entry *op_entry, *alt_entry;
      MD_Field *op_field, *alt_field, *format_field;
      int index1, index2, index3;
      /* 20031024 SZU */
      MD_Field *op_alt_field, *op_alt_resv_field, *port_field, *nop_field;
      MD_Entry *op_alt_entry, *op_alt_resv_entry;
      int num_lat_class, op;
      Set lat_class;

      operation = &mdes->operation[i];
      operation->external_name = name_buf_ptr;
      operation->alt = alt_ptr;
      operation->id = i;
      operation->heuristic_alt = -1;	/* Used for heuristics */

      /* Initialize mdes study stats */
      init_stats (&operation->can_sched_prepass);
      init_stats (&operation->sched_prepass);
      init_stats (&operation->can_sched_postpass);
      init_stats (&operation->sched_postpass);


      field = find_field_by_name (entry, "proc_opc");
      operation->opcode = MD_get_int (field, 0);

      strcpy (operation->external_name, entry->name);

      operation->num_alts = calc_num_op_alts (entry);

      field = find_field_by_name (entry, "bit_flags");
      operation->op_flags = MD_get_int (field, 0);

      /* 20030528 SZU
       * Added to look for port and syll info for IMPACT_Operation
       */
      /* Only looks at first entry under op field.
       * Assumes all Operation have same port.
       * Otherwise should have some kind of compatible_with syllable like A.
       * Maybe add check in future.
       */
      op_field = find_field_by_name (entry, "op");
      op_entry = MD_get_link (op_field, 0);

      /* Only looks at first entry under alt field.
       * Assumes all alts have same port.
       * Otherwise should have some kind of compatible_with syllable like A.
       * Maybe add check in future.
       */
      op_alt_field = find_field_by_name (op_entry, "alt");
      op_alt_entry = MD_get_link (op_alt_field, 0);

      /* Get resv field */
      op_alt_resv_field = find_field_by_name (op_alt_entry, "resv");
      op_alt_resv_entry = MD_get_link (op_alt_resv_field, 0);

      /* If port field exists, add port and syllable to Mdes_Operation.
       * Otherwise put 0.
       */
      if (MD_find_field_decl (op_alt_resv_entry->section, "port") != NULL)
	{
	  if ((port_field = find_field_by_name (op_alt_resv_entry, "port")) ==
	      NULL)
	    {
	      operation->syll_mask = 0;
	      operation->port_mask = 0;
	    }
	  else
	    {
	      MD_Entry *port_entry;
	      MD_Field *port_id_field;
	      int port_id;
	      SM_Port *port;

	      /* Get the port, the id, and the mask */
	      port_entry = MD_get_link (port_field, 0);
	      port_id_field = find_field_by_name (port_entry, "id");
	      port_id = MD_get_int (port_id_field, 0);
	      port = &mdes->mdes2->sm_mdes->port_array[port_id];
	      operation->port_mask = port->mask;

	      /* Get the syllable mask and num_slots */
	      operation->syll_mask = port->syllable->mask;
	      operation->num_slots = port->syllable->num_slots;
	    }
	}
      else
	{
	  operation->syll_mask = 0;
	  operation->port_mask = 0;
	}

#if DEBUG_LMDES2
      printf ("Operation %s: port 0x%x syll 0x%x num_slots %i\n",
	      operation->external_name, operation->port_mask,
	      operation->syll_mask, operation->num_slots);
#endif

      /* Look for nop field in op.
       * If exist, add to list of nop opcodes to be added to array.
       */
      if (MD_find_field_decl (entry->section, "nop") != NULL)
	{
	  if ((nop_field = find_field_by_name (entry, "nop")) != NULL)
	    {
	      operation->nop = 1;
	      num_nop++;

	      if ((current_nop = (SM_Nop *) malloc (sizeof (SM_Nop))) == NULL)
		I_punt ("load_lmdes_from_version2: Out of memory");
	      
	      current_nop->priority = MD_get_int (nop_field, 0);
	      current_nop->opcode = operation->opcode;
	      current_nop->prev_nop = NULL;
	      current_nop->next_nop = NULL;

	      /* No nops yet */
	      if (first_nop == NULL)
		{
		  first_nop = current_nop;
		  last_nop = current_nop;
		}
	      else
		{
		  last_nop->next_nop = current_nop;
		  current_nop->prev_nop = last_nop;
		  last_nop = current_nop;
		}
#if DEBUG_LMDES2
	      printf ("Operation %s is a nop: priority %i opcode %i\n",
		      operation->external_name, current_nop->priority,
		      current_nop->opcode);
#endif
	    }
	  else
	    {
	      operation->nop = 0;
	    }
	}
      else
	{
	  operation->nop = 0;
	}

      /* 20020805 SZU
       * Added to keep track of latency classes.
       * Should be done only if Prod_Cons_Latency exist
       */
      if (MD_find_field_decl (entry->section, "lat_class") != NULL)
	{
	  field = find_field_by_name (entry, "lat_class");
	  num_lat_class = field->max_element_index + 1;
	  lat_class = Set_new ();

	  for (index1 = 0; index1 < num_lat_class; index1++)
	    {
	      op = MD_get_int (field, index1);
	      Set_add (lat_class, op);
	    }

	  operation->lat_class = Set_copy (lat_class);
	  Set_dispose (lat_class);
	}

#if DEBUG_LMDES2
      printf ("%4i %-15s %i %08x\n", operation->opcode,
	      operation->external_name, operation->num_alts,
	      operation->op_flags);
#endif

      /* Insert into op_table */
      mdes->op_table[operation->opcode] = operation;

      /* Move name buf pointer after terminator */
      name_buf_ptr += strlen (operation->external_name) + 1;

      /* Read in alts for operation */
      j = 0;
      /* For each assembly op allowed for the operation */
      op_field = find_field_by_name (entry, "op");
      for (index1 = 0; index1 <= op_field->max_element_index; index1++)
	{
	  op_entry = MD_get_link (op_field, index1);

	  /* For each alternative for that assembly alt */
	  alt_field = find_field_by_name (op_entry, "alt");
	  for (index2 = 0; index2 <= alt_field->max_element_index; index2++)
	    {
	      alt_entry = MD_get_link (alt_field, index2);

	      /* For each format for that alt */
	      format_field = find_field_by_name (alt_entry, "format");
	      for (index3 = 0; index3 <= format_field->max_element_index;
		   index3++)
		{
		  MD_Entry *format_entry, *table_entry, *lat_entry;

		  alt = &operation->alt[j];
		  alt->id = j;
		  alt->asm_name = name_buf_ptr;
		  alt->operation = operation;

		  strcpy (alt->asm_name, op_entry->name);

		  field = find_field_by_name (alt_entry, "bit_flags");
		  alt->alt_flags = MD_get_int (field, 0);


		  format_entry = MD_get_link (format_field, index3);
		  field = find_field_by_name (format_entry, "id");
		  IO_item_id = MD_get_int (field, 0);

		  field = find_field_by_name (alt_entry, "resv");
		  table_entry = MD_get_link (field, 0);
		  field = find_field_by_name (table_entry, "id");
		  reslist_id = MD_get_int (field, 0);

		  field = find_field_by_name (alt_entry, "latency");
		  lat_entry = MD_get_link (field, 0);
		  field = find_field_by_name (lat_entry, "id");
		  latency_id = MD_get_int (field, 0);



		  /* Point at appropriate structures using ids */
		  alt->IO_item = &mdes->IO_item[IO_item_id];
		  alt->reslist = NULL;	/* Since load .lmdes2 not .lmdes */
		  alt->latency = &mdes->latency[latency_id];

		  /* Point at sm_mdes table structure for reslist */
		  alt->table = &mdes->mdes2->sm_mdes->table_array[reslist_id];

#if DEBUG_LMDES2
		  printf ("  %-15s %08x %2s %i (%i choices) %2s\n",
			  alt->asm_name, alt->alt_flags,
			  alt->IO_item->name, reslist_id,
			  alt->table->num_choices, alt->latency->name);
#endif

		  /* Move name buf pointer after terminator */
		  name_buf_ptr += strlen (alt->asm_name) + 1;

		  /* Update j (alt id) */
		  j++;
		}
	    }
	}

      /* Move alt ptr to next alt array */
      alt_ptr += operation->num_alts;

      /* Go to next IMPACT_Operation entry */
      entry = MD_next_entry (entry);
    }

  /* 20030603 SZU
   * Make nop array w/ nop linked list structure
   */
  if (num_nop > 0)
    {
      mdes->mdes2->sm_mdes->max_nop_index = num_nop - 1;

      if ((nop_array = (int *) malloc (num_nop * sizeof (int))) == NULL)
	I_punt ("load_lmdes_from_version2: Out of memory");

      for (i = 0; i < num_nop; i++)
	nop_array[i] = -1;

      for (current_nop = first_nop; current_nop != NULL;
	   current_nop = current_nop->next_nop)
	nop_array[current_nop->priority] = current_nop->opcode;

#if DEBUG_LMDES2
      printf ("Printing nop_array: num_nop %i\n", num_nop);
      for (i = 0; i < num_nop; i++)
	printf ("entry %i: %i\n", i, nop_array[i]);
#endif

      /* Sanity check */
      for (i = 0; i < num_nop; i++)
	if (nop_array[i] < 0)
	  I_punt ("load_lmdes_from_version2: bad nop specifications!");

      mdes->mdes2->sm_mdes->nop_array = nop_array;
    }
  else
    {
      /* No NOP specified */
      mdes->mdes2->sm_mdes->nop_array = NULL;
    }

  /* Allocate temporary buffers used by Mdes routines */
  buf_size = mdes->operand_count * sizeof (Mdes_IO_Set *);
  Malloc_struct ((void **) &mdes->operand_type_buf, buf_size);

  /* Get MDES2 specific parms.  See header file for description
   * of these parameters.
   */
  mdes->check_resources_for_only_one_alt =
    get_binary_parm (md, "check_resources_for_only_one_alt");

  return (mdes);
}

static void
init_stats (Mdes_Stats * stats)
{
  stats->num_oper_checks = 0;
  stats->num_oper_checks_failed = 0;
  stats->num_table_checks = 0;
  stats->num_table_checks_failed = 0;
  stats->num_option_checks = 0;
  stats->num_option_checks_failed = 0;
  stats->num_slot_checks = 0;
  stats->num_slot_checks_failed = 0;
  stats->num_usage_checks = 0;
  stats->num_usage_checks_failed = 0;
  stats->first_choice_dist = INT_new_symbol_table ("first_choice_dist", 0);
  stats->num_choice_dist = INT_new_symbol_table ("num_choice_dist", 0);
  stats->succeed_option_check_dist =
    INT_new_symbol_table ("succeed_option_check_dist", 0);
  stats->fail_option_check_dist =
    INT_new_symbol_table ("fail_option_check_dist", 0);
  stats->succeed_usage_check_dist =
    INT_new_symbol_table ("succeed_usage_check_dist", 0);
  stats->fail_usage_check_dist =
    INT_new_symbol_table ("fail_usage_check_dist", 0);
}

/* Add 'inc' to the check history for num_checks */
void
increment_check_history (INT_Symbol_Table * table, int num_checks, int inc)
{
  INT_Symbol *symbol;

  /* Get symbol for this number of checks, create if necessary */
  if ((symbol = INT_find_symbol (table, num_checks)) == NULL)
    {
      symbol = INT_add_symbol (table, num_checks, (void *) 0);
    }

  /* Increment count by 'inc' */
#ifdef LP64_ARCHITECTURE
  symbol->data = (void *) ((long)(((int)((long) symbol->data)) + inc));
#else
  symbol->data = (void *) (((int) symbol->data) + inc);
#endif
}

static void
add_check_history (INT_Symbol_Table * dest, INT_Symbol_Table * src)
{
  INT_Symbol *symbol;

  for (symbol = src->head_symbol; symbol != NULL;
       symbol = symbol->next_symbol)
    {
#ifdef LP64_ARCHITECTURE
      increment_check_history (dest, symbol->value, (int)((long)symbol->data));
#else
      increment_check_history (dest, symbol->value, (int) symbol->data);
#endif
    }
}

static void
add_stats (Mdes_Stats * total, Mdes_Stats * add)
{
  total->num_oper_checks += add->num_oper_checks;
  total->num_oper_checks_failed += add->num_oper_checks_failed;

  total->num_table_checks += add->num_table_checks;
  total->num_table_checks_failed += add->num_table_checks_failed;

  total->num_option_checks += add->num_option_checks;
  total->num_option_checks_failed += add->num_option_checks_failed;

  total->num_slot_checks += add->num_slot_checks;
  total->num_slot_checks_failed += add->num_slot_checks_failed;

  total->num_usage_checks += add->num_usage_checks;
  total->num_usage_checks_failed += add->num_usage_checks_failed;

  add_check_history (total->first_choice_dist, add->first_choice_dist);

  add_check_history (total->num_choice_dist, add->num_choice_dist);

  add_check_history (total->succeed_option_check_dist,
		     add->succeed_option_check_dist);

  add_check_history (total->fail_option_check_dist,
		     add->fail_option_check_dist);

  add_check_history (total->succeed_usage_check_dist,
		     add->succeed_usage_check_dist);

  add_check_history (total->fail_usage_check_dist,
		     add->fail_usage_check_dist);
}

void
print_check_history (FILE * out, char *name, char *type,
		     INT_Symbol_Table * table)
{
  INT_Symbol *symbol;
  Heap *heap;
  int total_checks;

  total_checks = 0;

  /* Create heap */
  heap = Heap_Create (HEAP_MIN);

  for (symbol = table->head_symbol; symbol != NULL;
       symbol = symbol->next_symbol)
    {
      Heap_Insert (heap, (void *) symbol, (double) symbol->value);
#ifdef LP64_ARCHITECTURE
      total_checks += (int)((long) symbol->data);
#else
      total_checks += (int) symbol->data;
#endif
    }


  fprintf (out, "%s %s check history:\n", name, type);

  /* Return now if history empty */
  if (total_checks == 0)
    {
      fprintf (out, "\n");
      return;
    }

  while ((symbol = (INT_Symbol *) Heap_ExtractTop (heap)) != NULL)
    {
#ifdef LP64_ARCHITECTURE
      fprintf (out, "  %4i  %8i  %5.2f%%\n", symbol->value,
	       (int)((long) symbol->data),
	       (double) 100.0 * ((double) ((int)((long) symbol->data))) /
	       ((double) total_checks));
#else
      fprintf (out, "  %4i  %8i  %5.2f%%\n", symbol->value,
	       (int) symbol->data,
	       (double) 100.0 * ((double) ((int) symbol->data)) /
	       ((double) total_checks));
#endif
    }
  fprintf (out, "\n");
}

void
print_op_stats (FILE * out, char *name,
		Mdes_Stats * can_stats, Mdes_Stats * do_stats)
{
  int total_checks, total_checks_failed;
  double tries_per_op, checks_per_op, fails_per_op;
  double checks_per_try, fails_per_try, options_per_try;
  double checks_per_option;

  tries_per_op = ((double) can_stats->num_oper_checks) /
    ((double) do_stats->num_oper_checks);

  /* Count only usage checks, but fails for usage or slot constraints */
  total_checks = can_stats->num_usage_checks;
  checks_per_op = ((double) total_checks) /
    ((double) do_stats->num_oper_checks);
  checks_per_try = ((double) total_checks) /
    ((double) can_stats->num_oper_checks);
  checks_per_option = ((double) total_checks) /
    ((double) can_stats->num_option_checks);

  options_per_try = ((double) can_stats->num_option_checks) /
    ((double) can_stats->num_oper_checks);



  total_checks_failed =
    can_stats->num_usage_checks_failed + can_stats->num_slot_checks_failed;
  fails_per_op = ((double) total_checks_failed) /
    ((double) do_stats->num_oper_checks);
  fails_per_try = ((double) total_checks_failed) /
    ((double) can_stats->num_oper_checks);

  /* For Micro 29 */
  fprintf (out, "%-12s %10i %10.2f %10.2f %10.2f %10.2f\n",
	   name, do_stats->num_oper_checks,
	   tries_per_op, options_per_try, checks_per_try, checks_per_option);
/*    
    fprintf (out, "%-25s %7i %4.1f %4.1f/%4.1f %4.1f/%4.1f\n",
	     name,
	     do_stats->num_oper_checks,
	     tries_per_op,
	     checks_per_op, checks_per_try,
	     fails_per_op, fails_per_try);
*/
}

void
print_mdes_stats (FILE * out, int prepass)
{
  Mdes_Stats *can_stats, *do_stats;
  Mdes_Operation *op;
  int opc;
  Mdes_Stats total_can_stats, total_do_stats;
  Mdes_Stats uncond_can_stats, uncond_do_stats;
  Mdes_Stats cbr_can_stats, cbr_do_stats;
  Mdes_Stats load_can_stats, load_do_stats;
  Mdes_Stats store_can_stats, store_do_stats;
  Mdes_Stats other_can_stats, other_do_stats;
  int total_checks;
  Heap *heap;

  /* Only can print stats for mdes version2! */
  if (lmdes->mdes2 == NULL)
    {
      fprintf (out, "Version 1 mdes, stats not printed\n");
      return;
    }

  init_stats (&total_can_stats);
  init_stats (&total_do_stats);

  init_stats (&uncond_can_stats);
  init_stats (&uncond_do_stats);
  init_stats (&cbr_can_stats);
  init_stats (&cbr_do_stats);
  init_stats (&load_can_stats);
  init_stats (&load_do_stats);
  init_stats (&store_can_stats);
  init_stats (&store_do_stats);
  init_stats (&other_can_stats);
  init_stats (&other_do_stats);

  /* Create heap */
  heap = Heap_Create (HEAP_MAX);

  /* Place each non-zero weighted item on heap, sort by total checks */
  for (opc = 0; opc <= lmdes->max_opcode; opc++)
    {
      op = lmdes->op_table[opc];

      /* Skip unused opcodes */
      if (op == NULL)
	continue;

      /* Skip unexecuted opcodes */
      if (prepass)
	{
	  can_stats = &op->can_sched_prepass;
	  do_stats = &op->sched_prepass;
	}
      else
	{
	  can_stats = &op->can_sched_postpass;
	  do_stats = &op->sched_postpass;
	}

      if ((can_stats->num_oper_checks == 0) &&
	  (do_stats->num_oper_checks == 0))
	continue;


      total_checks = can_stats->num_slot_checks + can_stats->num_usage_checks;

      Heap_Insert (heap, (void *) op,
		   (double) (total_checks) - ((double) opc / 1000.0));


      add_stats (&total_can_stats, can_stats);
      add_stats (&total_do_stats, do_stats);

      if ((op->op_flags & OP_FLAG_JMP) ||
	  (op->op_flags & OP_FLAG_RTS) || (op->op_flags & OP_FLAG_JSR))
	{
	  add_stats (&uncond_can_stats, can_stats);
	  add_stats (&uncond_do_stats, do_stats);
	}
      else if (op->op_flags & OP_FLAG_CBR)
	{
	  add_stats (&cbr_can_stats, can_stats);
	  add_stats (&cbr_do_stats, do_stats);
	}
      else if (op->op_flags & OP_FLAG_LOAD)
	{
	  add_stats (&load_can_stats, can_stats);
	  add_stats (&load_do_stats, do_stats);
	}
      else if (op->op_flags & OP_FLAG_STORE)
	{
	  add_stats (&store_can_stats, can_stats);
	  add_stats (&store_do_stats, do_stats);
	}
      else
	{
	  add_stats (&other_can_stats, can_stats);
	  add_stats (&other_do_stats, do_stats);
	}

    }

  /* Print summary stats */
  fprintf (out,
	   "  Category     # Ops     tries/op   opts/try  checks/try checks/opt"
	   "\n");
  fprintf (out,
	   "------------ ---------- ---------- ---------- ---------- ----------"
	   "\n");
  print_op_stats (out, "Jmp/Jsr/Rts", &uncond_can_stats, &uncond_do_stats);
  print_op_stats (out, "Cbr", &cbr_can_stats, &cbr_do_stats);
  print_op_stats (out, "Load", &load_can_stats, &load_do_stats);
  print_op_stats (out, "Store", &store_can_stats, &store_do_stats);
  print_op_stats (out, "Other", &other_can_stats, &other_do_stats);

  fprintf (out, "\n");
  print_op_stats (out, "Total", &total_can_stats, &total_do_stats);
  fprintf (out, "\n");

  /* Print detailed stats */
  /* Pull op's off heap in order and print */
  while ((op = (Mdes_Operation *) Heap_ExtractTop (heap)) != NULL)
    {
      if (prepass)
	{
	  can_stats = &op->can_sched_prepass;
	  do_stats = &op->sched_prepass;
	}
      else
	{
	  can_stats = &op->can_sched_postpass;
	  do_stats = &op->sched_postpass;
	}

      print_op_stats (out, op->external_name, can_stats, do_stats);
    }

  print_check_history (out, "Total", "static first choice",
		       total_do_stats.first_choice_dist);
  print_check_history (out, "Total", "static num choice",
		       total_do_stats.num_choice_dist);

  print_check_history (out, "Total", "dynamic first choice",
		       total_can_stats.first_choice_dist);
  print_check_history (out, "Total", "dynamic num choice",
		       total_can_stats.num_choice_dist);

  print_check_history (out, "Total", "succeed option check",
		       total_can_stats.succeed_option_check_dist);
  print_check_history (out, "Total", "fail option check",
		       total_can_stats.fail_option_check_dist);
  print_check_history (out, "Total", "succeed usage check",
		       total_can_stats.succeed_usage_check_dist);
  print_check_history (out, "Total", "fail usage check",
		       total_can_stats.fail_usage_check_dist);
}

#if 0
/* To debug mdes and sm library */
int
main (int argc, char **argv)
{
  char *file_name;

  if (argc != 2)
    {
      printf ("Usage: %s mdes_file.lmdes2\n", argv[0]);
      exit (1);
    }

  file_name = argv[1];

  printf ("Loading %s:\n", file_name);

  L_init_lmdes (file_name);

  printf ("\nDone\n");
  return (0);
}
#endif
