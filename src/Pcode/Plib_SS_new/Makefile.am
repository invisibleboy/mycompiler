## src/Pcode/Plib_SS_new/Makefile.am
## This file is (ultimately) included by the top level Makefile.am, so all
## files need to be specified relative to the top level directory.

## List the files generated in this directory.  Binaries go under _PRG,
## libraries under _LIB.  Higher level Makefiles use these variables
## unconditionally, so both _PRG and _LIB need to be defined.
src_Pcode_Plib_SS_new_PRG = src/Pcode/Plib_SS_new/PcodeSSA_new
src_Pcode_Plib_SS_new_LIB = src/Pcode/Plib_SS_new/libpcode_ssa_new.a
src_Pcode_Plib_SS_new_DST = src/Pcode/Plib_SS_new/doxygen_config.doc \
	src/Pcode/Plib_SS_new/doxygen_config.tag

## A target to support building everything under this subdirectory.  This
## should refer to anything defined above.  There should be two targets; one
## with and one without the trailing slash.  The trailing slash will allow
## the use of tab completion.
src/Pcode/Plib_SS_new src/Pcode/Plib_SS_new/: \
	include $(src_Pcode_Plib_SS_new_PRG) $(src_Pcode_Plib_SS_new_LIB)

## A target to support cleaning everything under this subdirectory.
src/Pcode/Plib_SS_new/clean:
	rm -f $(top_builddir)/src/Pcode/Plib_SS_new/*.$(OBJEXT)
	rm -rf $(srcdir)/src/Pcode/Plib_SS_new/html
	rm -rf $(srcdir)/src/Pcode/Plib_SS_new/latex
	rm -f $(srcdir)/src/Pcode/Plib_SS_new/libpcode_cf.tag
	rm -f $(src_Pcode_Plib_SS_new_PRG)
	rm -f $(src_Pcode_Plib_SS_new_LIB)

## List all source and header files under _SOURCES.  
src_Pcode_Plib_SS_new_SRC = \
	src/Pcode/Plib_SS_new/ss_ext.c \
	src/Pcode/Plib_SS_new/ss_ind_expr.c src/Pcode/Plib_SS_new/ss_ind_expr.h \
	src/Pcode/Plib_SS_new/ss_induct2.c src/Pcode/Plib_SS_new/ss_induct2.h \
	src/Pcode/Plib_SS_new/ss_lifetime.c src/Pcode/Plib_SS_new/ss_lifetime.h \
	src/Pcode/Plib_SS_new/ss_print.c \
	src/Pcode/Plib_SS_new/ss_ssa2.c src/Pcode/Plib_SS_new/ss_ssa2.h \
	src/Pcode/Plib_SS_new/ss_struct.c \
	src/Pcode/Plib_SS_new/test_driver.c

src_Pcode_Plib_SS_new_libpcode_ssa_new_a_SOURCES = $(src_Pcode_Plib_SS_new_SRC)

src_Pcode_Plib_SS_new_PcodeSSA_new_SOURCES = $(src_Pcode_Plib_SS_new_SRC)

## A target to build the tag file for this subdirectory's documentation.
## The tag file contains information on every documented symbol so that
## documentation in other directories can be linked to documentation in
## this one.
$(srcdir)/src/Pcode/Plib_SS_new/libpcode_ssa_new.tag: \
		$(srcdir)/src/Pcode/Plib_SS_new/doxygen_config.tag
	cd $(srcdir)/src/Pcode/Plib_SS_new && doxygen doxygen_config.tag

## A target to build all documentation under this subdirectory.  This
## builds the human readable documentation and links it to documentation
## in other directories.
src/Pcode/Plib_SS_new/doc $(srcdir)/src/Pcode/Plib_SS_new/html \
	$(srcdir)/src/Pcode/Plib_SS_new/latex: \
		$(srcdir)/src/Pcode/Plib_SS_new/doxygen_config.doc \
		$(srcdir)/src/Pcode/Plib_CF/libpcode_cf.tag \
		$(srcdir)/src/Pcode/Pcode/libpcode.tag \
		$(srcdir)/src/library/libimpact.tag
	cd $(srcdir)/src/Pcode/Plib_SS_new && doxygen doxygen_config.doc

## List all include files that should be symlinked into include under 
## noinst_..._INC.  These include files are not installed when the user
## does 'make install'.  This is appropriate for most include files.  If
## you have headers that need to be installed, see src/library/Makefile.am
## for an example.
noinst_src_Pcode_Plib_SS_new_INC = \
	src/Pcode/Plib_SS_new/ss_ind_expr.h \
	src/Pcode/Plib_SS_new/ss_induct.h \
	src/Pcode/Plib_SS_new/ss_setup.h \
	src/Pcode/Plib_SS_new/ss_ssa2.h \
	src/Pcode/Plib_SS_new/ss_lifetime.h

## The class for the include files.  This is the directory under the include
## directory where the symlinks will be created.  This needs to have the same
## variable name as the _INC variable, only with a _CLASS suffix.
noinst_src_Pcode_Plib_SS_new_CLASS = Pcode

## List any dependencies other than libraries under _DEPENDENDIES.  Defining
## this variable disables automatic dependency detection for this target, so
## you will need to relist dependencies that appear in _LDADD.
## src_Pcode_Plib_SS_new_lib_ssa_a_DEPENDENCIES = 

src_Pcode_Plib_SS_new_PcodeSSA_new_LDADD = -L$(top_builddir)/lib -lm \
	$(top_builddir)/lib/libpcode.a \
	$(top_builddir)/lib/libpcode_cf.a \
	$(top_builddir)/lib/libpcode_probe.a \
	$(top_builddir)/lib/libimpact.a 

## List any special compiler flags under _CFLAGS.
src_Pcode_Plib_SS_new_libpcode_ssa_new_a_CFLAGS = -DNEWPCODE $(AM_CFLAGS)
