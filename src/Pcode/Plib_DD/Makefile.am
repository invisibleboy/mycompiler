## src/Pcode/Plib_DD/Makefile.am
## This file is (ultimately) included by the top level Makefile.am, so all
## files need to be specified relative to the top level directory.

## List the files generated in this directory.  Binaries go under _PRG,
## libraries under _LIB.  Higher level Makefiles use these variables
## unconditionally, so both _PRG and _LIB need to be defined.
src_Pcode_Plib_DD_PRG = 
src_Pcode_Plib_DD_LIB = src/Pcode/Plib_DD/libpcode_dd.a
src_Pcode_Plib_DD_DST = src/Pcode/Plib_DD/doxygen_config.doc \
	src/Pcode/Plib_DD/doxygen_config.tag

## A target to support building everything under this subdirectory.  This
## should refer to anything defined above.  There should be two targets; one
## with and one without the trailing slash.  The trailing slash will allow
## the use of tab completion.
src/Pcode/Plib_DD src/Pcode/Plib_DD/: \
	include $(src_Pcode_Plib_DD_PRG) $(src_Pcode_Plib_DD_LIB)

## A target to support cleaning everything under this subdirectory.
src/Pcode/Plib_DD/clean:
	rm -f $(top_builddir)/src/Pcode/Plib_DD/*.$(OBJEXT)
	rm -rf $(srcdir)/src/Pcode/Plib_DD/html
	rm -rf $(srcdir)/src/Pcode/Plib_DD/latex
	rm -f $(srcdir)/src/Pcode/Plib_DD/libpcode_dd.tag
	rm -f $(src_Pcode_Plib_DD_LIB)

## List all source and header files under _SOURCES.  
src_Pcode_Plib_DD_libpcode_dd_a_SOURCES = \
	src/Pcode/Plib_DD/dd_dependence.c \
	src/Pcode/Plib_DD/dd_dir.h \
	src/Pcode/Plib_DD/dd_interface.c \
	src/Pcode/Plib_DD/dd_interface.h \
	src/Pcode/Plib_DD/dd_omega.h \
	src/Pcode/Plib_DD/dd_omega-ip.c \
	src/Pcode/Plib_DD/dd_omega-ip.h \
	src/Pcode/Plib_DD/dd_omega_driver.c \
	src/Pcode/Plib_DD/dd_preprocess.c \
	src/Pcode/Plib_DD/dd_preprocess.h \
	src/Pcode/Plib_DD/dd_setup.c \
	src/Pcode/Plib_DD/dd_setup.h

## A target to build the tag file for this subdirectory's documentation.
## The tag file contains information on every documented symbol so that
## documentation in other directories can be linked to documentation in
## this one.
$(srcdir)/src/Pcode/Plib_DD/libpcode_dd.tag: \
		$(srcdir)/src/Pcode/Plib_DD/doxygen_config.tag \
		$(src_Pcode_Plib_DD_libpcode_dd_SOURCES)
	cd $(srcdir)/src/Pcode/Plib_DD && doxygen doxygen_config.tag

## A target to build all documentation under this subdirectory.  This
## builds the human readable documentation and links it to documentation
## in other directories.
src/Pcode/Plib_DD/doc $(srcdir)/src/Pcode/Plib_DD/html \
	$(srcdir)/src/Pcode/Plib_DD/latex: \
		$(srcdir)/src/Pcode/Plib_DD/doxygen_config.doc \
		$(srcdir)/src/Pcode/Plib_DD/libpcode_dd.tag \
		$(srcdir)/src/Pcode/Pcode/libpcode.tag \
		$(srcdir)/src/library/libimpact.tag \
		$(srcdir)/src/Pcode/Plib_CF/libpcode_cf.tag
	cd $(srcdir)/src/Pcode/Plib_DD && doxygen doxygen_config.doc

## List all include files that should be symlinked into include under 
## noinst_..._INC.  These include files are not installed when the user
## does 'make install'.  This is appropriate for most include files.  If
## you have headers that need to be installed, see src/library/Makefile.am
## for an example.
noinst_src_Pcode_Plib_DD_INC = \
	src/Pcode/Plib_DD/dd_interface.h

## List any libraries needed when linking under _LDADD.  If a library is built
## as part of impact, list it as $(top_builddir)/lib/<library> so that it can
## be detected as a dependency automatically.  Automake assumes libraries
## listed as -l<lib> are system libraries and does not detect them as
## dependencies.
## src_Pcode_Plib_DD_libpcode_dd_a_LDADD = -L$(top_builddir)/lib -lm

## The class for the include files.  This is the directory under the include
## directory where the symlinks will be created.  This needs to have the same
## variable name as the _INC variable, only with a _CLASS suffix.
noinst_src_Pcode_Plib_DD_CLASS = Pcode

## List any dependencies other than libraries under _DEPENDENDIES.  Defining
## this variable disables automatic dependency detection for this target, so
## you will need to relist dependencies that appear in _LDADD.
## src_Pcode_Plib_DD_libpcode_dd_a_DEPENDENCIES = 

## List any special compiler flags under _CFLAGS.
src_Pcode_Plib_DD_libpcode_dd_a_CFLAGS = -DNEWPCODE $(AM_CFLAGS)
