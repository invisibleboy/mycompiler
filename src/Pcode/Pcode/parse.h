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
/* Line 1275 of yacc.c.  */
#line 426 "src/Pcode/Pcode/parse.h"
# define yystype YYSTYPE /* obsolescent; will be withdrawn */
# define YYSTYPE_IS_DECLARED 1
# define YYSTYPE_IS_TRIVIAL 1
#endif

extern YYSTYPE yylval;



