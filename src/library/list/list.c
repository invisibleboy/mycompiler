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
/* The IMPACT Research Group (www.crhc.uiuc.edu/IMPACT)                      */
/*****************************************************************************\
 *
 * Copyright Notices/Identification of Licensor(s) of
 * Original Software in the File
 *
 * Copyright 1990-1999 The Board of Trustees of the University of Illinois
 * Contact: Research and Technology Management Office,
 * University of Illinois at Urbana-Champaign;
 * FAX: 217-244-3716, or email: rtmo@uiuc.edu
 *
 * All rights reserved by the foregoing, respectively.
 *
 * This is licensed software.  The software license agreement with
 * the University of Illinois specifies the terms and conditions
 * for use and redistribution.
 *
\*****************************************************************************/
/* 10/29/02 REK Adding config.h */
#include <config.h>

# include "FIX_list.h"
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
# define REJECT { nstr = yyreject(); goto yyfussy;}
int yyleng;
extern char yytext[];
int yymorfg;
extern char *yysptr, yysbuf[];
int yytchar;
/* Initialization to stdin/stdout not portable.  Initialization
 * moved to lexOpen()! -JCG 5/99
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
 *      File:   list.c
 *      Author: Pohua Chang, Wen-mei Hwu
\*****************************************************************************/



/*---------------------------------------------------------------------------
 *--------------------------------------------------------------------------*/
#include <stdio.h>
#include <stdlib.h>
#include <ctype.h>
#include <malloc.h>
#include <string.h>
#include <library/i_types.h>
#include "list.h"

#undef YYLMAX

/* BCC - extended YYLMAX from 2048 to 2500 for EDG/132.ijpeg - 6/27/96*/
#define YYLMAX 2500

static int lexOpen (char *file_name, int inclusion);
static int lexClose (char *file_name);
static int lexIntLit (void);
static int lexRealLit (void);
static int lexStringLit (void);
static int lexCharLit (void);
static int lexIdentifier (void);
static void lexError (char *);
static char *FindString (char *str);

int yywrap (void);


int TokenType;                  /* token type */
char *TokenName;                /* token string */
ITintmax TokenInteger;          /* scalar value */
double TokenReal;               /* floating point value */
static int lex_error = 0;              /* number of lexical errors encountered */

#define ToState(n) BEGIN(n)     /* change lex state */
#define DefaultState 0          /* default lex state */


/* BCC - make '-' as a legal char for id names - 2/3/96 */
# define COMMENT 2
# define YYNEWLINE 10
int
yylex ()
{
  int nstr;
  extern int yyprevious;
  while ((nstr = yylook ()) >= 0)
  yyfussy:switch (nstr)
      {
      case 0:
        if (yywrap ())
          return (0);
        break;
      case 1:

        {
        }
        break;
      case 2:

        {
        }
        break;
      case 3:

        {
        }
        break;
      case 4:

        {
          ToState (DefaultState);
        }
        break;
      case 5:

        {
        }
        break;
      case 6:

        {
        }
        break;
      case 7:

        {
        }
        break;
      case 8:

        {
          ToState (COMMENT);
        }
        break;
      case 9:

        {
        }
        break;
      case 10:

        {
        }
        break;
      case 11:

        {
        }
        break;
      case 12:

        {
        }
        break;
      case 13:

        {
          return lexIntLit ();
        }
        break;
      case 14:

        {
          return lexIdentifier ();
        }
        break;
      case 15:

        {
          return lexRealLit ();
        }
        break;
      case 16:

        {
          return lexRealLit ();
        }
        break;
      case 17:

        {
          return lexRealLit ();
        }
        break;
      case 18:

        {
          return lexStringLit ();
        }
        break;
      case 19:

        {
          return lexCharLit ();
        }
        break;
      case 20:

        {
          return (TokenType = TokenChar (0));
        }
        break;
      case 21:

        {
          return (TokenType = TokenChar (0));
        }
        break;
      case 22:

        {
          return (TokenType = TokenChar (0));
        }
        break;
      case 23:

        {
          return (TokenType = TokenChar (0));
        }
        break;
      case 24:

        {
          return (TokenType = TokenChar (0));
        }
        break;
      case 25:

        {
          return (TokenType = TokenChar (0));
        }
        break;
      case 26:

        {
          return (TokenType = TokenChar (0));
        }
        break;
      case 27:

        {
          return (TokenType = TokenChar (0));
        }
        break;
      case 28:

        {
          return (TokenType = TokenChar (0));
        }
        break;
      case 29:

        {
          return (TokenType = TokenChar (0));
        }
        break;
      case 30:

        {
          return (TokenType = TokenChar (0));
        }
        break;
      case 31:

        {
          return (TokenType = TokenChar (0));
        }
        break;
      case 32:

        {
          return (TokenType = TokenChar (0));
        }
        break;
      case 33:

        {
          return (TokenType = TokenChar (0));
        }
        break;
      case 34:

        {
          return (TokenType = TokenChar (0));
        }
        break;
      case 35:

        {
          return (TokenType = TokenChar (0));
        }
        break;
      case 36:

        {
          return (TokenType = TokenChar (0));
        }
        break;
      case 37:

        {
          return (TokenType = TokenChar (0));
        }
        break;
      case 38:

        {
          return (TokenType = TokenChar (0));
        }
        break;
      case 39:

        {
          return (TokenType = TokenChar (0));
        }
        break;
      case 40:

        {
          lexError (TokenBuffer (0));
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

/*----------------------------------------------------------------*/
#define MAX_LEVEL       20
static int stack_top = 0;
static FILE *stack[MAX_LEVEL];
static char *fname[MAX_LEVEL];
static char *current_file = "??";

/* Lexical Error */
static void
lexError (char *message)
{
  lex_error++;
  fprintf (stderr, "lexical error : %s (in %s)\n", message, current_file);
  exit (-1);
}

static void
Push (FILE * fpt, char *name)
{
  stack[stack_top] = fpt;
  fname[stack_top] = name;
  stack_top += 1;
  if (stack_top >= MAX_LEVEL)
    lexError ("too many levels of file inclusion");
}

static FILE *
Pop (void)
{
  if (stack_top <= 0)
    return stdin;               /* default */
  stack_top -= 1;
  return stack[stack_top];
}

static char *
LastFile (void)
{
  if (stack_top <= 0)
    return "??";                /* default */
  return fname[stack_top - 1];
}

/* 1 for no wrap. */
int
yywrap (void)
{
  return 1;
}

/* BCC - Test if the file exists - 7/14/97 */
static int
lexExist (char *file_name, int inclusion)
{
  FILE *fpt;
  char *last_file;

  last_file = current_file;
  /*
   *      If the current_file is specified from the
   *      root directory, do nothing.
   *      Otherwise, add the prefix of the last file
   *      to the front of the file_name.
   */
  if (inclusion && (file_name[0] != '/'))
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
      file_name = FindString (temp1);
    }

  if (!strcmp (file_name, "stdin"))
    {
      return 1;
    }
  else
    {                           /* open new file */
      fpt = fopen (file_name, "r");
      if (fpt == NULL)
        {
          return 0;
        }
      return 1;
    }
}

/* Set Up The Input File */
int
old_lexOpen(char *file_name, int inclusion)
{
   return lexOpen (file_name, inclusion);
}

static int
lexOpen (char *file_name, int inclusion)
{
  FILE *fpt;
  char *last_file;

  Push (yyin, current_file);            /** save the previous fpt **/
  last_file = current_file;

  /* Moved initialization of yyin and yyout here -JCG 5/99 */
  yyin = stdin;
  yyout = stdout;

  /*
   *      If the current_file is specified from the
   *      root directory, do nothing.
   *      Otherwise, add the prefix of the last file
   *      to the front of the file_name.
   */
  if (inclusion && (file_name[0] != '/'))
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
      file_name = FindString (temp1);
    }
  current_file = file_name;             /** we have a new file **/

  if (!strcmp (file_name, "stdin"))
    {
      yyin = stdin;
      return 1;
    }
  else
    {                           /* open new file */
      fpt = fopen (file_name, "rt");
      if (fpt == NULL)
        {
          fprintf (stderr, "> %s\n", file_name);
          lexError ("can not open file");
          return 0;
        }
      yyin = fpt;
      return 1;
    }
}

/* Close an Input File */
int
old_lexClose (char *file_name)
{
   return lexClose (file_name);
}

static int
lexClose (char *file_name)
{
  if (strcmp (file_name, "stdin"))
    fclose (yyin);
  current_file = LastFile ();
  yyin = Pop ();
  return 1;
}

static FILE *
SwitchFile (FILE * fpt)
{
  FILE *old_yyin;
  old_yyin = yyin;
  if (fpt != NULL)
    {
      yyin = fpt;
      return old_yyin;
    }
  lexError ("cannot switch yyin to NULL");
  return 0;
}

/*----------------------------------------------------------------*/
/* Process an integer Literal */
#ifdef __linux__
/* ADA: 7/9/96: Linux libc atoi() will return MAX_LONG if input string is too
        large.  This is disastrous when reading a huge number still in unsigned
        long range.  Use the following routine instead.  Different name is used
        to avoid conflict with linux header files */
static int
atoi_linux (char *s)
{
  /* 
   * atoi() exsits to allow overflow int in s
   */
  char *p = s;
  int neg, ret;

  if ((p = s) == NULL)
    return 0;

  while (isspace (*p))
    p++;

  if (*p == '-')
    {
      p++;
      neg = 1;
    }
  else if (*p == '+')
    {
      p++;
      neg = 0;
    }
  else
    {
      neg = 0;
    }

  for (ret = 0; *p; p++)
    if (isdigit (*p))
      {
        ret *= 10;
        ret += *p - '0';
      }
    else
      {
        break;
      }

  return (neg) ? -ret : ret;
}
#endif

static int
lexIntLit (void)
{
  if (*(TokenBuffer (0)) == '-')
    sscanf (TokenBuffer (0), ITintmaxformat, &TokenInteger);
  else
    sscanf (TokenBuffer (0), ITuintmaxformat, (ITuintmax *) & TokenInteger);
  return (TokenType = L_INT);
}

/* Process a Real Literal */
static int
lexRealLit (void)
{
  TokenReal = atof (TokenBuffer (0));
  return (TokenType = L_REAL);
}

/* Process a String Literal */
static int
lexStringLit (void)
{
  TokenName = TokenBuffer (0);
  return (TokenType = L_STRING);
}

/* Process a Character Literal */
static int
lexCharLit (void)
{
  TokenName = TokenBuffer (0);
  return (TokenType = L_CHAR);
}

/* Process an Identifier/Typeid/Keyword */
static int
lexIdentifier (void)
{
  TokenName = TokenBuffer (0);
  return (TokenType = L_ID);
}


/*--------------------------------------------------------------------------*/
/*      We will implement a binary tree string management.
 */
typedef struct String
{
  char *name;                   /* string */
  struct String *left;
  struct String *right;
}
String;

static String *stringTree = 0;

static char *
lmalloc (int size)
{
  char *new_str = (char *) malloc (size);
  if (new_str == 0)
    {
      fprintf (stderr, "LIST : malloc returns nil\n");
      exit (1);
    }
  return new_str;
}

/* String Save Function. */
static char *
strsave (char *str)
{
  char *new_str = (char *) lmalloc (strlen (str) + 1);
  strcpy (new_str, str);
  return (new_str);
}

/*      Find a string.  */
static String *
FindS (String * T, char *str)
{
  int diff;

  if (T == 0)                   /* not found */
    return 0;
  diff = strcmp (str, T->name);
  if (diff == 0)
    return (T);                 /* find a match */
  if (diff > 0)
    return (FindS (T->left, str));      /* search left */
  else
    return (FindS (T->right, str));     /* search right */
}

/*      Add a string.   */
static String *
AddS (String * T, char *str)
{
  int diff;
  String *new_str;

  if (T == 0)
    {                           /* add new string */
      new_str = (String *) lmalloc (sizeof (String));
      new_str->name = strsave (str);
      new_str->left = new_str->right = 0;
      return (new_str);
    }
  diff = strcmp (str, T->name);
  if (diff == 0)
    return (T);                 /* already exist */
  if (diff > 0)
    {
      T->left = AddS (T->left, str);    /* add to left */
      return (T);
    }
  else
    {
      T->right = AddS (T->right, str);  /* add to right */
      return (T);
    }
}

/*      This function maintains all define-once-only names.
 *      It maintains a table of a list of unique names used
 *      by the entire program. This will save some space.
 */
char *
old_FindString (char *str)
{
  return FindString (str);
}

static char *
FindString (char *str)
{
  String *ptr;
  ptr = FindS (stringTree, str);
  if (ptr == 0)
    {
      /*      This is extremely inefficient.
       *      We should change the two routines
       *      to combine find and insert.
       */
      stringTree = AddS (stringTree, str);
      ptr = FindS (stringTree, str);
    }
  return (ptr->name);
}

/*--------------------------------------------------------------------------*/
static LIST dead_list = 0;
static int dead_list_length = 0;

/*      allocate space for a new node.  */
static LIST
NewNode (void)
{
  LIST new_node;
  if (dead_list != 0)
    {
      new_node = dead_list;
      dead_list = SiblingOf (dead_list);
      NodeType (new_node) = T_NULL;
      SiblingOf (new_node) = 0;
      ChildOf (new_node) = 0;
      /* BCC - 8/8/97 */
      LastChildOf (new_node) = 0;
    }
  else
    {
      new_node = (LIST) lmalloc (sizeof (NODE));
      NodeType (new_node) = T_NULL;
      SiblingOf (new_node) = 0;
      ChildOf (new_node) = 0;
      /* BCC - 8/8/97 */
      LastChildOf (new_node) = 0;
    }
  return new_node;
}

/*      dispose space of a node and its siblings and descendants.       */
LIST DisposeNode (LIST node)
{
  if (node == 0)
    return 0;
  if (dead_list_length > 50000)
    FreeDeadList ();
  if (NodeType (node) == T_LIST)
    {
      DisposeNode (ChildOf (node));
      ChildOf (node) = 0;
    }
  DisposeNode (SiblingOf (node));

  SiblingOf (node) = dead_list; /* add node to the dead list */
  dead_list = node;             /* for recycling */
  dead_list_length++;

  return 0;
}

void
FreeDeadList ()
{
  LIST node1, node2;

  node1 = dead_list;
  while (node1)
    {
      node2 = node1;
      node1 = SiblingOf (node1);
      free (node2);
    }
  dead_list = 0;
  dead_list_length = 0;
}

/*      add a child.    */
static void
AddChildNode (LIST parent, LIST child)
{
#if 0
  LIST ptr;
#endif
  if (ChildOf (parent) == 0)
    {
      /* the first child */
      ChildOf (parent) = child;
      /* BCC - 8/8/97 */
      for (LastChildOf (parent) = child;
           SiblingOf (LastChildOf (parent));
           LastChildOf (parent) = SiblingOf (LastChildOf (parent)));
    }
  else
    {
      if (SiblingOf (LastChildOf (parent)))
        {
          fprintf (stderr, "AddChildNode: Node found after LastChild");
          exit (1);
        }
      SiblingOf (LastChildOf (parent)) = child;
      LastChildOf (parent) = child;
      for (LastChildOf (parent) = child;
           SiblingOf (LastChildOf (parent));
           LastChildOf (parent) = SiblingOf (LastChildOf (parent)));
      /* BCC - this old implementation takes forever to build a 
         6000-node long list */
#if 0
      /* find last child */
      for (ptr = ChildOf (parent); SiblingOf (ptr) != 0;)
        ptr = SiblingOf (ptr);
      /* append the new child to it */
      SiblingOf (ptr) = child;
#endif
    }
}

/*
 *      Get a node or a complete list from input.
 *      It is assumed that the lexer has been properly set up.
 */
LIST GetNode (void)
{
  int node_type;
  LIST new_node, ptr;

  node_type = yylex ();         /* get token */

  if (node_type == 0)
    {                           /* end of file */
      new_node = NewNode ();
      NodeType (new_node) = T_EOF;
      return new_node;
    }

  switch (node_type)
    {
    case L_ID:                  /* identifier */
      new_node = NewNode ();
      NodeType (new_node) = T_ID;
      StringOf (new_node) = FindString (TokenName);
      break;
    case L_INT:         /* integer */
      new_node = NewNode ();
      NodeType (new_node) = T_INT;
      ScalarOf (new_node) = TokenInteger;
      break;
    case L_REAL:                /* real constant literal */
      new_node = NewNode ();
      NodeType (new_node) = T_REAL;
      RealOf (new_node) = TokenReal;
      break;
    case L_STRING:              /* string literal */
      new_node = NewNode ();
      NodeType (new_node) = T_STRING;
      StringOf (new_node) = FindString (TokenName);
      break;
    case L_CHAR:                /* character literal */
      new_node = NewNode ();
      NodeType (new_node) = T_CHAR;
      StringOf (new_node) = FindString (TokenName);
      break;
    case L_ERROR:               /* erroneous input */
      new_node = NewNode ();
      NodeType (new_node) = T_NULL;
      break;
    default:                    /* single character operator */
      if (node_type == '(')
        {
          new_node = NewNode ();
          NodeType (new_node) = T_LIST;
          ChildOf (new_node) = 0;
          while ((ptr = GetNode ()) != 0)
            AddChildNode (new_node, ptr);
        }
      else if (node_type == ')')
        {
          new_node = 0;
        }
      else
        {
          char line[10];
          sprintf (line, "'%c'", node_type);
          new_node = NewNode ();
          NodeType (new_node) = T_CHAR;
          StringOf (new_node) = FindString (line);
        }
      break;
    }
  return new_node;
}

/*      print a node.
 *      this is useful for program debugging.
 */
static FILE *FF = NULL;
void
print_node (LIST node, int indent)
{
  int i;
  LIST ptr;
  if (FF == NULL)
    FF = stdout;
  if (node == 0)
    return;
  for (i = 0; i < indent; i++)
    fprintf (FF, " ");
  switch (NodeType (node))
    {
    case T_EOF:
      fprintf (FF, "<EOF>");
      break;
    case T_NULL:
      fprintf (FF, "<NULL>");
      break;
    case T_LIST:
      fprintf (FF, "(\n");
      for (ptr = ChildOf (node); ptr != 0; ptr = SiblingOf (ptr))
        {
          print_node (ptr, indent + 2);
          fprintf (FF, "\n");
        }
      for (i = 0; i < indent; i++)
        fprintf (FF, " ");
      fprintf (FF, ")");
      break;
    case T_ID:
      fprintf (FF, "(id %s)", StringOf (node));
      break;
    case T_INT:
      fprintf (FF, "(int " ITintmaxformat ")", ScalarOf (node));
      break;
    case T_REAL:
      fprintf (FF, "(real %e)", RealOf (node));
      break;
    case T_STRING:
      fprintf (FF, "(string %s)", StringOf (node));
      break;
    case T_CHAR:
      fprintf (FF, "(char %s)", StringOf (node));
      break;
    case T_OPER:
      fprintf (FF, "(oper %c)", IntegerOf (node));
      break;
    default:
      fprintf (FF, "<ERROR>");
      break;
    }
}

/*
 *      print a node in a specified file.
 */
void
PrintNode (FILE * F, LIST node)
{
  FILE *save = FF;
  FF = F;
  print_node (node, 0);
  fprintf (F, "\n");
  FF = save;
}

LIST GetInteger (LIST list, int *val)
{
  if (list == 0)
    return 0;
  if (NodeType (list) != T_INT)
    return list;
  *val = ScalarOf (list);
  return SiblingOf (list);
}

LIST GetReal (LIST list, double *val)
{
  if (list == 0)
    return 0;
  if (NodeType (list) != T_REAL)
    return list;
  *val = RealOf (list);
  return SiblingOf (list);
}

LIST GetString (LIST list, char **val)
{
  if (list == 0)
    return 0;
  if (NodeType (list) != T_STRING)
    return list;
  *val = StringOf (list);
  return SiblingOf (list);
}

LIST GetId (LIST list, char **val)
{
  if (list == 0)
    return 0;
  if (NodeType (list) != T_ID)
    return list;
  *val = StringOf (list);
  return SiblingOf (list);
}

/* BCC - added for garbage collection - 8/22/96 */
static void
RemoveS (String * ptr)
{
  if (ptr == 0)
    return;
  RemoveS (ptr->left);
  ptr->left = 0;
  RemoveS (ptr->right);
  ptr->right = 0;
  free (ptr->name);
  free (ptr);
}

/* BCC - added for garbage collection - 8/22/96 */
void
RemoveAllString ()
{
  RemoveS (stringTree);
  stringTree = 0;
}
int yyvstop[] = {
  0,

  40,
  0,

  10,
  40,
  0,

  11,
  0,

  9,
  40,
  0,

  20,
  40,
  0,

  40,
  0,

  22,
  40,
  0,

  14,
  40,
  0,

  23,
  40,
  0,

  25,
  40,
  0,

  40,
  0,

  27,
  40,
  0,

  28,
  40,
  0,

  26,
  40,
  0,

  40,
  0,

  14,
  40,
  0,

  14,
  40,
  0,

  13,
  40,
  0,

  34,
  40,
  0,

  39,
  40,
  0,

  35,
  40,
  0,

  37,
  40,
  0,

  36,
  40,
  0,

  33,
  40,
  0,

  21,
  40,
  0,

  29,
  40,
  0,

  40,
  0,

  30,
  40,
  0,

  24,
  40,
  0,

  31,
  40,
  0,

  32,
  40,
  0,

  38,
  40,
  0,

  1,
  40,
  0,

  2,
  10,
  40,
  0,

  3,
  11,
  0,

  1,
  9,
  40,
  0,

  1,
  20,
  40,
  0,

  1,
  40,
  0,

  1,
  22,
  40,
  0,

  1,
  14,
  40,
  0,

  1,
  23,
  40,
  0,

  1,
  25,
  40,
  0,

  1,
  40,
  0,

  1,
  27,
  40,
  0,

  1,
  28,
  40,
  0,

  7,
  26,
  40,
  0,

  1,
  40,
  0,

  1,
  14,
  40,
  0,

  1,
  14,
  40,
  0,

  1,
  13,
  40,
  0,

  1,
  34,
  40,
  0,

  1,
  39,
  40,
  0,

  1,
  35,
  40,
  0,

  1,
  37,
  40,
  0,

  1,
  36,
  40,
  0,

  1,
  33,
  40,
  0,

  1,
  21,
  40,
  0,

  1,
  29,
  40,
  0,

  1,
  40,
  0,

  1,
  30,
  40,
  0,

  1,
  24,
  40,
  0,

  1,
  31,
  40,
  0,

  1,
  32,
  40,
  0,

  1,
  38,
  40,
  0,

  9,
  0,

  18,
  0,

  14,
  0,

  13,
  0,

  14,
  17,
  0,

  8,
  0,

  15,
  0,

  12,
  0,

  1,
  0,

  1,
  9,
  0,

  1,
  0,

  1,
  18,
  0,

  1,
  0,

  1,
  14,
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
  0,

  1,
  13,
  0,

  1,
  14,
  17,
  0,

  1,
  15,
  0,

  1,
  0,

  19,
  0,

  19,
  0,

  17,
  0,

  14,
  0,

  16,
  0,

  1,
  0,

  1,
  0,

  1,
  0,

  1,
  19,
  0,

  1,
  0,

  1,
  19,
  0,

  1,
  0,

  1,
  0,

  1,
  0,

  1,
  17,
  0,

  1,
  14,
  0,

  1,
  0,

  1,
  0,

  1,
  16,
  0,

  14,
  0,

  14,
  17,
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
  0,

  1,
  14,
  0,

  1,
  14,
  17,
  0,

  1,
  0,

  1,
  15,
  0,

  17,
  0,

  1,
  17,
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
    31, 82, 0, 0, 0, 0, 0, 0,
    0, 0, 41, 0, 41, 0, 43, 0,
    43, 0, 45, 0, 45, 0, 46, 0,
    46, 0, 48, 0, 48, 0, 0, 0,
    70, 0, 101, 0, 0, 0, 129, 0,
    130, 0, 1, 8, 1, 9, 1, 10,
    1, 11, 1, 12, 1, 13, 1, 14,
    1, 15, 1, 16, 1, 17, 1, 18,
    1, 19, 8, 69, 21, 79, 1, 20,
    1, 21, 1, 22, 41, 0, 22, 80,
    43, 0, 74, 103, 45, 0, 0, 0,
    46, 0, 1, 22, 48, 0, 1, 23,
    1, 24, 1, 25, 1, 26, 1, 27,
    1, 28, 1, 29, 1, 12, 101, 129,
    0, 0, 0, 0, 0, 0, 0, 0,
    2, 8, 2, 9, 22, 81, 2, 11,
    0, 0, 2, 13, 2, 14, 2, 15,
    2, 16, 2, 17, 2, 18, 109, 134,
    40, 0, 40, 0, 2, 20, 0, 0,
    0, 0, 1, 12, 49, 0, 49, 0,
    1, 30, 1, 31, 1, 32, 1, 33,
    55, 0, 55, 0, 1, 12, 2, 24,
    2, 25, 2, 26, 2, 27, 2, 28,
    2, 29, 77, 80, 22, 81, 40, 84,
    56, 0, 56, 0, 70, 72, 101, 72,
    3, 37, 129, 72, 130, 72, 109, 134,
    0, 0, 40, 0, 0, 0, 0, 0,
    3, 38, 3, 39, 0, 0, 49, 0,
    1, 34, 0, 0, 1, 35, 1, 36,
    77, 81, 55, 0, 0, 0, 2, 30,
    2, 31, 2, 32, 2, 33, 57, 0,
    57, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 56, 0, 0, 0, 3, 40,
    3, 41, 3, 42, 3, 43, 3, 44,
    3, 45, 3, 46, 3, 47, 3, 48,
    3, 49, 3, 50, 3, 51, 0, 0,
    0, 0, 3, 52, 3, 53, 3, 54,
    77, 81, 0, 0, 0, 0, 2, 34,
    0, 0, 2, 35, 2, 36, 3, 54,
    57, 0, 3, 55, 3, 56, 3, 57,
    3, 58, 3, 59, 3, 60, 3, 61,
    3, 44, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 4, 40, 4, 41,
    0, 0, 4, 43, 0, 0, 4, 45,
    4, 46, 4, 47, 4, 48, 4, 49,
    4, 50, 51, 0, 51, 0, 0, 0,
    4, 52, 0, 0, 0, 0, 3, 44,
    58, 0, 58, 0, 3, 62, 3, 63,
    3, 64, 3, 65, 59, 0, 59, 0,
    3, 44, 4, 56, 4, 57, 4, 58,
    4, 59, 4, 60, 4, 61, 10, 70,
    60, 0, 60, 0, 61, 0, 61, 0,
    62, 0, 62, 0, 0, 0, 10, 70,
    10, 0, 0, 0, 51, 0, 63, 0,
    63, 82, 0, 0, 51, 95, 0, 0,
    51, 96, 58, 0, 3, 66, 0, 0,
    3, 67, 3, 68, 0, 0, 59, 0,
    51, 96, 4, 62, 4, 63, 4, 64,
    4, 65, 0, 0, 0, 0, 0, 0,
    10, 71, 60, 0, 10, 70, 61, 0,
    0, 0, 62, 0, 0, 0, 0, 0,
    0, 0, 10, 70, 0, 0, 0, 0,
    63, 0, 10, 70, 10, 70, 0, 0,
    0, 0, 0, 0, 0, 0, 64, 0,
    64, 0, 0, 0, 10, 70, 0, 0,
    10, 70, 4, 66, 0, 0, 4, 67,
    4, 68, 0, 0, 19, 76, 10, 70,
    19, 77, 19, 77, 19, 77, 19, 77,
    19, 77, 19, 77, 19, 77, 19, 77,
    19, 77, 19, 77, 20, 78, 20, 78,
    20, 78, 20, 78, 20, 78, 20, 78,
    20, 78, 20, 78, 20, 78, 20, 78,
    64, 0, 0, 0, 10, 70, 12, 73,
    0, 0, 0, 0, 10, 72, 0, 0,
    0, 0, 0, 0, 0, 0, 10, 70,
    12, 73, 12, 73, 12, 73, 12, 73,
    12, 73, 12, 73, 12, 73, 12, 73,
    12, 73, 12, 73, 12, 73, 12, 73,
    12, 73, 12, 73, 65, 0, 65, 0,
    66, 0, 66, 0, 67, 0, 67, 0,
    12, 73, 12, 73, 12, 73, 12, 73,
    12, 73, 12, 73, 12, 73, 12, 73,
    12, 73, 12, 73, 12, 73, 12, 73,
    12, 73, 12, 73, 12, 73, 12, 73,
    12, 73, 12, 73, 12, 73, 12, 73,
    12, 73, 12, 73, 12, 73, 12, 73,
    12, 73, 12, 73, 0, 0, 65, 0,
    0, 0, 66, 0, 12, 73, 67, 0,
    12, 73, 12, 73, 12, 73, 12, 73,
    12, 73, 12, 73, 12, 73, 12, 73,
    12, 73, 12, 73, 12, 73, 12, 73,
    12, 73, 12, 73, 12, 73, 12, 73,
    12, 73, 12, 73, 12, 73, 12, 73,
    12, 73, 12, 73, 12, 73, 12, 73,
    12, 73, 12, 73, 15, 74, 68, 0,
    68, 0, 37, 83, 83, 0, 83, 0,
    42, 85, 0, 0, 15, 74, 15, 0,
    0, 0, 37, 0, 37, 0, 0, 0,
    0, 0, 42, 0, 76, 109, 76, 109,
    76, 109, 76, 109, 76, 109, 76, 109,
    76, 109, 76, 109, 76, 109, 76, 109,
    86, 0, 86, 0, 0, 0, 117, 0,
    117, 0, 0, 0, 0, 0, 15, 74,
    68, 0, 15, 74, 37, 83, 83, 0,
    37, 83, 42, 86, 0, 0, 42, 85,
    15, 74, 0, 0, 37, 0, 37, 83,
    15, 74, 15, 74, 42, 85, 37, 83,
    37, 83, 0, 0, 42, 85, 42, 85,
    100, 0, 15, 74, 0, 0, 15, 74,
    37, 83, 86, 0, 37, 83, 42, 85,
    117, 0, 42, 85, 15, 74, 84, 0,
    84, 0, 37, 83, 44, 0, 44, 0,
    42, 85, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 47, 89, 0, 0,
    0, 0, 0, 0, 89, 0, 89, 0,
    0, 0, 0, 0, 0, 0, 47, 0,
    0, 0, 15, 74, 84, 84, 0, 0,
    37, 83, 15, 75, 100, 101, 42, 85,
    0, 0, 44, 88, 15, 74, 42, 87,
    84, 0, 37, 83, 0, 0, 44, 0,
    42, 85, 0, 0, 44, 88, 44, 88,
    44, 88, 44, 88, 0, 0, 47, 89,
    89, 117, 47, 89, 0, 0, 89, 0,
    0, 0, 44, 88, 0, 0, 44, 88,
    47, 89, 50, 91, 118, 0, 118, 0,
    47, 89, 47, 89, 44, 88, 0, 0,
    0, 0, 50, 92, 50, 93, 119, 0,
    119, 0, 47, 89, 100, 128, 47, 89,
    0, 0, 0, 0, 100, 72, 0, 0,
    95, 0, 95, 0, 47, 89, 0, 0,
    52, 0, 52, 0, 0, 0, 0, 0,
    0, 0, 44, 88, 0, 0, 0, 0,
    118, 117, 0, 0, 50, 91, 118, 0,
    50, 91, 0, 0, 44, 88, 54, 0,
    54, 0, 119, 117, 50, 0, 50, 91,
    119, 0, 47, 89, 0, 0, 50, 94,
    50, 91, 47, 90, 0, 0, 52, 88,
    0, 0, 95, 0, 47, 89, 0, 0,
    50, 91, 52, 0, 50, 91, 95, 123,
    52, 88, 52, 88, 52, 88, 52, 97,
    0, 0, 50, 91, 0, 0, 95, 123,
    53, 0, 53, 0, 0, 0, 52, 97,
    54, 0, 52, 88, 0, 0, 0, 0,
    54, 98, 0, 0, 54, 96, 0, 0,
    52, 88, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 54, 96, 0, 0,
    50, 91, 0, 0, 0, 0, 0, 0,
    0, 0, 148, 0, 148, 0, 53, 88,
    72, 70, 50, 91, 0, 0, 54, 99,
    0, 0, 53, 79, 0, 0, 52, 88,
    53, 88, 53, 88, 53, 88, 53, 88,
    0, 0, 0, 0, 0, 0, 0, 0,
    52, 88, 0, 0, 0, 0, 53, 88,
    0, 0, 53, 88, 0, 0, 72, 70,
    72, 70, 72, 70, 72, 70, 72, 70,
    53, 88, 72, 70, 148, 0, 148, 83,
    0, 0, 148, 88, 148, 88, 54, 99,
    0, 0, 0, 0, 72, 100, 72, 101,
    72, 101, 72, 101, 72, 101, 72, 101,
    72, 101, 72, 101, 0, 0, 98, 0,
    98, 0, 0, 0, 0, 0, 53, 88,
    0, 0, 72, 70, 0, 0, 0, 0,
    99, 0, 99, 0, 126, 0, 126, 0,
    53, 88, 0, 0, 78, 78, 78, 78,
    78, 78, 78, 78, 78, 78, 78, 78,
    78, 78, 78, 78, 78, 78, 78, 78,
    0, 0, 0, 0, 0, 0, 0, 0,
    75, 104, 0, 0, 72, 102, 0, 0,
    98, 0, 0, 0, 72, 70, 78, 110,
    75, 104, 75, 0, 98, 98, 72, 70,
    72, 70, 99, 0, 99, 126, 126, 0,
    72, 70, 0, 0, 98, 98, 99, 127,
    0, 0, 126, 127, 0, 0, 0, 0,
    72, 70, 149, 0, 149, 0, 99, 127,
    72, 70, 126, 127, 72, 70, 98, 125,
    72, 70, 75, 104, 72, 102, 75, 104,
    0, 0, 0, 0, 75, 105, 78, 110,
    0, 0, 0, 0, 75, 104, 0, 0,
    0, 0, 0, 0, 75, 104, 75, 106,
    75, 107, 75, 107, 75, 107, 75, 107,
    75, 107, 75, 107, 75, 107, 75, 104,
    0, 0, 75, 104, 149, 0, 149, 83,
    0, 0, 149, 88, 149, 88, 98, 125,
    75, 104, 80, 80, 80, 80, 80, 80,
    80, 80, 80, 80, 80, 80, 80, 80,
    80, 80, 80, 80, 80, 80, 85, 85,
    96, 0, 96, 0, 128, 0, 0, 0,
    0, 0, 116, 0, 116, 0, 0, 0,
    85, 0, 0, 0, 80, 111, 75, 108,
    0, 0, 0, 0, 0, 0, 0, 0,
    81, 112, 0, 0, 81, 112, 0, 0,
    75, 104, 81, 113, 81, 113, 81, 113,
    81, 113, 81, 113, 81, 113, 81, 113,
    81, 113, 81, 113, 81, 113, 0, 0,
    85, 86, 96, 0, 85, 85, 0, 0,
    0, 0, 96, 98, 116, 0, 96, 96,
    128, 130, 85, 85, 80, 111, 0, 0,
    116, 142, 85, 85, 85, 85, 96, 96,
    128, 130, 0, 0, 0, 0, 0, 0,
    116, 142, 0, 0, 85, 85, 0, 0,
    85, 85, 128, 130, 0, 0, 0, 0,
    96, 99, 116, 142, 0, 0, 85, 85,
    87, 83, 107, 103, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0,
    87, 0, 87, 70, 107, 132, 107, 132,
    107, 132, 107, 132, 107, 132, 107, 132,
    107, 132, 107, 132, 0, 0, 0, 0,
    128, 72, 0, 0, 85, 85, 0, 0,
    0, 0, 128, 130, 85, 87, 0, 0,
    96, 99, 116, 142, 0, 0, 85, 85,
    87, 85, 87, 85, 87, 85, 87, 85,
    87, 85, 0, 0, 87, 85, 0, 0,
    0, 0, 87, 0, 87, 83, 0, 0,
    0, 0, 0, 0, 87, 83, 87, 114,
    87, 115, 87, 115, 87, 115, 87, 115,
    87, 115, 87, 115, 87, 115, 87, 83,
    0, 0, 87, 83, 0, 0, 0, 0,
    0, 0, 0, 0, 87, 85, 0, 0,
    87, 83, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 87, 83, 87, 83,
    87, 83, 87, 83, 87, 83, 87, 83,
    87, 83, 87, 83, 87, 83, 87, 83,
    87, 83, 87, 83, 87, 83, 87, 83,
    87, 83, 87, 83, 87, 83, 87, 116,
    87, 83, 87, 83, 88, 0, 88, 0,
    121, 0, 121, 0, 87, 83, 0, 0,
    87, 85, 0, 0, 87, 83, 87, 83,
    87, 83, 0, 0, 87, 83, 87, 83,
    87, 83, 87, 83, 87, 83, 87, 83,
    87, 83, 0, 0, 87, 83, 87, 83,
    87, 83, 0, 0, 87, 83, 0, 0,
    87, 83, 88, 88, 87, 83, 0, 0,
    87, 83, 87, 83, 121, 117, 88, 0,
    0, 0, 121, 0, 88, 88, 88, 88,
    88, 88, 88, 88, 0, 0, 121, 144,
    0, 0, 90, 118, 120, 0, 120, 0,
    0, 0, 88, 88, 0, 0, 88, 88,
    106, 103, 0, 0, 90, 0, 0, 0,
    0, 0, 0, 0, 88, 88, 0, 0,
    0, 0, 106, 107, 106, 107, 106, 107,
    106, 107, 106, 107, 106, 107, 106, 107,
    106, 107, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0,
    120, 117, 0, 0, 90, 118, 120, 0,
    90, 118, 88, 88, 0, 0, 90, 119,
    0, 0, 120, 121, 0, 0, 90, 118,
    0, 0, 0, 0, 88, 88, 90, 118,
    90, 120, 90, 121, 90, 121, 90, 121,
    90, 121, 90, 121, 90, 121, 90, 121,
    90, 118, 106, 131, 90, 118, 97, 83,
    127, 0, 127, 0, 0, 0, 0, 0,
    110, 135, 90, 118, 110, 136, 97, 0,
    97, 0, 110, 137, 110, 137, 110, 137,
    110, 137, 110, 137, 110, 137, 110, 137,
    110, 137, 110, 137, 110, 137, 125, 0,
    125, 0, 120, 143, 147, 0, 147, 0,
    0, 0, 0, 0, 0, 0, 0, 0,
    90, 122, 106, 131, 0, 0, 0, 0,
    97, 83, 127, 0, 97, 88, 0, 0,
    0, 0, 90, 118, 0, 0, 127, 127,
    97, 0, 97, 83, 0, 0, 97, 88,
    97, 88, 97, 88, 97, 97, 127, 127,
    0, 0, 0, 0, 0, 0, 0, 0,
    125, 0, 125, 150, 97, 97, 147, 0,
    97, 88, 0, 0, 125, 151, 0, 0,
    0, 0, 147, 153, 0, 0, 97, 88,
    0, 0, 0, 0, 125, 151, 97, 124,
    0, 0, 147, 153, 102, 130, 102, 130,
    102, 130, 102, 130, 102, 130, 102, 130,
    102, 130, 102, 130, 102, 130, 102, 130,
    0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 97, 88, 102, 130,
    102, 130, 102, 130, 102, 130, 102, 130,
    102, 130, 0, 0, 0, 0, 97, 88,
    0, 0, 0, 0, 0, 0, 97, 124,
    108, 103, 132, 104, 132, 104, 132, 104,
    132, 104, 132, 104, 132, 104, 132, 104,
    132, 104, 108, 133, 108, 133, 108, 133,
    108, 133, 108, 133, 108, 133, 108, 133,
    108, 133, 108, 133, 108, 133, 102, 130,
    102, 130, 102, 130, 102, 130, 102, 130,
    102, 130, 0, 0, 108, 133, 108, 133,
    108, 133, 108, 133, 108, 133, 108, 133,
    111, 138, 0, 0, 111, 138, 144, 0,
    144, 0, 111, 139, 111, 139, 111, 139,
    111, 139, 111, 139, 111, 139, 111, 139,
    111, 139, 111, 139, 111, 139, 112, 113,
    112, 113, 112, 113, 112, 113, 112, 113,
    112, 113, 112, 113, 112, 113, 112, 113,
    112, 113, 114, 85, 108, 133, 108, 133,
    108, 133, 108, 133, 108, 133, 108, 133,
    0, 0, 144, 117, 114, 0, 0, 0,
    144, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 115, 85, 144, 118, 0, 0,
    0, 0, 122, 0, 122, 0, 0, 0,
    0, 0, 0, 0, 115, 0, 0, 0,
    143, 0, 143, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 114, 86, 0, 0,
    114, 85, 0, 0, 0, 0, 0, 0,
    123, 0, 123, 0, 0, 0, 114, 85,
    0, 0, 0, 0, 0, 0, 114, 85,
    114, 115, 0, 0, 115, 86, 122, 117,
    115, 85, 0, 0, 122, 0, 0, 0,
    114, 85, 0, 0, 114, 85, 115, 85,
    122, 145, 143, 0, 0, 0, 115, 85,
    115, 141, 114, 85, 0, 0, 143, 145,
    122, 145, 0, 0, 0, 0, 0, 0,
    115, 85, 123, 0, 115, 85, 143, 145,
    0, 0, 122, 145, 0, 0, 123, 123,
    0, 0, 115, 85, 124, 83, 0, 0,
    143, 145, 0, 0, 0, 0, 123, 123,
    114, 140, 0, 0, 124, 0, 124, 0,
    114, 87, 0, 0, 0, 0, 0, 0,
    0, 0, 114, 85, 0, 0, 0, 0,
    123, 146, 146, 0, 146, 0, 0, 0,
    115, 85, 150, 0, 150, 0, 0, 0,
    115, 87, 122, 145, 0, 0, 0, 0,
    0, 0, 115, 85, 0, 0, 124, 83,
    143, 145, 124, 88, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 124, 0,
    124, 147, 0, 0, 124, 148, 124, 88,
    124, 88, 124, 149, 0, 0, 0, 0,
    123, 146, 0, 0, 146, 0, 146, 147,
    0, 0, 124, 149, 150, 0, 124, 88,
    146, 153, 0, 0, 0, 0, 0, 0,
    150, 151, 0, 0, 124, 88, 0, 0,
    146, 153, 0, 0, 0, 0, 0, 0,
    150, 151, 131, 133, 131, 133, 131, 133,
    131, 133, 131, 133, 131, 133, 131, 133,
    131, 133, 131, 133, 131, 133, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 124, 88, 131, 133, 131, 133,
    131, 133, 131, 133, 131, 133, 131, 133,
    0, 0, 0, 0, 124, 88, 133, 104,
    133, 104, 133, 104, 133, 104, 133, 104,
    133, 104, 133, 104, 133, 104, 133, 104,
    133, 104, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0,
    133, 104, 133, 104, 133, 104, 133, 104,
    133, 104, 133, 104, 131, 133, 131, 133,
    131, 133, 131, 133, 131, 133, 131, 133,
    134, 135, 0, 0, 134, 135, 0, 0,
    0, 0, 134, 152, 134, 152, 134, 152,
    134, 152, 134, 152, 134, 152, 134, 152,
    134, 152, 134, 152, 134, 152, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0,
    133, 104, 133, 104, 133, 104, 133, 104,
    133, 104, 133, 104, 135, 152, 135, 152,
    135, 152, 135, 152, 135, 152, 135, 152,
    135, 152, 135, 152, 135, 152, 135, 152,
    136, 137, 136, 137, 136, 137, 136, 137,
    136, 137, 136, 137, 136, 137, 136, 137,
    136, 137, 136, 137, 137, 137, 137, 137,
    137, 137, 137, 137, 137, 137, 137, 137,
    137, 137, 137, 137, 137, 137, 137, 137,
    138, 139, 138, 139, 138, 139, 138, 139,
    138, 139, 138, 139, 138, 139, 138, 139,
    138, 139, 138, 139, 140, 85, 0, 0,
    0, 0, 141, 85, 151, 0, 151, 0,
    142, 85, 0, 0, 0, 0, 140, 0,
    0, 0, 0, 0, 141, 0, 0, 0,
    0, 0, 142, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0,
    145, 0, 145, 0, 0, 0, 153, 0,
    153, 0, 0, 0, 0, 0, 140, 86,
    0, 0, 140, 85, 141, 86, 151, 0,
    141, 85, 142, 86, 0, 0, 142, 85,
    140, 85, 151, 151, 0, 0, 141, 85,
    140, 85, 140, 142, 142, 85, 141, 85,
    141, 85, 151, 151, 142, 85, 142, 85,
    0, 0, 140, 142, 145, 117, 140, 85,
    141, 85, 145, 0, 141, 85, 142, 85,
    153, 0, 142, 85, 140, 142, 145, 118,
    0, 0, 141, 85, 153, 153, 0, 0,
    142, 85, 0, 0, 0, 0, 145, 118,
    0, 0, 0, 0, 153, 153, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0,
    145, 118, 0, 0, 0, 0, 0, 0,
    0, 0, 140, 85, 0, 0, 0, 0,
    141, 85, 140, 87, 0, 0, 142, 85,
    141, 87, 0, 0, 140, 142, 142, 87,
    0, 0, 141, 85, 0, 0, 0, 0,
    142, 85, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0, 145, 118, 0, 0, 0, 0, 0, 0, 0, 0};
struct yysvf yysvec[] = {
  0, 0, 0,
  yycrank + -1, 0, 0,
  yycrank + -40, yysvec + 1, 0,
  yycrank + -111, 0, 0,
  yycrank + -150, yysvec + 3, 0,
  yycrank + 0, 0, yyvstop + 1,
  yycrank + 0, 0, yyvstop + 3,
  yycrank + 0, 0, yyvstop + 6,
  yycrank + 13, 0, yyvstop + 8,
  yycrank + 0, 0, yyvstop + 11,
  yycrank + -214, 0, yyvstop + 14,
  yycrank + 0, 0, yyvstop + 16,
  yycrank + 267, 0, yyvstop + 19,
  yycrank + 0, 0, yyvstop + 22,
  yycrank + 0, 0, yyvstop + 25,
  yycrank + -389, 0, yyvstop + 28,
  yycrank + 0, 0, yyvstop + 30,
  yycrank + 0, 0, yyvstop + 33,
  yycrank + 0, 0, yyvstop + 36,
  yycrank + 232, 0, yyvstop + 39,
  yycrank + 242, yysvec + 12, yyvstop + 41,
  yycrank + 4, yysvec + 12, yyvstop + 44,
  yycrank + 5, yysvec + 19, yyvstop + 47,
  yycrank + 0, 0, yyvstop + 50,
  yycrank + 0, 0, yyvstop + 53,
  yycrank + 0, 0, yyvstop + 56,
  yycrank + 0, 0, yyvstop + 59,
  yycrank + 0, 0, yyvstop + 62,
  yycrank + 0, 0, yyvstop + 65,
  yycrank + 0, 0, yyvstop + 68,
  yycrank + 0, 0, yyvstop + 71,
  yycrank + 2, 0, yyvstop + 74,
  yycrank + 0, 0, yyvstop + 76,
  yycrank + 0, 0, yyvstop + 79,
  yycrank + 0, 0, yyvstop + 82,
  yycrank + 0, 0, yyvstop + 85,
  yycrank + 0, 0, yyvstop + 88,
  yycrank + -392, 0, yyvstop + 91,
  yycrank + 0, 0, yyvstop + 94,
  yycrank + 0, 0, yyvstop + 98,
  yycrank + -75, yysvec + 37, yyvstop + 101,
  yycrank + -8, yysvec + 37, yyvstop + 105,
  yycrank + -395, yysvec + 10, yyvstop + 109,
  yycrank + -10, yysvec + 37, yyvstop + 112,
  yycrank + -449, yysvec + 37, yyvstop + 116,
  yycrank + -12, yysvec + 37, yyvstop + 120,
  yycrank + -14, yysvec + 37, yyvstop + 124,
  yycrank + -465, yysvec + 15, yyvstop + 128,
  yycrank + -16, yysvec + 37, yyvstop + 131,
  yycrank + -81, yysvec + 37, yyvstop + 135,
  yycrank + -508, 0, yyvstop + 139,
  yycrank + -184, yysvec + 37, yyvstop + 143,
  yycrank + -523, yysvec + 37, yyvstop + 146,
  yycrank + -567, yysvec + 37, yyvstop + 150,
  yycrank + -538, yysvec + 37, yyvstop + 154,
  yycrank + -87, yysvec + 37, yyvstop + 158,
  yycrank + -99, yysvec + 37, yyvstop + 162,
  yycrank + -126, yysvec + 37, yyvstop + 166,
  yycrank + -191, yysvec + 37, yyvstop + 170,
  yycrank + -197, yysvec + 37, yyvstop + 174,
  yycrank + -207, yysvec + 37, yyvstop + 178,
  yycrank + -209, yysvec + 37, yyvstop + 182,
  yycrank + -211, yysvec + 37, yyvstop + 186,
  yycrank + -218, yysvec + 37, yyvstop + 190,
  yycrank + -258, yysvec + 37, yyvstop + 193,
  yycrank + -317, yysvec + 37, yyvstop + 197,
  yycrank + -319, yysvec + 37, yyvstop + 201,
  yycrank + -321, yysvec + 37, yyvstop + 205,
  yycrank + -382, yysvec + 37, yyvstop + 209,
  yycrank + 0, yysvec + 8, yyvstop + 213,
  yycrank + -18, yysvec + 10, 0,
  yycrank + 0, 0, yyvstop + 215,
  yycrank + 594, 0, 0,
  yycrank + 0, yysvec + 12, yyvstop + 217,
  yycrank + 14, 0, 0,
  yycrank + -679, 0, 0,
  yycrank + 358, 0, 0,
  yycrank + 59, yysvec + 19, yyvstop + 219,
  yycrank + 618, yysvec + 12, yyvstop + 221,
  yycrank + 0, 0, yyvstop + 224,
  yycrank + 697, 0, yyvstop + 226,
  yycrank + 729, 0, 0,
  yycrank + 0, 0, yyvstop + 228,
  yycrank + -385, yysvec + 37, yyvstop + 230,
  yycrank + -446, yysvec + 37, yyvstop + 232,
  yycrank + -754, yysvec + 10, yyvstop + 235,
  yycrank + -407, yysvec + 37, yyvstop + 237,
  yycrank + -819, 0, yyvstop + 240,
  yycrank + -901, yysvec + 37, yyvstop + 242,
  yycrank + -461, yysvec + 37, yyvstop + 245,
  yycrank + -952, yysvec + 75, yyvstop + 247,
  yycrank + 0, 0, yyvstop + 249,
  yycrank + 0, 0, yyvstop + 251,
  yycrank + 0, 0, yyvstop + 253,
  yycrank + 0, 0, yyvstop + 255,
  yycrank + -519, yysvec + 37, yyvstop + 257,
  yycrank + -747, yysvec + 37, yyvstop + 259,
  yycrank + -1010, 0, yyvstop + 262,
  yycrank + -642, yysvec + 37, yyvstop + 266,
  yycrank + -651, yysvec + 37, yyvstop + 269,
  yycrank + -434, yysvec + 10, 0,
  yycrank + -19, yysvec + 10, 0,
  yycrank + 1034, 0, 0,
  yycrank + 0, 0, yyvstop + 271,
  yycrank + 0, yysvec + 74, 0,
  yycrank + 0, yysvec + 74, yyvstop + 273,
  yycrank + 921, 0, 0,
  yycrank + 782, 0, 0,
  yycrank + 1073, 0, 0,
  yycrank + 14, yysvec + 76, yyvstop + 275,
  yycrank + 973, yysvec + 12, yyvstop + 277,
  yycrank + 1101, 0, 0,
  yycrank + 1111, 0, 0,
  yycrank + 0, yysvec + 112, yyvstop + 279,
  yycrank + -1168, yysvec + 10, yyvstop + 281,
  yycrank + -1184, yysvec + 10, yyvstop + 283,
  yycrank + -752, yysvec + 37, yyvstop + 285,
  yycrank + -410, yysvec + 37, yyvstop + 287,
  yycrank + -501, yysvec + 37, yyvstop + 290,
  yycrank + -510, yysvec + 37, yyvstop + 292,
  yycrank + -945, yysvec + 37, yyvstop + 295,
  yycrank + -903, yysvec + 37, yyvstop + 297,
  yycrank + -1180, yysvec + 37, yyvstop + 299,
  yycrank + -1199, yysvec + 37, yyvstop + 301,
  yycrank + -1249, 0, yyvstop + 304,
  yycrank + -1022, yysvec + 37, yyvstop + 307,
  yycrank + -653, yysvec + 37, yyvstop + 309,
  yycrank + -1003, yysvec + 37, yyvstop + 311,
  yycrank + -748, yysvec + 10, 0,
  yycrank + -21, yysvec + 10, 0,
  yycrank + -22, yysvec + 10, 0,
  yycrank + 1273, 0, 0,
  yycrank + 1065, yysvec + 107, 0,
  yycrank + 1299, yysvec + 108, 0,
  yycrank + 1333, 0, 0,
  yycrank + 1354, 0, 0,
  yycrank + 1364, yysvec + 12, yyvstop + 314,
  yycrank + 1374, yysvec + 12, yyvstop + 316,
  yycrank + 1384, 0, 0,
  yycrank + 0, yysvec + 138, yyvstop + 319,
  yycrank + -1441, yysvec + 10, yyvstop + 321,
  yycrank + -1444, yysvec + 10, yyvstop + 323,
  yycrank + -1447, yysvec + 10, yyvstop + 325,
  yycrank + -1187, yysvec + 37, yyvstop + 327,
  yycrank + -1138, yysvec + 37, yyvstop + 329,
  yycrank + -1459, yysvec + 37, yyvstop + 331,
  yycrank + -1260, yysvec + 37, yyvstop + 333,
  yycrank + -1025, yysvec + 37, yyvstop + 335,
  yycrank + -592, yysvec + 124, yyvstop + 337,
  yycrank + -696, yysvec + 124, yyvstop + 340,
  yycrank + -1264, yysvec + 37, yyvstop + 344,
  yycrank + -1437, yysvec + 37, yyvstop + 346,
  yycrank + 0, yysvec + 135, yyvstop + 349,
  yycrank + -1462, yysvec + 37, yyvstop + 351,
  0, 0, 0
};
struct yywork *yytop = yycrank + 1556;
struct yysvf *yybgin = yysvec + 1;
char yymatch[] = {
  00, 01, 01, 01, 01, 01, 01, 01,
  01, 011, 012, 01, 01, 01, 01, 01,
  01, 01, 01, 01, 01, 01, 01, 01,
  01, 01, 01, 01, 01, 01, 01, 01,
  01, 01, '"', 01, '$', 01, 01, 01,
  01, 01, 011, '+', 01, '+', 01, '/',
  '0', '0', '0', '0', '0', '0', '0', '0',
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
