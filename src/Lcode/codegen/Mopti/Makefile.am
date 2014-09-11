## src/Lcode/codegen/Mopti/Makefile.am
## This file is (ultimately) included by the top level Makefile.am, so all
## files need to be specified relative to the top level directory.

## List the files generated in this directory.  Binaries go under _PRG,
## libraries under _LIB.  Higher level Makefiles use these variables
## unconditionally, so both _PRG and _LIB need to be defined.
src_Lcode_codegen_Mopti_PRG = 
src_Lcode_codegen_Mopti_LIB = src/Lcode/codegen/Mopti/libmopti.a
src_Lcode_codegen_Mopti_DST = src/Lcode/codegen/Mopti/doxygen_config.doc \
	src/Lcode/codegen/Mopti/doxygen_config.tag

## A target to support building everything under this subdirectory.  This
## should refer to anything defined above.  There should be two targets; one
## with and one without the trailing slash.  The trailing slash will allow
## the use of tab completion.
src/Lcode/codegen/Mopti src/Lcode/codegen/Mopti/: \
	include $(src_Lcode_codegen_Mopti_LIB)

## A target to support cleaning everything under this subdirectory.
src/Lcode/codegen/Mopti/clean:
	rm -f $(top_builddir)/src/Lcode/codegen/Mopti/*.$(OBJEXT)
	rm -rf $(srcdir)/src/Lcode/codegen/Mopti/html
	rm -rf $(srcdir)/src/Lcode/codegen/Mopti/latex
	rm -f $(srcdir)/src/Lcode/codegen/Mopti/libmopti.tag
	rm -f $(src_Lcode_codegen_Mopti_LIB)

## List all source and header files under _SOURCES.  
src_Lcode_codegen_Mopti_libmopti_a_SOURCES = \
	src/Lcode/codegen/Mopti/m_opti.c \
	src/Lcode/codegen/Mopti/m_opti.h

## A target to build the tag file for this subdirectory's documentation.
## The tag file contains information on every documented symbol so that
## documentation in other directories can be linked to documentation in
## this one.
$(srcdir)/src/Lcode/codegen/Mopti/libmopti.tag: \
		$(srcdir)/src/Lcode/codegen/Mopti/doxygen_config.tag \
		$(src_Lcode_codegen_Mopti_libmopti_a_SOURCES)
	cd $(srcdir)/src/Lcode/codegen/Mopti && doxygen doxygen_config.tag

## A target to build all documentation under this subdirectory.  This
## builds the human readable documentation and links it to documentation
## in other directories.
src/Lcode/codegen/Mopti/doc $(srcdir)/src/Lcode/codegen/Mopti/html \
	$(srcdir)/src/Lcode/codegen/Mopti/latex: \
		$(srcdir)/src/Lcode/codegen/Mopti/doxygen_config.doc \
		$(srcdir)/src/Lcode/codegen/Mopti/libmopti.tag \
		$(srcdir)/src/Lcode/Lcode/liblcode.tag \
		$(srcdir)/src/Lcode/opti/Lopti/Lopti.tag
	cd $(srcdir)/src/Lcode/codegen/Mopti && doxygen doxygen_config.doc

## List all include files that should be symlinked into include under 
## noinst_..._INC.  These include files are not installed when the user
## does 'make install'.  This is appropriate for most include files.  If
## you have headers that need to be installed, see src/library/Makefile.am
## for an example.
noinst_src_Lcode_codegen_Mopti_INC = src/Lcode/codegen/Mopti/m_opti.h

## The class for the include files.  This is the directory under the include
## directory where the symlinks will be created.  This needs to have the same
## variable name as the _INC variable, only with a _CLASS suffix.
noinst_src_Lcode_codegen_Mopti_CLASS = Lcode

## List any dependencies other than libraries under _DEPENDENDIES.  Defining
## this variable disables automatic dependency detection for this target, so
## you will need to relist dependencies that appear in _LDADD.
## src_Lcode_codegen_Mopti_libmopti_a_DEPENDENCIES = 

## List any special compiler flags under _CFLAGS.
## src_Lcode_codegen_Mopti_libmopti_a_CFLAGS = 
