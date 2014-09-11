<?xml version='1.0' encoding='ISO-8859-1' standalone='yes' ?>
<tagfile>
  <compound kind="file">
    <name>lmdes.c</name>
    <path>/mnt/hgfs/mkt/Desktop/openimpact/src/machine/Lmdes/</path>
    <filename>lmdes_8c</filename>
    <includes id="lmdes_8h" name="lmdes.h" local="yes" imported="no">lmdes.h</includes>
    <member kind="define">
      <type>#define</type>
      <name>MDES_VERSION</name>
      <anchorfile>lmdes_8c.html</anchorfile>
      <anchor>e92162ea5a88da4afa6f00925478830f</anchor>
      <arglist></arglist>
    </member>
    <member kind="function" static="yes">
      <type>static Mdes *</type>
      <name>load_lmdes</name>
      <anchorfile>lmdes_8c.html</anchorfile>
      <anchor>ac8f0578ba8e0385eaa000abb0afc134</anchor>
      <arglist>(char *file_name)</arglist>
    </member>
    <member kind="function">
      <type>void</type>
      <name>Malloc_struct</name>
      <anchorfile>lmdes_8c.html</anchorfile>
      <anchor>44598a63bd3f29a90fdd27cb3531aff7</anchor>
      <arglist>(void **ptr, int size)</arglist>
    </member>
    <member kind="function">
      <type>void</type>
      <name>Malloc_name</name>
      <anchorfile>lmdes_8c.html</anchorfile>
      <anchor>94186a85a4489b80b7d305e6e13fec4c</anchor>
      <arglist>(char **ptr, char *name)</arglist>
    </member>
    <member kind="function">
      <type>int</type>
      <name>lmdes_initialized</name>
      <anchorfile>lmdes_8c.html</anchorfile>
      <anchor>8036561f663279103ef2f7065b02f1dd</anchor>
      <arglist>(void)</arglist>
    </member>
    <member kind="function">
      <type>int</type>
      <name>mdes_defined_opcode</name>
      <anchorfile>lmdes_8c.html</anchorfile>
      <anchor>ac3528f18876ca767f050631903e3552</anchor>
      <arglist>(unsigned int opcode)</arglist>
    </member>
    <member kind="function">
      <type>int</type>
      <name>mdes_max_opcode</name>
      <anchorfile>lmdes_8c.html</anchorfile>
      <anchor>87c24ac7a003ff4ced90a725d84644f0</anchor>
      <arglist>(void)</arglist>
    </member>
    <member kind="function">
      <type>int</type>
      <name>op_flag_set</name>
      <anchorfile>lmdes_8c.html</anchorfile>
      <anchor>67805a2222096472207405f876bd1303</anchor>
      <arglist>(unsigned int opcode, int mask)</arglist>
    </member>
    <member kind="function">
      <type>int</type>
      <name>alt_flag_set</name>
      <anchorfile>lmdes_8c.html</anchorfile>
      <anchor>8170398096fe078ee847319d5f380200</anchor>
      <arglist>(unsigned int opcode, unsigned int alt_no, int mask)</arglist>
    </member>
    <member kind="function">
      <type>int</type>
      <name>any_alt_flag_set</name>
      <anchorfile>lmdes_8c.html</anchorfile>
      <anchor>9605b95ef76210011ca535870490cd9a</anchor>
      <arglist>(Mdes_Info *mdes_info, int mask)</arglist>
    </member>
    <member kind="function">
      <type>int</type>
      <name>mdes_operand_count</name>
      <anchorfile>lmdes_8c.html</anchorfile>
      <anchor>89ed8771e0c0f175530396acc020c4fe</anchor>
      <arglist>(void)</arglist>
    </member>
    <member kind="function">
      <type>int</type>
      <name>mdes_latency_count</name>
      <anchorfile>lmdes_8c.html</anchorfile>
      <anchor>7c180927e995b9efb844141769e8752a</anchor>
      <arglist>(void)</arglist>
    </member>
    <member kind="function">
      <type>int</type>
      <name>mdes_num_operands</name>
      <anchorfile>lmdes_8c.html</anchorfile>
      <anchor>16c4bed2c322b1d117acf2ad16a06f1c</anchor>
      <arglist>(unsigned int operand_type)</arglist>
    </member>
    <member kind="function">
      <type>int</type>
      <name>mdes_null_operand</name>
      <anchorfile>lmdes_8c.html</anchorfile>
      <anchor>57537f171f1012dd9f07c719b258cec3</anchor>
      <arglist>(void)</arglist>
    </member>
    <member kind="function">
      <type>void</type>
      <name>print_mdes_operand_name</name>
      <anchorfile>lmdes_8c.html</anchorfile>
      <anchor>9f72ff9b38d5ff16f0f12fc4a82d7cac</anchor>
      <arglist>(FILE *out, Mdes *mdes, unsigned int index)</arglist>
    </member>
    <member kind="function" static="yes">
      <type>static int</type>
      <name>IO_sets_intersect</name>
      <anchorfile>lmdes_8c.html</anchorfile>
      <anchor>6b8141e6178783fa24c3f14577e7a28b</anchor>
      <arglist>(Mdes *mdes, Mdes_IO_Set *set1, Mdes_IO_Set *set2)</arglist>
    </member>
    <member kind="function" static="yes">
      <type>static int</type>
      <name>IO_items_compatable</name>
      <anchorfile>lmdes_8c.html</anchorfile>
      <anchor>6c57cbcf8f540277cc5af9715a3ab7d2</anchor>
      <arglist>(Mdes *mdes, Mdes_IO_Set **operand_type1, Mdes_IO_Set **operand_type2)</arglist>
    </member>
    <member kind="function">
      <type>void</type>
      <name>mdes_print_IO_set</name>
      <anchorfile>lmdes_8c.html</anchorfile>
      <anchor>443caf6000199806854433e428d252b5</anchor>
      <arglist>(FILE *out, unsigned int id)</arglist>
    </member>
    <member kind="function">
      <type>Mdes_Info *</type>
      <name>build_mdes_info</name>
      <anchorfile>lmdes_8c.html</anchorfile>
      <anchor>e14f6808b51ea8eeb57db97bb06a5fc3</anchor>
      <arglist>(unsigned int opcode, int *io_list)</arglist>
    </member>
    <member kind="function">
      <type>void</type>
      <name>free_mdes_info</name>
      <anchorfile>lmdes_8c.html</anchorfile>
      <anchor>3e2b7abcf414fa5c31b177b5fa743192</anchor>
      <arglist>(Mdes_Info *info)</arglist>
    </member>
    <member kind="function">
      <type>int</type>
      <name>operand_index</name>
      <anchorfile>lmdes_8c.html</anchorfile>
      <anchor>e84d5f3f40b5dc3a74d29e1f3762c671</anchor>
      <arglist>(unsigned int operand_type, unsigned int operand_number)</arglist>
    </member>
    <member kind="function">
      <type>int</type>
      <name>operand_type</name>
      <anchorfile>lmdes_8c.html</anchorfile>
      <anchor>b5521bc27b7c95d668c2641a6dfcd715</anchor>
      <arglist>(unsigned int operand_index)</arglist>
    </member>
    <member kind="function">
      <type>int</type>
      <name>operand_number</name>
      <anchorfile>lmdes_8c.html</anchorfile>
      <anchor>ff92d6b9d3171335fbe5b748e0b54a74</anchor>
      <arglist>(unsigned int operand_index)</arglist>
    </member>
    <member kind="function">
      <type>int</type>
      <name>max_operand_time</name>
      <anchorfile>lmdes_8c.html</anchorfile>
      <anchor>447043ce5a84413cb09ce4eb5ff1050c</anchor>
      <arglist>(Mdes_Info *mdes_info, int operand_index)</arglist>
    </member>
    <member kind="function">
      <type>int</type>
      <name>min_operand_time</name>
      <anchorfile>lmdes_8c.html</anchorfile>
      <anchor>4b89be1b05bc574c5cc042cdeb8bb750</anchor>
      <arglist>(Mdes_Info *mdes_info, int operand_index)</arglist>
    </member>
    <member kind="function">
      <type>int</type>
      <name>mdes_calc_min_ready_time</name>
      <anchorfile>lmdes_8c.html</anchorfile>
      <anchor>aa82b5ba2b71f304d335536fd6ee607f</anchor>
      <arglist>(Mdes_Info *mdes_info, int *ready_time)</arglist>
    </member>
    <member kind="function">
      <type>int</type>
      <name>mdes_operand_latency</name>
      <anchorfile>lmdes_8c.html</anchorfile>
      <anchor>8d6617c093d8f85c8fd4a7f493b2341e</anchor>
      <arglist>(unsigned int opcode, unsigned int alt_no, unsigned int operand_index)</arglist>
    </member>
    <member kind="function">
      <type>int</type>
      <name>mdes_heuristic_alt_id</name>
      <anchorfile>lmdes_8c.html</anchorfile>
      <anchor>bcb6214dbe99b2a5c7b4fb47cc0b632e</anchor>
      <arglist>(int opcode)</arglist>
    </member>
    <member kind="function">
      <type>int</type>
      <name>mdes_max_completion_time</name>
      <anchorfile>lmdes_8c.html</anchorfile>
      <anchor>589f30951568d11531047a7b3404e6ac</anchor>
      <arglist>(int opcode, int alt_no)</arglist>
    </member>
    <member kind="function">
      <type>void</type>
      <name>L_init_lmdes</name>
      <anchorfile>lmdes_8c.html</anchorfile>
      <anchor>8a5b297b5c6aff02e81706246fe2b37d</anchor>
      <arglist>(char *mdes_file_name)</arglist>
    </member>
    <member kind="function">
      <type>void</type>
      <name>L_init_lmdes2</name>
      <anchorfile>lmdes_8c.html</anchorfile>
      <anchor>31b24a9481d90385d7406c58673a53d1</anchor>
      <arglist>(char *mdes_file_name, int num_pred, int num_dest, int num_src, int num_sync)</arglist>
    </member>
    <member kind="function" static="yes">
      <type>static void</type>
      <name>LM_read_mask</name>
      <anchorfile>lmdes_8c.html</anchorfile>
      <anchor>3a8a33a51509bf442b189ed800731cdf</anchor>
      <arglist>(FILE *in, int *mask, int width)</arglist>
    </member>
    <member kind="function">
      <type>void</type>
      <name>print_mdes</name>
      <anchorfile>lmdes_8c.html</anchorfile>
      <anchor>41090f500ae1c4c9214bbbf9926b249a</anchor>
      <arglist>(FILE *out, Mdes *mdes)</arglist>
    </member>
    <member kind="variable">
      <type>Mdes *</type>
      <name>lmdes</name>
      <anchorfile>lmdes_8c.html</anchorfile>
      <anchor>fccb3150d93023d97c16ad96e4db32dd</anchor>
      <arglist></arglist>
    </member>
    <member kind="variable" static="yes">
      <type>static L_Alloc_Pool *</type>
      <name>Mdes_Info_pool</name>
      <anchorfile>lmdes_8c.html</anchorfile>
      <anchor>5d6fec35dbf5f1069345bbc1cb3030ee</anchor>
      <arglist></arglist>
    </member>
    <member kind="variable" static="yes">
      <type>static L_Alloc_Pool *</type>
      <name>Mdes_Compatable_Alt_pool</name>
      <anchorfile>lmdes_8c.html</anchorfile>
      <anchor>b5865ba7b34c98e4c0763256bfa17a36</anchor>
      <arglist></arglist>
    </member>
  </compound>
  <compound kind="file">
    <name>lmdes.h</name>
    <path>/mnt/hgfs/mkt/Desktop/openimpact/src/machine/Lmdes/</path>
    <filename>lmdes_8h</filename>
    <class kind="struct">mdes_IO_set_st</class>
    <class kind="struct">mdes_IO_item_st</class>
    <class kind="struct">mdes_resource_st</class>
    <class kind="struct">mdes_rmask_st</class>
    <class kind="struct">mdes_Rused</class>
    <class kind="struct">mdes_reslist_st</class>
    <class kind="struct">mdes_latency_st</class>
    <class kind="struct">mdes_alt_st</class>
    <class kind="struct">Mdes_Stats</class>
    <class kind="struct">mdes_operation_st</class>
    <class kind="struct">mdes_st</class>
    <class kind="struct">mdes_compatable_alt_st</class>
    <class kind="struct">Mdes_Info</class>
    <member kind="define">
      <type>#define</type>
      <name>MDES_INFO</name>
      <anchorfile>lmdes_8h.html</anchorfile>
      <anchor>c3fee9a9b81fcd259ac346ef74766f48</anchor>
      <arglist>(oper)</arglist>
    </member>
    <member kind="define">
      <type>#define</type>
      <name>MDES_PRED</name>
      <anchorfile>lmdes_8h.html</anchorfile>
      <anchor>af4ae62f6ecd434d7159ead2988d8382</anchor>
      <arglist></arglist>
    </member>
    <member kind="define">
      <type>#define</type>
      <name>MDES_DEST</name>
      <anchorfile>lmdes_8h.html</anchorfile>
      <anchor>6b7b3b9d34884591f194e9362a4d787e</anchor>
      <arglist></arglist>
    </member>
    <member kind="define">
      <type>#define</type>
      <name>MDES_SRC</name>
      <anchorfile>lmdes_8h.html</anchorfile>
      <anchor>d6662cf14c3f61ef04bbfa00fa496bba</anchor>
      <arglist></arglist>
    </member>
    <member kind="define">
      <type>#define</type>
      <name>MDES_SYNC_IN</name>
      <anchorfile>lmdes_8h.html</anchorfile>
      <anchor>68139cefe2a913e6bf58ee671e70e620</anchor>
      <arglist></arglist>
    </member>
    <member kind="define">
      <type>#define</type>
      <name>MDES_SYNC_OUT</name>
      <anchorfile>lmdes_8h.html</anchorfile>
      <anchor>8d186a9d28efc70dbedced22702e23f8</anchor>
      <arglist></arglist>
    </member>
    <member kind="define">
      <type>#define</type>
      <name>MDES_SUPERSCALAR</name>
      <anchorfile>lmdes_8h.html</anchorfile>
      <anchor>2202ce53709c0e408a63030c29fead58</anchor>
      <arglist></arglist>
    </member>
    <member kind="define">
      <type>#define</type>
      <name>MDES_VLIW</name>
      <anchorfile>lmdes_8h.html</anchorfile>
      <anchor>1dd3b99b38350ea5ad6a1ae70624f7a4</anchor>
      <arglist></arglist>
    </member>
    <member kind="define">
      <type>#define</type>
      <name>MDES_OVERLAPPING_REQUEST</name>
      <anchorfile>lmdes_8h.html</anchorfile>
      <anchor>66e95518415e6886ce1ea4fea1f11f2a</anchor>
      <arglist></arglist>
    </member>
    <member kind="define">
      <type>#define</type>
      <name>mdes_processor_model</name>
      <anchorfile>lmdes_8h.html</anchorfile>
      <anchor>1c2eedb335670f5aab26b0b7b9686296</anchor>
      <arglist>()</arglist>
    </member>
    <member kind="define">
      <type>#define</type>
      <name>mdes_total_slots</name>
      <anchorfile>lmdes_8h.html</anchorfile>
      <anchor>a3177b0b40f3125b0ce34d7938e2dbbd</anchor>
      <arglist>()</arglist>
    </member>
    <member kind="typedef">
      <type>mdes_IO_set_st</type>
      <name>Mdes_IO_Set</name>
      <anchorfile>lmdes_8h.html</anchorfile>
      <anchor>952cfc93036cec851c6113fcb7cdd4a2</anchor>
      <arglist></arglist>
    </member>
    <member kind="typedef">
      <type>mdes_IO_item_st</type>
      <name>Mdes_IO_Item</name>
      <anchorfile>lmdes_8h.html</anchorfile>
      <anchor>c87d75377929b8a420f63ec606b8507b</anchor>
      <arglist></arglist>
    </member>
    <member kind="typedef">
      <type>mdes_resource_st</type>
      <name>Mdes_Resource</name>
      <anchorfile>lmdes_8h.html</anchorfile>
      <anchor>34745f34bf859432de674d86f42ec32a</anchor>
      <arglist></arglist>
    </member>
    <member kind="typedef">
      <type>mdes_rmask_st</type>
      <name>Mdes_Rmask</name>
      <anchorfile>lmdes_8h.html</anchorfile>
      <anchor>01b9dc93e286b9da26feca2b34d88f3c</anchor>
      <arglist></arglist>
    </member>
    <member kind="typedef">
      <type>mdes_Rused</type>
      <name>Mdes_Rused</name>
      <anchorfile>lmdes_8h.html</anchorfile>
      <anchor>6e56c6218e6e640a592e163cf3d0edbb</anchor>
      <arglist></arglist>
    </member>
    <member kind="typedef">
      <type>mdes_reslist_st</type>
      <name>Mdes_ResList</name>
      <anchorfile>lmdes_8h.html</anchorfile>
      <anchor>e3f1e591ab5452a9f1df447c9f7b5d2f</anchor>
      <arglist></arglist>
    </member>
    <member kind="typedef">
      <type>mdes_latency_st</type>
      <name>Mdes_Latency</name>
      <anchorfile>lmdes_8h.html</anchorfile>
      <anchor>48360901c68bd565427b2570d662d0b5</anchor>
      <arglist></arglist>
    </member>
    <member kind="typedef">
      <type>mdes_alt_st</type>
      <name>Mdes_Alt</name>
      <anchorfile>lmdes_8h.html</anchorfile>
      <anchor>4589dad1c6599eb0f74abbeda089e76c</anchor>
      <arglist></arglist>
    </member>
    <member kind="typedef">
      <type>mdes_operation_st</type>
      <name>Mdes_Operation</name>
      <anchorfile>lmdes_8h.html</anchorfile>
      <anchor>54073b7cc26278125d7a2d1737517542</anchor>
      <arglist></arglist>
    </member>
    <member kind="typedef">
      <type>mdes_st</type>
      <name>Mdes</name>
      <anchorfile>lmdes_8h.html</anchorfile>
      <anchor>81d40116ad293c4a88ed3f5ebde9a6c5</anchor>
      <arglist></arglist>
    </member>
    <member kind="typedef">
      <type>mdes_compatable_alt_st</type>
      <name>Mdes_Compatable_Alt</name>
      <anchorfile>lmdes_8h.html</anchorfile>
      <anchor>7713e4afedbaa57e0062663a1b6cbe22</anchor>
      <arglist></arglist>
    </member>
    <member kind="function">
      <type>void</type>
      <name>L_init_lmdes</name>
      <anchorfile>lmdes_8h.html</anchorfile>
      <anchor>8a5b297b5c6aff02e81706246fe2b37d</anchor>
      <arglist>(char *mdes_file_name)</arglist>
    </member>
    <member kind="function">
      <type>void</type>
      <name>L_init_lmdes2</name>
      <anchorfile>lmdes_8h.html</anchorfile>
      <anchor>31b24a9481d90385d7406c58673a53d1</anchor>
      <arglist>(char *mdes_file_name, int num_pred, int num_dest, int num_src, int num_sync)</arglist>
    </member>
    <member kind="function">
      <type>void</type>
      <name>print_mdes</name>
      <anchorfile>lmdes_8h.html</anchorfile>
      <anchor>41090f500ae1c4c9214bbbf9926b249a</anchor>
      <arglist>(FILE *out, Mdes *mdes)</arglist>
    </member>
    <member kind="function">
      <type>int</type>
      <name>lmdes_initialized</name>
      <anchorfile>lmdes_8h.html</anchorfile>
      <anchor>8036561f663279103ef2f7065b02f1dd</anchor>
      <arglist>(void)</arglist>
    </member>
    <member kind="function">
      <type>Mdes_Info *</type>
      <name>build_mdes_info</name>
      <anchorfile>lmdes_8h.html</anchorfile>
      <anchor>e14f6808b51ea8eeb57db97bb06a5fc3</anchor>
      <arglist>(unsigned int opcode, int *io_list)</arglist>
    </member>
    <member kind="function">
      <type>void</type>
      <name>free_mdes_info</name>
      <anchorfile>lmdes_8h.html</anchorfile>
      <anchor>3e2b7abcf414fa5c31b177b5fa743192</anchor>
      <arglist>(Mdes_Info *info)</arglist>
    </member>
    <member kind="function">
      <type>void</type>
      <name>print_mdes_operand_name</name>
      <anchorfile>lmdes_8h.html</anchorfile>
      <anchor>9f72ff9b38d5ff16f0f12fc4a82d7cac</anchor>
      <arglist>(FILE *out, Mdes *mdes, unsigned int index)</arglist>
    </member>
    <member kind="function">
      <type>void</type>
      <name>mdes_print_IO_set</name>
      <anchorfile>lmdes_8h.html</anchorfile>
      <anchor>443caf6000199806854433e428d252b5</anchor>
      <arglist>(FILE *out, unsigned int id)</arglist>
    </member>
    <member kind="function">
      <type>int</type>
      <name>mdes_defined_opcode</name>
      <anchorfile>lmdes_8h.html</anchorfile>
      <anchor>ac3528f18876ca767f050631903e3552</anchor>
      <arglist>(unsigned int opcode)</arglist>
    </member>
    <member kind="function">
      <type>int</type>
      <name>op_flag_set</name>
      <anchorfile>lmdes_8h.html</anchorfile>
      <anchor>67805a2222096472207405f876bd1303</anchor>
      <arglist>(unsigned int opcode, int mask)</arglist>
    </member>
    <member kind="function">
      <type>int</type>
      <name>alt_flag_set</name>
      <anchorfile>lmdes_8h.html</anchorfile>
      <anchor>8170398096fe078ee847319d5f380200</anchor>
      <arglist>(unsigned int opcode, unsigned int alt_no, int mask)</arglist>
    </member>
    <member kind="function">
      <type>int</type>
      <name>any_alt_flag_set</name>
      <anchorfile>lmdes_8h.html</anchorfile>
      <anchor>9605b95ef76210011ca535870490cd9a</anchor>
      <arglist>(Mdes_Info *mdes_info, int mask)</arglist>
    </member>
    <member kind="function">
      <type>int</type>
      <name>mdes_max_opcode</name>
      <anchorfile>lmdes_8h.html</anchorfile>
      <anchor>87c24ac7a003ff4ced90a725d84644f0</anchor>
      <arglist>(void)</arglist>
    </member>
    <member kind="function">
      <type>int</type>
      <name>mdes_operand_count</name>
      <anchorfile>lmdes_8h.html</anchorfile>
      <anchor>89ed8771e0c0f175530396acc020c4fe</anchor>
      <arglist>(void)</arglist>
    </member>
    <member kind="function">
      <type>int</type>
      <name>mdes_latency_count</name>
      <anchorfile>lmdes_8h.html</anchorfile>
      <anchor>7c180927e995b9efb844141769e8752a</anchor>
      <arglist>(void)</arglist>
    </member>
    <member kind="function">
      <type>int</type>
      <name>mdes_num_operands</name>
      <anchorfile>lmdes_8h.html</anchorfile>
      <anchor>16c4bed2c322b1d117acf2ad16a06f1c</anchor>
      <arglist>(unsigned int operand_type)</arglist>
    </member>
    <member kind="function">
      <type>int</type>
      <name>mdes_null_operand</name>
      <anchorfile>lmdes_8h.html</anchorfile>
      <anchor>57537f171f1012dd9f07c719b258cec3</anchor>
      <arglist>(void)</arglist>
    </member>
    <member kind="function">
      <type>int</type>
      <name>operand_index</name>
      <anchorfile>lmdes_8h.html</anchorfile>
      <anchor>e84d5f3f40b5dc3a74d29e1f3762c671</anchor>
      <arglist>(unsigned int operand_type, unsigned int operand_number)</arglist>
    </member>
    <member kind="function">
      <type>int</type>
      <name>operand_type</name>
      <anchorfile>lmdes_8h.html</anchorfile>
      <anchor>b5521bc27b7c95d668c2641a6dfcd715</anchor>
      <arglist>(unsigned int operand_index)</arglist>
    </member>
    <member kind="function">
      <type>int</type>
      <name>operand_number</name>
      <anchorfile>lmdes_8h.html</anchorfile>
      <anchor>ff92d6b9d3171335fbe5b748e0b54a74</anchor>
      <arglist>(unsigned int operand_index)</arglist>
    </member>
    <member kind="function">
      <type>int</type>
      <name>max_operand_time</name>
      <anchorfile>lmdes_8h.html</anchorfile>
      <anchor>447043ce5a84413cb09ce4eb5ff1050c</anchor>
      <arglist>(Mdes_Info *mdes_info, int operand_index)</arglist>
    </member>
    <member kind="function">
      <type>int</type>
      <name>min_operand_time</name>
      <anchorfile>lmdes_8h.html</anchorfile>
      <anchor>4b89be1b05bc574c5cc042cdeb8bb750</anchor>
      <arglist>(Mdes_Info *mdes_info, int operand_index)</arglist>
    </member>
    <member kind="function">
      <type>int</type>
      <name>mdes_calc_min_ready_time</name>
      <anchorfile>lmdes_8h.html</anchorfile>
      <anchor>aa82b5ba2b71f304d335536fd6ee607f</anchor>
      <arglist>(Mdes_Info *mdes_info, int *ready_time)</arglist>
    </member>
    <member kind="function">
      <type>int</type>
      <name>mdes_operand_latency</name>
      <anchorfile>lmdes_8h.html</anchorfile>
      <anchor>8d6617c093d8f85c8fd4a7f493b2341e</anchor>
      <arglist>(unsigned int opcode, unsigned int alt_no, unsigned int operand_index)</arglist>
    </member>
    <member kind="function">
      <type>int</type>
      <name>mdes_max_completion_time</name>
      <anchorfile>lmdes_8h.html</anchorfile>
      <anchor>589f30951568d11531047a7b3404e6ac</anchor>
      <arglist>(int opcode, int alt_no)</arglist>
    </member>
    <member kind="function">
      <type>int</type>
      <name>mdes_heuristic_alt_id</name>
      <anchorfile>lmdes_8h.html</anchorfile>
      <anchor>bcb6214dbe99b2a5c7b4fb47cc0b632e</anchor>
      <arglist>(int opcode)</arglist>
    </member>
    <member kind="function">
      <type>Mdes *</type>
      <name>load_lmdes_from_version2</name>
      <anchorfile>lmdes_8h.html</anchorfile>
      <anchor>e2378554663e4703270b58eafd85f52f</anchor>
      <arglist>(char *mdes_file_name, int num_pred, int num_dest, int num_src, int num_sync)</arglist>
    </member>
    <member kind="function">
      <type>void</type>
      <name>Malloc_struct</name>
      <anchorfile>lmdes_8h.html</anchorfile>
      <anchor>44598a63bd3f29a90fdd27cb3531aff7</anchor>
      <arglist>(void **ptr, int size)</arglist>
    </member>
    <member kind="function">
      <type>void</type>
      <name>Malloc_name</name>
      <anchorfile>lmdes_8h.html</anchorfile>
      <anchor>94186a85a4489b80b7d305e6e13fec4c</anchor>
      <arglist>(char **ptr, char *name)</arglist>
    </member>
    <member kind="function">
      <type>void</type>
      <name>print_mdes_stats</name>
      <anchorfile>lmdes_8h.html</anchorfile>
      <anchor>8df6c58c53e5c8878fdacd3b6bd89819</anchor>
      <arglist>(FILE *out, int prepass)</arglist>
    </member>
    <member kind="function">
      <type>void</type>
      <name>increment_check_history</name>
      <anchorfile>lmdes_8h.html</anchorfile>
      <anchor>29f817af40da58d6d2d8154a3d6f425f</anchor>
      <arglist>(INT_Symbol_Table *table, int num_checks, int inc)</arglist>
    </member>
    <member kind="variable">
      <type>Mdes *</type>
      <name>lmdes</name>
      <anchorfile>lmdes_8h.html</anchorfile>
      <anchor>fccb3150d93023d97c16ad96e4db32dd</anchor>
      <arglist></arglist>
    </member>
  </compound>
  <compound kind="file">
    <name>lmdes2.c</name>
    <path>/mnt/hgfs/mkt/Desktop/openimpact/src/machine/Lmdes/</path>
    <filename>lmdes2_8c</filename>
    <includes id="lmdes_8h" name="lmdes.h" local="yes" imported="no">lmdes.h</includes>
    <includes id="lmdes2_8h" name="lmdes2.h" local="yes" imported="no">lmdes2.h</includes>
    <includes id="mdes2_8h" name="mdes2.h" local="yes" imported="no">mdes2.h</includes>
    <member kind="define">
      <type>#define</type>
      <name>DEBUG_LMDES2</name>
      <anchorfile>lmdes2_8c.html</anchorfile>
      <anchor>cbd33eb94d61d242d3c1c9077a8cbe3a</anchor>
      <arglist></arglist>
    </member>
    <member kind="function">
      <type>int</type>
      <name>SM_add_ids_to_entries</name>
      <anchorfile>lmdes2_8c.html</anchorfile>
      <anchor>4d1665aeb658c58e66a84443a500d2c2</anchor>
      <arglist>(MD_Section *section)</arglist>
    </member>
    <member kind="function" static="yes">
      <type>static void</type>
      <name>init_stats</name>
      <anchorfile>lmdes2_8c.html</anchorfile>
      <anchor>78199cabec4aa12318cc0b82389c019e</anchor>
      <arglist>(Mdes_Stats *stats)</arglist>
    </member>
    <member kind="function">
      <type>MD_Field *</type>
      <name>find_parm_value</name>
      <anchorfile>lmdes2_8c.html</anchorfile>
      <anchor>8c53cfcfc329fbf79f3f646dad15c6af</anchor>
      <arglist>(MD *md, char *parm_name)</arglist>
    </member>
    <member kind="function">
      <type>MD_Field *</type>
      <name>find_field_by_name</name>
      <anchorfile>lmdes2_8c.html</anchorfile>
      <anchor>d8b800468ff0c9640e7b77394d5d0ae2</anchor>
      <arglist>(MD_Entry *entry, char *field_name)</arglist>
    </member>
    <member kind="function">
      <type>char *</type>
      <name>get_string_parm</name>
      <anchorfile>lmdes2_8c.html</anchorfile>
      <anchor>bbfde937235d598faa885cf283cec940</anchor>
      <arglist>(MD *md, char *parm_name)</arglist>
    </member>
    <member kind="function">
      <type>int</type>
      <name>get_int_parm</name>
      <anchorfile>lmdes2_8c.html</anchorfile>
      <anchor>b72d20b5453850f8f529b452f87f0c6b</anchor>
      <arglist>(MD *md, char *parm_name)</arglist>
    </member>
    <member kind="function" static="yes">
      <type>static int</type>
      <name>value_match</name>
      <anchorfile>lmdes2_8c.html</anchorfile>
      <anchor>fc22e1d6a3d854cec8597ccbfa14d6ca</anchor>
      <arglist>(char *s1, char *s2)</arglist>
    </member>
    <member kind="function">
      <type>int</type>
      <name>get_binary_parm</name>
      <anchorfile>lmdes2_8c.html</anchorfile>
      <anchor>02d89f445d36e1ba6e2cce84865d2689</anchor>
      <arglist>(MD *md, char *parm_name)</arglist>
    </member>
    <member kind="function">
      <type>int</type>
      <name>get_num_entries</name>
      <anchorfile>lmdes2_8c.html</anchorfile>
      <anchor>bebae7b6c0099bb98773c6352ead5f47</anchor>
      <arglist>(MD *md, char *section_name)</arglist>
    </member>
    <member kind="function">
      <type>int</type>
      <name>get_int_value</name>
      <anchorfile>lmdes2_8c.html</anchorfile>
      <anchor>43559819289a0e8fab0c93489486f057</anchor>
      <arglist>(MD *md, char *section_name, char *entry_name, char *field_name, int index)</arglist>
    </member>
    <member kind="function">
      <type>int</type>
      <name>calc_total_entry_name_len</name>
      <anchorfile>lmdes2_8c.html</anchorfile>
      <anchor>3643591e6f1acf88563df0bf51208342</anchor>
      <arglist>(MD *md, char *section_name)</arglist>
    </member>
    <member kind="function">
      <type>void</type>
      <name>set_latencies</name>
      <anchorfile>lmdes2_8c.html</anchorfile>
      <anchor>53965a6fe6b1113e05485fde7c10d391</anchor>
      <arglist>(MD_Entry *entry, char *field_name, int *lat_array)</arglist>
    </member>
    <member kind="function">
      <type>int</type>
      <name>calc_num_op_alts</name>
      <anchorfile>lmdes2_8c.html</anchorfile>
      <anchor>e51936eb893fc5d7cc61f2dea3c4cd9c</anchor>
      <arglist>(MD_Entry *impact_op_entry)</arglist>
    </member>
    <member kind="function">
      <type>int</type>
      <name>calc_num_alts</name>
      <anchorfile>lmdes2_8c.html</anchorfile>
      <anchor>faa1394d0cf90c7e14bf82c367326c07</anchor>
      <arglist>(MD *md)</arglist>
    </member>
    <member kind="function">
      <type>int</type>
      <name>calc_op_total_string_len</name>
      <anchorfile>lmdes2_8c.html</anchorfile>
      <anchor>7967aafc33685b5b6a38c9d7f3fe2bf9</anchor>
      <arglist>(MD *md)</arglist>
    </member>
    <member kind="function">
      <type>Mdes *</type>
      <name>load_lmdes_from_version2</name>
      <anchorfile>lmdes2_8c.html</anchorfile>
      <anchor>aab44dbf962252b56f0b1b97fd3da680</anchor>
      <arglist>(char *file_name, int num_pred, int num_dest, int num_src, int num_sync)</arglist>
    </member>
    <member kind="function">
      <type>void</type>
      <name>increment_check_history</name>
      <anchorfile>lmdes2_8c.html</anchorfile>
      <anchor>29f817af40da58d6d2d8154a3d6f425f</anchor>
      <arglist>(INT_Symbol_Table *table, int num_checks, int inc)</arglist>
    </member>
    <member kind="function" static="yes">
      <type>static void</type>
      <name>add_check_history</name>
      <anchorfile>lmdes2_8c.html</anchorfile>
      <anchor>0126b1769b797ae98c87c73835892b8f</anchor>
      <arglist>(INT_Symbol_Table *dest, INT_Symbol_Table *src)</arglist>
    </member>
    <member kind="function" static="yes">
      <type>static void</type>
      <name>add_stats</name>
      <anchorfile>lmdes2_8c.html</anchorfile>
      <anchor>4df107e0ccc474fa9c0486d3a0c6a35e</anchor>
      <arglist>(Mdes_Stats *total, Mdes_Stats *add)</arglist>
    </member>
    <member kind="function">
      <type>void</type>
      <name>print_check_history</name>
      <anchorfile>lmdes2_8c.html</anchorfile>
      <anchor>03b8d53dbd6d6382363c8186cb91193a</anchor>
      <arglist>(FILE *out, char *name, char *type, INT_Symbol_Table *table)</arglist>
    </member>
    <member kind="function">
      <type>void</type>
      <name>print_op_stats</name>
      <anchorfile>lmdes2_8c.html</anchorfile>
      <anchor>87e34ca62bc70490fa7d9c29f6f85044</anchor>
      <arglist>(FILE *out, char *name, Mdes_Stats *can_stats, Mdes_Stats *do_stats)</arglist>
    </member>
    <member kind="function">
      <type>void</type>
      <name>print_mdes_stats</name>
      <anchorfile>lmdes2_8c.html</anchorfile>
      <anchor>8df6c58c53e5c8878fdacd3b6bd89819</anchor>
      <arglist>(FILE *out, int prepass)</arglist>
    </member>
  </compound>
  <compound kind="file">
    <name>lmdes2.h</name>
    <path>/mnt/hgfs/mkt/Desktop/openimpact/src/machine/Lmdes/</path>
    <filename>lmdes2_8h</filename>
    <member kind="function">
      <type>int</type>
      <name>get_int_parm</name>
      <anchorfile>lmdes2_8h.html</anchorfile>
      <anchor>b72d20b5453850f8f529b452f87f0c6b</anchor>
      <arglist>(MD *md, char *parm_name)</arglist>
    </member>
  </compound>
  <compound kind="file">
    <name>lmdes_interface.c</name>
    <path>/mnt/hgfs/mkt/Desktop/openimpact/src/machine/Lmdes/</path>
    <filename>lmdes__interface_8c</filename>
    <includes id="lmdes_8h" name="lmdes.h" local="yes" imported="no">lmdes.h</includes>
    <includes id="lmdes__interface_8h" name="lmdes_interface.h" local="yes" imported="no">lmdes_interface.h</includes>
    <member kind="function" static="yes">
      <type>static void</type>
      <name>L_initialize_build_mdes_info</name>
      <anchorfile>lmdes__interface_8c.html</anchorfile>
      <anchor>7ae4c8e51787fe1d69ea4221f223d109</anchor>
      <arglist>(void)</arglist>
    </member>
    <member kind="function" static="yes">
      <type>static void</type>
      <name>print_mdes_info_debug_info</name>
      <anchorfile>lmdes__interface_8c.html</anchorfile>
      <anchor>331d0974b7653bdd616d2b956466aad1</anchor>
      <arglist>(L_Oper *op, int opcode, char *func_name)</arglist>
    </member>
    <member kind="function" static="yes">
      <type>static int</type>
      <name>mdes_build_iolist</name>
      <anchorfile>lmdes__interface_8c.html</anchorfile>
      <anchor>e4b6847eed215e3055e4565f7fe0670a</anchor>
      <arglist>(L_Oper *op, int **dest, int **src, int **pred)</arglist>
    </member>
    <member kind="function">
      <type>void</type>
      <name>L_build_cb_mdes_info</name>
      <anchorfile>lmdes__interface_8c.html</anchorfile>
      <anchor>2d2835ec3cbec05456f9a6c34bf5eb92</anchor>
      <arglist>(L_Cb *cb)</arglist>
    </member>
    <member kind="function">
      <type>void</type>
      <name>L_build_oper_mdes_info</name>
      <anchorfile>lmdes__interface_8c.html</anchorfile>
      <anchor>63a7bbae5be233d1ecd8e767e686f6b5</anchor>
      <arglist>(L_Oper *op)</arglist>
    </member>
    <member kind="function">
      <type>void</type>
      <name>L_free_cb_mdes_info</name>
      <anchorfile>lmdes__interface_8c.html</anchorfile>
      <anchor>78e2d3c1739cb67e8a13e5bdf9130233</anchor>
      <arglist>(L_Cb *cb)</arglist>
    </member>
    <member kind="function">
      <type>void</type>
      <name>L_free_oper_mdes_info</name>
      <anchorfile>lmdes__interface_8c.html</anchorfile>
      <anchor>16b54b93a47c58b6f32309b486b937be</anchor>
      <arglist>(L_Oper *op)</arglist>
    </member>
    <member kind="variable" static="yes">
      <type>static int</type>
      <name>_build_mdes_info_initialized</name>
      <anchorfile>lmdes__interface_8c.html</anchorfile>
      <anchor>63b6b1387c0e88a6bc5c7e3e31e1a252</anchor>
      <arglist></arglist>
    </member>
    <member kind="variable" static="yes">
      <type>static int **</type>
      <name>_io_list_dest</name>
      <anchorfile>lmdes__interface_8c.html</anchorfile>
      <anchor>1d6043f456863a888025840b1f1fa2f5</anchor>
      <arglist></arglist>
    </member>
    <member kind="variable" static="yes">
      <type>static int **</type>
      <name>_io_list_src</name>
      <anchorfile>lmdes__interface_8c.html</anchorfile>
      <anchor>bab5cb455004fc793810600adc9f8eb1</anchor>
      <arglist></arglist>
    </member>
    <member kind="variable" static="yes">
      <type>static int **</type>
      <name>_io_list_pred</name>
      <anchorfile>lmdes__interface_8c.html</anchorfile>
      <anchor>d02863cc9dee027e49dc7b55480fa826</anchor>
      <arglist></arglist>
    </member>
    <member kind="variable" static="yes">
      <type>static int *</type>
      <name>_io_list</name>
      <anchorfile>lmdes__interface_8c.html</anchorfile>
      <anchor>1796fb71ba72007ffbb7fd467e95c706</anchor>
      <arglist></arglist>
    </member>
    <member kind="variable" static="yes">
      <type>static int(*)</type>
      <name>mdes_get_operand_type_fptr</name>
      <anchorfile>lmdes__interface_8c.html</anchorfile>
      <anchor>6144d11657dc1f19d101c23f1767fe3b</anchor>
      <arglist>(L_Operand *operand)</arglist>
    </member>
  </compound>
  <compound kind="file">
    <name>lmdes_interface.h</name>
    <path>/mnt/hgfs/mkt/Desktop/openimpact/src/machine/Lmdes/</path>
    <filename>lmdes__interface_8h</filename>
    <includes id="lmdes_8h" name="lmdes.h" local="yes" imported="no">lmdes.h</includes>
    <member kind="function">
      <type>void</type>
      <name>L_build_oper_mdes_info</name>
      <anchorfile>lmdes__interface_8h.html</anchorfile>
      <anchor>63a7bbae5be233d1ecd8e767e686f6b5</anchor>
      <arglist>(L_Oper *op)</arglist>
    </member>
    <member kind="function">
      <type>void</type>
      <name>L_build_cb_mdes_info</name>
      <anchorfile>lmdes__interface_8h.html</anchorfile>
      <anchor>2d2835ec3cbec05456f9a6c34bf5eb92</anchor>
      <arglist>(L_Cb *cb)</arglist>
    </member>
    <member kind="function">
      <type>void</type>
      <name>L_free_oper_mdes_info</name>
      <anchorfile>lmdes__interface_8h.html</anchorfile>
      <anchor>16b54b93a47c58b6f32309b486b937be</anchor>
      <arglist>(L_Oper *op)</arglist>
    </member>
    <member kind="function">
      <type>void</type>
      <name>L_free_cb_mdes_info</name>
      <anchorfile>lmdes__interface_8h.html</anchorfile>
      <anchor>78e2d3c1739cb67e8a13e5bdf9130233</anchor>
      <arglist>(L_Cb *cb)</arglist>
    </member>
  </compound>
  <compound kind="file">
    <name>mdes2.c</name>
    <path>/mnt/hgfs/mkt/Desktop/openimpact/src/machine/Lmdes/</path>
    <filename>mdes2_8c</filename>
    <includes id="lmdes_8h" name="lmdes.h" local="yes" imported="no">lmdes.h</includes>
    <includes id="mdes2_8h" name="mdes2.h" local="yes" imported="no">mdes2.h</includes>
    <member kind="function">
      <type>Mdes2 *</type>
      <name>load_mdes2</name>
      <anchorfile>mdes2_8c.html</anchorfile>
      <anchor>a5240a35f7d58785ab2ff7b53a348154</anchor>
      <arglist>(char *file_name)</arglist>
    </member>
    <member kind="variable" static="yes">
      <type>static L_Alloc_Pool *</type>
      <name>Mdes2_pool</name>
      <anchorfile>mdes2_8c.html</anchorfile>
      <anchor>a90162fb3a168d7427d56e6251c4c185</anchor>
      <arglist></arglist>
    </member>
  </compound>
  <compound kind="file">
    <name>mdes2.h</name>
    <path>/mnt/hgfs/mkt/Desktop/openimpact/src/machine/Lmdes/</path>
    <filename>mdes2_8h</filename>
    <includes id="lmdes_8h" name="lmdes.h" local="yes" imported="no">lmdes.h</includes>
    <includes id="sm__mdes_8h" name="sm_mdes.h" local="yes" imported="no">sm_mdes.h</includes>
    <class kind="struct">Mdes2</class>
    <member kind="function">
      <type>Mdes2 *</type>
      <name>load_mdes2</name>
      <anchorfile>mdes2_8h.html</anchorfile>
      <anchor>a5240a35f7d58785ab2ff7b53a348154</anchor>
      <arglist>(char *file_name)</arglist>
    </member>
  </compound>
  <compound kind="file">
    <name>sm_mdes.c</name>
    <path>/mnt/hgfs/mkt/Desktop/openimpact/src/machine/Lmdes/</path>
    <filename>sm__mdes_8c</filename>
    <includes id="sm__mdes_8h" name="sm_mdes.h" local="yes" imported="no">sm_mdes.h</includes>
    <includes id="mdes2_8h" name="mdes2.h" local="yes" imported="no">mdes2.h</includes>
    <includes id="lmdes2_8h" name="lmdes2.h" local="yes" imported="no">lmdes2.h</includes>
    <member kind="define">
      <type>#define</type>
      <name>DEBUG_MDES</name>
      <anchorfile>sm__mdes_8c.html</anchorfile>
      <anchor>b96bf528414b7bb8a87c4af7bfa3e62e</anchor>
      <arglist></arglist>
    </member>
    <member kind="function">
      <type>int</type>
      <name>SM_add_ids_to_entries</name>
      <anchorfile>sm__mdes_8c.html</anchorfile>
      <anchor>4d1665aeb658c58e66a84443a500d2c2</anchor>
      <arglist>(MD_Section *section)</arglist>
    </member>
    <member kind="function">
      <type>void</type>
      <name>SM_build_resources</name>
      <anchorfile>sm__mdes_8c.html</anchorfile>
      <anchor>f3c40b1fee8b4c6daa1ad4682a1f830d</anchor>
      <arglist>(SM_Mdes *sm_mdes)</arglist>
    </member>
    <member kind="function">
      <type>void</type>
      <name>SM_build_units</name>
      <anchorfile>sm__mdes_8c.html</anchorfile>
      <anchor>57d959519b4d05185a589791c33d1dbc</anchor>
      <arglist>(SM_Mdes *sm_mdes)</arglist>
    </member>
    <member kind="function">
      <type>void</type>
      <name>SM_build_choices</name>
      <anchorfile>sm__mdes_8c.html</anchorfile>
      <anchor>b5ef49aff752297f821c77bc1cff26eb</anchor>
      <arglist>(SM_Mdes *sm_mdes)</arglist>
    </member>
    <member kind="function">
      <type>void</type>
      <name>SM_build_tables</name>
      <anchorfile>sm__mdes_8c.html</anchorfile>
      <anchor>1dfbd1008eb70627565211e3c1f7a8b3</anchor>
      <arglist>(SM_Mdes *sm_mdes)</arglist>
    </member>
    <member kind="function">
      <type>void</type>
      <name>SM_build_syllables</name>
      <anchorfile>sm__mdes_8c.html</anchorfile>
      <anchor>b5c9896da6b3cc808361174e581e5536</anchor>
      <arglist>(SM_Mdes *sm_mdes)</arglist>
    </member>
    <member kind="function">
      <type>void</type>
      <name>SM_build_ports</name>
      <anchorfile>sm__mdes_8c.html</anchorfile>
      <anchor>a9ddf4b929e903de405865e4b054fd23</anchor>
      <arglist>(SM_Mdes *sm_mdes)</arglist>
    </member>
    <member kind="function">
      <type>void</type>
      <name>SM_build_restrictions</name>
      <anchorfile>sm__mdes_8c.html</anchorfile>
      <anchor>cabe1e3171719ce7c801fd3ba8036980</anchor>
      <arglist>(SM_Mdes *sm_mdes)</arglist>
    </member>
    <member kind="function">
      <type>void</type>
      <name>SM_build_templates</name>
      <anchorfile>sm__mdes_8c.html</anchorfile>
      <anchor>49fcc91dc04c2f9a7810f62bc097599f</anchor>
      <arglist>(SM_Mdes *sm_mdes)</arglist>
    </member>
    <member kind="function">
      <type>void</type>
      <name>SM_build_tgroups</name>
      <anchorfile>sm__mdes_8c.html</anchorfile>
      <anchor>a1682008dc69596b9e2e654bb4128dec</anchor>
      <arglist>(SM_Mdes *sm_mdes)</arglist>
    </member>
    <member kind="function">
      <type>void</type>
      <name>SM_build_issues</name>
      <anchorfile>sm__mdes_8c.html</anchorfile>
      <anchor>ead8acb2036b72f891df6af0e3f0a1cc</anchor>
      <arglist>(SM_Mdes *sm_mdes)</arglist>
    </member>
    <member kind="function">
      <type>void</type>
      <name>SM_build_drules</name>
      <anchorfile>sm__mdes_8c.html</anchorfile>
      <anchor>1be740203a9e5470203daf8674c3fe2d</anchor>
      <arglist>(SM_Mdes *sm_mdes)</arglist>
    </member>
    <member kind="function">
      <type>void</type>
      <name>SM_build_prod_cons_latency</name>
      <anchorfile>sm__mdes_8c.html</anchorfile>
      <anchor>87a9e6302fda5c01d1a0484b39ab66fe</anchor>
      <arglist>(SM_Mdes *sm_mdes)</arglist>
    </member>
    <member kind="function">
      <type>void</type>
      <name>Print_prod_cons_latency</name>
      <anchorfile>sm__mdes_8c.html</anchorfile>
      <anchor>1b547ab6c50642646c1eaad613cc2e6f</anchor>
      <arglist>(SM_Mdes *sm_mdes)</arglist>
    </member>
    <member kind="function">
      <type>SM_Mdes *</type>
      <name>SM_build_mdes</name>
      <anchorfile>sm__mdes_8c.html</anchorfile>
      <anchor>960a9eb61c8904bfa51841f86ab97fb8</anchor>
      <arglist>(Mdes2 *mdes2)</arglist>
    </member>
    <member kind="variable" static="yes">
      <type>static L_Alloc_Pool *</type>
      <name>SM_Mdes_pool</name>
      <anchorfile>sm__mdes_8c.html</anchorfile>
      <anchor>24f5ce3df7b42ff05d92c8a588b5ec4d</anchor>
      <arglist></arglist>
    </member>
  </compound>
  <compound kind="file">
    <name>sm_mdes.h</name>
    <path>/mnt/hgfs/mkt/Desktop/openimpact/src/machine/Lmdes/</path>
    <filename>sm__mdes_8h</filename>
    <class kind="struct">SM_Resource</class>
    <class kind="struct">SM_Usage</class>
    <class kind="struct">SM_Option</class>
    <class kind="struct">SM_Choice</class>
    <class kind="struct">SM_Table</class>
    <class kind="struct">SM_Syllable</class>
    <class kind="struct">SM_Port</class>
    <class kind="struct">SM_Restriction</class>
    <class kind="struct">SM_Template</class>
    <class kind="struct">SM_TGroup</class>
    <class kind="struct">SM_Issue</class>
    <class kind="struct">SM_DRule</class>
    <class kind="struct">SM_Nop</class>
    <class kind="struct">SM_PCLat</class>
    <class kind="struct">SM_Mdes</class>
    <member kind="function">
      <type>SM_Mdes *</type>
      <name>SM_build_mdes</name>
      <anchorfile>sm__mdes_8h.html</anchorfile>
      <anchor>8d77c1d29c06062dc8d92084baff9a3d</anchor>
      <arglist>(struct Mdes2 *mdes2)</arglist>
    </member>
  </compound>
  <compound kind="struct">
    <name>Mdes2</name>
    <filename>structMdes2.html</filename>
    <member kind="variable">
      <type>char *</type>
      <name>file_name</name>
      <anchorfile>structMdes2.html</anchorfile>
      <anchor>2be3556449ed251743d1d8b41db2988b</anchor>
      <arglist></arglist>
    </member>
    <member kind="variable">
      <type>MD *</type>
      <name>md_mdes</name>
      <anchorfile>structMdes2.html</anchorfile>
      <anchor>92e6c49a5973752554959a2691003c1b</anchor>
      <arglist></arglist>
    </member>
    <member kind="variable">
      <type>SM_Mdes *</type>
      <name>sm_mdes</name>
      <anchorfile>structMdes2.html</anchorfile>
      <anchor>0940b9e06f781c72b4e686ce61b2cd6a</anchor>
      <arglist></arglist>
    </member>
    <member kind="variable">
      <type>Mdes *</type>
      <name>version1_mdes</name>
      <anchorfile>structMdes2.html</anchorfile>
      <anchor>ac135c0abfe0ae335016425afb06eb9c</anchor>
      <arglist></arglist>
    </member>
  </compound>
  <compound kind="struct">
    <name>mdes_alt_st</name>
    <filename>structmdes__alt__st.html</filename>
    <member kind="variable">
      <type>int</type>
      <name>id</name>
      <anchorfile>structmdes__alt__st.html</anchorfile>
      <anchor>55aae0953d0b100bb5b53e231fde86bd</anchor>
      <arglist></arglist>
    </member>
    <member kind="variable">
      <type>char *</type>
      <name>asm_name</name>
      <anchorfile>structmdes__alt__st.html</anchorfile>
      <anchor>158c1bb56f6965d6b93329e01647d171</anchor>
      <arglist></arglist>
    </member>
    <member kind="variable">
      <type>int</type>
      <name>alt_flags</name>
      <anchorfile>structmdes__alt__st.html</anchorfile>
      <anchor>1f343b194b6524edfc1bb329bfdb2c57</anchor>
      <arglist></arglist>
    </member>
    <member kind="variable">
      <type>mdes_operation_st *</type>
      <name>operation</name>
      <anchorfile>structmdes__alt__st.html</anchorfile>
      <anchor>d50b04c4e977de621f5202aa0fed864e</anchor>
      <arglist></arglist>
    </member>
    <member kind="variable">
      <type>Mdes_IO_Item *</type>
      <name>IO_item</name>
      <anchorfile>structmdes__alt__st.html</anchorfile>
      <anchor>79742a4ca813dce7ad2a421b2b56679a</anchor>
      <arglist></arglist>
    </member>
    <member kind="variable">
      <type>Mdes_ResList *</type>
      <name>reslist</name>
      <anchorfile>structmdes__alt__st.html</anchorfile>
      <anchor>bc503b74bb015c2e83977e65e30bc7b0</anchor>
      <arglist></arglist>
    </member>
    <member kind="variable">
      <type>Mdes_Latency *</type>
      <name>latency</name>
      <anchorfile>structmdes__alt__st.html</anchorfile>
      <anchor>885b2b98fa25f264c2c1bac7fdefb007</anchor>
      <arglist></arglist>
    </member>
    <member kind="variable">
      <type>SM_Table *</type>
      <name>table</name>
      <anchorfile>structmdes__alt__st.html</anchorfile>
      <anchor>079a0a9fe66124207ace9bd7b8a1c011</anchor>
      <arglist></arglist>
    </member>
  </compound>
  <compound kind="struct">
    <name>mdes_compatable_alt_st</name>
    <filename>structmdes__compatable__alt__st.html</filename>
    <member kind="variable">
      <type>Mdes_Alt *</type>
      <name>alt</name>
      <anchorfile>structmdes__compatable__alt__st.html</anchorfile>
      <anchor>5319d7027ebc3bff310086ea313080c7</anchor>
      <arglist></arglist>
    </member>
    <member kind="variable">
      <type>mdes_compatable_alt_st *</type>
      <name>next</name>
      <anchorfile>structmdes__compatable__alt__st.html</anchorfile>
      <anchor>14f426464c727abedf874de9c50776c1</anchor>
      <arglist></arglist>
    </member>
  </compound>
  <compound kind="struct">
    <name>Mdes_Info</name>
    <filename>structMdes__Info.html</filename>
    <member kind="variable">
      <type>int</type>
      <name>opcode</name>
      <anchorfile>structMdes__Info.html</anchorfile>
      <anchor>635a1f77a2a01516a8399444ceaf9fbe</anchor>
      <arglist></arglist>
    </member>
    <member kind="variable">
      <type>int</type>
      <name>num_compatable_alts</name>
      <anchorfile>structMdes__Info.html</anchorfile>
      <anchor>2d46091833cf2d0c810328217a9cd6da</anchor>
      <arglist></arglist>
    </member>
    <member kind="variable">
      <type>Mdes_Compatable_Alt *</type>
      <name>compatable_alts</name>
      <anchorfile>structMdes__Info.html</anchorfile>
      <anchor>ef1a9e5889654c6ee70e25f28c8c8f33</anchor>
      <arglist></arglist>
    </member>
  </compound>
  <compound kind="struct">
    <name>mdes_IO_item_st</name>
    <filename>structmdes__IO__item__st.html</filename>
    <member kind="variable">
      <type>int</type>
      <name>id</name>
      <anchorfile>structmdes__IO__item__st.html</anchorfile>
      <anchor>7aae29fa0ac0f1ccf9b1da5f510cfe35</anchor>
      <arglist></arglist>
    </member>
    <member kind="variable">
      <type>char *</type>
      <name>name</name>
      <anchorfile>structmdes__IO__item__st.html</anchorfile>
      <anchor>556ae9017c2be5ad184b004107ea5761</anchor>
      <arglist></arglist>
    </member>
    <member kind="variable">
      <type>Mdes_IO_Set **</type>
      <name>operand_type</name>
      <anchorfile>structmdes__IO__item__st.html</anchorfile>
      <anchor>f4ba7b4037f1fcfc0e390decda737443</anchor>
      <arglist></arglist>
    </member>
  </compound>
  <compound kind="struct">
    <name>mdes_IO_set_st</name>
    <filename>structmdes__IO__set__st.html</filename>
    <member kind="variable">
      <type>int</type>
      <name>id</name>
      <anchorfile>structmdes__IO__set__st.html</anchorfile>
      <anchor>827dc14f483a6213a640701306467ae5</anchor>
      <arglist></arglist>
    </member>
    <member kind="variable">
      <type>int</type>
      <name>external_id</name>
      <anchorfile>structmdes__IO__set__st.html</anchorfile>
      <anchor>fabb6d4d58c0daa390b1df63ac83c1d9</anchor>
      <arglist></arglist>
    </member>
    <member kind="variable">
      <type>char *</type>
      <name>name</name>
      <anchorfile>structmdes__IO__set__st.html</anchorfile>
      <anchor>204a1bad5b020f415a38cb383531dc08</anchor>
      <arglist></arglist>
    </member>
    <member kind="variable">
      <type>int *</type>
      <name>mask</name>
      <anchorfile>structmdes__IO__set__st.html</anchorfile>
      <anchor>9686f5e9e1c1c20ea435e4e1a654ad70</anchor>
      <arglist></arglist>
    </member>
  </compound>
  <compound kind="struct">
    <name>mdes_latency_st</name>
    <filename>structmdes__latency__st.html</filename>
    <member kind="variable">
      <type>int</type>
      <name>id</name>
      <anchorfile>structmdes__latency__st.html</anchorfile>
      <anchor>d60adbae92e6ce9d28d33d135210d853</anchor>
      <arglist></arglist>
    </member>
    <member kind="variable">
      <type>char *</type>
      <name>name</name>
      <anchorfile>structmdes__latency__st.html</anchorfile>
      <anchor>052697dc522f0aeb9ea1455f131df692</anchor>
      <arglist></arglist>
    </member>
    <member kind="variable">
      <type>int</type>
      <name>exception</name>
      <anchorfile>structmdes__latency__st.html</anchorfile>
      <anchor>d6bf7453d6437b4e2cf10ff5bfb9b684</anchor>
      <arglist></arglist>
    </member>
    <member kind="variable">
      <type>int *</type>
      <name>operand_latency</name>
      <anchorfile>structmdes__latency__st.html</anchorfile>
      <anchor>2001567fdd18018d2e7e7c3b88243113</anchor>
      <arglist></arglist>
    </member>
  </compound>
  <compound kind="struct">
    <name>mdes_operation_st</name>
    <filename>structmdes__operation__st.html</filename>
    <member kind="variable">
      <type>int</type>
      <name>id</name>
      <anchorfile>structmdes__operation__st.html</anchorfile>
      <anchor>6913741860f14acef9661e666ad812f0</anchor>
      <arglist></arglist>
    </member>
    <member kind="variable">
      <type>int</type>
      <name>opcode</name>
      <anchorfile>structmdes__operation__st.html</anchorfile>
      <anchor>225a9dcbe1226b4d16ec2b957653baee</anchor>
      <arglist></arglist>
    </member>
    <member kind="variable">
      <type>char *</type>
      <name>external_name</name>
      <anchorfile>structmdes__operation__st.html</anchorfile>
      <anchor>19afc4e397c1e5e4f7dfa743947997a7</anchor>
      <arglist></arglist>
    </member>
    <member kind="variable">
      <type>int</type>
      <name>num_alts</name>
      <anchorfile>structmdes__operation__st.html</anchorfile>
      <anchor>485fec080d34b838ddce7e949621fd8e</anchor>
      <arglist></arglist>
    </member>
    <member kind="variable">
      <type>Mdes_Alt *</type>
      <name>alt</name>
      <anchorfile>structmdes__operation__st.html</anchorfile>
      <anchor>7b463d984afbfb2ccf7641f7c0927f10</anchor>
      <arglist></arglist>
    </member>
    <member kind="variable">
      <type>int</type>
      <name>op_flags</name>
      <anchorfile>structmdes__operation__st.html</anchorfile>
      <anchor>b0f09c55e6decc77fca4e2b7d24d9980</anchor>
      <arglist></arglist>
    </member>
    <member kind="variable">
      <type>int</type>
      <name>heuristic_alt</name>
      <anchorfile>structmdes__operation__st.html</anchorfile>
      <anchor>1adff4df850ed9c1d2007764310ea4d5</anchor>
      <arglist></arglist>
    </member>
    <member kind="variable">
      <type>unsigned int</type>
      <name>syll_mask</name>
      <anchorfile>structmdes__operation__st.html</anchorfile>
      <anchor>bd067e6ea7dcce6259dcb4589d5b3afd</anchor>
      <arglist></arglist>
    </member>
    <member kind="variable">
      <type>unsigned int</type>
      <name>port_mask</name>
      <anchorfile>structmdes__operation__st.html</anchorfile>
      <anchor>1044fe9988672fbba3984f30d55e9287</anchor>
      <arglist></arglist>
    </member>
    <member kind="variable">
      <type>unsigned int</type>
      <name>num_slots</name>
      <anchorfile>structmdes__operation__st.html</anchorfile>
      <anchor>7bdff443703ea64f3bb9dda01ae65e52</anchor>
      <arglist></arglist>
    </member>
    <member kind="variable">
      <type>int</type>
      <name>nop</name>
      <anchorfile>structmdes__operation__st.html</anchorfile>
      <anchor>8becbd2f5cdbcefc1416755e284f575e</anchor>
      <arglist></arglist>
    </member>
    <member kind="variable">
      <type>int</type>
      <name>num_lat_class</name>
      <anchorfile>structmdes__operation__st.html</anchorfile>
      <anchor>1dd2ee1a06162463dc5bdc284659ee5f</anchor>
      <arglist></arglist>
    </member>
    <member kind="variable">
      <type>Set</type>
      <name>lat_class</name>
      <anchorfile>structmdes__operation__st.html</anchorfile>
      <anchor>b0114c696ce37ad2e8ec297b161cd836</anchor>
      <arglist></arglist>
    </member>
    <member kind="variable">
      <type>Mdes_Stats</type>
      <name>can_sched_prepass</name>
      <anchorfile>structmdes__operation__st.html</anchorfile>
      <anchor>c9735c79bc15e2c40c3695bb9eda233f</anchor>
      <arglist></arglist>
    </member>
    <member kind="variable">
      <type>Mdes_Stats</type>
      <name>sched_prepass</name>
      <anchorfile>structmdes__operation__st.html</anchorfile>
      <anchor>7ee0e9a372c01e375ac158eb9f24c0b5</anchor>
      <arglist></arglist>
    </member>
    <member kind="variable">
      <type>Mdes_Stats</type>
      <name>can_sched_postpass</name>
      <anchorfile>structmdes__operation__st.html</anchorfile>
      <anchor>64677ab60d5633447c7ac023148b3bf4</anchor>
      <arglist></arglist>
    </member>
    <member kind="variable">
      <type>Mdes_Stats</type>
      <name>sched_postpass</name>
      <anchorfile>structmdes__operation__st.html</anchorfile>
      <anchor>c5848d2ae18154a9a07985915fec5368</anchor>
      <arglist></arglist>
    </member>
  </compound>
  <compound kind="struct">
    <name>mdes_reslist_st</name>
    <filename>structmdes__reslist__st.html</filename>
    <member kind="variable">
      <type>int</type>
      <name>id</name>
      <anchorfile>structmdes__reslist__st.html</anchorfile>
      <anchor>572baafd712d4130b6c172625ca33af5</anchor>
      <arglist></arglist>
    </member>
    <member kind="variable">
      <type>char *</type>
      <name>name</name>
      <anchorfile>structmdes__reslist__st.html</anchorfile>
      <anchor>ec389a7080426fc5b6f4d0db8221299b</anchor>
      <arglist></arglist>
    </member>
    <member kind="variable">
      <type>int</type>
      <name>num_used</name>
      <anchorfile>structmdes__reslist__st.html</anchorfile>
      <anchor>3670ca7576ddda3a83becceabf550c44</anchor>
      <arglist></arglist>
    </member>
    <member kind="variable">
      <type>Mdes_Rused *</type>
      <name>used</name>
      <anchorfile>structmdes__reslist__st.html</anchorfile>
      <anchor>cb66cfe3b8b5e935c283068e0c3471db</anchor>
      <arglist></arglist>
    </member>
    <member kind="variable">
      <type>int</type>
      <name>num_slot_options</name>
      <anchorfile>structmdes__reslist__st.html</anchorfile>
      <anchor>22eaf9f6bb6bcd1e4b04b06c8ee97473</anchor>
      <arglist></arglist>
    </member>
    <member kind="variable">
      <type>int *</type>
      <name>slot_options</name>
      <anchorfile>structmdes__reslist__st.html</anchorfile>
      <anchor>94a471e0074383d2e9c63227e65aed69</anchor>
      <arglist></arglist>
    </member>
    <member kind="variable">
      <type>int</type>
      <name>num_RU_entries_required</name>
      <anchorfile>structmdes__reslist__st.html</anchorfile>
      <anchor>1634c9a64a39b575dcf28eeb4b50ebe0</anchor>
      <arglist></arglist>
    </member>
  </compound>
  <compound kind="struct">
    <name>mdes_resource_st</name>
    <filename>structmdes__resource__st.html</filename>
    <member kind="variable">
      <type>int</type>
      <name>id</name>
      <anchorfile>structmdes__resource__st.html</anchorfile>
      <anchor>42ca120f2786441b8b222381866e31d3</anchor>
      <arglist></arglist>
    </member>
    <member kind="variable">
      <type>char *</type>
      <name>name</name>
      <anchorfile>structmdes__resource__st.html</anchorfile>
      <anchor>c10bb6f8306bf22abe165ba9f07139c7</anchor>
      <arglist></arglist>
    </member>
  </compound>
  <compound kind="struct">
    <name>mdes_rmask_st</name>
    <filename>structmdes__rmask__st.html</filename>
    <member kind="variable">
      <type>int *</type>
      <name>uncond</name>
      <anchorfile>structmdes__rmask__st.html</anchorfile>
      <anchor>70ba267e31085d18f5d3edda1186d3f3</anchor>
      <arglist></arglist>
    </member>
    <member kind="variable">
      <type>int *</type>
      <name>pred</name>
      <anchorfile>structmdes__rmask__st.html</anchorfile>
      <anchor>341998ee97a9ba2f7b60dc8036d9ddd3</anchor>
      <arglist></arglist>
    </member>
  </compound>
  <compound kind="struct">
    <name>mdes_Rused</name>
    <filename>structmdes__Rused.html</filename>
    <member kind="variable">
      <type>int</type>
      <name>flags</name>
      <anchorfile>structmdes__Rused.html</anchorfile>
      <anchor>a47bf2b0a2f43df2f9faffddba1e907f</anchor>
      <arglist></arglist>
    </member>
    <member kind="variable">
      <type>int</type>
      <name>start_usage</name>
      <anchorfile>structmdes__Rused.html</anchorfile>
      <anchor>448cdf5c17b6fb5bc6bd285af0b8bd28</anchor>
      <arglist></arglist>
    </member>
    <member kind="variable">
      <type>int</type>
      <name>end_usage</name>
      <anchorfile>structmdes__Rused.html</anchorfile>
      <anchor>27e100be1ef1ebee7a56b770f342d011</anchor>
      <arglist></arglist>
    </member>
    <member kind="variable">
      <type>int</type>
      <name>num_options</name>
      <anchorfile>structmdes__Rused.html</anchorfile>
      <anchor>8f67008d709d238be2749f3d3258a902</anchor>
      <arglist></arglist>
    </member>
    <member kind="variable">
      <type>Mdes_Rmask *</type>
      <name>option</name>
      <anchorfile>structmdes__Rused.html</anchorfile>
      <anchor>8dc184ff98659df860bfa13e4cefa36d</anchor>
      <arglist></arglist>
    </member>
  </compound>
  <compound kind="struct">
    <name>mdes_st</name>
    <filename>structmdes__st.html</filename>
    <member kind="variable">
      <type>char *</type>
      <name>file_name</name>
      <anchorfile>structmdes__st.html</anchorfile>
      <anchor>ee752ea650881fc151fdc2f5cf212168</anchor>
      <arglist></arglist>
    </member>
    <member kind="variable">
      <type>int</type>
      <name>processor_model</name>
      <anchorfile>structmdes__st.html</anchorfile>
      <anchor>6b9ab0dc517ef1c37a6ffa34e417cb8f</anchor>
      <arglist></arglist>
    </member>
    <member kind="variable">
      <type>int</type>
      <name>number</name>
      <anchorfile>structmdes__st.html</anchorfile>
      <anchor>c34628a21ebb2f399adb56922b09d17c</anchor>
      <arglist>[5]</arglist>
    </member>
    <member kind="variable">
      <type>int</type>
      <name>offset</name>
      <anchorfile>structmdes__st.html</anchorfile>
      <anchor>35702b33712810e42a62051cd83a0649</anchor>
      <arglist>[5]</arglist>
    </member>
    <member kind="variable">
      <type>char *</type>
      <name>name</name>
      <anchorfile>structmdes__st.html</anchorfile>
      <anchor>7e4ee12b576ec4bbf87cd5b1766f6058</anchor>
      <arglist>[5]</arglist>
    </member>
    <member kind="variable">
      <type>int</type>
      <name>operand_count</name>
      <anchorfile>structmdes__st.html</anchorfile>
      <anchor>94d3ba15b112518d03a1b35489d1a558</anchor>
      <arglist></arglist>
    </member>
    <member kind="variable">
      <type>int</type>
      <name>latency_count</name>
      <anchorfile>structmdes__st.html</anchorfile>
      <anchor>af06863442c6f95040820a3cc4f93bbc</anchor>
      <arglist></arglist>
    </member>
    <member kind="variable">
      <type>int</type>
      <name>num_slots</name>
      <anchorfile>structmdes__st.html</anchorfile>
      <anchor>c586c61ba65b9cf1f47c3958a8733221</anchor>
      <arglist></arglist>
    </member>
    <member kind="variable">
      <type>int</type>
      <name>max_slot</name>
      <anchorfile>structmdes__st.html</anchorfile>
      <anchor>765cd60f19962a53e13c5cec60caeb34</anchor>
      <arglist></arglist>
    </member>
    <member kind="variable">
      <type>int</type>
      <name>IOmask_width</name>
      <anchorfile>structmdes__st.html</anchorfile>
      <anchor>29fc136d2dc0f414380fc59b235ee727</anchor>
      <arglist></arglist>
    </member>
    <member kind="variable">
      <type>int</type>
      <name>num_reg_files</name>
      <anchorfile>structmdes__st.html</anchorfile>
      <anchor>95981384eaa25ca98f34024f6b5205f2</anchor>
      <arglist></arglist>
    </member>
    <member kind="variable">
      <type>int</type>
      <name>max_IO_set_id</name>
      <anchorfile>structmdes__st.html</anchorfile>
      <anchor>75de6777cb4235aa6785356f9dded25c</anchor>
      <arglist></arglist>
    </member>
    <member kind="variable">
      <type>int</type>
      <name>null_external_id</name>
      <anchorfile>structmdes__st.html</anchorfile>
      <anchor>0d51ee331fc7f4a2105fbcc399170851</anchor>
      <arglist></arglist>
    </member>
    <member kind="variable">
      <type>Mdes_IO_Set **</type>
      <name>IO_set_table</name>
      <anchorfile>structmdes__st.html</anchorfile>
      <anchor>03c2f265d0bb249445f6b63e2c7f4bfc</anchor>
      <arglist></arglist>
    </member>
    <member kind="variable">
      <type>int</type>
      <name>num_IO_sets</name>
      <anchorfile>structmdes__st.html</anchorfile>
      <anchor>3b3f4986cd5a5e1c15bc56448a05ed60</anchor>
      <arglist></arglist>
    </member>
    <member kind="variable">
      <type>Mdes_IO_Set *</type>
      <name>IO_set</name>
      <anchorfile>structmdes__st.html</anchorfile>
      <anchor>038cc6be3823730b778c09e48376feb0</anchor>
      <arglist></arglist>
    </member>
    <member kind="variable">
      <type>int</type>
      <name>num_IO_items</name>
      <anchorfile>structmdes__st.html</anchorfile>
      <anchor>ae62c2738222d4114a58f074e9b7f5ba</anchor>
      <arglist></arglist>
    </member>
    <member kind="variable">
      <type>Mdes_IO_Item *</type>
      <name>IO_item</name>
      <anchorfile>structmdes__st.html</anchorfile>
      <anchor>681219453a7b2032a138b331ba4fb6ae</anchor>
      <arglist></arglist>
    </member>
    <member kind="variable">
      <type>int</type>
      <name>num_resources</name>
      <anchorfile>structmdes__st.html</anchorfile>
      <anchor>cf06ff3643ff3e4d0d0a4e8ba8422149</anchor>
      <arglist></arglist>
    </member>
    <member kind="variable">
      <type>Mdes_Resource *</type>
      <name>resource</name>
      <anchorfile>structmdes__st.html</anchorfile>
      <anchor>75ce60f24fe1161514001078b7098146</anchor>
      <arglist></arglist>
    </member>
    <member kind="variable">
      <type>int</type>
      <name>num_reslists</name>
      <anchorfile>structmdes__st.html</anchorfile>
      <anchor>1d5ecc9d3fa408ec56becadbbd3be2aa</anchor>
      <arglist></arglist>
    </member>
    <member kind="variable">
      <type>int</type>
      <name>Rmask_width</name>
      <anchorfile>structmdes__st.html</anchorfile>
      <anchor>8828807b63450fd197027a97962c1213</anchor>
      <arglist></arglist>
    </member>
    <member kind="variable">
      <type>Mdes_ResList *</type>
      <name>reslist</name>
      <anchorfile>structmdes__st.html</anchorfile>
      <anchor>4a7f6816b728d2a902e64d1f18a183e6</anchor>
      <arglist></arglist>
    </member>
    <member kind="variable">
      <type>int</type>
      <name>num_latencies</name>
      <anchorfile>structmdes__st.html</anchorfile>
      <anchor>aa337a8afe70c1f724e176bb015c9c28</anchor>
      <arglist></arglist>
    </member>
    <member kind="variable">
      <type>Mdes_Latency *</type>
      <name>latency</name>
      <anchorfile>structmdes__st.html</anchorfile>
      <anchor>682d6c434bc4536d925d68070be151d1</anchor>
      <arglist></arglist>
    </member>
    <member kind="variable">
      <type>int</type>
      <name>num_operations</name>
      <anchorfile>structmdes__st.html</anchorfile>
      <anchor>0c7d66330250070b9a723afcae436545</anchor>
      <arglist></arglist>
    </member>
    <member kind="variable">
      <type>Mdes_Operation *</type>
      <name>operation</name>
      <anchorfile>structmdes__st.html</anchorfile>
      <anchor>985578aacd1a7fc65ac03a323bc5ff84</anchor>
      <arglist></arglist>
    </member>
    <member kind="variable">
      <type>int</type>
      <name>max_opcode</name>
      <anchorfile>structmdes__st.html</anchorfile>
      <anchor>f49d24269206355ebdc64103af0ba61f</anchor>
      <arglist></arglist>
    </member>
    <member kind="variable">
      <type>Mdes_Operation **</type>
      <name>op_table</name>
      <anchorfile>structmdes__st.html</anchorfile>
      <anchor>2c96f25951773052039f2a6835b397f8</anchor>
      <arglist></arglist>
    </member>
    <member kind="variable">
      <type>Mdes_IO_Set **</type>
      <name>operand_type_buf</name>
      <anchorfile>structmdes__st.html</anchorfile>
      <anchor>075df46e855d98a350f0e8aa6ae3490e</anchor>
      <arglist></arglist>
    </member>
    <member kind="variable">
      <type>int *</type>
      <name>index_type</name>
      <anchorfile>structmdes__st.html</anchorfile>
      <anchor>e86ef055a4b821949430f28aeb444c19</anchor>
      <arglist></arglist>
    </member>
    <member kind="variable">
      <type>int *</type>
      <name>index_number</name>
      <anchorfile>structmdes__st.html</anchorfile>
      <anchor>9fdc97a8bdb375a2d30c70364d25843e</anchor>
      <arglist></arglist>
    </member>
    <member kind="variable">
      <type>Mdes2 *</type>
      <name>mdes2</name>
      <anchorfile>structmdes__st.html</anchorfile>
      <anchor>621c27e7a2b2759610921b298b6d24ce</anchor>
      <arglist></arglist>
    </member>
    <member kind="variable">
      <type>int</type>
      <name>check_resources_for_only_one_alt</name>
      <anchorfile>structmdes__st.html</anchorfile>
      <anchor>abdf4dd8937ff6286441bd8b50726e63</anchor>
      <arglist></arglist>
    </member>
  </compound>
  <compound kind="struct">
    <name>Mdes_Stats</name>
    <filename>structMdes__Stats.html</filename>
    <member kind="variable">
      <type>int</type>
      <name>num_oper_checks</name>
      <anchorfile>structMdes__Stats.html</anchorfile>
      <anchor>3a0209c939f35e700729639cca5c4930</anchor>
      <arglist></arglist>
    </member>
    <member kind="variable">
      <type>int</type>
      <name>num_oper_checks_failed</name>
      <anchorfile>structMdes__Stats.html</anchorfile>
      <anchor>ed64192023276bc96dbad048f6553aeb</anchor>
      <arglist></arglist>
    </member>
    <member kind="variable">
      <type>int</type>
      <name>num_table_checks</name>
      <anchorfile>structMdes__Stats.html</anchorfile>
      <anchor>0363a6381aaa849c5759bc3e03c49d54</anchor>
      <arglist></arglist>
    </member>
    <member kind="variable">
      <type>int</type>
      <name>num_table_checks_failed</name>
      <anchorfile>structMdes__Stats.html</anchorfile>
      <anchor>9f75994a7a2e0d7424f380624e13425d</anchor>
      <arglist></arglist>
    </member>
    <member kind="variable">
      <type>int</type>
      <name>num_option_checks</name>
      <anchorfile>structMdes__Stats.html</anchorfile>
      <anchor>a98ef74cf03c8dc240200a1aecc39866</anchor>
      <arglist></arglist>
    </member>
    <member kind="variable">
      <type>int</type>
      <name>num_option_checks_failed</name>
      <anchorfile>structMdes__Stats.html</anchorfile>
      <anchor>a5970edae9411728c070573eafff519c</anchor>
      <arglist></arglist>
    </member>
    <member kind="variable">
      <type>int</type>
      <name>num_usage_checks</name>
      <anchorfile>structMdes__Stats.html</anchorfile>
      <anchor>c1953e8ba6a6e02b8a45e5002aaed28f</anchor>
      <arglist></arglist>
    </member>
    <member kind="variable">
      <type>int</type>
      <name>num_usage_checks_failed</name>
      <anchorfile>structMdes__Stats.html</anchorfile>
      <anchor>94c6d4d26147f0973a10d6df96a8f44c</anchor>
      <arglist></arglist>
    </member>
    <member kind="variable">
      <type>int</type>
      <name>num_slot_checks</name>
      <anchorfile>structMdes__Stats.html</anchorfile>
      <anchor>7ff97edda08e7f5d28fd5a5326ce4148</anchor>
      <arglist></arglist>
    </member>
    <member kind="variable">
      <type>int</type>
      <name>num_slot_checks_failed</name>
      <anchorfile>structMdes__Stats.html</anchorfile>
      <anchor>c1a823436a55ee6be369aa154404beb0</anchor>
      <arglist></arglist>
    </member>
    <member kind="variable">
      <type>INT_Symbol_Table *</type>
      <name>first_choice_dist</name>
      <anchorfile>structMdes__Stats.html</anchorfile>
      <anchor>ade44a96c63dd3abdb9b4a6c71022a01</anchor>
      <arglist></arglist>
    </member>
    <member kind="variable">
      <type>INT_Symbol_Table *</type>
      <name>num_choice_dist</name>
      <anchorfile>structMdes__Stats.html</anchorfile>
      <anchor>b87e3703793b265a4c0965a5ceb2c26e</anchor>
      <arglist></arglist>
    </member>
    <member kind="variable">
      <type>INT_Symbol_Table *</type>
      <name>succeed_option_check_dist</name>
      <anchorfile>structMdes__Stats.html</anchorfile>
      <anchor>7bb1d0ee4d650ba2e782aa9e7a043688</anchor>
      <arglist></arglist>
    </member>
    <member kind="variable">
      <type>INT_Symbol_Table *</type>
      <name>fail_option_check_dist</name>
      <anchorfile>structMdes__Stats.html</anchorfile>
      <anchor>4b5dedb4736c52674547ae68a0c59f78</anchor>
      <arglist></arglist>
    </member>
    <member kind="variable">
      <type>INT_Symbol_Table *</type>
      <name>succeed_usage_check_dist</name>
      <anchorfile>structMdes__Stats.html</anchorfile>
      <anchor>b1a9acc2ff4c8e6658403bb83ad47155</anchor>
      <arglist></arglist>
    </member>
    <member kind="variable">
      <type>INT_Symbol_Table *</type>
      <name>fail_usage_check_dist</name>
      <anchorfile>structMdes__Stats.html</anchorfile>
      <anchor>7880416ff9053ece400bbe7b5ab8015c</anchor>
      <arglist></arglist>
    </member>
  </compound>
  <compound kind="struct">
    <name>SM_Choice</name>
    <filename>structSM__Choice.html</filename>
    <member kind="variable">
      <type>SM_Option *</type>
      <name>first_option</name>
      <anchorfile>structSM__Choice.html</anchorfile>
      <anchor>b6174dcd65fd2ea558905ea3739857c1</anchor>
      <arglist></arglist>
    </member>
    <member kind="variable">
      <type>SM_Option *</type>
      <name>last_option</name>
      <anchorfile>structSM__Choice.html</anchorfile>
      <anchor>2d2a2f01c886e4f2bcc3907750fba403</anchor>
      <arglist></arglist>
    </member>
  </compound>
  <compound kind="struct">
    <name>SM_DRule</name>
    <filename>structSM__DRule.html</filename>
    <member kind="variable">
      <type>char *</type>
      <name>name</name>
      <anchorfile>structSM__DRule.html</anchorfile>
      <anchor>c82ebfbdebf5a5531854fb68eed62ff6</anchor>
      <arglist></arglist>
    </member>
    <member kind="variable">
      <type>int *</type>
      <name>slot</name>
      <anchorfile>structSM__DRule.html</anchorfile>
      <anchor>9f331dda0f279c0d520ea1ea6b602972</anchor>
      <arglist></arglist>
    </member>
    <member kind="variable">
      <type>unsigned int</type>
      <name>num_slots</name>
      <anchorfile>structSM__DRule.html</anchorfile>
      <anchor>5f4bb499caadc452e2205098ffee2b79</anchor>
      <arglist></arglist>
    </member>
    <member kind="variable">
      <type>unsigned int</type>
      <name>port_mask</name>
      <anchorfile>structSM__DRule.html</anchorfile>
      <anchor>2c640353a12c5ba43d27c99022373adc</anchor>
      <arglist></arglist>
    </member>
    <member kind="variable">
      <type>unsigned int</type>
      <name>syll_mask</name>
      <anchorfile>structSM__DRule.html</anchorfile>
      <anchor>199d6d02cbf2ddfadc2d5d3139498d4e</anchor>
      <arglist></arglist>
    </member>
    <member kind="variable">
      <type>unsigned int *</type>
      <name>rsrc</name>
      <anchorfile>structSM__DRule.html</anchorfile>
      <anchor>c8588f000d9fcc8548e009416e92df37</anchor>
      <arglist></arglist>
    </member>
    <member kind="variable">
      <type>unsigned int</type>
      <name>num_rsrcs</name>
      <anchorfile>structSM__DRule.html</anchorfile>
      <anchor>8f33c1eaf0654d7513e158f64f62c219</anchor>
      <arglist></arglist>
    </member>
    <member kind="variable">
      <type>unsigned int</type>
      <name>templates</name>
      <anchorfile>structSM__DRule.html</anchorfile>
      <anchor>24a4fcf1c568d0436af9127745fbb4ea</anchor>
      <arglist></arglist>
    </member>
  </compound>
  <compound kind="struct">
    <name>SM_Issue</name>
    <filename>structSM__Issue.html</filename>
    <member kind="variable">
      <type>char *</type>
      <name>name</name>
      <anchorfile>structSM__Issue.html</anchorfile>
      <anchor>c0d2e5345d3a847fd24578590c170ed8</anchor>
      <arglist></arglist>
    </member>
    <member kind="variable">
      <type>unsigned int</type>
      <name>num_templates</name>
      <anchorfile>structSM__Issue.html</anchorfile>
      <anchor>0aebda34aa38889b6d0a3486c1cc7485</anchor>
      <arglist></arglist>
    </member>
    <member kind="variable">
      <type>unsigned int *</type>
      <name>templates</name>
      <anchorfile>structSM__Issue.html</anchorfile>
      <anchor>a7c7da141e6827cba217afcec7b57c68</anchor>
      <arglist></arglist>
    </member>
  </compound>
  <compound kind="struct">
    <name>SM_Mdes</name>
    <filename>structSM__Mdes.html</filename>
    <member kind="variable">
      <type>Mdes2 *</type>
      <name>mdes2</name>
      <anchorfile>structSM__Mdes.html</anchorfile>
      <anchor>90d730725db5d7f393f02563a6f1408b</anchor>
      <arglist></arglist>
    </member>
    <member kind="variable">
      <type>MD *</type>
      <name>md_mdes</name>
      <anchorfile>structSM__Mdes.html</anchorfile>
      <anchor>bdc3de19a29cf4150d1bd1e981855ead</anchor>
      <arglist></arglist>
    </member>
    <member kind="variable">
      <type>SM_Resource *</type>
      <name>resource</name>
      <anchorfile>structSM__Mdes.html</anchorfile>
      <anchor>8992583ede08e603ba43c64da275e4a2</anchor>
      <arglist></arglist>
    </member>
    <member kind="variable">
      <type>unsigned int</type>
      <name>num_resources</name>
      <anchorfile>structSM__Mdes.html</anchorfile>
      <anchor>c17dd34b37c13e5dc5a75021a2fa2dd4</anchor>
      <arglist></arglist>
    </member>
    <member kind="variable">
      <type>unsigned int</type>
      <name>map_width</name>
      <anchorfile>structSM__Mdes.html</anchorfile>
      <anchor>f23116b6bcaa9e2d7da7e89ac3db382a</anchor>
      <arglist></arglist>
    </member>
    <member kind="variable">
      <type>unsigned int</type>
      <name>time_shift</name>
      <anchorfile>structSM__Mdes.html</anchorfile>
      <anchor>0ed984bb41422f2e507773d1cf3f50f7</anchor>
      <arglist></arglist>
    </member>
    <member kind="variable">
      <type>unsigned int</type>
      <name>max_num_choices</name>
      <anchorfile>structSM__Mdes.html</anchorfile>
      <anchor>47cd4a0fd13340c3e39d3967015b1c85</anchor>
      <arglist></arglist>
    </member>
    <member kind="variable">
      <type>SM_Option *</type>
      <name>unit_array</name>
      <anchorfile>structSM__Mdes.html</anchorfile>
      <anchor>2cfe188c493008cff3a6efda8f77aef0</anchor>
      <arglist></arglist>
    </member>
    <member kind="variable">
      <type>unsigned int</type>
      <name>unit_array_size</name>
      <anchorfile>structSM__Mdes.html</anchorfile>
      <anchor>d3624bdbb415ea33aeb26718a615c112</anchor>
      <arglist></arglist>
    </member>
    <member kind="variable">
      <type>SM_Choice *</type>
      <name>choice_array</name>
      <anchorfile>structSM__Mdes.html</anchorfile>
      <anchor>18dbcf969d11f6cbe6872aca5cc53219</anchor>
      <arglist></arglist>
    </member>
    <member kind="variable">
      <type>unsigned int</type>
      <name>choice_array_size</name>
      <anchorfile>structSM__Mdes.html</anchorfile>
      <anchor>28c7e18ccf5469a4832477c0407329ee</anchor>
      <arglist></arglist>
    </member>
    <member kind="variable">
      <type>SM_Table *</type>
      <name>table_array</name>
      <anchorfile>structSM__Mdes.html</anchorfile>
      <anchor>29e3b4976fbe1e1888bec1f9bccd53c6</anchor>
      <arglist></arglist>
    </member>
    <member kind="variable">
      <type>unsigned int</type>
      <name>table_array_size</name>
      <anchorfile>structSM__Mdes.html</anchorfile>
      <anchor>9706ef21d2002ace0c5a3e70add5d122</anchor>
      <arglist></arglist>
    </member>
    <member kind="variable">
      <type>SM_Syllable *</type>
      <name>syllable_array</name>
      <anchorfile>structSM__Mdes.html</anchorfile>
      <anchor>35f9126db356ee337503587658e9b479</anchor>
      <arglist></arglist>
    </member>
    <member kind="variable">
      <type>unsigned int</type>
      <name>num_syllables</name>
      <anchorfile>structSM__Mdes.html</anchorfile>
      <anchor>bcbacea4368c16bd105a91694e7df0ff</anchor>
      <arglist></arglist>
    </member>
    <member kind="variable">
      <type>unsigned int</type>
      <name>top_syllable</name>
      <anchorfile>structSM__Mdes.html</anchorfile>
      <anchor>4df216cc89dc85f2097496da02a1e322</anchor>
      <arglist></arglist>
    </member>
    <member kind="variable">
      <type>SM_Port *</type>
      <name>port_array</name>
      <anchorfile>structSM__Mdes.html</anchorfile>
      <anchor>492861f2ef57a43b7324af4e5381d605</anchor>
      <arglist></arglist>
    </member>
    <member kind="variable">
      <type>unsigned int</type>
      <name>num_ports</name>
      <anchorfile>structSM__Mdes.html</anchorfile>
      <anchor>b15cc3a6613cf1269e73065c12406d14</anchor>
      <arglist></arglist>
    </member>
    <member kind="variable">
      <type>SM_Restriction *</type>
      <name>restrict_array</name>
      <anchorfile>structSM__Mdes.html</anchorfile>
      <anchor>1450bbb18bdafc7038305d5592527d81</anchor>
      <arglist></arglist>
    </member>
    <member kind="variable">
      <type>unsigned int</type>
      <name>num_restricts</name>
      <anchorfile>structSM__Mdes.html</anchorfile>
      <anchor>2a6555fa048e325387ee4f6c64d3234d</anchor>
      <arglist></arglist>
    </member>
    <member kind="variable">
      <type>SM_Template *</type>
      <name>template_array</name>
      <anchorfile>structSM__Mdes.html</anchorfile>
      <anchor>9ff2c79b9d7f0675a4711d4b0e6d721a</anchor>
      <arglist></arglist>
    </member>
    <member kind="variable">
      <type>unsigned int</type>
      <name>num_templates</name>
      <anchorfile>structSM__Mdes.html</anchorfile>
      <anchor>ff09d25cd2535f05de62452a5ce799cd</anchor>
      <arglist></arglist>
    </member>
    <member kind="variable">
      <type>unsigned int</type>
      <name>template_shift_var</name>
      <anchorfile>structSM__Mdes.html</anchorfile>
      <anchor>b7721b0722e0c0865c713ade86511a76</anchor>
      <arglist></arglist>
    </member>
    <member kind="variable">
      <type>unsigned int</type>
      <name>slots_per_template</name>
      <anchorfile>structSM__Mdes.html</anchorfile>
      <anchor>87a430ba0793a8df58ba1e9b7716c60f</anchor>
      <arglist></arglist>
    </member>
    <member kind="variable">
      <type>SM_TGroup *</type>
      <name>tgroup_array</name>
      <anchorfile>structSM__Mdes.html</anchorfile>
      <anchor>490250b9c31286a84b0529728bf318b7</anchor>
      <arglist></arglist>
    </member>
    <member kind="variable">
      <type>unsigned int</type>
      <name>num_tgroups</name>
      <anchorfile>structSM__Mdes.html</anchorfile>
      <anchor>78aa6465b68f4de612344116e9e3d4b5</anchor>
      <arglist></arglist>
    </member>
    <member kind="variable">
      <type>SM_Issue *</type>
      <name>issue_array</name>
      <anchorfile>structSM__Mdes.html</anchorfile>
      <anchor>99249ce21439b133afaa8645279974d0</anchor>
      <arglist></arglist>
    </member>
    <member kind="variable">
      <type>unsigned int</type>
      <name>num_issues</name>
      <anchorfile>structSM__Mdes.html</anchorfile>
      <anchor>a434dbe015bb202d1e247ef4dca2233c</anchor>
      <arglist></arglist>
    </member>
    <member kind="variable">
      <type>unsigned int</type>
      <name>max_template_per_issue</name>
      <anchorfile>structSM__Mdes.html</anchorfile>
      <anchor>1d6622cd0366e1f8aa205d170c587427</anchor>
      <arglist></arglist>
    </member>
    <member kind="variable">
      <type>SM_DRule *</type>
      <name>drule_array</name>
      <anchorfile>structSM__Mdes.html</anchorfile>
      <anchor>be2797e87ecc6e24efb2e416a30e721b</anchor>
      <arglist></arglist>
    </member>
    <member kind="variable">
      <type>unsigned int</type>
      <name>num_drules</name>
      <anchorfile>structSM__Mdes.html</anchorfile>
      <anchor>7aa4b1d1b9393d832a2511ffafb8c3f2</anchor>
      <arglist></arglist>
    </member>
    <member kind="variable">
      <type>int *</type>
      <name>nop_array</name>
      <anchorfile>structSM__Mdes.html</anchorfile>
      <anchor>95fc72f8c7bc5d1e9ec622cd806fac66</anchor>
      <arglist></arglist>
    </member>
    <member kind="variable">
      <type>int</type>
      <name>max_nop_index</name>
      <anchorfile>structSM__Mdes.html</anchorfile>
      <anchor>0f747c0ec8d233dedd6f336c3e03d875</anchor>
      <arglist></arglist>
    </member>
    <member kind="variable">
      <type>SM_PCLat *</type>
      <name>pclat_array</name>
      <anchorfile>structSM__Mdes.html</anchorfile>
      <anchor>1d910e561a438cee14edfd128d9a5ff4</anchor>
      <arglist></arglist>
    </member>
    <member kind="variable">
      <type>unsigned int</type>
      <name>pclat_array_size</name>
      <anchorfile>structSM__Mdes.html</anchorfile>
      <anchor>0ef2fe454d0828aba573b379067fe77d</anchor>
      <arglist></arglist>
    </member>
  </compound>
  <compound kind="struct">
    <name>SM_Nop</name>
    <filename>structSM__Nop.html</filename>
    <member kind="variable">
      <type>int</type>
      <name>opcode</name>
      <anchorfile>structSM__Nop.html</anchorfile>
      <anchor>7250f151a48e6fb190ae6bd8aa1d9bd9</anchor>
      <arglist></arglist>
    </member>
    <member kind="variable">
      <type>int</type>
      <name>priority</name>
      <anchorfile>structSM__Nop.html</anchorfile>
      <anchor>71974a282b685814e878416643a259db</anchor>
      <arglist></arglist>
    </member>
    <member kind="variable">
      <type>SM_Nop *</type>
      <name>prev_nop</name>
      <anchorfile>structSM__Nop.html</anchorfile>
      <anchor>654d71c050b6840e78a1b6e817014ac8</anchor>
      <arglist></arglist>
    </member>
    <member kind="variable">
      <type>SM_Nop *</type>
      <name>next_nop</name>
      <anchorfile>structSM__Nop.html</anchorfile>
      <anchor>6a6ee187be3ec08b6255f1d28a178ef6</anchor>
      <arglist></arglist>
    </member>
  </compound>
  <compound kind="struct">
    <name>SM_Option</name>
    <filename>structSM__Option.html</filename>
    <member kind="variable">
      <type>SM_Usage *</type>
      <name>first_usage</name>
      <anchorfile>structSM__Option.html</anchorfile>
      <anchor>98eecda72c401adaa440e747dfc710e4</anchor>
      <arglist></arglist>
    </member>
    <member kind="variable">
      <type>SM_Usage *</type>
      <name>last_usage</name>
      <anchorfile>structSM__Option.html</anchorfile>
      <anchor>49b8120ea400a64882c8cd2b4c974fa3</anchor>
      <arglist></arglist>
    </member>
  </compound>
  <compound kind="struct">
    <name>SM_PCLat</name>
    <filename>structSM__PCLat.html</filename>
    <member kind="variable">
      <type>char *</type>
      <name>name</name>
      <anchorfile>structSM__PCLat.html</anchorfile>
      <anchor>297c14591b529d2f4847a54769e728d4</anchor>
      <arglist></arglist>
    </member>
    <member kind="variable">
      <type>Set</type>
      <name>plat</name>
      <anchorfile>structSM__PCLat.html</anchorfile>
      <anchor>ab5660f3eca812cefd5f40a53b9ff32c</anchor>
      <arglist></arglist>
    </member>
    <member kind="variable">
      <type>int *</type>
      <name>pdest_latency_penalty</name>
      <anchorfile>structSM__PCLat.html</anchorfile>
      <anchor>6a335a4b03935829aff702567d4c22ca</anchor>
      <arglist></arglist>
    </member>
    <member kind="variable">
      <type>Set</type>
      <name>clat</name>
      <anchorfile>structSM__PCLat.html</anchorfile>
      <anchor>7cc8f498354de10029a2ebcfd45fa848</anchor>
      <arglist></arglist>
    </member>
    <member kind="variable">
      <type>int *</type>
      <name>csrc_latency_penalty</name>
      <anchorfile>structSM__PCLat.html</anchorfile>
      <anchor>5d4ca8faea74f146540cf39c6a1e2130</anchor>
      <arglist></arglist>
    </member>
  </compound>
  <compound kind="struct">
    <name>SM_Port</name>
    <filename>structSM__Port.html</filename>
    <member kind="variable">
      <type>char *</type>
      <name>name</name>
      <anchorfile>structSM__Port.html</anchorfile>
      <anchor>c30a0d9a4c1381707d7a81bffcc83957</anchor>
      <arglist></arglist>
    </member>
    <member kind="variable">
      <type>unsigned int</type>
      <name>mask</name>
      <anchorfile>structSM__Port.html</anchorfile>
      <anchor>359cc847fbfda3eef3b8e73d331441c9</anchor>
      <arglist></arglist>
    </member>
    <member kind="variable">
      <type>SM_Syllable *</type>
      <name>syllable</name>
      <anchorfile>structSM__Port.html</anchorfile>
      <anchor>6db15bfba527aaaca2e7c01733fb0ea4</anchor>
      <arglist></arglist>
    </member>
  </compound>
  <compound kind="struct">
    <name>SM_Resource</name>
    <filename>structSM__Resource.html</filename>
    <member kind="variable">
      <type>char *</type>
      <name>name</name>
      <anchorfile>structSM__Resource.html</anchorfile>
      <anchor>3aa6bf364dba61fa4b037a8f3c1a63ff</anchor>
      <arglist></arglist>
    </member>
    <member kind="variable">
      <type>unsigned int</type>
      <name>mask</name>
      <anchorfile>structSM__Resource.html</anchorfile>
      <anchor>ad7ff7f7fac9aa8a9d6a26d54cc44f1b</anchor>
      <arglist></arglist>
    </member>
    <member kind="variable">
      <type>unsigned short</type>
      <name>offset</name>
      <anchorfile>structSM__Resource.html</anchorfile>
      <anchor>5a076ebfcd7a569f4ca7c407d3ddc50f</anchor>
      <arglist></arglist>
    </member>
  </compound>
  <compound kind="struct">
    <name>SM_Restriction</name>
    <filename>structSM__Restriction.html</filename>
    <member kind="variable">
      <type>char *</type>
      <name>name</name>
      <anchorfile>structSM__Restriction.html</anchorfile>
      <anchor>074017ef29881a1b727977d865c9a4e0</anchor>
      <arglist></arglist>
    </member>
    <member kind="variable">
      <type>unsigned int</type>
      <name>num</name>
      <anchorfile>structSM__Restriction.html</anchorfile>
      <anchor>f35ef6269b70ebb9724091cda34c3d05</anchor>
      <arglist></arglist>
    </member>
    <member kind="variable">
      <type>unsigned int</type>
      <name>mask</name>
      <anchorfile>structSM__Restriction.html</anchorfile>
      <anchor>116441544c01df94f51659922ea3b69b</anchor>
      <arglist></arglist>
    </member>
  </compound>
  <compound kind="struct">
    <name>SM_Syllable</name>
    <filename>structSM__Syllable.html</filename>
    <member kind="variable">
      <type>char *</type>
      <name>name</name>
      <anchorfile>structSM__Syllable.html</anchorfile>
      <anchor>a66ef27d85bdeba77fa25dd9eca90eda</anchor>
      <arglist></arglist>
    </member>
    <member kind="variable">
      <type>unsigned int</type>
      <name>mask</name>
      <anchorfile>structSM__Syllable.html</anchorfile>
      <anchor>ab0dbe1aad0176b4ff1237513606cd8f</anchor>
      <arglist></arglist>
    </member>
    <member kind="variable">
      <type>unsigned int</type>
      <name>num_slots</name>
      <anchorfile>structSM__Syllable.html</anchorfile>
      <anchor>61c9e777787582828c4a220d5bdf7017</anchor>
      <arglist></arglist>
    </member>
  </compound>
  <compound kind="struct">
    <name>SM_Table</name>
    <filename>structSM__Table.html</filename>
    <member kind="variable">
      <type>SM_Choice *</type>
      <name>first_choice</name>
      <anchorfile>structSM__Table.html</anchorfile>
      <anchor>0adba2f1b2d467a3c9f4e69e19e1c074</anchor>
      <arglist></arglist>
    </member>
    <member kind="variable">
      <type>SM_Choice *</type>
      <name>last_choice</name>
      <anchorfile>structSM__Table.html</anchorfile>
      <anchor>a335742f5b2c7a5084f8e8b40ed87dec</anchor>
      <arglist></arglist>
    </member>
    <member kind="variable">
      <type>SM_Choice *</type>
      <name>slot_choice</name>
      <anchorfile>structSM__Table.html</anchorfile>
      <anchor>f5625d537c93c0fa99ce081f3633dfb6</anchor>
      <arglist></arglist>
    </member>
    <member kind="variable">
      <type>unsigned short</type>
      <name>num_choices</name>
      <anchorfile>structSM__Table.html</anchorfile>
      <anchor>19784b330e821f8944def28d84fc0af6</anchor>
      <arglist></arglist>
    </member>
    <member kind="variable">
      <type>unsigned short *</type>
      <name>slot_options</name>
      <anchorfile>structSM__Table.html</anchorfile>
      <anchor>1fceac25ab450c1b89d3d63631da69f6</anchor>
      <arglist></arglist>
    </member>
    <member kind="variable">
      <type>unsigned short</type>
      <name>max_usage_offset</name>
      <anchorfile>structSM__Table.html</anchorfile>
      <anchor>e8c6305a7872b2e7e6a03554037f1a51</anchor>
      <arglist></arglist>
    </member>
  </compound>
  <compound kind="struct">
    <name>SM_Template</name>
    <filename>structSM__Template.html</filename>
    <member kind="variable">
      <type>char *</type>
      <name>name</name>
      <anchorfile>structSM__Template.html</anchorfile>
      <anchor>64067d5876255279931b70090a9af1c3</anchor>
      <arglist></arglist>
    </member>
    <member kind="variable">
      <type>unsigned int</type>
      <name>mask</name>
      <anchorfile>structSM__Template.html</anchorfile>
      <anchor>6eb2e46e68aa3d2627ae1c5ec0063ed2</anchor>
      <arglist></arglist>
    </member>
    <member kind="variable">
      <type>int *</type>
      <name>stop_bits</name>
      <anchorfile>structSM__Template.html</anchorfile>
      <anchor>9eba8585dc09cb23d0496c6546260fe1</anchor>
      <arglist></arglist>
    </member>
    <member kind="variable">
      <type>unsigned int</type>
      <name>template_mask</name>
      <anchorfile>structSM__Template.html</anchorfile>
      <anchor>36e4502620ac85baeac032c5f98870ae</anchor>
      <arglist></arglist>
    </member>
  </compound>
  <compound kind="struct">
    <name>SM_TGroup</name>
    <filename>structSM__TGroup.html</filename>
    <member kind="variable">
      <type>char *</type>
      <name>name</name>
      <anchorfile>structSM__TGroup.html</anchorfile>
      <anchor>0396610df070d7d86d852542b6854ff8</anchor>
      <arglist></arglist>
    </member>
    <member kind="variable">
      <type>unsigned int</type>
      <name>mask</name>
      <anchorfile>structSM__TGroup.html</anchorfile>
      <anchor>fd6ebc9a4762418b200083ae815536cf</anchor>
      <arglist></arglist>
    </member>
  </compound>
  <compound kind="struct">
    <name>SM_Usage</name>
    <filename>structSM__Usage.html</filename>
    <member kind="variable">
      <type>unsigned int</type>
      <name>resources_used</name>
      <anchorfile>structSM__Usage.html</anchorfile>
      <anchor>a6c5397bc0a165691c0ca3beb7099c0e</anchor>
      <arglist></arglist>
    </member>
    <member kind="variable">
      <type>unsigned short</type>
      <name>map_offset</name>
      <anchorfile>structSM__Usage.html</anchorfile>
      <anchor>70c4334cff799f392e9fd0b0a70b846a</anchor>
      <arglist></arglist>
    </member>
  </compound>
</tagfile>
