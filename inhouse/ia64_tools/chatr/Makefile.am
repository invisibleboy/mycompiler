## inhouse/ia64_tools/chatr/Makefile.am
## This file is (ultimately) included by the top level Makefile.am, so all
## files need to be specified relative to the top level directory.

## List the files generated in this directory.  Binaries go under _PRG,
## libraries under _LIB.  Higher level Makefiles use these variables
## unconditionally, so both _PRG and _LIB need to be defined.
inhouse_ia64_tools_chatr_PRG = inhouse/ia64_tools/chatr/chatr
inhouse_ia64_tools_chatr_LIB = 

## A target to support building everything under this subdirectory.  This
## should refer to anything defined above.  There should be two targets; one
## with and one without the trailing slash.  The trailing slash will allow
## the use of tab completion.
inhouse/ia64_tools/chatr inhouse/ia64_tools/chatr/: \
	include $(inhouse_ia64_tools_chatr_PRG)

## A target to support cleaning everything under this subdirectory.
inhouse/ia64_tools/chatr/clean:
	rm -f $(top_builddir)/inhouse/ia64_tools/chatr/*.$(OBJEXT)
	rm -f $(inhouse_ia64_tools_chatr_PRG)

## List all source and header files under _SOURCES.  If the only source file
## has the same name as the binary, only with a .c extension, this can be
## omitted.
## inhouse_ia64_tools_chatr_chatr_SOURCES = 

## List any libraries needed when linking under _LDADD.  If a library is built
## as part of impact, list it as $(top_builddir)/lib/<library> so that it can
## be detected as a dependency automatically.  Automake assumes libraries
## listed as -l<lib> are system libraries and does not detect them as
## dependencies.
## inhouse_ia64_tools_chatr_chatr_LDADD = 

## List any dependencies other than libraries under _DEPENDENDIES.  Defining
## this variable disables automatic dependency detection for this target, so
## you will need to relist dependencies that appear in _LDADD.
## inhouse_ia64_tools_chatr_chatr_DEPENDENCIES = 

## List any special compiler flags under _CFLAGS.
## inhouse_ia64_tools_chatr_chatr_CFLAGS = 

