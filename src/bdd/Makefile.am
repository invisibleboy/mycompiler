## src/bdd/Makefile.am
## This file is (ultimately) included by the top level Makefile.am, so all
## files need to be specified relative to the top level directory.

## List the files generated in this directory.  Binaries go under _PRG,
## libraries under _LIB.  Higher level Makefiles use these variables
## unconditionally, so both _PRG and _LIB need to be defined.
src_bdd_PRG =
src_bdd_LIB = src/bdd/libbdd.a
src_bdd_DST = src/bdd/doxygen_config.doc src/bdd/doxygen_config.tag

## A target to support building everything under this subdirectory.  This
## should refer to anything defined above.  There should be two targets; one
## with and one without the trailing slash.  The trailing slash will allow
## the use of tab completion.
src/bdd src/bdd/: include $(src_bdd_LIB)

## A target to support cleaning everything under this subdirectory.
src/bdd/clean:
	rm -f $(top_builddir)/src/bdd/cudd/*.$(OBJEXT)
	rm -f $(top_builddir)/src/bdd/mtr/*.$(OBJEXT)
	rm -f $(top_builddir)/src/bdd/st/*.$(OBJEXT)
	rm -f $(top_builddir)/src/bdd/util/*.$(OBJEXT)
	rm -rf $(srcdir)/src/bdd/html
	rm -rf $(srcdir)/src/bdd/latex
	rm -f $(srcdir)/src/bdd/libbdd.tag
	rm -f $(src_bdd_LIB)

## List all source and header files under _SOURCES.  
src_bdd_libbdd_a_SOURCES = \
	src/bdd/cudd/cuddAddAbs.c src/bdd/cudd/cuddAddApply.c \
	src/bdd/cudd/cuddAddFind.c src/bdd/cudd/cuddAddInv.c \
	src/bdd/cudd/cuddAddIte.c src/bdd/cudd/cuddAddNeg.c \
	src/bdd/cudd/cuddAddWalsh.c src/bdd/cudd/cuddAndAbs.c \
	src/bdd/cudd/cuddAnneal.c src/bdd/cudd/cuddApa.c \
	src/bdd/cudd/cuddAPI.c src/bdd/cudd/cuddApprox.c \
	src/bdd/cudd/cuddBddAbs.c src/bdd/cudd/cuddBddCorr.c \
	src/bdd/cudd/cuddBddIte.c src/bdd/cudd/cuddBridge.c \
	src/bdd/cudd/cuddCache.c src/bdd/cudd/cuddCheck.c \
	src/bdd/cudd/cuddClip.c src/bdd/cudd/cuddCof.c \
	src/bdd/cudd/cuddCompose.c src/bdd/cudd/cuddDecomp.c \
	src/bdd/cudd/cuddEssent.c src/bdd/cudd/cuddExact.c \
	src/bdd/cudd/cuddExport.c src/bdd/cudd/cuddGenCof.c \
	src/bdd/cudd/cuddGenetic.c src/bdd/cudd/cuddGroup.c \
	src/bdd/cudd/cudd.h src/bdd/cudd/cuddHarwell.c \
	src/bdd/cudd/cuddInit.c src/bdd/cudd/cuddInteract.c \
	src/bdd/cudd/cuddInt.h src/bdd/cudd/cuddLCache.c \
	src/bdd/cudd/cuddLevelQ.c src/bdd/cudd/cuddLinear.c \
	src/bdd/cudd/cuddLiteral.c src/bdd/cudd/cuddMatMult.c \
	src/bdd/cudd/cuddPriority.c src/bdd/cudd/cuddRead.c \
	src/bdd/cudd/cuddRef.c src/bdd/cudd/cuddReorder.c \
	src/bdd/cudd/cuddSat.c src/bdd/cudd/cuddSign.c \
	src/bdd/cudd/cuddSolve.c src/bdd/cudd/cuddSplit.c \
	src/bdd/cudd/cuddSubsetHB.c src/bdd/cudd/cuddSubsetSP.c \
	src/bdd/cudd/cuddSymmetry.c src/bdd/cudd/cuddTable.c \
	src/bdd/cudd/cuddUtil.c src/bdd/cudd/cuddWindow.c \
	src/bdd/cudd/cuddZddCount.c src/bdd/cudd/cuddZddFuncs.c \
	src/bdd/cudd/cuddZddGroup.c src/bdd/cudd/cuddZddIsop.c \
	src/bdd/cudd/cuddZddLin.c src/bdd/cudd/cuddZddMisc.c \
	src/bdd/cudd/cuddZddPort.c src/bdd/cudd/cuddZddReord.c \
	src/bdd/cudd/cuddZddSetop.c src/bdd/cudd/cuddZddSymm.c \
	src/bdd/cudd/cuddZddUtil.c src/bdd/mtr/mtrBasic.c \
	src/bdd/mtr/mtrGroup.c src/bdd/mtr/mtr.h src/bdd/mtr/mtrInt.h \
	src/bdd/st/st.c src/bdd/st/st.h src/bdd/util/cpu_stats.c \
	src/bdd/util/cpu_time.c src/bdd/util/datalimit.c \
	src/bdd/util/getopt.c src/bdd/util/pathsearch.c \
	src/bdd/util/pipefork.c src/bdd/util/prtime.c src/bdd/util/ptime.c \
	src/bdd/util/restart.c src/bdd/util/safe_mem.c \
	src/bdd/util/saveimage.c src/bdd/util/strsav.c src/bdd/util/stub.c \
	src/bdd/util/texpand.c src/bdd/util/tmpfile.c src/bdd/util/util.h \
	src/bdd/util/util-std.h

## A target to build the tag file for this subdirectory's documentation.
## The tag file contains information on every documented symbol so that
## documentation in other directories can be linked to documentation in
## this one.
$(srcdir)/src/bdd/libbdd.tag: $(srcdir)/src/bdd/doxygen_config.tag \
		$(src_bdd_libbdd_a_SOURCES)
	cd $(srcdir)/src/bdd && doxygen doxygen_config.tag

## A target to build all documentation under this subdirectory.  This
## builds the human readable documentation and links it to documentation
## in other directories.
src/bdd/doc $(srcdir)/src/bdd/html $(srcdir)/src/bdd/latex: \
		$(srcdir)/src/bdd/doxygen_config.doc \
		$(srcdir)/src/bdd/libbdd.tag
	cd $(srcdir)/src/bdd && doxygen doxygen_config.doc

## List all include files that should be symlinked into include under 
## noinst_..._INC.  These include files are not installed when the user
## does 'make install'.  This is appropriate for most include files.  If
## you have headers that need to be installed, see src/library/Makefile.am
## for an example.
noinst_src_bdd_cudd_INC = src/bdd/cudd/cudd.h src/bdd/cudd/cuddInt.h

## The class for the include files.  This is the directory under the include
## directory where the symlinks will be created.  This needs to have the same
## variable name as the _INC variable, only with a _CLASS suffix.
noinst_src_bdd_cudd_CLASS = bdd

noinst_src_bdd_mtr_INC = src/bdd/mtr/mtr.h src/bdd/mtr/mtrInt.h
noinst_src_bdd_mtr_CLASS = bdd

noinst_src_bdd_st_INC = src/bdd/st/st.h
noinst_src_bdd_st_CLASS = bdd

noinst_src_bdd_util_INC = src/bdd/util/util.h
noinst_src_bdd_util_CLASS = bdd

noinst_src_bdd_INC = noinst_src_bdd_cudd_INC noinst_src_bdd_mtr_INC \
	noinst_src_bdd_st_INC noinst_src_bdd_util_INC

## List any dependencies other than libraries under _DEPENDENDIES.  Defining
## this variable disables automatic dependency detection for this target, so
## you will need to relist dependencies that appear in _LDADD.
## src_bdd_libbdd_a_DEPENDENCIES = 

## List any special compiler flags under _CFLAGS.
## src_bdd_libbdd_a_CFLAGS = 
