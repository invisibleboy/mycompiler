## inhouse/dumpelfsection/Makefile.am
## This file is (ultimately) included by the top level Makefile.am, so all
## files need to be specified relative to the top level directory.

## List the files generated in this directory.  Binaries go under _PRG,
## libraries under _LIB.  Higher level Makefiles use these variables
## unconditionally, so both _PRG and _LIB need to be defined.

## ntclark: we don't have the library needed, and I'm sick of the
## stupid compile error.
## inhouse_dumpelfsection_PRG = inhouse/dumpelfsection/dumpelfsection
inhouse_dumpelfsection_PRG = 
inhouse_dumpelfsection_LIB = 

## A target to support building everything under this subdirectory.  This
## should refer to anything defined above.  There should be two targets; one
## with and one without the trailing slash.  The trailing slash will allow
## the use of tab completion.
inhouse/dumpelfsection inhouse/dumpelfsection/: \
	include $(inhouse_dumpelfsection_PRG)

## A target to support cleaning everything under this subdirectory.
inhouse/dumpelfsection/clean:
	rm -f $(top_builddir)/inhouse/dumpelfsection/*.$(OBJEXT)
	rm -f $(inhouse_dumpelfsection_PRG)

## List all source and header files under _SOURCES.  If the only source file
## has the same name as the binary, only with a .c extension, this can be
## omitted.
## inhouse_dumpelfsection_dumpelfsection_SOURCES = 

## List any libraries needed when linking under _LDADD.  If a library is built
## as part of impact, list it as $(top_builddir)/lib/<library> so that it can
## be detected as a dependency automatically.  Automake assumes libraries
## listed as -l<lib> are system libraries and does not detect them as
## dependencies.
## ntclark:
## inhouse_dumpelfsection_dumpelfsection_LDADD = -lelf

## List any dependencies other than libraries under _DEPENDENDIES.  Defining
## this variable disables automatic dependency detection for this target, so
## you will need to relist dependencies that appear in _LDADD.
## inhouse_dumpelfsection_dumpelfsection_DEPENDENCIES = 

## List any special compiler flags under _CFLAGS.
## inhouse_dumpelfsection_dumpelfsection_CFLAGS = 
