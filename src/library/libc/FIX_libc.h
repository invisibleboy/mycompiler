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
 *
 * These #defines are used to prevent multiple definitions when using
 * libc/c_list.l.src and list/list.l.src
 *
\*****************************************************************************/

/* 10/29/02 REK Adding config.h */
#include <config.h>

#define LCODE

#ifdef HCODE

#define yysvec          Hcode_yysvec
#define yyback          Hcode_yyback
#define yylook          Hcode_yylook
#define yybgin          Hcode_yybgin
#define yysptr          Hcode_yysptr
#define yyprevious      Hcode_yyprevious
#define yyextra         Hcode_yyextra
#define yymatch         Hcode_yymatch
#define yycrank         Hcode_yycrank
#define yyvstop         Hcode_yyvstop
#define yyunput         Hcode_yyunput
#define yyinput         Hcode_yyinput
#define yyin            Hcode_yyin
#define yylineno        Hcode_yylineno
#define yyoutput        Hcode_yyoutput
#define yytop           Hcode_yytop
#define yyout           Hcode_yyout
#define yylex           Hcode_yylex
#define yywrap          Hcode_yywrap
#define yyleng          Hcode_yyleng
#define yymorfg         Hcode_yymorfg
#define yytchar         Hcode_yytchar
#define yyestate        Hcode_yyestate
#define yylsp           Hcode_yylsp
#define yyolsp          Hcode_yyolsp
#define yyfnd           Hcode_yyfnd

#else

#define yysvec          Lcode_yysvec
#define yyback          Lcode_yyback
#define yylook          Lcode_yylook
#define yybgin          Lcode_yybgin
#define yysptr          Lcode_yysptr
#define yyprevious      Lcode_yyprevious
#define yyextra         Lcode_yyextra
#define yymatch         Lcode_yymatch
#define yycrank         Lcode_yycrank
#define yyvstop         Lcode_yyvstop
#define yyunput         Lcode_yyunput
#define yyinput         Lcode_yyinput
#define yyin            Lcode_yyin
#define yylineno        Lcode_yylineno
#define yyoutput        Lcode_yyoutput
#define yytop           Lcode_yytop
#define yyout           Lcode_yyout
#define yylex           Lcode_yylex
#define yywrap          Lcode_yywrap
#define yyleng          Lcode_yyleng
#define yymorfg         Lcode_yymorfg
#define yytchar         Lcode_yytchar
#define yyestate        Lcode_yyestate
#define yylsp           Lcode_yylsp
#define yyolsp          Lcode_yyolsp
#define yyfnd           Lcode_yyfnd

#endif
