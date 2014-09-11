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
 *      File :          l_flags.h
 *      Description :   Bit field flag manipulations
 *      Date :          February 1993
 *      Author :        Scott Mahlke, Wen-mei Hwu
\*****************************************************************************/
#ifndef L_FLAGS_H
#define L_FLAGS_H

/* 10/29/02 REK Adding config.h */
#include <config.h>

/*
 *      Generic maninuplations, Bit order 32...1, val must be 0 or 1
 */

#define L_SET_BIT_FLAG(flag, val) ((flag) | (val))
#define L_CLR_BIT_FLAG(flag, val) ((flag) & (~(val)))
#define L_EXTRACT_BIT_VAL(flag, val) (((flag) & (val))==0 ? 0 : 1)


/*========================================================================*/
/*
 *      L_Oper
 */
/*========================================================================*/

#define L_OPER_MAX_FLAGS                        32

/* printed char */
#define L_OPER_SPECULATIVE                      0x00000001      /* S    */
#define L_OPER_PARENT                           0x00000002      /* -    */
#define L_OPER_SQUASHING                        0x00000004      /* Q    */
#define L_OPER_CAN_BE_DEMOTED                   0x00000008      /* -    */
#define L_OPER_CHECK                            0x00000010      /* C    */
#define L_OPER_SPILL_CODE                       0x00000020      /* R    */
#define L_OPER_PROMOTED                         0x00000040      /* P    */
#define L_OPER_SAFE_PEI                         0x00000080      /* F    */
#define L_OPER_MASK_PE                          0x00000100      /* M    */
#define L_OPER_SIDE_EFFECT_FREE                 0x00000200      /* E    */
#define L_OPER_LABEL_REFERENCE                  0x00000400      /* L    */
#define L_OPER_DATA_SPECULATIVE                 0x00000800      /* D    */
#define L_OPER_PROBE_MARK                       0x00001000      /* X    */
#define L_OPER_SYNC                             0x00002000      /* Y    */
#define L_OPER_PROCESSOR_SPECIFIC               0x00004000      /* ?    */
#define L_OPER_IS_CMOV                          0x00008000      /* -    */
#define L_OPER_VOLATILE                         0x00010000      /* V    */
#define L_OPER_NO_SPECULATION                   0x00020000      /* N    */
#define L_OPER_STACK_REFERENCE                  0x00040000      /* K    */
#define L_OPER_ROTATE_REGISTERS                 0x00080000      /* T    */
#define L_OPER_SUPER_SPECULATION                0x00100000      /* A    */

#define L_OPER_PRINT_CYCLE_DELIMITER            0x00800000      /* -    */
#define L_OPER_RESERVED_TEMP1                   0x01000000      /* -    */
#define L_OPER_RESERVED_TEMP2                   0x02000000      /* -    */
#define L_OPER_RESERVED_TEMP3                   0x04000000      /* -    */
#define L_OPER_RESERVED_TEMP4                   0x08000000      /* -    */
#define L_OPER_RESERVED_TEMP5                   0x10000000      /* -    */
#define L_OPER_RESERVED_TEMP6                   0x20000000      /* -    */
#define L_OPER_RESERVED_TEMP7                   0x40000000      /* -    */
#define L_OPER_RESERVED_TEMP8                   0x80000000      /* -    */

/* This bits should be set for the above oper flags that do not print */

#define L_OPER_PRINTED_FLAGS    (L_OPER_SPECULATIVE |\
                                 L_OPER_SQUASHING |\
                                 L_OPER_CHECK |\
                                 L_OPER_SPILL_CODE |\
                                 L_OPER_PROMOTED |\
                                 L_OPER_SAFE_PEI |\
                                 L_OPER_MASK_PE |\
                                 L_OPER_SIDE_EFFECT_FREE |\
                                 L_OPER_LABEL_REFERENCE |\
                                 L_OPER_DATA_SPECULATIVE |\
                                 L_OPER_PROBE_MARK |\
                                 L_OPER_SYNC |\
                                 L_OPER_PROCESSOR_SPECIFIC|\
                                 L_OPER_VOLATILE|\
                                 L_OPER_NO_SPECULATION|\
                                 L_OPER_STACK_REFERENCE|\
                                 L_OPER_ROTATE_REGISTERS|\
                                 L_OPER_SUPER_SPECULATION)

#define L_OPER_HIDDEN_FLAGS     ~(L_OPER_PRINTED_FLAGS)


/* Oper flag explanation, printed character in Lcode is in () before
 *      flag name.  A "(-)" indicates the flag is not printed.
 *
 *   (S) L_OPER_SPECULATIVE - control speculative - oper is moved above a
 *              control dependent branch.
 *   (-) L_OPER_PARENT - For Mcode generation, oper is an original Lcode op
 *   (Q) L_OPER_SQUASHING - Branch oper squashes next instr when it is
 *              mispredicted.
 *   (-) L_OPER_CAN_BE_DEMOTED - oper can legally be demoted to its original
 *              predicate, so analysis can use the original predicate.
 *   (C) L_OPER_CHECK - check instruction for MCB or sentinel scheduling
 *   (R) L_OPER_SPILL_CODE - instruction inserted by the register allocator,
 *              either spill/fill code or caller-callee save/restore code.
 *   (P) L_OPER_PROMOTED - instruction's predicate is advanced to higher level
 *   (F) L_OPER_SAFE_PEI - flag used to indicate which potentially excepting
 *              instructions are SAFE for speculation.
 *   (M) L_OPER_MASK_PE - code generator should mask (disable) any exceptions
 *              caused by this instruction.
 *   (E) L_OPER_SIDE_EFFECT_FREE - instruction doesnt produce any unknown side
 *              effects, only used for JSR's right now.
 *   (L) L_OPER_LABEL_REFERENCE - base address of memory op is a global label
 *   (D) L_OPER_DATA_SPECULATIVE - oper is executed before a preceding store
 *              which may reference the same location. 
 *   (Y) L_OPER_SYNC - oper behaves as a synchronization instructions. Examples
 *              jsrs to setjmp and longjmp, moving code above or below these
 *              ops is dangerous as well as optimizing across them.
 *   (?) L_OPER_PROCESSOR_SPECIFIC - the semantics of this operation do not
 *              obey Lcode conventions.  Any applied optimizations cannot 
 *              make any assumptions about its behavior based on its opcode
 *              or operand format.
 *   (V) L_OPER_VOLATILE -  The operation is accessing a memory location
 *              associated with a volatile variable and cannot be 
 *              eliminated via optimization.
 *   (T) L_OPER_ROTATE_REGISTERS - the operation causes the rotation
 *              of registers within the rotation windows.
 *              Used for software pipelined loops.
 *   (-) L_OPER_PRINT_CYCLE_DELIMITER - Setting this flag on a operation will
 *              cause an additional \n to be printed before the operation
 *              to allow textual grouping of instructions issue in the same 
 *              cycle.
 *   (-) L_OPER_RESERVED_TEMP1-8 - These are reserved as temporary oper flags
 *              to be used by Lcode modules to define as they like.
 *              NONE of these are to be printed out!!
 */


/*========================================================================*/
/*
 *      L_Expression
 */
/*========================================================================*/

#define L_EXPRESSION_MOVE_NUM_CONST             0x00000001
#define L_EXPRESSION_MOVE                       0x00000002
#define L_EXPRESSION_SINGLE_SOURCE              0x00000004

#define L_EXPRESSION_MEMORY                     0x00000010


/*========================================================================*/
/*
 *      L_Cb
 */
/*========================================================================*/

#define L_CB_MAX_FLAGS                          32

/* printed char */
#define L_CB_PRESENCE                           0x00000001      /* -    */
#define L_CB_ROT_REG_ALLOCATED                  0x00000002      /* T    */
#define L_CB_VISITED                            0x00000004      /* -    */
#define L_CB_SUB_CALL                           0x00000010      /* -    */
#define L_CB_GENERAL_SUB_CALL                   0x00000020      /* -    */
#define L_CB_SYNC                               0x00000040      /* -    */
#define L_CB_STORE                              0x00000080      /* -    */
#define L_CB_IS_DEAD                            0x00000100      /* -    */
#define L_CB_VISITED2                           0x00000200      /* -    */
#define L_CB_SUPERBLOCK                         0x00000400      /* S    */
#define L_CB_HYPERBLOCK                         0x00000800      /* H    */
#define L_CB_LOOP_HEADER                        0x00001000      /* -    */
#define L_CB_HYPERBLOCK_NO_FALLTHRU             0x00002000      /* N    */
#define L_CB_HAS_JRG                            0x00004000      /* -    */
#define L_CB_HYPERBLOCK_LOOP                    0x00008000      /* -    */
#define L_CB_HYPERBLOCK_HAMMOCK                 0x00010000      /* -    */
#define L_CB_SOFTPIPE                           0x00020000      /* P    */
#define L_CB_UNROLLED                           0x00040000      /* U    */
#define L_CB_ENTRANCE_BOUNDARY                  0x00080000      /* E    */
#define L_CB_EXIT_BOUNDARY                      0x00100000      /* X    */
#define L_CB_PROLOGUE                           0x00200000      /* R    */
#define L_CB_EPILOGUE                           0x00400000      /* I    */
#define L_CB_VIOLATES_LC_SEMANTICS              0x00800000      /* V    */
#define L_CB_RESERVED_TEMP1                     0x01000000      /* -    */
#define L_CB_RESERVED_TEMP2                     0x02000000      /* -    */
#define L_CB_RESERVED_TEMP3                     0x04000000      /* -    */
#define L_CB_RESERVED_TEMP4                     0x08000000      /* -    */
#define L_CB_RESERVED_TEMP5                     0x10000000      /* -    */
#define L_CB_RESERVED_TEMP6                     0x20000000      /* -    */
#define L_CB_RESERVED_TEMP7                     0x40000000      /* -    */
#define L_CB_RESERVED_TEMP8                     0x80000000      /* -    */

/* These bits should be set for the above cb flags that do not print */
#define L_CB_PRINTED            (L_CB_SUPERBLOCK |\
                                 L_CB_HYPERBLOCK |\
                                 L_CB_HYPERBLOCK_NO_FALLTHRU |\
                                 L_CB_SOFTPIPE |\
                                 L_CB_UNROLLED |\
                                 L_CB_ENTRANCE_BOUNDARY |\
                                 L_CB_EXIT_BOUNDARY|\
                                 L_CB_PROLOGUE|\
                                 L_CB_EPILOGUE|\
                                 L_CB_ROT_REG_ALLOCATED|\
                                 L_CB_VIOLATES_LC_SEMANTICS)
#define L_CB_HIDDEN_FLAGS       ~(L_CB_PRINTED)

#define L_CB_BOUNDARY   (L_CB_ENTRANCE_BOUNDARY|L_CB_EXIT_BOUNDARY)

/* Cb flag explanation, printed character in Lcode is in () before
 *      flag name.  A "(-)" indicates the flag is not printed.
 *
 *   (-) L_CB_PRESENCE - cb is already present in the Lcode, used for reading
 *              in Lcode for error checking
 *   (T) L_CB_ROT_REG_ALLOCATED - cb contains rotating registers
 *   (-) L_CB_VISITED - generic visited flag for cb search algorithms
 *   (-) L_CB_SUB_CALL - to be moved to Lopti
 *   (-) L_CB_GENERAL_SUB_CALL - to be moved to Lopti
 *   (-) L_CB_SYNC - to be moved to Lopti
 *   (-) L_CB_STORE - to be moved to Lopti
 *   (-) L_CB_IS_DEAD - to be moved to Lopti
 *   (-) L_CB_VISITED2 - second generic visited flag for cb search algorithms
 *   (S) L_CB_SUPERBLOCK - cb is a superblock.
 *   (H) L_CB_HYPERBLOCK - cb is a hyperblock.
 *   (-) L_CB_LOOP_HEADER - cb is the header (entry point) of a loop.
 *   (N) L_CB_HYPERBLOCK_NO_FALLTHRU - hyperblock has no fallthru path
 *              to next sequential cb.  This is not trivially determined in
 *              some cases so use the flag to determine it 1x.
 *   (-) L_CB_HAS_JRG - cb contains a jump_rg or jump_rg_fs
 *   (-) L_CB_HYPERBLOCK_LOOP - to be moved to Lhyper
 *   (-) L_CB_HYPERBLOCK_HAMMOCK - to be moved to Lhyper
 *   (P) L_CB_SOFTPIPE - cb is to be software pipelined scheduled rather
 *              than unrolled and scheduled acyclicly
 *   (U) L_CB_UNROLLED - cb is a superblock loop which has been unrolled by
 *              Lsuperscalar
 *   (E) L_CB_ENTRANCE_BOUNDARY - cb is an entrance boundary cb for a 
 *              function encapsulated region.
 *   (X) L_CB_EXIT_BOUNDARY - cb is an exit boundary cb for a 
 *              function encapsulated region.
 *   (R) L_CB_PROLOGUE - cb is the entry cb for the function
 *   (I) L_CB_EPILOGUE - cb is the exit point for the function
 *   (V) L_CB_VIOLATES_LC_SEMANTICS - cb violates the conventional Lcode 
 *              semantics, so analysis should be very conservative for these 
 *              blocks.  A common violation is created by generating kernel 
 *              only code with rotating registers by a modulo scheduler.
 *   (-) L_CB_RESERVED_TEMP1-8 - These are reserved as temporary cb flags
 *              to be used by Lcode modules to define as they like.
 *              NONE of these are to be printed out!!
 */


/*========================================================================*/
/*
 *      L_Func
 */
/*========================================================================*/

#define L_FUNC_MAX_FLAGS                        32

/* printed char */
#define L_FUNC_HYPERBLOCK                       0x00000001      /* H    */
#define L_FUNC_LEAF                             0x00000002      /* L    */
#define L_FUNC_SIDE_EFFECT_FREE                 0x00000004      /* E    */
#define L_FUNC_REGISTER_ALLOCATED               0x00000008      /* R    */
#define L_FUNC_SUPERBLOCK                       0x00000010      /* S    */
#define L_FUNC_MASK_PE                          0x00000020      /* M    */
#define L_FUNC_COMPILATION_COMPLETE             0x00000040      /* C    */
#define L_FUNC_SCHEDULED                        0x00000080      /* D    */
#define L_FUNC_CC_IN_PREDICATE_REGS             0x00000100      /* P    */
#define L_FUNC_PRED_REGS_IN_ATTR                0x00000200      /* A    */
#define L_FUNC_ROT_REG_ALLOCATED                0x00000400      /* T    */
#define L_FUNC_RESERVED_TEMP1                   0x01000000      /* -    */
#define L_FUNC_RESERVED_TEMP2                   0x02000000      /* -    */
#define L_FUNC_RESERVED_TEMP3                   0x04000000      /* -    */
#define L_FUNC_RESERVED_TEMP4                   0x08000000      /* -    */
#define L_FUNC_RESERVED_TEMP5                   0x10000000      /* -    */
#define L_FUNC_RESERVED_TEMP6                   0x20000000      /* -    */
#define L_FUNC_RESERVED_TEMP7                   0x40000000      /* -    */
#define L_FUNC_RESERVED_TEMP8                   0x80000000      /* -    */

/* These bits should be set for the above func flags that do not print */
#define L_FUNC_PRINTED          (L_FUNC_HYPERBLOCK |\
                                 L_FUNC_LEAF |\
                                 L_FUNC_SIDE_EFFECT_FREE|\
                                 L_FUNC_REGISTER_ALLOCATED|\
                                 L_FUNC_SUPERBLOCK|\
                                 L_FUNC_MASK_PE|\
                                 L_FUNC_COMPILATION_COMPLETE|\
                                 L_FUNC_SCHEDULED|\
                                 L_FUNC_CC_IN_PREDICATE_REGS|\
                                 L_FUNC_PRED_REGS_IN_ATTR |\
                                 L_FUNC_ROT_REG_ALLOCATED)
#define L_FUNC_HIDDEN_FLAGS     ~(L_FUNC_PRINTED)

/* Func flag explanation, printed character in Lcode is in () before
 *      flag name.  A "(-)" indicates the flag is not printed.
 *
 *   (H) L_FUNC_HYPERBLOCK - function contains predicated instrs.
 *   (L) L_FUNC_LEAF - functions which do not have any subroutine calls.
 *   (E) L_FUNC_SIDE_EFFECT_FREE - functions that contain either no store 
 *              instructions or only stack based stores which will be local to
 *              the function. 
 *   (R) L_FUNC_REGISTER_ALLOCATED - function has been reg allocated.  This
 *              refers to static register allocation.
 *   (S) L_FUNC_SUPERBLOCK - function contains 1 or more superblocks
 *   (M) L_FUNC_MASK_PE - function has instructions who's exceptions should
 *              be masked, tells the code generator to reserve a register
 *              for this purpose.
 *   (C) L_FUNC_COMPILATION_COMPLETE - compilation of function is complete
 *   (D) L_FUNC_SCHEDULED - function has been scheduled
 *   (P) L_FUNC_CC_IN_PREDICATE_REGS - branch compare conditions or condition
 *              codes are kept in predicate registers ala Playdoh.  So what
 *              this essentially means is BB's and SB's can have predicate
 *              defines and uses in them and not be classified as hyperblocks.
 *   (A) L_FUNC_PRED_REGS_IN_ATTR - virtual predicate registers are saved
 *              in the attribute field of opers, so predicate id's used
 *              for analysis are the attribute predicates not the operand.
 *   (T) L_FUNC_ROT_REG_ALLOCATED - function has been rotating reg allocated.
 *   (-) L_FUNC_RESERVED_TEMP1-8 - These are reserved as temporary func flags
 *              to be used by Lcode modules to define as they like.
 *              NONE of these are to be printed out!!
 */



/*
 * MDES Operation flags - these flags must be the same for all forms
 * of the operation with a given opcode described in the MDES file.
 */
#define OP_FLAG_CBR     0x00000001
#define OP_FLAG_JMP     0x00000002
#define OP_FLAG_RTS     0x00000004
#define OP_FLAG_JSR     0x00000008
#define OP_FLAG_SYNC    0x00000010
#define OP_FLAG_IGNORE  0x00000020
#define OP_FLAG_LOAD    0x00000040
#define OP_FLAG_STORE   0x00000080
#define OP_FLAG_EXCEPT  0x00000100
#define OP_FLAG_CHK     0x00000200
#define OP_FLAG_EXPANDS 0x00000400
#define OP_FLAG_NI      0x00000800      /* non-interlocking */
#define OP_FLAG_MEMCOPY 0x00001000
#define OP_FLAG_NOSPEC  0x00002000      /* oper cannot be speculated */

/*
 * MDES Alternative flags - these flags may differ for each variation of
 * of an operation regardless of the opcode described in the MDES file.
 */
#define ALT_FLAG_NT             0x00000001      /* nullify taken */
#define ALT_FLAG_NN             0x00000002      /* nullify not taken */
#define ALT_FLAG_SILENT         0x00000004      /* silent instruction */

#ifdef __cplusplus
extern "C"
{
#endif

  extern int L_cb_flags_string_to_int (char *flags_string);
  extern int L_cb_flags_to_string (char *flag_string, int cb_flags);
  extern int L_func_flags_string_to_int (char *flags_string);
  extern int L_func_flags_to_string (char *flag_string, int func_flags);
  extern int L_oper_flags_string_to_int (char *flags_string);
  extern int L_oper_flags_to_string (char *flag_string, int oper_flags);

#ifdef __cplusplus
}
#endif


#endif
