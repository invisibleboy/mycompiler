/* A Bison parser, made by GNU Bison 1.875c.  */

/* Skeleton parser for Yacc-like parsing with Bison,
   Copyright (C) 1984, 1989, 1990, 2000, 2001, 2002, 2003 Free Software Foundation, Inc.

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

/* Written by Richard Stallman by simplifying the original so called
   ``semantic'' parser.  */

/* All symbols defined below should begin with yy or YY, to avoid
   infringing on user name space.  This should be done even for local
   variables, as they might otherwise be expanded by user macros.
   There are some unavoidable exceptions within include files to
   define necessary library symbols; they are noted "INFRINGES ON
   USER NAME SPACE" below.  */

/* Identify Bison output.  */
#define YYBISON 1

/* Skeleton name.  */
#define YYSKELETON_NAME "yacc.c"

/* Pure parsers.  */
#define YYPURE 0

/* Using locations.  */
#define YYLSP_NEEDED 0



/* Tokens.  */
#ifndef YYTOKENTYPE
# define YYTOKENTYPE
   /* Put the tokens into the symbol table, so that GDB and other debuggers
      know about them.  */
   enum yytokentype {
     CONST = 258,
     VOLATILE = 259,
     AUTO = 260,
     STATIC = 261,
     EXTERN = 262,
     REGISTER = 263,
     GLOBAL = 264,
     SIGNED = 265,
     UNSIGNED = 266,
     VOID = 267,
     CHAR = 268,
     SHORT = 269,
     INT = 270,
     LONG = 271,
     LONGLONG = 272,
     FLOAT = 273,
     STRING = 274,
     DOUBLE = 275,
     LONGDOUBLE = 276,
     CDECL = 277,
     STDCALL = 278,
     FASTCALL = 279,
     DOT = 280,
     ARROW = 281,
     CAST = 282,
     EXPRSIZE = 283,
     TYPESIZE = 284,
     QUEST = 285,
     DISJ = 286,
     CONJ = 287,
     COMPEXPR = 288,
     ASSIGN = 289,
     OR = 290,
     XOR = 291,
     AND = 292,
     EQ = 293,
     NE = 294,
     LT = 295,
     LE = 296,
     GE = 297,
     GT = 298,
     RSHFT = 299,
     LSHFT = 300,
     ADD = 301,
     SUB = 302,
     MUL = 303,
     DIV = 304,
     MOD = 305,
     NEG = 306,
     NOT = 307,
     INV = 308,
     PREINC = 309,
     PREDEC = 310,
     POSTINC = 311,
     POSTDEC = 312,
     A_ADD = 313,
     A_SUB = 314,
     A_MUL = 315,
     A_DIV = 316,
     A_MOD = 317,
     A_RSHFT = 318,
     A_LSHFT = 319,
     A_AND = 320,
     A_OR = 321,
     A_XOR = 322,
     INDR = 323,
     ADDR = 324,
     INDEX = 325,
     CALL = 326,
     ASMOPRD = 327,
     STMTEXPR = 328,
     DEF = 329,
     TYPEDEF = 330,
     STRUCT = 331,
     UNION = 332,
     ENUM = 333,
     ERROR = 334,
     SIZE = 335,
     ALIGNMENT = 336,
     POS = 337,
     PROFILE = 338,
     SHADOW = 339,
     PRAGMA = 340,
     BREAK = 341,
     CONTINUE = 342,
     KW_NULL = 343,
     COMPSTMT = 344,
     DO = 345,
     WHILE = 346,
     FOR = 347,
     IF = 348,
     THEN = 349,
     ELSE = 350,
     SWITCH = 351,
     RETURN = 352,
     GOTO = 353,
     PSTMT = 354,
     ADVANCE = 355,
     AWAIT = 356,
     DOSERIAL = 357,
     INIT = 358,
     FINAL = 359,
     INC = 360,
     DOALL = 361,
     DOACROSS = 362,
     DOSUPER = 363,
     MUTEX = 364,
     COBEGIN = 365,
     ASM = 366,
     OPERANDS = 367,
     CLOBBERS = 368,
     ID = 369,
     ARRAY = 370,
     FUNCTION = 371,
     POINTER = 372,
     SYNC = 373,
     PARAMETER = 374,
     PARAM = 375,
     LABEL = 376,
     CASE = 377,
     DEFAULT = 378,
     VARARG = 379,
     BITFIELD = 380,
     VAR = 381,
     REAL = 382,
     TYPE = 383,
     SYM = 384,
     SCOPE = 385,
     IP_TABLE = 386,
     SYMBOL_TABLE = 387,
     NUM_ENTRIES = 388,
     BLOCK = 389,
     EXPR = 390,
     STMT = 391,
     FUNC = 392,
     OFFSET = 393,
     EXPLICIT_ALIGNMENT = 394,
     ENUMFIELD = 395,
     FIELD = 396,
     KEY = 397,
     DIM = 398,
     HEADER = 399,
     SOURCE = 400,
     IN = 401,
     OUT = 402,
     INCLUDE = 403,
     PARENT = 404,
     GVAR = 405,
     LTYPE = 406,
     REFS = 407,
     EMPTY = 408,
     INCOMPLETE = 409,
     IMPLICIT = 410,
     COMMON = 411,
     DEFINED = 412,
     WEAK = 413,
     COMDAT = 414,
     CONSTRUCTOR = 415,
     DESTRUCTOR = 416,
     UNNAMED = 417,
     END = 418,
     LINKED = 419,
     NOT_AVAIL = 420,
     ELLIPSIS = 421,
     OLD_PARAM = 422,
     PHI = 423,
     LINKMULTI = 424,
     I_LIT = 425,
     F_LIT = 426,
     C_LIT = 427,
     ST_LIT = 428
   };
#endif
#define CONST 258
#define VOLATILE 259
#define AUTO 260
#define STATIC 261
#define EXTERN 262
#define REGISTER 263
#define GLOBAL 264
#define SIGNED 265
#define UNSIGNED 266
#define VOID 267
#define CHAR 268
#define SHORT 269
#define INT 270
#define LONG 271
#define LONGLONG 272
#define FLOAT 273
#define STRING 274
#define DOUBLE 275
#define LONGDOUBLE 276
#define CDECL 277
#define STDCALL 278
#define FASTCALL 279
#define DOT 280
#define ARROW 281
#define CAST 282
#define EXPRSIZE 283
#define TYPESIZE 284
#define QUEST 285
#define DISJ 286
#define CONJ 287
#define COMPEXPR 288
#define ASSIGN 289
#define OR 290
#define XOR 291
#define AND 292
#define EQ 293
#define NE 294
#define LT 295
#define LE 296
#define GE 297
#define GT 298
#define RSHFT 299
#define LSHFT 300
#define ADD 301
#define SUB 302
#define MUL 303
#define DIV 304
#define MOD 305
#define NEG 306
#define NOT 307
#define INV 308
#define PREINC 309
#define PREDEC 310
#define POSTINC 311
#define POSTDEC 312
#define A_ADD 313
#define A_SUB 314
#define A_MUL 315
#define A_DIV 316
#define A_MOD 317
#define A_RSHFT 318
#define A_LSHFT 319
#define A_AND 320
#define A_OR 321
#define A_XOR 322
#define INDR 323
#define ADDR 324
#define INDEX 325
#define CALL 326
#define ASMOPRD 327
#define STMTEXPR 328
#define DEF 329
#define TYPEDEF 330
#define STRUCT 331
#define UNION 332
#define ENUM 333
#define ERROR 334
#define SIZE 335
#define ALIGNMENT 336
#define POS 337
#define PROFILE 338
#define SHADOW 339
#define PRAGMA 340
#define BREAK 341
#define CONTINUE 342
#define KW_NULL 343
#define COMPSTMT 344
#define DO 345
#define WHILE 346
#define FOR 347
#define IF 348
#define THEN 349
#define ELSE 350
#define SWITCH 351
#define RETURN 352
#define GOTO 353
#define PSTMT 354
#define ADVANCE 355
#define AWAIT 356
#define DOSERIAL 357
#define INIT 358
#define FINAL 359
#define INC 360
#define DOALL 361
#define DOACROSS 362
#define DOSUPER 363
#define MUTEX 364
#define COBEGIN 365
#define ASM 366
#define OPERANDS 367
#define CLOBBERS 368
#define ID 369
#define ARRAY 370
#define FUNCTION 371
#define POINTER 372
#define SYNC 373
#define PARAMETER 374
#define PARAM 375
#define LABEL 376
#define CASE 377
#define DEFAULT 378
#define VARARG 379
#define BITFIELD 380
#define VAR 381
#define REAL 382
#define TYPE 383
#define SYM 384
#define SCOPE 385
#define IP_TABLE 386
#define SYMBOL_TABLE 387
#define NUM_ENTRIES 388
#define BLOCK 389
#define EXPR 390
#define STMT 391
#define FUNC 392
#define OFFSET 393
#define EXPLICIT_ALIGNMENT 394
#define ENUMFIELD 395
#define FIELD 396
#define KEY 397
#define DIM 398
#define HEADER 399
#define SOURCE 400
#define IN 401
#define OUT 402
#define INCLUDE 403
#define PARENT 404
#define GVAR 405
#define LTYPE 406
#define REFS 407
#define EMPTY 408
#define INCOMPLETE 409
#define IMPLICIT 410
#define COMMON 411
#define DEFINED 412
#define WEAK 413
#define COMDAT 414
#define CONSTRUCTOR 415
#define DESTRUCTOR 416
#define UNNAMED 417
#define END 418
#define LINKED 419
#define NOT_AVAIL 420
#define ELLIPSIS 421
#define OLD_PARAM 422
#define PHI 423
#define LINKMULTI 424
#define I_LIT 425
#define F_LIT 426
#define C_LIT 427
#define ST_LIT 428




/* Copy the first part of user declarations.  */
#line 1 "parse.y"


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
/*****************************************************************************
 *	File:	 parse.y
 *      Authors: Robert Kidd and Wen-mei Hwu
 * 	Copyright (c) 2003 Robert Kidd, Wen-mei Hwu
 *		 and The Board of Trustees of the University of Illinois.
 *		 All rights reserved.
 *      License Agreement specifies the terms and conditions for 
 *      redistribution.
 *****************************************************************************/

#include <config.h>
#include <library/i_list.h>
#include <Pcode/pcode.h>
#include <Pcode/struct.h>
#include <Pcode/perror.h>
#include <Pcode/util.h>
#include <Pcode/symtab.h>
#include <Pcode/extension.h>
#include <Pcode/parse_prefix.h>
#include <Pcode/query.h>

#define YYPARSE_VERBOSE 1

int last_column;
int last_line;
char *yytext;

void yyerror (char *s);
extern int yylex (void);


/* Enabling traces.  */
#ifndef YYDEBUG
# define YYDEBUG 0
#endif

/* Enabling verbose error messages.  */
#ifdef YYERROR_VERBOSE
# undef YYERROR_VERBOSE
# define YYERROR_VERBOSE 1
#else
# define YYERROR_VERBOSE 0
#endif

#if ! defined (YYSTYPE) && ! defined (YYSTYPE_IS_DECLARED)
#line 82 "parse.y"
typedef union YYSTYPE {
  char c_val;
  char *st_val;
  long i_val;
  double f_val;

  _BasicType _basictype;
  _EntryType _entrytype;
  _FileType _filetype;
  _IPSTEFlags _ipsteflags;
  _TypeQual _typequal;
  _VarQual _varqual;
  _STFlags _stflags;
  _StructQual _structqual;

  AsmDcl _asmdcl;
  AsmStmt _asmstmt;
  Dcl _dcl;
  EnumField _enumfield;
  Expr _expr;
  Field _field;
  FuncDcl _funcdcl;
  Identifier _identifier;
  Init _init;
  IPSymTabEnt _ipsymtabent;
  Key _key;
  Label _label;
  Param _param;
  Position _position;
  Pragma _pragma;
  ProfST _profst;
  Pstmt _pstmt;
  Scope _scope;
  ScopeEntry _scopeentry;
  Stmt _stmt;
  SymbolTable _symboltable;
  SymTabEntry _symtabentry;
  TypeDcl _typedcl;
  TypeList _typelist;
  VarDcl _vardcl;
  VarList _varlist;
} YYSTYPE;
/* Line 191 of yacc.c.  */
#line 545 "src/Pcode/Pcode/parse.c"
# define yystype YYSTYPE /* obsolescent; will be withdrawn */
# define YYSTYPE_IS_DECLARED 1
# define YYSTYPE_IS_TRIVIAL 1
#endif



/* Copy the second part of user declarations.  */


/* Line 214 of yacc.c.  */
#line 557 "src/Pcode/Pcode/parse.c"

#if ! defined (yyoverflow) || YYERROR_VERBOSE

# ifndef YYFREE
#  define YYFREE free
# endif
# ifndef YYMALLOC
#  define YYMALLOC malloc
# endif

/* The parser invokes alloca or malloc; define the necessary symbols.  */

# ifdef YYSTACK_USE_ALLOCA
#  if YYSTACK_USE_ALLOCA
#   define YYSTACK_ALLOC alloca
#  endif
# else
#  if defined (alloca) || defined (_ALLOCA_H)
#   define YYSTACK_ALLOC alloca
#  else
#   ifdef __GNUC__
#    define YYSTACK_ALLOC __builtin_alloca
#   endif
#  endif
# endif

# ifdef YYSTACK_ALLOC
   /* Pacify GCC's `empty if-body' warning. */
#  define YYSTACK_FREE(Ptr) do { /* empty */; } while (0)
# else
#  if defined (__STDC__) || defined (__cplusplus)
#   include <stdlib.h> /* INFRINGES ON USER NAME SPACE */
#   define YYSIZE_T size_t
#  endif
#  define YYSTACK_ALLOC YYMALLOC
#  define YYSTACK_FREE YYFREE
# endif
#endif /* ! defined (yyoverflow) || YYERROR_VERBOSE */


#if (! defined (yyoverflow) \
     && (! defined (__cplusplus) \
	 || (defined (YYSTYPE_IS_TRIVIAL) && YYSTYPE_IS_TRIVIAL)))

/* A type that is properly aligned for any stack member.  */
union yyalloc
{
  short yyss;
  YYSTYPE yyvs;
  };

/* The size of the maximum gap between one aligned stack and the next.  */
# define YYSTACK_GAP_MAXIMUM (sizeof (union yyalloc) - 1)

/* The size of an array large to enough to hold all stacks, each with
   N elements.  */
# define YYSTACK_BYTES(N) \
     ((N) * (sizeof (short) + sizeof (YYSTYPE))				\
      + YYSTACK_GAP_MAXIMUM)

/* Copy COUNT objects from FROM to TO.  The source and destination do
   not overlap.  */
# ifndef YYCOPY
#  if defined (__GNUC__) && 1 < __GNUC__
#   define YYCOPY(To, From, Count) \
      __builtin_memcpy (To, From, (Count) * sizeof (*(From)))
#  else
#   define YYCOPY(To, From, Count)		\
      do					\
	{					\
	  register YYSIZE_T yyi;		\
	  for (yyi = 0; yyi < (Count); yyi++)	\
	    (To)[yyi] = (From)[yyi];		\
	}					\
      while (0)
#  endif
# endif

/* Relocate STACK from its old location to the new one.  The
   local variables YYSIZE and YYSTACKSIZE give the old and new number of
   elements in the stack, and YYPTR gives the new location of the
   stack.  Advance YYPTR to a properly aligned location for the next
   stack.  */
# define YYSTACK_RELOCATE(Stack)					\
    do									\
      {									\
	YYSIZE_T yynewbytes;						\
	YYCOPY (&yyptr->Stack, Stack, yysize);				\
	Stack = &yyptr->Stack;						\
	yynewbytes = yystacksize * sizeof (*Stack) + YYSTACK_GAP_MAXIMUM; \
	yyptr += yynewbytes / sizeof (*yyptr);				\
      }									\
    while (0)

#endif

#if defined (__STDC__) || defined (__cplusplus)
   typedef signed char yysigned_char;
#else
   typedef short yysigned_char;
#endif

/* YYFINAL -- State number of the termination state. */
#define YYFINAL  21
/* YYLAST -- Last index in YYTABLE.  */
#define YYLAST   851

/* YYNTOKENS -- Number of terminals. */
#define YYNTOKENS  176
/* YYNNTS -- Number of nonterminals. */
#define YYNNTS  72
/* YYNRULES -- Number of rules. */
#define YYNRULES  268
/* YYNRULES -- Number of states. */
#define YYNSTATES  655

/* YYTRANSLATE(YYLEX) -- Bison symbol number corresponding to YYLEX.  */
#define YYUNDEFTOK  2
#define YYMAXUTOK   428

#define YYTRANSLATE(YYX) 						\
  ((unsigned int) (YYX) <= YYMAXUTOK ? yytranslate[YYX] : YYUNDEFTOK)

/* YYTRANSLATE[YYLEX] -- Bison symbol number corresponding to YYLEX.  */
static const unsigned char yytranslate[] =
{
       0,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
     174,   175,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     1,     2,     3,     4,
       5,     6,     7,     8,     9,    10,    11,    12,    13,    14,
      15,    16,    17,    18,    19,    20,    21,    22,    23,    24,
      25,    26,    27,    28,    29,    30,    31,    32,    33,    34,
      35,    36,    37,    38,    39,    40,    41,    42,    43,    44,
      45,    46,    47,    48,    49,    50,    51,    52,    53,    54,
      55,    56,    57,    58,    59,    60,    61,    62,    63,    64,
      65,    66,    67,    68,    69,    70,    71,    72,    73,    74,
      75,    76,    77,    78,    79,    80,    81,    82,    83,    84,
      85,    86,    87,    88,    89,    90,    91,    92,    93,    94,
      95,    96,    97,    98,    99,   100,   101,   102,   103,   104,
     105,   106,   107,   108,   109,   110,   111,   112,   113,   114,
     115,   116,   117,   118,   119,   120,   121,   122,   123,   124,
     125,   126,   127,   128,   129,   130,   131,   132,   133,   134,
     135,   136,   137,   138,   139,   140,   141,   142,   143,   144,
     145,   146,   147,   148,   149,   150,   151,   152,   153,   154,
     155,   156,   157,   158,   159,   160,   161,   162,   163,   164,
     165,   166,   167,   168,   169,   170,   171,   172,   173
};

#if YYDEBUG
/* YYPRHS[YYN] -- Index of the first RHS symbol of rule number YYN in
   YYRHS.  */
static const unsigned short yyprhs[] =
{
       0,     0,     3,     4,     6,     8,    10,    12,    14,    16,
      18,    20,    21,    24,    25,    28,    35,    36,    39,    44,
      50,    52,    54,    56,    58,    60,    62,    64,    66,    68,
      70,    72,    74,    75,    78,    84,    86,    88,    90,    92,
      94,    96,    98,   100,   102,   104,   106,   108,   110,   112,
     114,   119,   123,   124,   127,   134,   137,   141,   144,   148,
     153,   158,   162,   166,   170,   174,   178,   182,   186,   190,
     193,   196,   201,   205,   209,   212,   216,   220,   224,   228,
     232,   236,   240,   244,   248,   252,   256,   260,   264,   268,
     272,   276,   280,   283,   286,   289,   292,   295,   298,   301,
     305,   309,   313,   317,   321,   325,   329,   333,   337,   341,
     344,   347,   351,   355,   359,   362,   364,   366,   370,   371,
     374,   378,   389,   403,   404,   407,   409,   411,   424,   435,
     438,   442,   443,   446,   451,   454,   457,   458,   461,   465,
     467,   468,   471,   486,   491,   496,   501,   505,   506,   509,
     511,   513,   515,   516,   519,   525,   526,   529,   531,   532,
     537,   541,   542,   545,   546,   549,   550,   553,   554,   557,
     562,   566,   567,   570,   571,   574,   575,   578,   589,   591,
     593,   598,   603,   610,   615,   622,   626,   628,   631,   634,
     636,   639,   643,   654,   665,   676,   687,   691,   694,   696,
     698,   700,   702,   703,   706,   710,   712,   714,   716,   718,
     719,   722,   729,   734,   739,   746,   756,   767,   769,   770,
     773,   778,   790,   795,   804,   813,   821,   822,   825,   829,
     831,   833,   835,   837,   839,   841,   843,   844,   847,   860,
     874,   891,   909,   924,   938,   947,   957,   958,   961,   965,
     967,   969,   971,   973,   975,   977,   979,   981,   983,   985,
     987,   989,   991,   993,   995,   997,   999,  1001,  1002
};

/* YYRHS -- A `-1'-separated list of the rules' RHS. */
static const short yyrhs[] =
{
     177,     0,    -1,    -1,   197,    -1,   243,    -1,   180,    -1,
     200,    -1,   236,    -1,   232,    -1,   206,    -1,   233,    -1,
      -1,    81,   170,    -1,    -1,   113,   193,    -1,   174,   182,
     207,   217,   215,   175,    -1,    -1,   112,   193,    -1,   111,
     189,   181,   179,    -1,   111,     4,   189,   181,   179,    -1,
      12,    -1,    13,    -1,    14,    -1,    15,    -1,    16,    -1,
      17,    -1,    18,    -1,    20,    -1,    21,    -1,    11,    -1,
     124,    -1,   125,    -1,    -1,   184,   183,    -1,    89,   170,
     239,   245,   229,    -1,   137,    -1,   151,    -1,   128,    -1,
     126,    -1,   150,    -1,    76,    -1,    77,    -1,    78,    -1,
     111,    -1,   136,    -1,   135,    -1,   141,    -1,   140,    -1,
     121,    -1,   130,    -1,   174,   198,   189,   175,    -1,   174,
     198,   175,    -1,    -1,   188,   187,    -1,   174,   170,   191,
     217,   220,   175,    -1,   174,   175,    -1,   174,   189,   175,
      -1,   126,   198,    -1,    15,   170,   207,    -1,    10,    15,
     170,   207,    -1,    11,    15,   170,   207,    -1,   127,   171,
     207,    -1,    18,   171,   207,    -1,    20,   171,   207,    -1,
      13,   172,   207,    -1,    19,   173,   207,    -1,    25,   189,
     198,    -1,    26,   189,   198,    -1,    27,   207,   189,    -1,
      28,   189,    -1,    29,   207,    -1,    30,   189,   189,   189,
      -1,    31,   189,   189,    -1,    32,   189,   189,    -1,    33,
     193,    -1,    34,   189,   189,    -1,    35,   189,   189,    -1,
      36,   189,   189,    -1,    37,   189,   189,    -1,    38,   189,
     189,    -1,    39,   189,   189,    -1,    40,   189,   189,    -1,
      41,   189,   189,    -1,    42,   189,   189,    -1,    43,   189,
     189,    -1,    44,   189,   189,    -1,    45,   189,   189,    -1,
      46,   189,   189,    -1,    47,   189,   189,    -1,    48,   189,
     189,    -1,    49,   189,   189,    -1,    50,   189,   189,    -1,
      51,   189,    -1,    52,   189,    -1,    53,   189,    -1,    54,
     189,    -1,    55,   189,    -1,    56,   189,    -1,    57,   189,
      -1,    58,   189,   189,    -1,    59,   189,   189,    -1,    60,
     189,   189,    -1,    61,   189,   189,    -1,    62,   189,   189,
      -1,    64,   189,   189,    -1,    63,   189,   189,    -1,    65,
     189,   189,    -1,    66,   189,   189,    -1,    67,   189,   189,
      -1,    68,   189,    -1,    69,   189,    -1,    70,   189,   193,
      -1,    71,   189,   193,    -1,    72,   173,   173,    -1,    73,
     226,    -1,    88,    -1,   118,    -1,   168,   189,   193,    -1,
      -1,   192,   189,    -1,   174,   192,   175,    -1,   174,   198,
     128,   207,   138,   170,   149,   207,   217,   175,    -1,   174,
     198,   128,   207,   125,   170,   170,   138,   170,   149,   207,
     217,   175,    -1,    -1,   195,   194,    -1,   145,    -1,   144,
      -1,   174,   116,   198,   128,   207,   247,   170,   217,   245,
     226,   215,   175,    -1,   174,   116,   198,   128,   207,   247,
     170,   217,   215,   175,    -1,   114,   207,    -1,   114,   173,
     207,    -1,    -1,   146,   173,    -1,   174,   148,   173,   175,
      -1,   189,   217,    -1,   203,   217,    -1,    -1,   202,   201,
      -1,   174,   202,   175,    -1,   165,    -1,    -1,   205,   204,
      -1,   174,   131,   173,   170,   196,   199,   211,   205,   133,
     170,   138,   170,   217,   175,    -1,   174,   170,   170,   175,
      -1,   174,   121,   198,   175,    -1,   174,   122,   189,   175,
      -1,   174,   123,   175,    -1,    -1,   209,   208,    -1,    76,
      -1,    77,    -1,    78,    -1,    -1,   147,   173,    -1,   174,
     120,   128,   207,   175,    -1,    -1,   213,   212,    -1,   198,
      -1,    -1,    82,   173,   170,   170,    -1,    85,   173,   193,
      -1,    -1,   217,   216,    -1,    -1,    83,   219,    -1,    -1,
     171,   219,    -1,    -1,    83,   171,    -1,    99,   215,   217,
     226,    -1,   130,   207,   223,    -1,    -1,   223,   207,    -1,
      -1,    80,   170,    -1,    -1,    84,   170,    -1,   174,   209,
     142,   207,   227,   217,   225,   215,   218,   175,    -1,   189,
      -1,   185,    -1,    90,   226,    91,   189,    -1,    91,   189,
      90,   226,    -1,    92,   190,   190,   190,    90,   226,    -1,
      93,   189,    94,   226,    -1,    93,   189,    94,   226,    95,
     226,    -1,    96,   189,   226,    -1,    97,    -1,    97,   189,
      -1,    98,   198,    -1,   221,    -1,   100,   170,    -1,   101,
     170,   170,    -1,   102,    70,   214,   103,   189,   104,   189,
     105,   189,   221,    -1,   106,    70,   214,   103,   189,   104,
     189,   105,   189,   221,    -1,   107,    70,   214,   103,   189,
     104,   189,   105,   189,   221,    -1,   108,    70,   214,   103,
     189,   104,   189,   105,   189,   221,    -1,   109,   189,   226,
      -1,   110,   226,    -1,   182,    -1,    86,    -1,    87,    -1,
      88,    -1,    -1,   228,   226,    -1,   174,   228,   175,    -1,
     153,    -1,   154,    -1,   162,    -1,   169,    -1,    -1,   231,
     230,    -1,   174,   132,   133,   170,   235,   175,    -1,   174,
     132,   163,   175,    -1,   174,   129,   163,   175,    -1,   174,
     129,   134,   207,   170,   175,    -1,   174,   129,   198,   207,
     186,   138,   170,   217,   175,    -1,   174,   129,   198,   207,
     186,   222,   138,   170,   217,   175,    -1,   164,    -1,    -1,
     235,   234,    -1,   174,    74,   237,   175,    -1,   174,    75,
     198,   241,   178,   152,   170,   207,   217,   215,   175,    -1,
     174,   128,   242,   175,    -1,   174,    76,   198,   195,   231,
     217,   215,   175,    -1,   174,    77,   198,   195,   231,   217,
     215,   175,    -1,   174,    78,   198,   188,   217,   215,   175,
      -1,    -1,   238,   242,    -1,   174,   238,   175,    -1,     3,
      -1,     4,    -1,   118,    -1,   155,    -1,   123,    -1,   139,
      -1,   162,    -1,    -1,   241,   240,    -1,   174,   142,   207,
     241,   184,   224,   178,   152,   170,   217,   215,   175,    -1,
     174,   210,   198,   241,   128,   207,   224,   178,   152,   170,
     217,   215,   175,    -1,   174,   142,   207,   241,   115,   128,
     207,   143,   190,   224,   178,   152,   170,   217,   215,   175,
      -1,   174,   142,   207,   241,   116,   173,    97,   128,   207,
     213,   224,   178,   152,   170,   217,   215,   175,    -1,   174,
     142,   207,   241,   117,   128,   207,   224,   178,   152,   170,
     217,   215,   175,    -1,   174,   142,   207,   241,    75,   128,
     207,   178,   152,   170,   217,   215,   175,    -1,   174,   198,
     128,   207,   247,   217,   215,   175,    -1,   174,   198,   128,
     207,   247,   203,   217,   215,   175,    -1,    -1,   244,   243,
      -1,   174,   244,   175,    -1,   157,    -1,   156,    -1,     8,
      -1,     5,    -1,     6,    -1,     7,    -1,     9,    -1,   119,
      -1,   155,    -1,    22,    -1,    23,    -1,    24,    -1,   158,
      -1,   159,    -1,   160,    -1,   161,    -1,   166,    -1,   167,
      -1,    -1,   247,   246,    -1
};

/* YYRLINE[YYN] -- source line where rule number YYN was defined.  */
static const unsigned short yyrline[] =
{
       0,   248,   248,   253,   261,   269,   277,   285,   291,   299,
     307,   319,   322,   329,   332,   338,   366,   369,   375,   382,
     392,   396,   400,   404,   408,   412,   416,   420,   424,   428,
     432,   436,   443,   446,   452,   465,   469,   473,   477,   481,
     485,   489,   493,   497,   501,   505,   509,   513,   517,   521,
     527,   534,   543,   546,   552,   568,   572,   578,   585,   592,
     599,   607,   614,   621,   628,   635,   643,   651,   659,   666,
     674,   682,   690,   697,   704,   710,   717,   724,   731,   738,
     745,   752,   759,   766,   773,   780,   787,   794,   801,   808,
     815,   822,   829,   835,   841,   847,   853,   859,   865,   871,
     878,   885,   892,   899,   906,   913,   920,   927,   934,   941,
     947,   953,   960,   967,  1019,  1025,  1030,  1035,  1045,  1048,
    1054,  1076,  1088,  1105,  1108,  1114,  1118,  1124,  1145,  1166,
    1171,  1181,  1184,  1191,  1198,  1207,  1219,  1222,  1228,  1235,
    1242,  1245,  1251,  1276,  1283,  1290,  1296,  1304,  1307,  1313,
    1317,  1321,  1328,  1331,  1338,  1346,  1349,  1355,  1367,  1370,
    1380,  1390,  1393,  1400,  1403,  1410,  1413,  1424,  1427,  1433,
    1449,  1458,  1461,  1470,  1473,  1480,  1483,  1489,  1519,  1525,
    1529,  1538,  1547,  1558,  1566,  1575,  1585,  1590,  1596,  1603,
    1609,  1616,  1624,  1637,  1649,  1662,  1675,  1683,  1690,  1696,
    1701,  1706,  1714,  1717,  1723,  1730,  1734,  1738,  1742,  1749,
    1752,  1758,  1766,  1772,  1776,  1783,  1796,  1813,  1820,  1823,
    1829,  1835,  1861,  1867,  1889,  1911,  1934,  1937,  1943,  1949,
    1953,  1957,  1961,  1965,  1969,  1973,  1980,  1983,  1989,  2009,
    2031,  2053,  2077,  2098,  2120,  2137,  2159,  2162,  2168,  2174,
    2178,  2182,  2186,  2190,  2194,  2198,  2202,  2206,  2210,  2214,
    2218,  2222,  2226,  2230,  2234,  2238,  2242,  2249,  2252
};
#endif

#if YYDEBUG || YYERROR_VERBOSE
/* YYTNME[SYMBOL-NUM] -- String name of the symbol SYMBOL-NUM.
   First, the terminals, then, starting at YYNTOKENS, nonterminals. */
static const char *const yytname[] =
{
  "$end", "error", "$undefined", "CONST", "VOLATILE", "AUTO", "STATIC",
  "EXTERN", "REGISTER", "GLOBAL", "SIGNED", "UNSIGNED", "VOID", "CHAR",
  "SHORT", "INT", "LONG", "LONGLONG", "FLOAT", "STRING", "DOUBLE",
  "LONGDOUBLE", "CDECL", "STDCALL", "FASTCALL", "DOT", "ARROW", "CAST",
  "EXPRSIZE", "TYPESIZE", "QUEST", "DISJ", "CONJ", "COMPEXPR", "ASSIGN",
  "OR", "XOR", "AND", "EQ", "NE", "LT", "LE", "GE", "GT", "RSHFT", "LSHFT",
  "ADD", "SUB", "MUL", "DIV", "MOD", "NEG", "NOT", "INV", "PREINC",
  "PREDEC", "POSTINC", "POSTDEC", "A_ADD", "A_SUB", "A_MUL", "A_DIV",
  "A_MOD", "A_RSHFT", "A_LSHFT", "A_AND", "A_OR", "A_XOR", "INDR", "ADDR",
  "INDEX", "CALL", "ASMOPRD", "STMTEXPR", "DEF", "TYPEDEF", "STRUCT",
  "UNION", "ENUM", "ERROR", "SIZE", "ALIGNMENT", "POS", "PROFILE",
  "SHADOW", "PRAGMA", "BREAK", "CONTINUE", "KW_NULL", "COMPSTMT", "DO",
  "WHILE", "FOR", "IF", "THEN", "ELSE", "SWITCH", "RETURN", "GOTO",
  "PSTMT", "ADVANCE", "AWAIT", "DOSERIAL", "INIT", "FINAL", "INC", "DOALL",
  "DOACROSS", "DOSUPER", "MUTEX", "COBEGIN", "ASM", "OPERANDS", "CLOBBERS",
  "ID", "ARRAY", "FUNCTION", "POINTER", "SYNC", "PARAMETER", "PARAM",
  "LABEL", "CASE", "DEFAULT", "VARARG", "BITFIELD", "VAR", "REAL", "TYPE",
  "SYM", "SCOPE", "IP_TABLE", "SYMBOL_TABLE", "NUM_ENTRIES", "BLOCK",
  "EXPR", "STMT", "FUNC", "OFFSET", "EXPLICIT_ALIGNMENT", "ENUMFIELD",
  "FIELD", "KEY", "DIM", "HEADER", "SOURCE", "IN", "OUT", "INCLUDE",
  "PARENT", "GVAR", "LTYPE", "REFS", "EMPTY", "INCOMPLETE", "IMPLICIT",
  "COMMON", "DEFINED", "WEAK", "COMDAT", "CONSTRUCTOR", "DESTRUCTOR",
  "UNNAMED", "END", "LINKED", "NOT_AVAIL", "ELLIPSIS", "OLD_PARAM", "PHI",
  "LINKMULTI", "I_LIT", "F_LIT", "C_LIT", "ST_LIT", "'('", "')'",
  "$accept", "dcl", "alignment", "asm_clobbers", "asm_dcl", "asm_operands",
  "asm_stmt", "basic_type", "basic_type_list", "comp_stmt", "entry_type",
  "enum_field", "enum_field_list", "expr", "expr_container", "expr_core",
  "expr_list", "expr_list_container", "field", "field_list", "file_type",
  "func_dcl", "identifier", "in_name", "include", "initializer",
  "initializer_list", "initializer_list_container", "ipste_flags",
  "ipste_flags_list", "ip_sym_tab_ent", "key", "label", "label_list",
  "named_basic_type", "out_name", "param", "param_list", "parloop_index",
  "position", "pragma", "pragma_list", "prof_st", "prof_st_list",
  "profile", "pstmt", "scope", "scope_entry", "size", "shadow", "stmt",
  "stmt_core", "stmt_list", "stmt_list_container", "struct_qual",
  "struct_qual_list", "symbol_table", "symbol_table_entry",
  "symbol_table_flags", "symbol_table_flags_list", "type_dcl",
  "type_definition", "type_list", "type_list_container", "type_qual",
  "type_qual_list", "type_spec", "var_dcl", "var_dcl_list",
  "var_dcl_list_container", "var_qual", "var_qual_list", 0
};
#endif

# ifdef YYPRINT
/* YYTOKNUM[YYLEX-NUM] -- Internal token number corresponding to
   token YYLEX-NUM.  */
static const unsigned short yytoknum[] =
{
       0,   256,   257,   258,   259,   260,   261,   262,   263,   264,
     265,   266,   267,   268,   269,   270,   271,   272,   273,   274,
     275,   276,   277,   278,   279,   280,   281,   282,   283,   284,
     285,   286,   287,   288,   289,   290,   291,   292,   293,   294,
     295,   296,   297,   298,   299,   300,   301,   302,   303,   304,
     305,   306,   307,   308,   309,   310,   311,   312,   313,   314,
     315,   316,   317,   318,   319,   320,   321,   322,   323,   324,
     325,   326,   327,   328,   329,   330,   331,   332,   333,   334,
     335,   336,   337,   338,   339,   340,   341,   342,   343,   344,
     345,   346,   347,   348,   349,   350,   351,   352,   353,   354,
     355,   356,   357,   358,   359,   360,   361,   362,   363,   364,
     365,   366,   367,   368,   369,   370,   371,   372,   373,   374,
     375,   376,   377,   378,   379,   380,   381,   382,   383,   384,
     385,   386,   387,   388,   389,   390,   391,   392,   393,   394,
     395,   396,   397,   398,   399,   400,   401,   402,   403,   404,
     405,   406,   407,   408,   409,   410,   411,   412,   413,   414,
     415,   416,   417,   418,   419,   420,   421,   422,   423,   424,
     425,   426,   427,   428,    40,    41
};
# endif

/* YYR1[YYN] -- Symbol number of symbol that rule YYN derives.  */
static const unsigned char yyr1[] =
{
       0,   176,   177,   177,   177,   177,   177,   177,   177,   177,
     177,   178,   178,   179,   179,   180,   181,   181,   182,   182,
     183,   183,   183,   183,   183,   183,   183,   183,   183,   183,
     183,   183,   184,   184,   185,   186,   186,   186,   186,   186,
     186,   186,   186,   186,   186,   186,   186,   186,   186,   186,
     187,   187,   188,   188,   189,   190,   190,   191,   191,   191,
     191,   191,   191,   191,   191,   191,   191,   191,   191,   191,
     191,   191,   191,   191,   191,   191,   191,   191,   191,   191,
     191,   191,   191,   191,   191,   191,   191,   191,   191,   191,
     191,   191,   191,   191,   191,   191,   191,   191,   191,   191,
     191,   191,   191,   191,   191,   191,   191,   191,   191,   191,
     191,   191,   191,   191,   191,   191,   191,   191,   192,   192,
     193,   194,   194,   195,   195,   196,   196,   197,   197,   198,
     198,   199,   199,   200,   201,   201,   202,   202,   203,   204,
     205,   205,   206,   207,   208,   208,   208,   209,   209,   210,
     210,   210,   211,   211,   212,   213,   213,   214,   215,   215,
     216,   217,   217,   218,   218,   219,   219,   220,   220,   221,
     222,   223,   223,   224,   224,   225,   225,   226,   227,   227,
     227,   227,   227,   227,   227,   227,   227,   227,   227,   227,
     227,   227,   227,   227,   227,   227,   227,   227,   227,   227,
     227,   227,   228,   228,   229,   230,   230,   230,   230,   231,
     231,   232,   232,   233,   233,   233,   233,   234,   235,   235,
     236,   237,   237,   237,   237,   237,   238,   238,   239,   240,
     240,   240,   240,   240,   240,   240,   241,   241,   242,   242,
     242,   242,   242,   242,   243,   243,   244,   244,   245,   246,
     246,   246,   246,   246,   246,   246,   246,   246,   246,   246,
     246,   246,   246,   246,   246,   246,   246,   247,   247
};

/* YYR2[YYN] -- Number of symbols composing right hand side of rule YYN.  */
static const unsigned char yyr2[] =
{
       0,     2,     0,     1,     1,     1,     1,     1,     1,     1,
       1,     0,     2,     0,     2,     6,     0,     2,     4,     5,
       1,     1,     1,     1,     1,     1,     1,     1,     1,     1,
       1,     1,     0,     2,     5,     1,     1,     1,     1,     1,
       1,     1,     1,     1,     1,     1,     1,     1,     1,     1,
       4,     3,     0,     2,     6,     2,     3,     2,     3,     4,
       4,     3,     3,     3,     3,     3,     3,     3,     3,     2,
       2,     4,     3,     3,     2,     3,     3,     3,     3,     3,
       3,     3,     3,     3,     3,     3,     3,     3,     3,     3,
       3,     3,     2,     2,     2,     2,     2,     2,     2,     3,
       3,     3,     3,     3,     3,     3,     3,     3,     3,     2,
       2,     3,     3,     3,     2,     1,     1,     3,     0,     2,
       3,    10,    13,     0,     2,     1,     1,    12,    10,     2,
       3,     0,     2,     4,     2,     2,     0,     2,     3,     1,
       0,     2,    14,     4,     4,     4,     3,     0,     2,     1,
       1,     1,     0,     2,     5,     0,     2,     1,     0,     4,
       3,     0,     2,     0,     2,     0,     2,     0,     2,     4,
       3,     0,     2,     0,     2,     0,     2,    10,     1,     1,
       4,     4,     6,     4,     6,     3,     1,     2,     2,     1,
       2,     3,    10,    10,    10,    10,     3,     2,     1,     1,
       1,     1,     0,     2,     3,     1,     1,     1,     1,     0,
       2,     6,     4,     4,     6,     9,    10,     1,     0,     2,
       4,    11,     4,     8,     8,     7,     0,     2,     3,     1,
       1,     1,     1,     1,     1,     1,     0,     2,    12,    13,
      16,    17,    14,    13,     8,     9,     0,     2,     3,     1,
       1,     1,     1,     1,     1,     1,     1,     1,     1,     1,
       1,     1,     1,     1,     1,     1,     1,     0,     2
};

/* YYDEFACT[STATE-NAME] -- Default rule to reduce with in state
   STATE-NUM when YYTABLE doesn't specify something else to do.  Zero
   means the default is an error.  */
static const unsigned short yydefact[] =
{
       2,     0,     0,     5,     3,     6,     9,     8,    10,     7,
       4,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     1,     0,     0,     0,     0,    16,     0,     0,   129,
       0,     0,     0,     0,     0,     0,     0,     0,   161,     0,
       0,     0,     0,     0,     0,   220,    16,     0,     0,    13,
     130,     0,     0,     0,   213,     0,     0,   218,   212,   133,
     158,   267,   236,   123,   123,    52,     0,     0,    13,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,   115,   116,     0,     0,     0,
     161,   118,    17,     0,    18,     0,   267,     0,    40,    41,
      42,    43,    48,    38,    37,    49,    45,    44,    35,    47,
      46,    39,    36,     0,   126,   125,   131,     0,     0,     0,
       0,   162,   161,    11,   209,   209,   161,   149,   150,   151,
       0,     0,   222,    19,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,    69,    70,     0,     0,     0,    74,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,    92,    93,    94,
      95,    96,    97,    98,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,   109,   110,     0,     0,     0,   147,
     114,    57,     0,     0,   167,     0,    14,   143,     0,   214,
       0,     0,     0,     0,   152,   217,   211,   219,     0,     0,
      15,   252,   253,   254,   251,   255,   258,   259,   260,   256,
     257,   250,   249,   261,   262,   263,   264,   265,   266,   136,
     161,   158,   268,   229,   230,     0,   231,   233,   234,   232,
     235,     0,   237,     0,   124,   161,   161,     0,    53,   158,
     236,   236,     0,     0,    64,    58,    62,    65,    63,    66,
      67,    68,     0,    72,    73,    75,    76,    77,    78,    79,
      80,    81,    82,    83,    84,    85,    86,    87,    88,    89,
      90,    91,    99,   100,   101,   102,   103,   105,   104,   106,
     107,   108,   111,   112,   113,     0,    61,   117,     0,     0,
     120,   119,   161,   171,   161,     0,   132,     0,   140,     0,
     160,     0,   158,     0,    12,     0,     0,   205,   206,   207,
     208,   158,   210,   158,     0,     0,    32,     0,    59,    60,
      71,     0,     0,   148,   168,    54,   158,   170,     0,   161,
     153,     0,   159,   136,   138,   161,   137,   161,     0,   244,
       0,     0,     0,     0,    51,     0,   225,     0,     0,     0,
       0,   173,     0,     0,     0,     0,     0,   246,     0,     0,
     172,   215,     0,     0,   139,   141,   134,   135,   245,   161,
       0,   223,   224,    50,     0,     0,     0,     0,    29,    20,
      21,    22,    23,    24,    25,    26,    27,    28,     0,    30,
      31,    33,    11,   173,   199,   200,   201,     0,     0,     0,
       0,     0,     0,   186,     0,   158,     0,     0,     0,     0,
       0,     0,     0,     0,   198,   179,   178,   189,   161,     0,
       0,   146,     0,   128,   158,   216,     0,   158,     0,     0,
      11,     0,     0,   173,   174,     0,    11,     0,     0,     0,
       0,     0,     0,     0,   187,   188,   161,   190,     0,     0,
       0,     0,     0,     0,   197,   175,   144,   145,     0,   248,
     247,     0,     0,     0,     0,     0,     0,     0,     0,    11,
       0,     0,   226,     0,     0,     0,    55,     0,     0,     0,
     185,     0,   191,   157,     0,     0,     0,     0,   196,     0,
     158,   127,   161,   221,     0,     0,     0,   173,   155,     0,
     161,     0,     0,     0,   180,   181,    56,     0,   183,   169,
       0,     0,     0,     0,   176,   163,     0,     0,   161,   161,
      11,   173,     0,   158,   161,   228,   227,   202,    34,     0,
       0,     0,     0,     0,     0,   165,     0,   142,     0,     0,
     158,     0,     0,   156,    11,   161,     0,   158,     0,   182,
     184,     0,     0,     0,     0,   165,   164,   177,     0,   121,
       0,     0,     0,     0,   158,   238,     0,   204,   203,     0,
       0,     0,     0,   166,   161,   243,   161,     0,     0,     0,
     239,     0,     0,     0,     0,     0,   158,     0,   161,   242,
       0,     0,     0,     0,   122,     0,   154,   158,   192,   193,
     194,   195,   240,     0,   241
};

/* YYDEFGOTO[NTERM-NUM]. */
static const short yydefgoto[] =
{
      -1,     2,   281,   134,     3,    49,    19,   441,   401,   465,
     153,   288,   166,    26,   491,   130,   235,   132,   284,   164,
     156,     4,   533,   244,     5,   386,   351,   270,   415,   381,
       6,    29,   373,   335,   171,   348,   593,   571,   534,   160,
     161,    60,   586,   606,   339,   467,   242,   377,   442,   540,
     230,   468,   598,   578,   362,   285,     7,     8,   247,   157,
       9,    23,   552,   523,   282,   163,    67,    10,   472,   409,
     272,   162
};

/* YYPACT[STATE-NUM] -- Index in YYTABLE of the portion describing
   STATE-NUM.  */
#define YYPACT_NINF -597
static const short yypact[] =
{
    -169,   159,    17,  -597,  -597,  -597,  -597,  -597,  -597,  -597,
    -597,  -144,     9,  -125,    12,  -113,  -119,  -129,  -117,  -123,
     -68,  -597,    59,   -46,   -43,   -37,    34,  -123,   -13,  -597,
      33,  -123,   -10,  -123,    40,    43,    25,    27,  -597,  -123,
      12,    12,    12,    12,    41,  -597,    34,   683,    42,   108,
    -597,    52,  -123,    55,  -597,   523,   -27,  -597,  -597,  -597,
     -50,  -597,  -597,  -597,  -597,  -597,   -40,    51,   108,   215,
     217,    57,    64,    65,    62,    67,   -43,   -43,  -123,   -43,
    -123,   -43,   -43,   -43,    42,   -43,   -43,   -43,   -43,   -43,
     -43,   -43,   -43,   -43,   -43,   -43,   -43,   -43,   -43,   -43,
     -43,   -43,   -43,   -43,   -43,   -43,   -43,   -43,   -43,   -43,
     -43,   -43,   -43,   -43,   -43,   -43,   -43,   -43,   -43,   -43,
     -43,   -43,   -43,    69,    63,  -597,  -597,    12,    68,   -43,
    -597,  -597,  -597,    42,  -597,    72,  -597,    73,  -597,  -597,
    -597,  -597,  -597,  -597,  -597,  -597,  -597,  -597,  -597,  -597,
    -597,  -597,  -597,   -97,  -597,  -597,    97,  -156,    71,    78,
      79,  -597,   399,   216,    81,    81,    87,  -597,  -597,  -597,
    -123,    12,  -597,  -597,    83,    92,  -123,  -123,  -123,  -123,
    -123,    12,    12,   -43,  -597,  -597,   -43,   -43,   -43,  -597,
     -43,   -43,   -43,   -43,   -43,   -43,   -43,   -43,   -43,   -43,
     -43,   -43,   -43,   -43,   -43,   -43,   -43,  -597,  -597,  -597,
    -597,  -597,  -597,  -597,   -43,   -43,   -43,   -43,   -43,   -43,
     -43,   -43,   -43,   -43,  -597,  -597,    42,    42,    98,  -597,
    -597,  -597,  -123,    42,   -57,   -54,  -597,  -597,   471,  -597,
    -123,   104,   138,   106,   130,  -597,  -597,  -597,   110,    42,
    -597,  -597,  -597,  -597,  -597,  -597,  -597,  -597,  -597,  -597,
    -597,  -597,  -597,  -597,  -597,  -597,  -597,  -597,  -597,  -597,
    -597,   -50,  -597,  -597,  -597,   111,  -597,  -597,  -597,  -597,
    -597,   131,  -597,    12,  -597,  -138,  -138,    12,  -597,   -50,
    -597,  -597,  -123,  -123,  -597,  -597,  -597,  -597,  -597,  -597,
    -597,  -597,   -43,  -597,  -597,  -597,  -597,  -597,  -597,  -597,
    -597,  -597,  -597,  -597,  -597,  -597,  -597,  -597,  -597,  -597,
    -597,  -597,  -597,  -597,  -597,  -597,  -597,  -597,  -597,  -597,
    -597,  -597,  -597,  -597,  -597,  -135,  -597,  -597,   114,   107,
    -597,  -597,  -597,  -597,  -597,   116,  -597,   119,  -597,   117,
    -597,   -36,   -50,   118,  -597,   126,   170,  -597,  -597,  -597,
    -597,   -50,  -597,   -50,   -26,   124,   214,   220,  -597,  -597,
    -597,  -123,     2,  -597,  -597,  -597,   -73,  -123,   -71,  -597,
    -597,  -122,  -597,   -37,  -597,  -597,  -597,  -597,   125,  -597,
    -123,  -123,   127,   128,  -597,   129,  -597,   173,   180,   136,
     182,   372,  -123,   676,    12,   -43,   139,  -597,   140,    63,
    -597,  -597,   -67,   141,  -597,  -597,   228,   228,  -597,  -597,
    -115,  -597,  -597,  -597,  -123,  -123,   219,  -123,  -597,  -597,
    -597,  -597,  -597,  -597,  -597,  -597,  -597,  -597,   149,  -597,
    -597,  -597,   241,   244,  -597,  -597,  -597,   156,    63,   -43,
     161,   -43,   -43,   -43,    12,   251,   171,   172,   274,   275,
     276,   277,   -43,    63,  -597,  -597,  -597,  -597,  -597,   174,
     175,  -597,    10,  -597,   251,  -597,   213,   -50,   184,   187,
     241,   209,   232,   244,  -597,   210,   241,   189,   270,   278,
      14,   161,   271,    63,  -597,  -597,  -597,  -597,   197,    12,
      12,    12,    12,    63,  -597,    66,  -597,  -597,    12,  -597,
    -597,   195,   204,   202,   224,   246,   245,   161,  -123,   241,
     229,   249,  -597,   235,   -43,    63,  -597,   227,   161,    63,
    -597,   -58,  -597,  -597,   300,   307,   308,   309,  -597,   243,
     251,  -597,  -597,  -597,   279,  -123,   248,   244,  -597,   262,
    -597,   250,    30,   242,  -597,  -597,  -597,   325,   324,  -597,
     -43,   -43,   -43,   -43,  -597,   343,   -65,   260,  -597,  -597,
     241,   -74,   261,   -50,  -597,  -597,  -597,  -597,  -597,    63,
      63,   328,   329,   330,   331,   265,   264,  -597,   288,   -63,
     -50,   290,   320,  -597,   241,  -597,   268,   -50,    32,  -597,
    -597,   -43,   -43,   -43,   -43,   265,  -597,  -597,  -123,  -597,
     269,   280,   317,   294,   -50,  -597,   281,  -597,  -597,   344,
     348,   349,   350,  -597,  -597,  -597,  -597,  -123,   289,   283,
    -597,   -43,   -43,   -43,   -43,   -60,   -50,   285,  -597,  -597,
     362,   362,   362,   362,  -597,   287,  -597,   -50,  -597,  -597,
    -597,  -597,  -597,   292,  -597
};

/* YYPGOTO[NTERM-NUM].  */
static const short yypgoto[] =
{
    -597,  -597,  -214,   395,  -597,   418,    70,  -597,  -597,  -597,
    -597,  -597,  -597,   -24,  -488,  -597,  -597,    19,  -597,   404,
    -597,  -597,   113,  -597,  -597,  -597,  -597,   121,  -597,  -597,
    -597,    80,  -597,  -597,  -597,  -597,  -597,  -597,  -358,  -149,
    -597,   -21,  -597,  -136,  -597,  -596,  -597,  -597,  -441,  -597,
    -262,  -597,  -597,  -597,  -597,   316,  -597,  -597,  -597,  -597,
    -597,  -597,  -597,  -597,  -597,   -82,   -78,    11,  -597,   -39,
    -597,   352
};

/* YYTABLE[YYPACT[STATE-NUM]].  What to do in state STATE-NUM.  If
   positive, shift that token.  If negative, reduce the rule which
   number is the opposite.  If zero, do what YYDEFACT says.
   If YYTABLE_NINF, syntax error.  */
#define YYTABLE_NINF -1
static const unsigned short yytable[] =
{
      46,    13,   486,   528,    35,     1,   438,   371,   245,   158,
     478,   413,   159,    24,   159,   357,   358,    21,   159,   246,
     159,    31,   159,   479,   359,   159,   338,   159,   159,   547,
      22,   360,   158,   240,    36,   159,   167,   168,   169,   372,
     557,   241,   519,   414,   648,   649,   650,   651,    27,    28,
      32,    28,   181,   182,    34,   184,    37,   186,   187,   188,
      39,   190,   191,   192,   193,   194,   195,   196,   197,   198,
     199,   200,   201,   202,   203,   204,   205,   206,   207,   208,
     209,   210,   211,   212,   213,   214,   215,   216,   217,   218,
     219,   220,   221,   222,   223,   224,   225,   226,   227,    38,
     592,   407,   170,   189,   411,   233,   570,    50,   475,   234,
     587,    53,   609,    55,    20,   644,   229,   154,   155,    61,
      25,   340,   353,   404,   405,   406,    13,    30,    33,    45,
     594,    25,   136,    47,    40,    41,    42,    43,   383,   384,
     365,   271,   535,   536,   537,   289,    48,   474,    25,   394,
     539,   159,   236,    62,    63,    64,    65,    51,   183,   301,
     185,    52,   302,   303,   304,    54,   305,   306,   307,   308,
     309,   310,   311,   312,   313,   314,   315,   316,   317,   318,
     319,   320,   321,    25,   508,   509,   488,    44,    25,   526,
     322,   323,   324,   325,   326,   327,   328,   329,   330,   331,
      58,   504,    59,   388,    66,   575,   229,   617,   366,   367,
      56,   341,   392,    57,   393,    66,   131,   273,   274,   273,
     274,   133,   135,   273,   274,   137,   172,   408,   485,   176,
     174,   530,   175,    11,   177,   179,   178,   229,   180,   232,
     231,   538,   228,   243,   248,   332,   333,   237,   239,   352,
     290,   249,   337,   292,   250,   283,   294,   295,   296,   297,
     298,   287,   293,   555,   361,   363,   516,   558,   350,   559,
      12,   334,   521,    13,   344,    14,   345,   347,   370,   346,
     349,   354,   375,   355,   291,   374,   379,   382,    15,   397,
      16,    17,   380,   389,   299,   300,   390,   275,   391,   396,
     418,   424,   421,   422,   423,   549,   496,    18,   425,   426,
     427,   476,   336,   159,   471,   473,   482,   599,   600,   484,
     343,   376,   275,   378,   438,   511,   487,   385,   513,   398,
     399,   400,   276,   158,   276,   490,   618,   277,   276,   277,
     395,   497,   498,   277,   499,   500,   501,   502,   402,   506,
     507,   512,   517,   278,   514,   278,   591,   515,   412,   278,
     518,   524,   520,   522,   416,   529,   417,   532,   525,   279,
     541,   279,   368,   369,   542,   279,   280,   543,   280,   466,
     613,   470,   280,   428,   429,   430,   431,   432,   433,   434,
     435,   565,   436,   437,   544,   545,   356,   546,   477,   550,
     364,   551,   556,   560,   251,   252,   253,   254,   255,   407,
     561,   562,   563,   564,   572,   579,   577,   567,   569,   580,
     574,   256,   257,   258,   596,   489,   585,   492,   493,   494,
     588,   595,   601,   602,   603,   604,   605,   608,   503,   607,
     612,   610,   611,   615,   625,   627,   628,   505,   616,   631,
     626,   403,   438,   632,   633,   634,   630,   410,   639,   638,
     646,   455,   652,   173,    68,   629,   527,   654,   165,   623,
     419,   420,   387,   464,   576,   531,   251,   252,   253,   254,
     255,   286,   443,   510,   553,     0,     0,   645,   238,     0,
       0,     0,     0,   256,   257,   258,   439,   440,   653,     0,
     554,     0,     0,     0,   480,   481,     0,   483,     0,     0,
       0,     0,     0,     0,     0,     0,     0,   469,   259,     0,
       0,   566,     0,     0,     0,     0,     0,     0,     0,   573,
       0,     0,     0,     0,     0,     0,   581,   582,   583,   584,
       0,     0,     0,     0,     0,     0,     0,   589,   590,     0,
       0,     0,     0,   597,   260,   261,   262,   263,   264,   265,
     266,     0,     0,     0,     0,   267,   268,   495,     0,     0,
       0,     0,     0,   269,   614,     0,     0,   619,   620,   621,
     622,     0,     0,     0,     0,     0,     0,     0,     0,     0,
     259,     0,     0,     0,     0,     0,     0,     0,   548,   138,
     139,   140,     0,   635,     0,   636,     0,   640,   641,   642,
     643,     0,     0,     0,     0,     0,     0,   647,     0,     0,
       0,    20,     0,     0,     0,   568,   260,   261,   262,   263,
     264,   265,   266,     0,   141,     0,     0,   267,   268,     0,
       0,   342,     0,     0,   142,     0,     0,     0,     0,   143,
       0,   144,     0,   145,     0,     0,     0,     0,   146,   147,
     148,     0,     0,   149,   150,     0,     0,     0,     0,     0,
       0,     0,     0,   151,   152,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,   624,     0,
       0,     0,     0,    69,    70,     0,    71,     0,    72,     0,
       0,    73,    74,    75,     0,     0,     0,   637,    76,    77,
      78,    79,    80,    81,    82,    83,    84,    85,    86,    87,
      88,    89,    90,    91,    92,    93,    94,    95,    96,    97,
      98,    99,   100,   101,   102,   103,   104,   105,   106,   107,
     108,   109,   110,   111,   112,   113,   114,   115,   116,   117,
     118,   119,   120,   121,   122,   123,   124,     0,     0,     0,
       0,     0,   444,   445,   446,   447,   448,   449,   450,   451,
       0,   125,   452,   453,   454,   455,   456,   457,   458,     0,
       0,     0,   459,   460,   461,   462,   463,    12,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,   126,     0,     0,     0,     0,     0,     0,     0,   127,
     128,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
      25,   129
};

static const short yycheck[] =
{
      24,   114,   443,   491,   133,   174,    80,   142,   164,    82,
     125,   133,    85,     4,    85,   153,   154,     0,    85,   175,
      85,   134,    85,   138,   162,    85,    83,    85,    85,   517,
     174,   169,    82,   130,   163,    85,    76,    77,    78,   174,
     528,   138,   483,   165,   640,   641,   642,   643,   173,   174,
     163,   174,    76,    77,   173,    79,   173,    81,    82,    83,
     128,    85,    86,    87,    88,    89,    90,    91,    92,    93,
      94,    95,    96,    97,    98,    99,   100,   101,   102,   103,
     104,   105,   106,   107,   108,   109,   110,   111,   112,   113,
     114,   115,   116,   117,   118,   119,   120,   121,   122,    19,
     174,   174,   142,    84,   175,   129,   547,    27,   175,   130,
     175,    31,   175,    33,     1,   175,   174,   144,   145,    39,
     174,   175,   271,   121,   122,   123,   114,    14,    15,   175,
     571,   174,    52,   170,    75,    76,    77,    78,   174,   175,
     289,   162,   500,   501,   502,   166,   112,   409,   174,   175,
      84,    85,   133,    40,    41,    42,    43,   170,    78,   183,
      80,   128,   186,   187,   188,   175,   190,   191,   192,   193,
     194,   195,   196,   197,   198,   199,   200,   201,   202,   203,
     204,   205,   206,   174,   174,   175,   448,   128,   174,   175,
     214,   215,   216,   217,   218,   219,   220,   221,   222,   223,
     175,   463,   175,   352,   174,   175,   174,   175,   290,   291,
     170,   235,   361,   170,   363,   174,   174,     3,     4,     3,
       4,   113,   170,     3,     4,   170,   175,   376,   442,   172,
      15,   493,    15,    74,   170,   173,   171,   174,   171,   171,
     127,   503,   173,   146,   173,   226,   227,   175,   175,   270,
     170,   173,   233,   170,   175,   174,   176,   177,   178,   179,
     180,   174,   170,   525,   285,   286,   480,   529,   249,   531,
     111,   173,   486,   114,   170,   116,   138,   147,   302,   173,
     170,   170,   175,   152,   171,   171,   170,   170,   129,    75,
     131,   132,   173,   175,   181,   182,   170,    81,   128,   175,
     175,   128,   175,   175,   175,   519,   455,   148,   128,   173,
     128,   170,   232,    85,   175,   175,    97,   579,   580,   170,
     240,   342,    81,   344,    80,   474,   170,   351,   477,   115,
     116,   117,   118,    82,   118,   174,   598,   123,   118,   123,
     364,   170,   170,   123,    70,    70,    70,    70,   128,   175,
     175,   138,   143,   139,   170,   139,   570,   170,   379,   139,
     128,    91,   152,   174,   385,    94,   387,   170,    90,   155,
     175,   155,   292,   293,   170,   155,   162,   175,   162,   403,
     594,   405,   162,    11,    12,    13,    14,    15,    16,    17,
      18,   540,    20,    21,   170,   149,   283,   152,   419,   170,
     287,   152,   175,   103,     5,     6,     7,     8,     9,   174,
     103,   103,   103,   170,   152,    90,   174,   138,   170,    95,
     170,    22,    23,    24,   573,   449,    83,   451,   452,   453,
     170,   170,   104,   104,   104,   104,   171,   149,   462,   175,
     120,   590,   152,   175,   175,   128,   152,   468,   597,   105,
     170,   371,    80,   105,   105,   105,   175,   377,   175,   170,
     175,    99,   175,    68,    46,   614,   490,   175,    64,   605,
     390,   391,   351,   403,   552,   496,     5,     6,     7,     8,
       9,   165,   402,   472,   523,    -1,    -1,   636,   136,    -1,
      -1,    -1,    -1,    22,    23,    24,   124,   125,   647,    -1,
     524,    -1,    -1,    -1,   424,   425,    -1,   427,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,   404,   119,    -1,
      -1,   542,    -1,    -1,    -1,    -1,    -1,    -1,    -1,   550,
      -1,    -1,    -1,    -1,    -1,    -1,   560,   561,   562,   563,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,   568,   569,    -1,
      -1,    -1,    -1,   574,   155,   156,   157,   158,   159,   160,
     161,    -1,    -1,    -1,    -1,   166,   167,   454,    -1,    -1,
      -1,    -1,    -1,   174,   595,    -1,    -1,   601,   602,   603,
     604,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
     119,    -1,    -1,    -1,    -1,    -1,    -1,    -1,   518,    76,
      77,    78,    -1,   624,    -1,   626,    -1,   631,   632,   633,
     634,    -1,    -1,    -1,    -1,    -1,    -1,   638,    -1,    -1,
      -1,   508,    -1,    -1,    -1,   545,   155,   156,   157,   158,
     159,   160,   161,    -1,   111,    -1,    -1,   166,   167,    -1,
      -1,   170,    -1,    -1,   121,    -1,    -1,    -1,    -1,   126,
      -1,   128,    -1,   130,    -1,    -1,    -1,    -1,   135,   136,
     137,    -1,    -1,   140,   141,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,   150,   151,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,   608,    -1,
      -1,    -1,    -1,    10,    11,    -1,    13,    -1,    15,    -1,
      -1,    18,    19,    20,    -1,    -1,    -1,   627,    25,    26,
      27,    28,    29,    30,    31,    32,    33,    34,    35,    36,
      37,    38,    39,    40,    41,    42,    43,    44,    45,    46,
      47,    48,    49,    50,    51,    52,    53,    54,    55,    56,
      57,    58,    59,    60,    61,    62,    63,    64,    65,    66,
      67,    68,    69,    70,    71,    72,    73,    -1,    -1,    -1,
      -1,    -1,    86,    87,    88,    89,    90,    91,    92,    93,
      -1,    88,    96,    97,    98,    99,   100,   101,   102,    -1,
      -1,    -1,   106,   107,   108,   109,   110,   111,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,   118,    -1,    -1,    -1,    -1,    -1,    -1,    -1,   126,
     127,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
     174,   168
};

/* YYSTOS[STATE-NUM] -- The (internal number of the) accessing
   symbol of state STATE-NUM.  */
static const unsigned char yystos[] =
{
       0,   174,   177,   180,   197,   200,   206,   232,   233,   236,
     243,    74,   111,   114,   116,   129,   131,   132,   148,   182,
     198,     0,   174,   237,     4,   174,   189,   173,   174,   207,
     198,   134,   163,   198,   173,   133,   163,   173,   207,   128,
      75,    76,    77,    78,   128,   175,   189,   170,   112,   181,
     207,   170,   128,   207,   175,   207,   170,   170,   175,   175,
     217,   207,   198,   198,   198,   198,   174,   242,   181,    10,
      11,    13,    15,    18,    19,    20,    25,    26,    27,    28,
      29,    30,    31,    32,    33,    34,    35,    36,    37,    38,
      39,    40,    41,    42,    43,    44,    45,    46,    47,    48,
      49,    50,    51,    52,    53,    54,    55,    56,    57,    58,
      59,    60,    61,    62,    63,    64,    65,    66,    67,    68,
      69,    70,    71,    72,    73,    88,   118,   126,   127,   168,
     191,   174,   193,   113,   179,   170,   207,   170,    76,    77,
      78,   111,   121,   126,   128,   130,   135,   136,   137,   140,
     141,   150,   151,   186,   144,   145,   196,   235,    82,    85,
     215,   216,   247,   241,   195,   195,   188,    76,    77,    78,
     142,   210,   175,   179,    15,    15,   172,   170,   171,   173,
     171,   189,   189,   207,   189,   207,   189,   189,   189,   193,
     189,   189,   189,   189,   189,   189,   189,   189,   189,   189,
     189,   189,   189,   189,   189,   189,   189,   189,   189,   189,
     189,   189,   189,   189,   189,   189,   189,   189,   189,   189,
     189,   189,   189,   189,   189,   189,   189,   189,   173,   174,
     226,   198,   171,   189,   217,   192,   193,   175,   247,   175,
     130,   138,   222,   146,   199,   164,   175,   234,   173,   173,
     175,     5,     6,     7,     8,     9,    22,    23,    24,   119,
     155,   156,   157,   158,   159,   160,   161,   166,   167,   174,
     203,   217,   246,     3,     4,    81,   118,   123,   139,   155,
     162,   178,   240,   174,   194,   231,   231,   174,   187,   217,
     207,   198,   170,   170,   207,   207,   207,   207,   207,   198,
     198,   189,   189,   189,   189,   189,   189,   189,   189,   189,
     189,   189,   189,   189,   189,   189,   189,   189,   189,   189,
     189,   189,   189,   189,   189,   189,   189,   189,   189,   189,
     189,   189,   193,   193,   173,   209,   207,   193,    83,   220,
     175,   189,   170,   207,   170,   138,   173,   147,   211,   170,
     193,   202,   217,   215,   170,   152,   198,   153,   154,   162,
     169,   217,   230,   217,   198,   215,   241,   241,   207,   207,
     189,   142,   174,   208,   171,   175,   217,   223,   217,   170,
     173,   205,   170,   174,   175,   189,   201,   203,   215,   175,
     170,   128,   215,   215,   175,   189,   175,    75,   115,   116,
     117,   184,   128,   207,   121,   122,   123,   174,   215,   245,
     207,   175,   217,   133,   165,   204,   217,   217,   175,   207,
     207,   175,   175,   175,   128,   128,   173,   128,    11,    12,
      13,    14,    15,    16,    17,    18,    20,    21,    80,   124,
     125,   183,   224,   207,    86,    87,    88,    89,    90,    91,
      92,    93,    96,    97,    98,    99,   100,   101,   102,   106,
     107,   108,   109,   110,   182,   185,   189,   221,   227,   198,
     189,   175,   244,   175,   226,   175,   170,   217,   125,   138,
     207,   207,    97,   207,   170,   178,   224,   170,   226,   189,
     174,   190,   189,   189,   189,   198,   215,   170,   170,    70,
      70,    70,    70,   189,   226,   217,   175,   175,   174,   175,
     243,   215,   138,   215,   170,   170,   178,   143,   128,   224,
     152,   178,   174,   239,    91,    90,   175,   189,   190,    94,
     226,   217,   170,   198,   214,   214,   214,   214,   226,    84,
     225,   175,   170,   175,   170,   149,   152,   190,   207,   178,
     170,   152,   238,   245,   189,   226,   175,   190,   226,   226,
     103,   103,   103,   103,   170,   215,   217,   138,   207,   170,
     224,   213,   152,   217,   170,   175,   242,   174,   229,    90,
      95,   189,   189,   189,   189,    83,   218,   175,   170,   217,
     217,   178,   174,   212,   224,   170,   215,   217,   228,   226,
     226,   104,   104,   104,   104,   171,   219,   175,   149,   175,
     215,   152,   120,   178,   217,   175,   215,   175,   226,   189,
     189,   189,   189,   219,   207,   175,   170,   128,   152,   215,
     175,   105,   105,   105,   105,   217,   217,   207,   170,   175,
     189,   189,   189,   189,   175,   215,   175,   217,   221,   221,
     221,   221,   175,   215,   175
};

#if ! defined (YYSIZE_T) && defined (__SIZE_TYPE__)
# define YYSIZE_T __SIZE_TYPE__
#endif
#if ! defined (YYSIZE_T) && defined (size_t)
# define YYSIZE_T size_t
#endif
#if ! defined (YYSIZE_T)
# if defined (__STDC__) || defined (__cplusplus)
#  include <stddef.h> /* INFRINGES ON USER NAME SPACE */
#  define YYSIZE_T size_t
# endif
#endif
#if ! defined (YYSIZE_T)
# define YYSIZE_T unsigned int
#endif

#define yyerrok		(yyerrstatus = 0)
#define yyclearin	(yychar = YYEMPTY)
#define YYEMPTY		(-2)
#define YYEOF		0

#define YYACCEPT	goto yyacceptlab
#define YYABORT		goto yyabortlab
#define YYERROR		goto yyerrorlab


/* Like YYERROR except do call yyerror.  This remains here temporarily
   to ease the transition to the new meaning of YYERROR, for GCC.
   Once GCC version 2 has supplanted version 1, this can go.  */

#define YYFAIL		goto yyerrlab

#define YYRECOVERING()  (!!yyerrstatus)

#define YYBACKUP(Token, Value)					\
do								\
  if (yychar == YYEMPTY && yylen == 1)				\
    {								\
      yychar = (Token);						\
      yylval = (Value);						\
      yytoken = YYTRANSLATE (yychar);				\
      YYPOPSTACK;						\
      goto yybackup;						\
    }								\
  else								\
    { 								\
      yyerror ("syntax error: cannot back up");\
      YYERROR;							\
    }								\
while (0)

#define YYTERROR	1
#define YYERRCODE	256

/* YYLLOC_DEFAULT -- Compute the default location (before the actions
   are run).  */

#ifndef YYLLOC_DEFAULT
# define YYLLOC_DEFAULT(Current, Rhs, N)		\
   ((Current).first_line   = (Rhs)[1].first_line,	\
    (Current).first_column = (Rhs)[1].first_column,	\
    (Current).last_line    = (Rhs)[N].last_line,	\
    (Current).last_column  = (Rhs)[N].last_column)
#endif

/* YYLEX -- calling `yylex' with the right arguments.  */

#ifdef YYLEX_PARAM
# define YYLEX yylex (YYLEX_PARAM)
#else
# define YYLEX yylex ()
#endif

/* Enable debugging if requested.  */
#if YYDEBUG

# ifndef YYFPRINTF
#  include <stdio.h> /* INFRINGES ON USER NAME SPACE */
#  define YYFPRINTF fprintf
# endif

# define YYDPRINTF(Args)			\
do {						\
  if (yydebug)					\
    YYFPRINTF Args;				\
} while (0)

# define YYDSYMPRINT(Args)			\
do {						\
  if (yydebug)					\
    yysymprint Args;				\
} while (0)

# define YYDSYMPRINTF(Title, Token, Value, Location)		\
do {								\
  if (yydebug)							\
    {								\
      YYFPRINTF (stderr, "%s ", Title);				\
      yysymprint (stderr, 					\
                  Token, Value);	\
      YYFPRINTF (stderr, "\n");					\
    }								\
} while (0)

/*------------------------------------------------------------------.
| yy_stack_print -- Print the state stack from its BOTTOM up to its |
| TOP (included).                                                   |
`------------------------------------------------------------------*/

#if defined (__STDC__) || defined (__cplusplus)
static void
yy_stack_print (short *bottom, short *top)
#else
static void
yy_stack_print (bottom, top)
    short *bottom;
    short *top;
#endif
{
  YYFPRINTF (stderr, "Stack now");
  for (/* Nothing. */; bottom <= top; ++bottom)
    YYFPRINTF (stderr, " %d", *bottom);
  YYFPRINTF (stderr, "\n");
}

# define YY_STACK_PRINT(Bottom, Top)				\
do {								\
  if (yydebug)							\
    yy_stack_print ((Bottom), (Top));				\
} while (0)


/*------------------------------------------------.
| Report that the YYRULE is going to be reduced.  |
`------------------------------------------------*/

#if defined (__STDC__) || defined (__cplusplus)
static void
yy_reduce_print (int yyrule)
#else
static void
yy_reduce_print (yyrule)
    int yyrule;
#endif
{
  int yyi;
  unsigned int yylno = yyrline[yyrule];
  YYFPRINTF (stderr, "Reducing stack by rule %d (line %u), ",
             yyrule - 1, yylno);
  /* Print the symbols being reduced, and their result.  */
  for (yyi = yyprhs[yyrule]; 0 <= yyrhs[yyi]; yyi++)
    YYFPRINTF (stderr, "%s ", yytname [yyrhs[yyi]]);
  YYFPRINTF (stderr, "-> %s\n", yytname [yyr1[yyrule]]);
}

# define YY_REDUCE_PRINT(Rule)		\
do {					\
  if (yydebug)				\
    yy_reduce_print (Rule);		\
} while (0)

/* Nonzero means print parse trace.  It is left uninitialized so that
   multiple parsers can coexist.  */
int yydebug;
#else /* !YYDEBUG */
# define YYDPRINTF(Args)
# define YYDSYMPRINT(Args)
# define YYDSYMPRINTF(Title, Token, Value, Location)
# define YY_STACK_PRINT(Bottom, Top)
# define YY_REDUCE_PRINT(Rule)
#endif /* !YYDEBUG */


/* YYINITDEPTH -- initial size of the parser's stacks.  */
#ifndef	YYINITDEPTH
# define YYINITDEPTH 200
#endif

/* YYMAXDEPTH -- maximum size the stacks can grow to (effective only
   if the built-in stack extension method is used).

   Do not make this value too large; the results are undefined if
   SIZE_MAX < YYSTACK_BYTES (YYMAXDEPTH)
   evaluated with infinite-precision integer arithmetic.  */

#if defined (YYMAXDEPTH) && YYMAXDEPTH == 0
# undef YYMAXDEPTH
#endif

#ifndef YYMAXDEPTH
# define YYMAXDEPTH 10000
#endif



#if YYERROR_VERBOSE

# ifndef yystrlen
#  if defined (__GLIBC__) && defined (_STRING_H)
#   define yystrlen strlen
#  else
/* Return the length of YYSTR.  */
static YYSIZE_T
#   if defined (__STDC__) || defined (__cplusplus)
yystrlen (const char *yystr)
#   else
yystrlen (yystr)
     const char *yystr;
#   endif
{
  register const char *yys = yystr;

  while (*yys++ != '\0')
    continue;

  return yys - yystr - 1;
}
#  endif
# endif

# ifndef yystpcpy
#  if defined (__GLIBC__) && defined (_STRING_H) && defined (_GNU_SOURCE)
#   define yystpcpy stpcpy
#  else
/* Copy YYSRC to YYDEST, returning the address of the terminating '\0' in
   YYDEST.  */
static char *
#   if defined (__STDC__) || defined (__cplusplus)
yystpcpy (char *yydest, const char *yysrc)
#   else
yystpcpy (yydest, yysrc)
     char *yydest;
     const char *yysrc;
#   endif
{
  register char *yyd = yydest;
  register const char *yys = yysrc;

  while ((*yyd++ = *yys++) != '\0')
    continue;

  return yyd - 1;
}
#  endif
# endif

#endif /* !YYERROR_VERBOSE */



#if YYDEBUG
/*--------------------------------.
| Print this symbol on YYOUTPUT.  |
`--------------------------------*/

#if defined (__STDC__) || defined (__cplusplus)
static void
yysymprint (FILE *yyoutput, int yytype, YYSTYPE *yyvaluep)
#else
static void
yysymprint (yyoutput, yytype, yyvaluep)
    FILE *yyoutput;
    int yytype;
    YYSTYPE *yyvaluep;
#endif
{
  /* Pacify ``unused variable'' warnings.  */
  (void) yyvaluep;

  if (yytype < YYNTOKENS)
    {
      YYFPRINTF (yyoutput, "token %s (", yytname[yytype]);
# ifdef YYPRINT
      YYPRINT (yyoutput, yytoknum[yytype], *yyvaluep);
# endif
    }
  else
    YYFPRINTF (yyoutput, "nterm %s (", yytname[yytype]);

  switch (yytype)
    {
      default:
        break;
    }
  YYFPRINTF (yyoutput, ")");
}

#endif /* ! YYDEBUG */
/*-----------------------------------------------.
| Release the memory associated to this symbol.  |
`-----------------------------------------------*/

#if defined (__STDC__) || defined (__cplusplus)
static void
yydestruct (int yytype, YYSTYPE *yyvaluep)
#else
static void
yydestruct (yytype, yyvaluep)
    int yytype;
    YYSTYPE *yyvaluep;
#endif
{
  /* Pacify ``unused variable'' warnings.  */
  (void) yyvaluep;

  switch (yytype)
    {

      default:
        break;
    }
}


/* Prevent warnings from -Wmissing-prototypes.  */

#ifdef YYPARSE_PARAM
# if defined (__STDC__) || defined (__cplusplus)
int yyparse (void *YYPARSE_PARAM);
# else
int yyparse ();
# endif
#else /* ! YYPARSE_PARAM */
#if defined (__STDC__) || defined (__cplusplus)
int yyparse (void);
#else
int yyparse ();
#endif
#endif /* ! YYPARSE_PARAM */



/* The lookahead symbol.  */
int yychar;

/* The semantic value of the lookahead symbol.  */
YYSTYPE yylval;

/* Number of syntax errors so far.  */
int yynerrs;



/*----------.
| yyparse.  |
`----------*/

#ifdef YYPARSE_PARAM
# if defined (__STDC__) || defined (__cplusplus)
int yyparse (void *YYPARSE_PARAM)
# else
int yyparse (YYPARSE_PARAM)
  void *YYPARSE_PARAM;
# endif
#else /* ! YYPARSE_PARAM */
#if defined (__STDC__) || defined (__cplusplus)
int
yyparse (void)
#else
int
yyparse ()

#endif
#endif
{
  
  register int yystate;
  register int yyn;
  int yyresult;
  /* Number of tokens to shift before error messages enabled.  */
  int yyerrstatus;
  /* Lookahead token as an internal (translated) token number.  */
  int yytoken = 0;

  /* Three stacks and their tools:
     `yyss': related to states,
     `yyvs': related to semantic values,
     `yyls': related to locations.

     Refer to the stacks thru separate pointers, to allow yyoverflow
     to reallocate them elsewhere.  */

  /* The state stack.  */
  short	yyssa[YYINITDEPTH];
  short *yyss = yyssa;
  register short *yyssp;

  /* The semantic value stack.  */
  YYSTYPE yyvsa[YYINITDEPTH];
  YYSTYPE *yyvs = yyvsa;
  register YYSTYPE *yyvsp;



#define YYPOPSTACK   (yyvsp--, yyssp--)

  YYSIZE_T yystacksize = YYINITDEPTH;

  /* The variables used to return semantic value and location from the
     action routines.  */
  YYSTYPE yyval;


  /* When reducing, the number of symbols on the RHS of the reduced
     rule.  */
  int yylen;

  YYDPRINTF ((stderr, "Starting parse\n"));

  yystate = 0;
  yyerrstatus = 0;
  yynerrs = 0;
  yychar = YYEMPTY;		/* Cause a token to be read.  */

  /* Initialize stack pointers.
     Waste one element of value and location stack
     so that they stay on the same level as the state stack.
     The wasted elements are never initialized.  */

  yyssp = yyss;
  yyvsp = yyvs;

  goto yysetstate;

/*------------------------------------------------------------.
| yynewstate -- Push a new state, which is found in yystate.  |
`------------------------------------------------------------*/
 yynewstate:
  /* In all cases, when you get here, the value and location stacks
     have just been pushed. so pushing a state here evens the stacks.
     */
  yyssp++;

 yysetstate:
  *yyssp = yystate;

  if (yyss + yystacksize - 1 <= yyssp)
    {
      /* Get the current used size of the three stacks, in elements.  */
      YYSIZE_T yysize = yyssp - yyss + 1;

#ifdef yyoverflow
      {
	/* Give user a chance to reallocate the stack. Use copies of
	   these so that the &'s don't force the real ones into
	   memory.  */
	YYSTYPE *yyvs1 = yyvs;
	short *yyss1 = yyss;


	/* Each stack pointer address is followed by the size of the
	   data in use in that stack, in bytes.  This used to be a
	   conditional around just the two extra args, but that might
	   be undefined if yyoverflow is a macro.  */
	yyoverflow ("parser stack overflow",
		    &yyss1, yysize * sizeof (*yyssp),
		    &yyvs1, yysize * sizeof (*yyvsp),

		    &yystacksize);

	yyss = yyss1;
	yyvs = yyvs1;
      }
#else /* no yyoverflow */
# ifndef YYSTACK_RELOCATE
      goto yyoverflowlab;
# else
      /* Extend the stack our own way.  */
      if (YYMAXDEPTH <= yystacksize)
	goto yyoverflowlab;
      yystacksize *= 2;
      if (YYMAXDEPTH < yystacksize)
	yystacksize = YYMAXDEPTH;

      {
	short *yyss1 = yyss;
	union yyalloc *yyptr =
	  (union yyalloc *) YYSTACK_ALLOC (YYSTACK_BYTES (yystacksize));
	if (! yyptr)
	  goto yyoverflowlab;
	YYSTACK_RELOCATE (yyss);
	YYSTACK_RELOCATE (yyvs);

#  undef YYSTACK_RELOCATE
	if (yyss1 != yyssa)
	  YYSTACK_FREE (yyss1);
      }
# endif
#endif /* no yyoverflow */

      yyssp = yyss + yysize - 1;
      yyvsp = yyvs + yysize - 1;


      YYDPRINTF ((stderr, "Stack size increased to %lu\n",
		  (unsigned long int) yystacksize));

      if (yyss + yystacksize - 1 <= yyssp)
	YYABORT;
    }

  YYDPRINTF ((stderr, "Entering state %d\n", yystate));

  goto yybackup;

/*-----------.
| yybackup.  |
`-----------*/
yybackup:

/* Do appropriate processing given the current state.  */
/* Read a lookahead token if we need one and don't already have one.  */
/* yyresume: */

  /* First try to decide what to do without reference to lookahead token.  */

  yyn = yypact[yystate];
  if (yyn == YYPACT_NINF)
    goto yydefault;

  /* Not known => get a lookahead token if don't already have one.  */

  /* YYCHAR is either YYEMPTY or YYEOF or a valid lookahead symbol.  */
  if (yychar == YYEMPTY)
    {
      YYDPRINTF ((stderr, "Reading a token: "));
      yychar = YYLEX;
    }

  if (yychar <= YYEOF)
    {
      yychar = yytoken = YYEOF;
      YYDPRINTF ((stderr, "Now at end of input.\n"));
    }
  else
    {
      yytoken = YYTRANSLATE (yychar);
      YYDSYMPRINTF ("Next token is", yytoken, &yylval, &yylloc);
    }

  /* If the proper action on seeing token YYTOKEN is to reduce or to
     detect an error, take that action.  */
  yyn += yytoken;
  if (yyn < 0 || YYLAST < yyn || yycheck[yyn] != yytoken)
    goto yydefault;
  yyn = yytable[yyn];
  if (yyn <= 0)
    {
      if (yyn == 0 || yyn == YYTABLE_NINF)
	goto yyerrlab;
      yyn = -yyn;
      goto yyreduce;
    }

  if (yyn == YYFINAL)
    YYACCEPT;

  /* Shift the lookahead token.  */
  YYDPRINTF ((stderr, "Shifting token %s, ", yytname[yytoken]));

  /* Discard the token being shifted unless it is eof.  */
  if (yychar != YYEOF)
    yychar = YYEMPTY;

  *++yyvsp = yylval;


  /* Count tokens shifted since error; after three, turn off error
     status.  */
  if (yyerrstatus)
    yyerrstatus--;

  yystate = yyn;
  goto yynewstate;


/*-----------------------------------------------------------.
| yydefault -- do the default action for the current state.  |
`-----------------------------------------------------------*/
yydefault:
  yyn = yydefact[yystate];
  if (yyn == 0)
    goto yyerrlab;
  goto yyreduce;


/*-----------------------------.
| yyreduce -- Do a reduction.  |
`-----------------------------*/
yyreduce:
  /* yyn is the number of a rule to reduce with.  */
  yylen = yyr2[yyn];

  /* If YYLEN is nonzero, implement the default value of the action:
     `$$ = $1'.

     Otherwise, the following line sets YYVAL to garbage.
     This behavior is undocumented and Bison
     users should not rely upon it.  Assigning to YYVAL
     unconditionally makes the parser a bit smaller, and it avoids a
     GCC warning that YYVAL may be used uninitialized.  */
  yyval = yyvsp[1-yylen];


  YY_REDUCE_PRINT (yyn);
  switch (yyn)
    {
        case 2:
#line 248 "parse.y"
    {
			yyval._dcl = NULL;
			P_Input = yyval._dcl;
			YYACCEPT;
		}
    break;

  case 3:
#line 254 "parse.y"
    {
			yyval._dcl = P_NewDcl ();
			P_SetDclType (yyval._dcl, TT_FUNC);
			P_SetDclFuncDcl (yyval._dcl, yyvsp[0]._funcdcl);
			P_Input = yyval._dcl;
			YYACCEPT;
		}
    break;

  case 4:
#line 262 "parse.y"
    {
			yyval._dcl = P_NewDcl ();
			P_SetDclType (yyval._dcl, TT_VAR);
			P_SetDclVarDcl (yyval._dcl, yyvsp[0]._vardcl);
			P_Input = yyval._dcl;
			YYACCEPT;
		}
    break;

  case 5:
#line 270 "parse.y"
    {
			yyval._dcl = P_NewDcl ();
			P_SetDclType (yyval._dcl, TT_ASM);
			P_SetDclAsmDcl (yyval._dcl, yyvsp[0]._asmdcl);
			P_Input = yyval._dcl;
			YYACCEPT;
		}
    break;

  case 6:
#line 278 "parse.y"
    {
			yyval._dcl = P_NewDcl ();
			P_SetDclType (yyval._dcl, TT_INCLUDE);
			P_SetDclInclude (yyval._dcl, yyvsp[0].st_val);
			P_Input = yyval._dcl;
			YYACCEPT;
		}
    break;

  case 7:
#line 286 "parse.y"
    {
			yyval._dcl = yyvsp[0]._dcl;
			P_Input = yyval._dcl;
			YYACCEPT;
		}
    break;

  case 8:
#line 292 "parse.y"
    {
			yyval._dcl = P_NewDcl ();
			P_SetDclType (yyval._dcl, TT_SYMBOLTABLE);
			P_SetDclSymbolTable (yyval._dcl, yyvsp[0]._symboltable);
			P_Input = yyval._dcl;
			YYACCEPT;
		}
    break;

  case 9:
#line 300 "parse.y"
    {
			yyval._dcl = P_NewDcl ();
			P_SetDclType (yyval._dcl, TT_IPSYMTABENT);
			P_SetDclIPSymTabEnt (yyval._dcl, yyvsp[0]._ipsymtabent);
			P_Input = yyval._dcl;
			YYACCEPT;
		}
    break;

  case 10:
#line 308 "parse.y"
    {
			yyval._dcl = P_NewDcl ();
			P_SetDclType (yyval._dcl, TT_SYMTABENTRY);
			P_SetDclSymTabEntry (yyval._dcl, yyvsp[0]._symtabentry);
			P_Input = yyval._dcl;
			YYACCEPT;
		}
    break;

  case 11:
#line 319 "parse.y"
    {
			yyval.i_val = 0;
		}
    break;

  case 12:
#line 323 "parse.y"
    {
			yyval.i_val = yyvsp[0].i_val;
		}
    break;

  case 13:
#line 329 "parse.y"
    {
			yyval._expr = NULL;
		}
    break;

  case 14:
#line 333 "parse.y"
    {
			yyval._expr = yyvsp[0]._expr;
		}
    break;

  case 15:
#line 339 "parse.y"
    {
			yyval._asmdcl = P_NewAsmDcl ();
			P_SetAsmDclIsVolatile (yyval._asmdcl,
					       P_GetAsmStmtIsVolatile (yyvsp[-4]._asmstmt));
			P_SetAsmDclAsmClobbers (yyval._asmdcl,
						P_GetAsmStmtAsmClobbers (yyvsp[-4]._asmstmt));
			P_SetAsmDclAsmString (yyval._asmdcl, P_GetAsmStmtAsmString (yyvsp[-4]._asmstmt));
			P_SetAsmDclAsmOperands (yyval._asmdcl,
						P_GetAsmStmtAsmOperands (yyvsp[-4]._asmstmt));

			yyvsp[-4]._asmstmt = P_RemoveAsmStmt (yyvsp[-4]._asmstmt);

			P_SetAsmDclKey (yyval._asmdcl, yyvsp[-3]._key);
			P_SetAsmDclPragma (yyval._asmdcl, yyvsp[-2]._pragma);

			if (yyvsp[-1]._position)
			{
				P_SetAsmDclPosition (yyval._asmdcl, yyvsp[-1]._position);
				yyvsp[-1]._position = P_RemovePosition (yyvsp[-1]._position);
			}

			if (Handlers[ES_ASM])
				P_ExtRead (ES_ASM, (void *)yyval._asmdcl);
		}
    break;

  case 16:
#line 366 "parse.y"
    {
			yyval._expr = NULL;
		}
    break;

  case 17:
#line 370 "parse.y"
    {
			yyval._expr = yyvsp[0]._expr;
		}
    break;

  case 18:
#line 376 "parse.y"
    {
			yyval._asmstmt = P_NewAsmStmt ();
			P_SetAsmStmtAsmString (yyval._asmstmt, yyvsp[-2]._expr);
			P_SetAsmStmtAsmOperands (yyval._asmstmt, yyvsp[-1]._expr);
			P_SetAsmStmtAsmClobbers (yyval._asmstmt, yyvsp[0]._expr);
		}
    break;

  case 19:
#line 383 "parse.y"
    {
			yyval._asmstmt = P_NewAsmStmt ();
			P_SetAsmStmtIsVolatile (yyval._asmstmt, 1);
			P_SetAsmStmtAsmString (yyval._asmstmt, yyvsp[-2]._expr);
			P_SetAsmStmtAsmOperands (yyval._asmstmt, yyvsp[-1]._expr);
			P_SetAsmStmtAsmClobbers (yyval._asmstmt, yyvsp[0]._expr);
		}
    break;

  case 20:
#line 393 "parse.y"
    {
			yyval._basictype = BT_VOID;
		}
    break;

  case 21:
#line 397 "parse.y"
    {
			yyval._basictype = BT_CHAR;
		}
    break;

  case 22:
#line 401 "parse.y"
    {
			yyval._basictype = BT_SHORT;
		}
    break;

  case 23:
#line 405 "parse.y"
    {
			yyval._basictype = BT_INT;
		}
    break;

  case 24:
#line 409 "parse.y"
    {
			yyval._basictype = BT_LONG;
		}
    break;

  case 25:
#line 413 "parse.y"
    {
			yyval._basictype = BT_LONGLONG;
		}
    break;

  case 26:
#line 417 "parse.y"
    {
			yyval._basictype = BT_FLOAT;
		}
    break;

  case 27:
#line 421 "parse.y"
    {
			yyval._basictype = BT_DOUBLE;
		}
    break;

  case 28:
#line 425 "parse.y"
    {
			yyval._basictype = BT_LONGDOUBLE;
		}
    break;

  case 29:
#line 429 "parse.y"
    {
			yyval._basictype = BT_UNSIGNED;
		}
    break;

  case 30:
#line 433 "parse.y"
    {
			yyval._basictype = BT_VARARG;
		}
    break;

  case 31:
#line 437 "parse.y"
    {
			yyval._basictype = BT_BIT_FIELD;
		}
    break;

  case 32:
#line 443 "parse.y"
    {
			yyval._basictype = 0;
		}
    break;

  case 33:
#line 447 "parse.y"
    {
			yyval._basictype = yyvsp[-1]._basictype | yyvsp[0]._basictype;
		}
    break;

  case 34:
#line 454 "parse.y"
    {
			yyval._stmt = P_NewStmt ();
			P_SetStmtType (yyval._stmt, ST_COMPOUND);
			P_SetStmtCompound (yyval._stmt, P_NewCompound ());
			P_SetCompoundUniqueVarID (P_GetStmtCompound (yyval._stmt), yyvsp[-3].i_val);
			P_SetCompoundTypeList (P_GetStmtCompound (yyval._stmt), yyvsp[-2]._typelist);
			P_SetCompoundVarList (P_GetStmtCompound (yyval._stmt), yyvsp[-1]._varlist);
			P_SetCompoundStmtList (P_GetStmtCompound (yyval._stmt), yyvsp[0]._stmt);
		}
    break;

  case 35:
#line 466 "parse.y"
    {
			yyval._entrytype = ET_FUNC;
		}
    break;

  case 36:
#line 470 "parse.y"
    {
			yyval._entrytype = ET_TYPE_LOCAL;
		}
    break;

  case 37:
#line 474 "parse.y"
    {
			yyval._entrytype = ET_TYPE_GLOBAL;
		}
    break;

  case 38:
#line 478 "parse.y"
    {
			yyval._entrytype = ET_VAR_LOCAL;
		}
    break;

  case 39:
#line 482 "parse.y"
    {
			yyval._entrytype = ET_VAR_GLOBAL;
		}
    break;

  case 40:
#line 486 "parse.y"
    {
			yyval._entrytype = ET_STRUCT;
		}
    break;

  case 41:
#line 490 "parse.y"
    {
			yyval._entrytype = ET_UNION;
		}
    break;

  case 42:
#line 494 "parse.y"
    {
			yyval._entrytype = ET_ENUM;
		}
    break;

  case 43:
#line 498 "parse.y"
    {
			yyval._entrytype = ET_ASM;
		}
    break;

  case 44:
#line 502 "parse.y"
    {
			yyval._entrytype = ET_STMT;
		}
    break;

  case 45:
#line 506 "parse.y"
    {	
			yyval._entrytype = ET_EXPR;
		}
    break;

  case 46:
#line 510 "parse.y"
    {
			yyval._entrytype = ET_FIELD;
		}
    break;

  case 47:
#line 514 "parse.y"
    {
			yyval._entrytype = ET_ENUMFIELD;
		}
    break;

  case 48:
#line 518 "parse.y"
    {
			yyval._entrytype = ET_LABEL;
		}
    break;

  case 49:
#line 522 "parse.y"
    {
			yyval._entrytype = ET_SCOPE;
		}
    break;

  case 50:
#line 528 "parse.y"
    {
			yyval._enumfield = P_NewEnumField ();
			P_SetEnumFieldIdentifier (yyval._enumfield, yyvsp[-2]._identifier);
			yyvsp[-2]._identifier = P_RemoveIdentifier (yyvsp[-2]._identifier);
			P_SetEnumFieldValue (yyval._enumfield, yyvsp[-1]._expr);
		}
    break;

  case 51:
#line 535 "parse.y"
    {
			yyval._enumfield = P_NewEnumField ();
			P_SetEnumFieldIdentifier (yyval._enumfield, yyvsp[-1]._identifier);
			yyvsp[-1]._identifier = P_RemoveIdentifier (yyvsp[-1]._identifier);
		}
    break;

  case 52:
#line 543 "parse.y"
    {
			yyval._enumfield = NULL;
		}
    break;

  case 53:
#line 547 "parse.y"
    {
			yyval._enumfield = P_AppendEnumFieldNext (yyvsp[-1]._enumfield, yyvsp[0]._enumfield);
		}
    break;

  case 54:
#line 553 "parse.y"
    {
			P_SetExprID (yyvsp[-3]._expr, yyvsp[-4].i_val);
			P_SetExprPragma (yyvsp[-3]._expr, yyvsp[-2]._pragma);
			if (yyvsp[-1].f_val > 0)
			{
				P_SetExprProfile (yyvsp[-3]._expr, P_NewProfEXPR ());
				P_SetProfEXPRCount (P_GetExprProfile (yyvsp[-3]._expr), yyvsp[-1].f_val);
			}
			yyval._expr = yyvsp[-3]._expr;

			if (Handlers[ES_EXPR])
				P_ExtRead (ES_EXPR, (void *)yyval._expr);
		}
    break;

  case 55:
#line 569 "parse.y"
    {
			yyval._expr = NULL;
		}
    break;

  case 56:
#line 573 "parse.y"
    {
			yyval._expr = yyvsp[-1]._expr;
		}
    break;

  case 57:
#line 579 "parse.y"
    {
			yyval._expr = P_NewExpr ();
			P_SetExprOpcode (yyval._expr, OP_var);
			P_SetExprVarIdentifier (yyval._expr, yyvsp[0]._identifier);
			yyvsp[0]._identifier = P_RemoveIdentifier (yyvsp[0]._identifier);
		}
    break;

  case 58:
#line 586 "parse.y"
    {
			yyval._expr = P_NewExpr ();
			P_SetExprOpcode (yyval._expr, OP_int);
			P_SetExprScalar (yyval._expr, yyvsp[-1].i_val);
			P_SetExprType (yyval._expr, yyvsp[0]._key);
		}
    break;

  case 59:
#line 593 "parse.y"
    {
			yyval._expr = P_NewExpr ();
			P_SetExprOpcode (yyval._expr, OP_int);
			P_SetExprScalar (yyval._expr, yyvsp[-1].i_val);
			P_SetExprType (yyval._expr, yyvsp[0]._key);
		}
    break;

  case 60:
#line 600 "parse.y"
    {
			yyval._expr = P_NewExpr ();
			P_SetExprOpcode (yyval._expr, OP_int);
			P_SetExprFlags (yyval._expr, EF_UNSIGNED);
			P_SetExprUScalar (yyval._expr, yyvsp[-1].i_val);
			P_SetExprType (yyval._expr, yyvsp[0]._key);
		}
    break;

  case 61:
#line 608 "parse.y"
    {
			yyval._expr = P_NewExpr ();
			P_SetExprOpcode (yyval._expr, OP_real);
			P_SetExprReal (yyval._expr, yyvsp[-1].f_val);
			P_SetExprType (yyval._expr, yyvsp[0]._key);
		}
    break;

  case 62:
#line 615 "parse.y"
    {
			yyval._expr = P_NewExpr ();
			P_SetExprOpcode (yyval._expr, OP_float);
			P_SetExprReal (yyval._expr, yyvsp[-1].f_val);
			P_SetExprType (yyval._expr, yyvsp[0]._key);
		}
    break;

  case 63:
#line 622 "parse.y"
    {
			yyval._expr = P_NewExpr ();
			P_SetExprOpcode (yyval._expr, OP_double);
			P_SetExprReal (yyval._expr, yyvsp[-1].f_val);
			P_SetExprType (yyval._expr, yyvsp[0]._key);
		}
    break;

  case 64:
#line 629 "parse.y"
    {
			yyval._expr = P_NewExpr ();
			P_SetExprOpcode (yyval._expr, OP_char);
			P_SetExprString (yyval._expr, strdup (&yyvsp[-1].c_val));
			P_SetExprType (yyval._expr, yyvsp[0]._key);
		}
    break;

  case 65:
#line 636 "parse.y"
    {
			yyval._expr = P_NewExpr ();
			P_SetExprOpcode (yyval._expr, OP_string);
			P_SetExprString (yyval._expr, P_DQString2String (yyvsp[-1].st_val));
			free (yyvsp[-1].st_val);
			P_SetExprType (yyval._expr, yyvsp[0]._key);
		}
    break;

  case 66:
#line 644 "parse.y"
    {
			yyval._expr = P_NewExpr ();
			P_SetExprOpcode (yyval._expr, OP_dot);
			P_AppendExprOperands (yyval._expr, yyvsp[-1]._expr);
			P_SetExprVarIdentifier (yyval._expr, yyvsp[0]._identifier);
			yyvsp[0]._identifier = P_RemoveIdentifier (yyvsp[0]._identifier);
		}
    break;

  case 67:
#line 652 "parse.y"
    {
			yyval._expr = P_NewExpr ();
			P_SetExprOpcode (yyval._expr, OP_arrow);
			P_AppendExprOperands (yyval._expr, yyvsp[-1]._expr);
			P_SetExprVarIdentifier (yyval._expr, yyvsp[0]._identifier);
			yyvsp[0]._identifier = P_RemoveIdentifier (yyvsp[0]._identifier);
		}
    break;

  case 68:
#line 660 "parse.y"
    {
			yyval._expr = P_NewExpr ();
			P_SetExprOpcode (yyval._expr, OP_cast);
			P_SetExprType (yyval._expr, yyvsp[-1]._key);
			P_AppendExprOperands (yyval._expr, yyvsp[0]._expr);
		}
    break;

  case 69:
#line 667 "parse.y"
    {
			yyval._expr = P_NewExpr ();
			P_SetExprOpcode (yyval._expr, OP_expr_size);
#if 0
			P_AppendExprOperands (yyval._expr, yyvsp[0]._expr);
#endif
		}
    break;

  case 70:
#line 675 "parse.y"
    {
			yyval._expr = P_NewExpr ();
			P_SetExprOpcode (yyval._expr, OP_type_size);
#if 0
			P_SetExprVType (yyval._expr, yyvsp[0]._key);
#endif
		}
    break;

  case 71:
#line 683 "parse.y"
    {
			yyval._expr = P_NewExpr ();
			P_SetExprOpcode (yyval._expr, OP_quest);
			P_AppendExprOperands (yyval._expr, yyvsp[-2]._expr);
			P_AppendExprOperands (yyval._expr, yyvsp[-1]._expr);
			P_AppendExprOperands (yyval._expr, yyvsp[0]._expr);
		}
    break;

  case 72:
#line 691 "parse.y"
    {
			yyval._expr = P_NewExpr ();
			P_SetExprOpcode (yyval._expr, OP_disj);
			P_AppendExprOperands (yyval._expr, yyvsp[-1]._expr);
			P_AppendExprOperands (yyval._expr, yyvsp[0]._expr);
		}
    break;

  case 73:
#line 698 "parse.y"
    {
			yyval._expr = P_NewExpr ();
			P_SetExprOpcode (yyval._expr, OP_conj);
			P_AppendExprOperands (yyval._expr, yyvsp[-1]._expr);
			P_AppendExprOperands (yyval._expr, yyvsp[0]._expr);
		}
    break;

  case 74:
#line 705 "parse.y"
    {
			yyval._expr = P_NewExpr ();
			P_SetExprOpcode (yyval._expr, OP_compexpr);
			P_SetExprOperands (yyval._expr, yyvsp[0]._expr);
		}
    break;

  case 75:
#line 711 "parse.y"
    {
			yyval._expr = P_NewExpr ();
			P_SetExprOpcode (yyval._expr, OP_assign);
			P_AppendExprOperands (yyval._expr, yyvsp[-1]._expr);
			P_AppendExprOperands (yyval._expr, yyvsp[0]._expr);
		}
    break;

  case 76:
#line 718 "parse.y"
    {
			yyval._expr = P_NewExpr ();
			P_SetExprOpcode (yyval._expr, OP_or);
			P_AppendExprOperands (yyval._expr, yyvsp[-1]._expr);
			P_AppendExprOperands (yyval._expr, yyvsp[0]._expr);
		}
    break;

  case 77:
#line 725 "parse.y"
    {
			yyval._expr = P_NewExpr ();	
			P_SetExprOpcode (yyval._expr, OP_xor);
			P_AppendExprOperands (yyval._expr, yyvsp[-1]._expr);
			P_AppendExprOperands (yyval._expr, yyvsp[0]._expr);
		}
    break;

  case 78:
#line 732 "parse.y"
    {
			yyval._expr = P_NewExpr ();	
			P_SetExprOpcode (yyval._expr, OP_and);
			P_AppendExprOperands (yyval._expr, yyvsp[-1]._expr);
			P_AppendExprOperands (yyval._expr, yyvsp[0]._expr);
		}
    break;

  case 79:
#line 739 "parse.y"
    {
			yyval._expr = P_NewExpr ();	
			P_SetExprOpcode (yyval._expr, OP_eq);
			P_AppendExprOperands (yyval._expr, yyvsp[-1]._expr);
			P_AppendExprOperands (yyval._expr, yyvsp[0]._expr);
		}
    break;

  case 80:
#line 746 "parse.y"
    {
			yyval._expr = P_NewExpr ();	
			P_SetExprOpcode (yyval._expr, OP_ne);
			P_AppendExprOperands (yyval._expr, yyvsp[-1]._expr);
			P_AppendExprOperands (yyval._expr, yyvsp[0]._expr);
		}
    break;

  case 81:
#line 753 "parse.y"
    {
			yyval._expr = P_NewExpr ();	
			P_SetExprOpcode (yyval._expr, OP_lt);
			P_AppendExprOperands (yyval._expr, yyvsp[-1]._expr);
			P_AppendExprOperands (yyval._expr, yyvsp[0]._expr);
		}
    break;

  case 82:
#line 760 "parse.y"
    {
			yyval._expr = P_NewExpr ();	
			P_SetExprOpcode (yyval._expr, OP_le);
			P_AppendExprOperands (yyval._expr, yyvsp[-1]._expr);
			P_AppendExprOperands (yyval._expr, yyvsp[0]._expr);
		}
    break;

  case 83:
#line 767 "parse.y"
    {
			yyval._expr = P_NewExpr ();	
			P_SetExprOpcode (yyval._expr, OP_ge);
			P_AppendExprOperands (yyval._expr, yyvsp[-1]._expr);
			P_AppendExprOperands (yyval._expr, yyvsp[0]._expr);
		}
    break;

  case 84:
#line 774 "parse.y"
    {
			yyval._expr = P_NewExpr ();	
			P_SetExprOpcode (yyval._expr, OP_gt);
			P_AppendExprOperands (yyval._expr, yyvsp[-1]._expr);
			P_AppendExprOperands (yyval._expr, yyvsp[0]._expr);
		}
    break;

  case 85:
#line 781 "parse.y"
    {
			yyval._expr = P_NewExpr ();	
			P_SetExprOpcode (yyval._expr, OP_rshft);
			P_AppendExprOperands (yyval._expr, yyvsp[-1]._expr);
			P_AppendExprOperands (yyval._expr, yyvsp[0]._expr);
		}
    break;

  case 86:
#line 788 "parse.y"
    {
			yyval._expr = P_NewExpr ();	
			P_SetExprOpcode (yyval._expr, OP_lshft);
			P_AppendExprOperands (yyval._expr, yyvsp[-1]._expr);
			P_AppendExprOperands (yyval._expr, yyvsp[0]._expr);
		}
    break;

  case 87:
#line 795 "parse.y"
    {
			yyval._expr = P_NewExpr ();	
			P_SetExprOpcode (yyval._expr, OP_add);
			P_AppendExprOperands (yyval._expr, yyvsp[-1]._expr);
			P_AppendExprOperands (yyval._expr, yyvsp[0]._expr);
		}
    break;

  case 88:
#line 802 "parse.y"
    {
			yyval._expr = P_NewExpr ();	
			P_SetExprOpcode (yyval._expr, OP_sub);
			P_AppendExprOperands (yyval._expr, yyvsp[-1]._expr);
			P_AppendExprOperands (yyval._expr, yyvsp[0]._expr);
		}
    break;

  case 89:
#line 809 "parse.y"
    {
			yyval._expr = P_NewExpr ();	
			P_SetExprOpcode (yyval._expr, OP_mul);
			P_AppendExprOperands (yyval._expr, yyvsp[-1]._expr);
			P_AppendExprOperands (yyval._expr, yyvsp[0]._expr);
		}
    break;

  case 90:
#line 816 "parse.y"
    {
			yyval._expr = P_NewExpr ();	
			P_SetExprOpcode (yyval._expr, OP_div);
			P_AppendExprOperands (yyval._expr, yyvsp[-1]._expr);
			P_AppendExprOperands (yyval._expr, yyvsp[0]._expr);
		}
    break;

  case 91:
#line 823 "parse.y"
    {
			yyval._expr = P_NewExpr ();	
			P_SetExprOpcode (yyval._expr, OP_mod);
			P_AppendExprOperands (yyval._expr, yyvsp[-1]._expr);
			P_AppendExprOperands (yyval._expr, yyvsp[0]._expr);
		}
    break;

  case 92:
#line 830 "parse.y"
    {
			yyval._expr = P_NewExpr ();	
			P_SetExprOpcode (yyval._expr, OP_neg);
			P_AppendExprOperands (yyval._expr, yyvsp[0]._expr);
		}
    break;

  case 93:
#line 836 "parse.y"
    {
			yyval._expr = P_NewExpr ();	
			P_SetExprOpcode (yyval._expr, OP_not);
			P_AppendExprOperands (yyval._expr, yyvsp[0]._expr);
		}
    break;

  case 94:
#line 842 "parse.y"
    {
			yyval._expr = P_NewExpr ();	
			P_SetExprOpcode (yyval._expr, OP_inv);
			P_AppendExprOperands (yyval._expr, yyvsp[0]._expr);
		}
    break;

  case 95:
#line 848 "parse.y"
    {
			yyval._expr = P_NewExpr ();	
			P_SetExprOpcode (yyval._expr, OP_preinc);
			P_AppendExprOperands (yyval._expr, yyvsp[0]._expr);
		}
    break;

  case 96:
#line 854 "parse.y"
    {
			yyval._expr = P_NewExpr ();	
			P_SetExprOpcode (yyval._expr, OP_predec);
			P_AppendExprOperands (yyval._expr, yyvsp[0]._expr);
		}
    break;

  case 97:
#line 860 "parse.y"
    {
			yyval._expr = P_NewExpr ();	
			P_SetExprOpcode (yyval._expr, OP_postinc);
			P_AppendExprOperands (yyval._expr, yyvsp[0]._expr);
		}
    break;

  case 98:
#line 866 "parse.y"
    {
			yyval._expr = P_NewExpr ();	
			P_SetExprOpcode (yyval._expr, OP_postdec);
			P_AppendExprOperands (yyval._expr, yyvsp[0]._expr);
		}
    break;

  case 99:
#line 872 "parse.y"
    {
			yyval._expr = P_NewExpr ();	
			P_SetExprOpcode (yyval._expr, OP_Aadd);
			P_AppendExprOperands (yyval._expr, yyvsp[-1]._expr);
			P_AppendExprOperands (yyval._expr, yyvsp[0]._expr);
		}
    break;

  case 100:
#line 879 "parse.y"
    {
			yyval._expr = P_NewExpr ();	
			P_SetExprOpcode (yyval._expr, OP_Asub);
			P_AppendExprOperands (yyval._expr, yyvsp[-1]._expr);
			P_AppendExprOperands (yyval._expr, yyvsp[0]._expr);
		}
    break;

  case 101:
#line 886 "parse.y"
    {
			yyval._expr = P_NewExpr ();	
			P_SetExprOpcode (yyval._expr, OP_Amul);
			P_AppendExprOperands (yyval._expr, yyvsp[-1]._expr);
			P_AppendExprOperands (yyval._expr, yyvsp[0]._expr);
		}
    break;

  case 102:
#line 893 "parse.y"
    {
			yyval._expr = P_NewExpr ();	
			P_SetExprOpcode (yyval._expr, OP_Adiv);
			P_AppendExprOperands (yyval._expr, yyvsp[-1]._expr);
			P_AppendExprOperands (yyval._expr, yyvsp[0]._expr);
		}
    break;

  case 103:
#line 900 "parse.y"
    {
			yyval._expr = P_NewExpr ();	
			P_SetExprOpcode (yyval._expr, OP_Amod);
			P_AppendExprOperands (yyval._expr, yyvsp[-1]._expr);
			P_AppendExprOperands (yyval._expr, yyvsp[0]._expr);
		}
    break;

  case 104:
#line 907 "parse.y"
    {
			yyval._expr = P_NewExpr ();	
			P_SetExprOpcode (yyval._expr, OP_Alshft);
			P_AppendExprOperands (yyval._expr, yyvsp[-1]._expr);
			P_AppendExprOperands (yyval._expr, yyvsp[0]._expr);
		}
    break;

  case 105:
#line 914 "parse.y"
    {
			yyval._expr = P_NewExpr ();	
			P_SetExprOpcode (yyval._expr, OP_Arshft);
			P_AppendExprOperands (yyval._expr, yyvsp[-1]._expr);
			P_AppendExprOperands (yyval._expr, yyvsp[0]._expr);
		}
    break;

  case 106:
#line 921 "parse.y"
    {
			yyval._expr = P_NewExpr ();	
			P_SetExprOpcode (yyval._expr, OP_Aand);
			P_AppendExprOperands (yyval._expr, yyvsp[-1]._expr);
			P_AppendExprOperands (yyval._expr, yyvsp[0]._expr);
		}
    break;

  case 107:
#line 928 "parse.y"
    {
			yyval._expr = P_NewExpr ();	
			P_SetExprOpcode (yyval._expr, OP_Aor);
			P_AppendExprOperands (yyval._expr, yyvsp[-1]._expr);
			P_AppendExprOperands (yyval._expr, yyvsp[0]._expr);
		}
    break;

  case 108:
#line 935 "parse.y"
    {
			yyval._expr = P_NewExpr ();	
			P_SetExprOpcode (yyval._expr, OP_Axor);
			P_AppendExprOperands (yyval._expr, yyvsp[-1]._expr);
			P_AppendExprOperands (yyval._expr, yyvsp[0]._expr);
		}
    break;

  case 109:
#line 942 "parse.y"
    {
			yyval._expr = P_NewExpr ();	
			P_SetExprOpcode (yyval._expr, OP_indr);
			P_AppendExprOperands (yyval._expr, yyvsp[0]._expr);
		}
    break;

  case 110:
#line 948 "parse.y"
    {
			yyval._expr = P_NewExpr ();	
			P_SetExprOpcode (yyval._expr, OP_addr);
			P_AppendExprOperands (yyval._expr, yyvsp[0]._expr);
		}
    break;

  case 111:
#line 954 "parse.y"
    {
			yyval._expr = P_NewExpr ();
			P_SetExprOpcode (yyval._expr, OP_index);
			P_AppendExprOperands (yyval._expr, yyvsp[-1]._expr);
			P_AppendExprOperands (yyval._expr, yyvsp[0]._expr);
		}
    break;

  case 112:
#line 961 "parse.y"
    {
			yyval._expr = P_NewExpr ();	
			P_SetExprOpcode (yyval._expr, OP_call);
			P_AppendExprOperands (yyval._expr, yyvsp[-1]._expr);
			P_AppendExprOperands (yyval._expr, yyvsp[0]._expr);
		}
    break;

  case 113:
#line 968 "parse.y"
    {
			int flag, i;

			yyval._expr = P_NewExpr ();	
			P_SetExprOpcode (yyval._expr, OP_asm_oprd);
			P_SetExprAsmoprd (yyval._expr, P_NewAsmoprd ());
			
			flag = 0;
			for (i = 0; yyvsp[-1].st_val[i]; i++)
			{
				switch (yyvsp[-1].st_val[i])
				{
				case '+':
					flag |= 0x03;
					break;
				case '=':
					flag |= 0x02;
					break;
				case '&':
					flag |= 0x04;
					break;
				case '%':
					flag |= 0x08;
					break;
				case '*':
					flag |= 0x10;
					break;
				case '#':
					flag |= 0x20;
					break;
				case '?':
					flag |= 0x40;
					break;
				case '!':
					flag |= 0x80;
					break;
				case '"':
					break;
				default:
					assert (0);
				}
			}
			if (flag == 0)
				flag = 0x01;

			P_SetAsmoprdModifiers (P_GetExprAsmoprd (yyval._expr), flag);
			P_SetAsmoprdConstraints (P_GetExprAsmoprd (yyval._expr), 
						 P_DQString2String (yyvsp[0].st_val));
			free (yyvsp[-1].st_val);
			free (yyvsp[0].st_val);
		}
    break;

  case 114:
#line 1020 "parse.y"
    {
			yyval._expr = P_NewExpr ();
			P_SetExprOpcode (yyval._expr, OP_stmt_expr);
			P_SetExprStmt (yyval._expr, yyvsp[0]._stmt);
		}
    break;

  case 115:
#line 1026 "parse.y"
    {
			yyval._expr = P_NewExpr ();
			P_SetExprOpcode (yyval._expr, OP_null);
		}
    break;

  case 116:
#line 1031 "parse.y"
    {
			yyval._expr = P_NewExpr ();
			P_SetExprOpcode (yyval._expr, OP_sync);
		}
    break;

  case 117:
#line 1036 "parse.y"
    {
			yyval._expr = P_NewExpr ();	
			P_SetExprOpcode (yyval._expr, OP_phi);
			P_AppendExprOperands (yyval._expr, yyvsp[-1]._expr);
			P_AppendExprOperands (yyval._expr, yyvsp[0]._expr);
		}
    break;

  case 118:
#line 1045 "parse.y"
    {
			yyval._expr = NULL;
		}
    break;

  case 119:
#line 1049 "parse.y"
    {
			yyval._expr = P_AppendExprNext (yyvsp[-1]._expr, yyvsp[0]._expr);
		}
    break;

  case 120:
#line 1055 "parse.y"
    {
			Expr temp_expr, prev_temp_expr;

			yyval._expr = yyvsp[-1]._expr;

			if (yyval._expr)
			{
				prev_temp_expr = yyval._expr;
				temp_expr = yyval._expr->next;

				while (temp_expr)
				{
					temp_expr->previous = prev_temp_expr;

					prev_temp_expr = temp_expr;
					temp_expr = temp_expr->next;
				}
			}
		}
    break;

  case 121:
#line 1077 "parse.y"
    {
			yyval._field = P_NewField ();
			P_SetFieldIdentifier (yyval._field, yyvsp[-8]._identifier);
			yyvsp[-8]._identifier = P_RemoveIdentifier (yyvsp[-8]._identifier);
			P_SetFieldType (yyval._field, yyvsp[-6]._key);
			P_SetFieldIsBitField(yyval._field, 0);
			P_SetFieldOffset (yyval._field, yyvsp[-4].i_val);
			P_SetFieldParentKey (yyval._field, yyvsp[-2]._key);
			P_SetFieldPragma (yyval._field, yyvsp[-1]._pragma);
		}
    break;

  case 122:
#line 1090 "parse.y"
    {
			yyval._field = P_NewField ();
			P_SetFieldIdentifier (yyval._field, yyvsp[-11]._identifier);
			yyvsp[-11]._identifier = P_RemoveIdentifier (yyvsp[-11]._identifier);
			P_SetFieldType (yyval._field, yyvsp[-9]._key);
			P_SetFieldIsBitField(yyval._field, 1);
			P_SetFieldBitSize (yyval._field, yyvsp[-7].i_val);
			P_SetFieldBitOffsetRemainder (yyval._field, yyvsp[-6].i_val);
			P_SetFieldOffset (yyval._field, yyvsp[-4].i_val);
			P_SetFieldParentKey (yyval._field, yyvsp[-2]._key);
			P_SetFieldPragma (yyval._field, yyvsp[-1]._pragma);
		}
    break;

  case 123:
#line 1105 "parse.y"
    {
			yyval._field = NULL;
		}
    break;

  case 124:
#line 1109 "parse.y"
    {
			yyval._field = P_AppendFieldNext (yyvsp[-1]._field, yyvsp[0]._field);
		}
    break;

  case 125:
#line 1115 "parse.y"
    {
			yyval._filetype = FT_SOURCE;
		}
    break;

  case 126:
#line 1119 "parse.y"
    {
			yyval._filetype = FT_HEADER;
		}
    break;

  case 127:
#line 1126 "parse.y"
    {
			yyval._funcdcl = P_NewFuncDcl ();
			P_SetFuncDclIdentifier (yyval._funcdcl, yyvsp[-9]._identifier);
			yyvsp[-9]._identifier = P_RemoveIdentifier (yyvsp[-9]._identifier);
			P_SetFuncDclType (yyval._funcdcl, yyvsp[-7]._key);
			P_SetFuncDclQualifier (yyval._funcdcl, yyvsp[-6]._varqual);
			P_SetFuncDclMaxExprID (yyval._funcdcl, yyvsp[-5].i_val);
			P_SetFuncDclPragma (yyval._funcdcl, yyvsp[-4]._pragma);
			P_SetFuncDclParam (yyval._funcdcl, yyvsp[-3]._varlist);
			P_SetFuncDclStmt (yyval._funcdcl, yyvsp[-2]._stmt);
			if (yyvsp[-1]._position)
			{
				P_SetFuncDclPosition (yyval._funcdcl, yyvsp[-1]._position);
				yyvsp[-1]._position = P_RemovePosition (yyvsp[-1]._position);
			}

			if (Handlers[ES_FUNC])
				P_ExtRead (ES_FUNC, (void *)yyval._funcdcl);
		}
    break;

  case 128:
#line 1147 "parse.y"
    {
			yyval._funcdcl = P_NewFuncDcl ();
			P_SetFuncDclIdentifier (yyval._funcdcl, yyvsp[-7]._identifier);
			yyvsp[-7]._identifier = P_RemoveIdentifier (yyvsp[-7]._identifier);
			P_SetFuncDclType (yyval._funcdcl, yyvsp[-5]._key);
			P_SetFuncDclQualifier (yyval._funcdcl, yyvsp[-4]._varqual);
			P_SetFuncDclMaxExprID (yyval._funcdcl, yyvsp[-3].i_val);
			P_SetFuncDclPragma (yyval._funcdcl, yyvsp[-2]._pragma);
			if (yyvsp[-1]._position)
			{
				P_SetFuncDclPosition (yyval._funcdcl, yyvsp[-1]._position);
				yyvsp[-1]._position = P_RemovePosition (yyvsp[-1]._position);
			}

			if (Handlers[ES_FUNC])
				P_ExtRead (ES_FUNC, (void *)yyval._funcdcl);
		}
    break;

  case 129:
#line 1167 "parse.y"
    {
			yyval._identifier = P_NewIdentifier ();
			P_SetIdentifierKey (yyval._identifier, yyvsp[0]._key);
		}
    break;

  case 130:
#line 1172 "parse.y"
    {
			yyval._identifier = P_NewIdentifier ();
			P_SetIdentifierName (yyval._identifier, P_DQString2String (yyvsp[-1].st_val));
			free (yyvsp[-1].st_val);
			P_SetIdentifierKey (yyval._identifier, yyvsp[0]._key);
		}
    break;

  case 131:
#line 1181 "parse.y"
    {
			yyval.st_val = NULL;
		}
    break;

  case 132:
#line 1185 "parse.y"
    {
			yyval.st_val = P_DQString2String (yyvsp[0].st_val);
			free (yyvsp[0].st_val);
		}
    break;

  case 133:
#line 1192 "parse.y"
    {
			yyval.st_val = P_DQString2String (yyvsp[-1].st_val);
			free (yyvsp[-1].st_val);
		}
    break;

  case 134:
#line 1199 "parse.y"
    {
			yyval._init = P_NewInit ();
			P_SetInitExpr (yyval._init, yyvsp[-1]._expr);
			P_SetInitPragma (yyval._init, yyvsp[0]._pragma);

			if (Handlers[ES_INIT])
				P_ExtRead (ES_INIT, (void *)yyval._init);
		}
    break;

  case 135:
#line 1208 "parse.y"
    {
			yyval._init = P_NewInit ();
			P_SetInitSet (yyval._init, yyvsp[-1]._init);
			P_SetInitPragma (yyval._init, yyvsp[0]._pragma);

			if (Handlers[ES_INIT])
				P_ExtRead (ES_INIT, (void *)yyval._init);
		}
    break;

  case 136:
#line 1219 "parse.y"
    {
			yyval._init = NULL;
		}
    break;

  case 137:
#line 1223 "parse.y"
    {
			yyval._init = P_AppendInitNext (yyvsp[-1]._init, yyvsp[0]._init);
		}
    break;

  case 138:
#line 1229 "parse.y"
    {
			yyval._init = yyvsp[-1]._init;
		}
    break;

  case 139:
#line 1236 "parse.y"
    {
			yyval._ipsteflags = IPSTEF_NOT_AVAIL;
		}
    break;

  case 140:
#line 1242 "parse.y"
    {
			yyval._ipsteflags = 0;
		}
    break;

  case 141:
#line 1246 "parse.y"
    {
			yyval._ipsteflags = yyvsp[-1]._ipsteflags | yyvsp[0]._ipsteflags;
		}
    break;

  case 142:
#line 1254 "parse.y"
    {
			yyval._ipsymtabent = P_NewIPSymTabEnt ();
			P_SetIPSymTabEntSourceName (yyval._ipsymtabent,
						    P_DQString2String (yyvsp[-11].st_val));
			free (yyvsp[-11].st_val);
			P_SetIPSymTabEntKey (yyval._ipsymtabent, yyvsp[-10].i_val);
			P_SetIPSymTabEntFileType (yyval._ipsymtabent, yyvsp[-9]._filetype);
			P_SetIPSymTabEntInName (yyval._ipsymtabent, yyvsp[-8].st_val);
			P_SetIPSymTabEntOutName (yyval._ipsymtabent, yyvsp[-7].st_val);
			P_SetIPSymTabEntFlags (yyval._ipsymtabent, yyvsp[-6]._ipsteflags);
			P_SetIPSymTabEntNumEntries (yyval._ipsymtabent, yyvsp[-4].i_val);
			P_SetIPSymTabEntOffset (yyval._ipsymtabent, yyvsp[-2].i_val);
			P_SetIPSymTabEntPragma (yyval._ipsymtabent, yyvsp[-1]._pragma);

			if (P_TstIPSymTabEntFlags (yyval._ipsymtabent, IPSTEF_NOT_AVAIL))
			  P_SetIPSymTabEntInFileStatus (yyval._ipsymtabent, FS_NOT_AVAIL);

			if (Handlers[ES_IPSYMTABENT])
				P_ExtRead (ES_IPSYMTABENT, (void *)yyval._ipsymtabent);
		}
    break;

  case 143:
#line 1277 "parse.y"
    {
			yyval._key.file = yyvsp[-2].i_val;
			yyval._key.sym = yyvsp[-1].i_val;
		}
    break;

  case 144:
#line 1284 "parse.y"
    {
			yyval._label = P_NewLabel ();
			P_SetLabelType (yyval._label, LB_LABEL);
			P_SetLabelIdentifier (yyval._label, yyvsp[-1]._identifier);
			yyvsp[-1]._identifier = P_RemoveIdentifier (yyvsp[-1]._identifier);
		}
    break;

  case 145:
#line 1291 "parse.y"
    {
			yyval._label = P_NewLabel ();
			P_SetLabelType (yyval._label, LB_CASE);
			P_SetLabelExpression (yyval._label, yyvsp[-1]._expr);
		}
    break;

  case 146:
#line 1297 "parse.y"
    {
			yyval._label = P_NewLabel ();
			P_SetLabelType (yyval._label, LB_DEFAULT);
		}
    break;

  case 147:
#line 1304 "parse.y"
    {
			yyval._label = NULL;
		}
    break;

  case 148:
#line 1308 "parse.y"
    {
			yyval._label = P_AppendLabelNext (yyvsp[-1]._label, yyvsp[0]._label);
		}
    break;

  case 149:
#line 1314 "parse.y"
    {
			yyval._basictype = BT_STRUCT;
		}
    break;

  case 150:
#line 1318 "parse.y"
    {
			yyval._basictype = BT_UNION;
		}
    break;

  case 151:
#line 1322 "parse.y"
    {
			yyval._basictype = BT_ENUM;
		}
    break;

  case 152:
#line 1328 "parse.y"
    {
			yyval.st_val = NULL;
		}
    break;

  case 153:
#line 1332 "parse.y"
    {
		   	yyval.st_val = P_DQString2String (yyvsp[0].st_val);
			free (yyvsp[0].st_val);
		}
    break;

  case 154:
#line 1339 "parse.y"
    {
			yyval._param = P_NewParam ();
			P_SetParamKey (yyval._param, yyvsp[-1]._key);
		}
    break;

  case 155:
#line 1346 "parse.y"
    {
			yyval._param = NULL;
		}
    break;

  case 156:
#line 1350 "parse.y"
    {
			yyval._param = P_AppendParamNext (yyvsp[-1]._param, yyvsp[0]._param);
		}
    break;

  case 157:
#line 1356 "parse.y"
    {
			yyval._expr = P_NewExpr ();
			P_SetExprOpcode (yyval._expr, OP_var);
			P_SetExprVarIdentifier (yyval._expr, yyvsp[0]._identifier);
			yyvsp[0]._identifier = P_RemoveIdentifier (yyvsp[0]._identifier);
		}
    break;

  case 158:
#line 1367 "parse.y"
    {
			yyval._position = NULL;
		}
    break;

  case 159:
#line 1371 "parse.y"
    {
			yyval._position = P_NewPosition ();
			P_SetPositionFilename (yyval._position, P_DQString2String (yyvsp[-2].st_val));
			free (yyvsp[-2].st_val);
			P_SetPositionLineno (yyval._position, yyvsp[-1].i_val);
			P_SetPositionColno (yyval._position, yyvsp[0].i_val);
		}
    break;

  case 160:
#line 1381 "parse.y"
    {
			yyval._pragma = P_NewPragma ();
			P_SetPragmaSpecifier (yyval._pragma, P_DQString2String (yyvsp[-1].st_val));
			free (yyvsp[-1].st_val);
			P_SetPragmaExpr (yyval._pragma, yyvsp[0]._expr);
		}
    break;

  case 161:
#line 1390 "parse.y"
    {
			yyval._pragma = NULL;
		}
    break;

  case 162:
#line 1394 "parse.y"
    {
			yyval._pragma = P_AppendPragmaNext (yyvsp[-1]._pragma, yyvsp[0]._pragma);
		}
    break;

  case 163:
#line 1400 "parse.y"
    {
			yyval._profst = NULL;
		}
    break;

  case 164:
#line 1404 "parse.y"
    {
			yyval._profst = yyvsp[0]._profst;
		}
    break;

  case 165:
#line 1410 "parse.y"
    {
			yyval._profst = NULL;
		}
    break;

  case 166:
#line 1414 "parse.y"
    {
			ProfST new = P_NewProfST ();

			P_SetProfSTCount (new, yyvsp[-1].f_val);
			P_SetProfSTNext (new, yyvsp[0]._profst);
			yyval._profst = new;
		}
    break;

  case 167:
#line 1424 "parse.y"
    {
			yyval.f_val = 0.0;
		}
    break;

  case 168:
#line 1428 "parse.y"
    {
			yyval.f_val = yyvsp[0].f_val;
		}
    break;

  case 169:
#line 1434 "parse.y"
    {
			yyval._pstmt = P_NewPstmt ();
			if (yyvsp[-2]._position)
			{
				P_SetPstmtPosition (yyval._pstmt, yyvsp[-2]._position);
				yyvsp[-2]._position = P_RemovePosition (yyvsp[-2]._position);
			}
			P_SetPstmtPragma (yyval._pstmt, yyvsp[-1]._pragma);
			P_SetPstmtStmt (yyval._pstmt, yyvsp[0]._stmt);

			if (Handlers[ES_PSTMT])
				P_ExtRead (ES_PSTMT, (void *)yyval._pstmt);
		}
    break;

  case 170:
#line 1450 "parse.y"
    {
			yyval._scope = P_NewScope ();
			P_SetScopeKey (yyval._scope, yyvsp[-1]._key);
			P_SetScopeScopeEntry (yyval._scope, yyvsp[0]._scopeentry);
		}
    break;

  case 171:
#line 1458 "parse.y"
    {
			yyval._scopeentry = NULL;
		}
    break;

  case 172:
#line 1462 "parse.y"
    {
			yyval._scopeentry = \
			  P_AppendScopeEntryNext (yyvsp[-1]._scopeentry,
						  P_NewScopeEntryWithKey (yyvsp[0]._key));
		}
    break;

  case 173:
#line 1470 "parse.y"
    {
			yyval.i_val = 0;
		}
    break;

  case 174:
#line 1474 "parse.y"
    {
			yyval.i_val = yyvsp[0].i_val;
		}
    break;

  case 175:
#line 1480 "parse.y"
    {
			yyval.i_val = -1;
		}
    break;

  case 176:
#line 1484 "parse.y"
    {
			yyval.i_val = yyvsp[0].i_val;
		}
    break;

  case 177:
#line 1490 "parse.y"
    {
			yyval._stmt = yyvsp[-5]._stmt;
			P_SetStmtLabels (yyval._stmt, yyvsp[-8]._label);
			P_SetStmtKey (yyval._stmt, yyvsp[-6]._key);
			P_SetStmtPragma (yyval._stmt, yyvsp[-4]._pragma);

			if (yyvsp[-3].i_val != -1)
			{
				P_SetStmtShadow \
				  (yyval._stmt, 
				   P_NewShadowWithExprID \
				     (P_GetStmtShadow (yyval._stmt), P_GetStmtExpr (yyval._stmt),
				      yyvsp[-3].i_val));
			}

			if (yyvsp[-2]._position)
			{
				P_SetStmtPosition (yyval._stmt, yyvsp[-2]._position);
				yyvsp[-2]._position = P_RemovePosition (yyvsp[-2]._position);
			}

			if (yyvsp[-1]._profst)
				P_SetStmtProfile (yyval._stmt, yyvsp[-1]._profst);

			if (Handlers[ES_STMT])
				P_ExtRead (ES_STMT, (void *)yyval._stmt);
		}
    break;

  case 178:
#line 1520 "parse.y"
    {
			yyval._stmt = P_NewStmt ();
			P_SetStmtType (yyval._stmt, ST_EXPR);
			P_SetStmtExpr (yyval._stmt, yyvsp[0]._expr);
		}
    break;

  case 179:
#line 1526 "parse.y"
    {
			yyval._stmt = yyvsp[0]._stmt;
		}
    break;

  case 180:
#line 1530 "parse.y"
    {
			yyval._stmt = P_NewStmt ();
			P_SetStmtType (yyval._stmt, ST_SERLOOP);
			P_SetStmtSerLoop (yyval._stmt, P_NewSerLoop ());
			P_SetSerLoopLoopType (P_GetStmtSerLoop (yyval._stmt), LT_DO);
			P_SetSerLoopLoopBody (P_GetStmtSerLoop (yyval._stmt), yyvsp[-2]._stmt);
			P_SetSerLoopCondExpr (P_GetStmtSerLoop (yyval._stmt), yyvsp[0]._expr);
		}
    break;

  case 181:
#line 1539 "parse.y"
    {
			yyval._stmt = P_NewStmt ();
			P_SetStmtType (yyval._stmt, ST_SERLOOP);
			P_SetStmtSerLoop (yyval._stmt, P_NewSerLoop ());
			P_SetSerLoopLoopType (P_GetStmtSerLoop (yyval._stmt), LT_WHILE);
			P_SetSerLoopCondExpr (P_GetStmtSerLoop (yyval._stmt), yyvsp[-2]._expr);
			P_SetSerLoopLoopBody (P_GetStmtSerLoop (yyval._stmt), yyvsp[0]._stmt);
		}
    break;

  case 182:
#line 1548 "parse.y"
    {
			yyval._stmt = P_NewStmt ();
			P_SetStmtType (yyval._stmt, ST_SERLOOP);
			P_SetStmtSerLoop (yyval._stmt, P_NewSerLoop ());
			P_SetSerLoopLoopType (P_GetStmtSerLoop (yyval._stmt), LT_FOR);
			P_SetSerLoopInitExpr (P_GetStmtSerLoop (yyval._stmt), yyvsp[-4]._expr);
			P_SetSerLoopCondExpr (P_GetStmtSerLoop (yyval._stmt), yyvsp[-3]._expr);
			P_SetSerLoopIterExpr (P_GetStmtSerLoop (yyval._stmt), yyvsp[-2]._expr);
			P_SetSerLoopLoopBody (P_GetStmtSerLoop (yyval._stmt), yyvsp[0]._stmt);
		}
    break;

  case 183:
#line 1559 "parse.y"
    {
			yyval._stmt = P_NewStmt ();
			P_SetStmtType (yyval._stmt, ST_IF);
			P_SetStmtIfStmt (yyval._stmt, P_NewIfStmt ());
			P_SetIfStmtCondExpr (P_GetStmtIfStmt (yyval._stmt), yyvsp[-2]._expr);
			P_SetIfStmtThenBlock (P_GetStmtIfStmt (yyval._stmt), yyvsp[0]._stmt);
		}
    break;

  case 184:
#line 1567 "parse.y"
    {
			yyval._stmt = P_NewStmt ();
			P_SetStmtType (yyval._stmt, ST_IF);
			P_SetStmtIfStmt (yyval._stmt, P_NewIfStmt ());
			P_SetIfStmtCondExpr (P_GetStmtIfStmt (yyval._stmt), yyvsp[-4]._expr);
			P_SetIfStmtThenBlock (P_GetStmtIfStmt (yyval._stmt), yyvsp[-2]._stmt);
			P_SetIfStmtElseBlock (P_GetStmtIfStmt (yyval._stmt), yyvsp[0]._stmt);
		}
    break;

  case 185:
#line 1576 "parse.y"
    {
			yyval._stmt = P_NewStmt ();
			P_SetStmtType (yyval._stmt, ST_SWITCH);
			P_SetStmtSwitchStmt (yyval._stmt, P_NewSwitchStmt ());
			P_SetSwitchStmtExpression (P_GetStmtSwitchStmt (yyval._stmt),
						   yyvsp[-1]._expr);
			P_SetSwitchStmtSwitchBody (P_GetStmtSwitchStmt (yyval._stmt),
						   yyvsp[0]._stmt);
		}
    break;

  case 186:
#line 1586 "parse.y"
    {
			yyval._stmt = P_NewStmt ();
			P_SetStmtType (yyval._stmt, ST_RETURN);
		}
    break;

  case 187:
#line 1591 "parse.y"
    {
			yyval._stmt = P_NewStmt ();
			P_SetStmtType (yyval._stmt, ST_RETURN);
			P_SetStmtRet (yyval._stmt, yyvsp[0]._expr);
		}
    break;

  case 188:
#line 1597 "parse.y"
    {
			yyval._stmt = P_NewStmt ();
			P_SetStmtType (yyval._stmt, ST_GOTO);
			P_SetStmtGotoIdentifier (yyval._stmt, yyvsp[0]._identifier);
			yyvsp[0]._identifier = P_RemoveIdentifier (yyvsp[0]._identifier);
		}
    break;

  case 189:
#line 1604 "parse.y"
    {
			yyval._stmt = P_NewStmt ();
			P_SetStmtType (yyval._stmt, ST_PSTMT);
			P_SetStmtPstmt (yyval._stmt, yyvsp[0]._pstmt);
		}
    break;

  case 190:
#line 1610 "parse.y"
    {
			yyval._stmt = P_NewStmt ();
			P_SetStmtType (yyval._stmt, ST_ADVANCE);
			P_SetStmtAdvance (yyval._stmt, P_NewAdvance ());
			P_SetAdvanceMarker (P_GetStmtAdvance (yyval._stmt), yyvsp[0].i_val);
		}
    break;

  case 191:
#line 1617 "parse.y"
    {
			yyval._stmt = P_NewStmt ();
			P_SetStmtType (yyval._stmt, ST_AWAIT);
			P_SetStmtAwait (yyval._stmt, P_NewAwait ());
			P_SetAwaitMarker (P_GetStmtAwait (yyval._stmt), yyvsp[-1].i_val);
			P_SetAwaitDistance (P_GetStmtAwait (yyval._stmt), yyvsp[0].i_val);
		}
    break;

  case 192:
#line 1625 "parse.y"
    {
			yyval._stmt = P_NewStmt ();
			P_SetStmtType (yyval._stmt, ST_PARLOOP);
			P_SetStmtParLoop (yyval._stmt, P_NewParLoop ());
			P_SetParLoopLoopType (P_GetStmtParLoop (yyval._stmt),
					      LT_DOSERIAL);
			P_SetParLoopIterationVar (P_GetStmtParLoop (yyval._stmt), yyvsp[-7]._expr);
			P_SetParLoopInitValue (P_GetStmtParLoop (yyval._stmt), yyvsp[-5]._expr);
			P_SetParLoopFinalValue (P_GetStmtParLoop (yyval._stmt), yyvsp[-3]._expr);
			P_SetParLoopIncrValue (P_GetStmtParLoop (yyval._stmt), yyvsp[-1]._expr);
			P_SetParLoopPstmt (P_GetStmtParLoop (yyval._stmt), yyvsp[0]._pstmt);
		}
    break;

  case 193:
#line 1638 "parse.y"
    {
			yyval._stmt = P_NewStmt ();
			P_SetStmtType (yyval._stmt, ST_PARLOOP);
			P_SetStmtParLoop (yyval._stmt, P_NewParLoop ());
			P_SetParLoopLoopType (P_GetStmtParLoop (yyval._stmt), LT_DOALL);
			P_SetParLoopIterationVar (P_GetStmtParLoop (yyval._stmt), yyvsp[-7]._expr);
			P_SetParLoopInitValue (P_GetStmtParLoop (yyval._stmt), yyvsp[-5]._expr);
			P_SetParLoopFinalValue (P_GetStmtParLoop (yyval._stmt), yyvsp[-3]._expr);
			P_SetParLoopIncrValue (P_GetStmtParLoop (yyval._stmt), yyvsp[-1]._expr);
			P_SetParLoopPstmt (P_GetStmtParLoop (yyval._stmt), yyvsp[0]._pstmt);
		}
    break;

  case 194:
#line 1650 "parse.y"
    {
			yyval._stmt = P_NewStmt ();
			P_SetStmtType (yyval._stmt, ST_PARLOOP);
			P_SetStmtParLoop (yyval._stmt, P_NewParLoop ());
			P_SetParLoopLoopType (P_GetStmtParLoop (yyval._stmt),
					      LT_DOACROSS);
			P_SetParLoopIterationVar (P_GetStmtParLoop (yyval._stmt), yyvsp[-7]._expr);
			P_SetParLoopInitValue (P_GetStmtParLoop (yyval._stmt), yyvsp[-5]._expr);
			P_SetParLoopFinalValue (P_GetStmtParLoop (yyval._stmt), yyvsp[-3]._expr);
			P_SetParLoopIncrValue (P_GetStmtParLoop (yyval._stmt), yyvsp[-1]._expr);
			P_SetParLoopPstmt (P_GetStmtParLoop (yyval._stmt), yyvsp[0]._pstmt);
		}
    break;

  case 195:
#line 1663 "parse.y"
    {
			yyval._stmt = P_NewStmt ();
			P_SetStmtType (yyval._stmt, ST_PARLOOP);
			P_SetStmtParLoop (yyval._stmt, P_NewParLoop ());
			P_SetParLoopLoopType (P_GetStmtParLoop (yyval._stmt),
					      LT_DOSUPER);
			P_SetParLoopIterationVar (P_GetStmtParLoop (yyval._stmt), yyvsp[-7]._expr);
			P_SetParLoopInitValue (P_GetStmtParLoop (yyval._stmt), yyvsp[-5]._expr);
			P_SetParLoopFinalValue (P_GetStmtParLoop (yyval._stmt), yyvsp[-3]._expr);
			P_SetParLoopIncrValue (P_GetStmtParLoop (yyval._stmt), yyvsp[-1]._expr);
			P_SetParLoopPstmt (P_GetStmtParLoop (yyval._stmt), yyvsp[0]._pstmt);
		}
    break;

  case 196:
#line 1676 "parse.y"
    {
			yyval._stmt = P_NewStmt ();
			P_SetStmtType (yyval._stmt, ST_MUTEX);
			P_SetStmtMutex (yyval._stmt, P_NewMutex ());
			P_SetMutexExpression (P_GetStmtMutex (yyval._stmt), yyvsp[-1]._expr);
			P_SetMutexStatement (P_GetStmtMutex (yyval._stmt), yyvsp[0]._stmt);
		}
    break;

  case 197:
#line 1684 "parse.y"
    {
			yyval._stmt = P_NewStmt ();
			P_SetStmtType (yyval._stmt, ST_COBEGIN);
			P_SetStmtCobegin (yyval._stmt, P_NewCobegin ());
			P_SetCobeginStatements (P_GetStmtCobegin (yyval._stmt), yyvsp[0]._stmt);
		}
    break;

  case 198:
#line 1691 "parse.y"
    {
			yyval._stmt = P_NewStmt ();
			P_SetStmtType (yyval._stmt, ST_ASM);
			P_SetStmtAsmStmt (yyval._stmt, yyvsp[0]._asmstmt);
		}
    break;

  case 199:
#line 1697 "parse.y"
    {
			yyval._stmt = P_NewStmt ();
			P_SetStmtType (yyval._stmt, ST_BREAK);
		}
    break;

  case 200:
#line 1702 "parse.y"
    {
			yyval._stmt = P_NewStmt ();
			P_SetStmtType (yyval._stmt, ST_CONT);
		}
    break;

  case 201:
#line 1707 "parse.y"
    {
			yyval._stmt = P_NewStmt ();
			P_SetStmtType (yyval._stmt, ST_NOOP);
		}
    break;

  case 202:
#line 1714 "parse.y"
    {
			yyval._stmt = NULL;
		}
    break;

  case 203:
#line 1718 "parse.y"
    {
			yyval._stmt = P_AppendStmtLexNext (yyvsp[-1]._stmt, yyvsp[0]._stmt);
		}
    break;

  case 204:
#line 1724 "parse.y"
    {
			yyval._stmt = yyvsp[-1]._stmt;
		}
    break;

  case 205:
#line 1731 "parse.y"
    {
			yyval._structqual = SQ_EMPTY;
		}
    break;

  case 206:
#line 1735 "parse.y"
    {
			yyval._structqual = SQ_INCOMPLETE;
		}
    break;

  case 207:
#line 1739 "parse.y"
    {
			yyval._structqual = SQ_UNNAMED;
		}
    break;

  case 208:
#line 1743 "parse.y"
    {
			yyval._structqual = SQ_LINKMULTI;
		}
    break;

  case 209:
#line 1749 "parse.y"
    {
			yyval._structqual = 0;
		}
    break;

  case 210:
#line 1753 "parse.y"
    {
			yyval._structqual = yyvsp[-1]._structqual | yyvsp[0]._structqual;
		}
    break;

  case 211:
#line 1759 "parse.y"
    {
			yyval._symboltable = P_NewSymbolTable ();
			P_SetSymbolTableNumFiles (yyval._symboltable, yyvsp[-2].i_val);
			P_SetSymbolTableIPTable \
			  (yyval._symboltable, malloc (sizeof (_IPSymTabEnt *) * (yyvsp[-2].i_val + 1)));
			P_SetSymbolTableFlags (yyval._symboltable, yyvsp[-1]._stflags);
		}
    break;

  case 212:
#line 1767 "parse.y"
    {
			yyval._symboltable = NULL;
		}
    break;

  case 213:
#line 1773 "parse.y"
    {
			yyval._symtabentry = NULL;
		}
    break;

  case 214:
#line 1777 "parse.y"
    {
			yyval._symtabentry = P_NewSymTabEntry ();
			P_SetSymTabEntryType (yyval._symtabentry, ET_BLOCK);
			P_SetSymTabEntryBlockStart (yyval._symtabentry, yyvsp[-2]._key);
			P_SetSymTabEntryBlockSize (yyval._symtabentry, yyvsp[-1].i_val);
		}
    break;

  case 215:
#line 1784 "parse.y"
    {
			yyval._symtabentry = P_NewSymTabEntry ();
			P_SetSymTabEntryIdentifier (yyval._symtabentry, yyvsp[-6]._identifier);
			yyvsp[-6]._identifier = P_RemoveIdentifier (yyvsp[-6]._identifier);
			P_SetSymTabEntryScopeKey (yyval._symtabentry, yyvsp[-5]._key);
			P_SetSymTabEntryType (yyval._symtabentry, yyvsp[-4]._entrytype);
			P_SetSymTabEntryOffset (yyval._symtabentry, yyvsp[-2].i_val);
			P_SetSymTabEntryPragma (yyval._symtabentry, yyvsp[-1]._pragma);

			if (Handlers[ES_SYMTABENTRY])
				P_ExtRead (ES_SYMTABENTRY, (void *)yyval._symtabentry);
		}
    break;

  case 216:
#line 1797 "parse.y"
    {
			yyval._symtabentry = P_NewSymTabEntry ();
			P_SetSymTabEntryIdentifier (yyval._symtabentry, yyvsp[-7]._identifier);
			yyvsp[-7]._identifier = P_RemoveIdentifier (yyvsp[-7]._identifier);
			P_SetSymTabEntryScopeKey (yyval._symtabentry, yyvsp[-6]._key);
			P_SetSymTabEntryType (yyval._symtabentry, yyvsp[-5]._entrytype);
			P_SetSymTabEntryScope (yyval._symtabentry, yyvsp[-4]._scope);
			P_SetSymTabEntryOffset (yyval._symtabentry, yyvsp[-2].i_val);
			P_SetSymTabEntryPragma (yyval._symtabentry, yyvsp[-1]._pragma);

			if (Handlers[ES_SYMTABENTRY])
				P_ExtRead (ES_SYMTABENTRY, (void *)yyval._symtabentry);
		}
    break;

  case 217:
#line 1814 "parse.y"
    {
			yyval._stflags = STF_LINKED;
		}
    break;

  case 218:
#line 1820 "parse.y"
    {
			yyval._stflags = 0;
		}
    break;

  case 219:
#line 1824 "parse.y"
    {
			yyval._stflags = yyvsp[-1]._stflags | yyvsp[0]._stflags;
		}
    break;

  case 220:
#line 1830 "parse.y"
    {
			yyval._dcl = yyvsp[-1]._dcl;
		}
    break;

  case 221:
#line 1837 "parse.y"
    {
			yyval._dcl = P_NewDcl ();
			P_SetDclType (yyval._dcl, TT_TYPE);
			P_SetDclTypeDcl (yyval._dcl, P_NewTypeDcl ());
			P_SetTypeDclIdentifier (P_GetDclTypeDcl (yyval._dcl), yyvsp[-8]._identifier);
			yyvsp[-8]._identifier = P_RemoveIdentifier (yyvsp[-8]._identifier);
			P_SetTypeDclQualifier (P_GetDclTypeDcl (yyval._dcl), yyvsp[-7]._typequal);
			P_SetTypeDclAlignment (P_GetDclTypeDcl (yyval._dcl), yyvsp[-6].i_val);
			P_SetTypeDclRefs (P_GetDclTypeDcl (yyval._dcl), yyvsp[-4].i_val);
			P_SetTypeDclBasicType (P_GetDclTypeDcl (yyval._dcl),
					       BT_TYPEDEF_E);
			P_SetTypeDclType (P_GetDclTypeDcl (yyval._dcl), yyvsp[-3]._key);
			P_SetTypeDclPragma (P_GetDclTypeDcl (yyval._dcl), yyvsp[-2]._pragma);
			if (yyvsp[-1]._position)
			{
				P_SetTypeDclPosition (P_GetDclTypeDcl (yyval._dcl),
						      yyvsp[-1]._position);
				yyvsp[-1]._position = P_RemovePosition (yyvsp[-1]._position);
			}

			if (Handlers[ES_TYPE])
				P_ExtRead (ES_TYPE,
					   (void *)P_GetDclTypeDcl (yyval._dcl));
		}
    break;

  case 222:
#line 1862 "parse.y"
    {
			yyval._dcl = P_NewDcl ();
			P_SetDclType (yyval._dcl, TT_TYPE);
			P_SetDclTypeDcl (yyval._dcl, yyvsp[-1]._typedcl);
		}
    break;

  case 223:
#line 1869 "parse.y"
    {
			yyval._dcl = P_NewDcl ();
			P_SetDclType (yyval._dcl, TT_STRUCT);
			P_SetDclStructDcl (yyval._dcl, P_NewStructDcl ());
			P_SetStructDclIdentifier (P_GetDclStructDcl (yyval._dcl), yyvsp[-5]._identifier);
			yyvsp[-5]._identifier = P_RemoveIdentifier (yyvsp[-5]._identifier);
			P_SetStructDclFields (P_GetDclStructDcl (yyval._dcl), yyvsp[-4]._field);
			P_SetStructDclQualifier (P_GetDclStructDcl (yyval._dcl), yyvsp[-3]._structqual);
			P_SetStructDclPragma (P_GetDclStructDcl (yyval._dcl), yyvsp[-2]._pragma);
			if (yyvsp[-1]._position)
			{
				P_SetStructDclPosition (P_GetDclStructDcl (yyval._dcl),
							yyvsp[-1]._position);
				yyvsp[-1]._position = P_RemovePosition (yyvsp[-1]._position);
			}

			if (Handlers[ES_STRUCT])
				P_ExtRead (ES_STRUCT,
					   (void *)P_GetDclStructDcl (yyval._dcl));
		}
    break;

  case 224:
#line 1891 "parse.y"
    {
			yyval._dcl = P_NewDcl ();
			P_SetDclType (yyval._dcl, TT_UNION);
			P_SetDclUnionDcl (yyval._dcl, P_NewUnionDcl ());
			P_SetUnionDclIdentifier (P_GetDclUnionDcl (yyval._dcl), yyvsp[-5]._identifier);
			yyvsp[-5]._identifier = P_RemoveIdentifier (yyvsp[-5]._identifier);
			P_SetUnionDclFields (P_GetDclUnionDcl (yyval._dcl), yyvsp[-4]._field);
			P_SetUnionDclQualifier (P_GetDclUnionDcl (yyval._dcl), yyvsp[-3]._structqual);
			P_SetUnionDclPragma (P_GetDclUnionDcl (yyval._dcl), yyvsp[-2]._pragma);
			if (yyvsp[-1]._position)
			{
				P_SetUnionDclPosition (P_GetDclUnionDcl (yyval._dcl),
						       yyvsp[-1]._position);
				yyvsp[-1]._position = P_RemovePosition (yyvsp[-1]._position);
			}

			if (Handlers[ES_UNION])
				P_ExtRead (ES_UNION,
					   (void *)P_GetDclUnionDcl (yyval._dcl));
		}
    break;

  case 225:
#line 1912 "parse.y"
    {
			yyval._dcl = P_NewDcl ();
			P_SetDclType (yyval._dcl, TT_ENUM);
			P_SetDclEnumDcl (yyval._dcl, P_NewEnumDcl ());
			P_SetEnumDclIdentifier (P_GetDclEnumDcl (yyval._dcl), yyvsp[-4]._identifier);
			yyvsp[-4]._identifier = P_RemoveIdentifier (yyvsp[-4]._identifier);
			P_SetEnumDclFields (P_GetDclEnumDcl (yyval._dcl), yyvsp[-3]._enumfield);
			P_SetEnumDclPragma (P_GetDclEnumDcl (yyval._dcl), yyvsp[-2]._pragma);
			if (yyvsp[-1]._position)
			{
				P_SetEnumDclPosition (P_GetDclEnumDcl (yyval._dcl),
						      yyvsp[-1]._position);
				yyvsp[-1]._position = P_RemovePosition (yyvsp[-1]._position);
			}

			if (Handlers[ES_ENUM])
				P_ExtRead (ES_ENUM,
					   (void *)P_GetDclEnumDcl (yyval._dcl));
		}
    break;

  case 226:
#line 1934 "parse.y"
    {
			yyval._typelist = NULL;
		}
    break;

  case 227:
#line 1938 "parse.y"
    {
		        yyval._typelist = List_insert_last (yyvsp[-1]._typelist, yyvsp[0]._typedcl);
		}
    break;

  case 228:
#line 1944 "parse.y"
    {
			yyval._typelist = yyvsp[-1]._typelist;
		}
    break;

  case 229:
#line 1950 "parse.y"
    {
			yyval._typequal = TY_CONST;
		}
    break;

  case 230:
#line 1954 "parse.y"
    {
			yyval._typequal = TY_VOLATILE;
		}
    break;

  case 231:
#line 1958 "parse.y"
    {
			yyval._typequal = TY_SYNC;
		}
    break;

  case 232:
#line 1962 "parse.y"
    {
			yyval._typequal = TY_IMPLICIT;
		}
    break;

  case 233:
#line 1966 "parse.y"
    {
			yyval._typequal = TY_DEFAULT;
		}
    break;

  case 234:
#line 1970 "parse.y"
    {
			yyval._typequal = TY_EXP_ALIGN;
		}
    break;

  case 235:
#line 1974 "parse.y"
    {
			yyval._typequal = TY_UNNAMED;
		}
    break;

  case 236:
#line 1980 "parse.y"
    {	
			yyval._typequal = 0;
		}
    break;

  case 237:
#line 1984 "parse.y"
    {
			yyval._typequal = yyvsp[-1]._typequal | yyvsp[0]._typequal;
		}
    break;

  case 238:
#line 1991 "parse.y"
    {
			yyval._typedcl = P_NewTypeDcl ();
			P_SetTypeDclKey (yyval._typedcl, yyvsp[-9]._key);
			P_SetTypeDclQualifier (yyval._typedcl, yyvsp[-8]._typequal);
			P_SetTypeDclBasicType (yyval._typedcl, yyvsp[-7]._basictype);
			P_SetTypeDclSize (yyval._typedcl, yyvsp[-6].i_val);
			P_SetTypeDclAlignment (yyval._typedcl, yyvsp[-5].i_val);
			P_SetTypeDclRefs (yyval._typedcl, yyvsp[-3].i_val);
			P_SetTypeDclPragma (yyval._typedcl, yyvsp[-2]._pragma);
			if (yyvsp[-1]._position)
			{
				P_SetTypeDclPosition (yyval._typedcl, yyvsp[-1]._position);
				yyvsp[-1]._position = P_RemovePosition (yyvsp[-1]._position);
			}

			if (Handlers[ES_TYPE])
				P_ExtRead (ES_TYPE, (void *)yyval._typedcl);
		}
    break;

  case 239:
#line 2011 "parse.y"
    {
			yyval._typedcl = P_NewTypeDcl ();
			P_SetTypeDclBasicType (yyval._typedcl, yyvsp[-11]._basictype);
			P_SetTypeDclIdentifier (yyval._typedcl, yyvsp[-10]._identifier);
			yyvsp[-10]._identifier = P_RemoveIdentifier (yyvsp[-10]._identifier);
			P_SetTypeDclQualifier (yyval._typedcl, yyvsp[-9]._typequal);
			P_SetTypeDclType (yyval._typedcl, yyvsp[-7]._key);
			P_SetTypeDclSize (yyval._typedcl, yyvsp[-6].i_val);
			P_SetTypeDclAlignment (yyval._typedcl, yyvsp[-5].i_val);
			P_SetTypeDclRefs (yyval._typedcl, yyvsp[-3].i_val);
			P_SetTypeDclPragma (yyval._typedcl, yyvsp[-2]._pragma);
			if (yyvsp[-1]._position)
			{
				P_SetTypeDclPosition (yyval._typedcl, yyvsp[-1]._position);
				yyvsp[-1]._position = P_RemovePosition (yyvsp[-1]._position);
			}

			if (Handlers[ES_TYPE])
				P_ExtRead (ES_TYPE, (void *)yyval._typedcl);
		}
    break;

  case 240:
#line 2033 "parse.y"
    {
			yyval._typedcl = P_NewTypeDcl ();
			P_SetTypeDclKey (yyval._typedcl, yyvsp[-13]._key);
			P_SetTypeDclQualifier (yyval._typedcl, yyvsp[-12]._typequal);
			P_SetTypeDclBasicType (yyval._typedcl, BT_ARRAY);
			P_SetTypeDclType (yyval._typedcl, yyvsp[-9]._key);
			P_SetTypeDclArraySize (yyval._typedcl, yyvsp[-7]._expr);
			P_SetTypeDclSize (yyval._typedcl, yyvsp[-6].i_val);
			P_SetTypeDclAlignment (yyval._typedcl, yyvsp[-5].i_val);
			P_SetTypeDclRefs (yyval._typedcl, yyvsp[-3].i_val);
			P_SetTypeDclPragma (yyval._typedcl, yyvsp[-2]._pragma);
			if (yyvsp[-1]._position)
			{
				P_SetTypeDclPosition (yyval._typedcl, yyvsp[-1]._position);
				yyvsp[-1]._position = P_RemovePosition (yyvsp[-1]._position);
			}

			if (Handlers[ES_TYPE])
				P_ExtRead (ES_TYPE, (void *)yyval._typedcl);
		}
    break;

  case 241:
#line 2055 "parse.y"
    {
			yyval._typedcl = P_NewTypeDcl ();
			P_SetTypeDclKey (yyval._typedcl, yyvsp[-14]._key);
			P_SetTypeDclQualifier (yyval._typedcl, yyvsp[-13]._typequal);
			P_SetTypeDclBasicType (yyval._typedcl, BT_FUNC);
			P_SetTypeDclName (yyval._typedcl, P_DQString2String (yyvsp[-11].st_val));
			free (yyvsp[-11].st_val);
			P_SetTypeDclType (yyval._typedcl, yyvsp[-8]._key);
			P_SetTypeDclParam (yyval._typedcl, yyvsp[-7]._param);
			P_SetTypeDclSize (yyval._typedcl, yyvsp[-6].i_val);
			P_SetTypeDclAlignment (yyval._typedcl, yyvsp[-5].i_val);
			P_SetTypeDclRefs (yyval._typedcl, yyvsp[-3].i_val);
			P_SetTypeDclPragma (yyval._typedcl, yyvsp[-2]._pragma);
			if (yyvsp[-1]._position)
			{
				P_SetTypeDclPosition (yyval._typedcl, yyvsp[-1]._position);
				yyvsp[-1]._position = P_RemovePosition (yyvsp[-1]._position);
			}

			if (Handlers[ES_TYPE])
				P_ExtRead (ES_TYPE, (void *)yyval._typedcl);
		}
    break;

  case 242:
#line 2079 "parse.y"
    {
			yyval._typedcl = P_NewTypeDcl ();
			P_SetTypeDclKey (yyval._typedcl, yyvsp[-11]._key);
			P_SetTypeDclQualifier (yyval._typedcl, yyvsp[-10]._typequal);
			P_SetTypeDclBasicType (yyval._typedcl, BT_POINTER);
			P_SetTypeDclType (yyval._typedcl, yyvsp[-7]._key);
			P_SetTypeDclSize (yyval._typedcl, yyvsp[-6].i_val);
			P_SetTypeDclAlignment (yyval._typedcl, yyvsp[-5].i_val);
			P_SetTypeDclRefs (yyval._typedcl, yyvsp[-3].i_val);
			P_SetTypeDclPragma (yyval._typedcl, yyvsp[-2]._pragma);
			if (yyvsp[-1]._position)
			{
				P_SetTypeDclPosition (yyval._typedcl, yyvsp[-1]._position);
				yyvsp[-1]._position = P_RemovePosition (yyvsp[-1]._position);
			}

			if (Handlers[ES_TYPE])
				P_ExtRead (ES_TYPE, (void *)yyval._typedcl);
		}
    break;

  case 243:
#line 2100 "parse.y"
    {
			yyval._typedcl = P_NewTypeDcl ();
			P_SetTypeDclKey (yyval._typedcl, yyvsp[-10]._key);
			P_SetTypeDclQualifier (yyval._typedcl, yyvsp[-9]._typequal);
			P_SetTypeDclBasicType (yyval._typedcl, BT_TYPEDEF_I);
			P_SetTypeDclType (yyval._typedcl, yyvsp[-6]._key);
			P_SetTypeDclAlignment (yyval._typedcl, yyvsp[-5].i_val);
			P_SetTypeDclRefs (yyval._typedcl, yyvsp[-3].i_val);
			P_SetTypeDclPragma (yyval._typedcl, yyvsp[-2]._pragma);
			if (yyvsp[-1]._position)
			{
				P_SetTypeDclPosition (yyval._typedcl, yyvsp[-1]._position);
				yyvsp[-1]._position = P_RemovePosition (yyvsp[-1]._position);
			}

			if (Handlers[ES_TYPE])
				P_ExtRead (ES_TYPE, (void *)yyval._typedcl);
		}
    break;

  case 244:
#line 2121 "parse.y"
    {
			yyval._vardcl = P_NewVarDcl ();
			P_SetVarDclIdentifier (yyval._vardcl, yyvsp[-6]._identifier);
			yyvsp[-6]._identifier = P_RemoveIdentifier (yyvsp[-6]._identifier);
			P_SetVarDclType (yyval._vardcl, yyvsp[-4]._key);
			P_SetVarDclQualifier (yyval._vardcl, yyvsp[-3]._varqual);
			P_SetVarDclPragma (yyval._vardcl, yyvsp[-2]._pragma);
			if (yyvsp[-1]._position)
			{
				P_SetVarDclPosition (yyval._vardcl, yyvsp[-1]._position);
				yyvsp[-1]._position = P_RemovePosition (yyvsp[-1]._position);
			}

			if (Handlers[ES_VAR])
				P_ExtRead (ES_VAR, (void *)yyval._vardcl);
		}
    break;

  case 245:
#line 2139 "parse.y"
    {
			yyval._vardcl = P_NewVarDcl ();
			P_SetVarDclIdentifier (yyval._vardcl, yyvsp[-7]._identifier);
			yyvsp[-7]._identifier = P_RemoveIdentifier (yyvsp[-7]._identifier);
			P_SetVarDclType (yyval._vardcl, yyvsp[-5]._key);
			P_SetVarDclQualifier (yyval._vardcl, yyvsp[-4]._varqual);
			P_SetVarDclInit (yyval._vardcl, yyvsp[-3]._init);
			P_SetVarDclPragma (yyval._vardcl, yyvsp[-2]._pragma);
			if (yyvsp[-1]._position)
			{
				P_SetVarDclPosition (yyval._vardcl, yyvsp[-1]._position);
				yyvsp[-1]._position = P_RemovePosition (yyvsp[-1]._position);
			}

			if (Handlers[ES_VAR])
				P_ExtRead (ES_VAR, (void *)yyval._vardcl);
		}
    break;

  case 246:
#line 2159 "parse.y"
    {
			yyval._varlist = NULL;
		}
    break;

  case 247:
#line 2163 "parse.y"
    {
			yyval._varlist = List_insert_last (yyvsp[-1]._varlist, yyvsp[0]._vardcl);
		}
    break;

  case 248:
#line 2169 "parse.y"
    {
			yyval._varlist = yyvsp[-1]._varlist;
		}
    break;

  case 249:
#line 2175 "parse.y"
    {
			yyval._varqual = VQ_DEFINED;
		}
    break;

  case 250:
#line 2179 "parse.y"
    {
			yyval._varqual = VQ_COMMON;
		}
    break;

  case 251:
#line 2183 "parse.y"
    {
			yyval._varqual = VQ_REGISTER;
		}
    break;

  case 252:
#line 2187 "parse.y"
    {
			yyval._varqual = VQ_AUTO;
		}
    break;

  case 253:
#line 2191 "parse.y"
    {
			yyval._varqual = VQ_STATIC;
		}
    break;

  case 254:
#line 2195 "parse.y"
    {
			yyval._varqual = VQ_EXTERN;
		}
    break;

  case 255:
#line 2199 "parse.y"
    {
			yyval._varqual = VQ_GLOBAL;
		}
    break;

  case 256:
#line 2203 "parse.y"
    {
			yyval._varqual = VQ_PARAMETER;
		}
    break;

  case 257:
#line 2207 "parse.y"
    {
			yyval._varqual = VQ_IMPLICIT;
		}
    break;

  case 258:
#line 2211 "parse.y"
    {
			yyval._varqual = VQ_CDECL;
		}
    break;

  case 259:
#line 2215 "parse.y"
    {
			yyval._varqual = VQ_STDCALL;
		}
    break;

  case 260:
#line 2219 "parse.y"
    {
			yyval._varqual = VQ_FASTCALL;
		}
    break;

  case 261:
#line 2223 "parse.y"
    {
			yyval._varqual = VQ_WEAK;
		}
    break;

  case 262:
#line 2227 "parse.y"
    {
			yyval._varqual = VQ_COMDAT;
		}
    break;

  case 263:
#line 2231 "parse.y"
    {
			yyval._varqual = VQ_CONSTRUCTOR;
		}
    break;

  case 264:
#line 2235 "parse.y"
    {
			yyval._varqual = VQ_DESTRUCTOR;
		}
    break;

  case 265:
#line 2239 "parse.y"
    {
			yyval._varqual = VQ_APP_ELLIPSIS;
		}
    break;

  case 266:
#line 2243 "parse.y"
    {
			yyval._varqual = VQ_OLD_PARAM;
		}
    break;

  case 267:
#line 2249 "parse.y"
    {
			yyval._varqual = 0;
		}
    break;

  case 268:
#line 2253 "parse.y"
    {
			yyval._varqual = yyvsp[-1]._varqual | yyvsp[0]._varqual;
		}
    break;


    }

/* Line 1000 of yacc.c.  */
#line 4727 "src/Pcode/Pcode/parse.c"

  yyvsp -= yylen;
  yyssp -= yylen;


  YY_STACK_PRINT (yyss, yyssp);

  *++yyvsp = yyval;


  /* Now `shift' the result of the reduction.  Determine what state
     that goes to, based on the state we popped back to and the rule
     number reduced by.  */

  yyn = yyr1[yyn];

  yystate = yypgoto[yyn - YYNTOKENS] + *yyssp;
  if (0 <= yystate && yystate <= YYLAST && yycheck[yystate] == *yyssp)
    yystate = yytable[yystate];
  else
    yystate = yydefgoto[yyn - YYNTOKENS];

  goto yynewstate;


/*------------------------------------.
| yyerrlab -- here on detecting error |
`------------------------------------*/
yyerrlab:
  /* If not already recovering from an error, report this error.  */
  if (!yyerrstatus)
    {
      ++yynerrs;
#if YYERROR_VERBOSE
      yyn = yypact[yystate];

      if (YYPACT_NINF < yyn && yyn < YYLAST)
	{
	  YYSIZE_T yysize = 0;
	  int yytype = YYTRANSLATE (yychar);
	  const char* yyprefix;
	  char *yymsg;
	  int yyx;

	  /* Start YYX at -YYN if negative to avoid negative indexes in
	     YYCHECK.  */
	  int yyxbegin = yyn < 0 ? -yyn : 0;

	  /* Stay within bounds of both yycheck and yytname.  */
	  int yychecklim = YYLAST - yyn;
	  int yyxend = yychecklim < YYNTOKENS ? yychecklim : YYNTOKENS;
	  int yycount = 0;

	  yyprefix = ", expecting ";
	  for (yyx = yyxbegin; yyx < yyxend; ++yyx)
	    if (yycheck[yyx + yyn] == yyx && yyx != YYTERROR)
	      {
		yysize += yystrlen (yyprefix) + yystrlen (yytname [yyx]);
		yycount += 1;
		if (yycount == 5)
		  {
		    yysize = 0;
		    break;
		  }
	      }
	  yysize += (sizeof ("syntax error, unexpected ")
		     + yystrlen (yytname[yytype]));
	  yymsg = (char *) YYSTACK_ALLOC (yysize);
	  if (yymsg != 0)
	    {
	      char *yyp = yystpcpy (yymsg, "syntax error, unexpected ");
	      yyp = yystpcpy (yyp, yytname[yytype]);

	      if (yycount < 5)
		{
		  yyprefix = ", expecting ";
		  for (yyx = yyxbegin; yyx < yyxend; ++yyx)
		    if (yycheck[yyx + yyn] == yyx && yyx != YYTERROR)
		      {
			yyp = yystpcpy (yyp, yyprefix);
			yyp = yystpcpy (yyp, yytname[yyx]);
			yyprefix = " or ";
		      }
		}
	      yyerror (yymsg);
	      YYSTACK_FREE (yymsg);
	    }
	  else
	    yyerror ("syntax error; also virtual memory exhausted");
	}
      else
#endif /* YYERROR_VERBOSE */
	yyerror ("syntax error");
    }



  if (yyerrstatus == 3)
    {
      /* If just tried and failed to reuse lookahead token after an
	 error, discard it.  */

      if (yychar <= YYEOF)
        {
          /* If at end of input, pop the error token,
	     then the rest of the stack, then return failure.  */
	  if (yychar == YYEOF)
	     for (;;)
	       {
		 YYPOPSTACK;
		 if (yyssp == yyss)
		   YYABORT;
		 YYDSYMPRINTF ("Error: popping", yystos[*yyssp], yyvsp, yylsp);
		 yydestruct (yystos[*yyssp], yyvsp);
	       }
        }
      else
	{
	  YYDSYMPRINTF ("Error: discarding", yytoken, &yylval, &yylloc);
	  yydestruct (yytoken, &yylval);
	  yychar = YYEMPTY;

	}
    }

  /* Else will try to reuse lookahead token after shifting the error
     token.  */
  goto yyerrlab1;


/*---------------------------------------------------.
| yyerrorlab -- error raised explicitly by YYERROR.  |
`---------------------------------------------------*/
yyerrorlab:

#ifdef __GNUC__
  /* Pacify GCC when the user code never invokes YYERROR and the label
     yyerrorlab therefore never appears in user code.  */
  if (0)
     goto yyerrorlab;
#endif

  yyvsp -= yylen;
  yyssp -= yylen;
  yystate = *yyssp;
  goto yyerrlab1;


/*-------------------------------------------------------------.
| yyerrlab1 -- common code for both syntax error and YYERROR.  |
`-------------------------------------------------------------*/
yyerrlab1:
  yyerrstatus = 3;	/* Each real token shifted decrements this.  */

  for (;;)
    {
      yyn = yypact[yystate];
      if (yyn != YYPACT_NINF)
	{
	  yyn += YYTERROR;
	  if (0 <= yyn && yyn <= YYLAST && yycheck[yyn] == YYTERROR)
	    {
	      yyn = yytable[yyn];
	      if (0 < yyn)
		break;
	    }
	}

      /* Pop the current state because it cannot handle the error token.  */
      if (yyssp == yyss)
	YYABORT;

      YYDSYMPRINTF ("Error: popping", yystos[*yyssp], yyvsp, yylsp);
      yydestruct (yystos[yystate], yyvsp);
      YYPOPSTACK;
      yystate = *yyssp;
      YY_STACK_PRINT (yyss, yyssp);
    }

  if (yyn == YYFINAL)
    YYACCEPT;

  YYDPRINTF ((stderr, "Shifting error token, "));

  *++yyvsp = yylval;


  yystate = yyn;
  goto yynewstate;


/*-------------------------------------.
| yyacceptlab -- YYACCEPT comes here.  |
`-------------------------------------*/
yyacceptlab:
  yyresult = 0;
  goto yyreturn;

/*-----------------------------------.
| yyabortlab -- YYABORT comes here.  |
`-----------------------------------*/
yyabortlab:
  yyresult = 1;
  goto yyreturn;

#ifndef yyoverflow
/*----------------------------------------------.
| yyoverflowlab -- parser overflow comes here.  |
`----------------------------------------------*/
yyoverflowlab:
  yyerror ("parser stack overflow");
  yyresult = 2;
  /* Fall through.  */
#endif

yyreturn:
#ifndef yyoverflow
  if (yyss != yyssa)
    YYSTACK_FREE (yyss);
#endif
  return yyresult;
}


#line 2257 "parse.y"


void
yyerror (char *s)
{
	P_warn ("parse.y: %s on line %d, col %d before %s", s, last_line,
	        last_column, yytext);

	return;
}

