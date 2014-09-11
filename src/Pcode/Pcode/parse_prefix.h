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

#define	yymaxdepth P_yymaxdepth
#define	yyparse	   P_yyparse
#define	yylex	   P_yylex
#define	yyerror	   P_yyerror
#define	yylval	   P_yylval
#define	yychar	   P_yychar
#define	yydebug	   P_yydebug
#define	yypact	   P_yypact
#define	yyr1	   P_yyr1
#define	yyr2	   P_yyr2
#define	yydef	   P_yydef
#define	yychk	   P_yychk
#define	yypgo      P_yypgo
#define	yyact	   P_yyact
#define	yyexca	   P_yyexca
#define yyerrflag  P_yyerrflag
#define yynerrs	   P_yynerrs
#define	yyps	   P_yyps
#define	yypv	   P_yypv
#define	yys	   P_yys
#define	yy_yys	   P_yy_yys
#define	yystate	   P_yystate
#define	yytmp	   P_yytmp
#define	yyv	   P_yyv
#define	yy_yyv	   P_yy_yyv
#define	yyval	   P_yyval
#define	yylloc	   P_yylloc
#define yyreds	   P_yyreds
#define yytoks	   P_yytoks
#define yylhs	   P_yylhs
#define yylen	   P_yylen
#define yydefred   P_yydefred
#define yydgoto	   P_yydgoto
#define yysindex   P_yysindex
#define yyrindex   P_yyrindex
#define yygindex   P_yygindex
#define yytable	   P_yytable
#define yycheck	   P_yycheck
#define yyname     P_yyname
#define yyrule     P_yyrule

#endif
