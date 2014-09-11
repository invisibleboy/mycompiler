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
#include <library/FIX_libc.h>
# include <stdio.h>
# define U(x) x
# define NLSTATE yyprevious=YYNEWLINE
# define BEGIN yybgin = yysvec + 1 +
# define INITIAL 0
# define YYLERR yysvec
# define YYSTATE (yyestate-yysvec-1)
# define YYOPTIM 1
# define YYLMAX 200
# define output(c) (void)putc(c,yyout)
#if defined(__cplusplus) || defined(__STDC__)
#if defined(__cplusplus) && defined(__EXTERN_C__)
extern "C"
{
#endif
  int yyback (int *, int);
  int yyinput (void);
  int yylook (void);
  void yyoutput (int);
  int yyracc (int);
  int yyreject (void);
  void yyunput (int);
  int yylex (void);
#ifndef yyless
  void yyless (int);
#endif
#ifndef yywrap
  int yywrap (void);
#endif
#ifdef LEXDEBUG
  void allprint (char);
  void sprint (char *);
#endif
#if defined(__cplusplus) && defined(__EXTERN_C__)
}
#endif
#endif
# define input() (((yytchar=yysptr>yysbuf?U(*--yysptr):getc(yyin))==10?(yylineno++,yytchar):yytchar)==EOF?0:yytchar)
# define unput(c) {yytchar= (c);if(yytchar=='\n')yylineno--;*yysptr++=yytchar;}
# define yymore() (yymorfg=1)
# define ECHO (void)fprintf(yyout, "%s",yytext)
int yyleng;
extern char yytext[];
int yymorfg;
extern char *yysptr, yysbuf[];
int yytchar;
/* Initialization to stdin/stdout not portable.  Initialization
 * moved to Cl_open()! -JCG 5/99 
 *
 * FILE *yyin = {stdin}, *yyout = {stdout};
 */
FILE *yyin = NULL, *yyout = NULL;
extern int yylineno;
struct yysvf
{
  struct yywork *yystoff;
  struct yysvf *yyother;
  int *yystops;
};
struct yysvf *yyestate;
extern struct yysvf yysvec[], *yybgin;


/*****************************************************************************\
 *      File:   c_list.l
 *      Author: Po-hua Chang, Wen-mei Hwu
 *      Creation Date:  June 1990
 *      Modified By: XXX, date, time, why
\*****************************************************************************/


/*===========================================================================
 *      Description :   List interface.
 *==========================================================================*/
#include <stdio.h>
#include <ctype.h>
#include <stdlib.h>
#include <string.h>
#include <library/c_basic.h>
#include <library/c_list.h>

#undef YYLMAX
#define YYLMAX 5120

static void lex_punt (char *);
static int lexChar (void);
int yywrap (void);


/*
 *      In order to switch between files, we also
 *      need to save the buffer space too.
 */
static int
save_buffer (char line[], int len)
{
  int i, ln;
  ln = yysptr - yysbuf;         /* number of characters in the buffer */
  for (i = 0; i < ln; i++)
    {
      if (i >= len)
        lex_punt ("failed to switch between files due to long buffer");
      line[i] = yysbuf[i];
    }
  yysptr = yysbuf;              /* empty buffer */
  return ln;
}
static void
restore_buffer (char line[], int len)
{
  int i;
  yysptr = yysbuf;              /* empty buffer */
  for (i = 0; i < len; i++)
    {
      *yysptr = line[i];
      yysptr++;
    }
}

/*--------------------------------------------------------------------------*/

/*
 *      Macro definitions to make interface with lex less painful 
 */
extern char yytext[];
#define TokenBuffer(n) (yytext+(n))     /* current token in text form */
#define TokenLength yyleng      /* length of yytext */
#define TokenChar(n) (yytext[n])        /* Nth character of yytext */


/*
 *      Location of tokens.
 */
static char *currentFile = "";  /* current file */
static int currentLine = 1;     /* current row pointer */
static int currentColumn = 1;   /* current column pointer */

#define TokenFile       (currentFile)
static int TokenLine;
static int TokenColumn;

#define TabSize 8
#define lexStepRight(len)       {currentColumn += len;}
#define lexNewLine              {currentLine += 1; currentColumn = 1;}
#define lexTab  {currentColumn = ((currentColumn/TabSize)+1)*TabSize+1;}


/* 
 *      Definitions for the input buffer.
 *      These information will be destroyed as soon as
 *      the next token comes in.
 */
#define L_ERROR         260
#define L_ID            261
#define L_INT           262
#define L_REAL          263
#define L_STRING        264
#define L_CHAR          265
#define L_MACRO         266
#define L_EOLN          267

static int TokenType;           /* token type */
static C_String TokenName;      /* token string */
static C_Integer TokenInteger;  /* scalar value */
static C_Double TokenReal;      /* floating point value */

static int lex_error = 0;       /* number of lexical errors encountered */
static int in_macro = 0;

#define ToState(n) BEGIN(n)     /* change lex state */
#define DefaultState 0          /* default lex state */

# define COMMENT 2
# define YYNEWLINE 10
int
yylex (void)
{
  int nstr;
  while ((nstr = yylook ()) >= 0)
    switch (nstr)
      {
      case 0:
        if (yywrap ())
          return (0);
        break;
      case 1:

        {
          lexStepRight (TokenLength);
        }
        break;
      case 2:

        {
          lexTab;
        }
        break;
      case 3:

        {
          lexNewLine;
        }
        break;
      case 4:

        {
          lexStepRight (TokenLength);
          ToState (DefaultState);
        }
        break;
      case 5:

        {
          lexStepRight (TokenLength);
          lexTab;
        }
        break;
      case 6:

        {
          lexStepRight (TokenLength);
          lexNewLine;
        }
        break;
      case 7:

        {
          lexStepRight (TokenLength);
        }
        break;
      case 8:

        {
          lexStepRight (TokenLength);
          ToState (COMMENT);
        }
        break;
      case 9:

        {
          lexStepRight (TokenLength);
        }
        break;
      case 10:

        {
          lexStepRight (TokenLength);
        }
        break;
      case 11:

        {
          lexTab;
        }
        break;
      case 12:

        {
          if (in_macro)
            {
              in_macro = 0;
              TokenName = TokenBuffer (0);
              TokenLine = currentLine;
              TokenColumn = currentColumn;
              lexNewLine;
              return (TokenType = L_EOLN);
            }
          lexNewLine;
        }
        break;
      case 13:

        {
          lexStepRight (TokenLength);
        }
        break;
      case 14:

        {
          if (in_macro)
            lex_punt ("detected # when expecting \\n");
          in_macro = 1;
          TokenName = TokenBuffer (0);
          TokenLine = currentLine;
          TokenColumn = currentColumn;
          lexStepRight (TokenLength);
          return (TokenType = L_MACRO);
        }
        break;
      case 15:

        {
          /* Process a Hexdecimal Literal */
          TokenInteger = C_string_to_hex (TokenBuffer (0) + 2);
          TokenLine = currentLine;
          TokenColumn = currentColumn;
          lexStepRight (TokenLength);
          return (TokenType = L_INT);
        }
        break;
      case 16:

        {
          /* Process an Octal Literal */
          TokenInteger = C_string_to_oct (TokenBuffer (0));
          TokenLine = currentLine;
          TokenColumn = currentColumn;
          lexStepRight (TokenLength);
          return (TokenType = L_INT);
        }
        break;
      case 17:

        {
          /* Process an Integer Literal */
          TokenInteger = C_string_to_integer (TokenBuffer (0));
          TokenLine = currentLine;
          TokenColumn = currentColumn;
          lexStepRight (TokenLength);
          return (TokenType = L_INT);
        }
        break;
      case 18:

        {
          /* Process an Identifier/Typeid/Keyword/Flags */
          TokenName = TokenBuffer (0);
          TokenLine = currentLine;
          TokenColumn = currentColumn;
          lexStepRight (TokenLength);
          return (TokenType = L_ID);
        }
        break;
      case 19:

        {
          /* Process a Real Literal */
          TokenReal = C_string_to_double (TokenBuffer (0));
          TokenLine = currentLine;
          TokenColumn = currentColumn;
          lexStepRight (TokenLength);
          return (TokenType = L_REAL);
        }
        break;
      case 20:

        {
          /* Process a Real Literal */
          TokenReal = C_string_to_double (TokenBuffer (0));
          TokenLine = currentLine;
          TokenColumn = currentColumn;
          lexStepRight (TokenLength);
          return (TokenType = L_REAL);
        }
        break;
      case 21:

        {
          /* Process a Real Literal */
          TokenReal = C_string_to_double (TokenBuffer (0));
          TokenLine = currentLine;
          TokenColumn = currentColumn;
          lexStepRight (TokenLength);
          return (TokenType = L_REAL);
        }
        break;
      case 22:

        {
          /* Process a String Literal */
          TokenName = TokenBuffer (0);
          TokenLine = currentLine;
          TokenColumn = currentColumn;
          lexStepRight (TokenLength);
          return (TokenType = L_STRING);
        }
        break;
      case 23:

        {
          /* Process a Character Literal */
          TokenName = TokenBuffer (0);
          TokenLine = currentLine;
          TokenColumn = currentColumn;
          lexStepRight (TokenLength);
          return (TokenType = L_CHAR);
        }
        break;
      case 24:

        {
          return lexChar ();
        }
        break;
      case 25:

        {
          return lexChar ();
        }
        break;
      case 26:

        {
          return lexChar ();
        }
        break;
      case 27:

        {
          return lexChar ();
        }
        break;
      case 28:

        {
          return lexChar ();
        }
        break;
      case 29:

        {
          return lexChar ();
        }
        break;
      case 30:

        {
          return lexChar ();
        }
        break;
      case 31:

        {
          return lexChar ();
        }
        break;
      case 32:

        {
          return lexChar ();
        }
        break;
      case 33:

        {
          return lexChar ();
        }
        break;
      case 34:

        {
          return lexChar ();
        }
        break;
      case 35:

        {
          return lexChar ();
        }
        break;
      case 36:

        {
          return lexChar ();
        }
        break;
      case 37:

        {
          return lexChar ();
        }
        break;
      case 38:

        {
          return lexChar ();
        }
        break;
      case 39:

        {
          return lexChar ();
        }
        break;
      case 40:

        {
          return lexChar ();
        }
        break;
      case 41:

        {
          return lexChar ();
        }
        break;
      case 42:

        {
          return lexChar ();
        }
        break;
      case 43:

        {
          return lexChar ();
        }
        break;
      case 44:

        {
          return lexChar ();
        }
        break;
      case 45:

        {
          return lexChar ();
        }
        break;
      case 46:

        {
          return lexChar ();
        }
        break;
      case 47:

        {
          return lexChar ();
        }
        break;
      case 48:

        {
          return lexChar ();
        }
        break;
      case 49:

        {
          return lexChar ();
        }
        break;
      case 50:

        {
          return lexChar ();
        }
        break;
      case 51:

        {
          lexStepRight (TokenLength);
        }
        break;
      case 52:

        {
          TokenName = TokenBuffer (0);
          TokenLine = currentLine;
          TokenColumn = currentColumn;
          lexStepRight (TokenLength);
          lex_punt (TokenBuffer (0));
          return (TokenType = L_ERROR);
        }
        break;
      case -1:
        break;
      default:
        (void) fprintf (yyout, "bad switch yylook %d", nstr);
      }
  return (0);
}
/* end of yylex */

static int
lexChar (void)
{
  TokenName = TokenBuffer (0);
  TokenLine = currentLine;
  TokenColumn = currentColumn;
  lexStepRight (TokenLength);
  TokenType = TokenChar (0);
  return TokenChar (0);
}

/*----------------------------------------------------------------*/
int C_name_tbl = -1;

static void
INIT (void)
{
  if (C_name_tbl == -1)
    {
      C_name_tbl = C_open_name_table (Cl_MAX_STRING_TABLE_SIZE);
    }
  else
    {
      C_clear (C_name_tbl);
    }
}

#define SAVE(str)       C_unique_name(C_name_tbl, str)

/*
 *      3-18-1991
 *      The symbol table overflows because we enter
 *      too many integers and floats.
 */
#define SAVE2(str)      (str)

/*----------------------------------------------------------------*/
#define MAX_LEVEL       20
static int stack_top = 0;
static FILE *stack[MAX_LEVEL];
static char *fname[MAX_LEVEL];
static int fline[MAX_LEVEL];
static int fcolumn[MAX_LEVEL];

#define MAX_BUF_DATA_SIZE       1024
static int buf_length[MAX_LEVEL];
static char buf_data[MAX_LEVEL][MAX_BUF_DATA_SIZE];

/* Lexical Error */
static void
lex_punt (char *message)
{
  lex_error++;
  fprintf (stderr, "> lexical error : %s (file %s line %d column %d)\n",
           message, currentFile, currentLine, currentColumn);
  exit (-1);
}
static void
push_file (void)
{
  stack[stack_top] = yyin;
  fname[stack_top] = currentFile;
  fline[stack_top] = currentLine;
  fcolumn[stack_top] = currentColumn;
  buf_length[stack_top] =
    save_buffer (buf_data[stack_top], MAX_BUF_DATA_SIZE);
  currentFile = "???";
  currentLine = 1;
  currentColumn = 1;
  stack_top += 1;
  if (stack_top >= MAX_LEVEL)
    lex_punt ("too many levels of file inclusion");
}
static void
pop_file (void)
{
  if (stack_top <= 0)
    {
      lex_punt ("pop_file: stack underflows");
    }
  stack_top -= 1;
  yyin = stack[stack_top];
  currentLine = fline[stack_top];
  currentColumn = fcolumn[stack_top];
  currentFile = fname[stack_top];
  restore_buffer (buf_data[stack_top], buf_length[stack_top]);
}
#if 0
static char *
last_file (void)
{
  if (stack_top <= 0)
    return "??";                /* default */
  return fname[stack_top - 1];
}
#endif
/*
 * 1 for no wrap around the input file. 
 */
int
yywrap (void)
{
  return 1;
}
/*-------------------------------------------------------------------------*/
/*
 *      EXPORT FUNCTIONS.
 */
/* 
 *      Set Up The Input File.
 */
int
Cl_open (char *file_name, int inclusion)
{
  FILE *fpt;
  char *last_file;
  INIT ();
  push_file ();         /** save the previous fpt **/
  last_file = currentFile;

  /* Moved yyin and yyout initialization here. -JCG 5/99 */
  yyin = stdin;
  yyout = stdout;

  /*
   *  If the current_file is specified from the
   *  root directory, do nothing.
   *  Otherwise, add the prefix of the last file
   *  to the front of the file_name.
   */
  if (inclusion && (last_file != 0) && (file_name[0] != '/'))
    {
      int len, j, i;
      char temp1[1024];
      len = strlen (last_file);
      /* find the prefix: /x/y/z -> /x/y */
      for (i = len - 1; i >= 0; i--)
        if (last_file[i] == '/')
          break;
      for (j = 0; j <= i; j++)
        temp1[j] = last_file[j];
      for (i = 0; i <= strlen (file_name); i++)
        temp1[i + j] = file_name[i];
      file_name = SAVE (temp1);
    }
  currentFile = SAVE (file_name);  /** we have a new file **/
  if (!strcmp (file_name, "stdin"))
    {
      yyin = stdin;
      return 1;
    }
  else
    {                           /* open new file */
      fpt = fopen (file_name, "r");
      if (fpt == NULL)
        {
          /*
           *  Since we have already save the state
           *  of the last file, we cannot simply
           *  return to the caller. If future
           *  versions requires this to return,
           *  need to close up the file first.
           */
          fprintf (stderr, "> %s\n", file_name);
          lex_punt ("can not open file");
          Cl_close (file_name);
          return 0;
        }
      yyin = fpt;
      return 1;
    }
}
/* 
 *      Close an Input File.
 */
int
Cl_close (char *file_name)
{
  if (strcmp (file_name, "stdin"))
    fclose (yyin);
  pop_file ();
  return 1;
}
/* 
 *      Seek to a specific offset within a file
 */
int
Cl_seek (int offset)
{
  return fseek (yyin, offset, 0);
}
/* 
 *      Return the current location within the current file
 */
int
Cl_tell (int offset)
{
  return ftell (yyin);
}
/*
 *      Allocate space for a new node.  
 *      Manage a garbage list automatically to reduce
 *      the number of calls to malloc.
 */
static Cl_List dead_list = 0;
static Cl_List
new_node (void)
{
  Cl_List new_list;
  if (dead_list != 0)
    {
      new_list = dead_list;
      dead_list = Cl_SiblingOf (dead_list);
    }
  else
    {
      new_list = (Cl_List) C_malloc (sizeof (_Cl_List));
    }
  Cl_NodeType (new_list) = Cl_NULL;
  Cl_StringOf (new_list) = "???";
  Cl_SiblingOf (new_list) = 0;
  return new_list;
}
/*
 *      Dispose space of a node and its siblings and descendants.       
 */
Cl_List Cl_dispose (Cl_List node)
{
  if (node == 0)
    return 0;
  if ((Cl_NodeType (node) == Cl_LIST) || (Cl_NodeType (node) == Cl_STD_GROUP)
      || (Cl_NodeType (node) == Cl_OPT_GROUP)
      || (Cl_NodeType (node) == Cl_OPT_SYNC))
    {
      Cl_dispose (Cl_ChildOf (node));
      Cl_ChildOf (node) = 0;
    }
  Cl_dispose (Cl_SiblingOf (node));
  Cl_SiblingOf (node) = dead_list;      /* add node to the dead list */
  dead_list = node;             /* for recycling */
  return 0;
}
/*      
 *      Add a child.    
 */
static void
add_child (Cl_List parent, Cl_List child)
{
  Cl_List ptr;
  if (Cl_ChildOf (parent) == 0)
    {
      /* the first child */
      Cl_ChildOf (parent) = child;
    }
  else
    {
      /* find last child */
      for (ptr = Cl_ChildOf (parent); Cl_SiblingOf (ptr) != 0;)
        ptr = Cl_SiblingOf (ptr);
      /* append the new child to it */
      Cl_SiblingOf (ptr) = child;
    }
}
/*--------------------------------------------------------------------------*/
/*
 *      Get a node or a complete list from input.
 *      It is assumed that the lexer has been properly set up.
 */
Cl_List Cl_get (void)
{
  int node_type;
  Cl_List new_list, ptr;
  node_type = yylex ();         /* get token */
  if (node_type == 0)
    {                           /* end of file */
      new_list = new_node ();
      Cl_NodeType (new_list) = Cl_EOF;
      Cl_StringOf (new_list) = "<EOF>";
      Cl_File (new_list) = TokenFile;
      Cl_Line (new_list) = TokenLine;
      Cl_Column (new_list) = TokenColumn;
      return new_list;
    }
  switch (node_type)
    {
    case L_ID:                  /* identifier */
      new_list = new_node ();
      Cl_NodeType (new_list) = Cl_ID;
      Cl_StringOf (new_list) = SAVE (TokenName);
      Cl_File (new_list) = TokenFile;
      Cl_Line (new_list) = TokenLine;
      Cl_Column (new_list) = TokenColumn;
      break;
    case L_INT:         /* integer */
      new_list = new_node ();
      Cl_NodeType (new_list) = Cl_INT;
      Cl_StringOf (new_list) = SAVE2 (TokenName);
      Cl_IntegerOf (new_list) = TokenInteger;
      Cl_File (new_list) = TokenFile;
      Cl_Line (new_list) = TokenLine;
      Cl_Column (new_list) = TokenColumn;
      break;
    case L_REAL:                /* real constant literal */
      new_list = new_node ();
      Cl_NodeType (new_list) = Cl_REAL;
      Cl_StringOf (new_list) = SAVE2 (TokenName);
      Cl_RealOf (new_list) = TokenReal;
      Cl_File (new_list) = TokenFile;
      Cl_Line (new_list) = TokenLine;
      Cl_Column (new_list) = TokenColumn;
      break;
    case L_STRING:              /* string literal */
      new_list = new_node ();
      Cl_NodeType (new_list) = Cl_STRING;
      Cl_StringOf (new_list) = SAVE (TokenName);
      Cl_File (new_list) = TokenFile;
      Cl_Line (new_list) = TokenLine;
      Cl_Column (new_list) = TokenColumn;
      break;
    case L_CHAR:                /* character literal */
      new_list = new_node ();
      Cl_NodeType (new_list) = Cl_CHAR;
      Cl_StringOf (new_list) = SAVE (TokenName);
      Cl_File (new_list) = TokenFile;
      Cl_Line (new_list) = TokenLine;
      Cl_Column (new_list) = TokenColumn;
      break;
    case L_ERROR:               /* erroneous input */
      new_list = new_node ();
      Cl_NodeType (new_list) = Cl_NULL;
      Cl_StringOf (new_list) = SAVE (TokenName);
      Cl_File (new_list) = TokenFile;
      Cl_Line (new_list) = TokenLine;
      Cl_Column (new_list) = TokenColumn;
      break;
    case L_MACRO:
      /* remove everything until \n */
      while ((ptr = Cl_get ()) != 0)
        {
          if (Cl_NodeType (ptr) == Cl_EOF)
            {
              Cl_dispose (ptr);
              break;
            }
          Cl_dispose (ptr);
        }
      new_list = Cl_get ();
      break;
    case L_EOLN:
      new_list = 0;
      break;
    case '(':
      new_list = new_node ();
      Cl_NodeType (new_list) = Cl_LIST;
      Cl_StringOf (new_list) = "()";
      Cl_ChildOf (new_list) = 0;
      Cl_File (new_list) = TokenFile;
      Cl_Line (new_list) = TokenLine;
      Cl_Column (new_list) = TokenColumn;
      while ((ptr = Cl_get ()) != 0)
        {
          if (Cl_NodeType (ptr) == Cl_EOF)
            lex_punt ("detect EOF when expecting )");
          add_child (new_list, ptr);
        }
      break;
    case '[':
      new_list = new_node ();
      Cl_NodeType (new_list) = Cl_STD_GROUP;
      Cl_StringOf (new_list) = "[]";
      Cl_ChildOf (new_list) = 0;
      Cl_File (new_list) = TokenFile;
      Cl_Line (new_list) = TokenLine;
      Cl_Column (new_list) = TokenColumn;
      while ((ptr = Cl_get ()) != 0)
        {
          if (Cl_NodeType (ptr) == Cl_EOF)
            lex_punt ("detect EOF when expecting ]");
          add_child (new_list, ptr);
        }
      break;
    case '<':
      new_list = new_node ();
      Cl_NodeType (new_list) = Cl_OPT_GROUP;
      Cl_StringOf (new_list) = "<>";
      Cl_ChildOf (new_list) = 0;
      Cl_File (new_list) = TokenFile;
      Cl_Line (new_list) = TokenLine;
      Cl_Column (new_list) = TokenColumn;
      while ((ptr = Cl_get ()) != 0)
        {
          if (Cl_NodeType (ptr) == Cl_EOF)
            lex_punt ("detect EOF when expecting >");
          add_child (new_list, ptr);
        }
      break;
    case '{':
      new_list = new_node ();
      Cl_NodeType (new_list) = Cl_OPT_SYNC;
      Cl_StringOf (new_list) = "{}";
      Cl_ChildOf (new_list) = 0;
      Cl_File (new_list) = TokenFile;
      Cl_Line (new_list) = TokenLine;
      Cl_Column (new_list) = TokenColumn;
      while ((ptr = Cl_get ()) != 0)
        {
          if (Cl_NodeType (ptr) == Cl_EOF)
            lex_punt ("detect EOF when expecting }");
          add_child (new_list, ptr);
        }
      break;
    case ')':
    case ']':
    case '>':
    case '}':
      new_list = 0;
      break;
    default:                    /* single character operator */
      new_list = new_node ();
      Cl_NodeType (new_list) = Cl_OPER;
      Cl_StringOf (new_list) = SAVE (TokenName);
      Cl_IntegerOf (new_list) = node_type;
      Cl_File (new_list) = TokenFile;
      Cl_Line (new_list) = TokenLine;
      Cl_Column (new_list) = TokenColumn;
      break;
    }
  return new_list;
}
/*--------------------------------------------------------------------------*/
/*
 *      Get a C token.
 *      It is assumed that the lexer has been properly set up.
 */
Cl_List Cl_get_C_token (void)
{
  int node_type;
  Cl_List new_list;
  node_type = yylex ();         /* get token */
  if (node_type == 0)
    {                           /* end of file */
      new_list = new_node ();
      Cl_NodeType (new_list) = Cl_EOF;
      Cl_StringOf (new_list) = "<EOF>";
      Cl_File (new_list) = TokenFile;
      Cl_Line (new_list) = TokenLine;
      Cl_Column (new_list) = TokenColumn;
      return new_list;
    }
  switch (node_type)
    {
    case L_ID:                  /* identifier */
      new_list = new_node ();
      Cl_NodeType (new_list) = Cl_ID;
      Cl_StringOf (new_list) = SAVE (TokenName);
      Cl_File (new_list) = TokenFile;
      Cl_Line (new_list) = TokenLine;
      Cl_Column (new_list) = TokenColumn;
      break;
    case L_INT:         /* integer */
      new_list = new_node ();
      Cl_NodeType (new_list) = Cl_INT;
      Cl_StringOf (new_list) = SAVE2 (TokenName);
      Cl_IntegerOf (new_list) = TokenInteger;
      Cl_File (new_list) = TokenFile;
      Cl_Line (new_list) = TokenLine;
      Cl_Column (new_list) = TokenColumn;
      break;
    case L_REAL:                /* real constant literal */
      new_list = new_node ();
      Cl_NodeType (new_list) = Cl_REAL;
      Cl_StringOf (new_list) = SAVE2 (TokenName);
      Cl_RealOf (new_list) = TokenReal;
      Cl_File (new_list) = TokenFile;
      Cl_Line (new_list) = TokenLine;
      Cl_Column (new_list) = TokenColumn;
      break;
    case L_STRING:              /* string literal */
      new_list = new_node ();
      Cl_NodeType (new_list) = Cl_STRING;
      Cl_StringOf (new_list) = SAVE (TokenName);
      Cl_File (new_list) = TokenFile;
      Cl_Line (new_list) = TokenLine;
      Cl_Column (new_list) = TokenColumn;
      break;
    case L_CHAR:                /* character literal */
      new_list = new_node ();
      Cl_NodeType (new_list) = Cl_CHAR;
      Cl_StringOf (new_list) = SAVE (TokenName);
      Cl_File (new_list) = TokenFile;
      Cl_Line (new_list) = TokenLine;
      Cl_Column (new_list) = TokenColumn;
      break;
    case L_MACRO:
      new_list = new_node ();
      Cl_NodeType (new_list) = Cl_MACRO;
      Cl_StringOf (new_list) = SAVE (TokenName);
      Cl_File (new_list) = TokenFile;
      Cl_Line (new_list) = TokenLine;
      Cl_Column (new_list) = TokenColumn;
      break;
    case L_EOLN:
      new_list = new_node ();
      Cl_NodeType (new_list) = Cl_EOLN;
      Cl_StringOf (new_list) = "<EOLN>";
      Cl_File (new_list) = TokenFile;
      Cl_Line (new_list) = TokenLine;
      Cl_Column (new_list) = TokenColumn;
      break;
    case L_ERROR:               /* erroneous input */
      new_list = new_node ();
      Cl_NodeType (new_list) = Cl_NULL;
      Cl_StringOf (new_list) = SAVE (TokenName);
      Cl_File (new_list) = TokenFile;
      Cl_Line (new_list) = TokenLine;
      Cl_Column (new_list) = TokenColumn;
      break;
    default:                    /* single character operator */
      new_list = new_node ();
      Cl_NodeType (new_list) = Cl_OPER;
      Cl_StringOf (new_list) = SAVE (TokenName);
      Cl_IntegerOf (new_list) = node_type;
      Cl_File (new_list) = TokenFile;
      Cl_Line (new_list) = TokenLine;
      Cl_Column (new_list) = TokenColumn;
    }
  return new_list;
}
/*-------------------------------------------------------------------------*/
/*      print a node.
 *      this is useful for program debugging.
 */
static FILE *FF = NULL;
static void
print_node (Cl_List node, int indent)
{
  int i;
  Cl_List ptr;
  if (FF == NULL)
    FF = stdout;
  if (node == 0)
    return;
  for (i = 0; i < indent; i++)
    fprintf (FF, " ");
  switch (Cl_NodeType (node))
    {
    case Cl_EOF:
      break;
    case Cl_NULL:
      fprintf (FF, "???");
      break;
    case Cl_LIST:
      fprintf (FF, "(\n");
      for (ptr = Cl_ChildOf (node); ptr != 0; ptr = Cl_SiblingOf (ptr))
        {
          print_node (ptr, indent + 2);
          fprintf (FF, "\n");
        }
      for (i = 0; i < indent; i++)
        fprintf (FF, " ");
      fprintf (FF, ")");
      break;
    case Cl_ID:
      fprintf (FF, "%s", Cl_StringOf (node));
      break;
    case Cl_INT:
      fprintf (FF, ITintmaxformat, Cl_IntegerOf (node));
      break;
    case Cl_REAL:
      fprintf (FF, "%f", Cl_RealOf (node));
      break;
    case Cl_STRING:
      fprintf (FF, "%s", Cl_StringOf (node));
      break;
    case Cl_CHAR:
      fprintf (FF, "%s", Cl_StringOf (node));
      break;
    case Cl_OPER:
      fprintf (FF, "%c", (char) Cl_IntegerOf (node));
      break;
    case Cl_MACRO:
    case Cl_EOLN:
      fprintf (FF, "%s", Cl_StringOf (node));
      break;
    case Cl_STD_GROUP:
      fprintf (FF, "[\n");
      for (ptr = Cl_ChildOf (node); ptr != 0; ptr = Cl_SiblingOf (ptr))
        {
          print_node (ptr, indent + 2);
          fprintf (FF, "\n");
        }
      for (i = 0; i < indent; i++)
        fprintf (FF, " ");
      fprintf (FF, "]");
      break;
    case Cl_OPT_GROUP:
      fprintf (FF, "<\n");
      for (ptr = Cl_ChildOf (node); ptr != 0; ptr = Cl_SiblingOf (ptr))
        {
          print_node (ptr, indent + 2);
          fprintf (FF, "\n");
        }
      for (i = 0; i < indent; i++)
        fprintf (FF, " ");
      fprintf (FF, ">");
      break;
    case Cl_OPT_SYNC:
      fprintf (FF, "{\n");
      for (ptr = Cl_ChildOf (node); ptr != 0; ptr = Cl_SiblingOf (ptr))
        {
          print_node (ptr, indent + 2);
          fprintf (FF, "\n");
        }
      for (i = 0; i < indent; i++)
        fprintf (FF, " ");
      fprintf (FF, "}");
      break;
    default:
      fprintf (FF, "???");
      break;
    }
}
/*
 *      print a node in a specified file.
 */
void
Cl_print (FILE * F, Cl_List node)
{
  FILE *save = FF;
  FF = F;
  print_node (node, 0);
  fprintf (F, "\n");
  FF = save;
}
/*-----------------------------------------------------------------------*/
/*
 *      [+-]* int
 */
Cl_List Cl_integer (Cl_List list, C_Integer * val)
{
  Cl_List ptr;
  int sign;
  ptr = list;
  if (ptr == 0)
    {
      *val = 0;
      return 0;
    }
  sign = 1;
  while (Cl_NodeType (ptr) == Cl_OPER)
    {
      if ((char) Cl_IntegerOf (ptr) == '-')
        {
          sign = -sign;
        }
      else if ((char) Cl_IntegerOf (ptr) == '+')
        {
        }
      else
        {
          *val = 0;
          return list;
        }
      ptr = Cl_SiblingOf (ptr);
      if (ptr == 0)
        break;
    }
  if (ptr == 0)
    {
      *val = 0;
      return list;
    }
  if (Cl_NodeType (ptr) != Cl_INT)
    {
      *val = 0;
      return list;
    }
  *val = (Cl_IntegerOf (ptr) * sign);
  return Cl_SiblingOf (ptr);
}
/*
 *      [+-]* int
 *      [+-]* real
 */
Cl_List Cl_real (Cl_List list, C_Double * val)
{
  Cl_List ptr;
  double sign;
  ptr = list;
  if (ptr == 0)
    {
      *val = 0.0;
      return 0;
    }
  sign = 1.0;
  while (Cl_NodeType (ptr) == Cl_OPER)
    {
      if ((char) Cl_IntegerOf (ptr) == '-')
        {
          sign = -sign;
        }
      else if ((char) Cl_IntegerOf (ptr) == '+')
        {
        }
      else
        {
          *val = 0.0;
          return list;
        }
      ptr = Cl_SiblingOf (ptr);
      if (ptr == 0)
        break;
    }
  if (ptr == 0)
    {
      *val = 0.0;
      return list;
    }
  if (Cl_NodeType (ptr) == Cl_INT)
    {
      *val = (Cl_IntegerOf (ptr) * sign);
      return Cl_SiblingOf (ptr);
    }
  else if (Cl_NodeType (ptr) == Cl_REAL)
    {
      *val = (Cl_RealOf (ptr) * sign);
      return Cl_SiblingOf (ptr);
    }
  else
    {
      *val = 0.0;
      return list;
    }
}
Cl_List Cl_string (Cl_List list, char **val)
{
  if (list == 0)
    {
      *val = 0;
      return 0;
    }
  if (Cl_NodeType (list) != Cl_STRING)
    return list;
  *val = Cl_StringOf (list);
  return Cl_SiblingOf (list);
}
Cl_List Cl_char (Cl_List list, char **val)
{
  if (list == 0)
    {
      *val = 0;
      return 0;
    }
  if (Cl_NodeType (list) != Cl_CHAR)
    return list;
  *val = Cl_StringOf (list);
  return Cl_SiblingOf (list);
}
Cl_List Cl_identifier (Cl_List list, char **val)
{
  if (list == 0)
    {
      *val = 0;
      return 0;
    }
  if (Cl_NodeType (list) != Cl_ID)
    return list;
  *val = Cl_StringOf (list);
  return Cl_SiblingOf (list);
}
/*-------------------------------------------------------------------------*/
int yyvstop[] = {
  0,

  52,
  0,

  11,
  52,
  0,

  12,
  0,

  51,
  52,
  0,

  9,
  52,
  0,

  24,
  52,
  0,

  52,
  0,

  14,
  52,
  0,

  18,
  52,
  0,

  26,
  52,
  0,

  28,
  52,
  0,

  52,
  0,

  30,
  52,
  0,

  31,
  52,
  0,

  29,
  52,
  0,

  33,
  52,
  0,

  47,
  52,
  0,

  32,
  52,
  0,

  18,
  48,
  52,
  0,

  18,
  44,
  52,
  0,

  16,
  52,
  0,

  17,
  52,
  0,

  10,
  41,
  52,
  0,

  42,
  52,
  0,

  45,
  52,
  0,

  34,
  52,
  0,

  46,
  52,
  0,

  43,
  52,
  0,

  25,
  52,
  0,

  35,
  52,
  0,

  50,
  52,
  0,

  36,
  52,
  0,

  27,
  52,
  0,

  40,
  52,
  0,

  37,
  52,
  0,

  49,
  52,
  0,

  38,
  52,
  0,

  39,
  52,
  0,

  1,
  52,
  0,

  2,
  11,
  52,
  0,

  3,
  12,
  0,

  1,
  51,
  52,
  0,

  1,
  9,
  52,
  0,

  1,
  24,
  52,
  0,

  1,
  52,
  0,

  1,
  14,
  52,
  0,

  1,
  18,
  52,
  0,

  1,
  26,
  52,
  0,

  1,
  28,
  52,
  0,

  1,
  52,
  0,

  1,
  30,
  52,
  0,

  1,
  31,
  52,
  0,

  7,
  29,
  52,
  0,

  1,
  33,
  52,
  0,

  1,
  47,
  52,
  0,

  1,
  32,
  52,
  0,

  1,
  18,
  48,
  52,
  0,

  1,
  18,
  44,
  52,
  0,

  1,
  16,
  52,
  0,

  1,
  17,
  52,
  0,

  1,
  10,
  41,
  52,
  0,

  1,
  42,
  52,
  0,

  1,
  45,
  52,
  0,

  1,
  34,
  52,
  0,

  1,
  46,
  52,
  0,

  1,
  43,
  52,
  0,

  1,
  25,
  52,
  0,

  1,
  35,
  52,
  0,

  1,
  50,
  52,
  0,

  1,
  36,
  52,
  0,

  1,
  27,
  52,
  0,

  1,
  40,
  52,
  0,

  1,
  37,
  52,
  0,

  1,
  49,
  52,
  0,

  1,
  38,
  52,
  0,

  1,
  39,
  52,
  0,

  9,
  0,

  22,
  0,

  18,
  0,

  18,
  21,
  0,

  8,
  0,

  19,
  0,

  16,
  0,

  16,
  0,

  17,
  0,

  17,
  0,

  10,
  0,

  13,
  0,

  1,
  0,

  1,
  9,
  0,

  1,
  0,

  1,
  22,
  0,

  1,
  0,

  1,
  18,
  0,

  1,
  0,

  1,
  0,

  7,
  0,

  5,
  0,

  6,
  0,

  4,
  0,

  1,
  18,
  21,
  0,

  1,
  19,
  0,

  1,
  16,
  0,

  1,
  0,

  1,
  0,

  1,
  16,
  0,

  1,
  0,

  1,
  17,
  0,

  1,
  17,
  0,

  1,
  10,
  0,

  23,
  0,

  23,
  0,

  18,
  0,

  20,
  0,

  15,
  0,

  1,
  0,

  1,
  0,

  1,
  0,

  1,
  23,
  0,

  1,
  0,

  1,
  23,
  0,

  1,
  0,

  1,
  0,

  1,
  0,

  1,
  18,
  0,

  1,
  0,

  1,
  0,

  1,
  20,
  0,

  1,
  15,
  0,

  18,
  21,
  0,

  19,
  0,

  15,
  0,

  1,
  0,

  1,
  0,

  1,
  0,

  1,
  0,

  1,
  0,

  1,
  0,

  1,
  0,

  1,
  18,
  21,
  0,

  1,
  0,

  1,
  19,
  0,

  1,
  15,
  0,

  21,
  0,

  1,
  21,
  0,
  0
};

# define YYTYPE unsigned char
struct yywork
{
  YYTYPE verify, advance;
}
yycrank[] =
{
  0, 0, 0, 0, 1, 5, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 1, 6, 1, 7,
    0, 0, 1, 8, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 35, 99,
    0, 0, 46, 0, 46, 0, 48, 0,
    48, 0, 50, 0, 50, 0, 0, 0,
    0, 0, 82, 0, 0, 0, 123, 0,
    151, 0, 1, 9, 1, 10, 1, 11,
    1, 12, 1, 13, 1, 14, 1, 15,
    1, 16, 1, 17, 1, 18, 1, 19,
    1, 20, 1, 21, 1, 22, 1, 23,
    1, 24, 1, 25, 1, 26, 9, 81,
    2, 8, 24, 89, 46, 0, 86, 125,
    48, 0, 1, 26, 50, 0, 1, 27,
    1, 28, 1, 29, 1, 30, 1, 31,
    1, 32, 1, 33, 1, 13, 27, 98,
    0, 0, 123, 151, 123, 151, 0, 0,
    2, 9, 2, 10, 0, 0, 2, 12,
    0, 0, 2, 14, 2, 15, 2, 16,
    2, 17, 2, 18, 2, 19, 2, 20,
    2, 21, 2, 22, 2, 23, 0, 0,
    0, 0, 1, 13, 135, 160, 0, 0,
    1, 34, 1, 35, 1, 36, 1, 37,
    152, 0, 1, 38, 1, 13, 2, 28,
    2, 29, 2, 30, 2, 31, 2, 32,
    2, 33, 47, 0, 47, 0, 52, 0,
    52, 0, 53, 0, 53, 0, 82, 84,
    3, 43, 123, 84, 151, 84, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0,
    3, 44, 3, 45, 135, 160, 3, 46,
    1, 39, 1, 40, 1, 41, 1, 42,
    47, 101, 0, 0, 0, 0, 2, 34,
    2, 35, 2, 36, 2, 37, 0, 0,
    2, 38, 0, 0, 47, 0, 0, 0,
    52, 0, 0, 0, 53, 0, 3, 47,
    3, 48, 3, 49, 3, 50, 3, 51,
    3, 52, 3, 53, 3, 54, 3, 55,
    3, 56, 3, 57, 3, 58, 3, 59,
    3, 60, 3, 61, 3, 62, 3, 63,
    3, 64, 0, 0, 4, 46, 2, 39,
    2, 40, 2, 41, 2, 42, 3, 64,
    0, 0, 3, 65, 3, 66, 3, 67,
    3, 68, 3, 69, 3, 70, 3, 71,
    3, 51, 0, 0, 152, 84, 0, 0,
    0, 0, 0, 0, 4, 47, 4, 48,
    0, 0, 4, 50, 0, 0, 4, 52,
    4, 53, 4, 54, 4, 55, 4, 56,
    4, 57, 4, 58, 4, 59, 4, 60,
    4, 61, 55, 0, 55, 0, 3, 51,
    56, 0, 56, 0, 3, 72, 3, 73,
    3, 74, 3, 75, 0, 0, 3, 76,
    3, 51, 4, 66, 4, 67, 4, 68,
    4, 69, 4, 70, 4, 71, 11, 82,
    58, 0, 58, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 11, 82,
    11, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 55, 0, 59, 0,
    59, 0, 56, 0, 3, 77, 3, 78,
    3, 79, 3, 80, 60, 0, 60, 0,
    0, 0, 4, 72, 4, 73, 4, 74,
    4, 75, 0, 0, 4, 76, 0, 0,
    11, 83, 58, 0, 11, 82, 23, 88,
    23, 88, 23, 88, 23, 88, 23, 88,
    23, 88, 23, 88, 23, 88, 23, 88,
    23, 88, 11, 82, 11, 82, 11, 82,
    59, 0, 66, 0, 66, 0, 67, 0,
    67, 0, 0, 0, 11, 82, 60, 0,
    11, 82, 4, 77, 4, 78, 4, 79,
    4, 80, 65, 0, 65, 0, 11, 82,
    133, 134, 133, 134, 133, 134, 133, 134,
    133, 134, 133, 134, 133, 134, 133, 134,
    133, 134, 133, 134, 0, 0, 0, 0,
    0, 0, 0, 0, 68, 0, 68, 0,
    69, 0, 69, 0, 66, 0, 0, 0,
    67, 0, 0, 0, 11, 82, 13, 85,
    0, 0, 0, 0, 11, 84, 0, 0,
    0, 0, 0, 0, 65, 0, 11, 82,
    0, 0, 13, 85, 13, 85, 13, 85,
    13, 85, 13, 85, 13, 85, 13, 85,
    13, 85, 13, 85, 13, 85, 13, 85,
    13, 85, 13, 85, 65, 121, 68, 0,
    0, 0, 69, 0, 70, 0, 70, 0,
    13, 85, 13, 85, 13, 85, 13, 85,
    13, 85, 13, 85, 13, 85, 13, 85,
    13, 85, 13, 85, 13, 85, 13, 85,
    13, 85, 13, 85, 13, 85, 13, 85,
    13, 85, 13, 85, 13, 85, 13, 85,
    13, 85, 13, 85, 13, 85, 13, 85,
    13, 85, 13, 85, 71, 0, 71, 0,
    72, 0, 72, 0, 13, 85, 70, 0,
    13, 85, 13, 85, 13, 85, 13, 85,
    13, 85, 13, 85, 13, 85, 13, 85,
    13, 85, 13, 85, 13, 85, 13, 85,
    13, 85, 13, 85, 13, 85, 13, 85,
    13, 85, 13, 85, 13, 85, 13, 85,
    13, 85, 13, 85, 13, 85, 13, 85,
    13, 85, 13, 85, 16, 86, 71, 0,
    0, 0, 72, 0, 73, 0, 73, 99,
    74, 0, 74, 0, 16, 86, 16, 0,
    0, 0, 25, 90, 0, 0, 25, 91,
    25, 91, 25, 91, 25, 91, 25, 91,
    25, 91, 25, 91, 25, 91, 25, 92,
    25, 92, 75, 0, 75, 0, 76, 0,
    76, 0, 0, 0, 77, 0, 77, 0,
    78, 0, 78, 0, 0, 0, 16, 86,
    25, 93, 16, 86, 0, 0, 73, 0,
    0, 0, 74, 0, 0, 0, 25, 94,
    0, 0, 0, 0, 0, 0, 0, 0,
    16, 86, 16, 86, 16, 86, 0, 0,
    0, 0, 43, 100, 0, 0, 25, 95,
    0, 0, 16, 86, 75, 0, 16, 86,
    76, 0, 43, 0, 43, 0, 77, 0,
    0, 0, 78, 0, 16, 86, 0, 0,
    25, 93, 79, 0, 79, 0, 0, 0,
    80, 0, 80, 0, 26, 90, 25, 94,
    26, 96, 26, 96, 26, 96, 26, 96,
    26, 96, 26, 96, 26, 96, 26, 96,
    26, 96, 26, 96, 43, 100, 25, 95,
    43, 100, 16, 86, 100, 0, 100, 0,
    0, 0, 16, 87, 43, 0, 51, 0,
    51, 0, 26, 93, 16, 86, 43, 100,
    43, 100, 43, 100, 79, 0, 0, 0,
    26, 97, 80, 0, 49, 102, 0, 0,
    43, 100, 0, 0, 43, 100, 0, 0,
    54, 106, 103, 0, 103, 0, 49, 0,
    0, 0, 43, 100, 0, 0, 0, 0,
    0, 0, 54, 0, 51, 105, 100, 0,
    117, 0, 117, 0, 0, 0, 122, 0,
    51, 0, 26, 93, 0, 0, 120, 0,
    120, 0, 51, 105, 51, 105, 51, 105,
    26, 97, 0, 0, 0, 0, 49, 103,
    43, 100, 49, 102, 51, 105, 0, 0,
    51, 105, 54, 106, 103, 0, 54, 106,
    0, 0, 43, 100, 0, 0, 51, 105,
    49, 102, 49, 102, 49, 102, 101, 0,
    101, 0, 117, 0, 54, 106, 54, 106,
    54, 106, 49, 102, 0, 0, 49, 102,
    120, 0, 122, 123, 122, 123, 54, 106,
    0, 0, 54, 106, 49, 102, 57, 108,
    0, 0, 0, 0, 51, 105, 0, 0,
    54, 106, 0, 0, 101, 101, 57, 109,
    57, 110, 0, 0, 0, 0, 51, 105,
    0, 0, 0, 0, 0, 0, 0, 0,
    101, 0, 0, 0, 106, 0, 106, 0,
    0, 0, 49, 102, 61, 0, 61, 0,
    0, 0, 49, 104, 0, 0, 54, 106,
    0, 0, 0, 0, 49, 102, 54, 107,
    57, 108, 122, 150, 57, 108, 0, 0,
    54, 106, 122, 84, 0, 0, 0, 0,
    57, 0, 62, 0, 62, 0, 0, 0,
    0, 0, 57, 111, 57, 108, 57, 108,
    106, 139, 61, 105, 0, 0, 106, 0,
    63, 0, 63, 0, 57, 108, 61, 0,
    57, 108, 0, 0, 0, 0, 0, 0,
    61, 105, 61, 112, 61, 112, 57, 108,
    0, 0, 121, 0, 121, 0, 0, 0,
    62, 105, 61, 112, 0, 0, 61, 105,
    139, 0, 139, 0, 62, 89, 0, 0,
    64, 0, 64, 0, 61, 105, 62, 105,
    62, 105, 62, 105, 0, 0, 0, 0,
    0, 0, 63, 0, 57, 108, 0, 0,
    62, 105, 63, 113, 62, 105, 63, 114,
    63, 114, 118, 0, 118, 0, 57, 108,
    0, 0, 62, 105, 121, 0, 63, 115,
    0, 0, 61, 105, 0, 0, 0, 0,
    0, 0, 139, 0, 0, 0, 0, 0,
    0, 0, 64, 0, 61, 105, 0, 0,
    63, 116, 64, 113, 121, 121, 64, 119,
    64, 119, 84, 82, 0, 0, 63, 117,
    62, 105, 171, 0, 171, 0, 64, 119,
    0, 0, 0, 0, 118, 0, 0, 0,
    0, 0, 62, 105, 0, 0, 63, 118,
    118, 149, 118, 149, 140, 0, 140, 0,
    64, 116, 0, 0, 0, 0, 87, 126,
    118, 149, 84, 82, 0, 0, 64, 120,
    63, 116, 0, 0, 84, 82, 87, 126,
    87, 0, 118, 149, 0, 0, 63, 117,
    0, 0, 0, 0, 171, 0, 84, 122,
    84, 123, 84, 123, 84, 123, 84, 123,
    84, 123, 84, 123, 84, 123, 0, 0,
    140, 139, 0, 0, 0, 0, 140, 0,
    64, 116, 0, 0, 84, 82, 0, 0,
    87, 126, 0, 0, 87, 126, 64, 120,
    0, 0, 87, 127, 141, 0, 141, 0,
    0, 0, 118, 149, 0, 0, 0, 0,
    0, 0, 87, 126, 87, 128, 87, 129,
    0, 0, 143, 0, 143, 0, 0, 0,
    0, 0, 0, 0, 87, 126, 84, 124,
    87, 126, 0, 0, 0, 0, 84, 82,
    0, 0, 0, 0, 0, 0, 87, 126,
    84, 82, 84, 82, 0, 0, 0, 0,
    141, 139, 84, 82, 0, 0, 141, 0,
    0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 84, 82, 0, 0, 143, 139,
    0, 0, 84, 82, 143, 0, 84, 82,
    0, 0, 84, 82, 87, 130, 84, 124,
    143, 165, 143, 165, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 87, 126,
    88, 88, 88, 88, 88, 88, 88, 88,
    88, 88, 88, 88, 88, 88, 88, 88,
    88, 88, 88, 88, 90, 90, 90, 90,
    90, 90, 90, 90, 90, 90, 90, 90,
    90, 90, 90, 90, 90, 90, 90, 90,
    0, 0, 88, 131, 154, 126, 154, 126,
    154, 126, 154, 126, 154, 126, 154, 126,
    154, 126, 154, 126, 0, 0, 90, 132,
    91, 91, 91, 91, 91, 91, 91, 91,
    91, 91, 91, 91, 91, 91, 91, 91,
    91, 92, 91, 92, 0, 0, 0, 0,
    147, 0, 147, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 88, 131, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0,
    91, 94, 0, 0, 92, 90, 90, 132,
    92, 92, 92, 92, 92, 92, 92, 92,
    92, 92, 92, 92, 92, 92, 92, 92,
    92, 92, 92, 92, 0, 0, 0, 0,
    0, 0, 147, 0, 0, 0, 0, 0,
    102, 102, 0, 0, 0, 0, 147, 148,
    147, 148, 92, 93, 0, 0, 0, 0,
    93, 133, 102, 0, 93, 133, 147, 148,
    91, 94, 93, 134, 93, 134, 93, 134,
    93, 134, 93, 134, 93, 134, 93, 134,
    93, 134, 93, 134, 93, 134, 95, 135,
    95, 135, 95, 135, 95, 135, 95, 135,
    95, 135, 95, 135, 95, 135, 95, 135,
    95, 135, 102, 103, 0, 0, 102, 102,
    0, 0, 92, 93, 0, 0, 0, 0,
    95, 135, 95, 135, 95, 135, 95, 135,
    95, 135, 95, 135, 102, 102, 102, 102,
    102, 102, 0, 0, 0, 0, 104, 100,
    0, 0, 0, 0, 0, 0, 102, 102,
    0, 0, 102, 102, 0, 0, 104, 0,
    104, 82, 0, 0, 0, 0, 0, 0,
    102, 102, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 113, 0, 113, 0,
    95, 135, 95, 135, 95, 135, 95, 135,
    95, 135, 95, 135, 0, 0, 0, 0,
    0, 0, 115, 0, 115, 0, 0, 0,
    104, 102, 0, 0, 104, 100, 102, 102,
    0, 0, 104, 102, 0, 0, 102, 104,
    104, 0, 0, 0, 0, 0, 0, 0,
    102, 102, 104, 100, 104, 136, 104, 137,
    0, 0, 0, 0, 0, 0, 113, 0,
    165, 0, 165, 0, 104, 100, 0, 0,
    104, 100, 113, 113, 113, 113, 0, 0,
    0, 0, 104, 102, 115, 0, 104, 100,
    0, 0, 113, 113, 115, 113, 0, 0,
    115, 115, 115, 115, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0,
    115, 115, 0, 0, 113, 146, 0, 0,
    105, 0, 105, 0, 165, 139, 0, 0,
    0, 0, 165, 0, 104, 138, 0, 0,
    0, 0, 115, 116, 0, 0, 165, 140,
    165, 140, 107, 140, 0, 0, 104, 102,
    0, 0, 104, 100, 104, 100, 104, 100,
    0, 0, 0, 0, 107, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 105, 105,
    104, 102, 0, 0, 113, 146, 0, 0,
    104, 102, 105, 0, 104, 102, 0, 0,
    104, 102, 0, 0, 105, 105, 105, 105,
    105, 105, 115, 116, 0, 0, 142, 0,
    142, 0, 0, 0, 107, 140, 105, 105,
    107, 140, 105, 105, 0, 0, 107, 141,
    0, 0, 0, 0, 148, 0, 148, 0,
    105, 105, 112, 100, 0, 0, 107, 140,
    107, 142, 107, 143, 0, 0, 0, 0,
    0, 0, 112, 0, 112, 0, 0, 0,
    107, 140, 0, 0, 107, 140, 0, 0,
    0, 0, 142, 139, 0, 0, 0, 0,
    142, 0, 107, 140, 0, 0, 105, 105,
    114, 0, 114, 0, 142, 143, 142, 143,
    0, 0, 0, 0, 0, 0, 148, 0,
    105, 105, 0, 0, 112, 100, 0, 0,
    112, 105, 148, 148, 148, 148, 116, 0,
    116, 0, 0, 0, 112, 0, 0, 0,
    107, 144, 148, 148, 0, 0, 112, 105,
    112, 112, 112, 112, 0, 0, 0, 0,
    0, 0, 107, 140, 0, 0, 0, 0,
    112, 112, 114, 0, 112, 105, 119, 0,
    119, 0, 114, 113, 0, 0, 114, 114,
    114, 114, 112, 105, 142, 164, 0, 0,
    0, 0, 112, 145, 0, 0, 114, 115,
    116, 0, 116, 147, 0, 0, 116, 147,
    0, 0, 0, 0, 116, 148, 116, 148,
    0, 0, 0, 0, 0, 0, 0, 0,
    114, 116, 0, 0, 116, 148, 0, 0,
    112, 105, 0, 0, 0, 0, 114, 117,
    119, 0, 0, 0, 0, 0, 0, 0,
    119, 113, 112, 105, 119, 119, 119, 119,
    0, 0, 112, 145, 0, 0, 0, 0,
    0, 0, 0, 0, 119, 119, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0,
    114, 116, 0, 0, 0, 0, 119, 116,
    0, 0, 0, 0, 0, 0, 114, 117,
    0, 0, 0, 0, 119, 120, 124, 152,
    124, 152, 124, 152, 124, 152, 124, 152,
    124, 152, 124, 152, 124, 152, 124, 152,
    124, 152, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0,
    124, 152, 124, 152, 124, 152, 124, 152,
    124, 152, 124, 152, 128, 125, 119, 116,
    0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 119, 120, 128, 129,
    128, 129, 128, 129, 128, 129, 128, 129,
    128, 129, 128, 129, 128, 129, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 129, 125, 0, 0, 0, 0,
    124, 152, 124, 152, 124, 152, 124, 152,
    124, 152, 124, 152, 129, 154, 129, 154,
    129, 154, 129, 154, 129, 154, 129, 154,
    129, 154, 129, 154, 0, 0, 130, 125,
    0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 128, 153,
    130, 155, 130, 155, 130, 155, 130, 155,
    130, 155, 130, 155, 130, 155, 130, 155,
    130, 155, 130, 155, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 130, 155, 130, 155, 130, 155,
    130, 155, 130, 155, 130, 155, 0, 0,
    0, 0, 0, 0, 0, 0, 131, 156,
    0, 0, 131, 156, 0, 0, 128, 153,
    131, 157, 131, 157, 131, 157, 131, 157,
    131, 157, 131, 157, 131, 157, 131, 157,
    131, 157, 131, 157, 0, 0, 0, 0,
    138, 0, 138, 0, 0, 0, 136, 102,
    0, 0, 130, 155, 130, 155, 130, 155,
    130, 155, 130, 155, 130, 155, 132, 158,
    136, 0, 132, 158, 0, 0, 0, 0,
    132, 159, 132, 159, 132, 159, 132, 159,
    132, 159, 132, 159, 132, 159, 132, 159,
    132, 159, 132, 159, 0, 0, 0, 0,
    137, 102, 0, 0, 0, 0, 150, 0,
    0, 0, 138, 0, 0, 0, 0, 0,
    136, 103, 137, 0, 136, 102, 138, 163,
    138, 163, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 138, 163,
    0, 0, 136, 102, 136, 137, 136, 137,
    0, 0, 0, 0, 0, 0, 0, 0,
    138, 163, 0, 0, 136, 102, 0, 0,
    136, 102, 137, 103, 0, 0, 137, 102,
    0, 0, 144, 0, 144, 0, 136, 102,
    0, 0, 150, 152, 150, 152, 0, 0,
    0, 0, 0, 0, 137, 102, 137, 162,
    137, 162, 150, 152, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 137, 102,
    0, 0, 137, 102, 150, 152, 0, 0,
    138, 163, 0, 0, 136, 161, 0, 0,
    137, 102, 145, 100, 136, 104, 144, 139,
    0, 0, 0, 0, 144, 0, 136, 102,
    0, 0, 145, 0, 145, 0, 0, 0,
    144, 166, 144, 166, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0,
    144, 166, 150, 84, 0, 0, 137, 102,
    0, 0, 0, 0, 150, 152, 137, 104,
    0, 0, 144, 166, 146, 0, 146, 0,
    137, 102, 0, 0, 145, 100, 0, 0,
    145, 105, 0, 0, 0, 0, 149, 0,
    149, 0, 0, 0, 145, 0, 145, 167,
    0, 0, 145, 167, 0, 0, 145, 105,
    145, 168, 145, 168, 0, 0, 167, 0,
    167, 0, 0, 0, 169, 0, 169, 0,
    145, 168, 0, 0, 145, 105, 0, 0,
    0, 0, 144, 166, 0, 0, 146, 0,
    146, 169, 145, 105, 146, 169, 0, 0,
    0, 0, 146, 170, 146, 170, 0, 0,
    149, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 146, 170, 149, 149, 149, 149,
    0, 0, 0, 0, 0, 0, 0, 0,
    167, 0, 0, 0, 149, 149, 169, 0,
    145, 105, 0, 0, 167, 173, 167, 173,
    0, 0, 169, 170, 169, 170, 149, 149,
    0, 0, 145, 105, 167, 173, 0, 0,
    0, 0, 169, 170, 0, 0, 0, 0,
    0, 0, 0, 0, 149, 171, 0, 0,
    0, 0, 0, 0, 0, 0, 153, 155,
    153, 155, 153, 155, 153, 155, 153, 155,
    153, 155, 153, 155, 153, 155, 153, 155,
    153, 155, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 149, 149,
    153, 155, 153, 155, 153, 155, 153, 155,
    153, 155, 153, 155, 0, 0, 0, 0,
    0, 0, 0, 0, 149, 171, 156, 172,
    156, 172, 156, 172, 156, 172, 156, 172,
    156, 172, 156, 172, 156, 172, 156, 172,
    156, 172, 155, 126, 155, 126, 155, 126,
    155, 126, 155, 126, 155, 126, 155, 126,
    155, 126, 155, 126, 155, 126, 0, 0,
    153, 155, 153, 155, 153, 155, 153, 155,
    153, 155, 153, 155, 155, 126, 155, 126,
    155, 126, 155, 126, 155, 126, 155, 126,
    157, 157, 157, 157, 157, 157, 157, 157,
    157, 157, 157, 157, 157, 157, 157, 157,
    157, 157, 157, 157, 158, 159, 158, 159,
    158, 159, 158, 159, 158, 159, 158, 159,
    158, 159, 158, 159, 158, 159, 158, 159,
    164, 0, 164, 0, 0, 0, 161, 102,
    0, 0, 0, 0, 155, 126, 155, 126,
    155, 126, 155, 126, 155, 126, 155, 126,
    161, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 162, 102, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0,
    163, 102, 0, 0, 0, 0, 162, 0,
    0, 0, 164, 0, 0, 0, 0, 0,
    161, 103, 163, 0, 161, 102, 164, 166,
    164, 166, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 164, 166,
    0, 0, 161, 102, 161, 163, 161, 163,
    0, 0, 0, 0, 0, 0, 162, 103,
    164, 166, 162, 102, 161, 163, 0, 0,
    161, 102, 163, 103, 0, 0, 163, 102,
    0, 0, 166, 0, 166, 0, 161, 163,
    162, 102, 162, 102, 162, 102, 0, 0,
    0, 0, 0, 0, 163, 102, 163, 102,
    163, 102, 162, 102, 0, 0, 162, 102,
    0, 0, 0, 0, 0, 0, 163, 102,
    0, 0, 163, 102, 162, 102, 0, 0,
    164, 166, 0, 0, 161, 102, 0, 0,
    163, 102, 0, 0, 161, 104, 166, 139,
    168, 0, 168, 0, 166, 0, 161, 163,
    170, 0, 170, 0, 0, 0, 0, 0,
    166, 140, 166, 140, 0, 0, 0, 0,
    0, 0, 162, 102, 0, 0, 0, 0,
    166, 140, 162, 104, 0, 0, 163, 102,
    173, 0, 173, 0, 162, 102, 163, 104,
    0, 0, 166, 140, 0, 0, 168, 105,
    163, 102, 0, 0, 0, 0, 0, 0,
    0, 0, 168, 0, 0, 0, 0, 0,
    0, 0, 170, 0, 168, 105, 168, 168,
    168, 168, 0, 0, 0, 0, 170, 170,
    170, 170, 0, 0, 0, 0, 168, 168,
    0, 0, 168, 105, 0, 0, 170, 170,
    0, 0, 173, 0, 0, 0, 0, 0,
    168, 105, 166, 140, 0, 0, 173, 173,
    173, 173, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 173, 173,
    0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 168, 105,
    0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0, 168, 105, 0, 0, 0, 0, 0, 0, 0, 0};
struct yysvf yysvec[] = {
  0, 0, 0,
  yycrank + -1, 0, 0,
  yycrank + -40, yysvec + 1, 0,
  yycrank + -111, 0, 0,
  yycrank + -150, yysvec + 3, 0,
  yycrank + 0, 0, yyvstop + 1,
  yycrank + 0, 0, yyvstop + 3,
  yycrank + 0, 0, yyvstop + 6,
  yycrank + 0, 0, yyvstop + 8,
  yycrank + 19, 0, yyvstop + 11,
  yycrank + 0, 0, yyvstop + 14,
  yycrank + -214, 0, yyvstop + 17,
  yycrank + 0, 0, yyvstop + 19,
  yycrank + 267, 0, yyvstop + 22,
  yycrank + 0, 0, yyvstop + 25,
  yycrank + 0, 0, yyvstop + 28,
  yycrank + -389, 0, yyvstop + 31,
  yycrank + 0, 0, yyvstop + 33,
  yycrank + 0, 0, yyvstop + 36,
  yycrank + 0, 0, yyvstop + 39,
  yycrank + 0, 0, yyvstop + 42,
  yycrank + 0, 0, yyvstop + 45,
  yycrank + 0, 0, yyvstop + 48,
  yycrank + 203, yysvec + 13, yyvstop + 51,
  yycrank + 11, yysvec + 13, yyvstop + 55,
  yycrank + 355, 0, yyvstop + 59,
  yycrank + 416, 0, yyvstop + 62,
  yycrank + 9, 0, yyvstop + 65,
  yycrank + 0, 0, yyvstop + 69,
  yycrank + 0, 0, yyvstop + 72,
  yycrank + 0, 0, yyvstop + 75,
  yycrank + 0, 0, yyvstop + 78,
  yycrank + 0, 0, yyvstop + 81,
  yycrank + 0, 0, yyvstop + 84,
  yycrank + 0, 0, yyvstop + 87,
  yycrank + 9, 0, yyvstop + 90,
  yycrank + 0, 0, yyvstop + 93,
  yycrank + 0, 0, yyvstop + 96,
  yycrank + 0, 0, yyvstop + 99,
  yycrank + 0, 0, yyvstop + 102,
  yycrank + 0, 0, yyvstop + 105,
  yycrank + 0, 0, yyvstop + 108,
  yycrank + 0, 0, yyvstop + 111,
  yycrank + -440, 0, yyvstop + 114,
  yycrank + 0, 0, yyvstop + 117,
  yycrank + 0, 0, yyvstop + 121,
  yycrank + -12, yysvec + 43, yyvstop + 124,
  yycrank + -96, yysvec + 43, yyvstop + 128,
  yycrank + -14, yysvec + 43, yyvstop + 132,
  yycrank + -493, yysvec + 11, yyvstop + 136,
  yycrank + -16, yysvec + 43, yyvstop + 139,
  yycrank + -474, yysvec + 43, yyvstop + 143,
  yycrank + -98, yysvec + 43, yyvstop + 147,
  yycrank + -100, yysvec + 43, yyvstop + 151,
  yycrank + -499, yysvec + 16, yyvstop + 155,
  yycrank + -188, yysvec + 43, yyvstop + 158,
  yycrank + -191, yysvec + 43, yyvstop + 162,
  yycrank + -558, 0, yyvstop + 166,
  yycrank + -207, yysvec + 43, yyvstop + 170,
  yycrank + -222, yysvec + 43, yyvstop + 174,
  yycrank + -229, yysvec + 43, yyvstop + 178,
  yycrank + -573, yysvec + 43, yyvstop + 182,
  yycrank + -592, yysvec + 43, yyvstop + 187,
  yycrank + -603, yysvec + 43, yyvstop + 192,
  yycrank + -627, yysvec + 43, yyvstop + 196,
  yycrank + -268, yysvec + 43, yyvstop + 200,
  yycrank + -256, yysvec + 43, yyvstop + 205,
  yycrank + -258, yysvec + 43, yyvstop + 209,
  yycrank + -285, yysvec + 43, yyvstop + 213,
  yycrank + -287, yysvec + 43, yyvstop + 217,
  yycrank + -321, yysvec + 43, yyvstop + 221,
  yycrank + -349, yysvec + 43, yyvstop + 225,
  yycrank + -351, yysvec + 43, yyvstop + 229,
  yycrank + -385, yysvec + 43, yyvstop + 233,
  yycrank + -387, yysvec + 43, yyvstop + 237,
  yycrank + -404, yysvec + 43, yyvstop + 241,
  yycrank + -406, yysvec + 43, yyvstop + 245,
  yycrank + -409, yysvec + 43, yyvstop + 249,
  yycrank + -411, yysvec + 43, yyvstop + 253,
  yycrank + -448, yysvec + 43, yyvstop + 257,
  yycrank + -451, yysvec + 43, yyvstop + 261,
  yycrank + 0, yysvec + 9, yyvstop + 265,
  yycrank + -19, yysvec + 11, 0,
  yycrank + 0, 0, yyvstop + 267,
  yycrank + 667, 0, 0,
  yycrank + 0, yysvec + 13, yyvstop + 269,
  yycrank + 16, 0, 0,
  yycrank + -698, 0, 0,
  yycrank + 748, yysvec + 13, yyvstop + 271,
  yycrank + 0, 0, yyvstop + 274,
  yycrank + 758, 0, yyvstop + 276,
  yycrank + 780, yysvec + 26, yyvstop + 278,
  yycrank + 812, 0, 0,
  yycrank + 841, 0, 0,
  yycrank + 0, 0, yyvstop + 280,
  yycrank + 851, 0, 0,
  yycrank + 0, yysvec + 26, yyvstop + 282,
  yycrank + 0, 0, yyvstop + 284,
  yycrank + 0, yysvec + 27, yyvstop + 286,
  yycrank + 0, 0, yyvstop + 288,
  yycrank + -469, yysvec + 43, yyvstop + 290,
  yycrank + -534, yysvec + 43, yyvstop + 292,
  yycrank + -875, yysvec + 11, yyvstop + 295,
  yycrank + -492, yysvec + 43, yyvstop + 297,
  yycrank + -926, 0, yyvstop + 300,
  yycrank + -999, yysvec + 43, yyvstop + 302,
  yycrank + -569, yysvec + 43, yyvstop + 305,
  yycrank + -1020, yysvec + 87, yyvstop + 307,
  yycrank + 0, 0, yyvstop + 309,
  yycrank + 0, 0, yyvstop + 311,
  yycrank + 0, 0, yyvstop + 313,
  yycrank + 0, 0, yyvstop + 315,
  yycrank + -1064, 0, yyvstop + 317,
  yycrank + -937, yysvec + 43, yyvstop + 321,
  yycrank + -1079, yysvec + 43, yyvstop + 324,
  yycrank + -948, yysvec + 43, yyvstop + 327,
  yycrank + -1094, yysvec + 43, yyvstop + 329,
  yycrank + -503, yysvec + 43, yyvstop + 331,
  yycrank + -644, yysvec + 43, yyvstop + 334,
  yycrank + -1114, yysvec + 43, yyvstop + 336,
  yycrank + -510, yysvec + 43, yyvstop + 339,
  yycrank + -616, yysvec + 43, yyvstop + 342,
  yycrank + -505, yysvec + 11, 0,
  yycrank + -21, yysvec + 11, 0,
  yycrank + 1143, 0, 0,
  yycrank + 0, 0, yyvstop + 345,
  yycrank + 0, yysvec + 86, 0,
  yycrank + 0, yysvec + 86, yyvstop + 347,
  yycrank + 1175, 0, 0,
  yycrank + 1198, 0, 0,
  yycrank + 1216, 0, 0,
  yycrank + 1248, yysvec + 13, yyvstop + 349,
  yycrank + 1276, 0, 0,
  yycrank + 232, 0, 0,
  yycrank + 0, yysvec + 133, yyvstop + 351,
  yycrank + 14, yysvec + 95, yyvstop + 353,
  yycrank + -1310, yysvec + 11, yyvstop + 355,
  yycrank + -1335, yysvec + 11, yyvstop + 357,
  yycrank + -1299, yysvec + 43, yyvstop + 359,
  yycrank + -623, yysvec + 43, yyvstop + 361,
  yycrank + -685, yysvec + 43, yyvstop + 364,
  yycrank + -729, yysvec + 43, yyvstop + 366,
  yycrank + -1042, yysvec + 43, yyvstop + 369,
  yycrank + -740, yysvec + 43, yyvstop + 371,
  yycrank + -1364, yysvec + 43, yyvstop + 373,
  yycrank + -1400, 0, yyvstop + 375,
  yycrank + -1421, yysvec + 43, yyvstop + 378,
  yycrank + -831, yysvec + 43, yyvstop + 380,
  yycrank + -1053, yysvec + 43, yyvstop + 382,
  yycrank + -1430, yysvec + 43, yyvstop + 385,
  yycrank + -1329, yysvec + 11, 0,
  yycrank + -22, yysvec + 11, 0,
  yycrank + -86, yysvec + 11, 0,
  yycrank + 1463, 0, 0,
  yycrank + 770, yysvec + 129, 0,
  yycrank + 1501, yysvec + 130, 0,
  yycrank + 1491, 0, 0,
  yycrank + 1524, yysvec + 13, yyvstop + 388,
  yycrank + 1534, 0, 0,
  yycrank + 0, yysvec + 158, yyvstop + 391,
  yycrank + 0, 0, yyvstop + 393,
  yycrank + -1594, yysvec + 11, yyvstop + 395,
  yycrank + -1613, yysvec + 11, yyvstop + 397,
  yycrank + -1619, yysvec + 11, yyvstop + 399,
  yycrank + -1583, yysvec + 43, yyvstop + 401,
  yycrank + -971, yysvec + 43, yyvstop + 403,
  yycrank + -1648, yysvec + 43, yyvstop + 405,
  yycrank + -1442, yysvec + 43, yyvstop + 407,
  yycrank + -1679, yysvec + 43, yyvstop + 409,
  yycrank + -1445, yysvec + 43, yyvstop + 413,
  yycrank + -1683, yysvec + 43, yyvstop + 415,
  yycrank + -672, yysvec + 43, yyvstop + 418,
  yycrank + 0, yysvec + 156, yyvstop + 421,
  yycrank + -1699, yysvec + 43, yyvstop + 423,
  0, 0, 0
};
struct yywork *yytop = yycrank + 1776;
struct yysvf *yybgin = yysvec + 1;
char yymatch[] = {
  00, 01, 01, 01, 01, 01, 01, 01,
  01, 011, 012, 01, 01, 01, 01, 01,
  01, 01, 01, 01, 01, 01, 01, 01,
  01, 01, 01, 01, 01, 01, 01, 01,
  01, 01, '"', 01, '$', 01, 01, 01,
  01, 01, 011, 01, 01, 01, '$', '/',
  '0', '1', '1', '1', '1', '1', '1', '1',
  '8', '8', ':', 01, 01, 01, 01, 01,
  01, 'A', 'A', 'A', 'A', 'A', 'A', '$',
  '$', '$', '$', '$', '$', '$', '$', '$',
  '$', '$', '$', '$', '$', '$', '$', '$',
  'X', '$', '$', 01, '"', 01, 01, '$',
  01, 'a', 'a', 'a', 'a', 'a', 'a', '$',
  '$', '$', '$', '$', '$', '$', '$', '$',
  '$', '$', '$', '$', '$', '$', '$', '$',
  'X', '$', '$', 01, 01, 01, 01, 01,
  01, 01, 01, 01, 01, 01, 01, 01,
  01, 01, 01, 01, 01, 01, 01, 01,
  01, 01, 01, 01, 01, 01, 01, 01,
  01, 01, 01, 01, 01, 01, 01, 01,
  01, 01, 01, 01, 01, 01, 01, 01,
  01, 01, 01, 01, 01, 01, 01, 01,
  01, 01, 01, 01, 01, 01, 01, 01,
  01, 01, 01, 01, 01, 01, 01, 01,
  01, 01, 01, 01, 01, 01, 01, 01,
  01, 01, 01, 01, 01, 01, 01, 01,
  01, 01, 01, 01, 01, 01, 01, 01,
  01, 01, 01, 01, 01, 01, 01, 01,
  01, 01, 01, 01, 01, 01, 01, 01,
  01, 01, 01, 01, 01, 01, 01, 01,
  01, 01, 01, 01, 01, 01, 01, 01,
  01, 01, 01, 01, 01, 01, 01, 01,
  0
};
char yyextra[] = {
  0, 0, 0, 0, 0, 0, 0, 0,
  0, 0, 0, 0, 0, 0, 0, 0,
  0, 0, 0, 0, 0, 0, 0, 0,
  0, 0, 0, 0, 0, 0, 0, 0,
  0, 0, 0, 0, 0, 0, 0, 0,
  0, 0, 0, 0, 0, 0, 0, 0,
  0, 0, 0, 0, 0, 0, 0, 0,
  0
};

int yylineno = 1;
# define YYU(x) x
# define NLSTATE yyprevious=YYNEWLINE
char yytext[YYLMAX];
struct yysvf *yylstate[YYLMAX], **yylsp, **yyolsp;
char yysbuf[YYLMAX];
char *yysptr = yysbuf;
int *yyfnd;
extern struct yysvf *yyestate;
int yyprevious = YYNEWLINE;
#if defined(__cplusplus) || defined(__STDC__)
int
yylook (void)
#else
yylook ()
#endif
{
  register struct yysvf *yystate, **lsp;
  register struct yywork *yyt;
  struct yysvf *yyz;
  int yych, yyfirst;
  struct yywork *yyr;
# ifdef LEXDEBUG
  int debug;
# endif
  char *yylastch;
  /* start off machines */
# ifdef LEXDEBUG
  debug = 0;
# endif
  yyfirst = 1;
  if (!yymorfg)
    yylastch = yytext;
  else
    {
      yymorfg = 0;
      yylastch = yytext + yyleng;
    }
  for (;;)
    {
      lsp = yylstate;
      yyestate = yystate = yybgin;
      if (yyprevious == YYNEWLINE)
        yystate++;
      for (;;)
        {
# ifdef LEXDEBUG
          if (debug)
            fprintf (yyout, "state %d\n", yystate - yysvec - 1);
# endif
          yyt = yystate->yystoff;
          if (yyt == yycrank && !yyfirst)
            {                   /* may not be any transitions */
              yyz = yystate->yyother;
              if (yyz == 0)
                break;
              if (yyz->yystoff == yycrank)
                break;
            }
          *yylastch++ = yych = input ();
          if (yylastch > &yytext[YYLMAX])
            {
              fprintf (yyout, "Input string too long, limit %d\n", YYLMAX);
              exit (1);
            }
          yyfirst = 0;
        tryagain:
# ifdef LEXDEBUG
          if (debug)
            {
              fprintf (yyout, "char ");
              allprint (yych);
              putchar ('\n');
            }
# endif
          yyr = yyt;
          if ((long int) yyt > (long int) yycrank)
            {
              yyt = yyr + yych;
              if (yyt <= yytop && yyt->verify + yysvec == yystate)
                {
                  if (yyt->advance + yysvec == YYLERR)  /* error transitions */
                    {
                      unput (*--yylastch);
                      break;
                    }
                  *lsp++ = yystate = yyt->advance + yysvec;
                  if (lsp > &yylstate[YYLMAX])
                    {
                      fprintf (yyout, "Input string too long, limit %d\n",
                               YYLMAX);
                      exit (1);
                    }
                  goto contin;
                }
            }
# ifdef YYOPTIM
          else if ((long int) yyt < (long int) yycrank)
            {                   /* r < yycrank */
              yyt = yyr = yycrank + (yycrank - yyt);
# ifdef LEXDEBUG
              if (debug)
                fprintf (yyout, "compressed state\n");
# endif
              yyt = yyt + yych;
              if (yyt <= yytop && yyt->verify + yysvec == yystate)
                {
                  if (yyt->advance + yysvec == YYLERR)  /* error transitions */
                    {
                      unput (*--yylastch);
                      break;
                    }
                  *lsp++ = yystate = yyt->advance + yysvec;
                  if (lsp > &yylstate[YYLMAX])
                    {
                      fprintf (yyout, "Input string too long, limit %d\n",
                               YYLMAX);
                      exit (1);
                    }
                  goto contin;
                }
              yyt = yyr + YYU (yymatch[yych]);
# ifdef LEXDEBUG
              if (debug)
                {
                  fprintf (yyout, "try fall back character ");
                  allprint (YYU (yymatch[yych]));
                  putchar ('\n');
                }
# endif
              if (yyt <= yytop && yyt->verify + yysvec == yystate)
                {
                  if (yyt->advance + yysvec == YYLERR)  /* error transition */
                    {
                      unput (*--yylastch);
                      break;
                    }
                  *lsp++ = yystate = yyt->advance + yysvec;
                  if (lsp > &yylstate[YYLMAX])
                    {
                      fprintf (yyout, "Input string too long, limit %d\n",
                               YYLMAX);
                      exit (1);
                    }
                  goto contin;
                }
            }
          if ((yystate = yystate->yyother)
              && (yyt = yystate->yystoff) != yycrank)
            {
# ifdef LEXDEBUG
              if (debug)
                fprintf (yyout, "fall back to state %d\n",
                         yystate - yysvec - 1);
# endif
              goto tryagain;
            }
# endif
          else
            {
              unput (*--yylastch);
              break;
            }
        contin:
# ifdef LEXDEBUG
          if (debug)
            {
              fprintf (yyout, "state %d char ", yystate - yysvec - 1);
              allprint (yych);
              putchar ('\n');
            }
# endif
          ;
        }
# ifdef LEXDEBUG
      if (debug)
        {
          fprintf (yyout, "stopped at %d with ", *(lsp - 1) - yysvec - 1);
          allprint (yych);
          putchar ('\n');
        }
# endif
      while (lsp-- > yylstate)
        {
          *yylastch-- = 0;
          if (*lsp != 0 && (yyfnd = (*lsp)->yystops) && *yyfnd > 0)
            {
              yyolsp = lsp;
              if (yyextra[*yyfnd])
                {               /* must backup */
                  while (yyback ((*lsp)->yystops, -*yyfnd) != 1
                         && lsp > yylstate)
                    {
                      lsp--;
                      unput (*yylastch--);
                    }
                }
              yyprevious = YYU (*yylastch);
              yylsp = lsp;
              yyleng = yylastch - yytext + 1;
              yytext[yyleng] = 0;
# ifdef LEXDEBUG
              if (debug)
                {
                  fprintf (yyout, "\nmatch ");
                  sprint (yytext);
                  fprintf (yyout, " action %d\n", *yyfnd);
                }
# endif
              return (*yyfnd++);
            }
          unput (*yylastch);
        }
      if (yytext[0] == 0 /* && feof(yyin) */ )
        {
          yysptr = yysbuf;
          return (0);
        }
      yyprevious = yytext[0] = input ();
      if (yyprevious > 0)
        output (yyprevious);
      yylastch = yytext;
# ifdef LEXDEBUG
      if (debug)
        putchar ('\n');
# endif
    }
}
#if defined(__cplusplus) || defined(__STDC__)
int
yyback (int *p, int m)
#else
yyback (p, m)
     int *p;
#endif
{
  if (p == 0)
    return (0);
  while (*p)
    {
      if (*p++ == m)
        return (1);
    }
  return (0);
}
        /* the following are only used in the lex library */
#if defined(__cplusplus) || defined(__STDC__)
int
yyinput (void)
#else
yyinput ()
#endif
{
  return (input ());
}
#if defined(__cplusplus) || defined(__STDC__)
void
yyoutput (int c)
#else
yyoutput (c)
     int c;
#endif
{
  output (c);
}
#if defined(__cplusplus) || defined(__STDC__)
void
yyunput (int c)
#else
yyunput (c)
     int c;
#endif
{
  unput (c);
}
