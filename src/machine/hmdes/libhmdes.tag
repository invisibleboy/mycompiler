<?xml version='1.0' encoding='ISO-8859-1' standalone='yes' ?>
<tagfile>
  <compound kind="file">
    <name>header_reader.c</name>
    <path>/mnt/hgfs/mkt/Desktop/openimpact/src/machine/hmdes/</path>
    <filename>header__reader_8c</filename>
    <includes id="hmdes_8h" name="hmdes.h" local="yes" imported="no">hmdes.h</includes>
    <class kind="struct">hparse_info_st</class>
    <member kind="typedef">
      <type>hparse_info_st</type>
      <name>Hparse_Info</name>
      <anchorfile>header__reader_8c.html</anchorfile>
      <anchor>1c7a4795de9ad80203df10f8f5b59d98</anchor>
      <arglist></arglist>
    </member>
    <member kind="function" static="yes">
      <type>static void</type>
      <name>Hparse_line</name>
      <anchorfile>header__reader_8c.html</anchorfile>
      <anchor>483c6ed64a0362299c11a4aaf7b60b3d</anchor>
      <arglist>(char *ptr, char *name, char *arg)</arglist>
    </member>
    <member kind="function" static="yes">
      <type>static int</type>
      <name>Hget_next_line</name>
      <anchorfile>header__reader_8c.html</anchorfile>
      <anchor>16e5e39843597bfa0f4d56d91502b4ed</anchor>
      <arglist>(Hparse_Info *hinfo, char *ret_buf)</arglist>
    </member>
    <member kind="function" static="yes">
      <type>static void</type>
      <name>Hskip_to_endif</name>
      <anchorfile>header__reader_8c.html</anchorfile>
      <anchor>f63b46bd1c5165e8ffdcc5823217d55f</anchor>
      <arglist>(Hparse_Info *hinfo, char *source, int line)</arglist>
    </member>
    <member kind="function">
      <type>Msymbol_Table *</type>
      <name>Hread_header_file</name>
      <anchorfile>header__reader_8c.html</anchorfile>
      <anchor>2e6daf4ca9e19d27b1a6df7911acff59</anchor>
      <arglist>(char *file_name)</arglist>
    </member>
  </compound>
  <compound kind="file">
    <name>hmdes.c</name>
    <path>/mnt/hgfs/mkt/Desktop/openimpact/src/machine/hmdes/</path>
    <filename>hmdes_8c</filename>
    <includes id="hmdes_8h" name="hmdes.h" local="yes" imported="no">hmdes.h</includes>
    <member kind="function">
      <type>int</type>
      <name>Mread_int</name>
      <anchorfile>hmdes_8c.html</anchorfile>
      <anchor>21167fd2307af816d91d77546db04321</anchor>
      <arglist>(Mparse_Info *pinfo, int *ptr)</arglist>
    </member>
    <member kind="function" static="yes">
      <type>static int</type>
      <name>Mparse_next_line</name>
      <anchorfile>hmdes_8c.html</anchorfile>
      <anchor>56f33ce1f0bd64ecc6125805c1d6b11b</anchor>
      <arglist>(Mparse_Info *pinfo)</arglist>
    </member>
    <member kind="function">
      <type>int</type>
      <name>mread_proc_model</name>
      <anchorfile>hmdes_8c.html</anchorfile>
      <anchor>a0d5044667c2d6c7abc5ecc0f0cc045f</anchor>
      <arglist>(Mparse_Info *pinfo, char *name, int *ptr)</arglist>
    </member>
    <member kind="function">
      <type>int</type>
      <name>mread_define_var</name>
      <anchorfile>hmdes_8c.html</anchorfile>
      <anchor>863ffb908bbc23df92008f34e8650173</anchor>
      <arglist>(Mparse_Info *pinfo, char *name, int *ptr)</arglist>
    </member>
    <member kind="function">
      <type>void</type>
      <name>Mdie_on_fatal_errors</name>
      <anchorfile>hmdes_8c.html</anchorfile>
      <anchor>c86bbf19bc7a087f23ce6dabbcc0ba13</anchor>
      <arglist>(Mparse_Info *pinfo)</arglist>
    </member>
    <member kind="function">
      <type>void</type>
      <name>Hbuild_environment_defines</name>
      <anchorfile>hmdes_8c.html</anchorfile>
      <anchor>ff072a5397cbb97e07781e19a7a213e5</anchor>
      <arglist>(Msymbol_Table *env_tab, char **envp)</arglist>
    </member>
    <member kind="function">
      <type>void</type>
      <name>Hbuild_command_line_defines</name>
      <anchorfile>hmdes_8c.html</anchorfile>
      <anchor>a80c53a0b9e7a49dbeaed5b087e74ff6</anchor>
      <arglist>(Msymbol_Table *cl_tab, char **argv)</arglist>
    </member>
    <member kind="function">
      <type>Hmdes *</type>
      <name>create_hmdes</name>
      <anchorfile>hmdes_8c.html</anchorfile>
      <anchor>3157c981faa1f80fffce64fe8a1685b2</anchor>
      <arglist>(char *hmdes_file_name, char **argv, char **envp)</arglist>
    </member>
    <member kind="function">
      <type>void</type>
      <name>hmdes_read_section</name>
      <anchorfile>hmdes_8c.html</anchorfile>
      <anchor>f122d4b6dad387d560f2445048d82f3f</anchor>
      <arglist>(Mparse_Info *pinfo, char *name, void(*func)())</arglist>
    </member>
    <member kind="function">
      <type>void</type>
      <name>hmdes_read_define</name>
      <anchorfile>hmdes_8c.html</anchorfile>
      <anchor>a3f0d60b373219c49efda8d55324f992</anchor>
      <arglist>(Mparse_Info *pinfo, char *sname)</arglist>
    </member>
    <member kind="function">
      <type>int</type>
      <name>Mis_name_next</name>
      <anchorfile>hmdes_8c.html</anchorfile>
      <anchor>7040d7057210a1a7921077362a420f82</anchor>
      <arglist>(Mparse_Info *pinfo)</arglist>
    </member>
    <member kind="function">
      <type>int</type>
      <name>Mis_next</name>
      <anchorfile>hmdes_8c.html</anchorfile>
      <anchor>8233d14a6a48cff6d2bd467a11583d87</anchor>
      <arglist>(Mparse_Info *pinfo, char *str)</arglist>
    </member>
    <member kind="function">
      <type>void</type>
      <name>Mread_name</name>
      <anchorfile>hmdes_8c.html</anchorfile>
      <anchor>e2d457c134c37f83e6c72d6c0f9d2281</anchor>
      <arglist>(Mparse_Info *pinfo, char *name, char *desc)</arglist>
    </member>
    <member kind="function">
      <type>void</type>
      <name>Mread_symbol</name>
      <anchorfile>hmdes_8c.html</anchorfile>
      <anchor>f06d6c42e20b1f16afe72019cce9feb3</anchor>
      <arglist>(Mparse_Info *pinfo, Msymbol_Table *table, void **ptr, char *desc)</arglist>
    </member>
    <member kind="function">
      <type>void</type>
      <name>Mread_IO_item_symbol</name>
      <anchorfile>hmdes_8c.html</anchorfile>
      <anchor>a4ef52a27eef051d7a947f6dfe72b35d</anchor>
      <arglist>(Mparse_Info *pinfo, Msymbol_Table *table, void **ptr, char *terminator)</arglist>
    </member>
    <member kind="function">
      <type>void</type>
      <name>Mread_match</name>
      <anchorfile>hmdes_8c.html</anchorfile>
      <anchor>6596ec52b951f99374fa8896ea78aa96</anchor>
      <arglist>(Mparse_Info *pinfo, char *str)</arglist>
    </member>
    <member kind="function">
      <type>int</type>
      <name>mscan_set</name>
      <anchorfile>hmdes_8c.html</anchorfile>
      <anchor>e31ac28eb1189aa14d222358f54daaae</anchor>
      <arglist>(Mparse_Info *pinfo, char *name)</arglist>
    </member>
    <member kind="function">
      <type>void</type>
      <name>Malloc_struct</name>
      <anchorfile>hmdes_8c.html</anchorfile>
      <anchor>44598a63bd3f29a90fdd27cb3531aff7</anchor>
      <arglist>(void **ptr, int size)</arglist>
    </member>
    <member kind="function">
      <type>void</type>
      <name>Malloc_name</name>
      <anchorfile>hmdes_8c.html</anchorfile>
      <anchor>94186a85a4489b80b7d305e6e13fec4c</anchor>
      <arglist>(char **ptr, char *name)</arglist>
    </member>
    <member kind="function">
      <type>Msymbol_Table *</type>
      <name>Mcreate_symbol_table</name>
      <anchorfile>hmdes_8c.html</anchorfile>
      <anchor>9a7308042188883a4be25b3e0ee777d0</anchor>
      <arglist>(char *name, int size)</arglist>
    </member>
    <member kind="function">
      <type>void *</type>
      <name>Mfind_symbol</name>
      <anchorfile>hmdes_8c.html</anchorfile>
      <anchor>6bcfff652cd0e21dc304d3b05c230a52</anchor>
      <arglist>(Msymbol_Table *table, char *name)</arglist>
    </member>
    <member kind="function">
      <type>void</type>
      <name>Minsert_unique_symbol</name>
      <anchorfile>hmdes_8c.html</anchorfile>
      <anchor>5ac6a523a50fd5a52219a1820c8ea6ad</anchor>
      <arglist>(Mparse_Info *pinfo, Msymbol_Table *table, char *name, void *ptr)</arglist>
    </member>
    <member kind="function">
      <type>void</type>
      <name>Minsert_symbol</name>
      <anchorfile>hmdes_8c.html</anchorfile>
      <anchor>74d2d13e74689801a91905ef832e65af</anchor>
      <arglist>(Msymbol_Table *table, char *name, void *ptr)</arglist>
    </member>
    <member kind="function">
      <type>void</type>
      <name>Mprint_symbol_table</name>
      <anchorfile>hmdes_8c.html</anchorfile>
      <anchor>e02a2eb880e45436edfaea7651aa8288</anchor>
      <arglist>(FILE *out, Msymbol_Table *table)</arglist>
    </member>
    <member kind="function">
      <type>int</type>
      <name>Mhash_name</name>
      <anchorfile>hmdes_8c.html</anchorfile>
      <anchor>bd9725ae2e255beb2f27b69261855d05</anchor>
      <arglist>(char *name, int mask)</arglist>
    </member>
    <member kind="function">
      <type>int</type>
      <name>find_header_value</name>
      <anchorfile>hmdes_8c.html</anchorfile>
      <anchor>c172781c19adf04abeb09bc1d5769470</anchor>
      <arglist>(Mparse_Info *pinfo, char *name, int *ptr)</arglist>
    </member>
    <member kind="function">
      <type>void</type>
      <name>hmdes_read_register_files</name>
      <anchorfile>hmdes_8c.html</anchorfile>
      <anchor>6151685cbaff3a77aa055033dfc491da</anchor>
      <arglist>(Mparse_Info *pinfo)</arglist>
    </member>
    <member kind="function">
      <type>void</type>
      <name>Madd_IO_node</name>
      <anchorfile>hmdes_8c.html</anchorfile>
      <anchor>8dae87d00a072289aa6110f4a050834f</anchor>
      <arglist>(Hmdes_IO_Set *IO_set, Hmdes_Reg_File *reg_file)</arglist>
    </member>
    <member kind="function">
      <type>void</type>
      <name>hmdes_read_IO_sets</name>
      <anchorfile>hmdes_8c.html</anchorfile>
      <anchor>dd0bc36d354a0e2a80aa51d52fa69bf9</anchor>
      <arglist>(Mparse_Info *pinfo)</arglist>
    </member>
    <member kind="function">
      <type>void</type>
      <name>hmdes_read_IO_items</name>
      <anchorfile>hmdes_8c.html</anchorfile>
      <anchor>f5c6d475d646d0b90cc6f70e003c9bb8</anchor>
      <arglist>(Mparse_Info *pinfo)</arglist>
    </member>
    <member kind="function">
      <type>void</type>
      <name>hmdes_add_res_subscript</name>
      <anchorfile>hmdes_8c.html</anchorfile>
      <anchor>c3ce0111eb5b22ea3eb78d74799c9512</anchor>
      <arglist>(Mparse_Info *pinfo, Hmdes_Resource *resource, int subscript, int id)</arglist>
    </member>
    <member kind="function">
      <type>void</type>
      <name>hmdes_read_resources</name>
      <anchorfile>hmdes_8c.html</anchorfile>
      <anchor>cd1ff0019bf0cc483c1ec95416c53450</anchor>
      <arglist>(Mparse_Info *pinfo)</arglist>
    </member>
    <member kind="function">
      <type>void</type>
      <name>Madd_res_option</name>
      <anchorfile>hmdes_8c.html</anchorfile>
      <anchor>c44837bb65e6328e99ae414c9ee8499c</anchor>
      <arglist>(Mparse_Info *pinfo, Hmdes_Res_Node *node, char *name, int subscript)</arglist>
    </member>
    <member kind="function">
      <type>void</type>
      <name>hmdes_read_restables</name>
      <anchorfile>hmdes_8c.html</anchorfile>
      <anchor>ac8add27a07c2d96cf0f917674ef99e9</anchor>
      <arglist>(Mparse_Info *pinfo)</arglist>
    </member>
    <member kind="function">
      <type>void</type>
      <name>Mread_latency</name>
      <anchorfile>hmdes_8c.html</anchorfile>
      <anchor>22d3dd7a1d22829c6e1ddf7a1b3f51ab</anchor>
      <arglist>(Mparse_Info *pinfo, int *ptr)</arglist>
    </member>
    <member kind="function">
      <type>void</type>
      <name>hmdes_read_latencies</name>
      <anchorfile>hmdes_8c.html</anchorfile>
      <anchor>471fe0226c28abff0c284e7249168862</anchor>
      <arglist>(Mparse_Info *pinfo)</arglist>
    </member>
    <member kind="function">
      <type>Hmdes_Operation_Class *</type>
      <name>hmdes_read_class_def</name>
      <anchorfile>hmdes_8c.html</anchorfile>
      <anchor>6f29bb3b7b186a76438eb0c3c07bcc30</anchor>
      <arglist>(Mparse_Info *pinfo, char *class_name)</arglist>
    </member>
    <member kind="function">
      <type>void</type>
      <name>hmdes_read_operation_class</name>
      <anchorfile>hmdes_8c.html</anchorfile>
      <anchor>db94157699c98fc24e869b0a9bf62d27</anchor>
      <arglist>(Mparse_Info *pinfo)</arglist>
    </member>
    <member kind="function">
      <type>void</type>
      <name>Mfree_flag_list</name>
      <anchorfile>hmdes_8c.html</anchorfile>
      <anchor>a12ab0bf2d586730b3c3248f688921af</anchor>
      <arglist>(Hmdes_Flag_List *flag_list)</arglist>
    </member>
    <member kind="function">
      <type>int</type>
      <name>Mmatch_flag_lists</name>
      <anchorfile>hmdes_8c.html</anchorfile>
      <anchor>061dc0415202f6c4f9e89f83b5fe543b</anchor>
      <arglist>(Hmdes_Flag_List *list1, Hmdes_Flag_List *list2)</arglist>
    </member>
    <member kind="function">
      <type>int</type>
      <name>valid_flag_value</name>
      <anchorfile>hmdes_8c.html</anchorfile>
      <anchor>a3a7e52722cb239b9007ac64a3dbd605</anchor>
      <arglist>(Mparse_Info *pinfo, Hmdes_Flag *flag)</arglist>
    </member>
    <member kind="function">
      <type>void</type>
      <name>Mread_flag</name>
      <anchorfile>hmdes_8c.html</anchorfile>
      <anchor>bfe7010416fe99e78e226c8f3a71815a</anchor>
      <arglist>(Mparse_Info *pinfo, Hmdes_Flag_List *flag_list, char *flag_prefix, char *desc)</arglist>
    </member>
    <member kind="function">
      <type>void</type>
      <name>hmdes_read_operations</name>
      <anchorfile>hmdes_8c.html</anchorfile>
      <anchor>59ea256e06da9fa9c177ec00dd71a4da</anchor>
      <arglist>(Mparse_Info *pinfo)</arglist>
    </member>
    <member kind="function">
      <type>void</type>
      <name>Mprint_op</name>
      <anchorfile>hmdes_8c.html</anchorfile>
      <anchor>009c16532ab5e1731f3a41830e88f47d</anchor>
      <arglist>(FILE *out, Hmdes *hmdes, char *name)</arglist>
    </member>
    <member kind="function">
      <type>int</type>
      <name>hmdes_section_end</name>
      <anchorfile>hmdes_8c.html</anchorfile>
      <anchor>169537c1ae702279208ac7b91eb1869b</anchor>
      <arglist>(Mparse_Info *pinfo)</arglist>
    </member>
    <member kind="function">
      <type>int</type>
      <name>Mpeek_ahead</name>
      <anchorfile>hmdes_8c.html</anchorfile>
      <anchor>d1b202220f173d1a1cf133ed53e7de09</anchor>
      <arglist>(Mparse_Info *pinfo, int dist, char *buf)</arglist>
    </member>
    <member kind="function">
      <type>int</type>
      <name>Mget_next</name>
      <anchorfile>hmdes_8c.html</anchorfile>
      <anchor>3048440efa1af4a82f688116e0e0c54b</anchor>
      <arglist>(Mparse_Info *pinfo, char *buf)</arglist>
    </member>
    <member kind="function">
      <type>void</type>
      <name>L_preprocess_line</name>
      <anchorfile>hmdes_8c.html</anchorfile>
      <anchor>9044d5ee723efcef9552470226662186</anchor>
      <arglist>(Mparse_Info *pinfo, char *raw_line, char *buf, int max)</arglist>
    </member>
    <member kind="function">
      <type>int</type>
      <name>Misspace</name>
      <anchorfile>hmdes_8c.html</anchorfile>
      <anchor>7812dadb15947686c187df7d3d07ffbc</anchor>
      <arglist>(char ch)</arglist>
    </member>
    <member kind="function">
      <type>char *</type>
      <name>Mstrip</name>
      <anchorfile>hmdes_8c.html</anchorfile>
      <anchor>4fcb92f95a2a4cd3b2beefcf2f8afef8</anchor>
      <arglist>(char *buf)</arglist>
    </member>
    <member kind="function">
      <type>Mparse_Info *</type>
      <name>create_mparse_info</name>
      <anchorfile>hmdes_8c.html</anchorfile>
      <anchor>adacad712867b0c7c17e6329ee3a0d98</anchor>
      <arglist>(char *name, Hmdes *hmdes)</arglist>
    </member>
    <member kind="function">
      <type>void</type>
      <name>Mparse_warn</name>
      <anchorfile>hmdes_8c.html</anchorfile>
      <anchor>d75289a172681118dcf3d2dc27779d8f</anchor>
      <arglist>(Mparse_Info *pinfo, char *fmt,...)</arglist>
    </member>
    <member kind="function">
      <type>void</type>
      <name>Mparse_fatal</name>
      <anchorfile>hmdes_8c.html</anchorfile>
      <anchor>2b6eb17357b8952a79a4e3d4ce84a2ae</anchor>
      <arglist>(Mparse_Info *pinfo, char *fmt,...)</arglist>
    </member>
    <member kind="function">
      <type>void</type>
      <name>Mparse_error</name>
      <anchorfile>hmdes_8c.html</anchorfile>
      <anchor>de1ca8ab012e8d317504c9c9b16f566d</anchor>
      <arglist>(Mparse_Info *pinfo, char *fmt,...)</arglist>
    </member>
    <member kind="function">
      <type>int</type>
      <name>Mmatch</name>
      <anchorfile>hmdes_8c.html</anchorfile>
      <anchor>d168399f07589e6ab6fad5c3ca7c6336</anchor>
      <arglist>(char *s1, char *s2)</arglist>
    </member>
    <member kind="function">
      <type>void</type>
      <name>associate_reg_files</name>
      <anchorfile>hmdes_8c.html</anchorfile>
      <anchor>d97bc6015f7945e01a4eede7f79ae0d1</anchor>
      <arglist>(Hmdes *hmdes)</arglist>
    </member>
    <member kind="function">
      <type>void</type>
      <name>associate_opcodes</name>
      <anchorfile>hmdes_8c.html</anchorfile>
      <anchor>2a96f6a3e990175d267175838cda8cf4</anchor>
      <arglist>(Hmdes *hmdes)</arglist>
    </member>
    <member kind="function">
      <type>void</type>
      <name>clear_mask</name>
      <anchorfile>hmdes_8c.html</anchorfile>
      <anchor>88f18e1a13f5f5d2b335ab3b5de0c127</anchor>
      <arglist>(int *mask, int size)</arglist>
    </member>
    <member kind="function">
      <type>void</type>
      <name>set_mask_bit</name>
      <anchorfile>hmdes_8c.html</anchorfile>
      <anchor>93982c65f2c3e166d73a84df641bbc40</anchor>
      <arglist>(int *mask, unsigned int bit)</arglist>
    </member>
    <member kind="function">
      <type>void</type>
      <name>print_mask</name>
      <anchorfile>hmdes_8c.html</anchorfile>
      <anchor>bdc578c791b46b07c9c23509b76545ca</anchor>
      <arglist>(FILE *out, int *mask, int size)</arglist>
    </member>
    <member kind="function">
      <type>void</type>
      <name>write_res_node</name>
      <anchorfile>hmdes_8c.html</anchorfile>
      <anchor>e58754ae0492339071171cc7f265095f</anchor>
      <arglist>(FILE *out, int mask_size, Hmdes_Res_Node *res_node)</arglist>
    </member>
    <member kind="function">
      <type>void</type>
      <name>write_lmdes</name>
      <anchorfile>hmdes_8c.html</anchorfile>
      <anchor>c02228103228aa8bc10596f96ca952bb</anchor>
      <arglist>(Hmdes *hmdes, FILE *out)</arglist>
    </member>
    <member kind="function">
      <type>void</type>
      <name>H_punt</name>
      <anchorfile>hmdes_8c.html</anchorfile>
      <anchor>4b5d684504778c8d474321a099a3c4b2</anchor>
      <arglist>(char *fmt,...)</arglist>
    </member>
    <member kind="variable">
      <type>int</type>
      <name>verbose</name>
      <anchorfile>hmdes_8c.html</anchorfile>
      <anchor>0b2caeb4b6f130be43e5a2f0267dd453</anchor>
      <arglist></arglist>
    </member>
    <member kind="variable" static="yes">
      <type>static L_Alloc_Pool *</type>
      <name>Mparse_Item_pool</name>
      <anchorfile>hmdes_8c.html</anchorfile>
      <anchor>c7dbbbf62127c82c8f72ef0aafab14ab</anchor>
      <arglist></arglist>
    </member>
    <member kind="variable">
      <type>int</type>
      <name>minimize_res_lists</name>
      <anchorfile>hmdes_8c.html</anchorfile>
      <anchor>1e78704d5e8ec4618d699113578f6ac7</anchor>
      <arglist></arglist>
    </member>
    <member kind="variable">
      <type>int</type>
      <name>fatal_parse_errors</name>
      <anchorfile>hmdes_8c.html</anchorfile>
      <anchor>d92f09f56ce353d319f5fb4f8388ddf6</anchor>
      <arglist></arglist>
    </member>
    <member kind="variable">
      <type>Header_File</type>
      <name>header_table</name>
      <anchorfile>hmdes_8c.html</anchorfile>
      <anchor>532fef42f980f82744b7a6cfd1999548</anchor>
      <arglist>[MAX_HEADER_FILES]</arglist>
    </member>
    <member kind="variable">
      <type>int</type>
      <name>num_header_files</name>
      <anchorfile>hmdes_8c.html</anchorfile>
      <anchor>1771c66351c3c7fd849af851b3e31455</anchor>
      <arglist></arglist>
    </member>
  </compound>
  <compound kind="file">
    <name>hmdes.h</name>
    <path>/mnt/hgfs/mkt/Desktop/openimpact/src/machine/hmdes/</path>
    <filename>hmdes_8h</filename>
    <class kind="struct">msymbol_st</class>
    <class kind="struct">msymbol_table_st</class>
    <class kind="struct">header_file_st</class>
    <class kind="struct">hmdes_reg_file_st</class>
    <class kind="struct">hmdes_io_node_st</class>
    <class kind="struct">hmdes_io_set_st</class>
    <class kind="struct">hmdes_io_item_st</class>
    <class kind="struct">hmdes_res_sub_st</class>
    <class kind="struct">hmdes_resource_st</class>
    <class kind="struct">hmdes_res_option_st</class>
    <class kind="struct">hmdes_res_node_st</class>
    <class kind="struct">hmdes_res_list_st</class>
    <class kind="struct">hmdes_latency_st</class>
    <class kind="struct">hmdes_class_node_st</class>
    <class kind="struct">hmdes_operation_class_st</class>
    <class kind="struct">hmdes_flag_st</class>
    <class kind="struct">hmdes_flag_list_st</class>
    <class kind="struct">hmdes_operation_node_st</class>
    <class kind="struct">hmdes_operation_st</class>
    <class kind="struct">hmdes_st</class>
    <class kind="struct">mparse_item_st</class>
    <class kind="struct">mparse_info_st</class>
    <member kind="define">
      <type>#define</type>
      <name>MITEM_SIZE</name>
      <anchorfile>hmdes_8h.html</anchorfile>
      <anchor>268d56812ab032343942979976e53ba9</anchor>
      <arglist></arglist>
    </member>
    <member kind="define">
      <type>#define</type>
      <name>MLINE_SIZE</name>
      <anchorfile>hmdes_8h.html</anchorfile>
      <anchor>4253cac11fcc1fba152b18205cc28a91</anchor>
      <arglist></arglist>
    </member>
    <member kind="define">
      <type>#define</type>
      <name>MAX_HEADER_FILES</name>
      <anchorfile>hmdes_8h.html</anchorfile>
      <anchor>3c4b12b095a395929d29e131bd140819</anchor>
      <arglist></arglist>
    </member>
    <member kind="define">
      <type>#define</type>
      <name>MSYMBOL_TABLE_SIZE</name>
      <anchorfile>hmdes_8h.html</anchorfile>
      <anchor>527d6f7ae08e4300987d486fe6736f29</anchor>
      <arglist></arglist>
    </member>
    <member kind="define">
      <type>#define</type>
      <name>MAX_MASK_SIZE</name>
      <anchorfile>hmdes_8h.html</anchorfile>
      <anchor>b28a82e5fcd0042f52dda8a7d0791904</anchor>
      <arglist></arglist>
    </member>
    <member kind="define">
      <type>#define</type>
      <name>MDES_SUPERSCALAR</name>
      <anchorfile>hmdes_8h.html</anchorfile>
      <anchor>2202ce53709c0e408a63030c29fead58</anchor>
      <arglist></arglist>
    </member>
    <member kind="define">
      <type>#define</type>
      <name>MDES_VLIW</name>
      <anchorfile>hmdes_8h.html</anchorfile>
      <anchor>1dd3b99b38350ea5ad6a1ae70624f7a4</anchor>
      <arglist></arglist>
    </member>
    <member kind="define">
      <type>#define</type>
      <name>MDES_OVERLAPPING_REQUEST</name>
      <anchorfile>hmdes_8h.html</anchorfile>
      <anchor>66e95518415e6886ce1ea4fea1f11f2a</anchor>
      <arglist></arglist>
    </member>
    <member kind="typedef">
      <type>msymbol_st</type>
      <name>Msymbol</name>
      <anchorfile>hmdes_8h.html</anchorfile>
      <anchor>5e724ae2243f689ae2ef6979669fcaa7</anchor>
      <arglist></arglist>
    </member>
    <member kind="typedef">
      <type>msymbol_table_st</type>
      <name>Msymbol_Table</name>
      <anchorfile>hmdes_8h.html</anchorfile>
      <anchor>e5a05ce585dab6419dab26c94c901403</anchor>
      <arglist></arglist>
    </member>
    <member kind="typedef">
      <type>header_file_st</type>
      <name>Header_File</name>
      <anchorfile>hmdes_8h.html</anchorfile>
      <anchor>36de1b6403148ee277b3472e18e5ab88</anchor>
      <arglist></arglist>
    </member>
    <member kind="typedef">
      <type>hmdes_reg_file_st</type>
      <name>Hmdes_Reg_File</name>
      <anchorfile>hmdes_8h.html</anchorfile>
      <anchor>983b10d23d3f39aca8e877270406d2ce</anchor>
      <arglist></arglist>
    </member>
    <member kind="typedef">
      <type>hmdes_io_node_st</type>
      <name>Hmdes_IO_Node</name>
      <anchorfile>hmdes_8h.html</anchorfile>
      <anchor>43b96c8cbffca3541e101aebfc084383</anchor>
      <arglist></arglist>
    </member>
    <member kind="typedef">
      <type>hmdes_io_set_st</type>
      <name>Hmdes_IO_Set</name>
      <anchorfile>hmdes_8h.html</anchorfile>
      <anchor>a27b62ab181acc58f551ed7f624d1252</anchor>
      <arglist></arglist>
    </member>
    <member kind="typedef">
      <type>hmdes_io_item_st</type>
      <name>Hmdes_IO_Item</name>
      <anchorfile>hmdes_8h.html</anchorfile>
      <anchor>b3d75b7f82fb8c4a976c45b13061fd7f</anchor>
      <arglist></arglist>
    </member>
    <member kind="typedef">
      <type>hmdes_res_sub_st</type>
      <name>Hmdes_Res_Sub</name>
      <anchorfile>hmdes_8h.html</anchorfile>
      <anchor>c6a0cf287c176c9939ed32e8d11061e3</anchor>
      <arglist></arglist>
    </member>
    <member kind="typedef">
      <type>hmdes_resource_st</type>
      <name>Hmdes_Resource</name>
      <anchorfile>hmdes_8h.html</anchorfile>
      <anchor>822804ceff178a4f2397f3b395ea7755</anchor>
      <arglist></arglist>
    </member>
    <member kind="typedef">
      <type>hmdes_res_option_st</type>
      <name>Hmdes_Res_Option</name>
      <anchorfile>hmdes_8h.html</anchorfile>
      <anchor>5cdd8808b70c123e7582d8ebdd83d02c</anchor>
      <arglist></arglist>
    </member>
    <member kind="typedef">
      <type>hmdes_res_node_st</type>
      <name>Hmdes_Res_Node</name>
      <anchorfile>hmdes_8h.html</anchorfile>
      <anchor>30d58325a4e7fd5ba31e8ce9ad5ea6bc</anchor>
      <arglist></arglist>
    </member>
    <member kind="typedef">
      <type>hmdes_res_list_st</type>
      <name>Hmdes_Res_List</name>
      <anchorfile>hmdes_8h.html</anchorfile>
      <anchor>e9387d7ee16a7e21a25a1e8d207e72c0</anchor>
      <arglist></arglist>
    </member>
    <member kind="typedef">
      <type>hmdes_latency_st</type>
      <name>Hmdes_Latency</name>
      <anchorfile>hmdes_8h.html</anchorfile>
      <anchor>0a27f3716bdb1e7eacbeee53a7323fed</anchor>
      <arglist></arglist>
    </member>
    <member kind="typedef">
      <type>hmdes_class_node_st</type>
      <name>Hmdes_Class_Node</name>
      <anchorfile>hmdes_8h.html</anchorfile>
      <anchor>335815ddabbc6da96d9a56b59024cceb</anchor>
      <arglist></arglist>
    </member>
    <member kind="typedef">
      <type>hmdes_operation_class_st</type>
      <name>Hmdes_Operation_Class</name>
      <anchorfile>hmdes_8h.html</anchorfile>
      <anchor>bc8ab655d23fc52e2e2ff5e84cee51e5</anchor>
      <arglist></arglist>
    </member>
    <member kind="typedef">
      <type>hmdes_flag_st</type>
      <name>Hmdes_Flag</name>
      <anchorfile>hmdes_8h.html</anchorfile>
      <anchor>36e2036902cf69a7edc17e9c8eb97cfd</anchor>
      <arglist></arglist>
    </member>
    <member kind="typedef">
      <type>hmdes_flag_list_st</type>
      <name>Hmdes_Flag_List</name>
      <anchorfile>hmdes_8h.html</anchorfile>
      <anchor>7eb8dbef728c32af9f107d338ae2b35e</anchor>
      <arglist></arglist>
    </member>
    <member kind="typedef">
      <type>hmdes_operation_node_st</type>
      <name>Hmdes_Operation_Node</name>
      <anchorfile>hmdes_8h.html</anchorfile>
      <anchor>59aafbdb8b82cf078ff43173a4777e29</anchor>
      <arglist></arglist>
    </member>
    <member kind="typedef">
      <type>hmdes_operation_st</type>
      <name>Hmdes_Operation</name>
      <anchorfile>hmdes_8h.html</anchorfile>
      <anchor>b53bbddaa84826dc6ef4c033b3727d8e</anchor>
      <arglist></arglist>
    </member>
    <member kind="typedef">
      <type>hmdes_st</type>
      <name>Hmdes</name>
      <anchorfile>hmdes_8h.html</anchorfile>
      <anchor>17c125f01b5474bfde1034df85b31854</anchor>
      <arglist></arglist>
    </member>
    <member kind="typedef">
      <type>mparse_item_st</type>
      <name>Mparse_Item</name>
      <anchorfile>hmdes_8h.html</anchorfile>
      <anchor>85add3d7236a5fe7baa3edfccf397a01</anchor>
      <arglist></arglist>
    </member>
    <member kind="typedef">
      <type>mparse_info_st</type>
      <name>Mparse_Info</name>
      <anchorfile>hmdes_8h.html</anchorfile>
      <anchor>8c87afd86709da1dd0b7a1c3faa73286</anchor>
      <arglist></arglist>
    </member>
    <member kind="function">
      <type>Msymbol_Table *</type>
      <name>Hread_header_file</name>
      <anchorfile>hmdes_8h.html</anchorfile>
      <anchor>2e6daf4ca9e19d27b1a6df7911acff59</anchor>
      <arglist>(char *file_name)</arglist>
    </member>
    <member kind="function">
      <type>void</type>
      <name>Mdie_on_fatal_errors</name>
      <anchorfile>hmdes_8h.html</anchorfile>
      <anchor>c86bbf19bc7a087f23ce6dabbcc0ba13</anchor>
      <arglist>(Mparse_Info *pinfo)</arglist>
    </member>
    <member kind="function">
      <type>void</type>
      <name>Hbuild_environment_defines</name>
      <anchorfile>hmdes_8h.html</anchorfile>
      <anchor>ff072a5397cbb97e07781e19a7a213e5</anchor>
      <arglist>(Msymbol_Table *env_tab, char **envp)</arglist>
    </member>
    <member kind="function">
      <type>void</type>
      <name>Hbuild_command_line_defines</name>
      <anchorfile>hmdes_8h.html</anchorfile>
      <anchor>a80c53a0b9e7a49dbeaed5b087e74ff6</anchor>
      <arglist>(Msymbol_Table *cl_tab, char **argv)</arglist>
    </member>
    <member kind="function">
      <type>Hmdes *</type>
      <name>create_hmdes</name>
      <anchorfile>hmdes_8h.html</anchorfile>
      <anchor>3157c981faa1f80fffce64fe8a1685b2</anchor>
      <arglist>(char *hmdes_file_name, char **argv, char **envp)</arglist>
    </member>
    <member kind="function">
      <type>void</type>
      <name>hmdes_read_section</name>
      <anchorfile>hmdes_8h.html</anchorfile>
      <anchor>f122d4b6dad387d560f2445048d82f3f</anchor>
      <arglist>(Mparse_Info *pinfo, char *name, void(*func)())</arglist>
    </member>
    <member kind="function">
      <type>void</type>
      <name>hmdes_read_define</name>
      <anchorfile>hmdes_8h.html</anchorfile>
      <anchor>a3f0d60b373219c49efda8d55324f992</anchor>
      <arglist>(Mparse_Info *pinfo, char *sname)</arglist>
    </member>
    <member kind="function">
      <type>int</type>
      <name>Mis_name_next</name>
      <anchorfile>hmdes_8h.html</anchorfile>
      <anchor>7040d7057210a1a7921077362a420f82</anchor>
      <arglist>(Mparse_Info *pinfo)</arglist>
    </member>
    <member kind="function">
      <type>int</type>
      <name>Mis_next</name>
      <anchorfile>hmdes_8h.html</anchorfile>
      <anchor>8233d14a6a48cff6d2bd467a11583d87</anchor>
      <arglist>(Mparse_Info *pinfo, char *str)</arglist>
    </member>
    <member kind="function">
      <type>void</type>
      <name>Mread_name</name>
      <anchorfile>hmdes_8h.html</anchorfile>
      <anchor>e2d457c134c37f83e6c72d6c0f9d2281</anchor>
      <arglist>(Mparse_Info *pinfo, char *name, char *desc)</arglist>
    </member>
    <member kind="function">
      <type>void</type>
      <name>Mread_symbol</name>
      <anchorfile>hmdes_8h.html</anchorfile>
      <anchor>f06d6c42e20b1f16afe72019cce9feb3</anchor>
      <arglist>(Mparse_Info *pinfo, Msymbol_Table *table, void **ptr, char *desc)</arglist>
    </member>
    <member kind="function">
      <type>void</type>
      <name>Mread_IO_item_symbol</name>
      <anchorfile>hmdes_8h.html</anchorfile>
      <anchor>a4ef52a27eef051d7a947f6dfe72b35d</anchor>
      <arglist>(Mparse_Info *pinfo, Msymbol_Table *table, void **ptr, char *terminator)</arglist>
    </member>
    <member kind="function">
      <type>void</type>
      <name>Mread_match</name>
      <anchorfile>hmdes_8h.html</anchorfile>
      <anchor>6596ec52b951f99374fa8896ea78aa96</anchor>
      <arglist>(Mparse_Info *pinfo, char *str)</arglist>
    </member>
    <member kind="function">
      <type>int</type>
      <name>mscan_set</name>
      <anchorfile>hmdes_8h.html</anchorfile>
      <anchor>e31ac28eb1189aa14d222358f54daaae</anchor>
      <arglist>(Mparse_Info *pinfo, char *name)</arglist>
    </member>
    <member kind="function">
      <type>int</type>
      <name>mread_proc_model</name>
      <anchorfile>hmdes_8h.html</anchorfile>
      <anchor>a0d5044667c2d6c7abc5ecc0f0cc045f</anchor>
      <arglist>(Mparse_Info *pinfo, char *name, int *ptr)</arglist>
    </member>
    <member kind="function">
      <type>int</type>
      <name>mread_define_var</name>
      <anchorfile>hmdes_8h.html</anchorfile>
      <anchor>863ffb908bbc23df92008f34e8650173</anchor>
      <arglist>(Mparse_Info *pinfo, char *name, int *ptr)</arglist>
    </member>
    <member kind="function">
      <type>void</type>
      <name>Malloc_struct</name>
      <anchorfile>hmdes_8h.html</anchorfile>
      <anchor>44598a63bd3f29a90fdd27cb3531aff7</anchor>
      <arglist>(void **ptr, int size)</arglist>
    </member>
    <member kind="function">
      <type>void</type>
      <name>Malloc_name</name>
      <anchorfile>hmdes_8h.html</anchorfile>
      <anchor>94186a85a4489b80b7d305e6e13fec4c</anchor>
      <arglist>(char **ptr, char *name)</arglist>
    </member>
    <member kind="function">
      <type>int</type>
      <name>Mread_int</name>
      <anchorfile>hmdes_8h.html</anchorfile>
      <anchor>21167fd2307af816d91d77546db04321</anchor>
      <arglist>(Mparse_Info *pinfo, int *ptr)</arglist>
    </member>
    <member kind="function">
      <type>Msymbol_Table *</type>
      <name>Mcreate_symbol_table</name>
      <anchorfile>hmdes_8h.html</anchorfile>
      <anchor>9a7308042188883a4be25b3e0ee777d0</anchor>
      <arglist>(char *name, int size)</arglist>
    </member>
    <member kind="function">
      <type>void *</type>
      <name>Mfind_symbol</name>
      <anchorfile>hmdes_8h.html</anchorfile>
      <anchor>6bcfff652cd0e21dc304d3b05c230a52</anchor>
      <arglist>(Msymbol_Table *table, char *name)</arglist>
    </member>
    <member kind="function">
      <type>void</type>
      <name>Minsert_unique_symbol</name>
      <anchorfile>hmdes_8h.html</anchorfile>
      <anchor>5ac6a523a50fd5a52219a1820c8ea6ad</anchor>
      <arglist>(Mparse_Info *pinfo, Msymbol_Table *table, char *name, void *ptr)</arglist>
    </member>
    <member kind="function">
      <type>void</type>
      <name>Minsert_symbol</name>
      <anchorfile>hmdes_8h.html</anchorfile>
      <anchor>74d2d13e74689801a91905ef832e65af</anchor>
      <arglist>(Msymbol_Table *table, char *name, void *ptr)</arglist>
    </member>
    <member kind="function">
      <type>void</type>
      <name>Mprint_symbol_table</name>
      <anchorfile>hmdes_8h.html</anchorfile>
      <anchor>e02a2eb880e45436edfaea7651aa8288</anchor>
      <arglist>(FILE *out, Msymbol_Table *table)</arglist>
    </member>
    <member kind="function">
      <type>int</type>
      <name>Mhash_name</name>
      <anchorfile>hmdes_8h.html</anchorfile>
      <anchor>bd9725ae2e255beb2f27b69261855d05</anchor>
      <arglist>(char *name, int mask)</arglist>
    </member>
    <member kind="function">
      <type>int</type>
      <name>find_header_value</name>
      <anchorfile>hmdes_8h.html</anchorfile>
      <anchor>c172781c19adf04abeb09bc1d5769470</anchor>
      <arglist>(Mparse_Info *pinfo, char *name, int *ptr)</arglist>
    </member>
    <member kind="function">
      <type>void</type>
      <name>hmdes_read_register_files</name>
      <anchorfile>hmdes_8h.html</anchorfile>
      <anchor>6151685cbaff3a77aa055033dfc491da</anchor>
      <arglist>(Mparse_Info *pinfo)</arglist>
    </member>
    <member kind="function">
      <type>void</type>
      <name>Madd_IO_node</name>
      <anchorfile>hmdes_8h.html</anchorfile>
      <anchor>8dae87d00a072289aa6110f4a050834f</anchor>
      <arglist>(Hmdes_IO_Set *IO_set, Hmdes_Reg_File *reg_file)</arglist>
    </member>
    <member kind="function">
      <type>void</type>
      <name>hmdes_read_IO_sets</name>
      <anchorfile>hmdes_8h.html</anchorfile>
      <anchor>dd0bc36d354a0e2a80aa51d52fa69bf9</anchor>
      <arglist>(Mparse_Info *pinfo)</arglist>
    </member>
    <member kind="function">
      <type>void</type>
      <name>hmdes_read_IO_items</name>
      <anchorfile>hmdes_8h.html</anchorfile>
      <anchor>f5c6d475d646d0b90cc6f70e003c9bb8</anchor>
      <arglist>(Mparse_Info *pinfo)</arglist>
    </member>
    <member kind="function">
      <type>void</type>
      <name>hmdes_add_res_subscript</name>
      <anchorfile>hmdes_8h.html</anchorfile>
      <anchor>c3ce0111eb5b22ea3eb78d74799c9512</anchor>
      <arglist>(Mparse_Info *pinfo, Hmdes_Resource *resource, int subscript, int id)</arglist>
    </member>
    <member kind="function">
      <type>void</type>
      <name>hmdes_read_resources</name>
      <anchorfile>hmdes_8h.html</anchorfile>
      <anchor>cd1ff0019bf0cc483c1ec95416c53450</anchor>
      <arglist>(Mparse_Info *pinfo)</arglist>
    </member>
    <member kind="function">
      <type>void</type>
      <name>Madd_res_option</name>
      <anchorfile>hmdes_8h.html</anchorfile>
      <anchor>c44837bb65e6328e99ae414c9ee8499c</anchor>
      <arglist>(Mparse_Info *pinfo, Hmdes_Res_Node *node, char *name, int subscript)</arglist>
    </member>
    <member kind="function">
      <type>void</type>
      <name>hmdes_read_restables</name>
      <anchorfile>hmdes_8h.html</anchorfile>
      <anchor>ac8add27a07c2d96cf0f917674ef99e9</anchor>
      <arglist>(Mparse_Info *pinfo)</arglist>
    </member>
    <member kind="function">
      <type>void</type>
      <name>Mread_latency</name>
      <anchorfile>hmdes_8h.html</anchorfile>
      <anchor>22d3dd7a1d22829c6e1ddf7a1b3f51ab</anchor>
      <arglist>(Mparse_Info *pinfo, int *ptr)</arglist>
    </member>
    <member kind="function">
      <type>void</type>
      <name>hmdes_read_latencies</name>
      <anchorfile>hmdes_8h.html</anchorfile>
      <anchor>471fe0226c28abff0c284e7249168862</anchor>
      <arglist>(Mparse_Info *pinfo)</arglist>
    </member>
    <member kind="function">
      <type>Hmdes_Operation_Class *</type>
      <name>hmdes_read_class_def</name>
      <anchorfile>hmdes_8h.html</anchorfile>
      <anchor>6f29bb3b7b186a76438eb0c3c07bcc30</anchor>
      <arglist>(Mparse_Info *pinfo, char *class_name)</arglist>
    </member>
    <member kind="function">
      <type>void</type>
      <name>hmdes_read_operation_class</name>
      <anchorfile>hmdes_8h.html</anchorfile>
      <anchor>db94157699c98fc24e869b0a9bf62d27</anchor>
      <arglist>(Mparse_Info *pinfo)</arglist>
    </member>
    <member kind="function">
      <type>void</type>
      <name>Mfree_flag_list</name>
      <anchorfile>hmdes_8h.html</anchorfile>
      <anchor>a12ab0bf2d586730b3c3248f688921af</anchor>
      <arglist>(Hmdes_Flag_List *flag_list)</arglist>
    </member>
    <member kind="function">
      <type>int</type>
      <name>Mmatch_flag_lists</name>
      <anchorfile>hmdes_8h.html</anchorfile>
      <anchor>061dc0415202f6c4f9e89f83b5fe543b</anchor>
      <arglist>(Hmdes_Flag_List *list1, Hmdes_Flag_List *list2)</arglist>
    </member>
    <member kind="function">
      <type>int</type>
      <name>valid_flag_value</name>
      <anchorfile>hmdes_8h.html</anchorfile>
      <anchor>a3a7e52722cb239b9007ac64a3dbd605</anchor>
      <arglist>(Mparse_Info *pinfo, Hmdes_Flag *flag)</arglist>
    </member>
    <member kind="function">
      <type>void</type>
      <name>Mread_flag</name>
      <anchorfile>hmdes_8h.html</anchorfile>
      <anchor>bfe7010416fe99e78e226c8f3a71815a</anchor>
      <arglist>(Mparse_Info *pinfo, Hmdes_Flag_List *flag_list, char *flag_prefix, char *desc)</arglist>
    </member>
    <member kind="function">
      <type>void</type>
      <name>hmdes_read_operations</name>
      <anchorfile>hmdes_8h.html</anchorfile>
      <anchor>59ea256e06da9fa9c177ec00dd71a4da</anchor>
      <arglist>(Mparse_Info *pinfo)</arglist>
    </member>
    <member kind="function">
      <type>void</type>
      <name>Mprint_op</name>
      <anchorfile>hmdes_8h.html</anchorfile>
      <anchor>009c16532ab5e1731f3a41830e88f47d</anchor>
      <arglist>(FILE *out, Hmdes *hmdes, char *name)</arglist>
    </member>
    <member kind="function">
      <type>int</type>
      <name>hmdes_section_end</name>
      <anchorfile>hmdes_8h.html</anchorfile>
      <anchor>169537c1ae702279208ac7b91eb1869b</anchor>
      <arglist>(Mparse_Info *pinfo)</arglist>
    </member>
    <member kind="function">
      <type>int</type>
      <name>Mpeek_ahead</name>
      <anchorfile>hmdes_8h.html</anchorfile>
      <anchor>d1b202220f173d1a1cf133ed53e7de09</anchor>
      <arglist>(Mparse_Info *pinfo, int dist, char *buf)</arglist>
    </member>
    <member kind="function">
      <type>int</type>
      <name>Mget_next</name>
      <anchorfile>hmdes_8h.html</anchorfile>
      <anchor>3048440efa1af4a82f688116e0e0c54b</anchor>
      <arglist>(Mparse_Info *pinfo, char *buf)</arglist>
    </member>
    <member kind="function">
      <type>void</type>
      <name>L_preprocess_line</name>
      <anchorfile>hmdes_8h.html</anchorfile>
      <anchor>9044d5ee723efcef9552470226662186</anchor>
      <arglist>(Mparse_Info *pinfo, char *raw_line, char *buf, int max)</arglist>
    </member>
    <member kind="function">
      <type>int</type>
      <name>Misspace</name>
      <anchorfile>hmdes_8h.html</anchorfile>
      <anchor>7812dadb15947686c187df7d3d07ffbc</anchor>
      <arglist>(char ch)</arglist>
    </member>
    <member kind="function">
      <type>char *</type>
      <name>Mstrip</name>
      <anchorfile>hmdes_8h.html</anchorfile>
      <anchor>4fcb92f95a2a4cd3b2beefcf2f8afef8</anchor>
      <arglist>(char *buf)</arglist>
    </member>
    <member kind="function">
      <type>Mparse_Info *</type>
      <name>create_mparse_info</name>
      <anchorfile>hmdes_8h.html</anchorfile>
      <anchor>adacad712867b0c7c17e6329ee3a0d98</anchor>
      <arglist>(char *name, Hmdes *hmdes)</arglist>
    </member>
    <member kind="function">
      <type>void</type>
      <name>Mparse_warn</name>
      <anchorfile>hmdes_8h.html</anchorfile>
      <anchor>d75289a172681118dcf3d2dc27779d8f</anchor>
      <arglist>(Mparse_Info *pinfo, char *fmt,...)</arglist>
    </member>
    <member kind="function">
      <type>void</type>
      <name>Mparse_fatal</name>
      <anchorfile>hmdes_8h.html</anchorfile>
      <anchor>2b6eb17357b8952a79a4e3d4ce84a2ae</anchor>
      <arglist>(Mparse_Info *pinfo, char *fmt,...)</arglist>
    </member>
    <member kind="function">
      <type>void</type>
      <name>Mparse_error</name>
      <anchorfile>hmdes_8h.html</anchorfile>
      <anchor>de1ca8ab012e8d317504c9c9b16f566d</anchor>
      <arglist>(Mparse_Info *pinfo, char *fmt,...)</arglist>
    </member>
    <member kind="function">
      <type>void</type>
      <name>H_punt</name>
      <anchorfile>hmdes_8h.html</anchorfile>
      <anchor>4b5d684504778c8d474321a099a3c4b2</anchor>
      <arglist>(char *fmt,...)</arglist>
    </member>
    <member kind="function">
      <type>int</type>
      <name>Mmatch</name>
      <anchorfile>hmdes_8h.html</anchorfile>
      <anchor>d168399f07589e6ab6fad5c3ca7c6336</anchor>
      <arglist>(char *s1, char *s2)</arglist>
    </member>
    <member kind="function">
      <type>void</type>
      <name>associate_reg_files</name>
      <anchorfile>hmdes_8h.html</anchorfile>
      <anchor>d97bc6015f7945e01a4eede7f79ae0d1</anchor>
      <arglist>(Hmdes *hmdes)</arglist>
    </member>
    <member kind="function">
      <type>void</type>
      <name>associate_opcodes</name>
      <anchorfile>hmdes_8h.html</anchorfile>
      <anchor>2a96f6a3e990175d267175838cda8cf4</anchor>
      <arglist>(Hmdes *hmdes)</arglist>
    </member>
    <member kind="function">
      <type>void</type>
      <name>clear_mask</name>
      <anchorfile>hmdes_8h.html</anchorfile>
      <anchor>88f18e1a13f5f5d2b335ab3b5de0c127</anchor>
      <arglist>(int *mask, int size)</arglist>
    </member>
    <member kind="function">
      <type>void</type>
      <name>set_mask_bit</name>
      <anchorfile>hmdes_8h.html</anchorfile>
      <anchor>93982c65f2c3e166d73a84df641bbc40</anchor>
      <arglist>(int *mask, unsigned int bit)</arglist>
    </member>
    <member kind="function">
      <type>void</type>
      <name>print_mask</name>
      <anchorfile>hmdes_8h.html</anchorfile>
      <anchor>bdc578c791b46b07c9c23509b76545ca</anchor>
      <arglist>(FILE *out, int *mask, int size)</arglist>
    </member>
    <member kind="function">
      <type>void</type>
      <name>write_res_node</name>
      <anchorfile>hmdes_8h.html</anchorfile>
      <anchor>e58754ae0492339071171cc7f265095f</anchor>
      <arglist>(FILE *out, int mask_size, Hmdes_Res_Node *res_node)</arglist>
    </member>
    <member kind="function">
      <type>void</type>
      <name>write_lmdes</name>
      <anchorfile>hmdes_8h.html</anchorfile>
      <anchor>c02228103228aa8bc10596f96ca952bb</anchor>
      <arglist>(Hmdes *hmdes, FILE *out)</arglist>
    </member>
    <member kind="variable">
      <type>int</type>
      <name>verbose</name>
      <anchorfile>hmdes_8h.html</anchorfile>
      <anchor>0b2caeb4b6f130be43e5a2f0267dd453</anchor>
      <arglist></arglist>
    </member>
  </compound>
  <compound kind="struct">
    <name>header_file_st</name>
    <filename>structheader__file__st.html</filename>
    <member kind="variable">
      <type>char *</type>
      <name>name</name>
      <anchorfile>structheader__file__st.html</anchorfile>
      <anchor>43c9f12480c2926b494091a52ae46a59</anchor>
      <arglist></arglist>
    </member>
    <member kind="variable">
      <type>Msymbol_Table *</type>
      <name>table</name>
      <anchorfile>structheader__file__st.html</anchorfile>
      <anchor>93c9ca0bd08242336760105f976f93e4</anchor>
      <arglist></arglist>
    </member>
  </compound>
  <compound kind="struct">
    <name>hmdes_class_node_st</name>
    <filename>structhmdes__class__node__st.html</filename>
    <member kind="variable">
      <type>Hmdes_IO_Item *</type>
      <name>io_item</name>
      <anchorfile>structhmdes__class__node__st.html</anchorfile>
      <anchor>e87968a8c806d06fecee0f8fa8190c17</anchor>
      <arglist></arglist>
    </member>
    <member kind="variable">
      <type>Hmdes_Res_List *</type>
      <name>res_list</name>
      <anchorfile>structhmdes__class__node__st.html</anchorfile>
      <anchor>e7634e6cf1a90dc4ca88aaf1d236886b</anchor>
      <arglist></arglist>
    </member>
    <member kind="variable">
      <type>Hmdes_Latency *</type>
      <name>latency</name>
      <anchorfile>structhmdes__class__node__st.html</anchorfile>
      <anchor>14db391f2f28c8230bea429d32f23aa0</anchor>
      <arglist></arglist>
    </member>
    <member kind="variable">
      <type>hmdes_class_node_st *</type>
      <name>next</name>
      <anchorfile>structhmdes__class__node__st.html</anchorfile>
      <anchor>4981097e77036642db9206eedbd169fe</anchor>
      <arglist></arglist>
    </member>
  </compound>
  <compound kind="struct">
    <name>hmdes_flag_list_st</name>
    <filename>structhmdes__flag__list__st.html</filename>
    <member kind="variable">
      <type>Hmdes_Flag *</type>
      <name>head</name>
      <anchorfile>structhmdes__flag__list__st.html</anchorfile>
      <anchor>91f4e2c2f8124253fae2de37ba0bec9a</anchor>
      <arglist></arglist>
    </member>
    <member kind="variable">
      <type>int</type>
      <name>num_flags</name>
      <anchorfile>structhmdes__flag__list__st.html</anchorfile>
      <anchor>a5cc9249f4dac266f5545e2b15876d80</anchor>
      <arglist></arglist>
    </member>
    <member kind="variable">
      <type>int</type>
      <name>bit_version</name>
      <anchorfile>structhmdes__flag__list__st.html</anchorfile>
      <anchor>cad4fbbc3820d213a683baad44df34d2</anchor>
      <arglist></arglist>
    </member>
  </compound>
  <compound kind="struct">
    <name>hmdes_flag_st</name>
    <filename>structhmdes__flag__st.html</filename>
    <member kind="variable">
      <type>char *</type>
      <name>name</name>
      <anchorfile>structhmdes__flag__st.html</anchorfile>
      <anchor>b25f2d4a0cd5b8252470d004e0a8c296</anchor>
      <arglist></arglist>
    </member>
    <member kind="variable">
      <type>int</type>
      <name>value</name>
      <anchorfile>structhmdes__flag__st.html</anchorfile>
      <anchor>e83a7107208bbaace699ba7b05e06bea</anchor>
      <arglist></arglist>
    </member>
    <member kind="variable">
      <type>hmdes_flag_st *</type>
      <name>next</name>
      <anchorfile>structhmdes__flag__st.html</anchorfile>
      <anchor>aa41ef0173585f7ef4e4f385e13b0e02</anchor>
      <arglist></arglist>
    </member>
  </compound>
  <compound kind="struct">
    <name>hmdes_io_item_st</name>
    <filename>structhmdes__io__item__st.html</filename>
    <member kind="variable">
      <type>int</type>
      <name>id</name>
      <anchorfile>structhmdes__io__item__st.html</anchorfile>
      <anchor>0ffc247e32ee8131b4840bab2a822989</anchor>
      <arglist></arglist>
    </member>
    <member kind="variable">
      <type>char *</type>
      <name>name</name>
      <anchorfile>structhmdes__io__item__st.html</anchorfile>
      <anchor>8607878410c73a0b45ee724f1eaebbbc</anchor>
      <arglist></arglist>
    </member>
    <member kind="variable">
      <type>Hmdes_IO_Set **</type>
      <name>src</name>
      <anchorfile>structhmdes__io__item__st.html</anchorfile>
      <anchor>6c773db376d5ad314a5647ffae81239a</anchor>
      <arglist></arglist>
    </member>
    <member kind="variable">
      <type>Hmdes_IO_Set **</type>
      <name>dest</name>
      <anchorfile>structhmdes__io__item__st.html</anchorfile>
      <anchor>99e8bb66e9a127e08832eab0393be549</anchor>
      <arglist></arglist>
    </member>
    <member kind="variable">
      <type>Hmdes_IO_Set **</type>
      <name>pred</name>
      <anchorfile>structhmdes__io__item__st.html</anchorfile>
      <anchor>eeb833889fdf62378bef2f4f0e6d700a</anchor>
      <arglist></arglist>
    </member>
  </compound>
  <compound kind="struct">
    <name>hmdes_io_node_st</name>
    <filename>structhmdes__io__node__st.html</filename>
    <member kind="variable">
      <type>Hmdes_Reg_File *</type>
      <name>reg_file</name>
      <anchorfile>structhmdes__io__node__st.html</anchorfile>
      <anchor>5ad6671c5124be7d88edb918f88bedb8</anchor>
      <arglist></arglist>
    </member>
    <member kind="variable">
      <type>hmdes_io_node_st *</type>
      <name>next</name>
      <anchorfile>structhmdes__io__node__st.html</anchorfile>
      <anchor>35fa6e1f8aae5172c6b8c2813d3cbbf0</anchor>
      <arglist></arglist>
    </member>
  </compound>
  <compound kind="struct">
    <name>hmdes_io_set_st</name>
    <filename>structhmdes__io__set__st.html</filename>
    <member kind="variable">
      <type>int</type>
      <name>id</name>
      <anchorfile>structhmdes__io__set__st.html</anchorfile>
      <anchor>097db74dde62d0b5d1fd4e149673c5ab</anchor>
      <arglist></arglist>
    </member>
    <member kind="variable">
      <type>int</type>
      <name>external_id</name>
      <anchorfile>structhmdes__io__set__st.html</anchorfile>
      <anchor>6f539ab30c7e162775e043003d8b750c</anchor>
      <arglist></arglist>
    </member>
    <member kind="variable">
      <type>char *</type>
      <name>name</name>
      <anchorfile>structhmdes__io__set__st.html</anchorfile>
      <anchor>20fb123dd7e30a2e1c06aed923787f6d</anchor>
      <arglist></arglist>
    </member>
    <member kind="variable">
      <type>Hmdes_IO_Node *</type>
      <name>head</name>
      <anchorfile>structhmdes__io__set__st.html</anchorfile>
      <anchor>4cea398cccec4faf9c0193326f443ed7</anchor>
      <arglist></arglist>
    </member>
    <member kind="variable">
      <type>Hmdes_IO_Node *</type>
      <name>tail</name>
      <anchorfile>structhmdes__io__set__st.html</anchorfile>
      <anchor>b5162446cbb5cb65ac82c0b52e42ecf4</anchor>
      <arglist></arglist>
    </member>
    <member kind="variable">
      <type>int</type>
      <name>size</name>
      <anchorfile>structhmdes__io__set__st.html</anchorfile>
      <anchor>27a7bec67b6b62ffdec3c830c3f0d203</anchor>
      <arglist></arglist>
    </member>
  </compound>
  <compound kind="struct">
    <name>hmdes_latency_st</name>
    <filename>structhmdes__latency__st.html</filename>
    <member kind="variable">
      <type>int</type>
      <name>id</name>
      <anchorfile>structhmdes__latency__st.html</anchorfile>
      <anchor>034a0cc8c1d9d9d2bbc9e13297f5b7ec</anchor>
      <arglist></arglist>
    </member>
    <member kind="variable">
      <type>char *</type>
      <name>name</name>
      <anchorfile>structhmdes__latency__st.html</anchorfile>
      <anchor>4a4687a87bf5909e45c86f4a4cf27d2e</anchor>
      <arglist></arglist>
    </member>
    <member kind="variable">
      <type>int</type>
      <name>exception</name>
      <anchorfile>structhmdes__latency__st.html</anchorfile>
      <anchor>9abed5d1517dc53232c021c69c20d1c2</anchor>
      <arglist></arglist>
    </member>
    <member kind="variable">
      <type>int *</type>
      <name>sync_src</name>
      <anchorfile>structhmdes__latency__st.html</anchorfile>
      <anchor>d183d41dc190f76ea87c630feb827e2c</anchor>
      <arglist></arglist>
    </member>
    <member kind="variable">
      <type>int *</type>
      <name>src</name>
      <anchorfile>structhmdes__latency__st.html</anchorfile>
      <anchor>9c05bb42cc63d70a71d1668b1a8eadd5</anchor>
      <arglist></arglist>
    </member>
    <member kind="variable">
      <type>int *</type>
      <name>sync_dest</name>
      <anchorfile>structhmdes__latency__st.html</anchorfile>
      <anchor>64e42361d990eaaf8d3b62e159b1ab1d</anchor>
      <arglist></arglist>
    </member>
    <member kind="variable">
      <type>int *</type>
      <name>dest</name>
      <anchorfile>structhmdes__latency__st.html</anchorfile>
      <anchor>82cb8a6b2061340b09195b92aa9fa7ed</anchor>
      <arglist></arglist>
    </member>
    <member kind="variable">
      <type>int *</type>
      <name>pred</name>
      <anchorfile>structhmdes__latency__st.html</anchorfile>
      <anchor>17a40bd9a5638cc8a4ba5df27cdd9d77</anchor>
      <arglist></arglist>
    </member>
  </compound>
  <compound kind="struct">
    <name>hmdes_operation_class_st</name>
    <filename>structhmdes__operation__class__st.html</filename>
    <member kind="variable">
      <type>char *</type>
      <name>name</name>
      <anchorfile>structhmdes__operation__class__st.html</anchorfile>
      <anchor>c790e71195282a26761ce542ac870062</anchor>
      <arglist></arglist>
    </member>
    <member kind="variable">
      <type>Hmdes_Class_Node *</type>
      <name>head</name>
      <anchorfile>structhmdes__operation__class__st.html</anchorfile>
      <anchor>48cad0fcb2f2e4e4aae44ddd577dc8a5</anchor>
      <arglist></arglist>
    </member>
    <member kind="variable">
      <type>Hmdes_Class_Node *</type>
      <name>tail</name>
      <anchorfile>structhmdes__operation__class__st.html</anchorfile>
      <anchor>0428162e3ea196f0a64e89fcd20c1680</anchor>
      <arglist></arglist>
    </member>
    <member kind="variable">
      <type>int</type>
      <name>size</name>
      <anchorfile>structhmdes__operation__class__st.html</anchorfile>
      <anchor>bbad0e6f8b200580b0cfba1ea7a1c053</anchor>
      <arglist></arglist>
    </member>
  </compound>
  <compound kind="struct">
    <name>hmdes_operation_node_st</name>
    <filename>structhmdes__operation__node__st.html</filename>
    <member kind="variable">
      <type>char *</type>
      <name>asm_name</name>
      <anchorfile>structhmdes__operation__node__st.html</anchorfile>
      <anchor>0560727d348590f71211af8974857d8d</anchor>
      <arglist></arglist>
    </member>
    <member kind="variable">
      <type>Hmdes_Operation_Class *</type>
      <name>class</name>
      <anchorfile>structhmdes__operation__node__st.html</anchorfile>
      <anchor>a2fcf76ee1ff5e758fd606717182dcbf</anchor>
      <arglist></arglist>
    </member>
    <member kind="variable">
      <type>Hmdes_Flag_List</type>
      <name>mdes_flags</name>
      <anchorfile>structhmdes__operation__node__st.html</anchorfile>
      <anchor>43aa083fb6062a5fe0d2de614b9aac4a</anchor>
      <arglist></arglist>
    </member>
    <member kind="variable">
      <type>hmdes_operation_node_st *</type>
      <name>next</name>
      <anchorfile>structhmdes__operation__node__st.html</anchorfile>
      <anchor>139fdf5d7486dd4ae3da84dca69ee66a</anchor>
      <arglist></arglist>
    </member>
  </compound>
  <compound kind="struct">
    <name>hmdes_operation_st</name>
    <filename>structhmdes__operation__st.html</filename>
    <member kind="variable">
      <type>int</type>
      <name>id</name>
      <anchorfile>structhmdes__operation__st.html</anchorfile>
      <anchor>3b1ab0398e5c7e017e1d45c3cd08ac66</anchor>
      <arglist></arglist>
    </member>
    <member kind="variable">
      <type>char *</type>
      <name>name</name>
      <anchorfile>structhmdes__operation__st.html</anchorfile>
      <anchor>fbe2e96895b480490df7a363822240ad</anchor>
      <arglist></arglist>
    </member>
    <member kind="variable">
      <type>Hmdes_Operation_Node *</type>
      <name>head</name>
      <anchorfile>structhmdes__operation__st.html</anchorfile>
      <anchor>a71caa844d56d1d29f24107be85231c5</anchor>
      <arglist></arglist>
    </member>
    <member kind="variable">
      <type>Hmdes_Operation_Node *</type>
      <name>tail</name>
      <anchorfile>structhmdes__operation__st.html</anchorfile>
      <anchor>8e8bc1ba6d615392cd73ebd6e1f317a3</anchor>
      <arglist></arglist>
    </member>
    <member kind="variable">
      <type>int</type>
      <name>size</name>
      <anchorfile>structhmdes__operation__st.html</anchorfile>
      <anchor>941b5fcc89daeafda815b79a0e4699ef</anchor>
      <arglist></arglist>
    </member>
    <member kind="variable">
      <type>Hmdes_Flag_List</type>
      <name>op_flags</name>
      <anchorfile>structhmdes__operation__st.html</anchorfile>
      <anchor>8879a131601f287239c317188d8551d1</anchor>
      <arglist></arglist>
    </member>
  </compound>
  <compound kind="struct">
    <name>hmdes_reg_file_st</name>
    <filename>structhmdes__reg__file__st.html</filename>
    <member kind="variable">
      <type>int</type>
      <name>id</name>
      <anchorfile>structhmdes__reg__file__st.html</anchorfile>
      <anchor>48f2ea1e2379bb504019fb0a5a7c2d79</anchor>
      <arglist></arglist>
    </member>
    <member kind="variable">
      <type>int</type>
      <name>external_id</name>
      <anchorfile>structhmdes__reg__file__st.html</anchorfile>
      <anchor>086fa4606ff9185000ba2f40deccb58d</anchor>
      <arglist></arglist>
    </member>
    <member kind="variable">
      <type>char *</type>
      <name>name</name>
      <anchorfile>structhmdes__reg__file__st.html</anchorfile>
      <anchor>22d8bf4afc93e6fcbf72a6ff44f0bb3f</anchor>
      <arglist></arglist>
    </member>
    <member kind="variable">
      <type>int</type>
      <name>static_regs</name>
      <anchorfile>structhmdes__reg__file__st.html</anchorfile>
      <anchor>de80fa78d15397acbb223216e5e05163</anchor>
      <arglist></arglist>
    </member>
    <member kind="variable">
      <type>int</type>
      <name>rotating_regs</name>
      <anchorfile>structhmdes__reg__file__st.html</anchorfile>
      <anchor>8c76be89048197ce4272dba4a1be1700</anchor>
      <arglist></arglist>
    </member>
    <member kind="variable">
      <type>int</type>
      <name>width</name>
      <anchorfile>structhmdes__reg__file__st.html</anchorfile>
      <anchor>c65674ca370b99fbfd21e32a0f26fa0a</anchor>
      <arglist></arglist>
    </member>
  </compound>
  <compound kind="struct">
    <name>hmdes_res_list_st</name>
    <filename>structhmdes__res__list__st.html</filename>
    <member kind="variable">
      <type>int</type>
      <name>id</name>
      <anchorfile>structhmdes__res__list__st.html</anchorfile>
      <anchor>b15e964410d220e9bf1010c1647383f1</anchor>
      <arglist></arglist>
    </member>
    <member kind="variable">
      <type>char *</type>
      <name>name</name>
      <anchorfile>structhmdes__res__list__st.html</anchorfile>
      <anchor>85c71cd8dd014a46e6201d4b1016655a</anchor>
      <arglist></arglist>
    </member>
    <member kind="variable">
      <type>Hmdes_Res_Node *</type>
      <name>head</name>
      <anchorfile>structhmdes__res__list__st.html</anchorfile>
      <anchor>2d1075e173a779cec43a71c6eae14b24</anchor>
      <arglist></arglist>
    </member>
    <member kind="variable">
      <type>Hmdes_Res_Node *</type>
      <name>tail</name>
      <anchorfile>structhmdes__res__list__st.html</anchorfile>
      <anchor>7c44eb3335628d3d3fbf11a632f11e11</anchor>
      <arglist></arglist>
    </member>
    <member kind="variable">
      <type>Hmdes_Res_Node *</type>
      <name>slot_options</name>
      <anchorfile>structhmdes__res__list__st.html</anchorfile>
      <anchor>5eba31ed560a7c235fa970afa85df1c3</anchor>
      <arglist></arglist>
    </member>
    <member kind="variable">
      <type>int</type>
      <name>size</name>
      <anchorfile>structhmdes__res__list__st.html</anchorfile>
      <anchor>36fe307b3277c48543f27d9816ec89df</anchor>
      <arglist></arglist>
    </member>
  </compound>
  <compound kind="struct">
    <name>hmdes_res_node_st</name>
    <filename>structhmdes__res__node__st.html</filename>
    <member kind="variable">
      <type>int</type>
      <name>flags</name>
      <anchorfile>structhmdes__res__node__st.html</anchorfile>
      <anchor>08cae646467c766b5938f7825b7bb1ba</anchor>
      <arglist></arglist>
    </member>
    <member kind="variable">
      <type>int</type>
      <name>start_usage</name>
      <anchorfile>structhmdes__res__node__st.html</anchorfile>
      <anchor>13ecad11606ad33168a8009d6f8fb7a4</anchor>
      <arglist></arglist>
    </member>
    <member kind="variable">
      <type>int</type>
      <name>end_usage</name>
      <anchorfile>structhmdes__res__node__st.html</anchorfile>
      <anchor>08ce91bb6a2ca3c1c833ccd224d47baf</anchor>
      <arglist></arglist>
    </member>
    <member kind="variable">
      <type>Hmdes_Res_Option *</type>
      <name>head</name>
      <anchorfile>structhmdes__res__node__st.html</anchorfile>
      <anchor>cd490d1bf7dd402b14cbc3b1f5d7282b</anchor>
      <arglist></arglist>
    </member>
    <member kind="variable">
      <type>Hmdes_Res_Option *</type>
      <name>tail</name>
      <anchorfile>structhmdes__res__node__st.html</anchorfile>
      <anchor>80c35ed255c1059aa4c7a9e1a7dbf9ff</anchor>
      <arglist></arglist>
    </member>
    <member kind="variable">
      <type>int</type>
      <name>num_options</name>
      <anchorfile>structhmdes__res__node__st.html</anchorfile>
      <anchor>32ee36c2aaa489909591c3385181b0e9</anchor>
      <arglist></arglist>
    </member>
    <member kind="variable">
      <type>int</type>
      <name>num_slots</name>
      <anchorfile>structhmdes__res__node__st.html</anchorfile>
      <anchor>c2ab5700dfa8ccb53b11c63926e2b037</anchor>
      <arglist></arglist>
    </member>
    <member kind="variable">
      <type>hmdes_res_node_st *</type>
      <name>next</name>
      <anchorfile>structhmdes__res__node__st.html</anchorfile>
      <anchor>b606d85754067aa6ae42cde6ba5c2412</anchor>
      <arglist></arglist>
    </member>
  </compound>
  <compound kind="struct">
    <name>hmdes_res_option_st</name>
    <filename>structhmdes__res__option__st.html</filename>
    <member kind="variable">
      <type>Hmdes_Resource *</type>
      <name>resource</name>
      <anchorfile>structhmdes__res__option__st.html</anchorfile>
      <anchor>07c78c96bd3b51bbfa561fe6e21c0fe0</anchor>
      <arglist></arglist>
    </member>
    <member kind="variable">
      <type>Hmdes_Res_Sub *</type>
      <name>subscript</name>
      <anchorfile>structhmdes__res__option__st.html</anchorfile>
      <anchor>cd91a2d2462669fcd7d087011b0fc60c</anchor>
      <arglist></arglist>
    </member>
    <member kind="variable">
      <type>hmdes_res_option_st *</type>
      <name>next</name>
      <anchorfile>structhmdes__res__option__st.html</anchorfile>
      <anchor>3d45d17fa906af82ae8f4f68f3de25b1</anchor>
      <arglist></arglist>
    </member>
  </compound>
  <compound kind="struct">
    <name>hmdes_res_sub_st</name>
    <filename>structhmdes__res__sub__st.html</filename>
    <member kind="variable">
      <type>hmdes_resource_st *</type>
      <name>resource</name>
      <anchorfile>structhmdes__res__sub__st.html</anchorfile>
      <anchor>aec375db03cc736acf62edf2c9a223e0</anchor>
      <arglist></arglist>
    </member>
    <member kind="variable">
      <type>int</type>
      <name>id</name>
      <anchorfile>structhmdes__res__sub__st.html</anchorfile>
      <anchor>df002bb8dd1ed980b0c5085ed48ae395</anchor>
      <arglist></arglist>
    </member>
    <member kind="variable">
      <type>int</type>
      <name>subscript</name>
      <anchorfile>structhmdes__res__sub__st.html</anchorfile>
      <anchor>2ffdefd39ebe287898f0f8a653387b36</anchor>
      <arglist></arglist>
    </member>
    <member kind="variable">
      <type>hmdes_res_sub_st *</type>
      <name>next</name>
      <anchorfile>structhmdes__res__sub__st.html</anchorfile>
      <anchor>c5c4aed47688209d449559dd50b45976</anchor>
      <arglist></arglist>
    </member>
  </compound>
  <compound kind="struct">
    <name>hmdes_resource_st</name>
    <filename>structhmdes__resource__st.html</filename>
    <member kind="variable">
      <type>char *</type>
      <name>name</name>
      <anchorfile>structhmdes__resource__st.html</anchorfile>
      <anchor>bb3acea2756d84fed3b9482e20a9746c</anchor>
      <arglist></arglist>
    </member>
    <member kind="variable">
      <type>Hmdes_Res_Sub *</type>
      <name>head</name>
      <anchorfile>structhmdes__resource__st.html</anchorfile>
      <anchor>bfaf104c2a6fc140cc5e496eb9ba8512</anchor>
      <arglist></arglist>
    </member>
    <member kind="variable">
      <type>Hmdes_Res_Sub *</type>
      <name>tail</name>
      <anchorfile>structhmdes__resource__st.html</anchorfile>
      <anchor>e13670032d513a830e67f6d307d1c106</anchor>
      <arglist></arglist>
    </member>
    <member kind="variable">
      <type>int</type>
      <name>num_subscripts</name>
      <anchorfile>structhmdes__resource__st.html</anchorfile>
      <anchor>a468252cf50f30516d4bc9559b7104b5</anchor>
      <arglist></arglist>
    </member>
  </compound>
  <compound kind="struct">
    <name>hmdes_st</name>
    <filename>structhmdes__st.html</filename>
    <member kind="variable">
      <type>char *</type>
      <name>file_name</name>
      <anchorfile>structhmdes__st.html</anchorfile>
      <anchor>fd1dc8fab430ab25be10960c41fe6c13</anchor>
      <arglist></arglist>
    </member>
    <member kind="variable">
      <type>int</type>
      <name>processor_model</name>
      <anchorfile>structhmdes__st.html</anchorfile>
      <anchor>90cd9923c96e9175630409e9fd7e9ff4</anchor>
      <arglist></arglist>
    </member>
    <member kind="variable">
      <type>int</type>
      <name>max_src_operands</name>
      <anchorfile>structhmdes__st.html</anchorfile>
      <anchor>d13b429ef4ce57f85b6e50cfc7e89ba4</anchor>
      <arglist></arglist>
    </member>
    <member kind="variable">
      <type>int</type>
      <name>max_src_syncs</name>
      <anchorfile>structhmdes__st.html</anchorfile>
      <anchor>cdee5e7de7c4d871114a3983232d9766</anchor>
      <arglist></arglist>
    </member>
    <member kind="variable">
      <type>int</type>
      <name>max_dest_operands</name>
      <anchorfile>structhmdes__st.html</anchorfile>
      <anchor>6227b0dc332b781877c35fdbf23d3772</anchor>
      <arglist></arglist>
    </member>
    <member kind="variable">
      <type>int</type>
      <name>max_dest_syncs</name>
      <anchorfile>structhmdes__st.html</anchorfile>
      <anchor>fd6e7b12c8fd77bc052c69715ba2e9c2</anchor>
      <arglist></arglist>
    </member>
    <member kind="variable">
      <type>int</type>
      <name>max_pred_operands</name>
      <anchorfile>structhmdes__st.html</anchorfile>
      <anchor>d1234881901cd6f69caf80f4238f874f</anchor>
      <arglist></arglist>
    </member>
    <member kind="variable">
      <type>int</type>
      <name>max_slot</name>
      <anchorfile>structhmdes__st.html</anchorfile>
      <anchor>edec89f9ab36ec4d87f60111d14cb645</anchor>
      <arglist></arglist>
    </member>
    <member kind="variable">
      <type>Msymbol_Table *</type>
      <name>defines_command</name>
      <anchorfile>structhmdes__st.html</anchorfile>
      <anchor>f97889ead3712c69d16c0b3065751a22</anchor>
      <arglist></arglist>
    </member>
    <member kind="variable">
      <type>Msymbol_Table *</type>
      <name>defines_environ</name>
      <anchorfile>structhmdes__st.html</anchorfile>
      <anchor>e49c39adcba261565e40c60a1ce6b4ee</anchor>
      <arglist></arglist>
    </member>
    <member kind="variable">
      <type>Msymbol_Table *</type>
      <name>defines_internal</name>
      <anchorfile>structhmdes__st.html</anchorfile>
      <anchor>dbef010eddb133b0624cbd7bb2028ae5</anchor>
      <arglist></arglist>
    </member>
    <member kind="variable">
      <type>Msymbol_Table *</type>
      <name>reg_file</name>
      <anchorfile>structhmdes__st.html</anchorfile>
      <anchor>13fefe0fdd041c49948508cd1931528a</anchor>
      <arglist></arglist>
    </member>
    <member kind="variable">
      <type>Msymbol_Table *</type>
      <name>IO_sets</name>
      <anchorfile>structhmdes__st.html</anchorfile>
      <anchor>1a722007506b5a11845e3eb11113c90b</anchor>
      <arglist></arglist>
    </member>
    <member kind="variable">
      <type>Msymbol_Table *</type>
      <name>IO_items</name>
      <anchorfile>structhmdes__st.html</anchorfile>
      <anchor>54c0db7ebd19e2926d32d6e8fc5689f2</anchor>
      <arglist></arglist>
    </member>
    <member kind="variable">
      <type>int</type>
      <name>num_resources</name>
      <anchorfile>structhmdes__st.html</anchorfile>
      <anchor>fa908f78fee9f43db0bb28a4b186b4c8</anchor>
      <arglist></arglist>
    </member>
    <member kind="variable">
      <type>Msymbol_Table *</type>
      <name>resources</name>
      <anchorfile>structhmdes__st.html</anchorfile>
      <anchor>194d8ae8e167cd72a67e1e02cc4ed2bd</anchor>
      <arglist></arglist>
    </member>
    <member kind="variable">
      <type>Msymbol_Table *</type>
      <name>res_lists</name>
      <anchorfile>structhmdes__st.html</anchorfile>
      <anchor>9ae40d45f69564985642878dd202a76b</anchor>
      <arglist></arglist>
    </member>
    <member kind="variable">
      <type>Msymbol_Table *</type>
      <name>latencies</name>
      <anchorfile>structhmdes__st.html</anchorfile>
      <anchor>7edd02826d139fa007a4d90348e81e7c</anchor>
      <arglist></arglist>
    </member>
    <member kind="variable">
      <type>Msymbol_Table *</type>
      <name>op_class</name>
      <anchorfile>structhmdes__st.html</anchorfile>
      <anchor>091b0d028acff626415238cf2ba93cf9</anchor>
      <arglist></arglist>
    </member>
    <member kind="variable">
      <type>Msymbol_Table *</type>
      <name>operations</name>
      <anchorfile>structhmdes__st.html</anchorfile>
      <anchor>7244cda8df1b26517a5e5371207355fc</anchor>
      <arglist></arglist>
    </member>
    <member kind="variable">
      <type>Hmdes_IO_Set *</type>
      <name>null_set</name>
      <anchorfile>structhmdes__st.html</anchorfile>
      <anchor>253c976a4e6426574a889e72c77f73fd</anchor>
      <arglist></arglist>
    </member>
  </compound>
  <compound kind="struct">
    <name>hparse_info_st</name>
    <filename>structhparse__info__st.html</filename>
    <member kind="variable">
      <type>Msymbol_Table *</type>
      <name>table</name>
      <anchorfile>structhparse__info__st.html</anchorfile>
      <anchor>d5cd7dcec657337c479077eb94b9af16</anchor>
      <arglist></arglist>
    </member>
    <member kind="variable">
      <type>char *</type>
      <name>name</name>
      <anchorfile>structhparse__info__st.html</anchorfile>
      <anchor>e6f44529056b337ea7faef2fd8380191</anchor>
      <arglist></arglist>
    </member>
    <member kind="variable">
      <type>FILE *</type>
      <name>in</name>
      <anchorfile>structhparse__info__st.html</anchorfile>
      <anchor>4f67d1e4720eb89684dab7b7d847ea9c</anchor>
      <arglist></arglist>
    </member>
    <member kind="variable">
      <type>int</type>
      <name>line</name>
      <anchorfile>structhparse__info__st.html</anchorfile>
      <anchor>fc6e23cc1b24fd8660de0f5a76f94c03</anchor>
      <arglist></arglist>
    </member>
    <member kind="variable">
      <type>int</type>
      <name>comment_level</name>
      <anchorfile>structhparse__info__st.html</anchorfile>
      <anchor>974f8dc473d77e6e5fd98d32004e1339</anchor>
      <arglist></arglist>
    </member>
    <member kind="variable">
      <type>int</type>
      <name>endif_level</name>
      <anchorfile>structhparse__info__st.html</anchorfile>
      <anchor>d88de4942e534b3a566cfef184eb944d</anchor>
      <arglist></arglist>
    </member>
  </compound>
  <compound kind="struct">
    <name>mparse_info_st</name>
    <filename>structmparse__info__st.html</filename>
    <member kind="variable">
      <type>char *</type>
      <name>file_name</name>
      <anchorfile>structmparse__info__st.html</anchorfile>
      <anchor>f73004c9cea9694234312a562f0d2c72</anchor>
      <arglist></arglist>
    </member>
    <member kind="variable">
      <type>char</type>
      <name>section_name</name>
      <anchorfile>structmparse__info__st.html</anchorfile>
      <anchor>f3c71fe570c19c3ada807a1f9912ca13</anchor>
      <arglist>[100]</arglist>
    </member>
    <member kind="variable">
      <type>FILE *</type>
      <name>in</name>
      <anchorfile>structmparse__info__st.html</anchorfile>
      <anchor>d9ad6c79267b4cc06380ad963827e83d</anchor>
      <arglist></arglist>
    </member>
    <member kind="variable">
      <type>int</type>
      <name>line</name>
      <anchorfile>structmparse__info__st.html</anchorfile>
      <anchor>4c9a3d5a9fb0463083f6bbbfa407ece1</anchor>
      <arglist></arglist>
    </member>
    <member kind="variable">
      <type>Mparse_Item *</type>
      <name>head</name>
      <anchorfile>structmparse__info__st.html</anchorfile>
      <anchor>d63ca9bbfb2ac2a92c75c1fefd130978</anchor>
      <arglist></arglist>
    </member>
    <member kind="variable">
      <type>Mparse_Item *</type>
      <name>tail</name>
      <anchorfile>structmparse__info__st.html</anchorfile>
      <anchor>fbaa581f1d4cdf3059ef5fb598bfa4f3</anchor>
      <arglist></arglist>
    </member>
    <member kind="variable">
      <type>int</type>
      <name>queue_size</name>
      <anchorfile>structmparse__info__st.html</anchorfile>
      <anchor>d256edcdec28f595d57173c973915364</anchor>
      <arglist></arglist>
    </member>
    <member kind="variable">
      <type>Hmdes *</type>
      <name>hmdes</name>
      <anchorfile>structmparse__info__st.html</anchorfile>
      <anchor>f95b69837519d3817bda320a0ad75e2c</anchor>
      <arglist></arglist>
    </member>
  </compound>
  <compound kind="struct">
    <name>mparse_item_st</name>
    <filename>structmparse__item__st.html</filename>
    <member kind="variable">
      <type>char</type>
      <name>buf</name>
      <anchorfile>structmparse__item__st.html</anchorfile>
      <anchor>a1c0c50833b59cce39c2993f70bbd4da</anchor>
      <arglist>[MITEM_SIZE]</arglist>
    </member>
    <member kind="variable">
      <type>mparse_item_st *</type>
      <name>next</name>
      <anchorfile>structmparse__item__st.html</anchorfile>
      <anchor>fa0ef512228ae0a35b8b6cebfd0c7153</anchor>
      <arglist></arglist>
    </member>
  </compound>
  <compound kind="struct">
    <name>msymbol_st</name>
    <filename>structmsymbol__st.html</filename>
    <member kind="variable">
      <type>char *</type>
      <name>name</name>
      <anchorfile>structmsymbol__st.html</anchorfile>
      <anchor>d64f14bf72b7b9b601133c68d6b2dd83</anchor>
      <arglist></arglist>
    </member>
    <member kind="variable">
      <type>void *</type>
      <name>ptr</name>
      <anchorfile>structmsymbol__st.html</anchorfile>
      <anchor>9cf9b630400e233fe0d3e88fd9890a70</anchor>
      <arglist></arglist>
    </member>
    <member kind="variable">
      <type>msymbol_st *</type>
      <name>next_hash</name>
      <anchorfile>structmsymbol__st.html</anchorfile>
      <anchor>38faf14b10f3deb00c1c33852b3e55e8</anchor>
      <arglist></arglist>
    </member>
    <member kind="variable">
      <type>msymbol_st *</type>
      <name>next_linear</name>
      <anchorfile>structmsymbol__st.html</anchorfile>
      <anchor>3ae57a496e08aa6b3cfdaac96db12241</anchor>
      <arglist></arglist>
    </member>
    <member kind="variable">
      <type>msymbol_st *</type>
      <name>prev_linear</name>
      <anchorfile>structmsymbol__st.html</anchorfile>
      <anchor>b4c16591e4c880a87d53dfc4ff5202f5</anchor>
      <arglist></arglist>
    </member>
  </compound>
  <compound kind="struct">
    <name>msymbol_table_st</name>
    <filename>structmsymbol__table__st.html</filename>
    <member kind="variable">
      <type>char *</type>
      <name>name</name>
      <anchorfile>structmsymbol__table__st.html</anchorfile>
      <anchor>f56efe18ecd3a218034e0c73c9cb3375</anchor>
      <arglist></arglist>
    </member>
    <member kind="variable">
      <type>int</type>
      <name>hash_mask</name>
      <anchorfile>structmsymbol__table__st.html</anchorfile>
      <anchor>832d3b8f64ae18cf30abc10107b2e5a3</anchor>
      <arglist></arglist>
    </member>
    <member kind="variable">
      <type>Msymbol **</type>
      <name>hash_table</name>
      <anchorfile>structmsymbol__table__st.html</anchorfile>
      <anchor>5d5af8510a253417360b60a16c8543c4</anchor>
      <arglist></arglist>
    </member>
    <member kind="variable">
      <type>Msymbol *</type>
      <name>head</name>
      <anchorfile>structmsymbol__table__st.html</anchorfile>
      <anchor>de6159e81bced21f9c5fb2149459aac3</anchor>
      <arglist></arglist>
    </member>
    <member kind="variable">
      <type>Msymbol *</type>
      <name>tail</name>
      <anchorfile>structmsymbol__table__st.html</anchorfile>
      <anchor>7a405eb2135ac076c2f2ba3d565b9a17</anchor>
      <arglist></arglist>
    </member>
    <member kind="variable">
      <type>int</type>
      <name>entry_count</name>
      <anchorfile>structmsymbol__table__st.html</anchorfile>
      <anchor>bac5d80d09b6598b64be0b57276b491a</anchor>
      <arglist></arglist>
    </member>
  </compound>
</tagfile>
