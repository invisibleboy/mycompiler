## mdes/Makefile.am
## This file is (ultimately) included by the top level Makefile.am, so all
## files need to be specified relative to the top level directory.

include $(srcdir)/mdes/structure/Makefile.am

include $(srcdir)/mdes/Ltahoe/Makefile.am
include $(srcdir)/mdes/Limpact/Makefile.am


## A target to support building everything under this subdirectory.  This
## should refer to anything defined above.  There should be two targets; one
## with and one without the trailing slash.  The trailing slash will allow
## the use of tab completion.
mdes mdes/: \
	mdes/structure mdes/Limpact mdes/Ltahoe mdes/Lstarcore \
	mdes/Limpact

## A target to support cleaning everything under this subdirectory.
mdes/clean:
	mdes/structure/clean  mdes/Ltahoe/clean mdes/Limpact/clean
	

mdes_CLN = \
	$(mdes_structure_CLN)  $(mdes_Ltahoe_CLN) $(mdes_Limpact_CLN)
	
