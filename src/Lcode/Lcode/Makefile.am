## src/Lcode/Lcode/Makefile.am
## This file is (ultimately) included by the top level Makefile.am, so all
## files need to be specified relative to the top level directory.

## List the files generated in this directory.  Binaries go under _PRG,
## libraries under _LIB.  Higher level Makefiles use these variables
## unconditionally, so both _PRG and _LIB need to be defined.
src_Lcode_Lcode_PRG = src/Lcode/Lcode/Lcode
src_Lcode_Lcode_LIB = src/Lcode/Lcode/liblcode.a
src_Lcode_Lcode_DST = src/Lcode/Lcode/doxygen_config.doc \
	src/Lcode/Lcode/doxygen_config.tag

## A target to support building everything under this subdirectory.  This
## should refer to anything defined above.  There should be two targets; one
## with and one without the trailing slash.  The trailing slash will allow
## the use of tab completion.
src/Lcode/Lcode src/Lcode/Lcode/: \
	include $(src_Lcode_Lcode_PRG) $(src_Lcode_Lcode_LIB)

## A target to support cleaning everything under this subdirectory.
src/Lcode/Lcode/clean:
	rm -f $(top_builddir)/src/Lcode/Lcode/*.$(OBJEXT)
	rm -rf $(srcdir)/src/Lcode/Lcode/html
	rm -rf $(srcdir)/src/Lcode/Lcode/latex
	rm -f $(srcdir)/src/Lcode/Lcode/liblcode.tag
	rm -f $(src_Lcode_Lcode_PRG) $(src_Lcode_Lcode_LIB)

src_Lcode_Lcode_SRC = \
	src/Lcode/Lcode/l_accspec.c src/Lcode/Lcode/l_accspec.h \
	src/Lcode/Lcode/l_addrpred.c src/Lcode/Lcode/l_appendix.c \
	src/Lcode/Lcode/l_appendix.h \
	src/Lcode/Lcode/l_bdf_cnt_dep.c src/Lcode/Lcode/l_bdf_cnt_dep.h \
	src/Lcode/Lcode/l_bdf_flow.c src/Lcode/Lcode/l_bdf_flow.h \
	src/Lcode/Lcode/l_bdf_graph.c src/Lcode/Lcode/l_bdf_graph.h \
	src/Lcode/Lcode/l_binaryio.c \
	src/Lcode/Lcode/l_binaryio.h src/Lcode/Lcode/l_callgraph.c \
	src/Lcode/Lcode/l_callgraph.h src/Lcode/Lcode/l_check.c \
	src/Lcode/Lcode/l_check.h src/Lcode/Lcode/l_code.c \
	src/Lcode/Lcode/l_codegen.c src/Lcode/Lcode/l_code.h \
	src/Lcode/Lcode/l_compare.c src/Lcode/Lcode/l_compare.h \
	src/Lcode/Lcode/l_constant.c src/Lcode/Lcode/l_constant.h \
	src/Lcode/Lcode/l_controlflow.c src/Lcode/Lcode/l_controlflow.h \
	src/Lcode/Lcode/l_dataflow.c src/Lcode/Lcode/l_dataflow.h \
	src/Lcode/Lcode/l_debug.h \
	src/Lcode/Lcode/l_dom.c src/Lcode/Lcode/l_dom.h \
	src/Lcode/Lcode/l_dominator.c \
	src/Lcode/Lcode/l_dominator.h src/Lcode/Lcode/l_error.c \
	src/Lcode/Lcode/l_error.h src/Lcode/Lcode/l_event.c \
	src/Lcode/Lcode/l_event.h src/Lcode/Lcode/l_extern.h \
	src/Lcode/Lcode/l_flags.h src/Lcode/Lcode/l_flagsio.c \
	src/Lcode/Lcode/l_graph.c src/Lcode/Lcode/l_graph.h \
	src/Lcode/Lcode/l_hashtbl.c src/Lcode/Lcode/l_hashtbl.h \
	src/Lcode/Lcode/l_indep_mem.c src/Lcode/Lcode/l_int_range.c \
	src/Lcode/Lcode/l_int_range.h src/Lcode/Lcode/l_intrinsic.c \
	src/Lcode/Lcode/l_intrinsic.h src/Lcode/Lcode/l_io.c \
	src/Lcode/Lcode/l_io.h src/Lcode/Lcode/l_lcode.c \
	src/Lcode/Lcode/l_lcode.h src/Lcode/Lcode/l_loop.c \
	src/Lcode/Lcode/l_loop.h src/Lcode/Lcode/l_main.h \
	src/Lcode/Lcode/l_mcode.c src/Lcode/Lcode/l_mcode.h \
	src/Lcode/Lcode/l_memory.c src/Lcode/Lcode/l_memory.h \
	src/Lcode/Lcode/l_misc.c src/Lcode/Lcode/l_opc.h \
	src/Lcode/Lcode/l_operand_symbol.c src/Lcode/Lcode/l_operand_symbol.h \
	src/Lcode/Lcode/l_opti_count.c src/Lcode/Lcode/l_opti_count.h \
	src/Lcode/Lcode/l_pce.c src/Lcode/Lcode/l_pce.h \
	src/Lcode/Lcode/l_pred_flow.c src/Lcode/Lcode/l_pred_flow.h \
	src/Lcode/Lcode/l_pred_graph.c src/Lcode/Lcode/l_pred_graph.h \
	src/Lcode/Lcode/l_predicate.c src/Lcode/Lcode/l_predicate.h \
	src/Lcode/Lcode/l_pred_query.c src/Lcode/Lcode/l_process_file.c \
	src/Lcode/Lcode/l_process_file.h src/Lcode/Lcode/l_region.c \
	src/Lcode/Lcode/l_region.h src/Lcode/Lcode/l_safe.c \
	src/Lcode/Lcode/l_safe.h src/Lcode/Lcode/l_speculate.c \
	src/Lcode/Lcode/l_speculate.h src/Lcode/Lcode/l_stat.c \
	src/Lcode/Lcode/l_ssa.c src/Lcode/Lcode/l_ssa.h \
	src/Lcode/Lcode/l_stat.h src/Lcode/Lcode/l_symbol.c \
	src/Lcode/Lcode/l_symbol.h src/Lcode/Lcode/l_sync.c \
	src/Lcode/Lcode/l_sync.h src/Lcode/Lcode/l_time.c \
	src/Lcode/Lcode/l_time.h src/Lcode/Lcode/main/l_main.c \
	src/Lcode/Lcode/r_dataflow.c src/Lcode/Lcode/r_dataflow.h \
	src/Lcode/Lcode/r_df_query.c src/Lcode/Lcode/r_pce_flow.c \
	src/Lcode/Lcode/r_pce_flow.h

## A target to build the tag file for this subdirectory's documentation.
## The tag file contains information on every documented symbol so that
## documentation in other directories can be linked to documentation in
## this one.
$(srcdir)/src/Lcode/Lcode/liblcode.tag: \
		$(srcdir)/src/Lcode/Lcode/doxygen_config.tag \
		$(src_Lcode_Lcode_SRC)
	cd $(srcdir)/src/Lcode/Lcode && doxygen doxygen_config.tag

## A target to build all documentation under this subdirectory.  This
## builds the human readable documentation and links it to documentation
## in other directories.
src/Lcode/Lcode/doc $(srcdir)/src/Lcode/Lcode/html \
	$(srcdir)/src/Lcode/Lcode/latex: \
		$(srcdir)/src/Lcode/Lcode/doxygen_config.doc \
		$(srcdir)/src/Lcode/Lcode/liblcode.tag \
		$(srcdir)/src/machine/Mspec/libmspec.tag \
		$(srcdir)/src/machine/Lmdes/liblmdes.tag \
		$(srcdir)/src/library/libimpact.tag \
		$(srcdir)/src/bdd/libbdd.tag
	cd $(srcdir)/src/Lcode/Lcode && doxygen doxygen_config.doc

## List all include files that should be symlinked into include under 
## noinst_..._INC.  These include files are not installed when the user
## does 'make install'.  This is appropriate for most include files.  If
## you have headers that need to be installed, see src/library/Makefile.am
## for an example.
noinst_src_Lcode_Lcode_INC = \
	src/Lcode/Lcode/l_accspec.h \
	src/Lcode/Lcode/l_appendix.h src/Lcode/Lcode/l_binaryio.h \
	src/Lcode/Lcode/l_callgraph.h src/Lcode/Lcode/l_check.h \
	src/Lcode/Lcode/l_code.h src/Lcode/Lcode/l_compare.h \
	src/Lcode/Lcode/l_constant.h src/Lcode/Lcode/l_controlflow.h \
	src/Lcode/Lcode/l_dataflow.h src/Lcode/Lcode/l_debug.h \
	src/Lcode/Lcode/l_dom.h \
	src/Lcode/Lcode/l_dominator.h src/Lcode/Lcode/l_error.h \
	src/Lcode/Lcode/l_event.h src/Lcode/Lcode/l_extern.h \
	src/Lcode/Lcode/l_flags.h src/Lcode/Lcode/l_graph.h \
	src/Lcode/Lcode/l_hashtbl.h src/Lcode/Lcode/l_int_range.h \
	src/Lcode/Lcode/l_intrinsic.h src/Lcode/Lcode/l_io.h \
	src/Lcode/Lcode/l_lcode.h src/Lcode/Lcode/l_loop.h \
	src/Lcode/Lcode/l_main.h src/Lcode/Lcode/l_mcode.h \
	src/Lcode/Lcode/l_memory.h src/Lcode/Lcode/l_opc.h \
	src/Lcode/Lcode/l_operand_symbol.h src/Lcode/Lcode/l_opti_count.h \
	src/Lcode/Lcode/l_pce.h	src/Lcode/Lcode/l_pred_flow.h \
	src/Lcode/Lcode/l_pred_graph.h src/Lcode/Lcode/l_predicate.h \
	src/Lcode/Lcode/l_process_file.h src/Lcode/Lcode/l_region.h \
	src/Lcode/Lcode/l_safe.h src/Lcode/Lcode/l_speculate.h \
	src/Lcode/Lcode/l_ssa.h \
	src/Lcode/Lcode/l_stat.h src/Lcode/Lcode/l_symbol.h \
	src/Lcode/Lcode/l_sync.h src/Lcode/Lcode/l_time.h \
	src/Lcode/Lcode/r_dataflow.h src/Lcode/Lcode/r_pce_flow.h

## The class for the include files.  This is the directory under the include
## directory where the symlinks will be created.  This needs to have the same
## variable name as the _INC variable, only with a _CLASS suffix.
noinst_src_Lcode_Lcode_CLASS = Lcode

## List all source and header files under _SOURCES.  
src_Lcode_Lcode_liblcode_a_SOURCES = $(src_Lcode_Lcode_SRC)

## List all source and header files under _SOURCES.  
src_Lcode_Lcode_Lcode_SOURCES = $(src_Lcode_Lcode_SRC)

## List any libraries needed when linking under _LDADD.  If a library is built
## as part of impact, list it as $(top_builddir)/lib/<library> so that it can
## be detected as a dependency automatically.  Automake assumes libraries
## listed as -l<lib> are system libraries and does not detect them as
## dependencies.
src_Lcode_Lcode_Lcode_LDADD = -L$(top_builddir)/lib \
			      $(top_builddir)/lib/libmspec.a \
			      $(top_builddir)/lib/liblmdes.a \
			      $(top_builddir)/lib/libimpact.a \
			      $(top_builddir)/lib/libbdd.a \
                              $(top_builddir)/lib/libproto.a -lm

## List any dependencies other than libraries under _DEPENDENDIES.  Defining
## this variable disables automatic dependency detection for this target, so
## you will need to relist dependencies that appear in _LDADD.
## src_Lcode_Lcode_liblcode_a_DEPENDENCIES = 

## List any dependencies other than libraries under _DEPENDENDIES.  Defining
## this variable disables automatic dependency detection for this target, so
## you will need to relist dependencies that appear in _LDADD.
## src_Lcode_Lcode_Lcode_DEPENDENCIES = 

## List any special compiler flags under _CFLAGS.
## src_Lcode_Lcode_liblcode_a_CFLAGS = 

## List any special compiler flags under _CFLAGS.
## src_Lcode_Lcode_Lcode_CFLAGS = 

