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
 *      File:   convert_hmdes.c
 *              Converts hmdes version1 file to a (barely) usable hmdes 
 *              version2 files
 * 
 *      Author: John C. Gyllenhaal, Wen-mei Hwu
 *      Creation Date:  February 1996
\*****************************************************************************/

/* 10/29/02 REK Adding config.h */
#include <config.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>
#include <string.h>
#include <ctype.h>
#include <library/md.h>
#include <machine/hmdes.h>

extern Header_File header_table[];
extern int num_header_files;

char *program_name;

void
L_punt (char *fmt, ...)
{
  va_list args;

  fprintf (stderr, "Error converting hmdes version1 to version2:\n");
  va_start (args, fmt);
  vfprintf (stderr, fmt, args);
  va_end (args);
  fprintf (stderr, "\n");
  exit (-1);

  return;
}

void
print_usage ()
{
  fprintf (stderr,
	   "usage: %s hmdes_file_name structure_IMPACT.lmdes2 "
	   "hmdes2_file_name [-Dname=value] [-verbose]\n", program_name);
  exit (1);

  return;
}


/* Return entry's pointer and creates entry if necessary */
MD_Entry *
add_entry (MD * md, char *section_name, char *entry_name)
{
  MD_Section *section;
  MD_Entry *entry;

  /* Find the section in the md */
  if ((section = MD_find_section (md, section_name)) == NULL)
    {
      L_punt ("Unable to find section '%s' in '%s'",
	      section_name, entry_name);
    }

  /* Add entry if not already there */
  if ((entry = MD_find_entry (section, entry_name)) == NULL)
    {
      return (MD_new_entry (section, entry_name));
    }
  return (entry);
}

/* Return's entry's pointer, returns NULL if not found */
MD_Entry *
find_entry_if_exists (MD * md, char *section_name, char *entry_name)
{
  MD_Section *section;

  /* Find the section in the md */
  if ((section = MD_find_section (md, section_name)) == NULL)
    {
      L_punt ("Unable to find section '%s' in '%s'",
	      section_name, entry_name);
    }

  /* Return entry name or NULL */
  return (MD_find_entry (section, entry_name));

}

/* Return entry's pointer, punt if not found */
MD_Entry *
find_entry (MD * md, char *section_name, char *entry_name)
{
  MD_Section *section;
  MD_Entry *entry;

  /* Find the section in the md */
  if ((section = MD_find_section (md, section_name)) == NULL)
    {
      L_punt ("Unable to find section '%s' in '%s'", section_name, md->name);
    }

  /* Punt if entry not found */
  if ((entry = MD_find_entry (section, entry_name)) == NULL)
    {
      L_punt ("find_entry: %s->%s not found\n", section_name, entry_name);
    }
  return (entry);
}

/* Adds a value to an entry's field.  Creates the field if necessary */
void
add_value_to_field (MD_Entry * entry, char *field_name, int index,
		    int type, void *value)
{
  MD_Field_Decl *field_decl;
  MD_Field *field;

  /* Get Field declaration */
  if ((field_decl = MD_find_field_decl (entry->section, field_name)) == NULL)
    {
      L_punt ("%s->%s: Field '%s' undefined.\n",
	      entry->section->name, entry->name, field_name);
    }

  /* Create field if not already present */
  if ((field = MD_find_field (entry, field_decl)) == NULL)
    {
      field = MD_new_field (entry, field_decl, 0);
    }

  /* Set field to specified value */
  switch (type)
    {
    case MD_INT:
      MD_set_int (field, index, *((int *) value));
      break;

    case MD_DOUBLE:
      MD_set_double (field, index, *((double *) value));
      break;

    case MD_STRING:
      MD_set_string (field, index, (char *) value);
      break;

    case MD_LINK:
      MD_set_link (field, index, (MD_Entry *) value);
      break;

    default:
      L_punt ("add_value_to_field: Unknown type '%i'!", type);
    }

}

/* Create an empty field */
void
create_empty_field (MD_Entry * entry, char *field_name)
{
  MD_Field_Decl *field_decl;
  MD_Field *field;

  /* Get Field declaration */
  if ((field_decl = MD_find_field_decl (entry->section, field_name)) == NULL)
    {
      L_punt ("%s->%s: Field '%s' undefined.\n",
	      entry->section->name, entry->name, field_name);
    }

  /* Create field if not already present */
  if ((field = MD_find_field (entry, field_decl)) == NULL)
    {
      MD_new_field (entry, field_decl, 0);
    }
}

/* Append a value to the end of an entry's field. 
 * Creates the field if necessary 
 */
void
append_value_to_field (MD_Entry * entry, char *field_name,
		       int type, void *value)
{
  MD_Field_Decl *field_decl;
  MD_Field *field;
  int index;

  /* Get Field declaration */
  if ((field_decl = MD_find_field_decl (entry->section, field_name)) == NULL)
    {
      L_punt ("%s->%s: Field '%s' undefined.\n",
	      entry->section->name, entry->name, field_name);
    }

  /* Create field if not already present */
  if ((field = MD_find_field (entry, field_decl)) == NULL)
    {
      field = MD_new_field (entry, field_decl, 0);
      index = 0;
    }
  else
    {
      index = field->max_element_index + 1;
    }

  /* Set field to specified value */
  switch (type)
    {
    case MD_INT:
      MD_set_int (field, index, *((int *) value));
      break;

    case MD_DOUBLE:
      MD_set_double (field, index, *((double *) value));
      break;

    case MD_STRING:
      MD_set_string (field, index, (char *) value);
      break;

    case MD_LINK:
      MD_set_link (field, index, (MD_Entry *) value);
      break;

    default:
      L_punt ("append_value_to_field: Unknown type '%i'!", type);
    }
}

/* Create processor_model and customization_header parameters */
void
convert_Parameters (MD * md, Hmdes * hmdes)
{
  MD_Entry *parm;
  int i;

  /* Add processor model */
  parm = add_entry (md, "Parameter", "processor_model");

  if (hmdes->processor_model == MDES_SUPERSCALAR)
    append_value_to_field (parm, "value", MD_STRING, "superscalar");
  else if (hmdes->processor_model == MDES_VLIW)
    append_value_to_field (parm, "value", MD_STRING, "vliw");
  else
    L_punt ("Unknown processor model :%i\n", hmdes->processor_model);


  /* Fill in the header files to be used */
  parm = add_entry (md, "Parameter", "customization_headers");
  for (i = 0; i < num_header_files; i++)
    {
      append_value_to_field (parm, "value", MD_STRING, header_table[i].name);
    }
}

/* Returns 1 if name in the set's io_node list, 0 otherwise */
int
in_set (Hmdes_IO_Set * set, char *name)
{
  Hmdes_IO_Node *node;

  for (node = set->head; node != NULL; node = node->next)
    {
      /* Return 1 if match found */
      if (strcmp (name, node->reg_file->name) == 0)
	return (1);
    }

  /* If didn't find it, return 0 */
  return (0);
}

/* Determine if target should be added to source's compatable set.
 * Check that target not already in compatable set, and that all of
 * the targets io_nodes are in the source's io_nodes.
 * Returns 1 if should be added, 0 otherwise.
 */
int
compatable_sets (Hmdes_IO_Set * src_set, Hmdes_IO_Set * target_set)
{
  Hmdes_IO_Node *target_node;

  /* Don't add target_set if already in src set */
  if (in_set (src_set, target_set->name))
    return (0);

  /* Make sure all compatable register files of target are in source */
  for (target_node = target_set->head; target_node != NULL;
       target_node = target_node->next)
    {
      /* If not in src set, not compatable */
      if (!in_set (src_set, target_node->reg_file->name))
	return (0);
    }

  /* If got through all the above tests, must be compatable */
  return (1);
}

/* Create the operation and operand format entries */
void
convert_format (MD * md, Hmdes * hmdes)
{
  Msymbol *old_symbol, *old_target;
  Hmdes_IO_Set *io_set, *target_set;
  Hmdes_IO_Node *io_node;
  Hmdes_IO_Item *io_item;
  MD_Entry *field_type, *op_format, *target;
  int i;

  /* Add all io_sets to the Field_Type section */
  for (old_symbol = hmdes->IO_sets->head; old_symbol != NULL;
       old_symbol = old_symbol->next_linear)
    {
      /* Get old io_set for ease of use */
      io_set = (Hmdes_IO_Set *) old_symbol->ptr;

      /* Add to field type section */
      add_entry (md, "Field_Type", io_set->name);
    }

  /* Fill out the compatable_with field for all io_sets */
  for (old_symbol = hmdes->IO_sets->head; old_symbol != NULL;
       old_symbol = old_symbol->next_linear)
    {
      /* Get old io_set for ease of use */
      io_set = (Hmdes_IO_Set *) old_symbol->ptr;

      /* find the to field type section */
      field_type = find_entry (md, "Field_Type", io_set->name);

      /* Add all compatable register files */
      for (io_node = io_set->head; io_node != NULL; io_node = io_node->next)
	{
	  target = find_entry (md, "Field_Type", io_node->reg_file->name);
	  append_value_to_field (field_type, "compatible_with", MD_LINK,
				 target);
	}

      /* Scan for other compatable field_types (non-register files) to
       * maintain same functionality of old mdes
       */
      for (old_target = hmdes->IO_sets->head; old_target != NULL;
	   old_target = old_target->next_linear)
	{
	  /* Get old target_set for ease of use */
	  target_set = (Hmdes_IO_Set *) old_target->ptr;

	  /* Add target set if compatable with this io set,
	   * may add self to list (this is ok and probably makes the
	   * translation more clear)
	   */
	  if (compatable_sets (io_set, target_set))
	    {
	      target = find_entry (md, "Field_Type", target_set->name);
	      append_value_to_field (field_type, "compatible_with", MD_LINK,
				     target);
	    }
	}
    }

  /* Add all the IO_items to the Operation_Format section */
  for (old_symbol = hmdes->IO_items->head; old_symbol != NULL;
       old_symbol = old_symbol->next_linear)
    {
      /* Get old io_item for ease of use */
      io_item = (Hmdes_IO_Item *) old_symbol->ptr;

      /* Add to operation format section */
      op_format = add_entry (md, "Operation_Format", io_item->name);


      /* Add all src, dest, and predicate field types */
      for (i = 0; i < hmdes->max_src_operands; i++)
	{
	  target = find_entry (md, "Field_Type", io_item->src[i]->name);
	  append_value_to_field (op_format, "src", MD_LINK, target);
	}

      for (i = 0; i < hmdes->max_dest_operands; i++)
	{
	  target = find_entry (md, "Field_Type", io_item->dest[i]->name);
	  append_value_to_field (op_format, "dest", MD_LINK, target);
	}

      for (i = 0; i < hmdes->max_pred_operands; i++)
	{
	  target = find_entry (md, "Field_Type", io_item->pred[i]->name);
	  append_value_to_field (op_format, "pred", MD_LINK, target);
	}
    }
}

MD_Entry *
add_op_lat (MD * md, char *prefix, int latency)
{
  char buf[50];
  MD_Entry *entry;

  /* Create latency name */
  sprintf (buf, "%s%i", prefix, latency);

  /* If already exists, return pointer to existing entry */
  entry = find_entry_if_exists (md, "Operand_Latency", buf);
  if (entry != NULL)
    return (entry);
  else
    {
      /* Create entry and add useage time to it */
      entry = add_entry (md, "Operand_Latency", buf);
      append_value_to_field (entry, "time", MD_INT, &latency);
      return (entry);
    }
}



void
convert_latency (MD * md, Hmdes * hmdes)
{
  Msymbol *old_symbol;
  Hmdes_Latency *old_latency;
  MD_Entry *op_lat, *target;
  int i;

  /* Add all latenciess to the Operation/Operand latency sections */
  for (old_symbol = hmdes->latencies->head; old_symbol != NULL;
       old_symbol = old_symbol->next_linear)
    {
      /* Get old latency for ease of use */
      old_latency = (Hmdes_Latency *) old_symbol->ptr;

      /* Add to operation latency section */
      op_lat = add_entry (md, "Operation_Latency", old_latency->name);

      for (i = 0; i < hmdes->max_src_operands; i++)
	{
	  target = add_op_lat (md, "s", old_latency->src[i]);
	  append_value_to_field (op_lat, "src", MD_LINK, target);
	}

      for (i = 0; i < hmdes->max_dest_operands; i++)
	{
	  target = add_op_lat (md, "d", old_latency->dest[i]);
	  append_value_to_field (op_lat, "dest", MD_LINK, target);
	}

      for (i = 0; i < hmdes->max_pred_operands; i++)
	{
	  target = add_op_lat (md, "p", old_latency->pred[i]);
	  append_value_to_field (op_lat, "pred", MD_LINK, target);
	}

      for (i = 0; i < hmdes->max_src_syncs; i++)
	{
	  target = add_op_lat (md, "ss", old_latency->sync_src[i]);
	  switch (i)
	    {
	    case 0:
	      append_value_to_field (op_lat, "mem_src", MD_LINK, target);
	      break;
	    case 1:
	      append_value_to_field (op_lat, "ctrl_src", MD_LINK, target);
	      break;
	    case 2:
	      append_value_to_field (op_lat, "sync_src", MD_LINK, target);
	      break;
	    default:
	      if (old_latency->sync_src[i] != 0)
		{
		  L_punt ("Latency->%s: converting sync_src[%i] not "
			  "currently supported", op_lat->name, i);
		}
	    }
	}

      for (i = 0; i < hmdes->max_dest_syncs; i++)
	{
	  target = add_op_lat (md, "sd", old_latency->sync_dest[i]);
	  switch (i)
	    {
	    case 0:
	      append_value_to_field (op_lat, "mem_dest", MD_LINK, target);
	      break;
	    case 1:
	      append_value_to_field (op_lat, "ctrl_dest", MD_LINK, target);
	      break;
	    case 2:
	      append_value_to_field (op_lat, "sync_dest", MD_LINK, target);
	      break;
	    default:
	      if (old_latency->sync_dest[i] != 0)
		{
		  L_punt ("Latency->%s: converting sync_dest[%i] not "
			  "currently supported", op_lat->name, i);
		}
	    }
	}
    }

}


void
convert_res_tables (MD * md, Hmdes * hmdes)
{
  Msymbol *old_symbol;
  Hmdes_Resource *old_res;
  Hmdes_Res_Sub *res_sub;
  Hmdes_Res_List *res_list;
  Hmdes_Res_Node *res_node;
  Hmdes_Res_Option *res_option;
  MD_Entry *res_table, *res_usage, *tab_option, *resource;
  int time;
  char name_buf[20000], sub_buf[100];

  /* Add all resources to the Resource section */
  for (old_symbol = hmdes->resources->head; old_symbol != NULL;
       old_symbol = old_symbol->next_linear)
    {
      /* Get old resource for ease of use */
      old_res = (Hmdes_Resource *) old_symbol->ptr;

      /* Create resource for each subscript */
      for (res_sub = old_res->head; res_sub != NULL; res_sub = res_sub->next)
	{
	  sprintf (name_buf, "%s[%i]", old_res->name, res_sub->subscript);
	  resource = add_entry (md, "Resource", name_buf);

	  /* Add slot annotations */
	  if (strcmp (old_res->name, "slot") == 0)
	    {
	      append_value_to_field (resource, "slot", MD_INT,
				     &res_sub->subscript);

	    }
	}
    }

  /* Add all reservation tables to the hmdes2, adding resource
   * usages and table options as necessary.
   */
  for (old_symbol = hmdes->res_lists->head; old_symbol != NULL;
       old_symbol = old_symbol->next_linear)
    {
      /* Get old res_list for ease of use */
      res_list = (Hmdes_Res_List *) old_symbol->ptr;

      /* Create reservation table for each res_list */
      res_table = add_entry (md, "Reservation_Table", res_list->name);

      /* Reservation table may be empty, start reservation table
       * with empty "use" field in case this happens
       */
      create_empty_field (res_table, "use");

      /* Point the slot_options node at head of resource list nodes
       * (So we can print out both slot and non-slot resources
       *  with one loop). 
       */
      res_list->slot_options->next = res_list->head;

      /* Create all the necessary resource usages and table options
       * for this reservation table.  
       */
      for (res_node = res_list->slot_options; res_node != NULL;
	   res_node = res_node->next)
	{
	  /* If have multiple options, create a table option */
	  if (res_node->num_options > 1)
	    {
	      /* Create table option name */
	      for (res_option = res_node->head; res_option != NULL;
		   res_option = res_option->next)
		{
		  if (res_option == res_node->head)
		    {
		      sprintf (name_buf, "any_%s[%i",
			       res_option->resource->name,
			       res_option->subscript->subscript);
		    }
		  else
		    {
		      sprintf (sub_buf, ",%i",
			       res_option->subscript->subscript);
		      strcat (name_buf, sub_buf);
		    }
		}
	      sprintf (sub_buf, "]_t%i_%i", res_node->start_usage,
		       res_node->end_usage);
	      strcat (name_buf, sub_buf);

	      /* If table option already exists, add to reservation
	       * table but skip the rest of the stuff to prevent
	       * adding multiple times */
	      tab_option = find_entry_if_exists (md, "Table_Option",
						 name_buf);
	      if (tab_option != NULL)
		{
		  /* Add to Reservation table */
		  append_value_to_field (res_table, "use", MD_LINK,
					 tab_option);
		  continue;
		}
	      else
		{
		  /* Create tab_option */
		  tab_option = add_entry (md, "Table_Option", name_buf);

		  /* Add to Reservation table */
		  append_value_to_field (res_table, "use", MD_LINK,
					 tab_option);
		}
	    }
	  else
	    {
	      /* Mark that there are no options */
	      tab_option = NULL;
	    }

	  /* Add a resource usage for each option */
	  for (res_option = res_node->head; res_option != NULL;
	       res_option = res_option->next)
	    {
	      sprintf (name_buf, "RU_%s[%i]_t%i_%i",
		       res_option->resource->name,
		       res_option->subscript->subscript,
		       res_node->start_usage, res_node->end_usage);

	      /* Find res_usage entry if already created */
	      res_usage = find_entry_if_exists (md, "Resource_Usage",
						name_buf);
	      /* Only add entry again if does'nt already exists */
	      if (res_usage == NULL)
		{
		  res_usage = add_entry (md, "Resource_Usage", name_buf);

		  /* Set the resource used */
		  sprintf (name_buf, "%s[%i]", res_option->resource->name,
			   res_option->subscript->subscript);
		  resource = find_entry (md, "Resource", name_buf);
		  append_value_to_field (res_usage, "use", MD_LINK, resource);


		  /* Set the times of usage */
		  if (res_node->start_usage <= res_node->end_usage)
		    {
		      for (time = res_node->start_usage;
			   time <= res_node->end_usage; time++)
			{
			  append_value_to_field (res_usage, "time", MD_INT,
						 &time);
			}

		    }
		  else
		    {
		      for (time = res_node->start_usage;
			   time >= res_node->end_usage; time--)
			{
			  append_value_to_field (res_usage, "time", MD_INT,
						 &time);
			}
		    }
		}

	      /* If part of an option, add to option, otherwise to
	       * reservation table 
	       */
	      if (tab_option != NULL)
		{
		  append_value_to_field (tab_option, "one_of", MD_LINK,
					 res_usage);
		}
	      else
		{
		  append_value_to_field (res_table, "use", MD_LINK,
					 res_usage);
		}
	    }
	}


    }
}


void
convert_operations (MD * md, Hmdes * hmdes)
{
  Msymbol *old_symbol;
  Hmdes_Operation *old_op;
  Hmdes_Operation_Node *op_node;
  Hmdes_Class_Node *class_node;
  MD_Entry *pristine_op, *impact_op, *flag, *alt, *res_table, *format, *lat;
  Hmdes_Flag *old_flag;
  char name_buf[1000];
  int id;

  for (old_symbol = hmdes->operations->head; old_symbol != NULL;
       old_symbol = old_symbol->next_linear)
    {
      /* Get old op for ease of use */
      old_op = (Hmdes_Operation *) old_symbol->ptr;

      /* Create pristine op for old_op */
      sprintf (name_buf, "OP_%s", old_op->name);
      pristine_op = add_entry (md, "Operation", name_buf);

      /* Create numbered scheduling alternatives for pristine op */
      id = 0;
      for (op_node = old_op->head; op_node != NULL; op_node = op_node->next)
	{
	  for (class_node = op_node->class->head; class_node != NULL;
	       class_node = class_node->next)
	    {
	      /* Create alt name from op name */
	      sprintf (name_buf, "ALT_%s_%i", old_op->name, id);

	      /* Create scheduling alt */
	      alt = add_entry (md, "Scheduling_Alternative", name_buf);

	      /* Add scheduling alt to pristine op */
	      append_value_to_field (pristine_op, "alt", MD_LINK, alt);

	      /* Link alternative to operation format */
	      format = find_entry (md, "Operation_Format",
				   class_node->io_item->name);

	      append_value_to_field (alt, "format", MD_LINK, format);

	      /* Link alternative to operation latency */
	      lat = find_entry (md, "Operation_Latency",
				class_node->latency->name);

	      append_value_to_field (alt, "latency", MD_LINK, lat);

	      /* Link alternative to reservation table */
	      res_table = find_entry (md, "Reservation_Table",
				      class_node->res_list->name);

	      append_value_to_field (alt, "resv", MD_LINK, res_table);

	      /* Add links to alt flags */
	      for (old_flag = op_node->mdes_flags.head; old_flag != NULL;
		   old_flag = old_flag->next)
		{
		  /* Add flag to IMPACT_Alt_Flag section */
		  flag = add_entry (md, "IMPACT_Alt_Flag", old_flag->name);

		  /* Add to flags field */
		  append_value_to_field (alt, "flags", MD_LINK, flag);
		}


	      /* Go to next id */
	      id++;
	    }
	}


      /* Create impact op for old_op */
      impact_op = add_entry (md, "IMPACT_Operation", old_op->name);

      /* Link to pristine op */
      add_value_to_field (impact_op, "op", 0, MD_LINK, pristine_op);

      /* Add all the operation flags to the impact_op */
      for (old_flag = old_op->op_flags.head; old_flag != NULL;
	   old_flag = old_flag->next)
	{
	  /* Add flag to IMPACT_Operation_Flag section */
	  flag = add_entry (md, "IMPACT_Operation_Flag", old_flag->name);

	  /* Add to flags field */
	  append_value_to_field (impact_op, "flags", MD_LINK, flag);
	}
    }
}

void
convert_hmdes (MD * md, Hmdes * hmdes)
{
  convert_Parameters (md, hmdes);

  convert_format (md, hmdes);

  convert_latency (md, hmdes);

  convert_res_tables (md, hmdes);

  convert_operations (md, hmdes);
}


int
main (int argc, char **argv, char **envp)
{
  char *hmdes_file_name;
  char *structure_file_name;
  char *hmdes2_file_name;
  FILE *out, *structure;
  Hmdes *hmdes;
  int num_args, i;
  MD *md;
  int errors;

  program_name = argv[0];

  num_args = 0;
  for (i = 1; argv[i] != 0; i++)
    {
      if ((argv[i][0] != '-') ||
	  ((argv[i][1] != 'D') && (strcmp (argv[i], "-verbose") != 0)))
	num_args++;
    }

  if (num_args != 3)
    print_usage ();

  /* Make sure first three arguments don't start with '-' */
  if ((argv[1][0] == '-') || (argv[2][0] == '-') || (argv[3][0] == '-'))
    print_usage ();

  hmdes_file_name = argv[1];
  structure_file_name = argv[2];
  hmdes2_file_name = argv[3];

  /* Make sure input and output files are not the same */
  if (strcmp (hmdes_file_name, hmdes2_file_name) == 0)
    H_punt ("May not read and write the same file '%s'\n", hmdes_file_name);

  if (strcmp (structure_file_name, hmdes2_file_name) == 0)
    H_punt ("May not read and write the same file '%s'\n",
	    structure_file_name);

  hmdes = create_hmdes (hmdes_file_name, argv, envp);


  /* Open lmdes2 structure file */
  if ((structure = fopen (structure_file_name, "r")) == NULL)
    L_punt ("Unable to open lmdes2 structure file '%s'.",
	    structure_file_name);

  /* Read in lmdes2 structure file */
  md = MD_read_md (structure, structure_file_name);

  /* Close the input file */
  fclose (structure);

  /* Convert hmdes to hmdes2 format */
  convert_hmdes (md, hmdes);

  /* Make sure format is correct */
  errors = MD_check_md (stderr, md);

  if (errors != 0)
    {
      fprintf (stderr,
	       "***\n*** Please tell John Gyllenhaal:\n***    %i hmdes "
	       "conversion errors when converting\n***    %s\n***\n",
	       errors, hmdes_file_name);
    }

  /* Open output file */
  if ((out = fopen (hmdes2_file_name, "w")) == NULL)
    H_punt ("Error opening '%s' for writing", hmdes2_file_name);

  /* 
   * Write hmdes2 file out 
   */

  /* Print out section and entry declarations so, combined with the
   * output of MD_print_md, the output will be a legal hmdes file.
   */
  MD_print_md_declarations (out, md, 80);

  /* Put blank line after declarations */
  putc ('\n', out);

  /* Print out field declations and entry contents for each section */
  MD_print_md (out, md, 80);

  /* Close the output file */
  fclose (out);

  return (0);
}
