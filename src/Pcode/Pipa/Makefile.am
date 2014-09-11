## src/Pcode/Pipa/Makefile.am
## This file is (ultimately) included by the top level Makefile.am, so all
## files need to be specified relative to the top level directory.

## List the files generated in this directory.  Binaries go under _PRG,
## libraries under _LIB.  Higher level Makefiles use these variables
## unconditionally, so both _PRG and _LIB need to be defined.
src_Pcode_Pipa_PRG = src/Pcode/Pipa/Pipa
src_Pcode_Pipa_LIB = 
src_Pcode_Pipa_DST = src/Pcode/Pipa/doxygen_config.doc \
	src/Pcode/Pipa/doxygen_config.tag

## A target to support building everything under this subdirectory.  This
## should refer to anything defined above.  There should be two targets; one
## with and one without the trailing slash.  The trailing slash will allow
## the use of tab completion.
src/Pcode/Pipa src/Pcode/Pipa/: \
	include $(src_Pcode_Pipa_PRG) $(src_Pcode_Pipa_LIB)

## A target to support cleaning everything under this subdirectory.
src/Pcode/Pipa/clean:
	rm -f $(top_builddir)/src/Pcode/Pipa/*.$(OBJEXT)
	rm -f $(src_Pcode_Pipa_PRG)

## List all source and header files under _SOURCES.  
src_Pcode_Pipa_Pipa_SOURCES = \
	src/Pcode/Pipa/do_pcode.c src/Pcode/Pipa/dyn_array.c \
	src/Pcode/Pipa/dyn_array.h src/Pcode/Pipa/pipa_callgraph.c \
	src/Pcode/Pipa/pipa_callgraph.h src/Pcode/Pipa/pipa_callsite.c \
	src/Pcode/Pipa/pipa_callsite.h src/Pcode/Pipa/pipa_common.c \
	src/Pcode/Pipa/pipa_common.h src/Pcode/Pipa/pipa_consg.c \
	src/Pcode/Pipa/pipa_consg_construct.c \
	src/Pcode/Pipa/pipa_consg_construct.h \
	src/Pcode/Pipa/pipa_consg_fdvs_effect_analysis.c \
	src/Pcode/Pipa/pipa_consg_fdvs.h \
	src/Pcode/Pipa/pipa_consg_fdvs_remove_redundancy.c \
	src/Pcode/Pipa/pipa_consg_fdvs_solve_fully.c \
	src/Pcode/Pipa/pipa_consg_fi_effect_analysis.c \
	src/Pcode/Pipa/pipa_consg_fi.h \
	src/Pcode/Pipa/pipa_consg_fi_remove_redundancy.c \
	src/Pcode/Pipa/pipa_consg_fi_solve_fully.c \
	src/Pcode/Pipa/pipa_consg.h src/Pcode/Pipa/pipa_driver.c \
	src/Pcode/Pipa/pipa_driver.h src/Pcode/Pipa/pipa_driver_utils.c \
	src/Pcode/Pipa/pipa_driver_utils.h \
	src/Pcode/Pipa/pipa_escape_analysis.c \
	src/Pcode/Pipa/pipa_escape_analysis.h \
	src/Pcode/Pipa/pipa_func_limit.c src/Pcode/Pipa/pipa_graph.c \
	src/Pcode/Pipa/pipa_graph.h src/Pcode/Pipa/pipa_hash.c \
	src/Pcode/Pipa/pipa_hash.h src/Pcode/Pipa/pipa_misc_utils.c \
	src/Pcode/Pipa/pipa_misc_utils.h src/Pcode/Pipa/pipa_options.c \
	src/Pcode/Pipa/pipa_options.h src/Pcode/Pipa/pipa_pcode2pointsto.c \
	src/Pcode/Pipa/pipa_pcode2pointsto.h \
	src/Pcode/Pipa/pipa_print_graph.c src/Pcode/Pipa/pipa_print_graph.h \
	src/Pcode/Pipa/pipa_program.c src/Pcode/Pipa/pipa_program.h \
	src/Pcode/Pipa/pipa_stats.c src/Pcode/Pipa/pipa_symbols.c \
	src/Pcode/Pipa/pipa_symbols.h src/Pcode/Pipa/pipa_sync_gen.c \
	src/Pcode/Pipa/pipa_sync_gen.h

## A target to build the tag file for this subdirectory's documentation.
## The tag file contains information on every documented symbol so that
## documentation in other directories can be linked to documentation in
## this one.
$(srcdir)/src/Pcode/Pipa/Pipa.tag: \
		$(srcdir)/src/Pcode/Pipa/doxygen_config.tag \
		$(src_Pcode_Pipa_Pipa_SOURCES)
	cd $(srcdir)/src/Pcode/Pipa && doxygen doxygen_config.tag

## A target to build all documentation under this subdirectory.  This
## build the human readable documentation and links it to documentation
## in other directories.
src/Pcode/Pipa/doc $(srcdir)/src/Pcode/Pipa/html \
	$(srcdir)/src/Pcode/Pipa/latex: \
		$(srcdir)/src/Pcode/Pipa/doxygen_config.doc \
		$(srcdir)/src/Pcode/Pcode/libpcode.tag \
		$(srcdir)/src/library/libimpact.tag
	cd $(srcdir)/src/Pcode/Pipa && doxygen doxygen_config.doc

## List all include files that should be symlinked into include under 
## noinst_..._INC.  These include files are not installed when the user
## does 'make install'.  This is appropriate for most include files.  If
## you have headers that need to be installed, see src/library/Makefile.am
## for an example.
## noinst_src_Pcode_Pipa_INC =

## The class for the include files.  This is the directory under the include
## directory where the symlinks will be created.  This needs to have the same
## variable name as the _INC variable, only with a _CLASS suffix.
## noinst_src_Pcode_Pipa_CLASS = Pcode

## List any libraries needed when linking under _LDADD.  If a library is built
## as part of impact, list it as $(top_builddir)/lib/<library> so that it can
## be detected as a dependency automatically.  Automake assumes libraries
## listed as -l<lib> are system libraries and does not detect them as
## dependencies.
src_Pcode_Pipa_Pipa_LDADD = -L$(top_builddir)/lib \
	$(top_builddir)/lib/libpcode_ssa_new.a  \
	$(top_builddir)/lib/libpcode_cf.a \
	$(top_builddir)/lib/libpcode.a $(top_builddir)/lib/libimpact.a

## List any dependencies other than libraries under _DEPENDENDIES.  Defining
## this variable disables automatic dependency detection for this target, so
## you will need to relist dependencies that appear in _LDADD.
## src_Pcode_Pipa_Pipa_DEPENDENCIES = 

## List any special compiler flags under _CFLAGS.
src_Pcode_Pipa_Pipa_CFLAGS = -I$(srcdir)/src/Pcode/Pipa $(AM_CFLAGS)
