/*****************************************************************************\
 *
 *		      Illinois Open Source License
 *                     University of Illinois/NCSA
 *                         Open Source License
 *
 * Copyright (c) 2002, The University of Illinois at Urbana-Champaign.
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
 *  File:  m_hpl_pd.h (Renamed from m_playdoh.h -JCG 7/14/98)
 *
 *  Description: Defines symbols used for reference in Mspec and code
 * 		generator.
 *
 *  Creation Date : August, 1993
 *
 *  Author:  Scott A. Mahlke, Wen-mei Hwu
 *
 *  Revisions:
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

#ifndef M_PLAYDOH_H
#define M_PLAYDOH_H

/* 10/29/02 REK Adding config.h */
#include <config.h>
#include <machine/m_spec.h>

/*
 * Declarations for processor models
 */
enum
{
  M_PLAYDOH_V1 = 0
};

enum
{
  PLAYDOH_MAC_ZERO = 100,	/* zero (gr0)                                */
  PLAYDOH_MAC_TEMPREG,		/* tempreg (gr1) used in prologue/epilogue   */
  PLAYDOH_MAC_RETADDR,		/* return address register (gr2)             */
  PLAYDOH_MAC_FZERO,
  PLAYDOH_MAC_TRUE_SP,

  PLAYDOH_MAC_FONE = 200,	/* Number this way to be same as m_hppa.h,
				   See SAM for details */
  PLAYDOH_MAC_PRED_FALSE,
  PLAYDOH_MAC_PRED_TRUE,

  PLAYDOH_MAC_LC,		/* Loop count */
  PLAYDOH_MAC_ESC,		/* Epilogue stage counter */
  PLAYDOH_MAC_RRB,		/* Rotating register base */

  PLAYDOH_MAC_PRED_ALL_ROT,	/* all the rotating predicates */
  PLAYDOH_MAC_PRED_ALL_STATIC,	/* all the non-rotating predicates */

  PLAYDOH_MAC_PV_0,		/* PV_0, ..., PV_7 are control register */   
  PLAYDOH_MAC_PV_1,		/* aliases for 32-bit wide access to    */
  PLAYDOH_MAC_PV_2,             /* predicate registers                  */
  PLAYDOH_MAC_PV_3,
  PLAYDOH_MAC_PV_4,
  PLAYDOH_MAC_PV_5,
  PLAYDOH_MAC_PV_6,
  PLAYDOH_MAC_PV_7		/* This should not exceed 256, the start
				   of the intrinsic macros */
};

/*=========================================================================*/
/*
 *	Playdoh specific opcodes
 */
/*=========================================================================*/

#define PLAYDOHop_FIRST_OPC	1000

/* prepare to branch ops */
#define PLAYDOHop_PBRR		1001	/* Lop_PBR */
#define PLAYDOHop_PBRA		1002	/* Lop_PBR */

//#if 0
/* sign extension ops */
#define PLAYDOHop_EXTRSB	1003	/* Lop_EXTRACT_C  */
#define PLAYDOHop_EXTRSH	1004	/* Lop_EXTRACT_C2 */
//#endif

/*
 * PlayDoh move extensions
 */
#define PLAYDOHop_MOVEGF_L	1005	/* not handled currently */
#define PLAYDOHop_MOVEGF_U	1006	/* not handled currently */
#define PLAYDOHop_MOVEFG_L	1007	/* not handled currently */
#define PLAYDOHop_MOVEFG_U	1008	/* not handled currently */
#define PLAYDOHop_MOVEPG	1009	/* Lop_MOV */
#define PLAYDOHop_LDCM		1010	/* not handled currently */

/*
 * PlayDoh branch extensions
 */
#define PLAYDOHop_BRU		1011	/* Lop_JUMP */
#define PLAYDOHop_BRCT		1012	/* Lop_BEQ  */
#define PLAYDOHop_BRCF		1013	/* Lop_BEQ  */
#define PLAYDOHop_BRL		1014	/* Lop_JSR  */
#define PLAYDOHop_BRLC		1015	/* Branch on zero loop count, also
					   decrement loop count.
					   Lop_BNE, with a dest reg */

/* brtop and wtop combinations */
#define PLAYDOHop_BRF_B_B_B	1016	/* will never occur */
#define PLAYDOHop_BRF_B_B_F	1017	/* Lop_BEQ */
#define PLAYDOHop_BRF_B_F_B	1018	/* will never occur */
#define PLAYDOHop_BRF_B_F_F	1019	/* Lop_BEQ */
#define PLAYDOHop_BRF_F_B_B	1020	/* Lop_BEQ */
#define PLAYDOHop_BRF_F_B_F	1021	/* will never occur */
#define PLAYDOHop_BRF_F_F_B	1022	/* Lop_BEQ */
#define PLAYDOHop_BRF_F_F_F	1023	/* Lop_BEQ */

#define PLAYDOHop_BRW_B_B_B	1024	/* will never occur */
#define PLAYDOHop_BRW_B_B_F	1025	/* Lop_BEQ */
#define PLAYDOHop_BRW_B_F_B	1026	/* will never occur */
#define PLAYDOHop_BRW_B_F_F	1027	/* Lop_BEQ */
#define PLAYDOHop_BRW_F_B_B	1028	/* Lop_BEQ */
#define PLAYDOHop_BRW_F_B_F	1029	/* will never occur */
#define PLAYDOHop_BRW_F_F_B	1030	/* Lop_BEQ */
#define PLAYDOHop_BRW_F_F_F	1031	/* Lop_BEQ */

/* data verify branches */
#define PLAYDOHop_BRDVI		1032	/* not handled currently */
#define PLAYDOHop_BRDVF		1033	/* not handled currently */

/*
 * PlayDoh integer load operations Lop_LD[_POST]_(UC|UC2|I)
 */
#define PLAYDOHop_L_B_V1_V1	1034	/* Lop_LD_UC */
#define PLAYDOHop_L_B_V1_C1	1035
#define PLAYDOHop_L_B_V1_C2	1036
#define PLAYDOHop_L_B_V1_C3	1037
#define PLAYDOHop_L_B_C1_V1	1038
#define PLAYDOHop_L_B_C1_C1	1039
#define PLAYDOHop_L_B_C1_C2	1040
#define PLAYDOHop_L_B_C1_C3	1041
#define PLAYDOHop_L_B_C2_V1	1042
#define PLAYDOHop_L_B_C2_C1	1043
#define PLAYDOHop_L_B_C2_C2	1044
#define PLAYDOHop_L_B_C2_C3	1045
#define PLAYDOHop_L_B_C3_V1	1046
#define PLAYDOHop_L_B_C3_C1	1047
#define PLAYDOHop_L_B_C3_C2	1048
#define PLAYDOHop_L_B_C3_C3	1049

#define PLAYDOHop_L_H_V1_V1	1050	/* Lop_LD_UC2 */
#define PLAYDOHop_L_H_V1_C1	1051
#define PLAYDOHop_L_H_V1_C2	1052
#define PLAYDOHop_L_H_V1_C3	1053
#define PLAYDOHop_L_H_C1_V1	1054
#define PLAYDOHop_L_H_C1_C1	1055
#define PLAYDOHop_L_H_C1_C2	1056
#define PLAYDOHop_L_H_C1_C3	1057
#define PLAYDOHop_L_H_C2_V1	1058
#define PLAYDOHop_L_H_C2_C1	1059
#define PLAYDOHop_L_H_C2_C2	1060
#define PLAYDOHop_L_H_C2_C3	1061
#define PLAYDOHop_L_H_C3_V1	1062
#define PLAYDOHop_L_H_C3_C1	1063
#define PLAYDOHop_L_H_C3_C2	1064
#define PLAYDOHop_L_H_C3_C3	1065

#define PLAYDOHop_L_W_V1_V1	1066	/* Lop_LD_I */
#define PLAYDOHop_L_W_V1_C1	1067
#define PLAYDOHop_L_W_V1_C2	1068
#define PLAYDOHop_L_W_V1_C3	1069
#define PLAYDOHop_L_W_C1_V1	1070
#define PLAYDOHop_L_W_C1_C1	1071
#define PLAYDOHop_L_W_C1_C2	1072
#define PLAYDOHop_L_W_C1_C3	1073
#define PLAYDOHop_L_W_C2_V1	1074
#define PLAYDOHop_L_W_C2_C1	1075
#define PLAYDOHop_L_W_C2_C2	1076
#define PLAYDOHop_L_W_C2_C3	1077
#define PLAYDOHop_L_W_C3_V1	1078
#define PLAYDOHop_L_W_C3_C1	1079
#define PLAYDOHop_L_W_C3_C2	1080
#define PLAYDOHop_L_W_C3_C3	1081

#define PLAYDOHop_L_Q_V1_V1	1082	/* Lop_LD_Q */
#define PLAYDOHop_L_Q_V1_C1	1083
#define PLAYDOHop_L_Q_V1_C2	1084
#define PLAYDOHop_L_Q_V1_C3	1085
#define PLAYDOHop_L_Q_C1_V1	1086
#define PLAYDOHop_L_Q_C1_C1	1087
#define PLAYDOHop_L_Q_C1_C2	1088
#define PLAYDOHop_L_Q_C1_C3	1089
#define PLAYDOHop_L_Q_C2_V1	1090
#define PLAYDOHop_L_Q_C2_C1	1091
#define PLAYDOHop_L_Q_C2_C2	1092
#define PLAYDOHop_L_Q_C2_C3	1093
#define PLAYDOHop_L_Q_C3_V1	1094
#define PLAYDOHop_L_Q_C3_C1	1095
#define PLAYDOHop_L_Q_C3_C2	1096
#define PLAYDOHop_L_Q_C3_C3	1097

#define PLAYDOHop_LI_B_V1_V1	1098	/* Lop_LD_POST_UC */
#define PLAYDOHop_LI_B_V1_C1	1099
#define PLAYDOHop_LI_B_V1_C2	1100
#define PLAYDOHop_LI_B_V1_C3	1101
#define PLAYDOHop_LI_B_C1_V1	1102
#define PLAYDOHop_LI_B_C1_C1	1103
#define PLAYDOHop_LI_B_C1_C2	1104
#define PLAYDOHop_LI_B_C1_C3	1105
#define PLAYDOHop_LI_B_C2_V1	1106
#define PLAYDOHop_LI_B_C2_C1	1107
#define PLAYDOHop_LI_B_C2_C2	1108
#define PLAYDOHop_LI_B_C2_C3	1109
#define PLAYDOHop_LI_B_C3_V1	1110
#define PLAYDOHop_LI_B_C3_C1	1111
#define PLAYDOHop_LI_B_C3_C2	1112
#define PLAYDOHop_LI_B_C3_C3	1113

#define PLAYDOHop_LI_H_V1_V1	1114	/* Lop_LD_POST_UC2 */
#define PLAYDOHop_LI_H_V1_C1	1115
#define PLAYDOHop_LI_H_V1_C2	1116
#define PLAYDOHop_LI_H_V1_C3	1117
#define PLAYDOHop_LI_H_C1_V1	1118
#define PLAYDOHop_LI_H_C1_C1	1119
#define PLAYDOHop_LI_H_C1_C2	1120
#define PLAYDOHop_LI_H_C1_C3	1121
#define PLAYDOHop_LI_H_C2_V1	1122
#define PLAYDOHop_LI_H_C2_C1	1123
#define PLAYDOHop_LI_H_C2_C2	1124
#define PLAYDOHop_LI_H_C2_C3	1125
#define PLAYDOHop_LI_H_C3_V1	1126
#define PLAYDOHop_LI_H_C3_C1	1127
#define PLAYDOHop_LI_H_C3_C2	1128
#define PLAYDOHop_LI_H_C3_C3	1129

#define PLAYDOHop_LI_W_V1_V1	1130	/* Lop_LD_POST_I */
#define PLAYDOHop_LI_W_V1_C1	1131
#define PLAYDOHop_LI_W_V1_C2	1132
#define PLAYDOHop_LI_W_V1_C3	1133
#define PLAYDOHop_LI_W_C1_V1	1134
#define PLAYDOHop_LI_W_C1_C1	1135
#define PLAYDOHop_LI_W_C1_C2	1136
#define PLAYDOHop_LI_W_C1_C3	1137
#define PLAYDOHop_LI_W_C2_V1	1138
#define PLAYDOHop_LI_W_C2_C1	1139
#define PLAYDOHop_LI_W_C2_C2	1140
#define PLAYDOHop_LI_W_C2_C3	1141
#define PLAYDOHop_LI_W_C3_V1	1142
#define PLAYDOHop_LI_W_C3_C1	1143
#define PLAYDOHop_LI_W_C3_C2	1144
#define PLAYDOHop_LI_W_C3_C3	1145

#define PLAYDOHop_LI_Q_V1_V1	1146	/* Lop_LD_POST_Q */
#define PLAYDOHop_LI_Q_V1_C1	1147
#define PLAYDOHop_LI_Q_V1_C2	1148
#define PLAYDOHop_LI_Q_V1_C3	1149
#define PLAYDOHop_LI_Q_C1_V1	1150
#define PLAYDOHop_LI_Q_C1_C1	1151
#define PLAYDOHop_LI_Q_C1_C2	1152
#define PLAYDOHop_LI_Q_C1_C3	1153
#define PLAYDOHop_LI_Q_C2_V1	1154
#define PLAYDOHop_LI_Q_C2_C1	1155
#define PLAYDOHop_LI_Q_C2_C2	1156
#define PLAYDOHop_LI_Q_C2_C3	1157
#define PLAYDOHop_LI_Q_C3_V1	1158
#define PLAYDOHop_LI_Q_C3_C1	1159
#define PLAYDOHop_LI_Q_C3_C2	1160
#define PLAYDOHop_LI_Q_C3_C3	1161

/*
 * PlayDoh integer store operations Lop_ST[_POST]_(C|C2|I)
 */
#define PLAYDOHop_S_B_V1	1162	/* Lop_ST_C */
#define PLAYDOHop_S_B_C1	1163
#define PLAYDOHop_S_B_C2	1164
#define PLAYDOHop_S_B_C3	1165

#define PLAYDOHop_S_H_V1	1166	/* Lop_ST_C2 */
#define PLAYDOHop_S_H_C1	1167
#define PLAYDOHop_S_H_C2	1168
#define PLAYDOHop_S_H_C3	1169

#define PLAYDOHop_S_W_V1	1170	/* Lop_ST_I */
#define PLAYDOHop_S_W_C1	1171
#define PLAYDOHop_S_W_C2	1172
#define PLAYDOHop_S_W_C3	1173

#define PLAYDOHop_S_Q_V1	1174	/* Lop_ST_Q */
#define PLAYDOHop_S_Q_C1	1175
#define PLAYDOHop_S_Q_C2	1176
#define PLAYDOHop_S_Q_C3	1177

#define PLAYDOHop_SI_B_V1	1178	/* Lop_ST_POST_C */
#define PLAYDOHop_SI_B_C1	1179
#define PLAYDOHop_SI_B_C2	1180
#define PLAYDOHop_SI_B_C3	1181

#define PLAYDOHop_SI_H_V1	1182	/* Lop_ST_POST_C2 */
#define PLAYDOHop_SI_H_C1	1183
#define PLAYDOHop_SI_H_C2	1184
#define PLAYDOHop_SI_H_C3	1185

#define PLAYDOHop_SI_W_V1	1186	/* Lop_ST_POST_I */
#define PLAYDOHop_SI_W_C1	1187
#define PLAYDOHop_SI_W_C2	1188
#define PLAYDOHop_SI_W_C3	1189

#define PLAYDOHop_SI_Q_V1	1190	/* Lop_ST_POST_Q */
#define PLAYDOHop_SI_Q_C1	1191
#define PLAYDOHop_SI_Q_C2	1192
#define PLAYDOHop_SI_Q_C3	1193

/*
 * PlayDoh floating point load operations Lop_LD[_POST]_F[2]
 */
#define PLAYDOHop_FL_S_V1_V1	1194	/* Lop_LD_F */
#define PLAYDOHop_FL_S_V1_C1	1195
#define PLAYDOHop_FL_S_V1_C2	1196
#define PLAYDOHop_FL_S_V1_C3	1197
#define PLAYDOHop_FL_S_C1_V1	1198
#define PLAYDOHop_FL_S_C1_C1	1199
#define PLAYDOHop_FL_S_C1_C2	1200
#define PLAYDOHop_FL_S_C1_C3	1201
#define PLAYDOHop_FL_S_C2_V1	1202
#define PLAYDOHop_FL_S_C2_C1	1203
#define PLAYDOHop_FL_S_C2_C2	1204
#define PLAYDOHop_FL_S_C2_C3	1205
#define PLAYDOHop_FL_S_C3_V1	1206
#define PLAYDOHop_FL_S_C3_C1	1207
#define PLAYDOHop_FL_S_C3_C2	1208
#define PLAYDOHop_FL_S_C3_C3	1209

#define PLAYDOHop_FL_D_V1_V1	1210	/* Lop_LD_F2 */
#define PLAYDOHop_FL_D_V1_C1	1211
#define PLAYDOHop_FL_D_V1_C2	1212
#define PLAYDOHop_FL_D_V1_C3	1213
#define PLAYDOHop_FL_D_C1_V1	1214
#define PLAYDOHop_FL_D_C1_C1	1215
#define PLAYDOHop_FL_D_C1_C2	1216
#define PLAYDOHop_FL_D_C1_C3	1217
#define PLAYDOHop_FL_D_C2_V1	1218
#define PLAYDOHop_FL_D_C2_C1	1219
#define PLAYDOHop_FL_D_C2_C2	1220
#define PLAYDOHop_FL_D_C2_C3	1221
#define PLAYDOHop_FL_D_C3_V1	1222
#define PLAYDOHop_FL_D_C3_C1	1223
#define PLAYDOHop_FL_D_C3_C2	1224
#define PLAYDOHop_FL_D_C3_C3	1225

#define PLAYDOHop_FLI_S_V1_V1	1226	/* Lop_LD_POST_F */
#define PLAYDOHop_FLI_S_V1_C1	1227
#define PLAYDOHop_FLI_S_V1_C2	1228
#define PLAYDOHop_FLI_S_V1_C3	1229
#define PLAYDOHop_FLI_S_C1_V1	1230
#define PLAYDOHop_FLI_S_C1_C1	1231
#define PLAYDOHop_FLI_S_C1_C2	1232
#define PLAYDOHop_FLI_S_C1_C3	1233
#define PLAYDOHop_FLI_S_C2_V1	1234
#define PLAYDOHop_FLI_S_C2_C1	1235
#define PLAYDOHop_FLI_S_C2_C2	1236
#define PLAYDOHop_FLI_S_C2_C3	1237
#define PLAYDOHop_FLI_S_C3_V1	1238
#define PLAYDOHop_FLI_S_C3_C1	1239
#define PLAYDOHop_FLI_S_C3_C2	1240
#define PLAYDOHop_FLI_S_C3_C3	1241

#define PLAYDOHop_FLI_D_V1_V1	1242	/* Lop_LD_POST_F2 */
#define PLAYDOHop_FLI_D_V1_C1	1243
#define PLAYDOHop_FLI_D_V1_C2	1244
#define PLAYDOHop_FLI_D_V1_C3	1245
#define PLAYDOHop_FLI_D_C1_V1	1246
#define PLAYDOHop_FLI_D_C1_C1	1247
#define PLAYDOHop_FLI_D_C1_C2	1248
#define PLAYDOHop_FLI_D_C1_C3	1249
#define PLAYDOHop_FLI_D_C2_V1	1250
#define PLAYDOHop_FLI_D_C2_C1	1251
#define PLAYDOHop_FLI_D_C2_C2	1252
#define PLAYDOHop_FLI_D_C2_C3	1253
#define PLAYDOHop_FLI_D_C3_V1	1254
#define PLAYDOHop_FLI_D_C3_C1	1255
#define PLAYDOHop_FLI_D_C3_C2	1256
#define PLAYDOHop_FLI_D_C3_C3	1257

/*
 * PlayDoh floating point store operations Lop_ST[_POST]_F[2]
 */
#define PLAYDOHop_FS_S_V1	1258	/* Lop_ST_F */
#define PLAYDOHop_FS_S_C1	1259
#define PLAYDOHop_FS_S_C2	1260
#define PLAYDOHop_FS_S_C3	1261

#define PLAYDOHop_FS_D_V1	1262	/* Lop_ST_F2 */
#define PLAYDOHop_FS_D_C1	1263
#define PLAYDOHop_FS_D_C2	1264
#define PLAYDOHop_FS_D_C3	1265

#define PLAYDOHop_FSI_S_V1	1266	/* Lop_ST_POST_F */
#define PLAYDOHop_FSI_S_C1	1267
#define PLAYDOHop_FSI_S_C2	1268
#define PLAYDOHop_FSI_S_C3	1269

#define PLAYDOHop_FSI_D_V1	1270	/* Lop_ST_POST_F2 */
#define PLAYDOHop_FSI_D_C1	1271
#define PLAYDOHop_FSI_D_C2	1272
#define PLAYDOHop_FSI_D_C3	1273

/* 
 * PlayDoh data speculative integer and floating point load operations
 */
#define PLAYDOHop_LDS_B_V1_V1	1274	/* Lop_LD_UC */
#define PLAYDOHop_LDS_B_V1_C1	1275
#define PLAYDOHop_LDS_B_V1_C2	1276
#define PLAYDOHop_LDS_B_V1_C3	1277
#define PLAYDOHop_LDS_B_C1_V1	1278
#define PLAYDOHop_LDS_B_C1_C1	1279
#define PLAYDOHop_LDS_B_C1_C2	1280
#define PLAYDOHop_LDS_B_C1_C3	1281
#define PLAYDOHop_LDS_B_C2_V1	1282
#define PLAYDOHop_LDS_B_C2_C1	1283
#define PLAYDOHop_LDS_B_C2_C2	1284
#define PLAYDOHop_LDS_B_C2_C3	1285
#define PLAYDOHop_LDS_B_C3_V1	1286
#define PLAYDOHop_LDS_B_C3_C1	1287
#define PLAYDOHop_LDS_B_C3_C2	1288
#define PLAYDOHop_LDS_B_C3_C3	1289

#define PLAYDOHop_LDS_H_V1_V1	1290	/* Lop_LD_UC2 */
#define PLAYDOHop_LDS_H_V1_C1	1291
#define PLAYDOHop_LDS_H_V1_C2	1292
#define PLAYDOHop_LDS_H_V1_C3	1293
#define PLAYDOHop_LDS_H_C1_V1	1294
#define PLAYDOHop_LDS_H_C1_C1	1295
#define PLAYDOHop_LDS_H_C1_C2	1296
#define PLAYDOHop_LDS_H_C1_C3	1297
#define PLAYDOHop_LDS_H_C2_V1	1298
#define PLAYDOHop_LDS_H_C2_C1	1299
#define PLAYDOHop_LDS_H_C2_C2	1300
#define PLAYDOHop_LDS_H_C2_C3	1301
#define PLAYDOHop_LDS_H_C3_V1	1302
#define PLAYDOHop_LDS_H_C3_C1	1303
#define PLAYDOHop_LDS_H_C3_C2	1304
#define PLAYDOHop_LDS_H_C3_C3	1305

#define PLAYDOHop_LDS_W_V1_V1	1306	/* Lop_LD_I */
#define PLAYDOHop_LDS_W_V1_C1	1307
#define PLAYDOHop_LDS_W_V1_C2	1308
#define PLAYDOHop_LDS_W_V1_C3	1309
#define PLAYDOHop_LDS_W_C1_V1	1310
#define PLAYDOHop_LDS_W_C1_C1	1311
#define PLAYDOHop_LDS_W_C1_C2	1312
#define PLAYDOHop_LDS_W_C1_C3	1313
#define PLAYDOHop_LDS_W_C2_V1	1314
#define PLAYDOHop_LDS_W_C2_C1	1315
#define PLAYDOHop_LDS_W_C2_C2	1316
#define PLAYDOHop_LDS_W_C2_C3	1317
#define PLAYDOHop_LDS_W_C3_V1	1318
#define PLAYDOHop_LDS_W_C3_C1	1319
#define PLAYDOHop_LDS_W_C3_C2	1320
#define PLAYDOHop_LDS_W_C3_C3	1321

#define PLAYDOHop_LDS_Q_V1_V1	1322	/* Lop_LD_Q */
#define PLAYDOHop_LDS_Q_V1_C1	1323
#define PLAYDOHop_LDS_Q_V1_C2	1324
#define PLAYDOHop_LDS_Q_V1_C3	1325
#define PLAYDOHop_LDS_Q_C1_V1	1326
#define PLAYDOHop_LDS_Q_C1_C1	1327
#define PLAYDOHop_LDS_Q_C1_C2	1328
#define PLAYDOHop_LDS_Q_C1_C3	1329
#define PLAYDOHop_LDS_Q_C2_V1	1330
#define PLAYDOHop_LDS_Q_C2_C1	1331
#define PLAYDOHop_LDS_Q_C2_C2	1332
#define PLAYDOHop_LDS_Q_C2_C3	1333
#define PLAYDOHop_LDS_Q_C3_V1	1334
#define PLAYDOHop_LDS_Q_C3_C1	1335
#define PLAYDOHop_LDS_Q_C3_C2	1336
#define PLAYDOHop_LDS_Q_C3_C3	1337

#define PLAYDOHop_LDSI_B_V1_V1	1338	/* Lop_LD_POST_UC */
#define PLAYDOHop_LDSI_B_V1_C1	1339
#define PLAYDOHop_LDSI_B_V1_C2	1340
#define PLAYDOHop_LDSI_B_V1_C3	1341
#define PLAYDOHop_LDSI_B_C1_V1	1342
#define PLAYDOHop_LDSI_B_C1_C1	1343
#define PLAYDOHop_LDSI_B_C1_C2	1344
#define PLAYDOHop_LDSI_B_C1_C3	1345
#define PLAYDOHop_LDSI_B_C2_V1	1346
#define PLAYDOHop_LDSI_B_C2_C1	1347
#define PLAYDOHop_LDSI_B_C2_C2	1348
#define PLAYDOHop_LDSI_B_C2_C3	1349
#define PLAYDOHop_LDSI_B_C3_V1	1350
#define PLAYDOHop_LDSI_B_C3_C1	1351
#define PLAYDOHop_LDSI_B_C3_C2	1352
#define PLAYDOHop_LDSI_B_C3_C3	1353

#define PLAYDOHop_LDSI_H_V1_V1	1354	/* Lop_LD_POST_UC2 */
#define PLAYDOHop_LDSI_H_V1_C1	1355
#define PLAYDOHop_LDSI_H_V1_C2	1356
#define PLAYDOHop_LDSI_H_V1_C3	1357
#define PLAYDOHop_LDSI_H_C1_V1	1358
#define PLAYDOHop_LDSI_H_C1_C1	1359
#define PLAYDOHop_LDSI_H_C1_C2	1360
#define PLAYDOHop_LDSI_H_C1_C3	1361
#define PLAYDOHop_LDSI_H_C2_V1	1362
#define PLAYDOHop_LDSI_H_C2_C1	1363
#define PLAYDOHop_LDSI_H_C2_C2	1364
#define PLAYDOHop_LDSI_H_C2_C3	1365
#define PLAYDOHop_LDSI_H_C3_V1	1366
#define PLAYDOHop_LDSI_H_C3_C1	1367
#define PLAYDOHop_LDSI_H_C3_C2	1368
#define PLAYDOHop_LDSI_H_C3_C3	1369

#define PLAYDOHop_LDSI_W_V1_V1	1370	/* Lop_LD_POST_I */
#define PLAYDOHop_LDSI_W_V1_C1	1371
#define PLAYDOHop_LDSI_W_V1_C2	1372
#define PLAYDOHop_LDSI_W_V1_C3	1373
#define PLAYDOHop_LDSI_W_C1_V1	1374
#define PLAYDOHop_LDSI_W_C1_C1	1375
#define PLAYDOHop_LDSI_W_C1_C2	1376
#define PLAYDOHop_LDSI_W_C1_C3	1377
#define PLAYDOHop_LDSI_W_C2_V1	1378
#define PLAYDOHop_LDSI_W_C2_C1	1379
#define PLAYDOHop_LDSI_W_C2_C2	1380
#define PLAYDOHop_LDSI_W_C2_C3	1381
#define PLAYDOHop_LDSI_W_C3_V1	1382
#define PLAYDOHop_LDSI_W_C3_C1	1383
#define PLAYDOHop_LDSI_W_C3_C2	1384
#define PLAYDOHop_LDSI_W_C3_C3	1385

#define PLAYDOHop_LDSI_Q_V1_V1	1386	/* Lop_LD_POST_Q */
#define PLAYDOHop_LDSI_Q_V1_C1	1387
#define PLAYDOHop_LDSI_Q_V1_C2	1388
#define PLAYDOHop_LDSI_Q_V1_C3	1389
#define PLAYDOHop_LDSI_Q_C1_V1	1390
#define PLAYDOHop_LDSI_Q_C1_C1	1391
#define PLAYDOHop_LDSI_Q_C1_C2	1392
#define PLAYDOHop_LDSI_Q_C1_C3	1393
#define PLAYDOHop_LDSI_Q_C2_V1	1394
#define PLAYDOHop_LDSI_Q_C2_C1	1395
#define PLAYDOHop_LDSI_Q_C2_C2	1396
#define PLAYDOHop_LDSI_Q_C2_C3	1397
#define PLAYDOHop_LDSI_Q_C3_V1	1398
#define PLAYDOHop_LDSI_Q_C3_C1	1399
#define PLAYDOHop_LDSI_Q_C3_C2	1400
#define PLAYDOHop_LDSI_Q_C3_C3	1401

#define PLAYDOHop_FLDS_S_V1_V1	1402	/* Lop_LD_F */
#define PLAYDOHop_FLDS_S_V1_C1	1403
#define PLAYDOHop_FLDS_S_V1_C2	1404
#define PLAYDOHop_FLDS_S_V1_C3	1405
#define PLAYDOHop_FLDS_S_C1_V1	1406
#define PLAYDOHop_FLDS_S_C1_C1	1407
#define PLAYDOHop_FLDS_S_C1_C2	1408
#define PLAYDOHop_FLDS_S_C1_C3	1409
#define PLAYDOHop_FLDS_S_C2_V1	1410
#define PLAYDOHop_FLDS_S_C2_C1	1411
#define PLAYDOHop_FLDS_S_C2_C2	1412
#define PLAYDOHop_FLDS_S_C2_C3	1413
#define PLAYDOHop_FLDS_S_C3_V1	1414
#define PLAYDOHop_FLDS_S_C3_C1	1415
#define PLAYDOHop_FLDS_S_C3_C2	1416
#define PLAYDOHop_FLDS_S_C3_C3	1417

#define PLAYDOHop_FLDS_D_V1_V1	1418	/* Lop_LD_F2 */
#define PLAYDOHop_FLDS_D_V1_C1	1419
#define PLAYDOHop_FLDS_D_V1_C2	1420
#define PLAYDOHop_FLDS_D_V1_C3	1421
#define PLAYDOHop_FLDS_D_C1_V1	1422
#define PLAYDOHop_FLDS_D_C1_C1	1423
#define PLAYDOHop_FLDS_D_C1_C2	1424
#define PLAYDOHop_FLDS_D_C1_C3	1425
#define PLAYDOHop_FLDS_D_C2_V1	1426
#define PLAYDOHop_FLDS_D_C2_C1	1427
#define PLAYDOHop_FLDS_D_C2_C2	1428
#define PLAYDOHop_FLDS_D_C2_C3	1429
#define PLAYDOHop_FLDS_D_C3_V1	1430
#define PLAYDOHop_FLDS_D_C3_C1	1431
#define PLAYDOHop_FLDS_D_C3_C2	1432
#define PLAYDOHop_FLDS_D_C3_C3	1433

#define PLAYDOHop_FLDSI_S_V1_V1	1434	/* Lop_LD_POST_F */
#define PLAYDOHop_FLDSI_S_V1_C1	1435
#define PLAYDOHop_FLDSI_S_V1_C2	1436
#define PLAYDOHop_FLDSI_S_V1_C3	1437
#define PLAYDOHop_FLDSI_S_C1_V1	1438
#define PLAYDOHop_FLDSI_S_C1_C1	1439
#define PLAYDOHop_FLDSI_S_C1_C2	1440
#define PLAYDOHop_FLDSI_S_C1_C3	1441
#define PLAYDOHop_FLDSI_S_C2_V1	1442
#define PLAYDOHop_FLDSI_S_C2_C1	1443
#define PLAYDOHop_FLDSI_S_C2_C2	1444
#define PLAYDOHop_FLDSI_S_C2_C3	1445
#define PLAYDOHop_FLDSI_S_C3_V1	1446
#define PLAYDOHop_FLDSI_S_C3_C1	1447
#define PLAYDOHop_FLDSI_S_C3_C2	1448
#define PLAYDOHop_FLDSI_S_C3_C3	1449

#define PLAYDOHop_FLDSI_D_V1_V1	1450	/* Lop_LD_POST_F2 */
#define PLAYDOHop_FLDSI_D_V1_C1	1451
#define PLAYDOHop_FLDSI_D_V1_C2	1452
#define PLAYDOHop_FLDSI_D_V1_C3	1453
#define PLAYDOHop_FLDSI_D_C1_V1	1454
#define PLAYDOHop_FLDSI_D_C1_C1	1455
#define PLAYDOHop_FLDSI_D_C1_C2	1456
#define PLAYDOHop_FLDSI_D_C1_C3	1457
#define PLAYDOHop_FLDSI_D_C2_V1	1458
#define PLAYDOHop_FLDSI_D_C2_C1	1459
#define PLAYDOHop_FLDSI_D_C2_C2	1460
#define PLAYDOHop_FLDSI_D_C2_C3	1461
#define PLAYDOHop_FLDSI_D_C3_V1	1462
#define PLAYDOHop_FLDSI_D_C3_C1	1463
#define PLAYDOHop_FLDSI_D_C3_C2	1464
#define PLAYDOHop_FLDSI_D_C3_C3	1465

/* data verify loads */
#define PLAYDOHop_LDV_B		1466	/* not handled currently */
#define PLAYDOHop_LDV_H		1467	/* not handled currently */
#define PLAYDOHop_LDV_W		1468	/* not handled currently */
#define PLAYDOHop_LDV_Q		1469	/* not handled currently */
#define PLAYDOHop_FLDV_S	1470	/* not handled currently */
#define PLAYDOHop_FLDV_D	1471	/* not handled currently */

/*
 * PlayDoh int ALU op extensions
 */
#define PLAYDOHop_SH1ADDL	1472	/* Lop_LSL */
#define PLAYDOHop_SH2ADDL	1473	/* Lop_LSL */
#define PLAYDOHop_SH3ADDL	1474	/* Lop_LSL */

/*
 * PlayDoh FP ALU op extensions
 */
#define PLAYDOHop_FMPYADDN_S	1475	/* Lop_MUL_ADD_F */
#define PLAYDOHop_FMPYADDN_D	1476	/* Lop_MUL_ADD_F2 */

/*
 * Playdoh int comparison extensions
 */
#define PLAYDOHop_CMPR_FALSE	1477	/* Lop_NE (0,0) */
#define PLAYDOHop_CMPR_TRUE	1478	/* Lop_EQ (0,0) */
#define PLAYDOHop_CMPR_OD	1479	/* Lop_EQ */
#define PLAYDOHop_CMPR_EV	1480	/* Lop_EQ */
#define PLAYDOHop_CMPR_SV	1481	/* not supported */
#define PLAYDOHop_CMPR_NSV	1482	/* not supported */

/*
 * Playdoh FP comparison extensions - Note unordered compares ala
 * PA-RISC are not supported by the playdoh code generator because
 * they are silly and pretty much only useful for assembly language
 * programmers.
 */
#define PLAYDOHop_FCMPR_S_FALSE	1483	/* Lop_NE_F (0,0) */
#define PLAYDOHop_FCMPR_S_TRUE	1484	/* Lop_EQ_F (0,0) */
#define PLAYDOHop_FCMPR_D_FALSE	1485	/* Lop_NE_F2 (0,0) */
#define PLAYDOHop_FCMPR_D_TRUE	1486	/* Lop_EQ_F2 (0,0) */

/*
 *	Multi-cycle no-op.  Added SAM 10-96
 */
#define PLAYDOHop_M_NO_OP	1487	/* Lop_NO_OP */

/*
 *      Playdoh literal forming operations
 */
#define PLAYDOHop_MOVELB      1488
#define PLAYDOHop_MOVELBX     1489
#define PLAYDOHop_MOVELBS     1490
#define PLAYDOHop_PBRRL       1491
#define PLAYDOHop_PBRAL       1492
#define PLAYDOHop_PBRRLBS     1493
#define PLAYDOHop_PBRALBS     1494
#define PLAYDOHop_MOVELG      1495
#define PLAYDOHop_MOVELGX     1496
#define PLAYDOHop_MOVELGS     1497
#define PLAYDOHop_MOVELF      1498
#define PLAYDOHop_MOVELFS     1499

/*
 *      Playdoh restricted io format moves
 */
#define PLAYDOHop_MOVEGC      1500
#define PLAYDOHop_MOVECG      1501
#define PLAYDOHop_MOVEGG      1502
#define PLAYDOHop_MOVEBB      1503

/*
 *      Regalloc operations
 */
#define PLAYDOHop_SAVE        1504
#define PLAYDOHop_RESTORE     1505
#define PLAYDOHop_FSAVE_S     1506
#define PLAYDOHop_FRESTORE_S  1507
#define PLAYDOHop_FSAVE_D     1508
#define PLAYDOHop_FRESTORE_D  1509
#define PLAYDOHop_BSAVE       1510
#define PLAYDOHop_BRESTORE    1511
#define PLAYDOHop_PSAVE       1512
#define PLAYDOHop_PRESTORE    1513
#define PLAYDOHop_SAVEG       1514
#define PLAYDOHop_RESTOREG    1515
#define PLAYDOHop_FSAVEG_S    1516
#define PLAYDOHop_FRESTOREG_S 1517
#define PLAYDOHop_FSAVEG_D    1518
#define PLAYDOHop_FRESTOREG_D 1519
#define PLAYDOHop_BSAVEG      1520
#define PLAYDOHop_BRESTOREG   1521
#define PLAYDOHop_PSAVEG      1522
#define PLAYDOHop_PRESTOREG   1523
#define PLAYDOHop_MOVEGBP     1524
#define PLAYDOHop_MOVEGCM     1525

/*
 *      Local memory loads and stores
 *              Currently set up for 4 local memories!
 */
#define PLAYDOHop_LL_B_L1     1526
#define PLAYDOHop_LL_B_L2     1527
#define PLAYDOHop_LL_B_L3     1528
#define PLAYDOHop_LL_B_L4     1529

#define PLAYDOHop_LL_H_L1     1530
#define PLAYDOHop_LL_H_L2     1531
#define PLAYDOHop_LL_H_L3     1532
#define PLAYDOHop_LL_H_L4     1533

#define PLAYDOHop_LL_W_L1     1534
#define PLAYDOHop_LL_W_L2     1535
#define PLAYDOHop_LL_W_L3     1536
#define PLAYDOHop_LL_W_L4     1537

#define PLAYDOHop_LL_Q_L1     1538
#define PLAYDOHop_LL_Q_L2     1539
#define PLAYDOHop_LL_Q_L3     1540
#define PLAYDOHop_LL_Q_L4     1541

#define PLAYDOHop_LLX_B_L1    1542
#define PLAYDOHop_LLX_B_L2    1543
#define PLAYDOHop_LLX_B_L3    1544
#define PLAYDOHop_LLX_B_L4    1545

#define PLAYDOHop_LLX_H_L1    1546
#define PLAYDOHop_LLX_H_L2    1547
#define PLAYDOHop_LLX_H_L3    1548
#define PLAYDOHop_LLX_H_L4    1549

#define PLAYDOHop_LLX_W_L1    1550
#define PLAYDOHop_LLX_W_L2    1551
#define PLAYDOHop_LLX_W_L3    1552
#define PLAYDOHop_LLX_W_L4    1553

#define PLAYDOHop_LLX_Q_L1    1554
#define PLAYDOHop_LLX_Q_L2    1555
#define PLAYDOHop_LLX_Q_L3    1556
#define PLAYDOHop_LLX_Q_L4    1557

#define PLAYDOHop_FLL_S_L1    1558
#define PLAYDOHop_FLL_S_L2    1559
#define PLAYDOHop_FLL_S_L3    1560
#define PLAYDOHop_FLL_S_L4    1561

#define PLAYDOHop_FLL_D_L1    1562
#define PLAYDOHop_FLL_D_L2    1563
#define PLAYDOHop_FLL_D_L3    1564
#define PLAYDOHop_FLL_D_L4    1565

#define PLAYDOHop_SL_B_L1     1566
#define PLAYDOHop_SL_B_L2     1567
#define PLAYDOHop_SL_B_L3     1568
#define PLAYDOHop_SL_B_L4     1569

#define PLAYDOHop_SL_H_L1     1570
#define PLAYDOHop_SL_H_L2     1571
#define PLAYDOHop_SL_H_L3     1572
#define PLAYDOHop_SL_H_L4     1573

#define PLAYDOHop_SL_W_L1     1574
#define PLAYDOHop_SL_W_L2     1575
#define PLAYDOHop_SL_W_L3     1576
#define PLAYDOHop_SL_W_L4     1577

#define PLAYDOHop_SL_Q_L1     1578
#define PLAYDOHop_SL_Q_L2     1579
#define PLAYDOHop_SL_Q_L3     1580
#define PLAYDOHop_SL_Q_L4     1581

#define PLAYDOHop_FSL_S_L1    1582
#define PLAYDOHop_FSL_S_L2    1583
#define PLAYDOHop_FSL_S_L3    1584
#define PLAYDOHop_FSL_S_L4    1585

#define PLAYDOHop_FSL_D_L1    1586
#define PLAYDOHop_FSL_D_L2    1587
#define PLAYDOHop_FSL_D_L3    1588
#define PLAYDOHop_FSL_D_L4    1589
#define PLAYDOHop_MOVEPP      1590

/*new load short operation LG (tangw 05-13-02)*/
#define PLAYDOHop_LG_B_V1_V1     1591    
#define PLAYDOHop_LG_B_V1_C1     1592
#define PLAYDOHop_LG_B_V1_C2     1593
#define PLAYDOHop_LG_B_V1_C3     1594
#define PLAYDOHop_LG_B_C1_V1     1595
#define PLAYDOHop_LG_B_C1_C1     1596
#define PLAYDOHop_LG_B_C1_C2     1597
#define PLAYDOHop_LG_B_C1_C3     1598
#define PLAYDOHop_LG_B_C2_V1     1599
#define PLAYDOHop_LG_B_C2_C1     1600
#define PLAYDOHop_LG_B_C2_C2     1601
#define PLAYDOHop_LG_B_C2_C3     1602
#define PLAYDOHop_LG_B_C3_V1     1603
#define PLAYDOHop_LG_B_C3_C1     1604
#define PLAYDOHop_LG_B_C3_C2     1605
#define PLAYDOHop_LG_B_C3_C3     1606

#define PLAYDOHop_LG_H_V1_V1     1607    
#define PLAYDOHop_LG_H_V1_C1     1608
#define PLAYDOHop_LG_H_V1_C2     1609
#define PLAYDOHop_LG_H_V1_C3     1610
#define PLAYDOHop_LG_H_C1_V1     1611
#define PLAYDOHop_LG_H_C1_C1     1612
#define PLAYDOHop_LG_H_C1_C2     1613
#define PLAYDOHop_LG_H_C1_C3     1614
#define PLAYDOHop_LG_H_C2_V1     1615
#define PLAYDOHop_LG_H_C2_C1     1616
#define PLAYDOHop_LG_H_C2_C2     1617
#define PLAYDOHop_LG_H_C2_C3     1618
#define PLAYDOHop_LG_H_C3_V1     1619
#define PLAYDOHop_LG_H_C3_C1     1620
#define PLAYDOHop_LG_H_C3_C2     1621
#define PLAYDOHop_LG_H_C3_C3     1622

#define PLAYDOHop_LG_W_V1_V1     1623    
#define PLAYDOHop_LG_W_V1_C1     1624
#define PLAYDOHop_LG_W_V1_C2     1625
#define PLAYDOHop_LG_W_V1_C3     1626
#define PLAYDOHop_LG_W_C1_V1     1627
#define PLAYDOHop_LG_W_C1_C1     1628
#define PLAYDOHop_LG_W_C1_C2     1629
#define PLAYDOHop_LG_W_C1_C3     1630
#define PLAYDOHop_LG_W_C2_V1     1631
#define PLAYDOHop_LG_W_C2_C1     1632
#define PLAYDOHop_LG_W_C2_C2     1633
#define PLAYDOHop_LG_W_C2_C3     1634
#define PLAYDOHop_LG_W_C3_V1     1635
#define PLAYDOHop_LG_W_C3_C1     1636
#define PLAYDOHop_LG_W_C3_C2     1637
#define PLAYDOHop_LG_W_C3_C3     1638

#define PLAYDOHop_LG_Q_V1_V1     1639    
#define PLAYDOHop_LG_Q_V1_C1     1640
#define PLAYDOHop_LG_Q_V1_C2     1641
#define PLAYDOHop_LG_Q_V1_C3     1642
#define PLAYDOHop_LG_Q_C1_V1     1643
#define PLAYDOHop_LG_Q_C1_C1     1644
#define PLAYDOHop_LG_Q_C1_C2     1645
#define PLAYDOHop_LG_Q_C1_C3     1646
#define PLAYDOHop_LG_Q_C2_V1     1647
#define PLAYDOHop_LG_Q_C2_C1     1648
#define PLAYDOHop_LG_Q_C2_C2     1649
#define PLAYDOHop_LG_Q_C2_C3     1650
#define PLAYDOHop_LG_Q_C3_V1     1651
#define PLAYDOHop_LG_Q_C3_C1     1652
#define PLAYDOHop_LG_Q_C3_C2     1653
#define PLAYDOHop_LG_Q_C3_C3     1654

/*new load medium operations LM (tangw 05-13-02)*/
#define PLAYDOHop_LM_B_V1_V1     1655    
#define PLAYDOHop_LM_B_V1_C1     1656
#define PLAYDOHop_LM_B_V1_C2     1657
#define PLAYDOHop_LM_B_V1_C3     1658
#define PLAYDOHop_LM_B_C1_V1     1659
#define PLAYDOHop_LM_B_C1_C1     1660
#define PLAYDOHop_LM_B_C1_C2     1661
#define PLAYDOHop_LM_B_C1_C3     1662
#define PLAYDOHop_LM_B_C2_V1     1663
#define PLAYDOHop_LM_B_C2_C1     1664
#define PLAYDOHop_LM_B_C2_C2     1665
#define PLAYDOHop_LM_B_C2_C3     1666
#define PLAYDOHop_LM_B_C3_V1     1667
#define PLAYDOHop_LM_B_C3_C1     1668
#define PLAYDOHop_LM_B_C3_C2     1669
#define PLAYDOHop_LM_B_C3_C3     1670

#define PLAYDOHop_LM_H_V1_V1     1671    
#define PLAYDOHop_LM_H_V1_C1     1672
#define PLAYDOHop_LM_H_V1_C2     1673
#define PLAYDOHop_LM_H_V1_C3     1674
#define PLAYDOHop_LM_H_C1_V1     1675
#define PLAYDOHop_LM_H_C1_C1     1676
#define PLAYDOHop_LM_H_C1_C2     1677
#define PLAYDOHop_LM_H_C1_C3     1678
#define PLAYDOHop_LM_H_C2_V1     1679
#define PLAYDOHop_LM_H_C2_C1     1680
#define PLAYDOHop_LM_H_C2_C2     1681
#define PLAYDOHop_LM_H_C2_C3     1682
#define PLAYDOHop_LM_H_C3_V1     1683
#define PLAYDOHop_LM_H_C3_C1     1684
#define PLAYDOHop_LM_H_C3_C2     1685
#define PLAYDOHop_LM_H_C3_C3     1686

#define PLAYDOHop_LM_W_V1_V1     1687    
#define PLAYDOHop_LM_W_V1_C1     1688
#define PLAYDOHop_LM_W_V1_C2     1689
#define PLAYDOHop_LM_W_V1_C3     1690
#define PLAYDOHop_LM_W_C1_V1     1691
#define PLAYDOHop_LM_W_C1_C1     1692
#define PLAYDOHop_LM_W_C1_C2     1693
#define PLAYDOHop_LM_W_C1_C3     1694
#define PLAYDOHop_LM_W_C2_V1     1695
#define PLAYDOHop_LM_W_C2_C1     1696
#define PLAYDOHop_LM_W_C2_C2     1697
#define PLAYDOHop_LM_W_C2_C3     1698
#define PLAYDOHop_LM_W_C3_V1     1699
#define PLAYDOHop_LM_W_C3_C1     1700
#define PLAYDOHop_LM_W_C3_C2     1701
#define PLAYDOHop_LM_W_C3_C3     1702

#define PLAYDOHop_LM_Q_V1_V1     1703    
#define PLAYDOHop_LM_Q_V1_C1     1704
#define PLAYDOHop_LM_Q_V1_C2     1705
#define PLAYDOHop_LM_Q_V1_C3     1706
#define PLAYDOHop_LM_Q_C1_V1     1707
#define PLAYDOHop_LM_Q_C1_C1     1708
#define PLAYDOHop_LM_Q_C1_C2     1709
#define PLAYDOHop_LM_Q_C1_C3     1710
#define PLAYDOHop_LM_Q_C2_V1     1711
#define PLAYDOHop_LM_Q_C2_C1     1712
#define PLAYDOHop_LM_Q_C2_C2     1713
#define PLAYDOHop_LM_Q_C2_C3     1714
#define PLAYDOHop_LM_Q_C3_V1     1715
#define PLAYDOHop_LM_Q_C3_C1     1716
#define PLAYDOHop_LM_Q_C3_C2     1717
#define PLAYDOHop_LM_Q_C3_C3     1718

#define PLAYDOHop_LX_B_V1_V1     1719    /* L w/ sign extension (tangw 05-10-02) */
#define PLAYDOHop_LX_B_V1_C1     1720
#define PLAYDOHop_LX_B_V1_C2     1721
#define PLAYDOHop_LX_B_V1_C3     1722
#define PLAYDOHop_LX_B_C1_V1     1723
#define PLAYDOHop_LX_B_C1_C1     1724
#define PLAYDOHop_LX_B_C1_C2     1725
#define PLAYDOHop_LX_B_C1_C3     1726
#define PLAYDOHop_LX_B_C2_V1     1727
#define PLAYDOHop_LX_B_C2_C1     1728
#define PLAYDOHop_LX_B_C2_C2     1729
#define PLAYDOHop_LX_B_C2_C3     1730
#define PLAYDOHop_LX_B_C3_V1     1731
#define PLAYDOHop_LX_B_C3_C1     1732
#define PLAYDOHop_LX_B_C3_C2     1733
#define PLAYDOHop_LX_B_C3_C3     1734

#define PLAYDOHop_LX_H_V1_V1     1735    
#define PLAYDOHop_LX_H_V1_C1     1736
#define PLAYDOHop_LX_H_V1_C2     1737
#define PLAYDOHop_LX_H_V1_C3     1738
#define PLAYDOHop_LX_H_C1_V1     1739
#define PLAYDOHop_LX_H_C1_C1     1740
#define PLAYDOHop_LX_H_C1_C2     1741
#define PLAYDOHop_LX_H_C1_C3     1742
#define PLAYDOHop_LX_H_C2_V1     1743
#define PLAYDOHop_LX_H_C2_C1     1744
#define PLAYDOHop_LX_H_C2_C2     1745
#define PLAYDOHop_LX_H_C2_C3     1746
#define PLAYDOHop_LX_H_C3_V1     1747
#define PLAYDOHop_LX_H_C3_C1     1748
#define PLAYDOHop_LX_H_C3_C2     1749
#define PLAYDOHop_LX_H_C3_C3     1750

#define PLAYDOHop_LX_W_V1_V1     1751    
#define PLAYDOHop_LX_W_V1_C1     1752
#define PLAYDOHop_LX_W_V1_C2     1753
#define PLAYDOHop_LX_W_V1_C3     1754
#define PLAYDOHop_LX_W_C1_V1     1755
#define PLAYDOHop_LX_W_C1_C1     1756
#define PLAYDOHop_LX_W_C1_C2     1757
#define PLAYDOHop_LX_W_C1_C3     1758
#define PLAYDOHop_LX_W_C2_V1     1759
#define PLAYDOHop_LX_W_C2_C1     1760
#define PLAYDOHop_LX_W_C2_C2     1761
#define PLAYDOHop_LX_W_C2_C3     1762
#define PLAYDOHop_LX_W_C3_V1     1763
#define PLAYDOHop_LX_W_C3_C1     1764
#define PLAYDOHop_LX_W_C3_C2     1765
#define PLAYDOHop_LX_W_C3_C3     1766

#define PLAYDOHop_LX_Q_V1_V1     1767    
#define PLAYDOHop_LX_Q_V1_C1     1768
#define PLAYDOHop_LX_Q_V1_C2     1769
#define PLAYDOHop_LX_Q_V1_C3     1770
#define PLAYDOHop_LX_Q_C1_V1     1771
#define PLAYDOHop_LX_Q_C1_C1     1772
#define PLAYDOHop_LX_Q_C1_C2     1773
#define PLAYDOHop_LX_Q_C1_C3     1774
#define PLAYDOHop_LX_Q_C2_V1     1775
#define PLAYDOHop_LX_Q_C2_C1     1776
#define PLAYDOHop_LX_Q_C2_C2     1777
#define PLAYDOHop_LX_Q_C2_C3     1778
#define PLAYDOHop_LX_Q_C3_V1     1779
#define PLAYDOHop_LX_Q_C3_C1     1780
#define PLAYDOHop_LX_Q_C3_C2     1781
#define PLAYDOHop_LX_Q_C3_C3     1782

#define PLAYDOHop_LGX_B_V1_V1     1783  /* LG w/ sign-extension (tangw 05-10-02)*/
#define PLAYDOHop_LGX_B_V1_C1     1784
#define PLAYDOHop_LGX_B_V1_C2     1785
#define PLAYDOHop_LGX_B_V1_C3     1786
#define PLAYDOHop_LGX_B_C1_V1     1787
#define PLAYDOHop_LGX_B_C1_C1     1788
#define PLAYDOHop_LGX_B_C1_C2     1789
#define PLAYDOHop_LGX_B_C1_C3     1790
#define PLAYDOHop_LGX_B_C2_V1     1791
#define PLAYDOHop_LGX_B_C2_C1     1792
#define PLAYDOHop_LGX_B_C2_C2     1793
#define PLAYDOHop_LGX_B_C2_C3     1794
#define PLAYDOHop_LGX_B_C3_V1     1795
#define PLAYDOHop_LGX_B_C3_C1     1796
#define PLAYDOHop_LGX_B_C3_C2     1797
#define PLAYDOHop_LGX_B_C3_C3     1798

#define PLAYDOHop_LGX_H_V1_V1     1799   
#define PLAYDOHop_LGX_H_V1_C1     1800
#define PLAYDOHop_LGX_H_V1_C2     1801
#define PLAYDOHop_LGX_H_V1_C3     1802
#define PLAYDOHop_LGX_H_C1_V1     1803
#define PLAYDOHop_LGX_H_C1_C1     1804
#define PLAYDOHop_LGX_H_C1_C2     1805
#define PLAYDOHop_LGX_H_C1_C3     1806
#define PLAYDOHop_LGX_H_C2_V1     1807
#define PLAYDOHop_LGX_H_C2_C1     1808
#define PLAYDOHop_LGX_H_C2_C2     1809
#define PLAYDOHop_LGX_H_C2_C3     1810
#define PLAYDOHop_LGX_H_C3_V1     1811
#define PLAYDOHop_LGX_H_C3_C1     1812
#define PLAYDOHop_LGX_H_C3_C2     1813
#define PLAYDOHop_LGX_H_C3_C3     1814

#define PLAYDOHop_LGX_W_V1_V1     1815
#define PLAYDOHop_LGX_W_V1_C1     1816
#define PLAYDOHop_LGX_W_V1_C2     1817
#define PLAYDOHop_LGX_W_V1_C3     1818
#define PLAYDOHop_LGX_W_C1_V1     1819
#define PLAYDOHop_LGX_W_C1_C1     1820
#define PLAYDOHop_LGX_W_C1_C2     1821
#define PLAYDOHop_LGX_W_C1_C3     1822
#define PLAYDOHop_LGX_W_C2_V1     1823
#define PLAYDOHop_LGX_W_C2_C1     1824
#define PLAYDOHop_LGX_W_C2_C2     1825
#define PLAYDOHop_LGX_W_C2_C3     1826
#define PLAYDOHop_LGX_W_C3_V1     1827
#define PLAYDOHop_LGX_W_C3_C1     1828
#define PLAYDOHop_LGX_W_C3_C2     1829
#define PLAYDOHop_LGX_W_C3_C3     1830

#define PLAYDOHop_LGX_Q_V1_V1     1831
#define PLAYDOHop_LGX_Q_V1_C1     1832
#define PLAYDOHop_LGX_Q_V1_C2     1833
#define PLAYDOHop_LGX_Q_V1_C3     1834
#define PLAYDOHop_LGX_Q_C1_V1     1835
#define PLAYDOHop_LGX_Q_C1_C1     1836
#define PLAYDOHop_LGX_Q_C1_C2     1837
#define PLAYDOHop_LGX_Q_C1_C3     1838
#define PLAYDOHop_LGX_Q_C2_V1     1839
#define PLAYDOHop_LGX_Q_C2_C1     1840
#define PLAYDOHop_LGX_Q_C2_C2     1841
#define PLAYDOHop_LGX_Q_C2_C3     1842
#define PLAYDOHop_LGX_Q_C3_V1     1843
#define PLAYDOHop_LGX_Q_C3_C1     1844
#define PLAYDOHop_LGX_Q_C3_C2     1845
#define PLAYDOHop_LGX_Q_C3_C3     1846

#define PLAYDOHop_LMX_B_V1_V1     1847       /*LM w/ sign_extension (tangw 05-10-02)*/
#define PLAYDOHop_LMX_B_V1_C1     1848
#define PLAYDOHop_LMX_B_V1_C2     1849
#define PLAYDOHop_LMX_B_V1_C3     1850
#define PLAYDOHop_LMX_B_C1_V1     1851
#define PLAYDOHop_LMX_B_C1_C1     1852
#define PLAYDOHop_LMX_B_C1_C2     1853
#define PLAYDOHop_LMX_B_C1_C3     1854
#define PLAYDOHop_LMX_B_C2_V1     1855
#define PLAYDOHop_LMX_B_C2_C1     1856
#define PLAYDOHop_LMX_B_C2_C2     1857
#define PLAYDOHop_LMX_B_C2_C3     1858
#define PLAYDOHop_LMX_B_C3_V1     1859
#define PLAYDOHop_LMX_B_C3_C1     1860
#define PLAYDOHop_LMX_B_C3_C2     1861
#define PLAYDOHop_LMX_B_C3_C3     1862

#define PLAYDOHop_LMX_H_V1_V1     1863
#define PLAYDOHop_LMX_H_V1_C1     1864
#define PLAYDOHop_LMX_H_V1_C2     1865
#define PLAYDOHop_LMX_H_V1_C3     1866
#define PLAYDOHop_LMX_H_C1_V1     1867
#define PLAYDOHop_LMX_H_C1_C1     1868
#define PLAYDOHop_LMX_H_C1_C2     1869
#define PLAYDOHop_LMX_H_C1_C3     1870
#define PLAYDOHop_LMX_H_C2_V1     1871
#define PLAYDOHop_LMX_H_C2_C1     1872
#define PLAYDOHop_LMX_H_C2_C2     1873
#define PLAYDOHop_LMX_H_C2_C3     1874
#define PLAYDOHop_LMX_H_C3_V1     1875
#define PLAYDOHop_LMX_H_C3_C1     1876
#define PLAYDOHop_LMX_H_C3_C2     1877
#define PLAYDOHop_LMX_H_C3_C3     1878

#define PLAYDOHop_LMX_W_V1_V1     1879
#define PLAYDOHop_LMX_W_V1_C1     1880
#define PLAYDOHop_LMX_W_V1_C2     1881
#define PLAYDOHop_LMX_W_V1_C3     1882
#define PLAYDOHop_LMX_W_C1_V1     1883
#define PLAYDOHop_LMX_W_C1_C1     1884
#define PLAYDOHop_LMX_W_C1_C2     1885
#define PLAYDOHop_LMX_W_C1_C3     1886
#define PLAYDOHop_LMX_W_C2_V1     1887
#define PLAYDOHop_LMX_W_C2_C1     1888
#define PLAYDOHop_LMX_W_C2_C2     1889
#define PLAYDOHop_LMX_W_C2_C3     1890
#define PLAYDOHop_LMX_W_C3_V1     1891
#define PLAYDOHop_LMX_W_C3_C1     1892
#define PLAYDOHop_LMX_W_C3_C2     1893
#define PLAYDOHop_LMX_W_C3_C3     1894

#define PLAYDOHop_LMX_Q_V1_V1     1895
#define PLAYDOHop_LMX_Q_V1_C1     1896
#define PLAYDOHop_LMX_Q_V1_C2     1897
#define PLAYDOHop_LMX_Q_V1_C3     1898
#define PLAYDOHop_LMX_Q_C1_V1     1899
#define PLAYDOHop_LMX_Q_C1_C1     1900
#define PLAYDOHop_LMX_Q_C1_C2     1901
#define PLAYDOHop_LMX_Q_C1_C3     1902
#define PLAYDOHop_LMX_Q_C2_V1     1903
#define PLAYDOHop_LMX_Q_C2_C1     1904
#define PLAYDOHop_LMX_Q_C2_C2     1905
#define PLAYDOHop_LMX_Q_C2_C3     1906
#define PLAYDOHop_LMX_Q_C3_V1     1907
#define PLAYDOHop_LMX_Q_C3_C1     1908
#define PLAYDOHop_LMX_Q_C3_C2     1909
#define PLAYDOHop_LMX_Q_C3_C3     1910

/*Floating point load short operation: FLG (tangw 05-15-02)*/
#define PLAYDOHop_FLG_S_V1_V1	1911	
#define PLAYDOHop_FLG_S_V1_C1	1912
#define PLAYDOHop_FLG_S_V1_C2	1913
#define PLAYDOHop_FLG_S_V1_C3	1914
#define PLAYDOHop_FLG_S_C1_V1	1915
#define PLAYDOHop_FLG_S_C1_C1	1916
#define PLAYDOHop_FLG_S_C1_C2	1917
#define PLAYDOHop_FLG_S_C1_C3	1918
#define PLAYDOHop_FLG_S_C2_V1	1919
#define PLAYDOHop_FLG_S_C2_C1	1920
#define PLAYDOHop_FLG_S_C2_C2	1921
#define PLAYDOHop_FLG_S_C2_C3	1922
#define PLAYDOHop_FLG_S_C3_V1	1923
#define PLAYDOHop_FLG_S_C3_C1	1924
#define PLAYDOHop_FLG_S_C3_C2	1925
#define PLAYDOHop_FLG_S_C3_C3	1926

#define PLAYDOHop_FLG_D_V1_V1	1927	
#define PLAYDOHop_FLG_D_V1_C1	1928
#define PLAYDOHop_FLG_D_V1_C2	1929
#define PLAYDOHop_FLG_D_V1_C3	1930
#define PLAYDOHop_FLG_D_C1_V1	1931
#define PLAYDOHop_FLG_D_C1_C1	1932
#define PLAYDOHop_FLG_D_C1_C2	1933
#define PLAYDOHop_FLG_D_C1_C3	1934
#define PLAYDOHop_FLG_D_C2_V1	1935
#define PLAYDOHop_FLG_D_C2_C1	1936
#define PLAYDOHop_FLG_D_C2_C2	1937
#define PLAYDOHop_FLG_D_C2_C3	1938
#define PLAYDOHop_FLG_D_C3_V1	1939
#define PLAYDOHop_FLG_D_C3_C1	1940
#define PLAYDOHop_FLG_D_C3_C2	1941
#define PLAYDOHop_FLG_D_C3_C3	1942

/*Floating point load medium operation: FLM (tangw 05-15-02)*/
#define PLAYDOHop_FLM_S_V1_V1	1943	
#define PLAYDOHop_FLM_S_V1_C1	1944
#define PLAYDOHop_FLM_S_V1_C2	1945
#define PLAYDOHop_FLM_S_V1_C3	1946
#define PLAYDOHop_FLM_S_C1_V1	1947
#define PLAYDOHop_FLM_S_C1_C1	1948
#define PLAYDOHop_FLM_S_C1_C2	1949
#define PLAYDOHop_FLM_S_C1_C3	1950
#define PLAYDOHop_FLM_S_C2_V1	1951
#define PLAYDOHop_FLM_S_C2_C1	1952
#define PLAYDOHop_FLM_S_C2_C2	1953
#define PLAYDOHop_FLM_S_C2_C3	1954
#define PLAYDOHop_FLM_S_C3_V1	1955
#define PLAYDOHop_FLM_S_C3_C1	1956
#define PLAYDOHop_FLM_S_C3_C2	1957
#define PLAYDOHop_FLM_S_C3_C3	1958

#define PLAYDOHop_FLM_D_V1_V1	1959	
#define PLAYDOHop_FLM_D_V1_C1	1960
#define PLAYDOHop_FLM_D_V1_C2	1961
#define PLAYDOHop_FLM_D_V1_C3	1962
#define PLAYDOHop_FLM_D_C1_V1	1963
#define PLAYDOHop_FLM_D_C1_C1	1964
#define PLAYDOHop_FLM_D_C1_C2	1965
#define PLAYDOHop_FLM_D_C1_C3	1966
#define PLAYDOHop_FLM_D_C2_V1	1967
#define PLAYDOHop_FLM_D_C2_C1	1968
#define PLAYDOHop_FLM_D_C2_C2	1969
#define PLAYDOHop_FLM_D_C2_C3	1970
#define PLAYDOHop_FLM_D_C3_V1	1971
#define PLAYDOHop_FLM_D_C3_C1	1972
#define PLAYDOHop_FLM_D_C3_C2	1973
#define PLAYDOHop_FLM_D_C3_C3	1974

/*Integer store short operation: SG (tangw 05-15-02)*/
#define PLAYDOHop_SG_B_V1	1975	
#define PLAYDOHop_SG_B_C1	1976
#define PLAYDOHop_SG_B_C2	1977
#define PLAYDOHop_SG_B_C3	1978

#define PLAYDOHop_SG_H_V1	1979	
#define PLAYDOHop_SG_H_C1	1980
#define PLAYDOHop_SG_H_C2	1981
#define PLAYDOHop_SG_H_C3	1982

#define PLAYDOHop_SG_W_V1	1983	
#define PLAYDOHop_SG_W_C1	1984
#define PLAYDOHop_SG_W_C2	1985
#define PLAYDOHop_SG_W_C3	1986

#define PLAYDOHop_SG_Q_V1	1987	
#define PLAYDOHop_SG_Q_C1	1988
#define PLAYDOHop_SG_Q_C2	1989
#define PLAYDOHop_SG_Q_C3	1990

/*Integer store medium operation: SM (tangw 05-15-02)*/
#define PLAYDOHop_SM_B_V1	1991	
#define PLAYDOHop_SM_B_C1	1992
#define PLAYDOHop_SM_B_C2	1993
#define PLAYDOHop_SM_B_C3	1994

#define PLAYDOHop_SM_H_V1	1995	
#define PLAYDOHop_SM_H_C1	1996
#define PLAYDOHop_SM_H_C2	1997
#define PLAYDOHop_SM_H_C3	1998

#define PLAYDOHop_SM_W_V1	1999	
#define PLAYDOHop_SM_W_C1	2000
#define PLAYDOHop_SM_W_C2	2001
#define PLAYDOHop_SM_W_C3	2002

#define PLAYDOHop_SM_Q_V1	2003	
#define PLAYDOHop_SM_Q_C1	2004
#define PLAYDOHop_SM_Q_C2	2005
#define PLAYDOHop_SM_Q_C3	2006

/* Floating point store: FSM (tangw 05-21-02)*/
#define PLAYDOHop_FSM_S_V1	2007   
#define PLAYDOHop_FSM_S_C1	2008
#define PLAYDOHop_FSM_S_C2	2009
#define PLAYDOHop_FSM_S_C3	2010

#define PLAYDOHop_FSM_D_V1	2011 
#define PLAYDOHop_FSM_D_C1	2012
#define PLAYDOHop_FSM_D_C2	2013
#define PLAYDOHop_FSM_D_C3	2014

/*Floating point store: FSG (tangw 05-21-02)*/
#define PLAYDOHop_FSG_S_V1	2015	
#define PLAYDOHop_FSG_S_C1	2016
#define PLAYDOHop_FSG_S_C2	2017
#define PLAYDOHop_FSG_S_C3	2018

#define PLAYDOHop_FSG_D_V1	2019	
#define PLAYDOHop_FSG_D_C1	2020
#define PLAYDOHop_FSG_D_C2	2021
#define PLAYDOHop_FSG_D_C3	2022

/* SLARSEN: Vector extract ops */
#define PLAYDOHop_VEXTRSB       2023	/* Lop_VEXTRACT_C  */
#define PLAYDOHop_VEXTRSH       2024	/* Lop_VEXTRACT_C2 */

/* SLARSEN: Vector memory ops */
#define PLAYDOHop_VL_B_C1_C1    2025
#define PLAYDOHop_VL_H_C1_C1    2026
#define PLAYDOHop_VL_W_C1_C1    2027
#define PLAYDOHop_VFL_S_C1_C1   2028
#define PLAYDOHop_VFL_D_C1_C1   2029
#define PLAYDOHop_VS_B_C1       2030
#define PLAYDOHop_VS_H_C1       2031
#define PLAYDOHop_VS_W_C1       2032
#define PLAYDOHop_VFS_S_C1      2033
#define PLAYDOHop_VFS_D_C1      2034

#define PLAYDOHop_VLE_B_C1_C1   2035
#define PLAYDOHop_VLE_H_C1_C1   2036
#define PLAYDOHop_VLE_W_C1_C1   2037
#define PLAYDOHop_VFLE_S_C1_C1  2038
#define PLAYDOHop_VFLE_D_C1_C1  2039
#define PLAYDOHop_VSE_B_C1      2040
#define PLAYDOHop_VSE_H_C1      2041
#define PLAYDOHop_VSE_W_C1      2042
#define PLAYDOHop_VFSE_S_C1     2043
#define PLAYDOHop_VFSE_D_C1     2044

/**/

/*=========================================================================*/
/*
 *	Playdoh specific opcode names
 */
/*=========================================================================*/

/* prepare to branch ops */
#define PLAYDOHopcode_PBRR		"PLAYDOHop_PBRR"
#define PLAYDOHopcode_PBRA		"PLAYDOHop_PBRA"

//#if 0
/* sign extension ops */
#define PLAYDOHopcode_EXTRSB		"PLAYDOHop_EXTRSB"
#define PLAYDOHopcode_EXTRSH		"PLAYDOHop_EXTRSH"
//#endif

/*
 * PlayDoh move extensions
 */
#define PLAYDOHopcode_MOVEGF_L		"PLAYDOHop_MOVEGF_L"
#define PLAYDOHopcode_MOVEGF_U		"PLAYDOHop_MOVEGF_U"
#define PLAYDOHopcode_MOVEFG_L		"PLAYDOHop_MOVEFG_L"
#define PLAYDOHopcode_MOVEFG_U		"PLAYDOHop_MOVEFG_U"
#define PLAYDOHopcode_MOVEPG		"PLAYDOHop_MOVEPG"
#define PLAYDOHopcode_LDCM		"PLAYDOHop_LDCM"

/*
 * PlayDoh branch extensions
 */
#define PLAYDOHopcode_BRU		"PLAYDOHop_BRU"
#define PLAYDOHopcode_BRCT		"PLAYDOHop_BRCT"
#define PLAYDOHopcode_BRCF		"PLAYDOHop_BRCF"
#define PLAYDOHopcode_BRL		"PLAYDOHop_BRL"
#define PLAYDOHopcode_BRLC		"PLAYDOHop_BRLC"

/* brtop and wtop combinations */
#define PLAYDOHopcode_BRF_B_B_B		"PLAYDOHop_BRF_B_B_B"
#define PLAYDOHopcode_BRF_B_B_F		"PLAYDOHop_BRF_B_B_F"
#define PLAYDOHopcode_BRF_B_F_B		"PLAYDOHop_BRF_B_F_B"
#define PLAYDOHopcode_BRF_B_F_F		"PLAYDOHop_BRF_B_F_F"
#define PLAYDOHopcode_BRF_F_B_B		"PLAYDOHop_BRF_F_B_B"
#define PLAYDOHopcode_BRF_F_B_F		"PLAYDOHop_BRF_F_B_F"
#define PLAYDOHopcode_BRF_F_F_B		"PLAYDOHop_BRF_F_F_B"
#define PLAYDOHopcode_BRF_F_F_F		"PLAYDOHop_BRF_F_F_F"

#define PLAYDOHopcode_BRW_B_B_B		"PLAYDOHop_BRW_B_B_B"
#define PLAYDOHopcode_BRW_B_B_F		"PLAYDOHop_BRW_B_B_F"
#define PLAYDOHopcode_BRW_B_F_B		"PLAYDOHop_BRW_B_F_B"
#define PLAYDOHopcode_BRW_B_F_F		"PLAYDOHop_BRW_B_F_F"
#define PLAYDOHopcode_BRW_F_B_B		"PLAYDOHop_BRW_F_B_B"
#define PLAYDOHopcode_BRW_F_B_F		"PLAYDOHop_BRW_F_B_F"
#define PLAYDOHopcode_BRW_F_F_B		"PLAYDOHop_BRW_F_F_B"
#define PLAYDOHopcode_BRW_F_F_F		"PLAYDOHop_BRW_F_F_F"

/* data verify branches */
#define PLAYDOHopcode_BRDVI		"PLAYDOHop_BRDVI"
#define PLAYDOHopcode_BRDVF		"PLAYDOHop_BRDVF"

/*
 * PlayDoh integer load operations Lopcode_LD[_POST]_(UC|UC2|I)
 */
#define PLAYDOHopcode_L_B_V1_V1		"PLAYDOHop_L_B_V1_V1"
#define PLAYDOHopcode_L_B_V1_C1		"PLAYDOHop_L_B_V1_C1"
#define PLAYDOHopcode_L_B_V1_C2		"PLAYDOHop_L_B_V1_C2"
#define PLAYDOHopcode_L_B_V1_C3		"PLAYDOHop_L_B_V1_C3"
#define PLAYDOHopcode_L_B_C1_V1		"PLAYDOHop_L_B_C1_V1"
#define PLAYDOHopcode_L_B_C1_C1		"PLAYDOHop_L_B_C1_C1"
#define PLAYDOHopcode_L_B_C1_C2		"PLAYDOHop_L_B_C1_C2"
#define PLAYDOHopcode_L_B_C1_C3		"PLAYDOHop_L_B_C1_C3"
#define PLAYDOHopcode_L_B_C2_V1		"PLAYDOHop_L_B_C2_V1"
#define PLAYDOHopcode_L_B_C2_C1		"PLAYDOHop_L_B_C2_C1"
#define PLAYDOHopcode_L_B_C2_C2		"PLAYDOHop_L_B_C2_C2"
#define PLAYDOHopcode_L_B_C2_C3		"PLAYDOHop_L_B_C2_C3"
#define PLAYDOHopcode_L_B_C3_V1		"PLAYDOHop_L_B_C3_V1"
#define PLAYDOHopcode_L_B_C3_C1		"PLAYDOHop_L_B_C3_C1"
#define PLAYDOHopcode_L_B_C3_C2		"PLAYDOHop_L_B_C3_C2"
#define PLAYDOHopcode_L_B_C3_C3		"PLAYDOHop_L_B_C3_C3"

#define PLAYDOHopcode_L_H_V1_V1		"PLAYDOHop_L_H_V1_V1"
#define PLAYDOHopcode_L_H_V1_C1		"PLAYDOHop_L_H_V1_C1"
#define PLAYDOHopcode_L_H_V1_C2		"PLAYDOHop_L_H_V1_C2"
#define PLAYDOHopcode_L_H_V1_C3		"PLAYDOHop_L_H_V1_C3"
#define PLAYDOHopcode_L_H_C1_V1		"PLAYDOHop_L_H_C1_V1"
#define PLAYDOHopcode_L_H_C1_C1		"PLAYDOHop_L_H_C1_C1"
#define PLAYDOHopcode_L_H_C1_C2		"PLAYDOHop_L_H_C1_C2"
#define PLAYDOHopcode_L_H_C1_C3		"PLAYDOHop_L_H_C1_C3"
#define PLAYDOHopcode_L_H_C2_V1		"PLAYDOHop_L_H_C2_V1"
#define PLAYDOHopcode_L_H_C2_C1		"PLAYDOHop_L_H_C2_C1"
#define PLAYDOHopcode_L_H_C2_C2		"PLAYDOHop_L_H_C2_C2"
#define PLAYDOHopcode_L_H_C2_C3		"PLAYDOHop_L_H_C2_C3"
#define PLAYDOHopcode_L_H_C3_V1		"PLAYDOHop_L_H_C3_V1"
#define PLAYDOHopcode_L_H_C3_C1		"PLAYDOHop_L_H_C3_C1"
#define PLAYDOHopcode_L_H_C3_C2		"PLAYDOHop_L_H_C3_C2"
#define PLAYDOHopcode_L_H_C3_C3		"PLAYDOHop_L_H_C3_C3"

#define PLAYDOHopcode_L_W_V1_V1		"PLAYDOHop_L_W_V1_V1"
#define PLAYDOHopcode_L_W_V1_C1		"PLAYDOHop_L_W_V1_C1"
#define PLAYDOHopcode_L_W_V1_C2		"PLAYDOHop_L_W_V1_C2"
#define PLAYDOHopcode_L_W_V1_C3		"PLAYDOHop_L_W_V1_C3"
#define PLAYDOHopcode_L_W_C1_V1		"PLAYDOHop_L_W_C1_V1"
#define PLAYDOHopcode_L_W_C1_C1		"PLAYDOHop_L_W_C1_C1"
#define PLAYDOHopcode_L_W_C1_C2		"PLAYDOHop_L_W_C1_C2"
#define PLAYDOHopcode_L_W_C1_C3		"PLAYDOHop_L_W_C1_C3"
#define PLAYDOHopcode_L_W_C2_V1		"PLAYDOHop_L_W_C2_V1"
#define PLAYDOHopcode_L_W_C2_C1		"PLAYDOHop_L_W_C2_C1"
#define PLAYDOHopcode_L_W_C2_C2		"PLAYDOHop_L_W_C2_C2"
#define PLAYDOHopcode_L_W_C2_C3		"PLAYDOHop_L_W_C2_C3"
#define PLAYDOHopcode_L_W_C3_V1		"PLAYDOHop_L_W_C3_V1"
#define PLAYDOHopcode_L_W_C3_C1		"PLAYDOHop_L_W_C3_C1"
#define PLAYDOHopcode_L_W_C3_C2		"PLAYDOHop_L_W_C3_C2"
#define PLAYDOHopcode_L_W_C3_C3		"PLAYDOHop_L_W_C3_C3"

#define PLAYDOHopcode_L_Q_V1_V1		"PLAYDOHop_L_Q_V1_V1"
#define PLAYDOHopcode_L_Q_V1_C1		"PLAYDOHop_L_Q_V1_C1"
#define PLAYDOHopcode_L_Q_V1_C2		"PLAYDOHop_L_Q_V1_C2"
#define PLAYDOHopcode_L_Q_V1_C3		"PLAYDOHop_L_Q_V1_C3"
#define PLAYDOHopcode_L_Q_C1_V1		"PLAYDOHop_L_Q_C1_V1"
#define PLAYDOHopcode_L_Q_C1_C1		"PLAYDOHop_L_Q_C1_C1"
#define PLAYDOHopcode_L_Q_C1_C2		"PLAYDOHop_L_Q_C1_C2"
#define PLAYDOHopcode_L_Q_C1_C3		"PLAYDOHop_L_Q_C1_C3"
#define PLAYDOHopcode_L_Q_C2_V1		"PLAYDOHop_L_Q_C2_V1"
#define PLAYDOHopcode_L_Q_C2_C1		"PLAYDOHop_L_Q_C2_C1"
#define PLAYDOHopcode_L_Q_C2_C2		"PLAYDOHop_L_Q_C2_C2"
#define PLAYDOHopcode_L_Q_C2_C3		"PLAYDOHop_L_Q_C2_C3"
#define PLAYDOHopcode_L_Q_C3_V1		"PLAYDOHop_L_Q_C3_V1"
#define PLAYDOHopcode_L_Q_C3_C1		"PLAYDOHop_L_Q_C3_C1"
#define PLAYDOHopcode_L_Q_C3_C2		"PLAYDOHop_L_Q_C3_C2"
#define PLAYDOHopcode_L_Q_C3_C3		"PLAYDOHop_L_Q_C3_C3"

#define PLAYDOHopcode_LI_B_V1_V1	"PLAYDOHop_LI_B_V1_V1"
#define PLAYDOHopcode_LI_B_V1_C1	"PLAYDOHop_LI_B_V1_C1"
#define PLAYDOHopcode_LI_B_V1_C2	"PLAYDOHop_LI_B_V1_C2"
#define PLAYDOHopcode_LI_B_V1_C3	"PLAYDOHop_LI_B_V1_C3"
#define PLAYDOHopcode_LI_B_C1_V1	"PLAYDOHop_LI_B_C1_V1"
#define PLAYDOHopcode_LI_B_C1_C1	"PLAYDOHop_LI_B_C1_C1"
#define PLAYDOHopcode_LI_B_C1_C2	"PLAYDOHop_LI_B_C1_C2"
#define PLAYDOHopcode_LI_B_C1_C3	"PLAYDOHop_LI_B_C1_C3"
#define PLAYDOHopcode_LI_B_C2_V1	"PLAYDOHop_LI_B_C2_V1"
#define PLAYDOHopcode_LI_B_C2_C1	"PLAYDOHop_LI_B_C2_C1"
#define PLAYDOHopcode_LI_B_C2_C2	"PLAYDOHop_LI_B_C2_C2"
#define PLAYDOHopcode_LI_B_C2_C3	"PLAYDOHop_LI_B_C2_C3"
#define PLAYDOHopcode_LI_B_C3_V1	"PLAYDOHop_LI_B_C3_V1"
#define PLAYDOHopcode_LI_B_C3_C1	"PLAYDOHop_LI_B_C3_C1"
#define PLAYDOHopcode_LI_B_C3_C2	"PLAYDOHop_LI_B_C3_C2"
#define PLAYDOHopcode_LI_B_C3_C3	"PLAYDOHop_LI_B_C3_C3"

#define PLAYDOHopcode_LI_H_V1_V1	"PLAYDOHop_LI_H_V1_V1"
#define PLAYDOHopcode_LI_H_V1_C1	"PLAYDOHop_LI_H_V1_C1"
#define PLAYDOHopcode_LI_H_V1_C2	"PLAYDOHop_LI_H_V1_C2"
#define PLAYDOHopcode_LI_H_V1_C3	"PLAYDOHop_LI_H_V1_C3"
#define PLAYDOHopcode_LI_H_C1_V1	"PLAYDOHop_LI_H_C1_V1"
#define PLAYDOHopcode_LI_H_C1_C1	"PLAYDOHop_LI_H_C1_C1"
#define PLAYDOHopcode_LI_H_C1_C2	"PLAYDOHop_LI_H_C1_C2"
#define PLAYDOHopcode_LI_H_C1_C3	"PLAYDOHop_LI_H_C1_C3"
#define PLAYDOHopcode_LI_H_C2_V1	"PLAYDOHop_LI_H_C2_V1"
#define PLAYDOHopcode_LI_H_C2_C1	"PLAYDOHop_LI_H_C2_C1"
#define PLAYDOHopcode_LI_H_C2_C2	"PLAYDOHop_LI_H_C2_C2"
#define PLAYDOHopcode_LI_H_C2_C3	"PLAYDOHop_LI_H_C2_C3"
#define PLAYDOHopcode_LI_H_C3_V1	"PLAYDOHop_LI_H_C3_V1"
#define PLAYDOHopcode_LI_H_C3_C1	"PLAYDOHop_LI_H_C3_C1"
#define PLAYDOHopcode_LI_H_C3_C2	"PLAYDOHop_LI_H_C3_C2"
#define PLAYDOHopcode_LI_H_C3_C3	"PLAYDOHop_LI_H_C3_C3"

#define PLAYDOHopcode_LI_W_V1_V1	"PLAYDOHop_LI_W_V1_V1"
#define PLAYDOHopcode_LI_W_V1_C1	"PLAYDOHop_LI_W_V1_C1"
#define PLAYDOHopcode_LI_W_V1_C2	"PLAYDOHop_LI_W_V1_C2"
#define PLAYDOHopcode_LI_W_V1_C3	"PLAYDOHop_LI_W_V1_C3"
#define PLAYDOHopcode_LI_W_C1_V1	"PLAYDOHop_LI_W_C1_V1"
#define PLAYDOHopcode_LI_W_C1_C1	"PLAYDOHop_LI_W_C1_C1"
#define PLAYDOHopcode_LI_W_C1_C2	"PLAYDOHop_LI_W_C1_C2"
#define PLAYDOHopcode_LI_W_C1_C3	"PLAYDOHop_LI_W_C1_C3"
#define PLAYDOHopcode_LI_W_C2_V1	"PLAYDOHop_LI_W_C2_V1"
#define PLAYDOHopcode_LI_W_C2_C1	"PLAYDOHop_LI_W_C2_C1"
#define PLAYDOHopcode_LI_W_C2_C2	"PLAYDOHop_LI_W_C2_C2"
#define PLAYDOHopcode_LI_W_C2_C3	"PLAYDOHop_LI_W_C2_C3"
#define PLAYDOHopcode_LI_W_C3_V1	"PLAYDOHop_LI_W_C3_V1"
#define PLAYDOHopcode_LI_W_C3_C1	"PLAYDOHop_LI_W_C3_C1"
#define PLAYDOHopcode_LI_W_C3_C2	"PLAYDOHop_LI_W_C3_C2"
#define PLAYDOHopcode_LI_W_C3_C3	"PLAYDOHop_LI_W_C3_C3"

#define PLAYDOHopcode_LI_Q_V1_V1	"PLAYDOHop_LI_Q_V1_V1"
#define PLAYDOHopcode_LI_Q_V1_C1	"PLAYDOHop_LI_Q_V1_C1"
#define PLAYDOHopcode_LI_Q_V1_C2	"PLAYDOHop_LI_Q_V1_C2"
#define PLAYDOHopcode_LI_Q_V1_C3	"PLAYDOHop_LI_Q_V1_C3"
#define PLAYDOHopcode_LI_Q_C1_V1	"PLAYDOHop_LI_Q_C1_V1"
#define PLAYDOHopcode_LI_Q_C1_C1	"PLAYDOHop_LI_Q_C1_C1"
#define PLAYDOHopcode_LI_Q_C1_C2	"PLAYDOHop_LI_Q_C1_C2"
#define PLAYDOHopcode_LI_Q_C1_C3	"PLAYDOHop_LI_Q_C1_C3"
#define PLAYDOHopcode_LI_Q_C2_V1	"PLAYDOHop_LI_Q_C2_V1"
#define PLAYDOHopcode_LI_Q_C2_C1	"PLAYDOHop_LI_Q_C2_C1"
#define PLAYDOHopcode_LI_Q_C2_C2	"PLAYDOHop_LI_Q_C2_C2"
#define PLAYDOHopcode_LI_Q_C2_C3	"PLAYDOHop_LI_Q_C2_C3"
#define PLAYDOHopcode_LI_Q_C3_V1	"PLAYDOHop_LI_Q_C3_V1"
#define PLAYDOHopcode_LI_Q_C3_C1	"PLAYDOHop_LI_Q_C3_C1"
#define PLAYDOHopcode_LI_Q_C3_C2	"PLAYDOHop_LI_Q_C3_C2"
#define PLAYDOHopcode_LI_Q_C3_C3	"PLAYDOHop_LI_Q_C3_C3"

/*
 * PlayDoh integer store operations Lop_ST[_POST]_(C|C2|I)
 */
#define PLAYDOHopcode_S_B_V1		"PLAYDOHop_S_B_V1"
#define PLAYDOHopcode_S_B_C1		"PLAYDOHop_S_B_C1"
#define PLAYDOHopcode_S_B_C2		"PLAYDOHop_S_B_C2"
#define PLAYDOHopcode_S_B_C3		"PLAYDOHop_S_B_C3"

#define PLAYDOHopcode_S_H_V1		"PLAYDOHop_S_H_V1"
#define PLAYDOHopcode_S_H_C1		"PLAYDOHop_S_H_C1"
#define PLAYDOHopcode_S_H_C2		"PLAYDOHop_S_H_C2"
#define PLAYDOHopcode_S_H_C3		"PLAYDOHop_S_H_C3"

#define PLAYDOHopcode_S_W_V1		"PLAYDOHop_S_W_V1"
#define PLAYDOHopcode_S_W_C1		"PLAYDOHop_S_W_C1"
#define PLAYDOHopcode_S_W_C2		"PLAYDOHop_S_W_C2"
#define PLAYDOHopcode_S_W_C3		"PLAYDOHop_S_W_C3"

#define PLAYDOHopcode_S_Q_V1		"PLAYDOHop_S_Q_V1"
#define PLAYDOHopcode_S_Q_C1		"PLAYDOHop_S_Q_C1"
#define PLAYDOHopcode_S_Q_C2		"PLAYDOHop_S_Q_C2"
#define PLAYDOHopcode_S_Q_C3		"PLAYDOHop_S_Q_C3"

#define PLAYDOHopcode_SI_B_V1		"PLAYDOHop_SI_B_V1"
#define PLAYDOHopcode_SI_B_C1		"PLAYDOHop_SI_B_C1"
#define PLAYDOHopcode_SI_B_C2		"PLAYDOHop_SI_B_C2"
#define PLAYDOHopcode_SI_B_C3		"PLAYDOHop_SI_B_C3"

#define PLAYDOHopcode_SI_H_V1		"PLAYDOHop_SI_H_V1"
#define PLAYDOHopcode_SI_H_C1		"PLAYDOHop_SI_H_C1"
#define PLAYDOHopcode_SI_H_C2		"PLAYDOHop_SI_H_C2"
#define PLAYDOHopcode_SI_H_C3		"PLAYDOHop_SI_H_C3"

#define PLAYDOHopcode_SI_W_V1		"PLAYDOHop_SI_W_V1"
#define PLAYDOHopcode_SI_W_C1		"PLAYDOHop_SI_W_C1"
#define PLAYDOHopcode_SI_W_C2		"PLAYDOHop_SI_W_C2"
#define PLAYDOHopcode_SI_W_C3		"PLAYDOHop_SI_W_C3"

#define PLAYDOHopcode_SI_Q_V1		"PLAYDOHop_SI_Q_V1"
#define PLAYDOHopcode_SI_Q_C1		"PLAYDOHop_SI_Q_C1"
#define PLAYDOHopcode_SI_Q_C2		"PLAYDOHop_SI_Q_C2"
#define PLAYDOHopcode_SI_Q_C3		"PLAYDOHop_SI_Q_C3"

/*
 * PlayDoh floating point load operations Lop_LD[_POST]_F[2]
 */
#define PLAYDOHopcode_FL_S_V1_V1	"PLAYDOHop_FL_S_V1_V1"
#define PLAYDOHopcode_FL_S_V1_C1	"PLAYDOHop_FL_S_V1_C1"
#define PLAYDOHopcode_FL_S_V1_C2	"PLAYDOHop_FL_S_V1_C2"
#define PLAYDOHopcode_FL_S_V1_C3	"PLAYDOHop_FL_S_V1_C3"
#define PLAYDOHopcode_FL_S_C1_V1	"PLAYDOHop_FL_S_C1_V1"
#define PLAYDOHopcode_FL_S_C1_C1	"PLAYDOHop_FL_S_C1_C1"
#define PLAYDOHopcode_FL_S_C1_C2	"PLAYDOHop_FL_S_C1_C2"
#define PLAYDOHopcode_FL_S_C1_C3	"PLAYDOHop_FL_S_C1_C3"
#define PLAYDOHopcode_FL_S_C2_V1	"PLAYDOHop_FL_S_C2_V1"
#define PLAYDOHopcode_FL_S_C2_C1	"PLAYDOHop_FL_S_C2_C1"
#define PLAYDOHopcode_FL_S_C2_C2	"PLAYDOHop_FL_S_C2_C2"
#define PLAYDOHopcode_FL_S_C2_C3	"PLAYDOHop_FL_S_C2_C3"
#define PLAYDOHopcode_FL_S_C3_V1	"PLAYDOHop_FL_S_C3_V1"
#define PLAYDOHopcode_FL_S_C3_C1	"PLAYDOHop_FL_S_C3_C1"
#define PLAYDOHopcode_FL_S_C3_C2	"PLAYDOHop_FL_S_C3_C2"
#define PLAYDOHopcode_FL_S_C3_C3	"PLAYDOHop_FL_S_C3_C3"

#define PLAYDOHopcode_FL_D_V1_V1	"PLAYDOHop_FL_D_V1_V1"
#define PLAYDOHopcode_FL_D_V1_C1	"PLAYDOHop_FL_D_V1_C1"
#define PLAYDOHopcode_FL_D_V1_C2	"PLAYDOHop_FL_D_V1_C2"
#define PLAYDOHopcode_FL_D_V1_C3	"PLAYDOHop_FL_D_V1_C3"
#define PLAYDOHopcode_FL_D_C1_V1	"PLAYDOHop_FL_D_C1_V1"
#define PLAYDOHopcode_FL_D_C1_C1	"PLAYDOHop_FL_D_C1_C1"
#define PLAYDOHopcode_FL_D_C1_C2	"PLAYDOHop_FL_D_C1_C2"
#define PLAYDOHopcode_FL_D_C1_C3	"PLAYDOHop_FL_D_C1_C3"
#define PLAYDOHopcode_FL_D_C2_V1	"PLAYDOHop_FL_D_C2_V1"
#define PLAYDOHopcode_FL_D_C2_C1	"PLAYDOHop_FL_D_C2_C1"
#define PLAYDOHopcode_FL_D_C2_C2	"PLAYDOHop_FL_D_C2_C2"
#define PLAYDOHopcode_FL_D_C2_C3	"PLAYDOHop_FL_D_C2_C3"
#define PLAYDOHopcode_FL_D_C3_V1	"PLAYDOHop_FL_D_C3_V1"
#define PLAYDOHopcode_FL_D_C3_C1	"PLAYDOHop_FL_D_C3_C1"
#define PLAYDOHopcode_FL_D_C3_C2	"PLAYDOHop_FL_D_C3_C2"
#define PLAYDOHopcode_FL_D_C3_C3	"PLAYDOHop_FL_D_C3_C3"

#define PLAYDOHopcode_FLI_S_V1_V1	"PLAYDOHop_FLI_S_V1_V1"
#define PLAYDOHopcode_FLI_S_V1_C1	"PLAYDOHop_FLI_S_V1_C1"
#define PLAYDOHopcode_FLI_S_V1_C2	"PLAYDOHop_FLI_S_V1_C2"
#define PLAYDOHopcode_FLI_S_V1_C3	"PLAYDOHop_FLI_S_V1_C3"
#define PLAYDOHopcode_FLI_S_C1_V1	"PLAYDOHop_FLI_S_C1_V1"
#define PLAYDOHopcode_FLI_S_C1_C1	"PLAYDOHop_FLI_S_C1_C1"
#define PLAYDOHopcode_FLI_S_C1_C2	"PLAYDOHop_FLI_S_C1_C2"
#define PLAYDOHopcode_FLI_S_C1_C3	"PLAYDOHop_FLI_S_C1_C3"
#define PLAYDOHopcode_FLI_S_C2_V1	"PLAYDOHop_FLI_S_C2_V1"
#define PLAYDOHopcode_FLI_S_C2_C1	"PLAYDOHop_FLI_S_C2_C1"
#define PLAYDOHopcode_FLI_S_C2_C2	"PLAYDOHop_FLI_S_C2_C2"
#define PLAYDOHopcode_FLI_S_C2_C3	"PLAYDOHop_FLI_S_C2_C3"
#define PLAYDOHopcode_FLI_S_C3_V1	"PLAYDOHop_FLI_S_C3_V1"
#define PLAYDOHopcode_FLI_S_C3_C1	"PLAYDOHop_FLI_S_C3_C1"
#define PLAYDOHopcode_FLI_S_C3_C2	"PLAYDOHop_FLI_S_C3_C2"
#define PLAYDOHopcode_FLI_S_C3_C3	"PLAYDOHop_FLI_S_C3_C3"

#define PLAYDOHopcode_FLI_D_V1_V1	"PLAYDOHop_FLI_D_V1_V1"
#define PLAYDOHopcode_FLI_D_V1_C1	"PLAYDOHop_FLI_D_V1_C1"
#define PLAYDOHopcode_FLI_D_V1_C2	"PLAYDOHop_FLI_D_V1_C2"
#define PLAYDOHopcode_FLI_D_V1_C3	"PLAYDOHop_FLI_D_V1_C3"
#define PLAYDOHopcode_FLI_D_C1_V1	"PLAYDOHop_FLI_D_C1_V1"
#define PLAYDOHopcode_FLI_D_C1_C1	"PLAYDOHop_FLI_D_C1_C1"
#define PLAYDOHopcode_FLI_D_C1_C2	"PLAYDOHop_FLI_D_C1_C2"
#define PLAYDOHopcode_FLI_D_C1_C3	"PLAYDOHop_FLI_D_C1_C3"
#define PLAYDOHopcode_FLI_D_C2_V1	"PLAYDOHop_FLI_D_C2_V1"
#define PLAYDOHopcode_FLI_D_C2_C1	"PLAYDOHop_FLI_D_C2_C1"
#define PLAYDOHopcode_FLI_D_C2_C2	"PLAYDOHop_FLI_D_C2_C2"
#define PLAYDOHopcode_FLI_D_C2_C3	"PLAYDOHop_FLI_D_C2_C3"
#define PLAYDOHopcode_FLI_D_C3_V1	"PLAYDOHop_FLI_D_C3_V1"
#define PLAYDOHopcode_FLI_D_C3_C1	"PLAYDOHop_FLI_D_C3_C1"
#define PLAYDOHopcode_FLI_D_C3_C2	"PLAYDOHop_FLI_D_C3_C2"
#define PLAYDOHopcode_FLI_D_C3_C3	"PLAYDOHop_FLI_D_C3_C3"

/*
 * PlayDoh floating point store operations Lop_ST[_POST]_F[2]
 */
#define PLAYDOHopcode_FS_S_V1		"PLAYDOHop_FS_S_V1"
#define PLAYDOHopcode_FS_S_C1		"PLAYDOHop_FS_S_C1"
#define PLAYDOHopcode_FS_S_C2		"PLAYDOHop_FS_S_C2"
#define PLAYDOHopcode_FS_S_C3		"PLAYDOHop_FS_S_C3"

#define PLAYDOHopcode_FS_D_V1		"PLAYDOHop_FS_D_V1"
#define PLAYDOHopcode_FS_D_C1		"PLAYDOHop_FS_D_C1"
#define PLAYDOHopcode_FS_D_C2		"PLAYDOHop_FS_D_C2"
#define PLAYDOHopcode_FS_D_C3		"PLAYDOHop_FS_D_C3"

#define PLAYDOHopcode_FSI_S_V1		"PLAYDOHop_FSI_S_V1"
#define PLAYDOHopcode_FSI_S_C1		"PLAYDOHop_FSI_S_C1"
#define PLAYDOHopcode_FSI_S_C2		"PLAYDOHop_FSI_S_C2"
#define PLAYDOHopcode_FSI_S_C3		"PLAYDOHop_FSI_S_C3"

#define PLAYDOHopcode_FSI_D_V1		"PLAYDOHop_FSI_D_V1"
#define PLAYDOHopcode_FSI_D_C1		"PLAYDOHop_FSI_D_C1"
#define PLAYDOHopcode_FSI_D_C2		"PLAYDOHop_FSI_D_C2"
#define PLAYDOHopcode_FSI_D_C3		"PLAYDOHop_FSI_D_C3"

/* 
 * PlayDoh data speculative integer and floating point load operations
 */

#define PLAYDOHopcode_LDS_B_V1_V1	"PLAYDOHop_LDS_B_V1_V1"
#define PLAYDOHopcode_LDS_B_V1_C1	"PLAYDOHop_LDS_B_V1_C1"
#define PLAYDOHopcode_LDS_B_V1_C2	"PLAYDOHop_LDS_B_V1_C2"
#define PLAYDOHopcode_LDS_B_V1_C3	"PLAYDOHop_LDS_B_V1_C3"
#define PLAYDOHopcode_LDS_B_C1_V1	"PLAYDOHop_LDS_B_C1_V1"
#define PLAYDOHopcode_LDS_B_C1_C1	"PLAYDOHop_LDS_B_C1_C1"
#define PLAYDOHopcode_LDS_B_C1_C2	"PLAYDOHop_LDS_B_C1_C2"
#define PLAYDOHopcode_LDS_B_C1_C3	"PLAYDOHop_LDS_B_C1_C3"
#define PLAYDOHopcode_LDS_B_C2_V1	"PLAYDOHop_LDS_B_C2_V1"
#define PLAYDOHopcode_LDS_B_C2_C1	"PLAYDOHop_LDS_B_C2_C1"
#define PLAYDOHopcode_LDS_B_C2_C2	"PLAYDOHop_LDS_B_C2_C2"
#define PLAYDOHopcode_LDS_B_C2_C3	"PLAYDOHop_LDS_B_C2_C3"
#define PLAYDOHopcode_LDS_B_C3_V1	"PLAYDOHop_LDS_B_C3_V1"
#define PLAYDOHopcode_LDS_B_C3_C1	"PLAYDOHop_LDS_B_C3_C1"
#define PLAYDOHopcode_LDS_B_C3_C2	"PLAYDOHop_LDS_B_C3_C2"
#define PLAYDOHopcode_LDS_B_C3_C3	"PLAYDOHop_LDS_B_C3_C3"

#define PLAYDOHopcode_LDS_H_V1_V1	"PLAYDOHop_LDS_H_V1_V1"
#define PLAYDOHopcode_LDS_H_V1_C1	"PLAYDOHop_LDS_H_V1_C1"
#define PLAYDOHopcode_LDS_H_V1_C2	"PLAYDOHop_LDS_H_V1_C2"
#define PLAYDOHopcode_LDS_H_V1_C3	"PLAYDOHop_LDS_H_V1_C3"
#define PLAYDOHopcode_LDS_H_C1_V1	"PLAYDOHop_LDS_H_C1_V1"
#define PLAYDOHopcode_LDS_H_C1_C1	"PLAYDOHop_LDS_H_C1_C1"
#define PLAYDOHopcode_LDS_H_C1_C2	"PLAYDOHop_LDS_H_C1_C2"
#define PLAYDOHopcode_LDS_H_C1_C3	"PLAYDOHop_LDS_H_C1_C3"
#define PLAYDOHopcode_LDS_H_C2_V1	"PLAYDOHop_LDS_H_C2_V1"
#define PLAYDOHopcode_LDS_H_C2_C1	"PLAYDOHop_LDS_H_C2_C1"
#define PLAYDOHopcode_LDS_H_C2_C2	"PLAYDOHop_LDS_H_C2_C2"
#define PLAYDOHopcode_LDS_H_C2_C3	"PLAYDOHop_LDS_H_C2_C3"
#define PLAYDOHopcode_LDS_H_C3_V1	"PLAYDOHop_LDS_H_C3_V1"
#define PLAYDOHopcode_LDS_H_C3_C1	"PLAYDOHop_LDS_H_C3_C1"
#define PLAYDOHopcode_LDS_H_C3_C2	"PLAYDOHop_LDS_H_C3_C2"
#define PLAYDOHopcode_LDS_H_C3_C3	"PLAYDOHop_LDS_H_C3_C3"

#define PLAYDOHopcode_LDS_W_V1_V1	"PLAYDOHop_LDS_W_V1_V1"
#define PLAYDOHopcode_LDS_W_V1_C1	"PLAYDOHop_LDS_W_V1_C1"
#define PLAYDOHopcode_LDS_W_V1_C2	"PLAYDOHop_LDS_W_V1_C2"
#define PLAYDOHopcode_LDS_W_V1_C3	"PLAYDOHop_LDS_W_V1_C3"
#define PLAYDOHopcode_LDS_W_C1_V1	"PLAYDOHop_LDS_W_C1_V1"
#define PLAYDOHopcode_LDS_W_C1_C1	"PLAYDOHop_LDS_W_C1_C1"
#define PLAYDOHopcode_LDS_W_C1_C2	"PLAYDOHop_LDS_W_C1_C2"
#define PLAYDOHopcode_LDS_W_C1_C3	"PLAYDOHop_LDS_W_C1_C3"
#define PLAYDOHopcode_LDS_W_C2_V1	"PLAYDOHop_LDS_W_C2_V1"
#define PLAYDOHopcode_LDS_W_C2_C1	"PLAYDOHop_LDS_W_C2_C1"
#define PLAYDOHopcode_LDS_W_C2_C2	"PLAYDOHop_LDS_W_C2_C2"
#define PLAYDOHopcode_LDS_W_C2_C3	"PLAYDOHop_LDS_W_C2_C3"
#define PLAYDOHopcode_LDS_W_C3_V1	"PLAYDOHop_LDS_W_C3_V1"
#define PLAYDOHopcode_LDS_W_C3_C1	"PLAYDOHop_LDS_W_C3_C1"
#define PLAYDOHopcode_LDS_W_C3_C2	"PLAYDOHop_LDS_W_C3_C2"
#define PLAYDOHopcode_LDS_W_C3_C3	"PLAYDOHop_LDS_W_C3_C3"

#define PLAYDOHopcode_LDS_Q_V1_V1	"PLAYDOHop_LDS_Q_V1_V1"
#define PLAYDOHopcode_LDS_Q_V1_C1	"PLAYDOHop_LDS_Q_V1_C1"
#define PLAYDOHopcode_LDS_Q_V1_C2	"PLAYDOHop_LDS_Q_V1_C2"
#define PLAYDOHopcode_LDS_Q_V1_C3	"PLAYDOHop_LDS_Q_V1_C3"
#define PLAYDOHopcode_LDS_Q_C1_V1	"PLAYDOHop_LDS_Q_C1_V1"
#define PLAYDOHopcode_LDS_Q_C1_C1	"PLAYDOHop_LDS_Q_C1_C1"
#define PLAYDOHopcode_LDS_Q_C1_C2	"PLAYDOHop_LDS_Q_C1_C2"
#define PLAYDOHopcode_LDS_Q_C1_C3	"PLAYDOHop_LDS_Q_C1_C3"
#define PLAYDOHopcode_LDS_Q_C2_V1	"PLAYDOHop_LDS_Q_C2_V1"
#define PLAYDOHopcode_LDS_Q_C2_C1	"PLAYDOHop_LDS_Q_C2_C1"
#define PLAYDOHopcode_LDS_Q_C2_C2	"PLAYDOHop_LDS_Q_C2_C2"
#define PLAYDOHopcode_LDS_Q_C2_C3	"PLAYDOHop_LDS_Q_C2_C3"
#define PLAYDOHopcode_LDS_Q_C3_V1	"PLAYDOHop_LDS_Q_C3_V1"
#define PLAYDOHopcode_LDS_Q_C3_C1	"PLAYDOHop_LDS_Q_C3_C1"
#define PLAYDOHopcode_LDS_Q_C3_C2	"PLAYDOHop_LDS_Q_C3_C2"
#define PLAYDOHopcode_LDS_Q_C3_C3	"PLAYDOHop_LDS_Q_C3_C3"

#define PLAYDOHopcode_LDSI_B_V1_V1	"PLAYDOHop_LDSI_B_V1_V1"
#define PLAYDOHopcode_LDSI_B_V1_C1	"PLAYDOHop_LDSI_B_V1_C1"
#define PLAYDOHopcode_LDSI_B_V1_C2	"PLAYDOHop_LDSI_B_V1_C2"
#define PLAYDOHopcode_LDSI_B_V1_C3	"PLAYDOHop_LDSI_B_V1_C3"
#define PLAYDOHopcode_LDSI_B_C1_V1	"PLAYDOHop_LDSI_B_C1_V1"
#define PLAYDOHopcode_LDSI_B_C1_C1	"PLAYDOHop_LDSI_B_C1_C1"
#define PLAYDOHopcode_LDSI_B_C1_C2	"PLAYDOHop_LDSI_B_C1_C2"
#define PLAYDOHopcode_LDSI_B_C1_C3	"PLAYDOHop_LDSI_B_C1_C3"
#define PLAYDOHopcode_LDSI_B_C2_V1	"PLAYDOHop_LDSI_B_C2_V1"
#define PLAYDOHopcode_LDSI_B_C2_C1	"PLAYDOHop_LDSI_B_C2_C1"
#define PLAYDOHopcode_LDSI_B_C2_C2	"PLAYDOHop_LDSI_B_C2_C2"
#define PLAYDOHopcode_LDSI_B_C2_C3	"PLAYDOHop_LDSI_B_C2_C3"
#define PLAYDOHopcode_LDSI_B_C3_V1	"PLAYDOHop_LDSI_B_C3_V1"
#define PLAYDOHopcode_LDSI_B_C3_C1	"PLAYDOHop_LDSI_B_C3_C1"
#define PLAYDOHopcode_LDSI_B_C3_C2	"PLAYDOHop_LDSI_B_C3_C2"
#define PLAYDOHopcode_LDSI_B_C3_C3	"PLAYDOHop_LDSI_B_C3_C3"

#define PLAYDOHopcode_LDSI_H_V1_V1	"PLAYDOHop_LDSI_H_V1_V1"
#define PLAYDOHopcode_LDSI_H_V1_C1	"PLAYDOHop_LDSI_H_V1_C1"
#define PLAYDOHopcode_LDSI_H_V1_C2	"PLAYDOHop_LDSI_H_V1_C2"
#define PLAYDOHopcode_LDSI_H_V1_C3	"PLAYDOHop_LDSI_H_V1_C3"
#define PLAYDOHopcode_LDSI_H_C1_V1	"PLAYDOHop_LDSI_H_C1_V1"
#define PLAYDOHopcode_LDSI_H_C1_C1	"PLAYDOHop_LDSI_H_C1_C1"
#define PLAYDOHopcode_LDSI_H_C1_C2	"PLAYDOHop_LDSI_H_C1_C2"
#define PLAYDOHopcode_LDSI_H_C1_C3	"PLAYDOHop_LDSI_H_C1_C3"
#define PLAYDOHopcode_LDSI_H_C2_V1	"PLAYDOHop_LDSI_H_C2_V1"
#define PLAYDOHopcode_LDSI_H_C2_C1	"PLAYDOHop_LDSI_H_C2_C1"
#define PLAYDOHopcode_LDSI_H_C2_C2	"PLAYDOHop_LDSI_H_C2_C2"
#define PLAYDOHopcode_LDSI_H_C2_C3	"PLAYDOHop_LDSI_H_C2_C3"
#define PLAYDOHopcode_LDSI_H_C3_V1	"PLAYDOHop_LDSI_H_C3_V1"
#define PLAYDOHopcode_LDSI_H_C3_C1	"PLAYDOHop_LDSI_H_C3_C1"
#define PLAYDOHopcode_LDSI_H_C3_C2	"PLAYDOHop_LDSI_H_C3_C2"
#define PLAYDOHopcode_LDSI_H_C3_C3	"PLAYDOHop_LDSI_H_C3_C3"

#define PLAYDOHopcode_LDSI_W_V1_V1	"PLAYDOHop_LDSI_W_V1_V1"
#define PLAYDOHopcode_LDSI_W_V1_C1	"PLAYDOHop_LDSI_W_V1_C1"
#define PLAYDOHopcode_LDSI_W_V1_C2	"PLAYDOHop_LDSI_W_V1_C2"
#define PLAYDOHopcode_LDSI_W_V1_C3	"PLAYDOHop_LDSI_W_V1_C3"
#define PLAYDOHopcode_LDSI_W_C1_V1	"PLAYDOHop_LDSI_W_C1_V1"
#define PLAYDOHopcode_LDSI_W_C1_C1	"PLAYDOHop_LDSI_W_C1_C1"
#define PLAYDOHopcode_LDSI_W_C1_C2	"PLAYDOHop_LDSI_W_C1_C2"
#define PLAYDOHopcode_LDSI_W_C1_C3	"PLAYDOHop_LDSI_W_C1_C3"
#define PLAYDOHopcode_LDSI_W_C2_V1	"PLAYDOHop_LDSI_W_C2_V1"
#define PLAYDOHopcode_LDSI_W_C2_C1	"PLAYDOHop_LDSI_W_C2_C1"
#define PLAYDOHopcode_LDSI_W_C2_C2	"PLAYDOHop_LDSI_W_C2_C2"
#define PLAYDOHopcode_LDSI_W_C2_C3	"PLAYDOHop_LDSI_W_C2_C3"
#define PLAYDOHopcode_LDSI_W_C3_V1	"PLAYDOHop_LDSI_W_C3_V1"
#define PLAYDOHopcode_LDSI_W_C3_C1	"PLAYDOHop_LDSI_W_C3_C1"
#define PLAYDOHopcode_LDSI_W_C3_C2	"PLAYDOHop_LDSI_W_C3_C2"
#define PLAYDOHopcode_LDSI_W_C3_C3	"PLAYDOHop_LDSI_W_C3_C3"

#define PLAYDOHopcode_LDSI_Q_V1_V1	"PLAYDOHop_LDSI_Q_V1_V1"
#define PLAYDOHopcode_LDSI_Q_V1_C1	"PLAYDOHop_LDSI_Q_V1_C1"
#define PLAYDOHopcode_LDSI_Q_V1_C2	"PLAYDOHop_LDSI_Q_V1_C2"
#define PLAYDOHopcode_LDSI_Q_V1_C3	"PLAYDOHop_LDSI_Q_V1_C3"
#define PLAYDOHopcode_LDSI_Q_C1_V1	"PLAYDOHop_LDSI_Q_C1_V1"
#define PLAYDOHopcode_LDSI_Q_C1_C1	"PLAYDOHop_LDSI_Q_C1_C1"
#define PLAYDOHopcode_LDSI_Q_C1_C2	"PLAYDOHop_LDSI_Q_C1_C2"
#define PLAYDOHopcode_LDSI_Q_C1_C3	"PLAYDOHop_LDSI_Q_C1_C3"
#define PLAYDOHopcode_LDSI_Q_C2_V1	"PLAYDOHop_LDSI_Q_C2_V1"
#define PLAYDOHopcode_LDSI_Q_C2_C1	"PLAYDOHop_LDSI_Q_C2_C1"
#define PLAYDOHopcode_LDSI_Q_C2_C2	"PLAYDOHop_LDSI_Q_C2_C2"
#define PLAYDOHopcode_LDSI_Q_C2_C3	"PLAYDOHop_LDSI_Q_C2_C3"
#define PLAYDOHopcode_LDSI_Q_C3_V1	"PLAYDOHop_LDSI_Q_C3_V1"
#define PLAYDOHopcode_LDSI_Q_C3_C1	"PLAYDOHop_LDSI_Q_C3_C1"
#define PLAYDOHopcode_LDSI_Q_C3_C2	"PLAYDOHop_LDSI_Q_C3_C2"
#define PLAYDOHopcode_LDSI_Q_C3_C3	"PLAYDOHop_LDSI_Q_C3_C3"

#define PLAYDOHopcode_FLDS_S_V1_V1	"PLAYDOHop_FLDS_S_V1_V1"
#define PLAYDOHopcode_FLDS_S_V1_C1	"PLAYDOHop_FLDS_S_V1_C1"
#define PLAYDOHopcode_FLDS_S_V1_C2	"PLAYDOHop_FLDS_S_V1_C2"
#define PLAYDOHopcode_FLDS_S_V1_C3	"PLAYDOHop_FLDS_S_V1_C3"
#define PLAYDOHopcode_FLDS_S_C1_V1	"PLAYDOHop_FLDS_S_C1_V1"
#define PLAYDOHopcode_FLDS_S_C1_C1	"PLAYDOHop_FLDS_S_C1_C1"
#define PLAYDOHopcode_FLDS_S_C1_C2	"PLAYDOHop_FLDS_S_C1_C2"
#define PLAYDOHopcode_FLDS_S_C1_C3	"PLAYDOHop_FLDS_S_C1_C3"
#define PLAYDOHopcode_FLDS_S_C2_V1	"PLAYDOHop_FLDS_S_C2_V1"
#define PLAYDOHopcode_FLDS_S_C2_C1	"PLAYDOHop_FLDS_S_C2_C1"
#define PLAYDOHopcode_FLDS_S_C2_C2	"PLAYDOHop_FLDS_S_C2_C2"
#define PLAYDOHopcode_FLDS_S_C2_C3	"PLAYDOHop_FLDS_S_C2_C3"
#define PLAYDOHopcode_FLDS_S_C3_V1	"PLAYDOHop_FLDS_S_C3_V1"
#define PLAYDOHopcode_FLDS_S_C3_C1	"PLAYDOHop_FLDS_S_C3_C1"
#define PLAYDOHopcode_FLDS_S_C3_C2	"PLAYDOHop_FLDS_S_C3_C2"
#define PLAYDOHopcode_FLDS_S_C3_C3	"PLAYDOHop_FLDS_S_C3_C3"

#define PLAYDOHopcode_FLDS_D_V1_V1	"PLAYDOHop_FLDS_D_V1_V1"
#define PLAYDOHopcode_FLDS_D_V1_C1	"PLAYDOHop_FLDS_D_V1_C1"
#define PLAYDOHopcode_FLDS_D_V1_C2	"PLAYDOHop_FLDS_D_V1_C2"
#define PLAYDOHopcode_FLDS_D_V1_C3	"PLAYDOHop_FLDS_D_V1_C3"
#define PLAYDOHopcode_FLDS_D_C1_V1	"PLAYDOHop_FLDS_D_C1_V1"
#define PLAYDOHopcode_FLDS_D_C1_C1	"PLAYDOHop_FLDS_D_C1_C1"
#define PLAYDOHopcode_FLDS_D_C1_C2	"PLAYDOHop_FLDS_D_C1_C2"
#define PLAYDOHopcode_FLDS_D_C1_C3	"PLAYDOHop_FLDS_D_C1_C3"
#define PLAYDOHopcode_FLDS_D_C2_V1	"PLAYDOHop_FLDS_D_C2_V1"
#define PLAYDOHopcode_FLDS_D_C2_C1	"PLAYDOHop_FLDS_D_C2_C1"
#define PLAYDOHopcode_FLDS_D_C2_C2	"PLAYDOHop_FLDS_D_C2_C2"
#define PLAYDOHopcode_FLDS_D_C2_C3	"PLAYDOHop_FLDS_D_C2_C3"
#define PLAYDOHopcode_FLDS_D_C3_V1	"PLAYDOHop_FLDS_D_C3_V1"
#define PLAYDOHopcode_FLDS_D_C3_C1	"PLAYDOHop_FLDS_D_C3_C1"
#define PLAYDOHopcode_FLDS_D_C3_C2	"PLAYDOHop_FLDS_D_C3_C2"
#define PLAYDOHopcode_FLDS_D_C3_C3	"PLAYDOHop_FLDS_D_C3_C3"

#define PLAYDOHopcode_FLDSI_S_V1_V1	"PLAYDOHop_FLDSI_S_V1_V1"
#define PLAYDOHopcode_FLDSI_S_V1_C1	"PLAYDOHop_FLDSI_S_V1_C1"
#define PLAYDOHopcode_FLDSI_S_V1_C2	"PLAYDOHop_FLDSI_S_V1_C2"
#define PLAYDOHopcode_FLDSI_S_V1_C3	"PLAYDOHop_FLDSI_S_V1_C3"
#define PLAYDOHopcode_FLDSI_S_C1_V1	"PLAYDOHop_FLDSI_S_C1_V1"
#define PLAYDOHopcode_FLDSI_S_C1_C1	"PLAYDOHop_FLDSI_S_C1_C1"
#define PLAYDOHopcode_FLDSI_S_C1_C2	"PLAYDOHop_FLDSI_S_C1_C2"
#define PLAYDOHopcode_FLDSI_S_C1_C3	"PLAYDOHop_FLDSI_S_C1_C3"
#define PLAYDOHopcode_FLDSI_S_C2_V1	"PLAYDOHop_FLDSI_S_C2_V1"
#define PLAYDOHopcode_FLDSI_S_C2_C1	"PLAYDOHop_FLDSI_S_C2_C1"
#define PLAYDOHopcode_FLDSI_S_C2_C2	"PLAYDOHop_FLDSI_S_C2_C2"
#define PLAYDOHopcode_FLDSI_S_C2_C3	"PLAYDOHop_FLDSI_S_C2_C3"
#define PLAYDOHopcode_FLDSI_S_C3_V1	"PLAYDOHop_FLDSI_S_C3_V1"
#define PLAYDOHopcode_FLDSI_S_C3_C1	"PLAYDOHop_FLDSI_S_C3_C1"
#define PLAYDOHopcode_FLDSI_S_C3_C2	"PLAYDOHop_FLDSI_S_C3_C2"
#define PLAYDOHopcode_FLDSI_S_C3_C3	"PLAYDOHop_FLDSI_S_C3_C3"

#define PLAYDOHopcode_FLDSI_D_V1_V1	"PLAYDOHop_FLDSI_D_V1_V1"
#define PLAYDOHopcode_FLDSI_D_V1_C1	"PLAYDOHop_FLDSI_D_V1_C1"
#define PLAYDOHopcode_FLDSI_D_V1_C2	"PLAYDOHop_FLDSI_D_V1_C2"
#define PLAYDOHopcode_FLDSI_D_V1_C3	"PLAYDOHop_FLDSI_D_V1_C3"
#define PLAYDOHopcode_FLDSI_D_C1_V1	"PLAYDOHop_FLDSI_D_C1_V1"
#define PLAYDOHopcode_FLDSI_D_C1_C1	"PLAYDOHop_FLDSI_D_C1_C1"
#define PLAYDOHopcode_FLDSI_D_C1_C2	"PLAYDOHop_FLDSI_D_C1_C2"
#define PLAYDOHopcode_FLDSI_D_C1_C3	"PLAYDOHop_FLDSI_D_C1_C3"
#define PLAYDOHopcode_FLDSI_D_C2_V1	"PLAYDOHop_FLDSI_D_C2_V1"
#define PLAYDOHopcode_FLDSI_D_C2_C1	"PLAYDOHop_FLDSI_D_C2_C1"
#define PLAYDOHopcode_FLDSI_D_C2_C2	"PLAYDOHop_FLDSI_D_C2_C2"
#define PLAYDOHopcode_FLDSI_D_C2_C3	"PLAYDOHop_FLDSI_D_C2_C3"
#define PLAYDOHopcode_FLDSI_D_C3_V1	"PLAYDOHop_FLDSI_D_C3_V1"
#define PLAYDOHopcode_FLDSI_D_C3_C1	"PLAYDOHop_FLDSI_D_C3_C1"
#define PLAYDOHopcode_FLDSI_D_C3_C2	"PLAYDOHop_FLDSI_D_C3_C2"
#define PLAYDOHopcode_FLDSI_D_C3_C3	"PLAYDOHop_FLDSI_D_C3_C3"

/* data verify loads */
#define PLAYDOHopcode_LDV_B		"PLAYDOHop_LDV_B"
#define PLAYDOHopcode_LDV_H		"PLAYDOHop_LDV_H"
#define PLAYDOHopcode_LDV_W		"PLAYDOHop_LDV_W"
#define PLAYDOHopcode_LDV_Q		"PLAYDOHop_LDV_Q"
#define PLAYDOHopcode_FLDV_S		"PLAYDOHop_FLDV_S"
#define PLAYDOHopcode_FLDV_D		"PLAYDOHop_FLDV_D"

/*
 * PlayDoh int ALU op extensions
 */
#define PLAYDOHopcode_SH1ADDL		"PLAYDOHop_SH1ADDL"
#define PLAYDOHopcode_SH2ADDL		"PLAYDOHop_SH2ADDL"
#define PLAYDOHopcode_SH3ADDL		"PLAYDOHop_SH3ADDL"

/*
 * PlayDoh FP ALU op extensions
 */
#define PLAYDOHopcode_FMPYADDN_S	"PLAYDOHop_FMPYADDN_S"
#define PLAYDOHopcode_FMPYADDN_D	"PLAYDOHop_FMPYADDN_D"

/*
 * Playdoh int comparison extensions
 */
#define PLAYDOHopcode_CMPR_FALSE	"PLAYDOHop_CMPR_FALSE"
#define PLAYDOHopcode_CMPR_TRUE		"PLAYDOHop_CMPR_TRUE"
#define PLAYDOHopcode_CMPR_OD		"PLAYDOHop_CMPR_OD"
#define PLAYDOHopcode_CMPR_EV		"PLAYDOHop_CMPR_EV"
#define PLAYDOHopcode_CMPR_SV		"PLAYDOHop_CMPR_SV"
#define PLAYDOHopcode_CMPR_NSV		"PLAYDOHop_CMPR_NSV"

/*
 * Playdoh FP comparison extensions - Note unordered compares ala
 * PA-RISC are not supported by the playdoh code generator because
 * they are silly and pretty much only useful for assembly language
 * programmers.
 */
#define PLAYDOHopcode_FCMPR_S_FALSE	"PLAYDOHop_FCMPR_S_FALSE"
#define PLAYDOHopcode_FCMPR_S_TRUE	"PLAYDOHop_FCMPR_S_TRUE"
#define PLAYDOHopcode_FCMPR_D_FALSE	"PLAYDOHop_FCMPR_D_FALSE"
#define PLAYDOHopcode_FCMPR_D_TRUE	"PLAYDOHop_FCMPR_D_TRUE"

/*
 *	Multi-cycle no-op.  Added SAM 10-96
 */
#define PLAYDOHopcode_M_NO_OP		"PLAYDOHop_M_NO_OP"

/*
 *      Playdoh literal forming operations
 */
#define PLAYDOHopcode_MOVELB          "PLAYDOHop_MOVELB"
#define PLAYDOHopcode_MOVELBX         "PLAYDOHop_MOVELBX"
#define PLAYDOHopcode_MOVELBS         "PLAYDOHop_MOVELBS"
#define PLAYDOHopcode_PBRRL           "PLAYDOHop_PBRRL"
#define PLAYDOHopcode_PBRAL           "PLAYDOHop_PBRAL"
#define PLAYDOHopcode_PBRRLBS         "PLAYDOHop_PBRRLBS"
#define PLAYDOHopcode_PBRALBS         "PLAYDOHop_PBRALBS"
#define PLAYDOHopcode_MOVELG          "PLAYDOHop_MOVELG"
#define PLAYDOHopcode_MOVELGX         "PLAYDOHop_MOVELGX"
#define PLAYDOHopcode_MOVELGS         "PLAYDOHop_MOVELGS"
#define PLAYDOHopcode_MOVELF          "PLAYDOHop_MOVELF"
#define PLAYDOHopcode_MOVELFS         "PLAYDOHop_MOVELFS"

#define PLAYDOHopcode_MOVEGC          "PLAYDOHop_MOVEGC"
#define PLAYDOHopcode_MOVECG          "PLAYDOHop_MOVECG"
#define PLAYDOHopcode_MOVEGG          "PLAYDOHop_MOVEGG"
#define PLAYDOHopcode_MOVEBB          "PLAYDOHop_MOVEBB"

/*
 *      Regalloc operations
 */
#define PLAYDOHopcode_SAVE            "PLAYDOHop_SAVE"
#define PLAYDOHopcode_RESTORE         "PLAYDOHop_RESTORE"
#define PLAYDOHopcode_FSAVE_S         "PLAYDOHop_FSAVE_S"
#define PLAYDOHopcode_FRESTORE_S      "PLAYDOHop_FRESTORE_S"
#define PLAYDOHopcode_FSAVE_D         "PLAYDOHop_FSAVE_D"
#define PLAYDOHopcode_FRESTORE_D      "PLAYDOHop_FRESTORE_D"
#define PLAYDOHopcode_BSAVE           "PLAYDOHop_BSAVE"
#define PLAYDOHopcode_BRESTORE        "PLAYDOHop_BRESTORE"
#define PLAYDOHopcode_PSAVE           "PLAYDOHop_PSAVE"
#define PLAYDOHopcode_PRESTORE        "PLAYDOHop_PRESTORE"
#define PLAYDOHopcode_SAVEG            "PLAYDOHop_SAVEG"
#define PLAYDOHopcode_RESTOREG         "PLAYDOHop_RESTOREG"
#define PLAYDOHopcode_FSAVEG_S         "PLAYDOHop_FSAVEG_S"
#define PLAYDOHopcode_FRESTOREG_S      "PLAYDOHop_FRESTOREG_S"
#define PLAYDOHopcode_FSAVEG_D         "PLAYDOHop_FSAVEG_D"
#define PLAYDOHopcode_FRESTOREG_D      "PLAYDOHop_FRESTOREG_D"
#define PLAYDOHopcode_BSAVEG           "PLAYDOHop_BSAVEG"
#define PLAYDOHopcode_BRESTOREG        "PLAYDOHop_BRESTOREG"
#define PLAYDOHopcode_PSAVEG           "PLAYDOHop_PSAVEG"
#define PLAYDOHopcode_PRESTOREG        "PLAYDOHop_PRESTOREG"
#define PLAYDOHopcode_MOVEGBP         "PLAYDOHop_MOVEGBP"
#define PLAYDOHopcode_MOVEGCM         "PLAYDOHop_MOVEGCM"

/*
 *      Local memory loads and stores
 *              Currently set up for 4 local memories!
 */
#define PLAYDOHopcode_LL_B_L1           "PLAYDOH_op_LL_B_L1"
#define PLAYDOHopcode_LL_B_L2           "PLAYDOH_op_LL_B_L2"
#define PLAYDOHopcode_LL_B_L3           "PLAYDOH_op_LL_B_L3"
#define PLAYDOHopcode_LL_B_L4           "PLAYDOH_op_LL_B_L4"

#define PLAYDOHopcode_LL_H_L1           "PLAYDOH_op_LL_H_L1"
#define PLAYDOHopcode_LL_H_L2           "PLAYDOH_op_LL_H_L2"
#define PLAYDOHopcode_LL_H_L3           "PLAYDOH_op_LL_H_L3"
#define PLAYDOHopcode_LL_H_L4           "PLAYDOH_op_LL_H_L4"

#define PLAYDOHopcode_LL_W_L1           "PLAYDOH_op_LL_W_L1"
#define PLAYDOHopcode_LL_W_L2           "PLAYDOH_op_LL_W_L2"
#define PLAYDOHopcode_LL_W_L3           "PLAYDOH_op_LL_W_L3"
#define PLAYDOHopcode_LL_W_L4           "PLAYDOH_op_LL_W_L4"

#define PLAYDOHopcode_LL_Q_L1           "PLAYDOH_op_LL_Q_L1"
#define PLAYDOHopcode_LL_Q_L2           "PLAYDOH_op_LL_Q_L2"
#define PLAYDOHopcode_LL_Q_L3           "PLAYDOH_op_LL_Q_L3"
#define PLAYDOHopcode_LL_Q_L4           "PLAYDOH_op_LL_Q_L4"

#define PLAYDOHopcode_LLX_B_L1          "PLAYDOH_op_LLX_B_L1"
#define PLAYDOHopcode_LLX_B_L2          "PLAYDOH_op_LLX_B_L2"
#define PLAYDOHopcode_LLX_B_L3          "PLAYDOH_op_LLX_B_L3"
#define PLAYDOHopcode_LLX_B_L4          "PLAYDOH_op_LLX_B_L4"

#define PLAYDOHopcode_LLX_H_L1          "PLAYDOH_op_LLX_H_L1"
#define PLAYDOHopcode_LLX_H_L2          "PLAYDOH_op_LLX_H_L2"
#define PLAYDOHopcode_LLX_H_L3          "PLAYDOH_op_LLX_H_L3"
#define PLAYDOHopcode_LLX_H_L4		"PLAYDOH_op_LLX_H_L4"

#define PLAYDOHopcode_LLX_W_L1          "PLAYDOH_op_LLX_W_L1"
#define PLAYDOHopcode_LLX_W_L2          "PLAYDOH_op_LLX_W_L2"
#define PLAYDOHopcode_LLX_W_L3          "PLAYDOH_op_LLX_W_L3"
#define PLAYDOHopcode_LLX_W_L4          "PLAYDOH_op_LLX_W_L4"

#define PLAYDOHopcode_LLX_Q_L1          "PLAYDOH_op_LLX_Q_L1"
#define PLAYDOHopcode_LLX_Q_L2          "PLAYDOH_op_LLX_Q_L2"
#define PLAYDOHopcode_LLX_Q_L3          "PLAYDOH_op_LLX_Q_L3"
#define PLAYDOHopcode_LLX_Q_L4          "PLAYDOH_op_LLX_Q_L4"

#define PLAYDOHopcode_FLL_S_L1          "PLAYDOH_op_FLL_S_L1"
#define PLAYDOHopcode_FLL_S_L2          "PLAYDOH_op_FLL_S_L2"
#define PLAYDOHopcode_FLL_S_L3          "PLAYDOH_op_FLL_S_L3"
#define PLAYDOHopcode_FLL_S_L4          "PLAYDOH_op_FLL_S_L4"

#define PLAYDOHopcode_FLL_D_L1          "PLAYDOH_op_FLL_D_L1"
#define PLAYDOHopcode_FLL_D_L2          "PLAYDOH_op_FLL_D_L2"
#define PLAYDOHopcode_FLL_D_L3          "PLAYDOH_op_FLL_D_L3"
#define PLAYDOHopcode_FLL_D_L4          "PLAYDOH_op_FLL_D_L4"

#define PLAYDOHopcode_SL_B_L1           "PLAYDOH_op_SL_B_L1"
#define PLAYDOHopcode_SL_B_L2           "PLAYDOH_op_SL_B_L2"
#define PLAYDOHopcode_SL_B_L3           "PLAYDOH_op_SL_B_L3"
#define PLAYDOHopcode_SL_B_L4           "PLAYDOH_op_SL_B_L4"

#define PLAYDOHopcode_SL_H_L1           "PLAYDOH_op_SL_H_L1"
#define PLAYDOHopcode_SL_H_L2           "PLAYDOH_op_SL_H_L2"
#define PLAYDOHopcode_SL_H_L3           "PLAYDOH_op_SL_H_L3"
#define PLAYDOHopcode_SL_H_L4           "PLAYDOH_op_SL_H_L4"

#define PLAYDOHopcode_SL_W_L1           "PLAYDOH_op_SL_W_L1"
#define PLAYDOHopcode_SL_W_L2           "PLAYDOH_op_SL_W_L2"
#define PLAYDOHopcode_SL_W_L3           "PLAYDOH_op_SL_W_L3"
#define PLAYDOHopcode_SL_W_L4           "PLAYDOH_op_SL_W_L4"

#define PLAYDOHopcode_SL_Q_L1           "PLAYDOH_op_SL_Q_L1"
#define PLAYDOHopcode_SL_Q_L2           "PLAYDOH_op_SL_Q_L2"
#define PLAYDOHopcode_SL_Q_L3           "PLAYDOH_op_SL_Q_L3"
#define PLAYDOHopcode_SL_Q_L4           "PLAYDOH_op_SL_Q_L4"

#define PLAYDOHopcode_FSL_S_L1          "PLAYDOH_op_FSL_S_L1"
#define PLAYDOHopcode_FSL_S_L2          "PLAYDOH_op_FSL_S_L2"
#define PLAYDOHopcode_FSL_S_L3          "PLAYDOH_op_FSL_S_L3"
#define PLAYDOHopcode_FSL_S_L4          "PLAYDOH_op_FSL_S_L4"

#define PLAYDOHopcode_FSL_D_L1          "PLAYDOH_op_FSL_D_L1"
#define PLAYDOHopcode_FSL_D_L2          "PLAYDOH_op_FSL_D_L2"
#define PLAYDOHopcode_FSL_D_L3          "PLAYDOH_op_FSL_D_L3"
#define PLAYDOHopcode_FSL_D_L4          "PLAYDOH_op_FSL_D_L4"

/* SLARSEN: Vector extract ops */
#define PLAYDOHopcode_VEXTRSB		"PLAYDOHop_VEXTRSB"
#define PLAYDOHopcode_VEXTRSH		"PLAYDOHop_VEXTRSH"

/* SLARSEN: Vector memory ops */
#define PLAYDOHopcode_VL_B_C1_C1	"PLAYDOHop_VL_B_C1_C1"
#define PLAYDOHopcode_VL_H_C1_C1	"PLAYDOHop_VL_H_C1_C1"
#define PLAYDOHopcode_VL_W_C1_C1	"PLAYDOHop_VL_W_C1_C1"
#define PLAYDOHopcode_VFL_S_C1_C1	"PLAYDOHop_VFL_S_C1_C1"
#define PLAYDOHopcode_VFL_D_C1_C1	"PLAYDOHop_VFL_D_C1_C1"
#define PLAYDOHopcode_VS_B_C1		"PLAYDOHop_VS_B_C1"
#define PLAYDOHopcode_VS_H_C1		"PLAYDOHop_VS_H_C1"
#define PLAYDOHopcode_VS_W_C1		"PLAYDOHop_VS_W_C1"
#define PLAYDOHopcode_VFS_S_C1		"PLAYDOHop_VFS_S_C1"
#define PLAYDOHopcode_VFS_D_C1		"PLAYDOHop_VFS_D_C1"

#define PLAYDOHopcode_VLE_B_C1_C1	"PLAYDOHop_VLE_B_C1_C1"
#define PLAYDOHopcode_VLE_H_C1_C1	"PLAYDOHop_VLE_H_C1_C1"
#define PLAYDOHopcode_VLE_W_C1_C1	"PLAYDOHop_VLE_W_C1_C1"
#define PLAYDOHopcode_VFLE_S_C1_C1	"PLAYDOHop_VFLE_S_C1_C1"
#define PLAYDOHopcode_VFLE_D_C1_C1	"PLAYDOHop_VFLE_D_C1_C1"
#define PLAYDOHopcode_VSE_B_C1		"PLAYDOHop_VSE_B_C1"
#define PLAYDOHopcode_VSE_H_C1		"PLAYDOHop_VSE_H_C1"
#define PLAYDOHopcode_VSE_W_C1		"PLAYDOHop_VSE_W_C1"
#define PLAYDOHopcode_VFSE_S_C1		"PLAYDOHop_VFSE_S_C1"
#define PLAYDOHopcode_VFSE_D_C1		"PLAYDOHop_VFSE_D_C1"

/*=========================================================================*/
/*
 *	Function prototypes
 */
/*=========================================================================*/

#ifdef __cplusplus
extern "C"
{
#endif

  extern int M_playdoh_model;

  extern int M_playdoh_type_size (int mtype);
  extern int M_playdoh_type_align (int mtype);
  extern void M_playdoh_void (M_Type type);
  extern void M_playdoh_bit_long (M_Type type, int n);
  extern void M_playdoh_bit_int (M_Type type, int n);
  extern void M_playdoh_bit_short (M_Type type, int n);
  extern void M_playdoh_bit_char (M_Type type, int n);
  extern void M_playdoh_char (M_Type type, int unsign);
  extern void M_playdoh_short (M_Type type, int unsign);
  extern void M_playdoh_int (M_Type type, int unsign);
  extern void M_playdoh_long (M_Type type, int unsign);
  extern void M_playdoh_llong (M_Type type, int unsign);
  extern void M_playdoh_float (M_Type type, int unsign);
  extern void M_playdoh_double (M_Type type, int unsign);
  extern void M_playdoh_pointer (M_Type type);
  extern int M_playdoh_eval_type (M_Type type, M_Type ntype);
  extern int M_playdoh_eval_type2 (M_Type type, M_Type ntype);
  extern int M_playdoh_call_type (M_Type type, M_Type ntype);
  extern int M_playdoh_call_type2 (M_Type type, M_Type ntype);
  extern void M_playdoh_array_layout (M_Type type, int *offset);
  extern int M_playdoh_array_align (M_Type type);
  extern int M_playdoh_array_size (M_Type type, int dim);
  extern void M_playdoh_union_layout (int n, _M_Type * type, int *offset,
				      int *bit_offset);
  extern int M_playdoh_union_align (int n, _M_Type * type);
  extern int M_playdoh_union_size (int n, _M_Type * type);
  extern void M_playdoh_struct_layout (int n, _M_Type * type, int *base,
				       int *bit_offset);
  extern int M_playdoh_struct_align (int n, _M_Type * type);
  extern int M_playdoh_struct_size (int n, _M_Type * type, int struct_align);
  extern int M_playdoh_layout_fnvar (List param_list, char **base_macro,
				     int *pcount, int purpose);
  extern int M_playdoh_fnvar_layout (int n, _M_Type * type, long int *offset,
				     int *mode, int *reg, int *paddr,
				     char **macro, int *su_sreg, int *su_ereg,
				     int *pcount, int is_st, int purpose);
  extern int M_playdoh_lvar_layout (int n, _M_Type * type, long int *offset,
				    char **base_macro);
  extern int M_playdoh_no_short_int (void);
  extern int M_playdoh_layout_order (void);
  extern void M_playdoh_cb_label_name (char *fn, int cb, char *line, int len);
  extern int M_playdoh_is_cb_label (char *label, char *fn, int *cb);
  extern void M_playdoh_jumptbl_label_name (char *fn, int tbl_id, char *line,
					    int len);
  extern int M_playdoh_is_jumptbl_label (char *label, char *fn, int *tbl_id);
  extern int M_playdoh_structure_pointer (int purpose);
  extern int M_playdoh_return_register (int type, int purpose);
  extern char *M_playdoh_fn_label_name (char *label,
					int (*is_func) (char *is_func_label));
  extern char *M_playdoh_fn_name_from_label (char *label);
  extern void M_set_model_playdoh (char *model_name);
  extern int M_playdoh_fragile_macro (int macro_value);
  extern Set M_playdoh_fragile_macro_set (void);
  extern int M_playdoh_dataflow_macro (int id);
  extern int M_playdoh_subroutine_call (int opc);
  extern void M_define_macros_playdoh (STRING_Symbol_Table * sym_tbl);
  extern char *M_get_macro_name_playdoh (int id);
  extern void M_define_opcode_name_playdoh (STRING_Symbol_Table * sym_tbl);
  extern char *M_get_opcode_name_playdoh (int id);
  extern int M_oper_supported_in_arch_playdoh (int opc);
  extern int M_num_oper_required_for_playdoh (L_Oper * oper, char *name);
  extern int M_num_registers_playdoh (int ctype);
  extern int M_is_stack_operand_playdoh (L_Operand * operand);
  extern int M_is_unsafe_macro_playdoh (L_Operand * operand);
  extern int M_operand_type_playdoh (L_Operand * operand);
  extern int M_conflicting_operands_playdoh (L_Operand * operand,
					     L_Operand ** conflict_array,
					     int len, int prepass);
  extern int M_opc_from_proc_opc_playdoh (int proc_opc);
  extern int M_base_displ_load_opcode (int proc_opc);
  extern int M_sign_extend_load_opcode (int proc_opc);
  extern int M_base_displ_store_opcode (int proc_opc);

#ifdef __cplusplus
}
#endif

#endif
