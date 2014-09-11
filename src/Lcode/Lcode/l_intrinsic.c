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
/* 10/29/02 REK Adding config.h */
#include <config.h>
#include <Lcode/l_main.h>
#include <library/md.h>


#define INTRINSIC_REGISTERS_SECTION_NAME    "Registers"
#define INTRINSIC_INTRINSICS_SECTION_NAME   "Intrinsics"
#define INTRINSIC_FUNCTION_CALL_FIELDNAME   "function_call"
#define INTRINSIC_FUNCTION_CALL_SILENT_FIELDNAME   "function_call_silent"
#define INTRINSIC_OPCODE_FIELDNAME          "opcode"
#define INTRINSIC_DEST_TYPE_FIELDNAME       "dest_type"
#define INTRINSIC_SRC_TYPE_FIELDNAME        "src_type"
#define INTRINSIC_DESTDEPEND_FIELDNAME      "dest_depend"
#define INTRINSIC_SRCDEPEND_FIELDNAME       "src_depend"
#define INTRINSIC_ENABLED_FIELDNAME         "enabled"
#define INTRINSIC_ENABLEDOPTI_FIELDNAME     "enabledopti"

/**************************************************************************
 * Various placeholders for MD indexing.
 *************************************************************************/
MD *L_intrinsic_md_g;
MD_Section *L_intrinsic_registers_section_g;
MD_Section *L_intrinsic_intrinsic_section_g;
MD_Field_Decl *L_intrinsic_functioncall_field_decl_g;
MD_Field_Decl *L_intrinsic_functioncallsilent_field_decl_g;
MD_Field_Decl *L_intrinsic_opcode_field_decl_g;
MD_Field_Decl *L_intrinsic_desttype_field_decl_g;
MD_Field_Decl *L_intrinsic_srctype_field_decl_g;
MD_Field_Decl *L_intrinsic_destdepend_field_decl_g;
MD_Field_Decl *L_intrinsic_srcdepend_field_decl_g;
MD_Field_Decl *L_intrinsic_enabled_field_decl_g;
MD_Field_Decl *L_intrinsic_enabledopti_field_decl_g;



/**************************************************************************
 * Return true if the given L_Operand is a macro register defined as
 * an intrinsic register.
 *************************************************************************/
int
L_is_intrinsic_register (L_Operand * operand)
{
  char *mac;


  if (L_intrinsic_support_enabled == 0)
    {
      L_punt ("L_is_intrinsic_register: Intrinsic support was not enabled.\n"
              "Illegal call to an intrinsic support function.");
    }
  if (!operand)
    return (0);
  if (operand->type != L_OPERAND_MACRO)
    return (0);
  mac = L_macro_name (operand->value.mac);
  if (MD_find_entry (L_intrinsic_registers_section_g, mac))
    return (1);
  return (0);
}


static void
strunquote (char *dest, char *src)
{
  strncpy (dest, &src[1], 80);
  dest[strlen (dest) - 1] = '\0';
}


int
L_intrinsic_is_opti_enabled (L_Oper * op, char *opti)
{
  L_Attr *opcode_attr;
  L_Operand *fn_name;
  char tmp_str[80];
  MD_Entry *entry;
  MD_Field *field;
  int i;



  if (L_intrinsic_support_enabled == 0)
    {
      L_punt
        ("L_intrinsic_is_opti_enabled: Intrinsic support was not enabled.\n"
         "Illegal call to an intrinsic support function.");
    }

   /***********************************************************************
    * Fetch the I_opcode attribute and look up the opcode in the database.
    **********************************************************************/
  if ((opcode_attr = L_find_attr (op->attr, "I_opcode")) == NULL)
    {
      L_punt ("L_intrinsic_is_opti_enabled: missing opcode attribute.\n"
              "Make sure this is valid intrinsic Lcode");
    }
  fn_name = opcode_attr->field[0];
  strunquote (tmp_str, fn_name->value.s);
  if ((entry = MD_find_entry (L_intrinsic_intrinsic_section_g,
                              tmp_str)) == NULL)
    {
      L_punt ("L_intrinsic_is_opti_enabled: intrinsic opcode not found in "
              "database.");
    }

  /*
   * Here's the real slow part...
   */
  if ((field = MD_find_field (entry,
                              L_intrinsic_enabledopti_field_decl_g)) == NULL)
    return (0);

  for (i = 0; i < MD_num_elements (field); i++)
    {
      if (!strcmp (opti, MD_get_link (field, i)->name))
        {
          return (1);
        }
    }

  return (0);
}


/**************************************************************************
 * Intrinsic support initialization.
 *  - Reads the intrinsic database and stores a few placeholders to 
 *    the meta description.  These placeholders are used to speed up 
 *    future references to the database.
 *  - Add the intrinsic registers as macro registers.
 *  - Also add them to a reverse lookup (symbol name to ID) so that they
 *    will be dynamically recognized when reading Lcode.
 *************************************************************************/
void
L_intrinsic_init ()
{
  FILE *fp;
  MD_Entry *entry;
  int i;


  if (L_intrinsic_support_enabled == 0)
    {
      return;
    }

   /***********************************************************************
    * Open the intrinsic database.  If none can be found, disable 
    * intrinsic support and do the best we can.
    **********************************************************************/
  if ((fp = fopen (L_intrinsic_database_filename, "r")) == NULL)
    {
      printf ("No intrinsic database file specified.  Disabling support.\n");
      L_intrinsic_support_enabled = 0;
      return;
    }
  L_intrinsic_md_g = MD_read_md (fp, L_intrinsic_database_filename);
  fclose (fp);

   /***********************************************************************
    * Query for a few placeholders to make searches faster later.
    **********************************************************************/
  L_intrinsic_registers_section_g =
    MD_find_section (L_intrinsic_md_g, INTRINSIC_REGISTERS_SECTION_NAME);
  L_intrinsic_intrinsic_section_g =
    MD_find_section (L_intrinsic_md_g, INTRINSIC_INTRINSICS_SECTION_NAME);
  L_intrinsic_functioncall_field_decl_g =
    MD_find_field_decl (L_intrinsic_intrinsic_section_g,
                        INTRINSIC_FUNCTION_CALL_FIELDNAME);
  L_intrinsic_functioncallsilent_field_decl_g =
    MD_find_field_decl (L_intrinsic_intrinsic_section_g,
                        INTRINSIC_FUNCTION_CALL_SILENT_FIELDNAME);
  L_intrinsic_opcode_field_decl_g =
    MD_find_field_decl (L_intrinsic_intrinsic_section_g,
                        INTRINSIC_OPCODE_FIELDNAME);
  L_intrinsic_desttype_field_decl_g =
    MD_find_field_decl (L_intrinsic_intrinsic_section_g,
                        INTRINSIC_DEST_TYPE_FIELDNAME);
  L_intrinsic_srctype_field_decl_g =
    MD_find_field_decl (L_intrinsic_intrinsic_section_g,
                        INTRINSIC_SRC_TYPE_FIELDNAME);
  L_intrinsic_destdepend_field_decl_g =
    MD_find_field_decl (L_intrinsic_intrinsic_section_g,
                        INTRINSIC_DESTDEPEND_FIELDNAME);
  L_intrinsic_srcdepend_field_decl_g =
    MD_find_field_decl (L_intrinsic_intrinsic_section_g,
                        INTRINSIC_SRCDEPEND_FIELDNAME);
  L_intrinsic_enabled_field_decl_g =
    MD_find_field_decl (L_intrinsic_intrinsic_section_g,
                        INTRINSIC_ENABLED_FIELDNAME);
  L_intrinsic_enabledopti_field_decl_g =
    MD_find_field_decl (L_intrinsic_intrinsic_section_g,
                        INTRINSIC_ENABLEDOPTI_FIELDNAME);

   /***********************************************************************
    * Add our intrinsic registers as macro registers.  This
    * facilitates dependency drawing.
    **********************************************************************/
  for (i = 0, entry = MD_first_entry (L_intrinsic_registers_section_g);
       entry != NULL; entry = MD_next_entry (entry), i++)
    {
#ifdef LP64_ARCHITECTURE
      L_add_symbol (L_macro_symbol_table, entry->name,
                    (long)(L_INTRINSIC_MACRO_REG_START + i));
#else
      L_add_symbol (L_macro_symbol_table, entry->name,
                    L_INTRINSIC_MACRO_REG_START + i);
#endif
      L_add_id_symbol (L_macro_id_symbol_table,
                       L_INTRINSIC_MACRO_REG_START + i, entry->name);
    }
}
