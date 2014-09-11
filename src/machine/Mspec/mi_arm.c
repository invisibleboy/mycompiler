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
 *	Auth :  Nate Clark
 *
 *    Modified : modified from code by Richard Hank and Wen-mei Hwu,
 *               which modified it from m_spec.c code by Pohua Paul Chang.
 *
 *==========================================================================*/
#include <stdio.h>
#ifdef M_ARM_FOR_HCODE
#include <Hcode/h_ccode.h>
#endif
#include <Lcode/l_main.h>
#include "m_arm.h"

/*--------------------------------------------------------------------------*/
#define M_ARM_SIZE_VOID        0
#define M_ARM_SIZE_BIT         1
#define M_ARM_SIZE_CHAR        8
#define M_ARM_SIZE_SHORT       16
#define M_ARM_SIZE_INT         32
#define M_ARM_SIZE_LONG        32
#define M_ARM_SIZE_FLOAT       32
#define M_ARM_SIZE_DOUBLE      64
#define M_ARM_SIZE_POINTER     32
#define M_ARM_SIZE_LLONG       64
#define M_ARM_SIZE_UNION       -1
#define M_ARM_SIZE_STRUCT      -1
#define M_ARM_SIZE_BLOCK       -1
#define M_ARM_SIZE_MAX         64

#define M_ARM_ALIGN_VOID       -1
#define M_ARM_ALIGN_BIT        1
#define M_ARM_ALIGN_CHAR       8
#define M_ARM_ALIGN_SHORT      16
#define M_ARM_ALIGN_INT        32
#define M_ARM_ALIGN_LONG       32
#define M_ARM_ALIGN_FLOAT      32
#define M_ARM_ALIGN_DOUBLE     64
#define M_ARM_ALIGN_POINTER    32
#define M_ARM_ALIGN_LLONG      64
#define M_ARM_ALIGN_UNION      -1      /* depends on the field */
#define M_ARM_ALIGN_STRUCT     -1
#define M_ARM_ALIGN_BLOCK      -1
#define M_ARM_ALIGN_MAX        64

int M_arm_type_size(int mtype)
{
    switch (mtype) {
        case M_TYPE_VOID:               return M_ARM_SIZE_VOID;
        case M_TYPE_BIT_LLONG:          return M_ARM_SIZE_BIT;
        case M_TYPE_BIT_LONG:           return M_ARM_SIZE_BIT;
        case M_TYPE_BIT_SHORT:          return M_ARM_SIZE_BIT;
        case M_TYPE_BIT_CHAR:           return M_ARM_SIZE_BIT;
        case M_TYPE_CHAR:               return M_ARM_SIZE_CHAR;
        case M_TYPE_SHORT:              return M_ARM_SIZE_SHORT;
        case M_TYPE_INT:                return M_ARM_SIZE_INT;
        case M_TYPE_LONG:               return M_ARM_SIZE_LONG;
        case M_TYPE_LLONG:              return M_ARM_SIZE_LLONG;
        case M_TYPE_FLOAT:              return M_ARM_SIZE_FLOAT;
        case M_TYPE_DOUBLE:             return M_ARM_SIZE_DOUBLE;
        case M_TYPE_POINTER:            return M_ARM_SIZE_POINTER;
        case M_TYPE_UNION:              return M_ARM_SIZE_UNION;
        case M_TYPE_STRUCT:             return M_ARM_SIZE_STRUCT;
        case M_TYPE_BLOCK:              return M_ARM_SIZE_BLOCK;
        default:
            return -1;
    }
}

int M_arm_type_align(int mtype)
{
    switch (mtype) {
        case M_TYPE_VOID:               return M_ARM_ALIGN_VOID;
        case M_TYPE_BIT_LLONG:          return M_ARM_SIZE_BIT;
        case M_TYPE_BIT_LONG:           return M_ARM_ALIGN_BIT;
        case M_TYPE_BIT_SHORT:          return M_ARM_ALIGN_BIT;
        case M_TYPE_BIT_CHAR:           return M_ARM_ALIGN_BIT;
        case M_TYPE_CHAR:               return M_ARM_ALIGN_CHAR;
        case M_TYPE_SHORT:              return M_ARM_ALIGN_SHORT;
        case M_TYPE_INT:                return M_ARM_ALIGN_INT;
        case M_TYPE_LONG:               return M_ARM_ALIGN_LONG;
        case M_TYPE_LLONG:              return M_ARM_ALIGN_LLONG;
        case M_TYPE_FLOAT:              return M_ARM_ALIGN_FLOAT;
        case M_TYPE_DOUBLE:             return M_ARM_ALIGN_DOUBLE;
        case M_TYPE_POINTER:            return M_ARM_ALIGN_POINTER;
        case M_TYPE_UNION:              return M_ARM_ALIGN_UNION;
        case M_TYPE_STRUCT:             return M_ARM_ALIGN_STRUCT;
        case M_TYPE_BLOCK:              return M_ARM_ALIGN_BLOCK;
        default:
            return -1;
    }
}

void M_arm_bit_llong (M_Type type, int n)
{
  type->type = M_TYPE_BIT_LLONG;
  type->unsign = 1;
  type->align = M_ARM_ALIGN_BIT;
  type->size = n * M_ARM_SIZE_BIT;
  type->nbits = n * M_ARM_SIZE_BIT;
  M_assert ((n <= 64),
	    "M_arm_bit_long: do not allow bit field of more than 64 bits");
}

void M_arm_char(M_Type type, int unsign)
{
    type->type = M_TYPE_CHAR;
    type->unsign = unsign;
    type->align = M_ARM_ALIGN_CHAR;
    type->size = M_ARM_SIZE_CHAR;
    type->nbits = M_ARM_SIZE_CHAR;
}

void M_arm_short(M_Type type, int unsign)
{
    type->type = M_TYPE_SHORT;
    type->unsign = unsign;
    type->align = M_ARM_ALIGN_SHORT;
    type->size = M_ARM_SIZE_SHORT;
    type->nbits = M_ARM_SIZE_SHORT;
}

void M_arm_int(M_Type type, int unsign)
{
    type->type = M_TYPE_INT;
    type->unsign = unsign;
    type->align = M_ARM_ALIGN_INT;
    type->size = M_ARM_SIZE_INT;
    type->nbits = M_ARM_SIZE_INT;
}

void M_arm_long(M_Type type, int unsign)
{
    type->type = M_TYPE_LONG;
    type->unsign = unsign;
    type->align = M_ARM_ALIGN_LONG;
    type->size = M_ARM_SIZE_LONG;
    type->nbits = M_ARM_SIZE_LONG;
}

void M_arm_llong (M_Type type, int unsign)
{
  type->type = M_TYPE_LLONG;
  type->unsign = unsign;
  type->align = M_ARM_ALIGN_LLONG;
  type->size = M_ARM_SIZE_LLONG;
  type->nbits = M_ARM_SIZE_LLONG;
}

int M_arm_layout_order(void) {
    return M_BIG_ENDIAN;
}

void M_set_model_arm(char *model_name)
{
    if ( !strcasecmp(model_name, "1.01") )
	M_model = M_ARM_1_01;
    else
	M_assert (0, "M_set_model_arm:  Illegal model specified!");
}

