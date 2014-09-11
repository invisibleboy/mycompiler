## platform/ia64lin_gcc/IPA_lib/Makefile.am
## This file is (ultimately) included by the top level Makefile.am, so all
## files need to be specified relative to the top level directory.

if TARGET_PLATFORM_IA64LIN_GCC
if HOST_PLATFORM_IA64LIN_GCC

platform_ia64lin_gcc_IPA_libdir = ${prefix}/platform/ia64lin_gcc/IPA_lib

if USE_CXX
platform_ia64lin_gcc_IPA_lib_DATA = \
	platform/ia64lin_gcc/IPA_lib/__impact_lib.pst \
	platform/ia64lin_gcc/IPA_lib/__impact_lib2.pst \
	platform/ia64lin_gcc/IPA_lib/__impact_intrinsic.pst
else
platform_ia64lin_gcc_IPA_lib_DATA = \
	platform/ia64lin_gcc/IPA_lib/__impact_lib.pst \
	platform/ia64lin_gcc/IPA_lib/__impact_intrinsic.pst
endif

## Specify the files that are created by this make file so that 'make clean'
## will delete them properly.
platform_ia64lin_gcc_IPA_lib_CLN = $(platform_ia64lin_gcc_IPA_lib_DATA)

## A target to support building everything under this subdirectory.  This
## should refer to anything defined above.  There should be two targets; one
## with and one without the trailing slash.  The trailing slash will allow
## the use of tab completion.
platform/ia64lin_gcc/IPA_lib platform/ia64lin_gcc/IPA_lib/: \
	$(platform_ia64lin_gcc_IPA_lib_DATA)

## A target to support cleaning everything under this subdirectory.
platform/ia64lin_gcc/IPA_lib/clean:
	$(foreach file, $(platform_ia64lin_gcc_IPA_lib_CLN), rm -f $(file);)

platform/ia64lin_gcc/IPA_lib/__impact_lib.pst: \
		$(top_builddir)/bin/impact-edgcpfe \
		$(top_builddir)/bin/Psymtab \
		$(top_builddir)/scripts/read_platform_info \
		$(abs_srcdir)/platform/lib.c
	mkdir -p platform/ia64lin_gcc/IPA_lib
	ln -sf $(abs_srcdir)/platform/lib.c \
		platform/ia64lin_gcc/IPA_lib/__impact_lib.c
	gen_CtoP -nolink -I$${IMPACT_ROOT}/include \
		-I$${IMPACT_ROOT}/config/$${IMPACT_HOST_PLATFORM}_c \
		platform/ia64lin_gcc/IPA_lib/__impact_lib.c

platform/ia64lin_gcc/IPA_lib/__impact_lib2.pst: \
		$(top_builddir)/bin/impact-edgcpfe \
		$(top_builddir)/bin/Psymtab \
		$(top_builddir)/scripts/read_platform_info \
		$(abs_srcdir)/platform/lib2.cc
	mkdir -p platform/ia64lin_gcc/IPA_lib
	ln -sf $(abs_srcdir)/platform/lib2.cc \
		platform/ia64lin_gcc/IPA_lib/__impact_lib2.cc
	gen_CtoP -nolink -I$${IMPACT_ROOT}/include \
		-I$${IMPACT_ROOT}/config/$${IMPACT_HOST_PLATFORM}_c \
		platform/ia64lin_gcc/IPA_lib/__impact_lib2.cc

platform/ia64lin_gcc/IPA_lib/__impact_intrinsic.pst: \
		$(top_builddir)/bin/impact-edgcpfe \
		$(top_builddir)/bin/Psymtab \
		$(top_builddir)/scripts/read_platform_info \
		$(abs_srcdir)/intrinsic.lib/intrinsic.c \
		$(abs_srcdir)/intrinsic.lib/intrinsic.h \
		$(abs_srcdir)/intrinsic.lib/typedef.h
	mkdir -p platform/ia64lin_gcc/IPA_lib
	ln -sf $(abs_srcdir)/intrinsic.lib/intrinsic.c \
		platform/ia64lin_gcc/IPA_lib/__impact_intrinsic.c
	ln -sf $(abs_srcdir)/intrinsic.lib/intrinsic.h \
		platform/ia64lin_gcc/IPA_lib/intrinsic.h
	ln -sf $(abs_srcdir)/intrinsic.lib/typedef.h \
		platform/ia64lin_gcc/IPA_lib/typedef.h
	gen_CtoP -nolink -I$${IMPACT_ROOT}/include \
		-I$${IMPACT_ROOT}/config/$${IMPACT_HOST_PLATFORM}_c \
		platform/ia64lin_gcc/IPA_lib/__impact_intrinsic.c

endif
endif
