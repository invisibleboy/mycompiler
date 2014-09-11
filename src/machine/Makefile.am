## src/machine/Makefile.am
## This file is (ultimately) included by the top level Makefile.am, so all
## files need to be specified relative to the top level directory.

include $(srcdir)/src/machine/Lmdes/Makefile.am
include $(srcdir)/src/machine/hmdes/Makefile.am
include $(srcdir)/src/machine/Mspec/Makefile.am
include $(srcdir)/src/machine/Lmdes_build/Makefile.am
include $(srcdir)/src/machine/convert_hmdes/Makefile.am
include $(srcdir)/src/machine/lmdes2_customizer/Makefile.am
include $(srcdir)/src/machine/md_compiler/Makefile.am
include $(srcdir)/src/machine/md_preprocessor/Makefile.am

## A target to support building everything under this subdirectory.  This
## should refer to anything defined above.  There should be two targets; one
## with and one without the trailing slash.  The trailing slash will allow
## the use of tab completion.
src/machine src/machine/: \
	include src/machine/Lmdes src/machine/hmdes src/machine/Mspec \
	src/machine/Lmdes_build src/machine/convert_hmdes \
	src/machine/lmdes2_customizer src/machine/md_compiler \
	src/machine/md_preprocessor

## A target to support cleaning everything under this subdirectory.
src/machine/clean: \
	include/clean src/machine/Lmdes/clean src/machine/hmdes/clean \
	src/machine/Mspec/clean src/machine/Lmdes_build/clean \
	src/machine/convert_hmdes/clean src/machine/lmdes2_customizer/clean \
	src/machine/md_compiler/clean src/machine/md_preprocessor/clean

## A target to build all documentation under this subdirectory.
src/machine/doc: \
	src/machine/Lmdes/doc src/machine/hmdes/doc src/machine/Mspec/doc \
	src/machine/Lmdes_build/doc src/machine/convert_hmdes/doc \
	src/machine/lmdes2_customizer/doc src/machine/md_compiler/doc \
	src/machine/md_preprocessor/doc

src_machine_PRG = \
	$(src_machine_Lmdes_PRG) $(src_machine_hmdes_PRG) \
	$(src_machine_Mspec_PRG) $(src_machine_Lmdes_build_PRG) \
	$(src_machine_convert_hmdes_PRG) $(src_machine_lmdes2_customizer_PRG) \
	$(src_machine_md_compiler_PRG) $(src_machine_md_preprocessor_PRG)
src_machine_LIB = \
	$(src_machine_Lmdes_LIB) $(src_machine_hmdes_LIB) \
	$(src_machine_Mspec_LIB) $(src_machine_Lmdes_build_LIB) \
	$(src_machine_convert_hmdes_LIB) $(src_machine_lmdes2_customizer_LIB) \
	$(src_machine_md_compiler_LIB) $(src_machine_md_preprocessor_LIB)
noinst_src_machine_INC = \
	noinst_src_machine_Lmdes_INC noinst_src_machine_hmdes_INC \
	noinst_src_machine_Mspec_INC noinst_src_machine_md_compiler_INC \
	noinst_src_machine_md_preprocessor_INC
src_machine_DST = \
	$(src_machine_Lmdes_DST) $(src_machine_hmdes_DST) \
	$(src_machine_Mspec_DST) $(src_machine_Lmdes_build_DST) \
	$(src_machine_convert_hmdes_DST) $(src_machine_lmdes2_customizer_DST) \
	$(src_machine_md_compiler_DST) $(src_machine_md_preprocessor_DST)

