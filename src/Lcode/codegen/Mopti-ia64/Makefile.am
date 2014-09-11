## src/Lcode/codegen/Mopti-ia64/Makefile.am
## This file is (ultimately) included by the top level Makefile.am, so all
## files need to be specified relative to the top level directory.

## List the files generated in this directory.  Binaries go under _PRG,
## libraries under _LIB.  Higher level Makefiles use these variables
## unconditionally, so both _PRG and _LIB need to be defined.
src_Lcode_codegen_Mopti_ia64_PRG = 
src_Lcode_codegen_Mopti_ia64_LIB = src/Lcode/codegen/Mopti-ia64/libiamopti.a
src_Lcode_codegen_Mopti_ia64_DST = \
	src/Lcode/codegen/Mopti-ia64/doxygen_config.doc \
	src/Lcode/codegen/Mopti-ia64/doxygen_config.tag

## A target to support building everything under this subdirectory.  This
## should refer to anything defined above.  There should be two targets; one
## with and one without the trailing slash.  The trailing slash will allow
## the use of tab completion.
src/Lcode/codegen/Mopti-ia64 src/Lcode/codegen/Mopti-ia64/: \
	include $(src_Lcode_codegen_Mopti_ia64_LIB)

## A target to support cleaning everything under this subdirectory.
src/Lcode/codegen/Mopti-ia64/clean:
	rm -f $(top_builddir)/src/Lcode/codegen/Mopti-ia64/*.$(OBJEXT)
	rm -rf $(srcdir)/src/Lcode/codegen/Mopti-ia64/html
	rm -rf $(srcdir)/src/Lcode/codegen/Mopti-ia64/latex
	rm -f $(srcdir)/src/Lcode/codegen/Mopti-ia64/libiamopti.tag
	rm -f $(src_Lcode_codegen_Mopti_ia64_LIB)

## List all source and header files under _SOURCES.  
src_Lcode_codegen_Mopti_ia64_libiamopti_a_SOURCES = \
	src/Lcode/codegen/Mopti-ia64/mia_compare.c \
	src/Lcode/codegen/Mopti-ia64/mia_epi_merge.c \
	src/Lcode/codegen/Mopti-ia64/mia_internal.h \
	src/Lcode/codegen/Mopti-ia64/mia_jump_opti.c \
	src/Lcode/codegen/Mopti-ia64/mia_lp_prel.c \
	src/Lcode/codegen/Mopti-ia64/mia_opti.c \
	src/Lcode/codegen/Mopti-ia64/mia_opti.h \
	src/Lcode/codegen/Mopti-ia64/mia_post_inc.c \
	src/Lcode/codegen/Mopti-ia64/mia_shladd.c

## A target to build the tag file for this subdirectory's documentation.
## The tag file contains information on every documented symbol so that
## documentation in other directories can be linked to documentation in
## this one.
$(srcdir)/src/Lcode/codegen/Mopti-ia64/libiamopti.tag: \
		$(srcdir)/src/Lcode/codegen/Mopti-ia64/doxygen_config.tag \
		$(src_Lcode_codegen_Mopti_ia64_libiamopti_a_SOURCES)
	cd $(srcdir)/src/Lcode/codegen/Mopti-ia64 && doxygen doxygen_config.tag

## A target to build all documentation under this subdirectory.  This
## builds the human readable documentation and links it to documentation
## in other directories.
src/Lcode/codegen/Mopti-ia64/doc $(srcdir)/src/Lcode/codegen/Mopti-ia64/html \
	$(srcdir)/src/Lcode/codegen/Mopti-ia64/latex: \
		$(srcdir)/src/Lcode/codegen/Mopti-ia64/doxygen_config.doc \
		$(srcdir)/src/Lcode/codegen/Mopti-ia64/libiamopti.tag \
		$(srcdir)/src/Lcode/Lcode/liblcode.tag \
		$(srcdir)/src/Lcode/opti/Lopti/Lopti.tag \
		$(srcdir)/src/Lcode/codegen/Ltahoe/Ltahoe.tag \
		$(srcdir)/src/machine/Mspec/libmspec.tag
	cd $(srcdir)/src/Lcode/codegen/Mopti-ia64 && doxygen doxygen_config.doc

## List all include files that should be symlinked into include under 
## noinst_..._INC.  These include files are not installed when the user
## does 'make install'.  This is appropriate for most include files.  If
## you have headers that need to be installed, see src/library/Makefile.am
## for an example.
noinst_src_Lcode_codegen_Mopti_ia64_INC = \
	src/Lcode/codegen/Mopti-ia64/mia_opti.h

## The class for the include files.  This is the directory under the include
## directory where the symlinks will be created.  This needs to have the same
## variable name as the _INC variable, only with a _CLASS suffix.
noinst_src_Lcode_codegen_Mopti_ia64_CLASS = Lcode

## List any dependencies other than libraries under _DEPENDENDIES.  Defining
## this variable disables automatic dependency detection for this target, so
## you will need to relist dependencies that appear in _LDADD.
## src_Lcode_codegen_Mopti_ia64_libiamopti_a_DEPENDENCIES = 

## List any special compiler flags under _CFLAGS.
## src_Lcode_codegen_Mopti_ia64_libiamopti_a_CFLAGS = 
