## src/Lcode/tools/Ldot/Makefile.am
## This file is (ultimately) included by the top level Makefile.am, so all
## files need to be specified relative to the top level directory.

## List the files generated in this directory.  Binaries go under _PRG,
## libraries under _LIB.  Higher level Makefiles use these variables
## unconditionally, so both _PRG and _LIB need to be defined.
src_Lcode_tools_Ldot_PRG = src/Lcode/tools/Ldot/Ldot
src_Lcode_tools_Ldot_LIB = src/Lcode/tools/Ldot/libdot.a
src_Lcode_tools_Ldot_DST = \
	src/Lcode/tools/Ldot/doxygen_config.doc \
	src/Lcode/tools/Ldot/doxygen_config.tag

## A target to support building everything under this subdirectory.  This
## should refer to anything defined above.  There should be two targets; one
## with and one without the trailing slash.  The trailing slash will allow
## the use of tab completion.
src/Lcode/tools/Ldot src/Lcode/tools/Ldot/: \
	include $(src_Lcode_tools_Ldot_PRG) \
	$(src_Lcode_tools_Ldot_LIB)

## A target to support cleaning everything under this subdirectory.
src/Lcode/tools/Ldot/clean:
	rm -f $(top_builddir)/src/Lcode/tools/Ldot/*.$(OBJEXT)
	rm -rf $(srcdir)/src/Lcode/tools/Ldot/html
	rm -rf $(srcdir)/src/Lcode/tools/Ldot/latex
	rm -f $(srcdir)/src/Lcode/tools/Ldot/Ldot.tag
	rm -f $(src_Lcode_tools_Ldot_PRG)
	rm -f $(src_Lcode_tools_Ldot_LIB)

src_Lcode_tools_Ldot_SRC = \
	src/Lcode/tools/Ldot/l_codegen.c \
	src/Lcode/tools/Ldot/l_dot.c

## A target to build the tag file for this subdirectory's documentation.
## The tag file contains information on every documented symbol so that
## documentation in other directories can be linked to documentation in
## this one.
$(srcdir)/src/Lcode/tools/Ldot/Ldot.tag: \
		$(srcdir)/src/Lcode/tools/Ldot/doxygen_config.tag \
		$(src_Lcode_tools_Ldot_SRC)
	cd $(srcdir)/src/Lcode/tools/Ldot && doxygen doxygen_config.tag

## A target to build all documentation under this subdirectory.  This
## builds the human readable documentation and links it to documentation
## in other directories.
src/Lcode/tools/Ldot/doc $(srcdir)/src/Lcode/tools/Ldot/html \
	$(srcdir)/src/Lcode/tools/Ldot/latex: \
		$(srcdir)/src/Lcode/tools/Ldot/doxygen_config.doc \
		$(srcdir)/src/Lcode/tools/Ldot/Ldot.tag \
		$(srcdir)/src/Lcode/sched/SM/libsm.tag \
		$(srcdir)/src/machine/Lmdes/liblmdes.tag \
		$(srcdir)/src/Lcode/codegen/Regalloc/libregalloc.tag \
		$(srcdir)/src/Lcode/tools/Lmarkpipe/libmarkpipe.tag \
		$(srcdir)/src/Lcode/Lcode/liblcode.tag \
		$(srcdir)/src/bdd/libbdd.tag \
		$(srcdir)/src/machine/Mspec/libmspec.tag \
		$(srcdir)/src/library/libimpact.tag
	cd $(srcdir)/src/Lcode/tools/Ldot && doxygen doxygen_config.doc

## List all include files that should be symlinked into include under 
## noinst_..._INC.  These include files are not installed when the user
## does 'make install'.  This is appropriate for most include files.  If
## you have headers that need to be installed, see src/library/Makefile.am
## for an example.
noinst_src_Lcode_tools_Ldot_INC = \
	src/Lcode/tools/Ldot/l_dot.h

## The class for the include files.  This is the directory under the include
## directory where the symlinks will be created.  This needs to have the same
## variable name as the _INC variable, only with a _CLASS suffix.
noinst_src_Lcode_tools_Ldot_CLASS = Lcode

## List all source and header files under _SOURCES.  
src_Lcode_tools_Ldot_Ldot_SOURCES = \
	$(src_Lcode_tools_Ldot_SRC)

## List any libraries needed when linking under _LDADD.  If a library is built
## as part of impact, list it as $(top_builddir)/lib/<library> so that it can
## be detected as a dependency automatically.  Automake assumes libraries
## listed as -l<lib> are system libraries and does not detect them as
## dependencies.
src_Lcode_tools_Ldot_Ldot_LDADD = \
	-L$(top_builddir)/lib \
	$(top_builddir)/lib/libproto.a \
	$(top_builddir)/lib/liblcode.a \
	$(top_builddir)/lib/libbdd.a \
	$(top_builddir)/lib/libmspec.a \
	$(top_builddir)/lib/libimpact.a -lm -lproto

## List any dependencies other than libraries under _DEPENDENDIES.  Defining
## this variable disables automatic dependency detection for this target, so
## you will need to relist dependencies that appear in _LDADD.
## src_Lcode_tools_Ldot_Ldot_DEPENDENCIES = 

## List any special compiler flags under _CFLAGS.
## src_Lcode_tools_Ldot_Ldot_CFLAGS = 

## List all source and header files under _SOURCES.  
src_Lcode_tools_Ldot_libdot_a_SOURCES = \
	$(src_Lcode_tools_Ldot_SRC)

## List any dependencies other than libraries under _DEPENDENDIES.  Defining
## this variable disables automatic dependency detection for this target, so
## you will need to relist dependencies that appear in _LDADD.
## src_Lcode_tools_Ldot_libdot_a_DEPENDENCIES = 

## List any special compiler flags under _CFLAGS.
## src_Lcode_tools_Ldot_libdot_a_CFLAGS = 
