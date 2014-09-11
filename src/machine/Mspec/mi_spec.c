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
 *	File:	mi_spec.c
 *	Author:	Pohua Chang, Wen-mei Hwu
 *	Creation Date:	June, 1990
\*****************************************************************************/

/*****************************************************************************\
 * NOTICE OF CONVENTION                                                      *
 * ------------------------------------------------------------------------- *
 * Mspec links to Pcode, Hcode, and Lcode modules.  In order to allow this   *
 * to take place without requiring front-end modules to link to liblcode.a,  *
 * Mspec code is divided into two classes as follows:                        *
 *  - mi_*.c must not depend on linkage to liblcode.a                        *
 *  - ml_*.c may depend on linkage to liblcode.a                             *
\*****************************************************************************/

/* 10/29/02 REK Adding config.h */
#include <config.h>
#include <Lcode/l_main.h>
#include <machine/m_spec.h>
#include <machine/m_hppa.h>
#include <machine/m_impact.h>
#include <machine/m_hpl_pd.h>
#include <machine/m_sparc.h>
#include <machine/m_x86.h>
#include <machine/m_ti.h>
#include <machine/m_sh.h>
#include <machine/m_bx86.h>
#include <machine/m_tahoe.h>
#include <machine/m_starcore.h>
#include <machine/m_arm.h>
#include <machine/m_wims.h>
#define MD_DEBUG_MACROS		/* Use type-checking versions of md.h macros */
#include <library/md.h>

/*
 *	Global variables
 */
int M_arch = 0;
int M_model = 0;
int M_swarch = 0;

int Mspec_num_int_caller_reg = 0;
int Mspec_num_int_callee_reg = 0;
int Mspec_num_flt_caller_reg = 0;
int Mspec_num_flt_callee_reg = 0;
int Mspec_num_dbl_caller_reg = 0;
int Mspec_num_dbl_callee_reg = 0;
int Mspec_num_prd_caller_reg = 0;
int Mspec_num_prd_callee_reg = 0;

void
M_read_parm (Parm_Parse_Info * ppi)
{
  L_read_parm_i (ppi, "num_int_caller_reg", &Mspec_num_int_caller_reg);
  L_read_parm_i (ppi, "num_int_callee_reg", &Mspec_num_int_callee_reg);
  L_read_parm_i (ppi, "num_flt_caller_reg", &Mspec_num_flt_caller_reg);
  L_read_parm_i (ppi, "num_flt_callee_reg", &Mspec_num_flt_callee_reg);
  L_read_parm_i (ppi, "num_dbl_caller_reg", &Mspec_num_dbl_caller_reg);
  L_read_parm_i (ppi, "num_dbl_callee_reg", &Mspec_num_dbl_callee_reg);
  L_read_parm_i (ppi, "num_prd_caller_reg", &Mspec_num_prd_caller_reg);
  L_read_parm_i (ppi, "num_prd_callee_reg", &Mspec_num_prd_callee_reg);
  L_read_parm_s (ppi, "layout_database_name", &M_layout_database_name);
}

 
/* New portability model reads much of Mspec information from a file
 * generated using a C program built using application structure information,
 * etc., and compiled with the host compiler.  
 * May want to make M_type_database_name a parameter in some modules later.
 * Initially, IMPACT/Lcode is the only arch/model to use this functionality.
 * -ITI/JCG 2/99
 */
int M_use_layout_database = 0;
char *M_layout_database_name = "./host_layout_info.md";

/* Used only by M_read_database_i() and M_read_database_s() */
static MD *M_layout_database = NULL;

void
M_assert (int cc, char *mesg)
{
  if (!cc)
    {
      I_punt ("m_spec: %s\n", mesg);
    }
}

void
M_set_machine (char *arch, char *model, char *swarch)
{
  /* IMPACT/Lcode is the new public (portable) arch/model, 
   * All the other arch/model combinations are being phased out.  -JCG 2/99
   */
  if (!strcasecmp (arch, "impact"))
    {
      M_arch = M_IMPACT;
      M_set_model_impact (model);
    }
  /* Renamed playdoh to HPL-PD, accept variations. -JCG 7/14/98 */
  else if ((strcasecmp (arch, "HPL-PD") == 0) ||
	   (strcasecmp (arch, "HPL_PD") == 0) ||
	   (strcasecmp (arch, "playdoh") == 0))
    {
      M_arch = M_PLAYDOH;
      M_set_model_playdoh (model);
    }
  else if (!strcasecmp (arch, "hppa"))
    {
      M_arch = M_HPPA;
      M_set_model_hppa (model);
    }
#ifndef IMPACT_EXTERNAL
  else if (!strcasecmp (arch, "sparc"))
    {
      M_arch = M_SPARC;
      M_set_model_sparc (model);
    }

  else if (!strcasecmp (arch, "x86"))
    {
      M_arch = M_X86;
      M_set_model_x86 (model);
    }
  else if (!strcasecmp (arch, "ti"))
    {
      M_arch = M_TI;
      M_set_model_ti (model);
    }
  else if (!strcasecmp (arch, "sh"))
    {
      M_arch = M_SH;
      M_set_model_sh (model);
    }
  else if (!strcasecmp (arch, "bx86"))
    {
      M_arch = M_BX86;
      M_set_model_bx86 (model);
    }
  else if (!strcasecmp (arch, "tahoe"))
    {
      M_arch = M_TAHOE;
      M_set_model_tahoe (model);
      M_set_swarch_tahoe (swarch);
    }
  else if (!strcasecmp (arch, "starcore"))
    {
      M_arch = M_STARCORE;
      M_set_model_starcore (model);
    }
#endif
  else if (!strcasecmp (arch, "arm"))
    {
      M_arch = M_ARM;
      M_set_model_arm (model);
    }
  else if (!strcasecmp (arch, "wims") || !strcasecmp(arch, "wims_gen1"))
    {
      M_arch = M_WIMS;
      M_set_model_wims (model);
    }
  else
    {
      fprintf (stderr, "[%s]\n", arch);
      M_assert (0, "unknown M_arch");
    }
}

/*
 *	The following functions are for documentation only.
 *	The function overhead is too great for actual usage.
 */
void
M_char (M_Type type, int unsign)
{
  /* Use layout database (if specified) to determine info -ITI/JCG 2/99 */
  if (M_use_layout_database)
    {
      type->type = M_TYPE_CHAR;
      type->unsign = (unsign != 0);
      type->align = M_read_database_i ("_HT__base_types", "char", "align");
      type->size = M_read_database_i ("_HT__base_types", "char", "size");
      type->nbits = M_read_database_i ("_HT__base_types", "char", "size");
    }
  /* Otherwise use orig hard-coded method to determine info -ITI(JCG) 2/99 */
  else
    {
      switch (M_arch)
	{
	case M_IMPACT:
	  M_impact_char (type, unsign);
	  break;
	case M_TAHOE:
	  M_tahoe_char (type, unsign);
	  break;
	case M_SPARC:
	  M_sparc_char (type, unsign);
	  break;
	case M_HPPA:
	  M_hppa_char (type, unsign);
	  break;
	case M_X86:
	  M_x86_char (type, unsign);
	  break;
	case M_PLAYDOH:
	  M_playdoh_char (type, unsign);
	  break;
	case M_TI:
	  M_ti_char (type, unsign);
	  break;
	case M_SH:
	  M_sh_char (type, unsign);
	  break;
	case M_BX86:
	  M_bx86_char (type, unsign);
	  break;
	case M_STARCORE:
	  M_starcore_char (type, unsign);
	  break;
	case M_ARM:
	  M_arm_char (type, unsign);
	  break;
	case M_WIMS:
	  M_wims_char (type, unsign);
	  break;
	default:
	  M_assert (0, "illegal M_arch type");
	}
    }
  type->flags = 0;
}

void
M_short (M_Type type, int unsign)
{
  /* Use layout database (if specified) to determine info -ITI/JCG 2/99 */
  if (M_use_layout_database)
    {
      type->type = M_TYPE_SHORT;
      type->unsign = (unsign != 0);
      type->align = M_read_database_i ("_HT__base_types", "short", "align");
      type->size = M_read_database_i ("_HT__base_types", "short", "size");
      type->nbits = M_read_database_i ("_HT__base_types", "short", "size");
    }
  /* Otherwise use orig hard-coded method to determine info -ITI(JCG) 2/99 */
  else
    {
      switch (M_arch)
	{
	case M_IMPACT:
	  M_impact_short (type, unsign);
	  break;
	case M_TAHOE:
	  M_tahoe_short (type, unsign);
	  break;
	case M_SPARC:
	  M_sparc_short (type, unsign);
	  break;
	case M_HPPA:
	  M_hppa_short (type, unsign);
	  break;
	case M_X86:
	  M_x86_short (type, unsign);
	  break;
	case M_PLAYDOH:
	  M_playdoh_short (type, unsign);
	  break;
	case M_TI:
	  M_ti_short (type, unsign);
	  break;
	case M_SH:
	  M_sh_short (type, unsign);
	  break;
	case M_BX86:
	  M_bx86_short (type, unsign);
	  break;
	case M_STARCORE:
	  M_starcore_short (type, unsign);
	  break;
	case M_ARM:
	  M_arm_short (type, unsign);
	  break;
	case M_WIMS:
	  M_wims_short (type, unsign);
	  break;
	default:
	  M_assert (0, "illegal M_arch type");
	}
    }
  type->flags = 0;
}

void
M_int (M_Type type, int unsign)
{
  /* Use layout database (if specified) to determine info -ITI/JCG 2/99 */
  if (M_use_layout_database)
    {
      type->type = M_TYPE_INT;
      type->unsign = (unsign != 0);
      type->align = M_read_database_i ("_HT__base_types", "int", "align");
      type->size = M_read_database_i ("_HT__base_types", "int", "size");
      type->nbits = M_read_database_i ("_HT__base_types", "int", "size");
    }
  /* Otherwise use orig hard-coded method to determine info -ITI(JCG) 2/99 */
  else
    {
      switch (M_arch)
	{
	case M_IMPACT:
	  M_impact_int (type, unsign);
	  break;
	case M_TAHOE:
	  M_tahoe_int (type, unsign);
	  break;
	case M_SPARC:
	  M_sparc_int (type, unsign);
	  break;
	case M_HPPA:
	  M_hppa_int (type, unsign);
	  break;
	case M_X86:
	  M_x86_int (type, unsign);
	  break;
	case M_PLAYDOH:
	  M_playdoh_int (type, unsign);
	  break;
	case M_TI:
	  M_ti_int (type, unsign);
	  break;
	case M_SH:
	  M_sh_int (type, unsign);
	  break;
	case M_BX86:
	  M_bx86_int (type, unsign);
	  break;
	case M_STARCORE:
	  M_starcore_int (type, unsign);
	  break;
	case M_ARM:
	  M_arm_int (type, unsign);
	  break;
	case M_WIMS:
	  M_wims_int (type, unsign);
	  break;
	default:
	  M_assert (0, "illegal M_arch type");
	}
    }
  type->flags = 0;
}

void
M_long (M_Type type, int unsign)
{
  /* Use layout database (if specified) to determine info -ITI/JCG 2/99 */
  if (M_use_layout_database)
    {
      type->type = M_TYPE_LONG;
      type->unsign = (unsign != 0);
      type->align = M_read_database_i ("_HT__base_types", "long", "align");
      type->size = M_read_database_i ("_HT__base_types", "long", "size");
      type->nbits = M_read_database_i ("_HT__base_types", "long", "size");
    }
  /* Otherwise use orig hard-coded method to determine info -ITI(JCG) 2/99 */
  else
    {
      switch (M_arch)
	{
	case M_IMPACT:
	  M_impact_long (type, unsign);
	  break;
	case M_TAHOE:
	  M_tahoe_long (type, unsign);
	  break;
	case M_SPARC:
	  M_sparc_long (type, unsign);
	  break;
	case M_HPPA:
	  M_hppa_long (type, unsign);
	  break;
	case M_X86:
	  M_x86_long (type, unsign);
	  break;
	case M_PLAYDOH:
	  M_playdoh_long (type, unsign);
	  break;
	case M_TI:
	  M_ti_long (type, unsign);
	  break;
	case M_SH:
	  M_sh_long (type, unsign);
	  break;
	case M_BX86:
	  M_bx86_long (type, unsign);
	  break;
	case M_STARCORE:
	  M_starcore_long (type, unsign);
	  break;
	case M_ARM:
	  M_arm_long (type, unsign);
	  break;
	case M_WIMS:
	  M_wims_long (type, unsign);
	  break;
	default:
	  M_assert (0, "illegal M_arch type");
	}
    }
  type->flags = 0;
}

void
M_llong (M_Type type, int unsign)
{
  /* Use layout database (if specified) to determine info -ITI/JCG 2/99 */
  if (M_use_layout_database)
    {
      type->type = M_TYPE_LLONG;
      type->unsign = (unsign != 0);
      type->align = M_read_database_i ("_HT__base_types", "longlong",
				       "align");
      type->size = M_read_database_i ("_HT__base_types", "longlong", "size");
      type->nbits = M_read_database_i ("_HT__base_types", "longlong", "size");
    }
  /* Otherwise use orig hard-coded method to determine info -ITI(JCG) 2/99 */
  else
    {
      switch (M_arch)
	{
	case M_TAHOE:
	  M_tahoe_llong (type, unsign);
	  break;
        case M_PLAYDOH:
          M_playdoh_llong(type, unsign);
          break;
	case M_ARM:
          M_arm_llong(type, unsign);
          break;
        case M_WIMS:
          M_wims_llong(type, unsign);
          break;
	case M_IMPACT:
	case M_SPARC:
	case M_HPPA:
	case M_X86:
	case M_TI:
	case M_SH:
	case M_BX86:
	case M_STARCORE:
	default:
	  M_assert (0, "illegal M_arch type");
	}
    }
  type->flags = 0;
}

int
M_type_size (int mtype)
{
  /* Use layout database (if specified) to determine info -ITI(JCG) 2/99 */
  if (M_use_layout_database)
    {
      switch (mtype)
	{
	case M_TYPE_VOID:
	  return 0;		/* No database read necessary, 
				   IMPACT convention */
	case M_TYPE_BIT_LONG:
	case M_TYPE_BIT_SHORT:	/* ITI(JCG) 2/99 */
	case M_TYPE_BIT_CHAR:
	  return 1;		/* No database read necessary, 
				   IMPACT convention */
	case M_TYPE_CHAR:
	  return M_read_database_i ("_HT__base_types", "char", "size");
	case M_TYPE_SHORT:
	  return M_read_database_i ("_HT__base_types", "short", "size");
	case M_TYPE_INT:
	  return M_read_database_i ("_HT__base_types", "int", "size");
	case M_TYPE_LONG:
	  return M_read_database_i ("_HT__base_types", "long", "size");
	case M_TYPE_LLONG:
	  return M_read_database_i ("_HT__base_types", "longlong", "size");
	case M_TYPE_FLOAT:
	  return M_read_database_i ("_HT__base_types", "float", "size");
	case M_TYPE_DOUBLE:
	  return M_read_database_i ("_HT__base_types", "double", "size");
	case M_TYPE_POINTER:
	  return M_read_database_i ("_HT__base_types", "void *", "size");
	case M_TYPE_UNION:
	case M_TYPE_STRUCT:
	case M_TYPE_BLOCK:
	  return -1;		/* No database read necessary, 
				   IMPACT convention */
	default:
	  return -1;
	}
    }
  /* Otherwise use orig hard-coded method to determine info -ITI(JCG) 2/99 */
  else
    {
      switch (M_arch)
	{
	case M_IMPACT:
	  return M_impact_type_size (mtype);
	case M_TAHOE:
	  return M_tahoe_type_size (mtype);
	case M_SPARC:
	  return M_sparc_type_size (mtype);
	case M_HPPA:
	  return M_hppa_type_size (mtype);
	case M_X86:
	  return M_x86_type_size (mtype);
	case M_PLAYDOH:
	  return M_playdoh_type_size (mtype);
	case M_TI:
	  return M_ti_type_size (mtype);
	case M_SH:
	  return M_sh_type_size (mtype);
	case M_STARCORE:
	  return M_starcore_type_size (mtype);
	case M_ARM:
	  return M_arm_type_size (mtype);
	case M_WIMS:
	  return M_wims_type_size (mtype);
	default:
	  M_assert (0, "illegal M_arch type");
	  return -1;
	}
    }
}

int
M_type_align (int mtype)
{
  /* Use layout database (if specified) to determine info -ITI(JCG) 2/99 */
  if (M_use_layout_database)
    {
      switch (mtype)
	{
	case M_TYPE_VOID:
	  return -1;		/* No database read necessary, 
				   IMPACT convention */
	case M_TYPE_BIT_LONG:
	case M_TYPE_BIT_SHORT:	/* ITI(JCG) 2/99 */
	case M_TYPE_BIT_CHAR:
	  return 1;		/* No database read necessary, 
				   IMPACT convention */
	case M_TYPE_CHAR:
	  return M_read_database_i ("_HT__base_types", "char", "align");
	case M_TYPE_SHORT:
	  return M_read_database_i ("_HT__base_types", "short", "align");
	case M_TYPE_INT:
	  return M_read_database_i ("_HT__base_types", "int", "align");
	case M_TYPE_LONG:
	  return M_read_database_i ("_HT__base_types", "long", "align");
	case M_TYPE_LLONG:
	  return M_read_database_i ("_HT__base_types", "longlong", "align");
	case M_TYPE_FLOAT:
	  return M_read_database_i ("_HT__base_types", "float", "align");
	case M_TYPE_DOUBLE:
	  return M_read_database_i ("_HT__base_types", "double", "align");
	case M_TYPE_POINTER:
	  return M_read_database_i ("_HT__base_types", "void *", "align");
	case M_TYPE_UNION:
	case M_TYPE_STRUCT:
	case M_TYPE_BLOCK:
	  return -1;		/* No database read necessary, 
				   IMPACT convention */
	default:
	  return -1;
	}
    }
  /* Otherwise use orig hard-coded method to determine info -ITI(JCG) 2/99 */
  else
    {
      switch (M_arch)
	{
	case M_IMPACT:
	  return M_impact_type_align (mtype);
	case M_TAHOE:
	  return M_tahoe_type_align (mtype);
	case M_SPARC:
	  return M_sparc_type_align (mtype);
	case M_HPPA:
	  return M_hppa_type_align (mtype);
	case M_X86:
	  return M_x86_type_align (mtype);
	case M_PLAYDOH:
	  return M_playdoh_type_align (mtype);
	case M_TI:
	  return M_ti_type_align (mtype);
	case M_SH:
	  return M_sh_type_align (mtype);
	case M_STARCORE:
	  return M_starcore_type_align (mtype);
	case M_ARM:
	  return M_arm_type_align (mtype);
	case M_WIMS:
	  return M_wims_type_align (mtype);
	default:
	  M_assert (0, "illegal M_arch type");
	  return -1;
	}
    }
}

int
M_layout_order (void)
{
  /* This may not need a database query (for now) -ITI/JCG 2/99 */
  switch (M_arch)
    {
    case M_IMPACT:
      return M_impact_layout_order ();
    case M_TAHOE:
      return M_tahoe_layout_order ();
    case M_SPARC:
      return M_sparc_layout_order ();
    case M_HPPA:
      return M_hppa_layout_order ();
    case M_X86:
      return M_x86_layout_order ();
    case M_PLAYDOH:
      return M_playdoh_layout_order ();
    case M_TI:
      return M_ti_layout_order ();
    case M_SH:
      return M_sh_layout_order ();
    case M_BX86:
      return M_bx86_layout_order ();
    case M_STARCORE:
      return M_starcore_layout_order ();
    case M_ARM:
      return M_arm_layout_order ();
    case M_WIMS:
      return M_wims_layout_order();
    default:
      M_assert (0, "illegal M_arch type");
      return -1;
    }
}

/* Returns the description of the layout_database.  String returned must
 * not be modified, freed, etc.
 */
char *
M_layout_database_desc ()
{
  char *desc;
  MD_Section *section;
  MD_Entry *entry;

  if (M_layout_database == NULL)
    return ("(Layout database not loaded!)");

  /* Find description of this database (if present).
   * Passed as the first entry name in the _HT__info section.
   */
  desc = "(no description present)";
  if ((section = MD_find_section (M_layout_database, "_HT__info")) != NULL)
    {
      entry = MD_first_entry (section);
      if (entry != NULL)
	desc = entry->name;
    }

  return (desc);
}

/* Reads and returns the integer field at the given section_name, entry_name, 
 * and field_name given in the layout_database.  Punts on any error. 
 * -ITI/JCG 2/99
 */
int
M_read_database_i (char *section_name, char *entry_name, char *field_name)
{
  FILE *in;
  char *temp_name;
  MD_Section *section;
  MD_Entry *entry;
  MD_Field_Decl *field_decl;
  MD_Field *field;
  int value;

  /* Open and read in layout database on first query */
  if (M_layout_database == NULL)
    {
      /* Get the md file */
      if ((in = fopen (M_layout_database_name, "r")) == NULL)
	{
	  I_punt ("\n"
		  "M_read_database_i: Unable to open '%s' for reading!\n"
		  "Query: Value of %s->%s->%s",
		  M_layout_database_name, section_name, entry_name,
		  field_name);
	}

      /* Read in the layout database */
      M_layout_database = MD_read_md (in, M_layout_database_name);

      /* Done with file, close it */
      fclose (in);
    }

  /* Give troubleshooting message if section not found */
  if ((section = MD_find_section (M_layout_database, section_name)) == NULL)
    {
      I_punt ("\n"
	      "M_read_database_i: Section '%s' not found in '%s'!\n"
	      "\n"
	      "  Database built for: %s\n"
	      "\n"
	      "  Troubleshooting tips:\n"
	      "    1) Verify that *ALL* the source was run through "
	      "gen_CtoP at the same time!\n"
	      "    2) Verify that %s was generated for this benchmark,\n"
	      "       platform, and compiler!\n",
	      section_name, M_layout_database->name,
	      M_layout_database_desc (), M_layout_database->name);
    }


  /* Find field declaration in section */
  if ((field_decl = MD_find_field_decl (section, field_name)) == NULL)
    {
      I_punt ("M_read_database_i: Field decl for '%s' not found in %s->%s!",
	      field_name, M_layout_database->name, section_name);
    }

  /* Find entry in section, strip off quotes if present */
  if (entry_name[0] == '"')
    {
      /* Make copy of entry name so we can delete last quote */
      temp_name = strdup (&entry_name[1]);
      temp_name[strlen (temp_name) - 1] = 0;
      if ((entry = MD_find_entry (section, temp_name)) == NULL)
	{
	  I_punt ("M_read_database_i: Entry '%s' not found in %s->%s!",
		  temp_name, M_layout_database->name, section_name);
	}
      free (temp_name);
    }
  /* Otherwise, use raw entry name */
  else
    {
      if ((entry = MD_find_entry (section, entry_name)) == NULL)
	{
	  I_punt ("M_read_database_i: Entry '%s' not found in %s->%s!",
		  entry_name, M_layout_database->name, section_name);
	}
    }

  /* Make sure field present for this entry */
  if ((field = MD_find_field (entry, field_decl)) == NULL)
    {
      I_punt ("M_read_database_i: Field '%s' not found in %s->%s->%s!",
	      field_name, M_layout_database->name, section_name, entry_name);
    }

  /* Get the first element of the field */
  value = MD_get_int (field, 0);

  /* Return the value */
  return (value);
}

/* Reads and returns the string field at the given section_name, entry_name, 
 * and field_name given in the layout_database.  Punts on any error. 
 * Note: Do not modify or free the returned string! -ITI/JCG 3/99
 */
char *
M_read_database_s (char *section_name, char *entry_name, char *field_name)
{
  FILE *in;
  char *temp_name;
  MD_Section *section;
  MD_Entry *entry;
  MD_Field_Decl *field_decl;
  MD_Field *field;
  char *value;

  /* Open and read in layout database on first query */
  if (M_layout_database == NULL)
    {
      /* Get the md file */
      if ((in = fopen (M_layout_database_name, "r")) == NULL)
	{
	  I_punt ("\n"
		  "M_read_database_s: Unable to open '%s' for reading!\n"
		  "Query: Value of %s->%s->%s",
		  M_layout_database_name, section_name, entry_name,
		  field_name);
	}

      /* Read in the layout database */
      M_layout_database = MD_read_md (in, M_layout_database_name);

      /* Done with file, close it */
      fclose (in);
    }

  /* Give troubleshooting message if section not found */
  if ((section = MD_find_section (M_layout_database, section_name)) == NULL)
    {
      I_punt ("\n"
	      "M_read_database_s: Section '%s' not found in '%s'!\n"
	      "\n"
	      "  Database built for: %s\n"
	      "\n"
	      "  Troubleshooting tips:\n"
	      "    1) Verify that *ALL* the source was run through "
	      "gen_CtoP at the same time!\n"
	      "    2) Verify that %s was generated for this benchmark,\n"
	      "       platform, and compiler!\n",
	      section_name, M_layout_database->name,
	      M_layout_database_desc (), M_layout_database->name);
    }


  /* Find field declaration in section */
  if ((field_decl = MD_find_field_decl (section, field_name)) == NULL)
    {
      I_punt ("M_read_database_s: Field decl for '%s' not found in %s->%s!",
	      field_name, M_layout_database->name, section_name);
    }

  /* Find entry in section, strip off quotes if present */
  if (entry_name[0] == '"')
    {
      /* Make copy of entry name so we can delete last quote */
      temp_name = strdup (&entry_name[1]);
      temp_name[strlen (temp_name) - 1] = 0;
      if ((entry = MD_find_entry (section, temp_name)) == NULL)
	{
	  I_punt ("M_read_database_s: Entry '%s' not found in %s->%s!",
		  temp_name, M_layout_database->name, section_name);
	}
      free (temp_name);
    }
  /* Otherwise, use raw entry name */
  else
    {
      if ((entry = MD_find_entry (section, entry_name)) == NULL)
	{
	  I_punt ("M_read_database_s: Entry '%s' not found in %s->%s!",
		  entry_name, M_layout_database->name, section_name);
	}
    }

  /* Make sure field present for this entry */
  if ((field = MD_find_field (entry, field_decl)) == NULL)
    {
      I_punt ("M_read_database_s: Field '%s' not found in %s->%s->%s!",
	      field_name, M_layout_database->name, section_name, entry_name);
    }

  /* Get the first element of the field */
  value = MD_get_string (field, 0);

  /* Return the value */
  return (value);
}

/* Returns 1 if request info is present in the database, otherwise 
 * returns 0.  Will punt if database itself is not readable/present.
 * -ITI/JCG 2/99
 */
int
M_database_info_present (char *section_name, char *entry_name,
			 char *field_name)
{
  FILE *in;
  char *temp_name;
  MD_Section *section;
  MD_Entry *entry;
  MD_Field_Decl *field_decl;
  MD_Field *field;

  /* Open and read in layout database on first query */
  if (M_layout_database == NULL)
    {
      /* Get the md file */
      if ((in = fopen (M_layout_database_name, "r")) == NULL)
	{
	  I_punt ("\n"
		  "M_database_info_present: Unable to open '%s' for reading!\n"
		  "Query: Value of %s->%s->%s",
		  M_layout_database_name, section_name, entry_name,
		  field_name);
	}

      /* Read in the layout database */
      M_layout_database = MD_read_md (in, M_layout_database_name);

      /* Done with file, close it */
      fclose (in);
    }

  /* Find section name */
  if ((section = MD_find_section (M_layout_database, section_name)) == NULL)
    return (0);


  /* Find field declaration in section */
  if ((field_decl = MD_find_field_decl (section, field_name)) == NULL)
    return (0);

  /* Find entry in section, strip off quotes if present */
  if (entry_name[0] == '"')
    {
      /* Make copy of entry name so we can delete last quote */
      temp_name = strdup (&entry_name[1]);
      temp_name[strlen (temp_name) - 1] = 0;
      entry = MD_find_entry (section, temp_name);
      free (temp_name);
    }
  /* Otherwise, use raw entry name */
  else
    {
      entry = MD_find_entry (section, entry_name);
    }
  if (entry == NULL)
    return (0);

  /* Find entry in section */
  if ((entry = MD_find_entry (section, entry_name)) == NULL)
    return (0);

  /* Make sure field present for this entry */
  if ((field = MD_find_field (entry, field_decl)) != NULL)
    return (0);

  /* Assume if got here, that the info should be present (which is
   * what this routine is designed to detect.
   * M_read_database_i/s() will punt if something is truely wrong.
   */
  return (1);
}

int
M_supports_long_long (void)
{
  switch (M_arch)
    {
    case M_TAHOE:
    case M_PLAYDOH:
    case M_ARM:
    case M_WIMS:
      return 1;
    default:
      return 0;
    }
}

int
M_pointer_size (void)
{
  switch (M_arch)
    {
    case M_TAHOE:
      return 8;
    default:
      return 4;
    }
}
