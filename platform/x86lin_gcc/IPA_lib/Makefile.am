## platform/x86lin_gcc/IPA_lib/Makefile.am
## This file is (ultimately) included by the top level Makefile.am, so all
## files need to be specified relative to the top level directory.

if TARGET_PLATFORM_X86LIN_GCC
if HOST_PLATFORM_X86LIN_GCC

platform_x86lin_gcc_IPA_libdir = ${prefix}/platform/x86lin_gcc/IPA_lib

platform_x86lin_gcc_IPA_lib_DATA = \
	platform/x86lin_gcc/IPA_lib/__impact_lib.pst \
	platform/x86lin_gcc/IPA_lib/__impact_intrinsic.pst

## Specify the files that are created by this make file so that 'make clean'
## will delete them properly.
platform_x86lin_gcc_IPA_lib_CLN = $(platform_x86lin_gcc_IPA_lib_DATA)

## A target to support building everything under this subdirectory.  This
## should refer to anything defined above.  There should be two targets; one
## with and one without the trailing slash.  The trailing slash will allow
## the use of tab completion.
platform/x86lin_gcc/IPA_lib platform/x86lin_gcc/IPA_lib/: \
	$(platform_x86lin_gcc_IPA_lib_DATA)

## A target to support cleaning everything under this subdirectory.
platform/x86lin_gcc/IPA_lib/clean:
	$(foreach file, $(platform_x86lin_gcc_IPA_lib_CLN), rm -f $(file);)

platform/x86lin_gcc/IPA_lib/__impact_lib.pst: \
		$(top_builddir)/bin/impact-edgcpfe \
		$(top_builddir)/bin/Psymtab \
		$(top_builddir)/scripts/read_platform_info \
		$(abs_srcdir)/platform/lib.c
	mkdir -p platform/x86lin_gcc/IPA_lib
	ln -sf $(abs_srcdir)/platform/lib.c \
		platform/x86lin_gcc/IPA_lib/__impact_lib.c
	gen_CtoP -nolink -I$${IMPACT_ROOT}/include \
		-I$${IMPACT_ROOT}/config/$${IMPACT_HOST_PLATFORM}_c \
		platform/x86lin_gcc/IPA_lib/__impact_lib.c

platform/x86lin_gcc/IPA_lib/__impact_intrinsic.pst: \
		$(top_builddir)/bin/impact-edgcpfe \
		$(top_builddir)/bin/Psymtab \
		$(top_builddir)/scripts/read_platform_info \
		$(abs_srcdir)/intrinsic.lib/intrinsic.c \
		$(abs_srcdir)/intrinsic.lib/intrinsic.h \
		$(abs_srcdir)/intrinsic.lib/typedef.h
	mkdir -p platform/x86lin_gcc/IPA_lib
	ln -sf $(abs_srcdir)/intrinsic.lib/intrinsic.c \
		platform/x86lin_gcc/IPA_lib/__impact_intrinsic.c
	ln -sf $(abs_srcdir)/intrinsic.lib/intrinsic.h \
		platform/x86lin_gcc/IPA_lib/intrinsic.h
	ln -sf $(abs_srcdir)/intrinsic.lib/typedef.h \
		platform/x86lin_gcc/IPA_lib/typedef.h
	gen_CtoP -nolink -I$${IMPACT_ROOT}/include \
		-I$${IMPACT_ROOT}/config/$${IMPACT_HOST_PLATFORM}_c \
		platform/x86lin_gcc/IPA_lib/__impact_intrinsic.c

endif
endif