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
 *	File :	m_wims.c 
 *	Desc :	Machine dependent specification.  
 *	Date :	Feb, 1992
 *	Auth :  Richard Hank, Wen-mei Hwu
 *
 *    Modified : modified from m_spec.c code by Pohua Paul Chang.
 *
 *==========================================================================*/
#include <stdio.h>
#ifdef M_WIMS_FOR_HCODE
#include <Hcode/h_ccode.h>
#endif
#include <Lcode/l_main.h>
#include "m_wims.h"

/*--------------------------------------------------------------------------*/
#define M_WIMS_SIZE_VOID	0
#define M_WIMS_SIZE_BIT		1
#define M_WIMS_SIZE_CHAR	8
#define M_WIMS_SIZE_SHORT	16
#define M_WIMS_SIZE_INT	        16
#define M_WIMS_SIZE_LONG	16
#define M_WIMS_SIZE_LLONG	32
#define M_WIMS_SIZE_FLOAT	-1
#define M_WIMS_SIZE_DOUBLE	-1
#define M_WIMS_SIZE_POINTER	((M_model == M_WIMS_16) ? 16 : 32)
#define M_WIMS_SIZE_UNION	-1
#define M_WIMS_SIZE_STRUCT	-1
#define M_WIMS_SIZE_BLOCK	-1
#define M_WIMS_SIZE_MAX	 	((M_model == M_WIMS_16) ? 16 : 32)

#define M_WIMS_ALIGN_VOID	-1
#define M_WIMS_ALIGN_BIT	1
#define M_WIMS_ALIGN_CHAR	8
#define M_WIMS_ALIGN_SHORT	16
#define M_WIMS_ALIGN_INT	16
#define M_WIMS_ALIGN_LONG	16
#define M_WIMS_ALIGN_LLONG      32
#define M_WIMS_ALIGN_FLOAT	-1 
#define M_WIMS_ALIGN_DOUBLE	-1
#define M_WIMS_ALIGN_POINTER	((M_model == M_WIMS_16) ? 16 : 32)
#define M_WIMS_ALIGN_UNION	-1	/* depends on the field */
#define M_WIMS_ALIGN_STRUCT	-1
#define M_WIMS_ALIGN_BLOCK	-1
#define M_WIMS_ALIGN_MAX	((M_model == M_WIMS_16) ? 16 : 32)

/*--------------------------------------------------------------------------*/
#define M_WIMS_MAX_FNVAR_REG 		0	
#define M_WIMS_SMALL_STRUCT_MAX 	32
#define MIN_PARAM_SIZE 	  		(32 * 0)

/* incoming and outgoing parameters */
#define M_WIMS_INT_BASE		0
#define M_WIMS_FLT_BASE		0

/* All return values are placed in P6, which is float/int */
#define M_WIMS_RET_F              15       /* return in R0 */
#define M_WIMS_RET_I32            15       /* return in R0 */
#define M_WIMS_RET_I64            15       /* return in R0 */
#define M_WIMS_RET_ST             15       /* return in R0 */


/*--------------------------------------------------------------------------*/
/*
 *	The following functions are for documentation only.
 *	The function overhead is too great for actual usage.
 */
void M_wims_void(M_Type type)
{
    type->type = M_TYPE_VOID;
    type->unsign = 1;
    type->align = M_WIMS_ALIGN_VOID;
    type->size = M_WIMS_SIZE_VOID;
    type->nbits = 0;
}

void M_wims_bit_long(M_Type type, int n)
{
    type->type = M_TYPE_BIT_LONG;
    type->unsign = 1;
    type->align = M_WIMS_ALIGN_BIT;
    type->size = n * M_WIMS_SIZE_BIT;
    type->nbits = n * M_WIMS_SIZE_BIT;
    M_assert ((n<=16), "M_bit_long: do not allow bit field of more than 16 bits");
}

void M_wims_bit_llong(M_Type type, int n)
{
    type->type = M_TYPE_BIT_LLONG;
    type->unsign = 1;
    type->align = M_WIMS_ALIGN_BIT;
    type->size = n * M_WIMS_SIZE_BIT;
    type->nbits = n * M_WIMS_SIZE_BIT;
    M_assert ((n<=16), "M_bit_long: do not allow bit field of more than 16 bits");
}

void M_wims_bit_int (M_Type type, int n)
{
    type->type = M_TYPE_BIT_INT;
    type->unsign = 1;
    type->align = M_WIMS_ALIGN_BIT;
    type->size = n * M_WIMS_SIZE_BIT;
    type->nbits = n * M_WIMS_SIZE_BIT;
    M_assert ((n <= 16), "M_bit_int: do not allow bit field of more than 16 bits");
}

void M_wims_bit_short(M_Type type, int n)
{
    type->type = M_TYPE_BIT_SHORT;
    type->unsign = 1;
    type->align = M_WIMS_ALIGN_BIT;
    type->size = n * M_WIMS_SIZE_BIT;
    type->nbits = n * M_WIMS_SIZE_BIT;
    M_assert ((n<=16), "M_bit_long: do not allow bit field of more than 16 bits");
}

void M_wims_bit_char(M_Type type, int n)
{
    type->type = M_TYPE_BIT_CHAR;
    type->unsign = 1;
    type->align = M_WIMS_ALIGN_BIT;
    type->size = n * M_WIMS_SIZE_BIT;
    type->nbits = n * M_WIMS_SIZE_BIT;
    M_assert ((n<=8), "M_bit_char: do not allow bit field of more than 8 bits");
}

void M_wims_float(M_Type type, int unsign)
{
    M_assert(0, "M_wims_float: float not supported in wims");
    type->type = M_TYPE_FLOAT;
    type->unsign = unsign;
    type->align = M_WIMS_ALIGN_FLOAT;
    type->size = M_WIMS_SIZE_FLOAT;
    type->nbits = M_WIMS_SIZE_FLOAT;
}

void M_wims_double(M_Type type, int unsign)
{
    M_assert(0, "M_wims_double: double not supported in wims");
    type->type = M_TYPE_DOUBLE;
    type->unsign = unsign;
    type->align = M_WIMS_ALIGN_DOUBLE;
    type->size = M_WIMS_SIZE_DOUBLE;
    type->nbits = M_WIMS_SIZE_DOUBLE;
}

void M_wims_pointer(M_Type type)
{
    type->type = M_TYPE_POINTER;
    type->unsign = 1;
    type->align = M_WIMS_ALIGN_POINTER;
    type->size = M_WIMS_SIZE_POINTER;
    type->nbits = M_WIMS_SIZE_POINTER;
}

/*--------------------------------------------------------------------------*/
int M_wims_eval_type(M_Type type, M_Type ntype)
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
  case M_TYPE_BIT_LLONG:
  case M_TYPE_LLONG:
    M_wims_llong(ntype, type->unsign);
    return(M_TYPE_LLONG);
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

int M_wims_eval_type2(M_Type type, M_Type ntype)
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
    M_wims_llong(ntype, type->unsign);
    return(M_TYPE_LLONG);
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

int M_wims_call_type(M_Type type, M_Type ntype)
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
    M_wims_llong (ntype, type->unsign);
    return (M_TYPE_LLONG);
  case M_TYPE_FLOAT:
#if 0
    /* commented out, because passing floats doesn't appear to work with
       library functions such as printf, although nowhere does it say that
       I have to use doubles						*/
    M_float(ntype, type->unsign);
    return(M_TYPE_FLOAT);
#endif
    /* BCC - 8/5/96
     * Pcode has inserted all the necessary castings. So don't insert
     * promotions here
     */
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

int M_wims_call_type2(M_Type type, M_Type ntype)
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
    M_wims_llong (ntype, type->unsign);
    return (M_TYPE_LLONG);
  case M_TYPE_POINTER:
    M_pointer(ntype);
    return(M_TYPE_POINTER);
  case M_TYPE_FLOAT:
#if 0
    /* commented out, because passing floats doesn't appear to work with
       library functions such as printf, although nowhere does it say that
       I have to use doubles						*/
    M_float(ntype, type->unsign);
    return(M_TYPE_FLOAT);
#endif
    /* BCC - 8/5/96
     * Pcode has inserted all the necessary castings. So don't insert
     * promotions here
     */
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

#if 0
int M_wims_call_type2(M_Type type, M_Type ntype)
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
    M_int(ntype, type->unsign);
    return(M_TYPE_INT);
  case M_TYPE_BIT_LONG:
  case M_TYPE_LONG:
    M_long(ntype, type->unsign);
    return(M_TYPE_LONG);
  case M_TYPE_POINTER:
    M_pointer(ntype);
    return(M_TYPE_POINTER);
  case M_TYPE_FLOAT:
#if 0
    /* commented out, because passing floats doesn't appear to work with
       library functions such as printf, although nowhere does it say that
       I have to use doubles						*/
    M_float(ntype, type->unsign);
    return(M_TYPE_FLOAT);
#endif
    /* BCC - 8/5/96
     * Pcode has inserted all the necessary castings. So don't insert
     * promotions here
     */
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
#endif

/*--------------------------------------------------------------------------*/
void M_wims_array_layout(M_Type type, int *offset)
{
    *offset = 0;			/* assume first element is aligned */
}

int M_wims_array_align(M_Type type)
{
    return type->align;
}

int M_wims_array_size(M_Type type, int dim)
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
void M_wims_union_layout(int n, _M_Type *type, int *offset, int *bit_offset)
{
    int i;
    for ( i=0; i<n; i++ ) {			/* assume the union is aligned */
    	offset[i] = 0;
    	bit_offset[i] = 0;
    }
}

int M_wims_union_align(int n, _M_Type *type)
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
    if ( max < M_WIMS_ALIGN_CHAR )
    	max = M_WIMS_ALIGN_CHAR;

    return max;
}

int M_wims_union_size(int n, _M_Type *type)
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
    if ( max_align < M_WIMS_ALIGN_CHAR )
    	max_align = M_WIMS_ALIGN_CHAR;

    /* need to increment to the max. align for future array extension */
    i = max_size % max_align;
    if ( i != 0 )
    	max_size += (max_align - i);
	
    return max_size;
}

/*--------------------------------------------------------------------------*/
/* NOTE: the bit_offset array is never used by Hcode, so I ignore it.       */
/*--------------------------------------------------------------------------*/
void M_wims_struct_layout(int n, _M_Type *type, int *base, int *bit_offset)
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
	mod_word = offset % M_WIMS_SIZE_INT;
	if ( type[i].type == M_TYPE_BIT_CHAR ) {
	    if ( (mod_word + size) > M_WIMS_SIZE_INT )
		offset += (M_WIMS_SIZE_INT - mod_word);
	    else {
		mod_type = offset % M_WIMS_SIZE_CHAR;
		if ( (mod_type + size) > M_WIMS_SIZE_CHAR ) {
		/* fprintf(stdout,"Changing bit char to bit short\n"); */
		    type[i].type = M_TYPE_BIT_SHORT;
		}
	    }
	}
	if ( type[i].type == M_TYPE_BIT_SHORT )  {
	    if ( (mod_word + size) > M_WIMS_SIZE_INT )
		offset += (M_WIMS_SIZE_INT - mod_word);
	    else {
		mod_type = offset % M_WIMS_SIZE_SHORT;
		if ( (mod_type + size) > M_WIMS_SIZE_SHORT ) {
		/* fprintf(stdout,"Changing bit short to bit long\n"); */
		    type[i].type = M_TYPE_BIT_LONG;
		}
	    }
	}
	else if ( type[i].type == M_TYPE_BIT_LONG )  {
	    if ( (mod_word + size) > M_WIMS_SIZE_INT )
		offset += (M_WIMS_SIZE_INT - mod_word);
	}

	mod = offset % align;		/* align to what the field */
        if ( mod!=0 )				/* needs to start from */
             offset += (align - mod);

        if ( type[i].type == M_TYPE_BIT_CHAR )  {
	    int mod = offset % M_WIMS_SIZE_INT;

	    bit_offset[i] = offset - mod + (M_WIMS_SIZE_INT - mod - size);
	    base[i] =   offset & (~(M_WIMS_SIZE_CHAR-1));
#if 0
fprintf(stderr, "type = %d, base = %d, bit_offset = %d\n", type[i].type, base[i], bit_offset[i]);
#endif
	}
	else if ( type[i].type == M_TYPE_BIT_SHORT )  {
	    int mod = offset % M_WIMS_SIZE_INT;

	    bit_offset[i] = offset - mod + (M_WIMS_SIZE_INT - mod - size);
	    base[i] =   offset & (~(M_WIMS_SIZE_SHORT-1));
#if 0
fprintf(stderr, "type = %d, base = %d, bit_offset = %d\n", type[i].type, base[i], bit_offset[i]);
#endif
	}
	else if ( type[i].type == M_TYPE_BIT_LONG )  {
	    int mod = offset % M_WIMS_SIZE_INT;

	    bit_offset[i] = offset - mod + (M_WIMS_SIZE_INT - mod - size);
	    base[i] =   offset & (~(M_WIMS_SIZE_LONG-1));
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

int M_wims_struct_align(int n, _M_Type *type)
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
    if (max<M_WIMS_ALIGN_CHAR)
    	max = M_WIMS_ALIGN_CHAR;
    return max;
}

int M_wims_struct_size(int n, _M_Type *type, int struct_align)
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
	mod_word = offset % M_WIMS_SIZE_INT;
	if ( type[i].type == M_TYPE_BIT_CHAR ) {
	    if ( (mod_word + size) > M_WIMS_SIZE_INT )
		offset += (M_WIMS_SIZE_INT - mod_word);
	    if ( M_WIMS_ALIGN_CHAR > max_align )
		max_align = M_WIMS_ALIGN_CHAR;
	}
	else if ( type[i].type == M_TYPE_BIT_SHORT )  {
	    if ( (mod_word + size) > M_WIMS_SIZE_INT )
		offset += (M_WIMS_SIZE_INT - mod_word);
	    if ( M_WIMS_ALIGN_SHORT > max_align )
		max_align = M_WIMS_ALIGN_SHORT;
	}
	else if ( type[i].type == M_TYPE_BIT_LONG )  {
	    if ( (mod_word + size) > M_WIMS_SIZE_INT )
		offset += (M_WIMS_SIZE_INT - mod_word);
	    if ( M_WIMS_ALIGN_LONG > max_align )
		max_align = M_WIMS_ALIGN_LONG;
	}
	mod = offset % align;		/* align to what the field */
        if ( mod!=0 )				/* needs to start from */
             offset += (align - mod);

        offset += size;
    }
     /*
      *	align to at least byte boundary.
      */
    if ( max_align<M_WIMS_ALIGN_CHAR )
	max_align = M_WIMS_ALIGN_CHAR;
    /* enforce max. alignment */
    mod = offset % max_align;
    if (mod!=0)
	offset += (max_align - mod);
    return offset;
}

int
M_wims_layout_fnvar (List param_list, char **base_macro,
                     int *pcount, int purpose)
{
  M_Param param;

  int max_align, off;
  int int_rg;
  int fp_rg;
  int size, align, mod, tp;

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
          M_assert (0, "M_wims_layout_fnvar: unknown purpose");
    }

  max_align = M_WIMS_ALIGN_INT;
  fp_rg = 0;
  int_rg = 0;
  off = M_WIMS_SIZE_INT;

  List_start (param_list);
  while ((param = (M_Param)List_next (param_list)))
    {
      tp = param->mtype.type;

      switch (tp)
        {
          case M_TYPE_CHAR:
          case M_TYPE_SHORT:
          case M_TYPE_INT:
          case M_TYPE_LONG:
          case M_TYPE_POINTER:
            if (int_rg < M_WIMS_MAX_FNVAR_REG)
              {
                param->mode = M_THRU_REGISTER;
                param->reg = (int_rg)++ + M_WIMS_INT_BASE;
              }
            else
              {
                param->mode = M_THRU_MEMORY;
                param->reg = -1;
              }
            break;

         case M_TYPE_LLONG:
           if(int_rg + 1 < M_WIMS_MAX_FNVAR_REG) {
             param->mode = M_THRU_REGISTER;
             param->reg = (int_rg) + M_WIMS_INT_BASE;
             int_rg += 2;
           }
           else {
             param->mode = M_THRU_MEMORY;
             param->reg = -1;
             if(int_rg < M_WIMS_MAX_FNVAR_REG)
               int_rg++;
           }
           break;

          case M_TYPE_FLOAT:
            M_assert(0, "M_wims_layout_fnvar: float params not allowed");
            if (int_rg < M_WIMS_MAX_FNVAR_REG)
              {
                param->mode = M_THRU_REGISTER;
                param->reg = (fp_rg) + 1 + M_WIMS_FLT_BASE;
                int_rg += 1;
                fp_rg  += 1;
              }
            else
              {
                param->mode = M_THRU_MEMORY;
                param->reg = -1;
              }
            break;

          case M_TYPE_DOUBLE:
            M_assert(0, "M_wims_layout_fnvar: float params not allowed");
            if (int_rg < M_WIMS_MAX_FNVAR_REG)
              {
                param->mode = M_THRU_REGISTER;
                param->reg = fp_rg + 1 + M_WIMS_FLT_BASE;
                int_rg += 2;
                fp_rg  += 1;
              }
            else
              {
                param->mode = M_THRU_MEMORY;
                param->reg = -1;
              }
            break;

          case M_TYPE_UNION:
          case M_TYPE_STRUCT:
            size = param->mtype.size;
            param->mode = M_INDIRECT_THRU_MEMORY;
            param->reg = 0;
            break;
          default:
            M_assert (0, "M_wims_layout_fnvar: argument is not promoted");
        }
      /* note, the method of calculating offsets may seem strange, but */
      /* the WIMS convention has the stack growing towards high memory   */
      /* and the parameters are referenced back from the $sp, with the */
      /* first parameter being closest to the stack pointer            */
      size = param->mtype.size;
      align = param->mtype.align;

      if (param->mtype.type == M_TYPE_UNION ||
          param->mtype.type == M_TYPE_STRUCT)
        {
          /* make sure correct size and alignment is used for
           * struct/union passed indirectly thru registers
           * if IMPACT allowed structures to be passed via registers,
           * this would not have to be done
           */
          if (param->mode == M_INDIRECT_THRU_REGISTER)
            {
              size = M_WIMS_SIZE_POINTER;
              align = M_WIMS_ALIGN_POINTER;
            }
        }
      if (align >= M_WIMS_SMALL_STRUCT_MAX &&
          param->mtype.type != M_TYPE_DOUBLE)
        /* anything larger than a 64-bit structure is passed */
        /* indirectly thru memory                            */
        align = M_WIMS_ALIGN_POINTER;
      else if (align < M_WIMS_ALIGN_INT)
        /* anything smaller that 32-bits is passed as 32-bits */
        align = M_WIMS_ALIGN_INT;

      mod = off % align;

      /* place the offset pointer to the boundary of the appropriate */
      /* data size                                                   */
      if (mod != 0)
        off += (align - mod);

      // high to low, with positive offsets
      param->offset = off;
      off += size;

      // high to low, with positive offsets
      // off += size;
      // param->offset = -off;
    }
  /* The param section must be at least certain size.
   * This is the backing store for the parameter regs. */
  if (off < MIN_PARAM_SIZE)
    off = MIN_PARAM_SIZE;

  /* now for the body of the structures... small ones first. */

  List_start (param_list);
  while ((param = (M_Param)List_next (param_list)))
    {
      tp = param->mtype.type;
      size = param->mtype.size;

      if (((tp == M_TYPE_UNION) ||
          (tp == M_TYPE_STRUCT)) &&
          (size <= M_WIMS_SMALL_STRUCT_MAX))
        {
          /* must align to a double boundry */
          align = M_WIMS_ALIGN_MAX;

          mod = off % align;

          if (mod != 0)
            off += (align - mod);

          param->paddr = off;
          off += size;
        }
    }

  /* now large ones */
  List_start (param_list);
  while ((param = (M_Param)List_next (param_list)))
    {
      tp = param->mtype.type;

      size = param->mtype.size;

      if (((tp == M_TYPE_UNION) ||
          (tp == M_TYPE_STRUCT)) &&
          (size > M_WIMS_SMALL_STRUCT_MAX))
        {

          /* must align to a double boundry */
          align = M_WIMS_ALIGN_MAX;

          mod = off % align;

          if (mod != 0)
            off += (align - mod);
         
          param->paddr = off;
          off += size;
        }
    }

  *pcount = int_rg;
  return off;               /* the total size needed */
}

/*--------------------------------------------------------------------------*/
int M_wims_fnvar_layout(int n, _M_Type *type, long int *offset, int *mode,
	                int *reg, int *paddr, char **macro, 
                        int *su_sreg, int *su_ereg,
                        int *pcount, int is_st, int purpose)
                  
               				/* need to return structure */
                 
{
    int i, max_align, off=0, rg;
    int size, align, mod, tp;


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

    // return value
    off = M_WIMS_SIZE_INT;

    max_align = M_WIMS_ALIGN_INT;
    rg = 0;

    for ( i = 0; i < n; i++ ) {
        tp = type[i].type;
        switch (tp) {
       	    case M_TYPE_CHAR:
            case M_TYPE_SHORT:
            case M_TYPE_INT:
	    case M_TYPE_LONG:
	    case M_TYPE_POINTER:
                if (rg < M_WIMS_MAX_FNVAR_REG) {
		    mode[i] = M_THRU_REGISTER;
		    reg[i] = (rg)++ + M_WIMS_INT_BASE;
		}
		else  {
		    mode[i] = M_THRU_MEMORY;
		    reg[i] = -1;
		}
		break;

           case M_TYPE_LLONG:
              if(rg + 1 < M_WIMS_MAX_FNVAR_REG) {
                mode[i] = M_THRU_REGISTER;
                reg[i] = (rg) + M_WIMS_INT_BASE;
                rg += 2;
              }
              else {
                mode[i] = M_THRU_MEMORY;
                reg[i] = -1;
                if(rg < M_WIMS_MAX_FNVAR_REG)
                  rg++;
              }
              break;

	    case M_TYPE_FLOAT:
                M_assert(0, "M_wims_fnvar_layout: float params not allowed");
		if ( rg < M_WIMS_MAX_FNVAR_REG )  {
		    mode[i] = M_THRU_REGISTER;
		    reg[i] = (rg)++ + M_WIMS_FLT_BASE;
		}
		else  {
		    mode[i] = M_THRU_MEMORY;
		    reg[i] = -1;
		}
		break;

            case M_TYPE_DOUBLE:
                M_assert(0, "M_wims_fnvar_layout: double params not allowed");
                if ( rg < M_WIMS_MAX_FNVAR_REG )  { 
		    if ( rg == 0 || rg == 2 )  {
			mode[i] = M_THRU_REGISTER;
			reg[i] = rg + 1 + M_WIMS_FLT_BASE;
			rg += 2;
		    }
		    else if ( rg == 1 ) {
			mode[i] = M_THRU_REGISTER;
			reg[i] = 3 + M_WIMS_FLT_BASE;
			rg += 3;
		    }
		    else {
		        mode[i] = M_THRU_MEMORY;
	 	    	reg[i] = -1;
		    }	
      		} 
		else {
		    mode[i] = M_THRU_MEMORY;
	 	    reg[i] = -1;
      		}
      		break;

	    case M_TYPE_UNION:
	    case M_TYPE_STRUCT:
		size = type[i].size;
		mode[i] = M_INDIRECT_THRU_MEMORY;
		reg[i] = 0;
		break;

    	    default:
      		M_assert(0, "M_fnvar_layout: argument is not promoted");
    	}

        /* note, the method of calculating offsets may seem strange, but */
	/* the WIMS convention has the stack growing towards high memory   */
	/* and the parameters are referenced back from the $sp, with the */
        /* first parameter being closest to the stack pointer		 */
        size = type[i].size;
    	align = type[i].align;

        if ( type[i].type == M_TYPE_UNION ||
	     type[i].type == M_TYPE_STRUCT ) {
        /* make sure correct size and alignment is used for struct/union */
	/* passed indirectly thru registers				 */
 	/* if IMPACT allowed structures to be passed via registers, this */
	/* would not have to be done					 */
 	    if ( mode[i] == M_INDIRECT_THRU_REGISTER )  {
	        size = M_WIMS_SIZE_POINTER;
		align = M_WIMS_ALIGN_POINTER;
            }	
	}
	if ( align >= M_WIMS_SMALL_STRUCT_MAX && type[i].type != M_TYPE_DOUBLE ) 
	    /* anything larger than a 64-bit structure is passed */
	    /* indirectly thru memory				 */
	    align = M_WIMS_ALIGN_POINTER; 
	else if ( align < M_WIMS_ALIGN_INT )
 	    /* anything smaller that 16-bits is passed as 16-bits */
	    align = M_WIMS_ALIGN_INT;

    	mod = off % align;

	/* place the offset pointer to the boundary of the appropriate */
	/* data size						       */
    	if ( mod != 0 ) 
	    off += (align - mod);

        /* now increment the offset to point to the actual location    */
	/* for this parameter					       */

        // for high to low, with positive offsets
        offset[i] = off;
        off += size; 
    }

    /* The param section must be at least certain size.
     * This is the backing store for the parameter regs. */
    if ( off < MIN_PARAM_SIZE )
        off = MIN_PARAM_SIZE;

    /* now for the body of the structures... small ones first. */
    for (i=0; i<n; i++) {
        tp = type[i].type;

        size = type[i].size;

        if ( ((tp == M_TYPE_UNION) ||
	      (tp == M_TYPE_STRUCT)) &&
	      (size <= M_WIMS_SMALL_STRUCT_MAX) ) {

            align = M_WIMS_ALIGN_MAX;	/* must align to a double boundry */

            mod = off % align;

            if ( mod!=0 )
	        off += (align - mod);

            paddr[i] = off;
            off += size;
        }
    }

    /* now large ones */
    for (i=0; i<n; i++) {
        tp = type[i].type;

        size = type[i].size;

        if ( ((tp == M_TYPE_UNION) ||
	      (tp == M_TYPE_STRUCT)) &&
	      (size > M_WIMS_SMALL_STRUCT_MAX) ) {

            align = M_WIMS_ALIGN_MAX;	/* must align to a double boundry */

            mod = off % align;

            if ( mod!=0 ) 
	        off += (align - mod);

            paddr[i] = off;
	    off += size;
        }
    }

    return off;   /* the total size needed */
}

/*--------------------------------------------------------------------------*/
int M_wims_lvar_layout(int n, _M_Type *type, long int *offset, char **base_macro)
{
    int i, max_align, off;
    int size, align, mod, tp;
    /*
     *	the LOCAL section must be max. aligned initially
     */
    max_align = M_WIMS_ALIGN_MAX;
    off = 0;
    for ( i = 0; i < n; i++ ) {
        tp = type[i].type;
        if (tp==M_TYPE_BIT_LONG) {
            M_assert(0, "M_lvar_layout: bit field not allowed");
        }
    	if (tp==M_TYPE_BIT_CHAR) {
       	    M_assert(0, "M_lvar_layout: bit field not allowed");
    	}
        size = type[i].size;
        align = type[i].align;
        mod = off % align;
        if ( mod != 0 )
      	    off += (align - mod);

        // high to low, with positive offsets
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
int M_wims_no_short_int(void) {
    return (M_WIMS_SIZE_SHORT == M_WIMS_SIZE_INT);
}
/*--------------------------------------------------------------------------*/
void M_wims_cb_label_name(char *fn, int cb, char *line, int len)
{
    sprintf(line, "cb%d%s", cb, fn);
}
/*--------------------------------------------------------------------------*/
int M_wims_is_cb_label(char *label, char *fn, int *cb)
{
    return (sscanf(label, "cb%d%s", cb, fn)==2);
}
/*--------------------------------------------------------------------------*/
void M_wims_jumptbl_label_name(char *fn, int tbl_id, char *line, int len)
{
    sprintf(line, "_$%s%s%d", fn, M_JUMPTBL_BASE_NAME, tbl_id);
}
/*--------------------------------------------------------------------------*/
/* Format for wims is: _$%sM_JUMPTBL_BASE_NAME%d, where %s is the func name */
int M_wims_is_jumptbl_label(char *label, char *fn, int *tbl_id)
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
int M_wims_structure_pointer(int purpose)
{
    return M_WIMS_RET_ST;
}
/*--------------------------------------------------------------------------*/
int M_wims_return_register(int type, int purpose)
{
    switch (type) {
  	case M_TYPE_INT:	return M_WIMS_RET_I32;
  	case M_TYPE_LONG:	return M_WIMS_RET_I32;
  	case M_TYPE_LLONG:	return M_WIMS_RET_I32;
  	case M_TYPE_FLOAT:	{M_assert(0, "return type cannot be float"); return 0;}
  	case M_TYPE_DOUBLE:	{M_assert(0, "return type cannot be double"); return 0;}
  	default:		return M_WIMS_RET_I32;
    }
}
/*--------------------------------------------------------------------------*/
/*
char *M_wims_fn_label_name(label)
*/
char *M_wims_fn_label_name(char *label, int (*is_func) (char *is_func_label))
{
    static char fn_label[1024];

    if ( (*is_func)(label) )  {
        sprintf(fn_label,"$fn_%s",label);
        return(fn_label);
    }
    else 
        return(label);
}

char *M_wims_fn_name_from_label(char *label)
{
    if ( !strncmp(label,"_$fn",4) )
	return(label+4);
    else
	return(label);
}

/*--------------------------------------------------------------------------*/
int M_wims_fragile_macro(int macro_value)
{
    switch (M_model)  {
	case M_WIMS_16:
	case M_WIMS_24:
	    switch(macro_value)  {
		case L_MAC_SP:
            	case L_MAC_FP:
            	case L_MAC_LV:		/* added SAM 10-94 */
            	case L_MAC_IP:		/* added SAM 10-94 */
            	case L_MAC_OP:		/* added SAM 10-94 */
            	case L_MAC_LOCAL_SIZE:
            	case L_MAC_PARAM_SIZE:
            	case L_MAC_SWAP_SIZE:       
				return 0;	
	        default:	return(1);
	    }
	default:
	    M_assert (0, "M_wims_fragile_macro:  Illegal model specified!");
	    return (0);
    }
}

/*--------------------------------------------------------------------------*/
int M_wims_subroutine_call(int opc)
{
    switch (M_model)  {
	case M_WIMS_16:
	case M_WIMS_24:
	    return ((opc==Lop_JSR) || (opc==Lop_JSR_FS) ||
                    (opc==Lop_MUL) || (opc==Lop_MUL_U) ||
                    (opc==Lop_DIV) || (opc==Lop_DIV_U) ||
                    (opc==Lop_REM) || (opc==Lop_REM_U));
	default:
	    M_assert (0, "M_wims_subroutine_call:  Illegal model specified!");
	    return (0);
    }
}
/*--------------------------------------------------------------------------*/
/*
 * Declare code generator specific macro registers to the front end parser.
 */
void M_define_macros_wims(STRING_Symbol_Table *sym_tbl)
{
    M_add_symbol(sym_tbl, "$true_sp", WIMS_MAC_TRUE_SP);
    /* 1 if leaf function, 0 if non-leaf */
    M_add_symbol(sym_tbl, "$leaf", WIMS_MAC_LEAF);
    /* total alloc requirements */
    M_add_symbol(sym_tbl, "$alloc_size", WIMS_MAC_ALLOC);
    /* number of integer and float callee saved registers used */
    M_add_symbol(sym_tbl, "$callee_i_regs", WIMS_MAC_CALLEE_I);
    M_add_symbol(sym_tbl, "gr2", WIMS_MAC_RETADDR);
  
}

char *M_get_macro_name_wims(int id)
{
    switch (id) {
  	case WIMS_MAC_LEAF:              return "$leaf";
  	case WIMS_MAC_ALLOC:             return "$alloc_size";
  	case WIMS_MAC_CALLEE_I:          return "$callee_i_regs";
  	case WIMS_MAC_TRUE_SP:           return "$true_sp";
        case WIMS_MAC_RETADDR:           return "gr2";

  	default:                       
				      return "?";
    }
}

void M_define_opcode_name_wims(STRING_Symbol_Table * sym_tbl)
{
}

char *M_get_opcode_name_wims(int id)
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
int M_oper_supported_in_arch_wims(int opc)
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
 */

int M_num_oper_required_for_wims(L_Oper *oper, char *name)
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
	    if ( indexed_memory_op(oper) || has_label_operand(oper) )
		return(2);
	    else
		return(1);
	    
	case Lop_LD_UC: 
	case Lop_LD_C: 
	case Lop_LD_UC2: 
	case Lop_LD_C2: 
	case Lop_LD_I: 
	    if (has_label_operand(oper))
		return (2);
	    else
		return(1);
	      	
  	case Lop_ST_F: 
	case Lop_ST_F2: 
	    if (has_label_operand(oper))
		return (2);
	    else
		return(1);

	case Lop_LD_F: 
	case Lop_LD_F2:
	    if (has_label_operand(oper))
		return (2);
	    else
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

int M_is_stack_operand_wims(L_Operand *operand)
{
    if (L_is_macro(operand) &&
	 ( operand->value.mac == L_MAC_SP ||
	   operand->value.mac == L_MAC_FP ||
	   operand->value.mac == WIMS_MAC_TRUE_SP ||
	   operand->value.mac == L_MAC_SAFE_MEM ||
	   operand->value.mac == L_MAC_P12 ||
	   operand->value.mac == L_MAC_IP ||
	   operand->value.mac == L_MAC_OP ||
	   operand->value.mac == L_MAC_RS ||
	   operand->value.mac == L_MAC_LV ) )
	return(1);
    
    return(0);
}

int M_is_unsafe_macro_wims(L_Operand *operand)
{
    if (!L_is_macro(operand))
	return(0);
    
    switch (operand->value.mac)  {
        case L_MAC_FP:
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

int M_operand_type_wims(L_Operand *operand)
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
            M_assert(0,"M_operand_type_wims: Unknown type");
	    return (0);
    }
}

int M_conflicting_operands_wims(L_Operand *operand, L_Operand **conflict_array, int len, int prepass)
{
    int right=0, left=0;

    if ( prepass && (!L_is_macro(operand)) ) {
        conflict_array[0] = L_copy_operand(operand);
        return(1);
    }
    if (L_is_macro(operand)) {
	switch (operand->value.mac)  {
	    case L_MAC_SP:
	    case WIMS_MAC_TRUE_SP:
	    case L_MAC_SAFE_MEM:
	      conflict_array[0] = L_new_macro_operand(L_MAC_SP, L_CTYPE_INT, L_PTYPE_NULL);
	      conflict_array[1] = L_new_macro_operand(WIMS_MAC_TRUE_SP, L_CTYPE_INT, L_PTYPE_NULL);
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
	M_assert(0,"Lwims_conflicting_operands: unsupported operand type");
    
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
      M_assert(0,"Lwims_conflicting_operands: unsupported operand type");
      return (0);
    }
}

int M_num_registers_wims(int ctype)
{
  if ((M_model==M_WIMS_16) || (M_model==M_WIMS_24)) {
    /* Dont know if these numbers are correct, just guesses.. */
    switch (ctype) {
      case L_CTYPE_INT:
        return (64);	/* over estimate here a bit */
      case L_CTYPE_FLOAT:
        M_assert(0, "M_num_registers_wims: WIMS does not support float data type");
        return (64);
      case L_CTYPE_DOUBLE:
        M_assert(0, "M_num_registers_wims: WIMS does not support double data type");
        return (32);
      default:
        return (0);
    }
  } else {
    M_assert(0, "M_num_registers_wims: unsupported model");
  }
  return (0);
}

int
M_wims_layout_retvar (M_Param param, int purpose)
{
  if (param->mtype.type & (M_TYPE_STRUCT | M_TYPE_UNION))
    param->mode = M_INDIRECT_THRU_MEMORY;
  else
    param->mode = M_THRU_MEMORY;
  return (param->mode == M_INDIRECT_THRU_MEMORY);
}
