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
/* 9/6/02 Robert Kidd
 * This program takes the name of an elf file and section as its arguments.
 * It dumps the contents of the given section to standard out.
 */
/* 12/02/02 REK Changing <libelf.h> to <libelf/libelf.h> for compatibility
 *              with older versions of libelf. */

#include <config.h>
#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#ifdef HAVE_LIBELF_LIBELF_H
#include <libelf/libelf.h>
#else
#include <libelf.h>
#endif
#include <string.h>
#include <unistd.h>

int main(int argc, char **argv)
{
    char *inputFile, *sectionName;
    int fileDes;
    Elf *elfDes;
    Elf64_Ehdr *elfHeader;
    Elf64_Shdr *sectionHeader;
    Elf_Scn *section;
    Elf_Data *data;

    if (argc != 3)
    {
	printf ("usage: dumpelfsection <elf file> <section name>\n");
	printf (" Dumps <section name> to stdout.\n");
	exit (0);
    } /* if */

    inputFile=argv[1];
    sectionName=argv[2];

    fileDes=open (inputFile, O_RDONLY);

    if (fileDes == -1)
    {
	printf ("Error: could not open input file %s\n", inputFile);
	exit (1);
    } /* if */

    elf_version (EV_CURRENT);
    
    elfDes=elf_begin (fileDes, ELF_C_READ, NULL);

    if (elfDes == NULL)
    {
	printf ("Error: Call to elf_begin failed.\n");
	exit (1);
    } /* if */

    elfHeader=elf64_getehdr (elfDes);

    if (elfHeader == NULL)
    {
	printf ("Error: Call to elf64_getehdr failed.\n");
	elf_end (elfDes);
	exit (1);
    } /* if */

    section=elf_getscn (elfDes, elfHeader->e_shstrndx);

    if (section == NULL)
    {
	printf ("Error: Call to elf_getscn failed.\n");
	elf_end (elfDes);
	exit (1);
    } /* if */

    data=elf_getdata (section, NULL);

    if (data == NULL)
    {
	printf ("Error: Call to elf_getdata failed.\n");
	elf_end (elfDes);
	exit (1);
    } /* if */

    /* Find the section to dump */
    section=NULL;

    while ((section=elf_nextscn (elfDes, section)) &&
	   (sectionHeader=elf64_getshdr (section)) &&
	   (strcmp ((char *)data->d_buf + sectionHeader->sh_name,
		    sectionName)!=0));

    /* At this point, section is the one that we want to dump if it exists,
     * or NULL. */
    if (section != NULL)
    {
	data=elf_rawdata (section, NULL);

	/* Write the data to stdout. */
	fwrite (data->d_buf, data->d_size, 1, stdout);
    } /* if */

    elf_end (elfDes);
    close (fileDes);
    exit (0);
} /* main */

