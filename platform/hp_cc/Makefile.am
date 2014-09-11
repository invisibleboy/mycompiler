## platform/hp_cc/Makefile.am
## This file is (ultimately) included by the top level Makefile.am, so all
## files need to be specified relative to the top level directory.

include $(srcdir)/platform/hp_cc/include/Makefile.am

if TARGET_PLATFORM_HP_CC
if HOST_PLATFORM_HP_CC
platform_hp_cc_LIB = platform/hp_cc/libimpact_lemul.a \
		     platform/hp_cc/libimpact_pprof.a \
		     platform/hp_cc/libintrinsics.a

platform_hp_ccdir = $(prefix)/platform/hp_cc

# Define SOURCES variables for these libraries so that automake doesn't try
# to implicitly find sources.
nodist_platform_hp_cc_libimpact_lemul_a_SOURCES = 
nodist_platform_hp_cc_libimpact_pprof_a_SOURCES = 
nodist_platform_hp_cc_libintrinsics_a_SOURCES = 

# Make sure automake distributes the library sources, since we've completely
# overridden the standard stuff.
platform_hp_cc_DST = platform/hp_cc/impact_lemul_lib.c \
		     platform/hp_cc/impact_pprof_lib.c \
		     platform/hp_cc/platform_info

platform_hp_cc_SCRIPTS = platform/hp_cc/print_cc_version

# Use the DATA primitive to install the libraries and objects without any 
# special processing by automake.
platform_hp_cc_DATA = $(platform_hp_cc_LIB) \
		      platform/hp_cc/impact_pprof_lib.o \
		      platform/hp_cc/impact_lemul_lib.o
dist_platform_hp_cc_DATA = platform/hp_cc/platform_info

platform_hp_cc_CLN = $(platform_hp_cc_DATA)

## A target to support building everything under this subdirectory.  This
## should refer to anything defined above.  There should be two targets; one
## with and one without the trailing slash.  The trailing slash will allow
## the use of tab completion.
platform/hp_cc platform/hp_cc/: $(platform_hp_cc_LIB)

## A target to support cleaning everything under this subdirectory.
platform/hp_cc/clean:
	rm -f $(platform_hp_cc_CLN)
else
platform_hp_cc_LIB = 
platform_hp_cc_DST = platform/hp_cc/impact_lemul_lib.c \
		     platform/hp_cc/impact_pprof_lib.c \
		     platform/hp_cc/platform_info \
		     platform/hp_cc/print_cc_version
platform_hp_cc_CLN = 

## A target to support building everything under this subdirectory.  This
## should refer to anything defined above.  There should be two targets; one
## with and one without the trailing slash.  The trailing slash will allow
## the use of tab completion.
platform/hp_cc platform/hp_cc/:

## A target to support cleaning everything under this subdirectory.
platform/hp_cc/clean:
endif
else
platform_hp_cc_LIB = 
platform_hp_cc_DST = platform/hp_cc/impact_lemul_lib.c \
		     platform/hp_cc/impact_pprof_lib.c \
		     platform/hp_cc/platform_info \
		     platform/hp_cc/print_cc_version
platform_hp_cc_CLN = 

## A target to support building everything under this subdirectory.  This
## should refer to anything defined above.  There should be two targets; one
## with and one without the trailing slash.  The trailing slash will allow
## the use of tab completion.
platform/hp_cc platform/hp_cc/:

## A target to support cleaning everything under this subdirectory.
platform/hp_cc/clean:
endif

# Override the automake rules so that we get the objects to have the correct
# name.
platform/hp_cc/libimpact_pprof.a: platform/hp_cc/impact_pprof_lib.o
	ar r platform/hp_cc/libimpact_pprof.a platform/hp_cc/impact_pprof_lib.o

platform/hp_cc/impact_pprof_lib.o: \
		$(abs_srcdir)/platform/hp_cc/impact_pprof_lib.c
	mkdir -p $(top_builddir)/platform/hp_cc
	cc -Ae +DA1.1 -c -g -I$(top_builddir)/include \
		$(abs_srcdir)/platform/hp_cc/impact_pprof_lib.c \
		-o platform/hp_cc/impact_pprof_lib.o

platform/hp_cc/libimpact_lemul.a: platform/hp_cc/impact_lemul_lib.o
	ar r platform/hp_cc/libimpact_lemul.a platform/hp_cc/impact_lemul_lib.o

platform/hp_cc/impact_lemul_lib.o: \
		$(abs_srcdir)/platform/hp_cc/impact_lemul_lib.c
	mkdir -p $(top_builddir)/platform/hp_cc
	cc -Ae +DA1.1 -c -g -I$(top_builddir)/include \
		$(abs_srcdir)/platform/hp_cc/impact_lemul_lib.c \
		-o platform/hp_cc/impact_lemul_lib.o

platform_hp_cc_libintrinsics_a_SOURCES = \
	intrinsic.lib/intrinsic.c

platform_hp_cc_libintrinsics_a_CFLAGS = -Ae +DA1.1 -Iintrinsic.lib $(AM_CFLAGS)


