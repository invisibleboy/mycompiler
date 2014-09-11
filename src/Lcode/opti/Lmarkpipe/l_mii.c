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
 *      File: l_mii.c
 *      Description: Routines to calculate MinII
 *      Creation Date: October, 1994
 *      Author: Daniel Lavery and Noubar Partamian
 * 
 *      Revision Date: Summer 1999
 *      Authors: IMPACT Technologies: Matthew Merten and John Gyllenhaal
\*****************************************************************************/

/* 10/29/02 REK Adding config.h */
#include <config.h>
#include <Lcode/l_markpipe.h>
#include <library/heap.h>

/*****************************************************************************/
/*           Resource MinII                                                  */

static int RU_build_resource_array (SM_Option *, int, int[]);
static void RU_update_usage_count (SM_Cb *, SM_Oper *, int *);
static int Lpipe_determine_res_mii (SM_Cb *);

/*****************************************************************************/
/*           Recurrence MinII                                                */

int RecMinII_total_op_num = 0;	/* number of opers in cb */
SM_Oper **op_array = 0;		/* array of pointers to the opers in cb */
int *MinDist = 0;		/* MinDist array -
				   MinDist[i*RecMinII_total_op_num + j] gives
				   the minimum permissible interval between
				   the time at which i is scheduled and the
				   time at which j is scheduled */

static void Lpipe_initialize_rec_mii (SM_Cb *);
static int Lpipe_max_time_interval (SM_Oper *, SM_Oper *, int);
static int Lpipe_determine_rec_mii (SM_Cb *, int, int);
static void Lpipe_print_MinDist (int);
static void Lpipe_print_MinDistDiag (int);

/*****************************************************************************/
/*           Resource MinII                                                  */

/* Build an array of used resources extracted from the
   bit mask representations. */
static int
RU_build_resource_array (SM_Option * option_ptr,
			 int max_resources, int resource_array[])
{
  SM_Usage *usage_ptr;
  int counter;
  unsigned int res;
  int resource, map_offset;
  int entries = 0;

  /* Cycle through all of the usage entries. */
  for (usage_ptr = option_ptr->first_usage;
       usage_ptr <= option_ptr->last_usage; usage_ptr++)
    {
      res = usage_ptr->resources_used;

      /* Eliminate the time component of the resource. */
      map_offset = usage_ptr->map_offset % lmdes->mdes2->sm_mdes->map_width;

      counter = 32;

      /* Cycle through all of the bits in the resource
         bit mask. */
      while (counter > 0)
	{
	  /* For each bit that is set, add that resource
	     into the array. */
	  if ((res & 1) == 1)
	    {
	      resource = (32 - counter) + (32 * map_offset);

	      resource_array[entries++] = resource;

	      if (entries >= max_resources)
		L_punt ("RU_build_resource_array: "
			"max resources per option exceeded");
	    }

	  res >>= 1;
	  counter--;
	}
    }

  return entries;
}


static void
RU_update_usage_count (SM_Cb * sm_cb, SM_Oper * sm_op, int *ru_count)
{
  int *current_ru_count;	/* ru_count updated with options selected
				   so far for current alternative */
  int *min_ru_count;		/* total resource usage count for the best 
				   alternative found so far. */
  SM_Compatible_Alt *c_alt;
  Mdes_Alt *alt;		/* current alternative */
  SM_Choice *choice_ptr;
  SM_Option *option_ptr, *best_option = NULL;
  int resources[MAX_RESOURCES_PER_OPTION];
  int num_resources;
  int option_max_res_count;
  int choice_max_res_count;
  int alt_max_res_count = -1;
  int max_res_count = -1;
  int index;

  /* Input checking. */
  if (sm_cb == NULL)
    L_punt ("RU_update_usage_count: sm_cb cannot be NULL");
  if (sm_op == NULL)
    L_punt ("RU_update_usage_count: sm_op cannot be NULL");
  if (ru_count == NULL)
    L_punt ("RU_update_usage_count: ru_count cannot be NULL");

  /* Build two temporary resource usage count arrays. */
  current_ru_count = (int *) Lcode_malloc (sizeof (int) *
					   sm_cb->sm_mdes->num_resources);
  min_ru_count = (int *) Lcode_malloc (sizeof (int) *
				       sm_cb->sm_mdes->num_resources);

  /* Examine each alternative. */
  for (c_alt = sm_op->first_compatible_alt; c_alt;
       c_alt = c_alt->next_compatible_alt)
    {
      alt = c_alt->normal_version;

      /* No need to check the alt flags since there are no
         speculative instructions at this point.  Just use the
         normal version of the instruction. */

      /* Need a new working copy of ru_count */
      for (index = 0; index < lmdes->num_resources; index++)
	current_ru_count[index] = ru_count[index];

      /* Each choice represents a decision to be made, selecting one of the
         options in that choice. */
      for (choice_ptr = alt->table->first_choice;
	   choice_ptr <= alt->table->last_choice; choice_ptr++)
	{
	  choice_max_res_count = -1;

	  /* Each option uses a number of resources.  When selecting an option,
	     all resources (represented by usages) must be available. */
	  for (option_ptr = choice_ptr->first_option;
	       option_ptr <= choice_ptr->last_option; option_ptr++)
	    {
	      option_max_res_count = -1;

	      num_resources =
		RU_build_resource_array (option_ptr,
					 MAX_RESOURCES_PER_OPTION, resources);

	      /* Find the resource in this option that has the highest usage
	         this far. */
	      for (index = 0; index < num_resources; index++)
		{
		  if (resources[index] >= lmdes->num_resources)
		    L_punt("RU_update_usage_count (1): "
			   "Attempting to access resource %d which is "
			   "larger than num resources %d.",
			   resources[index], lmdes->num_resources);

		  if (option_max_res_count <
		      current_ru_count[resources[index]])
		    {
		      option_max_res_count =
			current_ru_count[resources[index]];
		    }
		}

	      /* Select the option that has the lowest maximum. */
	      if (choice_max_res_count == -1 ||
		  option_max_res_count < choice_max_res_count)
		{
		  best_option = option_ptr;
		  choice_max_res_count = option_max_res_count;
		}
	    }

	  /* Commit the best option to the current_ru_count array. */
	  num_resources = RU_build_resource_array (best_option,
						   MAX_RESOURCES_PER_OPTION,
						   resources);
	  for (index = 0; index < num_resources; index++)
	    {
	      if (resources[index] >= lmdes->num_resources)
		L_punt("RU_update_usage_count (2): "
		       "Attempting to access resource %d which is "
		       "larger than num resources %d.",
		       resources[index], lmdes->num_resources);
	      
	      current_ru_count[resources[index]]++;
	    }
	}

      /* Now that all choices have been made, scan for the maximum usage
         of a resource (resII for this alt). */
      for (index = 0; index < lmdes->num_resources; index++)
	{
	  if (alt_max_res_count == -1 ||
	      current_ru_count[index] > alt_max_res_count)
	    alt_max_res_count = current_ru_count[index];
	}

      /* Choose the alt that has the lowest maximum resource usage.
         If this alt has a lower resource usage than other alts, then
         save it.
       */

      if (max_res_count == -1 || alt_max_res_count < max_res_count)
	{
	  max_res_count = alt_max_res_count;

	  for (index = 0; index < lmdes->num_resources; index++)
	    min_ru_count[index] = current_ru_count[index];
	}
    }

  /* Copy the final count back to ru_count */
  for (index = 0; index < lmdes->num_resources; index++)
    ru_count[index] = min_ru_count[index];

  Lcode_free (min_ru_count);
  Lcode_free (current_ru_count);

  return;
}


/* Compute the resource-based lower bound on the II.  Sort opers in order
   of the increasing number of alternatives.   Then for each oper, add
   the resource usage of the alternative chosen by the RU_manager to the
   total resource usage.  Most used resource then determines ResMinII. */
static int
Lpipe_determine_res_mii (SM_Cb * sm_cb)
{
  SM_Oper *sm_oper;
  int num_alts;			/* Number of scheduling alts for oper. */
  int i;
  int max;
  Heap *heap;
  SM_Compatible_Alt *alts;
  int *ru_count;

  /* Create the array of resource usages. */
  ru_count = (int *) Lcode_malloc (sizeof (int) * lmdes->num_resources);

  /* Create the op heap. */
  heap = Heap_Create (HEAP_MIN);
  /* Sort the ops by the number of alternatives. */
  for (sm_oper = sm_cb->first_serial_op; sm_oper != NULL;
       sm_oper = sm_oper->next_serial_op)
    {
      if (L_START_STOP_NODE(sm_oper->lcode_op))
	continue;

      num_alts = 0;
      alts = sm_oper->first_compatible_alt;
      while (alts)
	{
	  num_alts++;
	  alts = alts->next_compatible_alt;
	}
      Heap_Insert (heap, (void *) sm_oper, num_alts);
    }
  /* Initialize usage count for each resource. */
  for (i = 0; i < sm_cb->sm_mdes->num_resources; i++)
    {
      ru_count[i] = 0;
    }
  /* Get resource usage for each oper and add to total */
  while ((sm_oper = (SM_Oper *) Heap_ExtractTop (heap)) != NULL)
    {
      RU_update_usage_count (sm_cb, sm_oper, ru_count);
    }
  /* ResMinII is MAX(usage count for each resource) */
  max = ru_count[0];
  for (i = 1; i < lmdes->num_resources; i++)
    {
      if (ru_count[i] > max)
	max = ru_count[i];
    }

  Heap_Dispose (heap, NULL);
  Lcode_free (ru_count);

  return (max);
}

/*****************************************************************************/
/*           Recurrence MinII                                                */

/* initialize array of pointers to opers used by Lpipe_determine_rec_mii */
static void
Lpipe_initialize_rec_mii (SM_Cb * sm_cb)
{
  SM_Oper *sm_op;
  int i = 0;

  Lpipe_free_op_array();

  op_array = (SM_Oper **) Lcode_malloc (sm_cb->op_count * sizeof (SM_Oper *));

  if (op_array == NULL)
    L_punt ("Lpipe_initialize_rec_mii: malloc out of space, cb = %d\n",
	    sm_cb->lcode_cb->id);

  for (sm_op = sm_cb->first_serial_op; sm_op != NULL;
       sm_op = sm_op->next_serial_op)
    {
      op_array[i++] = sm_op;
    }

  RecMinII_total_op_num = i;
}


/* If there is a dependence between from_oper and to_oper, return the
   time interval required between the start of from_oper and the
   start of to_oper, *in the same iteration* - i.e. Distance - II * Omega.
   If there are multiple dependences between the two opers, return the
   dependence which requires the largest time interval between the
   two opers.  Otherwise, return NEGATIVE_INFINITY */
static int
Lpipe_max_time_interval (SM_Oper * from_sm_op, SM_Oper * to_sm_op, int MinII)
{
  int max_time_interval = NEGATIVE_INFINITY;
  int temp_time_interval;
  SM_Reg_Action *action;
  SM_Dep *dep_out;
  SM_Oper *temp_sm_op;

  if (from_sm_op->ignore)
    L_punt ("Cannot enable ignored deps out until from_sm_op is enabled.");
  if (to_sm_op->ignore)
    L_punt ("Cannot enable ignored deps out until to_sm_op is enabled.");

  for (action = from_sm_op->first_op_action; action != NULL;
       action = action->next_op_action)
    {
      for (dep_out = action->first_dep_out; dep_out != NULL;
	   dep_out = dep_out->next_dep_out)
	{
	  temp_sm_op = dep_out->to_action->sm_op;

	  if (temp_sm_op == to_sm_op && !dep_out->ignore)
	    {
	      temp_time_interval = dep_out->min_delay -
		MinII * dep_out->omega;

	      if (temp_time_interval > max_time_interval)
		{
		  max_time_interval = temp_time_interval;
		}
	    }
	}
    }

  return (max_time_interval);
}


/* compute MinDist and RecMinII */
static int
Lpipe_determine_rec_mii (SM_Cb * sm_cb, int MinII, int max_MinII)
{
  if (MinII == 0)
    L_punt ("Lpipe_determine_rec_mii: II of 0 does not make sense, "
	    "function = %s, cb = %d\n", L_fn->name, sm_cb->lcode_cb->id);

  for (; MinII <= max_MinII; MinII++)
    {
      if (!Lpipe_build_MinDist (sm_cb, MinII))
	break;
    }

#if 0
  if (Lpipe_debug >= 2)
    Lpipe_print_MinDist (sm_cb->lcode_cb->id);
  if (Lpipe_debug >= 1)
    Lpipe_print_MinDistDiag (sm_cb->lcode_cb->id);
#endif

  if( MinII > max_MinII )
    return -1;
  else
    return (MinII);
}

#define MD(r,c) (MinDist[(r)*RecMinII_total_op_num + (c)])

/* compute MinDist and RecMinII */
int
Lpipe_build_MinDist (SM_Cb * sm_cb, int ii)
{
  int i, j, k, dik, dkj, dij;

  if (ii == 0)
    L_punt ("Lpipe_build_MinDist: II of 0 does not make sense, "
	    "function = %s, cb = %d\n", L_fn->name, sm_cb->lcode_cb->id);

  Lpipe_free_MinDist();

  MinDist =
    (int *) Lcode_malloc ((RecMinII_total_op_num * RecMinII_total_op_num) *
			  sizeof (int));

  if (MinDist == NULL)
    L_punt ("Lpipe_determine_rec_mii: malloc out of space, cb = %d\n",
	    sm_cb->lcode_cb->id);

  SM_set_cb_II (sm_cb, ii);

  /* initialize MinDist array */
  for (i = 0; i < RecMinII_total_op_num; i++)
    for (j = 0; j < RecMinII_total_op_num; j++)
      MD(i,j) = Lpipe_max_time_interval (op_array[i], op_array[j], ii);

  /* compute MinDist */
  for (k = 0; k < RecMinII_total_op_num; k++)
    {
      for (i = 0; i < RecMinII_total_op_num; i++)
	{
	  if ((dik = MD(i,k)) == NEGATIVE_INFINITY)
	    continue;

	  for (j = 0; j < RecMinII_total_op_num; j++)
	    {
	      if ((dkj = MD(k,j)) == NEGATIVE_INFINITY)
		continue;

	      dij = dik + dkj;

	      if (dij > MD(i,j))
		{
		  MD(i,j) = dij;

		  if (i == j && dij > 0)
		    return 1; /* Failure */

		  if (j == k)
		    dik = dij;
		}
	    }
	}
    }

  /* Success */
  return 0;
}

static void
Lpipe_print_MinDist (int id)
{
  int i, j;

  /* ITI/MCM Print the MinDist Matrix */

  printf ("\n*** Begin MinDist for cb %d ***\n", id);
  /* initialize MinDist array */
  for (i = -1; i < RecMinII_total_op_num; i++)
    {
      if (i == -1)
	{
	  /* Print header */
	  printf ("        ");
	  for (j = 0; j < RecMinII_total_op_num; j++)
	    printf ("%4d ", op_array[j]->lcode_op->id);
	}
      else
	{
	  for (j = -1; j < RecMinII_total_op_num; j++)
	    {
	      if (j == -1)
		{
		  printf ("%7d:", op_array[i]->lcode_op->id);
		}
	      else
		{
		  int val = MinDist[i * RecMinII_total_op_num + j];

		  if (val != NEGATIVE_INFINITY)
		    printf ("%4d ", val);
		  else
		    printf ("   - ");
		}
	    }
	}
      printf ("\n\n");
    }

  printf ("*** End MinDist for cb %d ***\n\n", id);

  return;
}


static void
Lpipe_print_MinDistDiag (int id)
{
  int i;

  /* JWS Print the MinDist Matrix Diagonal Only */

  printf ("*** Begin MinDistDiag for cb %d ***\n", id);

  for (i = 0; i < RecMinII_total_op_num; i++)
    {
      printf ("OP %4d: %6d\n", op_array[i]->lcode_op->id,
	      MinDist[i * (RecMinII_total_op_num + 1)]);
    }

  printf ("\n*** End MinDist for cb %d ***\n", id);

  return;
}


/*****************************************************************************/
/*           External Functions                                              */


Softpipe_MinII *
Lpipe_determine_mii (SM_Cb * sm_cb, int max_MinII)
{
  Softpipe_MinII *MinII =
    (Softpipe_MinII *) Lcode_malloc (sizeof (Softpipe_MinII));
  L_Attr *attr = NULL;

  /* Compute resource MinII */

  MinII->res_MinII = Lpipe_determine_res_mii (sm_cb);

  /* Compute recurrence MinII */

  Lpipe_initialize_rec_mii (sm_cb);

  /* Eventually start from ResMinII to reduce computation.
     For now, start from 1 to determine actual RecMinII for
     statistics purposes.  */
  MinII->rec_MinII = Lpipe_determine_rec_mii (sm_cb, 1, max_MinII);

  /* Check for IIs that are too large. */

  if (MinII->rec_MinII == -1 ||
      MinII->rec_MinII > max_MinII ||
      MinII->res_MinII > max_MinII)
    {
      MinII->MinII = -1;
      return MinII;
    }

  /* Compute estimated MinII */

  MinII->MinII = (MinII->res_MinII > MinII->rec_MinII) ?
    MinII->res_MinII : MinII->rec_MinII;

  /* Compute effective MinII for unrolled loops */

  attr = L_find_attr (sm_cb->lcode_cb->attr, "unroll_AMP");
  if (attr != NULL)
    {
      MinII->eff_MinII =
	((float) MinII->MinII) / ((float) attr->field[0]->value.i);
    }
  else
    {
      /* Assume unroll_AMP = 1 */
      MinII->eff_MinII = ((float) MinII->MinII);
    }

  return MinII;
}


int
Lpipe_get_MinDist (int i, int j)
{
  return (MinDist[i * RecMinII_total_op_num + j]);
}

int
Lpipe_get_DepHeight()
{
  return (MinDist[RecMinII_total_op_num-1]);
}

void Lpipe_free_MinDist()
{
  /* make sure old MinDist matrix is freed before creating a new one */
  if (MinDist != NULL)
    {
      Lcode_free (MinDist);
      MinDist = NULL;
   }

  return;;
}

void Lpipe_free_op_array()
{
  /* make sure old array is freed before creating a new one */
  if (op_array != NULL)
    {
      Lcode_free (op_array);
      op_array = NULL;
    }
  
  return;
}
