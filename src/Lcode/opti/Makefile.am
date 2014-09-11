## src/Lcode/opti/Makefile.am
## This file is (ultimately) included by the top level Makefile.am, so all
## files need to be specified relative to the top level directory.

include $(srcdir)/src/Lcode/opti/Lmarkpipe/Makefile.am
include $(srcdir)/src/Lcode/opti/Lblock/Makefile.am
include $(srcdir)/src/Lcode/opti/Lopti/Makefile.am

include $(srcdir)/src/Lcode/opti/Lsuperscalar/Makefile.am
include $(srcdir)/src/Lcode/opti/Lsafe/Makefile.am



src_Lcode_opti_PRG = \
	$(src_Lcode_opti_Lmarkpipe_PRG) $(src_Lcode_opti_Lblock_PRG) \
	$(src_Lcode_opti_Lopti_PRG) $(src_Lcode_opti_Lsuperscalar_PRG) \
	$(src_Lcode_opti_Lsafe_PRG)  
	
src_Lcode_opti_LIB = \
	$(src_Lcode_opti_Lmarkpipe_LIB) $(src_Lcode_opti_Lblock_LIB) \
	$(src_Lcode_opti_Lopti_LIB) $(src_Lcode_opti_Lsuperscalar_LIB) \
	$(src_Lcode_opti_Lsafe_LIB)  
	
src_Lcode_opti_DST = \
	$(src_Lcode_opti_Lmarkpipe_DST) $(src_Lcode_opti_Lblock_DST) \
	$(src_Lcode_opti_Lopti_DST) $(src_Lcode_opti_Lsuperscalar_DST) \
	$(src_Lcode_opti_Lsafe_DST)  
	
noinst_src_Lcode_opti_INC = \
	noinst_src_Lcode_opti_Lmarkpipe_INC noinst_src_Lcode_opti_Lblock_INC \
	noinst_src_Lcode_opti_Lopti_INC \
	noinst_src_Lcode_opti_Lsuperscalar_INC \
	noinst_src_Lcode_opti_Lsafe_INC


## A target to support building everything under this subdirectory.  This
## should refer to anything defined above.  There should be two targets; one
## with and one without the trailing slash.  The trailing slash will allow
## the use of tab completion.
src/Lcode/opti src/Lcode/opti/: \
	include src/Lcode/opti/Lmarkpipe src/Lcode/opti/Lblock \
	src/Lcode/opti/Lopti src/Lcode/opti/Lsuperscalar src/Lcode/opti/Lsafe \
	src/Lcode/opti/Linline src/Lcode/opti/Lpred_opti \
	src/Lcode/opti/Lssaopti

## A target to support cleaning everything under this subdirectory.
src/Lcode/opti/clean: \
	include/clean src/Lcode/opti/Lmarkpipe/clean \
	src/Lcode/opti/Lblock/clean src/Lcode/opti/Lopti/clean \
	src/Lcode/opti/Lsuperscalar/clean src/Lcode/opti/Lsafe/clean 
	 


## A target to build all documentation under this subdirectory.
src/Lcode/opti/doc: \
	src/Lcode/opti/Lmarkpipe/doc src/Lcode/opti/Lblock/doc \
	src/Lcode/opti/Lopti/doc src/Lcode/opti/Lsuperscalar/doc \
	src/Lcode/opti/Lsafe/doc  
	
