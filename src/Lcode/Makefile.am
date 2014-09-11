## src/Lcode/Makefile.am
## This file is (ultimately) included by the top level Makefile.am, so all
## files need to be specified relative to the top level directory.

include $(srcdir)/src/Lcode/Lcode/Makefile.am
include $(srcdir)/src/Lcode/codegen/Makefile.am
include $(srcdir)/src/Lcode/opti/Makefile.am
include $(srcdir)/src/Lcode/performance/Makefile.am
include $(srcdir)/src/Lcode/tools/Makefile.am
include $(srcdir)/src/Lcode/sched/Makefile.am

src_Lcode_PRG = \
	$(src_Lcode_Lcode_PRG) $(src_Lcode_codegen_PRG) $(src_Lcode_opti_PRG) \
	$(src_Lcode_performance_PRG) $(src_Lcode_tools_PRG) \
	$(src_Lcode_sched_PRG)
src_Lcode_LIB = \
	$(src_Lcode_Lcode_LIB) $(src_Lcode_codegen_LIB) $(src_Lcode_opti_LIB) \
	$(src_Lcode_performance_LIB) $(src_Lcode_tools_LIB) \
	$(src_Lcode_sched_LIB)
src_Lcode_BLT = $(src_Lcode_codegen_BLT)
src_Lcode_DST = \
	$(src_Lcode_Lcode_DST) $(src_Lcode_codegen_DST) $(src_Lcode_opti_DST) \
	$(src_Lcode_performance_DST) $(src_Lcode_tools_DST) \
	$(src_Lcode_sched_DST)
src_Lcode_INC = $(src_Lcode_performance_INC)
noinst_src_Lcode_INC = \
	noinst_src_Lcode_Lcode_INC $(noinst_src_Lcode_codegen_INC) \
	$(noinst_src_Lcode_opti_INC) $(noinst_src_Lcode_performance_INC) \
	$(noinst_src_Lcode_tools_INC) $(noinst_src_Lcode_sched_INC)

## A target to support building everything under this subdirectory.  This
## should refer to anything defined above.  There should be two targets; one
## with and one without the trailing slash.  The trailing slash will allow
## the use of tab completion.
src/Lcode src/Lcode/: \
	include src/Lcode/Lcode src/Lcode/codegen src/Lcode/opti \
	src/Lcode/performance src/Lcode/tools src/Lcode/sched

## A target to support cleaning everything under this subdirectory.
src/Lcode/clean: \
	include/clean src/Lcode/Lcode/clean src/Lcode/codegen/clean \
	src/Lcode/opti/clean src/Lcode/performance/clean \
	src/Lcode/tools/clean src/Lcode/sched/clean

## A target to build all documentation under this subdirectory.
src/Lcode/doc: \
	src/Lcode/Lcode/doc src/Lcode/codegen/doc src/Lcode/opti/doc \
	src/Lcode/performance/doc src/Lcode/tools/doc src/Lcode/sched/doc
