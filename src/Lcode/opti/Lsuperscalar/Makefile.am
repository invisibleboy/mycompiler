## src/Lcode/opti/Lsuperscalar/Makefile.am
## This file is (ultimately) included by the top level Makefile.am, so all
## files need to be specified relative to the top level directory.

## List the files generated in this directory.  Binaries go under _PRG,
## libraries under _LIB.  Higher level Makefiles use these variables
## unconditionally, so both _PRG and _LIB need to be defined.
src_Lcode_opti_Lsuperscalar_PRG = src/Lcode/opti/Lsuperscalar/Lsuperscalar
src_Lcode_opti_Lsuperscalar_LIB = src/Lcode/opti/Lsuperscalar/libsuperscalar.a
src_Lcode_opti_Lsuperscalar_DST = \
	src/Lcode/opti/Lsuperscalar/doxygen_config.doc \
	src/Lcode/opti/Lsuperscalar/doxygen_config.tag

## A target to support building everything under this subdirectory.  This
## should refer to anything defined above.  There should be two targets; one
## with and one without the trailing slash.  The trailing slash will allow
## the use of tab completion.
src/Lcode/opti/Lsuperscalar src/Lcode/opti/Lsuperscalar/: \
	include $(src_Lcode_opti_Lsuperscalar_PRG) \
	$(src_Lcode_opti_Lsuperscalar_LIB)

## A target to support cleaning everything under this subdirectory.
src/Lcode/opti/Lsuperscalar/clean:
	rm -f $(top_builddir)/src/Lcode/opti/Lsuperscalar/*.$(OBJEXT)
	rm -rf $(srcdir)/src/Lcode/opti/Lsuperscalar/html
	rm -rf $(srcdir)/src/Lcode/opti/Lsuperscalar/latex
	rm -f $(srcdir)/src/Lcode/opti/Lsuperscalar/Lsuperscalar.tag
	rm -f $(src_Lcode_opti_Lsuperscalar_PRG)
	rm -f $(src_Lcode_opti_Lsuperscalar_LIB)

src_Lcode_opti_Lsuperscalar_SRC = \
	src/Lcode/opti/Lsuperscalar/l_accumulator.c \
	src/Lcode/opti/Lsuperscalar/l_br_elim.c \
	src/Lcode/opti/Lsuperscalar/l_codegen.c \
	src/Lcode/opti/Lsuperscalar/l_critical.c \
	src/Lcode/opti/Lsuperscalar/l_jump_driver.c \
	src/Lcode/opti/Lsuperscalar/l_local_driver.c \
	src/Lcode/opti/Lsuperscalar/l_loop_classic.c \
	src/Lcode/opti/Lsuperscalar/l_loop_classic.h \
	src/Lcode/opti/Lsuperscalar/l_loop_driver.c \
	src/Lcode/opti/Lsuperscalar/l_loop_driver.h \
	src/Lcode/opti/Lsuperscalar/l_loop_unroll.c \
	src/Lcode/opti/Lsuperscalar/l_multiway.c \
	src/Lcode/opti/Lsuperscalar/l_peel_accum.c \
	src/Lcode/opti/Lsuperscalar/l_rem_loop.c \
	src/Lcode/opti/Lsuperscalar/l_rem_loop.h \
	src/Lcode/opti/Lsuperscalar/l_strength.c \
	src/Lcode/opti/Lsuperscalar/lsuper_export.h \
	src/Lcode/opti/Lsuperscalar/l_super_parms.c \
	src/Lcode/opti/Lsuperscalar/l_super_parms.h \
	src/Lcode/opti/Lsuperscalar/l_superscalar.c \
	src/Lcode/opti/Lsuperscalar/l_superscalar.h

## A target to build the tag file for this subdirectory's documentation.
## The tag file contains information on every documented symbol so that
## documentation in other directories can be linked to documentation in
## this one.
$(srcdir)/src/Lcode/opti/Lsuperscalar/Lsuperscalar.tag: \
		$(srcdir)/src/Lcode/opti/Lsuperscalar/doxygen_config.tag \
		$(src_Lcode_opti_Lsuperscalar_SRC)
	cd $(srcdir)/src/Lcode/opti/Lsuperscalar && doxygen doxygen_config.tag

## A target to build all documentation under this subdirectory.  This
## builds the human readable documentation and links it to documentation
## in other directories.
src/Lcode/opti/Lsuperscalar/doc $(srcdir)/src/Lcode/opti/Lsuperscalar/html \
	$(srcdir)/src/Lcode/opti/Lsuperscalar/latex: \
		$(srcdir)/src/Lcode/opti/Lsuperscalar/doxygen_config.doc \
		$(srcdir)/src/Lcode/opti/Lsuperscalar/Lsuperscalar.tag \
		$(srcdir)/src/Lcode/opti/Lopti/Lopti.tag \
		$(srcdir)/src/Lcode/opti/Lblock/Lblock.tag \
		$(srcdir)/src/Lcode/sched/SM/libsm.tag \
		$(srcdir)/src/machine/Lmdes/liblmdes.tag \
		$(srcdir)/src/Lcode/codegen/Regalloc/libregalloc.tag \
		$(srcdir)/src/Lcode/opti/Lmarkpipe/libmarkpipe.tag \
		$(srcdir)/src/Lcode/Lcode/liblcode.tag \
		$(srcdir)/src/bdd/libbdd.tag \
		$(srcdir)/src/machine/Mspec/libmspec.tag \
		$(srcdir)/src/library/libimpact.tag
	cd $(srcdir)/src/Lcode/opti/Lsuperscalar && doxygen doxygen_config.doc

## List all include files that should be symlinked into include under 
## noinst_..._INC.  These include files are not installed when the user
## does 'make install'.  This is appropriate for most include files.  If
## you have headers that need to be installed, see src/library/Makefile.am
## for an example.
noinst_src_Lcode_opti_Lsuperscalar_INC = \
	src/Lcode/opti/Lsuperscalar/l_loop_classic.h \
	src/Lcode/opti/Lsuperscalar/l_rem_loop.h \
	src/Lcode/opti/Lsuperscalar/l_superscalar.h

## The class for the include files.  This is the directory under the include
## directory where the symlinks will be created.  This needs to have the same
## variable name as the _INC variable, only with a _CLASS suffix.
noinst_src_Lcode_opti_Lsuperscalar_CLASS = Lcode

## List all source and header files under _SOURCES.  
src_Lcode_opti_Lsuperscalar_Lsuperscalar_SOURCES = \
	$(src_Lcode_opti_Lsuperscalar_SRC)

## List any libraries needed when linking under _LDADD.  If a library is built
## as part of impact, list it as $(top_builddir)/lib/<library> so that it can
## be detected as a dependency automatically.  Automake assumes libraries
## listed as -l<lib> are system libraries and does not detect them as
## dependencies.
src_Lcode_opti_Lsuperscalar_Lsuperscalar_LDADD = \
	-L$(top_builddir)/lib $(top_builddir)/lib/liblopti.a \
	$(top_builddir)/lib/libblock.a $(top_builddir)/lib/libsm.a \
	$(top_builddir)/lib/liblmdes.a $(top_builddir)/lib/libregalloc.a \
	$(top_builddir)/lib/libmarkpipe.a $(top_builddir)/lib/liblcode.a \
	$(top_builddir)/lib/libbdd.a $(top_builddir)/lib/libmspec.a \
	$(top_builddir)/lib/libproto.a $(top_builddir)/lib/libimpact.a -lm

## List any dependencies other than libraries under _DEPENDENDIES.  Defining
## this variable disables automatic dependency detection for this target, so
## you will need to relist dependencies that appear in _LDADD.
## src_Lcode_opti_Lsuperscalar_Lsuperscalar_DEPENDENCIES = 

## List any special compiler flags under _CFLAGS.
## src_Lcode_opti_Lsuperscalar_Lsuperscalar_CFLAGS = 

## List all source and header files under _SOURCES.  
src_Lcode_opti_Lsuperscalar_libsuperscalar_a_SOURCES = \
	$(src_Lcode_opti_Lsuperscalar_SRC)

## List any dependencies other than libraries under _DEPENDENDIES.  Defining
## this variable disables automatic dependency detection for this target, so
## you will need to relist dependencies that appear in _LDADD.
## src_Lcode_opti_Lsuperscalar_libsuperscalar_a_DEPENDENCIES = 

## List any special compiler flags under _CFLAGS.
## src_Lcode_opti_Lsuperscalar_libsuperscalar_a_CFLAGS = 
