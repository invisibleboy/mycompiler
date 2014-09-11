## src/Lcode/codegen/Lschedule/Makefile.am
## This file is (ultimately) included by the top level Makefile.am, so all
## files need to be specified relative to the top level directory.

## List the files generated in this directory.  Binaries go under _PRG,
## libraries under _LIB.  Higher level Makefiles use these variables
## unconditionally, so both _PRG and _LIB need to be defined.
src_Lcode_codegen_Lschedule_PRG = 
src_Lcode_codegen_Lschedule_LIB = src/Lcode/codegen/Lschedule/libsched.a
src_Lcode_codegen_Lschedule_DST = \
	src/Lcode/codegen/Lschedule/doxygen_config.doc \
	src/Lcode/codegen/Lschedule/doxygen_config.tag

## A target to support building everything under this subdirectory.  This
## should refer to anything defined above.  There should be two targets; one
## with and one without the trailing slash.  The trailing slash will allow
## the use of tab completion.
src/Lcode/codegen/Lschedule src/Lcode/codegen/Lschedule/: \
	include $(src_Lcode_codegen_Lschedule_LIB)

## A target to support cleaning everything under this subdirectory.
src/Lcode/codegen/Lschedule/clean:
	rm -f $(top_builddir)/src/Lcode/codegen/Lschedule/*.$(OBJEXT)
	rm -rf $(srcdir)/src/Lcode/codegen/Lschedule/html
	rm -rf $(srcdir)/src/Lcode/codegen/Lschedule/latex
	rm -f $(srcdir)/src/Lcode/codegen/Lschedule/libsched.tag
	rm -f $(src_Lcode_codegen_Lschedule_LIB)

## List all source and header files under _SOURCES.  
src_Lcode_codegen_Lschedule_libsched_a_SOURCES = \
	src/Lcode/codegen/Lschedule/l_delay_slots.c \
	src/Lcode/codegen/Lschedule/l_dependence.c \
	src/Lcode/codegen/Lschedule/l_dependence.h \
	src/Lcode/codegen/Lschedule/l_mcb.c \
	src/Lcode/codegen/Lschedule/l_mcb.h \
	src/Lcode/codegen/Lschedule/l_queue.c \
	src/Lcode/codegen/Lschedule/l_regpres.c \
	src/Lcode/codegen/Lschedule/l_ru_interface.c \
	src/Lcode/codegen/Lschedule/l_ru_interface.h \
	src/Lcode/codegen/Lschedule/l_schedule.c \
	src/Lcode/codegen/Lschedule/l_schedule.h \
	src/Lcode/codegen/Lschedule/l_statistic.c \
	src/Lcode/codegen/Lschedule/l_wbs.c \
	src/Lcode/codegen/Lschedule/old_sm.h \
	src/Lcode/codegen/Lschedule/old_sm_rmap.c \
	src/Lcode/codegen/Lschedule/RU_manager.c \
	src/Lcode/codegen/Lschedule/RU_manager.h

## A target to build the tag file for this subdirectory's documentation.
## The tag file contains information on every documented symbol so that
## documentation in other directories can be linked to documentation in
## this one.
$(srcdir)/src/Lcode/codegen/Lschedule/libsched.tag: \
		$(srcdir)/src/Lcode/codegen/Lschedule/doxygen_config.tag \
		$(src_Lcode_codegen_Lschedule_libsched_a_SOURCES)
	cd $(srcdir)/src/Lcode/codegen/Lschedule && doxygen doxygen_config.tag

## A target to build all documentation under this subdirectory.  This
## builds the human readable documentation and links it to documentation
## in other directories.
src/Lcode/codegen/Lschedule/doc $(srcdir)/src/Lcode/codegen/Lschedule/html \
	$(srcdir)/src/Lcode/codegen/Lschedule/latex: \
		$(srcdir)/src/Lcode/codegen/Lschedule/doxygen_config.doc \
		$(srcdir)/src/Lcode/codegen/Lschedule/libsched.tag \
		$(srcdir)/src/Lcode/Lcode/liblcode.tag \
		$(srcdir)/src/machine/Mspec/libmspec.tag \
		$(srcdir)/src/library/libimpact.tag
	cd $(srcdir)/src/Lcode/codegen/Lschedule && doxygen doxygen_config.doc

## List all include files that should be symlinked into include under 
## noinst_..._INC.  These include files are not installed when the user
## does 'make install'.  This is appropriate for most include files.  If
## you have headers that need to be installed, see src/library/Makefile.am
## for an example.
noinst_src_Lcode_codegen_Lschedule_INC = \
	src/Lcode/codegen/Lschedule/l_dependence.h \
	src/Lcode/codegen/Lschedule/l_mcb.h \
	src/Lcode/codegen/Lschedule/l_ru_interface.h \
	src/Lcode/codegen/Lschedule/l_schedule.h \
	src/Lcode/codegen/Lschedule/RU_manager.h

## The class for the include files.  This is the directory under the include
## directory where the symlinks will be created.  This needs to have the same
## variable name as the _INC variable, only with a _CLASS suffix.
noinst_src_Lcode_codegen_Lschedule_CLASS = Lcode

## List any dependencies other than libraries under _DEPENDENDIES.  Defining
## this variable disables automatic dependency detection for this target, so
## you will need to relist dependencies that appear in _LDADD.
## src_Lcode_codegen_Lschedule_libsched_a_DEPENDENCIES = 

## List any special compiler flags under _CFLAGS.
## src_Lcode_codegen_Lschedule_libsched_a_CFLAGS = 

