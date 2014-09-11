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
 * \brief A 'lexer' for the symbol table level parser.
 *
 * \author Robert Kidd and Wen-mei Hwu
 *
 * PST_yylex calls the Pcode level parser to provide tokens for the
 * symbol table level parser.
 */

#include <config.h>
#include <stdio.h>
#include "pcode.h"
#include "struct.h"
#include "io_util.h"
#include "read.h"
#include "read_symtab.h"
#include "lex_symtab.h"
#if 0
#include "parse_symtab.h"
#endif

/*! The current input file for the symbol table. */
FILE *PST_yyin;

/*! \brief Returns the next token to the symbol table level parser.
 *
 * \param lvalp
 *  returns the token data.
 *
 * \return
 *  The token type.  The token data is returned in \a lvalp.
 *
 * Calls the Pcode level parser to provide a token to the symbol table
 * parser. */
int
PST_yylex (YYSTYPE *lvalp)
{
  Dcl dcl;
  int type = 0;

  if ((dcl = P_read_dcl (PST_yyin)))
    {
      switch (P_GetDclType (dcl))
	{
	case TT_INCLUDE:
	  {
	    char *include_file_name;

	    include_file_name = P_GetDclInclude (dcl);
	    P_SetDclInclude (dcl, NULL);
	    dcl = P_RemoveDcl (dcl);

	    type = INCLUDE_NAME;
	    lvalp->_includename = include_file_name;
	  }
	  break;
	case TT_SYMBOLTABLE:
	  {
	    SymbolTable table;

	    table = P_GetDclSymbolTable (dcl);
	    P_SetDclSymbolTable (dcl, NULL);
	    dcl = P_RemoveDcl (dcl);

	    if (table)
	      type = SYMBOL_TABLE_BEGIN;
	    else
	      type = SYMBOL_TABLE_END;

	    lvalp->_symboltable = table;
	  }
	  break;
	case TT_IPSYMTABENT:
	  {
	    IPSymTabEnt ipste;

	    ipste = P_GetDclIPSymTabEnt (dcl);
	    P_SetDclIPSymTabEnt (dcl, NULL);
	    dcl = P_RemoveDcl (dcl);

	    if (P_GetIPSymTabEntFileType (ipste) == FT_HEADER)
	      type = IP_SYM_TAB_ENT_HEADER;
	    else if (P_GetIPSymTabEntFileType (ipste) == FT_SOURCE)
	      type = IP_SYM_TAB_ENT_SOURCE;
	    else
	      P_punt ("lex_symtab.c:PST_yylex:%d Unknown file type %d",
		      __LINE__ - 1, P_GetIPSymTabEntFileType (ipste));

	    lvalp->_ipsymtabent = ipste;
	  }
	  break;
	case TT_SYMTABENTRY:
	  {
	    SymTabEntry entry;

	    entry = P_GetDclSymTabEntry (dcl);
	    P_SetDclSymTabEntry (dcl, NULL);
	    dcl = P_RemoveDcl (dcl);

	    if (entry)
	      {
		switch (P_GetSymTabEntryType (entry))
		  {
		  case ET_BLOCK:
		    type = SYM_TAB_ENTRY_BLOCK;
		    break;
		  case ET_SCOPE:
		    type = SYM_TAB_ENTRY_SCOPE;
		    break;
		  case ET_FUNC:
		    type = SYM_TAB_ENTRY_FUNC;
		    break;
		  default:
		    type = SYM_TAB_ENTRY_OTHER;
		    break;
		  }
	      }
	    else
	      {
		type = SYM_TAB_ENTRY_END;
	      }

	    lvalp->_symtabentry = entry;
	  }
	  break;
	case TT_FUNC:
	  type = DCL_FUNC;
	  lvalp->_dcl = dcl;
	  break;
	default:
	  type = DCL;
	  lvalp->_dcl = dcl;
	  break;
	}
    }

  return (type);
}
