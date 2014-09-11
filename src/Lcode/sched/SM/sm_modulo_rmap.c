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
 *      File:   sm_modulo_rmap.c based on sm_rmap.c (subset)
 *      Author: IMPACT Technologies Inc. (John C. Gyllenhaal)
 *      Creation Date:  August 1999
\*****************************************************************************/

#include <config.h>
#include <stdio.h>
#include "sm.h"
#include <library/l_alloc_new.h>

/* Prototypes */
extern void SM_modulo_commit_choices (unsigned int *map, SM_Table * table,
                                      unsigned short *choices_made,
                                      unsigned int start_offset,
                                      unsigned int offset_II);

extern void SM_modulo_release_choices (unsigned int *map, SM_Table * table,
                                       unsigned short *choices_made,
                                       unsigned int start_offset,
                                       unsigned int offset_II);

extern int SM_modulo_choose_first_avail_options (unsigned int *map,
                                                 SM_Table * table,
                                                 unsigned short *choices_made,
                                                 unsigned int start_offset,
                                                 unsigned int offset_II,
                                                 unsigned int min_slot,
                                                 unsigned int max_slot,
                                                 Mdes_Stats * stats);

extern int SM_modulo_choose_options_w_d_rule (SM_Oper * sm_op,
					      unsigned int template_index,
					      unsigned int *map,
					      SM_Table * table,
					      unsigned short *choices_made,
					      unsigned int start_offset,
					      unsigned int offset_II,
					      unsigned int abs_slot,
					      Mdes_Stats * stats);

/* Find a valid modulo resource placement for this table at the given time
 * and with the given slot bounds.  Place the options selected in
 * the choice array.  If commit ==1, then commit these resources to
 * the schedule.
 *
 * Returns -1 if a valid placement could not be found, or the slot (>= 0)
 * of the placement.  
 *
 * Note: the contents of the choice array are destroyed even if a 
 * valid placement cannot be found.
 *
 * 20030213 SZU
 * Header different for Itanium/Mckinley
 */
int
SM_modulo_sched_table (SM_Cb * sm_cb, SM_Oper * sm_op,
		       unsigned int template_index, SM_Table * table,
                       unsigned short *choice_array, int time,
                       unsigned short min_slot, unsigned short max_slot,
                       int commit, Mdes_Stats * stats)
#if 0
int
SM_modulo_sched_table (SM_Cb * sm_cb, SM_Table * table,
                       unsigned short *choice_array, int time,
                       unsigned short min_slot, unsigned short max_slot,
                       int commit, Mdes_Stats * stats)
#endif
{
  SM_Mdes *sm_mdes;
  unsigned int *map_array;
  int slot;
#if TAKE_STATS
  int start_option_checks, start_usage_checks;
  int net_option_checks, net_usage_checks;
#endif
  int time_II, offset_II, start_offset;

#if TAKE_STATS
  stats->num_table_checks++;
#endif


  /* Get the SM mdes for ease of use */
  sm_mdes = sm_cb->sm_mdes;

  /* Sanity check, expect II to be > 0 -ITI/JCG 8/99 */
  time_II = sm_cb->II;
  if (time_II < 1)
    L_punt ("SM_modulo_sched_table: II (%i) < 1!", time_II);

  /* Convert time-based II into offset-based II (may be the same).
   * If more than 32 resources are used, then multiple map words per
   * cycle will be used.  This is handled using a time shift to calculate
   * the usage offsets (and to modify II). -ITI/JCG 8/99
   */
  offset_II = time_II << sm_mdes->time_shift;

  /* Convert schedule time into a start offset that falls between
   * 0 and offset_II.  First convert the time to an offset (using time_shift)
   * and then mod it with offset_II.
   *
   * Since time may be negative, the modulo operation may result in a 
   * negative number.  In this case, add offset_II to get a number in 
   * the proper range (don't just negate it). -ITI/JCG 8/99
   */
  start_offset = (time << sm_mdes->time_shift) % offset_II;
  if (start_offset < 0)
    start_offset += offset_II;

  /* Create the map_array and initialize fields, if necessary */
  if (sm_cb->map_array == NULL)
    {
      /* May only use resources between 0 and (offset_II -1).

       * To get correct start offset, tell it to start at 4 so
       * that when it moves it back 4 cycles to give extra space,
       * we end up with the 0 start offset desired. -ITI/JCG 8/99
       */
      SM_create_map (sm_cb, 4, (offset_II - 1));

      /* Sanity check, expect map_start_offset to be 0 (assumed
       * for the code below). -ITI/JCG 8/99
       */
      if (sm_cb->map_start_offset != 0)
        {
          L_punt ("SM_modulo_sched_table: Expected map_start_offset (%i) "
                  "to be 0!", sm_cb->map_start_offset);
        }
    }

  /* Force intialization of all resource usage tables from
   * 0 to offset_II.  The rest of the algorithm assumes
   * that the full table is initialized.
   *
   * Do check outside of loop in order to handle changing II.
   */
  if (0 < sm_cb->min_init_offset)
    SM_init_for_min_usage (sm_cb, 0);

  if ((offset_II - 1) > sm_cb->max_init_offset)
    SM_init_for_max_usage (sm_cb, offset_II - 1);


  /* Get the pointer to the map array. */
  map_array = sm_cb->map_array;

#if TAKE_STATS
  /* Update choice distribution */
  increment_check_history (stats->num_choice_dist,
                           (int) (table->last_choice -
                                  table->first_choice) + 1, 1);
  increment_check_history (stats->first_choice_dist,
                           (int) (table->first_choice->last_option -
                                  table->first_choice->first_option) + 1, 1);
#endif

#if TAKE_STATS
  /* Use existing stats to calculate net checks for this table */
  start_option_checks = stats->num_option_checks;
  start_usage_checks = stats->num_usage_checks;
#endif

  /* Try to schedule the table at this time */
  /* 20030213 SZU
   * SM_modulo_choose_options_w_d_rule should only be called w/ one valid slot,
   * instead of slot range
   */
  /* 20031024 SZU
   * Template index MUST BE VALID for template bundling!
   */
  if (template_index != SM_NO_TEMPLATE)
    slot = SM_modulo_choose_options_w_d_rule (sm_op, template_index, map_array,
					      table, choice_array, start_offset,
					      offset_II, min_slot, stats);
  else
    slot = SM_modulo_choose_first_avail_options (map_array, table,
						 choice_array, start_offset,
						 offset_II, min_slot,
						 max_slot, stats);
#if 0
  slot = SM_modulo_choose_first_avail_options (map_array, table,
                                               choice_array, start_offset,
                                               offset_II, min_slot,
                                               max_slot, stats);
#endif

#if TAKE_STATS
  net_option_checks = stats->num_option_checks - start_option_checks;
  net_usage_checks = stats->num_usage_checks - start_usage_checks;
#endif

  /* Were we successful? */
  if (slot >= 0)
    {
#if TAKE_STATS
      /* Yes, update succeed distribution */
      increment_check_history (stats->succeed_option_check_dist,
                               net_option_checks, 1);
      increment_check_history (stats->succeed_usage_check_dist,
                               net_usage_checks, 1);
#endif

      if (commit)
        {
#if DEBUG_RMAP
          printf ("\n> Scheduling table at time %i in slot %i\n", time, slot);
          for (index = 0; index < table->num_choices; index++)
            {
              printf ("  Choice %i: option %i\n", index, choice_array[index]);
            }
          printf ("\n");
#endif

          SM_modulo_commit_choices (map_array, table, choice_array,
                                    start_offset, offset_II);
        }
      return (slot);
    }
  else
    {
#if TAKE_STATS
      /* No, update fail distribution */
      increment_check_history (stats->fail_option_check_dist,
                               net_option_checks, 1);
      increment_check_history (stats->fail_usage_check_dist,
                               net_usage_checks, 1);
#endif
#if DEBUG_RMAP
      if (commit)
        {
          printf ("Unable to schedule table at time %i (return value %i)\n",
                  time, slot);
        }
#endif
#if TAKE_STATS
      stats->num_table_checks_failed++;
#endif
      return (-1);
    }
}

/* Using the choice array and the time specified, the resources (modulo) for a 
 * scheduled operation are released.  Note, algorithm errors will occur
 * is this function is called for an unscheduler table and/or the
 * incorrect time.  (Some errors are detectable and will cause a punt.)
 */
void
SM_modulo_unsched_table (SM_Cb * sm_cb, SM_Table * table,
                         unsigned short *choice_array, int time)
{
  SM_Mdes *sm_mdes;
  unsigned int *map_array;
  int time_II, offset_II, start_offset;

  /* Get the SM mdes for ease of use */
  sm_mdes = sm_cb->sm_mdes;

  /* Sanity check, expect II to be > 0 */
  time_II = sm_cb->II;
  if (time_II < 1)
    L_punt ("SM_modulo_sched_table: II (%i) < 1!", time_II);

  /* Convert time-based II into offset-based II (may be the same).
   * If more than 32 resources are used, then multiple map words per
   * cycle will be used.  This is handled using a time shift to calculate
   * the usage offsets (and to modify II). -ITI/JCG 8/99
   */
  offset_II = time_II << sm_mdes->time_shift;

  /* Convert schedule time into a start offset that falls between
   * 0 and offset_II.  First convert the time to an offset (using time_shift)
   * and then mod it with offset_II.
   *
   * Since time may be negative, the modulo operation may result in a 
   * negative number.  In this case, add offset_II to get a number in 
   * the proper range (don't just negate it). -ITI/JCG 8/99
   */
  start_offset = (time << sm_mdes->time_shift) % offset_II;
  if (start_offset < 0)
    start_offset += offset_II;

  /* Punt if the map array is not initialized */
  if (sm_cb->map_array == NULL)
    L_punt ("SM_unsched_table: Error, map array not initialized!");

  /* Get the pointer to the map array. */
  map_array = sm_cb->map_array;

  /* Free all the resources used by this table */
  SM_modulo_release_choices (map_array, table, choice_array,
                             start_offset, offset_II);
}

/* 20021215 SZU
 * New function to choose slot and options instead of
 * SM_choose_first_avail_options.
 * Restrict by Dispersal_Rules
 * 20030606 SZU
 * Altered to mdes format.
 */
int
SM_modulo_choose_options_w_d_rule (SM_Oper * sm_op,
				   unsigned int template_index,
				   unsigned int *map,
				   SM_Table * table,
				   unsigned short *choices_made,
				   unsigned int start_offset,
				   unsigned int offset_II,
				   unsigned int abs_slot,
     				   Mdes_Stats * stats)
{ 
  int d_rule_match, d_rule_index, d_resources, d_resources_index;
  SM_DRule *d_rule;

  SM_Choice *choice_ptr, *first_choice, *last_choice, *slot_choice;
  SM_Option *option_ptr, *first_option, *last_option;
  SM_Usage *usage_ptr, *first_usage, *last_usage;
  unsigned short option_id, *choices_made_ptr;
  unsigned short *slot_options_ptr;
  unsigned int slot;
  unsigned int map_contents, resources_used, usage_offset, map_offset;

  slot = -1;

  /* Find the correct d_rule */
  d_rule_index = SM_find_drule (sm_op, abs_slot, template_index, -1);

  /* Should punt if !drule, but just in case */
  if (d_rule_index == -1)
    {
      d_rule_match = 0;
      d_rule = NULL;
      d_resources = 0;
      d_resources_index = -1;
    }
  else
    {
      d_rule_match = 1;
      d_rule = SM_get_drule (sm_op->sm_cb, d_rule_index);
      d_resources = SM_get_drsrc (d_rule, 0);
      d_resources_index = 0;
    }

  while (d_rule_match)
    {
      /* Get pointer to choices_made array for ease of use */
      choices_made_ptr = choices_made;

      /* Get the choice that sets the slot option for ease of use & speed */
      slot_choice = table->slot_choice;

      /* Get pointer to the slot options for the slot choice */
      slot_options_ptr = table->slot_options;

      /* Cache the choice array bounds for ease of use & speed */
      first_choice = table->first_choice;
      last_choice = table->last_choice;

      /* For each choice, start w/ the first option and find match w/ d_rule */
      choice_ptr = first_choice;

      while (d_rule_match)
	{
#if DEBUG_RMAP
	  printf ("    Testing choice %i:\n", (int) (choice_ptr - first_choice));
#endif

	  /* Cache the option array bounds for ease of use & speed */
	  first_option = choice_ptr->first_option;
	  last_option = choice_ptr->last_option;

	  /* Initialize the option id used for setting *choices_made_ptr */
	  option_id = 0;

	  /* Pick the option that has its resources available 
	   * and matches dispersal rule restrictions
	   */
	  option_ptr = first_option;

	  while (d_rule_match)
	    {
#if DEBUG_RMAP
	      printf ("      Testing option %i:\n",
		      (int) (option_ptr - first_option));
#endif

#if TAKE_STATS
	      stats->num_option_checks++;
#endif
	      /* Cache the usage array bounds for ease of use & speed */
	      first_usage = option_ptr->first_usage;
	      last_usage = option_ptr->last_usage;

	      /* In order to choice this option, all resources used by
	       * the usage array must be available in the resource map 
	       */
	      usage_ptr = first_usage;

	      while (d_rule_match)
		{
		  usage_offset = usage_ptr->map_offset;
		  resources_used = usage_ptr->resources_used;

		  /* Calculate the map offset from the usage offset, 
		   * start_offset, and the offset_II.  This is the
		   * key step in modulo resource usage.
		   *
		   * Note: this calculation assumes start_offset is non-negative
		   * (done in the caller) and that usage_offset it non-negative
		   * (done by lmdes2_customizer).  Otherwise, would need
		   * to detect negative map_offsets and add offset_II to
		   * them to make them legal. -ITI/JCG 8/99
		   */
		  map_offset = (start_offset + usage_offset) % offset_II;

#if TAKE_STATS
		  stats->num_usage_checks++;
#endif

#if DEBUG_RMAP
		  printf ("        Testing usage %i (offset %i mask %x)\n",
			  (int) (usage_ptr - first_usage), map_offset,
			  resources_used);
#endif
		  /* Get the contents of the resource map
		   * at this offset
		   */
		  map_contents = map[map_offset];

		  /* Goto next usage while loading map_contents */
		  usage_ptr++;

		  /* Check if same resources as those in d_rule */
		  if ((resources_used & d_resources) == 0) 
		    {
		      usage_ptr = NULL;
		      break;
		    }

		  /* Are the desired resources available
		   * (all desired bits 0)
		   */
		  if ((map_contents & resources_used) != 0)
		    {
#if DEBUG_RMAP
		      printf ("          Resources %x not available\n",
			      (map_contents & resources_used));
#endif

#if TAKE_STATS
		      stats->num_usage_checks_failed++;
		      stats->num_option_checks_failed++;
#endif
		      /* Resources not available, flag by setting usage_ptr
		       * to NULL and exit loop so can try the next option 
		       */
		      usage_ptr = NULL;
		      break;
		    }

		  /* Stop if have gotten all the desired resources */
		  if (usage_ptr > last_usage)
		    break;
		}

	      /* For the slot choice, the option chosen also determines the
	       * schedule slot.  Test only the slot choice options that
	       * fall within the min_slot and max_slot bounds
	       */
	      if (choice_ptr == slot_choice)
		{
		  if (usage_ptr != NULL)
		    {
		      /* Get the slot for this option and update pointer */
		      slot = *slot_options_ptr;
		      slot_options_ptr++;
#if DEBUG_RMAP
		      printf ("        Testing slot %i (allows %i).\n",
			      slot, abs_slot);
#endif

#if TAKE_STATS
		      stats->num_slot_checks++;
#endif
		      if (slot != abs_slot)
			{
#if DEBUG_RMAP
			  printf
			    ("       Slot %i rejected (allows %i).\n",
			     slot, abs_slot);
#endif

#if TAKE_STATS
			  stats->num_slot_checks_failed++;

			  /* Update usage and option failed stats since
			   * they have failed due to slot constraints.
			   */
			  stats->num_usage_checks_failed++;
			  stats->num_option_checks_failed++;
#endif
			  /* Update state so we can goto next option */
			  option_ptr++;
			  option_id++;

			  /* If we have used up all the options and
			   * have not found one that can be scheduled,
			   * change d_rule
			   */
			  if (option_ptr > last_option)
			    {
			      d_rule_match = 0;
			      break;
			    }

			  /* Goto next option */
			  continue;
			}
		    }
		  /* If resources are not available, just update pointer */
		  else
		    {
		      slot_options_ptr++;
		    }
		}

	      /* If option is available (usage_ptr != NULL), set the
	       * option used in the choices made array and exit loop
	       * so can go to next choice.
	       */
	      if (usage_ptr != NULL)
		{

		  /* If here, then resources and slot constraints are met! */

#if DEBUG_RMAP
		  if (lmdes->mdes2 != NULL)
		    {
		      printf ("  Option %i available:\n",
			      (int) (option_ptr - first_option));
		      SM_print_option (stdout, lmdes->mdes2->sm_mdes,
				       option_ptr);
		    }
#endif

		  /* Update the choices_made array */
		  *choices_made_ptr = option_id;

		  /* Goto next choice */
		  break;
		}

	      /* All loop update code (below this point) must also be placed
	       * in the slot bound testing code at the beginning of this loop!
	       */

	      /* Update option_ptr and option_id before trying next option */
	      option_ptr++;
	      option_id++;

	      /* If we have used up all the options and have not found
	       * one that can be scheduled, change d_rule
	       */
	      if (option_ptr > last_option)
		{
		  d_rule_match = 0;
		  break;
		}
	    }

	    /* Update choice_ptr and choices_made_ptr before try next choice */
	    choice_ptr++;
	    choices_made_ptr++;

	    /* Stop if we have gone through each choice */
	    if (choice_ptr > last_choice)
	      break;
	}

      /* Something went wrong. Try another d_rule. */
      if (!d_rule_match)
	{
	  /* First see if there are more resources left for this d_rule */
	  if (d_resources_index < d_rule->num_rsrcs - 1)
	    {
	      d_rule_match = 1;
	      d_resources_index++;
	      d_resources = SM_get_drsrc (d_rule, d_resources_index);
	    }
	  /* Else check for more drule */
	  else
	    {
	      d_rule_index =
		SM_find_drule (sm_op, abs_slot, template_index, d_rule_index);

	      if (d_rule_index == -1)
		{
		  d_rule_match = 0;
		  d_rule = NULL;
		  d_resources = 0;
		  d_resources_index = -1;
		}
	      else
		{
		  d_rule_match = 1;
		  d_rule = SM_get_drule (sm_op->sm_cb, d_rule_index);
		  d_resources_index = 0;
		  d_resources = SM_get_drsrc (d_rule, d_resources_index);
		}
	    }
	}
      /* Everything passed. */
      else
	{
	  break;
	}
    }

  if ((slot >= 0) && d_rule_match)
    return ((int) slot);

  return (-1);
}

/* Determines if the table can be scheduled (using modulo resources)
 * using a greedy algorthim (picking the first option that works).
 * 
 * Requires that map starts at time/offset 0 and that usage offsets
 * are non-negative (guarenteed by lmdes2_customizer). -ITI/JCG 8/99
 */
int
SM_modulo_choose_first_avail_options (unsigned int *map, SM_Table * table,
                                      unsigned short *choices_made,
                                      unsigned int start_offset,
                                      unsigned int offset_II,
                                      unsigned int min_slot,
                                      unsigned int max_slot,
                                      Mdes_Stats * stats)
{
  SM_Choice *choice_ptr, *first_choice, *last_choice, *slot_choice;
  SM_Option *option_ptr, *first_option, *last_option;
  SM_Usage *usage_ptr, *first_usage, *last_usage;
  unsigned short option_id, *choices_made_ptr;
  unsigned short *slot_options_ptr;
  unsigned int slot = 0;
  unsigned int map_contents, resources_used, usage_offset, map_offset;

  /* Get pointer to choices_made array for ease of use */
  choices_made_ptr = choices_made;

  /* Get the choice that sets the slot option for ease of use & speed */
  slot_choice = table->slot_choice;

  /* Get pointer to the slot options for the slot choice */
  slot_options_ptr = table->slot_options;

  /* Cache the choice array bounds for ease of use & speed */
  first_choice = table->first_choice;
  last_choice = table->last_choice;


  /* For each choice, pick the first option that is available */
  choice_ptr = first_choice;
  while (1)
    {
#if DEBUG_RMAP
      printf ("    Testing choice %i:\n", (int) (choice_ptr - first_choice));
#endif

      /* Cache the option array bounds for ease of use & speed */
      first_option = choice_ptr->first_option;
      last_option = choice_ptr->last_option;

      /* Initialize the option id used for setting *choices_made_ptr */
      option_id = 0;

      /* Pick the first available option that has its resources available */
      option_ptr = first_option;
      while (1)
        {
#if DEBUG_RMAP
          printf ("      Testing option %i:\n",
                  (int) (option_ptr - first_option));
#endif

#if TAKE_STATS
          stats->num_option_checks++;
#endif


          /* Cache the usage array bounds for ease of use & speed */
          first_usage = option_ptr->first_usage;
          last_usage = option_ptr->last_usage;

          /* In order to choice this option, all resources used by
           * the usage array must be available in the resource map 
           */
          usage_ptr = first_usage;
          while (1)
            {

              /* Cache the usage contents for ease of use & speed */
              usage_offset = usage_ptr->map_offset;
              resources_used = usage_ptr->resources_used;

              /* Calculate the map offset from the usage offset, 
               * start_offset, and the offset_II.  This is the
               * key step in modulo resource usage.
               *
               * Note: this calculation assumes start_offset is non-negative
               * (done in the caller) and that usage_offset it non-negative
               * (done by lmdes2_customizer).  Otherwise, would need
               * to detect negative map_offsets and add offset_II to
               * them to make them legal. -ITI/JCG 8/99
               */
              map_offset = (start_offset + usage_offset) % offset_II;

#if TAKE_STATS
              stats->num_usage_checks++;
#endif

#if DEBUG_RMAP
              printf ("        Testing usage %i (offset %i mask %x)\n",
                      (int) (usage_ptr - first_usage), map_offset,
                      resources_used);
#endif

              /* Get the contents of the resource map at this offset */
              map_contents = map[map_offset];

              /* Goto next usage while loading map_contents */
              usage_ptr++;

              /* Are the desired resources available (all desired bits 0)? */
              if ((map_contents & resources_used) != 0)
                {
#if DEBUG_RMAP
                  printf ("          Resources %x not available\n",
                          (map_contents & resources_used));
#endif

#if TAKE_STATS
                  stats->num_usage_checks_failed++;
                  stats->num_option_checks_failed++;
#endif

                  /* Resources not available, flag by setting usage_ptr
                   * to NULL and exit loop so can try the next option 
                   */
                  usage_ptr = NULL;
                  break;
                }

              /* Stop if have gotten all the desired resources */
              if (usage_ptr > last_usage)
                break;
            }

          /* For the slot choice, the option chosen also determines the
           * schedule slot.  Test only the slot choice options that
           * fall within the min_slot and max_slot bounds
           */
          if (choice_ptr == slot_choice)
            {
              if (usage_ptr != NULL)
                {
                  /* Get the slot for this option and update pointer */
                  slot = *slot_options_ptr;
                  slot_options_ptr++;

#if DEBUG_RMAP
                  printf ("        Testing slot %i (dep allows %i-%i).\n",
                          slot, min_slot, max_slot);
#endif

#if TAKE_STATS
                  stats->num_slot_checks++;
#endif


                  /* Reject option if slot doesn't fall within
                     required bounds */
                  if ((slot < min_slot) || (slot > max_slot))
                    {
#if DEBUG_RMAP
                      printf
                        ("          Slot %i rejected (dep allows %i-%i).\n",
                         slot, min_slot, max_slot);
#endif

#if TAKE_STATS
                      stats->num_slot_checks_failed++;

                      /* Update usage and option failed stats since
                       * they have failed due to slot constraints.
                       */
                      stats->num_usage_checks_failed++;
                      stats->num_option_checks_failed++;
#endif

                      /* Update state so we can goto next option */
                      option_ptr++;
                      option_id++;

                      /* If we have used up all the options and have not found
                       * one that can be scheduled, return -1
                       */
                      if (option_ptr > last_option)
                        return (-1);

                      /* Goto next option */
                      continue;
                    }
                }

              /* If resources are not available, just update pointer */
              else
                {
                  slot_options_ptr++;
                }
            }

          /* If option is available (usage_ptr != NULL), set the
           * option used in the choices made array and exit loop
           * so can go to next choice.
           */
          if (usage_ptr != NULL)
            {

              /* If here, then resources and slot constraints are met! */

#if DEBUG_RMAP
              if (lmdes->mdes2 != NULL)
                {
                  printf ("  Option %i available:\n",
                          (int) (option_ptr - first_option));
                  SM_print_option (stdout, lmdes->mdes2->sm_mdes, option_ptr);
                }
#endif

              /* Update the choices_made array */
              *choices_made_ptr = option_id;

              /* Goto next choice */
              break;
            }

          /* All loop update code (below this point) must also be placed
           * in the slot bound testing code at the beginning of this loop!
           */

          /* Update option_ptr and option_id before trying next option */
          option_ptr++;
          option_id++;

          /* If we have used up all the options and have not found
           * one that can be scheduled, return -1
           */
          if (option_ptr > last_option)
            return (-1);
        }

      /* Update choice_ptr and choices_made_ptr before try next choice */
      choice_ptr++;
      choices_made_ptr++;

      /* Stop if we have sucessfully selected an option for each choice */
      if (choice_ptr > last_choice)
        break;
    }

  /* We have successfully selected the first option available for each
   * choice.  Return the slot choosen to signal success.
   */
  return ((int) slot);
}

/* Mark the resources used by the scheduled operation using the
 * table, the choices made, and the offset_II.
 *
 * Requires that map starts at time/offset 0 and that usage offsets
 * are non-negative (guarenteed by lmdes2_customizer). -ITI/JCG 8/99
 */
void
SM_modulo_commit_choices (unsigned int *map, SM_Table * table,
                          unsigned short *choices_made,
                          unsigned int start_offset, unsigned int offset_II)
{
  SM_Choice *choice_ptr, *first_choice, *last_choice;
  SM_Option *option_ptr, *option_array;
  SM_Usage *usage_ptr, *first_usage, *last_usage;
  unsigned short option_id, *choices_made_ptr;
  unsigned int map_contents, resources_used, map_offset, usage_offset;


  /* Get pointer to choices_made array for clarity */
  choices_made_ptr = choices_made;

  /* Cache the choice array bounds for ease of use & speed */
  first_choice = table->first_choice;
  last_choice = table->last_choice;


  /* Commit all the resources for each specified choice */
  choice_ptr = first_choice;
  while (1)
    {

#if DEBUG_RMAP
      printf ("    Committing choice %i:\n",
              (int) (choice_ptr - first_choice));
#endif

      /* Load the option id chosen */
      option_id = *choices_made_ptr;

      /* Load the option array for this choice */
      option_array = choice_ptr->first_option;

      /* Get a pointer to the option selected */
      option_ptr = &option_array[option_id];

#if DEBUG_RMAP
      printf ("      Committing option %i:\n",
              (int) (option_ptr - option_array));
#endif

      /* Cache the usage array bounds for ease of use & speed */
      first_usage = option_ptr->first_usage;
      last_usage = option_ptr->last_usage;

      /* In order to choice this option, all resources used by
       * the usage array must be available in the resource map
       */
      usage_ptr = first_usage;
      while (1)
        {
          /* Cache the usage contents for ease of use & speed */
          usage_offset = usage_ptr->map_offset;
          resources_used = usage_ptr->resources_used;

          /* Calculate the map offset from the usage offset, 
           * start_offset, and the offset_II.  This is the
           * key step in modulo resource usage.
           *
           * Note: this calculation assumes start_offset is non-negative
           * (done in the caller) and that usage_offset it non-negative
           * (done by lmdes2_customizer).  Otherwise, would need
           * to detect negative map_offsets and add offset_II to
           * them to make them legal. -ITI/JCG 8/99
           */
          map_offset = (start_offset + usage_offset) % offset_II;

          /* Get the contents of the resource map at this offset */
          map_contents = map[map_offset];

          /* Mark the resources as used in the map_contents */
          map_contents = map_contents | resources_used;

#if DEBUG_RMAP
          printf
            ("        Committing usage %i (offset %i mask %x). "
             "Before %x after %x\n",
             (int) (usage_ptr - first_usage), map_offset, resources_used,
             map[map_offset], map_contents);
#endif

          /* Store the new map contents to the map */
          map[map_offset] = map_contents;


          /* Goto next usage */
          usage_ptr++;

          /* Stop after committing all the desired resource usages */
          if (usage_ptr > last_usage)
            break;
        }

      /* Update choice_ptr and choices_made_ptr before committing the next 
       * choice 
       */
      choice_ptr++;
      choices_made_ptr++;

      /* Stop if we have committed each choice */
      if (choice_ptr > last_choice)
        break;
    }
}

/* Release the resources previously used by the scheduled operation,
 * by marking them as available.  Punts if any of the resources 
 * being released are already available.
 *
 * Requires that map starts at time/offset 0 and that usage offsets
 * are non-negative (guarenteed by lmdes2_customizer). -ITI/JCG 8/99
 */
void
SM_modulo_release_choices (unsigned int *map, SM_Table * table,
                           unsigned short *choices_made,
                           unsigned int start_offset, unsigned int offset_II)
{
  SM_Choice *choice_ptr, *first_choice, *last_choice;
  SM_Option *option_ptr, *option_array;
  SM_Usage *usage_ptr, *first_usage, *last_usage;
  unsigned short option_id, *choices_made_ptr;
  unsigned int map_contents, resources_used, map_offset, usage_offset;


  /* Get pointer to choices_made array for clarity */
  choices_made_ptr = choices_made;

  /* Cache the choice array bounds for ease of use & speed */
  first_choice = table->first_choice;
  last_choice = table->last_choice;

  /* Release all the resources for each specified choice */
  choice_ptr = first_choice;
  while (1)
    {
      /* Load the option id chosen */
      option_id = *choices_made_ptr;

      /* Load the option array for this choice */
      option_array = choice_ptr->first_option;

      /* Get a pointer to the option selected */
      option_ptr = &option_array[option_id];

      /* Cache the usage array bounds for ease of use & speed */
      first_usage = option_ptr->first_usage;
      last_usage = option_ptr->last_usage;

      /* In order to release this option, all resources used by
       * the usage array must be unavailable in the resource map
       */
      usage_ptr = first_usage;
      while (1)
        {
          /* Cache the usage contents for ease of use & speed */
          usage_offset = usage_ptr->map_offset;
          resources_used = usage_ptr->resources_used;

          /* Calculate the map offset from the usage offset, 
           * start_offset, and the offset_II.  This is the
           * key step in modulo resource usage.
           *
           * Note: this calculation assumes start_offset is non-negative
           * (done in the caller) and that usage_offset it non-negative
           * (done by lmdes2_customizer).  Otherwise, would need
           * to detect negative map_offsets and add offset_II to
           * them to make them legal. -ITI/JCG 8/99
           */
          map_offset = (start_offset + usage_offset) % offset_II;

          /* Get the contents of the resource map at this offset */
          map_contents = map[map_offset];

          /* Sanity check, make sure these resources are all
           * being used.
           */
          if ((map_contents & resources_used) != resources_used)
            L_punt ("SM_release_choices: Resources already free!");

          /* Reverse bits to free resources */
          map_contents = map_contents ^ resources_used;

          /* Store the new map contents to the map */
          map[map_offset] = map_contents;

          /* Goto next usage */
          usage_ptr++;

          /* Stop after releasing all the specified resource usages */
          if (usage_ptr > last_usage)
            break;
        }

      /* Update choice_ptr and choices_made_ptr before releasing the next 
       * choice 
       */
      choice_ptr++;
      choices_made_ptr++;

      /* Stop if we have released each choice */
      if (choice_ptr > last_choice)
        break;
    }
}
