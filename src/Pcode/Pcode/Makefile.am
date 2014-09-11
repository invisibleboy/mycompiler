## src/Pcode/Pcode/Makefile.am
## This file is (ultimately) included by the top level Makefile.am, so all
## files need to be specified relative to the top level directory.

## List the files generated in this directory.  Binaries go under _PRG,
## libraries under _LIB.  Higher level Makefiles use these variables
## unconditionally, so both _PRG and _LIB need to be defined.
src_Pcode_Pcode_PRG = 
src_Pcode_Pcode_LIB = src/Pcode/Pcode/libpcode.a
src_Pcode_Pcode_DST = src/Pcode/Pcode/doxygen_config.doc \
	src/Pcode/Pcode/doxygen_config.tag

## A target to support building everything under this subdirectory.  This
## should refer to anything defined above.  There should be two targets; one
## with and one without the trailing slash.  The trailing slash will allow
## the use of tab completion.
src/Pcode/Pcode src/Pcode/Pcode/: include $(src_Pcode_Pcode_LIB)

## A target to support cleaning everything under this subdirectory.
src/Pcode/Pcode/clean:
	rm -f $(top_builddir)/src/Pcode/Pcode/*.$(OBJEXT)
	rm -f $(top_builddir)/src/Pcode/Pcode/lex.c
	rm -f $(top_builddir)/src/Pcode/Pcode/parse.c
	rm -f $(top_builddir)/src/Pcode/Pcode/parse.h
	rm -f $(top_builddir)/src/Pcode/Pcode/parse_symtab.c
	rm -f $(top_builddir)/src/Pcode/Pcode/parse_symtab.h
	rm -f $(src_Pcode_Pcode_LIB)

## List all source and header files under _SOURCES.  
src_Pcode_Pcode_libpcode_a_SOURCES = \
	src/Pcode/Pcode/parse.y src/Pcode/Pcode/parse_symtab.y \
	src/Pcode/Pcode/lex.l \
	src/Pcode/Pcode/cast.c src/Pcode/Pcode/cast.h \
	src/Pcode/Pcode/cast_symtab.c src/Pcode/Pcode/cast_symtab.h \
	src/Pcode/Pcode/extension.c src/Pcode/Pcode/extension.h \
	src/Pcode/Pcode/handler.c src/Pcode/Pcode/help.c \
	src/Pcode/Pcode/impact_global.h src/Pcode/Pcode/io_util.c \
	src/Pcode/Pcode/io_util.h src/Pcode/Pcode/lex.h \
	src/Pcode/Pcode/lex_symtab.c src/Pcode/Pcode/lex_symtab.h \
	src/Pcode/Pcode/parloop.c src/Pcode/Pcode/parloop.h \
	src/Pcode/Pcode/parms.c src/Pcode/Pcode/parms.h \
	src/Pcode/Pcode/parse_prefix.h \
	src/Pcode/Pcode/parse_prefix_symtab.h\
	src/Pcode/Pcode/pcode.c src/Pcode/Pcode/pcode.h \
	src/Pcode/Pcode/perror.c src/Pcode/Pcode/perror.h \
	src/Pcode/Pcode/p_main.c src/Pcode/Pcode/query.c \
	src/Pcode/Pcode/query.h src/Pcode/Pcode/query_symtab.c \
	src/Pcode/Pcode/query_symtab.h src/Pcode/Pcode/read.c \
	src/Pcode/Pcode/read.h src/Pcode/Pcode/read_symtab.c \
	src/Pcode/Pcode/read_symtab.h src/Pcode/Pcode/reduce.c \
	src/Pcode/Pcode/reduce.h src/Pcode/Pcode/reduce_symtab.c \
	src/Pcode/Pcode/reduce_symtab.h src/Pcode/Pcode/struct.c \
	src/Pcode/Pcode/struct.h src/Pcode/Pcode/struct_symtab.c \
	src/Pcode/Pcode/struct_symtab.h src/Pcode/Pcode/symtab.c \
	src/Pcode/Pcode/symtab.h src/Pcode/Pcode/symtab_i.c \
	src/Pcode/Pcode/symtab_i.h src/Pcode/Pcode/util.c \
	src/Pcode/Pcode/util.h src/Pcode/Pcode/write.c \
	src/Pcode/Pcode/write.h src/Pcode/Pcode/write_symtab.c \
	src/Pcode/Pcode/write_symtab.h

## A target to build the tag file for this subdirectory's documentation.
## The tag file contains information on every documented symbol so that
## documentation in other directories can be linked to documentation in
## this one.
$(srcdir)/src/Pcode/Pcode/libpcode.tag: \
		$(srcdir)/src/Pcode/Pcode/doxygen_config.tag \
		$(src_Pcode_Pcode_libpcode_a_SOURCES)
	cd $(srcdir)/src/Pcode/Pcode && doxygen doxygen_config.tag

## A target to build all documentation under this subdirectory.  This
## builds the human readable documentation and links it to documentation
## in other directories.
src/Pcode/Pcode/doc $(srcdir)/src/Pcode/Pcode/html \
	$(srcdir)/src/Pcode/Pcode/latex: \
		$(srcdir)/src/Pcode/Pcode/doxygen_config.doc \
		$(srcdir)/src/Pcode/Pcode/libpcode.tag \
		$(srcdir)/src/library/libimpact.tag
	cd $(srcdir)/src/Pcode/Pcode && doxygen doxygen_config.doc

## List all include files that should be symlinked into include under 
## noinst_..._INC.  These include files are not installed when the user
## does 'make install'.  This is appropriate for most include files.  If
## you have headers that need to be installed, see src/library/Makefile.am
## for an example.
noinst_src_Pcode_Pcode_INC = \
	src/Pcode/Pcode/cast.h src/Pcode/Pcode/cast_symtab.h \
	src/Pcode/Pcode/extension.h src/Pcode/Pcode/impact_global.h \
	src/Pcode/Pcode/io_util.h src/Pcode/Pcode/lex.h \
	src/Pcode/Pcode/lex_symtab.h src/Pcode/Pcode/parloop.h \
	src/Pcode/Pcode/parms.h src/Pcode/Pcode/parse_prefix.h \
	src/Pcode/Pcode/parse_prefix_symtab.h src/Pcode/Pcode/pcode.h \
	src/Pcode/Pcode/perror.h src/Pcode/Pcode/query.h \
	src/Pcode/Pcode/query_symtab.h src/Pcode/Pcode/read.h \
	src/Pcode/Pcode/read_symtab.h src/Pcode/Pcode/reduce.h \
	src/Pcode/Pcode/reduce_symtab.h src/Pcode/Pcode/struct.h \
	src/Pcode/Pcode/symtab.h src/Pcode/Pcode/symtab_i.h \
	src/Pcode/Pcode/struct_symtab.h src/Pcode/Pcode/util.h \
	src/Pcode/Pcode/write.h src/Pcode/Pcode/write_symtab.h

## The class for the include files.  This is the directory under the include
## directory where the symlinks will be created.  This needs to have the same
## variable name as the _INC variable, only with a _CLASS suffix.
noinst_src_Pcode_Pcode_CLASS = Pcode

## List any dependencies other than libraries under _DEPENDENDIES.  Defining
## this variable disables automatic dependency detection for this target, so
## you will need to relist dependencies that appear in _LDADD.
## src_Pcode_Pcode_libpcode_a_DEPENDENCIES = 

## List any special compiler flags under _CFLAGS.
src_Pcode_Pcode_libpcode_a_CFLAGS = \
	-I$(top_builddir)/src/Pcode/Pcode $(AM_CFLAGS)

