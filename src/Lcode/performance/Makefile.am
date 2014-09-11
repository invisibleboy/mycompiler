## src/Lcode/performance/Makefile.am
## This file is (ultimately) included by the top level Makefile.am, so all
## files need to be specified relative to the top level directory.

include $(srcdir)/src/Lcode/performance/Lprobe/Makefile.am
include $(srcdir)/src/Lcode/performance/Lannotate/Makefile.am

include $(srcdir)/src/Lcode/performance/Lencode/Makefile.am
include $(srcdir)/src/Lcode/performance/Lget/Makefile.am

include $(srcdir)/src/Lcode/performance/Lprofile/Makefile.am

include $(srcdir)/src/Lcode/performance/Lsim/Makefile.am



src_Lcode_performance_PRG = \
	$(src_Lcode_performance_Lannotate_PRG) \
	$(src_Lcode_performance_Lencode_PRG) \
	$(src_Lcode_performance_Lget_PRG)  \
	$(src_Lcode_performance_Lprofile_PRG) \
	$(src_Lcode_performance_Lsim_PRG) 
	
src_Lcode_performance_LIB = \
	$(src_Lcode_performance_Lannotate_LIB) \
	$(src_Lcode_performance_Lencode_LIB) \
	$(src_Lcode_performance_Lget_LIB)  \
	$(src_Lcode_performance_Lprofile_LIB) \
	$(src_Lcode_performance_Lsim_LIB)
	
src_Lcode_performance_DST = \
	$(src_Lcode_performance_Lannotate_DST) \
	$(src_Lcode_performance_Lprobe_DST) \
	$(src_Lcode_performance_Lencode_DST) \
	$(src_Lcode_performance_Lget_DST)  \
	$(src_Lcode_performance_Lprofile_DST) \
	$(src_Lcode_performance_Lsim_DST) 
	
src_Lcode_performance_INC = src_Lcode_performance_Lprobe_INC
noinst_src_Lcode_performance_INC = \
	noinst_src_Lcode_performance_Lannotate_INC \
	noinst_src_Lcode_performance_Lprobe_INC \
	noinst_src_Lcode_performance_Lencode_INC \
	noinst_src_Lcode_performance_Lget_INC \
	noinst_src_Lcode_performance_Lsim_INC 
	

## A target to support building everything under this subdirectory.  This
## should refer to anything defined above.  There should be two targets; one
## with and one without the trailing slash.  The trailing slash will allow
## the use of tab completion.
src/Lcode/performance src/Lcode/performance/: \
	include src/Lcode/performance/Lprobe src/Lcode/performance/Lannotate \
	src/Lcode/performance/Lcombine src/Lcode/performance/Lencode \
	src/Lcode/performance/Lget src/Lcode/performance/Lmix \
	src/Lcode/performance/Lprofile src/Lcode/performance/Lput \
	src/Lcode/performance/Lsim src/Lcode/performance/Lsimaddr_build \
	src/Lcode/performance/Lcmp_scheds

## A target to support cleaning everything under this subdirectory.
src/Lcode/performance/clean: \
	include/clean src/Lcode/performance/Lprobe/clean \
	src/Lcode/performance/Lannotate/clean \
	src/Lcode/performance/Lencode/clean \
	src/Lcode/performance/Lget/clean  \
	src/Lcode/performance/Lprofile/clean \
	src/Lcode/performance/Lsim/clean 
	

## A target to build all documentation under this subdirectory.
src/Lcode/performance/doc: \
	src/Lcode/performance/Lprobe/doc src/Lcode/performance/Lannotate/doc \
	 src/Lcode/performance/Lencode/doc \
	src/Lcode/performance/Lget/doc  \
	src/Lcode/performance/Lprofile/doc  \
	src/Lcode/performance/Lsim/doc
	

