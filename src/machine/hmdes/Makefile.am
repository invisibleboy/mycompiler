## src/machine/hmdes/Makefile.am
## This file is (ultimately) included by the top level Makefile.am, so all
## files need to be specified relative to the top level directory.

## List the files generated in this directory.  Binaries go under _PRG,
## libraries under _LIB.  Higher level Makefiles use these variables
## unconditionally, so both _PRG and _LIB need to be defined.
src_machine_hmdes_PRG =
src_machine_hmdes_LIB = src/machine/hmdes/libhmdes.a
src_machine_hmdes_DST = src/machine/hmdes/doxygen_config.doc \
	src/machine/hmdes/doxygen_config.tag

## A target to support building everything under this subdirectory.  This
## should refer to anything defined above.  There should be two targets; one
## with and one without the trailing slash.  The trailing slash will allow
## the use of tab completion.
src/machine/hmdes src/machine/hmdes/: include $(src_machine_hmdes_LIB)

## A target to support cleaning everything under this subdirectory.
src/machine/hmdes/clean:
	rm -f $(top_builddir)/src/machine/hmdes/*.$(OBJEXT)
	rm -rf $(srcdir)/src/machine/hmdes/html
	rm -rf $(srcdir)/src/machine/hmdes/latex
	rm -f $(srcdir)/src/machine/hmdes/libhmdes.tag
	rm -f $(src_machine_hmdes_LIB)

## List all source and header files under _SOURCES.  
src_machine_hmdes_libhmdes_a_SOURCES = \
	src/machine/hmdes/header_reader.c src/machine/hmdes/hmdes.c \
	src/machine/hmdes/hmdes.h

## A target to build the tag file for this subdirectory's documentation.
## The tag file contains information on every documented symbol so that
## documentation in other directories can be linked to documentation in
## this one.
$(srcdir)/src/machine/hmdes/libhmdes.tag: \
		$(srcdir)/src/machine/hmdes/doxygen_config.tag \
		$(src_machine_hmdes_libhmdes_a_SOURCES)
	cd $(srcdir)/src/machine/hmdes && doxygen doxygen_config.tag

## A target to build all documentation under this subdirectory.  This
## builds the human readable documentation and links it to documentation
## in other directories.
src/machine/hmdes/doc $(srcdir)/src/machine/hmdes/html \
	$(srcdir)/src/machine/hmdes/latex: \
		$(srcdir)/src/machine/hmdes/doxygen_config.doc \
		$(srcdir)/src/machine/hmdes/libhmdes.tag \
		$(srcdir)/src/library/libimpact.tag
	cd $(srcdir)/src/machine/hmdes && doxygen doxygen_config.doc

## List all include files that should be symlinked into include under 
## noinst_..._INC.  These include files are not installed when the user
## does 'make install'.  This is appropriate for most include files.  If
## you have headers that need to be installed, see src/library/Makefile.am
## for an example.
noinst_src_machine_hmdes_INC = src/machine/hmdes/hmdes.h

## The class for the include files.  This is the directory under the include
## directory where the symlinks will be created.  This needs to have the same
## variable name as the _INC variable, only with a _CLASS suffix.
noinst_src_machine_hmdes_CLASS = machine

## List any dependencies other than libraries under _DEPENDENDIES.  Defining
## this variable disables automatic dependency detection for this target, so
## you will need to relist dependencies that appear in _LDADD.
## src_machine_hmdes_libhmdes_a_DEPENDENCIES = 

## List any special compiler flags under _CFLAGS.
## src_machine_hmdes_libhmdes_a_CFLAGS = 


