<?xml version='1.0' encoding='ISO-8859-1' standalone='yes' ?>
<tagfile>
  <compound kind="file">
    <name>l_gen_pipe.c</name>
    <path>/mnt/hgfs/mkt/Desktop/openimpact/src/Lcode/codegen/Lsoftpipe/</path>
    <filename>l__gen__pipe_8c</filename>
    <includes id="l__softpipe__int_8h" name="l_softpipe_int.h" local="yes" imported="no">l_softpipe_int.h</includes>
    <includes id="l__mve_8h" name="l_mve.h" local="yes" imported="no">l_mve.h</includes>
    <includes id="l__pipe__sync_8h" name="l_pipe_sync.h" local="yes" imported="no">l_pipe_sync.h</includes>
    <member kind="define">
      <type>#define</type>
      <name>UNKNOWN_WEIGHT</name>
      <anchorfile>l__gen__pipe_8c.html</anchorfile>
      <anchor>c697651b01b512ea5feb51514bde3b69</anchor>
      <arglist></arglist>
    </member>
    <member kind="define">
      <type>#define</type>
      <name>FALL_THRU_EPI</name>
      <anchorfile>l__gen__pipe_8c.html</anchorfile>
      <anchor>5a0660410880f0cbe74ab9e1554b2e65</anchor>
      <arglist></arglist>
    </member>
    <member kind="define">
      <type>#define</type>
      <name>TARGET_EPI</name>
      <anchorfile>l__gen__pipe_8c.html</anchorfile>
      <anchor>e4186ebd15ff2109e517a995ff3d27bd</anchor>
      <arglist></arglist>
    </member>
    <member kind="function">
      <type>void</type>
      <name>Lpipe_rem_loop_gen</name>
      <anchorfile>l__gen__pipe_8c.html</anchorfile>
      <anchor>cf8d2344c93cfbe701567693dc65e006</anchor>
      <arglist>(L_Func *fn, L_Oper *loop_incr, int ii, int unroll, int stage_count)</arglist>
    </member>
    <member kind="function" static="yes">
      <type>static void</type>
      <name>Lpipe_gen_epi</name>
      <anchorfile>l__gen__pipe_8c.html</anchorfile>
      <anchor>2674fb9325b92361e2fe22b6bad6ceb0</anchor>
      <arglist>(L_Func *fn, L_Cb *header_cb, L_Oper *exit_branch, L_Cb *fall_thru_cb, int epi_type, int ii, int unroll, int stage_count)</arglist>
    </member>
    <member kind="function">
      <type>L_Oper *</type>
      <name>Lpipe_find_exit_branch_copy</name>
      <anchorfile>l__gen__pipe_8c.html</anchorfile>
      <anchor>e170c12c10c855baea6d432483bb8f47</anchor>
      <arglist>(L_Oper *exit_branch, L_Oper *first_oper, L_Oper *last_oper)</arglist>
    </member>
    <member kind="function" static="yes">
      <type>static void</type>
      <name>Lpipe_gen_partial_epi</name>
      <anchorfile>l__gen__pipe_8c.html</anchorfile>
      <anchor>da8391af357e6351d97848a628f5fe5a</anchor>
      <arglist>(L_Func *fn, L_Cb *header_cb, L_Oper *exit_branch, L_Cb *fall_thru_cb, int ii, int unroll, int stage_count)</arglist>
    </member>
    <member kind="function">
      <type>void</type>
      <name>M_starcore_negate_compare</name>
      <anchorfile>l__gen__pipe_8c.html</anchorfile>
      <anchor>342fb7dd4cac19f8ee85a2eafcca5b6a</anchor>
      <arglist>(L_Oper *)</arglist>
    </member>
    <member kind="function">
      <type>void</type>
      <name>Lpipe_multi_epi_gen</name>
      <anchorfile>l__gen__pipe_8c.html</anchorfile>
      <anchor>aab08a046531a4539e4a0dbb0f861007</anchor>
      <arglist>(L_Func *fn, int stage_count, L_Oper *loop_back_br, int ii, int unroll)</arglist>
    </member>
    <member kind="function" static="yes">
      <type>static void</type>
      <name>Lpipe_remove_comp_code_block</name>
      <anchorfile>l__gen__pipe_8c.html</anchorfile>
      <anchor>adcda83dc0d7fe6f138348970f5fee5b</anchor>
      <arglist>(L_Cb *ssa_cb)</arglist>
    </member>
    <member kind="function" static="yes">
      <type>static void</type>
      <name>Lpipe_remove_epilogue</name>
      <anchorfile>l__gen__pipe_8c.html</anchorfile>
      <anchor>e6db76f9944ace66b43330c69c01141d</anchor>
      <arglist>(L_Func *fn, L_Oper *exit, L_Cb *pipe_cb, L_Cb *epilogue_cb, L_Cb *fall_thru_cb, int epi_type)</arglist>
    </member>
    <member kind="function">
      <type>void</type>
      <name>Lpipe_remove_empty_epilogues</name>
      <anchorfile>l__gen__pipe_8c.html</anchorfile>
      <anchor>5b2118879870ec2987266350b965aa44</anchor>
      <arglist>(L_Func *fn, L_Cb *header_cb)</arglist>
    </member>
    <member kind="function">
      <type>L_Oper *</type>
      <name>Lpipe_gen_epilogue_counter</name>
      <anchorfile>l__gen__pipe_8c.html</anchorfile>
      <anchor>7be730411e03e5b53f8cf5cb5f54109b</anchor>
      <arglist>(L_Func *fn, L_Cb *cb, int i, int top)</arglist>
    </member>
    <member kind="function">
      <type>L_Oper *</type>
      <name>Lpipe_gen_loop_counter</name>
      <anchorfile>l__gen__pipe_8c.html</anchorfile>
      <anchor>33334914647461bc2c86d8527bee4f8e</anchor>
      <arglist>(L_Func *fn, L_Cb *cb, int i, int top)</arglist>
    </member>
    <member kind="function" static="yes">
      <type>static void</type>
      <name>Lpipe_change_branch_dir</name>
      <anchorfile>l__gen__pipe_8c.html</anchorfile>
      <anchor>0bec54aba5a2d1f6b74452f61bc4964d</anchor>
      <arglist>(L_Oper *branch, L_Cb *target)</arglist>
    </member>
    <member kind="function" static="yes">
      <type>static void</type>
      <name>Lpipe_gen_partial_epi_rot</name>
      <anchorfile>l__gen__pipe_8c.html</anchorfile>
      <anchor>9377572e449c670f48d1f004429378d0</anchor>
      <arglist>(L_Func *fn, L_Cb *header_cb, L_Oper *exit_branch, L_Cb *fall_thru_cb, int ii, int stage_count)</arglist>
    </member>
    <member kind="function" static="yes">
      <type>static void</type>
      <name>Lpipe_gen_epi_rot</name>
      <anchorfile>l__gen__pipe_8c.html</anchorfile>
      <anchor>5526a678946eac6147b0fa7128e53215</anchor>
      <arglist>(SM_Cb *sm_cb, L_Oper *exit_branch, L_Cb *fall_thru_cb, int epi_type)</arglist>
    </member>
    <member kind="function">
      <type>void</type>
      <name>Lpipe_multi_epi_gen_rot</name>
      <anchorfile>l__gen__pipe_8c.html</anchorfile>
      <anchor>730dc4cccecf03f47801ce2b86ebb338</anchor>
      <arglist>(SM_Cb *sm_cb, L_Oper *loop_back_br)</arglist>
    </member>
    <member kind="function">
      <type>void</type>
      <name>Lpipe_kernel_gen_rot</name>
      <anchorfile>l__gen__pipe_8c.html</anchorfile>
      <anchor>719224974bbbb1c9845b638a30cedd93</anchor>
      <arglist>(SM_Cb *sm_cb, L_Oper *loop_back_br)</arglist>
    </member>
  </compound>
  <compound kind="file">
    <name>l_gen_pipe.h</name>
    <path>/mnt/hgfs/mkt/Desktop/openimpact/src/Lcode/codegen/Lsoftpipe/</path>
    <filename>l__gen__pipe_8h</filename>
    <member kind="function">
      <type>void</type>
      <name>Lpipe_rem_loop_gen</name>
      <anchorfile>l__gen__pipe_8h.html</anchorfile>
      <anchor>416f050f19c7a933adf9fe1d4102d309</anchor>
      <arglist>(L_Func *, L_Oper *, int, int, int)</arglist>
    </member>
    <member kind="function">
      <type>void</type>
      <name>Lpipe_multi_epi_gen</name>
      <anchorfile>l__gen__pipe_8h.html</anchorfile>
      <anchor>7ff3a23339ed670d134900128579db81</anchor>
      <arglist>(L_Func *, int, L_Oper *, int, int)</arglist>
    </member>
    <member kind="function">
      <type>void</type>
      <name>Lpipe_multi_epi_gen_rot</name>
      <anchorfile>l__gen__pipe_8h.html</anchorfile>
      <anchor>3616d73f0912210bda14a73e1814421a</anchor>
      <arglist>(SM_Cb *, L_Oper *)</arglist>
    </member>
    <member kind="function">
      <type>void</type>
      <name>Lpipe_kernel_gen_rot</name>
      <anchorfile>l__gen__pipe_8h.html</anchorfile>
      <anchor>5253f9f10f873086c2794845af160b32</anchor>
      <arglist>(SM_Cb *, L_Oper *)</arglist>
    </member>
    <member kind="function">
      <type>void</type>
      <name>Lpipe_remove_empty_epilogues</name>
      <anchorfile>l__gen__pipe_8h.html</anchorfile>
      <anchor>d0580a626aeebbf86ad383faecbde955</anchor>
      <arglist>(L_Func *, L_Cb *)</arglist>
    </member>
  </compound>
  <compound kind="file">
    <name>l_loop_prep.c</name>
    <path>/mnt/hgfs/mkt/Desktop/openimpact/src/Lcode/codegen/Lsoftpipe/</path>
    <filename>l__loop__prep_8c</filename>
    <includes id="l__softpipe__int_8h" name="l_softpipe_int.h" local="yes" imported="no">l_softpipe_int.h</includes>
    <includes id="l__loop__prep_8h" name="l_loop_prep.h" local="yes" imported="no">l_loop_prep.h</includes>
    <member kind="define">
      <type>#define</type>
      <name>UNKNOWN_WEIGHT</name>
      <anchorfile>l__loop__prep_8c.html</anchorfile>
      <anchor>c697651b01b512ea5feb51514bde3b69</anchor>
      <arglist></arglist>
    </member>
    <member kind="define">
      <type>#define</type>
      <name>DUMMY_VAL</name>
      <anchorfile>l__loop__prep_8c.html</anchorfile>
      <anchor>56e0504cd80a4cf785e7d94588e099fb</anchor>
      <arglist></arglist>
    </member>
    <member kind="function" static="yes">
      <type>static int</type>
      <name>Lpipe_is_post_increment</name>
      <anchorfile>l__loop__prep_8c.html</anchorfile>
      <anchor>7a5c72b5abaa652ae2341191045d9e2d</anchor>
      <arglist>(L_Oper *ind_op, L_Oper *loop_back_br)</arglist>
    </member>
    <member kind="function" static="yes">
      <type>static void</type>
      <name>Lpipe_fix_max</name>
      <anchorfile>l__loop__prep_8c.html</anchorfile>
      <anchor>e9566433a68082088c8d7df1cd73bc6f</anchor>
      <arglist>(L_Func *fn, L_Operand *ind_incr, L_Operand *ind_var, L_Oper *loop_back_br, L_Cb *preheader)</arglist>
    </member>
    <member kind="function" static="yes">
      <type>static L_Oper *</type>
      <name>Lpipe_loop_induction_reversal</name>
      <anchorfile>l__loop__prep_8c.html</anchorfile>
      <anchor>05e15e20cb9123ceba439ef594aee91a</anchor>
      <arglist>(L_Func *fn, L_Cb *header_cb, L_Cb *preheader_cb, L_Oper *loop_back_br)</arglist>
    </member>
    <member kind="function" static="yes">
      <type>static void</type>
      <name>Lpipe_rem_loop</name>
      <anchorfile>l__loop__prep_8c.html</anchorfile>
      <anchor>2f87543f0c44feedf9c4085fb0037d22</anchor>
      <arglist>(L_Func *fn, L_Cb *header_cb, L_Cb *preheader_cb, L_Oper *loop_back_br, L_Oper *loop_incr_op)</arglist>
    </member>
    <member kind="function">
      <type>void</type>
      <name>Lpipe_loop_prep_init</name>
      <anchorfile>l__loop__prep_8c.html</anchorfile>
      <anchor>4eeb6fd3ff13b6090f380c27cd9c03f1</anchor>
      <arglist>(Parm_Macro_List *command_line_macro_list)</arglist>
    </member>
    <member kind="function">
      <type>void</type>
      <name>Lpipe_loop_prep_end</name>
      <anchorfile>l__loop__prep_8c.html</anchorfile>
      <anchor>bbe8166961529f868de1ab8ed3cd6ba3</anchor>
      <arglist>()</arglist>
    </member>
    <member kind="function">
      <type>void</type>
      <name>Lpipe_softpipe_loop_prep</name>
      <anchorfile>l__loop__prep_8c.html</anchorfile>
      <anchor>a7825f4c4240ff3b4d94ad59e278cda3</anchor>
      <arglist>(L_Func *fn)</arglist>
    </member>
    <member kind="variable" static="yes">
      <type>static FILE *</type>
      <name>prep_statfile</name>
      <anchorfile>l__loop__prep_8c.html</anchorfile>
      <anchor>57718bd8c073acf3763a16adc4ce7d14</anchor>
      <arglist></arglist>
    </member>
  </compound>
  <compound kind="file">
    <name>l_loop_prep.h</name>
    <path>/mnt/hgfs/mkt/Desktop/openimpact/src/Lcode/codegen/Lsoftpipe/</path>
    <filename>l__loop__prep_8h</filename>
    <member kind="function">
      <type>void</type>
      <name>Lpipe_loop_prep_init</name>
      <anchorfile>l__loop__prep_8h.html</anchorfile>
      <anchor>426d800c81ba417a208915c6d6949405</anchor>
      <arglist>(Parm_Macro_List *)</arglist>
    </member>
    <member kind="function">
      <type>void</type>
      <name>Lpipe_loop_prep_end</name>
      <anchorfile>l__loop__prep_8h.html</anchorfile>
      <anchor>bbe8166961529f868de1ab8ed3cd6ba3</anchor>
      <arglist>()</arglist>
    </member>
    <member kind="function">
      <type>void</type>
      <name>Lpipe_softpipe_loop_prep</name>
      <anchorfile>l__loop__prep_8h.html</anchorfile>
      <anchor>52ada762dc510133a69facd93899b1c9</anchor>
      <arglist>(L_Func *)</arglist>
    </member>
  </compound>
  <compound kind="file">
    <name>l_mve.c</name>
    <path>/mnt/hgfs/mkt/Desktop/openimpact/src/Lcode/codegen/Lsoftpipe/</path>
    <filename>l__mve_8c</filename>
    <includes id="l__softpipe__int_8h" name="l_softpipe_int.h" local="yes" imported="no">l_softpipe_int.h</includes>
    <includes id="l__mve_8h" name="l_mve.h" local="yes" imported="no">l_mve.h</includes>
    <member kind="define">
      <type>#define</type>
      <name>LPIPE_LR_TYPE</name>
      <anchorfile>l__mve_8c.html</anchorfile>
      <anchor>266ef6e31198aaf35fff458b7b01346c</anchor>
      <arglist></arglist>
    </member>
    <member kind="function" static="yes">
      <type>static Lpipe_LRInfo *</type>
      <name>Lpipe_create_live_range_info</name>
      <anchorfile>l__mve_8c.html</anchorfile>
      <anchor>88cf47566d993f9dbdcd9a5ad50c70f3</anchor>
      <arglist>(SM_Reg_Info *rinfo)</arglist>
    </member>
    <member kind="function" static="yes">
      <type>static List</type>
      <name>Lpipe_delete_all_live_range_info</name>
      <anchorfile>l__mve_8c.html</anchorfile>
      <anchor>9db1381b8d089a5564206ab6ba6800f1</anchor>
      <arglist>(List lr_list)</arglist>
    </member>
    <member kind="function" static="yes">
      <type>static Lpipe_LRInfo *</type>
      <name>Lpipe_find_live_range</name>
      <anchorfile>l__mve_8c.html</anchorfile>
      <anchor>b4bea3da2926f660a7ec49c20a4e6eab</anchor>
      <arglist>(List lr_list, SM_Reg_Info *rinfo)</arglist>
    </member>
    <member kind="function" static="yes">
      <type>static void</type>
      <name>Lpipe_update_lr_last_acc</name>
      <anchorfile>l__mve_8c.html</anchorfile>
      <anchor>1d39b58ce865b3522fddcb21b1195f28</anchor>
      <arglist>(Lpipe_LRInfo *lr_info, int issue_time, int issue_slot, int ii)</arglist>
    </member>
    <member kind="function" static="yes">
      <type>static Lpipe_MVEInfo *</type>
      <name>Lpipe_new_opd_mve_info</name>
      <anchorfile>l__mve_8c.html</anchorfile>
      <anchor>46a16b2e81536e0801824c9354f817fe</anchor>
      <arglist>(Lpipe_LRInfo *lr)</arglist>
    </member>
    <member kind="function" static="yes">
      <type>static void</type>
      <name>Lpipe_print_live_ranges</name>
      <anchorfile>l__mve_8c.html</anchorfile>
      <anchor>6351038c17aadb5b3409428a8d2fd12b</anchor>
      <arglist>(List lr_list, int ii)</arglist>
    </member>
    <member kind="function" static="yes">
      <type>static int</type>
      <name>Lpipe_modulo_decrement</name>
      <anchorfile>l__mve_8c.html</anchorfile>
      <anchor>7dd82e8867f2fca8e451a4c7be59fd17</anchor>
      <arglist>(int count, int unrolled)</arglist>
    </member>
    <member kind="function" static="yes">
      <type>static void</type>
      <name>Lpipe_print_mve_info</name>
      <anchorfile>l__mve_8c.html</anchorfile>
      <anchor>072448416103b6792781cb20ec102be1</anchor>
      <arglist>(SM_Cb *sm_cb)</arglist>
    </member>
    <member kind="function" static="yes">
      <type>static void</type>
      <name>Lpipe_delete_all_mve_info</name>
      <anchorfile>l__mve_8c.html</anchorfile>
      <anchor>746c620b9309f05bc7a78d8e1ef1f91b</anchor>
      <arglist>(L_Oper *oper)</arglist>
    </member>
    <member kind="function">
      <type>void</type>
      <name>Lpipe_free_mve_info</name>
      <anchorfile>l__mve_8c.html</anchorfile>
      <anchor>78c3c1d55b2de2cc8d2c30fc73669d6b</anchor>
      <arglist>()</arglist>
    </member>
    <member kind="function" static="yes">
      <type>static int</type>
      <name>Lpipe_get_best_lr</name>
      <anchorfile>l__mve_8c.html</anchorfile>
      <anchor>d79d013591249d3e2a5c053799910698</anchor>
      <arglist>(List *op_array, Lpipe_LRInfo *prev_lr_info, Lpipe_LRInfo **lr_info, int ii)</arglist>
    </member>
    <member kind="function" static="yes">
      <type>static int</type>
      <name>Lpipe_associate_rot_regs</name>
      <anchorfile>l__mve_8c.html</anchorfile>
      <anchor>cd205735f2bb6b94e9d3fb0b544e5bc0</anchor>
      <arglist>(List lr_list, SM_Cb *sm_cb, int count_only)</arglist>
    </member>
    <member kind="function" static="yes">
      <type>static int</type>
      <name>Lpipe_associate_mve_regs</name>
      <anchorfile>l__mve_8c.html</anchorfile>
      <anchor>cd8521ecc279eb04026b93e86c2d4d90</anchor>
      <arglist>(List lr_list, SM_Cb *sm_cb)</arglist>
    </member>
    <member kind="function" static="yes">
      <type>static List</type>
      <name>Lpipe_construct_lr</name>
      <anchorfile>l__mve_8c.html</anchorfile>
      <anchor>e3f004767b57ec9efce4fbd01338344d</anchor>
      <arglist>(SM_Cb *kernel_cb)</arglist>
    </member>
    <member kind="function">
      <type>int</type>
      <name>Lpipe_analyze_lr</name>
      <anchorfile>l__mve_8c.html</anchorfile>
      <anchor>66180998b7a313e243cc0cef7058723e</anchor>
      <arglist>(SM_Cb *kernel_cb, int count_only)</arglist>
    </member>
    <member kind="function">
      <type>int</type>
      <name>Lpipe_find_name</name>
      <anchorfile>l__mve_8c.html</anchorfile>
      <anchor>be2ad277734a8abfd86d44294fa8983d</anchor>
      <arglist>(SM_Cb *sm_cb, Lpipe_LRInfo *lr_info, Lpipe_MVEInfo *mve_info)</arglist>
    </member>
    <member kind="function" static="yes">
      <type>static int</type>
      <name>Lpipe_mov_opc</name>
      <anchorfile>l__mve_8c.html</anchorfile>
      <anchor>dbcc1c6752c12e5d507c9bb136143844</anchor>
      <arglist>(L_Operand *opd)</arglist>
    </member>
    <member kind="function">
      <type>int</type>
      <name>Lpipe_rreg_transform</name>
      <anchorfile>l__mve_8c.html</anchorfile>
      <anchor>6d37a198558a19c6fc216dc807812c85</anchor>
      <arglist>(SM_Cb *sm_cb)</arglist>
    </member>
    <member kind="function" static="yes">
      <type>static void</type>
      <name>Lpipe_gen_rot_pro_fixup</name>
      <anchorfile>l__mve_8c.html</anchorfile>
      <anchor>ae45409fd9131df5bc8f83729510812a</anchor>
      <arglist>(L_Func *fn, L_Cb *cb, Lpipe_LRInfo *lr_info, int reg)</arglist>
    </member>
    <member kind="function">
      <type>void</type>
      <name>Lpipe_fix_live_in_rot</name>
      <anchorfile>l__mve_8c.html</anchorfile>
      <anchor>bb2be6c164712e95e8e6126203115def</anchor>
      <arglist>(SM_Cb *sm_cb)</arglist>
    </member>
    <member kind="function" static="yes">
      <type>static void</type>
      <name>Lpipe_gen_rot_epi_fixup</name>
      <anchorfile>l__mve_8c.html</anchorfile>
      <anchor>4db10761962fd1923c7ca39914f642cc</anchor>
      <arglist>(L_Func *fn, L_Cb *cb, Lpipe_LRInfo *lr_info, int name_index)</arglist>
    </member>
    <member kind="function">
      <type>void</type>
      <name>Lpipe_fix_live_out_rot</name>
      <anchorfile>l__mve_8c.html</anchorfile>
      <anchor>980d54700d750838e30a785314e34805</anchor>
      <arglist>(SM_Cb *sm_cb, SM_Oper *sm_loop_back_br)</arglist>
    </member>
    <member kind="function">
      <type>void</type>
      <name>Lpipe_mve_transform</name>
      <anchorfile>l__mve_8c.html</anchorfile>
      <anchor>b3521b99ecff5fb6e631fe1caa6d81bc</anchor>
      <arglist>(SM_Cb *sm_cb, int unroll)</arglist>
    </member>
    <member kind="function">
      <type>void</type>
      <name>Lpipe_fix_live_in</name>
      <anchorfile>l__mve_8c.html</anchorfile>
      <anchor>b9b475f6cf9c77cd9d0d58f080c0bef5</anchor>
      <arglist>(SM_Cb *sm_cb)</arglist>
    </member>
    <member kind="function" static="yes">
      <type>static void</type>
      <name>Lpipe_merge_compensation_code</name>
      <anchorfile>l__mve_8c.html</anchorfile>
      <anchor>300fcb8a4b5a8a4fc7f52e0b8c193f91</anchor>
      <arglist>(L_Cb *epilogue_cb)</arglist>
    </member>
    <member kind="function" static="yes">
      <type>static void</type>
      <name>Lpipe_fix_live_out_epi</name>
      <anchorfile>l__mve_8c.html</anchorfile>
      <anchor>1ee5f471df4bc048477d40a2ab83bb47</anchor>
      <arglist>(L_Cb *kernel_cb, L_Oper *exit_branch)</arglist>
    </member>
    <member kind="function" static="yes">
      <type>static Set</type>
      <name>Lpipe_fix_set_out_epi</name>
      <anchorfile>l__mve_8c.html</anchorfile>
      <anchor>f6ff4bb1935ebcb64fec63d238e51fa0</anchor>
      <arglist>(Set to_fix, L_Cb *kernel_cb, L_Oper *exit_branch)</arglist>
    </member>
    <member kind="function" static="yes">
      <type>static void</type>
      <name>Lpipe_fix_live_out_stage</name>
      <anchorfile>l__mve_8c.html</anchorfile>
      <anchor>e88028a00126e6131b9cc667b88f86bb</anchor>
      <arglist>(L_Cb *kernel_cb, L_Oper *exit_branch, L_Oper *first_op, L_Oper *last_op, int stage)</arglist>
    </member>
    <member kind="function" static="yes">
      <type>static Set</type>
      <name>Lpipe_fix_set_out_stage</name>
      <anchorfile>l__mve_8c.html</anchorfile>
      <anchor>26ca66a6c5177a42f255a740d1979087</anchor>
      <arglist>(Set to_fix, L_Cb *kernel_cb, L_Oper *exit_branch, L_Oper *first_op, L_Oper *last_op, int stage)</arglist>
    </member>
    <member kind="function">
      <type>void</type>
      <name>Lpipe_fix_live_out</name>
      <anchorfile>l__mve_8c.html</anchorfile>
      <anchor>be5df0a0e4bf74c50235e9a87e157c37</anchor>
      <arglist>(SM_Cb *sm_cb, int unroll)</arglist>
    </member>
    <member kind="function">
      <type>int</type>
      <name>Lpipe_get_pro_reg</name>
      <anchorfile>l__mve_8c.html</anchorfile>
      <anchor>bc251ae063227ad587bde4091a7a8a7a</anchor>
      <arglist>(SM_Cb *sm_cb, Lpipe_LRInfo *lr_info, Lpipe_MVEInfo *mve_info, int stg_ofst)</arglist>
    </member>
    <member kind="variable">
      <type>L_Alloc_Pool *</type>
      <name>Lpipe_src_mve_pool</name>
      <anchorfile>l__mve_8c.html</anchorfile>
      <anchor>7f8629ad0a52237fabc7b5b90c9a369c</anchor>
      <arglist></arglist>
    </member>
    <member kind="variable">
      <type>L_Alloc_Pool *</type>
      <name>Lpipe_pred_mve_pool</name>
      <anchorfile>l__mve_8c.html</anchorfile>
      <anchor>3c5eb3ef3a82d664ba3a3da10b8e0093</anchor>
      <arglist></arglist>
    </member>
    <member kind="variable">
      <type>L_Alloc_Pool *</type>
      <name>Lpipe_dest_mve_pool</name>
      <anchorfile>l__mve_8c.html</anchorfile>
      <anchor>4fd7a438ad2a7b1efa5d57d7aa1933b2</anchor>
      <arglist></arglist>
    </member>
    <member kind="variable" static="yes">
      <type>static L_Alloc_Pool *</type>
      <name>Lpipe_LRInfo_pool</name>
      <anchorfile>l__mve_8c.html</anchorfile>
      <anchor>c304f63e31503bac19dfdafffb1acc0e</anchor>
      <arglist></arglist>
    </member>
    <member kind="variable">
      <type>L_Alloc_Pool *</type>
      <name>Lpipe_MVEInfo_pool</name>
      <anchorfile>l__mve_8c.html</anchorfile>
      <anchor>47ef06dbc5fdfb506db61338bd91c45d</anchor>
      <arglist></arglist>
    </member>
    <member kind="variable" static="yes">
      <type>static List</type>
      <name>Lpipe_mve_lr_list</name>
      <anchorfile>l__mve_8c.html</anchorfile>
      <anchor>b55d273205ba9ea778a0d8d0b79469cb</anchor>
      <arglist></arglist>
    </member>
  </compound>
  <compound kind="file">
    <name>l_mve.h</name>
    <path>/mnt/hgfs/mkt/Desktop/openimpact/src/Lcode/codegen/Lsoftpipe/</path>
    <filename>l__mve_8h</filename>
    <member kind="function">
      <type>void</type>
      <name>Lpipe_free_mve_info</name>
      <anchorfile>l__mve_8h.html</anchorfile>
      <anchor>78c3c1d55b2de2cc8d2c30fc73669d6b</anchor>
      <arglist>()</arglist>
    </member>
    <member kind="function">
      <type>int</type>
      <name>Lpipe_analyze_lr</name>
      <anchorfile>l__mve_8h.html</anchorfile>
      <anchor>6c1047b81a3439e75fd433210c0f623d</anchor>
      <arglist>(SM_Cb *, int)</arglist>
    </member>
    <member kind="function">
      <type>int</type>
      <name>Lpipe_rreg_transform</name>
      <anchorfile>l__mve_8h.html</anchorfile>
      <anchor>651dce08e6f236c3d39eb2f7f276d6cb</anchor>
      <arglist>(SM_Cb *)</arglist>
    </member>
    <member kind="function">
      <type>void</type>
      <name>Lpipe_fix_live_in_rot</name>
      <anchorfile>l__mve_8h.html</anchorfile>
      <anchor>26395c5756cc1ff986e4a75ffb6c841d</anchor>
      <arglist>(SM_Cb *)</arglist>
    </member>
    <member kind="function">
      <type>void</type>
      <name>Lpipe_fix_live_out_rot</name>
      <anchorfile>l__mve_8h.html</anchorfile>
      <anchor>c587875c384d37d616aeeb12c5a1de11</anchor>
      <arglist>(SM_Cb *, SM_Oper *)</arglist>
    </member>
    <member kind="function">
      <type>void</type>
      <name>Lpipe_mve_transform</name>
      <anchorfile>l__mve_8h.html</anchorfile>
      <anchor>250b56a32209a80f0e5258af0a8a28b1</anchor>
      <arglist>(SM_Cb *, int)</arglist>
    </member>
    <member kind="function">
      <type>void</type>
      <name>Lpipe_fix_live_in</name>
      <anchorfile>l__mve_8h.html</anchorfile>
      <anchor>e28dc02e45f01521186b53b02cc358eb</anchor>
      <arglist>(SM_Cb *)</arglist>
    </member>
    <member kind="function">
      <type>void</type>
      <name>Lpipe_fix_live_out</name>
      <anchorfile>l__mve_8h.html</anchorfile>
      <anchor>0d325bb02fcc4cb8db3c777339617c0c</anchor>
      <arglist>(SM_Cb *, int)</arglist>
    </member>
    <member kind="function">
      <type>int</type>
      <name>Lpipe_find_name</name>
      <anchorfile>l__mve_8h.html</anchorfile>
      <anchor>1d9d11bd30f05f00be63a23da7dec921</anchor>
      <arglist>(SM_Cb *, Lpipe_LRInfo *, Lpipe_MVEInfo *)</arglist>
    </member>
    <member kind="function">
      <type>int</type>
      <name>Lpipe_get_pro_reg</name>
      <anchorfile>l__mve_8h.html</anchorfile>
      <anchor>bc251ae063227ad587bde4091a7a8a7a</anchor>
      <arglist>(SM_Cb *sm_cb, Lpipe_LRInfo *lr_info, Lpipe_MVEInfo *mve_info, int stg_ofst)</arglist>
    </member>
    <member kind="variable">
      <type>L_Alloc_Pool *</type>
      <name>Lpipe_src_mve_pool</name>
      <anchorfile>l__mve_8h.html</anchorfile>
      <anchor>7f8629ad0a52237fabc7b5b90c9a369c</anchor>
      <arglist></arglist>
    </member>
    <member kind="variable">
      <type>L_Alloc_Pool *</type>
      <name>Lpipe_pred_mve_pool</name>
      <anchorfile>l__mve_8h.html</anchorfile>
      <anchor>3c5eb3ef3a82d664ba3a3da10b8e0093</anchor>
      <arglist></arglist>
    </member>
    <member kind="variable">
      <type>L_Alloc_Pool *</type>
      <name>Lpipe_dest_mve_pool</name>
      <anchorfile>l__mve_8h.html</anchorfile>
      <anchor>4fd7a438ad2a7b1efa5d57d7aa1933b2</anchor>
      <arglist></arglist>
    </member>
    <member kind="variable">
      <type>L_Alloc_Pool *</type>
      <name>Lpipe_dest_lr_pool</name>
      <anchorfile>l__mve_8h.html</anchorfile>
      <anchor>d85d3aeb137a2d43ed21f163dc4a88c8</anchor>
      <arglist></arglist>
    </member>
    <member kind="variable">
      <type>L_Alloc_Pool *</type>
      <name>Lpipe_MVEInfo_pool</name>
      <anchorfile>l__mve_8h.html</anchorfile>
      <anchor>47ef06dbc5fdfb506db61338bd91c45d</anchor>
      <arglist></arglist>
    </member>
  </compound>
  <compound kind="file">
    <name>l_pipe_mspec.c</name>
    <path>/mnt/hgfs/mkt/Desktop/openimpact/src/Lcode/codegen/Lsoftpipe/</path>
    <filename>l__pipe__mspec_8c</filename>
  </compound>
  <compound kind="file">
    <name>l_pipe_mspec.h</name>
    <path>/mnt/hgfs/mkt/Desktop/openimpact/src/Lcode/codegen/Lsoftpipe/</path>
    <filename>l__pipe__mspec_8h</filename>
  </compound>
  <compound kind="file">
    <name>l_pipe_rename.c</name>
    <path>/mnt/hgfs/mkt/Desktop/openimpact/src/Lcode/codegen/Lsoftpipe/</path>
    <filename>l__pipe__rename_8c</filename>
    <includes id="l__softpipe__int_8h" name="l_softpipe_int.h" local="yes" imported="no">l_softpipe_int.h</includes>
    <includes id="l__pipe__rename_8h" name="l_pipe_rename.h" local="yes" imported="no">l_pipe_rename.h</includes>
    <includes id="l__pipe__util_8h" name="l_pipe_util.h" local="yes" imported="no">l_pipe_util.h</includes>
    <class kind="struct">_LpipeLiveRange</class>
    <member kind="typedef">
      <type>_LpipeLiveRange</type>
      <name>LpipeLiveRange</name>
      <anchorfile>l__pipe__rename_8c.html</anchorfile>
      <anchor>97ac298743960bdda8547bdccb09b0df</anchor>
      <arglist></arglist>
    </member>
    <member kind="function" static="yes">
      <type>static int</type>
      <name>L_fix_exposed_pred_wrt</name>
      <anchorfile>l__pipe__rename_8c.html</anchorfile>
      <anchor>98c8a789b7d50b4785825a65dfecb6e7</anchor>
      <arglist>(L_Func *fn, L_Cb *pipe_cb, int vreg)</arglist>
    </member>
    <member kind="function">
      <type>void</type>
      <name>R_print_LpipeLiveRange</name>
      <anchorfile>l__pipe__rename_8c.html</anchorfile>
      <anchor>cc5acf4c525363e77a2c0e38dbe31139</anchor>
      <arglist>(LpipeLiveRange *lr)</arglist>
    </member>
    <member kind="function" static="yes">
      <type>static void</type>
      <name>Lpipe_create_rename_comp_code_block</name>
      <anchorfile>l__pipe__rename_8c.html</anchorfile>
      <anchor>1b33fd0f1447aaf5a66338f282158a1e</anchor>
      <arglist>(L_Func *fn, L_Cb *pipe_cb, L_Oper *exit_branch)</arglist>
    </member>
    <member kind="function">
      <type>void</type>
      <name>Lpipe_create_comp_code_blocks</name>
      <anchorfile>l__pipe__rename_8c.html</anchorfile>
      <anchor>fffde84a48fda4a4a865640c272c2220</anchor>
      <arglist>(L_Func *fn, L_Inner_Loop *loop)</arglist>
    </member>
    <member kind="function" static="yes">
      <type>static void</type>
      <name>Lpipe_rename_mac</name>
      <anchorfile>l__pipe__rename_8c.html</anchorfile>
      <anchor>fb7141767cb6ee389d450b56f31cc41c</anchor>
      <arglist>(L_Func *fn, L_Operand *mopd, L_Inner_Loop *inl)</arglist>
    </member>
    <member kind="function">
      <type>int</type>
      <name>Lpipe_rename_defined_macros</name>
      <anchorfile>l__pipe__rename_8c.html</anchorfile>
      <anchor>f87757396ae9cd301b0f54e3871e342a</anchor>
      <arglist>(L_Func *fn, L_Inner_Loop *inl)</arglist>
    </member>
    <member kind="function">
      <type>int</type>
      <name>Lpipe_fix_infinite_lifetimes</name>
      <anchorfile>l__pipe__rename_8c.html</anchorfile>
      <anchor>7dc8d4655a2ef52a3198fafa93d3ab1a</anchor>
      <arglist>(L_Func *fn, L_Cb *cb)</arglist>
    </member>
    <member kind="function">
      <type>void</type>
      <name>Lpipe_rename_disj_vregs_in_loop</name>
      <anchorfile>l__pipe__rename_8c.html</anchorfile>
      <anchor>c5fcbc92b251c0f8652e1aef2f1ea203</anchor>
      <arglist>(L_Func *fn, L_Inner_Loop *loop)</arglist>
    </member>
    <member kind="variable" static="yes">
      <type>static LpipeLiveRange **</type>
      <name>LpipeLiveRanges</name>
      <anchorfile>l__pipe__rename_8c.html</anchorfile>
      <anchor>32c2b4ca644c1642c0d5681fcb3ab0a1</anchor>
      <arglist></arglist>
    </member>
    <member kind="variable" static="yes">
      <type>static int</type>
      <name>numLpipeLiveRange</name>
      <anchorfile>l__pipe__rename_8c.html</anchorfile>
      <anchor>653b8519151fd1e1c24fe18695b3c87f</anchor>
      <arglist></arglist>
    </member>
  </compound>
  <compound kind="file">
    <name>l_pipe_rename.h</name>
    <path>/mnt/hgfs/mkt/Desktop/openimpact/src/Lcode/codegen/Lsoftpipe/</path>
    <filename>l__pipe__rename_8h</filename>
    <member kind="function">
      <type>void</type>
      <name>Lpipe_create_comp_code_blocks</name>
      <anchorfile>l__pipe__rename_8h.html</anchorfile>
      <anchor>fffde84a48fda4a4a865640c272c2220</anchor>
      <arglist>(L_Func *fn, L_Inner_Loop *loop)</arglist>
    </member>
    <member kind="function">
      <type>void</type>
      <name>Lpipe_rename_disj_vregs_in_loop</name>
      <anchorfile>l__pipe__rename_8h.html</anchorfile>
      <anchor>dd152c5db4e8084994802b61e35662b9</anchor>
      <arglist>(L_Func *, L_Inner_Loop *)</arglist>
    </member>
    <member kind="function">
      <type>int</type>
      <name>Lpipe_fix_infinite_lifetimes</name>
      <anchorfile>l__pipe__rename_8h.html</anchorfile>
      <anchor>7dc8d4655a2ef52a3198fafa93d3ab1a</anchor>
      <arglist>(L_Func *fn, L_Cb *cb)</arglist>
    </member>
    <member kind="function">
      <type>int</type>
      <name>Lpipe_rename_defined_macros</name>
      <anchorfile>l__pipe__rename_8h.html</anchorfile>
      <anchor>f87757396ae9cd301b0f54e3871e342a</anchor>
      <arglist>(L_Func *fn, L_Inner_Loop *inl)</arglist>
    </member>
  </compound>
  <compound kind="file">
    <name>l_pipe_sched.c</name>
    <path>/mnt/hgfs/mkt/Desktop/openimpact/src/Lcode/codegen/Lsoftpipe/</path>
    <filename>l__pipe__sched_8c</filename>
    <includes id="l__softpipe__int_8h" name="l_softpipe_int.h" local="yes" imported="no">l_softpipe_int.h</includes>
    <includes id="l__pipe__sched_8h" name="l_pipe_sched.h" local="yes" imported="no">l_pipe_sched.h</includes>
    <includes id="l__mve_8h" name="l_mve.h" local="yes" imported="no">l_mve.h</includes>
    <member kind="define">
      <type>#define</type>
      <name>LARGE_NUMBER</name>
      <anchorfile>l__pipe__sched_8c.html</anchorfile>
      <anchor>72e1d1d4b3243f4e7ec115f199298e34</anchor>
      <arglist></arglist>
    </member>
    <member kind="function" static="yes">
      <type>static void</type>
      <name>Lpipe_sort_MRT_rows</name>
      <anchorfile>l__pipe__sched_8c.html</anchorfile>
      <anchor>8959fddf750d749cd097c00736be43c6</anchor>
      <arglist>(SM_Cb *sm_cb)</arglist>
    </member>
    <member kind="function">
      <type>void</type>
      <name>Lpipe_mark_branch_path_opers</name>
      <anchorfile>l__pipe__sched_8c.html</anchorfile>
      <anchor>1f114627d53f0fcfd8f9b0b943d8750f</anchor>
      <arglist>(SM_Cb *sm_cb, SM_Oper *sm_loop_back_br)</arglist>
    </member>
    <member kind="function" static="yes">
      <type>static void</type>
      <name>Lpipe_do_compact_branch_path_opers</name>
      <anchorfile>l__pipe__sched_8c.html</anchorfile>
      <anchor>22d3a9898c3e6ce8efa572c7ac22c115</anchor>
      <arglist>(SM_Cb *sm_cb, SM_Oper *sm_loop_back_br)</arglist>
    </member>
    <member kind="function" static="yes">
      <type>static void</type>
      <name>Lpipe_optimize_lifetimes</name>
      <anchorfile>l__pipe__sched_8c.html</anchorfile>
      <anchor>6ea245db13fd3e44deea65c66ab25a8c</anchor>
      <arglist>(SM_Cb *sm_cb)</arglist>
    </member>
    <member kind="function" static="yes">
      <type>static void</type>
      <name>Lpipe_compute_slack_time_using_MinDist</name>
      <anchorfile>l__pipe__sched_8c.html</anchorfile>
      <anchor>39d57844abbfbc77f2ddfa8f196c3ff1</anchor>
      <arglist>(SM_Cb *sm_cb)</arglist>
    </member>
    <member kind="function" static="yes">
      <type>static void</type>
      <name>Lpipe_compute_static_priorities</name>
      <anchorfile>l__pipe__sched_8c.html</anchorfile>
      <anchor>3cfd0f1b7275f44a71eed2c6048ad046</anchor>
      <arglist>(SM_Cb *sm_cb)</arglist>
    </member>
    <member kind="function" static="yes">
      <type>static void</type>
      <name>Lpipe_compute_ready_time</name>
      <anchorfile>l__pipe__sched_8c.html</anchorfile>
      <anchor>ef9658a964f588c28e39d72654ddfec8</anchor>
      <arglist>(SM_Cb *sm_cb, SM_Oper *sm_op, int init_ready_time)</arglist>
    </member>
    <member kind="function" static="yes">
      <type>static void</type>
      <name>Lpipe_eject_opers_if_dependence_conflict</name>
      <anchorfile>l__pipe__sched_8c.html</anchorfile>
      <anchor>3958328b214031e4b78c8b9e33cb4a03</anchor>
      <arglist>(SM_Oper *sm_op, int II, int desired_cycle)</arglist>
    </member>
    <member kind="function" static="yes">
      <type>static void</type>
      <name>Lpipe_eject_oper_for_resource_conflict</name>
      <anchorfile>l__pipe__sched_8c.html</anchorfile>
      <anchor>001f35671967efcad3bd24f4fd7aeb13</anchor>
      <arglist>(SM_Oper *sm_op)</arglist>
    </member>
    <member kind="function" static="yes">
      <type>static void</type>
      <name>Lpipe_eject_earlier_branches_and_last_slot</name>
      <anchorfile>l__pipe__sched_8c.html</anchorfile>
      <anchor>ab04f61e01f3019bf9d52500cbf990a0</anchor>
      <arglist>(SM_Cb *sm_cb, int row, int last_slot)</arglist>
    </member>
    <member kind="function" static="yes">
      <type>static void</type>
      <name>Lpipe_eject_opers_if_resource_conflict</name>
      <anchorfile>l__pipe__sched_8c.html</anchorfile>
      <anchor>0eaa7e805044e0128ed051caf6039ffb</anchor>
      <arglist>(SM_Cb *sm_cb, int cycle)</arglist>
    </member>
    <member kind="function">
      <type>void</type>
      <name>Lpipe_sm_terminate</name>
      <anchorfile>l__pipe__sched_8c.html</anchorfile>
      <anchor>dee91af7d1d223f216f3c892e3ec9c0f</anchor>
      <arglist>(SM_Cb *sm_cb)</arglist>
    </member>
    <member kind="function">
      <type>int</type>
      <name>Lpipe_sm_schedule</name>
      <anchorfile>l__pipe__sched_8c.html</anchorfile>
      <anchor>01e3114cfbc096dc1fce4b7ee4039a61</anchor>
      <arglist>(SM_Cb *sm_cb, int II, SM_Oper *loop_back_br, int budget)</arglist>
    </member>
    <member kind="function">
      <type>void</type>
      <name>Lpipe_sm_commit</name>
      <anchorfile>l__pipe__sched_8c.html</anchorfile>
      <anchor>9d49fd9203ffb941ebcc7e0d5dca5140</anchor>
      <arglist>(SM_Cb *sm_cb)</arglist>
    </member>
    <member kind="variable" static="yes">
      <type>static Queue *</type>
      <name>Lpipe_ready_queue</name>
      <anchorfile>l__pipe__sched_8c.html</anchorfile>
      <anchor>d3571bda1f4af9f3c27d5c33ba538a2c</anchor>
      <arglist></arglist>
    </member>
  </compound>
  <compound kind="file">
    <name>l_pipe_sched.h</name>
    <path>/mnt/hgfs/mkt/Desktop/openimpact/src/Lcode/codegen/Lsoftpipe/</path>
    <filename>l__pipe__sched_8h</filename>
    <member kind="define">
      <type>#define</type>
      <name>IMPOSSIBLE_SLACK</name>
      <anchorfile>l__pipe__sched_8h.html</anchorfile>
      <anchor>c5c094a855c2acb6fc2483dfd24f5bf6</anchor>
      <arglist></arglist>
    </member>
    <member kind="function">
      <type>int</type>
      <name>Lpipe_sm_schedule</name>
      <anchorfile>l__pipe__sched_8h.html</anchorfile>
      <anchor>4a201d1e1f411d624841e0c211d4607f</anchor>
      <arglist>(SM_Cb *, int, SM_Oper *, int)</arglist>
    </member>
    <member kind="function">
      <type>void</type>
      <name>Lpipe_sm_terminate</name>
      <anchorfile>l__pipe__sched_8h.html</anchorfile>
      <anchor>c15a13771ee9ba2bcc09154f1dbed4f3</anchor>
      <arglist>(SM_Cb *)</arglist>
    </member>
    <member kind="function">
      <type>void</type>
      <name>Lpipe_sm_commit</name>
      <anchorfile>l__pipe__sched_8h.html</anchorfile>
      <anchor>dd7366875dda49d931f80947bf3a2dda</anchor>
      <arglist>(SM_Cb *)</arglist>
    </member>
    <member kind="function">
      <type>void</type>
      <name>Lpipe_mark_branch_path_opers</name>
      <anchorfile>l__pipe__sched_8h.html</anchorfile>
      <anchor>17a1a52d06eb95a93472c75b7d316989</anchor>
      <arglist>(SM_Cb *, SM_Oper *)</arglist>
    </member>
  </compound>
  <compound kind="file">
    <name>l_pipe_sync.c</name>
    <path>/mnt/hgfs/mkt/Desktop/openimpact/src/Lcode/codegen/Lsoftpipe/</path>
    <filename>l__pipe__sync_8c</filename>
    <includes id="l__softpipe__int_8h" name="l_softpipe_int.h" local="yes" imported="no">l_softpipe_int.h</includes>
    <includes id="l__pipe__sync_8h" name="l_pipe_sync.h" local="yes" imported="no">l_pipe_sync.h</includes>
    <member kind="function">
      <type>void</type>
      <name>Lpipe_adjust_syncs_for_modulo_sched</name>
      <anchorfile>l__pipe__sync_8c.html</anchorfile>
      <anchor>0bef7d95bb30bcb052fb9d8064a32d58</anchor>
      <arglist>(SM_Cb *sm_cb)</arglist>
    </member>
    <member kind="function">
      <type>void</type>
      <name>Lpipe_adjust_syncs_for_prologue</name>
      <anchorfile>l__pipe__sync_8c.html</anchorfile>
      <anchor>d9b8990e5b0abcad84d75eea67f9326e</anchor>
      <arglist>(void)</arglist>
    </member>
    <member kind="function">
      <type>void</type>
      <name>Lpipe_adjust_syncs_for_epilogue</name>
      <anchorfile>l__pipe__sync_8c.html</anchorfile>
      <anchor>a2557b5b8cba1b69a2ea00f31d0788c9</anchor>
      <arglist>(L_Cb *epilogue_cb)</arglist>
    </member>
  </compound>
  <compound kind="file">
    <name>l_pipe_sync.h</name>
    <path>/mnt/hgfs/mkt/Desktop/openimpact/src/Lcode/codegen/Lsoftpipe/</path>
    <filename>l__pipe__sync_8h</filename>
    <member kind="function">
      <type>void</type>
      <name>Lpipe_adjust_syncs_for_modulo_sched</name>
      <anchorfile>l__pipe__sync_8h.html</anchorfile>
      <anchor>b564d1a85ea22d364eabbe11970d3f24</anchor>
      <arglist>(SM_Cb *)</arglist>
    </member>
    <member kind="function">
      <type>void</type>
      <name>Lpipe_adjust_syncs_for_prologue</name>
      <anchorfile>l__pipe__sync_8h.html</anchorfile>
      <anchor>d9b8990e5b0abcad84d75eea67f9326e</anchor>
      <arglist>(void)</arglist>
    </member>
    <member kind="function">
      <type>void</type>
      <name>Lpipe_adjust_syncs_for_epilogue</name>
      <anchorfile>l__pipe__sync_8h.html</anchorfile>
      <anchor>2e04b79e2a0ce467271ebbfee10b7a9b</anchor>
      <arglist>(L_Cb *)</arglist>
    </member>
  </compound>
  <compound kind="file">
    <name>l_pipe_util.c</name>
    <path>/mnt/hgfs/mkt/Desktop/openimpact/src/Lcode/codegen/Lsoftpipe/</path>
    <filename>l__pipe__util_8c</filename>
    <includes id="l__softpipe__int_8h" name="l_softpipe_int.h" local="yes" imported="no">l_softpipe_int.h</includes>
    <member kind="function">
      <type>void</type>
      <name>L_read_parm_lpipe</name>
      <anchorfile>l__pipe__util_8c.html</anchorfile>
      <anchor>6f5aa25c2a95b94ee73ac1b754207d70</anchor>
      <arglist>(Parm_Parse_Info *ppi)</arglist>
    </member>
    <member kind="function">
      <type>Queue *</type>
      <name>Q_create_queue</name>
      <anchorfile>l__pipe__util_8c.html</anchorfile>
      <anchor>cdc1d20712bda5896582534b68c4acbf</anchor>
      <arglist>()</arglist>
    </member>
    <member kind="function">
      <type>void</type>
      <name>Q_reinit_queue</name>
      <anchorfile>l__pipe__util_8c.html</anchorfile>
      <anchor>df1c12ac44e95b30e76ae8af8d4b7e4c</anchor>
      <arglist>(Queue *queue)</arglist>
    </member>
    <member kind="function">
      <type>void</type>
      <name>Q_delete_queue</name>
      <anchorfile>l__pipe__util_8c.html</anchorfile>
      <anchor>942148e69d79ed6f032311fc5f673ec1</anchor>
      <arglist>(Queue *queue)</arglist>
    </member>
    <member kind="function">
      <type>void</type>
      <name>Q_priority_enqueue_increasing</name>
      <anchorfile>l__pipe__util_8c.html</anchorfile>
      <anchor>5d0d8ff30543d84c18044af612092e86</anchor>
      <arglist>(Queue *queue, SM_Oper *oper, int priority)</arglist>
    </member>
    <member kind="function">
      <type>void</type>
      <name>Q_priority_enqueue_decreasing</name>
      <anchorfile>l__pipe__util_8c.html</anchorfile>
      <anchor>8fcbbb02452d1ca7f1fb27f24bf23fc9</anchor>
      <arglist>(Queue *queue, SM_Oper *oper, int priority)</arglist>
    </member>
    <member kind="function">
      <type>SM_Oper *</type>
      <name>Q_dequeue</name>
      <anchorfile>l__pipe__util_8c.html</anchorfile>
      <anchor>133e3cf36bdcb5c8bd8a22cf3d73dc36</anchor>
      <arglist>(Queue *queue)</arglist>
    </member>
    <member kind="function">
      <type>SM_Oper *</type>
      <name>Q_peek_tail</name>
      <anchorfile>l__pipe__util_8c.html</anchorfile>
      <anchor>97fa4ed0e8f32c08ef93c041c69c542d</anchor>
      <arglist>(Queue *queue)</arglist>
    </member>
    <member kind="function">
      <type>SM_Oper *</type>
      <name>Q_peek_head</name>
      <anchorfile>l__pipe__util_8c.html</anchorfile>
      <anchor>c31c6a79d7e89f946aa510fb48beeb21</anchor>
      <arglist>(Queue *queue)</arglist>
    </member>
    <member kind="function">
      <type>int</type>
      <name>Lpipe_reordered_anti_dependent_ops</name>
      <anchorfile>l__pipe__util_8c.html</anchorfile>
      <anchor>af2a384e21644ca71a0c7f4a3a524d8d</anchor>
      <arglist>(SM_Oper *oper, SM_Oper *next_oper)</arglist>
    </member>
    <member kind="function">
      <type>void</type>
      <name>Lpipe_ignore_self_dependences</name>
      <anchorfile>l__pipe__util_8c.html</anchorfile>
      <anchor>fe0fa3d6120b61b121fc43231a6d890d</anchor>
      <arglist>(SM_Cb *sm_cb)</arglist>
    </member>
    <member kind="function">
      <type>L_Oper *</type>
      <name>Lpipe_find_loop_back_br</name>
      <anchorfile>l__pipe__util_8c.html</anchorfile>
      <anchor>a00a3d794df406f15d3d85b20f483941</anchor>
      <arglist>(L_Func *fn, L_Cb *header_cb)</arglist>
    </member>
    <member kind="function">
      <type>void</type>
      <name>Lpipe_remove_branches</name>
      <anchorfile>l__pipe__util_8c.html</anchorfile>
      <anchor>c7e6b4cf559095203372239c9544b2a8</anchor>
      <arglist>(L_Func *fn, int unroll)</arglist>
    </member>
    <member kind="function">
      <type>void</type>
      <name>Lpipe_cb_set_iter</name>
      <anchorfile>l__pipe__util_8c.html</anchorfile>
      <anchor>d0e4be247037cc75bcca0eeebb56dc71</anchor>
      <arglist>(SM_Cb *sm_cb)</arglist>
    </member>
    <member kind="function">
      <type>void</type>
      <name>Lpipe_cb_set_issue_time</name>
      <anchorfile>l__pipe__util_8c.html</anchorfile>
      <anchor>0b7f0cae0da09da3e5d8b982be9873e0</anchor>
      <arglist>(SM_Cb *sm_cb)</arglist>
    </member>
    <member kind="function">
      <type>L_Oper *</type>
      <name>Lpipe_gen_uncond_pred_define</name>
      <anchorfile>l__pipe__util_8c.html</anchorfile>
      <anchor>af4d8d01138767db21c4b576f99cd96d</anchor>
      <arglist>(L_Operand *pred0, L_Operand *pred1, L_Operand *dest, int opc)</arglist>
    </member>
    <member kind="function">
      <type>void</type>
      <name>Lpipe_create_uncond_pred_defines</name>
      <anchorfile>l__pipe__util_8c.html</anchorfile>
      <anchor>8e12dca1f06e9f8f224a39473ba1bb81</anchor>
      <arglist>(L_Func *fn)</arglist>
    </member>
    <member kind="function">
      <type>void</type>
      <name>Lpipe_create_start_stop_nodes</name>
      <anchorfile>l__pipe__util_8c.html</anchorfile>
      <anchor>4d45e5623b3ff132a813e2a9adf69150</anchor>
      <arglist>(SM_Cb *sm_cb)</arglist>
    </member>
    <member kind="function">
      <type>void</type>
      <name>Lpipe_delete_start_stop_nodes</name>
      <anchorfile>l__pipe__util_8c.html</anchorfile>
      <anchor>e21d164bb9d5070bbf1f2250d5d27501</anchor>
      <arglist>(SM_Cb *sm_cb)</arglist>
    </member>
    <member kind="function">
      <type>void</type>
      <name>Lpipe_print_cb_schedule</name>
      <anchorfile>l__pipe__util_8c.html</anchorfile>
      <anchor>b586f12896d4906e0e7015d4ea36b61e</anchor>
      <arglist>(FILE *file, L_Func *fn, SM_Cb *sm_cb)</arglist>
    </member>
    <member kind="function">
      <type>void</type>
      <name>Lpipe_create_defines</name>
      <anchorfile>l__pipe__util_8c.html</anchorfile>
      <anchor>689850f1f56bc56b23c950658e5aba87</anchor>
      <arglist>(L_Func *fn)</arglist>
    </member>
    <member kind="function">
      <type>void</type>
      <name>Lpipe_delete_defines</name>
      <anchorfile>l__pipe__util_8c.html</anchorfile>
      <anchor>e541b9878cec8748cd7882ee97c7ba4f</anchor>
      <arglist>(L_Func *fn)</arglist>
    </member>
    <member kind="function">
      <type>void</type>
      <name>Lpipe_print_cyclic_stats</name>
      <anchorfile>l__pipe__util_8c.html</anchorfile>
      <anchor>1717d05a7fa2b065c482d5476745d580</anchor>
      <arglist>(FILE *gen_statfile, L_Cb *header_cb, Softpipe_MinII *MinII, int ii, int tries, int schedule_length, int loop_dep_height, int stage_count, int unroll, int theta, int num_oper, int branch_count, L_Inner_Loop *loop)</arglist>
    </member>
    <member kind="function">
      <type>L_Oper *</type>
      <name>Lpipe_gen_mov_consuming_operands</name>
      <anchorfile>l__pipe__util_8c.html</anchorfile>
      <anchor>ee779950f38ef40325d91a06a673d9ae</anchor>
      <arglist>(L_Operand *dest, L_Operand *src)</arglist>
    </member>
    <member kind="function">
      <type>void</type>
      <name>Lpipe_move_int_parm_regs</name>
      <anchorfile>l__pipe__util_8c.html</anchorfile>
      <anchor>b17b99684ef3a58402ade4ddb0129407</anchor>
      <arglist>(L_Func *fn)</arglist>
    </member>
    <member kind="function">
      <type>int</type>
      <name>Lpipe_can_create_fallthru_cb</name>
      <anchorfile>l__pipe__util_8c.html</anchorfile>
      <anchor>bc942e4e2d34aabcfb238a6102ac9618</anchor>
      <arglist>(L_Cb *cb)</arglist>
    </member>
    <member kind="function">
      <type>void</type>
      <name>Lpipe_reduce_defines</name>
      <anchorfile>l__pipe__util_8c.html</anchorfile>
      <anchor>08dd82ece7672006298b40e75a866e6b</anchor>
      <arglist>(L_Func *fn)</arglist>
    </member>
    <member kind="function">
      <type>void</type>
      <name>Lpipe_insert_non_rr_defines</name>
      <anchorfile>l__pipe__util_8c.html</anchorfile>
      <anchor>9746794fce95842da4036ab2dea2779a</anchor>
      <arglist>(L_Func *fn)</arglist>
    </member>
    <member kind="variable">
      <type>L_Alloc_Pool *</type>
      <name>Queue_pool</name>
      <anchorfile>l__pipe__util_8c.html</anchorfile>
      <anchor>4a9d1f680824f7426e01b910d4cc2b03</anchor>
      <arglist></arglist>
    </member>
    <member kind="variable">
      <type>L_Alloc_Pool *</type>
      <name>Qnode_pool</name>
      <anchorfile>l__pipe__util_8c.html</anchorfile>
      <anchor>b2b35dba38efd9d6e53512e38d014729</anchor>
      <arglist></arglist>
    </member>
    <member kind="variable">
      <type>int</type>
      <name>Lpipe_do_induction_reversal</name>
      <anchorfile>l__pipe__util_8c.html</anchorfile>
      <anchor>fe93aca3da93bf91019df7f7726355fb</anchor>
      <arglist></arglist>
    </member>
    <member kind="variable">
      <type>int</type>
      <name>Lpipe_check_loops_in_phase1</name>
      <anchorfile>l__pipe__util_8c.html</anchorfile>
      <anchor>6535802484914fea24cb0c4267280e86</anchor>
      <arglist></arglist>
    </member>
    <member kind="variable">
      <type>char *</type>
      <name>Lpipe_schema_name</name>
      <anchorfile>l__pipe__util_8c.html</anchorfile>
      <anchor>659205866ba47e73cc85c2376ecd1985</anchor>
      <arglist></arglist>
    </member>
    <member kind="variable">
      <type>int</type>
      <name>Lpipe_schema</name>
      <anchorfile>l__pipe__util_8c.html</anchorfile>
      <anchor>9929661b757af9845e6325f4752a7d87</anchor>
      <arglist></arglist>
    </member>
    <member kind="variable">
      <type>int</type>
      <name>Lpipe_backward_sched</name>
      <anchorfile>l__pipe__util_8c.html</anchorfile>
      <anchor>1ed5180823f3e497dbc3becd928690f5</anchor>
      <arglist></arglist>
    </member>
    <member kind="variable">
      <type>float</type>
      <name>Lpipe_budget_ratio</name>
      <anchorfile>l__pipe__util_8c.html</anchorfile>
      <anchor>a50c2551f678955eabc6e79793268ffa</anchor>
      <arglist></arglist>
    </member>
    <member kind="variable">
      <type>int</type>
      <name>Lpipe_min_ii</name>
      <anchorfile>l__pipe__util_8c.html</anchorfile>
      <anchor>05fca7cb5ae9510942db3f6c0307a355</anchor>
      <arglist></arglist>
    </member>
    <member kind="variable">
      <type>int</type>
      <name>Lpipe_max_ii</name>
      <anchorfile>l__pipe__util_8c.html</anchorfile>
      <anchor>971e2f43a9bcf87d02ca3adb1f632358</anchor>
      <arglist></arglist>
    </member>
    <member kind="variable">
      <type>int</type>
      <name>Lpipe_max_stages</name>
      <anchorfile>l__pipe__util_8c.html</anchorfile>
      <anchor>d36c8bdff7baceb8d1cb7853c908bf03</anchor>
      <arglist></arglist>
    </member>
    <member kind="variable">
      <type>int</type>
      <name>Lpipe_max_tries</name>
      <anchorfile>l__pipe__util_8c.html</anchorfile>
      <anchor>b20efb4ff31868be5a5b0c3039351158</anchor>
      <arglist></arglist>
    </member>
    <member kind="variable">
      <type>int</type>
      <name>Lpipe_fixed_slots_for_branches</name>
      <anchorfile>l__pipe__util_8c.html</anchorfile>
      <anchor>5dfc4a9a1248286c8e15ad45456ddb90</anchor>
      <arglist></arglist>
    </member>
    <member kind="variable">
      <type>int</type>
      <name>Lpipe_do_only_postpass_steps</name>
      <anchorfile>l__pipe__util_8c.html</anchorfile>
      <anchor>414f010c3e4a932c6061bb593e7945b0</anchor>
      <arglist></arglist>
    </member>
    <member kind="variable">
      <type>int</type>
      <name>Lpipe_compact_branch_path_opers</name>
      <anchorfile>l__pipe__util_8c.html</anchorfile>
      <anchor>86f23e88bb2635324fa313d64bb50971</anchor>
      <arglist></arglist>
    </member>
    <member kind="variable">
      <type>int</type>
      <name>Lpipe_sort_mrt_rows</name>
      <anchorfile>l__pipe__util_8c.html</anchorfile>
      <anchor>1fd08397fdb9c8be171e9e45a1d811aa</anchor>
      <arglist></arglist>
    </member>
    <member kind="variable">
      <type>int</type>
      <name>Lpipe_combine_cbs</name>
      <anchorfile>l__pipe__util_8c.html</anchorfile>
      <anchor>33aa768a71f08754b1273f95c0cf1d2f</anchor>
      <arglist></arglist>
    </member>
    <member kind="variable">
      <type>int</type>
      <name>Lpipe_debug</name>
      <anchorfile>l__pipe__util_8c.html</anchorfile>
      <anchor>ad83dd0fea79c90886829873235d97be</anchor>
      <arglist></arglist>
    </member>
    <member kind="variable">
      <type>int</type>
      <name>Lpipe_debug_use_cb_bounds</name>
      <anchorfile>l__pipe__util_8c.html</anchorfile>
      <anchor>ab570eb13a3e6d913d45c8a3b85fad84</anchor>
      <arglist></arglist>
    </member>
    <member kind="variable">
      <type>int</type>
      <name>Lpipe_debug_lower_cb_bound</name>
      <anchorfile>l__pipe__util_8c.html</anchorfile>
      <anchor>4ebe475f1a840fbaa32b10d000fd7f70</anchor>
      <arglist></arglist>
    </member>
    <member kind="variable">
      <type>int</type>
      <name>Lpipe_debug_upper_cb_bound</name>
      <anchorfile>l__pipe__util_8c.html</anchorfile>
      <anchor>9142d5510348648db05b36ee12f6058e</anchor>
      <arglist></arglist>
    </member>
    <member kind="variable">
      <type>int</type>
      <name>Lpipe_print_statistics</name>
      <anchorfile>l__pipe__util_8c.html</anchorfile>
      <anchor>ab2140575cdf9b3526e6da5ea095a784</anchor>
      <arglist></arglist>
    </member>
    <member kind="variable">
      <type>int</type>
      <name>Lpipe_print_iteration_schedule</name>
      <anchorfile>l__pipe__util_8c.html</anchorfile>
      <anchor>f65026de9f77648de4df3b4f4ec74417</anchor>
      <arglist></arglist>
    </member>
    <member kind="variable">
      <type>int</type>
      <name>Lpipe_print_schedules_for_debug</name>
      <anchorfile>l__pipe__util_8c.html</anchorfile>
      <anchor>0b64713f1a54406156072dd81814ea25</anchor>
      <arglist></arglist>
    </member>
    <member kind="variable">
      <type>int</type>
      <name>Lpipe_print_mve_summary</name>
      <anchorfile>l__pipe__util_8c.html</anchorfile>
      <anchor>39d6ce980c9450094cb1c9372292b9ac</anchor>
      <arglist></arglist>
    </member>
    <member kind="variable">
      <type>int</type>
      <name>Lpipe_compute_loop_reg_pressure</name>
      <anchorfile>l__pipe__util_8c.html</anchorfile>
      <anchor>05b449c92ca8c59d61db6054cd56e7ee</anchor>
      <arglist></arglist>
    </member>
    <member kind="variable">
      <type>int</type>
      <name>Lpipe_add_spill_attributes</name>
      <anchorfile>l__pipe__util_8c.html</anchorfile>
      <anchor>e2d1368da174d6c3c8b7ec7d1cbbf44e</anchor>
      <arglist></arglist>
    </member>
    <member kind="variable">
      <type>int</type>
      <name>Lpipe_dump_dot</name>
      <anchorfile>l__pipe__util_8c.html</anchorfile>
      <anchor>2b86b75abb1973402ba2363621a03bb3</anchor>
      <arglist></arglist>
    </member>
  </compound>
  <compound kind="file">
    <name>l_pipe_util.h</name>
    <path>/mnt/hgfs/mkt/Desktop/openimpact/src/Lcode/codegen/Lsoftpipe/</path>
    <filename>l__pipe__util_8h</filename>
    <class kind="struct">Qnode</class>
    <class kind="struct">Queue</class>
    <member kind="define">
      <type>#define</type>
      <name>REM_LOOP</name>
      <anchorfile>l__pipe__util_8h.html</anchorfile>
      <anchor>3225a520483e53a39cab55104dcd0e5a</anchor>
      <arglist></arglist>
    </member>
    <member kind="define">
      <type>#define</type>
      <name>MULTI_EPI</name>
      <anchorfile>l__pipe__util_8h.html</anchorfile>
      <anchor>50a9671342717fc27f66fbb4178f4827</anchor>
      <arglist></arglist>
    </member>
    <member kind="define">
      <type>#define</type>
      <name>MULTI_EPI_ROT_REG</name>
      <anchorfile>l__pipe__util_8h.html</anchorfile>
      <anchor>82cbec0da6d1aaa4fc4326173c92f608</anchor>
      <arglist></arglist>
    </member>
    <member kind="define">
      <type>#define</type>
      <name>KERNEL_ONLY</name>
      <anchorfile>l__pipe__util_8h.html</anchorfile>
      <anchor>7c9c77d70fb6ca9475c561bedd2b6b8a</anchor>
      <arglist></arglist>
    </member>
    <member kind="define">
      <type>#define</type>
      <name>IS_MDES_FLAG_CBR</name>
      <anchorfile>l__pipe__util_8h.html</anchorfile>
      <anchor>d76b9120ce77483f097df3cd547660a2</anchor>
      <arglist>(a)</arglist>
    </member>
    <member kind="define">
      <type>#define</type>
      <name>IS_MDES_FLAG_BR</name>
      <anchorfile>l__pipe__util_8h.html</anchorfile>
      <anchor>ca99545f168fc106bfd5b1d32a442dd2</anchor>
      <arglist>(a)</arglist>
    </member>
    <member kind="function">
      <type>void</type>
      <name>L_read_parm_lpipe</name>
      <anchorfile>l__pipe__util_8h.html</anchorfile>
      <anchor>38ba5bc4aa0dd1d12f1f16934a3137b1</anchor>
      <arglist>(Parm_Parse_Info *)</arglist>
    </member>
    <member kind="function">
      <type>Queue *</type>
      <name>Q_create_queue</name>
      <anchorfile>l__pipe__util_8h.html</anchorfile>
      <anchor>cdc1d20712bda5896582534b68c4acbf</anchor>
      <arglist>()</arglist>
    </member>
    <member kind="function">
      <type>void</type>
      <name>Q_reinit_queue</name>
      <anchorfile>l__pipe__util_8h.html</anchorfile>
      <anchor>8a2a321420befe6a29c8ad803115cdc0</anchor>
      <arglist>(Queue *)</arglist>
    </member>
    <member kind="function">
      <type>void</type>
      <name>Q_delete_queue</name>
      <anchorfile>l__pipe__util_8h.html</anchorfile>
      <anchor>11ae875063d836b46f834dde6210bc16</anchor>
      <arglist>(Queue *)</arglist>
    </member>
    <member kind="function">
      <type>void</type>
      <name>Q_priority_enqueue_increasing</name>
      <anchorfile>l__pipe__util_8h.html</anchorfile>
      <anchor>5ae752c73b9349546549ee048228a045</anchor>
      <arglist>(Queue *, SM_Oper *, int)</arglist>
    </member>
    <member kind="function">
      <type>void</type>
      <name>Q_priority_enqueue_decreasing</name>
      <anchorfile>l__pipe__util_8h.html</anchorfile>
      <anchor>e3aff464ee7717b9aa8908499dcd3f5f</anchor>
      <arglist>(Queue *, SM_Oper *, int)</arglist>
    </member>
    <member kind="function">
      <type>SM_Oper *</type>
      <name>Q_dequeue</name>
      <anchorfile>l__pipe__util_8h.html</anchorfile>
      <anchor>7b9451e5338281ce08f9f3900b0da2aa</anchor>
      <arglist>(Queue *)</arglist>
    </member>
    <member kind="function">
      <type>SM_Oper *</type>
      <name>Q_peek_tail</name>
      <anchorfile>l__pipe__util_8h.html</anchorfile>
      <anchor>1246c0756b8a3deb050a6bd75d6e5fa1</anchor>
      <arglist>(Queue *)</arglist>
    </member>
    <member kind="function">
      <type>SM_Oper *</type>
      <name>Q_peek_head</name>
      <anchorfile>l__pipe__util_8h.html</anchorfile>
      <anchor>17f2ed633a7771cbc82b5df1aea6c12d</anchor>
      <arglist>(Queue *)</arglist>
    </member>
    <member kind="function">
      <type>int</type>
      <name>Lpipe_reordered_anti_dependent_ops</name>
      <anchorfile>l__pipe__util_8h.html</anchorfile>
      <anchor>7b60097f302ea9ca9d9c2bda114cd370</anchor>
      <arglist>(SM_Oper *, SM_Oper *)</arglist>
    </member>
    <member kind="function">
      <type>void</type>
      <name>Lpipe_ignore_self_dependences</name>
      <anchorfile>l__pipe__util_8h.html</anchorfile>
      <anchor>307da7f36b0c185cc93cccaabef4a233</anchor>
      <arglist>(SM_Cb *)</arglist>
    </member>
    <member kind="function">
      <type>void</type>
      <name>Lpipe_ignore_branch_flow_dependences</name>
      <anchorfile>l__pipe__util_8h.html</anchorfile>
      <anchor>ed65ff86dd8fa993bda749213be99abf</anchor>
      <arglist>(SM_Cb *)</arglist>
    </member>
    <member kind="function">
      <type>L_Oper *</type>
      <name>Lpipe_find_loop_back_br</name>
      <anchorfile>l__pipe__util_8h.html</anchorfile>
      <anchor>205159e237371c2666b10787e5ecdb95</anchor>
      <arglist>(L_Func *, L_Cb *)</arglist>
    </member>
    <member kind="function">
      <type>void</type>
      <name>Lpipe_remove_branches</name>
      <anchorfile>l__pipe__util_8h.html</anchorfile>
      <anchor>b00f5d107ce583fe74323bce6d807120</anchor>
      <arglist>(L_Func *, int)</arglist>
    </member>
    <member kind="function">
      <type>void</type>
      <name>Lpipe_cb_set_issue_time</name>
      <anchorfile>l__pipe__util_8h.html</anchorfile>
      <anchor>78c301317333e39c1f990dd753b7702a</anchor>
      <arglist>(SM_Cb *)</arglist>
    </member>
    <member kind="function">
      <type>void</type>
      <name>Lpipe_cb_set_iter</name>
      <anchorfile>l__pipe__util_8h.html</anchorfile>
      <anchor>ec1fbdb678fe723bd9872d734ef7647f</anchor>
      <arglist>(SM_Cb *)</arglist>
    </member>
    <member kind="function">
      <type>L_Oper *</type>
      <name>Lpipe_gen_uncond_pred_define</name>
      <anchorfile>l__pipe__util_8h.html</anchorfile>
      <anchor>9df034fd9bbb4661af51542996abeacc</anchor>
      <arglist>(L_Operand *, L_Operand *, L_Operand *, int)</arglist>
    </member>
    <member kind="function">
      <type>void</type>
      <name>Lpipe_create_uncond_pred_defines</name>
      <anchorfile>l__pipe__util_8h.html</anchorfile>
      <anchor>d5047dd886071898fb70d91fb6f80e99</anchor>
      <arglist>(L_Func *)</arglist>
    </member>
    <member kind="function">
      <type>void</type>
      <name>Lpipe_move_int_parm_regs</name>
      <anchorfile>l__pipe__util_8h.html</anchorfile>
      <anchor>bc532cb244121a0e4179f7d989a94f9e</anchor>
      <arglist>(L_Func *)</arglist>
    </member>
    <member kind="function">
      <type>void</type>
      <name>Lpipe_create_defines</name>
      <anchorfile>l__pipe__util_8h.html</anchorfile>
      <anchor>ffea3cc897c410d1404e778a2337b577</anchor>
      <arglist>(L_Func *)</arglist>
    </member>
    <member kind="function">
      <type>void</type>
      <name>Lpipe_delete_defines</name>
      <anchorfile>l__pipe__util_8h.html</anchorfile>
      <anchor>09d06183dc0f6729b321b7e2c3c60699</anchor>
      <arglist>(L_Func *)</arglist>
    </member>
    <member kind="function">
      <type>void</type>
      <name>Lpipe_reduce_defines</name>
      <anchorfile>l__pipe__util_8h.html</anchorfile>
      <anchor>a449ba296611b2121b42074e012392ae</anchor>
      <arglist>(L_Func *)</arglist>
    </member>
    <member kind="function">
      <type>L_Oper *</type>
      <name>Lpipe_gen_mov_consuming_operands</name>
      <anchorfile>l__pipe__util_8h.html</anchorfile>
      <anchor>da3a3357b8fde887ab841682e5ee9151</anchor>
      <arglist>(L_Operand *, L_Operand *)</arglist>
    </member>
    <member kind="function">
      <type>int</type>
      <name>Lpipe_can_create_fallthru_cb</name>
      <anchorfile>l__pipe__util_8h.html</anchorfile>
      <anchor>b7d0f47ef87d55f7475d9445aecd6fb4</anchor>
      <arglist>(L_Cb *)</arglist>
    </member>
    <member kind="function">
      <type>void</type>
      <name>Lpipe_create_start_stop_nodes</name>
      <anchorfile>l__pipe__util_8h.html</anchorfile>
      <anchor>a967bed10a5deec4a8f2949828656c72</anchor>
      <arglist>(SM_Cb *)</arglist>
    </member>
    <member kind="function">
      <type>void</type>
      <name>Lpipe_delete_start_stop_nodes</name>
      <anchorfile>l__pipe__util_8h.html</anchorfile>
      <anchor>2d5b9cca606c59bc37fc608efa6284c2</anchor>
      <arglist>(SM_Cb *)</arglist>
    </member>
    <member kind="function">
      <type>void</type>
      <name>Lpipe_print_cb_schedule</name>
      <anchorfile>l__pipe__util_8h.html</anchorfile>
      <anchor>a981448442d5ada28f8005bc460a55e4</anchor>
      <arglist>(FILE *, L_Func *, SM_Cb *)</arglist>
    </member>
    <member kind="function">
      <type>void</type>
      <name>Lpipe_print_cyclic_stats</name>
      <anchorfile>l__pipe__util_8h.html</anchorfile>
      <anchor>1717d05a7fa2b065c482d5476745d580</anchor>
      <arglist>(FILE *gen_statfile, L_Cb *header_cb, Softpipe_MinII *MinII, int ii, int tries, int schedule_length, int loop_dep_height, int stage_count, int unroll, int theta, int num_oper, int branch_count, L_Inner_Loop *loop)</arglist>
    </member>
    <member kind="variable">
      <type>int</type>
      <name>Lpipe_do_induction_reversal</name>
      <anchorfile>l__pipe__util_8h.html</anchorfile>
      <anchor>fe93aca3da93bf91019df7f7726355fb</anchor>
      <arglist></arglist>
    </member>
    <member kind="variable">
      <type>int</type>
      <name>Lpipe_check_loops_in_phase1</name>
      <anchorfile>l__pipe__util_8h.html</anchorfile>
      <anchor>6535802484914fea24cb0c4267280e86</anchor>
      <arglist></arglist>
    </member>
    <member kind="variable">
      <type>char *</type>
      <name>Lpipe_schema_name</name>
      <anchorfile>l__pipe__util_8h.html</anchorfile>
      <anchor>659205866ba47e73cc85c2376ecd1985</anchor>
      <arglist></arglist>
    </member>
    <member kind="variable">
      <type>int</type>
      <name>Lpipe_schema</name>
      <anchorfile>l__pipe__util_8h.html</anchorfile>
      <anchor>9929661b757af9845e6325f4752a7d87</anchor>
      <arglist></arglist>
    </member>
    <member kind="variable">
      <type>int</type>
      <name>Lpipe_backward_sched</name>
      <anchorfile>l__pipe__util_8h.html</anchorfile>
      <anchor>1ed5180823f3e497dbc3becd928690f5</anchor>
      <arglist></arglist>
    </member>
    <member kind="variable">
      <type>float</type>
      <name>Lpipe_budget_ratio</name>
      <anchorfile>l__pipe__util_8h.html</anchorfile>
      <anchor>a50c2551f678955eabc6e79793268ffa</anchor>
      <arglist></arglist>
    </member>
    <member kind="variable">
      <type>int</type>
      <name>Lpipe_min_ii</name>
      <anchorfile>l__pipe__util_8h.html</anchorfile>
      <anchor>05fca7cb5ae9510942db3f6c0307a355</anchor>
      <arglist></arglist>
    </member>
    <member kind="variable">
      <type>int</type>
      <name>Lpipe_max_ii</name>
      <anchorfile>l__pipe__util_8h.html</anchorfile>
      <anchor>971e2f43a9bcf87d02ca3adb1f632358</anchor>
      <arglist></arglist>
    </member>
    <member kind="variable">
      <type>int</type>
      <name>Lpipe_max_stages</name>
      <anchorfile>l__pipe__util_8h.html</anchorfile>
      <anchor>d36c8bdff7baceb8d1cb7853c908bf03</anchor>
      <arglist></arglist>
    </member>
    <member kind="variable">
      <type>int</type>
      <name>Lpipe_max_tries</name>
      <anchorfile>l__pipe__util_8h.html</anchorfile>
      <anchor>b20efb4ff31868be5a5b0c3039351158</anchor>
      <arglist></arglist>
    </member>
    <member kind="variable">
      <type>int</type>
      <name>Lpipe_fixed_slots_for_branches</name>
      <anchorfile>l__pipe__util_8h.html</anchorfile>
      <anchor>5dfc4a9a1248286c8e15ad45456ddb90</anchor>
      <arglist></arglist>
    </member>
    <member kind="variable">
      <type>int</type>
      <name>Lpipe_do_only_postpass_steps</name>
      <anchorfile>l__pipe__util_8h.html</anchorfile>
      <anchor>414f010c3e4a932c6061bb593e7945b0</anchor>
      <arglist></arglist>
    </member>
    <member kind="variable">
      <type>int</type>
      <name>Lpipe_compact_branch_path_opers</name>
      <anchorfile>l__pipe__util_8h.html</anchorfile>
      <anchor>86f23e88bb2635324fa313d64bb50971</anchor>
      <arglist></arglist>
    </member>
    <member kind="variable">
      <type>int</type>
      <name>Lpipe_sort_mrt_rows</name>
      <anchorfile>l__pipe__util_8h.html</anchorfile>
      <anchor>1fd08397fdb9c8be171e9e45a1d811aa</anchor>
      <arglist></arglist>
    </member>
    <member kind="variable">
      <type>int</type>
      <name>Lpipe_combine_cbs</name>
      <anchorfile>l__pipe__util_8h.html</anchorfile>
      <anchor>33aa768a71f08754b1273f95c0cf1d2f</anchor>
      <arglist></arglist>
    </member>
    <member kind="variable">
      <type>int</type>
      <name>Lpipe_debug</name>
      <anchorfile>l__pipe__util_8h.html</anchorfile>
      <anchor>ad83dd0fea79c90886829873235d97be</anchor>
      <arglist></arglist>
    </member>
    <member kind="variable">
      <type>int</type>
      <name>Lpipe_debug_use_cb_bounds</name>
      <anchorfile>l__pipe__util_8h.html</anchorfile>
      <anchor>ab570eb13a3e6d913d45c8a3b85fad84</anchor>
      <arglist></arglist>
    </member>
    <member kind="variable">
      <type>int</type>
      <name>Lpipe_debug_lower_cb_bound</name>
      <anchorfile>l__pipe__util_8h.html</anchorfile>
      <anchor>4ebe475f1a840fbaa32b10d000fd7f70</anchor>
      <arglist></arglist>
    </member>
    <member kind="variable">
      <type>int</type>
      <name>Lpipe_debug_upper_cb_bound</name>
      <anchorfile>l__pipe__util_8h.html</anchorfile>
      <anchor>9142d5510348648db05b36ee12f6058e</anchor>
      <arglist></arglist>
    </member>
    <member kind="variable">
      <type>int</type>
      <name>Lpipe_print_statistics</name>
      <anchorfile>l__pipe__util_8h.html</anchorfile>
      <anchor>ab2140575cdf9b3526e6da5ea095a784</anchor>
      <arglist></arglist>
    </member>
    <member kind="variable">
      <type>int</type>
      <name>Lpipe_print_iteration_schedule</name>
      <anchorfile>l__pipe__util_8h.html</anchorfile>
      <anchor>f65026de9f77648de4df3b4f4ec74417</anchor>
      <arglist></arglist>
    </member>
    <member kind="variable">
      <type>int</type>
      <name>Lpipe_print_schedules_for_debug</name>
      <anchorfile>l__pipe__util_8h.html</anchorfile>
      <anchor>0b64713f1a54406156072dd81814ea25</anchor>
      <arglist></arglist>
    </member>
    <member kind="variable">
      <type>int</type>
      <name>Lpipe_print_mve_summary</name>
      <anchorfile>l__pipe__util_8h.html</anchorfile>
      <anchor>39d6ce980c9450094cb1c9372292b9ac</anchor>
      <arglist></arglist>
    </member>
    <member kind="variable">
      <type>int</type>
      <name>Lpipe_print_acyclic_stats</name>
      <anchorfile>l__pipe__util_8h.html</anchorfile>
      <anchor>b7eb5faa7f39fb7bfa2b757370855a9a</anchor>
      <arglist></arglist>
    </member>
    <member kind="variable">
      <type>int</type>
      <name>Lpipe_compute_loop_reg_pressure</name>
      <anchorfile>l__pipe__util_8h.html</anchorfile>
      <anchor>05b449c92ca8c59d61db6054cd56e7ee</anchor>
      <arglist></arglist>
    </member>
    <member kind="variable">
      <type>int</type>
      <name>Lpipe_add_spill_attributes</name>
      <anchorfile>l__pipe__util_8h.html</anchorfile>
      <anchor>e2d1368da174d6c3c8b7ec7d1cbbf44e</anchor>
      <arglist></arglist>
    </member>
    <member kind="variable">
      <type>int</type>
      <name>Lpipe_dump_dot</name>
      <anchorfile>l__pipe__util_8h.html</anchorfile>
      <anchor>2b86b75abb1973402ba2363621a03bb3</anchor>
      <arglist></arglist>
    </member>
    <member kind="variable">
      <type>L_Alloc_Pool *</type>
      <name>Queue_pool</name>
      <anchorfile>l__pipe__util_8h.html</anchorfile>
      <anchor>4a9d1f680824f7426e01b910d4cc2b03</anchor>
      <arglist></arglist>
    </member>
    <member kind="variable">
      <type>L_Alloc_Pool *</type>
      <name>Qnode_pool</name>
      <anchorfile>l__pipe__util_8h.html</anchorfile>
      <anchor>b2b35dba38efd9d6e53512e38d014729</anchor>
      <arglist></arglist>
    </member>
  </compound>
  <compound kind="file">
    <name>l_softpipe.c</name>
    <path>/mnt/hgfs/mkt/Desktop/openimpact/src/Lcode/codegen/Lsoftpipe/</path>
    <filename>l__softpipe_8c</filename>
    <includes id="l__softpipe__int_8h" name="l_softpipe_int.h" local="yes" imported="no">l_softpipe_int.h</includes>
    <includes id="l__softpipe_8h" name="l_softpipe.h" local="yes" imported="no">l_softpipe.h</includes>
    <includes id="l__pipe__sched_8h" name="l_pipe_sched.h" local="yes" imported="no">l_pipe_sched.h</includes>
    <includes id="l__pipe__sync_8h" name="l_pipe_sync.h" local="yes" imported="no">l_pipe_sync.h</includes>
    <includes id="l__pipe__rename_8h" name="l_pipe_rename.h" local="yes" imported="no">l_pipe_rename.h</includes>
    <includes id="l__mve_8h" name="l_mve.h" local="yes" imported="no">l_mve.h</includes>
    <member kind="define">
      <type>#define</type>
      <name>MAX_TRIES</name>
      <anchorfile>l__softpipe_8c.html</anchorfile>
      <anchor>fd63d23830ad86d01b6fff2e6c615f7e</anchor>
      <arglist></arglist>
    </member>
    <member kind="function">
      <type>void</type>
      <name>Lpipe_init</name>
      <anchorfile>l__softpipe_8c.html</anchorfile>
      <anchor>f044c0c24784b6dda4e45d4e5688ba4d</anchor>
      <arglist>(Parm_Macro_List *command_line_macro_list)</arglist>
    </member>
    <member kind="function">
      <type>void</type>
      <name>Lpipe_measurement_init</name>
      <anchorfile>l__softpipe_8c.html</anchorfile>
      <anchor>4f72a55b73cc8bb4b70700d7ba9cffe2</anchor>
      <arglist>(Parm_Macro_List *command_line_macro_list)</arglist>
    </member>
    <member kind="function">
      <type>void</type>
      <name>Lpipe_cleanup</name>
      <anchorfile>l__softpipe_8c.html</anchorfile>
      <anchor>5f4f86dea88858861446d1e06c1dea15</anchor>
      <arglist>(void)</arglist>
    </member>
    <member kind="function">
      <type>void</type>
      <name>Lpipe_measurement_cleanup</name>
      <anchorfile>l__softpipe_8c.html</anchorfile>
      <anchor>c18c12740729ece750284668380d64dd</anchor>
      <arglist>(void)</arglist>
    </member>
    <member kind="function" static="yes">
      <type>static void</type>
      <name>Lpipe_failure</name>
      <anchorfile>l__softpipe_8c.html</anchorfile>
      <anchor>dc1a6d6920170c96e1c6d6d52eacf808</anchor>
      <arglist>(SM_Cb *sm_cb, Softpipe_MinII *MinII)</arglist>
    </member>
    <member kind="function" static="yes">
      <type>static void</type>
      <name>Lpipe_compute_start_stop_issue_time</name>
      <anchorfile>l__softpipe_8c.html</anchorfile>
      <anchor>950faa312298324e1d5634c3c4842ae1</anchor>
      <arglist>(SM_Cb *sm_cb)</arglist>
    </member>
    <member kind="function" static="yes">
      <type>static int</type>
      <name>Lpipe_pipeline_loop</name>
      <anchorfile>l__softpipe_8c.html</anchorfile>
      <anchor>cd9bb1e732e6b1dd2cb4071650da8534</anchor>
      <arglist>(L_Func *fn, L_Inner_Loop *loop)</arglist>
    </member>
    <member kind="function">
      <type>void</type>
      <name>Lpipe_software_pipeline</name>
      <anchorfile>l__softpipe_8c.html</anchorfile>
      <anchor>5b303ec4e784e21c2d7d38c47ea63b98</anchor>
      <arglist>(L_Func *fn)</arglist>
    </member>
    <member kind="function">
      <type>void</type>
      <name>Lpipe_mark_loops_with_spills</name>
      <anchorfile>l__softpipe_8c.html</anchorfile>
      <anchor>1240d252386c4bb4d5f9bc08a7c2039e</anchor>
      <arglist>(L_Func *fn)</arglist>
    </member>
    <member kind="function">
      <type>void</type>
      <name>Lpipe_compute_reg_pressure</name>
      <anchorfile>l__softpipe_8c.html</anchorfile>
      <anchor>432bc19bba68265dd93532edc814fcff</anchor>
      <arglist>(L_Func *fn)</arglist>
    </member>
    <member kind="variable">
      <type>L_Cb *</type>
      <name>header_cb</name>
      <anchorfile>l__softpipe_8c.html</anchorfile>
      <anchor>91e84fa9645e4d3beb7b8b6c150da74b</anchor>
      <arglist></arglist>
    </member>
    <member kind="variable">
      <type>L_Cb *</type>
      <name>preheader_cb</name>
      <anchorfile>l__softpipe_8c.html</anchorfile>
      <anchor>4ee6d00e67672cc9a04b6f5f533bfb72</anchor>
      <arglist></arglist>
    </member>
    <member kind="variable">
      <type>L_Cb *</type>
      <name>prologue_cb</name>
      <anchorfile>l__softpipe_8c.html</anchorfile>
      <anchor>4708e528902bf49b9c374df3e644309d</anchor>
      <arglist></arglist>
    </member>
    <member kind="variable">
      <type>L_Cb *</type>
      <name>remainder_cb</name>
      <anchorfile>l__softpipe_8c.html</anchorfile>
      <anchor>785a1b69c62478bf5624354abd3955e1</anchor>
      <arglist></arglist>
    </member>
    <member kind="variable">
      <type>int</type>
      <name>Lpipe_stage_count</name>
      <anchorfile>l__softpipe_8c.html</anchorfile>
      <anchor>e9b3310fa8a82cd6903c833cef98effd</anchor>
      <arglist></arglist>
    </member>
    <member kind="variable">
      <type>int</type>
      <name>Lpipe_counted_loop</name>
      <anchorfile>l__softpipe_8c.html</anchorfile>
      <anchor>e41610bb03edbaeb7c02cfcc7fc4467b</anchor>
      <arglist></arglist>
    </member>
    <member kind="variable">
      <type>L_Oper **</type>
      <name>kernel_copy_last_op</name>
      <anchorfile>l__softpipe_8c.html</anchorfile>
      <anchor>43dd5bc461bab37b4dc35075227b76df</anchor>
      <arglist></arglist>
    </member>
    <member kind="variable">
      <type>L_Oper **</type>
      <name>kernel_copy_first_op</name>
      <anchorfile>l__softpipe_8c.html</anchorfile>
      <anchor>394714920d405996c9ac56376596536a</anchor>
      <arglist></arglist>
    </member>
    <member kind="variable">
      <type>int</type>
      <name>loop_dep_height</name>
      <anchorfile>l__softpipe_8c.html</anchorfile>
      <anchor>4dc68702d9b52f9ccb3dc704c3382dad</anchor>
      <arglist></arglist>
    </member>
    <member kind="variable">
      <type>int</type>
      <name>Lpipe_total_issue_slots</name>
      <anchorfile>l__softpipe_8c.html</anchorfile>
      <anchor>377860f550eb8f35543d1b7c4a931268</anchor>
      <arglist></arglist>
    </member>
    <member kind="variable">
      <type>FILE *</type>
      <name>sched_file</name>
      <anchorfile>l__softpipe_8c.html</anchorfile>
      <anchor>dfcefa1a772012ef80d3b8f14f3f150b</anchor>
      <arglist></arglist>
    </member>
    <member kind="variable">
      <type>FILE *</type>
      <name>pipe_statfile</name>
      <anchorfile>l__softpipe_8c.html</anchorfile>
      <anchor>d4ad2d89187dec7ea2bbf365f1e10e1e</anchor>
      <arglist></arglist>
    </member>
    <member kind="variable">
      <type>FILE *</type>
      <name>gen_statfile</name>
      <anchorfile>l__softpipe_8c.html</anchorfile>
      <anchor>bfb992b8633f7de2f771384a01c7411e</anchor>
      <arglist></arglist>
    </member>
    <member kind="variable">
      <type>FILE *</type>
      <name>iters_statfile</name>
      <anchorfile>l__softpipe_8c.html</anchorfile>
      <anchor>065f771def475aca9432d340c6eaf35b</anchor>
      <arglist></arglist>
    </member>
    <member kind="variable">
      <type>FILE *</type>
      <name>reg_statfile</name>
      <anchorfile>l__softpipe_8c.html</anchorfile>
      <anchor>9f9788ffccb227a45934435833fe651e</anchor>
      <arglist></arglist>
    </member>
    <member kind="variable">
      <type>FILE *</type>
      <name>func_reg_statfile</name>
      <anchorfile>l__softpipe_8c.html</anchorfile>
      <anchor>66928bb443a445b2b093b1082d083379</anchor>
      <arglist></arglist>
    </member>
    <member kind="variable">
      <type>FILE *</type>
      <name>spill_statfile</name>
      <anchorfile>l__softpipe_8c.html</anchorfile>
      <anchor>b31a94b2b54c2bff51947401462afb8a</anchor>
      <arglist></arglist>
    </member>
    <member kind="variable">
      <type>FILE *</type>
      <name>code_statfile</name>
      <anchorfile>l__softpipe_8c.html</anchorfile>
      <anchor>05e5e553ea7e96a23f4379f954ce70bb</anchor>
      <arglist></arglist>
    </member>
    <member kind="variable">
      <type>FILE *</type>
      <name>swp_code_statfile</name>
      <anchorfile>l__softpipe_8c.html</anchorfile>
      <anchor>6124881d1ab46cfbcebdfa6852499434</anchor>
      <arglist></arglist>
    </member>
    <member kind="variable">
      <type>FILE *</type>
      <name>acyclic_statfile</name>
      <anchorfile>l__softpipe_8c.html</anchorfile>
      <anchor>e08449fa1216f1e3436bf8d7fe73731f</anchor>
      <arglist></arglist>
    </member>
  </compound>
  <compound kind="file">
    <name>l_softpipe.h</name>
    <path>/mnt/hgfs/mkt/Desktop/openimpact/src/Lcode/codegen/Lsoftpipe/</path>
    <filename>l__softpipe_8h</filename>
    <member kind="function">
      <type>void</type>
      <name>Lpipe_init</name>
      <anchorfile>l__softpipe_8h.html</anchorfile>
      <anchor>0044ecaec153a71b1a018fcb06d92665</anchor>
      <arglist>(Parm_Macro_List *)</arglist>
    </member>
    <member kind="function">
      <type>void</type>
      <name>Lpipe_cleanup</name>
      <anchorfile>l__softpipe_8h.html</anchorfile>
      <anchor>5f4f86dea88858861446d1e06c1dea15</anchor>
      <arglist>(void)</arglist>
    </member>
    <member kind="function">
      <type>void</type>
      <name>Lpipe_measurement_init</name>
      <anchorfile>l__softpipe_8h.html</anchorfile>
      <anchor>6b73cb449fa2281895b3a4aa9d030b76</anchor>
      <arglist>(Parm_Macro_List *)</arglist>
    </member>
    <member kind="function">
      <type>void</type>
      <name>Lpipe_measurement_cleanup</name>
      <anchorfile>l__softpipe_8h.html</anchorfile>
      <anchor>c18c12740729ece750284668380d64dd</anchor>
      <arglist>(void)</arglist>
    </member>
    <member kind="function">
      <type>void</type>
      <name>Lpipe_software_pipeline</name>
      <anchorfile>l__softpipe_8h.html</anchorfile>
      <anchor>462160b7baa0babe71d4d99a2ea1cacf</anchor>
      <arglist>(L_Func *)</arglist>
    </member>
    <member kind="function">
      <type>void</type>
      <name>Lpipe_fill_delay_slots</name>
      <anchorfile>l__softpipe_8h.html</anchorfile>
      <anchor>9f7e49adda2510d97489e2bfb1644da0</anchor>
      <arglist>(L_Func *)</arglist>
    </member>
    <member kind="function">
      <type>void</type>
      <name>Lpipe_mark_loops_with_spills</name>
      <anchorfile>l__softpipe_8h.html</anchorfile>
      <anchor>069ce3dfcfa7ef2d0e36b64aa8348cb3</anchor>
      <arglist>(L_Func *)</arglist>
    </member>
    <member kind="function">
      <type>void</type>
      <name>Lpipe_compute_reg_pressure</name>
      <anchorfile>l__softpipe_8h.html</anchorfile>
      <anchor>543bf3b6c57a16d1555f49e4645742bc</anchor>
      <arglist>(L_Func *)</arglist>
    </member>
    <member kind="function">
      <type>void</type>
      <name>Lpipe_move_int_parm_regs</name>
      <anchorfile>l__softpipe_8h.html</anchorfile>
      <anchor>bc532cb244121a0e4179f7d989a94f9e</anchor>
      <arglist>(L_Func *)</arglist>
    </member>
    <member kind="function">
      <type>void</type>
      <name>L_annotate_oper</name>
      <anchorfile>l__softpipe_8h.html</anchorfile>
      <anchor>fd8def20f61a7cd2a65cd6f3e89f233f</anchor>
      <arglist>(L_Func *, L_Cb *, L_Oper *)</arglist>
    </member>
  </compound>
  <compound kind="file">
    <name>l_softpipe_info.c</name>
    <path>/mnt/hgfs/mkt/Desktop/openimpact/src/Lcode/codegen/Lsoftpipe/</path>
    <filename>l__softpipe__info_8c</filename>
    <includes id="l__softpipe__int_8h" name="l_softpipe_int.h" local="yes" imported="no">l_softpipe_int.h</includes>
    <includes id="l__mve_8h" name="l_mve.h" local="yes" imported="no">l_mve.h</includes>
    <member kind="function">
      <type>void</type>
      <name>Lpipe_init_op_info</name>
      <anchorfile>l__softpipe__info_8c.html</anchorfile>
      <anchor>a1f243509717138e72fffec354e493a4</anchor>
      <arglist>(SM_Oper *sm_op, int init_ready_time, int lite)</arglist>
    </member>
    <member kind="function">
      <type>Softpipe_Op_Info *</type>
      <name>Lpipe_create_op_info</name>
      <anchorfile>l__softpipe__info_8c.html</anchorfile>
      <anchor>20e0bb7258cae1e6da705eff2689ea9c</anchor>
      <arglist>()</arglist>
    </member>
    <member kind="function">
      <type>void</type>
      <name>Lpipe_construct_op_info</name>
      <anchorfile>l__softpipe__info_8c.html</anchorfile>
      <anchor>8d0bf852772189f5c84753f7efa2f4e4</anchor>
      <arglist>(SM_Cb *sm_cb, SM_Oper *loop_back_br, int *num_oper, int *branch_count)</arglist>
    </member>
    <member kind="function">
      <type>Softpipe_Op_Info *</type>
      <name>Lpipe_copy_op_info</name>
      <anchorfile>l__softpipe__info_8c.html</anchorfile>
      <anchor>115fd1055670ac5193d46e9e10a2a9f4</anchor>
      <arglist>(Softpipe_Op_Info *softpipe_info)</arglist>
    </member>
    <member kind="function">
      <type>void</type>
      <name>Lpipe_copy_sched_info</name>
      <anchorfile>l__softpipe__info_8c.html</anchorfile>
      <anchor>2f8e0402029878eba06bf8e9c870bf9b</anchor>
      <arglist>(SM_Cb *sm_cb)</arglist>
    </member>
    <member kind="function">
      <type>void</type>
      <name>Lpipe_free_op_info</name>
      <anchorfile>l__softpipe__info_8c.html</anchorfile>
      <anchor>62ce42282a593806d185df143094a321</anchor>
      <arglist>(L_Cb *cb)</arglist>
    </member>
    <member kind="function">
      <type>void</type>
      <name>Lpipe_free_oper_op_info</name>
      <anchorfile>l__softpipe__info_8c.html</anchorfile>
      <anchor>070ce57374be3f0dd0db1e026e5f019c</anchor>
      <arglist>(L_Oper *oper)</arglist>
    </member>
    <member kind="function">
      <type>void</type>
      <name>Lpipe_print_cb_info</name>
      <anchorfile>l__softpipe__info_8c.html</anchorfile>
      <anchor>a5f0f3fb74601d90b6f5de3b32a39a7a</anchor>
      <arglist>(SM_Cb *sm_cb)</arglist>
    </member>
    <member kind="function">
      <type>void</type>
      <name>Lpipe_print_op_info</name>
      <anchorfile>l__softpipe__info_8c.html</anchorfile>
      <anchor>9f4a4d46b2e2c57f0677ad6e07365afc</anchor>
      <arglist>(SM_Oper *sm_op)</arglist>
    </member>
    <member kind="variable">
      <type>L_Alloc_Pool *</type>
      <name>Softpipe_Op_Info_pool</name>
      <anchorfile>l__softpipe__info_8c.html</anchorfile>
      <anchor>bec30e2c6dd19674777250eed6a17946</anchor>
      <arglist></arglist>
    </member>
  </compound>
  <compound kind="file">
    <name>l_softpipe_info.h</name>
    <path>/mnt/hgfs/mkt/Desktop/openimpact/src/Lcode/codegen/Lsoftpipe/</path>
    <filename>l__softpipe__info_8h</filename>
    <class kind="struct">_Lpipe_LRInfo</class>
    <class kind="struct">_Lpipe_MVEInfo</class>
    <class kind="struct">_Softpipe_Op_Info</class>
    <member kind="typedef">
      <type>_Lpipe_LRInfo</type>
      <name>Lpipe_LRInfo</name>
      <anchorfile>l__softpipe__info_8h.html</anchorfile>
      <anchor>fcdab3d96780ced265069f97231ad342</anchor>
      <arglist></arglist>
    </member>
    <member kind="typedef">
      <type>_Lpipe_MVEInfo</type>
      <name>Lpipe_MVEInfo</name>
      <anchorfile>l__softpipe__info_8h.html</anchorfile>
      <anchor>e2eec7242699189d0607f8776c99d169</anchor>
      <arglist></arglist>
    </member>
    <member kind="typedef">
      <type>_Softpipe_Op_Info</type>
      <name>Softpipe_Op_Info</name>
      <anchorfile>l__softpipe__info_8h.html</anchorfile>
      <anchor>7380905f4201b83058aedfdeca3a6e42</anchor>
      <arglist></arglist>
    </member>
    <member kind="function">
      <type>Softpipe_Op_Info *</type>
      <name>Lpipe_create_op_info</name>
      <anchorfile>l__softpipe__info_8h.html</anchorfile>
      <anchor>20e0bb7258cae1e6da705eff2689ea9c</anchor>
      <arglist>()</arglist>
    </member>
    <member kind="function">
      <type>void</type>
      <name>Lpipe_init_op_info</name>
      <anchorfile>l__softpipe__info_8h.html</anchorfile>
      <anchor>8e2902c259eea437f212d8dd2f37581d</anchor>
      <arglist>(SM_Oper *, int, int)</arglist>
    </member>
    <member kind="function">
      <type>void</type>
      <name>Lpipe_construct_op_info</name>
      <anchorfile>l__softpipe__info_8h.html</anchorfile>
      <anchor>a7da7240e2af1b57604def72d2690b96</anchor>
      <arglist>(SM_Cb *, SM_Oper *, int *, int *)</arglist>
    </member>
    <member kind="function">
      <type>Softpipe_Op_Info *</type>
      <name>Lpipe_copy_op_info</name>
      <anchorfile>l__softpipe__info_8h.html</anchorfile>
      <anchor>0fe00879c789bd794f722f6222a26ab2</anchor>
      <arglist>(Softpipe_Op_Info *)</arglist>
    </member>
    <member kind="function">
      <type>void</type>
      <name>Lpipe_free_op_info</name>
      <anchorfile>l__softpipe__info_8h.html</anchorfile>
      <anchor>45c673e7167fafd1100dac8ab302e403</anchor>
      <arglist>(L_Cb *)</arglist>
    </member>
    <member kind="function">
      <type>void</type>
      <name>Lpipe_free_oper_op_info</name>
      <anchorfile>l__softpipe__info_8h.html</anchorfile>
      <anchor>c089fa4aaed7b8b8fbc55c6bc2b7aaf4</anchor>
      <arglist>(L_Oper *)</arglist>
    </member>
    <member kind="function">
      <type>void</type>
      <name>Lpipe_print_op_info</name>
      <anchorfile>l__softpipe__info_8h.html</anchorfile>
      <anchor>f0e2cba88dd5514660ae4564c252f825</anchor>
      <arglist>(SM_Oper *)</arglist>
    </member>
    <member kind="function">
      <type>void</type>
      <name>Lpipe_print_cb_info</name>
      <anchorfile>l__softpipe__info_8h.html</anchorfile>
      <anchor>fbf960c17e0601b422b7f4e8869a621d</anchor>
      <arglist>(SM_Cb *)</arglist>
    </member>
    <member kind="function">
      <type>void</type>
      <name>Lpipe_copy_sched_info</name>
      <anchorfile>l__softpipe__info_8h.html</anchorfile>
      <anchor>8723da582e7e68a47c39627ccb944379</anchor>
      <arglist>(SM_Cb *)</arglist>
    </member>
    <member kind="variable">
      <type>L_Alloc_Pool *</type>
      <name>Softpipe_Op_Info_pool</name>
      <anchorfile>l__softpipe__info_8h.html</anchorfile>
      <anchor>bec30e2c6dd19674777250eed6a17946</anchor>
      <arglist></arglist>
    </member>
  </compound>
  <compound kind="file">
    <name>l_softpipe_int.h</name>
    <path>/mnt/hgfs/mkt/Desktop/openimpact/src/Lcode/codegen/Lsoftpipe/</path>
    <filename>l__softpipe__int_8h</filename>
    <includes id="l__loop__prep_8h" name="l_loop_prep.h" local="yes" imported="no">l_loop_prep.h</includes>
    <includes id="l__pipe__util_8h" name="l_pipe_util.h" local="yes" imported="no">l_pipe_util.h</includes>
    <includes id="l__pipe__rename_8h" name="l_pipe_rename.h" local="yes" imported="no">l_pipe_rename.h</includes>
    <includes id="l__softpipe__info_8h" name="l_softpipe_info.h" local="yes" imported="no">l_softpipe_info.h</includes>
    <includes id="l__gen__pipe_8h" name="l_gen_pipe.h" local="yes" imported="no">l_gen_pipe.h</includes>
    <member kind="define">
      <type>#define</type>
      <name>SOFTPIPE_OP_INFO</name>
      <anchorfile>l__softpipe__int_8h.html</anchorfile>
      <anchor>50a06a5d593647c987ffc62034b61991</anchor>
      <arglist>(oper)</arglist>
    </member>
    <member kind="define">
      <type>#define</type>
      <name>IMPOSSIBLE_PRIORITY</name>
      <anchorfile>l__softpipe__int_8h.html</anchorfile>
      <anchor>aa6440f8a241a8b4b71f4ea4f0352793</anchor>
      <arglist></arglist>
    </member>
    <member kind="define">
      <type>#define</type>
      <name>Lpipe_ignore_kernel_inst</name>
      <anchorfile>l__softpipe__int_8h.html</anchorfile>
      <anchor>2b1a0c0947bed531533824d870c251b9</anchor>
      <arglist>(oper)</arglist>
    </member>
    <member kind="variable">
      <type>L_Oper **</type>
      <name>kernel_copy_last_op</name>
      <anchorfile>l__softpipe__int_8h.html</anchorfile>
      <anchor>43dd5bc461bab37b4dc35075227b76df</anchor>
      <arglist></arglist>
    </member>
    <member kind="variable">
      <type>L_Oper **</type>
      <name>kernel_copy_first_op</name>
      <anchorfile>l__softpipe__int_8h.html</anchorfile>
      <anchor>394714920d405996c9ac56376596536a</anchor>
      <arglist></arglist>
    </member>
    <member kind="variable">
      <type>L_Cb *</type>
      <name>header_cb</name>
      <anchorfile>l__softpipe__int_8h.html</anchorfile>
      <anchor>91e84fa9645e4d3beb7b8b6c150da74b</anchor>
      <arglist></arglist>
    </member>
    <member kind="variable">
      <type>L_Cb *</type>
      <name>preheader_cb</name>
      <anchorfile>l__softpipe__int_8h.html</anchorfile>
      <anchor>4ee6d00e67672cc9a04b6f5f533bfb72</anchor>
      <arglist></arglist>
    </member>
    <member kind="variable">
      <type>L_Cb *</type>
      <name>prologue_cb</name>
      <anchorfile>l__softpipe__int_8h.html</anchorfile>
      <anchor>4708e528902bf49b9c374df3e644309d</anchor>
      <arglist></arglist>
    </member>
    <member kind="variable">
      <type>L_Cb *</type>
      <name>remainder_cb</name>
      <anchorfile>l__softpipe__int_8h.html</anchorfile>
      <anchor>785a1b69c62478bf5624354abd3955e1</anchor>
      <arglist></arglist>
    </member>
    <member kind="variable">
      <type>int</type>
      <name>Lpipe_stage_count</name>
      <anchorfile>l__softpipe__int_8h.html</anchorfile>
      <anchor>e9b3310fa8a82cd6903c833cef98effd</anchor>
      <arglist></arglist>
    </member>
    <member kind="variable">
      <type>int</type>
      <name>Lpipe_counted_loop</name>
      <anchorfile>l__softpipe__int_8h.html</anchorfile>
      <anchor>e41610bb03edbaeb7c02cfcc7fc4467b</anchor>
      <arglist></arglist>
    </member>
    <member kind="variable">
      <type>FILE *</type>
      <name>sched_file</name>
      <anchorfile>l__softpipe__int_8h.html</anchorfile>
      <anchor>dfcefa1a772012ef80d3b8f14f3f150b</anchor>
      <arglist></arglist>
    </member>
    <member kind="variable">
      <type>int</type>
      <name>loop_dep_height</name>
      <anchorfile>l__softpipe__int_8h.html</anchorfile>
      <anchor>4dc68702d9b52f9ccb3dc704c3382dad</anchor>
      <arglist></arglist>
    </member>
    <member kind="variable">
      <type>int</type>
      <name>Lpipe_total_issue_slots</name>
      <anchorfile>l__softpipe__int_8h.html</anchorfile>
      <anchor>377860f550eb8f35543d1b7c4a931268</anchor>
      <arglist></arglist>
    </member>
  </compound>
  <compound kind="struct">
    <name>_Lpipe_LRInfo</name>
    <filename>struct__Lpipe__LRInfo.html</filename>
    <member kind="variable">
      <type>SM_Reg_Info *</type>
      <name>rinfo</name>
      <anchorfile>struct__Lpipe__LRInfo.html</anchorfile>
      <anchor>c4314486f712f695d0c174ff10bc9428</anchor>
      <arglist></arglist>
    </member>
    <member kind="variable">
      <type>SM_Oper *</type>
      <name>def_sm_oper</name>
      <anchorfile>struct__Lpipe__LRInfo.html</anchorfile>
      <anchor>97f7aeb430c0ee2859209e32335b685d</anchor>
      <arglist></arglist>
    </member>
    <member kind="variable">
      <type>int</type>
      <name>lifetime</name>
      <anchorfile>struct__Lpipe__LRInfo.html</anchorfile>
      <anchor>5b6749fd6504e56ea29c985224fa6731</anchor>
      <arglist></arglist>
    </member>
    <member kind="variable">
      <type>int</type>
      <name>first_def_time</name>
      <anchorfile>struct__Lpipe__LRInfo.html</anchorfile>
      <anchor>d635005fcbfec1cd679e74ce08dd34a9</anchor>
      <arglist></arglist>
    </member>
    <member kind="variable">
      <type>int</type>
      <name>first_def_slot</name>
      <anchorfile>struct__Lpipe__LRInfo.html</anchorfile>
      <anchor>90cc1eadc7011d7347fa4ad94f44f97c</anchor>
      <arglist></arglist>
    </member>
    <member kind="variable">
      <type>int</type>
      <name>live_in_def_time</name>
      <anchorfile>struct__Lpipe__LRInfo.html</anchorfile>
      <anchor>9f1d5e793fecfd9d8392c8cb8f39f65f</anchor>
      <arglist></arglist>
    </member>
    <member kind="variable">
      <type>int</type>
      <name>last_access_time</name>
      <anchorfile>struct__Lpipe__LRInfo.html</anchorfile>
      <anchor>422c27c3653ac5bfa836ec2c62f0917b</anchor>
      <arglist></arglist>
    </member>
    <member kind="variable">
      <type>int</type>
      <name>last_access_slot</name>
      <anchorfile>struct__Lpipe__LRInfo.html</anchorfile>
      <anchor>1d22a7e1e5ddcb61db6db997a7d69b0f</anchor>
      <arglist></arglist>
    </member>
    <member kind="variable">
      <type>int</type>
      <name>num_names</name>
      <anchorfile>struct__Lpipe__LRInfo.html</anchorfile>
      <anchor>db15a0e8e7d797feaa48a78b14fc0ffc</anchor>
      <arglist></arglist>
    </member>
    <member kind="variable">
      <type>int</type>
      <name>num_names_w_pro</name>
      <anchorfile>struct__Lpipe__LRInfo.html</anchorfile>
      <anchor>60ccf00fa2c17a8f6cf454ecc7c66fa4</anchor>
      <arglist></arglist>
    </member>
    <member kind="variable">
      <type>L_Operand **</type>
      <name>names</name>
      <anchorfile>struct__Lpipe__LRInfo.html</anchorfile>
      <anchor>fd23833144a3d6c82aebcaea85e678c2</anchor>
      <arglist></arglist>
    </member>
    <member kind="variable">
      <type>unsigned int</type>
      <name>use_after_def_slot</name>
      <anchorfile>struct__Lpipe__LRInfo.html</anchorfile>
      <anchor>ae8e3bca328790f251be3123eabfe202</anchor>
      <arglist></arglist>
    </member>
    <member kind="variable">
      <type>unsigned int</type>
      <name>live_out</name>
      <anchorfile>struct__Lpipe__LRInfo.html</anchorfile>
      <anchor>594f265494d76619532559b1f3cde230</anchor>
      <arglist></arglist>
    </member>
    <member kind="variable">
      <type>unsigned int</type>
      <name>live_in</name>
      <anchorfile>struct__Lpipe__LRInfo.html</anchorfile>
      <anchor>848c52472b8515b66f534300612b2b31</anchor>
      <arglist></arglist>
    </member>
    <member kind="variable">
      <type>unsigned int</type>
      <name>prologue_mov_inserted</name>
      <anchorfile>struct__Lpipe__LRInfo.html</anchorfile>
      <anchor>eaffe45cfd023d8bf117f644ef7c58a3</anchor>
      <arglist></arglist>
    </member>
  </compound>
  <compound kind="struct">
    <name>_Lpipe_MVEInfo</name>
    <filename>struct__Lpipe__MVEInfo.html</filename>
    <member kind="variable">
      <type>_Lpipe_LRInfo *</type>
      <name>live_range</name>
      <anchorfile>struct__Lpipe__MVEInfo.html</anchorfile>
      <anchor>fefeadc6c253fedb37efcb629620114b</anchor>
      <arglist></arglist>
    </member>
    <member kind="variable">
      <type>int</type>
      <name>lifetime</name>
      <anchorfile>struct__Lpipe__MVEInfo.html</anchorfile>
      <anchor>f102af267f8a4b197c65664f8c3712c6</anchor>
      <arglist></arglist>
    </member>
    <member kind="variable">
      <type>int</type>
      <name>stage_lifetime_incr</name>
      <anchorfile>struct__Lpipe__MVEInfo.html</anchorfile>
      <anchor>ab600da820f182e8e72b661971cc68f9</anchor>
      <arglist></arglist>
    </member>
  </compound>
  <compound kind="struct">
    <name>_LpipeLiveRange</name>
    <filename>struct__LpipeLiveRange.html</filename>
    <member kind="variable">
      <type>short</type>
      <name>id</name>
      <anchorfile>struct__LpipeLiveRange.html</anchorfile>
      <anchor>dec6a394c3b37c85975c5416fc208f97</anchor>
      <arglist></arglist>
    </member>
    <member kind="variable">
      <type>short</type>
      <name>vreg</name>
      <anchorfile>struct__LpipeLiveRange.html</anchorfile>
      <anchor>08170f2c5692ac735ea9803b16872f32</anchor>
      <arglist></arglist>
    </member>
    <member kind="variable">
      <type>short</type>
      <name>valid</name>
      <anchorfile>struct__LpipeLiveRange.html</anchorfile>
      <anchor>e0b28e0aacadc13091deb968632d95a4</anchor>
      <arglist></arglist>
    </member>
    <member kind="variable">
      <type>short</type>
      <name>ctype</name>
      <anchorfile>struct__LpipeLiveRange.html</anchorfile>
      <anchor>114e7d202fd6c56252b179a987435368</anchor>
      <arglist></arglist>
    </member>
    <member kind="variable">
      <type>L_Oper *</type>
      <name>def_oper</name>
      <anchorfile>struct__LpipeLiveRange.html</anchorfile>
      <anchor>517e8efcaac54001327858e68ffb4737</anchor>
      <arglist></arglist>
    </member>
    <member kind="variable">
      <type>Set</type>
      <name>op</name>
      <anchorfile>struct__LpipeLiveRange.html</anchorfile>
      <anchor>6d7f7b0bcdde39f63e157c8d22fe5c86</anchor>
      <arglist></arglist>
    </member>
    <member kind="variable">
      <type>Set</type>
      <name>def_op</name>
      <anchorfile>struct__LpipeLiveRange.html</anchorfile>
      <anchor>10f91cde610906ee066d5f3ff5621cff</anchor>
      <arglist></arglist>
    </member>
    <member kind="variable">
      <type>Set</type>
      <name>ref_op</name>
      <anchorfile>struct__LpipeLiveRange.html</anchorfile>
      <anchor>3322ccbc352b6324d8879333cd04be7a</anchor>
      <arglist></arglist>
    </member>
    <member kind="variable">
      <type>Set</type>
      <name>live_out_op</name>
      <anchorfile>struct__LpipeLiveRange.html</anchorfile>
      <anchor>0a8215512e9cc4834195ebae83091b29</anchor>
      <arglist></arglist>
    </member>
  </compound>
  <compound kind="struct">
    <name>_Softpipe_Op_Info</name>
    <filename>struct__Softpipe__Op__Info.html</filename>
    <member kind="variable">
      <type>int</type>
      <name>home_block</name>
      <anchorfile>struct__Softpipe__Op__Info.html</anchorfile>
      <anchor>15915ccd432c55617a5283f8cca8c558</anchor>
      <arglist></arglist>
    </member>
    <member kind="variable">
      <type>L_Cb *</type>
      <name>exit_cb</name>
      <anchorfile>struct__Softpipe__Op__Info.html</anchorfile>
      <anchor>bcf3d5ebc41068339a81cec12717b661</anchor>
      <arglist></arglist>
    </member>
    <member kind="variable">
      <type>double</type>
      <name>exit_weight</name>
      <anchorfile>struct__Softpipe__Op__Info.html</anchorfile>
      <anchor>f25a421770f3010008e84f8f0b491302</anchor>
      <arglist></arglist>
    </member>
    <member kind="variable">
      <type>int</type>
      <name>intra_iter_issue_time</name>
      <anchorfile>struct__Softpipe__Op__Info.html</anchorfile>
      <anchor>7d7e64bcab97f4fccde857ee7430b6ec</anchor>
      <arglist></arglist>
    </member>
    <member kind="variable">
      <type>int</type>
      <name>issue_time</name>
      <anchorfile>struct__Softpipe__Op__Info.html</anchorfile>
      <anchor>f69da927376f1a1b3c0b3fc0ed76fb65</anchor>
      <arglist></arglist>
    </member>
    <member kind="variable">
      <type>int</type>
      <name>issue_slot</name>
      <anchorfile>struct__Softpipe__Op__Info.html</anchorfile>
      <anchor>020d2854e972ae85a214b3012f8e023e</anchor>
      <arglist></arglist>
    </member>
    <member kind="variable">
      <type>int</type>
      <name>stage</name>
      <anchorfile>struct__Softpipe__Op__Info.html</anchorfile>
      <anchor>1c69466278e98d42ca7bd8d91916cd93</anchor>
      <arglist></arglist>
    </member>
    <member kind="variable">
      <type>int</type>
      <name>kernel_copy</name>
      <anchorfile>struct__Softpipe__Op__Info.html</anchorfile>
      <anchor>3fadf4a8a69a2dad970274041fed7e69</anchor>
      <arglist></arglist>
    </member>
    <member kind="variable">
      <type>Lpipe_MVEInfo **</type>
      <name>src_mve_info</name>
      <anchorfile>struct__Softpipe__Op__Info.html</anchorfile>
      <anchor>6f7d8b726cd7038e448a61992d256fc8</anchor>
      <arglist></arglist>
    </member>
    <member kind="variable">
      <type>Lpipe_MVEInfo **</type>
      <name>pred_mve_info</name>
      <anchorfile>struct__Softpipe__Op__Info.html</anchorfile>
      <anchor>b94a61078ba4150ddc32e6ebb519ab2c</anchor>
      <arglist></arglist>
    </member>
    <member kind="variable">
      <type>Lpipe_MVEInfo **</type>
      <name>dest_mve_info</name>
      <anchorfile>struct__Softpipe__Op__Info.html</anchorfile>
      <anchor>416db2ab0ba42b6e793e89a903ae2105</anchor>
      <arglist></arglist>
    </member>
    <member kind="variable">
      <type>List</type>
      <name>isrc_mve_info</name>
      <anchorfile>struct__Softpipe__Op__Info.html</anchorfile>
      <anchor>7207512787d15f20544a489a9e7d6c2d</anchor>
      <arglist></arglist>
    </member>
    <member kind="variable">
      <type>int</type>
      <name>prologue_stage</name>
      <anchorfile>struct__Softpipe__Op__Info.html</anchorfile>
      <anchor>368464d3331a9a8060fec13d357b0312</anchor>
      <arglist></arglist>
    </member>
    <member kind="variable">
      <type>int</type>
      <name>epilogue_stage</name>
      <anchorfile>struct__Softpipe__Op__Info.html</anchorfile>
      <anchor>ccadd541b4797347084630474a27a946</anchor>
      <arglist></arglist>
    </member>
    <member kind="variable">
      <type>int</type>
      <name>unrolled_iter_num</name>
      <anchorfile>struct__Softpipe__Op__Info.html</anchorfile>
      <anchor>3a8f182441fa610e021d5140f0e65625</anchor>
      <arglist></arglist>
    </member>
    <member kind="variable">
      <type>int</type>
      <name>estart</name>
      <anchorfile>struct__Softpipe__Op__Info.html</anchorfile>
      <anchor>dcc3a8736b3d1269334af5fe728c232a</anchor>
      <arglist></arglist>
    </member>
    <member kind="variable">
      <type>int</type>
      <name>lstart</name>
      <anchorfile>struct__Softpipe__Op__Info.html</anchorfile>
      <anchor>2ff128cfe3a8287ec7b981c8259b7c55</anchor>
      <arglist></arglist>
    </member>
    <member kind="variable">
      <type>int</type>
      <name>slack</name>
      <anchorfile>struct__Softpipe__Op__Info.html</anchorfile>
      <anchor>ffee04f42974a52a0a393131c24e56f3</anchor>
      <arglist></arglist>
    </member>
    <member kind="variable">
      <type>int</type>
      <name>priority</name>
      <anchorfile>struct__Softpipe__Op__Info.html</anchorfile>
      <anchor>894628085295a2ff6742a3c68cf73761</anchor>
      <arglist></arglist>
    </member>
    <member kind="variable">
      <type>int</type>
      <name>ready_time</name>
      <anchorfile>struct__Softpipe__Op__Info.html</anchorfile>
      <anchor>245284eb1dba24400c1d117c8cbed493</anchor>
      <arglist></arglist>
    </member>
    <member kind="variable">
      <type>unsigned int</type>
      <name>loop_back_br</name>
      <anchorfile>struct__Softpipe__Op__Info.html</anchorfile>
      <anchor>034c9bb438f63aa517f1ab2fc1a6a66b</anchor>
      <arglist></arglist>
    </member>
    <member kind="variable">
      <type>unsigned int</type>
      <name>branch_path_node</name>
      <anchorfile>struct__Softpipe__Op__Info.html</anchorfile>
      <anchor>7bee54feda7a0bfb08b607ed1515b812</anchor>
      <arglist></arglist>
    </member>
    <member kind="variable">
      <type>unsigned int</type>
      <name>scheduled</name>
      <anchorfile>struct__Softpipe__Op__Info.html</anchorfile>
      <anchor>36a815a8c5d408f4e7c86d0c5df13ab4</anchor>
      <arglist></arglist>
    </member>
  </compound>
  <compound kind="struct">
    <name>Qnode</name>
    <filename>structQnode.html</filename>
    <member kind="variable">
      <type>SM_Oper *</type>
      <name>oper</name>
      <anchorfile>structQnode.html</anchorfile>
      <anchor>3aec68cc8a3e9cc21bfa9cdcc705b3f9</anchor>
      <arglist></arglist>
    </member>
    <member kind="variable">
      <type>int</type>
      <name>priority</name>
      <anchorfile>structQnode.html</anchorfile>
      <anchor>b3dc52a9270a3beba7d4fcc3b9c0fc10</anchor>
      <arglist></arglist>
    </member>
    <member kind="variable">
      <type>Qnode *</type>
      <name>next_qnode</name>
      <anchorfile>structQnode.html</anchorfile>
      <anchor>baf1d277b57755bb0116b37e5edb5708</anchor>
      <arglist></arglist>
    </member>
    <member kind="variable">
      <type>Qnode *</type>
      <name>prev_qnode</name>
      <anchorfile>structQnode.html</anchorfile>
      <anchor>ef9b69f39290fed76fa1d90abf33abf3</anchor>
      <arglist></arglist>
    </member>
  </compound>
  <compound kind="struct">
    <name>Queue</name>
    <filename>structQueue.html</filename>
    <member kind="variable">
      <type>Qnode *</type>
      <name>head</name>
      <anchorfile>structQueue.html</anchorfile>
      <anchor>ce2d277946f665c6b87a545102632cdf</anchor>
      <arglist></arglist>
    </member>
    <member kind="variable">
      <type>Qnode *</type>
      <name>tail</name>
      <anchorfile>structQueue.html</anchorfile>
      <anchor>fee6a6594dac629a1b7cb7d1b66c1242</anchor>
      <arglist></arglist>
    </member>
  </compound>
</tagfile>
