## src/machine/Lmdes_build/Makefile.am
## This file is (ultimately) included by the top level Makefile.am, so all
## files need to be specified relative to the top level directory.

## List the files generated in this directory.  Binaries go under _PRG,
## libraries under _LIB.  Higher level Makefiles use these variables
## unconditionally, so both _PRG and _LIB need to be defined.
src_machine_Lmdes_build_PRG = src/machine/Lmdes_build/Lmdes_build
src_machine_Lmdes_build_LIB = 
src_machine_Lmdes_build_DST = src/machine/Lmdes_build/doxygen_config.doc \
	src/machine/Lmdes_build/doxygen_config.tag

## A target to support building everything under this subdirectory.  This
## should refer to anything defined above.  There should be two targets; one
## with and one without the trailing slash.  The trailing slash will allow
## the use of tab completion.
src/machine/Lmdes_build src/machine/Lmdes_build/: \
	include $(src_machine_Lmdes_build_PRG)

## A target to support cleaning everything under this subdirectory.
src/machine/Lmdes_build/clean:
	rm -f $(top_builddir)/src/machine/Lmdes_build/*.$(OBJEXT)
	rm -rf $(srcdir)/src/machine/Lmdes_build/html
	rm -rf $(srcdir)/src/machine/Lmdes_build/latex
	rm -f $(srcdir)/src/machine/Lmdes_build/Lmdes_build.tag
	rm -f $(src_machine_Lmdes_build_PRG)

## List all source and header files under _SOURCES.  
src_machine_Lmdes_build_Lmdes_build_SOURCES = \
	src/machine/Lmdes_build/lmdes_build.c

## A target to build the tag file for this subdirectory's documentation.
## The tag file contains information on every documented symbol so that
## documentation in other directories can be linked to documentation in
## this one.
$(srcdir)/src/machine/Lmdes_build/Lmdes_build.tag: \
		$(srcdir)/src/machine/Lmdes_build/doxygen_config.tag \
		$(src_machine_Lmdes_build_Lmdes_build_SOURCES)
	cd $(srcdir)/src/machine/Lmdes_build && doxygen doxygen_config.tag

## A target to build all documentation under this subdirectory.  This
## builds the human readable documentation and links it to documentation
## in other directories.
src/machine/Lmdes_build/doc $(srcdir)/src/machine/Lmdes_build/html \
	$(srcdir)/src/machine/Lmdes_build/latex: \
		$(srcdir)/src/machine/Lmdes_build/doxygen_config.doc \
		$(srcdir)/src/machine/Lmdes_build/Lmdes_build.tag \
		$(srcdir)/src/machine/hmdes/libhmdes.tag \
		$(srcdir)/src/library/libimpact.tag
	cd $(srcdir)/src/machine/Lmdes_build && doxygen doxygen_config.doc

## List all include files that should be symlinked into include under 
## noinst_..._INC.  These include files are not installed when the user
## does 'make install'.  This is appropriate for most include files.  If
## you have headers that need to be installed, see src/library/Makefile.am
## for an example.
## noinst_src_machine_Lmdes_build_INC = 

## The class for the include files.  This is the directory under the include
## directory where the symlinks will be created.  This needs to have the same
## variable name as the _INC variable, only with a _CLASS suffix.
## noinst_src_machine_Lmdes_build_CLASS = 

## List any libraries needed when linking under _LDADD.  If a library is built
## as part of impact, list it as $(top_builddir)/lib/<library> so that it can
## be detected as a dependency automatically.  Automake assumes libraries
## listed as -l<lib> are system libraries and does not detect them as
## dependencies.
src_machine_Lmdes_build_Lmdes_build_LDADD = \
	-L$(top_builddir)/lib $(top_builddir)/lib/libhmdes.a \
	$(top_builddir)/lib/libimpact.a

## List any dependencies other than libraries under _DEPENDENDIES.  Defining
## this variable disables automatic dependency detection for this target, so
## you will need to relist dependencies that appear in _LDADD.
## src_machine_Lmdes_build_Lmdes_build_DEPENDENCIES = 

## List any special compiler flags under _CFLAGS.
## src_machine_Lmdes_build_Lmdes_build_CFLAGS = 
