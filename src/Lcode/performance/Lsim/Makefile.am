## src/Lcode/performance/Lsim/Makefile.am
## This file is (ultimately) included by the top level Makefile.am, so all
## files need to be specified relative to the top level directory.

## List the files generated in this directory.  Binaries go under _PRG,
## libraries under _LIB.  Higher level Makefiles use these variables
## unconditionally, so both _PRG and _LIB need to be defined.
src_Lcode_performance_Lsim_PRG = src/Lcode/performance/Lsim/Lsim
src_Lcode_performance_Lsim_LIB = 
src_Lcode_performance_Lsim_DST = \
	src/Lcode/performance/Lsim/doxygen_config.doc \
	src/Lcode/performance/Lsim/doxygen_config.tag

## A target to support building everything under this subdirectory.  This
## should refer to anything defined above.  There should be two targets; one
## with and one without the trailing slash.  The trailing slash will allow
## the use of tab completion.
src/Lcode/performance/Lsim src/Lcode/performance/Lsim/: \
	include $(src_Lcode_performance_Lsim_PRG)

## A target to support cleaning everything under this subdirectory.
src/Lcode/performance/Lsim/clean:
	rm -f $(top_builddir)/src/Lcode/performance/Lsim/*.$(OBJEXT)
	rm -rf $(srcdir)/src/Lcode/performance/Lsim/html
	rm -rf $(srcdir)/src/Lcode/performance/Lsim/latex
	rm -f $(srcdir)/src/Lcode/performance/Lsim/Lsim.tag
	rm -f $(src_Lcode_performance_Lsim_PRG)

## List all source and header files under _SOURCES.  
src_Lcode_performance_Lsim_Lsim_SOURCES = \
	src/Lcode/performance/Lsim/s_addr_lib.c \
	src/Lcode/performance/Lsim/s_alat.c \
	src/Lcode/performance/Lsim/s_btb.c \
	src/Lcode/performance/Lsim/s_btb.h \
	src/Lcode/performance/Lsim/s_cache.c \
	src/Lcode/performance/Lsim/s_dcache.c \
	src/Lcode/performance/Lsim/s_dcache.h \
	src/Lcode/performance/Lsim/s_debug.c \
	src/Lcode/performance/Lsim/s_generic.c \
	src/Lcode/performance/Lsim/s_hash.c \
	src/Lcode/performance/Lsim/s_hash.h \
	src/Lcode/performance/Lsim/s_icache.c \
	src/Lcode/performance/Lsim/s_lib.c \
	src/Lcode/performance/Lsim/s_load_code.c \
	src/Lcode/performance/Lsim/s_main.c \
	src/Lcode/performance/Lsim/s_main.h \
	src/Lcode/performance/Lsim/s_mcb.c \
	src/Lcode/performance/Lsim/s_mem.c \
	src/Lcode/performance/Lsim/s_mem.h \
	src/Lcode/performance/Lsim/s_mem_profile.c \
	src/Lcode/performance/Lsim/s_mem_profile.h \
	src/Lcode/performance/Lsim/s_mqueue.c \
	src/Lcode/performance/Lsim/s_nyfo_vliw.c \
	src/Lcode/performance/Lsim/s_object.c \
	src/Lcode/performance/Lsim/s_object.h \
	src/Lcode/performance/Lsim/s_playdoh_vliw.c \
	src/Lcode/performance/Lsim/s_playdoh_vliw.h \
	src/Lcode/performance/Lsim/s_profile.c \
	src/Lcode/performance/Lsim/s_profile.h \
	src/Lcode/performance/Lsim/s_queue.c \
	src/Lcode/performance/Lsim/s_scache.c \
	src/Lcode/performance/Lsim/s_scache.h \
	src/Lcode/performance/Lsim/s_stats.c \
	src/Lcode/performance/Lsim/s_super.c \
	src/Lcode/performance/Lsim/s_super.h \
	src/Lcode/performance/Lsim/s_trace.c \
	src/Lcode/performance/Lsim/s_vliw.c \
	src/Lcode/performance/Lsim/s_vliw.h \
	src/Lcode/performance/Lsim/s_x86_trace.c

## A target to build the tag file for this subdirectory's documentation.
## The tag file contains information on every documented symbol so that
## documentation in other directories can be linked to documentation in
## this one.
$(srcdir)/src/Lcode/performance/Lsim/Lsim.tag: \
		$(srcdir)/src/Lcode/performance/Lsim/doxygen_config.tag \
		$(src_Lcode_performance_Lsim_Lsim_SOURCES)
	cd $(srcdir)/src/Lcode/performance/Lsim && doxygen doxygen_config.tag

## A target to build all documentation under this subdirectory.  This
## builds the human readable documentation and links it to documentation
## in other directories.
src/Lcode/performance/Lsim/doc $(srcdir)/src/Lcode/performance/Lsim/html \
	$(srcdir)/src/Lcode/performance/Lsim/latex: \
		$(srcdir)/src/Lcode/performance/Lsim/doxygen_config.doc \
		$(srcdir)/src/Lcode/performance/Lsim/Lsim.tag \
		$(srcdir)/src/library/libimpact.tag
	cd $(srcdir)/src/Lcode/performance/Lsim && doxygen doxygen_config.doc

## List all include files that should be symlinked into include under 
## noinst_..._INC.  These include files are not installed when the user
## does 'make install'.  This is appropriate for most include files.  If
## you have headers that need to be installed, see src/library/Makefile.am
## for an example.
noinst_src_Lcode_performance_Lsim_INC = \
	src/Lcode/performance/Lsim/s_btb.h \
	src/Lcode/performance/Lsim/s_dcache.h \
	src/Lcode/performance/Lsim/s_hash.h \
	src/Lcode/performance/Lsim/s_main.h \
	src/Lcode/performance/Lsim/s_mem.h \
	src/Lcode/performance/Lsim/s_playdoh_vliw.h \
	src/Lcode/performance/Lsim/s_profile.h \
	src/Lcode/performance/Lsim/s_scache.h \
	src/Lcode/performance/Lsim/s_super.h \
	src/Lcode/performance/Lsim/s_vliw.h

## The class for the include files.  This is the directory under the include
## directory where the symlinks will be created.  This needs to have the same
## variable name as the _INC variable, only with a _CLASS suffix.
noinst_src_Lcode_performance_Lsim_CLASS = Lcode

## List any libraries needed when linking under _LDADD.  If a library is built
## as part of impact, list it as $(top_builddir)/lib/<library> so that it can
## be detected as a dependency automatically.  Automake assumes libraries
## listed as -l<lib> are system libraries and does not detect them as
## dependencies.
src_Lcode_performance_Lsim_Lsim_LDADD = -L$(top_builddir)/lib \
					$(top_builddir)/lib/libimpact.a -lm

## List any dependencies other than libraries under _DEPENDENDIES.  Defining
## this variable disables automatic dependency detection for this target, so
## you will need to relist dependencies that appear in _LDADD.
## src_Lcode_performance_Lsim_Lsim_DEPENDENCIES = 

## List any special compiler flags under _CFLAGS.
## src_Lcode_performance_Lsim_Lsim_CFLAGS = 
