## src/Lcode/performance/Lannotate/Makefile.am
## This file is (ultimately) included by the top level Makefile.am, so all
## files need to be specified relative to the top level directory.

## List the files generated in this directory.  Binaries go under _PRG,
## libraries under _LIB.  Higher level Makefiles use these variables
## unconditionally, so both _PRG and _LIB need to be defined.
src_Lcode_performance_Lannotate_PRG = src/Lcode/performance/Lannotate/Lannotate
src_Lcode_performance_Lannotate_LIB = 
src_Lcode_performance_Lannotate_DST = \
	src/Lcode/performance/Lannotate/doxygen_config.doc \
	src/Lcode/performance/Lannotate/doxygen_config.tag

## A target to support building everything under this subdirectory.  This
## should refer to anything defined above.  There should be two targets; one
## with and one without the trailing slash.  The trailing slash will allow
## the use of tab completion.
src/Lcode/performance/Lannotate src/Lcode/performance/Lannotate: \
	include $(src_Lcode_performance_Lannotate_PRG)

## A target to support cleaning everything under this subdirectory.
src/Lcode/performance/Lannotate/clean:
	rm -f $(top_builddir)/src/Lcode/performance/Lannotate/*.$(OBJEXT)
	rm -rf $(srcdir)/src/Lcode/performance/Lannotate/html
	rm -rf $(srcdir)/src/Lcode/performance/Lannotate/latex
	rm -f $(srcdir)/src/Lcode/performance/Lannotate/Lannotate.tag
	rm -f $(src_Lcode_performance_Lannotate_PRG)

## List all source and header files under _SOURCES.  
src_Lcode_performance_Lannotate_Lannotate_SOURCES = \
	src/Lcode/performance/Lannotate/l_annotate.c \
	src/Lcode/performance/Lannotate/l_annotate.h \
	src/Lcode/performance/Lannotate/l_hash.c \
	src/Lcode/performance/Lannotate/l_hash.h

## A target to build the tag file for this subdirectory's documentation.
## The tag file contains information on every documented symbol so that
## documentation in other directories can be linked to documentation in
## this one.
$(srcdir)/src/Lcode/performance/Lannotate/Lannotate.tag: \
		$(srcdir)/src/Lcode/performance/Lannotate/doxygen_config.tag \
		$(src_Lcode_performance_Lannotate_Lannotate_SOURCES)
	cd $(srcdir)/src/Lcode/performance/Lannotate && \
		doxygen doxygen_config.tag

## A target to build all documentation under this subdirectory.  This
## builds the human readable documentation and links it to documentation
## in other directories.
src/Lcode/performance/Lannotate/doc \
	$(srcdir)/src/Lcode/performance/Lannotate/html \
	$(srcdir)/src/Lcode/performance/Lannotate/latex: \
		$(srcdir)/src/Lcode/performance/Lannotate/doxygen_config.doc \
		$(srcdir)/src/Lcode/performance/Lannotate/Lannotate.tag \
		$(srcdir)/src/Lcode/Lcode/liblcode.tag \
		$(srcdir)/src/bdd/libbdd.tag \
		$(srcdir)/src/machine/Mspec/libmspec.tag \
		$(srcdir)/src/library/libimpact.tag \
		$(srcdir)/src/machine/Lmdes/liblmdes.tag
	cd $(srcdir)/src/Lcode/performance/Lannotate && \
		doxygen doxygen_config.doc

## List all include files that should be symlinked into include under 
## noinst_..._INC.  These include files are not installed when the user
## does 'make install'.  This is appropriate for most include files.  If
## you have headers that need to be installed, see src/library/Makefile.am
## for an example.
noinst_src_Lcode_performance_Lannotate_INC = \
	src/Lcode/performance/Lannotate/l_annotate.h

## The class for the include files.  This is the directory under the include
## directory where the symlinks will be created.  This needs to have the same
## variable name as the _INC variable, only with a _CLASS suffix.
noinst_src_Lcode_performance_Lannotate_CLASS = Lcode

## List any libraries needed when linking under _LDADD.  If a library is built
## as part of impact, list it as $(top_builddir)/lib/<library> so that it can
## be detected as a dependency automatically.  Automake assumes libraries
## listed as -l<lib> are system libraries and does not detect them as
## dependencies.
src_Lcode_performance_Lannotate_Lannotate_LDADD = \
	-L$(top_builddir) $(top_builddir)/lib/liblcode.a \
	$(top_builddir)/lib/libbdd.a $(top_builddir)/lib/libmspec.a \
	$(top_builddir)/lib/libimpact.a $(top_builddir)/lib/liblmdes.a \
        $(top_builddir)/lib/libproto.a -lm

## List any dependencies other than libraries under _DEPENDENDIES.  Defining
## this variable disables automatic dependency detection for this target, so
## you will need to relist dependencies that appear in _LDADD.
## src_Lcode_performance_Lannotate_Lannotate_DEPENDENCIES = 

## List any special compiler flags under _CFLAGS.
## src_Lcode_performance_Lannotate_Lannotate_CFLAGS = 

