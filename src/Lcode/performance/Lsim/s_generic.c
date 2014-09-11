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
 *      File:   s_generic.c
 *      Author: John Gyllenhaal
 *      Creation Date:  Aug 1994
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

/* Necessary external declarations */
extern L_Alloc_Pool	*Superscalar_Data_pool;
extern L_Alloc_Pool	*VLIW_Data_pool;
extern L_Alloc_Pool	*playdoh_VLIW_Data_pool;


/* Some of the generic parameters */
int S_num_fetch_stages = 1;

/*
 * Generic processor functions.
 */


/*
 * Read the parameters for all the processor models.
 *
 * Put common processor parameters in this routine.
 */
void S_read_parm_processor (Parm_Parse_Info *ppi)
{
    /* Common processor parameters */
    L_read_parm_s (ppi, "processor_model", &S_processor_model_name);
    L_read_parm_s (ppi, "processor_type", &S_processor_type_name);
    L_read_parm_i (ppi, "fetch_stages", &S_num_fetch_stages);
    
    /* Read processor specific parameters */
    S_read_parm_superscalar (ppi);
    S_read_parm_vliw (ppi);
    S_read_parm_playdoh_vliw (ppi);
}

/*
 * Get the processor model from the processor model name.
 */
void S_set_processor_model ()
{
    /* Convert processor model name into integer */
    if (L_pmatch (S_processor_model_name, "Superscalar"))
    {
	/* Reassign name to beautify output */
	S_processor_model_name = "Superscalar";
	S_processor_model = PROCESSOR_MODEL_SUPERSCALAR;
    }
    else if (L_pmatch (S_processor_model_name, "VLIW") ||
	     L_pmatch (S_processor_model_name, "EPIC"))
    {
	/* Reassign name to beautify output */
	S_processor_model_name = "VLIW";
	S_processor_model = PROCESSOR_MODEL_VLIW;
    }
    /* Kevin W. Rudd's reseach */
    else if (L_pmatch (S_processor_model_name, "NYFO_VLIW"))
    {
	/* Reassign name to beautify output */
	S_processor_model_name = "NYFO_VLIW";
	S_processor_model = PROCESSOR_MODEL_NYFO_VLIW;
    }
    else if (L_pmatch (S_processor_model_name, "PLAYDOH_VLIW"))
    {
	/* Reassign name to beautify output */
	S_processor_model_name = "PLAYDOH_VLIW";
	S_processor_model = PROCESSOR_MODEL_PLAYDOH_VLIW;
    }
    else
    {
	S_punt ("Invalid processor model '%s':  Use Superscalar, VLIW, NYFO_VLIW or PLAYDOH_VLIW.", 
		S_processor_model_name);
    }
    
    /* Convert processor type name into integer */
    if (L_pmatch (S_processor_type_name, "Static") ||
	L_pmatch (S_processor_type_name, "In-Order"))
    {
	/* Reassign name to beautify output */
	S_processor_type_name = "Static";
	S_processor_type = PROCESSOR_TYPE_STATIC;
    }
    else if (L_pmatch (S_processor_type_name, "Dynamic") ||
	L_pmatch (S_processor_type_name, ""))
    {
	/* Reassign name to beautify output */
	S_processor_type_name = "Dynamic";
	S_processor_type = PROCESSOR_TYPE_DYNAMIC;
    }
    else
    {
	S_punt ("Invalid processor type '%s':  Use Static or Dynamic.",
		S_processor_type_name);
    }

}

/*
 * Create proper processor structure and initialize it depending on 
 * processor model.
 */
void *S_create_processor (Pnode *pnode)
{
    void *processor = NULL;

    /* Create processor based on processor model */
    switch (S_processor_model)
    {
      case PROCESSOR_MODEL_SUPERSCALAR:
	processor = (void *) S_create_superscalar (pnode);
	break;

      case PROCESSOR_MODEL_VLIW:
	S_layout_vliw_code ();
	processor = (void *) S_create_vliw (pnode);
	break;

      case PROCESSOR_MODEL_PLAYDOH_VLIW:
	S_layout_playdoh_vliw_code ();
	processor = (void *) S_create_playdoh_vliw (pnode);
	break;

      default:
	S_punt ("S_print_configuration_processor: model '%s' undefined.",
		S_processor_model_name);
    }
    return (processor);
}

/*
 * Print the proper configuration depending on processor model.
 */
void S_print_configuration_processor (FILE *out)
{
    /* Print shared parameters here */
    fprintf (out, "# GENERAL PROCESSOR CONFIGURATION:\n");
    fprintf (out, "\n");
    fprintf (out, "%12s processor model specified.\n",
	     S_processor_model_name);
    fprintf (out, "%12s processor type specified.\n",  
	     S_processor_type_name);
    fprintf (out, "\n");

    switch (S_processor_model)
    {
      case PROCESSOR_MODEL_SUPERSCALAR:
	S_print_configuration_superscalar (out);
	break;

      case PROCESSOR_MODEL_VLIW:
	S_print_configuration_vliw (out);
	break;

      case PROCESSOR_MODEL_PLAYDOH_VLIW:
	S_print_configuration_playdoh_vliw (out);
	break;

      default:
	S_punt ("S_print_configuration_processor: model '%s' undefined.",
		S_processor_model_name);
    }
}

/*
 * Create the proper processor region stats depending on processor model.
 */
void *S_create_stats_processor ()
{
    void *pstats = NULL;

    switch (S_processor_model)
    {
      case PROCESSOR_MODEL_SUPERSCALAR:
	pstats = (void *) S_create_stats_superscalar();
	break;

      case PROCESSOR_MODEL_VLIW:
	pstats = (void *) S_create_stats_vliw();
	break;

      case PROCESSOR_MODEL_PLAYDOH_VLIW:
	pstats = (void *) S_create_stats_playdoh_vliw();
	break;

      default:
	S_punt ("S_create_stats_processor: model '%s' undefined.",
		S_processor_model_name);
    }

    return (pstats);
}

int S_sim_processor (Pnode *pnode, int pc, unsigned int sample_size)
{
    /* If not doing simulation, return now */
    if ((sample_size == 0) && !S_force_sim)
	return (pc);

    /* Get the cycle this region of code was entered */
    pnode->stats->region->entry_cycle = S_sim_cycle + 1;

    /* Use the processor model to determine simulation routine */
    switch (S_processor_model)
    {
      case PROCESSOR_MODEL_SUPERSCALAR:
	pc = S_sim_superscalar (pnode, pc, sample_size);
	break;
	
      case PROCESSOR_MODEL_VLIW:
	pc = S_sim_vliw (pnode, pc, sample_size);
	break;
	
      case PROCESSOR_MODEL_NYFO_VLIW:
	pc = S_sim_nyfo_vliw (pnode, pc, sample_size);
	break;
	
      case PROCESSOR_MODEL_PLAYDOH_VLIW:
	pc = S_sim_playdoh_vliw (pnode, pc, sample_size);
	break;
	
      default:
	S_punt ("S_sim_processor: model '%s' undefined.",
		S_processor_model_name);
    }

    /* 
     * Update cycles spend in region stats.
     */
    pnode->stats->region->num_sim_cycles += (S_sim_cycle + 1) -
	 pnode->stats->region->entry_cycle;
    pnode->stats->region->entry_cycle = S_sim_cycle;
    
    return (pc);
}

/*
 * Add the proper processor region stats depending on processor model.
 */
void S_add_stats_processor (void *dest, 
			      void *src1,
			      void *src2)
{
    switch (S_processor_model)
    {
      case PROCESSOR_MODEL_SUPERSCALAR:
	S_add_stats_superscalar ((struct Superscalar_Stats *) dest, 
				 (struct Superscalar_Stats *) src1,
				 (struct Superscalar_Stats *) src2);
	break;
	
      case PROCESSOR_MODEL_VLIW:
	S_add_stats_VLIW ((struct VLIW_Stats *) dest, 
			  (struct VLIW_Stats *) src1,
			  (struct VLIW_Stats *) src2);
	break;
	
      case PROCESSOR_MODEL_PLAYDOH_VLIW:
	S_add_stats_playdoh_VLIW ((struct VLIW_Stats *) dest, 
			  (struct VLIW_Stats *) src1,
			  (struct VLIW_Stats *) src2);
	break;

      default:
	S_punt ("S_add_stats_processor: model '%s' undefined.",
		S_processor_model_name);
    }
}

/*
 * Print the proper processor region stats depending on processor model.
 */
void S_print_stats_region_processor (FILE *out, Stats *stats,
				     char *rname, Stats *total_stats)
{
    switch (S_processor_model)
    {
      case PROCESSOR_MODEL_SUPERSCALAR:
	S_print_stats_region_superscalar (out, stats,
					  rname, total_stats);
	break;
	
      case PROCESSOR_MODEL_VLIW:
	S_print_stats_region_VLIW (out, stats, rname, total_stats);
	break;
	
      case PROCESSOR_MODEL_PLAYDOH_VLIW:
	S_print_stats_region_playdoh_VLIW (out, stats, rname, total_stats);
	break;

      default:
	S_punt ("S_print_region_stats_processor: model '%s' undefined.",
		S_processor_model_name);
    }
}


void S_free_processor_data (Sint *sint)
{
    /* For fake sints created by the data cache, there is no
     * processor data because they are internal to the dcache.
     * They will have serial_no == -1.
     * See replace_dcache_block() in s_dcache.c.
     */
    if (sint->serial_no == -1)
	return;

    switch (S_processor_model)
    {
      case PROCESSOR_MODEL_SUPERSCALAR:
	L_free (Superscalar_Data_pool, sint->proc_data_v);
	break;
	
      case PROCESSOR_MODEL_VLIW:
	L_free (VLIW_Data_pool, sint->proc_data_v);
	break;
	
      case PROCESSOR_MODEL_PLAYDOH_VLIW:
	L_free (playdoh_VLIW_Data_pool, sint->proc_data_v);
	break;

      default:
	S_punt ("S_free_processor_data: model '%s' undefined.",
		S_processor_model_name);
    }
}





