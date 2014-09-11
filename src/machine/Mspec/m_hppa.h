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
 *  File:  m_hppa.h
 *
 *  Description: Defines symbols used for reference in Mspec and code
 * 		generator.
 *
 *  Creation Date : March, 1993
 *
 *  Author:  Richard E. Hank, Wen-mei Hwu
 *
 *  Revisions:
 *
 *
\*****************************************************************************/

/*****************************************************************************\
 * NOTICE OF CONVENTION                                                      *
 * ------------------------------------------------------------------------- *
 * Mspec links to Pcode, Hcode, and Lcode modules.  In order to allow this   *
 * to take place without requiring front-end modules to link to liblcode.a,  *
 * Mspec code is divided into two classes as follows:                        *
 *  - mi_*.c must not depend on linkage to liblcode.a                        *
 *  - ml_*.c may depend on linkage to liblcode.a                             *
\*****************************************************************************/

#ifndef M_HPPA_H
#define M_HPPA_H

/* 10/29/02 REK Adding config.h */
#include <config.h>
#include <machine/m_spec.h>

/* WARNING WARNING WARNING ! ! !
 * 
 * Any changes to this file may need to be updated into
 * m_impact.h
 */

/*
 * Declarations for processor models
 */
enum
{
  M_HP_PA_1_0 = 0,		/* HP PA-RISC 1.0 */
  M_HP_PA_1_1,			/* HP PA-RISC 1.1 */
  M_HP_PA_7100,			/* HP PA-RISC 7100 9000/755 */
  M_HP_PA_X,			/* HP PA-RISC Experimental */

  M_HP_PLAYDOH_V1,		/* HP Emulation of PlayDoh */
  M_HP_PLAYDOH_LCODE,
  M_HP_PLAYDOH_MCODE
};

enum
{
  HPPA_MAC_ZERO = 100,		/* zero (gr0)                                */
  HPPA_MAC_TEMPREG,		/* tempreg (gr1) used in prologue/epilogue   */
  HPPA_MAC_RETADDR,		/* return address register (gr2)             */
  HPPA_MAC_MILLI_RET_VALUE,	/* millicode return value register (gr29)    */
  HPPA_MAC_MILLI_RETADDR,	/* millicode return address register (gr31)  */
  HPPA_MAC_LEAF,		/* 1 if leaf function, 0 if not leaf function */
  HPPA_MAC_ALLOC,		/* total alloc space requirements            */
  HPPA_MAC_CALLEE_I,
  HPPA_MAC_CALLEE_F,
  HPPA_MAC_CONV_LOC,		/* 1 if need conversion space, 0 else        */
  HPPA_MAC_CONV_OFF,		/* stack location reserved for conversion of */
  /* f(f2) to i and i to f(f2)                 */
  HPPA_MAC_SAR,			/* shift amount register                     */
  HPPA_MAC_DP,			/* data pointer gr27                         */
  HPPA_MAC_FZERO,
  HPPA_MAC_TRUE_SP,
  HPPA_MAC_DYNCALL,		/* parameter for dynamic function call(gr22) */
  HPPA_MAC_FLOAT_CBIT,		/* floating point nullification condition bit*/
  HPPA_MAC_SWAP_PTR,
  HPPA_MAC_SR3,			/* space registers 3 and 5                   */
  HPPA_MAC_SR5,

  /* PlayDoh Emulation Macro registers */
  HPPA_MAC_PLAYDOH_GPR,
  HPPA_MAC_PLAYDOH_GPM,
  HPPA_MAC_PLAYDOH_SRC0,
  HPPA_MAC_PLAYDOH_SRC1,
  HPPA_MAC_PLAYDOH_SRC2,
  HPPA_MAC_PLAYDOH_SRC3,
  HPPA_MAC_PLAYDOH_PRED0,
  HPPA_MAC_PLAYDOH_PRED1,
  HPPA_MAC_PLAYDOH_PSRC0,
  HPPA_MAC_PLAYDOH_PDST0,
  HPPA_MAC_PLAYDOH_PDST1,
  HPPA_MAC_PLAYDOH_DEST0,
  HPPA_MAC_PLAYDOH_DEST1,
  HPPA_MAC_PLAYDOH_BRTARGET,
  HPPA_MAC_PLAYDOH_BRTMP,
  HPPA_MAC_PLAYDOH_FSRC0,
  HPPA_MAC_PLAYDOH_FSRC1,
  HPPA_MAC_PLAYDOH_FSRC2,
  HPPA_MAC_PLAYDOH_FDEST0,

  HPPA_MAC_PLAYDOH_FONE = 500,	/* This number this way to be the same
				   as l_playdoh.h - SAM 6-95 */
  HPPA_MAC_PLAYDOH_PRED_FALSE,
  HPPA_MAC_PLAYDOH_PRED_TRUE,
  HPPA_MAC_PLAYDOH_LC,
  HPPA_MAC_PLAYDOH_ESC,
  HPPA_MAC_PLAYDOH_RRB,

  HPPA_MAC_PLAYDOH_PRED_ALL_ROT,
  HPPA_MAC_PLAYDOH_PRED_ALL_STATIC,

  HPPA_MAC_PLAYDOH_PV_0,
  HPPA_MAC_PLAYDOH_PV_1,
  HPPA_MAC_PLAYDOH_PV_2,
  HPPA_MAC_PLAYDOH_PV_3,
  HPPA_MAC_PLAYDOH_PV_4,
  HPPA_MAC_PLAYDOH_PV_5,
  HPPA_MAC_PLAYDOH_PV_6,
  HPPA_MAC_PLAYDOH_PV_7,

  HPPA_MAC_LAST			/* Please not to put anything after this. :) */
};

/*
 * PA Processor Specific Opcodes 
 */

#define LHPPAop_ADDIL		1000
#define LHPPAop_LDO		1001
#define LHPPAop_FMPYADD		1002
#define LHPPAop_FMPYSUB		1003

#define LHPPAop_MTSP		1005
#define LHPPAop_ZVDEP		1006
#define LHPPAop_VSHD		1007
#define	LHPPAop_VEXTRS		1008
#define LHPPAop_COMB_EQ_FWD	1009
#define LHPPAop_COMB_NE_FWD	1010
#define LHPPAop_COMB_GT_FWD	1011
#define LHPPAop_COMB_GE_FWD	1012
#define LHPPAop_COMB_LT_FWD	1013
#define LHPPAop_COMB_LE_FWD	1014
#define LHPPAop_COMB_GT_U_FWD	1015
#define LHPPAop_COMB_GE_U_FWD	1016
#define LHPPAop_COMB_LT_U_FWD	1017
#define LHPPAop_COMB_LE_U_FWD	1018
#define LHPPAop_COMIB_EQ_FWD	1019
#define LHPPAop_COMIB_NE_FWD	1020
#define LHPPAop_COMIB_GT_FWD	1021
#define LHPPAop_COMIB_GE_FWD	1022
#define LHPPAop_COMIB_LT_FWD	1023
#define LHPPAop_COMIB_LE_FWD	1024
#define LHPPAop_COMIB_GT_U_FWD	1025
#define LHPPAop_COMIB_GE_U_FWD	1026
#define LHPPAop_COMIB_LT_U_FWD	1027
#define LHPPAop_COMIB_LE_U_FWD	1028
#define LHPPAop_COMB_EQ_BWD	1029
#define LHPPAop_COMB_NE_BWD	1030
#define LHPPAop_COMB_GT_BWD	1031
#define LHPPAop_COMB_GE_BWD	1032
#define LHPPAop_COMB_LT_BWD	1033
#define LHPPAop_COMB_LE_BWD	1034
#define LHPPAop_COMB_GT_U_BWD	1035
#define LHPPAop_COMB_GE_U_BWD	1036
#define LHPPAop_COMB_LT_U_BWD	1037
#define LHPPAop_COMB_LE_U_BWD	1038
#define LHPPAop_COMIB_EQ_BWD	1039
#define LHPPAop_COMIB_NE_BWD	1040
#define LHPPAop_COMIB_GT_BWD	1041
#define LHPPAop_COMIB_GE_BWD	1042
#define LHPPAop_COMIB_LT_BWD	1043
#define LHPPAop_COMIB_LE_BWD	1044
#define LHPPAop_COMIB_GT_U_BWD	1045
#define LHPPAop_COMIB_GE_U_BWD	1046
#define LHPPAop_COMIB_LT_U_BWD	1047
#define LHPPAop_COMIB_LE_U_BWD	1048
#define LHPPAop_BB_0_FWD	1049
#define LHPPAop_BB_1_FWD	1050
#define LHPPAop_BB_0_BWD	1051
#define LHPPAop_BB_1_BWD	1052
#define LHPPAop_LDIL		1053
#define LHPPAop_ADDIB_LT_FWD    1054
#define LHPPAop_ADDIB_LT_BWD    1055
#define LHPPAop_EXTRU		1056
#define LHPPAop_DEPI		1057
#define LHPPAop_JSR_DYNCALL     1058
#define LHPPAop_ZDEPI		1059
#define LHPPAop_ZVDEPI		1060
#define LHPPAop_ASR_DIV		1061

#define LHPPAop_LD_UC_SV1	1100
#define LHPPAop_LD_UC_SC1	1101
#define LHPPAop_LD_UC_SC2	1102
#define LHPPAop_LD_UC_SC3	1103

#define LHPPAop_LD_PRE_UC_SV1	1104
#define LHPPAop_LD_PRE_UC_SC1	1105
#define LHPPAop_LD_PRE_UC_SC2	1106
#define LHPPAop_LD_PRE_UC_SC3	1107

#define LHPPAop_LD_POST_UC_SV1	1108
#define LHPPAop_LD_POST_UC_SC1	1109
#define LHPPAop_LD_POST_UC_SC2	1110
#define LHPPAop_LD_POST_UC_SC3	1111

#define LHPPAop_LD_C_SV1	1112
#define LHPPAop_LD_C_SC1	1113
#define LHPPAop_LD_C_SC2	1114
#define LHPPAop_LD_C_SC3	1115

#define LHPPAop_LD_PRE_C_SV1	1116
#define LHPPAop_LD_PRE_C_SC1	1117
#define LHPPAop_LD_PRE_C_SC2	1118
#define LHPPAop_LD_PRE_C_SC3	1119

#define LHPPAop_LD_POST_C_SV1	1120
#define LHPPAop_LD_POST_C_SC1	1121
#define LHPPAop_LD_POST_C_SC2	1122
#define LHPPAop_LD_POST_C_SC3	1123

#define LHPPAop_LD_UC2_SV1	1124
#define LHPPAop_LD_UC2_SC1	1125
#define LHPPAop_LD_UC2_SC2	1126
#define LHPPAop_LD_UC2_SC3	1127

#define LHPPAop_LD_PRE_UC2_SV1	1128
#define LHPPAop_LD_PRE_UC2_SC1	1129
#define LHPPAop_LD_PRE_UC2_SC2	1130
#define LHPPAop_LD_PRE_UC2_SC3	1131

#define LHPPAop_LD_POST_UC2_SV1	1132
#define LHPPAop_LD_POST_UC2_SC1	1133
#define LHPPAop_LD_POST_UC2_SC2	1134
#define LHPPAop_LD_POST_UC2_SC3	1135

#define LHPPAop_LD_C2_SV1	1136
#define LHPPAop_LD_C2_SC1	1137
#define LHPPAop_LD_C2_SC2	1138
#define LHPPAop_LD_C2_SC3	1139

#define LHPPAop_LD_PRE_C2_SV1	1140
#define LHPPAop_LD_PRE_C2_SC1	1141
#define LHPPAop_LD_PRE_C2_SC2	1142
#define LHPPAop_LD_PRE_C2_SC3	1143

#define LHPPAop_LD_POST_C2_SV1	1144
#define LHPPAop_LD_POST_C2_SC1	1145
#define LHPPAop_LD_POST_C2_SC2	1146
#define LHPPAop_LD_POST_C2_SC3	1147

#define LHPPAop_LD_I_SV1	1148
#define LHPPAop_LD_I_SC1	1149
#define LHPPAop_LD_I_SC2	1150
#define LHPPAop_LD_I_SC3	1151

#define LHPPAop_LD_PRE_I_SV1	1152
#define LHPPAop_LD_PRE_I_SC1	1153
#define LHPPAop_LD_PRE_I_SC2	1154
#define LHPPAop_LD_PRE_I_SC3	1155

#define LHPPAop_LD_POST_I_SV1	1156
#define LHPPAop_LD_POST_I_SC1	1157
#define LHPPAop_LD_POST_I_SC2	1158
#define LHPPAop_LD_POST_I_SC3	1159

#define LHPPAop_LD_F_SV1	1160
#define LHPPAop_LD_F_SC1	1161
#define LHPPAop_LD_F_SC2	1162
#define LHPPAop_LD_F_SC3	1163

#define LHPPAop_LD_PRE_F_SV1	1164
#define LHPPAop_LD_PRE_F_SC1	1165
#define LHPPAop_LD_PRE_F_SC2	1166
#define LHPPAop_LD_PRE_F_SC3	1167

#define LHPPAop_LD_POST_F_SV1	1168
#define LHPPAop_LD_POST_F_SC1	1169
#define LHPPAop_LD_POST_F_SC2	1170
#define LHPPAop_LD_POST_F_SC3	1171

#define LHPPAop_LD_F2_SV1	1172
#define LHPPAop_LD_F2_SC1	1173
#define LHPPAop_LD_F2_SC2	1174
#define LHPPAop_LD_F2_SC3	1175

#define LHPPAop_LD_PRE_F2_SV1	1176
#define LHPPAop_LD_PRE_F2_SC1	1177
#define LHPPAop_LD_PRE_F2_SC2	1178
#define LHPPAop_LD_PRE_F2_SC3	1179

#define LHPPAop_LD_POST_F2_SV1	1180
#define LHPPAop_LD_POST_F2_SC1	1181
#define LHPPAop_LD_POST_F2_SC2	1182
#define LHPPAop_LD_POST_F2_SC3	1183



#define LHPPAop_PRED_MOV	1200

#define PAopcode_ADDIL		"LHPPAop_ADDIL"
#define PAopcode_LDO		"LHPPAop_LDO"
#define PAopcode_FMPYADD	"LHPPAop_FMPYADD"
#define PAopcode_FMPYSUB	"LHPPAop_FMPYSUB"
#define PAopcode_MTSP		"LHPPAop_MTSP"
#define PAopcode_ZVDEP		"LHPPAop_ZVDEP"
#define PAopcode_VSHD		"LHPPAop_VSHD"
#define	PAopcode_VEXTRS		"LHPPAop_VEXTRS"
#define PAopcode_COMB_EQ_FWD	"LHPPAop_COMB_EQ_FWD"
#define PAopcode_COMB_NE_FWD	"LHPPAop_COMB_NE_FWD"
#define PAopcode_COMB_GT_FWD	"LHPPAop_COMB_GT_FWD"
#define PAopcode_COMB_GE_FWD	"LHPPAop_COMB_GE_FWD"
#define PAopcode_COMB_LT_FWD	"LHPPAop_COMB_LT_FWD"
#define PAopcode_COMB_LE_FWD	"LHPPAop_COMB_LE_FWD"
#define PAopcode_COMB_GT_U_FWD	"LHPPAop_COMB_GT_U_FWD"
#define PAopcode_COMB_GE_U_FWD	"LHPPAop_COMB_GE_U_FWD"
#define PAopcode_COMB_LT_U_FWD	"LHPPAop_COMB_LT_U_FWD"
#define PAopcode_COMB_LE_U_FWD	"LHPPAop_COMB_LE_U_FWD"
#define PAopcode_COMIB_EQ_FWD	"LHPPAop_COMIB_EQ_FWD"
#define PAopcode_COMIB_NE_FWD	"LHPPAop_COMIB_NE_FWD"
#define PAopcode_COMIB_GT_FWD	"LHPPAop_COMIB_GT_FWD"
#define PAopcode_COMIB_GE_FWD	"LHPPAop_COMIB_GE_FWD"
#define PAopcode_COMIB_LT_FWD	"LHPPAop_COMIB_LT_FWD"
#define PAopcode_COMIB_LE_FWD	"LHPPAop_COMIB_LE_FWD"
#define PAopcode_COMIB_GT_U_FWD	"LHPPAop_COMIB_GT_U_FWD"
#define PAopcode_COMIB_GE_U_FWD	"LHPPAop_COMIB_GE_U_FWD"
#define PAopcode_COMIB_LT_U_FWD	"LHPPAop_COMIB_LT_U_FWD"
#define PAopcode_COMIB_LE_U_FWD	"LHPPAop_COMIB_LE_U_FWD"
#define PAopcode_COMB_EQ_BWD	"LHPPAop_COMB_EQ_BWD"
#define PAopcode_COMB_NE_BWD	"LHPPAop_COMB_NE_BWD"
#define PAopcode_COMB_GT_BWD	"LHPPAop_COMB_GT_BWD"
#define PAopcode_COMB_GE_BWD	"LHPPAop_COMB_GE_BWD"
#define PAopcode_COMB_LT_BWD	"LHPPAop_COMB_LT_BWD"
#define PAopcode_COMB_LE_BWD	"LHPPAop_COMB_LE_BWD"
#define PAopcode_COMB_GT_U_BWD	"LHPPAop_COMB_GT_U_BWD"
#define PAopcode_COMB_GE_U_BWD	"LHPPAop_COMB_GE_U_BWD"
#define PAopcode_COMB_LT_U_BWD	"LHPPAop_COMB_LT_U_BWD"
#define PAopcode_COMB_LE_U_BWD	"LHPPAop_COMB_LE_U_BWD"
#define PAopcode_COMIB_EQ_BWD	"LHPPAop_COMIB_EQ_BWD"
#define PAopcode_COMIB_NE_BWD	"LHPPAop_COMIB_NE_BWD"
#define PAopcode_COMIB_GT_BWD	"LHPPAop_COMIB_GT_BWD"
#define PAopcode_COMIB_GE_BWD	"LHPPAop_COMIB_GE_BWD"
#define PAopcode_COMIB_LT_BWD	"LHPPAop_COMIB_LT_BWD"
#define PAopcode_COMIB_LE_BWD	"LHPPAop_COMIB_LE_BWD"
#define PAopcode_COMIB_GT_U_BWD	"LHPPAop_COMIB_GT_U_BWD"
#define PAopcode_COMIB_GE_U_BWD	"LHPPAop_COMIB_GE_U_BWD"
#define PAopcode_COMIB_LT_U_BWD	"LHPPAop_COMIB_LT_U_BWD"
#define PAopcode_COMIB_LE_U_BWD	"LHPPAop_COMIB_LE_U_BWD"
#define PAopcode_BB_0_FWD	"LHPPAop_BB_0_FWD"
#define PAopcode_BB_1_FWD	"LHPPAop_BB_1_FWD"
#define PAopcode_BB_0_BWD	"LHPPAop_BB_0_BWD"
#define PAopcode_BB_1_BWD	"LHPPAop_BB_1_BWD"
#define PAopcode_LDIL		"LHPPAop_LDIL"
#define PAopcode_ADDIB_LT_FWD   "LHPPAop_ADDIB_LT_FWD"
#define PAopcode_ADDIB_LT_BWD   "LHPPAop_ADDIB_LT_BWD"
#define PAopcode_EXTRU		"LHPPAop_EXTRU"
#define PAopcode_DEPI		"LHPPAop_DEPI"
#define PAopcode_JSR_DYNCALL    "LHPPAop_JSR_DYNCALL"
#define PAopcode_ZVDEPI		"LHPPAop_ZVDEPI"

#define PAopcode_LD_UC_SV1	"LHPPA_LD_UC_SV1"
#define PAopcode_LD_UC_SC1	"LHPPA_LD_UC_SC1"
#define PAopcode_LD_UC_SC2	"LHPPA_LD_UC_SC2"
#define PAopcode_LD_UC_SC3	"LHPPA_LD_UC_SC3"

#define PAopcode_LD_PRE_UC_SV1	"LHPPA_LD_PRE_UC_SV1"
#define PAopcode_LD_PRE_UC_SC1	"LHPPA_LD_PRE_UC_SC1"
#define PAopcode_LD_PRE_UC_SC2	"LHPPA_LD_PRE_UC_SC2"
#define PAopcode_LD_PRE_UC_SC3	"LHPPA_LD_PRE_UC_SC3"

#define PAopcode_LD_POST_UC_SV1	"LHPPA_LD_POST_UC_SV1"
#define PAopcode_LD_POST_UC_SC1	"LHPPA_LD_POST_UC_SC1"
#define PAopcode_LD_POST_UC_SC2	"LHPPA_LD_POST_UC_SC2"
#define PAopcode_LD_POST_UC_SC3	"LHPPA_LD_POST_UC_SC3"

#define PAopcode_LD_C_SV1	"LHPPA_LD_C_SV1"
#define PAopcode_LD_C_SC1	"LHPPA_LD_C_SC1"
#define PAopcode_LD_C_SC2	"LHPPA_LD_C_SC2"
#define PAopcode_LD_C_SC3	"LHPPA_LD_C_SC3"

#define PAopcode_LD_PRE_C_SV1	"LHPPA_LD_PRE_C_SV1"
#define PAopcode_LD_PRE_C_SC1	"LHPPA_LD_PRE_C_SC1"
#define PAopcode_LD_PRE_C_SC2	"LHPPA_LD_PRE_C_SC2"
#define PAopcode_LD_PRE_C_SC3	"LHPPA_LD_PRE_C_SC3"

#define PAopcode_LD_POST_C_SV1	"LHPPA_LD_POST_C_SV1"
#define PAopcode_LD_POST_C_SC1	"LHPPA_LD_POST_C_SC1"
#define PAopcode_LD_POST_C_SC2	"LHPPA_LD_POST_C_SC2"
#define PAopcode_LD_POST_C_SC3	"LHPPA_LD_POST_C_SC3"

#define PAopcode_LD_UC2_SV1	"LHPPA_LD_UC2_SV1"
#define PAopcode_LD_UC2_SC1	"LHPPA_LD_UC2_SC1"
#define PAopcode_LD_UC2_SC2	"LHPPA_LD_UC2_SC2"
#define PAopcode_LD_UC2_SC3	"LHPPA_LD_UC2_SC3"

#define PAopcode_LD_PRE_UC2_SV1	"LHPPA_LD_PRE_UC2_SV1"
#define PAopcode_LD_PRE_UC2_SC1	"LHPPA_LD_PRE_UC2_SC1"
#define PAopcode_LD_PRE_UC2_SC2	"LHPPA_LD_PRE_UC2_SC2"
#define PAopcode_LD_PRE_UC2_SC3	"LHPPA_LD_PRE_UC2_SC3"

#define PAopcode_LD_POST_UC2_SV1	"LHPPA_LD_POST_UC2_SV1"
#define PAopcode_LD_POST_UC2_SC1	"LHPPA_LD_POST_UC2_SC1"
#define PAopcode_LD_POST_UC2_SC2	"LHPPA_LD_POST_UC2_SC2"
#define PAopcode_LD_POST_UC2_SC3	"LHPPA_LD_POST_UC2_SC3"

#define PAopcode_LD_C2_SV1	"LHPPA_LD_C2_SV1"
#define PAopcode_LD_C2_SC1	"LHPPA_LD_C2_SC1"
#define PAopcode_LD_C2_SC2	"LHPPA_LD_C2_SC2"
#define PAopcode_LD_C2_SC3	"LHPPA_LD_C2_SC3"

#define PAopcode_LD_PRE_C2_SV1	"LHPPA_LD_PRE_C2_SV1"
#define PAopcode_LD_PRE_C2_SC1	"LHPPA_LD_PRE_C2_SC1"
#define PAopcode_LD_PRE_C2_SC2	"LHPPA_LD_PRE_C2_SC2"
#define PAopcode_LD_PRE_C2_SC3	"LHPPA_LD_PRE_C2_SC3"

#define PAopcode_LD_POST_C2_SV1	"LHPPA_LD_POST_C2_SV1"
#define PAopcode_LD_POST_C2_SC1	"LHPPA_LD_POST_C2_SC1"
#define PAopcode_LD_POST_C2_SC2	"LHPPA_LD_POST_C2_SC2"
#define PAopcode_LD_POST_C2_SC3	"LHPPA_LD_POST_C2_SC3"

#define PAopcode_LD_I_SV1	"LHPPA_LD_I_SV1"
#define PAopcode_LD_I_SC1	"LHPPA_LD_I_SC1"
#define PAopcode_LD_I_SC2	"LHPPA_LD_I_SC2"
#define PAopcode_LD_I_SC3	"LHPPA_LD_I_SC3"

#define PAopcode_LD_PRE_I_SV1	"LHPPA_LD_PRE_I_SV1"
#define PAopcode_LD_PRE_I_SC1	"LHPPA_LD_PRE_I_SC1"
#define PAopcode_LD_PRE_I_SC2	"LHPPA_LD_PRE_I_SC2"
#define PAopcode_LD_PRE_I_SC3	"LHPPA_LD_PRE_I_SC3"

#define PAopcode_LD_POST_I_SV1	"LHPPA_LD_POST_I_SV1"
#define PAopcode_LD_POST_I_SC1	"LHPPA_LD_POST_I_SC1"
#define PAopcode_LD_POST_I_SC2	"LHPPA_LD_POST_I_SC2"
#define PAopcode_LD_POST_I_SC3	"LHPPA_LD_POST_I_SC3"

#define PAopcode_LD_F_SV1	"LHPPA_LD_F_SV1"
#define PAopcode_LD_F_SC1	"LHPPA_LD_F_SC1"
#define PAopcode_LD_F_SC2	"LHPPA_LD_F_SC2"
#define PAopcode_LD_F_SC3	"LHPPA_LD_F_SC3"

#define PAopcode_LD_PRE_F_SV1	"LHPPA_LD_PRE_F_SV1"
#define PAopcode_LD_PRE_F_SC1	"LHPPA_LD_PRE_F_SC1"
#define PAopcode_LD_PRE_F_SC2	"LHPPA_LD_PRE_F_SC2"
#define PAopcode_LD_PRE_F_SC3	"LHPPA_LD_PRE_F_SC3"

#define PAopcode_LD_POST_F_SV1	"LHPPA_LD_POST_F_SV1"
#define PAopcode_LD_POST_F_SC1	"LHPPA_LD_POST_F_SC1"
#define PAopcode_LD_POST_F_SC2	"LHPPA_LD_POST_F_SC2"
#define PAopcode_LD_POST_F_SC3	"LHPPA_LD_POST_F_SC3"

#define PAopcode_LD_F2_SV1	"LHPPA_LD_F2_SV1"
#define PAopcode_LD_F2_SC1	"LHPPA_LD_F2_SC1"
#define PAopcode_LD_F2_SC2	"LHPPA_LD_F2_SC2"
#define PAopcode_LD_F2_SC3	"LHPPA_LD_F2_SC3"

#define PAopcode_LD_PRE_F2_SV1	"LHPPA_LD_PRE_F2_SV1"
#define PAopcode_LD_PRE_F2_SC1	"LHPPA_LD_PRE_F2_SC1"
#define PAopcode_LD_PRE_F2_SC2	"LHPPA_LD_PRE_F2_SC2"
#define PAopcode_LD_PRE_F2_SC3	"LHPPA_LD_PRE_F2_SC3"

#define PAopcode_LD_POST_F2_SV1	"LHPPA_LD_POST_F2_SV1"
#define PAopcode_LD_POST_F2_SC1	"LHPPA_LD_POST_F2_SC1"
#define PAopcode_LD_POST_F2_SC2	"LHPPA_LD_POST_F2_SC2"
#define PAopcode_LD_POST_F2_SC3	"LHPPA_LD_POST_F2_SC3"

#define PAopcode_PRED_MOV	"LHPPAop_PRED_MOV"


/* Define hppa specific mdes IO_set specifiers */

/*   There are none!  :)  */



/* maximum values for immediate operands */
#define INT_2EXP13		0x2000
#define INT_2EXP10		0x400
#define INT_2EXP4		0x10

#define MAX_INT_NAME		22

#define FIELD_5(a)	((((a) >= -INT_2EXP4)&&((a) < INT_2EXP4))?1:0)
#define FIELD_11(a)	((((a) >= -INT_2EXP10)&&((a) < INT_2EXP10))?1:0)
#define FIELD_14(a)	((((a) >= -INT_2EXP13)&&((a) < INT_2EXP13))?1:0)

#ifdef __cplusplus
extern "C"
{
#endif

  extern int M_hppa_type_size (int mtype);
  extern int M_hppa_type_align (int mtype);
  extern void M_hppa_void (M_Type type);
  extern void M_hppa_bit_long (M_Type type, int n);
  extern void M_hppa_bit_int (M_Type type, int n);
  extern void M_hppa_bit_short (M_Type type, int n);
  extern void M_hppa_bit_char (M_Type type, int n);
  extern void M_hppa_char (M_Type type, int unsign);
  extern void M_hppa_short (M_Type type, int unsign);
  extern void M_hppa_int (M_Type type, int unsign);
  extern void M_hppa_long (M_Type type, int unsign);
  extern void M_hppa_float (M_Type type, int unsign);
  extern void M_hppa_double (M_Type type, int unsign);
  extern void M_hppa_pointer (M_Type type);
  extern int M_hppa_eval_type (M_Type type, M_Type ntype);
  extern int M_hppa_eval_type2 (M_Type type, M_Type ntype);
  extern int M_hppa_call_type (M_Type type, M_Type ntype);
  extern int M_hppa_call_type2 (M_Type type, M_Type ntype);
  extern void M_hppa_array_layout (M_Type type, int *offset);
  extern int M_hppa_array_align (M_Type type);
  extern int M_hppa_array_size (M_Type type, int dim);
  extern void M_hppa_union_layout (int n, _M_Type * type, int *offset,
				   int *bit_offset);
  extern int M_hppa_union_align (int n, _M_Type * type);
  extern int M_hppa_union_size (int n, _M_Type * type);
  extern void M_hppa_struct_layout (int n, _M_Type * type, int *base,
				    int *bit_offset);
  extern int M_hppa_struct_align (int n, _M_Type * type);
  extern int M_hppa_struct_size (int n, _M_Type * type, int struct_align);
  extern int M_hppa_layout_fnvar (List param_list, char **base_macro,
				   int *pcount, int purpose);
  extern int M_hppa_fnvar_layout (int n, _M_Type * type, long int *offset,
				  int *mode, int *reg, int *paddr,
				  char **macro, int *su_sreg, int *su_ereg,
				  int *pcount, int is_st, int purpose);
  extern int M_hppa_lvar_layout (int n, _M_Type * type, long int *offset,
				 char **base_macro);
  extern int M_hppa_no_short_int (void);
  extern int M_hppa_layout_order (void);
  extern void M_hppa_cb_label_name (char *fn, int cb, char *line, int len);
  extern int M_hppa_is_cb_label (char *label, char *fn, int *cb);
  extern void M_hppa_jumptbl_label_name (char *fn, int tbl_id, char *line,
					 int len);
  extern int M_hppa_is_jumptbl_label (char *label, char *fn, int *tbl_id);
  extern int M_hppa_structure_pointer (int purpose);
  extern int M_hppa_return_register (int type, int purpose);
  extern char *M_hppa_fn_label_name (char *label,
				     int (*is_func) (char *is_func_label));
  extern char *M_hppa_fn_name_from_label (char *label);
  extern void M_set_model_hppa (char *model_name);
  extern int M_hppa_fragile_macro (int macro_value);
  extern Set M_hppa_fragile_macro_set ();
  extern int M_hppa_dataflow_macro (int id);
  extern int M_hppa_subroutine_call (int opc);
  extern void M_define_macros_hppa (STRING_Symbol_Table * sym_tbl);
  extern char *M_get_macro_name_hppa (int id);
  extern void M_define_opcode_name_hppa (STRING_Symbol_Table * sym_tbl);
  extern char *M_get_opcode_name_hppa (int id);
  extern int M_oper_supported_in_arch_hppa (int opc);
  extern int M_num_oper_required_for_hppa (L_Oper * oper, char *name);
  extern int M_is_stack_operand_hppa (L_Operand * operand);
  extern int M_is_unsafe_macro_hppa (L_Operand * operand);
  extern int M_operand_type_hppa (L_Operand * operand);
  extern int M_conflicting_operands_hppa (L_Operand * operand,
					  L_Operand ** conflict_array,
					  int len, int prepass);
  extern int M_num_registers_hppa (int ctype);

#ifdef __cplusplus
}
#endif

#endif
