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
 * \brief Structure definitions for the Pcode library.
 *
 * \author Nancy Warter, Wen-mei Hwu
 *
 * Extends ccode.h written by Po-hua Chang
 *
 * Copyright (c) 1991 Nancy Warter, Po-hua Chang, Wen-mei Hwu,
 * and the Board of Trustees of the University of Illinois.
 * All rights reserved.
 *
 * Pcode is a statement level representation (whereas hcode is a basic block
 * representation).  Pcode is organized as a parse tree structure with three
 * levels of structures: function, statement, and expression.  We assume that
 * functions are processed independently to save memory space.
 *
 * Definition of terms:
 *  Type - A representation of a C type.  This can consist of several
 *         TypeDcls linked together through the symbol table.  The Type
 *         is represented as a symbol table key.
 *  TypeDcl - The TypeDcl structure.
 */
/*****************************************************************************/

#ifndef _PCODE_PCODE_H_
#define _PCODE_PCODE_H_

#include <config.h>
#include <library/i_types.h>
#include <library/i_list.h>
#include <library/set.h>
#include <library/block_sparse_array.h>
#include "perror.h"
#include "parms.h"

/*
 * It may be necessary to modify the following constants to
 * compile larger programs.
 */
#define MAX_OPCODE_TBL_SIZE	201

/*! The maximum length of a Pcode identifier (+1 for the null). */
#define P_MAX_IDENTIFIER_LEN    1024

/*! For enums, value of fields starts at 0, unless overriden */
#define BASE_ENUM_VALUE         0

/*! Options for the BlockSparseArray structures in the symbol table. */
#define BSA_OPTIONS BSA_WARN_ON_OVERWRITE

#define USE_L_ALLOC

#define P_INPUT_EOF	0
#define P_INPUT_STRUCT	1
#define P_INPUT_UNION	2
#define P_INPUT_ENUM	3
#define P_INPUT_GVAR	4
#define P_INPUT_FUNC	5
#define P_INPUT_INCLUDE	6
#define P_INPUT_NULL	7

/*! The maximum length of a data signature for an extension field. */
#define EXT_SIG_LENGTH 4

/*===========================================================================*/
/* Enum definitions */

/*! Declaration structure types.  Used to determine the valid field in the
 * ::Dcl.ptr union. */
typedef enum _DclType
{
  TT_VAR,                       /*!< global variable tag */
  TT_STRUCT,                    /*!< struct tag */
  TT_UNION,                     /*!< union tag */
  TT_ENUM,                      /*!< enum tag */
#if 0
  TT_ENUMFIELD,                 /*!< enum field tag */
#endif
  TT_FUNC,                      /*!< func tag */ /* TLJ 2/27/96 */
  TT_TYPE,                      /*!< typedef tag */
  TT_ASM,                       /*!< asm tag */
  TT_INCLUDE,                   /*!< include file name */
  TT_SYMBOLTABLE,               /*!< symbol table tag */
  TT_IPSYMTABENT,               /*!< interprocedural symbol table entry tag */
  TT_SYMTABENTRY                /*!< symbol table entry tag */
}
_DclType;

/* Type Specifier. (This is a subset of those used in Csemantic)
 *
 * A type specifier consists of two pieces of information:
 * \li type: data type and special type properties of the data holder
 * \li dcltr: the access pattern of the data holder
 *
 * The type attribute can be further broken down to three
 * subfields: (qualifier, class, data_type). To efficiently
 * represent these fields, bit vectors can be used. The meaning
 * of each bit is defined as below:
 *
 * \note This enum must correspond to the L_DATA_ typedefs in
 * Lcode/l_debug.h */
/* BCC - 7/10/96
 * enumerate these constants. xups can display them in symbolic names 
 */
/* CWL - 12/31/01 for P-to-L.
 * - add TY_GLOBAL,TY_PARAMETER, TY_BIT_FIELD.
 * - change decimal numbers to hexadecimal numbers.
 * - rearrange the definition to match the definition in "l_debug.h".
 * (you can know the original arrangement by checking the decimal numbers)
 */
/* REK - 9/8/03 Splitting _TypeClassQual in to _BasicType and _TypeQual enums.
 */
/*! Basic Type specifier.
 *
 * These specify the fundamental datatypes that C knows about.
 *
 * basic_type.txt should be inserted here when the Pcode cleanup is done.
 */
typedef enum _BasicType
{
  BT_VOID 	  = 0x00000001, /*!< void type */

  BT_CHAR 	  = 0x00000002, /*!< char type */
  BT_SHORT 	  = 0x00000004, /*!< short type */
  BT_INT 	  = 0x00000008, /*!< int type */
  BT_LONG 	  = 0x00000010, /*!< long type */
  BT_LONGLONG 	  = 0x00000020, /*!< long long type */

  BT_FLOAT 	  = 0x00000040, /*!< float type */
  BT_DOUBLE 	  = 0x00000080, /*!< double type */
  BT_LONGDOUBLE   = 0x00000100, /*!< long double type */

  BT_UNSIGNED     = 0x00000400, /*!< unsigned type */

  BT_STRUCT 	  = 0x00000800, /*!< struct type */
  BT_UNION 	  = 0x00001000, /*!< union type */
  BT_ENUM 	  = 0x00002000, /*!< enum type */

  BT_VARARG 	  = 0x00004000, /*!< vararg type */
  BT_BIT_FIELD 	  = 0x00008000, /*!< struct/union bit field type */

  BT_TYPEDEF_E 	  = 0x00010000, /*!< explicit typedef (eg, typedef foo int;) */
  BT_TYPEDEF_I    = 0x00020000, /*!< implicit typedef (eg, int foo[10];) */

  BT_ARRAY        = 0x00040000, /*!< array type */
  BT_FUNC         = 0x00080000, /*!< function type */
  BT_POINTER      = 0x00100000, /*!< pointer type */
}
_BasicType;

/*! Any typedef (explicit or implicit) */
#define BT_TYPEDEF (BT_TYPEDEF_E | BT_TYPEDEF_I)

/*! A type that can take a user defined name */
#define BT_NAMED_TYPE (BT_STRUCT | BT_UNION | BT_ENUM)

/*! integral type */
#define BT_INTEGRAL_BASE (BT_CHAR | BT_SHORT | BT_INT | BT_LONG | BT_LONGLONG)

/*! integral type */
#define BT_INTEGRAL (BT_INTEGRAL_BASE | BT_UNSIGNED | BT_ENUM | BT_BIT_FIELD)

/*! real type */
#define BT_REAL (BT_FLOAT | BT_DOUBLE | BT_LONGDOUBLE)

/*! arithmetic type */
#define BT_ARITHMETIC (BT_INTEGRAL | BT_REAL)

/*! aggregate type */
#define BT_STRUCTURE (BT_UNION | BT_STRUCT)

/*! any type */
#define BT_TYPE \
    (BT_VOID | BT_INTEGRAL | BT_REAL | BT_STRUCTURE | BT_TYPEDEF | BT_VARARG)

/* query macros */
/*! Matches an integral type */
#define TYPE_INTEGRAL(x) ((BT_INTEGRAL & (x)) && !(BT_REAL & (x)))

/*! Matches a real type */
#define TYPE_REAL(x)     ((BT_REAL & (x)) && !(BT_INTEGRAL & (x)))

/*! Type Qualifier specifier.
 *
 * These specify additional qualifications to a type.  After Psymtab generates
 * the symbol table, each Type will have a single data structure somewhere in
 * the symbol table.  Variables of that type will reference the general type
 * through the symbol table, but specific attributes (such as static, extern,
 * etc) will be stored in the ::Type.qualifier field.
 *
 * The qualifier field accumulates as one walks up the type tree.  For 
 * example, consider the following TypeDcls.
 *
 * TypeDcl Key {1, 1} BasicType BT_INT
 * TypeDcl Key {1, 2} BasicType BT_TYPEDEF_I TypeQual TY_EXTERN Type {1, 1}
 *
 * Type {1, 2} is an int with an extern qualifier.  If we then define a
 * variable as an extern const int, we might end up adding a new type
 * to the table:
 *
 * TypeDcl Key {1, 3} BasicType BT_TYPEDEF_I TypeQual TY_CONST Type {1, 2}
 *
 * To evaluate the qualifier, it is necessary to recurse up the type tree
 * and accumulate the qualifier.  The function PST_GetTypeQualifier() 
 * will do this for you.
 *
 * The library should try to compress typedef lists so that there are no
 * chains of BT_TYPEDEF_Is.  Therefore, the last example would really have
 * created the following type.
 *
 * TypeDcl Key {1, 3} BasicType BT_TYPEDEF_I
 *     TypeQual TY_CONST | TY_EXTERN Type {1, 1}
 *
 * Some of the qualifiers are primarily meaningful to the Pcode library, not
 * to code generation.
 *
 * TY_DEFAULT is set when a type has the default size and alignment and
 * has no other qualifiers.  This is used by the Pcode library when creating
 * a type.  Even if the size and alignment are not set, the library assumes
 * they have the default values for the type's basic type.  For example, the
 * library considers the following two types to be the same.
 *
 * TypeDcl Key {1, 1} BasicType BT_INT Size 4 Alignment 4
 * TypeDcl Key {1, 2} BasicType BT_INT TypeQual TY_DEFAULT
 *
 * If you attempted to add type {1, 2} to the table, PST_FindTypeDcl would
 * return the key {1, 1} instead of adding a new type.
 *
 * TY_EXP_ALIGN indicates that the TypeDcl has the alignment explicitly
 * set to a non-default value.  The alignment is set on the TypeDcl with
 * TY_EXP_ALIGN specified.  When walking up the type tree, the first
 * explicitly set alignment should be used.  PST_GetTypeAlignment() will
 * do this for you.
 *
 * The following three types describe a void * with alignment 16.
 *
 * TypeDcl Key {1, 1} BasicType BT_VOID
 * TypeDcl Key {1, 2} BasicType BT_POINTER Size 8 Alignment 8 Type {1, 1}
 * TypeDcl Key {1, 3} BasicType BT_TYPEDEF_I 
 *     TypeQual TY_EXP_ALIGN Alignment 16 Type {1, 2}
 */
typedef enum _TypeQual
{
  /* (qualifier) */
  TY_CONST 	  = 0x00000001, /*!< constant qualifier */
  TY_VOLATILE 	  = 0x00000002, /*!< volatile qualifier */
  TY_SYNC 	  = 0x00000004, /*!< sync qualifier */

  TY_IMPLICIT     = 0x00000008, /*!< An implicitly defined function has
				 * TY_IMPLICIT set on its type. */

  /* Qualifiers that are meaningful to the Pcode library. */
  TY_DEFAULT      = 0x00000010, /*!< type has default size, alignment, and
				 * no other qualifiers. */
  TY_EXP_ALIGN    = 0x00000020, /*!< type has alignment explicitly set to a
				 * non-standard value. */
  TY_UNNAMED      = 0x00000040, /*!< Set on TypeDcls for unnamed struct
				 * and union types. */
}
_TypeQual;

/*! Varable and Function qualifiers. */
typedef enum _VarQual
{
  VQ_DEFINED      = 0x00000001, /*!< variable is defined */
  VQ_COMMON       = 0x00000002, /*!< common variable */

  VQ_REGISTER 	  = 0x00000004, /*!< auto in register class */
  VQ_AUTO 	  = 0x00000008, /*!< auto class */
  VQ_STATIC 	  = 0x00000010, /*!< static class */
  VQ_EXTERN 	  = 0x00000020, /*!< extern class */
  VQ_GLOBAL	  = 0x00000040, /*!< global variable class */
  VQ_PARAMETER 	  = 0x00000080, /*!< function parameter class */
  VQ_IMPLICIT     = 0x00000100, /*!< variable is implicitly defined */

  VQ_CDECL        = 0x00000200, /*!< Visual C call convention */
  VQ_STDCALL      = 0x00000400, /*!< Visual C call convention */
  VQ_FASTCALL     = 0x00000800, /*!< Visual C call convention */

  /* Variable Attributes. */
  VQ_WEAK         = 0x00001000, /*!< Weak variable. */
  VQ_COMDAT       = 0x00002000, /*!< COMDAT (for C++). */

  /* Function Attributes. */
  VQ_CONSTRUCTOR  = 0x00004000, /*!< Function is a constructor. */
  VQ_DESTRUCTOR   = 0x00008000, /*!< Function is a destructor. */
  VQ_APP_ELLIPSIS = 0x00010000, /*!< Function has ellipses appended.  This
				 *   is the same as the append_gcc_ellipsis
				 *   pragma from impact-edgcpfe. */
  VQ_OLD_PARAM    = 0x00020000, /*!< Function uses old (K&R) style parameters.
				 */
}
_VarQual;

/*! global storage class */
#define VQ_GCLASS (VQ_STATIC | VQ_EXTERN | VQ_GLOBAL)

/*! local storage class */
#define VQ_LCLASS (VQ_REGISTER | VQ_AUTO | VQ_STATIC | VQ_PARAMETER)

/*! any storage class */
#define VQ_CLASS (VQ_LCLASS | VQ_GCLASS)

/*! Struct and Union qualifiers.
 *
 * SQ_COMPARING is used when comparing aggregate types.  When beginning
 * comparison of two struct or union types, SQ_COMPARING is set on both
 * types.  Each field is then compared.  In the event that the struct
 * is recursive, the struct comparison function will eventually be called
 * on the struct, see the SQ_COMPARING flag, and return. */
typedef enum _StructQual
{
  SQ_COMPARING    = 0x00000001, /*!< Struct is being compared. */
  SQ_EMPTY        = 0x00000002, /*!< Set for an empty struct. */
  SQ_INCOMPLETE   = 0x00000004, /*!< Struct has no definition and should
				 * be merged during linking. */
  SQ_UNNAMED      = 0x00000008, /*!< Struct was originally unnamed. */
  SQ_LINKMULTI    = 0x00000010, /*!< Union was formed by Plink validation. */
}
_StructQual;

/*! statement types */
/* BCC - 7/10/96
 * enumerate these constants. xups can display them in symbolic names
 */
typedef enum _StmtType
{
  ST_NOOP = 1,                  /*!< No operation (label) */
  ST_CONT = 2,                  /*!< continue statement */
  ST_BREAK = 3,                 /*!< break statement */
  ST_RETURN = 4,                /*!< return statement */
  ST_GOTO = 5,                  /*!< goto statement */
  ST_COMPOUND = 6,              /*!< compound statement */
  ST_IF = 7,                    /*!< if statement */
  ST_SWITCH = 8,                /*!< switch statement */
  ST_PSTMT = 9,                 /*!< parallel statement */
  ST_ADVANCE = 10,              /*!< advance statement */
  ST_AWAIT = 11,                /*!< await statement */
  ST_MUTEX = 12,                /*!< mutex statement */
  ST_COBEGIN = 13,              /*!< cobegin statement */
  ST_PARLOOP = 14,              /*!< parallel loop statement */
  ST_SERLOOP = 15,              /*!< serial loop statement */
  ST_EXPR = 16,                 /*!< expression statement */
  ST_BODY = 17,                 /*!< body statement */
  ST_EPILOGUE = 18,             /*!< epilogue statement */
  ST_ASM = 19                   /*!< inline assembly */
}
_StmtType;

/* BCC - 7/10/96
 * enumerate these constants. xups can display them in symbolic names
 */
/*! Enum to indicate label type. */
typedef enum _LabelType
{
  LB_LABEL = 1,                 /*!< Normal label */
  LB_CASE = 2,                  /*!< switch case label */
  LB_DEFAULT = 3                /*!< switch default label */
}
_LabelType;

/*! parallel loop types */
/* BCC - 7/10/96
 * enumerate these constants. xups can display them in symbolic names
 */
typedef enum _ParLoopType
{
  LT_DOALL = 1,                 /*!< DOALL loop */
  LT_DOACROSS = 2,              /*!< DOACROSS loop */
  LT_DOSERIAL = 3,              /*!< DOSERIAL loop */
  LT_DOSUPER = 4                /*!< DOSUPER loop */
}
_ParLoopType;

/*! serial loop types */
/* BCC - 7/10/96
 * enumerate these constants. xups can display them in symbolic names
 */
typedef enum _SerLoopType
{
  LT_WHILE = 1,                 /*!< while loop */
  LT_FOR = 2,                   /*!< for loop */
  LT_DO = 3                     /*!< do...while loop */
}
_SerLoopType;

/*! Opcodes for the various expression types.
 */
/* BCC - 7/10/96
 * enumerate these constants. xups can display them in symbolic names
 */
typedef enum _Opcode
{
  OP_var = 1,                   /*!< variable expression */
  OP_enum = 2,                  /*!< enum expression */
  OP_int = 3,                   /*!< integer literal expression */
  OP_real = 4,                  /*!< real literal expression */
  OP_error = 5,                 /*!< don't know what error used for */
  OP_char = 6,                  /*!< character literal expression */
  OP_string = 7,                /*!< string literal expression */
  OP_dot = 8,                   /*!< struct/union dot operator */
  OP_arrow = 9,                 /*!< struct/union arrow operator */
  OP_cast = 10,                 /*!< cast operation */
  OP_expr_size = 11,            /*!< expression size */
  OP_type_size = 12,            /*!< type size */
  OP_quest = 13,                /*!< ternary (?:) operator */
  OP_disj = 14,                 /*!< disjunction (||) operator */
  OP_conj = 15,                 /*!< conjunction (&&) operator */
  OP_compexpr = 16,             /*!< compound expression (expressions
				 * separated by comma */
  OP_assign = 17,               /*!< assignment operation */
  OP_or = 18,                   /*!< bitwise or (|) operation */
  OP_xor = 19,                  /*!< bitwise xor (^) operation */
  OP_and = 20,                  /*!< bitwise and (&) operation */
  OP_eq = 21,                   /*!< equal (==) operation */
  OP_ne = 22,                   /*!< not equal (!=) operation */
  OP_lt = 23,                   /*!< less than (<) operation */
  OP_le = 24,                   /*!< less than or equal (<=) operation */
  OP_ge = 25,                   /*!< greater than or equal (>=) operation */
  OP_gt = 26,                   /*!< greater than (>) operation */
  OP_rshft = 27,                /*!< right shift (>>) operation */
  OP_lshft = 28,                /*!< left shift (<<) operation */
  OP_add = 29,                  /*!< add (+) operation */
  OP_sub = 30,                  /*!< subtraction (-) operation */
  OP_mul = 31,                  /*!< multiplication (*) operation */
  OP_div = 32,                  /*!< division (/) operation */
  OP_mod = 33,                  /*!< modulo (%) operation */
  OP_neg = 34,                  /*!< negation (unary -) operation */
  OP_not = 35,                  /*!< not (!) operation */
  OP_inv = 36,                  /*!< bitwise invert (~) operation */
                                /* no opcode 37 */
  OP_preinc = 38,               /*!< pre increment (++var) operation */
  OP_predec = 39,               /*!< pre decrement (--var) operation */
  OP_postinc = 40,              /*!< post increment (var++) operation */
  OP_postdec = 41,              /*!< post decrement (var--) operation */
  OP_Aadd = 42,                 /*!< accumulated add (+=) operation */
  OP_Asub = 43,                 /*!< accumulated subtract (-=) operation */
  OP_Amul = 44,                 /*!< accumulated multiplication (*=)
				 * operation */
  OP_Adiv = 45,                 /*!< accumulated division (/=) operation */
  OP_Amod = 46,                 /*!< accumulated modulo (%=) operation */
  OP_Arshft = 47,               /*!< accumulated right shift (>>=) operation */
  OP_Alshft = 48,               /*!< accumulated left shift (<<=) operation */
  OP_Aand = 49,                 /*!< accumulated bitwise and (&=) operation */
  OP_Aor = 50,                  /*!< accumulated bitwise or (|=) operation */
  OP_Axor = 51,                 /*!< accumulated bitwise xor (^=) operation */
  OP_indr = 52,                 /*!< indirection (*var) operator */
  OP_addr = 53,                 /*!< address (&var) operator */
  OP_index = 54,                /*!< array index (var[]) operator */
  OP_call = 55,                 /*!< function call operation */
  OP_float = 56,                /*!< floating point literal expression */
                                /* BCC - added to distinguish float and double
				 * constants */
  OP_double = 57,               /*!< double literal expression */
  OP_null = 58,                 /*!< null operation (for PtoL) */
                                /* CWL 12/30/01 */
  OP_sync = 59,                 /*!< sync operation (for PtoL) */
                                /* CWL 01/09/02 */
  OP_stmt_expr = 60,            /*!< statement expression (gcc
				 * compatibility) */
  OP_asm_oprd  = 61,            /*!< assembly language operand */
  OP_phi = 62,                  /*!< phi opcode for SSA */
  OP_last = 63                  /*!< used to size opcode arrays.  Must be the
				 * final opcode in the enum. */
                                /* REK 7/25/03 */
}
_Opcode;

/*! An enum to indicate which field in the ::SymTabEntry.scope union is
 * valid. */
typedef enum _EntryType
{
  /* The field in the structure will default to 0, so make
   * sure 1 is the lowest valid enum value. */
  ET_NONE         = 0x00000000, /*!< No type */
  ET_FUNC         = 0x00000001, /*!< function */
  ET_TYPE_LOCAL   = 0x00000002, /*!< typedef (local to a compound stmt) */
  ET_TYPE_GLOBAL  = 0x00000004, /*!< typedef (global) */
  ET_VAR_LOCAL    = 0x00000008, /*!< local variable declaration */
  ET_VAR_GLOBAL   = 0x00000010, /*!< global variable declaration */
  ET_STRUCT       = 0x00000020, /*!< structure definition */
  ET_UNION        = 0x00000040, /*!< union definition */
  ET_ENUM         = 0x00000080, /*!< enum definition */
  ET_ASM          = 0x00000100, /*!< inline assembly */
  ET_STMT         = 0x00000200, /*!< statement definition */
  ET_EXPR         = 0x00000400, /*!< expression */
  ET_FIELD        = 0x00000800, /*!< struct/union field. */
  ET_ENUMFIELD    = 0x00001000, /*!< enum field. */
  ET_LABEL        = 0x00002000, /*!< label field. */
  ET_SCOPE        = 0x00004000, /*!< scope entry. */
  ET_BLOCK        = 0x00008000  /*!< a dummy entry heading a block of entries.
				 */
}
_EntryType;

/*! A constant to match either a local or global type. */
#define ET_TYPE (ET_TYPE_LOCAL | ET_TYPE_GLOBAL)

/*! A constant to match either a local or global var. */
#define ET_VAR (ET_VAR_LOCAL | ET_VAR_GLOBAL)

/*! A constant to match any symbol table entry. */
#define ET_ANY (ET_FUNC | ET_TYPE | ET_VAR | ET_STRUCT | ET_UNION | ET_ENUM | \
                ET_ASM | ET_STMT | ET_EXPR | ET_FIELD | ET_ENUMFIELD | \
                ET_LABEL | ET_SCOPE | ET_BLOCK)

/*! An enum to indicate if a file is a source file or a header. */
typedef enum _FileType
{
  FT_SOURCE,                    /*!< source file */
  FT_HEADER                     /*!< header file */
}
_FileType;

/*! An enum to indicate the status of a file handle.
 *
 * Diagram of file states.
 *
 * Input file
 * Begins in CLOSED state
 * 
 *               ,------read------>.     
 * .===========./                   \.-----------.
 * |  CLOSED   |                     | READ_PERM |
 * '==========='\                   /'-----------'
 *               `<--close|flush---'
 *
 *
 * Output file
 * Begin in NOT_AVAIL state
 *
 * .===========.      (input)      .--------.
 * | NOT_AVAIL |---write|update--->| CLOSED |<--------.
 * '==========='                   '--------'         |
 *                                     |              |
 *                              (input)   | (output)  |
 *                           write|update |   read    |
 *                                     |              |
 *                                     '--------------'
 */
typedef enum _FileStatus
{
  FS_CLOSED,                    /*!< File is closed (0, default).  This
				 * is equivalent to no part of the file being
				 * in memory. */
  FS_READ_PERM,                 /*!< File is open and locked with read
				 * permission.  This is equivalent to part
				 * of the file being in memory. */
  FS_NOT_AVAIL,                 /*!< File is not available. */

  /* There is no write permission state, as write permission should not
   * persist past the end of the function that obtains it. */
#if 0
  FS_WRITE_PERM   = 0x00000002, /*!< File is open (and locked) with write
				 * permission. */
  FS_APPEND_PERM  = 0x00000004, /*!< File is open (and locked) with append
				 * permission. */
  FS_FLUSHED      = 0x00000010, /*!< File has been flushed. */
#endif
}
_FileStatus;

/*! An enum to indicate which struct type a handler handles.  This is
 * structured as a bit field so that handlers can be set up for multiple
 * handlers with one call to the registration function. */
typedef enum _ExtStruct
{
  ES_FUNC,                      /*!< ::FuncDcl */
  ES_TYPE,                      /*!< ::TypeDcl */
  ES_VAR,                       /*!< ::VarDcl */
  ES_INIT,                      /*!< ::Init */
  ES_STRUCT,                    /*!< ::StructDcl */
  ES_UNION,                     /*!< ::UnionDcl */
  ES_FIELD,                     /*!< ::Field */
  ES_ENUM,                      /*!< ::EnumDcl */
  ES_STMT,                      /*!< ::Stmt */
  ES_PSTMT,                     /*!< ::Pstmt */
  ES_EXPR,                      /*!< ::Expr */
  ES_ASM,                       /*!< ::AsmDcl */
  ES_SYMTABENTRY,               /*!< ::SymTabEntry */
  ES_IPSYMTABENT,               /*!< ::IPSymTabEnt */
  ES_LAST                       /*!< Must be the last entry in the enum.
				 * Used to size arrays. */
}
_ExtStruct;

/*! An enum to hold bit patterns for the Expr.flags field. */
typedef enum _ExprFlags
{
  EF_UNSIGNED     = 0x00000001, /*!< OP_int or OP_long is unsigned. */
  EF_RETAIN       = 0x00000002, /*!< Do not consider expr useless. */
  EF_ENUM         = 0x00000004, /*!< var is enum (probably obsolete). */
  EF_TEMP         = 0x00000008, /*!< Expr is a Pcode temporary expression. */
  EF_VISITED      = 0X00000010, /*!< Used for marking exprs as visited */
}
_ExprFlags;

/*! An enum to hold bit patters for the Handler.options field. */
typedef enum _HandlerOptions
{
  HO_MANUAL_ALLOC = 0x00000001  /*!< User will manually allocate the ext. */
}
_HandlerOptions;

/*! An enum to hold bit patterns for the SymTabEntry.flags field. */
typedef enum _STEFlags
{
  STE_DELETED     = 0x00000001, /*!< Symbol has been deleted. */
  STE_FLUSHED     = 0x00000002, /*!< Symbol has been flushed to disk. */

  STE_FLUSH_ME    = 0x00000004, /*!< Symbol needs to be written to disk. */
}
_STEFlags;

/*! An enum to hold values for the IPSymTabEnt.flags field. */
typedef enum _IPSTEFlags
{
  IPSTEF_EMBEDDED   = 0x00000001, /*!< This file is embedded in the symbol
				   * table's file. */
  IPSTEF_NOT_AVAIL  = 0x00000002, /*!< This file is not available for reading.
				   */
}
_IPSTEFlags;

/*! An enum to hold values for the SymbolTable.flags field. */
typedef enum _STFlags
{
  /* Options that can be specified by the user. */
  STF_READ_ONLY   = 0x00000001, /*!< Table is read only (will not be written
				 * on PST_Close()) */

  /* Flags used by the Pcode library. */
  STF_SINGLE_FILE = 0x10000000, /*!< Single file processing mode. */
  STF_MULTI_FILE  = 0x20000000, /*!< Multi file processing mode. */
  STF_LINKED      = 0x40000000, /*!< Set after linking. */
  STF_REMOVING    = 0x80000000, /*!< Set when removing the symbol table.
				 * This disables some cross-symbol interaction
				 * that would otherwise require figuring
				 * out dependencies and removing Pcode in the
				 * correct order. */
}
_STFlags;

/*! An enum to hold values for the SymbolTable.source_order field. */
typedef enum _STSearchOrder
{
  SO_MEM,                       /*!< Read from memory. */
  SO_IN,                        /*!< Read from input file. */
  SO_OUT,                       /*!< Read from output file. */
  SO_NUM_SOURCES                /*!< Must be last in the enum.  Used for
				 * iteration. */
}
_STSearchOrder;

/*===========================================================================*/
/* Struct definitions */

/*!
 * It is desirable to attach additional information to the
 * data structures. In different phases of the compilation
 * process, different sets of extended definitions are necessary.
 * Therefore, we should keep the extension field general. We
 * provide a general purpose pointer to whatever data structures
 * the programmer will define in each phase of the compiler.
 */
typedef void *Extension;

/* Function pointer typedefs for the handler functions. */
/*! \brief Allocates extension data.
 *
 * \return
 *  A pointer to a new Extension data field.
 */
typedef Extension (*AllocHandler) (void);

/*! \brief Frees extension data.
 *
 * \param e
 *  a pointer to an Extension data field.
 *
 * \return
 *  A null Extension data field pointer.
 */
typedef Extension (*FreeHandler) (Extension e);

/*! \brief Copies extension data.
 *
 * \param e
 *  a pointer to an Extension data field.
 *
 * \return
 *  A pointer to a copy of the Extension data field.  This will be freed
 *  separately from \a e, so it must be distinct.
 */
typedef Extension (*CopyHandler) (Extension e);

/*! \brief Reads extension data from a file.
 *
 * \param e
 *  a pre-allocated Extension data field to fill.
 * \param sig
 *  the signature of the module that wrote this extension field.
 * \param raw
 *  the Extension data field in character string form.
 *
 * This function must take the character string in \a raw and fill the
 * Extension data \a e.  If the read handler reads extension data from
 * multiple modules, it can use the \a sig field to determine what it
 * is reading.
 *
 * This function must not free \a e, \a sig, or \a raw.  This function
 * must not preserve a pointer to \a sig or \a raw.
 *
 * \note \a raw is considered a character string, not a byte pointer.
 *       It will have a trailing null, and can be used with the str*
 *       functions from string.h.
 */
typedef void (*ReadHandler) (Extension e, char *sig, char *raw);

/*! \brief Writes extension data to a file.
 *
 * \param sig
 *  the signature of the module that is writing the extension field.
 * \param e
 *  the Extension data to write.
 *
 * \return
 *  a character string that represents the extension data.
 *
 * This function must translate the Extension data \a e into a character
 * string and return it to the caller.  The character string will be
 * freed by the caller, so it must be allocated on the heap.
 *
 * This function must not free \a sig or \a e.
 *
 * \note The returned string is considered a character string, not a byte
 *       pointer.  It must be safe to use with the str* functions from
 *       string.h.  In particular, this means it must have exactly one
 *       null byte at the end of the string.
 */
typedef char *(*WriteHandler) (char *sig, Extension e);

/*! A struct to hold function callbacks to handle the Extension (user data)
 * field. */
typedef struct ExtHandler
{
  AllocHandler alloc;           /*!< allocates an Extension. */
  FreeHandler free;             /*!< frees an Extension. */
  CopyHandler copy;             /*!< copies an Extension. */

  ReadHandler read;             /*!< reads an Extension. */
  WriteHandler write;           /*!< writes an Extension. */
  _HandlerOptions options;      /*!< options for this Extension. */
  char signature[EXT_SIG_LENGTH + 1]; /*!< the data signature. */
}
ExtHandler;

/*! Table key
 * This struct contains the symbol key and file key for a table entry.  This
 * struct should be used like a non-aggregate type.  Use the struct directly
 * and pass by copying instead of allocating the struct on the heap and
 * passing a pointer.
 */
typedef struct _Key
{
  int file;                     /*!< file key */
  int sym;                      /*!< symbol key */
}
Key;

/*! The maximum length of a key when written as a string.  This is two
 * 10 digit integers plus a separator character and a final null. */
#define KEY_STRLEN 22

/*! A Type is one or more TypeDcls linked through the symbol table.  Use
 * of the Type masks the underlying implementation from the module.
 *
 * Each Type is a unique C type.  The underlying TypeDcls may be shared
 * between Types, and therefore indicate relations between Types.
 *
 * The Type appears as a Key to the module. */
typedef Key Type;

/*! A linked list of ::Dcl structures.  The List.data field holds a pointer
 * to a ::Dcl. */
typedef List DclList;

/*! A variable list is a list of variable declarations.  List.data holds a
 * pointer to a ::VarDcl.
 */
typedef List VarList;

/*! A general structure to hold any type of declaration. */
typedef struct _Dcl
{
  _DclType type;                  /*!< indicates which field of the ::Dcl.ptr
				   * union is valid. */
  union
  {
    struct _FuncDcl *funcDcl;     /*!< func definition */
    struct _TypeDcl *typeDcl;     /*!< type definition */
    struct _VarDcl *varDcl;       /*!< variable declaration */
    struct _StructDcl *structDcl; /*!< struct definition */
    struct _UnionDcl *unionDcl;   /*!< union definition */
    struct _EnumDcl *enumDcl;     /*!< enum definition */
    struct _AsmDcl *asmDcl;       /*!< asm routine */
    char *include;                /*!< name of included file. */
    struct _SymbolTable *symbolTable; /*!< symbol table */
    struct _IPSymTabEnt *ipSymTabEnt; /*!< interprocedural symbol table */
    struct _SymTabEntry *symTabEntry; /*!< symbol table entry */
  } ptr;
}
_Dcl, *Dcl;

/*! Function definition
 *
 * A special extension field is defined, so when computation is
 * necessary, additional information can be attached to the
 * data structures.
 *
 * \todo Eliminate ::FuncDcl.filename field as it is probably redundant with
 * the new symbol table.
 */
typedef struct _FuncDcl
{
  char *name;			/*!< function name */
  Key key;                      /*!< symbol table key */
  Type type;                    /*!< return type */
  int lineno;			/*!< line number */
  int colno;			/*!< column number */
  _VarQual qualifier;           /*!< function qualifiers */
  char *filename;		/*!< filename (probably not needed) */
  VarList param;                /*!< parameter definition */
  struct _Stmt *stmt;		/*!< function compound stmt (body) */
  struct __Pragma *pragma;	/*!< pragma specifiers */
  struct _ProfFN *profile;	/*!< profile information */
  struct _Stmt *par_loop;	/*!< pointer to first parallel loop */
  struct _FuncFlow *flow;	/*!< pointer to flow information */
  struct _FuncDepend *depend;	/*!< pointer to dependence information */
  VarList local;                /*!< local variables */ /* CWL 08/27/00 */
  struct _Shadow *shadow;
  int max_expr_id;              /*!< the largest expr id of all exprs in
				 * the function. */
  Extension *ext;		/*!< extension field */
}
_FuncDcl, *FuncDcl;

/*! A linked list of TypeDcls in a Compound.  The List.data field holds a
 * ::TypeDcl. */
typedef List TypeList;

/*! A list of Keys.
 *
 * To hold the formal parameters list to a function prototype, we added this
 * struct which is embedded within ::Type.
 */
/* BCC - 1/20/96 */
typedef struct _KeyList
{
  Key key;                      /*!< symbol table key */
  struct _KeyList *next;        /*!< next parameter */
  struct _KeyList *last;        /*!< Last node in the list.  This is only
				 *   defined on the head node. */
}
_KeyList, *KeyList;

/*! The KeyList was originally called Param.  It was renamed to KeyList so
 * it could be used more generally.  The Param type still exists to hold
 * the keys of a function type's argument types.  These keys correspond
 * to TypeDcls. */
typedef KeyList Param;

/*! For ::_BasicType.BT_STRUCT, ::_BasicType.BT_UNION, and 
 * ::_BasicType.BT_ENUM, another field is required to specify
 * the structure.  The simplest way is to remember the keys of the
 * structure declaration (in ::TypeDcl.type).
 * When we are required to know the structure information, we can use the
 * keys to index into the symbol table.  This may be less time efficient
 * than remembering the location of the struture definition, and save the
 * symbol table lookup.  However, I trust that the symbol table hashing
 * scheme is good, and it is better to keep the data structures
 * independent of each other to simplify the debugging (shorter pointer
 * chains).
 */
typedef struct _TypeDcl
{
  _BasicType basic_type;        /*!< basic type (int, long, etc) */
  _TypeQual qualifier;          /*!< type qualifier (static, extern, etc) */
  Key key;                      /*!< symbol table key */
  Type type;                    /*!< this type's base Type (for a typedef,
				 * pointer, array, etc.) */
  char *name;                   /*!< the type's name */
  union
  {
    struct _Expr *array_size;   /*!< For an array type, the number of elements
				 * in the array.  This is valid if BT_ARRAY
				 * is set in ::Type.basic_type. */
    Param param;                /*!< For function type, the types of the
				 * function's arguments.  This is valid
				 * if BT_FUNC is set in ::Type.basic_type. */
  } details;
  int size;                     /*!< type size */
  int alignment;                /*!< type alignment */
  int lineno;                   /*!< line number */
  int colno;                    /*!< column number */
  int refs;                     /*!< reference count */
  char *filename;               /*!< filename (probably not needed) */
  struct __Pragma *pragma;      /*!< pragma specifiers */ /* AES 7/8/03 */
  Extension *ext;               /*!< extension field */
}
_TypeDcl, *TypeDcl;

/*! Variable Definition.
 *
 * A variable definition consists of the following fields:
 * \li variable name
 * \li variable type
 * \li initializer (for global and local variables)
 *
 * A special extension field is defined, so when computation is
 * necessary, additional information can be attached to the _VarDcl
 * data structure.
 *
 * The type field is a key into the symbol table.  The key corresponds to
 * a ::SymTabEntry with type ET_TYPE that contains the ::TypeDcl for this
 * variable.
 *
 * \todo Eliminate ::VarDcl.filename field as it is probably redundant with
 * the new symbol table.
 */
typedef struct _VarDcl
{
  char *name;			/*!< variable name */
  Key key;                      /*!< symbol table key */
#if 0
  char *new_name;		/* name after renaming */
#endif
  Type type;                    /*!< the variable's Type */
  struct _Init *init;		/*!< data initialization */
  int lineno;			/*!< line number */
  int colno;			/*!< column number */
  int align;                    /*!< variable alignment */
  _VarQual qualifier;           /*!< variable qualifiers */
  char *filename;		/*!< filename (probably not needed) */
  struct __Pragma *pragma;	/*!< pragma specifiers */
  Extension *ext;		/*!< extension field */
}
_VarDcl, *VarDcl;

/*! A variable initializer
 *
 * Variables (not just global) can be initialized by aggregate initializer.
 * Multiple level of aggregate initialization can occur. The simplest
 * case is when it is just a simple expression.
 *
 * \li When it is a simple expression, the set field is set to 0.
 * \li When it is an aggregate initializer, the expr field is set to 0.
 *
 * A special extension field is defined, so when computation is
 * necessary, additional information can be attached to the _Init
 * data structure.
 */
typedef struct _Init
{
  struct _Expr *expr;		/*!< initial expression */
  struct _Init *set;		/*!< initializers for each member of an
				 * aggregate */
  struct _Init *next;		/*!< next initializer in the set */
  struct __Pragma *pragma;      /*!< pragma specifiers */
  Extension *ext;		/*!< extension field */
}
_Init, *Init;

/*! Struct/Union Definition.
 *
 * Struct and union declarations are very much alike. 
 *
 * Each has the following attributes:
 * \li its name
 * \li its fields
 *
 * A special extension field is defined, so when computation is
 * necessary, additional information can be attached to these
 * data structures.
 *
 * \todo Eliminate ::StructDcl.filename as it is probably redundant with
 * the new symbol table.
 */
typedef struct _StructDcl
{
  char *name;			/*!< structure name */
  Key key;                      /*!< symbol table key */
#if 0
  char *new_name;		/* name after renaming */
#endif
  _StructQual qualifier;        /*!< struct qualifiers */
  struct _Field *fields;	/*!< field definition */
  int lineno;			/*!< line number */
  int colno;			/*!< column number */
  char *filename;		/*!< filename (probably not needed) */
  int size;			/*!< struct size */ /* BCC 2/17/97 */
  int align;			/*!< first element alignment */ /* BCC */
  int group;			/*!< equivalence group id */ /* BCC */
  struct __Pragma *pragma;	/*!< pragma specifiers */ /* AES 7/8/03 */
  Extension *ext;		/*!< extension field */
}
_StructDcl, *StructDcl;

/*! Struct/Union Definition.
 *
 * Struct and union declarations are very much alike. 
 *
 * Each has the following attributes:
 * \li its name
 * \li its fields
 *
 * A special extension field is defined, so when computation is
 * necessary, additional information can be attached to these
 * data structures.
 *
 * \todo Eliminate ::UnionDcl.filename as it is probably redundant with
 * the new symbol table.
 */
typedef struct _UnionDcl
{
  char *name;			/*!< union name */
  Key key;                      /*!< symbol table key */
#if 0
  char *new_name;		/* name after renaming */
#endif
  _StructQual qualifier;        /*!< union qualifiers */
  struct _Field *fields;	/*!< field definition */
  int lineno;			/*!< line number */
  int colno;			/*!< column number */
  char *filename;		/*!< filename (probably not needed) */
  int size;			/*!< union size */ /* BCC 2/17/97 */
  int align;			/*!< first element alignment */ /* BCC */
  int group;			/*!< equivalence group id */ /* BCC */
  struct __Pragma *pragma;	/*!< pragma specifiers */ /* AES 7/8/03 */
  Extension *ext;		/*!< extension field */
}
_UnionDcl, *UnionDcl;

/*! Each field of the ::StructDcl or ::UnionDcl structures consists of
 * several subattributes:
 * \li name of the field
 * \li type declaration
 * \li bit field length
 * \li link to the next field
 *
 * The ::Field.bit_field attribute specifies the number of bits of this
 * field.  This allows the user to pack fields tightly together to reduce
 * data space requirement. The ::Field.bit_field attribute is a constant
 * expression, which can be expressed by using the ::Expr data structure.
 * It is not sufficient to use an integer to represent ::Field.bit_field
 * because of sizeof() and enum constants in C.
 *
 * A special extension field is defined, so when computation is
 * necessary, additional information can be attached to the
 * data structures.
 *
 * The old Field structure contained align and size fields.  The new type
 * system simply uses the align and size fields on the Field's type
 * structure.
 */
typedef struct _Field
{
  char *name;			/*!< field name */
  Key key;                      /*!< field symbol table key */
  Key parent_key;               /*!< parent struct or union's symbol table
				 * key */
  Type type;                    /*!< the field's Type */
  int is_bit_field;             /*!< is this a bit field */
  int bit_size;         	/*!< number of bits (if bitfield) */
  int bit_offset_remainder;     /*!< offset (in bits) of start of bit field  */
  struct _Field *next;		/*!< next field */
  int offset;			/*!< field offset in struct/union */
                                /* BCC 2/17/97 */
  struct __Pragma *pragma;	/*!< pragma specifiers */ /* AES 7/8/03 */
  Extension *ext;		/*!< extension field */
}
_Field, *Field;

/*! Enumeration Type.
 *
 * Enum structure is a fairly simple data structure which
 * has a name and a list of enum constants.
 *
 * \todo Eliminate ::EnumDcl.filename as it is probably redundant with
 * the new symbol table.
 */
typedef struct _EnumDcl
{
  char *name;			/*!< enum name*/
  Key key;                      /*!< symbol table key */
#if 0
  char *new_name;		/* name after renaming */
#endif
  struct _EnumField *fields;	/*!< list of enum fields */
  int lineno;			/*!< line number */
  int colno;			/*!< column number */
  char *filename;		/*!< filename (probably not needed) */
  struct __Pragma *pragma;	/*!< pragma specifiers */ /* AES 7/8/03 */
  Extension *ext;		/*!< extension field */
}
_EnumDcl, *EnumDcl;

/*! EnumField
 * Represents one enum constant.
 *
 * Each enum constant has a name and a value attribute. The value
 * attribute is assigned by the compiler. 
 */
typedef struct _EnumField
{
  char *name;			/*!< field name */
  Key key;                      /*!< symbol table key */
#if 0
  char *new_name;		/* name after renaming */
#endif
  struct _Expr *value;		/*!< constant value */
  struct _EnumField *next;	/*!< next enum field */
}
_EnumField, *EnumField;

/*! The Stmt structure is generic for all statments.  The :Stmt.stmtstruct
 * defines the specific structure.
 *
 * Statement structures: \n
 * simple: noop, cont, break \n
 * pointer: return, goto, label \n
 * structure: compound, if, switch, case, default, pstmt, advance, await,
 * mutex, cobegin, parloop, serloop \n
 *
 * \todo Eliminate ::Stmt.filename as it is probably redundant with
 * the new symbol table.
 */
typedef struct _Stmt
{
  _StmtType type;		    /*!< type of statement */
  Key key;                          /*!< symbol table key */
  int status;			    /*!< flags for marking statements */
                                    /* GEH */
  int lineno;			    /*!< line number */
  int colno;			    /*!< column number */
  int artificial;		    /*!< ignored by code gen */
  int foroverlap;                   /*!< ignored by code gen */   
  char *filename;		    /*!< filename (probably not needed) */
  struct _ProfST *profile;	    /*!< profile information */
  struct __Pragma *pragma;	    /*!< pragma specifiers */
  struct _Shadow *shadow;  
  struct _Stmt *lex_prev;	    /*!< lexically previous stmt */
  struct _Stmt *lex_next;	    /*!< lexically next stmt */
  struct _Label *labels;	    /*!< list of label of type
				     * label, case, and default */
  union
  {
    struct _Expr *ret;		    /*!< for return; ptr to returned
				     * expression */
    struct
    {
      char *val;                    /*!< goto label value (string) */
      Key key;                      /*!< goto label key. */
    } label;
    struct _Compound *compound;	    /*!< compound statement */
    struct _IfStmt *ifstmt;	    /*!< if statement */
    struct _SwitchStmt *switchstmt; /*!< switch statement */
    struct _Pstmt *pstmt;	    /*!< parallel statement */
    struct _Advance *advance;	    /*!< advance statement */
    struct _Await *await;	    /*!< await statement */
    struct _Mutex *mutex;	    /*!< mutex statement */
    struct _Cobegin *cobegin;	    /*!< cobegin statement */
    struct _ParLoop *parloop;	    /*!< parallel loop statement */
    struct _SerLoop *serloop;	    /*!< serial loop statement */
    struct _Expr *expr;		    /*!< expression statement */
                                    /* 1/2/92 */
    struct _BodyStmt *bodystmt;	    /*!< body of loop statement */
    struct _EpilogueStmt *epiloguestmt;	/*!< loop epilogue */
    struct _AsmStmt *asmstmt;	    /*!< inline assembly */
  } stmtstruct;
  struct _Stmt *parent;		    /*!< parent statement */
  struct _FuncDcl *parent_func;     /*!< parent function of statement */
  struct _Expr *parent_expr;        /*!< parent expression (for statement
				     * expressions) */
  struct _StmtFlow *flow;	    /*!< pointer to flow information */
  Extension *ext;		    /*!< extension field */
  void *ext2;
}
_Stmt, *Stmt;

/*! Structure representing a label. */
typedef struct _Label
{
  char *val;                    /*!< Label text */
  Key key;                      /*!< symbol table key */
  _LabelType type;              /*!< Label type (normal, switch case, 
				 * switch default) */
  struct
  {
    struct _Expr *expression;	/*!< expression for switch case */
    struct _Stmt *parent;       /*!< parent statement symbol table key for
				 * normal label case.  This lets us get the
				 * statement after looking a label up in the
				 * symbol table. */
  }
  data;
  struct _Label *next;          /*!< next label for this statement */
  struct _Label *prev;          /*!< previous label for this statement */
}
_Label, *Label;

/*! Compound statement consists of the locally defined (can be global)
 * variables and the list of stmts. */
typedef struct _Compound
{
  TypeList type_list;           /*!< type list */
  VarList var_list;	        /*!< variable list */
  struct _Stmt *stmt_list;	/*!< ptr to stmt list */
  unsigned int unique_var_id;   /*!< used to guarantee unique temp
				 * identifiers. */
  struct _Stmt *parent;         /*!< parent statement */
}
_Compound, *Compound;

/*! If statement consists of the if statment with conditional expression,
 * then block and and else block. */
typedef struct _IfStmt
{
  struct _Expr *cond_expr;	/*!< conditional expression */
  struct _Stmt *then_block;	/*!< then block */
  struct _Stmt *else_block;	/*!< else block */
  struct _Stmt *parent;         /*!< parent statement */
}
_IfStmt, *IfStmt;

/*! Switch statement consists of the expression and switch body. */
typedef struct _SwitchStmt
{
  struct _Expr *expression;	/*!< expression of switch */
  struct _Stmt *switchbody;	/*!< ptr to switch body */
  struct _Stmt *parent;         /*!< parent statement */
}
_SwitchStmt, *SwitchStmt;

/*! Parallel statement consists of a stmt and ::Extension field for any
 * additional info that may want to keep for a parallel stmt (such as list of
 * variables passed).
 *
 * \todo Eliminate ::Pstmt.filename as it is probably redundant with
 * the new symbol table.
 */
typedef struct _Pstmt
{
  struct _Stmt *stmt;		/*!< statement of pstmt */
  int lineno;			/*!< line number */
  int colno;                    /*!< column number */
  char *filename;               /*!< filename (probably not needed) */
  struct __Pragma *pragma;      /*!< pragma specifiers */
  struct _Stmt *parent;         /*!< parent statement */
  Extension *ext;		/*!< extension field */
}
_Pstmt, *Pstmt;

/*! The Advance statement consists of a marker. */
typedef struct _Advance
{
  int marker;			/*!< marker of advance/await pair */
}
_Advance, *Advance;

/*! The Await statement consists of a marker and a distance. */
typedef struct _Await
{
  int marker;			/*!< marker of advance/await pair */
  int distance;			/*!< distance to wait for */
}
_Await, *Await;

/*! The Mutex consists of an expression and statement (expression should
 * be simple). */
typedef struct _Mutex
{
  struct _Expr *expression;	/*!< expression of mutex */
  struct _Stmt *statement;	/*!< statement of mutex */
  struct _Stmt *parent;         /*!< parent statement */
}
_Mutex, *Mutex;

/*! The Cobegin statement consists of a list of statements. */
typedef struct _Cobegin
{
  struct _Stmt *statements;	/*!< list of statements */
  struct _Stmt *parent;         /*!< parent statement */
}
_Cobegin, *Cobegin;

/*
 * DoPstmt = local var decl, prologue stmt list, body stmt, epilogue stmt list,
 * 	     ext (can be used to point to variables that must be passed into
 *	     the pstmt).  The local variable declarations are actually in
 *	     the prologue when code is generated.  Local variables to the
 *	     loop body only can be placed in the compound stmt of body.
 *
 * Make prologue, body, and epilogue pstmts since when do check - each has
 * to be one.
 */
typedef struct _BodyStmt
{
  struct _Stmt *statement;	/*!< loop body stmt */
  struct _Stmt *parent;         /*!< parent statement */
}
_BodyStmt, *BodyStmt;

typedef struct _EpilogueStmt
{
  struct _Stmt *statement;	/*!< loop epilogue stmt */
  struct _Stmt *parent;         /*!< parent statement */
}
_EpilogueStmt, *EpilogueStmt;

/*! A ParLoop consists of a loop type, loop body, iteration var,
 * next par_loop stmt, loop prologue, loop epilogue, init value
 * expression, final value expression, and increment expression.
 */
typedef struct _ParLoop
{
  _ParLoopType loop_type;	/*!< type of parallel loop */
  struct _Pstmt *pstmt;         /*!< loop parallel statement */
  struct _Expr *iteration_var;	/*!< iteration variable */
  struct _Stmt *sibling;	/*!< sibling at same level */
  struct _Expr *init_value;	/*!< initial value expression */
  struct _Expr *final_value;	/*!< final value expression */
  struct _Expr *incr_value;	/*!< increment expression */
  struct _Stmt *child;		/*!< Linked list of all children */
  struct _Stmt *parent;		/*!< parent, if any */
  unsigned int depth;		/*!< loop's nesting depth (Used in DD) */
}
_ParLoop, *ParLoop;

/*! A SerLoop consists of a loop type, loop body, conditional
 * expression of loop (while/for/do), initial expression (for),
 * iterative expression(for)
 */
typedef struct _SerLoop
{
  _SerLoopType loop_type;	/*!< type of serial loop */
  struct _Stmt *loop_body;	/*!< loop body */
  struct _Expr *cond_expr;	/*!< conditional expression */
  struct _Expr *init_expr;	/*!< initial expression */
  struct _Expr *iter_expr;	/*!< iterative expression */
  struct _Stmt *parent;         /*!< parent statement */
}
_SerLoop, *SerLoop;

/*! An Asm statement consists of assembly code.
 */
typedef struct _AsmStmt
{
  char is_volatile;
  struct _Expr *asm_clobbers;
  struct _Expr *asm_string;     /*!< assembly code */
  struct _Expr *asm_operands;   /*!< operands to inline assembly */
  struct _Stmt *parent;         /*!< parent statement */
}
_AsmStmt, *AsmStmt;

typedef struct _Asmoprd
{
  int   modifiers;
  char *constraints;
}
_Asmoprd, *Asmoprd;

/*===========================================================================*/
/*! An expression.
 *
 * The ::Expr.status field is provided for storing temporary information
 * in computations on expressions. The ::Expr.opcode field is an integer
 * because integer comparison is much faster than string comparison.
 * We need another table to map the name of opcode to a unique
 * integer value, including pre-defined operators.
 * The ::Expr.type field specifies the type of the result of the expression.
 * If the expression is a primary type, the ::Expr.value field specifies
 * its value. The ::Expr.sibling field is used to link all operands together.
 * The ::Expr.parent and the child pointers connect all expressions in a
 * basic block together. A parent expression is executed before a
 * child expression.
 * Finally, a special extension field is defined, so when computation is
 * necessary, additional information can be attached to the
 * data structures.
 *
 * Add field to indicate that a var is actually an enum (would use
 * and existing field s.a. status or ext but may be modified during
 * expression reduction or other expression calcs).
 *
 * \todo ::Expr.ext2 (maybe ::Expr.acc and ::Expr.acc2 as well) should
 * go away.
 */
typedef struct _Expr
{
  int id;                        /*!< Expression ID */

  int status;			 /*!< status */

  Key key;                       /*!< symbol table key */
  _Opcode opcode;		 /*!< opcode */
  _ExprFlags flags;              /*!< Expr attributes used by Pcode. */
  Key type;                      /*!< type symbol table key */
  union
  {
    ITintmax scalar;		 /*!< signed scalar value */
    ITuintmax uscalar;		 /*!< unsigned scalar value */
    double real;		 /*!< floating point value */
    char *string;		 /*!< string/char/enum literal */
    struct
    {
      char *name;                /*!< name of variable/function/label */
      Key key;                   /*!< variable symbol table key */
      struct _PSS_Def *ssa;      /*!< ssa definition linked to var */ 
    }
    var;
    Type type;                   /*!< type_size/cast Type */
    struct _Stmt *stmt;          /*!< Statement expr's compound stmt */
    struct _Asmoprd  *asmoprd;   /*!< Assembly oprd info */
  } value;
  struct _Expr *sibling;         /*!< next expression in a compound
				  * expression or next operand. */
  struct _Expr *operands;        /*!< list of operands */
  struct _Expr *parentexpr;	 /*!< parent expr, if any */
  struct _Stmt *parentstmt;	 /*!< parent stmt */
  struct _VarDcl *parentvar;     /*!< parent variable (for an initializer). */
  struct _Expr *next, *previous; /*!< expression list links */
  struct _Expr *bb_next;	 /*!< next expr within the same bb */ 
                                 /* CWL - 12/29/01 */
  struct __Pragma *pragma;	 /*!< pragma */
  struct _ProfEXPR *profile;	 /*!< profile info */ /* LCW */
  Extension *ext;                /*!< extension field */

#if 1
  /* THE FOLLOWING EXISTS TEMPORARILY TO ALLOW
     SOME OLD MODULES TO WORK. HOWEVER, IT THEY
     WILL DISAPPEAR, SO DON'T USE THEM IN ANY
     NEW/RETAINED CODE */
  void             *acc;         /*!< acc tbl entry */
  void             *acc2;        /*!< second acc tbl entry */
  void             *ext2;
  void             *ext3;
#endif
}
_Expr, *Expr;

/*! Pragma specification */
typedef struct __Pragma
{
  char *specifier;		/*!< the specifier is the string */
  struct _Expr *expr;		/*!< exressions of pragma */
  int lineno;			/*!< line number */
  int colno;			/*!< column number */
  char *filename;		/*!< filename */
#if 0
  /* CWL - 12/30/01 for P-to-L */
  DepInfo dep_info;		
#endif
  struct __Pragma *next;	/*!< a link to the next specifier */
}
__Pragma, *Pragma;

/*! A struct to hold source code position information (file, line, column). */
typedef struct _Position
{
  int lineno;                   /*!< line number */
  int colno;                    /*!< column number */
  char *filename;               /*!< filename */
}
_Position, *Position;

/*! A struct to hold an identifier.  This is a symbol name and key pair. */
typedef struct _Identifier
{
  char *name;                   /*!< identifier name */
  Key key;                      /*!< identifer symbol table key */
}
_Identifier, *Identifier;

/*
 * Profile records - to be defined at a later point.  Now they are null 
 * structures.  There are two structures required in pcode - one for function
 * and one for statements.
 */
/* LCW - define the following structures for Pcode profiling - 10/23/95 */
/* LCW - copy the following data structures from Hcode for passing the 
 *	 profiling information to Hcode - 12/23/95 */
/*! Profile record for a function. */
typedef struct _ProfFN
{
  int fn_id;			/*!< profile id */
  double count;                 /*!< profile count */
  struct _ProfCS *calls;	/*!< call sites */
}
_ProfFN, *ProfFN;

/*! Profile record for a call site. */
typedef struct _ProfCS
{
  int call_site_id;		/*!< call site id */
  int callee_id;		/*!< callee id */
  double weight;		/*!< invocation count */
  struct _ProfCS *next;         /*!< next call site */
}
_ProfCS, *ProfCS;

/*! Profile record for a basic block. */
typedef struct _ProfBB
{
  double weight;		/*!< profile weight */
  struct _ProfArc *destination;	/*!< control transitions */
}
_ProfBB, *ProfBB;

/*! Profile record for an arc. */
typedef struct _ProfArc
{
  int bb_id;			/*!< destination bb */
  int condition;		/*!< branch condition */
  double weight;		/*!< profile weight */
  struct _ProfArc *next;        /*!< next arc */
}
_ProfArc, *ProfArc;

/* LCW - define the following structures for Pcode profiling - 10/23/95 */
/*! Profile record for a statement. */
typedef struct _ProfST
{
  double count;                 /*!< profile count */
  struct _ProfST *next;         /*!< next statement */
}
_ProfST, *ProfST;

/*! Profile record for an expression. */
typedef struct _ProfEXPR
{
  double count;                 /*!< profile count */
  struct _ProfEXPR *next;       /*!< next expression */
}
_ProfEXPR, *ProfEXPR;

typedef struct _Shadow
{
  int param_id;
  struct _Expr   *expr;
  struct _Shadow *next;
}
_Shadow, *Shadow;

/* IMS 7/3/03 */
/*! Assembly Routine
 *
 * Programmers can place assembly code outside of functions.  Such code
 * can declare functions (in assembly).  This was added because this
 * is used in the Linux Kernel.
 *
 * The following fields are defined for a function:
 * \li Is the assembly volatile
 * \li Is it "clobbers" ??
 * \li The Assembly String (asm_string)
 * \li Specified Operands
 * 
 * A special extension field is defined, so when computation is
 * necessary, additional information can be attached to the
 * data structures.
 */
typedef struct _AsmDcl
{
  char is_volatile;
  struct _Expr *asm_clobbers;
  struct _Expr *asm_string;     /*!< inline assembly code */
  struct _Expr *asm_operands;   /*!< assembly language operands */
  Key key;                      /*!< symbol table key */
  int lineno;                   /*!< line number */
  int colno;                    /*!< column number */
  char *filename;               /*!< filename */
  struct __Pragma *pragma;      /*!< pragma specifier */
  Extension *ext;		/*!< extension field */
}
_AsmDcl, *AsmDcl;

/*! The ScopeEntry data structure represents a single table entry that is
 * defined in a scope.  It forms a linked list with the other table entries
 * defined under the scope. */
typedef KeyList ScopeEntry;

/*! The Scope data structure is a list of keys of symbols defined in the
 * scope.  It also contains the keys for the structure that defines
 * the scope (function, file, etc). */
typedef struct _Scope
{
  Key key;                         /*!< symbol table key for the file,
				    * function, etc. that defines this
				    * scope. */
  ScopeEntry scope_entry;          /*!< A linked list of table entries defined
				    * in this scope. */
}
_Scope, *Scope;

/*! Symbol table entry.
 * Each symbol in a program has a copy of this structure in the symbol
 * table.
 *
 * The next and prev fields can be used to sort the symbol table into some
 * order without changing symbol keys.  All SymTabEntries in a table must
 * have next and prev be Invalid_Key ({0, 0}) or all must have next and prev
 * set to the keys of the next and previous SymTabEntries in the order.
 */
typedef struct _SymTabEntry
{
  Key key;                      /*!< The key for this table entry. */
  char *name;                   /*!< Table entry's name.  Not defined
                                 * for all table entries. */
  Key scope_key;                /*!< The key for this table entry's scope. */
  _EntryType type;              /*!< Indicates which field of the ptr
                                 * union is valid. */
  union
  {
    FuncDcl func_dcl;           /*!< function */
    TypeDcl type_dcl;           /*!< typedef */
    VarDcl var_dcl;             /*!< variable declaration */
    StructDcl struct_dcl;       /*!< struct definition */
    UnionDcl union_dcl;         /*!< union definition */
    EnumDcl enum_dcl;           /*!< enum definition */
    AsmDcl asm_dcl;             /*!< inline assembly */
    Stmt stmt;                  /*!< statement definition */
    Expr expr;                  /*!< expression */
    Field field;                /*!< struct/union field. */
    EnumField enum_field;       /*!< enum field. */
    Label label;                /*!< label field. */
    struct
    {
      Key start;                /*!< key of first SymTabEntry in following
				 * block. */
      int size;                 /*!< number of SymTabEntries following this one
				 * (does not include this one). */
    } block;
  } entry;
  Scope scope;                  /*!< If this table entry has a scope
                                 * (func, compound stmt, etc), this
                                 * will point to a ::Scope structure. */
  int offset;                   /*!< The offset of the dcl in its source
                                 * file. */
  Key next;                     /*!< The key of the next SymTabEntry for
				 * the PST_Get*Entry[Next] functions. */
  Key prev;                     /*!< The key of the previous SymTabEntry. */
  _STEFlags flags;              /*!< Symbol attribute flags. */
  struct __Pragma *pragma;      /*!< pragma specifier */
  Extension *ext;               /*!< User data. */
}
_SymTabEntry, *SymTabEntry;

/*! Interprocedural symbol table entry.
 * Each file in a program has a copy of this structure in the symbol table. */
typedef struct _IPSymTabEnt
{
  char *source_name;            /*!< filename (original source file) */
  char *in_name;                /*!< filename (input) */
  char *out_name;               /*!< filename (output) */
  _FileType file_type;          /*!< file type (FT_SOURCE or FT_HEADER) */
  int key;                      /*!< file key */
  int num_entries;              /*!< the number of table entries in
				 * IPSymTabEnt.table. */
  int offset;                   /*!< The file position of the first SymTabEntry
				 * for this file. */
  BlockSparseArray table;       /*!< The single file symbol table. */
  FILE *file;                   /*!< A filehandle for the source file. */
  _FileStatus in_file_status;   /*!< The status of the input file. */
  _FileStatus out_file_status;  /*!< The status of the output file. */
  _IPSTEFlags flags;            /*!< File attribute flags. */
  struct __Pragma *pragma;      /*!< pragma specifier */
  Extension *ext;               /*!< User data. */
}
_IPSymTabEnt, *IPSymTabEnt;

/*! A typedef of an array to hold the symbol table's search order. */
typedef _STSearchOrder SearchOrder[SO_NUM_SOURCES];

/*! Symbol table */
typedef struct _SymbolTable
{
  char *ip_table_name;            /*!< The filename of the interprocedural
				   * symbol table. */
  char *out_name;                 /*!< filename (output) */
  int modifiable_file;            /*!< The key of the file which can be
				   * modified.  This is 0 in multi-file
				   * mode. */
  int num_files;                  /*!< The number of files in the table. */
  struct _IPSymTabEnt **ip_table; /*!< The interprocedural symbol table.  This
				   * is a simple array, since insertions
				   * and deletions are rare. */
  SearchOrder search_order;       /*!< Used to prioritize where symbols are
				   * read from. */
  _STFlags flags;                 /*!< Symbol table attribute flags. */
  FILE *file;                     /*!< A filehandle for the symbol table
				   * file. */
  _FileStatus in_file_status;     /*!< The status of the input file. */
  _FileStatus out_file_status;    /*!< The status of the output file. */
}
_SymbolTable, *SymbolTable;

/*! A structure to map keys to new keys and uses.  This is used when
 * changing an entry's key, such as when copying a statement.  It allows
 * changing all keys in one pass. */
typedef struct _KeyMap
{
  Key new_key;                  /*!< This entry's new key. */
#if 0
  List users;                   /*!< A list of structures that refer to this
				 * key.  Each entry in this list is a
				 * ::STEData struct. */
#endif
}
_KeyMap, *KeyMap;


/*! Memory Dependence Information
 *    For now it is simply a list of ints i >= 1 where
 *    two mem exprs are dependent if there lists share
 *    the same number.
 */
typedef struct _P_memdep_core_t
{
  int   is_def;
  int   id;
  int   version;
  int   offset;
  int   size;
} _P_memdep_core_t, *P_memdep_core_t;

typedef struct _P_memdep_t
{
  List deps;
  Set  dep_set;
} _P_memdep_t, *P_memdep_t;


/*****************************************************************************
 * Variable declarations                                                     *
 *****************************************************************************/

/*! The parameter file indicated on the command line. */
extern char *P_parm_file;

/*! The Dcl returned by the parser. */
extern Dcl P_Input;

extern void P_init_io (void);
extern void P_init_handlers (char *prog_name, Parm_Macro_List *external_list);

/*! Handlers to manage the ext field on various data structures.
 *
 * To use the extension field on a Pcode data structure:
 * -Define a custom struct.
 * -Define handler functions to allocate, free, etc. your struct.
 * -Define P_def_handlers().  This function should call P_RegisterExtHandler()
 *  to register your handler functions.
 * -Each Pcode structure of the appropriate type will have a copy of
 *  your struct attached to its ext field.
 */
extern ExtHandler **Handlers;

/*! An array to keep track of the last library index in an ext array. */
extern int NumExtensions[ES_LAST];

/*! An array to map opcodes to human readable strings. */
extern const char *op_to_value[OP_last];

/*! An array to map _ExtStruct values to human readable strings. */
extern const char *ExtStruct_To_String[ES_LAST];

/*! An invalid key that can be used by any Pcode module. */
extern const Key Invalid_Key;

/*! The Pcode library's index in the extension field. */
extern int Indices[ES_LAST];

/*! \brief Maps _Opcode values to human readable strings.
 *
 * \param o
 *  the opcode.
 *
 * \return
 *  A human readable equivalent to \a o.
 */
#define P_OpcodeToString(o) (op_to_value[(o)])

/*! \brief Maps _ExtStruct values to human readable strings.
 *
 * \param e
 *  the _ExtStruct value.
 *
 * \return
 *  A human readable equivalent to \a e.
 */
#define P_ExtStructToString(e) (ExtStruct_To_String[(e)])

extern char *P_BasicTypeToString (_BasicType b);
extern char *P_EntryTypeToString (_EntryType e);

#endif
