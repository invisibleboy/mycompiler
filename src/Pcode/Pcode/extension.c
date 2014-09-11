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
/*! \file
 * \brief Functions to manage the Extension field.
 *
 * \author Robert Kidd and Wen-mei Hwu
 */

#include <config.h>
#include <malloc.h>
#include <string.h>
#include "pcode.h"
#include "extension.h"
#include "struct.h"
#include "query.h"

/*! \brief Sets up a struct's extension field for a module.
 *
 * \param struct_type
 *  the type of struct for which the extension field will be set up.
 * \param alloc
 *  a function to allocate a data struct.
 * \param free
 *  a function to free a data struct.
 *
 * \return
 *  The index in the extension array for the module's data.  This is always
 *  0.
 *
 * Sets up a struct's extension field for a module.  The module's data
 * will be allocated at index 0 in the struct's ext array.
 *
 * \sa #P_ExtRegisterCopyM(), #P_ExtRegisterReadM(), #P_ExtRegisterWriteM(),
 * P_ExtSetupL(), P_ExtRegisterCopyL(), P_ExtRegisterReadL(),
 * P_ExtRegisterWriteL() */
int
P_ExtSetupM (_ExtStruct struct_type, AllocHandler alloc, FreeHandler free)
{
  /* Allocate a struct to track the handlers if necessary. */
  if (Handlers[struct_type] == NULL)
    {
      Handlers[struct_type] = malloc (sizeof (ExtHandler));

      if (Handlers[struct_type] == NULL)
	{
	  P_punt ("extension.c:P_ExtSetupM:%d Could not alloc handler",
		  __LINE__ - 1);
	}

      memset (Handlers[struct_type], 0, sizeof (ExtHandler));

      /* A library handler may have been added before this one.  If so,
       * we already counted the space for the module's extension.
       * Therefore, we only need to update NumExtensions if it is zero. */
      if (NumExtensions[struct_type] == 0)
	NumExtensions[struct_type] = 1;
    }

  Handlers[struct_type][0].alloc = alloc;
  Handlers[struct_type][0].free = free;

  return (0);
}

/*! \brief Sets up a struct's extension field for a library.
 *
 * \param struct_type
 *  the type of struct for which the extension field will be set up.
 * \param alloc
 *  a function to allocate a data struct.
 * \param free
 *  a function to free a data struct.
 *
 * \return
 *  The index in the extension array for the library's data.
 *
 * Sets up a struct's extension field for a library.  The library's data
 * will be allocated at the returned index in the struct's ext array.
 *
 * \sa P_ExtSetupM(), #P_ExtRegisterCopyM(), #P_ExtRegisterReadM(),
 * #P_ExtRegisterWriteM(), P_ExtRegisterCopyL(), P_ExtRegisterReadL(),
 * P_ExtRegisterWriteL() */
int
P_ExtSetupL (_ExtStruct struct_type, AllocHandler alloc, FreeHandler free)
{
  int new_index;

  /* If this is the first extension to be added, we need to force it
   * to select index 1, as that is the first library index.  Otherwise,
   * just select the next index. */
  if (NumExtensions[struct_type] == 0)
    new_index = 1;
  else
    new_index = NumExtensions[struct_type];

  /* Increment the number of extensions on the struct. */
  NumExtensions[struct_type] = new_index + 1;

  /* Allocate and initialize a struct to track the handlers. */
  if (Handlers[struct_type] == NULL)
    {
      Handlers[struct_type] = \
	malloc (sizeof (ExtHandler) * NumExtensions[struct_type]);
      
      if (Handlers[struct_type] == NULL)
	{
	  P_punt ("extension.c:P_ExtSetupL:%d Could not alloc handler",
		  __LINE__ - 1);
	}

      memset (Handlers[struct_type], 0,
	      sizeof (ExtHandler) * NumExtensions[struct_type]);
    }
  else
    {
      Handlers[struct_type] = \
	realloc (Handlers[struct_type],
		 sizeof (ExtHandler) * NumExtensions[struct_type]);

      if (Handlers[struct_type] == NULL)
	{
	  P_punt ("extension.c:P_ExtSetupL:%d Could not alloc handler",
		  __LINE__ - 1);
	}

      memset ((void *)&(Handlers[struct_type][new_index]), 0,
	      sizeof (ExtHandler));
    }

  Handlers[struct_type][new_index].alloc = alloc;
  Handlers[struct_type][new_index].free = free;

  return (new_index);
}

/*! \brief Sets the copy handler for a struct's extension field for a library.
 *
 * \param struct_type
 *  the type of struct for which the extension field will be set up.
 * \param index
 *  the library's index in the struct's extension field.
 * \param copy
 *  a function to copy a data struct.
 *
 * If a data struct is to be copied along with the Pcode struct, a copy
 * handler must be registered.
 *
 * \sa P_ExtSetupM(), #P_ExtRegisterCopyM(), #P_ExtRegisterReadM(),
 * #P_ExtRegisterWriteM(), P_ExtSetupL(), P_ExtRegisterReadL(),
 * P_ExtRegisterWriteL() */
void
P_ExtRegisterCopyL (_ExtStruct struct_type, int index, CopyHandler copy)
{
  if (index >= NumExtensions[struct_type])
    {
      P_punt ("extension.c:P_ExtRegisterCopyL:%d Requested index %d for "
	      "struct %d\ngreater than max %d\n", __LINE__ - 1, index,
	      struct_type, NumExtensions[struct_type] - 1);
    }

  Handlers[struct_type][index].copy = copy;

  return;
}

/*! \brief Sets the read handler for a struct's extension field for a library.
 *
 * \param struct_type
 *  the type of struct for which the extension field will be set up.
 * \param index
 *  the library's index in the struct's extension field.
 * \param read
 *  a function to read a data struct from a character string.
 * \param sig
 *  the signature of the data to read.
 *
 * Sets a read handler for a struct's extension field.  When the Pcode library
 * read routines read a struct of type \a struct_type, any extension data with
 * signature \a sig will be passed to \a read for processing.
 *
 * The string passed as \a sig is copied, so the caller is responsible for
 * freeing its copy.
 *
 * \sa P_ExtSetupM(), #P_ExtRegisterCopyM(), #P_ExtRegisterReadM(),
 * #P_ExtRegisterWriteM(), P_ExtSetupL(), P_ExtRegisterCopyL(),
 * P_ExtRegisterWriteL() */
void
P_ExtRegisterReadL (_ExtStruct struct_type, int index, ReadHandler read,
		    char *sig)
{
  if (index >= NumExtensions[struct_type])
    {
      P_punt ("extension.c:P_ExtRegisterReadL:%d Requested index %d for "
	      "struct %d\ngreater than max %d\n", __LINE__ - 1, index,
	      struct_type, NumExtensions[struct_type] - 1);
    }

  if (Handlers[struct_type][index].signature[0] != '\0' && \
      strcmp (Handlers[struct_type][index].signature, sig) != 0)
    {
      P_punt ("extension.c:P_ExtRegisterReadL:%d Cannot register read handler "
	      "for\nsignature %s.  Handler already registered for signature "
	      "%s", __LINE__ - 2, sig, Handlers[struct_type][index].signature);
    }

  if (strlen (sig) > EXT_SIG_LENGTH)
    {
      P_punt ("extension.c:P_ExtRegisterReadL:%d signature %s too long (max "
	      "%d chars)", __LINE__ - 1, sig, EXT_SIG_LENGTH);
    }

  Handlers[struct_type][index].read = read;
  
  if (Handlers[struct_type][index].signature[0] == '\0')
    memcpy (Handlers[struct_type][index].signature, sig, EXT_SIG_LENGTH);

  return;
}

/*! \brief Sets the write handler for a struct's extension field for a library.
 *
 * \param struct_type
 *  the type of struct for which the extension field will be set up.
 * \param index
 *  the library's index in the struct's extension field.
 * \param write
 *  a function to write a character string from the data struct
 * \param sig
 *  the signature of the data to write.
 *
 * Sets a write handler for a struct's extension field.  When the Pcode library
 * write routines write a struct of type \a struct_type, any extension data
 * will be passed to \a write to be packed into a character string.
 *
 * The string passed as \a sig is copied, so the caller is responsible for
 * freeing its copy.
 *
 * \sa P_ExtSetupM(), #P_ExtRegisterCopyM(), #P_ExtRegisterReadM(),
 * #P_ExtRegisterWriteM(), P_ExtSetupL(), P_ExtRegisterCopyL(),
 * P_ExtRegisterReadL() */
void
P_ExtRegisterWriteL (_ExtStruct struct_type, int index, WriteHandler write,
		     char *sig)
{
  if (index >= NumExtensions[struct_type])
    {
      P_punt ("extension.c:P_ExtRegisterWriteL:%d Requested index %d for "
	      "struct %d\ngreater than max %d\n", __LINE__ - 1, index,
	      struct_type, NumExtensions[struct_type] - 1);
    }
      
  if (Handlers[struct_type][index].signature[0] != '\0' && \
      strcmp (Handlers[struct_type][index].signature, sig) != 0)
    {
      P_punt ("extension.c:P_ExtRegisterWriteL:%d Cannot register write "
	      "handler for\n signature %s.  Handler already registered for "
	      "signature %s", __LINE__ - 2, sig,
	      Handlers[struct_type][index].signature);
    }

  if (strlen (sig) > EXT_SIG_LENGTH)
    {
      P_punt ("extension.c:P_ExtRegisterWriteL:%d signature %s too long (max "
	      "%d chars)", __LINE__ - 1, sig, EXT_SIG_LENGTH);
    }

  Handlers[struct_type][index].write = write;

  if (Handlers[struct_type][index].signature[0] == '\0')
    memcpy (Handlers[struct_type][index].signature, sig, EXT_SIG_LENGTH);

  return;
}
  
/*! \brief Reads extension data from the a struct's pragma list.
 *
 * \param struct_type
 *  the type of struct to process.
 * \param data
 *  the struct to process.
 *
 * This function assumes that there are handlers defined for the struct.
 * This function searches \a data's pragma field for pragmas with
 * \a EXT_DATA_TAG as the specifier.  If any of these pragmas have a signature
 * that matches a handler, the signature, data, and extension structure
 * are passed to the read handler for processing.
 *
 * \sa P_ExtWrite()
 */
void
P_ExtRead (_ExtStruct struct_type, void *data)
{
  Pragma pragma_list = NULL, p = NULL;
  Extension *ext = NULL;
  int i;

  switch (struct_type)
    {
    case ES_FUNC:
      {
	FuncDcl f = (FuncDcl)data;

	pragma_list = f->pragma;
	ext = f->ext;
      }
      break;

    case ES_TYPE:
      {
	TypeDcl t = (TypeDcl)data;

	pragma_list = t->pragma;
	ext = t->ext;
      }
      break;

    case ES_VAR:
      {
	VarDcl v = (VarDcl)data;

	pragma_list = v->pragma;
	ext = v->ext;
      }
      break;

    case ES_INIT:
      {
	Init i = (Init)data;

	pragma_list = i->pragma;
	ext = i->ext;
      }
      break;

    case ES_STRUCT:
      {
	StructDcl s = (StructDcl)data;

	pragma_list = s->pragma;
	ext = s->ext;
      }
      break;

    case ES_UNION:
      {
	UnionDcl u = (UnionDcl)data;

	pragma_list = u->pragma;
	ext = u->ext;
      }
      break;

    case ES_FIELD:
      {
	Field f = (Field)data;

	pragma_list = f->pragma;
	ext = f->ext;
      }
      break;

    case ES_ENUM:
      {
	EnumDcl e = (EnumDcl)data;

	pragma_list = e->pragma;
	ext = e->ext;
      }
      break;

    case ES_STMT:
      {
	Stmt s = (Stmt)data;

	pragma_list = s->pragma;
	ext = s->ext;
      }
      break;

    case ES_PSTMT:
      {
	Pstmt p = (Pstmt)data;

	pragma_list = p->pragma;
	ext = p->ext;
      }
      break;

    case ES_EXPR:
      {
	Expr e = (Expr)data;

	pragma_list = e->pragma;
	ext = e->ext;
      }
      break;

    case ES_ASM:
      {
	AsmDcl a = (AsmDcl)data;

	pragma_list = a->pragma;
	ext = a->ext;
      }
      break;

    case ES_SYMTABENTRY:
      {
	SymTabEntry s = (SymTabEntry)data;

	pragma_list = s->pragma;
	ext = s->ext;
      }
      break;

    case ES_IPSYMTABENT:
      {
	IPSymTabEnt i = (IPSymTabEnt)data;

	pragma_list = i->pragma;
	ext = i->ext;
      }
      break;

    default:
      P_punt ("extension.c:P_ExtRead:%d Invalid struct_type %d", __LINE__,
	      struct_type);
    }

  for (i = 0; i < NumExtensions[struct_type]; i++)
    {
      if (Handlers[struct_type][i].read)
	{
	  p = pragma_list;

	  /* If this extension field is tagged to be manually allocated, it
	   * won't be allocated by the P_New*() function.  We need to call
	   * its allocation function now. */
	  if (ext[i] == NULL && \
	      Handlers[struct_type][i].options & HO_MANUAL_ALLOC)
	    ext[i] = Handlers[struct_type][i].alloc ();

	  while ((p = P_FindPragma (p, EXT_DATA_TAG)))
	    {
	      Expr e = P_GetPragmaExpr (p);
	      char *sig = P_GetExprString (e);
	      char *data = P_GetExprString (P_GetExprNext (e));

	      if (strcmp (Handlers[struct_type][i].signature, sig) == 0)
		Handlers[struct_type][i].read (ext[i], sig, data);

	      p = P_GetPragmaNext (p);
	    }
	}
    }
  
  return;
}

/*! \brief Writes extension data to a struct's pragma list.
 *
 * \param struct_type
 *  the type of struct to process.
 * \param data
 *  the struct to process.
 *
 * This function assumes that there are handlers defined for the struct.
 * If any extension field has a write handler defined, that field is passed
 * to the handler, and the resulting string is inserted into the structure's
 * pragma list.
 *
 * \sa P_ExtRead()
 */
void
P_ExtWrite (_ExtStruct struct_type, void *data)
{
  Pragma pragma_list = NULL;
  Extension *ext = NULL;
  FuncDcl func_dcl = NULL;
  TypeDcl type_dcl = NULL;
  VarDcl var_dcl = NULL;
  Init init = NULL;
  StructDcl struct_dcl = NULL;
  UnionDcl union_dcl = NULL;
  Field field = NULL;
  EnumDcl enum_dcl = NULL;
  Stmt stmt = NULL;
  Pstmt pstmt = NULL;
  Expr expr = NULL;
  AsmDcl asm_dcl = NULL;
  SymTabEntry sym_tab_entry = NULL;
  IPSymTabEnt ip_sym_tab_ent = NULL;
  int i;

  switch (struct_type)
    {
    case ES_FUNC:
      func_dcl = (FuncDcl)data;
      pragma_list = func_dcl->pragma;
      ext = func_dcl->ext;
      break;

    case ES_TYPE:
      type_dcl = (TypeDcl)data;
      pragma_list = type_dcl->pragma;
      ext = type_dcl->ext;
      break;

    case ES_VAR:
      var_dcl = (VarDcl)data;
      pragma_list = var_dcl->pragma;
      ext = var_dcl->ext;
      break;

    case ES_INIT:
      init = (Init)data;
      pragma_list = init->pragma;
      ext = init->ext;
      break;

    case ES_STRUCT:
      struct_dcl = (StructDcl)data;
      pragma_list = struct_dcl->pragma;
      ext = struct_dcl->ext;
      break;

    case ES_UNION:
      union_dcl = (UnionDcl)data;
      pragma_list = union_dcl->pragma;
      ext = union_dcl->ext;
      break;

    case ES_ENUM:
      enum_dcl = (EnumDcl)data;
      pragma_list = enum_dcl->pragma;
      ext = enum_dcl->ext;
      break;

    case ES_STMT:
      stmt = (Stmt)data;
      pragma_list = stmt->pragma;
      ext = stmt->ext;
      break;

    case ES_PSTMT:
      pstmt = (Pstmt)data;
      pragma_list = pstmt->pragma;
      ext = pstmt->ext;
      break;

    case ES_EXPR:
      expr = (Expr)data;
      pragma_list = expr->pragma;
      ext = expr->ext;
      break;

    case ES_ASM:
      asm_dcl = (AsmDcl)data;
      pragma_list = asm_dcl->pragma;
      ext = asm_dcl->ext;
      break;

    case ES_SYMTABENTRY:
      sym_tab_entry = (SymTabEntry)data;
      pragma_list = sym_tab_entry->pragma;
      ext = sym_tab_entry->ext;
      break;
      
    case ES_IPSYMTABENT:
      ip_sym_tab_ent = (IPSymTabEnt)data;
      pragma_list = ip_sym_tab_ent->pragma;
      ext = ip_sym_tab_ent->ext;
      break;

    default:
      P_punt ("extension.c:P_ExtWrite:%d Unknown struct_type %d", __LINE__,
	      struct_type);
    }

  /* Build a pragma list from the extension field. */
  for (i = 0; i < NumExtensions[struct_type]; i++)
    {
      /* If an extension field is to be manually allocated, it might
       * not exist on this struct. */
      if (ext[i] && Handlers[struct_type][i].write)
	{
	  Pragma p;
	  int pragma_inserted = 0;
	  char *ext_string;

	  if (!Handlers[struct_type][i].copy)
	    P_warn ("extension.c:P_ExtWrite:%d Writing extension field "
		    "requires copy handler.\n"
		    "Copy handler not defined for %s", __LINE__ - 2,
		    ExtStruct_To_String[struct_type]);

	  ext_string = \
	    Handlers[struct_type][i].write (Handlers[struct_type][i].signature,
					    ext[i]);
	  if (ext_string == NULL || ext_string[0] == 0)
	    {
	      if (ext_string)
		free(ext_string);
	      continue;
	    }

	  /* Search the pragma list for an old data pragma with our
	   * signature.   */
	  p = pragma_list;

	  while ((p = P_FindPragma (p, EXT_DATA_TAG)))
	    {
	      if (strcmp (P_GetExprString (P_GetPragmaExpr (p)),
			  Handlers[struct_type][i].signature) == 0)
		{
		  Expr data_expr = P_GetExprNext (P_GetPragmaExpr (p));

		  free (P_GetExprString (data_expr));
		  P_SetExprString (data_expr, ext_string);

 		  pragma_inserted = 1;
		  break;
		}

	      p = P_GetPragmaNext (p);
	    }

	  if (!pragma_inserted)
	    {
	      Pragma new_pragma;
	      Expr e;

	      e = P_NewStringExpr (Handlers[struct_type][i].signature);
	      e = P_AppendExprNext (e, P_NewStringExpr (ext_string));
	      new_pragma = P_NewPragmaWithSpecExpr (EXT_DATA_TAG, e);

	      pragma_list = P_AppendPragmaNext (pragma_list, new_pragma);
	    }
	}
    }

  /* Attach the pragma list to the original struct. */
  switch (struct_type)
    {
    case ES_FUNC:
      func_dcl->pragma = pragma_list;
      break;

    case ES_TYPE:
      type_dcl->pragma = pragma_list;
      break;

    case ES_VAR:
      var_dcl->pragma = pragma_list;
      break;

    case ES_INIT:
      init->pragma = pragma_list;
      break;

    case ES_STRUCT:
      struct_dcl->pragma = pragma_list;
      break;

    case ES_UNION:
      union_dcl->pragma = pragma_list;
      break;

    case ES_FIELD:
      field->pragma = pragma_list;
      break;

    case ES_ENUM:
      enum_dcl->pragma = pragma_list;
      break;

    case ES_STMT:
      stmt->pragma = pragma_list;
      break;

    case ES_PSTMT:
      pstmt->pragma = pragma_list;
      break;

    case ES_EXPR:
      expr->pragma = pragma_list;
      break;

    case ES_ASM:
      asm_dcl->pragma = pragma_list;
      break;

    case ES_SYMTABENTRY:
      sym_tab_entry->pragma = pragma_list;
      break;

    case ES_IPSYMTABENT:
      ip_sym_tab_ent->pragma = pragma_list;
      break;

    default:
      P_punt ("extension.c:P_ExtWrite:%d Unknown struct_type %d", __LINE__,
	      struct_type);
    }

  return;
}

/*! \brief Frees memory allocated for extension handlers.
 */
void
P_ExtCleanup ()
{
  int i;

  for (i = 0; i < ES_LAST; i++)
    {
      if (Handlers[i])
	free (Handlers[i]);
    }

  return;
}
