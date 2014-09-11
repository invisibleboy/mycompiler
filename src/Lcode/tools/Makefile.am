## src/Lcode/tools/Makefile.am
## This file is (ultimately) included by the top level Makefile.am, so all
## files need to be specified relative to the top level directory.

include $(srcdir)/src/Lcode/tools/Lbuild_prototype_info/Makefile.am
include $(srcdir)/src/Lcode/tools/Lemulate/Makefile.am
include $(srcdir)/src/Lcode/tools/Ldot/Makefile.am


include $(srcdir)/src/Lcode/tools/Lstatic/Makefile.am



include $(srcdir)/src/Lcode/tools/Lsplit/Makefile.am

include $(srcdir)/src/Lcode/tools/Lgp_rel/Makefile.am
include $(srcdir)/src/Lcode/tools/Linduct/Makefile.am



include $(srcdir)/src/Lcode/tools/Lsplit_cbs/Makefile.am

src_Lcode_tools_PRG = \
	$(src_Lcode_tools_Lbuild_prototype_info_PRG) \
	$(src_Lcode_tools_Lemulate_PRG)  \
	$(src_Lcode_tools_Ldot_PRG)  \
	 $(src_Lcode_tools_Lstatic_PRG) \
	 $(src_Lcode_tools_Lsplit_PRG) \
	 $(src_Lcode_tools_Lgp_rel_PRG) \
	$(src_Lcode_tools_Linduct_PRG)  \
	$(src_Lcode_tools_Lsplit_cbs_PRG)
src_Lcode_tools_LIB = \
	$(src_Lcode_tools_Lbuild_prototype_info_LIB) \
	$(src_Lcode_tools_Lemulate_LIB)  \
	$(src_Lcode_tools_Ldot_LIB)  \
	 $(src_Lcode_tools_Lstatic_LIB) \
	 $(src_Lcode_tools_Lsplit_LIB) \
	 $(src_Lcode_tools_Lgp_rel_LIB) \
	$(src_Lcode_tools_Linduct_LIB)  \
	$(src_Lcode_tools_Lsplit_cbs_LIB)
src_Lcode_tools_DST = \
	$(src_Lcode_tools_Lbuild_prototype_info_DST) \
	$(src_Lcode_tools_Lemulate_DST)  \
	$(src_Lcode_tools_Ldot_DST)  \
	 $(src_Lcode_tools_Lstatic_DST) \
	 $(src_Lcode_tools_Lsplit_DST) \
	 $(src_Lcode_tools_Lgp_rel_DST) \
	$(src_Lcode_tools_Linduct_DST)  \
	$(src_Lcode_tools_Lsplit_cbs_DST)
noinst_src_Lcode_tools_INC = \
	noinst_src_Lcode_tools_Lbuild_prototype_info_INC \
	noinst_src_Lcode_tools_Lemulate_INC
	noinst_src_Lcode_tools_Ldot_INC


## A target to support building everything under this subdirectory.  This
## should refer to anything defined above.  There should be two targets; one
## with and one without the trailing slash.  The trailing slash will allow
## the use of tab completion.
src/Lcode/tools src/Lcode/tools/: \
	include src/Lcode/tools/Lbuild_prototype_info \
	src/Lcode/tools/Lemulate src/Lcode/tools/Lmarkdoall \
	src/Lcode/tools/Lpretty src/Lcode/tools/Lstatic \
	src/Lcode/tools/Ldiff_trace src/Lcode/tools/Lprint_call_trace \
	src/Lcode/tools/Lis_binary src/Lcode/tools/Lsplit \
	src/Lcode/tools/Lvgraph src/Lcode/tools/Lgp_rel \
	src/Lcode/tools/Linduct src/Lcode/tools/Ldump \
	src/Lcode/tools/Lliberty src/Lcode/tools/Ldot \
	src/Lcode/tools/Lsplit_cbs

## A target to support cleaning everything under this subdirectory.
src/Lcode/tools/clean: \
	include/clean src/Lcode/tools/Lbuild_prototype_info/clean \
	 src/Lcode/tools/Lemulate/clean \
	 src/Lcode/tools/Ldot/clean \
	src/Lcode/tools/Lstatic/clean  \
	 src/Lcode/tools/Lsplit/clean \
	 src/Lcode/tools/Lgp_rel/clean \
	src/Lcode/tools/Linduct/clean  \
	src/Lcode/tools/Lsplit_cbs/clean

## A target to build all documentation under this subdirectory.
src/Lcode/tools/doc: \
	src/Lcode/tools/Lbuild_prototype_info/doc \
	 src/Lcode/tools/Lemulate/doc \
	 src/Lcode/tools/Ldot/doc \
	src/Lcode/tools/Lstatic/doc  \
	src/Lcode/tools/Lsplit/doc  \
	src/Lcode/tools/Lgp_rel/doc src/Lcode/tools/Linduct/doc \
	 src/Lcode/tools/Lsplit_cbs/doc




