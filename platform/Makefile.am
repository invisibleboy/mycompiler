## platform/Makefile.am
## This file is (ultimately) included by the top level Makefile.am, so all
## files need to be specified relative to the top level directory.

include $(srcdir)/platform/hp_cc/Makefile.am

include $(srcdir)/platform/ia64lin_gcc/Makefile.am
include $(srcdir)/platform/x86lin_gcc/Makefile.am

platform_LIB = $(platform_hp_cc_LIB)  \
	       $(platform_ia64lin_gcc_LIB) $(platform_x86lin_gcc_LIB)
platform_DST = platform/lib.c $(platform_hp_cc_DST)  \
	       $(platform_ia64lin_gcc_DST) $(platform_x86lin_gcc_DST)
platform_CLN = $(platform_hp_cc_CLN)  \
	       $(platform_ia64lin_gcc_CLN) $(platform/x86lin_gcc_CLN)

## A target to support building everything under this subdirectory.  This
## should refer to anything defined above.  There should be two targets; one
## with and one without the trailing slash.  The trailing slash will allow
## the use of tab completion.
platform platform/: platform/hp_cc platform/hp_gcc platform/ia64lin_gcc \
	platform/x86lin_gcc

## A target to support cleaning everything under this subdirectory.
platform/clean: platform/hp_cc/clean  \
	platform/ia64lin_gcc/clean platform/x86lin_gcc/clean

