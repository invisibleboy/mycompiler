## src/library/Makefile.am
## This file is (ultimately) included by the top level Makefile.am, so all
## files need to be specified relative to the top level directory.

## List the files generated in this directory.  Binaries go under _PRG,
## libraries under _LIB.  Higher level Makefiles use these variables
## unconditionally, so both _PRG and _LIB need to be defined.
src_library_PRG = 
src_library_LIB = src/library/libimpact.a
src_library_DST = src/library/doxygen_config.doc src/library/doxygen_config.tag

## A target to support building everything under this subdirectory.  This
## should refer to anything defined above.  There should be two targets; one
## with and one without the trailing slash.  The trailing slash will allow
## the use of tab completion.
src/library src/library/: include $(src_library_LIB)

## A target to support cleaning everything under this subdirectory.
src/library/clean:
	rm -f $(top_builddir)/src/library/alloc_new/*.$(OBJEXT)
	rm -f $(top_builddir)/src/library/args/*.$(OBJEXT)
	rm -f $(top_builddir)/src/library/attr_mngr/*.$(OBJEXT)
	rm -f $(top_builddir)/src/library/bdd/*.$(OBJEXT)
	rm -f $(top_builddir)/src/library/block_sparse_array/*.$(OBJEXT)
	rm -f $(top_builddir)/src/library/dynamic_symbol/*.$(OBJEXT)
	rm -f $(top_builddir)/src/library/func_list/*.$(OBJEXT)
	rm -f $(top_builddir)/src/library/hash/*.$(OBJEXT)
	rm -f $(top_builddir)/src/library/heap/*.$(OBJEXT)
	rm -f $(top_builddir)/src/library/histogram/*.$(OBJEXT)
	rm -f $(top_builddir)/src/library/libc/*.$(OBJEXT)
	rm -f $(top_builddir)/src/library/libflow/*.$(OBJEXT)
	rm -f $(top_builddir)/src/library/libmd/*.$(OBJEXT)
	rm -f $(top_builddir)/src/library/libparms/*.$(OBJEXT)
	rm -f $(top_builddir)/src/library/libs/*.$(OBJEXT)
	rm -f $(top_builddir)/src/library/list/*.$(OBJEXT)
	rm -f $(top_builddir)/src/library/mfile/*.$(OBJEXT)
	rm -f $(top_builddir)/src/library/name/*.$(OBJEXT)
	rm -f $(top_builddir)/src/library/new/*.$(OBJEXT)
	rm -f $(top_builddir)/src/library/sort/*.$(OBJEXT)
	rm -f $(top_builddir)/src/library/stack/*.$(OBJEXT)
	rm -f $(top_builddir)/src/library/symbol/*.$(OBJEXT)
	rm -f $(top_builddir)/src/library/types/*.$(OBJEXT)
	rm -rf $(srcdir)/src/library/html
	rm -rf $(srcdir)/src/library/latex
	rm -f $(srcdir)/src/library/libimpact.tag
	rm -f $(src_library_LIB)

## List all source and header files under _SOURCES.
src_library_libimpact_a_SOURCES = \
	src/library/alloc_new/l_alloc_new.c \
	src/library/alloc_new/l_alloc_new.h src/library/args/execute.c \
	src/library/args/execute.h src/library/args/file.c \
	src/library/args/file.h src/library/args/line.c \
	src/library/args/line.h src/library/attr_mngr/attr_mngr.c \
	src/library/attr_mngr/attr_mngr.h src/library/bdd/i_bdd_interface.c \
	src/library/bdd/i_bdd_interface.h \
	src/library/block_sparse_array/block_sparse_array.c \
	src/library/block_sparse_array/block_sparse_array.h \
	src/library/dynamic_symbol/char_array_symbol.c \
	src/library/dynamic_symbol/char_array_symbol.h \
	src/library/dynamic_symbol/dynamic_symbol.h \
	src/library/dynamic_symbol/int_array_symbol.c \
	src/library/dynamic_symbol/int_array_symbol.h \
	src/library/dynamic_symbol/int_symbol.c \
	src/library/dynamic_symbol/int_symbol.h \
	src/library/dynamic_symbol/string_symbol.c \
	src/library/dynamic_symbol/string_symbol.h \
	src/library/func_list/func_list.c src/library/func_list/func_list.h \
	src/library/hash/hash.c src/library/hash/hash.h \
	src/library/heap/heap.c src/library/heap/heap.h \
	src/library/histogram/l_histogram.c \
	src/library/histogram/l_histogram.h src/library/libc/c_arg.c \
	src/library/libc/c_arg.h src/library/libc/c_basic.c \
	src/library/libc/c_basic.h src/library/libc/c_list.c \
	src/library/libc/c_list.h src/library/libc/c_symbol2.c \
	src/library/libc/c_symbol2.h src/library/libc/c_symbol.c \
	src/library/libc/c_symbol.h src/library/libc/FIX_libc.h \
	src/library/libflow/dataflow.c src/library/libflow/dataflow.h \
	src/library/libflow/graph.c src/library/libflow/graph.h \
	src/library/libflow/io.c src/library/libflow/io.h \
	src/library/libflow/set.c src/library/libflow/set.h \
	src/library/libflow/trace.c src/library/libflow/trace.h \
	src/library/libmd/md.c src/library/libmd/md.h \
	src/library/libparms/l_parms.c src/library/libparms/l_parms.h \
	src/library/libs/s_recorder.c src/library/libs/s_recorder.h \
	src/library/list/FIX_list.h src/library/list/list.c \
	src/library/list/list.h src/library/list/llist.c \
	src/library/list/llist.h src/library/mfile/mbuf.c \
	src/library/mfile/mbuf.h src/library/mfile/mfile.c \
	src/library/mfile/mfile.h src/library/name/name.c \
	src/library/name/name.h src/library/new/i_error.c \
	src/library/new/i_error.h src/library/new/i_global.h \
	src/library/new/i_graph.c src/library/new/i_graph.h \
	src/library/new/i_hash.c src/library/new/i_hash.h \
	src/library/new/i_hashl.c src/library/new/i_hashl.h \
	src/library/new/i_io.c src/library/new/i_io.h \
	src/library/new/i_list.c src/library/new/i_list.h \
	src/library/new/i_set.h src/library/sort/merge_sort.c \
	src/library/sort/merge_sort.h src/library/sort/select_sort.c \
	src/library/sort/select_sort.h src/library/stack/stack.c \
	src/library/stack/stack.h src/library/symbol/symbol.c \
	src/library/symbol/symbol.h src/library/types/i_types.c \
	src/library/types/i_types.h

## A target to build the tag file for this subdirectory's documentation.
## The tag file contains information on every documented symbol so that
## documentation in other directories can be linked to documentation in
## this one.
$(srcdir)/src/library/libimpact.tag: $(srcdir)/src/library/doxygen_config.tag \
		$(src_library_libimpact_a_SOURCES)
	cd $(srcdir)/src/library && doxygen doxygen_config.tag

## A target to build all documentation under this subdirectory.  This
## builds the human readable documentation and links it to documentation
## in other directories.
src/library/doc $(srcdir)/src/library/html $(srcdir)/src/library/latex: \
		$(srcdir)/src/library/doxygen_config.doc \
		$(srcdir)/src/library/libimpact.tag
	cd $(srcdir)/src/library && doxygen doxygen_config.doc

## List all include files that should be symlinked into include under 
## noinst_..._INC.  These include files are not installed when the user
## does 'make install'.  This is appropriate for most include files.  If
## you have headers that need to be installed, see src/library/Makefile.am
## for an example.
noinst_src_library_alloc_new_INC = src/library/alloc_new/l_alloc_new.h

## The class for the include files.  This is the directory under the include
## directory where the symlinks will be created.  This needs to have the same
## variable name as the _INC variable, only with a _CLASS suffix.
noinst_src_library_alloc_new_CLASS = library

noinst_src_library_args_INC = \
	src/library/args/execute.h src/library/args/file.h \
	src/library/args/line.h
noinst_src_library_args_CLASS = library

noinst_src_library_attr_mngr_INC = src/library/attr_mngr/attr_mngr.h
noinst_src_library_attr_mngr_CLASS = library

noinst_src_library_bdd_INC = src/library/bdd/i_bdd_interface.h
noinst_src_library_bdd_CLASS = library

noinst_src_library_block_sparse_array_INC = \
	src/library/block_sparse_array/block_sparse_array.h
noinst_src_library_block_sparse_array_CLASS = library

noinst_src_library_dynamic_symbol_INC = \
	src/library/dynamic_symbol/char_array_symbol.h \
	src/library/dynamic_symbol/dynamic_symbol.h \
	src/library/dynamic_symbol/int_array_symbol.h \
	src/library/dynamic_symbol/int_symbol.h \
	src/library/dynamic_symbol/string_symbol.h
noinst_src_library_dynamic_symbol_CLASS = library

noinst_src_library_func_list_INC = src/library/func_list/func_list.h
noinst_src_library_func_list_CLASS = library

noinst_src_library_hash_INC = src/library/hash/hash.h
noinst_src_library_hash_CLASS = library

noinst_src_library_heap_INC = src/library/heap/heap.h
noinst_src_library_heap_CLASS = library

noinst_src_library_histogram_INC = src/library/histogram/l_histogram.h
noinst_src_library_histogram_CLASS = library

noinst_src_library_libc_INC = \
	src/library/libc/c_arg.h src/library/libc/c_basic.h \
	src/library/libc/c_list.h src/library/libc/c_symbol2.h \
	src/library/libc/c_symbol.h src/library/libc/FIX_libc.h
noinst_src_library_libc_CLASS = library

noinst_src_library_libflow_INC = \
	src/library/libflow/dataflow.h src/library/libflow/graph.h \
	src/library/libflow/io.h src/library/libflow/set.h \
	src/library/libflow/trace.h
noinst_src_library_libflow_CLASS = library

## If an include file needs to be installed when the user does 'make install',
## specify it without the noinst_ prefix.  See also below.
src_library_libmd_INC = src/library/libmd/md.h
src_library_libmd_CLASS = library

noinst_src_library_libparms_INC = src/library/libparms/l_parms.h
noinst_src_library_libparms_CLASS = library

noinst_src_library_libs_INC = src/library/libs/s_recorder.h
noinst_src_library_libs_CLASS = library

noinst_src_library_list_INC = \
	src/library/list/FIX_list.h src/library/list/list.h \
	src/library/list/llist.h
noinst_src_library_list_CLASS = library

noinst_src_library_mfile_INC = \
	src/library/mfile/mbuf.h src/library/mfile/mfile.h
noinst_src_library_mfile_CLASS = library

noinst_src_library_name_INC = src/library/name/name.h
noinst_src_library_name_CLASS = library

noinst_src_library_new_INC = \
	src/library/new/i_error.h src/library/new/i_global.h \
	src/library/new/i_graph.h src/library/new/i_hash.h \
	src/library/new/i_hashl.h src/library/new/i_io.h \
	src/library/new/i_list.h src/library/new/i_set.h
noinst_src_library_new_CLASS = library

noinst_src_library_sort_INC = \
	src/library/sort/merge_sort.h src/library/sort/select_sort.h
noinst_src_library_sort_CLASS = library

noinst_src_library_stack_INC = src/library/stack/stack.h
noinst_src_library_stack_CLASS = library

noinst_src_library_symbol_INC = src/library/symbol/symbol.h
noinst_src_library_symbol_CLASS = library

noinst_src_library_types_INC = src/library/types/i_types.h
noinst_src_library_types_CLASS = library

## If an include file needs to be installed when the user does 'make install',
## you need to define one of automake's variables to tell it where to install.
## ...dir tells automake where to install the files listed under ..._HEADERS.
## The nodist_ prefix on _HEADERS tells automake not to include this symlink
## in the distribution.  The symlink is generated when the user builds, so it
## doesn't need to be distributed.
src_librarydir = $(prefix)/include/library
nodist_src_library_HEADERS = $(src_library_libmd_INC)

src_library_INC = src_library_libmd_INC
noinst_src_library_INC = \
	noinst_src_library_alloc_new_INC noinst_src_library_args_INC \
	noinst_src_library_attr_mngr_INC noinst_src_library_bdd_INC \
	noinst_src_library_block_sparse_array_INC \
	noinst_src_library_dynamic_symbol_INC \
	noinst_src_library_func_list_INC noinst_src_library_hash_INC \
	noinst_src_library_heap_INC noinst_src_library_histogram_INC \
	noinst_src_library_libc_INC noinst_src_library_libflow_INC \
	noinst_src_library_libparms_INC noinst_src_library_libs_INC \
	noinst_src_library_list_INC noinst_src_library_mfile_INC \
	noinst_src_library_name_INC noinst_src_library_new_INC \
	noinst_src_library_sort_INC noinst_src_library_stack_INC \
	noinst_src_library_symbol_INC noinst_src_library_types_INC

## List any dependencies other than libraries under _DEPENDENDIES.  Defining
## this variable disables automatic dependency detection for this target, so
## you will need to relist dependencies that appear in _LDADD.
## src_library_libimpact_a_DEPENDENCIES = 

## List any special compiler flags under _CFLAGS.
## src_library_libimpact_a_CFLAGS = 
