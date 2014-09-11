<?xml version='1.0' encoding='ISO-8859-1' standalone='yes' ?>
<tagfile>
  <compound kind="file">
    <name>convert_hmdes.c</name>
    <path>/mnt/hgfs/mkt/Desktop/openimpact/src/machine/convert_hmdes/</path>
    <filename>convert__hmdes_8c</filename>
    <member kind="function">
      <type>void</type>
      <name>L_punt</name>
      <anchorfile>convert__hmdes_8c.html</anchorfile>
      <anchor>c8ccfa451c1bd1740a1b720ba5b4c3a9</anchor>
      <arglist>(char *fmt,...)</arglist>
    </member>
    <member kind="function">
      <type>void</type>
      <name>print_usage</name>
      <anchorfile>convert__hmdes_8c.html</anchorfile>
      <anchor>e5ad5cbeccaedc03a48d3c7eaa803e79</anchor>
      <arglist>()</arglist>
    </member>
    <member kind="function">
      <type>MD_Entry *</type>
      <name>add_entry</name>
      <anchorfile>convert__hmdes_8c.html</anchorfile>
      <anchor>06be49cfc7f58ccb3e02f1e051feb60e</anchor>
      <arglist>(MD *md, char *section_name, char *entry_name)</arglist>
    </member>
    <member kind="function">
      <type>MD_Entry *</type>
      <name>find_entry_if_exists</name>
      <anchorfile>convert__hmdes_8c.html</anchorfile>
      <anchor>34bf787819d11144e052c7527984718a</anchor>
      <arglist>(MD *md, char *section_name, char *entry_name)</arglist>
    </member>
    <member kind="function">
      <type>MD_Entry *</type>
      <name>find_entry</name>
      <anchorfile>convert__hmdes_8c.html</anchorfile>
      <anchor>f58daa7180bb89cd7f1db6c839e73d24</anchor>
      <arglist>(MD *md, char *section_name, char *entry_name)</arglist>
    </member>
    <member kind="function">
      <type>void</type>
      <name>add_value_to_field</name>
      <anchorfile>convert__hmdes_8c.html</anchorfile>
      <anchor>26b31203d2cfec0406e930f19e4d534f</anchor>
      <arglist>(MD_Entry *entry, char *field_name, int index, int type, void *value)</arglist>
    </member>
    <member kind="function">
      <type>void</type>
      <name>create_empty_field</name>
      <anchorfile>convert__hmdes_8c.html</anchorfile>
      <anchor>51b19f66e703ea6bc42975c17d0f56f8</anchor>
      <arglist>(MD_Entry *entry, char *field_name)</arglist>
    </member>
    <member kind="function">
      <type>void</type>
      <name>append_value_to_field</name>
      <anchorfile>convert__hmdes_8c.html</anchorfile>
      <anchor>9865cd40b273eaa099d6d14d1c38c186</anchor>
      <arglist>(MD_Entry *entry, char *field_name, int type, void *value)</arglist>
    </member>
    <member kind="function">
      <type>void</type>
      <name>convert_Parameters</name>
      <anchorfile>convert__hmdes_8c.html</anchorfile>
      <anchor>f202962df85bdaaedd37e868417af372</anchor>
      <arglist>(MD *md, Hmdes *hmdes)</arglist>
    </member>
    <member kind="function">
      <type>int</type>
      <name>in_set</name>
      <anchorfile>convert__hmdes_8c.html</anchorfile>
      <anchor>b72910e402756bb494e73ff644d6a468</anchor>
      <arglist>(Hmdes_IO_Set *set, char *name)</arglist>
    </member>
    <member kind="function">
      <type>int</type>
      <name>compatable_sets</name>
      <anchorfile>convert__hmdes_8c.html</anchorfile>
      <anchor>a1538bee109770d069d4df477d332a23</anchor>
      <arglist>(Hmdes_IO_Set *src_set, Hmdes_IO_Set *target_set)</arglist>
    </member>
    <member kind="function">
      <type>void</type>
      <name>convert_format</name>
      <anchorfile>convert__hmdes_8c.html</anchorfile>
      <anchor>e050552b2cb01a9ba04eda87064a6e4e</anchor>
      <arglist>(MD *md, Hmdes *hmdes)</arglist>
    </member>
    <member kind="function">
      <type>MD_Entry *</type>
      <name>add_op_lat</name>
      <anchorfile>convert__hmdes_8c.html</anchorfile>
      <anchor>2bf1f437337eda2962b935e0bc407b8b</anchor>
      <arglist>(MD *md, char *prefix, int latency)</arglist>
    </member>
    <member kind="function">
      <type>void</type>
      <name>convert_latency</name>
      <anchorfile>convert__hmdes_8c.html</anchorfile>
      <anchor>8d13d90966ee0a9fba3c647d2514e29c</anchor>
      <arglist>(MD *md, Hmdes *hmdes)</arglist>
    </member>
    <member kind="function">
      <type>void</type>
      <name>convert_res_tables</name>
      <anchorfile>convert__hmdes_8c.html</anchorfile>
      <anchor>a238ad20977fd84a65677e7c336a7f5a</anchor>
      <arglist>(MD *md, Hmdes *hmdes)</arglist>
    </member>
    <member kind="function">
      <type>void</type>
      <name>convert_operations</name>
      <anchorfile>convert__hmdes_8c.html</anchorfile>
      <anchor>4fb3226ec52c93c25bbf1f7daddd8c23</anchor>
      <arglist>(MD *md, Hmdes *hmdes)</arglist>
    </member>
    <member kind="function">
      <type>void</type>
      <name>convert_hmdes</name>
      <anchorfile>convert__hmdes_8c.html</anchorfile>
      <anchor>1c8abf12365ede1d73d0fb9183fb6cdd</anchor>
      <arglist>(MD *md, Hmdes *hmdes)</arglist>
    </member>
    <member kind="function">
      <type>int</type>
      <name>main</name>
      <anchorfile>convert__hmdes_8c.html</anchorfile>
      <anchor>647f21a28344e1d9c643f4115516d7c9</anchor>
      <arglist>(int argc, char **argv, char **envp)</arglist>
    </member>
    <member kind="variable">
      <type>Header_File</type>
      <name>header_table</name>
      <anchorfile>convert__hmdes_8c.html</anchorfile>
      <anchor>44407bf7029d07dd2cdec64d45500e7a</anchor>
      <arglist>[]</arglist>
    </member>
    <member kind="variable">
      <type>int</type>
      <name>num_header_files</name>
      <anchorfile>convert__hmdes_8c.html</anchorfile>
      <anchor>1771c66351c3c7fd849af851b3e31455</anchor>
      <arglist></arglist>
    </member>
    <member kind="variable">
      <type>char *</type>
      <name>program_name</name>
      <anchorfile>convert__hmdes_8c.html</anchorfile>
      <anchor>289c5900d90626d909f0a85d5a0ed61d</anchor>
      <arglist></arglist>
    </member>
  </compound>
</tagfile>
