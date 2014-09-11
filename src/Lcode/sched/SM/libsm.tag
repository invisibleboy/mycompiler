<?xml version='1.0' encoding='ISO-8859-1' standalone='yes' ?>
<tagfile>
  <compound kind="file">
    <name>sm.c</name>
    <path>/mnt/hgfs/mkt/Desktop/openimpact/src/Lcode/sched/SM/</path>
    <filename>sm_8c</filename>
    <includes id="sm_8h" name="sm.h" local="yes" imported="no">sm.h</includes>
    <member kind="function">
      <type>void</type>
      <name>SM_assign_stop_bit</name>
      <anchorfile>sm_8c.html</anchorfile>
      <anchor>978b0390722d62a243df9ffb9b18daf6</anchor>
      <arglist>(SM_Issue_Group *issue_group_ptr)</arglist>
    </member>
    <member kind="function">
      <type>L_Oper *</type>
      <name>SM_create_template_op</name>
      <anchorfile>sm_8c.html</anchorfile>
      <anchor>31c3f7a83c2839da89911e89337524a9</anchor>
      <arglist>(int template_type, int stop_bit_mask)</arglist>
    </member>
    <member kind="function">
      <type>void</type>
      <name>SM_free_alloc_pools</name>
      <anchorfile>sm_8c.html</anchorfile>
      <anchor>8054846f9bd9554214386acbccab6f77</anchor>
      <arglist>()</arglist>
    </member>
    <member kind="function">
      <type>void</type>
      <name>SM_print_alloc_info</name>
      <anchorfile>sm_8c.html</anchorfile>
      <anchor>9a1254a896670662606b7d8ed666805d</anchor>
      <arglist>(FILE *out, int verbose)</arglist>
    </member>
    <member kind="function">
      <type>SM_Compatible_Alt *</type>
      <name>SM_build_compatible_alt_list</name>
      <anchorfile>sm_8c.html</anchorfile>
      <anchor>ac6dcee80e721f6a27aacd6aa3035531</anchor>
      <arglist>(L_Oper *lcode_op, Mdes *version1_mdes)</arglist>
    </member>
    <member kind="function">
      <type>void</type>
      <name>SM_ignore_op</name>
      <anchorfile>sm_8c.html</anchorfile>
      <anchor>7591f86e6353106f8bb6ea65ed18cca4</anchor>
      <arglist>(SM_Oper *sm_op, int flag)</arglist>
    </member>
    <member kind="function">
      <type>void</type>
      <name>SM_enable_ignored_op</name>
      <anchorfile>sm_8c.html</anchorfile>
      <anchor>e98be9e84c070117a227eb309f385e62</anchor>
      <arglist>(SM_Oper *sm_op, int flag)</arglist>
    </member>
    <member kind="function">
      <type>void</type>
      <name>SM_init_sm_op_fields</name>
      <anchorfile>sm_8c.html</anchorfile>
      <anchor>a35e053130e04e8b5ade33870b7d56f7</anchor>
      <arglist>(SM_Oper *sm_op, SM_Cb *sm_cb, L_Oper *lcode_op)</arglist>
    </member>
    <member kind="function">
      <type>SM_Oper *</type>
      <name>SM_find_sm_op</name>
      <anchorfile>sm_8c.html</anchorfile>
      <anchor>8a87e702f9b4517fc0791d11d8a20438</anchor>
      <arglist>(SM_Cb *sm_cb, L_Oper *l_oper)</arglist>
    </member>
    <member kind="function" static="yes">
      <type>static void</type>
      <name>SM_initialize_special_operands</name>
      <anchorfile>sm_8c.html</anchorfile>
      <anchor>9cf2bde633c836d3631e7054bbf6a23d</anchor>
      <arglist>()</arglist>
    </member>
    <member kind="function" static="yes">
      <type>static void</type>
      <name>SM_remove_matching_check</name>
      <anchorfile>sm_8c.html</anchorfile>
      <anchor>38e45c15cffdb2614ddc381e5781d3a9</anchor>
      <arglist>(SM_Cb *sm_cb, SM_Oper *sm_op)</arglist>
    </member>
    <member kind="function" static="yes">
      <type>static void</type>
      <name>SM_remove_if_unused</name>
      <anchorfile>sm_8c.html</anchorfile>
      <anchor>a4023973a8ba0cb98f15321df623c579</anchor>
      <arglist>(L_Cb *cb, L_Oper *chk_op)</arglist>
    </member>
    <member kind="function">
      <type>List</type>
      <name>SM_insert_checks</name>
      <anchorfile>sm_8c.html</anchorfile>
      <anchor>1593dec858611eaf6416de9b17c31500</anchor>
      <arglist>(L_Func *fn, L_Cb *cb)</arglist>
    </member>
    <member kind="function">
      <type>List</type>
      <name>SM_delete_checks</name>
      <anchorfile>sm_8c.html</anchorfile>
      <anchor>44e1863bd7b29c0dff892a844d338ee9</anchor>
      <arglist>(L_Func *fn, L_Cb *cb, List chk_list)</arglist>
    </member>
    <member kind="function">
      <type>SM_Cb *</type>
      <name>SM_new_cb</name>
      <anchorfile>sm_8c.html</anchorfile>
      <anchor>a3abf3f985c8090e785d3ed290c9c7d4</anchor>
      <arglist>(Mdes *version1_mdes, L_Cb *lcode_cb, int cb_flags)</arglist>
    </member>
    <member kind="function">
      <type>void</type>
      <name>SM_set_cb_II</name>
      <anchorfile>sm_8c.html</anchorfile>
      <anchor>ea90a092c9a21248f1d58f320e5c2856</anchor>
      <arglist>(SM_Cb *sm_cb, int II)</arglist>
    </member>
    <member kind="function">
      <type>void</type>
      <name>SM_delete_cb</name>
      <anchorfile>sm_8c.html</anchorfile>
      <anchor>48459436e1be4f10529483a6c5f6770c</anchor>
      <arglist>(SM_Cb *sm_cb)</arglist>
    </member>
    <member kind="function">
      <type>int</type>
      <name>SM_mutually_exclusive_opers</name>
      <anchorfile>sm_8c.html</anchorfile>
      <anchor>d963ed407189963fa72143591b5e57be</anchor>
      <arglist>(SM_Oper *sm_op1, SM_Oper *sm_op2)</arglist>
    </member>
    <member kind="function">
      <type>L_Attr *</type>
      <name>SM_attach_isl</name>
      <anchorfile>sm_8c.html</anchorfile>
      <anchor>4b07b52de68d6ffbaa5073b740d4b411</anchor>
      <arglist>(SM_Oper *sm_op, int sched_cycle_offset)</arglist>
    </member>
    <member kind="function">
      <type>int</type>
      <name>SM_set_sched_cycle_offset</name>
      <anchorfile>sm_8c.html</anchorfile>
      <anchor>3717231690ad643569cefb30c7b678a0</anchor>
      <arglist>(SM_Cb *sm_cb)</arglist>
    </member>
    <member kind="function">
      <type>void</type>
      <name>SM_commit_cb_template_bundling</name>
      <anchorfile>sm_8c.html</anchorfile>
      <anchor>e330884c40c54cf113b3fab716c33097</anchor>
      <arglist>(SM_Cb *sm_cb)</arglist>
    </member>
    <member kind="function">
      <type>void</type>
      <name>SM_commit_cb_no_bundling</name>
      <anchorfile>sm_8c.html</anchorfile>
      <anchor>eb20ade2bfc66ea8d1230be09a99b9ec</anchor>
      <arglist>(SM_Cb *sm_cb)</arglist>
    </member>
    <member kind="function">
      <type>void</type>
      <name>SM_commit_cb</name>
      <anchorfile>sm_8c.html</anchorfile>
      <anchor>e52798e66e79a170b5bb050ad226bc42</anchor>
      <arglist>(SM_Cb *sm_cb)</arglist>
    </member>
    <member kind="function">
      <type>void</type>
      <name>SM_construct_temp_kernel_queue</name>
      <anchorfile>sm_8c.html</anchorfile>
      <anchor>2ac7c1af255f6338383d14a62ab6677a</anchor>
      <arglist>(SM_Cb *sm_cb)</arglist>
    </member>
    <member kind="function">
      <type>void</type>
      <name>SM_print_cb_schedule</name>
      <anchorfile>sm_8c.html</anchorfile>
      <anchor>3179b40d974163706535d488d249b8b6</anchor>
      <arglist>(SM_Cb *sm_cb)</arglist>
    </member>
    <member kind="function">
      <type>void</type>
      <name>SM_delete_oper</name>
      <anchorfile>sm_8c.html</anchorfile>
      <anchor>0883cb5cc83db52dddea8ba365807322</anchor>
      <arglist>(SM_Oper *sm_op)</arglist>
    </member>
    <member kind="function">
      <type>SM_Oper *</type>
      <name>SM_insert_oper_after</name>
      <anchorfile>sm_8c.html</anchorfile>
      <anchor>15c481b9c7cd129ace579e1e290a4385</anchor>
      <arglist>(SM_Cb *sm_cb, L_Oper *lcode_op, SM_Oper *after_sm_op)</arglist>
    </member>
    <member kind="function">
      <type>void</type>
      <name>SM_move_oper_after</name>
      <anchorfile>sm_8c.html</anchorfile>
      <anchor>c5d14bd37ad526b73b73b21d4a241418</anchor>
      <arglist>(SM_Oper *move_sm_op, SM_Oper *after_sm_op)</arglist>
    </member>
    <member kind="function">
      <type>void</type>
      <name>SM_change_operand</name>
      <anchorfile>sm_8c.html</anchorfile>
      <anchor>6adc15973135f090ce8559baf51c028f</anchor>
      <arglist>(SM_Oper *sm_op, int operand_type, int operand_number, L_Operand *new_operand)</arglist>
    </member>
    <member kind="function">
      <type>int</type>
      <name>SM_action_cycle_lower_bound</name>
      <anchorfile>sm_8c.html</anchorfile>
      <anchor>2a5c98dff00dbdb553ab4f48ac653527</anchor>
      <arglist>(SM_Reg_Action *action)</arglist>
    </member>
    <member kind="function">
      <type>double</type>
      <name>SM_calc_best_case_cycles</name>
      <anchorfile>sm_8c.html</anchorfile>
      <anchor>63e79399198766f2c34c99309dcfb829</anchor>
      <arglist>(SM_Cb *sm_cb)</arglist>
    </member>
    <member kind="function">
      <type>int</type>
      <name>SM_softfix_promotion</name>
      <anchorfile>sm_8c.html</anchorfile>
      <anchor>f2ad6feaacfbb4e8d9c5d93e30aa0907</anchor>
      <arglist>(SM_Dep *dep, int issue_time, int mode)</arglist>
    </member>
    <member kind="function">
      <type>void</type>
      <name>SM_undo_fix_soft_dep</name>
      <anchorfile>sm_8c.html</anchorfile>
      <anchor>309e37e19f6cb298078ad0891a5d9044</anchor>
      <arglist>(SM_Oper *sm_op)</arglist>
    </member>
    <member kind="function">
      <type>int</type>
      <name>SM_fix_soft_dep</name>
      <anchorfile>sm_8c.html</anchorfile>
      <anchor>dcd7056f6802b676a0de5ab84548cdc5</anchor>
      <arglist>(SM_Oper *sm_op, int issue_time, int mode)</arglist>
    </member>
    <member kind="function">
      <type>void</type>
      <name>SM_schedule_cb</name>
      <anchorfile>sm_8c.html</anchorfile>
      <anchor>3fc49b883e223850ea59ce59aa1c3cf3</anchor>
      <arglist>(SM_Cb *sm_cb)</arglist>
    </member>
    <member kind="function" static="yes">
      <type>static int</type>
      <name>SM_min_late_time</name>
      <anchorfile>sm_8c.html</anchorfile>
      <anchor>c8d0125c1f5df24a76b4977e7c9d42ba</anchor>
      <arglist>(SM_Oper *sm_op)</arglist>
    </member>
    <member kind="function">
      <type>void</type>
      <name>SM_reduce_liveranges_cb</name>
      <anchorfile>sm_8c.html</anchorfile>
      <anchor>5e5e212b199c8c2fcf353eaead971200</anchor>
      <arglist>(SM_Cb *sm_cb)</arglist>
    </member>
    <member kind="function">
      <type>void</type>
      <name>SM_unschedule_cb</name>
      <anchorfile>sm_8c.html</anchorfile>
      <anchor>31ddcafff5a15dd2d9a9479f6ece561a</anchor>
      <arglist>(SM_Cb *sm_cb)</arglist>
    </member>
    <member kind="function">
      <type>void</type>
      <name>SM_schedule_fn</name>
      <anchorfile>sm_8c.html</anchorfile>
      <anchor>d4aaadc4c5ec107db5684d78c9313a63</anchor>
      <arglist>(L_Func *fn, Mdes *version1_mdes, int prepass_sched)</arglist>
    </member>
    <member kind="function">
      <type>void</type>
      <name>SM_bundle_fn</name>
      <anchorfile>sm_8c.html</anchorfile>
      <anchor>5abb42a0de1b07a09b620aa8b2124139</anchor>
      <arglist>(L_Func *fn, Mdes *version1_mdes)</arglist>
    </member>
    <member kind="function" static="yes">
      <type>static void</type>
      <name>L_read_parm_SM</name>
      <anchorfile>sm_8c.html</anchorfile>
      <anchor>38a2ad074dbbd1085de911eab1b5eaba</anchor>
      <arglist>(Parm_Parse_Info *ppi)</arglist>
    </member>
    <member kind="function">
      <type>void</type>
      <name>SM_init</name>
      <anchorfile>sm_8c.html</anchorfile>
      <anchor>777f79731c44d45d9946fac96cc44b67</anchor>
      <arglist>(Parm_Macro_List *command_line_macro_list)</arglist>
    </member>
    <member kind="variable">
      <type>L_Alloc_Pool *</type>
      <name>SM_Func_pool</name>
      <anchorfile>sm_8c.html</anchorfile>
      <anchor>4bcf990452639418820979221f99082e</anchor>
      <arglist></arglist>
    </member>
    <member kind="variable">
      <type>L_Alloc_Pool *</type>
      <name>SM_Cb_pool</name>
      <anchorfile>sm_8c.html</anchorfile>
      <anchor>2ecd6d6ded4676006d71b7f324108ba4</anchor>
      <arglist></arglist>
    </member>
    <member kind="variable">
      <type>L_Alloc_Pool *</type>
      <name>SM_Oper_pool</name>
      <anchorfile>sm_8c.html</anchorfile>
      <anchor>ed27c056b3105b7cc9340854ec1c0b95</anchor>
      <arglist></arglist>
    </member>
    <member kind="variable">
      <type>L_Alloc_Pool *</type>
      <name>SM_Compatible_Alt_pool</name>
      <anchorfile>sm_8c.html</anchorfile>
      <anchor>73663188d3bba621968636358e792dc8</anchor>
      <arglist></arglist>
    </member>
    <member kind="variable">
      <type>L_Alloc_Pool *</type>
      <name>SM_Issue_Group_pool</name>
      <anchorfile>sm_8c.html</anchorfile>
      <anchor>af71c74ddcf6fdffb424187f04a435b3</anchor>
      <arglist></arglist>
    </member>
    <member kind="variable">
      <type>L_Alloc_Pool *</type>
      <name>SM_Dep_pool</name>
      <anchorfile>sm_8c.html</anchorfile>
      <anchor>9bb765c5a4a3f49a00cf29bb3cff7305</anchor>
      <arglist></arglist>
    </member>
    <member kind="variable">
      <type>L_Alloc_Pool *</type>
      <name>SM_Oper_Queue_pool</name>
      <anchorfile>sm_8c.html</anchorfile>
      <anchor>642915ffa15fe72eba80fcdaa580a790</anchor>
      <arglist></arglist>
    </member>
    <member kind="variable">
      <type>L_Alloc_Pool *</type>
      <name>SM_Oper_Qentry_pool</name>
      <anchorfile>sm_8c.html</anchorfile>
      <anchor>25c4a6633e5b6c97a582aebce9089d9c</anchor>
      <arglist></arglist>
    </member>
    <member kind="variable">
      <type>L_Alloc_Pool *</type>
      <name>SM_Action_Queue_pool</name>
      <anchorfile>sm_8c.html</anchorfile>
      <anchor>65f1fab19e660426f050c91409a1c504</anchor>
      <arglist></arglist>
    </member>
    <member kind="variable">
      <type>L_Alloc_Pool *</type>
      <name>SM_Action_Qentry_pool</name>
      <anchorfile>sm_8c.html</anchorfile>
      <anchor>730ea808f5c245cc608b2680ae2d722c</anchor>
      <arglist></arglist>
    </member>
    <member kind="variable">
      <type>L_Alloc_Pool *</type>
      <name>SM_Trans_Queue_pool</name>
      <anchorfile>sm_8c.html</anchorfile>
      <anchor>9c45950817ebbf5ee1bb2dbc5991a15f</anchor>
      <arglist></arglist>
    </member>
    <member kind="variable">
      <type>L_Alloc_Pool *</type>
      <name>SM_Trans_Qentry_pool</name>
      <anchorfile>sm_8c.html</anchorfile>
      <anchor>309bb3a3a4cf33d209f7b7d72f391e1c</anchor>
      <arglist></arglist>
    </member>
    <member kind="variable">
      <type>L_Alloc_Pool *</type>
      <name>SM_Reg_Action_pool</name>
      <anchorfile>sm_8c.html</anchorfile>
      <anchor>260ccaf3384a0dfea38e2e926ba7bfcf</anchor>
      <arglist></arglist>
    </member>
    <member kind="variable">
      <type>L_Alloc_Pool *</type>
      <name>SM_Reg_Info_pool</name>
      <anchorfile>sm_8c.html</anchorfile>
      <anchor>eaa41f8aa5f5cc1450a47a4e5fe66330</anchor>
      <arglist></arglist>
    </member>
    <member kind="variable">
      <type>L_Alloc_Pool *</type>
      <name>SM_Reg_Action_Conflict_pool</name>
      <anchorfile>sm_8c.html</anchorfile>
      <anchor>1ff695952d54a08314adb2dd0e3771d0</anchor>
      <arglist></arglist>
    </member>
    <member kind="variable">
      <type>L_Alloc_Pool *</type>
      <name>SM_Trans_pool</name>
      <anchorfile>sm_8c.html</anchorfile>
      <anchor>830a3d222b7438f0cb24be9090561d1b</anchor>
      <arglist></arglist>
    </member>
    <member kind="variable">
      <type>L_Alloc_Pool *</type>
      <name>SM_Priority_Queue_pool</name>
      <anchorfile>sm_8c.html</anchorfile>
      <anchor>8bdc1d4ae109504ae7e48943681103fb</anchor>
      <arglist></arglist>
    </member>
    <member kind="variable">
      <type>L_Alloc_Pool *</type>
      <name>SM_Priority_Qentry_pool</name>
      <anchorfile>sm_8c.html</anchorfile>
      <anchor>af366ac7278ce98e19580665340b4e92</anchor>
      <arglist></arglist>
    </member>
    <member kind="variable">
      <type>L_Operand</type>
      <name>_sm_mem_action_operand</name>
      <anchorfile>sm_8c.html</anchorfile>
      <anchor>e7fe0abd473941eb00c58aef727de47e</anchor>
      <arglist></arglist>
    </member>
    <member kind="variable">
      <type>L_Operand</type>
      <name>_sm_ctrl_action_operand</name>
      <anchorfile>sm_8c.html</anchorfile>
      <anchor>c2e1e9d6130b4f86edc1be2465acec34</anchor>
      <arglist></arglist>
    </member>
    <member kind="variable">
      <type>L_Operand</type>
      <name>_sm_sync_action_operand</name>
      <anchorfile>sm_8c.html</anchorfile>
      <anchor>e52b5e3b36e3d39d4b519286c096916d</anchor>
      <arglist></arglist>
    </member>
    <member kind="variable">
      <type>L_Operand</type>
      <name>_sm_vliw_action_operand</name>
      <anchorfile>sm_8c.html</anchorfile>
      <anchor>97b62f78e5420a1adb546de69059cae9</anchor>
      <arglist></arglist>
    </member>
    <member kind="variable">
      <type>char *</type>
      <name>prof_info</name>
      <anchorfile>sm_8c.html</anchorfile>
      <anchor>624cb4649f64d5ec16b959e5d68e25b8</anchor>
      <arglist></arglist>
    </member>
    <member kind="variable">
      <type>int</type>
      <name>suppress_lcode_output</name>
      <anchorfile>sm_8c.html</anchorfile>
      <anchor>d1c23d1e07d66124ee80af0c858ae5da</anchor>
      <arglist></arglist>
    </member>
    <member kind="variable">
      <type>int</type>
      <name>SM_debug_use_sched_cb_bounds</name>
      <anchorfile>sm_8c.html</anchorfile>
      <anchor>0e989c9ef319284e13edcf549fbce1f9</anchor>
      <arglist></arglist>
    </member>
    <member kind="variable">
      <type>int</type>
      <name>SM_debug_lower_sched_cb_bound</name>
      <anchorfile>sm_8c.html</anchorfile>
      <anchor>8b15ed0b8ad4788a93dc8171a259da5e</anchor>
      <arglist></arglist>
    </member>
    <member kind="variable">
      <type>int</type>
      <name>SM_debug_upper_sched_cb_bound</name>
      <anchorfile>sm_8c.html</anchorfile>
      <anchor>8dfae2cab053bb201fe3253760fa96b9</anchor>
      <arglist></arglist>
    </member>
    <member kind="variable">
      <type>int</type>
      <name>SM_print_dependence_graph</name>
      <anchorfile>sm_8c.html</anchorfile>
      <anchor>5da67169cfe0dc13778e6f3bcc072ce9</anchor>
      <arglist></arglist>
    </member>
    <member kind="variable">
      <type>int</type>
      <name>SM_check_dependence_symmetry</name>
      <anchorfile>sm_8c.html</anchorfile>
      <anchor>6113fa13a710c6bd4de9af49ad33c02e</anchor>
      <arglist></arglist>
    </member>
    <member kind="variable">
      <type>int</type>
      <name>SM_verify_reg_conflicts</name>
      <anchorfile>sm_8c.html</anchorfile>
      <anchor>f00f8f047efcd90a65c217f021f5119e</anchor>
      <arglist></arglist>
    </member>
    <member kind="variable">
      <type>int</type>
      <name>SM_output_dep_distance</name>
      <anchorfile>sm_8c.html</anchorfile>
      <anchor>ab0a90fa5ee31aa6b37030bb42e1742c</anchor>
      <arglist></arglist>
    </member>
    <member kind="variable">
      <type>int</type>
      <name>SM_ignore_pred_analysis</name>
      <anchorfile>sm_8c.html</anchorfile>
      <anchor>7ebc6c47c0604862b6cfe9eaea19059b</anchor>
      <arglist></arglist>
    </member>
    <member kind="variable">
      <type>int</type>
      <name>SM_perform_rename_with_copy</name>
      <anchorfile>sm_8c.html</anchorfile>
      <anchor>9075336d29d43ce08eb1cf90de03eb9d</anchor>
      <arglist></arglist>
    </member>
    <member kind="variable">
      <type>int</type>
      <name>SM_perform_relocate_cond_opti</name>
      <anchorfile>sm_8c.html</anchorfile>
      <anchor>5b707937b3ff9286ac4dbcfa937222b7</anchor>
      <arglist></arglist>
    </member>
    <member kind="variable">
      <type>int</type>
      <name>SM_sched_slack_loads_for_miss</name>
      <anchorfile>sm_8c.html</anchorfile>
      <anchor>0ae4b2ad45eaa593d23cd4512ef97ff7</anchor>
      <arglist></arglist>
    </member>
    <member kind="variable">
      <type>int</type>
      <name>SM_do_template_bundling</name>
      <anchorfile>sm_8c.html</anchorfile>
      <anchor>d9c780f4de4ccbbea7c2e754d2954c64</anchor>
      <arglist></arglist>
    </member>
    <member kind="variable">
      <type>int</type>
      <name>SM_do_bundle_compaction</name>
      <anchorfile>sm_8c.html</anchorfile>
      <anchor>6918c004ce79dd79b0171d57f5d931a3</anchor>
      <arglist></arglist>
    </member>
  </compound>
  <compound kind="file">
    <name>sm.h</name>
    <path>/mnt/hgfs/mkt/Desktop/openimpact/src/Lcode/sched/SM/</path>
    <filename>sm_8h</filename>
    <class kind="struct">SM_Oper_Queue</class>
    <class kind="struct">SM_Oper_Qentry</class>
    <class kind="struct">SM_Action_Queue</class>
    <class kind="struct">SM_Action_Qentry</class>
    <class kind="struct">SM_Trans_Queue</class>
    <class kind="struct">SM_Trans_Qentry</class>
    <class kind="struct">SM_Priority_Queue</class>
    <class kind="struct">SM_Priority_Qentry</class>
    <class kind="struct">SM_Reg_Action</class>
    <class kind="struct">SM_Reg_Info</class>
    <class kind="struct">SM_Dep_PCLat</class>
    <class kind="struct">SM_Dep</class>
    <class kind="struct">SM_Compatible_Alt</class>
    <class kind="struct">SM_Oper</class>
    <class kind="struct">SM_Bundle</class>
    <class kind="struct">SM_Issue_Group</class>
    <class kind="struct">SM_Cb</class>
    <class kind="struct">SM_Func</class>
    <class kind="struct">SM_Trans</class>
    <class kind="struct">SM_Stats</class>
    <member kind="define">
      <type>#define</type>
      <name>DEBUG_PCLAT</name>
      <anchorfile>sm_8h.html</anchorfile>
      <anchor>77cb66df8a8935fc4e60b6b2a39febbe</anchor>
      <arglist></arglist>
    </member>
    <member kind="define">
      <type>#define</type>
      <name>SM_DEF_ACTION</name>
      <anchorfile>sm_8h.html</anchorfile>
      <anchor>51d041daa13474688c0968ef586ad52b</anchor>
      <arglist></arglist>
    </member>
    <member kind="define">
      <type>#define</type>
      <name>SM_USE_ACTION</name>
      <anchorfile>sm_8h.html</anchorfile>
      <anchor>65ff29b25658e7d8940a9d8ce66f005a</anchor>
      <arglist></arglist>
    </member>
    <member kind="define">
      <type>#define</type>
      <name>SM_ACTUAL_ACTION</name>
      <anchorfile>sm_8h.html</anchorfile>
      <anchor>df29470ea6dcc7eb64b32cf73bc6a8cd</anchor>
      <arglist></arglist>
    </member>
    <member kind="define">
      <type>#define</type>
      <name>SM_CONFLICTING_ACTION</name>
      <anchorfile>sm_8h.html</anchorfile>
      <anchor>4017ef20a6ea18cd68f577ea26376dfa</anchor>
      <arglist></arglist>
    </member>
    <member kind="define">
      <type>#define</type>
      <name>SM_PRED_UNCOND_DEF</name>
      <anchorfile>sm_8h.html</anchorfile>
      <anchor>d1ebc83d4fdabaff8caf36962ad0a613</anchor>
      <arglist></arglist>
    </member>
    <member kind="define">
      <type>#define</type>
      <name>SM_PRED_TRANS_DEF</name>
      <anchorfile>sm_8h.html</anchorfile>
      <anchor>b385d27dc4e2da46b56d9741e1a7e729</anchor>
      <arglist></arglist>
    </member>
    <member kind="define">
      <type>#define</type>
      <name>SM_PRED_UNCOND_USE</name>
      <anchorfile>sm_8h.html</anchorfile>
      <anchor>94806f7c30a587c106e83f12d4cbe015</anchor>
      <arglist></arglist>
    </member>
    <member kind="define">
      <type>#define</type>
      <name>SM_PRED_COND_USE</name>
      <anchorfile>sm_8h.html</anchorfile>
      <anchor>ab960cdb11f595b3cd0695d6f4919a60</anchor>
      <arglist></arglist>
    </member>
    <member kind="define">
      <type>#define</type>
      <name>SM_EXPLICIT_ACTION</name>
      <anchorfile>sm_8h.html</anchorfile>
      <anchor>a399e73d6ff334aa67ec3cc03527a71e</anchor>
      <arglist></arglist>
    </member>
    <member kind="define">
      <type>#define</type>
      <name>SM_IMPLICIT_ACTION</name>
      <anchorfile>sm_8h.html</anchorfile>
      <anchor>7c48f8080ebd2dfacc18312c36842900</anchor>
      <arglist></arglist>
    </member>
    <member kind="define">
      <type>#define</type>
      <name>SM_VARIABLE_ACTION</name>
      <anchorfile>sm_8h.html</anchorfile>
      <anchor>474512ae493d5cece916cfe7f0dc9994</anchor>
      <arglist></arglist>
    </member>
    <member kind="define">
      <type>#define</type>
      <name>SM_TRANSPARENT_ACTION</name>
      <anchorfile>sm_8h.html</anchorfile>
      <anchor>eb84893bdbeae18a83989acf2e26854d</anchor>
      <arglist></arglist>
    </member>
    <member kind="define">
      <type>#define</type>
      <name>SM_LIVEIN_ACTION</name>
      <anchorfile>sm_8h.html</anchorfile>
      <anchor>096be8289bee5adffbfd263bf790c1b9</anchor>
      <arglist></arglist>
    </member>
    <member kind="define">
      <type>#define</type>
      <name>SM_FRAGILE_MACRO</name>
      <anchorfile>sm_8h.html</anchorfile>
      <anchor>7e68ae94b83c4862dd0baf3e21da4ab9</anchor>
      <arglist></arglist>
    </member>
    <member kind="define">
      <type>#define</type>
      <name>SM_REG_LIVE_IN</name>
      <anchorfile>sm_8h.html</anchorfile>
      <anchor>116be575c4139ed7a3000882ab7fe585</anchor>
      <arglist></arglist>
    </member>
    <member kind="define">
      <type>#define</type>
      <name>SM_FIXED_DELAY</name>
      <anchorfile>sm_8h.html</anchorfile>
      <anchor>3c42592e8b3fba29b7cacfc3f6edd0d4</anchor>
      <arglist></arglist>
    </member>
    <member kind="define">
      <type>#define</type>
      <name>SM_MDES_BASED_DELAY</name>
      <anchorfile>sm_8h.html</anchorfile>
      <anchor>5961a76caac304e24e13909492b54015</anchor>
      <arglist></arglist>
    </member>
    <member kind="define">
      <type>#define</type>
      <name>SM_VARIABLE_DELAY</name>
      <anchorfile>sm_8h.html</anchorfile>
      <anchor>b533a25a5662e6a2d6d3fdd562508507</anchor>
      <arglist></arglist>
    </member>
    <member kind="define">
      <type>#define</type>
      <name>SM_HARD_DEP</name>
      <anchorfile>sm_8h.html</anchorfile>
      <anchor>a132aa599b30be8d2dc30e17911a04be</anchor>
      <arglist></arglist>
    </member>
    <member kind="define">
      <type>#define</type>
      <name>SM_SOFT_DEP</name>
      <anchorfile>sm_8h.html</anchorfile>
      <anchor>bff4fbec1ce79ed404423ab2eb6183fe</anchor>
      <arglist></arglist>
    </member>
    <member kind="define">
      <type>#define</type>
      <name>SM_REG_DEP</name>
      <anchorfile>sm_8h.html</anchorfile>
      <anchor>4bd0796878efbf928852d0056b5df803</anchor>
      <arglist></arglist>
    </member>
    <member kind="define">
      <type>#define</type>
      <name>SM_MEM_DEP</name>
      <anchorfile>sm_8h.html</anchorfile>
      <anchor>6a71f16addc76974b2f3c5ca72da2710</anchor>
      <arglist></arglist>
    </member>
    <member kind="define">
      <type>#define</type>
      <name>SM_CTRL_DEP</name>
      <anchorfile>sm_8h.html</anchorfile>
      <anchor>87ec6c45c70be91ad5a1b3f7788a11f1</anchor>
      <arglist></arglist>
    </member>
    <member kind="define">
      <type>#define</type>
      <name>SM_SYNC_DEP</name>
      <anchorfile>sm_8h.html</anchorfile>
      <anchor>995c3ac7e1b553bb6826ce86befe1965</anchor>
      <arglist></arglist>
    </member>
    <member kind="define">
      <type>#define</type>
      <name>SM_VLIW_DEP</name>
      <anchorfile>sm_8h.html</anchorfile>
      <anchor>c07a5d6e7a4e11265313d913de92346a</anchor>
      <arglist></arglist>
    </member>
    <member kind="define">
      <type>#define</type>
      <name>SM_FLOW_DEP</name>
      <anchorfile>sm_8h.html</anchorfile>
      <anchor>a312013962761455a622b093085a6176</anchor>
      <arglist></arglist>
    </member>
    <member kind="define">
      <type>#define</type>
      <name>SM_ANTI_DEP</name>
      <anchorfile>sm_8h.html</anchorfile>
      <anchor>1715b9f8c4e846fe32075b63211d238a</anchor>
      <arglist></arglist>
    </member>
    <member kind="define">
      <type>#define</type>
      <name>SM_OUTPUT_DEP</name>
      <anchorfile>sm_8h.html</anchorfile>
      <anchor>6c1cb6ca5171bb1462c57cd4f0862e47</anchor>
      <arglist></arglist>
    </member>
    <member kind="define">
      <type>#define</type>
      <name>SM_BUILD_DEP_IN</name>
      <anchorfile>sm_8h.html</anchorfile>
      <anchor>547205573a97b28f2b4a1c9805cebc54</anchor>
      <arglist></arglist>
    </member>
    <member kind="define">
      <type>#define</type>
      <name>SM_BUILD_DEP_OUT</name>
      <anchorfile>sm_8h.html</anchorfile>
      <anchor>9091be27301195bba452de7f0795c87c</anchor>
      <arglist></arglist>
    </member>
    <member kind="define">
      <type>#define</type>
      <name>SM_PREVENT_REDUNDANT_DEPS</name>
      <anchorfile>sm_8h.html</anchorfile>
      <anchor>0eccffaf6d53f5c0a47ad871db3cdb5c</anchor>
      <arglist></arglist>
    </member>
    <member kind="define">
      <type>#define</type>
      <name>SM_USER_DEP_MARK1</name>
      <anchorfile>sm_8h.html</anchorfile>
      <anchor>a9d5e9710bf0143c40f4c4a26c1b68b8</anchor>
      <arglist></arglist>
    </member>
    <member kind="define">
      <type>#define</type>
      <name>SM_USER_DEP_MARK2</name>
      <anchorfile>sm_8h.html</anchorfile>
      <anchor>421bd93c7fdc90f817ae359398aac9b9</anchor>
      <arglist></arglist>
    </member>
    <member kind="define">
      <type>#define</type>
      <name>SM_USER_DEP_MARK3</name>
      <anchorfile>sm_8h.html</anchorfile>
      <anchor>ef11797c7adf7f1c1fd0151d6be29f8d</anchor>
      <arglist></arglist>
    </member>
    <member kind="define">
      <type>#define</type>
      <name>SM_USER_DEP_MARK4</name>
      <anchorfile>sm_8h.html</anchorfile>
      <anchor>ef4db4a375a29942e78aad9c2e6a7ade</anchor>
      <arglist></arglist>
    </member>
    <member kind="define">
      <type>#define</type>
      <name>SM_SOFTFIX_PROMOTION</name>
      <anchorfile>sm_8h.html</anchorfile>
      <anchor>0a69d5e30350b7e28bc22f907a1c82a7</anchor>
      <arglist></arglist>
    </member>
    <member kind="define">
      <type>#define</type>
      <name>SM_FORCE_DEP</name>
      <anchorfile>sm_8h.html</anchorfile>
      <anchor>875880cc41f42d6ed3e34f5344adc29f</anchor>
      <arglist></arglist>
    </member>
    <member kind="define">
      <type>#define</type>
      <name>SOFTFIX_POSSIBLE</name>
      <anchorfile>sm_8h.html</anchorfile>
      <anchor>0eb5c2d1fdd7723af7a11cff68782d0a</anchor>
      <arglist></arglist>
    </member>
    <member kind="define">
      <type>#define</type>
      <name>SOFTFIX_COMMIT</name>
      <anchorfile>sm_8h.html</anchorfile>
      <anchor>e3a2393b63f4cba81c021ee0769629b4</anchor>
      <arglist></arglist>
    </member>
    <member kind="define">
      <type>#define</type>
      <name>SOFTFIX_UNDO</name>
      <anchorfile>sm_8h.html</anchorfile>
      <anchor>b7fc963aea206f6ac56586d360bfaa79</anchor>
      <arglist></arglist>
    </member>
    <member kind="define">
      <type>#define</type>
      <name>SM_HAS_SILENT_VERSION</name>
      <anchorfile>sm_8h.html</anchorfile>
      <anchor>eac2cb52e82051ae49d577b17c1d5107</anchor>
      <arglist></arglist>
    </member>
    <member kind="define">
      <type>#define</type>
      <name>SM_OP_SCHEDULED</name>
      <anchorfile>sm_8h.html</anchorfile>
      <anchor>1cb0e46f79a59de1c7faf2b0dbedb85c</anchor>
      <arglist></arglist>
    </member>
    <member kind="define">
      <type>#define</type>
      <name>SM_OP_SPECULATIVE</name>
      <anchorfile>sm_8h.html</anchorfile>
      <anchor>3b38e7012339e7ff20b04b713fcf488f</anchor>
      <arglist></arglist>
    </member>
    <member kind="define">
      <type>#define</type>
      <name>SM_OP_SILENT</name>
      <anchorfile>sm_8h.html</anchorfile>
      <anchor>dc9b47f22b899631eda83534546410e2</anchor>
      <arglist></arglist>
    </member>
    <member kind="define">
      <type>#define</type>
      <name>SM_OP_TESTED</name>
      <anchorfile>sm_8h.html</anchorfile>
      <anchor>66c3782d13148bef93ec30d1d8daab8b</anchor>
      <arglist></arglist>
    </member>
    <member kind="define">
      <type>#define</type>
      <name>SM_RWC_UNBENEFICIAL</name>
      <anchorfile>sm_8h.html</anchorfile>
      <anchor>2644c1ccf1b73a27302d73de7361c448</anchor>
      <arglist></arglist>
    </member>
    <member kind="define">
      <type>#define</type>
      <name>SM_EWC_UNBENEFICIAL</name>
      <anchorfile>sm_8h.html</anchorfile>
      <anchor>b398114f6f30ae9b4e87ea49a546ec69</anchor>
      <arglist></arglist>
    </member>
    <member kind="define">
      <type>#define</type>
      <name>SM_OP_VARIABLE_ACTIONS</name>
      <anchorfile>sm_8h.html</anchorfile>
      <anchor>2ab582a5fd817494c1b295ed2252e02d</anchor>
      <arglist></arglist>
    </member>
    <member kind="define">
      <type>#define</type>
      <name>SM_USER_OP_MARK1</name>
      <anchorfile>sm_8h.html</anchorfile>
      <anchor>28698f2ab7ddec540d178209adc8c509</anchor>
      <arglist></arglist>
    </member>
    <member kind="define">
      <type>#define</type>
      <name>SM_USER_OP_MARK2</name>
      <anchorfile>sm_8h.html</anchorfile>
      <anchor>9d2c7fddb56ba35c2876509020d0526d</anchor>
      <arglist></arglist>
    </member>
    <member kind="define">
      <type>#define</type>
      <name>SM_USER_OP_MARK3</name>
      <anchorfile>sm_8h.html</anchorfile>
      <anchor>44908692c2bf3e1d39e73d1dcfcca054</anchor>
      <arglist></arglist>
    </member>
    <member kind="define">
      <type>#define</type>
      <name>SM_USER_OP_MARK4</name>
      <anchorfile>sm_8h.html</anchorfile>
      <anchor>1b59aaee77359625e59a6218c94bbe9d</anchor>
      <arglist></arglist>
    </member>
    <member kind="define">
      <type>#define</type>
      <name>SM_LOOP_BACK_BR</name>
      <anchorfile>sm_8h.html</anchorfile>
      <anchor>6ea6f14fb0429c44def7c161244fe4d3</anchor>
      <arglist></arglist>
    </member>
    <member kind="define">
      <type>#define</type>
      <name>SM_CAN_SPEC_IGNORE</name>
      <anchorfile>sm_8h.html</anchorfile>
      <anchor>31712ba59b659c74b38c1be44dbb9258</anchor>
      <arglist></arglist>
    </member>
    <member kind="define">
      <type>#define</type>
      <name>SM_USER_IGNORE_MARK1</name>
      <anchorfile>sm_8h.html</anchorfile>
      <anchor>b7e9531ccf70cea73aad1c6328bb5bb7</anchor>
      <arglist></arglist>
    </member>
    <member kind="define">
      <type>#define</type>
      <name>SM_USER_IGNORE_MARK2</name>
      <anchorfile>sm_8h.html</anchorfile>
      <anchor>482546f336a777ad21a87872b824bc90</anchor>
      <arglist></arglist>
    </member>
    <member kind="define">
      <type>#define</type>
      <name>SM_USER_IGNORE_MARK3</name>
      <anchorfile>sm_8h.html</anchorfile>
      <anchor>25baa56e6dbb36c9a3177b90ebcad505</anchor>
      <arglist></arglist>
    </member>
    <member kind="define">
      <type>#define</type>
      <name>SM_USER_IGNORE_MARK4</name>
      <anchorfile>sm_8h.html</anchorfile>
      <anchor>d2f93c896c5a3b8e99031073f428e691</anchor>
      <arglist></arglist>
    </member>
    <member kind="define">
      <type>#define</type>
      <name>SM_PRIC_PRIO_IGNORE</name>
      <anchorfile>sm_8h.html</anchorfile>
      <anchor>50ecf9796f1b2870feb3984387600023</anchor>
      <arglist></arglist>
    </member>
    <member kind="define">
      <type>#define</type>
      <name>SM_MVE_IGNORE</name>
      <anchorfile>sm_8h.html</anchorfile>
      <anchor>9ad2806dd5b02ac768ecbe8153eb871a</anchor>
      <arglist></arglist>
    </member>
    <member kind="define">
      <type>#define</type>
      <name>SM_PREPASS</name>
      <anchorfile>sm_8h.html</anchorfile>
      <anchor>b6f5a8525404290f28629d80ac61a0a8</anchor>
      <arglist></arglist>
    </member>
    <member kind="define">
      <type>#define</type>
      <name>SM_POSTPASS</name>
      <anchorfile>sm_8h.html</anchorfile>
      <anchor>b4ba300b4dba82d31e75226127116e23</anchor>
      <arglist></arglist>
    </member>
    <member kind="define">
      <type>#define</type>
      <name>SM_DHASY</name>
      <anchorfile>sm_8h.html</anchorfile>
      <anchor>1271816703f133a8c9ddb0d1a14de1f5</anchor>
      <arglist></arglist>
    </member>
    <member kind="define">
      <type>#define</type>
      <name>SM_MODULO</name>
      <anchorfile>sm_8h.html</anchorfile>
      <anchor>50988797370cea1fa3b8446bb99201b9</anchor>
      <arglist></arglist>
    </member>
    <member kind="define">
      <type>#define</type>
      <name>SM_NORMALIZE_ISSUE</name>
      <anchorfile>sm_8h.html</anchorfile>
      <anchor>5e5919e0b9090b92f54d9af0f05d5787</anchor>
      <arglist></arglist>
    </member>
    <member kind="define">
      <type>#define</type>
      <name>SM_CROSS_ITERATION</name>
      <anchorfile>sm_8h.html</anchorfile>
      <anchor>fc2fbb912e64db02b500e54df8f01e5f</anchor>
      <arglist></arglist>
    </member>
    <member kind="define">
      <type>#define</type>
      <name>SM_ASSUME_MVE</name>
      <anchorfile>sm_8h.html</anchorfile>
      <anchor>28372b76f06f005f72f21f8322d617e3</anchor>
      <arglist></arglist>
    </member>
    <member kind="define">
      <type>#define</type>
      <name>SM_MODULO_RESOURCES</name>
      <anchorfile>sm_8h.html</anchorfile>
      <anchor>30663b1a0bf8b6681abee1d4e16429d8</anchor>
      <arglist></arglist>
    </member>
    <member kind="define">
      <type>#define</type>
      <name>SM_CB_HAS_FALL_THRU</name>
      <anchorfile>sm_8h.html</anchorfile>
      <anchor>0f13c5adf3045535f28eab374d9af364</anchor>
      <arglist></arglist>
    </member>
    <member kind="define">
      <type>#define</type>
      <name>SM_CB_NO_FALL_THRU</name>
      <anchorfile>sm_8h.html</anchorfile>
      <anchor>139f93e207eee9ff379499da2752d297</anchor>
      <arglist></arglist>
    </member>
    <member kind="define">
      <type>#define</type>
      <name>SM_CB_TEST_ONLY_ONE_ALT</name>
      <anchorfile>sm_8h.html</anchorfile>
      <anchor>6aa2485cbdf4c028fe9956886bb7cef3</anchor>
      <arglist></arglist>
    </member>
    <member kind="define">
      <type>#define</type>
      <name>SM_CB_RESERVED_FLAGS</name>
      <anchorfile>sm_8h.html</anchorfile>
      <anchor>6104fca7d89563a66c1cc44e7ce23127</anchor>
      <arglist></arglist>
    </member>
    <member kind="define">
      <type>#define</type>
      <name>SM_SEQUENTIAL</name>
      <anchorfile>sm_8h.html</anchorfile>
      <anchor>5e7ab35e37c3a825819dff8ef4d2f551</anchor>
      <arglist></arglist>
    </member>
    <member kind="define">
      <type>#define</type>
      <name>SM_TEST_ONLY</name>
      <anchorfile>sm_8h.html</anchorfile>
      <anchor>084a3a2660581c8fbe924f5601c7f0c7</anchor>
      <arglist></arglist>
    </member>
    <member kind="define">
      <type>#define</type>
      <name>SM_MEM_ACTION_INDEX</name>
      <anchorfile>sm_8h.html</anchorfile>
      <anchor>f69daa61551ee8309d7672b699266af9</anchor>
      <arglist></arglist>
    </member>
    <member kind="define">
      <type>#define</type>
      <name>SM_CTRL_ACTION_INDEX</name>
      <anchorfile>sm_8h.html</anchorfile>
      <anchor>48daf21e03f04f2db39727b024a87e5d</anchor>
      <arglist></arglist>
    </member>
    <member kind="define">
      <type>#define</type>
      <name>SM_SYNC_ACTION_INDEX</name>
      <anchorfile>sm_8h.html</anchorfile>
      <anchor>8c681e935d56a8858a44b3ecd45118a6</anchor>
      <arglist></arglist>
    </member>
    <member kind="define">
      <type>#define</type>
      <name>SM_VLIW_ACTION_INDEX</name>
      <anchorfile>sm_8h.html</anchorfile>
      <anchor>86c1773c9d48301aefea2590add46163</anchor>
      <arglist></arglist>
    </member>
    <member kind="define">
      <type>#define</type>
      <name>SM_MEM_ACTION_OPERAND</name>
      <anchorfile>sm_8h.html</anchorfile>
      <anchor>3b0215dd90b8089236f0c188711af6ec</anchor>
      <arglist></arglist>
    </member>
    <member kind="define">
      <type>#define</type>
      <name>SM_CTRL_ACTION_OPERAND</name>
      <anchorfile>sm_8h.html</anchorfile>
      <anchor>80157e33712e17c44ca3f083d4ab77f5</anchor>
      <arglist></arglist>
    </member>
    <member kind="define">
      <type>#define</type>
      <name>SM_SYNC_ACTION_OPERAND</name>
      <anchorfile>sm_8h.html</anchorfile>
      <anchor>5fdcbaa2b827ff4ae58c4631ebd137f6</anchor>
      <arglist></arglist>
    </member>
    <member kind="define">
      <type>#define</type>
      <name>SM_VLIW_ACTION_OPERAND</name>
      <anchorfile>sm_8h.html</anchorfile>
      <anchor>a77ce2e278ba82c76c587e424e05e91b</anchor>
      <arglist></arglist>
    </member>
    <member kind="define">
      <type>#define</type>
      <name>SM_REGISTER_TYPE</name>
      <anchorfile>sm_8h.html</anchorfile>
      <anchor>1035e2ef3e636c739720e88b2746a305</anchor>
      <arglist></arglist>
    </member>
    <member kind="define">
      <type>#define</type>
      <name>SM_MACRO_TYPE</name>
      <anchorfile>sm_8h.html</anchorfile>
      <anchor>f34f0b5bb3aa0faaa31226154de76ed0</anchor>
      <arglist></arglist>
    </member>
    <member kind="define">
      <type>#define</type>
      <name>SM_EXT_ACTION_TYPE</name>
      <anchorfile>sm_8h.html</anchorfile>
      <anchor>4d318f8d90a411f1db03282eed74a515</anchor>
      <arglist></arglist>
    </member>
    <member kind="define">
      <type>#define</type>
      <name>SM_CTYPE_DOUBLE</name>
      <anchorfile>sm_8h.html</anchorfile>
      <anchor>7f41749d5d7f90db1230756a83f39234</anchor>
      <arglist></arglist>
    </member>
    <member kind="define">
      <type>#define</type>
      <name>MAX_SM_TYPE_POWER</name>
      <anchorfile>sm_8h.html</anchorfile>
      <anchor>5b101d26060b7cbf771869d9643422e2</anchor>
      <arglist></arglist>
    </member>
    <member kind="define">
      <type>#define</type>
      <name>RENAMING_WITH_COPY</name>
      <anchorfile>sm_8h.html</anchorfile>
      <anchor>993b6e28699e627f4ae8a98ce24de284</anchor>
      <arglist></arglist>
    </member>
    <member kind="define">
      <type>#define</type>
      <name>EXPR_WITH_COPY</name>
      <anchorfile>sm_8h.html</anchorfile>
      <anchor>961e7600346a7e5f482cfa81d8ae625f</anchor>
      <arglist></arglist>
    </member>
    <member kind="define">
      <type>#define</type>
      <name>SM_CAN_TRANS_SRC0</name>
      <anchorfile>sm_8h.html</anchorfile>
      <anchor>988e3137cff8dbf186d13bde5e8f7d30</anchor>
      <arglist></arglist>
    </member>
    <member kind="define">
      <type>#define</type>
      <name>SM_CAN_TRANS_SRC1</name>
      <anchorfile>sm_8h.html</anchorfile>
      <anchor>fb29ff4c2d506a7e2b98d7e1929153a5</anchor>
      <arglist></arglist>
    </member>
    <member kind="define">
      <type>#define</type>
      <name>SM_CANNOT_DO_TRANS</name>
      <anchorfile>sm_8h.html</anchorfile>
      <anchor>2c003073c348dc4ee354f7bb753d443a</anchor>
      <arglist></arglist>
    </member>
    <member kind="define">
      <type>#define</type>
      <name>SM_IGNORE_TRANS</name>
      <anchorfile>sm_8h.html</anchorfile>
      <anchor>9327082f3783510d3a824acdf82683d2</anchor>
      <arglist></arglist>
    </member>
    <member kind="define">
      <type>#define</type>
      <name>SM_MUST_REORDER_OPS</name>
      <anchorfile>sm_8h.html</anchorfile>
      <anchor>c7035e1db366ed8f7f338fbdc6d60e29</anchor>
      <arglist></arglist>
    </member>
    <member kind="define">
      <type>#define</type>
      <name>SM_NEEDS_RENAMING</name>
      <anchorfile>sm_8h.html</anchorfile>
      <anchor>dc66f1d2e54cd8a2840d31f32a1cadcc</anchor>
      <arglist></arglist>
    </member>
    <member kind="define">
      <type>#define</type>
      <name>SM_DUPLICATED_DEF</name>
      <anchorfile>sm_8h.html</anchorfile>
      <anchor>c494ef88899a23d2d386bdf3f127d6b5</anchor>
      <arglist></arglist>
    </member>
    <member kind="define">
      <type>#define</type>
      <name>SM_DELETED_DEF</name>
      <anchorfile>sm_8h.html</anchorfile>
      <anchor>7f99d732e140883d454a2b0ea0b5f927</anchor>
      <arglist></arglist>
    </member>
    <member kind="define">
      <type>#define</type>
      <name>SM_MAX_SLOT</name>
      <anchorfile>sm_8h.html</anchorfile>
      <anchor>24171cfb8faee5635d96d731e30b59f0</anchor>
      <arglist></arglist>
    </member>
    <member kind="define">
      <type>#define</type>
      <name>SM_MAX_CYCLE</name>
      <anchorfile>sm_8h.html</anchorfile>
      <anchor>e84ecd39117474b57748ff3565e7aa5c</anchor>
      <arglist></arglist>
    </member>
    <member kind="define">
      <type>#define</type>
      <name>SM_MIN_CYCLE</name>
      <anchorfile>sm_8h.html</anchorfile>
      <anchor>a876cebdda63a2bce1e558a87cae65c3</anchor>
      <arglist></arglist>
    </member>
    <member kind="define">
      <type>#define</type>
      <name>SM_NO_TEMPLATE</name>
      <anchorfile>sm_8h.html</anchorfile>
      <anchor>66899a845c94d2135e73007ca0bbecd7</anchor>
      <arglist></arglist>
    </member>
    <member kind="define">
      <type>#define</type>
      <name>S_AFTER_3RD</name>
      <anchorfile>sm_8h.html</anchorfile>
      <anchor>94f32bfe3af7e75ca33331334aab6eba</anchor>
      <arglist></arglist>
    </member>
    <member kind="define">
      <type>#define</type>
      <name>S_AFTER_2ND</name>
      <anchorfile>sm_8h.html</anchorfile>
      <anchor>6719d08f6a2cd1c9ef0e9aecd046aa2e</anchor>
      <arglist></arglist>
    </member>
    <member kind="define">
      <type>#define</type>
      <name>S_AFTER_1ST</name>
      <anchorfile>sm_8h.html</anchorfile>
      <anchor>136bde2670bedc18f63994ca32c2b361</anchor>
      <arglist></arglist>
    </member>
    <member kind="define">
      <type>#define</type>
      <name>NO_S_BIT</name>
      <anchorfile>sm_8h.html</anchorfile>
      <anchor>b0dc9fb4b6d67c08adab9b6525de6591</anchor>
      <arglist></arglist>
    </member>
    <member kind="define">
      <type>#define</type>
      <name>MII</name>
      <anchorfile>sm_8h.html</anchorfile>
      <anchor>46809378752cbd5e11e537b31a59d3da</anchor>
      <arglist></arglist>
    </member>
    <member kind="define">
      <type>#define</type>
      <name>MISI</name>
      <anchorfile>sm_8h.html</anchorfile>
      <anchor>17629e3901ed7a32ee2b57ce6ab9481e</anchor>
      <arglist></arglist>
    </member>
    <member kind="define">
      <type>#define</type>
      <name>MLI</name>
      <anchorfile>sm_8h.html</anchorfile>
      <anchor>d92003e2c25b3d14d5c4f6f663afaaac</anchor>
      <arglist></arglist>
    </member>
    <member kind="define">
      <type>#define</type>
      <name>RSVD_T1</name>
      <anchorfile>sm_8h.html</anchorfile>
      <anchor>ba0cd9fda7cf2579f6b9e7fdcb2b21f2</anchor>
      <arglist></arglist>
    </member>
    <member kind="define">
      <type>#define</type>
      <name>MMI</name>
      <anchorfile>sm_8h.html</anchorfile>
      <anchor>7beefba50058a040c3aee9f9140b8621</anchor>
      <arglist></arglist>
    </member>
    <member kind="define">
      <type>#define</type>
      <name>MSMI</name>
      <anchorfile>sm_8h.html</anchorfile>
      <anchor>cad5b250082c52a049bfbd44b16e956a</anchor>
      <arglist></arglist>
    </member>
    <member kind="define">
      <type>#define</type>
      <name>MFI</name>
      <anchorfile>sm_8h.html</anchorfile>
      <anchor>049408aca49f3369b5d29cf99f6c40e6</anchor>
      <arglist></arglist>
    </member>
    <member kind="define">
      <type>#define</type>
      <name>MMF</name>
      <anchorfile>sm_8h.html</anchorfile>
      <anchor>2ab77414482e02af7d393090be0b5c44</anchor>
      <arglist></arglist>
    </member>
    <member kind="define">
      <type>#define</type>
      <name>MIB</name>
      <anchorfile>sm_8h.html</anchorfile>
      <anchor>9d11258d2beb9cc8297f7c4eff586868</anchor>
      <arglist></arglist>
    </member>
    <member kind="define">
      <type>#define</type>
      <name>MBB</name>
      <anchorfile>sm_8h.html</anchorfile>
      <anchor>482713d1bbf911f6116016a8db72d920</anchor>
      <arglist></arglist>
    </member>
    <member kind="define">
      <type>#define</type>
      <name>RSVD_T3</name>
      <anchorfile>sm_8h.html</anchorfile>
      <anchor>c6541dd43889258d26f31f0022bc1960</anchor>
      <arglist></arglist>
    </member>
    <member kind="define">
      <type>#define</type>
      <name>BBB</name>
      <anchorfile>sm_8h.html</anchorfile>
      <anchor>56f71db70e5d3eb35f197bd1499987b1</anchor>
      <arglist></arglist>
    </member>
    <member kind="define">
      <type>#define</type>
      <name>MMB</name>
      <anchorfile>sm_8h.html</anchorfile>
      <anchor>48e7c1e96778d2b311ee4521ae9ad3f9</anchor>
      <arglist></arglist>
    </member>
    <member kind="define">
      <type>#define</type>
      <name>RSVD_T4</name>
      <anchorfile>sm_8h.html</anchorfile>
      <anchor>f30e7ecef96cbd8dc3da60bf6b01618b</anchor>
      <arglist></arglist>
    </member>
    <member kind="define">
      <type>#define</type>
      <name>MFB</name>
      <anchorfile>sm_8h.html</anchorfile>
      <anchor>809ec1d69f62594a702aa2376eee8a73</anchor>
      <arglist></arglist>
    </member>
    <member kind="define">
      <type>#define</type>
      <name>RSVD_T5</name>
      <anchorfile>sm_8h.html</anchorfile>
      <anchor>8f63f02a74bd32ba4747b7deef82dfeb</anchor>
      <arglist></arglist>
    </member>
    <member kind="define">
      <type>#define</type>
      <name>SM_is_reg</name>
      <anchorfile>sm_8h.html</anchorfile>
      <anchor>e3c02685ad05077d309d3f818c8b9dc4</anchor>
      <arglist>(operand)</arglist>
    </member>
    <member kind="define">
      <type>#define</type>
      <name>SM_get_num_slots</name>
      <anchorfile>sm_8h.html</anchorfile>
      <anchor>7a7e8e91f91d8c17679800eeea340ef2</anchor>
      <arglist>(sm_cb)</arglist>
    </member>
    <member kind="define">
      <type>#define</type>
      <name>SM_get_slots_per_template</name>
      <anchorfile>sm_8h.html</anchorfile>
      <anchor>d91b3f5dc9c24fd5700654f147c914b3</anchor>
      <arglist>(sm_cb)</arglist>
    </member>
    <member kind="define">
      <type>#define</type>
      <name>SM_is_nop</name>
      <anchorfile>sm_8h.html</anchorfile>
      <anchor>78b36d53c5543e87a2d8eae26db98944</anchor>
      <arglist>(sm_op)</arglist>
    </member>
    <member kind="define">
      <type>#define</type>
      <name>SM_get_num_ports</name>
      <anchorfile>sm_8h.html</anchorfile>
      <anchor>a5b9441278980cf14fc26409af6d6e65</anchor>
      <arglist>(sm_cb)</arglist>
    </member>
    <member kind="define">
      <type>#define</type>
      <name>SM_get_template_shift_var</name>
      <anchorfile>sm_8h.html</anchorfile>
      <anchor>0d7b3b436facb059bde07e088a85081b</anchor>
      <arglist>(sm_cb)</arglist>
    </member>
    <member kind="define">
      <type>#define</type>
      <name>SM_get_template</name>
      <anchorfile>sm_8h.html</anchorfile>
      <anchor>f9cdf4898f28fc3f01cce50489228c30</anchor>
      <arglist>(sm_cb, template_index)</arglist>
    </member>
    <member kind="define">
      <type>#define</type>
      <name>SM_get_template_per_issue</name>
      <anchorfile>sm_8h.html</anchorfile>
      <anchor>967414530eae626981fd2ec01012ce59</anchor>
      <arglist>(sm_cb)</arglist>
    </member>
    <member kind="define">
      <type>#define</type>
      <name>SM_get_num_restricts</name>
      <anchorfile>sm_8h.html</anchorfile>
      <anchor>9b52babbfdc0c2663b60aa9a855059e3</anchor>
      <arglist>(sm_cb)</arglist>
    </member>
    <member kind="define">
      <type>#define</type>
      <name>SM_get_restrict_num</name>
      <anchorfile>sm_8h.html</anchorfile>
      <anchor>7408a2c6679f3ef7ffa32a3fbe174e3a</anchor>
      <arglist>(sm_cb, index)</arglist>
    </member>
    <member kind="define">
      <type>#define</type>
      <name>SM_get_restrict_mask</name>
      <anchorfile>sm_8h.html</anchorfile>
      <anchor>c83eabd9248b25161bf110d5c104d8d8</anchor>
      <arglist>(sm_cb, index)</arglist>
    </member>
    <member kind="define">
      <type>#define</type>
      <name>SM_get_drule</name>
      <anchorfile>sm_8h.html</anchorfile>
      <anchor>f81df58eda7ef9876b952b9c4019ffa6</anchor>
      <arglist>(sm_cb, index)</arglist>
    </member>
    <member kind="define">
      <type>#define</type>
      <name>SM_get_drsrc</name>
      <anchorfile>sm_8h.html</anchorfile>
      <anchor>ecd1d5a1bbfdc3903842799a2f6bf75a</anchor>
      <arglist>(drule, drsrc_index)</arglist>
    </member>
    <member kind="define">
      <type>#define</type>
      <name>M_new_template</name>
      <anchorfile>sm_8h.html</anchorfile>
      <anchor>6f97056dfb8e3fcc6180199ebe353e2d</anchor>
      <arglist>(tmpl_op, tmpl_type)</arglist>
    </member>
    <member kind="define">
      <type>#define</type>
      <name>M_new_stop_bit_mask</name>
      <anchorfile>sm_8h.html</anchorfile>
      <anchor>9c0b19b9aab6b1d71c57a6ffa8f3dfef</anchor>
      <arglist>(tmpl_op, stop_mask)</arglist>
    </member>
    <member kind="define">
      <type>#define</type>
      <name>M_is_template_op</name>
      <anchorfile>sm_8h.html</anchorfile>
      <anchor>724d2ecec2dceac28b854fdc65fcd16f</anchor>
      <arglist>(op)</arglist>
    </member>
    <member kind="function">
      <type>void</type>
      <name>SM_init</name>
      <anchorfile>sm_8h.html</anchorfile>
      <anchor>777f79731c44d45d9946fac96cc44b67</anchor>
      <arglist>(Parm_Macro_List *command_line_macro_list)</arglist>
    </member>
    <member kind="function">
      <type>void</type>
      <name>SM_schedule_fn</name>
      <anchorfile>sm_8h.html</anchorfile>
      <anchor>d4aaadc4c5ec107db5684d78c9313a63</anchor>
      <arglist>(L_Func *fn, Mdes *version1_mdes, int prepass_sched)</arglist>
    </member>
    <member kind="function">
      <type>void</type>
      <name>SM_bundle_fn</name>
      <anchorfile>sm_8h.html</anchorfile>
      <anchor>dd48e0dd0e99613c86b1d6a38fe09d1f</anchor>
      <arglist>(L_Func *fn, Mdes *version1_mdes)</arglist>
    </member>
    <member kind="function">
      <type>SM_Oper *</type>
      <name>SM_find_sm_op</name>
      <anchorfile>sm_8h.html</anchorfile>
      <anchor>8a87e702f9b4517fc0791d11d8a20438</anchor>
      <arglist>(SM_Cb *sm_cb, L_Oper *l_oper)</arglist>
    </member>
    <member kind="function">
      <type>SM_Cb *</type>
      <name>SM_new_cb</name>
      <anchorfile>sm_8h.html</anchorfile>
      <anchor>a3abf3f985c8090e785d3ed290c9c7d4</anchor>
      <arglist>(Mdes *version1_mdes, L_Cb *lcode_cb, int cb_flags)</arglist>
    </member>
    <member kind="function">
      <type>void</type>
      <name>SM_set_cb_II</name>
      <anchorfile>sm_8h.html</anchorfile>
      <anchor>ea90a092c9a21248f1d58f320e5c2856</anchor>
      <arglist>(SM_Cb *sm_cb, int II)</arglist>
    </member>
    <member kind="function">
      <type>void</type>
      <name>SM_schedule_cb</name>
      <anchorfile>sm_8h.html</anchorfile>
      <anchor>3fc49b883e223850ea59ce59aa1c3cf3</anchor>
      <arglist>(SM_Cb *sm_cb)</arglist>
    </member>
    <member kind="function">
      <type>void</type>
      <name>SM_unschedule_cb</name>
      <anchorfile>sm_8h.html</anchorfile>
      <anchor>31ddcafff5a15dd2d9a9479f6ece561a</anchor>
      <arglist>(SM_Cb *sm_cb)</arglist>
    </member>
    <member kind="function">
      <type>double</type>
      <name>SM_calc_best_case_cycles</name>
      <anchorfile>sm_8h.html</anchorfile>
      <anchor>63e79399198766f2c34c99309dcfb829</anchor>
      <arglist>(SM_Cb *sm_cb)</arglist>
    </member>
    <member kind="function">
      <type>L_Attr *</type>
      <name>SM_attach_isl</name>
      <anchorfile>sm_8h.html</anchorfile>
      <anchor>4b07b52de68d6ffbaa5073b740d4b411</anchor>
      <arglist>(SM_Oper *sm_op, int sched_cycle_offset)</arglist>
    </member>
    <member kind="function">
      <type>int</type>
      <name>SM_set_sched_cycle_offset</name>
      <anchorfile>sm_8h.html</anchorfile>
      <anchor>3717231690ad643569cefb30c7b678a0</anchor>
      <arglist>(SM_Cb *sm_cb)</arglist>
    </member>
    <member kind="function">
      <type>void</type>
      <name>SM_commit_cb</name>
      <anchorfile>sm_8h.html</anchorfile>
      <anchor>e52798e66e79a170b5bb050ad226bc42</anchor>
      <arglist>(SM_Cb *sm_cb)</arglist>
    </member>
    <member kind="function">
      <type>void</type>
      <name>SM_print_cb_schedule</name>
      <anchorfile>sm_8h.html</anchorfile>
      <anchor>3179b40d974163706535d488d249b8b6</anchor>
      <arglist>(SM_Cb *sm_cb)</arglist>
    </member>
    <member kind="function">
      <type>void</type>
      <name>SM_delete_cb</name>
      <anchorfile>sm_8h.html</anchorfile>
      <anchor>48459436e1be4f10529483a6c5f6770c</anchor>
      <arglist>(SM_Cb *sm_cb)</arglist>
    </member>
    <member kind="function">
      <type>void</type>
      <name>SM_construct_temp_kernel_queue</name>
      <anchorfile>sm_8h.html</anchorfile>
      <anchor>2ac7c1af255f6338383d14a62ab6677a</anchor>
      <arglist>(SM_Cb *sm_cb)</arglist>
    </member>
    <member kind="function">
      <type>SM_Oper *</type>
      <name>SM_insert_oper_after</name>
      <anchorfile>sm_8h.html</anchorfile>
      <anchor>15c481b9c7cd129ace579e1e290a4385</anchor>
      <arglist>(SM_Cb *sm_cb, L_Oper *lcode_op, SM_Oper *after_sm_op)</arglist>
    </member>
    <member kind="function">
      <type>void</type>
      <name>SM_move_oper_after</name>
      <anchorfile>sm_8h.html</anchorfile>
      <anchor>c5d14bd37ad526b73b73b21d4a241418</anchor>
      <arglist>(SM_Oper *move_sm_op, SM_Oper *after_sm_op)</arglist>
    </member>
    <member kind="function">
      <type>void</type>
      <name>SM_delete_oper</name>
      <anchorfile>sm_8h.html</anchorfile>
      <anchor>0883cb5cc83db52dddea8ba365807322</anchor>
      <arglist>(SM_Oper *sm_op)</arglist>
    </member>
    <member kind="function">
      <type>void</type>
      <name>SM_ignore_op</name>
      <anchorfile>sm_8h.html</anchorfile>
      <anchor>e7bcad85068702dc233c4e5a6a9e5fbe</anchor>
      <arglist>(SM_Oper *, int)</arglist>
    </member>
    <member kind="function">
      <type>void</type>
      <name>SM_enable_ignored_op</name>
      <anchorfile>sm_8h.html</anchorfile>
      <anchor>cd3dc74626447cbcbee1db6092cec803</anchor>
      <arglist>(SM_Oper *, int)</arglist>
    </member>
    <member kind="function">
      <type>int</type>
      <name>SM_sched_table</name>
      <anchorfile>sm_8h.html</anchorfile>
      <anchor>27d124d41133b2ed180e083df33b951d</anchor>
      <arglist>(SM_Cb *sm_cb, SM_Oper *sm_op, unsigned int template_index, SM_Table *table, unsigned short *choice_array, int time, unsigned short min_slot, unsigned short max_slot, int commit, Mdes_Stats *stats)</arglist>
    </member>
    <member kind="function">
      <type>void</type>
      <name>SM_unsched_table</name>
      <anchorfile>sm_8h.html</anchorfile>
      <anchor>f5797ef9c78983a02c76cc82991cc223</anchor>
      <arglist>(SM_Cb *sm_cb, SM_Table *table, unsigned short *choice_array, int time)</arglist>
    </member>
    <member kind="function">
      <type>void</type>
      <name>SM_print_option</name>
      <anchorfile>sm_8h.html</anchorfile>
      <anchor>f468b1263fd5ba32863a0b67333ebd0a</anchor>
      <arglist>(FILE *out, SM_Mdes *sm_mdes, SM_Option *option)</arglist>
    </member>
    <member kind="function">
      <type>void</type>
      <name>SM_print_map</name>
      <anchorfile>sm_8h.html</anchorfile>
      <anchor>a83bf74e00d42409c45844d6694eca13</anchor>
      <arglist>(FILE *out, SM_Mdes *sm_mdes, SM_Cb *sm_cb)</arglist>
    </member>
    <member kind="function">
      <type>void</type>
      <name>SM_free_alloc_pools</name>
      <anchorfile>sm_8h.html</anchorfile>
      <anchor>8054846f9bd9554214386acbccab6f77</anchor>
      <arglist>()</arglist>
    </member>
    <member kind="function">
      <type>void</type>
      <name>SM_build_reg_info_table</name>
      <anchorfile>sm_8h.html</anchorfile>
      <anchor>44c7539377ce68066474efd8463e36dc</anchor>
      <arglist>(SM_Cb *sm_cb)</arglist>
    </member>
    <member kind="function">
      <type>void</type>
      <name>SM_free_reg_info_table</name>
      <anchorfile>sm_8h.html</anchorfile>
      <anchor>3400bae118e95434618b944ea2f7c08b</anchor>
      <arglist>(SM_Cb *sm_cb)</arglist>
    </member>
    <member kind="function">
      <type>SM_Reg_Info *</type>
      <name>SM_add_reg_info</name>
      <anchorfile>sm_8h.html</anchorfile>
      <anchor>5e88f05bd7ce34e7d3a6e40cecca607c</anchor>
      <arglist>(SM_Cb *sm_cb, L_Operand *reg_operand)</arglist>
    </member>
    <member kind="function">
      <type>SM_Reg_Info *</type>
      <name>SM_find_reg_info</name>
      <anchorfile>sm_8h.html</anchorfile>
      <anchor>4b8be6e1608e9be1626020b88d76db78</anchor>
      <arglist>(SM_Cb *sm_cb, L_Operand *reg_operand)</arglist>
    </member>
    <member kind="function">
      <type>SM_Reg_Action *</type>
      <name>SM_add_reg_action</name>
      <anchorfile>sm_8h.html</anchorfile>
      <anchor>ccb8edcd77ee94f72aa7c5fa774f2a0f</anchor>
      <arglist>(SM_Oper *sm_op, int operand_type, int operand_number, L_Operand *reg_operand)</arglist>
    </member>
    <member kind="function">
      <type>void</type>
      <name>SM_add_reg_actions_for_op</name>
      <anchorfile>sm_8h.html</anchorfile>
      <anchor>4f0f22f48ad3638fc4569b375e783e6e</anchor>
      <arglist>(SM_Oper *sm_op)</arglist>
    </member>
    <member kind="function">
      <type>void</type>
      <name>SM_delete_reg_action</name>
      <anchorfile>sm_8h.html</anchorfile>
      <anchor>45494cda0637597b8fa52b6a1f81af19</anchor>
      <arglist>(SM_Reg_Action *action)</arglist>
    </member>
    <member kind="function">
      <type>void</type>
      <name>SM_reposition_reg_action</name>
      <anchorfile>sm_8h.html</anchorfile>
      <anchor>bf79394ddba4abfb82b6eada74de3ffd</anchor>
      <arglist>(SM_Reg_Action *action)</arglist>
    </member>
    <member kind="function">
      <type>void</type>
      <name>SM_build_oper_dependences</name>
      <anchorfile>sm_8h.html</anchorfile>
      <anchor>ca4ceaf192b3e9be6abc7c6b0cd6520f</anchor>
      <arglist>(SM_Cb *sm_cb, SM_Oper *sm_op, unsigned int flags)</arglist>
    </member>
    <member kind="function">
      <type>void</type>
      <name>SM_build_cb_dependences</name>
      <anchorfile>sm_8h.html</anchorfile>
      <anchor>32828cc8c9780473ae2c45d8ac94e76e</anchor>
      <arglist>(SM_Cb *sm_cb)</arglist>
    </member>
    <member kind="function">
      <type>void</type>
      <name>SM_print_oper_dependences</name>
      <anchorfile>sm_8h.html</anchorfile>
      <anchor>5d03fe869330cd68884e1b6cec012d0b</anchor>
      <arglist>(FILE *out, SM_Oper *sm_op)</arglist>
    </member>
    <member kind="function">
      <type>void</type>
      <name>SM_print_cb_dependences</name>
      <anchorfile>sm_8h.html</anchorfile>
      <anchor>3146d8578acda2cab53688b0ed0354dc</anchor>
      <arglist>(FILE *out, SM_Cb *sm_cb)</arglist>
    </member>
    <member kind="function">
      <type>void</type>
      <name>SM_print_oper</name>
      <anchorfile>sm_8h.html</anchorfile>
      <anchor>2f11330eb01d12e8df1774421c308f27</anchor>
      <arglist>(FILE *out, SM_Oper *sm_op)</arglist>
    </member>
    <member kind="function">
      <type>void</type>
      <name>SM_print_dep</name>
      <anchorfile>sm_8h.html</anchorfile>
      <anchor>ff31fcd5849b6cad5018b4ccfb101c55</anchor>
      <arglist>(FILE *out, SM_Dep *dep)</arglist>
    </member>
    <member kind="function">
      <type>void</type>
      <name>SM_print_action_id</name>
      <anchorfile>sm_8h.html</anchorfile>
      <anchor>fafed09ceccb2e9e8b04e6417882f307</anchor>
      <arglist>(FILE *out, SM_Reg_Action *action)</arglist>
    </member>
    <member kind="function">
      <type>void</type>
      <name>SM_add_dep</name>
      <anchorfile>sm_8h.html</anchorfile>
      <anchor>c0ba8409ca8be6d55eda19c7122a3c14</anchor>
      <arglist>(SM_Reg_Action *from_action, SM_Reg_Action *to_action, unsigned int flags, unsigned int ignore, short delay_offset, short omega, unsigned int mode)</arglist>
    </member>
    <member kind="function">
      <type>void</type>
      <name>SM_insert_sync_dep_betw_ops</name>
      <anchorfile>sm_8h.html</anchorfile>
      <anchor>0030e13c6f63ea25980021040849f95f</anchor>
      <arglist>(SM_Oper *from_op, SM_Oper *to_op)</arglist>
    </member>
    <member kind="function">
      <type>void</type>
      <name>SM_build_src_reg_deps</name>
      <anchorfile>sm_8h.html</anchorfile>
      <anchor>7cb00c843c6585c4884a0db5a546860b</anchor>
      <arglist>(SM_Reg_Action *src_action, unsigned int flags)</arglist>
    </member>
    <member kind="function">
      <type>void</type>
      <name>SM_build_dest_reg_deps</name>
      <anchorfile>sm_8h.html</anchorfile>
      <anchor>5ded743fdf32f8ee86ae033c45264c79</anchor>
      <arglist>(SM_Reg_Action *dest_action, unsigned int flags)</arglist>
    </member>
    <member kind="function">
      <type>void</type>
      <name>SM_rebuild_dest_reg_deps</name>
      <anchorfile>sm_8h.html</anchorfile>
      <anchor>4ef2c5a485745481b74997f1ce3187c8</anchor>
      <arglist>(SM_Reg_Action *dest_action, unsigned int flags)</arglist>
    </member>
    <member kind="function">
      <type>void</type>
      <name>SM_build_src_mem_deps</name>
      <anchorfile>sm_8h.html</anchorfile>
      <anchor>3a27b3890cf76120ed386c20725ad8e5</anchor>
      <arglist>(SM_Reg_Action *src_action, unsigned int flags)</arglist>
    </member>
    <member kind="function">
      <type>void</type>
      <name>SM_build_dest_mem_deps</name>
      <anchorfile>sm_8h.html</anchorfile>
      <anchor>8f6b3db14ed4a5913e6086bb30e2268b</anchor>
      <arglist>(SM_Reg_Action *dest_action, unsigned int flags)</arglist>
    </member>
    <member kind="function">
      <type>void</type>
      <name>SM_build_src_ctrl_deps</name>
      <anchorfile>sm_8h.html</anchorfile>
      <anchor>3d2f9302524a17842be1b37a7bba35e6</anchor>
      <arglist>(SM_Cb *sm_cb, SM_Reg_Action *src_action, unsigned int flags)</arglist>
    </member>
    <member kind="function">
      <type>void</type>
      <name>SM_build_dest_ctrl_deps</name>
      <anchorfile>sm_8h.html</anchorfile>
      <anchor>529ab90c2d95bab6465c4ba5c5022714</anchor>
      <arglist>(SM_Cb *sm_cb, SM_Reg_Action *dest_action, unsigned int flags)</arglist>
    </member>
    <member kind="function">
      <type>void</type>
      <name>SM_build_src_sync_deps</name>
      <anchorfile>sm_8h.html</anchorfile>
      <anchor>6f787af080a3c4598a459e25a33ff5a3</anchor>
      <arglist>(SM_Reg_Action *src_action, unsigned int flags)</arglist>
    </member>
    <member kind="function">
      <type>void</type>
      <name>SM_build_dest_sync_deps</name>
      <anchorfile>sm_8h.html</anchorfile>
      <anchor>9a500402c6936eb2b5c962f063f652a7</anchor>
      <arglist>(SM_Reg_Action *dest_action, unsigned int flags)</arglist>
    </member>
    <member kind="function">
      <type>void</type>
      <name>SM_delete_dep</name>
      <anchorfile>sm_8h.html</anchorfile>
      <anchor>ac2fdd3298e3b911c3afca217b70ef75</anchor>
      <arglist>(SM_Dep *dep)</arglist>
    </member>
    <member kind="function">
      <type>void</type>
      <name>SM_ignore_dep</name>
      <anchorfile>sm_8h.html</anchorfile>
      <anchor>788482acf72fd688418e5d4560967a4a</anchor>
      <arglist>(SM_Dep *dep, int flag)</arglist>
    </member>
    <member kind="function">
      <type>void</type>
      <name>SM_enable_ignored_dep</name>
      <anchorfile>sm_8h.html</anchorfile>
      <anchor>9a522675eb1ff43978448d4a2c749277</anchor>
      <arglist>(SM_Dep *dep, int flag)</arglist>
    </member>
    <member kind="function">
      <type>void</type>
      <name>SM_ignore_dep_out</name>
      <anchorfile>sm_8h.html</anchorfile>
      <anchor>44142e3eb60a8617f5e88b2498e642d7</anchor>
      <arglist>(SM_Oper *sm_op, int flag)</arglist>
    </member>
    <member kind="function">
      <type>void</type>
      <name>SM_enable_ignored_dep_out</name>
      <anchorfile>sm_8h.html</anchorfile>
      <anchor>263290420f908b8afcb5c783ef65b1ac</anchor>
      <arglist>(SM_Oper *sm_op, int flag)</arglist>
    </member>
    <member kind="function">
      <type>void</type>
      <name>SM_ignore_dep_in</name>
      <anchorfile>sm_8h.html</anchorfile>
      <anchor>8e3744ccc23eee9c8b5af6b81555e459</anchor>
      <arglist>(SM_Oper *sm_op, int flag)</arglist>
    </member>
    <member kind="function">
      <type>void</type>
      <name>SM_enable_ignored_dep_in</name>
      <anchorfile>sm_8h.html</anchorfile>
      <anchor>358ba5a8828e39c9c2a19c1e99e7926e</anchor>
      <arglist>(SM_Oper *sm_op, int flag)</arglist>
    </member>
    <member kind="function">
      <type>int</type>
      <name>SM_def_post_dominates_action</name>
      <anchorfile>sm_8h.html</anchorfile>
      <anchor>a2f28155679ad09f0bc57b1542bad633</anchor>
      <arglist>(SM_Reg_Action *def_after, SM_Reg_Action *action)</arglist>
    </member>
    <member kind="function">
      <type>void</type>
      <name>SM_print_action_operand</name>
      <anchorfile>sm_8h.html</anchorfile>
      <anchor>eacae75507fbe3eb5ae13f934fac24af</anchor>
      <arglist>(FILE *out, SM_Reg_Action *action)</arglist>
    </member>
    <member kind="function">
      <type>void</type>
      <name>SM_print_reg_action</name>
      <anchorfile>sm_8h.html</anchorfile>
      <anchor>e77ff2e15505eab82fd52ce08dca0803</anchor>
      <arglist>(FILE *out, SM_Reg_Action *action)</arglist>
    </member>
    <member kind="function">
      <type>void</type>
      <name>SM_print_reg_info</name>
      <anchorfile>sm_8h.html</anchorfile>
      <anchor>2dad8a9222f55933eac028f8be311d59</anchor>
      <arglist>(FILE *out, SM_Reg_Info *rinfo)</arglist>
    </member>
    <member kind="function">
      <type>void</type>
      <name>SM_print_reg_info_operand</name>
      <anchorfile>sm_8h.html</anchorfile>
      <anchor>9f74b8d2905f61d10740dc5696ad7d27</anchor>
      <arglist>(FILE *out, SM_Reg_Info *rinfo)</arglist>
    </member>
    <member kind="function">
      <type>void</type>
      <name>SM_print_reg_info_table</name>
      <anchorfile>sm_8h.html</anchorfile>
      <anchor>397ff832d19fc2dc2e2239446e32179d</anchor>
      <arglist>(FILE *out, SM_Cb *sm_cb)</arglist>
    </member>
    <member kind="function">
      <type>void</type>
      <name>SM_print_sorted_reg_info_table</name>
      <anchorfile>sm_8h.html</anchorfile>
      <anchor>292431d5e5f90d68f6bd40b47bdbce57</anchor>
      <arglist>(FILE *out, SM_Cb *sm_cb)</arglist>
    </member>
    <member kind="function">
      <type>void</type>
      <name>SM_calculate_priorities</name>
      <anchorfile>sm_8h.html</anchorfile>
      <anchor>130cf52b2f5ee837159fb59182b1baaa</anchor>
      <arglist>(SM_Cb *sm_cb, int min_early_time)</arglist>
    </member>
    <member kind="function">
      <type>int</type>
      <name>SM_calculate_early_times</name>
      <anchorfile>sm_8h.html</anchorfile>
      <anchor>99799486dc342e24126a6d7c316edf0b</anchor>
      <arglist>(SM_Cb *sm_cb, int min_early_time)</arglist>
    </member>
    <member kind="function">
      <type>void</type>
      <name>SM_calculate_late_times</name>
      <anchorfile>sm_8h.html</anchorfile>
      <anchor>c8b7ef12dd37191f740b481a02016148</anchor>
      <arglist>(SM_Cb *sm_cb, int max_late_time)</arglist>
    </member>
    <member kind="function">
      <type>void</type>
      <name>SM_print_early_and_late_times</name>
      <anchorfile>sm_8h.html</anchorfile>
      <anchor>6e36dac40de39efc4957c22f0aafcbc9</anchor>
      <arglist>(FILE *out, SM_Cb *sm_cb)</arglist>
    </member>
    <member kind="function">
      <type>int</type>
      <name>SM_recalculate_early_time</name>
      <anchorfile>sm_8h.html</anchorfile>
      <anchor>b0691af602f049d3165a9dc22435d31f</anchor>
      <arglist>(SM_Oper *sm_op, int min_early_time)</arglist>
    </member>
    <member kind="function">
      <type>void</type>
      <name>SM_recalculate_lower_bound</name>
      <anchorfile>sm_8h.html</anchorfile>
      <anchor>9e79ecff076013ba3bfb73551a60b04d</anchor>
      <arglist>(SM_Oper *sm_op)</arglist>
    </member>
    <member kind="function">
      <type>void</type>
      <name>SM_recalculate_upper_bound</name>
      <anchorfile>sm_8h.html</anchorfile>
      <anchor>9033bfb17d8f1e7f744961567df9695b</anchor>
      <arglist>(SM_Oper *sm_op)</arglist>
    </member>
    <member kind="function">
      <type>int</type>
      <name>SM_schedule_oper</name>
      <anchorfile>sm_8h.html</anchorfile>
      <anchor>602f5c3eff20faea9a4949f2273feafb</anchor>
      <arglist>(SM_Oper *sm_op, int issue_time, int earliest_slot, int latest_slot, unsigned int sched_flags)</arglist>
    </member>
    <member kind="function">
      <type>void</type>
      <name>SM_unschedule_oper</name>
      <anchorfile>sm_8h.html</anchorfile>
      <anchor>75f1913ac8d175b9dfd6ce15fd27ffc9</anchor>
      <arglist>(SM_Oper *sm_op, SM_Priority_Queue *priority_queue)</arglist>
    </member>
    <member kind="function">
      <type>SM_Oper_Queue *</type>
      <name>SM_new_oper_queue</name>
      <anchorfile>sm_8h.html</anchorfile>
      <anchor>f72bd28d77797baf55f2e6b0af3a8456</anchor>
      <arglist>()</arglist>
    </member>
    <member kind="function">
      <type>void</type>
      <name>SM_delete_oper_queue</name>
      <anchorfile>sm_8h.html</anchorfile>
      <anchor>0b8d645214a07f21c6c97ce066eec6b5</anchor>
      <arglist>(SM_Oper_Queue *queue)</arglist>
    </member>
    <member kind="function">
      <type>SM_Oper_Qentry *</type>
      <name>SM_enqueue_oper_before</name>
      <anchorfile>sm_8h.html</anchorfile>
      <anchor>80620884311427f1817ee1b8f9c9a4c4</anchor>
      <arglist>(SM_Oper_Queue *queue, SM_Oper *sm_op, SM_Oper_Qentry *before_qentry)</arglist>
    </member>
    <member kind="function">
      <type>SM_Oper_Qentry *</type>
      <name>SM_enqueue_oper_after</name>
      <anchorfile>sm_8h.html</anchorfile>
      <anchor>2c5f0266ce44105027317e9d3d08102d</anchor>
      <arglist>(SM_Oper_Queue *queue, SM_Oper *sm_op, SM_Oper_Qentry *after_qentry)</arglist>
    </member>
    <member kind="function">
      <type>void</type>
      <name>SM_dequeue_oper</name>
      <anchorfile>sm_8h.html</anchorfile>
      <anchor>181d88a02e26e6a27dfd9b4a64cfefe0</anchor>
      <arglist>(SM_Oper_Qentry *qentry)</arglist>
    </member>
    <member kind="function">
      <type>void</type>
      <name>SM_dequeue_oper_from_all</name>
      <anchorfile>sm_8h.html</anchorfile>
      <anchor>185ebdb2a0ee196bf327cca3ac4725ce</anchor>
      <arglist>(SM_Oper *sm_op)</arglist>
    </member>
    <member kind="function">
      <type>void</type>
      <name>SM_print_oper_queue</name>
      <anchorfile>sm_8h.html</anchorfile>
      <anchor>330894fc4899ed745e8047e6586d13d8</anchor>
      <arglist>(FILE *out, SM_Oper_Queue *queue)</arglist>
    </member>
    <member kind="function">
      <type>SM_Action_Queue *</type>
      <name>SM_new_action_queue</name>
      <anchorfile>sm_8h.html</anchorfile>
      <anchor>4690c78915658abbb7a0977a668449a4</anchor>
      <arglist>()</arglist>
    </member>
    <member kind="function">
      <type>void</type>
      <name>SM_delete_action_queue</name>
      <anchorfile>sm_8h.html</anchorfile>
      <anchor>053a8c194a1a77f544a4bf8e535c25a8</anchor>
      <arglist>(SM_Action_Queue *queue)</arglist>
    </member>
    <member kind="function">
      <type>SM_Action_Qentry *</type>
      <name>SM_enqueue_action_before</name>
      <anchorfile>sm_8h.html</anchorfile>
      <anchor>8644246b75bb40b05e01e7bd1910b9d0</anchor>
      <arglist>(SM_Action_Queue *queue, SM_Reg_Action *action, SM_Action_Qentry *before_qentry)</arglist>
    </member>
    <member kind="function">
      <type>SM_Action_Qentry *</type>
      <name>SM_enqueue_action_after</name>
      <anchorfile>sm_8h.html</anchorfile>
      <anchor>b4c92a7098e53640e5e5c61b979986e5</anchor>
      <arglist>(SM_Action_Queue *queue, SM_Reg_Action *action, SM_Action_Qentry *after_qentry)</arglist>
    </member>
    <member kind="function">
      <type>void</type>
      <name>SM_dequeue_action</name>
      <anchorfile>sm_8h.html</anchorfile>
      <anchor>585b3321bd29443a433c054f40694be1</anchor>
      <arglist>(SM_Action_Qentry *qentry)</arglist>
    </member>
    <member kind="function">
      <type>void</type>
      <name>SM_dequeue_action_from_all</name>
      <anchorfile>sm_8h.html</anchorfile>
      <anchor>8025903102084a8a65ec619841a83f48</anchor>
      <arglist>(SM_Reg_Action *action)</arglist>
    </member>
    <member kind="function">
      <type>void</type>
      <name>SM_print_action_queue</name>
      <anchorfile>sm_8h.html</anchorfile>
      <anchor>a8294d51b74b62b933e89def637c61a2</anchor>
      <arglist>(FILE *out, SM_Action_Queue *queue)</arglist>
    </member>
    <member kind="function">
      <type>SM_Trans_Queue *</type>
      <name>SM_new_trans_queue</name>
      <anchorfile>sm_8h.html</anchorfile>
      <anchor>dbb9a3fff6b69ef787e1c3219d8ec572</anchor>
      <arglist>(char *name)</arglist>
    </member>
    <member kind="function">
      <type>void</type>
      <name>SM_delete_trans_queue</name>
      <anchorfile>sm_8h.html</anchorfile>
      <anchor>a0d62219e0aabccc40cfcd439882e0e2</anchor>
      <arglist>(SM_Trans_Queue *queue)</arglist>
    </member>
    <member kind="function">
      <type>SM_Trans_Qentry *</type>
      <name>SM_enqueue_trans_before</name>
      <anchorfile>sm_8h.html</anchorfile>
      <anchor>2e6e0b6b9a944234a75e58ebcba86fd6</anchor>
      <arglist>(SM_Trans_Queue *queue, SM_Trans *trans, SM_Trans_Qentry *before_qentry)</arglist>
    </member>
    <member kind="function">
      <type>SM_Trans_Qentry *</type>
      <name>SM_enqueue_trans_after</name>
      <anchorfile>sm_8h.html</anchorfile>
      <anchor>d5e38b215817c0dc12980ab8262f9cc5</anchor>
      <arglist>(SM_Trans_Queue *queue, SM_Trans *trans, SM_Trans_Qentry *after_qentry)</arglist>
    </member>
    <member kind="function">
      <type>void</type>
      <name>SM_dequeue_trans</name>
      <anchorfile>sm_8h.html</anchorfile>
      <anchor>41ad842f52933206658da8dc572879f6</anchor>
      <arglist>(SM_Trans_Qentry *qentry)</arglist>
    </member>
    <member kind="function">
      <type>void</type>
      <name>SM_dequeue_trans_from_all</name>
      <anchorfile>sm_8h.html</anchorfile>
      <anchor>2e71bd06304b51f1b9c1a05a8a64d31d</anchor>
      <arglist>(SM_Trans *trans)</arglist>
    </member>
    <member kind="function">
      <type>void</type>
      <name>SM_print_trans_queue</name>
      <anchorfile>sm_8h.html</anchorfile>
      <anchor>9efbab28fa13249f30565a57c4dbabad</anchor>
      <arglist>(FILE *out, SM_Trans_Queue *queue)</arglist>
    </member>
    <member kind="function">
      <type>void</type>
      <name>SM_change_operand</name>
      <anchorfile>sm_8h.html</anchorfile>
      <anchor>6adc15973135f090ce8559baf51c028f</anchor>
      <arglist>(SM_Oper *sm_op, int operand_type, int operand_number, L_Operand *new_operand)</arglist>
    </member>
    <member kind="function">
      <type>void</type>
      <name>SM_set_ext</name>
      <anchorfile>sm_8h.html</anchorfile>
      <anchor>0701d1d7a7ab77287c599f0f6bdd4836</anchor>
      <arglist>(SM_Oper *sm_op, ITintmax ext)</arglist>
    </member>
    <member kind="function">
      <type>ITintmax</type>
      <name>SM_get_ext</name>
      <anchorfile>sm_8h.html</anchorfile>
      <anchor>bb61423e9a1ffb10bd49eacd77e30fec</anchor>
      <arglist>(SM_Oper *sm_op)</arglist>
    </member>
    <member kind="function">
      <type>void</type>
      <name>SM_do_relocate_cond_opti</name>
      <anchorfile>sm_8h.html</anchorfile>
      <anchor>449a6d51faabc75002d5fda7c50d6427</anchor>
      <arglist>(SM_Cb *sm_cb)</arglist>
    </member>
    <member kind="function">
      <type>SM_Trans *</type>
      <name>SM_new_trans</name>
      <anchorfile>sm_8h.html</anchorfile>
      <anchor>b5e0db0d75fbeb826aa73a58e9d860a8</anchor>
      <arglist>(int type, SM_Oper *target_sm_op, int flags)</arglist>
    </member>
    <member kind="function">
      <type>void</type>
      <name>SM_delete_trans</name>
      <anchorfile>sm_8h.html</anchorfile>
      <anchor>e40c03f92b614356e6f3e4a8683bef42</anchor>
      <arglist>(SM_Trans *sm_trans)</arglist>
    </member>
    <member kind="function">
      <type>void</type>
      <name>SM_print_trans</name>
      <anchorfile>sm_8h.html</anchorfile>
      <anchor>68aa44ad1b9f901601d7216e6b3dd6ab</anchor>
      <arglist>(FILE *out, SM_Trans *sm_trans)</arglist>
    </member>
    <member kind="function">
      <type>SM_Trans *</type>
      <name>SM_can_rename_with_copy</name>
      <anchorfile>sm_8h.html</anchorfile>
      <anchor>fa4265d0ab00ebd1ae3df7dad0b6db92</anchor>
      <arglist>(SM_Oper *sm_op)</arglist>
    </member>
    <member kind="function">
      <type>void</type>
      <name>SM_do_renaming_with_copy</name>
      <anchorfile>sm_8h.html</anchorfile>
      <anchor>902d8feb6a2a317b3f091ec6bd63504e</anchor>
      <arglist>(SM_Trans *sm_trans)</arglist>
    </member>
    <member kind="function">
      <type>void</type>
      <name>SM_undo_renaming_with_copy</name>
      <anchorfile>sm_8h.html</anchorfile>
      <anchor>9bd3f8bfc0af04bbd5e1c525d23e67d1</anchor>
      <arglist>(SM_Trans *sm_trans)</arglist>
    </member>
    <member kind="function">
      <type>void</type>
      <name>SM_update_sched_based_trans_info</name>
      <anchorfile>sm_8h.html</anchorfile>
      <anchor>fd617cf2294c68cdd91fe7abd65bc753</anchor>
      <arglist>(SM_Trans_Queue *queue)</arglist>
    </member>
    <member kind="function">
      <type>int</type>
      <name>SM_calc_action_avail_time</name>
      <anchorfile>sm_8h.html</anchorfile>
      <anchor>dc67f87f50278c4b0fe8415bab0a699b</anchor>
      <arglist>(SM_Reg_Action *action)</arglist>
    </member>
    <member kind="function">
      <type>void</type>
      <name>SM_do_trivial_renaming</name>
      <anchorfile>sm_8h.html</anchorfile>
      <anchor>1cdc24757dd63ce057887c55f3dd15b9</anchor>
      <arglist>(SM_Cb *sm_cb)</arglist>
    </member>
    <member kind="function">
      <type>void</type>
      <name>SM_do_trans</name>
      <anchorfile>sm_8h.html</anchorfile>
      <anchor>5165b5e17f693a2f5067d7981267762c</anchor>
      <arglist>(SM_Trans *trans)</arglist>
    </member>
    <member kind="function">
      <type>int</type>
      <name>SM_can_undo_trans</name>
      <anchorfile>sm_8h.html</anchorfile>
      <anchor>15baf68b27c5793df755db1d9a362604</anchor>
      <arglist>(SM_Trans *trans)</arglist>
    </member>
    <member kind="function">
      <type>void</type>
      <name>SM_undo_trans</name>
      <anchorfile>sm_8h.html</anchorfile>
      <anchor>3c9ec3752ff0eec20efd909a2ea4fa3a</anchor>
      <arglist>(SM_Trans *trans)</arglist>
    </member>
    <member kind="function">
      <type>SM_Trans_Queue *</type>
      <name>SM_find_potential_trans</name>
      <anchorfile>sm_8h.html</anchorfile>
      <anchor>b7775bfb97ca2f7206879f5a7b9c42a5</anchor>
      <arglist>(SM_Cb *sm_cb, unsigned int allowed_trans)</arglist>
    </member>
    <member kind="function">
      <type>void</type>
      <name>SM_delete_trans_and_queue</name>
      <anchorfile>sm_8h.html</anchorfile>
      <anchor>ff666504a1ac4d18f6088328f9181b7d</anchor>
      <arglist>(SM_Trans_Queue *queue)</arglist>
    </member>
    <member kind="function">
      <type>double</type>
      <name>SM_calc_do_trans_priority</name>
      <anchorfile>sm_8h.html</anchorfile>
      <anchor>0f57648fdcb312268824acda1c755747</anchor>
      <arglist>(SM_Trans *sm_trans, int max_late_time)</arglist>
    </member>
    <member kind="function">
      <type>SM_Trans *</type>
      <name>SM_dequeue_best_do_trans</name>
      <anchorfile>sm_8h.html</anchorfile>
      <anchor>fd986dfcdffb3fe41a2b255363bb3c4c</anchor>
      <arglist>(SM_Trans_Queue *queue)</arglist>
    </member>
    <member kind="function">
      <type>void</type>
      <name>SM_gen_code</name>
      <anchorfile>sm_8h.html</anchorfile>
      <anchor>6499934f6bfb2effeb64c21694c0d3e8</anchor>
      <arglist>(Parm_Macro_List *external_macro_list)</arglist>
    </member>
    <member kind="function">
      <type>int</type>
      <name>SM_def_dominates_action</name>
      <anchorfile>sm_8h.html</anchorfile>
      <anchor>7771b59763c0f4b65c37b6f413d6d19e</anchor>
      <arglist>(SM_Reg_Action *def_before, SM_Reg_Action *action)</arglist>
    </member>
    <member kind="function">
      <type>int</type>
      <name>SM_def_has_exactly_one_use</name>
      <anchorfile>sm_8h.html</anchorfile>
      <anchor>c37f54f141f4d55fe31215053b7f8bce</anchor>
      <arglist>(SM_Reg_Action *def_action)</arglist>
    </member>
    <member kind="function">
      <type>void</type>
      <name>SM_create_map</name>
      <anchorfile>sm_8h.html</anchorfile>
      <anchor>effe9b8876d11c3b84ccafe953e94133</anchor>
      <arglist>(SM_Cb *sm_cb, int min_usage_offset, int max_usage_offset)</arglist>
    </member>
    <member kind="function">
      <type>void</type>
      <name>SM_init_for_min_usage</name>
      <anchorfile>sm_8h.html</anchorfile>
      <anchor>4b5b461c48dc75e06f2b24afaaeab80d</anchor>
      <arglist>(SM_Cb *sm_cb, int min_usage_offset)</arglist>
    </member>
    <member kind="function">
      <type>void</type>
      <name>SM_init_for_max_usage</name>
      <anchorfile>sm_8h.html</anchorfile>
      <anchor>224d563efd05ca72ae7d3fd4c8eb9b70</anchor>
      <arglist>(SM_Cb *sm_cb, int max_usage_offset)</arglist>
    </member>
    <member kind="function">
      <type>void</type>
      <name>SM_update_upper_bounds</name>
      <anchorfile>sm_8h.html</anchorfile>
      <anchor>c6bfb5a6c3b8fca141758f95eaebe5ea</anchor>
      <arglist>(SM_Dep *dep, int flags)</arglist>
    </member>
    <member kind="function">
      <type>void</type>
      <name>SM_update_lower_bounds</name>
      <anchorfile>sm_8h.html</anchorfile>
      <anchor>f75ca28e49bf67041938cff46238fbb7</anchor>
      <arglist>(SM_Dep *dep, int flags)</arglist>
    </member>
    <member kind="function">
      <type>void</type>
      <name>SM_check_deps</name>
      <anchorfile>sm_8h.html</anchorfile>
      <anchor>41e30bd0096d6c6ef9ba80bf9b4d0915</anchor>
      <arglist>(SM_Cb *sm_cb)</arglist>
    </member>
    <member kind="function">
      <type>void</type>
      <name>SM_add_recovery_code</name>
      <anchorfile>sm_8h.html</anchorfile>
      <anchor>59a690d789cfb23fb1ca983a66493db8</anchor>
      <arglist>(L_Func *fn)</arglist>
    </member>
    <member kind="function">
      <type>int</type>
      <name>SM_softfix_promotion</name>
      <anchorfile>sm_8h.html</anchorfile>
      <anchor>a818ce2711760516996f4064f60d26cf</anchor>
      <arglist>(SM_Dep *dep_in, int issue_time, int mode)</arglist>
    </member>
    <member kind="function">
      <type>void</type>
      <name>SM_undo_fix_soft_dep</name>
      <anchorfile>sm_8h.html</anchorfile>
      <anchor>309e37e19f6cb298078ad0891a5d9044</anchor>
      <arglist>(SM_Oper *sm_op)</arglist>
    </member>
    <member kind="function">
      <type>double</type>
      <name>SM_do_classic_renaming_with_copy</name>
      <anchorfile>sm_8h.html</anchorfile>
      <anchor>aa7d534b52fab604a9ed23054aaf7350</anchor>
      <arglist>(SM_Cb *sm_cb)</arglist>
    </member>
    <member kind="function">
      <type>int</type>
      <name>SM_modulo_sched_table</name>
      <anchorfile>sm_8h.html</anchorfile>
      <anchor>f2b59ef6f57473b170cfae169fd7111a</anchor>
      <arglist>(SM_Cb *sm_cb, SM_Oper *sm_op, unsigned int template_index, SM_Table *table, unsigned short *choice_array, int time, unsigned short min_slot, unsigned short max_slot, int commit, Mdes_Stats *stats)</arglist>
    </member>
    <member kind="function">
      <type>void</type>
      <name>SM_modulo_unsched_table</name>
      <anchorfile>sm_8h.html</anchorfile>
      <anchor>75269b8c6209e22d317a8720e66aec31</anchor>
      <arglist>(SM_Cb *sm_cb, SM_Table *table, unsigned short *choice_array, int time)</arglist>
    </member>
    <member kind="function">
      <type>int</type>
      <name>SM_fix_soft_dep</name>
      <anchorfile>sm_8h.html</anchorfile>
      <anchor>dcd7056f6802b676a0de5ab84548cdc5</anchor>
      <arglist>(SM_Oper *sm_op, int issue_time, int mode)</arglist>
    </member>
    <member kind="function">
      <type>int</type>
      <name>S_machine_check</name>
      <anchorfile>sm_8h.html</anchorfile>
      <anchor>1cf3946c60d75e862ea4795e9a19fa2f</anchor>
      <arglist>(L_Oper *oper)</arglist>
    </member>
    <member kind="function">
      <type>void</type>
      <name>S_machine_rts</name>
      <anchorfile>sm_8h.html</anchorfile>
      <anchor>cec0c5f5a0d3c08aea116d6871befbe8</anchor>
      <arglist>(L_Oper *oper)</arglist>
    </member>
    <member kind="function">
      <type>void</type>
      <name>S_machine_jump</name>
      <anchorfile>sm_8h.html</anchorfile>
      <anchor>3316c78b66394bf0677d38014a51c6dd</anchor>
      <arglist>(L_Oper *oper)</arglist>
    </member>
    <member kind="function">
      <type>void</type>
      <name>SM_check_prod_cons_lat_match</name>
      <anchorfile>sm_8h.html</anchorfile>
      <anchor>74ec1281733118c51a37aa096af8afc3</anchor>
      <arglist>(SM_Dep *dep, SM_Reg_Action *prod_action, SM_Reg_Action *cons_action)</arglist>
    </member>
    <member kind="function">
      <type>SM_Priority_Queue *</type>
      <name>SM_new_priority_queue</name>
      <anchorfile>sm_8h.html</anchorfile>
      <anchor>e297187b3c57aca27b3cd31dfaccb3c4</anchor>
      <arglist>()</arglist>
    </member>
    <member kind="function">
      <type>void</type>
      <name>SM_reinit_priority_queue</name>
      <anchorfile>sm_8h.html</anchorfile>
      <anchor>b357b1924fdc13d338e206d8352dc753</anchor>
      <arglist>(SM_Priority_Queue *)</arglist>
    </member>
    <member kind="function">
      <type>void</type>
      <name>SM_delete_priority_queue</name>
      <anchorfile>sm_8h.html</anchorfile>
      <anchor>63ab11a333294c4e0518fc6a81520ce2</anchor>
      <arglist>(SM_Priority_Queue *)</arglist>
    </member>
    <member kind="function">
      <type>void</type>
      <name>SM_enqueue_increasing_priority</name>
      <anchorfile>sm_8h.html</anchorfile>
      <anchor>19f0e21806096e3ff0b6ae53f9c6d4a7</anchor>
      <arglist>(SM_Priority_Queue *, SM_Oper *, int)</arglist>
    </member>
    <member kind="function">
      <type>SM_Oper *</type>
      <name>SM_dequeue_priority</name>
      <anchorfile>sm_8h.html</anchorfile>
      <anchor>402347c2103891f4333627df7acfc9d3</anchor>
      <arglist>(SM_Priority_Queue *)</arglist>
    </member>
    <member kind="function">
      <type>SM_Oper *</type>
      <name>SM_peek_end</name>
      <anchorfile>sm_8h.html</anchorfile>
      <anchor>900733754109ac85db8af46cf7d8c7fe</anchor>
      <arglist>(SM_Priority_Queue *)</arglist>
    </member>
    <member kind="function">
      <type>SM_Oper *</type>
      <name>SM_peek_head</name>
      <anchorfile>sm_8h.html</anchorfile>
      <anchor>18a10c019cb3cc1e0630b9fb43c56aa7</anchor>
      <arglist>(SM_Priority_Queue *)</arglist>
    </member>
    <member kind="function">
      <type>SM_Oper *</type>
      <name>SM_create_nop</name>
      <anchorfile>sm_8h.html</anchorfile>
      <anchor>09614662763d9c640b7c2f90593d28b4</anchor>
      <arglist>(SM_Cb *sm_cb, int proc_opc)</arglist>
    </member>
    <member kind="function">
      <type>int</type>
      <name>SM_insert_nop</name>
      <anchorfile>sm_8h.html</anchorfile>
      <anchor>ff3ddf50f7715d11767c701843471b52</anchor>
      <arglist>(SM_Issue_Group *issue_group_ptr, int slot, unsigned int sched_flags)</arglist>
    </member>
    <member kind="function">
      <type>void</type>
      <name>SM_delete_nop</name>
      <anchorfile>sm_8h.html</anchorfile>
      <anchor>dafbaa8210d64b6498014ff890f8b7dd</anchor>
      <arglist>(SM_Oper *sm_op)</arglist>
    </member>
    <member kind="function">
      <type>void</type>
      <name>SM_unschedule_nop</name>
      <anchorfile>sm_8h.html</anchorfile>
      <anchor>6ba636bdbd1041421b142fc7279bcc8e</anchor>
      <arglist>(SM_Oper *sm_op)</arglist>
    </member>
    <member kind="function">
      <type>void</type>
      <name>SM_calculate_template_vector</name>
      <anchorfile>sm_8h.html</anchorfile>
      <anchor>54c9346f255a68d7e6ee3762f82bae06</anchor>
      <arglist>(SM_Issue_Group *issue_group)</arglist>
    </member>
    <member kind="function">
      <type>void</type>
      <name>SM_assign_stop_bit</name>
      <anchorfile>sm_8h.html</anchorfile>
      <anchor>978b0390722d62a243df9ffb9b18daf6</anchor>
      <arglist>(SM_Issue_Group *issue_group_ptr)</arglist>
    </member>
    <member kind="function">
      <type>SM_Issue_Group *</type>
      <name>SM_create_issue_group</name>
      <anchorfile>sm_8h.html</anchorfile>
      <anchor>a4e1936d04b3593d2aed1b38a3058e6b</anchor>
      <arglist>(SM_Cb *sm_cb, int issue_time)</arglist>
    </member>
    <member kind="function">
      <type>void</type>
      <name>SM_copy_issue</name>
      <anchorfile>sm_8h.html</anchorfile>
      <anchor>b269195f789cef850d7c56c160efc9f6</anchor>
      <arglist>(SM_Issue_Group *dest_issue_group_ptr, SM_Issue_Group *src_issue_group_ptr)</arglist>
    </member>
    <member kind="function">
      <type>void</type>
      <name>SM_restore_issue_group</name>
      <anchorfile>sm_8h.html</anchorfile>
      <anchor>90a71091c5506878278bf3b50b65582e</anchor>
      <arglist>(SM_Issue_Group *issue_group_ptr, SM_Issue_Group *save_issue_group_ptr)</arglist>
    </member>
    <member kind="function">
      <type>int</type>
      <name>SM_schedule_oper_priority</name>
      <anchorfile>sm_8h.html</anchorfile>
      <anchor>72f55612976b7dd84719fe00c6069311</anchor>
      <arglist>(SM_Oper *sm_op, int issue_time, int earliest_slot, int latest_slot, unsigned int sched_flags, int compact)</arglist>
    </member>
    <member kind="function">
      <type>void</type>
      <name>SM_compact_w_internal_sbits</name>
      <anchorfile>sm_8h.html</anchorfile>
      <anchor>6a0a0bcad66694715097694df479f75f</anchor>
      <arglist>(SM_Cb *sm_cb)</arglist>
    </member>
    <member kind="function">
      <type>int</type>
      <name>SM_find_drule</name>
      <anchorfile>sm_8h.html</anchorfile>
      <anchor>940e48b11af85e3a973ac5d7e8fb94ea</anchor>
      <arglist>(SM_Oper *sm_op, unsigned int abs_slot, unsigned int template_index, int prev_index)</arglist>
    </member>
    <member kind="function">
      <type>int *</type>
      <name>SM_init_port_array</name>
      <anchorfile>sm_8h.html</anchorfile>
      <anchor>9bcde7caa3406c0cebd25a52f03f9f9e</anchor>
      <arglist>(SM_Cb *sm_cb)</arglist>
    </member>
    <member kind="function">
      <type>SM_Issue_Group *</type>
      <name>SM_check_for_issue_group</name>
      <anchorfile>sm_8h.html</anchorfile>
      <anchor>ed246353f15827ff68b8682bd97afe52</anchor>
      <arglist>(SM_Cb *sm_cb, int issue_time)</arglist>
    </member>
    <member kind="variable">
      <type>L_Operand</type>
      <name>_sm_mem_action_operand</name>
      <anchorfile>sm_8h.html</anchorfile>
      <anchor>e7fe0abd473941eb00c58aef727de47e</anchor>
      <arglist></arglist>
    </member>
    <member kind="variable">
      <type>L_Operand</type>
      <name>_sm_ctrl_action_operand</name>
      <anchorfile>sm_8h.html</anchorfile>
      <anchor>c2e1e9d6130b4f86edc1be2465acec34</anchor>
      <arglist></arglist>
    </member>
    <member kind="variable">
      <type>L_Operand</type>
      <name>_sm_sync_action_operand</name>
      <anchorfile>sm_8h.html</anchorfile>
      <anchor>e52b5e3b36e3d39d4b519286c096916d</anchor>
      <arglist></arglist>
    </member>
    <member kind="variable">
      <type>L_Operand</type>
      <name>_sm_vliw_action_operand</name>
      <anchorfile>sm_8h.html</anchorfile>
      <anchor>97b62f78e5420a1adb546de69059cae9</anchor>
      <arglist></arglist>
    </member>
    <member kind="variable">
      <type>int</type>
      <name>SM_make_Lbx86_assumptions</name>
      <anchorfile>sm_8h.html</anchorfile>
      <anchor>391e10c9ec3aee576745eaf3cae00d48</anchor>
      <arglist></arglist>
    </member>
    <member kind="variable">
      <type>int</type>
      <name>SM_prevent_dead_Lbx86_defs_from_reordering</name>
      <anchorfile>sm_8h.html</anchorfile>
      <anchor>910fcde5f547d7b6b0ce91f389bebd9b</anchor>
      <arglist></arglist>
    </member>
    <member kind="variable">
      <type>int</type>
      <name>SM_use_fake_Lbx86_flow_analysis</name>
      <anchorfile>sm_8h.html</anchorfile>
      <anchor>627102177305bc81aac9b5058579eace</anchor>
      <arglist></arglist>
    </member>
    <member kind="variable">
      <type>int</type>
      <name>SM_check_dependence_symmetry</name>
      <anchorfile>sm_8h.html</anchorfile>
      <anchor>6113fa13a710c6bd4de9af49ad33c02e</anchor>
      <arglist></arglist>
    </member>
    <member kind="variable">
      <type>int</type>
      <name>SM_debug_use_sched_cb_bounds</name>
      <anchorfile>sm_8h.html</anchorfile>
      <anchor>0e989c9ef319284e13edcf549fbce1f9</anchor>
      <arglist></arglist>
    </member>
    <member kind="variable">
      <type>int</type>
      <name>SM_debug_lower_sched_cb_bound</name>
      <anchorfile>sm_8h.html</anchorfile>
      <anchor>8b15ed0b8ad4788a93dc8171a259da5e</anchor>
      <arglist></arglist>
    </member>
    <member kind="variable">
      <type>int</type>
      <name>SM_debug_upper_sched_cb_bound</name>
      <anchorfile>sm_8h.html</anchorfile>
      <anchor>8dfae2cab053bb201fe3253760fa96b9</anchor>
      <arglist></arglist>
    </member>
    <member kind="variable">
      <type>int</type>
      <name>SM_use_fake_dataflow_info</name>
      <anchorfile>sm_8h.html</anchorfile>
      <anchor>bb150905a2531776bb8343ba659f1217</anchor>
      <arglist></arglist>
    </member>
    <member kind="variable">
      <type>int</type>
      <name>SM_verify_reg_conflicts</name>
      <anchorfile>sm_8h.html</anchorfile>
      <anchor>f00f8f047efcd90a65c217f021f5119e</anchor>
      <arglist></arglist>
    </member>
    <member kind="variable">
      <type>int</type>
      <name>SM_output_dep_distance</name>
      <anchorfile>sm_8h.html</anchorfile>
      <anchor>ab0a90fa5ee31aa6b37030bb42e1742c</anchor>
      <arglist></arglist>
    </member>
    <member kind="variable">
      <type>int</type>
      <name>SM_ignore_pred_analysis</name>
      <anchorfile>sm_8h.html</anchorfile>
      <anchor>7ebc6c47c0604862b6cfe9eaea19059b</anchor>
      <arglist></arglist>
    </member>
    <member kind="variable">
      <type>int</type>
      <name>SM_perform_rename_with_copy</name>
      <anchorfile>sm_8h.html</anchorfile>
      <anchor>9075336d29d43ce08eb1cf90de03eb9d</anchor>
      <arglist></arglist>
    </member>
    <member kind="variable">
      <type>int</type>
      <name>SM_perform_relocate_cond_opti</name>
      <anchorfile>sm_8h.html</anchorfile>
      <anchor>5b707937b3ff9286ac4dbcfa937222b7</anchor>
      <arglist></arglist>
    </member>
    <member kind="variable">
      <type>int</type>
      <name>SM_sched_slack_loads_for_miss</name>
      <anchorfile>sm_8h.html</anchorfile>
      <anchor>0ae4b2ad45eaa593d23cd4512ef97ff7</anchor>
      <arglist></arglist>
    </member>
    <member kind="variable">
      <type>int</type>
      <name>SM_do_template_bundling</name>
      <anchorfile>sm_8h.html</anchorfile>
      <anchor>d9c780f4de4ccbbea7c2e754d2954c64</anchor>
      <arglist></arglist>
    </member>
    <member kind="variable">
      <type>int</type>
      <name>SM_do_bundle_compaction</name>
      <anchorfile>sm_8h.html</anchorfile>
      <anchor>6918c004ce79dd79b0171d57f5d931a3</anchor>
      <arglist></arglist>
    </member>
  </compound>
  <compound kind="file">
    <name>sm_compact.c</name>
    <path>/mnt/hgfs/mkt/Desktop/openimpact/src/Lcode/sched/SM/</path>
    <filename>sm__compact_8c</filename>
    <includes id="sm_8h" name="sm.h" local="yes" imported="no">sm.h</includes>
    <member kind="function">
      <type>int</type>
      <name>SM_reschedule_issue</name>
      <anchorfile>sm__compact_8c.html</anchorfile>
      <anchor>08477d5d96627cf2f6d92be9e6f1bf8f</anchor>
      <arglist>(SM_Issue_Group *issue_group_ptr, int earliest_slot, int latest_slot)</arglist>
    </member>
    <member kind="function">
      <type>void</type>
      <name>SM_compact_w_internal_sbits</name>
      <anchorfile>sm__compact_8c.html</anchorfile>
      <anchor>6a0a0bcad66694715097694df479f75f</anchor>
      <arglist>(SM_Cb *sm_cb)</arglist>
    </member>
  </compound>
  <compound kind="file">
    <name>sm_cudd.c</name>
    <path>/mnt/hgfs/mkt/Desktop/openimpact/src/Lcode/sched/SM/</path>
    <filename>sm__cudd_8c</filename>
    <includes id="sm_8h" name="sm.h" local="yes" imported="no">sm.h</includes>
    <includes id="sm__cudd_8h" name="sm_cudd.h" local="yes" imported="no">sm_cudd.h</includes>
    <member kind="function">
      <type>void</type>
      <name>SM_logexpr_init</name>
      <anchorfile>sm__cudd_8c.html</anchorfile>
      <anchor>4bff684ba99ea46729f64bf4287570c3</anchor>
      <arglist>(void **p_sm_le, SM_Reg_Action *sm_ract)</arglist>
    </member>
    <member kind="function">
      <type>int</type>
      <name>SM_logexpr_sub_accum_def</name>
      <anchorfile>sm__cudd_8c.html</anchorfile>
      <anchor>bbb499a77d35cf0697721afe4b87aeb0</anchor>
      <arglist>(void **p_sm_le, SM_Reg_Action *sm_def_act)</arglist>
    </member>
    <member kind="function">
      <type>int</type>
      <name>SM_logexpr_ne</name>
      <anchorfile>sm__cudd_8c.html</anchorfile>
      <anchor>670f5c8d00c069e88214a998fc382f17</anchor>
      <arglist>(void *p_sm_le)</arglist>
    </member>
    <member kind="function">
      <type>void</type>
      <name>SM_logexpr_dispose</name>
      <anchorfile>sm__cudd_8c.html</anchorfile>
      <anchor>4ebbc83efa7a6e0ce00ee81b0f99cf58</anchor>
      <arglist>(void **p_sm_le)</arglist>
    </member>
  </compound>
  <compound kind="file">
    <name>sm_cudd.h</name>
    <path>/mnt/hgfs/mkt/Desktop/openimpact/src/Lcode/sched/SM/</path>
    <filename>sm__cudd_8h</filename>
    <member kind="function">
      <type>void</type>
      <name>SM_logexpr_init</name>
      <anchorfile>sm__cudd_8h.html</anchorfile>
      <anchor>27ab98f9b618a2c5fef027d62707ba99</anchor>
      <arglist>(void **sm_le, SM_Reg_Action *sm_use_act)</arglist>
    </member>
    <member kind="function">
      <type>int</type>
      <name>SM_logexpr_sub_accum_def</name>
      <anchorfile>sm__cudd_8h.html</anchorfile>
      <anchor>384b323bae1a84b9bab6d2242098d92c</anchor>
      <arglist>(void **sm_le, SM_Reg_Action *sm_def_act)</arglist>
    </member>
    <member kind="function">
      <type>int</type>
      <name>SM_logexpr_ne</name>
      <anchorfile>sm__cudd_8h.html</anchorfile>
      <anchor>53e75d1549716d1cc818c5bc21d3bb58</anchor>
      <arglist>(void *sm_le)</arglist>
    </member>
    <member kind="function">
      <type>void</type>
      <name>SM_logexpr_dispose</name>
      <anchorfile>sm__cudd_8h.html</anchorfile>
      <anchor>b5b6140ef164cc9c0ebfd660b7b2443b</anchor>
      <arglist>(void **sm_le)</arglist>
    </member>
  </compound>
  <compound kind="file">
    <name>sm_dep.c</name>
    <path>/mnt/hgfs/mkt/Desktop/openimpact/src/Lcode/sched/SM/</path>
    <filename>sm__dep_8c</filename>
    <includes id="sm_8h" name="sm.h" local="yes" imported="no">sm.h</includes>
    <includes id="sm__cudd_8h" name="sm_cudd.h" local="yes" imported="no">sm_cudd.h</includes>
    <member kind="function">
      <type>void</type>
      <name>SM_check_prod_cons_lat_match</name>
      <anchorfile>sm__dep_8c.html</anchorfile>
      <anchor>74ec1281733118c51a37aa096af8afc3</anchor>
      <arglist>(SM_Dep *dep, SM_Reg_Action *prod_action, SM_Reg_Action *cons_action)</arglist>
    </member>
    <member kind="function">
      <type>void</type>
      <name>SM_update_upper_bounds</name>
      <anchorfile>sm__dep_8c.html</anchorfile>
      <anchor>c6bfb5a6c3b8fca141758f95eaebe5ea</anchor>
      <arglist>(SM_Dep *dep, int flags)</arglist>
    </member>
    <member kind="function">
      <type>void</type>
      <name>SM_update_lower_bounds</name>
      <anchorfile>sm__dep_8c.html</anchorfile>
      <anchor>f75ca28e49bf67041938cff46238fbb7</anchor>
      <arglist>(SM_Dep *dep, int flags)</arglist>
    </member>
    <member kind="function">
      <type>int</type>
      <name>SM_mutually_exclusive_actions</name>
      <anchorfile>sm__dep_8c.html</anchorfile>
      <anchor>5f789fcb9f896a1143440b87d92185b8</anchor>
      <arglist>(SM_Reg_Action *action1, int demoted1, SM_Reg_Action *action2, int demoted2)</arglist>
    </member>
    <member kind="function">
      <type>void</type>
      <name>SM_check_deps</name>
      <anchorfile>sm__dep_8c.html</anchorfile>
      <anchor>41e30bd0096d6c6ef9ba80bf9b4d0915</anchor>
      <arglist>(SM_Cb *sm_cb)</arglist>
    </member>
    <member kind="function">
      <type>int</type>
      <name>SM_Lbx86_def_action_is_dead</name>
      <anchorfile>sm__dep_8c.html</anchorfile>
      <anchor>6a7fce0379e145748d07abadb61184ff</anchor>
      <arglist>(SM_Reg_Action *def_action)</arglist>
    </member>
    <member kind="function">
      <type>void</type>
      <name>SM_add_dep</name>
      <anchorfile>sm__dep_8c.html</anchorfile>
      <anchor>b8c070732c443a056f2f6a3d570a06fd</anchor>
      <arglist>(SM_Reg_Action *from_action, SM_Reg_Action *to_action, unsigned int dep_flags, unsigned int ignore, short delay_offset, short omega, unsigned int mode)</arglist>
    </member>
    <member kind="function">
      <type>void</type>
      <name>SM_enable_ignored_dep</name>
      <anchorfile>sm__dep_8c.html</anchorfile>
      <anchor>9a522675eb1ff43978448d4a2c749277</anchor>
      <arglist>(SM_Dep *dep, int flag)</arglist>
    </member>
    <member kind="function">
      <type>void</type>
      <name>SM_ignore_dep</name>
      <anchorfile>sm__dep_8c.html</anchorfile>
      <anchor>788482acf72fd688418e5d4560967a4a</anchor>
      <arglist>(SM_Dep *dep, int flag)</arglist>
    </member>
    <member kind="function">
      <type>void</type>
      <name>SM_ignore_dep_out</name>
      <anchorfile>sm__dep_8c.html</anchorfile>
      <anchor>44142e3eb60a8617f5e88b2498e642d7</anchor>
      <arglist>(SM_Oper *sm_op, int flag)</arglist>
    </member>
    <member kind="function">
      <type>void</type>
      <name>SM_enable_ignored_dep_out</name>
      <anchorfile>sm__dep_8c.html</anchorfile>
      <anchor>263290420f908b8afcb5c783ef65b1ac</anchor>
      <arglist>(SM_Oper *sm_op, int flag)</arglist>
    </member>
    <member kind="function">
      <type>void</type>
      <name>SM_ignore_dep_in</name>
      <anchorfile>sm__dep_8c.html</anchorfile>
      <anchor>8e3744ccc23eee9c8b5af6b81555e459</anchor>
      <arglist>(SM_Oper *sm_op, int flag)</arglist>
    </member>
    <member kind="function">
      <type>void</type>
      <name>SM_enable_ignored_dep_in</name>
      <anchorfile>sm__dep_8c.html</anchorfile>
      <anchor>358ba5a8828e39c9c2a19c1e99e7926e</anchor>
      <arglist>(SM_Oper *sm_op, int flag)</arglist>
    </member>
    <member kind="function">
      <type>void</type>
      <name>SM_delete_dep</name>
      <anchorfile>sm__dep_8c.html</anchorfile>
      <anchor>ac2fdd3298e3b911c3afca217b70ef75</anchor>
      <arglist>(SM_Dep *dep)</arglist>
    </member>
    <member kind="function">
      <type>int</type>
      <name>SM_def_dominates_action</name>
      <anchorfile>sm__dep_8c.html</anchorfile>
      <anchor>7771b59763c0f4b65c37b6f413d6d19e</anchor>
      <arglist>(SM_Reg_Action *def_before, SM_Reg_Action *action)</arglist>
    </member>
    <member kind="function">
      <type>int</type>
      <name>SM_def_dominates_cross_iter_action</name>
      <anchorfile>sm__dep_8c.html</anchorfile>
      <anchor>2044550c405b73f7c6bc2a79fca4dfc1</anchor>
      <arglist>(SM_Reg_Action *def_after, SM_Reg_Action *action)</arglist>
    </member>
    <member kind="function">
      <type>int</type>
      <name>SM_def_post_dominates_action</name>
      <anchorfile>sm__dep_8c.html</anchorfile>
      <anchor>a2f28155679ad09f0bc57b1542bad633</anchor>
      <arglist>(SM_Reg_Action *def_after, SM_Reg_Action *action)</arglist>
    </member>
    <member kind="function">
      <type>int</type>
      <name>SM_def_post_dominates_cross_iter_action</name>
      <anchorfile>sm__dep_8c.html</anchorfile>
      <anchor>956f86a0234edee09771ad9049226635</anchor>
      <arglist>(SM_Reg_Action *def_before, SM_Reg_Action *action)</arglist>
    </member>
    <member kind="function">
      <type>void</type>
      <name>SM_build_src_reg_deps</name>
      <anchorfile>sm__dep_8c.html</anchorfile>
      <anchor>7cb00c843c6585c4884a0db5a546860b</anchor>
      <arglist>(SM_Reg_Action *src_action, unsigned int flags)</arglist>
    </member>
    <member kind="function">
      <type>void</type>
      <name>SM_build_dest_reg_deps</name>
      <anchorfile>sm__dep_8c.html</anchorfile>
      <anchor>5ded743fdf32f8ee86ae033c45264c79</anchor>
      <arglist>(SM_Reg_Action *dest_action, unsigned int flags)</arglist>
    </member>
    <member kind="function">
      <type>void</type>
      <name>SM_rebuild_dest_reg_deps</name>
      <anchorfile>sm__dep_8c.html</anchorfile>
      <anchor>4ef2c5a485745481b74997f1ce3187c8</anchor>
      <arglist>(SM_Reg_Action *dest_action, unsigned int flags)</arglist>
    </member>
    <member kind="function" static="yes">
      <type>static int</type>
      <name>SM_independent_memory_actions</name>
      <anchorfile>sm__dep_8c.html</anchorfile>
      <anchor>4db7cdb5667bf85ca448815a55b6dc70</anchor>
      <arglist>(SM_Reg_Action *def_action, SM_Reg_Action *other_action)</arglist>
    </member>
    <member kind="function" static="yes">
      <type>static int</type>
      <name>SM_independent_cross_iter_memory_actions</name>
      <anchorfile>sm__dep_8c.html</anchorfile>
      <anchor>018dac5f71d448c13adddb1bc8c2d049</anchor>
      <arglist>(SM_Reg_Action *from_action, SM_Reg_Action *to_action, int *omega)</arglist>
    </member>
    <member kind="function">
      <type>void</type>
      <name>SM_build_src_mem_deps</name>
      <anchorfile>sm__dep_8c.html</anchorfile>
      <anchor>3a27b3890cf76120ed386c20725ad8e5</anchor>
      <arglist>(SM_Reg_Action *src_action, unsigned int flags)</arglist>
    </member>
    <member kind="function">
      <type>void</type>
      <name>SM_build_dest_mem_deps</name>
      <anchorfile>sm__dep_8c.html</anchorfile>
      <anchor>8f6b3db14ed4a5913e6086bb30e2268b</anchor>
      <arglist>(SM_Reg_Action *dest_action, unsigned int flags)</arglist>
    </member>
    <member kind="function">
      <type>int</type>
      <name>SM_prevent_from_moving_above_branch</name>
      <anchorfile>sm__dep_8c.html</anchorfile>
      <anchor>3e974cf5763f6688074e906ffd1bc7e0</anchor>
      <arglist>(int prepass, unsigned int *ignore, SM_Reg_Action *branch, SM_Reg_Action *action)</arglist>
    </member>
    <member kind="function">
      <type>int</type>
      <name>SM_safe_to_move_below_branch</name>
      <anchorfile>sm__dep_8c.html</anchorfile>
      <anchor>31fb0e6d913bdc7f3c5d7d28fcbaf98c</anchor>
      <arglist>(SM_Reg_Action *branch, SM_Reg_Action *action)</arglist>
    </member>
    <member kind="function">
      <type>void</type>
      <name>SM_build_src_ctrl_deps</name>
      <anchorfile>sm__dep_8c.html</anchorfile>
      <anchor>3d2f9302524a17842be1b37a7bba35e6</anchor>
      <arglist>(SM_Cb *sm_cb, SM_Reg_Action *src_action, unsigned int flags)</arglist>
    </member>
    <member kind="function">
      <type>void</type>
      <name>SM_build_dest_ctrl_deps</name>
      <anchorfile>sm__dep_8c.html</anchorfile>
      <anchor>529ab90c2d95bab6465c4ba5c5022714</anchor>
      <arglist>(SM_Cb *sm_cb, SM_Reg_Action *dest_action, unsigned int flags)</arglist>
    </member>
    <member kind="function">
      <type>void</type>
      <name>SM_build_src_sync_deps</name>
      <anchorfile>sm__dep_8c.html</anchorfile>
      <anchor>6f787af080a3c4598a459e25a33ff5a3</anchor>
      <arglist>(SM_Reg_Action *src_action, unsigned int flags)</arglist>
    </member>
    <member kind="function">
      <type>void</type>
      <name>SM_build_dest_sync_deps</name>
      <anchorfile>sm__dep_8c.html</anchorfile>
      <anchor>9a500402c6936eb2b5c962f063f652a7</anchor>
      <arglist>(SM_Reg_Action *dest_action, unsigned int flags)</arglist>
    </member>
    <member kind="function">
      <type>void</type>
      <name>SM_build_oper_dependences</name>
      <anchorfile>sm__dep_8c.html</anchorfile>
      <anchor>ca4ceaf192b3e9be6abc7c6b0cd6520f</anchor>
      <arglist>(SM_Cb *sm_cb, SM_Oper *sm_op, unsigned int flags)</arglist>
    </member>
    <member kind="function">
      <type>int</type>
      <name>SM_harmless_dep</name>
      <anchorfile>sm__dep_8c.html</anchorfile>
      <anchor>fcec434936230436447a21bc2c24bffd</anchor>
      <arglist>(SM_Dep *test_dep)</arglist>
    </member>
    <member kind="function">
      <type>int</type>
      <name>SM_extraneous_dep</name>
      <anchorfile>sm__dep_8c.html</anchorfile>
      <anchor>1a1cac10294a4808b9a3badaeb681795</anchor>
      <arglist>(SM_Dep *dep)</arglist>
    </member>
    <member kind="function">
      <type>void</type>
      <name>SM_check_and_delete_symmetric_deps</name>
      <anchorfile>sm__dep_8c.html</anchorfile>
      <anchor>879d4f7f85c7d271a67015c8254bae50</anchor>
      <arglist>(SM_Cb *sm_cb)</arglist>
    </member>
    <member kind="function">
      <type>void</type>
      <name>SM_build_cb_dependences</name>
      <anchorfile>sm__dep_8c.html</anchorfile>
      <anchor>32828cc8c9780473ae2c45d8ac94e76e</anchor>
      <arglist>(SM_Cb *sm_cb)</arglist>
    </member>
    <member kind="function">
      <type>void</type>
      <name>SM_print_action_operand</name>
      <anchorfile>sm__dep_8c.html</anchorfile>
      <anchor>eacae75507fbe3eb5ae13f934fac24af</anchor>
      <arglist>(FILE *out, SM_Reg_Action *action)</arglist>
    </member>
    <member kind="function">
      <type>void</type>
      <name>SM_print_oper</name>
      <anchorfile>sm__dep_8c.html</anchorfile>
      <anchor>2f11330eb01d12e8df1774421c308f27</anchor>
      <arglist>(FILE *out, SM_Oper *sm_op)</arglist>
    </member>
    <member kind="function">
      <type>void</type>
      <name>SM_print_action_id</name>
      <anchorfile>sm__dep_8c.html</anchorfile>
      <anchor>fafed09ceccb2e9e8b04e6417882f307</anchor>
      <arglist>(FILE *out, SM_Reg_Action *action)</arglist>
    </member>
    <member kind="function">
      <type>void</type>
      <name>SM_print_dep</name>
      <anchorfile>sm__dep_8c.html</anchorfile>
      <anchor>ff31fcd5849b6cad5018b4ccfb101c55</anchor>
      <arglist>(FILE *out, SM_Dep *dep)</arglist>
    </member>
    <member kind="function">
      <type>void</type>
      <name>SM_print_oper_dependences</name>
      <anchorfile>sm__dep_8c.html</anchorfile>
      <anchor>5d03fe869330cd68884e1b6cec012d0b</anchor>
      <arglist>(FILE *out, SM_Oper *sm_op)</arglist>
    </member>
    <member kind="function">
      <type>void</type>
      <name>SM_print_cb_dependences</name>
      <anchorfile>sm__dep_8c.html</anchorfile>
      <anchor>3146d8578acda2cab53688b0ed0354dc</anchor>
      <arglist>(FILE *out, SM_Cb *sm_cb)</arglist>
    </member>
    <member kind="function">
      <type>void</type>
      <name>SM_insert_sync_dep_betw_ops</name>
      <anchorfile>sm__dep_8c.html</anchorfile>
      <anchor>0030e13c6f63ea25980021040849f95f</anchor>
      <arglist>(SM_Oper *from_op, SM_Oper *to_op)</arglist>
    </member>
    <member kind="function">
      <type>void</type>
      <name>SM_print_dep_dot</name>
      <anchorfile>sm__dep_8c.html</anchorfile>
      <anchor>66fb33b42bc840c256d178cc72d09ea3</anchor>
      <arglist>(FILE *out, SM_Dep *dep)</arglist>
    </member>
    <member kind="function">
      <type>void</type>
      <name>SM_print_oper_deps_dot</name>
      <anchorfile>sm__dep_8c.html</anchorfile>
      <anchor>3222f3956a354196083acf5659378e83</anchor>
      <arglist>(FILE *out, SM_Oper *sm_op)</arglist>
    </member>
    <member kind="function">
      <type>void</type>
      <name>SM_print_cb_deps_dot</name>
      <anchorfile>sm__dep_8c.html</anchorfile>
      <anchor>2eac6c928a7f8433c7cbe8c0c3593f80</anchor>
      <arglist>(FILE *out, SM_Cb *sm_cb)</arglist>
    </member>
    <member kind="function">
      <type>void</type>
      <name>SM_print_cb_dot_graph</name>
      <anchorfile>sm__dep_8c.html</anchorfile>
      <anchor>637923635a2da477634e82f5a0fba872</anchor>
      <arglist>(SM_Cb *sm_cb, char *file)</arglist>
    </member>
    <member kind="function">
      <type>void</type>
      <name>SM_print_cb_deps_dot2</name>
      <anchorfile>sm__dep_8c.html</anchorfile>
      <anchor>e52c1f8b15397da9dfc59bdfa46f4204</anchor>
      <arglist>(SM_Cb *sm_cb)</arglist>
    </member>
    <member kind="variable">
      <type>L_Alloc_Pool *</type>
      <name>SM_Dep_pool</name>
      <anchorfile>sm__dep_8c.html</anchorfile>
      <anchor>9bb765c5a4a3f49a00cf29bb3cff7305</anchor>
      <arglist></arglist>
    </member>
    <member kind="variable">
      <type>L_Alloc_Pool *</type>
      <name>SM_Dep_PCLat_pool</name>
      <anchorfile>sm__dep_8c.html</anchorfile>
      <anchor>196714c0ac11d6664092521ba9548931</anchor>
      <arglist></arglist>
    </member>
    <member kind="variable">
      <type>int</type>
      <name>SM_make_Lbx86_assumptions</name>
      <anchorfile>sm__dep_8c.html</anchorfile>
      <anchor>391e10c9ec3aee576745eaf3cae00d48</anchor>
      <arglist></arglist>
    </member>
    <member kind="variable">
      <type>int</type>
      <name>SM_use_fake_dataflow_info</name>
      <anchorfile>sm__dep_8c.html</anchorfile>
      <anchor>bb150905a2531776bb8343ba659f1217</anchor>
      <arglist></arglist>
    </member>
    <member kind="variable">
      <type>int</type>
      <name>SM_prevent_dead_Lbx86_defs_from_reordering</name>
      <anchorfile>sm__dep_8c.html</anchorfile>
      <anchor>910fcde5f547d7b6b0ce91f389bebd9b</anchor>
      <arglist></arglist>
    </member>
    <member kind="variable">
      <type>int</type>
      <name>SM_use_fake_Lbx86_flow_analysis</name>
      <anchorfile>sm__dep_8c.html</anchorfile>
      <anchor>627102177305bc81aac9b5058579eace</anchor>
      <arglist></arglist>
    </member>
  </compound>
  <compound kind="file">
    <name>sm_linear.c</name>
    <path>/mnt/hgfs/mkt/Desktop/openimpact/src/Lcode/sched/SM/</path>
    <filename>sm__linear_8c</filename>
    <includes id="sm_8h" name="sm.h" local="yes" imported="no">sm.h</includes>
    <class kind="struct">Cb_Stats</class>
    <member kind="define">
      <type>#define</type>
      <name>MD_DEBUG_MACROS</name>
      <anchorfile>sm__linear_8c.html</anchorfile>
      <anchor>e7f9e74008b6fd5aacf14ed3e9c8bdf2</anchor>
      <arglist></arglist>
    </member>
    <member kind="define">
      <type>#define</type>
      <name>CHECK_TRANS</name>
      <anchorfile>sm__linear_8c.html</anchorfile>
      <anchor>863a5d17556a04011fbb96bf281636af</anchor>
      <arglist></arglist>
    </member>
    <member kind="function">
      <type>void</type>
      <name>SM_test_insert_and_delete</name>
      <anchorfile>sm__linear_8c.html</anchorfile>
      <anchor>9e52006c5e33277a7b589b9b7ca2b729</anchor>
      <arglist>(SM_Cb *sm_cb)</arglist>
    </member>
    <member kind="function">
      <type>void</type>
      <name>update_cb_stats</name>
      <anchorfile>sm__linear_8c.html</anchorfile>
      <anchor>644b0cf3b428db9bd895a154fcb0c318</anchor>
      <arglist>(SM_Cb *sm_cb, double orig_cycles, double opti_cycles)</arglist>
    </member>
    <member kind="function">
      <type>void</type>
      <name>print_top_weight_stats</name>
      <anchorfile>sm__linear_8c.html</anchorfile>
      <anchor>538bb1ba8588e31c3534e462e8d243e4</anchor>
      <arglist>(FILE *out, int max_count)</arglist>
    </member>
    <member kind="function">
      <type>void</type>
      <name>print_top_diff_stats</name>
      <anchorfile>sm__linear_8c.html</anchorfile>
      <anchor>86233bc909c7cd2602fb42f1c48e2f5a</anchor>
      <arglist>(FILE *out, int max_count)</arglist>
    </member>
    <member kind="function">
      <type>void</type>
      <name>update_diff_stats</name>
      <anchorfile>sm__linear_8c.html</anchorfile>
      <anchor>9b846d28a1e30a5de43d759bd06a9300</anchor>
      <arglist>(double percent_diff)</arglist>
    </member>
    <member kind="function">
      <type>void</type>
      <name>print_diff_stats</name>
      <anchorfile>sm__linear_8c.html</anchorfile>
      <anchor>e2788527be79c9cf028e6685bff7c8bd</anchor>
      <arglist>(FILE *out)</arglist>
    </member>
    <member kind="function">
      <type>void</type>
      <name>SM_summarize_trans_queue</name>
      <anchorfile>sm__linear_8c.html</anchorfile>
      <anchor>a134ce32d18a19eaf302edd0384f823a</anchor>
      <arglist>(FILE *out, SM_Trans_Queue *queue)</arglist>
    </member>
    <member kind="function">
      <type>double</type>
      <name>SM_do_classic_renaming_with_copy</name>
      <anchorfile>sm__linear_8c.html</anchorfile>
      <anchor>aa7d534b52fab604a9ed23054aaf7350</anchor>
      <arglist>(SM_Cb *sm_cb)</arglist>
    </member>
    <member kind="function">
      <type>double</type>
      <name>SM_do_linear_search</name>
      <anchorfile>sm__linear_8c.html</anchorfile>
      <anchor>a89bf876a6e82784d665bef7e9aabc05</anchor>
      <arglist>(SM_Cb *sm_cb, double initial_height, unsigned int allowed_trans)</arglist>
    </member>
    <member kind="function">
      <type>double</type>
      <name>SM_do_linear_search2</name>
      <anchorfile>sm__linear_8c.html</anchorfile>
      <anchor>88335a94c57fe50c28d23317f2e23de3</anchor>
      <arglist>(SM_Cb *sm_cb, double initial_height, unsigned int allowed_trans)</arglist>
    </member>
    <member kind="function" static="yes">
      <type>static void</type>
      <name>print_effect</name>
      <anchorfile>sm__linear_8c.html</anchorfile>
      <anchor>82b2892ed954dea7248f04847b4976a4</anchor>
      <arglist>(char *header, int pass, SM_Cb *sm_cb, double before, double after)</arglist>
    </member>
    <member kind="function" static="yes">
      <type>static double</type>
      <name>calc_eval_height</name>
      <anchorfile>sm__linear_8c.html</anchorfile>
      <anchor>0a85e0f7d1f83ddb493f3de7ccc97398</anchor>
      <arglist>(L_Cb *cb)</arglist>
    </member>
    <member kind="function">
      <type>void</type>
      <name>update_phd_stats</name>
      <anchorfile>sm__linear_8c.html</anchorfile>
      <anchor>094a76a3e7434ca6b4a826231ffce9b1</anchor>
      <arglist>(MD *phd_stats, SM_Cb *sm_cb, double cycles, int flags, int performed_flags)</arglist>
    </member>
    <member kind="function">
      <type>int</type>
      <name>test_opti</name>
      <anchorfile>sm__linear_8c.html</anchorfile>
      <anchor>1c9fa2be9cabbd266a514881167909e8</anchor>
      <arglist>(L_Func *fn, MD *phd_stats)</arglist>
    </member>
    <member kind="function">
      <type>void</type>
      <name>print_op</name>
      <anchorfile>sm__linear_8c.html</anchorfile>
      <anchor>db75a56676e4b787f742beddeb5112ac</anchor>
      <arglist>(SM_Oper *op)</arglist>
    </member>
    <member kind="function">
      <type>void</type>
      <name>print_exits</name>
      <anchorfile>sm__linear_8c.html</anchorfile>
      <anchor>ce897dd3a123fd759933fcde92177a0d</anchor>
      <arglist>(SM_Cb *cb)</arglist>
    </member>
    <member kind="function">
      <type>void</type>
      <name>print_all_op</name>
      <anchorfile>sm__linear_8c.html</anchorfile>
      <anchor>b193b8fb1cbf322c377f2908d7060293</anchor>
      <arglist>(SM_Cb *sm_cb)</arglist>
    </member>
    <member kind="function">
      <type>int</type>
      <name>find_flow_wight_cb2</name>
      <anchorfile>sm__linear_8c.html</anchorfile>
      <anchor>329754ebb32570cf32eb6656637875f6</anchor>
      <arglist>(SM_Cb *sm_cb, L_Cb *cb)</arglist>
    </member>
    <member kind="function">
      <type>int</type>
      <name>find_flow_wight_cb</name>
      <anchorfile>sm__linear_8c.html</anchorfile>
      <anchor>d492a96bebf88a2c13c8e46086cfe592</anchor>
      <arglist>(SM_Cb *sm_cb, L_Cb *cb)</arglist>
    </member>
    <member kind="function">
      <type>int</type>
      <name>find_flow_wight</name>
      <anchorfile>sm__linear_8c.html</anchorfile>
      <anchor>fa544ed15ad3eb18777a5a3636a43705</anchor>
      <arglist>(L_Func *fn)</arglist>
    </member>
    <member kind="function">
      <type>int</type>
      <name>calc_wcet</name>
      <anchorfile>sm__linear_8c.html</anchorfile>
      <anchor>1fcbc0b88abf8a53a0bb0c64d3749589</anchor>
      <arglist>(L_Cb *cb)</arglist>
    </member>
    <member kind="function">
      <type>int</type>
      <name>calc_wcet_2</name>
      <anchorfile>sm__linear_8c.html</anchorfile>
      <anchor>ea9eed62695eedf69bc82774d7ed0792</anchor>
      <arglist>(L_Cb *cb)</arglist>
    </member>
    <member kind="function">
      <type>void</type>
      <name>find_flow_wight_2</name>
      <anchorfile>sm__linear_8c.html</anchorfile>
      <anchor>8b4c2db2f9f36df21599d4d3e68a3e87</anchor>
      <arglist>(L_Func *fn)</arglist>
    </member>
    <member kind="function">
      <type>void</type>
      <name>L_read_parm_dhasy_opti</name>
      <anchorfile>sm__linear_8c.html</anchorfile>
      <anchor>0df72eb66d62c469b065b2455c402339</anchor>
      <arglist>(Parm_Parse_Info *ppi)</arglist>
    </member>
    <member kind="function">
      <type>void</type>
      <name>SM_gen_code</name>
      <anchorfile>sm__linear_8c.html</anchorfile>
      <anchor>6499934f6bfb2effeb64c21694c0d3e8</anchor>
      <arglist>(Parm_Macro_List *external_macro_list)</arglist>
    </member>
    <member kind="variable">
      <type>int</type>
      <name>do_classic_application</name>
      <anchorfile>sm__linear_8c.html</anchorfile>
      <anchor>09b7c48d146f3754f61086e3c0993e32</anchor>
      <arglist></arglist>
    </member>
    <member kind="variable">
      <type>int</type>
      <name>do_linear_search</name>
      <anchorfile>sm__linear_8c.html</anchorfile>
      <anchor>ea42ca81e5897262f9afb4c0708404ac</anchor>
      <arglist></arglist>
    </member>
    <member kind="variable">
      <type>int</type>
      <name>do_renaming_with_copy</name>
      <anchorfile>sm__linear_8c.html</anchorfile>
      <anchor>f2589a11a9c36521d9b2020d36b548b3</anchor>
      <arglist></arglist>
    </member>
    <member kind="variable">
      <type>int</type>
      <name>do_expression_reformulation</name>
      <anchorfile>sm__linear_8c.html</anchorfile>
      <anchor>63af6339df5290f6112a90dfd6d229d8</anchor>
      <arglist></arglist>
    </member>
    <member kind="variable">
      <type>char *</type>
      <name>prof_info</name>
      <anchorfile>sm__linear_8c.html</anchorfile>
      <anchor>624cb4649f64d5ec16b959e5d68e25b8</anchor>
      <arglist></arglist>
    </member>
    <member kind="variable">
      <type>char *</type>
      <name>opti_lmdes_file_name</name>
      <anchorfile>sm__linear_8c.html</anchorfile>
      <anchor>faff693f4079bf92d396aafe711de001</anchor>
      <arglist></arglist>
    </member>
    <member kind="variable">
      <type>int</type>
      <name>do_expr_without_copy_only</name>
      <anchorfile>sm__linear_8c.html</anchorfile>
      <anchor>6c010e93ecebaf04c4061cebc2616eda</anchor>
      <arglist></arglist>
    </member>
    <member kind="variable">
      <type>int</type>
      <name>use_classic_renaming_heuristics</name>
      <anchorfile>sm__linear_8c.html</anchorfile>
      <anchor>f333fa7bd537c6502caa2a25ae5c420d</anchor>
      <arglist></arglist>
    </member>
    <member kind="variable">
      <type>Mdes *</type>
      <name>opti_lmdes</name>
      <anchorfile>sm__linear_8c.html</anchorfile>
      <anchor>dd50af71f436537b980da8aa2761329f</anchor>
      <arglist></arglist>
    </member>
    <member kind="variable">
      <type>Mdes *</type>
      <name>eval_lmdes</name>
      <anchorfile>sm__linear_8c.html</anchorfile>
      <anchor>1ff1b5efdc1fc11644ca740bb9651993</anchor>
      <arglist></arglist>
    </member>
    <member kind="variable">
      <type>int</type>
      <name>verbose_optimization</name>
      <anchorfile>sm__linear_8c.html</anchorfile>
      <anchor>d15f17c1c26160dad9b5faf8a1394872</anchor>
      <arglist></arglist>
    </member>
    <member kind="variable">
      <type>int</type>
      <name>print_cb_histogram</name>
      <anchorfile>sm__linear_8c.html</anchorfile>
      <anchor>88301ec0cefb83ef360ddb977a523668</anchor>
      <arglist></arglist>
    </member>
    <member kind="variable">
      <type>int</type>
      <name>print_top_cb_stats</name>
      <anchorfile>sm__linear_8c.html</anchorfile>
      <anchor>79824b4d2fd0c130322d05c9c0539008</anchor>
      <arglist></arglist>
    </member>
    <member kind="variable">
      <type>int</type>
      <name>print_total_stats</name>
      <anchorfile>sm__linear_8c.html</anchorfile>
      <anchor>b7f7f459923931b96d93f4e3b56d9f11</anchor>
      <arglist></arglist>
    </member>
    <member kind="variable">
      <type>int</type>
      <name>suppress_lcode_output</name>
      <anchorfile>sm__linear_8c.html</anchorfile>
      <anchor>d1c23d1e07d66124ee80af0c858ae5da</anchor>
      <arglist></arglist>
    </member>
    <member kind="variable">
      <type>int</type>
      <name>write_phd_stats</name>
      <anchorfile>sm__linear_8c.html</anchorfile>
      <anchor>8bf117902071292d80a60568f8b6c164</anchor>
      <arglist></arglist>
    </member>
    <member kind="variable">
      <type>char *</type>
      <name>phd_stats_file_name</name>
      <anchorfile>sm__linear_8c.html</anchorfile>
      <anchor>95e88e53ea87e3f26e9fa168652b1bdd</anchor>
      <arglist></arglist>
    </member>
    <member kind="variable">
      <type>int</type>
      <name>always_undo_opti</name>
      <anchorfile>sm__linear_8c.html</anchorfile>
      <anchor>285a8ab22de95d529ccc2b2540ca93c4</anchor>
      <arglist></arglist>
    </member>
    <member kind="variable">
      <type>int</type>
      <name>max_opti_passes</name>
      <anchorfile>sm__linear_8c.html</anchorfile>
      <anchor>de0b0355343b7dc11d629d7e68c8db11</anchor>
      <arglist></arglist>
    </member>
    <member kind="variable">
      <type>double</type>
      <name>min_cb_opti_weight</name>
      <anchorfile>sm__linear_8c.html</anchorfile>
      <anchor>fadf6380e21ce6b8287edc1e8164b060</anchor>
      <arglist></arglist>
    </member>
    <member kind="variable">
      <type>int</type>
      <name>opti_cb_lower_bound</name>
      <anchorfile>sm__linear_8c.html</anchorfile>
      <anchor>cbea81bf80fb27729016d410ee2af546</anchor>
      <arglist></arglist>
    </member>
    <member kind="variable">
      <type>int</type>
      <name>opti_cb_upper_bound</name>
      <anchorfile>sm__linear_8c.html</anchorfile>
      <anchor>538aad7839b716084e315f5896fbd2f9</anchor>
      <arglist></arglist>
    </member>
    <member kind="variable">
      <type>int</type>
      <name>opti_oper_lower_bound</name>
      <anchorfile>sm__linear_8c.html</anchorfile>
      <anchor>5c78ae3f11dc77c51c4972f518fb8df3</anchor>
      <arglist></arglist>
    </member>
    <member kind="variable">
      <type>int</type>
      <name>opti_oper_upper_bound</name>
      <anchorfile>sm__linear_8c.html</anchorfile>
      <anchor>a016002834bb29ef2a5ba5f941171739</anchor>
      <arglist></arglist>
    </member>
    <member kind="variable">
      <type>int</type>
      <name>phd_flags</name>
      <anchorfile>sm__linear_8c.html</anchorfile>
      <anchor>cf1bd7fe5b836ae4ded358251c29a77a</anchor>
      <arglist></arglist>
    </member>
    <member kind="variable">
      <type>int</type>
      <name>phd_performed_flags</name>
      <anchorfile>sm__linear_8c.html</anchorfile>
      <anchor>26f3dd555a7f2a3acb465aa16b6e4320</anchor>
      <arglist></arglist>
    </member>
    <member kind="variable">
      <type>double</type>
      <name>picked_cycles</name>
      <anchorfile>sm__linear_8c.html</anchorfile>
      <anchor>72728a3b6ee4a29a1982e7bfd6952722</anchor>
      <arglist></arglist>
    </member>
    <member kind="variable">
      <type>double</type>
      <name>orig_cycles</name>
      <anchorfile>sm__linear_8c.html</anchorfile>
      <anchor>d190dc0bcd819f3baad58d24400de3b9</anchor>
      <arglist></arglist>
    </member>
    <member kind="variable">
      <type>double</type>
      <name>trival_expr_cycles</name>
      <anchorfile>sm__linear_8c.html</anchorfile>
      <anchor>c256d75e42d74d68e0227117612426b4</anchor>
      <arglist></arglist>
    </member>
    <member kind="variable">
      <type>double</type>
      <name>rename_cycles</name>
      <anchorfile>sm__linear_8c.html</anchorfile>
      <anchor>fc1c04a52b836df3dc0125dc578111c4</anchor>
      <arglist></arglist>
    </member>
    <member kind="variable">
      <type>double</type>
      <name>total_cycles</name>
      <anchorfile>sm__linear_8c.html</anchorfile>
      <anchor>ae24fe2d608fbf1737ab92c0eae68460</anchor>
      <arglist></arglist>
    </member>
    <member kind="variable">
      <type>INT_Symbol_Table *</type>
      <name>diff_table</name>
      <anchorfile>sm__linear_8c.html</anchorfile>
      <anchor>ce52da226c9321666466c0098863a9b5</anchor>
      <arglist></arglist>
    </member>
    <member kind="variable">
      <type>double</type>
      <name>total_diff</name>
      <anchorfile>sm__linear_8c.html</anchorfile>
      <anchor>a11260987336be28641d01fb232d963f</anchor>
      <arglist></arglist>
    </member>
    <member kind="variable">
      <type>int</type>
      <name>diff_count</name>
      <anchorfile>sm__linear_8c.html</anchorfile>
      <anchor>09a46298959a3590bbf45c4cc7d2bd76</anchor>
      <arglist></arglist>
    </member>
    <member kind="variable">
      <type>int</type>
      <name>RWC_possible</name>
      <anchorfile>sm__linear_8c.html</anchorfile>
      <anchor>025ab0944965a552b40e83afa54bad14</anchor>
      <arglist></arglist>
    </member>
    <member kind="variable">
      <type>int</type>
      <name>EWC_possible</name>
      <anchorfile>sm__linear_8c.html</anchorfile>
      <anchor>af6fbac0b0920fc88fb978848e08a1c6</anchor>
      <arglist></arglist>
    </member>
    <member kind="variable">
      <type>time_t</type>
      <name>start_time</name>
      <anchorfile>sm__linear_8c.html</anchorfile>
      <anchor>d20e89e0c86aa812dc3a8e2fb7b5777b</anchor>
      <arglist></arglist>
    </member>
    <member kind="variable">
      <type>time_t</type>
      <name>end_time</name>
      <anchorfile>sm__linear_8c.html</anchorfile>
      <anchor>1e324d62e65642694c00de363bf8a8ba</anchor>
      <arglist></arglist>
    </member>
    <member kind="variable">
      <type>L_Alloc_Pool *</type>
      <name>Cb_Stats_pool</name>
      <anchorfile>sm__linear_8c.html</anchorfile>
      <anchor>36f837c2932d4116c55ee1bbefce3c9a</anchor>
      <arglist></arglist>
    </member>
    <member kind="variable">
      <type>Cb_Stats *</type>
      <name>cb_stats_head</name>
      <anchorfile>sm__linear_8c.html</anchorfile>
      <anchor>c5e6751ee7858f92d5b4d3003a512369</anchor>
      <arglist></arglist>
    </member>
  </compound>
  <compound kind="file">
    <name>sm_machine.c</name>
    <path>/mnt/hgfs/mkt/Desktop/openimpact/src/Lcode/sched/SM/</path>
    <filename>sm__machine_8c</filename>
    <includes id="sm_8h" name="sm.h" local="yes" imported="no">sm.h</includes>
    <member kind="function">
      <type>int</type>
      <name>S_machine_check</name>
      <anchorfile>sm__machine_8c.html</anchorfile>
      <anchor>1cf3946c60d75e862ea4795e9a19fa2f</anchor>
      <arglist>(L_Oper *oper)</arglist>
    </member>
    <member kind="function">
      <type>void</type>
      <name>S_machine_rts</name>
      <anchorfile>sm__machine_8c.html</anchorfile>
      <anchor>cec0c5f5a0d3c08aea116d6871befbe8</anchor>
      <arglist>(L_Oper *oper)</arglist>
    </member>
    <member kind="function">
      <type>void</type>
      <name>S_machine_jump</name>
      <anchorfile>sm__machine_8c.html</anchorfile>
      <anchor>3316c78b66394bf0677d38014a51c6dd</anchor>
      <arglist>(L_Oper *oper)</arglist>
    </member>
  </compound>
  <compound kind="file">
    <name>sm_main.c</name>
    <path>/mnt/hgfs/mkt/Desktop/openimpact/src/Lcode/sched/SM/</path>
    <filename>sm__main_8c</filename>
    <includes id="sm_8h" name="sm.h" local="yes" imported="no">sm.h</includes>
    <member kind="function">
      <type>void</type>
      <name>L_gen_code</name>
      <anchorfile>sm__main_8c.html</anchorfile>
      <anchor>7b172a128a2010f005d6e27fa04f503a</anchor>
      <arglist>(Parm_Macro_List *external_macro_list)</arglist>
    </member>
  </compound>
  <compound kind="file">
    <name>sm_modulo_rmap.c</name>
    <path>/mnt/hgfs/mkt/Desktop/openimpact/src/Lcode/sched/SM/</path>
    <filename>sm__modulo__rmap_8c</filename>
    <includes id="sm_8h" name="sm.h" local="yes" imported="no">sm.h</includes>
    <member kind="function">
      <type>void</type>
      <name>SM_modulo_commit_choices</name>
      <anchorfile>sm__modulo__rmap_8c.html</anchorfile>
      <anchor>4b929e038ce161dc1b502a2d613d97cd</anchor>
      <arglist>(unsigned int *map, SM_Table *table, unsigned short *choices_made, unsigned int start_offset, unsigned int offset_II)</arglist>
    </member>
    <member kind="function">
      <type>void</type>
      <name>SM_modulo_release_choices</name>
      <anchorfile>sm__modulo__rmap_8c.html</anchorfile>
      <anchor>3eb43252e87b3bf5ce8e422e7940bee9</anchor>
      <arglist>(unsigned int *map, SM_Table *table, unsigned short *choices_made, unsigned int start_offset, unsigned int offset_II)</arglist>
    </member>
    <member kind="function">
      <type>int</type>
      <name>SM_modulo_choose_first_avail_options</name>
      <anchorfile>sm__modulo__rmap_8c.html</anchorfile>
      <anchor>8cef45ed8391617be929df6b1a9e2e44</anchor>
      <arglist>(unsigned int *map, SM_Table *table, unsigned short *choices_made, unsigned int start_offset, unsigned int offset_II, unsigned int min_slot, unsigned int max_slot, Mdes_Stats *stats)</arglist>
    </member>
    <member kind="function">
      <type>int</type>
      <name>SM_modulo_choose_options_w_d_rule</name>
      <anchorfile>sm__modulo__rmap_8c.html</anchorfile>
      <anchor>0de07da000c87084611db410b4993624</anchor>
      <arglist>(SM_Oper *sm_op, unsigned int template_index, unsigned int *map, SM_Table *table, unsigned short *choices_made, unsigned int start_offset, unsigned int offset_II, unsigned int abs_slot, Mdes_Stats *stats)</arglist>
    </member>
    <member kind="function">
      <type>int</type>
      <name>SM_modulo_sched_table</name>
      <anchorfile>sm__modulo__rmap_8c.html</anchorfile>
      <anchor>f2b59ef6f57473b170cfae169fd7111a</anchor>
      <arglist>(SM_Cb *sm_cb, SM_Oper *sm_op, unsigned int template_index, SM_Table *table, unsigned short *choice_array, int time, unsigned short min_slot, unsigned short max_slot, int commit, Mdes_Stats *stats)</arglist>
    </member>
    <member kind="function">
      <type>void</type>
      <name>SM_modulo_unsched_table</name>
      <anchorfile>sm__modulo__rmap_8c.html</anchorfile>
      <anchor>75269b8c6209e22d317a8720e66aec31</anchor>
      <arglist>(SM_Cb *sm_cb, SM_Table *table, unsigned short *choice_array, int time)</arglist>
    </member>
  </compound>
  <compound kind="file">
    <name>sm_opti.c</name>
    <path>/mnt/hgfs/mkt/Desktop/openimpact/src/Lcode/sched/SM/</path>
    <filename>sm__opti_8c</filename>
    <includes id="sm_8h" name="sm.h" local="yes" imported="no">sm.h</includes>
    <member kind="define">
      <type>#define</type>
      <name>ADD_EXPR</name>
      <anchorfile>sm__opti_8c.html</anchorfile>
      <anchor>25e7c75529aeb5b51321b93a6000957a</anchor>
      <arglist></arglist>
    </member>
    <member kind="define">
      <type>#define</type>
      <name>SUB_EXPR</name>
      <anchorfile>sm__opti_8c.html</anchorfile>
      <anchor>2d7a71d45924dbfcfbfc3f21232febce</anchor>
      <arglist></arglist>
    </member>
    <member kind="function">
      <type>void</type>
      <name>SM_print_expr_with_copy</name>
      <anchorfile>sm__opti_8c.html</anchorfile>
      <anchor>521754999faef75b265d9bed89cb57b7</anchor>
      <arglist>(FILE *out, SM_Trans *trans)</arglist>
    </member>
    <member kind="function">
      <type>void</type>
      <name>SM_print_minimal_lcode_op</name>
      <anchorfile>sm__opti_8c.html</anchorfile>
      <anchor>274d1d7eca80c9eff01fc92102395bc0</anchor>
      <arglist>(FILE *out, SM_Oper *sm_op)</arglist>
    </member>
    <member kind="function">
      <type>SM_Trans *</type>
      <name>SM_new_trans</name>
      <anchorfile>sm__opti_8c.html</anchorfile>
      <anchor>b5e0db0d75fbeb826aa73a58e9d860a8</anchor>
      <arglist>(int type, SM_Oper *target_sm_op, int flags)</arglist>
    </member>
    <member kind="function">
      <type>void</type>
      <name>SM_delete_trans</name>
      <anchorfile>sm__opti_8c.html</anchorfile>
      <anchor>fae4b09eb1233f646c1f455c91ada5ac</anchor>
      <arglist>(SM_Trans *trans)</arglist>
    </member>
    <member kind="function">
      <type>void</type>
      <name>SM_print_trans</name>
      <anchorfile>sm__opti_8c.html</anchorfile>
      <anchor>68aa44ad1b9f901601d7216e6b3dd6ab</anchor>
      <arglist>(FILE *out, SM_Trans *sm_trans)</arglist>
    </member>
    <member kind="function">
      <type>int</type>
      <name>SM_CRC_defs_before_or_selected</name>
      <anchorfile>sm__opti_8c.html</anchorfile>
      <anchor>161aaa610b541162885eac42629cc923</anchor>
      <arglist>(SM_Oper *sm_op, Set bef_set, List sel_list)</arglist>
    </member>
    <member kind="function">
      <type>int</type>
      <name>SM_CRC_long_lat_between</name>
      <anchorfile>sm__opti_8c.html</anchorfile>
      <anchor>78ab9620b3280a825006bf156e25f015</anchor>
      <arglist>(SM_Oper *sm_op, SM_Oper *dep_op)</arglist>
    </member>
    <member kind="function">
      <type>void</type>
      <name>SM_CRC_add_valid_op</name>
      <anchorfile>sm__opti_8c.html</anchorfile>
      <anchor>eda0b9ca8c53f08071a1694eee0e0213</anchor>
      <arglist>(SM_Oper *sm_op, List *work_list, Set bef_set, List sel_list)</arglist>
    </member>
    <member kind="function">
      <type>SM_Trans *</type>
      <name>SM_can_relocate_cond</name>
      <anchorfile>sm__opti_8c.html</anchorfile>
      <anchor>106fdb10f22d16c3cd2e72ccc2186582</anchor>
      <arglist>(SM_Oper *sm_op)</arglist>
    </member>
    <member kind="function">
      <type>void</type>
      <name>SM_do_relocate_cond_opti</name>
      <anchorfile>sm__opti_8c.html</anchorfile>
      <anchor>449a6d51faabc75002d5fda7c50d6427</anchor>
      <arglist>(SM_Cb *sm_cb)</arglist>
    </member>
    <member kind="function">
      <type>SM_Trans *</type>
      <name>SM_can_rename_with_copy</name>
      <anchorfile>sm__opti_8c.html</anchorfile>
      <anchor>fa4265d0ab00ebd1ae3df7dad0b6db92</anchor>
      <arglist>(SM_Oper *sm_op)</arglist>
    </member>
    <member kind="function">
      <type>int</type>
      <name>SM_expr_srcs_available</name>
      <anchorfile>sm__opti_8c.html</anchorfile>
      <anchor>9bafe7acc10364b553037620a5a94a22</anchor>
      <arglist>(SM_Oper *from_op, SM_Oper *to_op)</arglist>
    </member>
    <member kind="function">
      <type>int</type>
      <name>SM_can_move_before_def</name>
      <anchorfile>sm__opti_8c.html</anchorfile>
      <anchor>2c6d91782d6df964b7ca61b9e6209fda</anchor>
      <arglist>(SM_Oper *def_op, SM_Oper *use_op, SM_Reg_Action *src)</arglist>
    </member>
    <member kind="function">
      <type>int</type>
      <name>SM_can_move_before_def_with_renaming</name>
      <anchorfile>sm__opti_8c.html</anchorfile>
      <anchor>7976bde53c18d50922ca130fd74d8060</anchor>
      <arglist>(SM_Oper *def_op, SM_Oper *use_op, SM_Reg_Action *src)</arglist>
    </member>
    <member kind="function">
      <type>int</type>
      <name>SM_int_field_large_enough</name>
      <anchorfile>sm__opti_8c.html</anchorfile>
      <anchor>34b036dac242316a9bdd09435a6d8c90</anchor>
      <arglist>(int opc, int proc_opc, ITintmax int_val)</arglist>
    </member>
    <member kind="function">
      <type>int</type>
      <name>SM_is_valid_expr_use</name>
      <anchorfile>sm__opti_8c.html</anchorfile>
      <anchor>58e3bda753b132f77d176b1b232f1d6f</anchor>
      <arglist>(SM_Oper *sm_op)</arglist>
    </member>
    <member kind="function">
      <type>int</type>
      <name>SM_is_valid_expr_def</name>
      <anchorfile>sm__opti_8c.html</anchorfile>
      <anchor>109682f64e2937e8ca3857acf488c1f7</anchor>
      <arglist>(SM_Oper *sm_op)</arglist>
    </member>
    <member kind="function">
      <type>SM_Trans *</type>
      <name>SM_can_expr_with_copy</name>
      <anchorfile>sm__opti_8c.html</anchorfile>
      <anchor>8378f3e995b61b32e71a33f8b2ff7fbc</anchor>
      <arglist>(SM_Oper *sm_op)</arglist>
    </member>
    <member kind="function">
      <type>void</type>
      <name>SM_do_trivial_renaming</name>
      <anchorfile>sm__opti_8c.html</anchorfile>
      <anchor>1cdc24757dd63ce057887c55f3dd15b9</anchor>
      <arglist>(SM_Cb *sm_cb)</arglist>
    </member>
    <member kind="function">
      <type>void</type>
      <name>SM_do_renaming_with_copy</name>
      <anchorfile>sm__opti_8c.html</anchorfile>
      <anchor>902d8feb6a2a317b3f091ec6bd63504e</anchor>
      <arglist>(SM_Trans *sm_trans)</arglist>
    </member>
    <member kind="function">
      <type>void</type>
      <name>SM_undo_renaming_with_copy</name>
      <anchorfile>sm__opti_8c.html</anchorfile>
      <anchor>9bd3f8bfc0af04bbd5e1c525d23e67d1</anchor>
      <arglist>(SM_Trans *sm_trans)</arglist>
    </member>
    <member kind="function">
      <type>void</type>
      <name>SM_set_ext</name>
      <anchorfile>sm__opti_8c.html</anchorfile>
      <anchor>0701d1d7a7ab77287c599f0f6bdd4836</anchor>
      <arglist>(SM_Oper *sm_op, ITintmax ext)</arglist>
    </member>
    <member kind="function">
      <type>ITintmax</type>
      <name>SM_get_ext</name>
      <anchorfile>sm__opti_8c.html</anchorfile>
      <anchor>bb61423e9a1ffb10bd49eacd77e30fec</anchor>
      <arglist>(SM_Oper *sm_op)</arglist>
    </member>
    <member kind="function">
      <type>void</type>
      <name>SM_reverse_branch_opcode</name>
      <anchorfile>sm__opti_8c.html</anchorfile>
      <anchor>5d31997328aa67c7e10a6e3b60792831</anchor>
      <arglist>(int *opc, int *proc_opc)</arglist>
    </member>
    <member kind="function">
      <type>void</type>
      <name>SM_make_valid_mcode</name>
      <anchorfile>sm__opti_8c.html</anchorfile>
      <anchor>02df7b2532af9e91760992699c82a9a3</anchor>
      <arglist>(int *opc, int *proc_opc, L_Operand **src0, L_Operand **src1, int *ext)</arglist>
    </member>
    <member kind="function">
      <type>int</type>
      <name>SM_def_has_exactly_one_use</name>
      <anchorfile>sm__opti_8c.html</anchorfile>
      <anchor>c37f54f141f4d55fe31215053b7f8bce</anchor>
      <arglist>(SM_Reg_Action *def_action)</arglist>
    </member>
    <member kind="function">
      <type>void</type>
      <name>SM_do_expr_with_copy</name>
      <anchorfile>sm__opti_8c.html</anchorfile>
      <anchor>0bcb2f99b3ddf0099265af210c63f306</anchor>
      <arglist>(SM_Trans *trans)</arglist>
    </member>
    <member kind="function">
      <type>int</type>
      <name>SM_can_undo_expr_with_copy</name>
      <anchorfile>sm__opti_8c.html</anchorfile>
      <anchor>c9ce0e8c7a5c37e6b261292ab588a0f2</anchor>
      <arglist>(SM_Trans *trans)</arglist>
    </member>
    <member kind="function">
      <type>void</type>
      <name>SM_undo_expr_with_copy</name>
      <anchorfile>sm__opti_8c.html</anchorfile>
      <anchor>790d59ad0c63a1cbcf9b4eccfb84c057</anchor>
      <arglist>(SM_Trans *trans)</arglist>
    </member>
    <member kind="function">
      <type>void</type>
      <name>SM_do_trans</name>
      <anchorfile>sm__opti_8c.html</anchorfile>
      <anchor>5165b5e17f693a2f5067d7981267762c</anchor>
      <arglist>(SM_Trans *trans)</arglist>
    </member>
    <member kind="function">
      <type>void</type>
      <name>SM_undo_trans</name>
      <anchorfile>sm__opti_8c.html</anchorfile>
      <anchor>3c9ec3752ff0eec20efd909a2ea4fa3a</anchor>
      <arglist>(SM_Trans *trans)</arglist>
    </member>
    <member kind="function">
      <type>int</type>
      <name>SM_can_undo_trans</name>
      <anchorfile>sm__opti_8c.html</anchorfile>
      <anchor>15baf68b27c5793df755db1d9a362604</anchor>
      <arglist>(SM_Trans *trans)</arglist>
    </member>
    <member kind="function">
      <type>SM_Trans_Queue *</type>
      <name>SM_find_potential_trans</name>
      <anchorfile>sm__opti_8c.html</anchorfile>
      <anchor>b7775bfb97ca2f7206879f5a7b9c42a5</anchor>
      <arglist>(SM_Cb *sm_cb, unsigned int allowed_trans)</arglist>
    </member>
    <member kind="function">
      <type>void</type>
      <name>SM_delete_trans_and_queue</name>
      <anchorfile>sm__opti_8c.html</anchorfile>
      <anchor>ff666504a1ac4d18f6088328f9181b7d</anchor>
      <arglist>(SM_Trans_Queue *queue)</arglist>
    </member>
    <member kind="function">
      <type>double</type>
      <name>SM_calc_do_trans_priority</name>
      <anchorfile>sm__opti_8c.html</anchorfile>
      <anchor>6317d4b6f7fff3a1de568c4f3e4d24e9</anchor>
      <arglist>(SM_Trans *trans, int max_late_time)</arglist>
    </member>
    <member kind="function">
      <type>void</type>
      <name>SM_update_sched_based_expr_info</name>
      <anchorfile>sm__opti_8c.html</anchorfile>
      <anchor>7ba5b4ba3c3ff0c325108906f013c3fe</anchor>
      <arglist>(SM_Trans *trans)</arglist>
    </member>
    <member kind="function">
      <type>void</type>
      <name>SM_update_sched_based_trans_info</name>
      <anchorfile>sm__opti_8c.html</anchorfile>
      <anchor>fd617cf2294c68cdd91fe7abd65bc753</anchor>
      <arglist>(SM_Trans_Queue *queue)</arglist>
    </member>
    <member kind="function">
      <type>void</type>
      <name>SM_update_dep_based_trans_info</name>
      <anchorfile>sm__opti_8c.html</anchorfile>
      <anchor>dc6102a5bf629400334b67a07198f4ed</anchor>
      <arglist>(SM_Trans_Queue *queue)</arglist>
    </member>
    <member kind="function">
      <type>SM_Trans *</type>
      <name>SM_dequeue_best_do_trans</name>
      <anchorfile>sm__opti_8c.html</anchorfile>
      <anchor>fd986dfcdffb3fe41a2b255363bb3c4c</anchor>
      <arglist>(SM_Trans_Queue *queue)</arglist>
    </member>
    <member kind="function">
      <type>int</type>
      <name>SM_calc_action_avail_time</name>
      <anchorfile>sm__opti_8c.html</anchorfile>
      <anchor>dc67f87f50278c4b0fe8415bab0a699b</anchor>
      <arglist>(SM_Reg_Action *action)</arglist>
    </member>
    <member kind="variable">
      <type>int</type>
      <name>opti_oper_lower_bound</name>
      <anchorfile>sm__opti_8c.html</anchorfile>
      <anchor>5c78ae3f11dc77c51c4972f518fb8df3</anchor>
      <arglist></arglist>
    </member>
    <member kind="variable">
      <type>int</type>
      <name>opti_oper_upper_bound</name>
      <anchorfile>sm__opti_8c.html</anchorfile>
      <anchor>a016002834bb29ef2a5ba5f941171739</anchor>
      <arglist></arglist>
    </member>
    <member kind="variable">
      <type>int</type>
      <name>verbose_optimization</name>
      <anchorfile>sm__opti_8c.html</anchorfile>
      <anchor>d15f17c1c26160dad9b5faf8a1394872</anchor>
      <arglist></arglist>
    </member>
    <member kind="variable">
      <type>int</type>
      <name>do_expr_without_copy_only</name>
      <anchorfile>sm__opti_8c.html</anchorfile>
      <anchor>6c010e93ecebaf04c4061cebc2616eda</anchor>
      <arglist></arglist>
    </member>
    <member kind="variable">
      <type>int</type>
      <name>use_classic_renaming_heuristics</name>
      <anchorfile>sm__opti_8c.html</anchorfile>
      <anchor>f333fa7bd537c6502caa2a25ae5c420d</anchor>
      <arglist></arglist>
    </member>
    <member kind="variable">
      <type>L_Alloc_Pool *</type>
      <name>SM_Oper_pool</name>
      <anchorfile>sm__opti_8c.html</anchorfile>
      <anchor>ed27c056b3105b7cc9340854ec1c0b95</anchor>
      <arglist></arglist>
    </member>
    <member kind="variable">
      <type>L_Alloc_Pool *</type>
      <name>SM_Trans_pool</name>
      <anchorfile>sm__opti_8c.html</anchorfile>
      <anchor>830a3d222b7438f0cb24be9090561d1b</anchor>
      <arglist></arglist>
    </member>
  </compound>
  <compound kind="file">
    <name>sm_priority.c</name>
    <path>/mnt/hgfs/mkt/Desktop/openimpact/src/Lcode/sched/SM/</path>
    <filename>sm__priority_8c</filename>
    <includes id="sm_8h" name="sm.h" local="yes" imported="no">sm.h</includes>
    <member kind="function">
      <type>void</type>
      <name>SM_calculate_priorities</name>
      <anchorfile>sm__priority_8c.html</anchorfile>
      <anchor>130cf52b2f5ee837159fb59182b1baaa</anchor>
      <arglist>(SM_Cb *sm_cb, int min_early_time)</arglist>
    </member>
    <member kind="function">
      <type>int</type>
      <name>SM_calculate_early_times</name>
      <anchorfile>sm__priority_8c.html</anchorfile>
      <anchor>99799486dc342e24126a6d7c316edf0b</anchor>
      <arglist>(SM_Cb *sm_cb, int min_early_time)</arglist>
    </member>
    <member kind="function">
      <type>int</type>
      <name>SM_recalculate_early_time</name>
      <anchorfile>sm__priority_8c.html</anchorfile>
      <anchor>b0691af602f049d3165a9dc22435d31f</anchor>
      <arglist>(SM_Oper *sm_op, int min_early_time)</arglist>
    </member>
    <member kind="function">
      <type>void</type>
      <name>SM_calculate_late_times</name>
      <anchorfile>sm__priority_8c.html</anchorfile>
      <anchor>c8b7ef12dd37191f740b481a02016148</anchor>
      <arglist>(SM_Cb *sm_cb, int max_late_time)</arglist>
    </member>
    <member kind="function">
      <type>void</type>
      <name>SM_print_early_and_late_times</name>
      <anchorfile>sm__priority_8c.html</anchorfile>
      <anchor>6e36dac40de39efc4957c22f0aafcbc9</anchor>
      <arglist>(FILE *out, SM_Cb *sm_cb)</arglist>
    </member>
  </compound>
  <compound kind="file">
    <name>sm_queue.c</name>
    <path>/mnt/hgfs/mkt/Desktop/openimpact/src/Lcode/sched/SM/</path>
    <filename>sm__queue_8c</filename>
    <includes id="sm_8h" name="sm.h" local="yes" imported="no">sm.h</includes>
    <member kind="function">
      <type>SM_Oper_Queue *</type>
      <name>SM_new_oper_queue</name>
      <anchorfile>sm__queue_8c.html</anchorfile>
      <anchor>f72bd28d77797baf55f2e6b0af3a8456</anchor>
      <arglist>()</arglist>
    </member>
    <member kind="function">
      <type>void</type>
      <name>SM_delete_oper_queue</name>
      <anchorfile>sm__queue_8c.html</anchorfile>
      <anchor>0b8d645214a07f21c6c97ce066eec6b5</anchor>
      <arglist>(SM_Oper_Queue *queue)</arglist>
    </member>
    <member kind="function">
      <type>SM_Oper_Qentry *</type>
      <name>SM_enqueue_oper_before</name>
      <anchorfile>sm__queue_8c.html</anchorfile>
      <anchor>80620884311427f1817ee1b8f9c9a4c4</anchor>
      <arglist>(SM_Oper_Queue *queue, SM_Oper *sm_op, SM_Oper_Qentry *before_qentry)</arglist>
    </member>
    <member kind="function">
      <type>SM_Oper_Qentry *</type>
      <name>SM_enqueue_priority</name>
      <anchorfile>sm__queue_8c.html</anchorfile>
      <anchor>90cc85cd5ff3749ab8bd9fff611a1a52</anchor>
      <arglist>(SM_Oper_Queue *queue, SM_Oper *sm_op)</arglist>
    </member>
    <member kind="function">
      <type>SM_Oper_Qentry *</type>
      <name>SM_enqueue_oper_after</name>
      <anchorfile>sm__queue_8c.html</anchorfile>
      <anchor>2c5f0266ce44105027317e9d3d08102d</anchor>
      <arglist>(SM_Oper_Queue *queue, SM_Oper *sm_op, SM_Oper_Qentry *after_qentry)</arglist>
    </member>
    <member kind="function">
      <type>void</type>
      <name>SM_dequeue_oper</name>
      <anchorfile>sm__queue_8c.html</anchorfile>
      <anchor>181d88a02e26e6a27dfd9b4a64cfefe0</anchor>
      <arglist>(SM_Oper_Qentry *qentry)</arglist>
    </member>
    <member kind="function">
      <type>void</type>
      <name>SM_dequeue_oper_from_all</name>
      <anchorfile>sm__queue_8c.html</anchorfile>
      <anchor>185ebdb2a0ee196bf327cca3ac4725ce</anchor>
      <arglist>(SM_Oper *sm_op)</arglist>
    </member>
    <member kind="function">
      <type>void</type>
      <name>SM_print_oper_queue</name>
      <anchorfile>sm__queue_8c.html</anchorfile>
      <anchor>330894fc4899ed745e8047e6586d13d8</anchor>
      <arglist>(FILE *out, SM_Oper_Queue *queue)</arglist>
    </member>
    <member kind="function">
      <type>SM_Action_Queue *</type>
      <name>SM_new_action_queue</name>
      <anchorfile>sm__queue_8c.html</anchorfile>
      <anchor>4690c78915658abbb7a0977a668449a4</anchor>
      <arglist>()</arglist>
    </member>
    <member kind="function">
      <type>void</type>
      <name>SM_delete_action_queue</name>
      <anchorfile>sm__queue_8c.html</anchorfile>
      <anchor>053a8c194a1a77f544a4bf8e535c25a8</anchor>
      <arglist>(SM_Action_Queue *queue)</arglist>
    </member>
    <member kind="function">
      <type>SM_Action_Qentry *</type>
      <name>SM_enqueue_action_before</name>
      <anchorfile>sm__queue_8c.html</anchorfile>
      <anchor>8644246b75bb40b05e01e7bd1910b9d0</anchor>
      <arglist>(SM_Action_Queue *queue, SM_Reg_Action *action, SM_Action_Qentry *before_qentry)</arglist>
    </member>
    <member kind="function">
      <type>SM_Action_Qentry *</type>
      <name>SM_enqueue_action_after</name>
      <anchorfile>sm__queue_8c.html</anchorfile>
      <anchor>b4c92a7098e53640e5e5c61b979986e5</anchor>
      <arglist>(SM_Action_Queue *queue, SM_Reg_Action *action, SM_Action_Qentry *after_qentry)</arglist>
    </member>
    <member kind="function">
      <type>void</type>
      <name>SM_dequeue_action</name>
      <anchorfile>sm__queue_8c.html</anchorfile>
      <anchor>585b3321bd29443a433c054f40694be1</anchor>
      <arglist>(SM_Action_Qentry *qentry)</arglist>
    </member>
    <member kind="function">
      <type>void</type>
      <name>SM_dequeue_action_from_all</name>
      <anchorfile>sm__queue_8c.html</anchorfile>
      <anchor>8025903102084a8a65ec619841a83f48</anchor>
      <arglist>(SM_Reg_Action *action)</arglist>
    </member>
    <member kind="function">
      <type>void</type>
      <name>SM_print_action_queue</name>
      <anchorfile>sm__queue_8c.html</anchorfile>
      <anchor>a8294d51b74b62b933e89def637c61a2</anchor>
      <arglist>(FILE *out, SM_Action_Queue *queue)</arglist>
    </member>
    <member kind="function">
      <type>SM_Trans_Queue *</type>
      <name>SM_new_trans_queue</name>
      <anchorfile>sm__queue_8c.html</anchorfile>
      <anchor>dbb9a3fff6b69ef787e1c3219d8ec572</anchor>
      <arglist>(char *name)</arglist>
    </member>
    <member kind="function">
      <type>void</type>
      <name>SM_delete_trans_queue</name>
      <anchorfile>sm__queue_8c.html</anchorfile>
      <anchor>a0d62219e0aabccc40cfcd439882e0e2</anchor>
      <arglist>(SM_Trans_Queue *queue)</arglist>
    </member>
    <member kind="function">
      <type>SM_Trans_Qentry *</type>
      <name>SM_enqueue_trans_before</name>
      <anchorfile>sm__queue_8c.html</anchorfile>
      <anchor>2e6e0b6b9a944234a75e58ebcba86fd6</anchor>
      <arglist>(SM_Trans_Queue *queue, SM_Trans *trans, SM_Trans_Qentry *before_qentry)</arglist>
    </member>
    <member kind="function">
      <type>SM_Trans_Qentry *</type>
      <name>SM_enqueue_trans_after</name>
      <anchorfile>sm__queue_8c.html</anchorfile>
      <anchor>d5e38b215817c0dc12980ab8262f9cc5</anchor>
      <arglist>(SM_Trans_Queue *queue, SM_Trans *trans, SM_Trans_Qentry *after_qentry)</arglist>
    </member>
    <member kind="function">
      <type>void</type>
      <name>SM_dequeue_trans</name>
      <anchorfile>sm__queue_8c.html</anchorfile>
      <anchor>41ad842f52933206658da8dc572879f6</anchor>
      <arglist>(SM_Trans_Qentry *qentry)</arglist>
    </member>
    <member kind="function">
      <type>void</type>
      <name>SM_dequeue_trans_from_all</name>
      <anchorfile>sm__queue_8c.html</anchorfile>
      <anchor>2e71bd06304b51f1b9c1a05a8a64d31d</anchor>
      <arglist>(SM_Trans *trans)</arglist>
    </member>
    <member kind="function">
      <type>void</type>
      <name>SM_print_trans_queue</name>
      <anchorfile>sm__queue_8c.html</anchorfile>
      <anchor>9efbab28fa13249f30565a57c4dbabad</anchor>
      <arglist>(FILE *out, SM_Trans_Queue *queue)</arglist>
    </member>
    <member kind="function">
      <type>SM_Priority_Queue *</type>
      <name>SM_new_priority_queue</name>
      <anchorfile>sm__queue_8c.html</anchorfile>
      <anchor>e297187b3c57aca27b3cd31dfaccb3c4</anchor>
      <arglist>()</arglist>
    </member>
    <member kind="function">
      <type>void</type>
      <name>SM_reinit_priority_queue</name>
      <anchorfile>sm__queue_8c.html</anchorfile>
      <anchor>295f5616041fb08003456a44d08b9f22</anchor>
      <arglist>(SM_Priority_Queue *queue)</arglist>
    </member>
    <member kind="function">
      <type>void</type>
      <name>SM_delete_priority_queue</name>
      <anchorfile>sm__queue_8c.html</anchorfile>
      <anchor>adb4243f389d099d6fe65671db426577</anchor>
      <arglist>(SM_Priority_Queue *queue)</arglist>
    </member>
    <member kind="function">
      <type>void</type>
      <name>SM_enqueue_increasing_priority</name>
      <anchorfile>sm__queue_8c.html</anchorfile>
      <anchor>8158df004c2bb8df2e6295e737111949</anchor>
      <arglist>(SM_Priority_Queue *queue, SM_Oper *oper, int priority)</arglist>
    </member>
    <member kind="function">
      <type>SM_Oper *</type>
      <name>SM_dequeue_priority</name>
      <anchorfile>sm__queue_8c.html</anchorfile>
      <anchor>fb9bf29cc14c0d06a9f6004737f8f6d1</anchor>
      <arglist>(SM_Priority_Queue *queue)</arglist>
    </member>
    <member kind="function">
      <type>SM_Oper *</type>
      <name>SM_peek_end</name>
      <anchorfile>sm__queue_8c.html</anchorfile>
      <anchor>a406afcce283a652018017f4c40dc96e</anchor>
      <arglist>(SM_Priority_Queue *queue)</arglist>
    </member>
    <member kind="function">
      <type>SM_Oper *</type>
      <name>SM_peek_head</name>
      <anchorfile>sm__queue_8c.html</anchorfile>
      <anchor>6c619eb17080295b9d91b0e4156034fb</anchor>
      <arglist>(SM_Priority_Queue *queue)</arglist>
    </member>
    <member kind="variable">
      <type>L_Alloc_Pool *</type>
      <name>SM_Oper_Queue_pool</name>
      <anchorfile>sm__queue_8c.html</anchorfile>
      <anchor>642915ffa15fe72eba80fcdaa580a790</anchor>
      <arglist></arglist>
    </member>
    <member kind="variable">
      <type>L_Alloc_Pool *</type>
      <name>SM_Oper_Qentry_pool</name>
      <anchorfile>sm__queue_8c.html</anchorfile>
      <anchor>25c4a6633e5b6c97a582aebce9089d9c</anchor>
      <arglist></arglist>
    </member>
    <member kind="variable">
      <type>L_Alloc_Pool *</type>
      <name>SM_Action_Queue_pool</name>
      <anchorfile>sm__queue_8c.html</anchorfile>
      <anchor>65f1fab19e660426f050c91409a1c504</anchor>
      <arglist></arglist>
    </member>
    <member kind="variable">
      <type>L_Alloc_Pool *</type>
      <name>SM_Action_Qentry_pool</name>
      <anchorfile>sm__queue_8c.html</anchorfile>
      <anchor>730ea808f5c245cc608b2680ae2d722c</anchor>
      <arglist></arglist>
    </member>
    <member kind="variable">
      <type>L_Alloc_Pool *</type>
      <name>SM_Trans_Queue_pool</name>
      <anchorfile>sm__queue_8c.html</anchorfile>
      <anchor>9c45950817ebbf5ee1bb2dbc5991a15f</anchor>
      <arglist></arglist>
    </member>
    <member kind="variable">
      <type>L_Alloc_Pool *</type>
      <name>SM_Trans_Qentry_pool</name>
      <anchorfile>sm__queue_8c.html</anchorfile>
      <anchor>309bb3a3a4cf33d209f7b7d72f391e1c</anchor>
      <arglist></arglist>
    </member>
    <member kind="variable">
      <type>L_Alloc_Pool *</type>
      <name>SM_Priority_Queue_pool</name>
      <anchorfile>sm__queue_8c.html</anchorfile>
      <anchor>8bdc1d4ae109504ae7e48943681103fb</anchor>
      <arglist></arglist>
    </member>
    <member kind="variable">
      <type>L_Alloc_Pool *</type>
      <name>SM_Priority_Qentry_pool</name>
      <anchorfile>sm__queue_8c.html</anchorfile>
      <anchor>af366ac7278ce98e19580665340b4e92</anchor>
      <arglist></arglist>
    </member>
  </compound>
  <compound kind="file">
    <name>sm_recovery.c</name>
    <path>/mnt/hgfs/mkt/Desktop/openimpact/src/Lcode/sched/SM/</path>
    <filename>sm__recovery_8c</filename>
  </compound>
  <compound kind="file">
    <name>sm_recovery.h</name>
    <path>/mnt/hgfs/mkt/Desktop/openimpact/src/Lcode/sched/SM/</path>
    <filename>sm__recovery_8h</filename>
    <class kind="struct">RC_dep_info</class>
    <class kind="struct">RC_cb_info</class>
    <class kind="struct">RC_flow_info</class>
    <member kind="define">
      <type>#define</type>
      <name>RC_MAX_CHK</name>
      <anchorfile>sm__recovery_8h.html</anchorfile>
      <anchor>2b169002dd530b569472cd30fbf1e62a</anchor>
      <arglist></arglist>
    </member>
    <member kind="define">
      <type>#define</type>
      <name>RC_CB_INFO</name>
      <anchorfile>sm__recovery_8h.html</anchorfile>
      <anchor>6ca66e88f0b1b6930ee69b7325806ab5</anchor>
      <arglist>(s)</arglist>
    </member>
    <member kind="define">
      <type>#define</type>
      <name>RC_FLOW_INFO</name>
      <anchorfile>sm__recovery_8h.html</anchorfile>
      <anchor>56649961406fbef41769c39672f811c1</anchor>
      <arglist>(s)</arglist>
    </member>
    <member kind="enumeration">
      <name>rc_kind</name>
      <anchor>589f80e15270c5e734d4153b6b644e25</anchor>
      <arglist></arglist>
    </member>
    <member kind="enumvalue">
      <name>rc_notrc</name>
      <anchor>589f80e15270c5e734d4153b6b644e25ac017473b12c7c4e7f8da180b9b8eaab</anchor>
      <arglist></arglist>
    </member>
    <member kind="enumvalue">
      <name>rc_isrc</name>
      <anchor>589f80e15270c5e734d4153b6b644e25977d5b3e3fdc38aa165f1dba76d58153</anchor>
      <arglist></arglist>
    </member>
    <member kind="enumvalue">
      <name>rc_total</name>
      <anchor>589f80e15270c5e734d4153b6b644e2548178183d8d7a605fbc8a88539fc7ab7</anchor>
      <arglist></arglist>
    </member>
    <member kind="enumvalue">
      <name>rc_last</name>
      <anchor>589f80e15270c5e734d4153b6b644e25707daf039dfe6952bd4abecf54578c19</anchor>
      <arglist></arglist>
    </member>
    <member kind="function">
      <type>List</type>
      <name>RC_oprd_remove_from_list</name>
      <anchorfile>sm__recovery_8h.html</anchorfile>
      <anchor>c3fd1925bb9ff20817dde62525e4b504</anchor>
      <arglist>(List oprd_list, L_Operand *oprd)</arglist>
    </member>
    <member kind="function">
      <type>int</type>
      <name>RC_oprd_in_list</name>
      <anchorfile>sm__recovery_8h.html</anchorfile>
      <anchor>914971091e2d4b39ef8f73b8d2f0d888</anchor>
      <arglist>(List oprd_list, L_Operand *oprd)</arglist>
    </member>
    <member kind="function">
      <type>int</type>
      <name>RC_check_preserves_oprd</name>
      <anchorfile>sm__recovery_8h.html</anchorfile>
      <anchor>2a2d3642faee49148140829ed18f4e9c</anchor>
      <arglist>(L_Oper *chk_op, L_Operand *oprd)</arglist>
    </member>
    <member kind="function">
      <type>void</type>
      <name>RC_add_flow_for_op</name>
      <anchorfile>sm__recovery_8h.html</anchorfile>
      <anchor>c12a0045de5cb3b6d838902498e84662</anchor>
      <arglist>(L_Cb *cb, L_Oper *op)</arglist>
    </member>
    <member kind="function">
      <type>void</type>
      <name>RC_move_dest_flow_after</name>
      <anchorfile>sm__recovery_8h.html</anchorfile>
      <anchor>57f0b5bcde3600ecb4caab1da314b8af</anchor>
      <arglist>(L_Cb *from_cb, L_Flow *dst_flow, L_Cb *to_cb, L_Flow *to_after_flow)</arglist>
    </member>
    <member kind="function">
      <type>L_Flow *</type>
      <name>RC_move_op_after</name>
      <anchorfile>sm__recovery_8h.html</anchorfile>
      <anchor>5d65f3cb669bda04174a64ea717d5cc3</anchor>
      <arglist>(L_Cb *from_cb, L_Oper *op, L_Cb *to_cb, L_Oper *to_after_op)</arglist>
    </member>
    <member kind="function">
      <type>L_Cb *</type>
      <name>RC_split_cb_after</name>
      <anchorfile>sm__recovery_8h.html</anchorfile>
      <anchor>1222086e9d40f6c89d0465a9be0bf390</anchor>
      <arglist>(L_Func *fn, L_Cb *cb, L_Oper *op)</arglist>
    </member>
    <member kind="function">
      <type>L_Oper *</type>
      <name>RC_copy_op</name>
      <anchorfile>sm__recovery_8h.html</anchorfile>
      <anchor>602cc2623c6cf0a48440eb2b582ae650</anchor>
      <arglist>(L_Oper *op, List *oprd_list)</arglist>
    </member>
    <member kind="function">
      <type>void</type>
      <name>RC_dump_lcode</name>
      <anchorfile>sm__recovery_8h.html</anchorfile>
      <anchor>504496ee4d08a5e18b346f3aa4f56dce</anchor>
      <arglist>(L_Func *fn, char *ext)</arglist>
    </member>
    <member kind="function">
      <type>void</type>
      <name>RC_dump_lcode_cb</name>
      <anchorfile>sm__recovery_8h.html</anchorfile>
      <anchor>9787db41911f31a4e26ac328564b36ee</anchor>
      <arglist>(L_Func *fn, L_Cb *cb, char *ext)</arglist>
    </member>
    <member kind="function">
      <type>void</type>
      <name>RC_delete_all_checks</name>
      <anchorfile>sm__recovery_8h.html</anchorfile>
      <anchor>5f45b1d8a96aa2f6c4ac8b5f94ae6011</anchor>
      <arglist>(L_Func *fn)</arglist>
    </member>
    <member kind="function">
      <type>void</type>
      <name>RC_insert_antidep_defines</name>
      <anchorfile>sm__recovery_8h.html</anchorfile>
      <anchor>621b240fb3448cba29008e17929f7cce</anchor>
      <arglist>(L_Func *fn)</arglist>
    </member>
    <member kind="function">
      <type>void</type>
      <name>RC_delete_antidep_defines</name>
      <anchorfile>sm__recovery_8h.html</anchorfile>
      <anchor>f4a0b33298f09a1179a029eba378b980</anchor>
      <arglist>(L_Func *fn)</arglist>
    </member>
    <member kind="function">
      <type>RC_flow_info *</type>
      <name>RC_new_flow_info</name>
      <anchorfile>sm__recovery_8h.html</anchorfile>
      <anchor>30525d469f9a3a6aa6e718299551b0f4</anchor>
      <arglist>()</arglist>
    </member>
    <member kind="function">
      <type>void</type>
      <name>RC_delete_flow_info</name>
      <anchorfile>sm__recovery_8h.html</anchorfile>
      <anchor>943eb914db5da32ae37a98ed8e9c17ed</anchor>
      <arglist>(L_Oper *op)</arglist>
    </member>
    <member kind="function">
      <type>void</type>
      <name>RC_init</name>
      <anchorfile>sm__recovery_8h.html</anchorfile>
      <anchor>7355afd40ddf0d069961b54511ec6879</anchor>
      <arglist>(L_Func *fn)</arglist>
    </member>
    <member kind="function">
      <type>void</type>
      <name>RC_cleanup</name>
      <anchorfile>sm__recovery_8h.html</anchorfile>
      <anchor>5fed029e4ba70e28c118c5b7d8cce282</anchor>
      <arglist>(L_Func *fn)</arglist>
    </member>
    <member kind="function">
      <type>void</type>
      <name>RC_gather_stats</name>
      <anchorfile>sm__recovery_8h.html</anchorfile>
      <anchor>3f2d596b968b914ce613aa9bc78bdfa1</anchor>
      <arglist>(L_Func *fn, List lds_list)</arglist>
    </member>
    <member kind="function">
      <type>void</type>
      <name>RC_free_dep</name>
      <anchorfile>sm__recovery_8h.html</anchorfile>
      <anchor>b18f8cc522f512463fc902eca36cac13</anchor>
      <arglist>(void *info)</arglist>
    </member>
    <member kind="function">
      <type>RC_dep_info *</type>
      <name>RC_new_dep</name>
      <anchorfile>sm__recovery_8h.html</anchorfile>
      <anchor>3a1890bd79ca253410e3cc4443e33e5b</anchor>
      <arglist>(L_Oper *from_op, L_Oper *to_op, L_Operand *oprd)</arglist>
    </member>
    <member kind="function">
      <type>void</type>
      <name>RC_add_flow_dep</name>
      <anchorfile>sm__recovery_8h.html</anchorfile>
      <anchor>4c125ecd23ce98c5730c5395d7744fe8</anchor>
      <arglist>(L_Oper *def_op, L_Oper *use_op, L_Operand *oprd)</arglist>
    </member>
    <member kind="function">
      <type>void</type>
      <name>RC_add_anti_dep</name>
      <anchorfile>sm__recovery_8h.html</anchorfile>
      <anchor>e2641e64a77f7113c6d14fcfb66ebd09</anchor>
      <arglist>(L_Oper *def_op, L_Oper *use_op, L_Operand *oprd)</arglist>
    </member>
    <member kind="function">
      <type>void</type>
      <name>RC_reset_dataflow</name>
      <anchorfile>sm__recovery_8h.html</anchorfile>
      <anchor>dc4c59b96881458e138d63a7533e03f0</anchor>
      <arglist>(L_Func *fn)</arglist>
    </member>
    <member kind="function">
      <type>int</type>
      <name>RC_anti_dep_skip</name>
      <anchorfile>sm__recovery_8h.html</anchorfile>
      <anchor>17a77a2c88f486cce360495d3a52b9c1</anchor>
      <arglist>(L_Oper *op)</arglist>
    </member>
    <member kind="function">
      <type>void</type>
      <name>RC_generate_anti_deps_for_ops</name>
      <anchorfile>sm__recovery_8h.html</anchorfile>
      <anchor>9f62424bc64347b14fafec7ed921fb25</anchor>
      <arglist>(L_Oper *use_op, L_Oper *def_op)</arglist>
    </member>
    <member kind="function">
      <type>void</type>
      <name>RC_generate_global_flow_deps</name>
      <anchorfile>sm__recovery_8h.html</anchorfile>
      <anchor>03aad64fa248cda1f73e702704963884</anchor>
      <arglist>(L_Func *fn)</arglist>
    </member>
    <member kind="function">
      <type>void</type>
      <name>RC_generate_anti_deps</name>
      <anchorfile>sm__recovery_8h.html</anchorfile>
      <anchor>0a6e4fb72a332faa8e3b0881dcd82ca1</anchor>
      <arglist>(L_Func *fn, List lds_list)</arglist>
    </member>
    <member kind="function">
      <type>void</type>
      <name>RC_fix_antideps</name>
      <anchorfile>sm__recovery_8h.html</anchorfile>
      <anchor>8744183d28059bcef358e40ee869373f</anchor>
      <arglist>(L_Func *fn, List lds_list, int check_only)</arglist>
    </member>
    <member kind="function">
      <type>void</type>
      <name>RC_jump_opti</name>
      <anchorfile>sm__recovery_8h.html</anchorfile>
      <anchor>966e06a67af5477dd808f81d7edda916</anchor>
      <arglist>(L_Func *fn, List *cb_list, L_Cb *end_cb)</arglist>
    </member>
    <member kind="function">
      <type>void</type>
      <name>RC_cbs_reachable_from</name>
      <anchorfile>sm__recovery_8h.html</anchorfile>
      <anchor>d2b00badbd1d88bfa6f6434a41c7f0da</anchor>
      <arglist>(L_Func *fn, L_Cb *start_cb, L_Oper *start_op, List chk_op_list, List exclude_chk_op_list, List *unrch_check_cb_list, Set *vop_set, List *ctrl_list, List *cb_list)</arglist>
    </member>
    <member kind="function">
      <type>void</type>
      <name>RC_split_around_checks</name>
      <anchorfile>sm__recovery_8h.html</anchorfile>
      <anchor>9fbd230d6d1786cc750e07097180c67f</anchor>
      <arglist>(L_Func *fn, int sched)</arglist>
    </member>
    <member kind="function">
      <type>void</type>
      <name>RC_add_rc_return_jumps</name>
      <anchorfile>sm__recovery_8h.html</anchorfile>
      <anchor>e9a85de1bca20119748ffce902cf3037</anchor>
      <arglist>(L_Func *fn, int sched)</arglist>
    </member>
    <member kind="function">
      <type>int</type>
      <name>RC_valid_dependence</name>
      <anchorfile>sm__recovery_8h.html</anchorfile>
      <anchor>e2aa8eadc0724bda64686065929775f1</anchor>
      <arglist>(L_Cb *cur_cb, L_Oper *cur_op, L_Cb *dep_cb, L_Oper *dep_op, L_Cb *lds_cb, L_Oper *lds_op, L_Cb *chk_cb, L_Oper *chk_op)</arglist>
    </member>
    <member kind="function">
      <type>List</type>
      <name>RC_find_spec_load_and_checks</name>
      <anchorfile>sm__recovery_8h.html</anchorfile>
      <anchor>b17593ce7e28643fb748298265deed70</anchor>
      <arglist>(L_Func *fn)</arglist>
    </member>
    <member kind="function">
      <type>List</type>
      <name>RC_is_load_spec_above_check</name>
      <anchorfile>sm__recovery_8h.html</anchorfile>
      <anchor>a90f482ac10b837125da3e8e51dcd127</anchor>
      <arglist>(L_Func *fn, List lds_list, L_Oper *cur_op, L_Oper *chk_op)</arglist>
    </member>
    <member kind="function">
      <type>void</type>
      <name>RC_make_load_speculative</name>
      <anchorfile>sm__recovery_8h.html</anchorfile>
      <anchor>0f9e17161752cc172b75a59f7405e21c</anchor>
      <arglist>(L_Func *fn, L_Oper *lds_op)</arglist>
    </member>
    <member kind="function">
      <type>void</type>
      <name>RC_get_ctrl_for_rc</name>
      <anchorfile>sm__recovery_8h.html</anchorfile>
      <anchor>891fb83a191484a1f974b11c09a97b09</anchor>
      <arglist>(List *op_list, List cb_list, L_Cb *lds_cb, L_Oper *lds_op)</arglist>
    </member>
    <member kind="function">
      <type>void</type>
      <name>RC_get_ops_for_rc</name>
      <anchorfile>sm__recovery_8h.html</anchorfile>
      <anchor>5cf686bf6a77fa4ed78fb0ed3029b846</anchor>
      <arglist>(L_Func *fn, List *op_list, List *dep_ctrl_list, List *rc_list, List *lds_list, List *last_op_list, Set vop_set, List cb_list, L_Cb *lds_cb, L_Oper *lds_op, L_Cb *chk_cb, L_Oper *chk_op)</arglist>
    </member>
    <member kind="function">
      <type>void</type>
      <name>RC_get_prsv_for_rc</name>
      <anchorfile>sm__recovery_8h.html</anchorfile>
      <anchor>aea4bb52a503ef4dce635bdd75f96e88</anchor>
      <arglist>(List rc_list, List *psrv_list)</arglist>
    </member>
    <member kind="function">
      <type>int</type>
      <name>RC_get_anti_ops_for_rc</name>
      <anchorfile>sm__recovery_8h.html</anchorfile>
      <anchor>d7d6512132db8b4e22eacd39189a577d</anchor>
      <arglist>(List cb_list, Set vop_set, List *rc_list, List psrv_list, List *conf_list, List *svrt_list, L_Cb *lds_cb, L_Oper *lds_op, L_Cb *chk_cb, L_Oper *chk_op)</arglist>
    </member>
    <member kind="function">
      <type>void</type>
      <name>RC_get_dests_for_rc</name>
      <anchorfile>sm__recovery_8h.html</anchorfile>
      <anchor>2276610112d725a269894c0c95681cae</anchor>
      <arglist>(List rc_list, List *dest_list)</arglist>
    </member>
    <member kind="function">
      <type>int</type>
      <name>RC_get_def_ops_for_rc</name>
      <anchorfile>sm__recovery_8h.html</anchorfile>
      <anchor>43552a1da13a6c3edaebcc486214f05b</anchor>
      <arglist>(List cb_list, List dest_list, List *rc_list, L_Cb *lds_cb, L_Oper *lds_op, L_Cb *chk_cb, L_Oper *chk_op)</arglist>
    </member>
    <member kind="function">
      <type>void</type>
      <name>RC_build_rc_cbs</name>
      <anchorfile>sm__recovery_8h.html</anchorfile>
      <anchor>fb1c42d118d6cab1b52baefdd3d1a974</anchor>
      <arglist>(L_Func *fn, List cb_list, List rc_list, List *new_cb_list, List *new_op_list, L_Cb **start_cb, L_Cb **end_cb, L_Cb *lds_cb, L_Oper *lds_op, L_Cb *chk_cb, L_Oper *chk_op)</arglist>
    </member>
    <member kind="function">
      <type>void</type>
      <name>RC_fix_anti_deps</name>
      <anchorfile>sm__recovery_8h.html</anchorfile>
      <anchor>2bffd8b06cc024ff5ca6a4ad8d099bfb</anchor>
      <arglist>(List conf_list, List svrt_list, List *psrv_list, L_Cb *start_cb, L_Cb *end_cb, L_Cb *lds_cb, L_Oper *lds_op, L_Cb *chk_cb, L_Oper *chk_op)</arglist>
    </member>
    <member kind="function">
      <type>void</type>
      <name>RC_add_check_targets</name>
      <anchorfile>sm__recovery_8h.html</anchorfile>
      <anchor>a9a626e698fbb7d90c9e80e850212504</anchor>
      <arglist>(L_Cb *start_cb, L_Cb *end_cb, List last_op_list, L_Cb *chk_cb, L_Oper *chk_op)</arglist>
    </member>
    <member kind="function">
      <type>void</type>
      <name>RC_create_recovery_code</name>
      <anchorfile>sm__recovery_8h.html</anchorfile>
      <anchor>b9c870fa2b11073502f9d44e80b9a30b</anchor>
      <arglist>(L_Func *fn, List *lds_list)</arglist>
    </member>
    <member kind="function">
      <type>void</type>
      <name>L_read_parm_RC</name>
      <anchorfile>sm__recovery_8h.html</anchorfile>
      <anchor>f0a2f22b4b4e6e53a1bcbcc544fed7bf</anchor>
      <arglist>(Parm_Parse_Info *ppi)</arglist>
    </member>
    <member kind="function">
      <type>void</type>
      <name>RC_generate_recovery_code</name>
      <anchorfile>sm__recovery_8h.html</anchorfile>
      <anchor>41f36fc81498e35cb3bd83b5b0645ee0</anchor>
      <arglist>(L_Func *fn)</arglist>
    </member>
    <member kind="function">
      <type>void</type>
      <name>RC_recombine_cbs</name>
      <anchorfile>sm__recovery_8h.html</anchorfile>
      <anchor>1b97b478d5cdfcd665c3d6795bd60e22</anchor>
      <arglist>(L_Func *fn)</arglist>
    </member>
    <member kind="function">
      <type>void</type>
      <name>RC_fix_recovery_code_bundles</name>
      <anchorfile>sm__recovery_8h.html</anchorfile>
      <anchor>689af4922be6206a978383af6635ea3b</anchor>
      <arglist>(L_Func *fn)</arglist>
    </member>
  </compound>
  <compound kind="file">
    <name>sm_rinfo.c</name>
    <path>/mnt/hgfs/mkt/Desktop/openimpact/src/Lcode/sched/SM/</path>
    <filename>sm__rinfo_8c</filename>
    <includes id="sm_8h" name="sm.h" local="yes" imported="no">sm.h</includes>
    <member kind="define">
      <type>#define</type>
      <name>TYPICAL_NUM_CONFLICTS</name>
      <anchorfile>sm__rinfo_8c.html</anchorfile>
      <anchor>6917a99f3f172eccf4275944897a1e88</anchor>
      <arglist></arglist>
    </member>
    <member kind="define">
      <type>#define</type>
      <name>MAX_CONFLICTS</name>
      <anchorfile>sm__rinfo_8c.html</anchorfile>
      <anchor>14d30da6b838f3d718c0d92a9f3f3816</anchor>
      <arglist></arglist>
    </member>
    <member kind="function">
      <type>void</type>
      <name>SM_update_implicit_branch_actions</name>
      <anchorfile>sm__rinfo_8c.html</anchorfile>
      <anchor>88975748dd4e8524e2fccef6bf67ee6d</anchor>
      <arglist>(SM_Cb *sm_cb, SM_Reg_Info *new_rinfo)</arglist>
    </member>
    <member kind="function" static="yes">
      <type>static int</type>
      <name>SM_type_from_operand</name>
      <anchorfile>sm__rinfo_8c.html</anchorfile>
      <anchor>d0c67644b90124738d67a81adccd2060</anchor>
      <arglist>(L_Operand *operand)</arglist>
    </member>
    <member kind="function" static="yes">
      <type>static int</type>
      <name>SM_id_from_operand</name>
      <anchorfile>sm__rinfo_8c.html</anchorfile>
      <anchor>b90ac3460198aff176c76aad5262ae4e</anchor>
      <arglist>(L_Operand *operand)</arglist>
    </member>
    <member kind="function">
      <type>void</type>
      <name>SM_resize_rinfo_hash_array</name>
      <anchorfile>sm__rinfo_8c.html</anchorfile>
      <anchor>4031ab161521b34a39d825cb73be0870</anchor>
      <arglist>(SM_Cb *sm_cb)</arglist>
    </member>
    <member kind="function">
      <type>SM_Reg_Info *</type>
      <name>SM_add_reg_info</name>
      <anchorfile>sm__rinfo_8c.html</anchorfile>
      <anchor>5e88f05bd7ce34e7d3a6e40cecca607c</anchor>
      <arglist>(SM_Cb *sm_cb, L_Operand *reg_operand)</arglist>
    </member>
    <member kind="function">
      <type>SM_Reg_Info *</type>
      <name>SM_find_reg_info</name>
      <anchorfile>sm__rinfo_8c.html</anchorfile>
      <anchor>4b8be6e1608e9be1626020b88d76db78</anchor>
      <arglist>(SM_Cb *sm_cb, L_Operand *reg_operand)</arglist>
    </member>
    <member kind="function">
      <type>void</type>
      <name>SM_place_reg_action</name>
      <anchorfile>sm__rinfo_8c.html</anchorfile>
      <anchor>fb641dbb76fe7edcaee0df5028b21ebb</anchor>
      <arglist>(SM_Reg_Action *action)</arglist>
    </member>
    <member kind="function">
      <type>SM_Reg_Action *</type>
      <name>SM_insert_reg_action</name>
      <anchorfile>sm__rinfo_8c.html</anchorfile>
      <anchor>474ca7a4338cb62a75a2466d2885c79c</anchor>
      <arglist>(SM_Reg_Info *rinfo, SM_Oper *sm_op, int operand_type, int operand_number, L_Operand *reg_operand, unsigned int flags)</arglist>
    </member>
    <member kind="function">
      <type>void</type>
      <name>SM_unplace_reg_action</name>
      <anchorfile>sm__rinfo_8c.html</anchorfile>
      <anchor>3150638b3e74bdecbcdee69af0c3cf7a</anchor>
      <arglist>(SM_Reg_Action *action)</arglist>
    </member>
    <member kind="function">
      <type>void</type>
      <name>SM_delete_reg_action</name>
      <anchorfile>sm__rinfo_8c.html</anchorfile>
      <anchor>45494cda0637597b8fa52b6a1f81af19</anchor>
      <arglist>(SM_Reg_Action *action)</arglist>
    </member>
    <member kind="function">
      <type>void</type>
      <name>SM_reposition_reg_action</name>
      <anchorfile>sm__rinfo_8c.html</anchorfile>
      <anchor>bf79394ddba4abfb82b6eada74de3ffd</anchor>
      <arglist>(SM_Reg_Action *action)</arglist>
    </member>
    <member kind="function">
      <type>SM_Reg_Action *</type>
      <name>SM_add_reg_action</name>
      <anchorfile>sm__rinfo_8c.html</anchorfile>
      <anchor>ccb8edcd77ee94f72aa7c5fa774f2a0f</anchor>
      <arglist>(SM_Oper *sm_op, int operand_type, int operand_number, L_Operand *reg_operand)</arglist>
    </member>
    <member kind="function">
      <type>void</type>
      <name>SM_add_reg_actions_for_op</name>
      <anchorfile>sm__rinfo_8c.html</anchorfile>
      <anchor>4f0f22f48ad3638fc4569b375e783e6e</anchor>
      <arglist>(SM_Oper *sm_op)</arglist>
    </member>
    <member kind="function">
      <type>void</type>
      <name>SM_build_reg_info_table</name>
      <anchorfile>sm__rinfo_8c.html</anchorfile>
      <anchor>44c7539377ce68066474efd8463e36dc</anchor>
      <arglist>(SM_Cb *sm_cb)</arglist>
    </member>
    <member kind="function">
      <type>void</type>
      <name>SM_print_reg_action</name>
      <anchorfile>sm__rinfo_8c.html</anchorfile>
      <anchor>e77ff2e15505eab82fd52ce08dca0803</anchor>
      <arglist>(FILE *out, SM_Reg_Action *action)</arglist>
    </member>
    <member kind="function">
      <type>void</type>
      <name>SM_print_reg_info_operand</name>
      <anchorfile>sm__rinfo_8c.html</anchorfile>
      <anchor>9f74b8d2905f61d10740dc5696ad7d27</anchor>
      <arglist>(FILE *out, SM_Reg_Info *rinfo)</arglist>
    </member>
    <member kind="function">
      <type>void</type>
      <name>SM_print_reg_info</name>
      <anchorfile>sm__rinfo_8c.html</anchorfile>
      <anchor>2dad8a9222f55933eac028f8be311d59</anchor>
      <arglist>(FILE *out, SM_Reg_Info *rinfo)</arglist>
    </member>
    <member kind="function">
      <type>void</type>
      <name>SM_print_reg_info_table</name>
      <anchorfile>sm__rinfo_8c.html</anchorfile>
      <anchor>397ff832d19fc2dc2e2239446e32179d</anchor>
      <arglist>(FILE *out, SM_Cb *sm_cb)</arglist>
    </member>
    <member kind="function">
      <type>void</type>
      <name>SM_print_sorted_reg_info_table</name>
      <anchorfile>sm__rinfo_8c.html</anchorfile>
      <anchor>292431d5e5f90d68f6bd40b47bdbce57</anchor>
      <arglist>(FILE *out, SM_Cb *sm_cb)</arglist>
    </member>
    <member kind="function">
      <type>void</type>
      <name>SM_free_reg_info_table</name>
      <anchorfile>sm__rinfo_8c.html</anchorfile>
      <anchor>3400bae118e95434618b944ea2f7c08b</anchor>
      <arglist>(SM_Cb *sm_cb)</arglist>
    </member>
    <member kind="variable">
      <type>L_Alloc_Pool *</type>
      <name>SM_Reg_Action_pool</name>
      <anchorfile>sm__rinfo_8c.html</anchorfile>
      <anchor>260ccaf3384a0dfea38e2e926ba7bfcf</anchor>
      <arglist></arglist>
    </member>
    <member kind="variable">
      <type>L_Alloc_Pool *</type>
      <name>SM_Reg_Info_pool</name>
      <anchorfile>sm__rinfo_8c.html</anchorfile>
      <anchor>eaa41f8aa5f5cc1450a47a4e5fe66330</anchor>
      <arglist></arglist>
    </member>
    <member kind="variable">
      <type>L_Alloc_Pool *</type>
      <name>SM_Reg_Action_Conflict_pool</name>
      <anchorfile>sm__rinfo_8c.html</anchorfile>
      <anchor>1ff695952d54a08314adb2dd0e3771d0</anchor>
      <arglist></arglist>
    </member>
    <member kind="variable">
      <type>L_Alloc_Pool *</type>
      <name>SM_Dep_pool</name>
      <anchorfile>sm__rinfo_8c.html</anchorfile>
      <anchor>9bb765c5a4a3f49a00cf29bb3cff7305</anchor>
      <arglist></arglist>
    </member>
  </compound>
  <compound kind="file">
    <name>sm_rmap.c</name>
    <path>/mnt/hgfs/mkt/Desktop/openimpact/src/Lcode/sched/SM/</path>
    <filename>sm__rmap_8c</filename>
    <includes id="sm_8h" name="sm.h" local="yes" imported="no">sm.h</includes>
    <member kind="define">
      <type>#define</type>
      <name>DEBUG_RMAP_RESIZE</name>
      <anchorfile>sm__rmap_8c.html</anchorfile>
      <anchor>c5b47c4c50783b54a9e9efbe9262698f</anchor>
      <arglist></arglist>
    </member>
    <member kind="define">
      <type>#define</type>
      <name>DEBUG_RMAP</name>
      <anchorfile>sm__rmap_8c.html</anchorfile>
      <anchor>bb86bec221a00d270ed52f47811465d5</anchor>
      <arglist></arglist>
    </member>
    <member kind="define">
      <type>#define</type>
      <name>TAKE_STATS</name>
      <anchorfile>sm__rmap_8c.html</anchorfile>
      <anchor>ef6cd5c57e3470359ed522e3c3c69490</anchor>
      <arglist></arglist>
    </member>
    <member kind="function">
      <type>void</type>
      <name>SM_commit_choices</name>
      <anchorfile>sm__rmap_8c.html</anchorfile>
      <anchor>dea5e302ffa52a9a1b14cec3eb244022</anchor>
      <arglist>(unsigned int *map, SM_Table *table, unsigned short *choices_made)</arglist>
    </member>
    <member kind="function">
      <type>void</type>
      <name>SM_release_choices</name>
      <anchorfile>sm__rmap_8c.html</anchorfile>
      <anchor>997c5d1ab46607c7498d336e62cbff53</anchor>
      <arglist>(unsigned int *map, SM_Table *table, unsigned short *choices_made)</arglist>
    </member>
    <member kind="function">
      <type>int</type>
      <name>SM_choose_first_avail_options</name>
      <anchorfile>sm__rmap_8c.html</anchorfile>
      <anchor>263423053038e25bdb80aff4675a1914</anchor>
      <arglist>(unsigned int *map, SM_Table *table, unsigned short *choices_made, unsigned int min_slot, unsigned int max_slot, Mdes_Stats *stats)</arglist>
    </member>
    <member kind="function">
      <type>int</type>
      <name>SM_choose_options_w_d_rule</name>
      <anchorfile>sm__rmap_8c.html</anchorfile>
      <anchor>689f4d75a7912e15858ea8c716745733</anchor>
      <arglist>(SM_Oper *sm_op, unsigned int template_index, unsigned int *map, SM_Table *table, unsigned short *choices_made, unsigned int slot, Mdes_Stats *stats)</arglist>
    </member>
    <member kind="function">
      <type>void</type>
      <name>SM_check_map</name>
      <anchorfile>sm__rmap_8c.html</anchorfile>
      <anchor>8d890cd506f7e68627eb457669822017</anchor>
      <arglist>(SM_Cb *sm_cb)</arglist>
    </member>
    <member kind="function">
      <type>void</type>
      <name>SM_expand_map_end_offset</name>
      <anchorfile>sm__rmap_8c.html</anchorfile>
      <anchor>169cc66467a4e7881a8f7698742093bf</anchor>
      <arglist>(SM_Cb *sm_cb, int max_init_offset)</arglist>
    </member>
    <member kind="function">
      <type>void</type>
      <name>SM_expand_map_start_offset</name>
      <anchorfile>sm__rmap_8c.html</anchorfile>
      <anchor>7c7064fcf7a6e074fe26e80db122e8fb</anchor>
      <arglist>(SM_Cb *sm_cb, int min_init_offset)</arglist>
    </member>
    <member kind="function">
      <type>void</type>
      <name>SM_init_for_max_usage</name>
      <anchorfile>sm__rmap_8c.html</anchorfile>
      <anchor>224d563efd05ca72ae7d3fd4c8eb9b70</anchor>
      <arglist>(SM_Cb *sm_cb, int max_usage_offset)</arglist>
    </member>
    <member kind="function">
      <type>void</type>
      <name>SM_init_for_min_usage</name>
      <anchorfile>sm__rmap_8c.html</anchorfile>
      <anchor>4b5b461c48dc75e06f2b24afaaeab80d</anchor>
      <arglist>(SM_Cb *sm_cb, int min_usage_offset)</arglist>
    </member>
    <member kind="function">
      <type>void</type>
      <name>SM_create_map</name>
      <anchorfile>sm__rmap_8c.html</anchorfile>
      <anchor>effe9b8876d11c3b84ccafe953e94133</anchor>
      <arglist>(SM_Cb *sm_cb, int min_usage_offset, int max_usage_offset)</arglist>
    </member>
    <member kind="function">
      <type>int</type>
      <name>SM_sched_table</name>
      <anchorfile>sm__rmap_8c.html</anchorfile>
      <anchor>27d124d41133b2ed180e083df33b951d</anchor>
      <arglist>(SM_Cb *sm_cb, SM_Oper *sm_op, unsigned int template_index, SM_Table *table, unsigned short *choice_array, int time, unsigned short min_slot, unsigned short max_slot, int commit, Mdes_Stats *stats)</arglist>
    </member>
    <member kind="function">
      <type>void</type>
      <name>SM_unsched_table</name>
      <anchorfile>sm__rmap_8c.html</anchorfile>
      <anchor>f5797ef9c78983a02c76cc82991cc223</anchor>
      <arglist>(SM_Cb *sm_cb, SM_Table *table, unsigned short *choice_array, int time)</arglist>
    </member>
    <member kind="function">
      <type>int</type>
      <name>SM_find_drule</name>
      <anchorfile>sm__rmap_8c.html</anchorfile>
      <anchor>940e48b11af85e3a973ac5d7e8fb94ea</anchor>
      <arglist>(SM_Oper *sm_op, unsigned int abs_slot, unsigned int template_index, int prev_index)</arglist>
    </member>
    <member kind="function">
      <type>void</type>
      <name>SM_print_option</name>
      <anchorfile>sm__rmap_8c.html</anchorfile>
      <anchor>f468b1263fd5ba32863a0b67333ebd0a</anchor>
      <arglist>(FILE *out, SM_Mdes *sm_mdes, SM_Option *option)</arglist>
    </member>
    <member kind="function">
      <type>void</type>
      <name>SM_print_map</name>
      <anchorfile>sm__rmap_8c.html</anchorfile>
      <anchor>a83bf74e00d42409c45844d6694eca13</anchor>
      <arglist>(FILE *out, SM_Mdes *sm_mdes, SM_Cb *sm_cb)</arglist>
    </member>
    <member kind="function">
      <type>void</type>
      <name>SM_print_stats</name>
      <anchorfile>sm__rmap_8c.html</anchorfile>
      <anchor>969904a1892879e71c7356090d297769</anchor>
      <arglist>(FILE *out, Mdes_Stats *stats)</arglist>
    </member>
  </compound>
  <compound kind="file">
    <name>sm_sched.c</name>
    <path>/mnt/hgfs/mkt/Desktop/openimpact/src/Lcode/sched/SM/</path>
    <filename>sm__sched_8c</filename>
    <includes id="sm_8h" name="sm.h" local="yes" imported="no">sm.h</includes>
    <member kind="define">
      <type>#define</type>
      <name>DEBUG_NOP</name>
      <anchorfile>sm__sched_8c.html</anchorfile>
      <anchor>46e82140839791d257a52cb0f90917c9</anchor>
      <arglist></arglist>
    </member>
    <member kind="define">
      <type>#define</type>
      <name>DEBUG_PRIORITY</name>
      <anchorfile>sm__sched_8c.html</anchorfile>
      <anchor>03595079affa84dc87163d89ba309ac8</anchor>
      <arglist></arglist>
    </member>
    <member kind="define">
      <type>#define</type>
      <name>SM_BRANCHES_AT_END</name>
      <anchorfile>sm__sched_8c.html</anchorfile>
      <anchor>b68925b60a17fc0f1e0f43c81fdf548f</anchor>
      <arglist></arglist>
    </member>
    <member kind="function">
      <type>int</type>
      <name>SM_schedule_oper_schedule</name>
      <anchorfile>sm__sched_8c.html</anchorfile>
      <anchor>bb1bf135c420a2bb5922a4955a59a2ba</anchor>
      <arglist>(SM_Oper *sm_op, SM_Issue_Group *issue_group_ptr, int issue_time, int slot, unsigned int sched_flags)</arglist>
    </member>
    <member kind="function">
      <type>int *</type>
      <name>SM_init_port_array</name>
      <anchorfile>sm__sched_8c.html</anchorfile>
      <anchor>9bcde7caa3406c0cebd25a52f03f9f9e</anchor>
      <arglist>(SM_Cb *sm_cb)</arglist>
    </member>
    <member kind="function">
      <type>void</type>
      <name>SM_calculate_template_vector</name>
      <anchorfile>sm__sched_8c.html</anchorfile>
      <anchor>54c9346f255a68d7e6ee3762f82bae06</anchor>
      <arglist>(SM_Issue_Group *issue_group)</arglist>
    </member>
    <member kind="function">
      <type>int</type>
      <name>SM_check_template_validity</name>
      <anchorfile>sm__sched_8c.html</anchorfile>
      <anchor>6df12149d292674ed453823f0bdc7cb7</anchor>
      <arglist>(SM_Cb *sm_cb, unsigned int temp_template)</arglist>
    </member>
    <member kind="function">
      <type>SM_Issue_Group *</type>
      <name>SM_create_issue_group</name>
      <anchorfile>sm__sched_8c.html</anchorfile>
      <anchor>a4e1936d04b3593d2aed1b38a3058e6b</anchor>
      <arglist>(SM_Cb *sm_cb, int issue_time)</arglist>
    </member>
    <member kind="function">
      <type>void</type>
      <name>SM_link_issue_group</name>
      <anchorfile>sm__sched_8c.html</anchorfile>
      <anchor>adbfb7d7b2582ced63f3c4946f12f72c</anchor>
      <arglist>(SM_Issue_Group *issue_grp_ptr)</arglist>
    </member>
    <member kind="function">
      <type>SM_Issue_Group *</type>
      <name>SM_check_for_issue_group</name>
      <anchorfile>sm__sched_8c.html</anchorfile>
      <anchor>ed246353f15827ff68b8682bd97afe52</anchor>
      <arglist>(SM_Cb *sm_cb, int issue_time)</arglist>
    </member>
    <member kind="function">
      <type>void</type>
      <name>SM_issue_group_full_check</name>
      <anchorfile>sm__sched_8c.html</anchorfile>
      <anchor>f2b45291cd6a105c9dcaba667f002afb</anchor>
      <arglist>(SM_Issue_Group *issue_group_ptr)</arglist>
    </member>
    <member kind="function">
      <type>int</type>
      <name>SM_verify_issue</name>
      <anchorfile>sm__sched_8c.html</anchorfile>
      <anchor>2dfe53d5af58869a3b8fc9a402e8dcee</anchor>
      <arglist>(SM_Issue_Group *issue_group_ptr, int bundle_number)</arglist>
    </member>
    <member kind="function">
      <type>SM_Oper *</type>
      <name>SM_create_nop</name>
      <anchorfile>sm__sched_8c.html</anchorfile>
      <anchor>09614662763d9c640b7c2f90593d28b4</anchor>
      <arglist>(SM_Cb *sm_cb, int proc_opc)</arglist>
    </member>
    <member kind="function">
      <type>void</type>
      <name>SM_delete_nop</name>
      <anchorfile>sm__sched_8c.html</anchorfile>
      <anchor>dafbaa8210d64b6498014ff890f8b7dd</anchor>
      <arglist>(SM_Oper *sm_op)</arglist>
    </member>
    <member kind="function">
      <type>void</type>
      <name>SM_unschedule_nop</name>
      <anchorfile>sm__sched_8c.html</anchorfile>
      <anchor>6ba636bdbd1041421b142fc7279bcc8e</anchor>
      <arglist>(SM_Oper *sm_op)</arglist>
    </member>
    <member kind="function">
      <type>int</type>
      <name>SM_insert_nop</name>
      <anchorfile>sm__sched_8c.html</anchorfile>
      <anchor>ff3ddf50f7715d11767c701843471b52</anchor>
      <arglist>(SM_Issue_Group *issue_group_ptr, int slot, unsigned int sched_flags)</arglist>
    </member>
    <member kind="function">
      <type>void</type>
      <name>SM_copy_issue</name>
      <anchorfile>sm__sched_8c.html</anchorfile>
      <anchor>b269195f789cef850d7c56c160efc9f6</anchor>
      <arglist>(SM_Issue_Group *dest_issue_group_ptr, SM_Issue_Group *src_issue_group_ptr)</arglist>
    </member>
    <member kind="function">
      <type>void</type>
      <name>SM_restore_issue_group</name>
      <anchorfile>sm__sched_8c.html</anchorfile>
      <anchor>90a71091c5506878278bf3b50b65582e</anchor>
      <arglist>(SM_Issue_Group *issue_group_ptr, SM_Issue_Group *save_issue_group_ptr)</arglist>
    </member>
    <member kind="function">
      <type>void</type>
      <name>SM_update_adjacent_bounds</name>
      <anchorfile>sm__sched_8c.html</anchorfile>
      <anchor>adc6d3aed80f7712c152d226cc934c1f</anchor>
      <arglist>(SM_Oper *sm_op)</arglist>
    </member>
    <member kind="function" static="yes">
      <type>static void</type>
      <name>SM_recalculate_adjacent_bounds</name>
      <anchorfile>sm__sched_8c.html</anchorfile>
      <anchor>c6cd790900f0bfd259130d154c7e3b52</anchor>
      <arglist>(SM_Oper *sm_op, SM_Priority_Queue *priority_queue)</arglist>
    </member>
    <member kind="function">
      <type>void</type>
      <name>SM_recalculate_upper_bound</name>
      <anchorfile>sm__sched_8c.html</anchorfile>
      <anchor>9033bfb17d8f1e7f744961567df9695b</anchor>
      <arglist>(SM_Oper *sm_op)</arglist>
    </member>
    <member kind="function">
      <type>void</type>
      <name>SM_recalculate_lower_bound</name>
      <anchorfile>sm__sched_8c.html</anchorfile>
      <anchor>9e79ecff076013ba3bfb73551a60b04d</anchor>
      <arglist>(SM_Oper *sm_op)</arglist>
    </member>
    <member kind="function">
      <type>int</type>
      <name>SM_alt_ready</name>
      <anchorfile>sm__sched_8c.html</anchorfile>
      <anchor>677eabc193d7b15b037f6ca3d7473e83</anchor>
      <arglist>(SM_Oper *sm_op, SM_Compatible_Alt *alt, int issue_time)</arglist>
    </member>
    <member kind="function">
      <type>int</type>
      <name>SM_schedule_oper_insert</name>
      <anchorfile>sm__sched_8c.html</anchorfile>
      <anchor>3843294bd976740996dea71199965e1e</anchor>
      <arglist>(SM_Oper *sm_op, SM_Issue_Group *issue_group_ptr, int issue_time, unsigned int sched_flags)</arglist>
    </member>
    <member kind="function">
      <type>int</type>
      <name>SM_schedule_oper_priority</name>
      <anchorfile>sm__sched_8c.html</anchorfile>
      <anchor>72f55612976b7dd84719fe00c6069311</anchor>
      <arglist>(SM_Oper *sm_op, int issue_time, int earliest_slot, int latest_slot, unsigned int sched_flags, int compact)</arglist>
    </member>
    <member kind="function">
      <type>int</type>
      <name>SM_schedule_oper_template_bundling</name>
      <anchorfile>sm__sched_8c.html</anchorfile>
      <anchor>e7ec21bc3fe54f72dc6f7366a3b106fb</anchor>
      <arglist>(SM_Oper *sm_op, int issue_time, int earliest_slot, int latest_slot, unsigned int sched_flags)</arglist>
    </member>
    <member kind="function">
      <type>int</type>
      <name>SM_schedule_oper_no_bundling</name>
      <anchorfile>sm__sched_8c.html</anchorfile>
      <anchor>03ae3feb0d4b2ebd4c07293bde08d114</anchor>
      <arglist>(SM_Oper *sm_op, int issue_time, int earliest_slot, int latest_slot, unsigned int sched_flags)</arglist>
    </member>
    <member kind="function">
      <type>int</type>
      <name>SM_schedule_oper</name>
      <anchorfile>sm__sched_8c.html</anchorfile>
      <anchor>602f5c3eff20faea9a4949f2273feafb</anchor>
      <arglist>(SM_Oper *sm_op, int issue_time, int earliest_slot, int latest_slot, unsigned int sched_flags)</arglist>
    </member>
    <member kind="function">
      <type>void</type>
      <name>SM_unschedule_oper</name>
      <anchorfile>sm__sched_8c.html</anchorfile>
      <anchor>75f1913ac8d175b9dfd6ce15fd27ffc9</anchor>
      <arglist>(SM_Oper *sm_op, SM_Priority_Queue *priority_queue)</arglist>
    </member>
    <member kind="function">
      <type>void</type>
      <name>SM_spit_cb_issue_groups</name>
      <anchorfile>sm__sched_8c.html</anchorfile>
      <anchor>fc1e9c7dc1dfef78b2a805d0aa65cf38</anchor>
      <arglist>(SM_Cb *sm_cb, char *name)</arglist>
    </member>
    <member kind="variable">
      <type>L_Alloc_Pool *</type>
      <name>SM_Issue_Group_pool</name>
      <anchorfile>sm__sched_8c.html</anchorfile>
      <anchor>af71c74ddcf6fdffb424187f04a435b3</anchor>
      <arglist></arglist>
    </member>
    <member kind="variable">
      <type>L_Alloc_Pool *</type>
      <name>SM_Oper_pool</name>
      <anchorfile>sm__sched_8c.html</anchorfile>
      <anchor>ed27c056b3105b7cc9340854ec1c0b95</anchor>
      <arglist></arglist>
    </member>
    <member kind="variable">
      <type>L_Alloc_Pool *</type>
      <name>SM_Compatible_Alt_pool</name>
      <anchorfile>sm__sched_8c.html</anchorfile>
      <anchor>73663188d3bba621968636358e792dc8</anchor>
      <arglist></arglist>
    </member>
  </compound>
  <compound kind="struct">
    <name>Cb_Stats</name>
    <filename>structCb__Stats.html</filename>
    <member kind="variable">
      <type>char *</type>
      <name>fn_name</name>
      <anchorfile>structCb__Stats.html</anchorfile>
      <anchor>a7b13800235982e17999d1c2a99099da</anchor>
      <arglist></arglist>
    </member>
    <member kind="variable">
      <type>int</type>
      <name>cb_id</name>
      <anchorfile>structCb__Stats.html</anchorfile>
      <anchor>059be1dca913e266c2ba42229fc51156</anchor>
      <arglist></arglist>
    </member>
    <member kind="variable">
      <type>double</type>
      <name>orig_cycles</name>
      <anchorfile>structCb__Stats.html</anchorfile>
      <anchor>4f74ac07740ad4f742a10821af52e3b2</anchor>
      <arglist></arglist>
    </member>
    <member kind="variable">
      <type>double</type>
      <name>opti_cycles</name>
      <anchorfile>structCb__Stats.html</anchorfile>
      <anchor>143743d08d162e38c704fa5c9eff79c0</anchor>
      <arglist></arglist>
    </member>
    <member kind="variable">
      <type>Cb_Stats *</type>
      <name>next_stats</name>
      <anchorfile>structCb__Stats.html</anchorfile>
      <anchor>8446fa65a0edec858b13ff2ea3cfd84c</anchor>
      <arglist></arglist>
    </member>
  </compound>
  <compound kind="struct">
    <name>RC_cb_info</name>
    <filename>structRC__cb__info.html</filename>
    <member kind="variable">
      <type>Set</type>
      <name>valid_cbs</name>
      <anchorfile>structRC__cb__info.html</anchorfile>
      <anchor>aab60c9a39a672a7522410cdf77be645</anchor>
      <arglist></arglist>
    </member>
    <member kind="variable">
      <type>List</type>
      <name>chk_history</name>
      <anchorfile>structRC__cb__info.html</anchorfile>
      <anchor>86f315c27289fb72072bcf33ac7aa4ba</anchor>
      <arglist></arglist>
    </member>
    <member kind="variable">
      <type>int</type>
      <name>val_history</name>
      <anchorfile>structRC__cb__info.html</anchorfile>
      <anchor>c7ce4a4212c6ac9b9c4da3322679fa93</anchor>
      <arglist></arglist>
    </member>
  </compound>
  <compound kind="struct">
    <name>RC_dep_info</name>
    <filename>structRC__dep__info.html</filename>
    <member kind="variable">
      <type>L_Oper *</type>
      <name>from_op</name>
      <anchorfile>structRC__dep__info.html</anchorfile>
      <anchor>81adbe3ee2f2f242ee5546f271e81908</anchor>
      <arglist></arglist>
    </member>
    <member kind="variable">
      <type>L_Oper *</type>
      <name>to_op</name>
      <anchorfile>structRC__dep__info.html</anchorfile>
      <anchor>9484c4f5328abcbb3260c73b2fb4a5bf</anchor>
      <arglist></arglist>
    </member>
    <member kind="variable">
      <type>L_Operand *</type>
      <name>oprd</name>
      <anchorfile>structRC__dep__info.html</anchorfile>
      <anchor>97e098a078d34ae3b06cc185d5d20f89</anchor>
      <arglist></arglist>
    </member>
  </compound>
  <compound kind="struct">
    <name>RC_flow_info</name>
    <filename>structRC__flow__info.html</filename>
    <member kind="variable">
      <type>L_Cb *</type>
      <name>cb</name>
      <anchorfile>structRC__flow__info.html</anchorfile>
      <anchor>e2de5d847862ed8fc9cdcd48dca8490b</anchor>
      <arglist></arglist>
    </member>
    <member kind="variable">
      <type>List</type>
      <name>def_op_list</name>
      <anchorfile>structRC__flow__info.html</anchorfile>
      <anchor>5fb1379c31653a42ad81adcd8f462a6c</anchor>
      <arglist></arglist>
    </member>
    <member kind="variable">
      <type>List</type>
      <name>use_op_list</name>
      <anchorfile>structRC__flow__info.html</anchorfile>
      <anchor>67ed461ee3c0e2b4747eced327eb18f9</anchor>
      <arglist></arglist>
    </member>
    <member kind="variable">
      <type>int</type>
      <name>skip_op</name>
      <anchorfile>structRC__flow__info.html</anchorfile>
      <anchor>88934735acd46d29142014e9aa19e950</anchor>
      <arglist></arglist>
    </member>
    <member kind="variable">
      <type>List</type>
      <name>add_before</name>
      <anchorfile>structRC__flow__info.html</anchorfile>
      <anchor>c46a4ae5e75becab2273878b52cf41f0</anchor>
      <arglist></arglist>
    </member>
    <member kind="variable">
      <type>List</type>
      <name>add_after</name>
      <anchorfile>structRC__flow__info.html</anchorfile>
      <anchor>bdd29e0ab5bc2dfe004c0227cad36b7a</anchor>
      <arglist></arglist>
    </member>
    <member kind="variable">
      <type>List</type>
      <name>anti_def_op_list</name>
      <anchorfile>structRC__flow__info.html</anchorfile>
      <anchor>38332ea2da146a5a9c3698f1f5846b46</anchor>
      <arglist></arglist>
    </member>
    <member kind="variable">
      <type>List</type>
      <name>anti_use_op_list</name>
      <anchorfile>structRC__flow__info.html</anchorfile>
      <anchor>f1c09154f1ef2c35ff750cd084b13e17</anchor>
      <arglist></arglist>
    </member>
    <member kind="variable">
      <type>L_Oper **</type>
      <name>chk_ops</name>
      <anchorfile>structRC__flow__info.html</anchorfile>
      <anchor>8b519218d6787cd77cd85b555796159b</anchor>
      <arglist></arglist>
    </member>
    <member kind="variable">
      <type>List</type>
      <name>chk_list</name>
      <anchorfile>structRC__flow__info.html</anchorfile>
      <anchor>9e0b53ae31fdfba1101171a5509f4021</anchor>
      <arglist></arglist>
    </member>
    <member kind="variable">
      <type>L_Oper *</type>
      <name>chk_mark</name>
      <anchorfile>structRC__flow__info.html</anchorfile>
      <anchor>a6847ba92067821ce40cee30f5fe5462</anchor>
      <arglist></arglist>
    </member>
    <member kind="variable">
      <type>short</type>
      <name>chk_num</name>
      <anchorfile>structRC__flow__info.html</anchorfile>
      <anchor>744bdba7090806de915908a0c4d442ab</anchor>
      <arglist></arglist>
    </member>
  </compound>
  <compound kind="struct">
    <name>SM_Action_Qentry</name>
    <filename>structSM__Action__Qentry.html</filename>
    <member kind="variable">
      <type>SM_Action_Queue *</type>
      <name>queue</name>
      <anchorfile>structSM__Action__Qentry.html</anchorfile>
      <anchor>35b8f1c17a73867c6824faeaf60e3770</anchor>
      <arglist></arglist>
    </member>
    <member kind="variable">
      <type>SM_Reg_Action *</type>
      <name>action</name>
      <anchorfile>structSM__Action__Qentry.html</anchorfile>
      <anchor>a556d0fca8a5605249a3445fc3f2396c</anchor>
      <arglist></arglist>
    </member>
    <member kind="variable">
      <type>SM_Action_Qentry *</type>
      <name>next_qentry</name>
      <anchorfile>structSM__Action__Qentry.html</anchorfile>
      <anchor>7cdeb70035b9701c302c94e53f9129e7</anchor>
      <arglist></arglist>
    </member>
    <member kind="variable">
      <type>SM_Action_Qentry *</type>
      <name>prev_qentry</name>
      <anchorfile>structSM__Action__Qentry.html</anchorfile>
      <anchor>a1a946e0cc8b500f246d039d89ba7784</anchor>
      <arglist></arglist>
    </member>
    <member kind="variable">
      <type>SM_Action_Qentry *</type>
      <name>next_queue</name>
      <anchorfile>structSM__Action__Qentry.html</anchorfile>
      <anchor>ad1beaf17034c8e7e2b5a468e11e0151</anchor>
      <arglist></arglist>
    </member>
    <member kind="variable">
      <type>SM_Action_Qentry *</type>
      <name>prev_queue</name>
      <anchorfile>structSM__Action__Qentry.html</anchorfile>
      <anchor>af44a2c1b614f20b6c1c1d82ccb6f615</anchor>
      <arglist></arglist>
    </member>
  </compound>
  <compound kind="struct">
    <name>SM_Action_Queue</name>
    <filename>structSM__Action__Queue.html</filename>
    <member kind="variable">
      <type>int</type>
      <name>num_qentries</name>
      <anchorfile>structSM__Action__Queue.html</anchorfile>
      <anchor>cb90a403e42258756bd544ac95687e4c</anchor>
      <arglist></arglist>
    </member>
    <member kind="variable">
      <type>SM_Action_Qentry *</type>
      <name>first_qentry</name>
      <anchorfile>structSM__Action__Queue.html</anchorfile>
      <anchor>e5c380208c129c644ac82fde7f20b62d</anchor>
      <arglist></arglist>
    </member>
    <member kind="variable">
      <type>SM_Action_Qentry *</type>
      <name>last_qentry</name>
      <anchorfile>structSM__Action__Queue.html</anchorfile>
      <anchor>c7c5394da8c806573b1ee25effac0cc9</anchor>
      <arglist></arglist>
    </member>
  </compound>
  <compound kind="struct">
    <name>SM_Bundle</name>
    <filename>structSM__Bundle.html</filename>
    <member kind="variable">
      <type>unsigned int</type>
      <name>template_mask</name>
      <anchorfile>structSM__Bundle.html</anchorfile>
      <anchor>343c0ba291067fe202a764f7969467dd</anchor>
      <arglist></arglist>
    </member>
    <member kind="variable">
      <type>int</type>
      <name>template_index</name>
      <anchorfile>structSM__Bundle.html</anchorfile>
      <anchor>14b62ff649a36f97c3d89ef4b6c631ec</anchor>
      <arglist></arglist>
    </member>
    <member kind="variable">
      <type>int</type>
      <name>stop</name>
      <anchorfile>structSM__Bundle.html</anchorfile>
      <anchor>dea00351ceabac2d2720cee8c6f25409</anchor>
      <arglist></arglist>
    </member>
    <member kind="variable">
      <type>int</type>
      <name>empty</name>
      <anchorfile>structSM__Bundle.html</anchorfile>
      <anchor>78de4e26af4fd696caa0d433d78e858a</anchor>
      <arglist></arglist>
    </member>
    <member kind="variable">
      <type>int</type>
      <name>template_lock</name>
      <anchorfile>structSM__Bundle.html</anchorfile>
      <anchor>64535487d2ba2c454eaadffc0ca7506d</anchor>
      <arglist></arglist>
    </member>
    <member kind="variable">
      <type>int</type>
      <name>internal_stop_bit</name>
      <anchorfile>structSM__Bundle.html</anchorfile>
      <anchor>e9375a70171a49a62a31a39843b9e940</anchor>
      <arglist></arglist>
    </member>
  </compound>
  <compound kind="struct">
    <name>SM_Cb</name>
    <filename>structSM__Cb.html</filename>
    <member kind="variable">
      <type>L_Cb *</type>
      <name>lcode_cb</name>
      <anchorfile>structSM__Cb.html</anchorfile>
      <anchor>b69cef60244b3cc923619329b3f8e622</anchor>
      <arglist></arglist>
    </member>
    <member kind="variable">
      <type>L_Func *</type>
      <name>lcode_fn</name>
      <anchorfile>structSM__Cb.html</anchorfile>
      <anchor>19508ea8764d62f6c2c2b66ebed33204</anchor>
      <arglist></arglist>
    </member>
    <member kind="variable">
      <type>SM_Func *</type>
      <name>sm_func</name>
      <anchorfile>structSM__Cb.html</anchorfile>
      <anchor>1704d0098a857bdf1fa59d0520322088</anchor>
      <arglist></arglist>
    </member>
    <member kind="variable">
      <type>SM_Mdes *</type>
      <name>sm_mdes</name>
      <anchorfile>structSM__Cb.html</anchorfile>
      <anchor>cf43ee41bf2e7687f17256940102cb60</anchor>
      <arglist></arglist>
    </member>
    <member kind="variable">
      <type>Mdes *</type>
      <name>version1_mdes</name>
      <anchorfile>structSM__Cb.html</anchorfile>
      <anchor>afb5a7e1da77923aa1463e8a3d819cf1</anchor>
      <arglist></arglist>
    </member>
    <member kind="variable">
      <type>unsigned int</type>
      <name>flags</name>
      <anchorfile>structSM__Cb.html</anchorfile>
      <anchor>56d758ca67334dce204447f5e2c728d8</anchor>
      <arglist></arglist>
    </member>
    <member kind="variable">
      <type>int(*</type>
      <name>conflicting_operands</name>
      <anchorfile>structSM__Cb.html</anchorfile>
      <anchor>4de9f4841ce2f8f719ca0e0943a7380b</anchor>
      <arglist>)()</arglist>
    </member>
    <member kind="variable">
      <type>int</type>
      <name>prepass_sched</name>
      <anchorfile>structSM__Cb.html</anchorfile>
      <anchor>b954e007353ccdeb2556107a29141d53</anchor>
      <arglist></arglist>
    </member>
    <member kind="variable">
      <type>unsigned int</type>
      <name>op_count</name>
      <anchorfile>structSM__Cb.html</anchorfile>
      <anchor>84b62387724ee1dd859bdc3312269c78</anchor>
      <arglist></arglist>
    </member>
    <member kind="variable">
      <type>unsigned int</type>
      <name>num_unsched</name>
      <anchorfile>structSM__Cb.html</anchorfile>
      <anchor>e87950515c67e707ee50d007f66b7f6c</anchor>
      <arglist></arglist>
    </member>
    <member kind="variable">
      <type>unsigned int</type>
      <name>num_ignored</name>
      <anchorfile>structSM__Cb.html</anchorfile>
      <anchor>e0432cc768ead494503d41456557d01b</anchor>
      <arglist></arglist>
    </member>
    <member kind="variable">
      <type>SM_Oper *</type>
      <name>first_sched_op</name>
      <anchorfile>structSM__Cb.html</anchorfile>
      <anchor>a4ad0b5581237940e409243da13622a9</anchor>
      <arglist></arglist>
    </member>
    <member kind="variable">
      <type>SM_Oper *</type>
      <name>last_sched_op</name>
      <anchorfile>structSM__Cb.html</anchorfile>
      <anchor>df778b7b87cc03d978e4a881fadb2063</anchor>
      <arglist></arglist>
    </member>
    <member kind="variable">
      <type>SM_Oper *</type>
      <name>first_serial_op</name>
      <anchorfile>structSM__Cb.html</anchorfile>
      <anchor>a89868f9a5bb0dabe28a9c74d44a6286</anchor>
      <arglist></arglist>
    </member>
    <member kind="variable">
      <type>SM_Oper *</type>
      <name>last_serial_op</name>
      <anchorfile>structSM__Cb.html</anchorfile>
      <anchor>72a08b1707e79a4ac78ad8ee2e1a2574</anchor>
      <arglist></arglist>
    </member>
    <member kind="variable">
      <type>SM_Issue_Group *</type>
      <name>first_issue_group</name>
      <anchorfile>structSM__Cb.html</anchorfile>
      <anchor>be158167156db7cf63e87d1c1b1da340</anchor>
      <arglist></arglist>
    </member>
    <member kind="variable">
      <type>SM_Issue_Group *</type>
      <name>last_issue_group</name>
      <anchorfile>structSM__Cb.html</anchorfile>
      <anchor>7c499b091071c851f74dec5bc0eb7dd1</anchor>
      <arglist></arglist>
    </member>
    <member kind="variable">
      <type>SM_Oper_Queue *</type>
      <name>kernel_queue</name>
      <anchorfile>structSM__Cb.html</anchorfile>
      <anchor>8e754744130161765df739d3f6ed648a</anchor>
      <arglist></arglist>
    </member>
    <member kind="variable">
      <type>SM_Oper_Queue *</type>
      <name>dep_in_resolved</name>
      <anchorfile>structSM__Cb.html</anchorfile>
      <anchor>3ab463e9b3d26a75558853b248fab79f</anchor>
      <arglist></arglist>
    </member>
    <member kind="variable">
      <type>unsigned int</type>
      <name>special_dep_ignore_flags</name>
      <anchorfile>structSM__Cb.html</anchorfile>
      <anchor>80316b03172c244a20aa8c57cc0b8a5e</anchor>
      <arglist></arglist>
    </member>
    <member kind="variable">
      <type>SM_Cb *</type>
      <name>next_sm_cb</name>
      <anchorfile>structSM__Cb.html</anchorfile>
      <anchor>acce7d3f4617c58b044928247c7c98f2</anchor>
      <arglist></arglist>
    </member>
    <member kind="variable">
      <type>SM_Cb *</type>
      <name>prev_sm_cb</name>
      <anchorfile>structSM__Cb.html</anchorfile>
      <anchor>64973e3b1bc83d78d28996afc9c6e95a</anchor>
      <arglist></arglist>
    </member>
    <member kind="variable">
      <type>int</type>
      <name>num_exits</name>
      <anchorfile>structSM__Cb.html</anchorfile>
      <anchor>21f57ffb16a9f57743a863bc1cd5e46e</anchor>
      <arglist></arglist>
    </member>
    <member kind="variable">
      <type>SM_Oper **</type>
      <name>exit_op</name>
      <anchorfile>structSM__Cb.html</anchorfile>
      <anchor>685748c684ccc26ed3aab3147bc12a75</anchor>
      <arglist></arglist>
    </member>
    <member kind="variable">
      <type>double *</type>
      <name>exit_weight</name>
      <anchorfile>structSM__Cb.html</anchorfile>
      <anchor>07eda0050fe0929bd1585cb0c7c6c8c2</anchor>
      <arglist></arglist>
    </member>
    <member kind="variable">
      <type>double *</type>
      <name>exit_percentage</name>
      <anchorfile>structSM__Cb.html</anchorfile>
      <anchor>b518f6caf1ced1cd3a6e25c938bce550</anchor>
      <arglist></arglist>
    </member>
    <member kind="variable">
      <type>double</type>
      <name>cb_weight</name>
      <anchorfile>structSM__Cb.html</anchorfile>
      <anchor>98b769591bb9ebaf63b7b3818de06cc8</anchor>
      <arglist></arglist>
    </member>
    <member kind="variable">
      <type>SM_Reg_Info **</type>
      <name>rinfo_hash</name>
      <anchorfile>structSM__Cb.html</anchorfile>
      <anchor>ee7daff83f66069d9c56d0da0859a8ef</anchor>
      <arglist></arglist>
    </member>
    <member kind="variable">
      <type>unsigned int</type>
      <name>rinfo_hash_size</name>
      <anchorfile>structSM__Cb.html</anchorfile>
      <anchor>03e46bbeae1192954d2efa363bbb7260</anchor>
      <arglist></arglist>
    </member>
    <member kind="variable">
      <type>unsigned int</type>
      <name>rinfo_hash_mask</name>
      <anchorfile>structSM__Cb.html</anchorfile>
      <anchor>a08d74a0683124402d34b9978ec330d6</anchor>
      <arglist></arglist>
    </member>
    <member kind="variable">
      <type>unsigned int</type>
      <name>rinfo_resize_size</name>
      <anchorfile>structSM__Cb.html</anchorfile>
      <anchor>e756fdf8a8cde6bf0834e652a93737e4</anchor>
      <arglist></arglist>
    </member>
    <member kind="variable">
      <type>SM_Reg_Info *</type>
      <name>first_rinfo</name>
      <anchorfile>structSM__Cb.html</anchorfile>
      <anchor>f087221b562873167adfac33acfc6598</anchor>
      <arglist></arglist>
    </member>
    <member kind="variable">
      <type>SM_Reg_Info *</type>
      <name>last_rinfo</name>
      <anchorfile>structSM__Cb.html</anchorfile>
      <anchor>5ba44501a3e2f7b2c09f89c7d7250fb7</anchor>
      <arglist></arglist>
    </member>
    <member kind="variable">
      <type>unsigned int</type>
      <name>rinfo_count</name>
      <anchorfile>structSM__Cb.html</anchorfile>
      <anchor>38a351ec50272b49b4cb506e30d5b1fd</anchor>
      <arglist></arglist>
    </member>
    <member kind="variable">
      <type>SM_Reg_Info *</type>
      <name>mem_rinfo</name>
      <anchorfile>structSM__Cb.html</anchorfile>
      <anchor>0aad8090bf81f634fac7f9f2cf3fc0ee</anchor>
      <arglist></arglist>
    </member>
    <member kind="variable">
      <type>SM_Reg_Info *</type>
      <name>ctrl_rinfo</name>
      <anchorfile>structSM__Cb.html</anchorfile>
      <anchor>ebaf3cba412a86e6686137a2a20c7093</anchor>
      <arglist></arglist>
    </member>
    <member kind="variable">
      <type>SM_Reg_Info *</type>
      <name>sync_rinfo</name>
      <anchorfile>structSM__Cb.html</anchorfile>
      <anchor>565e60af95a2e57806efe6ed918fdf40</anchor>
      <arglist></arglist>
    </member>
    <member kind="variable">
      <type>SM_Reg_Info *</type>
      <name>vliw_rinfo</name>
      <anchorfile>structSM__Cb.html</anchorfile>
      <anchor>7d7c5196ba792bab7c58ff15a1c71f14</anchor>
      <arglist></arglist>
    </member>
    <member kind="variable">
      <type>unsigned int *</type>
      <name>map_array</name>
      <anchorfile>structSM__Cb.html</anchorfile>
      <anchor>cf4749b5f0d0c26981e60518e89ae61d</anchor>
      <arglist></arglist>
    </member>
    <member kind="variable">
      <type>int</type>
      <name>map_array_size</name>
      <anchorfile>structSM__Cb.html</anchorfile>
      <anchor>095e062dd8a34976b92ecb1d6a4c522a</anchor>
      <arglist></arglist>
    </member>
    <member kind="variable">
      <type>int</type>
      <name>map_start_offset</name>
      <anchorfile>structSM__Cb.html</anchorfile>
      <anchor>9cae533cbb0063cc9dc26e42aeb0e275</anchor>
      <arglist></arglist>
    </member>
    <member kind="variable">
      <type>int</type>
      <name>map_end_offset</name>
      <anchorfile>structSM__Cb.html</anchorfile>
      <anchor>5114802e5702d990222b07c2a97433a2</anchor>
      <arglist></arglist>
    </member>
    <member kind="variable">
      <type>int</type>
      <name>min_init_offset</name>
      <anchorfile>structSM__Cb.html</anchorfile>
      <anchor>b217c2bb03497221cc0be1290b9ada73</anchor>
      <arglist></arglist>
    </member>
    <member kind="variable">
      <type>int</type>
      <name>max_init_offset</name>
      <anchorfile>structSM__Cb.html</anchorfile>
      <anchor>940f293d1d2328d889b770008bd637e0</anchor>
      <arglist></arglist>
    </member>
    <member kind="variable">
      <type>int</type>
      <name>II</name>
      <anchorfile>structSM__Cb.html</anchorfile>
      <anchor>b361d1c3725a5b52fca50c4f365091c3</anchor>
      <arglist></arglist>
    </member>
    <member kind="variable">
      <type>int</type>
      <name>stages</name>
      <anchorfile>structSM__Cb.html</anchorfile>
      <anchor>fe843343aec8fe73be0f4b41367d2864</anchor>
      <arglist></arglist>
    </member>
    <member kind="variable">
      <type>int</type>
      <name>sched_cycle_offset</name>
      <anchorfile>structSM__Cb.html</anchorfile>
      <anchor>06f77339fe14ba232d6ca713565687b4</anchor>
      <arglist></arglist>
    </member>
    <member kind="variable">
      <type>List</type>
      <name>chk_list</name>
      <anchorfile>structSM__Cb.html</anchorfile>
      <anchor>392deec3b86fda0b39f70e45429bbca1</anchor>
      <arglist></arglist>
    </member>
  </compound>
  <compound kind="struct">
    <name>SM_Compatible_Alt</name>
    <filename>structSM__Compatible__Alt.html</filename>
    <member kind="variable">
      <type>Mdes_Alt *</type>
      <name>normal_version</name>
      <anchorfile>structSM__Compatible__Alt.html</anchorfile>
      <anchor>823b37888b5ac94b1fe75673f314d029</anchor>
      <arglist></arglist>
    </member>
    <member kind="variable">
      <type>Mdes_Alt *</type>
      <name>silent_version</name>
      <anchorfile>structSM__Compatible__Alt.html</anchorfile>
      <anchor>f9a7b176d024072b68bfe7d6df4532af</anchor>
      <arglist></arglist>
    </member>
    <member kind="variable">
      <type>SM_Compatible_Alt *</type>
      <name>next_compatible_alt</name>
      <anchorfile>structSM__Compatible__Alt.html</anchorfile>
      <anchor>4d89faaff7739bb212d2bb8b4920b671</anchor>
      <arglist></arglist>
    </member>
  </compound>
  <compound kind="struct">
    <name>SM_Dep</name>
    <filename>structSM__Dep.html</filename>
    <member kind="variable">
      <type>SM_Reg_Action *</type>
      <name>from_action</name>
      <anchorfile>structSM__Dep.html</anchorfile>
      <anchor>7986bc3d4adf6769d8bbfc26d6ad55b9</anchor>
      <arglist></arglist>
    </member>
    <member kind="variable">
      <type>SM_Reg_Action *</type>
      <name>to_action</name>
      <anchorfile>structSM__Dep.html</anchorfile>
      <anchor>27d6dcd9ece66927e168825df329e1c6</anchor>
      <arglist></arglist>
    </member>
    <member kind="variable">
      <type>unsigned int</type>
      <name>flags</name>
      <anchorfile>structSM__Dep.html</anchorfile>
      <anchor>6816b7a5b8cb7e6fcc2305b0c2147919</anchor>
      <arglist></arglist>
    </member>
    <member kind="variable">
      <type>unsigned int</type>
      <name>ignore</name>
      <anchorfile>structSM__Dep.html</anchorfile>
      <anchor>c86fd98a10649c5bc10931ef03c09bdf</anchor>
      <arglist></arglist>
    </member>
    <member kind="variable">
      <type>short</type>
      <name>delay_offset</name>
      <anchorfile>structSM__Dep.html</anchorfile>
      <anchor>f98cc20adbc6e56d6048bf5a6f220717</anchor>
      <arglist></arglist>
    </member>
    <member kind="variable">
      <type>short</type>
      <name>min_delay</name>
      <anchorfile>structSM__Dep.html</anchorfile>
      <anchor>e239c4ea463b4cb1917c0b2aab2f5668</anchor>
      <arglist></arglist>
    </member>
    <member kind="variable">
      <type>short</type>
      <name>max_delay</name>
      <anchorfile>structSM__Dep.html</anchorfile>
      <anchor>8fb6272113f4593e66721986cfa29510</anchor>
      <arglist></arglist>
    </member>
    <member kind="variable">
      <type>short</type>
      <name>omega</name>
      <anchorfile>structSM__Dep.html</anchorfile>
      <anchor>50dec9504610cd1b38099707f7102e50</anchor>
      <arglist></arglist>
    </member>
    <member kind="variable">
      <type>SM_Dep *</type>
      <name>next_dep_out</name>
      <anchorfile>structSM__Dep.html</anchorfile>
      <anchor>b504e52138aeabdcedf0a11fffa105a4</anchor>
      <arglist></arglist>
    </member>
    <member kind="variable">
      <type>SM_Dep *</type>
      <name>prev_dep_out</name>
      <anchorfile>structSM__Dep.html</anchorfile>
      <anchor>0d6b369ab84c7fa5fde4df61b2790a7d</anchor>
      <arglist></arglist>
    </member>
    <member kind="variable">
      <type>SM_Dep *</type>
      <name>next_dep_in</name>
      <anchorfile>structSM__Dep.html</anchorfile>
      <anchor>7db7e3db6b52d38f70d0a5e6d0b28b31</anchor>
      <arglist></arglist>
    </member>
    <member kind="variable">
      <type>SM_Dep *</type>
      <name>prev_dep_in</name>
      <anchorfile>structSM__Dep.html</anchorfile>
      <anchor>1d881fc5eb225709392943a9d3c4ef7e</anchor>
      <arglist></arglist>
    </member>
    <member kind="variable">
      <type>SM_Dep_PCLat *</type>
      <name>pclat_list</name>
      <anchorfile>structSM__Dep.html</anchorfile>
      <anchor>d5930ce5708bb998ccd9a387a6d1219f</anchor>
      <arglist></arglist>
    </member>
  </compound>
  <compound kind="struct">
    <name>SM_Dep_PCLat</name>
    <filename>structSM__Dep__PCLat.html</filename>
    <member kind="variable">
      <type>SM_PCLat *</type>
      <name>pclat</name>
      <anchorfile>structSM__Dep__PCLat.html</anchorfile>
      <anchor>b46b74aac44ae50a0c49619a0dc179c1</anchor>
      <arglist></arglist>
    </member>
    <member kind="variable">
      <type>int</type>
      <name>from_penalty</name>
      <anchorfile>structSM__Dep__PCLat.html</anchorfile>
      <anchor>6ffcd5ebf98a2f5fc491d698e22601b4</anchor>
      <arglist></arglist>
    </member>
    <member kind="variable">
      <type>int</type>
      <name>to_penalty</name>
      <anchorfile>structSM__Dep__PCLat.html</anchorfile>
      <anchor>631dbd857f75d9da9e8501a1aaa83da1</anchor>
      <arglist></arglist>
    </member>
    <member kind="variable">
      <type>SM_Dep_PCLat *</type>
      <name>next_dep_pclat</name>
      <anchorfile>structSM__Dep__PCLat.html</anchorfile>
      <anchor>b5717d76e9744904388cee89a0b806c4</anchor>
      <arglist></arglist>
    </member>
    <member kind="variable">
      <type>SM_Dep_PCLat *</type>
      <name>prev_dep_pclat</name>
      <anchorfile>structSM__Dep__PCLat.html</anchorfile>
      <anchor>359845ccec3d95c945bf0aa2bb8b8440</anchor>
      <arglist></arglist>
    </member>
  </compound>
  <compound kind="struct">
    <name>SM_Func</name>
    <filename>structSM__Func.html</filename>
    <member kind="variable">
      <type>SM_Cb *</type>
      <name>first_sm_cb</name>
      <anchorfile>structSM__Func.html</anchorfile>
      <anchor>78e4906e506aebacc0416993968dce0c</anchor>
      <arglist></arglist>
    </member>
    <member kind="variable">
      <type>SM_Cb *</type>
      <name>last_sm_cb</name>
      <anchorfile>structSM__Func.html</anchorfile>
      <anchor>67e607fc1adc93fc163c6c40c3ad4f8b</anchor>
      <arglist></arglist>
    </member>
  </compound>
  <compound kind="struct">
    <name>SM_Issue_Group</name>
    <filename>structSM__Issue__Group.html</filename>
    <member kind="variable">
      <type>SM_Cb *</type>
      <name>sm_cb</name>
      <anchorfile>structSM__Issue__Group.html</anchorfile>
      <anchor>2d7dde8e78d036c5060ad7cb20f39e79</anchor>
      <arglist></arglist>
    </member>
    <member kind="variable">
      <type>SM_Oper **</type>
      <name>slots</name>
      <anchorfile>structSM__Issue__Group.html</anchorfile>
      <anchor>43a77662558615c0ce71fb00073256f8</anchor>
      <arglist></arglist>
    </member>
    <member kind="variable">
      <type>int</type>
      <name>issue_time</name>
      <anchorfile>structSM__Issue__Group.html</anchorfile>
      <anchor>1b8ee51d7efb47f583c4a3662a98822f</anchor>
      <arglist></arglist>
    </member>
    <member kind="variable">
      <type>int</type>
      <name>full</name>
      <anchorfile>structSM__Issue__Group.html</anchorfile>
      <anchor>b932eceb18bc3102ac23c903e54f2001</anchor>
      <arglist></arglist>
    </member>
    <member kind="variable">
      <type>int</type>
      <name>num_slots_left</name>
      <anchorfile>structSM__Issue__Group.html</anchorfile>
      <anchor>7995072a4ad0045747b9c806f09267e1</anchor>
      <arglist></arglist>
    </member>
    <member kind="variable">
      <type>SM_Bundle **</type>
      <name>bundles</name>
      <anchorfile>structSM__Issue__Group.html</anchorfile>
      <anchor>cd936f7ae752b556a57a3127714e6527</anchor>
      <arglist></arglist>
    </member>
    <member kind="variable">
      <type>SM_Issue_Group *</type>
      <name>next_issue_group</name>
      <anchorfile>structSM__Issue__Group.html</anchorfile>
      <anchor>be548524f26d12be024c14c4d7974ed8</anchor>
      <arglist></arglist>
    </member>
    <member kind="variable">
      <type>SM_Issue_Group *</type>
      <name>prev_issue_group</name>
      <anchorfile>structSM__Issue__Group.html</anchorfile>
      <anchor>d85a155e2ef85ec4cd48e1b50b51b8bf</anchor>
      <arglist></arglist>
    </member>
  </compound>
  <compound kind="struct">
    <name>SM_Oper</name>
    <filename>structSM__Oper.html</filename>
    <member kind="variable">
      <type>L_Oper *</type>
      <name>lcode_op</name>
      <anchorfile>structSM__Oper.html</anchorfile>
      <anchor>3f5397d6d9a8bf4c7d31e4dd0219f6a9</anchor>
      <arglist></arglist>
    </member>
    <member kind="variable">
      <type>SM_Cb *</type>
      <name>sm_cb</name>
      <anchorfile>structSM__Oper.html</anchorfile>
      <anchor>403cdc247cfbc617d985413b7ecd95cb</anchor>
      <arglist></arglist>
    </member>
    <member kind="variable">
      <type>unsigned int</type>
      <name>flags</name>
      <anchorfile>structSM__Oper.html</anchorfile>
      <anchor>b938730250455390f086f1e0894307cb</anchor>
      <arglist></arglist>
    </member>
    <member kind="variable">
      <type>unsigned int</type>
      <name>ignore</name>
      <anchorfile>structSM__Oper.html</anchorfile>
      <anchor>ea27c7c4c11584a233d1e942650fcae8</anchor>
      <arglist></arglist>
    </member>
    <member kind="variable">
      <type>unsigned int</type>
      <name>mdes_flags</name>
      <anchorfile>structSM__Oper.html</anchorfile>
      <anchor>955a3f98b7a93c8dfcffb172c7adfd2d</anchor>
      <arglist></arglist>
    </member>
    <member kind="variable">
      <type>Mdes_Operation *</type>
      <name>mdes_op</name>
      <anchorfile>structSM__Oper.html</anchorfile>
      <anchor>0c05f7bf0d24815e07745910ad5fc936</anchor>
      <arglist></arglist>
    </member>
    <member kind="variable">
      <type>SM_Issue_Group *</type>
      <name>issue_group</name>
      <anchorfile>structSM__Oper.html</anchorfile>
      <anchor>30ceca3cbca884200232f768c8c5143f</anchor>
      <arglist></arglist>
    </member>
    <member kind="variable">
      <type>unsigned int</type>
      <name>syll_type</name>
      <anchorfile>structSM__Oper.html</anchorfile>
      <anchor>ac480d24762da9aab38d3fd6148581f0</anchor>
      <arglist></arglist>
    </member>
    <member kind="variable">
      <type>int</type>
      <name>old_issue_time</name>
      <anchorfile>structSM__Oper.html</anchorfile>
      <anchor>7a66e454c58bf7783e69e070909c728e</anchor>
      <arglist></arglist>
    </member>
    <member kind="variable">
      <type>SM_Priority_Qentry *</type>
      <name>qentry</name>
      <anchorfile>structSM__Oper.html</anchorfile>
      <anchor>277d1f09ea9934fce54adb87e12b8b4c</anchor>
      <arglist></arglist>
    </member>
    <member kind="variable">
      <type>SM_Compatible_Alt *</type>
      <name>first_compatible_alt</name>
      <anchorfile>structSM__Oper.html</anchorfile>
      <anchor>c420786597163342db80eaff3016b25a</anchor>
      <arglist></arglist>
    </member>
    <member kind="variable">
      <type>SM_Reg_Action **</type>
      <name>operand</name>
      <anchorfile>structSM__Oper.html</anchorfile>
      <anchor>b1258af603e4f98fbb9689c2316ea189</anchor>
      <arglist></arglist>
    </member>
    <member kind="variable">
      <type>SM_Reg_Action **</type>
      <name>dest</name>
      <anchorfile>structSM__Oper.html</anchorfile>
      <anchor>dd1e1bbe73565b3d3f0557ba3ba22c27</anchor>
      <arglist></arglist>
    </member>
    <member kind="variable">
      <type>SM_Reg_Action **</type>
      <name>src</name>
      <anchorfile>structSM__Oper.html</anchorfile>
      <anchor>846635adfafd7385659fb4be6a64c19a</anchor>
      <arglist></arglist>
    </member>
    <member kind="variable">
      <type>SM_Reg_Action **</type>
      <name>pred</name>
      <anchorfile>structSM__Oper.html</anchorfile>
      <anchor>ff6cbdc4c8f79caeb8b581ea0986ae49</anchor>
      <arglist></arglist>
    </member>
    <member kind="variable">
      <type>SM_Reg_Action **</type>
      <name>ext_dest</name>
      <anchorfile>structSM__Oper.html</anchorfile>
      <anchor>fc6e609ae82e06f3089cb4206909e6ba</anchor>
      <arglist></arglist>
    </member>
    <member kind="variable">
      <type>SM_Reg_Action **</type>
      <name>ext_src</name>
      <anchorfile>structSM__Oper.html</anchorfile>
      <anchor>b28625350558f9e8c88d1fa132c12cd7</anchor>
      <arglist></arglist>
    </member>
    <member kind="variable">
      <type>SM_Action_Queue *</type>
      <name>implicit_dests</name>
      <anchorfile>structSM__Oper.html</anchorfile>
      <anchor>a00653e48edf0628a7053f6e69b04f75</anchor>
      <arglist></arglist>
    </member>
    <member kind="variable">
      <type>SM_Action_Queue *</type>
      <name>implicit_srcs</name>
      <anchorfile>structSM__Oper.html</anchorfile>
      <anchor>01dfae863b2431beb1b879a95edd643f</anchor>
      <arglist></arglist>
    </member>
    <member kind="variable">
      <type>SM_Reg_Action *</type>
      <name>first_op_action</name>
      <anchorfile>structSM__Oper.html</anchorfile>
      <anchor>57586b443ee0ac219ebbdec855fd410d</anchor>
      <arglist></arglist>
    </member>
    <member kind="variable">
      <type>SM_Reg_Action *</type>
      <name>last_op_action</name>
      <anchorfile>structSM__Oper.html</anchorfile>
      <anchor>b8e915432b09702be750f1d10b621ac2</anchor>
      <arglist></arglist>
    </member>
    <member kind="variable">
      <type>float</type>
      <name>priority</name>
      <anchorfile>structSM__Oper.html</anchorfile>
      <anchor>5aa3c81ed0f0094af0e20dfb3f5b55ec</anchor>
      <arglist></arglist>
    </member>
    <member kind="variable">
      <type>int</type>
      <name>early_time</name>
      <anchorfile>structSM__Oper.html</anchorfile>
      <anchor>c8d5069084f567c7924ec7580eaaa833</anchor>
      <arglist></arglist>
    </member>
    <member kind="variable">
      <type>int *</type>
      <name>late_time</name>
      <anchorfile>structSM__Oper.html</anchorfile>
      <anchor>a73177483eb55fcdc5314413e58a8849</anchor>
      <arglist></arglist>
    </member>
    <member kind="variable">
      <type>int</type>
      <name>sched_cycle</name>
      <anchorfile>structSM__Oper.html</anchorfile>
      <anchor>9adb1e3b27a12ac7ae0ed7ad7002dce2</anchor>
      <arglist></arglist>
    </member>
    <member kind="variable">
      <type>unsigned short</type>
      <name>sched_slot</name>
      <anchorfile>structSM__Oper.html</anchorfile>
      <anchor>76c47ef1a4635d5f4a6abf01561b9b6d</anchor>
      <arglist></arglist>
    </member>
    <member kind="variable">
      <type>SM_Compatible_Alt *</type>
      <name>alt_chosen</name>
      <anchorfile>structSM__Oper.html</anchorfile>
      <anchor>1e593e8a98cf660f13cd461d8f412d11</anchor>
      <arglist></arglist>
    </member>
    <member kind="variable">
      <type>unsigned short *</type>
      <name>options_chosen</name>
      <anchorfile>structSM__Oper.html</anchorfile>
      <anchor>ee24a0ee4e13aeed089cad53d05ebf8b</anchor>
      <arglist></arglist>
    </member>
    <member kind="variable">
      <type>int</type>
      <name>cycle_lower_bound</name>
      <anchorfile>structSM__Oper.html</anchorfile>
      <anchor>a622e9cddfe71da8f749333ccf381862</anchor>
      <arglist></arglist>
    </member>
    <member kind="variable">
      <type>int</type>
      <name>cycle_upper_bound</name>
      <anchorfile>structSM__Oper.html</anchorfile>
      <anchor>6715c4bd874ae9ff47a755a8f734cbed</anchor>
      <arglist></arglist>
    </member>
    <member kind="variable">
      <type>int</type>
      <name>nosoft_cycle_lower_bound</name>
      <anchorfile>structSM__Oper.html</anchorfile>
      <anchor>1c6c71b87dcb457d7eba20472f369c06</anchor>
      <arglist></arglist>
    </member>
    <member kind="variable">
      <type>int</type>
      <name>nosoft_cycle_upper_bound</name>
      <anchorfile>structSM__Oper.html</anchorfile>
      <anchor>b0b2162707f2d2353bbfd81aeb63306f</anchor>
      <arglist></arglist>
    </member>
    <member kind="variable">
      <type>unsigned short</type>
      <name>slot_lower_bound</name>
      <anchorfile>structSM__Oper.html</anchorfile>
      <anchor>ff8fb2da8e1c3e36a40d595ad06ac02b</anchor>
      <arglist></arglist>
    </member>
    <member kind="variable">
      <type>unsigned short</type>
      <name>slot_upper_bound</name>
      <anchorfile>structSM__Oper.html</anchorfile>
      <anchor>cecc21a58ca12155be0c3983bfab50b1</anchor>
      <arglist></arglist>
    </member>
    <member kind="variable">
      <type>unsigned short</type>
      <name>nosoft_slot_lower_bound</name>
      <anchorfile>structSM__Oper.html</anchorfile>
      <anchor>1013bd86b1e2c4d8caf1ca049ff9de40</anchor>
      <arglist></arglist>
    </member>
    <member kind="variable">
      <type>unsigned short</type>
      <name>nosoft_slot_upper_bound</name>
      <anchorfile>structSM__Oper.html</anchorfile>
      <anchor>59775e1a1961e03b11c51c444cfb8e3a</anchor>
      <arglist></arglist>
    </member>
    <member kind="variable">
      <type>SM_Dep *</type>
      <name>dep_lower_bound</name>
      <anchorfile>structSM__Oper.html</anchorfile>
      <anchor>410b7d69dfd65383d215e25692930e95</anchor>
      <arglist></arglist>
    </member>
    <member kind="variable">
      <type>SM_Dep *</type>
      <name>dep_upper_bound</name>
      <anchorfile>structSM__Oper.html</anchorfile>
      <anchor>75e1e2451fd779e70a8d01220fa43f5c</anchor>
      <arglist></arglist>
    </member>
    <member kind="variable">
      <type>SM_Dep *</type>
      <name>nosoft_dep_lower_bound</name>
      <anchorfile>structSM__Oper.html</anchorfile>
      <anchor>127f60d42a12576c40075b3bcf2798b6</anchor>
      <arglist></arglist>
    </member>
    <member kind="variable">
      <type>SM_Dep *</type>
      <name>nosoft_dep_upper_bound</name>
      <anchorfile>structSM__Oper.html</anchorfile>
      <anchor>64cce2f2a581465243bb5f255fb271a9</anchor>
      <arglist></arglist>
    </member>
    <member kind="variable">
      <type>unsigned short</type>
      <name>num_hard_dep_in</name>
      <anchorfile>structSM__Oper.html</anchorfile>
      <anchor>e964f777cd6923a656361d66091298eb</anchor>
      <arglist></arglist>
    </member>
    <member kind="variable">
      <type>unsigned short</type>
      <name>num_unresolved_hard_dep_in</name>
      <anchorfile>structSM__Oper.html</anchorfile>
      <anchor>73c2a689bae98259322eb6a5446578f3</anchor>
      <arglist></arglist>
    </member>
    <member kind="variable">
      <type>unsigned short</type>
      <name>num_soft_dep_in</name>
      <anchorfile>structSM__Oper.html</anchorfile>
      <anchor>c78a6c50e136e8afc118f50ec968a080</anchor>
      <arglist></arglist>
    </member>
    <member kind="variable">
      <type>unsigned short</type>
      <name>num_unresolved_soft_dep_in</name>
      <anchorfile>structSM__Oper.html</anchorfile>
      <anchor>abf8bf5fd4044148307bf48115132793</anchor>
      <arglist></arglist>
    </member>
    <member kind="variable">
      <type>unsigned short</type>
      <name>num_ignore_dep_in</name>
      <anchorfile>structSM__Oper.html</anchorfile>
      <anchor>caf219fe5befc1cfa5ce43ecbb1cae06</anchor>
      <arglist></arglist>
    </member>
    <member kind="variable">
      <type>unsigned short</type>
      <name>num_unresolved_ignore_dep_in</name>
      <anchorfile>structSM__Oper.html</anchorfile>
      <anchor>0ec5065f3b1dcec9f4e0d62a29be239a</anchor>
      <arglist></arglist>
    </member>
    <member kind="variable">
      <type>unsigned short</type>
      <name>num_hard_dep_out</name>
      <anchorfile>structSM__Oper.html</anchorfile>
      <anchor>3aa843e2a209b8d7b0941a8a2f29825c</anchor>
      <arglist></arglist>
    </member>
    <member kind="variable">
      <type>unsigned short</type>
      <name>num_unresolved_hard_dep_out</name>
      <anchorfile>structSM__Oper.html</anchorfile>
      <anchor>6e2707ee5106c548dc4d139eceda61b4</anchor>
      <arglist></arglist>
    </member>
    <member kind="variable">
      <type>unsigned short</type>
      <name>num_soft_dep_out</name>
      <anchorfile>structSM__Oper.html</anchorfile>
      <anchor>ba11306cd2deb5c8d37db759b40299f1</anchor>
      <arglist></arglist>
    </member>
    <member kind="variable">
      <type>unsigned short</type>
      <name>num_unresolved_soft_dep_out</name>
      <anchorfile>structSM__Oper.html</anchorfile>
      <anchor>df8dc5a84b4ad343ce4909fd6b9fb024</anchor>
      <arglist></arglist>
    </member>
    <member kind="variable">
      <type>unsigned short</type>
      <name>num_ignore_dep_out</name>
      <anchorfile>structSM__Oper.html</anchorfile>
      <anchor>385d77d58e8c853b10c3029f3c716738</anchor>
      <arglist></arglist>
    </member>
    <member kind="variable">
      <type>unsigned short</type>
      <name>num_unresolved_ignore_dep_out</name>
      <anchorfile>structSM__Oper.html</anchorfile>
      <anchor>ec05e926de01e251cd2db071621ad63b</anchor>
      <arglist></arglist>
    </member>
    <member kind="variable">
      <type>SM_Oper *</type>
      <name>next_sched_op</name>
      <anchorfile>structSM__Oper.html</anchorfile>
      <anchor>1d05314da63eaba41305d86c117fe254</anchor>
      <arglist></arglist>
    </member>
    <member kind="variable">
      <type>SM_Oper *</type>
      <name>prev_sched_op</name>
      <anchorfile>structSM__Oper.html</anchorfile>
      <anchor>5db482713f081b7b64be4d07c08c4a43</anchor>
      <arglist></arglist>
    </member>
    <member kind="variable">
      <type>SM_Oper_Qentry *</type>
      <name>first_queue</name>
      <anchorfile>structSM__Oper.html</anchorfile>
      <anchor>6171eae5e43cbc84a32c2d211af1ed56</anchor>
      <arglist></arglist>
    </member>
    <member kind="variable">
      <type>SM_Oper_Qentry *</type>
      <name>dep_in_resolved_qentry</name>
      <anchorfile>structSM__Oper.html</anchorfile>
      <anchor>d67fe993f486c0f81a0bb69a808c0010</anchor>
      <arglist></arglist>
    </member>
    <member kind="variable">
      <type>unsigned int</type>
      <name>serial_number</name>
      <anchorfile>structSM__Oper.html</anchorfile>
      <anchor>a47229bc64cad7c5428ea9f51079f3d7</anchor>
      <arglist></arglist>
    </member>
    <member kind="variable">
      <type>SM_Oper *</type>
      <name>next_serial_op</name>
      <anchorfile>structSM__Oper.html</anchorfile>
      <anchor>c7c35fc98ddf3d343118f16a6bfd0f99</anchor>
      <arglist></arglist>
    </member>
    <member kind="variable">
      <type>SM_Oper *</type>
      <name>prev_serial_op</name>
      <anchorfile>structSM__Oper.html</anchorfile>
      <anchor>88143ecbdab16d214af38460be533d5a</anchor>
      <arglist></arglist>
    </member>
    <member kind="variable">
      <type>int</type>
      <name>temp_height</name>
      <anchorfile>structSM__Oper.html</anchorfile>
      <anchor>893f3f71424ca7586cefe7b562c1a910</anchor>
      <arglist></arglist>
    </member>
    <member kind="variable">
      <type>int</type>
      <name>liverange_reduced</name>
      <anchorfile>structSM__Oper.html</anchorfile>
      <anchor>e3a55118ce3bbc2c0686b5ef6a724ffc</anchor>
      <arglist></arglist>
    </member>
  </compound>
  <compound kind="struct">
    <name>SM_Oper_Qentry</name>
    <filename>structSM__Oper__Qentry.html</filename>
    <member kind="variable">
      <type>SM_Oper_Queue *</type>
      <name>queue</name>
      <anchorfile>structSM__Oper__Qentry.html</anchorfile>
      <anchor>d3939ac9579c1573c1a5979412baa60d</anchor>
      <arglist></arglist>
    </member>
    <member kind="variable">
      <type>SM_Oper *</type>
      <name>sm_op</name>
      <anchorfile>structSM__Oper__Qentry.html</anchorfile>
      <anchor>c7acbaad49f5d5dd9b0d9a20eab4ddc6</anchor>
      <arglist></arglist>
    </member>
    <member kind="variable">
      <type>SM_Oper_Qentry *</type>
      <name>next_qentry</name>
      <anchorfile>structSM__Oper__Qentry.html</anchorfile>
      <anchor>18f8ae6743db367a6e487fa74ac19dde</anchor>
      <arglist></arglist>
    </member>
    <member kind="variable">
      <type>SM_Oper_Qentry *</type>
      <name>prev_qentry</name>
      <anchorfile>structSM__Oper__Qentry.html</anchorfile>
      <anchor>89b2b9b686959ae4f42dcd526aadc6e5</anchor>
      <arglist></arglist>
    </member>
    <member kind="variable">
      <type>SM_Oper_Qentry *</type>
      <name>next_queue</name>
      <anchorfile>structSM__Oper__Qentry.html</anchorfile>
      <anchor>a871cd1a89cd042a6d48d973b54cc10a</anchor>
      <arglist></arglist>
    </member>
    <member kind="variable">
      <type>SM_Oper_Qentry *</type>
      <name>prev_queue</name>
      <anchorfile>structSM__Oper__Qentry.html</anchorfile>
      <anchor>261960b20c793bbde2963ecc1db2122c</anchor>
      <arglist></arglist>
    </member>
  </compound>
  <compound kind="struct">
    <name>SM_Oper_Queue</name>
    <filename>structSM__Oper__Queue.html</filename>
    <member kind="variable">
      <type>int</type>
      <name>num_qentries</name>
      <anchorfile>structSM__Oper__Queue.html</anchorfile>
      <anchor>39c4cbbf5c76e3bab81acce0585af524</anchor>
      <arglist></arglist>
    </member>
    <member kind="variable">
      <type>SM_Oper_Qentry *</type>
      <name>first_qentry</name>
      <anchorfile>structSM__Oper__Queue.html</anchorfile>
      <anchor>7045e014fcca8fcbde2a616ddcde1ba5</anchor>
      <arglist></arglist>
    </member>
    <member kind="variable">
      <type>SM_Oper_Qentry *</type>
      <name>last_qentry</name>
      <anchorfile>structSM__Oper__Queue.html</anchorfile>
      <anchor>e834dc233573151e9d56e55f5bfa5fd2</anchor>
      <arglist></arglist>
    </member>
  </compound>
  <compound kind="struct">
    <name>SM_Priority_Qentry</name>
    <filename>structSM__Priority__Qentry.html</filename>
    <member kind="variable">
      <type>SM_Priority_Queue *</type>
      <name>queue</name>
      <anchorfile>structSM__Priority__Qentry.html</anchorfile>
      <anchor>9804306d5e98aad2cdc2229a1fe84e6f</anchor>
      <arglist></arglist>
    </member>
    <member kind="variable">
      <type>SM_Oper *</type>
      <name>oper</name>
      <anchorfile>structSM__Priority__Qentry.html</anchorfile>
      <anchor>bcfeab23670b8b0d8e7b3360a26b0c8c</anchor>
      <arglist></arglist>
    </member>
    <member kind="variable">
      <type>int</type>
      <name>priority</name>
      <anchorfile>structSM__Priority__Qentry.html</anchorfile>
      <anchor>46159e5c42ed9e035ff1f0783e3ee4de</anchor>
      <arglist></arglist>
    </member>
    <member kind="variable">
      <type>int</type>
      <name>scheduled</name>
      <anchorfile>structSM__Priority__Qentry.html</anchorfile>
      <anchor>3ca20b8635dabd840366f3ab6d1f9dd6</anchor>
      <arglist></arglist>
    </member>
    <member kind="variable">
      <type>SM_Priority_Qentry *</type>
      <name>next_qentry</name>
      <anchorfile>structSM__Priority__Qentry.html</anchorfile>
      <anchor>711d1acf6e9fd39258a53d9d068c02fd</anchor>
      <arglist></arglist>
    </member>
    <member kind="variable">
      <type>SM_Priority_Qentry *</type>
      <name>prev_qentry</name>
      <anchorfile>structSM__Priority__Qentry.html</anchorfile>
      <anchor>707c268c5b98fe24bf501ba9aae01054</anchor>
      <arglist></arglist>
    </member>
  </compound>
  <compound kind="struct">
    <name>SM_Priority_Queue</name>
    <filename>structSM__Priority__Queue.html</filename>
    <member kind="variable">
      <type>int</type>
      <name>num_qentries</name>
      <anchorfile>structSM__Priority__Queue.html</anchorfile>
      <anchor>2bf33424f2666c8b12cbf486da3c0bc2</anchor>
      <arglist></arglist>
    </member>
    <member kind="variable">
      <type>int</type>
      <name>num_not_sched</name>
      <anchorfile>structSM__Priority__Queue.html</anchorfile>
      <anchor>6ad775a4ed644a3a4437d2c2281de428</anchor>
      <arglist></arglist>
    </member>
    <member kind="variable">
      <type>SM_Priority_Qentry *</type>
      <name>first_qentry</name>
      <anchorfile>structSM__Priority__Queue.html</anchorfile>
      <anchor>575997cff1924caf12a0f37b10129458</anchor>
      <arglist></arglist>
    </member>
    <member kind="variable">
      <type>SM_Priority_Qentry *</type>
      <name>last_qentry</name>
      <anchorfile>structSM__Priority__Queue.html</anchorfile>
      <anchor>9159c27fcfdd2cbbd6cb4ec5f12ac8c8</anchor>
      <arglist></arglist>
    </member>
  </compound>
  <compound kind="struct">
    <name>SM_Reg_Action</name>
    <filename>structSM__Reg__Action.html</filename>
    <member kind="variable">
      <type>SM_Oper *</type>
      <name>sm_op</name>
      <anchorfile>structSM__Reg__Action.html</anchorfile>
      <anchor>870ffbe025aeb52318e352bf64d9d155</anchor>
      <arglist></arglist>
    </member>
    <member kind="variable">
      <type>unsigned char</type>
      <name>operand_type</name>
      <anchorfile>structSM__Reg__Action.html</anchorfile>
      <anchor>60bf89801ad704cec9e462ba8fb91d8a</anchor>
      <arglist></arglist>
    </member>
    <member kind="variable">
      <type>unsigned char</type>
      <name>operand_number</name>
      <anchorfile>structSM__Reg__Action.html</anchorfile>
      <anchor>d21140e4086d8df1175d7f8f3bdaea9b</anchor>
      <arglist></arglist>
    </member>
    <member kind="variable">
      <type>unsigned short</type>
      <name>index</name>
      <anchorfile>structSM__Reg__Action.html</anchorfile>
      <anchor>9b11e4447ad230d0825409b64566dbfe</anchor>
      <arglist></arglist>
    </member>
    <member kind="variable">
      <type>unsigned int</type>
      <name>flags</name>
      <anchorfile>structSM__Reg__Action.html</anchorfile>
      <anchor>044a14479275d4b8c12a0f4e7ce56b55</anchor>
      <arglist></arglist>
    </member>
    <member kind="variable">
      <type>int</type>
      <name>add_lat</name>
      <anchorfile>structSM__Reg__Action.html</anchorfile>
      <anchor>437d754244063da270915df91ee27934</anchor>
      <arglist></arglist>
    </member>
    <member kind="variable">
      <type>SM_Dep *</type>
      <name>first_dep_in</name>
      <anchorfile>structSM__Reg__Action.html</anchorfile>
      <anchor>60b920d4ec6481af314c5d8a35b08e41</anchor>
      <arglist></arglist>
    </member>
    <member kind="variable">
      <type>SM_Dep *</type>
      <name>first_dep_out</name>
      <anchorfile>structSM__Reg__Action.html</anchorfile>
      <anchor>3171073306749483352a4c892cfc6221</anchor>
      <arglist></arglist>
    </member>
    <member kind="variable">
      <type>short</type>
      <name>min_early_use_time</name>
      <anchorfile>structSM__Reg__Action.html</anchorfile>
      <anchor>3480dcff32f9aeb383e3d0431d84ef81</anchor>
      <arglist></arglist>
    </member>
    <member kind="variable">
      <type>short</type>
      <name>max_early_use_time</name>
      <anchorfile>structSM__Reg__Action.html</anchorfile>
      <anchor>6d6db71fdebaeb45b81c6ed21c99f032</anchor>
      <arglist></arglist>
    </member>
    <member kind="variable">
      <type>short</type>
      <name>actual_early_use_time</name>
      <anchorfile>structSM__Reg__Action.html</anchorfile>
      <anchor>7285afff7979f3311269c7d22c73ba9b</anchor>
      <arglist></arglist>
    </member>
    <member kind="variable">
      <type>short</type>
      <name>min_late_use_time</name>
      <anchorfile>structSM__Reg__Action.html</anchorfile>
      <anchor>c56db9a11864e0af91683c8701f90e53</anchor>
      <arglist></arglist>
    </member>
    <member kind="variable">
      <type>short</type>
      <name>max_late_use_time</name>
      <anchorfile>structSM__Reg__Action.html</anchorfile>
      <anchor>24a10d6c7047f177524ebd08eea8e680</anchor>
      <arglist></arglist>
    </member>
    <member kind="variable">
      <type>short</type>
      <name>actual_late_use_time</name>
      <anchorfile>structSM__Reg__Action.html</anchorfile>
      <anchor>e9611cfa885a4eb80b1f0c959e871189</anchor>
      <arglist></arglist>
    </member>
    <member kind="variable">
      <type>SM_Reg_Action **</type>
      <name>conflict</name>
      <anchorfile>structSM__Reg__Action.html</anchorfile>
      <anchor>0a1d5a48ef292c3e2cc521dafcdea177</anchor>
      <arglist></arglist>
    </member>
    <member kind="variable">
      <type>SM_Reg_Info *</type>
      <name>rinfo</name>
      <anchorfile>structSM__Reg__Action.html</anchorfile>
      <anchor>b770af67e23a6a7e754f356f7cc54239</anchor>
      <arglist></arglist>
    </member>
    <member kind="variable">
      <type>SM_Reg_Action *</type>
      <name>next_complete</name>
      <anchorfile>structSM__Reg__Action.html</anchorfile>
      <anchor>83117d8b4a4223e579c3b7be95cc7159</anchor>
      <arglist></arglist>
    </member>
    <member kind="variable">
      <type>SM_Reg_Action *</type>
      <name>prev_complete</name>
      <anchorfile>structSM__Reg__Action.html</anchorfile>
      <anchor>5d4626df3ee1fb7f03a8405c134ea12e</anchor>
      <arglist></arglist>
    </member>
    <member kind="variable">
      <type>SM_Reg_Action *</type>
      <name>next_actual</name>
      <anchorfile>structSM__Reg__Action.html</anchorfile>
      <anchor>4708d9852aa225295999137441b952cb</anchor>
      <arglist></arglist>
    </member>
    <member kind="variable">
      <type>SM_Reg_Action *</type>
      <name>prev_actual</name>
      <anchorfile>structSM__Reg__Action.html</anchorfile>
      <anchor>a740020a7074f0974378b99df71b037a</anchor>
      <arglist></arglist>
    </member>
    <member kind="variable">
      <type>SM_Reg_Action *</type>
      <name>next_def</name>
      <anchorfile>structSM__Reg__Action.html</anchorfile>
      <anchor>ecf1671bd6765cbf0c4825cdc7961e22</anchor>
      <arglist></arglist>
    </member>
    <member kind="variable">
      <type>SM_Reg_Action *</type>
      <name>prev_def</name>
      <anchorfile>structSM__Reg__Action.html</anchorfile>
      <anchor>4d87439df7fca0c303934c3f907e9310</anchor>
      <arglist></arglist>
    </member>
    <member kind="variable">
      <type>SM_Reg_Action *</type>
      <name>next_op_action</name>
      <anchorfile>structSM__Reg__Action.html</anchorfile>
      <anchor>006ea32b06272e80d00b880e2858a926</anchor>
      <arglist></arglist>
    </member>
    <member kind="variable">
      <type>SM_Reg_Action *</type>
      <name>prev_op_action</name>
      <anchorfile>structSM__Reg__Action.html</anchorfile>
      <anchor>508db3822dcfcedbb4efaaeaf4285446</anchor>
      <arglist></arglist>
    </member>
    <member kind="variable">
      <type>SM_Action_Qentry *</type>
      <name>first_queue</name>
      <anchorfile>structSM__Reg__Action.html</anchorfile>
      <anchor>a15721c62a62911cdcf634c7d5037c5e</anchor>
      <arglist></arglist>
    </member>
  </compound>
  <compound kind="struct">
    <name>SM_Reg_Info</name>
    <filename>structSM__Reg__Info.html</filename>
    <member kind="variable">
      <type>int</type>
      <name>id</name>
      <anchorfile>structSM__Reg__Info.html</anchorfile>
      <anchor>23b30480d0481b359631e35c775e42b4</anchor>
      <arglist></arglist>
    </member>
    <member kind="variable">
      <type>int</type>
      <name>type</name>
      <anchorfile>structSM__Reg__Info.html</anchorfile>
      <anchor>944a16145044b276848166b3cea174ca</anchor>
      <arglist></arglist>
    </member>
    <member kind="variable">
      <type>L_Operand *</type>
      <name>operand</name>
      <anchorfile>structSM__Reg__Info.html</anchorfile>
      <anchor>fee9e2fbd5d24289998e62d79a6561c0</anchor>
      <arglist></arglist>
    </member>
    <member kind="variable">
      <type>unsigned int</type>
      <name>flags</name>
      <anchorfile>structSM__Reg__Info.html</anchorfile>
      <anchor>cc6c87b09dda66c631f99f14af2b4faf</anchor>
      <arglist></arglist>
    </member>
    <member kind="variable">
      <type>SM_Reg_Info **</type>
      <name>reg_conflict</name>
      <anchorfile>structSM__Reg__Info.html</anchorfile>
      <anchor>b62ccd279dbf78a64e463b5eb1491909</anchor>
      <arglist></arglist>
    </member>
    <member kind="variable">
      <type>int</type>
      <name>num_conflicts</name>
      <anchorfile>structSM__Reg__Info.html</anchorfile>
      <anchor>af638eded0480ece2b073140c379c52b</anchor>
      <arglist></arglist>
    </member>
    <member kind="variable">
      <type>SM_Reg_Action *</type>
      <name>first_complete</name>
      <anchorfile>structSM__Reg__Info.html</anchorfile>
      <anchor>bf8165a7fbe09d60dd2d25b081d2b04b</anchor>
      <arglist></arglist>
    </member>
    <member kind="variable">
      <type>SM_Reg_Action *</type>
      <name>last_complete</name>
      <anchorfile>structSM__Reg__Info.html</anchorfile>
      <anchor>07f59e3d4df318d78cde125e67376d89</anchor>
      <arglist></arglist>
    </member>
    <member kind="variable">
      <type>SM_Reg_Action *</type>
      <name>first_actual</name>
      <anchorfile>structSM__Reg__Info.html</anchorfile>
      <anchor>6d45f625f2116215745e76eaf5d2c360</anchor>
      <arglist></arglist>
    </member>
    <member kind="variable">
      <type>SM_Reg_Action *</type>
      <name>last_actual</name>
      <anchorfile>structSM__Reg__Info.html</anchorfile>
      <anchor>6f7108a753f9a4ac398e16848b829e45</anchor>
      <arglist></arglist>
    </member>
    <member kind="variable">
      <type>SM_Reg_Action *</type>
      <name>first_def</name>
      <anchorfile>structSM__Reg__Info.html</anchorfile>
      <anchor>656ce766bc8b2ab1cfe66cdac38abad6</anchor>
      <arglist></arglist>
    </member>
    <member kind="variable">
      <type>SM_Reg_Action *</type>
      <name>last_def</name>
      <anchorfile>structSM__Reg__Info.html</anchorfile>
      <anchor>a4c8fb7263f45b41a7e219d86f622316</anchor>
      <arglist></arglist>
    </member>
    <member kind="variable">
      <type>SM_Cb *</type>
      <name>sm_cb</name>
      <anchorfile>structSM__Reg__Info.html</anchorfile>
      <anchor>c9359f521d4dee89b4345cf079e4ac88</anchor>
      <arglist></arglist>
    </member>
    <member kind="variable">
      <type>SM_Reg_Info *</type>
      <name>next_hash</name>
      <anchorfile>structSM__Reg__Info.html</anchorfile>
      <anchor>cae556b454116e98afd58185126e58d4</anchor>
      <arglist></arglist>
    </member>
    <member kind="variable">
      <type>SM_Reg_Info *</type>
      <name>prev_hash</name>
      <anchorfile>structSM__Reg__Info.html</anchorfile>
      <anchor>dd665c782fa682963003908ada8ac8f6</anchor>
      <arglist></arglist>
    </member>
    <member kind="variable">
      <type>SM_Reg_Info *</type>
      <name>next_rinfo</name>
      <anchorfile>structSM__Reg__Info.html</anchorfile>
      <anchor>2446859580363af04b76afda0ca19ebc</anchor>
      <arglist></arglist>
    </member>
    <member kind="variable">
      <type>SM_Reg_Info *</type>
      <name>prev_rinfo</name>
      <anchorfile>structSM__Reg__Info.html</anchorfile>
      <anchor>dd3a89dfa7004cd72668e19e23f79300</anchor>
      <arglist></arglist>
    </member>
    <member kind="variable">
      <type>void *</type>
      <name>ext</name>
      <anchorfile>structSM__Reg__Info.html</anchorfile>
      <anchor>11ee7d90e3f3209eaa22a581cdd6cb5e</anchor>
      <arglist></arglist>
    </member>
  </compound>
  <compound kind="struct">
    <name>SM_Stats</name>
    <filename>structSM__Stats.html</filename>
    <member kind="variable">
      <type>int</type>
      <name>num_oper_checks</name>
      <anchorfile>structSM__Stats.html</anchorfile>
      <anchor>3a9724da430494d56e62e3373982bfe9</anchor>
      <arglist></arglist>
    </member>
    <member kind="variable">
      <type>int</type>
      <name>num_oper_checks_failed</name>
      <anchorfile>structSM__Stats.html</anchorfile>
      <anchor>3ca8d2ba10392c2ea32477df9002aa0d</anchor>
      <arglist></arglist>
    </member>
    <member kind="variable">
      <type>int</type>
      <name>num_table_checks</name>
      <anchorfile>structSM__Stats.html</anchorfile>
      <anchor>c0e20732a5c8fce7e01db2ab062d5d0f</anchor>
      <arglist></arglist>
    </member>
    <member kind="variable">
      <type>int</type>
      <name>num_table_checks_failed</name>
      <anchorfile>structSM__Stats.html</anchorfile>
      <anchor>8215f21d8b537392cf36e7db366b33cb</anchor>
      <arglist></arglist>
    </member>
    <member kind="variable">
      <type>int</type>
      <name>num_usage_checks</name>
      <anchorfile>structSM__Stats.html</anchorfile>
      <anchor>a9ccb6c8bc03422377e3dd898c3d16ce</anchor>
      <arglist></arglist>
    </member>
    <member kind="variable">
      <type>int</type>
      <name>num_usage_checks_failed</name>
      <anchorfile>structSM__Stats.html</anchorfile>
      <anchor>1ed2e3a63e05ad993ce61504d3229411</anchor>
      <arglist></arglist>
    </member>
    <member kind="variable">
      <type>int</type>
      <name>num_slot_checks</name>
      <anchorfile>structSM__Stats.html</anchorfile>
      <anchor>9f6ce50001fe0f7cc22ea2eac721eaed</anchor>
      <arglist></arglist>
    </member>
    <member kind="variable">
      <type>int</type>
      <name>num_slot_checks_failed</name>
      <anchorfile>structSM__Stats.html</anchorfile>
      <anchor>93edbba0a4b11e4f136bffeaf8e7527a</anchor>
      <arglist></arglist>
    </member>
  </compound>
  <compound kind="struct">
    <name>SM_Trans</name>
    <filename>structSM__Trans.html</filename>
    <member kind="variable">
      <type>int</type>
      <name>cb_id</name>
      <anchorfile>structSM__Trans.html</anchorfile>
      <anchor>6b3dce41a4aa2bf9b4970fe7a309f00d</anchor>
      <arglist></arglist>
    </member>
    <member kind="variable">
      <type>int</type>
      <name>op_id</name>
      <anchorfile>structSM__Trans.html</anchorfile>
      <anchor>4463b7e927193c405155d0215c5aa294</anchor>
      <arglist></arglist>
    </member>
    <member kind="variable">
      <type>int</type>
      <name>type</name>
      <anchorfile>structSM__Trans.html</anchorfile>
      <anchor>f895d5e5079f866effc3445a6664adf4</anchor>
      <arglist></arglist>
    </member>
    <member kind="variable">
      <type>unsigned int</type>
      <name>flags</name>
      <anchorfile>structSM__Trans.html</anchorfile>
      <anchor>be64302185ff5bfd3405bb7448b96d0f</anchor>
      <arglist></arglist>
    </member>
    <member kind="variable">
      <type>SM_Oper *</type>
      <name>target_sm_op</name>
      <anchorfile>structSM__Trans.html</anchorfile>
      <anchor>01ee7fea38cf82bf729fefd788a1e646</anchor>
      <arglist></arglist>
    </member>
    <member kind="variable">
      <type>SM_Oper *</type>
      <name>def_sm_op</name>
      <anchorfile>structSM__Trans.html</anchorfile>
      <anchor>6c620bed41f3c754313d432fe924de43</anchor>
      <arglist></arglist>
    </member>
    <member kind="variable">
      <type>SM_Oper *</type>
      <name>def2_sm_op</name>
      <anchorfile>structSM__Trans.html</anchorfile>
      <anchor>9d23a3b39fada796791a00b95a4aafc0</anchor>
      <arglist></arglist>
    </member>
    <member kind="variable">
      <type>short</type>
      <name>target_index</name>
      <anchorfile>structSM__Trans.html</anchorfile>
      <anchor>5b6f37c5c816b587c72a2599dbb14c52</anchor>
      <arglist></arglist>
    </member>
    <member kind="variable">
      <type>short</type>
      <name>other_index</name>
      <anchorfile>structSM__Trans.html</anchorfile>
      <anchor>b25588befd1ad8ad184491835c032344</anchor>
      <arglist></arglist>
    </member>
    <member kind="variable">
      <type>int</type>
      <name>target_sched_cycle</name>
      <anchorfile>structSM__Trans.html</anchorfile>
      <anchor>305b5fbaca49c8b2e89d172ce5244097</anchor>
      <arglist></arglist>
    </member>
    <member kind="variable">
      <type>SM_Oper *</type>
      <name>orig_prev_serial_op</name>
      <anchorfile>structSM__Trans.html</anchorfile>
      <anchor>bb3f50868f92c9f58e8fc4e1c93f659a</anchor>
      <arglist></arglist>
    </member>
    <member kind="variable">
      <type>SM_Oper *</type>
      <name>new_sm_op</name>
      <anchorfile>structSM__Trans.html</anchorfile>
      <anchor>5733d1733063e2ba1369e8769bd81156</anchor>
      <arglist></arglist>
    </member>
    <member kind="variable">
      <type>SM_Oper *</type>
      <name>renaming_sm_op</name>
      <anchorfile>structSM__Trans.html</anchorfile>
      <anchor>9739718d35f05f06efb3400d20636f5c</anchor>
      <arglist></arglist>
    </member>
    <member kind="variable">
      <type>L_Operand *</type>
      <name>orig_src</name>
      <anchorfile>structSM__Trans.html</anchorfile>
      <anchor>9e0e6e752d1eb47f7c1abbcff8f93e7d</anchor>
      <arglist>[2]</arglist>
    </member>
    <member kind="variable">
      <type>int</type>
      <name>orig_opc</name>
      <anchorfile>structSM__Trans.html</anchorfile>
      <anchor>0af554b117996ccbe1f1f78ca54c475f</anchor>
      <arglist></arglist>
    </member>
    <member kind="variable">
      <type>int</type>
      <name>orig_proc_opc</name>
      <anchorfile>structSM__Trans.html</anchorfile>
      <anchor>f9fe48308ab6476431c7686609d12fdf</anchor>
      <arglist></arglist>
    </member>
    <member kind="variable">
      <type>ITintmax</type>
      <name>orig_ext</name>
      <anchorfile>structSM__Trans.html</anchorfile>
      <anchor>cc63d1e62b3954e3ea8ff5b65b228ec4</anchor>
      <arglist></arglist>
    </member>
    <member kind="variable">
      <type>L_Operand *</type>
      <name>orig_def_src</name>
      <anchorfile>structSM__Trans.html</anchorfile>
      <anchor>30cae0276d276bbe8b003c068bb7eadf</anchor>
      <arglist>[2]</arglist>
    </member>
    <member kind="variable">
      <type>int</type>
      <name>orig_def_opc</name>
      <anchorfile>structSM__Trans.html</anchorfile>
      <anchor>62a24cb7bff216bb9db0b45eaa651ade</anchor>
      <arglist></arglist>
    </member>
    <member kind="variable">
      <type>int</type>
      <name>orig_def_proc_opc</name>
      <anchorfile>structSM__Trans.html</anchorfile>
      <anchor>b43865aebc8033193daeaf56f105f72e</anchor>
      <arglist></arglist>
    </member>
    <member kind="variable">
      <type>ITintmax</type>
      <name>orig_def_ext</name>
      <anchorfile>structSM__Trans.html</anchorfile>
      <anchor>48ddcf2baf1a20498e202eebfe69ac7a</anchor>
      <arglist></arglist>
    </member>
    <member kind="variable">
      <type>SM_Oper *</type>
      <name>orig_def_prev_serial_op</name>
      <anchorfile>structSM__Trans.html</anchorfile>
      <anchor>ea2acad3876b0c876fb4c3d9bacc0bde</anchor>
      <arglist></arglist>
    </member>
    <member kind="variable">
      <type>L_Oper *</type>
      <name>deleted_lcode_op</name>
      <anchorfile>structSM__Trans.html</anchorfile>
      <anchor>f9a39ffa112c5f5824af7120ffe4bcff</anchor>
      <arglist></arglist>
    </member>
    <member kind="variable">
      <type>L_Oper *</type>
      <name>deleted_def_lcode_op</name>
      <anchorfile>structSM__Trans.html</anchorfile>
      <anchor>c27be472ad311be6a99d1c60bc507f34</anchor>
      <arglist></arglist>
    </member>
    <member kind="variable">
      <type>SM_Trans_Qentry *</type>
      <name>first_queue</name>
      <anchorfile>structSM__Trans.html</anchorfile>
      <anchor>25e9d4c20507d8675d1561958e19d032</anchor>
      <arglist></arglist>
    </member>
  </compound>
  <compound kind="struct">
    <name>SM_Trans_Qentry</name>
    <filename>structSM__Trans__Qentry.html</filename>
    <member kind="variable">
      <type>SM_Trans_Queue *</type>
      <name>queue</name>
      <anchorfile>structSM__Trans__Qentry.html</anchorfile>
      <anchor>cfa046dd64450c983cc40e4f0f63c50d</anchor>
      <arglist></arglist>
    </member>
    <member kind="variable">
      <type>SM_Trans *</type>
      <name>trans</name>
      <anchorfile>structSM__Trans__Qentry.html</anchorfile>
      <anchor>fc747c98cb9fadb82bb7dd4eff529107</anchor>
      <arglist></arglist>
    </member>
    <member kind="variable">
      <type>SM_Trans_Qentry *</type>
      <name>next_qentry</name>
      <anchorfile>structSM__Trans__Qentry.html</anchorfile>
      <anchor>01768e8e776fe02520e8700b4494546c</anchor>
      <arglist></arglist>
    </member>
    <member kind="variable">
      <type>SM_Trans_Qentry *</type>
      <name>prev_qentry</name>
      <anchorfile>structSM__Trans__Qentry.html</anchorfile>
      <anchor>f2a659bd3cf2018c7514fa412b16aae8</anchor>
      <arglist></arglist>
    </member>
    <member kind="variable">
      <type>SM_Trans_Qentry *</type>
      <name>next_queue</name>
      <anchorfile>structSM__Trans__Qentry.html</anchorfile>
      <anchor>6ae99c106f7f944f33a0bdec7e0c2d55</anchor>
      <arglist></arglist>
    </member>
    <member kind="variable">
      <type>SM_Trans_Qentry *</type>
      <name>prev_queue</name>
      <anchorfile>structSM__Trans__Qentry.html</anchorfile>
      <anchor>cbd3e992799e6c62ccbb7ae33c6eafef</anchor>
      <arglist></arglist>
    </member>
  </compound>
  <compound kind="struct">
    <name>SM_Trans_Queue</name>
    <filename>structSM__Trans__Queue.html</filename>
    <member kind="variable">
      <type>char *</type>
      <name>name</name>
      <anchorfile>structSM__Trans__Queue.html</anchorfile>
      <anchor>183575e0f8e7ff75fbf9fecf1226abad</anchor>
      <arglist></arglist>
    </member>
    <member kind="variable">
      <type>int</type>
      <name>num_qentries</name>
      <anchorfile>structSM__Trans__Queue.html</anchorfile>
      <anchor>4c8a6817cf1dbf06bf3ab63d25bb0db7</anchor>
      <arglist></arglist>
    </member>
    <member kind="variable">
      <type>SM_Trans_Qentry *</type>
      <name>first_qentry</name>
      <anchorfile>structSM__Trans__Queue.html</anchorfile>
      <anchor>659b0deb38829d60c76b561f5d1fffd4</anchor>
      <arglist></arglist>
    </member>
    <member kind="variable">
      <type>SM_Trans_Qentry *</type>
      <name>last_qentry</name>
      <anchorfile>structSM__Trans__Queue.html</anchorfile>
      <anchor>38336748d7786e86a5e429818ced83b5</anchor>
      <arglist></arglist>
    </member>
  </compound>
</tagfile>
