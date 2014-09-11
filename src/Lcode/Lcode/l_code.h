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
 *      File :          l_code.h
 *      Description :   Lcode data structure.
 *      Original : Pohua Chang, Wen-mei Hwu June 1990
 *      Modified : Roland G. Ouellette July 1990
 *              added L_n_ reg and oper...
 *      Modified : Roger A. Bringmann April, 1991
 *              added extensions to support mcode.
 *      Modified : Scott A. Mahlke February 1992
 *              added support for predicated execution, format changed
 *      Revised : Scott A. Mahlke, Roger A. Bringmann, January 1993
 *              dynamic allocation of all data structures, unlimited 
 *              src/dest/pred operands, reduce space requirements, 
 *              interface to MDES.
 *      Modified : Scott A. Mahlke September 1993
 *              changed predicated execution model Lcode uses
 *      Modified : Scott A. Mahlke March 1994
 *              added oper hash table junk
 *      Modified : Richard E. Hank April 1995
 *              added  Region support
 *      Modified : Ronald D. Barnes October 2000
 *              added L_program support for Lcode modules to operate
 *              on multiple functions at once. 
\*****************************************************************************/
/* 9/20/02 REK Adding completers field to the L_Oper structure. */
#ifndef L_CODE_H
#define L_CODE_H

/* 10/29/02 REK Adding config.h */
#include <config.h>
#include <Lcode/l_extern.h>
#include <Lcode/l_opc.h>
#include <Lcode/l_loop.h>
#include <Lcode/l_symbol.h>
#include <Lcode/l_debug.h>
#include <Lcode/l_error.h>
#include <alloca.h>

/*==========================================================================*/
/*
 *      Top level input tokens
 */
/*==========================================================================*/

#define L_INPUT_EOF             0       /* EOF */
#define L_INPUT_MS              1       /* (ms) */
#define L_INPUT_VOID            2       /* (void) */
#define L_INPUT_BYTE            3       /* (byte) */
#define L_INPUT_WORD            4       /* (word) */
#define L_INPUT_LONG            5       /* (long) */
#define L_INPUT_LONGLONG        6       /* (longlong) */
#define L_INPUT_FLOAT           7       /* (float) */
#define L_INPUT_DOUBLE          8       /* (double) */
#define L_INPUT_ALIGN           9       /* (align) */
#define L_INPUT_ASCII           10      /* (ascii) */
#define L_INPUT_ASCIZ           11      /* (asciz) */
#define L_INPUT_RESERVE         12      /* (reserve) */
#define L_INPUT_GLOBAL          13      /* (global) */
#define L_INPUT_WB              14      /* (wb) */
#define L_INPUT_WW              15      /* (ww) */
#define L_INPUT_WI              16      /* (wi) */
#define L_INPUT_WQ              17      /* (wq) */
#define L_INPUT_WF              18      /* (wf) */
#define L_INPUT_WF2             19      /* (wf2) */
#define L_INPUT_WS              20      /* (ws) */
#define L_INPUT_FUNCTION        21      /* (function) ... (end) */
#define L_INPUT_CB              22      /* internal use only */
#define L_INPUT_OP              23      /* internal use only */
#define L_INPUT_END             24      /* internal use only */
#define L_INPUT_POP             25      /* internal use only */
#define L_INPUT_ELEMENT_SIZE    26      /* (element_size) */
#define L_INPUT_EVENT_LIST      27      /* (event_list)   */
#define L_INPUT_RESULT_LIST     28      /* (result_list)  */
#define L_INPUT_REGION          29      /* internal use only */
#define L_INPUT_APPENDIX        30      /* internal use only */
#define L_INPUT_AOP             31      /* internal use only */
#define L_INPUT_ACB             32      /* internal use only */
#define L_INPUT_REGION_ENTRY    33      /* internal use only */
#define L_INPUT_REGION_EXIT     34      /* internal use only */
#define L_INPUT_REGION_LIVE_IN  35      /* internal use only */
#define L_INPUT_REGION_LIVE_OUT 36      /* internal use only */
#define L_INPUT_REGION_REGALLOC 37      /* internal use only */
#define L_INPUT_REGION_END      38      /* internal use only */
/*
 * LCW - New keywords for debugging information -- 9/14/95
 */
#define L_INPUT_DEF_STRUCT      39      /* (def_struct) */
#define L_INPUT_DEF_UNION       40      /* (def_union) */
#define L_INPUT_DEF_ENUM        41      /* (def_enum) */
#define L_INPUT_FIELD           42      /* (field) */
#define L_INPUT_ENUMERATOR      43      /* (enumerator) */

#define L_INPUT_SKIP            44      /* (skip) */

/* 
 * Input file models
 */
#define L_FILE_MODEL_FILE       1
#define L_FILE_MODEL_LIST       2
#define L_FILE_MODEL_EXTENSION  3

/*==========================================================================*/
/*
 *      Symbol table definitions
 */
/*==========================================================================*/

#define L_MAX_SYMBOL            ((1024 * 4) - 1)

#define L_MACRO_SYMBOL          1       /* symbol types */
#define L_MS_SYMBOL             2
#define L_CTYPE_SYMBOL          3
#define L_PTYPE_SYMBOL          4
#define L_OPERAND_SYMBOL        5
#define L_EXPR_SYMBOL           6
#define L_LCODE_SYMBOL          7
#define L_OPCODE_SYMBOL         8

/*==========================================================================*/
/*
 *      Predefined Macro Variables
 */
/*==========================================================================*/

enum
{
  /*
   * These are the Lcode parameter/return value passing macroregisters
   * and by convention should be used only for this purpose.
   * USING THESE MACROREGISTERS FOR ANYTHING OTHER THAN THIS PURPOSE
   * MAY DEGRADE LATER COMPILATION PHASES!!!!
   */
  L_MAC_P0 = 0,
  L_MAC_P1,
  L_MAC_P2,
  L_MAC_P3,
  L_MAC_P4,
  L_MAC_P5,
  L_MAC_P6,
  L_MAC_P7,
  L_MAC_P8,
  L_MAC_P9,
  L_MAC_P10,
  L_MAC_P11,
  L_MAC_P12,
  L_MAC_P13,
  L_MAC_P14,
  L_MAC_P15,
  L_MAC_P16,
  L_MAC_P17,
  L_MAC_P18,
  L_MAC_P19,
  L_MAC_P20,
  L_MAC_P21,
  L_MAC_P22,
  L_MAC_P23,
  L_MAC_P24,
  L_MAC_P25,
  L_MAC_P26,
  L_MAC_P27,
  L_MAC_P28,
  L_MAC_P29,
  L_MAC_P30,
  L_MAC_P31,
  L_MAC_P32,
  L_MAC_P33,
  L_MAC_P34,
  L_MAC_P35,
  L_MAC_P36,
  L_MAC_P37,
  L_MAC_P38,
  L_MAC_P39,
  L_MAC_P40,
  L_MAC_P41,
  L_MAC_P42,
  L_MAC_P43,
  L_MAC_P44,
  L_MAC_P45,
  L_MAC_P46,
  L_MAC_P47,
  L_MAC_P48,
  L_MAC_P49,
  L_MAC_P50,
  L_MAC_P51,
  L_MAC_P52,
  L_MAC_P53,
  L_MAC_P54,
  L_MAC_P55,
  L_MAC_P56,
  L_MAC_P57,
  L_MAC_P58,
  L_MAC_P59,
  L_MAC_P60,
  L_MAC_P61,
  L_MAC_P62,
  L_MAC_P63,
  L_MAC_P64,

  /*
   * The following four macros should be used by the mspec to provide
   * a generic way to represent access to these four distinct spaces
   * on the stack frame.  Conversion to the more machine specific
   * macros of $SP and $FP should happen during code generation.
   * IF THIS CONVENTION IS NOT FOLLOWED, CERTAIN LCODE LEVEL
   * OPTIMIZATIONS WILL BE UNAVAILABLE TO THAT PARTICULAR 
   * COMPILATION PATH!!!!
   */
  L_MAC_IP,                     /* pointer to incoming parameter space */
  L_MAC_OP,                     /* pointer to outgoing parameter space */
  L_MAC_LV,                     /* pointer to local variable space     */
  L_MAC_RS,                     /* pointer to register swap space      */

  L_MAC_SP,
  L_MAC_FP,

  L_MAC_CR,                     /* condition register */

  L_MAC_LOCAL_SIZE,
  L_MAC_PARAM_SIZE,
  L_MAC_SWAP_SIZE,
  L_MAC_RET_TYPE,

  L_MAC_TR_PTR,
  L_MAC_TR_MARK,
  L_MAC_TR_TEMP,
  L_MAC_PRED_ALL,

  L_MAC_SAFE_MEM,
  L_MAC_TM_TYPE,

  L_MAC_RETADDR,

  /* Insert any new macros before L_MAC_LAST */
  L_MAC_LAST
};

/* 
 * WARNING - If L_MAC_LAST ever reaches this value, it will break the
 * code generators!!
 */
#define L_CODEGEN_START_VALUE   200

/* 
 * WARNING - If the values used by the code generators ever reach this
 * value, it will break everything that is passed through memory!
 */
#define L_TM_START_VALUE        300

#define L_MACRO_P0              "$P0"
#define L_MACRO_P1              "$P1"
#define L_MACRO_P2              "$P2"
#define L_MACRO_P3              "$P3"
#define L_MACRO_P4              "$P4"
#define L_MACRO_P5              "$P5"
#define L_MACRO_P6              "$P6"
#define L_MACRO_P7              "$P7"
#define L_MACRO_P8              "$P8"
#define L_MACRO_P9              "$P9"
#define L_MACRO_P10             "$P10"
#define L_MACRO_P11             "$P11"
#define L_MACRO_P12             "$P12"
#define L_MACRO_P13             "$P13"
#define L_MACRO_P14             "$P14"
#define L_MACRO_P15             "$P15"
#define L_MACRO_P16             "$P16"
#define L_MACRO_P17             "$P17"
#define L_MACRO_P18             "$P18"
#define L_MACRO_P19             "$P19"
#define L_MACRO_P20             "$P20"
#define L_MACRO_P21             "$P21"
#define L_MACRO_P22             "$P22"
#define L_MACRO_P23             "$P23"
#define L_MACRO_P24             "$P24"
#define L_MACRO_P25             "$P25"
#define L_MACRO_P26             "$P26"
#define L_MACRO_P27             "$P27"
#define L_MACRO_P28             "$P28"
#define L_MACRO_P29             "$P29"
#define L_MACRO_P30             "$P30"
#define L_MACRO_P31             "$P31"
#define L_MACRO_P32             "$P32"
#define L_MACRO_P33             "$P33"
#define L_MACRO_P34             "$P34"
#define L_MACRO_P35             "$P35"
#define L_MACRO_P36             "$P36"
#define L_MACRO_P37             "$P37"
#define L_MACRO_P38             "$P38"
#define L_MACRO_P39             "$P39"
#define L_MACRO_P40             "$P40"
#define L_MACRO_P41             "$P41"
#define L_MACRO_P42             "$P42"
#define L_MACRO_P43             "$P43"
#define L_MACRO_P44             "$P44"
#define L_MACRO_P45             "$P45"
#define L_MACRO_P46             "$P46"
#define L_MACRO_P47             "$P47"
#define L_MACRO_P48             "$P48"
#define L_MACRO_P49             "$P49"
#define L_MACRO_P50             "$P50"
#define L_MACRO_P51             "$P51"
#define L_MACRO_P52             "$P52"
#define L_MACRO_P53             "$P53"
#define L_MACRO_P54             "$P54"
#define L_MACRO_P55             "$P55"
#define L_MACRO_P56             "$P56"
#define L_MACRO_P57             "$P57"
#define L_MACRO_P58             "$P58"
#define L_MACRO_P59             "$P59"
#define L_MACRO_P60             "$P60"
#define L_MACRO_P61             "$P61"
#define L_MACRO_P62             "$P62"
#define L_MACRO_P63             "$P63"
#define L_MACRO_P64             "$P64"

#define L_MACRO_IP              "$IP"
#define L_MACRO_OP              "$OP"
#define L_MACRO_LV              "$LV"
#define L_MACRO_RS              "$RS"

#define L_MACRO_SP              "$SP"
#define L_MACRO_FP              "$FP"

#define L_MACRO_CR              "$CR"

#define L_MACRO_LOCAL_SIZE      "$local"
#define L_MACRO_PARAM_SIZE      "$param"
#define L_MACRO_SWAP_SIZE       "$swap"
#define L_MACRO_RET_TYPE        "$return_type"

#define L_MACRO_TR_PTR          "$tr_ptr"
#define L_MACRO_TR_MARK         "$tr_mark"
#define L_MACRO_TR_TEMP         "$tr_temp"
#define L_MACRO_PRED_ALL        "$pred_all"

#define L_MACRO_SAFE_MEM        "$safe_mem"

#define L_MACRO_TM_TYPE         "$tm_type"

#define L_MACRO_RETADDR         "$retaddr"

/*==========================================================================*/
/*
 *      Predefined memory segments
 */
/*==========================================================================*/

#define L_MS_TEXT               0
#define L_MS_DATA               1
#define L_MS_DATA1              2
#define L_MS_DATA2              3
#define L_MS_SDATA              4
#define L_MS_SDATA1             5
#define L_MS_RODATA             6
#define L_MS_BSS                7
#define L_MS_SBSS               8
#define L_MS_SYNC               9
#define L_MS_LAST_LCODE_SECT    9

/* last section defined to allow safe machine-specific extension */

/*==========================================================================*/
/*
 *      Lcode function - data structures
 */
/*==========================================================================*/

/*
 *      L_Flow
 */

typedef struct L_Flow {
  int cc;                       /* branch condition
                                   cc = 1 for taken, 
                                   cc = 0 for fallthru */
  struct L_Cb *src_cb;          /* source basic block */
  struct L_Cb *dst_cb;          /* destination basic block */
  int flags;                    /* flow flags */
  double weight;                /* occurrences of the transfer */
  double weight2;               /* static weights */
  struct L_Flow *prev_flow;     /* maintained as doubly linked list */
  struct L_Flow *next_flow;

  // Added by Sepehr:
  int id; 						/*unique id for used in ILP */
  int wcet_weight; //execution time of src block from this flow
  int ILP_is_in_wcet_path; // is this flow in wcep?
  int wcep_num_exec; // number of execution in current wcep
  int max_iteration;
  double wcet_until_chock;
  int is_simple;
  int is_backedge;
} L_Flow;
/*
 *      L_Operand
 */

/* Allowable types */
#define L_OPERAND_VOID          0x00
#define L_OPERAND_CB            0x40
#define L_OPERAND_IMMED         0x41
#define L_OPERAND_STRING        0x42
#define L_OPERAND_LABEL         0x43
#define L_OPERAND_MACRO         0x20
#define L_OPERAND_REGISTER      0x21
#define L_OPERAND_RREGISTER     0x22
#define L_OPERAND_EVR           0x23

#define L_OPERAND_INT           0x80
#define L_OPERAND_FLOAT         0x81
#define L_OPERAND_DOUBLE        0x82

#define L_OPERAND_RESERVED      0x01
#define L_OPERAND_RESERVED2     0x02

/* Allowable ctypes (computation types) */
/* integer ctypes */
#define L_CTYPE_VOID            0X30
#define L_CTYPE_CHAR            0X41
#define L_CTYPE_UCHAR           0X01
#define L_CTYPE_SHORT           0X42
#define L_CTYPE_USHORT          0X02
#define L_CTYPE_INT             0X43
#define L_CTYPE_UINT            0X03
#define L_CTYPE_LONG            0X44
#define L_CTYPE_ULONG           0X04
#define L_CTYPE_LLONG           0X45
#define L_CTYPE_ULLONG          0X05
#define L_CTYPE_LLLONG          0X46
#define L_CTYPE_ULLLONG         0X06
#define L_CTYPE_POINTER         0X07
/* RMR { adding support for vector file type */
#define L_CTYPE_VECTOR_INT	0X47
/* } RMR */
/* float ctypes */
#define L_CTYPE_FLOAT           0X51
#define L_CTYPE_DOUBLE          0X52
/* RMR { adding support for vector file type */
#define L_CTYPE_VECTOR_FLOAT	0X53
#define L_CTYPE_VECTOR_DOUBLE	0x54
/* } RMR */
/* register ctypes */
#define L_CTYPE_PREDICATE       0X21
/* RMR { adding support for vector file type */
#define L_CTYPE_VECTOR_MASK	0X24
/* } RMR */
#define L_CTYPE_CONTROL         0X22
#define L_CTYPE_BTR             0X23
/* label ctypes */
#define L_CTYPE_LOCAL_ABS       0X31
#define L_CTYPE_LOCAL_GP        0X32
#define L_CTYPE_GLOBAL_ABS      0X33
#define L_CTYPE_GLOBAL_GP       0X34

/* Allowable ptypes (predicate types) */
#define L_PTYPE_NULL            0
#define L_PTYPE_UNCOND_T        1
#define L_PTYPE_UNCOND_F        2
#define L_PTYPE_COND_T          3
#define L_PTYPE_COND_F          4
#define L_PTYPE_OR_T            5
#define L_PTYPE_OR_F            6
#define L_PTYPE_AND_T           7
#define L_PTYPE_AND_F           8
#define L_PTYPE_SAND_T          9
#define L_PTYPE_SAND_F         10

/* Allowable branch prediction state */
#define L_PREDICT_NOTHING       -1
#define L_PREDICT_NOTTAKEN      0
#define L_PREDICT_TAKEN         1

/* JWS 20000705 - Add 64-bit integers */

typedef struct L_Operand {
  ITuint8 type;                 /* operand type                  */
  ITuint8 ctype;                /* data type                     */
  ITuint8 ptype;                /* predicate type                */
  union {
    struct L_Cb *cb;            /* control block (branch target) */
    ITintmax i;                 /* integer constant              */
    float f;                    /* float constant                */
    double f2;                  /* double constant               */
    char *s;                    /* string constant               */
    char *l;                    /* label constant                */
    ITint32 mac;                /* macro (special register)      */
    ITint32 r;                  /* register                      */
    ITint32 rr;                 /* rotating register             */
    struct {
      ITint32 reg;              /* predicate register number     */
      struct _PG_Pred_SSA *ssa; /* predicate SSA descriptor      */
    } pred;
    struct {                    /* evr (expanded virtual reg)    */
      ITint32 num;              /* evr number                    */
      ITint32 omega;            /* evr omega                     */
    } evr;
    struct {                    /* used for initialization       */
      ITint32 u;                /* upper 32-bits                 */
      ITint32 l;                /* lower 32-bits                 */
    } init;
  } value;
  struct _L_SSA *ssa;
} L_Operand;

/* Processor specific opcode string used in the attribute field */
#define L_PROC_OPC      "popc"

/*
 *      L_Attr
 */

typedef struct L_Attr {
  char *name;                   /* attribute name */
  int max_field;                /* number of fields allocated */
  L_Operand **field;            /* array of fields for an attribute */
  struct L_Attr *next_attr;     /* next attribute */
} L_Attr;


/*
 *      L_Sync
 */

#define L_SYNC_ALLOC_SIZE 20

typedef struct L_Sync {
  struct L_Oper *dep_oper;
  short info;
  char dist;
  char prof_info;
} L_Sync;


typedef struct L_Sync_Info {
  L_Sync **sync_out;            /* ptr to array of sync ptrs */
  int num_sync_out;             /* length of sync_out array */
  L_Sync **sync_in;             /* ptr to array of sync ptrs */
  int num_sync_in;              /* length of sync_in array */
} L_Sync_Info;

typedef struct _L_AccSpec {
  int is_def;
  int id;
  int version;
  int offset;
  int size;
  struct _L_AccSpec *next;
} L_AccSpec;


/* "extern" structures defined in other header files so can put pointers
 * in L_Oper.  This allows the contents of the structures from changing
 * without having to recompile all Lcode programs. -JCG 2/24/94
 */
struct Dep_Info;
struct Mdes_Info;

/*
 *      L_Oper
 */

#define L_MAX_CMPLTR 2

/* 20000718 JWS -- Completers:
 * Branch, pred def: [0] = CTYPE of comparison; [1] = Lcmp_COM_...
 * LD, ST            [0] = CTYPE of ld/st
 */
/* 09/20/02 REK Adding completers field to this structure. */

typedef struct L_Oper {
  int id;                       /* unique id of oper */
  unsigned int flags;           /* bit field flag */
  int opc;                      /* integer representation of opcode, 
                                 * only to be used for internally 
                                 * defined opcodes */
  ITuint8 com[L_MAX_CMPLTR];    /* Operation completers */
  char *opcode;                 /* name of the operation */
  int proc_opc;                 /* processor specific opcode */
  L_Operand **dest;             /* ptr to array of ptrs to dest opds */
  L_Operand **src;              /* ptr to array of ptrs to src opds  */
  L_Operand **pred;             /* ptr to array of ptrs to pred opds */
  L_Sync_Info *sync_info;       /* ptr to sync_in and sync_out info */
  struct _L_AccSpec *acc_info;
  L_Attr *attr;                 /* additional attributes */
  struct L_Oper *prev_op;       /* previous oper in sequential mode */
  struct L_Oper *next_op;       /* next oper in sequential mode */
  struct L_Oper *parent_op;     /* if Lcode -> links to first Mcode  */
  /* if Mcode -> links to parent Lcode */
  double weight;                /* operation execution frequency */
  struct Dep_Info *dep_info;    /* Dependence information */
  struct Mdes_Info *mdes_info;  /* LMDES information */
  void *ambig_info;             /* memory disambiguation information */
  int completers;               /* Used as a bitfield to indicate the presence
                                 * of completers. */
  void *ext;                    /* generic extension field */
} L_Oper;

/*
 *      L_Cb
 */

typedef struct L_Cb {
  int id;                       /* unique cb id */
  unsigned int flags;           /* bit field flag */
  L_Oper *first_op;             /* first operation */
  L_Oper *last_op;              /* last operation */
  L_Attr *attr;                 /* additional attributes */
  struct L_Flow *src_flow;      /* incoming arcs */
  struct L_Flow *dest_flow;     /* outgoing arcs */
  struct L_Cb *prev_cb;         /* previous sequential cb */
  struct L_Cb *next_cb;         /* next sequential cb */
  L_Loop *deepest_loop;         /* deepest loop nest containing cb */
  double weight;                /* basic block weight */
  double weight2;               /* static weight */
  struct L_Region *region;      /* region cb is contained within */
  Set dom;                      /* fast version of dominator 
                                   analysis */
  Set pdom;                     /* fast version of post-dominator 
                                   analysis */
  void *ext;                    /* generic extension field */
  int hash_tgt_cnt;             /* Number of refs in hash tables */
  // Added by Sepehr:
  int wcet;		/* Worst case execution time of block */
  int chock;
  struct L_Cb * chock_end_cb;
  struct L_Cb * chock_first_cb;
  Set chock_middle_nodes;
  int ILP_is_in_wcet_path;
  double wcet_loop;
  struct L_Cb * back_edge_node;
  int max_iteration;
  int wcep_num_exec;
  double wcet_until_chock;
  int is_balance;
  int has_profit;
  int is_closed;
  int is_simple;
  Set loops;

} L_Cb;


/* SER - structure for expressions for use in PRE */
typedef struct L_Expression {
  int index;  /* index into set */
  int token;  /* semi-unique token from generator for oper matching */
  L_Operand **src;
  L_Operand *mem_ref[2];	  /* For stack and label refs. */
  L_Operand **dest;		  /* For PDE. */
  ITuint8 dest_ctype;
  ITuint8 com[L_MAX_CMPLTR];
  int opc;
  int reg_id;
  int pred_id;
  double weight;
  Set associates;                 /* Matching exprs/assignments */
  Set conflicts;		  /* Conflicting memory expressions. */
  struct _L_AccSpec * acc_info;
  struct L_Expression * general;  /* For general stores within PDE. */
} L_Expression;


/*
 *      Oper hash table entry
 */

typedef struct L_Oper_Hash_Entry {
  int id;                       /* hash key (oper id) */
  L_Oper *oper;                 /* oper ptr */
  L_Cb *cb;                     /* cb which contains oper */
  struct L_Oper_Hash_Entry *prev_oper_hash;
  struct L_Oper_Hash_Entry *next_oper_hash;
} L_Oper_Hash_Entry;

/*
 *      Cb hash table entry
 */

typedef struct L_Cb_Hash_Entry {
  int id;                       /* hash key (cb id) */
  L_Cb *cb;                     /* cb ptr */
  struct L_Cb_Hash_Entry *prev_cb_hash;
  struct L_Cb_Hash_Entry *next_cb_hash;
} L_Cb_Hash_Entry;

/*
 *      Expression hash table entry
 */

typedef struct L_Expression_Hash_Entry {
  int id;                       /* hash key (may be token or id) */
  L_Expression *expression;
  struct L_Expression_Hash_Entry *prev_expression_hash;
  struct L_Expression_Hash_Entry *next_expression_hash;
} L_Expression_Hash_Entry;

/*
 *      L_Func
 */

typedef struct L_Func {
  char *name;                   /* name of function */
  unsigned int flags;           /* bit field flag */
  double weight;                /* weight of function */
  L_Cb *first_cb;               /* first sequential cb */
  L_Cb *last_cb;                /* last sequential cb */
  struct L_Func *prev_func;     /* previous sequential 
                                   func */
  struct L_Func *next_func;     /* next sequential func */
  struct L_Region *first_region;        /* first function region */
  struct L_Region *last_region; /* last function region */
  int n_cb;                     /* number of control 
                                   blocks */
  int n_oper;                   /* number of operations */
  int n_parent_oper;            /* number of parent 
                                   operations */
  int n_expression;             /* number of expressions */
  int max_cb_id;                /* largest cb_id */
  int max_oper_id;              /* largest oper_id */
  int max_reg_id;               /* largest virtual reg 
                                   number */
  int max_spec_id;              /* largest speculation id */
  int s_local;                  /* $local */
  int s_param;                  /* $param */
  int s_swap;                   /* $swap */
  L_Attr *attr;                 /* additional attributes */
  L_Oper *last_parent_op;       /* first parent op in func */
  L_Oper_Hash_Entry **oper_hash_tbl;    /* hash table, 
                                           map op/cb to op_id */
  L_Cb_Hash_Entry **cb_hash_tbl;        /* hash table, 
                                           map cb to cb_id */
  L_Expression_Hash_Entry **expression_token_hash_tbl; /* has table,
						    map expression to
						    expression token */
  L_Expression_Hash_Entry **expression_index_hash_tbl; /* hash table,
							 map expression to
							 expression index */
  struct L_Region_Hash_Entry **region_hash_tbl; /* hash table, 
                                                   map region to region_id */
  L_Loop *first_loop;           /* first loop of funct */
  int max_loop_id;              /* max loop id */
  L_Inner_Loop *first_inner_loop;       /* first inner loop of 
                                           function */
  int max_inner_loop_id;        /* max inner loop id */
  struct L_Datalist *jump_tbls; /* jump tbls for this func */
  unsigned int jump_tbl_flags;  /* jump tbl status flags */
  void *ext;
} L_Func;

/*
 *      L_Program
 */

typedef struct L_Program {
  char *name;                   /* name of program */
  L_Func *first_func;           /* first sequential func */
  L_Func *last_func;            /* last sequential func */
  int n_func;                   /* number of functions */
} L_Program;

/*
 *      L_Oper_List
 */

typedef struct L_Oper_List {
  L_Oper *oper;
  struct L_Oper_List *next_list;
} L_Oper_List;


/*==========================================================================*/
/*
 *      Lcode data - data structures
 */
/*==========================================================================*/

/* 
 *      L_Expr
 */

#define L_EXPR_INT              0       /* integer constant */
#define L_EXPR_FLOAT            1       /* float */
#define L_EXPR_DOUBLE           2       /* double */
#define L_EXPR_LABEL            3       /* a label */
#define L_EXPR_STRING           4       /* a string constant */
#define L_EXPR_ADD              5       /* A + B (int oper) */
#define L_EXPR_SUB              6       /* A - B (int oper) */
#define L_EXPR_MUL              7       /* A * B (int oper) */
#define L_EXPR_DIV              8       /* A / B (int oper) */
#define L_EXPR_NEG              9       /* - A (int oper) */
#define L_EXPR_COM              10      /* ~ A (int oper) */

/* BCC - micro31 address prediction flags */
#define L_SETUP_IMPLIED_REG             "setup_implied_reg"
#define L_DONT_PREDICT                  "dont_predict"
#define L_PREDICT                       "predict"

typedef struct L_Expr {
  short type;
  union {
    ITintmax i;                 /* integer constant */
    float f;                    /* float constant */
    double f2;                  /* double constant */
    char *l;                    /* LABEL */
    char *s;                    /* STR.s */
  } value;
  struct L_Expr *A, *B;
  struct L_Expr *next_expr;
} L_Expr;


/*
 *      L_Data
 */

typedef struct L_Data {
  short type;
  int N;                        /* N.d, MS.d */
  int id;                       /* HCH 05/07/04: for getting OBJ id from Pcode */
  L_Expr *address;              /* LABEL, Address */
  L_Expr *value;                /* Expr*, STR.s, Value */
  struct L_Type *h_type;        /* LCW - Newly inserted field recording the
                                   type information of the data - 9/14/95 */
  void *ext;                    /* extension field */
} L_Data;

/*
 *      L_Datalist_Element
 */

typedef struct L_Datalist_Element {
  L_Data *data;
  struct L_Datalist_Element *next_element;
} L_Datalist_Element;


/*
 *      L_Datalist
 */

typedef struct L_Datalist {
  L_Datalist_Element *first_element;
  L_Datalist_Element *last_element;
} L_Datalist;

/*========================================================================*/
/*
 *      External variables
 */
/*========================================================================*/

#ifdef __cplusplus
extern "C"
{
#endif

  extern int L_token_type;
  extern int L_generation_info_printed;

  extern L_Func *L_fn;          /* current active function */
  extern L_Data *L_data;        /* current active data */
  extern L_Datalist *L_data_list;       /* Current active datalist */

  extern L_Oper **L_cnt_oper;
  extern L_Flow **L_cnt_flow;
  extern int L_n_color_entries_alloc;
  extern int L_n_cnt_oper;

  extern int L_func_read;

  extern char *L_file_arch;
  extern char *L_file_model;
  extern char *L_file_lmdes;

  extern STRING_Symbol_Table *L_string_table;
  extern STRING_Symbol_Table *L_opcode_symbol_table;
  extern STRING_Symbol_Table *L_cmp_compl_symbol_table;
  extern STRING_Symbol_Table *L_macro_symbol_table;
  extern INT_Symbol_Table *L_macro_id_symbol_table;     /* ITI/JWJ 7/99 */
/*========================================================================*/
/*
 *      External functions
 */
/*========================================================================*/

/*
 *      Symbol table related functions
 */

  extern void L_init_symbol (void);

  extern int L_macro_id (char *);
  extern int L_ms_id (char *);
  extern int L_ctype_id (char *);
  extern int L_ptype_id (char *);
  extern int L_operand_id (char *);
  extern int L_expr_id (char *);
  extern int L_lcode_id (char *);
  extern int L_opcode_id (char *);
  extern ITuint8 L_cmp_compl_id (char *);
  extern void L_cmp_compl_from_old_opc (L_Oper *);

  extern char *L_macro_name (int);
  extern char *L_ms_name (int);
  extern char *L_ctype_name (int);
  extern char *L_ptype_name (int);
  extern char *L_operand_name (int);
  extern char *L_expr_name (int);
  extern char *L_lcode_name (int);
  extern char *L_opcode_name (int);
  extern char *L_cmp_compl_name (ITuint8);

/*
 *      OPER hash table functions
 */

  extern L_Oper_Hash_Entry **L_oper_hash_tbl_create (void);
  extern void L_oper_hash_tbl_insert (L_Oper_Hash_Entry **, L_Oper *);
  extern void L_oper_hash_tbl_delete (L_Oper_Hash_Entry **, L_Oper *);
  extern void L_oper_hash_tbl_delete_specific (L_Oper_Hash_Entry **,
                                               L_Oper *);
  extern void L_oper_hash_tbl_delete_all (L_Oper_Hash_Entry **);
  extern void L_oper_hash_tbl_update_cb (L_Oper_Hash_Entry **, int, L_Cb *);
  extern L_Oper *L_oper_hash_tbl_find_oper (L_Oper_Hash_Entry **, int);
  extern L_Oper *L_oper_hash_tbl_find_and_alloc_oper (L_Oper_Hash_Entry **,
                                                      int);
  extern L_Cb *L_oper_hash_tbl_find_cb (L_Oper_Hash_Entry **, int);
  extern void L_oper_hash_tbl_check (L_Func *);
  extern void L_oper_hash_tbl_rebuild (L_Func *);

/*
 *      CB hash table functions
 */

  extern L_Cb_Hash_Entry **L_cb_hash_tbl_create (void);
  extern void L_cb_hash_tbl_insert (L_Cb_Hash_Entry **, L_Cb *);
  extern void L_cb_hash_tbl_delete (L_Cb_Hash_Entry **, L_Cb *);
  extern void L_cb_hash_tbl_delete_all (L_Cb_Hash_Entry **);
  extern L_Cb *L_cb_hash_tbl_find (L_Cb_Hash_Entry **, int);
  extern void L_cb_hash_tbl_check (L_Cb_Hash_Entry **);
  extern L_Cb *L_cb_hash_tbl_find_and_alloc (L_Cb_Hash_Entry **, int);

/*
 *      Expression hash table functions
 */

  extern void L_expression_hash_tbl_insert (L_Expression_Hash_Entry **,
					    L_Expression *, int);
  extern L_Expression_Hash_Entry
  *L_expression_hash_tbl_find_entry (L_Expression_Hash_Entry **, int);
  extern void L_expression_hash_tbl_delete_all (L_Expression_Hash_Entry **);
  extern Set L_get_jsr_conflicting_expressions (L_Oper *);
  extern L_Expression
  *L_find_oper_expression_in_hash (L_Expression_Hash_Entry **, 
				   int, L_Oper *, int);
  extern L_Expression
  *L_find_oper_assignment_in_hash (L_Expression_Hash_Entry **,
				   int, L_Oper *, int);
  extern int L_generate_expression_for_oper (L_Oper * oper, int);
  extern L_Expression * L_generate_assignment_for_oper (L_Oper * oper, int);
  extern L_Expression 
    *L_find_corresponding_store_expression_for_load  (L_Oper * oper);
  extern Set L_create_complement_load_expressions (L_Oper * oper);

/*
 *      Global variable initialization
 */
  extern void L_reset_data_global_vars (void);
  extern void L_reset_func_global_vars (void);

/*
 *      L_Expr
 */

  extern L_Expr *L_new_expr (int);
  extern void L_delete_expr_element (L_Expr *);
  extern L_Expr *L_delete_expr (L_Expr *, L_Expr *);
  extern L_Expr *L_concat_expr (L_Expr *, L_Expr *);
  extern L_Expr *L_new_expr_int (ITintmax value);
  extern L_Expr *L_new_expr_label (char *label);
  extern L_Expr *L_new_expr_label_no_underscore (char *label);
  extern L_Expr *L_new_expr_float (double value);
  extern L_Expr *L_new_expr_double (double value);
  extern L_Expr *L_new_expr_add (L_Expr * expr1, L_Expr * expr2);
  extern L_Expr *L_new_expr_sub (L_Expr * expr1, L_Expr * expr2);
  extern L_Expr *L_new_expr_mul (L_Expr * expr1, L_Expr * expr2);
  extern L_Expr *L_new_expr_div (L_Expr * expr1, L_Expr * expr2);
  extern L_Expr *L_new_expr_string (char *string);
  extern L_Expr *L_new_expr_addr (char *label, int offset);
  extern L_Expr *L_new_expr_addr_no_underscore (char *label, int offset);


/*
 *      LCW - functions for new data structures for debugging info - 4/17/96
 */

  extern L_Type *L_new_type ();
  extern void L_delete_type (L_Type *);
  extern L_Dcltr *L_new_dcltr ();
  extern void L_delete_dcltr (L_Dcltr *);
  extern L_Dcltr *L_concat_dcltr (L_Dcltr * d1, L_Dcltr * d2);
  extern L_Struct_Dcl *L_new_struct_dcl ();
  extern void L_delete_struct_dcl (L_Struct_Dcl *);
  extern L_Union_Dcl *L_new_union_dcl ();
  extern void L_delete_union_dcl (L_Union_Dcl *);
  extern L_Field *L_new_field ();
  extern void L_delete_field (L_Field *);
  extern L_Enum_Dcl *L_new_enum_dcl ();
  extern void L_delete_enum_dcl (L_Enum_Dcl *);
  extern L_Enum_Field *L_new_enum_field ();
  extern void L_delete_enum_field (L_Enum_Field *);


/*
 *      L_Data
 */

  extern L_Data *L_new_data (int);
  extern L_Data *L_new_data_w (int type, L_Expr * address, L_Expr * value);
  extern void L_delete_data (L_Data *);

/*
 *      L_Datalist_Element
 */
  extern L_Datalist_Element *L_new_datalist_element (L_Data *);
  extern void L_delete_datalist_element (L_Datalist *, L_Datalist_Element *);
  extern L_Datalist_Element *L_find_datalist_element (L_Datalist *, L_Data *);
  extern void L_concat_datalist_element (L_Datalist *, L_Datalist_Element *);
  extern void L_delete_all_datalist_element (L_Datalist *);

/*
 *      L_Datalist
 */

  extern L_Datalist *L_new_datalist (void);
  extern void L_delete_datalist (L_Datalist *);

/*
 *      L_Flow
 */

  extern L_Flow *L_new_flow (int, L_Cb *, L_Cb *, double);
  extern L_Flow *L_remove_flow (L_Flow *, L_Flow *);
  extern L_Flow *L_delete_flow (L_Flow *, L_Flow *);
  extern void L_delete_all_flow (L_Flow *);
  extern L_Flow *L_concat_flow (L_Flow *, L_Flow *);
  extern L_Flow *L_insert_flow_before (L_Flow *, L_Flow *, L_Flow *);
  extern L_Flow *L_insert_flow_after (L_Flow *, L_Flow *, L_Flow *);
  extern L_Flow *L_insert_flows_after (L_Flow * list, L_Flow * after_flow, 
				       L_Flow * flow);

/*
 *      L_Attr
 */

  extern void L_insert_attr_field (L_Attr * attr, L_Operand *new_op, int field);
  extern void L_set_attr_field (L_Attr *, int, L_Operand *);
  extern void L_set_cb_attr_field (L_Attr *, int, L_Cb *);
  extern void L_set_int_attr_field (L_Attr *, int, int);
  extern void L_set_float_attr_field (L_Attr *, int, float);
  extern void L_set_double_attr_field (L_Attr *, int, double);
  extern void L_set_string_attr_field (L_Attr *, int, char *);
  extern void L_set_macro_attr_field (L_Attr *, int, int, int, int);
  extern void L_set_register_attr_field (L_Attr *, int, int, int, int);
  extern void L_set_label_attr_field (L_Attr *, int, char *);
  extern void L_set_rregister_attr_field (L_Attr *, int, int, int, int);
  extern void L_set_evr_attr_field (L_Attr *, int, int, int, int, int);
  extern void L_delete_attr_field (L_Attr *, int);
  extern L_Attr *L_new_attr (char *, int);
  extern L_Attr *L_delete_attr (L_Attr *, L_Attr *);
  extern void L_delete_all_attr (L_Attr *);
  extern L_Attr *L_concat_attr (L_Attr *, L_Attr *);
  extern L_Attr *L_find_attr (L_Attr *, char *);
  extern int L_count_attr_prefix (L_Attr * attr, char *attr_name);
  extern L_Attr *L_find_attr_prefix (L_Attr * attr, char *attr_name, int n);
  extern void L_free_attr (L_Attr * attr);

/*
 *      L_Sync_Info
 */

  extern L_Sync_Info *L_new_sync_info (void);
  extern L_Sync_Info *L_copy_sync_info (L_Sync_Info * sync_info);
/*
 *      L_Sync
 */

  extern L_Sync *L_new_sync (L_Oper * oper);
  extern void L_delete_head_sync (L_Oper * oper, L_Sync * sync);
  extern void L_delete_tail_sync (L_Oper * oper, L_Sync * sync);
  extern void L_find_and_delete_head_sync (L_Oper * oper, L_Oper * dep_oper);
  extern void L_find_and_delete_tail_sync (L_Oper * oper, L_Oper * dep_oper);
  extern void L_delete_all_sync (L_Oper * oper, int delete_other_end);
  extern void L_insert_head_sync_in_oper (L_Oper * oper, L_Sync * sync);
  extern void L_insert_tail_sync_in_oper (L_Oper * oper, L_Sync * sync);
  extern L_Sync *L_remove_head_sync_from_oper (L_Oper * oper, L_Sync * sync);
  extern L_Sync *L_remove_tail_sync_from_oper (L_Oper * oper, L_Sync * sync);
  extern L_Sync *L_find_head_sync (L_Oper * oper, L_Oper * dep_oper);
  extern L_Sync *L_find_tail_sync (L_Oper * oper, L_Oper * dep_oper);
  extern L_Sync *L_copy_sync (L_Sync * sync);
  extern L_Sync **L_new_sync_array (int array_size);
  extern void L_copy_sync_array (L_Sync ** new_array, L_Sync ** old_array,
                                 int array_size);

  extern L_Sync *L_remove_sync_from_oper (L_Oper * oper, L_Sync * sync);
/*
 *      L_Operand
 */

  extern L_Operand *L_new_cb_operand (L_Cb *);
/* JLB - this function creates a general integer operand 
   ie. type = L_OPERAND_IMMEDIATE
       ctype = L_CTYPE_INT */
  extern L_Operand *L_new_gen_int_operand (ITintmax);
/* JLB - this function should not be called unless a specific size
   integer is desired.  if a general, or size int immediate
   is wanted, please call the above function.  this function will 
   be used more heavily as type information becomes available */
  extern L_Operand *L_new_int_operand (ITintmax, char);

#ifdef IT64BIT
  extern L_Operand *L_new_gen_llong_operand (ITint64 value);
  extern L_Operand *L_new_llong_operand (ITint64, char);
#endif

  extern L_Operand *L_new_float_operand (float);
  extern L_Operand *L_new_double_operand (double);
  extern L_Operand *L_new_gen_string_operand (char *);
/* JLB - this function should not be called unless a specific 
   ctype is desired with the string.  in general, if a new string
   with L_CTYPE_GLOBAL_ABS is desired, call the general string
   function defined above. */
  extern L_Operand *L_new_string_operand (char *, char);
  extern L_Operand *L_new_macro_operand (int, int, int);
  extern L_Operand *L_new_register_operand (int, int, int);
  extern L_Operand *L_new_gen_label_operand (char *);
/* JLB - this function should not be called unless a specific 
   ctype is desired with the label.  in general, if a new label
   with L_CTYPE_LOCAL_ABS is desired, call the general label
   function defined above. */
  extern L_Operand *L_new_label_operand (char *, char);
  extern L_Operand *L_new_rregister_operand (int, int, int);
  extern L_Operand *L_new_evr_operand (int, int, int, int);
  extern void L_delete_operand (L_Operand *);

/*
 *      L_Oper
 */

  extern L_Oper *L_new_oper (int);
  extern L_Oper *L_new_parent_oper (int);
  extern void L_remove_oper (L_Cb *, L_Oper *);
  extern void L_delete_oper (L_Cb *, L_Oper *);
  extern void L_delete_complete_oper (L_Cb *, L_Oper *);
  extern void L_delete_all_oper (L_Oper *, int delete_cross_sync);
  extern void L_insert_oper_before (L_Cb *, L_Oper *, L_Oper *);
  extern void L_insert_oper_after (L_Cb *, L_Oper *, L_Oper *);
  extern void L_insert_opers_before (L_Cb *, L_Oper *, L_Oper *);
  extern void L_insert_opers_after (L_Cb *, L_Oper *, L_Oper *);

  extern void L_convert_to_com (L_Oper *);

/*
 *      L_Cb
 */

  extern L_Cb *L_new_cb (int);
  extern void L_remove_cb (L_Func *, L_Cb *);
  extern void L_delete_cb (L_Func *, L_Cb *);
  extern void L_delete_all_cb (L_Cb *, L_Cb_Hash_Entry **);
  extern void L_insert_cb_before (L_Func *, L_Cb *, L_Cb *);
  extern void L_insert_cb_after (L_Func *, L_Cb *, L_Cb *);

/*
 *      L_Expression
 */

  extern L_Expression *L_new_expression (int, L_Oper *, int);
  extern L_Expression *L_new_assignment (int, L_Oper *, int);
  extern int L_oper_matches_expression (L_Oper *, int, L_Expression *,
					int, int);
  extern int L_oper_matches_assignment (L_Oper *, int, L_Expression *, int);
  extern int L_load_oper_matches_store_expression (L_Oper *, int, L_Expression *);
  extern int L_opers_same_expression (L_Oper *, L_Oper *);
  extern int L_opers_same_assignment (L_Oper *, L_Oper *, int);
  extern int L_opers_same_or_complementary_expression (L_Oper *, L_Oper *);
  extern void L_delete_expression (L_Expression *);
  extern void L_delete_all_expressions (L_Func *);

/*
 *      L_Oper_Hash_Entry
 */

  extern L_Oper_Hash_Entry *L_new_oper_hash_entry (int, L_Oper *);
  extern L_Oper_Hash_Entry *L_delete_oper_hash_entry (L_Oper_Hash_Entry *,
                                                      L_Oper_Hash_Entry *);
  extern void L_delete_all_oper_hash_entry (L_Oper_Hash_Entry *);
  extern L_Oper_Hash_Entry *L_find_oper_hash_entry (L_Oper_Hash_Entry *, int);
  extern L_Oper_Hash_Entry *L_find_specific_oper_hash_entry (L_Oper_Hash_Entry
                                                             *, L_Oper *);

/*
 *      L_Cb_Hash_Entry
 */

  extern L_Cb_Hash_Entry *L_new_cb_hash_entry (int, L_Cb *);
  extern L_Cb_Hash_Entry *L_delete_cb_hash_entry (L_Cb_Hash_Entry *,
                                                  L_Cb_Hash_Entry *);
  extern void L_delete_all_cb_hash_entry (L_Cb_Hash_Entry *);
  extern L_Cb_Hash_Entry *L_find_cb_hash_entry (L_Cb_Hash_Entry *, int);

/*
 *      L_Expression_Hash_Entry
 */

  extern L_Expression_Hash_Entry *L_new_expression_hash_entry (int, L_Expression *);
  extern void L_delete_all_expression_hash_entry (L_Expression_Hash_Entry *);

/*
 *      L_Func
 */

  extern L_Func *L_new_func (char *, double);
  extern void L_delete_func (L_Func *);

/*
 *      L_Oper_List
 */

  extern L_Oper_List *L_new_oper_list (void);
  extern L_Oper_List *L_delete_oper_list (L_Oper_List *, L_Oper_List *);
  extern void L_delete_all_oper_list (L_Oper_List *);
  extern L_Oper_List *L_concat_oper_list (L_Oper_List *, L_Oper_List *);
  extern L_Oper_List *L_find_oper_list (L_Oper_List *, L_Oper *);

/*
 * LCW - L_debug : modules for handling local variable information
 * for the purpose of debugging support - 4/21/97
 */
  extern L_Local_Var *new_local_var (char *);
  extern void delete_local_var (L_Local_Var *);
  extern L_Attr *extract_local_var_attr (L_Attr *);
  extern L_Attr *L_append_local_var_attr (L_Attr *);
  extern void update_local_var_spill_loc (int);

/*
 *      L_Program
 */

  extern L_Program *L_new_program (char *);
  extern void L_delete_program (L_Program *);

/*
 *      For Speculative Partial Redundancy Elimination
 */

#ifdef __cplusplus
}
#endif

#endif

