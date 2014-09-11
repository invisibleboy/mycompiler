## src/Lcode/sched/Makefile.am
## This file is (ultimately) included by the top level Makefile.am, so all
## files need to be specified relative to the top level directory.

include $(srcdir)/src/Lcode/sched/SM/Makefile.am



src_Lcode_sched_PRG = \
	$(src_Lcode_sched_SM_PRG)  
	
src_Lcode_sched_LIB = \
	$(src_Lcode_sched_SM_LIB)  
	
src_Lcode_sched_DST = \
	$(src_Lcode_sched_SM_DST)  
	
noinst_src_Lcode_sched_INC = \
	noinst_src_Lcode_sched_SM_INC  
	

## A target to support building everything under this subdirectory.  This
## should refer to anything defined above.  There should be two targets; one
## with and one without the trailing slash.  The trailing slash will allow
## the use of tab completion.
src/Lcode/sched src/Lcode/sched/: \
	include src/Lcode/sched/SM src/Lcode/sched/Partial_RIC \
	src/Lcode/sched/Lpartial_ric

## A target to support cleaning everything under this subdirectory.
src/Lcode/sched/clean: \
	include/clean src/Lcode/sched/SM/clean 
	 

## A target to build all documentation under this subdirectory.
src/Lcode/sched/doc: \
	src/Lcode/sched/SM/doc  
	
