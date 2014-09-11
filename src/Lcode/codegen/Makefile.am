## src/Lcode/codegen/Makefile.am
## This file is (ultimately) included by the top level Makefile.am, so all
## files need to be specified relative to the top level directory.

include $(srcdir)/src/Lcode/codegen/Lschedule/Makefile.am
include $(srcdir)/src/Lcode/codegen/Regalloc/Makefile.am
include $(srcdir)/src/Lcode/codegen/Mopti/Makefile.am
include $(srcdir)/src/Lcode/codegen/Lsoftpipe/Makefile.am
include $(srcdir)/src/Lcode/codegen/Mopti-ia64/Makefile.am
include $(srcdir)/src/Lcode/codegen/Ltahoe/Makefile.am
include $(srcdir)/src/Lcode/codegen/Lhpl_pd/Makefile.am

include $(srcdir)/src/Lcode/codegen/Limpact/Makefile.am

src_Lcode_codegen_PRG = \
	$(src_Lcode_codegen_Lschedule_PRG) $(src_Lcode_codegen_Regalloc_PRG) \
	$(src_Lcode_codegen_Mopti_PRG) $(src_Lcode_codegen_Lsoftpipe_PRG) \
	$(src_Lcode_codegen_Mopti_ia64_PRG) $(src_Lcode_codegen_Ltahoe_PRG) \
	 $(src_Lcode_codegen_Limpact_PRG) $(src_Lcode_codegen_Lhpl_pd_PRG)
src_Lcode_codegen_LIB = \
	$(src_Lcode_codegen_Lschedule_LIB) $(src_Lcode_codegen_Regalloc_LIB) \
	$(src_Lcode_codegen_Mopti_LIB) $(src_Lcode_codegen_Lsoftpipe_LIB) \
	$(src_Lcode_codegen_Mopti_ia64_LIB) $(src_Lcode_codegen_Ltahoe_LIB) \
	 $(src_Lcode_codegen_Limpact_LIB) $(src_Lcode_codegen_Lhpl_pd_LIB)
src_Lcode_codegen_BLT = $(src_Lcode_codegen_Ltahoe_BLT)
src_Lcode_codegen_DST = \
	$(src_Lcode_codegen_Lschedule_DST) $(src_Lcode_codegen_Regalloc_DST) \
	$(src_Lcode_codegen_Mopti_DST) $(src_Lcode_codegen_Lsoftpipe_DST) \
	$(src_Lcode_codegen_Mopti_ia64_DST) $(src_Lcode_codegen_Ltahoe_DST) \
	 $(src_Lcode_codegen_Limpact_DST)
noinst_src_Lcode_codegen_INC = \
	noinst_src_Lcode_codegen_Lschedule_INC \
	noinst_src_Lcode_codegen_Regalloc_INC \
	noinst_src_Lcode_codegen_Mopti_INC \
	noinst_src_Lcode_codegen_Lsoftpipe_INC \
	noinst_src_Lcode_codegen_Mopti_ia64_INC \
	noinst_src_Lcode_codegen_Ltahoe_INC \
	noinst_src_Lcode_codegen_Lhpl_pd_INC \
	noinst_src_Lcode_codegen_Limpact_INC

## A target to support building everything under this subdirectory.  This
## should refer to anything defined above.  There should be two targets; one
## with and one without the trailing slash.  The trailing slash will allow
## the use of tab completion.
src/Lcode/codegen src/Lcode/codegen/: \
	include src/Lcode/codegen/Lschedule src/Lcode/codegen/Regalloc \
	src/Lcode/codegen/Mopti src/Lcode/codegen/Lsoftpipe \
	src/Lcode/codegen/Mopti-ia64 src/Lcode/codegen/Ltahoe \
	src/Lcode/codegen/Lstarcore src/Lcode/codegen/Lhpl_pd src/Lcode/codegen/Limpact 

## A target to support cleaning everything under this subdirectory.
src/Lcode/codegen/clean: \
	include/clean src/Lcode/codegen/Lschedule/clean \
	src/Lcode/codegen/Regalloc/clean src/Lcode/codegen/Mopti/clean \
	src/Lcode/codegen/Lsoftpipe/clean src/Lcode/codegen/Mopti-ia64/clean \
	src/Lcode/codegen/Ltahoe/clean  \
	src/Lcode/codegen/Limpact/clean \
	src/Lcode/codegen/Lhpl_pd/clean

## A target to build all documentation under this subdirectory.
src/Lcode/codegen/doc: \
	src/Lcode/codegen/Lschedule/doc src/Lcode/codegen/Regalloc/doc \
	src/Lcode/codegen/Mopti/doc src/Lcode/codegen/Lsoftpipe/doc \
	src/Lcode/codegen/Mopti-ia64/doc src/Lcode/codegen/Ltahoe/doc \
	 src/Lcode/codegen/Limpact/doc
