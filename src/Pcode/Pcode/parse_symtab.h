/* A Bison parser, made by GNU Bison 1.875.  */

/* Skeleton parser for Yacc-like parsing with Bison,
   Copyright (C) 1984, 1989, 1990, 2000, 2001, 2002 Free Software Foundation, Inc.

   This program is free software; you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation; either version 2, or (at your option)
   any later version.

   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
   GNU General Public License for more details.

   You should have received a copy of the GNU General Public License
   along with this program; if not, write to the Free Software
   Foundation, Inc., 59 Temple Place - Suite 330,
   Boston, MA 02111-1307, USA.  */

/* As a special exception, when this file is copied by Bison into a
   Bison output file, you may use that output file without restriction.
   This special exception was added by the Free Software Foundation
   in version 1.24 of Bison.  */

/* Tokens.  */
#ifndef YYTOKENTYPE
# define YYTOKENTYPE
   /* Put the tokens into the symbol table, so that GDB and other debuggers
      know about them.  */
   enum yytokentype {
     DCL = 258,
     DCL_FUNC = 259,
     INCLUDE_NAME = 260,
     IP_SYM_TAB_ENT_HEADER = 261,
     IP_SYM_TAB_ENT_SOURCE = 262,
     SYMBOL_TABLE_BEGIN = 263,
     SYMBOL_TABLE_END = 264,
     SYM_TAB_ENTRY_BLOCK = 265,
     SYM_TAB_ENTRY_END = 266,
     SYM_TAB_ENTRY_FUNC = 267,
     SYM_TAB_ENTRY_OTHER = 268,
     SYM_TAB_ENTRY_SCOPE = 269
   };
#endif
#define DCL 258
#define DCL_FUNC 259
#define INCLUDE_NAME 260
#define IP_SYM_TAB_ENT_HEADER 261
#define IP_SYM_TAB_ENT_SOURCE 262
#define SYMBOL_TABLE_BEGIN 263
#define SYMBOL_TABLE_END 264
#define SYM_TAB_ENTRY_BLOCK 265
#define SYM_TAB_ENTRY_END 266
#define SYM_TAB_ENTRY_FUNC 267
#define SYM_TAB_ENTRY_OTHER 268
#define SYM_TAB_ENTRY_SCOPE 269




#if ! defined (YYSTYPE) && ! defined (YYSTYPE_IS_DECLARED)
#line 83 "parse_symtab.y"
typedef union YYSTYPE {
  char *_includename;
  Dcl _dcl;
  List _list;
  SymTabEntry _symtabentry;
  IPSymTabEnt _ipsymtabent;
  SymbolTable _symboltable;
} YYSTYPE;
/* Line 1248 of yacc.c.  */
#line 73 "src/Pcode/Pcode/parse_s"
# define yystype YYSTYPE /* obsolescent; will be withdrawn */
# define YYSTYPE_IS_DECLARED 1
# define YYSTYPE_IS_TRIVIAL 1
#endif





