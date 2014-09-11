## src/Lcode/tools/Lstatic/Makefile.am
## This file is (ultimately) included by the top level Makefile.am, so all
## files need to be specified relative to the top level directory.

## List the files generated in this directory.  Binaries go under _PRG,
## libraries under _LIB.  Higher level Makefiles use these variables
## unconditionally, so both _PRG and _LIB need to be defined.
src_Lcode_tools_Lstatic_PRG = src/Lcode/tools/Lstatic/Lstatic
src_Lcode_tools_Lstatic_LIB = 
src_Lcode_tools_Lstatic_DST = src/Lcode/tools/Lstatic/doxygen_config.doc \
	src/Lcode/tools/Lstatic/doxygen_config.tag

## A target to support building everything under this subdirectory.  This
## should refer to anything defined above.  There should be two targets; one
## with and one without the trailing slash.  The trailing slash will allow
## the use of tab completion.
src/Lcode/tools/Lstatic src/Lcode/tools/Lstatic/: \
	include $(src_Lcode_tools_Lstatic_PRG)

## A target to support cleaning everything under this subdirectory.
src/Lcode/tools/Lstatic/clean:
	rm -f $(top_builddir)/src/Lcode/tools/Lstatic/*.$(OBJEXT)
	rm -rf $(srcdir)/src/Lcode/tools/Lstatic/html
	rm -rf $(srcdir)/src/Lcode/tools/Lstatic/latex
	rm -f $(srcdir)/src/Lcode/tools/Lstatic/Lstatic.tag
	rm -f $(src_Lcode_tools_Lstatic_PRG)

## List all source and header files under _SOURCES.  
src_Lcode_tools_Lstatic_Lstatic_SOURCES = \
	src/Lcode/tools/Lstatic/l_static.c

## A target to build the tag file for this subdirectory's documentation.
## The tag file contains information on every documented symbol so that
## documentation in other directories can be linked to documentation in
## this one.
$(srcdir)/src/Lcode/tools/Lstatic/Lstatic.tag: \
		$(srcdir)/src/Lcode/tools/Lstatic/doxygen_config.tag \
		$(src_Lcode_tools_Lstatic_Lstatic_SOURCES)
	cd $(srcdir)/src/Lcode/tools/Lstatic && doxygen doxygen_config.tag

## A target to build all documentation under this subdirectory.  This
## builds the human readable documentation and links it to documentation
## in other directories.
src/Lcode/tools/Lstatic/doc $(srcdir)/src/Lcode/tools/Lstatic/html \
	$(srcdir)/src/Lcode/tools/Lstatic/latex: \
		$(srcdir)/src/Lcode/tools/Lstatic/doxygen_config.doc \
		$(srcdir)/src/Lcode/tools/Lstatic/Lstatic.tag \
		$(srcdir)/src/Lcode/codegen/Lschedule/libsched.tag \
		$(srcdir)/src/Lcode/codegen/Regalloc/libregalloc.tag \
		$(srcdir)/src/Lcode/opti/Lopti/Lopti.tag \
		$(srcdir)/src/machine/Lmdes/liblmdes.tag \
		$(srcdir)/src/Lcode/Lcode/liblcode.tag \
		$(srcdir)/src/bdd/libbdd.tag \
		$(srcdir)/src/machine/Mspec/libmspec.tag \
		$(srcdir)/src/library/libimpact.tag
	cd $(srcdir)/src/Lcode/tools/Lstatic && doxygen doxygen_config.doc

## List all include files that should be symlinked into include under 
## noinst_..._INC.  These include files are not installed when the user
## does 'make install'.  This is appropriate for most include files.  If
## you have headers that need to be installed, see src/library/Makefile.am
## for an example.
## noinst_src_Lcode_tools_Lstatic_INC = 

## The class for the include files.  This is the directory under the include
## directory where the symlinks will be created.  This needs to have the same
## variable name as the _INC variable, only with a _CLASS suffix.
## noinst_src_Lcode_tools_Lstatic_CLASS = 

## List any libraries needed when linking under _LDADD.  If a library is built
## as part of impact, list it as $(top_builddir)/lib/<library> so that it can
## be detected as a dependency automatically.  Automake assumes libraries
## listed as -l<lib> are system libraries and does not detect them as
## dependencies.
src_Lcode_tools_Lstatic_Lstatic_LDADD = -L$(top_builddir)/lib \
					$(top_builddir)/lib/libsched.a \
					$(top_builddir)/lib/libregalloc.a \
					$(top_builddir)/lib/liblopti.a \
					$(top_builddir)/lib/liblmdes.a \
					$(top_builddir)/lib/liblcode.a \
					$(top_builddir)/lib/libproto.a \
					$(top_builddir)/lib/libbdd.a \
					$(top_builddir)/lib/libmspec.a \
					$(top_builddir)/lib/libimpact.a -lm

## List any dependencies other than libraries under _DEPENDENDIES.  Defining
## this variable disables automatic dependency detection for this target, so
## you will need to relist dependencies that appear in _LDADD.
## src_Lcode_tools_Lstatic_Lstatic_DEPENDENCIES = 

## List any special compiler flags under _CFLAGS.
## src_Lcode_tools_Lstatic_Lstatic_CFLAGS = 
