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
 *  File:  m_tahoe.h
 *
 *  Description: Defines symbols used for reference in Mspec and code
 * 	 	generator.
 *
 *  Authors: Dan Connors and Jim Pierce
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
/* 08/30/02 REK Adding TAHOEop_START to hold the value of the first opcode
 *              constant.
 */
/* 09/10/02 REK Changing the TAHOEop defines to match the new breakdown.
 */

#ifndef _M_TAHOE_H_
#define _M_TAHOE_H_

/* 10/29/02 REK Adding config.h */
#include <config.h>
#include  "m_spec.h"

/* MCM 7/2000 As per 7-1 of the software conventions guide */

#define SCRATCH_SPACE_SIZE 16

#define TAHOE_NUM_INT_REG                  (128)
#define TAHOE_NUM_INT_STATIC_REG           (32)
#define TAHOE_NUM_INT_STACKED_REG          (TAHOE_NUM_INT_REG - \
                                            TAHOE_NUM_INT_STATIC_REG)
#define TAHOE_NUM_PRED_REG                 (64)
#define TAHOE_NUM_FLOAT_REG                (128)
#define TAHOE_NUM_BRANCH_REG               (8)

#define TAHOE_MIN_INT_REGISTER_ID          (0)
#define TAHOE_MAX_INT_REGISTER_ID          (TAHOE_NUM_INT_REG-1)
#define TAHOE_MIN_INT_STATIC_REGISTER_ID   (0)
#define TAHOE_MAX_INT_STATIC_REGISTER_ID   (TAHOE_NUM_INT_STATIC_REG-1)
#define TAHOE_MIN_INT_STACKED_REGISTER_ID  (32)
#define TAHOE_MAX_INT_STACKED_REGISTER_ID  (TAHOE_MAX_INT_REGISTER_ID)
#define TAHOE_MAX_PRED_REGISTER_NUMBER     (TAHOE_NUM_PRED_REG-1)
#define TAHOE_MAX_FLOAT_REGISTER_NUMBER    (TAHOE_NUM_FLOAT_REG-1)
#define TAHOE_MAX_BRANCH_REGISTER_NUMBER   (TAHOE_NUM_BRANCH_REG-1)

#define TAHOE_NUM_SPECIAL_INT_REG          (2)
				                /* 1 - PRED_SAVE
				                 * 2 - GP_SAVE
				                 */
#define TAHOE_NUM_SPECIAL_PRED_REG         (0)
#define TAHOE_NUM_SPECIAL_FLOAT_REG        (0)
#define TAHOE_NUM_SPECIAL_BRANCH_REG       (0)

#define TAHOE_INT_REG_BASE                 (0)
#define TAHOE_INT_STACK_REG_BASE (TAHOE_INT_REG_BASE + \
                                  TAHOE_NUM_INT_STATIC_REG)
#define TAHOE_INT_SPILL_REG_BASE (TAHOE_INT_REG_BASE + TAHOE_NUM_INT_REG - \
                                  TAHOE_NUM_SPILL_INT_REG)
#define TAHOE_FLOAT_REG_BASE     (TAHOE_INT_REG_BASE + TAHOE_NUM_INT_REG)
#define TAHOE_PRED_REG_BASE      (TAHOE_FLOAT_REG_BASE + TAHOE_NUM_FLOAT_REG)
#define TAHOE_BRANCH_REG_BASE    (TAHOE_PRED_REG_BASE + TAHOE_NUM_PRED_REG)

/* Macro used in m_tahoe.c and Ltahoe codegen */

#define ADDR_ALIGN(addr, align) ( ((addr)+(align-1)) & ~(align-1) )

/*
 * IPF processor models
 * ----------------------------------------------------------------------
 */

enum {
  M_IPF_ITANIUM = 0,
  M_IPF_MCKINLEY,
  M_IPF_MADISON,
  M_IPF_DEERFIELD,
  M_IPF_MENDOCITO,
  M_IPF_CHIVANO
};

#define M_IPF_MODEL_OK( model ) (((model) == M_IPF_ITANIUM) ||  \
			         ((model) == M_IPF_MCKINLEY) || \
			         ((model) == M_IPF_MADISON) ||  \
			         ((model) == M_IPF_DEERFIELD) || \
			         ((model) == M_IPF_MENDOCITO) || \
			         ((model) == M_IPF_CHIVANO))

/*
 * IPF software architecture
 * ----------------------------------------------------------------------
 */

enum {
  M_IPF_LIN_LP64 = 0,
  M_IPF_WIN_P64,
  M_IPF_HPUX_LP64,
  M_IPF_HPUX_ILP32
};

#define M_IPF_SWARCH_OK( model ) (((model) == M_IPF_LIN_LP64) ||  \
			          ((model) == M_IPF_WIN_P64) ||   \
			          ((model) == M_IPF_HPUX_LP64) || \
			          ((model) == M_IPF_HPUX_ILP32))

/*
 * IPF macro registers
 * ----------------------------------------------------------------------
 */

enum
{
  /* These need to be numbered starting after L_MAC_LAST in l_code.h
     since there is no way to distinguish better Lcode and machine specific
     macros. */
  TAHOE_MAC_ZERO = (L_MAC_LAST + 1),	/* R0                             */
  TAHOE_MAC_GP,			/* data pointer  (R1)                     */
  TAHOE_MAC_AP,			/* argument pointer  (R9)                 */
  TAHOE_MAC_RETADDR,		/* return address register                */
  TAHOE_MAC_PRED_TRUE,		/* P0                                     */
  TAHOE_MAC_FZERO,		/* Floating point register zero = 0.0     */
  TAHOE_MAC_FONE,		/* Floating point register one = 1.0      */

  TAHOE_PRED_SAVE_REG,
  TAHOE_PRED_BLK_REG,		/* pr                                     */
  TAHOE_PRED_ROT_BLK_REG,       /* pr.rot                                 */

  TAHOE_GP_SAVE_REG,

  TAHOE_MAC_TMPREG1,		/* r2 - Used for temp storage in phase2   */
  TAHOE_MAC_TMPREG2,		/* r3 - Used for temp storage in phase2   */

  TAHOE_MAC_LEAF,		/* 1 if leaf fn, 0 if not leaf function   */
  TAHOE_MAC_SYNC_SIZE,		/* size in bytes in LV space on stack     *
				 * used to spill integers around syncs    *
				 * such as setjmp                         */
  TAHOE_MAC_MEM_ALLOC,		/* total alloc space requirements         */
  TAHOE_MAC_TEMPLATE,
  TAHOE_MAC_LABEL,

  TAHOE_MAC_PSP,		/* Previous stack pointer (for alloca)    */

  TAHOE_MAC_PSPILL,             /* Predicate spill section pointer        */

/* Application Registers - If these change, update application register
   macro check TAHOE_APPLICATION_MACRO( macro ).  These have been
   ordered by their application register number.*/

  /* M-type */
  TAHOE_MAC_KR0,		/* Kernel Register 0                      */
  TAHOE_MAC_KR1,		/* Kernel Register 1                      */
  TAHOE_MAC_KR2,		/* Kernel Register 2                      */
  TAHOE_MAC_KR3,		/* Kernel Register 3                      */
  TAHOE_MAC_RSC,		/* Register Stack Configuration Register  */
  TAHOE_MAC_BSP,		/* Backing Store Pointer for reg stack    */
  TAHOE_MAC_RNAT,		/* RSE NAT Collection register            */
  TAHOE_MAC_CCV,		/* Cmp/Exch Value Register                */
  TAHOE_MAC_UNAT,		/* User NAT Collection register           */
  TAHOE_MAC_FPSR,		/* Floating Point Status Register         */
  TAHOE_MAC_ITC,		/* Interval Time Counter                  */
  /* I-type */
  TAHOE_MAC_AR_PFS,		/* Previous Frame State                   */
  TAHOE_MAC_LC,			/* Loop count                             */
  TAHOE_MAC_EC			/* Epilogue stage counter                 */
};

#define TAHOE_APPLICATION_MACRO(macro)   ((TAHOE_I_APPLICATION_MACRO(macro)|| \
				           TAHOE_M_APPLICATION_MACRO(macro)))

#define TAHOE_M_APPLICATION_MACRO(macro) ((macro->value.mac >= TAHOE_MAC_KR0) \
                                          &&                                  \
                                          (macro->value.mac <= TAHOE_MAC_ITC))

#define TAHOE_I_APPLICATION_MACRO(macro)                                      \
                                   ((macro->value.mac >= TAHOE_MAC_AR_PFS) && \
				    (macro->value.mac <= TAHOE_MAC_EC))

/*========================================================================*/
/*
 *    parameter and return value offsets
 */
/*========================================================================*/
/* 
 * 
 * Integers         P #'s
 * Using the register stack engine
 * 8 input parameters      0-7
 * 8 output parameters     8-15
 * 1 return (in static)    16 
 * 
 * Floats               P #'s
 * 
 * Don't use the register stack engine
 * 8 parameter registers   17-25
 * 1 return value          26
 * 
 */

/* Number of integer param registers */
#define M_TAHOE_MAX_FNVAR_INT_REG	 8

/* Number of float param registers */
#define M_TAHOE_MAX_FNVAR_FLT_REG	 8

/* Number of float param registers */
#define M_TAHOE_MAX_RET_REG	         4

/* parameter registers         */
#define M_TAHOE_SMALL_STRUCT_MAX        64


/* incoming and outgoing parameters */
#define M_TAHOE_IN_INT_BASE              0	/* Stacked Int Registers */
#define M_TAHOE_OUT_INT_BASE             8	/* Stacked Int Registers */

#define M_TAHOE_RET_INT_BASE	       	16	/* Non-stacked Int registers */

#define M_TAHOE_FLT_BASE		20	/* Non-stacked Fp registers */

#define M_TAHOE_RET_FLT_BASE		20	/* Non-stacked Fp registers */

/*
 * IPF opcodes
 * ----------------------------------------------------------------------
 */

#define TAHOEop_VERSION         1

/* 08/30/02 REK This constant is the lowest numbered opcode in the list. */
#define TAHOEop_START           1000

/* useless opcodes */

#define TAHOEop_INVALID    	   1000

#define TAHOEop_NOP_B              1001   /* Various NOPs */
#define TAHOEop_NOP_F              1002
#define TAHOEop_NOP_I              1003
#define TAHOEop_NOP_M              1004
#define TAHOEop_NOP_X              1005

#define TAHOEop_BREAK_B            1006   /* Various break instructions */
#define TAHOEop_BREAK_F            1007
#define TAHOEop_BREAK_I            1008
#define TAHOEop_BREAK_M            1009
#define TAHOEop_BREAK_X            1010

#define TAHOEop_ADD                1011   /* Register add */
#define TAHOEop_ADDL               1012   /* Add imm22 to either R0,R1,R2,R3 */
#define TAHOEop_ADDS               1013   /* Add imm15 */
#define TAHOEop_SHLADD             1014   /* left shift and add */
#define TAHOEop_SUB                1015   /* Subtract */
#define TAHOEop_AND                1016   /* Logical AND */
#define TAHOEop_ANDCM              1017   /* Logical AND Complement */
#define TAHOEop_OR                 1018   /* Logical OR */
#define TAHOEop_XOR                1019   /* Logical Exclusive OR */

#define TAHOEop_DEP                1020   /* Deposit */
#define TAHOEop_DEP_Z              1021   /* Deposit and zero clear */
#define TAHOEop_EXTR               1022   /* Extract and sign extend */
#define TAHOEop_EXTR_U             1023   /* Extract and no sign extend */
#define TAHOEop_SHRP               1024   /* Shift right pair */

#define TAHOEop_SXT1               1025   /* Sign extend - 1 byte */
#define TAHOEop_SXT2               1026   /* Sign extend - 2 bytes */
#define TAHOEop_SXT4               1027   /* Sign extend - 4 bytes */
#define TAHOEop_ZXT1               1028   /* Zero extend - 1 byte */
#define TAHOEop_ZXT2               1029   /* Zero extend - 2 bytes */
#define TAHOEop_ZXT4               1030   /* Zero extend - 4 bytes */

#define TAHOEop_CZX1_L             1031   /* Compute zero index */
#define TAHOEop_CZX1_R             1032
#define TAHOEop_CZX2_L             1033
#define TAHOEop_CZX2_R             1034

#define TAHOEop_LD1                1035   /* Load 1 byte */
#define TAHOEop_LD1_ACQ            1036   /* Load 1 byte with acquire */
#define TAHOEop_LD1_BIAS           1037   
#define TAHOEop_LD2                1038
#define TAHOEop_LD2_ACQ            1039
#define TAHOEop_LD2_BIAS           1040
#define TAHOEop_LD4                1041
#define TAHOEop_LD4_ACQ            1042
#define TAHOEop_LD4_BIAS           1043
#define TAHOEop_LD8                1044
#define TAHOEop_LD8_ACQ            1045
#define TAHOEop_LD8_BIAS           1046
#define TAHOEop_LD8_FILL           1047   /* Load 8 bytes, fill */

#define TAHOEop_LFETCH             1048   /* Line prefetch */
#define TAHOEop_LFETCH_FAULT       1049

#define TAHOEop_LDF_FILL           1050

#define TAHOEop_LDF8               1051   /* Floating point loads */
#define TAHOEop_LDF8_A             1052
#define TAHOEop_LDFD               1053
#define TAHOEop_LDFD_A             1054
#define TAHOEop_LDFE               1055
#define TAHOEop_LDFE_A             1056
#define TAHOEop_LDFS               1057
#define TAHOEop_LDFS_A             1058
#define TAHOEop_LDFP8              1059
#define TAHOEop_LDFPD              1060
#define TAHOEop_LDFPS              1061

#define TAHOEop_LD1_C              1062   /* Load check opcodes */
#define TAHOEop_LD2_C              1063
#define TAHOEop_LD4_C              1064
#define TAHOEop_LD8_C              1065
#define TAHOEop_LDF8_C             1066
#define TAHOEop_LDFD_C             1067
#define TAHOEop_LDFE_C             1068
#define TAHOEop_LDFP8_C            1069
#define TAHOEop_LDFPD_C            1070
#define TAHOEop_LDFPS_C            1071
#define TAHOEop_LDFS_C             1072

#define TAHOEop_CHK_A              1073   /* Speculation check opcodes */
#define TAHOEop_CHK_S_I            1074
#define TAHOEop_CHK_S_F            1075
#define TAHOEop_CHK_S_M            1076

#define TAHOEop_XMA_H              1077   /* Fixed point multiply add */
#define TAHOEop_XMA_HU             1078
#define TAHOEop_XMA_L              1079

#define TAHOEop_FMA_D              1080   /* Floating point multiply add */
#define TAHOEop_FMA_S              1081
#define TAHOEop_FMA                1082

#define TAHOEop_FMS_D              1083   /* Floating point multiply sub */
#define TAHOEop_FMS_S              1084
#define TAHOEop_FMS                1085

#define TAHOEop_FNMA_D             1086   /* FP negate multiply add */
#define TAHOEop_FNMA_S             1087   
#define TAHOEop_FNMA               1088

#define TAHOEop_FAMAX              1089   /* FP absolute max */
#define TAHOEop_FAMIN              1090   /* FP absolute min */
#define TAHOEop_FAND               1091   /* FP logical and */
#define TAHOEop_FANDCM             1092   /* FP logical and complement */
#define TAHOEop_FMAX               1093   /* FP max */
#define TAHOEop_FMERGE_NS          1094   /* FP merge */
#define TAHOEop_FMERGE_S           1095
#define TAHOEop_FMERGE_SE          1096
#define TAHOEop_FMIN               1097   /* FP min */
#define TAHOEop_FMIX_L             1098   /* FP mix */
#define TAHOEop_FMIX_LR            1099
#define TAHOEop_FMIX_R             1100
#define TAHOEop_FOR                1101   /* FP logical or */
#define TAHOEop_FPACK              1102   /* FP pack */
#define TAHOEop_FRCPA              1103   /* FP reciprocial approx */
#define TAHOEop_FRSQRTA            1104   /* FP square root */
#define TAHOEop_FSELECT            1105   /* FP select */
#define TAHOEop_FSWAP              1106   /* FP swap */
#define TAHOEop_FSWAP_NL           1107
#define TAHOEop_FSWAP_NR           1108
#define TAHOEop_FSXT_L             1109   /* FP sign extend */
#define TAHOEop_FSXT_R             1110
#define TAHOEop_FXOR               1111   /* FP exclusive or */
#define TAHOEop_FCHKF              1112   /* FP check flags */
#define TAHOEop_FCLRF              1113   /* FP clear flags */
#define TAHOEop_FSETC              1114   /* FP set controls */

#define TAHOEop_MOV_TOAR_I         1115   /* Move to application reg >= 64 */
#define TAHOEop_MOV_FRAR_I         1116   /* Move from application reg >= 64 */
#define TAHOEop_MOV_TOAR_M         1117   /* Move to application reg < 64 */
#define TAHOEop_MOV_FRAR_M         1118   /* Move from application reg < 64 */
#define TAHOEop_MOV_FRCR           1119   /* Move from control register */
#define TAHOEop_MOV_TOCR           1120   /* Move to control register */
#define TAHOEop_MOV_FRIP           1121   /* Move from instruction pointer */
#define TAHOEop_MOV_FRPR           1122   /* Move from predicate register */
#define TAHOEop_MOV_TOPR           1123   /* Move to predicate register */
#define TAHOEop_MOV_TOPR_ROT       1124   /* Move to pred rotating reg */

#define TAHOEop_SETF_D             1125   /* Set FP value (double) */
#define TAHOEop_SETF_EXP           1126
#define TAHOEop_SETF_S             1127
#define TAHOEop_SETF_SIG           1128
#define TAHOEop_GETF_D             1129   /* Get FP value (double) */
#define TAHOEop_GETF_EXP           1130
#define TAHOEop_GETF_S             1131
#define TAHOEop_GETF_SIG           1132

#define TAHOEop_MOV_FRBR           1133   /* Move from branch register */
#define TAHOEop_MOV_TOBR           1134   /* Move to branch register */
#define TAHOEop_MOV_TOSYST         1135
#define TAHOEop_MOV_FRSYST         1136
#define TAHOEop_MOVL               1137   /* Move long immediate */

#define TAHOEop_CMP                1138   /* Integer comparison */
#define TAHOEop_TBIT               1139   /* Test bit */
#define TAHOEop_TNAT               1140

#define TAHOEop_BR_CALL            1141   /* Branch opcodes */
#define TAHOEop_BR_COND            1142
#define TAHOEop_BR_IA              1143
#define TAHOEop_BR_RET             1144
#define TAHOEop_BRL_CALL           1145
#define TAHOEop_BRL_COND           1146
#define TAHOEop_BRP                1147
#define TAHOEop_BRP_RET            1148
#define TAHOEop_BR_CEXIT           1149
#define TAHOEop_BR_CLOOP           1150
#define TAHOEop_BR_CTOP            1151
#define TAHOEop_BR_WEXIT           1152
#define TAHOEop_BR_WTOP            1153

#define TAHOEop_PADD1              1154   /* Parallel add */
#define TAHOEop_PADD1_SSS          1155
#define TAHOEop_PADD1_UUS          1156
#define TAHOEop_PADD1_UUU          1157
#define TAHOEop_PADD2              1158
#define TAHOEop_PADD2_SSS          1159
#define TAHOEop_PADD2_UUS          1160
#define TAHOEop_PADD2_UUU          1161
#define TAHOEop_PADD4              1162

#define TAHOEop_PAVG1              1163   /* Parallel average */
#define TAHOEop_PAVG1_RAZ          1164
#define TAHOEop_PAVG2              1165
#define TAHOEop_PAVG2_RAZ          1166

#define TAHOEop_PAVGSUB1           1167   /* Parallel average subtract */
#define TAHOEop_PAVGSUB2           1168

#define TAHOEop_PCMP1_EQ           1169   /* Parallel compare */
#define TAHOEop_PCMP1_GT           1170
#define TAHOEop_PCMP2_EQ           1171
#define TAHOEop_PCMP2_GT           1172
#define TAHOEop_PCMP_EQ            1173
#define TAHOEop_PCMP_GT            1174

#define TAHOEop_PSHLADD2           1175   /* Parallel shift left, add */
#define TAHOEop_PSHRADD2           1176   /* Parallel shift right, add */

#define TAHOEop_PSUB1              1177   /* Parallel subtract */
#define TAHOEop_PSUB1_SSS          1178
#define TAHOEop_PSUB1_UUS          1179
#define TAHOEop_PSUB1_UUU          1180
#define TAHOEop_PSUB2              1181
#define TAHOEop_PSUB2_SSS          1182
#define TAHOEop_PSUB2_UUS          1183
#define TAHOEop_PSUB2_UUU          1184
#define TAHOEop_PSUB4              1185

#define TAHOEop_PMAX1_U            1186   /* Parallel maximum */
#define TAHOEop_PMAX2              1187

#define TAHOEop_PMIN1_U            1188   /* Parallel minimum */
#define TAHOEop_PMIN2              1189

#define TAHOEop_PSAD1              1190   /* Parallel sum of absolute diff */

#define TAHOEop_PMPY2_L            1191   /* Parallel multiply */
#define TAHOEop_PMPY2_R            1192

#define TAHOEop_PMPYSHR2           1193   /* Parallel multiply shift right */
#define TAHOEop_PMPYSHR2_U         1194

#define TAHOEop_POPCNT             1195   /* Population count */

#define TAHOEop_MIX1_L             1196   /* Mix */
#define TAHOEop_MIX1_R             1197
#define TAHOEop_MIX2_L             1198
#define TAHOEop_MIX2_R             1199
#define TAHOEop_MIX4_L             1200
#define TAHOEop_MIX4_R             1201

#define TAHOEop_MUX1               1202   /* Multiplex opcodes */
#define TAHOEop_MUX2               1203

#define TAHOEop_PACK2_SSS          1204
#define TAHOEop_PACK2_USS          1205
#define TAHOEop_PACK4_SSS          1206

#define TAHOEop_PSHL2              1207   /* Parallel shift opcodes */
#define TAHOEop_PSHL4              1208
#define TAHOEop_PSHR2              1209
#define TAHOEop_PSHR2_U            1210
#define TAHOEop_PSHR4              1211
#define TAHOEop_PSHR4_U            1212

#define TAHOEop_SHL                1213   /* Shift opcodes */
#define TAHOEop_SHR                1214
#define TAHOEop_SHR_U              1215

#define TAHOEop_UNPACK1_H          1216
#define TAHOEop_UNPACK1_L          1217
#define TAHOEop_UNPACK2_H          1218
#define TAHOEop_UNPACK2_L          1219
#define TAHOEop_UNPACK4_H          1220
#define TAHOEop_UNPACK4_L          1221

#define TAHOEop_ADDP4              1222   /* Add pointer */
#define TAHOEop_SHLADDP4           1223   /* Left shift and add pointer */

#define TAHOEop_CLRRRB             1224   /* Clear register rename base */
#define TAHOEop_CLRRRB_PR          1225

#define TAHOEop_COVER              1226   /* Cover stack frame */
#define TAHOEop_FLUSHRS            1227   /* Flush register stack */
#define TAHOEop_LOADRS             1228   /* Load register stack */

#define TAHOEop_CMPXCHG1_ACQ       1229   /* Compare and exchange opcodes */
#define TAHOEop_CMPXCHG1_REL       1230
#define TAHOEop_CMPXCHG2_ACQ       1231
#define TAHOEop_CMPXCHG2_REL       1232
#define TAHOEop_CMPXCHG4_ACQ       1233
#define TAHOEop_CMPXCHG4_REL       1234
#define TAHOEop_CMPXCHG8_ACQ       1235
#define TAHOEop_CMPXCHG8_REL       1236

#define TAHOEop_FETCHADD4_ACQ      1237   /* Fetch and add immediate */
#define TAHOEop_FETCHADD4_REL      1238
#define TAHOEop_FETCHADD8_ACQ      1239
#define TAHOEop_FETCHADD8_REL      1240

#define TAHOEop_XCHG1              1241   /* Exchange opcodes */
#define TAHOEop_XCHG2              1242
#define TAHOEop_XCHG4              1243
#define TAHOEop_XCHG8              1244

#define TAHOEop_FCLASS             1245   /* FP class */
#define TAHOEop_FCMP               1246   /* FP compare */
#define TAHOEop_FCVT_FX            1247   /* Convert FP to int */
#define TAHOEop_FCVT_FXU           1248
#define TAHOEop_FCVT_XF            1249
#define TAHOEop_FPCVT_FX           1250   /* Convert parallel FP to int */
#define TAHOEop_FPCVT_FX_TRUNC     1251
#define TAHOEop_FPCVT_FXU          1252
#define TAHOEop_FPCVT_FXU_TRUNC    1253
#define TAHOEop_FPMA               1254   /* FP parallel multiply add */
#define TAHOEop_FPMS               1255   /* FP parallel multiply subtract */
#define TAHOEop_FPNMA              1256   /* FP parallel negate multiply add */
#define TAHOEop_FPMERGE            1257   /* FP parallel merge */
#define TAHOEop_FPAMAX             1258   /* FP parallel absolute max */
#define TAHOEop_FPAMIN             1259   /* FP parallel absolute min */
#define TAHOEop_FPCMP              1260   /* FP parallel compare */
#define TAHOEop_FPMAX              1261   /* FP parallel maximum */
#define TAHOEop_FPMERGE_NS         1262   /* FP parallel merge */
#define TAHOEop_FPMERGE_S          1263
#define TAHOEop_FPMIN              1264   /* FP parallel minimum */
#define TAHOEop_FPRCPA             1265   /* FP parallel reciprocial approx */
#define TAHOEop_FPRSQRTA           1266   /* FP parallel recip. square root */

#define TAHOEop_ST1                1267   /* Store opcodes */
#define TAHOEop_ST1_REL            1268
#define TAHOEop_ST2                1269
#define TAHOEop_ST2_REL            1270
#define TAHOEop_ST4                1271
#define TAHOEop_ST4_REL            1272
#define TAHOEop_ST8                1273
#define TAHOEop_ST8_REL            1274
#define TAHOEop_ST8_SPILL          1275
#define TAHOEop_STF_SPILL          1276
#define TAHOEop_STF8               1277
#define TAHOEop_STFD               1278
#define TAHOEop_STFE               1279
#define TAHOEop_STFS               1280

#define TAHOEop_EPC                1281   /* Enter privileged code */
#define TAHOEop_BSW_0              1282   /* Bank switch */
#define TAHOEop_BSW_1              1283
#define TAHOEop_RFI                1284   /* Return from interruption */
#define TAHOEop_INVALA             1285   /* Invalidate alat */
#define TAHOEop_INVALA_E           1286
#define TAHOEop_INVALA_E_F         1287
#define TAHOEop_CC                 1288
#define TAHOEop_FWB                1289   /* Flush write buffers */
#define TAHOEop_HALT               1290
#define TAHOEop_HALT_MF            1291
#define TAHOEop_MF                 1292   /* Memory fence instructions */
#define TAHOEop_MF_A               1293
#define TAHOEop_SRLZ_D             1294   /* Serialize */
#define TAHOEop_SRLZ_I             1295
#define TAHOEop_SYNC_I             1296
#define TAHOEop_ALLOC              1297   /* Allocate stack frame */
#define TAHOEop_FC                 1298   /* Flush cache */
#define TAHOEop_ITC_D              1299   /* Insert translation cache */
#define TAHOEop_ITC_I              1300
#define TAHOEop_ITR_D              1301
#define TAHOEop_ITR_I              1302
#define TAHOEop_PROBE_R            1303   /* Probe access */
#define TAHOEop_PROBE_R_FAULT      1304
#define TAHOEop_PROBE_RW_FAULT     1305
#define TAHOEop_PROBE_W            1306
#define TAHOEop_PROBE_W_FAULT      1307
#define TAHOEop_PTC_E              1308   /* Purge translation cache entry */
#define TAHOEop_PTC_G              1309   /* Purge global translation cache */
#define TAHOEop_PTC_GA             1310
#define TAHOEop_PTC_L              1311   /* Purge local translation cache */
#define TAHOEop_PTR_D              1312   /* Purge translation register */
#define TAHOEop_PTR_I              1313
#define TAHOEop_RSM                1314   /* Reset system mask */
#define TAHOEop_RUM                1315   /* Reset user mask */
#define TAHOEop_SSM                1316   /* Set system mask */
#define TAHOEop_SUM                1317   /* Set user mask */
#define TAHOEop_TAK                1318   /* Translation access key */
#define TAHOEop_THASH              1319   /* Translation hashed entry addrss */
#define TAHOEop_TPA                1320   /* Translate to physical address */
#define TAHOEop_TTAG               1321   /* Translation hashed entry tag */

/* Extra pseudo-ops that need special processing */
#define TAHOEop_MOVI               1322   /* Move 22 bit immediate */
#define TAHOEop_MOV_GR             1323   /* Move between general registers */
#define TAHOEop_NON_INSTR          1324   /* Define, prologue, epilogue */
#define TAHOEop_FABS               1325   /* Floating point absolute value */
#define TAHOEop_FADD_D             1326   /* Floating point add */
#define TAHOEop_FADD_S             1327   /* Floating point add */
#define TAHOEop_FADD               1328   /* Floating point add */
#define TAHOEop_CHK_S              1329   /* CHK pseudo op */
#define TAHOEop_MOV_FR             1330   /* MOV between FP registers */
#define TAHOEop_FSUB_D             1331   /* Floating point subtract */
#define TAHOEop_FSUB_S             1332   /* Floating point subtract */
#define TAHOEop_FSUB               1333   /* Floating point subtract */


/*=========================================================================*/
/*
 *    Function prototypes
 */
/*=========================================================================*/

#ifdef __cplusplus
extern "C"
{
#endif

  extern int M_tahoe_type_size (int mtype);
  extern int M_tahoe_type_align (int mtype);
  extern void M_tahoe_void (M_Type type);
  extern void M_tahoe_bit_llong (M_Type type, int n);
  extern void M_tahoe_bit_long (M_Type type, int n);
  extern void M_tahoe_bit_int (M_Type type, int n);
  extern void M_tahoe_bit_short (M_Type type, int n);
  extern void M_tahoe_bit_char (M_Type type, int n);
  extern void M_tahoe_char (M_Type type, int unsign);
  extern void M_tahoe_short (M_Type type, int unsign);
  extern void M_tahoe_int (M_Type type, int unsign);
  extern void M_tahoe_long (M_Type type, int unsign);
  extern void M_tahoe_llong (M_Type type, int unsign);
  extern void M_tahoe_float (M_Type type, int unsign);
  extern void M_tahoe_double (M_Type type, int unsign);
  extern void M_tahoe_pointer (M_Type type);
  extern int M_tahoe_eval_type (M_Type type, M_Type ntype);
  extern int M_tahoe_eval_type2 (M_Type type, M_Type ntype);
  extern int M_tahoe_call_type (M_Type type, M_Type ntype);
  extern int M_tahoe_call_type2 (M_Type type, M_Type ntype);
  extern void M_tahoe_array_layout (M_Type type, int *offset);
  extern int M_tahoe_array_align (M_Type type);
  extern int M_tahoe_array_size (M_Type type, int dim);
  extern void M_tahoe_union_layout (int n, _M_Type * type, int *offset,
				    int *bit_offset);
  extern int M_tahoe_union_align (int n, _M_Type * type);
  extern int M_tahoe_union_size (int n, _M_Type * type);
  extern void M_tahoe_struct_layout (int n, _M_Type * type, int *base,
				     int *bit_offset);
  extern int M_tahoe_struct_align (int n, _M_Type * type);
  extern int M_tahoe_struct_size (int n, _M_Type * type, int struct_align);
  extern int M_tahoe_layout_fnvar (List param_list, char **macro,
				   int *pcount, int purpose);
  extern int M_tahoe_layout_retvar (M_Param param, int purpose);
  extern int M_tahoe_fnvar_layout (int n, _M_Type * type, long int *offset,
				   int *mode, int *reg, int *paddr,
				   char **macro, int *su_sreg, int *su_ereg,
				   int *pcount, int is_st, int purpose);
  extern int M_tahoe_lvar_layout (int n, _M_Type * type, long int *offset,
				  char **base_macro);
  extern int M_tahoe_fnvar_to_lvar (_M_Type type, long int *offset,
				    char **base_macro, int local_space);
  extern int M_tahoe_no_short_int (void);
  extern int M_tahoe_layout_order (void);
  extern void M_tahoe_cb_label_name (char *fn, int cb, char *line, int len);
  extern int M_tahoe_is_cb_label (char *label, char *fn, int *cb);

  extern void M_tahoe_jumptbl_label_name (char *fn, int tbl_id, char *line,
					  int len);
  extern int M_tahoe_is_jumptbl_label (char *label, char *fn, int *tbl_id);

  extern int M_tahoe_structure_pointer (int purpose);
  extern int M_tahoe_return_register (int type, int purpose);
  extern L_Operand *M_tahoe_epilogue_cntr_register ();
  extern L_Operand *M_tahoe_loop_cntr_register ();
  extern char *M_tahoe_fn_label_name (char *label,
				      int (*is_func) (char *is_func_label));
  extern char *M_tahoe_fn_name_from_label (char *label);
  extern void M_set_model_tahoe (char *model_name);
  extern int M_tahoe_fragile_macro (int macro_value);
  extern Set M_tahoe_fragile_macro_set (void);
  extern int M_tahoe_extra_pred_define_opcode (int proc_opc);
  extern int M_tahoe_extra_pred_define_type1 (L_Oper *);
  extern int M_tahoe_extra_pred_define_type2 (L_Oper *);
  extern int M_tahoe_subroutine_call (int opc);

  extern int M_tahoe_dataflow_macro (int id);
  extern void M_define_macros_tahoe (STRING_Symbol_Table * sym_tbl);
  extern char *M_get_macro_name_tahoe (int id);
  extern int M_oper_supported_in_arch_tahoe (int opc);
  extern int M_num_oper_required_for_tahoe (L_Oper * oper, char *name);
  extern int M_num_registers_tahoe (int ctype);
  extern int M_is_stack_operand_tahoe (L_Operand * operand);
  extern int M_is_unsafe_macro_tahoe (L_Operand * operand);
  extern int M_operand_type_tahoe (L_Operand * operand);
  extern int M_conflicting_operands_tahoe (L_Operand * operand,
					   L_Operand * conflict_array[],
					   int len, int prepass);
  extern int M_opc_from_proc_opc_tahoe (int proc_opc);

  extern int M_cannot_predicate_tahoe (L_Oper * oper);

  extern void M_get_memory_operands_tahoe (int *first, int *number, 
					   int proc_opc);
#ifdef __cplusplus
}
#endif

#endif
