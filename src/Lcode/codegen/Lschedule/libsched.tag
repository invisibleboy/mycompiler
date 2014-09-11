<?xml version='1.0' encoding='ISO-8859-1' standalone='yes' ?>
<tagfile>
  <compound kind="file">
    <name>l_delay_slots.c</name>
    <path>/mnt/hgfs/mkt/Desktop/openimpact/src/Lcode/codegen/Lschedule/</path>
    <filename>l__delay__slots_8c</filename>
    <includes id="l__schedule_8h" name="l_schedule.h" local="yes" imported="no">l_schedule.h</includes>
    <member kind="function" static="yes">
      <type>static L_Cb *</type>
      <name>L_alter_branch_destination</name>
      <anchorfile>l__delay__slots_8c.html</anchorfile>
      <anchor>9da40d4878761584579198f37f3a2528</anchor>
      <arglist>(L_Func *fn, L_Cb *cb, L_Cb *dest_cb, L_Oper *br, L_Oper *filler)</arglist>
    </member>
    <member kind="function" static="yes">
      <type>static int</type>
      <name>L_valid_branch_filler</name>
      <anchorfile>l__delay__slots_8c.html</anchorfile>
      <anchor>14aa11b42e71eebed3abe58e18b35125</anchor>
      <arglist>(L_Oper *br, L_Oper *filler)</arglist>
    </member>
    <member kind="function">
      <type>void</type>
      <name>Lsched_fill_from_target</name>
      <anchorfile>l__delay__slots_8c.html</anchorfile>
      <anchor>44e02966abab06e75570cb8888121db8</anchor>
      <arglist>(L_Func *fn)</arglist>
    </member>
    <member kind="function">
      <type>void</type>
      <name>Lsched_fill_nonsquashing_slots</name>
      <anchorfile>l__delay__slots_8c.html</anchorfile>
      <anchor>b61e607fcd85186dbba09d495c658696</anchor>
      <arglist>(L_Func *fn)</arglist>
    </member>
    <member kind="function">
      <type>void</type>
      <name>Lsched_fill_nonsquashing_branches</name>
      <anchorfile>l__delay__slots_8c.html</anchorfile>
      <anchor>64e73713da82b24bb5934505eeb64c45</anchor>
      <arglist>(L_Func *fn)</arglist>
    </member>
    <member kind="function">
      <type>void</type>
      <name>Lsched_fill_squashing_branches</name>
      <anchorfile>l__delay__slots_8c.html</anchorfile>
      <anchor>1575d9431756a2b1b0193d363fcaad7c</anchor>
      <arglist>(L_Func *fn)</arglist>
    </member>
    <member kind="function">
      <type>void</type>
      <name>Lsched_fill_unfilled_branches</name>
      <anchorfile>l__delay__slots_8c.html</anchorfile>
      <anchor>b19c2b3371e7f6c65e56e81a3f5f2f16</anchor>
      <arglist>(L_Func *fn)</arglist>
    </member>
    <member kind="function">
      <type>int</type>
      <name>Lsched_handle_spilled_branch</name>
      <anchorfile>l__delay__slots_8c.html</anchorfile>
      <anchor>6de719e078b25892d78ade0a72a72e1f</anchor>
      <arglist>(L_Cb *cb, Sched_Info *sinfo)</arglist>
    </member>
    <member kind="function">
      <type>int</type>
      <name>Lsched_update_ctl_delay_sinfo</name>
      <anchorfile>l__delay__slots_8c.html</anchorfile>
      <anchor>157c81f39e21bd9b32433e03030430ce</anchor>
      <arglist>(Squeue *queue, Sched_Info *sinfo)</arglist>
    </member>
    <member kind="function">
      <type>void</type>
      <name>Lsched_check_for_fill_candidates</name>
      <anchorfile>l__delay__slots_8c.html</anchorfile>
      <anchor>ce6a81e6030547cf9c25b9bf1927480c</anchor>
      <arglist>(Squeue *queue)</arglist>
    </member>
    <member kind="function">
      <type>void</type>
      <name>Lsched_schedule_with_delay_op</name>
      <anchorfile>l__delay__slots_8c.html</anchorfile>
      <anchor>36e2138d405088ad97ffca6e7832e401</anchor>
      <arglist>(L_Cb *cb, Sched_Info *sinfo, int current_time, int current_block, int issue_slot, int spec)</arglist>
    </member>
  </compound>
  <compound kind="file">
    <name>l_dependence.c</name>
    <path>/mnt/hgfs/mkt/Desktop/openimpact/src/Lcode/codegen/Lschedule/</path>
    <filename>l__dependence_8c</filename>
    <includes id="l__dependence_8h" name="l_dependence.h" local="yes" imported="no">l_dependence.h</includes>
    <includes id="l__schedule_8h" name="l_schedule.h" local="yes" imported="no">l_schedule.h</includes>
    <member kind="define">
      <type>#define</type>
      <name>DEP_MEM_OPERAND</name>
      <anchorfile>l__dependence_8c.html</anchorfile>
      <anchor>a1a43eadd7bb31302abdeb28f606497b</anchor>
      <arglist></arglist>
    </member>
    <member kind="define">
      <type>#define</type>
      <name>DEP_CNT_OPERAND</name>
      <anchorfile>l__dependence_8c.html</anchorfile>
      <anchor>ed4f1e9cee538335bc3a17a0c73241c3</anchor>
      <arglist></arglist>
    </member>
    <member kind="define">
      <type>#define</type>
      <name>DEP_SYNC_OPERAND</name>
      <anchorfile>l__dependence_8c.html</anchorfile>
      <anchor>14c0cbe25fab7704fb5d4b491c096dab</anchor>
      <arglist></arglist>
    </member>
    <member kind="define">
      <type>#define</type>
      <name>DEP_VLIW_OPERAND</name>
      <anchorfile>l__dependence_8c.html</anchorfile>
      <anchor>3326c6a5979f6eb3961e3d2142cd9b7f</anchor>
      <arglist></arglist>
    </member>
    <member kind="function" static="yes">
      <type>static void</type>
      <name>L_read_parm_ldep</name>
      <anchorfile>l__dependence_8c.html</anchorfile>
      <anchor>935fc35d7f7e8ebcf966ab4053168aa3</anchor>
      <arglist>(Parm_Parse_Info *ppi)</arglist>
    </member>
    <member kind="function">
      <type>void</type>
      <name>Ldep_init</name>
      <anchorfile>l__dependence_8c.html</anchorfile>
      <anchor>f86ee8daba5e7fa68b9b230cfee0ebe0</anchor>
      <arglist>(Parm_Macro_List *command_line_macro_list)</arglist>
    </member>
    <member kind="function">
      <type>void</type>
      <name>Ldep_change_ctl_dep_level_same_iter</name>
      <anchorfile>l__dependence_8c.html</anchorfile>
      <anchor>8668b200bd600d31de9483dcfdc65fd2</anchor>
      <arglist>(L_Oper *load_oper)</arglist>
    </member>
    <member kind="function">
      <type>int</type>
      <name>Ldep_invariant_oper</name>
      <anchorfile>l__dependence_8c.html</anchorfile>
      <anchor>948fc124a08d861c6a4e501a82ee58c4</anchor>
      <arglist>(L_Cb *cb, L_Oper *load_oper)</arglist>
    </member>
    <member kind="function" static="yes">
      <type>static int</type>
      <name>Ldep_uncond_def</name>
      <anchorfile>l__dependence_8c.html</anchorfile>
      <anchor>7f64ec615959ab19c308b7c52fa93eec</anchor>
      <arglist>(L_Operand *def_operand)</arglist>
    </member>
    <member kind="function" static="yes">
      <type>static int</type>
      <name>Ldep_defs_may_reorder</name>
      <anchorfile>l__dependence_8c.html</anchorfile>
      <anchor>682c50c8c07e0aec9286565007237ca6</anchor>
      <arglist>(Dep_Operand *dep_dest1, L_Operand *dest2)</arglist>
    </member>
    <member kind="function">
      <type>void</type>
      <name>Ldep_mark_safe_invariant</name>
      <anchorfile>l__dependence_8c.html</anchorfile>
      <anchor>1f244cfa861039836212d125b1a5e399</anchor>
      <arglist>(L_Func *fn)</arglist>
    </member>
    <member kind="function">
      <type>int</type>
      <name>L_safe_operands</name>
      <anchorfile>l__dependence_8c.html</anchorfile>
      <anchor>c1a24229bc1d954aebb8e9185946f3f1</anchor>
      <arglist>(L_Operand *reg_src, L_Operand *reg_dest, L_Operand *immed_src, L_Operand *immed_dest)</arglist>
    </member>
    <member kind="function">
      <type>int</type>
      <name>L_safe_load</name>
      <anchorfile>l__dependence_8c.html</anchorfile>
      <anchor>430a6d43709bb20d872c3e66699c09f2</anchor>
      <arglist>(L_Oper *load_oper, L_Oper *br_oper, L_Cb *cb)</arglist>
    </member>
    <member kind="function">
      <type>void</type>
      <name>Ldep_mark_safe</name>
      <anchorfile>l__dependence_8c.html</anchorfile>
      <anchor>eb5239833bb75d328e2e84dd49f420f0</anchor>
      <arglist>(L_Func *fn)</arglist>
    </member>
    <member kind="function">
      <type>int</type>
      <name>L_is_always_safe</name>
      <anchorfile>l__dependence_8c.html</anchorfile>
      <anchor>3edd827a868dd2511ca07c63805cf9ad</anchor>
      <arglist>(L_Oper *oper)</arglist>
    </member>
    <member kind="function">
      <type>int</type>
      <name>L_is_ctl_dep_safe</name>
      <anchorfile>l__dependence_8c.html</anchorfile>
      <anchor>80d98b26470385b84a121260baac6fd4</anchor>
      <arglist>(L_Oper *oper, L_Oper *br, L_Cb *cb)</arglist>
    </member>
    <member kind="function">
      <type>int</type>
      <name>L_safe_divide</name>
      <anchorfile>l__dependence_8c.html</anchorfile>
      <anchor>4ec7e635172f1105374c3845236ee264</anchor>
      <arglist>(L_Oper *oper, L_Oper *br, L_Cb *cb)</arglist>
    </member>
    <member kind="function">
      <type>int</type>
      <name>L_safe_rem</name>
      <anchorfile>l__dependence_8c.html</anchorfile>
      <anchor>09a5d5ce15b9f46918a83040d75832b0</anchor>
      <arglist>(L_Oper *oper, L_Oper *br, L_Cb *cb)</arglist>
    </member>
    <member kind="function">
      <type>int</type>
      <name>L_is_complex_safe</name>
      <anchorfile>l__dependence_8c.html</anchorfile>
      <anchor>26c981ff12249ed47a841e376b0f7d26</anchor>
      <arglist>(L_Oper *oper, L_Oper *br, L_Cb *cb)</arglist>
    </member>
    <member kind="function" static="yes">
      <type>static void</type>
      <name>L_init_dep_hash_table</name>
      <anchorfile>l__dependence_8c.html</anchorfile>
      <anchor>0d12c84bf72375f9068c8204ae3f34ae</anchor>
      <arglist>()</arglist>
    </member>
    <member kind="function" static="yes">
      <type>static void</type>
      <name>L_reset_dep_hash_table</name>
      <anchorfile>l__dependence_8c.html</anchorfile>
      <anchor>03b4cfd22ea7fd297e696ea6edcb6dcc</anchor>
      <arglist>()</arglist>
    </member>
    <member kind="function" static="yes">
      <type>static Dep_Operand *</type>
      <name>L_find_dep_operand</name>
      <anchorfile>l__dependence_8c.html</anchorfile>
      <anchor>242a2eeabb1652403af84b38c21e2d9d</anchor>
      <arglist>(L_Operand *operand)</arglist>
    </member>
    <member kind="function" static="yes">
      <type>static Dep_Operand *</type>
      <name>L_find_another_dep_operand</name>
      <anchorfile>l__dependence_8c.html</anchorfile>
      <anchor>26eed6ee75763820634212e8a116b79e</anchor>
      <arglist>(Dep_Operand *dep)</arglist>
    </member>
    <member kind="function" static="yes">
      <type>static void</type>
      <name>L_insert_dep_operand</name>
      <anchorfile>l__dependence_8c.html</anchorfile>
      <anchor>6cd90e727232f28cc6016f32a2dcbb32</anchor>
      <arglist>(int index, L_Operand *operand, L_Oper *oper)</arglist>
    </member>
    <member kind="function" static="yes">
      <type>static void</type>
      <name>L_remove_dep_operand</name>
      <anchorfile>l__dependence_8c.html</anchorfile>
      <anchor>5076f563e1903d57d7d790e9a39b1d2f</anchor>
      <arglist>(Dep_Operand *dep_operand)</arglist>
    </member>
    <member kind="function" static="yes">
      <type>static void</type>
      <name>L_add_input_dep</name>
      <anchorfile>l__dependence_8c.html</anchorfile>
      <anchor>5fd87f446901a7d61585eb99224c2b99</anchor>
      <arglist>(int type, int dist, int from_index, int to_index, L_Oper *from_oper, L_Oper *to_oper, int omega)</arglist>
    </member>
    <member kind="function" static="yes">
      <type>static void</type>
      <name>L_add_output_dep</name>
      <anchorfile>l__dependence_8c.html</anchorfile>
      <anchor>bf35d9d70fc1843ce96406273a599371</anchor>
      <arglist>(int type, int dist, int from_index, int to_index, L_Oper *from_oper, L_Oper *to_oper, int omega)</arglist>
    </member>
    <member kind="function">
      <type>void</type>
      <name>L_add_dep</name>
      <anchorfile>l__dependence_8c.html</anchorfile>
      <anchor>8a84f01e6521a557413c06468f1faf33</anchor>
      <arglist>(int type, int dist, int from_index, int to_index, L_Oper *from_oper, L_Oper *to_oper, int omega)</arglist>
    </member>
    <member kind="function">
      <type>L_Dep *</type>
      <name>L_find_dep</name>
      <anchorfile>l__dependence_8c.html</anchorfile>
      <anchor>f0ec9a2ae2398967737febf3f6bde466</anchor>
      <arglist>(L_Dep *dep, L_Oper *from_oper, L_Oper *to_oper, int type)</arglist>
    </member>
    <member kind="function">
      <type>L_Dep *</type>
      <name>L_remove_dep</name>
      <anchorfile>l__dependence_8c.html</anchorfile>
      <anchor>64456aa8e72a2caac9f3f45b002c89f8</anchor>
      <arglist>(L_Dep *dep, int *count, int type, L_Oper *from_oper, L_Oper *to_oper)</arglist>
    </member>
    <member kind="function">
      <type>void</type>
      <name>L_remove_dep_pair</name>
      <anchorfile>l__dependence_8c.html</anchorfile>
      <anchor>aae239e60591b789360532409a5547d0</anchor>
      <arglist>(int type, L_Oper *from_oper, L_Oper *to_oper)</arglist>
    </member>
    <member kind="function" static="yes">
      <type>static int</type>
      <name>L_same_register_operand</name>
      <anchorfile>l__dependence_8c.html</anchorfile>
      <anchor>79934227948b374b0bc8390f3d43e44a</anchor>
      <arglist>(L_Operand *op1, L_Operand *op2)</arglist>
    </member>
    <member kind="function" static="yes">
      <type>static int</type>
      <name>L_jsr_independent_oper</name>
      <anchorfile>l__dependence_8c.html</anchorfile>
      <anchor>a451ec761845e8b8d2f946aeab814fb4</anchor>
      <arglist>(L_Oper *jsr, L_Oper *oper)</arglist>
    </member>
    <member kind="function" static="yes">
      <type>static int</type>
      <name>L_safe_to_move_below_branch</name>
      <anchorfile>l__dependence_8c.html</anchorfile>
      <anchor>30631f51de7d743f9858bf3b4c867cf0</anchor>
      <arglist>(L_Cb *cb, L_Oper *oper, L_Oper *branch)</arglist>
    </member>
    <member kind="function" static="yes">
      <type>static int</type>
      <name>L_safe_to_move_below_branch_hb</name>
      <anchorfile>l__dependence_8c.html</anchorfile>
      <anchor>3407e8c379d563c292f0359b5ecbce28</anchor>
      <arglist>(L_Cb *cb, L_Oper *oper, L_Oper *branch)</arglist>
    </member>
    <member kind="function" static="yes">
      <type>static int</type>
      <name>L_safe_to_move_above_branch</name>
      <anchorfile>l__dependence_8c.html</anchorfile>
      <anchor>33aca5dc58648aa7d62b7a93178ce690</anchor>
      <arglist>(L_Cb *cb, L_Oper *oper, L_Oper *branch)</arglist>
    </member>
    <member kind="function" static="yes">
      <type>static int</type>
      <name>L_safe_to_move_above_branch_hb</name>
      <anchorfile>l__dependence_8c.html</anchorfile>
      <anchor>2b0d68d4fa57007b599bbdded3d70ef5</anchor>
      <arglist>(L_Cb *cb, L_Oper *oper, L_Oper *branch)</arglist>
    </member>
    <member kind="function" static="yes">
      <type>static void</type>
      <name>L_compute_register_dependence</name>
      <anchorfile>l__dependence_8c.html</anchorfile>
      <anchor>ddf74edbb73637b030c329169d91c526</anchor>
      <arglist>(L_Cb *cb, int prepass)</arglist>
    </member>
    <member kind="function" static="yes">
      <type>static void</type>
      <name>L_compute_cross_iter_register_dependence</name>
      <anchorfile>l__dependence_8c.html</anchorfile>
      <anchor>19ed66f522ef76bc785bcf1b6900c6f8</anchor>
      <arglist>(L_Cb *cb, int prepass)</arglist>
    </member>
    <member kind="function" static="yes">
      <type>static void</type>
      <name>L_compute_memory_dependence</name>
      <anchorfile>l__dependence_8c.html</anchorfile>
      <anchor>12c318e082de47185dec0af388596878</anchor>
      <arglist>(L_Cb *cb)</arglist>
    </member>
    <member kind="function" static="yes">
      <type>static void</type>
      <name>L_compute_cross_iter_memory_dependence</name>
      <anchorfile>l__dependence_8c.html</anchorfile>
      <anchor>0cbb9aeb94a9a195899995d0ad84eff7</anchor>
      <arglist>(L_Cb *cb)</arglist>
    </member>
    <member kind="function" static="yes">
      <type>static void</type>
      <name>L_compute_control_dependence</name>
      <anchorfile>l__dependence_8c.html</anchorfile>
      <anchor>2f283aa12bb18774c6525fb5743b279c</anchor>
      <arglist>(L_Cb *cb, int prepass)</arglist>
    </member>
    <member kind="function" static="yes">
      <type>static void</type>
      <name>L_compute_synchronization_dependence</name>
      <anchorfile>l__dependence_8c.html</anchorfile>
      <anchor>8d6cef7c54348a134629e30b2857df21</anchor>
      <arglist>(L_Cb *cb)</arglist>
    </member>
    <member kind="function" static="yes">
      <type>static void</type>
      <name>L_compute_hb_register_dependence</name>
      <anchorfile>l__dependence_8c.html</anchorfile>
      <anchor>59ec57ac8c9dda963160a4d9fac94c99</anchor>
      <arglist>(L_Cb *cb, int prepass)</arglist>
    </member>
    <member kind="function" static="yes">
      <type>static void</type>
      <name>L_compute_hb_memory_dependence</name>
      <anchorfile>l__dependence_8c.html</anchorfile>
      <anchor>40faaf1cf134688037d4aca47f0c147e</anchor>
      <arglist>(L_Cb *cb)</arglist>
    </member>
    <member kind="function" static="yes">
      <type>static void</type>
      <name>L_compute_hb_control_dependence</name>
      <anchorfile>l__dependence_8c.html</anchorfile>
      <anchor>6f15173bce843360caa512e3086abbb1</anchor>
      <arglist>(L_Cb *cb, int prepass)</arglist>
    </member>
    <member kind="function" static="yes">
      <type>static void</type>
      <name>L_compute_hb_synchronization_dependence</name>
      <anchorfile>l__dependence_8c.html</anchorfile>
      <anchor>027c6769ca5eacf63ffd6121574cebf9</anchor>
      <arglist>(L_Cb *cb)</arglist>
    </member>
    <member kind="function" static="yes">
      <type>static void</type>
      <name>L_compute_hb_blk_dependence</name>
      <anchorfile>l__dependence_8c.html</anchorfile>
      <anchor>ba1575c76d3c5b4033780287ea048cbb</anchor>
      <arglist>(L_Cb *cb)</arglist>
    </member>
    <member kind="function" static="yes">
      <type>static void</type>
      <name>L_resolve_dependence</name>
      <anchorfile>l__dependence_8c.html</anchorfile>
      <anchor>56894a0f24c76910fce610ba3b58c237</anchor>
      <arglist>(L_Cb *cb)</arglist>
    </member>
    <member kind="function" static="yes">
      <type>static void</type>
      <name>L_allow_concurrent_issue</name>
      <anchorfile>l__dependence_8c.html</anchorfile>
      <anchor>ddfb04cbdf5e137545d118b72af1702a</anchor>
      <arglist>(L_Cb *cb)</arglist>
    </member>
    <member kind="function">
      <type>void</type>
      <name>L_compute_mem_copy_dependences</name>
      <anchorfile>l__dependence_8c.html</anchorfile>
      <anchor>bdce56f36cb41bb9afab71582742e1d3</anchor>
      <arglist>(L_Cb *cb)</arglist>
    </member>
    <member kind="function">
      <type>void</type>
      <name>L_compute_dependence_level</name>
      <anchorfile>l__dependence_8c.html</anchorfile>
      <anchor>f669d306bab65f7ef2c088b8c7b36d0b</anchor>
      <arglist>(L_Cb *cb)</arglist>
    </member>
    <member kind="function">
      <type>void</type>
      <name>L_delete_dependence_info</name>
      <anchorfile>l__dependence_8c.html</anchorfile>
      <anchor>5388c5ae76618fb82066c22de862641f</anchor>
      <arglist>(Dep_Info *dep_info)</arglist>
    </member>
    <member kind="function">
      <type>void</type>
      <name>L_delete_dependence_graph</name>
      <anchorfile>l__dependence_8c.html</anchorfile>
      <anchor>d1437aabcc35bdaeb457e33113f3d2e5</anchor>
      <arglist>(L_Cb *cb)</arglist>
    </member>
    <member kind="function">
      <type>void</type>
      <name>L_print_dependences</name>
      <anchorfile>l__dependence_8c.html</anchorfile>
      <anchor>86ca16e95abb5f67556626e04e57d5ea</anchor>
      <arglist>(FILE *F, L_Dep *dep)</arglist>
    </member>
    <member kind="function">
      <type>void</type>
      <name>L_print_dependence_graph</name>
      <anchorfile>l__dependence_8c.html</anchorfile>
      <anchor>585e04c6115d5b91dd8005ef9cd031fa</anchor>
      <arglist>(FILE *F, L_Cb *cb)</arglist>
    </member>
    <member kind="function">
      <type>Dep_Info *</type>
      <name>L_new_dep_info</name>
      <anchorfile>l__dependence_8c.html</anchorfile>
      <anchor>36f50b716086eefdc8b0f60e72e98f71</anchor>
      <arglist>()</arglist>
    </member>
    <member kind="function">
      <type>void</type>
      <name>L_insert_program_order_dependence</name>
      <anchorfile>l__dependence_8c.html</anchorfile>
      <anchor>d1e1fb1c56a781712c05ed6a373e32ef</anchor>
      <arglist>(L_Cb *cb)</arglist>
    </member>
    <member kind="function">
      <type>void</type>
      <name>L_build_dependence_graph</name>
      <anchorfile>l__dependence_8c.html</anchorfile>
      <anchor>68d25f7aa698c6f1e9f8205eeda8173c</anchor>
      <arglist>(L_Cb *cb, int prepass, int mode)</arglist>
    </member>
    <member kind="variable">
      <type>int</type>
      <name>Ldep_option</name>
      <anchorfile>l__dependence_8c.html</anchorfile>
      <anchor>1486b324e7da3513f46008bf5df24297</anchor>
      <arglist></arglist>
    </member>
    <member kind="variable">
      <type>int</type>
      <name>Ldep_resolve_dependences</name>
      <anchorfile>l__dependence_8c.html</anchorfile>
      <anchor>0faf9114a6f87ede074d803d6291fb63</anchor>
      <arglist></arglist>
    </member>
    <member kind="variable">
      <type>int</type>
      <name>Ldep_resolve_all_memory_dep</name>
      <anchorfile>l__dependence_8c.html</anchorfile>
      <anchor>c42d1d5d56602a5bbcfb1daa875a0565</anchor>
      <arglist></arglist>
    </member>
    <member kind="variable">
      <type>int</type>
      <name>Ldep_resolve_all_control_dep</name>
      <anchorfile>l__dependence_8c.html</anchorfile>
      <anchor>a9a8f59283f1cde323d766c934f57be5</anchor>
      <arglist></arglist>
    </member>
    <member kind="variable">
      <type>int</type>
      <name>Ldep_resolve_all_anti_output_dep</name>
      <anchorfile>l__dependence_8c.html</anchorfile>
      <anchor>5525738349a9e12f9d3c8cd1b1ec43db</anchor>
      <arglist></arglist>
    </member>
    <member kind="variable">
      <type>int</type>
      <name>Ldep_add_copy_check_dependences</name>
      <anchorfile>l__dependence_8c.html</anchorfile>
      <anchor>3b6a79265e7f6e3d9c377ba070bd16c3</anchor>
      <arglist></arglist>
    </member>
    <member kind="variable">
      <type>int</type>
      <name>Ldep_add_copy_back_dependences</name>
      <anchorfile>l__dependence_8c.html</anchorfile>
      <anchor>5a733f573abfa56183923d73e702e3fa</anchor>
      <arglist></arglist>
    </member>
    <member kind="variable">
      <type>int</type>
      <name>Ldep_allow_concurrent_issue</name>
      <anchorfile>l__dependence_8c.html</anchorfile>
      <anchor>d41a47b60e195df743517b572422beb6</anchor>
      <arglist></arglist>
    </member>
    <member kind="variable">
      <type>int</type>
      <name>Ldep_remove_jsr_dependences</name>
      <anchorfile>l__dependence_8c.html</anchorfile>
      <anchor>102282e21dc3446eb88e572592238fe7</anchor>
      <arglist></arglist>
    </member>
    <member kind="variable">
      <type>int</type>
      <name>Ldep_add_pred_hardware_arcs</name>
      <anchorfile>l__dependence_8c.html</anchorfile>
      <anchor>259b270f9c50760af7af5f0ad374fdff</anchor>
      <arglist></arglist>
    </member>
    <member kind="variable">
      <type>int</type>
      <name>Ldep_allow_upward_code_perc</name>
      <anchorfile>l__dependence_8c.html</anchorfile>
      <anchor>371d331b6fd47a4946b18d1fd777c8b6</anchor>
      <arglist></arglist>
    </member>
    <member kind="variable">
      <type>int</type>
      <name>Ldep_branch_perc_limit</name>
      <anchorfile>l__dependence_8c.html</anchorfile>
      <anchor>e9122f97b994b1b50b3bdf3ec97cf96c</anchor>
      <arglist></arglist>
    </member>
    <member kind="variable">
      <type>int</type>
      <name>Ldep_except_branch_perc_limit</name>
      <anchorfile>l__dependence_8c.html</anchorfile>
      <anchor>3cdce7c77b5936041aed79ecfe9e127c</anchor>
      <arglist></arglist>
    </member>
    <member kind="variable">
      <type>int</type>
      <name>Ldep_ignore_live_out</name>
      <anchorfile>l__dependence_8c.html</anchorfile>
      <anchor>6d934474a953e6ae08ebb56bf3e665fb</anchor>
      <arglist></arglist>
    </member>
    <member kind="variable">
      <type>int</type>
      <name>Ldep_allow_speculative_stores</name>
      <anchorfile>l__dependence_8c.html</anchorfile>
      <anchor>a2f37b09c81574ae2501ac22e267a16b</anchor>
      <arglist></arglist>
    </member>
    <member kind="variable">
      <type>int</type>
      <name>Ldep_allow_downward_code_perc</name>
      <anchorfile>l__dependence_8c.html</anchorfile>
      <anchor>a37236ab38b4dece3053ea2afdec5481</anchor>
      <arglist></arglist>
    </member>
    <member kind="variable">
      <type>int</type>
      <name>Ldep_print_dependence_graph</name>
      <anchorfile>l__dependence_8c.html</anchorfile>
      <anchor>f37eabaa89a7a4aeed7b73da96e6d2f5</anchor>
      <arglist></arglist>
    </member>
    <member kind="variable">
      <type>int</type>
      <name>Ldep_remove_always_safe</name>
      <anchorfile>l__dependence_8c.html</anchorfile>
      <anchor>2f68c5b3a4589bab5b453dabe4ae7a4a</anchor>
      <arglist></arglist>
    </member>
    <member kind="variable">
      <type>int</type>
      <name>Ldep_remove_ctl_dep_safe</name>
      <anchorfile>l__dependence_8c.html</anchorfile>
      <anchor>4bb5d2f22feac9f535966256c7e42e1e</anchor>
      <arglist></arglist>
    </member>
    <member kind="variable">
      <type>int</type>
      <name>Ldep_remove_complex_safe</name>
      <anchorfile>l__dependence_8c.html</anchorfile>
      <anchor>511f90417dd270ddcac1a3d75ca2c12b</anchor>
      <arglist></arglist>
    </member>
    <member kind="variable">
      <type>int</type>
      <name>Ldep_use_iter</name>
      <anchorfile>l__dependence_8c.html</anchorfile>
      <anchor>0fe7e26f84bc9e8c1660eb6e3216c040</anchor>
      <arglist></arglist>
    </member>
    <member kind="variable">
      <type>int</type>
      <name>Ldep_hb_keep_branch_order</name>
      <anchorfile>l__dependence_8c.html</anchorfile>
      <anchor>81b3d1e55c5b2f515229d08e9ccf386d</anchor>
      <arglist></arglist>
    </member>
    <member kind="variable">
      <type>int</type>
      <name>Ldep_program_order</name>
      <anchorfile>l__dependence_8c.html</anchorfile>
      <anchor>bb1165b8a3c310ad7eb19f9b87c359e6</anchor>
      <arglist></arglist>
    </member>
    <member kind="variable">
      <type>int</type>
      <name>Ldep_check_profiled_memory_dependences</name>
      <anchorfile>l__dependence_8c.html</anchorfile>
      <anchor>630979af3763c48fc0ead7e44dcc19f6</anchor>
      <arglist></arglist>
    </member>
    <member kind="variable">
      <type>int</type>
      <name>Ldep_allow_lat_dangles_into_jsrs</name>
      <anchorfile>l__dependence_8c.html</anchorfile>
      <anchor>d193af86d66f9928071183d9bf7f3115</anchor>
      <arglist></arglist>
    </member>
    <member kind="variable">
      <type>int</type>
      <name>Ldep_debug_memory_disambiguation</name>
      <anchorfile>l__dependence_8c.html</anchorfile>
      <anchor>6e0424048d23e9f266a7b2e9c728a3b9</anchor>
      <arglist></arglist>
    </member>
    <member kind="variable">
      <type>int</type>
      <name>Ldep_debug_concurrent_issue</name>
      <anchorfile>l__dependence_8c.html</anchorfile>
      <anchor>cb02d0de546d9171fd33c28f83a4a22e</anchor>
      <arglist></arglist>
    </member>
    <member kind="variable">
      <type>int</type>
      <name>Ldep_debug_upward_code_perc</name>
      <anchorfile>l__dependence_8c.html</anchorfile>
      <anchor>db51799de02060d9ea607b6401a8787c</anchor>
      <arglist></arglist>
    </member>
    <member kind="variable">
      <type>int</type>
      <name>Ldep_debug_downward_code_perc</name>
      <anchorfile>l__dependence_8c.html</anchorfile>
      <anchor>32acee3fd00b56e2d0aecc59d202c40c</anchor>
      <arglist></arglist>
    </member>
    <member kind="variable">
      <type>int(*)</type>
      <name>L_conflicting_operands</name>
      <anchorfile>l__dependence_8c.html</anchorfile>
      <anchor>507f61fb8d90e2f65ee21c892edfd751</anchor>
      <arglist>()</arglist>
    </member>
    <member kind="variable">
      <type>L_Alloc_Pool *</type>
      <name>L_alloc_dep</name>
      <anchorfile>l__dependence_8c.html</anchorfile>
      <anchor>3859244e82b54cb81afa9f544e4888bf</anchor>
      <arglist></arglist>
    </member>
    <member kind="variable">
      <type>L_Alloc_Pool *</type>
      <name>L_alloc_dep_info</name>
      <anchorfile>l__dependence_8c.html</anchorfile>
      <anchor>8ce30600dd67f67c1bf167af09a82fb6</anchor>
      <arglist></arglist>
    </member>
    <member kind="variable">
      <type>L_Alloc_Pool *</type>
      <name>L_alloc_dep_operand</name>
      <anchorfile>l__dependence_8c.html</anchorfile>
      <anchor>c0a99059945c7d5000cdec3cbcf66183</anchor>
      <arglist></arglist>
    </member>
    <member kind="variable" static="yes">
      <type>static Set</type>
      <name>loop_set</name>
      <anchorfile>l__dependence_8c.html</anchorfile>
      <anchor>8f93e9bc2600170bf600510cf9fc0a2f</anchor>
      <arglist></arglist>
    </member>
    <member kind="variable" static="yes">
      <type>static int</type>
      <name>Ldep_mode</name>
      <anchorfile>l__dependence_8c.html</anchorfile>
      <anchor>681b9a75fbb67d7e5cd569408eb3f3da</anchor>
      <arglist></arglist>
    </member>
    <member kind="variable" static="yes">
      <type>static Dep_Operand *</type>
      <name>Dep_Hash_Table</name>
      <anchorfile>l__dependence_8c.html</anchorfile>
      <anchor>f34f2e696e6f8e641a630ae0dcd4af30</anchor>
      <arglist>[DEP_HASH_TABLE_SIZE]</arglist>
    </member>
  </compound>
  <compound kind="file">
    <name>l_dependence.h</name>
    <path>/mnt/hgfs/mkt/Desktop/openimpact/src/Lcode/codegen/Lschedule/</path>
    <filename>l__dependence_8h</filename>
    <class kind="struct">Dep_Operand</class>
    <class kind="struct">L_Dep</class>
    <class kind="struct">Dep_Info</class>
    <member kind="define">
      <type>#define</type>
      <name>LDEP_MODE_ACYCLIC</name>
      <anchorfile>l__dependence_8h.html</anchorfile>
      <anchor>ca761b0b451255076f49e439051ff47d</anchor>
      <arglist></arglist>
    </member>
    <member kind="define">
      <type>#define</type>
      <name>LDEP_MODE_CYCLIC</name>
      <anchorfile>l__dependence_8h.html</anchorfile>
      <anchor>e417dbe2c9f367eaf46cf098e476c6c3</anchor>
      <arglist></arglist>
    </member>
    <member kind="define">
      <type>#define</type>
      <name>LDEP_PREPASS</name>
      <anchorfile>l__dependence_8h.html</anchorfile>
      <anchor>daf2531836c0a433be76cc3530568278</anchor>
      <arglist></arglist>
    </member>
    <member kind="define">
      <type>#define</type>
      <name>LDEP_POSTPASS</name>
      <anchorfile>l__dependence_8h.html</anchorfile>
      <anchor>27474e7ed5ec0830f2bbd60326758c68</anchor>
      <arglist></arglist>
    </member>
    <member kind="define">
      <type>#define</type>
      <name>L_DEP_ANY</name>
      <anchorfile>l__dependence_8h.html</anchorfile>
      <anchor>3744ed9cb461fa1557d895e4b84fcbbe</anchor>
      <arglist></arglist>
    </member>
    <member kind="define">
      <type>#define</type>
      <name>L_DEP_NONE</name>
      <anchorfile>l__dependence_8h.html</anchorfile>
      <anchor>a86183580d9b521d02223327976d7a40</anchor>
      <arglist></arglist>
    </member>
    <member kind="define">
      <type>#define</type>
      <name>L_DEP_REG_FLOW</name>
      <anchorfile>l__dependence_8h.html</anchorfile>
      <anchor>7e5a0511f265decccf9c5ec21bdea4b8</anchor>
      <arglist></arglist>
    </member>
    <member kind="define">
      <type>#define</type>
      <name>L_DEP_REG_ANTI</name>
      <anchorfile>l__dependence_8h.html</anchorfile>
      <anchor>7d009edbac1dc5b067b5f5eedeb0b423</anchor>
      <arglist></arglist>
    </member>
    <member kind="define">
      <type>#define</type>
      <name>L_DEP_REG_OUTPUT</name>
      <anchorfile>l__dependence_8h.html</anchorfile>
      <anchor>d0f48f8141b45ad238c6907fdd065592</anchor>
      <arglist></arglist>
    </member>
    <member kind="define">
      <type>#define</type>
      <name>L_DEP_MEM_FLOW</name>
      <anchorfile>l__dependence_8h.html</anchorfile>
      <anchor>cfdf914f0b4330290fb5e493b28219f7</anchor>
      <arglist></arglist>
    </member>
    <member kind="define">
      <type>#define</type>
      <name>L_DEP_MEM_ANTI</name>
      <anchorfile>l__dependence_8h.html</anchorfile>
      <anchor>701044c9da61b4177e3b1059458290f7</anchor>
      <arglist></arglist>
    </member>
    <member kind="define">
      <type>#define</type>
      <name>L_DEP_MEM_OUTPUT</name>
      <anchorfile>l__dependence_8h.html</anchorfile>
      <anchor>eb463aeba71453ced885e88d79fb70f5</anchor>
      <arglist></arglist>
    </member>
    <member kind="define">
      <type>#define</type>
      <name>L_DEP_CNT</name>
      <anchorfile>l__dependence_8h.html</anchorfile>
      <anchor>97a696524ddcfcf52e99447d6ab90987</anchor>
      <arglist></arglist>
    </member>
    <member kind="define">
      <type>#define</type>
      <name>L_DEP_SYNC</name>
      <anchorfile>l__dependence_8h.html</anchorfile>
      <anchor>8a2cca461224abece7afb57e5ff2befe</anchor>
      <arglist></arglist>
    </member>
    <member kind="define">
      <type>#define</type>
      <name>L_DEP_VLIW</name>
      <anchorfile>l__dependence_8h.html</anchorfile>
      <anchor>44966ab55c56638a952468d098a9e173</anchor>
      <arglist></arglist>
    </member>
    <member kind="define">
      <type>#define</type>
      <name>L_DEP_IGNORE_MEM_FLOW_DEP</name>
      <anchorfile>l__dependence_8h.html</anchorfile>
      <anchor>3523f735698d632a27db5c1feb019db5</anchor>
      <arglist></arglist>
    </member>
    <member kind="define">
      <type>#define</type>
      <name>L_DEP_IGNORE_MEM_OUTPUT_DEP</name>
      <anchorfile>l__dependence_8h.html</anchorfile>
      <anchor>ab9cb924c968d91b2c8e962d66fd84da</anchor>
      <arglist></arglist>
    </member>
    <member kind="define">
      <type>#define</type>
      <name>L_DEP_IGNORE_MEM_ANTI_DEP</name>
      <anchorfile>l__dependence_8h.html</anchorfile>
      <anchor>75b96298c5074efacde7e6e267dbce98</anchor>
      <arglist></arglist>
    </member>
    <member kind="define">
      <type>#define</type>
      <name>L_DEP_IGNORE_ALL_MEMORY_DEP</name>
      <anchorfile>l__dependence_8h.html</anchorfile>
      <anchor>59849a68b770fbb0c4cc1b8a2161a34d</anchor>
      <arglist></arglist>
    </member>
    <member kind="define">
      <type>#define</type>
      <name>L_DEP_IGNORE_REG_FLOW_DEP</name>
      <anchorfile>l__dependence_8h.html</anchorfile>
      <anchor>08a5cbca5ee59c9426575953aac7337e</anchor>
      <arglist></arglist>
    </member>
    <member kind="define">
      <type>#define</type>
      <name>L_DEP_IGNORE_REG_OUTPUT_DEP</name>
      <anchorfile>l__dependence_8h.html</anchorfile>
      <anchor>80680af4d1495deeab44876910107d89</anchor>
      <arglist></arglist>
    </member>
    <member kind="define">
      <type>#define</type>
      <name>L_DEP_IGNORE_REG_ANTI_DEP</name>
      <anchorfile>l__dependence_8h.html</anchorfile>
      <anchor>5921c176ef91590c8db893d4a23ca8fb</anchor>
      <arglist></arglist>
    </member>
    <member kind="define">
      <type>#define</type>
      <name>L_DEP_IGNORE_ALL_REG_DEP</name>
      <anchorfile>l__dependence_8h.html</anchorfile>
      <anchor>c320f7b14c2e5027a2778e774364019f</anchor>
      <arglist></arglist>
    </member>
    <member kind="define">
      <type>#define</type>
      <name>L_DEP_IGNORE_CNT_DEP</name>
      <anchorfile>l__dependence_8h.html</anchorfile>
      <anchor>7e9a61e2d3433205a5eb4b2ef69fc564</anchor>
      <arglist></arglist>
    </member>
    <member kind="define">
      <type>#define</type>
      <name>DEP_HASH_TABLE_SIZE</name>
      <anchorfile>l__dependence_8h.html</anchorfile>
      <anchor>d753d4c2f7a70931c90e6b133a09cd40</anchor>
      <arglist></arglist>
    </member>
    <member kind="define">
      <type>#define</type>
      <name>DEP_HASH_MASK</name>
      <anchorfile>l__dependence_8h.html</anchorfile>
      <anchor>2b4b2ea3fee3c4efcba5e5d876784f3f</anchor>
      <arglist></arglist>
    </member>
    <member kind="define">
      <type>#define</type>
      <name>DEP_MEM_OPERAND</name>
      <anchorfile>l__dependence_8h.html</anchorfile>
      <anchor>a1a43eadd7bb31302abdeb28f606497b</anchor>
      <arglist></arglist>
    </member>
    <member kind="define">
      <type>#define</type>
      <name>DEP_CNT_OPERAND</name>
      <anchorfile>l__dependence_8h.html</anchorfile>
      <anchor>ed4f1e9cee538335bc3a17a0c73241c3</anchor>
      <arglist></arglist>
    </member>
    <member kind="define">
      <type>#define</type>
      <name>DEP_SYNC_OPERAND</name>
      <anchorfile>l__dependence_8h.html</anchorfile>
      <anchor>14c0cbe25fab7704fb5d4b491c096dab</anchor>
      <arglist></arglist>
    </member>
    <member kind="define">
      <type>#define</type>
      <name>DEP_VLIW_OPERAND</name>
      <anchorfile>l__dependence_8h.html</anchorfile>
      <anchor>3326c6a5979f6eb3961e3d2142cd9b7f</anchor>
      <arglist></arglist>
    </member>
    <member kind="define">
      <type>#define</type>
      <name>DEP_CANT_SPECULATE</name>
      <anchorfile>l__dependence_8h.html</anchorfile>
      <anchor>902c066b60539a643ac88ce96d3a4dad</anchor>
      <arglist></arglist>
    </member>
    <member kind="define">
      <type>#define</type>
      <name>DEP_NON_EXCEPTING</name>
      <anchorfile>l__dependence_8h.html</anchorfile>
      <anchor>17bacd910de7ce678a5feb98a92904cb</anchor>
      <arglist></arglist>
    </member>
    <member kind="define">
      <type>#define</type>
      <name>DEP_ALWAYS_SAFE</name>
      <anchorfile>l__dependence_8h.html</anchorfile>
      <anchor>09c92343f8bd5f2604389a19fde17cf9</anchor>
      <arglist></arglist>
    </member>
    <member kind="define">
      <type>#define</type>
      <name>DEP_CTL_DEP_SAFE</name>
      <anchorfile>l__dependence_8h.html</anchorfile>
      <anchor>f5c26632fc5f44d1d985a80f495c5324</anchor>
      <arglist></arglist>
    </member>
    <member kind="define">
      <type>#define</type>
      <name>DEP_COMPLEX_SAFE</name>
      <anchorfile>l__dependence_8h.html</anchorfile>
      <anchor>e02d4aa9d473f1531a9bf542f96cfdd3</anchor>
      <arglist></arglist>
    </member>
    <member kind="define">
      <type>#define</type>
      <name>DEP_SILENT</name>
      <anchorfile>l__dependence_8h.html</anchorfile>
      <anchor>88c64501bb0da9a0a6ebccdd444ed621</anchor>
      <arglist></arglist>
    </member>
    <member kind="define">
      <type>#define</type>
      <name>DEP_DELAYS_EXCEPTION</name>
      <anchorfile>l__dependence_8h.html</anchorfile>
      <anchor>f54c8a0b866148e1cef586112452ed13</anchor>
      <arglist></arglist>
    </member>
    <member kind="define">
      <type>#define</type>
      <name>DEP_SAFE_STORE</name>
      <anchorfile>l__dependence_8h.html</anchorfile>
      <anchor>2f9ca3b96f1a67be58b18f78caea81b1</anchor>
      <arglist></arglist>
    </member>
    <member kind="define">
      <type>#define</type>
      <name>DEP_INFO</name>
      <anchorfile>l__dependence_8h.html</anchorfile>
      <anchor>0fa8d67fefe7bbeb158c334702bdf1a9</anchor>
      <arglist>(oper)</arglist>
    </member>
    <member kind="function">
      <type>void</type>
      <name>Ldep_mark_safe_invariant</name>
      <anchorfile>l__dependence_8h.html</anchorfile>
      <anchor>1f244cfa861039836212d125b1a5e399</anchor>
      <arglist>(L_Func *fn)</arglist>
    </member>
    <member kind="function">
      <type>void</type>
      <name>Ldep_mark_safe</name>
      <anchorfile>l__dependence_8h.html</anchorfile>
      <anchor>eb5239833bb75d328e2e84dd49f420f0</anchor>
      <arglist>(L_Func *fn)</arglist>
    </member>
    <member kind="function">
      <type>void</type>
      <name>Ldep_init</name>
      <anchorfile>l__dependence_8h.html</anchorfile>
      <anchor>d2a4936e04bc8c14dd34d18d914e9980</anchor>
      <arglist>(Parm_Macro_List *)</arglist>
    </member>
    <member kind="function">
      <type>void</type>
      <name>L_build_dependence_graph</name>
      <anchorfile>l__dependence_8h.html</anchorfile>
      <anchor>0bbba3ce0e64cc507c81ba677b2b6b03</anchor>
      <arglist>(L_Cb *, int, int)</arglist>
    </member>
    <member kind="function">
      <type>void</type>
      <name>L_delete_dependence_graph</name>
      <anchorfile>l__dependence_8h.html</anchorfile>
      <anchor>c5adb5f0283fd2a872096d4fe80b8048</anchor>
      <arglist>(L_Cb *)</arglist>
    </member>
    <member kind="function">
      <type>void</type>
      <name>L_compute_dependence_level</name>
      <anchorfile>l__dependence_8h.html</anchorfile>
      <anchor>cfe711d4aba7e4c8980897955204029f</anchor>
      <arglist>(L_Cb *)</arglist>
    </member>
    <member kind="function">
      <type>void</type>
      <name>L_delete_dependence_info</name>
      <anchorfile>l__dependence_8h.html</anchorfile>
      <anchor>42fab7fffd27cd1352a69e066dda806c</anchor>
      <arglist>(Dep_Info *)</arglist>
    </member>
    <member kind="function">
      <type>void</type>
      <name>L_add_dep</name>
      <anchorfile>l__dependence_8h.html</anchorfile>
      <anchor>1a7647b8ab91e39718b42ca94a936a33</anchor>
      <arglist>(int, int, int, int, L_Oper *, L_Oper *, int)</arglist>
    </member>
    <member kind="function">
      <type>void</type>
      <name>L_remove_dep_pair</name>
      <anchorfile>l__dependence_8h.html</anchorfile>
      <anchor>56c4fb84c90b08674cad178142a61973</anchor>
      <arglist>(int, L_Oper *, L_Oper *)</arglist>
    </member>
    <member kind="function">
      <type>L_Dep *</type>
      <name>L_remove_dep</name>
      <anchorfile>l__dependence_8h.html</anchorfile>
      <anchor>762d94e307b6c25aa78fef8b11cea19a</anchor>
      <arglist>(L_Dep *, int *, int, L_Oper *, L_Oper *)</arglist>
    </member>
    <member kind="function">
      <type>L_Dep *</type>
      <name>L_find_dep</name>
      <anchorfile>l__dependence_8h.html</anchorfile>
      <anchor>d7f6a5b7e3b7e65cbb74b1ef97f2df49</anchor>
      <arglist>(L_Dep *, L_Oper *, L_Oper *, int)</arglist>
    </member>
    <member kind="function">
      <type>Dep_Info *</type>
      <name>L_new_dep_info</name>
      <anchorfile>l__dependence_8h.html</anchorfile>
      <anchor>36f50b716086eefdc8b0f60e72e98f71</anchor>
      <arglist>()</arglist>
    </member>
    <member kind="variable">
      <type>int</type>
      <name>Ldep_branch_perc_limit</name>
      <anchorfile>l__dependence_8h.html</anchorfile>
      <anchor>e9122f97b994b1b50b3bdf3ec97cf96c</anchor>
      <arglist></arglist>
    </member>
    <member kind="variable">
      <type>int</type>
      <name>Ldep_alloc_target_size_dep</name>
      <anchorfile>l__dependence_8h.html</anchorfile>
      <anchor>052af7babdd1a01507ab274586f974f5</anchor>
      <arglist></arglist>
    </member>
    <member kind="variable">
      <type>int</type>
      <name>Ldep_alloc_target_size_dep_info</name>
      <anchorfile>l__dependence_8h.html</anchorfile>
      <anchor>24855d7f12c68eddde6562e91651ae8d</anchor>
      <arglist></arglist>
    </member>
    <member kind="variable">
      <type>int</type>
      <name>Ldep_alloc_target_size_dep_operand</name>
      <anchorfile>l__dependence_8h.html</anchorfile>
      <anchor>b58047a9332cc2a01da6a85464e2c451</anchor>
      <arglist></arglist>
    </member>
    <member kind="variable">
      <type>int</type>
      <name>Ldep_branch_perc_limit</name>
      <anchorfile>l__dependence_8h.html</anchorfile>
      <anchor>e9122f97b994b1b50b3bdf3ec97cf96c</anchor>
      <arglist></arglist>
    </member>
    <member kind="variable">
      <type>int</type>
      <name>Ldep_except_branch_perc_limit</name>
      <anchorfile>l__dependence_8h.html</anchorfile>
      <anchor>3cdce7c77b5936041aed79ecfe9e127c</anchor>
      <arglist></arglist>
    </member>
    <member kind="variable">
      <type>int</type>
      <name>Ldep_allow_speculative_stores</name>
      <anchorfile>l__dependence_8h.html</anchorfile>
      <anchor>a2f37b09c81574ae2501ac22e267a16b</anchor>
      <arglist></arglist>
    </member>
    <member kind="variable">
      <type>int</type>
      <name>Ldep_allow_upward_code_perc</name>
      <anchorfile>l__dependence_8h.html</anchorfile>
      <anchor>371d331b6fd47a4946b18d1fd777c8b6</anchor>
      <arglist></arglist>
    </member>
    <member kind="variable">
      <type>int</type>
      <name>Ldep_allow_downward_code_perc</name>
      <anchorfile>l__dependence_8h.html</anchorfile>
      <anchor>a37236ab38b4dece3053ea2afdec5481</anchor>
      <arglist></arglist>
    </member>
    <member kind="variable">
      <type>int</type>
      <name>Ldep_remove_always_safe</name>
      <anchorfile>l__dependence_8h.html</anchorfile>
      <anchor>2f68c5b3a4589bab5b453dabe4ae7a4a</anchor>
      <arglist></arglist>
    </member>
    <member kind="variable">
      <type>int</type>
      <name>Ldep_remove_ctl_dep_safe</name>
      <anchorfile>l__dependence_8h.html</anchorfile>
      <anchor>4bb5d2f22feac9f535966256c7e42e1e</anchor>
      <arglist></arglist>
    </member>
    <member kind="variable">
      <type>int</type>
      <name>Ldep_remove_complex_safe</name>
      <anchorfile>l__dependence_8h.html</anchorfile>
      <anchor>511f90417dd270ddcac1a3d75ca2c12b</anchor>
      <arglist></arglist>
    </member>
    <member kind="variable">
      <type>int</type>
      <name>Ldep_hb_keep_branch_order</name>
      <anchorfile>l__dependence_8h.html</anchorfile>
      <anchor>81b3d1e55c5b2f515229d08e9ccf386d</anchor>
      <arglist></arglist>
    </member>
    <member kind="variable">
      <type>int</type>
      <name>Ldep_check_profiled_memory_dependences</name>
      <anchorfile>l__dependence_8h.html</anchorfile>
      <anchor>630979af3763c48fc0ead7e44dcc19f6</anchor>
      <arglist></arglist>
    </member>
    <member kind="variable">
      <type>int</type>
      <name>Ldep_allow_lat_dangles_into_jsrs</name>
      <anchorfile>l__dependence_8h.html</anchorfile>
      <anchor>d193af86d66f9928071183d9bf7f3115</anchor>
      <arglist></arglist>
    </member>
  </compound>
  <compound kind="file">
    <name>l_mcb.c</name>
    <path>/mnt/hgfs/mkt/Desktop/openimpact/src/Lcode/codegen/Lschedule/</path>
    <filename>l__mcb_8c</filename>
    <includes id="l__mcb_8h" name="l_mcb.h" local="yes" imported="no">l_mcb.h</includes>
    <member kind="define">
      <type>#define</type>
      <name>L_MAX_COPY_OPER</name>
      <anchorfile>l__mcb_8c.html</anchorfile>
      <anchor>f0a77018e61cc8d0607e3b5fef379a5a</anchor>
      <arglist></arglist>
    </member>
    <member kind="define">
      <type>#define</type>
      <name>L_MAX_CB_LOADS</name>
      <anchorfile>l__mcb_8c.html</anchorfile>
      <anchor>43574b2d835a9e874738e64e23e4a348</anchor>
      <arglist></arglist>
    </member>
    <member kind="define">
      <type>#define</type>
      <name>L_MAX_CB_STORES</name>
      <anchorfile>l__mcb_8c.html</anchorfile>
      <anchor>d3eeddc721ff56909542221baf589402</anchor>
      <arglist></arglist>
    </member>
    <member kind="define">
      <type>#define</type>
      <name>MIN_MCB_SPEEDUP</name>
      <anchorfile>l__mcb_8c.html</anchorfile>
      <anchor>2afef09fe0afc9994a8e4736a9413a56</anchor>
      <arglist></arglist>
    </member>
    <member kind="define">
      <type>#define</type>
      <name>MAX_STORES_BYPASSED</name>
      <anchorfile>l__mcb_8c.html</anchorfile>
      <anchor>5262663f922a4f37193962ebd54db357</anchor>
      <arglist></arglist>
    </member>
    <member kind="define">
      <type>#define</type>
      <name>MCB_MIN_CB_WEIGHT</name>
      <anchorfile>l__mcb_8c.html</anchorfile>
      <anchor>d51acee83fd019040304d18c7ffd63fa</anchor>
      <arglist></arglist>
    </member>
    <member kind="function">
      <type>L_Oper *</type>
      <name>L_find_next_branch</name>
      <anchorfile>l__mcb_8c.html</anchorfile>
      <anchor>6d691d5261d79228e4e680680a58f405</anchor>
      <arglist>(L_Cb *cb, L_Oper *oper)</arglist>
    </member>
    <member kind="function" static="yes">
      <type>static L_Oper *</type>
      <name>L_get_regs_load_def</name>
      <anchorfile>l__mcb_8c.html</anchorfile>
      <anchor>e5566e98d51ae8efeae62c1587353c0d</anchor>
      <arglist>(L_Oper *oper, L_Operand *reg)</arglist>
    </member>
    <member kind="function" static="yes">
      <type>static int</type>
      <name>L_are_same_pointer_regs</name>
      <anchorfile>l__mcb_8c.html</anchorfile>
      <anchor>ac402c12aca7290c76d4eec6758f5480</anchor>
      <arglist>(L_Oper *op1, L_Operand *reg1, L_Oper *op2, L_Operand *reg2)</arglist>
    </member>
    <member kind="function" static="yes">
      <type>static int</type>
      <name>L_check_equivalent_regs</name>
      <anchorfile>l__mcb_8c.html</anchorfile>
      <anchor>7298a80fd887066c0a903aca94cd52d9</anchor>
      <arglist>(L_Oper *op1, L_Operand *reg1, L_Oper *op2, L_Operand *reg2)</arglist>
    </member>
    <member kind="function" static="yes">
      <type>static void</type>
      <name>L_check_flow_op</name>
      <anchorfile>l__mcb_8c.html</anchorfile>
      <anchor>4320cbf6282b78d2ac158ce94fd10227</anchor>
      <arglist>(int dest_reg, L_Oper *begin_op, L_Oper *end_op)</arglist>
    </member>
    <member kind="function" static="yes">
      <type>static int</type>
      <name>L_get_reg_num</name>
      <anchorfile>l__mcb_8c.html</anchorfile>
      <anchor>f533fffdb1bb0ae17afefa3264600bb9</anchor>
      <arglist>(L_Operand *operand)</arglist>
    </member>
    <member kind="function" static="yes">
      <type>static void</type>
      <name>L_rename_src_reg</name>
      <anchorfile>l__mcb_8c.html</anchorfile>
      <anchor>50bea1902b546b8d39ad9ce49e97db0a</anchor>
      <arglist>(L_Cb *cb, int operand_num, L_Oper *oper, L_Oper *end_op)</arglist>
    </member>
    <member kind="function" static="yes">
      <type>static void</type>
      <name>L_change_ext_operand</name>
      <anchorfile>l__mcb_8c.html</anchorfile>
      <anchor>b1d13da98f43d492e0a01955f1906d58</anchor>
      <arglist>(L_Oper *oper, int operand_num, L_Operand *new_operand)</arglist>
    </member>
    <member kind="function" static="yes">
      <type>static void</type>
      <name>L_mcb_add_store_to_load_list</name>
      <anchorfile>l__mcb_8c.html</anchorfile>
      <anchor>1ca14cb238c2d86325a87cdf70d088ec</anchor>
      <arglist>(L_Oper *store, L_Oper *load)</arglist>
    </member>
    <member kind="function" static="yes">
      <type>static void</type>
      <name>L_add_check_ptr_to_oper</name>
      <anchorfile>l__mcb_8c.html</anchorfile>
      <anchor>45fa3b90dfc4e7f439ac3da0d95588d1</anchor>
      <arglist>(L_Oper *check, L_Oper *load)</arglist>
    </member>
    <member kind="function" static="yes">
      <type>static int</type>
      <name>L_is_mem_or_flow_dep</name>
      <anchorfile>l__mcb_8c.html</anchorfile>
      <anchor>0f8879fc8c3ff1ca855016ae4251a8d4</anchor>
      <arglist>(L_Dep *depend)</arglist>
    </member>
    <member kind="function" static="yes">
      <type>static L_Oper *</type>
      <name>L_mcb_insert_check</name>
      <anchorfile>l__mcb_8c.html</anchorfile>
      <anchor>f411e7eb7027b50b7b7b6a3400d6b69f</anchor>
      <arglist>(L_Oper *oper)</arglist>
    </member>
    <member kind="function" static="yes">
      <type>static void</type>
      <name>L_add_check_to_check_dep</name>
      <anchorfile>l__mcb_8c.html</anchorfile>
      <anchor>0737f2797f1808792164e73445bcd227</anchor>
      <arglist>(L_Oper *prev_check, L_Oper *check)</arglist>
    </member>
    <member kind="function" static="yes">
      <type>static int</type>
      <name>L_is_corr_cb</name>
      <anchorfile>l__mcb_8c.html</anchorfile>
      <anchor>f0220a0f5980572aa3150ec3bb07617d</anchor>
      <arglist>(L_Cb *cb)</arglist>
    </member>
    <member kind="function" static="yes">
      <type>static void</type>
      <name>L_insert_correction_code</name>
      <anchorfile>l__mcb_8c.html</anchorfile>
      <anchor>9f582a6c0544dd2f38ec762ac7130a99</anchor>
      <arglist>(L_Cb *cb)</arglist>
    </member>
    <member kind="function" static="yes">
      <type>static void</type>
      <name>L_insert_fall_thru_code</name>
      <anchorfile>l__mcb_8c.html</anchorfile>
      <anchor>9343a80881ff36a7d4172fc3781befb3</anchor>
      <arglist>(L_Cb *cb)</arglist>
    </member>
    <member kind="function" static="yes">
      <type>static void</type>
      <name>L_free_sched_info</name>
      <anchorfile>l__mcb_8c.html</anchorfile>
      <anchor>28e578244421a4ccae1bb527ae6aa4d1</anchor>
      <arglist>(L_Cb *cb)</arglist>
    </member>
    <member kind="function" static="yes">
      <type>static void</type>
      <name>L_mcb_delete_check</name>
      <anchorfile>l__mcb_8c.html</anchorfile>
      <anchor>707fbd6c45fa76a337bac75eda82282e</anchor>
      <arglist>(L_Cb *cb, L_Oper *check, int current_time)</arglist>
    </member>
    <member kind="function">
      <type>void</type>
      <name>L_change_beq_mcb</name>
      <anchorfile>l__mcb_8c.html</anchorfile>
      <anchor>a9aa874499a28b5d29c17cdd1084e064</anchor>
      <arglist>(L_Cb *cb)</arglist>
    </member>
    <member kind="function">
      <type>int</type>
      <name>L_dependent_memory_ops</name>
      <anchorfile>l__mcb_8c.html</anchorfile>
      <anchor>87c9795d2b98de45ce888950080a3f9f</anchor>
      <arglist>(L_Cb *cb, L_Oper *op1, L_Oper *op2)</arglist>
    </member>
    <member kind="function">
      <type>L_Cb *</type>
      <name>L_copy_all_oper_in_cb</name>
      <anchorfile>l__mcb_8c.html</anchorfile>
      <anchor>d7ea649eb15d34bb940fea7fc01f55cf</anchor>
      <arglist>(L_Func *fn, L_Cb *cb)</arglist>
    </member>
    <member kind="function">
      <type>void</type>
      <name>L_remove_sched_cb</name>
      <anchorfile>l__mcb_8c.html</anchorfile>
      <anchor>ec6ec3dd6c6b7e836560d4ae0ef698b1</anchor>
      <arglist>(L_Func *fn, L_Cb *cb)</arglist>
    </member>
    <member kind="function">
      <type>void</type>
      <name>L_mcb_init</name>
      <anchorfile>l__mcb_8c.html</anchorfile>
      <anchor>072c41f7e97e34203efbc40f1714b2a7</anchor>
      <arglist>()</arglist>
    </member>
    <member kind="function" static="yes">
      <type>static void</type>
      <name>L_free_oper_list_cb</name>
      <anchorfile>l__mcb_8c.html</anchorfile>
      <anchor>0e603faf6b2763b633839759cf999737</anchor>
      <arglist>(L_Cb *cb)</arglist>
    </member>
    <member kind="function">
      <type>void</type>
      <name>L_mcb_insert_check_and_rem_dependences</name>
      <anchorfile>l__mcb_8c.html</anchorfile>
      <anchor>b74980959bae37f721bec075fe9171ba</anchor>
      <arglist>(L_Cb *cb)</arglist>
    </member>
    <member kind="function" static="yes">
      <type>static void</type>
      <name>L_mcb_convert_sched_info</name>
      <anchorfile>l__mcb_8c.html</anchorfile>
      <anchor>075622f23f1acb22f6a1c3db0a589a8b</anchor>
      <arglist>(L_Oper *new_oper, L_Oper *old_oper)</arglist>
    </member>
    <member kind="function" static="yes">
      <type>static void</type>
      <name>L_mcb_convert_to_parent</name>
      <anchorfile>l__mcb_8c.html</anchorfile>
      <anchor>0f7dc4af57eefd0c374df42f684ca73b</anchor>
      <arglist>(L_Cb *cb)</arglist>
    </member>
    <member kind="function">
      <type>void</type>
      <name>Lsched_mcb_schedule_block</name>
      <anchorfile>l__mcb_8c.html</anchorfile>
      <anchor>87c4a42bf87228adc317fe08e2aa57d1</anchor>
      <arglist>(L_Func *fn, L_Cb *cb, L_Cb *nomcb_cb)</arglist>
    </member>
    <member kind="function">
      <type>void</type>
      <name>L_mcb_remove_check</name>
      <anchorfile>l__mcb_8c.html</anchorfile>
      <anchor>86fbe77057384630ecf6336d91ed3fc5</anchor>
      <arglist>(L_Cb *cb, L_Oper *oper, int ready_time)</arglist>
    </member>
    <member kind="variable">
      <type>L_Alloc_Pool *</type>
      <name>Lmcb_list_pool</name>
      <anchorfile>l__mcb_8c.html</anchorfile>
      <anchor>f14f50cac6624f247009281f9fa796ab</anchor>
      <arglist></arglist>
    </member>
    <member kind="variable">
      <type>int</type>
      <name>Lsched_num_opers</name>
      <anchorfile>l__mcb_8c.html</anchorfile>
      <anchor>81a1b68006a09b691fdbe6e8d1fb354d</anchor>
      <arglist></arglist>
    </member>
    <member kind="variable">
      <type>int</type>
      <name>Lmcb_keep_checks</name>
      <anchorfile>l__mcb_8c.html</anchorfile>
      <anchor>7fa8b9ab156b1588456d5055d2d2633f</anchor>
      <arglist></arglist>
    </member>
  </compound>
  <compound kind="file">
    <name>l_mcb.h</name>
    <path>/mnt/hgfs/mkt/Desktop/openimpact/src/Lcode/codegen/Lschedule/</path>
    <filename>l__mcb_8h</filename>
    <includes id="l__schedule_8h" name="l_schedule.h" local="yes" imported="no">l_schedule.h</includes>
    <member kind="function">
      <type>void</type>
      <name>L_change_beq_mcb</name>
      <anchorfile>l__mcb_8h.html</anchorfile>
      <anchor>a9aa874499a28b5d29c17cdd1084e064</anchor>
      <arglist>(L_Cb *cb)</arglist>
    </member>
    <member kind="function">
      <type>L_Cb *</type>
      <name>L_copy_all_oper_in_cb</name>
      <anchorfile>l__mcb_8h.html</anchorfile>
      <anchor>d7ea649eb15d34bb940fea7fc01f55cf</anchor>
      <arglist>(L_Func *fn, L_Cb *cb)</arglist>
    </member>
    <member kind="function">
      <type>void</type>
      <name>L_mcb_insert_check_and_rem_dependences</name>
      <anchorfile>l__mcb_8h.html</anchorfile>
      <anchor>b74980959bae37f721bec075fe9171ba</anchor>
      <arglist>(L_Cb *cb)</arglist>
    </member>
    <member kind="function">
      <type>int</type>
      <name>L_dependent_memory_ops</name>
      <anchorfile>l__mcb_8h.html</anchorfile>
      <anchor>87c9795d2b98de45ce888950080a3f9f</anchor>
      <arglist>(L_Cb *cb, L_Oper *op1, L_Oper *op2)</arglist>
    </member>
    <member kind="function">
      <type>void</type>
      <name>L_remove_sched_cb</name>
      <anchorfile>l__mcb_8h.html</anchorfile>
      <anchor>ec6ec3dd6c6b7e836560d4ae0ef698b1</anchor>
      <arglist>(L_Func *fn, L_Cb *cb)</arglist>
    </member>
    <member kind="function">
      <type>void</type>
      <name>L_mcb_init</name>
      <anchorfile>l__mcb_8h.html</anchorfile>
      <anchor>072c41f7e97e34203efbc40f1714b2a7</anchor>
      <arglist>()</arglist>
    </member>
    <member kind="function">
      <type>void</type>
      <name>L_free_oper_list</name>
      <anchorfile>l__mcb_8h.html</anchorfile>
      <anchor>9d53ae7d4aacc13a3cc3b63e4f4e4a2d</anchor>
      <arglist>(L_Oper *oper)</arglist>
    </member>
    <member kind="function">
      <type>void</type>
      <name>L_mcb_remove_check</name>
      <anchorfile>l__mcb_8h.html</anchorfile>
      <anchor>86fbe77057384630ecf6336d91ed3fc5</anchor>
      <arglist>(L_Cb *cb, L_Oper *oper, int ready_time)</arglist>
    </member>
    <member kind="function">
      <type>L_Oper_List *</type>
      <name>L_new_oper_list</name>
      <anchorfile>l__mcb_8h.html</anchorfile>
      <anchor>8449c9f6ad25a217fd5bc0cffc96eeb2</anchor>
      <arglist>()</arglist>
    </member>
  </compound>
  <compound kind="file">
    <name>l_queue.c</name>
    <path>/mnt/hgfs/mkt/Desktop/openimpact/src/Lcode/codegen/Lschedule/</path>
    <filename>l__queue_8c</filename>
    <includes id="l__schedule_8h" name="l_schedule.h" local="yes" imported="no">l_schedule.h</includes>
    <member kind="function">
      <type>Squeue *</type>
      <name>L_create_queue</name>
      <anchorfile>l__queue_8c.html</anchorfile>
      <anchor>43403c4293b8d84da6b0cd6944543310</anchor>
      <arglist>(char *name, int id)</arglist>
    </member>
    <member kind="function">
      <type>void</type>
      <name>L_delete_queue</name>
      <anchorfile>l__queue_8c.html</anchorfile>
      <anchor>bbbeef78440f0b1f5dcef84469fda654</anchor>
      <arglist>(Squeue *queue)</arglist>
    </member>
    <member kind="function">
      <type>void</type>
      <name>L_reset_queue_current</name>
      <anchorfile>l__queue_8c.html</anchorfile>
      <anchor>b8eee0f8115f1643c7baa5b3bf5f18eb</anchor>
      <arglist>(Squeue *queue)</arglist>
    </member>
    <member kind="function">
      <type>Sched_Info *</type>
      <name>L_get_queue_next_entry</name>
      <anchorfile>l__queue_8c.html</anchorfile>
      <anchor>40d01bb9c459ecfbff2e17481751f642</anchor>
      <arglist>(Squeue *queue)</arglist>
    </member>
    <member kind="function">
      <type>Sched_Info *</type>
      <name>L_get_queue_head</name>
      <anchorfile>l__queue_8c.html</anchorfile>
      <anchor>c6639a02ef45dc36a183e8e63332a728</anchor>
      <arglist>(Squeue *queue)</arglist>
    </member>
    <member kind="function">
      <type>int</type>
      <name>L_get_queue_size</name>
      <anchorfile>l__queue_8c.html</anchorfile>
      <anchor>8327b592c57e3869fd7188e366761320</anchor>
      <arglist>(Squeue *queue)</arglist>
    </member>
    <member kind="function">
      <type>void</type>
      <name>L_enqueue</name>
      <anchorfile>l__queue_8c.html</anchorfile>
      <anchor>69f52ba011a108dd2780c616a31a87be</anchor>
      <arglist>(Squeue *queue, Sched_Info *sinfo)</arglist>
    </member>
    <member kind="function">
      <type>void</type>
      <name>L_dequeue</name>
      <anchorfile>l__queue_8c.html</anchorfile>
      <anchor>ab29b10359f2002adc92857394cbb1ff</anchor>
      <arglist>(Squeue *queue, Sched_Info *sinfo)</arglist>
    </member>
    <member kind="function">
      <type>void</type>
      <name>L_dequeue_entry</name>
      <anchorfile>l__queue_8c.html</anchorfile>
      <anchor>712aee973c1fce1d5dbb20ed2b28c0da</anchor>
      <arglist>(Sq_entry *entry)</arglist>
    </member>
    <member kind="function">
      <type>void</type>
      <name>L_dequeue_from_all</name>
      <anchorfile>l__queue_8c.html</anchorfile>
      <anchor>e7af13146ff4d982c5f88e141abe785f</anchor>
      <arglist>(Sched_Info *sinfo)</arglist>
    </member>
    <member kind="function">
      <type>void</type>
      <name>L_enqueue_before</name>
      <anchorfile>l__queue_8c.html</anchorfile>
      <anchor>46675dc20c89b99f8127ea049d63661b</anchor>
      <arglist>(Squeue *queue, Sched_Info *sinfo, Sq_entry *before)</arglist>
    </member>
    <member kind="function">
      <type>void</type>
      <name>L_move_entry_before</name>
      <anchorfile>l__queue_8c.html</anchorfile>
      <anchor>4789ec0e89cb6776c8f4a58758e5c45e</anchor>
      <arglist>(Squeue *new_queue, Sq_entry *entry, Sq_entry *before)</arglist>
    </member>
    <member kind="function">
      <type>void</type>
      <name>L_enqueue_min_to_max_1</name>
      <anchorfile>l__queue_8c.html</anchorfile>
      <anchor>1ac15cca6ba6958e428c64cba5aafb47</anchor>
      <arglist>(Squeue *queue, Sched_Info *sinfo, float key)</arglist>
    </member>
    <member kind="function">
      <type>void</type>
      <name>L_enqueue_max_to_min_1</name>
      <anchorfile>l__queue_8c.html</anchorfile>
      <anchor>03c5b423bc932779e1a8baf558f7df36</anchor>
      <arglist>(Squeue *queue, Sched_Info *sinfo, float key)</arglist>
    </member>
    <member kind="function">
      <type>void</type>
      <name>L_enqueue_min_to_max_2</name>
      <anchorfile>l__queue_8c.html</anchorfile>
      <anchor>fb122397e7ce41f45cb1aa891dd1dc19</anchor>
      <arglist>(Squeue *queue, Sched_Info *sinfo, float key1, float key2)</arglist>
    </member>
    <member kind="function">
      <type>void</type>
      <name>L_enqueue_max_to_min_2</name>
      <anchorfile>l__queue_8c.html</anchorfile>
      <anchor>33d377c4451e9af0f1ba651643991074</anchor>
      <arglist>(Squeue *queue, Sched_Info *sinfo, float key1, float key2)</arglist>
    </member>
    <member kind="function">
      <type>void</type>
      <name>L_enqueue_regpres</name>
      <anchorfile>l__queue_8c.html</anchorfile>
      <anchor>8227bd45b0f57f5d6f07e6af4a46b1d6</anchor>
      <arglist>(Squeue *queue, Sched_Info *sinfo, float key1, float key2)</arglist>
    </member>
    <member kind="function">
      <type>int</type>
      <name>L_in_queue</name>
      <anchorfile>l__queue_8c.html</anchorfile>
      <anchor>e1596c8570dd1f1923719e076898d804</anchor>
      <arglist>(Squeue *queue, Sched_Info *sinfo)</arglist>
    </member>
  </compound>
  <compound kind="file">
    <name>l_regpres.c</name>
    <path>/mnt/hgfs/mkt/Desktop/openimpact/src/Lcode/codegen/Lschedule/</path>
    <filename>l__regpres_8c</filename>
    <includes id="l__schedule_8h" name="l_schedule.h" local="yes" imported="no">l_schedule.h</includes>
    <member kind="define">
      <type>#define</type>
      <name>CALLEE</name>
      <anchorfile>l__regpres_8c.html</anchorfile>
      <anchor>7a2ce10cbef56fc76d9d137d04ad19cd</anchor>
      <arglist></arglist>
    </member>
    <member kind="define">
      <type>#define</type>
      <name>CALLER</name>
      <anchorfile>l__regpres_8c.html</anchorfile>
      <anchor>7bf83495c4c1a47e4b5711455b0e29a4</anchor>
      <arglist></arglist>
    </member>
    <member kind="function">
      <type>void</type>
      <name>Lregpres_compute_benefit</name>
      <anchorfile>l__regpres_8c.html</anchorfile>
      <anchor>7525959893a5935d83a5c7a95119856a</anchor>
      <arglist>(Sched_Info *sinfo)</arglist>
    </member>
    <member kind="function">
      <type>int</type>
      <name>Lregpres_inc_current</name>
      <anchorfile>l__regpres_8c.html</anchorfile>
      <anchor>179027718e5fae96806f8cad2c1ce5b9</anchor>
      <arglist>(int ctype, int count)</arglist>
    </member>
    <member kind="function">
      <type>int</type>
      <name>Lregpres_dec_current</name>
      <anchorfile>l__regpres_8c.html</anchorfile>
      <anchor>df259cc603a97e20b027bbe5f2732242</anchor>
      <arglist>(int ctype, int count)</arglist>
    </member>
    <member kind="function">
      <type>void</type>
      <name>Lregpres_inc_set</name>
      <anchorfile>l__regpres_8c.html</anchorfile>
      <anchor>9d3717de99e535d882ba67ab4343593a</anchor>
      <arglist>(Regtypes *set, int ctype)</arglist>
    </member>
    <member kind="function">
      <type>void</type>
      <name>Lregpres_add_virt_reg_use</name>
      <anchorfile>l__regpres_8c.html</anchorfile>
      <anchor>b3b44eca744e1ff03be0b63846341219</anchor>
      <arglist>(Sched_Info *sinfo, int reg_num)</arglist>
    </member>
    <member kind="function">
      <type>void</type>
      <name>Lregpres_init_func</name>
      <anchorfile>l__regpres_8c.html</anchorfile>
      <anchor>9fce931a4a1c83c5e81f5f4c81cf59d3</anchor>
      <arglist>(L_Func *fn)</arglist>
    </member>
    <member kind="function">
      <type>void</type>
      <name>Lregpres_deinit_func</name>
      <anchorfile>l__regpres_8c.html</anchorfile>
      <anchor>180d90dbb20fb9775c8ce3b2301372a3</anchor>
      <arglist>(L_Func *fn)</arglist>
    </member>
    <member kind="function">
      <type>void</type>
      <name>Lregpres_init_cb</name>
      <anchorfile>l__regpres_8c.html</anchorfile>
      <anchor>c5c7ebf3f69738f27a8c4b02f1e6cb60</anchor>
      <arglist>(L_Cb *cb)</arglist>
    </member>
    <member kind="function">
      <type>void</type>
      <name>L_init_ready_queue</name>
      <anchorfile>l__regpres_8c.html</anchorfile>
      <anchor>ce269a612036a7f00eed18771f53b4f4</anchor>
      <arglist>()</arglist>
    </member>
    <member kind="function">
      <type>Sched_Info *</type>
      <name>L_get_next_entry</name>
      <anchorfile>l__regpres_8c.html</anchorfile>
      <anchor>0c02c6c4c315bd64ecf3c512ee700d87</anchor>
      <arglist>()</arglist>
    </member>
    <member kind="function">
      <type>void</type>
      <name>L_insert_entry</name>
      <anchorfile>l__regpres_8c.html</anchorfile>
      <anchor>3774dee00a536b57c57645d709df23fe</anchor>
      <arglist>(Sched_Info *sinfo)</arglist>
    </member>
    <member kind="function">
      <type>void</type>
      <name>L_update_virt_reg</name>
      <anchorfile>l__regpres_8c.html</anchorfile>
      <anchor>1027a0a1f48301f97f9abb9dbf882368</anchor>
      <arglist>(Sched_Info *sinfo)</arglist>
    </member>
    <member kind="variable">
      <type>int</type>
      <name>Lsched_use_register_pressure_heuristic</name>
      <anchorfile>l__regpres_8c.html</anchorfile>
      <anchor>dcfe1fc65128905189f3a03d1983154d</anchor>
      <arglist></arglist>
    </member>
    <member kind="variable">
      <type>int</type>
      <name>Lsched_register_pressure_threshhold</name>
      <anchorfile>l__regpres_8c.html</anchorfile>
      <anchor>d6b0405973ba10a6f980c3dcc31abeb5</anchor>
      <arglist></arglist>
    </member>
    <member kind="variable">
      <type>int</type>
      <name>Lsched_regpres_heuristic</name>
      <anchorfile>l__regpres_8c.html</anchorfile>
      <anchor>b801097b81716757db39ca58936d99d8</anchor>
      <arglist></arglist>
    </member>
    <member kind="variable">
      <type>int</type>
      <name>Lregpres_problem</name>
      <anchorfile>l__regpres_8c.html</anchorfile>
      <anchor>b96327d5a8917f9e25ef7d545d6b61b8</anchor>
      <arglist></arglist>
    </member>
    <member kind="variable">
      <type>int</type>
      <name>num_virt_reg</name>
      <anchorfile>l__regpres_8c.html</anchorfile>
      <anchor>29f2c6acf4cc9d4768a15fe331319db9</anchor>
      <arglist></arglist>
    </member>
    <member kind="variable">
      <type>Vreg *</type>
      <name>virt_reg</name>
      <anchorfile>l__regpres_8c.html</anchorfile>
      <anchor>634254654ecb462669c4a093390bfbc3</anchor>
      <arglist></arglist>
    </member>
    <member kind="variable">
      <type>CB_REG_INFO</type>
      <name>preg_info</name>
      <anchorfile>l__regpres_8c.html</anchorfile>
      <anchor>0647b4f1b08b4ccefd9a78e4a6ab1b54</anchor>
      <arglist></arglist>
    </member>
    <member kind="variable">
      <type>CB_REG_INFO</type>
      <name>ireg_info</name>
      <anchorfile>l__regpres_8c.html</anchorfile>
      <anchor>abc4d42199592ce82fbf255e14709f17</anchor>
      <arglist></arglist>
    </member>
    <member kind="variable">
      <type>CB_REG_INFO</type>
      <name>freg_info</name>
      <anchorfile>l__regpres_8c.html</anchorfile>
      <anchor>360b283a865b12acd3078199a40caafd</anchor>
      <arglist></arglist>
    </member>
    <member kind="variable">
      <type>CB_REG_INFO</type>
      <name>dreg_info</name>
      <anchorfile>l__regpres_8c.html</anchorfile>
      <anchor>3c2afab1c79a0e20251f42283e7ad216</anchor>
      <arglist></arglist>
    </member>
  </compound>
  <compound kind="file">
    <name>l_ru_interface.c</name>
    <path>/mnt/hgfs/mkt/Desktop/openimpact/src/Lcode/codegen/Lschedule/</path>
    <filename>l__ru__interface_8c</filename>
    <includes id="l__schedule_8h" name="l_schedule.h" local="yes" imported="no">l_schedule.h</includes>
    <includes id="RU__manager_8h" name="RU_manager.h" local="yes" imported="no">RU_manager.h</includes>
    <includes id="l__ru__interface_8h" name="l_ru_interface.h" local="yes" imported="no">l_ru_interface.h</includes>
    <member kind="function">
      <type>RU_Info *</type>
      <name>L_create_ru_info_oper</name>
      <anchorfile>l__ru__interface_8c.html</anchorfile>
      <anchor>67483f170cf0499fd0a9883f20996904</anchor>
      <arglist>(L_Oper *oper)</arglist>
    </member>
    <member kind="function">
      <type>void</type>
      <name>L_delete_ru_info_oper</name>
      <anchorfile>l__ru__interface_8c.html</anchorfile>
      <anchor>eb901062d6a1d316284588ab0c1a1748</anchor>
      <arglist>(RU_Info *ru_info)</arglist>
    </member>
  </compound>
  <compound kind="file">
    <name>l_ru_interface.h</name>
    <path>/mnt/hgfs/mkt/Desktop/openimpact/src/Lcode/codegen/Lschedule/</path>
    <filename>l__ru__interface_8h</filename>
    <member kind="function">
      <type>RU_Info *</type>
      <name>L_create_ru_info_oper</name>
      <anchorfile>l__ru__interface_8h.html</anchorfile>
      <anchor>67483f170cf0499fd0a9883f20996904</anchor>
      <arglist>(L_Oper *oper)</arglist>
    </member>
    <member kind="function">
      <type>void</type>
      <name>L_create_ru_info_cb</name>
      <anchorfile>l__ru__interface_8h.html</anchorfile>
      <anchor>079198444274d448dbc1619e4880f75f</anchor>
      <arglist>(L_Cb *cb)</arglist>
    </member>
    <member kind="function">
      <type>void</type>
      <name>L_create_ru_info_fn</name>
      <anchorfile>l__ru__interface_8h.html</anchorfile>
      <anchor>d47d6ce431312256dc8b3bc27fa25ba3</anchor>
      <arglist>(L_Func *fn)</arglist>
    </member>
    <member kind="function">
      <type>void</type>
      <name>L_delete_ru_info_oper</name>
      <anchorfile>l__ru__interface_8h.html</anchorfile>
      <anchor>eb901062d6a1d316284588ab0c1a1748</anchor>
      <arglist>(RU_Info *ru_info)</arglist>
    </member>
    <member kind="function">
      <type>void</type>
      <name>L_delete_ru_info_cb</name>
      <anchorfile>l__ru__interface_8h.html</anchorfile>
      <anchor>68e5f8e822141078909afcc69ff53d8c</anchor>
      <arglist>(L_Cb *cb)</arglist>
    </member>
    <member kind="function">
      <type>void</type>
      <name>L_delete_ru_info_fn</name>
      <anchorfile>l__ru__interface_8h.html</anchorfile>
      <anchor>633dc82be8860b9c7a36bbf17cfc21f4</anchor>
      <arglist>(L_Func *fn)</arglist>
    </member>
  </compound>
  <compound kind="file">
    <name>l_schedule.c</name>
    <path>/mnt/hgfs/mkt/Desktop/openimpact/src/Lcode/codegen/Lschedule/</path>
    <filename>l__schedule_8c</filename>
    <includes id="l__schedule_8h" name="l_schedule.h" local="yes" imported="no">l_schedule.h</includes>
    <includes id="l__mcb_8h" name="l_mcb.h" local="yes" imported="no">l_mcb.h</includes>
    <class kind="struct">LEAF</class>
    <member kind="define">
      <type>#define</type>
      <name>FALL_THRU</name>
      <anchorfile>l__schedule_8c.html</anchorfile>
      <anchor>78acf9a760383460d3aac987a8e46bd3</anchor>
      <arglist></arglist>
    </member>
    <member kind="define">
      <type>#define</type>
      <name>UBR_TAKEN</name>
      <anchorfile>l__schedule_8c.html</anchorfile>
      <anchor>9edb73b1b8df6d4ae828b11ea66b461e</anchor>
      <arglist></arglist>
    </member>
    <member kind="define">
      <type>#define</type>
      <name>CBR_TAKEN</name>
      <anchorfile>l__schedule_8c.html</anchorfile>
      <anchor>87e3ac61028234f430493ba624e9c6f5</anchor>
      <arglist></arglist>
    </member>
    <member kind="define">
      <type>#define</type>
      <name>NO_CB_PAD</name>
      <anchorfile>l__schedule_8c.html</anchorfile>
      <anchor>41d6c6a83879de5e8a3c3c62532d9a4c</anchor>
      <arglist></arglist>
    </member>
    <member kind="function">
      <type>int</type>
      <name>Lsched_loads_per_cycle</name>
      <anchorfile>l__schedule_8c.html</anchorfile>
      <anchor>374490810b4ebc38b10ce86b61d20241</anchor>
      <arglist>()</arglist>
    </member>
    <member kind="function">
      <type>int</type>
      <name>Lsched_stores_per_cycle</name>
      <anchorfile>l__schedule_8c.html</anchorfile>
      <anchor>5a7ad0e9648a9f387c0d7421d2f55947</anchor>
      <arglist>()</arglist>
    </member>
    <member kind="function">
      <type>int</type>
      <name>Lsched_branches_per_cycle</name>
      <anchorfile>l__schedule_8c.html</anchorfile>
      <anchor>88f473ee64c9d02c72fbce4fbed050b1</anchor>
      <arglist>()</arglist>
    </member>
    <member kind="function">
      <type>int</type>
      <name>Lsched_issue_time</name>
      <anchorfile>l__schedule_8c.html</anchorfile>
      <anchor>92dd48019f20a24c8bb797604f98568e</anchor>
      <arglist>(L_Oper *oper)</arglist>
    </member>
    <member kind="function">
      <type>int</type>
      <name>Lsched_completion_time</name>
      <anchorfile>l__schedule_8c.html</anchorfile>
      <anchor>9e4ae98a71c7047b700d6f583cac4810</anchor>
      <arglist>(L_Oper *oper)</arglist>
    </member>
    <member kind="function">
      <type>int</type>
      <name>Lsched_latency</name>
      <anchorfile>l__schedule_8c.html</anchorfile>
      <anchor>f5f80bb6ba0f18e8be67e0ec1be834c1</anchor>
      <arglist>(L_Oper *oper)</arglist>
    </member>
    <member kind="function">
      <type>void</type>
      <name>L_read_parm_lsched</name>
      <anchorfile>l__schedule_8c.html</anchorfile>
      <anchor>a94ad87ba5f6079a85916abd4c36e0c2</anchor>
      <arglist>(Parm_Parse_Info *ppi)</arglist>
    </member>
    <member kind="function">
      <type>void</type>
      <name>Lsched_init</name>
      <anchorfile>l__schedule_8c.html</anchorfile>
      <anchor>a55aae15093dd030630975c50933ac30</anchor>
      <arglist>(Parm_Macro_List *command_line_macro_list, char *lmdes_file_name)</arglist>
    </member>
    <member kind="function">
      <type>void</type>
      <name>Lsched_renumber_issue_slots</name>
      <anchorfile>l__schedule_8c.html</anchorfile>
      <anchor>e494cec59630ffe567ad5b8a1062ab5c</anchor>
      <arglist>(L_Func *fn)</arglist>
    </member>
    <member kind="function">
      <type>void</type>
      <name>Lsched_func_complete</name>
      <anchorfile>l__schedule_8c.html</anchorfile>
      <anchor>548bd94fba94b9dfa0fb7dfea903bbd4</anchor>
      <arglist>(L_Func *fn)</arglist>
    </member>
    <member kind="function">
      <type>Sched_Info *</type>
      <name>L_create_sched_info</name>
      <anchorfile>l__schedule_8c.html</anchorfile>
      <anchor>268dd685af46604df42953774a82cca3</anchor>
      <arglist>(L_Oper *oper, int home_block, L_Oper *prev_br, L_Oper *post_br)</arglist>
    </member>
    <member kind="function">
      <type>void</type>
      <name>L_update_sched_info</name>
      <anchorfile>l__schedule_8c.html</anchorfile>
      <anchor>3d2397076410181103f5fea34a5acff0</anchor>
      <arglist>(Sched_Info *sinfo, L_Oper *oper, int home_block)</arglist>
    </member>
    <member kind="function">
      <type>void</type>
      <name>L_delete_sched_info</name>
      <anchorfile>l__schedule_8c.html</anchorfile>
      <anchor>8a1f800cff6d1e8b40ae9b11667c05dd</anchor>
      <arglist>(Sched_Info *sinfo)</arglist>
    </member>
    <member kind="function">
      <type>void</type>
      <name>L_print_sched_info</name>
      <anchorfile>l__schedule_8c.html</anchorfile>
      <anchor>984f853032a84d37536ca6225c560c9a</anchor>
      <arglist>(FILE *F, Sched_Info *s_info)</arglist>
    </member>
    <member kind="function">
      <type>void</type>
      <name>Lsched_init_etimes_and_ltimes</name>
      <anchorfile>l__schedule_8c.html</anchorfile>
      <anchor>6c5b84bc622cfdd643bb13b29e9570d5</anchor>
      <arglist>(L_Cb *cb)</arglist>
    </member>
    <member kind="function">
      <type>int</type>
      <name>Lsched_determine_etimes</name>
      <anchorfile>l__schedule_8c.html</anchorfile>
      <anchor>d60fd7dc501d262065cd706caf9b6fc8</anchor>
      <arglist>(L_Cb *cb, L_Oper ***exit_ops)</arglist>
    </member>
    <member kind="function">
      <type>void</type>
      <name>Lsched_determine_ltimes</name>
      <anchorfile>l__schedule_8c.html</anchorfile>
      <anchor>08a2b260fa792027bc09bbaf9858e331</anchor>
      <arglist>(L_Cb *cb, int num_exits, L_Oper **exit_ops)</arglist>
    </member>
    <member kind="function">
      <type>void</type>
      <name>Lsched_calculate_priority_normal</name>
      <anchorfile>l__schedule_8c.html</anchorfile>
      <anchor>4e16f3a046599410c9dea41ee3313473</anchor>
      <arglist>(L_Cb *cb, int num_exits, L_Oper **exit_ops)</arglist>
    </member>
    <member kind="function">
      <type>int</type>
      <name>Lsched_is_branch</name>
      <anchorfile>l__schedule_8c.html</anchorfile>
      <anchor>cc88d49ca4d88e9903f8f4becff22831</anchor>
      <arglist>(int proc_opc)</arglist>
    </member>
    <member kind="function">
      <type>L_Oper *</type>
      <name>L_find_next_branch</name>
      <anchorfile>l__schedule_8c.html</anchorfile>
      <anchor>6d691d5261d79228e4e680680a58f405</anchor>
      <arglist>(L_Cb *cb, L_Oper *oper)</arglist>
    </member>
    <member kind="function">
      <type>void</type>
      <name>L_annotate_instr</name>
      <anchorfile>l__schedule_8c.html</anchorfile>
      <anchor>e1d1f2e8c81606a280a520d8feb5d7f8</anchor>
      <arglist>(L_Cb *cb)</arglist>
    </member>
    <member kind="function">
      <type>void</type>
      <name>Lsched_make_ready</name>
      <anchorfile>l__schedule_8c.html</anchorfile>
      <anchor>afdc2e63f43924ffd008603f33e5f56a</anchor>
      <arglist>(L_Cb *cb, int oper_issue_time, Sched_Info *sinfo, Sched_Info *dep_sinfo, int to_index, int from_index, int distance)</arglist>
    </member>
    <member kind="function">
      <type>void</type>
      <name>Lsched_schedule_op</name>
      <anchorfile>l__schedule_8c.html</anchorfile>
      <anchor>53b29f1251e9c43df47311cd38555125</anchor>
      <arglist>(L_Cb *cb, Sched_Info *sinfo, int spec, int ready_time)</arglist>
    </member>
    <member kind="function">
      <type>int</type>
      <name>Lsched_can_schedule_op</name>
      <anchorfile>l__schedule_8c.html</anchorfile>
      <anchor>dffafbbbcd5e813ae2e9057db8a5853e</anchor>
      <arglist>(L_Cb *cb, Sched_Info *sinfo, int current_time, int current_block, int *issue_slot, int *spec, int *silent)</arglist>
    </member>
    <member kind="function" static="yes">
      <type>static L_Oper *</type>
      <name>Lsched_create_nop</name>
      <anchorfile>l__schedule_8c.html</anchorfile>
      <anchor>29f3519886531eee9bcddfd3d204d364</anchor>
      <arglist>(int issue_time, int issue_slot)</arglist>
    </member>
    <member kind="function" static="yes">
      <type>static L_Oper *</type>
      <name>Lsched_create_jump</name>
      <anchorfile>l__schedule_8c.html</anchorfile>
      <anchor>b274a5699dfc00ff67c351458a2703ad</anchor>
      <arglist>(int issue_time, int issue_slot, L_Operand *target)</arglist>
    </member>
    <member kind="function" static="yes">
      <type>static void</type>
      <name>Lsched_pad_slots_with_nops</name>
      <anchorfile>l__schedule_8c.html</anchorfile>
      <anchor>7d13a484c6cb9d6e9717830d702cca04</anchor>
      <arglist>(L_Cb *cb)</arglist>
    </member>
    <member kind="function">
      <type>int</type>
      <name>Lsched_get_cb_start_pad_value</name>
      <anchorfile>l__schedule_8c.html</anchorfile>
      <anchor>8df7af51ac5aa60f63d83ae7d6989db6</anchor>
      <arglist>(L_Cb *cb)</arglist>
    </member>
    <member kind="function">
      <type>void</type>
      <name>Lsched_create_cb_start_pad</name>
      <anchorfile>l__schedule_8c.html</anchorfile>
      <anchor>c26c26ff3edd8f6887aaf172c38fcfd2</anchor>
      <arglist>(L_Cb *cb, int pad_value)</arglist>
    </member>
    <member kind="function">
      <type>int</type>
      <name>Lsched_get_cb_end_pad_value</name>
      <anchorfile>l__schedule_8c.html</anchorfile>
      <anchor>497f05aa980592a2e1edbdd0fbfaee76</anchor>
      <arglist>(L_Cb *cb)</arglist>
    </member>
    <member kind="function">
      <type>void</type>
      <name>Lsched_create_cb_end_pad</name>
      <anchorfile>l__schedule_8c.html</anchorfile>
      <anchor>d3e42788dce5fe5bece79fd15af24462</anchor>
      <arglist>(L_Cb *cb, int pad_value)</arglist>
    </member>
    <member kind="function">
      <type>int</type>
      <name>Lsched_get_br_pad_value</name>
      <anchorfile>l__schedule_8c.html</anchorfile>
      <anchor>e4c45c26b8351919c84a714d5f98882f</anchor>
      <arglist>(L_Oper *oper)</arglist>
    </member>
    <member kind="function">
      <type>void</type>
      <name>Lsched_create_br_pad</name>
      <anchorfile>l__schedule_8c.html</anchorfile>
      <anchor>f3df48ff01088f85fd1e6b1ddf8ca06b</anchor>
      <arglist>(L_Oper *oper, int pad_value)</arglist>
    </member>
    <member kind="function">
      <type>int</type>
      <name>Lsched_cb_completion_time</name>
      <anchorfile>l__schedule_8c.html</anchorfile>
      <anchor>ea239380278678733fcc0300478dca01</anchor>
      <arglist>(L_Cb *cb)</arglist>
    </member>
    <member kind="function">
      <type>int</type>
      <name>Lsched_need_pad</name>
      <anchorfile>l__schedule_8c.html</anchorfile>
      <anchor>aa099d7828bbac61af2f67eaac8c4b9a</anchor>
      <arglist>(L_Cb *target, L_Oper *oper, int overextend)</arglist>
    </member>
    <member kind="function" static="yes">
      <type>static void</type>
      <name>Lsched_determine_pad_for_vliw</name>
      <anchorfile>l__schedule_8c.html</anchorfile>
      <anchor>7d91b6ba74db4857cb38588bfbfe9c63</anchor>
      <arglist>(L_Cb *cb)</arglist>
    </member>
    <member kind="function" static="yes">
      <type>static void</type>
      <name>Lsched_pad_cycles_start_of_cb_for_vliw</name>
      <anchorfile>l__schedule_8c.html</anchorfile>
      <anchor>285509e2641f69859c90cc655f512dd8</anchor>
      <arglist>(L_Cb *cb)</arglist>
    </member>
    <member kind="function" static="yes">
      <type>static void</type>
      <name>Lsched_pad_cycles_end_of_cb_for_vliw</name>
      <anchorfile>l__schedule_8c.html</anchorfile>
      <anchor>39431a5fbacd2cea668db7733c89038e</anchor>
      <arglist>(L_Cb *cb)</arglist>
    </member>
    <member kind="function" static="yes">
      <type>static void</type>
      <name>Lsched_pad_cycles_for_vliw</name>
      <anchorfile>l__schedule_8c.html</anchorfile>
      <anchor>219e20eb7ceb63961aab25b01b50708e</anchor>
      <arglist>(L_Func *fn, L_Cb *cb)</arglist>
    </member>
    <member kind="function" static="yes">
      <type>static void</type>
      <name>Lsched_correct_schedule_for_vliw</name>
      <anchorfile>l__schedule_8c.html</anchorfile>
      <anchor>ae97816316ba9759ce6777c433e9f3c4</anchor>
      <arglist>(L_Func *fn)</arglist>
    </member>
    <member kind="function">
      <type>void</type>
      <name>Lsched_add_isl_attr</name>
      <anchorfile>l__schedule_8c.html</anchorfile>
      <anchor>1882c164271a9d0021a970f2735de0b0</anchor>
      <arglist>(L_Func *fn)</arglist>
    </member>
    <member kind="function">
      <type>void</type>
      <name>Lsched_delete_isl_attr</name>
      <anchorfile>l__schedule_8c.html</anchorfile>
      <anchor>5877322a652d30df0a7ad278ee7d26e6</anchor>
      <arglist>(L_Func *fn)</arglist>
    </member>
    <member kind="function">
      <type>void</type>
      <name>Lsched_fix_flows</name>
      <anchorfile>l__schedule_8c.html</anchorfile>
      <anchor>9a49b776bfb91fe021d561770910b8a2</anchor>
      <arglist>(L_Cb *cb)</arglist>
    </member>
    <member kind="function">
      <type>void</type>
      <name>Lsched_schedule_block</name>
      <anchorfile>l__schedule_8c.html</anchorfile>
      <anchor>7d9e535e6ceefbc9b8be92d3349d67a0</anchor>
      <arglist>(L_Cb *cb)</arglist>
    </member>
    <member kind="function">
      <type>void</type>
      <name>Lsched_prepass_code_scheduling</name>
      <anchorfile>l__schedule_8c.html</anchorfile>
      <anchor>d4ed46984d617a811319d1f38f269481</anchor>
      <arglist>(L_Func *fn)</arglist>
    </member>
    <member kind="function">
      <type>void</type>
      <name>Lsched_postpass_code_scheduling</name>
      <anchorfile>l__schedule_8c.html</anchorfile>
      <anchor>97697bfbdf7be22085fc3d8b3f177d72</anchor>
      <arglist>(L_Func *fn)</arglist>
    </member>
    <member kind="variable">
      <type>int</type>
      <name>Lsched_debug_operation_priority</name>
      <anchorfile>l__schedule_8c.html</anchorfile>
      <anchor>54976628291eb350de98246e583e7069</anchor>
      <arglist></arglist>
    </member>
    <member kind="variable">
      <type>int</type>
      <name>Lsched_debug_prepass_scheduling</name>
      <anchorfile>l__schedule_8c.html</anchorfile>
      <anchor>ab0ccf85b37a82c182ce4edd6814eab9</anchor>
      <arglist></arglist>
    </member>
    <member kind="variable">
      <type>int</type>
      <name>Lsched_debug_postpass_scheduling</name>
      <anchorfile>l__schedule_8c.html</anchorfile>
      <anchor>cb6bd68cf55e01ed524d25aec561a569</anchor>
      <arglist></arglist>
    </member>
    <member kind="variable">
      <type>int</type>
      <name>Lsched_debug_squashing_branches</name>
      <anchorfile>l__schedule_8c.html</anchorfile>
      <anchor>edb9555f6c00ca8943e8a8a058731e68</anchor>
      <arglist></arglist>
    </member>
    <member kind="variable">
      <type>int</type>
      <name>Lsched_debug_nonsquashing_branches</name>
      <anchorfile>l__schedule_8c.html</anchorfile>
      <anchor>9ee6e81eaa3a35d7523b7d81f38cdbc8</anchor>
      <arglist></arglist>
    </member>
    <member kind="variable">
      <type>int</type>
      <name>Lsched_debug_unfilled_branches</name>
      <anchorfile>l__schedule_8c.html</anchorfile>
      <anchor>4c875e480a2233a014e03f62cab39191</anchor>
      <arglist></arglist>
    </member>
    <member kind="variable">
      <type>int</type>
      <name>Lsched_debug_use_sched_cb_bounds</name>
      <anchorfile>l__schedule_8c.html</anchorfile>
      <anchor>c3f7692f32a5f078bb087c6709be3dc6</anchor>
      <arglist></arglist>
    </member>
    <member kind="variable">
      <type>int</type>
      <name>Lsched_debug_lower_sched_cb_bound</name>
      <anchorfile>l__schedule_8c.html</anchorfile>
      <anchor>3b4f7a21cbaf1b8ed78ae395b1f610b5</anchor>
      <arglist></arglist>
    </member>
    <member kind="variable">
      <type>int</type>
      <name>Lsched_debug_upper_sched_cb_bound</name>
      <anchorfile>l__schedule_8c.html</anchorfile>
      <anchor>e75029d0ece9ff7e181c53e7091e054c</anchor>
      <arglist></arglist>
    </member>
    <member kind="variable">
      <type>int</type>
      <name>Lsched_num_branches_per_cycle</name>
      <anchorfile>l__schedule_8c.html</anchorfile>
      <anchor>42512cec3f5bb5c7b60f1db41f252ec7</anchor>
      <arglist></arglist>
    </member>
    <member kind="variable">
      <type>int</type>
      <name>Lsched_static_fall_thru_weight</name>
      <anchorfile>l__schedule_8c.html</anchorfile>
      <anchor>87ee175780759eb78fb67c94f584eeef</anchor>
      <arglist></arglist>
    </member>
    <member kind="variable">
      <type>int</type>
      <name>Lsched_pad_height</name>
      <anchorfile>l__schedule_8c.html</anchorfile>
      <anchor>cdd4938179f0cfad035fefce3c2ced7a</anchor>
      <arglist></arglist>
    </member>
    <member kind="variable">
      <type>int</type>
      <name>Lsched_use_fan_out</name>
      <anchorfile>l__schedule_8c.html</anchorfile>
      <anchor>dc2b7936a51554fc91cf180edacf0b19</anchor>
      <arglist></arglist>
    </member>
    <member kind="variable">
      <type>int</type>
      <name>Lsched_infinite_issue</name>
      <anchorfile>l__schedule_8c.html</anchorfile>
      <anchor>a41d94228aa003482c3c8da8841c420f</anchor>
      <arglist></arglist>
    </member>
    <member kind="variable">
      <type>int</type>
      <name>Lsched_do_postpass_scheduling</name>
      <anchorfile>l__schedule_8c.html</anchorfile>
      <anchor>0feea7b69688d91321677afa857b2e9b</anchor>
      <arglist></arglist>
    </member>
    <member kind="variable">
      <type>int</type>
      <name>Lsched_demote_all_the_way</name>
      <anchorfile>l__schedule_8c.html</anchorfile>
      <anchor>a42d0c45d271917fdbdc1cc4daa18b1d</anchor>
      <arglist></arglist>
    </member>
    <member kind="variable">
      <type>int</type>
      <name>Lsched_do_fill_squashing_branches</name>
      <anchorfile>l__schedule_8c.html</anchorfile>
      <anchor>073f10e729943841844c9ac33ab08760</anchor>
      <arglist></arglist>
    </member>
    <member kind="variable">
      <type>int</type>
      <name>Lsched_do_fill_nonsquashing_branches</name>
      <anchorfile>l__schedule_8c.html</anchorfile>
      <anchor>71c03ff0a19ef4532954edc78836a18b</anchor>
      <arglist></arglist>
    </member>
    <member kind="variable">
      <type>int</type>
      <name>Lsched_do_fill_unfilled_branches</name>
      <anchorfile>l__schedule_8c.html</anchorfile>
      <anchor>51b02dd4ff5d452f0832853d61065c2c</anchor>
      <arglist></arglist>
    </member>
    <member kind="variable">
      <type>int</type>
      <name>Lsched_include_only_oper_lt_hb</name>
      <anchorfile>l__schedule_8c.html</anchorfile>
      <anchor>20a06bed1bdca8cd03cffb1cbe5b588b</anchor>
      <arglist></arglist>
    </member>
    <member kind="variable">
      <type>int</type>
      <name>Lsched_print_statistics</name>
      <anchorfile>l__schedule_8c.html</anchorfile>
      <anchor>e677cd429292523a696a32d17cc7e389</anchor>
      <arglist></arglist>
    </member>
    <member kind="variable">
      <type>int</type>
      <name>Lsched_print_prepass_statistics</name>
      <anchorfile>l__schedule_8c.html</anchorfile>
      <anchor>181a18821bedca4935435e87b165d3a0</anchor>
      <arglist></arglist>
    </member>
    <member kind="variable">
      <type>int</type>
      <name>Lsched_print_spec_condition</name>
      <anchorfile>l__schedule_8c.html</anchorfile>
      <anchor>c3946267c46785410b547ee22a070b17</anchor>
      <arglist></arglist>
    </member>
    <member kind="variable">
      <type>int</type>
      <name>Lsched_do_sentinel_recovery</name>
      <anchorfile>l__schedule_8c.html</anchorfile>
      <anchor>e92a2b503333948c586d0b6907833c94</anchor>
      <arglist></arglist>
    </member>
    <member kind="variable">
      <type>int</type>
      <name>Lsched_prepass</name>
      <anchorfile>l__schedule_8c.html</anchorfile>
      <anchor>fac4677c213f235af5efb6e5fd7cde1b</anchor>
      <arglist></arglist>
    </member>
    <member kind="variable">
      <type>int</type>
      <name>Lsched_print_hb_and_spec</name>
      <anchorfile>l__schedule_8c.html</anchorfile>
      <anchor>af219a307f8e80d27600cf40832135af</anchor>
      <arglist></arglist>
    </member>
    <member kind="variable">
      <type>int</type>
      <name>Lsched_print_cycle_delimiter</name>
      <anchorfile>l__schedule_8c.html</anchorfile>
      <anchor>7689853b9040f1b843690d0c0e7de833</anchor>
      <arglist></arglist>
    </member>
    <member kind="variable">
      <type>int</type>
      <name>Lsched_num_opers</name>
      <anchorfile>l__schedule_8c.html</anchorfile>
      <anchor>81a1b68006a09b691fdbe6e8d1fb354d</anchor>
      <arglist></arglist>
    </member>
    <member kind="variable">
      <type>int</type>
      <name>Lsched_loads_each_cycle</name>
      <anchorfile>l__schedule_8c.html</anchorfile>
      <anchor>14f9b657ee8e365347a970ed01a869a1</anchor>
      <arglist></arglist>
    </member>
    <member kind="variable">
      <type>int</type>
      <name>Lsched_stores_each_cycle</name>
      <anchorfile>l__schedule_8c.html</anchorfile>
      <anchor>9be7ba98ab64e7c71052045fd2e066b2</anchor>
      <arglist></arglist>
    </member>
    <member kind="variable">
      <type>int</type>
      <name>current_time</name>
      <anchorfile>l__schedule_8c.html</anchorfile>
      <anchor>8915224642e155ba06d1682e27c56c41</anchor>
      <arglist></arglist>
    </member>
    <member kind="variable">
      <type>int</type>
      <name>Lsched_infinite_issue_slot</name>
      <anchorfile>l__schedule_8c.html</anchorfile>
      <anchor>2d920740be5e3d47a1f43c90428182eb</anchor>
      <arglist></arglist>
    </member>
    <member kind="variable">
      <type>int</type>
      <name>Lsched_pad_vliw_slots_with_nops</name>
      <anchorfile>l__schedule_8c.html</anchorfile>
      <anchor>b5f9a08750a50c4fa56c2f52f372749b</anchor>
      <arglist></arglist>
    </member>
    <member kind="variable">
      <type>int</type>
      <name>Lsched_vliw_has_interlocking</name>
      <anchorfile>l__schedule_8c.html</anchorfile>
      <anchor>50679cc20a4e01b36d91b0593ea51875</anchor>
      <arglist></arglist>
    </member>
    <member kind="variable">
      <type>int</type>
      <name>Lsched_do_renumber_issue_slots</name>
      <anchorfile>l__schedule_8c.html</anchorfile>
      <anchor>a4b5460a6770efd4d849eb261e856daa</anchor>
      <arglist></arglist>
    </member>
    <member kind="variable">
      <type>char *</type>
      <name>Lsched_profile_info_to_use</name>
      <anchorfile>l__schedule_8c.html</anchorfile>
      <anchor>11161fe0b364a73fa794db32524bd926</anchor>
      <arglist></arglist>
    </member>
    <member kind="variable">
      <type>L_Oper *</type>
      <name>Lsched_latest_br</name>
      <anchorfile>l__schedule_8c.html</anchorfile>
      <anchor>9038dbd897cfe1df535858875651b3e4</anchor>
      <arglist></arglist>
    </member>
    <member kind="variable">
      <type>Squeue *</type>
      <name>scheduled_queue</name>
      <anchorfile>l__schedule_8c.html</anchorfile>
      <anchor>b45d8d03ce37d7b7cf89b0321c5d3c28</anchor>
      <arglist></arglist>
    </member>
    <member kind="variable">
      <type>Squeue *</type>
      <name>priority_ready_queue</name>
      <anchorfile>l__schedule_8c.html</anchorfile>
      <anchor>7ad0faa72489db11d3885ff9a0f42366</anchor>
      <arglist></arglist>
    </member>
    <member kind="variable">
      <type>Squeue *</type>
      <name>regpres_ready_queue</name>
      <anchorfile>l__schedule_8c.html</anchorfile>
      <anchor>99d40e945682d462016f6899c74be8d9</anchor>
      <arglist></arglist>
    </member>
    <member kind="variable">
      <type>Squeue *</type>
      <name>pending_ready_queue</name>
      <anchorfile>l__schedule_8c.html</anchorfile>
      <anchor>065691cd75db4683c71bba868f90264d</anchor>
      <arglist></arglist>
    </member>
    <member kind="variable">
      <type>Squeue *</type>
      <name>not_ready_queue</name>
      <anchorfile>l__schedule_8c.html</anchorfile>
      <anchor>fe3f0aa3f7900d8acb68f9c302f32a7f</anchor>
      <arglist></arglist>
    </member>
    <member kind="variable">
      <type>int</type>
      <name>Lsched_processor_model</name>
      <anchorfile>l__schedule_8c.html</anchorfile>
      <anchor>4d59bf1a4de63a2857de2b9f53abd7b2</anchor>
      <arglist></arglist>
    </member>
    <member kind="variable">
      <type>int</type>
      <name>Lsched_total_issue_slots</name>
      <anchorfile>l__schedule_8c.html</anchorfile>
      <anchor>7511cf13b18125e863ce2bb076d8f1cd</anchor>
      <arglist></arglist>
    </member>
    <member kind="variable">
      <type>float</type>
      <name>Lsched_branch_increase</name>
      <anchorfile>l__schedule_8c.html</anchorfile>
      <anchor>426e0448adb8542b89a2e698d6020bb2</anchor>
      <arglist></arglist>
    </member>
    <member kind="variable">
      <type>int</type>
      <name>Lsched_fill_delay_slots</name>
      <anchorfile>l__schedule_8c.html</anchorfile>
      <anchor>6d53c903b4fd077ed1a02626dbebfda0</anchor>
      <arglist></arglist>
    </member>
    <member kind="variable">
      <type>int</type>
      <name>Lsched_debug_messages</name>
      <anchorfile>l__schedule_8c.html</anchorfile>
      <anchor>69b188aa5e7ffe9ebe9d25ada889bb4f</anchor>
      <arglist></arglist>
    </member>
    <member kind="variable">
      <type>int</type>
      <name>Lmcb_keep_checks</name>
      <anchorfile>l__schedule_8c.html</anchorfile>
      <anchor>7fa8b9ab156b1588456d5055d2d2633f</anchor>
      <arglist></arglist>
    </member>
    <member kind="variable">
      <type>L_Alloc_Pool *</type>
      <name>Squeue_pool</name>
      <anchorfile>l__schedule_8c.html</anchorfile>
      <anchor>70490b1c3107e1406d0a788c58460d61</anchor>
      <arglist></arglist>
    </member>
    <member kind="variable">
      <type>L_Alloc_Pool *</type>
      <name>Sq_entry_pool</name>
      <anchorfile>l__schedule_8c.html</anchorfile>
      <anchor>905d903f4a74734bcd6535bdc20f5c84</anchor>
      <arglist></arglist>
    </member>
    <member kind="variable">
      <type>L_Alloc_Pool *</type>
      <name>Sched_Info_pool</name>
      <anchorfile>l__schedule_8c.html</anchorfile>
      <anchor>373147875f98887da63ee267f1ac4cfe</anchor>
      <arglist></arglist>
    </member>
    <member kind="variable">
      <type>L_Alloc_Pool *</type>
      <name>Operand_ready_pool</name>
      <anchorfile>l__schedule_8c.html</anchorfile>
      <anchor>5aeea507b43d25bfcbdd09f85aa420ca</anchor>
      <arglist></arglist>
    </member>
  </compound>
  <compound kind="file">
    <name>l_schedule.h</name>
    <path>/mnt/hgfs/mkt/Desktop/openimpact/src/Lcode/codegen/Lschedule/</path>
    <filename>l__schedule_8h</filename>
    <includes id="l__dependence_8h" name="l_dependence.h" local="yes" imported="no">l_dependence.h</includes>
    <includes id="RU__manager_8h" name="RU_manager.h" local="yes" imported="no">RU_manager.h</includes>
    <includes id="l__ru__interface_8h" name="l_ru_interface.h" local="yes" imported="no">l_ru_interface.h</includes>
    <class kind="struct">Squeue</class>
    <class kind="struct">Sq_entry</class>
    <class kind="struct">Regtypes</class>
    <class kind="struct">CB_REG_INFO</class>
    <class kind="struct">Vreg</class>
    <class kind="struct">Sched_Info</class>
    <member kind="define">
      <type>#define</type>
      <name>TRUE</name>
      <anchorfile>l__schedule_8h.html</anchorfile>
      <anchor>a8cecfc5c5c054d2875c03e77b7be15d</anchor>
      <arglist></arglist>
    </member>
    <member kind="define">
      <type>#define</type>
      <name>FALSE</name>
      <anchorfile>l__schedule_8h.html</anchorfile>
      <anchor>a93f0eb578d23995850d61f7d61c55c1</anchor>
      <arglist></arglist>
    </member>
    <member kind="define">
      <type>#define</type>
      <name>FILLER</name>
      <anchorfile>l__schedule_8h.html</anchorfile>
      <anchor>6b05dd372c722a88dc8ee967ca69853e</anchor>
      <arglist></arglist>
    </member>
    <member kind="define">
      <type>#define</type>
      <name>FILLED_ABOVE</name>
      <anchorfile>l__schedule_8h.html</anchorfile>
      <anchor>70d31d79e902bf3c29f60056750ba89d</anchor>
      <arglist></arglist>
    </member>
    <member kind="define">
      <type>#define</type>
      <name>FILLED_BELOW</name>
      <anchorfile>l__schedule_8h.html</anchorfile>
      <anchor>e2248cdf40679f28b0ffbb6ba4e6d957</anchor>
      <arglist></arglist>
    </member>
    <member kind="define">
      <type>#define</type>
      <name>CAN_SCHEDULE</name>
      <anchorfile>l__schedule_8h.html</anchorfile>
      <anchor>c0ae0877300188829ef842b4479e1c69</anchor>
      <arglist></arglist>
    </member>
    <member kind="define">
      <type>#define</type>
      <name>CANT_SCHEDULE</name>
      <anchorfile>l__schedule_8h.html</anchorfile>
      <anchor>763ecfc7a859945e3f34e10564cf7030</anchor>
      <arglist></arglist>
    </member>
    <member kind="define">
      <type>#define</type>
      <name>SCHED_INFO</name>
      <anchorfile>l__schedule_8h.html</anchorfile>
      <anchor>a250be9242d11e63322214859df40491</anchor>
      <arglist>(oper)</arglist>
    </member>
    <member kind="define">
      <type>#define</type>
      <name>IS_UCOND_BRANCH</name>
      <anchorfile>l__schedule_8h.html</anchorfile>
      <anchor>3823d8a6013674a00b7cf154ae2788d2</anchor>
      <arglist>(opc)</arglist>
    </member>
    <member kind="define">
      <type>#define</type>
      <name>IS_COND_BRANCH</name>
      <anchorfile>l__schedule_8h.html</anchorfile>
      <anchor>f60b61d9aa08c98c539bc0ee46c99305</anchor>
      <arglist>(opc)</arglist>
    </member>
    <member kind="define">
      <type>#define</type>
      <name>IS_BRANCH</name>
      <anchorfile>l__schedule_8h.html</anchorfile>
      <anchor>729ec4e90dc5212fbe085b2daf20dd48</anchor>
      <arglist>(opc)</arglist>
    </member>
    <member kind="define">
      <type>#define</type>
      <name>IS_CTL</name>
      <anchorfile>l__schedule_8h.html</anchorfile>
      <anchor>915fa5b082d3211a2c8763c0d955c71e</anchor>
      <arglist>(opc)</arglist>
    </member>
    <member kind="define">
      <type>#define</type>
      <name>IS_JSR</name>
      <anchorfile>l__schedule_8h.html</anchorfile>
      <anchor>b1d78684bb00b444ceaf1953f3cfab34</anchor>
      <arglist>(opc)</arglist>
    </member>
    <member kind="define">
      <type>#define</type>
      <name>IS_MEMORY_LOAD</name>
      <anchorfile>l__schedule_8h.html</anchorfile>
      <anchor>b8a45273e2f062695778365babad6deb</anchor>
      <arglist>(opc)</arglist>
    </member>
    <member kind="define">
      <type>#define</type>
      <name>IS_MEMORY_STORE</name>
      <anchorfile>l__schedule_8h.html</anchorfile>
      <anchor>dfc68ee0cdc9005816ee7cd829f9a89e</anchor>
      <arglist>(opc)</arglist>
    </member>
    <member kind="define">
      <type>#define</type>
      <name>IS_MEMORY_OP</name>
      <anchorfile>l__schedule_8h.html</anchorfile>
      <anchor>e325b4d8ae3b1a1e227b6a73e14a4d78</anchor>
      <arglist>(opc)</arglist>
    </member>
    <member kind="define">
      <type>#define</type>
      <name>HAS_DELAY_SLOT</name>
      <anchorfile>l__schedule_8h.html</anchorfile>
      <anchor>3894c07e0b8385285e924410a40a219e</anchor>
      <arglist>(opc)</arglist>
    </member>
    <member kind="define">
      <type>#define</type>
      <name>IS_IGNORE</name>
      <anchorfile>l__schedule_8h.html</anchorfile>
      <anchor>eed734fb96244d42b66fe9a63ca9184e</anchor>
      <arglist>(opc)</arglist>
    </member>
    <member kind="define">
      <type>#define</type>
      <name>IS_EXCEPTING</name>
      <anchorfile>l__schedule_8h.html</anchorfile>
      <anchor>349208a9816a53a3979e5dff18b5e96b</anchor>
      <arglist>(opc)</arglist>
    </member>
    <member kind="define">
      <type>#define</type>
      <name>BASIC_BLOCK</name>
      <anchorfile>l__schedule_8h.html</anchorfile>
      <anchor>2ada784f937925e251cbc0a03eaee1c3</anchor>
      <arglist></arglist>
    </member>
    <member kind="define">
      <type>#define</type>
      <name>RESTRICTED</name>
      <anchorfile>l__schedule_8h.html</anchorfile>
      <anchor>edb0804dfadf0c8d668cab284168494f</anchor>
      <arglist></arglist>
    </member>
    <member kind="define">
      <type>#define</type>
      <name>GENERAL</name>
      <anchorfile>l__schedule_8h.html</anchorfile>
      <anchor>437af5f01218b7f90764a1353dd15289</anchor>
      <arglist></arglist>
    </member>
    <member kind="define">
      <type>#define</type>
      <name>WBS</name>
      <anchorfile>l__schedule_8h.html</anchorfile>
      <anchor>dbd0f0983d6aee1cc17996ba6dd3889d</anchor>
      <arglist></arglist>
    </member>
    <member kind="define">
      <type>#define</type>
      <name>WRITEBACK_SUPPRESSION</name>
      <anchorfile>l__schedule_8h.html</anchorfile>
      <anchor>50d2ac1e0a4ce3cd20af118f81481136</anchor>
      <arglist></arglist>
    </member>
    <member kind="define">
      <type>#define</type>
      <name>SENTINEL</name>
      <anchorfile>l__schedule_8h.html</anchorfile>
      <anchor>f2af475b85aef66019c417d8ded1c435</anchor>
      <arglist></arglist>
    </member>
    <member kind="define">
      <type>#define</type>
      <name>BOOSTING</name>
      <anchorfile>l__schedule_8h.html</anchorfile>
      <anchor>0ef648568d9ad0edf6c82e66fe0e8074</anchor>
      <arglist></arglist>
    </member>
    <member kind="define">
      <type>#define</type>
      <name>MCB</name>
      <anchorfile>l__schedule_8h.html</anchorfile>
      <anchor>a09c29e7b5e67d47ef12162e89405449</anchor>
      <arglist></arglist>
    </member>
    <member kind="define">
      <type>#define</type>
      <name>EXTERN_LOG_FILE</name>
      <anchorfile>l__schedule_8h.html</anchorfile>
      <anchor>8b554a9a0abb5566cebe77675fd21794</anchor>
      <arglist></arglist>
    </member>
    <member kind="define">
      <type>#define</type>
      <name>EXTERN_LOG_FILE_PREPASS</name>
      <anchorfile>l__schedule_8h.html</anchorfile>
      <anchor>635c5f78625a3542169478e9f6e53c03</anchor>
      <arglist></arglist>
    </member>
    <member kind="function">
      <type>int</type>
      <name>Lsched_is_branch</name>
      <anchorfile>l__schedule_8h.html</anchorfile>
      <anchor>b90bfe92560ffd5c0809222feb68344f</anchor>
      <arglist>(int)</arglist>
    </member>
    <member kind="function">
      <type>int</type>
      <name>Lsched_loads_per_cycle</name>
      <anchorfile>l__schedule_8h.html</anchorfile>
      <anchor>8dc9c8930405b68ae031fc5d17082581</anchor>
      <arglist>(void)</arglist>
    </member>
    <member kind="function">
      <type>int</type>
      <name>Lsched_stores_per_cycle</name>
      <anchorfile>l__schedule_8h.html</anchorfile>
      <anchor>15a5f6106c4f92cb7268d905fdb1ebc8</anchor>
      <arglist>(void)</arglist>
    </member>
    <member kind="function">
      <type>int</type>
      <name>Lsched_branches_per_cycle</name>
      <anchorfile>l__schedule_8h.html</anchorfile>
      <anchor>ff965250ced17845e2f5165e881e5776</anchor>
      <arglist>(void)</arglist>
    </member>
    <member kind="function">
      <type>int</type>
      <name>Lsched_issue_time</name>
      <anchorfile>l__schedule_8h.html</anchorfile>
      <anchor>97ce712a4c1677368c240d83e2d779c8</anchor>
      <arglist>(L_Oper *)</arglist>
    </member>
    <member kind="function">
      <type>int</type>
      <name>Lsched_completion_time</name>
      <anchorfile>l__schedule_8h.html</anchorfile>
      <anchor>2891a465013dd9f1cdc04c438be9220b</anchor>
      <arglist>(L_Oper *)</arglist>
    </member>
    <member kind="function">
      <type>int</type>
      <name>Lsched_latency</name>
      <anchorfile>l__schedule_8h.html</anchorfile>
      <anchor>703567119cde60627277b89680e7c56a</anchor>
      <arglist>(L_Oper *)</arglist>
    </member>
    <member kind="function">
      <type>void</type>
      <name>Lsched_prepass_code_scheduling</name>
      <anchorfile>l__schedule_8h.html</anchorfile>
      <anchor>c38e8a63d0a4f9ab39d65a7f83d59a7d</anchor>
      <arglist>(L_Func *)</arglist>
    </member>
    <member kind="function">
      <type>void</type>
      <name>Lsched_postpass_code_scheduling</name>
      <anchorfile>l__schedule_8h.html</anchorfile>
      <anchor>0c1b6380eecbd9730e44bfe81b1621bc</anchor>
      <arglist>(L_Func *)</arglist>
    </member>
    <member kind="function">
      <type>int</type>
      <name>Lsched_update_ctl_delay_sinfo</name>
      <anchorfile>l__schedule_8h.html</anchorfile>
      <anchor>5ba30a128fd9e1edf5e538797007509b</anchor>
      <arglist>(Squeue *, Sched_Info *)</arglist>
    </member>
    <member kind="function">
      <type>void</type>
      <name>Lsched_check_for_fill_candidates</name>
      <anchorfile>l__schedule_8h.html</anchorfile>
      <anchor>703cb754b90b99c53efb631cf3a47444</anchor>
      <arglist>(Squeue *)</arglist>
    </member>
    <member kind="function">
      <type>void</type>
      <name>Lsched_schedule_with_delay_op</name>
      <anchorfile>l__schedule_8h.html</anchorfile>
      <anchor>cda824c560796f56f032cea6a184ad23</anchor>
      <arglist>(L_Cb *, Sched_Info *, int, int, int, int)</arglist>
    </member>
    <member kind="function">
      <type>void</type>
      <name>Lsched_fill_nonsquashing_branches</name>
      <anchorfile>l__schedule_8h.html</anchorfile>
      <anchor>bf84f492134eb3fe47455ba9ef62ffa2</anchor>
      <arglist>(L_Func *)</arglist>
    </member>
    <member kind="function">
      <type>void</type>
      <name>Lsched_fill_squashing_branches</name>
      <anchorfile>l__schedule_8h.html</anchorfile>
      <anchor>2ffaa8687f357a681524ecd46a1f1e50</anchor>
      <arglist>(L_Func *)</arglist>
    </member>
    <member kind="function">
      <type>void</type>
      <name>Lsched_fill_unfilled_branches</name>
      <anchorfile>l__schedule_8h.html</anchorfile>
      <anchor>4693926197cdc3090fa3c5cf9de34cb5</anchor>
      <arglist>(L_Func *)</arglist>
    </member>
    <member kind="function">
      <type>void</type>
      <name>Lsched_init</name>
      <anchorfile>l__schedule_8h.html</anchorfile>
      <anchor>3764ca0089d2ec24309d9baec7d2696e</anchor>
      <arglist>()</arglist>
    </member>
    <member kind="function">
      <type>int</type>
      <name>Lsched_can_schedule_op</name>
      <anchorfile>l__schedule_8h.html</anchorfile>
      <anchor>96359c2354c002d34b4f5804322979dd</anchor>
      <arglist>(L_Cb *, Sched_Info *, int, int, int *, int *, int *)</arglist>
    </member>
    <member kind="function">
      <type>void</type>
      <name>Lsched_schedule_op</name>
      <anchorfile>l__schedule_8h.html</anchorfile>
      <anchor>08764d633a209aeda43dfd64245ec30f</anchor>
      <arglist>(L_Cb *, Sched_Info *, int, int)</arglist>
    </member>
    <member kind="function">
      <type>void</type>
      <name>Lsched_make_ready</name>
      <anchorfile>l__schedule_8h.html</anchorfile>
      <anchor>38a7ad11bd117f1e19d2e5256eb19a61</anchor>
      <arglist>(L_Cb *, int, Sched_Info *, Sched_Info *, int, int, int)</arglist>
    </member>
    <member kind="function">
      <type>void</type>
      <name>L_delete_sched_info</name>
      <anchorfile>l__schedule_8h.html</anchorfile>
      <anchor>fc54ef260623cbb0b6e6c13ab288d747</anchor>
      <arglist>(Sched_Info *)</arglist>
    </member>
    <member kind="function">
      <type>void</type>
      <name>Lsched_add_isl_attr</name>
      <anchorfile>l__schedule_8h.html</anchorfile>
      <anchor>1882c164271a9d0021a970f2735de0b0</anchor>
      <arglist>(L_Func *fn)</arglist>
    </member>
    <member kind="function">
      <type>void</type>
      <name>Lsched_delete_isl_attr</name>
      <anchorfile>l__schedule_8h.html</anchorfile>
      <anchor>5877322a652d30df0a7ad278ee7d26e6</anchor>
      <arglist>(L_Func *fn)</arglist>
    </member>
    <member kind="function">
      <type>void</type>
      <name>wbs_remove_check</name>
      <anchorfile>l__schedule_8h.html</anchorfile>
      <anchor>03ec5813b6f4f103e78129a38f37805b</anchor>
      <arglist>()</arglist>
    </member>
    <member kind="function">
      <type>void</type>
      <name>wbs_insert_check</name>
      <anchorfile>l__schedule_8h.html</anchorfile>
      <anchor>19312afbfe319ab758c890296a576863</anchor>
      <arglist>()</arglist>
    </member>
    <member kind="function">
      <type>void</type>
      <name>wbs_extend_live_range</name>
      <anchorfile>l__schedule_8h.html</anchorfile>
      <anchor>654c3ce3e012eb5f44eb74ac29317cec</anchor>
      <arglist>()</arglist>
    </member>
    <member kind="function">
      <type>void</type>
      <name>Lsched_schedule_block</name>
      <anchorfile>l__schedule_8h.html</anchorfile>
      <anchor>7d9e535e6ceefbc9b8be92d3349d67a0</anchor>
      <arglist>(L_Cb *cb)</arglist>
    </member>
    <member kind="function">
      <type>Sched_Info *</type>
      <name>L_create_sched_info</name>
      <anchorfile>l__schedule_8h.html</anchorfile>
      <anchor>268dd685af46604df42953774a82cca3</anchor>
      <arglist>(L_Oper *oper, int home_block, L_Oper *prev_br, L_Oper *post_br)</arglist>
    </member>
    <member kind="function">
      <type>void</type>
      <name>L_mcb_remove_check</name>
      <anchorfile>l__schedule_8h.html</anchorfile>
      <anchor>86fbe77057384630ecf6336d91ed3fc5</anchor>
      <arglist>(L_Cb *cb, L_Oper *oper, int ready_time)</arglist>
    </member>
    <member kind="function">
      <type>Squeue *</type>
      <name>L_create_queue</name>
      <anchorfile>l__schedule_8h.html</anchorfile>
      <anchor>5d4e806c6e14e8c8fbb00bfe7132373b</anchor>
      <arglist>(char *, int)</arglist>
    </member>
    <member kind="function">
      <type>void</type>
      <name>L_delete_queue</name>
      <anchorfile>l__schedule_8h.html</anchorfile>
      <anchor>bbbeef78440f0b1f5dcef84469fda654</anchor>
      <arglist>(Squeue *queue)</arglist>
    </member>
    <member kind="function">
      <type>void</type>
      <name>L_reset_queue_current</name>
      <anchorfile>l__schedule_8h.html</anchorfile>
      <anchor>820fdc8d61abc9d2c1d9239521ce1dad</anchor>
      <arglist>(Squeue *)</arglist>
    </member>
    <member kind="function">
      <type>Sched_Info *</type>
      <name>L_get_queue_next_entry</name>
      <anchorfile>l__schedule_8h.html</anchorfile>
      <anchor>b9dfd4272b10bda2f8fc386fc01fa49c</anchor>
      <arglist>(Squeue *)</arglist>
    </member>
    <member kind="function">
      <type>Sched_Info *</type>
      <name>L_get_queue_head</name>
      <anchorfile>l__schedule_8h.html</anchorfile>
      <anchor>c5f40457eb3bbc0213d09e6802d910e9</anchor>
      <arglist>(Squeue *)</arglist>
    </member>
    <member kind="function">
      <type>int</type>
      <name>L_get_queue_size</name>
      <anchorfile>l__schedule_8h.html</anchorfile>
      <anchor>7c88d211fbea7f5590420724fc97091c</anchor>
      <arglist>(Squeue *)</arglist>
    </member>
    <member kind="function">
      <type>void</type>
      <name>L_enqueue</name>
      <anchorfile>l__schedule_8h.html</anchorfile>
      <anchor>4946e9588196557d185aee160616ad87</anchor>
      <arglist>(Squeue *, Sched_Info *)</arglist>
    </member>
    <member kind="function">
      <type>void</type>
      <name>L_dequeue</name>
      <anchorfile>l__schedule_8h.html</anchorfile>
      <anchor>c4fccedc1f5ee957f2c5443cc693c0a4</anchor>
      <arglist>(Squeue *, Sched_Info *)</arglist>
    </member>
    <member kind="function">
      <type>void</type>
      <name>L_dequeue_from_all</name>
      <anchorfile>l__schedule_8h.html</anchorfile>
      <anchor>db6c1e54d04aef08e8af1be336961318</anchor>
      <arglist>(Sched_Info *)</arglist>
    </member>
    <member kind="function">
      <type>void</type>
      <name>L_enqueue_min_to_max_1</name>
      <anchorfile>l__schedule_8h.html</anchorfile>
      <anchor>e278f5234f84c9b05911d79d4f35889f</anchor>
      <arglist>(Squeue *, Sched_Info *, float)</arglist>
    </member>
    <member kind="function">
      <type>void</type>
      <name>L_enqueue_max_to_min_1</name>
      <anchorfile>l__schedule_8h.html</anchorfile>
      <anchor>e8b74658cb198f513e583544417d62f8</anchor>
      <arglist>(Squeue *, Sched_Info *, float)</arglist>
    </member>
    <member kind="function">
      <type>void</type>
      <name>L_enqueue_min_to_max_2</name>
      <anchorfile>l__schedule_8h.html</anchorfile>
      <anchor>54d2f953a9ec7bb9960f1a6dc2bfb26a</anchor>
      <arglist>(Squeue *, Sched_Info *, float, float)</arglist>
    </member>
    <member kind="function">
      <type>void</type>
      <name>L_enqueue_max_to_min_2</name>
      <anchorfile>l__schedule_8h.html</anchorfile>
      <anchor>1963c4e3ecf49853a303b4c218258b99</anchor>
      <arglist>(Squeue *, Sched_Info *, float, float)</arglist>
    </member>
    <member kind="function">
      <type>void</type>
      <name>L_enqueue_regpres</name>
      <anchorfile>l__schedule_8h.html</anchorfile>
      <anchor>8227bd45b0f57f5d6f07e6af4a46b1d6</anchor>
      <arglist>(Squeue *queue, Sched_Info *sinfo, float key1, float key2)</arglist>
    </member>
    <member kind="function">
      <type>int</type>
      <name>L_in_queue</name>
      <anchorfile>l__schedule_8h.html</anchorfile>
      <anchor>a5622848600459f67a920a9b2cce7bea</anchor>
      <arglist>(Squeue *, Sched_Info *)</arglist>
    </member>
    <member kind="function">
      <type>void</type>
      <name>Lregpres_init_func</name>
      <anchorfile>l__schedule_8h.html</anchorfile>
      <anchor>526d3b7e511047a384d3d76acca48803</anchor>
      <arglist>(L_Func *)</arglist>
    </member>
    <member kind="function">
      <type>void</type>
      <name>Lregpres_deinit_func</name>
      <anchorfile>l__schedule_8h.html</anchorfile>
      <anchor>9f2d57bd870fd926308781a6c2892c42</anchor>
      <arglist>(L_Func *)</arglist>
    </member>
    <member kind="function">
      <type>void</type>
      <name>Lregpres_init_cb</name>
      <anchorfile>l__schedule_8h.html</anchorfile>
      <anchor>503927a80e2d6aa324f47919932e7b1f</anchor>
      <arglist>(L_Cb *)</arglist>
    </member>
    <member kind="function">
      <type>void</type>
      <name>L_init_ready_queue</name>
      <anchorfile>l__schedule_8h.html</anchorfile>
      <anchor>ce269a612036a7f00eed18771f53b4f4</anchor>
      <arglist>()</arglist>
    </member>
    <member kind="function">
      <type>Sched_Info *</type>
      <name>L_get_next_entry</name>
      <anchorfile>l__schedule_8h.html</anchorfile>
      <anchor>0c02c6c4c315bd64ecf3c512ee700d87</anchor>
      <arglist>()</arglist>
    </member>
    <member kind="function">
      <type>void</type>
      <name>L_insert_entry</name>
      <anchorfile>l__schedule_8h.html</anchorfile>
      <anchor>c7b001641028f429393021b0f92eb846</anchor>
      <arglist>(Sched_Info *)</arglist>
    </member>
    <member kind="function">
      <type>void</type>
      <name>L_update_virt_reg</name>
      <anchorfile>l__schedule_8h.html</anchorfile>
      <anchor>b7013fd5ea6dbb80ebd3e4adc3eaea25</anchor>
      <arglist>(Sched_Info *)</arglist>
    </member>
    <member kind="function">
      <type>double</type>
      <name>L_approx_cb_time</name>
      <anchorfile>l__schedule_8h.html</anchorfile>
      <anchor>a94cf304bcc32ebd982fa08332a51ae5</anchor>
      <arglist>(L_Cb *cb)</arglist>
    </member>
    <member kind="function">
      <type>L_Cb *</type>
      <name>L_copy_all_oper_in_cb</name>
      <anchorfile>l__schedule_8h.html</anchorfile>
      <anchor>d7ea649eb15d34bb940fea7fc01f55cf</anchor>
      <arglist>(L_Func *fn, L_Cb *cb)</arglist>
    </member>
    <member kind="function">
      <type>void</type>
      <name>Lsched_mcb_prepass_schedule_block</name>
      <anchorfile>l__schedule_8h.html</anchorfile>
      <anchor>f6777b96ea9652dc942976a6cd50acd9</anchor>
      <arglist>(L_Func *fn, L_Cb *cb, L_Cb *nomcb_cb)</arglist>
    </member>
    <member kind="function">
      <type>void</type>
      <name>Lsched_mcb_schedule_block</name>
      <anchorfile>l__schedule_8h.html</anchorfile>
      <anchor>87c4a42bf87228adc317fe08e2aa57d1</anchor>
      <arglist>(L_Func *fn, L_Cb *cb, L_Cb *nomcb_cb)</arglist>
    </member>
    <member kind="function">
      <type>int</type>
      <name>Lsched_handle_spilled_branch</name>
      <anchorfile>l__schedule_8h.html</anchorfile>
      <anchor>6de719e078b25892d78ade0a72a72e1f</anchor>
      <arglist>(L_Cb *cb, Sched_Info *sinfo)</arglist>
    </member>
    <member kind="function">
      <type>void</type>
      <name>L_print_schedule</name>
      <anchorfile>l__schedule_8h.html</anchorfile>
      <anchor>436ebb2a3d9d135acb6558e1920d3555</anchor>
      <arglist>(char *F, L_Func *fn)</arglist>
    </member>
    <member kind="variable">
      <type>L_Alloc_Pool *</type>
      <name>Squeue_pool</name>
      <anchorfile>l__schedule_8h.html</anchorfile>
      <anchor>70490b1c3107e1406d0a788c58460d61</anchor>
      <arglist></arglist>
    </member>
    <member kind="variable">
      <type>L_Alloc_Pool *</type>
      <name>Sq_entry_pool</name>
      <anchorfile>l__schedule_8h.html</anchorfile>
      <anchor>905d903f4a74734bcd6535bdc20f5c84</anchor>
      <arglist></arglist>
    </member>
    <member kind="variable">
      <type>Squeue *</type>
      <name>scheduled_queue</name>
      <anchorfile>l__schedule_8h.html</anchorfile>
      <anchor>b45d8d03ce37d7b7cf89b0321c5d3c28</anchor>
      <arglist></arglist>
    </member>
    <member kind="variable">
      <type>Squeue *</type>
      <name>priority_ready_queue</name>
      <anchorfile>l__schedule_8h.html</anchorfile>
      <anchor>7ad0faa72489db11d3885ff9a0f42366</anchor>
      <arglist></arglist>
    </member>
    <member kind="variable">
      <type>Squeue *</type>
      <name>regpres_ready_queue</name>
      <anchorfile>l__schedule_8h.html</anchorfile>
      <anchor>99d40e945682d462016f6899c74be8d9</anchor>
      <arglist></arglist>
    </member>
    <member kind="variable">
      <type>Squeue *</type>
      <name>pending_ready_queue</name>
      <anchorfile>l__schedule_8h.html</anchorfile>
      <anchor>065691cd75db4683c71bba868f90264d</anchor>
      <arglist></arglist>
    </member>
    <member kind="variable">
      <type>Squeue *</type>
      <name>not_ready_queue</name>
      <anchorfile>l__schedule_8h.html</anchorfile>
      <anchor>fe3f0aa3f7900d8acb68f9c302f32a7f</anchor>
      <arglist></arglist>
    </member>
    <member kind="variable">
      <type>CB_REG_INFO</type>
      <name>preg_info</name>
      <anchorfile>l__schedule_8h.html</anchorfile>
      <anchor>0647b4f1b08b4ccefd9a78e4a6ab1b54</anchor>
      <arglist></arglist>
    </member>
    <member kind="variable">
      <type>CB_REG_INFO</type>
      <name>ireg_info</name>
      <anchorfile>l__schedule_8h.html</anchorfile>
      <anchor>abc4d42199592ce82fbf255e14709f17</anchor>
      <arglist></arglist>
    </member>
    <member kind="variable">
      <type>CB_REG_INFO</type>
      <name>freg_info</name>
      <anchorfile>l__schedule_8h.html</anchorfile>
      <anchor>360b283a865b12acd3078199a40caafd</anchor>
      <arglist></arglist>
    </member>
    <member kind="variable">
      <type>CB_REG_INFO</type>
      <name>dreg_info</name>
      <anchorfile>l__schedule_8h.html</anchorfile>
      <anchor>3c2afab1c79a0e20251f42283e7ad216</anchor>
      <arglist></arglist>
    </member>
    <member kind="variable">
      <type>int</type>
      <name>Lsched_use_register_pressure_heuristic</name>
      <anchorfile>l__schedule_8h.html</anchorfile>
      <anchor>dcfe1fc65128905189f3a03d1983154d</anchor>
      <arglist></arglist>
    </member>
    <member kind="variable">
      <type>int</type>
      <name>Lsched_register_pressure_threshhold</name>
      <anchorfile>l__schedule_8h.html</anchorfile>
      <anchor>d6b0405973ba10a6f980c3dcc31abeb5</anchor>
      <arglist></arglist>
    </member>
    <member kind="variable">
      <type>Vreg *</type>
      <name>virt_reg</name>
      <anchorfile>l__schedule_8h.html</anchorfile>
      <anchor>634254654ecb462669c4a093390bfbc3</anchor>
      <arglist></arglist>
    </member>
    <member kind="variable">
      <type>char *</type>
      <name>Lsched_model_name</name>
      <anchorfile>l__schedule_8h.html</anchorfile>
      <anchor>aa648b78c4f34d06e6b1b3ffc5aea7b3</anchor>
      <arglist></arglist>
    </member>
    <member kind="variable">
      <type>int</type>
      <name>Lsched_model</name>
      <anchorfile>l__schedule_8h.html</anchorfile>
      <anchor>b7d554b62471e5d19028965e1c2ccc30</anchor>
      <arglist></arglist>
    </member>
    <member kind="variable">
      <type>int</type>
      <name>Lsched_processor_model</name>
      <anchorfile>l__schedule_8h.html</anchorfile>
      <anchor>4d59bf1a4de63a2857de2b9f53abd7b2</anchor>
      <arglist></arglist>
    </member>
    <member kind="variable">
      <type>int</type>
      <name>Lsched_total_issue_slots</name>
      <anchorfile>l__schedule_8h.html</anchorfile>
      <anchor>7511cf13b18125e863ce2bb076d8f1cd</anchor>
      <arglist></arglist>
    </member>
    <member kind="variable">
      <type>int</type>
      <name>Lsched_debug_prepass_scheduling</name>
      <anchorfile>l__schedule_8h.html</anchorfile>
      <anchor>ab0ccf85b37a82c182ce4edd6814eab9</anchor>
      <arglist></arglist>
    </member>
    <member kind="variable">
      <type>int</type>
      <name>Lsched_debug_postpass_scheduling</name>
      <anchorfile>l__schedule_8h.html</anchorfile>
      <anchor>cb6bd68cf55e01ed524d25aec561a569</anchor>
      <arglist></arglist>
    </member>
    <member kind="variable">
      <type>int</type>
      <name>Lsched_debug_messages</name>
      <anchorfile>l__schedule_8h.html</anchorfile>
      <anchor>69b188aa5e7ffe9ebe9d25ada889bb4f</anchor>
      <arglist></arglist>
    </member>
    <member kind="variable">
      <type>int</type>
      <name>Lsched_do_postpass_scheduling</name>
      <anchorfile>l__schedule_8h.html</anchorfile>
      <anchor>0feea7b69688d91321677afa857b2e9b</anchor>
      <arglist></arglist>
    </member>
    <member kind="variable">
      <type>char *</type>
      <name>Lsched_profile_info_to_use</name>
      <anchorfile>l__schedule_8h.html</anchorfile>
      <anchor>11161fe0b364a73fa794db32524bd926</anchor>
      <arglist></arglist>
    </member>
    <member kind="variable">
      <type>int</type>
      <name>Lsched_use_fan_out</name>
      <anchorfile>l__schedule_8h.html</anchorfile>
      <anchor>dc2b7936a51554fc91cf180edacf0b19</anchor>
      <arglist></arglist>
    </member>
    <member kind="variable">
      <type>int</type>
      <name>Lsched_infinite_issue</name>
      <anchorfile>l__schedule_8h.html</anchorfile>
      <anchor>a41d94228aa003482c3c8da8841c420f</anchor>
      <arglist></arglist>
    </member>
    <member kind="variable">
      <type>int</type>
      <name>Lsched_debug_squashing_branches</name>
      <anchorfile>l__schedule_8h.html</anchorfile>
      <anchor>edb9555f6c00ca8943e8a8a058731e68</anchor>
      <arglist></arglist>
    </member>
    <member kind="variable">
      <type>int</type>
      <name>Lsched_debug_nonsquashing_branches</name>
      <anchorfile>l__schedule_8h.html</anchorfile>
      <anchor>9ee6e81eaa3a35d7523b7d81f38cdbc8</anchor>
      <arglist></arglist>
    </member>
    <member kind="variable">
      <type>int</type>
      <name>Lsched_debug_unfilled_branches</name>
      <anchorfile>l__schedule_8h.html</anchorfile>
      <anchor>4c875e480a2233a014e03f62cab39191</anchor>
      <arglist></arglist>
    </member>
    <member kind="variable">
      <type>int</type>
      <name>Lsched_do_fill_squashing_branches</name>
      <anchorfile>l__schedule_8h.html</anchorfile>
      <anchor>073f10e729943841844c9ac33ab08760</anchor>
      <arglist></arglist>
    </member>
    <member kind="variable">
      <type>int</type>
      <name>Lsched_do_fill_nonsquashing_branches</name>
      <anchorfile>l__schedule_8h.html</anchorfile>
      <anchor>71c03ff0a19ef4532954edc78836a18b</anchor>
      <arglist></arglist>
    </member>
    <member kind="variable">
      <type>int</type>
      <name>Lsched_do_fill_unfilled_branches</name>
      <anchorfile>l__schedule_8h.html</anchorfile>
      <anchor>51b02dd4ff5d452f0832853d61065c2c</anchor>
      <arglist></arglist>
    </member>
    <member kind="variable">
      <type>int</type>
      <name>Lsched_fill_delay_slots</name>
      <anchorfile>l__schedule_8h.html</anchorfile>
      <anchor>6d53c903b4fd077ed1a02626dbebfda0</anchor>
      <arglist></arglist>
    </member>
    <member kind="variable">
      <type>int</type>
      <name>Lsched_include_only_oper_lt_hb</name>
      <anchorfile>l__schedule_8h.html</anchorfile>
      <anchor>20a06bed1bdca8cd03cffb1cbe5b588b</anchor>
      <arglist></arglist>
    </member>
    <member kind="variable">
      <type>int</type>
      <name>Lsched_print_statistics</name>
      <anchorfile>l__schedule_8h.html</anchorfile>
      <anchor>e677cd429292523a696a32d17cc7e389</anchor>
      <arglist></arglist>
    </member>
    <member kind="variable">
      <type>int</type>
      <name>Lsched_print_prepass_statistics</name>
      <anchorfile>l__schedule_8h.html</anchorfile>
      <anchor>181a18821bedca4935435e87b165d3a0</anchor>
      <arglist></arglist>
    </member>
    <member kind="variable">
      <type>int</type>
      <name>Lsched_demote_all_the_way</name>
      <anchorfile>l__schedule_8h.html</anchorfile>
      <anchor>a42d0c45d271917fdbdc1cc4daa18b1d</anchor>
      <arglist></arglist>
    </member>
    <member kind="variable">
      <type>int</type>
      <name>Lsched_do_sentinel_recovery</name>
      <anchorfile>l__schedule_8h.html</anchorfile>
      <anchor>e92a2b503333948c586d0b6907833c94</anchor>
      <arglist></arglist>
    </member>
    <member kind="variable">
      <type>int</type>
      <name>Lsched_prepass</name>
      <anchorfile>l__schedule_8h.html</anchorfile>
      <anchor>fac4677c213f235af5efb6e5fd7cde1b</anchor>
      <arglist></arglist>
    </member>
    <member kind="variable">
      <type>L_Oper *</type>
      <name>Lsched_latest_br</name>
      <anchorfile>l__schedule_8h.html</anchorfile>
      <anchor>bf5ac60970385f7d1582ea7d94884faa</anchor>
      <arglist></arglist>
    </member>
    <member kind="variable">
      <type>int</type>
      <name>Lsched_use_register_pressure_heuristic</name>
      <anchorfile>l__schedule_8h.html</anchorfile>
      <anchor>dcfe1fc65128905189f3a03d1983154d</anchor>
      <arglist></arglist>
    </member>
    <member kind="variable">
      <type>int</type>
      <name>Lsched_register_pressure_threshhold</name>
      <anchorfile>l__schedule_8h.html</anchorfile>
      <anchor>d6b0405973ba10a6f980c3dcc31abeb5</anchor>
      <arglist></arglist>
    </member>
    <member kind="variable">
      <type>int</type>
      <name>Lsched_regpres_heuristic</name>
      <anchorfile>l__schedule_8h.html</anchorfile>
      <anchor>b801097b81716757db39ca58936d99d8</anchor>
      <arglist></arglist>
    </member>
    <member kind="variable">
      <type>int</type>
      <name>Lsched_num_branches_per_cycle</name>
      <anchorfile>l__schedule_8h.html</anchorfile>
      <anchor>42512cec3f5bb5c7b60f1db41f252ec7</anchor>
      <arglist></arglist>
    </member>
    <member kind="variable">
      <type>L_Alloc_Pool *</type>
      <name>Operand_ready_pool</name>
      <anchorfile>l__schedule_8h.html</anchorfile>
      <anchor>5aeea507b43d25bfcbdd09f85aa420ca</anchor>
      <arglist></arglist>
    </member>
    <member kind="variable">
      <type>L_Alloc_Pool *</type>
      <name>Sched_Info_pool</name>
      <anchorfile>l__schedule_8h.html</anchorfile>
      <anchor>373147875f98887da63ee267f1ac4cfe</anchor>
      <arglist></arglist>
    </member>
  </compound>
  <compound kind="file">
    <name>l_statistic.c</name>
    <path>/mnt/hgfs/mkt/Desktop/openimpact/src/Lcode/codegen/Lschedule/</path>
    <filename>l__statistic_8c</filename>
    <includes id="l__schedule_8h" name="l_schedule.h" local="yes" imported="no">l_schedule.h</includes>
    <member kind="function" static="yes">
      <type>static double</type>
      <name>L_taken_weight</name>
      <anchorfile>l__statistic_8c.html</anchorfile>
      <anchor>676e6fd479545e262b123f73c2d6a98f</anchor>
      <arglist>(L_Cb *cb, L_Oper *oper)</arglist>
    </member>
    <member kind="function" static="yes">
      <type>static double</type>
      <name>L_fall_thru_weight</name>
      <anchorfile>l__statistic_8c.html</anchorfile>
      <anchor>f94b2acd8c2d8ee41abe9f9a5a622aee</anchor>
      <arglist>(L_Cb *cb, L_Oper *oper)</arglist>
    </member>
    <member kind="function">
      <type>double</type>
      <name>L_approx_cb_time</name>
      <anchorfile>l__statistic_8c.html</anchorfile>
      <anchor>a94cf304bcc32ebd982fa08332a51ae5</anchor>
      <arglist>(L_Cb *cb)</arglist>
    </member>
    <member kind="function">
      <type>void</type>
      <name>L_print_schedule</name>
      <anchorfile>l__statistic_8c.html</anchorfile>
      <anchor>436ebb2a3d9d135acb6558e1920d3555</anchor>
      <arglist>(char *F, L_Func *fn)</arglist>
    </member>
  </compound>
  <compound kind="file">
    <name>l_wbs.c</name>
    <path>/mnt/hgfs/mkt/Desktop/openimpact/src/Lcode/codegen/Lschedule/</path>
    <filename>l__wbs_8c</filename>
    <includes id="l__schedule_8h" name="l_schedule.h" local="yes" imported="no">l_schedule.h</includes>
    <member kind="function" static="yes">
      <type>static void</type>
      <name>wbs_delete_check</name>
      <anchorfile>l__wbs_8c.html</anchorfile>
      <anchor>88163c730223406d6e9e493ea4f8c4b2</anchor>
      <arglist>(L_Cb *cb, L_Oper *check, int current_time)</arglist>
    </member>
    <member kind="function">
      <type>void</type>
      <name>wbs_insert_check</name>
      <anchorfile>l__wbs_8c.html</anchorfile>
      <anchor>76591b5f94a673030c28abbcc9a2b299</anchor>
      <arglist>(L_Cb *cb)</arglist>
    </member>
    <member kind="function">
      <type>void</type>
      <name>wbs_extend_live_range</name>
      <anchorfile>l__wbs_8c.html</anchorfile>
      <anchor>a97fcb27b1169928383561c2d0c7e9cb</anchor>
      <arglist>(L_Cb *cb)</arglist>
    </member>
    <member kind="function">
      <type>void</type>
      <name>wbs_remove_check</name>
      <anchorfile>l__wbs_8c.html</anchorfile>
      <anchor>f84d1a493e2338334028178dadda7c7f</anchor>
      <arglist>(L_Cb *cb, Sched_Info *branch_sinfo, int current_time)</arglist>
    </member>
  </compound>
  <compound kind="file">
    <name>old_sm.h</name>
    <path>/mnt/hgfs/mkt/Desktop/openimpact/src/Lcode/codegen/Lschedule/</path>
    <filename>old__sm_8h</filename>
    <class kind="struct">OLD_SM_Dep</class>
    <class kind="struct">OLD_SM_Oper</class>
    <class kind="struct">OLD_SM_Cb</class>
    <class kind="struct">OLD_SM_Func</class>
    <class kind="struct">OLD_SM_Stats</class>
    <member kind="function">
      <type>OLD_SM_Cb *</type>
      <name>OLD_SM_new_cb</name>
      <anchorfile>old__sm_8h.html</anchorfile>
      <anchor>d934a193f9acaf1e10a63a04cf3708f1</anchor>
      <arglist>(struct Mdes2 *mdes2, OLD_SM_Func *sm_func, L_Cb *cb)</arglist>
    </member>
    <member kind="function">
      <type>void</type>
      <name>OLD_SM_delete_cb</name>
      <anchorfile>old__sm_8h.html</anchorfile>
      <anchor>bba3da2bbe4aa10245598f765e55f45f</anchor>
      <arglist>(OLD_SM_Cb *sm_cb)</arglist>
    </member>
    <member kind="function">
      <type>int</type>
      <name>OLD_SM_sched_table</name>
      <anchorfile>old__sm_8h.html</anchorfile>
      <anchor>e99a5403da030759f300328c3273fcdf</anchor>
      <arglist>(OLD_SM_Cb *sm_cb, SM_Table *table, int time, unsigned short min_slot, unsigned short max_slot, int commit, Mdes_Stats *stats)</arglist>
    </member>
    <member kind="function">
      <type>void</type>
      <name>OLD_SM_print_option</name>
      <anchorfile>old__sm_8h.html</anchorfile>
      <anchor>76de179c12baa6033e082dd8776988b7</anchor>
      <arglist>(FILE *out, SM_Mdes *sm_mdes, SM_Option *option)</arglist>
    </member>
    <member kind="function">
      <type>void</type>
      <name>OLD_SM_print_map</name>
      <anchorfile>old__sm_8h.html</anchorfile>
      <anchor>a21abe36de254eb267e395ef14dcd133</anchor>
      <arglist>(FILE *out, SM_Mdes *sm_mdes, OLD_SM_Cb *sm_cb)</arglist>
    </member>
    <member kind="function">
      <type>int</type>
      <name>OLD_SM_choose_first_avail_options</name>
      <anchorfile>old__sm_8h.html</anchorfile>
      <anchor>582037c63a20285d19dfe12ed2dea91d</anchor>
      <arglist>(unsigned int *map, SM_Table *table, unsigned short *choices_made, unsigned int min_slot, unsigned int max_slot, Mdes_Stats *stats)</arglist>
    </member>
  </compound>
  <compound kind="file">
    <name>old_sm_rmap.c</name>
    <path>/mnt/hgfs/mkt/Desktop/openimpact/src/Lcode/codegen/Lschedule/</path>
    <filename>old__sm__rmap_8c</filename>
    <includes id="old__sm_8h" name="old_sm.h" local="yes" imported="no">old_sm.h</includes>
    <member kind="define">
      <type>#define</type>
      <name>DEBUG_RMAP_RESIZE</name>
      <anchorfile>old__sm__rmap_8c.html</anchorfile>
      <anchor>c5b47c4c50783b54a9e9efbe9262698f</anchor>
      <arglist></arglist>
    </member>
    <member kind="define">
      <type>#define</type>
      <name>DEBUG_RMAP</name>
      <anchorfile>old__sm__rmap_8c.html</anchorfile>
      <anchor>bb86bec221a00d270ed52f47811465d5</anchor>
      <arglist></arglist>
    </member>
    <member kind="define">
      <type>#define</type>
      <name>TAKE_STATS</name>
      <anchorfile>old__sm__rmap_8c.html</anchorfile>
      <anchor>ef6cd5c57e3470359ed522e3c3c69490</anchor>
      <arglist></arglist>
    </member>
    <member kind="function">
      <type>void</type>
      <name>OLD_SM_commit_choices</name>
      <anchorfile>old__sm__rmap_8c.html</anchorfile>
      <anchor>92eb28f97648d8884686ce4bee6766bb</anchor>
      <arglist>(unsigned int *map, SM_Table *table, unsigned short *choices_made)</arglist>
    </member>
    <member kind="function">
      <type>OLD_SM_Cb *</type>
      <name>OLD_SM_new_cb</name>
      <anchorfile>old__sm__rmap_8c.html</anchorfile>
      <anchor>2a8e20d7b4bcc0db776ae4b6332a4b5d</anchor>
      <arglist>(Mdes2 *mdes2, OLD_SM_Func *sm_func, L_Cb *cb)</arglist>
    </member>
    <member kind="function">
      <type>void</type>
      <name>OLD_SM_delete_cb</name>
      <anchorfile>old__sm__rmap_8c.html</anchorfile>
      <anchor>bba3da2bbe4aa10245598f765e55f45f</anchor>
      <arglist>(OLD_SM_Cb *sm_cb)</arglist>
    </member>
    <member kind="function">
      <type>void</type>
      <name>OLD_SM_check_map</name>
      <anchorfile>old__sm__rmap_8c.html</anchorfile>
      <anchor>18f0c2176ab8e4618d61b0c3bdcb92fc</anchor>
      <arglist>(OLD_SM_Cb *sm_cb)</arglist>
    </member>
    <member kind="function">
      <type>void</type>
      <name>OLD_SM_expand_map_end_offset</name>
      <anchorfile>old__sm__rmap_8c.html</anchorfile>
      <anchor>28de85a9612fd3a1beab750ef2dd4173</anchor>
      <arglist>(OLD_SM_Cb *sm_cb, int max_init_offset)</arglist>
    </member>
    <member kind="function">
      <type>void</type>
      <name>OLD_SM_expand_map_start_offset</name>
      <anchorfile>old__sm__rmap_8c.html</anchorfile>
      <anchor>ae011071b72f337e045a87caf9cd2fda</anchor>
      <arglist>(OLD_SM_Cb *sm_cb, int min_init_offset)</arglist>
    </member>
    <member kind="function">
      <type>void</type>
      <name>OLD_SM_init_for_max_usage</name>
      <anchorfile>old__sm__rmap_8c.html</anchorfile>
      <anchor>0dcd9031ad45b544fdf36bd029baaa00</anchor>
      <arglist>(OLD_SM_Cb *sm_cb, int max_usage_offset)</arglist>
    </member>
    <member kind="function">
      <type>void</type>
      <name>OLD_SM_init_for_min_usage</name>
      <anchorfile>old__sm__rmap_8c.html</anchorfile>
      <anchor>694759c1c614a5d187e5ff67e22e9bb0</anchor>
      <arglist>(OLD_SM_Cb *sm_cb, int min_usage_offset)</arglist>
    </member>
    <member kind="function">
      <type>void</type>
      <name>OLD_SM_create_map</name>
      <anchorfile>old__sm__rmap_8c.html</anchorfile>
      <anchor>ca14bd7d089f3c11cf973af6fd7fe2c0</anchor>
      <arglist>(OLD_SM_Cb *sm_cb, int min_usage_offset, int max_usage_offset)</arglist>
    </member>
    <member kind="function">
      <type>int</type>
      <name>OLD_SM_sched_table</name>
      <anchorfile>old__sm__rmap_8c.html</anchorfile>
      <anchor>e99a5403da030759f300328c3273fcdf</anchor>
      <arglist>(OLD_SM_Cb *sm_cb, SM_Table *table, int time, unsigned short min_slot, unsigned short max_slot, int commit, Mdes_Stats *stats)</arglist>
    </member>
    <member kind="function">
      <type>int</type>
      <name>OLD_SM_choose_first_avail_options</name>
      <anchorfile>old__sm__rmap_8c.html</anchorfile>
      <anchor>582037c63a20285d19dfe12ed2dea91d</anchor>
      <arglist>(unsigned int *map, SM_Table *table, unsigned short *choices_made, unsigned int min_slot, unsigned int max_slot, Mdes_Stats *stats)</arglist>
    </member>
    <member kind="function">
      <type>void</type>
      <name>OLD_SM_print_option</name>
      <anchorfile>old__sm__rmap_8c.html</anchorfile>
      <anchor>76de179c12baa6033e082dd8776988b7</anchor>
      <arglist>(FILE *out, SM_Mdes *sm_mdes, SM_Option *option)</arglist>
    </member>
    <member kind="function">
      <type>void</type>
      <name>OLD_SM_print_map</name>
      <anchorfile>old__sm__rmap_8c.html</anchorfile>
      <anchor>a21abe36de254eb267e395ef14dcd133</anchor>
      <arglist>(FILE *out, SM_Mdes *sm_mdes, OLD_SM_Cb *sm_cb)</arglist>
    </member>
    <member kind="function">
      <type>void</type>
      <name>OLD_SM_print_stats</name>
      <anchorfile>old__sm__rmap_8c.html</anchorfile>
      <anchor>ff6155436294299fda77bc86ed972ae1</anchor>
      <arglist>(FILE *out, Mdes_Stats *stats)</arglist>
    </member>
    <member kind="variable" static="yes">
      <type>static L_Alloc_Pool *</type>
      <name>OLD_SM_Cb_pool</name>
      <anchorfile>old__sm__rmap_8c.html</anchorfile>
      <anchor>d314575f4cc77999cbc830d8a6405daa</anchor>
      <arglist></arglist>
    </member>
  </compound>
  <compound kind="file">
    <name>RU_manager.c</name>
    <path>/mnt/hgfs/mkt/Desktop/openimpact/src/Lcode/codegen/Lschedule/</path>
    <filename>RU__manager_8c</filename>
    <includes id="l__schedule_8h" name="l_schedule.h" local="yes" imported="no">l_schedule.h</includes>
    <includes id="old__sm_8h" name="old_sm.h" local="yes" imported="no">old_sm.h</includes>
    <member kind="define">
      <type>#define</type>
      <name>ERR</name>
      <anchorfile>RU__manager_8c.html</anchorfile>
      <anchor>735563036dced0b7d6cc98f97ea4978b</anchor>
      <arglist></arglist>
    </member>
    <member kind="function">
      <type>void</type>
      <name>RU_set_max_pred</name>
      <anchorfile>RU__manager_8c.html</anchorfile>
      <anchor>87347d084aae40509198e30f58e6e478</anchor>
      <arglist>(int max)</arglist>
    </member>
    <member kind="function">
      <type>int *</type>
      <name>RU_pred_alloc</name>
      <anchorfile>RU__manager_8c.html</anchorfile>
      <anchor>34fc03b647d985f62071736581eeada0</anchor>
      <arglist>()</arglist>
    </member>
    <member kind="function">
      <type>void</type>
      <name>RU_pred_free</name>
      <anchorfile>RU__manager_8c.html</anchorfile>
      <anchor>7fe7b921ed438a87c1cad6bafa91b8c9</anchor>
      <arglist>(int *ptr)</arglist>
    </member>
    <member kind="function">
      <type>void</type>
      <name>RU_pred_print_alloc_data</name>
      <anchorfile>RU__manager_8c.html</anchorfile>
      <anchor>4cac910dd43ec706aad66ab1ca80377e</anchor>
      <arglist>(FILE *F, int verbose)</arglist>
    </member>
    <member kind="function">
      <type>RU_Info *</type>
      <name>RU_info_alloc</name>
      <anchorfile>RU__manager_8c.html</anchorfile>
      <anchor>e25d4f480fef627b2aa2c3c0f0dd7f6a</anchor>
      <arglist>()</arglist>
    </member>
    <member kind="function">
      <type>void</type>
      <name>RU_info_free</name>
      <anchorfile>RU__manager_8c.html</anchorfile>
      <anchor>9fde0ac1f37deeb5971b5e79ab69022f</anchor>
      <arglist>(RU_Info *ptr)</arglist>
    </member>
    <member kind="function">
      <type>void</type>
      <name>RU_info_print_alloc_data</name>
      <anchorfile>RU__manager_8c.html</anchorfile>
      <anchor>e39d950d620a4bfdd3872cc8d122995d</anchor>
      <arglist>(FILE *F, int verbose)</arglist>
    </member>
    <member kind="function">
      <type>RU_Info *</type>
      <name>RU_info_create</name>
      <anchorfile>RU__manager_8c.html</anchorfile>
      <anchor>29762a07dba8b05f2473077e4a60dc59</anchor>
      <arglist>(L_Oper *op, int *pred)</arglist>
    </member>
    <member kind="function">
      <type>void</type>
      <name>RU_info_delete</name>
      <anchorfile>RU__manager_8c.html</anchorfile>
      <anchor>ae07a2cb1d5ab3c87d9bc65533e6618b</anchor>
      <arglist>(RU_Info *info)</arglist>
    </member>
    <member kind="function">
      <type>RU_Node *</type>
      <name>RU_node_alloc</name>
      <anchorfile>RU__manager_8c.html</anchorfile>
      <anchor>a889b1b89b178b55414274fe9e308863</anchor>
      <arglist>()</arglist>
    </member>
    <member kind="function">
      <type>void</type>
      <name>RU_node_free</name>
      <anchorfile>RU__manager_8c.html</anchorfile>
      <anchor>3b6792cb608faebaff731d21aaf3280d</anchor>
      <arglist>(RU_Node *ptr)</arglist>
    </member>
    <member kind="function">
      <type>void</type>
      <name>RU_node_print_alloc_data</name>
      <anchorfile>RU__manager_8c.html</anchorfile>
      <anchor>f2144b0c30c68942e20c4d545d53b3af</anchor>
      <arglist>(FILE *F, int verbose)</arglist>
    </member>
    <member kind="function">
      <type>RU_Node *</type>
      <name>RU_node_create</name>
      <anchorfile>RU__manager_8c.html</anchorfile>
      <anchor>a315764809eb265c289f574a6e007dea</anchor>
      <arglist>(RU_Info *info)</arglist>
    </member>
    <member kind="function">
      <type>void</type>
      <name>RU_node_delete</name>
      <anchorfile>RU__manager_8c.html</anchorfile>
      <anchor>c88c4d3a796e4c8b83832f18e219e084</anchor>
      <arglist>(RU_Map *map, RU_Node *node)</arglist>
    </member>
    <member kind="function">
      <type>void</type>
      <name>RU_node_delete_all</name>
      <anchorfile>RU__manager_8c.html</anchorfile>
      <anchor>8f707464bf5da98622a5bad82a0f42ae</anchor>
      <arglist>(RU_Node *list)</arglist>
    </member>
    <member kind="function">
      <type>void</type>
      <name>RU_node_insert_before</name>
      <anchorfile>RU__manager_8c.html</anchorfile>
      <anchor>6df41a2223c79da7639cc70226e5a44f</anchor>
      <arglist>(RU_Map *map, RU_Node *before_node, RU_Node *node)</arglist>
    </member>
    <member kind="function">
      <type>void</type>
      <name>RU_node_insert_after</name>
      <anchorfile>RU__manager_8c.html</anchorfile>
      <anchor>24938ed1acf23555ac3375874f2dc36e</anchor>
      <arglist>(RU_Map *map, RU_Node *after_node, RU_Node *node)</arglist>
    </member>
    <member kind="function">
      <type>RU_Node *</type>
      <name>RU_node_find</name>
      <anchorfile>RU__manager_8c.html</anchorfile>
      <anchor>8bc3a24cb101b8e888e5ca4cdcdd46cd</anchor>
      <arglist>(RU_Node *list, RU_Info *ru_info, Mdes_Rused *rused)</arglist>
    </member>
    <member kind="function">
      <type>void</type>
      <name>RU_map_create</name>
      <anchorfile>RU__manager_8c.html</anchorfile>
      <anchor>8ee2628e7e9d01b3fd92ba1863a997c6</anchor>
      <arglist>(int length)</arglist>
    </member>
    <member kind="function">
      <type>void</type>
      <name>RU_map_realloc</name>
      <anchorfile>RU__manager_8c.html</anchorfile>
      <anchor>8d236d81b60c9e8059d44b25b79f0d7b</anchor>
      <arglist>()</arglist>
    </member>
    <member kind="function">
      <type>void</type>
      <name>RU_map_init</name>
      <anchorfile>RU__manager_8c.html</anchorfile>
      <anchor>9fd19f8a47592bd1bf712ea4aa1ac59c</anchor>
      <arglist>(int mode, int cycles)</arglist>
    </member>
    <member kind="function">
      <type>void</type>
      <name>RU_map_delete</name>
      <anchorfile>RU__manager_8c.html</anchorfile>
      <anchor>d1c9b9ac585b8b3b21b3333341b3ea64</anchor>
      <arglist>()</arglist>
    </member>
    <member kind="function">
      <type>int</type>
      <name>RU_number_of_alts</name>
      <anchorfile>RU__manager_8c.html</anchorfile>
      <anchor>2bb3c099a39bca63ac454c2d8bf390c3</anchor>
      <arglist>(RU_Info *ru_info, Mdes_Info *mdes_info, int flags)</arglist>
    </member>
    <member kind="function" static="yes">
      <type>static int</type>
      <name>RU_find_resource_index</name>
      <anchorfile>RU__manager_8c.html</anchorfile>
      <anchor>eb113fdbde35088a445fb114cb393d4a</anchor>
      <arglist>(Mdes_Rmask *mask)</arglist>
    </member>
    <member kind="function" static="yes">
      <type>static int</type>
      <name>RU_find_max</name>
      <anchorfile>RU__manager_8c.html</anchorfile>
      <anchor>865b549327e21580cf97523f09713419</anchor>
      <arglist>(int *ru_count)</arglist>
    </member>
    <member kind="function">
      <type>void</type>
      <name>RU_update_usage_count</name>
      <anchorfile>RU__manager_8c.html</anchorfile>
      <anchor>c263a00da77bf491bc9a294b31b90f22</anchor>
      <arglist>(RU_Info *ru_info, Mdes_Info *mdes_info, int *ru_count, int flags)</arglist>
    </member>
    <member kind="function">
      <type>int</type>
      <name>RU_alt_flags_compatible</name>
      <anchorfile>RU__manager_8c.html</anchorfile>
      <anchor>279cb324520a5ca67d29a98a6e04fa75</anchor>
      <arglist>(RU_Info *ru_info, Mdes_Alt *alt, int flags)</arglist>
    </member>
    <member kind="function">
      <type>int</type>
      <name>RU_can_place</name>
      <anchorfile>RU__manager_8c.html</anchorfile>
      <anchor>559445243def88381c49a1d653f25e77</anchor>
      <arglist>(int time, Mdes_Rused *rused, int option_num)</arglist>
    </member>
    <member kind="function">
      <type>void</type>
      <name>RU_place</name>
      <anchorfile>RU__manager_8c.html</anchorfile>
      <anchor>6da67e7a294620fb20f83f07d1ba8391</anchor>
      <arglist>(int time, Mdes_Rused *rused, int option_num, RU_Info *ru_info)</arglist>
    </member>
    <member kind="function">
      <type>void</type>
      <name>RU_unplace</name>
      <anchorfile>RU__manager_8c.html</anchorfile>
      <anchor>a90890a46cfcd6cc9ef17c0726c5d39f</anchor>
      <arglist>(int time, Mdes_Rused *rused, RU_Info *ru_info)</arglist>
    </member>
    <member kind="function">
      <type>int</type>
      <name>RU_schedule_op</name>
      <anchorfile>RU__manager_8c.html</anchorfile>
      <anchor>fa44bbc4a93b2ec9886306943a1ceba6</anchor>
      <arglist>(RU_Info *ru_info, Mdes_Info *mdes_info, int *operand_ready_times, int issue_time, int earliest_slot, int latest_slot, int flags)</arglist>
    </member>
    <member kind="function">
      <type>int</type>
      <name>RU_schedule_op_reverse</name>
      <anchorfile>RU__manager_8c.html</anchorfile>
      <anchor>4041d3ebf720b70c9b966220065b11ce</anchor>
      <arglist>(RU_Info *ru_info, Mdes_Info *mdes_info, int *operand_ready_times, int issue_time, int earliest_slot, int latest_slot, int flags)</arglist>
    </member>
    <member kind="function">
      <type>int</type>
      <name>RU_can_schedule_op</name>
      <anchorfile>RU__manager_8c.html</anchorfile>
      <anchor>2a33fab35682d96de7b2bf14a3d3ce51</anchor>
      <arglist>(RU_Info *ru_info, Mdes_Info *mdes_info, int *operand_ready_times, int issue_time, int earliest_slot, int latest_slot, int flags)</arglist>
    </member>
    <member kind="function">
      <type>int</type>
      <name>RU_schedule_op_at</name>
      <anchorfile>RU__manager_8c.html</anchorfile>
      <anchor>450f3855ffb14ac781f78eda9ca33ed0</anchor>
      <arglist>(RU_Info *ru_info, Mdes_Info *mdes_info, int *operand_ready_times, int issue_time, int slot, int flags)</arglist>
    </member>
    <member kind="function">
      <type>void</type>
      <name>RU_unschedule_op</name>
      <anchorfile>RU__manager_8c.html</anchorfile>
      <anchor>7c5e9e0c48f57d8752d3a958ccba3022</anchor>
      <arglist>(RU_Info *ru_info)</arglist>
    </member>
    <member kind="variable">
      <type>int</type>
      <name>RU_max_pred</name>
      <anchorfile>RU__manager_8c.html</anchorfile>
      <anchor>b7005c442d3e96f4e308bc03d86a5e6f</anchor>
      <arglist></arglist>
    </member>
    <member kind="variable">
      <type>int</type>
      <name>RU_pred_alloc_target_size</name>
      <anchorfile>RU__manager_8c.html</anchorfile>
      <anchor>b3cf7ccf4896943986e5d5b41a522f41</anchor>
      <arglist></arglist>
    </member>
    <member kind="variable" static="yes">
      <type>static RU_Alloc_Data</type>
      <name>RU_pred_alloc_data</name>
      <anchorfile>RU__manager_8c.html</anchorfile>
      <anchor>6eaa9be91382d71648e535c178fe054b</anchor>
      <arglist></arglist>
    </member>
    <member kind="variable">
      <type>int</type>
      <name>RU_info_alloc_target_size</name>
      <anchorfile>RU__manager_8c.html</anchorfile>
      <anchor>cc700ea8ef1b28e647eb33ac7847efb5</anchor>
      <arglist></arglist>
    </member>
    <member kind="variable" static="yes">
      <type>static RU_Alloc_Data</type>
      <name>RU_info_alloc_data</name>
      <anchorfile>RU__manager_8c.html</anchorfile>
      <anchor>8e82045ba167ef0c11a19853d637e585</anchor>
      <arglist></arglist>
    </member>
    <member kind="variable">
      <type>int</type>
      <name>RU_node_alloc_target_size</name>
      <anchorfile>RU__manager_8c.html</anchorfile>
      <anchor>1914198ed1e4792f3b272b0388cca222</anchor>
      <arglist></arglist>
    </member>
    <member kind="variable" static="yes">
      <type>static RU_Alloc_Data</type>
      <name>RU_node_alloc_data</name>
      <anchorfile>RU__manager_8c.html</anchorfile>
      <anchor>2304aff567f58368fbccf2420c81d705</anchor>
      <arglist></arglist>
    </member>
    <member kind="variable">
      <type>RU_Map *</type>
      <name>RU_map</name>
      <anchorfile>RU__manager_8c.html</anchorfile>
      <anchor>1ec86ac6a189f5faa72bf350572c35e1</anchor>
      <arglist></arglist>
    </member>
    <member kind="variable">
      <type>int</type>
      <name>RU_map_length</name>
      <anchorfile>RU__manager_8c.html</anchorfile>
      <anchor>f866276404030638648a12e4eb39e048</anchor>
      <arglist></arglist>
    </member>
    <member kind="variable">
      <type>int</type>
      <name>RU_map_mode</name>
      <anchorfile>RU__manager_8c.html</anchorfile>
      <anchor>ab284499f85b8370428d80e89545a59a</anchor>
      <arglist></arglist>
    </member>
    <member kind="variable">
      <type>int</type>
      <name>RU_map_cycles</name>
      <anchorfile>RU__manager_8c.html</anchorfile>
      <anchor>5b28f480dd80edd9d86edc9bfd31ce2b</anchor>
      <arglist></arglist>
    </member>
    <member kind="variable">
      <type>int *</type>
      <name>RU_mask</name>
      <anchorfile>RU__manager_8c.html</anchorfile>
      <anchor>82ca50c94ed1893421f461fb9d7cd40e</anchor>
      <arglist></arglist>
    </member>
    <member kind="variable">
      <type>int</type>
      <name>RU_mask_width</name>
      <anchorfile>RU__manager_8c.html</anchorfile>
      <anchor>056f20a9e7273cb15678a35197ead400</anchor>
      <arglist></arglist>
    </member>
    <member kind="variable">
      <type>OLD_SM_Cb *</type>
      <name>RU_sm_cb</name>
      <anchorfile>RU__manager_8c.html</anchorfile>
      <anchor>280f83ab226ce658de845ba6242dab13</anchor>
      <arglist></arglist>
    </member>
  </compound>
  <compound kind="file">
    <name>RU_manager.h</name>
    <path>/mnt/hgfs/mkt/Desktop/openimpact/src/Lcode/codegen/Lschedule/</path>
    <filename>RU__manager_8h</filename>
    <class kind="struct">_RU_Alloc_Header</class>
    <class kind="struct">_RU_Alloc_Data</class>
    <class kind="struct">_RU_Info</class>
    <class kind="struct">_RU_Node</class>
    <class kind="struct">_RU_Map</class>
    <member kind="define">
      <type>#define</type>
      <name>RU_MAP_DEFAULT_SIZE</name>
      <anchorfile>RU__manager_8h.html</anchorfile>
      <anchor>92bab7a3c8cef64380203a8fde359883</anchor>
      <arglist></arglist>
    </member>
    <member kind="define">
      <type>#define</type>
      <name>RU_MODE_ACYCLIC</name>
      <anchorfile>RU__manager_8h.html</anchorfile>
      <anchor>6877026a9baa7740de052ee6dcb9bdb5</anchor>
      <arglist></arglist>
    </member>
    <member kind="define">
      <type>#define</type>
      <name>RU_MODE_CYCLIC</name>
      <anchorfile>RU__manager_8h.html</anchorfile>
      <anchor>e7963acf93fd18b28d3b2fd33a251f9f</anchor>
      <arglist></arglist>
    </member>
    <member kind="define">
      <type>#define</type>
      <name>RU_SELECTED_ALT</name>
      <anchorfile>RU__manager_8h.html</anchorfile>
      <anchor>6179092fd9ae50a6dced97a586d9d4ee</anchor>
      <arglist>(info)</arglist>
    </member>
    <member kind="define">
      <type>#define</type>
      <name>RU_SELECTED_ALT_ID</name>
      <anchorfile>RU__manager_8h.html</anchorfile>
      <anchor>2dd1e8e937302f3ae977f35fd78b53e7</anchor>
      <arglist>(info)</arglist>
    </member>
    <member kind="typedef">
      <type>_RU_Alloc_Header</type>
      <name>RU_Alloc_Header</name>
      <anchorfile>RU__manager_8h.html</anchorfile>
      <anchor>d54fb6ddaf8dca24010220cd8b35fa76</anchor>
      <arglist></arglist>
    </member>
    <member kind="typedef">
      <type>_RU_Alloc_Data</type>
      <name>RU_Alloc_Data</name>
      <anchorfile>RU__manager_8h.html</anchorfile>
      <anchor>7651c76e9739872d39b42a582d564b67</anchor>
      <arglist></arglist>
    </member>
    <member kind="typedef">
      <type>_RU_Info</type>
      <name>RU_Info</name>
      <anchorfile>RU__manager_8h.html</anchorfile>
      <anchor>fdd3a5337864c0ce9db4a5b2382ff64a</anchor>
      <arglist></arglist>
    </member>
    <member kind="typedef">
      <type>_RU_Node</type>
      <name>RU_Node</name>
      <anchorfile>RU__manager_8h.html</anchorfile>
      <anchor>15d79281e283bdb6b53b2aa50bcb377a</anchor>
      <arglist></arglist>
    </member>
    <member kind="typedef">
      <type>_RU_Map</type>
      <name>RU_Map</name>
      <anchorfile>RU__manager_8h.html</anchorfile>
      <anchor>4e4d18b14eb7053201dc51ff90477ec1</anchor>
      <arglist></arglist>
    </member>
    <member kind="function">
      <type>void</type>
      <name>RU_set_max_pred</name>
      <anchorfile>RU__manager_8h.html</anchorfile>
      <anchor>d3869bd24daa61c7855389ad6bb52e9a</anchor>
      <arglist>(int)</arglist>
    </member>
    <member kind="function">
      <type>int *</type>
      <name>RU_pred_alloc</name>
      <anchorfile>RU__manager_8h.html</anchorfile>
      <anchor>34fc03b647d985f62071736581eeada0</anchor>
      <arglist>()</arglist>
    </member>
    <member kind="function">
      <type>void</type>
      <name>RU_pred_free</name>
      <anchorfile>RU__manager_8h.html</anchorfile>
      <anchor>c1b4744f4c28256aeb105fe6936f5e24</anchor>
      <arglist>(int *)</arglist>
    </member>
    <member kind="function">
      <type>void</type>
      <name>RU_pred_print_alloc_data</name>
      <anchorfile>RU__manager_8h.html</anchorfile>
      <anchor>0c23eb2d3930233b1d9a4b62b9454f55</anchor>
      <arglist>(FILE *, int)</arglist>
    </member>
    <member kind="function">
      <type>RU_Info *</type>
      <name>RU_info_alloc</name>
      <anchorfile>RU__manager_8h.html</anchorfile>
      <anchor>e25d4f480fef627b2aa2c3c0f0dd7f6a</anchor>
      <arglist>()</arglist>
    </member>
    <member kind="function">
      <type>void</type>
      <name>RU_info_free</name>
      <anchorfile>RU__manager_8h.html</anchorfile>
      <anchor>ba967b537368eef6402320c889fa18f6</anchor>
      <arglist>(RU_Info *)</arglist>
    </member>
    <member kind="function">
      <type>void</type>
      <name>RU_info_print_alloc_data</name>
      <anchorfile>RU__manager_8h.html</anchorfile>
      <anchor>ffb4e8c5bb93b902e6a9c40ec3056e4f</anchor>
      <arglist>(FILE *, int)</arglist>
    </member>
    <member kind="function">
      <type>RU_Info *</type>
      <name>RU_info_create</name>
      <anchorfile>RU__manager_8h.html</anchorfile>
      <anchor>32cacbeadb930b21ae8c8ba7e8c09024</anchor>
      <arglist>(L_Oper *, int *)</arglist>
    </member>
    <member kind="function">
      <type>void</type>
      <name>RU_info_delete</name>
      <anchorfile>RU__manager_8h.html</anchorfile>
      <anchor>93d939f38cbf8214dc9d6ae068f27e74</anchor>
      <arglist>(RU_Info *)</arglist>
    </member>
    <member kind="function">
      <type>RU_Node *</type>
      <name>RU_node_alloc</name>
      <anchorfile>RU__manager_8h.html</anchorfile>
      <anchor>a889b1b89b178b55414274fe9e308863</anchor>
      <arglist>()</arglist>
    </member>
    <member kind="function">
      <type>void</type>
      <name>RU_node_free</name>
      <anchorfile>RU__manager_8h.html</anchorfile>
      <anchor>94dd726cb909a5be4ce976dd05f2608a</anchor>
      <arglist>(RU_Node *)</arglist>
    </member>
    <member kind="function">
      <type>void</type>
      <name>RU_node_print_alloc_data</name>
      <anchorfile>RU__manager_8h.html</anchorfile>
      <anchor>c7baefae90da3f1ca772414f485eb84b</anchor>
      <arglist>(FILE *, int)</arglist>
    </member>
    <member kind="function">
      <type>RU_Node *</type>
      <name>RU_node_create</name>
      <anchorfile>RU__manager_8h.html</anchorfile>
      <anchor>62603687ae727a2092479d0611311730</anchor>
      <arglist>(RU_Info *)</arglist>
    </member>
    <member kind="function">
      <type>void</type>
      <name>RU_node_delete</name>
      <anchorfile>RU__manager_8h.html</anchorfile>
      <anchor>44643b97f111225d5770ad8d6af51fd9</anchor>
      <arglist>(RU_Map *, RU_Node *)</arglist>
    </member>
    <member kind="function">
      <type>void</type>
      <name>RU_node_delete_all</name>
      <anchorfile>RU__manager_8h.html</anchorfile>
      <anchor>6132af9e1b1c68ce857eb75c65468d3a</anchor>
      <arglist>(RU_Node *)</arglist>
    </member>
    <member kind="function">
      <type>void</type>
      <name>RU_node_insert_before</name>
      <anchorfile>RU__manager_8h.html</anchorfile>
      <anchor>0708a98451689e5e0b8aed7b577d987d</anchor>
      <arglist>(RU_Map *, RU_Node *, RU_Node *)</arglist>
    </member>
    <member kind="function">
      <type>void</type>
      <name>RU_node_insert_after</name>
      <anchorfile>RU__manager_8h.html</anchorfile>
      <anchor>fe34e62f6f57acd366429acf665dbd39</anchor>
      <arglist>(RU_Map *, RU_Node *, RU_Node *)</arglist>
    </member>
    <member kind="function">
      <type>RU_Node *</type>
      <name>RU_node_find</name>
      <anchorfile>RU__manager_8h.html</anchorfile>
      <anchor>907c314f12da8630b0ee860342810722</anchor>
      <arglist>(RU_Node *, RU_Info *, Mdes_Rused *)</arglist>
    </member>
    <member kind="function">
      <type>void</type>
      <name>RU_map_create</name>
      <anchorfile>RU__manager_8h.html</anchorfile>
      <anchor>107060b82b808a89e050dfd1e48cfaf5</anchor>
      <arglist>(int)</arglist>
    </member>
    <member kind="function">
      <type>void</type>
      <name>RU_map_realloc</name>
      <anchorfile>RU__manager_8h.html</anchorfile>
      <anchor>8d236d81b60c9e8059d44b25b79f0d7b</anchor>
      <arglist>()</arglist>
    </member>
    <member kind="function">
      <type>void</type>
      <name>RU_map_init</name>
      <anchorfile>RU__manager_8h.html</anchorfile>
      <anchor>608a308dfcee869eb90db3f407aa9677</anchor>
      <arglist>(int, int)</arglist>
    </member>
    <member kind="function">
      <type>void</type>
      <name>RU_map_delete</name>
      <anchorfile>RU__manager_8h.html</anchorfile>
      <anchor>d1c9b9ac585b8b3b21b3333341b3ea64</anchor>
      <arglist>()</arglist>
    </member>
    <member kind="function">
      <type>int</type>
      <name>RU_number_of_alts</name>
      <anchorfile>RU__manager_8h.html</anchorfile>
      <anchor>147238fe2f1f61c3f177c0305c46e3ac</anchor>
      <arglist>(RU_Info *, Mdes_Info *, int)</arglist>
    </member>
    <member kind="function">
      <type>void</type>
      <name>RU_update_usage_count</name>
      <anchorfile>RU__manager_8h.html</anchorfile>
      <anchor>7d710d79c094efeca8b2494f51f14175</anchor>
      <arglist>(RU_Info *, Mdes_Info *, int *, int)</arglist>
    </member>
    <member kind="function">
      <type>int</type>
      <name>RU_alt_flags_compatible</name>
      <anchorfile>RU__manager_8h.html</anchorfile>
      <anchor>ed1a4fe69a3b5ca6c8eb8186a56617d8</anchor>
      <arglist>(RU_Info *, Mdes_Alt *, int)</arglist>
    </member>
    <member kind="function">
      <type>int</type>
      <name>RU_can_place</name>
      <anchorfile>RU__manager_8h.html</anchorfile>
      <anchor>5d59b5179dc1205ab512ddaf63e14718</anchor>
      <arglist>(int, Mdes_Rused *, int)</arglist>
    </member>
    <member kind="function">
      <type>void</type>
      <name>RU_place</name>
      <anchorfile>RU__manager_8h.html</anchorfile>
      <anchor>f0190ae26dbcaaa382458120fbe5a2fc</anchor>
      <arglist>(int, Mdes_Rused *, int, RU_Info *)</arglist>
    </member>
    <member kind="function">
      <type>void</type>
      <name>RU_unplace</name>
      <anchorfile>RU__manager_8h.html</anchorfile>
      <anchor>57a602b677e350820ecfaff8b30736be</anchor>
      <arglist>(int, Mdes_Rused *, RU_Info *)</arglist>
    </member>
    <member kind="function">
      <type>int</type>
      <name>RU_schedule_op</name>
      <anchorfile>RU__manager_8h.html</anchorfile>
      <anchor>ad3ff2c23e590a0892548b0ba3975365</anchor>
      <arglist>(RU_Info *, Mdes_Info *, int *, int, int, int, int)</arglist>
    </member>
    <member kind="function">
      <type>int</type>
      <name>RU_schedule_op_reverse</name>
      <anchorfile>RU__manager_8h.html</anchorfile>
      <anchor>3d57bf9df20e52ded3641f0a91b51f6d</anchor>
      <arglist>(RU_Info *, Mdes_Info *, int *, int, int, int, int)</arglist>
    </member>
    <member kind="function">
      <type>int</type>
      <name>RU_can_schedule_op</name>
      <anchorfile>RU__manager_8h.html</anchorfile>
      <anchor>ef4276ddd21694a495c73ed3cb2f8b0e</anchor>
      <arglist>(RU_Info *, Mdes_Info *, int *, int, int, int, int)</arglist>
    </member>
    <member kind="function">
      <type>int</type>
      <name>RU_schedule_op_at</name>
      <anchorfile>RU__manager_8h.html</anchorfile>
      <anchor>0ca2a20b06bb4d5c52bcd36ecca6336b</anchor>
      <arglist>(RU_Info *, Mdes_Info *, int *, int, int, int)</arglist>
    </member>
    <member kind="function">
      <type>void</type>
      <name>RU_unschedule_op</name>
      <anchorfile>RU__manager_8h.html</anchorfile>
      <anchor>34cd3c8b997e2f6edd2d3e56b6ba4a7d</anchor>
      <arglist>(RU_Info *)</arglist>
    </member>
    <member kind="variable">
      <type>RU_Map *</type>
      <name>RU_map</name>
      <anchorfile>RU__manager_8h.html</anchorfile>
      <anchor>1ec86ac6a189f5faa72bf350572c35e1</anchor>
      <arglist></arglist>
    </member>
    <member kind="variable">
      <type>int</type>
      <name>RU_map_length</name>
      <anchorfile>RU__manager_8h.html</anchorfile>
      <anchor>f866276404030638648a12e4eb39e048</anchor>
      <arglist></arglist>
    </member>
    <member kind="variable">
      <type>int</type>
      <name>RU_map_mode</name>
      <anchorfile>RU__manager_8h.html</anchorfile>
      <anchor>ab284499f85b8370428d80e89545a59a</anchor>
      <arglist></arglist>
    </member>
    <member kind="variable">
      <type>int</type>
      <name>RU_map_cycles</name>
      <anchorfile>RU__manager_8h.html</anchorfile>
      <anchor>5b28f480dd80edd9d86edc9bfd31ce2b</anchor>
      <arglist></arglist>
    </member>
    <member kind="variable">
      <type>int *</type>
      <name>RU_mask</name>
      <anchorfile>RU__manager_8h.html</anchorfile>
      <anchor>82ca50c94ed1893421f461fb9d7cd40e</anchor>
      <arglist></arglist>
    </member>
    <member kind="variable">
      <type>int</type>
      <name>RU_mask_width</name>
      <anchorfile>RU__manager_8h.html</anchorfile>
      <anchor>056f20a9e7273cb15678a35197ead400</anchor>
      <arglist></arglist>
    </member>
    <member kind="variable">
      <type>int</type>
      <name>RU_max_pred</name>
      <anchorfile>RU__manager_8h.html</anchorfile>
      <anchor>b7005c442d3e96f4e308bc03d86a5e6f</anchor>
      <arglist></arglist>
    </member>
  </compound>
  <compound kind="struct">
    <name>_RU_Alloc_Data</name>
    <filename>struct__RU__Alloc__Data.html</filename>
    <member kind="variable">
      <type>RU_Alloc_Header *</type>
      <name>head</name>
      <anchorfile>struct__RU__Alloc__Data.html</anchorfile>
      <anchor>81f7abdcd147f40787bd09e28f77ce4a</anchor>
      <arglist></arglist>
    </member>
    <member kind="variable">
      <type>int</type>
      <name>allocated</name>
      <anchorfile>struct__RU__Alloc__Data.html</anchorfile>
      <anchor>e35d390e52eb5e7b1f6840706f75812b</anchor>
      <arglist></arglist>
    </member>
    <member kind="variable">
      <type>int</type>
      <name>free</name>
      <anchorfile>struct__RU__Alloc__Data.html</anchorfile>
      <anchor>1d4c5a9525653b224315236d1f5bcdf1</anchor>
      <arglist></arglist>
    </member>
  </compound>
  <compound kind="struct">
    <name>_RU_Alloc_Header</name>
    <filename>struct__RU__Alloc__Header.html</filename>
    <member kind="variable">
      <type>_RU_Alloc_Header *</type>
      <name>next</name>
      <anchorfile>struct__RU__Alloc__Header.html</anchorfile>
      <anchor>b434a041f64b181649c1db5dcd3fa490</anchor>
      <arglist></arglist>
    </member>
  </compound>
  <compound kind="struct">
    <name>_RU_Info</name>
    <filename>struct__RU__Info.html</filename>
    <member kind="variable">
      <type>L_Oper *</type>
      <name>op</name>
      <anchorfile>struct__RU__Info.html</anchorfile>
      <anchor>0fbe3df378ce5b4a285c286f2f9186cc</anchor>
      <arglist></arglist>
    </member>
    <member kind="variable">
      <type>int</type>
      <name>proc_opc</name>
      <anchorfile>struct__RU__Info.html</anchorfile>
      <anchor>ed052fe9a0e2efc69743c07859b5d811</anchor>
      <arglist></arglist>
    </member>
    <member kind="variable">
      <type>int *</type>
      <name>pred</name>
      <anchorfile>struct__RU__Info.html</anchorfile>
      <anchor>e6963e947427f45cfef9c739a47fdfe0</anchor>
      <arglist></arglist>
    </member>
    <member kind="variable">
      <type>Mdes_Alt *</type>
      <name>selected_alt</name>
      <anchorfile>struct__RU__Info.html</anchorfile>
      <anchor>0832bb8b97ce3d88bce7e843341aa41c</anchor>
      <arglist></arglist>
    </member>
    <member kind="variable">
      <type>int</type>
      <name>issue_time</name>
      <anchorfile>struct__RU__Info.html</anchorfile>
      <anchor>ed9ba01a26407d816a111fa87bf3774f</anchor>
      <arglist></arglist>
    </member>
    <member kind="variable">
      <type>int</type>
      <name>slot_used</name>
      <anchorfile>struct__RU__Info.html</anchorfile>
      <anchor>1da33123eb56a1da68de8071a75371ef</anchor>
      <arglist></arglist>
    </member>
  </compound>
  <compound kind="struct">
    <name>_RU_Map</name>
    <filename>struct__RU__Map.html</filename>
    <member kind="variable">
      <type>Mdes_Rmask</type>
      <name>mask</name>
      <anchorfile>struct__RU__Map.html</anchorfile>
      <anchor>6fb02fdb5d39ca9700fcef3b8d852cc2</anchor>
      <arglist></arglist>
    </member>
    <member kind="variable">
      <type>RU_Node *</type>
      <name>first_node</name>
      <anchorfile>struct__RU__Map.html</anchorfile>
      <anchor>26a6664a846b703079a8048a263fe1ff</anchor>
      <arglist></arglist>
    </member>
    <member kind="variable">
      <type>RU_Node *</type>
      <name>last_node</name>
      <anchorfile>struct__RU__Map.html</anchorfile>
      <anchor>e218d808e2c933e9a9cc2e7a0bb7b47b</anchor>
      <arglist></arglist>
    </member>
  </compound>
  <compound kind="struct">
    <name>_RU_Node</name>
    <filename>struct__RU__Node.html</filename>
    <member kind="variable">
      <type>RU_Info *</type>
      <name>info</name>
      <anchorfile>struct__RU__Node.html</anchorfile>
      <anchor>f422ed39c1dd54b72ba1b5f37446e489</anchor>
      <arglist></arglist>
    </member>
    <member kind="variable">
      <type>Mdes_Rused *</type>
      <name>rused</name>
      <anchorfile>struct__RU__Node.html</anchorfile>
      <anchor>87d0a81e4c56b835f6d1731c6dfcce0c</anchor>
      <arglist></arglist>
    </member>
    <member kind="variable">
      <type>int</type>
      <name>option_num</name>
      <anchorfile>struct__RU__Node.html</anchorfile>
      <anchor>3003e03711b4c1cc067942e6d7d06e16</anchor>
      <arglist></arglist>
    </member>
    <member kind="variable">
      <type>_RU_Node *</type>
      <name>prev_node</name>
      <anchorfile>struct__RU__Node.html</anchorfile>
      <anchor>4e0a9379ea9e27097d583af68e34667e</anchor>
      <arglist></arglist>
    </member>
    <member kind="variable">
      <type>_RU_Node *</type>
      <name>next_node</name>
      <anchorfile>struct__RU__Node.html</anchorfile>
      <anchor>1e24a03a249fcd0fb58887417d22f889</anchor>
      <arglist></arglist>
    </member>
  </compound>
  <compound kind="struct">
    <name>CB_REG_INFO</name>
    <filename>structCB__REG__INFO.html</filename>
    <member kind="variable">
      <type>float</type>
      <name>current_reg</name>
      <anchorfile>structCB__REG__INFO.html</anchorfile>
      <anchor>2e72cd7897f49a64f88615bd3958c243</anchor>
      <arglist></arglist>
    </member>
    <member kind="variable">
      <type>float</type>
      <name>threshhold</name>
      <anchorfile>structCB__REG__INFO.html</anchorfile>
      <anchor>d6cfce5d02a0f868f6ad9c6162513bc2</anchor>
      <arglist></arglist>
    </member>
    <member kind="variable">
      <type>float</type>
      <name>caller_reg</name>
      <anchorfile>structCB__REG__INFO.html</anchorfile>
      <anchor>bbfee6c098040e43a2961e34b252b8d6</anchor>
      <arglist></arglist>
    </member>
    <member kind="variable">
      <type>float</type>
      <name>caller_thresh</name>
      <anchorfile>structCB__REG__INFO.html</anchorfile>
      <anchor>df148a1508312db2e6c1d8cfa50f00bf</anchor>
      <arglist></arglist>
    </member>
    <member kind="variable">
      <type>float</type>
      <name>callee_reg</name>
      <anchorfile>structCB__REG__INFO.html</anchorfile>
      <anchor>fe48d7307161ffe9247dacaaafc40ffa</anchor>
      <arglist></arglist>
    </member>
    <member kind="variable">
      <type>float</type>
      <name>callee_thresh</name>
      <anchorfile>structCB__REG__INFO.html</anchorfile>
      <anchor>d7cb761eff1bd8abadff7050113e9e48</anchor>
      <arglist></arglist>
    </member>
    <member kind="variable">
      <type>float</type>
      <name>size</name>
      <anchorfile>structCB__REG__INFO.html</anchorfile>
      <anchor>cd5f550383f7a5061b76b281d7b62772</anchor>
      <arglist></arglist>
    </member>
    <member kind="variable">
      <type>Regtypes</type>
      <name>change</name>
      <anchorfile>structCB__REG__INFO.html</anchorfile>
      <anchor>74c08203c7eeffe64de7c5e7d27fc2e1</anchor>
      <arglist></arglist>
    </member>
  </compound>
  <compound kind="struct">
    <name>Dep_Info</name>
    <filename>structDep__Info.html</filename>
    <member kind="variable">
      <type>int</type>
      <name>level</name>
      <anchorfile>structDep__Info.html</anchorfile>
      <anchor>2b3b9716f8659a23c5bddeebb852f36e</anchor>
      <arglist></arglist>
    </member>
    <member kind="variable">
      <type>int</type>
      <name>spec_cond</name>
      <anchorfile>structDep__Info.html</anchorfile>
      <anchor>f9027196bbcd05436d9ad94078f2c468</anchor>
      <arglist></arglist>
    </member>
    <member kind="variable">
      <type>L_Oper *</type>
      <name>prev_branch</name>
      <anchorfile>structDep__Info.html</anchorfile>
      <anchor>c4f5448637f2af44a2a059dc489f551d</anchor>
      <arglist></arglist>
    </member>
    <member kind="variable">
      <type>L_Oper *</type>
      <name>post_branch</name>
      <anchorfile>structDep__Info.html</anchorfile>
      <anchor>2f25629154a32c7e7f5ea22e46085988</anchor>
      <arglist></arglist>
    </member>
    <member kind="variable">
      <type>int</type>
      <name>n_input_dep</name>
      <anchorfile>structDep__Info.html</anchorfile>
      <anchor>bdd954afd95eadb9d190dcefc9d69ccc</anchor>
      <arglist></arglist>
    </member>
    <member kind="variable">
      <type>int</type>
      <name>n_output_dep</name>
      <anchorfile>structDep__Info.html</anchorfile>
      <anchor>ff84d3cffdbbdd362947e3c25adce76e</anchor>
      <arglist></arglist>
    </member>
    <member kind="variable">
      <type>L_Dep *</type>
      <name>input_dep</name>
      <anchorfile>structDep__Info.html</anchorfile>
      <anchor>b034c3f32696bf3bb7cb6802bd4a445e</anchor>
      <arglist></arglist>
    </member>
    <member kind="variable">
      <type>L_Dep *</type>
      <name>output_dep</name>
      <anchorfile>structDep__Info.html</anchorfile>
      <anchor>1649b5225c3af1cfec36ad1283e8d5b2</anchor>
      <arglist></arglist>
    </member>
    <member kind="variable">
      <type>L_Oper *</type>
      <name>oper</name>
      <anchorfile>structDep__Info.html</anchorfile>
      <anchor>977274aa7e08d6decb0251bc4d3b1f49</anchor>
      <arglist></arglist>
    </member>
    <member kind="variable">
      <type>Dep_Info *</type>
      <name>next</name>
      <anchorfile>structDep__Info.html</anchorfile>
      <anchor>91234d01ebe7e748c8a967c08a6c9126</anchor>
      <arglist></arglist>
    </member>
  </compound>
  <compound kind="struct">
    <name>Dep_Operand</name>
    <filename>structDep__Operand.html</filename>
    <member kind="variable">
      <type>unsigned char</type>
      <name>ctype</name>
      <anchorfile>structDep__Operand.html</anchorfile>
      <anchor>24282d5eb15a67425bcdedaefde24126</anchor>
      <arglist></arglist>
    </member>
    <member kind="variable">
      <type>unsigned char</type>
      <name>type</name>
      <anchorfile>structDep__Operand.html</anchorfile>
      <anchor>6ecb91754ca9c77ed86282701b9c4805</anchor>
      <arglist></arglist>
    </member>
    <member kind="variable">
      <type>unsigned char</type>
      <name>ptype</name>
      <anchorfile>structDep__Operand.html</anchorfile>
      <anchor>bded32a0d6663ba050e993d97c4595d0</anchor>
      <arglist></arglist>
    </member>
    <member kind="variable">
      <type>unsigned char</type>
      <name>uncond_def</name>
      <anchorfile>structDep__Operand.html</anchorfile>
      <anchor>17e20ffd7a58c9de95f6bed7c32dcb11</anchor>
      <arglist></arglist>
    </member>
    <member kind="variable">
      <type>int</type>
      <name>value</name>
      <anchorfile>structDep__Operand.html</anchorfile>
      <anchor>dc160357113332acb58a628be253cee6</anchor>
      <arglist></arglist>
    </member>
    <member kind="variable">
      <type>int</type>
      <name>index</name>
      <anchorfile>structDep__Operand.html</anchorfile>
      <anchor>eec3b55f14e198feee174bdc922b3de5</anchor>
      <arglist></arglist>
    </member>
    <member kind="variable">
      <type>L_Oper *</type>
      <name>oper</name>
      <anchorfile>structDep__Operand.html</anchorfile>
      <anchor>7c82d0d618743a813b5c55aed19fb654</anchor>
      <arglist></arglist>
    </member>
    <member kind="variable">
      <type>Dep_Operand *</type>
      <name>next</name>
      <anchorfile>structDep__Operand.html</anchorfile>
      <anchor>0843199a93b411ca4973e2cc43218717</anchor>
      <arglist></arglist>
    </member>
    <member kind="variable">
      <type>Dep_Operand *</type>
      <name>prev</name>
      <anchorfile>structDep__Operand.html</anchorfile>
      <anchor>42b52e93466cbba530835e21cb38c268</anchor>
      <arglist></arglist>
    </member>
  </compound>
  <compound kind="struct">
    <name>L_Dep</name>
    <filename>structL__Dep.html</filename>
    <member kind="variable">
      <type>int</type>
      <name>type</name>
      <anchorfile>structL__Dep.html</anchorfile>
      <anchor>a75e270c32101c223f10620bc4ed49d3</anchor>
      <arglist></arglist>
    </member>
    <member kind="variable">
      <type>int</type>
      <name>omega</name>
      <anchorfile>structL__Dep.html</anchorfile>
      <anchor>c941545f366152dd8ee99d14b873f3eb</anchor>
      <arglist></arglist>
    </member>
    <member kind="variable">
      <type>int</type>
      <name>distance</name>
      <anchorfile>structL__Dep.html</anchorfile>
      <anchor>3aa4c183f3db7e0f38d6d6d4c0b6ea5f</anchor>
      <arglist></arglist>
    </member>
    <member kind="variable">
      <type>int</type>
      <name>from_index</name>
      <anchorfile>structL__Dep.html</anchorfile>
      <anchor>ed792e7cfe402fab33a16f86e9f4852d</anchor>
      <arglist></arglist>
    </member>
    <member kind="variable">
      <type>int</type>
      <name>to_index</name>
      <anchorfile>structL__Dep.html</anchorfile>
      <anchor>2c2923be7c8b34d5d567fe0fd606ac56</anchor>
      <arglist></arglist>
    </member>
    <member kind="variable">
      <type>L_Oper *</type>
      <name>from_oper</name>
      <anchorfile>structL__Dep.html</anchorfile>
      <anchor>9a5be23d0afbd0c502bff914e109051d</anchor>
      <arglist></arglist>
    </member>
    <member kind="variable">
      <type>L_Oper *</type>
      <name>to_oper</name>
      <anchorfile>structL__Dep.html</anchorfile>
      <anchor>0908456ef0f4f4c04f161092e5bc9243</anchor>
      <arglist></arglist>
    </member>
    <member kind="variable">
      <type>L_Dep *</type>
      <name>next_dep</name>
      <anchorfile>structL__Dep.html</anchorfile>
      <anchor>0caaaa0a6e7173a84236f3a199abaade</anchor>
      <arglist></arglist>
    </member>
  </compound>
  <compound kind="struct">
    <name>LEAF</name>
    <filename>structLEAF.html</filename>
    <member kind="variable">
      <type>int</type>
      <name>num_opers</name>
      <anchorfile>structLEAF.html</anchorfile>
      <anchor>a227154d61cb43f06f004a8c5a9cfe11</anchor>
      <arglist></arglist>
    </member>
    <member kind="variable">
      <type>L_Oper **</type>
      <name>oper</name>
      <anchorfile>structLEAF.html</anchorfile>
      <anchor>a92c57bab2c8642ba0cd4c2ef3cf9fad</anchor>
      <arglist></arglist>
    </member>
    <member kind="variable">
      <type>int</type>
      <name>type</name>
      <anchorfile>structLEAF.html</anchorfile>
      <anchor>63c34c3b06e18a0e85c8ee103ae89540</anchor>
      <arglist></arglist>
    </member>
    <member kind="variable">
      <type>int</type>
      <name>initial_height</name>
      <anchorfile>structLEAF.html</anchorfile>
      <anchor>8af80c3770000ebaf3777a686587d87f</anchor>
      <arglist></arglist>
    </member>
    <member kind="variable">
      <type>float</type>
      <name>prob</name>
      <anchorfile>structLEAF.html</anchorfile>
      <anchor>d74aefd20b25b0d95f0d2178b7290e0a</anchor>
      <arglist></arglist>
    </member>
    <member kind="variable">
      <type>int *</type>
      <name>heights</name>
      <anchorfile>structLEAF.html</anchorfile>
      <anchor>db3018de5e9d1f396b24bb9f89668457</anchor>
      <arglist></arglist>
    </member>
  </compound>
  <compound kind="struct">
    <name>OLD_SM_Cb</name>
    <filename>structOLD__SM__Cb.html</filename>
    <member kind="variable">
      <type>SM_Mdes *</type>
      <name>sm_mdes</name>
      <anchorfile>structOLD__SM__Cb.html</anchorfile>
      <anchor>b6289fef8df5372c3a9b4556b88c38ea</anchor>
      <arglist></arglist>
    </member>
    <member kind="variable">
      <type>L_Cb *</type>
      <name>cb</name>
      <anchorfile>structOLD__SM__Cb.html</anchorfile>
      <anchor>4b8ec599ad169f8a5cb0c0325866eb8d</anchor>
      <arglist></arglist>
    </member>
    <member kind="variable">
      <type>OLD_SM_Func *</type>
      <name>sm_func</name>
      <anchorfile>structOLD__SM__Cb.html</anchorfile>
      <anchor>4570cf205a971bfab2323c3c65aaaea4</anchor>
      <arglist></arglist>
    </member>
    <member kind="variable">
      <type>OLD_SM_Oper *</type>
      <name>first_sm_op</name>
      <anchorfile>structOLD__SM__Cb.html</anchorfile>
      <anchor>958efa3e71de60165c7509750b5ccc0b</anchor>
      <arglist></arglist>
    </member>
    <member kind="variable">
      <type>OLD_SM_Oper *</type>
      <name>last_sm_op</name>
      <anchorfile>structOLD__SM__Cb.html</anchorfile>
      <anchor>c4f74b5f9aac0112cb5e647a2de97ee1</anchor>
      <arglist></arglist>
    </member>
    <member kind="variable">
      <type>OLD_SM_Cb *</type>
      <name>next_sm_cb</name>
      <anchorfile>structOLD__SM__Cb.html</anchorfile>
      <anchor>f98bb5e99b4c7528c4f14b1642523d88</anchor>
      <arglist></arglist>
    </member>
    <member kind="variable">
      <type>OLD_SM_Cb *</type>
      <name>prev_sm_cb</name>
      <anchorfile>structOLD__SM__Cb.html</anchorfile>
      <anchor>df6e6cb36cf459ce9fa0ebe70cf4f43b</anchor>
      <arglist></arglist>
    </member>
    <member kind="variable">
      <type>unsigned int *</type>
      <name>map_array</name>
      <anchorfile>structOLD__SM__Cb.html</anchorfile>
      <anchor>b13ab3afa4920d7b982832f4c59df7d8</anchor>
      <arglist></arglist>
    </member>
    <member kind="variable">
      <type>int</type>
      <name>map_array_size</name>
      <anchorfile>structOLD__SM__Cb.html</anchorfile>
      <anchor>b6bea72701ac60f69544ec61df14cf00</anchor>
      <arglist></arglist>
    </member>
    <member kind="variable">
      <type>int</type>
      <name>map_start_offset</name>
      <anchorfile>structOLD__SM__Cb.html</anchorfile>
      <anchor>5df68efd9eefbc24de1747063fc1acde</anchor>
      <arglist></arglist>
    </member>
    <member kind="variable">
      <type>int</type>
      <name>map_end_offset</name>
      <anchorfile>structOLD__SM__Cb.html</anchorfile>
      <anchor>e95b5ff6153e6c655d0db197111983fe</anchor>
      <arglist></arglist>
    </member>
    <member kind="variable">
      <type>int</type>
      <name>min_init_offset</name>
      <anchorfile>structOLD__SM__Cb.html</anchorfile>
      <anchor>2628b42750a8da1b4ce456c87026a4b9</anchor>
      <arglist></arglist>
    </member>
    <member kind="variable">
      <type>int</type>
      <name>max_init_offset</name>
      <anchorfile>structOLD__SM__Cb.html</anchorfile>
      <anchor>5287f4698f85b4e1fefad5c376ed42d1</anchor>
      <arglist></arglist>
    </member>
  </compound>
  <compound kind="struct">
    <name>OLD_SM_Dep</name>
    <filename>structOLD__SM__Dep.html</filename>
    <member kind="variable">
      <type>unsigned char</type>
      <name>type</name>
      <anchorfile>structOLD__SM__Dep.html</anchorfile>
      <anchor>da01e844428883777ea1a4b498d989cf</anchor>
      <arglist></arglist>
    </member>
    <member kind="variable">
      <type>unsigned char</type>
      <name>from_index</name>
      <anchorfile>structOLD__SM__Dep.html</anchorfile>
      <anchor>0e2e67609151b820737a32645b042dc3</anchor>
      <arglist></arglist>
    </member>
    <member kind="variable">
      <type>unsigned char</type>
      <name>to_index</name>
      <anchorfile>structOLD__SM__Dep.html</anchorfile>
      <anchor>86aba489952a26411b1a649ebe8e889d</anchor>
      <arglist></arglist>
    </member>
    <member kind="variable">
      <type>short</type>
      <name>omega</name>
      <anchorfile>structOLD__SM__Dep.html</anchorfile>
      <anchor>282076c360df01837495404d4278bfa4</anchor>
      <arglist></arglist>
    </member>
    <member kind="variable">
      <type>short</type>
      <name>distance</name>
      <anchorfile>structOLD__SM__Dep.html</anchorfile>
      <anchor>bbc73112b9ac3c9d8b4ff184b3edffc1</anchor>
      <arglist></arglist>
    </member>
    <member kind="variable">
      <type>OLD_SM_Oper *</type>
      <name>from_op</name>
      <anchorfile>structOLD__SM__Dep.html</anchorfile>
      <anchor>080ae1717be94e47f7e1eaa27d3e9ec1</anchor>
      <arglist></arglist>
    </member>
    <member kind="variable">
      <type>OLD_SM_Oper *</type>
      <name>to_op</name>
      <anchorfile>structOLD__SM__Dep.html</anchorfile>
      <anchor>05f54fe3fefced2dbf8851c530144510</anchor>
      <arglist></arglist>
    </member>
    <member kind="variable">
      <type>OLD_SM_Dep *</type>
      <name>from_next_dep</name>
      <anchorfile>structOLD__SM__Dep.html</anchorfile>
      <anchor>a53a00343562302b46b210af867c188b</anchor>
      <arglist></arglist>
    </member>
    <member kind="variable">
      <type>OLD_SM_Dep *</type>
      <name>to_next_dep</name>
      <anchorfile>structOLD__SM__Dep.html</anchorfile>
      <anchor>a16d36393a79d02257545dc01b375ab9</anchor>
      <arglist></arglist>
    </member>
  </compound>
  <compound kind="struct">
    <name>OLD_SM_Func</name>
    <filename>structOLD__SM__Func.html</filename>
    <member kind="variable">
      <type>OLD_SM_Cb *</type>
      <name>first_sm_cb</name>
      <anchorfile>structOLD__SM__Func.html</anchorfile>
      <anchor>95adcedfc9a70df9c8fd985bb8c73784</anchor>
      <arglist></arglist>
    </member>
    <member kind="variable">
      <type>OLD_SM_Cb *</type>
      <name>last_sm_cb</name>
      <anchorfile>structOLD__SM__Func.html</anchorfile>
      <anchor>9d9ba650c0484185274ed2208676fcbc</anchor>
      <arglist></arglist>
    </member>
  </compound>
  <compound kind="struct">
    <name>OLD_SM_Oper</name>
    <filename>structOLD__SM__Oper.html</filename>
    <member kind="variable">
      <type>L_Oper *</type>
      <name>op</name>
      <anchorfile>structOLD__SM__Oper.html</anchorfile>
      <anchor>a8657698fb3cd8c02793e0db7079aba4</anchor>
      <arglist></arglist>
    </member>
    <member kind="variable">
      <type>OLD_SM_Cb *</type>
      <name>sm_cb</name>
      <anchorfile>structOLD__SM__Oper.html</anchorfile>
      <anchor>3f65491bf5683015f9f016e0607eec46</anchor>
      <arglist></arglist>
    </member>
    <member kind="variable">
      <type>unsigned int</type>
      <name>flags</name>
      <anchorfile>structOLD__SM__Oper.html</anchorfile>
      <anchor>ad9cb1d97701ed47c0b435b03c8033de</anchor>
      <arglist></arglist>
    </member>
    <member kind="variable">
      <type>unsigned int</type>
      <name>sched_cycle</name>
      <anchorfile>structOLD__SM__Oper.html</anchorfile>
      <anchor>6c3c71e5fc5a42f4538e479f11b1b0f9</anchor>
      <arglist></arglist>
    </member>
    <member kind="variable">
      <type>unsigned int</type>
      <name>min_cycle</name>
      <anchorfile>structOLD__SM__Oper.html</anchorfile>
      <anchor>d44178146aeaedc96dc0960704bbe19b</anchor>
      <arglist></arglist>
    </member>
    <member kind="variable">
      <type>unsigned int</type>
      <name>max_cycle</name>
      <anchorfile>structOLD__SM__Oper.html</anchorfile>
      <anchor>48783ef0f93ef5784ff3beea37caa483</anchor>
      <arglist></arglist>
    </member>
    <member kind="variable">
      <type>unsigned short</type>
      <name>sched_slot</name>
      <anchorfile>structOLD__SM__Oper.html</anchorfile>
      <anchor>d58062620a2f5546f706edcf33df8a51</anchor>
      <arglist></arglist>
    </member>
    <member kind="variable">
      <type>unsigned short</type>
      <name>min_slot</name>
      <anchorfile>structOLD__SM__Oper.html</anchorfile>
      <anchor>c8e97c8f0a0ad1b05169ddd33aa24e98</anchor>
      <arglist></arglist>
    </member>
    <member kind="variable">
      <type>unsigned short</type>
      <name>max_slot</name>
      <anchorfile>structOLD__SM__Oper.html</anchorfile>
      <anchor>66348c62d24bc8cd208eda00f6370e3b</anchor>
      <arglist></arglist>
    </member>
    <member kind="variable">
      <type>unsigned short</type>
      <name>num_unsched_in</name>
      <anchorfile>structOLD__SM__Oper.html</anchorfile>
      <anchor>46c69482dd0c63557c2b3dd06a21633a</anchor>
      <arglist></arglist>
    </member>
    <member kind="variable">
      <type>unsigned short</type>
      <name>num_unsched_out</name>
      <anchorfile>structOLD__SM__Oper.html</anchorfile>
      <anchor>a2954d41856be48e72add9acbdcf6363</anchor>
      <arglist></arglist>
    </member>
    <member kind="variable">
      <type>OLD_SM_Dep *</type>
      <name>input_dep</name>
      <anchorfile>structOLD__SM__Oper.html</anchorfile>
      <anchor>d8a2365b978ee44fbe095ed62e9f17d7</anchor>
      <arglist></arglist>
    </member>
    <member kind="variable">
      <type>OLD_SM_Dep *</type>
      <name>output_dep</name>
      <anchorfile>structOLD__SM__Oper.html</anchorfile>
      <anchor>99831f15ddf4b8f1564358d20058538b</anchor>
      <arglist></arglist>
    </member>
    <member kind="variable">
      <type>OLD_SM_Oper *</type>
      <name>next_smop</name>
      <anchorfile>structOLD__SM__Oper.html</anchorfile>
      <anchor>143e31736bf57214c1dd50ce2e72d540</anchor>
      <arglist></arglist>
    </member>
    <member kind="variable">
      <type>OLD_SM_Oper *</type>
      <name>prev_smop</name>
      <anchorfile>structOLD__SM__Oper.html</anchorfile>
      <anchor>82ded9872fcbf2fd2ab31d0100ce85e6</anchor>
      <arglist></arglist>
    </member>
  </compound>
  <compound kind="struct">
    <name>OLD_SM_Stats</name>
    <filename>structOLD__SM__Stats.html</filename>
    <member kind="variable">
      <type>int</type>
      <name>num_oper_checks</name>
      <anchorfile>structOLD__SM__Stats.html</anchorfile>
      <anchor>5da508a4b660f1e480ad45c1f127f00a</anchor>
      <arglist></arglist>
    </member>
    <member kind="variable">
      <type>int</type>
      <name>num_oper_checks_failed</name>
      <anchorfile>structOLD__SM__Stats.html</anchorfile>
      <anchor>1d69225dfef4a498387505ec18eaf31b</anchor>
      <arglist></arglist>
    </member>
    <member kind="variable">
      <type>int</type>
      <name>num_table_checks</name>
      <anchorfile>structOLD__SM__Stats.html</anchorfile>
      <anchor>95de09761a20fe5d5cc810adb34b7e5f</anchor>
      <arglist></arglist>
    </member>
    <member kind="variable">
      <type>int</type>
      <name>num_table_checks_failed</name>
      <anchorfile>structOLD__SM__Stats.html</anchorfile>
      <anchor>a2b0c80b26a9355859426b5eb11d1bb3</anchor>
      <arglist></arglist>
    </member>
    <member kind="variable">
      <type>int</type>
      <name>num_usage_checks</name>
      <anchorfile>structOLD__SM__Stats.html</anchorfile>
      <anchor>0a750a1bb30af44a2a1ddb4b788b5c66</anchor>
      <arglist></arglist>
    </member>
    <member kind="variable">
      <type>int</type>
      <name>num_usage_checks_failed</name>
      <anchorfile>structOLD__SM__Stats.html</anchorfile>
      <anchor>b1a9040433c434b802301e4917852db4</anchor>
      <arglist></arglist>
    </member>
    <member kind="variable">
      <type>int</type>
      <name>num_slot_checks</name>
      <anchorfile>structOLD__SM__Stats.html</anchorfile>
      <anchor>080d1ed62d29b318c96662ede2abf5f0</anchor>
      <arglist></arglist>
    </member>
    <member kind="variable">
      <type>int</type>
      <name>num_slot_checks_failed</name>
      <anchorfile>structOLD__SM__Stats.html</anchorfile>
      <anchor>b541cfa5db9e75a0b38903e52ebb12c6</anchor>
      <arglist></arglist>
    </member>
  </compound>
  <compound kind="struct">
    <name>Regtypes</name>
    <filename>structRegtypes.html</filename>
    <member kind="variable">
      <type>float</type>
      <name>p</name>
      <anchorfile>structRegtypes.html</anchorfile>
      <anchor>2af8c8ba5d9e2ee614a045fa02272bd8</anchor>
      <arglist></arglist>
    </member>
    <member kind="variable">
      <type>float</type>
      <name>i</name>
      <anchorfile>structRegtypes.html</anchorfile>
      <anchor>871a9fd7031bbcf4b52f327394260667</anchor>
      <arglist></arglist>
    </member>
    <member kind="variable">
      <type>float</type>
      <name>f</name>
      <anchorfile>structRegtypes.html</anchorfile>
      <anchor>bb3424a959a936d3fc7861b73c34924e</anchor>
      <arglist></arglist>
    </member>
    <member kind="variable">
      <type>float</type>
      <name>d</name>
      <anchorfile>structRegtypes.html</anchorfile>
      <anchor>558a146c9877f5814d90732d3e452c45</anchor>
      <arglist></arglist>
    </member>
  </compound>
  <compound kind="struct">
    <name>Sched_Info</name>
    <filename>structSched__Info.html</filename>
    <member kind="variable">
      <type>float</type>
      <name>sort_key1</name>
      <anchorfile>structSched__Info.html</anchorfile>
      <anchor>eab19cb67f100c1e8739ff977a3589b1</anchor>
      <arglist></arglist>
    </member>
    <member kind="variable">
      <type>float</type>
      <name>sort_key2</name>
      <anchorfile>structSched__Info.html</anchorfile>
      <anchor>3bdb78edd15e2d225e854f40b8ab3301</anchor>
      <arglist></arglist>
    </member>
    <member kind="variable">
      <type>float</type>
      <name>rsort_key1</name>
      <anchorfile>structSched__Info.html</anchorfile>
      <anchor>bb2769b8703fd0c911edcd93d3c91c7e</anchor>
      <arglist></arglist>
    </member>
    <member kind="variable">
      <type>float</type>
      <name>rsort_key2</name>
      <anchorfile>structSched__Info.html</anchorfile>
      <anchor>a282d2f1f486240abb917cd4e6d8b64b</anchor>
      <arglist></arglist>
    </member>
    <member kind="variable">
      <type>L_Oper *</type>
      <name>oper</name>
      <anchorfile>structSched__Info.html</anchorfile>
      <anchor>a90db55284bb8cab72794a2cc7628644</anchor>
      <arglist></arglist>
    </member>
    <member kind="variable">
      <type>int</type>
      <name>id</name>
      <anchorfile>structSched__Info.html</anchorfile>
      <anchor>bbb61a37f8f95573cce2103a89ac6f4c</anchor>
      <arglist></arglist>
    </member>
    <member kind="variable">
      <type>int</type>
      <name>proc_opc</name>
      <anchorfile>structSched__Info.html</anchorfile>
      <anchor>9db1f9fdee24e1f908f4ff62a36bedde</anchor>
      <arglist></arglist>
    </member>
    <member kind="variable">
      <type>RU_Info *</type>
      <name>ru_info</name>
      <anchorfile>structSched__Info.html</anchorfile>
      <anchor>4f68c8b780856fd98d7e5de7c93cf609</anchor>
      <arglist></arglist>
    </member>
    <member kind="variable">
      <type>Dep_Info *</type>
      <name>dep_info</name>
      <anchorfile>structSched__Info.html</anchorfile>
      <anchor>2b0bd9c90f7e0483241a0af2e89bc74c</anchor>
      <arglist></arglist>
    </member>
    <member kind="variable">
      <type>Mdes_Info *</type>
      <name>mdes_info</name>
      <anchorfile>structSched__Info.html</anchorfile>
      <anchor>19d71a45986dbd38e365a5fe46f9bfb8</anchor>
      <arglist></arglist>
    </member>
    <member kind="variable">
      <type>Sq_entry *</type>
      <name>entry_list</name>
      <anchorfile>structSched__Info.html</anchorfile>
      <anchor>349f3d661fbe9ed459640b48073aece8</anchor>
      <arglist></arglist>
    </member>
    <member kind="variable">
      <type>int</type>
      <name>spec_cond</name>
      <anchorfile>structSched__Info.html</anchorfile>
      <anchor>a66483d138f7f1ee82ee3662381275f9</anchor>
      <arglist></arglist>
    </member>
    <member kind="variable">
      <type>int</type>
      <name>ctl_dep_level</name>
      <anchorfile>structSched__Info.html</anchorfile>
      <anchor>5d02f68c7bfdacff0b2378e37ff66f48</anchor>
      <arglist></arglist>
    </member>
    <member kind="variable">
      <type>int</type>
      <name>num_depend</name>
      <anchorfile>structSched__Info.html</anchorfile>
      <anchor>42a75fe06a20eb4c61154fe00bce5ef2</anchor>
      <arglist></arglist>
    </member>
    <member kind="variable">
      <type>int *</type>
      <name>operand_ready_times</name>
      <anchorfile>structSched__Info.html</anchorfile>
      <anchor>64c3aeff8171670ce68fde9d3644bcf1</anchor>
      <arglist></arglist>
    </member>
    <member kind="variable">
      <type>float</type>
      <name>benefit</name>
      <anchorfile>structSched__Info.html</anchorfile>
      <anchor>823e7ab4887d9431cb14ad7a21f390c0</anchor>
      <arglist></arglist>
    </member>
    <member kind="variable">
      <type>Regtypes</type>
      <name>kill_set</name>
      <anchorfile>structSched__Info.html</anchorfile>
      <anchor>ca8d2d104f24563407a59e7d77bbd7b4</anchor>
      <arglist></arglist>
    </member>
    <member kind="variable">
      <type>Regtypes</type>
      <name>def_set</name>
      <anchorfile>structSched__Info.html</anchorfile>
      <anchor>70f5b6b0c12b83bf73606f1df239c6d9</anchor>
      <arglist></arglist>
    </member>
    <member kind="variable">
      <type>int</type>
      <name>branch_kill_set_size</name>
      <anchorfile>structSched__Info.html</anchorfile>
      <anchor>c02d368ce947d325a788c0b97e208415</anchor>
      <arglist></arglist>
    </member>
    <member kind="variable">
      <type>int *</type>
      <name>branch_kill_set</name>
      <anchorfile>structSched__Info.html</anchorfile>
      <anchor>8783c93fe484760291bd49a4d175774a</anchor>
      <arglist></arglist>
    </member>
    <member kind="variable">
      <type>float</type>
      <name>priority</name>
      <anchorfile>structSched__Info.html</anchorfile>
      <anchor>29b6c433f08d1f2b6f33bf9bcc728c1d</anchor>
      <arglist></arglist>
    </member>
    <member kind="variable">
      <type>double</type>
      <name>weight</name>
      <anchorfile>structSched__Info.html</anchorfile>
      <anchor>381644649cf0aaa68331b1da73950152</anchor>
      <arglist></arglist>
    </member>
    <member kind="variable">
      <type>float</type>
      <name>instr_prob</name>
      <anchorfile>structSched__Info.html</anchorfile>
      <anchor>66c240cdff24f24bf052ed51613c0632</anchor>
      <arglist></arglist>
    </member>
    <member kind="variable">
      <type>float</type>
      <name>taken_instr_prob</name>
      <anchorfile>structSched__Info.html</anchorfile>
      <anchor>a8a4e140b3f9d8b9c7626d4bf9c8a3fd</anchor>
      <arglist></arglist>
    </member>
    <member kind="variable">
      <type>float</type>
      <name>not_taken_instr_prob</name>
      <anchorfile>structSched__Info.html</anchorfile>
      <anchor>d62e2167e7f6a487d6d2fc7009ee85de</anchor>
      <arglist></arglist>
    </member>
    <member kind="variable">
      <type>int</type>
      <name>index</name>
      <anchorfile>structSched__Info.html</anchorfile>
      <anchor>528ec4ffc5ff171ab6fc3ab5a9e7868c</anchor>
      <arglist></arglist>
    </member>
    <member kind="variable">
      <type>int</type>
      <name>on_stack</name>
      <anchorfile>structSched__Info.html</anchorfile>
      <anchor>d0fdbcdfa6b46f8e11093b4aee43a9e6</anchor>
      <arglist></arglist>
    </member>
    <member kind="variable">
      <type>int</type>
      <name>etime</name>
      <anchorfile>structSched__Info.html</anchorfile>
      <anchor>6c085e39bf1a7416c80584c2aa39d705</anchor>
      <arglist></arglist>
    </member>
    <member kind="variable">
      <type>int *</type>
      <name>ltimes</name>
      <anchorfile>structSched__Info.html</anchorfile>
      <anchor>e544f054abafd1921f4e685c53cc41a6</anchor>
      <arglist></arglist>
    </member>
    <member kind="variable">
      <type>int</type>
      <name>scheduled</name>
      <anchorfile>structSched__Info.html</anchorfile>
      <anchor>b91ed6e88e2007b5f350d91ba4c72d70</anchor>
      <arglist></arglist>
    </member>
    <member kind="variable">
      <type>int</type>
      <name>issue_time</name>
      <anchorfile>structSched__Info.html</anchorfile>
      <anchor>dcf1af1cd102ca766bb1b1bbac96a997</anchor>
      <arglist></arglist>
    </member>
    <member kind="variable">
      <type>int</type>
      <name>issue_slot</name>
      <anchorfile>structSched__Info.html</anchorfile>
      <anchor>80e67cf3c427308cc92215ff91a9b31c</anchor>
      <arglist></arglist>
    </member>
    <member kind="variable">
      <type>int *</type>
      <name>relative_latency</name>
      <anchorfile>structSched__Info.html</anchorfile>
      <anchor>9a7146f1394576ce3da081788c4e63e1</anchor>
      <arglist></arglist>
    </member>
    <member kind="variable">
      <type>int</type>
      <name>ready_time</name>
      <anchorfile>structSched__Info.html</anchorfile>
      <anchor>a0decb6432e5f04ad6c8478402f18b7c</anchor>
      <arglist></arglist>
    </member>
    <member kind="variable">
      <type>int</type>
      <name>earliest_slot</name>
      <anchorfile>structSched__Info.html</anchorfile>
      <anchor>65160cab01d2693b03cb1dda2b3017b4</anchor>
      <arglist></arglist>
    </member>
    <member kind="variable">
      <type>Sched_Info *</type>
      <name>delay_sinfo</name>
      <anchorfile>structSched__Info.html</anchorfile>
      <anchor>e37fe1401cdfa750e6c8969cfc042db4</anchor>
      <arglist></arglist>
    </member>
    <member kind="variable">
      <type>int</type>
      <name>home_block</name>
      <anchorfile>structSched__Info.html</anchorfile>
      <anchor>0729370674717d8dc073e8c8d795c3d1</anchor>
      <arglist></arglist>
    </member>
    <member kind="variable">
      <type>int</type>
      <name>current_block</name>
      <anchorfile>structSched__Info.html</anchorfile>
      <anchor>ad5c63aafaa1ba037a56be2e10f8922c</anchor>
      <arglist></arglist>
    </member>
    <member kind="variable">
      <type>int</type>
      <name>orig_block</name>
      <anchorfile>structSched__Info.html</anchorfile>
      <anchor>5dc48577b12198f60db35220ba3adba1</anchor>
      <arglist></arglist>
    </member>
    <member kind="variable">
      <type>L_Oper *</type>
      <name>prev_br</name>
      <anchorfile>structSched__Info.html</anchorfile>
      <anchor>567a780c15b4d3125f17bf391973003d</anchor>
      <arglist></arglist>
    </member>
    <member kind="variable">
      <type>L_Oper *</type>
      <name>post_br</name>
      <anchorfile>structSched__Info.html</anchorfile>
      <anchor>21ec4e846c59104aeca50c249167e8d5</anchor>
      <arglist></arglist>
    </member>
    <member kind="variable">
      <type>L_Oper *</type>
      <name>extend_lr_down</name>
      <anchorfile>structSched__Info.html</anchorfile>
      <anchor>aa024cfcfaf259b832c203cc219cd151</anchor>
      <arglist></arglist>
    </member>
    <member kind="variable">
      <type>L_Oper *</type>
      <name>check_op</name>
      <anchorfile>structSched__Info.html</anchorfile>
      <anchor>bdd5d9297515e751ed153f5c6bdb42e1</anchor>
      <arglist></arglist>
    </member>
    <member kind="variable">
      <type>L_Oper_List *</type>
      <name>store_list</name>
      <anchorfile>structSched__Info.html</anchorfile>
      <anchor>da5936895a62f32f5355614cb74aecb3</anchor>
      <arglist></arglist>
    </member>
  </compound>
  <compound kind="struct">
    <name>Sq_entry</name>
    <filename>structSq__entry.html</filename>
    <member kind="variable">
      <type>Squeue *</type>
      <name>queue</name>
      <anchorfile>structSq__entry.html</anchorfile>
      <anchor>52d13b14a8db413a62a992c560815171</anchor>
      <arglist></arglist>
    </member>
    <member kind="variable">
      <type>Sched_Info *</type>
      <name>sinfo</name>
      <anchorfile>structSq__entry.html</anchorfile>
      <anchor>6839d29afae500d88efd490fb557c6cd</anchor>
      <arglist></arglist>
    </member>
    <member kind="variable">
      <type>Sq_entry *</type>
      <name>next_entry</name>
      <anchorfile>structSq__entry.html</anchorfile>
      <anchor>06d0ad1b3078e0034e33eb329297a883</anchor>
      <arglist></arglist>
    </member>
    <member kind="variable">
      <type>Sq_entry *</type>
      <name>prev_entry</name>
      <anchorfile>structSq__entry.html</anchorfile>
      <anchor>29d35af86ee481c92549514bc52986e9</anchor>
      <arglist></arglist>
    </member>
    <member kind="variable">
      <type>Sq_entry *</type>
      <name>next_queue</name>
      <anchorfile>structSq__entry.html</anchorfile>
      <anchor>2c602e3d1d923512028b6a8ad753d233</anchor>
      <arglist></arglist>
    </member>
    <member kind="variable">
      <type>Sq_entry *</type>
      <name>prev_queue</name>
      <anchorfile>structSq__entry.html</anchorfile>
      <anchor>1c2bb8a676cf54b08704f36f2709877c</anchor>
      <arglist></arglist>
    </member>
  </compound>
  <compound kind="struct">
    <name>Squeue</name>
    <filename>structSqueue.html</filename>
    <member kind="variable">
      <type>char</type>
      <name>name</name>
      <anchorfile>structSqueue.html</anchorfile>
      <anchor>52c5171f9689ba39176d5e4faf49735f</anchor>
      <arglist>[40]</arglist>
    </member>
    <member kind="variable">
      <type>int</type>
      <name>size</name>
      <anchorfile>structSqueue.html</anchorfile>
      <anchor>166b421ff449a1e6ad7fb5195ca3001e</anchor>
      <arglist></arglist>
    </member>
    <member kind="variable">
      <type>Sq_entry *</type>
      <name>head</name>
      <anchorfile>structSqueue.html</anchorfile>
      <anchor>4f8a89bc1de87d6976d1a6a314671450</anchor>
      <arglist></arglist>
    </member>
    <member kind="variable">
      <type>Sq_entry *</type>
      <name>tail</name>
      <anchorfile>structSqueue.html</anchorfile>
      <anchor>cdc1f8d33a5bc5f0313b44d5a02d3fa4</anchor>
      <arglist></arglist>
    </member>
    <member kind="variable">
      <type>Sq_entry *</type>
      <name>current</name>
      <anchorfile>structSqueue.html</anchorfile>
      <anchor>1739147b7428df9891da9bb6bbb29584</anchor>
      <arglist></arglist>
    </member>
  </compound>
  <compound kind="struct">
    <name>Vreg</name>
    <filename>structVreg.html</filename>
    <member kind="variable">
      <type>int</type>
      <name>live_in_fall_thru_path</name>
      <anchorfile>structVreg.html</anchorfile>
      <anchor>cf94cbee2bfe45387349a4db0917e306</anchor>
      <arglist></arglist>
    </member>
    <member kind="variable">
      <type>int</type>
      <name>ctype</name>
      <anchorfile>structVreg.html</anchorfile>
      <anchor>dd5a9ea90cd35f072502ea849823a36b</anchor>
      <arglist></arglist>
    </member>
    <member kind="variable">
      <type>Squeue *</type>
      <name>use_queue</name>
      <anchorfile>structVreg.html</anchorfile>
      <anchor>3e675d1a9707ce8ebedbd27bef25dae7</anchor>
      <arglist></arglist>
    </member>
  </compound>
</tagfile>
