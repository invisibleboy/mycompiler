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
 *      File:   s_stats.c
 *      Author: John Gyllenhaal
 *      Creation Date:  June 1994
 *      Copyright (c) 1994 John Gyllenhaal, Wen-mei Hwu and
 *                         The Board of Trustees of the University of Illinois.
 *                         All rights reserved.
 *      The University of Illinois software License Agreement
 *      specifies the terms and conditions for redistribution.
\*****************************************************************************/

#include <config.h>

#ifndef lint
#define lint
static char copyright[] =
"@(#) Copyright (c) 1994 John Gyllenhaal, Wen-mei Hwu and The Board of \n\
 Trustees of the University of Illinois. All rights reserved.\n";
#endif

#include "s_main.h"

/* Debug */
extern L_Alloc_Pool	*Superscalar_Data_pool;


/* Global parameters for Pstats */
FILE *Pstats_out = NULL;
char *Pstats_rname = NULL;

#define PSTATS_INDENT 71

/* Print out global and region stats with padding and labelling done
 * automatically.  Requires Pstats_out and Pstats_rname to be
 * set before calling.
 * 
 * Cannot handle if newline is put in string.
 */
void Pstats (char *fmt, ...)
{
    va_list     args;
    char	buf[200];
    int		len;

    if (Pstats_out == NULL)
	S_punt ("Pstats: Pstats_out must be set before calling.");

    va_start (args, fmt);
    vsprintf (buf, fmt, args);
    va_end(args);
    
    fprintf (Pstats_out, buf);

    /* If Pstats_rname not NULL, pad out string and print region name */
    if (Pstats_rname != NULL)
    {
	/* Get the length of the string */
	len = strlen (buf);
	
	/* Pad out string until hit PSTATS_INDENT */
	while (len < PSTATS_INDENT)
	{
	    fprintf (Pstats_out, " ");
	    len++;
	}
	
	/* Print out region name */
	fprintf (Pstats_out, Pstats_rname);
    }
    
    fprintf (Pstats_out,"\n");
}

Stats *S_create_stats (char *name)
{
    Stats *stats;
    static int id = 0;

    /* Create stats structure */
    stats = (Stats *) L_alloc (Stats_pool);

    /* Alloc and copy stats name */
    stats->name = strdup (name);

    /* Assign an id */
    stats->id = id;
    id++;

    /* Create each stat subsection */
    stats->region = S_create_stats_region ();
    stats->processor_v = S_create_stats_processor ();
    stats->btb = S_create_stats_btb();
    stats->mcb = S_create_stats_mcb();
    stats->alat = S_create_stats_alat();
    if (S_secondary_cache)
    {
    	stats->L2_bus = S_create_stats_L2_bus ();
    	stats->L2_cache = S_create_stats_L2 ();
    }
    stats->bus = S_create_stats_bus ();
    stats->icache = S_create_stats_icache ();
    stats->dcache = S_create_stats_dcache ();
    stats->histogram = S_create_stats_histogram ();
    
    /* Add to linked list of stats structures */
    stats->next = NULL;
    if (S_tail_stats == NULL)
	S_head_stats = stats;
    else
	S_tail_stats->next = stats;
    S_tail_stats = stats;

    /* Return the initialized structure */
    return (stats);
}

#if 0
/* Templates */
DUMMY_Stats *S_create_stats_DUMMY()
{
    DUMMY_Stats *stats;

    /* Create bus stats structure */
    stats = (DUMMY_Stats *) L_alloc (DUMMY_Stats_pool);

    /* Use STATS_ZERO(...) to initialize stats */

    /* Return the initialized structure */
    return (stats);
}

#endif

Region_Stats *S_create_stats_region()
{
    Region_Stats *stats;

    /* Create preformance stats structure */
    stats = (Region_Stats *) L_alloc (Region_Stats_pool);

    STATS_ZERO(num_entries);
    STATS_ZERO(num_exits);
    STATS_ZERO(num_sim_cycles);
    STATS_ZERO(num_sim_on_path);
    STATS_ZERO(num_skip_on_path);
    STATS_ZERO(billions_skipped);
    STATS_ZERO(num_packets_sim_on_path);
    STATS_ZERO(num_packets_skip_on_path);
    STATS_ZERO(billion_packets_skipped);
    STATS_ZERO(total_preds);
    STATS_ZERO(promoted_preds);
    STATS_ZERO(promoted_unpreds);
    STATS_ZERO(total_preds_squashed);
    STATS_ZERO(promoted_preds_squashed);
    STATS_ZERO(branches);
    STATS_ZERO(untraced_jsrs);
    STATS_ZERO(longjmps);
    STATS_ZERO(promoted_loads);
    STATS_ZERO(promoted_non_trapping_loads);
    STATS_ZERO(promoted_masked_seg_faults);
    STATS_ZERO(promoted_masked_bus_errors);
    STATS_ZERO(promoted_masked_both_traps);
    STATS_ZERO(promoted_added_buffer_pages);
    STATS_ZERO(speculative_loads);
    STATS_ZERO(speculative_non_trapping_loads);
    STATS_ZERO(speculative_masked_seg_faults);
    STATS_ZERO(speculative_masked_bus_errors);
    STATS_ZERO(speculative_masked_both_traps);
    STATS_ZERO(speculative_added_buffer_pages);
    STATS_ZERO(unmarked_loads);
    STATS_ZERO(unmarked_non_trapping_loads);
    STATS_ZERO(unmarked_masked_seg_faults);
    STATS_ZERO(unmarked_masked_bus_errors);
    STATS_ZERO(unmarked_masked_both_traps);
    STATS_ZERO(unmarked_added_buffer_pages);

    /* variables used for calculating stats efficiently */
    stats->entry_cycle = 0;

    /* Return the initialized structure */
    return (stats);
}

MCB_Stats *S_create_stats_mcb()
{
    MCB_Stats *stats;

    /* Create bus stats structure */
    stats = (MCB_Stats *) L_alloc (MCB_Stats_pool);

    /* Use STATS_ZERO(...) to initialize stats */
    STATS_ZERO(beqs);
    STATS_ZERO(loads);
    STATS_ZERO(true_conflicts);
    STATS_ZERO(false_conflicts);
    STATS_ZERO(load_load_conflicts);
    STATS_ZERO(load_store_conflicts);
    STATS_ZERO(context_switch_conflicts);
    STATS_ZERO(load_load_signals);
    STATS_ZERO(load_store_signals);


    /* Return the initialized structure */
    return (stats);
}

ALAT_Stats *S_create_stats_alat()
{
    ALAT_Stats *stats;

    /* Create bus stats structure */
    stats = (ALAT_Stats *) L_alloc (ALAT_Stats_pool);

    /* Use STATS_ZERO(...) to initialize stats */
    STATS_ZERO(beqs);
    STATS_ZERO(loads);
    STATS_ZERO(true_conflicts);
    STATS_ZERO(false_conflicts);
    STATS_ZERO(load_load_conflicts);
    STATS_ZERO(load_store_conflicts);
    STATS_ZERO(context_switch_conflicts);
    STATS_ZERO(load_load_signals);
    STATS_ZERO(load_store_signals);

    /* Return the initialized structure */
    return (stats);
}

Icache_Stats *S_create_stats_icache()
{
    Icache_Stats *stats;

    /* Create bus stats structure */
    stats = (Icache_Stats *) L_alloc (Icache_Stats_pool);

    /* Use STATS_ZERO(...) to initialize stats */
    STATS_ZERO(hits);
    STATS_ZERO(misses);
    STATS_ZERO(blocks_kicked_out);
    STATS_ZERO(split_blocks);

    /* Return the initialized structure */
    return (stats);
}



Histogram_Stats *S_create_stats_histogram()
{
    Histogram_Stats *stats;

    /* Create dcache stats structure */
    stats = (Histogram_Stats *) L_alloc (Histogram_Stats_pool);

    stats->ops_between_branches = 
	L_create_histogram ("Op_between_branch", L_DEFAULT_HISTOGRAM_SIZE);
    stats->cycles_between_branches = 
	L_create_histogram ("Cycle_between_branch", L_DEFAULT_HISTOGRAM_SIZE);
    stats->ops_between_mispredictions = 
	L_create_histogram ("Op_between_mp_branch", L_DEFAULT_HISTOGRAM_SIZE);
    stats->cycles_between_mispredictions = 
	L_create_histogram ("Cycle_between_mp_branch", 
			    L_DEFAULT_HISTOGRAM_SIZE);

    /* Return the initialized structure */
    return (stats);
}






void S_add_stats_region (Region_Stats *dest, 
			     Region_Stats *src1,
			     Region_Stats *src2)
{
    STATS_ADD(num_entries);
    STATS_ADD(num_exits);
    STATS_ADD(num_sim_cycles);
    STATS_ADD(num_sim_on_path);
    STATS_ADD(num_skip_on_path);
    STATS_ADD(billions_skipped);
    STATS_ADD(num_packets_sim_on_path);
    STATS_ADD(num_packets_skip_on_path);
    STATS_ADD(billion_packets_skipped);
    STATS_ADD(total_preds);
    STATS_ADD(promoted_preds);
    STATS_ADD(promoted_unpreds);
    STATS_ADD(total_preds_squashed);
    STATS_ADD(promoted_preds_squashed);
    STATS_ADD(branches);
    STATS_ADD(untraced_jsrs);
    STATS_ADD(longjmps);
    STATS_ADD(promoted_loads);
    STATS_ADD(promoted_non_trapping_loads);
    STATS_ADD(promoted_masked_seg_faults);
    STATS_ADD(promoted_masked_bus_errors);
    STATS_ADD(promoted_masked_both_traps);
    STATS_ADD(promoted_added_buffer_pages);
    STATS_ADD(speculative_loads);
    STATS_ADD(speculative_non_trapping_loads);
    STATS_ADD(speculative_masked_seg_faults);
    STATS_ADD(speculative_masked_bus_errors);
    STATS_ADD(speculative_masked_both_traps);
    STATS_ADD(speculative_added_buffer_pages);
    STATS_ADD(unmarked_loads);
    STATS_ADD(unmarked_non_trapping_loads);
    STATS_ADD(unmarked_masked_seg_faults);
    STATS_ADD(unmarked_masked_bus_errors);
    STATS_ADD(unmarked_masked_both_traps);
    STATS_ADD(unmarked_added_buffer_pages);

    /* Handle int overflow on for num_skip_on_path */
    while (dest->num_skip_on_path > BILLION)
    {
	dest->billions_skipped++;
	dest->num_skip_on_path -= BILLION;
    }

    /* Handle int overflow on for num_packets_skip_on_path */
    while (dest->num_packets_skip_on_path > BILLION)
    {
	dest->billion_packets_skipped++;
	dest->num_packets_skip_on_path -= BILLION;
    }
}


void S_add_stats_mcb (MCB_Stats *dest, 
		      MCB_Stats *src1,
		      MCB_Stats *src2)
{
    STATS_ADD(beqs);
    STATS_ADD(loads);
    STATS_ADD(true_conflicts);
    STATS_ADD(false_conflicts);
    STATS_ADD(load_load_conflicts);
    STATS_ADD(load_store_conflicts);
    STATS_ADD(context_switch_conflicts);
    STATS_ADD(load_load_signals);
    STATS_ADD(load_store_signals);
}

void S_add_stats_alat (ALAT_Stats *dest, 
		       ALAT_Stats *src1,
		       ALAT_Stats *src2)
{
    STATS_ADD(beqs);
    STATS_ADD(loads);
    STATS_ADD(true_conflicts);
    STATS_ADD(false_conflicts);
    STATS_ADD(load_load_conflicts);
    STATS_ADD(load_store_conflicts);
    STATS_ADD(context_switch_conflicts);
    STATS_ADD(load_load_signals);
    STATS_ADD(load_store_signals);
}

void S_add_stats_icache (Icache_Stats *dest, 
			 Icache_Stats *src1,
			 Icache_Stats *src2)
{
    STATS_ADD(hits);
    STATS_ADD(misses);
    STATS_ADD(blocks_kicked_out);
    STATS_ADD(split_blocks);
}


void S_add_stats_histogram (Histogram_Stats *dest, 
			    Histogram_Stats *src1,
			    Histogram_Stats *src2)
{
    L_add_histograms (dest->ops_between_branches,
		      src1->ops_between_branches,
		      src2->ops_between_branches);
    L_add_histograms (dest->cycles_between_branches,
		      src1->cycles_between_branches,
		      src2->cycles_between_branches);
    L_add_histograms (dest->ops_between_mispredictions,
		      src1->ops_between_mispredictions,
		      src2->ops_between_mispredictions);
    L_add_histograms (dest->cycles_between_mispredictions,
		      src1->cycles_between_mispredictions,
		      src2->cycles_between_mispredictions);

}

void S_add_stats (Stats *dest, Stats *src1, Stats *src2)
{
    S_add_stats_region (dest->region, src1->region, 
			    src2->region);
    S_add_stats_bus (dest->bus, src1->bus, 
			    src2->bus);
    S_add_stats_dcache (dest->dcache, src1->dcache, 
			    src2->dcache);
    if (S_secondary_cache)
    {
    	S_add_stats_L2_bus (dest->L2_bus, src1->L2_bus,
                            src2->L2_bus);
    	S_add_stats_L2 (dest->L2_cache, src1->L2_cache,
                            src2->L2_cache);
    }
    S_add_stats_processor (dest->processor_v, src1->processor_v, 
			    src2->processor_v);
    S_add_stats_btb (dest->btb, src1->btb, 
			    src2->btb);
    S_add_stats_mcb (dest->mcb, src1->mcb, 
			    src2->mcb);
    S_add_stats_alat (dest->alat, src1->alat, 
		      src2->alat);
    S_add_stats_icache (dest->icache, src1->icache, 
			    src2->icache);
    S_add_stats_histogram (dest->histogram, src1->histogram,
			   src2->histogram);
}

void S_print_stats_region_region (FILE *out, Stats *stats,
				       char *rname, Stats *total_stats)
{
    double total_ipc, pred_squashed_ipc, sim_ratio;
    double percent_all_pred_squashed, percent_promoted_pred_squashed;
    double num_skipped, num_simmed, total_instr_count;
    double est_cycles;
    double code_ratio, cycle_ratio;
    Region_Stats *rstats;
    char *desc;

    /* Set up Pstats calls */
    Pstats_out = out;
    Pstats_rname = rname;

    /* Get region stats structure for ease of use */
    rstats = stats->region;

    /* Change the description depending on whether the program is
     * using region stats.
     */
    if (S_region_stats)
	desc = "region";
    else
	desc = "program";

    if (rstats->total_preds > 0)
    {
	percent_all_pred_squashed = 100.0 *
	    ((double) rstats->total_preds_squashed/ 
	     ((double) rstats->total_preds));
    }

    else
    {
	percent_all_pred_squashed = 0.0;
    }

    if (rstats->promoted_preds > 0)
    {
	percent_promoted_pred_squashed = 100.0 *
	    ((double) rstats->promoted_preds_squashed/ 
	     ((double) rstats->promoted_preds));
    }

    else
    {
	percent_promoted_pred_squashed = 0.0;
    }



    Pstats ("");
    Pstats ("# NON-TRAPPING LOADS:");
    Pstats ("");
    Pstats ("%12u total load instructions simulated.", 
	    rstats->promoted_loads + rstats->speculative_loads +
	    rstats->unmarked_loads);
    Pstats ("%12u total non-trapping loads simulated.",
	    rstats->promoted_non_trapping_loads + 
	    rstats->speculative_non_trapping_loads +
	    rstats->unmarked_non_trapping_loads);
    Pstats ("%12u total masked traps without buffer pages support.",
	    rstats->promoted_masked_seg_faults + 
	    rstats->speculative_masked_seg_faults +
	    rstats->unmarked_masked_seg_faults +
	    rstats->promoted_masked_bus_errors + 
	    rstats->speculative_masked_bus_errors +
	    rstats->unmarked_masked_bus_errors +
	    rstats->promoted_masked_both_traps + 
	    rstats->speculative_masked_both_traps +
	    rstats->unmarked_masked_both_traps);
    if (S_page_buffer_overflowed)
    {
	Pstats ("%12s total masked traps with buffer pages support.",
		"(overflow)");
    }
    else
    {
	Pstats ("%12u total masked traps with buffer pages support.",
		rstats->promoted_added_buffer_pages + 
		rstats->speculative_added_buffer_pages +
		rstats->unmarked_added_buffer_pages +
		rstats->promoted_masked_bus_errors + 
		rstats->speculative_masked_bus_errors +
		rstats->unmarked_masked_bus_errors +
		rstats->promoted_masked_both_traps + 
		rstats->speculative_masked_both_traps +
		rstats->unmarked_masked_both_traps);
    }
    Pstats ("%12u total non-trapping loads that masked seg fault.",
	    rstats->promoted_masked_seg_faults + 
	    rstats->speculative_masked_seg_faults +
	    rstats->unmarked_masked_seg_faults);
    Pstats ("%12u total non-trapping loads that masked bus error.",
	    rstats->promoted_masked_bus_errors + 
	    rstats->speculative_masked_bus_errors +
	    rstats->unmarked_masked_bus_errors);
    Pstats ("%12u total non-trapping loads that masked both traps.",
	    rstats->promoted_masked_both_traps + 
	    rstats->speculative_masked_both_traps +
	    rstats->unmarked_masked_both_traps);
    if (S_page_buffer_overflowed)
    {
	Pstats ("%12s total non-trapping loads that added buffer page.",
		"(overflow)");
    }
    else
    {
	Pstats ("%12u total non-trapping loads that added buffer page.",
		rstats->promoted_added_buffer_pages +
		rstats->speculative_added_buffer_pages +
		rstats->unmarked_added_buffer_pages);
    }

    Pstats ("");
    Pstats ("%-12s'promoted' includes speculative promoted instructions.",
	    "#");
    Pstats ("%12u promoted load instructions simulated.", 
	    rstats->promoted_loads);
    Pstats ("%12u promoted non-trapping loads simulated.",
	    rstats->promoted_non_trapping_loads);
    Pstats ("%12u promoted masked traps without buffer page support.",
	    rstats->promoted_masked_seg_faults + 
	    rstats->promoted_masked_bus_errors +
	    rstats->promoted_masked_both_traps);
    if (S_page_buffer_overflowed)
    {
	Pstats ("%12s promoted masked traps with buffer page support.",
		"(overflow)");
    }
    else
    {
	Pstats ("%12u promoted masked traps with buffer page support.",
		rstats->promoted_added_buffer_pages + 
		rstats->promoted_masked_bus_errors +
		rstats->promoted_masked_both_traps);
    }
    Pstats ("%12u promoted non-trapping loads that masked seg fault.",
	    rstats->promoted_masked_seg_faults);
    Pstats ("%12u promoted non-trapping loads that masked bus error.",
	    rstats->promoted_masked_bus_errors);
    Pstats ("%12u promoted non-trapping loads that masked both traps.",
	    rstats->promoted_masked_both_traps);
    if (S_page_buffer_overflowed)
    {
	Pstats ("%12s promoted non-trapping loads that added buffer page.",
		"(overflow)");
    }
    else
    {    
	Pstats ("%12u promoted non-trapping loads that added buffer page.",
		rstats->promoted_added_buffer_pages);
    }

    Pstats ("");
    Pstats ("%-12s'speculative' excludes speculative promoted instructions.",
	    "#");
    Pstats ("%12u speculative load instructions simulated.", 
	    rstats->speculative_loads);
    Pstats ("%12u speculative non-trapping loads simulated.",
	    rstats->speculative_non_trapping_loads);
    Pstats ("%12u speculative masked traps without buffer page support.",
	    rstats->speculative_masked_seg_faults +
	    rstats->speculative_masked_bus_errors +
	    rstats->speculative_masked_both_traps);
    if (S_page_buffer_overflowed)
    {
	Pstats ("%12s speculative masked traps with buffer page support.",
		"(overflow)");
    }
    else
    {
	Pstats ("%12u speculative masked traps with buffer page support.",
		rstats->speculative_added_buffer_pages +
		rstats->speculative_masked_bus_errors +
		rstats->speculative_masked_both_traps);
    }
    Pstats ("%12u speculative non-trapping loads that masked seg fault.",
	    rstats->speculative_masked_seg_faults);
    Pstats ("%12u speculative non-trapping loads that masked bus error.",
	    rstats->speculative_masked_bus_errors);
    Pstats ("%12u speculative non-trapping loads that masked both traps.",
	    rstats->speculative_masked_both_traps);
    if (S_page_buffer_overflowed)
    {
	Pstats ("%12s speculative non-trapping loads that added buffer page.",
		"(overflow)");
    }
    else
    {   
	Pstats ("%12u speculative non-trapping loads that added buffer page.",
		rstats->speculative_added_buffer_pages);
    }

    Pstats ("");
    Pstats ("%12u unmarked load instructions simulated.", 
	    rstats->unmarked_loads);
    Pstats ("%12u unmarked non-trapping loads simulated.",
	    rstats->unmarked_non_trapping_loads);
    Pstats ("%12u unmarked masked traps without buffer page support.",
	    rstats->unmarked_masked_seg_faults +
	    rstats->unmarked_masked_bus_errors +
	    rstats->unmarked_masked_both_traps);
    if (S_page_buffer_overflowed)
    {
	Pstats ("%12s unmarked masked traps with buffer page support.",
		"(overflow)");
    }
    else
    {   
	Pstats ("%12u unmarked masked traps with buffer page support.",
		rstats->unmarked_added_buffer_pages +
		rstats->unmarked_masked_bus_errors +
		rstats->unmarked_masked_both_traps);
    }
    Pstats ("%12u unmarked non-trapping loads that masked seg fault.",
	    rstats->unmarked_masked_seg_faults);
    Pstats ("%12u unmarked non-trapping loads that masked bus error.",
	    rstats->unmarked_masked_bus_errors);
    Pstats ("%12u unmarked non-trapping loads that masked both traps.",
	    rstats->unmarked_masked_both_traps);
    if (S_page_buffer_overflowed)
    {
	Pstats ("%12s unmarked non-trapping loads that added buffer page.",
		"(overflow)");
    }
    else
    {   
	Pstats ("%12u unmarked non-trapping loads that added buffer page.",
		rstats->unmarked_added_buffer_pages);
    }

    Pstats ("");
    Pstats ("# INSTRUCTION MIX:");
    Pstats ("");
    Pstats ("%12u branch instructions simulated.", 
	    rstats->branches - rstats->untraced_jsrs);
    Pstats ("%12u untraced jsrs simulated (not considered to be branches).", 
	    rstats->untraced_jsrs);
    Pstats ("%12u longjmps simulated.", 
	    rstats->longjmps);
    Pstats ("");

    Pstats ("%12u total predicated instructions simulated.",
	    rstats->total_preds);
    Pstats ("%12u promoted predicated instructions simulated.",
	    rstats->promoted_preds);
    Pstats ("%12u promoted unpredicated instructions simulated.",
	    rstats->promoted_unpreds);
    Pstats ("%12u total pred squashed instructions simulated.", 
	     rstats->total_preds_squashed);
    Pstats ("%12u promoted pred squashed instructions simulated.", 
	     rstats->promoted_preds_squashed);
    Pstats ("%12.2lf percent of all predicated instructions squashed.",
	     percent_all_pred_squashed);
    Pstats ("%12.2lf percent of promoted predicated instructions squashed.",
	     percent_promoted_pred_squashed);

    /* Calculate some total stats used below */
    total_instr_count = (double) total_stats->region->billions_skipped *
	(double) BILLION;
    total_instr_count += (double) total_stats->region->num_skip_on_path;
    total_instr_count += (double) total_stats->region->num_sim_on_path;


    Pstats ("");
    Pstats ("# PERFORMANCE SUMMARY:");
    Pstats ("");


    /* Calculate number skipped */
    num_skipped = (double) rstats->billions_skipped * (double) BILLION;
    num_skipped += (double) rstats->num_skip_on_path;
    num_simmed = (double) rstats->num_sim_on_path;
    
    if (total_instr_count > 0.1)
	code_ratio = (num_skipped + num_simmed) / total_instr_count;
    else
	code_ratio = 0.0;
    
    if (rstats->num_sim_on_path > 0)
    {
	total_ipc = num_simmed / ((double) rstats->num_sim_cycles);
	pred_squashed_ipc = ((double) rstats->total_preds_squashed) / 
	    ((double) rstats->num_sim_cycles);
	sim_ratio = num_simmed / (num_simmed + num_skipped);
    }
    else
    {
	total_ipc = 0.0;
	sim_ratio = 0.0;
	pred_squashed_ipc = 0.0;
    }
    
    if (sim_ratio < 1.0E-20)
	est_cycles = 0.0;
    else
	est_cycles = ((double) rstats->num_sim_cycles) / sim_ratio;
    
    if (S_sim_cycle > 0)
    {
	cycle_ratio = ((double) rstats->num_sim_cycles) / 
	    ((double) S_sim_cycle);
    }
    else
	cycle_ratio = 0.0;
    
    /* Only print region relavant stats if using regions */
    if (S_region_stats)
    {
	Pstats ("%12u times region entered.",
		rstats->num_entries);
    
	Pstats ("%12u times region exited.",
		rstats->num_exits);
    }

    Pstats ("%12u cycles in %s simulated.",
	    rstats->num_sim_cycles, desc);
    
    Pstats ("%12.0lf instructions in %s simulated on path.",
	    num_simmed, desc);
    
    Pstats ("%12.0lf instructions in %s skipped on path.",
	    num_skipped, desc);

    Pstats ("%12u scheduled operation packets in %s simulated on path.",
	    rstats->num_packets_sim_on_path, desc);
    Pstats ("%12u scheduled operation packets in %s skipped on path.",
	    rstats->num_packets_skip_on_path, desc);
    
    Pstats ("");
    Pstats ("%12.2lf total IPC for %s.",
	    total_ipc, desc);

    Pstats ("%12.2lf executed IPC for %s.", 
	    total_ipc - pred_squashed_ipc, desc);
    Pstats ("%12.2lf pred squashed IPC for %s.",
	    pred_squashed_ipc, desc);

    /* Only print region relavant stats if using regions */
    if (S_region_stats)
    {
	Pstats ("%12.2lf percent of simulated program cycles spent in region.",
		cycle_ratio * 100.0);

	Pstats ("%12.2lf percent of program instructions in region.",
		code_ratio * 100.0);
    }

    Pstats ("%12.2lf percent of %s was simulated.",
	    sim_ratio * 100.0, desc);

    Pstats ("%12.0lf total instruction count of %s.",
	    num_simmed + num_skipped, desc);
    
    /*
     * If exited simulation early due to sample count, print
     * out in estimated cycles so that it is obvious.
     */
    if (S_end_of_program && S_normal_termination)
    {
	
	Pstats ("%12.0lf total estimated execution time for %s (in cycles).",
		est_cycles, desc);
    }
    else
    {
	
	Pstats ("%12.0lf (partial simulation) total estimated execution time for %s (in cycles).",
		est_cycles, desc);
    }

    Pstats ("");
}


void S_print_stats_region_mcb (FILE *out, Stats *stats,
			       char *rname, Stats *total_stats)
{
    MCB_Stats *mstats;
    
    /* Setup Pstats calls */
    Pstats_out = out;
    Pstats_rname = rname;

    /* Get the icache stats structure for ease of use */
    mstats = stats->mcb;

    Pstats ("# MCB:");
    Pstats ("");
    Pstats ("%12s MCB simulation model.", S_MCB_model_name);
    if (S_MCB_model != MCB_MODEL_NO_MCB)
    {
	Pstats ("%12u MCB load instructions.", mstats->loads);
	Pstats ("%12u MCB beq instructions.", mstats->beqs);
	Pstats ("%12u MCB total conflicts.", 
		 mstats->true_conflicts + mstats->false_conflicts);
	Pstats ("%12u MCB true conflicts.",
		 mstats->true_conflicts);
	Pstats ("%12u MCB false conflicts.",
		 mstats->false_conflicts);
	Pstats ("%12u MCB load-load false conflicts.",
		 mstats->load_load_conflicts);
	Pstats ("%12u MCB load-store false conflicts.",
		 mstats->load_store_conflicts);
	Pstats ("%12u MCB context-switch conflicts (ignored).",
		 mstats->context_switch_conflicts);
	Pstats ("%12u MCB load-load conflict signals.",
		 mstats->load_load_signals);
	Pstats ("%12u MCB load-store conflict signals.",
		 mstats->load_store_signals);
    }
    Pstats ("");
}

void S_print_stats_region_alat (FILE *out, Stats *stats,
				char *rname, Stats *total_stats)
{
    ALAT_Stats *mstats;
    
    /* Setup Pstats calls */
    Pstats_out = out;
    Pstats_rname = rname;

    /* Get the alat stats structure for ease of use */
    mstats = stats->alat;

    Pstats ("# ALAT:");
    Pstats ("");
    Pstats ("%12s ALAT simulation model.", S_ALAT_model_name);
    if (S_ALAT_model != ALAT_MODEL_NO_ALAT)
    {
	Pstats ("%12u ALAT load instructions.", mstats->loads);
	Pstats ("%12u ALAT beq instructions.", mstats->beqs);
	Pstats ("%12u ALAT total conflicts.", 
		 mstats->true_conflicts + mstats->false_conflicts);
	Pstats ("%12u ALAT true conflicts.",
		 mstats->true_conflicts);
	Pstats ("%12u ALAT false conflicts.",
		 mstats->false_conflicts);
	Pstats ("%12u ALAT load-load false conflicts.",
		 mstats->load_load_conflicts);
	Pstats ("%12u ALAT load-store false conflicts.",
		 mstats->load_store_conflicts);
	Pstats ("%12u ALAT context-switch conflicts (ignored).",
		 mstats->context_switch_conflicts);
	Pstats ("%12u ALAT load-load conflict signals.",
		 mstats->load_load_signals);
	Pstats ("%12u ALAT load-store conflict signals.",
		 mstats->load_store_signals);
    }
    Pstats ("");
}

void S_print_stats_region_icache (FILE *out, Stats *stats,
			       char *rname, Stats *total_stats)
{
    double icache_hit_ratio;
    Icache_Stats *istats;
    
    /* Setup Pstats calls */
    Pstats_out = out;
    Pstats_rname = rname;

    /* Get the icache stats structure for ease of use */
    istats = stats->icache;

    /* Want to distiguish parameter and result model lines */
    Pstats ("# ICACHE:");
    Pstats ("");
    Pstats ("%12s icache simulation model.", S_icache_model_name);
    Pstats ("%12u icache requests.", 
	     istats->hits + istats->misses);
    Pstats ("%12u icache hits.", istats->hits);
    Pstats ("%12u icache misses.", istats->misses);
    Pstats ("%12u icache blocks kicked out.",
	     istats->blocks_kicked_out);
    Pstats ("%12u icache split block reads.", istats->split_blocks);
    if ((istats->hits + istats->misses)> 0)
    {
	icache_hit_ratio = 100.0 * ((double) istats->hits)/ 
	    ((float) istats->hits + istats->misses);
    }
    else
    {
	icache_hit_ratio = 0.0;
    }
    Pstats ("%12.2lf icache hit ratio.", icache_hit_ratio);
    
    Pstats ("");


}



void S_print_stats_region_histogram (FILE *out, Stats *stats,
				     char *rname, Stats *total_stats)
{
    double scale;
    double sim_ratio, num_simmed, num_skipped;
    Histogram_Stats *hstats;
    Region_Stats *pstats;

    /* Only do if printing histogram stats */
    if (!S_print_branch_histograms)
	return;

    hstats = stats->histogram;
    pstats = stats->region;

    /* Calculate number skipped */
    num_skipped = (double) pstats->billions_skipped * (double) BILLION;
    num_skipped += (double) pstats->num_skip_on_path;
    num_simmed = (double) pstats->num_sim_on_path;

    /* Scale if simulated something is this region */
    if (pstats->num_sim_on_path > 0)
    {
	sim_ratio = num_simmed / (num_simmed + num_skipped);
	scale = 1.00/sim_ratio;
	L_scale_histogram_entries (hstats->ops_between_branches, scale);
	L_scale_histogram_entries (hstats->cycles_between_branches, scale);
	L_scale_histogram_entries (hstats->ops_between_mispredictions, scale);
	L_scale_histogram_entries (hstats->cycles_between_mispredictions, 
				   scale);
    }

    /* Print header for region stats*/
    if (S_region_stats)
	fprintf (S_histogram_file, "\n# %s STATS:\n\n", stats->name);

    L_print_histogram (S_histogram_file, hstats->ops_between_branches,
		       L_PRINT_GROUPED_VALUES, 0.0);
    fprintf (S_histogram_file, "\n");
    L_print_histogram (S_histogram_file, hstats->ops_between_branches,
		       L_PRINT_AVERAGE_VALUE, 0.0);
    fprintf (S_histogram_file, "\n");

    L_print_histogram (S_histogram_file, hstats->ops_between_mispredictions,
		       L_PRINT_GROUPED_VALUES, 0.0);
    fprintf (S_histogram_file, "\n");
    L_print_histogram (S_histogram_file, hstats->ops_between_mispredictions,
		       L_PRINT_AVERAGE_VALUE, 0.0);
    fprintf (S_histogram_file, "\n");

    L_print_histogram (S_histogram_file, hstats->cycles_between_branches,
		       L_PRINT_GROUPED_VALUES, 0.0);
    fprintf (S_histogram_file, "\n");
    L_print_histogram (S_histogram_file, hstats->cycles_between_branches,
		       L_PRINT_AVERAGE_VALUE, 0.0);
    fprintf (S_histogram_file, "\n");

    L_print_histogram (S_histogram_file, hstats->cycles_between_mispredictions,
		       L_PRINT_GROUPED_VALUES, 0.0);
    fprintf (S_histogram_file, "\n");
    L_print_histogram (S_histogram_file, hstats->cycles_between_mispredictions,
		       L_PRINT_AVERAGE_VALUE, 0.0);
    fprintf (S_histogram_file, "\n");
}



/* These system stats must remain global.  */
void S_print_stats_global_system (FILE *out)
{
    double total_time, init_time, sim_time, skip_time;

    fprintf (out, "# PROGRAM:\n");
    fprintf (out, "\n");
    fprintf (out, "%12u trace words read.\n", S_trace_words_read);
    fprintf (out, "%12u sints used in simulation.\n",
	     Sint_pool->allocated);
    fprintf (out, "%12u sints free at end of simulation.\n",
	     Sint_pool->free);
    
    /* For now */
    if (S_processor_model == PROCESSOR_MODEL_SUPERSCALAR)
    {
	fprintf (out, "%12u superscalar datas used in simulation.\n",
		 Superscalar_Data_pool->allocated);
	fprintf (out, "%12u superscalar datas free at end of simulation.\n",
		 Superscalar_Data_pool->free);
    }

    fprintf (out, "\n");

    /* Get the end time */
    S_end_time = time (NULL);
    
    /* Print how long simulation took (if available) */
    if (S_start_time != -1)
    {
	total_time = (double) (S_end_time - S_start_time)/ 60.0;
	init_time =((double)(S_init_time - S_start_time)) / (double) 60.0;
	skip_time = ((double)S_skip_time) / (double) 60.0;
	sim_time = total_time - init_time - skip_time;
    }
    else
    {
	total_time = -1.0;
	init_time = -1.0;
	sim_time = -1.0;
	skip_time = -1.0;
    }
    
    fprintf (out, "# TIME:\n");
    fprintf (out, "\n");
    fprintf (out, "%12.2lf minutes total execution time.\n", 
	     total_time);
    fprintf (out, "%12.2lf minutes initializing %s.\n", init_time,
	     S_mode_name);
    fprintf (out, "%12.2lf minutes simulating instructions.\n", sim_time);
    fprintf (out, "%12.2lf minutes skipping instructions.\n", skip_time);
    fprintf (out, "%12i starting nice value for %s.\n", 
	     S_start_nice_value, S_mode_name);
    fprintf (out, "%12i ending nice value for %s.\n", 
	     S_end_nice_value, S_mode_name);
    fprintf (out, "\n");
}

void S_print_stats_global (FILE *out)
{
    S_print_stats_region_btb (out, S_program_stats, NULL, S_program_stats);
    S_print_stats_region_mcb (out, S_program_stats, NULL, S_program_stats);
    S_print_stats_region_alat (out, S_program_stats, NULL, S_program_stats);
    S_print_stats_region_icache (out, S_program_stats, NULL, S_program_stats);
    S_print_stats_region_dcache (out, S_program_stats, NULL, S_program_stats);
    if (S_secondary_cache)
    {
        S_print_stats_region_L2_bus (out, S_program_stats, NULL, S_program_stats
);
        S_print_stats_region_L2 (out, S_program_stats, NULL, S_program_stats);
    }
    S_print_stats_region_bus (out, S_program_stats, NULL, S_program_stats);
    S_print_stats_region_processor (out, S_program_stats, NULL, S_program_stats);
    S_print_stats_global_system (out);
    S_print_stats_region_region (out, S_program_stats, NULL, S_program_stats);
    S_print_stats_region_histogram (out, S_program_stats, NULL, S_program_stats);

}

void S_print_stats_region (FILE *out)
{
    Stats *stats, *total_stats;
    char rname[100];

    /* Create a "total" stats structure */
    total_stats = S_create_stats ("TOTAL");

    /* total_stats better be last */
    if (total_stats->next != NULL)
	S_punt ("Expected TOTAL stats to be last");
    
    /* Loop thru, summing all the stats (except total) */
    for (stats = S_head_stats; (stats != NULL) && (stats != total_stats);
	 stats = stats->next)
    {
	S_add_stats (total_stats, total_stats, stats);
    }

    S_print_stats_global_system (out);

    /* Print out stats for all regions of code */
    for (stats = S_head_stats; stats != NULL; stats = stats->next)
    {
	if (stats->id == 0)
	    sprintf (rname, "(NOTAG)");
	else if (stats == total_stats)
	    sprintf (rname, "(TOTAL)");
	else
	    sprintf (rname, "(%05i)", stats->id);

	/* Set up the Pstats call */
	Pstats_out = out;
	Pstats_rname = rname;

	fprintf (out, "\n\n");
	Pstats ("#");
	Pstats ("# %s STATS:", stats->name);
	Pstats ("#");
	Pstats ("#");
	S_print_stats_region_btb (out, stats, rname, total_stats);
	S_print_stats_region_mcb (out, stats, rname, total_stats);
	S_print_stats_region_alat (out, stats, rname, total_stats);
	S_print_stats_region_icache (out, stats, rname, total_stats);
	S_print_stats_region_dcache (out, stats, rname, total_stats);
	S_print_stats_region_bus (out, stats, rname, total_stats);
    	if (S_secondary_cache)
    	{
	    S_print_stats_region_L2 (out, stats, rname, total_stats);
	    S_print_stats_region_L2_bus (out, stats, rname, total_stats);
	}
	S_print_stats_region_processor (out, stats, rname, total_stats);
	S_print_stats_region_region (out, stats, rname, total_stats);
	S_print_stats_region_histogram (out, stats, rname, total_stats);
    }
}

