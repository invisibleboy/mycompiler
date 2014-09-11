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
 *      File :          l_flagsio.c
 *      Description :   Bit field flag encoding and decoding routines
 *      Date :          April 1996
 *      Author :        Richard Hank, Wen-mei Hwu
\*****************************************************************************/
/* 10/29/02 REK Adding config.h */
#include <config.h>
#include <string.h>
#include <Lcode/l_flags.h>
#include <Lcode/l_error.h>

int
L_oper_flags_string_to_int (char *flags_string)
{
  int i;
  int num_flags;
  int flags;

  flags = 0;
  num_flags = strlen (flags_string);

  for (i = 0; i < num_flags; i++)
    {
      switch (flags_string[i])
        {
        case 'C':
        case 'c':
          flags = L_SET_BIT_FLAG (flags, L_OPER_CHECK);
          break;
        case 'F':
        case 'f':
          flags = L_SET_BIT_FLAG (flags, L_OPER_SAFE_PEI);
          break;
        case 'L':
        case 'l':
          flags = L_SET_BIT_FLAG (flags, L_OPER_LABEL_REFERENCE);
          break;
        case 'P':
        case 'p':
          flags = L_SET_BIT_FLAG (flags, L_OPER_PROMOTED);
          break;
        case 'Q':
        case 'q':
          flags = L_SET_BIT_FLAG (flags, L_OPER_SQUASHING);
          break;
        case 'D':
        case 'd':
          flags = L_SET_BIT_FLAG (flags, L_OPER_DATA_SPECULATIVE);
          break;
        case 'R':
        case 'r':
          flags = L_SET_BIT_FLAG (flags, L_OPER_SPILL_CODE);
          break;
        case 'E':
        case 'e':
          flags = L_SET_BIT_FLAG (flags, L_OPER_SIDE_EFFECT_FREE);
          break;
        case 'S':
        case 's':
          flags = L_SET_BIT_FLAG (flags, L_OPER_SPECULATIVE);
          break;
        case 'M':
        case 'm':
          flags = L_SET_BIT_FLAG (flags, L_OPER_MASK_PE);
          break;
        case 'X':
        case 'x':
          flags = L_SET_BIT_FLAG (flags, L_OPER_PROBE_MARK);
          break;
        case 'Y':
        case 'y':
          flags = L_SET_BIT_FLAG (flags, L_OPER_SYNC);
          break;
        case '?':
          flags = L_SET_BIT_FLAG (flags, L_OPER_PROCESSOR_SPECIFIC);
          break;
        case 'V':
        case 'v':
          flags = L_SET_BIT_FLAG (flags, L_OPER_VOLATILE);
          break;
        case 'K':
        case 'k':
          flags = L_SET_BIT_FLAG (flags, L_OPER_STACK_REFERENCE);
          break;
        case 'T':
        case 't':
          flags = L_SET_BIT_FLAG (flags, L_OPER_ROTATE_REGISTERS);
          break;
        case 'N':
        case 'n':
          flags = L_SET_BIT_FLAG (flags, L_OPER_NO_SPECULATION);
          break;
        case 'A':
        case 'a':
          flags = L_SET_BIT_FLAG (flags, L_OPER_SUPER_SPECULATION);
          break;
        default:
          L_punt ("L_read_oper_flags: invalid flag <%c> in current oper.",
                  flags_string[i]);
        }
    }
  return (flags);
}

int
L_oper_flags_to_string (char *flag_string, int oper_flags)
{
  int flags;
  int num_flags;

  num_flags = 0;
  flags = L_CLR_BIT_FLAG (oper_flags, L_OPER_HIDDEN_FLAGS);

  if (flags != 0)
    {
      if (L_EXTRACT_BIT_VAL (flags, L_OPER_CHECK))
        flag_string[num_flags++] = 'C';
      if (L_EXTRACT_BIT_VAL (flags, L_OPER_LABEL_REFERENCE))
        flag_string[num_flags++] = 'L';
      if (L_EXTRACT_BIT_VAL (flags, L_OPER_PROMOTED))
        flag_string[num_flags++] = 'P';
      if (L_EXTRACT_BIT_VAL (flags, L_OPER_SQUASHING))
        flag_string[num_flags++] = 'Q';
      if (L_EXTRACT_BIT_VAL (flags, L_OPER_DATA_SPECULATIVE))
        flag_string[num_flags++] = 'D';
      if (L_EXTRACT_BIT_VAL (flags, L_OPER_SPILL_CODE))
        flag_string[num_flags++] = 'R';
      if (L_EXTRACT_BIT_VAL (flags, L_OPER_SIDE_EFFECT_FREE))
        flag_string[num_flags++] = 'E';
      if (L_EXTRACT_BIT_VAL (flags, L_OPER_SPECULATIVE))
        flag_string[num_flags++] = 'S';
      if (L_EXTRACT_BIT_VAL (flags, L_OPER_MASK_PE))
        flag_string[num_flags++] = 'M';
      if (L_EXTRACT_BIT_VAL (flags, L_OPER_SAFE_PEI))
        flag_string[num_flags++] = 'F';
      if (L_EXTRACT_BIT_VAL (flags, L_OPER_PROBE_MARK))
        flag_string[num_flags++] = 'X';
      if (L_EXTRACT_BIT_VAL (flags, L_OPER_SYNC))
        flag_string[num_flags++] = 'Y';
      if (L_EXTRACT_BIT_VAL (flags, L_OPER_PROCESSOR_SPECIFIC))
        flag_string[num_flags++] = '?';
      if (L_EXTRACT_BIT_VAL (flags, L_OPER_ROTATE_REGISTERS))
        flag_string[num_flags++] = 'T';
      if (L_EXTRACT_BIT_VAL (flags, L_OPER_VOLATILE))
        flag_string[num_flags++] = 'V';
      if (L_EXTRACT_BIT_VAL (flags, L_OPER_STACK_REFERENCE))
        flag_string[num_flags++] = 'K';
      if (L_EXTRACT_BIT_VAL (flags, L_OPER_NO_SPECULATION))
        flag_string[num_flags++] = 'N';
      if (L_EXTRACT_BIT_VAL (flags, L_OPER_SUPER_SPECULATION))
        flag_string[num_flags++] = 'A';
    }
  flag_string[num_flags] = '\0';

  return (num_flags);
}

int
L_cb_flags_string_to_int (char *flags_string)
{
  int i;
  int num_flags;
  int flags;

  flags = 0;
  num_flags = strlen (flags_string);

  for (i = 0; i < num_flags; i++)
    {
      switch (flags_string[i])
        {
          /* Rememeber to include lower and upper case! */
        case 'E':
        case 'e':
          flags = L_SET_BIT_FLAG (flags, L_CB_ENTRANCE_BOUNDARY);
          break;
        case 'H':
        case 'h':
          flags = L_SET_BIT_FLAG (flags, L_CB_HYPERBLOCK);
          break;
        case 'I':
        case 'i':
          flags = L_SET_BIT_FLAG (flags, L_CB_EPILOGUE);
          break;
        case 'N':
        case 'n':
          flags = L_SET_BIT_FLAG (flags, L_CB_HYPERBLOCK_NO_FALLTHRU);
          break;
        case 'P':
        case 'p':
          flags = L_SET_BIT_FLAG (flags, L_CB_SOFTPIPE);
          break;
        case 'R':
        case 'r':
          flags = L_SET_BIT_FLAG (flags, L_CB_PROLOGUE);
          break;
        case 'S':
        case 's':
          flags = L_SET_BIT_FLAG (flags, L_CB_SUPERBLOCK);
          break;
        case 'T':
        case 't':
          flags = L_SET_BIT_FLAG (flags, L_CB_ROT_REG_ALLOCATED);
          break;
        case 'U':
        case 'u':
          flags = L_SET_BIT_FLAG (flags, L_CB_UNROLLED);
          break;
        case 'V':
        case 'v':
          flags = L_SET_BIT_FLAG (flags, L_CB_VIOLATES_LC_SEMANTICS);
          break;
        case 'X':
        case 'x':
          flags = L_SET_BIT_FLAG (flags, L_CB_EXIT_BOUNDARY);
          break;
        default:
          L_punt ("L_read_cb: invalid flag in cb of <%c>", flags_string[i]);
        }
    }
  return (flags);
}

int
L_cb_flags_to_string (char *flag_string, int cb_flags)
{
  int flags;
  int num_flags;

  num_flags = 0;
  flags = L_CLR_BIT_FLAG (cb_flags, L_CB_HIDDEN_FLAGS);

  if (flags != 0)
    {
      if (L_EXTRACT_BIT_VAL (flags, L_CB_ENTRANCE_BOUNDARY))
        flag_string[num_flags++] = 'E';
      if (L_EXTRACT_BIT_VAL (flags, L_CB_HYPERBLOCK))
        flag_string[num_flags++] = 'H';
      if (L_EXTRACT_BIT_VAL (flags, L_CB_EPILOGUE))
        flag_string[num_flags++] = 'I';
      if (L_EXTRACT_BIT_VAL (flags, L_CB_HYPERBLOCK_NO_FALLTHRU))
        flag_string[num_flags++] = 'N';
      if (L_EXTRACT_BIT_VAL (flags, L_CB_SOFTPIPE))
        flag_string[num_flags++] = 'P';
      if (L_EXTRACT_BIT_VAL (flags, L_CB_PROLOGUE))
        flag_string[num_flags++] = 'R';
      if (L_EXTRACT_BIT_VAL (flags, L_CB_SUPERBLOCK))
        flag_string[num_flags++] = 'S';
      if (L_EXTRACT_BIT_VAL (flags, L_CB_ROT_REG_ALLOCATED))
        flag_string[num_flags++] = 'T';
      if (L_EXTRACT_BIT_VAL (flags, L_CB_UNROLLED))
        flag_string[num_flags++] = 'U';
      if (L_EXTRACT_BIT_VAL (flags, L_CB_VIOLATES_LC_SEMANTICS))
        flag_string[num_flags++] = 'V';
      if (L_EXTRACT_BIT_VAL (flags, L_CB_EXIT_BOUNDARY))
        flag_string[num_flags++] = 'X';
    }
  flag_string[num_flags] = '\0';

  return (num_flags);
}

int
L_func_flags_string_to_int (char *flags_string)
{
  int i;
  int num_flags;
  int flags;

  flags = 0;
  num_flags = strlen (flags_string);

  for (i = 0; i < num_flags; i++)
    {
      switch (flags_string[i])
        {
          /* Rememeber to include lower and upper case! */
        case 'C':
        case 'c':
          flags = L_SET_BIT_FLAG (flags, L_FUNC_COMPILATION_COMPLETE);
          break;
        case 'D':
        case 'd':
          flags = L_SET_BIT_FLAG (flags, L_FUNC_SCHEDULED);
          break;
        case 'E':
        case 'e':
          flags = L_SET_BIT_FLAG (flags, L_FUNC_SIDE_EFFECT_FREE);
          break;
        case 'H':
        case 'h':
          flags = L_SET_BIT_FLAG (flags, L_FUNC_HYPERBLOCK);
          break;
        case 'L':
        case 'l':
          flags = L_SET_BIT_FLAG (flags, L_FUNC_LEAF);
          break;
        case 'R':
        case 'r':
          flags = L_SET_BIT_FLAG (flags, L_FUNC_REGISTER_ALLOCATED);
          break;
        case 'S':
        case 's':
          flags = L_SET_BIT_FLAG (flags, L_FUNC_SUPERBLOCK);
          break;
        case 'A':
        case 'a':
          flags = L_SET_BIT_FLAG (flags, L_FUNC_PRED_REGS_IN_ATTR);
          break;
        case 'M':
        case 'm':
          flags = L_SET_BIT_FLAG (flags, L_FUNC_MASK_PE);
          break;
        case 'P':
        case 'p':
          flags = L_SET_BIT_FLAG (flags, L_FUNC_CC_IN_PREDICATE_REGS);
          break;
        case 'T':
        case 't':
          flags = L_SET_BIT_FLAG (flags, L_FUNC_ROT_REG_ALLOCATED);
          break;
        default:
          L_punt ("L_func_flag_string_to_int: invalid function flag <%c>",
                  flags_string[i]);
        }
    }
  return (flags);
}

int
L_func_flags_to_string (char *flag_string, int func_flags)
{
  int flags;
  int num_flags;

  num_flags = 0;
  flags = L_CLR_BIT_FLAG (func_flags, L_FUNC_HIDDEN_FLAGS);

  if (flags != 0)
    {
      if (L_EXTRACT_BIT_VAL (flags, L_FUNC_COMPILATION_COMPLETE))
        flag_string[num_flags++] = 'C';
      if (L_EXTRACT_BIT_VAL (flags, L_FUNC_SCHEDULED))
        flag_string[num_flags++] = 'D';
      if (L_EXTRACT_BIT_VAL (flags, L_FUNC_REGISTER_ALLOCATED))
        flag_string[num_flags++] = 'R';
      if (L_EXTRACT_BIT_VAL (flags, L_FUNC_SIDE_EFFECT_FREE))
        flag_string[num_flags++] = 'E';
      if (L_EXTRACT_BIT_VAL (flags, L_FUNC_HYPERBLOCK))
        flag_string[num_flags++] = 'H';
      if (L_EXTRACT_BIT_VAL (flags, L_FUNC_LEAF))
        flag_string[num_flags++] = 'L';
      if (L_EXTRACT_BIT_VAL (flags, L_FUNC_SUPERBLOCK))
        flag_string[num_flags++] = 'S';
      if (L_EXTRACT_BIT_VAL (flags, L_FUNC_PRED_REGS_IN_ATTR))
        flag_string[num_flags++] = 'A';
      if (L_EXTRACT_BIT_VAL (flags, L_FUNC_MASK_PE))
        flag_string[num_flags++] = 'M';
      if (L_EXTRACT_BIT_VAL (flags, L_FUNC_CC_IN_PREDICATE_REGS))
        flag_string[num_flags++] = 'P';
      if (L_EXTRACT_BIT_VAL (flags, L_FUNC_ROT_REG_ALLOCATED))
        flag_string[num_flags++] = 'T';
    }
  flag_string[num_flags] = '\0';

  return (num_flags);
}
