## src/Pcode/Pmerge_prof/Makefile.am
## This file is (ultimately) included by the top level Makefile.am, so all
## files need to be specified relative to the top level directory.

## List the files generated in this directory.  Binaries go under _PRG,
## libraries under _LIB.  Higher level Makefiles use these variables
## unconditionally, so both _PRG and _LIB need to be defined.
src_Pcode_Pmerge_prof_PRG = src/Pcode/Pmerge_prof/Pmerge_prof
src_Pcode_Pmerge_prof_LIB = 
src_Pcode_Pmerge_prof_DST = src/Pcode/Pmerge_prof/doxygen_config.doc \
	src/Pcode/Pmerge_prof/doxygen_config.tag

## A target to support building everything under this subdirectory.  This
## should refer to anything defined above.  There should be two targets; one
## with and one without the trailing slash.  The trailing slash will allow
## the use of tab completion.
src/Pcode/Pmerge_prof src/Pcode/Pmerge_prof/: \
	include $(src_Pcode_Pmerge_prof_PRG)

## A target to support cleaning everything under this subdirectory.
src/Pcode/Pmerge_prof/clean:
	rm -f $(top_builddir)/src/Pcode/Pmerge_prof/*.$(OBJEXT)
	rm -rf $(srcdir)/src/Pcode/Pmerge_prof/html
	rm -rf $(srcdir)/src/Pcode/Pmerge_prof/latex
	rm -f $(srcdir)/src/Pcode/Pmerge_prof/Pmerge_prof.tag
	rm -f $(src_Pcode_Pmerge_prof_PRG)

## List all source and header files under _SOURCES.  
src_Pcode_Pmerge_prof_Pmerge_prof_SOURCES = src/Pcode/Pmerge_prof/merge_prof.c

## A target to build the tag file for this subdirectory's documentation.
## The tag file contains information on every documented symbol so that
## documentation in other directories can be linked to documentation in
## this one.
$(srcdir)/src/Pcode/Pmerge_prof/Pmerge_prof.tag: \
		$(srcdir)/src/Pcode/Pmerge_prof/doxygen_config.tag \
		$(src_Pcode_Pmerge_prof_Pmerge_prof_SOURCES)
	cd $(srcdir)/src/Pcode/Pmerge_prof && doxygen doxygen_config.tag

## A target to build all documentation under this subdirectory.  This
## builds the human readable documentation and links it to documentation
## in other directories.
src/Pcode/Pmerge_prof/doc $(srcdir)/src/Pcode/Pmerge_prof/html \
	$(srcdir)/src/Pcode/Pmerge_prof/latex: \
		$(srcdir)/src/Pcode/Pmerge_prof/doxygen_config.doc \
		$(srcdir)/src/Pcode/Pmerge_prof/Pmerge_prof.tag
	cd $(srcdir)/src/Pcode/Pmerge_prof && doxygen doxygen_config.doc

## List any libraries needed when linking under _LDADD.  If a library is built
## as part of impact, list it as $(top_builddir)/lib/<library> so that it can
## be detected as a dependency automatically.  Automake assumes libraries
## listed as -l<lib> are system libraries and does not detect them as
## dependencies.
## src_Pcode_Pmerge_prof_Pmerge_prof_SOURCES = 

## List all include files that should be symlinked into include under 
## noinst_..._INC.  These include files are not installed when the user
## does 'make install'.  This is appropriate for most include files.  If
## you have headers that need to be installed, see src/library/Makefile.am
## for an example.
## noinst_src_Pcode_Pmerge_prof_INC = 

## The class for the include files.  This is the directory under the include
## directory where the symlinks will be created.  This needs to have the same
## variable name as the _INC variable, only with a _CLASS suffix.
## noinst_src_Pcode_Pmerge_prof_CLASS = 

## List any dependencies other than libraries under _DEPENDENDIES.  Defining
## this variable disables automatic dependency detection for this target, so
## you will need to relist dependencies that appear in _LDADD.
## src_Pcode_Pmerge_prof_Pmerge_prof_DEPENDENCIES = 

## List any special compiler flags under _CFLAGS.
## src_Pcode_Pmerge_prof_Pmerge_prof_CFLAGS = 

