## inhouse/ia64_tools/Makefile.am
## This file is (ultimately) included by the top level Makefile.am, so all
## files need to be specified relative to the top level directory.

include $(srcdir)/inhouse/ia64_tools/chatr/Makefile.am

inhouse_ia64_tools_PRG = $(inhouse_ia64_tools_chatr_PRG)
inhouse_ia64_tools_LIB = $(inhouse_ia64_tools_chatr_LIB)

## A target to support building everything under this subdirectory.  This
## should refer to anything defined above.  There should be two targets; one
## with and one without the trailing slash.  The trailing slash will allow
## the use of tab completion.
inhouse/ia64_tools inhouse/ia64_tools/: include inhouse/ia64_tools/chatr

## A target to support cleaning everything under this subdirectory.
inhouse/ia64_tools/clean: include/clean inhouse/ia64_tools/chatr/clean
