inhouse_edgcpfe_PRG = 
inhouse_edgcpfe_LIB = 
inhouse_edgcpfe_DBIN = inhouse/edgcpfe/impact-edgcpfe
inhouse/edgcpfe/impact-edgcpfe: $(abs_builddir)/inhouse/edgcpfe
	if [ ! -f $(abs_builddir)/inhouse/edgcpfe/impact-edgcpfe ]; then \
		ln -sf $(abs_srcdir)/inhouse/edgcpfe/impact-edgcpfe \
			$(abs_builddir)/inhouse/edgcpfe/impact-edgcpfe; \
	fi
inhouse/edgcpfe inhouse/edgcpfe/: include inhouse/edgcpfe/impact-edgcpfe 
inhouse/edgcpfe/clean:
$(abs_builddir)/inhouse/edgcpfe:
	mkdir -p $(abs_builddir)/inhouse/edgcpfe
