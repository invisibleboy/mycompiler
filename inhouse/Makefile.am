## inhouse/Makefile.am
## This file is (ultimately) included by the top level Makefile.am, so all
## files need to be specified relative to the top level directory.

include $(srcdir)/inhouse/dumpelfsection/Makefile.am
include $(srcdir)/inhouse/edgcpfe/Makefile.am
if USE_CXX

endif
if HOST_PLATFORM_IA64LIN_GCC
include $(srcdir)/inhouse/ia64_tools/Makefile.am
endif

if USE_CXX
if HOST_PLATFORM_IA64LIN_GCC
inhouse_PRG = \
	$(inhouse_dumpelfsection_PRG) $(inhouse_edgcpfe_PRG) \
	 $(inhouse_ia64_tools_PRG)
inhouse_LIB = \
	$(inhouse_dumpelfsection_LIB) $(inhouse_edgcpfe_LIB) \
	 $(inhouse_ia64_tools_LIB)

## A target to support building everything under this subdirectory.  This
## should refer to anything defined above.  There should be two targets; one
## with and one without the trailing slash.  The trailing slash will allow
## the use of tab completion.
inhouse inhouse/: \
	include inhouse/dumpelfsection inhouse/edgcpfe inhouse/edglibC \
	inhouse/ia64_tools

## A target to support cleaning everything under this subdirectory.
inhouse/clean: \
	include/clean inhouse/dumpelfsection/clean inhouse/edgcpfe/clean \
	 inhouse/ia64_tools/clean
else
## The blank lines between each variable definition is significant.  It allows
## us to remove the edgcpfe line and still have a valid definition.
inhouse_PRG = $(inhouse_edgcpfe_PRG) 
inhouse_LIB = $(inhouse_edgcpfe_LIB) 

## A target to support building everything under this subdirectory.  This
## should refer to anything defined above.  There should be two targets; one
## with and one without the trailing slash.  The trailing slash will allow
## the use of tab completion.
inhouse inhouse/: include inhouse/edgcpfe inhouse/edglibC

## A target to support cleaning everything under this subdirectory.
inhouse/clean: inhouse/edgcpfe/clean 
endif
else
if HOST_PLATFORM_IA64LIN_GCC
inhouse_PRG = \
	$(inhouse_dumpelfsection_PRG) $(inhouse_edgcpfe_PRG) \
	$(inhouse_ia64_tools_PRG)
inhouse_LIB = \
	$(inhouse_dumpelfsection_LIB) $(inhouse_edgcpfe_LIB) \
	$(inhouse_ia64_tools_LIB)

## A target to support building everything under this subdirectory.  This
## should refer to anything defined above.  There should be two targets; one
## with and one without the trailing slash.  The trailing slash will allow
## the use of tab completion.
inhouse inhouse/: \
	include inhouse/dumpelfsection inhouse/edgcpfe inhouse/ia64_tools

## A target to support cleaning everything under this subdirectory.
inhouse/clean: \
	include/clean inhouse/dumpelfsection/clean inhouse/edgcpfe/clean \
	inhouse/ia64_tools/clean
else
## The blank lines between each variable definition is significant.  It allows
## us to remove the edgcpfe line and still have a valid definition.
inhouse_PRG = $(inhouse_edgcpfe_PRG)
inhouse_LIB = $(inhouse_edgcpfe_LIB)

## A target to support building everything under this subdirectory.  This
## should refer to anything defined above.  There should be two targets; one
## with and one without the trailing slash.  The trailing slash will allow
## the use of tab completion.
inhouse inhouse/: include inhouse/edgcpfe

## A target to support cleaning everything under this subdirectory.
inhouse/clean: inhouse/edgcpfe/clean
endif
endif
