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
 *
 *  File:  l_parms.c
 *
 *  Description:  Reads in parameters 
 *
 *  Creation Date :  10-6-92
 *
 *  Authors:  John C. Gyllenhaal, Wen-mei Hwu
 *
 *  Revisions:
 *      Roger A. Bringmann - February 1993
 *      Modified for standard use within Lcode.
 *
 *      John C. Gyllenhaal - September 1993
 *      Added standard functions for reading command line and environment
 *      macro definitions and for determining the STD_PARMS file to use.
 * 
 *      Brian Deitrich - September 1994
 *      Added hash table for parameters (only read parameters into routines
 *      after all parameters have been read in.  Added more warnings to help
 *      in debugging of parameter files.
 *
 *      John C. Gyllenhaal - May 1998
 *      Enhanced load parameters to accept up to two equivalent names
 *      for a section.  This functionality was added to allow the Trimaran
 *      release to "rename" some IMPACT parameter sections (namely,
 *      global, file, arch, RegAlloc, Dependence, and Scheduler). 
 *
 *      John C. Gyllenhaal - Oct 1998
 *      Added 'default' defaults_path specifier '(* defaults_path' to parm 
 *      reader.  Allows users to override just a few parameters and get the 
 *      rest (including other unspecified parm sections) from a different file.
 *      Before this enhancement, every section was required to be in every 
 *      file, which was tiresome. :)  Added to support new benchmark compile 
 *      info specification framework.
 *
 *      Copyright (c) 1993 The Board of Trustees of the University of Illinois.
 *                         All rights reserved.
 *      The University of Illinois software License Agreement
 *      specifies the terms and conditions for redistribution.
\*****************************************************************************/

/* 10/29/02 REK Adding config.h */
#include <config.h>
#include <stdio.h>
#include <stdarg.h>
#include <string.h>
#include <ctype.h>
#include <math.h>
#include <stdlib.h>
#include <malloc.h>
#include <library/i_error.h>

#include "l_parms.h"

/* Internal prototypes */
extern Parm_Table_List *L_create_parm_table_list (void);
extern void L_free_parm_table_list (Parm_Table_List * list);
extern Parm_Table_Node *L_get_parm_table_entry (Parm_Table_List * list,
                                                char *name);
extern void L_add_to_parm_table_list (Parm_Table_List * list,
                                      Parm_Parse_Info * ppi,
                                      char *section_name1,
                                      char *section_name2);
extern int L_pmatch (char *s1, char *s2);
extern Parm_Macro_List *L_create_parm_macro_list (void);


/* GLOBAL VARIABLE DEFINITION */

/* Global variables to configure parameter file reader */
int L_punt_on_unknown_parm = 0; /* Default to warn on parm_unknown */


/* The following variables are needed so that people do not need to alter
   their parameter loading programs very much. These globals means that
   parameters do not need to be added to the routines that are already in
   place. */
Parm_Warn_List *warn_list;   /* pointer to list of all warnings encountered. */
char *current_section_name1;    /* name of section currently being parsed. */
char *current_section_name2;    /* aliased version of section name. */

int section_name1_used = 0;  /* Indicates if "name1" used while reading parms */
int section_name2_used = 0;  /* Indicates if "name2" used while reading parms */

Parm_Table_List *parameter_list;    /* pointer to list of hash table pointers */

/* Global variable that signifies start of parameters list for section. List
   is designed so that the order of the list corresponds to the order that
   these elements were defined in the parameters file. */
Parm_Table_Node *start_node_of_input;

/* used to know to print "#PARAMETER DUMP" the first time this is invoked */
static int first_time = 1;

/* variables used to process the parameter warnings and output */
char *L_parm_dump_file_name = (char *) "stderr";
char *L_parm_warn_file_name = (char *) "stderr";
int L_warn_old_parm_section_name_used = WARNINGS_OFF;
int L_warn_parm_not_used = WARNINGS_ON;
int L_warn_parm_not_defined = WARNINGS_ON;
int L_warn_dev_parm_not_defined = WARNINGS_OFF;
int L_warn_parm_defined_twice = WARNINGS_ON;
int L_dump_parms = WARNINGS_OFF;

/* END OF GLOBAL VARIABLE DEFINITION */

/* Declare external functions that must be defined by user */
void I_punt (char *msg, ...);

/* Internal prototypes */
static int L_get_next_parm (Parm_Parse_Info * ppi);
void L_define_parm_macro (Parm_Macro_List * list, char *name, char *value,
                          int from);
void L_read_parm_sections (Parm_Parse_Info * ppi, Parm_Section_Info * psi);
int L_parm_calc_complete_expr (Parm_Parse_Info * ppi);
int L_parm_calc_expr (Parm_Parse_Info * ppi);
int L_parm_calc_factor (Parm_Parse_Info * ppi);
int L_parm_calc_term (Parm_Parse_Info * ppi);
int L_peek_next_token (Parm_Parse_Info * ppi);
int L_get_next_token (Parm_Parse_Info * ppi);
void L_add_parm_already_defined (Parm_Table_Node * old_parm_node,
                                 char *new_value, char *new_file_name,
                                 int new_parm_line_no, char *section_name1,
                                 char *section_name2);

/*******************************/
/* Hash table routines follow. */
/*******************************/

/* Initializes the hash table */
Parm_Table_List *
L_create_parm_table_list (void)
{
  int i;
  Parm_Table_List *list;

  if ((list = (Parm_Table_List *) malloc (sizeof (Parm_Table_List))) == NULL)
    I_punt ("Parm_Table_List: out of memory\n");

  for (i = 0; i < PARM_TABLE_HASH_SIZE; i++)
    list->head[i] = NULL;

  return (list);
}

/* Frees all the definitions in a hash table list */
void
L_free_parm_table_list (Parm_Table_List * list)
{
  int i;
  Parm_Table_Node *node, *next_node;

  for (i = 0; i < PARM_TABLE_HASH_SIZE; i++)
    {
      for (node = list->head[i]; node != NULL; node = next_node)
        {
          next_node = node->next;
          free (node->name);
          free (node->user_name);
          free (node->value);
          free (node->parm_file_name);
          free (node);
        }
    }
  free (list);
}

/* Return the ppi pointer in the hash table found under a particular name
 * Also set the used field to USED when entry is found
 * If no entry can be found, return NULL
 */
Parm_Table_Node *
L_get_parm_table_entry (Parm_Table_List * list, char *name)
{
  Parm_Table_Node *node, *next_node;
  int hash_value, i, name_size;
  char *change_case_char, *name_to_check;

  /* Strip leading whitespaces */
  while (isspace (*name))
    name++;

  /* Save the original name provided by the user (before it is converted to
     all caps. */
  name_size = strlen (name) + 1;
  if ((name_to_check = (char *) malloc (name_size)) == NULL)
    I_punt ("L_get_parm_table_entry: Out of memory");
  strcpy (name_to_check, name);

  /* change all letters in name to caps */
  change_case_char = name_to_check;
  while (*change_case_char != '\0')
    {
      *change_case_char = toupper (*change_case_char);
      change_case_char++;
    }

  /* calculate hash value */
  hash_value = 0;
  for (i = 0; i < name_size; i++)
    hash_value = hash_value + *(name_to_check + i);
  hash_value = hash_value % PARM_TABLE_HASH_SIZE;

  for (node = list->head[hash_value]; node != NULL; node = next_node)
    {
      next_node = node->next;
      if (strcmp (name_to_check, node->name) == 0)
        {
          node->used = USED;
          break;
        }
    }

  free (name_to_check);

  /* the above loop guarantees that node is either NULL or set to
   * the node that has a matching name.
   */
  return (node);
}

/* Add an element to the hash table
 * Overwrite any previous definitions of parameters
*/
void
L_add_to_parm_table_list (Parm_Table_List * list, Parm_Parse_Info * ppi,
                          char *section_name1, char *section_name2)
{
  Parm_Table_Node *node, *next_node, *check_for_node, *prev_check_for_node;
  Parm_Table_Node *inp_node, *prev_inp_node;
  Parm_Table_Node *prev_node_location, *next_inp_node;
  int node_size, name_size, value_size, file_name_size, hash_value, i;
  char *change_case_char, *name, *user_name;

  name = ppi->parm_buf;

  /* initialize this pointer so it can be determined whether a new
     record is being added without replacing a previous one.  needed
     for linked list of inputs. */
  prev_node_location = NULL;
  /* initialize this so new next_inp pointer gets set correctly.
     if replacing a record, this gets overwritten.
     else, this stays NULL and gets appended to input linked list */
  next_inp_node = NULL;

  /* Strip leading whitespace */
  while (isspace (*name))
    name++;

  /* Save the original name provided by the user (before it is converted to
     all caps. */
  name_size = strlen (name) + 1;
  if ((user_name = (char *) malloc (name_size)) == NULL)
    I_punt ("L_add_to_parm_table_list: Out of memory");
  strcpy (user_name, name);

  /* change all letters in name to caps */
  change_case_char = name;
  while (*change_case_char != '\0')
    {
      *change_case_char = toupper (*change_case_char);
      change_case_char++;
    }

  value_size = strlen (ppi->setting) + 1;
  file_name_size = strlen (ppi->name) + 1;
  node_size = sizeof (Parm_Table_Node);

  /* calculate hash value */
  hash_value = 0;
  for (i = 0; i < name_size; i++)
    hash_value = hash_value + *(name + i);
  hash_value = hash_value % PARM_TABLE_HASH_SIZE;

  /* check for previous definition of parameter */
  for (node = list->head[hash_value]; node != NULL; node = next_node)
    {
      next_node = node->next;
      if (strcmp (name, node->name) == 0)
        {
          /* repair the pointers in this linked list for this deletion */
          if (node == list->head[hash_value])
            list->head[hash_value] = node->next;
          else
            {
              check_for_node = list->head[hash_value];
              while (check_for_node != node)
                {
                  prev_check_for_node = check_for_node;
                  check_for_node = check_for_node->next;
                  if (check_for_node == node)
                    prev_check_for_node->next = check_for_node->next;
                }
            }

          /* input parameter linked list stays unchanged.
             save old pointer value so list does not get corrupted */
          next_inp_node = node->next_input;

          /* also save pointer to node that is being replaced, so that
             the element pointing to it can be changed to the new location
             of this node */
          prev_node_location = node;

          /* save the warning if the parm definitions come from the same 
             parmeter file. Multiple definitions are allowed as long as they
             come from different files (or levels of recursion). */
          if (strcmp (ppi->name, node->parm_file_name) == 0)
            L_add_parm_already_defined (node, ppi->setting,
                                        ppi->name, ppi->line_no,
                                        section_name1, section_name2);

          /* remove this node.  the new info will be added below */
          free (node->name);
          free (node->user_name);
          free (node->value);
          free (node->parm_file_name);
          free (node);
          break;
        }
    }

  /*  add to front of link list for hash value */
  if (((node = (Parm_Table_Node *) malloc (node_size)) == NULL) ||
      ((node->name = (char *) malloc (name_size)) == NULL) ||
      ((node->value = (char *) malloc (value_size)) == NULL) ||
      ((node->parm_file_name = (char *) malloc (file_name_size)) == NULL))
    I_punt ("L_add_to_parm_table_list: Out of memory");
  strcpy (node->name, name);
  node->user_name = user_name;
  strcpy (node->value, ppi->setting);
  strcpy (node->parm_file_name, ppi->name);
  node->parm_line_no = ppi->line_no;
  node->used = NOT_USED;
  node->next_input = next_inp_node;     /* gets inserted into old location.
                                           if new, set to NULL above and it
                                           gets inserted at end of list */
  node->next = list->head[hash_value];
  list->head[hash_value] = node;

  /* insert new element at end of input list or its old location in
     the input linked list */
  for (inp_node = start_node_of_input, prev_inp_node = NULL;
       (inp_node != NULL) && (inp_node != prev_node_location);
       prev_inp_node = inp_node, inp_node = inp_node->next_input);

  if ((inp_node != NULL) && (inp_node == prev_node_location))
    /* replacing an old record */
    if (prev_inp_node == NULL)  /* only one element in list */
      start_node_of_input = node;
    else  /* update pointer in previous element to new location */
      prev_inp_node->next_input = node;
  else /* appending to end of list */ 
    if (prev_inp_node == NULL)  /* this is first element put into linked list */
      start_node_of_input = node;
    else                          /* append to end of list */
      prev_inp_node->next_input = node;
}


/* 
 * Returns the STD_PARMS file name to use.
 * 
 * If -p STD_PARMS_FILE is used on the commmand line, this will be returned.
 * otherwise if the env_var_name is defined, this definition will be returned,
 * otherwise, the default_std_parms will be returned.
 *
 * argv, and envp are the argument and environment lists that
 * get passed into main(argc, argv, envp).
 *
 * See John Gyllenhaal with questions.
 */
char *
L_get_std_parm_name (char **argv, char **envp, char *env_var_name,
                     char *default_std_parms)
{
  int i;
  char *name_ptr, *value_ptr, *end_ptr;
  char temp_val;

  /* Search command line for -p STD_PARMS_FILE */
  for (i = 1; argv[i] != NULL; i++)
    {
      if (strcmp (argv[i], "-p") == 0)
        {
          if (argv[i + 1] == NULL)
            I_punt ("-p STD_PARMS_FILE name missing");
          return (strdup (argv[i + 1]));
        }
    }

  /* Search environment for env_var_name definitions */
  for (i = 0; envp[i] != NULL; i++)
    {
      name_ptr = envp[i];
      value_ptr = envp[i];

      /* Scan to end of name */
      while ((*value_ptr != 0) && !isspace (*value_ptr) &&
             (*value_ptr != '='))
        value_ptr++;

      end_ptr = value_ptr;

      /* Scan to beginning of value */
      while (isspace (*value_ptr) || (*value_ptr == '='))
        value_ptr++;

      /* Save char at end_ptr so can be restored later */
      temp_val = *end_ptr;

      /* Terminate the name */
      *end_ptr = 0;

      /* If name matches, return value */
      if (strcmp (name_ptr, env_var_name) == 0)
        {
          /* Restore the end_ptr so envp can be used by someone else */
          *end_ptr = temp_val;

          return (strdup (value_ptr));
        }

      /* Restore the end_ptr so envp can be used by someone else */
      *end_ptr = temp_val;
    }

  /* Otherwise return default value */
  return (strdup (default_std_parms));
}

/* 
 * Returns the macro list that should be passed to L_load_parameters.
 * Note the name change from command_line_macro_list to
 * external_macro_list.  This is because it contains both the
 * command-line macro definitions and those defined in the
 * environment.
 *
 * argv, and envp are the argument and environment lists that
 * get passed into main(argc, argv, envp).
 *
 * See John Gyllenhaal with questions.
 */
Parm_Macro_List *
L_create_external_macro_list (char **argv, char **envp)
{
  int temp;
  return L_create_external_macro_list_new (argv, envp, &temp);
}

Parm_Macro_List *
L_create_external_macro_list_new (char **argv, char **envp, int *lastarg)
{
  Parm_Macro_List *external_list;
  char *name_ptr, *value_ptr, *end_ptr;
  char temp_val;
  int i;

  external_list = L_create_parm_macro_list ();

  *lastarg = -1;

  /*
   * Read in all environment variables as macros
   * Will override internal macro definitions and can be
   * overridden by command line definitions.
   */
  for (i = 0; envp[i] != NULL; i++)
    {
      name_ptr = envp[i];
      value_ptr = envp[i];

      /* Scan to end of name */
      while ((*value_ptr != 0) && !isspace (*value_ptr) &&
             (*value_ptr != '='))
        value_ptr++;

      end_ptr = value_ptr;

      /* Scan to beginning of value */
      while (isspace (*value_ptr) || (*value_ptr == '='))
        value_ptr++;

      /* Save char at end_ptr so can be restored later */
      temp_val = *end_ptr;

      /* Terminate the name */
      *end_ptr = 0;

      /* There are some environment variables that we really don't
       * want the STD_PARMS to see if they happen to use a macro
       * of that name (ie path)
       */
      if (strcmp (name_ptr, "PATH") != 0)
        {
          /* Define the environment variable macro */
          L_define_parm_macro (external_list, name_ptr, value_ptr,
                               PARM_ENVIRONMENT);
        }

      /* Restore the end_ptr so envp can be used by someone else */
      *end_ptr = temp_val;
    }

  /*
   * Read in all -Pmacro_name=value from command line parameters
   * These macro definitions have highest precidence and
   * will override environment and internal macro definitions.
   */
  for (i = 1; argv[i] != NULL; i++)
    {
      /* Do not process arguements past the -- parameter! MCM 20001023 */
      if ((argv[i][0] == '-') && (argv[i][1] == '-'))
        {
          *lastarg = i;
          break;
        }

      /* Skip arguments that don't start with -P */
      if ((argv[i][0] != '-') || (argv[i][1] != 'P'))
        continue;

      name_ptr = &argv[i][2];
      value_ptr = name_ptr;

      /* Scan to end of name */
      while ((*value_ptr != 0) && !isspace (*value_ptr) &&
             (*value_ptr != '='))
        value_ptr++;

      end_ptr = value_ptr;

      /* The next character better be a equals sign */
      if (*value_ptr != '=')
        {
          I_punt ("Command line parse error: '=' expected after parm "
                  "macro name in arg '%s'", argv[i]);
        }

      /* Consume '=' sign */
      value_ptr++;

      /* Save char at end_ptr so can be restored later */
      temp_val = *end_ptr;

      /* Terminate the name */
      *end_ptr = 0;

      L_define_parm_macro (external_list, name_ptr, value_ptr,
                           PARM_COMMAND_LINE);

      /* Restore the end_ptr so argv can be used by someone else */
      *end_ptr = temp_val;
    }
  /*
   * Read in all -Fmacro_name=value from command line parameters
   * These macro definitions force the value, and we
   * want them to be first in the macro list (must be first, search
   * algorithm stops at first non-forced definition).
   * . 
   */
  for (i = 1; argv[i] != NULL; i++)
    {
      /* Do not process arguements past the -- parameter! MCM 20001023 */
      if ((argv[i][0] == '-') && (argv[i][1] == '-'))
        {
          if (i != *lastarg)
            I_punt("L_create_external_macro_list: error when locating parm "
                   "-- (%d != %d)", *lastarg, i);
          break;
        }

      /* Skip arguments that don't start with -F */
      if ((argv[i][0] != '-') || (argv[i][1] != 'F'))
        continue;

      name_ptr = &argv[i][2];
      value_ptr = name_ptr;

      /* Scan to end of name */
      while ((*value_ptr != 0) && !isspace (*value_ptr) &&
             (*value_ptr != '='))
        value_ptr++;

      end_ptr = value_ptr;

      /* The next character better be a equals sign */
      if (*value_ptr != '=')
        {
          I_punt
            ("Command line parse error: '=' expected after "
	     "parm name in arg '%s'",
             argv[i]);
        }

      /* Consume '=' sign */
      value_ptr++;

      /* Save char at end_ptr so can be restored later */
      temp_val = *end_ptr;

      /* Terminate the name */
      *end_ptr = 0;

      L_define_parm_macro (external_list, name_ptr, value_ptr,
                           PARM_FORCE_PARM);

      /* Restore the end_ptr so argv can be used by someone else */
      *end_ptr = temp_val;
    }

  /* Return the list */
  return (external_list);
}


void
L_warn_about_unused_macros (FILE * out, Parm_Macro_List * external_list)
{
  Parm_Macro_Node *node;
  char flag;
  int printed = 0;

  /* for lint */
  flag = 0;

  /* Go through list until hit environment parms (look at
   * only command line parms) and print warnings if it
   * parameter was unused.
   */
  for (node = external_list->head;
       (node != NULL) && (node->from <= PARM_COMMAND_LINE); node = node->next)
    {
      if (node->use_count == 0)
        {
          if (node->from == PARM_FORCE_PARM)
            flag = 'F';
          else if (node->from == PARM_COMMAND_LINE)
            flag = 'P';
          else
            I_punt ("L_warn_about_unused_macros: algorithm error");

          if (printed == 0)
            {
              fprintf (out,"THE FOLLOWING PARAMETERS FROM THE COMMAND "
                       "LINE ARE NOT USED:\n");
              printed = 1;
            }

          fprintf (out, "      -%c%s=%s ignored.\n",
                   flag, node->name, node->value);
        }
    }

  if (printed == 1)
    fprintf (out, "\n");

}

void
L_show_parms_warnings (void)
{
  int printed;
  Parm_Warn_Node *node, *next_node;
  Parm_Table_Node *inp_node;
  FILE *dump, *out;

  /* Open warning output, if stdout or stderr, route directly */
  if (strcmp (L_parm_warn_file_name, "stdout") == 0)
    out = stdout;
  else if (strcmp (L_parm_warn_file_name, "stderr") == 0)
    out = stderr;
  else if ((out = fopen (L_parm_warn_file_name, "a")) == NULL)
    I_punt ("Cannot open dump output file '%s'.", L_parm_warn_file_name);

  if (L_dump_parms)
    {
      /* Open dump output, if stdout or stderr, route directly */
      if (strcmp (L_parm_dump_file_name, "stdout") == 0)
        dump = stdout;
      else if (strcmp (L_parm_dump_file_name, "stderr") == 0)
        dump = stderr;
      else
        {
          /* Create/zero out file if first time dumping section,
           * otherwise append to end of file.
           */
          if (first_time)
            {
              if ((dump = fopen (L_parm_dump_file_name, "w")) == NULL)
                I_punt ("Cannot open dump output file '%s' for writing.",
                        L_parm_dump_file_name);
            }
          else
            {
              if ((dump = fopen (L_parm_dump_file_name, "a")) == NULL)
                I_punt ("Cannot open dump output file '%s' for appending.",
                        L_parm_dump_file_name);
            }
        }

      /* Dump file */
      if (first_time)
        {
          fprintf (dump, "\n# PARAMETER DUMP:\n");
          first_time = 0;
        }

      if (current_section_name2 == NULL)
        {
          fprintf (dump, "%s\n", current_section_name1);
        }
      else
        {
          fprintf (dump, "%s # AKA %s\n", current_section_name1,
                   current_section_name2);
        }


      for (inp_node = start_node_of_input; inp_node != NULL;
           inp_node = inp_node->next_input)
        if (inp_node->used == USED)
          fprintf (dump, "    %s = %s;\n", inp_node->user_name,
                   inp_node->value);

      fprintf (dump, "end)\n");

      if ((strcmp (L_parm_dump_file_name, "stdout") != 0) &&
          (strcmp (L_parm_dump_file_name, "stderr") != 0))
        /* Close only if really a file */
        fclose (dump);
      else
        fflush (dump);
    }


  printed = 0;
  if (L_warn_parm_not_used)
    {
      for (node = warn_list->head; node != NULL; node = node->next)
        if (node->flag == PARM_NOT_USED)
          {
            if (printed == 0)
              {
                printed = 1;
                fprintf (out, "\nTHE FOLLOWING PARAMETERS WERE IGNORED:\n");
              }
            if (node->section_name2 == NULL)
              {
                fprintf (out, "    '%s' in section '%s' ('%s' line %d).\n",
                         node->name, node->section_name1,
                         node->parm_file_name, node->parm_line_no);
              }
            else
              {
                fprintf (out,
                         "    '%s' in section '%s' aka '%s' ('%s' line %d).\n",
                         node->name, node->section_name1, node->section_name2,
                         node->parm_file_name, node->parm_line_no);
              }
          }
      if (printed == 1)
        fprintf (out, "\n");
    }

  printed = 0;
  if (L_warn_parm_not_defined)
    {
      for (node = warn_list->head; node != NULL; node = node->next)
        if (node->flag == NO_PARM_DEF)
          {
            if (printed == 0)
              {
                printed = 1;
                fprintf (out,
                         "\nTHE FOLLOWING PARAMETERS WERE NOT SET IN THE "
                         "PARAMETER FILE:\n");
              }
            if (node->section_name2 == NULL)
              {
                fprintf (out, "    '%s' in section '%s'.\n",
                         node->name, node->section_name1);
              }
            else
              {
                fprintf (out, "    '%s' in section '%s' aka '%s'.\n",
                         node->name, node->section_name1,
                         node->section_name2);
              }
          }
      if (printed == 1)
        fprintf (out, "\n");
    }

  printed = 0;
  if (L_warn_parm_defined_twice)
    {
      for (node = warn_list->head; node != NULL; node = node->next)
        if (node->flag == MULT_PARM_DEF)
          {
            if (printed == 0)
              {
                printed = 1;
                fprintf (out,"\nTHE FOLLOWING PARAMETERS WERE SET TWICE "
			 "IN THE SAME PARAMETER FILE:\n");
              }
            /* node->section_name1 is the name of the section that
             * had the problem (even if there are two names).
             */
            fprintf (out, "    '%s' in file '%s' and section '%s'.\n",
                     node->name, node->old_parm_file_name,
                     node->section_name1);

            fprintf (out, "        line %d: set to '%s'\n",
                     node->old_parm_line_no, node->old_value);
            fprintf (out, "        line %d: set to '%s'\n",
                     node->parm_line_no, node->value);
          }
      if (printed == 1)
        fprintf (out, "\n");
    }

  if ((strcmp (L_parm_warn_file_name, "stdout") != 0) &&
      (strcmp (L_parm_warn_file_name, "stderr") != 0))
    /* Close only if really a file */
    fclose (out);
  else
    fflush (out);

  /* free up memory used for the warning list */
  for (node = warn_list->head; node != NULL; node = next_node)
    {
      next_node = node->next;
      free (node->name);
      free (node->section_name1);
      if (node->section_name2 != NULL)
        free (node->section_name2);
      /* JCG added test for NULL, to prevent freeing unused fields */
      if (node->value != NULL)
        free (node->value);
      if (node->old_value != NULL)
        free (node->old_value);
      if (node->parm_file_name != NULL)
        free (node->parm_file_name);
      if (node->old_parm_file_name != NULL)
        free (node->old_parm_file_name);
      free (node);
    }
  free (warn_list);
}

/* Initializes a parm macro list */
Parm_Macro_List *
L_create_parm_macro_list (void)
{
  Parm_Macro_List *list;

  if ((list = (Parm_Macro_List *) malloc (sizeof (Parm_Macro_List))) == NULL)
    I_punt ("Parm_Macro_List: out of memory\n");

  list->head = NULL;
  return (list);
}

/* Frees all the definitions in a macro list */
void
L_free_parm_macro_list (Parm_Macro_List * list)
{
  Parm_Macro_Node *node, *next_node;

  for (node = list->head; node != NULL; node = next_node)
    {
      next_node = node->next;
      free (node->name);
      free (node->value);
      free (node);
    }
  free (list);
}

int
look_for_forced_param (char *name, Parm_Macro_List * list)
{
  Parm_Macro_Node *node;
  Parm_Parse_Info ppi;
  int name_size, value_size;

  /*
     * Search command line list for definitions that will force the
     * parameter's value.  The list is constructed so forced parms
     * are first, so stop when hit first non-forced parm.
   */
  if (list != NULL)
    {
      for (node = list->head;
           (node != NULL) && (node->from == PARM_FORCE_PARM);
           node = node->next)
        {
          /*
           * See if name matches one found from command line
           */
          if (!strcmp (node->name, name))
            {
              /* Update use_count */
              node->use_count++;

              name_size = strlen (name) + 1;
              value_size = strlen (node->value) + 1;

              if ((ppi.name = (char *) malloc (10)) == NULL)
                I_punt ("look_for_forced_param: out of memory");

              strcpy (ppi.parm_buf, name);
              strcpy (ppi.setting, node->value);
              strcpy (ppi.name, "FORCED");
              ppi.line_no = 0;

              L_add_to_parm_table_list (parameter_list, &ppi,
                                        current_section_name1,
                                        current_section_name2);
              return (1);
            }
        }
    }

  return (0); /* if this point reached, parameter is not forced */
}


char *
L_add_forced_parms (Parm_Parse_Info * ppi, Parm_Table_List * list)
{
  Parm_Macro_Node *node;
  Parm_Table_Node *parm_node;
  int value_size;

  /*
   * Search command line list for definitions that will force the
   * parameter's value.  The list is constructed so forced parms
   * are first, so stop when hit first non-forced parm.
   */
  if (ppi->command_line_list != NULL)
    {
      for (node = ppi->command_line_list->head;
           (node != NULL) && (node->from == PARM_FORCE_PARM);
           node = node->next)
        {
          /*
           * See if forced value is found in hash table
           */
          if ((parm_node = L_get_parm_table_entry (list, node->name)) != 0)
            {
              /* Update use_count */
              node->use_count++;

              /* Change value in the hash list */
              value_size = strlen (node->value) + 1;
              free (parm_node->value);
              if ((parm_node->value = (char *) malloc (value_size)) == NULL)
                I_punt ("L_add_forced_parms: Out of memory");
              strcpy (parm_node->value, node->value);
            }
        }
    }

  /* If not found, return NULL */
  return (NULL);
}

char *
L_get_parm_macro_value (Parm_Parse_Info * ppi, char *name)
{
  Parm_Macro_Node *node;
  int i;

  /*
   * Macro definitions from the command line have higher precedence
   * than internal definitions.  Search this list first.
   */
  if (ppi->command_line_list != NULL)
    {
      for (node = ppi->command_line_list->head; node != NULL;
           node = node->next)
        {
          /* Case-sensitive string compare */
          if (strcmp (name, node->name) == 0)
            {
              /* Update use count */
              node->use_count++;

              /* Return value */
              return (node->value);
            }
        }
    }

  /* If not defined on the command line, search internal definitions */
  for (i = 0; i <= ppi->internal_level; i++)
    {
      for (node = ppi->internal_list[i]->head; node != NULL;
           node = node->next)
        {
          /* Don't care about use count for internal macros */
          if (strcmp (name, node->name) == 0)
            return (node->value);
        }
    }

  /* If not found, return NULL */
  return (NULL);
}

/*
 * Adds a macro definition to the macro list.
 */
void
L_define_parm_macro (Parm_Macro_List * list, char *name, char *value,
                     int from)
{
  Parm_Macro_Node *node, *next_node, *prev_node;
  int node_size, name_size, value_size;
  char name_buf[100], *ptr, *value_ptr;

  if (list == NULL)
    I_punt ("L_define_parm_macro: null list");

  /* Strip off $'s if they exists */
  ptr = name_buf;
  while (*name != '\0')
    {
      if (*name != '$')
        {
          *ptr = *name;
          ptr++;
        }
      name++;
    }
  *ptr = 0;

  /* Point name at stripped name */
  name = name_buf;

  node_size = sizeof (Parm_Macro_Node);
  name_size = strlen (name) + 1;
  value_size = strlen (value) + 1;

  /* Get memory for value */
  if ((value_ptr = (char *) malloc (value_size)) == NULL)
    I_punt ("L_define_parm_macro: out of memory");

  /* Copy over value */
  strcpy (value_ptr, value);

#if 0
  /* Why in the world did I do the below? JCG 12-17-93 */
  /* Remove a trailing ) if it exists */
  for (i = strlen (value_ptr) - 1; i >= 0; i--)
    {
      if (value_ptr[i] == ')')
        {
          value_ptr[i] = 0;
          break;
        }
    }

#endif

  /* Set prev_node to NULL initially */
  prev_node = NULL;

  /* If already in list, remove it before adding new node */
  for (node = list->head; node != NULL; node = next_node)
    {
      next_node = node->next;
      if (strcmp (name, node->name) == 0)
        {
          /* It is unusual to have a macro defined twice from the
           * the same place, print warning
           */
          if (node->from == from)
            {
              fprintf (stderr,
                       "Warning: Parameter macro '%s' redefined to '%s'.\n",
                       name, value);
            }

          /* Remove from linked list */
          if (prev_node == NULL)
            list->head = node->next;
          else
            prev_node->next = node->next;

          /* Free memory */
          free (node->name);
          free (node->value);
          free (node);

          /* Found and removed previous defintion, stop looking */
          break;
        }

      /* Set prev_node for next pass */
      prev_node = node;
    }

  /*  add to front of macro list */
  if (((node = (Parm_Macro_Node *) malloc (node_size)) == NULL) ||
      ((node->name = (char *) malloc (name_size)) == NULL))
    I_punt ("L_define_parm_macro: out of memory");
  strcpy (node->name, name);
  node->value = value_ptr;
  node->from = from;
  node->use_count = 0;
  node->next = list->head;
  list->head = node;
}

void
L_expand_parm_macros (Parm_Parse_Info * ppi, char *line, char *buf, int max)
{
  int i, j, value_size;
  char name_buf[100], *value;

#if 0
  /* Test if parameter value has forced value */
  if ((forced_value = L_find_forced_value (ppi, ppi->parm_buf)) != NULL)
    {
      /* Make sure forced value not too long */
      if (strlen (forced_value) >= (max - 1))
        I_punt ("%s line %i: expanded line too long (max len %i)",
                ppi->name, ppi->line_no, max);

      /* Just copy forced value, and return */
      strcpy (buf, forced_value);
      return;
    }
#endif

  /* Decrement max, so have room for terminator */
  max--;
  i = 0;
  while (*line != 0)
    {
      /* Make sure we dont exceed buffer size */
      if (i >= max)
        I_punt ("%s line %i: expanded line too long (max len %i)",
                ppi->name, ppi->line_no, max + 1);

      /*
       * If it is a '\', copy the next character to the line buffer
       * without any special meaning (ie \$ yields a non-macro $)
       */
      if (*line == '\\')
        {
          /* Have problem if next character is terminator */
          if (line[1] == 0)
            I_punt ("%s line %i: cannot end line with a '\\'.\n",
                    ppi->name, ppi->line_no);

          /* Just copy next character to buffer */
          line++;
          buf[i] = *line;
          i++;
        }

      /* If not a macro start symbol $, just copy to buf */
      else if (*line != '$')
        {
          buf[i] = *line;
          i++;
        }


      /* Otherwise, replace $macro$ with it's value */
      else
        {
          line++;
          j = 0;

          /* Get the name of the macro */
          while ((*line != 0) && (*line != '$'))
            {
              /* Make sure there isn't any white space in name */
              if (isspace (*line))
                {
                  name_buf[j] = 0;
                  I_punt ("%s line %i: macro name contains space after '%s'",
                          ppi->name, ppi->line_no, name_buf);
                }
              /* Copy over character */
              name_buf[j] = *line;
              j++;
              line++;

              /* Make sure we dont exceeded name buf size */
              if (j >= (int) sizeof (name_buf))
                {
                  name_buf[sizeof (name_buf) - 1] = 0;
                  I_punt ("%s line %i: macro name too long '%s...'",
                          ppi->name, ppi->line_no, name_buf);
                }

            }
          name_buf[j] = 0;

          /* Verify macro had ending $ */
          if (*line != '$')
            I_punt ("%s line %i: ill-formed macro '%s'",
                    ppi->name, ppi->line_no, name_buf);

          /* Get macro's value */
          if ((value = L_get_parm_macro_value (ppi, name_buf)) == NULL)
            I_punt ("%s line %i: Undefined macro '%s'",
                    ppi->name, ppi->line_no, name_buf);

          /* Get length of value */
          value_size = strlen (value);

          /* Make sure copying in value won't exceed buffer size */
          if ((i + value_size) > max)
            I_punt ("%s line %i: Expanded line too long (max len %i).",
                    ppi->name, ppi->line_no, max + 1);

          /* Copy macro to buf */
          strcpy (&buf[i], value);

          /* Update size of buffer */
          i += value_size;
        }

      line++;
    }
  /* Terminate buffer */
  buf[i] = 0;
}
/*
 * Sets string parameter to passed string.  Memory is allocated for
 * the new string.  Also, if currently points at non-null value,
 * that memory is also freed (So this routine MUST always be
 * used to set string parameters).
 */
void
L_set_parm_s (char **parm, char *new_string)
{
  int size;
  char *new_str;

  size = strlen (new_string) + 1;

  if ((new_str = (char *) malloc (size)) == NULL)
    I_punt ("set_parm_s: Out of memory");

  /* Copy over new parameter */
  strcpy (new_str, new_string);

#if 0
  /* Free old string, (This may cause more trouble than it is worth) */
  if (*parm != NULL)
    free (*parm);
#endif

  *parm = new_str;
}

/* Returns 1 s1 matches s2 (ignoring case), 0 otherwise */
int
L_pmatch (char *s1, char *s2)
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

/* Adds the parameters that are defined more than once to the warning list.
 * These warnings are appended to the end of the warning list so it is printed
 * out after similar warnings that have already been marked.
 */
void
L_add_parm_already_defined (Parm_Table_Node * old_parm_node,
                            char *new_value, char *new_file_name,
                            int new_parm_line_no, char *section_name1,
                            char *section_name2)
{
  int node_size, name_size, value_size, old_value_size, file_name_size;
  int old_file_name_size, section_name_size1, section_name_size2;
  Parm_Warn_Node *node, *last_node, *next_node;

  node_size = sizeof (Parm_Warn_Node);
  name_size = strlen (old_parm_node->user_name) + 1;
  value_size = strlen (new_value) + 1;
  old_value_size = strlen (old_parm_node->value) + 1;
  section_name_size1 = strlen (section_name1) + 1;
  file_name_size = strlen (new_file_name) + 1;
  old_file_name_size = strlen (old_parm_node->parm_file_name) + 1;

  /* find the last node in the list */
  last_node = warn_list->head;
  if (last_node != NULL)
    for (next_node = last_node->next; next_node != NULL;
         next_node = last_node->next)
      last_node = next_node;

  if (((node = (Parm_Warn_Node *) malloc (node_size)) == NULL) ||
      ((node->name = (char *) malloc (name_size)) == NULL) ||
      ((node->value = (char *) malloc (value_size)) == NULL) ||
      ((node->old_value = (char *) malloc (old_value_size)) == NULL) ||
      ((node->section_name1 = (char *) malloc (section_name_size1)) == NULL)
      || ((node->parm_file_name = (char *) malloc (file_name_size)) == NULL)
      || ((node->old_parm_file_name = (char *) malloc (old_file_name_size)) ==
          NULL))
    I_punt ("L_add_parm_defined_already: Out of memory");

  node->flag = MULT_PARM_DEF;
  strcpy (node->name, old_parm_node->user_name);
  strcpy (node->value, new_value);
  strcpy (node->old_value, old_parm_node->value);
  strcpy (node->section_name1, section_name1);
  if (section_name2 != NULL)
    {
      section_name_size2 = strlen (section_name2) + 1;
      if ((node->section_name2 = (char *) malloc (section_name_size2)) ==
          NULL)
        I_punt ("L_add_parm_defined_already: Out of memory");
      strcpy (node->section_name2, section_name2);

    }
  else
    {
      node->section_name2 = NULL;
    }

  strcpy (node->parm_file_name, new_file_name);
  strcpy (node->old_parm_file_name, old_parm_node->parm_file_name);
  node->parm_line_no = new_parm_line_no;
  node->old_parm_line_no = old_parm_node->parm_line_no;
  node->next = NULL;

  if (last_node == NULL)        /* this is first warning in the list */
    warn_list->head = node;
  else
    last_node->next = node;
  last_node = node;
}


/* Adds the parameters not used warning for an entire section to the warning
 * list.
 * These warnings are appended to the end of the warning list so it is printed
 * out after similar warnings that have already been marked.
 */
void
L_add_parm_not_used (Parm_Table_List * parm_list,
                     char *section_name1, char *section_name2)
{
  int node_size, name_size, file_name_size, section_name_size1, i;
  int section_name_size2;
  Parm_Warn_Node *node, *last_node, *next_node;
  Parm_Table_Node *parm_node;

  /* these sizes are constant for this entire procedure */
  node_size = sizeof (Parm_Warn_Node);
  section_name_size1 = strlen (section_name1) + 1;

  /* find the last node in the list */
  last_node = warn_list->head;
  if (last_node != NULL)
    for (next_node = last_node->next; next_node != NULL;
         next_node = last_node->next)
      last_node = next_node;

  /* search the entire hast table for entries that are not being used */
  for (i = 0; i < PARM_TABLE_HASH_SIZE; i++)

    for (parm_node = parm_list->head[i]; parm_node != NULL;
         parm_node = parm_node->next)

      if (parm_node->used == NOT_USED)
        {
          name_size = strlen (parm_node->user_name) + 1;
          file_name_size = strlen (parm_node->parm_file_name) + 1;

          if (((node = (Parm_Warn_Node *) malloc (node_size)) == NULL) ||
              ((node->name = (char *) malloc (name_size)) == NULL) ||
              ((node->section_name1 = (char *) malloc (section_name_size1)) ==
               NULL)
              || ((node->parm_file_name = (char *) malloc (file_name_size)) ==
                  NULL))
            I_punt ("L_add_parm_not_used: Out of memory");

          node->flag = PARM_NOT_USED;
          strcpy (node->name, parm_node->user_name);
          strcpy (node->section_name1, section_name1);
          if (section_name2 != NULL)
            {
              section_name_size2 = strlen (section_name2) + 1;
              if ((node->section_name2 =
                   (char *) malloc (section_name_size2)) == NULL)
                {
                  I_punt ("L_add_parm_not_used: Out of memory");
                }
              strcpy (node->section_name2, section_name2);

            }
          else
            {
              node->section_name2 = NULL;
            }

          strcpy (node->parm_file_name, parm_node->parm_file_name);
          node->parm_line_no = parm_node->parm_line_no;
          node->next = NULL;

          /* JCG , set unused fields to NULL so don't free garbage */
          node->value = NULL;
          node->old_value = NULL;
          node->old_parm_file_name = NULL;

          if (last_node == NULL)        /* this is first warning in the list */
            warn_list->head = node;
          else
            last_node->next = node;
          last_node = node;
        }

  return;
}

/* Adds a parameter not found warning to the warning list.
 * This warning is appended to the end of the warning list so it is
 * printed out after similar warnings that have already been marked.
 */
void
L_add_parm_not_defined (char *name, char *section_name1, char *section_name2)
{
  int node_size, name_size, section_name_size1, section_name_size2;
  Parm_Warn_Node *node, *prev_node;

  node_size = sizeof (Parm_Warn_Node);
  name_size = strlen (name) + 1;
  section_name_size1 = strlen (section_name1) + 1;

  if (((node = (Parm_Warn_Node *) malloc (node_size)) == NULL) ||
      ((node->name = (char *) malloc (name_size)) == NULL) ||
      ((node->section_name1 = (char *) malloc (section_name_size1)) == NULL))
    I_punt ("L_add_parm_not_defined: Out of memory");

  node->flag = NO_PARM_DEF;
  strcpy (node->name, name);
  strcpy (node->section_name1, section_name1);
  if (section_name2 != NULL)
    {
      section_name_size2 = strlen (section_name2) + 1;
      if ((node->section_name2 =
           (char *) malloc (section_name_size2)) == NULL)
        {
          I_punt ("L_add_parm_not_defined: Out of memory");
        }
      strcpy (node->section_name2, section_name2);

    }
  else
    {
      node->section_name2 = NULL;
    }
  node->next = NULL;

  /* JCG, set unused fields to NULL so don't free garbage */
  node->value = NULL;
  node->old_value = NULL;
  node->parm_file_name = NULL;
  node->old_parm_file_name = NULL;


  if (warn_list->head == NULL)
    warn_list->head = node;
  else
    {
      for (prev_node = warn_list->head; prev_node->next != NULL;
           prev_node = prev_node->next);
      prev_node->next = node;
    }

  return;
}

/* Reads in an int parameter.  Nothing else may be on the line. 
  */
void
L_read_parm_i (Parm_Parse_Info * ppi_in, char *name, int *ptr)
{
  Parm_Table_List *list;
  Parm_Table_Node *node;
  Parm_Parse_Info ppi;
  int dev_parm;

  list = ppi_in->parm_list;

  /* Strip leading whitespace */
  while (isspace (*name))
    name++;

  /* If leading char '?', then dev parm (warn on different flag) */
  dev_parm = 0;
  if (*name == '?')
    {
      dev_parm = 1;
      name++;
    }

  /* Do we have this paramater in the buffer, if no, return */
  node = L_get_parm_table_entry (list, name);
  if (node == NULL)
    {
      if (!look_for_forced_param (name, ppi_in->command_line_list))
        {
          if (!dev_parm || L_warn_dev_parm_not_defined)
            {
              L_add_parm_not_defined (name, current_section_name1,
                                      current_section_name2);
            }
          return;
        }
      else
        node = L_get_parm_table_entry (list, name);
    }

  /* Create a ppi structure so it is easier to reuse current code */
  strcpy (ppi.setting, node->value);
  ppi.name = node->parm_file_name;
  ppi.line_no = node->parm_line_no;
  /* Read int expression from setting ptr */
  ppi.calc_ptr = ppi.setting;

  *ptr = L_parm_calc_complete_expr (&ppi);

  return;
}

/*
 * Reads in an binary parameter.  Nothing else may be on the line. 
 * The following are equivalent (yes, y, 1, on) (no, n, 0, off)
 */
void
L_read_parm_b (Parm_Parse_Info * ppi_in, char *name, int *ptr)
{
  Parm_Table_List *list;
  Parm_Table_Node *node;
  int dev_parm;

  list = ppi_in->parm_list;
  /* Strip leading whitespace */
  while (isspace (*name))
    name++;

  /* If leading char '?', then dev parm (warn on different flag) */
  dev_parm = 0;
  if (*name == '?')
    {
      dev_parm = 1;
      name++;
    }

  /* Do we have this parmater in the buffer, if no, return */
  node = L_get_parm_table_entry (list, name);
  if (node == NULL)
    {
      if (!look_for_forced_param (name, ppi_in->command_line_list))
        {
          if (!dev_parm || L_warn_dev_parm_not_defined)
            {
              L_add_parm_not_defined (name, current_section_name1,
                                      current_section_name2);
            }
          return;
        }
      else
        node = L_get_parm_table_entry (list, name);
    }

  if (node->value[0] == '\0')
    *ptr = 1;
  else if (L_pmatch (node->value, "yes") ||
           L_pmatch (node->value, "y") ||
           L_pmatch (node->value, "1") || L_pmatch (node->value, "on"))
    *ptr = 1;

  else if (L_pmatch (node->value, "no") ||
           L_pmatch (node->value, "n") ||
           L_pmatch (node->value, "0") || L_pmatch (node->value, "off"))
    *ptr = 0;

  else
    I_punt ("%s line %i: parm '%s' expects a binary setting, not '%s'",
            node->parm_file_name, node->parm_line_no, node->name,
            node->value);

  return;
}

/* Reads in an string parameter, everything on the line is taken.
 * Returns 1 if parm read in, 0 if not.
 */
void
L_read_parm_s (Parm_Parse_Info * ppi_in, char *name, char **ptr)
{
  Parm_Table_List *list;
  Parm_Table_Node *node;
  int dev_parm;

  list = ppi_in->parm_list;
  /* Strip leading whitespace */
  while (isspace (*name))
    name++;

  /* If leading char '?', then dev parm (warn on different flag) */
  dev_parm = 0;
  if (*name == '?')
    {
      dev_parm = 1;
      name++;
    }

  /* Do we have this parmater in the buffer, if no, return */
  node = L_get_parm_table_entry (list, name);
  if (node == NULL)
    {
      if (!look_for_forced_param (name, ppi_in->command_line_list))
        {
          if (!dev_parm || L_warn_dev_parm_not_defined)
            {
              L_add_parm_not_defined (name, current_section_name1,
                                      current_section_name2);
            }
          return;
        }
      else
        node = L_get_parm_table_entry (list, name);
    }

  /* set string parameter to setting */
  L_set_parm_s (ptr, node->value);

  return;
}

/* Reads in an float parameter.  Nothing else may be on the line. 
 */
void
L_read_parm_f (Parm_Parse_Info * ppi_in, char *name, float *ptr)
{
  Parm_Table_List *list;
  Parm_Table_Node *node;
  char *end_ptr;
  int dev_parm;

  list = ppi_in->parm_list;
  /* Strip leading whitespace */
  while (isspace (*name))
    name++;

  /* If leading char '?', then dev parm (warn on different flag) */
  dev_parm = 0;
  if (*name == '?')
    {
      dev_parm = 1;
      name++;
    }

  /* Do we have this paramater in the buffer, if no, return */
  node = L_get_parm_table_entry (list, name);
  if (node == NULL)
    {
      if (!look_for_forced_param (name, ppi_in->command_line_list))
        {
          if (!dev_parm || L_warn_dev_parm_not_defined)
            {
              L_add_parm_not_defined (name, current_section_name1,
                                      current_section_name2);
            }
          return;
        }
      else
        node = L_get_parm_table_entry (list, name);
    }

  /* Read in as double, converted to float */
  *ptr = strtod (node->value, &end_ptr);

  /* Make sure properly read in */
  if (*end_ptr != 0)
    {
      I_punt ("%s line %i parameter %s: invalid float '%s'",
              node->parm_file_name, node->parm_line_no, node->name,
              node->value);
    }

  return;
}

/* Reads in an double parameter.  Nothing else may be on the line. 
 */
void
L_read_parm_lf (Parm_Parse_Info * ppi_in, char *name, double *ptr)
{
  Parm_Table_List *list;
  Parm_Table_Node *node;
  char *end_ptr;
  int dev_parm;

  list = ppi_in->parm_list;
  /* Strip leading whitespace */
  while (isspace (*name))
    name++;

  /* If leading char '?', then dev parm (warn on different flag) */
  dev_parm = 0;
  if (*name == '?')
    {
      dev_parm = 1;
      name++;
    }

  /* Do we have this paramater in the buffer, if no, return */
  node = L_get_parm_table_entry (list, name);
  if (node == NULL)
    {
      if (!look_for_forced_param (name, ppi_in->command_line_list))
        {
          if (!dev_parm || L_warn_dev_parm_not_defined)
            {
              L_add_parm_not_defined (name, current_section_name1,
                                      current_section_name2);
            }
          return;
        }
      else
        node = L_get_parm_table_entry (list, name);
    }

  /* Read in as double, converted to float */
  *ptr = strtod (node->value, &end_ptr);

  /* Make sure properly read in */
  if (*end_ptr != 0)
    {
      I_punt ("%s line %i parameter %s: invalid double '%s'",
              node->parm_file_name, node->parm_line_no, node->name,
              node->value);
    }

  return;
}


/*
 * Gets next parameter line, partially parsed.
 * Returns 1 if successful, 0 otherwise.
 */
static int
L_get_next_parm (Parm_Parse_Info * ppi)
{
  char *ptr, *parm;
  char *setting_ptr;
  int count, i;

  while (fgets (ppi->line_buf, PARM_LINE_MAX, ppi->file) != NULL)
    {
      ppi->line_no++;

      ptr = ppi->line_buf;

      /* Skip whitespace */
      while (isspace (*ptr))
        ptr++;

      /* Skip comments */
      if ((*ptr == 0) || (*ptr == '#'))
        continue;

      /* Copy parm to parm_buf (delimited by white space or '=') */
      parm = ppi->parm_buf;
      while (!isspace (*ptr) && (*ptr != '='))
        {
          *parm = *ptr;
          ptr++;
          parm++;
        }
      *parm = 0;

      /* find beginning of parm setting */

      while (isspace (*ptr) || (*ptr == '='))
        ptr++;

      /* Point setting at beginning of setting */
      setting_ptr = ptr;

      /* Remove comments and trailing whitespace from buffer */
      /* First, find end of buffer or start of comment */
      while ((*ptr != 0) && (*ptr != '#'))
        {
          /* Allow \# to be read as a # not as a begin  comment */
          if ((*ptr == '\\') && (ptr[1] == '#'))
            ptr += 2;
          else
            ptr++;
        }

      /* If found '#', rest is comment, terminate here */
      if (*ptr == '#')
        *ptr = 0;

      /* Back up to character before termintation */
      ptr--;

      /* Strip off trailing white space */
      while (isspace (*ptr))
        {
          *ptr = 0;
          ptr--;
        }

      /* Strip off at most one trailing ; */
      if (*ptr == ';')
        {
          *ptr = 0;
          ptr--;
        }

      /* Strip off trailing white space (after ; stripped) */
      while (isspace (*ptr))
        {
          *ptr = 0;
          ptr--;
        }


      /* Expand macros in setting field, write to setting buffer */
      L_expand_parm_macros (ppi, setting_ptr, ppi->setting, PARM_BUF_MAX);

      /* 
       * Get macro definitions and put into internal list.
       * Dont return to caller, process next line.
       */
      if (ppi->parm_buf[0] == '$')
        {
          /* Make sure have exactly two '$'s in macro name */
          count = 0;
          for (i = 0; ppi->parm_buf[i] != 0; i++)
            {
              if (ppi->parm_buf[i] == '$')
                count++;
              if (isspace (ppi->parm_buf[i]))
                I_punt ("Macro name '%s' cannot have whitespace in it\n",
                        ppi->parm_buf);
            }

          if ((ppi->parm_buf[0] != '$') || (ppi->parm_buf[i - 1] != '$') ||
              (i < 3) || (count != 2))
            I_punt ("Badly formed macro name '%s'\n", ppi->parm_buf);


          L_define_parm_macro (ppi->internal_list[ppi->internal_level],
                               ppi->parm_buf, ppi->setting, PARM_INTERNAL);
          continue;
        }

      /* Line successfully read, return 1 */
      return (1);
    }

  /* End of file, return 0 */
  return (0);

}


#define TNULL   0
#define TPLUS   1
#define TMINUS  2
#define TSTAR   3
#define TSLASH  4
#define TLPARN  5
#define TRPARN  6
#define TNUM    7

/* Calculate the value of the complete expression.  Punt if
 * anything is left over after expression's value is calculated.
 */
int
L_parm_calc_complete_expr (Parm_Parse_Info * ppi)
{
  int result;

  /* Get the expression's value */
  result = L_parm_calc_expr (ppi);

  /* Punt if there is anything left that was not parsed */
  if (ppi->calc_ptr[0] != 0)
    {
      I_punt ("%s line %i: Expr formulation error.  "
              "Unable to process '%s'.",
              ppi->name, ppi->line_no, ppi->calc_ptr);
    }

  return (result);
}

int
L_parm_calc_expr (Parm_Parse_Info * ppi)
{
  int result;

  result = L_parm_calc_term (ppi);

  while (1)
    {
      L_peek_next_token (ppi);
      if (ppi->token == TPLUS)
        {
          L_get_next_token (ppi);
          result = result + L_parm_calc_term (ppi);
        }
      else if (ppi->token == TMINUS)
        {
          L_get_next_token (ppi);
          result = result - L_parm_calc_term (ppi);
        }
      else
        break;
    }
  return (result);

}
int
L_parm_calc_term (Parm_Parse_Info * ppi)
{
  int result;

  result = L_parm_calc_factor (ppi);

  while (1)
    {
      L_peek_next_token (ppi);
      if (ppi->token == TSTAR)
        {
          L_get_next_token (ppi);
          result = result * L_parm_calc_factor (ppi);
        }
      else if (ppi->token == TSLASH)
        {
          L_get_next_token (ppi);
          result = result / L_parm_calc_factor (ppi);
        }
      else
        break;
    }
  return (result);

}
int
L_parm_calc_factor (Parm_Parse_Info * ppi)
{
  int result;

  L_get_next_token (ppi);

  switch (ppi->token)
    {
      /* Extension to handle negative numbers */
    case TMINUS:
      L_get_next_token (ppi);
      if (ppi->token == TNUM)
        return (-(ppi->value));
      else
        I_punt ("%s line %i: parse_error, expected number after '-'.",
                ppi->name, ppi->line_no);
    case TNUM:
      return (ppi->value);

    case TLPARN:
      result = L_parm_calc_expr (ppi);
      L_get_next_token (ppi);
      if (ppi->token != TRPARN)
        I_punt ("%s line %i: ')' expected.", ppi->name, ppi->line_no);
      return (result);

    case TRPARN:
      I_punt ("%s line %i: unexpected ')' before '%s'.",
              ppi->name, ppi->line_no, ppi->calc_ptr);

    default:
      I_punt ("%s line %i: Expression formulation error before '%s'.",
              ppi->name, ppi->line_no, ppi->calc_ptr);

    }

  /* Should never get here */
  exit (1);
  return (0);
}

int
L_get_next_token (Parm_Parse_Info * ppi)
{
  int len;
  len = L_peek_next_token (ppi);
  ppi->calc_ptr += len;
  return (len);
}

int
L_peek_next_token (Parm_Parse_Info * ppi)
{
  char *ptr;

  ptr = ppi->calc_ptr;

  /* Skip over leading whitespace */
  while (isspace (*ptr))
    ptr++;

  /* Return 0 at end of string */
  if (*ptr == 0)
    {
      ppi->token = TNULL;
      return (0);
    }

  else if (isdigit (*ptr))
    {
      ppi->token = TNUM;
      ppi->value = (int) strtol (ptr, &ptr, 0);
    }
  else
    {
      switch (*ptr)
        {
        case '+':
          ppi->token = TPLUS;
          break;
        case '-':
          ppi->token = TMINUS;
          break;
        case '*':
          ppi->token = TSTAR;
          break;
        case '/':
          ppi->token = TSLASH;
          break;
        case '(':
          ppi->token = TLPARN;
          break;
        case ')':
          ppi->token = TRPARN;
          break;
        default:
          I_punt ("%s line %i: Parse error at '%c' in '%s'.",
                  ppi->name, ppi->line_no, *ptr, ptr);
        }
      ptr++;
    }
  return (ptr - ppi->calc_ptr);
}
void
L_read_parm_section_defaults (Parm_Parse_Info * ppi,
                              Parm_Section_Info * psi,
                              char *defaults_file_name)
{
  char *old_name;
  FILE *old_file;
  int old_line_no;
  char name[PARM_LINE_MAX];

  /* Ignore defaults_file_name of declaration (for back compatablity */
  if (L_pmatch (defaults_file_name, "declaration"))
    return;

  /* Save the name of the file, (FILE *), and line no of parm file
   * currently reading 
   */
  old_name = ppi->name;
  old_file = ppi->file;
  old_line_no = ppi->line_no;

  /* Increment current level */
  ppi->internal_level++;

  /* Make sure we haven't recursed too deep */
  if (ppi->internal_level >= PARM_LEVEL_MAX)
    {
      if (psi->name2 == NULL)
        {
          I_punt ("Recursed too many times reading parameter defaults "
                  " for '%s'.", psi->name1);
        }
      else
        {
          I_punt ("Recursed too many times reading parameter defaults "
                  " for '%s' aka '%s'.", psi->name1, psi->name2);
        }
    }

  /* Create macro list for this level */
  ppi->internal_list[ppi->internal_level] = L_create_parm_macro_list ();

  /* Open defaults level */
  if ((ppi->file = fopen (defaults_file_name, "r")) == NULL)
    {
      if (psi->name2 == NULL)
        {
          I_punt ("Unable to open parameter defaults file '%s'.\n"
                  "Defaults file for '%s' and specified in '%s'",
                  defaults_file_name, psi->name1, ppi->name);
        }
      else
        {
          I_punt ("Unable to open parameter defaults file '%s'.\n"
                  "Defaults file for '%s' aka '%s' and specified in '%s'",
                  defaults_file_name, psi->name1, psi->name2, ppi->name);
        }
    }

  /* Need to copy defaults file name into buffer, because reading
   * a parameter will destory the value in defaults_file_name)
   */
  strcpy (name, defaults_file_name);
  ppi->name = name;
  ppi->line_no = 0;

  /* Read the default parm section */
  L_read_parm_sections (ppi, psi);

  /* Close defaults parameter file */
  fclose (ppi->file);

  /* Free internal macro list for this level */
  L_free_parm_macro_list (ppi->internal_list[ppi->internal_level]);

  /* Restore old level */
  ppi->internal_level--;

  /* Restore old file name, (FILE *), and line no */
  ppi->name = old_name;
  ppi->file = old_file;
  ppi->line_no = old_line_no;
}

void
L_read_parm_sections (Parm_Parse_Info * ppi, Parm_Section_Info * psi)
{
  int section_match, first_match_line, first_match_found;
  char *first_match_name;
  char *default_defaults_path;
  int default_defaults_path_line;
  int nest;

  /* Initialize found counters */
  psi->found[ppi->internal_level] = 0;

  /* Initialize default_defaults_path variables */
  default_defaults_path = NULL;
  default_defaults_path_line = 0;

  /* Initialize first match found variables */
  first_match_found = 0;
  first_match_line = 0;
  first_match_name = NULL;
  while (L_get_next_parm (ppi))
    {
      section_match = 0;
      /* Detect default defaults_path specification -JCG 10/13/98 */
      if (L_pmatch (ppi->parm_buf, "(*"))
        {
          if (default_defaults_path != NULL)
            {
              fprintf (stderr,
                       "IGNORING REDUNDANT DEFAULT 'DEFAULTS PATH' "
                       "SPECIFICATION:\n");
              fprintf (stderr, "  PROCESSED: '%s' at line %i of '%s'\n",
                       default_defaults_path, default_defaults_path_line,
                       ppi->name);
              fprintf (stderr, "  IGNORED:   '%s' at line %i of '%s'\n\n",
                       ppi->setting, ppi->line_no, ppi->name);
            }
          else
            {
              /* Get the line number of the default defaults_path spec */
              default_defaults_path_line = ppi->line_no;

              /* Make sure default defaults_path specified! */
              if ((ppi->setting[0] == 0))
                {
                  I_punt("A 'defaults_path' expected after '(*' on "
			 "line %i of '%s'.",
                     ppi->line_no, ppi->name);
                }

              /* Strdup defaults_path, since setting will go away */
              default_defaults_path = strdup (ppi->setting);
            }

          /* The rest of the section MUST be empty! */
          if (!L_get_next_parm (ppi))
            {
              I_punt ("Expected 'end)' after '(*' on line %i of '%s'.",
                      ppi->line_no, ppi->name);
            }

          /* Make sure have end)! */
          if (!L_pmatch (ppi->parm_buf, "end)"))
            {
              I_punt ("Expected 'end)' not '%s' after '(*'\n"
                      "on line %i of '%s'.",
                      ppi->parm_buf, ppi->line_no, ppi->name);
            }

          /* Mark that we don't need to skip the rest of the section! */
          section_match = 1;
        }

      /* Search for a match to the specified sections */
      else if (L_pmatch (ppi->parm_buf, psi->name1))
        {
          /* Print out warning if this section already seen */
          if (first_match_found)
            {
              fprintf (stderr,
                       "IGNORING REDUNDANT PARAMETER SECTION DEFINITIONS:\n");
              fprintf (stderr, "  PROCESSED: '%s' at line %i of '%s'\n",
                       first_match_name, first_match_line, ppi->name);
              fprintf (stderr, "  IGNORED:   '%s' at line %i of '%s'\n\n",
                       psi->name1, ppi->line_no, ppi->name);
            }
          else
            {
              /* Get the line number of the section name match */
              first_match_line = ppi->line_no;

              /* Get the section name that matched */
              first_match_name = psi->name1;

              /* The name after the section is taken to be the
               * name of a 'defaults' file that should be read first
               */
              if (ppi->setting[0] != 0)
                {
                  L_read_parm_section_defaults (ppi, psi, ppi->setting);
                }

              /* Increment number of times found */
              psi->found[ppi->internal_level]++;

              /* Mark that this section name was used */
              section_name1_used = 1;

              /* Load the parameters into the hash table */
              while (L_get_next_parm (ppi) &&
                     !L_pmatch (ppi->parm_buf, "end)"))
                {
                  L_add_to_parm_table_list (psi->parm_list, ppi, psi->name1,
                                            psi->name2);
                }
              section_match = 1;

              /* Mark that we have found the section we are looking for */
              first_match_found = 1;
            }
        }

      /* Allow matching of either name1 or name2 
       * If name two matched, switch "section" alias's to let warnings
       * know what section the parameter really came from.
       */
      else if ((psi->name2 != NULL) && L_pmatch (ppi->parm_buf, psi->name2))
        {
          /* Print out warning if this section already seen */
          if (first_match_found)
            {
              fprintf (stderr,
                       "IGNORING REDUNDANT PARAMETER SECTION DEFINITIONS:\n");
              fprintf (stderr, "  PROCESSED: '%s' at line %i of '%s'\n",
                       first_match_name, first_match_line, ppi->name);
              fprintf (stderr, "  IGNORED:   '%s' at line %i of '%s'\n\n",
                       psi->name2, ppi->line_no, ppi->name);
            }
          else
            {
              /* Get the line number of the section name match */
              first_match_line = ppi->line_no;

              /* Get the section name that matched */
              first_match_name = psi->name2;

              /* The name after the section is taken to be the
               * name of a 'defaults' file that should be read first
               */
              if (ppi->setting[0] != 0)
                {
                  L_read_parm_section_defaults (ppi, psi, ppi->setting);
                }

              /* Increment number of times found */
              psi->found[ppi->internal_level]++;

              /* Mark that this section name was used */
              section_name2_used = 1;

              /* Load the parameters into the hash table */
              while (L_get_next_parm (ppi)
                     && !L_pmatch (ppi->parm_buf, "end)"))
                L_add_to_parm_table_list (psi->parm_list, ppi, psi->name2,
                                          psi->name1);
              section_match = 1;

              /* Mark that we have found the first definition of this
               */
              first_match_found = 1;
            }
        }

      if (!section_match)
        {                       /* Skip over the current section */
          nest = 1;
          while ((nest != 0) && (L_get_next_parm (ppi)))
            if (ppi->parm_buf[0] == '(')
              nest++;
            else if (L_pmatch (ppi->parm_buf, "end)"))
              nest--;
        }
    }

  /* Use default 'defaults_path' if section not explicitly found in
   * file, otherwise punt. -JCG 10/13/98
   */
  if (psi->found[ppi->internal_level] == 0)
    {
      if (default_defaults_path != NULL)
        {
          /* Read in section from default_defaults_path */
          L_read_parm_section_defaults (ppi, psi, default_defaults_path);
        }
      else
        {
          /* Print messages for all parameter sections not found */
          if (psi->name2 == NULL)
            {
              fprintf (stderr,
                       "Error: cannot find parameter file section '%s'.\n",
                       psi->name1);
            }
          else
            {
              fprintf (stderr,
                       "Error: cannot find parameter file section '%s' "
                       "aka '%s'.\n", psi->name1, psi->name2);
            }

          I_punt ("Cannot continue: missing parameter file section in %s.",
                  ppi->name);
        }
    }

  /* Free default_defaults_path if strduped */
  if (default_defaults_path != NULL)
    free (default_defaults_path);
}

/* Enhanced load parameters to accept up to two equivalent names
 * for a section.  This functionality was added to allow the Trimaran
 * release to "rename" some IMPACT parameter sections (namely,
 * global, file, arch, RegAlloc, and Scheduler). -JCG 5/19/98
 *
 * Used by the "classic" and "new" interfaces below.
 */
void
_L_load_parameters (char *parm_file_name,
                    Parm_Macro_List * command_line_macro_list,
                    char *section_name1, char *section_name2,
                    void (*read_func) (Parm_Parse_Info * ppi))
{
  Parm_Parse_Info ppi;
  Parm_Section_Info *psi;


  /* Set file name */
  ppi.name = parm_file_name;
  ppi.line_no = 0;

  /* Initialize macro lists */
  ppi.command_line_list = command_line_macro_list;
  ppi.internal_list[0] = L_create_parm_macro_list ();
  ppi.internal_level = 0;

  /* 
   * Copy the parse strings and functions to call, and
   * Allocate array so can mark when section found
   */
  psi = (Parm_Section_Info *) malloc (sizeof (Parm_Section_Info));
  if (psi == NULL)
    I_punt ("Out of memory");

  /* JCG, malloced incorrect size (used sizeof instead of strlen).  
   * Changed to strdup. 
   */
  psi->name1 = strdup (section_name1);
  if (section_name2 != NULL)
    psi->name2 = strdup (section_name2);
  else
    psi->name2 = NULL;

  psi->read_func = read_func;
  psi->parm_list = L_create_parm_table_list ();
  parameter_list = psi->parm_list;

  /* initialize the input start pointer for parameters */
  start_node_of_input = NULL;

  if ((ppi.file = fopen (parm_file_name, "r")) == NULL)
    {
      fprintf (stderr, "Unable to open parameter file '%s'.\n ",
               parm_file_name);
      I_punt ("Check setting of environment variable STD_PARMS_FILE.");
    }

  /* initailize the warn list pointer to null.
   * the pointer must be initialized for the warnings to be added correctly.
   */
  if ((warn_list = (Parm_Warn_List *) malloc (sizeof (Parm_Warn_List))) ==
      NULL)
    I_punt ("out of memory");
  warn_list->head = NULL;

  /* Initially neither name is used */
  section_name1_used = 0;
  section_name2_used = 0;

  L_read_parm_sections (&ppi, psi);

  L_add_forced_parms (&ppi, psi->parm_list);

  /* Set "current_section_name1&2" based on what section names
   * were actually used while reading parameters.
   */
  /* If only name1 used, don't use name2 at all in messages */
  if (section_name1_used && (!section_name2_used))
    {
      current_section_name1 = psi->name1;
      current_section_name2 = NULL;
    }
  /* If only name2 used, don't use name1 at all in messages */
  else if ((!section_name1_used) && section_name2_used)
    {
      current_section_name1 = psi->name2;
      current_section_name2 = NULL;

      /* If turned on, warn external users that they are using the "old" 
       * version of the parameter file section name.
       */
      if (L_warn_old_parm_section_name_used)
        {
          fprintf (stderr,
                   "\nWarning: Read parameters from '%s' (old name) "
                   "because unable\n"
                   "         to find '%s' (new name) in\n"
                   "         '%s'.\n\n",
                   psi->name2, psi->name1, parm_file_name);
        }
    }
  /* If both names are used, use both in messages */
  else if (section_name1_used && section_name2_used)
    {
      current_section_name1 = psi->name1;
      current_section_name2 = psi->name2;

      /* If turned on, warn external users that they are using a mixture of
       * the "old" and "new" version of the parameter file section name.
       */
      if (L_warn_old_parm_section_name_used)
        {
          fprintf (stderr,
                   "\nWarning: Mixture of '%s' (new name) and "
                   "'%s' (old name)\n"
                   "         found in '%s'.\n\n",
                   psi->name1, psi->name2, parm_file_name);
        }
    }
  /* Should never get here */
  else
    {
      if (psi->name2 != NULL)
        {
          I_punt ("_L_load_parameters: Neither section %s or %s used!",
                  psi->name1, psi->name2);
        }
      else
        {
          I_punt ("_L_load_parameters: Section not used!", psi->name1);
        }
    }

  ppi.parm_list = psi->parm_list;

  /* JCG 1/18/95 added & to front of ppi so that it is passed by reference
   * as read_func is expecting.
   * Our compilers hid this bug because the structure is about 10k and
   * they passed it by reference anyway. :)
   */
  psi->read_func (&ppi);
  L_add_parm_not_used (psi->parm_list, current_section_name1,
                       current_section_name2);

  L_show_parms_warnings ();

  /* Free the allocated memory */
  free (psi->name1);
  if (psi->name2 != NULL)
    free (psi->name2);
  L_free_parm_table_list (psi->parm_list);

  free (psi);

  fclose (ppi.file);

  /* Free internal macro list */
  L_free_parm_macro_list (ppi.internal_list[0]);
}

/* Classic interface for when section name is not aliased  -JCG 5/19/98 */
void
L_load_parameters (char *parm_file_name,
                   Parm_Macro_List * command_line_macro_list,
                   char *section, void (*read_func) (Parm_Parse_Info * ppi))
{

  _L_load_parameters (parm_file_name, command_line_macro_list,
                      section, NULL, read_func);
}

/* New interface for when section name is aliased  -JCG 5/19/98
 * Allows both an "new" and "old" name for a section to be used. */
void
L_load_parameters_aliased (char *parm_file_name,
                           Parm_Macro_List * command_line_macro_list,
                           char *new_section_name,
                           char *old_section_name,
                           void (*read_func) (Parm_Parse_Info * ppi))
{

  _L_load_parameters (parm_file_name, command_line_macro_list,
                      new_section_name, old_section_name, read_func);
}
