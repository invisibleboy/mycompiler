## src/Lcode/codegen/Lsoftpipe/Makefile.am
## This file is (ultimately) included by the top level Makefile.am, so all
## files need to be specified relative to the top level directory.

## List the files generated in this directory.  Binaries go under _PRG,
## libraries under _LIB.  Higher level Makefiles use these variables
## unconditionally, so both _PRG and _LIB need to be defined.
src_Lcode_codegen_Lsoftpipe_PRG = 
src_Lcode_codegen_Lsoftpipe_LIB = src/Lcode/codegen/Lsoftpipe/libsoftpipe.a
src_Lcode_codegen_Lsoftpipe_DST = \
	src/Lcode/codegen/Lsoftpipe/doxygen_config.doc \
	src/Lcode/codegen/Lsoftpipe/doxygen_config.tag

## A target to support building everything under this subdirectory.  This
## should refer to anything defined above.  There should be two targets; one
## with and one without the trailing slash.  The trailing slash will allow
## the use of tab completion.
src/Lcode/codegen/Lsoftpipe src/Lcode/codegen/Lsoftpipe/: \
	include $(src_Lcode_codegen_Lsoftpipe_LIB)

## A target to support cleaning everything under this subdirectory.
src/Lcode/codegen/Lsoftpipe/clean:
	rm -f $(top_builddir)/src/Lcode/codegen/Lsoftpipe/*.$(OBJEXT)
	rm -rf $(srcdir)/src/Lcode/codegen/Lsoftpipe/html
	rm -rf $(srcdir)/src/Lcode/codegen/Lsoftpipe/latex
	rm -f $(srcdir)/src/Lcode/codegen/Lsoftpipe/libsoftpipe.tag
	rm -f $(src_Lcode_codegen_Lsoftpipe_LIB)

## List all source and header files under _SOURCES.  
src_Lcode_codegen_Lsoftpipe_libsoftpipe_a_SOURCES = \
	src/Lcode/codegen/Lsoftpipe/l_gen_pipe.c \
	src/Lcode/codegen/Lsoftpipe/l_gen_pipe.h \
	src/Lcode/codegen/Lsoftpipe/l_loop_prep.c \
	src/Lcode/codegen/Lsoftpipe/l_loop_prep.h \
	src/Lcode/codegen/Lsoftpipe/l_mve.c \
	src/Lcode/codegen/Lsoftpipe/l_mve.h \
	src/Lcode/codegen/Lsoftpipe/l_pipe_mspec.c \
	src/Lcode/codegen/Lsoftpipe/l_pipe_mspec.h \
	src/Lcode/codegen/Lsoftpipe/l_pipe_rename.c \
	src/Lcode/codegen/Lsoftpipe/l_pipe_rename.h \
	src/Lcode/codegen/Lsoftpipe/l_pipe_sched.c \
	src/Lcode/codegen/Lsoftpipe/l_pipe_sched.h \
	src/Lcode/codegen/Lsoftpipe/l_pipe_sync.c \
	src/Lcode/codegen/Lsoftpipe/l_pipe_sync.h \
	src/Lcode/codegen/Lsoftpipe/l_pipe_util.c \
	src/Lcode/codegen/Lsoftpipe/l_pipe_util.h \
	src/Lcode/codegen/Lsoftpipe/l_softpipe.c \
	src/Lcode/codegen/Lsoftpipe/l_softpipe.h \
	src/Lcode/codegen/Lsoftpipe/l_softpipe_info.c \
	src/Lcode/codegen/Lsoftpipe/l_softpipe_info.h \
	src/Lcode/codegen/Lsoftpipe/l_softpipe_int.h

## A target to build the tag file for this subdirectory's documentation.
## The tag file contains information on every documented symbol so that
## documentation in other directories can be linked to documentation in
## this one.
$(srcdir)/src/Lcode/codegen/Lsoftpipe/libsoftpipe.tag: \
		$(srcdir)/src/Lcode/codegen/Lsoftpipe/doxygen_config.tag \
		$(src_Lcode_codegen_Lsoftpipe_libsoftpipe_a_SOURCES)
	cd $(srcdir)/src/Lcode/codegen/Lsoftpipe && doxygen doxygen_config.tag

## A target to build all documentation under this subdirectory.  This
## builds the human readable documentation and links it to documentation
## in other directories.
src/Lcode/codegen/Lsoftpipe/doc $(srcdir)/src/Lcode/codegen/Lsoftpipe/html \
	$(srcdir)/src/Lcode/codegen/Lsoftpipe/latex: \
		$(srcdir)/src/Lcode/codegen/Lsoftpipe/doxygen_config.doc \
		$(srcdir)/src/Lcode/codegen/Lsoftpipe/libsoftpipe.tag \
		$(srcdir)/src/Lcode/Lcode/liblcode.tag \
		$(srcdir)/src/machine/Mspec/libmspec.tag \
		$(srcdir)/src/Lcode/sched/SM/libsm.tag \
		$(srcdir)/src/Lcode/codegen/Regalloc/libregalloc.tag \
		$(srcdir)/src/Lcode/opti/Lopti/Lopti.tag \
		$(srcdir)/src/machine/Lmdes/liblmdes.tag
	cd $(srcdir)/src/Lcode/codegen/Lsoftpipe && doxygen doxygen_config.doc

## List all include files that should be symlinked into include under 
## noinst_..._INC.  These include files are not installed when the user
## does 'make install'.  This is appropriate for most include files.  If
## you have headers that need to be installed, see src/library/Makefile.am
## for an example.
noinst_src_Lcode_codegen_Lsoftpipe_INC = \
	src/Lcode/codegen/Lsoftpipe/l_loop_prep.h \
	src/Lcode/codegen/Lsoftpipe/l_softpipe.h

## The class for the include files.  This is the directory under the include
## directory where the symlinks will be created.  This needs to have the same
## variable name as the _INC variable, only with a _CLASS suffix.
noinst_src_Lcode_codegen_Lsoftpipe_CLASS = Lcode

## List any dependencies other than libraries under _DEPENDENDIES.  Defining
## this variable disables automatic dependency detection for this target, so
## you will need to relist dependencies that appear in _LDADD.
## src_Lcode_codegen_Lsoftpipe_libsoftpipe_a_DEPENDENCIES = 

## List any special compiler flags under _CFLAGS.
src_Lcode_codegen_Lsoftpipe_libsoftpipe_a_CFLAGS = -DIMPACT_FIX $(AM_CFLAGS)


