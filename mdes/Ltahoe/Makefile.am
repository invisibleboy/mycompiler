## mdes/Ltahoe/Makefile.am
## This file is (ultimately) included by the top level Makefile.am, so all
## files need to be specified relative to the top level directory.

mdes_Ltahoedir = $(prefix)/mdes/Ltahoe

mdes_Ltahoe_DATA = \
	mdes/Ltahoe/merced.lmdes2 mdes/Ltahoe/mckinley.lmdes2 \
	mdes/Ltahoe/mckinley_szu.lmdes2
dist_mdes_Ltahoe_DATA = \
	mdes/Ltahoe/merced.hmdes2 mdes/Ltahoe/mckinley.hmdes2 \
	mdes/Ltahoe/mckinley_szu.hmdes2 mdes/Ltahoe/mckinley_addon.hmdes2 \
	mdes/Ltahoe/merced_knobsfile.knb mdes/Ltahoe/mckinley_knobsfile.knb

## Specify the files that are created by this make file so that 'make clean'
## will delete them properly.
mdes_Ltahoe_CLN = $(mdes_Ltahoe_DATA)

## A target to support building everything under this subdirectory.  This
## should refer to anything defined above.  There should be two targets; one
## with and one without the trailing slash.  The trailing slash will allow
## the use of tab completion.
mdes/Ltahoe mdes/Ltahoe/: $(mdes_Ltahoe_DATA)

## A target to support cleaning everything under this subdirectory.
mdes/Ltahoe/clean:
	rm -f $(mdes_Ltahoe_CLN)

mdes/Ltahoe/merced.lmdes2: $(abs_srcdir)/mdes/Ltahoe/merced.hmdes2 \
		$(top_builddir)/mdes/structure/structure_IMPACT.ho \
		bin/md_preprocessor bin/md_compiler bin/lmdes2_customizer \
		bin/convert_hmdes
	mkdir -p mdes/Ltahoe
	hmdesc $(abs_srcdir)/mdes/Ltahoe/merced.hmdes2 \
		-o mdes/Ltahoe/merced.lmdes2

mdes/Ltahoe/mckinley.lmdes2: $(abs_srcdir)/mdes/Ltahoe/mckinley.hmdes2 \
		$(top_builddir)/mdes/structure/structure_IMPACT.ho \
		bin/md_preprocessor bin/md_compiler bin/lmdes2_customizer \
		bin/convert_hmdes
	mkdir -p mdes/Ltahoe
	hmdesc $(abs_srcdir)/mdes/Ltahoe/mckinley.hmdes2 \
		-o mdes/Ltahoe/mckinley.lmdes2

mdes/Ltahoe/mckinley_szu.lmdes2: \
		$(abs_srcdir)/mdes/Ltahoe/mckinley_szu.hmdes2 \
		$(top_builddir)/mdes/structure/structure_IMPACT.ho \
		bin/md_preprocessor bin/md_compiler bin/lmdes2_customizer \
		bin/convert_hmdes
	mkdir -p mdes/Ltahoe
	hmdesc $(abs_srcdir)/mdes/Ltahoe/mckinley_szu.hmdes2 \
		-o mdes/Ltahoe/mckinley_szu.lmdes2
