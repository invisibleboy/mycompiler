## src/Lcode/codegen/Lhpl_pd/Makefile.am
## This file is (ultimately) included by the top level Makefile.am, so all
## files need to be specified relative to the top level directory.

## List the files generated in this directory.  Binaries go under _PRG,
## libraries under _LIB.  Higher level Makefiles use these variables
## unconditionally, so both _PRG and _LIB need to be defined.
src_Lcode_codegen_Lhpl_pd_PRG = src/Lcode/codegen/Lhpl_pd/Lhpl_pd
src_Lcode_codegen_Lhpl_pd_LIB = 

## A target to support building everything under this subdirectory.  This
## should refer to anything defined above.  There should be two targets; one
## with and one without the trailing slash.  The trailing slash will allow
## the use of tab completion.
src/Lcode/codegen/Lhpl_pd src/Lcode/codegen/Lhpl_pd/: \
	include $(src_Lcode_codegen_Lhpl_pd_PRG)

## A target to support cleaning everything under this subdirectory.
src/Lcode/codegen/Lhpl_pd/clean:
	rm -f $(top_builddir)/src/Lcode/codegen/Lhpl_pd/*.$(OBJEXT)
	rm -f $(src_Lcode_codegen_Lhpl_pd_PRG)

## List all source and header files under _SOURCES.  
src_Lcode_codegen_Lhpl_pd_Lhpl_pd_SOURCES = \
	src/Lcode/codegen/Lhpl_pd/lhpl_pd_main.c \
	src/Lcode/codegen/Lhpl_pd/lhpl_pd_main.h \
	src/Lcode/codegen/Lhpl_pd/lhpl_pd_phase1_func.c \
	src/Lcode/codegen/Lhpl_pd/lhpl_pd_phase1_varargs.c \
	src/Lcode/codegen/Lhpl_pd/lhpl_pd_phase1.h \
	src/Lcode/codegen/Lhpl_pd/lhpl_pd_phase2_func.c \
	src/Lcode/codegen/Lhpl_pd/lhpl_pd_phase2.h \
	src/Lcode/codegen/Lhpl_pd/lhpl_pd_phase2_reg.c \
	src/Lcode/codegen/Lhpl_pd/lhpl_pd_phase3_data.c \
	src/Lcode/codegen/Lhpl_pd/lhpl_pd_phase3_func.c \
	src/Lcode/codegen/Lhpl_pd/lhpl_pd_phase3.h

## List all include files that should be symlinked into include under
## noinst_..._INC.  These include files are not installed when the user
## does 'make install'.  This is appropriate for most include files.  If
## you have headers that need to be installed, see src/library/Makefile.am
## for an example.
noinst_src_Lcode_codegen_Lhpl_pd_INC = \
        src/Lcode/codegen/Lhpl_pd/lhpl_pd_main.h \
        src/Lcode/codegen/Lhpl_pd/lhpl_pd_phase1.h \
        src/Lcode/codegen/Lhpl_pd/lhpl_pd_phase2.h \
        src/Lcode/codegen/Lhpl_pd/lhpl_pd_phase3.h

## The class for the include files.  This is the directory under the include
## directory where the symlinks will be created.  This needs to have the same
## variable name as the _INC variable, only with a _CLASS suffix.
noinst_src_Lcode_codegen_Lhpl_pd_CLASS = Lcode

## List any libraries needed when linking under _LDADD.  If a library is built
## as part of impact, list it as $(top_builddir)/lib/<library> so that it can
## be detected as a dependency automatically.  Automake assumes libraries
## listed as -l<lib> are system libraries and does not detect them as
## dependencies.
src_Lcode_codegen_Lhpl_pd_Lhpl_pd_LDADD = \
	-L$(top_builddir)/lib $(top_builddir)/lib/libmopti.a \
	$(top_builddir)/lib/libblock.a $(top_builddir)/lib/libsoftpipe.a \
	$(top_builddir)/lib/liblopti.a $(top_builddir)/lib/libmarkpipe.a \
	$(top_builddir)/lib/libsched.a $(top_builddir)/lib/liblmdes.a \
	$(top_builddir)/lib/libregalloc.a \
	$(top_builddir)/lib/liblcode.a $(top_builddir)/lib/libbdd.a \
	$(top_builddir)/lib/libmspec.a $(top_builddir)/lib/libimpact.a \
	$(top_builddir)/lib/libsm.a $(top_builddir)/lib/libproto.a -lm

## List any dependencies other than libraries under _DEPENDENDIES.  Defining
## this variable disables automatic dependency detection for this target, so
## you will need to relist dependencies that appear in _LDADD.
## src_Lcode_codegen_Lhpl_pd_Lhpl_pd_DEPENDENCIES = 

## List any special compiler flags under _CFLAGS.
## src_Lcode_codegen_Lhpl_pd_Lhpl_pd_CFLAGS = 

