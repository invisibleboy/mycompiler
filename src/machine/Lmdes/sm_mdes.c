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
 *      File:   sm_mdes.c
 *      Author: John C. Gyllenhaal, Wen-mei Hwu
 *      Creation Date:  March 1996
\*****************************************************************************/


/* 10/29/02 REK Adding config.h */
#include <config.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include "sm_mdes.h"
#include "mdes2.h"
#include "lmdes2.h"
#include <library/l_alloc_new.h>
#include <library/i_error.h>

#define DEBUG_MDES 0

static L_Alloc_Pool *SM_Mdes_pool = NULL;

int
SM_add_ids_to_entries (MD_Section * section)
{
  MD_Entry *entry;
  MD_Field_Decl *id_field_decl;
  MD_Field *id_field;
  int id;

  /* Punt if there is already a field declared */
  if ((id_field_decl = MD_find_field_decl (section, "id")) != NULL)
    I_punt ("SM_add_ids_to_entries: Section '%s' already has id field!",
	    section->name);

  /* Create required id field with one INT */
  id_field_decl = MD_new_field_decl (section, "id", MD_REQUIRED_FIELD);
  MD_require_int (id_field_decl, 0);

  /* Add ids to each field, starting with 0 */
  id = 0;
  for (entry = MD_first_entry (section); entry != NULL;
       entry = MD_next_entry (entry))
    {
      /* Create id field */
      id_field = MD_new_field (entry, id_field_decl, 1);
      MD_set_int (id_field, 0, id);

      id++;
    }

  /* Return the number of entries in this section */
  return (id);
}

void
SM_build_resources (SM_Mdes * sm_mdes)
{
  SM_Resource *resource, *resource_array;
  MD_Section *resource_section;
  MD_Entry *resource_entry;
  MD_Field_Decl *map_location_field_decl;
  MD_Field *map_location_field;
  int num_resources, size, index, id;
  int time_shift, max_offset;

  /* Get the resources section and the decl for map_location */
  resource_section = MD_find_section (sm_mdes->md_mdes, "Resource");
  map_location_field_decl = MD_find_field_decl (resource_section,
						"map_location");

  /* Add id's to each Resource entry (used to index array) 
   * and get the number of entries in the Resource section.
   */
  num_resources = SM_add_ids_to_entries (resource_section);

  /* Malloc an array of resources to hold info about each resource */
  size = num_resources * sizeof (SM_Resource);
  if ((resource_array = (SM_Resource *) malloc (size)) == NULL)
    I_punt ("SM_build_resources: Out of memory");

  /* Copy contents of resource section into resource array,
   * array indexed by id of entry (numbered from 0 above)
   */
  id = 0;
  max_offset = -1;
  for (resource_entry = MD_first_entry (resource_section);
       resource_entry != NULL;
       resource_entry = MD_next_entry (resource_entry))
    {
      /* Get pointer to resource we are building */
      resource = &resource_array[id];

      /* Copy the resource's name */
      resource->name = strdup (resource_entry->name);

      /* Copy the map location information to resource array */
      map_location_field = MD_find_field (resource_entry,
					  map_location_field_decl);

      /* Get the resource base offset into resource map */
      resource->offset = MD_get_int (map_location_field, 0);

      if (resource->offset > max_offset)
	max_offset = resource->offset;

      /* Place 1 at bit location specified by resource's index */
      index = MD_get_int (map_location_field, 1);
      resource->mask = (1 << index);

#if DEBUG_MDES
      printf ("Resource %s: %i offset %x mask\n", resource->name,
	      resource->offset, resource->mask);
#endif

      /* Increment the id */
      id++;
    }

  /* Sanity check */
  if (id != num_resources)
    {
      I_punt ("SM_build_resources: id (%i) and # resources (%i) should agree",
	      id, num_resources);
    }

  /* Calculate how many words/cycle is needed in the resource map
   * Always make a power of two so shifts can be used!
   * Store the amount to shift in sm_mdes->time_shift.
   */
  time_shift = 0;
  while ((max_offset + 1) > (1 << time_shift))
    time_shift++;

  sm_mdes->map_width = 1 << time_shift;
  sm_mdes->time_shift = time_shift;

#if 0
  sm_mdes->time_shift += 0;
  printf ("Increasing time shift to %i\n", sm_mdes->time_shift);
#endif


  /* Point sm_mdes at resource array */
  sm_mdes->resource = resource_array;
  sm_mdes->num_resources = num_resources;
}

void
SM_build_units (SM_Mdes * sm_mdes)
{
  SM_Resource *resource_array, *resource;
  SM_Option *unit_array, *unit;
  SM_Usage *usage_array, *usage;
  MD_Section *unit_section, *usage_section, *resource_section;
  MD_Entry *unit_entry, *ru_entry, *resource_entry;
  MD_Field_Decl *unit_use_field_decl, *resource_id_field_decl;
  MD_Field_Decl *ru_use_field_decl, *ru_time_field_decl;
  MD_Field *unit_use_field, *ru_use_field, *ru_time_field;
  MD_Field *resource_id_field;
  int num_units, array_size, num_usages;
  int index1, index2, id;
  int resources_used, map_offset, resource_offset;
  int usage_time, resource_id, time_shift;

  /* Get resource array for speed and ease of use */
  resource_array = sm_mdes->resource;

  /* Get the amount to shift the time of usage for ease of use */
  time_shift = sm_mdes->time_shift;

  /* Get the unit section and the use decl for use */
  unit_section = MD_find_section (sm_mdes->md_mdes, "Resource_Unit");
  unit_use_field_decl = MD_find_field_decl (unit_section, "use");

  /* Get the usage section and decls for ease of use */
  usage_section = MD_find_section (sm_mdes->md_mdes, "Resource_Usage");
  ru_use_field_decl = MD_find_field_decl (usage_section, "use");
  ru_time_field_decl = MD_find_field_decl (usage_section, "time");

  /* Get the resource section and id decl for ease of use */
  resource_section = MD_find_section (sm_mdes->md_mdes, "Resource");
  resource_id_field_decl = MD_find_field_decl (resource_section, "id");

  /* Add id's to each unit entry (used to index array) 
   * and get the number of entries in the Resource_Unit section.
   */
  num_units = SM_add_ids_to_entries (unit_section);

  /* Malloc an array of units to hold info about each resource unit */
  array_size = num_units * sizeof (SM_Option);
  if ((unit_array = (SM_Option *) malloc (array_size)) == NULL)
    I_punt ("SM_build_units: Out of memory");

  /* Use contents of the resource unit section (with Resource_Usage
   * and Resource links used) to the unit array.
   * Index array by id (numbered from 0 by SM_add_ids_to_entries)
   */
  id = 0;
  for (unit_entry = MD_first_entry (unit_section);
       unit_entry != NULL; unit_entry = MD_next_entry (unit_entry))
    {
      /* Get pointer to unit we are building */
      unit = &unit_array[id];

#if DEBUG_MDES
      printf ("Building %s:\n", unit_entry->name);
#endif

      /* Get the unit's use field */
      unit_use_field = MD_find_field (unit_entry, unit_use_field_decl);

      /* Create an array to hold all the usages in the use field */
      num_usages = unit_use_field->max_element_index + 1;
      array_size = num_usages * sizeof (SM_Usage);
      if ((usage_array = (SM_Usage *) malloc (array_size)) == NULL)
	I_punt ("SM_build_units: Out of memory");

      /* Build each usage in the array */
      for (index1 = 0; index1 < num_usages; index1++)
	{
	  /* Get pointer to usage we are building */
	  usage = &usage_array[index1];

	  /* Get the ru_entry for this usage */
	  ru_entry = MD_get_link (unit_use_field, index1);

	  /* Get the time of this usage */
	  ru_time_field = MD_find_field (ru_entry, ru_time_field_decl);
	  usage_time = MD_get_int (ru_time_field, 0);

	  /* Build the bit mask for the resource(s) used by this usage */
	  resources_used = 0;
	  resource_offset = -1;
	  ru_use_field = MD_find_field (ru_entry, ru_use_field_decl);
	  for (index2 = 0; index2 <= ru_use_field->max_element_index;
	       index2++)
	    {
	      /* Get the resource entry at this index */
	      resource_entry = MD_get_link (ru_use_field, index2);

	      /* Get the id of this resource entry */
	      resource_id_field = MD_find_field (resource_entry,
						 resource_id_field_decl);
	      resource_id = MD_get_int (resource_id_field, 0);

	      /* Get the resource from this id */
	      resource = &resource_array[resource_id];

	      /* Or in the bit for this resource */
	      resources_used |= resource->mask;

	      /* If this is the first resource added, get resource offset 
	       * from it, otherwise check for consistency
	       */
	      if (index2 == 0)
		resource_offset = resource->offset;
	      else if (resource_offset != resource->offset)
		{
		  I_punt ("SM_build_units: Offset mismatch (%i != %i)",
			  resource_offset, resource->offset);
		}
	    }

	  /* Calculate the map offset, uses the width of the map 
	   * (encoded into the map shift) and the offset of the usage.
	   */
	  map_offset = (usage_time << time_shift) + resource_offset;

	  /* Save into to usage */
	  usage->resources_used = resources_used;
	  usage->map_offset = map_offset;

#if DEBUG_MDES
	  printf ("  %x at offset %i\n", resources_used, map_offset);
#endif
	}

      /* Point unit at begining and end of usage */
      unit->first_usage = usage_array;
      unit->last_usage = &usage_array[num_usages - 1];

      /* Increment the id */
      id++;
    }

  /* Sanity check */
  if (id != num_units)
    I_punt ("SM_build_units: id (%i) and num_units (%i) should agree!",
	    id, num_units);


  /* Point sm_mdes at unit array and set its size */
  sm_mdes->unit_array = unit_array;
  sm_mdes->unit_array_size = num_units;
}

void
SM_build_choices (SM_Mdes * sm_mdes)
{
  SM_Choice *choice_array, *choice;
  SM_Option *option_array, *option, *unit;
  MD_Section *option_section, *unit_section;
  MD_Entry *option_entry, *unit_entry;
  MD_Field_Decl *one_of_field_decl, *unit_id_field_decl;
  MD_Field *one_of_field, *unit_id_field;
  int num_choices, num_options, unit_id;
  int id, array_size, index1;

  /* Get the unit section and id field decl for ease of use */
  unit_section = MD_find_section (sm_mdes->md_mdes, "Resource_Unit");
  unit_id_field_decl = MD_find_field_decl (unit_section, "id");

  /* Get the option section and one_of field decl for ease of use 
   * Used to build SM_Choice array.
   */
  option_section = MD_find_section (sm_mdes->md_mdes, "Table_Option");
  one_of_field_decl = MD_find_field_decl (option_section, "one_of");

  /* Add id's to each Option entry (used to index array)
   * and get the number of entries in the Option section.
   */
  num_choices = SM_add_ids_to_entries (option_section);

  /* Malloc an array to hold choice info */
  array_size = num_choices * sizeof (SM_Choice);
  if ((choice_array = (SM_Choice *) malloc (array_size)) == NULL)
    I_punt ("SM_build_choices: Out of memory");

  /* Build choice array using option_section entry info */
  id = 0;
  for (option_entry = MD_first_entry (option_section); option_entry != NULL;
       option_entry = MD_next_entry (option_entry))
    {
      /* Get pointer to the choice we are building */
      choice = &choice_array[id];

#if DEBUG_MDES
      printf ("Building choice %s:\n", option_entry->name);
#endif

      /* Get the option's one_of field */
      one_of_field = MD_find_field (option_entry, one_of_field_decl);

      /* Create an array to hold all the units in the one_of field */
      num_options = one_of_field->max_element_index + 1;
      array_size = num_options * sizeof (SM_Option);
      if ((option_array = (SM_Option *) malloc (array_size)) == NULL)
	I_punt ("SM_build_choices: Out of memory");

      /* Build the options in the array */
      for (index1 = 0; index1 < num_options; index1++)
	{
	  /* Get pointer to the option we are building */
	  option = &option_array[index1];

	  /* Get pointer to this option's unit */
	  unit_entry = MD_get_link (one_of_field, index1);
	  unit_id_field = MD_find_field (unit_entry, unit_id_field_decl);
	  unit_id = MD_get_int (unit_id_field, 0);
	  unit = &sm_mdes->unit_array[unit_id];

	  /* Point the option at the unit's usages */
	  option->first_usage = unit->first_usage;
	  option->last_usage = unit->last_usage;
#if DEBUG_MDES
	  printf ("  Option %i: unit %s,  offset %i to %i\n",
		  index1,
		  unit_entry->name,
		  option->first_usage->map_offset,
		  option->last_usage->map_offset);
#endif
	}

      /* Set the bounds of the option array in this choice */
      choice->first_option = option_array;
      choice->last_option = &option_array[num_options - 1];

      /* Update id */
      id++;
    }

  /* Sanity check */
  if (id != num_choices)
    I_punt ("SM_build_choices: id (%i) and num_choices (%i) should agree!",
	    id, num_choices);


  /* Point sm_mdes at choice array and set its size */
  sm_mdes->choice_array = choice_array;
  sm_mdes->choice_array_size = num_choices;
}

void
SM_build_tables (SM_Mdes * sm_mdes)
{
  SM_Table *table_array, *table;
  SM_Choice *choice_array, *choice, *target_choice;
  MD_Section *table_section, *option_section, *unit_section;
  MD_Entry *table_entry, *option_entry, *unit_entry, *slot_specifier;
  MD_Field_Decl *one_of_field_decl, *option_id_field_decl;
  MD_Field_Decl *table_use_field_decl, *slot_specifier_field_decl;
  MD_Field_Decl *slot_field_decl, *max_time_field_decl;
  MD_Field *one_of_field, *option_id_field;
  MD_Field *table_use_field, *slot_specifier_field, *max_time_field;
  MD_Field *slot_field;
  unsigned short *slot_options;
  int num_tables, num_choices, num_options, option_id;
  int id, array_size, index1, slot_index;
  unsigned int max_usage_time, time_shift;
  int max_num_choices;


  /* Get the table section and use field decl for ease of use 
   * Used to build SM_Table array.
   */
  table_section = MD_find_section (sm_mdes->md_mdes, "Reservation_Table");
  table_use_field_decl = MD_find_field_decl (table_section, "use");
  slot_specifier_field_decl = MD_find_field_decl (table_section,
						  "slot_specifier");
  max_time_field_decl = MD_find_field_decl (table_section, "max_usage_time");

  /* Get the option section and id and one_of field decl for ease of use */
  option_section = MD_find_section (sm_mdes->md_mdes, "Table_Option");
  option_id_field_decl = MD_find_field_decl (option_section, "id");
  one_of_field_decl = MD_find_field_decl (option_section, "one_of");

  /* Get the unit section and the slot field for ease of use */
  unit_section = MD_find_section (sm_mdes->md_mdes, "Resource_Unit");
  slot_field_decl = MD_find_field_decl (unit_section, "slot");

  /* Get the time shift for max_usage_times's */
  time_shift = sm_mdes->time_shift;

  /* Add id's to each Reservation Table entry (used to index array)
   * and get the number of entries in the Reservation Table section.
   */
  num_tables = SM_add_ids_to_entries (table_section);

  /* Malloc an array to hold table info */
  array_size = num_tables * sizeof (SM_Table);
  if ((table_array = (SM_Table *) malloc (array_size)) == NULL)
    I_punt ("SM_build_tables: Out of memory");

  /* Calculate the max number of choices, so the schedule manager
   * can always create a choice array big enough for any table.
   */
  max_num_choices = 0;

  /* Build table array using table_section entry info */
  id = 0;
  for (table_entry = MD_first_entry (table_section); table_entry != NULL;
       table_entry = MD_next_entry (table_entry))
    {
      /* Get pointer to the table we are building */
      table = &table_array[id];

#if DEBUG_MDES
      printf ("Building table %s:\n", table_entry->name);
#endif

      /* Get the table's use field */
      table_use_field = MD_find_field (table_entry, table_use_field_decl);

      /* Get the slot specifier (option entry pointer) */
      slot_specifier_field = MD_find_field (table_entry,
					    slot_specifier_field_decl);
      slot_specifier = MD_get_link (slot_specifier_field, 0);

      /* Create an array to hold all the choices in the use field */
      num_choices = table_use_field->max_element_index + 1;
      array_size = num_choices * sizeof (SM_Choice);
      if ((choice_array = (SM_Choice *) malloc (array_size)) == NULL)
	I_punt ("SM_build_tables: Out of memory");

      /* Update max_num_choices */
      if (num_choices > max_num_choices)
	max_num_choices = num_choices;

      /* Point each choice at the appropriate array.  Also identify
       * choice that specifies the slot
       */
      slot_index = -1;
      for (index1 = 0; index1 < num_choices; index1++)
	{
	  /* Get pointer to the choice we are building */
	  choice = &choice_array[index1];

	  /* Get pointer to this choice's option entry */
	  option_entry = MD_get_link (table_use_field, index1);
	  option_id_field = MD_find_field (option_entry,
					   option_id_field_decl);
	  option_id = MD_get_int (option_id_field, 0);
	  target_choice = &sm_mdes->choice_array[option_id];

	  /* Point the option at the unit's usages */
	  choice->first_option = target_choice->first_option;
	  choice->last_option = target_choice->last_option;

#if DEBUG_MDES
	  printf ("  Choice %i: option %s id %i %x to %x (%i options)\n",
		  index1,
		  option_entry->name, option_id,
		  choice->first_option, choice->last_option,
		  (choice->last_option - choice->first_option) + 1);
#endif

	  /* Is this the choice that specifies the slot */
	  if (option_entry == slot_specifier)
	    {
	      slot_index = index1;
#if DEBUG_MDES
	      printf ("    Slot specifier %s found.\n", slot_specifier->name);
#endif
	    }

	}
      table->first_choice = choice_array;
      table->last_choice = &choice_array[num_choices - 1];
      table->num_choices = num_choices;

      /* Make sure a slot was specified */
      if (slot_index == -1)
	{
	  I_punt ("SM_build_tables: slot specifier %s not found in %s!\n",
		  slot_specifier->name, table_entry->name);
	}
      table->slot_choice = &choice_array[slot_index];

      /* Build the slot_options array (specifies slot used based on
       * the option chosen in slot_choice.
       */
      one_of_field = MD_find_field (slot_specifier, one_of_field_decl);
      num_options = one_of_field->max_element_index + 1;
      array_size = num_options * sizeof (unsigned short);
      if ((slot_options = (unsigned short *) malloc (array_size)) == NULL)
	I_punt ("SM_build_tables: Out of memory");

#if DEBUG_MDES
      printf ("    Slot options:");
#endif
      /* Get the slot from each option's unit entry */
      for (index1 = 0; index1 <= one_of_field->max_element_index; index1++)
	{
	  unit_entry = MD_get_link (one_of_field, index1);
	  slot_field = MD_find_field (unit_entry, slot_field_decl);
	  slot_options[index1] = (unsigned short) MD_get_int (slot_field, 0);
#if DEBUG_MDES
	  printf (" %i", slot_options[index1]);
#endif
	}
#if DEBUG_MDES
      printf ("\n");
#endif

      /* Point table at slot option array */
      table->slot_options = slot_options;

      /* Get the max_usage_time for this table */
      max_time_field = MD_find_field (table_entry, max_time_field_decl);
      max_usage_time = MD_get_int (max_time_field, 0);

      /* Calculate the max usage offset using the resource time_shift.
       * Want to get the offset for the last element at that time.
       * To do this, find the first element of time + 1, and subtract 1.
       * This turns 3 with time shift 1, into ((3 + 1) << 1) -1 = 7
       */
      table->max_usage_offset = ((max_usage_time + 1) << time_shift) - 1;

#if DEBUG_OPTI
      printf (" %s max_usage_time = %i\n", table_entry->name,
	      table->max_usage_time);
#endif

      /* Update id */
      id++;
    }

  /* Sanity check */
  if (id != num_tables)
    I_punt ("SM_build_tables: id (%i) and num_tables (%i) should agree!",
	    id, num_tables);

  /* Point sm_mdes at table array and set its size */
  sm_mdes->table_array = table_array;
  sm_mdes->table_array_size = num_tables;

  /* Write the maximumn number of choices across all tables to sm_mdes */
  sm_mdes->max_num_choices = max_num_choices;
}

/* 20030527 SZU
 * Function reads in the Syllable_Type section in mdes.
 * Assigns an index and power of 2 hex mask to each entry.
 * Entries with a compatible_with field are given the AND of all hex mask.
 * Record the top mask value in sm_mdes.
 */
void
SM_build_syllables (SM_Mdes * sm_mdes)
{
  MD_Section *syll_section;
  MD_Field_Decl *num_slots_field_decl, *compatible_field_decl, *id_field_decl;
  MD_Entry *syll_entry;
  MD_Field *num_slots_field, *compatible_field;
  SM_Syllable *syllable, *syllable_array;
  int num_syllables, size, id, mask_offset, index;
  unsigned int top_syllable = 0;

  /* Get the Syllable_Types section and field_decl*/
  syll_section = MD_find_section (sm_mdes->md_mdes, "Syllable_Type");
  num_slots_field_decl = MD_find_field_decl (syll_section, "num_slots");
  compatible_field_decl = MD_find_field_decl (syll_section, "compatible_with");

  /* Add id's to each Syllable entry (used to index array)
   * and get the number of entries in the section.
   */
  num_syllables = SM_add_ids_to_entries (syll_section);
  id_field_decl = MD_find_field_decl (syll_section, "id");

  /* Malloc an array of syllables */
  size = num_syllables * sizeof (SM_Syllable);
  if ((syllable_array = (SM_Syllable *) malloc (size)) == NULL)
    I_punt ("SM_build_syllables: Out of memory");
  
  /* Copy contents of section into array.
   * Index array by id (numbered from 0)
   */
  id = 0;
  mask_offset = 0;
  for (syll_entry = MD_first_entry (syll_section); syll_entry != NULL;
       syll_entry = MD_next_entry (syll_entry))
    {
      syllable = &syllable_array[id];
      syllable->name = strdup (syll_entry->name);

      num_slots_field = MD_find_field (syll_entry, num_slots_field_decl);
      syllable->num_slots = MD_get_int (num_slots_field, 0);

      /* If compatible_with section exists for this entry,
       * AND the compatible_with field entries for the mask.
       */
      if ((compatible_field =
	   MD_find_field (syll_entry, compatible_field_decl)) != NULL)
	{
	  syllable->mask = 0;
	  
	  for (index = 0; index <= compatible_field->max_element_index; index++)
	    {
	      MD_Entry *target_entry;
	      MD_Field *target_field;
	      int input_id;
	      SM_Syllable *compatible_syll;
	      
	      target_entry = MD_get_link (compatible_field, index);
	      target_field = MD_find_field (target_entry, id_field_decl);
	      input_id = MD_get_int (target_field, 0);

	      /* Get the syll from this id */
	      compatible_syll = &syllable_array[input_id];

	      /* OR in the bit for this syll */
	      syllable->mask |= compatible_syll->mask;
	    }
	}
      /* Otherwise base the mask on the id */
      else
	{
	  syllable->mask = (1 << mask_offset);
	  /* Keep track of the top hex syllable value.
	   * Don't need to worry about compatible_with results.
	   */
	  top_syllable = syllable->mask;
	  mask_offset++;
	}

#if DEBUG_MDES
      printf ("Syllable %s: num_slots %i mask 0x%x\n", syllable->name, 
	      syllable->num_slots, syllable->mask);
#endif
      id++;
    }

  /* Point sm_mdes at syllable array */
  sm_mdes->syllable_array = syllable_array;
  sm_mdes->num_syllables = num_syllables;
  sm_mdes->top_syllable = top_syllable;
}

/* 20030527 SZU
 * Function reads in the Port_Type section in mdes.
 * Assigns an index and power of 2 hex mask to each entry.
 * Make sure order of entry is kept in the array;
 * becomes the scheduling priority order.
 */
void
SM_build_ports (SM_Mdes * sm_mdes)
{
  MD_Section *port_section, *syll_section;
  MD_Field_Decl *syll_field_decl, *syll_id_field_decl;
  MD_Entry *port_entry, *syll_entry;
  MD_Field *syll_field, *syll_id_field;
  SM_Syllable *syll_array;
  SM_Port *port, *port_array;
  int num_ports, size, id, syll_id;

  /* Get the syllable array */
  syll_array = sm_mdes->syllable_array;

  /* Get the Port section and field_decl*/
  port_section = MD_find_section (sm_mdes->md_mdes, "Port_Type");
  syll_field_decl = MD_find_field_decl (port_section, "syll");

  syll_section = MD_find_section (sm_mdes->md_mdes, "Syllable_Type");
  syll_id_field_decl = MD_find_field_decl (syll_section, "id");

  num_ports = SM_add_ids_to_entries (port_section);

  /* Malloc an array of ports */
  size = num_ports * sizeof (SM_Port);
  if ((port_array = (SM_Port *) malloc (size)) == NULL)
    I_punt ("SM_build_ports: Out of memory");
  
  /* Copy contents of section into array.
   * Index array by id (numbered from 0)
   */
  id = 0;
  for (port_entry = MD_first_entry (port_section); port_entry != NULL;
       port_entry = MD_next_entry (port_entry))
    {
      port = &port_array[id];
      port->name = strdup (port_entry->name);

      port->mask = (1 << id);

      /* Get the corresponding syllable */
      syll_field = MD_find_field (port_entry, syll_field_decl);
      syll_entry = MD_get_link (syll_field, 0);

      /* Get id */
      syll_id_field = MD_find_field (syll_entry, syll_id_field_decl);
      syll_id = MD_get_int (syll_id_field, 0);

      port->syllable = &syll_array[syll_id];

#if DEBUG_MDES
      printf ("Port %s: mask 0x%x syllable %s mask 0x%x\n", port->name,
	      port->mask, port->syllable->name, port->syllable->mask);
#endif
      id++;
    }

  /* Point sm_mdes at port array */
  sm_mdes->port_array = port_array;
  sm_mdes->num_ports = num_ports;
}

/* 20030528 SZU
 * Reads in Restrictions section.
 * Create resource restrictions based on Port_Types.
 */
void
SM_build_restrictions (SM_Mdes * sm_mdes)
{
  MD_Section *restrict_section, *port_section;
  MD_Field_Decl *num_field_decl, *ports_field_decl,*id_field_decl;
  MD_Entry *restrict_entry, *port_entry;
  MD_Field *num_field, *ports_field, *port_id_field;
  SM_Restriction *restrict, *restrict_array;
  SM_Port *port_array, *compatible_port;
  int num_restricts, size, id, index, port_id;

  /* Get the port array */
  port_array = sm_mdes->port_array;

  /* Get the Restrictions section and field_decl*/
  restrict_section = MD_find_section (sm_mdes->md_mdes, "Restrictions");
  num_field_decl = MD_find_field_decl (restrict_section, "num");
  ports_field_decl = MD_find_field_decl (restrict_section, "ports");

  port_section = MD_find_section (sm_mdes->md_mdes, "Port_Type");
  id_field_decl = MD_find_field_decl (port_section, "id");

  num_restricts = SM_add_ids_to_entries (restrict_section);

  /* Malloc an array of ports */
  size = num_restricts * sizeof (SM_Restriction);
  if ((restrict_array = (SM_Restriction *) malloc (size)) == NULL)
    I_punt ("SM_build_restrictions: Out of memory");
  
  /* Copy contents of section into array.
   * Index array by id (numbered from 0)
   */
  id = 0;
  for (restrict_entry = MD_first_entry (restrict_section);
       restrict_entry != NULL; restrict_entry = MD_next_entry (restrict_entry))
    {
      restrict = &restrict_array[id];
      restrict->name = strdup (restrict_entry->name);
      restrict->mask = 0;

      num_field = MD_find_field (restrict_entry, num_field_decl);

      restrict->num = MD_get_int (num_field, 0);

      /* Get the corresponding ports and their masks
       * OR together all the port masks to create mask for restriction
       */
      ports_field = MD_find_field (restrict_entry, ports_field_decl);

      for (index = 0; index <= ports_field->max_element_index; index++)
	{
	  port_entry = MD_get_link (ports_field, index);
	  port_id_field = MD_find_field (port_entry, id_field_decl);
	  port_id = MD_get_int (port_id_field, 0);

	  /* Get the port from this id */
	  compatible_port = &port_array[port_id];

	  /* OR in the bit for this syll */
	  restrict->mask |= compatible_port->mask;
	}

#if DEBUG_MDES
      printf ("Restriction %s: num %i mask 0x%x\n", restrict->name,
	      restrict->num, restrict->mask);
#endif
      id++;
    }
  /* Point sm_mdes at restrict array */
  sm_mdes->restrict_array = restrict_array;
  sm_mdes->num_restricts = num_restricts;
}

/* 20030529 SZU
 * Read in the Template_Type section.
 */
void
SM_build_templates (SM_Mdes * sm_mdes)
{
  MD_Section *template_section, *syll_section;
  MD_Field_Decl *syll_field_decl, *stop_field_decl, *syll_id_field_decl;
  MD_Field *syll_field, *stop_field;
  MD_Entry *template_entry;
  SM_Template *template, *template_array;
  int num_templates, size, shift_var, slots_per_template, index, id;
  int current_template_slots;
  unsigned int tmp_shift;

  /* Get the Template_Type section and field_decl*/
  template_section = MD_find_section (sm_mdes->md_mdes, "Template_Type");
  syll_field_decl = MD_find_field_decl (template_section, "syll");
  stop_field_decl = MD_find_field_decl (template_section, "stop");

  /* Get the Syllable_Type section and id_field_decl */
  syll_section = MD_find_section (sm_mdes->md_mdes, "Syllable_Type");
  syll_id_field_decl = MD_find_field_decl (syll_section, "id");

  num_templates = SM_add_ids_to_entries (template_section);

  /* Malloc an array of templates */
  size = num_templates * sizeof (SM_Template);
  if ((template_array = (SM_Template *) malloc (size)) == NULL)
    I_punt ("SM_build_templates: Out of memory");

  /* Get the highest syll type to figure out the shift_var.
   * shift_var set as multiple of 4 for clarity's sake.
   * Not required, and can be changed.
   */
  shift_var = 0;
  tmp_shift = sm_mdes->top_syllable;
  while (tmp_shift)
    {
      tmp_shift = tmp_shift >> 4;
      shift_var += 4;
    }

  slots_per_template = 0;
  id = 0;
  for (template_entry = MD_first_entry (template_section);
       template_entry != NULL; template_entry = MD_next_entry (template_entry))
    {
      template = &template_array[id];
      template->name = strdup (template_entry->name);
      template->mask = 0;

      /* Get the entries under syll field.
       * Make sure number of slots per template all equal.
       * Shift and OR together the syllables.
       */
      current_template_slots = 0;
      syll_field = MD_find_field (template_entry, syll_field_decl);

      for (index = 0; index <= syll_field->max_element_index; index++)
	{
	  MD_Entry *syll_entry;
	  MD_Field *syll_id_field;
	  int syll_id;
	  SM_Syllable *syllable;

	  syll_entry = MD_get_link (syll_field, index);
	  syll_id_field = MD_find_field (syll_entry, syll_id_field_decl);
	  syll_id = MD_get_int (syll_id_field, 0);

	  /* Get the syllable from this id */
	  syllable = &sm_mdes->syllable_array[syll_id];

	  /* Increment the current templates slots */
	  current_template_slots += syllable->num_slots;

	  /* OR in bits for this syll after shifting */
	  template->mask = template->mask << shift_var;
	  template->mask |= syllable->mask;

	  /* Account for syllables that take more than one slot */
	  if (syllable->num_slots > 1)
	    template->mask =
	      template->mask << shift_var * (syllable->num_slots - 1);
	}

      /* current_template_slots better equal slots_per_template */
      if (slots_per_template == 0)
	slots_per_template = current_template_slots;
      else if (current_template_slots != slots_per_template)
	I_punt ("SM_build_templates: Template %s have different # of slots!",
		template->name);

      /* Allocate space for stop bits and initialize to 0 */
      size = slots_per_template * sizeof (int);
      if ((template->stop_bits = (int *) malloc (size)) == NULL)
	I_punt ("SM_build_templates: Out of memory");
      for (index = 0; index < slots_per_template; index++)
	template->stop_bits[index] = 0;

      /* Go through the stop field and record the information */
      stop_field = MD_find_field (template_entry, stop_field_decl);
      for (index = 0; index <= stop_field->max_element_index; index++)
	{
	  int stop_id;

	  stop_id = MD_get_int (stop_field, index);

	  /* Sanity check */
	  if ((stop_id < 0) || (stop_id >= slots_per_template))
	    I_punt ("SM_build_templates: stop bits out of range");

	  /* Set the appropriate spot in the stop_bit array */
	  template->stop_bits[stop_id] = 1;
	}

      /* Derive bit mask for template_mask. Used by Issue_Group */
      template->template_mask = 1 << id;

#if DEBUG_MDES
      printf ("Template %s: mask 0x%x template_mask 0x%x\n", template->name,
	      template->mask, template->template_mask);
      for (index = 0; index < slots_per_template; index++)
	{
	  printf ("stop%i %i\n", index, template->stop_bits[index]);
	}
#endif

      id++;
    }

#if 0
  /* Sanity check.
   * Total number of slots is a multiple of slots_per_template.
   * 20030529 SZU
   * Do this check in lmdes2, when num_slots is available.
   */
  if (sm_mdes->md_mdes->num_slots % slots_per_template != 0)
    I_punt ("SM_build_templates: Total number of slots not divisible by "
	    "slots per template!");
#endif
  
  /* Point sm_mdes fields */
  sm_mdes->template_array = template_array;
  sm_mdes->num_templates = num_templates;
  sm_mdes->template_shift_var = shift_var;
  sm_mdes->slots_per_template = slots_per_template;
}

/* 20030529 SZU
 * Read in Template_Group section
 */
void
SM_build_tgroups (SM_Mdes * sm_mdes)
{
  MD_Section *tgroup_section, *template_section;
  MD_Field_Decl *one_of_field_decl, *one_of_id_field_decl;
  MD_Field *one_of_field, *one_of_id_field;
  MD_Entry *tgroup_entry, *one_of_entry;
  SM_TGroup *tgroup, *tgroup_array;
  SM_Template *template;
  int num_tgroups, id, index, size, one_of_id;

  /* Get the Template_Type section and field_decl*/
  tgroup_section = MD_find_section (sm_mdes->md_mdes, "Template_Group");
  one_of_field_decl = MD_find_field_decl (tgroup_section, "one_of");

  template_section = MD_find_section (sm_mdes->md_mdes, "Template_Type");
  one_of_id_field_decl = MD_find_field_decl (template_section, "id");

  num_tgroups = SM_add_ids_to_entries (tgroup_section);

  /* Malloc an array of tgroups */
  size = num_tgroups * sizeof (SM_TGroup);
  if ((tgroup_array = (SM_TGroup *) malloc (size)) == NULL)
    I_punt ("SM_build_tgroups: Out of memory");
  
  /* Copy contents of section into array.
   * Index array by id (numbered from 0)
   */
  id = 0;
  for (tgroup_entry = MD_first_entry (tgroup_section);
       tgroup_entry != NULL; tgroup_entry = MD_next_entry (tgroup_entry))
    {
      tgroup = &tgroup_array[id];
      tgroup->name = strdup (tgroup_entry->name);
      tgroup->mask = 0;

      /* Get the corresponding templates and their masks
       * OR together all the templates masks to create mask for tgroup
       */
      one_of_field = MD_find_field (tgroup_entry, one_of_field_decl);

      for (index = 0; index <= one_of_field->max_element_index; index++)
	{
	  one_of_entry = MD_get_link (one_of_field, index);
	  one_of_id_field = MD_find_field (one_of_entry, one_of_id_field_decl);
	  one_of_id = MD_get_int (one_of_id_field, 0);

	  /* Get the template from this id */
	  template = &sm_mdes->template_array[one_of_id];

	  /* OR in the template mask for this template */
	  tgroup->mask |= template->template_mask;
	}

#if DEBUG_MDES
      printf ("TGroup %s: mask 0x%x\n", tgroup->name, tgroup->mask);
#endif
      id++;
    }

  /* Point sm_mdes fields */
  sm_mdes->tgroup_array = tgroup_array;
  sm_mdes->num_tgroups = num_tgroups;
}

/* 20030530 SZU
 * Read in Issue_Group section
 */
void
SM_build_issues (SM_Mdes * sm_mdes)
{
  MD_Section *issue_section, *template_section, *tgroup_section;
  MD_Field_Decl *templates_field_decl, *template_id_field_decl;
  MD_Field_Decl *tgroup_id_field_decl;
  MD_Field *templates_field;
  MD_Entry *issue_entry;
  SM_Issue *issue, *issue_array;
  int num_issues, size, index, id, max_template_per_issue;

  /* Get the Issue_Group section and field_decl*/
  issue_section = MD_find_section (sm_mdes->md_mdes, "Issue_Group");
  templates_field_decl = MD_find_field_decl (issue_section, "templates");

  template_section = MD_find_section (sm_mdes->md_mdes, "Template_Type");
  template_id_field_decl = MD_find_field_decl (template_section, "id");
  tgroup_section = MD_find_section (sm_mdes->md_mdes, "Template_Group");
  tgroup_id_field_decl = MD_find_field_decl (tgroup_section, "id");

  num_issues = SM_add_ids_to_entries (issue_section);

  /* Malloc an array of ports */
  size = num_issues * sizeof (SM_Issue);
  if ((issue_array = (SM_Issue *) malloc (size)) == NULL)
    I_punt ("SM_build_issues: Out of memory");
  
  /* Copy contents of section into array.
   * Index array by id (numbered from 0)
   */
  id = 0;
  max_template_per_issue = 0;
  for (issue_entry = MD_first_entry (issue_section); issue_entry != NULL;
       issue_entry = MD_next_entry (issue_entry))
    {
      issue = &issue_array[id];
      issue->name = strdup (issue_entry->name);

      /* Get the template field */
      templates_field = MD_find_field (issue_entry, templates_field_decl);
      issue->num_templates = templates_field->max_element_index + 1;

      if (issue->num_templates > max_template_per_issue)
	max_template_per_issue = issue->num_templates;
	  
      /* Malloc an array of unsigned ints */
      size = issue->num_templates * sizeof (unsigned int);
      if ((issue->templates = (unsigned int *) malloc (size)) == NULL)
	I_punt ("SM_build_issues: Out of memory");
  
      for (index = 0; index <= templates_field->max_element_index; index++)
	{
	  MD_Entry *target_entry;
	  MD_Field *target_field;
	  int target_id;
	  SM_Template *template;
	  SM_TGroup *tgroup;

	  /* Get the target entry */
	  target_entry = MD_get_link (templates_field, index);

	  /* See if target is Template_Type, otherwise Template_Group */
	  if (strcmp (target_entry->section->name, "Template_Type") == 0)
	    {
	      target_field =
		MD_find_field (target_entry, template_id_field_decl);
	      target_id = MD_get_int (target_field, 0);

	      /* Get the template */
	      template = &sm_mdes->template_array[target_id];

	      issue->templates[index] = template->template_mask;
	    }
	  else
	    {
	      target_field = MD_find_field (target_entry, tgroup_id_field_decl);
	      target_id = MD_get_int (target_field, 0);

	      /* Get the tgroup */
	      tgroup = &sm_mdes->tgroup_array[target_id];

	      issue->templates[index] = tgroup->mask;
	    }
	}
#if DEBUG_MDES
      printf ("Issue %s: num_templates %i\n", issue->name,
	      issue->num_templates);
      for (index = 0; index < issue->num_templates; index++)
	{
	  printf ("template%i 0x%x\n", index, issue->templates[index]);
	}
#endif
      id++;
    }

  /* Point sm_mdes fields */
  sm_mdes->issue_array = issue_array;
  sm_mdes->num_issues = num_issues;
  sm_mdes->max_template_per_issue = max_template_per_issue;
}

/* 20030531 SZU
 * Read in Dispersal_Rule section
 * 20030617 SZU
 * Take care of new OPTIONAL template field.
 */
void
SM_build_drules (SM_Mdes * sm_mdes)
{
  MD_Section *drule_section, *port_section, *syll_section, *rsrc_section;
  MD_Section *template_section, *tgroup_section;
  MD_Field_Decl *slot_field_decl, *port_field_decl, *syll_field_decl;
  MD_Field_Decl *rsrc_field_decl, *port_id_field_decl, *syll_id_field_decl;
  MD_Field_Decl *rsrc_id_field_decl, *subrsrc_field_decl, *temp_field_decl;
  MD_Field_Decl *template_id_field_decl, *tgroup_id_field_decl;
  MD_Field *slot_field, *port_field, *syll_field, *rsrc_field, *subrsrc_field;
  MD_Field *temp_field;
  MD_Entry *drule_entry;
  SM_DRule *drule, *drule_array;
  int num_drules, size, index, id;
  unsigned int subrsrc_mask = 0;

  /* Get the Dispersal_Rule section and field_decl*/
  drule_section = MD_find_section (sm_mdes->md_mdes, "Dispersal_Rule");
  slot_field_decl = MD_find_field_decl (drule_section, "slot");
  port_field_decl = MD_find_field_decl (drule_section, "port_type");
  syll_field_decl = MD_find_field_decl (drule_section, "syll_type");
  rsrc_field_decl = MD_find_field_decl (drule_section, "resource");
  subrsrc_field_decl = MD_find_field_decl (drule_section, "subresource");

  port_section = MD_find_section (sm_mdes->md_mdes, "Port_Type");
  port_id_field_decl = MD_find_field_decl (port_section, "id");
  syll_section = MD_find_section (sm_mdes->md_mdes, "Syllable_Type");
  syll_id_field_decl = MD_find_field_decl (syll_section, "id");
  rsrc_section = MD_find_section (sm_mdes->md_mdes, "Resource");
  rsrc_id_field_decl = MD_find_field_decl (rsrc_section, "id");

  /* 20030617 SZU */
  temp_field_decl = MD_find_field_decl (drule_section, "template");
  template_section = MD_find_section (sm_mdes->md_mdes, "Template_Type");
  template_id_field_decl = MD_find_field_decl (template_section, "id");
  tgroup_section = MD_find_section (sm_mdes->md_mdes, "Template_Group");
  tgroup_id_field_decl = MD_find_field_decl (tgroup_section, "id");

  num_drules = SM_add_ids_to_entries (drule_section);

  /* Malloc an array of ports */
  size = num_drules * sizeof (SM_DRule);
  if ((drule_array = (SM_DRule *) malloc (size)) == NULL)
    I_punt ("SM_build_drules: Out of memory");
  
  /* Copy contents of section into array.
   * Index array by id (numbered from 0)
   */
  id = 0;
  for (drule_entry = MD_first_entry (drule_section); drule_entry != NULL;
       drule_entry = MD_next_entry (drule_entry))
    {
      MD_Entry *target_entry;
      MD_Field *target_field;
      int target_id;
      SM_Port *port;
      SM_Syllable *syll;
      SM_Resource *rsrc;

      drule = &drule_array[id];
      drule->name = strdup (drule_entry->name);
      subrsrc_mask = 0;

      /* Get the slot field.
       * If no slot field, then applies to all slots.
       * Assign 0 to drule->num_slots
       */
      if ((slot_field = MD_find_field (drule_entry, slot_field_decl)) != NULL)
	{
	  drule->num_slots = slot_field->max_element_index + 1;

	  /* Malloc an array of ints for slots */
	  size = drule->num_slots * sizeof (int);
	  if ((drule->slot = (int *) malloc (size)) == NULL)
	    I_punt ("SM_build_drules: Out of memory");
      
	  for (index = 0; index <= slot_field->max_element_index; index++)
	    {
	      /* Get the slot integer and save it */
	      target_id = MD_get_int (slot_field, index);
	      drule->slot[index] = target_id;
	    }
	}
      else
	{
	  drule->num_slots = 0;
	}

      /* Get the port field and the mask */
      port_field = MD_find_field (drule_entry, port_field_decl);
      target_entry = MD_get_link (port_field, 0);
      target_field = MD_find_field (target_entry, port_id_field_decl);
      target_id = MD_get_int (target_field, 0);
      port = &sm_mdes->port_array[target_id];
      drule->port_mask = port->mask;

      /* Get the syll field and the mask */
      syll_field = MD_find_field (drule_entry, syll_field_decl);
      target_entry = MD_get_link (syll_field, 0);
      target_field = MD_find_field (target_entry, syll_id_field_decl);
      target_id = MD_get_int (target_field, 0);
      syll = &sm_mdes->syllable_array[target_id];
      drule->syll_mask = syll->mask;

      drule->templates = 0;
      /* Get the template masks if any */
      if ((temp_field = MD_find_field (drule_entry, temp_field_decl)) != NULL)
	{
	  for (index = 0; index <= temp_field->max_element_index; index++)
	    {
	      MD_Entry *target_entry;
	      MD_Field *target_field;
	      int target_id;
	      SM_Template *template;
	      SM_TGroup *tgroup;

	      /* Get the target entry */
	      target_entry = MD_get_link (temp_field, index);

	      /* See if target is Template_Type, otherwise Template_Group */
	      if (strcmp (target_entry->section->name, "Template_Type") == 0)
		{
		  target_field =
		    MD_find_field (target_entry, template_id_field_decl);
		  target_id = MD_get_int (target_field, 0);

		  /* Get the template */
		  template = &sm_mdes->template_array[target_id];

		  drule->templates |= template->template_mask;
		}
	      else
		{
		  target_field =
		    MD_find_field (target_entry, tgroup_id_field_decl);
		  target_id = MD_get_int (target_field, 0);

		  /* Get the tgroup */
		  tgroup = &sm_mdes->tgroup_array[target_id];

		  drule->templates |= tgroup->mask;
		}
	    }
	}

      /* Go through the subrsrc fields and OR all resources if any */
      if ((subrsrc_field = MD_find_field (drule_entry, subrsrc_field_decl)) !=
	  NULL)
	{
	  for (index = 0; index <= subrsrc_field->max_element_index; index++)
	    {
	      /* Get the resource mask */
	      target_entry = MD_get_link (subrsrc_field, index);
	      target_field =
		MD_find_field (target_entry, rsrc_id_field_decl);
	      target_id = MD_get_int (target_field, 0);

	      rsrc = &sm_mdes->resource[target_id];
	      subrsrc_mask |= rsrc->mask;
	    }
	}

      /* Get the resource field */
      rsrc_field = MD_find_field (drule_entry, rsrc_field_decl);
      drule->num_rsrcs = rsrc_field->max_element_index + 1;

      /* Malloc an array of unsigned ints for resources */
      size = drule->num_rsrcs * sizeof (unsigned int);
      if ((drule->rsrc = (unsigned int *) malloc (size)) == NULL)
	I_punt ("SM_build_drules: Out of memory");
  
      for (index = 0; index <= rsrc_field->max_element_index; index++)
	{
	  /* Get the resource mask */
	  target_entry = MD_get_link (rsrc_field, index);
	  target_field = MD_find_field (target_entry, rsrc_id_field_decl);
	  target_id = MD_get_int (target_field, 0);

	  rsrc = &sm_mdes->resource[target_id];
	  drule->rsrc[index] = rsrc->mask;

	  /* Add subrsrc mask */
	  drule->rsrc[index] |= subrsrc_mask;
	}
#if DEBUG_MDES
      printf ("Dispersal Rule %s:\n", drule->name);
      for (index = 0; index < drule->num_slots; index++)
	printf ("slot %i ", drule->slot[index]);
      printf ("\nPort 0x%x Syllable 0x%x\n", drule->port_mask,
	      drule->syll_mask);
      for (index = 0; index < drule->num_rsrcs; index++)
	printf ("resource 0x%x ", drule->rsrc[index]);
      printf ("\ntemplates 0x%x\n", drule->templates);
#endif
      id++;
    }

  /* Point sm_mdes fields */
  sm_mdes->drule_array = drule_array;
  sm_mdes->num_drules = num_drules;
}

/* 20020805 SZU
 * Modified to read in p_lat_class(c_lat_class) instead of prod(cons),
 * because prod(cons) altered by customizer and don't match latency class.
 * 20020822 SZU
 * Updated due to lat_class changing from string to hash int.
 */
void
SM_build_prod_cons_latency (SM_Mdes * sm_mdes)
{
  SM_PCLat *pclat_array, *pclat;
  MD_Section *pclat_section;
  MD_Field_Decl *plat_field_decl, *pdest_field_decl, *clat_field_decl;
  MD_Field_Decl *csrc_field_decl, *latency_field_decl;
  MD_Field *plat_field, *pdest_field, *clat_field, *csrc_field, *latency_field;
  MD_Entry *pclat_entry;
  int num_pclat, array_size, id, num_prod, num_cons;
  int current_plat, current_clat, index;
  int latency, penalize;
  int *pdest_array, *pdest, *csrc_array, *csrc, op;
  Set plat, clat;
  int src_operands, dest_operands;
  MD *md;

  /* Get the Prod_Cons_Latency section and use field decl for ease of use. */
  pclat_section = MD_find_section (sm_mdes->md_mdes, "Prod_Cons_Latency");
  plat_field_decl = MD_find_field_decl(pclat_section, "p_lat_class");
  pdest_field_decl = MD_find_field_decl(pclat_section, "pdest");
  clat_field_decl = MD_find_field_decl(pclat_section, "c_lat_class");
  csrc_field_decl = MD_find_field_decl(pclat_section, "csrc");
  latency_field_decl = MD_find_field_decl(pclat_section, "latency");

  /* Add id's to each Prod_Cons_Lat entry.
   * Get number of entries.
   */
  num_pclat = SM_add_ids_to_entries (pclat_section);

  /* Malloc an array to hold the pclat info */
  array_size = num_pclat * sizeof (SM_PCLat);
  if ((pclat_array = (SM_PCLat *) malloc (array_size)) == NULL)
    I_punt ("SM_build_prod_cons_latency: Out of memory");

  /* Build array using pclat_section entry info */
  id = 0;
  for (pclat_entry = MD_first_entry (pclat_section); pclat_entry != NULL;
       pclat_entry = MD_next_entry (pclat_entry))
    {
      /* Get pointer inside pclat_array */
      pclat = &pclat_array[id];

      /* Copy the PCLat name */
      pclat->name = strdup (pclat_entry->name);

      /* Get the latency field and latency value */
      latency_field = MD_find_field (pclat_entry, latency_field_decl);
      latency = MD_get_int (latency_field, 0);

      /* Get the pclat's p_lat_class field */
      plat_field = MD_find_field (pclat_entry, plat_field_decl);

      /* 20020822 SZU */
      /* Create a Set to hold the plat info */
      num_prod = plat_field->max_element_index + 1;
      plat = Set_new();

      /* Go through each producer and add to the Set */
      for (current_plat = 0; current_plat < num_prod; current_plat++)
	{
	  /* Get the hash int of this producer */
	  op = MD_get_int (plat_field, current_plat);

	  Set_add (plat, op);
	}

#if 0
      pclat->plat = plat;
#else
      pclat->plat = Set_copy (plat);
      Set_dispose (plat);
#endif

      /* Get the pdest field */
      pdest_field = MD_find_field (pclat_entry, pdest_field_decl);

      /* 20020911 SZU
       * Change from hard code 4 dest to get number from Mdes
       */
      md = sm_mdes->md_mdes;
      dest_operands = get_int_parm (md, "num_dest_operands");

      /* Malloc array to hold pdest info */
      if((pdest_array = (int *) malloc (sizeof(int) * dest_operands)) == NULL)
	    I_punt ("SM_build_prod_cons_latency: Out of memory");
      for (index = 0; index < 4; index++)
	{
	  /* Get pointer */
	  pdest = &pdest_array[index];

	  /* Get current index value; 1 or 0 */
	  penalize = MD_get_int (pdest_field, index);

	  /* If penalize = 1, add penalty for this operand */
	  *pdest = penalize * latency;
	}

      pclat->pdest_latency_penalty = pdest_array;

      /* Get the clat field */
      clat_field = MD_find_field (pclat_entry, clat_field_decl);

      num_cons = clat_field->max_element_index + 1;
      clat = Set_new();

      /* Go through each consumer and add to the Set */
      for (current_clat = 0; current_clat < num_cons; current_clat++)
	{
	  /* Get the hash int of this consumer */
	  op = MD_get_int (clat_field, current_clat);

	  Set_add (clat, op);
	}

#if 0
      pclat->clat = clat;
#else
      pclat->clat = Set_copy (clat);
      Set_dispose (clat);
#endif

      /* Get the csrc field */
      csrc_field = MD_find_field (pclat_entry, csrc_field_decl);

      /* 20020911 SZU
       * Change from hard code 6 src to get number from Mdes
       */
      src_operands = get_int_parm (md, "num_src_operands");

      /* Malloc array to hold csrc info */
      if((csrc_array = (int *) malloc (sizeof(int) * src_operands)) == NULL)
	    I_punt ("SM_build_prod_cons_latency: Out of memory");
      for (index = 0; index < 6; index++)
	{
	  /* Get pointer */
	  csrc = &csrc_array[index];

	  /* Get current index value; 1 or 0 */
	  penalize = MD_get_int (csrc_field, index);

	  /* If penalize = 1, add penalty for this operand */
	  /* 20020808 SZU
	   * Due to the way SM takes the latency info, 
	   * csrc is now only a bit vector, not multiplied w/ latency.
	   */
	  *csrc = penalize;
	}

      pclat->csrc_latency_penalty = csrc_array;

      /* Increment the id */
      id++;
    }

  /* Point sm_mdes at table array and set its size */
  sm_mdes->pclat_array = pclat_array;
  sm_mdes->pclat_array_size = num_pclat;
}

/* Print out the information stored in sm_mdes->pclat_array
 * for debugging purposes.
 * 20020823 SZU
 * Modified due to data structure change to Set.
 */
void
Print_prod_cons_latency (SM_Mdes *sm_mdes)
{
  SM_PCLat *pclat_array, *pclat;
  int index1, index2, latency;

  pclat_array = sm_mdes->pclat_array;

  for (index1 = 0; index1 < sm_mdes->pclat_array_size; index1++)
    {
      /* Get a pointer to current pclat */
      pclat = &pclat_array[index1];

      /* Print the PCLat's name */
      printf ("PCLat: %s\n", pclat->name);

      //printf ("Producers:\n");
      Set_print (stdout, "Producers:", pclat->plat);

      /* Print the pdest latency */
      for (index2 = 0; index2 < 4; index2++)
	{
	  latency = pclat->pdest_latency_penalty[index2];
	  printf ("%i", latency);
	}
      printf ("\n");

      //printf ("Consumers:\n");
      Set_print (stdout, "Consumers:", pclat->clat);

      /* Print the csrc latency */
      for (index2 = 0; index2 < 6; index2++)
      {
	latency = pclat->csrc_latency_penalty[index2];
	printf ("%i", latency);
      }
      printf ("\n");
      printf("\n");
    }
}

SM_Mdes *
SM_build_mdes (Mdes2 * mdes2)
{
  SM_Mdes *sm_mdes;

  /* Create alloc pool if necessary */
  if (SM_Mdes_pool == NULL)
    SM_Mdes_pool = L_create_alloc_pool ("SM_Mdes", sizeof (SM_Mdes), 1);

  /* Alloc sm_mdes data structure */
  sm_mdes = (SM_Mdes *) L_alloc (SM_Mdes_pool);

  /* Point at the mdes2 this sm_mdes is being built from */
  sm_mdes->mdes2 = mdes2;
  sm_mdes->md_mdes = mdes2->md_mdes;

  /* Build the resource array, set num_resources, map_width, 
   * and time_shift.
   */
  SM_build_resources (sm_mdes);

  /* Build the unit array and set num_units */
  SM_build_units (sm_mdes);

  /* Build the choice array and set num_choices */
  SM_build_choices (sm_mdes);

  /* Build the table array and set num_tables */
  SM_build_tables (sm_mdes);

  /* 20030527 SZU
   * Functions necessary to read in new Itanium descriptions in mdes.
   * Replaced hard coded version.
   */
  if (MD_find_section (sm_mdes->md_mdes, "Syllable_Type") != NULL)
    {
      SM_build_syllables (sm_mdes);
    }
  else
    {
      sm_mdes->syllable_array = NULL;
      sm_mdes->num_syllables = 0;
      sm_mdes->top_syllable = 0;
    }

  if (MD_find_section (sm_mdes->md_mdes, "Port_Type") != NULL)
    {
      SM_build_ports (sm_mdes);
    }
  else
    {
      sm_mdes->port_array = NULL;
      sm_mdes->num_ports = 0;
    }

  if (MD_find_section (sm_mdes->md_mdes, "Restrictions") != NULL)
    {
      SM_build_restrictions (sm_mdes);
    }
  else
    {
      sm_mdes->restrict_array = NULL;
      sm_mdes->num_restricts = 0;
    }

  if (MD_find_section (sm_mdes->md_mdes, "Template_Type") != NULL)
    {
      SM_build_templates (sm_mdes);
    }
  else
    {
      sm_mdes->template_array = NULL;
      sm_mdes->num_templates = 0;
      sm_mdes->template_shift_var = 0;
      sm_mdes->slots_per_template = 0;
    }

  if (MD_find_section (sm_mdes->md_mdes, "Template_Group") != NULL)
    {
      SM_build_tgroups (sm_mdes);
    }
  else
    {
      sm_mdes->tgroup_array = NULL;
      sm_mdes->num_tgroups = 0;
    }

  if (MD_find_section (sm_mdes->md_mdes, "Issue_Group") != NULL)
    {
      SM_build_issues (sm_mdes);
    }
  else
    {
      sm_mdes->issue_array = NULL;
      sm_mdes->num_issues = 0;
      sm_mdes->max_template_per_issue = 0;
    }

  if (MD_find_section (sm_mdes->md_mdes, "Dispersal_Rule") != NULL)
    {
      SM_build_drules (sm_mdes);
    }
  else
    {
      sm_mdes->drule_array = NULL;
      sm_mdes->num_drules = 0;
    }

  /* 20020723 SZU
   * Build the prod_cons_latency array
   * Prod_Cons_Latency section not in all mdes. Check first.
   */
  if (MD_find_section (sm_mdes->md_mdes, "Prod_Cons_Latency") != NULL)
    {
      SM_build_prod_cons_latency (sm_mdes);

#if DEBUG_MDES
      /* Function to print out prod_cons_latency for debugging */
      Print_prod_cons_latency (sm_mdes);
#endif
    }
  else
    {
      /* Prod_Cons_Latency section not in mdes. Set to 0. */
#if 0
      printf("No Prod_Cons_Latency in mdes\n");
#endif
      sm_mdes->pclat_array = NULL;
      sm_mdes->pclat_array_size = 0;
    }

  /* Return the newly created sm_mdes data structure */
  return (sm_mdes);
}
