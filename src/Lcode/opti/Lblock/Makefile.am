## src/Lcode/opti/Lblock/Makefile.am
## This file is (ultimately) included by the top level Makefile.am, so all
## files need to be specified relative to the top level directory.

## List the files generated in this directory.  Binaries go under _PRG,
## libraries under _LIB.  Higher level Makefiles use these variables
## unconditionally, so both _PRG and _LIB need to be defined.
src_Lcode_opti_Lblock_PRG = src/Lcode/opti/Lblock/Lblock
src_Lcode_opti_Lblock_LIB = src/Lcode/opti/Lblock/libblock.a
src_Lcode_opti_Lblock_DST = src/Lcode/opti/Lblock/doxygen_config.doc \
	src/Lcode/opti/Lblock/doxygen_config.tag

## A target to support building everything under this subdirectory.  This
## should refer to anything defined above.  There should be two targets; one
## with and one without the trailing slash.  The trailing slash will allow
## the use of tab completion.
src/Lcode/opti/Lblock src/Lcode/opti/Lblock/: \
	include $(src_Lcode_opti_Lblock_PRG) $(src_Lcode_opti_Lblock_BLT)

## A target to support cleaning everything under this subdirectory.
src/Lcode/opti/Lblock/clean:
	rm -f $(top_builddir)/src/Lcode/opti/Lblock/*.$(OBJEXT)
	rm -rf $(srcdir)/src/Lcode/opti/Lblock/html
	rm -rf $(srcdir)/src/Lcode/opti/Lblock/latex
	rm -f $(srcdir)/src/Lcode/opti/Lblock/Lblock.tag
	rm -f $(src_Lcode_opti_Lblock_PRG)
	rm -f $(src_Lcode_opti_Lblock_LIB)

src_Lcode_opti_Lblock_SRC = \
	src/Lcode/opti/Lblock/lb_b_internal.h \
	src/Lcode/opti/Lblock/lb_block.c \
	src/Lcode/opti/Lblock/lb_block.h \
	src/Lcode/opti/Lblock/lb_block_init.c \
	src/Lcode/opti/Lblock/lb_br_split.c \
	src/Lcode/opti/Lblock/lb_codegen.c \
	src/Lcode/opti/Lblock/lb_elim.c \
	src/Lcode/opti/Lblock/lb_explicit_br.c \
	src/Lcode/opti/Lblock/lb_flow.h \
	src/Lcode/opti/Lblock/lb_graph.c \
	src/Lcode/opti/Lblock/lb_graph.h \
	src/Lcode/opti/Lblock/lb_hb_block_enum.c \
	src/Lcode/opti/Lblock/lb_hb_block_enum.h \
	src/Lcode/opti/Lblock/lb_hb_hyperblock_former.c \
	src/Lcode/opti/Lblock/lb_hb_hyperblock_former_full.c \
	src/Lcode/opti/Lblock/lb_hb_hyperblock_former_WCET.c \
	src/Lcode/opti/Lblock/lb_hb_hyperblock.h \
	src/Lcode/opti/Lblock/lb_hb_merge.c \
	src/Lcode/opti/Lblock/lb_hb_merge.h \
	src/Lcode/opti/Lblock/lb_hb_misc.c \
	src/Lcode/opti/Lblock/lb_hb_misc.h \
	src/Lcode/opti/Lblock/lb_hb_path.c \
	src/Lcode/opti/Lblock/lb_hb_path.h \
	src/Lcode/opti/Lblock/lb_hb_peel.c \
	src/Lcode/opti/Lblock/lb_hb_peel.h \
	src/Lcode/opti/Lblock/lb_hb_region_general.c \
	src/Lcode/opti/Lblock/lb_hb_region_general.h \
	src/Lcode/opti/Lblock/lb_hb_region_hammock.c \
	src/Lcode/opti/Lblock/lb_hb_region_hammock.h \
	src/Lcode/opti/Lblock/lb_hb_region_loop.c \
	src/Lcode/opti/Lblock/lb_hb_region_loop.h \
	src/Lcode/opti/Lblock/lb_hb_stack.c \
	src/Lcode/opti/Lblock/lb_hb_stack.h \
	src/Lcode/opti/Lblock/lb_mod_loop.c \
	src/Lcode/opti/Lblock/lb_peel.c \
	src/Lcode/opti/Lblock/lb_peel.h \
	src/Lcode/opti/Lblock/lb_pred_tools.c \
	src/Lcode/opti/Lblock/lb_pred_tools.h \
	src/Lcode/opti/Lblock/lb_register_branch.c \
	src/Lcode/opti/Lblock/lb_sb_layoutfn.c \
	src/Lcode/opti/Lblock/lb_sb_make_traceregions.c \
	src/Lcode/opti/Lblock/lb_sb_order_traceregions.c \
	src/Lcode/opti/Lblock/lb_sb_superblock_former.c \
	src/Lcode/opti/Lblock/lb_sb_superblock.h \
	src/Lcode/opti/Lblock/lb_strict_bb.c \
	src/Lcode/opti/Lblock/lb_tail.c \
	src/Lcode/opti/Lblock/lb_tail.h \
	src/Lcode/opti/Lblock/lb_tool.c \
	src/Lcode/opti/Lblock/lb_tool.h \
	src/Lcode/opti/Lblock/lb_traceregion.c \
	src/Lcode/opti/Lblock/lb_traceregion.h

## A target to build the tag file for this subdirectory's documentation.
## The tag file contains information on every documented symbol so that
## documentation in other directories can be linked to documentation in
## this one.
$(srcdir)/src/Lcode/opti/Lblock/Lblock.tag: \
		$(srcdir)/src/Lcode/opti/Lblock/doxygen_config.tag \
		$(src_Lcode_opti_Lblock_SRC)
	cd $(srcdir)/src/Lcode/opti/Lblock && doxygen doxygen_config.tag

## A target to build all documentation under this subdirectory.  This
## builds the human readable documentation and links it to documentation
## in other directories.
src/Lcode/opti/Lblock/doc $(srcdir)/src/Lcode/opti/Lblock/html \
	$(srcdir)/src/Lcode/opti/Lblock/latex: \
		$(srcdir)/src/Lcode/opti/Lblock/doxygen_config.doc \
		$(srcdir)/src/Lcode/opti/Lblock/Lblock.tag \
		$(srcdir)/src/Lcode/opti/Lopti/Lopti.tag \
		$(srcdir)/src/Lcode/sched/SM/libsm.tag \
		$(srcdir)/src/Lcode/Lcode/liblcode.tag \
		$(srcdir)/src/machine/Mspec/libmspec.tag \
		$(srcdir)/src/bdd/libbdd.tag \
		$(srcdir)/src/library/libimpact.tag \
		$(srcdir)/src/machine/Lmdes/liblmdes.tag
	cd $(srcdir)/src/Lcode/opti/Lblock && doxygen doxygen_config.doc

## List all include files that should be symlinked into include under 
## noinst_..._INC.  These include files are not installed when the user
## does 'make install'.  This is appropriate for most include files.  If
## you have headers that need to be installed, see src/library/Makefile.am
## for an example.
noinst_src_Lcode_opti_Lblock_INC = \
	src/Lcode/opti/Lblock/lb_block.h src/Lcode/opti/Lblock/lb_flow.h

## The class for the include files.  This is the directory under the include
## directory where the symlinks will be created.  This needs to have the same
## variable name as the _INC variable, only with a _CLASS suffix.
noinst_src_Lcode_opti_Lblock_CLASS = Lcode

## List all source and header files under _SOURCES.  
src_Lcode_opti_Lblock_Lblock_SOURCES = $(src_Lcode_opti_Lblock_SRC)

## List any libraries needed when linking under _LDADD.  If a library is built
## as part of impact, list it as $(top_builddir)/lib/<library> so that it can
## be detected as a dependency automatically.  Automake assumes libraries
## listed as -l<lib> are system libraries and does not detect them as
## dependencies.
src_Lcode_opti_Lblock_Lblock_LDADD = \
	-L$(top_builddir)/lib \
	$(top_builddir)/lib/libsm.a \
	$(top_builddir)/lib/liblcode.a \
	$(top_builddir)/lib/liblopti.a \
	$(top_builddir)/lib/libmspec.a \
	$(top_builddir)/lib/libbdd.a \
	$(top_builddir)/lib/libimpact.a \
	$(top_builddir)/lib/libproto.a \
	$(top_builddir)/lib/liblmdes.a -lm

## List any dependencies other than libraries under _DEPENDENDIES.  Defining
## this variable disables automatic dependency detection for this target, so
## you will need to relist dependencies that appear in _LDADD.
## src_Lcode_opti_Lblock_Lblock_DEPENDENCIES = 

## List any special compiler flags under _CFLAGS.
## src_Lcode_opti_Lblock_Lblock_CFLAGS = 

## List all source and header files under _SOURCES.  
src_Lcode_opti_Lblock_libblock_a_SOURCES = $(src_Lcode_opti_Lblock_SRC)

## List any dependencies other than libraries under _DEPENDENDIES.  Defining
## this variable disables automatic dependency detection for this target, so
## you will need to relist dependencies that appear in _LDADD.
## src_Lcode_opti_Lblock_libblock_a_DEPENDENCIES = 

## List any special compiler flags under _CFLAGS.
## src_Lcode_opti_Lblock_libblock_a_CFLAGS = 
