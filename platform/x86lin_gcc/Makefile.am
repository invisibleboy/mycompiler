## platform/x86lin_gcc/Makefile.am
## This file is (ultimately) included by the top level Makefile.am, so all
## files need to be specified relative to the top level directory.

include $(srcdir)/platform/x86lin_gcc/include/Makefile.am
include $(srcdir)/platform/x86lin_gcc/IPA_lib/Makefile.am

if TARGET_PLATFORM_X86LIN_GCC
if HOST_PLATFORM_X86LIN_GCC
platform_x86lin_gcc_LIB = platform/x86lin_gcc/libimpact_lemul.a \
			  platform/x86lin_gcc/libimpact_pprof.a \
			  platform/x86lin_gcc/libintrinsics.a

platform_x86lin_gccdir = $(prefix)/platform/x86lin_gcc

# Define SOURCES variables for these libraries so that automake doesn't try
# to implicitly find sources.
nodist_platform_x86lin_gcc_libimpact_lemul_a_SOURCES = 
nodist_platform_x86lin_gcc_libimpact_pprof_a_SOURCES = 
nodist_platform_x86lin_gcc_libintrinsics_a_SOURCES = 

# Make sure automake distributes the library sources, since we've completely
# overridden the standard stuff.
platform_x86lin_gcc_DST = platform/x86lin_gcc/impact_lemul_lib.c \
			  platform/x86lin_gcc/impact_pprof_lib.c \
			  platform/x86lin_gcc/platform_info \
			  platform/x86lin_gcc/host_layout_info.md

# Use the DATA primitive to install the libraries and objects without any
# special processing by automake.
platform_x86lin_gcc_DATA = $(platform_x86lin_gcc_LIB) \
			   platform/x86lin_gcc/impact_pprof_lib.o \
			   platform/x86lin_gcc/impact_lemul_lib.o
dist_platform_x86lin_gcc_DATA = platform/x86lin_gcc/platform_info \
				platform/x86lin_gcc/host_layout_info.md

## Specify the files that are created by this make file so that 'make clean'
## will delete them properly.
platform_x86lin_gcc_CLN = $(platform_x86lin_gcc_DATA)

## A target to support building everything under this subdirectory.  This
## should refer to anything defined above.  There should be two targets; one
## with and one without the trailing slash.  The trailing slash will allow
## the use of tab completion.
platform/x86lin_gcc platform/x86lin_gcc/: \
	platform/x86lin_gcc/IPA_lib $(platform_x86lin_gcc_LIB)

## A target to support cleaning everything under this subdirectory.
platform/x86lin_gcc/clean: platform/x86lin_gcc/IPA_lib/clean
	rm -f $(platform_x86lin_gcc_CLN)
else
platform_x86lin_gcc_LIB = 
platform_x86lin_gcc_DST = platform/x86lin_gcc/impact_lemul_lib.c \
			  platform/x86lin_gcc/impact_pprof_lib.c \
			  platform/x86lin_gcc/platform_info \
			  platform/x86lin_gcc/host_layout_info.md
platform_x86lin_gcc_CLN = 

## A target to support building everything under this subdirectory.  This
## should refer to anything defined above.  There should be two targets; one
## with and one without the trailing slash.  The trailing slash will allow
## the use of tab completion.
platform/x86lin_gcc platform/x86lin_gcc/:

## A target to support cleaning everything under this subdirectory.
platform/x86lin_gcc/clean:
endif
else
platform_x86lin_gcc_LIB = 
platform_x86lin_gcc_DST = platform/x86lin_gcc/impact_lemul_lib.c \
			  platform/x86lin_gcc/impact_pprof_lib.c \
			  platform/x86lin_gcc/platform_info \
			  platform/x86lin_gcc/host_layout_info.md
platform_x86lin_gcc_CLN = 

## A target to support building everything under this subdirectory.  This
## should refer to anything defined above.  There should be two targets; one
## with and one without the trailing slash.  The trailing slash will allow
## the use of tab completion.
platform/x86lin_gcc platform/x86lin_gcc/:

## A target to support cleaning everything under this subdirectory.
platform/x86lin_gcc/clean:
endif

platform_x86lin_gcc_libimpact_lemul_a_SOURCES = \
	platform/x86lin_gcc/impact_lemul_lib.c

platform_x86lin_gcc_libimpact_pprof_a_SOURCES = \
	platform/x86lin_gcc/impact_pprof_lib.c

platform_x86lin_gcc_libintrinsics_a_SOURCES = \
	intrinsic.lib/intrinsic.c

platform_x86lin_gcc_libintrinsics_a_CFLAGS = -Iintrinsic.lib $(AM_CFLAGS)

