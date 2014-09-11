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
 *      File :          l_stat.c
 *      Description :   compute execution estimated execution statistics
 *      Creation Date : July, 1990
 *      Author :        Pohua Chang, Scott Mahlke, Wen-mei Hwu
 *
 *==========================================================================*/
/* 10/29/02 REK Adding config.h */
#include <config.h>
#include <Lcode/l_main.h>

#define MAX_FN  1700

extern int Lcode_print_stat;    /* defined in l_codegen.c */
extern int Lcode_unit_time;

struct Fstat
{
  char *fn_name;
  double fn_weight;             /* number of times being called */

  double bb_weight_0;           /* total accumulated weight of bb */
  double bb_weight_10;
  double bb_weight_100;
  double bb_weight_1000;
  double bb_weight_10000;
  int bb_count_0;               /* static number of bb */
  int bb_count_10;
  int bb_count_100;
  int bb_count_1000;
  int bb_count_10000;

  double op_weight_0;           /* total accumulated weight of oper */
  double op_weight_10;
  double op_weight_100;
  double op_weight_1000;
  double op_weight_10000;
  int op_count_0;               /* static number of oper */
  int op_count_10;
  int op_count_100;
  int op_count_1000;
  int op_count_10000;

  double br_weight;
  double st_weight;
  double ld_weight;
  double alu_weight;
  double fpu_weight;
  double other_weight;
  int br_count;
  int st_count;
  int ld_count;
  int alu_count;
  int fpu_count;
  int other_count;
};

static struct Fstat *fstat = NULL;
static int n_fstat = 0;

/*-----------------------------------------------------------------*/
static void
print_header (FILE * F)
{
  fprintf (F, "/** Lcode : approximate weighted execution cycle count **/\n");
}

static void
print_fn_stat (FILE * F, struct Fstat *stat)
{
  fprintf (F, "(function %f %s)\n", stat->fn_weight, stat->fn_name);
  fprintf (F, "  (bbw0 %f)(bbw10 %f)(bbw100 %f)(bbw1000 %f)(bbw10000 %f)\n",
           stat->bb_weight_0,
           stat->bb_weight_10,
           stat->bb_weight_100, stat->bb_weight_1000, stat->bb_weight_10000);
  fprintf (F, "  (bbc0 %d)(bbc10 %d)(bbc100 %d)(bbc1000 %d)(bbc10000 %d)\n",
           stat->bb_count_0,
           stat->bb_count_10,
           stat->bb_count_100, stat->bb_count_1000, stat->bb_count_10000);
  fprintf (F, "  (opw0 %f)(opw10 %f)(opw100 %f)(opw1000 %f)(opw10000 %f)\n",
           stat->op_weight_0,
           stat->op_weight_10,
           stat->op_weight_100, stat->op_weight_1000, stat->op_weight_10000);
  fprintf (F, "  (opc0 %d)(opc10 %d)(opc100 %d)(opc1000 %d)(opc10000 %d)\n",
           stat->op_count_0,
           stat->op_count_10,
           stat->op_count_100, stat->op_count_1000, stat->op_count_10000);
  fprintf (F, "  (wbr %f)(wst %f)(wld %f)(walu %f)(wfpu %f)(wother %f)\n",
           stat->br_weight,
           stat->st_weight,
           stat->ld_weight,
           stat->alu_weight, stat->fpu_weight, stat->other_weight);
  fprintf (F, "  (br %d)(st %d)(ld %d)(alu %d)(fpu %d)(other %d)\n",
           stat->br_count,
           stat->st_count,
           stat->ld_count,
           stat->alu_count, stat->fpu_count, stat->other_count);
  fprintf (F, "\n");
}
/*-----------------------------------------------------------------*/
static double
opcode_weight (L_Oper * oper)
{
  int opc = oper->opc;
  if (Lcode_unit_time)
    {
      switch (opc)
        {
        case Lop_DEFINE:
        case Lop_ALLOC:
          return 0;
        default:
          return 1;
        }
    }
  switch (opc)
    {
    case Lop_JSR:
    case Lop_JSR_FS:
    case Lop_RTS:
    case Lop_RTS_FS:
      return 2;
    case Lop_PROLOGUE:
    case Lop_EPILOGUE:
      return 10;
    case Lop_DEFINE:
    case Lop_ALLOC:
      return 0;
    case Lop_MUL:
    case Lop_MUL_U:
      return 6;
    case Lop_DIV:
    case Lop_DIV_U:
    case Lop_REM:
    case Lop_REM_U:
      return 15;
    case Lop_LSL:
    case Lop_LSR:
    case Lop_ASR:
      return 4;
    case Lop_ADD_F2:
    case Lop_SUB_F2:
      return 4;
    case Lop_RCMP_F:
      return 4;
    case Lop_RCMP:
      return 1;
    case Lop_MUL_F2:
      return 6;
    case Lop_DIV_F2:
      return 15;
    case Lop_ADD_F:
    case Lop_SUB_F:
      return 4;
    case Lop_MUL_F:
      return 6;
    case Lop_DIV_F:
      return 15;
    case Lop_F2_I:
    case Lop_F_I:
    case Lop_I_F:
    case Lop_F2_F:
    case Lop_I_F2:
    case Lop_F_F2:
      return 4;
    case Lop_LD_UC:
    case Lop_LD_C:
    case Lop_LD_UC2:
    case Lop_LD_C2:
    case Lop_LD_I:
    case Lop_LD_UI:
    case Lop_LD_Q:
    case Lop_LD_F:
    case Lop_LD_F2:
      return 2;
    default:
      return 1;
    }
}
/*-----------------------------------------------------------------*/
void
L_record_stat (L_Func * fn)
{
  int index;
  L_Cb *cb;
  if (n_fstat == 0)
    {
      if (fstat != NULL)
        L_punt ("L_record_stat: fstat not freed");
      fstat = (struct Fstat *) malloc (sizeof (struct Fstat) * MAX_FN);
    }
  index = n_fstat++;
  if (index >= MAX_FN)
    L_punt ("L_record_stat: too many functions");
  fstat[index].fn_name = fn->name;
  fstat[index].fn_weight = fn->weight;

  fstat[index].bb_weight_0 = 0.0;
  fstat[index].bb_weight_10 = 0.0;
  fstat[index].bb_weight_100 = 0.0;
  fstat[index].bb_weight_1000 = 0.0;
  fstat[index].bb_weight_10000 = 0.0;
  fstat[index].bb_count_0 = 0;
  fstat[index].bb_count_10 = 0;
  fstat[index].bb_count_100 = 0;
  fstat[index].bb_count_1000 = 0;
  fstat[index].bb_count_10000 = 0;

  fstat[index].op_weight_0 = 0.0;
  fstat[index].op_weight_10 = 0.0;
  fstat[index].op_weight_100 = 0.0;
  fstat[index].op_weight_1000 = 0.0;
  fstat[index].op_weight_10000 = 0.0;
  fstat[index].op_count_0 = 0;
  fstat[index].op_count_10 = 0;
  fstat[index].op_count_100 = 0;
  fstat[index].op_count_1000 = 0;
  fstat[index].op_count_10000 = 0;

  fstat[index].br_weight = 0;
  fstat[index].st_weight = 0;
  fstat[index].ld_weight = 0;
  fstat[index].alu_weight = 0;
  fstat[index].fpu_weight = 0;
  fstat[index].other_weight = 0;
  fstat[index].br_count = 0;
  fstat[index].st_count = 0;
  fstat[index].ld_count = 0;
  fstat[index].alu_count = 0;
  fstat[index].fpu_count = 0;
  fstat[index].other_count = 0;

  L_compute_oper_weight (fn, 0, 1);
  for (cb = fn->first_cb; cb != NULL; cb = cb->next_cb)
    {
      L_Oper *oper;
      int num_oper;
      double weight, op_weight;
      num_oper = 0;
      op_weight = 0.0;
      for (oper = cb->first_op; oper != NULL; oper = oper->next_op)
        {
          int opc;
          op_weight += (opcode_weight (oper) * oper->weight);
          num_oper += 1;
          opc = oper->opc;
          switch (opc)
            {
            case Lop_NO_OP:
            case Lop_PROLOGUE:
            case Lop_EPILOGUE:
            case Lop_DEFINE:
            case Lop_ALLOC:
              /* ignore */
              break;
            default:
              if ((opc >= Lop_JSR) && (opc <= Lop_RTS_FS))
                {
                  fstat[index].br_count += 1;
                  fstat[index].br_weight += oper->weight;
                }
              else if ((opc >= Lop_JUMP) && (opc <= Lop_BR_F))
                {
                  fstat[index].br_count += 1;
                  fstat[index].br_weight += oper->weight;
                }
              else if ((opc >= Lop_MOV) && (opc <= Lop_BIT_POS))
                {
                  fstat[index].alu_count += 1;
                  fstat[index].alu_weight += oper->weight;
                }
              else if ((opc >= Lop_ADD_F2) && (opc <= Lop_F_F2))
                {
                  fstat[index].fpu_count += 1;
                  fstat[index].fpu_weight += oper->weight;
                }
              else if ((opc >= Lop_LD_UC) && (opc <= Lop_LD_F2))
                {
                  fstat[index].ld_count += 1;
                  fstat[index].ld_weight += oper->weight;
                }
              else if ((opc >= Lop_ST_C) && (opc <= Lop_ST_F2))
                {
                  fstat[index].st_count += 1;
                  fstat[index].st_weight += oper->weight;
                }
              else
                {
                  fstat[index].other_count += 1;
                  fstat[index].other_weight += oper->weight;
                }
            }
        }
      weight = cb->weight;
      fstat[index].bb_weight_0 += weight;
      fstat[index].bb_count_0 += 1;
      fstat[index].op_weight_0 += op_weight;
      fstat[index].op_count_0 += num_oper;
      if (weight >= 10.0)
        {
          fstat[index].bb_weight_10 += weight;
          fstat[index].bb_count_10 += 1;
          fstat[index].op_weight_10 += op_weight;
          fstat[index].op_count_10 += num_oper;
        }
      if (weight >= 100.0)
        {
          fstat[index].bb_weight_100 += weight;
          fstat[index].bb_count_100 += 1;
          fstat[index].op_weight_100 += op_weight;
          fstat[index].op_count_100 += num_oper;
        }
      if (weight >= 1000.0)
        {
          fstat[index].bb_weight_1000 += weight;
          fstat[index].bb_count_1000 += 1;
          fstat[index].op_weight_1000 += op_weight;
          fstat[index].op_count_1000 += num_oper;
        }
      if (weight >= 10000.0)
        {
          fstat[index].bb_weight_10000 += weight;
          fstat[index].bb_count_10000 += 1;
          fstat[index].op_weight_10000 += op_weight;
          fstat[index].op_count_10000 += num_oper;
        }
    }
}

void
L_print_stat (void)
{
  int i;
  struct Element *list1, *list2;
  list1 = list2 = NULL;
  list1 = (struct Element *) malloc (sizeof (struct Element) * n_fstat);
  list2 = (struct Element *) malloc (sizeof (struct Element) * n_fstat);
  for (i = 0; i < n_fstat; i++)
    {
      list1[i].weight = fstat[i].op_weight_0;
      list1[i].index = i;
    }
  merge_sort (list1, list2, n_fstat);
  print_header (stdout);
  for (i = 0; i < n_fstat; i++)
    {
      print_fn_stat (stdout, fstat + list2[i].index);
    }
  if (list1 != NULL)
    free (list1);
  if (list2 != NULL)
    free (list2);
  if (fstat != NULL)
    free (fstat);
}
