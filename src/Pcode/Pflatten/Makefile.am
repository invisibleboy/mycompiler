## src/Pcode/Pflatten/Makefile.am
## This file is (ultimately) included by the top level Makefile.am, so all
## files need to be specified relative to the top level directory.

## List the files generated in this directory.  Binaries go under _PRG,
## libraries under _LIB.  Higher level Makefiles use these variables
## unconditionally, so both _PRG and _LIB need to be defined.
src_Pcode_Pflatten_PRG = src/Pcode/Pflatten/Pflatten
src_Pcode_Pflatten_LIB = src/Pcode/Pflatten/libpcode_flatten.a
src_Pcode_Pflatten_DST = src/Pcode/Pflatten/doxygen_config.doc \
	src/Pcode/Pflatten/doxygen_config.tag

## A target to support building everything under this subdirectory.  This
## should refer to anything defined above.  There should be two targets; one
## with and one without the trailing slash.  The trailing slash will allow
## the use of tab completion.
src/Pcode/Pflatten src/Pcode/Pflatten/: \
	include $(src_Pcode_Pflatten_PRG) $(src_Pcode_Pflatten_LIB)

## A target to support cleaning everything under this subdirectory.
src/Pcode/Pflatten/clean:
	rm -f $(top_builddir)/src/Pcode/Pflatten/*.$(OBJEXT)
	rm -rf $(srcdir)/src/Pcode/Pflatten/html
	rm -rf $(srcdir)/src/Pcode/Pflatten/latex
	rm -f $(srcdir)/src/Pcode/Pflatten/Pflatten.tag
	rm -f $(src_Pcode_Pflatten_PRG)
	rm -f $(src_Pcode_Pflatten_LIB)

src_Pcode_Pflatten_SRC = \
	src/Pcode/Pflatten/data.c src/Pcode/Pflatten/data.h \
	src/Pcode/Pflatten/do_pcode.c src/Pcode/Pflatten/flatten.c \
	src/Pcode/Pflatten/flatten.h src/Pcode/Pflatten/libflatten.c \
	src/Pcode/Pflatten/process.c src/Pcode/Pflatten/process.h \
	src/Pcode/Pflatten/pf_array.c \
	src/Pcode/Pflatten/pf_restructure.c \
	src/Pcode/Pflatten/pf_utility.c

## A target to build the tag file for this subdirectory's documentation.
## The tag file contains information on every documented symbol so that
## documentation in other directories can be linked to documentation in
## this one.
$(srcdir)/src/Pcode/Pflatten/Pflatten.tag: \
		$(srcdir)/src/Pcode/Pflatten/doxygen_config.tag \
		$(src_Pcode_Pflatten_SRC)
	cd $(srcdir)/src/Pcode/Pflatten && doxygen doxygen_config.tag

## A target to build all documentation under this subdirectory.  This
## builds the human readable documentation and links it to documentation
## in other directories.
src/Pcode/Pflatten/doc $(srcdir)/src/Pcode/Pflatten/html \
	$(srcdir)/src/Pcode/Pflatten/latex: \
		$(srcdir)/src/Pcode/Pflatten/doxygen_config.doc \
		$(srcdir)/src/Pcode/Pflatten/Pflatten.tag \
		$(srcdir)/src/Pcode/Pcode/libpcode.tag \
		$(srcdir)/src/library/libimpact.tag
	cd $(srcdir)/src/Pcode/Pflatten && doxygen doxygen_config.doc

## List all source and header files under _SOURCES.  
src_Pcode_Pflatten_Pflatten_SOURCES = $(src_Pcode_Pflatten_SRC)

## List all include files that should be symlinked into include under 
## noinst_..._INC.  These include files are not installed when the user
## does 'make install'.  This is appropriate for most include files.  If
## you have headers that need to be installed, see src/library/Makefile.am
## for an example.
noinst_src_Pcode_Pflatten_INC = src/Pcode/Pflatten/flatten.h

## The class for the include files.  This is the directory under the include
## directory where the symlinks will be created.  This needs to have the same
## variable name as the _INC variable, only with a _CLASS suffix.
noinst_src_Pcode_Pflatten_CLASS = Pcode

## List any libraries needed when linking under _LDADD.  If a library is built
## as part of impact, list it as $(top_builddir)/lib/<library> so that it can
## be detected as a dependency automatically.  Automake assumes libraries
## listed as -l<lib> are system libraries and does not detect them as
## dependencies.
src_Pcode_Pflatten_Pflatten_LDADD = -L$(top_builddir)/lib \
	$(top_builddir)/lib/libpcode.a $(top_builddir)/lib/libimpact.a

## List any dependencies other than libraries under _DEPENDENDIES.  Defining
## this variable disables automatic dependency detection for this target, so
## you will need to relist dependencies that appear in _LDADD.
## src_Pcode_Pflatten_Pflatten_DEPENDENCIES = 

## List any special compiler flags under _CFLAGS.
## src_Pcode_Pflatten_Pflatten_CFLAGS = 

## List all source and header files under _SOURCES.  
src_Pcode_Pflatten_libpcode_flatten_a_SOURCES = $(src_Pcode_Pflatten_SRC)

## List any dependencies other than libraries under _DEPENDENDIES.  Defining
## this variable disables automatic dependency detection for this target, so
## you will need to relist dependencies that appear in _LDADD.
## src_Pcode_Pflatten_libpcode_flatten_a_DEPENDENCIES = 

## List any special compiler flags under _CFLAGS.
## src_Pcode_Pflatten_libpcode_flatten_a_CFLAGS = 

