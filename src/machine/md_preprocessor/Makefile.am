## src/machine/md_preprocessor/Makefile.am
## This file is (ultimately) included by the top level Makefile.am, so all
## files need to be specified relative to the top level directory.

## List the files generated in this directory.  Binaries go under _PRG,
## libraries under _LIB.  Higher level Makefiles use these variables
## unconditionally, so both _PRG and _LIB need to be defined.
src_machine_md_preprocessor_PRG = src/machine/md_preprocessor/md_preprocessor
src_machine_md_preprocessor_LIB = 
src_machine_md_preprocessor_DST = \
	src/machine/md_preprocessor/doxygen_config.doc \
	src/machine/md_preprocessor/doxygen_config.tag

## A target to support building everything under this subdirectory.  This
## should refer to anything defined above.  There should be two targets; one
## with and one without the trailing slash.  The trailing slash will allow
## the use of tab completion.
src/machine/md_preprocessor src/machine/md_preprocessor/: \
	include $(src_machine_md_preprocessor_PRG)

## A target to support cleaning everything under this subdirectory.
src/machine/md_preprocessor/clean:
	rm -f $(top_builddir)/src/machine/md_preprocessor/*.$(OBJEXT)
	rm -rf $(srcdir)/src/machine/md_preprocessor/html
	rm -rf $(srcdir)/src/machine/md_preprocessor/latex
	rm -f $(srcdir)/src/machine/md_preprocessor/md_preprocessor.tag
	rm -f $(src_machine_md_preprocessor_PRG)

## List all source and header files under _SOURCES.  
src_machine_md_preprocessor_md_preprocessor_SOURCES = \
	src/machine/md_preprocessor/md_preprocessor.c \
	src/machine/md_preprocessor/md_preprocessor.h \
	src/machine/md_preprocessor/psymbol.c \
	src/machine/md_preprocessor/psymbol.h

## A target to build the tag file for this subdirectory's documentation.
## The tag file contains information on every documented symbol so that
## documentation in other directories can be linked to documentation in
## this one.
$(srcdir)/src/machine/md_preprocessor/md_preprocessor.tag: \
		$(srcdir)/src/machine/md_preprocessor/doxygen_config.tag \
		$(src_machine_md_preprocessor_md_preprocessor_SOURCES)
	cd $(srcdir)/src/machine/md_preprocessor && doxygen doxygen_config.tag

## A target to build all documentation under this subdirectory.  This
## builds the human readable documentation and links it to documentation
## in other directories.
src/machine/md_preprocessor/doc $(srcdir)/src/machine/md_preprocessor/html \
	$(srcdir)/src/machine/md_preprocessor/latex: \
		$(srcdir)/src/machine/md_preprocessor/doxygen_config.doc \
		$(srcdir)/src/machine/md_preprocessor/md_preprocessor.tag \
		$(srcdir)/src/library/libimpact.tag
	cd $(srcdir)/src/machine/md_preprocessor && doxygen doxygen_config.doc

## List all include files that should be symlinked into include under 
## noinst_..._INC.  These include files are not installed when the user
## does 'make install'.  This is appropriate for most include files.  If
## you have headers that need to be installed, see src/library/Makefile.am
## for an example.
noinst_src_machine_md_preprocessor_INC = \
	src/machine/md_preprocessor/md_preprocessor.h \
	src/machine/md_preprocessor/psymbol.h

## The class for the include files.  This is the directory under the include
## directory where the symlinks will be created.  This needs to have the same
## variable name as the _INC variable, only with a _CLASS suffix.
noinst_src_machine_md_preprocessor_CLASS = machine

## List any libraries needed when linking under _LDADD.  If a library is built
## as part of impact, list it as $(top_builddir)/lib/<library> so that it can
## be detected as a dependency automatically.  Automake assumes libraries
## listed as -l<lib> are system libraries and does not detect them as
## dependencies.
src_machine_md_preprocessor_md_preprocessor_LDADD = \
	-L$(top_builddir)/lib $(top_builddir)/lib/libimpact.a

## List any dependencies other than libraries under _DEPENDENDIES.  Defining
## this variable disables automatic dependency detection for this target, so
## you will need to relist dependencies that appear in _LDADD.
## src_machine_md_preprocessor_md_preprocessor_DEPENDENCIES = 

## List any special compiler flags under _CFLAGS.
## src_machine_md_preprocessor_md_preprocessor_CFLAGS = 
