## src/Pcode/Pannotate/Makefile.am
## This file is (ultimately) included by the top level Makefile.am, so all
## files need to be specified relative to the top level directory.

## List the files generated in this directory.  Binaries go under _PRG,
## libraries under _LIB.  Higher level Makefiles use these variables
## unconditionally, so both _PRG and _LIB need to be defined.
src_Pcode_Pannotate_PRG = src/Pcode/Pannotate/Pannotate
src_Pcode_Pannotate_LIB = 
src_Pcode_Pannotate_DST = src/Pcode/Pannotate/doxygen_config.doc \
	src/Pcode/Pannotate/doxygen_config.tag

## A target to support building everything under this subdirectory.  This
## should refer to anything defined above.  There should be two targets; one
## with and one without the trailing slash.  The trailing slash will allow
## the use of tab completion.
src/Pcode/Pannotate src/Pcode/Pannotate/: include $(src_Pcode_Pannotate_PRG)

## A target to support cleaning everything under this subdirectory.
src/Pcode/Pannotate/clean:
	rm -f $(top_builddir)/src/Pcode/Pannotate/*.$(OBJEXT)
	rm -rf $(srcdir)/src/Pcode/Pannotate/html
	rm -rf $(srcdir)/src/Pcode/Pannotate/latex
	rm -f $(srcdir)/src/Pcode/Pannotate/Pannotate.tag
	rm -f $(src_Pcode_Pannotate_PRG)

## List all source and header files under _SOURCES.  
src_Pcode_Pannotate_Pannotate_SOURCES = \
	src/Pcode/Pannotate/annotate.c src/Pcode/Pannotate/profile.c \
	src/Pcode/Pannotate/annotate.h

## A target to build the tag file for this subdirectory's documentation.
## The tag file contains information on every documented symbol so that
## documentation in other directories can be linked to documentation in
## this one.
$(srcdir)/src/Pcode/Pannotate/Pannotate.tag: \
		$(srcdir)/src/Pcode/Pannotate/doxygen_config.tag \
		$(src_Pcode_Pannotate_Pannotate_SOURCES)
	cd $(srcdir)/src/Pcode/Pannotate && doxygen doxygen_config.tag

## A target to build all documentation under this subdirectory.  This
## builds the human readable documentation and links it to documentation
## in other directories.
src/Pcode/Pannotate/doc $(srcdir)/src/Pcode/Pannotate/html \
	$(srcdir)/src/Pcode/Pannotate/latex: \
		$(srcdir)/src/Pcode/Pannotate/doxygen_config.doc \
		$(srcdir)/src/Pcode/Pannotate/Pannotate.tag \
		$(srcdir)/src/Pcode/Pcode/libpcode.tag \
		$(srcdir)/src/Pcode/Plib_probe/libpcode_probe.tag \
		$(srcdir)/src/Pcode/Plib_CF/libpcode_cf.tag \
		$(srcdir)/src/machine/Mspec/libmspec.tag \
		$(srcdir)/src/library/libimpact.tag
	cd $(srcdir)/src/Pcode/Pannotate && doxygen doxygen_config.doc

## List all include files that should be symlinked into include under 
## noinst_..._INC.  These include files are not installed when the user
## does 'make install'.  This is appropriate for most include files.  If
## you have headers that need to be installed, see src/library/Makefile.am
## for an example.
## noinst_src_Pcode_Pannotate_INC = 

## The class for the include files.  This is the directory under the include
## directory where the symlinks will be created.  This needs to have the same
## variable name as the _INC variable, only with a _CLASS suffix.
## noinst_src_Pcode_Pannotate_CLASS = 

## List any libraries needed when linking under _LDADD.  If a library is built
## as part of impact, list it as $(top_builddir)/lib/<library> so that it can
## be detected as a dependency automatically.  Automake assumes libraries
## listed as -l<lib> are system libraries and does not detect them as
## dependencies.
src_Pcode_Pannotate_Pannotate_LDADD = \
	-L$(top_builddir)/lib $(top_builddir)/lib/libpcode.a \
	$(top_builddir)/lib/libpcode_probe.a \
	$(top_builddir)/lib/libpcode_cf.a $(top_builddir)/lib/libmspec.a \
	$(top_builddir)/lib/libimpact.a -lm

## List any dependencies other than libraries under _DEPENDENDIES.  Defining
## this variable disables automatic dependency detection for this target, so
## you will need to relist dependencies that appear in _LDADD.
## src_Pcode_Pannotate_Pannotate_DEPENDENCIES = 

## List any special compiler flags under _CFLAGS.
## src_Pcode_Pannotate_Pannotate_CFLAGS = 
