## src/Pcode/Plib_CF/Makefile.am
## This file is (ultimately) included by the top level Makefile.am, so all
## files need to be specified relative to the top level directory.

## List the files generated in this directory.  Binaries go under _PRG,
## libraries under _LIB.  Higher level Makefiles use these variables
## unconditionally, so both _PRG and _LIB need to be defined.
src_Pcode_Plib_CF_PRG =
src_Pcode_Plib_CF_LIB = src/Pcode/Plib_CF/libpcode_cf.a
src_Pcode_Plib_CF_DST = src/Pcode/Plib_CF/doxygen_config.doc \
	src/Pcode/Plib_CF/doxygen_config.tag

## A target to support building everything under this subdirectory.  This
## should refer to anything defined above.  There should be two targets; one
## with and one without the trailing slash.  The trailing slash will allow
## the use of tab completion.
src/Pcode/Plib_CF src/Pcode/Plib_CF/: \
	include $(src_Pcode_Plib_CF_PRG) $(src_Pcode_Plib_CF_LIB)

## A target to support cleaning everything under this subdirectory.
src/Pcode/Plib_CF/clean:
	rm -f $(top_builddir)/src/Pcode/Plib_CF/*.$(OBJEXT)
	rm -f $(src_Pcode_Plib_CF_LIB)

## List all source and header files under _SOURCES.  
src_Pcode_Plib_CF_libpcode_cf_a_SOURCES = \
	src/Pcode/Plib_CF/cfg.c src/Pcode/Plib_CF/cfg.h \
	src/Pcode/Plib_CF/dom.c src/Pcode/Plib_CF/dom.h \
	src/Pcode/Plib_CF/loop.c src/Pcode/Plib_CF/loop.h

## A target to build the tag file for this subdirectory's documentation.
## The tag file contains information on every documented symbol so that
## documentation in other directories can be linked to documentation in
## this one.
$(srcdir)/src/Pcode/Plib_CF/libpcode_cf.tag: \
		$(srcdir)/src/Pcode/Plib_CF/doxygen_config.tag \
		$(src_Pcode_Plib_CF_libpcode_cf_SOURCES)
	cd $(srcdir)/src/Pcode/Plib_CF && doxygen doxygen_config.tag

## A target to build all documentation under this subdirectory.  This
## build the human readable documentation and links it to documentation
## in other directories.
src/Pcode/Plib_CF/doc $(srcdir)/src/Pcode/Plib_CF/html \
	$(srcdir)/src/Pcode/Plib_CF/latex: \
		$(srcdir)/src/Pcode/Plib_CF/doxygen_config.doc \
		$(srcdir)/src/Pcode/Plib_CF/libpcode_cf.tag \
		$(srcdir)/src/Pcode/Pcode/libpcode.tag \
		$(srcdir)/src/library/libimpact.tag
	cd $(srcdir)/src/Pcode/Plib_CF && doxygen doxygen_config.doc

## List all include files that should be symlinked into include under 
## noinst_..._INC.  These include files are not installed when the user
## does 'make install'.  This is appropriate for most include files.  If
## you have headers that need to be installed, see src/library/Makefile.am
## for an example.
noinst_src_Pcode_Plib_CF_INC = \
	src/Pcode/Plib_CF/cfg.h src/Pcode/Plib_CF/dom.h \
	src/Pcode/Plib_CF/loop.h

## The class for the include files.  This is the directory under the include
## directory where the symlinks will be created.  This needs to have the same
## variable name as the _INC variable, only with a _CLASS suffix.
noinst_src_Pcode_Plib_CF_CLASS = Pcode

## List any libraries needed when linking under _LDADD.  If a library is built
## as part of impact, list it as $(top_builddir)/lib/<library> so that it can
## be detected as a dependency automatically.  Automake assumes libraries
## listed as -l<lib> are system libraries and does not detect them as
## dependencies.
##src_Pcode_Plib_CF_libpcode_cf_a_LDADD = 

## List any dependencies other than libraries under _DEPENDENDIES.  Defining
## this variable disables automatic dependency detection for this target, so
## you will need to relist dependencies that appear in _LDADD.
## src_Pcode_Plib_CF_Plib_CF_DEPENDENCIES = 

## List any special compiler flags under _CFLAGS.
src_Pcode_Plib_CF_libpcode_cf_a_CFLAGS = -DNEWPCODE $(AM_CFLAGS)


