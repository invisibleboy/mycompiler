## Makefile.am 
## This file is (ultimately) included by the top level Makefile.am, so all
## files need to be specified relative to the top level directory.

AUTOMAKE_OPTIONS = 1.6 subdir-objects
if HOST_X86_64
AM_CFLAGS = -g -m32 -Wall -I$(top_builddir)/include -L$(top_builddir)/lib
else
AM_CFLAGS = -g -Wall -I$(top_builddir)/include -L$(top_builddir)/lib
endif
AM_YFLAGS = -d
AM_LDFLAGS = 

abs_srcdir = @abs_srcdir@
abs_builddir = @abs_builddir@

BLT = $(abs_builddir)/benchmarks $(abs_builddir)/parms \
	$(abs_builddir)/projects $(abs_builddir)/scripts

# Set the environment variables needed by the IMPACT tools
# In the event that we're compiling oicc with oicc, these environment
# variables will be appropriately set by init-impact, so it is not 
# necessary to redefine them.
if COMPILER_IS_OICC
else
export IMPACT_REL_PATH = $(abs_srcdir)
export IMPACT_ROOT = $(abs_builddir)
export DEFAULT_PROJECT full
export IMPACT_BUILD_TYPE = @TARGET_PLATFORM@
export IMPACT_HOST_PLATFORM = @HOST_PLATFORM@
export IMPACT_HOST_COMPILER = @HOST_COMPILER@
export STD_PARMS_FILE = $(abs_srcdir)/parms/STD_PARMS.IPF-MCKINLEY
export PATH := $(abs_srcdir)/scripts:$(abs_srcdir)/driver:$(abs_builddir)/bin:$(PATH)
endif

include $(srcdir)/src/Makefile.am
include $(srcdir)/inhouse/Makefile.am
include $(srcdir)/mdes/Makefile.am
include $(srcdir)/platform/Makefile.am
include $(srcdir)/scripts/Makefile.am
include $(srcdir)/parms/Makefile.am
include $(srcdir)/intrinsic.lib/Makefile.am
include $(srcdir)/driver/Makefile.am
include $(srcdir)/benchmarks/Makefile.am
include $(srcdir)/projects/Makefile.am
include $(srcdir)/doc/Makefile.am

EXTRA_DIST = $(platform_DST) $(src_DST) $(intrinsic_lib_DST) $(scripts_DST) \
	     $(driver_DST) $(doc_DST)

lib_LIBRARIES = $(src_LIB) $(platform_LIB)

bin_PROGRAMS = $(src_PRG) $(inhouse_PRG)

inc_INC = $(src_INC) $(noinst_src_INC)

# inc_INC contains the names of the variables containing include files to
# link into include.  For each file defined in each of these variables,
# find the directory under include by substituting _CLASS for _INC in the 
# variable name, then add the filename to the end.
inc_BLT = $(foreach incvar,$(inc_INC), \
		$(foreach file, $($(incvar)), \
			include/$($(subst _INC,_CLASS,$(incvar)))/$(notdir $(file))))

# bin_BLT contains the names of each program in bin_PROGRAMS prefixed
# with bin/
bin_BLT = $(foreach program,$(bin_PROGRAMS), bin/$(notdir $(program)))

# lib_BLT contains the names of each library in lib_LIBRARIES prefixed
# with lib/
lib_BLT = $(foreach library,$(lib_LIBRARIES), lib/$(notdir $(library)))

nodist_noinst_HEADERS = $(inc_BLT)

BUILT_SOURCES = $(src_BLT) $(inc_BLT) $(BLT) lib bin

CLEANFILES = $(mdes_CLN) $(platform_CLN) $(intrinsic_lib_CLN) $(inc_BLT)

bin_SCRIPTS = init-impact

doc: src/doc

# A rule to write the init-impact script
init-impact:
	echo 'test `echo $${SHELL} | sed -n -e "/csh/p"` && \' > init-impact
	echo '	setenv IMPACT_REL_PATH $(abs_srcdir) && \' >> init-impact
	echo '	setenv IMPACT_ROOT $(abs_builddir) && \' >> init-impact
	echo '	setenv DEFAULT_PROJECT full && \' >> init-impact
	echo '	setenv IMPACT_BUILD_TYPE @TARGET_PLATFORM@ && \' >> init-impact
	echo '	setenv IMPACT_HOST_PLATFORM @HOST_PLATFORM@ && \' >> init-impact
	echo '	setenv IMPACT_HOST_COMPILER @HOST_COMPILER@ && \' >> init-impact
	echo '	setenv STD_PARMS_FILE $(abs_srcdir)/parms/STD_PARMS.IPF-MCKINLEY && \' >> init-impact
	echo '	setenv PATH "$(abs_srcdir)/scripts:$(abs_srcdir)/driver:$(abs_builddir)/bin:"$${PATH}' >> init-impact
	echo 'test `echo $${SHELL} | sed -n -e "/csh/p"` || \' >> init-impact
	echo '	export IMPACT_REL_PATH=$(abs_srcdir) && \' >> init-impact
	echo '	export IMPACT_ROOT=$(abs_builddir) && \' >> init-impact
	echo '	export DEFAULT_PROJECT=full && \' >> init-impact
	echo '	export IMPACT_BUILD_TYPE=@TARGET_PLATFORM@ && \' >> init-impact
	echo '	export IMPACT_HOST_PLATFORM=@HOST_PLATFORM@ && \' >> init-impact
	echo '	export IMPACT_HOST_COMPILER=@HOST_COMPILER@ && \' >> init-impact
	echo '	export STD_PARMS_FILE=$(abs_srcdir)/parms/STD_PARMS.IPF-MCKINLEY && \' >> init-impact
	echo '	export PATH="$(abs_srcdir)/scripts:$(abs_srcdir)/driver:$(abs_builddir)/bin:"$${PATH}' >> init-impact

$(BLT):
	$(foreach file, $(BLT), \
		ln -sf $(abs_srcdir)/$(notdir $(file)) $(file);)

# Modify the init-impact script so that the paths point to the install tree.
# This also copies impact-edgcpfe from inhouse to bin.
install-exec-hook:
	cat $(prefix)/bin/init-impact | \
		sed -e 's%$(abs_srcdir)%$(prefix)%g; \
			s%$(abs_builddir)%$(prefix)%g' > \
		$(prefix)/bin/init-impact.fixed
	mv $(prefix)/bin/init-impact.fixed $(prefix)/bin/init-impact
	$(binPROGRAMS_INSTALL) inhouse/edgcpfe/impact-edgcpfe \
		$(prefix)/bin/impact-edgcpfe

# When we do 'make dist', write a custom Makefile.am for impact-edgcpfe that
# makes sure the precompiled binary is installed correctly.
dist-hook:
	rm -f $(distdir)/inhouse/edgcpfe/Makefile.am
	mkdir -p $(distdir)/inhouse/edgcpfe
	echo 'inhouse_edgcpfe_PRG = ' > $(distdir)/inhouse/edgcpfe/Makefile.am
	echo 'inhouse_edgcpfe_LIB = ' >> $(distdir)/inhouse/edgcpfe/Makefile.am
	echo 'inhouse_edgcpfe_DBIN = inhouse/edgcpfe/impact-edgcpfe' >> $(distdir)/inhouse/edgcpfe/Makefile.am
	echo 'inhouse/edgcpfe/impact-edgcpfe: $$(abs_builddir)/inhouse/edgcpfe' >> $(distdir)/inhouse/edgcpfe/Makefile.am
	echo '	if [ ! -f $$(abs_builddir)/inhouse/edgcpfe/impact-edgcpfe ]; then \' >> $(distdir)/inhouse/edgcpfe/Makefile.am
	echo '		ln -sf $$(abs_srcdir)/inhouse/edgcpfe/impact-edgcpfe \' >> $(distdir)/inhouse/edgcpfe/Makefile.am
	echo '			$$(abs_builddir)/inhouse/edgcpfe/impact-edgcpfe; \' >> $(distdir)/inhouse/edgcpfe/Makefile.am
	echo '	fi' >> $(distdir)/inhouse/edgcpfe/Makefile.am
	echo 'inhouse/edgcpfe inhouse/edgcpfe/: include inhouse/edgcpfe/impact-edgcpfe ' >> $(distdir)/inhouse/edgcpfe/Makefile.am
	echo 'inhouse/edgcpfe/clean:' >> $(distdir)/inhouse/edgcpfe/Makefile.am
	echo '$$(abs_builddir)/inhouse/edgcpfe:' >> $(distdir)/inhouse/edgcpfe/Makefile.am
	echo '	mkdir -p $$(abs_builddir)/inhouse/edgcpfe' >> $(distdir)/inhouse/edgcpfe/Makefile.am
	cd $(distdir) && automake --foreign

# A rule to list each distributed file.  This is used when producing the
# diff to patch two releases.
dist-list:
	@for file in $(DISTFILES) ; do \
		echo $${file} ; \
	done

# Rules to build distribution tarballs containing only the
# impact-edgcpfe binary.  There is a rule for each supported
# architecture.
if HOST_PLATFORM_IA64LIN_GCC
edgdist-ia64: inhouse/edgcpfe
	mkdir -p $(distdir)/inhouse/edgcpfe
	cp inhouse/edgcpfe/impact-edgcpfe \
		$(distdir)/inhouse/edgcpfe/impact-edgcpfe
	strip $(distdir)/inhouse/edgcpfe/impact-edgcpfe
	tar chof - $(distdir)/inhouse/edgcpfe/impact-edgcpfe | \
		gzip -c > $(PACKAGE)-fe-ia64-$(PACKAGE_VERSION).tar.gz
	rm -r $(distdir)
else
edgdist-ia64:
	@echo "'make edgdist-ia64' must be run on an ia64 machine."
endif

if HOST_PLATFORM_HP_CC
edgdist-hp: inhouse/edgcpfe
	mkdir -p $(distdir)/inhouse/edgcpfe
	cp inhouse/edgcpfe/impact-edgcpfe \
		$(distdir)/inhouse/edgcpfe/impact-edgcpfe
	strip $(distdir)/inhouse/edgcpfe/impact-edgcpfe
	tar chof - $(distdir)/inhouse/edgcpfe/impact-edgcpfe | \
		gzip -c > $(PACKAGE)-fe-hp-$(PACKAGE_VERSION).tar.gz
	rm -r $(distdir)
else
if HOST_PLATFORM_HP_GCC
edgdist-hp: inhouse/edgcpfe
	mkdir -p $(distdir)/inhouse/edgcpfe
	cp inhouse/edgcpfe/impact-edgcpfe \
		$(distdir)/inhouse/edgcpfe/impact-edgcpfe
	strip $(distdir)/inhouse/edgcpfe/impact-edgcpfe
	tar chof - $(distdir)/inhouse/edgcpfe/impact-edgcpfe | \
		gzip -c > $(PACKAGE)-fe-hp-$(PACKAGE_VERSION).tar.gz
	rm -r $(distdir)
else
edgdist-hp:
	@echo "'make edgdist-hp' must be run on an hpux machine."
endif
endif

if HOST_PLATFORM_X86LIN_GCC
edgdist-x86: inhouse/edgcpfe
	mkdir -p $(distdir)/inhouse/edgcpfe
	cp inhouse/edgcpfe/impact-edgcpfe \
		$(distdir)/inhouse/edgcpfe/impact-edgcpfe
	strip $(distdir)/inhouse/edgcpfe/impact-edgcpfe
	tar chof - $(distdir)/inhouse/edgcpfe/impact-edgcpfe | \
		gzip -c > $(PACKAGE)-fe-x86-$(PACKAGE_VERSION).tar.gz
	rm -r $(distdir)
else
edgdist-x86:
	@echo "'make edgdist-x86' must be run on an x86 machine."
endif

# This automatically creates the symlinks in include fromthe various _INC 
# variables and their corresponding _CLASS variables.
#
# _INC variables contain the paths to the include files that should be
# symlinked into include.
# _CLASS variables contain the subdirectory under include where the symlink
# should be created.  This corresponds directly to the CLASS variable in the
# Imakefile system.
#
# inc_INC is a list of variable names, not a list of the include files.  This
# lets us find the corresponding _CLASS variable for every _INC variable.
#
# $(incvar) is set to each _INC variable name.
# $($(subst _INC,_CLASS,$(incvar))) substitutes _CLASS for _INC in the variable
#  name, then evaluates to the value of the _CLASS variable.
#
# For every _INC variable, we evaluate the value of the corresponding _CLASS
# variable and create the directory if needed.  We then create symlinks in
# this directory for every file listed in the _INC variable.
# $(inc_BLT):
# 	result=$(foreach incvar, $(inc_INC), \
# 		$(shell mkdir -p include/$($(subst _INC,_CLASS,$(incvar)));) \
# 		$(foreach file, $($(incvar)), \
# 			$(shell ln -sf $(abs_srcdir)/$(file) \
# 				include/$($(subst _INC,_CLASS,$(incvar)));)))

# include: $(inc_BLT)

bin:: ; make_links.pl bin

bin/%: ; make_links.pl $@

lib:: ; make_links.pl lib

lib/%: ; make_links.pl $@

include:: ; make_links.pl include

include/%: ; make_links.pl $@

# See the comment for the $(inc_BLT): rule.
# include/clean:
# 	result=$(foreach incvar,$(inc_INC), \
# 		$(foreach file,$($(incvar)), \
# 			$(shell rm -f include/$($(subst _INC,_CLASS,$(incvar)))/$(notdir $(file));)))

all-hook:
	make_links.pl
