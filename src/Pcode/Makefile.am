## src/Pcode/Makefile.am
## This file is (ultimately) included by the top level Makefile.am, so all
## files need to be specified relative to the top level directory.

include $(srcdir)/src/Pcode/Pannotate/Makefile.am
include $(srcdir)/src/Pcode/Pcode/Makefile.am
include $(srcdir)/src/Pcode/Pflatten/Makefile.am
include $(srcdir)/src/Pcode/Pinline/Makefile.am
include $(srcdir)/src/Pcode/Pipa/Makefile.am
include $(srcdir)/src/Pcode/Plib_CF/Makefile.am
include $(srcdir)/src/Pcode/Plib_DD/Makefile.am
include $(srcdir)/src/Pcode/Plib_probe/Makefile.am
include $(srcdir)/src/Pcode/Plib_SS/Makefile.am
include $(srcdir)/src/Pcode/Plib_SS_new/Makefile.am
include $(srcdir)/src/Pcode/Plink/Makefile.am
include $(srcdir)/src/Pcode/Pmerge_iter/Makefile.am
include $(srcdir)/src/Pcode/Pmerge_prof/Makefile.am
include $(srcdir)/src/Pcode/Pomega/Makefile.am
include $(srcdir)/src/Pcode/Protate/Makefile.am
include $(srcdir)/src/Pcode/Psymtab/Makefile.am
include $(srcdir)/src/Pcode/PtoC/Makefile.am
include $(srcdir)/src/Pcode/PtoL/Makefile.am
include $(srcdir)/src/Pcode/PtoNM/Makefile.am

src_Pcode_PRG = \
	$(src_Pcode_Pannotate_PRG) $(src_Pcode_Pcode_PRG) \
	$(src_Pcode_Pflatten_PRG) $(src_Pcode_Pinline_PRG) \
	$(src_Pcode_Pipa_PRG) $(src_Pcode_Plib_CF_PRG) \
	$(src_Pcode_Plib_DD_PRG) $(src_Pcode_Plib_probe_PRG) \
	$(src_Pcode_Plib_SS_PRG) $(src_Pcode_Plink_PRG) \
	$(src_Pcode_Pmerge_iter_PRG) $(src_Pcode_Pmerge_prof_PRG) \
	$(src_Pcode_Pomega_PRG) $(src_Pcode_Protate_PRG) \
	$(src_Pcode_Psymtab_PRG) $(src_Pcode_PtoC_PRG) $(src_Pcode_PtoL_PRG) \
	$(src_Pcode_PtoNM_PRG) $(src_Pcode_Plib_SS_new_PRG)
src_Pcode_LIB = \
	$(src_Pcode_Pannotate_LIB) $(src_Pcode_Pcode_LIB) \
	$(src_Pcode_Pflatten_LIB) $(src_Pcode_Pinline_LIB) \
	$(src_Pcode_Pipa_LIB) $(src_Pcode_Plib_CF_LIB) \
	$(src_Pcode_Plib_DD_LIB) $(src_Pcode_Plib_probe_LIB) \
	$(src_Pcode_Plib_SS_LIB) $(src_Pcode_Plink_LIB) \
	$(src_Pcode_Pmerge_iter_LIB) $(src_Pcode_Pmerge_prof_LIB) \
	$(src_Pcode_Pomega_LIB) $(src_Pcode_Protate_LIB) \
	$(src_Pcode_Psymtab_LIB) $(src_Pcode_PtoC_LIB) $(src_Pcode_PtoL_LIB) \
	$(src_Pcode_PtoNM_LIB) $(src_Pcode_Plib_SS_new_LIB)
src_Pcode_DST = \
	$(src_Pcode_Pannotate_DST) $(src_Pcode_Pcode_DST) \
	$(src_Pcode_Pflatten_DST) $(src_Pcode_Pinline_DST) \
	$(src_Pcode_Pipa_DST) $(src_Pcode_Plib_CF_DST) \
	$(src_Pcode_Plib_DD_DST) $(src_Pcode_Plib_probe_DST) \
	$(src_Pcode_Plib_SS_DST) $(src_Pcode_Plink_DST) \
	$(src_Pcode_Pmerge_iter_DST) $(src_Pcode_Pmerge_prof_DST) \
	$(src_Pcode_Pomega_DST) $(src_Pcode_Protate_DST) \
	$(src_Pcode_Psymtab_DST) $(src_Pcode_PtoC_DST) $(src_Pcode_PtoL_DST) \
	$(src_Pcode_PtoNM_DST) $(src_Pcode_Plib_SS_new_DST)
noinst_src_Pcode_INC = \
	noinst_src_Pcode_Pcode_INC noinst_src_Pcode_Pflatten_new_INC \
	noinst_src_Pcode_Plib_CF_INC noinst_src_Pcode_Plib_DD_INC \
	noinst_src_Pcode_Plib_probe_INC noinst_src_Pcode_Plib_SS_INC \
	noinst_src_Pcode_Plib_SS_new_INC

## A target to support building everything under this subdirectory.  This
## should refer to anything defined above.  There should be two targets; one
## with and one without the trailing slash.  The trailing slash will allow
## the use of tab completion.
src/Pcode src/Pcode/: \
	include src/Pcode/Pannotate src/Pcode/Pcode src/Pcode/Pflatten \
	src/Pcode/Pinline src/Pcode/Pipa src/Pcode/Plib_CF src/Pcode/Plib_DD \
	src/Pcode/Plib_probe src/Pcode/Plib_SS src/Pcode/Plink \
	src/Pcode/Pmerge_iter src/Pcode/Pmerge_prof src/Pcode/Pomega \
	src/Pcode/Protate src/Pcode/Psymtab src/Pcode/PtoC src/Pcode/PtoL \
	src/Pcode/PtoNM src/Pcode/Plib_SS_new

## A target to support cleaning everything under this subdirectory.
src/Pcode/clean: \
	include/clean src/Pcode/Pannotate/clean src/Pcode/Pcode/clean \
	src/Pcode/Pflatten/clean src/Pcode/Pinline/clean src/Pcode/Pipa/clean \
	src/Pcode/Plib_CF/clean src/Pcode/Plib_DD/clean \
	src/Pcode/Plib_probe/clean src/Pcode/Plib_SS/clean \
	src/Pcode/Plink/clean src/Pcode/Pmerge_iter/clean \
	src/Pcode/Pmerge_prof/clean src/Pcode/Pomega/clean \
	src/Pcode/Protate/clean src/Pcode/Psymtab/clean src/Pcode/PtoC/clean \
	src/Pcode/PtoL/clean src/Pcode/PtoNM src/Pcode/Plib_SS_new

## A target to build all documentation under this subdirectory.
src/Pcode/doc: \
	src/Pcode/Pannotate/doc src/Pcode/Pcode/doc src/Pcode/Pflatten/doc \
	src/Pcode/Pinline/doc src/Pcode/Pipa/doc src/Pcode/Plib_CF/doc \
	src/Pcode/Plib_DD/doc src/Pcode/Plib_probe/doc src/Pcode/Plib_SS/doc \
	src/Pcode/Plink/doc src/Pcode/Pmerge_iter/doc \
	src/Pcode/Pmerge_prof/doc src/Pcode/Pomega/doc src/Pcode/Protate/doc \
	src/Pcode/Psymtab/doc src/Pcode/PtoC/doc src/Pcode/PtoL/doc \
	src/Pcode/PtoNM/doc src/Pcode/Plib_SS_new/doc
