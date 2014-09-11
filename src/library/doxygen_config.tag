# src/library/doxygen_config.tag
@INCLUDE = ../Doxyfile
PROJECT_NAME = libimpact
INPUT = alloc_new args attr_mngr bdd block_sparse_array dynamic_symbol \
        func_list hash heap histogram libc libflow libmd libparms libs list \
        mfile new sort stack symbol types
GENERATE_TAGFILE = libimpact.tag
GENERATE_HTML = NO
GENERATE_LATEX = NO
QUIET = YES
WARNINGS = NO
