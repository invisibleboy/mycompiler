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
 *      File :          l_opc.h
 *      Description :   Predefined Lcode opcodes.
 *      Author: Po-hua Chang, Wen-mei Hwu
 *      Creation Date:  June 1990
 *      Revised: Scott A. Mahlke, Feb. 1993.
 *
\*****************************************************************************/
#ifndef L_OPC_H
#define L_OPC_H

/* 10/29/02 REK Adding config.h */
#include <config.h>

/*
 *      Opcode Id's
 */

#define Lop_NO_OP               0

#define Lop_JSR                 1
#define Lop_JSR_FS              2
#define Lop_RTS                 3
#define Lop_RTS_FS              4
#define Lop_PROLOGUE            5
#define Lop_EPILOGUE            6
#define Lop_DEFINE              7
#define Lop_ALLOC               8

#define Lop_JUMP                10
#define Lop_JUMP_FS             11
#define Lop_JUMP_RG             12
#define Lop_JUMP_RG_FS          13

#define Lop_BR                  14      /* New conditional branch */
#define Lop_BR_F                15      /* Floating-point branch */

#define Lop_PHI			50
#define Lop_MU			51
#define Lop_GAMMA		52

#define Lop_PBR                 90
#define Lop_BR_UNCOND           91      /* gonna be nuked */
#define Lop_BR_CONDT            92      /* gonna be nuked */
#define Lop_BR_CONDF            93      /* gonna be nuked */
#define Lop_BR_LINK             94      /* gonna be nuked */

#define Lop_MOV                 95
#define Lop_MOV_F               96
#define Lop_MOV_F2              97

#define Lop_ADD                 99

#define Lop_L_MAC               100     /* Added by ITI/JWJ 8.11.1999 */
#define Lop_L_MSU               101
#define Lop_ADD_SAT             102
#define Lop_ADD_SAT_U           103
#define Lop_SUB_SAT             104
#define Lop_SUB_SAT_U           105
#define Lop_MUL_SAT             106
#define Lop_MUL_SAT_U           107
#define Lop_SAT                 108
#define Lop_SAT_U               109     /* End opcodes added by 
                                           ITI/JWJ 8.11.1999 */
#define Lop_ADD_U               111
#define Lop_SUB                 112
#define Lop_SUB_U               113
#define Lop_MUL                 114
#define Lop_MUL_U               115
#define Lop_DIV                 116
#define Lop_DIV_U               117
#define Lop_REM                 118
#define Lop_REM_U               119
#define Lop_ABS                 120
#define Lop_MUL_ADD             121
#define Lop_MUL_ADD_U           122
#define Lop_MUL_SUB             123
#define Lop_MUL_SUB_U           124
#define Lop_MUL_SUB_REV         125
#define Lop_MUL_SUB_REV_U       126
#define Lop_MAX                 127
#define Lop_MIN                 128

#define Lop_OR                  130
#define Lop_AND                 131
#define Lop_XOR                 132
#define Lop_NOR                 133
#define Lop_NAND                134
#define Lop_NXOR                135
#define Lop_OR_NOT              136
#define Lop_AND_NOT             137
#define Lop_OR_COMPL            138
#define Lop_AND_COMPL           139

#define Lop_LSL                 170
#define Lop_LSR                 171
#define Lop_ASR                 172
#define Lop_REV                 173
#define Lop_BIT_POS             174

#define Lop_ADD_F2              180
#define Lop_SUB_F2              181
#define Lop_MUL_F2              182
#define Lop_DIV_F2              183
#define Lop_RCP_F2              184
#define Lop_ABS_F2              185
#define Lop_MUL_ADD_F2          186
#define Lop_MUL_SUB_F2          187
#define Lop_MUL_SUB_REV_F2      188
#define Lop_SQRT_F2             189
#define Lop_MAX_F2              190
#define Lop_MIN_F2              191

#define Lop_ADD_F               210
#define Lop_SUB_F               211
#define Lop_MUL_F               212
#define Lop_DIV_F               213
#define Lop_RCP_F               214
#define Lop_ABS_F               215
#define Lop_MUL_ADD_F           216
#define Lop_MUL_SUB_F           217
#define Lop_MUL_SUB_REV_F       218
#define Lop_SQRT_F              219
#define Lop_MAX_F               220
#define Lop_MIN_F               221

#define Lop_F2_I                240
#define Lop_I_F2                241
#define Lop_F_I                 242
#define Lop_I_F                 243
#define Lop_F2_F                244
#define Lop_F_F2                245

#define Lop_ty_GLD_START        250
#define Lop_ty_LD_START         250
#define Lop_ty_LD_INT_START     250
#define Lop_LD_UC               250
#define Lop_LD_C                251
#define Lop_LD_UC2              252
#define Lop_LD_C2               253
#define Lop_LD_UI               254
#define Lop_LD_I                255
#define Lop_LD_Q                256
#define Lop_ty_LD_INT_END       256
#define Lop_LD_F                257
#define Lop_LD_F2               258
#define Lop_ty_LD_END           258

#define Lop_LD_PRE_UC           260
#define Lop_LD_PRE_C            261
#define Lop_LD_PRE_UC2          262
#define Lop_LD_PRE_C2           263
#define Lop_LD_PRE_UI           264
#define Lop_LD_PRE_I            265
#define Lop_LD_PRE_Q            266
#define Lop_LD_PRE_F            267
#define Lop_LD_PRE_F2           268

#define Lop_LD_POST_UC          270
#define Lop_LD_POST_C           271
#define Lop_LD_POST_UC2         272
#define Lop_LD_POST_C2          273
#define Lop_LD_POST_UI          274
#define Lop_LD_POST_I           275
#define Lop_LD_POST_Q           276
#define Lop_LD_POST_F           277
#define Lop_LD_POST_F2          278
#define Lop_ty_GLD_END          278

#define Lop_ST_C                280
#define Lop_ST_C2               281
#define Lop_ST_I                282
#define Lop_ST_Q                283
#define Lop_ST_F                284
#define Lop_ST_F2               285

#define Lop_ST_PRE_C            290
#define Lop_ST_PRE_C2           291
#define Lop_ST_PRE_I            292
#define Lop_ST_PRE_Q            293
#define Lop_ST_PRE_F            294
#define Lop_ST_PRE_F2           295

#define Lop_ST_POST_C           300
#define Lop_ST_POST_C2          301
#define Lop_ST_POST_I           302
#define Lop_ST_POST_Q           303
#define Lop_ST_POST_F           304
#define Lop_ST_POST_F2          305

#define Lop_EXTRACT_C           310
#define Lop_EXTRACT_C2          311
#define Lop_EXTRACT             312
#define Lop_EXTRACT_U           313
#define Lop_DEPOSIT             314

#define Lop_FETCH_AND_ADD       320
#define Lop_FETCH_AND_OR        321
#define Lop_FETCH_AND_AND       322
#define Lop_FETCH_AND_ST        323
#define Lop_FETCH_AND_COND_ST   324

#define Lop_ADVANCE             330
#define Lop_AWAIT               331
#define Lop_MUTEX_B             332
#define Lop_MUTEX_E             333

#define Lop_CO_PROC             340

#define Lop_CHECK               350
#define Lop_CONFIRM             351

/* Opcodes for predicated execution */
#define Lop_PRED_CLEAR          360
#define Lop_PRED_SET            361
#define Lop_PRED_LD             362
#define Lop_PRED_ST             363
#define Lop_PRED_LD_BLK         364
#define Lop_PRED_ST_BLK         365
#define Lop_PRED_MERGE          366
#define Lop_PRED_AND            367
#define Lop_PRED_COMPL          368
#define Lop_PRED_COPY           369

#define Lop_CMP                 400  /* predicate compare, integer           */
#define Lop_CMP_F               401  /* predicate compare, float             */
#define Lop_RCMP                402  /* register compare, integer            */
#define Lop_RCMP_F              403  /* register compare, float              */

/* Predicate mask defines */

#define Lop_PRED_MASK_AND       404
#define Lop_PRED_MASK_OR        405

/* Opcodes for limited predicated execution */
#define Lop_CMOV                410
#define Lop_CMOV_COM            411
#define Lop_CMOV_F              412
#define Lop_CMOV_COM_F          413
#define Lop_CMOV_F2             414
#define Lop_CMOV_COM_F2         415

#define Lop_SELECT              420
#define Lop_SELECT_F            421
#define Lop_SELECT_F2           422

#define Lop_PREF_LD             430

#define Lop_JSR_ND              440

#define Lop_EXPAND              450

/* Stuff added for Yoji's block prefetch stuff */
#define Lop_MEM_COPY            460
#define Lop_MEM_COPY_BACK       461
#define Lop_MEM_COPY_CHECK      462
#define Lop_MEM_COPY_RESET      463
#define Lop_MEM_COPY_SETUP      464
#define Lop_MEM_COPY_TAG        465

/* simulator directive */
#define Lop_SIM_DIR             470

/* region boundary place holder */
#define Lop_BOUNDARY            471

#define Lop_REMAP               480

#define Lop_BIT_EXTRACT         491
#define Lop_BIT_DEPOSIT         492

/* Intrinsic opcode -ITI/JWJ 6.23.1999 */
#define Lop_INTRINSIC           493

/* IA-64 inspired opcodes */

#define Lop_LSLADD              500
#define Lop_SXT_C               501
#define Lop_SXT_C2              502
#define Lop_SXT_I               503
#define Lop_ZXT_C               504
#define Lop_ZXT_C2              505
#define Lop_ZXT_I               506

/* CHK ops are provisional; see JWS */
#define Lop_ty_LD_CHK_START     510
#define Lop_ty_LD_CHK_INT_START 510
#define Lop_LD_UC_CHK           510
#define Lop_LD_C_CHK            511
#define Lop_LD_UC2_CHK          512
#define Lop_LD_C2_CHK           513
#define Lop_LD_UI_CHK           514
#define Lop_LD_I_CHK            515
#define Lop_LD_Q_CHK            516
#define Lop_ty_LD_CHK_INT_END   516
#define Lop_LD_F_CHK            517
#define Lop_LD_F2_CHK           518
#define Lop_ty_LD_CHK_END       518

#define Lop_CHECK_ALAT          519

/* KVM : Ops for 64bit emulation on a machine with narrower int size.
 */

#define Lop_ADD_CARRY           520 
#define Lop_ADD_CARRY_U         521
#define Lop_SUB_CARRY           522
#define Lop_SUB_CARRY_U         523
#define Lop_MUL_WIDE            524
#define Lop_MUL_WIDE_U          525

/* JWS 20000719
 * Vestigial opcodes -- replaced with Lop_CMP, Lop_RCMP, Lop_BR and completers.
 */

#define Lop_BEGIN_VESTIGIAL     600

#define Lop_BEGIN_VCOM          600
#define Lop_BEQ                 600
#define Lop_BEQ_FS              601
#define Lop_BNE                 602
#define Lop_BNE_FS              603
#define Lop_BGT                 604
#define Lop_BGT_FS              605
#define Lop_BLE                 606
#define Lop_BLE_FS              607
#define Lop_BGE                 608
#define Lop_BGE_FS              609
#define Lop_BLT                 610
#define Lop_BLT_FS              611
#define Lop_BGT_U               612
#define Lop_BGT_U_FS            613
#define Lop_BLE_U               614
#define Lop_BLE_U_FS            615
#define Lop_BGE_U               616
#define Lop_BGE_U_FS            617
#define Lop_BLT_U               618
#define Lop_BLT_U_FS            619
#define Lop_BEQ_F               620
#define Lop_BEQ_F_FS            621
#define Lop_BNE_F               622
#define Lop_BNE_F_FS            623
#define Lop_BGT_F               624
#define Lop_BGT_F_FS            625
#define Lop_BLE_F               626
#define Lop_BLE_F_FS            627
#define Lop_BGE_F               628
#define Lop_BGE_F_FS            629
#define Lop_BLT_F               630
#define Lop_BLT_F_FS            631
#define Lop_BEQ_F2              632
#define Lop_BEQ_F2_FS           633
#define Lop_BNE_F2              634
#define Lop_BNE_F2_FS           635
#define Lop_BGT_F2              636
#define Lop_BGT_F2_FS           637
#define Lop_BLE_F2              638
#define Lop_BLE_F2_FS           639
#define Lop_BGE_F2              640
#define Lop_BGE_F2_FS           641
#define Lop_BLT_F2              642
#define Lop_BLT_F2_FS           643
#define Lop_PRED_EQ             644
#define Lop_PRED_NE             645
#define Lop_PRED_GT             646
#define Lop_PRED_GT_U           647
#define Lop_PRED_GE             648
#define Lop_PRED_GE_U           649
#define Lop_PRED_LT             650
#define Lop_PRED_LT_U           651
#define Lop_PRED_LE             652
#define Lop_PRED_LE_U           653
#define Lop_PRED_EQ_F2          654
#define Lop_PRED_NE_F2          655
#define Lop_PRED_GT_F2          656
#define Lop_PRED_GE_F2          657
#define Lop_PRED_LT_F2          658
#define Lop_PRED_LE_F2          659
#define Lop_PRED_EQ_F           660
#define Lop_PRED_NE_F           661
#define Lop_PRED_GT_F           662
#define Lop_PRED_GE_F           663
#define Lop_PRED_LT_F           664
#define Lop_PRED_LE_F           665
#define Lop_EQ                  666
#define Lop_NE                  667
#define Lop_GT                  668
#define Lop_LE                  669
#define Lop_GE                  670
#define Lop_LT                  671
#define Lop_GT_U                672
#define Lop_LE_U                673
#define Lop_GE_U                674
#define Lop_LT_U                675
#define Lop_EQ_F                676
#define Lop_NE_F                677
#define Lop_GT_F                678
#define Lop_LE_F                679
#define Lop_GE_F                680
#define Lop_LT_F                681
#define Lop_EQ_F2               682
#define Lop_NE_F2               683
#define Lop_GT_F2               684
#define Lop_LE_F2               685
#define Lop_GE_F2               686
#define Lop_LT_F2               687
#define Lop_END_VCOM            687

#define Lop_END_VESTIGIAL       687

// SLARSEN: Vector instructions
#define Lop_VADD		688
#define Lop_VADD_U		689
#define Lop_VSUB		690
#define Lop_VSUB_U		691
#define Lop_VMUL		692
#define Lop_VMUL_U		693
#define Lop_VDIV		694
#define Lop_VDIV_U		695
#define Lop_VREM		696
#define Lop_VREM_U		697
#define Lop_VMAX		698
#define Lop_VMIN		699

#define Lop_VOR			700
#define Lop_VAND		701

#define Lop_VADD_F		702
#define Lop_VSUB_F		703
#define Lop_VMUL_F		704
#define Lop_VDIV_F		705
#define Lop_VABS_F		706
#define Lop_VSQRT_F		707
#define Lop_VMAX_F		708
#define Lop_VMIN_F		709

#define Lop_VADD_F2		710
#define Lop_VSUB_F2		711
#define Lop_VMUL_F2		712
#define Lop_VDIV_F2		713
#define Lop_VABS_F2		714
#define Lop_VSQRT_F2		715
#define Lop_VMAX_F2		716
#define Lop_VMIN_F2		717

#define Lop_VMOV		718
#define Lop_VMOV_F		719
#define Lop_VMOV_F2		720

#define Lop_VI_VF		721
#define Lop_VI_VF2		722
#define Lop_VF_VI		723
#define Lop_VF2_VI		724
#define Lop_VF_VF2		725
#define Lop_VF2_VF		726

#define Lop_VI_I		727
#define Lop_I_VI		728
#define Lop_VF_F		729
#define Lop_F_VF		730
#define Lop_VF2_F2		731
#define Lop_F2_VF2		732

#define Lop_VPERM		733
#define Lop_VPERM_F		734
#define Lop_VPERM_F2		735

#define Lop_VSPLAT		736
#define Lop_VSPLAT_F		737
#define Lop_VSPLAT_F2		738

#define Lop_VLD_UC		739
#define Lop_VLD_C		740
#define Lop_VLD_UC2		741
#define Lop_VLD_C2		742
#define Lop_VLD_I		743
#define Lop_VLD_F		744
#define Lop_VLD_F2		745

#define Lop_VST_C		746
#define Lop_VST_C2		747
#define Lop_VST_I		748
#define Lop_VST_F		749
#define Lop_VST_F2		750

#define Lop_VLDE_UC		751
#define Lop_VLDE_C		752
#define Lop_VLDE_UC2		753
#define Lop_VLDE_C2		754
#define Lop_VLDE_I		755
#define Lop_VLDE_F		756
#define Lop_VLDE_F2		757

#define Lop_VSTE_C		758
#define Lop_VSTE_C2		759
#define Lop_VSTE_I		760
#define Lop_VSTE_F		761
#define Lop_VSTE_F2		762

#define Lop_VEXTRACT_C		763
#define Lop_VEXTRACT_C2		764

/* RMR { lime opcodes */
#define Lop_MPOP              765
#define Lop_POP               775
#define Lop_PEEK               777
#define Lop_POP_I             766
#define Lop_POP_F             767
#define Lop_POP_F2            768

#define Lop_PEEK_I            769
#define Lop_PEEK_F            770
#define Lop_PEEK_F2           771

#define Lop_PUSH              776
#define Lop_PUSH_I            772
#define Lop_PUSH_F            773
#define Lop_PUSH_F2           774
/* } RMR */

/*
 * Set the variable Lop_LAST_OP to be equal to the last defined opcode
 * in Lcode
 */
#define Lop_LAST_OP             774

/*
 *      Opcode mnemonics
 */

#define Lopcode_NO_OP           "no_op"

#define Lopcode_JSR             "jsr"
#define Lopcode_JSR_FS          "jsr_fs"
#define Lopcode_RTS             "rts"
#define Lopcode_RTS_FS          "rts_fs"
#define Lopcode_PROLOGUE        "prologue"
#define Lopcode_EPILOGUE        "epilogue"
#define Lopcode_DEFINE          "define"
#define Lopcode_ALLOC           "alloc"

#define Lopcode_PHI		"phi"
#define Lopcode_MU		"mu"
#define Lopcode_GAMMA		"gamma"

#define Lopcode_JUMP            "jump"
#define Lopcode_JUMP_FS         "jump_fs"
#define Lopcode_JUMP_RG         "jump_rg"
#define Lopcode_JUMP_RG_FS      "jump_rg_fs"

#define Lopcode_BR              "br"
#define Lopcode_BR_F            "br_f"

#define Lopcode_BEQ             "beq"
#define Lopcode_BEQ_FS          "beq_fs"
#define Lopcode_BNE             "bne"
#define Lopcode_BNE_FS          "bne_fs"
#define Lopcode_BGT             "bgt"
#define Lopcode_BGT_FS          "bgt_fs"
#define Lopcode_BGE             "bge"
#define Lopcode_BGE_FS          "bge_fs"
#define Lopcode_BLT             "blt"
#define Lopcode_BLT_FS          "blt_fs"
#define Lopcode_BLE             "ble"
#define Lopcode_BLE_FS          "ble_fs"

#define Lopcode_BGT_U           "bgt_u"
#define Lopcode_BGT_U_FS        "bgt_u_fs"
#define Lopcode_BGE_U           "bge_u"
#define Lopcode_BGE_U_FS        "bge_u_fs"
#define Lopcode_BLT_U           "blt_u"
#define Lopcode_BLT_U_FS        "blt_u_fs"
#define Lopcode_BLE_U           "ble_u"
#define Lopcode_BLE_U_FS        "ble_u_fs"

#define Lopcode_BEQ_F           "beq_f"
#define Lopcode_BEQ_F_FS        "beq_f_fs"
#define Lopcode_BNE_F           "bne_f"
#define Lopcode_BNE_F_FS        "bne_f_fs"
#define Lopcode_BGT_F           "bgt_f"
#define Lopcode_BGT_F_FS        "bgt_f_fs"
#define Lopcode_BGE_F           "bge_f"
#define Lopcode_BGE_F_FS        "bge_f_fs"
#define Lopcode_BLT_F           "blt_f"
#define Lopcode_BLT_F_FS        "blt_f_fs"
#define Lopcode_BLE_F           "ble_f"
#define Lopcode_BLE_F_FS        "ble_f_fs"

#define Lopcode_BEQ_F2          "beq_f2"
#define Lopcode_BEQ_F2_FS       "beq_f2_fs"
#define Lopcode_BNE_F2          "bne_f2"
#define Lopcode_BNE_F2_FS       "bne_f2_fs"
#define Lopcode_BGT_F2          "bgt_f2"
#define Lopcode_BGT_F2_FS       "bgt_f2_fs"
#define Lopcode_BGE_F2          "bge_f2"
#define Lopcode_BGE_F2_FS       "bge_f2_fs"
#define Lopcode_BLT_F2          "blt_f2"
#define Lopcode_BLT_F2_FS       "blt_f2_fs"
#define Lopcode_BLE_F2          "ble_f2"
#define Lopcode_BLE_F2_FS       "ble_f2_fs"

#define Lopcode_PBR             "pbr"
#define Lopcode_BR_UNCOND       "br_uncond"     /* to be nuked */
#define Lopcode_BR_CONDT        "br_condt"      /* to be nuked */
#define Lopcode_BR_CONDF        "br_condf"      /* to be nuked */
#define Lopcode_BR_LINK         "br_link"       /* to be nuked */

#define Lopcode_MOV             "mov"
#define Lopcode_MOV_F           "mov_f"
#define Lopcode_MOV_F2          "mov_f2"

/* Added by ITI/JWJ 8.11.1999 */
#define Lopcode_L_MAC           "l_mac"
#define Lopcode_L_MSU           "l_msu"
#define Lopcode_ADD_SAT         "add_sat"
#define Lopcode_ADD_SAT_U       "add_sat_u"
#define Lopcode_SUB_SAT         "sub_sat"
#define Lopcode_SUB_SAT_U       "sub_sat_u"
#define Lopcode_MUL_SAT         "mul_sat"
#define Lopcode_MUL_SAT_U       "mul_sat_u"
#define Lopcode_SAT             "sat"
#define Lopcode_SAT_U           "sat_u"



#define Lopcode_ADD             "add"
#define Lopcode_ADD_U           "add_u"
#define Lopcode_SUB             "sub"
#define Lopcode_SUB_U           "sub_u"
#define Lopcode_MUL             "mul"
#define Lopcode_MUL_U           "mul_u"
#define Lopcode_DIV             "div"
#define Lopcode_DIV_U           "div_u"
#define Lopcode_REM             "rem"
#define Lopcode_REM_U           "rem_u"
#define Lopcode_ABS             "abs"
#define Lopcode_MUL_ADD         "mul_add"
#define Lopcode_MUL_ADD_U       "mul_add_u"
#define Lopcode_MUL_SUB         "mul_sub"
#define Lopcode_MUL_SUB_U       "mul_sub_u"
#define Lopcode_MUL_SUB_REV     "mul_sub_rev"
#define Lopcode_MUL_SUB_REV_U   "mul_sub_rev_u"
#define Lopcode_MAX             "max"
#define Lopcode_MIN             "min"

#define Lopcode_OR              "or"
#define Lopcode_AND             "and"
#define Lopcode_XOR             "xor"
#define Lopcode_NOR             "nor"
#define Lopcode_NAND            "nand"
#define Lopcode_NXOR            "nxor"
#define Lopcode_OR_NOT          "or_not"
#define Lopcode_AND_NOT         "and_not"
#define Lopcode_OR_COMPL        "or_compl"
#define Lopcode_AND_COMPL       "and_compl"

#define Lopcode_EQ              "eq"
#define Lopcode_NE              "ne"
#define Lopcode_GT              "gt"
#define Lopcode_GT_U            "gt_u"
#define Lopcode_GE              "ge"
#define Lopcode_GE_U            "ge_u"
#define Lopcode_LT              "lt"
#define Lopcode_LT_U            "lt_u"
#define Lopcode_LE              "le"
#define Lopcode_LE_U            "le_u"

#define Lopcode_LSL             "lsl"
#define Lopcode_LSR             "lsr"
#define Lopcode_ASR             "asr"
#define Lopcode_REV             "rev"
#define Lopcode_BIT_POS         "bit_pos"

#define Lopcode_ADD_F2          "add_f2"
#define Lopcode_SUB_F2          "sub_f2"
#define Lopcode_MUL_F2          "mul_f2"
#define Lopcode_DIV_F2          "div_f2"
#define Lopcode_RCP_F2          "rcp_f2"
#define Lopcode_ABS_F2          "abs_f2"
#define Lopcode_MUL_ADD_F2      "mul_add_f2"
#define Lopcode_MUL_SUB_F2      "mul_sub_f2"
#define Lopcode_MUL_SUB_REV_F2  "mul_sub_rev_f2"
#define Lopcode_SQRT_F2         "sqrt_f2"
#define Lopcode_MAX_F2          "max_f2"
#define Lopcode_MIN_F2          "min_f2"

#define Lopcode_EQ_F2           "eq_f2"
#define Lopcode_NE_F2           "ne_f2"
#define Lopcode_GT_F2           "gt_f2"
#define Lopcode_GE_F2           "ge_f2"
#define Lopcode_LT_F2           "lt_f2"
#define Lopcode_LE_F2           "le_f2"

#define Lopcode_ADD_F           "add_f"
#define Lopcode_SUB_F           "sub_f"
#define Lopcode_MUL_F           "mul_f"
#define Lopcode_DIV_F           "div_f"
#define Lopcode_RCP_F           "rcp_f"
#define Lopcode_ABS_F           "abs_f"
#define Lopcode_MUL_ADD_F       "mul_add_f"
#define Lopcode_MUL_SUB_F       "mul_sub_f"
#define Lopcode_MUL_SUB_REV_F   "mul_sub_rev_f"
#define Lopcode_SQRT_F          "sqrt_f"
#define Lopcode_MAX_F           "max_f"
#define Lopcode_MIN_F           "min_f"

#define Lopcode_EQ_F            "eq_f"
#define Lopcode_NE_F            "ne_f"
#define Lopcode_GT_F            "gt_f"
#define Lopcode_GE_F            "ge_f"
#define Lopcode_LT_F            "lt_f"
#define Lopcode_LE_F            "le_f"

#define Lopcode_F2_I            "f2_i"
#define Lopcode_I_F2            "i_f2"
#define Lopcode_F_I             "f_i"
#define Lopcode_I_F             "i_f"
#define Lopcode_F2_F            "f2_f"
#define Lopcode_F_F2            "f_f2"

#define Lopcode_LD_UC           "ld_uc"
#define Lopcode_LD_C            "ld_c"
#define Lopcode_LD_UC2          "ld_uc2"
#define Lopcode_LD_C2           "ld_c2"
#define Lopcode_LD_UI           "ld_ui"
#define Lopcode_LD_I            "ld_i"
#define Lopcode_LD_Q            "ld_q"
#define Lopcode_LD_F            "ld_f"
#define Lopcode_LD_F2           "ld_f2"

#define Lopcode_LD_PRE_UC       "ld_pre_uc"
#define Lopcode_LD_PRE_C        "ld_pre_c"
#define Lopcode_LD_PRE_UC2      "ld_pre_uc2"
#define Lopcode_LD_PRE_C2       "ld_pre_c2"
#define Lopcode_LD_PRE_UI       "ld_pre_ui"
#define Lopcode_LD_PRE_I        "ld_pre_i"
#define Lopcode_LD_PRE_Q        "ld_pre_q"
#define Lopcode_LD_PRE_F        "ld_pre_f"
#define Lopcode_LD_PRE_F2       "ld_pre_f2"

#define Lopcode_LD_POST_UC      "ld_post_uc"
#define Lopcode_LD_POST_C       "ld_post_c"
#define Lopcode_LD_POST_UC2     "ld_post_uc2"
#define Lopcode_LD_POST_C2      "ld_post_c2"
#define Lopcode_LD_POST_UI      "ld_post_ui"
#define Lopcode_LD_POST_I       "ld_post_i"
#define Lopcode_LD_POST_Q       "ld_post_q"
#define Lopcode_LD_POST_F       "ld_post_f"
#define Lopcode_LD_POST_F2      "ld_post_f2"

#define Lopcode_ST_C            "st_c"
#define Lopcode_ST_C2           "st_c2"
#define Lopcode_ST_I            "st_i"
#define Lopcode_ST_Q            "st_q"
#define Lopcode_ST_F            "st_f"
#define Lopcode_ST_F2           "st_f2"

#define Lopcode_ST_PRE_C        "st_pre_c"
#define Lopcode_ST_PRE_C2       "st_pre_c2"
#define Lopcode_ST_PRE_I        "st_pre_i"
#define Lopcode_ST_PRE_Q        "st_pre_q"
#define Lopcode_ST_PRE_F        "st_pre_f"
#define Lopcode_ST_PRE_F2       "st_pre_f2"

#define Lopcode_ST_POST_C       "st_post_c"
#define Lopcode_ST_POST_C2      "st_post_c2"
#define Lopcode_ST_POST_I       "st_post_i"
#define Lopcode_ST_POST_Q       "st_post_q"
#define Lopcode_ST_POST_F       "st_post_f"
#define Lopcode_ST_POST_F2      "st_post_f2"

#define Lopcode_EXTRACT_C       "extract_c"
#define Lopcode_EXTRACT_C2      "extract_c2"
#define Lopcode_EXTRACT         "extr"
#define Lopcode_EXTRACT_U       "extr_u"
#define Lopcode_DEPOSIT         "dep"

#define Lopcode_FETCH_AND_ADD   "fetch_and_add"
#define Lopcode_FETCH_AND_OR    "fetch_and_or"
#define Lopcode_FETCH_AND_AND   "fetch_and_and"
#define Lopcode_FETCH_AND_ST    "fetch_and_st"
#define Lopcode_FETCH_AND_COND_ST "fetch_and_cond_st"

#define Lopcode_ADVANCE         "advance"
#define Lopcode_AWAIT           "await"
#define Lopcode_MUTEX_B         "mutex_b"
#define Lopcode_MUTEX_E         "mutex_e"

#define Lopcode_CO_PROC         "co_proc"

#define Lopcode_CHECK           "check"
#define Lopcode_CONFIRM         "confirm"

#define Lopcode_PRED_CLEAR      "pred_clear"
#define Lopcode_PRED_SET        "pred_set"
#define Lopcode_PRED_LD         "pred_ld"
#define Lopcode_PRED_ST         "pred_st"
#define Lopcode_PRED_LD_BLK     "pred_ld_blk"
#define Lopcode_PRED_ST_BLK     "pred_st_blk"
#define Lopcode_PRED_MERGE      "pred_merge"
#define Lopcode_PRED_AND        "pred_and"
#define Lopcode_PRED_COMPL      "pred_compl"
#define Lopcode_PRED_COPY       "pred_copy"

#define Lopcode_CMP             "cmp"
#define Lopcode_CMP_F           "cmp_f"
#define Lopcode_RCMP            "rcmp"
#define Lopcode_RCMP_F          "rcmp_f"

#define Lopcode_PRED_EQ         "pred_eq"
#define Lopcode_PRED_NE         "pred_ne"
#define Lopcode_PRED_GT         "pred_gt"
#define Lopcode_PRED_GT_U       "pred_gt_u"
#define Lopcode_PRED_GE         "pred_ge"
#define Lopcode_PRED_GE_U       "pred_ge_u"
#define Lopcode_PRED_LT         "pred_lt"
#define Lopcode_PRED_LT_U       "pred_lt_u"
#define Lopcode_PRED_LE         "pred_le"
#define Lopcode_PRED_LE_U       "pred_le_u"
#define Lopcode_PRED_EQ_F2      "pred_eq_f2"
#define Lopcode_PRED_NE_F2      "pred_ne_f2"
#define Lopcode_PRED_GT_F2      "pred_gt_f2"
#define Lopcode_PRED_GE_F2      "pred_ge_f2"
#define Lopcode_PRED_LT_F2      "pred_lt_f2"
#define Lopcode_PRED_LE_F2      "pred_le_f2"
#define Lopcode_PRED_EQ_F       "pred_eq_f"
#define Lopcode_PRED_NE_F       "pred_ne_f"
#define Lopcode_PRED_GT_F       "pred_gt_f"
#define Lopcode_PRED_GE_F       "pred_ge_f"
#define Lopcode_PRED_LT_F       "pred_lt_f"
#define Lopcode_PRED_LE_F       "pred_le_f"

#define Lopcode_PRED_MASK_AND   "pred_mask_and"
#define Lopcode_PRED_MASK_OR    "pred_mask_or"

#define Lopcode_CMOV            "cmov"
#define Lopcode_CMOV_COM        "cmov_com"
#define Lopcode_CMOV_F          "cmov_f"
#define Lopcode_CMOV_COM_F      "cmov_com_f"
#define Lopcode_CMOV_F2         "cmov_f2"
#define Lopcode_CMOV_COM_F2     "cmov_com_f2"

#define Lopcode_SELECT          "select"
#define Lopcode_SELECT_F        "select_f"
#define Lopcode_SELECT_F2       "select_f2"

#define Lopcode_PREF_LD         "pref_ld"

#define Lopcode_JSR_ND          "jsr_nd"

#define Lopcode_EXPAND          "expand"

/* Stuff added for Yoji's block prefetch stuff */
#define Lopcode_MEM_COPY        "mem_copy"
#define Lopcode_MEM_COPY_BACK   "mem_copy_back"
#define Lopcode_MEM_COPY_CHECK  "mem_copy_check"
#define Lopcode_MEM_COPY_RESET  "mem_copy_reset"
#define Lopcode_MEM_COPY_SETUP  "mem_copy_setup"
#define Lopcode_MEM_COPY_TAG    "mem_copy_tag"

#define Lopcode_SIM_DIR         "sim_dir"

#define Lopcode_BOUNDARY        "boundary"

#define Lopcode_REMAP           "remap"

#define Lopcode_BIT_EXTRACT     "bit_extract"
#define Lopcode_BIT_DEPOSIT     "bit_deposit"

/* Intrinsic opcode -ITI/JWJ 6.23.1999 */
#define Lopcode_INTRINSIC       "intrinsic"

#define Lopcode_LSLADD          "lsladd"
#define Lopcode_SXT_C           "sxt_c"
#define Lopcode_SXT_C2          "sxt_c2"
#define Lopcode_SXT_I           "sxt_i"
#define Lopcode_ZXT_C           "zxt_c"
#define Lopcode_ZXT_C2          "zxt_c2"
#define Lopcode_ZXT_I           "zxt_i"

#define Lopcode_LD_UC_CHK       "ld_uc_chk"
#define Lopcode_LD_C_CHK        "ld_c_chk"
#define Lopcode_LD_UC2_CHK      "ld_uc2_chk"
#define Lopcode_LD_C2_CHK       "ld_c2_chk"
#define Lopcode_LD_UI_CHK       "ld_ui_chk"
#define Lopcode_LD_I_CHK        "ld_i_chk"
#define Lopcode_LD_Q_CHK        "ld_q_chk"
#define Lopcode_LD_F_CHK        "ld_f_chk"
#define Lopcode_LD_F2_CHK       "ld_f2_chk"
#define Lopcode_CHECK_ALAT      "check_alat"

/* KVM : Ops for 64bit emulation on a machine with narrower int size.
 */
#define Lopcode_ADD_CARRY       "add_carry"        
#define Lopcode_ADD_CARRY_U     "add_carry_u"       
#define Lopcode_SUB_CARRY       "sub_carry"       
#define Lopcode_SUB_CARRY_U     "sub_carry_u"       
#define Lopcode_MUL_WIDE        "mul_wide"       
#define Lopcode_MUL_WIDE_U      "mul_wide_u"       

/* Comparison completers (com[1]) */

/* These are related very specifically to each other --
   change only with extreme care!! (see l_compare.c) */

#define Lcmp_COM_EQ            0x02u
#define Lcmp_COM_NE            0x03u
#define Lcmp_COM_GT            0x04u
#define Lcmp_COM_LE            0x05u
#define Lcmp_COM_GE            0x06u
#define Lcmp_COM_LT            0x07u
#define Lcmp_COM_TZ            0x08u
#define Lcmp_COM_TN            0x09u

#define Lcompl_COM_EQ          "eq"
#define Lcompl_COM_NE          "ne"
#define Lcompl_COM_GT          "gt"
#define Lcompl_COM_LE          "le"
#define Lcompl_COM_GE          "ge"
#define Lcompl_COM_LT          "lt"
#define Lcompl_COM_TZ          "tz"
#define Lcompl_COM_TN          "tn"

#define L_opc_vestigial(opc) ((((opc) >= Lop_BEGIN_VESTIGIAL) && \
                               ((opc) <= Lop_END_VESTIGIAL)) ? 1 : 0)

#define L_opc_vestigial_compare(opc)((((opc) >= Lop_BEGIN_VCOM) && \
                                      ((opc) <= Lop_END_VCOM)) ? 1 : 0)

// SLARSEN: Vector instructions
#define Lopcode_VADD		"vadd"
#define Lopcode_VADD_U		"vadd_u"
#define Lopcode_VSUB		"vsub"
#define Lopcode_VSUB_U		"vsub_u"
#define Lopcode_VMUL		"vmul"
#define Lopcode_VMUL_U		"vmul_u"
#define Lopcode_VDIV		"vdiv"
#define Lopcode_VDIV_U		"vdiv_u"
#define Lopcode_VREM		"vrem"
#define Lopcode_VREM_U		"vrem_u"
#define Lopcode_VMIN		"vmin"
#define Lopcode_VMAX		"vmax"

#define Lopcode_VOR		"vor"
#define Lopcode_VAND		"vand"

#define Lopcode_VADD_F		"vadd_f"
#define Lopcode_VSUB_F		"vsub_f"
#define Lopcode_VMUL_F		"vmul_f"
#define Lopcode_VDIV_F		"vdiv_f"
#define Lopcode_VABS_F		"vabs_f"
#define Lopcode_VSQRT_F		"vsqrt_f"
#define Lopcode_VMAX_F		"vmax_f"
#define Lopcode_VMIN_F		"vmin_f"

#define Lopcode_VADD_F2		"vadd_f2"
#define Lopcode_VSUB_F2		"vsub_f2"
#define Lopcode_VMUL_F2		"vmul_f2"
#define Lopcode_VDIV_F2		"vdiv_f2"
#define Lopcode_VABS_F2		"vabs_f2"
#define Lopcode_VSQRT_F2	"vsqrt_f2"
#define Lopcode_VMAX_F2		"vmax_f2"
#define Lopcode_VMIN_F2		"vmin_f2"

#define Lopcode_VMOVE		"vmove"
#define Lopcode_VMOVE_F		"vmove_f"
#define Lopcode_VMOVE_F2	"vmove_f2"

#define Lopcode_VI_VF		"vi_vf"
#define Lopcode_VI_VF2		"vi_vf2"
#define Lopcode_VF_VI		"vf_vi"
#define Lopcode_VF2_VI		"vf2_vi"
#define Lopcode_VF_VF2		"vf_vf2"
#define Lopcode_VF2_VF		"vf2_vf"

#define Lopcode_VI_I		"vi_i"
#define Lopcode_I_VI		"i_vi"
#define Lopcode_VF_F		"vf_f"
#define Lopcode_F_VF		"f_vf"
#define Lopcode_VF2_F2		"vf2_f2"
#define Lopcode_F2_VF2		"f2_vf2"

#define Lopcode_VPERM		"vperm"
#define Lopcode_VPERM_F		"vperm_f"
#define Lopcode_VPERM_F2	"vperm_f2"

#define Lopcode_VSPLAT		"vsplat"
#define Lopcode_VSPLAT_F	"vsplat_f"
#define Lopcode_VSPLAT_F2	"vsplat_f2"

#define Lopcode_VLD_UC		"vld_uc"
#define Lopcode_VLD_C		"vld_c"
#define Lopcode_VLD_UC2		"vld_uc2"
#define Lopcode_VLD_C2		"vld_c2"
#define Lopcode_VLD_I		"vld_i"
#define Lopcode_VLD_F		"vld_f"
#define Lopcode_VLD_F2		"vld_f2"

#define Lopcode_VST_C		"vst_c"
#define Lopcode_VST_C2		"vst_c2"
#define Lopcode_VST_I		"vst_i"
#define Lopcode_VST_F		"vst_f"
#define Lopcode_VST_F2		"vst_f2"

#define Lopcode_VLDE_UC		"vlde_uc"
#define Lopcode_VLDE_C		"vlde_c"
#define Lopcode_VLDE_UC2	"vlde_uc2"
#define Lopcode_VLDE_C2		"vlde_c2"
#define Lopcode_VLDE_I		"vlde_i"
#define Lopcode_VLDE_F		"vlde_f"
#define Lopcode_VLDE_F2		"vlde_f2"

#define Lopcode_VSTE_C		"vste_c"
#define Lopcode_VSTE_C2		"vste_c2"
#define Lopcode_VSTE_I		"vste_i"
#define Lopcode_VSTE_F		"vste_f"
#define Lopcode_VSTE_F2		"vste_f2"

#define Lopcode_VEXTRACT_C	"vextract_c"
#define Lopcode_VEXTRACT_C2	"vextract_c2"

/* RMR { lime opcodes */
#define Lopcode_MPOP          "mpop"
#define Lopcode_POP           "pop"
#define Lopcode_PEEK           "peek"
#define Lopcode_POP_I         "pop_i"
#define Lopcode_POP_F         "pop_f"
#define Lopcode_POP_F2        "pop_f2"

#define Lopcode_PEEK_I        "peek_i"
#define Lopcode_PEEK_F        "peek_f"
#define Lopcode_PEEK_F2       "peek_f2"

#define Lopcode_PUSH          "push"
#define Lopcode_PUSH_I        "push_i"
#define Lopcode_PUSH_F        "push_f"
#define Lopcode_PUSH_F2       "push_f2"
/* } RMR */

#endif
