## mdes/Limpact/Makefile.am
## This file is (ultimately) included by the top level Makefile.am, so all
## files need to be specified relative to the top level directory.

## .lmdes2 file naming conventions:
##
##   EPIC_8G_2BL.lmdes2
##      ^----------- Processor model (EPIC or SS)
##        ^--------- Issue width (>= 1)
##         ^-------- Percolation model (G or R)
##           ^------ Branches per cycle ( >= 1 && <= issue width)
##             ^---- Branches placement (L or X)
##
##
##   Processor model:
##
##     EPIC -> EPIC processor.  The scheduler explicitly specifies what
##             operations can be issued together (in parallel) and the
##             processor's issue stage does exactly what it is told.
##             This release's EPIC tools requires that branches to be
##             placed last (L) in all EPIC operation packets. (This is
##             not a significant restriction since if generating code
##             for a real EPIC processor, you simply do a post-pass to
##             rearrange the packet's operations into the desired
##             order).
##
##     SS   -> Superscalar processor.  The scheduler explicitly
##             specifies what operations can be issued together (in
##             parallel) and the processor's issue stage *totally
##             ignores* this information and dynamically decides what 
##             to issue together each cycle.  Support for both the
##             branches last (L) and branches anywhere (X) model.
##             Note: Using the branches last model with SS creates an
##             .lmdes2 file identical to the EPIC variation (the
##             scheduled code can be simulated on either processor
##             model).
##
##   Issue width:
##
##     int  -> The maximum number of operations that may be issued per
##             cycle.
##
##   Percolation model:
##
##      G   -> General percolation.  Non-trapping (silent) versions of
##             normally excepting operations (load, div, etc.) are
##             available.  This allows aggressive code motion across
##             branches, significantly increasing ILP.
##	
##      R   -> Restricted percolation.  No non-trapping operations.
##             This restricts code motion across branches to those
##             operations that can be proved to never throw an
##             exception (except if it would in the original
##             unoptimized code).
##             Note: These .lmdes2 files only restricts the scheduler
##             from generating non-trapping operations.  The optimizers
##             (classical and ILP) also generates non-trapping
##             operations if the Lglobal parameter
##             non_excepting_ops == yes (default).  If you want *no*
##             non-trapping operations, you must prevent them from
##             being added by both the optimizers and the scheduler.
##
##   Branches per cycle:
##
##      int -> The maximum number of branches that may be issued per
##             cycle.
##
##   Branch placement:
##
##      L   -> Branches last.  Branches are scheduled so they are last
##             in each cycle's operation packet and no non-branches
##             occur after a branch in the same cycle.  Required for
##             EPIC and may be used for SS.
##
##      X   -> Branches anywhere.  Branches may occur anywhere in each
##             cycle's operation packet.  These branches guard the
##             the non-branch operations placed after it in the same
##             cycle (prevent execution their execution, if the branch
##             is taken).  Produces tighter schedules (since you
##             effectively have a type of free predication on a few
##             operations each cycle) but requires a more complex
##             processor implementation.
##

mdes_Limpactdir = $(prefix)/mdes/Limpact

mdes_Limpact_DATA = \
	mdes/Limpact/EPIC_1G_1BL.lmdes2 mdes/Limpact/EPIC_2G_1BL.lmdes2 \
	mdes/Limpact/EPIC_2G_2BL.lmdes2 mdes/Limpact/EPIC_4G_1BL.lmdes2 \
	mdes/Limpact/EPIC_4G_2BL.lmdes2 mdes/Limpact/EPIC_4G_4BL.lmdes2 \
	mdes/Limpact/EPIC_8G_1BL.lmdes2	mdes/Limpact/EPIC_8G_2BL.lmdes2 \
	mdes/Limpact/EPIC_8G_4BL.lmdes2 mdes/Limpact/EPIC_8G_8BL.lmdes2 \
	mdes/Limpact/EPIC_8G_1BL_TI.lmdes2 \
	mdes/Limpact/EPIC_8G_MIX_3BX_LDLAT3.lmdes2 \
	mdes/Limpact/EPIC_8G_MIX_3BX.lmdes2  mdes/Limpact/EPIC_16G_1BL.lmdes2 \
	mdes/Limpact/EPIC_16G_2BL.lmdes2 mdes/Limpact/EPIC_16G_4BL.lmdes2 \
	mdes/Limpact/EPIC_16G_8BL.lmdes2 mdes/Limpact/EPIC_16G_16BL.lmdes2 \
	mdes/Limpact/EPIC_1R_1BL.lmdes2 mdes/Limpact/EPIC_2R_1BL.lmdes2 \
	mdes/Limpact/EPIC_2R_2BL.lmdes2 mdes/Limpact/EPIC_4R_1BL.lmdes2 \
	mdes/Limpact/EPIC_4R_2BL.lmdes2 mdes/Limpact/EPIC_4R_4BL.lmdes2 \
	mdes/Limpact/EPIC_8R_1BL.lmdes2 mdes/Limpact/EPIC_8R_2BL.lmdes2 \
	mdes/Limpact/EPIC_8R_4BL.lmdes2 mdes/Limpact/EPIC_8R_8BL.lmdes2 \
	mdes/Limpact/EPIC_16R_16BL.lmdes2 mdes/Limpact/EPIC_16R_1BL.lmdes2 \
	mdes/Limpact/EPIC_16R_2BL.lmdes2 mdes/Limpact/EPIC_16R_4BL.lmdes2 \
	mdes/Limpact/EPIC_16R_8BL.lmdes2 \
	mdes/Limpact/IMPACT_1G.lmdes mdes/Limpact/IMPACT_1G.lmdes2 \
	mdes/Limpact/IMPACT_2G_1BR.lmdes mdes/Limpact/IMPACT_2G_1BR.lmdes2 \
	mdes/Limpact/IMPACT_2G.lmdes mdes/Limpact/IMPACT_2G.lmdes2 \
	mdes/Limpact/IMPACT_3G_1BR.lmdes mdes/Limpact/IMPACT_3G_1BR.lmdes2 \
	mdes/Limpact/IMPACT_3G_2BR.lmdes mdes/Limpact/IMPACT_3G_2BR.lmdes2 \
	mdes/Limpact/IMPACT_3G.lmdes mdes/Limpact/IMPACT_3G.lmdes2 \
	mdes/Limpact/IMPACT_4G_1BR.lmdes mdes/Limpact/IMPACT_4G_1BR.lmdes2 \
	mdes/Limpact/IMPACT_4G_2BR.lmdes mdes/Limpact/IMPACT_4G_2BR.lmdes2 \
	mdes/Limpact/IMPACT_4G.lmdes mdes/Limpact/IMPACT_4G.lmdes2 \
	mdes/Limpact/IMPACT_8G_1BR.lmdes mdes/Limpact/IMPACT_8G_1BR.lmdes2 \
	mdes/Limpact/IMPACT_8G_2BR.lmdes mdes/Limpact/IMPACT_8G_2BR.lmdes2 \
	mdes/Limpact/IMPACT_8G_4BR.lmdes mdes/Limpact/IMPACT_8G_4BR.lmdes2 \
	mdes/Limpact/IMPACT_8G.lmdes mdes/Limpact/IMPACT_8G.lmdes2 \
	mdes/Limpact/IMPACT_16G_1BR.lmdes mdes/Limpact/IMPACT_16G_1BR.lmdes2 \
	mdes/Limpact/IMPACT_16G_2BR.lmdes mdes/Limpact/IMPACT_16G_2BR.lmdes2 \
	mdes/Limpact/IMPACT_16G_4BR.lmdes mdes/Limpact/IMPACT_16G_4BR.lmdes2 \
	mdes/Limpact/IMPACT_16G_8BR.lmdes mdes/Limpact/IMPACT_16G_8BR.lmdes2 \
	mdes/Limpact/IMPACT_16G.lmdes mdes/Limpact/IMPACT_16G.lmdes2 \
	mdes/Limpact/IMPACT_1R.lmdes mdes/Limpact/IMPACT_1R.lmdes2 \
	mdes/Limpact/IMPACT_2R_1BR.lmdes mdes/Limpact/IMPACT_2R_1BR.lmdes2 \
	mdes/Limpact/IMPACT_2R.lmdes mdes/Limpact/IMPACT_2R.lmdes2 \
	mdes/Limpact/IMPACT_4R_1BR.lmdes mdes/Limpact/IMPACT_4R_1BR.lmdes2 \
	mdes/Limpact/IMPACT_4R_2BR.lmdes mdes/Limpact/IMPACT_4R_2BR.lmdes2 \
	mdes/Limpact/IMPACT_4R.lmdes mdes/Limpact/IMPACT_4R.lmdes2 \
	mdes/Limpact/IMPACT_8R_1BR.lmdes mdes/Limpact/IMPACT_8R_1BR.lmdes2 \
	mdes/Limpact/IMPACT_8R_2BR.lmdes mdes/Limpact/IMPACT_8R_2BR.lmdes2 \
	mdes/Limpact/IMPACT_8R_4BR.lmdes mdes/Limpact/IMPACT_8R_4BR.lmdes2 \
	mdes/Limpact/IMPACT_8R.lmdes mdes/Limpact/IMPACT_8R.lmdes2 \
	mdes/Limpact/IMPACT_16R_1BR.lmdes mdes/Limpact/IMPACT_16R_1BR.lmdes2 \
	mdes/Limpact/IMPACT_16R_2BR.lmdes mdes/Limpact/IMPACT_16R_2BR.lmdes2 \
	mdes/Limpact/IMPACT_16R_4BR.lmdes mdes/Limpact/IMPACT_16R_4BR.lmdes2 \
	mdes/Limpact/IMPACT_16R_8BR.lmdes mdes/Limpact/IMPACT_16R_8BR.lmdes2 \
	mdes/Limpact/IMPACT_16R.lmdes mdes/Limpact/IMPACT_16R.lmdes2 \
	mdes/Limpact/SS_1G_1BL.lmdes2 mdes/Limpact/SS_1G_1BX.lmdes2 \
	mdes/Limpact/SS_2G_1BL.lmdes2 mdes/Limpact/SS_2G_1BX.lmdes2 \
	mdes/Limpact/SS_2G_2BL.lmdes2 mdes/Limpact/SS_2G_2BX.lmdes2 \
	mdes/Limpact/SS_4G_1BL.lmdes2 mdes/Limpact/SS_4G_1BX.lmdes2 \
	mdes/Limpact/SS_4G_2BL.lmdes2 mdes/Limpact/SS_4G_2BX.lmdes2 \
	mdes/Limpact/SS_4G_4BL.lmdes2 mdes/Limpact/SS_4G_4BX.lmdes2 \
	mdes/Limpact/SS_8G_1BL.lmdes2 mdes/Limpact/SS_8G_1BX.lmdes2 \
	mdes/Limpact/SS_8G_2BL.lmdes2 mdes/Limpact/SS_8G_2BX.lmdes2 \
	mdes/Limpact/SS_8G_4BL.lmdes2 mdes/Limpact/SS_8G_4BX.lmdes2 \
	mdes/Limpact/SS_8G_8BL.lmdes2 mdes/Limpact/SS_8G_8BX.lmdes2 \
	mdes/Limpact/SS_16G_1BL.lmdes2 mdes/Limpact/SS_16G_1BX.lmdes2 \
	mdes/Limpact/SS_16G_2BL.lmdes2 mdes/Limpact/SS_16G_2BX.lmdes2 \
	mdes/Limpact/SS_16G_4BL.lmdes2 mdes/Limpact/SS_16G_4BX.lmdes2 \
	mdes/Limpact/SS_16G_8BL.lmdes2 mdes/Limpact/SS_16G_8BX.lmdes2 \
	mdes/Limpact/SS_16G_16BL.lmdes2 mdes/Limpact/SS_16G_16BX.lmdes2 \
	mdes/Limpact/SS_1R_1BL.lmdes2 mdes/Limpact/SS_1R_1BX.lmdes2 \
	mdes/Limpact/SS_2R_1BL.lmdes2 mdes/Limpact/SS_2R_1BX.lmdes2 \
	mdes/Limpact/SS_2R_2BL.lmdes2 mdes/Limpact/SS_2R_2BX.lmdes2 \
	mdes/Limpact/SS_4R_1BL.lmdes2 mdes/Limpact/SS_4R_1BX.lmdes2 \
	mdes/Limpact/SS_4R_2BL.lmdes2 mdes/Limpact/SS_4R_2BX.lmdes2 \
	mdes/Limpact/SS_4R_4BL.lmdes2 mdes/Limpact/SS_4R_4BX.lmdes2 \
	mdes/Limpact/SS_8R_1BL.lmdes2 mdes/Limpact/SS_8R_1BX.lmdes2 \
	mdes/Limpact/SS_8R_2BL.lmdes2 mdes/Limpact/SS_8R_2BX.lmdes2 \
	mdes/Limpact/SS_8R_4BL.lmdes2 mdes/Limpact/SS_8R_4BX.lmdes2 \
	mdes/Limpact/SS_8R_8BL.lmdes2 mdes/Limpact/SS_8R_8BX.lmdes2 \
	mdes/Limpact/SS_16R_1BL.lmdes2 mdes/Limpact/SS_16R_1BX.lmdes2 \
	mdes/Limpact/SS_16R_2BL.lmdes2 mdes/Limpact/SS_16R_2BX.lmdes2 \
	mdes/Limpact/SS_16R_4BL.lmdes2 mdes/Limpact/SS_16R_4BX.lmdes2 \
	mdes/Limpact/SS_16R_8BL.lmdes2 mdes/Limpact/SS_16R_8BX.lmdes2 \
	mdes/Limpact/SS_16R_16BL.lmdes2 mdes/Limpact/SS_16R_16BX.lmdes2
dist_mdes_Limpact_DATA = mdes/Limpact/EPIC_8G_1BL_TI.hmdes2 \
			 mdes/Limpact/IMPACT_BASE_TEMPLATE.hmdes2 \
			 mdes/Limpact/IMPACT_GEN_BASE.hmdes \
			 mdes/Limpact/IMPACT_GEN_NEWPRED_BASE.hmdes \
			 mdes/Limpact/IMPACT_IA64_TEMPLATE.hmdes2 \
			 mdes/Limpact/IMPACT_RES_BASE.hmdes \
			 mdes/Limpact/IMPACT_RES_NEWPRED_BASE.hmdes \
			 mdes/Limpact/IMPACT_SIMPLE_TEMPLATE.hmdes2 \
			 mdes/Limpact/PACT_BASE_TEMPLATE.hmdes2 \
			 mdes/Limpact/PACT_SIMPLE_TEMPLATE.hmdes2

## Specify the files that are created by this make file so that 'make clean'
## will delete them properly.
mdes_Limpact_CLN = $(mdes_Limpact_DATA)

## A target to support building everything under this subdirectory.  This
## should refer to anything defined above.  There should be two targets; one
## with and one without the trailing slash.  The trailing slash will allow
## the use of tab completion.
mdes/Limpact mdes/Limpact/: $(mdes_Limpact_DATA)

## A target to support cleaning everything under this subdirectory.
mdes/Limpact/clean:
	rm -f $(mdes_Limpact_CLN)

mdes/Limpact/EPIC_1G_1BL.lmdes2: \
		$(abs_srcdir)/mdes/Limpact/IMPACT_SIMPLE_TEMPLATE.hmdes2 \
		$(top_builddir)/mdes/structure/structure_IMPACT.ho \
		bin/md_preprocessor bin/md_compiler bin/lmdes2_customizer \
		bin/convert_hmdes
	mkdir -p mdes/Limpact
	hmdesc $(abs_srcdir)/mdes/Limpact/IMPACT_SIMPLE_TEMPLATE.hmdes2 \
		-o mdes/Limpact/EPIC_1G_1BL.lmdes2 -DWIDTH=1 \
		-DNUM_BRANCHES=1 -DBRANCHES_AT_END=1 -DNON_TRAPPING_OPS=1

mdes/Limpact/EPIC_2G_1BL.lmdes2: \
		$(abs_srcdir)/mdes/Limpact/IMPACT_SIMPLE_TEMPLATE.hmdes2 \
		$(top_builddir)/mdes/structure/structure_IMPACT.ho \
		bin/md_preprocessor bin/md_compiler bin/lmdes2_customizer \
		bin/convert_hmdes
	mkdir -p mdes/Limpact
	hmdesc $(abs_srcdir)/mdes/Limpact/IMPACT_SIMPLE_TEMPLATE.hmdes2 \
		-o mdes/Limpact/EPIC_2G_1BL.lmdes2 -DWIDTH=2 \
		-DNUM_BRANCHES=1 -DBRANCHES_AT_END=1 -DNON_TRAPPING_OPS=1

mdes/Limpact/EPIC_2G_2BL.lmdes2: \
		$(abs_srcdir)/mdes/Limpact/IMPACT_SIMPLE_TEMPLATE.hmdes2 \
		$(top_builddir)/mdes/structure/structure_IMPACT.ho \
		bin/md_preprocessor bin/md_compiler bin/lmdes2_customizer \
		bin/convert_hmdes
	mkdir -p mdes/Limpact
	hmdesc $(abs_srcdir)/mdes/Limpact/IMPACT_SIMPLE_TEMPLATE.hmdes2 \
		-o mdes/Limpact/EPIC_2G_2BL.lmdes2 -DWIDTH=2 \
		-DNUM_BRANCHES=2 -DBRANCHES_AT_END=1 -DNON_TRAPPING_OPS=1

mdes/Limpact/EPIC_4G_1BL.lmdes2: \
		$(abs_srcdir)/mdes/Limpact/IMPACT_SIMPLE_TEMPLATE.hmdes2 \
		$(top_builddir)/mdes/structure/structure_IMPACT.ho \
		bin/md_preprocessor bin/md_compiler bin/lmdes2_customizer \
		bin/convert_hmdes
	mkdir -p mdes/Limpact
	hmdesc $(abs_srcdir)/mdes/Limpact/IMPACT_SIMPLE_TEMPLATE.hmdes2 \
		-o mdes/Limpact/EPIC_4G_1BL.lmdes2 -DWIDTH=4 \
		-DNUM_BRANCHES=1 -DBRANCHES_AT_END=1 -DNON_TRAPPING_OPS=1

mdes/Limpact/EPIC_4G_2BL.lmdes2: \
		$(abs_srcdir)/mdes/Limpact/IMPACT_SIMPLE_TEMPLATE.hmdes2 \
		$(top_builddir)/mdes/structure/structure_IMPACT.ho \
		bin/md_preprocessor bin/md_compiler bin/lmdes2_customizer \
		bin/convert_hmdes
	mkdir -p mdes/Limpact
	hmdesc $(abs_srcdir)/mdes/Limpact/IMPACT_SIMPLE_TEMPLATE.hmdes2 \
		-o mdes/Limpact/EPIC_4G_2BL.lmdes2 -DWIDTH=4 \
		-DNUM_BRANCHES=2 -DBRANCHES_AT_END=1 -DNON_TRAPPING_OPS=1

mdes/Limpact/EPIC_4G_4BL.lmdes2: \
		$(abs_srcdir)/mdes/Limpact/IMPACT_SIMPLE_TEMPLATE.hmdes2 \
		$(top_builddir)/mdes/structure/structure_IMPACT.ho \
		bin/md_preprocessor bin/md_compiler bin/lmdes2_customizer \
		bin/convert_hmdes
	mkdir -p mdes/Limpact
	hmdesc $(abs_srcdir)/mdes/Limpact/IMPACT_SIMPLE_TEMPLATE.hmdes2 \
		-o mdes/Limpact/EPIC_4G_4BL.lmdes2 -DWIDTH=4 \
		-DNUM_BRANCHES=4 -DBRANCHES_AT_END=1 -DNON_TRAPPING_OPS=1

mdes/Limpact/EPIC_8G_1BL.lmdes2: \
		$(abs_srcdir)/mdes/Limpact/IMPACT_SIMPLE_TEMPLATE.hmdes2 \
		$(top_builddir)/mdes/structure/structure_IMPACT.ho \
		bin/md_preprocessor bin/md_compiler bin/lmdes2_customizer \
		bin/convert_hmdes
	mkdir -p mdes/Limpact
	hmdesc $(abs_srcdir)/mdes/Limpact/IMPACT_SIMPLE_TEMPLATE.hmdes2 \
		-o mdes/Limpact/EPIC_8G_1BL.lmdes2 -DWIDTH=8 \
		-DNUM_BRANCHES=1 -DBRANCHES_AT_END=1 -DNON_TRAPPING_OPS=1

mdes/Limpact/EPIC_8G_2BL.lmdes2: \
		$(abs_srcdir)/mdes/Limpact/IMPACT_SIMPLE_TEMPLATE.hmdes2 \
		$(top_builddir)/mdes/structure/structure_IMPACT.ho \
		bin/md_preprocessor bin/md_compiler bin/lmdes2_customizer \
		bin/convert_hmdes
	mkdir -p mdes/Limpact
	hmdesc $(abs_srcdir)/mdes/Limpact/IMPACT_SIMPLE_TEMPLATE.hmdes2 \
		-o mdes/Limpact/EPIC_8G_2BL.lmdes2 -DWIDTH=8 \
		-DNUM_BRANCHES=2 -DBRANCHES_AT_END=1 -DNON_TRAPPING_OPS=1

mdes/Limpact/EPIC_8G_4BL.lmdes2: \
		$(abs_srcdir)/mdes/Limpact/IMPACT_SIMPLE_TEMPLATE.hmdes2 \
		$(top_builddir)/mdes/structure/structure_IMPACT.ho \
		bin/md_preprocessor bin/md_compiler bin/lmdes2_customizer \
		bin/convert_hmdes
	mkdir -p mdes/Limpact
	hmdesc $(abs_srcdir)/mdes/Limpact/IMPACT_SIMPLE_TEMPLATE.hmdes2 \
		-o mdes/Limpact/EPIC_8G_4BL.lmdes2 -DWIDTH=8 \
		-DNUM_BRANCHES=4 -DBRANCHES_AT_END=1 -DNON_TRAPPING_OPS=1

mdes/Limpact/EPIC_8G_8BL.lmdes2: \
		$(abs_srcdir)/mdes/Limpact/IMPACT_SIMPLE_TEMPLATE.hmdes2 \
		$(top_builddir)/mdes/structure/structure_IMPACT.ho \
		bin/md_preprocessor bin/md_compiler bin/lmdes2_customizer \
		bin/convert_hmdes
	mkdir -p mdes/Limpact
	hmdesc $(abs_srcdir)/mdes/Limpact/IMPACT_SIMPLE_TEMPLATE.hmdes2 \
		-o mdes/Limpact/EPIC_8G_8BL.lmdes2 -DWIDTH=8 \
		-DNUM_BRANCHES=8 -DBRANCHES_AT_END=1 -DNON_TRAPPING_OPS=1

# Build separate TI `C6x-based EPIC MIX    MCM 2/2000
mdes/Limpact/EPIC_8G_1BL_TI.lmdes2: \
		$(abs_srcdir)/mdes/Limpact/EPIC_8G_1BL_TI.hmdes2 \
		$(top_builddir)/mdes/structure/structure_IMPACT.ho \
		bin/md_preprocessor bin/md_compiler bin/lmdes2_customizer \
		bin/convert_hmdes
	mkdir -p mdes/Limpact
	hmdesc $(abs_srcdir)/mdes/Limpact/EPIC_8G_1BL_TI.hmdes2 \
		-o mdes/Limpact/EPIC_8G_1BL_TI.lmdes2 -DWIDTH=8

# Build Itanium-like EPIC MIX MCM 3/2001
mdes/Limpact/EPIC_8G_MIX_3BX.lmdes2: \
		$(abs_srcdir)/mdes/Limpact/PACT_SIMPLE_TEMPLATE.hmdes2 \
		$(top_builddir)/mdes/structure/structure_IMPACT.ho \
		bin/md_preprocessor bin/md_compiler bin/lmdes2_customizer \
		bin/convert_hmdes
	mkdir -p mdes/Limpact
	hmdesc $(abs_srcdir)/mdes/Limpact/PACT_SIMPLE_TEMPLATE.hmdes2 \
		-o mdes/Limpact/EPIC_8G_MIX_3BX.lmdes2 -DWIDTH=8 \
		-DNUM_BRANCHES=3 -DNUM_IALUS=5 -DNUM_FALUS=3 -DNUM_DIVS=1 \
		-DNUM_MEM_UNITS=3 -DNUM_CHK=8 -DBRANCHES_AT_END=0 \
		-DNON_TRAPPING_OPS=1 -DLOAD_LAT=2

# Build Itanium-like EPIC MIX MCM 3/2001
mdes/Limpact/EPIC_8G_MIX_3BX_LDLAT3.lmdes2: \
		$(abs_srcdir)/mdes/Limpact/PACT_SIMPLE_TEMPLATE.hmdes2 \
		$(top_builddir)/mdes/structure/structure_IMPACT.ho \
		bin/md_preprocessor bin/md_compiler bin/lmdes2_customizer \
		bin/convert_hmdes
	mkdir -p mdes/Limpact
	hmdesc $(abs_srcdir)/mdes/Limpact/PACT_SIMPLE_TEMPLATE.hmdes2 \
		-o mdes/Limpact/EPIC_8G_MIX_3BX_LDLAT3.lmdes2 -DWIDTH=8 \
		-DNUM_BRANCHES=3 -DNUM_IALUS=5 -DNUM_FALUS=3 -DNUM_DIVS=1 \
		-DNUM_MEM_UNITS=3 -DNUM_CHK=8 -DBRANCHES_AT_END=0 \
		-DNON_TRAPPING_OPS=1 -DLOAD_LAT=3

mdes/Limpact/EPIC_16G_1BL.lmdes2: \
		$(abs_srcdir)/mdes/Limpact/IMPACT_SIMPLE_TEMPLATE.hmdes2 \
		$(top_builddir)/mdes/structure/structure_IMPACT.ho \
		bin/md_preprocessor bin/md_compiler bin/lmdes2_customizer \
		bin/convert_hmdes
	mkdir -p mdes/Limpact
	hmdesc $(abs_srcdir)/mdes/Limpact/IMPACT_SIMPLE_TEMPLATE.hmdes2 \
		-o mdes/Limpact/EPIC_16G_1BL.lmdes2 -DWIDTH=16 \
		-DNUM_BRANCHES=1 -DBRANCHES_AT_END=1 -DNON_TRAPPING_OPS=1

mdes/Limpact/EPIC_16G_2BL.lmdes2: \
		$(abs_srcdir)/mdes/Limpact/IMPACT_SIMPLE_TEMPLATE.hmdes2 \
		$(top_builddir)/mdes/structure/structure_IMPACT.ho \
		bin/md_preprocessor bin/md_compiler bin/lmdes2_customizer \
		bin/convert_hmdes
	mkdir -p mdes/Limpact
	hmdesc $(abs_srcdir)/mdes/Limpact/IMPACT_SIMPLE_TEMPLATE.hmdes2 \
		-o mdes/Limpact/EPIC_16G_2BL.lmdes2 -DWIDTH=16 \
		-DNUM_BRANCHES=2 -DBRANCHES_AT_END=1 -DNON_TRAPPING_OPS=1

mdes/Limpact/EPIC_16G_4BL.lmdes2: \
		$(abs_srcdir)/mdes/Limpact/IMPACT_SIMPLE_TEMPLATE.hmdes2 \
		$(top_builddir)/mdes/structure/structure_IMPACT.ho \
		bin/md_preprocessor bin/md_compiler bin/lmdes2_customizer \
		bin/convert_hmdes
	mkdir -p mdes/Limpact
	hmdesc $(abs_srcdir)/mdes/Limpact/IMPACT_SIMPLE_TEMPLATE.hmdes2 \
		-o mdes/Limpact/EPIC_16G_4BL.lmdes2 -DWIDTH=16 \
		-DNUM_BRANCHES=4 -DBRANCHES_AT_END=1 -DNON_TRAPPING_OPS=1

mdes/Limpact/EPIC_16G_8BL.lmdes2: \
		$(abs_srcdir)/mdes/Limpact/IMPACT_SIMPLE_TEMPLATE.hmdes2 \
		$(top_builddir)/mdes/structure/structure_IMPACT.ho \
		bin/md_preprocessor bin/md_compiler bin/lmdes2_customizer \
		bin/convert_hmdes
	mkdir -p mdes/Limpact
	hmdesc $(abs_srcdir)/mdes/Limpact/IMPACT_SIMPLE_TEMPLATE.hmdes2 \
		-o mdes/Limpact/EPIC_16G_8BL.lmdes2 -DWIDTH=16 \
		-DNUM_BRANCHES=8 -DBRANCHES_AT_END=1 -DNON_TRAPPING_OPS=1

mdes/Limpact/EPIC_16G_16BL.lmdes2: \
		$(abs_srcdir)/mdes/Limpact/IMPACT_SIMPLE_TEMPLATE.hmdes2 \
		$(top_builddir)/mdes/structure/structure_IMPACT.ho \
		bin/md_preprocessor bin/md_compiler bin/lmdes2_customizer \
		bin/convert_hmdes
	mkdir -p mdes/Limpact
	hmdesc $(abs_srcdir)/mdes/Limpact/IMPACT_SIMPLE_TEMPLATE.hmdes2 \
		-o mdes/Limpact/EPIC_16G_16BL.lmdes2 -DWIDTH=16 \
		-DNUM_BRANCHES=16 -DBRANCHES_AT_END=1 -DNON_TRAPPING_OPS=1

mdes/Limpact/EPIC_1R_1BL.lmdes2: \
		$(abs_srcdir)/mdes/Limpact/IMPACT_SIMPLE_TEMPLATE.hmdes2 \
		$(top_builddir)/mdes/structure/structure_IMPACT.ho \
		bin/md_preprocessor bin/md_compiler bin/lmdes2_customizer \
		bin/convert_hmdes
	mkdir -p mdes/Limpact
	hmdesc $(abs_srcdir)/mdes/Limpact/IMPACT_SIMPLE_TEMPLATE.hmdes2 \
		-o mdes/Limpact/EPIC_1R_1BL.lmdes2 -DWIDTH=1 \
		-DNUM_BRANCHES=1 -DBRANCHES_AT_END=1 -DNON_TRAPPING_OPS=0

mdes/Limpact/EPIC_2R_1BL.lmdes2: \
		$(abs_srcdir)/mdes/Limpact/IMPACT_SIMPLE_TEMPLATE.hmdes2 \
		$(top_builddir)/mdes/structure/structure_IMPACT.ho \
		bin/md_preprocessor bin/md_compiler bin/lmdes2_customizer \
		bin/convert_hmdes
	mkdir -p mdes/Limpact
	hmdesc $(abs_srcdir)/mdes/Limpact/IMPACT_SIMPLE_TEMPLATE.hmdes2 \
		-o mdes/Limpact/EPIC_2R_1BL.lmdes2 -DWIDTH=2 \
		-DNUM_BRANCHES=1 -DBRANCHES_AT_END=1 -DNON_TRAPPING_OPS=0

mdes/Limpact/EPIC_2R_2BL.lmdes2: \
		$(abs_srcdir)/mdes/Limpact/IMPACT_SIMPLE_TEMPLATE.hmdes2 \
		$(top_builddir)/mdes/structure/structure_IMPACT.ho \
		bin/md_preprocessor bin/md_compiler bin/lmdes2_customizer \
		bin/convert_hmdes
	mkdir -p mdes/Limpact
	hmdesc $(abs_srcdir)/mdes/Limpact/IMPACT_SIMPLE_TEMPLATE.hmdes2 \
		-o mdes/Limpact/EPIC_2R_2BL.lmdes2 -DWIDTH=2 \
		-DNUM_BRANCHES=2 -DBRANCHES_AT_END=1 -DNON_TRAPPING_OPS=0

mdes/Limpact/EPIC_4R_1BL.lmdes2: \
		$(abs_srcdir)/mdes/Limpact/IMPACT_SIMPLE_TEMPLATE.hmdes2 \
		$(top_builddir)/mdes/structure/structure_IMPACT.ho \
		bin/md_preprocessor bin/md_compiler bin/lmdes2_customizer \
		bin/convert_hmdes
	mkdir -p mdes/Limpact
	hmdesc $(abs_srcdir)/mdes/Limpact/IMPACT_SIMPLE_TEMPLATE.hmdes2 \
		-o mdes/Limpact/EPIC_4R_1BL.lmdes2 -DWIDTH=4 \
		-DNUM_BRANCHES=1 -DBRANCHES_AT_END=1 -DNON_TRAPPING_OPS=0

mdes/Limpact/EPIC_4R_2BL.lmdes2: \
		$(abs_srcdir)/mdes/Limpact/IMPACT_SIMPLE_TEMPLATE.hmdes2 \
		$(top_builddir)/mdes/structure/structure_IMPACT.ho \
		bin/md_preprocessor bin/md_compiler bin/lmdes2_customizer \
		bin/convert_hmdes
	mkdir -p mdes/Limpact
	hmdesc $(abs_srcdir)/mdes/Limpact/IMPACT_SIMPLE_TEMPLATE.hmdes2 \
		-o mdes/Limpact/EPIC_4R_2BL.lmdes2 -DWIDTH=4 \
		-DNUM_BRANCHES=2 -DBRANCHES_AT_END=1 -DNON_TRAPPING_OPS=0

mdes/Limpact/EPIC_4R_4BL.lmdes2: \
		$(abs_srcdir)/mdes/Limpact/IMPACT_SIMPLE_TEMPLATE.hmdes2 \
		$(top_builddir)/mdes/structure/structure_IMPACT.ho \
		bin/md_preprocessor bin/md_compiler bin/lmdes2_customizer \
		bin/convert_hmdes
	mkdir -p mdes/Limpact
	hmdesc $(abs_srcdir)/mdes/Limpact/IMPACT_SIMPLE_TEMPLATE.hmdes2 \
		-o mdes/Limpact/EPIC_4R_4BL.lmdes2 -DWIDTH=4 \
		-DNUM_BRANCHES=4 -DBRANCHES_AT_END=1 -DNON_TRAPPING_OPS=0

mdes/Limpact/EPIC_8R_1BL.lmdes2: \
		$(abs_srcdir)/mdes/Limpact/IMPACT_SIMPLE_TEMPLATE.hmdes2 \
		$(top_builddir)/mdes/structure/structure_IMPACT.ho \
		bin/md_preprocessor bin/md_compiler bin/lmdes2_customizer \
		bin/convert_hmdes
	mkdir -p mdes/Limpact
	hmdesc $(abs_srcdir)/mdes/Limpact/IMPACT_SIMPLE_TEMPLATE.hmdes2 \
		-o mdes/Limpact/EPIC_8R_1BL.lmdes2 -DWIDTH=8 \
		-DNUM_BRANCHES=1 -DBRANCHES_AT_END=1 -DNON_TRAPPING_OPS=0

mdes/Limpact/EPIC_8R_2BL.lmdes2: \
		$(abs_srcdir)/mdes/Limpact/IMPACT_SIMPLE_TEMPLATE.hmdes2 \
		$(top_builddir)/mdes/structure/structure_IMPACT.ho \
		bin/md_preprocessor bin/md_compiler bin/lmdes2_customizer \
		bin/convert_hmdes
	mkdir -p mdes/Limpact
	hmdesc $(abs_srcdir)/mdes/Limpact/IMPACT_SIMPLE_TEMPLATE.hmdes2 \
		-o mdes/Limpact/EPIC_8R_2BL.lmdes2 -DWIDTH=8 \
		-DNUM_BRANCHES=2 -DBRANCHES_AT_END=1 -DNON_TRAPPING_OPS=0

mdes/Limpact/EPIC_8R_4BL.lmdes2: \
		$(abs_srcdir)/mdes/Limpact/IMPACT_SIMPLE_TEMPLATE.hmdes2 \
		$(top_builddir)/mdes/structure/structure_IMPACT.ho \
		bin/md_preprocessor bin/md_compiler bin/lmdes2_customizer \
		bin/convert_hmdes
	mkdir -p mdes/Limpact
	hmdesc $(abs_srcdir)/mdes/Limpact/IMPACT_SIMPLE_TEMPLATE.hmdes2 \
		-o mdes/Limpact/EPIC_8R_4BL.lmdes2 -DWIDTH=8 \
		-DNUM_BRANCHES=4 -DBRANCHES_AT_END=1 -DNON_TRAPPING_OPS=0

mdes/Limpact/EPIC_8R_8BL.lmdes2: \
		$(abs_srcdir)/mdes/Limpact/IMPACT_SIMPLE_TEMPLATE.hmdes2 \
		$(top_builddir)/mdes/structure/structure_IMPACT.ho \
		bin/md_preprocessor bin/md_compiler bin/lmdes2_customizer \
		bin/convert_hmdes
	mkdir -p mdes/Limpact
	hmdesc $(abs_srcdir)/mdes/Limpact/IMPACT_SIMPLE_TEMPLATE.hmdes2 \
		-o mdes/Limpact/EPIC_8R_8BL.lmdes2 -DWIDTH=8 \
		-DNUM_BRANCHES=8 -DBRANCHES_AT_END=1 -DNON_TRAPPING_OPS=0

mdes/Limpact/EPIC_16R_1BL.lmdes2: \
		$(abs_srcdir)/mdes/Limpact/IMPACT_SIMPLE_TEMPLATE.hmdes2 \
		$(top_builddir)/mdes/structure/structure_IMPACT.ho \
		bin/md_preprocessor bin/md_compiler bin/lmdes2_customizer \
		bin/convert_hmdes
	mkdir -p mdes/Limpact
	hmdesc $(abs_srcdir)/mdes/Limpact/IMPACT_SIMPLE_TEMPLATE.hmdes2 \
		-o mdes/Limpact/EPIC_16R_1BL.lmdes2 -DWIDTH=16 \
		-DNUM_BRANCHES=1 -DBRANCHES_AT_END=1 -DNON_TRAPPING_OPS=0

mdes/Limpact/EPIC_16R_2BL.lmdes2: \
		$(abs_srcdir)/mdes/Limpact/IMPACT_SIMPLE_TEMPLATE.hmdes2 \
		$(top_builddir)/mdes/structure/structure_IMPACT.ho \
		bin/md_preprocessor bin/md_compiler bin/lmdes2_customizer \
		bin/convert_hmdes
	mkdir -p mdes/Limpact
	hmdesc $(abs_srcdir)/mdes/Limpact/IMPACT_SIMPLE_TEMPLATE.hmdes2 \
		-o mdes/Limpact/EPIC_16R_2BL.lmdes2 -DWIDTH=16 \
		-DNUM_BRANCHES=2 -DBRANCHES_AT_END=1 -DNON_TRAPPING_OPS=0

mdes/Limpact/EPIC_16R_4BL.lmdes2: \
		$(abs_srcdir)/mdes/Limpact/IMPACT_SIMPLE_TEMPLATE.hmdes2 \
		$(top_builddir)/mdes/structure/structure_IMPACT.ho \
		bin/md_preprocessor bin/md_compiler bin/lmdes2_customizer \
		bin/convert_hmdes
	mkdir -p mdes/Limpact
	hmdesc $(abs_srcdir)/mdes/Limpact/IMPACT_SIMPLE_TEMPLATE.hmdes2 \
		-o mdes/Limpact/EPIC_16R_4BL.lmdes2 -DWIDTH=16 \
		-DNUM_BRANCHES=4 -DBRANCHES_AT_END=1 -DNON_TRAPPING_OPS=0

mdes/Limpact/EPIC_16R_8BL.lmdes2: \
		$(abs_srcdir)/mdes/Limpact/IMPACT_SIMPLE_TEMPLATE.hmdes2 \
		$(top_builddir)/mdes/structure/structure_IMPACT.ho \
		bin/md_preprocessor bin/md_compiler bin/lmdes2_customizer \
		bin/convert_hmdes
	mkdir -p mdes/Limpact
	hmdesc $(abs_srcdir)/mdes/Limpact/IMPACT_SIMPLE_TEMPLATE.hmdes2 \
		-o mdes/Limpact/EPIC_16R_8BL.lmdes2 -DWIDTH=16 \
		-DNUM_BRANCHES=8 -DBRANCHES_AT_END=1 -DNON_TRAPPING_OPS=0

mdes/Limpact/EPIC_16R_16BL.lmdes2: \
		$(abs_srcdir)/mdes/Limpact/IMPACT_SIMPLE_TEMPLATE.hmdes2 \
		$(top_builddir)/mdes/structure/structure_IMPACT.ho \
		bin/md_preprocessor bin/md_compiler bin/lmdes2_customizer \
		bin/convert_hmdes
	mkdir -p mdes/Limpact
	hmdesc $(abs_srcdir)/mdes/Limpact/IMPACT_SIMPLE_TEMPLATE.hmdes2 \
		-o mdes/Limpact/EPIC_16R_16BL.lmdes2 -DWIDTH=16 \
		-DNUM_BRANCHES=16 -DBRANCHES_AT_END=1 -DNON_TRAPPING_OPS=0

# WARNING - ISSUE rates and BRANCH rates are zero relative!!!!
# general percolation models
mdes/Limpact/IMPACT_1G.lmdes mdes/Limpact/IMPACT_1G.lmdes2: \
		$(abs_srcdir)/mdes/Limpact/IMPACT_GEN_BASE.hmdes \
		$(top_builddir)/mdes/structure/structure_IMPACT.ho \
		bin/Lmdes_build bin/md_preprocessor bin/md_compiler \
		bin/lmdes2_customizer bin/convert_hmdes
	mkdir -p mdes/Limpact
	Lmdes_build2 $(abs_srcdir)/mdes/Limpact/IMPACT_GEN_BASE.hmdes \
		mdes/Limpact/IMPACT_1G.lmdes -DISSUE=0 -DBRANCH=0

mdes/Limpact/IMPACT_2G_1BR.lmdes mdes/Limpact/IMPACT_2G_1BR.lmdes2: \
		$(abs_srcdir)/mdes/Limpact/IMPACT_GEN_BASE.hmdes \
		$(top_builddir)/mdes/structure/structure_IMPACT.ho \
		bin/Lmdes_build bin/md_preprocessor bin/md_compiler \
		bin/lmdes2_customizer bin/convert_hmdes
	mkdir -p mdes/Limpact
	Lmdes_build2 $(abs_srcdir)/mdes/Limpact/IMPACT_GEN_BASE.hmdes \
		mdes/Limpact/IMPACT_2G_1BR.lmdes -DISSUE=1 -DBRANCH=0

mdes/Limpact/IMPACT_2G.lmdes mdes/Limpact/IMPACT_2G.lmdes2: \
		$(abs_srcdir)/mdes/Limpact/IMPACT_GEN_BASE.hmdes \
		$(top_builddir)/mdes/structure/structure_IMPACT.ho \
		bin/Lmdes_build bin/md_preprocessor bin/md_compiler \
		bin/lmdes2_customizer bin/convert_hmdes
	mkdir -p mdes/Limpact
	Lmdes_build2 $(abs_srcdir)/mdes/Limpact/IMPACT_GEN_BASE.hmdes \
		mdes/Limpact/IMPACT_2G.lmdes -DISSUE=1 -DBRANCH=1

mdes/Limpact/IMPACT_3G_1BR.lmdes mdes/Limpact/IMPACT_3G_1BR.lmdes2: \
		$(abs_srcdir)/mdes/Limpact/IMPACT_GEN_BASE.hmdes \
		$(top_builddir)/mdes/structure/structure_IMPACT.ho \
		bin/Lmdes_build bin/md_preprocessor bin/md_compiler \
		bin/lmdes2_customizer bin/convert_hmdes
	mkdir -p mdes/Limpact
	Lmdes_build2 $(abs_srcdir)/mdes/Limpact/IMPACT_GEN_BASE.hmdes \
		mdes/Limpact/IMPACT_3G_1BR.lmdes -DISSUE=2 -DBRANCH=0

mdes/Limpact/IMPACT_3G_2BR.lmdes mdes/Limpact/IMPACT_3G_2BR.lmdes2: \
		$(abs_srcdir)/mdes/Limpact/IMPACT_GEN_BASE.hmdes \
		$(top_builddir)/mdes/structure/structure_IMPACT.ho \
		bin/Lmdes_build bin/md_preprocessor bin/md_compiler \
		bin/lmdes2_customizer bin/convert_hmdes
	mkdir -p mdes/Limpact
	Lmdes_build2 $(abs_srcdir)/mdes/Limpact/IMPACT_GEN_BASE.hmdes \
		mdes/Limpact/IMPACT_3G_2BR.lmdes -DISSUE=2 -DBRANCH=1

mdes/Limpact/IMPACT_3G.lmdes mdes/Limpact/IMPACT_3G.lmdes2: \
		$(abs_srcdir)/mdes/Limpact/IMPACT_GEN_BASE.hmdes \
		$(top_builddir)/mdes/structure/structure_IMPACT.ho \
		bin/Lmdes_build bin/md_preprocessor bin/md_compiler \
		bin/lmdes2_customizer bin/convert_hmdes
	mkdir -p mdes/Limpact
	Lmdes_build2 $(abs_srcdir)/mdes/Limpact/IMPACT_GEN_BASE.hmdes \
		mdes/Limpact/IMPACT_3G.lmdes -DISSUE=2 -DBRANCH=2

mdes/Limpact/IMPACT_4G_1BR.lmdes mdes/Limpact/IMPACT_4G_1BR.lmdes2: \
		$(abs_srcdir)/mdes/Limpact/IMPACT_GEN_BASE.hmdes \
		$(top_builddir)/mdes/structure/structure_IMPACT.ho \
		bin/Lmdes_build bin/md_preprocessor bin/md_compiler \
		bin/lmdes2_customizer bin/convert_hmdes
	mkdir -p mdes/Limpact
	Lmdes_build2 $(abs_srcdir)/mdes/Limpact/IMPACT_GEN_BASE.hmdes \
		mdes/Limpact/IMPACT_4G_1BR.lmdes -DISSUE=3 -DBRANCH=0

mdes/Limpact/IMPACT_4G_2BR.lmdes mdes/Limpact/IMPACT_4G_2BR.lmdes2: \
		$(abs_srcdir)/mdes/Limpact/IMPACT_GEN_BASE.hmdes \
		$(top_builddir)/mdes/structure/structure_IMPACT.ho \
		bin/Lmdes_build bin/md_preprocessor bin/md_compiler \
		bin/lmdes2_customizer bin/convert_hmdes
	mkdir -p mdes/Limpact
	Lmdes_build2 $(abs_srcdir)/mdes/Limpact/IMPACT_GEN_BASE.hmdes \
		mdes/Limpact/IMPACT_4G_2BR.lmdes -DISSUE=3 -DBRANCH=1

mdes/Limpact/IMPACT_4G.lmdes mdes/Limpact/IMPACT_4G.lmdes2: \
		$(abs_srcdir)/mdes/Limpact/IMPACT_GEN_BASE.hmdes \
		$(top_builddir)/mdes/structure/structure_IMPACT.ho \
		bin/Lmdes_build bin/md_preprocessor bin/md_compiler \
		bin/lmdes2_customizer bin/convert_hmdes
	mkdir -p mdes/Limpact
	Lmdes_build2 $(abs_srcdir)/mdes/Limpact/IMPACT_GEN_BASE.hmdes \
		mdes/Limpact/IMPACT_4G.lmdes -DISSUE=3 -DBRANCH=3

mdes/Limpact/IMPACT_8G_1BR.lmdes mdes/Limpact/IMPACT_8G_1BR.lmdes2: \
		$(abs_srcdir)/mdes/Limpact/IMPACT_GEN_BASE.hmdes \
		$(top_builddir)/mdes/structure/structure_IMPACT.ho \
		bin/Lmdes_build bin/md_preprocessor bin/md_compiler \
		bin/lmdes2_customizer bin/convert_hmdes
	mkdir -p mdes/Limpact
	Lmdes_build2 $(abs_srcdir)/mdes/Limpact/IMPACT_GEN_BASE.hmdes \
		mdes/Limpact/IMPACT_8G_1BR.lmdes -DISSUE=7 -DBRANCH=0

mdes/Limpact/IMPACT_8G_2BR.lmdes mdes/Limpact/IMPACT_8G_2BR.lmdes2: \
		$(abs_srcdir)/mdes/Limpact/IMPACT_GEN_BASE.hmdes \
		$(top_builddir)/mdes/structure/structure_IMPACT.ho \
		bin/Lmdes_build bin/md_preprocessor bin/md_compiler \
		bin/lmdes2_customizer bin/convert_hmdes
	mkdir -p mdes/Limpact
	Lmdes_build2 $(abs_srcdir)/mdes/Limpact/IMPACT_GEN_BASE.hmdes \
		mdes/Limpact/IMPACT_8G_2BR.lmdes -DISSUE=7 -DBRANCH=1

mdes/Limpact/IMPACT_8G_4BR.lmdes mdes/Limpact/IMPACT_8G_4BR.lmdes2: \
		$(abs_srcdir)/mdes/Limpact/IMPACT_GEN_BASE.hmdes \
		$(top_builddir)/mdes/structure/structure_IMPACT.ho \
		bin/Lmdes_build bin/md_preprocessor bin/md_compiler \
		bin/lmdes2_customizer bin/convert_hmdes
	mkdir -p mdes/Limpact
	Lmdes_build2 $(abs_srcdir)/mdes/Limpact/IMPACT_GEN_BASE.hmdes \
		mdes/Limpact/IMPACT_8G_4BR.lmdes -DISSUE=7 -DBRANCH=3

mdes/Limpact/IMPACT_8G.lmdes mdes/Limpact/IMPACT_8G.lmdes2: \
		$(abs_srcdir)/mdes/Limpact/IMPACT_GEN_BASE.hmdes \
		$(top_builddir)/mdes/structure/structure_IMPACT.ho \
		bin/Lmdes_build bin/md_preprocessor bin/md_compiler \
		bin/lmdes2_customizer bin/convert_hmdes
	mkdir -p mdes/Limpact
	Lmdes_build2 $(abs_srcdir)/mdes/Limpact/IMPACT_GEN_BASE.hmdes \
		mdes/Limpact/IMPACT_8G.lmdes -DISSUE=7 -DBRANCH=7


mdes/Limpact/IMPACT_16G_1BR.lmdes mdes/Limpact/IMPACT_16G_1BR.lmdes2: \
		$(abs_srcdir)/mdes/Limpact/IMPACT_GEN_BASE.hmdes \
		$(top_builddir)/mdes/structure/structure_IMPACT.ho \
		bin/Lmdes_build bin/md_preprocessor bin/md_compiler \
		bin/lmdes2_customizer bin/convert_hmdes
	mkdir -p mdes/Limpact
	Lmdes_build2 $(abs_srcdir)/mdes/Limpact/IMPACT_GEN_BASE.hmdes \
		mdes/Limpact/IMPACT_16G_1BR.lmdes -DISSUE=15 -DBRANCH=0

mdes/Limpact/IMPACT_16G_2BR.lmdes mdes/Limpact/IMPACT_16G_2BR.lmdes2: \
		$(abs_srcdir)/mdes/Limpact/IMPACT_GEN_BASE.hmdes \
		$(top_builddir)/mdes/structure/structure_IMPACT.ho \
		bin/Lmdes_build bin/md_preprocessor bin/md_compiler \
		bin/lmdes2_customizer bin/convert_hmdes
	mkdir -p mdes/Limpact
	Lmdes_build2 $(abs_srcdir)/mdes/Limpact/IMPACT_GEN_BASE.hmdes \
		mdes/Limpact/IMPACT_16G_2BR.lmdes -DISSUE=15 -DBRANCH=1

mdes/Limpact/IMPACT_16G_4BR.lmdes mdes/Limpact/IMPACT_16G_4BR.lmdes2: \
		$(abs_srcdir)/mdes/Limpact/IMPACT_GEN_BASE.hmdes \
		$(top_builddir)/mdes/structure/structure_IMPACT.ho \
		bin/Lmdes_build bin/md_preprocessor bin/md_compiler \
		bin/lmdes2_customizer bin/convert_hmdes
	mkdir -p mdes/Limpact
	Lmdes_build2 $(abs_srcdir)/mdes/Limpact/IMPACT_GEN_BASE.hmdes \
		mdes/Limpact/IMPACT_16G_4BR.lmdes -DISSUE=15 -DBRANCH=3

mdes/Limpact/IMPACT_16G_8BR.lmdes mdes/Limpact/IMPACT_16G_8BR.lmdes2: \
		$(abs_srcdir)/mdes/Limpact/IMPACT_GEN_BASE.hmdes \
		$(top_builddir)/mdes/structure/structure_IMPACT.ho \
		bin/Lmdes_build bin/md_preprocessor bin/md_compiler \
		bin/lmdes2_customizer bin/convert_hmdes
	mkdir -p mdes/Limpact
	Lmdes_build2 $(abs_srcdir)/mdes/Limpact/IMPACT_GEN_BASE.hmdes \
		mdes/Limpact/IMPACT_16G_8BR.lmdes -DISSUE=15 -DBRANCH=7

mdes/Limpact/IMPACT_16G.lmdes mdes/Limpact/IMPACT_16G.lmdes2: \
		$(abs_srcdir)/mdes/Limpact/IMPACT_GEN_BASE.hmdes \
		$(top_builddir)/mdes/structure/structure_IMPACT.ho \
		bin/Lmdes_build bin/md_preprocessor bin/md_compiler \
		bin/lmdes2_customizer bin/convert_hmdes
	mkdir -p mdes/Limpact
	Lmdes_build2 $(abs_srcdir)/mdes/Limpact/IMPACT_GEN_BASE.hmdes \
		mdes/Limpact/IMPACT_16G.lmdes -DISSUE=15 -DBRANCH=15

# restricted percolation models
mdes/Limpact/IMPACT_1R.lmdes mdes/Limpact/IMPACT_1R.lmdes2: \
		$(abs_srcdir)/mdes/Limpact/IMPACT_RES_BASE.hmdes \
		$(top_builddir)/mdes/structure/structure_IMPACT.ho \
		bin/Lmdes_build bin/md_preprocessor bin/md_compiler \
		bin/lmdes2_customizer bin/convert_hmdes
	mkdir -p mdes/Limpact
	Lmdes_build2 $(abs_srcdir)/mdes/Limpact/IMPACT_RES_BASE.hmdes \
		mdes/Limpact/IMPACT_1R.lmdes -DISSUE=0 -DBRANCH=0

mdes/Limpact/IMPACT_2R_1BR.lmdes mdes/Limpact/IMPACT_2R_1BR.lmdes2: \
		$(abs_srcdir)/mdes/Limpact/IMPACT_RES_BASE.hmdes \
		$(top_builddir)/mdes/structure/structure_IMPACT.ho \
		bin/Lmdes_build bin/md_preprocessor bin/md_compiler \
		bin/lmdes2_customizer bin/convert_hmdes
	mkdir -p mdes/Limpact
	Lmdes_build2 $(abs_srcdir)/mdes/Limpact/IMPACT_RES_BASE.hmdes \
		mdes/Limpact/IMPACT_2R_1BR.lmdes -DISSUE=1 -DBRANCH=0

mdes/Limpact/IMPACT_2R.lmdes mdes/Limpact/IMPACT_2R.lmdes2: \
		$(abs_srcdir)/mdes/Limpact/IMPACT_RES_BASE.hmdes \
		$(top_builddir)/mdes/structure/structure_IMPACT.ho \
		bin/Lmdes_build bin/md_preprocessor bin/md_compiler \
		bin/lmdes2_customizer bin/convert_hmdes
	mkdir -p mdes/Limpact
	Lmdes_build2 $(abs_srcdir)/mdes/Limpact/IMPACT_RES_BASE.hmdes \
		mdes/Limpact/IMPACT_2R.lmdes -DISSUE=1 -DBRANCH=1

mdes/Limpact/IMPACT_4R_1BR.lmdes mdes/Limpact/IMPACT_4R_1BR.lmdes2: \
		$(abs_srcdir)/mdes/Limpact/IMPACT_RES_BASE.hmdes \
		$(top_builddir)/mdes/structure/structure_IMPACT.ho \
		bin/Lmdes_build bin/md_preprocessor bin/md_compiler \
		bin/lmdes2_customizer bin/convert_hmdes
	mkdir -p mdes/Limpact
	Lmdes_build2 $(abs_srcdir)/mdes/Limpact/IMPACT_RES_BASE.hmdes \
		mdes/Limpact/IMPACT_4R_1BR.lmdes -DISSUE=3 -DBRANCH=0

mdes/Limpact/IMPACT_4R_2BR.lmdes mdes/Limpact/IMPACT_4R_2BR.lmdes2: \
		$(abs_srcdir)/mdes/Limpact/IMPACT_RES_BASE.hmdes \
		$(top_builddir)/mdes/structure/structure_IMPACT.ho \
		bin/Lmdes_build bin/md_preprocessor bin/md_compiler \
		bin/lmdes2_customizer bin/convert_hmdes
	mkdir -p mdes/Limpact
	Lmdes_build2 $(abs_srcdir)/mdes/Limpact/IMPACT_RES_BASE.hmdes \
		mdes/Limpact/IMPACT_4R_2BR.lmdes -DISSUE=3 -DBRANCH=1

mdes/Limpact/IMPACT_4R.lmdes mdes/Limpact/IMPACT_4R.lmdes2: \
		$(abs_srcdir)/mdes/Limpact/IMPACT_RES_BASE.hmdes \
		$(top_builddir)/mdes/structure/structure_IMPACT.ho \
		bin/Lmdes_build bin/md_preprocessor bin/md_compiler \
		bin/lmdes2_customizer bin/convert_hmdes
	mkdir -p mdes/Limpact
	Lmdes_build2 $(abs_srcdir)/mdes/Limpact/IMPACT_RES_BASE.hmdes \
		mdes/Limpact/IMPACT_4R.lmdes -DISSUE=3 -DBRANCH=3

mdes/Limpact/IMPACT_8R_1BR.lmdes mdes/Limpact/IMPACT_8R_1BR.lmdes2: \
		$(abs_srcdir)/mdes/Limpact/IMPACT_RES_BASE.hmdes \
		$(top_builddir)/mdes/structure/structure_IMPACT.ho \
		bin/Lmdes_build bin/md_preprocessor bin/md_compiler \
		bin/lmdes2_customizer bin/convert_hmdes
	mkdir -p mdes/Limpact
	Lmdes_build2 $(abs_srcdir)/mdes/Limpact/IMPACT_RES_BASE.hmdes \
		mdes/Limpact/IMPACT_8R_1BR.lmdes -DISSUE=7 -DBRANCH=0

mdes/Limpact/IMPACT_8R_2BR.lmdes mdes/Limpact/IMPACT_8R_2BR.lmdes2: \
		$(abs_srcdir)/mdes/Limpact/IMPACT_RES_BASE.hmdes \
		$(top_builddir)/mdes/structure/structure_IMPACT.ho \
		bin/Lmdes_build bin/md_preprocessor bin/md_compiler \
		bin/lmdes2_customizer bin/convert_hmdes
	mkdir -p mdes/Limpact
	Lmdes_build2 $(abs_srcdir)/mdes/Limpact/IMPACT_RES_BASE.hmdes \
		mdes/Limpact/IMPACT_8R_2BR.lmdes -DISSUE=7 -DBRANCH=1

mdes/Limpact/IMPACT_8R_4BR.lmdes mdes/Limpact/IMPACT_8R_4BR.lmdes2: \
		$(abs_srcdir)/mdes/Limpact/IMPACT_RES_BASE.hmdes \
		$(top_builddir)/mdes/structure/structure_IMPACT.ho \
		bin/Lmdes_build bin/md_preprocessor bin/md_compiler \
		bin/lmdes2_customizer bin/convert_hmdes
	mkdir -p mdes/Limpact
	Lmdes_build2 $(abs_srcdir)/mdes/Limpact/IMPACT_RES_BASE.hmdes \
		mdes/Limpact/IMPACT_8R_4BR.lmdes -DISSUE=7 -DBRANCH=3

mdes/Limpact/IMPACT_8R.lmdes mdes/Limpact/IMPACT_8R.lmdes2: \
		$(abs_srcdir)/mdes/Limpact/IMPACT_RES_BASE.hmdes \
		$(top_builddir)/mdes/structure/structure_IMPACT.ho \
		bin/Lmdes_build bin/md_preprocessor bin/md_compiler \
		bin/lmdes2_customizer bin/convert_hmdes
	mkdir -p mdes/Limpact
	Lmdes_build2 $(abs_srcdir)/mdes/Limpact/IMPACT_RES_BASE.hmdes \
		mdes/Limpact/IMPACT_8R.lmdes -DISSUE=7 -DBRANCH=7

mdes/Limpact/IMPACT_16R_1BR.lmdes mdes/Limpact/IMPACT_16R_1BR.lmdes2: \
		$(abs_srcdir)/mdes/Limpact/IMPACT_RES_BASE.hmdes \
		$(top_builddir)/mdes/structure/structure_IMPACT.ho \
		bin/Lmdes_build bin/md_preprocessor bin/md_compiler \
		bin/lmdes2_customizer bin/convert_hmdes
	mkdir -p mdes/Limpact
	Lmdes_build2 $(abs_srcdir)/mdes/Limpact/IMPACT_RES_BASE.hmdes \
		mdes/Limpact/IMPACT_16R_1BR.lmdes -DISSUE=15 -DBRANCH=0

mdes/Limpact/IMPACT_16R_2BR.lmdes mdes/Limpact/IMPACT_16R_2BR.lmdes2: \
		$(abs_srcdir)/mdes/Limpact/IMPACT_RES_BASE.hmdes \
		$(top_builddir)/mdes/structure/structure_IMPACT.ho \
		bin/Lmdes_build bin/md_preprocessor bin/md_compiler \
		bin/lmdes2_customizer bin/convert_hmdes
	mkdir -p mdes/Limpact
	Lmdes_build2 $(abs_srcdir)/mdes/Limpact/IMPACT_RES_BASE.hmdes \
		mdes/Limpact/IMPACT_16R_2BR.lmdes -DISSUE=15 -DBRANCH=1

mdes/Limpact/IMPACT_16R_4BR.lmdes mdes/Limpact/IMPACT_16R_4BR.lmdes2: \
		$(abs_srcdir)/mdes/Limpact/IMPACT_RES_BASE.hmdes \
		$(top_builddir)/mdes/structure/structure_IMPACT.ho \
		bin/Lmdes_build bin/md_preprocessor bin/md_compiler \
		bin/lmdes2_customizer bin/convert_hmdes
	mkdir -p mdes/Limpact
	Lmdes_build2 $(abs_srcdir)/mdes/Limpact/IMPACT_RES_BASE.hmdes \
		mdes/Limpact/IMPACT_16R_4BR.lmdes -DISSUE=15 -DBRANCH=3

mdes/Limpact/IMPACT_16R_8BR.lmdes mdes/Limpact/IMPACT_16R_8BR.lmdes2: \
		$(abs_srcdir)/mdes/Limpact/IMPACT_RES_BASE.hmdes \
		$(top_builddir)/mdes/structure/structure_IMPACT.ho \
		bin/Lmdes_build bin/md_preprocessor bin/md_compiler \
		bin/lmdes2_customizer bin/convert_hmdes
	mkdir -p mdes/Limpact
	Lmdes_build2 $(abs_srcdir)/mdes/Limpact/IMPACT_RES_BASE.hmdes \
		mdes/Limpact/IMPACT_16R_8BR.lmdes -DISSUE=15 -DBRANCH=7

mdes/Limpact/IMPACT_16R.lmdes mdes/Limpact/IMPACT_16R.lmdes2: \
		$(abs_srcdir)/mdes/Limpact/IMPACT_RES_BASE.hmdes \
		$(top_builddir)/mdes/structure/structure_IMPACT.ho \
		bin/Lmdes_build bin/md_preprocessor bin/md_compiler \
		bin/lmdes2_customizer bin/convert_hmdes
	mkdir -p mdes/Limpact
	Lmdes_build2 $(abs_srcdir)/mdes/Limpact/IMPACT_RES_BASE.hmdes \
		mdes/Limpact/IMPACT_16R.lmdes -DISSUE=15 -DBRANCH=15

mdes/Limpact/SS_1G_1BL.lmdes2: \
		$(abs_srcdir)/mdes/Limpact/IMPACT_SIMPLE_TEMPLATE.hmdes2 \
		$(top_builddir)/mdes/structure/structure_IMPACT.ho \
		bin/md_preprocessor bin/md_compiler bin/lmdes2_customizer \
		bin/convert_hmdes
	mkdir -p mdes/Limpact
	hmdesc $(abs_srcdir)/mdes/Limpact/IMPACT_SIMPLE_TEMPLATE.hmdes2 \
		-o mdes/Limpact/SS_1G_1BL.lmdes2 -DWIDTH=1 \
		-DNUM_BRANCHES=1 -DBRANCHES_AT_END=1 -DNON_TRAPPING_OPS=1

mdes/Limpact/SS_1G_1BX.lmdes2: \
		$(abs_srcdir)/mdes/Limpact/IMPACT_SIMPLE_TEMPLATE.hmdes2 \
		$(top_builddir)/mdes/structure/structure_IMPACT.ho \
		bin/md_preprocessor bin/md_compiler bin/lmdes2_customizer \
		bin/convert_hmdes
	mkdir -p mdes/Limpact
	hmdesc $(abs_srcdir)/mdes/Limpact/IMPACT_SIMPLE_TEMPLATE.hmdes2 \
		-o mdes/Limpact/SS_1G_1BX.lmdes2 -DWIDTH=1 \
		-DNUM_BRANCHES=1 -DBRANCHES_AT_END=0 -DNON_TRAPPING_OPS=1

mdes/Limpact/SS_2G_1BL.lmdes2: \
		$(abs_srcdir)/mdes/Limpact/IMPACT_SIMPLE_TEMPLATE.hmdes2 \
		$(top_builddir)/mdes/structure/structure_IMPACT.ho \
		bin/md_preprocessor bin/md_compiler bin/lmdes2_customizer \
		bin/convert_hmdes
	mkdir -p mdes/Limpact
	hmdesc $(abs_srcdir)/mdes/Limpact/IMPACT_SIMPLE_TEMPLATE.hmdes2 \
		-o mdes/Limpact/SS_2G_1BL.lmdes2 -DWIDTH=2 \
		-DNUM_BRANCHES=1 -DBRANCHES_AT_END=1 -DNON_TRAPPING_OPS=1

mdes/Limpact/SS_2G_1BX.lmdes2: \
		$(abs_srcdir)/mdes/Limpact/IMPACT_SIMPLE_TEMPLATE.hmdes2 \
		$(top_builddir)/mdes/structure/structure_IMPACT.ho \
		bin/md_preprocessor bin/md_compiler bin/lmdes2_customizer \
		bin/convert_hmdes
	mkdir -p mdes/Limpact
	hmdesc $(abs_srcdir)/mdes/Limpact/IMPACT_SIMPLE_TEMPLATE.hmdes2 \
		-o mdes/Limpact/SS_2G_1BX.lmdes2 -DWIDTH=2 \
		-DNUM_BRANCHES=1 -DBRANCHES_AT_END=0 -DNON_TRAPPING_OPS=1

mdes/Limpact/SS_2G_2BL.lmdes2: \
		$(abs_srcdir)/mdes/Limpact/IMPACT_SIMPLE_TEMPLATE.hmdes2 \
		$(top_builddir)/mdes/structure/structure_IMPACT.ho \
		bin/md_preprocessor bin/md_compiler bin/lmdes2_customizer \
		bin/convert_hmdes
	mkdir -p mdes/Limpact
	hmdesc $(abs_srcdir)/mdes/Limpact/IMPACT_SIMPLE_TEMPLATE.hmdes2 \
		-o mdes/Limpact/SS_2G_2BL.lmdes2 -DWIDTH=2 \
		-DNUM_BRANCHES=2 -DBRANCHES_AT_END=1 -DNON_TRAPPING_OPS=1

mdes/Limpact/SS_2G_2BX.lmdes2: \
		$(abs_srcdir)/mdes/Limpact/IMPACT_SIMPLE_TEMPLATE.hmdes2 \
		$(top_builddir)/mdes/structure/structure_IMPACT.ho \
		bin/md_preprocessor bin/md_compiler bin/lmdes2_customizer \
		bin/convert_hmdes
	mkdir -p mdes/Limpact
	hmdesc $(abs_srcdir)/mdes/Limpact/IMPACT_SIMPLE_TEMPLATE.hmdes2 \
		-o mdes/Limpact/SS_2G_2BX.lmdes2 -DWIDTH=2 \
		-DNUM_BRANCHES=2 -DBRANCHES_AT_END=0 -DNON_TRAPPING_OPS=1

mdes/Limpact/SS_4G_1BL.lmdes2: \
		$(abs_srcdir)/mdes/Limpact/IMPACT_SIMPLE_TEMPLATE.hmdes2 \
		$(top_builddir)/mdes/structure/structure_IMPACT.ho \
		bin/md_preprocessor bin/md_compiler bin/lmdes2_customizer \
		bin/convert_hmdes
	mkdir -p mdes/Limpact
	hmdesc $(abs_srcdir)/mdes/Limpact/IMPACT_SIMPLE_TEMPLATE.hmdes2 \
		-o mdes/Limpact/SS_4G_1BL.lmdes2 -DWIDTH=4 \
		-DNUM_BRANCHES=1 -DBRANCHES_AT_END=1 -DNON_TRAPPING_OPS=1

mdes/Limpact/SS_4G_1BX.lmdes2: \
		$(abs_srcdir)/mdes/Limpact/IMPACT_SIMPLE_TEMPLATE.hmdes2 \
		$(top_builddir)/mdes/structure/structure_IMPACT.ho \
		bin/md_preprocessor bin/md_compiler bin/lmdes2_customizer \
		bin/convert_hmdes
	mkdir -p mdes/Limpact
	hmdesc $(abs_srcdir)/mdes/Limpact/IMPACT_SIMPLE_TEMPLATE.hmdes2 \
		-o mdes/Limpact/SS_4G_1BX.lmdes2 -DWIDTH=4 \
		-DNUM_BRANCHES=1 -DBRANCHES_AT_END=0 -DNON_TRAPPING_OPS=1

mdes/Limpact/SS_4G_2BL.lmdes2: \
		$(abs_srcdir)/mdes/Limpact/IMPACT_SIMPLE_TEMPLATE.hmdes2 \
		$(top_builddir)/mdes/structure/structure_IMPACT.ho \
		bin/md_preprocessor bin/md_compiler bin/lmdes2_customizer \
		bin/convert_hmdes
	mkdir -p mdes/Limpact
	hmdesc $(abs_srcdir)/mdes/Limpact/IMPACT_SIMPLE_TEMPLATE.hmdes2 \
		-o mdes/Limpact/SS_4G_2BL.lmdes2 -DWIDTH=4 \
		-DNUM_BRANCHES=2 -DBRANCHES_AT_END=1 -DNON_TRAPPING_OPS=1

mdes/Limpact/SS_4G_2BX.lmdes2: \
		$(abs_srcdir)/mdes/Limpact/IMPACT_SIMPLE_TEMPLATE.hmdes2 \
		$(top_builddir)/mdes/structure/structure_IMPACT.ho \
		bin/md_preprocessor bin/md_compiler bin/lmdes2_customizer \
		bin/convert_hmdes
	mkdir -p mdes/Limpact
	hmdesc $(abs_srcdir)/mdes/Limpact/IMPACT_SIMPLE_TEMPLATE.hmdes2 \
		-o mdes/Limpact/SS_4G_2BX.lmdes2 -DWIDTH=4 \
		-DNUM_BRANCHES=2 -DBRANCHES_AT_END=0 -DNON_TRAPPING_OPS=1

mdes/Limpact/SS_4G_4BL.lmdes2: \
		$(abs_srcdir)/mdes/Limpact/IMPACT_SIMPLE_TEMPLATE.hmdes2 \
		$(top_builddir)/mdes/structure/structure_IMPACT.ho \
		bin/md_preprocessor bin/md_compiler bin/lmdes2_customizer \
		bin/convert_hmdes
	mkdir -p mdes/Limpact
	hmdesc $(abs_srcdir)/mdes/Limpact/IMPACT_SIMPLE_TEMPLATE.hmdes2 \
		-o mdes/Limpact/SS_4G_4BL.lmdes2 -DWIDTH=4 \
		-DNUM_BRANCHES=4 -DBRANCHES_AT_END=1 -DNON_TRAPPING_OPS=1

mdes/Limpact/SS_4G_4BX.lmdes2: \
		$(abs_srcdir)/mdes/Limpact/IMPACT_SIMPLE_TEMPLATE.hmdes2 \
		$(top_builddir)/mdes/structure/structure_IMPACT.ho \
		bin/md_preprocessor bin/md_compiler bin/lmdes2_customizer \
		bin/convert_hmdes
	mkdir -p mdes/Limpact
	hmdesc $(abs_srcdir)/mdes/Limpact/IMPACT_SIMPLE_TEMPLATE.hmdes2 \
		-o mdes/Limpact/SS_4G_4BX.lmdes2 -DWIDTH=4 \
		-DNUM_BRANCHES=4 -DBRANCHES_AT_END=0 -DNON_TRAPPING_OPS=1

mdes/Limpact/SS_8G_1BL.lmdes2: \
		$(abs_srcdir)/mdes/Limpact/IMPACT_SIMPLE_TEMPLATE.hmdes2 \
		$(top_builddir)/mdes/structure/structure_IMPACT.ho \
		bin/md_preprocessor bin/md_compiler bin/lmdes2_customizer \
		bin/convert_hmdes
	mkdir -p mdes/Limpact
	hmdesc $(abs_srcdir)/mdes/Limpact/IMPACT_SIMPLE_TEMPLATE.hmdes2 \
		-o mdes/Limpact/SS_8G_1BL.lmdes2 -DWIDTH=8 \
		-DNUM_BRANCHES=1 -DBRANCHES_AT_END=1 -DNON_TRAPPING_OPS=1

mdes/Limpact/SS_8G_1BX.lmdes2: \
		$(abs_srcdir)/mdes/Limpact/IMPACT_SIMPLE_TEMPLATE.hmdes2 \
		$(top_builddir)/mdes/structure/structure_IMPACT.ho \
		bin/md_preprocessor bin/md_compiler bin/lmdes2_customizer \
		bin/convert_hmdes
	mkdir -p mdes/Limpact
	hmdesc $(abs_srcdir)/mdes/Limpact/IMPACT_SIMPLE_TEMPLATE.hmdes2 \
		-o mdes/Limpact/SS_8G_1BX.lmdes2 -DWIDTH=8 \
		-DNUM_BRANCHES=1 -DBRANCHES_AT_END=0 -DNON_TRAPPING_OPS=1

mdes/Limpact/SS_8G_2BL.lmdes2: \
		$(abs_srcdir)/mdes/Limpact/IMPACT_SIMPLE_TEMPLATE.hmdes2 \
		$(top_builddir)/mdes/structure/structure_IMPACT.ho \
		bin/md_preprocessor bin/md_compiler bin/lmdes2_customizer \
		bin/convert_hmdes
	mkdir -p mdes/Limpact
	hmdesc $(abs_srcdir)/mdes/Limpact/IMPACT_SIMPLE_TEMPLATE.hmdes2 \
		-o mdes/Limpact/SS_8G_2BL.lmdes2 -DWIDTH=8 \
		-DNUM_BRANCHES=2 -DBRANCHES_AT_END=1 -DNON_TRAPPING_OPS=1

mdes/Limpact/SS_8G_2BX.lmdes2: \
		$(abs_srcdir)/mdes/Limpact/IMPACT_SIMPLE_TEMPLATE.hmdes2 \
		$(top_builddir)/mdes/structure/structure_IMPACT.ho \
		bin/md_preprocessor bin/md_compiler bin/lmdes2_customizer \
		bin/convert_hmdes
	mkdir -p mdes/Limpact
	hmdesc $(abs_srcdir)/mdes/Limpact/IMPACT_SIMPLE_TEMPLATE.hmdes2 \
		-o mdes/Limpact/SS_8G_2BX.lmdes2 -DWIDTH=8 \
		-DNUM_BRANCHES=2 -DBRANCHES_AT_END=0 -DNON_TRAPPING_OPS=1

mdes/Limpact/SS_8G_4BL.lmdes2: \
		$(abs_srcdir)/mdes/Limpact/IMPACT_SIMPLE_TEMPLATE.hmdes2 \
		$(top_builddir)/mdes/structure/structure_IMPACT.ho \
		bin/md_preprocessor bin/md_compiler bin/lmdes2_customizer \
		bin/convert_hmdes
	mkdir -p mdes/Limpact
	hmdesc $(abs_srcdir)/mdes/Limpact/IMPACT_SIMPLE_TEMPLATE.hmdes2 \
		-o mdes/Limpact/SS_8G_4BL.lmdes2 -DWIDTH=8 \
		-DNUM_BRANCHES=4 -DBRANCHES_AT_END=1 -DNON_TRAPPING_OPS=1

mdes/Limpact/SS_8G_4BX.lmdes2: \
		$(abs_srcdir)/mdes/Limpact/IMPACT_SIMPLE_TEMPLATE.hmdes2 \
		$(top_builddir)/mdes/structure/structure_IMPACT.ho \
		bin/md_preprocessor bin/md_compiler bin/lmdes2_customizer \
		bin/convert_hmdes
	mkdir -p mdes/Limpact
	hmdesc $(abs_srcdir)/mdes/Limpact/IMPACT_SIMPLE_TEMPLATE.hmdes2 \
		-o mdes/Limpact/SS_8G_4BX.lmdes2 -DWIDTH=8 \
		-DNUM_BRANCHES=4 -DBRANCHES_AT_END=0 -DNON_TRAPPING_OPS=1

mdes/Limpact/SS_8G_8BL.lmdes2: \
		$(abs_srcdir)/mdes/Limpact/IMPACT_SIMPLE_TEMPLATE.hmdes2 \
		$(top_builddir)/mdes/structure/structure_IMPACT.ho \
		bin/md_preprocessor bin/md_compiler bin/lmdes2_customizer \
		bin/convert_hmdes
	mkdir -p mdes/Limpact
	hmdesc $(abs_srcdir)/mdes/Limpact/IMPACT_SIMPLE_TEMPLATE.hmdes2 \
		-o mdes/Limpact/SS_8G_8BL.lmdes2 -DWIDTH=8 \
		-DNUM_BRANCHES=8 -DBRANCHES_AT_END=1 -DNON_TRAPPING_OPS=1

mdes/Limpact/SS_8G_8BX.lmdes2: \
		$(abs_srcdir)/mdes/Limpact/IMPACT_SIMPLE_TEMPLATE.hmdes2 \
		$(top_builddir)/mdes/structure/structure_IMPACT.ho \
		bin/md_preprocessor bin/md_compiler bin/lmdes2_customizer \
		bin/convert_hmdes
	mkdir -p mdes/Limpact
	hmdesc $(abs_srcdir)/mdes/Limpact/IMPACT_SIMPLE_TEMPLATE.hmdes2 \
		-o mdes/Limpact/SS_8G_8BX.lmdes2 -DWIDTH=8 \
		-DNUM_BRANCHES=8 -DBRANCHES_AT_END=0 -DNON_TRAPPING_OPS=1

mdes/Limpact/SS_16G_1BL.lmdes2: \
		$(abs_srcdir)/mdes/Limpact/IMPACT_SIMPLE_TEMPLATE.hmdes2 \
		$(top_builddir)/mdes/structure/structure_IMPACT.ho \
		bin/md_preprocessor bin/md_compiler bin/lmdes2_customizer \
		bin/convert_hmdes
	mkdir -p mdes/Limpact
	hmdesc $(abs_srcdir)/mdes/Limpact/IMPACT_SIMPLE_TEMPLATE.hmdes2 \
		-o mdes/Limpact/SS_16G_1BL.lmdes2 -DWIDTH=16 \
		-DNUM_BRANCHES=1 -DBRANCHES_AT_END=1 -DNON_TRAPPING_OPS=1

mdes/Limpact/SS_16G_1BX.lmdes2: \
		$(abs_srcdir)/mdes/Limpact/IMPACT_SIMPLE_TEMPLATE.hmdes2 \
		$(top_builddir)/mdes/structure/structure_IMPACT.ho \
		bin/md_preprocessor bin/md_compiler bin/lmdes2_customizer \
		bin/convert_hmdes
	mkdir -p mdes/Limpact
	hmdesc $(abs_srcdir)/mdes/Limpact/IMPACT_SIMPLE_TEMPLATE.hmdes2 \
		-o mdes/Limpact/SS_16G_1BX.lmdes2 -DWIDTH=16 \
		-DNUM_BRANCHES=1 -DBRANCHES_AT_END=0 -DNON_TRAPPING_OPS=1

mdes/Limpact/SS_16G_2BL.lmdes2: \
		$(abs_srcdir)/mdes/Limpact/IMPACT_SIMPLE_TEMPLATE.hmdes2 \
		$(top_builddir)/mdes/structure/structure_IMPACT.ho \
		bin/md_preprocessor bin/md_compiler bin/lmdes2_customizer \
		bin/convert_hmdes
	mkdir -p mdes/Limpact
	hmdesc $(abs_srcdir)/mdes/Limpact/IMPACT_SIMPLE_TEMPLATE.hmdes2 \
		-o mdes/Limpact/SS_16G_2BL.lmdes2 -DWIDTH=16 \
		-DNUM_BRANCHES=2 -DBRANCHES_AT_END=1 -DNON_TRAPPING_OPS=1

mdes/Limpact/SS_16G_2BX.lmdes2: \
		$(abs_srcdir)/mdes/Limpact/IMPACT_SIMPLE_TEMPLATE.hmdes2 \
		$(top_builddir)/mdes/structure/structure_IMPACT.ho \
		bin/md_preprocessor bin/md_compiler bin/lmdes2_customizer \
		bin/convert_hmdes
	mkdir -p mdes/Limpact

	hmdesc $(abs_srcdir)/mdes/Limpact/IMPACT_SIMPLE_TEMPLATE.hmdes2 \
		-o mdes/Limpact/SS_16G_2BX.lmdes2 -DWIDTH=16 \
		-DNUM_BRANCHES=2 -DBRANCHES_AT_END=0 -DNON_TRAPPING_OPS=1

mdes/Limpact/SS_16G_4BL.lmdes2: \
		$(abs_srcdir)/mdes/Limpact/IMPACT_SIMPLE_TEMPLATE.hmdes2 \
		$(top_builddir)/mdes/structure/structure_IMPACT.ho \
		bin/md_preprocessor bin/md_compiler bin/lmdes2_customizer \
		bin/convert_hmdes
	mkdir -p mdes/Limpact
	hmdesc $(abs_srcdir)/mdes/Limpact/IMPACT_SIMPLE_TEMPLATE.hmdes2 \
		-o mdes/Limpact/SS_16G_4BL.lmdes2 -DWIDTH=16 \
		-DNUM_BRANCHES=4 -DBRANCHES_AT_END=1 -DNON_TRAPPING_OPS=1

mdes/Limpact/SS_16G_4BX.lmdes2: \
		$(abs_srcdir)/mdes/Limpact/IMPACT_SIMPLE_TEMPLATE.hmdes2 \
		$(top_builddir)/mdes/structure/structure_IMPACT.ho \
		bin/md_preprocessor bin/md_compiler bin/lmdes2_customizer \
		bin/convert_hmdes
	mkdir -p mdes/Limpact
	hmdesc $(abs_srcdir)/mdes/Limpact/IMPACT_SIMPLE_TEMPLATE.hmdes2 \
		-o mdes/Limpact/SS_16G_4BX.lmdes2 -DWIDTH=16 \
		-DNUM_BRANCHES=4 -DBRANCHES_AT_END=0 -DNON_TRAPPING_OPS=1

mdes/Limpact/SS_16G_8BL.lmdes2: \
		$(abs_srcdir)/mdes/Limpact/IMPACT_SIMPLE_TEMPLATE.hmdes2 \
		$(top_builddir)/mdes/structure/structure_IMPACT.ho \
		bin/md_preprocessor bin/md_compiler bin/lmdes2_customizer \
		bin/convert_hmdes
	mkdir -p mdes/Limpact
	hmdesc $(abs_srcdir)/mdes/Limpact/IMPACT_SIMPLE_TEMPLATE.hmdes2 \
		-o mdes/Limpact/SS_16G_8BL.lmdes2 -DWIDTH=16 \
		-DNUM_BRANCHES=8 -DBRANCHES_AT_END=1 -DNON_TRAPPING_OPS=1

mdes/Limpact/SS_16G_8BX.lmdes2: \
		$(abs_srcdir)/mdes/Limpact/IMPACT_SIMPLE_TEMPLATE.hmdes2 \
		$(top_builddir)/mdes/structure/structure_IMPACT.ho \
		bin/md_preprocessor bin/md_compiler bin/lmdes2_customizer \
		bin/convert_hmdes
	mkdir -p mdes/Limpact
	hmdesc $(abs_srcdir)/mdes/Limpact/IMPACT_SIMPLE_TEMPLATE.hmdes2 \
		-o mdes/Limpact/SS_16G_8BX.lmdes2 -DWIDTH=16 \
		-DNUM_BRANCHES=8 -DBRANCHES_AT_END=0 -DNON_TRAPPING_OPS=1

mdes/Limpact/SS_16G_16BL.lmdes2: \
		$(abs_srcdir)/mdes/Limpact/IMPACT_SIMPLE_TEMPLATE.hmdes2 \
		$(top_builddir)/mdes/structure/structure_IMPACT.ho \
		bin/md_preprocessor bin/md_compiler bin/lmdes2_customizer \
		bin/convert_hmdes
	mkdir -p mdes/Limpact
	hmdesc $(abs_srcdir)/mdes/Limpact/IMPACT_SIMPLE_TEMPLATE.hmdes2 \
		-o mdes/Limpact/SS_16G_16BL.lmdes2 -DWIDTH=16 \
		-DNUM_BRANCHES=16 -DBRANCHES_AT_END=1 -DNON_TRAPPING_OPS=1

mdes/Limpact/SS_16G_16BX.lmdes2: \
		$(abs_srcdir)/mdes/Limpact/IMPACT_SIMPLE_TEMPLATE.hmdes2 \
		$(top_builddir)/mdes/structure/structure_IMPACT.ho \
		bin/md_preprocessor bin/md_compiler bin/lmdes2_customizer \
		bin/convert_hmdes
	mkdir -p mdes/Limpact
	hmdesc $(abs_srcdir)/mdes/Limpact/IMPACT_SIMPLE_TEMPLATE.hmdes2 \
		-o mdes/Limpact/SS_16G_16BX.lmdes2 -DWIDTH=16 \
		-DNUM_BRANCHES=16 -DBRANCHES_AT_END=0 -DNON_TRAPPING_OPS=1

mdes/Limpact/SS_1R_1BL.lmdes2: \
		$(abs_srcdir)/mdes/Limpact/IMPACT_SIMPLE_TEMPLATE.hmdes2 \
		$(top_builddir)/mdes/structure/structure_IMPACT.ho \
		bin/md_preprocessor bin/md_compiler bin/lmdes2_customizer \
		bin/convert_hmdes
	mkdir -p mdes/Limpact
	hmdesc $(abs_srcdir)/mdes/Limpact/IMPACT_SIMPLE_TEMPLATE.hmdes2 \
		-o mdes/Limpact/SS_1R_1BL.lmdes2 -DWIDTH=1 \
		-DNUM_BRANCHES=1 -DBRANCHES_AT_END=1 -DNON_TRAPPING_OPS=0

mdes/Limpact/SS_1R_1BX.lmdes2: \
		$(abs_srcdir)/mdes/Limpact/IMPACT_SIMPLE_TEMPLATE.hmdes2 \
		$(top_builddir)/mdes/structure/structure_IMPACT.ho \
		bin/md_preprocessor bin/md_compiler bin/lmdes2_customizer \
		bin/convert_hmdes
	mkdir -p mdes/Limpact
	hmdesc $(abs_srcdir)/mdes/Limpact/IMPACT_SIMPLE_TEMPLATE.hmdes2 \
		-o mdes/Limpact/SS_1R_1BX.lmdes2 -DWIDTH=1 \
		-DNUM_BRANCHES=1 -DBRANCHES_AT_END=0 -DNON_TRAPPING_OPS=0

mdes/Limpact/SS_2R_1BL.lmdes2: \
		$(abs_srcdir)/mdes/Limpact/IMPACT_SIMPLE_TEMPLATE.hmdes2 \
		$(top_builddir)/mdes/structure/structure_IMPACT.ho \
		bin/md_preprocessor bin/md_compiler bin/lmdes2_customizer \
		bin/convert_hmdes
	mkdir -p mdes/Limpact
	hmdesc $(abs_srcdir)/mdes/Limpact/IMPACT_SIMPLE_TEMPLATE.hmdes2 \
		-o mdes/Limpact/SS_2R_1BL.lmdes2 -DWIDTH=2 \
		-DNUM_BRANCHES=1 -DBRANCHES_AT_END=1 -DNON_TRAPPING_OPS=0

mdes/Limpact/SS_2R_1BX.lmdes2: \
		$(abs_srcdir)/mdes/Limpact/IMPACT_SIMPLE_TEMPLATE.hmdes2 \
		$(top_builddir)/mdes/structure/structure_IMPACT.ho \
		bin/md_preprocessor bin/md_compiler bin/lmdes2_customizer \
		bin/convert_hmdes
	mkdir -p mdes/Limpact
	hmdesc $(abs_srcdir)/mdes/Limpact/IMPACT_SIMPLE_TEMPLATE.hmdes2 \
		-o mdes/Limpact/SS_2R_1BX.lmdes2 -DWIDTH=2 \
		-DNUM_BRANCHES=1 -DBRANCHES_AT_END=0 -DNON_TRAPPING_OPS=0

mdes/Limpact/SS_2R_2BL.lmdes2: \
		$(abs_srcdir)/mdes/Limpact/IMPACT_SIMPLE_TEMPLATE.hmdes2 \
		$(top_builddir)/mdes/structure/structure_IMPACT.ho \
		bin/md_preprocessor bin/md_compiler bin/lmdes2_customizer \
		bin/convert_hmdes
	mkdir -p mdes/Limpact
	hmdesc $(abs_srcdir)/mdes/Limpact/IMPACT_SIMPLE_TEMPLATE.hmdes2 \
		-o mdes/Limpact/SS_2R_2BL.lmdes2 -DWIDTH=2 \
		-DNUM_BRANCHES=2 -DBRANCHES_AT_END=1 -DNON_TRAPPING_OPS=0

mdes/Limpact/SS_2R_2BX.lmdes2: \
		$(abs_srcdir)/mdes/Limpact/IMPACT_SIMPLE_TEMPLATE.hmdes2 \
		$(top_builddir)/mdes/structure/structure_IMPACT.ho \
		bin/md_preprocessor bin/md_compiler bin/lmdes2_customizer \
		bin/convert_hmdes
	mkdir -p mdes/Limpact

	hmdesc $(abs_srcdir)/mdes/Limpact/IMPACT_SIMPLE_TEMPLATE.hmdes2 \
		-o mdes/Limpact/SS_2R_2BX.lmdes2 -DWIDTH=2 \
		-DNUM_BRANCHES=2 -DBRANCHES_AT_END=0 -DNON_TRAPPING_OPS=0

mdes/Limpact/SS_4R_1BL.lmdes2: \
		$(abs_srcdir)/mdes/Limpact/IMPACT_SIMPLE_TEMPLATE.hmdes2 \
		$(top_builddir)/mdes/structure/structure_IMPACT.ho \
		bin/md_preprocessor bin/md_compiler bin/lmdes2_customizer \
		bin/convert_hmdes
	mkdir -p mdes/Limpact
	hmdesc $(abs_srcdir)/mdes/Limpact/IMPACT_SIMPLE_TEMPLATE.hmdes2 \
		-o mdes/Limpact/SS_4R_1BL.lmdes2 -DWIDTH=4 \
		-DNUM_BRANCHES=1 -DBRANCHES_AT_END=1 -DNON_TRAPPING_OPS=0

mdes/Limpact/SS_4R_1BX.lmdes2: \
		$(abs_srcdir)/mdes/Limpact/IMPACT_SIMPLE_TEMPLATE.hmdes2 \
		$(top_builddir)/mdes/structure/structure_IMPACT.ho \
		bin/md_preprocessor bin/md_compiler bin/lmdes2_customizer \
		bin/convert_hmdes
	mkdir -p mdes/Limpact
	hmdesc $(abs_srcdir)/mdes/Limpact/IMPACT_SIMPLE_TEMPLATE.hmdes2 \
		-o mdes/Limpact/SS_4R_1BX.lmdes2 -DWIDTH=4 \
		-DNUM_BRANCHES=1 -DBRANCHES_AT_END=0 -DNON_TRAPPING_OPS=0

mdes/Limpact/SS_4R_2BL.lmdes2: \
		$(abs_srcdir)/mdes/Limpact/IMPACT_SIMPLE_TEMPLATE.hmdes2 \
		$(top_builddir)/mdes/structure/structure_IMPACT.ho \
		bin/md_preprocessor bin/md_compiler bin/lmdes2_customizer \
		bin/convert_hmdes
	mkdir -p mdes/Limpact
	hmdesc $(abs_srcdir)/mdes/Limpact/IMPACT_SIMPLE_TEMPLATE.hmdes2 \
		-o mdes/Limpact/SS_4R_2BL.lmdes2 -DWIDTH=4 \
		-DNUM_BRANCHES=2 -DBRANCHES_AT_END=1 -DNON_TRAPPING_OPS=0

mdes/Limpact/SS_4R_2BX.lmdes2: \
		$(abs_srcdir)/mdes/Limpact/IMPACT_SIMPLE_TEMPLATE.hmdes2 \
		$(top_builddir)/mdes/structure/structure_IMPACT.ho \
		bin/md_preprocessor bin/md_compiler bin/lmdes2_customizer \
		bin/convert_hmdes
	mkdir -p mdes/Limpact
	hmdesc $(abs_srcdir)/mdes/Limpact/IMPACT_SIMPLE_TEMPLATE.hmdes2 \
		-o mdes/Limpact/SS_4R_2BX.lmdes2 -DWIDTH=4 \
		-DNUM_BRANCHES=2 -DBRANCHES_AT_END=0 -DNON_TRAPPING_OPS=0

mdes/Limpact/SS_4R_4BL.lmdes2: \
		$(abs_srcdir)/mdes/Limpact/IMPACT_SIMPLE_TEMPLATE.hmdes2 \
		$(top_builddir)/mdes/structure/structure_IMPACT.ho \
		bin/md_preprocessor bin/md_compiler bin/lmdes2_customizer \
		bin/convert_hmdes
	mkdir -p mdes/Limpact
	hmdesc $(abs_srcdir)/mdes/Limpact/IMPACT_SIMPLE_TEMPLATE.hmdes2 \
		-o mdes/Limpact/SS_4R_4BL.lmdes2 -DWIDTH=4 \
		-DNUM_BRANCHES=4 -DBRANCHES_AT_END=1 -DNON_TRAPPING_OPS=0

mdes/Limpact/SS_4R_4BX.lmdes2: \
		$(abs_srcdir)/mdes/Limpact/IMPACT_SIMPLE_TEMPLATE.hmdes2 \
		$(top_builddir)/mdes/structure/structure_IMPACT.ho \
		bin/md_preprocessor bin/md_compiler bin/lmdes2_customizer \
		bin/convert_hmdes
	mkdir -p mdes/Limpact
	hmdesc $(abs_srcdir)/mdes/Limpact/IMPACT_SIMPLE_TEMPLATE.hmdes2 \
		-o mdes/Limpact/SS_4R_4BX.lmdes2 -DWIDTH=4 \
		-DNUM_BRANCHES=4 -DBRANCHES_AT_END=0 -DNON_TRAPPING_OPS=0

mdes/Limpact/SS_8R_1BL.lmdes2: \
		$(abs_srcdir)/mdes/Limpact/IMPACT_SIMPLE_TEMPLATE.hmdes2 \
		$(top_builddir)/mdes/structure/structure_IMPACT.ho \
		bin/md_preprocessor bin/md_compiler bin/lmdes2_customizer \
		bin/convert_hmdes
	mkdir -p mdes/Limpact
	hmdesc $(abs_srcdir)/mdes/Limpact/IMPACT_SIMPLE_TEMPLATE.hmdes2 \
		-o mdes/Limpact/SS_8R_1BL.lmdes2 -DWIDTH=8 \
		-DNUM_BRANCHES=1 -DBRANCHES_AT_END=1 -DNON_TRAPPING_OPS=0

mdes/Limpact/SS_8R_1BX.lmdes2: \
		$(abs_srcdir)/mdes/Limpact/IMPACT_SIMPLE_TEMPLATE.hmdes2 \
		$(top_builddir)/mdes/structure/structure_IMPACT.ho \
		bin/md_preprocessor bin/md_compiler bin/lmdes2_customizer \
		bin/convert_hmdes
	mkdir -p mdes/Limpact
	hmdesc $(abs_srcdir)/mdes/Limpact/IMPACT_SIMPLE_TEMPLATE.hmdes2 \
		-o mdes/Limpact/SS_8R_1BX.lmdes2 -DWIDTH=8 \
		-DNUM_BRANCHES=1 -DBRANCHES_AT_END=0 -DNON_TRAPPING_OPS=0

mdes/Limpact/SS_8R_2BL.lmdes2: \
		$(abs_srcdir)/mdes/Limpact/IMPACT_SIMPLE_TEMPLATE.hmdes2 \
		$(top_builddir)/mdes/structure/structure_IMPACT.ho \
		bin/md_preprocessor bin/md_compiler bin/lmdes2_customizer \
		bin/convert_hmdes
	mkdir -p mdes/Limpact
	hmdesc $(abs_srcdir)/mdes/Limpact/IMPACT_SIMPLE_TEMPLATE.hmdes2 \
		-o mdes/Limpact/SS_8R_2BL.lmdes2 -DWIDTH=8 \
		-DNUM_BRANCHES=2 -DBRANCHES_AT_END=1 -DNON_TRAPPING_OPS=0

mdes/Limpact/SS_8R_2BX.lmdes2: \
		$(abs_srcdir)/mdes/Limpact/IMPACT_SIMPLE_TEMPLATE.hmdes2 \
		$(top_builddir)/mdes/structure/structure_IMPACT.ho \
		bin/md_preprocessor bin/md_compiler bin/lmdes2_customizer \
		bin/convert_hmdes
	mkdir -p mdes/Limpact
	hmdesc $(abs_srcdir)/mdes/Limpact/IMPACT_SIMPLE_TEMPLATE.hmdes2 \
		-o mdes/Limpact/SS_8R_2BX.lmdes2 -DWIDTH=8 \
		-DNUM_BRANCHES=2 -DBRANCHES_AT_END=0 -DNON_TRAPPING_OPS=0

mdes/Limpact/SS_8R_4BL.lmdes2: \
		$(abs_srcdir)/mdes/Limpact/IMPACT_SIMPLE_TEMPLATE.hmdes2 \
		$(top_builddir)/mdes/structure/structure_IMPACT.ho \
		bin/md_preprocessor bin/md_compiler bin/lmdes2_customizer \
		bin/convert_hmdes
	mkdir -p mdes/Limpact
	hmdesc $(abs_srcdir)/mdes/Limpact/IMPACT_SIMPLE_TEMPLATE.hmdes2 \
		-o mdes/Limpact/SS_8R_4BL.lmdes2 -DWIDTH=8 \
		-DNUM_BRANCHES=4 -DBRANCHES_AT_END=1 -DNON_TRAPPING_OPS=0

mdes/Limpact/SS_8R_4BX.lmdes2: \
		$(abs_srcdir)/mdes/Limpact/IMPACT_SIMPLE_TEMPLATE.hmdes2 \
		$(top_builddir)/mdes/structure/structure_IMPACT.ho \
		bin/md_preprocessor bin/md_compiler bin/lmdes2_customizer \
		bin/convert_hmdes
	mkdir -p mdes/Limpact
	hmdesc $(abs_srcdir)/mdes/Limpact/IMPACT_SIMPLE_TEMPLATE.hmdes2 \
		-o mdes/Limpact/SS_8R_4BX.lmdes2 -DWIDTH=8 \
		-DNUM_BRANCHES=4 -DBRANCHES_AT_END=0 -DNON_TRAPPING_OPS=0

mdes/Limpact/SS_8R_8BL.lmdes2: \
		$(abs_srcdir)/mdes/Limpact/IMPACT_SIMPLE_TEMPLATE.hmdes2 \
		$(top_builddir)/mdes/structure/structure_IMPACT.ho \
		bin/md_preprocessor bin/md_compiler bin/lmdes2_customizer \
		bin/convert_hmdes
	mkdir -p mdes/Limpact
	hmdesc $(abs_srcdir)/mdes/Limpact/IMPACT_SIMPLE_TEMPLATE.hmdes2 \
		-o mdes/Limpact/SS_8R_8BL.lmdes2 -DWIDTH=8 \
		-DNUM_BRANCHES=8 -DBRANCHES_AT_END=1 -DNON_TRAPPING_OPS=0

mdes/Limpact/SS_8R_8BX.lmdes2: \
		$(abs_srcdir)/mdes/Limpact/IMPACT_SIMPLE_TEMPLATE.hmdes2 \
		$(top_builddir)/mdes/structure/structure_IMPACT.ho \
		bin/md_preprocessor bin/md_compiler bin/lmdes2_customizer \
		bin/convert_hmdes
	mkdir -p mdes/Limpact
	hmdesc $(abs_srcdir)/mdes/Limpact/IMPACT_SIMPLE_TEMPLATE.hmdes2 \
		-o mdes/Limpact/SS_8R_8BX.lmdes2 -DWIDTH=8 \
		-DNUM_BRANCHES=8 -DBRANCHES_AT_END=0 -DNON_TRAPPING_OPS=0

mdes/Limpact/SS_16R_1BL.lmdes2: \
		$(abs_srcdir)/mdes/Limpact/IMPACT_SIMPLE_TEMPLATE.hmdes2 \
		$(top_builddir)/mdes/structure/structure_IMPACT.ho \
		bin/md_preprocessor bin/md_compiler bin/lmdes2_customizer \
		bin/convert_hmdes
	mkdir -p mdes/Limpact
	hmdesc $(abs_srcdir)/mdes/Limpact/IMPACT_SIMPLE_TEMPLATE.hmdes2 \
		-o mdes/Limpact/SS_16R_1BL.lmdes2 -DWIDTH=16 \
		-DNUM_BRANCHES=1 -DBRANCHES_AT_END=1 -DNON_TRAPPING_OPS=0

mdes/Limpact/SS_16R_1BX.lmdes2: \
		$(abs_srcdir)/mdes/Limpact/IMPACT_SIMPLE_TEMPLATE.hmdes2 \
		$(top_builddir)/mdes/structure/structure_IMPACT.ho \
		bin/md_preprocessor bin/md_compiler bin/lmdes2_customizer \
		bin/convert_hmdes
	mkdir -p mdes/Limpact
	hmdesc $(abs_srcdir)/mdes/Limpact/IMPACT_SIMPLE_TEMPLATE.hmdes2 \
		-o mdes/Limpact/SS_16R_1BX.lmdes2 -DWIDTH=16 \
		-DNUM_BRANCHES=1 -DBRANCHES_AT_END=0 -DNON_TRAPPING_OPS=0

mdes/Limpact/SS_16R_2BL.lmdes2: \
		$(abs_srcdir)/mdes/Limpact/IMPACT_SIMPLE_TEMPLATE.hmdes2 \
		$(top_builddir)/mdes/structure/structure_IMPACT.ho \
		bin/md_preprocessor bin/md_compiler bin/lmdes2_customizer \
		bin/convert_hmdes
	mkdir -p mdes/Limpact
	hmdesc $(abs_srcdir)/mdes/Limpact/IMPACT_SIMPLE_TEMPLATE.hmdes2 \
		-o mdes/Limpact/SS_16R_2BL.lmdes2 -DWIDTH=16 \
		-DNUM_BRANCHES=2 -DBRANCHES_AT_END=1 -DNON_TRAPPING_OPS=0

mdes/Limpact/SS_16R_2BX.lmdes2: \
		$(abs_srcdir)/mdes/Limpact/IMPACT_SIMPLE_TEMPLATE.hmdes2 \
		$(top_builddir)/mdes/structure/structure_IMPACT.ho \
		bin/md_preprocessor bin/md_compiler bin/lmdes2_customizer \
		bin/convert_hmdes
	mkdir -p mdes/Limpact
	hmdesc $(abs_srcdir)/mdes/Limpact/IMPACT_SIMPLE_TEMPLATE.hmdes2 \
		-o mdes/Limpact/SS_16R_2BX.lmdes2 -DWIDTH=16 \
		-DNUM_BRANCHES=2 -DBRANCHES_AT_END=0 -DNON_TRAPPING_OPS=0

mdes/Limpact/SS_16R_4BL.lmdes2: \
		$(abs_srcdir)/mdes/Limpact/IMPACT_SIMPLE_TEMPLATE.hmdes2 \
		$(top_builddir)/mdes/structure/structure_IMPACT.ho \
		bin/md_preprocessor bin/md_compiler bin/lmdes2_customizer \
		bin/convert_hmdes
	mkdir -p mdes/Limpact
	hmdesc $(abs_srcdir)/mdes/Limpact/IMPACT_SIMPLE_TEMPLATE.hmdes2 \
		-o mdes/Limpact/SS_16R_4BL.lmdes2 -DWIDTH=16 \
		-DNUM_BRANCHES=4 -DBRANCHES_AT_END=1 -DNON_TRAPPING_OPS=0

mdes/Limpact/SS_16R_4BX.lmdes2: \
		$(abs_srcdir)/mdes/Limpact/IMPACT_SIMPLE_TEMPLATE.hmdes2 \
		$(top_builddir)/mdes/structure/structure_IMPACT.ho \
		bin/md_preprocessor bin/md_compiler bin/lmdes2_customizer \
		bin/convert_hmdes
	mkdir -p mdes/Limpact
	hmdesc $(abs_srcdir)/mdes/Limpact/IMPACT_SIMPLE_TEMPLATE.hmdes2 \
		-o mdes/Limpact/SS_16R_4BX.lmdes2 -DWIDTH=16 \
		-DNUM_BRANCHES=4 -DBRANCHES_AT_END=0 -DNON_TRAPPING_OPS=0

mdes/Limpact/SS_16R_8BL.lmdes2: \
		$(abs_srcdir)/mdes/Limpact/IMPACT_SIMPLE_TEMPLATE.hmdes2 \
		$(top_builddir)/mdes/structure/structure_IMPACT.ho \
		bin/md_preprocessor bin/md_compiler bin/lmdes2_customizer \
		bin/convert_hmdes
	mkdir -p mdes/Limpact
	hmdesc $(abs_srcdir)/mdes/Limpact/IMPACT_SIMPLE_TEMPLATE.hmdes2 \
		-o mdes/Limpact/SS_16R_8BL.lmdes2 -DWIDTH=16 \
		-DNUM_BRANCHES=8 -DBRANCHES_AT_END=1 -DNON_TRAPPING_OPS=0

mdes/Limpact/SS_16R_8BX.lmdes2: \
		$(abs_srcdir)/mdes/Limpact/IMPACT_SIMPLE_TEMPLATE.hmdes2 \
		$(top_builddir)/mdes/structure/structure_IMPACT.ho \
		bin/md_preprocessor bin/md_compiler bin/lmdes2_customizer \
		bin/convert_hmdes
	mkdir -p mdes/Limpact
	hmdesc $(abs_srcdir)/mdes/Limpact/IMPACT_SIMPLE_TEMPLATE.hmdes2 \
		-o mdes/Limpact/SS_16R_8BX.lmdes2 -DWIDTH=16 \
		-DNUM_BRANCHES=8 -DBRANCHES_AT_END=0 -DNON_TRAPPING_OPS=0

mdes/Limpact/SS_16R_16BL.lmdes2: \
		$(abs_srcdir)/mdes/Limpact/IMPACT_SIMPLE_TEMPLATE.hmdes2 \
		$(top_builddir)/mdes/structure/structure_IMPACT.ho \
		bin/md_preprocessor bin/md_compiler bin/lmdes2_customizer \
		bin/convert_hmdes
	mkdir -p mdes/Limpact
	hmdesc $(abs_srcdir)/mdes/Limpact/IMPACT_SIMPLE_TEMPLATE.hmdes2 \
		-o mdes/Limpact/SS_16R_16BL.lmdes2 -DWIDTH=16 \
		-DNUM_BRANCHES=16 -DBRANCHES_AT_END=1 -DNON_TRAPPING_OPS=0

mdes/Limpact/SS_16R_16BX.lmdes2: \
		$(abs_srcdir)/mdes/Limpact/IMPACT_SIMPLE_TEMPLATE.hmdes2 \
		$(top_builddir)/mdes/structure/structure_IMPACT.ho \
		bin/md_preprocessor bin/md_compiler bin/lmdes2_customizer \
		bin/convert_hmdes
	mkdir -p mdes/Limpact
	hmdesc $(abs_srcdir)/mdes/Limpact/IMPACT_SIMPLE_TEMPLATE.hmdes2 \
		-o mdes/Limpact/SS_16R_16BX.lmdes2 -DWIDTH=16 \
		-DNUM_BRANCHES=16 -DBRANCHES_AT_END=0 -DNON_TRAPPING_OPS=0








