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
 *      File :          l_histogram.c
 *      Description :   histogram manipulation functions
 *      Creation Date : June 1994
 *      Author :        Scott Mahlke
 *
 *==========================================================================*/
/* 10/29/02 REK Adding config.h */
#include <config.h>
#include <string.h>
#include <library/l_histogram.h>
#include <library/l_alloc_new.h>
#include <library/i_error.h>


L_Alloc_Pool *L_histogram_pool = NULL;

L_Histogram *
L_create_histogram (char *name, int init_size)
{
  int size = 0, temp;
  L_Histogram *hist;

  if (L_histogram_pool == NULL)
    L_histogram_pool =
      L_create_alloc_pool ("L_Histogram", sizeof (L_Histogram), 8);
  hist = (L_Histogram *) L_alloc (L_histogram_pool);
  hist->name = strdup (name);

  if (init_size <= L_DEFAULT_HISTOGRAM_SIZE)
    {
      size = L_DEFAULT_HISTOGRAM_SIZE;
    }
  else if (init_size > L_MAX_HISTOGRAM_SIZE)
    {
      I_punt ("L_create_histogram: init_size %d too big", init_size);
    }
  else
    {
      /* pick multiple of default size >= init_size */
      temp = L_DEFAULT_HISTOGRAM_SIZE;
      while (temp < init_size)
        {
          temp *= 2;
        }
      if (temp > L_MAX_HISTOGRAM_SIZE)
        I_punt ("L_create_histogram: init_size %d too big", init_size);
      size = temp;
    }

  hist->data = (double *) calloc (size, sizeof (double));
  hist->size = size;
  hist->over_max_data = 0.0;

  return (hist);
}

void
L_delete_histogram (L_Histogram * hist)
{
  if (hist->name)
    {
      free (hist->name);
      hist->name = NULL;
    }
  if (hist->data)
    {
      free (hist->data);
      hist->data = NULL;
    }

  L_free (L_histogram_pool, hist);
}

/*
 *      Entry is the number for which u want to update, so there must
 *      be space in data array to access data[entry]
 */
void
L_realloc_histogram_data (L_Histogram * hist, int entry)
{
  int i, temp, target_size, alloc_size;
  double *temp_data;

  if (entry >= L_MAX_HISTOGRAM_SIZE)
    I_punt ("L_realloc_histogram_data: entry %d is too large", entry);

  /* to access location entry, must have array of size atleast entry+1 */
  target_size = entry + 1;

  temp = hist->size;
  while (temp < target_size)
    {
      temp *= 2;
    }
  if (temp > L_MAX_HISTOGRAM_SIZE)
    I_punt ("L_realloc_histogram_data: entry %d is too large", entry);
  alloc_size = temp;

  temp_data = (double *) calloc (alloc_size, sizeof (double));

  /* copy over old values into new array */
  for (i = 0; i < hist->size; i++)
    {
      temp_data[i] = hist->data[i];
    }

  /* free old data array */
  free (hist->data);

  /* update hist structure */
  hist->data = temp_data;
  hist->size = alloc_size;
}

void
L_update_histogram (L_Histogram * hist, int entry, double val)
{
  if (hist == NULL)
    I_punt ("L_update_histogram: histogram is NULL");
  if (entry < 0)
    I_punt ("L_update_histogram: cannot specify negative entry");

  /* check if need to realloc */
  if ((hist->size <= entry) && (entry < L_MAX_HISTOGRAM_SIZE))
    {
      L_realloc_histogram_data (hist, entry);
    }

  /* update it by val */
  if (entry >= L_MAX_HISTOGRAM_SIZE)
    hist->over_max_data += val;
  else
    hist->data[entry] += val;
}

void
L_clear_histogram (L_Histogram * hist)
{
  int i, size;

  if (hist == NULL)
    I_punt ("L_clear_histogram: hist is NULL");

  size = hist->size;
  for (i = 0; i < size; i++)
    {
      hist->data[i] = 0.0;
    }
  hist->over_max_data = 0.0;
}

void
L_scale_histogram_entries (L_Histogram * hist, double scale)
{
  int i, size;

  if (hist == NULL)
    I_punt ("L_scale_histogram_entries: hist is NULL");

  size = hist->size;
  for (i = 0; i < size; i++)
    {
      hist->data[i] *= scale;
    }
  hist->over_max_data *= scale;
}

void
L_copy_histogram (L_Histogram * dest, L_Histogram * src)
{
  int i, size;

  if ((dest == NULL) || (src == NULL))
    I_punt ("L_copy_histogram: dest or src is NULL");

  size = src->size;
  if (size > dest->size)
    L_realloc_histogram_data (dest, size);

  for (i = 0; i < size; i++)
    {
      dest->data[i] = src->data[i];
    }

  dest->over_max_data = src->over_max_data;
}

void
L_add_histograms (L_Histogram * dest, L_Histogram * src1, L_Histogram * src2)
{
  int i, size;
  double src1_data, src2_data;

  if ((dest == NULL) || (src1 == NULL) || (src2 == NULL))
    I_punt ("L_add_histograms: dest or src1 are NULL");

  /* set size of dest to max of src1 and src2 */
  if (src1->size > src2->size)
    size = src1->size;
  else
    size = src2->size;
  if (size > dest->size)
    L_realloc_histogram_data (dest, size - 1);

  /* add up the elements from each histogram */
  for (i = 0; i < size; i++)
    {

      if (i < src1->size)
        src1_data = src1->data[i];
      else
        src1_data = 0.0;

      if (i < src2->size)
        src2_data = src2->data[i];
      else
        src2_data = 0.0;

      dest->data[i] = src1_data + src2_data;
    }

  dest->over_max_data = src1->over_max_data + src2->over_max_data;
}

/*=========================================================================*/
/*
 *      Functions for printing histograms
 */
/*=========================================================================*/

/*
 *      Mode = L_PRINT_ALL_ENTRIES
 *      Print out every histogram entry
 */
static void
L_print_histogram_all_entries (FILE * F, L_Histogram * hist)
{
  int i, marker, last;
  double ratio, sum;

  fprintf (F, "    Histogram of %s\n", hist->name);

  /* find number of largest entry with nonzero value in it */
  marker = -1;
  for (i = hist->size - 1; i >= 0; i--)
    {
      if (hist->data[i] != 0)
        {
          marker = i;
          break;
        }
    }

  if (marker != -1)
    {
      marker += 1;
      sum = 0.0;
      for (i = 0; i < marker; i++)
        {
          sum += ((double) ((int) hist->data[i]));
        }
      sum += ((double) ((int) hist->over_max_data));

      /* header info */
      fprintf (F, "    %d entries\n", marker);

      last = 0;
      for (i = 0; i < marker; i++)
        {
          ratio = ((double) ((int) hist->data[i])) / sum;
          fprintf (F, "        %5d %5d   %11d              %1.4f\n",
                   last, i, (int) hist->data[i], ratio);
          last = i + 1;
        }

      /* print out over_max_data entries */
      if (hist->over_max_data > 0.0)
        {
          ratio = ((double) ((int) hist->over_max_data)) / sum;
          fprintf (F, "        %5d %5d   %11d              %1.4f\n",
                   last, -1, (int) hist->over_max_data, ratio);
        }
    }
  else
    {
      fprintf (F, "    0 entries\n");
    }
}

/*
 *      Mode = L_PRINT_GROUPED_VALUES
 *      Print histogram according to the following ranges
 */
#define NUM_HIST_PRINT  10
static int hist_print[NUM_HIST_PRINT] = {
  0,
  1,
  2,
  4,
  8,
  16,
  32,
  64,
  128,
  256
};

static void
L_print_histogram_grouped_values (FILE * F, L_Histogram * hist)
{
  int i, marker, last, print_index;
  double ratio, sum, cur, rest;

  fprintf (F, "    Histogram of %s\n", hist->name);

  /* find number of largest entry with nonzero value in it */
  marker = -1;
  for (i = hist->size - 1; i >= 0; i--)
    {
      if (hist->data[i] != 0)
        {
          marker = i;
          break;
        }
    }

  if (marker != -1)
    {
      marker++;
      sum = 0;
      for (i = 0; i < marker; i++)
        {
          sum += (double) ((int) hist->data[i]);
        }
      sum += ((double) ((int) hist->over_max_data));

      /* header info */
      fprintf (F, "    %d entries\n", (NUM_HIST_PRINT + 1));

      cur = 0.0;
      last = 0;
      print_index = 0;
      for (i = 0; i <= hist_print[NUM_HIST_PRINT - 1]; i++)
        {
          if (hist_print[print_index] == i)
            {
              cur += hist->data[i];
              ratio = ((double) ((int) cur)) / sum;
              fprintf (F, "        %5d %5d   %11d              %1.4f\n",
                       last, i, (int) cur, ratio);
              cur = 0.0;
              last = i + 1;
              print_index++;
            }
          else
            {
              cur += hist->data[i];
            }
        }

      rest = 0.0;
      for (i = (hist_print[NUM_HIST_PRINT - 1] + 1); i < marker; i++)
        {
          rest += hist->data[i];
        }
      rest += hist->over_max_data;
      ratio = ((double) ((int) rest)) / sum;
      fprintf (F, "        %5d %5d   %11d              %1.4f\n",
               last, -1, (int) rest, ratio);
    }
  else
    {
      fprintf (F, "    0 entries\n");
    }
}

/*
 *      Mode = L_PRINT_GROUPED_PERCENTAGES
 */
static void
L_print_histogram_grouped_percentages (FILE * F, L_Histogram * hist)
{
  int i, marker, last;
  double ratio, sum, cur;

  fprintf (F, "    Histogram of %s\n", hist->name);

  /* find number of largest entry with nonzero value in it */
  marker = -1;
  for (i = hist->size - 1; i >= 0; i--)
    {
      if (hist->data[i] != 0)
        {
          marker = i;
          break;
        }
    }

  if (marker > 100)
    I_punt ("L_print_histogram_grouped_percentages: illegal histogram size");

  if (marker != -1)
    {
      marker = 101;
      sum = 0;
      for (i = 0; i < marker; i++)
        {
          sum += (double) ((int) hist->data[i]);
        }

      /* header info */
      fprintf (F, "    11 entries\n");

      /* 0th entry handled separately */
      ratio = ((double) ((int) hist->data[0])) / sum;
      fprintf (F, "        %5d %5d   %11d              %1.4f\n",
               0, 0, (int) hist->data[0], ratio);

      cur = 0.0;
      last = 1;
      for (i = 1; i < marker; i++)
        {
          if ((i % 10) == 0)
            {
              cur += hist->data[i];
              ratio = ((double) ((int) cur)) / sum;
              fprintf (F, "        %5d %5d   %11d              %1.4f\n",
                       last, i, (int) cur, ratio);
              cur = 0.0;
              last = i + 1;
            }
          else
            {
              cur += hist->data[i];
            }
        }
    }
  else
    {
      fprintf (F, "    0 entries\n");
    }
}


/*
 *      Mode = L_PRINT_INDIVIDUAL_PERCENTAGES
 *      Have to pass in sum, since we are not summing up entries for
 *      computing a ratio.
 */
static void
L_print_histogram_individual_percentages (FILE * F, L_Histogram * hist,
                                          double sum)
{
  int i, marker, last;
  double ratio;

  fprintf (F, "    Histogram of %s\n", hist->name);

  /* find number of largest entry with nonzero value in it */
  marker = -1;
  for (i = hist->size - 1; i >= 0; i--)
    {
      if (hist->data[i] != 0)
        {
          marker = i;
          break;
        }
    }

  if (marker > 100)
    I_punt
      ("L_print_histogram_individual_percentages: illegal histogram size");

  if (marker != -1)
    {
      marker = 101;

      /* header info */
      fprintf (F, "    11 entries\n");

      /* 0th entry handled separately */
      ratio = ((double) ((int) hist->data[0])) / sum;
      fprintf (F, "        %5d %5d   %11d              %1.4f\n",
               0, 0, (int) hist->data[0], ratio);

      last = 1;
      for (i = 1; i < marker; i++)
        {
          if ((i % 10) == 0)
            {
              ratio = ((double) ((int) hist->data[i])) / sum;
              fprintf (F, "        %5d %5d   %11d              %1.4f\n",
                       last, i, (int) hist->data[i], ratio);
              last = i + 1;
            }
          /* else do nothing */
        }
    }
  else
    {
      fprintf (F, "    0 entries\n");
    }
}

static void
L_print_histogram_average_value (FILE * F, L_Histogram * hist)
{
  int i, marker;
  double tot, sum, ratio;

  /* find number of largest entry with nonzero value in it */
  marker = -1;
  for (i = hist->size - 1; i >= 0; i--)
    {
      if (hist->data[i] != 0)
        {
          marker = i;
          break;
        }
    }

  fprintf (F, "    Histogram of %s \n", hist->name);
  if (marker != -1)
    {
      marker += 1;
      sum = 0.0;
      for (i = 0; i < marker; i++)
        {
          sum += hist->data[i];
        }
      sum += hist->over_max_data;

      tot = 0.0;
      for (i = 0; i < marker; i++)
        {
          tot = tot + (double) (hist->data[i] * i);
        }
      tot = tot + (hist->over_max_data * L_MAX_HISTOGRAM_SIZE);
      ratio = tot / sum;

      fprintf (F, "    Average_value %f\n", ratio);
    }

  else
    {
      fprintf (F, "    Average_value 0.00\n");
    }



}

/*
 *      Main print routine
 */
void
L_print_histogram (FILE * F, L_Histogram * hist, int mode, double sum)
{
  if (F == NULL)
    I_punt ("L_print_histogram: F is NULL");
  if (hist == NULL)
    I_punt ("L_print_histogram: hist is NULL");

  switch (mode)
    {
    case L_PRINT_ALL_ENTRIES:
      L_print_histogram_all_entries (F, hist);
      break;
    case L_PRINT_GROUPED_VALUES:
      L_print_histogram_grouped_values (F, hist);
      break;
    case L_PRINT_GROUPED_PERCENTAGES:
      L_print_histogram_grouped_percentages (F, hist);
      break;
    case L_PRINT_INDIVIDUAL_PERCENTAGES:
      L_print_histogram_individual_percentages (F, hist, sum);
      break;
    case L_PRINT_AVERAGE_VALUE:
      L_print_histogram_average_value (F, hist);
      break;
    default:
      I_punt ("L_print_histogram: illegal mode, %d", mode);
    }
}
