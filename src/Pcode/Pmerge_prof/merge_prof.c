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
 *      File:   merge_prof.c
 *      Author: Le-Chun Wu and Wen-mei Hwu
 * Modified By: Andrew Trick and John C. Gyllenhaal -4/99
\*****************************************************************************/
#include <config.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <dirent.h>
#include <sys/types.h>
#include <sys/stat.h>

#define BUFFER_SZ 1024

/* how to merge the multiple prof data files */
#define AVERAGE   1
#define SUMMATION 2
#define MAXIMUM   3

struct file_list {
  char *name;
  struct file_list *next;
};

void process_prof_files(struct file_list *prof_flst, int probe_num, 
			char *dest_file_name, int merge_type)
{
  struct file_list *flst_ptr;
  double *final_probe_array, *tmp_probe_array;
  FILE *Fin, *Fout;
  int valid_prof_file_no = 0;
  double prof_weight;
  int result_is_valid = 0, error_occured;
  int probe_count = 0;
  int i;

  /* temp array stores values until entire file is considered valid */
  tmp_probe_array = (double *) malloc (sizeof(double)*probe_num);
  final_probe_array = (double *) malloc (sizeof(double)*probe_num);

  /* initialize final_probe_array */
  for (i = 0; i < probe_num; i++)
      final_probe_array[i] = 0;

  /* process each profile data file */
  for (flst_ptr = prof_flst; flst_ptr != NULL; flst_ptr = flst_ptr->next) {
      probe_count = 0;
      error_occured = 0;

      /* Open input file */
      if ((Fin = fopen(flst_ptr->name, "r")) == NULL) {
	 fprintf(stderr, "Pmerge_prof warning: cannot open %s.\n", 
		 flst_ptr->name);
	 continue;
      }
      /* Read each probe value form the input file */
      while (fscanf(Fin, "%lf", &prof_weight) != EOF) {
	 if (probe_count >= probe_num) {
	    fprintf(stderr, "Pmerge_prof warning: %s has too many lines.\n", 
		 flst_ptr->name);
	    error_occured = 1;
	    break;
	 }
	 tmp_probe_array[probe_count++] = prof_weight;
      }
      if (error_occured)
	 continue;
      if (probe_count < probe_num) {
	 fprintf(stderr, "Pmerge_prof warning: %s has too few lines.\n", 
		 flst_ptr->name);
	 continue;
      }

      /* control reaching here means the profile data file is valid */
      result_is_valid = 1;
      valid_prof_file_no ++;
      for (i = 0; i < probe_num; i++) {
	 switch(merge_type) {
	   case AVERAGE:
	   case SUMMATION:
	     final_probe_array[i] += tmp_probe_array[i];
	     break;
	   case MAXIMUM:
	     if (tmp_probe_array[i] > final_probe_array[i])
	        final_probe_array[i] = tmp_probe_array[i];
	     break;
	   default:
	     fprintf(stderr, "Pmerge_prof error: invalid merge type.\n"); 
	     exit(-1);
	     break;
	 }
      }
  }
  
  if (result_is_valid) {
     /* take the average when the merge type is AVERAGE */
     if (merge_type == AVERAGE) {
        for (i = 0; i < probe_num; i++)
	    final_probe_array[i] = final_probe_array[i] / valid_prof_file_no;
     }
     if ((Fout = fopen(dest_file_name, "w")) != NULL)
        for (i = 0; i < probe_num; i++)
	    fprintf(Fout, "\t%f\n", final_probe_array[i]);
     else {
        fprintf(stderr, "Pmerge_prof error: cannot open %s.\n", 
		dest_file_name);
	exit(-1);
     }
  }
  else {
     fprintf(stderr, "Pmerge_prof error: no valid profile data files.\n"); 
     exit(-1);
  }
}


void copyFile (char *in_name, char *out_name) {
  FILE *Fin, *Fout;
  char buffer[BUFFER_SZ];
  int num_read;

  if ((Fin = fopen (in_name, "r")) == NULL) {
    fprintf (stderr, "Pmerge_prof error: cannot open %s\n", in_name);
    exit (-1);
  }
  if ((Fout = fopen (out_name, "w")) == NULL) {
    fprintf (stderr, "Pmerge_prof error: cannot open %s\n", out_name);
    exit (-1);
  }

  while (!feof (Fin)) {
    num_read = fread (buffer, sizeof( char ), BUFFER_SZ, Fin);
    if (ferror (Fin)) {
      fprintf(stderr, "Pmerge_prof error reading input file %s.\n", in_name);
      exit(-1);
    }
    fwrite (buffer, sizeof( char ), num_read, Fout);
    if (ferror (Fout)) {
      fprintf(stderr, "Pmerge_prof error writing output file %s.\n", out_name);
      exit(-1);
    }
  }
  fclose (Fin);
  fclose (Fout);
}

void printUsage() {
  fprintf(stderr, "Usage: Pmerge_prof profile_dat.* [options]\n\n");
  fprintf(stderr, " Generates profile.dat (merged profile data).\n\n");
  fprintf(stderr, " One of the following options may be specified:\n");
  fprintf(stderr, "   -a\taverage profile values (default)\n");
  fprintf(stderr, "   -s\t    sum profile values\n");
  fprintf(stderr, "   -m\tuse max profile value\n\n");
  fprintf(stderr, "   -numprobes {val}\tnumber of probe values (greater than 0)\n");
  fprintf(stderr, "     by default, read from ./impact_probe.tmp\n\n");
  fprintf(stderr, " \"profile_data.*\" is a list of input files to merge\n");
}

int
main(int argc, char **argv)
{
  char *dir_name, *dest_file_name;
  char *probe_tmp_name = "impact_probe.tmp";
  struct file_list *prof_flst = NULL, *flst_ptr = NULL, *new_file;
  int num_probes = 0;
  int merge_type = SUMMATION;
  int i;

  if (argc < 2) {
     printUsage();
     exit(-1);
  }

  dir_name = ".";
  for (i = 1; i < argc; i++) {
     if (argv[i][0] == '-') {
        switch(argv[i][1]) {
          case 'a':
            merge_type = AVERAGE;
            break;
          case 's':
            merge_type = SUMMATION;
            break;
          case 'm':
            merge_type = MAXIMUM;
            break;
          case 'n':
            if (!strncmp (argv[i], "-numprobes", 10) && argc > ++i) {
              probe_tmp_name = NULL;
              num_probes = atoi (argv[i]);
              break;
            }
          default:
            printUsage();
            exit(-1);
        }
     }
     else {
       /* Add a new profile file name */
       new_file = (struct file_list *)malloc(sizeof(struct file_list));
       new_file->next = NULL;
       new_file->name = argv[i];
       if (prof_flst == NULL)
           prof_flst = flst_ptr = new_file;
       else {
         flst_ptr->next = new_file;
         flst_ptr = new_file;
       }
     }
  }

  if (prof_flst == NULL) {
    printUsage();
    fprintf (stderr, "\nPmerge_prof error: At least one input file must be specified.\n");
    exit(-1);
  }

  /* Read num probes from temp file */
  if (probe_tmp_name != NULL) {
    char *tmp_name;
    FILE *probe_tmp_file;
    tmp_name = (char *)malloc(strlen(dir_name) + strlen(probe_tmp_name) + 2);
    sprintf (tmp_name, "%s/%s", dir_name, probe_tmp_name);
    if ((probe_tmp_file = fopen(tmp_name, "r")) != NULL) {
      fscanf (probe_tmp_file, "%d", &num_probes);
      fclose (probe_tmp_file);
    }
    else {
      fprintf (stderr, "Pmerge_prof error: cannot open %s\n", tmp_name);
      exit (-1);
    }
  }

  dest_file_name = (char *)malloc(strlen(dir_name) + 14);
  sprintf(dest_file_name, "%s/profile.dat", dir_name);

  /* If only one input file exists, simply copy it */
  if (prof_flst->next == NULL) {
    copyFile (prof_flst->name, dest_file_name);
  }
  else {
    process_prof_files (prof_flst, num_probes, dest_file_name, merge_type);
  }

  exit(0);
  return 0;
}
