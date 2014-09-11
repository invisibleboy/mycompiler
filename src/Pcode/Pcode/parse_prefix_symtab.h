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
 * \brief Preprocessor defines to change the name of Bison's global symbols.
 *
 * Preprocessor defines to change the name of the global symbols used
 * by bison and flex.  These are needed to have multiple bison parsers
 * in the same program.
 */

#ifndef _PCODE_PARSE_PREFIX_H_
#define _PCODE_PARSE_PREFIX_H_

#include <config.h>

#define	yymaxdepth PST_yymaxdepth
#define	yyparse	   PST_yyparse
#define	yylex	   PST_yylex
#define	yyerror	   PST_yyerror
#define	yylval	   PST_yylval
#define	yychar	   PST_yychar
#define	yydebug	   PST_yydebug
#define	yypact	   PST_yypact
#define	yyr1	   PST_yyr1
#define	yyr2	   PST_yyr2
#define	yydef	   PST_yydef
#define	yychk	   PST_yychk
#define	yypgo      PST_yypgo
#define	yyact	   PST_yyact
#define	yyexca	   PST_yyexca
#define yyerrflag  PST_yyerrflag
#define yynerrs	   PST_yynerrs
#define	yyps	   PST_yyps
#define	yypv	   PST_yypv
#define	yys	   PST_yys
#define	yy_yys	   PST_yy_yys
#define	yystate	   PST_yystate
#define	yytmp	   PST_yytmp
#define	yyv	   PST_yyv
#define	yy_yyv	   PST_yy_yyv
#define	yyval	   PST_yyval
#define	yylloc	   PST_yylloc
#define yyreds	   PST_yyreds
#define yytoks	   PST_yytoks
#define yylhs	   PST_yylhs
#define yylen	   PST_yylen
#define yydefred   PST_yydefred
#define yydgoto	   PST_yydgoto
#define yysindex   PST_yysindex
#define yyrindex   PST_yyrindex
#define yygindex   PST_yygindex
#define yytable	   PST_yytable
#define yycheck	   PST_yycheck
#define yyname     PST_yyname
#define yyrule     PST_yyrule

#endif
