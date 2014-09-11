## src/Makefile.am
## This file is (ultimately) included by the top level Makefile.am, so all
## files need to be specified relative to the top level directory.

include $(srcdir)/src/bdd/Makefile.am
include $(srcdir)/src/library/Makefile.am
include $(srcdir)/src/machine/Makefile.am
include $(srcdir)/src/Lcode/Makefile.am
include $(srcdir)/src/Pcode/Makefile.am

src_PRG = \
	$(src_bdd_PRG) $(src_library_PRG) $(src_machine_PRG) $(src_Lcode_PRG) \
	$(src_Pcode_PRG)
src_LIB = \
	$(src_bdd_LIB) $(src_library_LIB) $(src_machine_LIB) $(src_Lcode_LIB) \
	$(src_Pcode_LIB)
src_BLT = $(src_Lcode_BLT)
src_DST = \
	$(src_bdd_DST) $(src_library_DST) $(src_machine_DST) $(src_Lcode_DST) \
	$(src_Pcode_DST) src/Doxyfile
src_INC = $(src_library_INC) $(src_Lcode_INC)
noinst_src_INC = \
	$(noinst_src_bdd_INC) $(noinst_src_library_INC) \
	$(noinst_src_machine_INC) $(noinst_src_Lcode_INC) \
	$(noinst_src_Pcode_INC)

## A target to support building everything under this subdirectory.  This
## should refer to anything defined above.  There should be two targets; one
## with and one without the trailing slash.  The trailing slash will allow
## the use of tab completion.
src src/: include src/bdd src/library src/machine src/Lcode src/Pcode

## A target to support cleaning everything under this subdirectory.
src/clean: include/clean src/bdd/clean src/library/clean src/machine/clean \
	src/Lcode/clean src/Pcode/clean

## A target to build all documentation under this subdirectory.
src/doc: src/bdd/doc src/library/doc src/machine/doc src/Lcode/doc \
	src/Pcode/doc
