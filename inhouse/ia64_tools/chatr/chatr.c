/*
    chatr -- a tool to modify ELF executable header flags
    Copyright (C) 2002  Hewlett-Packard Company
      Contributed by David Mosberger-Tang <davidm@hpl.hp.com>

    This program is free software; you can redistribute it and/or
    modify it under the terms of the GNU General Public License as
    published by the Free Software Foundation; either version 2 of the
    License, or (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
    General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program; if not, write to the Free Software
    Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA
    02111-1307 USA
*/

#include <config.h>
#include <elf.h>
#include <fcntl.h>
#include <getopt.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

#include <sys/types.h>

#ifndef EF_IA_64_LINUX_EXECUTABLE_STACK
# define EF_IA_64_LINUX_EXECUTABLE_STACK        0x1
#endif

#ifndef PF_IA_64_NORECOV
# define PF_IA_64_NORECOV   0x80000000
#endif

static struct option long_options[] = {
  {"help", 0, 0, 'h'},
  {"executable-stack", 0, 0, 'E'},
  {"no-executable-stack", 0, 0, 'e'},
  {"recovery-code", 0, 0, 'R'},
  {"no-recovery-code", 0, 0, 'r'},
};

static const char *prog_name;

static void
usage (FILE *fp)
{
  fprintf (fp, "Usage: %s [-eErRh] files...\n"
           "\t-e: mark stack and data of image as not executable\n"
           "\t-E: mark stack and data of image as executable\n"
           "\t-r: mark as having no recovery code\n"
           "\t-R: mark as having recovery code\n"
           "\t-h: print this help message\n", prog_name);
}

static void
update_file (const char *filename, int executable_stack, int recovery_code)
{
  Elf64_Ehdr ehdr;
  ssize_t ret;
  int fd;

  fd = open (filename, executable_stack | recovery_code ? O_RDWR : O_RDONLY);
  if (fd < 0)
    {
      perror (filename);
      exit (-1);
    }

  if ((ret = read (fd, &ehdr, sizeof (ehdr)) != sizeof (ehdr)))
    {
      if (ret < 0)
        perror (filename);
      else
        fprintf (stderr, "%s: short read\n", filename);
      exit (-1);
    }

  /* Update executable_stack */

  if (executable_stack != 0)
    {
      if (executable_stack == 1)
	ehdr.e_flags |= EF_IA_64_LINUX_EXECUTABLE_STACK;
      else
	ehdr.e_flags &= ~EF_IA_64_LINUX_EXECUTABLE_STACK;

      if (lseek (fd, 0, SEEK_SET) < 0)
	{
	  perror ("lseek");
	  exit (-1);
	}
      if (write (fd, &ehdr, sizeof (ehdr)) != sizeof (ehdr))
	{
	  perror ("write");
	  exit (-1);
	}
    }
  else if (!(recovery_code | executable_stack))
    {
      printf ("%s:\n\tstack and data executable: %s\n", filename,
	    (ehdr.e_flags & EF_IA_64_LINUX_EXECUTABLE_STACK) ? "yes" : "no");
    }

  {
    unsigned long phoff;
    /* 10/25/04 REK Commenting out unused variables to quiet compiler
     *              warnings. */
#if 0
    unsigned short phnum, phentsize;
#endif
    Elf64_Phdr phdr;
    int i;

    phoff = ehdr.e_phoff;

    for (i = 0; i < ehdr.e_phnum; i++)
      {
	if (lseek (fd, phoff, SEEK_SET) < 0)
	  {
	    perror ("lseek");
	    exit (-1);
	  }
	  
	if ((ret = read (fd, &phdr, sizeof (phdr))) != sizeof (phdr))
	  {
	    if (ret < 0)
	      perror (filename);
	    else
	      fprintf (stderr, "%s: short read\n", filename);
	    exit (-1);
	  }

	if (recovery_code != 0)
	  {
	    if (recovery_code == 1)
	      phdr.p_flags &= ~PF_IA_64_NORECOV;
	    else
	      phdr.p_flags |= PF_IA_64_NORECOV;

	    if (lseek (fd, phoff, SEEK_SET) < 0)
	      {
		perror ("lseek");
		exit (-1);
	      }
	    if (write (fd, &phdr, sizeof (phdr)) != sizeof (phdr))
	      {
		perror ("write");
		exit (-1);
	      }	  
	  }
	else if (!(recovery_code | executable_stack))
	  {
	    printf ("\trecovery code (ph%d): %s\n", i,
		    (phdr.p_flags & PF_IA_64_NORECOV) ? "no" : "yes");
	  }
	phoff += ehdr.e_phentsize;
      }
  }
}

int
main (int argc, char **argv)
{
  int ch, executable_stack = 0, recovery_code = 0;
  extern int optind;

  prog_name = argv[0];

  if (argc < 2)
    {
      usage (stderr);
      exit (-1);
    }

  while (1)
    {
      ch = getopt_long (argc, argv, "eErRh", long_options, NULL);
      if (ch == -1)
        break;

      switch (ch)
        {
        case 'e': executable_stack = -1; break;
        case 'E': executable_stack =  1; break;
        case 'r': recovery_code = -1; break;
        case 'R': recovery_code =  1; break;
        case 'h': usage (stdout); exit (0);
        }
    }

  while (optind < argc)
    update_file (argv[optind++], executable_stack, recovery_code);

  return 0;
}

