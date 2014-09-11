<?xml version='1.0' encoding='ISO-8859-1' standalone='yes' ?>
<tagfile>
  <compound kind="file">
    <name>lb_b_internal.h</name>
    <path>/mnt/hgfs/mkt/Desktop/openimpact/src/Lcode/opti/Lblock/</path>
    <filename>lb__b__internal_8h</filename>
    <includes id="lb__traceregion_8h" name="lb_traceregion.h" local="yes" imported="no">lb_traceregion.h</includes>
    <includes id="lb__graph_8h" name="lb_graph.h" local="yes" imported="no">lb_graph.h</includes>
    <includes id="lb__flow_8h" name="lb_flow.h" local="yes" imported="no">lb_flow.h</includes>
    <includes id="lb__pred__tools_8h" name="lb_pred_tools.h" local="yes" imported="no">lb_pred_tools.h</includes>
    <includes id="lb__tail_8h" name="lb_tail.h" local="yes" imported="no">lb_tail.h</includes>
    <includes id="lb__tool_8h" name="lb_tool.h" local="yes" imported="no">lb_tool.h</includes>
    <member kind="define">
      <type>#define</type>
      <name>TRUE</name>
      <anchorfile>lb__b__internal_8h.html</anchorfile>
      <anchor>a8cecfc5c5c054d2875c03e77b7be15d</anchor>
      <arglist></arglist>
    </member>
    <member kind="define">
      <type>#define</type>
      <name>FALSE</name>
      <anchorfile>lb__b__internal_8h.html</anchorfile>
      <anchor>a93f0eb578d23995850d61f7d61c55c1</anchor>
      <arglist></arglist>
    </member>
    <member kind="define">
      <type>#define</type>
      <name>LB_HAND_MARKED_FOR_LOOP_PEEL</name>
      <anchorfile>lb__b__internal_8h.html</anchorfile>
      <anchor>5e86159a35dbfe98ef741599a701eae8</anchor>
      <arglist></arglist>
    </member>
    <member kind="define">
      <type>#define</type>
      <name>LB_MARKED_FOR_LOOP_PEEL</name>
      <anchorfile>lb__b__internal_8h.html</anchorfile>
      <anchor>930531c716a4c598c2920492c855463a</anchor>
      <arglist></arglist>
    </member>
    <member kind="define">
      <type>#define</type>
      <name>LB_MARKED_AS_PEELED</name>
      <anchorfile>lb__b__internal_8h.html</anchorfile>
      <anchor>20ad961f2ed36e451a3ba2ccd0b49e03</anchor>
      <arglist></arglist>
    </member>
    <member kind="define">
      <type>#define</type>
      <name>ITERATION_INFO_HEADER</name>
      <anchorfile>lb__b__internal_8h.html</anchorfile>
      <anchor>dd5c3e4ed6e84a01869372d4443915af</anchor>
      <arglist></arglist>
    </member>
    <member kind="define">
      <type>#define</type>
      <name>ITERATION_INFO_PREFIX</name>
      <anchorfile>lb__b__internal_8h.html</anchorfile>
      <anchor>896e7731434270d405395deeb49dc759</anchor>
      <arglist></arglist>
    </member>
    <member kind="define">
      <type>#define</type>
      <name>ITERATION_INFO_PREFIX_LENGTH</name>
      <anchorfile>lb__b__internal_8h.html</anchorfile>
      <anchor>e50f3034e9f1f798e2da180fd8e2352d</anchor>
      <arglist></arglist>
    </member>
    <member kind="typedef">
      <type>enum _LB_Branch_Pred_Types</type>
      <name>LB_Branch_Pred_Types</name>
      <anchorfile>lb__b__internal_8h.html</anchorfile>
      <anchor>d5d94e792362765f6e5a98808bad312f</anchor>
      <arglist></arglist>
    </member>
    <member kind="enumeration">
      <name>_LB_Branch_Pred_Types</name>
      <anchor>e2af09e62f4c88d521570190ebbfa57b</anchor>
      <arglist></arglist>
    </member>
    <member kind="enumvalue">
      <name>PROFILE</name>
      <anchor>e2af09e62f4c88d521570190ebbfa57baf6762155cbba62562dbb07ba2efc46c</anchor>
      <arglist></arglist>
    </member>
    <member kind="enumvalue">
      <name>STATIC</name>
      <anchor>e2af09e62f4c88d521570190ebbfa57be55a36a850c67d46b3b3325de7fce0b8</anchor>
      <arglist></arglist>
    </member>
    <member kind="enumvalue">
      <name>BOTH</name>
      <anchor>e2af09e62f4c88d521570190ebbfa57b627abe5a430420baf29ebe1940a7f2fb</anchor>
      <arglist></arglist>
    </member>
    <member kind="function">
      <type>void</type>
      <name>LB_read_parm_lblock</name>
      <anchorfile>lb__b__internal_8h.html</anchorfile>
      <anchor>ccc82d7821cb9956c95d133034ae8a51</anchor>
      <arglist>(Parm_Parse_Info *)</arglist>
    </member>
    <member kind="function">
      <type>LB_TraceRegion_Header *</type>
      <name>LB_function_init</name>
      <anchorfile>lb__b__internal_8h.html</anchorfile>
      <anchor>52dce452ec0eaf404ac768c8d909e863</anchor>
      <arglist>(L_Func *)</arglist>
    </member>
    <member kind="function">
      <type>void</type>
      <name>LB_function_deinit</name>
      <anchorfile>lb__b__internal_8h.html</anchorfile>
      <anchor>ded1339fabffb7ad4d3855a248845942</anchor>
      <arglist>(LB_TraceRegion_Header *)</arglist>
    </member>
    <member kind="variable">
      <type>char *</type>
      <name>LB_block_formation_type</name>
      <anchorfile>lb__b__internal_8h.html</anchorfile>
      <anchor>bb2751581b6fc590cf7565da677f163e</anchor>
      <arglist></arglist>
    </member>
    <member kind="variable">
      <type>char *</type>
      <name>LB_predicate_formation_type</name>
      <anchorfile>lb__b__internal_8h.html</anchorfile>
      <anchor>cde56f48ee00d9b4117bacbe6abb1baf</anchor>
      <arglist></arglist>
    </member>
    <member kind="variable">
      <type>int</type>
      <name>LB_use_block_enum_selector</name>
      <anchorfile>lb__b__internal_8h.html</anchorfile>
      <anchor>ab40a64dd60f29224ecd5b0ee5bb254d</anchor>
      <arglist></arglist>
    </member>
    <member kind="variable">
      <type>int</type>
      <name>LB_make_zero_weight_regions</name>
      <anchorfile>lb__b__internal_8h.html</anchorfile>
      <anchor>dd4a1356b4683ceff16b2ab54b9986c9</anchor>
      <arglist></arglist>
    </member>
    <member kind="variable">
      <type>char *</type>
      <name>LB_prevent_block_functions</name>
      <anchorfile>lb__b__internal_8h.html</anchorfile>
      <anchor>443b408354ebd10c8069b006896abd95</anchor>
      <arglist></arglist>
    </member>
    <member kind="variable">
      <type>Func_Name_List *</type>
      <name>LB_prevent_block_list</name>
      <anchorfile>lb__b__internal_8h.html</anchorfile>
      <anchor>4f70cc19ad64237ab4d4b8625218e54a</anchor>
      <arglist></arglist>
    </member>
    <member kind="variable">
      <type>int</type>
      <name>LB_verbose_prevent_block</name>
      <anchorfile>lb__b__internal_8h.html</anchorfile>
      <anchor>c70517f26d606a9705e92b752b9a79b7</anchor>
      <arglist></arglist>
    </member>
    <member kind="variable">
      <type>int</type>
      <name>LB_hb_pred_all_paths</name>
      <anchorfile>lb__b__internal_8h.html</anchorfile>
      <anchor>8181a397d1ab57bc921430373c06bba9</anchor>
      <arglist></arglist>
    </member>
    <member kind="variable">
      <type>int</type>
      <name>LB_hb_exclude_all_jsrs</name>
      <anchorfile>lb__b__internal_8h.html</anchorfile>
      <anchor>6e9251fea37c6f140c03cfe320342396</anchor>
      <arglist></arglist>
    </member>
    <member kind="variable">
      <type>int</type>
      <name>LB_hb_exclude_all_pointer_stores</name>
      <anchorfile>lb__b__internal_8h.html</anchorfile>
      <anchor>5206cb3c2d7e5cb3c93dc86f7f2ebbbd</anchor>
      <arglist></arglist>
    </member>
    <member kind="variable">
      <type>int</type>
      <name>LB_hb_peel_enable</name>
      <anchorfile>lb__b__internal_8h.html</anchorfile>
      <anchor>365d6afb68ee2ef9820f43a90c21081a</anchor>
      <arglist></arglist>
    </member>
    <member kind="variable">
      <type>int</type>
      <name>LB_hb_peel_heuristic</name>
      <anchorfile>lb__b__internal_8h.html</anchorfile>
      <anchor>50bf32338f935a60736b913f0ada4c4e</anchor>
      <arglist></arglist>
    </member>
    <member kind="variable">
      <type>int</type>
      <name>LB_hb_peel_max_ops</name>
      <anchorfile>lb__b__internal_8h.html</anchorfile>
      <anchor>a3e5f94a4ea9c84faec7b16832ced74a</anchor>
      <arglist></arglist>
    </member>
    <member kind="variable">
      <type>int</type>
      <name>LB_hb_peel_infinity_iter</name>
      <anchorfile>lb__b__internal_8h.html</anchorfile>
      <anchor>5f4c803c8f742157b1020361cc2718d1</anchor>
      <arglist></arglist>
    </member>
    <member kind="variable">
      <type>double</type>
      <name>LB_hb_peel_min_overall_coverage</name>
      <anchorfile>lb__b__internal_8h.html</anchorfile>
      <anchor>5268dfbc9754229e2d690c1adb320dc6</anchor>
      <arglist></arglist>
    </member>
    <member kind="variable">
      <type>double</type>
      <name>LB_hb_peel_min_peelable_coverage</name>
      <anchorfile>lb__b__internal_8h.html</anchorfile>
      <anchor>4361213fe5779161b51a8ef0e9341208</anchor>
      <arglist></arglist>
    </member>
    <member kind="variable">
      <type>double</type>
      <name>LB_hb_peel_inc_peelable_coverage</name>
      <anchorfile>lb__b__internal_8h.html</anchorfile>
      <anchor>cf915fa52e39071cee478a204c8f2e63</anchor>
      <arglist></arglist>
    </member>
    <member kind="variable">
      <type>double</type>
      <name>LB_hb_peel_min_util</name>
      <anchorfile>lb__b__internal_8h.html</anchorfile>
      <anchor>499bd22b300c916c7e73d207e077b786</anchor>
      <arglist></arglist>
    </member>
    <member kind="variable">
      <type>int</type>
      <name>LB_hb_peel_partial</name>
      <anchorfile>lb__b__internal_8h.html</anchorfile>
      <anchor>82b02a5e862910f56e6172a9a9c3a526</anchor>
      <arglist></arglist>
    </member>
    <member kind="variable">
      <type>int</type>
      <name>LB_hb_issue_width</name>
      <anchorfile>lb__b__internal_8h.html</anchorfile>
      <anchor>7f3d0aff35526de028252e3d2127ea4a</anchor>
      <arglist></arglist>
    </member>
    <member kind="variable">
      <type>double</type>
      <name>LB_hb_path_max_op_growth</name>
      <anchorfile>lb__b__internal_8h.html</anchorfile>
      <anchor>ddefbfb19cc27d53d050bb820c267337</anchor>
      <arglist></arglist>
    </member>
    <member kind="variable">
      <type>double</type>
      <name>LB_hb_path_max_dep_growth</name>
      <anchorfile>lb__b__internal_8h.html</anchorfile>
      <anchor>aadf2f704b67f8a8e8ae7e708e041e78</anchor>
      <arglist></arglist>
    </member>
    <member kind="variable">
      <type>int</type>
      <name>LB_hb_min_ratio_sens_opct</name>
      <anchorfile>lb__b__internal_8h.html</anchorfile>
      <anchor>67846f1b593c184e0d934a3acbaf72c8</anchor>
      <arglist></arglist>
    </member>
    <member kind="variable">
      <type>double</type>
      <name>LB_hb_path_min_exec_ratio</name>
      <anchorfile>lb__b__internal_8h.html</anchorfile>
      <anchor>9756dfd78bb92c66e9de12c69e99b6b0</anchor>
      <arglist></arglist>
    </member>
    <member kind="variable">
      <type>double</type>
      <name>LB_hb_path_main_rel_exec_ratio</name>
      <anchorfile>lb__b__internal_8h.html</anchorfile>
      <anchor>950e8922d47ea6c9c2d122eaded5da70</anchor>
      <arglist></arglist>
    </member>
    <member kind="variable">
      <type>double</type>
      <name>LB_hb_path_min_main_exec_ratio</name>
      <anchorfile>lb__b__internal_8h.html</anchorfile>
      <anchor>8a35d16f39a31fe9092075a1d1db388c</anchor>
      <arglist></arglist>
    </member>
    <member kind="variable">
      <type>double</type>
      <name>LB_hb_path_min_priority_ratio</name>
      <anchorfile>lb__b__internal_8h.html</anchorfile>
      <anchor>de35c2c0ea59296f5df53f962cb684a7</anchor>
      <arglist></arglist>
    </member>
    <member kind="variable">
      <type>double</type>
      <name>LB_hb_min_cb_weight</name>
      <anchorfile>lb__b__internal_8h.html</anchorfile>
      <anchor>394f63d8d0e0e728f73f220c1d65e1fb</anchor>
      <arglist></arglist>
    </member>
    <member kind="variable">
      <type>double</type>
      <name>LB_hb_block_min_weight_ratio</name>
      <anchorfile>lb__b__internal_8h.html</anchorfile>
      <anchor>0097f5ae356a93a1ae27e7931de9c8c5</anchor>
      <arglist></arglist>
    </member>
    <member kind="variable">
      <type>double</type>
      <name>LB_hb_block_min_path_ratio</name>
      <anchorfile>lb__b__internal_8h.html</anchorfile>
      <anchor>7608dc27ce790053015e48d2555605cf</anchor>
      <arglist></arglist>
    </member>
    <member kind="variable">
      <type>int</type>
      <name>LB_hb_max_cb_in_hammock</name>
      <anchorfile>lb__b__internal_8h.html</anchorfile>
      <anchor>3aa34d26db717b0d33402096d7badca8</anchor>
      <arglist></arglist>
    </member>
    <member kind="variable">
      <type>int</type>
      <name>LB_hb_form_simple_hammocks_only</name>
      <anchorfile>lb__b__internal_8h.html</anchorfile>
      <anchor>85313c68c2f1e220d5b99395de9bed29</anchor>
      <arglist></arglist>
    </member>
    <member kind="variable">
      <type>int</type>
      <name>LB_hb_available_predicates</name>
      <anchorfile>lb__b__internal_8h.html</anchorfile>
      <anchor>2d5a153f58c6392d99e3a1d943913ae6</anchor>
      <arglist></arglist>
    </member>
    <member kind="variable">
      <type>int</type>
      <name>LB_hb_branch_split</name>
      <anchorfile>lb__b__internal_8h.html</anchorfile>
      <anchor>1bc51e70670dee64d20ca12ec64ed07e</anchor>
      <arglist></arglist>
    </member>
    <member kind="variable">
      <type>int</type>
      <name>LB_hb_verbose_level</name>
      <anchorfile>lb__b__internal_8h.html</anchorfile>
      <anchor>948b3aa7cbf2e28c9bde4f59b6ce12f9</anchor>
      <arglist></arglist>
    </member>
    <member kind="variable">
      <type>int</type>
      <name>LB_hb_do_pred_merge</name>
      <anchorfile>lb__b__internal_8h.html</anchorfile>
      <anchor>1b70df1a7abdefb6ef910b0872137c2c</anchor>
      <arglist></arglist>
    </member>
    <member kind="variable">
      <type>int</type>
      <name>LB_hb_do_loop_collapsing</name>
      <anchorfile>lb__b__internal_8h.html</anchorfile>
      <anchor>76d9f99f72259f0d10938d95e1a782bc</anchor>
      <arglist></arglist>
    </member>
    <member kind="variable">
      <type>double</type>
      <name>LB_tail_dup_growth</name>
      <anchorfile>lb__b__internal_8h.html</anchorfile>
      <anchor>e3404278b2d92111c98d35bad4534c94</anchor>
      <arglist></arglist>
    </member>
    <member kind="variable">
      <type>double</type>
      <name>LB_max_static_tail_dup</name>
      <anchorfile>lb__b__internal_8h.html</anchorfile>
      <anchor>e013646edd9b345f2c05389af84e3797</anchor>
      <arglist></arglist>
    </member>
    <member kind="variable">
      <type>double</type>
      <name>LB_max_dyn_tail_dup</name>
      <anchorfile>lb__b__internal_8h.html</anchorfile>
      <anchor>094a35e17362bd32fdcd13adcb0e59ac</anchor>
      <arglist></arglist>
    </member>
    <member kind="variable">
      <type>char *</type>
      <name>LB_hb_unsafe_jsr_hazard_method_name</name>
      <anchorfile>lb__b__internal_8h.html</anchorfile>
      <anchor>a0764f8d5e22f99f9f2e1fbf8fce63b4</anchor>
      <arglist></arglist>
    </member>
    <member kind="variable">
      <type>char *</type>
      <name>LB_hb_safe_jsr_hazard_method_name</name>
      <anchorfile>lb__b__internal_8h.html</anchorfile>
      <anchor>4739c8a7ca2e181b77ba3ad564f7aea7</anchor>
      <arglist></arglist>
    </member>
    <member kind="variable">
      <type>char *</type>
      <name>LB_hb_pointer_st_hazard_method_name</name>
      <anchorfile>lb__b__internal_8h.html</anchorfile>
      <anchor>858cf215ca28e171a459679aa489d178</anchor>
      <arglist></arglist>
    </member>
    <member kind="variable">
      <type>int</type>
      <name>LB_hb_unsafe_jsr_hazard_method</name>
      <anchorfile>lb__b__internal_8h.html</anchorfile>
      <anchor>e9c9e160053548538e015d5fefc8b8ba</anchor>
      <arglist></arglist>
    </member>
    <member kind="variable">
      <type>int</type>
      <name>LB_hb_safe_jsr_hazard_method</name>
      <anchorfile>lb__b__internal_8h.html</anchorfile>
      <anchor>a5be1fd1944505e184dce7ebe0dc2db8</anchor>
      <arglist></arglist>
    </member>
    <member kind="variable">
      <type>int</type>
      <name>LB_hb_pointer_st_hazard_method</name>
      <anchorfile>lb__b__internal_8h.html</anchorfile>
      <anchor>96449cf5df1801eb9465e64c7153c980</anchor>
      <arglist></arglist>
    </member>
    <member kind="variable">
      <type>double</type>
      <name>LB_hb_unsafe_jsr_priority_penalty</name>
      <anchorfile>lb__b__internal_8h.html</anchorfile>
      <anchor>cd6e1e79004d00242ccf177012e89ac3</anchor>
      <arglist></arglist>
    </member>
    <member kind="variable">
      <type>double</type>
      <name>LB_hb_safe_jsr_priority_penalty</name>
      <anchorfile>lb__b__internal_8h.html</anchorfile>
      <anchor>3084d6ab7e538785ef99369fbde81a53</anchor>
      <arglist></arglist>
    </member>
    <member kind="variable">
      <type>double</type>
      <name>LB_hb_pointer_st_priority_penalty</name>
      <anchorfile>lb__b__internal_8h.html</anchorfile>
      <anchor>666773e96cf1315c5ba3e9f25c87f294</anchor>
      <arglist></arglist>
    </member>
    <member kind="variable">
      <type>int</type>
      <name>LB_hb_do_combine_exits</name>
      <anchorfile>lb__b__internal_8h.html</anchorfile>
      <anchor>08a8edaaa6ea80f4c0fd7771cd9982f8</anchor>
      <arglist></arglist>
    </member>
    <member kind="variable">
      <type>int</type>
      <name>LB_setup_only</name>
      <anchorfile>lb__b__internal_8h.html</anchorfile>
      <anchor>08872ea254e089bb8370357532b4a8ed</anchor>
      <arglist></arglist>
    </member>
    <member kind="variable">
      <type>int</type>
      <name>LB_split_critical</name>
      <anchorfile>lb__b__internal_8h.html</anchorfile>
      <anchor>a31ec687b73eaffe089200bcd5ac9daf</anchor>
      <arglist></arglist>
    </member>
    <member kind="variable">
      <type>char *</type>
      <name>LB_parm_branch_prediction_method</name>
      <anchorfile>lb__b__internal_8h.html</anchorfile>
      <anchor>ec17640bfd7e283c5c20061938d89d7c</anchor>
      <arglist></arglist>
    </member>
    <member kind="variable">
      <type>double</type>
      <name>LB_minimum_superblock_weight</name>
      <anchorfile>lb__b__internal_8h.html</anchorfile>
      <anchor>00025c300d4302ca2ab1955e0d7efd1b</anchor>
      <arglist></arglist>
    </member>
    <member kind="variable">
      <type>double</type>
      <name>LB_maximum_code_growth</name>
      <anchorfile>lb__b__internal_8h.html</anchorfile>
      <anchor>01b244cec54a5b01015a6801e8ed16e5</anchor>
      <arglist></arglist>
    </member>
    <member kind="variable">
      <type>double</type>
      <name>LB_min_branch_ratio</name>
      <anchorfile>lb__b__internal_8h.html</anchorfile>
      <anchor>f2556eac008096bc4dcb2c8e7929e4b6</anchor>
      <arglist></arglist>
    </member>
    <member kind="variable">
      <type>double</type>
      <name>LB_trace_min_cb_ratio</name>
      <anchorfile>lb__b__internal_8h.html</anchorfile>
      <anchor>f6a9c8001b9dbf60f661038073ff3e33</anchor>
      <arglist></arglist>
    </member>
    <member kind="variable">
      <type>LB_Branch_Pred_Types</type>
      <name>LB_branch_prediction_method</name>
      <anchorfile>lb__b__internal_8h.html</anchorfile>
      <anchor>c03a456e10f55c7fef5a828f05c6c03b</anchor>
      <arglist></arglist>
    </member>
    <member kind="variable">
      <type>int</type>
      <name>LB_do_lightweight_pred_opti</name>
      <anchorfile>lb__b__internal_8h.html</anchorfile>
      <anchor>74a6334e8a43974e5c6c3156541ac48b</anchor>
      <arglist></arglist>
    </member>
  </compound>
  <compound kind="file">
    <name>lb_block.c</name>
    <path>/mnt/hgfs/mkt/Desktop/openimpact/src/Lcode/opti/Lblock/</path>
    <filename>lb__block_8c</filename>
    <includes id="lb__b__internal_8h" name="lb_b_internal.h" local="yes" imported="no">lb_b_internal.h</includes>
    <member kind="function">
      <type>void</type>
      <name>LB_block_formation</name>
      <anchorfile>lb__block_8c.html</anchorfile>
      <anchor>6e0c6a731b1c13dc5e4dc5f23d3c41d9</anchor>
      <arglist>(L_Func *fn)</arglist>
    </member>
  </compound>
  <compound kind="file">
    <name>lb_block.h</name>
    <path>/mnt/hgfs/mkt/Desktop/openimpact/src/Lcode/opti/Lblock/</path>
    <filename>lb__block_8h</filename>
    <member kind="function">
      <type>void</type>
      <name>LB_block_init</name>
      <anchorfile>lb__block_8h.html</anchorfile>
      <anchor>8e33a9544a3403395edac156e57900be</anchor>
      <arglist>(Parm_Macro_List *)</arglist>
    </member>
    <member kind="function">
      <type>void</type>
      <name>LB_block_formation</name>
      <anchorfile>lb__block_8h.html</anchorfile>
      <anchor>da334fe41d4d168153f2b016ec2cafb3</anchor>
      <arglist>(L_Func *)</arglist>
    </member>
    <member kind="function">
      <type>void</type>
      <name>LB_superblock_formation</name>
      <anchorfile>lb__block_8h.html</anchorfile>
      <anchor>3c83c72e035cfd81fd04da2355b5f8b2</anchor>
      <arglist>(L_Func *)</arglist>
    </member>
    <member kind="function">
      <type>void</type>
      <name>LB_hyperblock_formation</name>
      <anchorfile>lb__block_8h.html</anchorfile>
      <anchor>ba7016d9cf1920ec358e71e5979dd44c</anchor>
      <arglist>(L_Func *)</arglist>
    </member>
    <member kind="function">
      <type>void</type>
      <name>LB_code_layout</name>
      <anchorfile>lb__block_8h.html</anchorfile>
      <anchor>9e4a4d996ec870dcbb45e5f59758f246</anchor>
      <arglist>(L_Func *)</arglist>
    </member>
  </compound>
  <compound kind="file">
    <name>lb_block_init.c</name>
    <path>/mnt/hgfs/mkt/Desktop/openimpact/src/Lcode/opti/Lblock/</path>
    <filename>lb__block__init_8c</filename>
    <includes id="lb__b__internal_8h" name="lb_b_internal.h" local="yes" imported="no">lb_b_internal.h</includes>
    <member kind="define">
      <type>#define</type>
      <name>LB_HAZARD_UNDEFINED</name>
      <anchorfile>lb__block__init_8c.html</anchorfile>
      <anchor>87e807cbff8fcd197e91a16d25a57454</anchor>
      <arglist></arglist>
    </member>
    <member kind="define">
      <type>#define</type>
      <name>LB_HAZARD_EXCLUDE_ALL</name>
      <anchorfile>lb__block__init_8c.html</anchorfile>
      <anchor>8c5ed492f0df71427c869040e5f9e1fe</anchor>
      <arglist></arglist>
    </member>
    <member kind="define">
      <type>#define</type>
      <name>LB_HAZARD_EXCLUDE_NON_TRACE</name>
      <anchorfile>lb__block__init_8c.html</anchorfile>
      <anchor>bb68cb2e4db422eaa53f7ec27ddb72b0</anchor>
      <arglist></arglist>
    </member>
    <member kind="define">
      <type>#define</type>
      <name>LB_HAZARD_EXCLUDE_HEURISTIC</name>
      <anchorfile>lb__block__init_8c.html</anchorfile>
      <anchor>8b060aaf7da37135545223e2ee86c6a2</anchor>
      <arglist></arglist>
    </member>
    <member kind="define">
      <type>#define</type>
      <name>LB_HAZARD_IGNORE</name>
      <anchorfile>lb__block__init_8c.html</anchorfile>
      <anchor>4c42f122829f2e2b594e18572b0b33bb</anchor>
      <arglist></arglist>
    </member>
    <member kind="function" static="yes">
      <type>static void</type>
      <name>LB_hb_check_parm_values</name>
      <anchorfile>lb__block__init_8c.html</anchorfile>
      <anchor>2bca6adfb7624fdc44d842aff3661aa3</anchor>
      <arglist>(void)</arglist>
    </member>
    <member kind="function" static="yes">
      <type>static void</type>
      <name>LB_hb_override_priority_penalties</name>
      <anchorfile>lb__block__init_8c.html</anchorfile>
      <anchor>b9fc2a79a7f5fb0c2eaa10e5b6ee185a</anchor>
      <arglist>(void)</arglist>
    </member>
    <member kind="function" static="yes">
      <type>static int</type>
      <name>LB_hb_hazard_method_string_to_define</name>
      <anchorfile>lb__block__init_8c.html</anchorfile>
      <anchor>739dee74e0f39fb8856550ba11160d46</anchor>
      <arglist>(char *s)</arglist>
    </member>
    <member kind="function">
      <type>void</type>
      <name>LB_read_parm_lblock</name>
      <anchorfile>lb__block__init_8c.html</anchorfile>
      <anchor>8023d2e2a2761bf40c56c5de8ff63a7e</anchor>
      <arglist>(Parm_Parse_Info *ppi)</arglist>
    </member>
    <member kind="function">
      <type>void</type>
      <name>LB_block_init</name>
      <anchorfile>lb__block__init_8c.html</anchorfile>
      <anchor>02abb71b52c17fcd63d537ce49d8010f</anchor>
      <arglist>(Parm_Macro_List *command_line_macro_list)</arglist>
    </member>
    <member kind="function">
      <type>LB_TraceRegion_Header *</type>
      <name>LB_function_init</name>
      <anchorfile>lb__block__init_8c.html</anchorfile>
      <anchor>4b92944f6a2d1b3dc5757c3f8cd19991</anchor>
      <arglist>(L_Func *fn)</arglist>
    </member>
    <member kind="function">
      <type>void</type>
      <name>LB_function_deinit</name>
      <anchorfile>lb__block__init_8c.html</anchorfile>
      <anchor>40be530ea9eee17c1a73aecab9c74bae</anchor>
      <arglist>(LB_TraceRegion_Header *header)</arglist>
    </member>
    <member kind="variable">
      <type>char *</type>
      <name>LB_block_formation_type</name>
      <anchorfile>lb__block__init_8c.html</anchorfile>
      <anchor>bb2751581b6fc590cf7565da677f163e</anchor>
      <arglist></arglist>
    </member>
    <member kind="variable">
      <type>char *</type>
      <name>LB_predicate_formation_type</name>
      <anchorfile>lb__block__init_8c.html</anchorfile>
      <anchor>cde56f48ee00d9b4117bacbe6abb1baf</anchor>
      <arglist></arglist>
    </member>
    <member kind="variable">
      <type>int</type>
      <name>LB_use_block_enum_selector</name>
      <anchorfile>lb__block__init_8c.html</anchorfile>
      <anchor>ab40a64dd60f29224ecd5b0ee5bb254d</anchor>
      <arglist></arglist>
    </member>
    <member kind="variable">
      <type>int</type>
      <name>LB_make_zero_weight_regions</name>
      <anchorfile>lb__block__init_8c.html</anchorfile>
      <anchor>dd4a1356b4683ceff16b2ab54b9986c9</anchor>
      <arglist></arglist>
    </member>
    <member kind="variable">
      <type>char *</type>
      <name>LB_prevent_block_functions</name>
      <anchorfile>lb__block__init_8c.html</anchorfile>
      <anchor>443b408354ebd10c8069b006896abd95</anchor>
      <arglist></arglist>
    </member>
    <member kind="variable">
      <type>Func_Name_List *</type>
      <name>LB_prevent_block_list</name>
      <anchorfile>lb__block__init_8c.html</anchorfile>
      <anchor>4f70cc19ad64237ab4d4b8625218e54a</anchor>
      <arglist></arglist>
    </member>
    <member kind="variable">
      <type>int</type>
      <name>LB_verbose_prevent_block</name>
      <anchorfile>lb__block__init_8c.html</anchorfile>
      <anchor>c70517f26d606a9705e92b752b9a79b7</anchor>
      <arglist></arglist>
    </member>
    <member kind="variable">
      <type>int</type>
      <name>LB_hb_pred_all_paths</name>
      <anchorfile>lb__block__init_8c.html</anchorfile>
      <anchor>8181a397d1ab57bc921430373c06bba9</anchor>
      <arglist></arglist>
    </member>
    <member kind="variable">
      <type>int</type>
      <name>LB_hb_exclude_all_jsrs</name>
      <anchorfile>lb__block__init_8c.html</anchorfile>
      <anchor>6e9251fea37c6f140c03cfe320342396</anchor>
      <arglist></arglist>
    </member>
    <member kind="variable">
      <type>int</type>
      <name>LB_hb_exclude_all_pointer_stores</name>
      <anchorfile>lb__block__init_8c.html</anchorfile>
      <anchor>5206cb3c2d7e5cb3c93dc86f7f2ebbbd</anchor>
      <arglist></arglist>
    </member>
    <member kind="variable">
      <type>int</type>
      <name>LB_hb_peel_enable</name>
      <anchorfile>lb__block__init_8c.html</anchorfile>
      <anchor>365d6afb68ee2ef9820f43a90c21081a</anchor>
      <arglist></arglist>
    </member>
    <member kind="variable">
      <type>int</type>
      <name>LB_hb_peel_heuristic</name>
      <anchorfile>lb__block__init_8c.html</anchorfile>
      <anchor>50bf32338f935a60736b913f0ada4c4e</anchor>
      <arglist></arglist>
    </member>
    <member kind="variable">
      <type>int</type>
      <name>LB_hb_peel_partial</name>
      <anchorfile>lb__block__init_8c.html</anchorfile>
      <anchor>82b02a5e862910f56e6172a9a9c3a526</anchor>
      <arglist></arglist>
    </member>
    <member kind="variable">
      <type>int</type>
      <name>LB_hb_peel_max_ops</name>
      <anchorfile>lb__block__init_8c.html</anchorfile>
      <anchor>a3e5f94a4ea9c84faec7b16832ced74a</anchor>
      <arglist></arglist>
    </member>
    <member kind="variable">
      <type>int</type>
      <name>LB_hb_peel_infinity_iter</name>
      <anchorfile>lb__block__init_8c.html</anchorfile>
      <anchor>5f4c803c8f742157b1020361cc2718d1</anchor>
      <arglist></arglist>
    </member>
    <member kind="variable">
      <type>double</type>
      <name>LB_hb_peel_min_overall_coverage</name>
      <anchorfile>lb__block__init_8c.html</anchorfile>
      <anchor>5268dfbc9754229e2d690c1adb320dc6</anchor>
      <arglist></arglist>
    </member>
    <member kind="variable">
      <type>double</type>
      <name>LB_hb_peel_min_peelable_coverage</name>
      <anchorfile>lb__block__init_8c.html</anchorfile>
      <anchor>4361213fe5779161b51a8ef0e9341208</anchor>
      <arglist></arglist>
    </member>
    <member kind="variable">
      <type>double</type>
      <name>LB_hb_peel_inc_peelable_coverage</name>
      <anchorfile>lb__block__init_8c.html</anchorfile>
      <anchor>cf915fa52e39071cee478a204c8f2e63</anchor>
      <arglist></arglist>
    </member>
    <member kind="variable">
      <type>double</type>
      <name>LB_hb_peel_min_util</name>
      <anchorfile>lb__block__init_8c.html</anchorfile>
      <anchor>499bd22b300c916c7e73d207e077b786</anchor>
      <arglist></arglist>
    </member>
    <member kind="variable">
      <type>int</type>
      <name>LB_hb_issue_width</name>
      <anchorfile>lb__block__init_8c.html</anchorfile>
      <anchor>7f3d0aff35526de028252e3d2127ea4a</anchor>
      <arglist></arglist>
    </member>
    <member kind="variable">
      <type>double</type>
      <name>LB_hb_path_max_op_growth</name>
      <anchorfile>lb__block__init_8c.html</anchorfile>
      <anchor>ddefbfb19cc27d53d050bb820c267337</anchor>
      <arglist></arglist>
    </member>
    <member kind="variable">
      <type>double</type>
      <name>LB_hb_path_max_dep_growth</name>
      <anchorfile>lb__block__init_8c.html</anchorfile>
      <anchor>aadf2f704b67f8a8e8ae7e708e041e78</anchor>
      <arglist></arglist>
    </member>
    <member kind="variable">
      <type>int</type>
      <name>LB_hb_min_ratio_sens_opct</name>
      <anchorfile>lb__block__init_8c.html</anchorfile>
      <anchor>67846f1b593c184e0d934a3acbaf72c8</anchor>
      <arglist></arglist>
    </member>
    <member kind="variable">
      <type>double</type>
      <name>LB_hb_path_min_exec_ratio</name>
      <anchorfile>lb__block__init_8c.html</anchorfile>
      <anchor>9756dfd78bb92c66e9de12c69e99b6b0</anchor>
      <arglist></arglist>
    </member>
    <member kind="variable">
      <type>double</type>
      <name>LB_hb_path_min_main_exec_ratio</name>
      <anchorfile>lb__block__init_8c.html</anchorfile>
      <anchor>8a35d16f39a31fe9092075a1d1db388c</anchor>
      <arglist></arglist>
    </member>
    <member kind="variable">
      <type>double</type>
      <name>LB_hb_path_main_rel_exec_ratio</name>
      <anchorfile>lb__block__init_8c.html</anchorfile>
      <anchor>950e8922d47ea6c9c2d122eaded5da70</anchor>
      <arglist></arglist>
    </member>
    <member kind="variable">
      <type>double</type>
      <name>LB_hb_path_min_priority_ratio</name>
      <anchorfile>lb__block__init_8c.html</anchorfile>
      <anchor>de35c2c0ea59296f5df53f962cb684a7</anchor>
      <arglist></arglist>
    </member>
    <member kind="variable">
      <type>double</type>
      <name>LB_hb_min_cb_weight</name>
      <anchorfile>lb__block__init_8c.html</anchorfile>
      <anchor>394f63d8d0e0e728f73f220c1d65e1fb</anchor>
      <arglist></arglist>
    </member>
    <member kind="variable">
      <type>double</type>
      <name>LB_hb_block_min_weight_ratio</name>
      <anchorfile>lb__block__init_8c.html</anchorfile>
      <anchor>0097f5ae356a93a1ae27e7931de9c8c5</anchor>
      <arglist></arglist>
    </member>
    <member kind="variable">
      <type>double</type>
      <name>LB_hb_block_min_path_ratio</name>
      <anchorfile>lb__block__init_8c.html</anchorfile>
      <anchor>7608dc27ce790053015e48d2555605cf</anchor>
      <arglist></arglist>
    </member>
    <member kind="variable">
      <type>int</type>
      <name>LB_hb_max_cb_in_hammock</name>
      <anchorfile>lb__block__init_8c.html</anchorfile>
      <anchor>3aa34d26db717b0d33402096d7badca8</anchor>
      <arglist></arglist>
    </member>
    <member kind="variable">
      <type>int</type>
      <name>LB_hb_form_simple_hammocks_only</name>
      <anchorfile>lb__block__init_8c.html</anchorfile>
      <anchor>85313c68c2f1e220d5b99395de9bed29</anchor>
      <arglist></arglist>
    </member>
    <member kind="variable">
      <type>int</type>
      <name>LB_hb_available_predicates</name>
      <anchorfile>lb__block__init_8c.html</anchorfile>
      <anchor>2d5a153f58c6392d99e3a1d943913ae6</anchor>
      <arglist></arglist>
    </member>
    <member kind="variable">
      <type>int</type>
      <name>LB_hb_branch_split</name>
      <anchorfile>lb__block__init_8c.html</anchorfile>
      <anchor>1bc51e70670dee64d20ca12ec64ed07e</anchor>
      <arglist></arglist>
    </member>
    <member kind="variable">
      <type>int</type>
      <name>LB_hb_verbose_level</name>
      <anchorfile>lb__block__init_8c.html</anchorfile>
      <anchor>948b3aa7cbf2e28c9bde4f59b6ce12f9</anchor>
      <arglist></arglist>
    </member>
    <member kind="variable">
      <type>int</type>
      <name>LB_hb_do_pred_merge</name>
      <anchorfile>lb__block__init_8c.html</anchorfile>
      <anchor>1b70df1a7abdefb6ef910b0872137c2c</anchor>
      <arglist></arglist>
    </member>
    <member kind="variable">
      <type>int</type>
      <name>LB_hb_do_loop_collapsing</name>
      <anchorfile>lb__block__init_8c.html</anchorfile>
      <anchor>76d9f99f72259f0d10938d95e1a782bc</anchor>
      <arglist></arglist>
    </member>
    <member kind="variable">
      <type>double</type>
      <name>LB_tail_dup_growth</name>
      <anchorfile>lb__block__init_8c.html</anchorfile>
      <anchor>e3404278b2d92111c98d35bad4534c94</anchor>
      <arglist></arglist>
    </member>
    <member kind="variable">
      <type>double</type>
      <name>LB_max_static_tail_dup</name>
      <anchorfile>lb__block__init_8c.html</anchorfile>
      <anchor>e013646edd9b345f2c05389af84e3797</anchor>
      <arglist></arglist>
    </member>
    <member kind="variable">
      <type>double</type>
      <name>LB_max_dyn_tail_dup</name>
      <anchorfile>lb__block__init_8c.html</anchorfile>
      <anchor>094a35e17362bd32fdcd13adcb0e59ac</anchor>
      <arglist></arglist>
    </member>
    <member kind="variable">
      <type>char *</type>
      <name>LB_hb_unsafe_jsr_hazard_method_name</name>
      <anchorfile>lb__block__init_8c.html</anchorfile>
      <anchor>a0764f8d5e22f99f9f2e1fbf8fce63b4</anchor>
      <arglist></arglist>
    </member>
    <member kind="variable">
      <type>char *</type>
      <name>LB_hb_safe_jsr_hazard_method_name</name>
      <anchorfile>lb__block__init_8c.html</anchorfile>
      <anchor>4739c8a7ca2e181b77ba3ad564f7aea7</anchor>
      <arglist></arglist>
    </member>
    <member kind="variable">
      <type>char *</type>
      <name>LB_hb_pointer_st_hazard_method_name</name>
      <anchorfile>lb__block__init_8c.html</anchorfile>
      <anchor>858cf215ca28e171a459679aa489d178</anchor>
      <arglist></arglist>
    </member>
    <member kind="variable">
      <type>int</type>
      <name>LB_hb_unsafe_jsr_hazard_method</name>
      <anchorfile>lb__block__init_8c.html</anchorfile>
      <anchor>e9c9e160053548538e015d5fefc8b8ba</anchor>
      <arglist></arglist>
    </member>
    <member kind="variable">
      <type>int</type>
      <name>LB_hb_safe_jsr_hazard_method</name>
      <anchorfile>lb__block__init_8c.html</anchorfile>
      <anchor>a5be1fd1944505e184dce7ebe0dc2db8</anchor>
      <arglist></arglist>
    </member>
    <member kind="variable">
      <type>int</type>
      <name>LB_hb_pointer_st_hazard_method</name>
      <anchorfile>lb__block__init_8c.html</anchorfile>
      <anchor>96449cf5df1801eb9465e64c7153c980</anchor>
      <arglist></arglist>
    </member>
    <member kind="variable">
      <type>double</type>
      <name>LB_hb_unsafe_jsr_priority_penalty</name>
      <anchorfile>lb__block__init_8c.html</anchorfile>
      <anchor>cd6e1e79004d00242ccf177012e89ac3</anchor>
      <arglist></arglist>
    </member>
    <member kind="variable">
      <type>double</type>
      <name>LB_hb_safe_jsr_priority_penalty</name>
      <anchorfile>lb__block__init_8c.html</anchorfile>
      <anchor>3084d6ab7e538785ef99369fbde81a53</anchor>
      <arglist></arglist>
    </member>
    <member kind="variable">
      <type>double</type>
      <name>LB_hb_pointer_st_priority_penalty</name>
      <anchorfile>lb__block__init_8c.html</anchorfile>
      <anchor>666773e96cf1315c5ba3e9f25c87f294</anchor>
      <arglist></arglist>
    </member>
    <member kind="variable">
      <type>int</type>
      <name>LB_hb_do_combine_exits</name>
      <anchorfile>lb__block__init_8c.html</anchorfile>
      <anchor>08a8edaaa6ea80f4c0fd7771cd9982f8</anchor>
      <arglist></arglist>
    </member>
    <member kind="variable">
      <type>int</type>
      <name>LB_setup_only</name>
      <anchorfile>lb__block__init_8c.html</anchorfile>
      <anchor>08872ea254e089bb8370357532b4a8ed</anchor>
      <arglist></arglist>
    </member>
    <member kind="variable">
      <type>int</type>
      <name>LB_split_critical</name>
      <anchorfile>lb__block__init_8c.html</anchorfile>
      <anchor>a31ec687b73eaffe089200bcd5ac9daf</anchor>
      <arglist></arglist>
    </member>
    <member kind="variable">
      <type>double</type>
      <name>LB_minimum_superblock_weight</name>
      <anchorfile>lb__block__init_8c.html</anchorfile>
      <anchor>00025c300d4302ca2ab1955e0d7efd1b</anchor>
      <arglist></arglist>
    </member>
    <member kind="variable">
      <type>double</type>
      <name>LB_maximum_code_growth</name>
      <anchorfile>lb__block__init_8c.html</anchorfile>
      <anchor>01b244cec54a5b01015a6801e8ed16e5</anchor>
      <arglist></arglist>
    </member>
    <member kind="variable">
      <type>double</type>
      <name>LB_min_branch_ratio</name>
      <anchorfile>lb__block__init_8c.html</anchorfile>
      <anchor>f2556eac008096bc4dcb2c8e7929e4b6</anchor>
      <arglist></arglist>
    </member>
    <member kind="variable">
      <type>double</type>
      <name>LB_trace_min_cb_ratio</name>
      <anchorfile>lb__block__init_8c.html</anchorfile>
      <anchor>f6a9c8001b9dbf60f661038073ff3e33</anchor>
      <arglist></arglist>
    </member>
    <member kind="variable">
      <type>int</type>
      <name>LB_do_lightweight_pred_opti</name>
      <anchorfile>lb__block__init_8c.html</anchorfile>
      <anchor>74a6334e8a43974e5c6c3156541ac48b</anchor>
      <arglist></arglist>
    </member>
  </compound>
  <compound kind="file">
    <name>lb_br_split.c</name>
    <path>/mnt/hgfs/mkt/Desktop/openimpact/src/Lcode/opti/Lblock/</path>
    <filename>lb__br__split_8c</filename>
    <includes id="lb__flow_8h" name="lb_flow.h" local="yes" imported="no">lb_flow.h</includes>
    <member kind="function">
      <type>void</type>
      <name>LB_branch_split_cb</name>
      <anchorfile>lb__br__split_8c.html</anchorfile>
      <anchor>342fd156868f5b5e4dd4b2f3858abb96</anchor>
      <arglist>(L_Func *fn, L_Cb *cb)</arglist>
    </member>
    <member kind="function">
      <type>void</type>
      <name>LB_branch_split_func</name>
      <anchorfile>lb__br__split_8c.html</anchorfile>
      <anchor>8a8c940f4175b86c3350e8e570f7591b</anchor>
      <arglist>(L_Func *fn)</arglist>
    </member>
  </compound>
  <compound kind="file">
    <name>lb_codegen.c</name>
    <path>/mnt/hgfs/mkt/Desktop/openimpact/src/Lcode/opti/Lblock/</path>
    <filename>lb__codegen_8c</filename>
    <includes id="lb__b__internal_8h" name="lb_b_internal.h" local="yes" imported="no">lb_b_internal.h</includes>
    <member kind="function" static="yes">
      <type>static void</type>
      <name>process_input</name>
      <anchorfile>lb__codegen_8c.html</anchorfile>
      <anchor>7e66299580e638dd1368d29ee0812a35</anchor>
      <arglist>(void)</arglist>
    </member>
    <member kind="function">
      <type>void</type>
      <name>L_gen_code</name>
      <anchorfile>lb__codegen_8c.html</anchorfile>
      <anchor>6346124c5fad5929833a7f7084f6be8c</anchor>
      <arglist>(Parm_Macro_List *command_line_macro_list)</arglist>
    </member>
  </compound>
  <compound kind="file">
    <name>lb_elim.c</name>
    <path>/mnt/hgfs/mkt/Desktop/openimpact/src/Lcode/opti/Lblock/</path>
    <filename>lb__elim_8c</filename>
    <includes id="lb__flow_8h" name="lb_flow.h" local="yes" imported="no">lb_flow.h</includes>
    <member kind="define">
      <type>#define</type>
      <name>ERR</name>
      <anchorfile>lb__elim_8c.html</anchorfile>
      <anchor>735563036dced0b7d6cc98f97ea4978b</anchor>
      <arglist></arglist>
    </member>
    <member kind="function" static="yes">
      <type>static void</type>
      <name>LB_do_uncond_branch_elim1</name>
      <anchorfile>lb__elim_8c.html</anchorfile>
      <anchor>37166fb67a25d893a1e35e0db57f877e</anchor>
      <arglist>(L_Func *fn)</arglist>
    </member>
    <member kind="function">
      <type>void</type>
      <name>LB_uncond_branch_elim</name>
      <anchorfile>lb__elim_8c.html</anchorfile>
      <anchor>f410bf0795f89e3a411be4cd84099631</anchor>
      <arglist>(L_Func *fn)</arglist>
    </member>
  </compound>
  <compound kind="file">
    <name>lb_explicit_br.c</name>
    <path>/mnt/hgfs/mkt/Desktop/openimpact/src/Lcode/opti/Lblock/</path>
    <filename>lb__explicit__br_8c</filename>
    <includes id="lb__flow_8h" name="lb_flow.h" local="yes" imported="no">lb_flow.h</includes>
    <member kind="function">
      <type>void</type>
      <name>LB_insert_explicit_branches_cb</name>
      <anchorfile>lb__explicit__br_8c.html</anchorfile>
      <anchor>32810268d44d5a98a3a9c9d3281e45b6</anchor>
      <arglist>(L_Cb *cb)</arglist>
    </member>
    <member kind="function">
      <type>void</type>
      <name>LB_insert_explicit_branches_func</name>
      <anchorfile>lb__explicit__br_8c.html</anchorfile>
      <anchor>a131901ed5905bfdadfd0e90f2e5bc14</anchor>
      <arglist>(L_Func *fn)</arglist>
    </member>
    <member kind="function">
      <type>void</type>
      <name>LB_delete_explicit_branches_cb</name>
      <anchorfile>lb__explicit__br_8c.html</anchorfile>
      <anchor>03fc5d9193101d2a5f2eb436b7ec90a8</anchor>
      <arglist>(L_Cb *cb)</arglist>
    </member>
    <member kind="function">
      <type>void</type>
      <name>LB_delete_explicit_branches_func</name>
      <anchorfile>lb__explicit__br_8c.html</anchorfile>
      <anchor>c719f6a8374a4bb996642ec9d27b9be1</anchor>
      <arglist>(L_Func *fn)</arglist>
    </member>
  </compound>
  <compound kind="file">
    <name>lb_flow.h</name>
    <path>/mnt/hgfs/mkt/Desktop/openimpact/src/Lcode/opti/Lblock/</path>
    <filename>lb__flow_8h</filename>
    <member kind="function">
      <type>void</type>
      <name>LB_branch_split_func</name>
      <anchorfile>lb__flow_8h.html</anchorfile>
      <anchor>871875ed286239ad635ddd488357b9a6</anchor>
      <arglist>(L_Func *)</arglist>
    </member>
    <member kind="function">
      <type>void</type>
      <name>LB_branch_split_cb</name>
      <anchorfile>lb__flow_8h.html</anchorfile>
      <anchor>e5afcf560c4f1cb2c44ecdf994e672c9</anchor>
      <arglist>(L_Func *, L_Cb *)</arglist>
    </member>
    <member kind="function">
      <type>void</type>
      <name>LB_insert_explicit_branches_func</name>
      <anchorfile>lb__flow_8h.html</anchorfile>
      <anchor>ab7c64d7076796ba6e92f744e7a4f6e8</anchor>
      <arglist>(L_Func *)</arglist>
    </member>
    <member kind="function">
      <type>void</type>
      <name>LB_insert_explicit_branches_cb</name>
      <anchorfile>lb__flow_8h.html</anchorfile>
      <anchor>0646c56b89d3b939dc41c319c572a04e</anchor>
      <arglist>(L_Cb *)</arglist>
    </member>
    <member kind="function">
      <type>void</type>
      <name>LB_delete_explicit_branches_func</name>
      <anchorfile>lb__flow_8h.html</anchorfile>
      <anchor>8f173cfdad2acc3037a85f6f1234a2be</anchor>
      <arglist>(L_Func *)</arglist>
    </member>
    <member kind="function">
      <type>void</type>
      <name>LB_delete_explicit_branches_cb</name>
      <anchorfile>lb__flow_8h.html</anchorfile>
      <anchor>73cf2a387b1027ae128f857b20ee4e91</anchor>
      <arglist>(L_Cb *)</arglist>
    </member>
    <member kind="function">
      <type>L_Cb *</type>
      <name>LB_breakup_cb</name>
      <anchorfile>lb__flow_8h.html</anchorfile>
      <anchor>39a1b3e8d02cd620fc471055e4a9a7d8</anchor>
      <arglist>(L_Func *, L_Cb *)</arglist>
    </member>
    <member kind="function">
      <type>void</type>
      <name>LB_convert_to_strict_basic_block_code</name>
      <anchorfile>lb__flow_8h.html</anchorfile>
      <anchor>811ef3187ed5a89bf2ca7aa9e159bd55</anchor>
      <arglist>(L_Func *, int)</arglist>
    </member>
    <member kind="function">
      <type>int</type>
      <name>LB_cb_contains_register_branch</name>
      <anchorfile>lb__flow_8h.html</anchorfile>
      <anchor>97ccd521a96e82a1417abfd51339c265</anchor>
      <arglist>(L_Cb *)</arglist>
    </member>
    <member kind="function">
      <type>void</type>
      <name>LB_mark_jrg_flag</name>
      <anchorfile>lb__flow_8h.html</anchorfile>
      <anchor>064b3919682dddcaeaa585417de119f9</anchor>
      <arglist>(L_Func *)</arglist>
    </member>
    <member kind="function">
      <type>void</type>
      <name>LB_uncond_branch_elim</name>
      <anchorfile>lb__flow_8h.html</anchorfile>
      <anchor>f410bf0795f89e3a411be4cd84099631</anchor>
      <arglist>(L_Func *fn)</arglist>
    </member>
    <member kind="function">
      <type>void</type>
      <name>LB_elim_all_loop_backedges</name>
      <anchorfile>lb__flow_8h.html</anchorfile>
      <anchor>41b44967d70bd76250cce3a9cfb9594a</anchor>
      <arglist>(L_Func *fn)</arglist>
    </member>
  </compound>
  <compound kind="file">
    <name>lb_graph.c</name>
    <path>/mnt/hgfs/mkt/Desktop/openimpact/src/Lcode/opti/Lblock/</path>
    <filename>lb__graph_8c</filename>
    <includes id="lb__b__internal_8h" name="lb_b_internal.h" local="yes" imported="no">lb_b_internal.h</includes>
    <member kind="function" static="yes">
      <type>static LB_BB *</type>
      <name>LB_new_bb</name>
      <anchorfile>lb__graph_8c.html</anchorfile>
      <anchor>ac04b30be8f8867b0358f48dad5ba179</anchor>
      <arglist>(L_Cb *cb, GraphNode node, int type)</arglist>
    </member>
    <member kind="function" static="yes">
      <type>static LB_BB *</type>
      <name>LB_free_bb</name>
      <anchorfile>lb__graph_8c.html</anchorfile>
      <anchor>849c6f385974beb968cd7477e13346e8</anchor>
      <arglist>(LB_BB *bb)</arglist>
    </member>
    <member kind="function" static="yes">
      <type>static void</type>
      <name>LB_form_region_graph</name>
      <anchorfile>lb__graph_8c.html</anchorfile>
      <anchor>f54753f853b088106bc35d677e4deb3a</anchor>
      <arglist>(L_Func *fn, Graph graph, L_Cb *header, Set blocks)</arglist>
    </member>
    <member kind="function">
      <type>Graph</type>
      <name>LB_create_flow_graph</name>
      <anchorfile>lb__graph_8c.html</anchorfile>
      <anchor>f1650115b518f5de03580d0f530dd025</anchor>
      <arglist>(L_Func *fn, L_Cb *initial_cb, Set blocks)</arglist>
    </member>
    <member kind="function">
      <type>Graph</type>
      <name>LB_free_flow_graph</name>
      <anchorfile>lb__graph_8c.html</anchorfile>
      <anchor>43d66ce2caf8360803294edcbf8e5e50</anchor>
      <arglist>(Graph graph)</arglist>
    </member>
    <member kind="function">
      <type>Graph</type>
      <name>LB_add_cb_to_graph</name>
      <anchorfile>lb__graph_8c.html</anchorfile>
      <anchor>0c0c1f021a6349cb52612b19aa572374</anchor>
      <arglist>(Graph flow_graph, L_Cb *inserted, L_Cb *anchor, int dir)</arglist>
    </member>
    <member kind="function">
      <type>Graph</type>
      <name>LB_finish_frp_graph</name>
      <anchorfile>lb__graph_8c.html</anchorfile>
      <anchor>071cb4a660c01a765e0a0bcca8043e11</anchor>
      <arglist>(Graph flow_graph)</arglist>
    </member>
    <member kind="function">
      <type>void</type>
      <name>LB_bb_print_hook</name>
      <anchorfile>lb__graph_8c.html</anchorfile>
      <anchor>fc62f56cf9d6732547b92bc2c9052d49</anchor>
      <arglist>(FILE *file, GraphNode node)</arglist>
    </member>
    <member kind="variable">
      <type>char *</type>
      <name>LB_predicate_formation_type</name>
      <anchorfile>lb__graph_8c.html</anchorfile>
      <anchor>cde56f48ee00d9b4117bacbe6abb1baf</anchor>
      <arglist></arglist>
    </member>
    <member kind="variable" static="yes">
      <type>static L_Alloc_Pool *</type>
      <name>LB_BB_Pool</name>
      <anchorfile>lb__graph_8c.html</anchorfile>
      <anchor>3c13c03c31c7ec2ecbd52ef441c261db</anchor>
      <arglist></arglist>
    </member>
  </compound>
  <compound kind="file">
    <name>lb_graph.h</name>
    <path>/mnt/hgfs/mkt/Desktop/openimpact/src/Lcode/opti/Lblock/</path>
    <filename>lb__graph_8h</filename>
    <class kind="struct">_LB_BB</class>
    <member kind="define">
      <type>#define</type>
      <name>CB</name>
      <anchorfile>lb__graph_8h.html</anchorfile>
      <anchor>f2af66b22013ff5ec25e2cdaf98ba3a4</anchor>
      <arglist></arglist>
    </member>
    <member kind="define">
      <type>#define</type>
      <name>START</name>
      <anchorfile>lb__graph_8h.html</anchorfile>
      <anchor>3018c7600b7bb9866400596a56a57af7</anchor>
      <arglist></arglist>
    </member>
    <member kind="define">
      <type>#define</type>
      <name>STOP</name>
      <anchorfile>lb__graph_8h.html</anchorfile>
      <anchor>e19b6bb2940d2fbe0a79852b070eeafd</anchor>
      <arglist></arglist>
    </member>
    <member kind="define">
      <type>#define</type>
      <name>FALLTHRU</name>
      <anchorfile>lb__graph_8h.html</anchorfile>
      <anchor>9a87c9dc1623c0f6389046974ea831e2</anchor>
      <arglist></arglist>
    </member>
    <member kind="define">
      <type>#define</type>
      <name>TAKEN</name>
      <anchorfile>lb__graph_8h.html</anchorfile>
      <anchor>2d654b8b0f7e8c3912cf0a336ce224ed</anchor>
      <arglist></arglist>
    </member>
    <member kind="define">
      <type>#define</type>
      <name>CONNECT_TO_STOP</name>
      <anchorfile>lb__graph_8h.html</anchorfile>
      <anchor>eff22590699eb29ed2866e8d242cee29</anchor>
      <arglist></arglist>
    </member>
    <member kind="define">
      <type>#define</type>
      <name>GraphNodeCB</name>
      <anchorfile>lb__graph_8h.html</anchorfile>
      <anchor>a49239026323969cc3c1fe8d7e15684c</anchor>
      <arglist>(n)</arglist>
    </member>
    <member kind="typedef">
      <type>_LB_BB</type>
      <name>LB_BB</name>
      <anchorfile>lb__graph_8h.html</anchorfile>
      <anchor>0bba32087528bd51030f429a081b5e14</anchor>
      <arglist></arglist>
    </member>
    <member kind="function">
      <type>Graph</type>
      <name>LB_create_flow_graph</name>
      <anchorfile>lb__graph_8h.html</anchorfile>
      <anchor>086b1289e6725a0b2ab6b5b33efbf3c1</anchor>
      <arglist>(L_Func *, L_Cb *, Set)</arglist>
    </member>
    <member kind="function">
      <type>Graph</type>
      <name>LB_free_flow_graph</name>
      <anchorfile>lb__graph_8h.html</anchorfile>
      <anchor>cad96406ef8ce4b896152e62360a18a5</anchor>
      <arglist>(Graph)</arglist>
    </member>
    <member kind="function">
      <type>Graph</type>
      <name>LB_add_cb_to_graph</name>
      <anchorfile>lb__graph_8h.html</anchorfile>
      <anchor>8df4f462d2693a6bfdbee5ba3bd59ffa</anchor>
      <arglist>(Graph, L_Cb *, L_Cb *, int)</arglist>
    </member>
    <member kind="function">
      <type>Graph</type>
      <name>LB_finish_frp_graph</name>
      <anchorfile>lb__graph_8h.html</anchorfile>
      <anchor>7da31fc40ac2371254be6d9520e4f476</anchor>
      <arglist>(Graph)</arglist>
    </member>
    <member kind="function">
      <type>void</type>
      <name>LB_bb_print_hook</name>
      <anchorfile>lb__graph_8h.html</anchorfile>
      <anchor>95f6d7e518660b815d0844d0e3d14448</anchor>
      <arglist>(FILE *, GraphNode)</arglist>
    </member>
  </compound>
  <compound kind="file">
    <name>lb_hb_block_enum.c</name>
    <path>/mnt/hgfs/mkt/Desktop/openimpact/src/Lcode/opti/Lblock/</path>
    <filename>lb__hb__block__enum_8c</filename>
    <includes id="lb__hb__hyperblock_8h" name="lb_hb_hyperblock.h" local="yes" imported="no">lb_hb_hyperblock.h</includes>
    <includes id="lb__b__internal_8h" name="lb_b_internal.h" local="yes" imported="no">lb_b_internal.h</includes>
    <member kind="define">
      <type>#define</type>
      <name>ERR</name>
      <anchorfile>lb__hb__block__enum_8c.html</anchorfile>
      <anchor>735563036dced0b7d6cc98f97ea4978b</anchor>
      <arglist></arglist>
    </member>
    <member kind="function" static="yes">
      <type>static void</type>
      <name>LB_find_total_num_ops</name>
      <anchorfile>lb__hb__block__enum_8c.html</anchorfile>
      <anchor>5ce59c2845f486bede79a5f68961b60f</anchor>
      <arglist>(Set blocks)</arglist>
    </member>
    <member kind="function" static="yes">
      <type>static void</type>
      <name>LB_find_tr_num_ops</name>
      <anchorfile>lb__hb__block__enum_8c.html</anchorfile>
      <anchor>9339b1d283ab686873330bf996b46ba2</anchor>
      <arglist>(LB_TraceRegion *tr)</arglist>
    </member>
    <member kind="function" static="yes">
      <type>static void</type>
      <name>LB_find_tr_dep_height</name>
      <anchorfile>lb__hb__block__enum_8c.html</anchorfile>
      <anchor>ec74f8e8af95e75498ffc2dc5d079501</anchor>
      <arglist>(LB_TraceRegion *tr)</arglist>
    </member>
    <member kind="function" static="yes">
      <type>static void</type>
      <name>LB_find_tr_wcet</name>
      <anchorfile>lb__hb__block__enum_8c.html</anchorfile>
      <anchor>a825f33de4f5c1b1e4a369c90a7f4725</anchor>
      <arglist>(LB_TraceRegion *tr)</arglist>
    </member>
    <member kind="function" static="yes">
      <type>static void</type>
      <name>LB_find_tr_flags</name>
      <anchorfile>lb__hb__block__enum_8c.html</anchorfile>
      <anchor>129860aed1cb3d2aa0056c236393d162</anchor>
      <arglist>(LB_TraceRegion *tr)</arglist>
    </member>
    <member kind="function">
      <type>LB_TraceRegion *</type>
      <name>LB_compute_traceregion_info</name>
      <anchorfile>lb__hb__block__enum_8c.html</anchorfile>
      <anchor>4191be5e2b36d9925e2b04a889b0676b</anchor>
      <arglist>(LB_TraceRegion *tr)</arglist>
    </member>
    <member kind="function" static="yes">
      <type>static void</type>
      <name>LB_set_cb_flags</name>
      <anchorfile>lb__hb__block__enum_8c.html</anchorfile>
      <anchor>677a6e7087a74a879dfc97668b0959af</anchor>
      <arglist>(LB_TraceRegion *tr, Set blocks, int type)</arglist>
    </member>
    <member kind="function" static="yes">
      <type>static int</type>
      <name>LB_compare_traceregion_info</name>
      <anchorfile>lb__hb__block__enum_8c.html</anchorfile>
      <anchor>f811ccf3927366a2cd4a0b392cdc1e71</anchor>
      <arglist>(LB_TraceRegion *selected, LB_TraceRegion *possible)</arglist>
    </member>
    <member kind="function" static="yes">
      <type>static Set</type>
      <name>LB_static_main_path_selector</name>
      <anchorfile>lb__hb__block__enum_8c.html</anchorfile>
      <anchor>491212a2672397beba75160d66d2a4b8</anchor>
      <arglist>(L_Cb *start_cb, L_Cb *end_cb, Set blocks)</arglist>
    </member>
    <member kind="function" static="yes">
      <type>static Set</type>
      <name>LB_dyn_main_path_selector</name>
      <anchorfile>lb__hb__block__enum_8c.html</anchorfile>
      <anchor>09eb46923cf7bb6feb928ca4a723303e</anchor>
      <arglist>(L_Cb *start_cb, L_Cb *end_cb, Set blocks)</arglist>
    </member>
    <member kind="function">
      <type>LB_TraceRegion *</type>
      <name>LB_block_enumeration_selector</name>
      <anchorfile>lb__hb__block__enum_8c.html</anchorfile>
      <anchor>3294721185e7a6ac510e9f6430cb7fde</anchor>
      <arglist>(int type, L_Cb *start_cb, L_Cb *end_cb, Set blocks, int id)</arglist>
    </member>
    <member kind="variable">
      <type>int</type>
      <name>LB_max_dependence_height</name>
      <anchorfile>lb__hb__block__enum_8c.html</anchorfile>
      <anchor>0d1ba2a0c8d9e8c78f2d12549e952d4d</anchor>
      <arglist></arglist>
    </member>
    <member kind="variable">
      <type>int</type>
      <name>LB_region_total_ops</name>
      <anchorfile>lb__hb__block__enum_8c.html</anchorfile>
      <anchor>f1927629053d2604688c220a25efcf63</anchor>
      <arglist></arglist>
    </member>
  </compound>
  <compound kind="file">
    <name>lb_hb_block_enum.h</name>
    <path>/mnt/hgfs/mkt/Desktop/openimpact/src/Lcode/opti/Lblock/</path>
    <filename>lb__hb__block__enum_8h</filename>
    <member kind="function">
      <type>LB_TraceRegion *</type>
      <name>LB_block_enumeration_selector</name>
      <anchorfile>lb__hb__block__enum_8h.html</anchorfile>
      <anchor>83bd553678092e406761998c4444aa91</anchor>
      <arglist>(int, L_Cb *, L_Cb *, Set blocks, int)</arglist>
    </member>
    <member kind="function">
      <type>LB_TraceRegion *</type>
      <name>LB_compute_traceregion_info</name>
      <anchorfile>lb__hb__block__enum_8h.html</anchorfile>
      <anchor>4191be5e2b36d9925e2b04a889b0676b</anchor>
      <arglist>(LB_TraceRegion *tr)</arglist>
    </member>
  </compound>
  <compound kind="file">
    <name>lb_hb_hyperblock.h</name>
    <path>/mnt/hgfs/mkt/Desktop/openimpact/src/Lcode/opti/Lblock/</path>
    <filename>lb__hb__hyperblock_8h</filename>
    <includes id="lb__b__internal_8h" name="lb_b_internal.h" local="yes" imported="no">lb_b_internal.h</includes>
    <includes id="lb__hb__stack_8h" name="lb_hb_stack.h" local="yes" imported="no">lb_hb_stack.h</includes>
    <includes id="lb__hb__path_8h" name="lb_hb_path.h" local="yes" imported="no">lb_hb_path.h</includes>
    <includes id="lb__hb__region__loop_8h" name="lb_hb_region_loop.h" local="yes" imported="no">lb_hb_region_loop.h</includes>
    <includes id="lb__hb__region__hammock_8h" name="lb_hb_region_hammock.h" local="yes" imported="no">lb_hb_region_hammock.h</includes>
    <includes id="lb__hb__region__general_8h" name="lb_hb_region_general.h" local="yes" imported="no">lb_hb_region_general.h</includes>
    <includes id="lb__hb__merge_8h" name="lb_hb_merge.h" local="yes" imported="no">lb_hb_merge.h</includes>
    <includes id="lb__hb__misc_8h" name="lb_hb_misc.h" local="yes" imported="no">lb_hb_misc.h</includes>
    <includes id="lb__hb__block__enum_8h" name="lb_hb_block_enum.h" local="yes" imported="no">lb_hb_block_enum.h</includes>
    <class kind="struct">_LB_HB_Stat</class>
    <member kind="define">
      <type>#define</type>
      <name>LB_HAZARD_UNDEFINED</name>
      <anchorfile>lb__hb__hyperblock_8h.html</anchorfile>
      <anchor>87e807cbff8fcd197e91a16d25a57454</anchor>
      <arglist></arglist>
    </member>
    <member kind="define">
      <type>#define</type>
      <name>LB_HAZARD_EXCLUDE_ALL</name>
      <anchorfile>lb__hb__hyperblock_8h.html</anchorfile>
      <anchor>8c5ed492f0df71427c869040e5f9e1fe</anchor>
      <arglist></arglist>
    </member>
    <member kind="define">
      <type>#define</type>
      <name>LB_HAZARD_EXCLUDE_NON_TRACE</name>
      <anchorfile>lb__hb__hyperblock_8h.html</anchorfile>
      <anchor>bb68cb2e4db422eaa53f7ec27ddb72b0</anchor>
      <arglist></arglist>
    </member>
    <member kind="define">
      <type>#define</type>
      <name>LB_HAZARD_EXCLUDE_HEURISTIC</name>
      <anchorfile>lb__hb__hyperblock_8h.html</anchorfile>
      <anchor>8b060aaf7da37135545223e2ee86c6a2</anchor>
      <arglist></arglist>
    </member>
    <member kind="define">
      <type>#define</type>
      <name>LB_HAZARD_IGNORE</name>
      <anchorfile>lb__hb__hyperblock_8h.html</anchorfile>
      <anchor>4c42f122829f2e2b594e18572b0b33bb</anchor>
      <arglist></arglist>
    </member>
    <member kind="typedef">
      <type>_LB_HB_Stat</type>
      <name>LB_HB_Stat</name>
      <anchorfile>lb__hb__hyperblock_8h.html</anchorfile>
      <anchor>43d6cfe15d46c1fc0e3a1a5fe2db48d6</anchor>
      <arglist></arglist>
    </member>
    <member kind="function">
      <type>L_Loop *</type>
      <name>LB_hb_do_collapse_loops</name>
      <anchorfile>lb__hb__hyperblock_8h.html</anchorfile>
      <anchor>b7061e92465878b60e96119a6b161fb1</anchor>
      <arglist>(LB_TraceRegion_Header *trh, L_Func *fn, L_Loop *loop)</arglist>
    </member>
    <member kind="function">
      <type>int</type>
      <name>M_starcore_subroutine_call</name>
      <anchorfile>lb__hb__hyperblock_8h.html</anchorfile>
      <anchor>a1b9c9dbde5285a993270550fdc2957b</anchor>
      <arglist>(int)</arglist>
    </member>
    <member kind="variable">
      <type>LB_HB_Stat</type>
      <name>LB_hb_stat</name>
      <anchorfile>lb__hb__hyperblock_8h.html</anchorfile>
      <anchor>ec4f4fd241108675eecde27e69c3b870</anchor>
      <arglist></arglist>
    </member>
  </compound>
  <compound kind="file">
    <name>lb_hb_hyperblock_former.c</name>
    <path>/mnt/hgfs/mkt/Desktop/openimpact/src/Lcode/opti/Lblock/</path>
    <filename>lb__hb__hyperblock__former_8c</filename>
    <includes id="lb__hb__hyperblock_8h" name="lb_hb_hyperblock.h" local="yes" imported="no">lb_hb_hyperblock.h</includes>
    <includes id="lb__hb__peel_8h" name="lb_hb_peel.h" local="yes" imported="no">lb_hb_peel.h</includes>
    <class kind="struct">My_path</class>
    <member kind="define">
      <type>#define</type>
      <name>IMPACT_OWN</name>
      <anchorfile>lb__hb__hyperblock__former_8c.html</anchorfile>
      <anchor>70bdccae7e262afab54dc63b411c0f63</anchor>
      <arglist></arglist>
    </member>
    <member kind="function">
      <type>L_Cb *</type>
      <name>My_LB_hb_find_immediate_dominator</name>
      <anchorfile>lb__hb__hyperblock__former_8c.html</anchorfile>
      <anchor>ba5e94764821766eef2dad8a7b5c2954</anchor>
      <arglist>(L_Cb *cb)</arglist>
    </member>
    <member kind="function">
      <type>GraphNode</type>
      <name>get_imm_post_domin</name>
      <anchorfile>lb__hb__hyperblock__former_8c.html</anchorfile>
      <anchor>7d147e43a42ea1ecae68c2fe71889791</anchor>
      <arglist>(List all_nodes, GraphNode node)</arglist>
    </member>
    <member kind="function">
      <type>void</type>
      <name>List_set_postion</name>
      <anchorfile>lb__hb__hyperblock__former_8c.html</anchorfile>
      <anchor>08a697e6d488c28411a5aa573335eade</anchor>
      <arglist>(List list, void *ptr)</arglist>
    </member>
    <member kind="function">
      <type>void</type>
      <name>is_simple_2</name>
      <anchorfile>lb__hb__hyperblock__former_8c.html</anchorfile>
      <anchor>9db20ed991a18c8274b855405e14e024</anchor>
      <arglist>(List all_nodes, GraphNode first_node, GraphNode last_node)</arglist>
    </member>
    <member kind="function">
      <type>void</type>
      <name>find_simples</name>
      <anchorfile>lb__hb__hyperblock__former_8c.html</anchorfile>
      <anchor>cb361e91b106715a61752ba05e5fa267</anchor>
      <arglist>(Graph g)</arglist>
    </member>
    <member kind="function">
      <type>Set</type>
      <name>find_set_backedges</name>
      <anchorfile>lb__hb__hyperblock__former_8c.html</anchorfile>
      <anchor>af6f9447e8e738b534d2dff7f24b0c2b</anchor>
      <arglist>(L_Func *fn)</arglist>
    </member>
    <member kind="function">
      <type>Set</type>
      <name>find_set_headers</name>
      <anchorfile>lb__hb__hyperblock__former_8c.html</anchorfile>
      <anchor>8f162265f39768f943151800f05df1f4</anchor>
      <arglist>(L_Func *fn)</arglist>
    </member>
    <member kind="function">
      <type>LB_TraceRegion *</type>
      <name>My_simple_trace_formation</name>
      <anchorfile>lb__hb__hyperblock__former_8c.html</anchorfile>
      <anchor>cc7ccbc550ac834847b6e63db76962f5</anchor>
      <arglist>(L_Func *fn, LB_TraceRegion_Header *header, int type, L_Cb *start, L_Cb *end, Set all_blocks, int id)</arglist>
    </member>
    <member kind="function">
      <type>void</type>
      <name>my_trace_formation</name>
      <anchorfile>lb__hb__hyperblock__former_8c.html</anchorfile>
      <anchor>4aed782c68da524e8d2374a0be17ba88</anchor>
      <arglist>(L_Func *fn, LB_TraceRegion_Header *header, int c)</arglist>
    </member>
    <member kind="function">
      <type>void</type>
      <name>visit_2</name>
      <anchorfile>lb__hb__hyperblock__former_8c.html</anchorfile>
      <anchor>5507531643302c3e9ac8f554d7e74290</anchor>
      <arglist>(L_Cb *node, L_Cb *first_node, L_Cb *last_node, Set blocks, LB_TraceRegion_Header *header, double wcet)</arglist>
    </member>
    <member kind="function">
      <type>void</type>
      <name>visit</name>
      <anchorfile>lb__hb__hyperblock__former_8c.html</anchorfile>
      <anchor>44e4c1e8df0579c85c416544546ac711</anchor>
      <arglist>(L_Cb *node, L_Cb *first_node, L_Cb *last_node, Set blocks, LB_TraceRegion_Header *header, double wcet)</arglist>
    </member>
    <member kind="function">
      <type>double</type>
      <name>find_max_weight_of_paths</name>
      <anchorfile>lb__hb__hyperblock__former_8c.html</anchorfile>
      <anchor>56b91c08381368547c77582fb1cb4fdc</anchor>
      <arglist>(L_Cb *first, L_Cb *last)</arglist>
    </member>
    <member kind="function">
      <type>void</type>
      <name>my_trace_formation2_2</name>
      <anchorfile>lb__hb__hyperblock__former_8c.html</anchorfile>
      <anchor>cf108d1888f02a824668812b7aae7b10</anchor>
      <arglist>(L_Func *fn, LB_TraceRegion_Header *header)</arglist>
    </member>
    <member kind="function">
      <type>void</type>
      <name>my_trace_formation2</name>
      <anchorfile>lb__hb__hyperblock__former_8c.html</anchorfile>
      <anchor>4fd7bc82592f406e4d65c540fa30c524</anchor>
      <arglist>(L_Func *fn, LB_TraceRegion_Header *header)</arglist>
    </member>
    <member kind="function">
      <type>int</type>
      <name>my_select_trace</name>
      <anchorfile>lb__hb__hyperblock__former_8c.html</anchorfile>
      <anchor>d1f94c0fa903cb77f1219a644c3d941f</anchor>
      <arglist>(L_Func *fn, LB_TraceRegion_Header *header)</arglist>
    </member>
    <member kind="function">
      <type>double</type>
      <name>find_wcet_path</name>
      <anchorfile>lb__hb__hyperblock__former_8c.html</anchorfile>
      <anchor>fcc7376cf2f5cda21e50416033948de7</anchor>
      <arglist>(L_Cb *start, L_Cb *end)</arglist>
    </member>
    <member kind="function">
      <type>int</type>
      <name>is_balance</name>
      <anchorfile>lb__hb__hyperblock__former_8c.html</anchorfile>
      <anchor>e1e6a3a842abc71a3b4972f9a7c9ba9f</anchor>
      <arglist>(L_Cb *node)</arglist>
    </member>
    <member kind="function">
      <type>void</type>
      <name>find_loops_wcet</name>
      <anchorfile>lb__hb__hyperblock__former_8c.html</anchorfile>
      <anchor>9c6dc267699d04bd4a65943d4e82653d</anchor>
      <arglist>(L_Func *fn)</arglist>
    </member>
    <member kind="function">
      <type>void</type>
      <name>my_select_trace_2</name>
      <anchorfile>lb__hb__hyperblock__former_8c.html</anchorfile>
      <anchor>93fdf81f1167175300bb9a0e3afd8492</anchor>
      <arglist>(L_Func *fn, LB_TraceRegion_Header *header)</arglist>
    </member>
    <member kind="function">
      <type>int</type>
      <name>find_chock</name>
      <anchorfile>lb__hb__hyperblock__former_8c.html</anchorfile>
      <anchor>279036c0318836dbda45431882dddf17</anchor>
      <arglist>(L_Cb *start, L_Cb *end, L_Cb *current, Set blocks, LB_TraceRegion_Header *header)</arglist>
    </member>
    <member kind="function">
      <type>void</type>
      <name>my_trace_formation3</name>
      <anchorfile>lb__hb__hyperblock__former_8c.html</anchorfile>
      <anchor>64520594559940f97cf19c44b94daaf3</anchor>
      <arglist>(L_Func *fn, LB_TraceRegion_Header *header)</arglist>
    </member>
    <member kind="function">
      <type>double</type>
      <name>my_find_special_chocks</name>
      <anchorfile>lb__hb__hyperblock__former_8c.html</anchorfile>
      <anchor>7e8842c7e51f696110c20f4e1d563d98</anchor>
      <arglist>(L_Cb *start, L_Cb *end, L_Cb *current, Set blocks, LB_TraceRegion_Header *header, double wcet)</arglist>
    </member>
    <member kind="function">
      <type>void</type>
      <name>my_pre_path_hb_formation</name>
      <anchorfile>lb__hb__hyperblock__former_8c.html</anchorfile>
      <anchor>0f45885cb4f44d34e793aa77d293e72c</anchor>
      <arglist>(L_Func *fn, LB_TraceRegion_Header *header)</arglist>
    </member>
    <member kind="function">
      <type>void</type>
      <name>LB_split_exit_block</name>
      <anchorfile>lb__hb__hyperblock__former_8c.html</anchorfile>
      <anchor>aef1a11858642b0af294288535e00338</anchor>
      <arglist>(L_Func *fn)</arglist>
    </member>
    <member kind="function" static="yes">
      <type>static int</type>
      <name>LB_mark_ncycle_regions</name>
      <anchorfile>lb__hb__hyperblock__former_8c.html</anchorfile>
      <anchor>699ee86287c6a8fe26c974967625f0eb</anchor>
      <arglist>(LB_TraceRegion_Header *header)</arglist>
    </member>
    <member kind="function">
      <type>void</type>
      <name>LB_hyperblock_formation</name>
      <anchorfile>lb__hb__hyperblock__former_8c.html</anchorfile>
      <anchor>f619b8ef2e26dc7e82f60fec25f2f79b</anchor>
      <arglist>(L_Func *fn)</arglist>
    </member>
    <member kind="variable">
      <type>LB_HB_Stat</type>
      <name>LB_hb_stat</name>
      <anchorfile>lb__hb__hyperblock__former_8c.html</anchorfile>
      <anchor>ec4f4fd241108675eecde27e69c3b870</anchor>
      <arglist></arglist>
    </member>
    <member kind="variable">
      <type>double</type>
      <name>Max_exec</name>
      <anchorfile>lb__hb__hyperblock__former_8c.html</anchorfile>
      <anchor>8215c89a792f6fb3cafdbad9afb65928</anchor>
      <arglist></arglist>
    </member>
    <member kind="variable">
      <type>Set</type>
      <name>backedge</name>
      <anchorfile>lb__hb__hyperblock__former_8c.html</anchorfile>
      <anchor>9fd9d9b51e5d60fb16b316233d9cdb92</anchor>
      <arglist></arglist>
    </member>
    <member kind="variable">
      <type>Set</type>
      <name>headers</name>
      <anchorfile>lb__hb__hyperblock__former_8c.html</anchorfile>
      <anchor>d9495e4528bbc993a1a1b94d89672a2d</anchor>
      <arglist></arglist>
    </member>
  </compound>
  <compound kind="file">
    <name>lb_hb_hyperblock_former_full.c</name>
    <path>/mnt/hgfs/mkt/Desktop/openimpact/src/Lcode/opti/Lblock/</path>
    <filename>lb__hb__hyperblock__former__full_8c</filename>
    <includes id="lb__hb__hyperblock_8h" name="lb_hb_hyperblock.h" local="yes" imported="no">lb_hb_hyperblock.h</includes>
    <includes id="lb__hb__peel_8h" name="lb_hb_peel.h" local="yes" imported="no">lb_hb_peel.h</includes>
    <class kind="struct">My_path</class>
    <member kind="define">
      <type>#define</type>
      <name>IMPACT_OWN</name>
      <anchorfile>lb__hb__hyperblock__former__full_8c.html</anchorfile>
      <anchor>70bdccae7e262afab54dc63b411c0f63</anchor>
      <arglist></arglist>
    </member>
    <member kind="function">
      <type>L_Cb *</type>
      <name>My_LB_hb_find_immediate_dominator</name>
      <anchorfile>lb__hb__hyperblock__former__full_8c.html</anchorfile>
      <anchor>ba5e94764821766eef2dad8a7b5c2954</anchor>
      <arglist>(L_Cb *cb)</arglist>
    </member>
    <member kind="function">
      <type>GraphNode</type>
      <name>get_imm_post_domin</name>
      <anchorfile>lb__hb__hyperblock__former__full_8c.html</anchorfile>
      <anchor>7d147e43a42ea1ecae68c2fe71889791</anchor>
      <arglist>(List all_nodes, GraphNode node)</arglist>
    </member>
    <member kind="function">
      <type>void</type>
      <name>List_set_postion</name>
      <anchorfile>lb__hb__hyperblock__former__full_8c.html</anchorfile>
      <anchor>08a697e6d488c28411a5aa573335eade</anchor>
      <arglist>(List list, void *ptr)</arglist>
    </member>
    <member kind="function">
      <type>void</type>
      <name>is_simple_2</name>
      <anchorfile>lb__hb__hyperblock__former__full_8c.html</anchorfile>
      <anchor>9db20ed991a18c8274b855405e14e024</anchor>
      <arglist>(List all_nodes, GraphNode first_node, GraphNode last_node)</arglist>
    </member>
    <member kind="function">
      <type>void</type>
      <name>find_simples</name>
      <anchorfile>lb__hb__hyperblock__former__full_8c.html</anchorfile>
      <anchor>cb361e91b106715a61752ba05e5fa267</anchor>
      <arglist>(Graph g)</arglist>
    </member>
    <member kind="function">
      <type>Set</type>
      <name>find_set_backedges</name>
      <anchorfile>lb__hb__hyperblock__former__full_8c.html</anchorfile>
      <anchor>af6f9447e8e738b534d2dff7f24b0c2b</anchor>
      <arglist>(L_Func *fn)</arglist>
    </member>
    <member kind="function">
      <type>Set</type>
      <name>find_set_headers</name>
      <anchorfile>lb__hb__hyperblock__former__full_8c.html</anchorfile>
      <anchor>8f162265f39768f943151800f05df1f4</anchor>
      <arglist>(L_Func *fn)</arglist>
    </member>
    <member kind="function">
      <type>LB_TraceRegion *</type>
      <name>My_simple_trace_formation</name>
      <anchorfile>lb__hb__hyperblock__former__full_8c.html</anchorfile>
      <anchor>cc7ccbc550ac834847b6e63db76962f5</anchor>
      <arglist>(L_Func *fn, LB_TraceRegion_Header *header, int type, L_Cb *start, L_Cb *end, Set all_blocks, int id)</arglist>
    </member>
    <member kind="function">
      <type>void</type>
      <name>my_trace_formation</name>
      <anchorfile>lb__hb__hyperblock__former__full_8c.html</anchorfile>
      <anchor>4aed782c68da524e8d2374a0be17ba88</anchor>
      <arglist>(L_Func *fn, LB_TraceRegion_Header *header, int c)</arglist>
    </member>
    <member kind="function">
      <type>void</type>
      <name>visit_2</name>
      <anchorfile>lb__hb__hyperblock__former__full_8c.html</anchorfile>
      <anchor>5507531643302c3e9ac8f554d7e74290</anchor>
      <arglist>(L_Cb *node, L_Cb *first_node, L_Cb *last_node, Set blocks, LB_TraceRegion_Header *header, double wcet)</arglist>
    </member>
    <member kind="function">
      <type>void</type>
      <name>visit</name>
      <anchorfile>lb__hb__hyperblock__former__full_8c.html</anchorfile>
      <anchor>44e4c1e8df0579c85c416544546ac711</anchor>
      <arglist>(L_Cb *node, L_Cb *first_node, L_Cb *last_node, Set blocks, LB_TraceRegion_Header *header, double wcet)</arglist>
    </member>
    <member kind="function">
      <type>double</type>
      <name>find_max_weight_of_paths</name>
      <anchorfile>lb__hb__hyperblock__former__full_8c.html</anchorfile>
      <anchor>56b91c08381368547c77582fb1cb4fdc</anchor>
      <arglist>(L_Cb *first, L_Cb *last)</arglist>
    </member>
    <member kind="function">
      <type>void</type>
      <name>my_trace_formation2_2</name>
      <anchorfile>lb__hb__hyperblock__former__full_8c.html</anchorfile>
      <anchor>cf108d1888f02a824668812b7aae7b10</anchor>
      <arglist>(L_Func *fn, LB_TraceRegion_Header *header)</arglist>
    </member>
    <member kind="function">
      <type>void</type>
      <name>my_trace_formation2</name>
      <anchorfile>lb__hb__hyperblock__former__full_8c.html</anchorfile>
      <anchor>4fd7bc82592f406e4d65c540fa30c524</anchor>
      <arglist>(L_Func *fn, LB_TraceRegion_Header *header)</arglist>
    </member>
    <member kind="function">
      <type>int</type>
      <name>my_select_trace</name>
      <anchorfile>lb__hb__hyperblock__former__full_8c.html</anchorfile>
      <anchor>d1f94c0fa903cb77f1219a644c3d941f</anchor>
      <arglist>(L_Func *fn, LB_TraceRegion_Header *header)</arglist>
    </member>
    <member kind="function">
      <type>double</type>
      <name>find_wcet_path</name>
      <anchorfile>lb__hb__hyperblock__former__full_8c.html</anchorfile>
      <anchor>fcc7376cf2f5cda21e50416033948de7</anchor>
      <arglist>(L_Cb *start, L_Cb *end)</arglist>
    </member>
    <member kind="function">
      <type>int</type>
      <name>is_balance</name>
      <anchorfile>lb__hb__hyperblock__former__full_8c.html</anchorfile>
      <anchor>e1e6a3a842abc71a3b4972f9a7c9ba9f</anchor>
      <arglist>(L_Cb *node)</arglist>
    </member>
    <member kind="function">
      <type>void</type>
      <name>find_loops_wcet</name>
      <anchorfile>lb__hb__hyperblock__former__full_8c.html</anchorfile>
      <anchor>9c6dc267699d04bd4a65943d4e82653d</anchor>
      <arglist>(L_Func *fn)</arglist>
    </member>
    <member kind="function">
      <type>void</type>
      <name>my_select_trace_2</name>
      <anchorfile>lb__hb__hyperblock__former__full_8c.html</anchorfile>
      <anchor>93fdf81f1167175300bb9a0e3afd8492</anchor>
      <arglist>(L_Func *fn, LB_TraceRegion_Header *header)</arglist>
    </member>
    <member kind="function">
      <type>int</type>
      <name>find_chock</name>
      <anchorfile>lb__hb__hyperblock__former__full_8c.html</anchorfile>
      <anchor>279036c0318836dbda45431882dddf17</anchor>
      <arglist>(L_Cb *start, L_Cb *end, L_Cb *current, Set blocks, LB_TraceRegion_Header *header)</arglist>
    </member>
    <member kind="function">
      <type>void</type>
      <name>my_trace_formation3</name>
      <anchorfile>lb__hb__hyperblock__former__full_8c.html</anchorfile>
      <anchor>64520594559940f97cf19c44b94daaf3</anchor>
      <arglist>(L_Func *fn, LB_TraceRegion_Header *header)</arglist>
    </member>
    <member kind="function">
      <type>double</type>
      <name>my_find_special_chocks</name>
      <anchorfile>lb__hb__hyperblock__former__full_8c.html</anchorfile>
      <anchor>7e8842c7e51f696110c20f4e1d563d98</anchor>
      <arglist>(L_Cb *start, L_Cb *end, L_Cb *current, Set blocks, LB_TraceRegion_Header *header, double wcet)</arglist>
    </member>
    <member kind="function">
      <type>void</type>
      <name>my_pre_path_hb_formation</name>
      <anchorfile>lb__hb__hyperblock__former__full_8c.html</anchorfile>
      <anchor>0f45885cb4f44d34e793aa77d293e72c</anchor>
      <arglist>(L_Func *fn, LB_TraceRegion_Header *header)</arglist>
    </member>
    <member kind="function">
      <type>void</type>
      <name>LB_split_exit_block</name>
      <anchorfile>lb__hb__hyperblock__former__full_8c.html</anchorfile>
      <anchor>aef1a11858642b0af294288535e00338</anchor>
      <arglist>(L_Func *fn)</arglist>
    </member>
    <member kind="function" static="yes">
      <type>static int</type>
      <name>LB_mark_ncycle_regions</name>
      <anchorfile>lb__hb__hyperblock__former__full_8c.html</anchorfile>
      <anchor>699ee86287c6a8fe26c974967625f0eb</anchor>
      <arglist>(LB_TraceRegion_Header *header)</arglist>
    </member>
    <member kind="function">
      <type>void</type>
      <name>LB_hyperblock_formation</name>
      <anchorfile>lb__hb__hyperblock__former__full_8c.html</anchorfile>
      <anchor>f619b8ef2e26dc7e82f60fec25f2f79b</anchor>
      <arglist>(L_Func *fn)</arglist>
    </member>
    <member kind="variable">
      <type>LB_HB_Stat</type>
      <name>LB_hb_stat</name>
      <anchorfile>lb__hb__hyperblock__former__full_8c.html</anchorfile>
      <anchor>ec4f4fd241108675eecde27e69c3b870</anchor>
      <arglist></arglist>
    </member>
    <member kind="variable">
      <type>double</type>
      <name>Max_exec</name>
      <anchorfile>lb__hb__hyperblock__former__full_8c.html</anchorfile>
      <anchor>8215c89a792f6fb3cafdbad9afb65928</anchor>
      <arglist></arglist>
    </member>
    <member kind="variable">
      <type>Set</type>
      <name>backedge</name>
      <anchorfile>lb__hb__hyperblock__former__full_8c.html</anchorfile>
      <anchor>9fd9d9b51e5d60fb16b316233d9cdb92</anchor>
      <arglist></arglist>
    </member>
    <member kind="variable">
      <type>Set</type>
      <name>headers</name>
      <anchorfile>lb__hb__hyperblock__former__full_8c.html</anchorfile>
      <anchor>d9495e4528bbc993a1a1b94d89672a2d</anchor>
      <arglist></arglist>
    </member>
  </compound>
  <compound kind="file">
    <name>lb_hb_hyperblock_former_WCET.c</name>
    <path>/mnt/hgfs/mkt/Desktop/openimpact/src/Lcode/opti/Lblock/</path>
    <filename>lb__hb__hyperblock__former__WCET_8c</filename>
    <includes id="lb__hb__hyperblock_8h" name="lb_hb_hyperblock.h" local="yes" imported="no">lb_hb_hyperblock.h</includes>
    <includes id="lb__hb__peel_8h" name="lb_hb_peel.h" local="yes" imported="no">lb_hb_peel.h</includes>
    <class kind="struct">My_path</class>
    <member kind="define">
      <type>#define</type>
      <name>IMPACT_OWN</name>
      <anchorfile>lb__hb__hyperblock__former__WCET_8c.html</anchorfile>
      <anchor>70bdccae7e262afab54dc63b411c0f63</anchor>
      <arglist></arglist>
    </member>
    <member kind="function">
      <type>L_Cb *</type>
      <name>My_LB_hb_find_immediate_dominator</name>
      <anchorfile>lb__hb__hyperblock__former__WCET_8c.html</anchorfile>
      <anchor>ba5e94764821766eef2dad8a7b5c2954</anchor>
      <arglist>(L_Cb *cb)</arglist>
    </member>
    <member kind="function">
      <type>GraphNode</type>
      <name>get_imm_post_domin</name>
      <anchorfile>lb__hb__hyperblock__former__WCET_8c.html</anchorfile>
      <anchor>7d147e43a42ea1ecae68c2fe71889791</anchor>
      <arglist>(List all_nodes, GraphNode node)</arglist>
    </member>
    <member kind="function">
      <type>void</type>
      <name>List_set_postion</name>
      <anchorfile>lb__hb__hyperblock__former__WCET_8c.html</anchorfile>
      <anchor>08a697e6d488c28411a5aa573335eade</anchor>
      <arglist>(List list, void *ptr)</arglist>
    </member>
    <member kind="function">
      <type>void</type>
      <name>is_simple_2</name>
      <anchorfile>lb__hb__hyperblock__former__WCET_8c.html</anchorfile>
      <anchor>9db20ed991a18c8274b855405e14e024</anchor>
      <arglist>(List all_nodes, GraphNode first_node, GraphNode last_node)</arglist>
    </member>
    <member kind="function">
      <type>void</type>
      <name>find_simples</name>
      <anchorfile>lb__hb__hyperblock__former__WCET_8c.html</anchorfile>
      <anchor>cb361e91b106715a61752ba05e5fa267</anchor>
      <arglist>(Graph g)</arglist>
    </member>
    <member kind="function">
      <type>Set</type>
      <name>find_set_backedges</name>
      <anchorfile>lb__hb__hyperblock__former__WCET_8c.html</anchorfile>
      <anchor>af6f9447e8e738b534d2dff7f24b0c2b</anchor>
      <arglist>(L_Func *fn)</arglist>
    </member>
    <member kind="function">
      <type>Set</type>
      <name>find_set_headers</name>
      <anchorfile>lb__hb__hyperblock__former__WCET_8c.html</anchorfile>
      <anchor>8f162265f39768f943151800f05df1f4</anchor>
      <arglist>(L_Func *fn)</arglist>
    </member>
    <member kind="function">
      <type>LB_TraceRegion *</type>
      <name>My_simple_trace_formation</name>
      <anchorfile>lb__hb__hyperblock__former__WCET_8c.html</anchorfile>
      <anchor>cc7ccbc550ac834847b6e63db76962f5</anchor>
      <arglist>(L_Func *fn, LB_TraceRegion_Header *header, int type, L_Cb *start, L_Cb *end, Set all_blocks, int id)</arglist>
    </member>
    <member kind="function">
      <type>void</type>
      <name>my_trace_formation</name>
      <anchorfile>lb__hb__hyperblock__former__WCET_8c.html</anchorfile>
      <anchor>4aed782c68da524e8d2374a0be17ba88</anchor>
      <arglist>(L_Func *fn, LB_TraceRegion_Header *header, int c)</arglist>
    </member>
    <member kind="function">
      <type>void</type>
      <name>visit_2</name>
      <anchorfile>lb__hb__hyperblock__former__WCET_8c.html</anchorfile>
      <anchor>5507531643302c3e9ac8f554d7e74290</anchor>
      <arglist>(L_Cb *node, L_Cb *first_node, L_Cb *last_node, Set blocks, LB_TraceRegion_Header *header, double wcet)</arglist>
    </member>
    <member kind="function">
      <type>void</type>
      <name>visit</name>
      <anchorfile>lb__hb__hyperblock__former__WCET_8c.html</anchorfile>
      <anchor>44e4c1e8df0579c85c416544546ac711</anchor>
      <arglist>(L_Cb *node, L_Cb *first_node, L_Cb *last_node, Set blocks, LB_TraceRegion_Header *header, double wcet)</arglist>
    </member>
    <member kind="function">
      <type>double</type>
      <name>find_max_weight_of_paths</name>
      <anchorfile>lb__hb__hyperblock__former__WCET_8c.html</anchorfile>
      <anchor>56b91c08381368547c77582fb1cb4fdc</anchor>
      <arglist>(L_Cb *first, L_Cb *last)</arglist>
    </member>
    <member kind="function">
      <type>void</type>
      <name>my_trace_formation2_2</name>
      <anchorfile>lb__hb__hyperblock__former__WCET_8c.html</anchorfile>
      <anchor>cf108d1888f02a824668812b7aae7b10</anchor>
      <arglist>(L_Func *fn, LB_TraceRegion_Header *header)</arglist>
    </member>
    <member kind="function">
      <type>void</type>
      <name>my_trace_formation2</name>
      <anchorfile>lb__hb__hyperblock__former__WCET_8c.html</anchorfile>
      <anchor>4fd7bc82592f406e4d65c540fa30c524</anchor>
      <arglist>(L_Func *fn, LB_TraceRegion_Header *header)</arglist>
    </member>
    <member kind="function">
      <type>int</type>
      <name>my_select_trace</name>
      <anchorfile>lb__hb__hyperblock__former__WCET_8c.html</anchorfile>
      <anchor>d1f94c0fa903cb77f1219a644c3d941f</anchor>
      <arglist>(L_Func *fn, LB_TraceRegion_Header *header)</arglist>
    </member>
    <member kind="function">
      <type>double</type>
      <name>find_wcet_path</name>
      <anchorfile>lb__hb__hyperblock__former__WCET_8c.html</anchorfile>
      <anchor>fcc7376cf2f5cda21e50416033948de7</anchor>
      <arglist>(L_Cb *start, L_Cb *end)</arglist>
    </member>
    <member kind="function">
      <type>int</type>
      <name>is_balance</name>
      <anchorfile>lb__hb__hyperblock__former__WCET_8c.html</anchorfile>
      <anchor>e1e6a3a842abc71a3b4972f9a7c9ba9f</anchor>
      <arglist>(L_Cb *node)</arglist>
    </member>
    <member kind="function">
      <type>void</type>
      <name>find_loops_wcet</name>
      <anchorfile>lb__hb__hyperblock__former__WCET_8c.html</anchorfile>
      <anchor>9c6dc267699d04bd4a65943d4e82653d</anchor>
      <arglist>(L_Func *fn)</arglist>
    </member>
    <member kind="function">
      <type>void</type>
      <name>my_select_trace_2</name>
      <anchorfile>lb__hb__hyperblock__former__WCET_8c.html</anchorfile>
      <anchor>93fdf81f1167175300bb9a0e3afd8492</anchor>
      <arglist>(L_Func *fn, LB_TraceRegion_Header *header)</arglist>
    </member>
    <member kind="function">
      <type>int</type>
      <name>find_chock</name>
      <anchorfile>lb__hb__hyperblock__former__WCET_8c.html</anchorfile>
      <anchor>279036c0318836dbda45431882dddf17</anchor>
      <arglist>(L_Cb *start, L_Cb *end, L_Cb *current, Set blocks, LB_TraceRegion_Header *header)</arglist>
    </member>
    <member kind="function">
      <type>void</type>
      <name>my_trace_formation3</name>
      <anchorfile>lb__hb__hyperblock__former__WCET_8c.html</anchorfile>
      <anchor>64520594559940f97cf19c44b94daaf3</anchor>
      <arglist>(L_Func *fn, LB_TraceRegion_Header *header)</arglist>
    </member>
    <member kind="function">
      <type>double</type>
      <name>my_find_special_chocks</name>
      <anchorfile>lb__hb__hyperblock__former__WCET_8c.html</anchorfile>
      <anchor>7e8842c7e51f696110c20f4e1d563d98</anchor>
      <arglist>(L_Cb *start, L_Cb *end, L_Cb *current, Set blocks, LB_TraceRegion_Header *header, double wcet)</arglist>
    </member>
    <member kind="function">
      <type>void</type>
      <name>my_pre_path_hb_formation</name>
      <anchorfile>lb__hb__hyperblock__former__WCET_8c.html</anchorfile>
      <anchor>0f45885cb4f44d34e793aa77d293e72c</anchor>
      <arglist>(L_Func *fn, LB_TraceRegion_Header *header)</arglist>
    </member>
    <member kind="function">
      <type>void</type>
      <name>LB_split_exit_block</name>
      <anchorfile>lb__hb__hyperblock__former__WCET_8c.html</anchorfile>
      <anchor>aef1a11858642b0af294288535e00338</anchor>
      <arglist>(L_Func *fn)</arglist>
    </member>
    <member kind="function" static="yes">
      <type>static int</type>
      <name>LB_mark_ncycle_regions</name>
      <anchorfile>lb__hb__hyperblock__former__WCET_8c.html</anchorfile>
      <anchor>699ee86287c6a8fe26c974967625f0eb</anchor>
      <arglist>(LB_TraceRegion_Header *header)</arglist>
    </member>
    <member kind="function">
      <type>void</type>
      <name>LB_hyperblock_formation</name>
      <anchorfile>lb__hb__hyperblock__former__WCET_8c.html</anchorfile>
      <anchor>f619b8ef2e26dc7e82f60fec25f2f79b</anchor>
      <arglist>(L_Func *fn)</arglist>
    </member>
    <member kind="variable">
      <type>LB_HB_Stat</type>
      <name>LB_hb_stat</name>
      <anchorfile>lb__hb__hyperblock__former__WCET_8c.html</anchorfile>
      <anchor>ec4f4fd241108675eecde27e69c3b870</anchor>
      <arglist></arglist>
    </member>
    <member kind="variable">
      <type>double</type>
      <name>Max_exec</name>
      <anchorfile>lb__hb__hyperblock__former__WCET_8c.html</anchorfile>
      <anchor>8215c89a792f6fb3cafdbad9afb65928</anchor>
      <arglist></arglist>
    </member>
    <member kind="variable">
      <type>Set</type>
      <name>backedge</name>
      <anchorfile>lb__hb__hyperblock__former__WCET_8c.html</anchorfile>
      <anchor>9fd9d9b51e5d60fb16b316233d9cdb92</anchor>
      <arglist></arglist>
    </member>
    <member kind="variable">
      <type>Set</type>
      <name>headers</name>
      <anchorfile>lb__hb__hyperblock__former__WCET_8c.html</anchorfile>
      <anchor>d9495e4528bbc993a1a1b94d89672a2d</anchor>
      <arglist></arglist>
    </member>
  </compound>
  <compound kind="file">
    <name>lb_hb_merge.c</name>
    <path>/mnt/hgfs/mkt/Desktop/openimpact/src/Lcode/opti/Lblock/</path>
    <filename>lb__hb__merge_8c</filename>
    <includes id="lb__hb__hyperblock_8h" name="lb_hb_hyperblock.h" local="yes" imported="no">lb_hb_hyperblock.h</includes>
    <member kind="define">
      <type>#define</type>
      <name>ERR</name>
      <anchorfile>lb__hb__merge_8c.html</anchorfile>
      <anchor>735563036dced0b7d6cc98f97ea4978b</anchor>
      <arglist></arglist>
    </member>
    <member kind="define">
      <type>#define</type>
      <name>DO_MERGES</name>
      <anchorfile>lb__hb__merge_8c.html</anchorfile>
      <anchor>c47f59436c66aa06d921c6e4ae725a59</anchor>
      <arglist></arglist>
    </member>
    <member kind="define">
      <type>#define</type>
      <name>DO_MERGE1</name>
      <anchorfile>lb__hb__merge_8c.html</anchorfile>
      <anchor>837dae011bd81ab4238f2d8978129455</anchor>
      <arglist></arglist>
    </member>
    <member kind="define">
      <type>#define</type>
      <name>DO_MERGE2</name>
      <anchorfile>lb__hb__merge_8c.html</anchorfile>
      <anchor>dc9c5ccff7bdaa8e281382e2f7926ee1</anchor>
      <arglist></arglist>
    </member>
    <member kind="define">
      <type>#define</type>
      <name>DO_MERGE3</name>
      <anchorfile>lb__hb__merge_8c.html</anchorfile>
      <anchor>9cf136fd1d8cb304f1729b16e2276a1b</anchor>
      <arglist></arglist>
    </member>
    <member kind="define">
      <type>#define</type>
      <name>DO_MERGE4</name>
      <anchorfile>lb__hb__merge_8c.html</anchorfile>
      <anchor>66ad453e0435b8f1bea78079eef81895</anchor>
      <arglist></arglist>
    </member>
    <member kind="define">
      <type>#define</type>
      <name>DO_MIGRATION_MERGE</name>
      <anchorfile>lb__hb__merge_8c.html</anchorfile>
      <anchor>ca4ac9034b37f3946b3f0bbe51ccb769</anchor>
      <arglist></arglist>
    </member>
    <member kind="define">
      <type>#define</type>
      <name>DEBUG_MERGE</name>
      <anchorfile>lb__hb__merge_8c.html</anchorfile>
      <anchor>75bf61a518e79a927905d9cf42e08d8d</anchor>
      <arglist></arglist>
    </member>
    <member kind="define">
      <type>#define</type>
      <name>L_MAX_ITERATION</name>
      <anchorfile>lb__hb__merge_8c.html</anchorfile>
      <anchor>b35c51a682e662e82b71adde15e459d7</anchor>
      <arglist></arglist>
    </member>
    <member kind="function" static="yes">
      <type>static int</type>
      <name>LB_hb_do_pred_merge1</name>
      <anchorfile>lb__hb__merge_8c.html</anchorfile>
      <anchor>f2c61874d0165b0a53b8df6398a4e902</anchor>
      <arglist>(L_Cb *cb)</arglist>
    </member>
    <member kind="function" static="yes">
      <type>static int</type>
      <name>LB_hb_do_pred_merge2</name>
      <anchorfile>lb__hb__merge_8c.html</anchorfile>
      <anchor>c5001b9ac02d5da5e0f74fe671000048</anchor>
      <arglist>(L_Cb *cb)</arglist>
    </member>
    <member kind="function" static="yes">
      <type>static int</type>
      <name>LB_hb_do_pred_merge3</name>
      <anchorfile>lb__hb__merge_8c.html</anchorfile>
      <anchor>d3eac9817b58542f8e4adf5678f87fee</anchor>
      <arglist>(L_Cb *cb)</arglist>
    </member>
    <member kind="function" static="yes">
      <type>static int</type>
      <name>LB_hb_do_pred_merge4</name>
      <anchorfile>lb__hb__merge_8c.html</anchorfile>
      <anchor>66317bfc5a6c32cbfbf06a45dc4ed5cb</anchor>
      <arglist>(L_Cb *cb)</arglist>
    </member>
    <member kind="function" static="yes">
      <type>static int</type>
      <name>LB_hb_do_migration_merging</name>
      <anchorfile>lb__hb__merge_8c.html</anchorfile>
      <anchor>99f0b6b965f1952f27d6f8356ebda0dc</anchor>
      <arglist>(L_Cb *cb)</arglist>
    </member>
    <member kind="function" static="yes">
      <type>static int</type>
      <name>L_can_merge_mems</name>
      <anchorfile>lb__hb__merge_8c.html</anchorfile>
      <anchor>ffaaebad0003008937508063ff6f646d</anchor>
      <arglist>(L_Cb *cb, L_Oper *opA, L_Oper *opB)</arglist>
    </member>
    <member kind="function">
      <type>void</type>
      <name>LB_hb_pred_merging</name>
      <anchorfile>lb__hb__merge_8c.html</anchorfile>
      <anchor>ba970c2c53d05742fe7b1a94a9c3f95a</anchor>
      <arglist>(L_Func *fn)</arglist>
    </member>
  </compound>
  <compound kind="file">
    <name>lb_hb_merge.h</name>
    <path>/mnt/hgfs/mkt/Desktop/openimpact/src/Lcode/opti/Lblock/</path>
    <filename>lb__hb__merge_8h</filename>
    <member kind="function">
      <type>void</type>
      <name>LB_hb_pred_merging</name>
      <anchorfile>lb__hb__merge_8h.html</anchorfile>
      <anchor>ba970c2c53d05742fe7b1a94a9c3f95a</anchor>
      <arglist>(L_Func *fn)</arglist>
    </member>
    <member kind="function">
      <type>int</type>
      <name>L_safe_to_promote</name>
      <anchorfile>lb__hb__merge_8h.html</anchorfile>
      <anchor>0126b488cb50659632916ae499be1e6e</anchor>
      <arglist>(L_Cb *cb, L_Oper *oper, L_Oper *def_op)</arglist>
    </member>
    <member kind="function">
      <type>int</type>
      <name>L_can_merge_with_op_above</name>
      <anchorfile>lb__hb__merge_8h.html</anchorfile>
      <anchor>929c08a7e3f266b00485e01490c4cb1e</anchor>
      <arglist>(L_Cb *cb, L_Oper *op, L_Oper *tomove_op)</arglist>
    </member>
    <member kind="function">
      <type>int</type>
      <name>L_can_merge_with_op_below</name>
      <anchorfile>lb__hb__merge_8h.html</anchorfile>
      <anchor>2bd58b147600e52d14415438887ff8fc</anchor>
      <arglist>(L_Cb *cb, L_Oper *op, L_Oper *tomove_op)</arglist>
    </member>
  </compound>
  <compound kind="file">
    <name>lb_hb_misc.c</name>
    <path>/mnt/hgfs/mkt/Desktop/openimpact/src/Lcode/opti/Lblock/</path>
    <filename>lb__hb__misc_8c</filename>
    <includes id="lb__hb__hyperblock_8h" name="lb_hb_hyperblock.h" local="yes" imported="no">lb_hb_hyperblock.h</includes>
    <member kind="define">
      <type>#define</type>
      <name>ERR</name>
      <anchorfile>lb__hb__misc_8c.html</anchorfile>
      <anchor>735563036dced0b7d6cc98f97ea4978b</anchor>
      <arglist></arglist>
    </member>
    <member kind="function">
      <type>void</type>
      <name>LB_hb_reset_max_oper_id</name>
      <anchorfile>lb__hb__misc_8c.html</anchorfile>
      <anchor>5e332fbbdf5040e6da548935d6952da8</anchor>
      <arglist>(L_Func *fn)</arglist>
    </member>
    <member kind="function">
      <type>int</type>
      <name>LB_hb_is_single_block_loop</name>
      <anchorfile>lb__hb__misc_8c.html</anchorfile>
      <anchor>d173cb7903424164250834a385072a5e</anchor>
      <arglist>(L_Cb *cb)</arglist>
    </member>
    <member kind="function" static="yes">
      <type>static int</type>
      <name>dfs_visit</name>
      <anchorfile>lb__hb__misc_8c.html</anchorfile>
      <anchor>a8e386c0dccfb06ff92beddae4a6c2a6</anchor>
      <arglist>(L_Cb *cb, Set blocks, L_Cb *header)</arglist>
    </member>
    <member kind="function">
      <type>int</type>
      <name>LB_hb_region_contains_cycle</name>
      <anchorfile>lb__hb__misc_8c.html</anchorfile>
      <anchor>1526c1caf67f480bb6c6c299286919d8</anchor>
      <arglist>(Set blocks, L_Cb *header)</arglist>
    </member>
  </compound>
  <compound kind="file">
    <name>lb_hb_misc.h</name>
    <path>/mnt/hgfs/mkt/Desktop/openimpact/src/Lcode/opti/Lblock/</path>
    <filename>lb__hb__misc_8h</filename>
    <member kind="function">
      <type>void</type>
      <name>LB_hb_reset_max_oper_id</name>
      <anchorfile>lb__hb__misc_8h.html</anchorfile>
      <anchor>f32d4621dc85e47c25be31b5f243103e</anchor>
      <arglist>(L_Func *)</arglist>
    </member>
    <member kind="function">
      <type>int</type>
      <name>LB_hb_is_single_block_loop</name>
      <anchorfile>lb__hb__misc_8h.html</anchorfile>
      <anchor>ba9d5e1665dc84fbdc8d42fec7dde572</anchor>
      <arglist>(L_Cb *)</arglist>
    </member>
    <member kind="function">
      <type>void</type>
      <name>LB_elim_loop_backedges</name>
      <anchorfile>lb__hb__misc_8h.html</anchorfile>
      <anchor>294b341d77d72f68564104ae5a935a98</anchor>
      <arglist>(L_Func *fn, L_Loop *loop)</arglist>
    </member>
    <member kind="function">
      <type>int</type>
      <name>LB_hb_loop_collapsible</name>
      <anchorfile>lb__hb__misc_8h.html</anchorfile>
      <anchor>3882ff9e271cd4b04929ad9af62ebfe9</anchor>
      <arglist>(LB_TraceRegion_Header *trh, L_Loop *loop)</arglist>
    </member>
    <member kind="function">
      <type>L_Loop *</type>
      <name>LB_hb_do_collapse_loops</name>
      <anchorfile>lb__hb__misc_8h.html</anchorfile>
      <anchor>b7061e92465878b60e96119a6b161fb1</anchor>
      <arglist>(LB_TraceRegion_Header *trh, L_Func *fn, L_Loop *loop)</arglist>
    </member>
    <member kind="function">
      <type>int</type>
      <name>LB_hb_region_contains_cycle</name>
      <anchorfile>lb__hb__misc_8h.html</anchorfile>
      <anchor>1526c1caf67f480bb6c6c299286919d8</anchor>
      <arglist>(Set blocks, L_Cb *header)</arglist>
    </member>
  </compound>
  <compound kind="file">
    <name>lb_hb_path.c</name>
    <path>/mnt/hgfs/mkt/Desktop/openimpact/src/Lcode/opti/Lblock/</path>
    <filename>lb__hb__path_8c</filename>
    <includes id="lb__hb__hyperblock_8h" name="lb_hb_hyperblock.h" local="yes" imported="no">lb_hb_hyperblock.h</includes>
    <class kind="struct">_LB_hb_Path</class>
    <member kind="define">
      <type>#define</type>
      <name>LB_MAX_PATHS</name>
      <anchorfile>lb__hb__path_8c.html</anchorfile>
      <anchor>30ced35a6edad00a12c3b039ee2de424</anchor>
      <arglist></arglist>
    </member>
    <member kind="define">
      <type>#define</type>
      <name>LB_MAX_Z_PATHS</name>
      <anchorfile>lb__hb__path_8c.html</anchorfile>
      <anchor>a31af2956332381bf4eadb512108ea46</anchor>
      <arglist></arglist>
    </member>
    <member kind="define">
      <type>#define</type>
      <name>ERR</name>
      <anchorfile>lb__hb__path_8c.html</anchorfile>
      <anchor>735563036dced0b7d6cc98f97ea4978b</anchor>
      <arglist></arglist>
    </member>
    <member kind="define">
      <type>#define</type>
      <name>DEBUG_PATH</name>
      <anchorfile>lb__hb__path_8c.html</anchorfile>
      <anchor>a88c629f16157e7ff17dd464f9cc9b86</anchor>
      <arglist></arglist>
    </member>
    <member kind="typedef">
      <type>_LB_hb_Path</type>
      <name>LB_hb_Path</name>
      <anchorfile>lb__hb__path_8c.html</anchorfile>
      <anchor>36e397b4c23fc66399a7fec97e017a67</anchor>
      <arglist></arglist>
    </member>
    <member kind="function" static="yes">
      <type>static LB_hb_Path *</type>
      <name>LB_hb_new_path</name>
      <anchorfile>lb__hb__path_8c.html</anchorfile>
      <anchor>31d4b5f79ecca483181fbafe4bcb4461</anchor>
      <arglist>(void)</arglist>
    </member>
    <member kind="function" static="yes">
      <type>static void</type>
      <name>LB_hb_delete_path</name>
      <anchorfile>lb__hb__path_8c.html</anchorfile>
      <anchor>672028aa454b9d5176c4f3c42b9df107</anchor>
      <arglist>(LB_hb_Path *)</arglist>
    </member>
    <member kind="function" static="yes">
      <type>static void</type>
      <name>LB_hb_delete_all_path</name>
      <anchorfile>lb__hb__path_8c.html</anchorfile>
      <anchor>3f5908f43bd3d5d5b64c992bdb9f8a29</anchor>
      <arglist>(void)</arglist>
    </member>
    <member kind="function">
      <type>void</type>
      <name>LB_hb_print_path</name>
      <anchorfile>lb__hb__path_8c.html</anchorfile>
      <anchor>735b3ec98465ccc532d9b4fa1ac9e4b4</anchor>
      <arglist>(FILE *, LB_hb_Path *)</arglist>
    </member>
    <member kind="function">
      <type>void</type>
      <name>LB_hb_print_path_list</name>
      <anchorfile>lb__hb__path_8c.html</anchorfile>
      <anchor>e9d63bfdbb4dd1429dceeafcad12e42a</anchor>
      <arglist>(FILE *, LB_hb_Path *)</arglist>
    </member>
    <member kind="function">
      <type>int</type>
      <name>LB_hb_select_all_blocks</name>
      <anchorfile>lb__hb__path_8c.html</anchorfile>
      <anchor>698812c146e7193845cf096deb01d042</anchor>
      <arglist>(L_Cb *cb)</arglist>
    </member>
    <member kind="function">
      <type>int</type>
      <name>LB_hb_select_exact_blocks</name>
      <anchorfile>lb__hb__path_8c.html</anchorfile>
      <anchor>ad8bcb62c52a7e9108fbafab47311e66</anchor>
      <arglist>(L_Cb *cb)</arglist>
    </member>
    <member kind="function">
      <type>int</type>
      <name>LB_hb_ignore_block</name>
      <anchorfile>lb__hb__path_8c.html</anchorfile>
      <anchor>82b34b0386da0db0b63d7de9f7348bc6</anchor>
      <arglist>(L_Cb *cb)</arglist>
    </member>
    <member kind="function">
      <type>Set</type>
      <name>LB_hb_find_exact_blocks</name>
      <anchorfile>lb__hb__path_8c.html</anchorfile>
      <anchor>51c96260a249b9021fbed2435af9241a</anchor>
      <arglist>(L_Cb *cb)</arglist>
    </member>
    <member kind="function" static="yes">
      <type>static void</type>
      <name>LB_hb_unmark</name>
      <anchorfile>lb__hb__path_8c.html</anchorfile>
      <anchor>4bbd1012149a9895d08a54c32cdde843</anchor>
      <arglist>(L_Cb *cb)</arglist>
    </member>
    <member kind="function" static="yes">
      <type>static void</type>
      <name>LB_hb_record_path</name>
      <anchorfile>lb__hb__path_8c.html</anchorfile>
      <anchor>82ca9cc95754d0d950b09ffef1c60554</anchor>
      <arglist>(L_Cb *header)</arglist>
    </member>
    <member kind="function" static="yes">
      <type>static void</type>
      <name>LB_hb_find_path</name>
      <anchorfile>lb__hb__path_8c.html</anchorfile>
      <anchor>a355a483ccf26da921118ae7f4cd0001</anchor>
      <arglist>(L_Cb *start, L_Cb *end, Set blocks, L_Cb *cb, int *flag)</arglist>
    </member>
    <member kind="function">
      <type>int</type>
      <name>LB_hb_find_all_paths</name>
      <anchorfile>lb__hb__path_8c.html</anchorfile>
      <anchor>6da9f0a8465500a7af65c93a71fc9bf6</anchor>
      <arglist>(L_Cb *start, L_Cb *end, Set blocks)</arglist>
    </member>
    <member kind="function" static="yes">
      <type>static void</type>
      <name>LB_hb_find_total_num_ops</name>
      <anchorfile>lb__hb__path_8c.html</anchorfile>
      <anchor>9e522537fc45f9db7e8fe32f7f97bd14</anchor>
      <arglist>(Set blocks)</arglist>
    </member>
    <member kind="function" static="yes">
      <type>static void</type>
      <name>LB_hb_find_path_num_ops</name>
      <anchorfile>lb__hb__path_8c.html</anchorfile>
      <anchor>695bb42147e3005b6cf94709f90f5f55</anchor>
      <arglist>(LB_hb_Path *path)</arglist>
    </member>
    <member kind="function" static="yes">
      <type>static void</type>
      <name>LB_hb_find_path_exec_ratio</name>
      <anchorfile>lb__hb__path_8c.html</anchorfile>
      <anchor>3a0d3539fe87e64ee030f77d55385f19</anchor>
      <arglist>(LB_hb_Path *path)</arglist>
    </member>
    <member kind="function" static="yes">
      <type>static void</type>
      <name>LB_hb_find_path_flags</name>
      <anchorfile>lb__hb__path_8c.html</anchorfile>
      <anchor>653f13c120c6b8ba659e616216ad4980</anchor>
      <arglist>(LB_hb_Path *path)</arglist>
    </member>
    <member kind="function" static="yes">
      <type>static int</type>
      <name>LB_hb_find_path_dep_height</name>
      <anchorfile>lb__hb__path_8c.html</anchorfile>
      <anchor>b326829007226d485aad2efffc6820a0</anchor>
      <arglist>(LB_hb_Path *path)</arglist>
    </member>
    <member kind="function" static="yes">
      <type>static double</type>
      <name>LB_hb_find_path_priority</name>
      <anchorfile>lb__hb__path_8c.html</anchorfile>
      <anchor>db2cf352d08fcf0fa424b3c887ea7b9c</anchor>
      <arglist>(LB_hb_Path *path)</arglist>
    </member>
    <member kind="function" static="yes">
      <type>static void</type>
      <name>swap</name>
      <anchorfile>lb__hb__path_8c.html</anchorfile>
      <anchor>dfc841cedc30b11c4bf8199e347419d5</anchor>
      <arglist>(int i, int j)</arglist>
    </member>
    <member kind="function" static="yes">
      <type>static void</type>
      <name>heapify</name>
      <anchorfile>lb__hb__path_8c.html</anchorfile>
      <anchor>f5097c2092fccb02fe467261b74fbd18</anchor>
      <arglist>(int p, int size)</arglist>
    </member>
    <member kind="function" static="yes">
      <type>static void</type>
      <name>LB_hb_sort_paths</name>
      <anchorfile>lb__hb__path_8c.html</anchorfile>
      <anchor>7dec8bfbdd991c27b71a8e89f0d2cf3e</anchor>
      <arglist>(void)</arglist>
    </member>
    <member kind="function">
      <type>void</type>
      <name>LB_hb_find_path_info</name>
      <anchorfile>lb__hb__path_8c.html</anchorfile>
      <anchor>2f9023684f59b245fcf842174adbf4e8</anchor>
      <arglist>(L_Cb *start, L_Cb *end, Set blocks)</arglist>
    </member>
    <member kind="function">
      <type>void</type>
      <name>LB_hb_init_path_globals</name>
      <anchorfile>lb__hb__path_8c.html</anchorfile>
      <anchor>336206e14e3361b1d04d569b40e984fb</anchor>
      <arglist>(void)</arglist>
    </member>
    <member kind="function">
      <type>void</type>
      <name>LB_hb_deinit_path_globals</name>
      <anchorfile>lb__hb__path_8c.html</anchorfile>
      <anchor>f11a5e9b39e7d72cd6eb4c213de3f313</anchor>
      <arglist>()</arglist>
    </member>
    <member kind="function">
      <type>int</type>
      <name>LB_hb_path_contains_excludable_hazard</name>
      <anchorfile>lb__hb__path_8c.html</anchorfile>
      <anchor>babd06b533c7a40f30e57cd900a70c52</anchor>
      <arglist>(LB_hb_Path *path, int main_path)</arglist>
    </member>
    <member kind="function">
      <type>LB_TraceRegion *</type>
      <name>LB_hb_select_trivial</name>
      <anchorfile>lb__hb__path_8c.html</anchorfile>
      <anchor>5dd53a3bf00f616354741015ea01d557</anchor>
      <arglist>(int type, L_Cb *start, L_Cb *end, Set all_blocks, int id)</arglist>
    </member>
    <member kind="function">
      <type>LB_TraceRegion *</type>
      <name>LB_hb_select_paths</name>
      <anchorfile>lb__hb__path_8c.html</anchorfile>
      <anchor>656ab0d58e3271ee4c5df937606b28c2</anchor>
      <arglist>(int type, L_Cb *start, L_Cb *end, Set all_blocks, int id)</arglist>
    </member>
    <member kind="function">
      <type>LB_TraceRegion *</type>
      <name>LB_hb_path_region_formation</name>
      <anchorfile>lb__hb__path_8c.html</anchorfile>
      <anchor>8174589e8a4af00c4f4dbfdf6fc56a31</anchor>
      <arglist>(L_Cb *header_cb, L_Cb *exit_cb, Set region_cb, int type, LB_TraceRegion_Header *header)</arglist>
    </member>
    <member kind="variable" static="yes">
      <type>static L_Alloc_Pool *</type>
      <name>LB_hb_path_pool</name>
      <anchorfile>lb__hb__path_8c.html</anchorfile>
      <anchor>49235b0e43a359de47fb60f82da7b12d</anchor>
      <arglist></arglist>
    </member>
    <member kind="variable" static="yes">
      <type>static List</type>
      <name>LB_hb_all_paths</name>
      <anchorfile>lb__hb__path_8c.html</anchorfile>
      <anchor>97d485f7e17a3efb73f9c0a36e1d9fb8</anchor>
      <arglist></arglist>
    </member>
    <member kind="variable" static="yes">
      <type>static Set</type>
      <name>LB_hb_path_all_blocks</name>
      <anchorfile>lb__hb__path_8c.html</anchorfile>
      <anchor>e455ef6ce2fa99cc8bce477e25227df6</anchor>
      <arglist></arglist>
    </member>
    <member kind="variable" static="yes">
      <type>static Stack *</type>
      <name>LB_path_stack</name>
      <anchorfile>lb__hb__path_8c.html</anchorfile>
      <anchor>e13786b297504a032feaa623a174dfed</anchor>
      <arglist></arglist>
    </member>
    <member kind="variable" static="yes">
      <type>static Set *</type>
      <name>B</name>
      <anchorfile>lb__hb__path_8c.html</anchorfile>
      <anchor>a5c6e5aa9707fb994ee7a49093c9eceb</anchor>
      <arglist></arglist>
    </member>
    <member kind="variable" static="yes">
      <type>static L_Cb *</type>
      <name>LB_path_cb</name>
      <anchorfile>lb__hb__path_8c.html</anchorfile>
      <anchor>8329efc7724f3cd0bb829ba54ce9b15a</anchor>
      <arglist></arglist>
    </member>
    <member kind="variable" static="yes">
      <type>static int</type>
      <name>LB_hb_path_max_id</name>
      <anchorfile>lb__hb__path_8c.html</anchorfile>
      <anchor>d80924fb941703781993335dd8b839f5</anchor>
      <arglist></arglist>
    </member>
    <member kind="variable" static="yes">
      <type>static int</type>
      <name>LB_hb_path_max_dep_height</name>
      <anchorfile>lb__hb__path_8c.html</anchorfile>
      <anchor>f98bf96845b0de46efb25e509ae5c0ff</anchor>
      <arglist></arglist>
    </member>
    <member kind="variable" static="yes">
      <type>static int</type>
      <name>LB_hb_path_total_ops</name>
      <anchorfile>lb__hb__path_8c.html</anchorfile>
      <anchor>1c55dc9ed7b0abc088794d1d8708777a</anchor>
      <arglist></arglist>
    </member>
    <member kind="variable" static="yes">
      <type>static int</type>
      <name>LB_hb_path_total_paths</name>
      <anchorfile>lb__hb__path_8c.html</anchorfile>
      <anchor>d95d9748d7ae0ad668285b649c71e87b</anchor>
      <arglist></arglist>
    </member>
    <member kind="variable" static="yes">
      <type>static int</type>
      <name>LB_hb_path_total_zpaths</name>
      <anchorfile>lb__hb__path_8c.html</anchorfile>
      <anchor>98f218ca5da641d05e29b34c552252ef</anchor>
      <arglist></arglist>
    </member>
    <member kind="variable">
      <type>int</type>
      <name>LB_hb_path_max_path_exceeded</name>
      <anchorfile>lb__hb__path_8c.html</anchorfile>
      <anchor>8bc037ccf46d521f84f6a4fc5a4c9e8e</anchor>
      <arglist></arglist>
    </member>
    <member kind="variable" static="yes">
      <type>static LB_hb_Sort_List *</type>
      <name>sort_buf</name>
      <anchorfile>lb__hb__path_8c.html</anchorfile>
      <anchor>73a014c96675991b05de49411b4c1650</anchor>
      <arglist></arglist>
    </member>
  </compound>
  <compound kind="file">
    <name>lb_hb_path.h</name>
    <path>/mnt/hgfs/mkt/Desktop/openimpact/src/Lcode/opti/Lblock/</path>
    <filename>lb__hb__path_8h</filename>
    <class kind="struct">_LB_hb_Sort_List</class>
    <member kind="define">
      <type>#define</type>
      <name>HB_SELECT_ALL_ATTR</name>
      <anchorfile>lb__hb__path_8h.html</anchorfile>
      <anchor>5561ebb6c06898b7d792fd0a1eeeb5f0</anchor>
      <arglist></arglist>
    </member>
    <member kind="define">
      <type>#define</type>
      <name>HB_SELECT_EXACT_ATTR</name>
      <anchorfile>lb__hb__path_8h.html</anchorfile>
      <anchor>70570985303e8f1139de8a03006e56e0</anchor>
      <arglist></arglist>
    </member>
    <member kind="define">
      <type>#define</type>
      <name>HB_IGNORE_ATTR</name>
      <anchorfile>lb__hb__path_8h.html</anchorfile>
      <anchor>40a7ceb5b588fed8a755f0bbe8bbd636</anchor>
      <arglist></arglist>
    </member>
    <member kind="typedef">
      <type>_LB_hb_Sort_List</type>
      <name>LB_hb_Sort_List</name>
      <anchorfile>lb__hb__path_8h.html</anchorfile>
      <anchor>bdee6b29243d8cc474ac2cfff4743c96</anchor>
      <arglist></arglist>
    </member>
    <member kind="function">
      <type>int</type>
      <name>LB_hb_select_all_blocks</name>
      <anchorfile>lb__hb__path_8h.html</anchorfile>
      <anchor>63bec443782b9712d400d3d4812cec74</anchor>
      <arglist>(L_Cb *)</arglist>
    </member>
    <member kind="function">
      <type>int</type>
      <name>LB_hb_select_exact_blocks</name>
      <anchorfile>lb__hb__path_8h.html</anchorfile>
      <anchor>b58d5db0e6cfdc3d2c755d7d53ee9bf9</anchor>
      <arglist>(L_Cb *)</arglist>
    </member>
    <member kind="function">
      <type>int</type>
      <name>LB_hb_ignore_block</name>
      <anchorfile>lb__hb__path_8h.html</anchorfile>
      <anchor>a7ffd54a5d2b1bc889f6b74985f13018</anchor>
      <arglist>(L_Cb *)</arglist>
    </member>
    <member kind="function">
      <type>void</type>
      <name>LB_hb_init_path_globals</name>
      <anchorfile>lb__hb__path_8h.html</anchorfile>
      <anchor>336206e14e3361b1d04d569b40e984fb</anchor>
      <arglist>(void)</arglist>
    </member>
    <member kind="function">
      <type>void</type>
      <name>LB_hb_deinit_path_globals</name>
      <anchorfile>lb__hb__path_8h.html</anchorfile>
      <anchor>606da6c755a1c97e05bddb9b76924f91</anchor>
      <arglist>(void)</arglist>
    </member>
    <member kind="function">
      <type>int</type>
      <name>LB_hb_find_all_paths</name>
      <anchorfile>lb__hb__path_8h.html</anchorfile>
      <anchor>bd1d18f97aa2b302777248e6629abf31</anchor>
      <arglist>(L_Cb *, L_Cb *, Set)</arglist>
    </member>
    <member kind="function">
      <type>void</type>
      <name>LB_hb_find_path_info</name>
      <anchorfile>lb__hb__path_8h.html</anchorfile>
      <anchor>d0dba6d713de1421d6eeeb923061f773</anchor>
      <arglist>(L_Cb *, L_Cb *, Set)</arglist>
    </member>
    <member kind="function">
      <type>LB_TraceRegion *</type>
      <name>LB_hb_select_paths</name>
      <anchorfile>lb__hb__path_8h.html</anchorfile>
      <anchor>5c1440627909287079106a323ae6d1f2</anchor>
      <arglist>(int, L_Cb *, L_Cb *, Set, int)</arglist>
    </member>
    <member kind="function">
      <type>LB_TraceRegion *</type>
      <name>LB_hb_select_trivial</name>
      <anchorfile>lb__hb__path_8h.html</anchorfile>
      <anchor>6428481c71764b64013ef462aada0763</anchor>
      <arglist>(int, L_Cb *, L_Cb *, Set, int)</arglist>
    </member>
    <member kind="function">
      <type>LB_TraceRegion *</type>
      <name>LB_hb_path_region_formation</name>
      <anchorfile>lb__hb__path_8h.html</anchorfile>
      <anchor>628323ea6edf0571ae0d8bc51586e5a9</anchor>
      <arglist>(L_Cb *, L_Cb *, Set, int, LB_TraceRegion_Header *)</arglist>
    </member>
    <member kind="variable">
      <type>int</type>
      <name>LB_hb_path_max_path_exceeded</name>
      <anchorfile>lb__hb__path_8h.html</anchorfile>
      <anchor>8bc037ccf46d521f84f6a4fc5a4c9e8e</anchor>
      <arglist></arglist>
    </member>
  </compound>
  <compound kind="file">
    <name>lb_hb_peel.c</name>
    <path>/mnt/hgfs/mkt/Desktop/openimpact/src/Lcode/opti/Lblock/</path>
    <filename>lb__hb__peel_8c</filename>
    <includes id="lb__hb__hyperblock_8h" name="lb_hb_hyperblock.h" local="yes" imported="no">lb_hb_hyperblock.h</includes>
    <includes id="lb__b__internal_8h" name="lb_b_internal.h" local="yes" imported="no">lb_b_internal.h</includes>
    <includes id="lb__peel_8h" name="lb_peel.h" local="yes" imported="no">lb_peel.h</includes>
    <includes id="lb__hb__peel_8h" name="lb_hb_peel.h" local="yes" imported="no">lb_hb_peel.h</includes>
    <member kind="define">
      <type>#define</type>
      <name>DBG_LB_PEEL</name>
      <anchorfile>lb__hb__peel_8c.html</anchorfile>
      <anchor>861590cc952febaeae492e34f9f469ed</anchor>
      <arglist></arglist>
    </member>
    <member kind="define">
      <type>#define</type>
      <name>LB_IGNORE_EXCLUDED_NESTS</name>
      <anchorfile>lb__hb__peel_8c.html</anchorfile>
      <anchor>b32adb91c32976dd359a815f6ebdad81</anchor>
      <arglist></arglist>
    </member>
    <member kind="function">
      <type>int</type>
      <name>LB_hb_is_hand_marked_for_peel</name>
      <anchorfile>lb__hb__peel_8c.html</anchorfile>
      <anchor>a9f203dfba6067c7cea5b4fe5abcae47</anchor>
      <arglist>(L_Loop *loop)</arglist>
    </member>
    <member kind="function" static="yes">
      <type>static int</type>
      <name>LB_hb_get_hand_marked_peel_amount</name>
      <anchorfile>lb__hb__peel_8c.html</anchorfile>
      <anchor>0928c7dd6b30bf267a1dae5c91cc5c01</anchor>
      <arglist>(L_Loop *loop)</arglist>
    </member>
    <member kind="function" static="yes">
      <type>static int</type>
      <name>LB_hb_get_peel_amount</name>
      <anchorfile>lb__hb__peel_8c.html</anchorfile>
      <anchor>698998555e5cd444873549e3ece5fa4e</anchor>
      <arglist>(L_Loop *loop)</arglist>
    </member>
    <member kind="function">
      <type>int</type>
      <name>LB_hb_find_num_peel</name>
      <anchorfile>lb__hb__peel_8c.html</anchorfile>
      <anchor>0707a13fdcc6e35e148ca01ccfdd4a1d</anchor>
      <arglist>(L_Loop *loop, Set peel_cb)</arglist>
    </member>
    <member kind="function">
      <type>int</type>
      <name>LB_hb_consider_loop_for_peeling</name>
      <anchorfile>lb__hb__peel_8c.html</anchorfile>
      <anchor>9cd2401c120aa26fb89a12fba7fa70b1</anchor>
      <arglist>(L_Loop *loop, LB_TraceRegion_Header *header)</arglist>
    </member>
    <member kind="function">
      <type>void</type>
      <name>LB_hb_do_loop_peel</name>
      <anchorfile>lb__hb__peel_8c.html</anchorfile>
      <anchor>9487cc1556ca4190fa20fb814ed66f88</anchor>
      <arglist>(L_Func *fn, LB_TraceRegion *tr, LB_TraceRegion_Header *tr_header)</arglist>
    </member>
  </compound>
  <compound kind="file">
    <name>lb_hb_peel.h</name>
    <path>/mnt/hgfs/mkt/Desktop/openimpact/src/Lcode/opti/Lblock/</path>
    <filename>lb__hb__peel_8h</filename>
    <member kind="function">
      <type>int</type>
      <name>LB_hb_consider_loop_for_peeling</name>
      <anchorfile>lb__hb__peel_8h.html</anchorfile>
      <anchor>609b4ce51f2d884b10a1306668eb39cc</anchor>
      <arglist>(L_Loop *loop, LB_TraceRegion_Header *)</arglist>
    </member>
    <member kind="function">
      <type>void</type>
      <name>LB_hb_do_loop_peel</name>
      <anchorfile>lb__hb__peel_8h.html</anchorfile>
      <anchor>e1285476dc72d9556ea7b999d4b16919</anchor>
      <arglist>(L_Func *, LB_TraceRegion *, LB_TraceRegion_Header *)</arglist>
    </member>
    <member kind="function">
      <type>int</type>
      <name>LB_hb_find_num_peel</name>
      <anchorfile>lb__hb__peel_8h.html</anchorfile>
      <anchor>0707a13fdcc6e35e148ca01ccfdd4a1d</anchor>
      <arglist>(L_Loop *loop, Set peel_cb)</arglist>
    </member>
  </compound>
  <compound kind="file">
    <name>lb_hb_region_general.c</name>
    <path>/mnt/hgfs/mkt/Desktop/openimpact/src/Lcode/opti/Lblock/</path>
    <filename>lb__hb__region__general_8c</filename>
    <includes id="lb__hb__hyperblock_8h" name="lb_hb_hyperblock.h" local="yes" imported="no">lb_hb_hyperblock.h</includes>
    <member kind="define">
      <type>#define</type>
      <name>ERR</name>
      <anchorfile>lb__hb__region__general_8c.html</anchorfile>
      <anchor>735563036dced0b7d6cc98f97ea4978b</anchor>
      <arglist></arglist>
    </member>
    <member kind="define">
      <type>#define</type>
      <name>LB_MAX_CB_IN_HYPERBLOCK</name>
      <anchorfile>lb__hb__region__general_8c.html</anchorfile>
      <anchor>e680ff33962ba9ff78b047c42360a12c</anchor>
      <arglist></arglist>
    </member>
    <member kind="function" static="yes">
      <type>static int</type>
      <name>LB_hb_calc_cb_flags</name>
      <anchorfile>lb__hb__region__general_8c.html</anchorfile>
      <anchor>940a5066cb10057dffad06be3457eb22</anchor>
      <arglist>(L_Cb *cb)</arglist>
    </member>
    <member kind="function" static="yes">
      <type>static void</type>
      <name>LB_remove_deps_assumed_optimized_away</name>
      <anchorfile>lb__hb__region__general_8c.html</anchorfile>
      <anchor>8c4ff9ddaee4a91ee1aa0e16f65d9c12</anchor>
      <arglist>(SM_Cb *sm_cb)</arglist>
    </member>
    <member kind="function">
      <type>int</type>
      <name>LB_hb_calc_cb_dep_height</name>
      <anchorfile>lb__hb__region__general_8c.html</anchorfile>
      <anchor>eed6ecc28c2c9f03c77fa63412e75d28</anchor>
      <arglist>(L_Cb *cb)</arglist>
    </member>
    <member kind="function">
      <type>void</type>
      <name>LB_hb_print_cb_info</name>
      <anchorfile>lb__hb__region__general_8c.html</anchorfile>
      <anchor>3ca230a810058db14507c324ef187597</anchor>
      <arglist>(FILE *F, L_Cb *cb)</arglist>
    </member>
    <member kind="function">
      <type>void</type>
      <name>LB_hb_print_all_cb_info</name>
      <anchorfile>lb__hb__region__general_8c.html</anchorfile>
      <anchor>22ec70439fba371e494391a6a8526047</anchor>
      <arglist>(FILE *F, L_Func *fn)</arglist>
    </member>
    <member kind="function">
      <type>void</type>
      <name>LB_hb_find_cb_info</name>
      <anchorfile>lb__hb__region__general_8c.html</anchorfile>
      <anchor>9bcca93bf5daa25c155ceab469a56cec</anchor>
      <arglist>(L_Func *fn)</arglist>
    </member>
    <member kind="function">
      <type>int</type>
      <name>LB_hb_can_predicate_cb</name>
      <anchorfile>lb__hb__region__general_8c.html</anchorfile>
      <anchor>40f93ec4dd5dbada52d8fc2b9883fa35</anchor>
      <arglist>(L_Cb *cb)</arglist>
    </member>
    <member kind="function" static="yes">
      <type>static double</type>
      <name>LB_hb_scale_weight_ratio_for_hazards</name>
      <anchorfile>lb__hb__region__general_8c.html</anchorfile>
      <anchor>6f354e343a734cbc96dc0a9302eada0a</anchor>
      <arglist>(double val, L_Cb *cb)</arglist>
    </member>
    <member kind="function">
      <type>int</type>
      <name>LB_hb_cb_contains_excludable_hazard</name>
      <anchorfile>lb__hb__region__general_8c.html</anchorfile>
      <anchor>bb60f96b7d6f526f9daefa9abb867f2b</anchor>
      <arglist>(L_Cb *cb, int main_path)</arglist>
    </member>
    <member kind="function">
      <type>int</type>
      <name>LB_hb_select_blocks</name>
      <anchorfile>lb__hb__region__general_8c.html</anchorfile>
      <anchor>1ddfec6322ba415dbe818b8bc5d282c2</anchor>
      <arglist>(L_Cb *cb, L_Cb *header, Set *blocks, Set avail_blocks, int main_path, double path_ratio, int flags, LB_TraceRegion_Header *tr_header)</arglist>
    </member>
    <member kind="function">
      <type>void</type>
      <name>LB_hb_sort_cb_by_weight</name>
      <anchorfile>lb__hb__region__general_8c.html</anchorfile>
      <anchor>35b398e45d700a8b4783ba69f404dda4</anchor>
      <arglist>(L_Func *fn)</arglist>
    </member>
    <member kind="function">
      <type>void</type>
      <name>LB_hb_find_general_regions</name>
      <anchorfile>lb__hb__region__general_8c.html</anchorfile>
      <anchor>0caa092355c6ded9b683d3f29b845ee8</anchor>
      <arglist>(L_Func *fn, int flags, LB_TraceRegion_Header *header)</arglist>
    </member>
    <member kind="variable">
      <type>LB_Cb_Info *</type>
      <name>LB_hb_cb_info</name>
      <anchorfile>lb__hb__region__general_8c.html</anchorfile>
      <anchor>71f161d99e40d752b7876aae4da2a6b5</anchor>
      <arglist></arglist>
    </member>
    <member kind="variable" static="yes">
      <type>static LB_hb_Sort_List *</type>
      <name>LB_hb_cb_sort_buf</name>
      <anchorfile>lb__hb__region__general_8c.html</anchorfile>
      <anchor>9fd57e87133643eaf6f7db19abc102a3</anchor>
      <arglist></arglist>
    </member>
    <member kind="variable" static="yes">
      <type>static Set</type>
      <name>LB_hb_nested_region_blocks</name>
      <anchorfile>lb__hb__region__general_8c.html</anchorfile>
      <anchor>ceba5acab296b98cd9b34f74173243e4</anchor>
      <arglist></arglist>
    </member>
    <member kind="variable">
      <type>int</type>
      <name>LB_hb_curr_slots_used</name>
      <anchorfile>lb__hb__region__general_8c.html</anchorfile>
      <anchor>2d52589fec215faa80aa125ae4348103</anchor>
      <arglist></arglist>
    </member>
    <member kind="variable">
      <type>int</type>
      <name>LB_hb_curr_slots_avail</name>
      <anchorfile>lb__hb__region__general_8c.html</anchorfile>
      <anchor>2c8adafeb53822e6f04dc6f9d2599d08</anchor>
      <arglist></arglist>
    </member>
    <member kind="variable">
      <type>int</type>
      <name>LB_hb_curr_dep_height</name>
      <anchorfile>lb__hb__region__general_8c.html</anchorfile>
      <anchor>7656a23471093766fb6d4f3ed71928d9</anchor>
      <arglist></arglist>
    </member>
  </compound>
  <compound kind="file">
    <name>lb_hb_region_general.h</name>
    <path>/mnt/hgfs/mkt/Desktop/openimpact/src/Lcode/opti/Lblock/</path>
    <filename>lb__hb__region__general_8h</filename>
    <class kind="struct">_LB_Cb_Info</class>
    <member kind="define">
      <type>#define</type>
      <name>LB_BLOCK_SEL_NULL</name>
      <anchorfile>lb__hb__region__general_8h.html</anchorfile>
      <anchor>4fef79886cf5db84041efe9fd3ca3e15</anchor>
      <arglist></arglist>
    </member>
    <member kind="define">
      <type>#define</type>
      <name>LB_BLOCK_SEL_NO_NESTED_HAMMOCKS</name>
      <anchorfile>lb__hb__region__general_8h.html</anchorfile>
      <anchor>8663251746216355f5d26e176250d694</anchor>
      <arglist></arglist>
    </member>
    <member kind="define">
      <type>#define</type>
      <name>LB_BLOCK_SEL_NO_NESTED_LOOPS</name>
      <anchorfile>lb__hb__region__general_8h.html</anchorfile>
      <anchor>5c9088488c6ea4902a2e8efa72e8b629</anchor>
      <arglist></arglist>
    </member>
    <member kind="define">
      <type>#define</type>
      <name>LB_BLOCK_SEL_AGGRESSIVE</name>
      <anchorfile>lb__hb__region__general_8h.html</anchorfile>
      <anchor>8f0517aeb8492e31ddbb150ca7daf78c</anchor>
      <arglist></arglist>
    </member>
    <member kind="typedef">
      <type>_LB_Cb_Info</type>
      <name>LB_Cb_Info</name>
      <anchorfile>lb__hb__region__general_8h.html</anchorfile>
      <anchor>ab3c404bd732e1915cd0de510b7e17f6</anchor>
      <arglist></arglist>
    </member>
    <member kind="function">
      <type>int</type>
      <name>LB_hb_calc_cb_dep_height</name>
      <anchorfile>lb__hb__region__general_8h.html</anchorfile>
      <anchor>eed6ecc28c2c9f03c77fa63412e75d28</anchor>
      <arglist>(L_Cb *cb)</arglist>
    </member>
    <member kind="function">
      <type>void</type>
      <name>LB_hb_print_cb_info</name>
      <anchorfile>lb__hb__region__general_8h.html</anchorfile>
      <anchor>3ca230a810058db14507c324ef187597</anchor>
      <arglist>(FILE *F, L_Cb *cb)</arglist>
    </member>
    <member kind="function">
      <type>void</type>
      <name>LB_hb_print_all_cb_info</name>
      <anchorfile>lb__hb__region__general_8h.html</anchorfile>
      <anchor>22ec70439fba371e494391a6a8526047</anchor>
      <arglist>(FILE *F, L_Func *fn)</arglist>
    </member>
    <member kind="function">
      <type>void</type>
      <name>LB_hb_find_cb_info</name>
      <anchorfile>lb__hb__region__general_8h.html</anchorfile>
      <anchor>9bcca93bf5daa25c155ceab469a56cec</anchor>
      <arglist>(L_Func *fn)</arglist>
    </member>
    <member kind="function">
      <type>int</type>
      <name>LB_hb_can_predicate_cb</name>
      <anchorfile>lb__hb__region__general_8h.html</anchorfile>
      <anchor>40f93ec4dd5dbada52d8fc2b9883fa35</anchor>
      <arglist>(L_Cb *cb)</arglist>
    </member>
    <member kind="function">
      <type>int</type>
      <name>LB_hb_select_blocks</name>
      <anchorfile>lb__hb__region__general_8h.html</anchorfile>
      <anchor>6e6ec9a36db034b03ff8091a6b8dd2ae</anchor>
      <arglist>(L_Cb *cb, L_Cb *header, Set *blocks, Set avail_blocks, int main_path, double path_ratio, int simple_formation, LB_TraceRegion_Header *tr_header)</arglist>
    </member>
    <member kind="function">
      <type>void</type>
      <name>LB_hb_find_general_regions</name>
      <anchorfile>lb__hb__region__general_8h.html</anchorfile>
      <anchor>f3aa015e689defa2ddf6c48a69690e91</anchor>
      <arglist>(L_Func *fn, int simple_formation, LB_TraceRegion_Header *)</arglist>
    </member>
    <member kind="variable">
      <type>LB_Cb_Info *</type>
      <name>LB_hb_cb_info</name>
      <anchorfile>lb__hb__region__general_8h.html</anchorfile>
      <anchor>71f161d99e40d752b7876aae4da2a6b5</anchor>
      <arglist></arglist>
    </member>
    <member kind="variable">
      <type>int</type>
      <name>LB_hb_curr_slots_used</name>
      <anchorfile>lb__hb__region__general_8h.html</anchorfile>
      <anchor>2d52589fec215faa80aa125ae4348103</anchor>
      <arglist></arglist>
    </member>
    <member kind="variable">
      <type>int</type>
      <name>LB_hb_curr_slots_avail</name>
      <anchorfile>lb__hb__region__general_8h.html</anchorfile>
      <anchor>2c8adafeb53822e6f04dc6f9d2599d08</anchor>
      <arglist></arglist>
    </member>
    <member kind="variable">
      <type>int</type>
      <name>LB_hb_curr_dep_height</name>
      <anchorfile>lb__hb__region__general_8h.html</anchorfile>
      <anchor>7656a23471093766fb6d4f3ed71928d9</anchor>
      <arglist></arglist>
    </member>
  </compound>
  <compound kind="file">
    <name>lb_hb_region_hammock.c</name>
    <path>/mnt/hgfs/mkt/Desktop/openimpact/src/Lcode/opti/Lblock/</path>
    <filename>lb__hb__region__hammock_8c</filename>
    <includes id="lb__hb__hyperblock_8h" name="lb_hb_hyperblock.h" local="yes" imported="no">lb_hb_hyperblock.h</includes>
    <class kind="struct">_LB_Hammock</class>
    <member kind="define">
      <type>#define</type>
      <name>LB_HAMMOCK_NEEDS_EXPANSION</name>
      <anchorfile>lb__hb__region__hammock_8c.html</anchorfile>
      <anchor>eafeb0693ff6ad0a1c737be01d5fb57a</anchor>
      <arglist></arglist>
    </member>
    <member kind="define">
      <type>#define</type>
      <name>DEBUG_HAMMOCK</name>
      <anchorfile>lb__hb__region__hammock_8c.html</anchorfile>
      <anchor>6b5b7eb7dbf971910c6a6c3812012798</anchor>
      <arglist></arglist>
    </member>
    <member kind="define">
      <type>#define</type>
      <name>LB_HB_SELECT_IMPROPER_HAMMOCKS</name>
      <anchorfile>lb__hb__region__hammock_8c.html</anchorfile>
      <anchor>ff54c13284f6bd0d969c17fcf303964b</anchor>
      <arglist></arglist>
    </member>
    <member kind="typedef">
      <type>_LB_Hammock</type>
      <name>LB_Hammock</name>
      <anchorfile>lb__hb__region__hammock_8c.html</anchorfile>
      <anchor>9f2a9b40a51fcc3b70664554333fb683</anchor>
      <arglist></arglist>
    </member>
    <member kind="function" static="yes">
      <type>static LB_Hammock *</type>
      <name>LB_hb_new_hammock</name>
      <anchorfile>lb__hb__region__hammock_8c.html</anchorfile>
      <anchor>d124f3f24f5ec407b4519269fd88a7d2</anchor>
      <arglist>(L_Cb *, L_Cb *)</arglist>
    </member>
    <member kind="function" static="yes">
      <type>static void</type>
      <name>LB_hb_delete_hammock</name>
      <anchorfile>lb__hb__region__hammock_8c.html</anchorfile>
      <anchor>a8a094b7a97ea4b7493613e86c43f067</anchor>
      <arglist>(LB_Hammock *)</arglist>
    </member>
    <member kind="function" static="yes">
      <type>static void</type>
      <name>LB_hb_delete_all_hammocks</name>
      <anchorfile>lb__hb__region__hammock_8c.html</anchorfile>
      <anchor>b2dca79f614be08b4f8492297d2653f5</anchor>
      <arglist>(void)</arglist>
    </member>
    <member kind="function" static="yes">
      <type>static LB_Hammock *</type>
      <name>LB_hb_find_hammock</name>
      <anchorfile>lb__hb__region__hammock_8c.html</anchorfile>
      <anchor>af524ab6dfcde555d5edf412cb0f5741</anchor>
      <arglist>(L_Cb *, L_Cb *)</arglist>
    </member>
    <member kind="function" static="yes">
      <type>static int</type>
      <name>LB_hb_valid_hammock</name>
      <anchorfile>lb__hb__region__hammock_8c.html</anchorfile>
      <anchor>43048fb74cbbce04a8e4cb52f5954db8</anchor>
      <arglist>(LB_Hammock *hammock)</arglist>
    </member>
    <member kind="function" static="yes">
      <type>static void</type>
      <name>LB_remove_subsumed_hammock_traceregions</name>
      <anchorfile>lb__hb__region__hammock_8c.html</anchorfile>
      <anchor>30d3b2a596bd0b2d54c059c39d62a804</anchor>
      <arglist>(LB_TraceRegion_Header *header)</arglist>
    </member>
    <member kind="function" static="yes">
      <type>static void</type>
      <name>LB_hb_print_hammock</name>
      <anchorfile>lb__hb__region__hammock_8c.html</anchorfile>
      <anchor>b28b4e5e27ee623974114b232dded7c6</anchor>
      <arglist>(FILE *, LB_Hammock *)</arglist>
    </member>
    <member kind="function" static="yes">
      <type>static void</type>
      <name>LB_hb_add_hammock_blocks</name>
      <anchorfile>lb__hb__region__hammock_8c.html</anchorfile>
      <anchor>74b7f7b6f3526a40c9eb1baf97f8d84a</anchor>
      <arglist>(L_Cb *cb, L_Cb *end, LB_Hammock *hammock)</arglist>
    </member>
    <member kind="function" static="yes">
      <type>static int</type>
      <name>LB_hb_valid_traceregion</name>
      <anchorfile>lb__hb__region__hammock_8c.html</anchorfile>
      <anchor>88aa3f5b95d7b04cb74c2f8471123479</anchor>
      <arglist>(LB_TraceRegion *tr)</arglist>
    </member>
    <member kind="function" static="yes">
      <type>static void</type>
      <name>LB_hb_find_choke_points</name>
      <anchorfile>lb__hb__region__hammock_8c.html</anchorfile>
      <anchor>376d29371303961a9f794b6a9c2b8627</anchor>
      <arglist>(L_Func *fn)</arglist>
    </member>
    <member kind="function" static="yes">
      <type>static void</type>
      <name>LB_hb_find_innermost_blocks</name>
      <anchorfile>lb__hb__region__hammock_8c.html</anchorfile>
      <anchor>b0ddc809da04f302a798f0b92dc67312</anchor>
      <arglist>(L_Func *fn)</arglist>
    </member>
    <member kind="function" static="yes">
      <type>static L_Cb *</type>
      <name>LB_hb_find_immediate_dominator</name>
      <anchorfile>lb__hb__region__hammock_8c.html</anchorfile>
      <anchor>af84477fdcdf0216428ba98b866389a8</anchor>
      <arglist>(L_Cb *cb)</arglist>
    </member>
    <member kind="function" static="yes">
      <type>static L_Cb *</type>
      <name>LB_hb_find_immediate_post_dominator</name>
      <anchorfile>lb__hb__region__hammock_8c.html</anchorfile>
      <anchor>d890ed42b06a48d91b6738745b2cddce</anchor>
      <arglist>(L_Cb *cb)</arglist>
    </member>
    <member kind="function" static="yes">
      <type>static LB_TraceRegion *</type>
      <name>LB_hb_form_hammock_traceregion</name>
      <anchorfile>lb__hb__region__hammock_8c.html</anchorfile>
      <anchor>bd2e889eefdb0334cf8ee1a6c08e2139</anchor>
      <arglist>(LB_Hammock *ham, LB_TraceRegion_Header *header)</arglist>
    </member>
    <member kind="function" static="yes">
      <type>static int</type>
      <name>LB_hb_find_hammock_endpoints</name>
      <anchorfile>lb__hb__region__hammock_8c.html</anchorfile>
      <anchor>ff5aefb7fe5948dea78d0d36efbceda9</anchor>
      <arglist>(L_Cb *s_cb, L_Cb *e_cb, L_Cb **h_start, L_Cb **h_end)</arglist>
    </member>
    <member kind="function" static="yes">
      <type>static int</type>
      <name>LB_hb_expand_hammocks</name>
      <anchorfile>lb__hb__region__hammock_8c.html</anchorfile>
      <anchor>1bfcd5c8b9d397c3bdac3ad39ba51a6c</anchor>
      <arglist>(LB_TraceRegion_Header *header, int ham_level)</arglist>
    </member>
    <member kind="function" static="yes">
      <type>static int</type>
      <name>LB_hb_merge_hammocks</name>
      <anchorfile>lb__hb__region__hammock_8c.html</anchorfile>
      <anchor>f92d2ab20748ed5d15fa52255cc4fae6</anchor>
      <arglist>(LB_TraceRegion_Header *header, int merge_level1, int merge_level2)</arglist>
    </member>
    <member kind="function">
      <type>void</type>
      <name>LB_hb_find_hammock_regions</name>
      <anchorfile>lb__hb__region__hammock_8c.html</anchorfile>
      <anchor>608c87defb672b4295f51cd10b39b4bf</anchor>
      <arglist>(L_Func *fn, LB_TraceRegion_Header *header)</arglist>
    </member>
    <member kind="variable" static="yes">
      <type>static L_Alloc_Pool *</type>
      <name>LB_hb_hammock_pool</name>
      <anchorfile>lb__hb__region__hammock_8c.html</anchorfile>
      <anchor>5550fb33cd73807d0b74a003ee9e9a58</anchor>
      <arglist></arglist>
    </member>
    <member kind="variable" static="yes">
      <type>static List</type>
      <name>LB_hb_hammocks</name>
      <anchorfile>lb__hb__region__hammock_8c.html</anchorfile>
      <anchor>3ba062498d879dff7cd538bd0daea3c9</anchor>
      <arglist></arglist>
    </member>
    <member kind="variable" static="yes">
      <type>static Set</type>
      <name>LB_hb_inner_cb</name>
      <anchorfile>lb__hb__region__hammock_8c.html</anchorfile>
      <anchor>a4e9b75557ab95e25b09a3dba43283e8</anchor>
      <arglist></arglist>
    </member>
    <member kind="variable" static="yes">
      <type>static Set</type>
      <name>LB_hb_choke_cb</name>
      <anchorfile>lb__hb__region__hammock_8c.html</anchorfile>
      <anchor>1b341bf11d33095174ea60eea9451bbb</anchor>
      <arglist></arglist>
    </member>
  </compound>
  <compound kind="file">
    <name>lb_hb_region_hammock.h</name>
    <path>/mnt/hgfs/mkt/Desktop/openimpact/src/Lcode/opti/Lblock/</path>
    <filename>lb__hb__region__hammock_8h</filename>
    <member kind="function">
      <type>void</type>
      <name>LB_hb_find_hammock_regions</name>
      <anchorfile>lb__hb__region__hammock_8h.html</anchorfile>
      <anchor>aa5c507860a75a7786a4cca19d469885</anchor>
      <arglist>(L_Func *, LB_TraceRegion_Header *)</arglist>
    </member>
  </compound>
  <compound kind="file">
    <name>lb_hb_region_loop.c</name>
    <path>/mnt/hgfs/mkt/Desktop/openimpact/src/Lcode/opti/Lblock/</path>
    <filename>lb__hb__region__loop_8c</filename>
    <includes id="lb__hb__hyperblock_8h" name="lb_hb_hyperblock.h" local="yes" imported="no">lb_hb_hyperblock.h</includes>
    <includes id="lb__hb__peel_8h" name="lb_hb_peel.h" local="yes" imported="no">lb_hb_peel.h</includes>
    <member kind="define">
      <type>#define</type>
      <name>LB_MAX_LOOP_SIZE_TO_CONSIDER</name>
      <anchorfile>lb__hb__region__loop_8c.html</anchorfile>
      <anchor>ae1a18a86fa2496c6645b5f1234d1bc9</anchor>
      <arglist></arglist>
    </member>
    <member kind="function" static="yes">
      <type>static List</type>
      <name>LB_subgraph_chokepoints</name>
      <anchorfile>lb__hb__region__loop_8c.html</anchorfile>
      <anchor>f44be0a626b289c26b46cdfdb5241d42</anchor>
      <arglist>(L_Func *fn, L_Cb *start_cb, L_Cb *end_cb, Set blocks)</arglist>
    </member>
    <member kind="function" static="yes">
      <type>static LB_TraceRegion *</type>
      <name>LB_hb_loop_path_selector</name>
      <anchorfile>lb__hb__region__loop_8c.html</anchorfile>
      <anchor>c10711626ac5c6139abd3caf3b0e9f13</anchor>
      <arglist>(L_Func *fn, L_Cb *start_cb, L_Cb *end_cb, Set candidates, LB_TraceRegion_Header *header)</arglist>
    </member>
    <member kind="function">
      <type>void</type>
      <name>LB_hb_find_loop_regions</name>
      <anchorfile>lb__hb__region__loop_8c.html</anchorfile>
      <anchor>4b12bbd221e277227929f54e761c5a1c</anchor>
      <arglist>(L_Func *fn, LB_TraceRegion_Header *header)</arglist>
    </member>
  </compound>
  <compound kind="file">
    <name>lb_hb_region_loop.h</name>
    <path>/mnt/hgfs/mkt/Desktop/openimpact/src/Lcode/opti/Lblock/</path>
    <filename>lb__hb__region__loop_8h</filename>
    <member kind="function">
      <type>void</type>
      <name>LB_hb_find_loop_regions</name>
      <anchorfile>lb__hb__region__loop_8h.html</anchorfile>
      <anchor>bb0596944858aac04be794230c116aea</anchor>
      <arglist>(L_Func *fn, LB_TraceRegion_Header *)</arglist>
    </member>
  </compound>
  <compound kind="file">
    <name>lb_hb_stack.c</name>
    <path>/mnt/hgfs/mkt/Desktop/openimpact/src/Lcode/opti/Lblock/</path>
    <filename>lb__hb__stack_8c</filename>
    <includes id="lb__hb__stack_8h" name="lb_hb_stack.h" local="yes" imported="no">lb_hb_stack.h</includes>
    <member kind="function" static="yes">
      <type>static void</type>
      <name>Stack_punt</name>
      <anchorfile>lb__hb__stack_8c.html</anchorfile>
      <anchor>aa13961ecb03da8976297103f3f60206</anchor>
      <arglist>(char *message)</arglist>
    </member>
    <member kind="function">
      <type>Stack_item *</type>
      <name>Stack_item_create</name>
      <anchorfile>lb__hb__stack_8c.html</anchorfile>
      <anchor>491d085c481280a4da71c0cb086a482b</anchor>
      <arglist>(void)</arglist>
    </member>
    <member kind="function">
      <type>Stack *</type>
      <name>Stack_create</name>
      <anchorfile>lb__hb__stack_8c.html</anchorfile>
      <anchor>fec6172e48d1eeb6d70ca59a5a33d627</anchor>
      <arglist>(void)</arglist>
    </member>
    <member kind="function">
      <type>void</type>
      <name>Stack_item_delete</name>
      <anchorfile>lb__hb__stack_8c.html</anchorfile>
      <anchor>26912d8822203c194d463537e8d0f671</anchor>
      <arglist>(Stack_item *s_item)</arglist>
    </member>
    <member kind="function">
      <type>void</type>
      <name>Stack_delete</name>
      <anchorfile>lb__hb__stack_8c.html</anchorfile>
      <anchor>2ac5380b21f23dc05774db94537145b4</anchor>
      <arglist>(Stack *s)</arglist>
    </member>
    <member kind="function">
      <type>void</type>
      <name>Stack_print</name>
      <anchorfile>lb__hb__stack_8c.html</anchorfile>
      <anchor>241784e58e1ce3774210e115bb890a66</anchor>
      <arglist>(FILE *F, Stack *s)</arglist>
    </member>
    <member kind="function">
      <type>int</type>
      <name>Stack_size</name>
      <anchorfile>lb__hb__stack_8c.html</anchorfile>
      <anchor>b55499af687636831edc2d0266810e38</anchor>
      <arglist>(Stack *s)</arglist>
    </member>
    <member kind="function">
      <type>void</type>
      <name>Stack_clear</name>
      <anchorfile>lb__hb__stack_8c.html</anchorfile>
      <anchor>d5a30a69c4a3debbc0ec887f1fd49536</anchor>
      <arglist>(Stack *s)</arglist>
    </member>
    <member kind="function">
      <type>int</type>
      <name>Stack_in</name>
      <anchorfile>lb__hb__stack_8c.html</anchorfile>
      <anchor>7a9f134adc2ea3643b761aa789086232</anchor>
      <arglist>(Stack *s, int data)</arglist>
    </member>
    <member kind="function">
      <type>int</type>
      <name>Stack_first</name>
      <anchorfile>lb__hb__stack_8c.html</anchorfile>
      <anchor>3b526111c83c552edc54d4c04dcf5c0a</anchor>
      <arglist>(Stack *s)</arglist>
    </member>
    <member kind="function">
      <type>int</type>
      <name>Stack_last</name>
      <anchorfile>lb__hb__stack_8c.html</anchorfile>
      <anchor>894db61e8eb6e465be514428ecaf0de7</anchor>
      <arglist>(Stack *s)</arglist>
    </member>
    <member kind="function">
      <type>int</type>
      <name>Stack_get_contents</name>
      <anchorfile>lb__hb__stack_8c.html</anchorfile>
      <anchor>4e542c1e89698f612b138a99e5d15206</anchor>
      <arglist>(Stack *s, int **buf)</arglist>
    </member>
    <member kind="function">
      <type>Set</type>
      <name>Stack_get_content_set</name>
      <anchorfile>lb__hb__stack_8c.html</anchorfile>
      <anchor>fb4dba65033fe97328b3d38565752e68</anchor>
      <arglist>(Stack *s)</arglist>
    </member>
    <member kind="function">
      <type>void</type>
      <name>Stack_push_top</name>
      <anchorfile>lb__hb__stack_8c.html</anchorfile>
      <anchor>2359f67108fc8d83cfd6c961d0db88ed</anchor>
      <arglist>(Stack *s, int data)</arglist>
    </member>
    <member kind="function">
      <type>void</type>
      <name>Stack_push_bottom</name>
      <anchorfile>lb__hb__stack_8c.html</anchorfile>
      <anchor>543fc765e25b8a44ffb03b28e188c9ab</anchor>
      <arglist>(Stack *s, int data)</arglist>
    </member>
    <member kind="function">
      <type>int</type>
      <name>Stack_pop</name>
      <anchorfile>lb__hb__stack_8c.html</anchorfile>
      <anchor>1eaa73d4286d222b12253e03a2da422a</anchor>
      <arglist>(Stack *s)</arglist>
    </member>
    <member kind="variable" static="yes">
      <type>static L_Alloc_Pool *</type>
      <name>Stack_pool</name>
      <anchorfile>lb__hb__stack_8c.html</anchorfile>
      <anchor>1f5abec0c01aa3997ba48c5fde854ce9</anchor>
      <arglist></arglist>
    </member>
    <member kind="variable" static="yes">
      <type>static L_Alloc_Pool *</type>
      <name>Stack_item_pool</name>
      <anchorfile>lb__hb__stack_8c.html</anchorfile>
      <anchor>2593678b41a1c65e1a1393678e8022a1</anchor>
      <arglist></arglist>
    </member>
  </compound>
  <compound kind="file">
    <name>lb_hb_stack.h</name>
    <path>/mnt/hgfs/mkt/Desktop/openimpact/src/Lcode/opti/Lblock/</path>
    <filename>lb__hb__stack_8h</filename>
    <class kind="struct">_Stack_item</class>
    <class kind="struct">_Stack</class>
    <member kind="typedef">
      <type>_Stack_item</type>
      <name>Stack_item</name>
      <anchorfile>lb__hb__stack_8h.html</anchorfile>
      <anchor>7831e725a9ca6cab24e306e7ee12d8e2</anchor>
      <arglist></arglist>
    </member>
    <member kind="typedef">
      <type>_Stack</type>
      <name>Stack</name>
      <anchorfile>lb__hb__stack_8h.html</anchorfile>
      <anchor>3b3af8164611109cabdd1c7a22aa6eaa</anchor>
      <arglist></arglist>
    </member>
    <member kind="function">
      <type>Stack_item *</type>
      <name>Stack_item_create</name>
      <anchorfile>lb__hb__stack_8h.html</anchorfile>
      <anchor>491d085c481280a4da71c0cb086a482b</anchor>
      <arglist>(void)</arglist>
    </member>
    <member kind="function">
      <type>Stack *</type>
      <name>Stack_create</name>
      <anchorfile>lb__hb__stack_8h.html</anchorfile>
      <anchor>fec6172e48d1eeb6d70ca59a5a33d627</anchor>
      <arglist>(void)</arglist>
    </member>
    <member kind="function">
      <type>void</type>
      <name>Stack_item_delete</name>
      <anchorfile>lb__hb__stack_8h.html</anchorfile>
      <anchor>587e8c0c2341560c46d5341d8e5ae3f9</anchor>
      <arglist>(Stack_item *)</arglist>
    </member>
    <member kind="function">
      <type>void</type>
      <name>Stack_delete</name>
      <anchorfile>lb__hb__stack_8h.html</anchorfile>
      <anchor>970c3e3415bc5c4fded9aaa2edd5aed4</anchor>
      <arglist>(Stack *)</arglist>
    </member>
    <member kind="function">
      <type>void</type>
      <name>Stack_print</name>
      <anchorfile>lb__hb__stack_8h.html</anchorfile>
      <anchor>a889bac01dca13f9f55c52329be0ccff</anchor>
      <arglist>(FILE *, Stack *)</arglist>
    </member>
    <member kind="function">
      <type>int</type>
      <name>Stack_size</name>
      <anchorfile>lb__hb__stack_8h.html</anchorfile>
      <anchor>fc0bef126a287c39d94288d812da5acf</anchor>
      <arglist>(Stack *)</arglist>
    </member>
    <member kind="function">
      <type>int</type>
      <name>Stack_in</name>
      <anchorfile>lb__hb__stack_8h.html</anchorfile>
      <anchor>b6db35797de11501ebe9ad08489ef5b3</anchor>
      <arglist>(Stack *, int)</arglist>
    </member>
    <member kind="function">
      <type>void</type>
      <name>Stack_clear</name>
      <anchorfile>lb__hb__stack_8h.html</anchorfile>
      <anchor>ed78faea040aaa9f13e50cd85584abe0</anchor>
      <arglist>(Stack *)</arglist>
    </member>
    <member kind="function">
      <type>int</type>
      <name>Stack_first</name>
      <anchorfile>lb__hb__stack_8h.html</anchorfile>
      <anchor>a253e024940e75fac526b3bdce3b2c96</anchor>
      <arglist>(Stack *)</arglist>
    </member>
    <member kind="function">
      <type>int</type>
      <name>Stack_last</name>
      <anchorfile>lb__hb__stack_8h.html</anchorfile>
      <anchor>d21e1fd8777809897d252971b7f56012</anchor>
      <arglist>(Stack *)</arglist>
    </member>
    <member kind="function">
      <type>int</type>
      <name>Stack_get_contents</name>
      <anchorfile>lb__hb__stack_8h.html</anchorfile>
      <anchor>8545a27fe04cffbb5fd0ea63c0529b20</anchor>
      <arglist>(Stack *, int **)</arglist>
    </member>
    <member kind="function">
      <type>Set</type>
      <name>Stack_get_content_set</name>
      <anchorfile>lb__hb__stack_8h.html</anchorfile>
      <anchor>13e384d5fece0f5dd14d118224f840e4</anchor>
      <arglist>(Stack *)</arglist>
    </member>
    <member kind="function">
      <type>void</type>
      <name>Stack_push_top</name>
      <anchorfile>lb__hb__stack_8h.html</anchorfile>
      <anchor>dad07eda15b50c1185e10fa04d43000e</anchor>
      <arglist>(Stack *, int)</arglist>
    </member>
    <member kind="function">
      <type>void</type>
      <name>Stack_push_bottom</name>
      <anchorfile>lb__hb__stack_8h.html</anchorfile>
      <anchor>f757dad0baa031f7d829d96a4517972d</anchor>
      <arglist>(Stack *, int)</arglist>
    </member>
    <member kind="function">
      <type>int</type>
      <name>Stack_pop</name>
      <anchorfile>lb__hb__stack_8h.html</anchorfile>
      <anchor>82482c53fe90b77b457e45d68c23d97e</anchor>
      <arglist>(Stack *)</arglist>
    </member>
  </compound>
  <compound kind="file">
    <name>lb_mod_loop.c</name>
    <path>/mnt/hgfs/mkt/Desktop/openimpact/src/Lcode/opti/Lblock/</path>
    <filename>lb__mod__loop_8c</filename>
    <includes id="lb__hb__hyperblock_8h" name="lb_hb_hyperblock.h" local="yes" imported="no">lb_hb_hyperblock.h</includes>
    <includes id="lb__b__internal_8h" name="lb_b_internal.h" local="yes" imported="no">lb_b_internal.h</includes>
    <includes id="lb__flow_8h" name="lb_flow.h" local="yes" imported="no">lb_flow.h</includes>
    <includes id="lb__hb__peel_8h" name="lb_hb_peel.h" local="yes" imported="no">lb_hb_peel.h</includes>
    <member kind="define">
      <type>#define</type>
      <name>ERR</name>
      <anchorfile>lb__mod__loop_8c.html</anchorfile>
      <anchor>735563036dced0b7d6cc98f97ea4978b</anchor>
      <arglist></arglist>
    </member>
    <member kind="define">
      <type>#define</type>
      <name>LB_COLLAPSE_MAX_DYN_EXPN</name>
      <anchorfile>lb__mod__loop_8c.html</anchorfile>
      <anchor>9d175e0803884d49c3fbd996f8b6fa22</anchor>
      <arglist></arglist>
    </member>
    <member kind="function" static="yes">
      <type>static void</type>
      <name>LB_reconnect_blocks</name>
      <anchorfile>lb__mod__loop_8c.html</anchorfile>
      <anchor>8b3ba93b8dc3746660f6bc89d950b1a8</anchor>
      <arglist>(L_Cb *src_cb, L_Cb *old_dst_cb, L_Cb *new_dst_cb)</arglist>
    </member>
    <member kind="function" static="yes">
      <type>static int</type>
      <name>LB_are_equivalent_cbs</name>
      <anchorfile>lb__mod__loop_8c.html</anchorfile>
      <anchor>9c3e6e4b7f5babdf70879cd5fdf81dfe</anchor>
      <arglist>(L_Cb *cb1, L_Cb *cb2)</arglist>
    </member>
    <member kind="function" static="yes">
      <type>static int</type>
      <name>LB_functionally_equivalent_cbs</name>
      <anchorfile>lb__mod__loop_8c.html</anchorfile>
      <anchor>cfeeb6442b2a01f205fe4bced14442a4</anchor>
      <arglist>(Set cb_set)</arglist>
    </member>
    <member kind="function" static="yes">
      <type>static void</type>
      <name>LB_combine_equivalent_backedges</name>
      <anchorfile>lb__mod__loop_8c.html</anchorfile>
      <anchor>c0d5331128c595ec1e36135ec5919274</anchor>
      <arglist>(L_Loop *loop)</arglist>
    </member>
    <member kind="function">
      <type>void</type>
      <name>LB_elim_loop_backedges</name>
      <anchorfile>lb__mod__loop_8c.html</anchorfile>
      <anchor>294b341d77d72f68564104ae5a935a98</anchor>
      <arglist>(L_Func *fn, L_Loop *loop)</arglist>
    </member>
    <member kind="function">
      <type>void</type>
      <name>LB_elim_all_loop_backedges</name>
      <anchorfile>lb__mod__loop_8c.html</anchorfile>
      <anchor>41b44967d70bd76250cce3a9cfb9594a</anchor>
      <arglist>(L_Func *fn)</arglist>
    </member>
    <member kind="function">
      <type>int</type>
      <name>LB_hb_loop_collapsible</name>
      <anchorfile>lb__mod__loop_8c.html</anchorfile>
      <anchor>3882ff9e271cd4b04929ad9af62ebfe9</anchor>
      <arglist>(LB_TraceRegion_Header *trh, L_Loop *loop)</arglist>
    </member>
    <member kind="function">
      <type>L_Loop *</type>
      <name>LB_hb_do_collapse_loops</name>
      <anchorfile>lb__mod__loop_8c.html</anchorfile>
      <anchor>b7061e92465878b60e96119a6b161fb1</anchor>
      <arglist>(LB_TraceRegion_Header *trh, L_Func *fn, L_Loop *loop)</arglist>
    </member>
  </compound>
  <compound kind="file">
    <name>lb_peel.c</name>
    <path>/mnt/hgfs/mkt/Desktop/openimpact/src/Lcode/opti/Lblock/</path>
    <filename>lb__peel_8c</filename>
    <includes id="lb__b__internal_8h" name="lb_b_internal.h" local="yes" imported="no">lb_b_internal.h</includes>
    <includes id="lb__peel_8h" name="lb_peel.h" local="yes" imported="no">lb_peel.h</includes>
    <member kind="define">
      <type>#define</type>
      <name>ERR</name>
      <anchorfile>lb__peel_8c.html</anchorfile>
      <anchor>735563036dced0b7d6cc98f97ea4978b</anchor>
      <arglist></arglist>
    </member>
    <member kind="function" static="yes">
      <type>static void</type>
      <name>LB_adjust_weight_for_peel</name>
      <anchorfile>lb__peel_8c.html</anchorfile>
      <anchor>ce0ea43554242414ac26133f87a58b50</anchor>
      <arglist>(L_Func *fn, L_Loop *loop, double *pinvoc_wt, double *piter_wt, double *precov_wt)</arglist>
    </member>
    <member kind="function" static="yes">
      <type>static void</type>
      <name>LB_attach_peel_attribute</name>
      <anchorfile>lb__peel_8c.html</anchorfile>
      <anchor>0bcfdfb2fb4bb22462438dfb883a93b5</anchor>
      <arglist>(L_Cb *cb, int iter_num, int peel_loop_id)</arglist>
    </member>
    <member kind="function">
      <type>void</type>
      <name>LB_peel_loop</name>
      <anchorfile>lb__peel_8c.html</anchorfile>
      <anchor>144d6344da1c2103963900b3a9540dc5</anchor>
      <arglist>(L_Func *fn, L_Loop *loop, Set peel_cbs, Set region_cbs, int peel_id, int peel_num, Set *peeled_cbs, Set *mod_cbs)</arglist>
    </member>
  </compound>
  <compound kind="file">
    <name>lb_peel.h</name>
    <path>/mnt/hgfs/mkt/Desktop/openimpact/src/Lcode/opti/Lblock/</path>
    <filename>lb__peel_8h</filename>
    <member kind="function">
      <type>void</type>
      <name>LB_peel_loop</name>
      <anchorfile>lb__peel_8h.html</anchorfile>
      <anchor>144d6344da1c2103963900b3a9540dc5</anchor>
      <arglist>(L_Func *fn, L_Loop *loop, Set peel_cbs, Set region_cbs, int peel_id, int peel_num, Set *peeled_cbs, Set *mod_cbs)</arglist>
    </member>
  </compound>
  <compound kind="file">
    <name>lb_pred_tools.c</name>
    <path>/mnt/hgfs/mkt/Desktop/openimpact/src/Lcode/opti/Lblock/</path>
    <filename>lb__pred__tools_8c</filename>
    <includes id="lb__b__internal_8h" name="lb_b_internal.h" local="yes" imported="no">lb_b_internal.h</includes>
    <class kind="struct">_LB_Predicate</class>
    <member kind="define">
      <type>#define</type>
      <name>LB_FLOW_BACKWARD</name>
      <anchorfile>lb__pred__tools_8c.html</anchorfile>
      <anchor>13ac919be95e7fb015129edd88d56136</anchor>
      <arglist></arglist>
    </member>
    <member kind="define">
      <type>#define</type>
      <name>LB_FLOW_EXIT</name>
      <anchorfile>lb__pred__tools_8c.html</anchorfile>
      <anchor>110df4e2bdeaff63c7aac52b695197b3</anchor>
      <arglist></arglist>
    </member>
    <member kind="define">
      <type>#define</type>
      <name>LB_FLOW_FORWARD</name>
      <anchorfile>lb__pred__tools_8c.html</anchorfile>
      <anchor>260ac92a08d8db8a6cfa393f3dcd6d34</anchor>
      <arglist></arglist>
    </member>
    <member kind="typedef">
      <type>_LB_Predicate</type>
      <name>LB_Predicate</name>
      <anchorfile>lb__pred__tools_8c.html</anchorfile>
      <anchor>c91a4f2f897bbef6be7fe2183df04b22</anchor>
      <arglist></arglist>
    </member>
    <member kind="function">
      <type>void</type>
      <name>LB_predicate_init</name>
      <anchorfile>lb__pred__tools_8c.html</anchorfile>
      <anchor>9fb064b7a49236575bc546574b88551e</anchor>
      <arglist>()</arglist>
    </member>
    <member kind="function">
      <type>void</type>
      <name>LB_predicate_deinit</name>
      <anchorfile>lb__pred__tools_8c.html</anchorfile>
      <anchor>b615b4416302d98f75dc71410a80c613</anchor>
      <arglist>(L_Func *fn)</arglist>
    </member>
    <member kind="function" static="yes">
      <type>static LB_Predicate *</type>
      <name>LB_create_predicate</name>
      <anchorfile>lb__pred__tools_8c.html</anchorfile>
      <anchor>92a9d4ec7dcb6d1645c364ace251f920</anchor>
      <arglist>(L_Func *fn)</arglist>
    </member>
    <member kind="function">
      <type>void</type>
      <name>LB_free_predicate</name>
      <anchorfile>lb__pred__tools_8c.html</anchorfile>
      <anchor>f4ecd648980c472610b70125f81c5a8e</anchor>
      <arglist>(LB_Predicate *pred)</arglist>
    </member>
    <member kind="function" static="yes">
      <type>static void</type>
      <name>LB_predicate_clear</name>
      <anchorfile>lb__pred__tools_8c.html</anchorfile>
      <anchor>8a4da89c3f99b3fd8d27fc9d83890826</anchor>
      <arglist>(L_Func *fn, LB_TraceRegion *tr)</arglist>
    </member>
    <member kind="function" static="yes">
      <type>static void</type>
      <name>LB_predicate_define</name>
      <anchorfile>lb__pred__tools_8c.html</anchorfile>
      <anchor>42785106560ade2fe5fda70ffd802040</anchor>
      <arglist>(LB_TraceRegion *tr)</arglist>
    </member>
    <member kind="function" static="yes">
      <type>static int</type>
      <name>LB_flow_type</name>
      <anchorfile>lb__pred__tools_8c.html</anchorfile>
      <anchor>03a9c0e1d25411194e0b695366a2177e</anchor>
      <arglist>(L_Flow *flow, LB_TraceRegion *tr)</arglist>
    </member>
    <member kind="function" static="yes">
      <type>static void</type>
      <name>LB_predicate_this_oper</name>
      <anchorfile>lb__pred__tools_8c.html</anchorfile>
      <anchor>4d774f97017ef01d8b67e8ed897edb58</anchor>
      <arglist>(L_Cb *cb, L_Oper *oper, LB_Predicate *pred, double weight, double taken_weight)</arglist>
    </member>
    <member kind="function" static="yes">
      <type>static void</type>
      <name>LB_predicate_branch</name>
      <anchorfile>lb__pred__tools_8c.html</anchorfile>
      <anchor>1c56ab0e09fe6a6a85d683615cd54c95</anchor>
      <arglist>(LB_TraceRegion *tr, L_Cb *cb, L_Oper *oper, LB_Predicate *pred)</arglist>
    </member>
    <member kind="function" static="yes">
      <type>static void</type>
      <name>LB_add_fall_thru_flow_arc</name>
      <anchorfile>lb__pred__tools_8c.html</anchorfile>
      <anchor>2bd1e836433db32596a4d5cd4df470bc</anchor>
      <arglist>(L_Cb *cb)</arglist>
    </member>
    <member kind="function" static="yes">
      <type>static void</type>
      <name>LB_add_dummy_block_to_end_of_fn</name>
      <anchorfile>lb__pred__tools_8c.html</anchorfile>
      <anchor>837f811cc501aa9fa494136c5c470337</anchor>
      <arglist>(L_Func *fn, L_Cb *cb)</arglist>
    </member>
    <member kind="function" static="yes">
      <type>static void</type>
      <name>LB_predicate_region</name>
      <anchorfile>lb__pred__tools_8c.html</anchorfile>
      <anchor>5c1b1c9337e04bfac7877b708523a467</anchor>
      <arglist>(L_Func *fn, LB_TraceRegion *tr)</arglist>
    </member>
    <member kind="function" static="yes">
      <type>static void</type>
      <name>LB_combine_tr_exits</name>
      <anchorfile>lb__pred__tools_8c.html</anchorfile>
      <anchor>f8b860f20df153fb019d4cf57c6347c5</anchor>
      <arglist>(LB_TraceRegion *tr)</arglist>
    </member>
    <member kind="function" static="yes">
      <type>static int</type>
      <name>LB_materialize_tr_exits</name>
      <anchorfile>lb__pred__tools_8c.html</anchorfile>
      <anchor>4b88dd5505d351c2d28e4baa73d46b30</anchor>
      <arglist>(LB_TraceRegion *tr)</arglist>
    </member>
    <member kind="function" static="yes">
      <type>static void</type>
      <name>LB_mark_traceregion</name>
      <anchorfile>lb__pred__tools_8c.html</anchorfile>
      <anchor>498062a33d24f7c9c263aa80529e3f4e</anchor>
      <arglist>(L_Func *fn, LB_TraceRegion *tr)</arglist>
    </member>
    <member kind="function">
      <type>void</type>
      <name>LB_predicate_traceregion</name>
      <anchorfile>lb__pred__tools_8c.html</anchorfile>
      <anchor>ceee24e7aed6b36adfbb41dc6bdbd172</anchor>
      <arglist>(L_Func *fn, LB_TraceRegion *tr, int do_frp)</arglist>
    </member>
    <member kind="function">
      <type>void</type>
      <name>LB_predicate_traceregions</name>
      <anchorfile>lb__pred__tools_8c.html</anchorfile>
      <anchor>7ef1d1f24f22b3b56a209dd69e3d2362</anchor>
      <arglist>(L_Func *fn, LB_TraceRegion_Header *header)</arglist>
    </member>
    <member kind="function">
      <type>void</type>
      <name>LB_set_hyperblock_flag</name>
      <anchorfile>lb__pred__tools_8c.html</anchorfile>
      <anchor>582b676f368f4137f5def092dbd42ebd</anchor>
      <arglist>(L_Func *fn)</arglist>
    </member>
    <member kind="function">
      <type>void</type>
      <name>LB_remove_unnec_hyperblock_flags</name>
      <anchorfile>lb__pred__tools_8c.html</anchorfile>
      <anchor>5520951845c8d0bdebb3078bd000fa41</anchor>
      <arglist>(L_Func *fn)</arglist>
    </member>
    <member kind="function">
      <type>void</type>
      <name>LB_set_hyperblock_func_flag</name>
      <anchorfile>lb__pred__tools_8c.html</anchorfile>
      <anchor>435b7ad780234fb8c337b2c77d94a7aa</anchor>
      <arglist>(L_Func *fn)</arglist>
    </member>
    <member kind="function">
      <type>void</type>
      <name>LB_clr_hyperblock_flag</name>
      <anchorfile>lb__pred__tools_8c.html</anchorfile>
      <anchor>a8b80523aac6053c8ad194a010c94bcf</anchor>
      <arglist>(L_Func *fn)</arglist>
    </member>
    <member kind="variable" static="yes">
      <type>static L_Cb *</type>
      <name>TEMP_CB</name>
      <anchorfile>lb__pred__tools_8c.html</anchorfile>
      <anchor>00b50e631ed82490574d2c8541e8b09e</anchor>
      <arglist></arglist>
    </member>
    <member kind="variable" static="yes">
      <type>static L_Alloc_Pool *</type>
      <name>LB_Predicate_Pool</name>
      <anchorfile>lb__pred__tools_8c.html</anchorfile>
      <anchor>1d6b4807b6a5e0eefa148ddd26cc7dbf</anchor>
      <arglist></arglist>
    </member>
  </compound>
  <compound kind="file">
    <name>lb_pred_tools.h</name>
    <path>/mnt/hgfs/mkt/Desktop/openimpact/src/Lcode/opti/Lblock/</path>
    <filename>lb__pred__tools_8h</filename>
    <member kind="function">
      <type>void</type>
      <name>LB_predicate_init</name>
      <anchorfile>lb__pred__tools_8h.html</anchorfile>
      <anchor>a6b4b64e0f42c290e7b8e2e0bcdd3cb3</anchor>
      <arglist>(void)</arglist>
    </member>
    <member kind="function">
      <type>void</type>
      <name>LB_predicate_deinit</name>
      <anchorfile>lb__pred__tools_8h.html</anchorfile>
      <anchor>499e310905b588970ec1f37ae6265338</anchor>
      <arglist>(L_Func *)</arglist>
    </member>
    <member kind="function">
      <type>void</type>
      <name>LB_free_predicate</name>
      <anchorfile>lb__pred__tools_8h.html</anchorfile>
      <anchor>981ac19ecffecd40fdde836aca2db018</anchor>
      <arglist>(struct _LB_Predicate *)</arglist>
    </member>
    <member kind="function">
      <type>void</type>
      <name>LB_predicate_traceregions</name>
      <anchorfile>lb__pred__tools_8h.html</anchorfile>
      <anchor>0bae19bf7d81458dfa890a8a5735187f</anchor>
      <arglist>(L_Func *, LB_TraceRegion_Header *)</arglist>
    </member>
    <member kind="function">
      <type>void</type>
      <name>LB_predicate_traceregion</name>
      <anchorfile>lb__pred__tools_8h.html</anchorfile>
      <anchor>764cd4c3096f7915b9af3838a06f2cfc</anchor>
      <arglist>(L_Func *, LB_TraceRegion *, int do_frp)</arglist>
    </member>
    <member kind="function">
      <type>void</type>
      <name>LB_set_hyperblock_func_flag</name>
      <anchorfile>lb__pred__tools_8h.html</anchorfile>
      <anchor>63e9d775e7f01ede03071562c60b0b9c</anchor>
      <arglist>(L_Func *)</arglist>
    </member>
    <member kind="function">
      <type>void</type>
      <name>LB_set_hyperblock_flag</name>
      <anchorfile>lb__pred__tools_8h.html</anchorfile>
      <anchor>2626a92c5595326da3292178ed807a78</anchor>
      <arglist>(L_Func *)</arglist>
    </member>
    <member kind="function">
      <type>void</type>
      <name>LB_clr_hyperblock_flag</name>
      <anchorfile>lb__pred__tools_8h.html</anchorfile>
      <anchor>63d0a332f9496493a3a44ad72c1fbcd0</anchor>
      <arglist>(L_Func *)</arglist>
    </member>
    <member kind="function">
      <type>void</type>
      <name>LB_remove_unnec_hyperblock_flags</name>
      <anchorfile>lb__pred__tools_8h.html</anchorfile>
      <anchor>9129f0cff00f6cecaee907569740420a</anchor>
      <arglist>(L_Func *)</arglist>
    </member>
  </compound>
  <compound kind="file">
    <name>lb_register_branch.c</name>
    <path>/mnt/hgfs/mkt/Desktop/openimpact/src/Lcode/opti/Lblock/</path>
    <filename>lb__register__branch_8c</filename>
    <includes id="lb__flow_8h" name="lb_flow.h" local="yes" imported="no">lb_flow.h</includes>
    <member kind="function">
      <type>int</type>
      <name>LB_cb_contains_register_branch</name>
      <anchorfile>lb__register__branch_8c.html</anchorfile>
      <anchor>4add1211c3f03bdaa585abef62834663</anchor>
      <arglist>(L_Cb *cb)</arglist>
    </member>
    <member kind="function">
      <type>void</type>
      <name>LB_mark_jrg_flag</name>
      <anchorfile>lb__register__branch_8c.html</anchorfile>
      <anchor>b8c32f6d1411b8ed007f054ed2cee74d</anchor>
      <arglist>(L_Func *fn)</arglist>
    </member>
  </compound>
  <compound kind="file">
    <name>lb_sb_layoutfn.c</name>
    <path>/mnt/hgfs/mkt/Desktop/openimpact/src/Lcode/opti/Lblock/</path>
    <filename>lb__sb__layoutfn_8c</filename>
    <includes id="lb__sb__superblock_8h" name="lb_sb_superblock.h" local="yes" imported="no">lb_sb_superblock.h</includes>
    <member kind="function" static="yes">
      <type>static int</type>
      <name>LB_switch_taken_and_fall_thru_path</name>
      <anchorfile>lb__sb__layoutfn_8c.html</anchorfile>
      <anchor>f1628bc72cb8a8a3ed8d9ee7d23418e7</anchor>
      <arglist>(L_Cb *cb)</arglist>
    </member>
    <member kind="function">
      <type>void</type>
      <name>LB_correct_flow_inst</name>
      <anchorfile>lb__sb__layoutfn_8c.html</anchorfile>
      <anchor>baa699d9f243eed8da2a9d8659b81073</anchor>
      <arglist>(L_Cb *src_cb)</arglist>
    </member>
    <member kind="function">
      <type>void</type>
      <name>LB_layout_function</name>
      <anchorfile>lb__sb__layoutfn_8c.html</anchorfile>
      <anchor>563472d24433ea5cb9c9db98e962cf2c</anchor>
      <arglist>(L_Func *fn, LB_TraceRegion_Header *header)</arglist>
    </member>
  </compound>
  <compound kind="file">
    <name>lb_sb_make_traceregions.c</name>
    <path>/mnt/hgfs/mkt/Desktop/openimpact/src/Lcode/opti/Lblock/</path>
    <filename>lb__sb__make__traceregions_8c</filename>
    <includes id="lb__sb__superblock_8h" name="lb_sb_superblock.h" local="yes" imported="no">lb_sb_superblock.h</includes>
    <member kind="function" static="yes">
      <type>static void</type>
      <name>LB_clear_cb_visit_flags</name>
      <anchorfile>lb__sb__make__traceregions_8c.html</anchorfile>
      <anchor>a3632b98606b387f4accab8f94d829ed</anchor>
      <arglist>(L_Func *fn, LB_TraceRegion_Header *header)</arglist>
    </member>
    <member kind="function" static="yes">
      <type>static void</type>
      <name>LB_mark_boundary_blocks_visited</name>
      <anchorfile>lb__sb__make__traceregions_8c.html</anchorfile>
      <anchor>632c0d42b43c0574c7b33c2aa4a7cb38</anchor>
      <arglist>(L_Func *fn, LB_TraceRegion_Header *header)</arglist>
    </member>
    <member kind="function" static="yes">
      <type>static L_Cb *</type>
      <name>LB_find_epilog_cb</name>
      <anchorfile>lb__sb__make__traceregions_8c.html</anchorfile>
      <anchor>e7fdf2272e6b5ab4423d9c13f41e9a4e</anchor>
      <arglist>(L_Func *fn)</arglist>
    </member>
    <member kind="function" static="yes">
      <type>static L_Cb *</type>
      <name>LB_find_cb_seed</name>
      <anchorfile>lb__sb__make__traceregions_8c.html</anchorfile>
      <anchor>44f980f3326a3db5af4046f6055455e2</anchor>
      <arglist>(L_Func *fn)</arglist>
    </member>
    <member kind="function" static="yes">
      <type>static int</type>
      <name>LB_unvisited_cbs</name>
      <anchorfile>lb__sb__make__traceregions_8c.html</anchorfile>
      <anchor>e71070c5af44517758f3a2e4ff125a22</anchor>
      <arglist>(L_Func *fn)</arglist>
    </member>
    <member kind="function" static="yes">
      <type>static L_Cb *</type>
      <name>LB_best_cb_successor_of</name>
      <anchorfile>lb__sb__make__traceregions_8c.html</anchorfile>
      <anchor>e812d842bc43647abfe10b68c86aa367</anchor>
      <arglist>(L_Cb *current, int check_for_backedges)</arglist>
    </member>
    <member kind="function" static="yes">
      <type>static L_Cb *</type>
      <name>LB_best_cb_predecessor_of</name>
      <anchorfile>lb__sb__make__traceregions_8c.html</anchorfile>
      <anchor>e3872f1495b5e98e20c2ffa0a3b717a0</anchor>
      <arglist>(L_Cb *current, int check_for_backedges)</arglist>
    </member>
    <member kind="function" static="yes">
      <type>static void</type>
      <name>LB_make_super_hyper_block_traces</name>
      <anchorfile>lb__sb__make__traceregions_8c.html</anchorfile>
      <anchor>a3315af76d08059d7610ae39a33644f3</anchor>
      <arglist>(L_Func *fn, LB_TraceRegion_Header *header)</arglist>
    </member>
    <member kind="function">
      <type>void</type>
      <name>LB_make_traceregions</name>
      <anchorfile>lb__sb__make__traceregions_8c.html</anchorfile>
      <anchor>ecd0eeaab498cbf33a6916160b6e5a82</anchor>
      <arglist>(L_Func *fn, int check_for_backedges, LB_TraceRegion_Header *header)</arglist>
    </member>
  </compound>
  <compound kind="file">
    <name>lb_sb_order_traceregions.c</name>
    <path>/mnt/hgfs/mkt/Desktop/openimpact/src/Lcode/opti/Lblock/</path>
    <filename>lb__sb__order__traceregions_8c</filename>
    <includes id="lb__sb__superblock_8h" name="lb_sb_superblock.h" local="yes" imported="no">lb_sb_superblock.h</includes>
    <member kind="function" static="yes">
      <type>static LB_TraceRegion *</type>
      <name>LB_find_prolog_traceregion</name>
      <anchorfile>lb__sb__order__traceregions_8c.html</anchorfile>
      <anchor>1368d9edc9ca26e0f4fdfd1bbac20315</anchor>
      <arglist>(L_Func *fn, LB_TraceRegion_Header *header)</arglist>
    </member>
    <member kind="function" static="yes">
      <type>static LB_TraceRegion *</type>
      <name>LB_find_traceregion_seed</name>
      <anchorfile>lb__sb__order__traceregions_8c.html</anchorfile>
      <anchor>8e0d33a441f0626da51d1db7cf713a1b</anchor>
      <arglist>(L_Func *fn, LB_TraceRegion_Header *header)</arglist>
    </member>
    <member kind="function" static="yes">
      <type>static LB_TraceRegion *</type>
      <name>LB_jump_rg_most_taken_traceregion</name>
      <anchorfile>lb__sb__order__traceregions_8c.html</anchorfile>
      <anchor>ea89edee82f66677c1c5f82a9c6f0a7b</anchor>
      <arglist>(L_Cb *cb, LB_TraceRegion_Header *header)</arglist>
    </member>
    <member kind="function" static="yes">
      <type>static LB_TraceRegion *</type>
      <name>LB_best_traceregion_successor_of</name>
      <anchorfile>lb__sb__order__traceregions_8c.html</anchorfile>
      <anchor>a078933b3d543bdc58694cdac20ab63c</anchor>
      <arglist>(LB_TraceRegion *current, LB_TraceRegion_Header *header)</arglist>
    </member>
    <member kind="function" static="yes">
      <type>static LB_TraceRegion *</type>
      <name>LB_best_traceregion_predecessor_of</name>
      <anchorfile>lb__sb__order__traceregions_8c.html</anchorfile>
      <anchor>758887037d3e7b6c3dcb415b355319f1</anchor>
      <arglist>(LB_TraceRegion *current, LB_TraceRegion_Header *header)</arglist>
    </member>
    <member kind="function">
      <type>void</type>
      <name>LB_order_traceregions</name>
      <anchorfile>lb__sb__order__traceregions_8c.html</anchorfile>
      <anchor>cf2a45888836b9701c33b36365ca55bb</anchor>
      <arglist>(L_Func *fn, LB_TraceRegion_Header *header)</arglist>
    </member>
  </compound>
  <compound kind="file">
    <name>lb_sb_superblock.h</name>
    <path>/mnt/hgfs/mkt/Desktop/openimpact/src/Lcode/opti/Lblock/</path>
    <filename>lb__sb__superblock_8h</filename>
    <includes id="lb__b__internal_8h" name="lb_b_internal.h" local="yes" imported="no">lb_b_internal.h</includes>
    <member kind="function">
      <type>void</type>
      <name>LB_make_traceregions</name>
      <anchorfile>lb__sb__superblock_8h.html</anchorfile>
      <anchor>d57a700cfd7be9ed4332e00515975f58</anchor>
      <arglist>(L_Func *, int, LB_TraceRegion_Header *)</arglist>
    </member>
    <member kind="function">
      <type>void</type>
      <name>LB_order_traceregions</name>
      <anchorfile>lb__sb__superblock_8h.html</anchorfile>
      <anchor>80c940f7b4befdadc369972e2151ddf1</anchor>
      <arglist>(L_Func *, LB_TraceRegion_Header *)</arglist>
    </member>
    <member kind="function">
      <type>void</type>
      <name>LB_layout_function</name>
      <anchorfile>lb__sb__superblock_8h.html</anchorfile>
      <anchor>cb4ec16e4b7a55f9b1c146e77e36720d</anchor>
      <arglist>(L_Func *, LB_TraceRegion_Header *)</arglist>
    </member>
  </compound>
  <compound kind="file">
    <name>lb_sb_superblock_former.c</name>
    <path>/mnt/hgfs/mkt/Desktop/openimpact/src/Lcode/opti/Lblock/</path>
    <filename>lb__sb__superblock__former_8c</filename>
    <includes id="lb__sb__superblock_8h" name="lb_sb_superblock.h" local="yes" imported="no">lb_sb_superblock.h</includes>
    <member kind="function" static="yes">
      <type>static int</type>
      <name>LB_traceregion_code_size</name>
      <anchorfile>lb__sb__superblock__former_8c.html</anchorfile>
      <anchor>fa2d6d4dab3de019bdbc2ad028079f1f</anchor>
      <arglist>(LB_TraceRegion *tr)</arglist>
    </member>
    <member kind="function">
      <type>void</type>
      <name>LB_superblock_formation</name>
      <anchorfile>lb__sb__superblock__former_8c.html</anchorfile>
      <anchor>9da934de4e70da422f9f8a4e91ea80e5</anchor>
      <arglist>(L_Func *fn)</arglist>
    </member>
    <member kind="function">
      <type>void</type>
      <name>LB_code_layout</name>
      <anchorfile>lb__sb__superblock__former_8c.html</anchorfile>
      <anchor>ea5d38718847b3ddfde45402f0678f84</anchor>
      <arglist>(L_Func *fn)</arglist>
    </member>
  </compound>
  <compound kind="file">
    <name>lb_strict_bb.c</name>
    <path>/mnt/hgfs/mkt/Desktop/openimpact/src/Lcode/opti/Lblock/</path>
    <filename>lb__strict__bb_8c</filename>
    <includes id="lb__flow_8h" name="lb_flow.h" local="yes" imported="no">lb_flow.h</includes>
    <member kind="function">
      <type>L_Cb *</type>
      <name>LB_breakup_cb</name>
      <anchorfile>lb__strict__bb_8c.html</anchorfile>
      <anchor>8783955b655b6d81f2053d04367a0478</anchor>
      <arglist>(L_Func *fn, L_Cb *cb)</arglist>
    </member>
    <member kind="function">
      <type>void</type>
      <name>LB_convert_to_strict_basic_block_code</name>
      <anchorfile>lb__strict__bb_8c.html</anchorfile>
      <anchor>0b0243d5c913ec44316b893621fe431c</anchor>
      <arglist>(L_Func *fn, int exclude_flags)</arglist>
    </member>
  </compound>
  <compound kind="file">
    <name>lb_tail.c</name>
    <path>/mnt/hgfs/mkt/Desktop/openimpact/src/Lcode/opti/Lblock/</path>
    <filename>lb__tail_8c</filename>
    <includes id="lb__b__internal_8h" name="lb_b_internal.h" local="yes" imported="no">lb_b_internal.h</includes>
    <member kind="function" static="yes">
      <type>static void</type>
      <name>LB_find_duplicate_blocks</name>
      <anchorfile>lb__tail_8c.html</anchorfile>
      <anchor>8782559cfb0a7dba3d3bac3b36ec7387</anchor>
      <arglist>(L_Cb *cb, LB_TraceRegion *tr)</arglist>
    </member>
    <member kind="function" static="yes">
      <type>static int</type>
      <name>LB_hb_find_loops_for_dup_rcr</name>
      <anchorfile>lb__tail_8c.html</anchorfile>
      <anchor>67788d2dd71515ac6beab6695f98f15f</anchor>
      <arglist>(LB_TraceRegion *tr, L_Cb *cb)</arglist>
    </member>
    <member kind="function" static="yes">
      <type>static int</type>
      <name>LB_hb_find_loops_for_dup</name>
      <anchorfile>lb__tail_8c.html</anchorfile>
      <anchor>c3ee1a9da5b9d9e9c588826955adfe6b</anchor>
      <arglist>(LB_TraceRegion *tr)</arglist>
    </member>
    <member kind="function">
      <type>void</type>
      <name>LB_scale_incl_dest_flows</name>
      <anchorfile>lb__tail_8c.html</anchorfile>
      <anchor>9678c6404700cc0ccfc0491187f91a4c</anchor>
      <arglist>(L_Cb *cb, double ratio)</arglist>
    </member>
    <member kind="function">
      <type>void</type>
      <name>LB_adjust_copied_region_weight</name>
      <anchorfile>lb__tail_8c.html</anchorfile>
      <anchor>596459e8f16ed6611808440435f85858</anchor>
      <arglist>(LB_TraceRegion *tr, List inflows)</arglist>
    </member>
    <member kind="function">
      <type>int</type>
      <name>LB_tail_duplication</name>
      <anchorfile>lb__tail_8c.html</anchorfile>
      <anchor>98d5249bfe2799f4c496463837616c57</anchor>
      <arglist>(L_Func *fn, LB_TraceRegion *tr, int flag, int measure_only)</arglist>
    </member>
    <member kind="function" static="yes">
      <type>static int</type>
      <name>LB_tail_translate</name>
      <anchorfile>lb__tail_8c.html</anchorfile>
      <anchor>9b404733b66e5422402e0816fe18ab52</anchor>
      <arglist>(L_Func *fn, LB_TraceRegion *tr)</arglist>
    </member>
    <member kind="function">
      <type>int</type>
      <name>LB_tail_duplicate</name>
      <anchorfile>lb__tail_8c.html</anchorfile>
      <anchor>756200873e99952043f54e55486cba4f</anchor>
      <arglist>(L_Func *fn, LB_TraceRegion_Header *header, int flag)</arglist>
    </member>
    <member kind="function">
      <type>void</type>
      <name>LB_tail_duplication_codesize</name>
      <anchorfile>lb__tail_8c.html</anchorfile>
      <anchor>cb8c3582dd8ae05bf58e9c14a1a1aa53</anchor>
      <arglist>(L_Func *fn, LB_TraceRegion *tr, int flag)</arglist>
    </member>
    <member kind="variable" static="yes">
      <type>static Set</type>
      <name>LB_duplicate</name>
      <anchorfile>lb__tail_8c.html</anchorfile>
      <anchor>7151dae4880d37aa3eaa8ee1d3965aee</anchor>
      <arglist></arglist>
    </member>
    <member kind="variable" static="yes">
      <type>static Set</type>
      <name>LB_side_entry</name>
      <anchorfile>lb__tail_8c.html</anchorfile>
      <anchor>13f8246ddcd7fc6eecb75a735a1609a6</anchor>
      <arglist></arglist>
    </member>
    <member kind="variable" static="yes">
      <type>static List</type>
      <name>LB_side_in_flows</name>
      <anchorfile>lb__tail_8c.html</anchorfile>
      <anchor>0b6f19205df4d64a4e0d77fa0c37446f</anchor>
      <arglist></arglist>
    </member>
    <member kind="variable" static="yes">
      <type>static List</type>
      <name>LB_change_flows</name>
      <anchorfile>lb__tail_8c.html</anchorfile>
      <anchor>157dab3cbf7b53ac09429bd74adf56c2</anchor>
      <arglist></arglist>
    </member>
    <member kind="variable" static="yes">
      <type>static HashTable</type>
      <name>LB_dup_trans</name>
      <anchorfile>lb__tail_8c.html</anchorfile>
      <anchor>bb4b8509a628be7c774f57556c2e9e7e</anchor>
      <arglist></arglist>
    </member>
    <member kind="variable" static="yes">
      <type>static Set</type>
      <name>LB_hb_visiting</name>
      <anchorfile>lb__tail_8c.html</anchorfile>
      <anchor>5d84c5768cfa972d5a4abb3a1007500a</anchor>
      <arglist></arglist>
    </member>
    <member kind="variable" static="yes">
      <type>static Set</type>
      <name>LB_hb_visited</name>
      <anchorfile>lb__tail_8c.html</anchorfile>
      <anchor>7d7f48161a6bd1ca03b8c345b718db66</anchor>
      <arglist></arglist>
    </member>
    <member kind="variable" static="yes">
      <type>static HashTable</type>
      <name>LB_hb_dup_hash</name>
      <anchorfile>lb__tail_8c.html</anchorfile>
      <anchor>1ebbfa7729ac03d65320c6c0696cebd1</anchor>
      <arglist></arglist>
    </member>
  </compound>
  <compound kind="file">
    <name>lb_tail.h</name>
    <path>/mnt/hgfs/mkt/Desktop/openimpact/src/Lcode/opti/Lblock/</path>
    <filename>lb__tail_8h</filename>
    <member kind="define">
      <type>#define</type>
      <name>LB_DUP_INSIDE_REGION</name>
      <anchorfile>lb__tail_8h.html</anchorfile>
      <anchor>ddfe2f925842248a641591df4f7d4853</anchor>
      <arglist></arglist>
    </member>
    <member kind="define">
      <type>#define</type>
      <name>LB_DUP_OUTSIDE_REGION</name>
      <anchorfile>lb__tail_8h.html</anchorfile>
      <anchor>c384f02272b1ea77312fdf35c413a627</anchor>
      <arglist></arglist>
    </member>
    <member kind="function">
      <type>int</type>
      <name>LB_tail_duplication</name>
      <anchorfile>lb__tail_8h.html</anchorfile>
      <anchor>1f8a463aa25641dbfd112d855c00b189</anchor>
      <arglist>(L_Func *, LB_TraceRegion *, int, int)</arglist>
    </member>
    <member kind="function">
      <type>int</type>
      <name>LB_tail_duplicate</name>
      <anchorfile>lb__tail_8h.html</anchorfile>
      <anchor>6098a08c446a1e37a5d8e8ac236700d3</anchor>
      <arglist>(L_Func *, LB_TraceRegion_Header *, int)</arglist>
    </member>
    <member kind="function">
      <type>void</type>
      <name>LB_tail_duplication_codesize</name>
      <anchorfile>lb__tail_8h.html</anchorfile>
      <anchor>ed180796e7ae4e7b0723ab0207bb3a3c</anchor>
      <arglist>(L_Func *, LB_TraceRegion *, int)</arglist>
    </member>
  </compound>
  <compound kind="file">
    <name>lb_tool.c</name>
    <path>/mnt/hgfs/mkt/Desktop/openimpact/src/Lcode/opti/Lblock/</path>
    <filename>lb__tool_8c</filename>
    <includes id="lb__b__internal_8h" name="lb_b_internal.h" local="yes" imported="no">lb_b_internal.h</includes>
    <includes id="lb__tool_8h" name="lb_tool.h" local="yes" imported="no">lb_tool.h</includes>
    <member kind="function">
      <type>int</type>
      <name>LB_remove_empty_cbs</name>
      <anchorfile>lb__tool_8c.html</anchorfile>
      <anchor>752615183040bebde8c8c67908040c3b</anchor>
      <arglist>(L_Func *fn)</arglist>
    </member>
    <member kind="function">
      <type>int</type>
      <name>LB_hb_num_ops_in_cb_set</name>
      <anchorfile>lb__tool_8c.html</anchorfile>
      <anchor>34250d7ff26e34555008b806f32a8a0d</anchor>
      <arglist>(Set cb_set)</arglist>
    </member>
    <member kind="function">
      <type>int</type>
      <name>LB_hb_jsr_in_cb_set</name>
      <anchorfile>lb__tool_8c.html</anchorfile>
      <anchor>194731485f862396018d728706529f80</anchor>
      <arglist>(Set cb_set)</arglist>
    </member>
    <member kind="function">
      <type>void</type>
      <name>LB_hb_mark_all_cbs_with_attr</name>
      <anchorfile>lb__tool_8c.html</anchorfile>
      <anchor>e12cfbfccd71febae632f5b9bb247c21</anchor>
      <arglist>(Set cb_set, L_Attr *attr)</arglist>
    </member>
    <member kind="function">
      <type>int</type>
      <name>LB_fn_split_critical_edges</name>
      <anchorfile>lb__tool_8c.html</anchorfile>
      <anchor>37db9c7eae173d95a801a9b7145216df</anchor>
      <arglist>(L_Func *fn)</arglist>
    </member>
    <member kind="function">
      <type>int</type>
      <name>LB_convert_to_frp</name>
      <anchorfile>lb__tool_8c.html</anchorfile>
      <anchor>4dd2250fd89b77bdefb9fb2a4bc26e9d</anchor>
      <arglist>(L_Cb *cb, L_Oper *fr_op, L_Oper *to_op)</arglist>
    </member>
  </compound>
  <compound kind="file">
    <name>lb_tool.h</name>
    <path>/mnt/hgfs/mkt/Desktop/openimpact/src/Lcode/opti/Lblock/</path>
    <filename>lb__tool_8h</filename>
    <member kind="function">
      <type>void</type>
      <name>LB_cb_fix_branches_using_flows</name>
      <anchorfile>lb__tool_8h.html</anchorfile>
      <anchor>4e7871e74b0fbf00a6f3177d64d3f727</anchor>
      <arglist>(L_Cb *cb)</arglist>
    </member>
    <member kind="function">
      <type>int</type>
      <name>LB_remove_empty_cbs</name>
      <anchorfile>lb__tool_8h.html</anchorfile>
      <anchor>752615183040bebde8c8c67908040c3b</anchor>
      <arglist>(L_Func *fn)</arglist>
    </member>
    <member kind="function">
      <type>int</type>
      <name>LB_hb_num_ops_in_cb_set</name>
      <anchorfile>lb__tool_8h.html</anchorfile>
      <anchor>34250d7ff26e34555008b806f32a8a0d</anchor>
      <arglist>(Set cb_set)</arglist>
    </member>
    <member kind="function">
      <type>int</type>
      <name>LB_hb_jsr_in_cb_set</name>
      <anchorfile>lb__tool_8h.html</anchorfile>
      <anchor>194731485f862396018d728706529f80</anchor>
      <arglist>(Set cb_set)</arglist>
    </member>
    <member kind="function">
      <type>void</type>
      <name>LB_hb_mark_all_cbs_with_attr</name>
      <anchorfile>lb__tool_8h.html</anchorfile>
      <anchor>e12cfbfccd71febae632f5b9bb247c21</anchor>
      <arglist>(Set cb_set, L_Attr *attr)</arglist>
    </member>
    <member kind="function">
      <type>int</type>
      <name>LB_fn_split_critical_edges</name>
      <anchorfile>lb__tool_8h.html</anchorfile>
      <anchor>37db9c7eae173d95a801a9b7145216df</anchor>
      <arglist>(L_Func *fn)</arglist>
    </member>
    <member kind="function">
      <type>int</type>
      <name>LB_convert_to_frp</name>
      <anchorfile>lb__tool_8h.html</anchorfile>
      <anchor>4dd2250fd89b77bdefb9fb2a4bc26e9d</anchor>
      <arglist>(L_Cb *cb, L_Oper *fr_op, L_Oper *to_op)</arglist>
    </member>
  </compound>
  <compound kind="file">
    <name>lb_traceregion.c</name>
    <path>/mnt/hgfs/mkt/Desktop/openimpact/src/Lcode/opti/Lblock/</path>
    <filename>lb__traceregion_8c</filename>
    <includes id="lb__b__internal_8h" name="lb_b_internal.h" local="yes" imported="no">lb_b_internal.h</includes>
    <member kind="define">
      <type>#define</type>
      <name>ERR</name>
      <anchorfile>lb__traceregion_8c.html</anchorfile>
      <anchor>735563036dced0b7d6cc98f97ea4978b</anchor>
      <arglist></arglist>
    </member>
    <member kind="function">
      <type>LB_TraceRegion_Header *</type>
      <name>LB_create_tr_header</name>
      <anchorfile>lb__traceregion_8c.html</anchorfile>
      <anchor>e5d3995ef5f35469523a7cd3c5e1821f</anchor>
      <arglist>(L_Func *fn)</arglist>
    </member>
    <member kind="function">
      <type>void</type>
      <name>LB_free_tr_header</name>
      <anchorfile>lb__traceregion_8c.html</anchorfile>
      <anchor>f16df1bcecb4ffcce99325e5a80d7935</anchor>
      <arglist>(LB_TraceRegion_Header *header)</arglist>
    </member>
    <member kind="function">
      <type>void</type>
      <name>LB_update_traceregion</name>
      <anchorfile>lb__traceregion_8c.html</anchorfile>
      <anchor>d3d09bbeb54dc44d8e222055125cd9ed</anchor>
      <arglist>(LB_TraceRegion *tr, L_Func *fn, L_Cb *hdr, Set new_tr_cbs)</arglist>
    </member>
    <member kind="function">
      <type>LB_TraceRegion *</type>
      <name>LB_create_traceregion</name>
      <anchorfile>lb__traceregion_8c.html</anchorfile>
      <anchor>0612ca3a5f245ab8a46650ede2f5dd86</anchor>
      <arglist>(L_Func *fn, int id, L_Cb *initial_cb, Set blocks, int type)</arglist>
    </member>
    <member kind="function">
      <type>void</type>
      <name>LB_free_traceregion</name>
      <anchorfile>lb__traceregion_8c.html</anchorfile>
      <anchor>ddbea2a47ffd99092deaf8d73fca12fc</anchor>
      <arglist>(LB_TraceRegion *tr)</arglist>
    </member>
    <member kind="function">
      <type>void</type>
      <name>LB_free_all_traceregions</name>
      <anchorfile>lb__traceregion_8c.html</anchorfile>
      <anchor>22c0623958f329e2d25b408883589f9b</anchor>
      <arglist>(LB_TraceRegion_Header *header)</arglist>
    </member>
    <member kind="function">
      <type>void</type>
      <name>LB_add_cb_to_traceregion</name>
      <anchorfile>lb__traceregion_8c.html</anchorfile>
      <anchor>e6d137825ee1e97a0a76979adad7fe74</anchor>
      <arglist>(LB_TraceRegion *tr, L_Cb *current, L_Cb *cb, int end)</arglist>
    </member>
    <member kind="function">
      <type>L_Cb *</type>
      <name>LB_first_cb_in_region</name>
      <anchorfile>lb__traceregion_8c.html</anchorfile>
      <anchor>5c21f0d024869a028cdbb189be827079</anchor>
      <arglist>(LB_TraceRegion *tr)</arglist>
    </member>
    <member kind="function">
      <type>L_Cb *</type>
      <name>LB_next_cb_in_region</name>
      <anchorfile>lb__traceregion_8c.html</anchorfile>
      <anchor>875a3477279dbe494afaf52ddee84090</anchor>
      <arglist>(LB_TraceRegion *tr)</arglist>
    </member>
    <member kind="function">
      <type>GraphNode</type>
      <name>LB_next_graphnode</name>
      <anchorfile>lb__traceregion_8c.html</anchorfile>
      <anchor>da02f370a784ffee87f52663ada59543</anchor>
      <arglist>(LB_TraceRegion *tr)</arglist>
    </member>
    <member kind="function">
      <type>L_Cb *</type>
      <name>LB_get_first_cb_in_region</name>
      <anchorfile>lb__traceregion_8c.html</anchorfile>
      <anchor>f89f079a035a2b194564858fb1483d83</anchor>
      <arglist>(LB_TraceRegion *tr)</arglist>
    </member>
    <member kind="function">
      <type>L_Cb *</type>
      <name>LB_get_next_cb_in_region</name>
      <anchorfile>lb__traceregion_8c.html</anchorfile>
      <anchor>03b18a4ab07d49fe8f59a9fc6ec7aa03</anchor>
      <arglist>(LB_TraceRegion *tr)</arglist>
    </member>
    <member kind="function">
      <type>L_Cb *</type>
      <name>LB_last_cb_in_region</name>
      <anchorfile>lb__traceregion_8c.html</anchorfile>
      <anchor>692b67b475cb2eacf0f62f48b1bae3e2</anchor>
      <arglist>(LB_TraceRegion *tr)</arglist>
    </member>
    <member kind="function">
      <type>L_Cb *</type>
      <name>LB_return_cb_in_region</name>
      <anchorfile>lb__traceregion_8c.html</anchorfile>
      <anchor>a901ff36f877df957753943a2c019d4e</anchor>
      <arglist>(LB_TraceRegion *tr, int cb_id)</arglist>
    </member>
    <member kind="function">
      <type>Set</type>
      <name>LB_return_cbs_region_as_set</name>
      <anchorfile>lb__traceregion_8c.html</anchorfile>
      <anchor>ea0d7e4fb54c430488b6ca11a8436303</anchor>
      <arglist>(LB_TraceRegion *tr)</arglist>
    </member>
    <member kind="function">
      <type>LB_TraceRegion *</type>
      <name>LB_concat_seq_trs</name>
      <anchorfile>lb__traceregion_8c.html</anchorfile>
      <anchor>d5e869e46e402bfb1752ef6d23237a27</anchor>
      <arglist>(LB_TraceRegion_Header *hdr, LB_TraceRegion *tr1, LB_TraceRegion *tr2)</arglist>
    </member>
    <member kind="function">
      <type>void</type>
      <name>LB_traceregion_set_fallthru_flag</name>
      <anchorfile>lb__traceregion_8c.html</anchorfile>
      <anchor>4c14e86e21d87d57880e783a64294ac7</anchor>
      <arglist>(LB_TraceRegion *tr)</arglist>
    </member>
    <member kind="function">
      <type>void</type>
      <name>LB_clear_traceregion_visit_flags</name>
      <anchorfile>lb__traceregion_8c.html</anchorfile>
      <anchor>7ce29973ea8b6c467da7b97636daedbe</anchor>
      <arglist>(LB_TraceRegion_Header *header)</arglist>
    </member>
    <member kind="function">
      <type>int</type>
      <name>LB_unvisited_traceregions</name>
      <anchorfile>lb__traceregion_8c.html</anchorfile>
      <anchor>a6c082550db21ff486e011a00c634ca2</anchor>
      <arglist>(LB_TraceRegion_Header *header)</arglist>
    </member>
    <member kind="function">
      <type>LB_TraceRegion *</type>
      <name>LB_find_traceregion_by_header</name>
      <anchorfile>lb__traceregion_8c.html</anchorfile>
      <anchor>91de03b040f7017cf35169f1877558ac</anchor>
      <arglist>(LB_TraceRegion_Header *header, L_Cb *cb)</arglist>
    </member>
    <member kind="function">
      <type>LB_TraceRegion *</type>
      <name>LB_find_traceregion_by_cb</name>
      <anchorfile>lb__traceregion_8c.html</anchorfile>
      <anchor>60ce4e0d77828df9f862419b0901147c</anchor>
      <arglist>(LB_TraceRegion_Header *header, L_Cb *cb)</arglist>
    </member>
    <member kind="function">
      <type>LB_TraceRegion *</type>
      <name>LB_find_traceregion_of_number</name>
      <anchorfile>lb__traceregion_8c.html</anchorfile>
      <anchor>879e45ec42860a4f1c5a0b2b3faf478e</anchor>
      <arglist>(LB_TraceRegion_Header *header, int tr_num)</arglist>
    </member>
    <member kind="function" static="yes">
      <type>static int</type>
      <name>LB_tr_is_subset</name>
      <anchorfile>lb__traceregion_8c.html</anchorfile>
      <anchor>18118fbfc9dcc5bba3298dc937f30a20</anchor>
      <arglist>(LB_TraceRegion *tr1, LB_TraceRegion *tr2)</arglist>
    </member>
    <member kind="function" static="yes">
      <type>static int</type>
      <name>LB_traceregions_intersection_empty</name>
      <anchorfile>lb__traceregion_8c.html</anchorfile>
      <anchor>e916e818afa0fdc07c03247a664a47de</anchor>
      <arglist>(LB_TraceRegion *tr1, LB_TraceRegion *tr2)</arglist>
    </member>
    <member kind="function">
      <type>int</type>
      <name>LB_traceregion_is_subsumed</name>
      <anchorfile>lb__traceregion_8c.html</anchorfile>
      <anchor>a0b043eb74ff0c71e39224dd9a6ce121</anchor>
      <arglist>(LB_TraceRegion *tr, LB_TraceRegion_Header *header)</arglist>
    </member>
    <member kind="function" static="yes">
      <type>static LB_TraceRegion *</type>
      <name>LB_traceregion_is_partially_subsumed</name>
      <anchorfile>lb__traceregion_8c.html</anchorfile>
      <anchor>6d48d6227a3825eed4dba41aa5616a85</anchor>
      <arglist>(LB_TraceRegion *tr, LB_TraceRegion_Header *header)</arglist>
    </member>
    <member kind="function">
      <type>void</type>
      <name>LB_remove_subsumed_traceregions</name>
      <anchorfile>lb__traceregion_8c.html</anchorfile>
      <anchor>c9f3ee7d27ab10eca258f82395895f98</anchor>
      <arglist>(LB_TraceRegion_Header *header)</arglist>
    </member>
    <member kind="function">
      <type>void</type>
      <name>LB_remove_partially_subsumed_hammock_traceregions</name>
      <anchorfile>lb__traceregion_8c.html</anchorfile>
      <anchor>f4204b518a7ad6729cf2128e91f2ed5f</anchor>
      <arglist>(LB_TraceRegion_Header *header)</arglist>
    </member>
    <member kind="function">
      <type>void</type>
      <name>LB_remove_partially_subsumed_traceregions</name>
      <anchorfile>lb__traceregion_8c.html</anchorfile>
      <anchor>66a6334c7ee761de669675b7e654be33</anchor>
      <arglist>(LB_TraceRegion_Header *header)</arglist>
    </member>
    <member kind="function">
      <type>void</type>
      <name>LB_remove_conflicting_traceregions</name>
      <anchorfile>lb__traceregion_8c.html</anchorfile>
      <anchor>d79a6384476c7d73efb45432f8b2fee3</anchor>
      <arglist>(LB_TraceRegion_Header *header)</arglist>
    </member>
    <member kind="function">
      <type>void</type>
      <name>LB_print_traceregions</name>
      <anchorfile>lb__traceregion_8c.html</anchorfile>
      <anchor>700c9707fd739c92494b92042f824d2b</anchor>
      <arglist>(FILE *fp, LB_TraceRegion_Header *header)</arglist>
    </member>
    <member kind="function">
      <type>void</type>
      <name>LB_summarize_traceregions</name>
      <anchorfile>lb__traceregion_8c.html</anchorfile>
      <anchor>c80be77178d75ca5714e721b88d0a971</anchor>
      <arglist>(FILE *fp, LB_TraceRegion_Header *header)</arglist>
    </member>
    <member kind="function">
      <type>void</type>
      <name>LB_print_inorder_trs</name>
      <anchorfile>lb__traceregion_8c.html</anchorfile>
      <anchor>362f9ac2492e83fd45bbac2a4af1499b</anchor>
      <arglist>(FILE *fp, LB_TraceRegion_Header *header)</arglist>
    </member>
    <member kind="function">
      <type>void</type>
      <name>LB_print_tr_by_num</name>
      <anchorfile>lb__traceregion_8c.html</anchorfile>
      <anchor>2d110f468130656fef8520bdf0cb0b0b</anchor>
      <arglist>(FILE *fp, LB_TraceRegion_Header *header, int num)</arglist>
    </member>
    <member kind="function">
      <type>void</type>
      <name>LB_print_tr</name>
      <anchorfile>lb__traceregion_8c.html</anchorfile>
      <anchor>81d4ac7be1e903c690e17ef2de06ecc0</anchor>
      <arglist>(FILE *fp, LB_TraceRegion *tr)</arglist>
    </member>
    <member kind="function">
      <type>void</type>
      <name>LB_summarize_tr</name>
      <anchorfile>lb__traceregion_8c.html</anchorfile>
      <anchor>ac57e6a70f9d6d38c9826eadcec4db82</anchor>
      <arglist>(FILE *fp, LB_TraceRegion *tr)</arglist>
    </member>
    <member kind="variable" static="yes">
      <type>static L_Alloc_Pool *</type>
      <name>LB_TraceRegion_Pool</name>
      <anchorfile>lb__traceregion_8c.html</anchorfile>
      <anchor>7c09c45b6a5d443e7c3fb14e37789fca</anchor>
      <arglist></arglist>
    </member>
    <member kind="variable" static="yes">
      <type>static L_Alloc_Pool *</type>
      <name>LB_TraceRegion_Header_Pool</name>
      <anchorfile>lb__traceregion_8c.html</anchorfile>
      <anchor>de037e769094874f759dc2b1fcf330a7</anchor>
      <arglist></arglist>
    </member>
    <member kind="variable" static="yes">
      <type>static char</type>
      <name>LB_traceregion_flag_name</name>
      <anchorfile>lb__traceregion_8c.html</anchorfile>
      <anchor>1939920a0fad319fe040f36ad40177ff</anchor>
      <arglist>[32][8]</arglist>
    </member>
  </compound>
  <compound kind="file">
    <name>lb_traceregion.h</name>
    <path>/mnt/hgfs/mkt/Desktop/openimpact/src/Lcode/opti/Lblock/</path>
    <filename>lb__traceregion_8h</filename>
    <class kind="struct">_LB_TraceRegion</class>
    <class kind="struct">_LB_TraceRegion_Header</class>
    <member kind="define">
      <type>#define</type>
      <name>END</name>
      <anchorfile>lb__traceregion_8h.html</anchorfile>
      <anchor>29fd18bed01c4d836c7ebfe73a125c3f</anchor>
      <arglist></arglist>
    </member>
    <member kind="define">
      <type>#define</type>
      <name>TOP</name>
      <anchorfile>lb__traceregion_8h.html</anchorfile>
      <anchor>fc0eef637f1016e8786e45e106a4881e</anchor>
      <arglist></arglist>
    </member>
    <member kind="define">
      <type>#define</type>
      <name>L_TRACEREGION_VISITED</name>
      <anchorfile>lb__traceregion_8h.html</anchorfile>
      <anchor>c288f1da1b6da5d26217e128b86f6c29</anchor>
      <arglist></arglist>
    </member>
    <member kind="define">
      <type>#define</type>
      <name>L_TRACEREGION_HYPERBLOCK</name>
      <anchorfile>lb__traceregion_8h.html</anchorfile>
      <anchor>4c5eabf806f00278dc0bb1a16a4d0842</anchor>
      <arglist></arglist>
    </member>
    <member kind="define">
      <type>#define</type>
      <name>L_TRACEREGION_SUPERBLOCK</name>
      <anchorfile>lb__traceregion_8h.html</anchorfile>
      <anchor>46686382c14f9a7df20626aad6c7b14e</anchor>
      <arglist></arglist>
    </member>
    <member kind="define">
      <type>#define</type>
      <name>L_TRACEREGION_INSERTED</name>
      <anchorfile>lb__traceregion_8h.html</anchorfile>
      <anchor>b9df9f8bb4ad9735b7af70fa82eb620e</anchor>
      <arglist></arglist>
    </member>
    <member kind="define">
      <type>#define</type>
      <name>L_TRACEREGION_LOOP</name>
      <anchorfile>lb__traceregion_8h.html</anchorfile>
      <anchor>672b78e55ac5aa4872f363f5e7c4f097</anchor>
      <arglist></arglist>
    </member>
    <member kind="define">
      <type>#define</type>
      <name>L_TRACEREGION_HAMMOCK</name>
      <anchorfile>lb__traceregion_8h.html</anchorfile>
      <anchor>bceef4c51bd8b6facbdb3bdeb5ac4a82</anchor>
      <arglist></arglist>
    </member>
    <member kind="define">
      <type>#define</type>
      <name>L_TRACEREGION_GENERAL</name>
      <anchorfile>lb__traceregion_8h.html</anchorfile>
      <anchor>589413e971a5c7c4ccd8831ef8270d19</anchor>
      <arglist></arglist>
    </member>
    <member kind="define">
      <type>#define</type>
      <name>L_TRACEREGION_TRACE</name>
      <anchorfile>lb__traceregion_8h.html</anchorfile>
      <anchor>731593b83145dbab15bb33dfa7bf0960</anchor>
      <arglist></arglist>
    </member>
    <member kind="define">
      <type>#define</type>
      <name>L_TRACEREGION_TYPE</name>
      <anchorfile>lb__traceregion_8h.html</anchorfile>
      <anchor>4b59ad1408792827a20d9933887a7223</anchor>
      <arglist></arglist>
    </member>
    <member kind="define">
      <type>#define</type>
      <name>L_TRACEREGION_NOFALLTHRU</name>
      <anchorfile>lb__traceregion_8h.html</anchorfile>
      <anchor>ce170a84bd3c3d72ee86f36473d9b0c2</anchor>
      <arglist></arglist>
    </member>
    <member kind="define">
      <type>#define</type>
      <name>L_TRACEREGION_FALLTHRU</name>
      <anchorfile>lb__traceregion_8h.html</anchorfile>
      <anchor>2583b37064b39213e0362ae57cac44c2</anchor>
      <arglist></arglist>
    </member>
    <member kind="define">
      <type>#define</type>
      <name>L_TRACEREGION_PRELIM</name>
      <anchorfile>lb__traceregion_8h.html</anchorfile>
      <anchor>e60d9467958b6c9d7578a82de32d2052</anchor>
      <arglist></arglist>
    </member>
    <member kind="define">
      <type>#define</type>
      <name>L_TRACEREGION_FLAG_NESTED_CYCLE</name>
      <anchorfile>lb__traceregion_8h.html</anchorfile>
      <anchor>232b001240d29d56602630ab35454605</anchor>
      <arglist></arglist>
    </member>
    <member kind="define">
      <type>#define</type>
      <name>L_TRACEREGION_FLAG_HAS_UNSAFE_JSR</name>
      <anchorfile>lb__traceregion_8h.html</anchorfile>
      <anchor>95a23aad4018c33bf2b2cbb31da0f641</anchor>
      <arglist></arglist>
    </member>
    <member kind="define">
      <type>#define</type>
      <name>L_TRACEREGION_FLAG_HAS_JSR</name>
      <anchorfile>lb__traceregion_8h.html</anchorfile>
      <anchor>51ad88ab415f03ebd0cf5957344e88ae</anchor>
      <arglist></arglist>
    </member>
    <member kind="define">
      <type>#define</type>
      <name>L_TRACEREGION_FLAG_HAS_POINTER_ST</name>
      <anchorfile>lb__traceregion_8h.html</anchorfile>
      <anchor>4b191aa3afb8b6177d95f02e6b466db9</anchor>
      <arglist></arglist>
    </member>
    <member kind="define">
      <type>#define</type>
      <name>FlowGraph</name>
      <anchorfile>lb__traceregion_8h.html</anchorfile>
      <anchor>8acae6952ab90503a5cea64ecf371cd1</anchor>
      <arglist>(tr)</arglist>
    </member>
    <member kind="define">
      <type>#define</type>
      <name>FlowGraphTopoList</name>
      <anchorfile>lb__traceregion_8h.html</anchorfile>
      <anchor>7daa9d5131541f74935ad72563061709</anchor>
      <arglist>(tr)</arglist>
    </member>
    <member kind="typedef">
      <type>_LB_TraceRegion</type>
      <name>LB_TraceRegion</name>
      <anchorfile>lb__traceregion_8h.html</anchorfile>
      <anchor>f49e9a4052c3afeba5a4c08167469bfc</anchor>
      <arglist></arglist>
    </member>
    <member kind="typedef">
      <type>_LB_TraceRegion_Header</type>
      <name>LB_TraceRegion_Header</name>
      <anchorfile>lb__traceregion_8h.html</anchorfile>
      <anchor>e031f3cec8ad9ea3e51f681955d9ec02</anchor>
      <arglist></arglist>
    </member>
    <member kind="function">
      <type>LB_TraceRegion_Header *</type>
      <name>LB_create_tr_header</name>
      <anchorfile>lb__traceregion_8h.html</anchorfile>
      <anchor>b90e080f9fe1a28f7eaaa2941271b741</anchor>
      <arglist>(L_Func *)</arglist>
    </member>
    <member kind="function">
      <type>void</type>
      <name>LB_free_tr_header</name>
      <anchorfile>lb__traceregion_8h.html</anchorfile>
      <anchor>85cc169ebb141db3ac2e54ff69f292e9</anchor>
      <arglist>(LB_TraceRegion_Header *)</arglist>
    </member>
    <member kind="function">
      <type>LB_TraceRegion *</type>
      <name>LB_create_traceregion</name>
      <anchorfile>lb__traceregion_8h.html</anchorfile>
      <anchor>1cd5bc0c6f08cc6921acc22f530f56db</anchor>
      <arglist>(L_Func *, int, L_Cb *, Set, int)</arglist>
    </member>
    <member kind="function">
      <type>void</type>
      <name>LB_update_traceregion</name>
      <anchorfile>lb__traceregion_8h.html</anchorfile>
      <anchor>d3d09bbeb54dc44d8e222055125cd9ed</anchor>
      <arglist>(LB_TraceRegion *tr, L_Func *fn, L_Cb *hdr, Set new_tr_cbs)</arglist>
    </member>
    <member kind="function">
      <type>void</type>
      <name>LB_free_traceregion</name>
      <anchorfile>lb__traceregion_8h.html</anchorfile>
      <anchor>e7d235813b95e24c86bbb004aeaff8ad</anchor>
      <arglist>(LB_TraceRegion *)</arglist>
    </member>
    <member kind="function">
      <type>void</type>
      <name>LB_free_all_traceregions</name>
      <anchorfile>lb__traceregion_8h.html</anchorfile>
      <anchor>509d7ca60944d922cd60cb31282f67c7</anchor>
      <arglist>(LB_TraceRegion_Header *)</arglist>
    </member>
    <member kind="function">
      <type>void</type>
      <name>LB_add_cb_to_traceregion</name>
      <anchorfile>lb__traceregion_8h.html</anchorfile>
      <anchor>1bfbc79a98a006552348211c9dbd9bc6</anchor>
      <arglist>(LB_TraceRegion *, L_Cb *, L_Cb *, int)</arglist>
    </member>
    <member kind="function">
      <type>L_Cb *</type>
      <name>LB_first_cb_in_region</name>
      <anchorfile>lb__traceregion_8h.html</anchorfile>
      <anchor>2638f63081048c51cf698ed56125e67d</anchor>
      <arglist>(LB_TraceRegion *)</arglist>
    </member>
    <member kind="function">
      <type>L_Cb *</type>
      <name>LB_next_cb_in_region</name>
      <anchorfile>lb__traceregion_8h.html</anchorfile>
      <anchor>f3797c2d039286c828cac9eb04122a5b</anchor>
      <arglist>(LB_TraceRegion *)</arglist>
    </member>
    <member kind="function">
      <type>L_Cb *</type>
      <name>LB_get_first_cb_in_region</name>
      <anchorfile>lb__traceregion_8h.html</anchorfile>
      <anchor>c83f49e325ab9dba02a57361bbcb8586</anchor>
      <arglist>(LB_TraceRegion *)</arglist>
    </member>
    <member kind="function">
      <type>L_Cb *</type>
      <name>LB_get_next_cb_in_region</name>
      <anchorfile>lb__traceregion_8h.html</anchorfile>
      <anchor>fab547b3d57b6610e1d7092b5f4f49b2</anchor>
      <arglist>(LB_TraceRegion *)</arglist>
    </member>
    <member kind="function">
      <type>GraphNode</type>
      <name>LB_next_graphnode</name>
      <anchorfile>lb__traceregion_8h.html</anchorfile>
      <anchor>bb3885e30b120527e2366adfb586509c</anchor>
      <arglist>(LB_TraceRegion *)</arglist>
    </member>
    <member kind="function">
      <type>L_Cb *</type>
      <name>LB_last_cb_in_region</name>
      <anchorfile>lb__traceregion_8h.html</anchorfile>
      <anchor>18784db76b775e1ccaf7b2de786a3812</anchor>
      <arglist>(LB_TraceRegion *)</arglist>
    </member>
    <member kind="function">
      <type>L_Cb *</type>
      <name>LB_return_cb_in_region</name>
      <anchorfile>lb__traceregion_8h.html</anchorfile>
      <anchor>f09c18ffb94c2f6ea1b9eeda7c00fe43</anchor>
      <arglist>(LB_TraceRegion *, int)</arglist>
    </member>
    <member kind="function">
      <type>Set</type>
      <name>LB_return_cbs_region_as_set</name>
      <anchorfile>lb__traceregion_8h.html</anchorfile>
      <anchor>e1d1d05ed070c16a012ab2ebb157c5e5</anchor>
      <arglist>(LB_TraceRegion *)</arglist>
    </member>
    <member kind="function">
      <type>void</type>
      <name>LB_traceregion_set_fallthru_flag</name>
      <anchorfile>lb__traceregion_8h.html</anchorfile>
      <anchor>4ec9b55f291304d4989737a4cb0ca865</anchor>
      <arglist>(LB_TraceRegion *)</arglist>
    </member>
    <member kind="function">
      <type>LB_TraceRegion *</type>
      <name>LB_concat_seq_trs</name>
      <anchorfile>lb__traceregion_8h.html</anchorfile>
      <anchor>a0ffb8227491a26405978c8522fb73bf</anchor>
      <arglist>(LB_TraceRegion_Header *, LB_TraceRegion *, LB_TraceRegion *)</arglist>
    </member>
    <member kind="function">
      <type>void</type>
      <name>LB_clear_traceregion_visit_flags</name>
      <anchorfile>lb__traceregion_8h.html</anchorfile>
      <anchor>412ca56b3f18865cd48d5e47208ebf2c</anchor>
      <arglist>(LB_TraceRegion_Header *)</arglist>
    </member>
    <member kind="function">
      <type>int</type>
      <name>LB_unvisited_traceregions</name>
      <anchorfile>lb__traceregion_8h.html</anchorfile>
      <anchor>f4dd2bc8752af76b07d019fb23ac90ae</anchor>
      <arglist>(LB_TraceRegion_Header *)</arglist>
    </member>
    <member kind="function">
      <type>int</type>
      <name>LB_traceregion_is_subsumed</name>
      <anchorfile>lb__traceregion_8h.html</anchorfile>
      <anchor>b63b0a1c0a919df24dfd5a6b68f915d5</anchor>
      <arglist>(LB_TraceRegion *, LB_TraceRegion_Header *)</arglist>
    </member>
    <member kind="function">
      <type>void</type>
      <name>LB_remove_subsumed_traceregions</name>
      <anchorfile>lb__traceregion_8h.html</anchorfile>
      <anchor>b4ff17e65a1a401f78a0908bd4d5f7f4</anchor>
      <arglist>(LB_TraceRegion_Header *)</arglist>
    </member>
    <member kind="function">
      <type>void</type>
      <name>LB_remove_partially_subsumed_hammock_traceregions</name>
      <anchorfile>lb__traceregion_8h.html</anchorfile>
      <anchor>a88a8734bb731649ab35bfbc0a088c55</anchor>
      <arglist>(LB_TraceRegion_Header *)</arglist>
    </member>
    <member kind="function">
      <type>void</type>
      <name>LB_remove_partially_subsumed_traceregions</name>
      <anchorfile>lb__traceregion_8h.html</anchorfile>
      <anchor>ff8453d7868525aedc42d49c586f0d1b</anchor>
      <arglist>(LB_TraceRegion_Header *)</arglist>
    </member>
    <member kind="function">
      <type>void</type>
      <name>LB_remove_conflicting_traceregions</name>
      <anchorfile>lb__traceregion_8h.html</anchorfile>
      <anchor>484711e04cf30c81a013352f19ec3b20</anchor>
      <arglist>(LB_TraceRegion_Header *)</arglist>
    </member>
    <member kind="function">
      <type>LB_TraceRegion *</type>
      <name>LB_find_traceregion_by_header</name>
      <anchorfile>lb__traceregion_8h.html</anchorfile>
      <anchor>b5ce50ea25d13fbd260ec53042a71e92</anchor>
      <arglist>(LB_TraceRegion_Header *, L_Cb *)</arglist>
    </member>
    <member kind="function">
      <type>LB_TraceRegion *</type>
      <name>LB_find_traceregion_by_cb</name>
      <anchorfile>lb__traceregion_8h.html</anchorfile>
      <anchor>8afe160c62814f5888287f278b07de76</anchor>
      <arglist>(LB_TraceRegion_Header *, L_Cb *)</arglist>
    </member>
    <member kind="function">
      <type>LB_TraceRegion *</type>
      <name>LB_find_traceregion_of_number</name>
      <anchorfile>lb__traceregion_8h.html</anchorfile>
      <anchor>fd818604dfddb274001a0bb1bd70f6a6</anchor>
      <arglist>(LB_TraceRegion_Header *, int)</arglist>
    </member>
    <member kind="function">
      <type>void</type>
      <name>LB_print_traceregions</name>
      <anchorfile>lb__traceregion_8h.html</anchorfile>
      <anchor>1a6bffc26b51a85c74487b7ffed14a52</anchor>
      <arglist>(FILE *, LB_TraceRegion_Header *)</arglist>
    </member>
    <member kind="function">
      <type>void</type>
      <name>LB_print_inorder_trs</name>
      <anchorfile>lb__traceregion_8h.html</anchorfile>
      <anchor>98a520d97aa830e37dfc9eb51f9d24fa</anchor>
      <arglist>(FILE *, LB_TraceRegion_Header *)</arglist>
    </member>
    <member kind="function">
      <type>void</type>
      <name>LB_print_tr_by_num</name>
      <anchorfile>lb__traceregion_8h.html</anchorfile>
      <anchor>cec7e5df5ca5004c19550839741f0206</anchor>
      <arglist>(FILE *, LB_TraceRegion_Header *, int)</arglist>
    </member>
    <member kind="function">
      <type>void</type>
      <name>LB_print_tr</name>
      <anchorfile>lb__traceregion_8h.html</anchorfile>
      <anchor>6c5331f205a20bbd99ccff839f9ff0d4</anchor>
      <arglist>(FILE *, LB_TraceRegion *)</arglist>
    </member>
    <member kind="function">
      <type>void</type>
      <name>LB_summarize_traceregions</name>
      <anchorfile>lb__traceregion_8h.html</anchorfile>
      <anchor>3b00c6818c487bb7bbe9b4530488de7d</anchor>
      <arglist>(FILE *, LB_TraceRegion_Header *)</arglist>
    </member>
    <member kind="function">
      <type>void</type>
      <name>LB_summarize_tr</name>
      <anchorfile>lb__traceregion_8h.html</anchorfile>
      <anchor>b861332cc20f3ff3972948d2f99cd198</anchor>
      <arglist>(FILE *, LB_TraceRegion *)</arglist>
    </member>
  </compound>
  <compound kind="struct">
    <name>_LB_BB</name>
    <filename>struct__LB__BB.html</filename>
    <member kind="variable">
      <type>int</type>
      <name>type</name>
      <anchorfile>struct__LB__BB.html</anchorfile>
      <anchor>585357bbeac3710de0c0b96354bddb90</anchor>
      <arglist></arglist>
    </member>
    <member kind="variable">
      <type>int</type>
      <name>flag</name>
      <anchorfile>struct__LB__BB.html</anchorfile>
      <anchor>defd2f41e9907e131301c8627b79e844</anchor>
      <arglist></arglist>
    </member>
    <member kind="variable">
      <type>L_Cb *</type>
      <name>cb</name>
      <anchorfile>struct__LB__BB.html</anchorfile>
      <anchor>06bc9a25ce08c11181f4a380ee338c83</anchor>
      <arglist></arglist>
    </member>
    <member kind="variable">
      <type>GraphNode</type>
      <name>node</name>
      <anchorfile>struct__LB__BB.html</anchorfile>
      <anchor>053feda8b415ca0d9786e26daf371fd6</anchor>
      <arglist></arglist>
    </member>
  </compound>
  <compound kind="struct">
    <name>_LB_Cb_Info</name>
    <filename>struct__LB__Cb__Info.html</filename>
    <member kind="variable">
      <type>int</type>
      <name>flags</name>
      <anchorfile>struct__LB__Cb__Info.html</anchorfile>
      <anchor>7b68c5b56d2a6016a302ff24c81f2a79</anchor>
      <arglist></arglist>
    </member>
    <member kind="variable">
      <type>int</type>
      <name>dep_height</name>
      <anchorfile>struct__LB__Cb__Info.html</anchorfile>
      <anchor>22d8dcc90fb43874757402305f08ee66</anchor>
      <arglist></arglist>
    </member>
    <member kind="variable">
      <type>int</type>
      <name>slots_used</name>
      <anchorfile>struct__LB__Cb__Info.html</anchorfile>
      <anchor>1e1d5b10d70a2504f7c9ddcdd3eda690</anchor>
      <arglist></arglist>
    </member>
  </compound>
  <compound kind="struct">
    <name>_LB_Hammock</name>
    <filename>struct__LB__Hammock.html</filename>
    <member kind="variable">
      <type>int</type>
      <name>flags</name>
      <anchorfile>struct__LB__Hammock.html</anchorfile>
      <anchor>63ba5f2ed0efb686e933ae4909b32aa2</anchor>
      <arglist></arglist>
    </member>
    <member kind="variable">
      <type>L_Cb *</type>
      <name>start_cb</name>
      <anchorfile>struct__LB__Hammock.html</anchorfile>
      <anchor>6a7cfbafe426efafc5c9f5da081da772</anchor>
      <arglist></arglist>
    </member>
    <member kind="variable">
      <type>L_Cb *</type>
      <name>end_cb</name>
      <anchorfile>struct__LB__Hammock.html</anchorfile>
      <anchor>32b639dd467a7809291b23c9da4784d1</anchor>
      <arglist></arglist>
    </member>
    <member kind="variable">
      <type>Set</type>
      <name>blocks</name>
      <anchorfile>struct__LB__Hammock.html</anchorfile>
      <anchor>c7fc5898cc15364b88f5c6ee128e3f81</anchor>
      <arglist></arglist>
    </member>
    <member kind="variable">
      <type>LB_TraceRegion *</type>
      <name>tr</name>
      <anchorfile>struct__LB__Hammock.html</anchorfile>
      <anchor>26fde3b848102d1a98b5627465ee237f</anchor>
      <arglist></arglist>
    </member>
  </compound>
  <compound kind="struct">
    <name>_LB_hb_Path</name>
    <filename>struct__LB__hb__Path.html</filename>
    <member kind="variable">
      <type>double</type>
      <name>exec_ratio</name>
      <anchorfile>struct__LB__hb__Path.html</anchorfile>
      <anchor>964134c6c2d4b47efa5b429b6043c868</anchor>
      <arglist></arglist>
    </member>
    <member kind="variable">
      <type>double</type>
      <name>priority</name>
      <anchorfile>struct__LB__hb__Path.html</anchorfile>
      <anchor>23aa6a2cbeef6fc5d517097331773a65</anchor>
      <arglist></arglist>
    </member>
    <member kind="variable">
      <type>int</type>
      <name>id</name>
      <anchorfile>struct__LB__hb__Path.html</anchorfile>
      <anchor>2ac417ac6f43aa374d61f416937f5230</anchor>
      <arglist></arglist>
    </member>
    <member kind="variable">
      <type>int</type>
      <name>selected</name>
      <anchorfile>struct__LB__hb__Path.html</anchorfile>
      <anchor>21417f77dfbee391afd3dab08ec8c348</anchor>
      <arglist></arglist>
    </member>
    <member kind="variable">
      <type>int</type>
      <name>num_blocks</name>
      <anchorfile>struct__LB__hb__Path.html</anchorfile>
      <anchor>1096bb85d1416107bbdb7c08f828a9fb</anchor>
      <arglist></arglist>
    </member>
    <member kind="variable">
      <type>int *</type>
      <name>blocks</name>
      <anchorfile>struct__LB__hb__Path.html</anchorfile>
      <anchor>e4ee06e9c9c4cc515a1d104d3776918b</anchor>
      <arglist></arglist>
    </member>
    <member kind="variable">
      <type>Set</type>
      <name>block_set</name>
      <anchorfile>struct__LB__hb__Path.html</anchorfile>
      <anchor>cd8043308822146677bc353e842bffc2</anchor>
      <arglist></arglist>
    </member>
    <member kind="variable">
      <type>int</type>
      <name>num_ops</name>
      <anchorfile>struct__LB__hb__Path.html</anchorfile>
      <anchor>7430953d00c5479ea4d68c0a7970ae7d</anchor>
      <arglist></arglist>
    </member>
    <member kind="variable">
      <type>int</type>
      <name>flags</name>
      <anchorfile>struct__LB__hb__Path.html</anchorfile>
      <anchor>c68a1aaaa8cb7b562aaae3ca4cd447ff</anchor>
      <arglist></arglist>
    </member>
    <member kind="variable">
      <type>int</type>
      <name>dep_height</name>
      <anchorfile>struct__LB__hb__Path.html</anchorfile>
      <anchor>e4523d31c07304c8055ae0b035b5b6ca</anchor>
      <arglist></arglist>
    </member>
  </compound>
  <compound kind="struct">
    <name>_LB_hb_Sort_List</name>
    <filename>struct__LB__hb__Sort__List.html</filename>
    <member kind="variable">
      <type>double</type>
      <name>weight</name>
      <anchorfile>struct__LB__hb__Sort__List.html</anchorfile>
      <anchor>64b11dc8c04f35c5add11837e7282e09</anchor>
      <arglist></arglist>
    </member>
    <member kind="variable">
      <type>void *</type>
      <name>ptr</name>
      <anchorfile>struct__LB__hb__Sort__List.html</anchorfile>
      <anchor>b14de6ed4f443bb1d6d4ad96daf7a26b</anchor>
      <arglist></arglist>
    </member>
  </compound>
  <compound kind="struct">
    <name>_LB_HB_Stat</name>
    <filename>struct__LB__HB__Stat.html</filename>
    <member kind="variable">
      <type>int</type>
      <name>orig_ops</name>
      <anchorfile>struct__LB__HB__Stat.html</anchorfile>
      <anchor>2648b702debbf947c3c18211b1a1a5b8</anchor>
      <arglist></arglist>
    </member>
    <member kind="variable">
      <type>int</type>
      <name>tail_dup_ops</name>
      <anchorfile>struct__LB__HB__Stat.html</anchorfile>
      <anchor>dd24c11ca6671f67797a9deb0e6d47f9</anchor>
      <arglist></arglist>
    </member>
    <member kind="variable">
      <type>int</type>
      <name>peel_ops</name>
      <anchorfile>struct__LB__HB__Stat.html</anchorfile>
      <anchor>19e191db74fd89105e68645eec5f00ee</anchor>
      <arglist></arglist>
    </member>
    <member kind="variable">
      <type>int</type>
      <name>jump_ops</name>
      <anchorfile>struct__LB__HB__Stat.html</anchorfile>
      <anchor>14c381bf4a2dfab90ee6c6a017a649ee</anchor>
      <arglist></arglist>
    </member>
  </compound>
  <compound kind="struct">
    <name>_LB_Predicate</name>
    <filename>struct__LB__Predicate.html</filename>
    <member kind="variable">
      <type>int</type>
      <name>id</name>
      <anchorfile>struct__LB__Predicate.html</anchorfile>
      <anchor>d7db402a01deae35dd84fe27527a7fb6</anchor>
      <arglist></arglist>
    </member>
  </compound>
  <compound kind="struct">
    <name>_LB_TraceRegion</name>
    <filename>struct__LB__TraceRegion.html</filename>
    <member kind="variable">
      <type>int</type>
      <name>id</name>
      <anchorfile>struct__LB__TraceRegion.html</anchorfile>
      <anchor>7f4acec07be0ac994a461b21f9ae5f8e</anchor>
      <arglist></arglist>
    </member>
    <member kind="variable">
      <type>int</type>
      <name>flags</name>
      <anchorfile>struct__LB__TraceRegion.html</anchorfile>
      <anchor>5a8f7e4bb9785729e33f9a1fd603b3c0</anchor>
      <arglist></arglist>
    </member>
    <member kind="variable">
      <type>Graph</type>
      <name>flow_graph</name>
      <anchorfile>struct__LB__TraceRegion.html</anchorfile>
      <anchor>83a60bdd07e492fb2f5170f1579461c6</anchor>
      <arglist></arglist>
    </member>
    <member kind="variable">
      <type>L_Cb *</type>
      <name>header</name>
      <anchorfile>struct__LB__TraceRegion.html</anchorfile>
      <anchor>3d0df6cbbd6041d441c500504dd115ef</anchor>
      <arglist></arglist>
    </member>
    <member kind="variable">
      <type>double</type>
      <name>weight</name>
      <anchorfile>struct__LB__TraceRegion.html</anchorfile>
      <anchor>e0a37dc9120ff09fd0aca98bbedc6e77</anchor>
      <arglist></arglist>
    </member>
    <member kind="variable">
      <type>double</type>
      <name>weight_temp</name>
      <anchorfile>struct__LB__TraceRegion.html</anchorfile>
      <anchor>c590584d2779b745d07acf908993d39c</anchor>
      <arglist></arglist>
    </member>
    <member kind="variable">
      <type>int</type>
      <name>dep_height</name>
      <anchorfile>struct__LB__TraceRegion.html</anchorfile>
      <anchor>c5987ffcddb586a0f2c6cba421a59fb1</anchor>
      <arglist></arglist>
    </member>
    <member kind="variable">
      <type>int</type>
      <name>num_ops</name>
      <anchorfile>struct__LB__TraceRegion.html</anchorfile>
      <anchor>57251fac3f083fae47c71fa0f04921c4</anchor>
      <arglist></arglist>
    </member>
    <member kind="variable">
      <type>int</type>
      <name>tail_dup</name>
      <anchorfile>struct__LB__TraceRegion.html</anchorfile>
      <anchor>b9a56aaeb7f1b5b75908ed5f37ebdaa4</anchor>
      <arglist></arglist>
    </member>
    <member kind="variable">
      <type>int</type>
      <name>slots_used</name>
      <anchorfile>struct__LB__TraceRegion.html</anchorfile>
      <anchor>69a19456f27cbe4b5a507e242c211b67</anchor>
      <arglist></arglist>
    </member>
    <member kind="variable">
      <type>int</type>
      <name>slots_avail</name>
      <anchorfile>struct__LB__TraceRegion.html</anchorfile>
      <anchor>ad4ff7c9a398513413572d9fb61d9b9e</anchor>
      <arglist></arglist>
    </member>
    <member kind="variable">
      <type>double</type>
      <name>exec_ratio</name>
      <anchorfile>struct__LB__TraceRegion.html</anchorfile>
      <anchor>bad98c81db172fa6c6824cdedf403e0f</anchor>
      <arglist></arglist>
    </member>
    <member kind="variable">
      <type>double</type>
      <name>priority</name>
      <anchorfile>struct__LB__TraceRegion.html</anchorfile>
      <anchor>35135d8b5e3c304fb7e1532ee29c200e</anchor>
      <arglist></arglist>
    </member>
    <member kind="variable">
      <type>int</type>
      <name>wcet</name>
      <anchorfile>struct__LB__TraceRegion.html</anchorfile>
      <anchor>9199c7f1f9234867014bd708f7ba1d4a</anchor>
      <arglist></arglist>
    </member>
    <member kind="variable">
      <type>int</type>
      <name>wcet2</name>
      <anchorfile>struct__LB__TraceRegion.html</anchorfile>
      <anchor>944eec2af1100a526907d2b03a9779af</anchor>
      <arglist></arglist>
    </member>
  </compound>
  <compound kind="struct">
    <name>_LB_TraceRegion_Header</name>
    <filename>struct__LB__TraceRegion__Header.html</filename>
    <member kind="variable">
      <type>L_Func *</type>
      <name>fn</name>
      <anchorfile>struct__LB__TraceRegion__Header.html</anchorfile>
      <anchor>3b73f04b131ca85f721cf3d39f614c98</anchor>
      <arglist></arglist>
    </member>
    <member kind="variable">
      <type>int</type>
      <name>next_id</name>
      <anchorfile>struct__LB__TraceRegion__Header.html</anchorfile>
      <anchor>4d397c9b1eb35d2fb60ee26b749d4210</anchor>
      <arglist></arglist>
    </member>
    <member kind="variable">
      <type>List</type>
      <name>traceregions</name>
      <anchorfile>struct__LB__TraceRegion__Header.html</anchorfile>
      <anchor>82d62190ad360eb1d7d02d70fb8379aa</anchor>
      <arglist></arglist>
    </member>
    <member kind="variable">
      <type>List</type>
      <name>inorder_trs</name>
      <anchorfile>struct__LB__TraceRegion__Header.html</anchorfile>
      <anchor>8d467e8ee9092d2c16ed6306e25607d6</anchor>
      <arglist></arglist>
    </member>
  </compound>
  <compound kind="struct">
    <name>_Stack</name>
    <filename>struct__Stack.html</filename>
    <member kind="variable">
      <type>_Stack_item *</type>
      <name>head</name>
      <anchorfile>struct__Stack.html</anchorfile>
      <anchor>c3299c9dafba6fd6f0e72b815237a755</anchor>
      <arglist></arglist>
    </member>
    <member kind="variable">
      <type>_Stack_item *</type>
      <name>tail</name>
      <anchorfile>struct__Stack.html</anchorfile>
      <anchor>68486083dc343e1fde3518090462368f</anchor>
      <arglist></arglist>
    </member>
    <member kind="variable">
      <type>int</type>
      <name>size</name>
      <anchorfile>struct__Stack.html</anchorfile>
      <anchor>bb68932d5c508414e1e6c56feee2a81c</anchor>
      <arglist></arglist>
    </member>
  </compound>
  <compound kind="struct">
    <name>_Stack_item</name>
    <filename>struct__Stack__item.html</filename>
    <member kind="variable">
      <type>int</type>
      <name>data</name>
      <anchorfile>struct__Stack__item.html</anchorfile>
      <anchor>561348a23207750b73e6bd3fcca0508e</anchor>
      <arglist></arglist>
    </member>
    <member kind="variable">
      <type>_Stack_item *</type>
      <name>prev_item</name>
      <anchorfile>struct__Stack__item.html</anchorfile>
      <anchor>28e938f5d6242f057bb9c528258f61b5</anchor>
      <arglist></arglist>
    </member>
    <member kind="variable">
      <type>_Stack_item *</type>
      <name>next_item</name>
      <anchorfile>struct__Stack__item.html</anchorfile>
      <anchor>78759b78aef373bc11c20eaa25714d6d</anchor>
      <arglist></arglist>
    </member>
  </compound>
  <compound kind="struct">
    <name>My_path</name>
    <filename>structMy__path.html</filename>
    <member kind="variable">
      <type>Set</type>
      <name>nodes</name>
      <anchorfile>structMy__path.html</anchorfile>
      <anchor>1399dfbfda2e075bcf77f67e6f63f1e2</anchor>
      <arglist></arglist>
    </member>
    <member kind="variable">
      <type>double</type>
      <name>wcet</name>
      <anchorfile>structMy__path.html</anchorfile>
      <anchor>63a017eeb7837388ee79da764e68250a</anchor>
      <arglist></arglist>
    </member>
  </compound>
</tagfile>
