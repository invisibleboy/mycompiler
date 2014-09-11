## src/Lcode/tools/Lemulate/Makefile.am
## This file is (ultimately) included by the top level Makefile.am, so all
## files need to be specified relative to the top level directory.

## List the files generated in this directory.  Binaries go under _PRG,
## libraries under _LIB.  Higher level Makefiles use these variables
## unconditionally, so both _PRG and _LIB need to be defined.
src_Lcode_tools_Lemulate_PRG = src/Lcode/tools/Lemulate/Lemulate
src_Lcode_tools_Lemulate_LIB = 
src_Lcode_tools_Lemulate_DST = src/Lcode/tools/Lemulate/doxygen_config.doc \
	src/Lcode/tools/Lemulate/doxygen_config.tag

## A target to support building everything under this subdirectory.  This
## should refer to anything defined above.  There should be two targets; one
## with and one without the trailing slash.  The trailing slash will allow
## the use of tab completion.
src/Lcode/tools/Lemulate src/Lcode/tools/Lemulate/: \
	include $(src_Lcode_tools_Lemulate_PRG)

## A target to support cleaning everything under this subdirectory.
src/Lcode/tools/Lemulate/clean:
	rm -f $(top_builddir)/src/Lcode/tools/Lemulate/*.$(OBJEXT)
	rm -rf $(srcdir)/src/Lcode/tools/Lemulate/html
	rm -rf $(srcdir)/src/Lcode/tools/Lemulate/latex
	rm -f $(srcdir)/src/Lcode/tools/Lemulate/Lemulate.tag
	rm -f $(src_Lcode_tools_Lemulate_PRG)

## List all source and header files under _SOURCES.  
src_Lcode_tools_Lemulate_Lemulate_SOURCES = \
	src/Lcode/tools/Lemulate/l_emul.c \
	src/Lcode/tools/Lemulate/l_emul_decl.c \
	src/Lcode/tools/Lemulate/l_emul_decl.h \
	src/Lcode/tools/Lemulate/l_emul_emit_builtin.c \
	src/Lcode/tools/Lemulate/l_emul_emit_builtin.h \
	src/Lcode/tools/Lemulate/l_emul_emit_data.c \
	src/Lcode/tools/Lemulate/l_emul_emit_data.h \
	src/Lcode/tools/Lemulate/l_emul_emit_op.c \
	src/Lcode/tools/Lemulate/l_emul_emit_operand.c \
	src/Lcode/tools/Lemulate/l_emul_emit_operand.h \
	src/Lcode/tools/Lemulate/l_emul_emit_op.h \
	src/Lcode/tools/Lemulate/l_emul.h \
	src/Lcode/tools/Lemulate/l_emul_intrinsic.c \
	src/Lcode/tools/Lemulate/l_emul_intrinsic.h \
	src/Lcode/tools/Lemulate/l_emul_trace.c \
	src/Lcode/tools/Lemulate/l_emul_trace.h \
	src/Lcode/tools/Lemulate/l_emul_util.c \
	src/Lcode/tools/Lemulate/l_emul_util.h

## A target to build the tag file for this subdirectory's documentation.
## The tag file contains information on every documented symbol so that
## documentation in other directories can be linked to documentation in
## this one.
$(srcdir)/src/Lcode/tools/Lemulate/Lemulate.tag: \
		$(srcdir)/src/Lcode/tools/Lemulate/doxygen_config.tag \
		$(src_Lcode_tools_Lemulate_Lemulate_SOURCES)
	cd $(srcdir)/src/Lcode/tools/Lemulate && doxygen doxygen_config.tag

## A target to build all documentation under this subdirectory.  This
## builds the human readable documentation and links it to documentation
## in other directories.
src/Lcode/tools/Lemulate/doc $(srcdir)/src/Lcode/tools/Lemulate/html \
	$(srcdir)/src/Lcode/tools/Lemulate/latex: \
		$(srcdir)/src/Lcode/tools/Lemulate/doxygen_config.doc \
		$(srcdir)/src/Lcode/tools/Lemulate/Lemulate.tag \
		$(srcdir)/src/Lcode/tools/Lbuild_prototype_info/libproto.tag \
		$(srcdir)/src/Lcode/Lcode/liblcode.tag \
		$(srcdir)/src/bdd/libbdd.tag \
		$(srcdir)/src/machine/Mspec/libmspec.tag \
		$(srcdir)/src/library/libimpact.tag \
		$(srcdir)/src/machine/Lmdes/liblmdes.tag
	cd $(srcdir)/src/Lcode/tools/Lemulate && doxygen doxygen_config.doc

## List all include files that should be symlinked into include under 
## noinst_..._INC.  These include files are not installed when the user
## does 'make install'.  This is appropriate for most include files.  If
## you have headers that need to be installed, see src/library/Makefile.am
## for an example.
noinst_src_Lcode_tools_Lemulate_INC = src/Lcode/tools/Lemulate/l_emul.h

## The class for the include files.  This is the directory under the include
## directory where the symlinks will be created.  This needs to have the same
## variable name as the _INC variable, only with a _CLASS suffix.
noinst_src_Lcode_tools_Lemulate_CLASS = Lcode

## List any libraries needed when linking under _LDADD.  If a library is built
## as part of impact, list it as $(top_builddir)/lib/<library> so that it can
## be detected as a dependency automatically.  Automake assumes libraries
## listed as -l<lib> are system libraries and does not detect them as
## dependencies.
src_Lcode_tools_Lemulate_Lemulate_LDADD = -L$(top_builddir)/lib \
					  $(top_builddir)/lib/libproto.a \
					  $(top_builddir)/lib/liblcode.a \
					  $(top_builddir)/lib/libbdd.a \
					  $(top_builddir)/lib/libmspec.a \
					  $(top_builddir)/lib/libimpact.a \
					  $(top_builddir)/lib/liblmdes.a -lm

## List any dependencies other than libraries under _DEPENDENDIES.  Defining
## this variable disables automatic dependency detection for this target, so
## you will need to relist dependencies that appear in _LDADD.
## src_Lcode_tools_Lemulate_Lemulate_DEPENDENCIES = 

## List any special compiler flags under _CFLAGS.
## src_Lcode_tools_Lemulate_Lemulate_CFLAGS = 
