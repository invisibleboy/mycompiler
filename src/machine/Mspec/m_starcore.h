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
 *  File:  m_starcore.h
 *
 *  Description: Defines symbols used for reference in Mspec and code
 * 	 	generator.
 *
 *  Authors: Christopher Shannon
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

#ifndef _M_STARCORE_H_
#define _M_STARCORE_H_

/* 10/29/02 REK Adding config.h */
#include <config.h>
#include  "m_spec.h"

#define INT_MAX 0x7fffffff

#define STARCORE_NUM_DATA_REG            (16)
#define STARCORE_NUM_ADDR_REG            (16)
#define STARCORE_NUM_PRED_REG            (2)
#define STARCORE_NUM_START_ADDR_REG      (4)
#define STARCORE_NUM_LOOP_COUNTER_REG    (4)
#define STARCORE_NUM_ADDR_OFFSET_REG     (4)
#define STARCORE_NUM_BASE_ADDR_REG       (8)
#define STARCORE_NUM_MODULO_ADDR_REG     (4)

#define STARCORE_MIN_REG_ID              (0)
#define STARCORE_MAX_DATA_REG_ID         (STARCORE_NUM_DATA_REG-1)
#define STARCORE_MAX_ADDR_REG_ID         (STARCORE_NUM_ADDR_REG-1+\
                                          M_STARCORE_ADDR_BASE)
#define STARCORE_MAX_PRED_REG_ID         (STARCORE_NUM_PRED_REG-1)
#define STARCORE_MAX_START_ADDR_REG_ID   (STARCORE_NUM_START_ADDR_REG-1)
#define STARCORE_MAX_LOOP_COUNTER_REG_ID (STARCORE_NUM_LOOP_COUNTER_REG-1)
#define STARCORE_MAX_ADDR_OFFSET_REG_ID  (STARCORE_NUM_ADDR_OFFSET_REG-1)
#define STARCORE_MAX_BASE_ADDR_REG_ID    (STARCORE_NUM_BASE_ADDR_REG-1)
#define STARCORE_MAX_MODULO_ADDR_REG_ID  (STARCORE_NUM_MODULO_ADDR_REG-1)

/* Macro used in m_starcore.c and Lstarcore codegen */

#define ADDR_ALIGN(addr, align) ( ((addr)+(align-1)) & ~(align-1) )

/*
 * Declarations for processor models
 */

enum
{
  M_EM_SC140 = 0
};


#define EM_SC_MODEL_OK( model ) ( ( (model) == M_EM_SC140))

enum
{
  /* These need to be numbered starting after L_MAC_LAST in l_code.h
     since there is no way to distinguish between Lcode and machine specific
     macros. */
  STARCORE_MAC_SP = L_CODEGEN_START_VALUE,/* stack pointer                */

  STARCORE_MAC_LEAF,		/* 1 if leaf fn, 0 if not leaf function   */
  STARCORE_MAC_SYNC_SIZE,	/* size in bytes in LV space on stack     *
				 * used to spill integers around syncs    *
				 * such as setjmp                         */
  STARCORE_MAC_MEM_ALLOC,	/* total alloc space requirements         */
  STARCORE_MAC_TEMPLATE,
  STARCORE_MAC_LABEL,

  /* Application Registers - If these change, update application register
     macro check STARCORE_APPLICATION_MACRO( macro ).  These have been
     ordered by their application register number.*/

  STARCORE_MAC_NSP,             /* Normal Stack Pointer Register          */
  STARCORE_MAC_ESP,             /* Exception Stack Pointer Register       */
  STARCORE_MAC_PC,              /* Program Counter                        */
  STARCORE_MAC_MCTL,            /* Modifier Control Register              */
  STARCORE_MAC_SR,              /* Status Register                        */
  STARCORE_MAC_EMR,             /* Exception and Mode Register            */
  STARCORE_MAC_VBA,             /* Vector Base Address Register           */
  STARCORE_MAC_PCTL0,           /* PLL Control Register 0                 */
  STARCORE_MAC_PCTL1,           /* Clock Control Register 1               */
  STARCORE_MAC_T_BIT,           /* Represents the T-bit in the SR for     *
				 * conditional execution                  */
  STARCORE_MAC_T_BIT_INV,	/* Represents the inverse of the T-bit    */
  STARCORE_MAC_TEMP             /* Will become r3 and is used for stack   *
				 * adjustments after register allocation  */
};

#define STARCORE_APPLICATION_MACRO( m ) ((m->value.mac >= STARCORE_MAC_NSP)&& \
				         (m->value.mac <= STARCORE_MAC_T_BIT))

/*========================================================================*/
/*
 *    StarCore parameter and return value offsets
 */
/*========================================================================*/
/* 
 * 
 * Data Registers          d
 * 
 * 2 input parameters      0-1
 * 1 return value          0 
 * 
 * Address Registers       r
 * 
 * 2 parameter registers   0-1
 * 1 return value          0
 * 
 */

/* Number of scalar param registers */
#define M_STARCORE_MAX_FNVAR_INT_REG	 2

/* Number of address param registers */
#define M_STARCORE_MAX_FNVAR_ADDR_REG	 2

/* Number of return registers */
#define M_STARCORE_MAX_RET_REG	         1

/* parameter registers */
#define M_STARCORE_SMALL_STRUCT_MAX      2


/* incoming and outgoing parameters */
#define M_STARCORE_IN_INT_BASE           0	/* Int Registers     */
#define M_STARCORE_OUT_INT_BASE          0	/* Int Registers     */
#define M_STARCORE_IN_ADDR_BASE          2
#define M_STARCORE_OUT_ADDR_BASE         2

#define M_STARCORE_RET_INT_BASE	       	 0	/* Non-stacked Int registers */

#define M_STARCORE_ADDR_BASE            16      /* Address Registers         */

#define M_STARCORE_LCODE_RET_INT         0
#define M_STARCORE_LCODE_RET_ADDR        2
#define M_STARCORE_LCODE_RET_ST         15

/*=========================================================================*/
/*
 *    StarCore specific data sizes
 */
/*=========================================================================*/


#define M_STARCORE_SIZE_VOID          0
#define M_STARCORE_SIZE_BIT           1
#define M_STARCORE_SIZE_CHAR          8
#define M_STARCORE_SIZE_BYTE          8
#define M_STARCORE_SIZE_SHORT        16
#define M_STARCORE_SIZE_WORD         16
#define M_STARCORE_SIZE_INT          32
#define M_STARCORE_SIZE_LONG         32
#define M_STARCORE_SIZE_FLOAT        32
#define M_STARCORE_SIZE_DOUBLE       32
#define M_STARCORE_SIZE_LONG_DOUBLE  32
#define M_STARCORE_SIZE_POINTER      32
#define M_STARCORE_SIZE_UNION        -1
#define M_STARCORE_SIZE_STRUCT       -1
#define M_STARCORE_SIZE_BLOCK        -1
#define M_STARCORE_SIZE_MAX          32

#define M_STARCORE_ALIGN_VOID        -1
#define M_STARCORE_ALIGN_BIT          1
#define M_STARCORE_ALIGN_CHAR         8
#define M_STARCORE_ALIGN_SHORT       16
#define M_STARCORE_ALIGN_INT         32
#define M_STARCORE_ALIGN_LONG        32
#define M_STARCORE_ALIGN_FLOAT       32
#define M_STARCORE_ALIGN_DOUBLE      32
#define M_STARCORE_ALIGN_LONG_DOUBLE 32
#define M_STARCORE_ALIGN_POINTER     32
#define M_STARCORE_ALIGN_UNION       -1	/* depends on the field */
#define M_STARCORE_ALIGN_STRUCT      -1
#define M_STARCORE_ALIGN_BLOCK       -1
#define M_STARCORE_ALIGN_MAX         32


/*=========================================================================*/
/*
 *    StarCore specific opcodes
 */
/*=========================================================================*/


/*
 * DALU Arithmetic op extensions
 */

#define STARCOREop_VERSION         1
#define STARCOREop_NON_INSTR   	5001  /* Define, prologue, epilogue          */

#define STARCOREop_ABS          1001  /* Absolute value                      */
#define STARCOREop_ADC          1002  /* Add long with carry                 */
#define STARCOREop_ADD          1003  /* Add                                 */
#define STARCOREop_ADD2         1004  /* Add two 16-bit values               */
#define STARCOREop_ADDNC_W      1005  /* Add without changing the carry      *
				       * bit in the SR                       */
#define STARCOREop_ADR          1006  /* Add and round                       */
#define STARCOREop_ASL          1007  /* Arithmetic shift left by one bit    */
#define STARCOREop_ASR          1008  /* Arithmetic shift right by one bit   */
#define STARCOREop_CLR          1009  /* Clear                               */
#define STARCOREop_CMPEQ        1010  /* Compare for equal                   */
#define STARCOREop_CMPEQ_W_1    1011  /* Compare for equal using immediate   */
#define STARCOREop_CMPEQ_W_2    1012  /* Compare for equal using immediate   */
#define STARCOREop_CMPGT        1013  /* Compare for greater than            */
#define STARCOREop_CMPGT_W_1    1014  /* Compare for greater than using      *
				       * immediate                           */
#define STARCOREop_CMPGT_W_2    1015  /* Compare for greater than using      *
				       * immediate                           */
#define STARCOREop_CMPHI        1016  /* Compare for higher (unsigned)       */
#define STARCOREop_DECEQ        1017  /* Decrement a data register and set   *
				       * T if zero                           */
#define STARCOREop_DECGE        1018  /* Decrement a data register and set   *
				       * T if greater than or equal to 0     */
#define STARCOREop_DIV          1019  /* Divide iteration                    */
#define STARCOREop_DMACSS       1020  /* Multiply signed by signed and       *
				       * accumulate with data register right *
				       * shifted by word size                */
#define STARCOREop_DMACSU       1021  /* Multiply signed by unsigned and     *
				       * accumulate with data register right *
				       * shifted by word size                */
#define STARCOREop_IADD         1022  /* Add integers                        */
#define STARCOREop_IMAC         1023  /* Multiply-accumulate integers        */
#define STARCOREop_IMACLHUU     1024  /* Multiply-accumulate unsigned        *
				       * integers; first source from low     *
				       * portion, second from high portion   */
#define STARCOREop_IMACUS       1025  /* Multiply-accumulate unsigned integer*
				       * and signed integer                  */
#define STARCOREop_IMPY         1026  /* Multiply signed integers in data    *
				       * registers                           */
#define STARCOREop_IMPY_W       1027  /* Signed immediate integer multiply   */
#define STARCOREop_IMPYHLUU     1028  /* Multiply unsigned integer and       *
				       * unsigned integer; first source from *
				       * high portion, second from low       *
				       * portion                             */
#define STARCOREop_IMPYSU       1029  /* Multiply signed integer and unsigned*
				       * integer                             */
#define STARCOREop_IMPYUU       1030  /* Multiply unsigned integer and       *
				       * unsigned integer                    */
#define STARCOREop_INC          1031  /* Increment a data register (as       *
				       * integer data)                       */
#define STARCOREop_INC_F        1032  /* Increment a data register (as       *
				       * fractional data)                    */
#define STARCOREop_MAC_1        1033  /* Multiply-accumulate signed          *
				       * fractions                           */
#define STARCOREop_MAC_2        1034  /* Multiply-accumulate signed          *
				       * fractions                           */
#define STARCOREop_MACR         1035  /* Multiply-accumulate signed          *
				       * fractions and round                 */
#define STARCOREop_MACSU        1036  /* Multiply-accumulate signed fraction *
				       * and unsigned fraction               */
#define STARCOREop_MACUS        1037  /* Multiply-accumulate unsigned        *
				       * fraction and signed fraction        */
#define STARCOREop_MACUU        1038  /* Multiply-accumulate unsigned        *
				       * fraction and unsigned fraction      */
#define STARCOREop_MAX          1039  /* Transfer maximum signed value       */
#define STARCOREop_MAX2         1040  /* Transfer two 16-bit maximum signed  *
				       * values                              */
#define STARCOREop_MAX2VIT      1041  /* Special MAX2 version for Viterbi    *
				       * kernel                              */
#define STARCOREop_MAXM         1042  /* Transfer maximum magnitude value    */
#define STARCOREop_MIN          1043  /* Transfer minimum signed value       */
#define STARCOREop_MPY          1044  /* Multiply signed fractions           */
#define STARCOREop_MPYR         1045  /* Multiply signed fractions and round */
#define STARCOREop_MPYSU        1046  /* Multiply signed fraction and        *
				       * unsigned fraction                   */
#define STARCOREop_MPYUS        1047  /* Multiply unsigned fraction and      *
				       * signed fraction                     */
#define STARCOREop_MPYUU        1048  /* Multiply unsigned fraction and      *
				       * unsigned fraction                   */
#define STARCOREop_NEG          1049  /* Negate                              */
#define STARCOREop_RND          1050  /* Round                               */
#define STARCOREop_SAT_F        1051  /* Saturate fractional value in data   *
				       * register to fit in high portion     */
#define STARCOREop_SAT_L        1052  /* Saturate value in data register to  *
				       * fit in 32 bits                      */
#define STARCOREop_SBC          1053  /* Subtract long with carry            */
#define STARCOREop_SBR          1054  /* Subtract and round                  */
#define STARCOREop_SUB          1055  /* Subtract                            */
#define STARCOREop_SUB2         1056  /* Subtract two words                  */
#define STARCOREop_SUBL         1057  /* Shift left and subtract             */
#define STARCOREop_SUBNC_W      1058  /* Subtract without changing the carry *
				       * bit in the status register          */
#define STARCOREop_TFR          1059  /* Transfer data register to a data    *
				       * register                            */
#define STARCOREop_TFRF         1060  /* Conditional data register transfer, *
				       * if the T bit is clear               */
#define STARCOREop_TFRT         1061  /* Conditional data register transfer, *
				       * if the T bit is set                 */
#define STARCOREop_TSTEQ        1062  /* Test for equal to 0                 */
#define STARCOREop_TSTGE        1063  /* Test for greater than or equal to 0 */
#define STARCOREop_TSTGT        1064  /* Test for greater than 0             */


/*
 * DALU Logical op extensions
 */

#define STARCOREop_AND_1        1101  /* Bitwise AND                         */
#define STARCOREop_AND_2        1102  /* Bitwise AND                         */
#define STARCOREop_AND_W_2      1103  /* Bitwise AND with 16-bit immediate   *
				       * and operand from memory             */
#define STARCOREop_AND_W_3      1104  /* Bitwise AND with 16-bit immediate   *
				       * and operand from memory             */
#define STARCOREop_ASLL         1105  /* Multi-bit arithmetic shift left     */
#define STARCOREop_ASLW         1106  /* Word arithmetic shift left (16-bit  *
				       * shift)                              */
#define STARCOREop_ASRR         1107  /* Multi-bit arithmetic shift right    */
#define STARCOREop_ASRW         1108  /* Word arithmetic shift right (16-bit *
				       * shift)                              */
#define STARCOREop_CLB          1109  /* Count leading bits (ones or zeros)  */
#define STARCOREop_EOR_1        1110  /* Bitwise exclusive OR                */
#define STARCOREop_EOR_2        1111  /* Bitwise exclusive OR                */
#define STARCOREop_EOR_W_2      1112  /* Bitwise exclusive OR with 16-bit    *
				       * immediate and operand from memory   */
#define STARCOREop_EOR_W_3      1113  /* Bitwise exclusive OR with 16-bit    *
				       * immediate and operand from memory   */
#define STARCOREop_EXTRACT      1114  /* Extract signed bit field            */
#define STARCOREop_EXTRACTU     1115  /* Extract unsigned bit field          */
#define STARCOREop_INSERT       1116  /* Insert bit field                    */
#define STARCOREop_LSLL         1117  /* Multi-bit logical shift left        */
#define STARCOREop_LSR          1118  /* Logical shift right by one bit      */
#define STARCOREop_LSRR         1119  /* Multi-bit logical shift right       */
#define STARCOREop_LSRW         1120  /* Word logical shift right (16-bit    *
				       * shift)                              */
#define STARCOREop_NOT_1        1121  /* Binary inversion                    */
#define STARCOREop_NOT_2        1122  /* Binary inversion                    */
#define STARCOREop_NOT_W_2      1123  /* Binary inversion of a 16-bit        *
				       * operand in memory                   */
#define STARCOREop_NOT_W_3      1124  /* Binary inversion of a 16-bit        *
				       * operand in memory                   */
#define STARCOREop_OR_1         1125  /* Bitwise inclusive OR                */
#define STARCOREop_OR_2         1126  /* Bitwise inclusive OR                */
#define STARCOREop_OR_W_2       1127  /* Bitwise inclusive OR with 16-bit    *
				       * immediate and operand from memory   */
#define STARCOREop_OR_W_3       1128  /* Bitwise inclusive OR with 16-bit    *
				       * immediate and operand from memory   */
#define STARCOREop_ROL          1129  /* Rotate one bit left through the     *
				       * carry bit                           */
#define STARCOREop_ROR          1130  /* Rotate one bit right through the    *
				       * carry bit                           */
#define STARCOREop_SXT_B        1131  /* Sign extend byte                    */
#define STARCOREop_SXT_L        1132  /* Sign extend long                    */
#define STARCOREop_SXT_W        1133  /* Sign extend word                    */
#define STARCOREop_ZXT_B        1134  /* Zero extend byte                    */
#define STARCOREop_ZXT_L        1135  /* Zero extend long                    */
#define STARCOREop_ZXT_W        1136  /* Zero extend word                    */


/*
 * AGU Arithmetic op extensions
 */

#define STARCOREop_ADDA_1       1201  /* Add (affected by the modifier mode) */
#define STARCOREop_ADDA_2       1202  /* Add (affected by the modifier mode) */
#define STARCOREop_ADDL1A       1203  /* Add with 1-bit left shift of source *
				       * operand (affected by the modifier   *
				       * mode)                               */
#define STARCOREop_ADDL2A       1204  /* Add with 2-bit left shift of source *
				       * operand (affected by the modifier   *
				       * mode)                               */
#define STARCOREop_ASL2A        1205  /* Arithmetic shift left by 2 bits     */
#define STARCOREop_ASLA         1206  /* Arithmetic shift left by one bit    */
#define STARCOREop_ASRA         1207  /* Arithmetic shift right (32-bit)     */
#define STARCOREop_CMPEQA       1208  /* Compare for equal                   */
#define STARCOREop_CMPGTA       1209  /* Compare for greater than            */
#define STARCOREop_CMPHIA       1210  /* Compare for higher than (unsigned)  */
#define STARCOREop_DECA         1211  /* Decrement register                  */
#define STARCOREop_DECEQA       1212  /* Decrement and set T if zero         */
#define STARCOREop_DECGEA       1213  /* Decrement and set T if equal or     *
				       * greater than zero                   */
#define STARCOREop_INCA         1214  /* Increment register                  */
#define STARCOREop_LSRA         1215  /* Logical shift right (32-bit)        */
#define STARCOREop_SUBA         1216  /* Subtract (affected by the modifier  *
				       * mode)                               */
#define STARCOREop_SXTA_B       1217  /* Sign extend byte                    */
#define STARCOREop_SXTA_W       1218  /* Sign extend word                    */
#define STARCOREop_TFRA         1219  /* Register transfer                   */
#define STARCOREop_TSTEQA_L     1220  /* Test for equal                      */
#define STARCOREop_TSTEQA_W     1221  /* Test for equal on lower 16 bits     */
#define STARCOREop_TSTGEA       1222  /* Test for greater than or equal      */
#define STARCOREop_TSTGTA       1223  /* Test for greater than               */
#define STARCOREop_ZXTA_B       1224  /* Zero extend byte                    */
#define STARCOREop_ZXTA_W       1225  /* Zero extend word                    */


/*
 * Move op extensions
 */

#define STARCOREop_MOVE_B_ST_1  1301  /* Move byte to memory                 */
#define STARCOREop_MOVE_B_ST_2  1302  /* Move byte to memory                 */
#define STARCOREop_MOVE_B_ST_3  1303  /* Move byte to memory                 */
#define STARCOREop_MOVE_B_LD_1  1304  /* Move byte from memory               */
#define STARCOREop_MOVE_B_LD_2  1305  /* Move byte from memory               */
#define STARCOREop_MOVEU_B_LD_1 1306  /* Move unsigned byte from memory      */
#define STARCOREop_MOVEU_B_LD_2 1307  /* Move unsigned byte from memory      */
#define STARCOREop_MOVEU_B_LD_3 1308  /* Move unsigned byte from memory      */
#define STARCOREop_MOVE_W_ST_1  1309  /* Move integer word to memory,        *
				       * or immediate to memory              */
#define STARCOREop_MOVE_W_ST_2  1310  /* Move integer word to memory,        *
				       * or immediate to memory              */
#define STARCOREop_MOVE_W_ST_3  1311  /* Move integer word to memory,        *
				       * or immediate to memory              */
#define STARCOREop_MOVE_W_LD_1  1312  /* Move integer word from memory       */
#define STARCOREop_MOVE_W_LD_2  1313  /* Move integer word from memory       */
#define STARCOREop_MOVE_W_LD_3  1314  /* Move integer word from memory       */
#define STARCOREop_MOVE_W_MV_1  1315  /* Move immediate to register          */
#define STARCOREop_MOVE_W_MV_2  1316  /* Move immediate to register          */
#define STARCOREop_MOVEU_W_LD_1 1317  /* Move unsigned integer word from     *
				       * memory                              */
#define STARCOREop_MOVEU_W_LD_2 1318  /* Move unsigned integer word from     *
				       * memory                              */
#define STARCOREop_MOVEU_W_LD_3 1319  /* Move unsigned integer word from     *
				       * memory                              */
#define STARCOREop_MOVEU_W_MV   1320  /* Move unsigned integer word from     *
				       * immediate                           */
#define STARCOREop_MOVE_2W_ST   1321  /* Move two integer words to           *
				       * memory from a register pair         */
#define STARCOREop_MOVE_2W_LD   1322  /* Move two integer words from         *
				       * memory to a register pair           */
#define STARCOREop_MOVE_4W_ST   1323  /* Move four integer words to          *
				       * memory from a register quad         */
#define STARCOREop_MOVE_4W_LD   1324  /* Move four integer words from        *
				       * memory to a register quad           */
#define STARCOREop_MOVE_L_ST_1  1325  /* Move long to memory                 */
#define STARCOREop_MOVE_L_ST_2  1326  /* Move long to memory                 */
#define STARCOREop_MOVE_L_ST_3  1327  /* Move long to memory                 */
#define STARCOREop_MOVE_L_LD_1  1328  /* Move long from memory               */
#define STARCOREop_MOVE_L_LD_2  1329  /* Move long from memory               */
#define STARCOREop_MOVE_L_LD_3  1330  /* Move long from memory               */
#define STARCOREop_MOVE_L_MV_1  1331  /* Move long between registers or      *
				       * immediates                          */
#define STARCOREop_MOVE_L_MV_3  1332  /* Move long between registers or      *
				       * immediates                          */
#define STARCOREop_MOVEU_L_MV   1333  /* Move unsigned long from immediate   */
#define STARCOREop_MOVES_L_ST   1334  /* Move long to memory with scaling    *
				       * and limiting enabled                */
#define STARCOREop_MOVE_2L_ST   1335  /* Move two longs from a register pair */
#define STARCOREop_MOVE_2L_LD   1336  /* Move two longs to a register pair   */
#define STARCOREop_MOVE_F_ST_1  1337  /* Move fractional word to memory      */
#define STARCOREop_MOVE_F_LD_1  1338  /* Move fractional word from memory    */
#define STARCOREop_MOVE_F_LD_2  1339  /* Move fractional word from memory    */
#define STARCOREop_MOVE_F_LD_3  1340  /* Move fractional word from memory    */
#define STARCOREop_MOVE_F_MV_2  1341  /* Move fractional word to/from        *
				       * registers                           */
#define STARCOREop_MOVES_F_ST_1 1342  /* Move fractional word to memory with *
				       * scaling and limiting enabled        */
#define STARCOREop_MOVES_F_ST_2 1343  /* Move fractional word to memory with *
				       * scaling and limiting enabled        */
#define STARCOREop_MOVES_F_ST_3 1344  /* Move fractional word to memory with *
				       * scaling and limiting enabled        */
#define STARCOREop_MOVE_2F_LD   1345  /* Move two fractional words from      *
				       * memory to a register pair           */
#define STARCOREop_MOVES_2F_ST  1346  /* Move two fractional words to memory *
				       * with scaling and limiting enabled   */
#define STARCOREop_MOVE_4F_LD   1347  /* Move four fractional words from     *
				       * memory to a register quad           */
#define STARCOREop_MOVES_4F_ST  1348  /* Move four fractional words to memory*
				       * with scaling and limiting enabled   */
#define STARCOREop_MOVET        1349  /* Move address register to address    *
				       * register if T-bit is 1              */
#define STARCOREop_MOVEF        1350  /* Move address register to address    *
				       * register if T-bit is 0              */
#define STARCOREop_VSL_4W       1351  /* Viterbi shift left: special move    *
				       * for Viterbi kernel                  */
#define STARCOREop_VSL_4F       1352  /* Viterbi shift left: special move    *
				       * for Viterbi kernel                  */
#define STARCOREop_VSL_2W       1353  /* Viterbi shift left: special move    *
				       * for Viterbi kernel                  */
#define STARCOREop_VSL_2F       1354  /* Viterbi shift left: special move    *
				       * for Viterbi kernel                  */


/*
 * Stack Support op extensions
 */

#define STARCOREop_POP          1401  /* Pop a register from the software    *
				       * stack                               */
#define STARCOREop_POPN         1402  /* Pop a register from the software    *
				       * stack using the normal stack        *
				       * pointer                             */
#define STARCOREop_PUSH         1403  /* Push a register onto the software   *
				       * stack                               */
#define STARCOREop_PUSHN        1404  /* Push a register onto the software   *
				       * stack using the normal stack        *
				       * pointer                             */
#define STARCOREop_TFRA_OSP     1405  /* Move the "other" stack pointer      *
				       * to/from a register, inversely       *
				       * defined by the exception mode       */


/*
 * Bit-mask op extensions
 */

#define STARCOREop_BMCHG        1501  /* Bit-mask change a 16-bit operand    */
#define STARCOREop_BMCHG_W_2    1502  /* Bit-mask change a 16-bit operand in *
				       * memory                              */
#define STARCOREop_BMCHG_W_3    1503  /* Bit-mask change a 16-bit operand in *
				       * memory                              */
#define STARCOREop_BMCLR        1504  /* Bit-mask clear a 16-bit operand     */
#define STARCOREop_BMCLR_W_2    1505  /* Bit-mask clear a 16-bit operand in  *
				       * memory                              */
#define STARCOREop_BMCLR_W_3    1506  /* Bit-mask clear a 16-bit operand in  *
				       * memory                              */
#define STARCOREop_BMSET        1507  /* Bit-mask set a 16-bit operand       */
#define STARCOREop_BMSET_W_2    1508  /* Bit-mask set a 16-bit operand in    *
				       * memory                              */
#define STARCOREop_BMSET_W_3    1509  /* Bit-mask set a 16-bit operand in    *
				       * memory                              */
#define STARCOREop_BMTSET       1510  /* Bit-mask test and set a 16-bit      *
				       * operand                             */
#define STARCOREop_BMTSET_W_2   1511  /* Bit-mask test and set a 16-bit      *
				       * operand in memory                   */
#define STARCOREop_BMTSET_W_3   1512  /* Bit-mask test and set a 16-bit      *
				       * operand in memory                   */
#define STARCOREop_BMTSTC       1513  /* Bit-mask test if clear - Sets the   *
				       * T-bit if every bit position that has*
				       * the value 1 in the mask is 0 in     *
				       * an operand                          */
#define STARCOREop_BMTSTC_W_2   1514  /* Bit-mask test if clear - Sets the   *
				       * T-bit if every bit position that has*
				       * the value 1 in the mask is 0 in     *
				       * an operand in memory                */
#define STARCOREop_BMTSTC_W_3   1515  /* Bit-mask test if clear - Sets the   *
				       * T-bit if every bit position that has*
				       * the value 1 in the mask is 0 in     *
				       * an operand in memory                */
#define STARCOREop_BMTSTS       1516  /* Bit-mask test if set - Sets the     *
				       * T-bit if every bit position that has*
				       * the value 1 in the mask is 1 in     *
				       * an operand                          */
#define STARCOREop_BMTSTS_W_2   1517  /* Bit-mask test if set - Sets the     *
				       * T-bit if every bit position that has*
				       * the value 1 in the mask is 1 in     *
				       * an operand in memory                */
#define STARCOREop_BMTSTS_W_3   1518  /* Bit-mask test if set - Sets the     *
				       * T-bit if every bit position that has*
				       * the value 1 in the mask is 1 in     *
				       * an operand in memory                */


/*
 * Change-of-flow op extensions
 */

#define STARCOREop_BF_1         1601  /* Branch if false                     */
#define STARCOREop_BF_2         1602  /* Branch if false                     */
#define STARCOREop_BFD_1        1603  /* Branch if false (delayed)           */
#define STARCOREop_BFD_2        1604  /* Branch if false (delayed)           */
#define STARCOREop_BRA_1        1605  /* Branch                              */
#define STARCOREop_BRA_2        1606  /* Branch                              */
#define STARCOREop_BRAD_1       1607  /* Branch (delayed)                    */
#define STARCOREop_BRAD_2       1608  /* Branch (delayed)                    */
#define STARCOREop_BSR_1        1609  /* Branch to subroutine                */
#define STARCOREop_BSR_2        1610  /* Branch to subroutine                */
#define STARCOREop_BSRD_1       1611  /* Branch to subroutine (delayed)      */
#define STARCOREop_BSRD_2       1612  /* Branch to subroutine (delayed)      */
#define STARCOREop_BT_1         1613  /* Branch if true                      */
#define STARCOREop_BT_2         1614  /* Branch if true                      */
#define STARCOREop_BTD_1        1615  /* Branch if true (delayed)            */
#define STARCOREop_BTD_2        1616  /* Branch if true (delayed)            */
#define STARCOREop_JF_1         1617  /* Jump if false                       */
#define STARCOREop_JF_3         1618  /* Jump if false                       */
#define STARCOREop_JFD_1        1619  /* Jump if false (delayed)             */
#define STARCOREop_JFD_3        1620  /* Jump if false (delayed)             */
#define STARCOREop_JMP_1        1621  /* Jump                                */
#define STARCOREop_JMP_3        1622  /* Jump                                */
#define STARCOREop_JMPD_1       1623  /* Jump (delayed)                      */
#define STARCOREop_JMPD_3       1624  /* Jump (delayed)                      */
#define STARCOREop_JSR_1        1625  /* Jump to subroutine                  */
#define STARCOREop_JSR_3        1626  /* Jump to subroutine                  */
#define STARCOREop_JSRD_1       1627  /* Jump to subroutine (delayed)        */
#define STARCOREop_JSRD_3       1628  /* Jump to subroutine (delayed)        */
#define STARCOREop_JT_1         1629  /* Jump if true                        */
#define STARCOREop_JT_3         1630  /* Jump if true                        */
#define STARCOREop_JTD_1        1631  /* Jump if true (delayed)              */
#define STARCOREop_JTD_3        1632  /* Jump if true (delayed)              */
#define STARCOREop_RTE          1633  /* Return from exception               */
#define STARCOREop_RTED         1634  /* Return from exception (delayed)     */
#define STARCOREop_RTS          1635  /* Return from subroutine              */
#define STARCOREop_RTSD         1636  /* Return from subroutine (delayed)    */
#define STARCOREop_RTSTK        1637  /* Force restore PC from the stack,    *
				       * updating SP                         */
#define STARCOREop_RTSTKD       1638  /* Force restore PC from the stack,    *
				       * updating SP (delayed)               */


/*
 * Loop op extensions
 */

#define STARCOREop_DOSETUP0     1701  /* Setup loop start address 0          */
#define STARCOREop_DOSETUP1     1702  /* Setup loop start address 1          */
#define STARCOREop_DOSETUP2     1703  /* Setup loop start address 2          */
#define STARCOREop_DOSETUP3     1704  /* Setup loop start address 3          */
#define STARCOREop_DOEN0        1705  /* Do enable - set loop counter 0 and  *
				       * enable loop 0 as a long loop        */
#define STARCOREop_DOEN1        1706  /* Do enable - set loop counter 1 and  *
				       * enable loop 1 as a long loop        */
#define STARCOREop_DOEN2        1707  /* Do enable - set loop counter 2 and  *
				       * enable loop 2 as a long loop        */
#define STARCOREop_DOEN3        1708  /* Do enable - set loop counter 3 and  *
				       * enable loop 3 as a long loop        */
#define STARCOREop_DOENSH0      1709  /* Do enable short - set loop counter 0*
				       * and enable loop 0 as a short loop   */
#define STARCOREop_DOENSH1      1710  /* Do enable short - set loop counter 1*
				       * and enable loop 1 as a short loop   */
#define STARCOREop_DOENSH2      1711  /* Do enable short- set loop counter 2 *
				       * and enable loop 2 as a short loop   */
#define STARCOREop_DOENSH3      1712  /* Do enable short- set loop counter 3 *
				       * and enable loop 3 as a short loop   */
#define STARCOREop_SKIPLS       1713  /* Test the active LC and skip the loop*
				       * if LCn is equal or smaller than 0   */
#define STARCOREop_BREAK        1714  /* Terminate the loop and branch to an *
				       * address                             */
#define STARCOREop_CONT         1715  /* Jump to the start of the loop to    *
				       * start the next iteration            */
#define STARCOREop_CONTD        1716  /* Jump to the start of the loop to    *
				       * start the next iteration (delayed)  */
#define STARCOREop_LPMARKA      1717  /* End-of-loop mark                    */
#define STARCOREop_LPMARKB      1718  /* End-of-loop mark                    */


/*
 * Program Control op extensions
 */

#define STARCOREop_NOP          1801  /* No operation                        */
#define STARCOREop_IFA          1802  /* Execute current execution set or    *
				       * subgroup unconditionally            */
#define STARCOREop_IFF          1803  /* Execute current execution set or    *
				       * subgroup if the T bit is clear      */
#define STARCOREop_IFT          1804  /* Execute current execution set or    *
				       * subgroup if the T bit is set        */
#define STARCOREop_DI           1805  /* Disable interrupts (sets the DI bit *
				       * in the status register              */
#define STARCOREop_EI           1806  /* Enable interrupts (clears the DI bit*
				       * in the status register)             */
#define STARCOREop_WAIT         1807  /* Wait for interrupt (low power       *
				       * stand-by)                           */
#define STARCOREop_STOP         1808  /* Stop processing (lowest power       *
				       * stand-by)                           */
#define STARCOREop_TRAP         1809  /* Execute a precise software exception*/
#define STARCOREop_ILLEGAL      1810  /* Trigger an imprecise illegal        *
				       * instruction exception               */
#define STARCOREop_DEBUG        1811  /* Enter debug mode                    */
#define STARCOREop_DEBUGEV      1812  /* Signal debug event                  */
#define STARCOREop_MARK         1813  /* Push the PC into the trace buffer   */


/*=========================================================================*/
/*
 *    Function prototypes
 */
/*=========================================================================*/

#ifdef __cplusplus
extern "C"
{
#endif

  extern int M_starcore_type_size (int mtype);
  extern int M_starcore_type_align (int mtype);
  extern void M_starcore_void (M_Type type);
  extern void M_starcore_bit_llong (M_Type type, int n);
  extern void M_starcore_bit_long (M_Type type, int n);
  extern void M_starcore_bit_int (M_Type type, int n);
  extern void M_starcore_bit_short (M_Type type, int n);
  extern void M_starcore_bit_char (M_Type type, int n);
  extern void M_starcore_char (M_Type type, int unsign);
  extern void M_starcore_short (M_Type type, int unsign);
  extern void M_starcore_int (M_Type type, int unsign);
  extern void M_starcore_long (M_Type type, int unsign);
  extern void M_starcore_llong (M_Type type, int unsign);
  extern void M_starcore_float (M_Type type, int unsign);
  extern void M_starcore_double (M_Type type, int unsign);
  extern void M_starcore_pointer (M_Type type);
  extern int M_starcore_eval_type (M_Type type, M_Type ntype);
  extern int M_starcore_eval_type2 (M_Type type, M_Type ntype);
  extern int M_starcore_call_type (M_Type type, M_Type ntype);
  extern int M_starcore_call_type2 (M_Type type, M_Type ntype);
  extern void M_starcore_array_layout (M_Type type, int *offset);
  extern int M_starcore_array_align (M_Type type);
  extern int M_starcore_array_size (M_Type type, int dim);
  extern void M_starcore_union_layout (int n, _M_Type * type, int *offset,
				       int *bit_offset);
  extern int M_starcore_union_align (int n, _M_Type * type);
  extern int M_starcore_union_size (int n, _M_Type * type);
  extern void M_starcore_struct_layout (int n, _M_Type * type, int *base,
					int *bit_offset);
  extern int M_starcore_struct_align (int n, _M_Type * type);
  extern int M_starcore_struct_size (int n, _M_Type * type, int struct_align);
  extern int M_starcore_layout_fnvar (List param_list, char **base_macro,
				      int *pcount, int purpose);
  extern int M_starcore_fnvar_layout (int n, _M_Type * type, long int *offset,
				      int *mode, int *reg, int *paddr,
				      char **macro, int *su_sreg, int *su_ereg,
				      int *pcount, int is_st, int purpose);
  extern int M_starcore_fnvar_to_lvar (_M_Type type, long int *offset,
				       char **base_macro, int local_space);
  extern int M_starcore_lvar_layout (int n, _M_Type * type, long int *offset,
				     char **base_macro);
  extern int M_starcore_no_short_int (void);
  extern int M_starcore_layout_order (void);
  extern void M_starcore_cb_label_name (char *fn, int cb, char *line, int len);
  extern int M_starcore_is_cb_label (char *label, char *fn, int *cb);

  extern void M_starcore_jumptbl_label_name (char *fn, int tbl_id, char *line,
					     int len);
  extern int M_starcore_is_jumptbl_label (char *label, char *fn, int *tbl_id);

  extern int M_starcore_structure_pointer (int purpose);
  extern int M_starcore_return_register (int type, int purpose);
  extern L_Operand *M_starcore_epilogue_cntr_register ();
  extern L_Operand *M_starcore_loop_cntr_register ();
  extern char *M_starcore_fn_label_name (char *label,
					 int (*is_func) (char *is_func_label));
  extern char *M_starcore_fn_name_from_label (char *label);
  extern void M_set_model_starcore (char *model_name);
  extern int M_starcore_fragile_macro (int macro_value);
  extern Set M_starcore_fragile_macro_set (void);
  extern int M_starcore_extra_pred_define_opcode (int proc_opc);
  extern int M_starcore_extra_pred_define_type1 (L_Oper *);
  extern int M_starcore_extra_pred_define_type2 (L_Oper *);
  extern int M_starcore_subroutine_call (int opc);

  extern int M_starcore_dataflow_macro (int id);
  extern void M_define_macros_starcore (STRING_Symbol_Table * sym_tbl);
  extern char *M_get_macro_name_starcore (int id);
  extern int M_oper_supported_in_arch_starcore (int opc);
  extern int M_num_oper_required_for_starcore (L_Oper * oper, char *name);
  extern int M_num_registers_starcore (int ctype);
  extern int M_is_stack_operand_starcore (L_Operand * operand);
  extern int M_is_unsafe_macro_starcore (L_Operand * operand);
  extern int M_operand_type_starcore (L_Operand * operand);
  extern int M_conflicting_operands_starcore (L_Operand * operand,
					      L_Operand * conflict_array[],
					      int len, int prepass);
  extern int M_opc_from_proc_opc_starcore (int proc_opc);

  extern int M_cannot_predicate_starcore (L_Oper *oper);

  extern void M_get_memory_operands_starcore (int *first, int *number, 
					      int proc_opc);

  extern void M_define_opcode_name_starcore (STRING_Symbol_Table *sym_tbl);
  extern char *M_get_opcode_name_starcore (int id);
  extern int M_memory_access_size_starcore (L_Oper *op);
  extern int M_get_data_type_starcore (L_Oper *op);
  extern int M_is_implicit_memory_op_starcore (L_Oper *oper);
  extern int M_starcore_coalescing_oper (L_Oper *);

  extern int M_EM_SC_model;

  extern void M_starcore_negate_compare (L_Oper *oper);

#ifdef __cplusplus
}
#endif

#endif
