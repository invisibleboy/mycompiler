## src/machine/Mspec/Makefile.am
## This file is (ultimately) included by the top level Makefile.am, so all
## files need to be specified relative to the top level directory.

## List the files generated in this directory.  Binaries go under _PRG,
## libraries under _LIB.  Higher level Makefiles use these variables
## unconditionally, so both _PRG and _LIB need to be defined.
src_machine_Mspec_PRG = 
src_machine_Mspec_LIB = src/machine/Mspec/libmspec.a
src_machine_Mspec_DST = src/machine/Mspec/doxygen_config.doc \
	src/machine/Mspec/doxygen_config.tag

## A target to support building everything under this subdirectory.  This
## should refer to anything defined above.  There should be two targets; one
## with and one without the trailing slash.  The trailing slash will allow
## the use of tab completion.
src/machine/Mspec src/machine/Mspec/: include $(src_machine_Mspec_LIB)

## A target to support cleaning everything under this subdirectory.
src/machine/Mspec/clean:
	rm -f $(top_builddir)/src/machine/Mspec/*.$(OBJEXT)
	rm -rf $(srcdir)/src/machine/Mspec/html
	rm -rf $(srcdir)/src/machine/Mspec/latex
	rm -f $(srcdir)/src/machine/Mspec/libmspec.tag
	rm -f $(src_machine_Mspec_LIB)

## List all source and header files under _SOURCES.  
src_machine_Mspec_libmspec_a_SOURCES = \
	src/machine/Mspec/m_bx86.h src/machine/Mspec/m_hpl_pd.h \
	src/machine/Mspec/m_hppa.h src/machine/Mspec/mi_bx86.c \
	src/machine/Mspec/mi_hpl_pd.c src/machine/Mspec/mi_hppa.c \
	src/machine/Mspec/mi_impact.c src/machine/Mspec/m_impact.h \
	src/machine/Mspec/mi_sh.c src/machine/Mspec/mi_sparc.c \
	src/machine/Mspec/mi_spec.c src/machine/Mspec/mi_starcore.c \
	src/machine/Mspec/mi_tahoe.c src/machine/Mspec/mi_ti.c \
	src/machine/Mspec/mi_x86.c src/machine/Mspec/ml_bx86.c \
	src/machine/Mspec/ml_hpl_pd.c src/machine/Mspec/ml_hppa.c \
	src/machine/Mspec/ml_impact.c src/machine/Mspec/ml_sh.c \
	src/machine/Mspec/ml_sparc.c src/machine/Mspec/ml_spec.c \
	src/machine/Mspec/ml_starcore.c src/machine/Mspec/ml_tahoe.c \
	src/machine/Mspec/ml_ti.c src/machine/Mspec/ml_x86.c \
	src/machine/Mspec/m_sh.h src/machine/Mspec/m_sparc.h \
	src/machine/Mspec/m_spec.h src/machine/Mspec/m_starcore.h \
	src/machine/Mspec/m_tahoe.h src/machine/Mspec/m_ti.h \
        src/machine/Mspec/m_arm.h src/machine/Mspec/ml_arm.c \
	src/machine/Mspec/m_x86.h src/machine/Mspec/m_wims.h \
	src/machine/Mspec/mi_arm.c src/machine/Mspec/ml_wims.c \
	src/machine/Mspec/mi_wims.c

## A target to build the tag file for this subdirectory's documentation.
## The tag file contains information on every documented symbol so that
## documentation in other directories can be linked to documentation in
## this one.
$(srcdir)/src/machine/Mspec/libmspec.tag: \
		$(srcdir)/src/machine/Mspec/doxygen_config.tag \
		$(src_machine_Mspec_libmspec_a_SOURCES)
	cd $(srcdir)/src/machine/Mspec && doxygen doxygen_config.tag

## A target to build all documentation under this subdirectory.  This
## builds the human readable documentation and links it to documentation
## in other directories.
src/machine/Mspec/doc $(srcdir)/src/machine/Mspec/html \
	$(srcdir)/src/machine/Mspec/latex: \
		$(srcdir)/src/machine/Mspec/doxygen_config.doc \
		$(srcdir)/src/machine/Mspec/libmspec.tag \
		$(srcdir)/src/Lcode/Lcode/liblcode.tag \
		$(srcdir)/src/library/libimpact.tag
	cd $(srcdir)/src/machine/Mspec && doxygen doxygen_config.doc

## List all include files that should be symlinked into include under 
## noinst_..._INC.  These include files are not installed when the user
## does 'make install'.  This is appropriate for most include files.  If
## you have headers that need to be installed, see src/library/Makefile.am
## for an example.
noinst_src_machine_Mspec_INC = \
	src/machine/Mspec/m_bx86.h src/machine/Mspec/m_hpl_pd.h \
	src/machine/Mspec/m_hppa.h src/machine/Mspec/m_impact.h \
	src/machine/Mspec/m_sh.h src/machine/Mspec/m_sparc.h \
	src/machine/Mspec/m_spec.h src/machine/Mspec/m_starcore.h \
	src/machine/Mspec/m_tahoe.h src/machine/Mspec/m_ti.h \
	src/machine/Mspec/m_x86.h src/machine/Mspec/m_arm.h \
	src/machine/Mspec/m_wims.h

## The class for the include files.  This is the directory under the include
## directory where the symlinks will be created.  This needs to have the same
## variable name as the _INC variable, only with a _CLASS suffix.
noinst_src_machine_Mspec_CLASS = machine

## List any dependencies other than libraries under _DEPENDENDIES.  Defining
## this variable disables automatic dependency detection for this target, so
## you will need to relist dependencies that appear in _LDADD.
## src_machine_Mspec_libmspec_a_DEPENDENCIES = 

## List any special compiler flags under _CFLAGS.
## src_machine_Mspec_libmspec_a_CFLAGS = 
