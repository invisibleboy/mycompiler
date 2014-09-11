<?xml version='1.0' encoding='ISO-8859-1' standalone='yes' ?>
<tagfile>
  <compound kind="file">
    <name>header_reader2.c</name>
    <path>/mnt/hgfs/mkt/Desktop/openimpact/src/machine/lmdes2_customizer/</path>
    <filename>header__reader2_8c</filename>
    <class kind="struct">hparse_info_st</class>
    <member kind="define">
      <type>#define</type>
      <name>MLINE_SIZE</name>
      <anchorfile>header__reader2_8c.html</anchorfile>
      <anchor>4253cac11fcc1fba152b18205cc28a91</anchor>
      <arglist></arglist>
    </member>
    <member kind="typedef">
      <type>hparse_info_st</type>
      <name>Hparse_Info</name>
      <anchorfile>header__reader2_8c.html</anchorfile>
      <anchor>1c7a4795de9ad80203df10f8f5b59d98</anchor>
      <arglist></arglist>
    </member>
    <member kind="function">
      <type>void</type>
      <name>Hskip_to_endif</name>
      <anchorfile>header__reader2_8c.html</anchorfile>
      <anchor>8fc585b250c1ca25bc77a4960efd41f7</anchor>
      <arglist>(Hparse_Info *hinfo, char *source, int line)</arglist>
    </member>
    <member kind="function">
      <type>int</type>
      <name>Hget_next_line</name>
      <anchorfile>header__reader2_8c.html</anchorfile>
      <anchor>a8f0c95c6f82b49624faba4e34616dd6</anchor>
      <arglist>(Hparse_Info *hinfo, char *ret_buf)</arglist>
    </member>
    <member kind="function">
      <type>void</type>
      <name>Hparse_line</name>
      <anchorfile>header__reader2_8c.html</anchorfile>
      <anchor>672ef2e32077cd2ca8eff3cb18b4166d</anchor>
      <arglist>(char *ptr, char *name, char *arg)</arglist>
    </member>
    <member kind="function">
      <type>STRING_Symbol_Table *</type>
      <name>read_header_file</name>
      <anchorfile>header__reader2_8c.html</anchorfile>
      <anchor>33574aec74728b493ce42679b1c18d10</anchor>
      <arglist>(char *file_name, int print_warnings)</arglist>
    </member>
  </compound>
  <compound kind="file">
    <name>lmdes2_customizer.c</name>
    <path>/mnt/hgfs/mkt/Desktop/openimpact/src/machine/lmdes2_customizer/</path>
    <filename>lmdes2__customizer_8c</filename>
    <class kind="struct">Pointer_Mapping</class>
    <class kind="struct">Resource_Info</class>
    <class kind="struct">Pair_Node</class>
    <class kind="struct">ENTRY_Contents_Table</class>
    <class kind="struct">ENTRY_Contents_Node</class>
    <member kind="define">
      <type>#define</type>
      <name>SLOT_RESOURCE</name>
      <anchorfile>lmdes2__customizer_8c.html</anchorfile>
      <anchor>d2fffad840fe164eb77141c323a1bc9b</anchor>
      <arglist></arglist>
    </member>
    <member kind="define">
      <type>#define</type>
      <name>DEBUG_OPTI</name>
      <anchorfile>lmdes2__customizer_8c.html</anchorfile>
      <anchor>6ae2868cfb34bf62378235a206c9800c</anchor>
      <arglist></arglist>
    </member>
    <member kind="define">
      <type>#define</type>
      <name>VERBOSE_OPTI</name>
      <anchorfile>lmdes2__customizer_8c.html</anchorfile>
      <anchor>c6d967d290f940fb8143bf141f311a2a</anchor>
      <arglist></arglist>
    </member>
    <member kind="define">
      <type>#define</type>
      <name>VERBOSE_STATS</name>
      <anchorfile>lmdes2__customizer_8c.html</anchorfile>
      <anchor>a0fa4d0652fbff641ed614b478884266</anchor>
      <arglist></arglist>
    </member>
    <member kind="define">
      <type>#define</type>
      <name>MAX_IDENT_LEN</name>
      <anchorfile>lmdes2__customizer_8c.html</anchorfile>
      <anchor>9cf0008f332fed26b2b4149b66205559</anchor>
      <arglist></arglist>
    </member>
    <member kind="function">
      <type>STRING_Symbol_Table *</type>
      <name>read_header_file</name>
      <anchorfile>lmdes2__customizer_8c.html</anchorfile>
      <anchor>33574aec74728b493ce42679b1c18d10</anchor>
      <arglist>(char *file_name, int print_warnings)</arglist>
    </member>
    <member kind="function">
      <type>void</type>
      <name>customize_md</name>
      <anchorfile>lmdes2__customizer_8c.html</anchorfile>
      <anchor>325650c03d399b4e850fe3a9bda3df17</anchor>
      <arglist>(MD *md)</arglist>
    </member>
    <member kind="function">
      <type>int</type>
      <name>eliminate_redundant_new_entries_only</name>
      <anchorfile>lmdes2__customizer_8c.html</anchorfile>
      <anchor>4286ee81d149a7bd3dda9056304a94bd</anchor>
      <arglist>(MD *md, char *section_name)</arglist>
    </member>
    <member kind="function">
      <type>int</type>
      <name>CU_redundant_entries</name>
      <anchorfile>lmdes2__customizer_8c.html</anchorfile>
      <anchor>33651693828cf0dceddf0652bb22eac0</anchor>
      <arglist>(MD_Entry *entry1, MD_Entry *entry2, char *ignore_field)</arglist>
    </member>
    <member kind="function">
      <type>void</type>
      <name>print_generating_set</name>
      <anchorfile>lmdes2__customizer_8c.html</anchorfile>
      <anchor>ebeba51fe729c503fd5b0918d2631671</anchor>
      <arglist>(FILE *out, INT_Symbol_Table *generating_set, MD_Entry **unit_array, int committed)</arglist>
    </member>
    <member kind="function">
      <type>void</type>
      <name>commit_usage</name>
      <anchorfile>lmdes2__customizer_8c.html</anchorfile>
      <anchor>7bbd7a4ef676ca025d871cea39549a0d</anchor>
      <arglist>(INT_Symbol_Table ***forbidden_matrix, Resource_Info *resource, int user, int time)</arglist>
    </member>
    <member kind="function">
      <type>void</type>
      <name>print_table_trees</name>
      <anchorfile>lmdes2__customizer_8c.html</anchorfile>
      <anchor>a2b1793d18fb89fc6b66c789c85cc39a</anchor>
      <arglist>(FILE *out, MD *md)</arglist>
    </member>
    <member kind="function" static="yes">
      <type>static void</type>
      <name>C_punt</name>
      <anchorfile>lmdes2__customizer_8c.html</anchorfile>
      <anchor>6efdeefd9e046fe3a6d8369c3a48758b</anchor>
      <arglist>(MD *md, char *fmt,...)</arglist>
    </member>
    <member kind="function">
      <type>void</type>
      <name>print_time_stamp</name>
      <anchorfile>lmdes2__customizer_8c.html</anchorfile>
      <anchor>0544d5712b0f4afe9036291b18aab413</anchor>
      <arglist>(FILE *out)</arglist>
    </member>
    <member kind="function">
      <type>void</type>
      <name>print_usage</name>
      <anchorfile>lmdes2__customizer_8c.html</anchorfile>
      <anchor>62ac7a1cfa066f61a3317977c936b8f2</anchor>
      <arglist>(char *fmt,...)</arglist>
    </member>
    <member kind="function">
      <type>void</type>
      <name>read_command_line_parameters</name>
      <anchorfile>lmdes2__customizer_8c.html</anchorfile>
      <anchor>b5ad1e75f3e95a8372855846f911efe1</anchor>
      <arglist>(int argc, char **arg_ptr)</arglist>
    </member>
    <member kind="function">
      <type>int</type>
      <name>main</name>
      <anchorfile>lmdes2__customizer_8c.html</anchorfile>
      <anchor>3c04138a5bfe5d72780bb7e82a18e627</anchor>
      <arglist>(int argc, char **argv)</arglist>
    </member>
    <member kind="function">
      <type>void</type>
      <name>read_header_tables</name>
      <anchorfile>lmdes2__customizer_8c.html</anchorfile>
      <anchor>335c1116a90b866de4a2951d0bb9eca0</anchor>
      <arglist>(MD *md)</arglist>
    </member>
    <member kind="function">
      <type>int</type>
      <name>find_header_value</name>
      <anchorfile>lmdes2__customizer_8c.html</anchorfile>
      <anchor>3104e311f120343924d365c7d4a9f816</anchor>
      <arglist>(MD *md, char *name, int *value)</arglist>
    </member>
    <member kind="function">
      <type>void</type>
      <name>annotate_header_values</name>
      <anchorfile>lmdes2__customizer_8c.html</anchorfile>
      <anchor>66d922d9b46761dde0bd9e8d03168a2a</anchor>
      <arglist>(MD *md, char *section_name, char *field_name, char *prefix, int required)</arglist>
    </member>
    <member kind="function">
      <type>void</type>
      <name>annotate_hash</name>
      <anchorfile>lmdes2__customizer_8c.html</anchorfile>
      <anchor>9269b069890aaa0c3eadf5b9e64e2a72</anchor>
      <arglist>(MD *md, char *section_name)</arglist>
    </member>
    <member kind="function">
      <type>void</type>
      <name>annotate_latency_class</name>
      <anchorfile>lmdes2__customizer_8c.html</anchorfile>
      <anchor>4f7bcf59270a7e4092afdc10b639fc36</anchor>
      <arglist>(MD *md, char *section_name, char *src_field_name, char *dest_field_name)</arglist>
    </member>
    <member kind="function">
      <type>void</type>
      <name>convert_to_bit_representation</name>
      <anchorfile>lmdes2__customizer_8c.html</anchorfile>
      <anchor>8d183a09983eb17f4c21eba43c6ac1cc</anchor>
      <arglist>(MD *md, char *src_section_name, char *src_field_name, char *value_section_name, char *value_field_name, char *dest_field_name)</arglist>
    </member>
    <member kind="function">
      <type>void</type>
      <name>find_children</name>
      <anchorfile>lmdes2__customizer_8c.html</anchorfile>
      <anchor>f1a4477e115e0eba3990744a657173c2</anchor>
      <arglist>(INT_Symbol_Table *child_table, MD_Entry *entry, MD_Field_Decl *field_decl)</arglist>
    </member>
    <member kind="function">
      <type>void</type>
      <name>add_all_children_to_list</name>
      <anchorfile>lmdes2__customizer_8c.html</anchorfile>
      <anchor>63b739f1e28f24f379411fb7743e6b44</anchor>
      <arglist>(MD *md, char *section_name, char *field_name)</arglist>
    </member>
    <member kind="function">
      <type>void</type>
      <name>convert_to_int_list</name>
      <anchorfile>lmdes2__customizer_8c.html</anchorfile>
      <anchor>21573698051dc7050183b82a6f62e003</anchor>
      <arglist>(MD *md, char *src_section_name, char *src_field_name, char *value_section_name, char *value_field_name, char *dest_field_name)</arglist>
    </member>
    <member kind="function">
      <type>void</type>
      <name>convert_to_int_list_all_targets</name>
      <anchorfile>lmdes2__customizer_8c.html</anchorfile>
      <anchor>634270b643e7deeab2184f71ca619024</anchor>
      <arglist>(MD *md, char *src_section_name, char *src_field_name, char *value_field_name, char *dest_field_name)</arglist>
    </member>
    <member kind="function">
      <type>void</type>
      <name>check_int_bounds</name>
      <anchorfile>lmdes2__customizer_8c.html</anchorfile>
      <anchor>41d1d44089b9ab6bbc29aa96db125b25</anchor>
      <arglist>(MD *md, char *section_name, char *field_name, int lower_bound, int upper_bound)</arglist>
    </member>
    <member kind="function">
      <type>void</type>
      <name>check_proc_opcs</name>
      <anchorfile>lmdes2__customizer_8c.html</anchorfile>
      <anchor>9bfc7ce417705fe84aae84f4d1403015</anchor>
      <arglist>(MD *md)</arglist>
    </member>
    <member kind="function">
      <type>void</type>
      <name>check_table_option_count</name>
      <anchorfile>lmdes2__customizer_8c.html</anchorfile>
      <anchor>d7581428a805b5351485f9d09da4aad0</anchor>
      <arglist>(MD *md)</arglist>
    </member>
    <member kind="function">
      <type>void</type>
      <name>check_bidirectional_links</name>
      <anchorfile>lmdes2__customizer_8c.html</anchorfile>
      <anchor>6740c00001d924e1656171ce296a54d0</anchor>
      <arglist>(MD *md, char *section_name, char *field_name)</arglist>
    </member>
    <member kind="function">
      <type>MD_Section *</type>
      <name>CU_find_section</name>
      <anchorfile>lmdes2__customizer_8c.html</anchorfile>
      <anchor>5b29c261eac5c2802f9c5ef741c7ef12</anchor>
      <arglist>(MD *md, char *section_name)</arglist>
    </member>
    <member kind="function">
      <type>MD_Field_Decl *</type>
      <name>CU_find_field_decl</name>
      <anchorfile>lmdes2__customizer_8c.html</anchorfile>
      <anchor>b75391e91939a490c363358e9c12f40c</anchor>
      <arglist>(MD_Section *section, char *field_name)</arglist>
    </member>
    <member kind="function">
      <type>MD_Field *</type>
      <name>CU_find_field</name>
      <anchorfile>lmdes2__customizer_8c.html</anchorfile>
      <anchor>dfba5ab2edf1d22c8eef8e33bfbb4e8f</anchor>
      <arglist>(MD_Entry *entry, MD_Field_Decl *field_decl)</arglist>
    </member>
    <member kind="function">
      <type>MD_Field *</type>
      <name>CU_find_field_by_name</name>
      <anchorfile>lmdes2__customizer_8c.html</anchorfile>
      <anchor>92da4a5722a22962930cd80ebcbfe551</anchor>
      <arglist>(MD_Entry *entry, char *field_name)</arglist>
    </member>
    <member kind="function">
      <type>void</type>
      <name>CU_get_unique_name</name>
      <anchorfile>lmdes2__customizer_8c.html</anchorfile>
      <anchor>0d6d799591481b18affc74f76c135c41</anchor>
      <arglist>(MD *md, MD_Entry *entry, char *base_name, char *new_name)</arglist>
    </member>
    <member kind="function">
      <type>MD_Entry *</type>
      <name>CU_dup_entry</name>
      <anchorfile>lmdes2__customizer_8c.html</anchorfile>
      <anchor>0f5365fc4e33bb5f9ed676c4fbba9ea1</anchor>
      <arglist>(MD_Entry *orig_entry)</arglist>
    </member>
    <member kind="function">
      <type>void</type>
      <name>CU_rename_entry</name>
      <anchorfile>lmdes2__customizer_8c.html</anchorfile>
      <anchor>1a485003e9cc947cf64e3769e68af28e</anchor>
      <arglist>(MD_Entry *entry, char *new_name)</arglist>
    </member>
    <member kind="function">
      <type>void</type>
      <name>CU_add_link_at</name>
      <anchorfile>lmdes2__customizer_8c.html</anchorfile>
      <anchor>6654d07ff1321b2e08dfd348ff1e3e2b</anchor>
      <arglist>(MD_Field *field, int at_index, MD_Entry *new_link)</arglist>
    </member>
    <member kind="function">
      <type>void</type>
      <name>CU_delete_link_at</name>
      <anchorfile>lmdes2__customizer_8c.html</anchorfile>
      <anchor>92c91ed1ca364644c08508886798782e</anchor>
      <arglist>(MD_Field *field, int at_index)</arglist>
    </member>
    <member kind="function">
      <type>MD_Entry *</type>
      <name>CU_dup_alt</name>
      <anchorfile>lmdes2__customizer_8c.html</anchorfile>
      <anchor>21fc335b4dfe1e7941cdc26722a47457</anchor>
      <arglist>(MD_Entry *orig_alt)</arglist>
    </member>
    <member kind="function">
      <type>MD_Entry *</type>
      <name>CU_dup_res_table</name>
      <anchorfile>lmdes2__customizer_8c.html</anchorfile>
      <anchor>ba4d5d0af6041b1626b8cd7186b2e73f</anchor>
      <arglist>(MD_Entry *orig_res_table)</arglist>
    </member>
    <member kind="function">
      <type>MD_Entry *</type>
      <name>CU_dup_resource_usage</name>
      <anchorfile>lmdes2__customizer_8c.html</anchorfile>
      <anchor>5908c785db2a738c076ba133610e281f</anchor>
      <arglist>(MD_Entry *orig_ru)</arglist>
    </member>
    <member kind="function">
      <type>int</type>
      <name>CU_get_slot</name>
      <anchorfile>lmdes2__customizer_8c.html</anchorfile>
      <anchor>68cc9dad453a2e899020a7ee00ae52bb</anchor>
      <arglist>(MD_Entry *entry)</arglist>
    </member>
    <member kind="function">
      <type>int</type>
      <name>CU_get_min_time</name>
      <anchorfile>lmdes2__customizer_8c.html</anchorfile>
      <anchor>04c8d543fffa447091d73d185e5e6765</anchor>
      <arglist>(MD_Entry *entry)</arglist>
    </member>
    <member kind="function">
      <type>int</type>
      <name>CU_overlapping_resource_usage</name>
      <anchorfile>lmdes2__customizer_8c.html</anchorfile>
      <anchor>409c425142d1a29b7cd608ce586bb35a</anchor>
      <arglist>(MD_Entry *entry1, MD_Entry *entry2)</arglist>
    </member>
    <member kind="function">
      <type>int</type>
      <name>CU_make_res_table_orthogonal</name>
      <anchorfile>lmdes2__customizer_8c.html</anchorfile>
      <anchor>5b210418b87ea11a38a9685106fcadb0</anchor>
      <arglist>(MD_Entry *res_table)</arglist>
    </member>
    <member kind="function">
      <type>void</type>
      <name>CU_consolidate_res_table_usages</name>
      <anchorfile>lmdes2__customizer_8c.html</anchorfile>
      <anchor>0ef75bfb7f290b6513271853e6fd74fc</anchor>
      <arglist>(MD_Entry *res_table)</arglist>
    </member>
    <member kind="function">
      <type>int</type>
      <name>usage_cycle_compatable</name>
      <anchorfile>lmdes2__customizer_8c.html</anchorfile>
      <anchor>aa03acc0339bfa5eb1dfac59e896fe1a</anchor>
      <arglist>(MD_Entry *usage_entry, MD_Entry *option_entry)</arglist>
    </member>
    <member kind="function">
      <type>int</type>
      <name>usage_resource_compatable</name>
      <anchorfile>lmdes2__customizer_8c.html</anchorfile>
      <anchor>bc7d12e76acc86904bdebd5296230f21</anchor>
      <arglist>(MD_Entry *usage_entry, MD_Entry *option_entry)</arglist>
    </member>
    <member kind="function">
      <type>void</type>
      <name>distribute_usage</name>
      <anchorfile>lmdes2__customizer_8c.html</anchorfile>
      <anchor>ec658bc2635a4784ff353fc3cad8bb0b</anchor>
      <arglist>(MD_Field *table_use_field, int index, MD_Entry *usage_entry)</arglist>
    </member>
    <member kind="function">
      <type>void</type>
      <name>CU_distribute_unconditional_usages</name>
      <anchorfile>lmdes2__customizer_8c.html</anchorfile>
      <anchor>edcc55f67d62d16fa42c8dc98dec4a43</anchor>
      <arglist>(MD_Entry *table_entry)</arglist>
    </member>
    <member kind="function">
      <type>void</type>
      <name>CU_homogenize_table_options</name>
      <anchorfile>lmdes2__customizer_8c.html</anchorfile>
      <anchor>722d64044c191dff9b39056217e3cb8a</anchor>
      <arglist>(MD_Entry *table_option)</arglist>
    </member>
    <member kind="function">
      <type>void</type>
      <name>CU_homogenize_resource_usages</name>
      <anchorfile>lmdes2__customizer_8c.html</anchorfile>
      <anchor>3332767efcdc1ab552393d010b97bf22</anchor>
      <arglist>(MD_Entry *ru_entry)</arglist>
    </member>
    <member kind="function">
      <type>void</type>
      <name>CU_optimize_resource_usage_times</name>
      <anchorfile>lmdes2__customizer_8c.html</anchorfile>
      <anchor>772a2fd220caf518585c5ed7bfe622be</anchor>
      <arglist>(MD *md)</arglist>
    </member>
    <member kind="function">
      <type>int</type>
      <name>CU_get_ru_time</name>
      <anchorfile>lmdes2__customizer_8c.html</anchorfile>
      <anchor>9c7268b691bd7ad874ab904435b25527</anchor>
      <arglist>(MD_Entry *ru_entry)</arglist>
    </member>
    <member kind="function">
      <type>int</type>
      <name>CU_num_table_uses</name>
      <anchorfile>lmdes2__customizer_8c.html</anchorfile>
      <anchor>fdf290d1fef84d92c3a52bb51e638226</anchor>
      <arglist>(MD_Entry *option_entry)</arglist>
    </member>
    <member kind="function">
      <type>void</type>
      <name>CU_order_use_fields</name>
      <anchorfile>lmdes2__customizer_8c.html</anchorfile>
      <anchor>09e11110f5b6a98ed4d840057dc1e125</anchor>
      <arglist>(MD *md, int min_reorder)</arglist>
    </member>
    <member kind="function">
      <type>void</type>
      <name>homogenize_reservation_tables</name>
      <anchorfile>lmdes2__customizer_8c.html</anchorfile>
      <anchor>3f2c2ef62b99bf93eba2656e2f742c67</anchor>
      <arglist>(MD *md)</arglist>
    </member>
    <member kind="function">
      <type>void</type>
      <name>check_for_unconditional_overlap</name>
      <anchorfile>lmdes2__customizer_8c.html</anchorfile>
      <anchor>2fcefe048bbfcd44785ed45ad58bde9d</anchor>
      <arglist>(MD *md)</arglist>
    </member>
    <member kind="function">
      <type>int</type>
      <name>assign_resource_map_locations</name>
      <anchorfile>lmdes2__customizer_8c.html</anchorfile>
      <anchor>80f65397ab1027641ff4bae7d6b48595</anchor>
      <arglist>(MD *md)</arglist>
    </member>
    <member kind="function">
      <type>int</type>
      <name>usage_common_to_all_options</name>
      <anchorfile>lmdes2__customizer_8c.html</anchorfile>
      <anchor>9676b54b21aedb75cd1024a8d8192b82</anchor>
      <arglist>(MD_Entry *option_entry, MD_Entry *ru_entry)</arglist>
    </member>
    <member kind="function">
      <type>int</type>
      <name>option_usage_common_at_time</name>
      <anchorfile>lmdes2__customizer_8c.html</anchorfile>
      <anchor>b3184354774cd9adb6c800a4751ca015</anchor>
      <arglist>(MD_Entry *option_entry, MD_Entry *usage_entry)</arglist>
    </member>
    <member kind="function">
      <type>void</type>
      <name>remove_common_usage</name>
      <anchorfile>lmdes2__customizer_8c.html</anchorfile>
      <anchor>bd8894f969d13c98f666bbf98a5383a3</anchor>
      <arglist>(MD_Entry *option_entry, MD_Entry *usage_entry)</arglist>
    </member>
    <member kind="function">
      <type>void</type>
      <name>CU_coalesce_unconditional_usages</name>
      <anchorfile>lmdes2__customizer_8c.html</anchorfile>
      <anchor>f2197180102ce352a30d4087b462e14e</anchor>
      <arglist>(MD *md)</arglist>
    </member>
    <member kind="function">
      <type>void</type>
      <name>optimize_resource_units</name>
      <anchorfile>lmdes2__customizer_8c.html</anchorfile>
      <anchor>2160faed9e117c61232adfb550843aaa</anchor>
      <arglist>(MD *md, int max_offset)</arglist>
    </member>
    <member kind="function">
      <type>void</type>
      <name>free_link_table</name>
      <anchorfile>lmdes2__customizer_8c.html</anchorfile>
      <anchor>fa4c4f19e0105b0a4fb487dafb37bd84</anchor>
      <arglist>(INT_Symbol_Table *link_table)</arglist>
    </member>
    <member kind="function">
      <type>INT_Symbol_Table *</type>
      <name>build_link_table</name>
      <anchorfile>lmdes2__customizer_8c.html</anchorfile>
      <anchor>5c514dd5e4fda6d0660bfaf6ea103344</anchor>
      <arglist>(MD *md, MD_Section *target_section)</arglist>
    </member>
    <member kind="function">
      <type>void</type>
      <name>free_remap_table</name>
      <anchorfile>lmdes2__customizer_8c.html</anchorfile>
      <anchor>7b841160a37c9819cb24c2709727eb58</anchor>
      <arglist>(INT_Symbol_Table *remap_table)</arglist>
    </member>
    <member kind="function">
      <type>INT_Symbol_Table *</type>
      <name>build_remap_table</name>
      <anchorfile>lmdes2__customizer_8c.html</anchorfile>
      <anchor>7a6ced70906068dcb26b7acce31d0ca9</anchor>
      <arglist>(MD_Section *section)</arglist>
    </member>
    <member kind="function">
      <type>void</type>
      <name>add_remap_entry</name>
      <anchorfile>lmdes2__customizer_8c.html</anchorfile>
      <anchor>81b5e74760544ee45c3246205a9db199</anchor>
      <arglist>(INT_Symbol_Table *remap_table, MD_Entry *from_entry, MD_Entry *to_entry)</arglist>
    </member>
    <member kind="function">
      <type>void</type>
      <name>free_referenced_table</name>
      <anchorfile>lmdes2__customizer_8c.html</anchorfile>
      <anchor>4b79fa02bfedc02fd8a999e382611b64</anchor>
      <arglist>(INT_Symbol_Table *referenced_table)</arglist>
    </member>
    <member kind="function">
      <type>INT_Symbol_Table *</type>
      <name>build_referenced_table</name>
      <anchorfile>lmdes2__customizer_8c.html</anchorfile>
      <anchor>a6b6366bf2fed45e215362f0acf2c5ba</anchor>
      <arglist>(INT_Symbol_Table *link_table, int init_size)</arglist>
    </member>
    <member kind="function">
      <type>int</type>
      <name>entry_referenced</name>
      <anchorfile>lmdes2__customizer_8c.html</anchorfile>
      <anchor>576134def61f55e5825097f6c622e8be</anchor>
      <arglist>(INT_Symbol_Table *referenced_table, MD_Entry *test_entry)</arglist>
    </member>
    <member kind="function">
      <type>int</type>
      <name>remove_unreferenced_entries</name>
      <anchorfile>lmdes2__customizer_8c.html</anchorfile>
      <anchor>a4a74696d0e80cb977df64ded2259566</anchor>
      <arglist>(MD *md, char *section_name)</arglist>
    </member>
    <member kind="function">
      <type>unsigned int</type>
      <name>CU_hash_string</name>
      <anchorfile>lmdes2__customizer_8c.html</anchorfile>
      <anchor>2c1c0879b965f678466adfa8d6c8e412</anchor>
      <arglist>(char *string)</arglist>
    </member>
    <member kind="function">
      <type>ENTRY_Contents_Table *</type>
      <name>ENTRY_new_contents_table</name>
      <anchorfile>lmdes2__customizer_8c.html</anchorfile>
      <anchor>f16fbbf8f638e1751157ef9598f2ef3a</anchor>
      <arglist>(char *ignore_field, int size)</arglist>
    </member>
    <member kind="function">
      <type>unsigned int</type>
      <name>ENTRY_hash_contents</name>
      <anchorfile>lmdes2__customizer_8c.html</anchorfile>
      <anchor>5111443c1c6b69caeee383c07d23fdc1</anchor>
      <arglist>(MD_Entry *entry1, char *ignore_field)</arglist>
    </member>
    <member kind="function">
      <type>void</type>
      <name>ENTRY_add_contents</name>
      <anchorfile>lmdes2__customizer_8c.html</anchorfile>
      <anchor>f8b740c45578bf755315091df166950f</anchor>
      <arglist>(ENTRY_Contents_Table *table, MD_Entry *entry)</arglist>
    </member>
    <member kind="function">
      <type>MD_Entry *</type>
      <name>ENTRY_find_match</name>
      <anchorfile>lmdes2__customizer_8c.html</anchorfile>
      <anchor>7b037f4ab90fd72816810e1990293073</anchor>
      <arglist>(ENTRY_Contents_Table *table, MD_Entry *entry)</arglist>
    </member>
    <member kind="function">
      <type>void</type>
      <name>ENTRY_free_nodes</name>
      <anchorfile>lmdes2__customizer_8c.html</anchorfile>
      <anchor>9ce7b111a4ff0767a08abb0fd60b3ad3</anchor>
      <arglist>(ENTRY_Contents_Node *first_node)</arglist>
    </member>
    <member kind="function">
      <type>void</type>
      <name>ENTRY_delete_contents_table</name>
      <anchorfile>lmdes2__customizer_8c.html</anchorfile>
      <anchor>935d5d6ef4bbee76bfd07e7f15b71b71</anchor>
      <arglist>(ENTRY_Contents_Table *table)</arglist>
    </member>
    <member kind="function">
      <type>void</type>
      <name>CU_replace_references</name>
      <anchorfile>lmdes2__customizer_8c.html</anchorfile>
      <anchor>e91f10afbc8636670c067c17090d0cf3</anchor>
      <arglist>(INT_Symbol_Table *link_table, INT_Symbol_Table *remap_table)</arglist>
    </member>
    <member kind="function">
      <type>int</type>
      <name>eliminate_redundant_entries</name>
      <anchorfile>lmdes2__customizer_8c.html</anchorfile>
      <anchor>31d467358cd44db3f94464165d183232</anchor>
      <arglist>(MD *md, char *section_name)</arglist>
    </member>
    <member kind="function">
      <type>void</type>
      <name>remove_original_name</name>
      <anchorfile>lmdes2__customizer_8c.html</anchorfile>
      <anchor>6747e11fe0efbbeea18472c8a80f3a2c</anchor>
      <arglist>(MD *md)</arglist>
    </member>
    <member kind="function">
      <type>void</type>
      <name>rename_section_entries</name>
      <anchorfile>lmdes2__customizer_8c.html</anchorfile>
      <anchor>7c5c454e016c0ad79e30d314beb9f3fd</anchor>
      <arglist>(MD *md, char *section_name, char *prefix)</arglist>
    </member>
    <member kind="function">
      <type>int</type>
      <name>optimize_alt_lists</name>
      <anchorfile>lmdes2__customizer_8c.html</anchorfile>
      <anchor>68281f2aaba233f2380c5774122253be</anchor>
      <arglist>(MD *md)</arglist>
    </member>
    <member kind="function">
      <type>int</type>
      <name>do_classical_optimizations</name>
      <anchorfile>lmdes2__customizer_8c.html</anchorfile>
      <anchor>609ff85ad7bd0ce810ba3f0732aef469</anchor>
      <arglist>(MD *md)</arglist>
    </member>
    <member kind="function">
      <type>int</type>
      <name>CU_count_section_entries</name>
      <anchorfile>lmdes2__customizer_8c.html</anchorfile>
      <anchor>5bc12c0dabc53b4b665ed97e99dde230</anchor>
      <arglist>(MD_Section *section)</arglist>
    </member>
    <member kind="function">
      <type>int</type>
      <name>CU_count_entries</name>
      <anchorfile>lmdes2__customizer_8c.html</anchorfile>
      <anchor>d85163db809465f91b4a016d3862ad69</anchor>
      <arglist>(MD *md)</arglist>
    </member>
    <member kind="function">
      <type>void</type>
      <name>check_slot_specification</name>
      <anchorfile>lmdes2__customizer_8c.html</anchorfile>
      <anchor>e58a6292d04bae2e646a784873aff4da</anchor>
      <arglist>(MD *md)</arglist>
    </member>
    <member kind="function">
      <type>void</type>
      <name>mark_resource_unit_slots</name>
      <anchorfile>lmdes2__customizer_8c.html</anchorfile>
      <anchor>5654a965175dc1d0b0e5b172cead0ac7</anchor>
      <arglist>(MD *md)</arglist>
    </member>
    <member kind="function">
      <type>int</type>
      <name>CU_num_options</name>
      <anchorfile>lmdes2__customizer_8c.html</anchorfile>
      <anchor>b856325e80c0fe729a87901b233899df</anchor>
      <arglist>(MD_Entry *entry)</arglist>
    </member>
    <member kind="function">
      <type>int</type>
      <name>CU_max_usage_time</name>
      <anchorfile>lmdes2__customizer_8c.html</anchorfile>
      <anchor>345b1023db31d3c2ed2cde83095ae799</anchor>
      <arglist>(MD_Entry *entry)</arglist>
    </member>
    <member kind="function">
      <type>void</type>
      <name>format_reservation_tables</name>
      <anchorfile>lmdes2__customizer_8c.html</anchorfile>
      <anchor>7633a9393e597832a6f16c70d882d3b5</anchor>
      <arglist>(MD *md)</arglist>
    </member>
    <member kind="function">
      <type>int</type>
      <name>CU_parm_exists</name>
      <anchorfile>lmdes2__customizer_8c.html</anchorfile>
      <anchor>77b3447d0de2a17ecaa97e21cad307d5</anchor>
      <arglist>(MD *md, char *name)</arglist>
    </member>
    <member kind="function">
      <type>void</type>
      <name>CU_set_string_parm</name>
      <anchorfile>lmdes2__customizer_8c.html</anchorfile>
      <anchor>81e61dd51203a0bd0d4756451d4b6c78</anchor>
      <arglist>(MD *md, char *name, char *string_value)</arglist>
    </member>
    <member kind="function">
      <type>void</type>
      <name>CU_set_int_parm</name>
      <anchorfile>lmdes2__customizer_8c.html</anchorfile>
      <anchor>41d97bbca3aeb3d1e569f1e822129a16</anchor>
      <arglist>(MD *md, char *name, int value)</arglist>
    </member>
    <member kind="function">
      <type>void</type>
      <name>expand_options</name>
      <anchorfile>lmdes2__customizer_8c.html</anchorfile>
      <anchor>77106874c18f2790c5bb0073dadf1313</anchor>
      <arglist>(MD_Entry *dest_option, MD_Entry *src_option)</arglist>
    </member>
    <member kind="function">
      <type>void</type>
      <name>expand_out_reservation_tables</name>
      <anchorfile>lmdes2__customizer_8c.html</anchorfile>
      <anchor>fc9ac81a9950e113cd26337a38f6079c</anchor>
      <arglist>(MD *md)</arglist>
    </member>
    <member kind="function">
      <type>void</type>
      <name>calc_forbidden_latencies</name>
      <anchorfile>lmdes2__customizer_8c.html</anchorfile>
      <anchor>4b2133824eaddc147250f9325ba07988</anchor>
      <arglist>(INT_Symbol_Table *forbidden_set, MD_Field *use1_field, MD_Field *use2_field)</arglist>
    </member>
    <member kind="function">
      <type>void</type>
      <name>add_usage</name>
      <anchorfile>lmdes2__customizer_8c.html</anchorfile>
      <anchor>8c6ba9919f402479699c8861c5e39512</anchor>
      <arglist>(Resource_Info *resource, int user, int time)</arglist>
    </member>
    <member kind="function">
      <type>int</type>
      <name>usage_exists</name>
      <anchorfile>lmdes2__customizer_8c.html</anchorfile>
      <anchor>50502d936ec33def8a9137058724306b</anchor>
      <arglist>(Resource_Info *resource, int user, int time)</arglist>
    </member>
    <member kind="function">
      <type>int</type>
      <name>is_forbidden</name>
      <anchorfile>lmdes2__customizer_8c.html</anchorfile>
      <anchor>ac6b4ad0603ffe89733d6f648fe92f55</anchor>
      <arglist>(INT_Symbol_Table ***forbidden_matrix, int user1, int user2, int latency)</arglist>
    </member>
    <member kind="function">
      <type>int</type>
      <name>is_compatable</name>
      <anchorfile>lmdes2__customizer_8c.html</anchorfile>
      <anchor>e57b1f371837875dcd922b96c5d4099d</anchor>
      <arglist>(INT_Symbol_Table ***forbidden_matrix, Resource_Info *resource, int new_user, int new_time)</arglist>
    </member>
    <member kind="function">
      <type>void</type>
      <name>add_to_generating_set</name>
      <anchorfile>lmdes2__customizer_8c.html</anchorfile>
      <anchor>c56e6d5cf030a514b50156a8d5705302</anchor>
      <arglist>(INT_Symbol_Table ***forbidden_matrix, INT_Symbol_Table *set, int user1, int user2, int latency)</arglist>
    </member>
    <member kind="function">
      <type>int</type>
      <name>resource_redundant</name>
      <anchorfile>lmdes2__customizer_8c.html</anchorfile>
      <anchor>ddf6fc9b02168b82e781ab4cfb4f96a2</anchor>
      <arglist>(Resource_Info *resource1, Resource_Info *resource2)</arglist>
    </member>
    <member kind="function">
      <type>void</type>
      <name>delete_resource</name>
      <anchorfile>lmdes2__customizer_8c.html</anchorfile>
      <anchor>bfdef3d3f07e9a7c115420edfd247755</anchor>
      <arglist>(Resource_Info *resource)</arglist>
    </member>
    <member kind="function">
      <type>INT_Symbol_Table *</type>
      <name>build_generating_set</name>
      <anchorfile>lmdes2__customizer_8c.html</anchorfile>
      <anchor>e427d30cb6d003905bbbf42b3fa712b5</anchor>
      <arglist>(MD *md, INT_Symbol_Table ***forbidden_matrix, MD_Entry **unit_array, int num_units)</arglist>
    </member>
    <member kind="function">
      <type>void</type>
      <name>enumerate_pairs</name>
      <anchorfile>lmdes2__customizer_8c.html</anchorfile>
      <anchor>f5f197f8e2818c8aeebcad98a7af54ba</anchor>
      <arglist>(INT_Symbol_Table ***forbidden_matrix, Resource_Info *resource)</arglist>
    </member>
    <member kind="function">
      <type>int</type>
      <name>mark_covered</name>
      <anchorfile>lmdes2__customizer_8c.html</anchorfile>
      <anchor>9f86fef88cfa05bb7b44606ba6d92f8b</anchor>
      <arglist>(INT_Symbol_Table ***forbidden_matrix, Resource_Info *resource, int user, int time)</arglist>
    </member>
    <member kind="function">
      <type>int</type>
      <name>is_usage_committed</name>
      <anchorfile>lmdes2__customizer_8c.html</anchorfile>
      <anchor>9de3070c3c12a6fffae5eab9c66d1925</anchor>
      <arglist>(INT_Symbol_Table ***forbidden_matrix, Resource_Info *resource, int user, int time)</arglist>
    </member>
    <member kind="function">
      <type>int</type>
      <name>count_num_covered</name>
      <anchorfile>lmdes2__customizer_8c.html</anchorfile>
      <anchor>ab8af320e914a5226e613527f8e57a63</anchor>
      <arglist>(INT_Symbol_Table ***forbidden_matrix, Resource_Info *resource, int user, int time)</arglist>
    </member>
    <member kind="function">
      <type>void</type>
      <name>minimize_generating_set</name>
      <anchorfile>lmdes2__customizer_8c.html</anchorfile>
      <anchor>4a7cfc80caf1cc2ac73c2347fa710f63</anchor>
      <arglist>(INT_Symbol_Table *generating_set, INT_Symbol_Table ***forbidden_matrix, int num_units, MD_Entry **unit_array)</arglist>
    </member>
    <member kind="function">
      <type>void</type>
      <name>free_generating_set</name>
      <anchorfile>lmdes2__customizer_8c.html</anchorfile>
      <anchor>4327d8c541da78d5f3f9cf53831da30d</anchor>
      <arglist>(INT_Symbol_Table *generating_set)</arglist>
    </member>
    <member kind="function">
      <type>void</type>
      <name>instantiate_generating_set</name>
      <anchorfile>lmdes2__customizer_8c.html</anchorfile>
      <anchor>9e2ad332512f8a39005bb239d100eef6</anchor>
      <arglist>(INT_Symbol_Table *generating_set, MD_Field **use_array, int num_units, MD_Section *usage_section)</arglist>
    </member>
    <member kind="function">
      <type>void</type>
      <name>reduce_resource_units</name>
      <anchorfile>lmdes2__customizer_8c.html</anchorfile>
      <anchor>80bbeee5328a5227004302dbc729efa9</anchor>
      <arglist>(MD *md)</arglist>
    </member>
    <member kind="function">
      <type>int</type>
      <name>count_table_uses</name>
      <anchorfile>lmdes2__customizer_8c.html</anchorfile>
      <anchor>4ce135688fc954521f1946964d6d3025</anchor>
      <arglist>(MD *md, MD_Entry *table_entry)</arglist>
    </member>
    <member kind="function">
      <type>double</type>
      <name>average_usage_checks</name>
      <anchorfile>lmdes2__customizer_8c.html</anchorfile>
      <anchor>6eb3b2ecf46c562b81d39d54afd41f36</anchor>
      <arglist>(MD *md)</arglist>
    </member>
    <member kind="function">
      <type>int</type>
      <name>calc_best_case_succeed</name>
      <anchorfile>lmdes2__customizer_8c.html</anchorfile>
      <anchor>6953d208b5ecd712ff502c07d454e67b</anchor>
      <arglist>(MD_Entry *alt_entry)</arglist>
    </member>
    <member kind="function">
      <type>int</type>
      <name>calc_best_case_fail</name>
      <anchorfile>lmdes2__customizer_8c.html</anchorfile>
      <anchor>ceb3d2d68fbf7f1698accb82d5742719</anchor>
      <arglist>(MD_Entry *alt_entry)</arglist>
    </member>
    <member kind="function">
      <type>int</type>
      <name>calc_worst_case_fail</name>
      <anchorfile>lmdes2__customizer_8c.html</anchorfile>
      <anchor>d0902b41af4b8f27f9e37f303a00cada</anchor>
      <arglist>(MD_Entry *alt_entry)</arglist>
    </member>
    <member kind="function">
      <type>int</type>
      <name>calc_memory_size</name>
      <anchorfile>lmdes2__customizer_8c.html</anchorfile>
      <anchor>d54eaab305504150964b0f25c12b30d2</anchor>
      <arglist>(MD *md)</arglist>
    </member>
    <member kind="function">
      <type>void</type>
      <name>print_static_stats</name>
      <anchorfile>lmdes2__customizer_8c.html</anchorfile>
      <anchor>b970674a582a8ea1ad5140f669f34581</anchor>
      <arglist>(FILE *out, MD *md)</arglist>
    </member>
    <member kind="function">
      <type>void</type>
      <name>print_ru_tree</name>
      <anchorfile>lmdes2__customizer_8c.html</anchorfile>
      <anchor>fb00d3f1fdaa1c335512b34734911cd3</anchor>
      <arglist>(FILE *out, MD_Entry *ru_entry)</arglist>
    </member>
    <member kind="function">
      <type>void</type>
      <name>print_unit_tree</name>
      <anchorfile>lmdes2__customizer_8c.html</anchorfile>
      <anchor>a8a42d2c311cc844d1ba895443b70aa4</anchor>
      <arglist>(FILE *out, MD_Entry *unit_entry, int option_id)</arglist>
    </member>
    <member kind="function">
      <type>void</type>
      <name>print_option_tree</name>
      <anchorfile>lmdes2__customizer_8c.html</anchorfile>
      <anchor>403032539b9119ba7e8100ea00975277</anchor>
      <arglist>(FILE *out, MD_Entry *option_entry)</arglist>
    </member>
    <member kind="function">
      <type>void</type>
      <name>print_and_or_tree</name>
      <anchorfile>lmdes2__customizer_8c.html</anchorfile>
      <anchor>b3fc3f2427e4193ddc11562a6ed56f7b</anchor>
      <arglist>(FILE *out, MD_Entry *table)</arglist>
    </member>
    <member kind="function">
      <type>int</type>
      <name>CU_calc_max_field_size</name>
      <anchorfile>lmdes2__customizer_8c.html</anchorfile>
      <anchor>ebb6e3da59774d2be9fea3034f0e749a</anchor>
      <arglist>(MD *md, char *section_name, char *field_name)</arglist>
    </member>
    <member kind="function">
      <type>int</type>
      <name>CU_calc_int_field_max_value</name>
      <anchorfile>lmdes2__customizer_8c.html</anchorfile>
      <anchor>5dd5fab3cd53184cc8789f2e2c3ba33a</anchor>
      <arglist>(MD *md, char *section_name, char *field_name)</arglist>
    </member>
    <member kind="function">
      <type>void</type>
      <name>CU_set_slot_parameters</name>
      <anchorfile>lmdes2__customizer_8c.html</anchorfile>
      <anchor>c9352cf521d85bfd11ab88a863ae1459</anchor>
      <arglist>(MD *md)</arglist>
    </member>
    <member kind="function">
      <type>void</type>
      <name>dump_md</name>
      <anchorfile>lmdes2__customizer_8c.html</anchorfile>
      <anchor>e15b6758241bc43ed7472236b84c63bd</anchor>
      <arglist>(char *file_name, MD *md)</arglist>
    </member>
    <member kind="variable">
      <type>char *</type>
      <name>program_name</name>
      <anchorfile>lmdes2__customizer_8c.html</anchorfile>
      <anchor>289c5900d90626d909f0a85d5a0ed61d</anchor>
      <arglist></arglist>
    </member>
    <member kind="variable">
      <type>char *</type>
      <name>input_file_name</name>
      <anchorfile>lmdes2__customizer_8c.html</anchorfile>
      <anchor>a91ff4ee3ee60baee9c097d4da3e3d18</anchor>
      <arglist></arglist>
    </member>
    <member kind="variable">
      <type>char *</type>
      <name>output_file_name</name>
      <anchorfile>lmdes2__customizer_8c.html</anchorfile>
      <anchor>cdcd0d11bb729145f180d52a0755cddf</anchor>
      <arglist></arglist>
    </member>
    <member kind="variable">
      <type>int</type>
      <name>using_stdin</name>
      <anchorfile>lmdes2__customizer_8c.html</anchorfile>
      <anchor>bc340a0cfa46e3f25a4d1adab28331c0</anchor>
      <arglist></arglist>
    </member>
    <member kind="variable">
      <type>char *</type>
      <name>output_mode</name>
      <anchorfile>lmdes2__customizer_8c.html</anchorfile>
      <anchor>14a61661c70b91f945e8456ead9fb6c1</anchor>
      <arglist></arglist>
    </member>
    <member kind="variable">
      <type>char *</type>
      <name>input_mode</name>
      <anchorfile>lmdes2__customizer_8c.html</anchorfile>
      <anchor>f81839da89e889adb517d1fc96a456f9</anchor>
      <arglist></arglist>
    </member>
    <member kind="variable">
      <type>int</type>
      <name>output_page_width</name>
      <anchorfile>lmdes2__customizer_8c.html</anchorfile>
      <anchor>bbe586720dc5d6766839fbdc260002c1</anchor>
      <arglist></arglist>
    </member>
    <member kind="variable">
      <type>int</type>
      <name>verbose</name>
      <anchorfile>lmdes2__customizer_8c.html</anchorfile>
      <anchor>0b2caeb4b6f130be43e5a2f0267dd453</anchor>
      <arglist></arglist>
    </member>
    <member kind="variable">
      <type>int</type>
      <name>expand_tables</name>
      <anchorfile>lmdes2__customizer_8c.html</anchorfile>
      <anchor>563cdd5e89becf23ee5a738704c6f08f</anchor>
      <arglist></arglist>
    </member>
    <member kind="variable">
      <type>int</type>
      <name>tree_opti</name>
      <anchorfile>lmdes2__customizer_8c.html</anchorfile>
      <anchor>b8dfd5ac67573c68c04e14078ea2aeee</anchor>
      <arglist></arglist>
    </member>
    <member kind="variable">
      <type>int</type>
      <name>place_slots_first</name>
      <anchorfile>lmdes2__customizer_8c.html</anchorfile>
      <anchor>b5d3cfa3af0f0d0f3aae26ebc6253224</anchor>
      <arglist></arglist>
    </member>
    <member kind="variable">
      <type>int</type>
      <name>static_stats</name>
      <anchorfile>lmdes2__customizer_8c.html</anchorfile>
      <anchor>0464ae67d9a81d2af58909687534f835</anchor>
      <arglist></arglist>
    </member>
    <member kind="variable">
      <type>int</type>
      <name>print_trees</name>
      <anchorfile>lmdes2__customizer_8c.html</anchorfile>
      <anchor>34ea62a4bb6a61dc0899f0f65c9b8f46</anchor>
      <arglist></arglist>
    </member>
    <member kind="variable">
      <type>int</type>
      <name>resource_minimization</name>
      <anchorfile>lmdes2__customizer_8c.html</anchorfile>
      <anchor>138a3fb2763ae65b4e1a58716ed69355</anchor>
      <arglist></arglist>
    </member>
    <member kind="variable">
      <type>int</type>
      <name>opti_level</name>
      <anchorfile>lmdes2__customizer_8c.html</anchorfile>
      <anchor>20ad8b535e62b1a107e319b7d900a640</anchor>
      <arglist></arglist>
    </member>
    <member kind="variable">
      <type>L_Alloc_Pool *</type>
      <name>ENTRY_Contents_Table_pool</name>
      <anchorfile>lmdes2__customizer_8c.html</anchorfile>
      <anchor>28bf32dd862451062fe00d50a0e83cac</anchor>
      <arglist></arglist>
    </member>
    <member kind="variable">
      <type>L_Alloc_Pool *</type>
      <name>ENTRY_Contents_Node_pool</name>
      <anchorfile>lmdes2__customizer_8c.html</anchorfile>
      <anchor>8f9a19ca198e5d6477591152edc4b9e9</anchor>
      <arglist></arglist>
    </member>
    <member kind="variable">
      <type>STRING_Symbol_Table *</type>
      <name>header_table_table</name>
      <anchorfile>lmdes2__customizer_8c.html</anchorfile>
      <anchor>f225bb0e4137fd9f5fc16ee72bfbd988</anchor>
      <arglist></arglist>
    </member>
    <member kind="variable">
      <type>STRING_Symbol_Table *</type>
      <name>rename_table</name>
      <anchorfile>lmdes2__customizer_8c.html</anchorfile>
      <anchor>b4ce2bf45b0a0cbca226867bcee5970f</anchor>
      <arglist></arglist>
    </member>
    <member kind="variable">
      <type>int</type>
      <name>header_errors</name>
      <anchorfile>lmdes2__customizer_8c.html</anchorfile>
      <anchor>b0b0139b9ffe1f2089e4e43fb71fb4d3</anchor>
      <arglist></arglist>
    </member>
    <member kind="variable">
      <type>int</type>
      <name>customization_errors</name>
      <anchorfile>lmdes2__customizer_8c.html</anchorfile>
      <anchor>d3c82b27bc8b51337e625c1969a5e7bc</anchor>
      <arglist></arglist>
    </member>
  </compound>
  <compound kind="struct">
    <name>ENTRY_Contents_Node</name>
    <filename>structENTRY__Contents__Node.html</filename>
    <member kind="variable">
      <type>MD_Entry *</type>
      <name>entry</name>
      <anchorfile>structENTRY__Contents__Node.html</anchorfile>
      <anchor>1aadfd826769ae425301e7786df83b85</anchor>
      <arglist></arglist>
    </member>
    <member kind="variable">
      <type>ENTRY_Contents_Node *</type>
      <name>next_node</name>
      <anchorfile>structENTRY__Contents__Node.html</anchorfile>
      <anchor>9a1cf42f115da41c98d28ff488611a8f</anchor>
      <arglist></arglist>
    </member>
  </compound>
  <compound kind="struct">
    <name>ENTRY_Contents_Table</name>
    <filename>structENTRY__Contents__Table.html</filename>
    <member kind="variable">
      <type>char *</type>
      <name>ignore_field</name>
      <anchorfile>structENTRY__Contents__Table.html</anchorfile>
      <anchor>ce208a4e15a5fadc43a213d8a4a24dd1</anchor>
      <arglist></arglist>
    </member>
    <member kind="variable">
      <type>INT_Symbol_Table *</type>
      <name>hash_table</name>
      <anchorfile>structENTRY__Contents__Table.html</anchorfile>
      <anchor>849c8abfa8c7e646434e26dbfa80fada</anchor>
      <arglist></arglist>
    </member>
  </compound>
  <compound kind="struct">
    <name>hparse_info_st</name>
    <filename>structhparse__info__st.html</filename>
    <member kind="variable">
      <type>STRING_Symbol_Table *</type>
      <name>table</name>
      <anchorfile>structhparse__info__st.html</anchorfile>
      <anchor>6b396dace9be5ee8ee8d7bfdaa52caad</anchor>
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
    <name>Pair_Node</name>
    <filename>structPair__Node.html</filename>
    <member kind="variable">
      <type>Resource_Info *</type>
      <name>resource</name>
      <anchorfile>structPair__Node.html</anchorfile>
      <anchor>9791ae4ee8b004db541141a808b74b95</anchor>
      <arglist></arglist>
    </member>
    <member kind="variable">
      <type>int</type>
      <name>time1</name>
      <anchorfile>structPair__Node.html</anchorfile>
      <anchor>0794150083c96d6831c84db393e034c8</anchor>
      <arglist></arglist>
    </member>
    <member kind="variable">
      <type>Pair_Node *</type>
      <name>next</name>
      <anchorfile>structPair__Node.html</anchorfile>
      <anchor>52a91852dc793620658dc5ff52f569f7</anchor>
      <arglist></arglist>
    </member>
  </compound>
  <compound kind="struct">
    <name>Pointer_Mapping</name>
    <filename>structPointer__Mapping.html</filename>
    <member kind="variable">
      <type>void *</type>
      <name>from</name>
      <anchorfile>structPointer__Mapping.html</anchorfile>
      <anchor>661c5e8b6062b1e7d04fdcfd8f2df2c4</anchor>
      <arglist></arglist>
    </member>
    <member kind="variable">
      <type>void *</type>
      <name>to</name>
      <anchorfile>structPointer__Mapping.html</anchorfile>
      <anchor>78ef5abc3b5cb05326bb41277ec887e3</anchor>
      <arglist></arglist>
    </member>
  </compound>
  <compound kind="struct">
    <name>Resource_Info</name>
    <filename>structResource__Info.html</filename>
    <member kind="variable">
      <type>char *</type>
      <name>name</name>
      <anchorfile>structResource__Info.html</anchorfile>
      <anchor>9897dc471a7e5f4a91fb0bc2c614b12b</anchor>
      <arglist></arglist>
    </member>
    <member kind="variable">
      <type>MD_Entry *</type>
      <name>resource_entry</name>
      <anchorfile>structResource__Info.html</anchorfile>
      <anchor>a13a7304c90dda9445585f943726e69a</anchor>
      <arglist></arglist>
    </member>
    <member kind="variable">
      <type>INT_Symbol_Table *</type>
      <name>time_table</name>
      <anchorfile>structResource__Info.html</anchorfile>
      <anchor>c445ffe205bc68c63cb5197080e0df73</anchor>
      <arglist></arglist>
    </member>
    <member kind="variable">
      <type>int</type>
      <name>flags</name>
      <anchorfile>structResource__Info.html</anchorfile>
      <anchor>44136b3784d799962f70d46b95e00245</anchor>
      <arglist></arglist>
    </member>
  </compound>
</tagfile>
