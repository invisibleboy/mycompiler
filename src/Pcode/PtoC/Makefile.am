## src/Pcode/PtoC/Makefile.am
## This file is (ultimately) included by the top level Makefile.am, so all
## files need to be specified relative to the top level directory.

## List the files generated in this directory.  Binaries go under _PRG,
## libraries under _LIB.  Higher level Makefiles use these variables
## unconditionally, so both _PRG and _LIB need to be defined.
src_Pcode_PtoC_PRG = src/Pcode/PtoC/PtoC
src_Pcode_PtoC_LIB = 
src_Pcode_PtoC_DST = src/Pcode/PtoC/doxygen_config.doc \
	src/Pcode/PtoC/doxygen_config.tag

## A target to support building everything under this subdirectory.  This
## should refer to anything defined above.  There should be two targets; one
## with and one without the trailing slash.  The trailing slash will allow
## the use of tab completion.
src/Pcode/PtoC src/Pcode/PtoC/: \
	include $(src_Pcode_PtoC_PRG) $(src_Pcode_PtoC_LIB)

## A target to support cleaning everything under this subdirectory.
src/Pcode/PtoC/clean:
	rm -f $(top_builddir)/src/Pcode/PtoC/*.$(OBJEXT)
	rm -f $(src_Pcode_PtoC_PRG)

## List all source and header files under _SOURCES.  
src_Pcode_PtoC_PtoC_SOURCES = \
	src/Pcode/PtoC/do_pcode.c

## A target to build the tag file for this subdirectory's documentation.
## The tag file contains information on every documented symbol so that
## documentation in other directories can be linked to documentation in
## this one.
$(srcdir)/src/Pcode/PtoC/PtoC.tag: \
		$(srcdir)/src/Pcode/PtoC/doxygen_config.tag \
		$(src_Pcode_PtoC_PtoC_SOURCES)
	cd $(srcdir)/src/Pcode/PtoC && doxygen doxygen_config.tag

## A target to build all documentation under this subdirectory.  This
## build the human readable documentation and links it to documentation
## in other directories.
src/Pcode/PtoC/doc $(srcdir)/src/Pcode/PtoC/html \
	$(srcdir)/src/Pcode/PtoC/latex: \
		$(srcdir)/src/Pcode/PtoC/doxygen_config.doc \
		$(srcdir)/src/Pcode/Pcode/libpcode.tag \
		$(srcdir)/src/library/libimpact.tag
	cd $(srcdir)/src/Pcode/PtoC && doxygen doxygen_config.doc

## List all include files that should be symlinked into include under 
## noinst_..._INC.  These include files are not installed when the user
## does 'make install'.  This is appropriate for most include files.  If
## you have headers that need to be installed, see src/library/Makefile.am
## for an example.
## noinst_src_Pcode_PtoC_INC = 

## The class for the include files.  This is the directory under the include
## directory where the symlinks will be created.  This needs to have the same
## variable name as the _INC variable, only with a _CLASS suffix.
## noinst_src_Pcode_PtoC_CLASS = 

## List any libraries needed when linking under _LDADD.  If a library is built
## as part of impact, list it as $(top_builddir)/lib/<library> so that it can
## be detected as a dependency automatically.  Automake assumes libraries
## listed as -l<lib> are system libraries and does not detect them as
## dependencies.
src_Pcode_PtoC_PtoC_LDADD = -L$(top_builddir)/lib \
	$(top_builddir)/lib/libpcode_probe.a \
	$(top_builddir)/lib/libmspec.a $(top_builddir)/lib/libpcode_cf.a \
	$(top_builddir)/lib/libpcode.a $(top_builddir)/lib/libimpact.a

## List any dependencies other than libraries under _DEPENDENDIES.  Defining
## this variable disables automatic dependency detection for this target, so
## you will need to relist dependencies that appear in _LDADD.
## src_Pcode_PtoC_PtoC_DEPENDENCIES = 

## List any special compiler flags under _CFLAGS.
## src_Pcode_PtoC_PtoC_CFLAGS = 


