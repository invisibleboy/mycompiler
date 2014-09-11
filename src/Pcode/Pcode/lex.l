%{

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
 *	File:	lex.l
 *	Author:	Pohua Chang
 * 	Copyright (c) 1991 Pohua Chang. All rights reserved.  
 * 	All rights granted to the University of Illinois.
 *	The University of Illinois software License Agreement
 * 	specifies the terms and conditions for redistribution.
\*****************************************************************************/

#ifndef lint
#define lint
static char copyright[] =
"@(#) Copyright (c) 1991 Pohua Chang.\n All rights reserved.\n";
#endif

/*---------------------------------------------------------------------------
 *--------------------------------------------------------------------------*/
#include <config.h>
#include <stdio.h>
#include <stdlib.h>
#include <ctype.h>
#include <malloc.h>
#include <string.h>
#include <Pcode/pcode.h>
#include <Pcode/lex.h>
#include <Pcode/parse_prefix.h>
#include "parse.h"

#undef YYLMAX
/* BCC - extended YYLMAX from 2048 to 2500 for EDG/132.ijpeg - 6/27/96*/
#define YYLMAX 2500 

int last_line = 1;
int last_column = 0;

int lexIntLit (void);
int lexLongLit (void);
int lexRealLit (void);
int lexStringLit (void);
int lexCharLit (void);
int lexIdentifier (void);
void lexError (char *);
int yywrap (void);


int	TokenType;		/* token type */
char	*TokenName;		/* token string */
int	TokenInteger;		/* scalar value */
double	TokenReal;		/* floating point value */
int 	lex_error = 0;		/* number of lexical errors encountered */

#define ToState(n) BEGIN(n)	/* change lex state */
#define DefaultState 0		/* default lex state */

%}

Integer		[+\-]?[0-9]+
/* BCC - make '-' as a legal char for id names - 2/3/96 */
Identifier     	("."|[/$_a-zA-Z])([/:$_a-zA-Z0-9]|"."|"-")*   
String          "\""([^\\\n"]|\\\$|\\\!|\\\#|\\\%|\\a|\\b|\\f|\\n|\\r|\\t|\\v|\\\\|\\\"|\\'|\\\?|\\0?[0-7][0-7]?[0-7]?|\\0?[Xx]([0-9]|[a-f]|[A-F])([0-9]|[a-f]|[A-F])?|\\\n)*"\""
Character    	("'"."'"|"'"("\\"(.|(0?[0-7]?[0-7]?[0-7]?)|(0?[xX]([0-9]|[a-f]|[A-F])([0-9]|[a-f]|[A-F])?)))"'")  
Float1		[+\-]?[0-9]+"."[0-9]*(("e"|"E")("+"|"-")?[0-9]+)?
Float2	 	[+\-]?[0-9]+(("e"|"E")("+"|"-")?[0-9]+)
Float3		[+\-]?"."[0-9]+(("e"|"E")("+"|"-")?[0-9]+)?
Float4          [+\-]?inf
%Start COMMENT LINENO
%%

<COMMENT>[^\t\n*]+      { last_column += yyleng; }
<COMMENT>"\t"           { last_column += yyleng; }
<COMMENT>"\n"           { last_line++; last_column = 0; }
<COMMENT>"*/"           { last_column += yyleng; ToState (DefaultState); }
<COMMENT>"*\t"          { last_column += yyleng; }
<COMMENT>"*\n"          { last_line++; last_column = 0; }
<COMMENT>"*"[^\t\n/*]? 	{ last_column += yyleng; }
<LINENO>{Integer}       { last_line = atoi (yytext); last_column = 0;
                          ToState (DefaultState); }
"#line"                 { ToState (LINENO); }

"/*"		        { last_column += yyleng; ToState (COMMENT); }
" "+			{ last_column += yyleng; }
"\t"    	    	{ last_column += yyleng; }
"\n"        		{ last_line++; last_column = 0; }
"\\\n"			{ last_line++; last_column = 0; }

{Integer}	  	{ last_column += yyleng; return (lexLongLit ()); }
{Identifier}      	{ last_column += yyleng; 
                          /* Special case: "inf" is a floating point
			   * literal, not an identifier. */
                          if (strcmp (yytext, "inf") == 0)
			    return (lexRealLit ());
			  else
			    return (lexIdentifier ());
                        }
{Float1}	        { last_column += yyleng; return (lexRealLit ()); }
{Float2}	        { last_column += yyleng; return (lexRealLit ()); }
{Float3}		{ last_column += yyleng; return (lexRealLit ()); }
{Float4}                { last_column += yyleng; return (lexRealLit ()); }
{String}	  	{ last_column += yyleng; return (lexStringLit ()); }
{Character}		{ last_column += yyleng; return (lexCharLit ()); }

"!"			{ last_column += yyleng; return (yytext[0]); }
"@"			{ last_column += yyleng; return (yytext[0]); }
"#"			{ last_column += yyleng; return (yytext[0]); }
"%"			{ last_column += yyleng; return (yytext[0]); }
"^"			{ last_column += yyleng; return (yytext[0]); }
"&"			{ last_column += yyleng; return (yytext[0]); }
"*"			{ last_column += yyleng; return (yytext[0]); }
"("			{ last_column += yyleng; return (yytext[0]); }
")"			{ last_column += yyleng; return (yytext[0]); }
"["			{ last_column += yyleng; return (yytext[0]); }
"]"			{ last_column += yyleng; return (yytext[0]); }
"{"			{ last_column += yyleng; return (yytext[0]); }
"}"			{ last_column += yyleng; return (yytext[0]); }
"?"			{ last_column += yyleng; return (yytext[0]); }
":"			{ last_column += yyleng; return (yytext[0]); }
"<"			{ last_column += yyleng; return (yytext[0]); }
">"			{ last_column += yyleng; return (yytext[0]); }
"="			{ last_column += yyleng; return (yytext[0]); }
"~"			{ last_column += yyleng; return (yytext[0]); }
";"			{ last_column += yyleng; return (yytext[0]); }

.         		{ lexError (yytext); 
			  return (ERROR); }

%%

/*----------------------------------------------------------------*/
#define MAX_LEVEL	20
#if 0
static int stack_top = 0;
static FILE *stack[MAX_LEVEL];
static char *fname[MAX_LEVEL];
#endif
static char *current_file = "??";

/* Lexical Error */
void
lexError (char *message)
{
  lex_error++;
  fprintf (stderr, "lexical error : %s (in %s)\n", message, current_file);
  exit (-1);
}

#if 0
static void
Push (FILE *fpt, char *name)
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
    return (stdin);	/* default */
  stack_top -= 1;
  return (stack[stack_top]);
}

static char *
LastFile (void)
{
  if (stack_top <= 0)
    return ("??");	/* default */
  return (fname[stack_top-1]);
}
#endif

/* 1 for no wrap.  We won't set up a second source file to lex after the
 * current one. */
int
yywrap (void)
{
  return (1);
}	

#if 0
/* BCC - Test if the file exists - 7/14/97 */
int
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
      for (i = len-1; i >= 0; i--)
	if (last_file[i] == '/')
	  break;
      for (j = 0; j <= i; j++)
	temp1[j] = last_file[j];
      for (i = 0; i <= strlen (file_name); i++)
	temp1[i+j] = file_name[i];
      file_name = FindString (temp1);
    }

  if (!strcmp (file_name, "stdin"))
    {
      return (1);
    }
  else
    {                        /* open new file */
      fpt = fopen (file_name, "r");
      if (fpt==NULL)
	{
	  return (0);
	}
      return (1);
    }
}
#endif

int
lexSetInput (FILE *in)
{
  if (in != yyin)
    yyrestart (in);

  return (1);
}

/* Assumes the given file handle has just been adjusted with seek.  Calls
 * yyrestart if the file is the same as yyin. */
int
lexNotifySeek (FILE *in)
{
  if (in == yyin)
    yyrestart (in);

  return (1);
}

int
lexFinish (void)
{
  YY_FLUSH_BUFFER;

#if 0
  yy_delete_buffer (YY_CURRENT_BUFFER);
#endif

  return (1);
}

/* Set Up The Input File */
int
lexOpen (char *file_name)
{
  FILE *fpt;

#if 0
  Push (yyin, current_file);	/** save the previous fpt **/
  last_file = current_file;

  /*
   *	If the current_file is specified from the
   *	root directory, do nothing.
   *	Otherwise, add the prefix of the last file
   *	to the front of the file_name.
   */
  if (inclusion && (file_name[0] != '/'))
    {
      int len, j, i;
      char temp1[1024];

      len = strlen (last_file);
      /* find the prefix: /x/y/z -> /x/y */
      for (i = len-1; i >= 0; i--)
	if (last_file[i] == '/')
	  break;
      for (j = 0; j <= i; j++)
	temp1[j] = last_file[j];
      for (i = 0; i <= strlen (file_name); i++)
	temp1[i+j] = file_name[i];
      file_name = FindString (temp1);
    }
  current_file = file_name;	/** we have a new file **/
#endif

  if (!strcmp (file_name, "stdin"))
    {
      yyin = stdin;			
      return (1);
    }
  else
    {			/* open new file */
      fpt = fopen (file_name, "r");
      if (fpt == NULL)
	{
	  fprintf (stderr, "> %s\n", file_name);
	  lexError ("can not open file");
	  return (0);
	}
      yyin = fpt;
      return (1);
    }
}

/* Close an Input File */
int
lexClose (char *file_name)
{
  if (strcmp (file_name, "stdin"))
    fclose (yyin);

#if 0
  current_file = LastFile ();
  yyin = Pop ();
#endif
  return (1);
}

FILE *
SwitchFile (FILE *fpt)
{
  FILE *old_yyin;
  old_yyin = yyin;
  if (fpt != NULL)
    {
      yyin = fpt;
      return (old_yyin);
    }
  lexError ("cannot switch yyin to NULL");
  return (0);
}

/*----------------------------------------------------------------*/
/* Process an integer Literal */
#ifdef __linux__
/* ADA: 7/9/96: Linux libc atoi() will return MAX_LONG if input string is too
        large.  This is disastrous when reading a huge number still in unsigned
        long range.  Use the following routine instead.  Different name is used
        to avoid conflict with linux header files */
int
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
    neg = 0;

  for (ret = 0; *p; p++) 
    if (isdigit (*p))
      {
	ret *= 10;
	ret += *p - '0';
      }
    else
      break;
  
  return ((neg) ? -ret : ret);  
}
#endif

int
lexIntLit ()
{
#ifdef __linux__
  yylval.i_val = atoi_linux (yytext);  /* See above note */
#else
  yylval.i_val = atoi (yytext);
#endif
  return(I_LIT);
}

int
lexLongLit ()
{
  yylval.i_val = atol (yytext);
  return (I_LIT);
}

/* Process a Real Literal */
int
lexRealLit (void)
{
  yylval.f_val = atof (yytext);
  return (F_LIT);
}

/* Process a String Literal */
int
lexStringLit ()
{
  yylval.st_val = strdup (yytext);
  return (ST_LIT);
}

/* Process a Character Literal */
int
lexCharLit ()
{
  yylval.c_val = yytext[0];
  return (C_LIT);
}

/* Process an Identifier/Typeid/Keyword */
int
lexIdentifier ()
{
  /* Rather than a huge list of strcmps, use switch statements to narrow
   * identifier down to a couple choices.  This block switches on each
   * character of a keyword until we have three or fewer choices. */
#if 0
  yylval.st_val = strdup (yytext);
#endif

  switch (yytext[0])
    {
    case 'A':
      switch (yytext[1])
	{
	case 'D':
	  if (strcmp (yytext, "ADD") == 0)
	    return (ADD);
	  else if (strcmp (yytext, "ADDR") == 0)
	    return (ADDR);
	  else if (strcmp (yytext, "ADVANCE") == 0)
	    return (ADVANCE);
	  break;
	case 'L':
	  if (strcmp (yytext, "ALIGNMENT") == 0)
	    return (ALIGNMENT);
	  break;
	case 'N':
	  if (strcmp (yytext, "AND") == 0)
	    return (AND);
	  break;
	case 'R':
	  if (strcmp (yytext, "ARRAY") == 0)
	    return (ARRAY);
	  else if (strcmp (yytext, "ARROW") == 0)
	    return (ARROW);
	  break;
	case 'S':
	  if (strcmp (yytext, "ASM") == 0)
	    return (ASM);
	  else if (strcmp (yytext, "ASMOPRD") == 0)
	    return (ASMOPRD);
	  else if (strcmp (yytext, "ASSIGN") == 0)
	    return (ASSIGN);
	  break;
	case 'U':
	  if (strcmp (yytext, "AUTO") == 0)
	    return (AUTO);
	  break;
	case 'W':
	  if (strcmp (yytext, "AWAIT") == 0)
	    return (AWAIT);
	  break;
	case '_':
	  switch (yytext[2])
	    {
	    case 'A':
	      if (strcmp (yytext, "A_ADD") == 0)
		return (A_ADD);
	      else if (strcmp (yytext, "A_AND") == 0)
		return (A_AND);
	      break;
	    case 'D':
	      if (strcmp (yytext, "A_DIV") == 0)
		return (A_DIV);
	      break;
	    case 'L':
	      if (strcmp (yytext, "A_LSHFT") == 0)
		return (A_LSHFT);
	      break;
	    case 'M':
	      if (strcmp (yytext, "A_MOD") == 0)
		return (A_MOD);
	      else if (strcmp (yytext, "A_MUL") == 0)
		return (A_MUL);
	      break;
	    case 'O':
	      if (strcmp (yytext, "A_OR") == 0)
		return (A_OR);
	      break;
	    case 'R':
	      if (strcmp (yytext, "A_RSHFT") == 0)
		return (A_RSHFT);
	      break;
	    case 'S':
	      if (strcmp (yytext, "A_SUB") == 0)
		return (A_SUB);
	      break;
	    case 'X':
	      if (strcmp (yytext, "A_XOR") == 0)
		return (A_XOR);
	      break;
	    }
	  break;
	}
      break;
    case 'B':
      switch (yytext[1])
	{
	case 'I':
	  if (strcmp (yytext, "BITFIELD") == 0)
	    return (BITFIELD);
	  break;
	case 'L':
	  if (strcmp (yytext, "BLOCK") == 0)
	    return (BLOCK);
	  break;
	case 'R':
	  if (strcmp (yytext, "BREAK") == 0)
	    return (BREAK);
	  break;
	}
      break;
    case 'C':
      switch (yytext[1])
	{
	case 'A':
	  if (strcmp (yytext, "CALL") == 0)
	    return (CALL);
	  else if (strcmp (yytext, "CASE") == 0)
	    return (CASE);
	  else if (strcmp (yytext, "CAST") == 0)
	    return (CAST);
	  break;
	case 'D':
	  if (strcmp (yytext, "CDECL") == 0)
	    return (CDECL);
	  break;
	case 'H':
	  if (strcmp (yytext, "CHAR") == 0)
	    return (CHAR);
	  break;
	case 'L':
	  if (strcmp (yytext, "CLOBBERS") == 0)
	    return (CLOBBERS);
	  break;
	case 'O':
	  switch (yytext[2])
	    {
	    case 'B':
	      if (strcmp (yytext, "COBEGIN") == 0)
		return (COBEGIN);
	      break;
	    case 'M':
	      switch (yytext[3])
		{
		case 'D':
		  if (strcmp (yytext, "COMDAT") == 0)
		    return (COMDAT);
		  break;
		case 'M':
		  if (strcmp (yytext, "COMMON") == 0)
		    return (COMMON);
		  break;
		case 'P':
		  if (strcmp (yytext, "COMPEXPR") == 0)
		    return (COMPEXPR);
		  else if (strcmp (yytext, "COMPSTMT") == 0)
		    return (COMPSTMT);
		  break;
		}
	      break;
	    case 'N':
	      switch (yytext[3])
		{
		case 'J':
		  if (strcmp (yytext, "CONJ") == 0)
		    return (CONJ);
		  break;
		case 'S':
		  if (strcmp (yytext, "CONST") == 0)
		    return (CONST);
		  else if (strcmp (yytext, "CONSTRUCTOR") == 0)
		    return (CONSTRUCTOR);
		  break;
		case 'T':
		  if (strcmp (yytext, "CONTINUE") == 0)
		    return (CONTINUE);
		  break;
		}
	      break;
	    }
	  break;
	}
      break;
    case 'D':
      switch (yytext[1])
	{
	case 'E':
	  switch (yytext[2])
	    {
	    case 'F':
	      if (strcmp (yytext, "DEF") == 0)
		return (DEF);
	      else if (strcmp (yytext, "DEFAULT") == 0)
		return (DEFAULT);
	      else if (strcmp (yytext, "DEFINED") == 0)
		return (DEFINED);
	      break;
	    case 'S':
	      if (strcmp (yytext, "DESTRUCTOR") == 0)
		return (DESTRUCTOR);
	      break;
	    }
	  break;
	case 'I':
	  if (strcmp (yytext, "DIM") == 0)
	    return (DIM);
	  else if (strcmp (yytext, "DISJ") == 0)
	    return (DISJ);
	  else if (strcmp (yytext, "DIV") == 0)
	    return (DIV);
	  break;
	case 'O':
	  if (strcmp (yytext, "DO") == 0)
	    return (DO);
	  else
	    switch (yytext[2])
	      {
	      case 'A':
		if (strcmp (yytext, "DOACROSS") == 0)
		  return (DOACROSS);
		else if (strcmp (yytext, "DOALL") == 0)
		  return (DOALL);
		break;
	      case 'S':
		if (strcmp (yytext, "DOSERIAL") == 0)
		  return (DOSERIAL);
		else if (strcmp (yytext, "DOSUPER") == 0)
		  return (DOSUPER);
		break;
	      case 'T':
		if (strcmp (yytext, "DOT") == 0)
		  return (DOT);
		break;
	      case 'U':
		if (strcmp (yytext, "DOUBLE") == 0)
		  return (DOUBLE);
		break;
	      }
	  break;
	}
      break;
    case 'E':
      switch (yytext[1])
	{
	case 'L':
	  if (strcmp (yytext, "ELSE") == 0)
	    return (ELSE);
	  else if (strcmp (yytext, "ELLIPSIS") == 0)
	    return (ELLIPSIS);
	  break;
	case 'M':
	  if (strcmp (yytext, "EMPTY") == 0)
	    return (EMPTY);
	  break;
	case 'N':
	  if (strcmp (yytext, "END") == 0)
	    return (END);
	  if (strcmp (yytext, "ENUM") == 0)
	    return (ENUM);
	  else if (strcmp (yytext, "ENUMFIELD") == 0)
	    return (ENUMFIELD);
	  break;
	case 'Q':
	  if (strcmp (yytext, "EQ") == 0)
	    return (EQ);
	  break;
	case 'R':
	  if (strcmp (yytext, "ERROR") == 0)
	    return (ERROR);
	  break;
	case 'X':
	  switch (yytext[2])
	    {
	    case 'P':
	      if (strcmp (yytext, "EXPLICIT_ALIGNMENT") == 0)
		return (EXPLICIT_ALIGNMENT);
	      else if (strcmp (yytext, "EXPR") == 0)
		return (EXPR);
	      else if (strcmp (yytext, "EXPRSIZE") == 0)
		return (EXPRSIZE);
	      break;
	    case 'T':
	      if (strcmp (yytext, "EXTERN") == 0)
		return (EXTERN);
	      break;
	    }
	  break;
	}
      break;
    case 'F':
      switch (yytext[1])
	{
	case 'A':
	  if (strcmp (yytext, "FASTCALL") == 0)
	    return (FASTCALL);
	  break;
	case 'I':
	  if (strcmp (yytext, "FIELD") == 0)
	    return (FIELD);
	  else if (strcmp (yytext, "FINAL") == 0)
	    return (FINAL);
	  break;
	case 'L':
	  if (strcmp (yytext, "FLOAT") == 0)
	    return (FLOAT);
	  break;
	case 'O':
	  if (strcmp (yytext, "FOR") == 0)
	    return (FOR);
	  break;
	case 'U':
	  if (strcmp (yytext, "FUNC") == 0)
	    return (FUNC);
	  else if (strcmp (yytext, "FUNCTION") == 0)
	    return (FUNCTION);
	  break;
	}
      break;
    case 'G':
      switch (yytext[1])
	{
	case 'E':
	  if (strcmp (yytext, "GE") == 0)
	    return (GE);
	  break;
	case 'L':
	  if (strcmp (yytext, "GLOBAL") == 0)
	    return (GLOBAL);
	  break;
	case 'O':
	  if (strcmp (yytext, "GOTO") == 0)
	    return (GOTO);
	  break;
	case 'T':
	  if (strcmp (yytext, "GT") == 0)
	    return (GT);
	  break;
	case 'V':
	  if (strcmp (yytext, "GVAR") == 0)
	    return (GVAR);
	  break;
	}
      break;
    case 'H':
      if (strcmp (yytext, "HEADER") == 0)
	return (HEADER);
      break;
    case 'I':
      switch (yytext[1])
	{
	case 'D':
	  if (strcmp (yytext, "ID") == 0)
	    return (ID);
	  break;
	case 'F':
	  if (strcmp (yytext, "IF") == 0)
	    return (IF);
	  break;
	case 'M':
	  if (strcmp (yytext, "IMPLICIT") == 0)
	    return (IMPLICIT);
	  break;
	case 'N':
	  if (strcmp (yytext, "IN") == 0)
	    return (IN);
	  else
	    {
	      switch (yytext[2])
		{
		case 'C':
		  if (strcmp (yytext, "INC") == 0)
		    return (INC);
		  else if (strcmp (yytext, "INCLUDE") == 0)
		    return (INCLUDE);
		  else if (strcmp (yytext, "INCOMPLETE") == 0)
		    return (INCOMPLETE);
		  break;
		case 'D':
		  if (strcmp (yytext, "INDEX") == 0)
		    return (INDEX);
		  else if (strcmp (yytext, "INDR") == 0)
		    return (INDR);
		  break;
		case 'I':
		  if (strcmp (yytext, "INIT") == 0)
		    return (INIT);
		  break;
		case 'T':
		  if (strcmp (yytext, "INT") == 0)
		    return (INT);
		  break;
		case 'V':
		  if (strcmp (yytext, "INV") == 0)
		    return (INV);
		  break;
		}
	    }
	  break;
	case 'P':
	  if (strcmp (yytext, "IP_TABLE") == 0)
	    return (IP_TABLE);
	  break;
	}
      break;
    case 'K':
      if (strcmp (yytext, "KEY") == 0)
	return (KEY);
      break;
    case 'L':
      switch (yytext[1])
	{
	case 'A':
	  if (strcmp (yytext, "LABEL") == 0)
	    return (LABEL);
	  break;
	case 'E':
	  if (strcmp (yytext, "LE") == 0)
	    return (LE);
	  break;
	case 'I':
	  if (strcmp (yytext, "LINKED") == 0)
	    return (LINKED);
	  else if (strcmp (yytext, "LINKMULTI") == 0)
	    return (LINKMULTI);
	  break;
	case 'O':
	  if (strcmp (yytext, "LONG") == 0)
	    return (LONG);
	  else if (strcmp (yytext, "LONGLONG") == 0)
	    return (LONGLONG);
	  else if (strcmp (yytext, "LONGDOUBLE") == 0)
	    return (LONGDOUBLE);
	  break;
	case 'S':
	  if (strcmp (yytext, "LSHFT") == 0)
	    return (LSHFT);
	  break;
	case 'T':
	  if (strcmp (yytext, "LT") == 0)
	    return (LT);
	  else if (strcmp (yytext, "LTYPE") == 0)
	    return (LTYPE);
	  break;
	}
      break;
    case 'M':
      switch (yytext[1])
	{
	case 'O':
	  if (strcmp (yytext, "MOD") == 0)
	    return (MOD);
	  break;
	case 'U':
	  if (strcmp (yytext, "MUL") == 0)
	    return (MUL);
	  else if (strcmp (yytext, "MUTEX") == 0)
	    return (MUTEX);
	  break;
	}
      break;
    case 'N':
      switch (yytext[1])
	{
	case 'E':
	  if (strcmp (yytext, "NE") == 0)
	    return (NE);
	  else if (strcmp (yytext, "NEG") == 0)
	    return (NEG);
	  break;
	case 'O':
	  if (strcmp (yytext, "NOT") == 0)
	    return (NOT);
	  else if (strcmp (yytext, "NOT_AVAIL") == 0)
	    return (NOT_AVAIL);
	  break;
	case 'U':
	  if (strcmp (yytext, "NULL") == 0)
	    return (KW_NULL);
	  else if (strcmp (yytext, "NUM_ENTRIES") == 0)
	    return (NUM_ENTRIES);
	  break;
	}
      break;
    case 'O':
      switch (yytext[1])
	{
	case 'F':
	  if (strcmp (yytext, "OFFSET") == 0)
	    return (OFFSET);
	  break;
	case 'L':
	  if (strcmp (yytext, "OLD_PARAM") == 0)
	    return (OLD_PARAM);
	  break;
	case 'P':
	  if (strcmp (yytext, "OPERANDS") == 0)
	    return (OPERANDS);
	  break;
	case 'R':
	  if (strcmp (yytext, "OR") == 0)
	    return (OR);
	  break;
	case 'U':
	  if (strcmp (yytext, "OUT") == 0)
	    return (OUT);
	  break;
	}
      break;
    case 'P':
      switch (yytext[1])
	{
	case 'A':
	  if (strcmp (yytext, "PARAM") == 0)
	    return (PARAM);
	  else if (strcmp (yytext, "PARAMETER") == 0)
	    return (PARAMETER);
	  else if (strcmp (yytext, "PARENT") == 0)
	    return (PARENT);
	  break;
	case 'H':
	  if (strcmp (yytext, "PHI") == 0)
            return (PHI);
          break;
	case 'O':
	  switch (yytext[2])
	    {
	    case 'I':
	      if (strcmp (yytext, "POINTER") == 0)
		return (POINTER);
	      break;
	    case 'S':
	      if (strcmp (yytext, "POS") == 0)
		return (POS);
	      else if (strcmp (yytext, "POSTDEC") == 0)
		return (POSTDEC);
	      else if (strcmp (yytext, "POSTINC") == 0)
		return (POSTINC);
	      break;
	    }
	  break;
	case 'R':
	  switch (yytext[2])
	    {
	    case 'A':
	      if (strcmp (yytext, "PRAGMA") == 0)
		return (PRAGMA);
	      break;
	    case 'E':
	      if (strcmp (yytext, "PREDEC") == 0)
		return (PREDEC);
	      else if (strcmp (yytext, "PREINC") == 0)
		return (PREINC);
	      break;
	    case 'O':
	      if (strcmp (yytext, "PROFILE") == 0)
		return (PROFILE);
	      break;
	    }
	  break;
	case 'S':
	  if (strcmp (yytext, "PSTMT") == 0)
	    return (PSTMT);
	  break;
	}
      break;
    case 'Q':
      if (strcmp (yytext, "QUEST") == 0)
	return (QUEST);
      break;
    case 'R':
      switch (yytext[1])
	{
	case 'E':
	  if (strcmp (yytext, "REFS") == 0)
	    return (REFS);
	  else if (strcmp (yytext, "REGISTER") == 0)
	    return (REGISTER);
	  else if (strcmp (yytext, "RETURN") == 0)
	    return (RETURN);
	  break;
	case 'S':
	  if (strcmp (yytext, "RSHFT") == 0)
	    return (RSHFT);
	  break;
	}
      break;
    case 'S':
      switch (yytext[1])
	{
	case 'C':
	  if (strcmp (yytext, "SCOPE") == 0)
	    return (SCOPE);
	  break;
	case 'I':
	  if (strcmp (yytext, "SIGNED") == 0)
	    return (SIGNED);
	  else if (strcmp (yytext, "SIZE") == 0)
	    return (SIZE);
	  break;
	case 'H':
	  if (strcmp (yytext, "SHADOW") == 0)
	    return (SHADOW);
	  else if (strcmp (yytext, "SHORT") == 0)
	    return (SHORT);
	  break;
	case 'O':
	  if (strcmp (yytext, "SOURCE") == 0)
	    return (SOURCE);
	  break;
	case 'T':
	  switch (yytext[2])
	    {
	    case 'A':
	      if (strcmp (yytext, "STATIC") == 0)
		return (STATIC);
	      break;
	    case 'D':
	      if (strcmp (yytext, "STDCALL") == 0)
		return (STDCALL);
	      break;
	    case 'M':
	      if (strcmp (yytext, "STMT") == 0)
		return (STMT);
	      else if (strcmp (yytext, "STMTEXPR") == 0)
		return (STMTEXPR);
	      break;
	    case 'R':
	      if (strcmp (yytext, "STRUCT") == 0)
		return (STRUCT);
	      else if (strcmp (yytext, "STRING") == 0)
		return (STRING);
	      break;
	    }
	  break;
	case 'U':
	  if (strcmp (yytext, "SUB") == 0)
	    return (SUB);
	  break;
	case 'W':
	  if (strcmp (yytext, "SWITCH") == 0)
	    return (SWITCH);
	  break;
	case 'Y':
	  if (strcmp (yytext, "SYM") == 0)
	    return (SYM);
	  else if (strcmp (yytext, "SYMBOL_TABLE") == 0)
	    return (SYMBOL_TABLE);
	  else if (strcmp (yytext, "SYNC") == 0)
	    return (SYNC);
	  break;
	}
      break;
    case 'T':
      switch (yytext[1])
	{
	case 'H':
	  if (strcmp (yytext, "THEN") == 0)
	    return (THEN);
	  break;
	case 'Y':
	  if (strcmp (yytext, "TYPE") == 0)
	    return (TYPE);
	  else if (strcmp (yytext, "TYPEDEF") == 0)
	    return (TYPEDEF);
	  else if (strcmp (yytext, "TYPESIZE") == 0)
	    return (TYPESIZE);
	  break;
	}
      break;
    case 'U':
      if (strcmp (yytext, "UNION") == 0)
	return (UNION);
      else if (strcmp (yytext, "UNSIGNED") == 0)
	return (UNSIGNED);
      else if (strcmp (yytext, "UNNAMED") == 0)
	return (UNNAMED);
      break;
    case 'V':
      switch (yytext[1])
	{
	case 'A':
	  if (strcmp (yytext, "VAR") == 0)
	    return (VAR);
	  else if (strcmp (yytext, "VARARG") == 0)
	    return (VARARG);
	  break;
	case 'O':
	  if (strcmp (yytext, "VOID") == 0)
	    return (VOID);
	  else if (strcmp (yytext, "VOLATILE") == 0)
	    return (VOLATILE);
	  break;
	}
      break;
    case 'W':
      if (strcmp (yytext, "WEAK") == 0)
	return (WEAK);
      else if (strcmp (yytext, "WHILE") == 0)
	return (WHILE);
      break;
    case 'X':
      if (strcmp (yytext, "XOR") == 0)
	return (XOR);
      break;
    }
  /* If we didn't match a keyword, complain. */
  P_warn ("lex.l:lexIdentifier:%d Unknown identifier %s", __LINE__, yytext);

  return (0);
}


/*--------------------------------------------------------------------------*/
/*      We will implement a binary tree string management.
 */
typedef struct String
{
  char            *name;          /* string */
  struct String   *left;
  struct String   *right;
} String;
 
static String *stringTree = 0;
 
static char *
lmalloc (int size)
{
  char *new_str = (char *)malloc (size);
  if (new_str==0)
    {
      fprintf (stderr, "LIST : malloc returns nil\n");
      exit (1);
    }
  return (new_str);
}

/* String Save Function. */
static char *
strsave (char *str)
{
  char *new_str = (char *)lmalloc (strlen (str) + 1);
  strcpy (new_str, str);
  return (new_str);
}
 
/*      Find a string.  */
static String *
FindS (String *T, char *str)
{
  int diff;
  
  if (T == 0)                         /* not found */
    return (0);
  diff = strcmp (str, T->name);
  if (diff == 0)
    return (T);                 /* find a match */
  if (diff > 0)
    return (FindS (T->left, str));    /* search left */
  else
    return (FindS (T->right, str));   /* search right */
}

/*      Add a string.   */
static String *
AddS (String *T, char *str)
{
  int diff;
  String *new_str;
  
  if (T == 0)
    {                       /* add new string */
      new_str = (String *)lmalloc (sizeof (String));
      new_str->name = strsave (str);
      new_str->left = new_str->right = 0;
      return (new_str);
    }
  diff = strcmp (str, T->name);
  if (diff == 0) return (T);                 /* already exist */
  if (diff > 0)
    {
      T->left = AddS (T->left, str);   /* add to left */
      return (T);
    }
  else
    {
      T->right = AddS (T->right, str); /* add to right */
      return (T);
    }
}
 
/*      This function maintains all define-once-only names.
 *      It maintains a table of a list of unique names used
 *      by the entire program. This will save some space.
 */
char *
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
#if 0
static LIST dead_list = 0;
static int dead_list_length = 0;

/*	allocate space for a new node.	*/
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
      new_node = (LIST)lmalloc (sizeof (NODE));
      NodeType (new_node) = T_NULL;
      SiblingOf (new_node) = 0;
      ChildOf (new_node) = 0;
      /* BCC - 8/8/97 */
      LastChildOf (new_node) = 0;
    }
  return (new_node);
}

/*	dispose space of a node and its siblings and descendants.	*/
LIST
DisposeNode (LIST node) 
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
  
  SiblingOf (node) = dead_list;	/* add node to the dead list */
  dead_list = node;		/* for recycling */
  dead_list_length++;

  return (0);
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

/*	add a child.	*/
static void
AddChildNode (LIST parent, LIST child)
{
  LIST ptr;
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
      for (ptr = ChildOf (parent); SiblingOf (ptr) != 0; )
	ptr = SiblingOf (ptr);
      /* append the new child to it */
      SiblingOf (ptr) = child;
#endif
    }
}

/*
 *	Get a node or a complete list from input.
 *	It is assumed that the lexer has been properly set up.
 */
LIST
GetNode (void)
{
  int node_type;
  LIST new_node, ptr;

  node_type = yylex ();		/* get token */

  if (node_type == 0)
    {		/* end of file */
      new_node = NewNode ();
      NodeType (new_node) = T_EOF;
      return (new_node);
    }

  switch (node_type)
    {
    case L_ID:		/* identifier */
      new_node = NewNode ();
      NodeType (new_node) = T_ID;
      StringOf (new_node) = FindString (TokenName);
      break;
    case L_INT:		/* integer */
      new_node = NewNode ();
      NodeType (new_node) = T_INT;
      IntegerOf (new_node) = TokenInteger;
      break;
    case L_REAL:		/* real constant literal */
      new_node = NewNode ();
      NodeType (new_node) = T_REAL;
      RealOf (new_node) = TokenReal;
      break;
    case L_STRING:		/* string literal */
      new_node = NewNode ();
      NodeType (new_node) = T_STRING;
      StringOf (new_node) = FindString (TokenName);
      break;
    case L_CHAR:		/* character literal */
      new_node = NewNode ();
      NodeType (new_node) = T_CHAR;
      StringOf (new_node) = FindString (TokenName);
      break;
    case L_ERROR:		/* erroneous input */
      new_node = NewNode ();
      NodeType (new_node) = T_NULL;
      break;
    default:		/* single character operator */
      if (node_type == '(')
	{
	  new_node = NewNode ();
	  NodeType (new_node) = T_LIST;
	  ChildOf (new_node) = 0;
	  while ((ptr = GetNode ()) != 0) 
	    AddChildNode (new_node, ptr);
	}
      else
	if (node_type == ')')
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
  return (new_node);
}

/*	print a node.
 *	this is useful for program debugging.
 */
static FILE *FF = stdout;
void
print_node (LIST node, int indent)
{
  int i;
  LIST ptr;
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
	  print_node (ptr, indent+2);
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
      fprintf (FF, "(int %d)", IntegerOf (node));
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
 *	print a node in a specified file.
 */
void
PrintNode (FILE *F, LIST node)
{
  FILE *save = FF;
  FF = F;
  print_node (node, 0);
  fprintf (F, "\n");
  FF = save;
}

LIST
GetInteger (LIST list, int *val)
{
  if (list == 0)
    return (0);
  if (NodeType (list) != T_INT)
    return (list);
  *val = IntegerOf (list);
  return (SiblingOf (list));
}

LIST
GetReal (LIST list, double *val)
{
  if (list == 0)
    return (0);
  if (NodeType (list) != T_REAL)
    return (list);
  *val = RealOf (list);
  return (SiblingOf (list));
}

LIST
GetString (LIST list, char **val)
{
  if (list == 0)
    return (0);
  if (NodeType (list) != T_STRING)
    return (list);
  *val = StringOf (list);
  return (SiblingOf (list));
}

LIST
GetId (LIST list, char **val)
{
  if (list == 0)
    return (0);
  if (NodeType (list) != T_ID)
    return (list);
  *val = StringOf (list);
  return (SiblingOf (list));
}

/* BCC - added for garbage collection - 8/22/96 */
static void
RemoveS (String *ptr)
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

#endif



