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
 *	File:	hl_code.c
 *	Author:	Po-hua Chang, Wen-mei Hwu
 *	Creation Date:	June, 1990
 *	Revised: Dave Gallagher, Scott Mahlke - June 1994
 *		Build Lcode structure rather than just printing out text file
 *      Revised by: Ben-Chung Cheng - June 1995
 *              Change M_SIZE_INT, M_SIZE_CHAR to H_INT_SIZE, P_CHAR_SIZE
 *              Those should be determined in runtime, not compile time
 *        Chien-Wei Li - 12/2001
 *          change H_CHAR_SIZE to P_CHAR_SIZE.
\*****************************************************************************/
#include <config.h>
#include "pl_main.h"


/* This are used throughout PtoL for
 *  the sizing of the fundemental types.
 */

int P_CHAR_SIZE = 0;
int P_SHORT_SIZE = 0;
int P_INT_SIZE = 0;
int P_LONG_SIZE = 0;
int P_LONGLONG_SIZE = 0;
int P_POINTER_SIZE = 0;
int P_UNSIGNED_CHAR_MASK = 0;
int P_UNSIGNED_SHORT_MASK = 0;
int P_UNSIGNED_INT_MASK = 0;
int P_UNSIGNED_LONG_MASK = 0;
int P_UNSIGNED_LONGLONG_MASK = 0;
int PL_file_scope = -1;


/*
 * THIS IS USED TO GENERATE THE CALL INFO STRINGS
 *  WHICH ARE CONSUMED BY LEMULATE
 */

void
PL_encode_type_name (Key type, Dyn_str_t *name)
{
  Key base_type;
  int i;
  _BasicType bt;

  PL_dstr_clear(name);

  PL_is_aggr_type (type, &type);

  base_type = PST_GetBaseType (PL_symtab, type);

  bt = PST_GetTypeBasicType(PL_symtab, base_type);

  if (bt & BT_UNSIGNED)
    PL_dstr_strcat (name, "u");

  if (bt & BT_VOID)
    PL_dstr_strcat (name, "void");
  else if (bt & BT_CHAR)
    PL_dstr_strcat (name, "char");
  else if (bt & BT_SHORT)
    PL_dstr_strcat (name, "short");
  else if (bt & BT_LONG)
    PL_dstr_strcat (name, "long");
  else if (bt & BT_LONGLONG)
    PL_dstr_strcat (name, "longlong");
  /* 03/01/04 REK This was treating BT_SIGNED as equivalent to a signed int.
   *              BT_SIGNED is deprecated, so I'm removing it. */
  else if (bt & (BT_INT | BT_ENUM))
    PL_dstr_strcat (name, "int");
  else if (bt & BT_FLOAT)
    PL_dstr_strcat (name, "float");
  else if (bt & BT_DOUBLE)
    PL_dstr_strcat (name, "double");
  else if (bt & BT_LONGDOUBLE)
    PL_dstr_strcat (name, "long double");
  else if (bt & BT_STRUCT)
    {
      StructDcl st = PST_GetStructDclEntry(PL_symtab, 
					   PST_GetTypeType(PL_symtab, base_type));
#if PL_MANGLE_NAMES
      PL_dstr_sprintf (name, "S_%s", 
		       PL_mangle_name(st->name, st->key));
#else
      PL_dstr_sprintf (name, "S_%s", st->name);
#endif
    }
  else if (bt & BT_UNION)
    {
      UnionDcl un = PST_GetUnionDclEntry(PL_symtab, 
					 PST_GetTypeType(PL_symtab, base_type));
#if PL_MANGLE_NAMES
      PL_dstr_sprintf (name, "U_%s", 
		       PL_mangle_name(un->name, un->key));
#else
      PL_dstr_sprintf (name, "U_%s", un->name);
#endif
    }
  else if (bt & BT_VARARG)
    {
      PL_dstr_strcat (name, "vararg");
    }
  else if (!name->str[0])
    {
      P_punt ("encode_type_name: No type information");
    }

  if (!strcmp (name->str, "u"))
     PL_dstr_strcat (name, "int");
     
  if (!PST_IsBaseType(PL_symtab, type))
    {
      PL_dstr_strcat (name, "+");

      for (i = 0;
	   !PST_IsBaseType(PL_symtab,type); 
	   i++, type = PST_GetTypeType(PL_symtab, type))
	{
	  switch (PST_GetTypeBasicType(PL_symtab, type))
	    {
	    case BT_ARRAY:
	      {
		char dim[20];
		if (i == 0)
		  {
		    sprintf (dim, "P");
		  }
		else
		  {
		    Expr expr = PST_GetTypeArraySize(PL_symtab, type);
		    if (expr) 
		      sprintf (dim, "A" ITintmaxformat, 
			       P_IntegralExprValue(expr));
		    else
		      sprintf (dim, "A");
		  }
		assert(strlen(dim) < 20);
		PL_dstr_strcat (name, dim);
	      }
	      break;
	    case BT_POINTER:
	      PL_dstr_strcat (name, "P");
	      break;
	    case BT_FUNC:
	      if (i == 0)
		{
		  PL_dstr_strcat (name, "PF");
		}
	      else
		PL_dstr_strcat (name, "F");
	      break;
	    default:
	      assert(0);
	    }
	}
    }
}


/* 
 * Helper routines
 *
 *****************************************/


static int
PL_BasicTypeSize(_BasicType basic_type)
{
  return PST_GetTypeSize (PL_symtab, 
			  PST_FindBasicType(PL_symtab, basic_type));
}

static int
PL_BasicTypeAlignment(_BasicType basic_type)
{
  return PST_GetTypeAlignment (PL_symtab, 
			       PST_FindBasicType(PL_symtab, basic_type));
}

/* THE FOLLOWING FEW ROUTINES OPERATE
 *   ON MTYPES DIRECTLY. I EXPECT MTYPES
 *   TO DISAPPEAR AS THEY ARE LARGELY USED
 *   AS INTERMEDIATES BETWEEN BASIC_TYPES
 *   AND CTYPES.
 *****************************************/

int 
PL_MType_Size(int type)
{
  return M_type_size (type);
#if 0
  switch (type)
    {
    case M_TYPE_CHAR:
    case M_TYPE_BIT_CHAR:
      return 8 * PL_BasicTypeSize(BT_CHAR);
    case M_TYPE_SHORT:
    case M_TYPE_BIT_SHORT:
      return 8 * PL_BasicTypeSize(BT_SHORT);
    case M_TYPE_INT:
      return 8 * PL_BasicTypeSize(BT_INT);
    case M_TYPE_BIT_LONG:
    case M_TYPE_LONG:
      return 8 * PL_BasicTypeSize(BT_LONG);
    case M_TYPE_BIT_LLONG:
    case M_TYPE_LLONG:
      return 8 * PL_BasicTypeSize(BT_LONGLONG);
    case M_TYPE_FLOAT:
      return 8 * PL_BasicTypeSize(BT_FLOAT);
    case M_TYPE_DOUBLE:
      return 8 * PL_BasicTypeSize(BT_DOUBLE);
    case M_TYPE_POINTER:
      return 8 * PST_GetTypeSize (PL_symtab,
			      PST_FindPointerToType (PL_symtab,
						     PST_FindBasicType(PL_symtab,
								       BT_VOID)));
    case M_TYPE_BLOCK:
    case M_TYPE_UNION:
    case M_TYPE_STRUCT:
      assert(0);
    case M_TYPE_VOID:
      assert(0);
    default:
      assert(0);
    }  

  assert(0);
  return 0;
#endif
}

int 
PL_MType_Align(int type)
{
  return M_type_align(type);
#if 0
  switch (type)
    {
    case M_TYPE_CHAR:
    case M_TYPE_BIT_CHAR:
      return 8 * PL_BasicTypeAlignment(BT_CHAR);
    case M_TYPE_SHORT:
    case M_TYPE_BIT_SHORT:
      return 8 * PL_BasicTypeAlignment(BT_SHORT);
    case M_TYPE_INT:
      return 8 * PL_BasicTypeAlignment(BT_INT);
    case M_TYPE_BIT_LONG:
    case M_TYPE_LONG:
      return 8 * PL_BasicTypeAlignment(BT_LONG);
    case M_TYPE_BIT_LLONG:
    case M_TYPE_LLONG:
      return 8 * PL_BasicTypeAlignment(BT_LONGLONG);
    case M_TYPE_FLOAT:
      return 8 * PL_BasicTypeAlignment(BT_FLOAT);
    case M_TYPE_DOUBLE:
      return 8 * PL_BasicTypeAlignment(BT_DOUBLE);
    case M_TYPE_POINTER:
      return 8 * PST_GetTypeAlignment (PL_symtab,
			      PST_FindPointerToType (PL_symtab,
						     PST_FindBasicType(PL_symtab,
								       BT_VOID)));
    case M_TYPE_BLOCK:
    case M_TYPE_UNION:
    case M_TYPE_STRUCT:
      assert(0);
    case M_TYPE_VOID:
      assert(0);
    default:
      assert(0);
    }  

  assert(0);
  return 0;
#endif
}

int
PL_MType_Eval(int type)
{
  switch (type)
    {
    case M_TYPE_VOID:
      return M_TYPE_VOID;
    case M_TYPE_BIT_LONG:
    case M_TYPE_BIT_CHAR:
    case M_TYPE_CHAR:
    case M_TYPE_SHORT:
    case M_TYPE_INT:
    case M_TYPE_LONG:
    case M_TYPE_LLONG:
    case M_TYPE_POINTER:
    case M_TYPE_BLOCK:
      if (PL_native_int_reg_mtype == M_TYPE_INT)
	return M_TYPE_INT;
      else if (PL_native_int_reg_mtype == M_TYPE_LLONG)
	return M_TYPE_LLONG;
      else
	P_punt ("PL_MType_Eval: Invalid native machine mtype %d\n",
		PL_native_int_reg_mtype);
    case M_TYPE_FLOAT:
      return M_TYPE_FLOAT;
    case M_TYPE_DOUBLE:
      return M_TYPE_DOUBLE;
    case M_TYPE_UNION:
    case M_TYPE_STRUCT:
      return type;
    default:
      assert(0);
    }

  assert(0);
  return 0;
}

int
PL_MType_Convert(int type)
{
  if (type == M_TYPE_BLOCK)
    return M_TYPE_POINTER;
  return type;
}

int 
PL_MType_Compatible(int type1, int type2)
{
  return (PL_MType_Eval(type1) == PL_MType_Eval(type2));
}

void
PL_InitTypeSizes (void)
{
  int i;

  P_CHAR_SIZE = 8 * PL_BasicTypeSize(BT_CHAR);
  P_SHORT_SIZE = 8 * PL_BasicTypeSize(BT_SHORT);
  P_INT_SIZE = 8 * PL_BasicTypeSize(BT_INT);
  P_LONG_SIZE = 8 * PL_BasicTypeSize(BT_LONG);
  P_POINTER_SIZE = 8 * PST_GetTypeSize (PL_symtab,
					PST_FindPointerToType (PL_symtab,
							       PST_FindBasicType(PL_symtab,
										 BT_VOID)));
  P_LONGLONG_SIZE = 8 * PL_BasicTypeSize(BT_LONGLONG);

  assert(P_CHAR_SIZE > 0);
  assert(P_SHORT_SIZE > 0);
  assert(P_INT_SIZE > 0);
  assert(P_LONG_SIZE > 0);
  assert(P_POINTER_SIZE > 0);
  assert(P_LONGLONG_SIZE > 0);

  for (i = 1, P_UNSIGNED_CHAR_MASK = 1; i < P_CHAR_SIZE; i++)
    P_UNSIGNED_CHAR_MASK = (P_UNSIGNED_CHAR_MASK << 1) + 1;
  for (i = 1, P_UNSIGNED_SHORT_MASK = 1; i < P_SHORT_SIZE; i++)
    P_UNSIGNED_SHORT_MASK = (P_UNSIGNED_SHORT_MASK << 1) + 1;
  for (i = 1, P_UNSIGNED_INT_MASK = 1; i < P_INT_SIZE; i++)
    P_UNSIGNED_INT_MASK = (P_UNSIGNED_INT_MASK << 1) + 1;
  for (i = 1, P_UNSIGNED_LONG_MASK = 1; i < P_LONG_SIZE; i++)
    P_UNSIGNED_LONG_MASK = (P_UNSIGNED_LONG_MASK << 1) + 1;
  for (i = 1, P_UNSIGNED_LONGLONG_MASK = 1; i < P_LONGLONG_SIZE; i++)
    P_UNSIGNED_LONGLONG_MASK = (P_UNSIGNED_LONGLONG_MASK << 1) + 1;
  return;
}



/* Query routines from Key to size, alignment
 *
 *****************************************/

int
PL_key_get_size (Key type)
{
    _M_Type mtype;
    PL_pcode2lcode_type(type, &mtype, 0);
    return mtype.size / P_CHAR_SIZE;
//  return PST_GetTypeSize (PL_symtab, 
//			  PST_ReduceTypedefs(PL_symtab, 
//					     type));
}

int
PL_key_get_align (Key type)
{
    _M_Type mtype;
    PL_pcode2lcode_type(type, &mtype, 0);
    return mtype.align / P_CHAR_SIZE;
//  return PST_GetTypeAlignment (PL_symtab,
//			       PST_ReduceTypedefs(PL_symtab, 
//						  type));
}



/* Search routine from structure key, name
 *   to field pointer
 *
 *****************************************/

Field
PL_key_get_field (Key type, char *field_name)
{
  Field field;
  int bt = PST_GetTypeBasicType (PL_symtab, type);

  if (bt & BT_STRUCT)
    {
      StructDcl st = PST_GetStructDclEntry (PL_symtab, 
					    PST_GetTypeType(PL_symtab, type));
      field = P_GetStructDclFields (st);
    }
  else if (bt & BT_UNION)
    {
      UnionDcl un = PST_GetUnionDclEntry (PL_symtab, 
					  PST_GetTypeType(PL_symtab, type));
      field = P_GetUnionDclFields (un);
    }
  else
    {
      assert(0);
    }

  if (!field_name)
    return field;

  for (; field; field = P_GetFieldNext (field))
    {
      if (!strcmp (P_GetFieldName (field), field_name))
	return field;
    }

  return NULL;
}


/* Calculation routine to get field mask and shift
 *   for bit fields
 *
 *****************************************/

int
PL_bit_field_info (Field field, ITuintmax *mask, int *shift)
{
  /*
   * The field offset is a raw offset into the surrounding
   *  structure. The start of the bit field is the offset
   *  in bytes plus some number of remainder bits. 
   *
   *  struct
   *  |----offset*8----> 
   *                    [THE BIT FIELD CONTAINER]
   *                    |- remainder ->
   *                                   |BITFLD|
   *                                    111111  
   *
   *  Any load/store will load the bit field
   *    which had better be larger than the bit field
   *    + remainder bits
   *
   *  The mask and shift should be computed assuming
   *    a load/store of the container
   *
   *
   * BIG_ENDIAN
   *         |<-  BIT FIELD CONTAINER SIZE  ->|
   *                (raw_mask)
   *                  |<-->|
   *         +--------------------------------+
   *         |00000000111111000000000000000000| <--- mask
   *         +--------------------------------+
   *         |<--^-->|     |<---- shift  ---->| 
   *        (remainder
   *         bits)
   *
   *LITTLE_ENDIAN
   *         |<-  BIT FIELD CONTAINER SIZE  ->|
   *                (raw_mask)
   *                  |<-->|
   *         +--------------------------------+
   *         |00000000111111000000000000000000| <--- mask
   *         +--------------------------------+
   *                       |<---- shift  ---->| 
   *                         (remainder bits)
   *        
   *
   *      To read a bit field (for a long bit field),
   *      1. data = fetch long word
   *      2. data = data & bit_mask
   *      3. data >> bit_offset (logical shift)
   *
   *
   * If there are alignment issues this
   * should be handled elsewhere by setting:
   *    type   - how big a container load to do
   *    offset - where to start the container load
   *    bit_offset_remainder - where in container is bitfield
   *
   *
   * NOTE: explicitly_align
   *   For PL_gen_XXX routines, the address already includes
   *        the offset calculation so only the bit offset is necessary
   *        If a particular alignment is necessary then
   *        make sure that the address generated is aligned and then 
   *        set explicitly_align to the number of BYTES for the
   *        alignment (0 turns it off).
   *   For PL_gen_init in pl_data.c, the static initializer is naturally
   *        in container sized chunks, thus the offset must be explicitly
   *        included into the calculations. explicitly_align should always
   *        be set to a container size alignment.
   *
   */

  int container_bit_size, bit_offset, bit_size, bit_shift = 0, container_adj;
  ITuintmax raw_mask;

  container_adj = field->offset - 
    PST_GetFieldContainerOffset (PL_symtab, field);

  container_bit_size = 8 * PL_key_get_size (field->type);  

  bit_offset = 8 * container_adj;

  if (field->is_bit_field)
    {
      bit_offset += field->bit_offset_remainder;
      bit_size = field->bit_size;
    }
  else
    {
      bit_size = container_bit_size;
    }

  assert (bit_size + bit_offset <= container_bit_size);

  raw_mask = (1 << bit_size) - 1;

  if (PL_native_order == M_BIG_ENDIAN)
    bit_shift = (container_bit_size - bit_size - bit_offset);
  else if (PL_native_order == M_LITTLE_ENDIAN)
    bit_shift = bit_offset;

  *mask = raw_mask << bit_shift;
  *shift = bit_shift;

#if 0
  fprintf(stderr,"MASK %X SHIFT %d\n", (int)*mask, (int)*shift);
#endif

  return bit_size;
}


/* Conversion routine from Key to mtype
 *
 *****************************************/

int
PL_key_get_mtype (Key type)
{
  _BasicType bt;

  bt = PST_GetTypeBasicType(PL_symtab, type);

  if (bt & BT_STRUCT)
    return M_TYPE_STRUCT;
  else if (bt & BT_UNION)
    {
      if (PL_is_link_multi (type, &type))
	return PL_key_get_mtype (type);

      return M_TYPE_UNION;
    }
  else if (bt & BT_INT)
    return M_TYPE_INT;    
  else if (bt & BT_ENUM)
    return M_TYPE_INT;
  else if (bt & BT_DOUBLE)
    return M_TYPE_DOUBLE;
  else if (bt & BT_LONGDOUBLE)
    P_punt ("Long double not implemented");
  else if (bt & BT_FLOAT)
    return M_TYPE_FLOAT;
  else if (bt & BT_LONG)
    return M_TYPE_LONG;
  else if (bt & BT_LONGLONG)
    return M_TYPE_LLONG;
  else if (bt & BT_SHORT)
    return M_TYPE_SHORT;
  else if (bt & BT_CHAR)
    return M_TYPE_CHAR;
  else if (bt & BT_VOID)
    return M_TYPE_VOID;
  else if (bt & BT_POINTER)
    return M_TYPE_POINTER;
  else if (bt & BT_VARARG)
    return M_TYPE_POINTER;
  else if (bt & BT_FUNC)
    return M_TYPE_POINTER;
  else if (bt & BT_ARRAY)
    {
      Expr index =  PST_GetTypeArraySize(PL_symtab, type);
      if (!index)
	{
	  return M_TYPE_POINTER;
	}
      else
	{
	  return M_TYPE_BLOCK;
	}
    }

  assert(0);
  return -1;
}


/* Conversion routine from Key to ctype
 *
 *****************************************/

int
PL_key_get_ctype (Key type)
{
  int is_unsigned;
  int ctype = 0;
  int size;
  _BasicType bt;

  bt = PST_GetTypeBasicType(PL_symtab, type);

  is_unsigned = PST_IsUnsignedType(PL_symtab, type);
  size = PL_key_get_size (type);
  
  if (bt & BT_FLOAT)
    ctype = L_CTYPE_FLOAT;
  else if (bt & BT_DOUBLE)
    ctype = L_CTYPE_DOUBLE;
  else if (bt & BT_CHAR)
    ctype = is_unsigned ? L_CTYPE_UCHAR : L_CTYPE_CHAR;
  else if (bt & BT_SHORT)
    ctype = is_unsigned ? L_CTYPE_USHORT : L_CTYPE_SHORT;
  else if (bt & BT_INT)
    ctype = is_unsigned ? L_CTYPE_UINT : L_CTYPE_INT;
  else if (bt & BT_LONG)
    {
      if (size == 64)
	ctype = is_unsigned ? L_CTYPE_ULLONG : L_CTYPE_LLONG;
      else
	ctype = is_unsigned ? L_CTYPE_UINT : L_CTYPE_INT;
    }
  else if (bt & BT_LONGLONG)
    ctype = is_unsigned ? L_CTYPE_ULLONG : L_CTYPE_LLONG;
  else if (bt & (BT_POINTER|BT_FUNC|BT_ARRAY))
    {
      if (P_POINTER_SIZE == 64)
	ctype = L_CTYPE_LLONG;
      else if (P_POINTER_SIZE == 32)
	ctype = L_CTYPE_INT;
      else
	P_punt ("PL_key_get_ctype: unusual pointer size");
    }
  else
    assert(0);

  return ctype;
}


/* Conversion routine from Key to register ctype
 *
 *****************************************/

int
PL_key_get_regctype (Key type)
{
  int is_unsigned;
  int ctype;
  _BasicType bt;

  bt = PST_GetTypeBasicType(PL_symtab, type);
  is_unsigned = PST_IsUnsignedType(PL_symtab, type);
  
  if (L_propagate_sign_size_ctype_info)
    {
      if (bt & BT_FLOAT)
	ctype = L_CTYPE_FLOAT;
      else if (bt & BT_DOUBLE)
	ctype = L_CTYPE_DOUBLE;
      else if (bt & BT_CHAR)
	ctype = is_unsigned ? L_CTYPE_UCHAR : L_CTYPE_CHAR;
      else if (bt & BT_SHORT)
	ctype = is_unsigned ? L_CTYPE_USHORT : L_CTYPE_SHORT;
      else if (bt & BT_INT)
	ctype = is_unsigned ? L_CTYPE_UINT : L_CTYPE_INT;
      else if (bt & BT_LONG)
	ctype = is_unsigned ? L_CTYPE_ULONG : L_CTYPE_LONG;
      else if (bt & BT_LONGLONG)
	ctype = is_unsigned ? L_CTYPE_ULLONG : L_CTYPE_LLONG;
      else if (bt & (BT_POINTER|BT_FUNC|BT_ARRAY))
	ctype = L_CTYPE_POINTER;
      else
        ctype = PL_native_int_reg_ctype;
      //assert(0);
    }
  else
    {
      if (bt & BT_FLOAT)
	{
#ifdef PL_GEN_FLOAT_OPERANDS	  
	  ctype = L_CTYPE_FLOAT;
#else
	  ctype = L_CTYPE_DOUBLE;
#endif
	}
      else if (bt & BT_DOUBLE)
	ctype = L_CTYPE_DOUBLE;
      else
	ctype = PL_native_int_reg_ctype;
    }

  return ctype;
}


/* Conversion routine from Key to parameter ctype
 *
 *****************************************/

int
PL_key_get_parmctype (Key type)
{
  int is_unsigned;
  int ctype;
  _BasicType bt;

  bt = PST_GetTypeBasicType(PL_symtab, type);
  is_unsigned = PST_IsUnsignedType(PL_symtab, type);

  if (bt & BT_FLOAT)
    ctype = L_CTYPE_FLOAT;
  else if (bt & BT_DOUBLE)
    ctype = L_CTYPE_DOUBLE;
  else if (bt & BT_CHAR)
    ctype = is_unsigned ? L_CTYPE_UCHAR : L_CTYPE_CHAR;
  else if (bt & BT_SHORT)
    ctype = is_unsigned ? L_CTYPE_USHORT : L_CTYPE_SHORT;
  else if (bt & BT_INT)
    ctype = is_unsigned ? L_CTYPE_UINT : L_CTYPE_INT;
  else if (bt & BT_LONG)
    ctype = is_unsigned ? L_CTYPE_ULONG : L_CTYPE_LONG;
  else if (bt & BT_LONGLONG)
    ctype = is_unsigned ? L_CTYPE_ULLONG : L_CTYPE_LLONG;
  else if (bt & (BT_POINTER|BT_FUNC|BT_ARRAY))
    ctype = L_CTYPE_POINTER;
  else if (bt & (BT_STRUCT|BT_UNION))
    ctype = PL_native_int_reg_ctype;
  else
    assert(0);

  return ctype;
}


/*****************************************
 * 
 * ORIGINAL LCODE GENERATION ROUTINES
 *
 *****************************************/


void
PL_gen_lcode_struct (L_Datalist * list, StructDcl st)
{
  if (PL_emit_source_info || PL_emit_data_type_info)
    PL_gen_struct (list, st);	
}


void
PL_gen_lcode_union (L_Datalist * list, UnionDcl un)
{
  if (PL_emit_source_info || PL_emit_data_type_info)
    PL_gen_union (list, un);
}


void
PL_gen_lcode_var (L_Datalist * list, VarDcl var)
{
  PL_gen_var (list, var);
}


void
PL_gen_lcode_func (FuncDcl fn)
{
  VarDcl var;
  Pragma prag;
  Dyn_str_t *pragmastr = NULL;
  Dyn_str_t *temp_str = NULL;

  pragmastr = PL_dstr_new(256);
  temp_str = PL_dstr_new(256);
  
  /* EMN 9/2002 - Extract shadow statements and create
   *    shadow list on function for use during lcode gen
   */
  PL_extract_shadow_info (fn);

  /* generate \"call_info\${return type}{%{param type}}*\" */
  PL_dstr_sprintf (pragmastr, "call_info\\$");
  PL_encode_type_name (PST_GetTypeType(PL_symtab, fn->type), temp_str);
  PL_dstr_strcat (pragmastr, temp_str->str);	/* add {return type} */

  List_start(fn->param);
  while ((var = List_next(fn->param)))
    {
      /* add {%{param type}}* */
      PL_dstr_strcat (pragmastr, "%");
      PL_encode_type_name (var->type, temp_str);
      PL_dstr_strcat (pragmastr, temp_str->str);
    }
  fn->pragma = P_AppendPragmaNext (fn->pragma, 
				   P_NewPragmaWithSpecExpr (strdup (pragmastr->str), NULL));

  /* generate function and file name pragma */
  if (fn->filename)
    {
      if (fn->name == NULL)
	P_punt ("PL_gen_lcode_func: missing function name for pragma");

      PL_dstr_sprintf (pragmastr, "FUNC\\$%s\\%%%d", fn->name, fn->lineno);
      fn->pragma = P_AppendPragmaNext (fn->pragma, 
				       P_NewPragmaWithSpecExpr (strdup (pragmastr->str), NULL));

      PL_dstr_sprintf (pragmastr, "FILE\\$%s", fn->filename);
      fn->pragma = P_AppendPragmaNext (fn->pragma, 
				       P_NewPragmaWithSpecExpr (strdup (pragmastr->str), NULL));
    }

  /* JWS: mark functions with acc_specs as having them, so that the
   * Lcode disambiguator will expect to see them.  Unreachable functions
   * don't appear to have access specifiers at present, so don't mark
   * those as having them.  This results in conservative optimization
   * in the back end.
   */
  if (PL_gen_acc_specs && P_FindPragma (fn->pragma, "CALLED"))
    {
      fn->pragma = P_AppendPragmaNext (fn->pragma, 
				       P_NewPragmaWithSpecExpr ("ACC_SPECS", 
								NULL));
      if (PL_annotate_omega)
	fn->pragma = P_AppendPragmaNext (fn->pragma, 
					 P_NewPragmaWithSpecExpr ("ACC_OMEGA", 
								  NULL));
    }

  /* generate \"args_pcode_promoted\" */
  if (!P_FindPragma (fn->pragma, "args_pcode_promoted"))
    fn->pragma = P_AppendPragmaNext (fn->pragma, 
				     P_NewPragmaWithSpecExpr ("args_pcode_promoted", NULL));

  /* generate brand names: \"impact_info{\\${string}}*\" */
  if ((prag = P_FindPragmaPrefix (fn->pragma, "IMPACT_INFO")))
    {
      PL_dstr_sprintf (pragmastr, "impact_info\\$%s", prag->expr->value.string);
      fn->pragma = P_AppendPragmaNext (fn->pragma, 
				       P_NewPragmaWithSpecExpr(strdup (pragmastr->str), NULL));
    }

  /* generate \"host_info{\\${string}}*\" */
  if ((prag = P_FindPragmaPrefix (fn->pragma, "HOST_INFO")))
    {
      PL_dstr_sprintf (pragmastr, "host_info\\$%s", prag->expr->value.string);
      fn->pragma = P_AppendPragmaNext (fn->pragma, 
				       P_NewPragmaWithSpecExpr (strdup (pragmastr->str), NULL));
    }

  /* generate \"preprocess_info{\\${string}}*\" */
  if ((prag = P_FindPragmaPrefix (fn->pragma, "PREPROCESS_INFO")))
    {
      PL_dstr_sprintf (pragmastr, "preprocess_info\\$%s",
		       prag->expr->value.string);
      fn->pragma = P_AppendPragmaNext (fn->pragma, 
				       P_NewPragmaWithSpecExpr (strdup (pragmastr->str), NULL));
    }

  /*
   *  Generate code.
   */

  PL_gen_func (fn);

  PL_dstr_free(pragmastr);
  PL_dstr_free(temp_str);

  if (PL_gen_acc_specs)
    {
      PL_draw_sync_arcs (L_fn);
      
      /* DML - remove extra invalid arcs added for
	 operators +=, -=, etc */
      L_adjust_invalid_sync_arcs_in_func (L_fn);
    }

  if (PL_initialize_function_live_ins)
    {
      /* Ensure that live-in registers are initialized to
       * prevent inadvertent NaT consumption.
       */
      L_do_flow_analysis (L_fn, LIVE_VARIABLE_CB);
      L_initialize_function_live_ins (L_fn);
    }

  return;
}


static int
IsHFATypeR (Type type, int *prec, int level)
{
  int bt = PST_GetTypeBasicType (PL_symtab, type);

  if (PST_IsBaseType (PL_symtab, type))
    {
      Field field;

      if (bt & BT_STRUCT)
	{
	  StructDcl st = PST_GetStructDclEntry (PL_symtab, 
						PST_GetTypeType(PL_symtab, 
								type));
	  
	  for (field = P_GetStructDclFields (st); field; 
	       field = P_GetFieldNext (field))
	    {
	      if (!IsHFATypeR (field->type, prec, level + 1))
		return 0;
	    }

	  return 1;
	}
      else if (bt & BT_UNION)
	{
	  UnionDcl un = PST_GetUnionDclEntry (PL_symtab, 
					      PST_GetTypeType(PL_symtab, 
							      type));
	  for (field = P_GetUnionDclFields (un); field; 
	       field = P_GetFieldNext (field))
	    {
	      if (!IsHFATypeR (field->type, prec, level + 1))
		return 0;
	    }

	  return 1;
	}
      else if (level > 0 && (bt & (BT_FLOAT | BT_DOUBLE)))
	{
	  if (*prec == -1)
	    {
	      *prec = bt & (BT_FLOAT|BT_DOUBLE);
	      return 1;
	    }
	  else if (*prec != bt)
	    {
	      *prec = 0;
	      return 0;
	    }
	}
    }
  else if (level > 0 && (PST_IsArrayType (PL_symtab, type)))
    {
      return IsHFATypeR (PST_GetTypeType (PL_symtab, type), prec, level + 1);
    }

  return 0;
}


/* Returns TY_FLOAT or TY_DOUBLE if type is an HFA of such type. */

int
PL_is_HFA_type (Type type)
{
  int prec;

  if (IsHFATypeR (type, &prec, 0))
    return prec;
  else
    return 0;
}


/*
 *  This is soon to be removed once fnvar_layout and
 *    lvar_layout no longer call it
 */

void
PL_pcode2lcode_type (Key type, M_Type mtype, int is_param)
{
  int bt;

  if (!mtype)
    P_punt ("PL_pcode2lcode_type: nil argument");

  mtype->flags = 0;
  mtype->type = -1;
  mtype->unsign = PST_IsUnsignedType(PL_symtab, type);
//  mtype->align = 8*PL_key_get_align(type);
//  mtype->size = 8*PL_key_get_size (type);
//  mtype->nbits = 8*PL_key_get_size (type);

  bt = PST_GetTypeBasicType(PL_symtab, type);

  if (PST_IsBaseType(PL_symtab, type))
    {
      if (bt & BT_STRUCT)
	{
	  // mtype->type = M_TYPE_STRUCT;
          int align = 8*PST_GetTypeAlignment (PL_symtab,
                                              PST_ReduceTypedefs(PL_symtab, type));
          int size = 8*PST_GetTypeSize (PL_symtab,
                                        PST_ReduceTypedefs(PL_symtab, type));
          M_struct (mtype, align, size);
	}
      else if (bt & BT_UNION)
	{
	  if (PL_is_link_multi (type, &type))
	    return PL_pcode2lcode_type (type, mtype, is_param);

	  // mtype->type = M_TYPE_UNION;
          int align = 8*PST_GetTypeAlignment (PL_symtab,
                                              PST_ReduceTypedefs(PL_symtab, type));
          int size = 8*PST_GetTypeSize (PL_symtab,
                                        PST_ReduceTypedefs(PL_symtab, type));
          M_union (mtype, align, size);
	}
      else if (bt & BT_ENUM)
	{
	  // mtype->type = M_TYPE_INT;
          M_int (mtype, 0);
	}
      else if (bt & BT_DOUBLE)
	{
	  // mtype->type = M_TYPE_DOUBLE;
          M_double (mtype, mtype->unsign);
	}
      else if (bt & BT_LONGDOUBLE)
	{
#if 1
	  P_punt ("Long double not implemented");
#else
	  // mtype->type = M_TYPE_LDOUBLE;
          M_longdouble (mtype, mtype->unsign);
#endif
	}
      else if (bt & BT_FLOAT)
	{
	  // mtype->type = M_TYPE_FLOAT;
          M_float (mtype, mtype->unsign);
	}
      else if (bt & BT_LONG)
	{
	  if (P_LONG_SIZE == 64)
            M_llong (mtype, mtype->unsign);
	  else if (P_LONG_SIZE == 32)
            M_long (mtype, mtype->unsign);
          else
	    P_punt ("PL_gen_var: illegal long size %d", P_LONG_SIZE);
          /*
	  if (P_LONG_SIZE == 64)
	    mtype->type = M_TYPE_LLONG;
	  else if (P_LONG_SIZE == 32)
	    mtype->type = M_TYPE_INT;
	  else
	    P_punt ("PL_gen_var: illegal long size %d", P_LONG_SIZE);
          */
	}
      else if (bt & BT_LONGLONG)
	{
	  // mtype->type = M_TYPE_LLONG;
          M_llong (mtype, mtype->unsign);
	}
      else if (bt & BT_SHORT)
	{
	  // mtype->type = M_TYPE_SHORT;
          M_short (mtype, mtype->unsign);
	}
      else if (bt & BT_CHAR)
	{
	  // mtype->type = M_TYPE_CHAR;
          M_char (mtype, mtype->unsign);
	}
      else if (bt & BT_VOID)
	{
	  // mtype->type = M_TYPE_VOID;
          M_void (mtype);
	}
      else
	{
	  // mtype->type = M_TYPE_INT;
          M_int (mtype, mtype->unsign);
	}
    }
  else
    {
      /* DMG - arrays references are now handled as pointers; however,
         they will have an index field */
      if (PST_IsPointerType(PL_symtab, type))
	{
	  // mtype->type = M_TYPE_POINTER;
          M_pointer (mtype);
	}
      else if (PST_IsFunctionType(PL_symtab, type))
	{
          M_pointer (mtype);
          /*
	  mtype->type = M_TYPE_POINTER;
	  mtype->align = PL_MType_Align(M_TYPE_POINTER);
	  mtype->size = PL_MType_Size(M_TYPE_POINTER);
	  mtype->nbits = PL_MType_Size(M_TYPE_POINTER);
          */
	}
      else if (PST_IsArrayType(PL_symtab, type))
	{
	  Expr index = PST_GetTypeArraySize(PL_symtab, type);
	  if (!index)
	    {
              M_pointer (mtype);
              /*
	      mtype->type = M_TYPE_POINTER;
	      mtype->align = PL_MType_Align(M_TYPE_POINTER);
	      mtype->size = PL_MType_Size(M_TYPE_POINTER);
	      mtype->nbits = PL_MType_Size(M_TYPE_POINTER);
              */
	    }
	  else
	    {
	      _M_Type emtype;
	      int dim, align, size;
	      Key el_type;

	      el_type = PST_GetTypeType (PL_symtab, type);
	      if (is_param)
		{
                  M_pointer (mtype);
                  /*
		  mtype->type = M_TYPE_POINTER;
		  mtype->align = PL_MType_Align(M_TYPE_POINTER);
		  mtype->size = PL_MType_Size(M_TYPE_POINTER);
		  mtype->nbits = PL_MType_Size(M_TYPE_POINTER);
                  */
		}
	      else
		{
		  /* BCC - use temporary IsIntegralExprForArray - 5/24/95 */
		  if (!P_IsIntegralExpr (index))
		    P_punt ("array dimension must be a constant");
		  
		  /* BCC - use temporary IntegralExprValueForArray - 5/24/95 */
		  dim = P_IntegralExprValue(index);
		  
		  PL_pcode2lcode_type (el_type, &emtype, is_param);
		  align = M_array_align (&emtype);
		  size = M_array_size (&emtype, dim);

                  M_block (mtype, align, size);
                  /*
		  mtype->type = M_TYPE_BLOCK;
		  mtype->align = align;
		  mtype->size = size;
		  mtype->nbits = size;
                  */
		}
	    }
	}
      else
	{
	  P_punt ("unknown dcltr type");
	}
    }

  switch (PL_is_HFA_type (type))
    {
    case BT_DOUBLE:
      mtype->flags |= M_TYFL_HFA_DBL;
      break;
    case BT_FLOAT:
      mtype->flags |= M_TYFL_HFA_SGL;
      break;
    default:
      break;
    }

  return;
}


int
PL_is_func_var (Key var_key)
{
  FuncDcl fdcl;
  VarDcl vdcl;

  if ((vdcl = PST_GetVarDclEntry(PL_symtab, var_key)))
    {
      if (PST_IsFunctionType(PL_symtab, vdcl->type))
	{
	  /* Can this happen? */
	  assert(0);
	  return 1;
	}
      return 0;
    }

  if ((fdcl = PST_GetFuncDclEntry(PL_symtab, var_key)))
    {
      assert(PST_IsFunctionType(PL_symtab, fdcl->type));
      return 1;
    }

  assert(0);
  return 0;
}


char *
PL_M_fn_label_name (char *label, int is_func)
{
  if (is_func)
    {
      Dyn_str_t *fn_label;
      char *retstr;

      fn_label = PL_dstr_new(256);
      PL_dstr_sprintf (fn_label, "$fn_%s", label);
      
      retstr = C_findstr(fn_label->str);
      PL_dstr_free(fn_label);

      return retstr;
    }
  else
    return (label);
}


void
PL_M_pointer(M_Type mtype)
{
  mtype->type = M_TYPE_POINTER;
  mtype->align = PL_MType_Align(M_TYPE_POINTER);
  mtype->unsign = 0;
  mtype->size = PL_MType_Size(M_TYPE_POINTER);
  mtype->nbits = PL_MType_Size(M_TYPE_POINTER);
}


void
PL_M_int(M_Type mtype, int unsign)
{
  mtype->type = M_TYPE_INT;
  mtype->align = PL_MType_Align(M_TYPE_INT);
  mtype->unsign = unsign;
  mtype->size = PL_MType_Size(M_TYPE_INT);
  mtype->nbits = PL_MType_Size(M_TYPE_INT);
}


int
PL_M_no_short_int()
{
  return 0;
}


int
PL_is_link_multi (Key type, Key *pstype)
{
  TypeDcl td;
  UnionDcl ud;
  int bt;

  type = PST_ReduceTypedefs (PL_symtab, type);
  td = PST_GetTypeDclEntry (PL_symtab, type);
  bt = P_GetTypeDclBasicType (td);

  if (!(bt & BT_UNION))
    return 0;

  ud = PST_GetUnionDclEntry (PL_symtab, P_GetTypeDclType (td));

  if (!(P_GetUnionDclQualifier (ud) & SQ_LINKMULTI))
    return 0;

  P_warn ("Found Plink multi type union %s", ud->name);

  if (pstype)
    *pstype = P_GetFieldType (ud->fields);

  return 1;
}


int
PL_is_aggr_type (Key type, Key *pstype)
{
  return PST_IsStructureType (PL_symtab, type) &&
    !PL_is_link_multi (type, pstype);
}
