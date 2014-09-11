/* IMPACT Public Release (www.crhc.uiuc.edu/IMPACT)            Version 2.00  */
/* IMPACT Trimaran Release (www.trimaran.org)                  Feb. 22, 1999 */
/*****************************************************************************\
 * LICENSE AGREEMENT NOTICE
 * 
 * IT IS A BREACH OF THIS LICENSE AGREEMENT TO REMOVE THIS NOTICE FROM
 * THE FILE OR SOFTWARE, OR ANY MODIFIED VERSIONS OF THIS FILE OR
 * SOFTWARE OR DERIVATIVE WORKS.
 * 
 * ------------------------------
 * 
 * Copyright Notices/Identification of Licensor(s) of 
 * Original Software in the File
 * 
 * Copyright 1990-1999 The Board of Trustees of the University of Illinois
 * For commercial license rights, contact: Research and Technology
 * Management Office, University of Illinois at Urbana-Champaign; 
 * FAX: 217-244-3716, or email: rtmo@uiuc.edu
 * 
 * All rights reserved by the foregoing, respectively.
 * 
 * ------------------------------
 * 	
 * Copyright Notices/Identification of Subsequent Licensor(s)/Contributors 
 * of Derivative Works
 * 
 * Copyright  <Year> <Owner>
 * <Optional:  For commercial license rights, contact:_____________________>
 * 
 * 
 * All rights reserved by the foregoing, respectively.
 * 
 * ------------------------------
 * 
 * The code contained in this file, including both binary and source 
 * [if released by the owner(s)] (hereafter, Software) is subject to
 * copyright by the respective Licensor(s) and ownership remains with
 * such Licensor(s).  The Licensor(s) of the original Software remain
 * free to license their respective proprietary Software for other
 * purposes that are independent and separate from this file, without
 * obligation to any party.
 * 
 * Licensor(s) grant(s) you (hereafter, Licensee) a license to use the
 * Software for academic, research and internal business purposes only,
 * without a fee.  "Internal business purposes" means that Licensee may
 * install, use and execute the Software for the purpose of designing and
 * evaluating products.  Licensee may submit proposals for research
 * support, and receive funding from private and Government sponsors for
 * continued development, support and maintenance of the Software for the
 * purposes permitted herein.
 * 
 * Licensee may also disclose results obtained by executing the Software,
 * as well as algorithms embodied therein.  Licensee may redistribute the
 * Software to third parties provided that the copyright notices and this
 * License Agreement Notice statement are reproduced on all copies and
 * that no charge is associated with such copies.  No patent or other
 * intellectual property license is granted or implied by this Agreement,
 * and this Agreement does not license any acts except those expressly
 * recited.
 * 
 * Licensee may modify the Software to make derivative works (as defined
 * in Section 101 of Title 17, U.S. Code) (hereafter, Derivative Works),
 * as necessary for its own academic, research and internal business
 * purposes.  Title to copyrights and other proprietary rights in
 * Derivative Works created by Licensee shall be owned by Licensee
 * subject, however, to the underlying ownership interest(s) of the
 * Licensor(s) in the copyrights and other proprietary rights in the
 * original Software.  All the same rights and licenses granted herein
 * and all other terms and conditions contained in this Agreement
 * pertaining to the Software shall continue to apply to any parts of the
 * Software included in Derivative Works.  Licensee's Derivative Work
 * should clearly notify users that it is a modified version and not the
 * original Software distributed by the Licensor(s).
 * 
 * If Licensee wants to make its Derivative Works available to other
 * parties, such distribution will be governed by the terms and
 * conditions of this License Agreement.  Licensee shall not modify this
 * License Agreement, except that Licensee shall clearly identify the
 * contribution of its Derivative Work to this file by adding an
 * additional copyright notice to the other copyright notices listed
 * above, to be added below the line "Copyright Notices/Identification of
 * Subsequent Licensor(s)/Contributors of Derivative Works."  A party who
 * is not an owner of such Derivative Work within the meaning of
 * U.S. Copyright Law (i.e., the original author, or the employer of the
 * author if "work of hire") shall not modify this License Agreement or
 * add such party's name to the copyright notices above.
 * 
 * Each party who contributes Software or makes a Derivative Work to this
 * file (hereafter, Contributed Code) represents to each Licensor and to
 * other Licensees for its own Contributed Code that:
 * 
 * (a) Such Contributed Code does not violate (or cause the Software to
 * violate) the laws of the United States, including the export control
 * laws of the United States, or the laws of any other jurisdiction.
 * 
 * (b) The contributing party has all legal right and authority to make
 * such Contributed Code available and to grant the rights and licenses
 * contained in this License Agreement without violation or conflict with
 * any law.
 * 
 * (c) To the best of the contributing party's knowledge and belief,
 * the Contributed Code does not infringe upon any proprietary rights or
 * intellectual property rights of any third party.
 * 
 * LICENSOR(S) MAKE(S) NO REPRESENTATIONS ABOUT THE SUITABILITY OF THE
 * SOFTWARE OR DERIVATIVE WORKS FOR ANY PURPOSE.  IT IS PROVIDED "AS IS"
 * WITHOUT EXPRESS OR IMPLIED WARRANTY, INCLUDING BUT NOT LIMITED TO THE
 * MERCHANTABILITY, USE OR FITNESS FOR ANY PARTICULAR PURPOSE AND ANY
 * WARRANTY AGAINST INFRINGEMENT OF ANY INTELLECTUAL PROPERTY RIGHTS.
 * LICENSOR(S) SHALL NOT BE LIABLE FOR ANY DAMAGES SUFFERED BY THE USERS
 * OF THE SOFTWARE OR DERIVATIVE WORKS.
 * 
 * Any Licensee wishing to make commercial use of the Software or
 * Derivative Works should contact each and every Licensor to negotiate
 * an appropriate license for such commercial use, and written permission
 * of all Licensors will be required for such a commercial license.
 * Commercial use includes (1) integration of all or part of the source
 * code into a product for sale by or on behalf of Licensee to third
 * parties, or (2) distribution of the Software or Derivative Works to
 * third parties that need it to utilize a commercial product sold or
 * licensed by or on behalf of Licensee.
 * 
 * By using or copying this Contributed Code, Licensee agrees to abide by
 * the copyright law and all other applicable laws of the U.S., and the
 * terms of this License Agreement.  Any individual Licensor shall have
 * the right to terminate this license immediately by written notice upon
 * Licensee's breach of, or non-compliance with, any of its terms.
 * Licensee may be held legally responsible for any copyright
 * infringement that is caused or encouraged by Licensee's failure to
 * abide by the terms of this License Agreement.  
\*****************************************************************************/
/*=========================================================================== 
 *	File :	m_arm.c 
 *	Desc :	Machine dependent specification.  
 *	Date :	January 2004
 *	Auth :  
 *
 *    Modified : Modified from code by Richard Hank and Wen-mei Hwu, who 
 *               modified it from m_spec.c code by Pohua Paul Chang.
 *
 *==========================================================================*/
#include <stdio.h>
#ifdef M_ARM_FOR_HCODE
#include <Hcode/h_ccode.h>
#endif
#include <Lcode/l_main.h>
#include "m_arm.h"

/*--------------------------------------------------------------------------*/
#define M_ARM_SIZE_VOID	        0
#define M_ARM_SIZE_BIT		1
#define M_ARM_SIZE_CHAR	        8
#define M_ARM_SIZE_SHORT	16
#define M_ARM_SIZE_INT	        32	
#define M_ARM_SIZE_LONG	        32
#define M_ARM_SIZE_LLONG        64
#define M_ARM_SIZE_FLOAT	32
#define M_ARM_SIZE_DOUBLE	64
#define M_ARM_SIZE_POINTER	32
#define M_ARM_SIZE_UNION	-1
#define M_ARM_SIZE_STRUCT	-1
#define M_ARM_SIZE_BLOCK	-1
#define M_ARM_SIZE_MAX	 	64	

#define M_ARM_ALIGN_VOID	-1
#define M_ARM_ALIGN_BIT	        1
#define M_ARM_ALIGN_CHAR	8
#define M_ARM_ALIGN_SHORT	16
#define M_ARM_ALIGN_INT	        32
#define M_ARM_ALIGN_LONG	32
#define M_ARM_ALIGN_LLONG	64
#define M_ARM_ALIGN_FLOAT	32 
#define M_ARM_ALIGN_DOUBLE	64
#define M_ARM_ALIGN_POINTER	32
#define M_ARM_ALIGN_UNION	-1	/* depends on the field */
#define M_ARM_ALIGN_STRUCT	-1
#define M_ARM_ALIGN_BLOCK	-1
#define M_ARM_ALIGN_MAX	        64

/*--------------------------------------------------------------------------*/
#define M_ARM_MAX_FNVAR_REG 	4
#define M_ARM_SMALL_STRUCT_MAX 	32
#define MIN_PARAM_SIZE 	  	(M_ARM_MAX_FNVAR_REG *  M_ARM_SIZE_INT)	

/* incoming and outgoing parameters */
#define M_ARM_INT_BASE		0
#define M_ARM_FLT_BASE		5
#define M_ARM_DBL_BASE		9

#define M_ARM_RET_F              15       /* return in f0 */

#define M_ARM_RET_I32            15       /* return in R0 */
#define M_ARM_RET_I64            15       /* return in R0 */
#define M_ARM_RET_ST             15       /* return in R0 */


/*--------------------------------------------------------------------------*/
/*
 *	The following functions are for documentation only.
 *	The function overhead is too great for actual usage.
 */
void M_arm_void(M_Type type)
{
    type->type = M_TYPE_VOID;
    type->unsign = 1;
    type->align = M_ARM_ALIGN_VOID;
    type->size = M_ARM_SIZE_VOID;
    type->nbits = 0;
}

void M_arm_bit_long(M_Type type, int n)
{
    type->type = M_TYPE_BIT_LONG;
    type->unsign = 1;
    type->align = M_ARM_ALIGN_BIT;
    type->size = n * M_ARM_SIZE_BIT;
    type->nbits = n * M_ARM_SIZE_BIT;
    M_assert ((n<=16), "M_bit_long: do not allow bit field of more than 16 bits");
}

void M_arm_bit_int (M_Type type, int n)
{
    type->type = M_TYPE_BIT_INT;
    type->unsign = 1;
    type->align = M_ARM_ALIGN_BIT;
    type->size = n * M_ARM_SIZE_BIT;
    type->nbits = n * M_ARM_SIZE_BIT;
    M_assert ((n <= 16), "M_bit_int: do not allow bit field of more than 16 bits");
}

void M_arm_bit_short(M_Type type, int n)
{
    type->type = M_TYPE_BIT_SHORT;
    type->unsign = 1;
    type->align = M_ARM_ALIGN_BIT;
    type->size = n * M_ARM_SIZE_BIT;
    type->nbits = n * M_ARM_SIZE_BIT;
    M_assert ((n<=16), "M_bit_long: do not allow bit field of more than 16 bits");
}

void M_arm_bit_char(M_Type type, int n)
{
    type->type = M_TYPE_BIT_CHAR;
    type->unsign = 1;
    type->align = M_ARM_ALIGN_BIT;
    type->size = n * M_ARM_SIZE_BIT;
    type->nbits = n * M_ARM_SIZE_BIT;
    M_assert ((n<=8), "M_bit_char: do not allow bit field of more than 8 bits");
}

void M_arm_float(M_Type type, int unsign)
{
    type->type = M_TYPE_FLOAT;
    type->unsign = unsign;
    type->align = M_ARM_ALIGN_FLOAT;
    type->size = M_ARM_SIZE_FLOAT;
    type->nbits = M_ARM_SIZE_FLOAT;
}

void M_arm_double(M_Type type, int unsign)
{
    type->type = M_TYPE_DOUBLE;
    type->unsign = unsign;
    type->align = M_ARM_ALIGN_DOUBLE;
    type->size = M_ARM_SIZE_DOUBLE;
    type->nbits = M_ARM_SIZE_DOUBLE;
}

void M_arm_pointer(M_Type type)
{
    type->type = M_TYPE_POINTER;
    type->unsign = 1;
    type->align = M_ARM_ALIGN_POINTER;
    type->size = M_ARM_SIZE_POINTER;
    type->nbits = M_ARM_SIZE_POINTER;
}

/*--------------------------------------------------------------------------*/
int M_arm_eval_type(M_Type type, M_Type ntype)
{
  switch (type->type) {
  case M_TYPE_VOID:
    M_void(ntype);
    return(-1);			/* can not be evaluated */
  case M_TYPE_BIT_CHAR:
  case M_TYPE_CHAR:
  case M_TYPE_SHORT:
  case M_TYPE_BLOCK:
  case M_TYPE_INT:
  case M_TYPE_BIT_LONG:
  case M_TYPE_LONG:
  case M_TYPE_POINTER:
    /* the starting address of array is used */
    M_int(ntype, type->unsign);
    return(M_TYPE_INT);
  case M_TYPE_LLONG:
    M_arm_llong (ntype, type->unsign);
    break;
  case M_TYPE_FLOAT:
    M_float(ntype, type->unsign);
    return(M_TYPE_FLOAT);
  case M_TYPE_DOUBLE:
    M_double(ntype, type->unsign);
    return(M_TYPE_DOUBLE);
  case M_TYPE_UNION:
  case M_TYPE_STRUCT:
    *ntype = *type;
    return type->type;
  default:
    return(-1);
  }
}

int M_arm_eval_type2(M_Type type, M_Type ntype)
{
  switch (type->type) {
  case M_TYPE_VOID:
    M_void(ntype);
    return(-1);			/* can not be evaluated */
  case M_TYPE_BIT_CHAR:
  case M_TYPE_CHAR:
    M_char(ntype, type->unsign);
    return(M_TYPE_CHAR);
  case M_TYPE_SHORT:
    M_short(ntype, type->unsign);
    return(M_TYPE_SHORT);
  case M_TYPE_BLOCK: 
  case M_TYPE_INT:
    /* the starting address of array is used */
    M_int(ntype, type->unsign);
    return(M_TYPE_INT);
  case M_TYPE_BIT_LONG:
  case M_TYPE_LONG:
    M_long(ntype, type->unsign);
    return(M_TYPE_LONG);
  case M_TYPE_BIT_LLONG:
  case M_TYPE_LLONG:
    M_arm_llong (ntype, type->unsign);
    return (M_TYPE_INT);
  case M_TYPE_POINTER:
    M_pointer(ntype);
    return(M_TYPE_POINTER);
  case M_TYPE_FLOAT:
    M_float(ntype, type->unsign);
    return(M_TYPE_FLOAT);
  case M_TYPE_DOUBLE:
    M_double(ntype, type->unsign);
    return(M_TYPE_DOUBLE);
  case M_TYPE_UNION:
  case M_TYPE_STRUCT:
    *ntype = *type;
    return type->type;
  default:
    return(-1);
  }
}

int M_arm_call_type(M_Type type, M_Type ntype)
{
  switch (type->type) {
  case M_TYPE_VOID:
    M_void(ntype);
    return(-1);			/* can not be evaluated */
  case M_TYPE_BIT_CHAR:
  case M_TYPE_CHAR:
  case M_TYPE_SHORT:
  case M_TYPE_BIT_LONG:
  case M_TYPE_INT:
  case M_TYPE_LONG:
  case M_TYPE_POINTER:
  case M_TYPE_BLOCK:
    /* the starting address of array is used */
    M_int(ntype, type->unsign);
    return(M_TYPE_INT);
  case M_TYPE_LLONG:
  case M_TYPE_BIT_LLONG:
    M_arm_llong (ntype, type->unsign);
    return (M_TYPE_LLONG);
  case M_TYPE_FLOAT:
    M_float(ntype, type->unsign);
    return(M_TYPE_FLOAT);
  case M_TYPE_DOUBLE:
    M_double(ntype, type->unsign);
    return M_TYPE_DOUBLE;
  case M_TYPE_UNION:
  case M_TYPE_STRUCT:
    *ntype = *type;
    return(type->type);
  default:
    return(-1);
  }
}

int M_arm_call_type2(M_Type type, M_Type ntype)
{
  switch (type->type) {
  case M_TYPE_VOID:
    M_void(ntype);
    return(-1);			/* can not be evaluated */
  case M_TYPE_BIT_CHAR:
  case M_TYPE_CHAR:
  case M_TYPE_SHORT:
  case M_TYPE_BLOCK:
  case M_TYPE_INT:
  case M_TYPE_BIT_LONG:
  case M_TYPE_LONG:
    M_int(ntype, type->unsign);
    return(M_TYPE_INT);
  case M_TYPE_BIT_LLONG:
  case M_TYPE_LLONG:
    M_arm_llong (ntype, type->unsign);
    return (M_TYPE_LLONG);
  case M_TYPE_POINTER:
    M_pointer(ntype);
    return(M_TYPE_POINTER);
  case M_TYPE_FLOAT:
    M_float(ntype, type->unsign);
    return(M_TYPE_FLOAT);
  case M_TYPE_DOUBLE:
    M_double(ntype, type->unsign);
    return M_TYPE_DOUBLE;
  case M_TYPE_UNION:
  case M_TYPE_STRUCT:
    *ntype = *type;
    return(type->type);
  default:
    return(-1);
  }
}


/*--------------------------------------------------------------------------*/
void M_arm_array_layout(M_Type type, int *offset)
{
    *offset = 0;			/* assume first element is aligned */
}

int M_arm_array_align(M_Type type)
{
    return type->align;
}

int M_arm_array_size(M_Type type, int dim)
{
    int mod, size, align;

    size = type->size;
    align = type->align;
    mod = size % align;
    if ( mod!=0 )
      	size += (align - mod);

    return (size * dim);
}

/*--------------------------------------------------------------------------*/
void M_arm_union_layout(int n, _M_Type *type, int *offset, int *bit_offset)
{
    int i;
    for ( i=0; i<n; i++ ) {	 /* assume the union is aligned */
    	offset[i] = 0;
    	bit_offset[i] = 0;
    }
}

int M_arm_union_align(int n, _M_Type *type)
{
    int i, max;
    max = 0;
    for ( i=0; i<n; i++ ) {
    	int aln = type[i].align;
    	if (aln > max)
      	    max = aln;
    }
    /*
     *	align to at least byte boundary.
     */
    if ( max < M_ARM_ALIGN_CHAR )
    	max = M_ARM_ALIGN_CHAR;

    return max;
}

int M_arm_union_size(int n, _M_Type *type)
{
    int i, aln, max_size, max_align;

    max_size = 0;
    max_align = 0;
    for ( i=0; i < n; i++ ) {
    	int size;
    	size = type[i].size;
    	if (size > max_size) 
	    max_size = size;
    	aln = type[i].align;
    	if (aln > max_align) 
	    max_align = aln;
    }
    /*
     *	align to at least byte boundary.
     */
    if ( max_align < M_ARM_ALIGN_CHAR )
    	max_align = M_ARM_ALIGN_CHAR;

    /* need to increment to the max. align for future array extension */
    i = max_size % max_align;
    if ( i != 0 )
    	max_size += (max_align - i);
	
    return max_size;
}

/*--------------------------------------------------------------------------*/
/* NOTE: the bit_offset array is never used by Hcode, so I ignore it.       */
/*--------------------------------------------------------------------------*/
void M_arm_struct_layout(int n, _M_Type *type, int *base, int *bit_offset)
{
    int i, offset;
    int mod, size, align, mod_word,mod_type;

    offset = 0;				/* assume initially aligned */
    for ( i=0; i<n ; i++ ) {
    	size = type[i].size;
    	align = type[i].align;
    	M_assert((size!=0) && (align!=0),
	     	"M_struct_layout: void is not allowed in structure");
        /*
         *  need to treat bit fields specially.
      	 *  keep them in a word when possible.
     	 */
	mod_word = offset % M_ARM_SIZE_INT;
	if ( type[i].type == M_TYPE_BIT_CHAR ) {
	    if ( (mod_word + size) > M_ARM_SIZE_INT )
		offset += (M_ARM_SIZE_INT - mod_word);
	    else {
		mod_type = offset % M_ARM_SIZE_CHAR;
		if ( (mod_type + size) > M_ARM_SIZE_CHAR ) {
		/* fprintf(stdout,"Changing bit char to bit short\n"); */
		    type[i].type = M_TYPE_BIT_SHORT;
		}
	    }
	}
	if ( type[i].type == M_TYPE_BIT_SHORT )  {
	    if ( (mod_word + size) > M_ARM_SIZE_INT )
		offset += (M_ARM_SIZE_INT - mod_word);
	    else {
		mod_type = offset % M_ARM_SIZE_SHORT;
		if ( (mod_type + size) > M_ARM_SIZE_SHORT ) {
		/* fprintf(stdout,"Changing bit short to bit long\n"); */
		    type[i].type = M_TYPE_BIT_LONG;
		}
	    }
	}
	else if ( type[i].type == M_TYPE_BIT_LONG )  {
	    if ( (mod_word + size) > M_ARM_SIZE_INT )
		offset += (M_ARM_SIZE_INT - mod_word);
	}

	mod = offset % align;		/* align to what the field */
        if ( mod!=0 )				/* needs to start from */
             offset += (align - mod);

        if ( type[i].type == M_TYPE_BIT_CHAR )  {
	    int mod = offset % M_ARM_SIZE_INT;

	    bit_offset[i] = offset - mod + (M_ARM_SIZE_INT - mod - size);
	    base[i] =   offset & (~(M_ARM_SIZE_CHAR-1));
#if 0
fprintf(stderr, "type = %d, base = %d, bit_offset = %d\n", type[i].type, base[i], bit_offset[i]);
#endif
	}
	else if ( type[i].type == M_TYPE_BIT_SHORT )  {
	    int mod = offset % M_ARM_SIZE_INT;

	    bit_offset[i] = offset - mod + (M_ARM_SIZE_INT - mod - size);
	    base[i] =   offset & (~(M_ARM_SIZE_SHORT-1));
#if 0
fprintf(stderr, "type = %d, base = %d, bit_offset = %d\n", type[i].type, base[i], bit_offset[i]);
#endif
	}
	else if ( type[i].type == M_TYPE_BIT_LONG )  {
	    int mod = offset % M_ARM_SIZE_INT;

	    bit_offset[i] = offset - mod + (M_ARM_SIZE_INT - mod - size);
	    base[i] =   offset & (~(M_ARM_SIZE_LONG-1));
#if 0
fprintf(stderr, "type = %d, base = %d, bit_offset = %d\n", type[i].type, base[i], bit_offset[i]);
#endif
	}
	else { 
            base[i] = offset;
	    bit_offset[i] = 0;
#if 0
fprintf(stderr, "type = %d, base = %d, bit_offset = %d\n", type[i].type, base[i], bit_offset[i]);
#endif

	}

        offset += size;			/* allocate space */
    }
}

int M_arm_struct_align(int n, _M_Type *type)
{
    int i, max;
    max = 0;
    for (i=0; i<n; i++) {
    	int aln = type[i].align;
    	if (aln > max) max = aln;
    }
    /*
     *	align to at least byte boundary.
     */
    if (max<M_ARM_ALIGN_CHAR)
    	max = M_ARM_ALIGN_CHAR;
    return max;
}

int M_arm_struct_size(int n, _M_Type *type, int struct_align)
{
    int i, offset;
    int mod, size, align, max_align, mod_word;
    offset = 0;				/* assume initially aligned */
    max_align = struct_align;
    for ( i=0; i<n; i++ ) {
    	size = type[i].size;
    	align = type[i].align;
    	M_assert((size!=0) && (align!=0),
	     		"M_struct_layout: void is not allowed in structure");
    	if (align > max_align)
            max_align = align;
    	/*
         *  need to treat bit fields specially.
         *  keep them in a word when possible.
     	 */
	mod_word = offset % M_ARM_SIZE_INT;
	if ( type[i].type == M_TYPE_BIT_CHAR ) {
	    if ( (mod_word + size) > M_ARM_SIZE_INT )
		offset += (M_ARM_SIZE_INT - mod_word);
	    if ( M_ARM_ALIGN_CHAR > max_align )
		max_align = M_ARM_ALIGN_CHAR;
	}
	else if ( type[i].type == M_TYPE_BIT_SHORT )  {
	    if ( (mod_word + size) > M_ARM_SIZE_INT )
		offset += (M_ARM_SIZE_INT - mod_word);
	    if ( M_ARM_ALIGN_SHORT > max_align )
		max_align = M_ARM_ALIGN_SHORT;
	}
	else if ( type[i].type == M_TYPE_BIT_LONG )  {
	    if ( (mod_word + size) > M_ARM_SIZE_INT )
		offset += (M_ARM_SIZE_INT - mod_word);
	    if ( M_ARM_ALIGN_LONG > max_align )
		max_align = M_ARM_ALIGN_LONG;
	}
	mod = offset % align;		/* align to what the field */
        if ( mod!=0 )				/* needs to start from */
             offset += (align - mod);

        offset += size;
    }
     /*
      *	align to at least byte boundary.
      */
    if ( max_align<M_ARM_ALIGN_CHAR )
	max_align = M_ARM_ALIGN_CHAR;
    /* enforce max. alignment */
    mod = offset % max_align;
    if (mod!=0)
	offset += (max_align - mod);
    return offset;
}


static int
M_ARM_align_offset(int offset, int align_bit)
{
  int mod;
  int align = align_bit / 8;

  if ((align_bit % 8) != 0)
    I_punt("M_ARM_align_offset: alignment not multiple of 8 = %d\n", align_bit);

  mod = offset % align;
  if (mod != 0)
    offset += (align - mod);  

  return offset;
}


int
M_arm_layout_fnvar (List param_list, char **base_macro,
		    int *pcount, int purpose, int needs_st)
{
  M_Param param;

  int off;
  int cur_rg;
  int size, align, tp;

  switch (purpose)
    {
        case M_GET_FNVAR:
          *base_macro = "$IP";
          break;
        case M_PUT_FNVAR:
          *base_macro = "$OP";
          break;
        case M_DONT_CARE_FNVAR:
        default:
          M_assert (0, "M_arm_layout_fnvar: unknown purpose");
    }

  /* If we're returning a struct, then the first param reg needs to
   * be saved for a pointer to the return value. */
  if(needs_st)
    cur_rg = 1;
  else
    cur_rg = 0;

  off = 0;
  
  List_start (param_list);
  while ((param = (M_Param)List_next (param_list)))
    {
      tp = param->mtype.type;
      size = param->mtype.size;
      align = param->mtype.align;

      switch (tp)
        {
          case M_TYPE_CHAR:
          case M_TYPE_SHORT:
          case M_TYPE_INT:
          case M_TYPE_LONG:
          case M_TYPE_POINTER:
            if (cur_rg < M_ARM_MAX_FNVAR_REG)
              {
                param->mode = M_THRU_REGISTER;
		param->offset = 0;
                param->reg = cur_rg + M_ARM_INT_BASE;
		cur_rg++;
		param->paddr = -1;
              }
            else
              {
                param->mode = M_THRU_MEMORY;
                param->reg = -1;
		param->paddr = -1;
		off = M_ARM_align_offset(off, align);
		param->offset = off;
		off += M_ARM_SIZE_INT;
              }
            break;

          case M_TYPE_FLOAT:
            if (cur_rg < M_ARM_MAX_FNVAR_REG)
              {
                param->mode = M_THRU_REGISTER;
		param->offset = 0;
                param->reg = cur_rg + M_ARM_FLT_BASE;
		param->paddr = -1;
                cur_rg++;
              }
            else
              {
                param->mode = M_THRU_MEMORY;
                param->reg = -1;
		param->paddr = -1;
		off = M_ARM_align_offset(off, align);
		param->offset = off;
		off += M_ARM_SIZE_FLOAT;
              }
            break;

          case M_TYPE_LLONG:
            if (cur_rg < M_ARM_MAX_FNVAR_REG)
              {
		int regs_needed;

                param->mode = M_THRU_REGISTER;
		param->offset = 0;
                param->reg = cur_rg + M_ARM_INT_BASE;

		regs_needed = (size / M_ARM_SIZE_INT);
		if (size % M_ARM_SIZE_INT)
		  regs_needed++;

		if (regs_needed + cur_rg < M_ARM_MAX_FNVAR_REG)
		  {
		    /* double fits entirely into param regs */
		    cur_rg += regs_needed;
		    param->offset = -1;
		    param->paddr = -1;
		  }
		else
		  {
		    /* double fills remaining regs and overlaps into memory */
		    int extra_space;
		    regs_needed = regs_needed - (M_ARM_MAX_FNVAR_REG - cur_rg);
		    cur_rg = M_ARM_MAX_FNVAR_REG;
		    extra_space = M_ARM_SIZE_INT * regs_needed;
		    param->offset = off;
		    param->paddr = -1;
		    off += extra_space;
		  }
              }
            else
              {
                param->mode = M_THRU_MEMORY;
                param->reg = -1;
		param->su_sreg = -1;
		param->su_ereg = -1;
		param->paddr = -1;
		off = M_ARM_align_offset(off, align);
		param->offset = off;
		off += M_ARM_SIZE_LLONG;
              }
            break;

          case M_TYPE_DOUBLE:
            if (cur_rg < M_ARM_MAX_FNVAR_REG)
              {
		int regs_needed;

                param->mode = M_THRU_REGISTER;
		param->offset = 0;
                param->reg = cur_rg + M_ARM_DBL_BASE;

		regs_needed = (size / M_ARM_SIZE_INT);
		if (size % M_ARM_SIZE_INT)
		  regs_needed++;

		if (regs_needed + cur_rg < M_ARM_MAX_FNVAR_REG)
		  {
		    /* double fits entirely into param regs */
		    cur_rg += regs_needed;
		    param->offset = -1;
		    param->paddr = -1;
		  }
		else
		  {
		    /* double fills remaining regs and overlaps into memory */
		    int extra_space;
		    regs_needed = regs_needed - (M_ARM_MAX_FNVAR_REG - cur_rg);
		    cur_rg = M_ARM_MAX_FNVAR_REG;
		    extra_space = M_ARM_SIZE_INT * regs_needed;
		    param->offset = off;
		    param->paddr = -1;
		    off += extra_space;
		  }
              }
            else
              {
                param->mode = M_THRU_MEMORY;
                param->reg = -1;
		param->su_sreg = -1;
		param->su_ereg = -1;
		param->paddr = -1;
		off = M_ARM_align_offset(off, align);
		param->offset = off;
		off += M_ARM_SIZE_DOUBLE;
              }
            break;

          case M_TYPE_UNION:
          case M_TYPE_STRUCT:
	    if (cur_rg < M_ARM_MAX_FNVAR_REG)
	      {
		int regs_needed;

		param->mode = M_THRU_REGISTER;
		param->reg = -1;
		param->su_sreg = cur_rg + M_ARM_INT_BASE;
		param->paddr = cur_rg;

		regs_needed = (size / M_ARM_SIZE_INT);
		if (size % M_ARM_SIZE_INT)
		  regs_needed++;

		if (regs_needed + cur_rg < M_ARM_MAX_FNVAR_REG)
		  {
		    /* Struct fits entirely into param regs */
		    cur_rg += regs_needed;
		    param->offset = -1;
		    param->su_ereg = cur_rg + M_ARM_INT_BASE - 1;
		  }
		else
		  {
		    /* Struct fills remaining regs and overlaps into memory */
		    int extra_space;
		    regs_needed = regs_needed - (M_ARM_MAX_FNVAR_REG - cur_rg);
		    cur_rg = M_ARM_MAX_FNVAR_REG;
		    extra_space = M_ARM_SIZE_INT * regs_needed;
		    param->offset = off;
		    param->su_ereg = cur_rg + M_ARM_INT_BASE - 1;
		    off += extra_space;
		  }
	      }
	    else
	      {
		param->mode = M_THRU_MEMORY;
		param->reg = -1;
		param->su_sreg = -1;
		param->su_ereg = -1;
		param->paddr = -1;
		off = M_ARM_align_offset(off, align);
		param->offset = off;
		off += size;
	      }
            break;
          default:
            M_assert (0, "M_arm_layout_fnvar: argument is not promoted");
        }

    }

  *pcount = cur_rg;
  return off;               /* the total size needed */
}

/*--------------------------------------------------------------------------*/
int M_arm_fnvar_layout(int n, _M_Type *type, long int *offset, int *mode,
	                int *reg, int *paddr, char **macro, 
                        int *su_sreg, int *su_ereg,
                        int *pcount, int needs_st, int purpose)
                  
               				/* need to return structure */
                 
{
    int i, off, cur_rg;
    int size, align, tp;

    switch (purpose) {
    	case M_GET_FNVAR: 
    	    *macro = "$IP"; 
	    break;
  	case M_PUT_FNVAR: 
	    *macro = "$OP";
	    break;
  	case M_DONT_CARE_FNVAR:
        default:	    
	    M_assert(0, "M_fnvar_layout: unknown purpose");
    }

    /* If we're returning a struct, then the first param reg needs to
     * be saved for a pointer to the return value. */
    if(needs_st)
      cur_rg = 1;
    else
      cur_rg = 0;

    off = 0;

    for ( i = 0; i < n; i++ ) {
      tp = type[i].type;
      size = type[i].size;
      align = type[i].align;
      switch (tp) {
	case M_TYPE_CHAR:
	case M_TYPE_SHORT:
	case M_TYPE_INT:
	case M_TYPE_LONG:
	case M_TYPE_POINTER:
	  if (cur_rg < M_ARM_MAX_FNVAR_REG)
	    {
	      mode[i] = M_THRU_REGISTER;
	      reg[i] = cur_rg + M_ARM_INT_BASE;
	      offset[i] = 0;
	      cur_rg++;
	    }
	  else
	    {
	      mode[i] = M_THRU_MEMORY;
	      reg[i] = -1;
	      paddr[i] = -1;
	      off = M_ARM_align_offset(off, align);
	      offset[i] = off;
	      off += M_ARM_SIZE_INT;
	    }
	  break;

	case M_TYPE_FLOAT:
	  if (cur_rg < M_ARM_MAX_FNVAR_REG)
	    {
	      mode[i] = M_THRU_REGISTER;
	      reg[i] = cur_rg + M_ARM_FLT_BASE;
	      paddr[i] = -1;
	      offset[i] = 0;
	      cur_rg++;
	    }
	  else
	    {
	      mode[i] = M_THRU_MEMORY;
	      reg[i] = -1;
	      paddr[i] = -1;
	      off = M_ARM_align_offset(off, align);
	      offset[i] = off;
	      off += M_ARM_SIZE_FLOAT;
	    }
	  break;

	case M_TYPE_LLONG:
	  if (cur_rg < M_ARM_MAX_FNVAR_REG)
	    {
	      int regs_needed;

	      mode[i] = M_THRU_REGISTER;
	      reg[i] = cur_rg + M_ARM_INT_BASE;
	      paddr[i] = -1;
	      offset[i] = 0;

	      regs_needed = (size / M_ARM_SIZE_INT);
	      if (size % M_ARM_SIZE_INT)
		regs_needed++;

	      if (regs_needed + cur_rg < M_ARM_MAX_FNVAR_REG)
		{
		  /* double fits entirely into param regs */
		  cur_rg += regs_needed;
		  offset[i] = 0;
		}
	      else
		{
		  /* double fills remaining regs and overlaps into memory */
		  int extra_space;
		  regs_needed = regs_needed - (M_ARM_MAX_FNVAR_REG - cur_rg);
		  cur_rg = M_ARM_MAX_FNVAR_REG;
		  extra_space = M_ARM_SIZE_INT * regs_needed;
		  offset[i] = off;
		  off += extra_space;
		}
	    }
	  else
	    {
	      mode[i] = M_THRU_MEMORY;
	      reg[i] = -1;
	      su_sreg[i] = -1;
	      su_ereg[i] = -1;
	      paddr[i] = -1;
	      off = M_ARM_align_offset(off, align);
	      offset[i] = off;
	      off += M_ARM_SIZE_LLONG;
	    }
	  break;

	case M_TYPE_DOUBLE:
	  if (cur_rg < M_ARM_MAX_FNVAR_REG)
	    {
	      int regs_needed;

	      mode[i] = M_THRU_REGISTER;
	      reg[i] = cur_rg + M_ARM_DBL_BASE;
	      paddr[i] = -1;
	      offset[i] = 0;

	      regs_needed = (size / M_ARM_SIZE_INT);
	      if (size % M_ARM_SIZE_INT)
		regs_needed++;

	      if (regs_needed + cur_rg < M_ARM_MAX_FNVAR_REG)
		{
		  /* double fits entirely into param regs */
		  cur_rg += regs_needed;
		  offset[i] = 0;
		}
	      else
		{
		  /* double fills remaining regs and overlaps into memory */
		  int extra_space;
		  regs_needed = regs_needed - (M_ARM_MAX_FNVAR_REG - cur_rg);
		  cur_rg = M_ARM_MAX_FNVAR_REG;
		  extra_space = M_ARM_SIZE_INT * regs_needed;
		  offset[i] = off;
		  off += extra_space;
		}
	    }
	  else
	    {
	      mode[i] = M_THRU_MEMORY;
	      reg[i] = -1;
	      su_sreg[i] = -1;
	      su_ereg[i] = -1;
	      paddr[i] = -1;
	      off = M_ARM_align_offset(off, align);
	      offset[i] = off;
	      off += M_ARM_SIZE_DOUBLE;
	    }
	  break;

	case M_TYPE_UNION:
	case M_TYPE_STRUCT:
	  if (cur_rg < M_ARM_MAX_FNVAR_REG)
	    {
	      int regs_needed;

	      mode[i] = M_THRU_REGISTER;
	      reg[i] = -1;
	      su_sreg[i] = cur_rg + M_ARM_INT_BASE;
	      paddr[i] = cur_rg;

	      regs_needed = (size / M_ARM_SIZE_INT);
	      if (size % M_ARM_SIZE_INT)
		regs_needed++;

	      if (regs_needed + cur_rg < M_ARM_MAX_FNVAR_REG)
		{
		  /* Struct fits entirely into param regs */
		  cur_rg += regs_needed;
		  offset[i] = -1;
		  su_ereg[i] = cur_rg + M_ARM_INT_BASE - 1;
		}
	      else
		{
		  /* Struct fills remaining regs and overlaps into memory */
		  int extra_space;
		  regs_needed = regs_needed - (M_ARM_MAX_FNVAR_REG - cur_rg);
		  cur_rg = M_ARM_MAX_FNVAR_REG;
		  extra_space = M_ARM_SIZE_INT * regs_needed;
		  offset[i] = off;
		  su_ereg[i] = cur_rg + M_ARM_INT_BASE - 1;
		  off += extra_space;
		}
	    }
	  else
	    {
	      mode[i] = M_THRU_MEMORY;
	      reg[i] = -1;
	      su_sreg[i] = -1;
	      su_ereg[i] = -1;
	      paddr[i] = -1;
	      off = M_ARM_align_offset(off, align);
	      offset[i] = off;
	      off += size;
	    }
	  break;
	default:
	  M_assert (0, "M_arm_layout_fnvar: argument is not promoted");
        }
    }

    *pcount = cur_rg;
    return off;   /* the total size needed */
}

/*--------------------------------------------------------------------------*/
int M_arm_lvar_layout(int n, _M_Type *type, long int *offset, char **base_macro)
{
    int i, max_align, off;
    int size, align, mod, tp;
    /*
     *	the LOCAL section must be max. aligned initially
     */
    max_align = M_ARM_ALIGN_MAX;
    off = 0;
    for ( i = 0; i < n; i++ ) {
        tp = type[i].type;
        if (tp == M_TYPE_BIT_LONG) {
            M_assert(0, "M_lvar_layout: bit field not allowed");
        }
    	if (tp == M_TYPE_BIT_CHAR) {
       	    M_assert(0, "M_lvar_layout: bit field not allowed");
    	}
        size = type[i].size;
        align = type[i].align;
        mod = off % align;
        if ( mod != 0 )
      	    off += (align - mod);

        // high to low, with posative offsets
        offset[i] = off;
	off += size;
    }
    
    /* the local section must be max. aligned */
    mod = off % max_align;
    if ( mod != 0 )
    	off += (max_align - mod);
    /*
     *	Local variables are relative to $LV
     */
    *base_macro = "$LV";
    return off;				/* the total size needed */
}

/*--------------------------------------------------------------------------*/
int M_arm_no_short_int(void) {
    return (M_ARM_SIZE_SHORT == M_ARM_SIZE_INT);
}
/*--------------------------------------------------------------------------*/
void M_arm_cb_label_name(char *fn, int cb, char *line, int len)
{
    sprintf(line, "cb%d%s", cb, fn);
}
/*--------------------------------------------------------------------------*/
int M_arm_is_cb_label(char *label, char *fn, int *cb)
{
    return (sscanf(label, "cb%d%s", cb, fn)==2);
}
/*--------------------------------------------------------------------------*/
void M_arm_jumptbl_label_name(char *fn, int tbl_id, char *line, int len)
{
    sprintf(line, "_$%s%s%d", fn, M_JUMPTBL_BASE_NAME, tbl_id);
}
/*--------------------------------------------------------------------------*/
/* Format for arm is: _$%sM_JUMPTBL_BASE_NAME%d, where %s is the func name */
int M_arm_is_jumptbl_label(char *label, char *fn, int *tbl_id)
{
    char *ptr;
    int label_len, fn_len, base_len;

    /* Check the prefix */
    if (strncmp(label, "_$", 2))
	return (0);

    /* Some length checks, to make sure we dont step outside array */
    label_len = strlen(label);
    fn_len = strlen(fn);
    base_len = strlen(M_JUMPTBL_BASE_NAME);
    if (label_len <= (2+fn_len+base_len))
	return (0);

    /* Check that fn is correct */
    ptr = label;
    ptr += 2;
    if (strncmp(ptr, fn, fn_len))
	return (0);

    /* Check that jumptbl base name is correct */
    ptr += fn_len;
    if (strncmp(ptr, M_JUMPTBL_BASE_NAME, base_len))
	return (0);

    /* Get the id */
    ptr += base_len;
    return (sscanf(ptr, "%d", tbl_id)==1);
}
/*--------------------------------------------------------------------------*/
int M_arm_structure_pointer(int purpose)
{
    return M_ARM_RET_ST;
}
/*--------------------------------------------------------------------------*/
int M_arm_return_register(int type, int purpose)
{
    switch (type) {
  	case M_TYPE_INT:	return M_ARM_RET_I32;
  	case M_TYPE_LONG:	return M_ARM_RET_I32;
  	case M_TYPE_FLOAT:	return M_ARM_RET_F;
  	case M_TYPE_DOUBLE:	return M_ARM_RET_F;
	case M_TYPE_LLONG:      return M_ARM_RET_I64;
  	default:		return M_ARM_RET_I32;
    }
}
/*--------------------------------------------------------------------------*/
/*
char *M_arm_fn_label_name(label)
*/
char *M_arm_fn_label_name(char *label, int (*is_func) (char *is_func_label))
{
    static char fn_label[1024];

    if ( (*is_func)(label) )  {
        sprintf(fn_label,"$fn_%s",label);
        return(fn_label);
    }
    else 
        return(label);
}

char *M_arm_fn_name_from_label(char *label)
{
    if ( !strncmp(label,"_$fn",4) )
	return(label+4);
    else
	return(label);
}

/*--------------------------------------------------------------------------*/
int M_arm_fragile_macro(int macro_value)
{
    switch (M_model)  {
	case M_ARM_1_01:
	    switch(macro_value)  {
		case L_MAC_SP:
            	case L_MAC_FP:
            	case L_MAC_LV:
            	case L_MAC_IP:
            	case L_MAC_OP:
            	case L_MAC_LOCAL_SIZE:
            	case L_MAC_PARAM_SIZE:
            	case L_MAC_SWAP_SIZE:       
				return 0;	
	        default:	return(1);
	    }
	default:
	    M_assert (0, "M_arm_fragile_macro:  Illegal model specified!");
	    return (0);
    }
}

/*--------------------------------------------------------------------------*/
int M_arm_subroutine_call(int opc)
{
    switch (M_model)  {
	case M_ARM_1_01:
	    return ((opc==Lop_JSR) || (opc==Lop_JSR_FS) ||
                    (opc==Lop_DIV) || (opc==Lop_DIV_U) ||
                    (opc==Lop_REM) || (opc==Lop_REM_U));
	default:
	    M_assert (0, "M_arm_subroutine_call:  Illegal model specified!");
	    return (0);
    }
}
/*--------------------------------------------------------------------------*/
/*
 * Declare code generator specific macro registers to the front end parser.
 */
void M_define_macros_arm(STRING_Symbol_Table *sym_tbl)
{
    M_add_symbol(sym_tbl, "$true_sp",    ARM_MAC_TRUE_SP);
    /* 1 if leaf function, 0 if non-leaf */
    M_add_symbol(sym_tbl, "$leaf",   ARM_MAC_LEAF);
    /* total alloc requirements */
    M_add_symbol(sym_tbl, "$alloc_size",   ARM_MAC_ALLOC);
    /* number of integer and float callee saved registers used */
    M_add_symbol(sym_tbl, "$callee_i_regs",    ARM_MAC_CALLEE_I);
    M_add_symbol(sym_tbl, "gr2", ARM_MAC_RETADDR);
    M_add_symbol(sym_tbl, "fr0", ARM_MAC_FZERO);
    M_add_symbol(sym_tbl, "fr1", ARM_MAC_FONE);
}

char *M_get_macro_name_arm(int id)
{
    switch (id) {
  	case ARM_MAC_LEAF:              return "$leaf";
  	case ARM_MAC_ALLOC:             return "$alloc_size";
  	case ARM_MAC_CALLEE_I:          return "$callee_i_regs";
  	case ARM_MAC_TRUE_SP:           return "$true_sp";
        case ARM_MAC_RETADDR:           return "gr2";
        case ARM_MAC_FZERO:             return "fr0";
        case ARM_MAC_FONE:              return "fr1";

  	default:                       
				      return "?";
    }
}

void M_define_opcode_name_arm(STRING_Symbol_Table * sym_tbl)
{
}

char *M_get_opcode_name_arm(int id)
{
    switch(id)  {
	default:			return ("?");
    }
}

/*--------------------------------------------------------------------------*/
/*
 * Return true (1) if the instruction is supported in the hardware of the
 * processor.  Return false (0) otherwise.
 */
int M_oper_supported_in_arch_arm(int opc)
{
    switch(opc) {

       	case Lop_NOR:
	case Lop_NAND:
 	case Lop_NXOR:
	case Lop_OR_NOT:
        case Lop_EXTRACT:
        case Lop_EXTRACT_U:
        case Lop_DEPOSIT:
	case Lop_MUL_ADD:
	case Lop_MUL_ADD_U:
	case Lop_MUL_SUB:
	case Lop_MUL_SUB_U:
	case Lop_MUL_SUB_REV:
	case Lop_MUL_SUB_REV_U:
	case Lop_MUL_SUB_F:
	case Lop_MUL_SUB_REV_F:
	case Lop_MUL_SUB_F2:
	case Lop_MUL_SUB_REV_F2:
	    return(0);
	    
	case Lop_MUL_ADD_F:
	case Lop_MUL_ADD_F2:
	        return(0);
	    
	default:	
	    return(1);
    }
}
/*
 * Returns the number of machine instructions required to implement the
 * specified oper in the best case.  It is assumed that this is for
 * supported instructions.  A call to M_oper_supported_in_arch should be
 * made for abnormal instructions.
 *
 * FIX: this should probably be reexamined when time permits.
 */

int M_num_oper_required_for_arm(L_Oper *oper, char *name)
{
#define	has_label_operand(a)	((a->src[0]->type == L_OPERAND_LABEL)||\
    				 (a->src[1]->type == L_OPERAND_LABEL))

#define indexed_memory_op(a)	((a->src[0]->type == L_OPERAND_REGISTER)&&\
    				 (a->src[1]->type == L_OPERAND_REGISTER))
   
#define non_zero_offset(a)	((!((a->src[1]->type==L_OPERAND_IMMED)&&\
                                   ((a->src[1]->ctype&0x30)==0x00)))||\
    				 (a->src[1]->value.i != 0))
    
#define short_int_inc(a,b)	((a->src[b]->type==L_OPERAND_IMMED)&&\
                                 ((a->src[b]->ctype&0x30)==0x00)&&\
    				 (a->src[b]->value.i >= -0x10) &&\
    				 (a->src[b]->value.i < 0x10))
#define long_pos_int_inc(a,b)	((a->src[b]->type==L_OPERAND_IMMED)&&\
                                 ((a->src[b]->ctype&0x30)==0x00)&&\
    				 (a->src[b]->value.i > 0) && \
    				 (a->src[b]->value.i < 0x2000))
#define long_neg_int_inc(a,b)	((a->src[b]->type==L_OPERAND_IMMED)&&\
                                 ((a->src[b]->ctype&0x30)==0x00)&&\
    				 (a->src[b]->value.i >= -0x2000) && \
    				 (a->src[b]->value.i < 0))
#define register_inc(a,b)	 (a->src[b]->type == L_OPERAND_REGISTER)

    switch (oper->opc)  {
	case Lop_ST_C:  
	case Lop_ST_C2:
	case Lop_ST_I:
	  return(1);
	    
	case Lop_LD_UC: 
	case Lop_LD_C: 
	case Lop_LD_UC2: 
	case Lop_LD_C2: 
	case Lop_LD_I: 
	  return(1);
	      	
  	case Lop_ST_F: 
	case Lop_ST_F2: 
	  return(1);

	case Lop_LD_F: 
	case Lop_LD_F2:
	  return(1);
	    
	case Lop_LD_PRE_UC:  
	case Lop_LD_PRE_C: 
	case Lop_LD_PRE_UC2: 
	case Lop_LD_PRE_C2: 
	case Lop_LD_PRE_I:
	    if ( indexed_memory_op(oper) || non_zero_offset(oper) )
		return(2);
	    else if ( short_int_inc(oper,2) ||
		      long_neg_int_inc(oper,2) )
		return(1);
	    else 
		return(2);
	    
	case Lop_ST_PRE_C: 
	case Lop_ST_PRE_C2: 
	case Lop_ST_PRE_I: 
	    if ( indexed_memory_op(oper) || non_zero_offset(oper) )
		return(2);
	    else if ( short_int_inc(oper,3) ||
		      long_neg_int_inc(oper,3) )
		return(1);
	    else 
		return(2);
	    
	case Lop_LD_POST_UC: 
	case Lop_LD_POST_C: 
	case Lop_LD_POST_UC2: 
	case Lop_LD_POST_C2: 
	case Lop_LD_POST_I:
	    if ( non_zero_offset(oper) )
		return(2);
	    else if ( short_int_inc(oper,2) ||
		      long_pos_int_inc(oper,2) ||
		      register_inc(oper,2) )
		return(1);
	    else
		return(2);
	    
	case Lop_ST_POST_C: 
	case Lop_ST_POST_C2: 
	case Lop_ST_POST_I:  
	    if ( indexed_memory_op(oper) || non_zero_offset(oper) )
		return(2);
	    else if ( short_int_inc(oper,3) ||
		      long_pos_int_inc(oper,3) )
		return(1);
	    else
		return(2);
	    
	case Lop_LD_PRE_F: 
	case Lop_LD_PRE_F2:
	    if ( indexed_memory_op(oper) || non_zero_offset(oper) )
		return(2);
	    else if ( short_int_inc(oper,2) )
		return(1);
	    else 
		return(2);
	    
	case Lop_ST_PRE_F: 
	case Lop_ST_PRE_F2:
	    if ( indexed_memory_op(oper) || non_zero_offset(oper) )
		return(2);
	    else if ( short_int_inc(oper,3) )
		return(1);
	    else 
		return(2);
	    
	case Lop_LD_POST_F: 
	case Lop_LD_POST_F2:  
	    if ( non_zero_offset(oper) )
		return(2);
	    else if ( short_int_inc(oper,2) ||
		      register_inc(oper,2) )
		return(1);
	    else
		return(2);
	case Lop_ST_POST_F: 
	case Lop_ST_POST_F2:
	    if ( non_zero_offset(oper) )
		return(2);
	    else if ( short_int_inc(oper,3) ||
		      register_inc(oper,3) )
		return(1);
	    else
		return(2);

	case Lop_PREF_LD:
	    if (has_label_operand(oper))
		return (2);
	    else
		return(1);

	default:
	    return(1);
    }
}

int M_is_stack_operand_arm(L_Operand *operand)
{
    if (L_is_macro(operand) &&
	 ( operand->value.mac == L_MAC_SP ||
	   operand->value.mac == L_MAC_FP ||
	   operand->value.mac == ARM_MAC_TRUE_SP ||
	   operand->value.mac == L_MAC_SAFE_MEM ||
	   operand->value.mac == L_MAC_P12 ||
	   operand->value.mac == L_MAC_IP ||
	   operand->value.mac == L_MAC_OP ||
	   operand->value.mac == L_MAC_RS ||
	   operand->value.mac == L_MAC_LV ) )
	return(1);
    
    return(0);
}

int M_is_unsafe_macro_arm(L_Operand *operand)
{
    if (!L_is_macro(operand))
	return(0);
    
    switch (operand->value.mac)  {
        // int_param_1
        case L_MAC_P0:
	// flt_return
        case L_MAC_P4:
	// flt_param_1
        case L_MAC_P5:
	// int_return
        case L_MAC_P15:
        case L_MAC_LV:
        case L_MAC_IP:
        case L_MAC_OP:
        case L_MAC_LOCAL_SIZE:
        case L_MAC_PARAM_SIZE:
        case L_MAC_SWAP_SIZE:
                return 1;
	default:		
                return 0;
    }
}

int M_operand_type_arm(L_Operand *operand)
{
    /* If NULL operand pointer, then return MDES_OPERAND_NULL */
    if (operand == NULL)
        return(MDES_OPERAND_NULL);

    switch (L_operand_case_type(operand))  {
        case L_OPERAND_INT:
	    if ( FIELD_5(operand->value.i) )
		return(MDES_OPERAND_Lit5);
            else if ( FIELD_11(operand->value.i) )
                return(MDES_OPERAND_Lit11);
            else if ( FIELD_14(operand->value.i) )
                return(MDES_OPERAND_Lit14);
	    else
		return(MDES_OPERAND_Lit21);

        case L_OPERAND_MACRO:
	case L_OPERAND_REGISTER:
            return(MDES_OPERAND_REG);

        case L_OPERAND_CB:
	case L_OPERAND_LABEL:
	case L_OPERAND_FLOAT:
        case L_OPERAND_DOUBLE:
        case L_OPERAND_STRING:
	    return(MDES_OPERAND_Label);

        default:
            M_assert(0,"M_operand_type_arm: Unknown type");
	    return (0);
    }
}

int M_conflicting_operands_arm(L_Operand *operand, L_Operand **conflict_array, int len, int prepass)
{
    int right=0, left=0;

    if ( prepass && (!L_is_macro(operand)) ) {
        conflict_array[0] = L_copy_operand(operand);
        return(1);
    }
    if (L_is_macro(operand)) {
	switch (operand->value.mac)  {
	    case L_MAC_SP:
	    case ARM_MAC_TRUE_SP:
	    case L_MAC_SAFE_MEM:
	      conflict_array[0] = L_new_macro_operand(L_MAC_SP, L_CTYPE_INT, L_PTYPE_NULL);
	      conflict_array[1] = L_new_macro_operand(ARM_MAC_TRUE_SP, L_CTYPE_INT, L_PTYPE_NULL);
	      conflict_array[2] = L_new_macro_operand(L_MAC_SAFE_MEM, L_CTYPE_INT, L_PTYPE_NULL);
	      return(3);
	    default:
	      conflict_array[0] = L_copy_operand(operand);
	      return(1);
	}
    }	    
    else if (L_is_reg(operand)) {
	conflict_array[0] = L_copy_operand(operand);
	return(1);
    }
    else
	M_assert(0,"Larm_conflicting_operands: unsupported operand type");
    
    if (L_is_ctype_dbl(operand)) {
      /* Set up conflicting double register */
      conflict_array[0] = L_copy_operand(operand);
      conflict_array[0]->value.r = left;
      
      /* Set up conflicting float register */
      conflict_array[1] = L_copy_operand(operand);
      conflict_array[1]->ctype = L_CTYPE_FLOAT;
      conflict_array[1]->value.r = left;
      
      conflict_array[2] = L_copy_operand(operand);
      conflict_array[2]->ctype = L_CTYPE_FLOAT;
      conflict_array[2]->value.r = right;
      return(3);
    }
    else if (L_is_ctype_flt(operand)) {
      /* Set up conflicting double register */
      conflict_array[0] = L_copy_operand(operand);
      conflict_array[0]->ctype = L_CTYPE_DOUBLE;
      conflict_array[0]->value.r = left;
      
      /* Set up conflicting double register */
      conflict_array[1] = L_copy_operand(operand);
      return(2);
    }
    else {
      M_assert(0,"Larm_conflicting_operands: unsupported operand type");
      return (0);
    }
}

int M_num_registers_arm(int ctype)
{

    if (M_model==M_ARM_1_01) {
	/* These numbers shouldn't matter... */
        switch (ctype) {
            case L_CTYPE_INT:
                    return (15);
            case L_CTYPE_FLOAT:
                    return (64);
            case L_CTYPE_DOUBLE:
                    return (64);
            default:
                    return (0);
        }
    }
    else {
	M_assert(0, "M_num_registers_arm: unsupported model");
    }
    return (0);
}


int
M_arm_layout_retvar (M_Param param, int purpose)
{
  if (param->mtype.type & (M_TYPE_STRUCT | M_TYPE_UNION))
    param->mode = M_INDIRECT_THRU_MEMORY;
  else
    param->mode = M_THRU_MEMORY;
  return (param->mode == M_INDIRECT_THRU_MEMORY);
}
