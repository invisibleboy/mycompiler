## src/Pcode/Pinline/Makefile.am
## This file is (ultimately) included by the top level Makefile.am, so all
## files need to be specified relative to the top level directory.

## List the files generated in this directory.  Binaries go under _PRG,
## libraries under _LIB.  Higher level Makefiles use these variables
## unconditionally, so both _PRG and _LIB need to be defined.
src_Pcode_Pinline_PRG = src/Pcode/Pinline/Pinline
src_Pcode_Pinline_LIB = 
src_Pcode_Pinline_DST = src/Pcode/Pinline/doxygen_config.doc \
	src/Pcode/Pinline/doxygen_config.tag

## A target to support building everything under this subdirectory.  This
## should refer to anything defined above.  There should be two targets; one
## with and one without the trailing slash.  The trailing slash will allow
## the use of tab completion.
src/Pcode/Pinline src/Pcode/Pinline/: \
	include $(src_Pcode_Pinline_PRG) $(src_Pcode_Pinline_LIB)

## A target to support cleaning everything under this subdirectory.
src/Pcode/Pinline/clean:
	rm -f $(top_builddir)/src/Pcode/Pinline/*.$(OBJEXT)
	rm -rf $(srcdir)/src/Pcode/Pinline/html
	rm -rf $(srcdir)/src/Pcode/Pinline/latex
	rm -f $(srcdir)/src/Pcode/Pinline/Pinline.tag
	rm -f $(src_Pcode_Pinline_PRG)
	rm -f $(src_Pcode_Pinline_LIB)

src_Pcode_Pinline_Pinline_SOURCES = \
	src/Pcode/Pinline/pin_annotate.c \
	src/Pcode/Pinline/pin_callgraph.c \
	src/Pcode/Pinline/pin_callgraph.h \
	src/Pcode/Pinline/pin_cg_dot.c \
	src/Pcode/Pinline/pin_cg_inline.c \
	src/Pcode/Pinline/pin_expand.c src/Pcode/Pinline/pin_inline.c \
	src/Pcode/Pinline/pin_inline.h src/Pcode/Pinline/pin_util.c

## A target to build the tag file for this subdirectory's documentation.
## The tag file contains information on every documented symbol so that
## documentation in other directories can be linked to documentation in
## this one.
$(srcdir)/src/Pcode/Pinline/Pinline.tag: \
		$(srcdir)/src/Pcode/Pinline/doxygen_config.tag \
		$(src_Pcode_Pinline_SRC)
	cd $(srcdir)/src/Pcode/Pinline && doxygen doxygen_config.tag

## A target to build all documentation under this subdirectory.  This
## builds the human readable documentation and links it to documentation
## in other directories.
src/Pcode/Pinline/doc $(srcdir)/src/Pcode/Pinline/html \
	$(srcdir)/src/Pcode/Pinline/latex: \
		$(srcdir)/src/Pcode/Pinline/doxygen_config.doc \
		$(srcdir)/src/Pcode/Pinline/Pinline.tag \
		$(srcdir)/src/Pcode/Pcode/libpcode.tag \
		$(srcdir)/src/Pcode/Plib_CF/libpcode_cf.tag \
		$(srcdir)/src/machine/Mspec/libmspec.tag \
		$(srcdir)/src/library/libimpact.tag
	cd $(srcdir)/src/Pcode/Pinline && doxygen doxygen_config.doc

## List all include files that should be symlinked into include under 
## noinst_..._INC.  These include files are not installed when the user
## does 'make install'.  This is appropriate for most include files.  If
## you have headers that need to be installed, see src/library/Makefile.am
## for an example.
## noinst_src_Pcode_Pinline_INC =

## The class for the include files.  This is the directory under the include
## directory where the symlinks will be created.  This needs to have the same
## variable name as the _INC variable, only with a _CLASS suffix.
## noinst_src_Pcode_Pinline_CLASS =

## List any libraries needed when linking under _LDADD.  If a library is built
## as part of impact, list it as $(top_builddir)/lib/<library> so that it can
## be detected as a dependency automatically.  Automake assumes libraries
## listed as -l<lib> are system libraries and does not detect them as
## dependencies.
## $(top_builddir)/lib/libpcode_dd.a \
##	$(top_builddir)/lib/libpcode_ip.a $(top_builddir)/lib/libpcode_cf.a \
##	$(top_builddir)/lib/libpcode_df.a $(top_builddir)/lib/libmspec.a \
src_Pcode_Pinline_Pinline_LDADD = -L$(top_builddir)/lib \
	$(top_builddir)/lib/libpcode.a $(top_builddir)/lib/libpcode_cf.a \
	$(top_builddir)/lib/libmspec.a $(top_builddir)/lib/libimpact.a -lm

## List any dependencies other than libraries under _DEPENDENDIES.  Defining
## this variable disables automatic dependency detection for this target, so
## you will need to relist dependencies that appear in _LDADD.
## src_Pcode_Pinline_Pinline_DEPENDENCIES = 

## List any special compiler flags under _CFLAGS.
## src_Pcode_Pinline_Pinline_CFLAGS = 

