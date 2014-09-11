## src/Pcode/PtoL/Makefile.am
## This file is (ultimately) included by the top level Makefile.am, so all
## files need to be specified relative to the top level directory.

## List the files generated in this directory.  Binaries go under _PRG,
## libraries under _LIB.  Higher level Makefiles use these variables
## unconditionally, so both _PRG and _LIB need to be defined.
src_Pcode_PtoL_PRG = src/Pcode/PtoL/PtoL
src_Pcode_PtoL_LIB = 
src_Pcode_PtoL_DST = src/Pcode/PtoL/doxygen_config.doc \
	src/Pcode/PtoL/doxygen_config.tag

## A target to support building everything under this subdirectory.  This
## should refer to anything defined above.  There should be two targets; one
## with and one without the trailing slash.  The trailing slash will allow
## the use of tab completion.
src/Pcode/PtoL src/Pcode/PtoL/: \
	include $(src_Pcode_PtoL_PRG) $(src_Pcode_PtoL_LIB)

## A target to support cleaning everything under this subdirectory.
src/Pcode/PtoL/clean:
	rm -f $(top_builddir)/src/Pcode/PtoL/*.$(OBJEXT)
	rm -f $(src_Pcode_PtoL_PRG)

## List all source and header files under _SOURCES.  
src_Pcode_PtoL_PtoL_SOURCES = \
	src/Pcode/PtoL/do_pcode.c src/Pcode/PtoL/pl_code.c \
	src/Pcode/PtoL/pl_data.c src/Pcode/PtoL/pl_func.c \
	src/Pcode/PtoL/pl_gen.c src/Pcode/PtoL/pl_intrinsic.c \
	src/Pcode/PtoL/pl_lcode.c src/Pcode/PtoL/pl_lcode.h \
	src/Pcode/PtoL/pl_main.h src/Pcode/PtoL/pl_mspec.c \
	src/Pcode/PtoL/pl_pragma.c src/Pcode/PtoL/pl_pr.c \
	src/Pcode/PtoL/pl_sync.c src/Pcode/PtoL/polymorph.c

## A target to build the tag file for this subdirectory's documentation.
## The tag file contains information on every documented symbol so that
## documentation in other directories can be linked to documentation in
## this one.
$(srcdir)/src/Pcode/PtoL/PtoL.tag: \
		$(srcdir)/src/Pcode/PtoL/doxygen_config.tag \
		$(src_Pcode_PtoL_PtoL_SOURCES)
	cd $(srcdir)/src/Pcode/PtoL && doxygen doxygen_config.tag

## A target to build all documentation under this subdirectory.  This
## build the human readable documentation and links it to documentation
## in other directories.
src/Pcode/PtoL/doc $(srcdir)/src/Pcode/PtoL/html \
	$(srcdir)/src/Pcode/PtoL/latex: \
		$(srcdir)/src/Pcode/PtoL/doxygen_config.doc \
		$(srcdir)/src/Pcode/Pcode/libpcode.tag \
		$(srcdir)/src/Pcode/Plib_CF/libpcode_cf.tag \
		$(srcdir)/src/Pcode/Plib_DD/libpcode_dd.tag \
		$(srcdir)/src/Pcode/Plib_SS/libpcode_ssa.tag \
		$(srcdir)/src/Pcode/Plib_probe/libpcode_probe.tag \
		$(srcdir)/src/machine/Mspec/libmspec.tag \
		$(srcdir)/src/Lcode/Lcode/liblcode.tag \
		$(srcdir)/src/library/libimpact.tag \
		$(srcdir)/src/bdd/libbdd.tag \
		$(srcdir)/src/Pcode/Pflatten/Pflatten.tag
	cd $(srcdir)/src/Pcode/PtoL && doxygen doxygen_config.doc

## List all include files that should be symlinked into include under 
## noinst_..._INC.  These include files are not installed when the user
## does 'make install'.  This is appropriate for most include files.  If
## you have headers that need to be installed, see src/library/Makefile.am
## for an example.
## noinst_src_Pcode_PtoL_INC = 

## The class for the include files.  This is the directory under the include
## directory where the symlinks will be created.  This needs to have the same
## variable name as the _INC variable, only with a _CLASS suffix.
## noinst_src_Pcode_PtoL_CLASS = 

## List any libraries needed when linking under _LDADD.  If a library is built
## as part of impact, list it as $(top_builddir)/lib/<library> so that it can
## be detected as a dependency automatically.  Automake assumes libraries
## listed as -l<lib> are system libraries and does not detect them as
## dependencies.
src_Pcode_PtoL_PtoL_LDADD = -L$(top_builddir)/lib \
	$(top_builddir)/lib/libpcode.a $(top_builddir)/lib/libpcode_cf.a \
	$(top_builddir)/lib/libpcode_dd.a $(top_builddir)/lib/libpcode_ssa.a \
	$(top_builddir)/lib/libpcode_probe.a \
	$(top_builddir)/lib/libmspec.a $(top_builddir)/lib/liblcode.a \
	$(top_builddir)/lib/libimpact.a $(top_builddir)/lib/libbdd.a \
	$(top_builddir)/lib/libpcode_flatten.a $(top_builddir)/lib/libproto.a -lm

## List any dependencies other than libraries under _DEPENDENDIES.  Defining
## this variable disables automatic dependency detection for this target, so
## you will need to relist dependencies that appear in _LDADD.
## src_Pcode_PtoL_PtoL_DEPENDENCIES = 

## List any special compiler flags under _CFLAGS.
## src_Pcode_PtoL_PtoL_CFLAGS = 


