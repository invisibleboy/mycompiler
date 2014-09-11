## mdes/structure/Makefile.am
## This file is (ultimately) included by the top level Makefile.am, so all
## files need to be specified relative to the top level directory.

mdes_structuredir = $(prefix)/mdes/structure

mdes_structure_DATA = mdes/structure/structure_IMPACT.ho
dist_mdes_structure_DATA = mdes/structure/structure_IMPACT.hmdes2 \
			   mdes/structure/structure_Itanium.hmdes2 \
			   mdes/structure/structure_pristine.hmdes2

## Specify the files that are created by this make file so that 'make clean'
## will delete them properly.
mdes_structure_CLN = $(mdes_structure_DATA)

## A target to support building everything under this subdirectory.  This
## should refer to anything defined above.  There should be two targets; one
## with and one without the trailing slash.  The trailing slash will allow
## the use of tab completion.
mdes/structure mdes/structure/: $(mdes_structure_DATA)

## A target to support cleaning everything under this subdirectory.
mdes/structure/clean:
	rm -f $(mdes_structure_CLN)

$(mdes_structure_DATA): $(dist_mdes_structure_DATA) \
		       bin/md_preprocessor \
		       bin/md_compiler
	mkdir -p mdes/structure
	$(foreach file, $(dist_mdes_structure_DATA), \
		output=`echo $(file) | sed -e 's/\.hmdes2/.ho/'` ; \
		bin/md_preprocessor $(abs_srcdir)/$(file) | \
		bin/md_compiler -stdin -o $${output};)
