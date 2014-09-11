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
/*===========================================================================
 *      File :          constant.c
 *      Description :   Adjust constants for sign and zero extension
 *      Creation Date : September 1997
 *      Author :        Robert McGowan
 *
 *==========================================================================*/

/* 10/29/02 REK Adding config.h */
#include <config.h>
#include <Lcode/l_main.h>

#ifdef IT64BIT
#define LLONG_SIGN_BIT          ULLCONST(0x8000000000000000)
#define UNSIGNED_LLONG_MAX      ULLCONST(0xFFFFFFFFFFFFFFFF)
#define SIGNED_LLONG_POS_MAX    LLCONST(0x7FFFFFFFFFFFFFFF)
#define SIGNED_LLONG_NEG_MAX    LLONG_SIGN_BIT
#endif

#define INT_SIGN_BIT          0x80000000
#define UNSIGNED_INT_MAX      0xFFFFFFFF
#define SIGNED_INT_POS_MAX    0x7FFFFFFF
#define SIGNED_INT_NEG_MAX    INT_SIGN_BIT

#define SHORT_SIGN_BIT        0x8000
#define UNSIGNED_SHORT_MAX    0xFFFF
#define SIGNED_SHORT_POS_MAX  0x7FFF
#define SIGNED_SHORT_NEG_MAX  SHORT_SIGN_BIT

#define CHAR_SIGN_BIT         0x80
#define UNSIGNED_CHAR_MAX     0xFF
#define SIGNED_CHAR_POS_MAX   0x7F
#define SIGNED_CHAR_NEG_MAX   CHAR_SIGN_BIT

/* prototypes */
static ITintmax L_convert_immed_operand (unsigned char old_ctype,
                                         unsigned char new_ctype,
                                         ITintmax value);

/****************************************************************************
 *
 * routine: L_copy_immed_operand()
 * purpose: Copy the immediate in the operand and adjust the value of
 *          immediate to account for a change in the ctype.
 *          This only works for char, short, and int right now.
 *          This routine creates a new operand.
 * input: new_ctype - The desired ctype
 *        old_ctype - indictes how the given immediate should be interpreted
 *        from_operand - an operand with an immediate
 * output: 
 * returns:
 * modified: Bob McGowan - 9/97 - created
 * note:
 *-------------------------------------------------------------------------*/

L_Operand *
L_copy_immed_operand (unsigned char new_ctype,
                      unsigned char old_ctype, L_Operand * from_operand)
{
  ITintmax value;

  value = L_convert_immed_operand (old_ctype, new_ctype,
                                   from_operand->value.i);
  return (L_new_int_operand (value, new_ctype));
}


/****************************************************************************
 *
 * routine: L_convert_immed_operand()
 * purpose: Adjust the value of immediate so that it valid with respect to
 *          the new ctype.
 *          This only works for char, short, and int right now.
 * input: new_ctype - The desired ctype.
 *        old_ctype - indictes how the given immediate should be interpreted.
 *        value - The existing immediate.
 * input:
 * output: 
 * returns:
 * modified: Bob McGowan - 9/97 - created
 * note:
 *-------------------------------------------------------------------------*/

static ITintmax
L_convert_immed_operand (unsigned char old_ctype,
                         unsigned char new_ctype, ITintmax value)
{
  if (L_is_ctype_unsigned_direct (new_ctype))
    {
#ifdef IT64BIT
      if (L_is_size_llong_direct (old_ctype))
        return (value & UNSIGNED_LLONG_MAX);
      else
#endif
      if (L_is_size_int_direct (old_ctype))
        return (value & UNSIGNED_INT_MAX);
      else if (L_is_size_char_direct (old_ctype))
        return (value & UNSIGNED_CHAR_MAX);
      else if (L_is_size_short_direct (old_ctype))
        return (value & UNSIGNED_SHORT_MAX);
      else
        L_punt ("L_copy_immed_operand: Unknown unsigned ctype");
    }

  /* The new ctype is signed */

  if (L_is_size_llong_direct (old_ctype))
    {
      return (value);
    }
  else
    {
      if (L_is_size_int_direct (old_ctype))
        {
          if (value & INT_SIGN_BIT)
            value = (value == INT_SIGN_BIT) ?
              SIGNED_INT_NEG_MAX : -((-value) & SIGNED_INT_POS_MAX);
          return (value);
        }
      else if (L_is_size_char_direct (old_ctype))
        {
          if (value & CHAR_SIGN_BIT)
            value = (value == CHAR_SIGN_BIT) ?
              SIGNED_CHAR_NEG_MAX : -((-value) & SIGNED_CHAR_POS_MAX);
          return (value);
        }
      else if (L_is_size_short_direct (old_ctype))
        {
          if (value & SHORT_SIGN_BIT)
            value = (value == SHORT_SIGN_BIT) ?
              SIGNED_SHORT_NEG_MAX : -((-value) & SIGNED_SHORT_POS_MAX);
          return (value);
        }
    }

  L_punt ("L_copy_immed_operand: Unknown ctype");

  return (0);                   /* not reachable */
}
