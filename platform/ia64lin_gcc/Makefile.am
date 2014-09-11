## platform/ia64lin_gcc/Makefile.am
## This file is (ultimately) included by the top level Makefile.am, so all
## files need to be specified relative to the top level directory.

include $(srcdir)/platform/ia64lin_gcc/include/Makefile.am
include $(srcdir)/platform/ia64lin_gcc/IPA_lib/Makefile.am

if TARGET_PLATFORM_IA64LIN_GCC
if HOST_PLATFORM_IA64LIN_GCC
platform_ia64lin_gcc_LIB = platform/ia64lin_gcc/libimpact_lemul.a \
			   platform/ia64lin_gcc/libimpact_pprof.a \
			   platform/ia64lin_gcc/libintrinsics.a

platform_ia64lin_gccdir = $(prefix)/platform/ia64lin_gcc

# Define SOURCES variables for these libraries so that automake doesn't try
# to implicitly find sources.
nodist_platform_ia64lin_gcc_libimpact_lemul_a_SOURCES = 
nodist_platform_ia64lin_gcc_libimpact_pprof_a_SOURCES = 
nodist_platform_ia64lin_gcc_libintrinsics_a_SOURCES = 

# Make sure automake distributes the library sources, since we've completely
# overridden the standard stuff.
platform_ia64lin_gcc_DST = platform/ia64lin_gcc/impact_lemul_lib.c \
			   platform/ia64lin_gcc/impact_pprof_lib.c \
			   platform/ia64lin_gcc/platform_info \
			   platform/ia64lin_gcc/host_layout_info.md

# Use the DATA primitive to install the libraries and objects without any
# special processing by automake.
platform_ia64lin_gcc_DATA = $(platform_ia64lin_gcc_LIB) \
			    platform/ia64lin_gcc/impact_pprof_lib.o \
			    platform/ia64lin_gcc/impact_lemul_lib.o
dist_platform_ia64lin_gcc_DATA = platform/ia64lin_gcc/platform_info \
				 platform/ia64lin_gcc/host_layout_info.md

## Specify the files that are created by this make file so that 'make clean'
## will delete them properly.
platform_ia64lin_gcc_CLN = $(platform_ia64lin_gcc_DATA)

## A target to support building everything under this subdirectory.  This
## should refer to anything defined above.  There should be two targets; one
## with and one without the trailing slash.  The trailing slash will allow
## the use of tab completion.
platform/ia64lin_gcc platform/ia64lin_gcc/: \
	platform/ia64lin_gcc/IPA_lib $(platform_ia64lin_gcc_LIB)

## A target to support cleaning everything under this subdirectory.
platform/ia64lin_gcc/clean: platform/ia64lin_gcc/IPA_lib/clean
	rm -f $(platform/ia64lin_gcc_CLN)
else
platform_ia64lin_gcc_LIB = 
platform_ia64lin_gcc_DST = platform/ia64lin_gcc/impact_lemul_lib.c \
			   platform/ia64lin_gcc/impact_pprof_lib.c \
			   platform/ia64lin_gcc/platform_info \
			   platform/ia64lin_gcc/host_layout_info.md
platform_ia64lin_gcc_CLN = 

## A target to support building everything under this subdirectory.  This
## should refer to anything defined above.  There should be two targets; one
## with and one without the trailing slash.  The trailing slash will allow
## the use of tab completion.
platform/ia64lin_gcc platform/ia64lin_gcc/:

## A target to support cleaning everything under this subdirectory.
platform/ia64lin_gcc/clean:
endif
else
platform_ia64lin_gcc_LIB = 
platform_ia64lin_gcc_DST = platform/ia64lin_gcc/impact_lemul_lib.c \
			   platform/ia64lin_gcc/impact_pprof_lib.c \
			   platform/ia64lin_gcc/platform_info \
			   platform/ia64lin_gcc/host_layout_info.md
platform_ia64lin_gcc_CLN = 

## A target to support building everything under this subdirectory.  This
## should refer to anything defined above.  There should be two targets; one
## with and one without the trailing slash.  The trailing slash will allow
## the use of tab completion.
platform/ia64lin_gcc platform/ia64lin_gcc/:

## A target to support cleaning everything under this subdirectory.
platform/ia64lin_gcc/clean:
endif

## The lemul and pprof libraries have to be compiled with gcc, since they
## use inline assembly.  We have to override the automake stuff to force
## gcc.
platform/ia64lin_gcc/libimpact_pprof.a: platform/ia64lin_gcc/impact_pprof_lib.o
	ar r platform/ia64lin_gcc/libimpact_pprof.a \
		platform/ia64lin_gcc/impact_pprof_lib.o

platform/ia64lin_gcc/impact_pprof_lib.o: \
		$(abs_srcdir)/platform/ia64lin_gcc/impact_pprof_lib.c
	mkdir -p $(top_builddir)/platform/ia64lin_gcc
	gcc -c -I$(top_builddir)/include \
		$(abs_srcdir)/platform/ia64lin_gcc/impact_pprof_lib.c \
		-o platform/ia64lin_gcc/impact_pprof_lib.o

platform/ia64lin_gcc/libimpact_lemul.a: platform/ia64lin_gcc/impact_lemul_lib.o
	ar r platform/ia64lin_gcc/libimpact_lemul.a \
		platform/ia64lin_gcc/impact_lemul_lib.o

platform/ia64lin_gcc/impact_lemul_lib.o: \
		$(abs_srcdir)/platform/ia64lin_gcc/impact_lemul_lib.c
	mkdir -p $(top_builddir)/platform/ia64lin_gcc
	gcc -c -I$(top_builddir)/include \
		$(abs_srcdir)/platform/ia64lin_gcc/impact_lemul_lib.c \
		-o platform/ia64lin_gcc/impact_lemul_lib.o

platform_ia64lin_gcc_libintrinsics_a_SOURCES = \
	intrinsic.lib/intrinsic.c

platform_ia64lin_gcc_libintrinsics_a_CFLAGS = -Iintrinsic.lib $(AM_CFLAGS)

