## src/Lcode/opti/Lopti/Makefile.am
## This file is (ultimately) included by the top level Makefile.am, so all
## files need to be specified relative to the top level directory.

## List the files generated in this directory.  Binaries go under _PRG,
## libraries under _LIB.  Higher level Makefiles use these variables
## unconditionally, so both _PRG and _LIB need to be defined.
src_Lcode_opti_Lopti_PRG = src/Lcode/opti/Lopti/Lopti
src_Lcode_opti_Lopti_LIB = src/Lcode/opti/Lopti/liblopti.a
src_Lcode_opti_Lopti_DST = src/Lcode/opti/Lopti/doxygen_config.doc \
	src/Lcode/opti/Lopti/doxygen_config.tag

## A target to support building everything under this subdirectory.  This
## should refer to anything defined above.  There should be two targets; one
## with and one without the trailing slash.  The trailing slash will allow
## the use of tab completion.
src/Lcode/opti/Lopti src/Lcode/opti/Lopti/: \
	include $(src_Lcode_opti_Lopti_PRG) $(src_Lcode_opti_Lopti_LIB)

## A target to support cleaning everything under this subdirectory.
src/Lcode/opti/Lopti/clean:
	rm -f $(top_builddir)/src/Lcode/opti/Lopti/*.$(OBJEXT)
	rm -rf $(srcdir)/src/Lcode/opti/Lopti/html
	rm -rf $(srcdir)/src/Lcode/opti/Lopti/latex
	rm -f $(srcdir)/src/Lcode/opti/Lopti/Lopti.tag
	rm -f $(src_Lcode_opti_Lopti_PRG)
	rm -f $(src_Lcode_opti_Lopti_LIB)

## List all source under _SRC so that we don't have to duplicate the entire
## list when defining _SOURCES for the program and library.
src_Lcode_opti_Lopti_SRC = \
	src/Lcode/opti/Lopti/l_benchmark_opti.c \
	src/Lcode/opti/Lopti/l_branch.c \
	src/Lcode/opti/Lopti/l_branch.h \
	src/Lcode/opti/Lopti/l_codegen.c \
	src/Lcode/opti/Lopti/l_danger.c \
	src/Lcode/opti/Lopti/l_danger.h \
	src/Lcode/opti/Lopti/l_disjvreg.c \
	src/Lcode/opti/Lopti/l_disjvreg.h \
	src/Lcode/opti/Lopti/l_evaluate.c \
	src/Lcode/opti/Lopti/l_global_driver.c \
	src/Lcode/opti/Lopti/l_global_opti.c \
	src/Lcode/opti/Lopti/l_global_opti.h \
	src/Lcode/opti/Lopti/l_jump_driver.c \
	src/Lcode/opti/Lopti/l_jump_expand.c \
	src/Lcode/opti/Lopti/l_jump_opti.c \
	src/Lcode/opti/Lopti/l_jump_opti.h \
	src/Lcode/opti/Lopti/l_local_driver.c \
	src/Lcode/opti/Lopti/l_local_opti.c \
	src/Lcode/opti/Lopti/l_local_opti.h \
	src/Lcode/opti/Lopti/l_loop_driver.c \
	src/Lcode/opti/Lopti/l_loop_opti.c \
	src/Lcode/opti/Lopti/l_loop_opti.h \
	src/Lcode/opti/Lopti/l_memflow_opti.c \
	src/Lcode/opti/Lopti/l_memflow_opti.h \
	src/Lcode/opti/Lopti/l_opti_functions.c \
	src/Lcode/opti/Lopti/l_opti_functions.h \
	src/Lcode/opti/Lopti/l_opti.h \
	src/Lcode/opti/Lopti/l_optimize.c \
	src/Lcode/opti/Lopti/l_opti_parms.c \
	src/Lcode/opti/Lopti/l_opti_predicates.c \
	src/Lcode/opti/Lopti/l_opti_predicates.h \
	src/Lcode/opti/Lopti/l_opti_tag_load.c \
	src/Lcode/opti/Lopti/l_PCE_driver.c \
	src/Lcode/opti/Lopti/l_PCE_opti.c \
	src/Lcode/opti/Lopti/l_PCE_opti.h \
	src/Lcode/opti/Lopti/l_pred_opti.c \
	src/Lcode/opti/Lopti/l_pred_opti.h \
	src/Lcode/opti/Lopti/l_promotion.c \
	src/Lcode/opti/Lopti/l_promotion.h \
	src/Lcode/opti/Lopti/l_unification.c

## A target to build the tag file for this subdirectory's documentation.
## The tag file contains information on every documented symbol so that
## documentation in other directories can be linked to documentation in
## this one.
$(srcdir)/src/Lcode/opti/Lopti/Lopti.tag: \
		$(srcdir)/src/Lcode/opti/Lopti/doxygen_config.tag \
		$(src_Lcode_opti_Lopti_SRC)
	cd $(srcdir)/src/Lcode/opti/Lopti && doxygen doxygen_config.tag

## A target to build all documentation under this subdirectory.  This
## builds the human readable documentation and links it to documentation
## in other directories.
src/Lcode/opti/Lopti/doc $(srcdir)/src/Lcode/opti/Lopti/html \
	$(srcdir)/src/Lcode/opti/Lopti/latex: \
		$(srcdir)/src/Lcode/opti/Lopti/doxygen_config.doc \
		$(srcdir)/src/Lcode/opti/Lopti/Lopti.tag \
		$(srcdir)/src/Lcode/opti/Lblock/Lblock.tag \
		$(srcdir)/src/Lcode/Lcode/liblcode.tag \
		$(srcdir)/src/bdd/libbdd.tag \
		$(srcdir)/src/machine/Lmdes/liblmdes.tag \
		$(srcdir)/src/machine/Mspec/libmspec.tag \
		$(srcdir)/src/library/libimpact.tag
	cd $(srcdir)/src/Lcode/opti/Lopti && doxygen doxygen_config.doc

## List all include files that should be symlinked into include under 
## noinst_..._INC.  These include files are not installed when the user
## does 'make install'.  This is appropriate for most include files.  If
## you have headers that need to be installed, see src/library/Makefile.am
## for an example.
noinst_src_Lcode_opti_Lopti_INC = \
	src/Lcode/opti/Lopti/l_branch.h src/Lcode/opti/Lopti/l_danger.h \
	src/Lcode/opti/Lopti/l_disjvreg.h \
	src/Lcode/opti/Lopti/l_global_opti.h \
	src/Lcode/opti/Lopti/l_jump_opti.h \
	src/Lcode/opti/Lopti/l_local_opti.h \
	src/Lcode/opti/Lopti/l_loop_opti.h \
	src/Lcode/opti/Lopti/l_memflow_opti.h \
	src/Lcode/opti/Lopti/l_opti_functions.h \
	src/Lcode/opti/Lopti/l_opti.h \
	src/Lcode/opti/Lopti/l_opti_predicates.h \
	src/Lcode/opti/Lopti/l_PCE_opti.h src/Lcode/opti/Lopti/l_pred_opti.h \
	src/Lcode/opti/Lopti/l_promotion.h

## The class for the include files.  This is the directory under the include
## directory where the symlinks will be created.  This needs to have the same
## variable name as the _INC variable, only with a _CLASS suffix.
noinst_src_Lcode_opti_Lopti_CLASS = Lcode

## List all source and header files under _SOURCES.  
src_Lcode_opti_Lopti_Lopti_SOURCES = $(src_Lcode_opti_Lopti_SRC)

## List any libraries needed when linking under _LDADD.  If a library is built
## as part of impact, list it as $(top_builddir)/lib/<library> so that it can
## be detected as a dependency automatically.  Automake assumes libraries
## listed as -l<lib> are system libraries and does not detect them as
## dependencies.
src_Lcode_opti_Lopti_Lopti_LDADD = -L$(top_builddir)/lib \
				   $(top_builddir)/lib/libblock.a \
				   $(top_builddir)/lib/liblcode.a \
				   $(top_builddir)/lib/libbdd.a \
				   $(top_builddir)/lib/liblmdes.a \
				   $(top_builddir)/lib/libmspec.a \
				   $(top_builddir)/lib/libproto.a \
				   $(top_builddir)/lib/libimpact.a -lm

## List any dependencies other than libraries under _DEPENDENDIES.  Defining
## this variable disables automatic dependency detection for this target, so
## you will need to relist dependencies that appear in _LDADD.
## src_Lcode_opti_Lopti_Lopti_DEPENDENCIES = 

## List any special compiler flags under _CFLAGS.
## src_Lcode_opti_Lopti_Lopti_CFLAGS = 

## List all source and header files under _SOURCES.  
src_Lcode_opti_Lopti_liblopti_a_SOURCES = $(src_Lcode_opti_Lopti_SRC)

## List any dependencies other than libraries under _DEPENDENDIES.  Defining
## this variable disables automatic dependency detection for this target, so
## you will need to relist dependencies that appear in _LDADD.
## src_Lcode_opti_Lopti_liblopti_a_DEPENDENCIES = 

## List any special compiler flags under _CFLAGS.
## src_Lcode_opti_Lopti_liblopti_a_CFLAGS = 
