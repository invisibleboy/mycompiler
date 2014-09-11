## src/Lcode/performance/Lprobe/Makefile.am
## This file is (ultimately) included by the top level Makefile.am, so all
## files need to be specified relative to the top level directory.

## List the files generated in this directory.  Binaries go under _PRG,
## libraries under _LIB.  Higher level Makefiles use these variables
## unconditionally, so both _PRG and _LIB need to be defined.
#src_Lcode_performance_Lprobe_PRG = src/Lcode/performance/Lprobe/Lprobe
#src_Lcode_performance_Lprobe_LIB = src/Lcode/performance/Lprobe/libprobe.a
src_Lcode_performance_Lprobe_DST = \
	src/Lcode/performance/Lprobe/l_trace_interface.h \
	src/Lcode/performance/Lprobe/l_probe.h \
	src/Lcode/performance/Lprobe/doxygen_config.doc \
	src/Lcode/performance/Lprobe/doxygen_config.tag

## A target to support building everything under this subdirectory.  This
## should refer to anything defined above.  There should be two targets; one
## with and one without the trailing slash.  The trailing slash will allow
## the use of tab completion.
src/Lcode/performance/Lprobe src/Lcode/performance/Lprobe/: \
	include $(src_Lcode_performance_Lprobe_PRG) \
	$(src_Lcode_performance_Lprobe_LIB)

## A target to support cleaning everything under this subdirectory.
src/Lcode/performance/Lprobe/clean:
	rm -f $(top_builddir)/src/Lcode/performance/Lprobe/*.$(OBJEXT)
	rm -rf $(srcdir)/src/Lcode/performance/Lprobe/html
	rm -rf $(srcdir)/src/Lcode/performance/Lprobe/latex
	rm -f $(srcdir)/src/Lcode/performance/Lprobe/Lprobe.tag
	rm -f $(src_Lcode_performance_Lprobe_PRG)
	rm -f $(src_Lcode_performance_Lprobe_LIB)

src_Lcode_performance_Lprobe_SRC = \
	src/Lcode/performance/Lprobe/l_codegen_main.c \
	src/Lcode/performance/Lprobe/l_probe.c \
	src/Lcode/performance/Lprobe/l_probe.h \
	src/Lcode/performance/Lprobe/l_probe_hppa.c \
	src/Lcode/performance/Lprobe/l_probe_playdoh.c \
	src/Lcode/performance/Lprobe/l_probe_sparc.c \
	src/Lcode/performance/Lprobe/l_probe_x86.c \
	src/Lcode/performance/Lprobe/l_trace_interface.h

## A target to build the tag file for this subdirectory's documentation.
## The tag file contains information on every documented symbol so that
## documentation in other directories can be linked to documentation in
## this one.
$(srcdir)/src/Lcode/performance/Lprobe/Lprobe.tag: \
		$(srcdir)/src/Lcode/performance/Lprobe/doxygen_config.tag \
		$(src_Lcode_performance_Lprobe_SRC)
	cd $(srcdir)/src/Lcode/performance/Lprobe && doxygen doxygen_config.tag

## A target to build all documentation under this subdirectory.  This
## builds the human readable documentation and links it to documentation
## in other directories.
src/Lcode/performance/Lprobe/doc $(srcdir)/src/Lcode/performance/Lprobe/html \
	$(srcdir)/src/Lcode/performance/Lprobe/latex: \
		$(srcdir)/src/Lcode/performance/Lprobe/doxygen_config.doc \
		$(srcdir)/src/Lcode/performance/Lprobe/Lprobe.tag \
		$(srcdir)/src/Lcode/Lcode/liblcode.tag \
		$(srcdir)/src/bdd/libbdd.tag \
		$(srcdir)/src/machine/Mspec/libmspec.tag \
		$(srcdir)/src/library/libimpact.tag \
		$(srcdir)/src/machine/Lmdes/liblmdes.tag
	cd $(srcdir)/src/Lcode/performance/Lprobe && doxygen doxygen_config.doc

## List all include files that should be symlinked into include under 
## noinst_..._INC.  These include files are not installed when the user
## does 'make install'.  This is appropriate for most include files.  If
## you have headers that need to be installed, see src/library/Makefile.am
## for an example.  If an include file needs to be installed when the user
## does 'make install', omit the noinst_ prefix.
noinst_src_Lcode_performance_Lprobe_INC = \
	src/Lcode/performance/Lprobe/l_probe.h

## The class for the include files.  This is the directory under the include
## directory where the symlinks will be created.  This needs to have the same
## variable name as the _INC variable, only with a _CLASS suffix.
noinst_src_Lcode_performance_Lprobe_CLASS = Lcode

src_Lcode_performance_Lprobe_INC = \
	src/Lcode/performance/Lprobe/l_trace_interface.h
src_Lcode_performance_Lprobe_CLASS = Lcode

## If an include file needs to be installed when the user does 'make install',
## you need to define one of automake's variables to tell it where to install.
## ...dir tells automake where to install the files listed under ..._HEADERS.
## The nodist_ prefix on _HEADERS tells automake not to include this symlink
## in the distribution.  The symlink is generated when the user builds, so it
## doesn't need to be distributed.
src_Lcode_performance_Lprobedir = $(prefix)/include/Lcode
nodist_src_Lcode_performance_Lprobe_HEADERS = \
	$(src_Lcode_performance_Lprobe_INC)

## List all source and header files under _SOURCES.  
#src_Lcode_performance_Lprobe_Lprobe_SOURCES = \
#	$(src_Lcode_performance_Lprobe_SRC)

## List any libraries needed when linking under _LDADD.  If a library is built
## as part of impact, list it as $(top_builddir)/lib/<library> so that it can
## be detected as a dependency automatically.  Automake assumes libraries
## listed as -l<lib> are system libraries and does not detect them as
## dependencies.
#src_Lcode_performance_Lprobe_Lprobe_LDADD = \
#	-L$(top_builddir)/lib $(top_builddir)/lib/liblcode.a \
#	$(top_builddir)/lib/libbdd.a $(top_builddir)/lib/libmspec.a \
#	$(top_builddir)/lib/libimpact.a $(top_builddir)/lib/liblmdes.a -lm

## List any dependencies other than libraries under _DEPENDENDIES.  Defining
## this variable disables automatic dependency detection for this target, so
## you will need to relist dependencies that appear in _LDADD.
## src_Lcode_performance_Lprobe_Lprobe_DEPENDENCIES = 

## List any special compiler flags under _CFLAGS.
## src_Lcode_performance_Lprobe_Lprobe_CFLAGS = 

## List all source and header files under _SOURCES.  
#src_Lcode_performance_Lprobe_libprobe_a_SOURCES = \
#	$(src_Lcode_performance_Lprobe_SRC)

## List any dependencies other than libraries under _DEPENDENDIES.  Defining
## this variable disables automatic dependency detection for this target, so
## you will need to relist dependencies that appear in _LDADD.
## src_Lcode_performance_Lprobe_libprobe_a_DEPENDENCIES = 

## List any special compiler flags under _CFLAGS.
## src_Lcode_performance_Lprobe_libprobe_a_CFLAGS = 
