## src/Lcode/codegen/Ltahoe/Makefile.am
## This file is (ultimately) included by the top level Makefile.am, so all
## files need to be specified relative to the top level directory.

## List the files generated in this directory.  Binaries go under _PRG,
## libraries under _LIB.  Higher level Makefiles use these variables
## unconditionally, so both _PRG and _LIB need to be defined.
src_Lcode_codegen_Ltahoe_PRG = src/Lcode/codegen/Ltahoe/Ltahoe
#src_Lcode_codegen_Ltahoe_LIB = src/Lcode/codegen/Ltahoe/libtahoe.a
src_Lcode_codegen_Ltahoe_LIB = 
src_Lcode_codegen_Ltahoe_BLT = src/Lcode/codegen/Ltahoe/ltahoe_table.c \
			       src/Lcode/codegen/Ltahoe/ltahoe_table.h
src_Lcode_codegen_Ltahoe_DST = src/Lcode/codegen/Ltahoe/table.pl \
			       src/Lcode/codegen/Ltahoe/ltahoe_table.src \
			       src/Lcode/codegen/Ltahoe/doxygen_config.doc \
			       src/Lcode/codegen/Ltahoe/doxygen_config.tag

## A target to support building everything under this subdirectory.  This
## should refer to anything defined above.  There should be two targets; one
## with and one without the trailing slash.  The trailing slash will allow
## the use of tab completion.
src/Lcode/codegen/Ltahoe src/Lcode/codegen/Ltahoe/: \
	include $(src_Lcode_codegen_Ltahoe_BLT) \
	$(src_Lcode_codegen_Ltahoe_PRG) $(src_Lcode_codegen_Ltahoe_LIB)

## A target to support cleaning everything under this subdirectory.
src/Lcode/codegen/Ltahoe/clean:
	rm -f $(top_builddir)/src/Lcode/codegen/Ltahoe/*.$(OBJEXT)
	rm -rf $(srcdir)/src/Lcode/codegen/Ltahoe/html
	rm -rf $(srcdir)/src/Lcode/codegen/Ltahoe/latex
	rm -f $(srcdir)/src/Lcode/codegen/Ltahoe/Ltahoe.tag
	rm -f $(src_Lcode_codegen_Ltahoe_PRG)
	rm -f $(src_Lcode_codegen_Ltahoe_LIB)
	rm -f $(src_Lcode_codegen_Ltahoe_BLT)

src_Lcode_codegen_Ltahoe_SRC = \
	$(src_Lcode_codegen_Ltahoe_BLT) \
	src/Lcode/codegen/Ltahoe/ltahoe_bitvec.h \
	src/Lcode/codegen/Ltahoe/ltahoe_codegen.c \
	src/Lcode/codegen/Ltahoe/ltahoe_completers.c \
	src/Lcode/codegen/Ltahoe/ltahoe_completers.h \
	src/Lcode/codegen/Ltahoe/ltahoe_main.c \
	src/Lcode/codegen/Ltahoe/ltahoe_main.h \
	src/Lcode/codegen/Ltahoe/ltahoe_op_query.c \
	src/Lcode/codegen/Ltahoe/ltahoe_op_query.h \
	src/Lcode/codegen/Ltahoe/ltahoe_redux.c \
	src/Lcode/codegen/Ltahoe/phase1_bitopt.c \
	src/Lcode/codegen/Ltahoe/phase1_bitopt.h \
	src/Lcode/codegen/Ltahoe/phase1_func.c \
	src/Lcode/codegen/Ltahoe/phase1_func.h \
	src/Lcode/codegen/Ltahoe/phase1_idiv.c \
	src/Lcode/codegen/Ltahoe/phase1_idiv.h \
	src/Lcode/codegen/Ltahoe/phase1_imath.c \
	src/Lcode/codegen/Ltahoe/phase1_imult.c \
	src/Lcode/codegen/Ltahoe/phase1_imult.h \
	src/Lcode/codegen/Ltahoe/phase1_imult_tab2k.h \
	src/Lcode/codegen/Ltahoe/phase1_imult_tab.h \
	src/Lcode/codegen/Ltahoe/phase1_opgen.c \
	src/Lcode/codegen/Ltahoe/phase1_opgen.h \
	src/Lcode/codegen/Ltahoe/phase1_param.c \
	src/Lcode/codegen/Ltahoe/phase1_param.h \
	src/Lcode/codegen/Ltahoe/phase1_pred.c \
	src/Lcode/codegen/Ltahoe/phase1_varargs.c \
	src/Lcode/codegen/Ltahoe/phase1_varargs.h \
	src/Lcode/codegen/Ltahoe/phase2_br_hint.c \
	src/Lcode/codegen/Ltahoe/phase2_br_hint.h \
	src/Lcode/codegen/Ltahoe/phase2_func.c \
	src/Lcode/codegen/Ltahoe/phase2_func.h \
	src/Lcode/codegen/Ltahoe/phase2_icache.c \
	src/Lcode/codegen/Ltahoe/phase2_icache.h \
	src/Lcode/codegen/Ltahoe/phase2_memstk.c \
	src/Lcode/codegen/Ltahoe/phase2_memstk.h \
	src/Lcode/codegen/Ltahoe/phase2_reg.c \
	src/Lcode/codegen/Ltahoe/phase2_reg.h \
	src/Lcode/codegen/Ltahoe/phase2_sync.c \
	src/Lcode/codegen/Ltahoe/phase2_sync.h \
	src/Lcode/codegen/Ltahoe/phase3_data.c \
	src/Lcode/codegen/Ltahoe/phase3_func.c \
	src/Lcode/codegen/Ltahoe/phase3.h \
	src/Lcode/codegen/Ltahoe/phase3_oper.c \
	src/Lcode/codegen/Ltahoe/phase3_oper.h \
	src/Lcode/codegen/Ltahoe/phase3_symbol.c \
	src/Lcode/codegen/Ltahoe/phase3_unwind.c \
	src/Lcode/codegen/Ltahoe/phase3_unwind.h \
	src/Lcode/codegen/Ltahoe/phase3_load.c

## A target to build the tag file for this subdirectory's documentation.
## The tag file contains information on every documented symbol so that
## documentation in other directories can be linked to documentation in
## this one.
$(srcdir)/src/Lcode/codegen/Ltahoe/Ltahoe.tag: \
		$(srcdir)/src/Lcode/codegen/Ltahoe/doxygen_config.tag \
		$(src_Lcode_codegen_Ltahoe_SRC)
	cd $(srcdir)/src/Lcode/codegen/Ltahoe && doxygen doxygen_config.tag

## A target to build all documentation under this subdirectory.  This
## builds the human readable documentation and links it to documentation
## in other directories.
src/Lcode/codegen/Ltahoe/doc $(srcdir)/src/Lcode/codegen/Ltahoe/html \
	$(srcdir)/src/Lcode/codegen/Ltahoe/latex: \
		$(srcdir)/src/Lcode/codegen/Ltahoe/doxygen_config.doc \
		$(srcdir)/src/Lcode/codegen/Ltahoe/Ltahoe.tag \
		$(srcdir)/src/Lcode/codegen/Mopti-ia64/libiamopti.tag \
		$(srcdir)/src/Lcode/opti/Lopti/Lopti.tag \
		$(srcdir)/src/Lcode/opti/Lblock/Lblock.tag \
		$(srcdir)/src/machine/Lmdes/liblmdes.tag \
		$(srcdir)/src/Lcode/codegen/Regalloc/libregalloc.tag \
		$(srcdir)/src/Lcode/performance/Lprobe/Lprobe.tag \
		$(srcdir)/src/Lcode/Lcode/liblcode.tag \
		$(srcdir)/src/bdd/libbdd.tag \
		$(srcdir)/src/machine/Mspec/libmspec.tag \
		$(srcdir)/src/library/libimpact.tag \
		$(srcdir)/src/Lcode/opti/Lmarkpipe/libmarkpipe.tag
	cd $(srcdir)/src/Lcode/codegen/Ltahoe && doxygen doxygen_config.doc

## List all include files that should be symlinked into include under 
## noinst_..._INC.  These include files are not installed when the user
## does 'make install'.  This is appropriate for most include files.  If
## you have headers that need to be installed, see src/library/Makefile.am
## for an example.
noinst_src_Lcode_codegen_Ltahoe_INC = \
	src/Lcode/codegen/Ltahoe/ltahoe_completers.h \
	src/Lcode/codegen/Ltahoe/ltahoe_main.h \
	src/Lcode/codegen/Ltahoe/ltahoe_op_query.h

## The class for the include files.  This is the directory under the include
## directory where the symlinks will be created.  This needs to have the same
## variable name as the _INC variable, only with a _CLASS suffix.
noinst_src_Lcode_codegen_Ltahoe_CLASS = Lcode

## List all source and header files under _SOURCES.  
src_Lcode_codegen_Ltahoe_Ltahoe_SOURCES = $(src_Lcode_codegen_Ltahoe_SRC)

## List any libraries needed when linking under _LDADD.  If a library is built
## as part of impact, list it as $(top_builddir)/lib/<library> so that it can
## be detected as a dependency automatically.  Automake assumes libraries
## listed as -l<lib> are system libraries and does not detect them as
## dependencies.
src_Lcode_codegen_Ltahoe_Ltahoe_LDADD = -L$(top_builddir)/lib \
					$(top_builddir)/lib/libiamopti.a \
					$(top_builddir)/lib/liblopti.a \
					$(top_builddir)/lib/libblock.a \
					$(top_builddir)/lib/liblmdes.a \
					$(top_builddir)/lib/libregalloc.a \
					$(top_builddir)/lib/libsm.a \
					$(top_builddir)/lib/liblcode.a \
					$(top_builddir)/lib/libbdd.a \
					$(top_builddir)/lib/libmspec.a \
					$(top_builddir)/lib/libimpact.a \
					$(top_builddir)/lib/libsoftpipe.a \
					$(top_builddir)/lib/libproto.a \
					$(top_builddir)/lib/libmarkpipe.a -lm

## List any dependencies other than libraries under _DEPENDENDIES.  Defining
## this variable disables automatic dependency detection for this target, so
## you will need to relist dependencies that appear in _LDADD.
## src_Lcode_codegen_Ltahoe_Ltahoe_DEPENDENCIES = 

## List any special compiler flags under _CFLAGS.
src_Lcode_codegen_Ltahoe_Ltahoe_CFLAGS = \
	-DIMPACT_FIX -I$(top_builddir)/src/Lcode/codegen/Ltahoe \
	-I$(abs_srcdir)/src/Lcode/codegen/Ltahoe $(AM_CFLAGS)

## List all source and header files under _SOURCES.  
#src_Lcode_codegen_Ltahoe_libtahoe_a_SOURCES = $(src_Lcode_codegen_Ltahoe_SRC)

## List any dependencies other than libraries under _DEPENDENDIES.  Defining
## this variable disables automatic dependency detection for this target, so
## you will need to relist dependencies that appear in _LDADD.
## src_Lcode_codegen_Ltahoe_libtahoe_a_DEPENDENCIES = 

## List any special compiler flags under _CFLAGS.
#src_Lcode_codegen_Ltahoe_libtahoe_a_CFLAGS = \
	-DIMPACT_FIX -I$(top_builddir)/src/Lcode/codegen/Ltahoe \
	-I$(abs_builddir)/src/Lcode/codegen/Ltahoe $(AM_CFLAGS)

$(src_Lcode_codegen_Ltahoe_BLT): \
		$(srcdir)/src/Lcode/codegen/Ltahoe/ltahoe_table.src \
		$(srcdir)/src/Lcode/codegen/Ltahoe/table.pl \
		$(top_builddir)/include/machine/m_tahoe.h
	$(srcdir)/src/Lcode/codegen/Ltahoe/table.pl -h $(top_builddir)/include/machine/m_tahoe.h -i $(srcdir)/src/Lcode/codegen/Ltahoe/ltahoe_table.src -o $(top_builddir)/src/Lcode/codegen/Ltahoe/ltahoe_table

