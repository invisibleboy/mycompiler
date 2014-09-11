## src/Lcode/sched/SM/Makefile.am
## This file is (ultimately) included by the top level Makefile.am, so all
## files need to be specified relative to the top level directory.

## List the files generated in this directory.  Binaries go under _PRG,
## libraries under _LIB.  Higher level Makefiles use these variables
## unconditionally, so both _PRG and _LIB need to be defined.
src_Lcode_sched_SM_PRG = src/Lcode/sched/SM/SM
src_Lcode_sched_SM_LIB = src/Lcode/sched/SM/libsm.a
src_Lcode_sched_SM_DST = src/Lcode/sched/SM/doxygen_config.doc \
	src/Lcode/sched/SM/doxygen_config.tag

## A target to support building everything under this subdirectory.  This
## should refer to anything defined above.  There should be two targets; one
## with and one without the trailing slash.  The trailing slash will allow
## the use of tab completion.
src/Lcode/sched/SM src/Lcode/sched/SM/: include $(src_Lcode_sched_SM_PRG)

## A target to support cleaning everything under this subdirectory.
src/Lcode/sched/SM/clean:
	rm -f $(top_builddir)/src/Lcode/sched/SM/*.$(OBJEXT)
	rm -rf $(srcdir)/src/Lcode/sched/SM/html
	rm -rf $(srcdir)/src/Lcode/sched/SM/latex
	rm -f $(srcdir)/src/Lcode/sched/SM/libsm.tag
	rm -f $(src_Lcode_sched_SM_PRG)
	rm -f $(src_Lcode_sched_SM_LIB)

src_Lcode_sched_SM_SRC = \
	src/Lcode/sched/SM/sm.c src/Lcode/sched/SM/sm_compact.c \
	src/Lcode/sched/SM/sm_cudd.c src/Lcode/sched/SM/sm_cudd.h \
	src/Lcode/sched/SM/sm_dep.c src/Lcode/sched/SM/sm.h \
	src/Lcode/sched/SM/sm_linear.c \
	src/Lcode/sched/SM/sm_machine.c src/Lcode/sched/SM/sm_main.c \
	src/Lcode/sched/SM/sm_modulo_rmap.c src/Lcode/sched/SM/sm_opti.c \
	src/Lcode/sched/SM/sm_priority.c src/Lcode/sched/SM/sm_queue.c \
	src/Lcode/sched/SM/sm_recovery.c src/Lcode/sched/SM/sm_recovery.h \
	src/Lcode/sched/SM/sm_rinfo.c src/Lcode/sched/SM/sm_rmap.c \
	src/Lcode/sched/SM/sm_sched.c

## A target to build the tag file for this subdirectory's documentation.
## The tag file contains information on every documented symbol so that
## documentation in other directories can be linked to documentation in
## this one.
$(srcdir)/src/Lcode/sched/SM/libsm.tag: \
		$(srcdir)/src/Lcode/sched/SM/doxygen_config.tag \
		$(src_Lcode_sched_SM_SRC)
	cd $(srcdir)/src/Lcode/sched/SM && doxygen doxygen_config.tag

## A target to build all documentation under this subdirectory.  This
## builds the human readable documentation and links it to documentation
## in other directories.
src/Lcode/sched/SM/doc $(srcdir)/src/Lcode/sched/SM/html \
	$(srcdir)/src/Lcode/sched/SM/latex: \
		$(srcdir)/src/Lcode/sched/SM/doxygen_config.doc \
		$(srcdir)/src/Lcode/sched/SM/libsm.tag \
		$(srcdir)/src/Lcode/Lcode/liblcode.tag \
		$(srcdir)/src/machine/Mspec/libmspec.tag \
		$(srcdir)/src/machine/Lmdes/liblmdes.tag \
		$(srcdir)/src/library/libimpact.tag \
		$(srcdir)/src/bdd/libbdd.tag
	cd $(srcdir)/src/Lcode/sched/SM && doxygen doxygen_config.doc

## List all include files that should be symlinked into include under 
## noinst_..._INC.  These include files are not installed when the user
## does 'make install'.  This is appropriate for most include files.  If
## you have headers that need to be installed, see src/library/Makefile.am
## for an example.
noinst_src_Lcode_sched_SM_INC = \
	src/Lcode/sched/SM/sm.h src/Lcode/sched/SM/sm_cudd.h \
	src/Lcode/sched/SM/sm_recovery.h

## The class for the include files.  This is the directory under the include
## directory where the symlinks will be created.  This needs to have the same
## variable name as the _INC variable, only with a _CLASS suffix.
noinst_src_Lcode_sched_SM_CLASS = Lcode

## List all source and header files under _SOURCES.  
src_Lcode_sched_SM_SM_SOURCES = $(src_Lcode_sched_SM_SRC)

## List any libraries needed when linking under _LDADD.  If a library is built
## as part of impact, list it as $(top_builddir)/lib/<library> so that it can
## be detected as a dependency automatically.  Automake assumes libraries
## listed as -l<lib> are system libraries and does not detect them as
## dependencies.
src_Lcode_sched_SM_SM_LDADD = -L$(top_builddir)/lib \
			      $(top_builddir)/lib/liblcode.a \
			      $(top_builddir)/lib/libproto.a \
			      $(top_builddir)/lib/libmspec.a \
			      $(top_builddir)/lib/liblmdes.a \
			      $(top_builddir)/lib/libimpact.a \
			      $(top_builddir)/lib/libbdd.a -lm

## List any dependencies other than libraries under _DEPENDENDIES.  Defining
## this variable disables automatic dependency detection for this target, so
## you will need to relist dependencies that appear in _LDADD.
## src_Lcode_sched_SM_SM_DEPENDENCIES = 

## List any special compiler flags under _CFLAGS.
## src_Lcode_sched_SM_SM_CFLAGS = 

## List all source and header files under _SOURCES.  
src_Lcode_sched_SM_libsm_a_SOURCES = $(src_Lcode_sched_SM_SRC)

## List any dependencies other than libraries under _DEPENDENDIES.  Defining
## this variable disables automatic dependency detection for this target, so
## you will need to relist dependencies that appear in _LDADD.
## src_Lcode_sched_SM_libsm_a_DEPENDENCIES = 

## List any special compiler flags under _CFLAGS.
## src_Lcode_sched_SM_libsm_a_CFLAGS = 


