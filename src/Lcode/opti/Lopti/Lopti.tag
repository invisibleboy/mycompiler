<?xml version='1.0' encoding='ISO-8859-1' standalone='yes' ?>
<tagfile>
  <compound kind="file">
    <name>l_benchmark_opti.c</name>
    <path>/mnt/hgfs/mkt/Desktop/openimpact/src/Lcode/opti/Lopti/</path>
    <filename>l__benchmark__opti_8c</filename>
    <includes id="l__opti_8h" name="l_opti.h" local="yes" imported="no">l_opti.h</includes>
    <class kind="struct">L_Cb_Map</class>
    <class kind="struct">L_Short_Info</class>
    <class kind="struct">L_Ind_Var_Info</class>
    <class kind="struct">L_Ind_Var_Branch_Info</class>
    <member kind="define">
      <type>#define</type>
      <name>PRUNE_LOOP_ATTR</name>
      <anchorfile>l__benchmark__opti_8c.html</anchorfile>
      <anchor>47d2b2483bed2237617cfae4f28bca1f</anchor>
      <arglist></arglist>
    </member>
    <member kind="define">
      <type>#define</type>
      <name>MAX_NUM_TUNE_ITERATION</name>
      <anchorfile>l__benchmark__opti_8c.html</anchorfile>
      <anchor>e5b1367a53c3e2f785b29f5260b72689</anchor>
      <arglist></arglist>
    </member>
    <member kind="define">
      <type>#define</type>
      <name>MAX_IND_VAR_BRANCHES</name>
      <anchorfile>l__benchmark__opti_8c.html</anchorfile>
      <anchor>213444f6d61d3ecd07bf480e3524d69a</anchor>
      <arglist></arglist>
    </member>
    <member kind="define">
      <type>#define</type>
      <name>MAX_SHORT_OPERANDS</name>
      <anchorfile>l__benchmark__opti_8c.html</anchorfile>
      <anchor>9243d381ce42d97da12bea8d5b1c9d2e</anchor>
      <arglist></arglist>
    </member>
    <member kind="define">
      <type>#define</type>
      <name>CHAR1_MASK</name>
      <anchorfile>l__benchmark__opti_8c.html</anchorfile>
      <anchor>c9941bd0d12dc7b686b6b152fc995f2f</anchor>
      <arglist></arglist>
    </member>
    <member kind="define">
      <type>#define</type>
      <name>CHAR2_MASK</name>
      <anchorfile>l__benchmark__opti_8c.html</anchorfile>
      <anchor>063687b3528d73166e4a8529f26bad2f</anchor>
      <arglist></arglist>
    </member>
    <member kind="function">
      <type>int</type>
      <name>L_do_benchmark_tuning</name>
      <anchorfile>l__benchmark__opti_8c.html</anchorfile>
      <anchor>5963474694db27a1aade1346abaa0136</anchor>
      <arglist>(L_Func *fn)</arglist>
    </member>
    <member kind="function" static="yes">
      <type>static int</type>
      <name>L_do_tott_branch_tuning</name>
      <anchorfile>l__benchmark__opti_8c.html</anchorfile>
      <anchor>ce3d91d96955d2d0b513510de09705b5</anchor>
      <arglist>(L_Func *fn)</arglist>
    </member>
    <member kind="function" static="yes">
      <type>static int</type>
      <name>L_same_cb</name>
      <anchorfile>l__benchmark__opti_8c.html</anchorfile>
      <anchor>e29b2be0c9de83143dff82ceee005f7c</anchor>
      <arglist>(L_Cb *cb1, L_Cb *cb2)</arglist>
    </member>
    <member kind="function" static="yes">
      <type>static int</type>
      <name>L_short_operand</name>
      <anchorfile>l__benchmark__opti_8c.html</anchorfile>
      <anchor>16b8d4e9c23919520a0c5faf2245179f</anchor>
      <arglist>(L_Operand *operand, L_Short_Info *short_info, int *index)</arglist>
    </member>
    <member kind="function" static="yes">
      <type>static int</type>
      <name>L_add_short_operand</name>
      <anchorfile>l__benchmark__opti_8c.html</anchorfile>
      <anchor>7df519ca59f062639e15c2b20edbd693</anchor>
      <arglist>(L_Short_Info *short_info, L_Oper *op)</arglist>
    </member>
    <member kind="function" static="yes">
      <type>static int</type>
      <name>L_can_longword_convert_loop</name>
      <anchorfile>l__benchmark__opti_8c.html</anchorfile>
      <anchor>6aa2bd196a7e1b1ae5c73205dcd79efe</anchor>
      <arglist>(L_Loop *loop, L_Short_Info *short_info)</arglist>
    </member>
    <member kind="function" static="yes">
      <type>static int</type>
      <name>L_find_character_oper</name>
      <anchorfile>l__benchmark__opti_8c.html</anchorfile>
      <anchor>07f3d4ff1e8c3b24c2e4560823338c2f</anchor>
      <arglist>(L_Loop *loop, L_Short_Info *short_info, L_Operand **index_operand)</arglist>
    </member>
    <member kind="function" static="yes">
      <type>static int</type>
      <name>L_cb_in_loop</name>
      <anchorfile>l__benchmark__opti_8c.html</anchorfile>
      <anchor>7a7db0d916ed9b70294760cf58d77655</anchor>
      <arglist>(L_Cb *cb, int *loop_cb, int num_cb)</arglist>
    </member>
    <member kind="function" static="yes">
      <type>static int</type>
      <name>L_find_ind_var</name>
      <anchorfile>l__benchmark__opti_8c.html</anchorfile>
      <anchor>35003d31310c40181c4642c254a5f227</anchor>
      <arglist>(L_Operand *ind_var, L_Ind_Var_Info *ind_var_info, int num_ind_var)</arglist>
    </member>
    <member kind="function" static="yes">
      <type>static L_Cb *</type>
      <name>L_copy_cb</name>
      <anchorfile>l__benchmark__opti_8c.html</anchorfile>
      <anchor>f47aa24bda339c346163e14ddb4a14ca</anchor>
      <arglist>(L_Cb *cb, L_Cb *exit_cb, L_Cb *insert_after_cb, L_Cb_Map *cb_map)</arglist>
    </member>
    <member kind="function" static="yes">
      <type>static L_Cb *</type>
      <name>L_copy_loop</name>
      <anchorfile>l__benchmark__opti_8c.html</anchorfile>
      <anchor>0aa50bfae93fb31c16eda52fecab17e6</anchor>
      <arglist>(L_Loop *loop, L_Cb *exit_cb)</arglist>
    </member>
    <member kind="function" static="yes">
      <type>static L_Cb *</type>
      <name>L_make_char_loop</name>
      <anchorfile>l__benchmark__opti_8c.html</anchorfile>
      <anchor>a5e1c32a53283bb3e2a08af5df8b01f3</anchor>
      <arglist>(L_Loop *loop, L_Cb *exit_cb, L_Cb *insert_after, int delete_back_edge)</arglist>
    </member>
    <member kind="function" static="yes">
      <type>static L_Oper *</type>
      <name>L_find_reaching_def</name>
      <anchorfile>l__benchmark__opti_8c.html</anchorfile>
      <anchor>beecf294dda2e672e24d6936952ed950</anchor>
      <arglist>(L_Operand *src, L_Oper *use_op)</arglist>
    </member>
    <member kind="function">
      <type>L_Oper *</type>
      <name>L_find_first_reaching_oper</name>
      <anchorfile>l__benchmark__opti_8c.html</anchorfile>
      <anchor>b7ab27451dcb0ed5e5b9ab79f677f5c8</anchor>
      <arglist>(L_Cb *cb, L_Oper *oper)</arglist>
    </member>
    <member kind="function" static="yes">
      <type>static void</type>
      <name>L_mark_needed_opers</name>
      <anchorfile>l__benchmark__opti_8c.html</anchorfile>
      <anchor>7e93c6dcf4b0d126aa8e2064477e08b0</anchor>
      <arglist>(L_Cb *cb, L_Oper *oper, L_Short_Info *short_info)</arglist>
    </member>
    <member kind="function">
      <type>void</type>
      <name>L_trim_char2_loops</name>
      <anchorfile>l__benchmark__opti_8c.html</anchorfile>
      <anchor>e73ba99edb3d10940b21611f155ac94b</anchor>
      <arglist>(L_Cb *char_loop1, L_Cb *char_loop2)</arglist>
    </member>
    <member kind="function" static="yes">
      <type>static void</type>
      <name>L_expand_longword_exit</name>
      <anchorfile>l__benchmark__opti_8c.html</anchorfile>
      <anchor>1b73b61e0a9fc12c5b9f32865be4f069</anchor>
      <arglist>(L_Loop *loop, L_Cb *cb, L_Oper *branch, L_Short_Info *short_info, int size)</arglist>
    </member>
    <member kind="function" static="yes">
      <type>static void</type>
      <name>L_expand_longword_branch</name>
      <anchorfile>l__benchmark__opti_8c.html</anchorfile>
      <anchor>4b5a4ba13caea63bd44baae8ab1bb63f</anchor>
      <arglist>(L_Loop *loop, L_Cb *cb, L_Oper *branch, int size, L_Operand *index_operand)</arglist>
    </member>
    <member kind="function">
      <type>int</type>
      <name>L_do_longword_loop_conversion</name>
      <anchorfile>l__benchmark__opti_8c.html</anchorfile>
      <anchor>07d0a33e4f13c799460306aa6deca460</anchor>
      <arglist>(L_Loop *loop)</arglist>
    </member>
    <member kind="variable" static="yes">
      <type>static int</type>
      <name>num_exit_cb</name>
      <anchorfile>l__benchmark__opti_8c.html</anchorfile>
      <anchor>b1cfde2a8a2a38ff5462f35e3761c473</anchor>
      <arglist></arglist>
    </member>
    <member kind="variable" static="yes">
      <type>static int</type>
      <name>num_cb</name>
      <anchorfile>l__benchmark__opti_8c.html</anchorfile>
      <anchor>fe78219112b6a7feb42f7349266dd872</anchor>
      <arglist></arglist>
    </member>
    <member kind="variable" static="yes">
      <type>static int *</type>
      <name>loop_cb</name>
      <anchorfile>l__benchmark__opti_8c.html</anchorfile>
      <anchor>d19829152ffbdb14f9230c9a55a5d131</anchor>
      <arglist></arglist>
    </member>
    <member kind="variable" static="yes">
      <type>static int *</type>
      <name>exit_cb</name>
      <anchorfile>l__benchmark__opti_8c.html</anchorfile>
      <anchor>340207743e8aeb80a087395e330b6cf1</anchor>
      <arglist></arglist>
    </member>
  </compound>
  <compound kind="file">
    <name>l_branch.c</name>
    <path>/mnt/hgfs/mkt/Desktop/openimpact/src/Lcode/opti/Lopti/</path>
    <filename>l__branch_8c</filename>
    <includes id="l__opti_8h" name="l_opti.h" local="yes" imported="no">l_opti.h</includes>
    <member kind="function">
      <type>void</type>
      <name>L_mark_branches</name>
      <anchorfile>l__branch_8c.html</anchorfile>
      <anchor>41f517d0fdce4857172011c426f41c3a</anchor>
      <arglist>(L_Func *fn)</arglist>
    </member>
    <member kind="function" static="yes">
      <type>static void</type>
      <name>L_remove_old_classifications</name>
      <anchorfile>l__branch_8c.html</anchorfile>
      <anchor>591f761b4ec391d9bc249e08db4e1b68</anchor>
      <arglist>(L_Func *fn)</arglist>
    </member>
    <member kind="function" static="yes">
      <type>static void</type>
      <name>L_do_classify_branches</name>
      <anchorfile>l__branch_8c.html</anchorfile>
      <anchor>45dc043e34b8d4a00f7ff5892f17cc28</anchor>
      <arglist>(L_Func *fn)</arglist>
    </member>
    <member kind="function">
      <type>void</type>
      <name>L_classify_branches</name>
      <anchorfile>l__branch_8c.html</anchorfile>
      <anchor>2aeff608093363891de333fd52b2577b</anchor>
      <arglist>(L_Func *fn)</arglist>
    </member>
  </compound>
  <compound kind="file">
    <name>l_branch.h</name>
    <path>/mnt/hgfs/mkt/Desktop/openimpact/src/Lcode/opti/Lopti/</path>
    <filename>l__branch_8h</filename>
    <member kind="define">
      <type>#define</type>
      <name>L_OPER_CBR</name>
      <anchorfile>l__branch_8h.html</anchorfile>
      <anchor>8127d4ca5ec59d799dbe272ec87e68d1</anchor>
      <arglist></arglist>
    </member>
    <member kind="define">
      <type>#define</type>
      <name>L_OPER_UBR</name>
      <anchorfile>l__branch_8h.html</anchorfile>
      <anchor>72c35f50aa9ef1d585d2c79d0fa63950</anchor>
      <arglist></arglist>
    </member>
    <member kind="define">
      <type>#define</type>
      <name>L_OPER_JSR</name>
      <anchorfile>l__branch_8h.html</anchorfile>
      <anchor>754d30a5273bc71a3a1aab4f8f008687</anchor>
      <arglist></arglist>
    </member>
    <member kind="define">
      <type>#define</type>
      <name>L_OPER_RTS</name>
      <anchorfile>l__branch_8h.html</anchorfile>
      <anchor>6384776eb56685bc883a3430c2246d22</anchor>
      <arglist></arglist>
    </member>
    <member kind="define">
      <type>#define</type>
      <name>L_OPER_JRG</name>
      <anchorfile>l__branch_8h.html</anchorfile>
      <anchor>c1a2f6b40d5e6fa7789a0877ba53c56a</anchor>
      <arglist></arglist>
    </member>
    <member kind="define">
      <type>#define</type>
      <name>L_BR_LOOPBACK_INNER</name>
      <anchorfile>l__branch_8h.html</anchorfile>
      <anchor>ef17e4c8a0ac35f916d1e9407c712d6d</anchor>
      <arglist></arglist>
    </member>
    <member kind="define">
      <type>#define</type>
      <name>L_BR_LOOPBACK_OUTER</name>
      <anchorfile>l__branch_8h.html</anchorfile>
      <anchor>495e09b94b5283964d6f4310a12e0e84</anchor>
      <arglist></arglist>
    </member>
    <member kind="define">
      <type>#define</type>
      <name>L_BR_LOOPEXIT_INNER</name>
      <anchorfile>l__branch_8h.html</anchorfile>
      <anchor>ebafc97598753a5d932b058b8478130c</anchor>
      <arglist></arglist>
    </member>
    <member kind="define">
      <type>#define</type>
      <name>L_BR_LOOPEXIT_OUTER</name>
      <anchorfile>l__branch_8h.html</anchorfile>
      <anchor>1bc4d994308c100471f73fe58b210e9e</anchor>
      <arglist></arglist>
    </member>
    <member kind="define">
      <type>#define</type>
      <name>L_BR_NONLOOP_INNER</name>
      <anchorfile>l__branch_8h.html</anchorfile>
      <anchor>f47da3ca60d74cbea4ad65443557da54</anchor>
      <arglist></arglist>
    </member>
    <member kind="define">
      <type>#define</type>
      <name>L_BR_NONLOOP_OUTER</name>
      <anchorfile>l__branch_8h.html</anchorfile>
      <anchor>170ba3012a2d2f1a2251d20d6299fd53</anchor>
      <arglist></arglist>
    </member>
    <member kind="define">
      <type>#define</type>
      <name>L_BR_NONLOOP_STLN</name>
      <anchorfile>l__branch_8h.html</anchorfile>
      <anchor>b285f5378559a96efafb48693ab9a758</anchor>
      <arglist></arglist>
    </member>
    <member kind="define">
      <type>#define</type>
      <name>L_BR_LOOPBACK_INNER_NAME</name>
      <anchorfile>l__branch_8h.html</anchorfile>
      <anchor>12d49596b7468c69d988a93acd1fbc2a</anchor>
      <arglist></arglist>
    </member>
    <member kind="define">
      <type>#define</type>
      <name>L_BR_LOOPBACK_OUTER_NAME</name>
      <anchorfile>l__branch_8h.html</anchorfile>
      <anchor>a8f1d8fccc8d48f8a4063ecd97a5b4bf</anchor>
      <arglist></arglist>
    </member>
    <member kind="define">
      <type>#define</type>
      <name>L_BR_LOOPEXIT_INNER_NAME</name>
      <anchorfile>l__branch_8h.html</anchorfile>
      <anchor>6cd351c3191475d4a9fcc7e0757d2e15</anchor>
      <arglist></arglist>
    </member>
    <member kind="define">
      <type>#define</type>
      <name>L_BR_LOOPEXIT_OUTER_NAME</name>
      <anchorfile>l__branch_8h.html</anchorfile>
      <anchor>9909accb67ca50e646cea43cb57500ad</anchor>
      <arglist></arglist>
    </member>
    <member kind="define">
      <type>#define</type>
      <name>L_BR_NONLOOP_INNER_NAME</name>
      <anchorfile>l__branch_8h.html</anchorfile>
      <anchor>7813c4d5c3bf0dc67937be0b31b48d53</anchor>
      <arglist></arglist>
    </member>
    <member kind="define">
      <type>#define</type>
      <name>L_BR_NONLOOP_OUTER_NAME</name>
      <anchorfile>l__branch_8h.html</anchorfile>
      <anchor>7199958faeb7398beb1afe0ca655fa61</anchor>
      <arglist></arglist>
    </member>
    <member kind="define">
      <type>#define</type>
      <name>L_BR_NONLOOP_STLN_NAME</name>
      <anchorfile>l__branch_8h.html</anchorfile>
      <anchor>5692c7b227a36838303e082ffc517b48</anchor>
      <arglist></arglist>
    </member>
    <member kind="define">
      <type>#define</type>
      <name>L_BR_LOOPNEST_NAME</name>
      <anchorfile>l__branch_8h.html</anchorfile>
      <anchor>226662d05364866f023a5411624b4169</anchor>
      <arglist></arglist>
    </member>
    <member kind="function">
      <type>void</type>
      <name>L_mark_branches</name>
      <anchorfile>l__branch_8h.html</anchorfile>
      <anchor>208c3239c984b6caf196a544bf8f7e72</anchor>
      <arglist>(L_Func *)</arglist>
    </member>
    <member kind="function">
      <type>void</type>
      <name>L_classify_branches</name>
      <anchorfile>l__branch_8h.html</anchorfile>
      <anchor>6d7c7899ab7b7cb651e7ba68421e8112</anchor>
      <arglist>(L_Func *)</arglist>
    </member>
  </compound>
  <compound kind="file">
    <name>l_codegen.c</name>
    <path>/mnt/hgfs/mkt/Desktop/openimpact/src/Lcode/opti/Lopti/</path>
    <filename>l__codegen_8c</filename>
    <includes id="l__opti_8h" name="l_opti.h" local="yes" imported="no">l_opti.h</includes>
    <member kind="function" static="yes">
      <type>static void</type>
      <name>process_input</name>
      <anchorfile>l__codegen_8c.html</anchorfile>
      <anchor>7e66299580e638dd1368d29ee0812a35</anchor>
      <arglist>(void)</arglist>
    </member>
    <member kind="function">
      <type>void</type>
      <name>L_gen_code</name>
      <anchorfile>l__codegen_8c.html</anchorfile>
      <anchor>6346124c5fad5929833a7f7084f6be8c</anchor>
      <arglist>(Parm_Macro_List *command_line_macro_list)</arglist>
    </member>
  </compound>
  <compound kind="file">
    <name>l_danger.c</name>
    <path>/mnt/hgfs/mkt/Desktop/openimpact/src/Lcode/opti/Lopti/</path>
    <filename>l__danger_8c</filename>
    <includes id="l__opti_8h" name="l_opti.h" local="yes" imported="no">l_opti.h</includes>
    <member kind="function" static="yes">
      <type>static L_Danger_Ext *</type>
      <name>L_new_danger_ext</name>
      <anchorfile>l__danger_8c.html</anchorfile>
      <anchor>2063d6073bdf943d59418848a2817b40</anchor>
      <arglist>()</arglist>
    </member>
    <member kind="function">
      <type>void</type>
      <name>L_delete_danger_ext</name>
      <anchorfile>l__danger_8c.html</anchorfile>
      <anchor>35a287695859480b6dc5aac650c03544</anchor>
      <arglist>(L_Cb *cb)</arglist>
    </member>
    <member kind="function">
      <type>void</type>
      <name>L_delete_all_danger_ext</name>
      <anchorfile>l__danger_8c.html</anchorfile>
      <anchor>966b3dfad44c93e5c6a3ad55d8ffc7f2</anchor>
      <arglist>(L_Func *fn)</arglist>
    </member>
    <member kind="function">
      <type>void</type>
      <name>L_print_all_danger_info</name>
      <anchorfile>l__danger_8c.html</anchorfile>
      <anchor>9f8e5c6d1936b6405c19f967c8ec54b5</anchor>
      <arglist>(L_Func *fn)</arglist>
    </member>
    <member kind="function" static="yes">
      <type>static void</type>
      <name>L_reset_visited_flag</name>
      <anchorfile>l__danger_8c.html</anchorfile>
      <anchor>2e1c420e1b0dfdac91de715bbf2c4971</anchor>
      <arglist>(L_Func *fn)</arglist>
    </member>
    <member kind="function" static="yes">
      <type>static void</type>
      <name>L_search_predecessors</name>
      <anchorfile>l__danger_8c.html</anchorfile>
      <anchor>48802ffc8f17892b27f252b9ad5fb769</anchor>
      <arglist>(int *sub_call, int *gen_sub_call, int *sync, int *store, L_Cb *cb, L_Cb *dest_cb)</arglist>
    </member>
    <member kind="function">
      <type>void</type>
      <name>L_compute_danger_info</name>
      <anchorfile>l__danger_8c.html</anchorfile>
      <anchor>657277eb07d1d35c053bb7251e8235d7</anchor>
      <arglist>(L_Func *fn)</arglist>
    </member>
  </compound>
  <compound kind="file">
    <name>l_danger.h</name>
    <path>/mnt/hgfs/mkt/Desktop/openimpact/src/Lcode/opti/Lopti/</path>
    <filename>l__danger_8h</filename>
    <class kind="struct">L_Danger_Ext</class>
    <member kind="define">
      <type>#define</type>
      <name>L_DANGER_EXT_IDENTIFIER</name>
      <anchorfile>l__danger_8h.html</anchorfile>
      <anchor>75c45967200c612f618ddc1facfeb317</anchor>
      <arglist></arglist>
    </member>
    <member kind="function">
      <type>void</type>
      <name>L_delete_danger_ext</name>
      <anchorfile>l__danger_8h.html</anchorfile>
      <anchor>fe00f77e77accba8f05a3c4c38d2f3b2</anchor>
      <arglist>(L_Cb *)</arglist>
    </member>
    <member kind="function">
      <type>void</type>
      <name>L_delete_all_danger_ext</name>
      <anchorfile>l__danger_8h.html</anchorfile>
      <anchor>b3cc47d84d37bac796b507b02b14a868</anchor>
      <arglist>(L_Func *)</arglist>
    </member>
    <member kind="function">
      <type>void</type>
      <name>L_print_all_danger_info</name>
      <anchorfile>l__danger_8h.html</anchorfile>
      <anchor>a5e7c4c6bc7918b9ef56cf45a76d8165</anchor>
      <arglist>(L_Func *)</arglist>
    </member>
    <member kind="function">
      <type>void</type>
      <name>L_compute_danger_info</name>
      <anchorfile>l__danger_8h.html</anchorfile>
      <anchor>add834421cb5bc62a0db74d9d5048b4a</anchor>
      <arglist>(L_Func *)</arglist>
    </member>
  </compound>
  <compound kind="file">
    <name>l_disjvreg.c</name>
    <path>/mnt/hgfs/mkt/Desktop/openimpact/src/Lcode/opti/Lopti/</path>
    <filename>l__disjvreg_8c</filename>
    <includes id="l__opti_8h" name="l_opti.h" local="yes" imported="no">l_opti.h</includes>
    <includes id="l__disjvreg_8h" name="l_disjvreg.h" local="yes" imported="no">l_disjvreg.h</includes>
    <class kind="struct">_LiveRange</class>
    <member kind="define">
      <type>#define</type>
      <name>RDVR_LR_NOSPLIT</name>
      <anchorfile>l__disjvreg_8c.html</anchorfile>
      <anchor>b00890731f4da1b78f501ec6cd9d6808</anchor>
      <arglist></arglist>
    </member>
    <member kind="define">
      <type>#define</type>
      <name>RDVR_DO_RENAME</name>
      <anchorfile>l__disjvreg_8c.html</anchorfile>
      <anchor>d95a849fc0ed33d928a77ebee11f16cd</anchor>
      <arglist></arglist>
    </member>
    <member kind="define">
      <type>#define</type>
      <name>RDVR_NO_RENAME</name>
      <anchorfile>l__disjvreg_8c.html</anchorfile>
      <anchor>e89b9979a16b2809d611a4337390313b</anchor>
      <arglist></arglist>
    </member>
    <member kind="typedef">
      <type>_LiveRange</type>
      <name>LiveRange</name>
      <anchorfile>l__disjvreg_8c.html</anchorfile>
      <anchor>6c903c1d932a9855b700d0904e2da854</anchor>
      <arglist></arglist>
    </member>
    <member kind="function" static="yes">
      <type>static LiveRange *</type>
      <name>L_rdvr_new_live_range</name>
      <anchorfile>l__disjvreg_8c.html</anchorfile>
      <anchor>e0dd88471227b05e981395b9cbb417e8</anchor>
      <arglist>(L_Oper *def_op, L_Operand *def_dest)</arglist>
    </member>
    <member kind="function" static="yes">
      <type>static LiveRange *</type>
      <name>L_rdvr_delete_live_range</name>
      <anchorfile>l__disjvreg_8c.html</anchorfile>
      <anchor>4752f55c53e4bae7d8586210d6f2e75e</anchor>
      <arglist>(LiveRange *lr)</arglist>
    </member>
    <member kind="function" static="yes">
      <type>static void</type>
      <name>L_rdvr_rename_live_range</name>
      <anchorfile>l__disjvreg_8c.html</anchorfile>
      <anchor>b481e47a5e8df46db50f621626c7bcc1</anchor>
      <arglist>(L_Func *fn, LiveRange *lr, int vreg, int *oparray)</arglist>
    </member>
    <member kind="function">
      <type>void</type>
      <name>L_rdvr_print_liverange</name>
      <anchorfile>l__disjvreg_8c.html</anchorfile>
      <anchor>9ba4ac7e7930661d7c3048c2bcb67ad6</anchor>
      <arglist>(LiveRange *lr)</arglist>
    </member>
    <member kind="function" static="yes">
      <type>static void</type>
      <name>L_rdvr_dataflow_analysis</name>
      <anchorfile>l__disjvreg_8c.html</anchorfile>
      <anchor>44d79b38a28f13fc4c1a9d4957f06f15</anchor>
      <arglist>(L_Func *fn)</arglist>
    </member>
    <member kind="function" static="yes">
      <type>static void</type>
      <name>L_rdvr_create_pre_live_ranges</name>
      <anchorfile>l__disjvreg_8c.html</anchorfile>
      <anchor>50f184cc52da227cac8c9c72135e9613</anchor>
      <arglist>(L_Func *fn, HashTable vhash, Set nosplit)</arglist>
    </member>
    <member kind="function" static="yes">
      <type>static void</type>
      <name>L_rdvr_annotate_interference</name>
      <anchorfile>l__disjvreg_8c.html</anchorfile>
      <anchor>fe3459025e19b22f69643794aaf2bdf1</anchor>
      <arglist>(L_Func *fn, List *plrlist)</arglist>
    </member>
    <member kind="function" static="yes">
      <type>static int</type>
      <name>L_rdvr_generate_disjvreg</name>
      <anchorfile>l__disjvreg_8c.html</anchorfile>
      <anchor>a30b1cbc22ca72636632d4264ef41654</anchor>
      <arglist>(L_Func *fn, List *plrlist, int do_rename, Set nosplit)</arglist>
    </member>
    <member kind="function" static="yes">
      <type>static void</type>
      <name>L_rdvr_add_ref_op</name>
      <anchorfile>l__disjvreg_8c.html</anchorfile>
      <anchor>5cfd5d69499dff5b52bab265c5065756</anchor>
      <arglist>(HashTable vhash, L_Oper *oper, L_Operand *opd)</arglist>
    </member>
    <member kind="function" static="yes">
      <type>static int</type>
      <name>L_rdvr_coalesce_live_ranges</name>
      <anchorfile>l__disjvreg_8c.html</anchorfile>
      <anchor>0d060e19f04e82a785e8cffacdf94454</anchor>
      <arglist>(L_Func *fn, List *plrlist)</arglist>
    </member>
    <member kind="function" static="yes">
      <type>static void</type>
      <name>L_rdvr_cleanup</name>
      <anchorfile>l__disjvreg_8c.html</anchorfile>
      <anchor>e6d8d2d0928deb0d8f7d3c1439ef7717</anchor>
      <arglist>(List *plrlist)</arglist>
    </member>
    <member kind="function">
      <type>void</type>
      <name>L_rename_disjoint_virtual_registers</name>
      <anchorfile>l__disjvreg_8c.html</anchorfile>
      <anchor>53b6688a9927650d0ff3ff44b2288e9c</anchor>
      <arglist>(L_Func *fn, Set nosplit)</arglist>
    </member>
    <member kind="function">
      <type>void</type>
      <name>L_rename_coalesce_disjoint_virtual_registers</name>
      <anchorfile>l__disjvreg_8c.html</anchorfile>
      <anchor>180c43d98984c2da1d824673478dde90</anchor>
      <arglist>(L_Func *fn, Set nosplit)</arglist>
    </member>
    <member kind="function" static="yes">
      <type>static int</type>
      <name>L_rdvr_create_live_ranges</name>
      <anchorfile>l__disjvreg_8c.html</anchorfile>
      <anchor>ace0bae2142d6d7c05c757ff1cf9b83f</anchor>
      <arglist>(L_Func *fn, LiveRange **LiveRanges)</arglist>
    </member>
    <member kind="function" static="yes">
      <type>static void</type>
      <name>L_rdvr_merge_into</name>
      <anchorfile>l__disjvreg_8c.html</anchorfile>
      <anchor>3bc8465c38e28d338cf655d124244eeb</anchor>
      <arglist>(LiveRange *lr1, LiveRange *lr2)</arglist>
    </member>
    <member kind="function" static="yes">
      <type>static int</type>
      <name>L_check_for_coalescing</name>
      <anchorfile>l__disjvreg_8c.html</anchorfile>
      <anchor>e58898f54174a6d452615bb6272ca5c6</anchor>
      <arglist>(L_Func *fn, LiveRange *lr1, LiveRange *lr2)</arglist>
    </member>
    <member kind="function" static="yes">
      <type>static void</type>
      <name>L_rdvr_compress_live_ranges</name>
      <anchorfile>l__disjvreg_8c.html</anchorfile>
      <anchor>409c0de16fab0b5f0c050e07166711ac</anchor>
      <arglist>(LiveRange **LiveRanges, int numLiveRange)</arglist>
    </member>
    <member kind="function" static="yes">
      <type>static void</type>
      <name>L_rdvr_merge_live_ranges</name>
      <anchorfile>l__disjvreg_8c.html</anchorfile>
      <anchor>5dd244fdc07c34f87da80d094b68bc3b</anchor>
      <arglist>(L_Func *fn, LiveRange **LiveRanges, int numLiveRange, Set nosplit)</arglist>
    </member>
    <member kind="function" static="yes">
      <type>static void</type>
      <name>L_rdvr_compute_liveness_live_ranges</name>
      <anchorfile>l__disjvreg_8c.html</anchorfile>
      <anchor>e652184a3cde49cfaa413b36af352fb8</anchor>
      <arglist>(L_Func *fn, LiveRange **LiveRanges, int numLiveRange)</arglist>
    </member>
    <member kind="function" static="yes">
      <type>static void</type>
      <name>L_rdvr_delete_live_ranges</name>
      <anchorfile>l__disjvreg_8c.html</anchorfile>
      <anchor>ba6755e4302cf6834b28506070b7c72f</anchor>
      <arglist>(LiveRange **LiveRanges, int numLiveRange)</arglist>
    </member>
    <member kind="function">
      <type>void</type>
      <name>L_coalesce_live_ranges</name>
      <anchorfile>l__disjvreg_8c.html</anchorfile>
      <anchor>1a24a0efe503b2d1e6dcc62f9237c844</anchor>
      <arglist>(L_Func *fn)</arglist>
    </member>
    <member kind="variable">
      <type>L_Alloc_Pool *</type>
      <name>LiveRange_pool</name>
      <anchorfile>l__disjvreg_8c.html</anchorfile>
      <anchor>4f319c73196b088034364e0e563dbf30</anchor>
      <arglist></arglist>
    </member>
  </compound>
  <compound kind="file">
    <name>l_disjvreg.h</name>
    <path>/mnt/hgfs/mkt/Desktop/openimpact/src/Lcode/opti/Lopti/</path>
    <filename>l__disjvreg_8h</filename>
    <member kind="function">
      <type>void</type>
      <name>L_rename_disjoint_virtual_registers</name>
      <anchorfile>l__disjvreg_8h.html</anchorfile>
      <anchor>53b6688a9927650d0ff3ff44b2288e9c</anchor>
      <arglist>(L_Func *fn, Set nosplit)</arglist>
    </member>
    <member kind="function">
      <type>void</type>
      <name>L_rename_coalesce_disjoint_virtual_registers</name>
      <anchorfile>l__disjvreg_8h.html</anchorfile>
      <anchor>180c43d98984c2da1d824673478dde90</anchor>
      <arglist>(L_Func *fn, Set nosplit)</arglist>
    </member>
    <member kind="function">
      <type>void</type>
      <name>L_coalesce_live_ranges</name>
      <anchorfile>l__disjvreg_8h.html</anchorfile>
      <anchor>1a24a0efe503b2d1e6dcc62f9237c844</anchor>
      <arglist>(L_Func *fn)</arglist>
    </member>
  </compound>
  <compound kind="file">
    <name>l_evaluate.c</name>
    <path>/mnt/hgfs/mkt/Desktop/openimpact/src/Lcode/opti/Lopti/</path>
    <filename>l__evaluate_8c</filename>
    <includes id="l__opti_8h" name="l_opti.h" local="yes" imported="no">l_opti.h</includes>
    <member kind="define">
      <type>#define</type>
      <name>ERR</name>
      <anchorfile>l__evaluate_8c.html</anchorfile>
      <anchor>735563036dced0b7d6cc98f97ea4978b</anchor>
      <arglist></arglist>
    </member>
    <member kind="function">
      <type>ITintmax</type>
      <name>L_evaluate_int_arithmetic</name>
      <anchorfile>l__evaluate_8c.html</anchorfile>
      <anchor>879bfcaddf4697210d7d02b2c45c6b77</anchor>
      <arglist>(L_Oper *oper)</arglist>
    </member>
    <member kind="function">
      <type>float</type>
      <name>L_evaluate_flt_arithmetic</name>
      <anchorfile>l__evaluate_8c.html</anchorfile>
      <anchor>0c082aa096d39c0c13913d6b898cbbf1</anchor>
      <arglist>(L_Oper *oper)</arglist>
    </member>
    <member kind="function">
      <type>double</type>
      <name>L_evaluate_dbl_arithmetic</name>
      <anchorfile>l__evaluate_8c.html</anchorfile>
      <anchor>ba89329f1ee3f93c988099e056f1ea39</anchor>
      <arglist>(L_Oper *oper)</arglist>
    </member>
    <member kind="function">
      <type>int</type>
      <name>L_evaluate_int_compare_with_sources</name>
      <anchorfile>l__evaluate_8c.html</anchorfile>
      <anchor>e02d3432cb6b07a641b217d273131b51</anchor>
      <arglist>(L_Oper *oper, ITintmax s1, ITintmax s2)</arglist>
    </member>
    <member kind="function">
      <type>int</type>
      <name>L_evaluate_int_compare</name>
      <anchorfile>l__evaluate_8c.html</anchorfile>
      <anchor>297b5b261f75044033d2902302546db2</anchor>
      <arglist>(L_Oper *oper)</arglist>
    </member>
    <member kind="function">
      <type>int</type>
      <name>L_evaluate_flt_compare_with_sources</name>
      <anchorfile>l__evaluate_8c.html</anchorfile>
      <anchor>8e8b6a83f1f18f1aaf1cc01fe656236f</anchor>
      <arglist>(L_Oper *oper, float s1, float s2)</arglist>
    </member>
    <member kind="function">
      <type>int</type>
      <name>L_evaluate_flt_compare</name>
      <anchorfile>l__evaluate_8c.html</anchorfile>
      <anchor>4301b40e1a8511c490a6ca4c2fb31407</anchor>
      <arglist>(L_Oper *oper)</arglist>
    </member>
    <member kind="function">
      <type>int</type>
      <name>L_evaluate_dbl_compare_with_sources</name>
      <anchorfile>l__evaluate_8c.html</anchorfile>
      <anchor>7cce3742a931e4e7dcf837ff1a3c9d2f</anchor>
      <arglist>(L_Oper *oper, double s1, double s2)</arglist>
    </member>
    <member kind="function">
      <type>int</type>
      <name>L_evaluate_dbl_compare</name>
      <anchorfile>l__evaluate_8c.html</anchorfile>
      <anchor>a20c7348e125f4da6c2d2014544aa3d2</anchor>
      <arglist>(L_Oper *oper)</arglist>
    </member>
    <member kind="function">
      <type>int</type>
      <name>L_evaluate_compare</name>
      <anchorfile>l__evaluate_8c.html</anchorfile>
      <anchor>2fd06d600f24e33f3656b33fc2cc4a7e</anchor>
      <arglist>(L_Oper *oper)</arglist>
    </member>
  </compound>
  <compound kind="file">
    <name>l_global_driver.c</name>
    <path>/mnt/hgfs/mkt/Desktop/openimpact/src/Lcode/opti/Lopti/</path>
    <filename>l__global__driver_8c</filename>
    <includes id="l__opti_8h" name="l_opti.h" local="yes" imported="no">l_opti.h</includes>
    <member kind="define">
      <type>#define</type>
      <name>DO_GLOB_DEAD_CODE</name>
      <anchorfile>l__global__driver_8c.html</anchorfile>
      <anchor>008fe4fb0bab10c94b5b8dc65a7da5de</anchor>
      <arglist></arglist>
    </member>
    <member kind="define">
      <type>#define</type>
      <name>DO_GLOB_CONST_PROP</name>
      <anchorfile>l__global__driver_8c.html</anchorfile>
      <anchor>c1e6c54e9bfd2e09b10a012953eb0eb6</anchor>
      <arglist></arglist>
    </member>
    <member kind="define">
      <type>#define</type>
      <name>DO_GLOB_BRANCH_VAL_PROP</name>
      <anchorfile>l__global__driver_8c.html</anchorfile>
      <anchor>ff9b43efee665d3b998f0939b3c17234</anchor>
      <arglist></arglist>
    </member>
    <member kind="define">
      <type>#define</type>
      <name>DO_GLOB_COPY_PROP</name>
      <anchorfile>l__global__driver_8c.html</anchorfile>
      <anchor>9f799e9af89e3f513584acd234d95f77</anchor>
      <arglist></arglist>
    </member>
    <member kind="define">
      <type>#define</type>
      <name>DO_GLOB_MEM_COPY_PROP</name>
      <anchorfile>l__global__driver_8c.html</anchorfile>
      <anchor>b5eb9e8da3dc2f34e635937644d2c8ee</anchor>
      <arglist></arglist>
    </member>
    <member kind="define">
      <type>#define</type>
      <name>DO_GLOB_COMMON_SUB</name>
      <anchorfile>l__global__driver_8c.html</anchorfile>
      <anchor>58aaa00da25ecf88a0338f85a3d1b758</anchor>
      <arglist></arglist>
    </member>
    <member kind="define">
      <type>#define</type>
      <name>DO_GLOB_RED_LOAD</name>
      <anchorfile>l__global__driver_8c.html</anchorfile>
      <anchor>3c3dfc6bd4e0b334d47528837fd3b6f6</anchor>
      <arglist></arglist>
    </member>
    <member kind="define">
      <type>#define</type>
      <name>DO_GLOB_RED_STORE</name>
      <anchorfile>l__global__driver_8c.html</anchorfile>
      <anchor>b1a43551b46dbce1fb3a427bedf7982f</anchor>
      <arglist></arglist>
    </member>
    <member kind="define">
      <type>#define</type>
      <name>DO_GLOB_UNNEC_BOOL</name>
      <anchorfile>l__global__driver_8c.html</anchorfile>
      <anchor>ce37c3422a30bdaebc8c42f8cf679f39</anchor>
      <arglist></arglist>
    </member>
    <member kind="define">
      <type>#define</type>
      <name>DEBUG_COMPLETE_STORE_LOAD_REMOVAL</name>
      <anchorfile>l__global__driver_8c.html</anchorfile>
      <anchor>de87e90fe6afa589dda4ba29657dc6a0</anchor>
      <arglist></arglist>
    </member>
    <member kind="define">
      <type>#define</type>
      <name>MAX_NUM_GLOB_ITERATION</name>
      <anchorfile>l__global__driver_8c.html</anchorfile>
      <anchor>7710e11cc3b7e2b41794e3e16b339039</anchor>
      <arglist></arglist>
    </member>
    <member kind="function">
      <type>int</type>
      <name>L_global_dead_code_optimization</name>
      <anchorfile>l__global__driver_8c.html</anchorfile>
      <anchor>5142ceb52df857310613889e6a78eb3e</anchor>
      <arglist>(L_Func *fn)</arglist>
    </member>
    <member kind="function">
      <type>int</type>
      <name>L_global_code_optimization</name>
      <anchorfile>l__global__driver_8c.html</anchorfile>
      <anchor>c3c841e0ed7bf8fb4401b60bf60afa0c</anchor>
      <arglist>(L_Func *fn)</arglist>
    </member>
  </compound>
  <compound kind="file">
    <name>l_global_opti.c</name>
    <path>/mnt/hgfs/mkt/Desktop/openimpact/src/Lcode/opti/Lopti/</path>
    <filename>l__global__opti_8c</filename>
    <includes id="l__opti_8h" name="l_opti.h" local="yes" imported="no">l_opti.h</includes>
    <member kind="function">
      <type>int</type>
      <name>L_global_dead_code_removal</name>
      <anchorfile>l__global__opti_8c.html</anchorfile>
      <anchor>86e849b8ee731e1d486d6f1d5da3cc85</anchor>
      <arglist>(L_Cb *cb)</arglist>
    </member>
    <member kind="function">
      <type>int</type>
      <name>L_aggressive_dead_code_removal</name>
      <anchorfile>l__global__opti_8c.html</anchorfile>
      <anchor>c08eeb9ea3bc29ef0ae6044487afb014</anchor>
      <arglist>(L_Cb *cb)</arglist>
    </member>
    <member kind="function">
      <type>int</type>
      <name>L_global_constant_propagation</name>
      <anchorfile>l__global__opti_8c.html</anchorfile>
      <anchor>efad317da598a42c9e3fe6616e65dd04</anchor>
      <arglist>(L_Cb *cbA, L_Cb *cbB, int immed_only)</arglist>
    </member>
    <member kind="function">
      <type>int</type>
      <name>L_global_branch_val_propagation</name>
      <anchorfile>l__global__opti_8c.html</anchorfile>
      <anchor>a0e47d477500691ea9e60d0cbb943801</anchor>
      <arglist>(L_Cb *cbA, L_Cb *cbB)</arglist>
    </member>
    <member kind="function">
      <type>int</type>
      <name>L_global_copy_propagation</name>
      <anchorfile>l__global__opti_8c.html</anchorfile>
      <anchor>d3c0ee5ce40d18edc59892c8ea49569a</anchor>
      <arglist>(L_Cb *cbA, L_Cb *cbB)</arglist>
    </member>
    <member kind="function">
      <type>int</type>
      <name>L_global_common_subexpression</name>
      <anchorfile>l__global__opti_8c.html</anchorfile>
      <anchor>f919d36538871d5b474e87009b93ac5b</anchor>
      <arglist>(L_Cb *cbA, L_Cb *cbB, int move_flags)</arglist>
    </member>
    <member kind="function">
      <type>int</type>
      <name>L_global_memflow_redundant_load</name>
      <anchorfile>l__global__opti_8c.html</anchorfile>
      <anchor>c3ce5388334000bcc917f24c19af5f84</anchor>
      <arglist>(L_Cb *cbA, L_Cb *cbB, int *inserted)</arglist>
    </member>
    <member kind="function">
      <type>int</type>
      <name>L_global_memflow_redundant_load_with_store</name>
      <anchorfile>l__global__opti_8c.html</anchorfile>
      <anchor>3618e079af1de6d868fe863bdc30b81d</anchor>
      <arglist>(L_Cb *cbA, L_Cb *cbB, int *inserted)</arglist>
    </member>
    <member kind="function">
      <type>int</type>
      <name>L_global_memflow_redundant_store</name>
      <anchorfile>l__global__opti_8c.html</anchorfile>
      <anchor>56f65e7d07458406c7541f5d80c6566e</anchor>
      <arglist>(L_Cb *cbA, L_Cb *cbB)</arglist>
    </member>
    <member kind="function">
      <type>int</type>
      <name>L_global_remove_unnec_boolean</name>
      <anchorfile>l__global__opti_8c.html</anchorfile>
      <anchor>23440a6a9bd6a54b9343523f263fee3b</anchor>
      <arglist>(L_Cb *cbA)</arglist>
    </member>
    <member kind="function">
      <type>int</type>
      <name>L_global_dead_if_then_else_rem</name>
      <anchorfile>l__global__opti_8c.html</anchorfile>
      <anchor>dede0ae9a03e838809862ac79bf30d77</anchor>
      <arglist>(L_Cb *cbA, L_Cb *cbB)</arglist>
    </member>
    <member kind="function">
      <type>int</type>
      <name>L_global_mem_expression_copy_prop</name>
      <anchorfile>l__global__opti_8c.html</anchorfile>
      <anchor>9161932c8a721dc805aba75bd729d557</anchor>
      <arglist>(L_Func *fn)</arglist>
    </member>
    <member kind="function">
      <type>int</type>
      <name>L_global_dead_store_removal</name>
      <anchorfile>l__global__opti_8c.html</anchorfile>
      <anchor>193929c0f7e13a9f3f6e70f72a0c00d9</anchor>
      <arglist>(L_Func *fn)</arglist>
    </member>
  </compound>
  <compound kind="file">
    <name>l_global_opti.h</name>
    <path>/mnt/hgfs/mkt/Desktop/openimpact/src/Lcode/opti/Lopti/</path>
    <filename>l__global__opti_8h</filename>
    <member kind="function">
      <type>int</type>
      <name>L_global_dead_code_removal</name>
      <anchorfile>l__global__opti_8h.html</anchorfile>
      <anchor>6b09930d6c109337668cba885d5c0250</anchor>
      <arglist>(L_Cb *)</arglist>
    </member>
    <member kind="function">
      <type>int</type>
      <name>L_global_constant_propagation</name>
      <anchorfile>l__global__opti_8h.html</anchorfile>
      <anchor>ccea704c8aeb5d0d96af08ab1a1739ad</anchor>
      <arglist>(L_Cb *, L_Cb *, int)</arglist>
    </member>
    <member kind="function">
      <type>int</type>
      <name>L_global_copy_propagation</name>
      <anchorfile>l__global__opti_8h.html</anchorfile>
      <anchor>57c9c4f6f93f7b229471b284bce0d4ae</anchor>
      <arglist>(L_Cb *, L_Cb *)</arglist>
    </member>
    <member kind="function">
      <type>int</type>
      <name>L_global_branch_val_propagation</name>
      <anchorfile>l__global__opti_8h.html</anchorfile>
      <anchor>c1939208388c4cec199031cb85c4d902</anchor>
      <arglist>(L_Cb *, L_Cb *)</arglist>
    </member>
    <member kind="function">
      <type>int</type>
      <name>L_global_memory_copy_propagation</name>
      <anchorfile>l__global__opti_8h.html</anchorfile>
      <anchor>4a1036a6bacb1795e9978a802c413b95</anchor>
      <arglist>(L_Cb *, L_Cb *)</arglist>
    </member>
    <member kind="function">
      <type>int</type>
      <name>L_global_common_subexpression</name>
      <anchorfile>l__global__opti_8h.html</anchorfile>
      <anchor>83ee03f8e8f5befb6762f1b28ec1a1cd</anchor>
      <arglist>(L_Cb *, L_Cb *, int)</arglist>
    </member>
    <member kind="function">
      <type>int</type>
      <name>L_global_memflow_redundant_load</name>
      <anchorfile>l__global__opti_8h.html</anchorfile>
      <anchor>4e938ebafb9270da2ae48c862e9a0118</anchor>
      <arglist>(L_Cb *, L_Cb *, int *)</arglist>
    </member>
    <member kind="function">
      <type>int</type>
      <name>L_global_memflow_redundant_load_with_store</name>
      <anchorfile>l__global__opti_8h.html</anchorfile>
      <anchor>43863b97ded599d4fe78cb7450ea4f48</anchor>
      <arglist>(L_Cb *, L_Cb *, int *)</arglist>
    </member>
    <member kind="function">
      <type>int</type>
      <name>L_global_memflow_redundant_store</name>
      <anchorfile>l__global__opti_8h.html</anchorfile>
      <anchor>7dfdf949e5c39f410eb831757757711e</anchor>
      <arglist>(L_Cb *, L_Cb *)</arglist>
    </member>
    <member kind="function">
      <type>int</type>
      <name>L_global_dead_if_then_else_rem</name>
      <anchorfile>l__global__opti_8h.html</anchorfile>
      <anchor>d861ec293d639443dea12eaba9cccf8f</anchor>
      <arglist>(L_Cb *, L_Cb *)</arglist>
    </member>
    <member kind="function">
      <type>int</type>
      <name>L_global_remove_unnec_boolean</name>
      <anchorfile>l__global__opti_8h.html</anchorfile>
      <anchor>fef2374a47e685096d2d5a8697a04141</anchor>
      <arglist>(L_Cb *)</arglist>
    </member>
    <member kind="function">
      <type>int</type>
      <name>L_global_mem_expression_copy_prop</name>
      <anchorfile>l__global__opti_8h.html</anchorfile>
      <anchor>b834b454ca94bb5f81ee9950cdb47b9a</anchor>
      <arglist>(L_Func *)</arglist>
    </member>
    <member kind="function">
      <type>int</type>
      <name>L_global_dead_store_removal</name>
      <anchorfile>l__global__opti_8h.html</anchorfile>
      <anchor>dacf4f42dd8b311a22a0ea90c93775c1</anchor>
      <arglist>(L_Func *)</arglist>
    </member>
  </compound>
  <compound kind="file">
    <name>l_jump_driver.c</name>
    <path>/mnt/hgfs/mkt/Desktop/openimpact/src/Lcode/opti/Lopti/</path>
    <filename>l__jump__driver_8c</filename>
    <includes id="l__opti_8h" name="l_opti.h" local="yes" imported="no">l_opti.h</includes>
    <member kind="define">
      <type>#define</type>
      <name>DO_DECIDABLE_BR</name>
      <anchorfile>l__jump__driver_8c.html</anchorfile>
      <anchor>0bc00298561f6d7a45b222be397e04ee</anchor>
      <arglist></arglist>
    </member>
    <member kind="define">
      <type>#define</type>
      <name>DO_DEAD_BLOCK</name>
      <anchorfile>l__jump__driver_8c.html</anchorfile>
      <anchor>bf1109d65e3abb6d560469a7acba3c21</anchor>
      <arglist></arglist>
    </member>
    <member kind="define">
      <type>#define</type>
      <name>DO_BRANCH_TO_NEXT_BLOCK</name>
      <anchorfile>l__jump__driver_8c.html</anchorfile>
      <anchor>a563a167153f88effe81992e3e926125</anchor>
      <arglist></arglist>
    </member>
    <member kind="define">
      <type>#define</type>
      <name>DO_BRANCHES_TO_SAME_TARGET</name>
      <anchorfile>l__jump__driver_8c.html</anchorfile>
      <anchor>b886f69c48f79cb3e9a46e3544a17b7e</anchor>
      <arglist></arglist>
    </member>
    <member kind="define">
      <type>#define</type>
      <name>DO_BRANCH_TO_UNCOND_BRANCH</name>
      <anchorfile>l__jump__driver_8c.html</anchorfile>
      <anchor>db420314b26408c2295e7c1ca5a999a7</anchor>
      <arglist></arglist>
    </member>
    <member kind="define">
      <type>#define</type>
      <name>DO_MERGE</name>
      <anchorfile>l__jump__driver_8c.html</anchorfile>
      <anchor>fe65774867bf511cbf38756dfcf508eb</anchor>
      <arglist></arglist>
    </member>
    <member kind="define">
      <type>#define</type>
      <name>DO_COMBINE_LABELS</name>
      <anchorfile>l__jump__driver_8c.html</anchorfile>
      <anchor>471286788f8ee45c5ed1a272db08b2f2</anchor>
      <arglist></arglist>
    </member>
    <member kind="define">
      <type>#define</type>
      <name>DO_BRANCH_SWAP</name>
      <anchorfile>l__jump__driver_8c.html</anchorfile>
      <anchor>f8a606ca938680cd85478a6f09387000</anchor>
      <arglist></arglist>
    </member>
    <member kind="define">
      <type>#define</type>
      <name>MAX_NUM_ITERATION</name>
      <anchorfile>l__jump__driver_8c.html</anchorfile>
      <anchor>1c880301dfeb72ac3147820876f81613</anchor>
      <arglist></arglist>
    </member>
    <member kind="function">
      <type>int</type>
      <name>L_jump_initial_cleanup</name>
      <anchorfile>l__jump__driver_8c.html</anchorfile>
      <anchor>d81ba41d9d66ce265c7d6f2676b413ca</anchor>
      <arglist>(L_Func *fn)</arglist>
    </member>
    <member kind="function">
      <type>int</type>
      <name>L_jump_optimization</name>
      <anchorfile>l__jump__driver_8c.html</anchorfile>
      <anchor>87b11190aab6a635141b1b48b6528700</anchor>
      <arglist>(L_Func *fn)</arglist>
    </member>
  </compound>
  <compound kind="file">
    <name>l_jump_expand.c</name>
    <path>/mnt/hgfs/mkt/Desktop/openimpact/src/Lcode/opti/Lopti/</path>
    <filename>l__jump__expand_8c</filename>
    <includes id="l__opti_8h" name="l_opti.h" local="yes" imported="no">l_opti.h</includes>
    <class kind="struct">_L_Jrg_Element</class>
    <member kind="define">
      <type>#define</type>
      <name>ERR</name>
      <anchorfile>l__jump__expand_8c.html</anchorfile>
      <anchor>735563036dced0b7d6cc98f97ea4978b</anchor>
      <arglist></arglist>
    </member>
    <member kind="define">
      <type>#define</type>
      <name>MIN_JRG_OPTI_WEIGHT</name>
      <anchorfile>l__jump__expand_8c.html</anchorfile>
      <anchor>6c6a7025e42a22ff9fe272f7c1fc78d4</anchor>
      <arglist></arglist>
    </member>
    <member kind="define">
      <type>#define</type>
      <name>LIKELY_JUMP_RG_TAKEN_RATIO</name>
      <anchorfile>l__jump__expand_8c.html</anchorfile>
      <anchor>4c73ddebc0ae36b2ffbff98416b3b472</anchor>
      <arglist></arglist>
    </member>
    <member kind="define">
      <type>#define</type>
      <name>MIN_JUMP_RG_TAKEN_RATIO</name>
      <anchorfile>l__jump__expand_8c.html</anchorfile>
      <anchor>c73880eb0593bf82ef7d06f40ec00179</anchor>
      <arglist></arglist>
    </member>
    <member kind="define">
      <type>#define</type>
      <name>MAX_NUM_PRETEST_BRANCH</name>
      <anchorfile>l__jump__expand_8c.html</anchorfile>
      <anchor>aa63ba17c19dd56decf9df95098d1ec1</anchor>
      <arglist></arglist>
    </member>
    <member kind="define">
      <type>#define</type>
      <name>MAX_SIZE</name>
      <anchorfile>l__jump__expand_8c.html</anchorfile>
      <anchor>0592dba56693fad79136250c11e5a7fe</anchor>
      <arglist></arglist>
    </member>
    <member kind="typedef">
      <type>_L_Jrg_Element</type>
      <name>L_Jrg_Element</name>
      <anchorfile>l__jump__expand_8c.html</anchorfile>
      <anchor>547795b89f8197a4a633bdacb45a957a</anchor>
      <arglist></arglist>
    </member>
    <member kind="function" static="yes">
      <type>static void</type>
      <name>bubble_sort</name>
      <anchorfile>l__jump__expand_8c.html</anchorfile>
      <anchor>bb266f455e5cd33226adecc9ed255ec5</anchor>
      <arglist>(L_Jrg_Element *x, int n)</arglist>
    </member>
    <member kind="function" static="yes">
      <type>static L_Cb *</type>
      <name>L_find_jump_rg_most_likely_tgt</name>
      <anchorfile>l__jump__expand_8c.html</anchorfile>
      <anchor>629d556285b76e8e7e00f13c337a408f</anchor>
      <arglist>(L_Cb *cb)</arglist>
    </member>
    <member kind="function">
      <type>void</type>
      <name>L_jump_rg_expansion</name>
      <anchorfile>l__jump__expand_8c.html</anchorfile>
      <anchor>9b71b7aea52232f282e9f2cd6193381f</anchor>
      <arglist>(L_Func *fn)</arglist>
    </member>
    <member kind="variable" static="yes">
      <type>static int</type>
      <name>L_num_jrg_element</name>
      <anchorfile>l__jump__expand_8c.html</anchorfile>
      <anchor>0efe7d70cc64a20499dcb91f07f19a78</anchor>
      <arglist></arglist>
    </member>
    <member kind="variable" static="yes">
      <type>static int</type>
      <name>L_num_pretest_branch</name>
      <anchorfile>l__jump__expand_8c.html</anchorfile>
      <anchor>6a71c4e00ff198a62c86f922c8df7d98</anchor>
      <arglist></arglist>
    </member>
    <member kind="variable" static="yes">
      <type>static L_Jrg_Element *</type>
      <name>L_jrg</name>
      <anchorfile>l__jump__expand_8c.html</anchorfile>
      <anchor>c86631f0e4f3326f6859abaaf4675fc8</anchor>
      <arglist></arglist>
    </member>
  </compound>
  <compound kind="file">
    <name>l_jump_opti.c</name>
    <path>/mnt/hgfs/mkt/Desktop/openimpact/src/Lcode/opti/Lopti/</path>
    <filename>l__jump__opti_8c</filename>
    <includes id="l__opti_8h" name="l_opti.h" local="yes" imported="no">l_opti.h</includes>
    <member kind="define">
      <type>#define</type>
      <name>ERR</name>
      <anchorfile>l__jump__opti_8c.html</anchorfile>
      <anchor>735563036dced0b7d6cc98f97ea4978b</anchor>
      <arglist></arglist>
    </member>
    <member kind="define">
      <type>#define</type>
      <name>L_MAX_CB_SIZE_FOR_JUMP_OPTI</name>
      <anchorfile>l__jump__opti_8c.html</anchorfile>
      <anchor>bf4046d9ea0183f350d9c3d9b3fb230d</anchor>
      <arglist></arglist>
    </member>
    <member kind="define">
      <type>#define</type>
      <name>L_MIN_WEIGHT_FOR_BRANCH_EXP</name>
      <anchorfile>l__jump__opti_8c.html</anchorfile>
      <anchor>d61a41176748256f181f4a618497b897</anchor>
      <arglist></arglist>
    </member>
    <member kind="define">
      <type>#define</type>
      <name>L_MIN_WEIGHT_FOR_LOOP_BRANCH_EXP</name>
      <anchorfile>l__jump__opti_8c.html</anchorfile>
      <anchor>70a3ff1e040577c9dbe3c091fb372a24</anchor>
      <arglist></arglist>
    </member>
    <member kind="define">
      <type>#define</type>
      <name>L_MIN_COND_BR_TAKEN_RATIO</name>
      <anchorfile>l__jump__opti_8c.html</anchorfile>
      <anchor>4bb1fd22d356ce588e8df644df16a437</anchor>
      <arglist></arglist>
    </member>
    <member kind="define">
      <type>#define</type>
      <name>L_MIN_LOOP_COND_BR_TAKEN_RATIO</name>
      <anchorfile>l__jump__opti_8c.html</anchorfile>
      <anchor>3a21c3ab8f71dbc29980daee95a418d1</anchor>
      <arglist></arglist>
    </member>
    <member kind="define">
      <type>#define</type>
      <name>L_MIN_FLOW_RATIO</name>
      <anchorfile>l__jump__opti_8c.html</anchorfile>
      <anchor>4e5e8d64620605184ea9ed277beb3e28</anchor>
      <arglist></arglist>
    </member>
    <member kind="define">
      <type>#define</type>
      <name>L_MIN_LOOP_FLOW_RATIO</name>
      <anchorfile>l__jump__opti_8c.html</anchorfile>
      <anchor>3736fd8fa5b162ccbe6680fc5d56cd14</anchor>
      <arglist></arglist>
    </member>
    <member kind="define">
      <type>#define</type>
      <name>L_MIN_TARGET_CB_RATIO</name>
      <anchorfile>l__jump__opti_8c.html</anchorfile>
      <anchor>0d26f3fdfee7272e9f773431ea8121f9</anchor>
      <arglist></arglist>
    </member>
    <member kind="define">
      <type>#define</type>
      <name>NO_EXPANSION</name>
      <anchorfile>l__jump__opti_8c.html</anchorfile>
      <anchor>bb43a8b2c6ea5d320b8df134d062e233</anchor>
      <arglist></arglist>
    </member>
    <member kind="define">
      <type>#define</type>
      <name>EXPAND_TAKEN</name>
      <anchorfile>l__jump__opti_8c.html</anchorfile>
      <anchor>dabd832ec3ee094c241afe8b37e97f1b</anchor>
      <arglist></arglist>
    </member>
    <member kind="define">
      <type>#define</type>
      <name>EXPAND_FALLTHRU</name>
      <anchorfile>l__jump__opti_8c.html</anchorfile>
      <anchor>9f12d8d93ca25cea7fdf04001d72f7b7</anchor>
      <arglist></arglist>
    </member>
    <member kind="define">
      <type>#define</type>
      <name>MAX_DEFS</name>
      <anchorfile>l__jump__opti_8c.html</anchorfile>
      <anchor>f6a0436cac237e57c318bbc1af898038</anchor>
      <arglist></arglist>
    </member>
    <member kind="function">
      <type>int</type>
      <name>L_jump_elim_branch_to_next_block</name>
      <anchorfile>l__jump__opti_8c.html</anchorfile>
      <anchor>bfe7e119c00d82dac2c19d9ffce4b648</anchor>
      <arglist>(L_Func *fn, int flags)</arglist>
    </member>
    <member kind="function">
      <type>int</type>
      <name>L_jump_combine_branches_to_same_target</name>
      <anchorfile>l__jump__opti_8c.html</anchorfile>
      <anchor>c87a6b1a3988baa3a00d9e6574fbc81c</anchor>
      <arglist>(L_Func *fn, int flags)</arglist>
    </member>
    <member kind="function">
      <type>int</type>
      <name>L_jump_combine_branch_to_uncond_branch</name>
      <anchorfile>l__jump__opti_8c.html</anchorfile>
      <anchor>84f246633d5116b6351ab4784c228ffc</anchor>
      <arglist>(L_Func *fn, int flags)</arglist>
    </member>
    <member kind="function">
      <type>int</type>
      <name>L_jump_merge_always_successive_blocks</name>
      <anchorfile>l__jump__opti_8c.html</anchorfile>
      <anchor>5ab6683acf0262a56817e0bd82d5e1ca</anchor>
      <arglist>(L_Func *fn, int flags)</arglist>
    </member>
    <member kind="function">
      <type>int</type>
      <name>L_jump_combine_labels</name>
      <anchorfile>l__jump__opti_8c.html</anchorfile>
      <anchor>052b0aca2df7b58bbcdbddb7641c4ab4</anchor>
      <arglist>(L_Func *fn, int flags)</arglist>
    </member>
    <member kind="function" static="yes">
      <type>static int</type>
      <name>L_likely_cb_ending_branch</name>
      <anchorfile>l__jump__opti_8c.html</anchorfile>
      <anchor>ff489eed542749c3aa70f89950b29b90</anchor>
      <arglist>(L_Cb *cb)</arglist>
    </member>
    <member kind="function" static="yes">
      <type>static int</type>
      <name>L_likely_flow_for_expansion</name>
      <anchorfile>l__jump__opti_8c.html</anchorfile>
      <anchor>fbb41c0824a4fb8a45ef081506987931</anchor>
      <arglist>(L_Cb *cb, L_Flow *fl)</arglist>
    </member>
    <member kind="function" static="yes">
      <type>static void</type>
      <name>L_find_superblock_loop_flow</name>
      <anchorfile>l__jump__opti_8c.html</anchorfile>
      <anchor>ac22718c171694a07b3a014fa4e53136</anchor>
      <arglist>(L_Cb *cb, L_Cb *target_cb, L_Oper **p_loop_br, L_Flow **p_loop_flow, int flags)</arglist>
    </member>
    <member kind="function" static="yes">
      <type>static int</type>
      <name>L_br_expand</name>
      <anchorfile>l__jump__opti_8c.html</anchorfile>
      <anchor>689f5a2b8ae97b947981fbce3270e901</anchor>
      <arglist>(L_Flow *expand_fl, int flags)</arglist>
    </member>
    <member kind="function">
      <type>int</type>
      <name>L_jump_branch_target_expansion</name>
      <anchorfile>l__jump__opti_8c.html</anchorfile>
      <anchor>e742334867e69335d1a29803779e5730</anchor>
      <arglist>(L_Func *fn, int flags)</arglist>
    </member>
    <member kind="function">
      <type>int</type>
      <name>L_jump_branch_swap</name>
      <anchorfile>l__jump__opti_8c.html</anchorfile>
      <anchor>8ebb4327055f409b3b45bce124b552bb</anchor>
      <arglist>(L_Func *fn, int flags)</arglist>
    </member>
    <member kind="function">
      <type>void</type>
      <name>L_remove_opers_after_uncond_branch</name>
      <anchorfile>l__jump__opti_8c.html</anchorfile>
      <anchor>703928191d143c795a78be489b8bba54</anchor>
      <arglist>(L_Cb *cb, L_Oper *jump_oper)</arglist>
    </member>
    <member kind="function">
      <type>int</type>
      <name>L_remove_decidable_cond_branches</name>
      <anchorfile>l__jump__opti_8c.html</anchorfile>
      <anchor>a531c011fec841d7f2d9115b9a4f8a74</anchor>
      <arglist>(L_Func *fn, int flags)</arglist>
    </member>
    <member kind="function" static="yes">
      <type>static void</type>
      <name>L_split_node</name>
      <anchorfile>l__jump__opti_8c.html</anchorfile>
      <anchor>1ed77ba762d8783388b9d489047ff232</anchor>
      <arglist>(L_Func *fn, L_Cb *orig_cb)</arglist>
    </member>
    <member kind="function">
      <type>void</type>
      <name>L_split_multidef_branches</name>
      <anchorfile>l__jump__opti_8c.html</anchorfile>
      <anchor>7f6ff1839630fb1ced682caa7c968623</anchor>
      <arglist>(L_Func *fn)</arglist>
    </member>
    <member kind="variable" static="yes">
      <type>static double</type>
      <name>expand_ratio</name>
      <anchorfile>l__jump__opti_8c.html</anchorfile>
      <anchor>ab10347c093c82de8df67715b94b7d09</anchor>
      <arglist></arglist>
    </member>
    <member kind="variable" static="yes">
      <type>static double</type>
      <name>expand_flow_ratio</name>
      <anchorfile>l__jump__opti_8c.html</anchorfile>
      <anchor>e7304cb797aab2ae396452486c1e8649</anchor>
      <arglist></arglist>
    </member>
    <member kind="variable" static="yes">
      <type>static L_Oper *</type>
      <name>expand_br</name>
      <anchorfile>l__jump__opti_8c.html</anchorfile>
      <anchor>a757e17ed85bb3e407d4ffa39d8a8723</anchor>
      <arglist></arglist>
    </member>
    <member kind="variable" static="yes">
      <type>static L_Flow *</type>
      <name>expand_flow</name>
      <anchorfile>l__jump__opti_8c.html</anchorfile>
      <anchor>578e05aa582d8f14104e5370a31a5e56</anchor>
      <arglist></arglist>
    </member>
  </compound>
  <compound kind="file">
    <name>l_jump_opti.h</name>
    <path>/mnt/hgfs/mkt/Desktop/openimpact/src/Lcode/opti/Lopti/</path>
    <filename>l__jump__opti_8h</filename>
    <member kind="define">
      <type>#define</type>
      <name>L_JUMP_ALLOW_SUPERBLOCKS</name>
      <anchorfile>l__jump__opti_8h.html</anchorfile>
      <anchor>ba7679b6688c404439c4296d3d01268f</anchor>
      <arglist></arglist>
    </member>
    <member kind="define">
      <type>#define</type>
      <name>L_JUMP_ALLOW_BACKEDGE_EXP</name>
      <anchorfile>l__jump__opti_8h.html</anchorfile>
      <anchor>e5df5cfc463914e6534e6ad482fae4d2</anchor>
      <arglist></arglist>
    </member>
    <member kind="define">
      <type>#define</type>
      <name>L_JUMP_ALLOW_LOOP_BODY_EXP</name>
      <anchorfile>l__jump__opti_8h.html</anchorfile>
      <anchor>b7c5514ca7c6f99a3637648689f60167</anchor>
      <arglist></arglist>
    </member>
    <member kind="function">
      <type>int</type>
      <name>L_jump_dead_block_removal</name>
      <anchorfile>l__jump__opti_8h.html</anchorfile>
      <anchor>c14aa76e3d5465c3d544f0d117396d83</anchor>
      <arglist>(L_Func *, int)</arglist>
    </member>
    <member kind="function">
      <type>int</type>
      <name>L_jump_elim_branch_to_next_block</name>
      <anchorfile>l__jump__opti_8h.html</anchorfile>
      <anchor>6ac8648f42dcb933af17b3c546b9b2c5</anchor>
      <arglist>(L_Func *, int)</arglist>
    </member>
    <member kind="function">
      <type>int</type>
      <name>L_jump_combine_branches_to_same_target</name>
      <anchorfile>l__jump__opti_8h.html</anchorfile>
      <anchor>bc2b077f0e922b2e488a7fcf9f2b0580</anchor>
      <arglist>(L_Func *, int)</arglist>
    </member>
    <member kind="function">
      <type>int</type>
      <name>L_jump_combine_branch_to_uncond_branch</name>
      <anchorfile>l__jump__opti_8h.html</anchorfile>
      <anchor>1ff9bf5af5a3ef03ff93a5eb8f08339d</anchor>
      <arglist>(L_Func *, int)</arglist>
    </member>
    <member kind="function">
      <type>int</type>
      <name>L_jump_merge_always_successive_blocks</name>
      <anchorfile>l__jump__opti_8h.html</anchorfile>
      <anchor>4614875beafdfe17b0e340aabfbd41ab</anchor>
      <arglist>(L_Func *, int)</arglist>
    </member>
    <member kind="function">
      <type>int</type>
      <name>L_jump_combine_labels</name>
      <anchorfile>l__jump__opti_8h.html</anchorfile>
      <anchor>8d60e5cda97e44959713284d91143496</anchor>
      <arglist>(L_Func *, int)</arglist>
    </member>
    <member kind="function">
      <type>int</type>
      <name>L_jump_branch_target_expansion</name>
      <anchorfile>l__jump__opti_8h.html</anchorfile>
      <anchor>a4804e497f911334113d0c6f936f92ae</anchor>
      <arglist>(L_Func *, int)</arglist>
    </member>
    <member kind="function">
      <type>int</type>
      <name>L_jump_branch_swap</name>
      <anchorfile>l__jump__opti_8h.html</anchorfile>
      <anchor>f732e43b896bdd685858e69de6ef4299</anchor>
      <arglist>(L_Func *, int)</arglist>
    </member>
    <member kind="function">
      <type>void</type>
      <name>L_jump_branch_prediction</name>
      <anchorfile>l__jump__opti_8h.html</anchorfile>
      <anchor>85b30f2203da643429c13892b2a9d0cd</anchor>
      <arglist>(L_Func *)</arglist>
    </member>
    <member kind="function">
      <type>int</type>
      <name>L_remove_decidable_cond_branches</name>
      <anchorfile>l__jump__opti_8h.html</anchorfile>
      <anchor>749e26609d671d4ab47d7567dbcebf03</anchor>
      <arglist>(L_Func *, int)</arglist>
    </member>
    <member kind="function">
      <type>void</type>
      <name>L_split_multidef_branches</name>
      <anchorfile>l__jump__opti_8h.html</anchorfile>
      <anchor>7f6ff1839630fb1ced682caa7c968623</anchor>
      <arglist>(L_Func *fn)</arglist>
    </member>
    <member kind="function">
      <type>void</type>
      <name>L_jump_rg_expansion</name>
      <anchorfile>l__jump__opti_8h.html</anchorfile>
      <anchor>9b71b7aea52232f282e9f2cd6193381f</anchor>
      <arglist>(L_Func *fn)</arglist>
    </member>
  </compound>
  <compound kind="file">
    <name>l_local_driver.c</name>
    <path>/mnt/hgfs/mkt/Desktop/openimpact/src/Lcode/opti/Lopti/</path>
    <filename>l__local__driver_8c</filename>
    <includes id="l__opti_8h" name="l_opti.h" local="yes" imported="no">l_opti.h</includes>
    <member kind="define">
      <type>#define</type>
      <name>DO_CONST_PROP</name>
      <anchorfile>l__local__driver_8c.html</anchorfile>
      <anchor>7edbb2b6b74705c1e517acb211be5529</anchor>
      <arglist></arglist>
    </member>
    <member kind="define">
      <type>#define</type>
      <name>DO_COPY_PROP</name>
      <anchorfile>l__local__driver_8c.html</anchorfile>
      <anchor>b711798e7811d3eb3426358c54b680d5</anchor>
      <arglist></arglist>
    </member>
    <member kind="define">
      <type>#define</type>
      <name>DO_REV_COPY_PROP</name>
      <anchorfile>l__local__driver_8c.html</anchorfile>
      <anchor>7b73eaf04769d717cb57a372eff7570b</anchor>
      <arglist></arglist>
    </member>
    <member kind="define">
      <type>#define</type>
      <name>DO_MEM_COPY_PROP</name>
      <anchorfile>l__local__driver_8c.html</anchorfile>
      <anchor>a83bc7bd9beb1882ea38504a4aacf960</anchor>
      <arglist></arglist>
    </member>
    <member kind="define">
      <type>#define</type>
      <name>DO_COMMON_SUB</name>
      <anchorfile>l__local__driver_8c.html</anchorfile>
      <anchor>98b330635a789fb1903e802d111f2ebc</anchor>
      <arglist></arglist>
    </member>
    <member kind="define">
      <type>#define</type>
      <name>DO_RED_LOAD</name>
      <anchorfile>l__local__driver_8c.html</anchorfile>
      <anchor>2f6d246cc7d49a03227cec8be2e5f1dc</anchor>
      <arglist></arglist>
    </member>
    <member kind="define">
      <type>#define</type>
      <name>DO_RED_STORE</name>
      <anchorfile>l__local__driver_8c.html</anchorfile>
      <anchor>707fb57038f8edf3de3a788e63e1f93d</anchor>
      <arglist></arglist>
    </member>
    <member kind="define">
      <type>#define</type>
      <name>DO_CONST_FOLD</name>
      <anchorfile>l__local__driver_8c.html</anchorfile>
      <anchor>c8b6ad61f45b6e2d312d5db5fadd33f8</anchor>
      <arglist></arglist>
    </member>
    <member kind="define">
      <type>#define</type>
      <name>DO_CONST_COMBINE</name>
      <anchorfile>l__local__driver_8c.html</anchorfile>
      <anchor>78a64279934f83a5ac95ae86ed88d11a</anchor>
      <arglist></arglist>
    </member>
    <member kind="define">
      <type>#define</type>
      <name>DO_STR_RED</name>
      <anchorfile>l__local__driver_8c.html</anchorfile>
      <anchor>dd29362cd3122b0083ee136566e43e8a</anchor>
      <arglist></arglist>
    </member>
    <member kind="define">
      <type>#define</type>
      <name>DO_OP_FOLD</name>
      <anchorfile>l__local__driver_8c.html</anchorfile>
      <anchor>dd8d046291ac10c8680bccddd1e5a700</anchor>
      <arglist></arglist>
    </member>
    <member kind="define">
      <type>#define</type>
      <name>DO_BR_FOLD</name>
      <anchorfile>l__local__driver_8c.html</anchorfile>
      <anchor>3f762860e4fb6d8207e01138ed375ab5</anchor>
      <arglist></arglist>
    </member>
    <member kind="define">
      <type>#define</type>
      <name>DO_OP_CANCEL</name>
      <anchorfile>l__local__driver_8c.html</anchorfile>
      <anchor>0104300248fec07a5936c2b288cae525</anchor>
      <arglist></arglist>
    </member>
    <member kind="define">
      <type>#define</type>
      <name>DO_DEAD_CODE</name>
      <anchorfile>l__local__driver_8c.html</anchorfile>
      <anchor>149f49422a0e4d1e649ee968fdfd9f69</anchor>
      <arglist></arglist>
    </member>
    <member kind="define">
      <type>#define</type>
      <name>DO_CODE_MOTION</name>
      <anchorfile>l__local__driver_8c.html</anchorfile>
      <anchor>f04ea54a515ba799709fe4e9245f3324</anchor>
      <arglist></arglist>
    </member>
    <member kind="define">
      <type>#define</type>
      <name>DO_REMOVE_SIGN_EXT</name>
      <anchorfile>l__local__driver_8c.html</anchorfile>
      <anchor>1aa49c4aaf46ca1b82642c4af3df60f1</anchor>
      <arglist></arglist>
    </member>
    <member kind="define">
      <type>#define</type>
      <name>DO_REG_RENAMING</name>
      <anchorfile>l__local__driver_8c.html</anchorfile>
      <anchor>39173e5d8fb15d1825c3a283e94eaa65</anchor>
      <arglist></arglist>
    </member>
    <member kind="define">
      <type>#define</type>
      <name>DO_COMMON_SUB_2</name>
      <anchorfile>l__local__driver_8c.html</anchorfile>
      <anchor>15589545a24a52a53ce57b19863b2b75</anchor>
      <arglist></arglist>
    </member>
    <member kind="define">
      <type>#define</type>
      <name>DO_COPY_PROP_2</name>
      <anchorfile>l__local__driver_8c.html</anchorfile>
      <anchor>56b1f92a03273223c5e196e3df7b9bb4</anchor>
      <arglist></arglist>
    </member>
    <member kind="define">
      <type>#define</type>
      <name>DO_OPER_BRKDWN</name>
      <anchorfile>l__local__driver_8c.html</anchorfile>
      <anchor>9a1ea9dd3cb59556bf9812751d1adf3f</anchor>
      <arglist></arglist>
    </member>
    <member kind="define">
      <type>#define</type>
      <name>DO_OPER_RECOMBINE</name>
      <anchorfile>l__local__driver_8c.html</anchorfile>
      <anchor>8ff310dd716294a5c4dbf4c858d9eaf1</anchor>
      <arglist></arglist>
    </member>
    <member kind="define">
      <type>#define</type>
      <name>DO_RED_LOGIC</name>
      <anchorfile>l__local__driver_8c.html</anchorfile>
      <anchor>5db9aaab558538035295ec04f4ec48bf</anchor>
      <arglist></arglist>
    </member>
    <member kind="define">
      <type>#define</type>
      <name>DO_BRANCH_VAL_PROP</name>
      <anchorfile>l__local__driver_8c.html</anchorfile>
      <anchor>276797d73e406d15313af43051ac7eae</anchor>
      <arglist></arglist>
    </member>
    <member kind="define">
      <type>#define</type>
      <name>MAX_NUM_ITERATION</name>
      <anchorfile>l__local__driver_8c.html</anchorfile>
      <anchor>1c880301dfeb72ac3147820876f81613</anchor>
      <arglist></arglist>
    </member>
    <member kind="function">
      <type>void</type>
      <name>mydebug</name>
      <anchorfile>l__local__driver_8c.html</anchorfile>
      <anchor>e8786c40ebb53f97c0a59fffc61cac97</anchor>
      <arglist>(char c, L_Func *fn, L_Cb *cb)</arglist>
    </member>
    <member kind="function">
      <type>int</type>
      <name>L_local_code_optimization</name>
      <anchorfile>l__local__driver_8c.html</anchorfile>
      <anchor>488b9509d76cb486ce6b5c05d87e4b03</anchor>
      <arglist>(L_Func *fn, int mov_flag)</arglist>
    </member>
    <member kind="function">
      <type>void</type>
      <name>L_oper_breakdown</name>
      <anchorfile>l__local__driver_8c.html</anchorfile>
      <anchor>123f5667bceb416195163889c45e4f5e</anchor>
      <arglist>(L_Func *fn)</arglist>
    </member>
    <member kind="function">
      <type>void</type>
      <name>L_oper_recombine</name>
      <anchorfile>l__local__driver_8c.html</anchorfile>
      <anchor>ef51555572902a70e2e4d9154e8a1ea0</anchor>
      <arglist>(L_Func *fn)</arglist>
    </member>
  </compound>
  <compound kind="file">
    <name>l_local_opti.c</name>
    <path>/mnt/hgfs/mkt/Desktop/openimpact/src/Lcode/opti/Lopti/</path>
    <filename>l__local__opti_8c</filename>
    <includes id="l__opti_8h" name="l_opti.h" local="yes" imported="no">l_opti.h</includes>
    <member kind="define">
      <type>#define</type>
      <name>ERR</name>
      <anchorfile>l__local__opti_8c.html</anchorfile>
      <anchor>735563036dced0b7d6cc98f97ea4978b</anchor>
      <arglist></arglist>
    </member>
    <member kind="define">
      <type>#define</type>
      <name>Lopti_copy_prop_incl_sz_ext</name>
      <anchorfile>l__local__opti_8c.html</anchorfile>
      <anchor>a946eb9360c45bcbfe39300b67c3d5b6</anchor>
      <arglist></arglist>
    </member>
    <member kind="define">
      <type>#define</type>
      <name>L_CCF_MAX_PARM</name>
      <anchorfile>l__local__opti_8c.html</anchorfile>
      <anchor>29610d2c5cdac77d427f3377ee539fb8</anchor>
      <arglist></arglist>
    </member>
    <member kind="define">
      <type>#define</type>
      <name>MAX_FN_NUMBER</name>
      <anchorfile>l__local__opti_8c.html</anchorfile>
      <anchor>215c792ca382c89704d2eca26399d1c6</anchor>
      <arglist></arglist>
    </member>
    <member kind="define">
      <type>#define</type>
      <name>FN_FMOD</name>
      <anchorfile>l__local__opti_8c.html</anchorfile>
      <anchor>e18884b9bd803bdd8cc4829e9af3760a</anchor>
      <arglist></arglist>
    </member>
    <member kind="define">
      <type>#define</type>
      <name>FN_POW</name>
      <anchorfile>l__local__opti_8c.html</anchorfile>
      <anchor>bcc7127ec4d4de9633bdde69ec99e381</anchor>
      <arglist></arglist>
    </member>
    <member kind="define">
      <type>#define</type>
      <name>FN_LOG</name>
      <anchorfile>l__local__opti_8c.html</anchorfile>
      <anchor>31638451cea5e986bc5d858ca25d4012</anchor>
      <arglist></arglist>
    </member>
    <member kind="define">
      <type>#define</type>
      <name>FN_FLOOR</name>
      <anchorfile>l__local__opti_8c.html</anchorfile>
      <anchor>4ac631a11a006e2f3ff0dc15f5267dac</anchor>
      <arglist></arglist>
    </member>
    <member kind="define">
      <type>#define</type>
      <name>FN_CEIL</name>
      <anchorfile>l__local__opti_8c.html</anchorfile>
      <anchor>165c2d7278c64eb3efda9c766ac74eb4</anchor>
      <arglist></arglist>
    </member>
    <member kind="define">
      <type>#define</type>
      <name>FN_FABS</name>
      <anchorfile>l__local__opti_8c.html</anchorfile>
      <anchor>a7fc66738b870e6972b6933dfd9d7b32</anchor>
      <arglist></arglist>
    </member>
    <member kind="define">
      <type>#define</type>
      <name>FN_SQRT</name>
      <anchorfile>l__local__opti_8c.html</anchorfile>
      <anchor>d1f787420360de6e2ec33efa4e12458c</anchor>
      <arglist></arglist>
    </member>
    <member kind="define">
      <type>#define</type>
      <name>FN_EXP</name>
      <anchorfile>l__local__opti_8c.html</anchorfile>
      <anchor>6cd1f493d204fd4786595c2838b3d92a</anchor>
      <arglist></arglist>
    </member>
    <member kind="define">
      <type>#define</type>
      <name>FN_ACOS</name>
      <anchorfile>l__local__opti_8c.html</anchorfile>
      <anchor>3a9668cd9a1d086f704b6baa24773627</anchor>
      <arglist></arglist>
    </member>
    <member kind="define">
      <type>#define</type>
      <name>FN_ASIN</name>
      <anchorfile>l__local__opti_8c.html</anchorfile>
      <anchor>52b4d03d64572295b583e003cdc9becd</anchor>
      <arglist></arglist>
    </member>
    <member kind="define">
      <type>#define</type>
      <name>FN_ATAN</name>
      <anchorfile>l__local__opti_8c.html</anchorfile>
      <anchor>cff8bed41de8e10147cbda3dea2a99fe</anchor>
      <arglist></arglist>
    </member>
    <member kind="define">
      <type>#define</type>
      <name>FN_ATAN2</name>
      <anchorfile>l__local__opti_8c.html</anchorfile>
      <anchor>b360d3169cbdfd9d277c0b246a5443c2</anchor>
      <arglist></arglist>
    </member>
    <member kind="define">
      <type>#define</type>
      <name>FN_COS</name>
      <anchorfile>l__local__opti_8c.html</anchorfile>
      <anchor>1b11f3ca0bbc7782a94b09da1d73b396</anchor>
      <arglist></arglist>
    </member>
    <member kind="define">
      <type>#define</type>
      <name>FN_SIN</name>
      <anchorfile>l__local__opti_8c.html</anchorfile>
      <anchor>1bce74ccaa3f9744852d15e4ed374c5a</anchor>
      <arglist></arglist>
    </member>
    <member kind="define">
      <type>#define</type>
      <name>FN_TAN</name>
      <anchorfile>l__local__opti_8c.html</anchorfile>
      <anchor>2b5e754653a574edfe8743ac536b8bd5</anchor>
      <arglist></arglist>
    </member>
    <member kind="define">
      <type>#define</type>
      <name>FN_COSH</name>
      <anchorfile>l__local__opti_8c.html</anchorfile>
      <anchor>30ff9519313df08d0e4c91ee79342854</anchor>
      <arglist></arglist>
    </member>
    <member kind="define">
      <type>#define</type>
      <name>FN_SINH</name>
      <anchorfile>l__local__opti_8c.html</anchorfile>
      <anchor>880a2adf810e6f4e1fbc6b5ffa8a01c5</anchor>
      <arglist></arglist>
    </member>
    <member kind="define">
      <type>#define</type>
      <name>FN_TANH</name>
      <anchorfile>l__local__opti_8c.html</anchorfile>
      <anchor>997cf8133ac3b09e125c1fc7c2fbdadb</anchor>
      <arglist></arglist>
    </member>
    <member kind="define">
      <type>#define</type>
      <name>FN_FMIN</name>
      <anchorfile>l__local__opti_8c.html</anchorfile>
      <anchor>ec939b6331cbba20f2aa8dae4020c510</anchor>
      <arglist></arglist>
    </member>
    <member kind="define">
      <type>#define</type>
      <name>FN_FMAX</name>
      <anchorfile>l__local__opti_8c.html</anchorfile>
      <anchor>4835aab565be3f5d0bbf28f4df45ff1b</anchor>
      <arglist></arglist>
    </member>
    <member kind="function">
      <type>int</type>
      <name>L_local_constant_propagation</name>
      <anchorfile>l__local__opti_8c.html</anchorfile>
      <anchor>25a5e8076b4e43e178db59d9b8b21948</anchor>
      <arglist>(L_Cb *cb, int immed_only)</arglist>
    </member>
    <member kind="function">
      <type>int</type>
      <name>L_local_copy_propagation</name>
      <anchorfile>l__local__opti_8c.html</anchorfile>
      <anchor>c9a1859d57894a36cb699a75730f3e25</anchor>
      <arglist>(L_Cb *cb)</arglist>
    </member>
    <member kind="function">
      <type>int</type>
      <name>L_local_rev_copy_propagation</name>
      <anchorfile>l__local__opti_8c.html</anchorfile>
      <anchor>cb348c75b671e4455474dfaa9c42f36d</anchor>
      <arglist>(L_Cb *cb)</arglist>
    </member>
    <member kind="function">
      <type>int</type>
      <name>L_local_memory_same_location</name>
      <anchorfile>l__local__opti_8c.html</anchorfile>
      <anchor>584de91e5902fd7fa36236f3adbe703b</anchor>
      <arglist>(L_Oper *opA, L_Oper *opB)</arglist>
    </member>
    <member kind="function">
      <type>int</type>
      <name>L_classify_overlap</name>
      <anchorfile>l__local__opti_8c.html</anchorfile>
      <anchor>c8449fe34a67d3f99af6af8a5920e3f7</anchor>
      <arglist>(int offset1, int size1, int offset2, int size2, int *p_indep, int *p_dep, int *p_pdep)</arglist>
    </member>
    <member kind="function">
      <type>int</type>
      <name>L_local_memory_subset_access</name>
      <anchorfile>l__local__opti_8c.html</anchorfile>
      <anchor>1df5814c47c70a0ab4645fc0590fcc49</anchor>
      <arglist>(L_Oper *opA, L_Oper *opB)</arglist>
    </member>
    <member kind="function">
      <type>int</type>
      <name>L_local_memory_copy_propagation</name>
      <anchorfile>l__local__opti_8c.html</anchorfile>
      <anchor>c0b018379408a6e11ab8ca88958ee351</anchor>
      <arglist>(L_Cb *cb)</arglist>
    </member>
    <member kind="function">
      <type>int</type>
      <name>L_local_common_subexpression</name>
      <anchorfile>l__local__opti_8c.html</anchorfile>
      <anchor>28f430e203ec1ce55bc01580461c80e7</anchor>
      <arglist>(L_Cb *cb, int move_flags)</arglist>
    </member>
    <member kind="function">
      <type>int</type>
      <name>L_local_redundant_load</name>
      <anchorfile>l__local__opti_8c.html</anchorfile>
      <anchor>dfcca13e056f4df2b1277a6652535930</anchor>
      <arglist>(L_Cb *cb)</arglist>
    </member>
    <member kind="function">
      <type>int</type>
      <name>L_local_redundant_store</name>
      <anchorfile>l__local__opti_8c.html</anchorfile>
      <anchor>45a9a7df48d4f99b4bc6c53cb6516866</anchor>
      <arglist>(L_Cb *cb)</arglist>
    </member>
    <member kind="function">
      <type>int</type>
      <name>L_local_constant_folding</name>
      <anchorfile>l__local__opti_8c.html</anchorfile>
      <anchor>892c7753c4749dc7070fc223e868e071</anchor>
      <arglist>(L_Cb *cb)</arglist>
    </member>
    <member kind="function">
      <type>int</type>
      <name>L_constant_fold_subroutine</name>
      <anchorfile>l__local__opti_8c.html</anchorfile>
      <anchor>f2dd12b050bf82c7d44be121c03e1ea0</anchor>
      <arglist>(L_Operand *label, int *pcount)</arglist>
    </member>
    <member kind="function">
      <type>float</type>
      <name>L_evaluate_function_with_sources</name>
      <anchorfile>l__local__opti_8c.html</anchorfile>
      <anchor>c51ddc02781ab6362cc55d903811e788</anchor>
      <arglist>(int func_type, double s0, double s1)</arglist>
    </member>
    <member kind="function">
      <type>float</type>
      <name>L_evaluate_function</name>
      <anchorfile>l__local__opti_8c.html</anchorfile>
      <anchor>f0681b9f3f6c7d583933f44bd066e414</anchor>
      <arglist>(int func_type, L_Operand *src0, L_Operand *src1)</arglist>
    </member>
    <member kind="function">
      <type>int</type>
      <name>L_local_complex_constant_folding</name>
      <anchorfile>l__local__opti_8c.html</anchorfile>
      <anchor>a4c5aa7ed81c646ba9eba6c989dfe102</anchor>
      <arglist>(L_Cb *cb)</arglist>
    </member>
    <member kind="function">
      <type>int</type>
      <name>L_local_constant_combining</name>
      <anchorfile>l__local__opti_8c.html</anchorfile>
      <anchor>cb458d20a95115cfb4604fde04b83e8e</anchor>
      <arglist>(L_Cb *cb)</arglist>
    </member>
    <member kind="function">
      <type>int</type>
      <name>L_local_strength_reduction</name>
      <anchorfile>l__local__opti_8c.html</anchorfile>
      <anchor>bad48e5fef4c16397d3203c531b90f77</anchor>
      <arglist>(L_Cb *cb)</arglist>
    </member>
    <member kind="function">
      <type>int</type>
      <name>L_local_operation_folding</name>
      <anchorfile>l__local__opti_8c.html</anchorfile>
      <anchor>51f45c37c9878230e95340cc74552eb5</anchor>
      <arglist>(L_Cb *cb)</arglist>
    </member>
    <member kind="function">
      <type>int</type>
      <name>L_local_branch_folding</name>
      <anchorfile>l__local__opti_8c.html</anchorfile>
      <anchor>7051e4d99f1cd9148b6cac5b59ee8e19</anchor>
      <arglist>(L_Cb *cb)</arglist>
    </member>
    <member kind="function">
      <type>int</type>
      <name>L_local_operation_cancellation</name>
      <anchorfile>l__local__opti_8c.html</anchorfile>
      <anchor>3e5d5c7f3df61470237ce0e8623bbc9a</anchor>
      <arglist>(L_Cb *cb)</arglist>
    </member>
    <member kind="function">
      <type>int</type>
      <name>L_local_dead_code_removal</name>
      <anchorfile>l__local__opti_8c.html</anchorfile>
      <anchor>9d77946beaafeb1ed17f318a4528a229</anchor>
      <arglist>(L_Cb *cb)</arglist>
    </member>
    <member kind="function">
      <type>int</type>
      <name>L_is_oper_between</name>
      <anchorfile>l__local__opti_8c.html</anchorfile>
      <anchor>7b00dfa213ad4fabd937956e09111254</anchor>
      <arglist>(L_Oper *target, L_Oper *opA, L_Oper *opB)</arglist>
    </member>
    <member kind="function">
      <type>int</type>
      <name>L_local_code_motion</name>
      <anchorfile>l__local__opti_8c.html</anchorfile>
      <anchor>254ff254827608ef5cad012257fc0a32</anchor>
      <arglist>(L_Cb *cb)</arglist>
    </member>
    <member kind="function">
      <type>int</type>
      <name>L_local_remove_sign_extension</name>
      <anchorfile>l__local__opti_8c.html</anchorfile>
      <anchor>22b2aabeb1419616e1ac90e1d7cbe2b5</anchor>
      <arglist>(L_Cb *cb)</arglist>
    </member>
    <member kind="function">
      <type>int</type>
      <name>L_local_register_renaming</name>
      <anchorfile>l__local__opti_8c.html</anchorfile>
      <anchor>13f62b684d28fb5c68792c3f99a426ed</anchor>
      <arglist>(L_Cb *cb)</arglist>
    </member>
    <member kind="function">
      <type>int</type>
      <name>L_local_operation_migration</name>
      <anchorfile>l__local__opti_8c.html</anchorfile>
      <anchor>20349742c3df4d31cd1a77922aaa4f7e</anchor>
      <arglist>(L_Cb *cb, int complex_mig)</arglist>
    </member>
    <member kind="function">
      <type>int</type>
      <name>L_local_logic_reduction</name>
      <anchorfile>l__local__opti_8c.html</anchorfile>
      <anchor>067c8a0d85c59564e6a5ab5fed86bd57</anchor>
      <arglist>(L_Cb *cb)</arglist>
    </member>
    <member kind="function">
      <type>int</type>
      <name>L_local_oper_breakdown</name>
      <anchorfile>l__local__opti_8c.html</anchorfile>
      <anchor>c661ef0f904aca54592923a797971de5</anchor>
      <arglist>(L_Cb *cb)</arglist>
    </member>
    <member kind="function">
      <type>int</type>
      <name>L_local_oper_recombine</name>
      <anchorfile>l__local__opti_8c.html</anchorfile>
      <anchor>109b6f2d52105b85b778be591dce8d18</anchor>
      <arglist>(L_Cb *cb)</arglist>
    </member>
    <member kind="function">
      <type>int</type>
      <name>L_local_pred_dead_code_removal</name>
      <anchorfile>l__local__opti_8c.html</anchorfile>
      <anchor>67f51afbbab8a1842250e463824e1156</anchor>
      <arglist>(L_Cb *cb)</arglist>
    </member>
    <member kind="function">
      <type>int</type>
      <name>L_local_branch_val_propagation</name>
      <anchorfile>l__local__opti_8c.html</anchorfile>
      <anchor>c4b57629409cef710e28bda700e5d365</anchor>
      <arglist>(L_Cb *cb)</arglist>
    </member>
    <member kind="variable" static="yes">
      <type>static int</type>
      <name>subset_offset</name>
      <anchorfile>l__local__opti_8c.html</anchorfile>
      <anchor>af21d1a3db2670a9a4c4f7f7d93f7d5b</anchor>
      <arglist></arglist>
    </member>
    <member kind="variable">
      <type>char *</type>
      <name>CCF_function</name>
      <anchorfile>l__local__opti_8c.html</anchorfile>
      <anchor>51f63f00ec6e4efb0d45403fb3618485</anchor>
      <arglist>[MAX_FN_NUMBER]</arglist>
    </member>
    <member kind="variable">
      <type>int</type>
      <name>CCF_pcount</name>
      <anchorfile>l__local__opti_8c.html</anchorfile>
      <anchor>edfc0c89b7fa1f6e16f6496b53780181</anchor>
      <arglist>[MAX_FN_NUMBER]</arglist>
    </member>
  </compound>
  <compound kind="file">
    <name>l_local_opti.h</name>
    <path>/mnt/hgfs/mkt/Desktop/openimpact/src/Lcode/opti/Lopti/</path>
    <filename>l__local__opti_8h</filename>
    <member kind="define">
      <type>#define</type>
      <name>L_COMMON_SUB_MOVES_WITH_INT_CONSTANT</name>
      <anchorfile>l__local__opti_8h.html</anchorfile>
      <anchor>1ee9f95c5878d7e82c110fe513605337</anchor>
      <arglist></arglist>
    </member>
    <member kind="define">
      <type>#define</type>
      <name>L_COMMON_SUB_MOVES_WITH_FLT_CONSTANT</name>
      <anchorfile>l__local__opti_8h.html</anchorfile>
      <anchor>e10915046631f67b1bce10950bbc3531</anchor>
      <arglist></arglist>
    </member>
    <member kind="define">
      <type>#define</type>
      <name>L_COMMON_SUB_MOVES_WITH_LABEL_CONSTANT</name>
      <anchorfile>l__local__opti_8h.html</anchorfile>
      <anchor>f62c636f052fd996fe09f23613c0d7a1</anchor>
      <arglist></arglist>
    </member>
    <member kind="define">
      <type>#define</type>
      <name>L_COMMON_SUB_MOVES_WITH_STRING_CONSTANT</name>
      <anchorfile>l__local__opti_8h.html</anchorfile>
      <anchor>868d07056953f0c27410aeb1c599ff70</anchor>
      <arglist></arglist>
    </member>
    <member kind="function">
      <type>int</type>
      <name>L_local_constant_propagation</name>
      <anchorfile>l__local__opti_8h.html</anchorfile>
      <anchor>6eec765de643444da580eeae4bd31e04</anchor>
      <arglist>(L_Cb *, int)</arglist>
    </member>
    <member kind="function">
      <type>int</type>
      <name>L_local_copy_propagation</name>
      <anchorfile>l__local__opti_8h.html</anchorfile>
      <anchor>f6fb372f2fc6eb7adf59f37cdd047b47</anchor>
      <arglist>(L_Cb *)</arglist>
    </member>
    <member kind="function">
      <type>int</type>
      <name>L_local_rev_copy_propagation</name>
      <anchorfile>l__local__opti_8h.html</anchorfile>
      <anchor>5d341c064cc6db44d612bf45bb36afa2</anchor>
      <arglist>(L_Cb *)</arglist>
    </member>
    <member kind="function">
      <type>int</type>
      <name>L_local_memory_copy_propagation</name>
      <anchorfile>l__local__opti_8h.html</anchorfile>
      <anchor>b44c488f54014db9abfeb9eb62ab91eb</anchor>
      <arglist>(L_Cb *)</arglist>
    </member>
    <member kind="function">
      <type>int</type>
      <name>L_local_common_subexpression</name>
      <anchorfile>l__local__opti_8h.html</anchorfile>
      <anchor>d207a0b7e8cf23fa946df9104abb5ddf</anchor>
      <arglist>(L_Cb *, int)</arglist>
    </member>
    <member kind="function">
      <type>int</type>
      <name>L_local_redundant_load</name>
      <anchorfile>l__local__opti_8h.html</anchorfile>
      <anchor>bae4e3eb94e8737aaad313dc966526ce</anchor>
      <arglist>(L_Cb *)</arglist>
    </member>
    <member kind="function">
      <type>int</type>
      <name>L_local_redundant_store</name>
      <anchorfile>l__local__opti_8h.html</anchorfile>
      <anchor>fe0bed266e80a3d2ec0de3cc137842e6</anchor>
      <arglist>(L_Cb *)</arglist>
    </member>
    <member kind="function">
      <type>int</type>
      <name>L_local_constant_folding</name>
      <anchorfile>l__local__opti_8h.html</anchorfile>
      <anchor>3c8492e6b8eb26bcd9b87582d4291dbb</anchor>
      <arglist>(L_Cb *)</arglist>
    </member>
    <member kind="function">
      <type>int</type>
      <name>L_local_constant_combining</name>
      <anchorfile>l__local__opti_8h.html</anchorfile>
      <anchor>fd57171710b52a655f64b8b2577ea1b4</anchor>
      <arglist>(L_Cb *)</arglist>
    </member>
    <member kind="function">
      <type>int</type>
      <name>L_local_strength_reduction</name>
      <anchorfile>l__local__opti_8h.html</anchorfile>
      <anchor>c66fe083732f27f2d866b00e0f272a07</anchor>
      <arglist>(L_Cb *)</arglist>
    </member>
    <member kind="function">
      <type>int</type>
      <name>L_local_operation_folding</name>
      <anchorfile>l__local__opti_8h.html</anchorfile>
      <anchor>eccc59eabf51ad164a83cbde1425d258</anchor>
      <arglist>(L_Cb *)</arglist>
    </member>
    <member kind="function">
      <type>int</type>
      <name>L_local_branch_folding</name>
      <anchorfile>l__local__opti_8h.html</anchorfile>
      <anchor>063ed96ad214e4845b386e00aa831e09</anchor>
      <arglist>(L_Cb *)</arglist>
    </member>
    <member kind="function">
      <type>int</type>
      <name>L_local_operation_cancellation</name>
      <anchorfile>l__local__opti_8h.html</anchorfile>
      <anchor>8884a20f12315ee69b2149823b2a79d5</anchor>
      <arglist>(L_Cb *)</arglist>
    </member>
    <member kind="function">
      <type>int</type>
      <name>L_local_dead_code_removal</name>
      <anchorfile>l__local__opti_8h.html</anchorfile>
      <anchor>a75f07b5e0480d6fd7dccd4afabcda77</anchor>
      <arglist>(L_Cb *)</arglist>
    </member>
    <member kind="function">
      <type>int</type>
      <name>L_local_pred_dead_code_removal</name>
      <anchorfile>l__local__opti_8h.html</anchorfile>
      <anchor>d2c5a355c379d29e65bd7114ab2fe78a</anchor>
      <arglist>(L_Cb *)</arglist>
    </member>
    <member kind="function">
      <type>int</type>
      <name>L_local_code_motion</name>
      <anchorfile>l__local__opti_8h.html</anchorfile>
      <anchor>525702470db889398fa52f3d312c4dba</anchor>
      <arglist>(L_Cb *)</arglist>
    </member>
    <member kind="function">
      <type>int</type>
      <name>L_local_remove_sign_extension</name>
      <anchorfile>l__local__opti_8h.html</anchorfile>
      <anchor>fb41f04b34a667128ddf5993958d5b2d</anchor>
      <arglist>(L_Cb *)</arglist>
    </member>
    <member kind="function">
      <type>int</type>
      <name>L_local_register_renaming</name>
      <anchorfile>l__local__opti_8h.html</anchorfile>
      <anchor>468c2d5572e5ea4d5c7204739205edca</anchor>
      <arglist>(L_Cb *)</arglist>
    </member>
    <member kind="function">
      <type>int</type>
      <name>L_local_operation_migration</name>
      <anchorfile>l__local__opti_8h.html</anchorfile>
      <anchor>5880fe2f84afc996b6ba2fb2c4f8baff</anchor>
      <arglist>(L_Cb *, int)</arglist>
    </member>
    <member kind="function">
      <type>int</type>
      <name>L_local_logic_reduction</name>
      <anchorfile>l__local__opti_8h.html</anchorfile>
      <anchor>478721415aa49e1ba5733230f9878334</anchor>
      <arglist>(L_Cb *)</arglist>
    </member>
    <member kind="function">
      <type>int</type>
      <name>L_local_oper_breakdown</name>
      <anchorfile>l__local__opti_8h.html</anchorfile>
      <anchor>8085129c26c47ef9bea4110bd376e1ad</anchor>
      <arglist>(L_Cb *)</arglist>
    </member>
    <member kind="function">
      <type>int</type>
      <name>L_local_oper_recombine</name>
      <anchorfile>l__local__opti_8h.html</anchorfile>
      <anchor>a88ba7d8e3d88971a8f090011f645760</anchor>
      <arglist>(L_Cb *)</arglist>
    </member>
    <member kind="function">
      <type>int</type>
      <name>L_local_branch_val_propagation</name>
      <anchorfile>l__local__opti_8h.html</anchorfile>
      <anchor>d558ca18c0e2283ff24d68d564f37464</anchor>
      <arglist>(L_Cb *)</arglist>
    </member>
  </compound>
  <compound kind="file">
    <name>l_loop_driver.c</name>
    <path>/mnt/hgfs/mkt/Desktop/openimpact/src/Lcode/opti/Lopti/</path>
    <filename>l__loop__driver_8c</filename>
    <includes id="l__opti_8h" name="l_opti.h" local="yes" imported="no">l_opti.h</includes>
    <includes id="l__pred__opti_8h" name="l_pred_opti.h" local="yes" imported="no">l_pred_opti.h</includes>
    <member kind="define">
      <type>#define</type>
      <name>DO_LOOP_INVAR_CODE_REM</name>
      <anchorfile>l__loop__driver_8c.html</anchorfile>
      <anchor>45a32d6332a9861b0b4f50cb88a283ff</anchor>
      <arglist></arglist>
    </member>
    <member kind="define">
      <type>#define</type>
      <name>DO_LOOP_SIMPLIFY_LOOP_BR</name>
      <anchorfile>l__loop__driver_8c.html</anchorfile>
      <anchor>1b8bf7bc021d8c8554b5cc1dd9366dfb</anchor>
      <arglist></arglist>
    </member>
    <member kind="define">
      <type>#define</type>
      <name>DO_LOOP_GLOBAL_VAR_MIG</name>
      <anchorfile>l__loop__driver_8c.html</anchorfile>
      <anchor>976038eaab47dabaee2bdb6b5585d8ea</anchor>
      <arglist></arglist>
    </member>
    <member kind="define">
      <type>#define</type>
      <name>DO_LOOP_IND_STR_RED</name>
      <anchorfile>l__loop__driver_8c.html</anchorfile>
      <anchor>75d497ec49ec12e238f0b7cebabf8024</anchor>
      <arglist></arglist>
    </member>
    <member kind="define">
      <type>#define</type>
      <name>DO_LOOP_IND_ELIM</name>
      <anchorfile>l__loop__driver_8c.html</anchorfile>
      <anchor>aa86e16260f412b92966f04ea6d5a17d</anchor>
      <arglist></arglist>
    </member>
    <member kind="define">
      <type>#define</type>
      <name>DO_LOOP_DEAD_REM</name>
      <anchorfile>l__loop__driver_8c.html</anchorfile>
      <anchor>8aa60ec5a33334c9ebdd9f39062a10d0</anchor>
      <arglist></arglist>
    </member>
    <member kind="function">
      <type>int</type>
      <name>L_loop_optimization</name>
      <anchorfile>l__loop__driver_8c.html</anchorfile>
      <anchor>efb1182842ba555f3819a1e695631587</anchor>
      <arglist>(L_Func *fn)</arglist>
    </member>
  </compound>
  <compound kind="file">
    <name>l_loop_opti.c</name>
    <path>/mnt/hgfs/mkt/Desktop/openimpact/src/Lcode/opti/Lopti/</path>
    <filename>l__loop__opti_8c</filename>
    <includes id="l__opti_8h" name="l_opti.h" local="yes" imported="no">l_opti.h</includes>
    <class kind="struct">_invariant_address_access</class>
    <class kind="struct">_operand_pair</class>
    <class kind="struct">_operand_node</class>
    <member kind="define">
      <type>#define</type>
      <name>BUFSIZE</name>
      <anchorfile>l__loop__opti_8c.html</anchorfile>
      <anchor>eca034f67218340ecb2261a22c2f3dcd</anchor>
      <arglist></arglist>
    </member>
    <member kind="define">
      <type>#define</type>
      <name>ERR</name>
      <anchorfile>l__loop__opti_8c.html</anchorfile>
      <anchor>735563036dced0b7d6cc98f97ea4978b</anchor>
      <arglist></arglist>
    </member>
    <member kind="define">
      <type>#define</type>
      <name>LOOP_USE_WEIGHT_METRIC</name>
      <anchorfile>l__loop__opti_8c.html</anchorfile>
      <anchor>7d71d0bf767fe351580d955b09eed8a8</anchor>
      <arglist></arglist>
    </member>
    <member kind="define">
      <type>#define</type>
      <name>LOOP_USE_CUTSET_METRIC</name>
      <anchorfile>l__loop__opti_8c.html</anchorfile>
      <anchor>0e30d7e8a5662d681e5385a81415dd87</anchor>
      <arglist></arglist>
    </member>
    <member kind="define">
      <type>#define</type>
      <name>LOOP_CUTSET_RATIO</name>
      <anchorfile>l__loop__opti_8c.html</anchorfile>
      <anchor>d89142d5239a086350c7ab5e53dadaf4</anchor>
      <arglist></arglist>
    </member>
    <member kind="typedef">
      <type>_invariant_address_access</type>
      <name>invariant_address_access</name>
      <anchorfile>l__loop__opti_8c.html</anchorfile>
      <anchor>b5abaae62791eb3977c11ed917cb9ad8</anchor>
      <arglist></arglist>
    </member>
    <member kind="typedef">
      <type>_operand_pair</type>
      <name>operand_pair</name>
      <anchorfile>l__loop__opti_8c.html</anchorfile>
      <anchor>a355d489ac47ab6e3bcec8952d7944e8</anchor>
      <arglist></arglist>
    </member>
    <member kind="typedef">
      <type>_operand_node</type>
      <name>operand_node</name>
      <anchorfile>l__loop__opti_8c.html</anchorfile>
      <anchor>56647168406c663380fb6b99dafcb2eb</anchor>
      <arglist></arglist>
    </member>
    <member kind="function" static="yes">
      <type>static L_Oper *</type>
      <name>L_insert_alias_compensation_store</name>
      <anchorfile>l__loop__opti_8c.html</anchorfile>
      <anchor>6fccd5bf9d18f207b11bb726ca580ff2</anchor>
      <arglist>(L_Cb *old_cb, L_Oper *store_oper, L_Oper *alias_oper, int st_opc, L_Operand *st_s1, L_Operand *st_s2, L_Operand *st_s3, L_Attr *attr, int flags, int safe, L_Oper *cond_set)</arglist>
    </member>
    <member kind="function" static="yes">
      <type>static void</type>
      <name>L_insert_alias_compensation_load</name>
      <anchorfile>l__loop__opti_8c.html</anchorfile>
      <anchor>600211afb23411227c7dda9c0f91c924</anchor>
      <arglist>(L_Cb *cb, L_Oper *old_oper, L_Oper *alias_oper, int ld_opc, L_Operand *ld_dest, L_Operand *ld_s1, L_Operand *ld_s2, L_Attr *attr, int flags, int safe)</arglist>
    </member>
    <member kind="function" static="yes">
      <type>static L_Oper *</type>
      <name>L_insert_load_ops_for_global_var_mig</name>
      <anchorfile>l__loop__opti_8c.html</anchorfile>
      <anchor>88767e640825497578d66934eb7f3189</anchor>
      <arglist>(L_Loop *, L_Oper *, int, L_Operand *, L_Operand *, L_Operand *, L_Attr *, int, int, L_Cb *)</arglist>
    </member>
    <member kind="function" static="yes">
      <type>static void</type>
      <name>L_insert_store_ops_for_global_var_mig</name>
      <anchorfile>l__loop__opti_8c.html</anchorfile>
      <anchor>b0f0c2955b09b1f535d1284a23596f3c</anchor>
      <arglist>(L_Loop *, int *, int, L_Oper *, int, L_Operand *, L_Operand *, L_Operand *, L_Attr *, int, int, L_Oper **pred_set)</arglist>
    </member>
    <member kind="function">
      <type>int</type>
      <name>L_loop_invariant_code_removal</name>
      <anchorfile>l__loop__opti_8c.html</anchorfile>
      <anchor>34ebf29209aeca60e2b5ba75a4bf3e34</anchor>
      <arglist>(L_Loop *loop)</arglist>
    </member>
    <member kind="function">
      <type>int</type>
      <name>L_loop_global_var_migration</name>
      <anchorfile>l__loop__opti_8c.html</anchorfile>
      <anchor>e3264cc6a9b308b7f09dc067ff4525b7</anchor>
      <arglist>(L_Loop *loop)</arglist>
    </member>
    <member kind="function" static="yes">
      <type>static invariant_address_access *</type>
      <name>L_new_invariant_address_access</name>
      <anchorfile>l__loop__opti_8c.html</anchorfile>
      <anchor>f414d24f81b64be3b9d4bdd21103fea7</anchor>
      <arglist>()</arglist>
    </member>
    <member kind="function" static="yes">
      <type>static void</type>
      <name>L_free_invariant_address_access</name>
      <anchorfile>l__loop__opti_8c.html</anchorfile>
      <anchor>cef7b3d6b842a3fb9a36a3e3ddc841ef</anchor>
      <arglist>(void *vp)</arglist>
    </member>
    <member kind="function" static="yes">
      <type>static void</type>
      <name>L_generate_operand_name</name>
      <anchorfile>l__loop__opti_8c.html</anchorfile>
      <anchor>15f2de6f2c7155947a0f425ee9f26c8a</anchor>
      <arglist>(L_Operand *oper, char *oper_name)</arglist>
    </member>
    <member kind="function" static="yes">
      <type>static operand_node *</type>
      <name>L_new_operand_node</name>
      <anchorfile>l__loop__opti_8c.html</anchorfile>
      <anchor>c56c27690fc4c7ac86622ecdfa69fbc9</anchor>
      <arglist>(L_Oper *oper, L_Operand *op, Set loop_ops)</arglist>
    </member>
    <member kind="function" static="yes">
      <type>static operand_node *</type>
      <name>L_find_ast_node</name>
      <anchorfile>l__loop__opti_8c.html</anchorfile>
      <anchor>0a8fd15fbbdc477da78a481382afafd0</anchor>
      <arglist>(L_Oper *op, L_Operand *opd, Set loop_ops)</arglist>
    </member>
    <member kind="function" static="yes">
      <type>static operand_pair *</type>
      <name>L_new_operand_pair</name>
      <anchorfile>l__loop__opti_8c.html</anchorfile>
      <anchor>871461dd2335e5822d4455b6dbe053d7</anchor>
      <arglist>(L_Oper *op, Set loop_ops)</arglist>
    </member>
    <member kind="function" static="yes">
      <type>static void</type>
      <name>L_delete_operand_pair</name>
      <anchorfile>l__loop__opti_8c.html</anchorfile>
      <anchor>99b11606e9eaf77b2ac2a43538eac8c1</anchor>
      <arglist>(operand_pair *p)</arglist>
    </member>
    <member kind="function" static="yes">
      <type>static void</type>
      <name>L_delete_operand_node</name>
      <anchorfile>l__loop__opti_8c.html</anchorfile>
      <anchor>7576197d854d1f16c0be832dfbd07cb3</anchor>
      <arglist>(operand_node *p)</arglist>
    </member>
    <member kind="function" static="yes">
      <type>static int</type>
      <name>L_def_already_found</name>
      <anchorfile>l__loop__opti_8c.html</anchorfile>
      <anchor>2982da491452edd56a475aeb7b9aea4c</anchor>
      <arglist>(operand_node *op_node, L_Oper *op)</arglist>
    </member>
    <member kind="function" static="yes">
      <type>static void</type>
      <name>L_build_ast_for_reg_in_loop</name>
      <anchorfile>l__loop__opti_8c.html</anchorfile>
      <anchor>947b4dd7629f27793fe30ce8be0b6293</anchor>
      <arglist>(L_Loop *loop, int *loop_cb, int num_cb)</arglist>
    </member>
    <member kind="function" static="yes">
      <type>static void</type>
      <name>L_delete_ast_for_reg_in_loop</name>
      <anchorfile>l__loop__opti_8c.html</anchorfile>
      <anchor>1e16f180fde61b6a6d6c902ccf3ef289</anchor>
      <arglist>()</arglist>
    </member>
    <member kind="function" static="yes">
      <type>static int</type>
      <name>L_find_oper_acc_name</name>
      <anchorfile>l__loop__opti_8c.html</anchorfile>
      <anchor>8e6a1f99470f3528ffd7a454b574b16c</anchor>
      <arglist>(L_Operand *operand, char *acc_name)</arglist>
    </member>
    <member kind="function" static="yes">
      <type>static L_Operand *</type>
      <name>L_initialize_addr_register</name>
      <anchorfile>l__loop__opti_8c.html</anchorfile>
      <anchor>06070d18f6957aa51b385f5ac4fa1165</anchor>
      <arglist>(L_Loop *loop, operand_node *op_node)</arglist>
    </member>
    <member kind="function" static="yes">
      <type>static int</type>
      <name>L_gen_oper_acc_name</name>
      <anchorfile>l__loop__opti_8c.html</anchorfile>
      <anchor>bcb5ed93ee3722830aa21d735dda3b0a</anchor>
      <arglist>(L_Oper *op, char *buf)</arglist>
    </member>
    <member kind="function">
      <type>int</type>
      <name>L_loop_global_var_migration_by_sync</name>
      <anchorfile>l__loop__opti_8c.html</anchorfile>
      <anchor>276327d59f9c5375745a6c2b433e9645</anchor>
      <arglist>(L_Loop *loop)</arglist>
    </member>
    <member kind="function">
      <type>int</type>
      <name>L_loop_simplify_back_branch</name>
      <anchorfile>l__loop__opti_8c.html</anchorfile>
      <anchor>3a60837e2d80f458b5797132cb26ce70</anchor>
      <arglist>(L_Loop *loop)</arglist>
    </member>
    <member kind="function">
      <type>int</type>
      <name>L_loop_induction_strength_reduction</name>
      <anchorfile>l__loop__opti_8c.html</anchorfile>
      <anchor>bc08c5eca949a83261578d52e1ce810c</anchor>
      <arglist>(L_Loop *loop)</arglist>
    </member>
    <member kind="function" static="yes">
      <type>static int</type>
      <name>L_loop_induction_elimination0</name>
      <anchorfile>l__loop__opti_8c.html</anchorfile>
      <anchor>d22681d54876ffa232bf1cfa2046429c</anchor>
      <arglist>(L_Loop *loop)</arglist>
    </member>
    <member kind="function" static="yes">
      <type>static int</type>
      <name>L_loop_induction_elimination1</name>
      <anchorfile>l__loop__opti_8c.html</anchorfile>
      <anchor>0312e4a5c93a647c0bca0b98365b0b8f</anchor>
      <arglist>(L_Loop *loop)</arglist>
    </member>
    <member kind="function" static="yes">
      <type>static int</type>
      <name>L_loop_induction_elimination2</name>
      <anchorfile>l__loop__opti_8c.html</anchorfile>
      <anchor>5db5e374b0d0745b6ae27338dd021e04</anchor>
      <arglist>(L_Loop *loop)</arglist>
    </member>
    <member kind="function" static="yes">
      <type>static int</type>
      <name>L_loop_induction_elimination3</name>
      <anchorfile>l__loop__opti_8c.html</anchorfile>
      <anchor>48bb8cbf15e41aa404c45a44bb7efe8e</anchor>
      <arglist>(L_Loop *loop)</arglist>
    </member>
    <member kind="function" static="yes">
      <type>static int</type>
      <name>L_loop_induction_elimination4</name>
      <anchorfile>l__loop__opti_8c.html</anchorfile>
      <anchor>5cb1e388da4760059218663b1e310f17</anchor>
      <arglist>(L_Loop *loop)</arglist>
    </member>
    <member kind="function" static="yes">
      <type>static int</type>
      <name>L_loop_induction_reinit</name>
      <anchorfile>l__loop__opti_8c.html</anchorfile>
      <anchor>fc633ea430665d99ac9e65d44601f2f1</anchor>
      <arglist>(L_Loop *loop)</arglist>
    </member>
    <member kind="function" static="yes">
      <type>static int</type>
      <name>L_loop_induction_reassociation</name>
      <anchorfile>l__loop__opti_8c.html</anchorfile>
      <anchor>20eb74bd9ca8de7b5ec7aa9daf94cd81</anchor>
      <arglist>(L_Loop *loop)</arglist>
    </member>
    <member kind="function">
      <type>int</type>
      <name>L_loop_induction_elimination</name>
      <anchorfile>l__loop__opti_8c.html</anchorfile>
      <anchor>7adf2805cf3a6176ca5f8a4260166502</anchor>
      <arglist>(L_Loop *loop)</arglist>
    </member>
    <member kind="function">
      <type>int</type>
      <name>L_loop_post_increment_conversion</name>
      <anchorfile>l__loop__opti_8c.html</anchorfile>
      <anchor>cafaec09ff33a617a4842fc834d0f49e</anchor>
      <arglist>(L_Loop *loop)</arglist>
    </member>
    <member kind="function">
      <type>int</type>
      <name>L_dead_loop_removal</name>
      <anchorfile>l__loop__opti_8c.html</anchorfile>
      <anchor>7a93cf2cf1a8bf9fac602c810576f98b</anchor>
      <arglist>(L_Loop *loop)</arglist>
    </member>
    <member kind="variable" static="yes">
      <type>static L_Alloc_Pool *</type>
      <name>Invariant_Address_Access_Pool</name>
      <anchorfile>l__loop__opti_8c.html</anchorfile>
      <anchor>69a300fd7f35fa6c0fdcd879b9c59b5f</anchor>
      <arglist></arglist>
    </member>
    <member kind="variable" static="yes">
      <type>static L_Alloc_Pool *</type>
      <name>Operand_Node_Pool</name>
      <anchorfile>l__loop__opti_8c.html</anchorfile>
      <anchor>8101fd1c87eaf1350588db05315c814f</anchor>
      <arglist></arglist>
    </member>
    <member kind="variable" static="yes">
      <type>static L_Alloc_Pool *</type>
      <name>Operand_Pair_Pool</name>
      <anchorfile>l__loop__opti_8c.html</anchorfile>
      <anchor>a5f76b227e380797a516500a5de73e9c</anchor>
      <arglist></arglist>
    </member>
    <member kind="variable" static="yes">
      <type>static STRING_Symbol_Table *</type>
      <name>operand_node_tbl</name>
      <anchorfile>l__loop__opti_8c.html</anchorfile>
      <anchor>ef878b4615d8956d76052232471d34cd</anchor>
      <arglist></arglist>
    </member>
  </compound>
  <compound kind="file">
    <name>l_loop_opti.h</name>
    <path>/mnt/hgfs/mkt/Desktop/openimpact/src/Lcode/opti/Lopti/</path>
    <filename>l__loop__opti_8h</filename>
    <member kind="function">
      <type>int</type>
      <name>L_loop_invariant_code_removal</name>
      <anchorfile>l__loop__opti_8h.html</anchorfile>
      <anchor>03b7b10e5f3b394723d9c77ec1daa490</anchor>
      <arglist>(L_Loop *)</arglist>
    </member>
    <member kind="function">
      <type>int</type>
      <name>L_loop_global_var_migration</name>
      <anchorfile>l__loop__opti_8h.html</anchorfile>
      <anchor>932d0830e17dbc8197bd27a5df4cbdc9</anchor>
      <arglist>(L_Loop *)</arglist>
    </member>
    <member kind="function">
      <type>int</type>
      <name>L_loop_global_var_migration_by_sync</name>
      <anchorfile>l__loop__opti_8h.html</anchorfile>
      <anchor>644d78e8da2940ddef4cc7a543ef8034</anchor>
      <arglist>(L_Loop *)</arglist>
    </member>
    <member kind="function">
      <type>int</type>
      <name>L_loop_simplify_back_branch</name>
      <anchorfile>l__loop__opti_8h.html</anchorfile>
      <anchor>0e7ca0fd02064832ac65553fc79c11ae</anchor>
      <arglist>(L_Loop *)</arglist>
    </member>
    <member kind="function">
      <type>int</type>
      <name>L_loop_induction_strength_reduction</name>
      <anchorfile>l__loop__opti_8h.html</anchorfile>
      <anchor>c692b75bb2153531dd80aa55bf532c93</anchor>
      <arglist>(L_Loop *)</arglist>
    </member>
    <member kind="function">
      <type>int</type>
      <name>L_loop_induction_elimination</name>
      <anchorfile>l__loop__opti_8h.html</anchorfile>
      <anchor>84a8499bda5cd9422943d90708a9bf67</anchor>
      <arglist>(L_Loop *)</arglist>
    </member>
    <member kind="function">
      <type>int</type>
      <name>L_loop_post_increment_conversion</name>
      <anchorfile>l__loop__opti_8h.html</anchorfile>
      <anchor>4ef72535279c6a3312b19b17e7bd19f0</anchor>
      <arglist>(L_Loop *)</arglist>
    </member>
    <member kind="function">
      <type>int</type>
      <name>L_dead_loop_removal</name>
      <anchorfile>l__loop__opti_8h.html</anchorfile>
      <anchor>74df545d2887212650bc6d8b11ef0ded</anchor>
      <arglist>(L_Loop *)</arglist>
    </member>
  </compound>
  <compound kind="file">
    <name>l_memflow_opti.c</name>
    <path>/mnt/hgfs/mkt/Desktop/openimpact/src/Lcode/opti/Lopti/</path>
    <filename>l__memflow__opti_8c</filename>
    <includes id="l__opti_8h" name="l_opti.h" local="yes" imported="no">l_opti.h</includes>
    <member kind="function">
      <type>int</type>
      <name>L_load_compatible_each_store</name>
      <anchorfile>l__memflow__opti_8c.html</anchorfile>
      <anchor>d9f407cab848d3452a826fd27fbeda57</anchor>
      <arglist>(L_Func *fn, L_Oper *oper, Set RDEF)</arglist>
    </member>
    <member kind="function">
      <type>int</type>
      <name>L_load_compatible_each_load</name>
      <anchorfile>l__memflow__opti_8c.html</anchorfile>
      <anchor>16762d739a305962cb2df1a7ee0bf0a6</anchor>
      <arglist>(L_Func *fn, L_Oper *oper, Set RDEF)</arglist>
    </member>
    <member kind="function">
      <type>int</type>
      <name>L_load_postdominates_each_store</name>
      <anchorfile>l__memflow__opti_8c.html</anchorfile>
      <anchor>0dbbf18b2ce60de6a5e17ee1d2026c17</anchor>
      <arglist>(L_Func *fn, L_Cb *cb, L_Oper *oper, Set RDEF)</arglist>
    </member>
    <member kind="function">
      <type>int</type>
      <name>L_cb_set_find_visit</name>
      <anchorfile>l__memflow__opti_8c.html</anchorfile>
      <anchor>b36e139bf657a4fcce1dcbf5cfb43388</anchor>
      <arglist>(L_Cb *start, L_Cb *stop)</arglist>
    </member>
    <member kind="function">
      <type>int</type>
      <name>L_cb_set_dominates</name>
      <anchorfile>l__memflow__opti_8c.html</anchorfile>
      <anchor>f4a017c40ba94cc72b6e08cf899c72aa</anchor>
      <arglist>(L_Func *fn, L_Cb *start, L_Cb *stop, Set cb_set)</arglist>
    </member>
    <member kind="function">
      <type>int</type>
      <name>L_store_union_dominates_load</name>
      <anchorfile>l__memflow__opti_8c.html</anchorfile>
      <anchor>bf5fdb77f39dd3cf202e3f8feb21497e</anchor>
      <arglist>(L_Func *fn, L_Cb *cb, L_Oper *oper, Set RDEF)</arglist>
    </member>
    <member kind="function">
      <type>int</type>
      <name>L_loadstore_union_postdominates_amb</name>
      <anchorfile>l__memflow__opti_8c.html</anchorfile>
      <anchor>ab1fbf597538be8244b1924056ed40ac</anchor>
      <arglist>(L_Func *fn, L_Cb *cb, L_Oper *oper, Set LDST, Set AMB)</arglist>
    </member>
    <member kind="function">
      <type>int</type>
      <name>L_global_memflow_multistore_load</name>
      <anchorfile>l__memflow__opti_8c.html</anchorfile>
      <anchor>4e85c1fe5d6d2aa711f566dad740110d</anchor>
      <arglist>(L_Func *fn, L_Cb *cb, L_Oper *oper)</arglist>
    </member>
    <member kind="function">
      <type>int</type>
      <name>L_global_memflow_multiloadstore_load</name>
      <anchorfile>l__memflow__opti_8c.html</anchorfile>
      <anchor>6e142331f17f17b807401dc3530652b7</anchor>
      <arglist>(L_Func *fn, L_Cb *cb, L_Oper *oper)</arglist>
    </member>
    <member kind="function">
      <type>int</type>
      <name>L_global_memflow_optimization</name>
      <anchorfile>l__memflow__opti_8c.html</anchorfile>
      <anchor>90ad9e372bd3285020a1a44edf8f62a7</anchor>
      <arglist>(L_Func *fn)</arglist>
    </member>
  </compound>
  <compound kind="file">
    <name>l_memflow_opti.h</name>
    <path>/mnt/hgfs/mkt/Desktop/openimpact/src/Lcode/opti/Lopti/</path>
    <filename>l__memflow__opti_8h</filename>
    <member kind="function">
      <type>int</type>
      <name>L_global_memflow_multistore_load</name>
      <anchorfile>l__memflow__opti_8h.html</anchorfile>
      <anchor>b62073474b265826eb631e79d04e5b3c</anchor>
      <arglist>(L_Func *, L_Cb *, L_Oper *)</arglist>
    </member>
    <member kind="function">
      <type>int</type>
      <name>L_global_memflow_optimization</name>
      <anchorfile>l__memflow__opti_8h.html</anchorfile>
      <anchor>99ffac2b1e4e787f9672cccc486ff6ad</anchor>
      <arglist>(L_Func *)</arglist>
    </member>
  </compound>
  <compound kind="file">
    <name>l_opti.h</name>
    <path>/mnt/hgfs/mkt/Desktop/openimpact/src/Lcode/opti/Lopti/</path>
    <filename>l__opti_8h</filename>
    <includes id="l__danger_8h" name="l_danger.h" local="yes" imported="no">l_danger.h</includes>
    <includes id="l__opti__predicates_8h" name="l_opti_predicates.h" local="yes" imported="no">l_opti_predicates.h</includes>
    <includes id="l__opti__functions_8h" name="l_opti_functions.h" local="yes" imported="no">l_opti_functions.h</includes>
    <includes id="l__local__opti_8h" name="l_local_opti.h" local="yes" imported="no">l_local_opti.h</includes>
    <includes id="l__global__opti_8h" name="l_global_opti.h" local="yes" imported="no">l_global_opti.h</includes>
    <includes id="l__memflow__opti_8h" name="l_memflow_opti.h" local="yes" imported="no">l_memflow_opti.h</includes>
    <includes id="l__jump__opti_8h" name="l_jump_opti.h" local="yes" imported="no">l_jump_opti.h</includes>
    <includes id="l__loop__opti_8h" name="l_loop_opti.h" local="yes" imported="no">l_loop_opti.h</includes>
    <includes id="l__branch_8h" name="l_branch.h" local="yes" imported="no">l_branch.h</includes>
    <includes id="l__PCE__opti_8h" name="l_PCE_opti.h" local="yes" imported="no">l_PCE_opti.h</includes>
    <includes id="l__pred__opti_8h" name="l_pred_opti.h" local="yes" imported="no">l_pred_opti.h</includes>
    <member kind="define">
      <type>#define</type>
      <name>L_POST_INC_MARK</name>
      <anchorfile>l__opti_8h.html</anchorfile>
      <anchor>f3e68f93613da74b0337ac7f8837baaf</anchor>
      <arglist></arglist>
    </member>
    <member kind="define">
      <type>#define</type>
      <name>L_PRE_INC_MARK</name>
      <anchorfile>l__opti_8h.html</anchorfile>
      <anchor>e0d05828a19b6257a661be3aaf4315c8</anchor>
      <arglist></arglist>
    </member>
    <member kind="define">
      <type>#define</type>
      <name>L_STORE_MIGRATION_FULL_PRED</name>
      <anchorfile>l__opti_8h.html</anchorfile>
      <anchor>4aac40497c0f2c95a3459f1068a8025a</anchor>
      <arglist></arglist>
    </member>
    <member kind="define">
      <type>#define</type>
      <name>L_STORE_MIGRATION_NO_PRED</name>
      <anchorfile>l__opti_8h.html</anchorfile>
      <anchor>da678197d4e506e034affe52a034c3c8</anchor>
      <arglist></arglist>
    </member>
    <member kind="define">
      <type>#define</type>
      <name>L_STORE_MIGRATION_NO_COND</name>
      <anchorfile>l__opti_8h.html</anchorfile>
      <anchor>f50ae9a66072da9c172d0fadfc0d4972</anchor>
      <arglist></arglist>
    </member>
    <member kind="function">
      <type>void</type>
      <name>Lopti_init</name>
      <anchorfile>l__opti_8h.html</anchorfile>
      <anchor>9beda43a41b15a7f35a71bacc8f780b4</anchor>
      <arglist>()</arglist>
    </member>
    <member kind="function">
      <type>void</type>
      <name>Lopti_deinit</name>
      <anchorfile>l__opti_8h.html</anchorfile>
      <anchor>2a25ade4b368852c90f2151c5e40c1d1</anchor>
      <arglist>()</arglist>
    </member>
    <member kind="function">
      <type>void</type>
      <name>L_code_optimize</name>
      <anchorfile>l__opti_8h.html</anchorfile>
      <anchor>6d1c108a57cd261e8ccf0ee57bc86c34</anchor>
      <arglist>(L_Func *)</arglist>
    </member>
    <member kind="function">
      <type>int</type>
      <name>L_do_longword_loop_conversion</name>
      <anchorfile>l__opti_8h.html</anchorfile>
      <anchor>864d2cb678855498a841f4f215288755</anchor>
      <arglist>(L_Loop *)</arglist>
    </member>
    <member kind="function">
      <type>int</type>
      <name>L_do_benchmark_tuning</name>
      <anchorfile>l__opti_8h.html</anchorfile>
      <anchor>0809b7289ec9a40bc3735c5360004ac3</anchor>
      <arglist>(L_Func *)</arglist>
    </member>
    <member kind="function">
      <type>void</type>
      <name>L_tag_load</name>
      <anchorfile>l__opti_8h.html</anchorfile>
      <anchor>4f315f037a292853c686fb5a9384b7fe</anchor>
      <arglist>(L_Func *)</arglist>
    </member>
    <member kind="variable">
      <type>int</type>
      <name>Lopti_opti_level</name>
      <anchorfile>l__opti_8h.html</anchorfile>
      <anchor>b6cc9f1f9ef13936f727f3d7790bf08d</anchor>
      <arglist></arglist>
    </member>
    <member kind="variable">
      <type>int</type>
      <name>Lopti_do_benchmark_specific_opti</name>
      <anchorfile>l__opti_8h.html</anchorfile>
      <anchor>d6ba2b78336cd55bbed471d8974453bb</anchor>
      <arglist></arglist>
    </member>
    <member kind="variable">
      <type>int</type>
      <name>Lopti_do_local_opti</name>
      <anchorfile>l__opti_8h.html</anchorfile>
      <anchor>48a43f36d3cfc8cd0e64d2d9e27a6e1e</anchor>
      <arglist></arglist>
    </member>
    <member kind="variable">
      <type>int</type>
      <name>Lopti_do_local_constant_prop</name>
      <anchorfile>l__opti_8h.html</anchorfile>
      <anchor>2b51b32868b675238ed3d360dec5fdba</anchor>
      <arglist></arglist>
    </member>
    <member kind="variable">
      <type>int</type>
      <name>Lopti_do_local_copy_prop</name>
      <anchorfile>l__opti_8h.html</anchorfile>
      <anchor>f38dc34103b85690393a088b07d43384</anchor>
      <arglist></arglist>
    </member>
    <member kind="variable">
      <type>int</type>
      <name>Lopti_do_local_rev_copy_prop</name>
      <anchorfile>l__opti_8h.html</anchorfile>
      <anchor>6fe39968d8193a76a1ca570a38d84b9b</anchor>
      <arglist></arglist>
    </member>
    <member kind="variable">
      <type>int</type>
      <name>Lopti_do_local_common_sub_elim</name>
      <anchorfile>l__opti_8h.html</anchorfile>
      <anchor>f81e0580b790b1450f2dde1e444fe1b5</anchor>
      <arglist></arglist>
    </member>
    <member kind="variable">
      <type>int</type>
      <name>Lopti_do_local_mem_copy_prop</name>
      <anchorfile>l__opti_8h.html</anchorfile>
      <anchor>f6f81dbc0283828adc73b53ca4cb1323</anchor>
      <arglist></arglist>
    </member>
    <member kind="variable">
      <type>int</type>
      <name>Lopti_do_local_red_load_elim</name>
      <anchorfile>l__opti_8h.html</anchorfile>
      <anchor>f171355ce8f8e2eb09c6718c726a4ec7</anchor>
      <arglist></arglist>
    </member>
    <member kind="variable">
      <type>int</type>
      <name>Lopti_do_local_red_store_elim</name>
      <anchorfile>l__opti_8h.html</anchorfile>
      <anchor>f4e47f0dd49b173f20efa49dd7e93e18</anchor>
      <arglist></arglist>
    </member>
    <member kind="variable">
      <type>int</type>
      <name>Lopti_do_local_constant_comb</name>
      <anchorfile>l__opti_8h.html</anchorfile>
      <anchor>e32ef97291fadacab40ad11eed6da241</anchor>
      <arglist></arglist>
    </member>
    <member kind="variable">
      <type>int</type>
      <name>Lopti_do_local_constant_fold</name>
      <anchorfile>l__opti_8h.html</anchorfile>
      <anchor>b4ac721c899caa505b82ffc2bba45a1c</anchor>
      <arglist></arglist>
    </member>
    <member kind="variable">
      <type>int</type>
      <name>Lopti_do_local_strength_red</name>
      <anchorfile>l__opti_8h.html</anchorfile>
      <anchor>9c4344b7fd1a5dc55c28defb44047cfa</anchor>
      <arglist></arglist>
    </member>
    <member kind="variable">
      <type>int</type>
      <name>Lopti_do_local_strength_red_for_signed_div_rem</name>
      <anchorfile>l__opti_8h.html</anchorfile>
      <anchor>e360ddddf64065625d518817167cc06f</anchor>
      <arglist></arglist>
    </member>
    <member kind="variable">
      <type>int</type>
      <name>Lopti_do_local_operation_fold</name>
      <anchorfile>l__opti_8h.html</anchorfile>
      <anchor>1436bf975849f81a0693dfb14d717831</anchor>
      <arglist></arglist>
    </member>
    <member kind="variable">
      <type>int</type>
      <name>Lopti_do_local_branch_fold</name>
      <anchorfile>l__opti_8h.html</anchorfile>
      <anchor>6b071820ac532884d6e3f2e5eee338c8</anchor>
      <arglist></arglist>
    </member>
    <member kind="variable">
      <type>int</type>
      <name>Lopti_do_local_operation_cancel</name>
      <anchorfile>l__opti_8h.html</anchorfile>
      <anchor>c55554d83aef0da021c3d4c12e1ab037</anchor>
      <arglist></arglist>
    </member>
    <member kind="variable">
      <type>int</type>
      <name>Lopti_do_local_dead_code_rem</name>
      <anchorfile>l__opti_8h.html</anchorfile>
      <anchor>bd8bf8a9cb0fca9c8f0808375a789635</anchor>
      <arglist></arglist>
    </member>
    <member kind="variable">
      <type>int</type>
      <name>Lopti_do_local_code_motion</name>
      <anchorfile>l__opti_8h.html</anchorfile>
      <anchor>75b1c0e287fc91a8d1b4ba5871961fce</anchor>
      <arglist></arglist>
    </member>
    <member kind="variable">
      <type>int</type>
      <name>Lopti_do_local_remove_sign_ext</name>
      <anchorfile>l__opti_8h.html</anchorfile>
      <anchor>52016f510018711b0149ad596639b375</anchor>
      <arglist></arglist>
    </member>
    <member kind="variable">
      <type>int</type>
      <name>Lopti_do_local_reduce_logic</name>
      <anchorfile>l__opti_8h.html</anchorfile>
      <anchor>13baf9289de9e30d52bafd075d2cb5fc</anchor>
      <arglist></arglist>
    </member>
    <member kind="variable">
      <type>int</type>
      <name>Lopti_do_local_register_rename</name>
      <anchorfile>l__opti_8h.html</anchorfile>
      <anchor>875a17dac0636f866d1fbb703ff12941</anchor>
      <arglist></arglist>
    </member>
    <member kind="variable">
      <type>int</type>
      <name>Lopti_do_local_op_breakdown</name>
      <anchorfile>l__opti_8h.html</anchorfile>
      <anchor>1b79505d950a7576adab063a25d4c29a</anchor>
      <arglist></arglist>
    </member>
    <member kind="variable">
      <type>int</type>
      <name>Lopti_do_local_op_recombine</name>
      <anchorfile>l__opti_8h.html</anchorfile>
      <anchor>39cfbac8f2a3dc5e42dfdc15552f3ee8</anchor>
      <arglist></arglist>
    </member>
    <member kind="variable">
      <type>int</type>
      <name>Lopti_do_local_branch_val_prop</name>
      <anchorfile>l__opti_8h.html</anchorfile>
      <anchor>3653a831f240dc8877e038c428c4c9e4</anchor>
      <arglist></arglist>
    </member>
    <member kind="variable">
      <type>int</type>
      <name>Lopti_debug_local_opti</name>
      <anchorfile>l__opti_8h.html</anchorfile>
      <anchor>d9c6cd30ffd288471563be3659ac0b8c</anchor>
      <arglist></arglist>
    </member>
    <member kind="variable">
      <type>int</type>
      <name>Lopti_ignore_sync_arcs_for_red_elim</name>
      <anchorfile>l__opti_8h.html</anchorfile>
      <anchor>59c6101096c4a4756292aa8bbe618d6b</anchor>
      <arglist></arglist>
    </member>
    <member kind="variable">
      <type>int</type>
      <name>Lopti_ignore_sync_arcs_for_loop_inv_migration</name>
      <anchorfile>l__opti_8h.html</anchorfile>
      <anchor>0327068e61449ece69266453b8275269</anchor>
      <arglist></arglist>
    </member>
    <member kind="variable">
      <type>int</type>
      <name>Lopti_do_global_opti</name>
      <anchorfile>l__opti_8h.html</anchorfile>
      <anchor>841399696e9c5ad241bafd94333a4741</anchor>
      <arglist></arglist>
    </member>
    <member kind="variable">
      <type>int</type>
      <name>Lopti_do_global_dead_code_rem</name>
      <anchorfile>l__opti_8h.html</anchorfile>
      <anchor>631efafeed6e68512dd0d0514515146a</anchor>
      <arglist></arglist>
    </member>
    <member kind="variable">
      <type>int</type>
      <name>Lopti_do_global_constant_prop</name>
      <anchorfile>l__opti_8h.html</anchorfile>
      <anchor>1acdda55963ba1c83b5453549039b423</anchor>
      <arglist></arglist>
    </member>
    <member kind="variable">
      <type>int</type>
      <name>Lopti_do_global_copy_prop</name>
      <anchorfile>l__opti_8h.html</anchorfile>
      <anchor>aa61849b001cba768021f57ea30e0289</anchor>
      <arglist></arglist>
    </member>
    <member kind="variable">
      <type>int</type>
      <name>Lopti_do_global_common_sub_elim</name>
      <anchorfile>l__opti_8h.html</anchorfile>
      <anchor>a39001fe2b34d0387773399068f69868</anchor>
      <arglist></arglist>
    </member>
    <member kind="variable">
      <type>int</type>
      <name>Lopti_do_global_mem_copy_prop</name>
      <anchorfile>l__opti_8h.html</anchorfile>
      <anchor>47a778d7c55bb919b56b67964b7982f1</anchor>
      <arglist></arglist>
    </member>
    <member kind="variable">
      <type>int</type>
      <name>Lopti_do_global_red_load_elim</name>
      <anchorfile>l__opti_8h.html</anchorfile>
      <anchor>30ca934a5fa4793b8c29393f6d73e42c</anchor>
      <arglist></arglist>
    </member>
    <member kind="variable">
      <type>int</type>
      <name>Lopti_do_global_red_store_elim</name>
      <anchorfile>l__opti_8h.html</anchorfile>
      <anchor>eee1983d9271b950b9c6597738db726d</anchor>
      <arglist></arglist>
    </member>
    <member kind="variable">
      <type>int</type>
      <name>Lopti_do_global_elim_boolean_ops</name>
      <anchorfile>l__opti_8h.html</anchorfile>
      <anchor>52bf4389d392ec7a28b5a235ff2bdd66</anchor>
      <arglist></arglist>
    </member>
    <member kind="variable">
      <type>int</type>
      <name>Lopti_do_global_branch_val_prop</name>
      <anchorfile>l__opti_8h.html</anchorfile>
      <anchor>793e5499a49b804a8aea772c87c1c4e6</anchor>
      <arglist></arglist>
    </member>
    <member kind="variable">
      <type>int</type>
      <name>Lopti_do_global_dead_if_then_else_rem</name>
      <anchorfile>l__opti_8h.html</anchorfile>
      <anchor>06a44c0e30eb555c845c4265f61c9cdc</anchor>
      <arglist></arglist>
    </member>
    <member kind="variable">
      <type>int</type>
      <name>Lopti_debug_global_opti</name>
      <anchorfile>l__opti_8h.html</anchorfile>
      <anchor>242376cd6f559d2f95f8c3f87c7dc414</anchor>
      <arglist></arglist>
    </member>
    <member kind="variable">
      <type>int</type>
      <name>Lopti_do_memflow_opti</name>
      <anchorfile>l__opti_8h.html</anchorfile>
      <anchor>134a676bfb74c50665fee8e2dd69f7d4</anchor>
      <arglist></arglist>
    </member>
    <member kind="variable">
      <type>int</type>
      <name>Lopti_do_memflow_multistore_load</name>
      <anchorfile>l__opti_8h.html</anchorfile>
      <anchor>197ceca1dc001384529b5a15c8ed7b65</anchor>
      <arglist></arglist>
    </member>
    <member kind="variable">
      <type>int</type>
      <name>Lopti_memflow_bypass_load</name>
      <anchorfile>l__opti_8h.html</anchorfile>
      <anchor>ed7588f0a0cae8c2cf6fc574fbc14976</anchor>
      <arglist></arglist>
    </member>
    <member kind="variable">
      <type>int</type>
      <name>Lopti_memflow_bypass_store</name>
      <anchorfile>l__opti_8h.html</anchorfile>
      <anchor>1020485c9d05b77663fc706cb70dd7ea</anchor>
      <arglist></arglist>
    </member>
    <member kind="variable">
      <type>int</type>
      <name>Lopti_memflow_bypass_jsr</name>
      <anchorfile>l__opti_8h.html</anchorfile>
      <anchor>56407fd544268da0f61340135718ab6b</anchor>
      <arglist></arglist>
    </member>
    <member kind="variable">
      <type>int</type>
      <name>Lopti_memflow_bypass_total</name>
      <anchorfile>l__opti_8h.html</anchorfile>
      <anchor>2e971688aa06c79b74a7e01e49c77d3b</anchor>
      <arglist></arglist>
    </member>
    <member kind="variable">
      <type>int</type>
      <name>Lopti_debug_memflow</name>
      <anchorfile>l__opti_8h.html</anchorfile>
      <anchor>2029c5594bb2d10992047e58ac0460fa</anchor>
      <arglist></arglist>
    </member>
    <member kind="variable">
      <type>int</type>
      <name>Lopti_do_jump_opti</name>
      <anchorfile>l__opti_8h.html</anchorfile>
      <anchor>531df982924f1f3826d76d55b916ab49</anchor>
      <arglist></arglist>
    </member>
    <member kind="variable">
      <type>int</type>
      <name>Lopti_do_jump_dead_block_elim</name>
      <anchorfile>l__opti_8h.html</anchorfile>
      <anchor>4d09370c333d159af706fd23679c4628</anchor>
      <arglist></arglist>
    </member>
    <member kind="variable">
      <type>int</type>
      <name>Lopti_do_jump_br_to_next_block</name>
      <anchorfile>l__opti_8h.html</anchorfile>
      <anchor>2091799c89a1aee713b4fa01f6cd7716</anchor>
      <arglist></arglist>
    </member>
    <member kind="variable">
      <type>int</type>
      <name>Lopti_do_jump_br_to_same_target</name>
      <anchorfile>l__opti_8h.html</anchorfile>
      <anchor>d2928595a0a4144cc2c6cfc398d7e64c</anchor>
      <arglist></arglist>
    </member>
    <member kind="variable">
      <type>int</type>
      <name>Lopti_do_jump_br_to_uncond_br</name>
      <anchorfile>l__opti_8h.html</anchorfile>
      <anchor>5a9bb6e4c0bcf5d8db05ed7624d0d7d3</anchor>
      <arglist></arglist>
    </member>
    <member kind="variable">
      <type>int</type>
      <name>Lopti_do_jump_block_merge</name>
      <anchorfile>l__opti_8h.html</anchorfile>
      <anchor>c14ca34a654634b078cc5c72256cb970</anchor>
      <arglist></arglist>
    </member>
    <member kind="variable">
      <type>int</type>
      <name>Lopti_do_jump_combine_labels</name>
      <anchorfile>l__opti_8h.html</anchorfile>
      <anchor>5a578d07171cc6915efa0b99fb07a8f0</anchor>
      <arglist></arglist>
    </member>
    <member kind="variable">
      <type>int</type>
      <name>Lopti_do_jump_br_target_expansion</name>
      <anchorfile>l__opti_8h.html</anchorfile>
      <anchor>1cf938ac9f8b9e62624d6d40aad1b0df</anchor>
      <arglist></arglist>
    </member>
    <member kind="variable">
      <type>int</type>
      <name>Lopti_do_jump_br_swap</name>
      <anchorfile>l__opti_8h.html</anchorfile>
      <anchor>a74f3a03eeafdd3d6e14737c0ad9f8c4</anchor>
      <arglist></arglist>
    </member>
    <member kind="variable">
      <type>int</type>
      <name>Lopti_allow_jump_expansion_of_pcode_loops</name>
      <anchorfile>l__opti_8h.html</anchorfile>
      <anchor>020db124e350174d36b5397f90db3b8a</anchor>
      <arglist></arglist>
    </member>
    <member kind="variable">
      <type>int</type>
      <name>Lopti_do_remove_decidable_cond_branches</name>
      <anchorfile>l__opti_8h.html</anchorfile>
      <anchor>c460d8f28d8f289b1fd9b2df6c82dc1e</anchor>
      <arglist></arglist>
    </member>
    <member kind="variable">
      <type>int</type>
      <name>Lopti_debug_jump_opti</name>
      <anchorfile>l__opti_8h.html</anchorfile>
      <anchor>b46ebf46d4802b5736e5eaaa2a6f3ee1</anchor>
      <arglist></arglist>
    </member>
    <member kind="variable">
      <type>int</type>
      <name>Lopti_do_split_branches</name>
      <anchorfile>l__opti_8h.html</anchorfile>
      <anchor>47d4b0605b5d1f51a7859077212d333c</anchor>
      <arglist></arglist>
    </member>
    <member kind="variable">
      <type>int</type>
      <name>Lopti_do_split_unification</name>
      <anchorfile>l__opti_8h.html</anchorfile>
      <anchor>a0dadb5efb9f366ad85948d29fe431be</anchor>
      <arglist></arglist>
    </member>
    <member kind="variable">
      <type>int</type>
      <name>Lopti_do_merge_unification</name>
      <anchorfile>l__opti_8h.html</anchorfile>
      <anchor>9eb2a748e20353df882067e88cd68ddd</anchor>
      <arglist></arglist>
    </member>
    <member kind="variable">
      <type>int</type>
      <name>Lopti_do_jrg_expansion</name>
      <anchorfile>l__opti_8h.html</anchorfile>
      <anchor>4d9bbe0d5c15d5cc589281834e2071c5</anchor>
      <arglist></arglist>
    </member>
    <member kind="variable">
      <type>int</type>
      <name>Lopti_do_loop_opti</name>
      <anchorfile>l__opti_8h.html</anchorfile>
      <anchor>089d8dc69a680c39f7efd73a0e795d9b</anchor>
      <arglist></arglist>
    </member>
    <member kind="variable">
      <type>int</type>
      <name>Lopti_do_loop_br_simp</name>
      <anchorfile>l__opti_8h.html</anchorfile>
      <anchor>1ce2adb367119c1882d0ba416b983fcc</anchor>
      <arglist></arglist>
    </member>
    <member kind="variable">
      <type>int</type>
      <name>Lopti_do_loop_inv_code_rem</name>
      <anchorfile>l__opti_8h.html</anchorfile>
      <anchor>625b24bc76728ac485f49ae758ed0952</anchor>
      <arglist></arglist>
    </member>
    <member kind="variable">
      <type>int</type>
      <name>Lopti_do_loop_global_var_mig</name>
      <anchorfile>l__opti_8h.html</anchorfile>
      <anchor>015c73873925f4096e21f611aa4ec59f</anchor>
      <arglist></arglist>
    </member>
    <member kind="variable">
      <type>int</type>
      <name>Lopti_do_loop_ind_var_str_red</name>
      <anchorfile>l__opti_8h.html</anchorfile>
      <anchor>6e77e5fc0ae70f85f3e0928a0cd011f2</anchor>
      <arglist></arglist>
    </member>
    <member kind="variable">
      <type>int</type>
      <name>Lopti_do_loop_ind_var_reinit</name>
      <anchorfile>l__opti_8h.html</anchorfile>
      <anchor>268270120bf5bd588e8ba1d7b48ba091</anchor>
      <arglist></arglist>
    </member>
    <member kind="variable">
      <type>int</type>
      <name>Lopti_do_loop_ind_var_elim</name>
      <anchorfile>l__opti_8h.html</anchorfile>
      <anchor>8ef93ea2eb3acaaf3b24d0df6ff43ac0</anchor>
      <arglist></arglist>
    </member>
    <member kind="variable">
      <type>int</type>
      <name>Lopti_do_dead_loop_rem</name>
      <anchorfile>l__opti_8h.html</anchorfile>
      <anchor>8b1f44b3dc6c05c71be488a8baa4681d</anchor>
      <arglist></arglist>
    </member>
    <member kind="variable">
      <type>int</type>
      <name>Lopti_preserve_loop_var</name>
      <anchorfile>l__opti_8h.html</anchorfile>
      <anchor>b9613b0eb6c236013f39942919f80063</anchor>
      <arglist></arglist>
    </member>
    <member kind="variable">
      <type>int</type>
      <name>Lopti_do_complex_ind_elim</name>
      <anchorfile>l__opti_8h.html</anchorfile>
      <anchor>86474fd45132fbef3d46036fed7e7897</anchor>
      <arglist></arglist>
    </member>
    <member kind="variable">
      <type>int</type>
      <name>Lopti_debug_loop_opti</name>
      <anchorfile>l__opti_8h.html</anchorfile>
      <anchor>7832e63d33b2b7f45c19be2dd128a2d9</anchor>
      <arglist></arglist>
    </member>
    <member kind="variable">
      <type>int</type>
      <name>Lopti_do_longword_loop_opti</name>
      <anchorfile>l__opti_8h.html</anchorfile>
      <anchor>dcdf1f55b373780e5e07a5bab4d10f08</anchor>
      <arglist></arglist>
    </member>
    <member kind="variable">
      <type>int</type>
      <name>Lopti_store_migration_mode</name>
      <anchorfile>l__opti_8h.html</anchorfile>
      <anchor>7878f82b9eaa623ce66ca745e1a2404c</anchor>
      <arglist></arglist>
    </member>
    <member kind="variable">
      <type>int</type>
      <name>Lopti_do_PCE</name>
      <anchorfile>l__opti_8h.html</anchorfile>
      <anchor>fe4e51d9023d243c2e1c31d8d6575fbc</anchor>
      <arglist></arglist>
    </member>
    <member kind="variable">
      <type>int</type>
      <name>Lopti_do_PCE_split_critical_edges</name>
      <anchorfile>l__opti_8h.html</anchorfile>
      <anchor>2c3b9d064c6e75820298af27953a5857</anchor>
      <arglist></arglist>
    </member>
    <member kind="variable">
      <type>int</type>
      <name>Lopti_do_PCE_merge_same_cbs</name>
      <anchorfile>l__opti_8h.html</anchorfile>
      <anchor>1013b89fd1870b0eea8726ffdd78065f</anchor>
      <arglist></arglist>
    </member>
    <member kind="variable">
      <type>int</type>
      <name>Lopti_do_PCE_optimize_memory_ops</name>
      <anchorfile>l__opti_8h.html</anchorfile>
      <anchor>8ec89bd2ff7a5c70f21252e9d9edf1d4</anchor>
      <arglist></arglist>
    </member>
    <member kind="variable">
      <type>int</type>
      <name>Lopti_do_PCE_conservative_memory_opti</name>
      <anchorfile>l__opti_8h.html</anchorfile>
      <anchor>ae8b125df9a1be974c23fc26f1a5402e</anchor>
      <arglist></arglist>
    </member>
    <member kind="variable">
      <type>int</type>
      <name>Lopti_do_PRE</name>
      <anchorfile>l__opti_8h.html</anchorfile>
      <anchor>277f271d95b9bf6c3bfdb872181da36a</anchor>
      <arglist></arglist>
    </member>
    <member kind="variable">
      <type>int</type>
      <name>Lopti_do_PRE_lazy_code_motion</name>
      <anchorfile>l__opti_8h.html</anchorfile>
      <anchor>b0a966586b823d137e945a7cc2699374</anchor>
      <arglist></arglist>
    </member>
    <member kind="variable">
      <type>int</type>
      <name>Lopti_do_PRE_mem_copy_prop</name>
      <anchorfile>l__opti_8h.html</anchorfile>
      <anchor>c3067bb34faeec459cabb8bf75025a03</anchor>
      <arglist></arglist>
    </member>
    <member kind="variable">
      <type>int</type>
      <name>Lopti_do_PRE_merge_loads_diff_types</name>
      <anchorfile>l__opti_8h.html</anchorfile>
      <anchor>2a5196deb17548cd00789f0ea768d0a8</anchor>
      <arglist></arglist>
    </member>
    <member kind="variable">
      <type>int</type>
      <name>Lopti_do_PRE_cutset_metric</name>
      <anchorfile>l__opti_8h.html</anchorfile>
      <anchor>7f1a8514d0194590064f3652019cea2d</anchor>
      <arglist></arglist>
    </member>
    <member kind="variable">
      <type>int</type>
      <name>Lopti_do_PRE_optimize_moves_of_numerical_constants</name>
      <anchorfile>l__opti_8h.html</anchorfile>
      <anchor>ae5535bb2adfcef99fd47ca2769f688f</anchor>
      <arglist></arglist>
    </member>
    <member kind="variable">
      <type>int</type>
      <name>Lopti_do_PRE_optimize_moves</name>
      <anchorfile>l__opti_8h.html</anchorfile>
      <anchor>f1d28513396bb4f0ccf8a61ba3ce04ef</anchor>
      <arglist></arglist>
    </member>
    <member kind="variable">
      <type>int</type>
      <name>Lopti_do_PRE_optimize_single_source_ops</name>
      <anchorfile>l__opti_8h.html</anchorfile>
      <anchor>bcca784c5de65c67d571d3a264ac7d81</anchor>
      <arglist></arglist>
    </member>
    <member kind="variable">
      <type>int</type>
      <name>Lopti_do_PRE_speculative_code_motion</name>
      <anchorfile>l__opti_8h.html</anchorfile>
      <anchor>4fc5e5f9579403732754ca1a040074f7</anchor>
      <arglist></arglist>
    </member>
    <member kind="variable">
      <type>int</type>
      <name>Lopti_do_global_mem_expression_copy_prop</name>
      <anchorfile>l__opti_8h.html</anchorfile>
      <anchor>8523ea54a17ff479811bf931d5bf8c3a</anchor>
      <arglist></arglist>
    </member>
    <member kind="variable">
      <type>int</type>
      <name>Lopti_do_PDE</name>
      <anchorfile>l__opti_8h.html</anchorfile>
      <anchor>6ad9aa74dc755fae5ee9708852ebfc33</anchor>
      <arglist></arglist>
    </member>
    <member kind="variable">
      <type>int</type>
      <name>Lopti_do_PDE_cutset_metric</name>
      <anchorfile>l__opti_8h.html</anchorfile>
      <anchor>d3e3477a2b2eb1d2ff026ff57b98fcb9</anchor>
      <arglist></arglist>
    </member>
    <member kind="variable">
      <type>int</type>
      <name>Lopti_do_PDE_min_cut</name>
      <anchorfile>l__opti_8h.html</anchorfile>
      <anchor>e4b18668df73df7ac27fcc0fe912c61b</anchor>
      <arglist></arglist>
    </member>
    <member kind="variable">
      <type>int</type>
      <name>Lopti_do_PDE_predicated</name>
      <anchorfile>l__opti_8h.html</anchorfile>
      <anchor>1b129deae2ea43566e529fe8603d763b</anchor>
      <arglist></arglist>
    </member>
    <member kind="variable">
      <type>int</type>
      <name>Lopti_do_PDE_sink_stores</name>
      <anchorfile>l__opti_8h.html</anchorfile>
      <anchor>09958c1780d13eb1b644120e25d33bed</anchor>
      <arglist></arglist>
    </member>
    <member kind="variable">
      <type>int</type>
      <name>Lopti_do_PDE_sink_only_stores</name>
      <anchorfile>l__opti_8h.html</anchorfile>
      <anchor>427787974c27b187ffb8402317ea5e6f</anchor>
      <arglist></arglist>
    </member>
    <member kind="variable">
      <type>int</type>
      <name>Lopti_do_global_dead_store_removal</name>
      <anchorfile>l__opti_8h.html</anchorfile>
      <anchor>6b4fdbd536b408404869741eefb2d9ee</anchor>
      <arglist></arglist>
    </member>
    <member kind="variable">
      <type>int</type>
      <name>Lopti_do_dead_local_var_store_removal</name>
      <anchorfile>l__opti_8h.html</anchorfile>
      <anchor>43e3212be05265309d78781d49e289b7</anchor>
      <arglist></arglist>
    </member>
    <member kind="variable">
      <type>int</type>
      <name>Lopti_only_lvl1_for_zero_weight_fn</name>
      <anchorfile>l__opti_8h.html</anchorfile>
      <anchor>61d4df5f9daa29e72b1b5e65a93370f6</anchor>
      <arglist></arglist>
    </member>
    <member kind="variable">
      <type>int</type>
      <name>Lopti_do_post_inc_conv</name>
      <anchorfile>l__opti_8h.html</anchorfile>
      <anchor>8f8bb28200aabfc716915ffd62bbbe69</anchor>
      <arglist></arglist>
    </member>
    <member kind="variable">
      <type>int</type>
      <name>Lopti_do_mark_memory_labels</name>
      <anchorfile>l__opti_8h.html</anchorfile>
      <anchor>4467a3f6a41c843ce83ed639cc366d97</anchor>
      <arglist></arglist>
    </member>
    <member kind="variable">
      <type>int</type>
      <name>Lopti_do_mark_incoming_parms</name>
      <anchorfile>l__opti_8h.html</anchorfile>
      <anchor>a8f6a948f910c94e7faadfaa7ca01101</anchor>
      <arglist></arglist>
    </member>
    <member kind="variable">
      <type>int</type>
      <name>Lopti_do_mark_trivial_sef_jsrs</name>
      <anchorfile>l__opti_8h.html</anchorfile>
      <anchor>851d2ad4b326c8a7b561d1bdefd330d1</anchor>
      <arglist></arglist>
    </member>
    <member kind="variable">
      <type>int</type>
      <name>Lopti_do_mark_sync_jsrs</name>
      <anchorfile>l__opti_8h.html</anchorfile>
      <anchor>90d1e97994303430b1e59a64003adff9</anchor>
      <arglist></arglist>
    </member>
    <member kind="variable">
      <type>int</type>
      <name>Lopti_do_mark_trivial_safe_ops</name>
      <anchorfile>l__opti_8h.html</anchorfile>
      <anchor>0efbb3fcaf198a4b835438a04b329da7</anchor>
      <arglist></arglist>
    </member>
    <member kind="variable">
      <type>int</type>
      <name>Lopti_do_code_layout</name>
      <anchorfile>l__opti_8h.html</anchorfile>
      <anchor>59fa9dbcf7a82ace2e3affa0c393de5b</anchor>
      <arglist></arglist>
    </member>
    <member kind="variable">
      <type>int</type>
      <name>Lopti_do_classify_branches</name>
      <anchorfile>l__opti_8h.html</anchorfile>
      <anchor>620bfa0179d368afb575233b175a098e</anchor>
      <arglist></arglist>
    </member>
    <member kind="variable">
      <type>int</type>
      <name>Lopti_print_opti_count</name>
      <anchorfile>l__opti_8h.html</anchorfile>
      <anchor>3acc753376ac648863c45ffc6d5c63fb</anchor>
      <arglist></arglist>
    </member>
    <member kind="variable">
      <type>int</type>
      <name>Lopti_print_opti_breakdown</name>
      <anchorfile>l__opti_8h.html</anchorfile>
      <anchor>3532b96f82647148be3cc6c9e799d93e</anchor>
      <arglist></arglist>
    </member>
    <member kind="variable">
      <type>int</type>
      <name>L_native_machine_ctype</name>
      <anchorfile>l__opti_8h.html</anchorfile>
      <anchor>370445c969e7255431a032fd14d0cfe5</anchor>
      <arglist></arglist>
    </member>
    <member kind="variable">
      <type>L_Alloc_Pool *</type>
      <name>L_alloc_danger_ext</name>
      <anchorfile>l__opti_8h.html</anchorfile>
      <anchor>cc6c274b5e4f63b0b0ceaf53b0be780e</anchor>
      <arglist></arglist>
    </member>
    <member kind="variable">
      <type>int *</type>
      <name>Lopti_ctype_array</name>
      <anchorfile>l__opti_8h.html</anchorfile>
      <anchor>a388f4015f2d1c31fe4f4c0be7e523d2</anchor>
      <arglist></arglist>
    </member>
    <member kind="variable">
      <type>int</type>
      <name>Lopti_ctype_array_size</name>
      <anchorfile>l__opti_8h.html</anchorfile>
      <anchor>33e0aa4c3bec083f608cf4aebeb80822</anchor>
      <arglist></arglist>
    </member>
    <member kind="variable">
      <type>int</type>
      <name>Lopti_ctype_max_reg</name>
      <anchorfile>l__opti_8h.html</anchorfile>
      <anchor>bc627a3c9a90bf8f16a190f90f1665a4</anchor>
      <arglist></arglist>
    </member>
    <member kind="variable">
      <type>int</type>
      <name>Lopti_cnt_local_opti</name>
      <anchorfile>l__opti_8h.html</anchorfile>
      <anchor>f78fd58926406ffd1d8521ce2d25cc92</anchor>
      <arglist></arglist>
    </member>
    <member kind="variable">
      <type>int</type>
      <name>Lopti_cnt_local_constant_prop</name>
      <anchorfile>l__opti_8h.html</anchorfile>
      <anchor>5aca3bfee0de8dae097d77d01941b618</anchor>
      <arglist></arglist>
    </member>
    <member kind="variable">
      <type>int</type>
      <name>Lopti_cnt_local_copy_prop</name>
      <anchorfile>l__opti_8h.html</anchorfile>
      <anchor>5c98b89c4f6568611d89cf4f9b2aa2b9</anchor>
      <arglist></arglist>
    </member>
    <member kind="variable">
      <type>int</type>
      <name>Lopti_cnt_local_rev_copy_prop</name>
      <anchorfile>l__opti_8h.html</anchorfile>
      <anchor>6fe06f00c602c87a1b8853fdb740b2ea</anchor>
      <arglist></arglist>
    </member>
    <member kind="variable">
      <type>int</type>
      <name>Lopti_cnt_local_mem_copy_prop</name>
      <anchorfile>l__opti_8h.html</anchorfile>
      <anchor>f78ee891827d2c4395cf4fd70e9298ac</anchor>
      <arglist></arglist>
    </member>
    <member kind="variable">
      <type>int</type>
      <name>Lopti_cnt_local_common_sub_elim</name>
      <anchorfile>l__opti_8h.html</anchorfile>
      <anchor>d30a316c82cb4d99d070713c20c76cdb</anchor>
      <arglist></arglist>
    </member>
    <member kind="variable">
      <type>int</type>
      <name>Lopti_cnt_local_red_load_elim</name>
      <anchorfile>l__opti_8h.html</anchorfile>
      <anchor>59de8e8cc71c5dc9d495be678e502ece</anchor>
      <arglist></arglist>
    </member>
    <member kind="variable">
      <type>int</type>
      <name>Lopti_cnt_local_red_store_elim</name>
      <anchorfile>l__opti_8h.html</anchorfile>
      <anchor>15be1073f55f1aa9cdfb1e81f57044b1</anchor>
      <arglist></arglist>
    </member>
    <member kind="variable">
      <type>int</type>
      <name>Lopti_cnt_local_constant_fold</name>
      <anchorfile>l__opti_8h.html</anchorfile>
      <anchor>17b600d1763b9bedd53311de308aab52</anchor>
      <arglist></arglist>
    </member>
    <member kind="variable">
      <type>int</type>
      <name>Lopti_cnt_local_strength_red</name>
      <anchorfile>l__opti_8h.html</anchorfile>
      <anchor>60b336a1fb70d652314036cee9ba781b</anchor>
      <arglist></arglist>
    </member>
    <member kind="variable">
      <type>int</type>
      <name>Lopti_cnt_local_constant_comb</name>
      <anchorfile>l__opti_8h.html</anchorfile>
      <anchor>6d7f2d2acb7fd344a5d26d94a77d1bfb</anchor>
      <arglist></arglist>
    </member>
    <member kind="variable">
      <type>int</type>
      <name>Lopti_cnt_local_operation_fold</name>
      <anchorfile>l__opti_8h.html</anchorfile>
      <anchor>c948f4cbf3efc503222f24bcc8a8fd93</anchor>
      <arglist></arglist>
    </member>
    <member kind="variable">
      <type>int</type>
      <name>Lopti_cnt_local_branch_fold</name>
      <anchorfile>l__opti_8h.html</anchorfile>
      <anchor>0ac99815a2aaea80c22e2d149a510121</anchor>
      <arglist></arglist>
    </member>
    <member kind="variable">
      <type>int</type>
      <name>Lopti_cnt_local_operation_cancel</name>
      <anchorfile>l__opti_8h.html</anchorfile>
      <anchor>8c84c3c7235c2dbdb5dce63ad1f1a712</anchor>
      <arglist></arglist>
    </member>
    <member kind="variable">
      <type>int</type>
      <name>Lopti_cnt_local_dead_code_rem</name>
      <anchorfile>l__opti_8h.html</anchorfile>
      <anchor>3fa298047ca082c9db37718ac1e0444d</anchor>
      <arglist></arglist>
    </member>
    <member kind="variable">
      <type>int</type>
      <name>Lopti_cnt_local_code_motion</name>
      <anchorfile>l__opti_8h.html</anchorfile>
      <anchor>5d95e9ed875f732a910cdccc65d57a85</anchor>
      <arglist></arglist>
    </member>
    <member kind="variable">
      <type>int</type>
      <name>Lopti_cnt_local_remove_sign_ext</name>
      <anchorfile>l__opti_8h.html</anchorfile>
      <anchor>593af5042b5662d6c278eebe4646493a</anchor>
      <arglist></arglist>
    </member>
    <member kind="variable">
      <type>int</type>
      <name>Lopti_cnt_local_reduce_logic</name>
      <anchorfile>l__opti_8h.html</anchorfile>
      <anchor>2b58cf2a4f2233991eecf178ac64e3e3</anchor>
      <arglist></arglist>
    </member>
    <member kind="variable">
      <type>int</type>
      <name>Lopti_cnt_local_register_rename</name>
      <anchorfile>l__opti_8h.html</anchorfile>
      <anchor>13832534d2e6012014e36bdd089d5535</anchor>
      <arglist></arglist>
    </member>
    <member kind="variable">
      <type>int</type>
      <name>Lopti_cnt_global_opti</name>
      <anchorfile>l__opti_8h.html</anchorfile>
      <anchor>f9ff5dabd7f0ef90058d2cdac5c0aba1</anchor>
      <arglist></arglist>
    </member>
    <member kind="variable">
      <type>int</type>
      <name>Lopti_cnt_global_dead_code_rem</name>
      <anchorfile>l__opti_8h.html</anchorfile>
      <anchor>0d45f69f2768e2014dfcf77e97840005</anchor>
      <arglist></arglist>
    </member>
    <member kind="variable">
      <type>int</type>
      <name>Lopti_cnt_global_constant_prop</name>
      <anchorfile>l__opti_8h.html</anchorfile>
      <anchor>8621757a8fdc40bea064cbc4cc95a0f8</anchor>
      <arglist></arglist>
    </member>
    <member kind="variable">
      <type>int</type>
      <name>Lopti_cnt_global_copy_prop</name>
      <anchorfile>l__opti_8h.html</anchorfile>
      <anchor>a7b11a41270f176603986ce6044b75b5</anchor>
      <arglist></arglist>
    </member>
    <member kind="variable">
      <type>int</type>
      <name>Lopti_cnt_global_mem_copy_prop</name>
      <anchorfile>l__opti_8h.html</anchorfile>
      <anchor>1c93aea77aba62f587a3d53613f6b6f8</anchor>
      <arglist></arglist>
    </member>
    <member kind="variable">
      <type>int</type>
      <name>Lopti_cnt_global_common_sub_elim</name>
      <anchorfile>l__opti_8h.html</anchorfile>
      <anchor>055e8cc717bd60b3dbcb77a4c2cac817</anchor>
      <arglist></arglist>
    </member>
    <member kind="variable">
      <type>int</type>
      <name>Lopti_cnt_global_red_load_elim</name>
      <anchorfile>l__opti_8h.html</anchorfile>
      <anchor>fe1252352ceaef605d21ae21268e0f49</anchor>
      <arglist></arglist>
    </member>
    <member kind="variable">
      <type>int</type>
      <name>Lopti_cnt_global_red_store_elim</name>
      <anchorfile>l__opti_8h.html</anchorfile>
      <anchor>fa6c19336f7b640c6ea91ccbf3fed5cf</anchor>
      <arglist></arglist>
    </member>
    <member kind="variable">
      <type>int</type>
      <name>Lopti_cnt_global_elim_boolean_ops</name>
      <anchorfile>l__opti_8h.html</anchorfile>
      <anchor>c9209b77ce2ae36391542e497297a6e3</anchor>
      <arglist></arglist>
    </member>
    <member kind="variable">
      <type>int</type>
      <name>Lopti_cnt_global_dead_if_then_else_rem</name>
      <anchorfile>l__opti_8h.html</anchorfile>
      <anchor>d10d5deb47779848cf437ef157eb1eca</anchor>
      <arglist></arglist>
    </member>
    <member kind="variable">
      <type>int</type>
      <name>Lopti_cnt_memflow_multistore_load</name>
      <anchorfile>l__opti_8h.html</anchorfile>
      <anchor>c2d2d6dd5de6b978bbab77ccb9fe0cd0</anchor>
      <arglist></arglist>
    </member>
    <member kind="variable">
      <type>int</type>
      <name>Lopti_cnt_jump_opti</name>
      <anchorfile>l__opti_8h.html</anchorfile>
      <anchor>8cb94e323fe13961a49048adb755c43b</anchor>
      <arglist></arglist>
    </member>
    <member kind="variable">
      <type>int</type>
      <name>Lopti_cnt_jump_dead_block_elim</name>
      <anchorfile>l__opti_8h.html</anchorfile>
      <anchor>5320444d1e95c1cadc3272874550225a</anchor>
      <arglist></arglist>
    </member>
    <member kind="variable">
      <type>int</type>
      <name>Lopti_cnt_jump_br_to_next_block</name>
      <anchorfile>l__opti_8h.html</anchorfile>
      <anchor>1c1796e7b3c5056ca6c3795b668c30e2</anchor>
      <arglist></arglist>
    </member>
    <member kind="variable">
      <type>int</type>
      <name>Lopti_cnt_jump_br_to_same_target</name>
      <anchorfile>l__opti_8h.html</anchorfile>
      <anchor>53f5ca958523321945c149a529001c87</anchor>
      <arglist></arglist>
    </member>
    <member kind="variable">
      <type>int</type>
      <name>Lopti_cnt_jump_br_to_uncond_br</name>
      <anchorfile>l__opti_8h.html</anchorfile>
      <anchor>ad4703e6f398d3b04f5e95e4e50742a6</anchor>
      <arglist></arglist>
    </member>
    <member kind="variable">
      <type>int</type>
      <name>Lopti_cnt_jump_block_merge</name>
      <anchorfile>l__opti_8h.html</anchorfile>
      <anchor>c130eb576413b392adb8bb59c47aac5e</anchor>
      <arglist></arglist>
    </member>
    <member kind="variable">
      <type>int</type>
      <name>Lopti_cnt_jump_combine_labels</name>
      <anchorfile>l__opti_8h.html</anchorfile>
      <anchor>6cf2ad1a28b50dcacfbbabe53c342454</anchor>
      <arglist></arglist>
    </member>
    <member kind="variable">
      <type>int</type>
      <name>Lopti_cnt_jump_br_target_expansion</name>
      <anchorfile>l__opti_8h.html</anchorfile>
      <anchor>a262e831d3836b068d853bdc5bde5d30</anchor>
      <arglist></arglist>
    </member>
    <member kind="variable">
      <type>int</type>
      <name>Lopti_cnt_jump_br_swap</name>
      <anchorfile>l__opti_8h.html</anchorfile>
      <anchor>e6e4bf4d5bcd33b6eeafcec52a3c90c6</anchor>
      <arglist></arglist>
    </member>
    <member kind="variable">
      <type>int</type>
      <name>Lopti_cnt_loop_opti</name>
      <anchorfile>l__opti_8h.html</anchorfile>
      <anchor>89cdfb3508b0a365cae6018120cd130e</anchor>
      <arglist></arglist>
    </member>
    <member kind="variable">
      <type>int</type>
      <name>Lopti_cnt_loop_br_simp</name>
      <anchorfile>l__opti_8h.html</anchorfile>
      <anchor>8438a83b7217665243ad7707dc1e3a4c</anchor>
      <arglist></arglist>
    </member>
    <member kind="variable">
      <type>int</type>
      <name>Lopti_cnt_loop_inv_code_rem</name>
      <anchorfile>l__opti_8h.html</anchorfile>
      <anchor>afbec35e49d6645912981e63b8f75f73</anchor>
      <arglist></arglist>
    </member>
    <member kind="variable">
      <type>int</type>
      <name>Lopti_cnt_loop_global_var_mig</name>
      <anchorfile>l__opti_8h.html</anchorfile>
      <anchor>9f0ab2a9bd24e6e60dbf97ff5c32da6a</anchor>
      <arglist></arglist>
    </member>
    <member kind="variable">
      <type>int</type>
      <name>Lopti_cnt_loop_ind_var_str_red</name>
      <anchorfile>l__opti_8h.html</anchorfile>
      <anchor>f3e73e64c2e421b982815f59765afd9f</anchor>
      <arglist></arglist>
    </member>
    <member kind="variable">
      <type>int</type>
      <name>Lopti_cnt_loop_ind_var_reinit</name>
      <anchorfile>l__opti_8h.html</anchorfile>
      <anchor>71f1293ecb44fd991678849ecccc97f3</anchor>
      <arglist></arglist>
    </member>
    <member kind="variable">
      <type>int</type>
      <name>Lopti_cnt_loop_ind_var_elim</name>
      <anchorfile>l__opti_8h.html</anchorfile>
      <anchor>b9c1423c30aac4448c045e5e018d7331</anchor>
      <arglist></arglist>
    </member>
    <member kind="variable">
      <type>int</type>
      <name>Lopti_cnt_dead_loop_rem</name>
      <anchorfile>l__opti_8h.html</anchorfile>
      <anchor>959cdcafbd890248663fd15252dc8d9d</anchor>
      <arglist></arglist>
    </member>
    <member kind="variable">
      <type>int</type>
      <name>Lopti_inter_region_global_opti</name>
      <anchorfile>l__opti_8h.html</anchorfile>
      <anchor>235264fca0bdf817175864fdc621fda4</anchor>
      <arglist></arglist>
    </member>
    <member kind="variable">
      <type>int</type>
      <name>Lopti_inter_region_global_dead_code_rem</name>
      <anchorfile>l__opti_8h.html</anchorfile>
      <anchor>4272b446f4e75548ac96fdaf005b4185</anchor>
      <arglist></arglist>
    </member>
    <member kind="variable">
      <type>int</type>
      <name>Lopti_inter_region_global_constant_prop</name>
      <anchorfile>l__opti_8h.html</anchorfile>
      <anchor>e092aa5228b67dd83dcf76407ea9ff14</anchor>
      <arglist></arglist>
    </member>
    <member kind="variable">
      <type>int</type>
      <name>Lopti_inter_region_global_copy_prop</name>
      <anchorfile>l__opti_8h.html</anchorfile>
      <anchor>2aad0d4fae1ea18ec8cf91a8d9200f3e</anchor>
      <arglist></arglist>
    </member>
    <member kind="variable">
      <type>int</type>
      <name>Lopti_inter_region_global_mem_copy_prop</name>
      <anchorfile>l__opti_8h.html</anchorfile>
      <anchor>75e868145a3428ce1b19e71f3669f80e</anchor>
      <arglist></arglist>
    </member>
    <member kind="variable">
      <type>int</type>
      <name>Lopti_inter_region_global_common_sub_elim</name>
      <anchorfile>l__opti_8h.html</anchorfile>
      <anchor>4fc422beb94b60df03e1d5c64d769286</anchor>
      <arglist></arglist>
    </member>
    <member kind="variable">
      <type>int</type>
      <name>Lopti_inter_region_global_red_load_elim</name>
      <anchorfile>l__opti_8h.html</anchorfile>
      <anchor>39f315882dfceecd7c7c14c894781e07</anchor>
      <arglist></arglist>
    </member>
    <member kind="variable">
      <type>int</type>
      <name>Lopti_inter_region_global_red_store_elim</name>
      <anchorfile>l__opti_8h.html</anchorfile>
      <anchor>70f6cece59884ca2975ef076e6ee2f17</anchor>
      <arglist></arglist>
    </member>
    <member kind="variable">
      <type>int</type>
      <name>Lopti_inter_region_global_elim_boolean_ops</name>
      <anchorfile>l__opti_8h.html</anchorfile>
      <anchor>4a0703bf7600acbc259401035b927aa2</anchor>
      <arglist></arglist>
    </member>
    <member kind="variable">
      <type>int</type>
      <name>Lopti_inter_region_global_dead_if_then_else_rem</name>
      <anchorfile>l__opti_8h.html</anchorfile>
      <anchor>3f62d4a0b08edac2317647364a7a4f13</anchor>
      <arglist></arglist>
    </member>
    <member kind="variable">
      <type>double</type>
      <name>Lopti_inter_region_global_common_sub_elim_wgt</name>
      <anchorfile>l__opti_8h.html</anchorfile>
      <anchor>06763a4f290a170bfae6cbd1d112acce</anchor>
      <arglist></arglist>
    </member>
    <member kind="variable">
      <type>double</type>
      <name>Lopti_inter_region_global_copy_prop_wgt</name>
      <anchorfile>l__opti_8h.html</anchorfile>
      <anchor>b46aa21992eeda81af82f6e63af2f866</anchor>
      <arglist></arglist>
    </member>
    <member kind="variable">
      <type>double</type>
      <name>Lopti_inter_region_loop_inv_wgt</name>
      <anchorfile>l__opti_8h.html</anchorfile>
      <anchor>be1b2d240545f9250220f056adf52f95</anchor>
      <arglist></arglist>
    </member>
    <member kind="variable">
      <type>double</type>
      <name>Lopti_inter_region_gvm_wgt</name>
      <anchorfile>l__opti_8h.html</anchorfile>
      <anchor>509f9f7052b0bba49b66f043c5d4c47b</anchor>
      <arglist></arglist>
    </member>
    <member kind="variable">
      <type>int</type>
      <name>Lopti_pred_promotion_level</name>
      <anchorfile>l__opti_8h.html</anchorfile>
      <anchor>5b57300fa5693338e2016d38512b3fa5</anchor>
      <arglist></arglist>
    </member>
    <member kind="variable">
      <type>int</type>
      <name>Lopti_remove_red_guards</name>
      <anchorfile>l__opti_8h.html</anchorfile>
      <anchor>13d0af114a679934de5e38f21d1d75a6</anchor>
      <arglist></arglist>
    </member>
  </compound>
  <compound kind="file">
    <name>l_opti_functions.c</name>
    <path>/mnt/hgfs/mkt/Desktop/openimpact/src/Lcode/opti/Lopti/</path>
    <filename>l__opti__functions_8c</filename>
    <includes id="l__opti_8h" name="l_opti.h" local="yes" imported="no">l_opti.h</includes>
    <member kind="define">
      <type>#define</type>
      <name>ERR</name>
      <anchorfile>l__opti__functions_8c.html</anchorfile>
      <anchor>735563036dced0b7d6cc98f97ea4978b</anchor>
      <arglist></arglist>
    </member>
    <member kind="function">
      <type>void</type>
      <name>L_create_divide_operations</name>
      <anchorfile>l__opti__functions_8c.html</anchorfile>
      <anchor>52029dd001a2c3e3ed950f1406082fcb</anchor>
      <arglist>(L_Oper *div_oper, L_Cb *cb)</arglist>
    </member>
    <member kind="function">
      <type>void</type>
      <name>L_create_rem_operations</name>
      <anchorfile>l__opti__functions_8c.html</anchorfile>
      <anchor>dff8ac039b3ecffe07700183c587017c</anchor>
      <arglist>(L_Oper *rem_oper, L_Cb *cb)</arglist>
    </member>
    <member kind="function">
      <type>L_Oper *</type>
      <name>L_create_move</name>
      <anchorfile>l__opti__functions_8c.html</anchorfile>
      <anchor>efb698f19fb7851cd2d49fadc5804c6b</anchor>
      <arglist>(L_Operand *dest, L_Operand *src)</arglist>
    </member>
    <member kind="function">
      <type>L_Oper *</type>
      <name>L_create_move_using</name>
      <anchorfile>l__opti__functions_8c.html</anchorfile>
      <anchor>7824370b3b80e17ccde25ec3ea394e56</anchor>
      <arglist>(L_Operand *dest, L_Operand *src, L_Oper *oper)</arglist>
    </member>
    <member kind="function">
      <type>void</type>
      <name>L_convert_to_zero_extend_oper</name>
      <anchorfile>l__opti__functions_8c.html</anchorfile>
      <anchor>69a3d03f90b6738c687b4258ec047d7b</anchor>
      <arglist>(L_Oper *oper, L_Operand *dest, L_Operand *src)</arglist>
    </member>
    <member kind="function">
      <type>void</type>
      <name>L_convert_to_extended_move</name>
      <anchorfile>l__opti__functions_8c.html</anchorfile>
      <anchor>c386348213abaa802fd52bec644381be</anchor>
      <arglist>(L_Oper *oper, L_Operand *dest, L_Operand *src)</arglist>
    </member>
    <member kind="function">
      <type>void</type>
      <name>L_convert_to_move</name>
      <anchorfile>l__opti__functions_8c.html</anchorfile>
      <anchor>8eae4d01044de655c18f1c5eff1c0deb</anchor>
      <arglist>(L_Oper *oper, L_Operand *dest, L_Operand *src)</arglist>
    </member>
    <member kind="function">
      <type>void</type>
      <name>L_convert_to_extract</name>
      <anchorfile>l__opti__functions_8c.html</anchorfile>
      <anchor>116ff5b7bb490bce7f57cda768aa2ce1</anchor>
      <arglist>(L_Oper *op, L_Operand *dest, L_Operand *src, int of)</arglist>
    </member>
    <member kind="function">
      <type>void</type>
      <name>L_convert_to_move_of_zero</name>
      <anchorfile>l__opti__functions_8c.html</anchorfile>
      <anchor>45bf1b16fcb83a42047b28ee55639617</anchor>
      <arglist>(L_Oper *oper, L_Operand *dest)</arglist>
    </member>
    <member kind="function">
      <type>void</type>
      <name>L_convert_to_move_of_one</name>
      <anchorfile>l__opti__functions_8c.html</anchorfile>
      <anchor>fc23a486beb880c5774585219417970f</anchor>
      <arglist>(L_Oper *oper, L_Operand *dest)</arglist>
    </member>
    <member kind="function">
      <type>void</type>
      <name>L_convert_to_jump</name>
      <anchorfile>l__opti__functions_8c.html</anchorfile>
      <anchor>d581802b236034bfa7865810e86baad8</anchor>
      <arglist>(L_Oper *oper, L_Operand *target)</arglist>
    </member>
    <member kind="function">
      <type>void</type>
      <name>L_fix_cond_br</name>
      <anchorfile>l__opti__functions_8c.html</anchorfile>
      <anchor>8c9bbbcb684a76b7774848f938ffde50</anchor>
      <arglist>(L_Cb *cb, L_Oper *oper, int cc)</arglist>
    </member>
    <member kind="function">
      <type>int</type>
      <name>L_move_from_ctype</name>
      <anchorfile>l__opti__functions_8c.html</anchorfile>
      <anchor>21280157c6eddd4442f28f86e2d09368</anchor>
      <arglist>(int ctype)</arglist>
    </member>
    <member kind="function">
      <type>int</type>
      <name>L_corresponding_load</name>
      <anchorfile>l__opti__functions_8c.html</anchorfile>
      <anchor>c375f2fc6e03d3c593999325bb6b1c5d</anchor>
      <arglist>(L_Oper *oper)</arglist>
    </member>
    <member kind="function">
      <type>int</type>
      <name>L_corresponding_preincrement_load</name>
      <anchorfile>l__opti__functions_8c.html</anchorfile>
      <anchor>28ae997fde0789a2d26da40bc4557a62</anchor>
      <arglist>(L_Oper *oper)</arglist>
    </member>
    <member kind="function">
      <type>int</type>
      <name>L_corresponding_postincrement_load</name>
      <anchorfile>l__opti__functions_8c.html</anchorfile>
      <anchor>ba725ef3d904f61d0b2d9be5855fa21c</anchor>
      <arglist>(L_Oper *oper)</arglist>
    </member>
    <member kind="function">
      <type>int</type>
      <name>L_corresponding_store</name>
      <anchorfile>l__opti__functions_8c.html</anchorfile>
      <anchor>60082678271b6bb241f07adc117b4253</anchor>
      <arglist>(L_Oper *oper)</arglist>
    </member>
    <member kind="function">
      <type>int</type>
      <name>L_corresponding_preincrement_store</name>
      <anchorfile>l__opti__functions_8c.html</anchorfile>
      <anchor>0f1ce6b939c934a47911d8f567dbef94</anchor>
      <arglist>(L_Oper *oper)</arglist>
    </member>
    <member kind="function">
      <type>int</type>
      <name>L_corresponding_postincrement_store</name>
      <anchorfile>l__opti__functions_8c.html</anchorfile>
      <anchor>3482d4c20455139f522ab003b2a68b44</anchor>
      <arglist>(L_Oper *oper)</arglist>
    </member>
    <member kind="function">
      <type>int</type>
      <name>L_corresponding_mov</name>
      <anchorfile>l__opti__functions_8c.html</anchorfile>
      <anchor>9f402a238c72a5a67f2e36c3ee0ebdf6</anchor>
      <arglist>(L_Oper *oper)</arglist>
    </member>
    <member kind="function">
      <type>int</type>
      <name>L_corresponding_add</name>
      <anchorfile>l__opti__functions_8c.html</anchorfile>
      <anchor>d9beccc83450fac241c17ce02b015f35</anchor>
      <arglist>(L_Oper *oper)</arglist>
    </member>
    <member kind="function">
      <type>int</type>
      <name>L_corresponding_mul_add</name>
      <anchorfile>l__opti__functions_8c.html</anchorfile>
      <anchor>b76ee5452e9ab406c93e3e93bb27ff60</anchor>
      <arglist>(L_Oper *oper)</arglist>
    </member>
    <member kind="function">
      <type>int</type>
      <name>L_corresponding_mul_sub</name>
      <anchorfile>l__opti__functions_8c.html</anchorfile>
      <anchor>4f78aa9e17df9253b4c87c01499c6f47</anchor>
      <arglist>(L_Oper *oper)</arglist>
    </member>
    <member kind="function">
      <type>int</type>
      <name>L_corresponding_mul_sub_rev</name>
      <anchorfile>l__opti__functions_8c.html</anchorfile>
      <anchor>325812595960103c1b1ef0d6eba6050b</anchor>
      <arglist>(L_Oper *oper)</arglist>
    </member>
    <member kind="function">
      <type>int</type>
      <name>L_inverse_arithmetic</name>
      <anchorfile>l__opti__functions_8c.html</anchorfile>
      <anchor>463f22fc4acc3c31f960ffa5d7a71ded</anchor>
      <arglist>(int opc)</arglist>
    </member>
    <member kind="function">
      <type>int</type>
      <name>L_has_const_operand_and_realign_oper</name>
      <anchorfile>l__opti__functions_8c.html</anchorfile>
      <anchor>036c43ecc670f83fc0b732b32ad802ac</anchor>
      <arglist>(L_Oper *oper)</arglist>
    </member>
    <member kind="function">
      <type>void</type>
      <name>L_undo_and_combine</name>
      <anchorfile>l__opti__functions_8c.html</anchorfile>
      <anchor>e3221d7fbfaecd90664b426c213520bd</anchor>
      <arglist>(L_Oper *opB, L_Oper *opC)</arglist>
    </member>
    <member kind="function">
      <type>int</type>
      <name>L_combine_operations</name>
      <anchorfile>l__opti__functions_8c.html</anchorfile>
      <anchor>03063215a036c161d1ab05faabb761bc</anchor>
      <arglist>(L_Oper *opA, L_Oper *opB)</arglist>
    </member>
    <member kind="function">
      <type>void</type>
      <name>L_undo_increment</name>
      <anchorfile>l__opti__functions_8c.html</anchorfile>
      <anchor>1ce6f0b74781d4cac545b555c531551e</anchor>
      <arglist>(L_Oper *opA, L_Oper *opB)</arglist>
    </member>
    <member kind="function">
      <type>void</type>
      <name>L_combine_increment_operations</name>
      <anchorfile>l__opti__functions_8c.html</anchorfile>
      <anchor>d330b45fbe277683cf864137d1b91859</anchor>
      <arglist>(L_Oper *opA, L_Oper *opB)</arglist>
    </member>
    <member kind="function">
      <type>void</type>
      <name>L_move_oper_before</name>
      <anchorfile>l__opti__functions_8c.html</anchorfile>
      <anchor>c96b3eb8c7c63180fab2d9ea335c6835</anchor>
      <arglist>(L_Cb *cb, L_Oper *move_op, L_Oper *before_op)</arglist>
    </member>
    <member kind="function">
      <type>void</type>
      <name>L_move_oper_after</name>
      <anchorfile>l__opti__functions_8c.html</anchorfile>
      <anchor>ee6686419500df426238566edffb0296</anchor>
      <arglist>(L_Cb *cb, L_Oper *move_op, L_Oper *after_op)</arglist>
    </member>
    <member kind="function">
      <type>void</type>
      <name>L_move_dest_flow_after_emn</name>
      <anchorfile>l__opti__functions_8c.html</anchorfile>
      <anchor>ae72e38766f5e57723e0e6e923cd554e</anchor>
      <arglist>(L_Cb *from_cb, L_Flow *dst_flow, L_Cb *to_cb, L_Flow *to_after_flow)</arglist>
    </member>
    <member kind="function">
      <type>L_Flow *</type>
      <name>L_move_op_after_emn</name>
      <anchorfile>l__opti__functions_8c.html</anchorfile>
      <anchor>f8b97477df1ecf816552c1d274669a94</anchor>
      <arglist>(L_Cb *from_cb, L_Oper *op, L_Cb *to_cb, L_Oper *to_after_op)</arglist>
    </member>
    <member kind="function">
      <type>L_Cb *</type>
      <name>L_split_cb_after</name>
      <anchorfile>l__opti__functions_8c.html</anchorfile>
      <anchor>c67d6a549e966629413a6590e7746676</anchor>
      <arglist>(L_Func *fn, L_Cb *cb, L_Oper *op)</arglist>
    </member>
    <member kind="function">
      <type>L_Cb *</type>
      <name>L_split_arc</name>
      <anchorfile>l__opti__functions_8c.html</anchorfile>
      <anchor>c7aea48296ff77ef48781943b1e25627</anchor>
      <arglist>(L_Func *fn, L_Cb *src_cb, L_Flow *dst_fl)</arglist>
    </member>
    <member kind="function">
      <type>int</type>
      <name>L_expand_flow</name>
      <anchorfile>l__opti__functions_8c.html</anchorfile>
      <anchor>24eda308c338478bbeb24e24ecf12841</anchor>
      <arglist>(L_Func *fn, L_Cb *src_cb, L_Flow *oefl, int flags)</arglist>
    </member>
    <member kind="function">
      <type>void</type>
      <name>L_insert_op_at_dest_of_br</name>
      <anchorfile>l__opti__functions_8c.html</anchorfile>
      <anchor>f57536f4ee7457c128767f98607253ad</anchor>
      <arglist>(L_Cb *cb, L_Oper *br_op, L_Oper *op, int copy)</arglist>
    </member>
    <member kind="function">
      <type>void</type>
      <name>L_insert_op_at_fallthru_dest</name>
      <anchorfile>l__opti__functions_8c.html</anchorfile>
      <anchor>8a536e0f691d3ce1a92ed9e6550972c3</anchor>
      <arglist>(L_Cb *cb, L_Oper *op, int copy)</arglist>
    </member>
    <member kind="function">
      <type>L_Oper *</type>
      <name>L_setup_conditional_op_with_pred</name>
      <anchorfile>l__opti__functions_8c.html</anchorfile>
      <anchor>c7397df937db52acfda829dd3d112079</anchor>
      <arglist>(L_Cb *preheader)</arglist>
    </member>
    <member kind="function">
      <type>L_Oper *</type>
      <name>L_setup_conditional_op_with_cbs</name>
      <anchorfile>l__opti__functions_8c.html</anchorfile>
      <anchor>8ae5a1db6ae7aab688b8ddc8796acdf4</anchor>
      <arglist>(L_Cb *preheader)</arglist>
    </member>
    <member kind="function">
      <type>L_Cb *</type>
      <name>L_create_conditional_op_with_pred_in_cb</name>
      <anchorfile>l__opti__functions_8c.html</anchorfile>
      <anchor>d71620dc99f1163ad3c6b539432a9d3c</anchor>
      <arglist>(L_Oper *select, L_Oper *cond_op, L_Cb *at_cb, L_Oper *at_op)</arglist>
    </member>
    <member kind="function">
      <type>L_Cb *</type>
      <name>L_create_conditional_op_with_cbs_at_cb</name>
      <anchorfile>l__opti__functions_8c.html</anchorfile>
      <anchor>a26df010febcf5179b209fa597238f87</anchor>
      <arglist>(L_Func *fn, L_Oper *select, L_Oper *cond_op, L_Cb *at_cb, L_Oper *at_op)</arglist>
    </member>
    <member kind="function">
      <type>L_Oper *</type>
      <name>L_find_last_def_in_cb</name>
      <anchorfile>l__opti__functions_8c.html</anchorfile>
      <anchor>f8347f4e6e9cf4d7c7bbcd01d38fb095</anchor>
      <arglist>(L_Cb *cb, L_Operand *operand)</arglist>
    </member>
    <member kind="function">
      <type>void</type>
      <name>L_simplify_loop_branch1</name>
      <anchorfile>l__opti__functions_8c.html</anchorfile>
      <anchor>7b46291dae340b549c8f624de690a6a5</anchor>
      <arglist>(L_Loop *loop, L_Oper *op)</arglist>
    </member>
    <member kind="function">
      <type>void</type>
      <name>L_simplify_loop_branch2</name>
      <anchorfile>l__opti__functions_8c.html</anchorfile>
      <anchor>ca945572735ca1ce9908e94bf778f68b</anchor>
      <arglist>(L_Loop *loop, L_Oper *op)</arglist>
    </member>
    <member kind="function">
      <type>int</type>
      <name>L_str_reduced_opcode</name>
      <anchorfile>l__opti__functions_8c.html</anchorfile>
      <anchor>f5fec53de91627c8c1e9d553cdb9eb69</anchor>
      <arglist>(L_Oper *op)</arglist>
    </member>
    <member kind="function">
      <type>ITintmax</type>
      <name>L_evaluate_str_reduced_opcode</name>
      <anchorfile>l__opti__functions_8c.html</anchorfile>
      <anchor>4ef5a40901d34e500ce70532bd42bb9b</anchor>
      <arglist>(int opc, ITintmax s1, ITintmax s2)</arglist>
    </member>
    <member kind="function">
      <type>void</type>
      <name>L_insert_strength_reduced_op_into_loop</name>
      <anchorfile>l__opti__functions_8c.html</anchorfile>
      <anchor>e83e29caa5058f3aa4df6a51f345b493</anchor>
      <arglist>(L_Loop *loop, int *loop_cb, int num_cb, L_Oper *op, L_Operand *new_reg)</arglist>
    </member>
    <member kind="function">
      <type>void</type>
      <name>L_reinit_induction_var</name>
      <anchorfile>l__opti__functions_8c.html</anchorfile>
      <anchor>858ba5ab6ed5f546e8b4406ba70b7364</anchor>
      <arglist>(L_Loop *loop, int *loop_cb, int num_cb, L_Operand *operand, L_Oper *last_use)</arglist>
    </member>
    <member kind="function">
      <type>void</type>
      <name>L_delete_all_basic_ind_var_op_from_loop</name>
      <anchorfile>l__opti__functions_8c.html</anchorfile>
      <anchor>dea718bfe273cb74baa34d23d5ffe989</anchor>
      <arglist>(L_Loop *loop, int *loop_cb, int num_cb, L_Operand *operand)</arglist>
    </member>
    <member kind="function">
      <type>int</type>
      <name>L_reorder_ops_so_no_use_betw</name>
      <anchorfile>l__opti__functions_8c.html</anchorfile>
      <anchor>0aeef98116ba7a7d96320f4f4868482c</anchor>
      <arglist>(L_Loop *loop, L_Cb *cb, L_Operand *operand, L_Oper *op1, L_Oper *op2)</arglist>
    </member>
    <member kind="function">
      <type>int</type>
      <name>L_find_ind_initial_offset</name>
      <anchorfile>l__opti__functions_8c.html</anchorfile>
      <anchor>66d53d26b00f7694cbcb83ac86274b80</anchor>
      <arglist>(L_Cb *preheader, L_Operand *operand1, L_Operand *operand2, L_Oper *start_op1, L_Oper *start_op2, L_Ind_Info *ind_info)</arglist>
    </member>
    <member kind="function">
      <type>void</type>
      <name>L_simplify_combs_of_ind_vars</name>
      <anchorfile>l__opti__functions_8c.html</anchorfile>
      <anchor>bfd523cf4662a05845cb7512fbfcb874</anchor>
      <arglist>(L_Loop *loop, int *loop_cb, int num_cb)</arglist>
    </member>
    <member kind="function">
      <type>void</type>
      <name>L_induction_elim_1</name>
      <anchorfile>l__opti__functions_8c.html</anchorfile>
      <anchor>afb10acf9f9fb1748992fc1ff534d942</anchor>
      <arglist>(L_Loop *loop, int *loop_cb, int num_cb, L_Operand *operand1, L_Operand *operand2)</arglist>
    </member>
    <member kind="function">
      <type>void</type>
      <name>L_change_offset_for_all_uses</name>
      <anchorfile>l__opti__functions_8c.html</anchorfile>
      <anchor>c79d093831afb9b9193ac0a6fe8bc5a3</anchor>
      <arglist>(L_Loop *loop, int *loop_cb, int num_cb, L_Operand *operand, int offset)</arglist>
    </member>
    <member kind="function">
      <type>void</type>
      <name>L_induction_elim_2</name>
      <anchorfile>l__opti__functions_8c.html</anchorfile>
      <anchor>8d2d2cec44cb4ccf4b5ef61735d4ea03</anchor>
      <arglist>(L_Loop *loop, int *loop_cb, int num_cb, L_Operand *operand1, L_Operand *operand2)</arglist>
    </member>
    <member kind="function">
      <type>void</type>
      <name>L_induction_elim_3</name>
      <anchorfile>l__opti__functions_8c.html</anchorfile>
      <anchor>0ab2db7dcfd9e7d92715c848eeac684c</anchor>
      <arglist>(L_Loop *loop, int *loop_cb, int num_cb, L_Operand *operand1, L_Operand *operand2)</arglist>
    </member>
    <member kind="function">
      <type>void</type>
      <name>L_induction_elim_4</name>
      <anchorfile>l__opti__functions_8c.html</anchorfile>
      <anchor>1d858df352804494e1b66cd80ca7b1ab</anchor>
      <arglist>(L_Loop *loop, int *loop_cb, int num_cb, L_Operand *operand1, L_Operand *operand2)</arglist>
    </member>
    <member kind="function">
      <type>L_Oper *</type>
      <name>L_find_last_use_of_ind_var</name>
      <anchorfile>l__opti__functions_8c.html</anchorfile>
      <anchor>fc7f60c94f3b63974d1b4bf1b10ec283</anchor>
      <arglist>(L_Loop *loop, int *loop_cb, int num_cb, int *backedge_cb, int num_backedge_cb, L_Cb *cb, L_Oper *op)</arglist>
    </member>
    <member kind="function">
      <type>int</type>
      <name>L_num_ind_var_to_reassociate</name>
      <anchorfile>l__opti__functions_8c.html</anchorfile>
      <anchor>605277176ae536832e5d283b2244b221</anchor>
      <arglist>(L_Loop *loop, int *loop_cb, int num_cb, L_Operand *ind_var)</arglist>
    </member>
    <member kind="function">
      <type>void</type>
      <name>L_reassociate_ind_var</name>
      <anchorfile>l__opti__functions_8c.html</anchorfile>
      <anchor>a1b6a84f2fa7602855bd01e44724e6ac</anchor>
      <arglist>(L_Loop *loop, int *loop_cb, int num_cb, L_Operand *ind_var, L_Oper *last_use, int num_new_ind_var)</arglist>
    </member>
    <member kind="function">
      <type>void</type>
      <name>L_mark_as_post_increment</name>
      <anchorfile>l__opti__functions_8c.html</anchorfile>
      <anchor>25bc1b967173709d675a2dd60d798b99</anchor>
      <arglist>(L_Oper *mem_op, L_Oper *inc_op)</arglist>
    </member>
    <member kind="function">
      <type>void</type>
      <name>L_mark_as_pre_increment</name>
      <anchorfile>l__opti__functions_8c.html</anchorfile>
      <anchor>05fd0c4bd5ded94bee20e5872bccdc5f</anchor>
      <arglist>(L_Oper *mem_op, L_Oper *inc_op)</arglist>
    </member>
    <member kind="function">
      <type>void</type>
      <name>L_unmark_as_post_increment</name>
      <anchorfile>l__opti__functions_8c.html</anchorfile>
      <anchor>8558e583b22d44cd01c1f51f875140cf</anchor>
      <arglist>(L_Oper *mem_op, L_Oper *inc_op)</arglist>
    </member>
    <member kind="function">
      <type>void</type>
      <name>L_unmark_as_pre_increment</name>
      <anchorfile>l__opti__functions_8c.html</anchorfile>
      <anchor>78650fd8b7bbba9129086bb9540f1353</anchor>
      <arglist>(L_Oper *mem_op, L_Oper *inc_op)</arglist>
    </member>
    <member kind="function">
      <type>void</type>
      <name>L_unmark_as_pre_post_increment</name>
      <anchorfile>l__opti__functions_8c.html</anchorfile>
      <anchor>ae88b3d51830a7aa7bd076bb132e4da4</anchor>
      <arglist>(L_Oper *oper)</arglist>
    </member>
    <member kind="function">
      <type>int</type>
      <name>L_breakup_pre_post_inc_ops</name>
      <anchorfile>l__opti__functions_8c.html</anchorfile>
      <anchor>d4f3a58593ad14389a4190229bd53a0c</anchor>
      <arglist>(L_Func *fn)</arglist>
    </member>
    <member kind="function">
      <type>int</type>
      <name>L_remove_uncombinable_pre_post_inc_ops</name>
      <anchorfile>l__opti__functions_8c.html</anchorfile>
      <anchor>5cc156530780d390115a8adf8a132703</anchor>
      <arglist>(L_Func *fn)</arglist>
    </member>
    <member kind="function">
      <type>int</type>
      <name>L_generate_pre_post_inc_ops</name>
      <anchorfile>l__opti__functions_8c.html</anchorfile>
      <anchor>4ef3a17e7988750f10a2f3b506d9585a</anchor>
      <arglist>(L_Func *fn)</arglist>
    </member>
    <member kind="function">
      <type>void</type>
      <name>L_unmark_all_pre_post_increments_for_ind</name>
      <anchorfile>l__opti__functions_8c.html</anchorfile>
      <anchor>caf53d0e6b325b78d0ce798e1d88d4a5</anchor>
      <arglist>(L_Loop *loop, int *loop_cb, int num_cb, L_Operand *ind_var)</arglist>
    </member>
    <member kind="function">
      <type>void</type>
      <name>L_unmark_all_pre_post_increments_for_operand</name>
      <anchorfile>l__opti__functions_8c.html</anchorfile>
      <anchor>b256a2cb4c83ac33d05c3b03fa67f9ff</anchor>
      <arglist>(L_Func *fn, L_Operand *operand)</arglist>
    </member>
    <member kind="function">
      <type>void</type>
      <name>L_unmark_all_pre_post_increments</name>
      <anchorfile>l__opti__functions_8c.html</anchorfile>
      <anchor>10eda6b1381970ce3f1fc842a52767dd</anchor>
      <arglist>(L_Func *fn)</arglist>
    </member>
    <member kind="function">
      <type>int</type>
      <name>L_num_reg_used</name>
      <anchorfile>l__opti__functions_8c.html</anchorfile>
      <anchor>7d7d32a40c2e35b40f5af95282aba8fa</anchor>
      <arglist>(int ctype)</arglist>
    </member>
    <member kind="function">
      <type>void</type>
      <name>L_reset_reg_count</name>
      <anchorfile>l__opti__functions_8c.html</anchorfile>
      <anchor>94052e13fa9e5ce840ba0f0750c128a6</anchor>
      <arglist>(L_Reg_Count *reg_count)</arglist>
    </member>
    <member kind="function">
      <type>void</type>
      <name>L_increment_reg_count</name>
      <anchorfile>l__opti__functions_8c.html</anchorfile>
      <anchor>05fe6be5a9e8595a351196a64d9380ce</anchor>
      <arglist>(int reg_id, L_Reg_Count *reg_count)</arglist>
    </member>
    <member kind="function">
      <type>void</type>
      <name>L_update_reg_count</name>
      <anchorfile>l__opti__functions_8c.html</anchorfile>
      <anchor>4b5495200e4681c81efc9cd87684e917</anchor>
      <arglist>(L_Reg_Count *max, L_Reg_Count *cur)</arglist>
    </member>
    <member kind="function">
      <type>void</type>
      <name>L_reset_ctype_array</name>
      <anchorfile>l__opti__functions_8c.html</anchorfile>
      <anchor>33a42abfb1462c76b5a9885d41595aa9</anchor>
      <arglist>()</arglist>
    </member>
    <member kind="function">
      <type>void</type>
      <name>L_setup_ctype_array</name>
      <anchorfile>l__opti__functions_8c.html</anchorfile>
      <anchor>29fe55f112e87514ec5be81cd17c5b60</anchor>
      <arglist>(L_Func *fn)</arglist>
    </member>
    <member kind="function">
      <type>void</type>
      <name>L_estimate_num_live_regs_in_cb</name>
      <anchorfile>l__opti__functions_8c.html</anchorfile>
      <anchor>130b052edfff9e68ecb717e1ca78f067</anchor>
      <arglist>(L_Cb *cb)</arglist>
    </member>
    <member kind="function">
      <type>void</type>
      <name>L_estimate_num_live_regs_in_loop</name>
      <anchorfile>l__opti__functions_8c.html</anchorfile>
      <anchor>a4b467584adebf67337d086b2b73dc9f</anchor>
      <arglist>(L_Loop *loop, int *loop_cb, int num_cb)</arglist>
    </member>
    <member kind="function">
      <type>void</type>
      <name>L_estimate_num_live_regs_in_func</name>
      <anchorfile>l__opti__functions_8c.html</anchorfile>
      <anchor>b4b328eac3a0d50f36df02b9415e14f4</anchor>
      <arglist>(L_Func *fn)</arglist>
    </member>
    <member kind="function">
      <type>void</type>
      <name>L_set_function_mask_flag</name>
      <anchorfile>l__opti__functions_8c.html</anchorfile>
      <anchor>ee0f9b23bad04d825f78297f44afafdc</anchor>
      <arglist>(L_Func *fn)</arglist>
    </member>
    <member kind="function">
      <type>void</type>
      <name>L_mark_superblocks</name>
      <anchorfile>l__opti__functions_8c.html</anchorfile>
      <anchor>3efab201a62386c5f9eafcc63bea3a7c</anchor>
      <arglist>(L_Func *fn)</arglist>
    </member>
    <member kind="function">
      <type>void</type>
      <name>L_rename_subsequent_uses</name>
      <anchorfile>l__opti__functions_8c.html</anchorfile>
      <anchor>66d46db53af7f9522b573cbfceb41f24</anchor>
      <arglist>(L_Oper *ren_oper, L_Oper *def_oper, L_Operand *dest_reg, L_Operand *new_reg)</arglist>
    </member>
    <member kind="variable">
      <type>L_Reg_Count</type>
      <name>L_max_reg_count</name>
      <anchorfile>l__opti__functions_8c.html</anchorfile>
      <anchor>05ef741c2a86535dcc5a7ac54cac993f</anchor>
      <arglist></arglist>
    </member>
    <member kind="variable">
      <type>L_Reg_Count</type>
      <name>L_cur_reg_count</name>
      <anchorfile>l__opti__functions_8c.html</anchorfile>
      <anchor>79e85c77637dfbc198af8a7d27892f34</anchor>
      <arglist></arglist>
    </member>
  </compound>
  <compound kind="file">
    <name>l_opti_functions.h</name>
    <path>/mnt/hgfs/mkt/Desktop/openimpact/src/Lcode/opti/Lopti/</path>
    <filename>l__opti__functions_8h</filename>
    <class kind="struct">L_Reg_Count</class>
    <member kind="define">
      <type>#define</type>
      <name>LOPTI_EXPAND_ALLOW_FALLTHRU</name>
      <anchorfile>l__opti__functions_8h.html</anchorfile>
      <anchor>9adb6a3ef4839f32cdca7ef4cfdc4ea2</anchor>
      <arglist></arglist>
    </member>
    <member kind="define">
      <type>#define</type>
      <name>LOPTI_EXPAND_ALLOW_UNCOND</name>
      <anchorfile>l__opti__functions_8h.html</anchorfile>
      <anchor>cac67730ff6735a0446a3c0be60e5f05</anchor>
      <arglist></arglist>
    </member>
    <member kind="define">
      <type>#define</type>
      <name>LOPTI_EXPAND_ALLOW_COND</name>
      <anchorfile>l__opti__functions_8h.html</anchorfile>
      <anchor>49aec66149394b87eb2f21480709bc05</anchor>
      <arglist></arglist>
    </member>
    <member kind="function">
      <type>void</type>
      <name>L_create_divide_operations</name>
      <anchorfile>l__opti__functions_8h.html</anchorfile>
      <anchor>52029dd001a2c3e3ed950f1406082fcb</anchor>
      <arglist>(L_Oper *div_oper, L_Cb *cb)</arglist>
    </member>
    <member kind="function">
      <type>void</type>
      <name>L_create_rem_operations</name>
      <anchorfile>l__opti__functions_8h.html</anchorfile>
      <anchor>dff8ac039b3ecffe07700183c587017c</anchor>
      <arglist>(L_Oper *rem_oper, L_Cb *cb)</arglist>
    </member>
    <member kind="function">
      <type>L_Oper *</type>
      <name>L_create_move</name>
      <anchorfile>l__opti__functions_8h.html</anchorfile>
      <anchor>efb698f19fb7851cd2d49fadc5804c6b</anchor>
      <arglist>(L_Operand *dest, L_Operand *src)</arglist>
    </member>
    <member kind="function">
      <type>L_Oper *</type>
      <name>L_create_move_using</name>
      <anchorfile>l__opti__functions_8h.html</anchorfile>
      <anchor>7824370b3b80e17ccde25ec3ea394e56</anchor>
      <arglist>(L_Operand *dest, L_Operand *src, L_Oper *oper)</arglist>
    </member>
    <member kind="function">
      <type>void</type>
      <name>L_convert_to_zero_extend_oper</name>
      <anchorfile>l__opti__functions_8h.html</anchorfile>
      <anchor>844f8a3ffdca6b53f13c916b80784024</anchor>
      <arglist>(L_Oper *, L_Operand *, L_Operand *)</arglist>
    </member>
    <member kind="function">
      <type>void</type>
      <name>L_convert_to_move</name>
      <anchorfile>l__opti__functions_8h.html</anchorfile>
      <anchor>08dcf3d643e44dcedb9aff510f650f8e</anchor>
      <arglist>(L_Oper *, L_Operand *, L_Operand *)</arglist>
    </member>
    <member kind="function">
      <type>void</type>
      <name>L_convert_to_move_of_zero</name>
      <anchorfile>l__opti__functions_8h.html</anchorfile>
      <anchor>07e2ddd27241526ba3683bf40f860fdc</anchor>
      <arglist>(L_Oper *, L_Operand *)</arglist>
    </member>
    <member kind="function">
      <type>void</type>
      <name>L_convert_to_move_of_one</name>
      <anchorfile>l__opti__functions_8h.html</anchorfile>
      <anchor>db99c9cdaf043b3a05c84cd3b2a9f7ac</anchor>
      <arglist>(L_Oper *, L_Operand *)</arglist>
    </member>
    <member kind="function">
      <type>void</type>
      <name>L_convert_to_jump</name>
      <anchorfile>l__opti__functions_8h.html</anchorfile>
      <anchor>8b7b63b4000cbd9fc77769a83947212c</anchor>
      <arglist>(L_Oper *, L_Operand *)</arglist>
    </member>
    <member kind="function">
      <type>void</type>
      <name>L_convert_to_extended_move</name>
      <anchorfile>l__opti__functions_8h.html</anchorfile>
      <anchor>0f63b5bded042c2c16c2c5384770bd94</anchor>
      <arglist>(L_Oper *, L_Operand *, L_Operand *)</arglist>
    </member>
    <member kind="function">
      <type>void</type>
      <name>L_convert_to_extract</name>
      <anchorfile>l__opti__functions_8h.html</anchorfile>
      <anchor>9e702a37bc6feb8094f76bb5cb17c941</anchor>
      <arglist>(L_Oper *, L_Operand *, L_Operand *, int)</arglist>
    </member>
    <member kind="function">
      <type>void</type>
      <name>L_fix_cond_br</name>
      <anchorfile>l__opti__functions_8h.html</anchorfile>
      <anchor>03581c1373345cdbd349597c243dd0c9</anchor>
      <arglist>(L_Cb *, L_Oper *, int)</arglist>
    </member>
    <member kind="function">
      <type>ITintmax</type>
      <name>L_evaluate_int_arithmetic</name>
      <anchorfile>l__opti__functions_8h.html</anchorfile>
      <anchor>e52c00ae9182bfd3285bc35e0b35ba37</anchor>
      <arglist>(L_Oper *)</arglist>
    </member>
    <member kind="function">
      <type>float</type>
      <name>L_evaluate_flt_arithmetic</name>
      <anchorfile>l__opti__functions_8h.html</anchorfile>
      <anchor>5db0e3fd87e1098ba9a9262c55bb3b48</anchor>
      <arglist>(L_Oper *)</arglist>
    </member>
    <member kind="function">
      <type>double</type>
      <name>L_evaluate_dbl_arithmetic</name>
      <anchorfile>l__opti__functions_8h.html</anchorfile>
      <anchor>e829c63bd1fd8352927a0248d9146965</anchor>
      <arglist>(L_Oper *)</arglist>
    </member>
    <member kind="function">
      <type>int</type>
      <name>L_evaluate_int_compare_with_sources</name>
      <anchorfile>l__opti__functions_8h.html</anchorfile>
      <anchor>21cecf20b9b516b1a96336c5b90cf637</anchor>
      <arglist>(L_Oper *, ITintmax, ITintmax)</arglist>
    </member>
    <member kind="function">
      <type>int</type>
      <name>L_evaluate_flt_compare_with_sources</name>
      <anchorfile>l__opti__functions_8h.html</anchorfile>
      <anchor>179dc9fb90db1117932e1825f8f9dd08</anchor>
      <arglist>(L_Oper *, float, float)</arglist>
    </member>
    <member kind="function">
      <type>int</type>
      <name>L_evaluate_dbl_compare_with_sources</name>
      <anchorfile>l__opti__functions_8h.html</anchorfile>
      <anchor>c45019b66efbf5b1b043da174f067867</anchor>
      <arglist>(L_Oper *, double, double)</arglist>
    </member>
    <member kind="function">
      <type>int</type>
      <name>L_evaluate_int_compare</name>
      <anchorfile>l__opti__functions_8h.html</anchorfile>
      <anchor>cda26aa084957629f38812eb5191ab9c</anchor>
      <arglist>(L_Oper *)</arglist>
    </member>
    <member kind="function">
      <type>int</type>
      <name>L_evaluate_flt_compare</name>
      <anchorfile>l__opti__functions_8h.html</anchorfile>
      <anchor>1824ec2b61ee5da89d4dcbacf1655325</anchor>
      <arglist>(L_Oper *)</arglist>
    </member>
    <member kind="function">
      <type>int</type>
      <name>L_evaluate_dbl_compare</name>
      <anchorfile>l__opti__functions_8h.html</anchorfile>
      <anchor>cbfb5a5fc6123e61d17450a97ac2a019</anchor>
      <arglist>(L_Oper *)</arglist>
    </member>
    <member kind="function">
      <type>int</type>
      <name>L_evaluate_compare</name>
      <anchorfile>l__opti__functions_8h.html</anchorfile>
      <anchor>2f6091c924155db21ca9072b5f7f5d65</anchor>
      <arglist>(L_Oper *)</arglist>
    </member>
    <member kind="function">
      <type>int</type>
      <name>L_move_from_ctype</name>
      <anchorfile>l__opti__functions_8h.html</anchorfile>
      <anchor>50a2e04ceaced682104d5003f5e924d0</anchor>
      <arglist>(int)</arglist>
    </member>
    <member kind="function">
      <type>int</type>
      <name>L_corresponding_load</name>
      <anchorfile>l__opti__functions_8h.html</anchorfile>
      <anchor>df1ddb39e2b2fc25be523c2a812eaa2f</anchor>
      <arglist>(L_Oper *)</arglist>
    </member>
    <member kind="function">
      <type>int</type>
      <name>L_corresponding_preincrement_load</name>
      <anchorfile>l__opti__functions_8h.html</anchorfile>
      <anchor>b05fe2f9d3bcff30740d784139b84e49</anchor>
      <arglist>(L_Oper *)</arglist>
    </member>
    <member kind="function">
      <type>int</type>
      <name>L_corresponding_postincrement_load</name>
      <anchorfile>l__opti__functions_8h.html</anchorfile>
      <anchor>bafcf5ace550dda95ead69d402b4549b</anchor>
      <arglist>(L_Oper *)</arglist>
    </member>
    <member kind="function">
      <type>int</type>
      <name>L_corresponding_store</name>
      <anchorfile>l__opti__functions_8h.html</anchorfile>
      <anchor>c7858bf7ecc8512cc6511f2167ea374b</anchor>
      <arglist>(L_Oper *)</arglist>
    </member>
    <member kind="function">
      <type>int</type>
      <name>L_corresponding_preincrement_store</name>
      <anchorfile>l__opti__functions_8h.html</anchorfile>
      <anchor>19a4171261a9e7fa26a3359c17b619d1</anchor>
      <arglist>(L_Oper *)</arglist>
    </member>
    <member kind="function">
      <type>int</type>
      <name>L_corresponding_postincrement_store</name>
      <anchorfile>l__opti__functions_8h.html</anchorfile>
      <anchor>d5023e58c26fc970784cd8c747858ecc</anchor>
      <arglist>(L_Oper *)</arglist>
    </member>
    <member kind="function">
      <type>int</type>
      <name>L_corresponding_mov</name>
      <anchorfile>l__opti__functions_8h.html</anchorfile>
      <anchor>ace784fea0bf7535c924be2d67979670</anchor>
      <arglist>(L_Oper *)</arglist>
    </member>
    <member kind="function">
      <type>int</type>
      <name>L_corresponding_add</name>
      <anchorfile>l__opti__functions_8h.html</anchorfile>
      <anchor>666f675135cb78b18ccb8d7620584f1c</anchor>
      <arglist>(L_Oper *)</arglist>
    </member>
    <member kind="function">
      <type>int</type>
      <name>L_corresponding_mul_add</name>
      <anchorfile>l__opti__functions_8h.html</anchorfile>
      <anchor>80cc770e8f5e570b4112ea4d3db63e48</anchor>
      <arglist>(L_Oper *)</arglist>
    </member>
    <member kind="function">
      <type>int</type>
      <name>L_corresponding_mul_sub</name>
      <anchorfile>l__opti__functions_8h.html</anchorfile>
      <anchor>9718f99963e5ee603921c3fc2a3ad9d2</anchor>
      <arglist>(L_Oper *)</arglist>
    </member>
    <member kind="function">
      <type>int</type>
      <name>L_corresponding_mul_sub_rev</name>
      <anchorfile>l__opti__functions_8h.html</anchorfile>
      <anchor>580d1c3fec2d573475c59b6fbf0f4cad</anchor>
      <arglist>(L_Oper *)</arglist>
    </member>
    <member kind="function">
      <type>int</type>
      <name>L_inverse_arithmetic</name>
      <anchorfile>l__opti__functions_8h.html</anchorfile>
      <anchor>b72895b6c9dfbf7893acb489c225a9b9</anchor>
      <arglist>(int)</arglist>
    </member>
    <member kind="function">
      <type>int</type>
      <name>L_has_const_operand_and_realign_oper</name>
      <anchorfile>l__opti__functions_8h.html</anchorfile>
      <anchor>f9d1b91d158579d9bfeeef744aef5c28</anchor>
      <arglist>(L_Oper *)</arglist>
    </member>
    <member kind="function">
      <type>void</type>
      <name>L_undo_and_combine</name>
      <anchorfile>l__opti__functions_8h.html</anchorfile>
      <anchor>0800f1fc57579e662598e40dcdd7c8a2</anchor>
      <arglist>(L_Oper *, L_Oper *)</arglist>
    </member>
    <member kind="function">
      <type>int</type>
      <name>L_combine_operations</name>
      <anchorfile>l__opti__functions_8h.html</anchorfile>
      <anchor>2bafea70fb61efdd85272562264d948c</anchor>
      <arglist>(L_Oper *, L_Oper *)</arglist>
    </member>
    <member kind="function">
      <type>void</type>
      <name>L_undo_increment</name>
      <anchorfile>l__opti__functions_8h.html</anchorfile>
      <anchor>e375ebd3c511afc9194f268694ccb6dd</anchor>
      <arglist>(L_Oper *, L_Oper *)</arglist>
    </member>
    <member kind="function">
      <type>void</type>
      <name>L_combine_increment_operations</name>
      <anchorfile>l__opti__functions_8h.html</anchorfile>
      <anchor>22da3812f0376cd5806d832a1f9c11b1</anchor>
      <arglist>(L_Oper *, L_Oper *)</arglist>
    </member>
    <member kind="function">
      <type>void</type>
      <name>L_move_oper_before</name>
      <anchorfile>l__opti__functions_8h.html</anchorfile>
      <anchor>2f7b5628091cd66477700a27fa964707</anchor>
      <arglist>(L_Cb *, L_Oper *, L_Oper *)</arglist>
    </member>
    <member kind="function">
      <type>void</type>
      <name>L_move_oper_after</name>
      <anchorfile>l__opti__functions_8h.html</anchorfile>
      <anchor>bc251bbba045988863b74d671c3aa747</anchor>
      <arglist>(L_Cb *, L_Oper *, L_Oper *)</arglist>
    </member>
    <member kind="function">
      <type>L_Cb *</type>
      <name>L_split_cb_after</name>
      <anchorfile>l__opti__functions_8h.html</anchorfile>
      <anchor>fbe9a62ba034d52c014d0fed246b7f7b</anchor>
      <arglist>(L_Func *, L_Cb *, L_Oper *)</arglist>
    </member>
    <member kind="function">
      <type>L_Cb *</type>
      <name>L_split_arc</name>
      <anchorfile>l__opti__functions_8h.html</anchorfile>
      <anchor>c7aea48296ff77ef48781943b1e25627</anchor>
      <arglist>(L_Func *fn, L_Cb *src_cb, L_Flow *dst_fl)</arglist>
    </member>
    <member kind="function">
      <type>int</type>
      <name>L_expand_flow</name>
      <anchorfile>l__opti__functions_8h.html</anchorfile>
      <anchor>24eda308c338478bbeb24e24ecf12841</anchor>
      <arglist>(L_Func *fn, L_Cb *src_cb, L_Flow *oefl, int flags)</arglist>
    </member>
    <member kind="function">
      <type>void</type>
      <name>L_insert_op_at_dest_of_br</name>
      <anchorfile>l__opti__functions_8h.html</anchorfile>
      <anchor>97e4d859f13efaa1b6946c854e9b0d1c</anchor>
      <arglist>(L_Cb *, L_Oper *, L_Oper *, int)</arglist>
    </member>
    <member kind="function">
      <type>void</type>
      <name>L_insert_op_at_fallthru_dest</name>
      <anchorfile>l__opti__functions_8h.html</anchorfile>
      <anchor>12078fdb01a359a9104c1319eda8341e</anchor>
      <arglist>(L_Cb *, L_Oper *, int)</arglist>
    </member>
    <member kind="function">
      <type>L_Oper *</type>
      <name>L_setup_conditional_op_with_pred</name>
      <anchorfile>l__opti__functions_8h.html</anchorfile>
      <anchor>7e1b461603eca513329c80880a15353c</anchor>
      <arglist>(L_Cb *)</arglist>
    </member>
    <member kind="function">
      <type>L_Oper *</type>
      <name>L_setup_conditional_op_with_cbs</name>
      <anchorfile>l__opti__functions_8h.html</anchorfile>
      <anchor>e8fa496169aefb46c4f6ba8c0a2a961a</anchor>
      <arglist>(L_Cb *)</arglist>
    </member>
    <member kind="function">
      <type>L_Cb *</type>
      <name>L_create_conditional_op_with_pred_in_cb</name>
      <anchorfile>l__opti__functions_8h.html</anchorfile>
      <anchor>c267f3a614ee56f4578a63a8b2ac84c5</anchor>
      <arglist>(L_Oper *, L_Oper *, L_Cb *, L_Oper *)</arglist>
    </member>
    <member kind="function">
      <type>L_Cb *</type>
      <name>L_create_conditional_op_with_cbs_at_cb</name>
      <anchorfile>l__opti__functions_8h.html</anchorfile>
      <anchor>11c560cb02a80f216a8358b21a57a028</anchor>
      <arglist>(L_Func *, L_Oper *, L_Oper *, L_Cb *, L_Oper *)</arglist>
    </member>
    <member kind="function">
      <type>L_Oper *</type>
      <name>L_find_last_def_in_cb</name>
      <anchorfile>l__opti__functions_8h.html</anchorfile>
      <anchor>0e67de9d018d959c6e439b3f0b5df31c</anchor>
      <arglist>(L_Cb *, L_Operand *)</arglist>
    </member>
    <member kind="function">
      <type>void</type>
      <name>L_simplify_loop_branch1</name>
      <anchorfile>l__opti__functions_8h.html</anchorfile>
      <anchor>f0d3ec61bd4bb22edab724f2caf734d2</anchor>
      <arglist>(L_Loop *, L_Oper *)</arglist>
    </member>
    <member kind="function">
      <type>void</type>
      <name>L_simplify_loop_branch2</name>
      <anchorfile>l__opti__functions_8h.html</anchorfile>
      <anchor>2d121548775a99d0a896829ad0ac98f1</anchor>
      <arglist>(L_Loop *, L_Oper *)</arglist>
    </member>
    <member kind="function">
      <type>int</type>
      <name>L_str_reduced_opcode</name>
      <anchorfile>l__opti__functions_8h.html</anchorfile>
      <anchor>bc33ca459b09046602aaa664a174527f</anchor>
      <arglist>(L_Oper *)</arglist>
    </member>
    <member kind="function">
      <type>ITintmax</type>
      <name>L_evaluate_str_reduced_opcode</name>
      <anchorfile>l__opti__functions_8h.html</anchorfile>
      <anchor>7846e5dd1a40c2acdfb66a1d5128c297</anchor>
      <arglist>(int, ITintmax, ITintmax)</arglist>
    </member>
    <member kind="function">
      <type>void</type>
      <name>L_insert_strength_reduced_op_into_loop</name>
      <anchorfile>l__opti__functions_8h.html</anchorfile>
      <anchor>a7f9d1b07fd9a5dcc8d51379f71f32b3</anchor>
      <arglist>(L_Loop *, int *, int, L_Oper *, L_Operand *)</arglist>
    </member>
    <member kind="function">
      <type>void</type>
      <name>L_reinit_induction_var</name>
      <anchorfile>l__opti__functions_8h.html</anchorfile>
      <anchor>4ac2012daa24fd7ce1ce8cc90ce0aaf2</anchor>
      <arglist>(L_Loop *, int *, int, L_Operand *, L_Oper *)</arglist>
    </member>
    <member kind="function">
      <type>void</type>
      <name>L_delete_all_basic_ind_var_op_from_loop</name>
      <anchorfile>l__opti__functions_8h.html</anchorfile>
      <anchor>1ddb07d76365b699a0dbe503cbee98c6</anchor>
      <arglist>(L_Loop *, int *, int, L_Operand *)</arglist>
    </member>
    <member kind="function">
      <type>int</type>
      <name>L_reorder_ops_so_no_use_betw</name>
      <anchorfile>l__opti__functions_8h.html</anchorfile>
      <anchor>bdf3551229758bbcc2eac23fdc205e26</anchor>
      <arglist>(L_Loop *, L_Cb *, L_Operand *, L_Oper *, L_Oper *)</arglist>
    </member>
    <member kind="function">
      <type>int</type>
      <name>L_find_ind_initial_offset</name>
      <anchorfile>l__opti__functions_8h.html</anchorfile>
      <anchor>bd5761f2591b2bc2208998a9c0503d91</anchor>
      <arglist>(L_Cb *, L_Operand *, L_Operand *, L_Oper *, L_Oper *, L_Ind_Info *)</arglist>
    </member>
    <member kind="function">
      <type>void</type>
      <name>L_simplify_combs_of_ind_vars</name>
      <anchorfile>l__opti__functions_8h.html</anchorfile>
      <anchor>6b7e801ae83b3d749ce9613be664a295</anchor>
      <arglist>(L_Loop *, int *, int)</arglist>
    </member>
    <member kind="function">
      <type>void</type>
      <name>L_induction_elim_1</name>
      <anchorfile>l__opti__functions_8h.html</anchorfile>
      <anchor>f61357ec467e8f33be46c15d8de0f50f</anchor>
      <arglist>(L_Loop *, int *, int, L_Operand *, L_Operand *)</arglist>
    </member>
    <member kind="function">
      <type>void</type>
      <name>L_induction_elim_2</name>
      <anchorfile>l__opti__functions_8h.html</anchorfile>
      <anchor>bcb89c00514b16f74685b8bdb62e9c02</anchor>
      <arglist>(L_Loop *, int *, int, L_Operand *, L_Operand *)</arglist>
    </member>
    <member kind="function">
      <type>void</type>
      <name>L_induction_elim_3</name>
      <anchorfile>l__opti__functions_8h.html</anchorfile>
      <anchor>15dc11bba38a071fefb4daba9e64f6c2</anchor>
      <arglist>(L_Loop *, int *, int, L_Operand *, L_Operand *)</arglist>
    </member>
    <member kind="function">
      <type>void</type>
      <name>L_induction_elim_4</name>
      <anchorfile>l__opti__functions_8h.html</anchorfile>
      <anchor>e89a610e042c73b34055e65df9691fff</anchor>
      <arglist>(L_Loop *, int *, int, L_Operand *, L_Operand *)</arglist>
    </member>
    <member kind="function">
      <type>L_Oper *</type>
      <name>L_find_last_use_of_ind_var</name>
      <anchorfile>l__opti__functions_8h.html</anchorfile>
      <anchor>da7e2e41ca274f3685fe47bc16fb804c</anchor>
      <arglist>(L_Loop *, int *, int, int *, int, L_Cb *, L_Oper *)</arglist>
    </member>
    <member kind="function">
      <type>int</type>
      <name>L_num_ind_var_to_reassociate</name>
      <anchorfile>l__opti__functions_8h.html</anchorfile>
      <anchor>c983f4ceee042fc2d29a03b6fdff4380</anchor>
      <arglist>(L_Loop *, int *, int, L_Operand *)</arglist>
    </member>
    <member kind="function">
      <type>void</type>
      <name>L_reassociate_ind_var</name>
      <anchorfile>l__opti__functions_8h.html</anchorfile>
      <anchor>5bd30f560ad1a18271fbe7c3c4fa156e</anchor>
      <arglist>(L_Loop *, int *, int, L_Operand *, L_Oper *, int)</arglist>
    </member>
    <member kind="function">
      <type>void</type>
      <name>L_mark_as_post_increment</name>
      <anchorfile>l__opti__functions_8h.html</anchorfile>
      <anchor>c922d8895eaaa03dcb4ddedfcee154db</anchor>
      <arglist>(L_Oper *, L_Oper *)</arglist>
    </member>
    <member kind="function">
      <type>void</type>
      <name>L_mark_as_pre_increment</name>
      <anchorfile>l__opti__functions_8h.html</anchorfile>
      <anchor>55c1372893cf4c96cf58d551ee60a098</anchor>
      <arglist>(L_Oper *, L_Oper *)</arglist>
    </member>
    <member kind="function">
      <type>void</type>
      <name>L_unmark_as_post_increment</name>
      <anchorfile>l__opti__functions_8h.html</anchorfile>
      <anchor>e658c054bad3522c0c9667fc6b309c83</anchor>
      <arglist>(L_Oper *, L_Oper *)</arglist>
    </member>
    <member kind="function">
      <type>void</type>
      <name>L_unmark_as_pre_increment</name>
      <anchorfile>l__opti__functions_8h.html</anchorfile>
      <anchor>b33e50438c4148ea0e00cfd058c02ffb</anchor>
      <arglist>(L_Oper *, L_Oper *)</arglist>
    </member>
    <member kind="function">
      <type>void</type>
      <name>L_unmark_as_pre_post_increment</name>
      <anchorfile>l__opti__functions_8h.html</anchorfile>
      <anchor>70a8f02dcd30bf31e88214d446e3a0c0</anchor>
      <arglist>(L_Oper *)</arglist>
    </member>
    <member kind="function">
      <type>int</type>
      <name>L_breakup_pre_post_inc_ops</name>
      <anchorfile>l__opti__functions_8h.html</anchorfile>
      <anchor>40aa72c7cbfeedb2b4b95fd511f8155d</anchor>
      <arglist>(L_Func *)</arglist>
    </member>
    <member kind="function">
      <type>int</type>
      <name>L_remove_uncombinable_pre_post_inc_ops</name>
      <anchorfile>l__opti__functions_8h.html</anchorfile>
      <anchor>aa74fc3ad78fd2827a9b1aa3c82bee06</anchor>
      <arglist>(L_Func *)</arglist>
    </member>
    <member kind="function">
      <type>int</type>
      <name>L_generate_pre_post_inc_ops</name>
      <anchorfile>l__opti__functions_8h.html</anchorfile>
      <anchor>24bfee793bdffd7218201ce2ad58ecf1</anchor>
      <arglist>(L_Func *)</arglist>
    </member>
    <member kind="function">
      <type>void</type>
      <name>L_unmark_all_pre_post_increments_for_ind</name>
      <anchorfile>l__opti__functions_8h.html</anchorfile>
      <anchor>600ac4327033ef177c6b85335485507d</anchor>
      <arglist>(L_Loop *, int *, int, L_Operand *)</arglist>
    </member>
    <member kind="function">
      <type>void</type>
      <name>L_unmark_all_pre_post_increments_for_operand</name>
      <anchorfile>l__opti__functions_8h.html</anchorfile>
      <anchor>ca25e7c89eb6b71c460ba75686f2321b</anchor>
      <arglist>(L_Func *, L_Operand *)</arglist>
    </member>
    <member kind="function">
      <type>void</type>
      <name>L_unmark_all_pre_post_increments</name>
      <anchorfile>l__opti__functions_8h.html</anchorfile>
      <anchor>725a00c7f9ee6466d4f9ed88b817b05c</anchor>
      <arglist>(L_Func *)</arglist>
    </member>
    <member kind="function">
      <type>int</type>
      <name>L_num_reg_used</name>
      <anchorfile>l__opti__functions_8h.html</anchorfile>
      <anchor>f3b2d7330855ccf355fe4ea937fc24be</anchor>
      <arglist>(int)</arglist>
    </member>
    <member kind="function">
      <type>void</type>
      <name>L_reset_reg_count</name>
      <anchorfile>l__opti__functions_8h.html</anchorfile>
      <anchor>d390f44373c6a73d2e973020fd32fd10</anchor>
      <arglist>(L_Reg_Count *)</arglist>
    </member>
    <member kind="function">
      <type>void</type>
      <name>L_increment_reg_count</name>
      <anchorfile>l__opti__functions_8h.html</anchorfile>
      <anchor>19a6aa69f773291b495346560754674c</anchor>
      <arglist>(int, L_Reg_Count *)</arglist>
    </member>
    <member kind="function">
      <type>void</type>
      <name>L_update_reg_count</name>
      <anchorfile>l__opti__functions_8h.html</anchorfile>
      <anchor>eb68e0570755a131d802b75ee5d115f5</anchor>
      <arglist>(L_Reg_Count *, L_Reg_Count *)</arglist>
    </member>
    <member kind="function">
      <type>void</type>
      <name>L_reset_ctype_array</name>
      <anchorfile>l__opti__functions_8h.html</anchorfile>
      <anchor>33a42abfb1462c76b5a9885d41595aa9</anchor>
      <arglist>()</arglist>
    </member>
    <member kind="function">
      <type>void</type>
      <name>L_setup_ctype_array</name>
      <anchorfile>l__opti__functions_8h.html</anchorfile>
      <anchor>7ceb932286d62d72d1055f0846f5b6d3</anchor>
      <arglist>(L_Func *)</arglist>
    </member>
    <member kind="function">
      <type>void</type>
      <name>L_estimate_num_live_regs_in_cb</name>
      <anchorfile>l__opti__functions_8h.html</anchorfile>
      <anchor>a91848aa42dbb76a9931eef23d9152dc</anchor>
      <arglist>(L_Cb *)</arglist>
    </member>
    <member kind="function">
      <type>void</type>
      <name>L_estimate_num_live_regs_in_loop</name>
      <anchorfile>l__opti__functions_8h.html</anchorfile>
      <anchor>38554c313d82d75d020a331e30952925</anchor>
      <arglist>(L_Loop *, int *, int)</arglist>
    </member>
    <member kind="function">
      <type>void</type>
      <name>L_estimate_num_live_regs_in_func</name>
      <anchorfile>l__opti__functions_8h.html</anchorfile>
      <anchor>4c3e033d201497ec55f9e250e7e46c29</anchor>
      <arglist>(L_Func *)</arglist>
    </member>
    <member kind="function">
      <type>void</type>
      <name>L_set_function_mask_flag</name>
      <anchorfile>l__opti__functions_8h.html</anchorfile>
      <anchor>543f01adbde4647f6e047a09b66b0842</anchor>
      <arglist>(L_Func *)</arglist>
    </member>
    <member kind="function">
      <type>void</type>
      <name>L_mark_superblocks</name>
      <anchorfile>l__opti__functions_8h.html</anchorfile>
      <anchor>555144a7101385fe6c1fede76e14be8b</anchor>
      <arglist>(L_Func *)</arglist>
    </member>
    <member kind="function">
      <type>L_Oper *</type>
      <name>L_find_pred_definition</name>
      <anchorfile>l__opti__functions_8h.html</anchorfile>
      <anchor>57536dea702d556c72b306105a8aab5b</anchor>
      <arglist>(L_Oper *oper)</arglist>
    </member>
    <member kind="function">
      <type>void</type>
      <name>L_rename_subsequent_uses</name>
      <anchorfile>l__opti__functions_8h.html</anchorfile>
      <anchor>66d46db53af7f9522b573cbfceb41f24</anchor>
      <arglist>(L_Oper *ren_oper, L_Oper *def_oper, L_Operand *dest_reg, L_Operand *new_reg)</arglist>
    </member>
  </compound>
  <compound kind="file">
    <name>l_opti_parms.c</name>
    <path>/mnt/hgfs/mkt/Desktop/openimpact/src/Lcode/opti/Lopti/</path>
    <filename>l__opti__parms_8c</filename>
    <includes id="l__opti_8h" name="l_opti.h" local="yes" imported="no">l_opti.h</includes>
    <member kind="function">
      <type>void</type>
      <name>L_read_parm_lopti</name>
      <anchorfile>l__opti__parms_8c.html</anchorfile>
      <anchor>7d9152fbe6a66b41aabc4bea5ea7c1a5</anchor>
      <arglist>(Parm_Parse_Info *ppi)</arglist>
    </member>
    <member kind="function">
      <type>void</type>
      <name>Lopti_init</name>
      <anchorfile>l__opti__parms_8c.html</anchorfile>
      <anchor>9beda43a41b15a7f35a71bacc8f780b4</anchor>
      <arglist>()</arglist>
    </member>
    <member kind="function">
      <type>void</type>
      <name>Lopti_deinit</name>
      <anchorfile>l__opti__parms_8c.html</anchorfile>
      <anchor>2a25ade4b368852c90f2151c5e40c1d1</anchor>
      <arglist>()</arglist>
    </member>
    <member kind="variable">
      <type>int</type>
      <name>Lopti_opti_level</name>
      <anchorfile>l__opti__parms_8c.html</anchorfile>
      <anchor>b6cc9f1f9ef13936f727f3d7790bf08d</anchor>
      <arglist></arglist>
    </member>
    <member kind="variable">
      <type>int</type>
      <name>Lopti_do_benchmark_specific_opti</name>
      <anchorfile>l__opti__parms_8c.html</anchorfile>
      <anchor>d6ba2b78336cd55bbed471d8974453bb</anchor>
      <arglist></arglist>
    </member>
    <member kind="variable">
      <type>int</type>
      <name>Lopti_do_local_opti</name>
      <anchorfile>l__opti__parms_8c.html</anchorfile>
      <anchor>48a43f36d3cfc8cd0e64d2d9e27a6e1e</anchor>
      <arglist></arglist>
    </member>
    <member kind="variable">
      <type>int</type>
      <name>Lopti_do_local_constant_prop</name>
      <anchorfile>l__opti__parms_8c.html</anchorfile>
      <anchor>2b51b32868b675238ed3d360dec5fdba</anchor>
      <arglist></arglist>
    </member>
    <member kind="variable">
      <type>int</type>
      <name>Lopti_do_local_copy_prop</name>
      <anchorfile>l__opti__parms_8c.html</anchorfile>
      <anchor>f38dc34103b85690393a088b07d43384</anchor>
      <arglist></arglist>
    </member>
    <member kind="variable">
      <type>int</type>
      <name>Lopti_do_local_rev_copy_prop</name>
      <anchorfile>l__opti__parms_8c.html</anchorfile>
      <anchor>6fe39968d8193a76a1ca570a38d84b9b</anchor>
      <arglist></arglist>
    </member>
    <member kind="variable">
      <type>int</type>
      <name>Lopti_do_local_common_sub_elim</name>
      <anchorfile>l__opti__parms_8c.html</anchorfile>
      <anchor>f81e0580b790b1450f2dde1e444fe1b5</anchor>
      <arglist></arglist>
    </member>
    <member kind="variable">
      <type>int</type>
      <name>Lopti_do_local_mem_copy_prop</name>
      <anchorfile>l__opti__parms_8c.html</anchorfile>
      <anchor>f6f81dbc0283828adc73b53ca4cb1323</anchor>
      <arglist></arglist>
    </member>
    <member kind="variable">
      <type>int</type>
      <name>Lopti_do_local_red_load_elim</name>
      <anchorfile>l__opti__parms_8c.html</anchorfile>
      <anchor>f171355ce8f8e2eb09c6718c726a4ec7</anchor>
      <arglist></arglist>
    </member>
    <member kind="variable">
      <type>int</type>
      <name>Lopti_do_local_red_store_elim</name>
      <anchorfile>l__opti__parms_8c.html</anchorfile>
      <anchor>f4e47f0dd49b173f20efa49dd7e93e18</anchor>
      <arglist></arglist>
    </member>
    <member kind="variable">
      <type>int</type>
      <name>Lopti_do_local_constant_comb</name>
      <anchorfile>l__opti__parms_8c.html</anchorfile>
      <anchor>e32ef97291fadacab40ad11eed6da241</anchor>
      <arglist></arglist>
    </member>
    <member kind="variable">
      <type>int</type>
      <name>Lopti_do_local_constant_fold</name>
      <anchorfile>l__opti__parms_8c.html</anchorfile>
      <anchor>b4ac721c899caa505b82ffc2bba45a1c</anchor>
      <arglist></arglist>
    </member>
    <member kind="variable">
      <type>int</type>
      <name>Lopti_do_local_strength_red</name>
      <anchorfile>l__opti__parms_8c.html</anchorfile>
      <anchor>9c4344b7fd1a5dc55c28defb44047cfa</anchor>
      <arglist></arglist>
    </member>
    <member kind="variable">
      <type>int</type>
      <name>Lopti_do_local_strength_red_for_signed_div_rem</name>
      <anchorfile>l__opti__parms_8c.html</anchorfile>
      <anchor>e360ddddf64065625d518817167cc06f</anchor>
      <arglist></arglist>
    </member>
    <member kind="variable">
      <type>int</type>
      <name>Lopti_do_local_operation_fold</name>
      <anchorfile>l__opti__parms_8c.html</anchorfile>
      <anchor>1436bf975849f81a0693dfb14d717831</anchor>
      <arglist></arglist>
    </member>
    <member kind="variable">
      <type>int</type>
      <name>Lopti_do_local_branch_fold</name>
      <anchorfile>l__opti__parms_8c.html</anchorfile>
      <anchor>6b071820ac532884d6e3f2e5eee338c8</anchor>
      <arglist></arglist>
    </member>
    <member kind="variable">
      <type>int</type>
      <name>Lopti_do_local_operation_cancel</name>
      <anchorfile>l__opti__parms_8c.html</anchorfile>
      <anchor>c55554d83aef0da021c3d4c12e1ab037</anchor>
      <arglist></arglist>
    </member>
    <member kind="variable">
      <type>int</type>
      <name>Lopti_do_local_dead_code_rem</name>
      <anchorfile>l__opti__parms_8c.html</anchorfile>
      <anchor>bd8bf8a9cb0fca9c8f0808375a789635</anchor>
      <arglist></arglist>
    </member>
    <member kind="variable">
      <type>int</type>
      <name>Lopti_do_local_code_motion</name>
      <anchorfile>l__opti__parms_8c.html</anchorfile>
      <anchor>75b1c0e287fc91a8d1b4ba5871961fce</anchor>
      <arglist></arglist>
    </member>
    <member kind="variable">
      <type>int</type>
      <name>Lopti_do_local_remove_sign_ext</name>
      <anchorfile>l__opti__parms_8c.html</anchorfile>
      <anchor>52016f510018711b0149ad596639b375</anchor>
      <arglist></arglist>
    </member>
    <member kind="variable">
      <type>int</type>
      <name>Lopti_do_local_reduce_logic</name>
      <anchorfile>l__opti__parms_8c.html</anchorfile>
      <anchor>13baf9289de9e30d52bafd075d2cb5fc</anchor>
      <arglist></arglist>
    </member>
    <member kind="variable">
      <type>int</type>
      <name>Lopti_do_local_register_rename</name>
      <anchorfile>l__opti__parms_8c.html</anchorfile>
      <anchor>875a17dac0636f866d1fbb703ff12941</anchor>
      <arglist></arglist>
    </member>
    <member kind="variable">
      <type>int</type>
      <name>Lopti_do_local_op_breakdown</name>
      <anchorfile>l__opti__parms_8c.html</anchorfile>
      <anchor>1b79505d950a7576adab063a25d4c29a</anchor>
      <arglist></arglist>
    </member>
    <member kind="variable">
      <type>int</type>
      <name>Lopti_do_local_op_recombine</name>
      <anchorfile>l__opti__parms_8c.html</anchorfile>
      <anchor>39cfbac8f2a3dc5e42dfdc15552f3ee8</anchor>
      <arglist></arglist>
    </member>
    <member kind="variable">
      <type>int</type>
      <name>Lopti_do_local_branch_val_prop</name>
      <anchorfile>l__opti__parms_8c.html</anchorfile>
      <anchor>3653a831f240dc8877e038c428c4c9e4</anchor>
      <arglist></arglist>
    </member>
    <member kind="variable">
      <type>int</type>
      <name>Lopti_do_remove_decidable_cond_branches</name>
      <anchorfile>l__opti__parms_8c.html</anchorfile>
      <anchor>c460d8f28d8f289b1fd9b2df6c82dc1e</anchor>
      <arglist></arglist>
    </member>
    <member kind="variable">
      <type>int</type>
      <name>Lopti_debug_local_opti</name>
      <anchorfile>l__opti__parms_8c.html</anchorfile>
      <anchor>d9c6cd30ffd288471563be3659ac0b8c</anchor>
      <arglist></arglist>
    </member>
    <member kind="variable">
      <type>int</type>
      <name>Lopti_ignore_sync_arcs_for_red_elim</name>
      <anchorfile>l__opti__parms_8c.html</anchorfile>
      <anchor>59c6101096c4a4756292aa8bbe618d6b</anchor>
      <arglist></arglist>
    </member>
    <member kind="variable">
      <type>int</type>
      <name>Lopti_ignore_sync_arcs_for_loop_inv_migration</name>
      <anchorfile>l__opti__parms_8c.html</anchorfile>
      <anchor>0327068e61449ece69266453b8275269</anchor>
      <arglist></arglist>
    </member>
    <member kind="variable">
      <type>int</type>
      <name>Lopti_pred_promotion_level</name>
      <anchorfile>l__opti__parms_8c.html</anchorfile>
      <anchor>5b57300fa5693338e2016d38512b3fa5</anchor>
      <arglist></arglist>
    </member>
    <member kind="variable">
      <type>int</type>
      <name>Lopti_remove_red_guards</name>
      <anchorfile>l__opti__parms_8c.html</anchorfile>
      <anchor>13d0af114a679934de5e38f21d1d75a6</anchor>
      <arglist></arglist>
    </member>
    <member kind="variable">
      <type>int</type>
      <name>Lopti_do_global_opti</name>
      <anchorfile>l__opti__parms_8c.html</anchorfile>
      <anchor>841399696e9c5ad241bafd94333a4741</anchor>
      <arglist></arglist>
    </member>
    <member kind="variable">
      <type>int</type>
      <name>Lopti_do_global_dead_code_rem</name>
      <anchorfile>l__opti__parms_8c.html</anchorfile>
      <anchor>631efafeed6e68512dd0d0514515146a</anchor>
      <arglist></arglist>
    </member>
    <member kind="variable">
      <type>int</type>
      <name>Lopti_do_global_constant_prop</name>
      <anchorfile>l__opti__parms_8c.html</anchorfile>
      <anchor>1acdda55963ba1c83b5453549039b423</anchor>
      <arglist></arglist>
    </member>
    <member kind="variable">
      <type>int</type>
      <name>Lopti_do_global_copy_prop</name>
      <anchorfile>l__opti__parms_8c.html</anchorfile>
      <anchor>aa61849b001cba768021f57ea30e0289</anchor>
      <arglist></arglist>
    </member>
    <member kind="variable">
      <type>int</type>
      <name>Lopti_do_global_common_sub_elim</name>
      <anchorfile>l__opti__parms_8c.html</anchorfile>
      <anchor>a39001fe2b34d0387773399068f69868</anchor>
      <arglist></arglist>
    </member>
    <member kind="variable">
      <type>int</type>
      <name>Lopti_do_global_mem_copy_prop</name>
      <anchorfile>l__opti__parms_8c.html</anchorfile>
      <anchor>47a778d7c55bb919b56b67964b7982f1</anchor>
      <arglist></arglist>
    </member>
    <member kind="variable">
      <type>int</type>
      <name>Lopti_do_global_red_load_elim</name>
      <anchorfile>l__opti__parms_8c.html</anchorfile>
      <anchor>30ca934a5fa4793b8c29393f6d73e42c</anchor>
      <arglist></arglist>
    </member>
    <member kind="variable">
      <type>int</type>
      <name>Lopti_do_global_red_store_elim</name>
      <anchorfile>l__opti__parms_8c.html</anchorfile>
      <anchor>eee1983d9271b950b9c6597738db726d</anchor>
      <arglist></arglist>
    </member>
    <member kind="variable">
      <type>int</type>
      <name>Lopti_do_global_elim_boolean_ops</name>
      <anchorfile>l__opti__parms_8c.html</anchorfile>
      <anchor>52bf4389d392ec7a28b5a235ff2bdd66</anchor>
      <arglist></arglist>
    </member>
    <member kind="variable">
      <type>int</type>
      <name>Lopti_do_global_branch_val_prop</name>
      <anchorfile>l__opti__parms_8c.html</anchorfile>
      <anchor>793e5499a49b804a8aea772c87c1c4e6</anchor>
      <arglist></arglist>
    </member>
    <member kind="variable">
      <type>int</type>
      <name>Lopti_do_global_dead_if_then_else_rem</name>
      <anchorfile>l__opti__parms_8c.html</anchorfile>
      <anchor>06a44c0e30eb555c845c4265f61c9cdc</anchor>
      <arglist></arglist>
    </member>
    <member kind="variable">
      <type>int</type>
      <name>Lopti_debug_global_opti</name>
      <anchorfile>l__opti__parms_8c.html</anchorfile>
      <anchor>242376cd6f559d2f95f8c3f87c7dc414</anchor>
      <arglist></arglist>
    </member>
    <member kind="variable">
      <type>int</type>
      <name>Lopti_do_memflow_opti</name>
      <anchorfile>l__opti__parms_8c.html</anchorfile>
      <anchor>134a676bfb74c50665fee8e2dd69f7d4</anchor>
      <arglist></arglist>
    </member>
    <member kind="variable">
      <type>int</type>
      <name>Lopti_do_memflow_multistore_load</name>
      <anchorfile>l__opti__parms_8c.html</anchorfile>
      <anchor>197ceca1dc001384529b5a15c8ed7b65</anchor>
      <arglist></arglist>
    </member>
    <member kind="variable">
      <type>int</type>
      <name>Lopti_memflow_bypass_load</name>
      <anchorfile>l__opti__parms_8c.html</anchorfile>
      <anchor>ed7588f0a0cae8c2cf6fc574fbc14976</anchor>
      <arglist></arglist>
    </member>
    <member kind="variable">
      <type>int</type>
      <name>Lopti_memflow_bypass_store</name>
      <anchorfile>l__opti__parms_8c.html</anchorfile>
      <anchor>1020485c9d05b77663fc706cb70dd7ea</anchor>
      <arglist></arglist>
    </member>
    <member kind="variable">
      <type>int</type>
      <name>Lopti_memflow_bypass_jsr</name>
      <anchorfile>l__opti__parms_8c.html</anchorfile>
      <anchor>56407fd544268da0f61340135718ab6b</anchor>
      <arglist></arglist>
    </member>
    <member kind="variable">
      <type>int</type>
      <name>Lopti_memflow_bypass_total</name>
      <anchorfile>l__opti__parms_8c.html</anchorfile>
      <anchor>2e971688aa06c79b74a7e01e49c77d3b</anchor>
      <arglist></arglist>
    </member>
    <member kind="variable">
      <type>int</type>
      <name>Lopti_debug_memflow</name>
      <anchorfile>l__opti__parms_8c.html</anchorfile>
      <anchor>2029c5594bb2d10992047e58ac0460fa</anchor>
      <arglist></arglist>
    </member>
    <member kind="variable">
      <type>int</type>
      <name>Lopti_do_jump_opti</name>
      <anchorfile>l__opti__parms_8c.html</anchorfile>
      <anchor>531df982924f1f3826d76d55b916ab49</anchor>
      <arglist></arglist>
    </member>
    <member kind="variable">
      <type>int</type>
      <name>Lopti_do_jump_dead_block_elim</name>
      <anchorfile>l__opti__parms_8c.html</anchorfile>
      <anchor>4d09370c333d159af706fd23679c4628</anchor>
      <arglist></arglist>
    </member>
    <member kind="variable">
      <type>int</type>
      <name>Lopti_do_jump_br_to_next_block</name>
      <anchorfile>l__opti__parms_8c.html</anchorfile>
      <anchor>2091799c89a1aee713b4fa01f6cd7716</anchor>
      <arglist></arglist>
    </member>
    <member kind="variable">
      <type>int</type>
      <name>Lopti_do_jump_br_to_same_target</name>
      <anchorfile>l__opti__parms_8c.html</anchorfile>
      <anchor>d2928595a0a4144cc2c6cfc398d7e64c</anchor>
      <arglist></arglist>
    </member>
    <member kind="variable">
      <type>int</type>
      <name>Lopti_do_jump_br_to_uncond_br</name>
      <anchorfile>l__opti__parms_8c.html</anchorfile>
      <anchor>5a9bb6e4c0bcf5d8db05ed7624d0d7d3</anchor>
      <arglist></arglist>
    </member>
    <member kind="variable">
      <type>int</type>
      <name>Lopti_do_jump_block_merge</name>
      <anchorfile>l__opti__parms_8c.html</anchorfile>
      <anchor>c14ca34a654634b078cc5c72256cb970</anchor>
      <arglist></arglist>
    </member>
    <member kind="variable">
      <type>int</type>
      <name>Lopti_do_jump_combine_labels</name>
      <anchorfile>l__opti__parms_8c.html</anchorfile>
      <anchor>5a578d07171cc6915efa0b99fb07a8f0</anchor>
      <arglist></arglist>
    </member>
    <member kind="variable">
      <type>int</type>
      <name>Lopti_do_jump_br_target_expansion</name>
      <anchorfile>l__opti__parms_8c.html</anchorfile>
      <anchor>1cf938ac9f8b9e62624d6d40aad1b0df</anchor>
      <arglist></arglist>
    </member>
    <member kind="variable">
      <type>int</type>
      <name>Lopti_do_jump_br_swap</name>
      <anchorfile>l__opti__parms_8c.html</anchorfile>
      <anchor>a74f3a03eeafdd3d6e14737c0ad9f8c4</anchor>
      <arglist></arglist>
    </member>
    <member kind="variable">
      <type>int</type>
      <name>Lopti_allow_jump_expansion_of_pcode_loops</name>
      <anchorfile>l__opti__parms_8c.html</anchorfile>
      <anchor>020db124e350174d36b5397f90db3b8a</anchor>
      <arglist></arglist>
    </member>
    <member kind="variable">
      <type>int</type>
      <name>Lopti_debug_jump_opti</name>
      <anchorfile>l__opti__parms_8c.html</anchorfile>
      <anchor>b46ebf46d4802b5736e5eaaa2a6f3ee1</anchor>
      <arglist></arglist>
    </member>
    <member kind="variable">
      <type>int</type>
      <name>Lopti_do_split_branches</name>
      <anchorfile>l__opti__parms_8c.html</anchorfile>
      <anchor>47d4b0605b5d1f51a7859077212d333c</anchor>
      <arglist></arglist>
    </member>
    <member kind="variable">
      <type>int</type>
      <name>Lopti_do_split_unification</name>
      <anchorfile>l__opti__parms_8c.html</anchorfile>
      <anchor>a0dadb5efb9f366ad85948d29fe431be</anchor>
      <arglist></arglist>
    </member>
    <member kind="variable">
      <type>int</type>
      <name>Lopti_do_merge_unification</name>
      <anchorfile>l__opti__parms_8c.html</anchorfile>
      <anchor>9eb2a748e20353df882067e88cd68ddd</anchor>
      <arglist></arglist>
    </member>
    <member kind="variable">
      <type>int</type>
      <name>Lopti_do_jrg_expansion</name>
      <anchorfile>l__opti__parms_8c.html</anchorfile>
      <anchor>4d9bbe0d5c15d5cc589281834e2071c5</anchor>
      <arglist></arglist>
    </member>
    <member kind="variable">
      <type>int</type>
      <name>Lopti_do_loop_opti</name>
      <anchorfile>l__opti__parms_8c.html</anchorfile>
      <anchor>089d8dc69a680c39f7efd73a0e795d9b</anchor>
      <arglist></arglist>
    </member>
    <member kind="variable">
      <type>int</type>
      <name>Lopti_do_loop_br_simp</name>
      <anchorfile>l__opti__parms_8c.html</anchorfile>
      <anchor>1ce2adb367119c1882d0ba416b983fcc</anchor>
      <arglist></arglist>
    </member>
    <member kind="variable">
      <type>int</type>
      <name>Lopti_do_loop_inv_code_rem</name>
      <anchorfile>l__opti__parms_8c.html</anchorfile>
      <anchor>625b24bc76728ac485f49ae758ed0952</anchor>
      <arglist></arglist>
    </member>
    <member kind="variable">
      <type>int</type>
      <name>Lopti_do_loop_global_var_mig</name>
      <anchorfile>l__opti__parms_8c.html</anchorfile>
      <anchor>015c73873925f4096e21f611aa4ec59f</anchor>
      <arglist></arglist>
    </member>
    <member kind="variable">
      <type>int</type>
      <name>Lopti_do_loop_ind_var_str_red</name>
      <anchorfile>l__opti__parms_8c.html</anchorfile>
      <anchor>6e77e5fc0ae70f85f3e0928a0cd011f2</anchor>
      <arglist></arglist>
    </member>
    <member kind="variable">
      <type>int</type>
      <name>Lopti_do_loop_ind_var_reinit</name>
      <anchorfile>l__opti__parms_8c.html</anchorfile>
      <anchor>268270120bf5bd588e8ba1d7b48ba091</anchor>
      <arglist></arglist>
    </member>
    <member kind="variable">
      <type>int</type>
      <name>Lopti_do_loop_ind_var_elim</name>
      <anchorfile>l__opti__parms_8c.html</anchorfile>
      <anchor>8ef93ea2eb3acaaf3b24d0df6ff43ac0</anchor>
      <arglist></arglist>
    </member>
    <member kind="variable">
      <type>int</type>
      <name>Lopti_do_dead_loop_rem</name>
      <anchorfile>l__opti__parms_8c.html</anchorfile>
      <anchor>8b1f44b3dc6c05c71be488a8baa4681d</anchor>
      <arglist></arglist>
    </member>
    <member kind="variable">
      <type>int</type>
      <name>Lopti_preserve_loop_var</name>
      <anchorfile>l__opti__parms_8c.html</anchorfile>
      <anchor>b9613b0eb6c236013f39942919f80063</anchor>
      <arglist></arglist>
    </member>
    <member kind="variable">
      <type>int</type>
      <name>Lopti_do_complex_ind_elim</name>
      <anchorfile>l__opti__parms_8c.html</anchorfile>
      <anchor>86474fd45132fbef3d46036fed7e7897</anchor>
      <arglist></arglist>
    </member>
    <member kind="variable">
      <type>int</type>
      <name>Lopti_debug_loop_opti</name>
      <anchorfile>l__opti__parms_8c.html</anchorfile>
      <anchor>7832e63d33b2b7f45c19be2dd128a2d9</anchor>
      <arglist></arglist>
    </member>
    <member kind="variable">
      <type>int</type>
      <name>Lopti_do_longword_loop_opti</name>
      <anchorfile>l__opti__parms_8c.html</anchorfile>
      <anchor>dcdf1f55b373780e5e07a5bab4d10f08</anchor>
      <arglist></arglist>
    </member>
    <member kind="variable">
      <type>int</type>
      <name>Lopti_store_migration_mode</name>
      <anchorfile>l__opti__parms_8c.html</anchorfile>
      <anchor>7878f82b9eaa623ce66ca745e1a2404c</anchor>
      <arglist></arglist>
    </member>
    <member kind="variable">
      <type>int</type>
      <name>Lopti_do_PCE</name>
      <anchorfile>l__opti__parms_8c.html</anchorfile>
      <anchor>fe4e51d9023d243c2e1c31d8d6575fbc</anchor>
      <arglist></arglist>
    </member>
    <member kind="variable">
      <type>int</type>
      <name>Lopti_do_PCE_split_critical_edges</name>
      <anchorfile>l__opti__parms_8c.html</anchorfile>
      <anchor>2c3b9d064c6e75820298af27953a5857</anchor>
      <arglist></arglist>
    </member>
    <member kind="variable">
      <type>int</type>
      <name>Lopti_do_PCE_merge_same_cbs</name>
      <anchorfile>l__opti__parms_8c.html</anchorfile>
      <anchor>1013b89fd1870b0eea8726ffdd78065f</anchor>
      <arglist></arglist>
    </member>
    <member kind="variable">
      <type>int</type>
      <name>Lopti_do_PCE_optimize_memory_ops</name>
      <anchorfile>l__opti__parms_8c.html</anchorfile>
      <anchor>8ec89bd2ff7a5c70f21252e9d9edf1d4</anchor>
      <arglist></arglist>
    </member>
    <member kind="variable">
      <type>int</type>
      <name>Lopti_do_PCE_conservative_memory_opti</name>
      <anchorfile>l__opti__parms_8c.html</anchorfile>
      <anchor>ae8b125df9a1be974c23fc26f1a5402e</anchor>
      <arglist></arglist>
    </member>
    <member kind="variable">
      <type>int</type>
      <name>Lopti_do_PRE</name>
      <anchorfile>l__opti__parms_8c.html</anchorfile>
      <anchor>277f271d95b9bf6c3bfdb872181da36a</anchor>
      <arglist></arglist>
    </member>
    <member kind="variable">
      <type>int</type>
      <name>Lopti_do_PRE_lazy_code_motion</name>
      <anchorfile>l__opti__parms_8c.html</anchorfile>
      <anchor>b0a966586b823d137e945a7cc2699374</anchor>
      <arglist></arglist>
    </member>
    <member kind="variable">
      <type>int</type>
      <name>Lopti_do_PRE_mem_copy_prop</name>
      <anchorfile>l__opti__parms_8c.html</anchorfile>
      <anchor>c3067bb34faeec459cabb8bf75025a03</anchor>
      <arglist></arglist>
    </member>
    <member kind="variable">
      <type>int</type>
      <name>Lopti_do_PRE_merge_loads_diff_types</name>
      <anchorfile>l__opti__parms_8c.html</anchorfile>
      <anchor>2a5196deb17548cd00789f0ea768d0a8</anchor>
      <arglist></arglist>
    </member>
    <member kind="variable">
      <type>int</type>
      <name>Lopti_do_PRE_cutset_metric</name>
      <anchorfile>l__opti__parms_8c.html</anchorfile>
      <anchor>7f1a8514d0194590064f3652019cea2d</anchor>
      <arglist></arglist>
    </member>
    <member kind="variable">
      <type>int</type>
      <name>Lopti_do_PRE_optimize_moves_of_numerical_constants</name>
      <anchorfile>l__opti__parms_8c.html</anchorfile>
      <anchor>ae5535bb2adfcef99fd47ca2769f688f</anchor>
      <arglist></arglist>
    </member>
    <member kind="variable">
      <type>int</type>
      <name>Lopti_do_PRE_optimize_moves</name>
      <anchorfile>l__opti__parms_8c.html</anchorfile>
      <anchor>f1d28513396bb4f0ccf8a61ba3ce04ef</anchor>
      <arglist></arglist>
    </member>
    <member kind="variable">
      <type>int</type>
      <name>Lopti_do_PRE_optimize_single_source_ops</name>
      <anchorfile>l__opti__parms_8c.html</anchorfile>
      <anchor>bcca784c5de65c67d571d3a264ac7d81</anchor>
      <arglist></arglist>
    </member>
    <member kind="variable">
      <type>int</type>
      <name>Lopti_do_PRE_speculative_code_motion</name>
      <anchorfile>l__opti__parms_8c.html</anchorfile>
      <anchor>4fc5e5f9579403732754ca1a040074f7</anchor>
      <arglist></arglist>
    </member>
    <member kind="variable">
      <type>int</type>
      <name>Lopti_do_global_mem_expression_copy_prop</name>
      <anchorfile>l__opti__parms_8c.html</anchorfile>
      <anchor>8523ea54a17ff479811bf931d5bf8c3a</anchor>
      <arglist></arglist>
    </member>
    <member kind="variable">
      <type>int</type>
      <name>Lopti_do_PDE</name>
      <anchorfile>l__opti__parms_8c.html</anchorfile>
      <anchor>6ad9aa74dc755fae5ee9708852ebfc33</anchor>
      <arglist></arglist>
    </member>
    <member kind="variable">
      <type>int</type>
      <name>Lopti_do_PDE_cutset_metric</name>
      <anchorfile>l__opti__parms_8c.html</anchorfile>
      <anchor>d3e3477a2b2eb1d2ff026ff57b98fcb9</anchor>
      <arglist></arglist>
    </member>
    <member kind="variable">
      <type>int</type>
      <name>Lopti_do_PDE_min_cut</name>
      <anchorfile>l__opti__parms_8c.html</anchorfile>
      <anchor>e4b18668df73df7ac27fcc0fe912c61b</anchor>
      <arglist></arglist>
    </member>
    <member kind="variable">
      <type>int</type>
      <name>Lopti_do_PDE_predicated</name>
      <anchorfile>l__opti__parms_8c.html</anchorfile>
      <anchor>1b129deae2ea43566e529fe8603d763b</anchor>
      <arglist></arglist>
    </member>
    <member kind="variable">
      <type>int</type>
      <name>Lopti_do_PDE_sink_stores</name>
      <anchorfile>l__opti__parms_8c.html</anchorfile>
      <anchor>09958c1780d13eb1b644120e25d33bed</anchor>
      <arglist></arglist>
    </member>
    <member kind="variable">
      <type>int</type>
      <name>Lopti_do_PDE_sink_only_stores</name>
      <anchorfile>l__opti__parms_8c.html</anchorfile>
      <anchor>427787974c27b187ffb8402317ea5e6f</anchor>
      <arglist></arglist>
    </member>
    <member kind="variable">
      <type>int</type>
      <name>Lopti_do_global_dead_store_removal</name>
      <anchorfile>l__opti__parms_8c.html</anchorfile>
      <anchor>6b4fdbd536b408404869741eefb2d9ee</anchor>
      <arglist></arglist>
    </member>
    <member kind="variable">
      <type>int</type>
      <name>Lopti_do_dead_local_var_store_removal</name>
      <anchorfile>l__opti__parms_8c.html</anchorfile>
      <anchor>43e3212be05265309d78781d49e289b7</anchor>
      <arglist></arglist>
    </member>
    <member kind="variable">
      <type>int</type>
      <name>Lopti_only_lvl1_for_zero_weight_fn</name>
      <anchorfile>l__opti__parms_8c.html</anchorfile>
      <anchor>61d4df5f9daa29e72b1b5e65a93370f6</anchor>
      <arglist></arglist>
    </member>
    <member kind="variable">
      <type>int</type>
      <name>Lopti_do_post_inc_conv</name>
      <anchorfile>l__opti__parms_8c.html</anchorfile>
      <anchor>8f8bb28200aabfc716915ffd62bbbe69</anchor>
      <arglist></arglist>
    </member>
    <member kind="variable">
      <type>int</type>
      <name>Lopti_do_mark_memory_labels</name>
      <anchorfile>l__opti__parms_8c.html</anchorfile>
      <anchor>4467a3f6a41c843ce83ed639cc366d97</anchor>
      <arglist></arglist>
    </member>
    <member kind="variable">
      <type>int</type>
      <name>Lopti_do_mark_incoming_parms</name>
      <anchorfile>l__opti__parms_8c.html</anchorfile>
      <anchor>a8f6a948f910c94e7faadfaa7ca01101</anchor>
      <arglist></arglist>
    </member>
    <member kind="variable">
      <type>int</type>
      <name>Lopti_do_mark_trivial_sef_jsrs</name>
      <anchorfile>l__opti__parms_8c.html</anchorfile>
      <anchor>851d2ad4b326c8a7b561d1bdefd330d1</anchor>
      <arglist></arglist>
    </member>
    <member kind="variable">
      <type>int</type>
      <name>Lopti_do_mark_sync_jsrs</name>
      <anchorfile>l__opti__parms_8c.html</anchorfile>
      <anchor>90d1e97994303430b1e59a64003adff9</anchor>
      <arglist></arglist>
    </member>
    <member kind="variable">
      <type>int</type>
      <name>Lopti_do_mark_trivial_safe_ops</name>
      <anchorfile>l__opti__parms_8c.html</anchorfile>
      <anchor>0efbb3fcaf198a4b835438a04b329da7</anchor>
      <arglist></arglist>
    </member>
    <member kind="variable">
      <type>int</type>
      <name>Lopti_do_code_layout</name>
      <anchorfile>l__opti__parms_8c.html</anchorfile>
      <anchor>59fa9dbcf7a82ace2e3affa0c393de5b</anchor>
      <arglist></arglist>
    </member>
    <member kind="variable">
      <type>int</type>
      <name>Lopti_do_classify_branches</name>
      <anchorfile>l__opti__parms_8c.html</anchorfile>
      <anchor>620bfa0179d368afb575233b175a098e</anchor>
      <arglist></arglist>
    </member>
    <member kind="variable">
      <type>int</type>
      <name>Lopti_print_opti_count</name>
      <anchorfile>l__opti__parms_8c.html</anchorfile>
      <anchor>3acc753376ac648863c45ffc6d5c63fb</anchor>
      <arglist></arglist>
    </member>
    <member kind="variable">
      <type>int</type>
      <name>Lopti_print_opti_breakdown</name>
      <anchorfile>l__opti__parms_8c.html</anchorfile>
      <anchor>3532b96f82647148be3cc6c9e799d93e</anchor>
      <arglist></arglist>
    </member>
    <member kind="variable">
      <type>L_Alloc_Pool *</type>
      <name>L_alloc_danger_ext</name>
      <anchorfile>l__opti__parms_8c.html</anchorfile>
      <anchor>cc6c274b5e4f63b0b0ceaf53b0be780e</anchor>
      <arglist></arglist>
    </member>
    <member kind="variable">
      <type>int *</type>
      <name>Lopti_ctype_array</name>
      <anchorfile>l__opti__parms_8c.html</anchorfile>
      <anchor>a388f4015f2d1c31fe4f4c0be7e523d2</anchor>
      <arglist></arglist>
    </member>
    <member kind="variable">
      <type>int</type>
      <name>Lopti_ctype_array_size</name>
      <anchorfile>l__opti__parms_8c.html</anchorfile>
      <anchor>33e0aa4c3bec083f608cf4aebeb80822</anchor>
      <arglist></arglist>
    </member>
    <member kind="variable">
      <type>int</type>
      <name>Lopti_ctype_max_reg</name>
      <anchorfile>l__opti__parms_8c.html</anchorfile>
      <anchor>bc627a3c9a90bf8f16a190f90f1665a4</anchor>
      <arglist></arglist>
    </member>
    <member kind="variable">
      <type>int</type>
      <name>Lopti_cnt_local_opti</name>
      <anchorfile>l__opti__parms_8c.html</anchorfile>
      <anchor>f78fd58926406ffd1d8521ce2d25cc92</anchor>
      <arglist></arglist>
    </member>
    <member kind="variable">
      <type>int</type>
      <name>Lopti_cnt_local_constant_prop</name>
      <anchorfile>l__opti__parms_8c.html</anchorfile>
      <anchor>5aca3bfee0de8dae097d77d01941b618</anchor>
      <arglist></arglist>
    </member>
    <member kind="variable">
      <type>int</type>
      <name>Lopti_cnt_local_copy_prop</name>
      <anchorfile>l__opti__parms_8c.html</anchorfile>
      <anchor>5c98b89c4f6568611d89cf4f9b2aa2b9</anchor>
      <arglist></arglist>
    </member>
    <member kind="variable">
      <type>int</type>
      <name>Lopti_cnt_local_rev_copy_prop</name>
      <anchorfile>l__opti__parms_8c.html</anchorfile>
      <anchor>6fe06f00c602c87a1b8853fdb740b2ea</anchor>
      <arglist></arglist>
    </member>
    <member kind="variable">
      <type>int</type>
      <name>Lopti_cnt_local_mem_copy_prop</name>
      <anchorfile>l__opti__parms_8c.html</anchorfile>
      <anchor>f78ee891827d2c4395cf4fd70e9298ac</anchor>
      <arglist></arglist>
    </member>
    <member kind="variable">
      <type>int</type>
      <name>Lopti_cnt_local_common_sub_elim</name>
      <anchorfile>l__opti__parms_8c.html</anchorfile>
      <anchor>d30a316c82cb4d99d070713c20c76cdb</anchor>
      <arglist></arglist>
    </member>
    <member kind="variable">
      <type>int</type>
      <name>Lopti_cnt_local_red_load_elim</name>
      <anchorfile>l__opti__parms_8c.html</anchorfile>
      <anchor>59de8e8cc71c5dc9d495be678e502ece</anchor>
      <arglist></arglist>
    </member>
    <member kind="variable">
      <type>int</type>
      <name>Lopti_cnt_local_red_store_elim</name>
      <anchorfile>l__opti__parms_8c.html</anchorfile>
      <anchor>15be1073f55f1aa9cdfb1e81f57044b1</anchor>
      <arglist></arglist>
    </member>
    <member kind="variable">
      <type>int</type>
      <name>Lopti_cnt_local_constant_fold</name>
      <anchorfile>l__opti__parms_8c.html</anchorfile>
      <anchor>17b600d1763b9bedd53311de308aab52</anchor>
      <arglist></arglist>
    </member>
    <member kind="variable">
      <type>int</type>
      <name>Lopti_cnt_local_strength_red</name>
      <anchorfile>l__opti__parms_8c.html</anchorfile>
      <anchor>60b336a1fb70d652314036cee9ba781b</anchor>
      <arglist></arglist>
    </member>
    <member kind="variable">
      <type>int</type>
      <name>Lopti_cnt_local_constant_comb</name>
      <anchorfile>l__opti__parms_8c.html</anchorfile>
      <anchor>6d7f2d2acb7fd344a5d26d94a77d1bfb</anchor>
      <arglist></arglist>
    </member>
    <member kind="variable">
      <type>int</type>
      <name>Lopti_cnt_local_operation_fold</name>
      <anchorfile>l__opti__parms_8c.html</anchorfile>
      <anchor>c948f4cbf3efc503222f24bcc8a8fd93</anchor>
      <arglist></arglist>
    </member>
    <member kind="variable">
      <type>int</type>
      <name>Lopti_cnt_local_branch_fold</name>
      <anchorfile>l__opti__parms_8c.html</anchorfile>
      <anchor>0ac99815a2aaea80c22e2d149a510121</anchor>
      <arglist></arglist>
    </member>
    <member kind="variable">
      <type>int</type>
      <name>Lopti_cnt_local_operation_cancel</name>
      <anchorfile>l__opti__parms_8c.html</anchorfile>
      <anchor>8c84c3c7235c2dbdb5dce63ad1f1a712</anchor>
      <arglist></arglist>
    </member>
    <member kind="variable">
      <type>int</type>
      <name>Lopti_cnt_local_dead_code_rem</name>
      <anchorfile>l__opti__parms_8c.html</anchorfile>
      <anchor>3fa298047ca082c9db37718ac1e0444d</anchor>
      <arglist></arglist>
    </member>
    <member kind="variable">
      <type>int</type>
      <name>Lopti_cnt_local_code_motion</name>
      <anchorfile>l__opti__parms_8c.html</anchorfile>
      <anchor>5d95e9ed875f732a910cdccc65d57a85</anchor>
      <arglist></arglist>
    </member>
    <member kind="variable">
      <type>int</type>
      <name>Lopti_cnt_local_remove_sign_ext</name>
      <anchorfile>l__opti__parms_8c.html</anchorfile>
      <anchor>593af5042b5662d6c278eebe4646493a</anchor>
      <arglist></arglist>
    </member>
    <member kind="variable">
      <type>int</type>
      <name>Lopti_cnt_local_reduce_logic</name>
      <anchorfile>l__opti__parms_8c.html</anchorfile>
      <anchor>2b58cf2a4f2233991eecf178ac64e3e3</anchor>
      <arglist></arglist>
    </member>
    <member kind="variable">
      <type>int</type>
      <name>Lopti_cnt_local_register_rename</name>
      <anchorfile>l__opti__parms_8c.html</anchorfile>
      <anchor>13832534d2e6012014e36bdd089d5535</anchor>
      <arglist></arglist>
    </member>
    <member kind="variable">
      <type>int</type>
      <name>Lopti_cnt_global_opti</name>
      <anchorfile>l__opti__parms_8c.html</anchorfile>
      <anchor>f9ff5dabd7f0ef90058d2cdac5c0aba1</anchor>
      <arglist></arglist>
    </member>
    <member kind="variable">
      <type>int</type>
      <name>Lopti_cnt_global_dead_code_rem</name>
      <anchorfile>l__opti__parms_8c.html</anchorfile>
      <anchor>0d45f69f2768e2014dfcf77e97840005</anchor>
      <arglist></arglist>
    </member>
    <member kind="variable">
      <type>int</type>
      <name>Lopti_cnt_global_constant_prop</name>
      <anchorfile>l__opti__parms_8c.html</anchorfile>
      <anchor>8621757a8fdc40bea064cbc4cc95a0f8</anchor>
      <arglist></arglist>
    </member>
    <member kind="variable">
      <type>int</type>
      <name>Lopti_cnt_global_copy_prop</name>
      <anchorfile>l__opti__parms_8c.html</anchorfile>
      <anchor>a7b11a41270f176603986ce6044b75b5</anchor>
      <arglist></arglist>
    </member>
    <member kind="variable">
      <type>int</type>
      <name>Lopti_cnt_global_mem_copy_prop</name>
      <anchorfile>l__opti__parms_8c.html</anchorfile>
      <anchor>1c93aea77aba62f587a3d53613f6b6f8</anchor>
      <arglist></arglist>
    </member>
    <member kind="variable">
      <type>int</type>
      <name>Lopti_cnt_global_common_sub_elim</name>
      <anchorfile>l__opti__parms_8c.html</anchorfile>
      <anchor>055e8cc717bd60b3dbcb77a4c2cac817</anchor>
      <arglist></arglist>
    </member>
    <member kind="variable">
      <type>int</type>
      <name>Lopti_cnt_global_red_load_elim</name>
      <anchorfile>l__opti__parms_8c.html</anchorfile>
      <anchor>fe1252352ceaef605d21ae21268e0f49</anchor>
      <arglist></arglist>
    </member>
    <member kind="variable">
      <type>int</type>
      <name>Lopti_cnt_global_red_store_elim</name>
      <anchorfile>l__opti__parms_8c.html</anchorfile>
      <anchor>fa6c19336f7b640c6ea91ccbf3fed5cf</anchor>
      <arglist></arglist>
    </member>
    <member kind="variable">
      <type>int</type>
      <name>Lopti_cnt_global_elim_boolean_ops</name>
      <anchorfile>l__opti__parms_8c.html</anchorfile>
      <anchor>c9209b77ce2ae36391542e497297a6e3</anchor>
      <arglist></arglist>
    </member>
    <member kind="variable">
      <type>int</type>
      <name>Lopti_cnt_global_dead_if_then_else_rem</name>
      <anchorfile>l__opti__parms_8c.html</anchorfile>
      <anchor>d10d5deb47779848cf437ef157eb1eca</anchor>
      <arglist></arglist>
    </member>
    <member kind="variable">
      <type>int</type>
      <name>Lopti_cnt_memflow_multistore_load</name>
      <anchorfile>l__opti__parms_8c.html</anchorfile>
      <anchor>c2d2d6dd5de6b978bbab77ccb9fe0cd0</anchor>
      <arglist></arglist>
    </member>
    <member kind="variable">
      <type>int</type>
      <name>Lopti_cnt_jump_opti</name>
      <anchorfile>l__opti__parms_8c.html</anchorfile>
      <anchor>8cb94e323fe13961a49048adb755c43b</anchor>
      <arglist></arglist>
    </member>
    <member kind="variable">
      <type>int</type>
      <name>Lopti_cnt_jump_dead_block_elim</name>
      <anchorfile>l__opti__parms_8c.html</anchorfile>
      <anchor>5320444d1e95c1cadc3272874550225a</anchor>
      <arglist></arglist>
    </member>
    <member kind="variable">
      <type>int</type>
      <name>Lopti_cnt_jump_br_to_next_block</name>
      <anchorfile>l__opti__parms_8c.html</anchorfile>
      <anchor>1c1796e7b3c5056ca6c3795b668c30e2</anchor>
      <arglist></arglist>
    </member>
    <member kind="variable">
      <type>int</type>
      <name>Lopti_cnt_jump_br_to_same_target</name>
      <anchorfile>l__opti__parms_8c.html</anchorfile>
      <anchor>53f5ca958523321945c149a529001c87</anchor>
      <arglist></arglist>
    </member>
    <member kind="variable">
      <type>int</type>
      <name>Lopti_cnt_jump_br_to_uncond_br</name>
      <anchorfile>l__opti__parms_8c.html</anchorfile>
      <anchor>ad4703e6f398d3b04f5e95e4e50742a6</anchor>
      <arglist></arglist>
    </member>
    <member kind="variable">
      <type>int</type>
      <name>Lopti_cnt_jump_block_merge</name>
      <anchorfile>l__opti__parms_8c.html</anchorfile>
      <anchor>c130eb576413b392adb8bb59c47aac5e</anchor>
      <arglist></arglist>
    </member>
    <member kind="variable">
      <type>int</type>
      <name>Lopti_cnt_jump_combine_labels</name>
      <anchorfile>l__opti__parms_8c.html</anchorfile>
      <anchor>6cf2ad1a28b50dcacfbbabe53c342454</anchor>
      <arglist></arglist>
    </member>
    <member kind="variable">
      <type>int</type>
      <name>Lopti_cnt_jump_br_target_expansion</name>
      <anchorfile>l__opti__parms_8c.html</anchorfile>
      <anchor>a262e831d3836b068d853bdc5bde5d30</anchor>
      <arglist></arglist>
    </member>
    <member kind="variable">
      <type>int</type>
      <name>Lopti_cnt_jump_br_swap</name>
      <anchorfile>l__opti__parms_8c.html</anchorfile>
      <anchor>e6e4bf4d5bcd33b6eeafcec52a3c90c6</anchor>
      <arglist></arglist>
    </member>
    <member kind="variable">
      <type>int</type>
      <name>Lopti_cnt_loop_opti</name>
      <anchorfile>l__opti__parms_8c.html</anchorfile>
      <anchor>89cdfb3508b0a365cae6018120cd130e</anchor>
      <arglist></arglist>
    </member>
    <member kind="variable">
      <type>int</type>
      <name>Lopti_cnt_loop_br_simp</name>
      <anchorfile>l__opti__parms_8c.html</anchorfile>
      <anchor>8438a83b7217665243ad7707dc1e3a4c</anchor>
      <arglist></arglist>
    </member>
    <member kind="variable">
      <type>int</type>
      <name>Lopti_cnt_loop_inv_code_rem</name>
      <anchorfile>l__opti__parms_8c.html</anchorfile>
      <anchor>afbec35e49d6645912981e63b8f75f73</anchor>
      <arglist></arglist>
    </member>
    <member kind="variable">
      <type>int</type>
      <name>Lopti_cnt_loop_global_var_mig</name>
      <anchorfile>l__opti__parms_8c.html</anchorfile>
      <anchor>9f0ab2a9bd24e6e60dbf97ff5c32da6a</anchor>
      <arglist></arglist>
    </member>
    <member kind="variable">
      <type>int</type>
      <name>Lopti_cnt_loop_ind_var_str_red</name>
      <anchorfile>l__opti__parms_8c.html</anchorfile>
      <anchor>f3e73e64c2e421b982815f59765afd9f</anchor>
      <arglist></arglist>
    </member>
    <member kind="variable">
      <type>int</type>
      <name>Lopti_cnt_loop_ind_var_reinit</name>
      <anchorfile>l__opti__parms_8c.html</anchorfile>
      <anchor>71f1293ecb44fd991678849ecccc97f3</anchor>
      <arglist></arglist>
    </member>
    <member kind="variable">
      <type>int</type>
      <name>Lopti_cnt_loop_ind_var_elim</name>
      <anchorfile>l__opti__parms_8c.html</anchorfile>
      <anchor>b9c1423c30aac4448c045e5e018d7331</anchor>
      <arglist></arglist>
    </member>
    <member kind="variable">
      <type>int</type>
      <name>Lopti_cnt_dead_loop_rem</name>
      <anchorfile>l__opti__parms_8c.html</anchorfile>
      <anchor>959cdcafbd890248663fd15252dc8d9d</anchor>
      <arglist></arglist>
    </member>
    <member kind="variable">
      <type>int</type>
      <name>Lopti_inter_region_global_opti</name>
      <anchorfile>l__opti__parms_8c.html</anchorfile>
      <anchor>235264fca0bdf817175864fdc621fda4</anchor>
      <arglist></arglist>
    </member>
    <member kind="variable">
      <type>int</type>
      <name>Lopti_inter_region_global_dead_code_rem</name>
      <anchorfile>l__opti__parms_8c.html</anchorfile>
      <anchor>4272b446f4e75548ac96fdaf005b4185</anchor>
      <arglist></arglist>
    </member>
    <member kind="variable">
      <type>int</type>
      <name>Lopti_inter_region_global_constant_prop</name>
      <anchorfile>l__opti__parms_8c.html</anchorfile>
      <anchor>e092aa5228b67dd83dcf76407ea9ff14</anchor>
      <arglist></arglist>
    </member>
    <member kind="variable">
      <type>int</type>
      <name>Lopti_inter_region_global_copy_prop</name>
      <anchorfile>l__opti__parms_8c.html</anchorfile>
      <anchor>2aad0d4fae1ea18ec8cf91a8d9200f3e</anchor>
      <arglist></arglist>
    </member>
    <member kind="variable">
      <type>int</type>
      <name>Lopti_inter_region_global_mem_copy_prop</name>
      <anchorfile>l__opti__parms_8c.html</anchorfile>
      <anchor>75e868145a3428ce1b19e71f3669f80e</anchor>
      <arglist></arglist>
    </member>
    <member kind="variable">
      <type>int</type>
      <name>Lopti_inter_region_global_common_sub_elim</name>
      <anchorfile>l__opti__parms_8c.html</anchorfile>
      <anchor>4fc422beb94b60df03e1d5c64d769286</anchor>
      <arglist></arglist>
    </member>
    <member kind="variable">
      <type>int</type>
      <name>Lopti_inter_region_global_red_load_elim</name>
      <anchorfile>l__opti__parms_8c.html</anchorfile>
      <anchor>39f315882dfceecd7c7c14c894781e07</anchor>
      <arglist></arglist>
    </member>
    <member kind="variable">
      <type>int</type>
      <name>Lopti_inter_region_global_red_store_elim</name>
      <anchorfile>l__opti__parms_8c.html</anchorfile>
      <anchor>70f6cece59884ca2975ef076e6ee2f17</anchor>
      <arglist></arglist>
    </member>
    <member kind="variable">
      <type>int</type>
      <name>Lopti_inter_region_global_elim_boolean_ops</name>
      <anchorfile>l__opti__parms_8c.html</anchorfile>
      <anchor>4a0703bf7600acbc259401035b927aa2</anchor>
      <arglist></arglist>
    </member>
    <member kind="variable">
      <type>int</type>
      <name>Lopti_inter_region_global_dead_if_then_else_rem</name>
      <anchorfile>l__opti__parms_8c.html</anchorfile>
      <anchor>3f62d4a0b08edac2317647364a7a4f13</anchor>
      <arglist></arglist>
    </member>
    <member kind="variable">
      <type>double</type>
      <name>Lopti_inter_region_global_common_sub_elim_wgt</name>
      <anchorfile>l__opti__parms_8c.html</anchorfile>
      <anchor>06763a4f290a170bfae6cbd1d112acce</anchor>
      <arglist></arglist>
    </member>
    <member kind="variable">
      <type>double</type>
      <name>Lopti_inter_region_global_copy_prop_wgt</name>
      <anchorfile>l__opti__parms_8c.html</anchorfile>
      <anchor>b46aa21992eeda81af82f6e63af2f866</anchor>
      <arglist></arglist>
    </member>
    <member kind="variable">
      <type>double</type>
      <name>Lopti_inter_region_loop_inv_wgt</name>
      <anchorfile>l__opti__parms_8c.html</anchorfile>
      <anchor>be1b2d240545f9250220f056adf52f95</anchor>
      <arglist></arglist>
    </member>
    <member kind="variable">
      <type>double</type>
      <name>Lopti_inter_region_gvm_wgt</name>
      <anchorfile>l__opti__parms_8c.html</anchorfile>
      <anchor>509f9f7052b0bba49b66f043c5d4c47b</anchor>
      <arglist></arglist>
    </member>
    <member kind="variable">
      <type>int</type>
      <name>L_native_machine_ctype</name>
      <anchorfile>l__opti__parms_8c.html</anchorfile>
      <anchor>370445c969e7255431a032fd14d0cfe5</anchor>
      <arglist></arglist>
    </member>
  </compound>
  <compound kind="file">
    <name>l_opti_predicates.c</name>
    <path>/mnt/hgfs/mkt/Desktop/openimpact/src/Lcode/opti/Lopti/</path>
    <filename>l__opti__predicates_8c</filename>
    <includes id="l__opti_8h" name="l_opti.h" local="yes" imported="no">l_opti.h</includes>
    <class kind="union">convert_union</class>
    <class kind="union">convert_float_union</class>
    <member kind="define">
      <type>#define</type>
      <name>L_MIN_WEIGHT_FOR_OP_MIG</name>
      <anchorfile>l__opti__predicates_8c.html</anchorfile>
      <anchor>1adfc499e94fddf8f314c6c228b007ab</anchor>
      <arglist></arglist>
    </member>
    <member kind="define">
      <type>#define</type>
      <name>L_MAX_OP_MIG_COST_RATIO</name>
      <anchorfile>l__opti__predicates_8c.html</anchorfile>
      <anchor>09fcc01ecbc8f845a99e8744da456de6</anchor>
      <arglist></arglist>
    </member>
    <member kind="define">
      <type>#define</type>
      <name>L_MAX_CB_FOR_PREHEADER_MOVE</name>
      <anchorfile>l__opti__predicates_8c.html</anchorfile>
      <anchor>1d88c8e284fc30954539dad0e4b007e4</anchor>
      <arglist></arglist>
    </member>
    <member kind="define">
      <type>#define</type>
      <name>L_MIN_ITER_FOR_IND_COMPLEX_ELIM</name>
      <anchorfile>l__opti__predicates_8c.html</anchorfile>
      <anchor>3acefc87f40ed5b5be3823b2bfd520b9</anchor>
      <arglist></arglist>
    </member>
    <member kind="define">
      <type>#define</type>
      <name>L_MIN_RATIO_FOR_IND_COMPLEX_ELIM</name>
      <anchorfile>l__opti__predicates_8c.html</anchorfile>
      <anchor>0765cd69028b5dd8d4b5f5324822cd5d</anchor>
      <arglist></arglist>
    </member>
    <member kind="function">
      <type>int</type>
      <name>L_is_compatible_opc</name>
      <anchorfile>l__opti__predicates_8c.html</anchorfile>
      <anchor>e42d3b53ce482c335614a762f3d27a5b</anchor>
      <arglist>(int opc1, int opc2)</arglist>
    </member>
    <member kind="function">
      <type>int</type>
      <name>L_compatible_opcodes</name>
      <anchorfile>l__opti__predicates_8c.html</anchorfile>
      <anchor>0902283f561640d0906a164b18c19595</anchor>
      <arglist>(L_Oper *oper1, L_Oper *oper2)</arglist>
    </member>
    <member kind="function">
      <type>int</type>
      <name>L_compatible_arithmetic_ops</name>
      <anchorfile>l__opti__predicates_8c.html</anchorfile>
      <anchor>440f9c0f1f4669e3a95ecdc8a9d5a92e</anchor>
      <arglist>(L_Oper *oper1, L_Oper *oper2)</arglist>
    </member>
    <member kind="function">
      <type>int</type>
      <name>L_compatible_to_combine_consts</name>
      <anchorfile>l__opti__predicates_8c.html</anchorfile>
      <anchor>77b935a1652dc972583f454c60980dd5</anchor>
      <arglist>(L_Oper *oper1, L_Oper *oper2)</arglist>
    </member>
    <member kind="function">
      <type>int</type>
      <name>L_unary_opcode</name>
      <anchorfile>l__opti__predicates_8c.html</anchorfile>
      <anchor>e8b8c31f04f9e0a581da83b5de68c2a8</anchor>
      <arglist>(L_Oper *oper)</arglist>
    </member>
    <member kind="function">
      <type>int</type>
      <name>L_are_opposite_branch_opcodes</name>
      <anchorfile>l__opti__predicates_8c.html</anchorfile>
      <anchor>83f81533c667f461f4e4ad32cc787d39</anchor>
      <arglist>(L_Oper *oper1, L_Oper *oper2)</arglist>
    </member>
    <member kind="function">
      <type>int</type>
      <name>L_are_reverse_branch_opcodes</name>
      <anchorfile>l__opti__predicates_8c.html</anchorfile>
      <anchor>0f195aaa3070f865408fd55387b5c79f</anchor>
      <arglist>(L_Oper *oper1, L_Oper *oper2)</arglist>
    </member>
    <member kind="function">
      <type>int</type>
      <name>L_are_same_branch_opcodes</name>
      <anchorfile>l__opti__predicates_8c.html</anchorfile>
      <anchor>18b5ca4534642dad4874230a23f52ab4</anchor>
      <arglist>(L_Oper *oper1, L_Oper *oper2)</arglist>
    </member>
    <member kind="function">
      <type>int</type>
      <name>L_load_store_sign_extend_conflict</name>
      <anchorfile>l__opti__predicates_8c.html</anchorfile>
      <anchor>85777daac49e97c7f4a0d80e590f320c</anchor>
      <arglist>(L_Oper *op1, L_Oper *op2)</arglist>
    </member>
    <member kind="function">
      <type>int</type>
      <name>L_compatible_load_store</name>
      <anchorfile>l__opti__predicates_8c.html</anchorfile>
      <anchor>5f7771a4ff3915d6ce2a8fc0cb58acbb</anchor>
      <arglist>(L_Oper *op1, L_Oper *op2)</arglist>
    </member>
    <member kind="function">
      <type>int</type>
      <name>L_cancelling_opcodes</name>
      <anchorfile>l__opti__predicates_8c.html</anchorfile>
      <anchor>67b6b81957309ceab16ddedc21cd08f5</anchor>
      <arglist>(L_Oper *opA, L_Oper *opB)</arglist>
    </member>
    <member kind="function">
      <type>int</type>
      <name>L_is_legal_unsigned_value_offset_32</name>
      <anchorfile>l__opti__predicates_8c.html</anchorfile>
      <anchor>75fc8aa776abce9e6824fe8b4c270444</anchor>
      <arglist>(unsigned int value, int offset)</arglist>
    </member>
    <member kind="function">
      <type>int</type>
      <name>L_is_int_24</name>
      <anchorfile>l__opti__predicates_8c.html</anchorfile>
      <anchor>d94943cc51634dd76e355d6dacb6f244</anchor>
      <arglist>(L_Operand *operand)</arglist>
    </member>
    <member kind="function">
      <type>int</type>
      <name>L_is_int_between_0_and_128</name>
      <anchorfile>l__opti__predicates_8c.html</anchorfile>
      <anchor>a41025a0519159357cbdc02ab989747d</anchor>
      <arglist>(L_Operand *operand)</arglist>
    </member>
    <member kind="function">
      <type>int</type>
      <name>L_is_branch_target</name>
      <anchorfile>l__opti__predicates_8c.html</anchorfile>
      <anchor>52046e081317d82c6fce824e077663e0</anchor>
      <arglist>(L_Oper *oper, L_Cb *cb)</arglist>
    </member>
    <member kind="function">
      <type>int</type>
      <name>L_can_change_all_uses_between</name>
      <anchorfile>l__opti__predicates_8c.html</anchorfile>
      <anchor>b7f42e01e2e01e6b8225e830d0f4beef</anchor>
      <arglist>(L_Cb *cb, L_Operand *operand, L_Operand *replace, L_Oper *opA, L_Oper *opB)</arglist>
    </member>
    <member kind="function">
      <type>int</type>
      <name>L_can_change_all_later_uses_with</name>
      <anchorfile>l__opti__predicates_8c.html</anchorfile>
      <anchor>595ed88085091eb65ad7f6090a703df3</anchor>
      <arglist>(L_Cb *cb, L_Operand *operand, L_Operand *replace, L_Oper *op)</arglist>
    </member>
    <member kind="function">
      <type>int</type>
      <name>L_increment_operation</name>
      <anchorfile>l__opti__predicates_8c.html</anchorfile>
      <anchor>68c94883a1dac97a8cfcbfbff48c58bb</anchor>
      <arglist>(L_Oper *oper)</arglist>
    </member>
    <member kind="function">
      <type>int</type>
      <name>L_no_overlap_write</name>
      <anchorfile>l__opti__predicates_8c.html</anchorfile>
      <anchor>ae3d99ff7e4e9963845653f3e2ec75ed</anchor>
      <arglist>(L_Cb *cb, L_Oper *op, L_Oper *opA, int pathA, L_Oper *opB, int pathB)</arglist>
    </member>
    <member kind="function">
      <type>int</type>
      <name>L_no_overlap_read</name>
      <anchorfile>l__opti__predicates_8c.html</anchorfile>
      <anchor>d617ab46ed4d630a2d7308b152489474</anchor>
      <arglist>(L_Cb *cb, L_Oper *op, L_Oper *opA, int pathA, L_Oper *opB, int pathB)</arglist>
    </member>
    <member kind="function">
      <type>int</type>
      <name>L_no_sb_loop_br_between</name>
      <anchorfile>l__opti__predicates_8c.html</anchorfile>
      <anchor>db03544cbc5912b6e4637688effa40d1</anchor>
      <arglist>(L_Cb *cb, L_Oper *opA, L_Oper *opB)</arglist>
    </member>
    <member kind="function">
      <type>int</type>
      <name>L_can_merge_with_op_above</name>
      <anchorfile>l__opti__predicates_8c.html</anchorfile>
      <anchor>664e545b67f3805c873213c16329ea8c</anchor>
      <arglist>(L_Cb *cb, L_Oper *pred_op, L_Oper *merge_op)</arglist>
    </member>
    <member kind="function">
      <type>int</type>
      <name>L_can_merge_with_op_below</name>
      <anchorfile>l__opti__predicates_8c.html</anchorfile>
      <anchor>ed9d41bcb30b2a088bb12d966b29931e</anchor>
      <arglist>(L_Cb *cb, L_Oper *succ_op, L_Oper *merge_op)</arglist>
    </member>
    <member kind="function">
      <type>int</type>
      <name>L_can_move_above</name>
      <anchorfile>l__opti__predicates_8c.html</anchorfile>
      <anchor>41e40904967dd2c60c80757c90c4df70</anchor>
      <arglist>(L_Cb *cb, L_Oper *op, L_Oper *tomove_op)</arglist>
    </member>
    <member kind="function">
      <type>int</type>
      <name>L_can_move_below</name>
      <anchorfile>l__opti__predicates_8c.html</anchorfile>
      <anchor>66ff5038db3e4b9caa39fac6877ee407</anchor>
      <arglist>(L_Cb *cb, L_Oper *op, L_Oper *tomove_op)</arglist>
    </member>
    <member kind="function">
      <type>int</type>
      <name>L_can_undo_and_combine</name>
      <anchorfile>l__opti__predicates_8c.html</anchorfile>
      <anchor>29cabb32be46a37e01989464007fd923</anchor>
      <arglist>(L_Oper *op1, L_Oper *op2, L_Oper *op3)</arglist>
    </member>
    <member kind="function">
      <type>int</type>
      <name>L_can_undo_and_combine2</name>
      <anchorfile>l__opti__predicates_8c.html</anchorfile>
      <anchor>a48aa64d81e2b72f605ade471759c327</anchor>
      <arglist>(L_Oper *opA, L_Oper *opB)</arglist>
    </member>
    <member kind="function">
      <type>int</type>
      <name>L_can_make_mul_add_sub</name>
      <anchorfile>l__opti__predicates_8c.html</anchorfile>
      <anchor>6d546ecd157620921b75f4b7fcb3988a</anchor>
      <arglist>(L_Oper *opA, L_Oper *opB)</arglist>
    </member>
    <member kind="function">
      <type>int</type>
      <name>L_can_undo_increment</name>
      <anchorfile>l__opti__predicates_8c.html</anchorfile>
      <anchor>601306d98a236335b95d1a86d6165562</anchor>
      <arglist>(L_Oper *opA, L_Oper *opB)</arglist>
    </member>
    <member kind="function">
      <type>int</type>
      <name>L_no_exceptions</name>
      <anchorfile>l__opti__predicates_8c.html</anchorfile>
      <anchor>4bb6c59cae77c6bbb55efbbf3b963519</anchor>
      <arglist>(L_Oper *oper)</arglist>
    </member>
    <member kind="function">
      <type>int</type>
      <name>L_will_lose_accuracy</name>
      <anchorfile>l__opti__predicates_8c.html</anchorfile>
      <anchor>60395a55c8362ae866f7a112e3a151c4</anchor>
      <arglist>(L_Oper *oper)</arglist>
    </member>
    <member kind="function">
      <type>int</type>
      <name>L_invertible_float_constant</name>
      <anchorfile>l__opti__predicates_8c.html</anchorfile>
      <anchor>250b4aa66749f1037812ea655ced0b4a</anchor>
      <arglist>(float val)</arglist>
    </member>
    <member kind="function">
      <type>int</type>
      <name>L_invertible_constant</name>
      <anchorfile>l__opti__predicates_8c.html</anchorfile>
      <anchor>b4c1bae31c761cfd29ccdca4f977705b</anchor>
      <arglist>(double val)</arglist>
    </member>
    <member kind="function">
      <type>int</type>
      <name>L_live_outside_cb_after</name>
      <anchorfile>l__opti__predicates_8c.html</anchorfile>
      <anchor>4d7522511bcda5e2f523e6d40873768c</anchor>
      <arglist>(L_Cb *cb, L_Operand *operand, L_Oper *oper)</arglist>
    </member>
    <member kind="function">
      <type>int</type>
      <name>L_not_live_outside_cb_between</name>
      <anchorfile>l__opti__predicates_8c.html</anchorfile>
      <anchor>740bbacb8b13f3f5d164fc7c32fb7248</anchor>
      <arglist>(L_Cb *cb, L_Operand *operand, L_Oper *start, L_Oper *end)</arglist>
    </member>
    <member kind="function">
      <type>int</type>
      <name>L_all_dest_operand_not_live_outside_cb_between</name>
      <anchorfile>l__opti__predicates_8c.html</anchorfile>
      <anchor>dfb825881694d6028649aa34ec8dec5c</anchor>
      <arglist>(L_Cb *cb, L_Oper *oper, L_Oper *start, L_Oper *end)</arglist>
    </member>
    <member kind="function">
      <type>int</type>
      <name>L_all_src_operand_not_live_outside_cb_between</name>
      <anchorfile>l__opti__predicates_8c.html</anchorfile>
      <anchor>209e540470b90e77f865b5bed88c1349</anchor>
      <arglist>(L_Cb *cb, L_Oper *oper, L_Oper *start, L_Oper *end)</arglist>
    </member>
    <member kind="function">
      <type>int</type>
      <name>L_single_use_of</name>
      <anchorfile>l__opti__predicates_8c.html</anchorfile>
      <anchor>d8e41d93c3bb9656072567cf62c54adb</anchor>
      <arglist>(L_Cb *cb, L_Operand *operand, L_Oper *oper)</arglist>
    </member>
    <member kind="function">
      <type>int</type>
      <name>L_no_use_of</name>
      <anchorfile>l__opti__predicates_8c.html</anchorfile>
      <anchor>c24795d3cf733ef70015b9c437604793</anchor>
      <arglist>(L_Cb *cb, L_Operand *operand, L_Oper *oper)</arglist>
    </member>
    <member kind="function">
      <type>int</type>
      <name>L_is_redefined_in_cb_after</name>
      <anchorfile>l__opti__predicates_8c.html</anchorfile>
      <anchor>f6bbd15df655d057f13ec92b21b7e948</anchor>
      <arglist>(L_Oper *oper, L_Operand *operand)</arglist>
    </member>
    <member kind="function">
      <type>int</type>
      <name>L_all_uses_can_be_renamed</name>
      <anchorfile>l__opti__predicates_8c.html</anchorfile>
      <anchor>c900265c13e318dd3f0c1553f2791208</anchor>
      <arglist>(L_Cb *cb, L_Oper *oper, L_Operand *operand)</arglist>
    </member>
    <member kind="function">
      <type>int</type>
      <name>L_not_live_at_cb_end</name>
      <anchorfile>l__opti__predicates_8c.html</anchorfile>
      <anchor>7aaa4a0deb8f808bf56d3373252fa4a5</anchor>
      <arglist>(L_Cb *cb, L_Oper *oper, L_Operand *operand)</arglist>
    </member>
    <member kind="function">
      <type>int</type>
      <name>L_no_flow_dep_from_for</name>
      <anchorfile>l__opti__predicates_8c.html</anchorfile>
      <anchor>10d9e111c91bb957faa50497bf773723</anchor>
      <arglist>(L_Cb *cb, L_Oper *oper, L_Operand *operand)</arglist>
    </member>
    <member kind="function">
      <type>int</type>
      <name>L_no_flow_dep_from</name>
      <anchorfile>l__opti__predicates_8c.html</anchorfile>
      <anchor>eeddbb886df2e0b3e6ab1d38105d5ef7</anchor>
      <arglist>(L_Cb *cb, L_Oper *oper)</arglist>
    </member>
    <member kind="function">
      <type>int</type>
      <name>L_can_copy_op_to_all_live_paths</name>
      <anchorfile>l__opti__predicates_8c.html</anchorfile>
      <anchor>449c1e17b5b91aeebc695e8235a4bbf3</anchor>
      <arglist>(L_Cb *cb, L_Oper *oper)</arglist>
    </member>
    <member kind="function">
      <type>int</type>
      <name>L_no_anti_dep_from_before_redef_of</name>
      <anchorfile>l__opti__predicates_8c.html</anchorfile>
      <anchor>406ee4d210c622f8068c488568b3417b</anchor>
      <arglist>(L_Cb *cb, L_Oper *oper, L_Operand *operand)</arglist>
    </member>
    <member kind="function">
      <type>int</type>
      <name>L_no_anti_dep_from_before_redef</name>
      <anchorfile>l__opti__predicates_8c.html</anchorfile>
      <anchor>4efb87cb66a8def29f921df311301add</anchor>
      <arglist>(L_Cb *cb, L_Oper *oper)</arglist>
    </member>
    <member kind="function">
      <type>int</type>
      <name>L_single_anti_dep_from_before_redef_of</name>
      <anchorfile>l__opti__predicates_8c.html</anchorfile>
      <anchor>1e544f77ca5b796006e3266d097057f0</anchor>
      <arglist>(L_Oper *oper, L_Operand *operand)</arglist>
    </member>
    <member kind="function">
      <type>int</type>
      <name>L_single_anti_dep_from_before_redef</name>
      <anchorfile>l__opti__predicates_8c.html</anchorfile>
      <anchor>72f5b40dd161f914c93439b0d3a8ba9d</anchor>
      <arglist>(L_Oper *oper)</arglist>
    </member>
    <member kind="function">
      <type>int</type>
      <name>L_profitable_for_migration</name>
      <anchorfile>l__opti__predicates_8c.html</anchorfile>
      <anchor>58bb70f0f89ddd9487337b824d5048b9</anchor>
      <arglist>(L_Cb *cb, L_Oper *oper)</arglist>
    </member>
    <member kind="function">
      <type>int</type>
      <name>L_conditionally_redefined_in_cb</name>
      <anchorfile>l__opti__predicates_8c.html</anchorfile>
      <anchor>2b521dc3f50ecacb0f3add67c725dfae</anchor>
      <arglist>(L_Cb *cb, L_Oper *opA, L_Operand *operand)</arglist>
    </member>
    <member kind="function">
      <type>int</type>
      <name>L_no_intersecting_br_between</name>
      <anchorfile>l__opti__predicates_8c.html</anchorfile>
      <anchor>176bd6c630528111877a79c7e8bd4173</anchor>
      <arglist>(L_Oper *opA, L_Oper *opB)</arglist>
    </member>
    <member kind="function">
      <type>int</type>
      <name>L_only_disjoint_br_between</name>
      <anchorfile>l__opti__predicates_8c.html</anchorfile>
      <anchor>50a771fd0c73499cdb658285f50083b7</anchor>
      <arglist>(L_Oper *opA, L_Oper *opB)</arglist>
    </member>
    <member kind="function">
      <type>int</type>
      <name>L_global_no_defs_between</name>
      <anchorfile>l__opti__predicates_8c.html</anchorfile>
      <anchor>502ca7bb9677722cfd3668007614a7c6</anchor>
      <arglist>(L_Operand *operand, L_Cb *cbA, L_Oper *opA, L_Cb *cbB, L_Oper *opB)</arglist>
    </member>
    <member kind="function">
      <type>int</type>
      <name>L_all_dest_operand_global_no_defs_between</name>
      <anchorfile>l__opti__predicates_8c.html</anchorfile>
      <anchor>2eec806a810baf09ee7f267c5c275866</anchor>
      <arglist>(L_Oper *op, L_Cb *cbA, L_Oper *opA, L_Cb *cbB, L_Oper *opB)</arglist>
    </member>
    <member kind="function">
      <type>int</type>
      <name>L_global_same_def_reachs</name>
      <anchorfile>l__opti__predicates_8c.html</anchorfile>
      <anchor>94d75f16d2e9628e72305b077b1abf12</anchor>
      <arglist>(L_Operand *operand, L_Cb *cbA, L_Oper *opA, L_Cb *cbB, L_Oper *opB)</arglist>
    </member>
    <member kind="function">
      <type>int</type>
      <name>L_all_src_operand_global_same_def_reachs</name>
      <anchorfile>l__opti__predicates_8c.html</anchorfile>
      <anchor>75b06e742705940c6f5b1d66388c8ef9</anchor>
      <arglist>(L_Oper *op, L_Cb *cbA, L_Oper *opA, L_Cb *cbB, L_Oper *opB)</arglist>
    </member>
    <member kind="function">
      <type>int</type>
      <name>L_global_share_same_def</name>
      <anchorfile>l__opti__predicates_8c.html</anchorfile>
      <anchor>7425ae63bb86086b2da0d776e0ba508b</anchor>
      <arglist>(L_Operand *operand, L_Cb *cbA, L_Operand *operandA, L_Operand *operandB)</arglist>
    </member>
    <member kind="function">
      <type>int</type>
      <name>L_global_no_defs_between_cb_only</name>
      <anchorfile>l__opti__predicates_8c.html</anchorfile>
      <anchor>b5f7baddf73421b724f5c384221153af</anchor>
      <arglist>(L_Cb *cbA, L_Cb *cbB, L_Operand *var)</arglist>
    </member>
    <member kind="function">
      <type>int</type>
      <name>L_global_no_sub_call_between</name>
      <anchorfile>l__opti__predicates_8c.html</anchorfile>
      <anchor>9423331e99473bb2469d2980befb1ad7</anchor>
      <arglist>(L_Cb *cbA, L_Oper *opA, L_Cb *cbB, L_Oper *opB)</arglist>
    </member>
    <member kind="function">
      <type>int</type>
      <name>L_global_no_general_sub_call_between</name>
      <anchorfile>l__opti__predicates_8c.html</anchorfile>
      <anchor>79f71fb08d545f3ce34ac589cef53c24</anchor>
      <arglist>(L_Cb *cbA, L_Oper *opA, L_Cb *cbB, L_Oper *opB)</arglist>
    </member>
    <member kind="function">
      <type>int</type>
      <name>L_global_no_sync_between</name>
      <anchorfile>l__opti__predicates_8c.html</anchorfile>
      <anchor>33d96a18c000f40d0001e689d5d5ce17</anchor>
      <arglist>(L_Cb *cbA, L_Oper *opA, L_Cb *cbB, L_Oper *opB)</arglist>
    </member>
    <member kind="function">
      <type>int</type>
      <name>L_global_no_danger</name>
      <anchorfile>l__opti__predicates_8c.html</anchorfile>
      <anchor>e06fcd8a2b555330a9ad962f8187f6fe</anchor>
      <arglist>(int macro_flag, int load_flag, int store_flag, L_Cb *cbA, L_Oper *opA, L_Cb *cbB, L_Oper *opB)</arglist>
    </member>
    <member kind="function">
      <type>int</type>
      <name>L_global_no_danger_to_boundary</name>
      <anchorfile>l__opti__predicates_8c.html</anchorfile>
      <anchor>25f91e7ed67baa9359a1b2ac9505981b</anchor>
      <arglist>(int macro_flag, int load_flag, int store_flag, L_Cb *cbA, L_Oper *opA, L_Cb *cbB, L_Oper *opB)</arglist>
    </member>
    <member kind="function">
      <type>int</type>
      <name>L_global_no_overlap_write</name>
      <anchorfile>l__opti__predicates_8c.html</anchorfile>
      <anchor>d6e1d02aeee93ec239bfebff2c5b9bd2</anchor>
      <arglist>(L_Cb *cbA, L_Oper *opA, L_Cb *cbB, L_Oper *opB)</arglist>
    </member>
    <member kind="function">
      <type>int</type>
      <name>L_global_only_branch_src_operand</name>
      <anchorfile>l__opti__predicates_8c.html</anchorfile>
      <anchor>4090de4a7b829f84b43454dfdef13308</anchor>
      <arglist>(L_Operand *operand)</arglist>
    </member>
    <member kind="function">
      <type>int</type>
      <name>L_in_nested_loop</name>
      <anchorfile>l__opti__predicates_8c.html</anchorfile>
      <anchor>38931a84afee91b2b037666600529175</anchor>
      <arglist>(L_Loop *loop, L_Cb *cb)</arglist>
    </member>
    <member kind="function">
      <type>int</type>
      <name>L_cb_dominates_all_loop_exit_cb</name>
      <anchorfile>l__opti__predicates_8c.html</anchorfile>
      <anchor>2007afd046ce384e817e0df0f38df5a9</anchor>
      <arglist>(L_Loop *loop, int *exit_cb, int num_exit_cb, L_Cb *cb)</arglist>
    </member>
    <member kind="function">
      <type>int</type>
      <name>L_cb_dominates_all_loop_backedge_cb</name>
      <anchorfile>l__opti__predicates_8c.html</anchorfile>
      <anchor>7d40e70d9534d08dad7ffd44fb31ad15</anchor>
      <arglist>(L_Loop *loop, int *backedge_cb, int num_backedge_cb, L_Cb *cb)</arglist>
    </member>
    <member kind="function">
      <type>int</type>
      <name>L_loop_invariant_operands</name>
      <anchorfile>l__opti__predicates_8c.html</anchorfile>
      <anchor>7de7e44d2f3f6bd23ea4217cb03c0625</anchor>
      <arglist>(L_Loop *loop, int *loop_cb, int num_cb, L_Oper *oper)</arglist>
    </member>
    <member kind="function">
      <type>int</type>
      <name>L_cost_effective_to_move_ops_to_preheader</name>
      <anchorfile>l__opti__predicates_8c.html</anchorfile>
      <anchor>8e7859aaa1a81effdeafb8888c399571</anchor>
      <arglist>(L_Loop *loop)</arglist>
    </member>
    <member kind="function">
      <type>int</type>
      <name>L_all_uses_in_loop_from</name>
      <anchorfile>l__opti__predicates_8c.html</anchorfile>
      <anchor>a3dd6b039b1d34d32771a1c84201bd5a</anchor>
      <arglist>(L_Loop *loop, int *loop_cb, int num_cb, L_Cb *cb, L_Oper *op)</arglist>
    </member>
    <member kind="function">
      <type>int</type>
      <name>L_no_uses_before_in_loop</name>
      <anchorfile>l__opti__predicates_8c.html</anchorfile>
      <anchor>dd81d96b028febd0c87070132664bc5b</anchor>
      <arglist>(L_Loop *loop, int *loop_cb, int num_cb, L_Cb *cb, L_Oper *op)</arglist>
    </member>
    <member kind="function">
      <type>int</type>
      <name>L_def_reachs_all_out_cb_of_loop</name>
      <anchorfile>l__opti__predicates_8c.html</anchorfile>
      <anchor>d0409414832ef4800dcec61c5ea234b5</anchor>
      <arglist>(L_Loop *loop, int *exit_cb, int num_exit_cb, L_Cb *cb, L_Oper *op)</arglist>
    </member>
    <member kind="function">
      <type>int</type>
      <name>L_safe_to_move_out_of_loop</name>
      <anchorfile>l__opti__predicates_8c.html</anchorfile>
      <anchor>e4a41b496cceb527dfc3a939961bcfcc</anchor>
      <arglist>(L_Loop *loop, int *exit_cb, int num_exit_cb, L_Cb *cb, L_Oper *op)</arglist>
    </member>
    <member kind="function">
      <type>int</type>
      <name>L_no_danger_in_loop</name>
      <anchorfile>l__opti__predicates_8c.html</anchorfile>
      <anchor>7e2424498d1a13098018adf8406eb5be</anchor>
      <arglist>(L_Loop *loop, int *loop_cb, int num_cb, int macro_flag, int load_flag, int store_flag)</arglist>
    </member>
    <member kind="function">
      <type>int</type>
      <name>L_no_memory_conflicts_in_loop</name>
      <anchorfile>l__opti__predicates_8c.html</anchorfile>
      <anchor>4ed48feb95af8df8e3c7d10512634ca6</anchor>
      <arglist>(L_Loop *loop, int *loop_cb, int num_cb, L_Oper *op, L_Oper *omit)</arglist>
    </member>
    <member kind="function">
      <type>int</type>
      <name>L_unique_memory_location</name>
      <anchorfile>l__opti__predicates_8c.html</anchorfile>
      <anchor>09ab7abe389abc6b84df9b8f4c8649d3</anchor>
      <arglist>(L_Loop *loop, int *loop_cb, int num_cb, L_Oper *op, int *n_read, int *n_write, L_Oper **store_op)</arglist>
    </member>
    <member kind="function">
      <type>int</type>
      <name>L_all_predecessor_cb_in_loop</name>
      <anchorfile>l__opti__predicates_8c.html</anchorfile>
      <anchor>64606547daed4afd60df14b8f36bd452</anchor>
      <arglist>(L_Loop *loop, L_Cb *cb)</arglist>
    </member>
    <member kind="function">
      <type>int</type>
      <name>L_branch_to_loop_cb</name>
      <anchorfile>l__opti__predicates_8c.html</anchorfile>
      <anchor>21396041aee280a7b1355fd23e9df129</anchor>
      <arglist>(L_Loop *loop, L_Oper *op)</arglist>
    </member>
    <member kind="function">
      <type>int</type>
      <name>L_basic_induction_var</name>
      <anchorfile>l__opti__predicates_8c.html</anchorfile>
      <anchor>a49ec2d67250315537b871def6d68eea</anchor>
      <arglist>(L_Loop *loop, L_Operand *operand)</arglist>
    </member>
    <member kind="function">
      <type>int</type>
      <name>L_num_constant_increment_of_ind</name>
      <anchorfile>l__opti__predicates_8c.html</anchorfile>
      <anchor>db762481be648c9121a164527f84e855</anchor>
      <arglist>(L_Operand *operand, L_Ind_Info *ind_info)</arglist>
    </member>
    <member kind="function">
      <type>int</type>
      <name>L_same_ind_increment</name>
      <anchorfile>l__opti__predicates_8c.html</anchorfile>
      <anchor>d6ee0d4066f201ed25f2eafd9ac7c7b9</anchor>
      <arglist>(L_Operand *operand1, L_Operand *operand2, L_Ind_Info *ind_info)</arglist>
    </member>
    <member kind="function">
      <type>int</type>
      <name>L_int_one_increment_of_ind</name>
      <anchorfile>l__opti__predicates_8c.html</anchorfile>
      <anchor>63bd19c11f576862e64d566da5a1aaff</anchor>
      <arglist>(L_Operand *operand, L_Ind_Info *ind_info)</arglist>
    </member>
    <member kind="function">
      <type>int</type>
      <name>L_int_neg_one_increment_of_ind</name>
      <anchorfile>l__opti__predicates_8c.html</anchorfile>
      <anchor>9d58bec6fa5376ab00d7707e3f56c8b5</anchor>
      <arglist>(L_Operand *operand, L_Ind_Info *ind_info)</arglist>
    </member>
    <member kind="function">
      <type>int</type>
      <name>L_ind_increment_is_multiple_of</name>
      <anchorfile>l__opti__predicates_8c.html</anchorfile>
      <anchor>811d0654e490bfb433affad20b3004f7</anchor>
      <arglist>(L_Operand *operand1, L_Operand *operand2, L_Ind_Info *ind_info)</arglist>
    </member>
    <member kind="function">
      <type>int</type>
      <name>L_num_constant_init_val_of_ind</name>
      <anchorfile>l__opti__predicates_8c.html</anchorfile>
      <anchor>a5f96ee8503306d82b4509041ee07e33</anchor>
      <arglist>(L_Operand *operand, L_Ind_Info *ind_info)</arglist>
    </member>
    <member kind="function">
      <type>int</type>
      <name>L_same_ind_initial_val</name>
      <anchorfile>l__opti__predicates_8c.html</anchorfile>
      <anchor>619a8f9dc432e4aa4e100dadb69b916b</anchor>
      <arglist>(L_Loop *loop, L_Operand *operand1, L_Operand *operand2, L_Ind_Info *ind_info)</arglist>
    </member>
    <member kind="function">
      <type>int</type>
      <name>L_ind_var_will_reach_limit</name>
      <anchorfile>l__opti__predicates_8c.html</anchorfile>
      <anchor>6c9c7954c6f2a2c96986b3d1388d213e</anchor>
      <arglist>(L_Loop *loop, L_Oper *op, L_Ind_Info *ind_info)</arglist>
    </member>
    <member kind="function">
      <type>int</type>
      <name>L_can_simplify_loop_branch</name>
      <anchorfile>l__opti__predicates_8c.html</anchorfile>
      <anchor>791bb09c604d08b69c9c21cf09f3f61a</anchor>
      <arglist>(L_Loop *loop, L_Oper *op, L_Ind_Info *ind_info)</arglist>
    </member>
    <member kind="function">
      <type>int</type>
      <name>L_is_str_reducible_opcode</name>
      <anchorfile>l__opti__predicates_8c.html</anchorfile>
      <anchor>c68ff9ec2bd798982bd05bcba614b04c</anchor>
      <arglist>(L_Oper *op)</arglist>
    </member>
    <member kind="function">
      <type>int</type>
      <name>L_useful_str_red</name>
      <anchorfile>l__opti__predicates_8c.html</anchorfile>
      <anchor>a55e21fe6db7a17bf6279cf657836065</anchor>
      <arglist>(L_Loop *loop, L_Oper *op, L_Operand *ind_operand)</arglist>
    </member>
    <member kind="function">
      <type>int</type>
      <name>L_not_live_in_out_cb</name>
      <anchorfile>l__opti__predicates_8c.html</anchorfile>
      <anchor>e1eecbc5a1d0f779c9d667d03cea71d5</anchor>
      <arglist>(int *out_cb, int num_out_cb, L_Operand *operand)</arglist>
    </member>
    <member kind="function">
      <type>int</type>
      <name>L_no_uses_of_ind</name>
      <anchorfile>l__opti__predicates_8c.html</anchorfile>
      <anchor>819bef2b17274094be49217d6e3a5a59</anchor>
      <arglist>(L_Loop *loop, int *loop_cb, int num_cb, L_Operand *operand)</arglist>
    </member>
    <member kind="function">
      <type>int</type>
      <name>L_ind_should_be_reinitialized</name>
      <anchorfile>l__opti__predicates_8c.html</anchorfile>
      <anchor>801acf3dac17ef3ad834ff8b6fde4dac</anchor>
      <arglist>(L_Loop *loop, int *loop_cb, int num_cb, L_Operand *operand)</arglist>
    </member>
    <member kind="function">
      <type>int</type>
      <name>L_basic_ind_var_in_same_family</name>
      <anchorfile>l__opti__predicates_8c.html</anchorfile>
      <anchor>e0f8f92225f62edcf3f355be57fdd6c9</anchor>
      <arglist>(L_Loop *loop, int *loop_cb, int num_cb, L_Operand *operand1, L_Operand *operand2)</arglist>
    </member>
    <member kind="function">
      <type>int</type>
      <name>L_no_uses_of_between_first_and_last_defs</name>
      <anchorfile>l__opti__predicates_8c.html</anchorfile>
      <anchor>023369591c9d0a8b90acf750705b3ba3</anchor>
      <arglist>(L_Loop *loop, int *loop_cb, int num_cb, L_Operand *operand1, L_Operand *operand2)</arglist>
    </member>
    <member kind="function">
      <type>int</type>
      <name>L_ind_constant_offset_initial_val</name>
      <anchorfile>l__opti__predicates_8c.html</anchorfile>
      <anchor>b39a4ded4d6401da4a036e3cc9597000</anchor>
      <arglist>(L_Cb *preheader, L_Operand *operand1, L_Operand *operand2, L_Oper *start_op1, L_Oper *start_op2, L_Ind_Info *ind_info)</arglist>
    </member>
    <member kind="function">
      <type>int</type>
      <name>L_only_used_as_base_addr_with_const_offset_in_loop</name>
      <anchorfile>l__opti__predicates_8c.html</anchorfile>
      <anchor>46cd8c20ec62e0659d0e16f0aa8b05dd</anchor>
      <arglist>(L_Loop *loop, int *loop_cb, int num_cb, L_Operand *operand, int offset)</arglist>
    </member>
    <member kind="function">
      <type>int</type>
      <name>L_all_uses_of_ind_can_change_offset</name>
      <anchorfile>l__opti__predicates_8c.html</anchorfile>
      <anchor>57f775c31d6cc22cc79a8c90c9542e14</anchor>
      <arglist>(L_Loop *loop, int *loop_cb, int num_cb, int *out_cb, int num_out_cb, L_Operand *operand1, L_Operand *operand2)</arglist>
    </member>
    <member kind="function">
      <type>int</type>
      <name>L_can_modify_dep_branches_in_loop</name>
      <anchorfile>l__opti__predicates_8c.html</anchorfile>
      <anchor>2597e99b4e40a02435c1c7d1d6e66282</anchor>
      <arglist>(L_Loop *loop, int *loop_cb, int num_cb, L_Oper *start_op, L_Cb *start_cb)</arglist>
    </member>
    <member kind="function">
      <type>int</type>
      <name>L_ind_only_used_with_loop_inv_operands</name>
      <anchorfile>l__opti__predicates_8c.html</anchorfile>
      <anchor>9bbd8e608c99c207ece3c2c6dd9a3ccc</anchor>
      <arglist>(L_Loop *loop, int *loop_cb, int num_cb, L_Operand *operand)</arglist>
    </member>
    <member kind="function">
      <type>int</type>
      <name>L_ind_only_used_with_memory_ops</name>
      <anchorfile>l__opti__predicates_8c.html</anchorfile>
      <anchor>e1348fe0468ca9a228eb6df63aa98061</anchor>
      <arglist>(L_Loop *loop, int *loop_cb, int num_cb, L_Operand *operand)</arglist>
    </member>
    <member kind="function">
      <type>int</type>
      <name>L_all_uses_of_ind_can_be_modified1</name>
      <anchorfile>l__opti__predicates_8c.html</anchorfile>
      <anchor>e1abab3d195d38114d2aad51dff76a2c</anchor>
      <arglist>(L_Loop *loop, int *loop_cb, int num_cb, L_Operand *operand)</arglist>
    </member>
    <member kind="function">
      <type>int</type>
      <name>L_all_uses_of_ind_can_be_modified2</name>
      <anchorfile>l__opti__predicates_8c.html</anchorfile>
      <anchor>ac8587aa103e764da31365087f2b7e5c</anchor>
      <arglist>(L_Loop *loop, int *loop_cb, int num_cb, L_Operand *operand)</arglist>
    </member>
    <member kind="function">
      <type>int</type>
      <name>L_better_to_eliminate_operand2</name>
      <anchorfile>l__opti__predicates_8c.html</anchorfile>
      <anchor>2d40e2e7b301e506ed1b24cab97666d0</anchor>
      <arglist>(L_Loop *loop, int *loop_cb, int num_cb, int *out_cb, int num_out_cb, L_Operand *operand1, L_Operand *operand2)</arglist>
    </member>
    <member kind="function">
      <type>int</type>
      <name>L_cost_effective_for_ind_complex_elim</name>
      <anchorfile>l__opti__predicates_8c.html</anchorfile>
      <anchor>195b4590b4886194fde4138a7c0bb121</anchor>
      <arglist>(L_Loop *loop, int *loop_cb, int num_cb)</arglist>
    </member>
    <member kind="function">
      <type>int</type>
      <name>L_live_at_nondominated_exits</name>
      <anchorfile>l__opti__predicates_8c.html</anchorfile>
      <anchor>146572247f82e5e3126cc9f35a3236d4</anchor>
      <arglist>(L_Loop *loop, int *exit_cb, int num_exit_cb, L_Cb *cb, L_Operand *operand)</arglist>
    </member>
    <member kind="function">
      <type>int</type>
      <name>L_is_loop_var</name>
      <anchorfile>l__opti__predicates_8c.html</anchorfile>
      <anchor>87e26e5f0ab2acc3df030c9b76c48a99</anchor>
      <arglist>(L_Loop *loop, int *loop_cb, int num_cb, L_Operand *ind_var, L_Ind_Info *ind_info)</arglist>
    </member>
    <member kind="function">
      <type>int</type>
      <name>L_empty_block</name>
      <anchorfile>l__opti__predicates_8c.html</anchorfile>
      <anchor>f063b6afd3936102663f17166b9de2e0</anchor>
      <arglist>(L_Cb *cb)</arglist>
    </member>
    <member kind="function">
      <type>int</type>
      <name>L_only_uncond_branch_in_block</name>
      <anchorfile>l__opti__predicates_8c.html</anchorfile>
      <anchor>6fcc0af9eddc48531ad1c5d7971510cf</anchor>
      <arglist>(L_Cb *cb)</arglist>
    </member>
    <member kind="function">
      <type>int</type>
      <name>L_multiple_cond_branch_in_block</name>
      <anchorfile>l__opti__predicates_8c.html</anchorfile>
      <anchor>d2d263446506b32d94acfecd79a9b63f</anchor>
      <arglist>(L_Cb *cb)</arglist>
    </member>
    <member kind="function">
      <type>int</type>
      <name>L_cb_contains_prologue</name>
      <anchorfile>l__opti__predicates_8c.html</anchorfile>
      <anchor>13be1d78920a772f85f1a7f26c8f9b0b</anchor>
      <arglist>(L_Cb *cb)</arglist>
    </member>
    <member kind="function">
      <type>int</type>
      <name>L_cb_contains_epilogue</name>
      <anchorfile>l__opti__predicates_8c.html</anchorfile>
      <anchor>c3bdc41589cc53d9a98b2bb63fdef578</anchor>
      <arglist>(L_Cb *cb)</arglist>
    </member>
    <member kind="function">
      <type>int</type>
      <name>L_cb_contains_multiple_branches_to</name>
      <anchorfile>l__opti__predicates_8c.html</anchorfile>
      <anchor>ee57a4450ebd8253222c73b9c0d3b673</anchor>
      <arglist>(L_Cb *cb, L_Cb *target)</arglist>
    </member>
    <member kind="function">
      <type>int</type>
      <name>L_need_fallthru_path</name>
      <anchorfile>l__opti__predicates_8c.html</anchorfile>
      <anchor>c33b51481df7189eb542dd8be6d170fa</anchor>
      <arglist>(L_Cb *cb)</arglist>
    </member>
    <member kind="function">
      <type>int</type>
      <name>L_marked_as_post_increment</name>
      <anchorfile>l__opti__predicates_8c.html</anchorfile>
      <anchor>5fa6e89289be3289220afe2c75f943fa</anchor>
      <arglist>(L_Oper *oper)</arglist>
    </member>
    <member kind="function">
      <type>int</type>
      <name>L_marked_as_pre_increment</name>
      <anchorfile>l__opti__predicates_8c.html</anchorfile>
      <anchor>863bd2a1f8ce934f416a20c6fd43071d</anchor>
      <arglist>(L_Oper *oper)</arglist>
    </member>
    <member kind="function">
      <type>int</type>
      <name>L_marked_as_pre_post_increment</name>
      <anchorfile>l__opti__predicates_8c.html</anchorfile>
      <anchor>0b926975dad830474d067047eeb0655d</anchor>
      <arglist>(L_Oper *oper)</arglist>
    </member>
    <member kind="function">
      <type>int</type>
      <name>L_can_recombine_mem_inc_ops</name>
      <anchorfile>l__opti__predicates_8c.html</anchorfile>
      <anchor>92947cd9e6a951986edd0dc43c246f52</anchor>
      <arglist>(L_Oper *mem, L_Oper *inc)</arglist>
    </member>
    <member kind="function">
      <type>int</type>
      <name>L_can_recombine_inc_mem_ops</name>
      <anchorfile>l__opti__predicates_8c.html</anchorfile>
      <anchor>f7ad75bae2cc4e9be4615df517eff828</anchor>
      <arglist>(L_Oper *mem, L_Oper *inc)</arglist>
    </member>
    <member kind="function">
      <type>int</type>
      <name>L_can_make_post_inc</name>
      <anchorfile>l__opti__predicates_8c.html</anchorfile>
      <anchor>8b598043276ba1951b75eca7cf9fe88f</anchor>
      <arglist>(L_Oper *mem_op, L_Oper *ind_op)</arglist>
    </member>
    <member kind="function">
      <type>int</type>
      <name>L_can_make_pre_inc</name>
      <anchorfile>l__opti__predicates_8c.html</anchorfile>
      <anchor>36c91931c918c822519b3979ad9b295e</anchor>
      <arglist>(L_Oper *mem_op, L_Oper *ind_op)</arglist>
    </member>
    <member kind="function">
      <type>int</type>
      <name>L_ind_var_is_updated_by_pre_post_increment</name>
      <anchorfile>l__opti__predicates_8c.html</anchorfile>
      <anchor>6da073a737c8baf7d32d026ace000bae</anchor>
      <arglist>(L_Loop *loop, int *loop_cb, int num_cb, L_Operand *ind_var)</arglist>
    </member>
    <member kind="function">
      <type>int</type>
      <name>L_oper_used_in_address_calc</name>
      <anchorfile>l__opti__predicates_8c.html</anchorfile>
      <anchor>6f4e99dc40e901981fb3efa513f3a376</anchor>
      <arglist>(L_Cb *cb, L_Oper *op)</arglist>
    </member>
    <member kind="function">
      <type>int</type>
      <name>L_extension_compatible_ctype</name>
      <anchorfile>l__opti__predicates_8c.html</anchorfile>
      <anchor>7142116794ff3af9ae92b8ede1d1a03b</anchor>
      <arglist>(ITuint8 from, ITuint8 to)</arglist>
    </member>
    <member kind="function">
      <type>int</type>
      <name>L_redundant_extension_rev</name>
      <anchorfile>l__opti__predicates_8c.html</anchorfile>
      <anchor>3f5ce123e9a95a7a368066f70b568aae</anchor>
      <arglist>(L_Oper *src_op, L_Oper *ext_op)</arglist>
    </member>
    <member kind="function">
      <type>int</type>
      <name>L_redundant_extension_fwd</name>
      <anchorfile>l__opti__predicates_8c.html</anchorfile>
      <anchor>b813608b50f2d3c549445902b1e1e07a</anchor>
      <arglist>(L_Oper *ext_op, L_Oper *dst_op)</arglist>
    </member>
    <member kind="function">
      <type>int</type>
      <name>L_cb_first_flow_const_compare_branch</name>
      <anchorfile>l__opti__predicates_8c.html</anchorfile>
      <anchor>eba069233eb5a13e2360f9421cab177b</anchor>
      <arglist>(L_Cb *cb, L_Operand **var, L_Operand **constant)</arglist>
    </member>
    <member kind="function">
      <type>int</type>
      <name>L_cb_all_incoming_flows_same_const_compare</name>
      <anchorfile>l__opti__predicates_8c.html</anchorfile>
      <anchor>4d26fe2f752328abf99f6460a5590c0f</anchor>
      <arglist>(L_Cb *cb, L_Operand *var, L_Operand *constant)</arglist>
    </member>
  </compound>
  <compound kind="file">
    <name>l_opti_predicates.h</name>
    <path>/mnt/hgfs/mkt/Desktop/openimpact/src/Lcode/opti/Lopti/</path>
    <filename>l__opti__predicates_8h</filename>
    <member kind="function">
      <type>int</type>
      <name>L_is_compatible_opc</name>
      <anchorfile>l__opti__predicates_8h.html</anchorfile>
      <anchor>e42d3b53ce482c335614a762f3d27a5b</anchor>
      <arglist>(int opc1, int opc2)</arglist>
    </member>
    <member kind="function">
      <type>int</type>
      <name>L_compatible_opcodes</name>
      <anchorfile>l__opti__predicates_8h.html</anchorfile>
      <anchor>dccb2aa6e00a1e49e06f527f1bd6b277</anchor>
      <arglist>(L_Oper *, L_Oper *)</arglist>
    </member>
    <member kind="function">
      <type>int</type>
      <name>L_compatible_arithmetic_ops</name>
      <anchorfile>l__opti__predicates_8h.html</anchorfile>
      <anchor>4ee06bb6528b0ae0674ee1fde2f65713</anchor>
      <arglist>(L_Oper *, L_Oper *)</arglist>
    </member>
    <member kind="function">
      <type>int</type>
      <name>L_compatible_to_combine_consts</name>
      <anchorfile>l__opti__predicates_8h.html</anchorfile>
      <anchor>3a2eb95412eb3c51787af8bc1cfb2992</anchor>
      <arglist>(L_Oper *, L_Oper *)</arglist>
    </member>
    <member kind="function">
      <type>int</type>
      <name>L_unary_opcode</name>
      <anchorfile>l__opti__predicates_8h.html</anchorfile>
      <anchor>e60036afcf8563c693003e6045ba508a</anchor>
      <arglist>(L_Oper *)</arglist>
    </member>
    <member kind="function">
      <type>int</type>
      <name>L_are_opposite_branch_opcodes</name>
      <anchorfile>l__opti__predicates_8h.html</anchorfile>
      <anchor>cb4e0f9cad088fa98211a5fc011db3d2</anchor>
      <arglist>(L_Oper *, L_Oper *)</arglist>
    </member>
    <member kind="function">
      <type>int</type>
      <name>L_are_reverse_branch_opcodes</name>
      <anchorfile>l__opti__predicates_8h.html</anchorfile>
      <anchor>89265276c3b4814e769666f2cbb0b387</anchor>
      <arglist>(L_Oper *, L_Oper *)</arglist>
    </member>
    <member kind="function">
      <type>int</type>
      <name>L_are_same_branch_opcodes</name>
      <anchorfile>l__opti__predicates_8h.html</anchorfile>
      <anchor>3626ac80192541dd454c64809d96eab5</anchor>
      <arglist>(L_Oper *, L_Oper *)</arglist>
    </member>
    <member kind="function">
      <type>int</type>
      <name>L_load_store_sign_extend_conflict</name>
      <anchorfile>l__opti__predicates_8h.html</anchorfile>
      <anchor>c814198c50fb66ae9e4d50c7a6058b27</anchor>
      <arglist>(L_Oper *, L_Oper *)</arglist>
    </member>
    <member kind="function">
      <type>int</type>
      <name>L_compatible_load_store</name>
      <anchorfile>l__opti__predicates_8h.html</anchorfile>
      <anchor>f17e0165cdda255faca4f87b79d6be29</anchor>
      <arglist>(L_Oper *, L_Oper *)</arglist>
    </member>
    <member kind="function">
      <type>int</type>
      <name>L_cancelling_opcodes</name>
      <anchorfile>l__opti__predicates_8h.html</anchorfile>
      <anchor>cc9dc1289ccbaab08686f237e50143bb</anchor>
      <arglist>(L_Oper *, L_Oper *)</arglist>
    </member>
    <member kind="function">
      <type>int</type>
      <name>L_is_legal_unsigned_value_offset_32</name>
      <anchorfile>l__opti__predicates_8h.html</anchorfile>
      <anchor>ae291902a690275106988c6c2c258ef1</anchor>
      <arglist>(unsigned int, int)</arglist>
    </member>
    <member kind="function">
      <type>int</type>
      <name>L_is_int_24</name>
      <anchorfile>l__opti__predicates_8h.html</anchorfile>
      <anchor>0adb071506df4622e99f19a067fec01c</anchor>
      <arglist>(L_Operand *)</arglist>
    </member>
    <member kind="function">
      <type>int</type>
      <name>L_is_int_between_0_and_128</name>
      <anchorfile>l__opti__predicates_8h.html</anchorfile>
      <anchor>404f40bf263767895193d47ea5ef48a0</anchor>
      <arglist>(L_Operand *)</arglist>
    </member>
    <member kind="function">
      <type>int</type>
      <name>L_is_branch_target</name>
      <anchorfile>l__opti__predicates_8h.html</anchorfile>
      <anchor>7d0dfe3819fc6cacaff0343e9e963302</anchor>
      <arglist>(L_Oper *, L_Cb *)</arglist>
    </member>
    <member kind="function">
      <type>int</type>
      <name>L_can_change_all_later_uses_with</name>
      <anchorfile>l__opti__predicates_8h.html</anchorfile>
      <anchor>b7e8060ddc13312d3e934eea3fa87737</anchor>
      <arglist>(L_Cb *, L_Operand *, L_Operand *, L_Oper *)</arglist>
    </member>
    <member kind="function">
      <type>int</type>
      <name>L_can_change_all_uses_between</name>
      <anchorfile>l__opti__predicates_8h.html</anchorfile>
      <anchor>b7f42e01e2e01e6b8225e830d0f4beef</anchor>
      <arglist>(L_Cb *cb, L_Operand *operand, L_Operand *replace, L_Oper *opA, L_Oper *opB)</arglist>
    </member>
    <member kind="function">
      <type>int</type>
      <name>L_increment_operation</name>
      <anchorfile>l__opti__predicates_8h.html</anchorfile>
      <anchor>6eba4f5367e727fdce50135ac2c7f79d</anchor>
      <arglist>(L_Oper *)</arglist>
    </member>
    <member kind="function">
      <type>int</type>
      <name>L_no_overlap_write</name>
      <anchorfile>l__opti__predicates_8h.html</anchorfile>
      <anchor>0e58d077129261027cdfd2f927d865ec</anchor>
      <arglist>(L_Cb *, L_Oper *, L_Oper *, int, L_Oper *, int)</arglist>
    </member>
    <member kind="function">
      <type>int</type>
      <name>L_no_overlap_read</name>
      <anchorfile>l__opti__predicates_8h.html</anchorfile>
      <anchor>9f28725b0e8e93892e5af740d4305b5d</anchor>
      <arglist>(L_Cb *, L_Oper *, L_Oper *, int, L_Oper *, int)</arglist>
    </member>
    <member kind="function">
      <type>int</type>
      <name>L_no_sb_loop_br_between</name>
      <anchorfile>l__opti__predicates_8h.html</anchorfile>
      <anchor>c792709fc5c04321e2c0ac1c24c7c6f6</anchor>
      <arglist>(L_Cb *, L_Oper *, L_Oper *)</arglist>
    </member>
    <member kind="function">
      <type>int</type>
      <name>L_can_move_above</name>
      <anchorfile>l__opti__predicates_8h.html</anchorfile>
      <anchor>b233fca3018938fbf04b7c13d7ece2f7</anchor>
      <arglist>(L_Cb *, L_Oper *, L_Oper *)</arglist>
    </member>
    <member kind="function">
      <type>int</type>
      <name>L_can_move_below</name>
      <anchorfile>l__opti__predicates_8h.html</anchorfile>
      <anchor>b1e9fdd7eee632ea141694868189b737</anchor>
      <arglist>(L_Cb *, L_Oper *, L_Oper *)</arglist>
    </member>
    <member kind="function">
      <type>int</type>
      <name>L_can_undo_and_combine</name>
      <anchorfile>l__opti__predicates_8h.html</anchorfile>
      <anchor>f3773f472289f64f90af375c20fd6ec7</anchor>
      <arglist>(L_Oper *, L_Oper *, L_Oper *)</arglist>
    </member>
    <member kind="function">
      <type>int</type>
      <name>L_can_undo_and_combine2</name>
      <anchorfile>l__opti__predicates_8h.html</anchorfile>
      <anchor>d5ee1b648feaf02a1611bf4f9f5dce68</anchor>
      <arglist>(L_Oper *, L_Oper *)</arglist>
    </member>
    <member kind="function">
      <type>int</type>
      <name>L_can_make_mul_add_sub</name>
      <anchorfile>l__opti__predicates_8h.html</anchorfile>
      <anchor>856c8d38eb7baf3178635b9109b07070</anchor>
      <arglist>(L_Oper *, L_Oper *)</arglist>
    </member>
    <member kind="function">
      <type>int</type>
      <name>L_can_undo_increment</name>
      <anchorfile>l__opti__predicates_8h.html</anchorfile>
      <anchor>978adcc632538804f6756a317bfe8d3b</anchor>
      <arglist>(L_Oper *, L_Oper *)</arglist>
    </member>
    <member kind="function">
      <type>int</type>
      <name>L_no_exceptions</name>
      <anchorfile>l__opti__predicates_8h.html</anchorfile>
      <anchor>2eaac47352b8c4f7956cdd208a0190fe</anchor>
      <arglist>(L_Oper *)</arglist>
    </member>
    <member kind="function">
      <type>int</type>
      <name>L_will_lose_accuracy</name>
      <anchorfile>l__opti__predicates_8h.html</anchorfile>
      <anchor>e851c9a3f8838ab40f0cc9dd738b1e2b</anchor>
      <arglist>(L_Oper *)</arglist>
    </member>
    <member kind="function">
      <type>int</type>
      <name>L_invertible_float_constant</name>
      <anchorfile>l__opti__predicates_8h.html</anchorfile>
      <anchor>14e56bb3411e363d81f0c6e8e299d41e</anchor>
      <arglist>(float)</arglist>
    </member>
    <member kind="function">
      <type>int</type>
      <name>L_invertible_constant</name>
      <anchorfile>l__opti__predicates_8h.html</anchorfile>
      <anchor>4058cede8f9d8230ac9957063491026c</anchor>
      <arglist>(double)</arglist>
    </member>
    <member kind="function">
      <type>int</type>
      <name>L_live_outside_cb_after</name>
      <anchorfile>l__opti__predicates_8h.html</anchorfile>
      <anchor>e196e1a1a611f37147eded7f3fb14e08</anchor>
      <arglist>(L_Cb *, L_Operand *, L_Oper *)</arglist>
    </member>
    <member kind="function">
      <type>int</type>
      <name>L_not_live_outside_cb_between</name>
      <anchorfile>l__opti__predicates_8h.html</anchorfile>
      <anchor>fc24c483fe39e30d31df96d7951dbb41</anchor>
      <arglist>(L_Cb *, L_Operand *, L_Oper *, L_Oper *)</arglist>
    </member>
    <member kind="function">
      <type>int</type>
      <name>L_all_dest_operand_not_live_outside_cb_between</name>
      <anchorfile>l__opti__predicates_8h.html</anchorfile>
      <anchor>93fcf2ba5ed6da0b3b4e1da8663b057c</anchor>
      <arglist>(L_Cb *, L_Oper *, L_Oper *, L_Oper *)</arglist>
    </member>
    <member kind="function">
      <type>int</type>
      <name>L_all_src_operand_not_live_outside_cb_between</name>
      <anchorfile>l__opti__predicates_8h.html</anchorfile>
      <anchor>b6962af7028d3c07ddf6c2a9f96bbf21</anchor>
      <arglist>(L_Cb *, L_Oper *, L_Oper *, L_Oper *)</arglist>
    </member>
    <member kind="function">
      <type>int</type>
      <name>L_single_use_of</name>
      <anchorfile>l__opti__predicates_8h.html</anchorfile>
      <anchor>b542761672e9e2b57bd6ddd49bb85fc9</anchor>
      <arglist>(L_Cb *, L_Operand *, L_Oper *)</arglist>
    </member>
    <member kind="function">
      <type>int</type>
      <name>L_no_use_of</name>
      <anchorfile>l__opti__predicates_8h.html</anchorfile>
      <anchor>69b4ec96d8fe676292f1a2931cceab79</anchor>
      <arglist>(L_Cb *, L_Operand *, L_Oper *)</arglist>
    </member>
    <member kind="function">
      <type>int</type>
      <name>L_is_redefined_in_cb_after</name>
      <anchorfile>l__opti__predicates_8h.html</anchorfile>
      <anchor>fcb0df0f453964af285101ac24a83b73</anchor>
      <arglist>(L_Oper *, L_Operand *)</arglist>
    </member>
    <member kind="function">
      <type>int</type>
      <name>L_all_uses_can_be_renamed</name>
      <anchorfile>l__opti__predicates_8h.html</anchorfile>
      <anchor>130ac4bd61376caa705d1248f7613713</anchor>
      <arglist>(L_Cb *, L_Oper *, L_Operand *)</arglist>
    </member>
    <member kind="function">
      <type>int</type>
      <name>L_not_live_at_cb_end</name>
      <anchorfile>l__opti__predicates_8h.html</anchorfile>
      <anchor>c371d9b7fa54099ae4fa0102912a9130</anchor>
      <arglist>(L_Cb *, L_Oper *, L_Operand *)</arglist>
    </member>
    <member kind="function">
      <type>int</type>
      <name>L_no_flow_dep_dep_from_for</name>
      <anchorfile>l__opti__predicates_8h.html</anchorfile>
      <anchor>65ca95d687ff45ee94d6e702fbedecfd</anchor>
      <arglist>(L_Oper *, L_Operand *)</arglist>
    </member>
    <member kind="function">
      <type>int</type>
      <name>L_no_flow_dep_from</name>
      <anchorfile>l__opti__predicates_8h.html</anchorfile>
      <anchor>6683d2cd5f5ad6b3805461a5ca1676da</anchor>
      <arglist>(L_Cb *, L_Oper *)</arglist>
    </member>
    <member kind="function">
      <type>int</type>
      <name>L_can_copy_op_to_all_live_paths</name>
      <anchorfile>l__opti__predicates_8h.html</anchorfile>
      <anchor>d7ee521b38556e1ed121839b2c98bac4</anchor>
      <arglist>(L_Cb *, L_Oper *)</arglist>
    </member>
    <member kind="function">
      <type>int</type>
      <name>L_no_anti_dep_from_before_redef_of</name>
      <anchorfile>l__opti__predicates_8h.html</anchorfile>
      <anchor>0c821d556eead3c87a1cd6706f706ae2</anchor>
      <arglist>(L_Cb *, L_Oper *, L_Operand *)</arglist>
    </member>
    <member kind="function">
      <type>int</type>
      <name>L_no_anti_dep_from_before_redef</name>
      <anchorfile>l__opti__predicates_8h.html</anchorfile>
      <anchor>86b98009a08c137a69086627ff10ca9a</anchor>
      <arglist>(L_Cb *, L_Oper *)</arglist>
    </member>
    <member kind="function">
      <type>int</type>
      <name>L_single_anti_dep_from_before_redef_of</name>
      <anchorfile>l__opti__predicates_8h.html</anchorfile>
      <anchor>8edbe975d681f94327562efc1778d4b9</anchor>
      <arglist>(L_Oper *, L_Operand *)</arglist>
    </member>
    <member kind="function">
      <type>int</type>
      <name>L_single_anti_dep_from_before_redef</name>
      <anchorfile>l__opti__predicates_8h.html</anchorfile>
      <anchor>70ed9adce8a98f74702afa474341ec1d</anchor>
      <arglist>(L_Oper *)</arglist>
    </member>
    <member kind="function">
      <type>int</type>
      <name>L_profitable_for_migration</name>
      <anchorfile>l__opti__predicates_8h.html</anchorfile>
      <anchor>3ef7dfd25374e61bb08c20005aafbf71</anchor>
      <arglist>(L_Cb *, L_Oper *)</arglist>
    </member>
    <member kind="function">
      <type>int</type>
      <name>L_conditionally_redefined_in_cb</name>
      <anchorfile>l__opti__predicates_8h.html</anchorfile>
      <anchor>5de854b09a37e56a2395af12c4de63ad</anchor>
      <arglist>(L_Cb *, L_Oper *, L_Operand *)</arglist>
    </member>
    <member kind="function">
      <type>int</type>
      <name>L_no_intersecting_br_between</name>
      <anchorfile>l__opti__predicates_8h.html</anchorfile>
      <anchor>163c2ce91fd19f1fc1d4c91e84503609</anchor>
      <arglist>(L_Oper *, L_Oper *)</arglist>
    </member>
    <member kind="function">
      <type>int</type>
      <name>L_only_disjoint_br_between</name>
      <anchorfile>l__opti__predicates_8h.html</anchorfile>
      <anchor>c2858617dadc7c2eb6c171a98b68c8e4</anchor>
      <arglist>(L_Oper *, L_Oper *)</arglist>
    </member>
    <member kind="function">
      <type>int</type>
      <name>L_global_no_defs_between</name>
      <anchorfile>l__opti__predicates_8h.html</anchorfile>
      <anchor>94c9b1053b774d33a5a134508604d251</anchor>
      <arglist>(L_Operand *, L_Cb *, L_Oper *, L_Cb *, L_Oper *)</arglist>
    </member>
    <member kind="function">
      <type>int</type>
      <name>L_all_dest_operand_global_no_defs_between</name>
      <anchorfile>l__opti__predicates_8h.html</anchorfile>
      <anchor>288082aebaa7b15db839d4ee1e8f71a9</anchor>
      <arglist>(L_Oper *, L_Cb *, L_Oper *, L_Cb *, L_Oper *)</arglist>
    </member>
    <member kind="function">
      <type>int</type>
      <name>L_global_same_def_reachs</name>
      <anchorfile>l__opti__predicates_8h.html</anchorfile>
      <anchor>1b0fcf2245bad125cd53da1fdf708ff8</anchor>
      <arglist>(L_Operand *, L_Cb *, L_Oper *, L_Cb *, L_Oper *)</arglist>
    </member>
    <member kind="function">
      <type>int</type>
      <name>L_global_share_same_def</name>
      <anchorfile>l__opti__predicates_8h.html</anchorfile>
      <anchor>c0604e018eb4270c1c82f6e410d0a826</anchor>
      <arglist>(L_Operand *, L_Cb *, L_Operand *, L_Operand *)</arglist>
    </member>
    <member kind="function">
      <type>int</type>
      <name>L_all_src_operand_global_same_def_reachs</name>
      <anchorfile>l__opti__predicates_8h.html</anchorfile>
      <anchor>156033ea45d8a591e63e8d9c88606df4</anchor>
      <arglist>(L_Oper *, L_Cb *, L_Oper *, L_Cb *, L_Oper *)</arglist>
    </member>
    <member kind="function">
      <type>int</type>
      <name>L_global_no_defs_between_cb_only</name>
      <anchorfile>l__opti__predicates_8h.html</anchorfile>
      <anchor>feb88b1b5011274117bf1151e7053831</anchor>
      <arglist>(L_Cb *, L_Cb *, L_Operand *)</arglist>
    </member>
    <member kind="function">
      <type>int</type>
      <name>L_global_no_sub_call_between</name>
      <anchorfile>l__opti__predicates_8h.html</anchorfile>
      <anchor>f24386a1e862ab55d8f1f7852d82fc60</anchor>
      <arglist>(L_Cb *, L_Oper *, L_Cb *, L_Oper *)</arglist>
    </member>
    <member kind="function">
      <type>int</type>
      <name>L_global_no_general_sub_call_between</name>
      <anchorfile>l__opti__predicates_8h.html</anchorfile>
      <anchor>b3c56f41cffb61dc730d083270e7b4b1</anchor>
      <arglist>(L_Cb *, L_Oper *, L_Cb *, L_Oper *)</arglist>
    </member>
    <member kind="function">
      <type>int</type>
      <name>L_global_no_sync_between</name>
      <anchorfile>l__opti__predicates_8h.html</anchorfile>
      <anchor>132bf97521af13e706ac957e8ce6a432</anchor>
      <arglist>(L_Cb *, L_Oper *, L_Cb *, L_Oper *)</arglist>
    </member>
    <member kind="function">
      <type>int</type>
      <name>L_global_no_danger</name>
      <anchorfile>l__opti__predicates_8h.html</anchorfile>
      <anchor>fed7758e9b8c98d1a6e542ac844b2485</anchor>
      <arglist>(int, int, int, L_Cb *, L_Oper *, L_Cb *, L_Oper *)</arglist>
    </member>
    <member kind="function">
      <type>int</type>
      <name>L_global_no_overlap_write</name>
      <anchorfile>l__opti__predicates_8h.html</anchorfile>
      <anchor>e0fabe5f5b6734c44d2497b016321d15</anchor>
      <arglist>(L_Cb *, L_Oper *, L_Cb *, L_Oper *)</arglist>
    </member>
    <member kind="function">
      <type>int</type>
      <name>L_global_only_branch_src_operand</name>
      <anchorfile>l__opti__predicates_8h.html</anchorfile>
      <anchor>f415716f7877803f8138f6b563ede50f</anchor>
      <arglist>(L_Operand *)</arglist>
    </member>
    <member kind="function">
      <type>int</type>
      <name>L_global_no_danger_to_boundary</name>
      <anchorfile>l__opti__predicates_8h.html</anchorfile>
      <anchor>34070a84d18c13d0d4ae3e568884ac6c</anchor>
      <arglist>(int, int, int, L_Cb *, L_Oper *, L_Cb *, L_Oper *)</arglist>
    </member>
    <member kind="function">
      <type>int</type>
      <name>L_in_nested_loop</name>
      <anchorfile>l__opti__predicates_8h.html</anchorfile>
      <anchor>00de0bb6d76c32507d6bca9063c01013</anchor>
      <arglist>(L_Loop *, L_Cb *)</arglist>
    </member>
    <member kind="function">
      <type>int</type>
      <name>L_cb_dominates_all_loop_exit_cb</name>
      <anchorfile>l__opti__predicates_8h.html</anchorfile>
      <anchor>78f723b0ae86475e0f2dfb1cb1ff68a5</anchor>
      <arglist>(L_Loop *, int *, int, L_Cb *)</arglist>
    </member>
    <member kind="function">
      <type>int</type>
      <name>L_cb_dominates_all_loop_backedge_cb</name>
      <anchorfile>l__opti__predicates_8h.html</anchorfile>
      <anchor>e52ba7f9cb9f7850089f377af2d4ddb7</anchor>
      <arglist>(L_Loop *, int *, int, L_Cb *)</arglist>
    </member>
    <member kind="function">
      <type>int</type>
      <name>L_loop_invariant_operands</name>
      <anchorfile>l__opti__predicates_8h.html</anchorfile>
      <anchor>8b97bf80bcb4696275e9f8fa8cb84233</anchor>
      <arglist>(L_Loop *, int *, int, L_Oper *)</arglist>
    </member>
    <member kind="function">
      <type>int</type>
      <name>L_cost_effective_to_move_ops_to_preheader</name>
      <anchorfile>l__opti__predicates_8h.html</anchorfile>
      <anchor>3e806fdac2ed88eac60ca6dd08f186fc</anchor>
      <arglist>(L_Loop *)</arglist>
    </member>
    <member kind="function">
      <type>int</type>
      <name>L_all_uses_in_loop_from</name>
      <anchorfile>l__opti__predicates_8h.html</anchorfile>
      <anchor>37984acfdeaa0c75056fd1f09f013bd9</anchor>
      <arglist>(L_Loop *, int *, int, L_Cb *, L_Oper *)</arglist>
    </member>
    <member kind="function">
      <type>int</type>
      <name>L_def_reachs_all_out_cb_of_loop</name>
      <anchorfile>l__opti__predicates_8h.html</anchorfile>
      <anchor>b20e9792eafdb6db06a7dcec1e3bda3a</anchor>
      <arglist>(L_Loop *, int *, int, L_Cb *, L_Oper *)</arglist>
    </member>
    <member kind="function">
      <type>int</type>
      <name>L_safe_to_move_out_of_loop</name>
      <anchorfile>l__opti__predicates_8h.html</anchorfile>
      <anchor>e5be043fa7ee1626553fccf7417e728b</anchor>
      <arglist>(L_Loop *, int *, int, L_Cb *, L_Oper *)</arglist>
    </member>
    <member kind="function">
      <type>int</type>
      <name>L_no_danger_in_loop</name>
      <anchorfile>l__opti__predicates_8h.html</anchorfile>
      <anchor>44e79899e79e3c57cbc0dceca653e615</anchor>
      <arglist>(L_Loop *, int *, int, int, int, int)</arglist>
    </member>
    <member kind="function">
      <type>int</type>
      <name>L_no_memory_conflicts_in_loop</name>
      <anchorfile>l__opti__predicates_8h.html</anchorfile>
      <anchor>888fb40b7ddd51b51974e0a0cbfbe9de</anchor>
      <arglist>(L_Loop *, int *, int, L_Oper *, L_Oper *)</arglist>
    </member>
    <member kind="function">
      <type>int</type>
      <name>L_unique_memory_location</name>
      <anchorfile>l__opti__predicates_8h.html</anchorfile>
      <anchor>d9f011933b7b32514ffb44882eb2887e</anchor>
      <arglist>(L_Loop *, int *, int, L_Oper *, int *, int *, L_Oper **)</arglist>
    </member>
    <member kind="function">
      <type>int</type>
      <name>L_all_predecessor_cb_in_loop</name>
      <anchorfile>l__opti__predicates_8h.html</anchorfile>
      <anchor>cfce1c2e5220bff1c7087bb3de305493</anchor>
      <arglist>(L_Loop *, L_Cb *)</arglist>
    </member>
    <member kind="function">
      <type>int</type>
      <name>L_branch_to_loop_cb</name>
      <anchorfile>l__opti__predicates_8h.html</anchorfile>
      <anchor>4f8fd478e44fabaa027580850138979f</anchor>
      <arglist>(L_Loop *, L_Oper *)</arglist>
    </member>
    <member kind="function">
      <type>int</type>
      <name>L_basic_induction_var</name>
      <anchorfile>l__opti__predicates_8h.html</anchorfile>
      <anchor>5f73fbe5d4fc77198756b3066f36fe23</anchor>
      <arglist>(L_Loop *, L_Operand *)</arglist>
    </member>
    <member kind="function">
      <type>int</type>
      <name>L_num_constant_increment_of_ind</name>
      <anchorfile>l__opti__predicates_8h.html</anchorfile>
      <anchor>70c9c5b84426e99ee4707732b4e6f9fe</anchor>
      <arglist>(L_Operand *, L_Ind_Info *)</arglist>
    </member>
    <member kind="function">
      <type>int</type>
      <name>L_same_ind_increment</name>
      <anchorfile>l__opti__predicates_8h.html</anchorfile>
      <anchor>8a700d3ac01e24ad82bf5f2d50f6103c</anchor>
      <arglist>(L_Operand *, L_Operand *, L_Ind_Info *)</arglist>
    </member>
    <member kind="function">
      <type>int</type>
      <name>L_int_one_increment_of_ind</name>
      <anchorfile>l__opti__predicates_8h.html</anchorfile>
      <anchor>cde521e11427fae7bbf052a522ccce22</anchor>
      <arglist>(L_Operand *, L_Ind_Info *)</arglist>
    </member>
    <member kind="function">
      <type>int</type>
      <name>L_int_neg_one_increment_of_ind</name>
      <anchorfile>l__opti__predicates_8h.html</anchorfile>
      <anchor>f8841d37b35d05397b277526c9a86d4a</anchor>
      <arglist>(L_Operand *, L_Ind_Info *)</arglist>
    </member>
    <member kind="function">
      <type>int</type>
      <name>L_ind_increment_is_multiple_of</name>
      <anchorfile>l__opti__predicates_8h.html</anchorfile>
      <anchor>68948d1634f167e8c118404f0546f920</anchor>
      <arglist>(L_Operand *, L_Operand *, L_Ind_Info *)</arglist>
    </member>
    <member kind="function">
      <type>int</type>
      <name>L_num_constant_init_val_of_ind</name>
      <anchorfile>l__opti__predicates_8h.html</anchorfile>
      <anchor>80d4258007e9ffee98a4bc408a1d11a4</anchor>
      <arglist>(L_Operand *, L_Ind_Info *)</arglist>
    </member>
    <member kind="function">
      <type>int</type>
      <name>L_same_ind_initial_val</name>
      <anchorfile>l__opti__predicates_8h.html</anchorfile>
      <anchor>fa503454851ce84f92a806c72b5e6362</anchor>
      <arglist>(L_Loop *, L_Operand *, L_Operand *, L_Ind_Info *)</arglist>
    </member>
    <member kind="function">
      <type>int</type>
      <name>L_ind_var_will_reach_limit</name>
      <anchorfile>l__opti__predicates_8h.html</anchorfile>
      <anchor>cde26ccfa9eb013f45827c66587f3c98</anchor>
      <arglist>(L_Loop *, L_Oper *, L_Ind_Info *)</arglist>
    </member>
    <member kind="function">
      <type>int</type>
      <name>L_can_simplify_loop_branch</name>
      <anchorfile>l__opti__predicates_8h.html</anchorfile>
      <anchor>2017b432b897d28a7f19b188e1877072</anchor>
      <arglist>(L_Loop *, L_Oper *, L_Ind_Info *)</arglist>
    </member>
    <member kind="function">
      <type>int</type>
      <name>L_is_str_reducible_opcode</name>
      <anchorfile>l__opti__predicates_8h.html</anchorfile>
      <anchor>dd8cc26fa64631bdd294767b9511c51d</anchor>
      <arglist>(L_Oper *)</arglist>
    </member>
    <member kind="function">
      <type>int</type>
      <name>L_useful_str_red</name>
      <anchorfile>l__opti__predicates_8h.html</anchorfile>
      <anchor>d3a9050520036d7e1f684db46911c377</anchor>
      <arglist>(L_Loop *, L_Oper *, L_Operand *)</arglist>
    </member>
    <member kind="function">
      <type>int</type>
      <name>L_not_live_in_out_cb</name>
      <anchorfile>l__opti__predicates_8h.html</anchorfile>
      <anchor>1d512dd8dbb3c5067c6397a33af4b754</anchor>
      <arglist>(int *, int, L_Operand *)</arglist>
    </member>
    <member kind="function">
      <type>int</type>
      <name>L_no_uses_of_ind</name>
      <anchorfile>l__opti__predicates_8h.html</anchorfile>
      <anchor>1c4f7a26ea47cb8dd6e5fdf83d2ac2bc</anchor>
      <arglist>(L_Loop *, int *, int, L_Operand *)</arglist>
    </member>
    <member kind="function">
      <type>int</type>
      <name>L_ind_should_be_reinitialized</name>
      <anchorfile>l__opti__predicates_8h.html</anchorfile>
      <anchor>66c594f4c83f88ca81fd4394977699ce</anchor>
      <arglist>(L_Loop *, int *, int, L_Operand *)</arglist>
    </member>
    <member kind="function">
      <type>int</type>
      <name>L_basic_ind_var_in_same_family</name>
      <anchorfile>l__opti__predicates_8h.html</anchorfile>
      <anchor>cc410cc9ccf06c73f9d60a6972f47bd0</anchor>
      <arglist>(L_Loop *, int *, int, L_Operand *, L_Operand *)</arglist>
    </member>
    <member kind="function">
      <type>int</type>
      <name>L_no_uses_of_between_first_and_last_defs</name>
      <anchorfile>l__opti__predicates_8h.html</anchorfile>
      <anchor>daf219680f5042c49fb739db852d4858</anchor>
      <arglist>(L_Loop *, int *, int, L_Operand *, L_Operand *)</arglist>
    </member>
    <member kind="function">
      <type>int</type>
      <name>L_ind_constant_offset_initial_val</name>
      <anchorfile>l__opti__predicates_8h.html</anchorfile>
      <anchor>b85cbb49d7cfbb6d5ab823a331b4ae5c</anchor>
      <arglist>(L_Cb *, L_Operand *, L_Operand *, L_Oper *, L_Oper *, L_Ind_Info *)</arglist>
    </member>
    <member kind="function">
      <type>int</type>
      <name>L_only_used_as_base_addr_with_const_offset_in_loop</name>
      <anchorfile>l__opti__predicates_8h.html</anchorfile>
      <anchor>cb8f5e4fc5b9687cf981b42d940f28a9</anchor>
      <arglist>(L_Loop *, int *, int, L_Operand *, int)</arglist>
    </member>
    <member kind="function">
      <type>int</type>
      <name>L_all_uses_of_ind_can_change_offset</name>
      <anchorfile>l__opti__predicates_8h.html</anchorfile>
      <anchor>57e4c877453aa3d6af227ffb468b9474</anchor>
      <arglist>(L_Loop *, int *, int, int *, int, L_Operand *, L_Operand *)</arglist>
    </member>
    <member kind="function">
      <type>int</type>
      <name>L_can_modify_dep_branches_in_loop</name>
      <anchorfile>l__opti__predicates_8h.html</anchorfile>
      <anchor>1b1d7af86751b03d9a292a6ed5cdbb96</anchor>
      <arglist>(L_Loop *, int *, int, L_Oper *, L_Cb *)</arglist>
    </member>
    <member kind="function">
      <type>int</type>
      <name>L_ind_only_used_with_loop_inv_operands</name>
      <anchorfile>l__opti__predicates_8h.html</anchorfile>
      <anchor>08c0ef29dc257aa4abbfd5efc92fd42a</anchor>
      <arglist>(L_Loop *, int *, int, L_Operand *)</arglist>
    </member>
    <member kind="function">
      <type>int</type>
      <name>L_ind_only_used_with_memory_ops</name>
      <anchorfile>l__opti__predicates_8h.html</anchorfile>
      <anchor>06731647519bd21e2b9bf3ec83d5c33c</anchor>
      <arglist>(L_Loop *, int *, int, L_Operand *)</arglist>
    </member>
    <member kind="function">
      <type>int</type>
      <name>L_all_uses_of_ind_can_be_modified1</name>
      <anchorfile>l__opti__predicates_8h.html</anchorfile>
      <anchor>6a33b39952b9ce3ae29f5210c999139c</anchor>
      <arglist>(L_Loop *, int *, int, L_Operand *)</arglist>
    </member>
    <member kind="function">
      <type>int</type>
      <name>L_all_uses_of_ind_can_be_modified2</name>
      <anchorfile>l__opti__predicates_8h.html</anchorfile>
      <anchor>5d3c9defaf8171bf6e3f46ad616ed779</anchor>
      <arglist>(L_Loop *, int *, int, L_Operand *)</arglist>
    </member>
    <member kind="function">
      <type>int</type>
      <name>L_better_to_eliminate_operand2</name>
      <anchorfile>l__opti__predicates_8h.html</anchorfile>
      <anchor>ed1f379033e7e1c01e77d357c0bb5131</anchor>
      <arglist>(L_Loop *, int *, int, int *, int, L_Operand *, L_Operand *)</arglist>
    </member>
    <member kind="function">
      <type>int</type>
      <name>L_cost_effective_for_ind_complex_elim</name>
      <anchorfile>l__opti__predicates_8h.html</anchorfile>
      <anchor>15bec240e6c580bd3e2f83330c94b518</anchor>
      <arglist>(L_Loop *, int *, int)</arglist>
    </member>
    <member kind="function">
      <type>int</type>
      <name>L_live_at_nondominated_exits</name>
      <anchorfile>l__opti__predicates_8h.html</anchorfile>
      <anchor>dbac02053eee47fed24e3ba87e4763aa</anchor>
      <arglist>(L_Loop *, int *, int, L_Cb *, L_Operand *)</arglist>
    </member>
    <member kind="function">
      <type>int</type>
      <name>L_is_loop_var</name>
      <anchorfile>l__opti__predicates_8h.html</anchorfile>
      <anchor>80efbbdae69f5202d00ceeef45007cde</anchor>
      <arglist>(L_Loop *, int *, int, L_Operand *, L_Ind_Info *ind_info)</arglist>
    </member>
    <member kind="function">
      <type>int</type>
      <name>L_empty_block</name>
      <anchorfile>l__opti__predicates_8h.html</anchorfile>
      <anchor>2f5c1727e4772fabf3796b911cd8a0d8</anchor>
      <arglist>(L_Cb *)</arglist>
    </member>
    <member kind="function">
      <type>int</type>
      <name>L_only_uncond_branch_in_block</name>
      <anchorfile>l__opti__predicates_8h.html</anchorfile>
      <anchor>7e2c35e0cc836623ab4f44e13a702194</anchor>
      <arglist>(L_Cb *)</arglist>
    </member>
    <member kind="function">
      <type>int</type>
      <name>L_multiple_cond_branch_in_block</name>
      <anchorfile>l__opti__predicates_8h.html</anchorfile>
      <anchor>f987624a82f3597a50e50bb1babe7bf0</anchor>
      <arglist>(L_Cb *)</arglist>
    </member>
    <member kind="function">
      <type>int</type>
      <name>L_cb_contains_prologue</name>
      <anchorfile>l__opti__predicates_8h.html</anchorfile>
      <anchor>76f8ce75680baaf28f1f360c51fb04f7</anchor>
      <arglist>(L_Cb *)</arglist>
    </member>
    <member kind="function">
      <type>int</type>
      <name>L_cb_contains_epilogue</name>
      <anchorfile>l__opti__predicates_8h.html</anchorfile>
      <anchor>d12da31fb829fd44b7fed1e7aaa4cf98</anchor>
      <arglist>(L_Cb *)</arglist>
    </member>
    <member kind="function">
      <type>int</type>
      <name>L_cb_contains_multiple_branches_to</name>
      <anchorfile>l__opti__predicates_8h.html</anchorfile>
      <anchor>76db2abb9244451b5c6aefff55a08021</anchor>
      <arglist>(L_Cb *, L_Cb *)</arglist>
    </member>
    <member kind="function">
      <type>int</type>
      <name>L_need_fallthru_path</name>
      <anchorfile>l__opti__predicates_8h.html</anchorfile>
      <anchor>a126d011b50e2e3af72d24c8790b803b</anchor>
      <arglist>(L_Cb *)</arglist>
    </member>
    <member kind="function">
      <type>int</type>
      <name>L_marked_as_post_increment</name>
      <anchorfile>l__opti__predicates_8h.html</anchorfile>
      <anchor>241038769154a341ee494e32b1052123</anchor>
      <arglist>(L_Oper *)</arglist>
    </member>
    <member kind="function">
      <type>int</type>
      <name>L_marked_as_pre_increment</name>
      <anchorfile>l__opti__predicates_8h.html</anchorfile>
      <anchor>ac4601b1a6ec29e164226d6499ee0110</anchor>
      <arglist>(L_Oper *)</arglist>
    </member>
    <member kind="function">
      <type>int</type>
      <name>L_marked_as_pre_post_increment</name>
      <anchorfile>l__opti__predicates_8h.html</anchorfile>
      <anchor>4aa28a4b177cf47cb3b4a8570c09192b</anchor>
      <arglist>(L_Oper *)</arglist>
    </member>
    <member kind="function">
      <type>int</type>
      <name>L_can_recombine_mem_inc_ops</name>
      <anchorfile>l__opti__predicates_8h.html</anchorfile>
      <anchor>f7f3a298e6891193cc4a74f1bb55b710</anchor>
      <arglist>(L_Oper *, L_Oper *)</arglist>
    </member>
    <member kind="function">
      <type>int</type>
      <name>L_can_recombine_inc_mem_ops</name>
      <anchorfile>l__opti__predicates_8h.html</anchorfile>
      <anchor>368ad08912973c0d0de3ae9fc5cb2543</anchor>
      <arglist>(L_Oper *, L_Oper *)</arglist>
    </member>
    <member kind="function">
      <type>int</type>
      <name>L_can_make_post_inc</name>
      <anchorfile>l__opti__predicates_8h.html</anchorfile>
      <anchor>f71b7bb3bea52ae3794799064eb8e38f</anchor>
      <arglist>(L_Oper *, L_Oper *)</arglist>
    </member>
    <member kind="function">
      <type>int</type>
      <name>L_can_make_pre_inc</name>
      <anchorfile>l__opti__predicates_8h.html</anchorfile>
      <anchor>b6cfa7d9c4c355725d0e79f168c41297</anchor>
      <arglist>(L_Oper *, L_Oper *)</arglist>
    </member>
    <member kind="function">
      <type>int</type>
      <name>L_ind_var_is_updated_by_pre_post_increment</name>
      <anchorfile>l__opti__predicates_8h.html</anchorfile>
      <anchor>a1bbe08e403ec633b6fe666dc20603d3</anchor>
      <arglist>(L_Loop *, int *, int, L_Operand *)</arglist>
    </member>
    <member kind="function">
      <type>int</type>
      <name>L_extension_compatible_ctype</name>
      <anchorfile>l__opti__predicates_8h.html</anchorfile>
      <anchor>7142116794ff3af9ae92b8ede1d1a03b</anchor>
      <arglist>(ITuint8 from, ITuint8 to)</arglist>
    </member>
    <member kind="function">
      <type>int</type>
      <name>L_redundant_extension_rev</name>
      <anchorfile>l__opti__predicates_8h.html</anchorfile>
      <anchor>3f5ce123e9a95a7a368066f70b568aae</anchor>
      <arglist>(L_Oper *src_op, L_Oper *ext_op)</arglist>
    </member>
    <member kind="function">
      <type>int</type>
      <name>L_redundant_extension_fwd</name>
      <anchorfile>l__opti__predicates_8h.html</anchorfile>
      <anchor>b813608b50f2d3c549445902b1e1e07a</anchor>
      <arglist>(L_Oper *ext_op, L_Oper *dst_op)</arglist>
    </member>
    <member kind="function">
      <type>int</type>
      <name>L_cb_first_flow_const_compare_branch</name>
      <anchorfile>l__opti__predicates_8h.html</anchorfile>
      <anchor>23eb452feef213fba8de8fad2829fa77</anchor>
      <arglist>(L_Cb *, L_Operand **, L_Operand **)</arglist>
    </member>
    <member kind="function">
      <type>int</type>
      <name>L_cb_all_incoming_flows_same_const_compare</name>
      <anchorfile>l__opti__predicates_8h.html</anchorfile>
      <anchor>c7633f9f6e662171817f351e24c6d829</anchor>
      <arglist>(L_Cb *, L_Operand *, L_Operand *)</arglist>
    </member>
  </compound>
  <compound kind="file">
    <name>l_opti_tag_load.c</name>
    <path>/mnt/hgfs/mkt/Desktop/openimpact/src/Lcode/opti/Lopti/</path>
    <filename>l__opti__tag__load_8c</filename>
    <includes id="l__opti_8h" name="l_opti.h" local="yes" imported="no">l_opti.h</includes>
    <member kind="function">
      <type>void</type>
      <name>L_tag_load</name>
      <anchorfile>l__opti__tag__load_8c.html</anchorfile>
      <anchor>fa386e5524764aa65018f4dd76f9d8b4</anchor>
      <arglist>(L_Func *fn)</arglist>
    </member>
  </compound>
  <compound kind="file">
    <name>l_optimize.c</name>
    <path>/mnt/hgfs/mkt/Desktop/openimpact/src/Lcode/opti/Lopti/</path>
    <filename>l__optimize_8c</filename>
    <includes id="l__opti_8h" name="l_opti.h" local="yes" imported="no">l_opti.h</includes>
    <includes id="l__disjvreg_8h" name="l_disjvreg.h" local="yes" imported="no">l_disjvreg.h</includes>
    <member kind="define">
      <type>#define</type>
      <name>MAX_ITER</name>
      <anchorfile>l__optimize_8c.html</anchorfile>
      <anchor>cd517c6f195c75b9dd0f3aad65326f3b</anchor>
      <arglist></arglist>
    </member>
    <member kind="define">
      <type>#define</type>
      <name>MAX_ITER1</name>
      <anchorfile>l__optimize_8c.html</anchorfile>
      <anchor>f093f95d436cc69c31418f62cd81b083</anchor>
      <arglist></arglist>
    </member>
    <member kind="function">
      <type>int</type>
      <name>L_oper_breakdown</name>
      <anchorfile>l__optimize_8c.html</anchorfile>
      <anchor>43234e235368abda703cb9153f560dde</anchor>
      <arglist>(L_Func *)</arglist>
    </member>
    <member kind="function">
      <type>int</type>
      <name>L_local_code_optimization</name>
      <anchorfile>l__optimize_8c.html</anchorfile>
      <anchor>d65a1d31ab10f9a58f0e4af63ff5628e</anchor>
      <arglist>(L_Func *, int)</arglist>
    </member>
    <member kind="function">
      <type>int</type>
      <name>L_global_dead_code_optimization</name>
      <anchorfile>l__optimize_8c.html</anchorfile>
      <anchor>0145b59639285a8450214f71c15ed7c2</anchor>
      <arglist>(L_Func *)</arglist>
    </member>
    <member kind="function">
      <type>int</type>
      <name>L_global_code_optimization</name>
      <anchorfile>l__optimize_8c.html</anchorfile>
      <anchor>2a9c6ae7583dc3f3d40c220c1b377054</anchor>
      <arglist>(L_Func *)</arglist>
    </member>
    <member kind="function">
      <type>int</type>
      <name>L_partial_code_elimination</name>
      <anchorfile>l__optimize_8c.html</anchorfile>
      <anchor>51d7df7a94b2b91ce7630a9c1fe3ea87</anchor>
      <arglist>(L_Func *)</arglist>
    </member>
    <member kind="function">
      <type>int</type>
      <name>L_PRE_optimization</name>
      <anchorfile>l__optimize_8c.html</anchorfile>
      <anchor>10ce87d5dbb15320841bf2dfd66d04c7</anchor>
      <arglist>(L_Func *)</arglist>
    </member>
    <member kind="function">
      <type>int</type>
      <name>L_partial_dead_code_elimination</name>
      <anchorfile>l__optimize_8c.html</anchorfile>
      <anchor>8310b93d7f87a8835c30c9422aeea044</anchor>
      <arglist>(L_Func *)</arglist>
    </member>
    <member kind="function">
      <type>int</type>
      <name>L_PCE_cleanup</name>
      <anchorfile>l__optimize_8c.html</anchorfile>
      <anchor>4fc1af2c76863ee2516417aaa16e830f</anchor>
      <arglist>(L_Func *, int)</arglist>
    </member>
    <member kind="function">
      <type>int</type>
      <name>L_min_cut_PDE</name>
      <anchorfile>l__optimize_8c.html</anchorfile>
      <anchor>2e5f2914b805e9360f920e5ba8163ebb</anchor>
      <arglist>(L_Func *)</arglist>
    </member>
    <member kind="function">
      <type>int</type>
      <name>L_jump_initial_cleanup</name>
      <anchorfile>l__optimize_8c.html</anchorfile>
      <anchor>2e17badffc527042447ce39a91e7e290</anchor>
      <arglist>(L_Func *)</arglist>
    </member>
    <member kind="function">
      <type>int</type>
      <name>L_jump_optimization</name>
      <anchorfile>l__optimize_8c.html</anchorfile>
      <anchor>763a770f48d3ee2f710e4e6e4a73250c</anchor>
      <arglist>(L_Func *)</arglist>
    </member>
    <member kind="function">
      <type>int</type>
      <name>L_loop_optimization</name>
      <anchorfile>l__optimize_8c.html</anchorfile>
      <anchor>c113f5d4afcf9707f534cd1a440b5442</anchor>
      <arglist>(L_Func *)</arglist>
    </member>
    <member kind="function">
      <type>void</type>
      <name>L_oper_recombine</name>
      <anchorfile>l__optimize_8c.html</anchorfile>
      <anchor>6659008fd8623935c3fb399c5a7c0edf</anchor>
      <arglist>(L_Func *)</arglist>
    </member>
    <member kind="function">
      <type>int</type>
      <name>L_unification</name>
      <anchorfile>l__optimize_8c.html</anchorfile>
      <anchor>7f9127c58f0054a678c098c91f3c4e44</anchor>
      <arglist>(L_Func *)</arglist>
    </member>
    <member kind="function">
      <type>int</type>
      <name>L_global_mem_expression_copy_prop</name>
      <anchorfile>l__optimize_8c.html</anchorfile>
      <anchor>b834b454ca94bb5f81ee9950cdb47b9a</anchor>
      <arglist>(L_Func *)</arglist>
    </member>
    <member kind="function">
      <type>int</type>
      <name>L_global_dead_store_removal</name>
      <anchorfile>l__optimize_8c.html</anchorfile>
      <anchor>dacf4f42dd8b311a22a0ea90c93775c1</anchor>
      <arglist>(L_Func *)</arglist>
    </member>
    <member kind="function">
      <type>void</type>
      <name>L_code_optimize</name>
      <anchorfile>l__optimize_8c.html</anchorfile>
      <anchor>931eb765543a8e23916ed72f308a5672</anchor>
      <arglist>(L_Func *fn)</arglist>
    </member>
  </compound>
  <compound kind="file">
    <name>l_PCE_driver.c</name>
    <path>/mnt/hgfs/mkt/Desktop/openimpact/src/Lcode/opti/Lopti/</path>
    <filename>l__PCE__driver_8c</filename>
    <includes id="l__opti_8h" name="l_opti.h" local="yes" imported="no">l_opti.h</includes>
    <member kind="define">
      <type>#define</type>
      <name>MAX_PRE_ITER</name>
      <anchorfile>l__PCE__driver_8c.html</anchorfile>
      <anchor>13520b0b61fd1fba302ada0fabf5d306</anchor>
      <arglist></arglist>
    </member>
    <member kind="function">
      <type>int</type>
      <name>L_jump_optimization</name>
      <anchorfile>l__PCE__driver_8c.html</anchorfile>
      <anchor>763a770f48d3ee2f710e4e6e4a73250c</anchor>
      <arglist>(L_Func *)</arglist>
    </member>
    <member kind="function">
      <type>int</type>
      <name>L_partial_code_elimination</name>
      <anchorfile>l__PCE__driver_8c.html</anchorfile>
      <anchor>390aa7a833fa45400a4f55430dbfb081</anchor>
      <arglist>(L_Func *fn)</arglist>
    </member>
    <member kind="function">
      <type>int</type>
      <name>L_partial_dead_code_optimization</name>
      <anchorfile>l__PCE__driver_8c.html</anchorfile>
      <anchor>0a130f3009283871eb33fc1437108ef6</anchor>
      <arglist>(L_Func *fn)</arglist>
    </member>
    <member kind="function">
      <type>int</type>
      <name>L_PRE_optimization</name>
      <anchorfile>l__PCE__driver_8c.html</anchorfile>
      <anchor>3a1e91dbeaaf5b39e874a167ac3636b9</anchor>
      <arglist>(L_Func *fn)</arglist>
    </member>
  </compound>
  <compound kind="file">
    <name>l_PCE_opti.c</name>
    <path>/mnt/hgfs/mkt/Desktop/openimpact/src/Lcode/opti/Lopti/</path>
    <filename>l__PCE__opti_8c</filename>
    <includes id="l__opti_8h" name="l_opti.h" local="yes" imported="no">l_opti.h</includes>
    <member kind="define">
      <type>#define</type>
      <name>MAX_PCE_LOCAL_CLEANUP_ITER</name>
      <anchorfile>l__PCE__opti_8c.html</anchorfile>
      <anchor>67c64a3805c44dadf5076e9e1cdf5934</anchor>
      <arglist></arglist>
    </member>
    <member kind="define">
      <type>#define</type>
      <name>MAX_PDE_ITER</name>
      <anchorfile>l__PCE__opti_8c.html</anchorfile>
      <anchor>0efaa24a78dd935067779bc7d89dd4f6</anchor>
      <arglist></arglist>
    </member>
    <member kind="define">
      <type>#define</type>
      <name>P3DE_GENERAL_STORE_MOTION</name>
      <anchorfile>l__PCE__opti_8c.html</anchorfile>
      <anchor>b1ddf41e17fcaad2bef62e0cd02f3f97</anchor>
      <arglist></arglist>
    </member>
    <member kind="function">
      <type>void</type>
      <name>L_do_SPRE_analysis</name>
      <anchorfile>l__PCE__opti_8c.html</anchorfile>
      <anchor>336c68c905a261048f6c5cb8ddb2a76e</anchor>
      <arglist>(L_Func *, int, int)</arglist>
    </member>
    <member kind="function">
      <type>void</type>
      <name>L_do_P3DE_analysis</name>
      <anchorfile>l__PCE__opti_8c.html</anchorfile>
      <anchor>1e22ffb195b33b2308fc22a65598d8cc</anchor>
      <arglist>(L_Func *, int, int)</arglist>
    </member>
    <member kind="function" static="yes">
      <type>static int</type>
      <name>L_split_cb_critical_edges</name>
      <anchorfile>l__PCE__opti_8c.html</anchorfile>
      <anchor>ed6ee9a6a05ee95a5dae7cfe78a72125</anchor>
      <arglist>(L_Cb *cb)</arglist>
    </member>
    <member kind="function">
      <type>int</type>
      <name>L_split_fn_critical_edges</name>
      <anchorfile>l__PCE__opti_8c.html</anchorfile>
      <anchor>9165019b4005eee2d9b240f392411598</anchor>
      <arglist>(L_Func *fn)</arglist>
    </member>
    <member kind="function">
      <type>int</type>
      <name>L_split_fn_loop_out_critical_edges</name>
      <anchorfile>l__PCE__opti_8c.html</anchorfile>
      <anchor>c8f7377373e1de3a4eba4140ea819969</anchor>
      <arglist>(L_Func *fn)</arglist>
    </member>
    <member kind="function">
      <type>int</type>
      <name>L_split_fn_loopback_critical_edges</name>
      <anchorfile>l__PCE__opti_8c.html</anchorfile>
      <anchor>e597a0ab161e725babc222fe877a3418</anchor>
      <arglist>(L_Func *fn)</arglist>
    </member>
    <member kind="function" static="yes">
      <type>static L_Oper *</type>
      <name>L_PRE_n_insert_replace</name>
      <anchorfile>l__PCE__opti_8c.html</anchorfile>
      <anchor>ff529cb5a7a29bc174eb17c32232e1c0</anchor>
      <arglist>(int expression_index, L_Cb *cb, L_Oper *first_op, Set *oper_insert)</arglist>
    </member>
    <member kind="function" static="yes">
      <type>static L_Oper *</type>
      <name>L_PRE_x_insert_replace</name>
      <anchorfile>l__PCE__opti_8c.html</anchorfile>
      <anchor>49a32a19c8cb52f6d195f7e925e692b2</anchor>
      <arglist>(int expression_index, L_Cb *cb, L_Oper *last_op, Set *oper_insert, Set *oper_new_insert, int complement_flag)</arglist>
    </member>
    <member kind="function" static="yes">
      <type>static L_Oper *</type>
      <name>L_PRE_n_insert</name>
      <anchorfile>l__PCE__opti_8c.html</anchorfile>
      <anchor>ebebbb52c86c4f6e2e15f887113f468e</anchor>
      <arglist>(int expression_index, L_Cb *cb, L_Oper *first_op, Set *oper_insert, int speculate_flag)</arglist>
    </member>
    <member kind="function" static="yes">
      <type>static L_Oper *</type>
      <name>L_PRE_x_insert</name>
      <anchorfile>l__PCE__opti_8c.html</anchorfile>
      <anchor>9b7a7bef4c366d0d88dacd6711737dfc</anchor>
      <arglist>(int expression_index, L_Cb *cb, L_Oper *last_op, Set *oper_insert, int speculate_flag)</arglist>
    </member>
    <member kind="function" static="yes">
      <type>static L_Oper *</type>
      <name>L_PRE_n_replace</name>
      <anchorfile>l__PCE__opti_8c.html</anchorfile>
      <anchor>da8ef88056a54e2f97b7efdb53aafdb0</anchor>
      <arglist>(int expression_index, L_Cb *cb, L_Oper *first_op, Set *oper_replace)</arglist>
    </member>
    <member kind="function" static="yes">
      <type>static void</type>
      <name>L_PRE_correct_spec_and_sync_info</name>
      <anchorfile>l__PCE__opti_8c.html</anchorfile>
      <anchor>395573a9e4f7230dd0abb3cb90c9d600</anchor>
      <arglist>(Set *all_insert, Set *new_insert, Set *replace)</arglist>
    </member>
    <member kind="function">
      <type>int</type>
      <name>L_PRE_lazy_code_motion</name>
      <anchorfile>l__PCE__opti_8c.html</anchorfile>
      <anchor>a41dc66cc45513dbc6fbea7e48ad837e</anchor>
      <arglist>(L_Func *fn)</arglist>
    </member>
    <member kind="function">
      <type>int</type>
      <name>L_partial_redundancy_elimination</name>
      <anchorfile>l__PCE__opti_8c.html</anchorfile>
      <anchor>a900f7b2ab31e5989fc92e1e83a26c75</anchor>
      <arglist>(L_Func *fn)</arglist>
    </member>
    <member kind="function" static="yes">
      <type>static int</type>
      <name>L_PRE_speculative_code_motion</name>
      <anchorfile>l__PCE__opti_8c.html</anchorfile>
      <anchor>a4daf08030217a5dadaacc6eda576700</anchor>
      <arglist>(L_Func *fn)</arglist>
    </member>
    <member kind="function">
      <type>int</type>
      <name>L_speculative_PRE</name>
      <anchorfile>l__PCE__opti_8c.html</anchorfile>
      <anchor>9aead0f83a8da8a7b37d6506f0ade262</anchor>
      <arglist>(L_Func *fn)</arglist>
    </member>
    <member kind="function" static="yes">
      <type>static L_Oper *</type>
      <name>L_PDE_insert</name>
      <anchorfile>l__PCE__opti_8c.html</anchorfile>
      <anchor>b80ae81d698a10ab659f134271d86ab5</anchor>
      <arglist>(int assignment_index, L_Cb *cb, L_Oper *first_op, Set *oper_insert, Set *oper_generalized, int pred_guard)</arglist>
    </member>
    <member kind="function" static="yes">
      <type>static L_Oper *</type>
      <name>L_PDE_delete</name>
      <anchorfile>l__PCE__opti_8c.html</anchorfile>
      <anchor>431abc97ec4b46c9d3f7690733d48d95</anchor>
      <arglist>(int assignment_index, L_Cb *cb, L_Oper *last_op, Set *oper_delete, int pred_set)</arglist>
    </member>
    <member kind="function" static="yes">
      <type>static L_Oper *</type>
      <name>L_PDE_clear_pred</name>
      <anchorfile>l__PCE__opti_8c.html</anchorfile>
      <anchor>d515373bbdf6ad6e665776a1dc92a121</anchor>
      <arglist>(int assignment_index, L_Cb *cb, L_Oper *last_op, Set *oper_load_insert)</arglist>
    </member>
    <member kind="function" static="yes">
      <type>static void</type>
      <name>L_PDE_correct_spec_and_sync_info</name>
      <anchorfile>l__PCE__opti_8c.html</anchorfile>
      <anchor>b0c74c3b9551ce2aad0b297ff696b1e6</anchor>
      <arglist>(Set *insert, Set *delete, Set *generalized)</arglist>
    </member>
    <member kind="function" static="yes">
      <type>static void</type>
      <name>L_PDE_correct_inserted_load_sync_info</name>
      <anchorfile>l__PCE__opti_8c.html</anchorfile>
      <anchor>6edc8cf37f5b8fcab06158ef2f39a883</anchor>
      <arglist>(Set *generalized, Set *load_insert)</arglist>
    </member>
    <member kind="function">
      <type>int</type>
      <name>L_PDE_code_motion</name>
      <anchorfile>l__PCE__opti_8c.html</anchorfile>
      <anchor>8a65cd1b6eae2da4fdbc501c53b6089e</anchor>
      <arglist>(L_Func *fn)</arglist>
    </member>
    <member kind="function">
      <type>int</type>
      <name>L_partial_dead_code_elimination</name>
      <anchorfile>l__PCE__opti_8c.html</anchorfile>
      <anchor>bec7c669bd43d3a35a610981ab264999</anchor>
      <arglist>(L_Func *fn)</arglist>
    </member>
    <member kind="function" static="yes">
      <type>static int</type>
      <name>L_PDE_predicated_code_motion</name>
      <anchorfile>l__PCE__opti_8c.html</anchorfile>
      <anchor>e647b8212973c19fdeeb738c0bd2fc0f</anchor>
      <arglist>(L_Func *fn)</arglist>
    </member>
    <member kind="function">
      <type>int</type>
      <name>L_PDE_combine_pred_guards</name>
      <anchorfile>l__PCE__opti_8c.html</anchorfile>
      <anchor>590bb3bf45c7ae696c6fb318686984fe</anchor>
      <arglist>(L_Func *fn)</arglist>
    </member>
    <member kind="function">
      <type>int</type>
      <name>L_min_cut_PDE</name>
      <anchorfile>l__PCE__opti_8c.html</anchorfile>
      <anchor>b2b7635e11eeede4465d724e82aa51f3</anchor>
      <arglist>(L_Func *fn)</arglist>
    </member>
    <member kind="function">
      <type>int</type>
      <name>L_coalesce_cbs</name>
      <anchorfile>l__PCE__opti_8c.html</anchorfile>
      <anchor>d85bc3a60be840461a2b9978c28d7e82</anchor>
      <arglist>(L_Func *fn, int jump_cleanup_flag)</arglist>
    </member>
    <member kind="function">
      <type>void</type>
      <name>L_PCE_disable_subsumed_optis</name>
      <anchorfile>l__PCE__opti_8c.html</anchorfile>
      <anchor>e33123a5266b57a7535642cced548217</anchor>
      <arglist>()</arglist>
    </member>
    <member kind="function">
      <type>void</type>
      <name>L_PCE_fix_function_weight</name>
      <anchorfile>l__PCE__opti_8c.html</anchorfile>
      <anchor>b861d9c2b135a13ea6d5fa9bf57ae1a1</anchor>
      <arglist>()</arglist>
    </member>
    <member kind="function">
      <type>int</type>
      <name>L_PCE_merge_same_cbs</name>
      <anchorfile>l__PCE__opti_8c.html</anchorfile>
      <anchor>1477cb6c86ddc015b9c7448c34f44dd3</anchor>
      <arglist>(L_Func *fn)</arglist>
    </member>
    <member kind="function">
      <type>int</type>
      <name>L_PCE_cleanup</name>
      <anchorfile>l__PCE__opti_8c.html</anchorfile>
      <anchor>333f0901ae479b06638b1bf51e568abe</anchor>
      <arglist>(L_Func *fn, int mov_flag)</arglist>
    </member>
  </compound>
  <compound kind="file">
    <name>l_PCE_opti.h</name>
    <path>/mnt/hgfs/mkt/Desktop/openimpact/src/Lcode/opti/Lopti/</path>
    <filename>l__PCE__opti_8h</filename>
    <member kind="function">
      <type>int</type>
      <name>L_split_fn_critical_edges</name>
      <anchorfile>l__PCE__opti_8h.html</anchorfile>
      <anchor>8de4199a89c4b1a0caa90a2e1932d8ac</anchor>
      <arglist>(L_Func *)</arglist>
    </member>
    <member kind="function">
      <type>int</type>
      <name>L_split_fn_loop_out_critical_edges</name>
      <anchorfile>l__PCE__opti_8h.html</anchorfile>
      <anchor>d5f836cc06e8b11a81562bcace1ac7be</anchor>
      <arglist>(L_Func *)</arglist>
    </member>
    <member kind="function">
      <type>int</type>
      <name>L_split_fn_loopback_critical_edges</name>
      <anchorfile>l__PCE__opti_8h.html</anchorfile>
      <anchor>5afc1102edd33298b1442717b76c688a</anchor>
      <arglist>(L_Func *)</arglist>
    </member>
    <member kind="function">
      <type>int</type>
      <name>L_PRE_lazy_code_motion</name>
      <anchorfile>l__PCE__opti_8h.html</anchorfile>
      <anchor>dfee0bc4d1fdb96f1e8a075ba0639d57</anchor>
      <arglist>(L_Func *)</arglist>
    </member>
    <member kind="function">
      <type>int</type>
      <name>L_partial_redundancy_elimination</name>
      <anchorfile>l__PCE__opti_8h.html</anchorfile>
      <anchor>22a4c764048282bf089fce95b9f6cc1c</anchor>
      <arglist>(L_Func *)</arglist>
    </member>
    <member kind="function">
      <type>int</type>
      <name>L_speculative_PRE</name>
      <anchorfile>l__PCE__opti_8h.html</anchorfile>
      <anchor>3c397b646ed4af7afedeece58cc665bb</anchor>
      <arglist>(L_Func *)</arglist>
    </member>
    <member kind="function">
      <type>int</type>
      <name>L_PDE_code_motion</name>
      <anchorfile>l__PCE__opti_8h.html</anchorfile>
      <anchor>d7b977bd4c836310315d1048d695c9f6</anchor>
      <arglist>(L_Func *)</arglist>
    </member>
    <member kind="function">
      <type>int</type>
      <name>L_partial_dead_code_elimination</name>
      <anchorfile>l__PCE__opti_8h.html</anchorfile>
      <anchor>8310b93d7f87a8835c30c9422aeea044</anchor>
      <arglist>(L_Func *)</arglist>
    </member>
    <member kind="function">
      <type>int</type>
      <name>L_PDE_combine_pred_guards</name>
      <anchorfile>l__PCE__opti_8h.html</anchorfile>
      <anchor>838d8e3643e291a556d50a0583a2f467</anchor>
      <arglist>(L_Func *)</arglist>
    </member>
    <member kind="function">
      <type>int</type>
      <name>L_min_cut_PDE</name>
      <anchorfile>l__PCE__opti_8h.html</anchorfile>
      <anchor>2e5f2914b805e9360f920e5ba8163ebb</anchor>
      <arglist>(L_Func *)</arglist>
    </member>
    <member kind="function">
      <type>int</type>
      <name>L_coalesce_cbs</name>
      <anchorfile>l__PCE__opti_8h.html</anchorfile>
      <anchor>3319f7768eb484edff6996aa2bb03e6c</anchor>
      <arglist>(L_Func *, int)</arglist>
    </member>
    <member kind="function">
      <type>void</type>
      <name>L_PCE_disable_subsumed_optis</name>
      <anchorfile>l__PCE__opti_8h.html</anchorfile>
      <anchor>e33123a5266b57a7535642cced548217</anchor>
      <arglist>()</arglist>
    </member>
    <member kind="function">
      <type>void</type>
      <name>L_PCE_fix_function_weight</name>
      <anchorfile>l__PCE__opti_8h.html</anchorfile>
      <anchor>b861d9c2b135a13ea6d5fa9bf57ae1a1</anchor>
      <arglist>()</arglist>
    </member>
    <member kind="function">
      <type>int</type>
      <name>L_PCE_merge_same_cbs</name>
      <anchorfile>l__PCE__opti_8h.html</anchorfile>
      <anchor>d0af6f4283038e49ea0d2fab437bc164</anchor>
      <arglist>(L_Func *)</arglist>
    </member>
    <member kind="function">
      <type>int</type>
      <name>L_PCE_cleanup</name>
      <anchorfile>l__PCE__opti_8h.html</anchorfile>
      <anchor>4fc1af2c76863ee2516417aaa16e830f</anchor>
      <arglist>(L_Func *, int)</arglist>
    </member>
  </compound>
  <compound kind="file">
    <name>l_pred_opti.c</name>
    <path>/mnt/hgfs/mkt/Desktop/openimpact/src/Lcode/opti/Lopti/</path>
    <filename>l__pred__opti_8c</filename>
    <includes id="l__opti_8h" name="l_opti.h" local="yes" imported="no">l_opti.h</includes>
    <includes id="l__promotion_8h" name="l_promotion.h" local="yes" imported="no">l_promotion.h</includes>
    <member kind="function">
      <type>int</type>
      <name>L_local_pred_cse</name>
      <anchorfile>l__pred__opti_8c.html</anchorfile>
      <anchor>f9a1721cfa7bb13c099ce805822d3773</anchor>
      <arglist>(L_Cb *cb)</arglist>
    </member>
    <member kind="function">
      <type>int</type>
      <name>L_remove_red_guards</name>
      <anchorfile>l__pred__opti_8c.html</anchorfile>
      <anchor>794d328f7542201077e30fa35e8e6707</anchor>
      <arglist>(L_Func *fn)</arglist>
    </member>
    <member kind="function">
      <type>int</type>
      <name>L_form_and_chains</name>
      <anchorfile>l__pred__opti_8c.html</anchorfile>
      <anchor>4ce57d626ae9f852202bd4b15837e447</anchor>
      <arglist>(L_Func *fn)</arglist>
    </member>
    <member kind="function" static="yes">
      <type>static int</type>
      <name>L_remove_red_cmps</name>
      <anchorfile>l__pred__opti_8c.html</anchorfile>
      <anchor>bc8077f6eb41c76829369f73db84b9a2</anchor>
      <arglist>(L_Func *fn)</arglist>
    </member>
    <member kind="function">
      <type>int</type>
      <name>L_lightweight_pred_opti</name>
      <anchorfile>l__pred__opti_8c.html</anchorfile>
      <anchor>9e22e0e70fb4e9b2e7f2c3e3d65b6296</anchor>
      <arglist>(L_Func *fn)</arglist>
    </member>
  </compound>
  <compound kind="file">
    <name>l_pred_opti.h</name>
    <path>/mnt/hgfs/mkt/Desktop/openimpact/src/Lcode/opti/Lopti/</path>
    <filename>l__pred__opti_8h</filename>
    <member kind="function">
      <type>int</type>
      <name>L_lightweight_pred_opti</name>
      <anchorfile>l__pred__opti_8h.html</anchorfile>
      <anchor>9e22e0e70fb4e9b2e7f2c3e3d65b6296</anchor>
      <arglist>(L_Func *fn)</arglist>
    </member>
  </compound>
  <compound kind="file">
    <name>l_promotion.c</name>
    <path>/mnt/hgfs/mkt/Desktop/openimpact/src/Lcode/opti/Lopti/</path>
    <filename>l__promotion_8c</filename>
    <includes id="l__opti_8h" name="l_opti.h" local="yes" imported="no">l_opti.h</includes>
    <includes id="l__promotion_8h" name="l_promotion.h" local="yes" imported="no">l_promotion.h</includes>
    <member kind="define">
      <type>#define</type>
      <name>ERR</name>
      <anchorfile>l__promotion_8c.html</anchorfile>
      <anchor>735563036dced0b7d6cc98f97ea4978b</anchor>
      <arglist></arglist>
    </member>
    <member kind="function">
      <type>void</type>
      <name>L_set_pred_promotion_level</name>
      <anchorfile>l__promotion_8c.html</anchorfile>
      <anchor>2d61e7c4f4b94b7cde7dc0365b9be09e</anchor>
      <arglist>(int level)</arglist>
    </member>
    <member kind="function" static="yes">
      <type>static void</type>
      <name>L_promote_oper</name>
      <anchorfile>l__promotion_8c.html</anchorfile>
      <anchor>9a6a3fa3b4a2ad4bbb829cc000f44956</anchor>
      <arglist>(L_Cb *cb, L_Oper *oper, L_Operand *pred)</arglist>
    </member>
    <member kind="function" static="yes">
      <type>static L_Oper *</type>
      <name>L_uncond_def_or_null</name>
      <anchorfile>l__promotion_8c.html</anchorfile>
      <anchor>94ef1ba8a13c3e0ea6bfd18b020c04bc</anchor>
      <arglist>(L_Operand *pred)</arglist>
    </member>
    <member kind="function" static="yes">
      <type>static int</type>
      <name>L_def_reaches_all_subsequent_uses</name>
      <anchorfile>l__promotion_8c.html</anchorfile>
      <anchor>831e9ca139d57cd325f73847e378d195</anchor>
      <arglist>(L_Cb *cb, L_Oper *oper, L_Oper *poper, L_Operand *dest)</arglist>
    </member>
    <member kind="function">
      <type>int</type>
      <name>L_promotion_may_make_unsafe</name>
      <anchorfile>l__promotion_8c.html</anchorfile>
      <anchor>b644af8a5c731ed3ee40c5b68532f1c6</anchor>
      <arglist>(L_Func *fn, L_Operand *pred, L_Oper *op)</arglist>
    </member>
    <member kind="function">
      <type>int</type>
      <name>L_promotion_unprofitable</name>
      <anchorfile>l__promotion_8c.html</anchorfile>
      <anchor>8e6c8bb7a78daf94e126cafd24b22d18</anchor>
      <arglist>(L_Oper *pop, L_Oper *op)</arglist>
    </member>
    <member kind="function">
      <type>int</type>
      <name>L_oper_promotable</name>
      <anchorfile>l__promotion_8c.html</anchorfile>
      <anchor>f821973c2b473d3e1619c1f781cfc367</anchor>
      <arglist>(L_Oper *op)</arglist>
    </member>
    <member kind="function">
      <type>int</type>
      <name>L_oper_promotable_without_check</name>
      <anchorfile>l__promotion_8c.html</anchorfile>
      <anchor>7052dfd9924b53ef6244650e67302f86</anchor>
      <arglist>(L_Oper *oper)</arglist>
    </member>
    <member kind="function" static="yes">
      <type>static int</type>
      <name>L_safe_to_promote1</name>
      <anchorfile>l__promotion_8c.html</anchorfile>
      <anchor>6c2ba86ef2688f9a4264500b94cfa163</anchor>
      <arglist>(L_Cb *cb, L_Oper *op, L_Oper *pop)</arglist>
    </member>
    <member kind="function" static="yes">
      <type>static int</type>
      <name>L_safe_to_promote2</name>
      <anchorfile>l__promotion_8c.html</anchorfile>
      <anchor>0a59596a79877699b9643371dae708ca</anchor>
      <arglist>(L_Cb *cb, L_Oper *oper, L_Oper *poper)</arglist>
    </member>
    <member kind="function">
      <type>int</type>
      <name>L_safe_to_promote</name>
      <anchorfile>l__promotion_8c.html</anchorfile>
      <anchor>bee6fce071052963685de721db8cc7d4</anchor>
      <arglist>(L_Cb *cb, L_Oper *oper, L_Oper *poper)</arglist>
    </member>
    <member kind="function" static="yes">
      <type>static int</type>
      <name>L_all_dest_not_live_into_cb</name>
      <anchorfile>l__promotion_8c.html</anchorfile>
      <anchor>aa432b4563a2be88a0015b4d1e768bb3</anchor>
      <arglist>(L_Cb *cb, L_Oper *oper)</arglist>
    </member>
    <member kind="function" static="yes">
      <type>static int</type>
      <name>L_profitable_for_renaming</name>
      <anchorfile>l__promotion_8c.html</anchorfile>
      <anchor>b11ed35f8b4b82586eebe9fb3d323840</anchor>
      <arglist>(L_Oper *oper)</arglist>
    </member>
    <member kind="function">
      <type>L_Oper *</type>
      <name>L_find_pred_definition</name>
      <anchorfile>l__promotion_8c.html</anchorfile>
      <anchor>57536dea702d556c72b306105a8aab5b</anchor>
      <arglist>(L_Oper *oper)</arglist>
    </member>
    <member kind="function" static="yes">
      <type>static int</type>
      <name>L_kill_after_promotion</name>
      <anchorfile>l__promotion_8c.html</anchorfile>
      <anchor>c2fab44d4536c9be31248602891700be</anchor>
      <arglist>(L_Cb *cb, L_Oper *op)</arglist>
    </member>
    <member kind="function" static="yes">
      <type>static void</type>
      <name>L_predicate_promotion_cb</name>
      <anchorfile>l__promotion_8c.html</anchorfile>
      <anchor>2d192bac8de50cd6e768f53cb1e6bb14</anchor>
      <arglist>(L_Cb *cb, int level)</arglist>
    </member>
    <member kind="function">
      <type>void</type>
      <name>L_predicate_promotion</name>
      <anchorfile>l__promotion_8c.html</anchorfile>
      <anchor>598e87eadac9dd5e3c41ede1784efc97</anchor>
      <arglist>(L_Func *fn, int only_simple)</arglist>
    </member>
  </compound>
  <compound kind="file">
    <name>l_promotion.h</name>
    <path>/mnt/hgfs/mkt/Desktop/openimpact/src/Lcode/opti/Lopti/</path>
    <filename>l__promotion_8h</filename>
    <member kind="function">
      <type>void</type>
      <name>L_set_pred_promotion_level</name>
      <anchorfile>l__promotion_8h.html</anchorfile>
      <anchor>2d61e7c4f4b94b7cde7dc0365b9be09e</anchor>
      <arglist>(int level)</arglist>
    </member>
    <member kind="function">
      <type>int</type>
      <name>L_safe_to_promote</name>
      <anchorfile>l__promotion_8h.html</anchorfile>
      <anchor>0126b488cb50659632916ae499be1e6e</anchor>
      <arglist>(L_Cb *cb, L_Oper *oper, L_Oper *def_op)</arglist>
    </member>
    <member kind="function">
      <type>int</type>
      <name>L_oper_promotable_without_check</name>
      <anchorfile>l__promotion_8h.html</anchorfile>
      <anchor>7052dfd9924b53ef6244650e67302f86</anchor>
      <arglist>(L_Oper *oper)</arglist>
    </member>
    <member kind="function">
      <type>L_Oper *</type>
      <name>L_find_pred_definition</name>
      <anchorfile>l__promotion_8h.html</anchorfile>
      <anchor>57536dea702d556c72b306105a8aab5b</anchor>
      <arglist>(L_Oper *oper)</arglist>
    </member>
    <member kind="function">
      <type>int</type>
      <name>L_promotion_may_make_unsafe</name>
      <anchorfile>l__promotion_8h.html</anchorfile>
      <anchor>b644af8a5c731ed3ee40c5b68532f1c6</anchor>
      <arglist>(L_Func *fn, L_Operand *pred, L_Oper *op)</arglist>
    </member>
    <member kind="function">
      <type>void</type>
      <name>L_predicate_promotion</name>
      <anchorfile>l__promotion_8h.html</anchorfile>
      <anchor>598e87eadac9dd5e3c41ede1784efc97</anchor>
      <arglist>(L_Func *fn, int only_simple)</arglist>
    </member>
    <member kind="function">
      <type>int</type>
      <name>L_promotion_unprofitable</name>
      <anchorfile>l__promotion_8h.html</anchorfile>
      <anchor>8e6c8bb7a78daf94e126cafd24b22d18</anchor>
      <arglist>(L_Oper *pop, L_Oper *op)</arglist>
    </member>
  </compound>
  <compound kind="file">
    <name>l_unification.c</name>
    <path>/mnt/hgfs/mkt/Desktop/openimpact/src/Lcode/opti/Lopti/</path>
    <filename>l__unification_8c</filename>
    <class kind="struct">_UniPath</class>
    <member kind="typedef">
      <type>_UniPath</type>
      <name>UniPath</name>
      <anchorfile>l__unification_8c.html</anchorfile>
      <anchor>290938b43ab499344c4543464e4109de</anchor>
      <arglist></arglist>
    </member>
    <member kind="function" static="yes">
      <type>static UniPath *</type>
      <name>Luni_new_unipath</name>
      <anchorfile>l__unification_8c.html</anchorfile>
      <anchor>ac816d50b9291501aee3daedf2859640</anchor>
      <arglist>(L_Cb *cb, L_Oper *op)</arglist>
    </member>
    <member kind="function" static="yes">
      <type>static UniPath *</type>
      <name>Luni_free_unipath</name>
      <anchorfile>l__unification_8c.html</anchorfile>
      <anchor>c469d7391521b80e6fb359fc2f27151a</anchor>
      <arglist>(UniPath *up)</arglist>
    </member>
    <member kind="function" static="yes">
      <type>static void</type>
      <name>Luni_clean_unipath</name>
      <anchorfile>l__unification_8c.html</anchorfile>
      <anchor>8672e277e80eb21fca3e88404d9a9cf4</anchor>
      <arglist>(UniPath *up)</arglist>
    </member>
    <member kind="function" static="yes">
      <type>static List</type>
      <name>L_construct_merge_list</name>
      <anchorfile>l__unification_8c.html</anchorfile>
      <anchor>d3291e5b5dcd3ef75f70a462051342e4</anchor>
      <arglist>(L_Cb *cb)</arglist>
    </member>
    <member kind="function" static="yes">
      <type>static int</type>
      <name>L_two_way_split</name>
      <anchorfile>l__unification_8c.html</anchorfile>
      <anchor>7a7582c30a6145cf26688395aeca1693</anchor>
      <arglist>(L_Cb *cb, L_Oper *br, L_Cb **plt, L_Cb **prt, L_Oper **pfoplt)</arglist>
    </member>
    <member kind="function" static="yes">
      <type>static Set</type>
      <name>L_sync_dep_opers_acc</name>
      <anchorfile>l__unification_8c.html</anchorfile>
      <anchor>d092bf3dad3ee1e13c8dcc26e57a6bb4</anchor>
      <arglist>(Set sdset, L_Oper *op)</arglist>
    </member>
    <member kind="function" static="yes">
      <type>static Set</type>
      <name>L_defined_rmid_acc</name>
      <anchorfile>l__unification_8c.html</anchorfile>
      <anchor>14a9c15c55704bc98b3903b6e86297ba</anchor>
      <arglist>(Set rmset, L_Oper *op)</arglist>
    </member>
    <member kind="function" static="yes">
      <type>static Set</type>
      <name>L_consumed_rmid_acc</name>
      <anchorfile>l__unification_8c.html</anchorfile>
      <anchor>5674bcee1d8fb1b737bfaed9c7100bfe</anchor>
      <arglist>(Set rmset, L_Oper *op)</arglist>
    </member>
    <member kind="function">
      <type>void</type>
      <name>L_update_unipath</name>
      <anchorfile>l__unification_8c.html</anchorfile>
      <anchor>0cb49750ca7657b222b141ddff6bc026</anchor>
      <arglist>(UniPath *up, L_Oper *op)</arglist>
    </member>
    <member kind="function" static="yes">
      <type>static L_Oper *</type>
      <name>L_insert_compensation_mov_before</name>
      <anchorfile>l__unification_8c.html</anchorfile>
      <anchor>2cec95ab58a4a5a55e1ff0bb8e9b5aa2</anchor>
      <arglist>(L_Cb *cb, L_Oper *before, L_Operand *dest, L_Operand *src)</arglist>
    </member>
    <member kind="function" static="yes">
      <type>static int</type>
      <name>L_is_compensation_mov</name>
      <anchorfile>l__unification_8c.html</anchorfile>
      <anchor>023ad5f4691e0afe973797ad700342cd</anchor>
      <arglist>(HashTable hash, L_Oper *oper)</arglist>
    </member>
    <member kind="function" static="yes">
      <type>static int</type>
      <name>L_conflicting_src</name>
      <anchorfile>l__unification_8c.html</anchorfile>
      <anchor>b23dce90b13f9120643bffb7f35a4831</anchor>
      <arglist>(Set rmset, L_Oper *op)</arglist>
    </member>
    <member kind="function" static="yes">
      <type>static int</type>
      <name>L_conflicting_dest</name>
      <anchorfile>l__unification_8c.html</anchorfile>
      <anchor>9310e75a4982736178d0050aa73ad849</anchor>
      <arglist>(Set rmset, L_Oper *op)</arglist>
    </member>
    <member kind="function" static="yes">
      <type>static int</type>
      <name>L_rename_operand</name>
      <anchorfile>l__unification_8c.html</anchorfile>
      <anchor>83bd61aae2b893f89e38ae1df295facf</anchor>
      <arglist>(HashTable nh, L_Operand *opd)</arglist>
    </member>
    <member kind="function" static="yes">
      <type>static int</type>
      <name>Luni_up_candidate</name>
      <anchorfile>l__unification_8c.html</anchorfile>
      <anchor>54b5f815bb6e17439f4b898546c4ce94</anchor>
      <arglist>(UniPath *up, L_Oper *op)</arglist>
    </member>
    <member kind="function" static="yes">
      <type>static int</type>
      <name>Luni_legal_up_unification</name>
      <anchorfile>l__unification_8c.html</anchorfile>
      <anchor>389638fb862648394f3cab9aef439fe6</anchor>
      <arglist>(UniPath *lp, L_Oper *op_lt, UniPath *rp, L_Oper *op_rt, L_Oper *sbr)</arglist>
    </member>
    <member kind="function" static="yes">
      <type>static L_Oper *</type>
      <name>L_unify_up</name>
      <anchorfile>l__unification_8c.html</anchorfile>
      <anchor>2b246040e8d884992b457e6d20cb541b</anchor>
      <arglist>(L_Cb *cb, L_Oper *before, UniPath *upA, L_Oper *opA, UniPath *upB, L_Oper *opB)</arglist>
    </member>
    <member kind="function" static="yes">
      <type>static int</type>
      <name>Luni_dn_candidate</name>
      <anchorfile>l__unification_8c.html</anchorfile>
      <anchor>9eddec4d275665d293ce04cc9f0e84f1</anchor>
      <arglist>(UniPath *up, L_Oper *op)</arglist>
    </member>
    <member kind="function" static="yes">
      <type>static int</type>
      <name>Luni_legal_dn_unification</name>
      <anchorfile>l__unification_8c.html</anchorfile>
      <anchor>e2ca2038371631a181e4fcf713f062e7</anchor>
      <arglist>(UniPath *upA, L_Oper *opA, UniPath *upB, L_Oper *opB)</arglist>
    </member>
    <member kind="function" static="yes">
      <type>static L_Oper *</type>
      <name>L_unify_down</name>
      <anchorfile>l__unification_8c.html</anchorfile>
      <anchor>49ed0361e5e54920b2f4f5d55d734303</anchor>
      <arglist>(L_Cb *mrg_cb, List uplist)</arglist>
    </member>
    <member kind="function" static="yes">
      <type>static void</type>
      <name>L_activate_remap</name>
      <anchorfile>l__unification_8c.html</anchorfile>
      <anchor>8eeccd5212fe3094b5856cad40d746cf</anchor>
      <arglist>(UniPath *up, L_Oper *mov)</arglist>
    </member>
    <member kind="function">
      <type>int</type>
      <name>L_unification</name>
      <anchorfile>l__unification_8c.html</anchorfile>
      <anchor>57f9164fbcbba7c68dd51781f13c53ec</anchor>
      <arglist>(L_Func *fn)</arglist>
    </member>
    <member kind="variable" static="yes">
      <type>static L_Alloc_Pool *</type>
      <name>UniPath_pool</name>
      <anchorfile>l__unification_8c.html</anchorfile>
      <anchor>b2b7e16c4608624d86c10ba63452105a</anchor>
      <arglist></arglist>
    </member>
  </compound>
  <compound kind="struct">
    <name>_invariant_address_access</name>
    <filename>struct__invariant__address__access.html</filename>
    <member kind="variable">
      <type>List</type>
      <name>store_mem_op</name>
      <anchorfile>struct__invariant__address__access.html</anchorfile>
      <anchor>f190e2f7ac03db9ce429f132bac3a318</anchor>
      <arglist></arglist>
    </member>
    <member kind="variable">
      <type>List</type>
      <name>load_mem_op</name>
      <anchorfile>struct__invariant__address__access.html</anchorfile>
      <anchor>619cafb3229d771bf81f923f240797d0</anchor>
      <arglist></arglist>
    </member>
    <member kind="variable">
      <type>List</type>
      <name>alias_store_mem_op</name>
      <anchorfile>struct__invariant__address__access.html</anchorfile>
      <anchor>1372925080aa083fdaa17ea40cd029d2</anchor>
      <arglist></arglist>
    </member>
    <member kind="variable">
      <type>List</type>
      <name>alias_load_mem_op</name>
      <anchorfile>struct__invariant__address__access.html</anchorfile>
      <anchor>95d00ea9a65ec588228c9c065b502df5</anchor>
      <arglist></arglist>
    </member>
    <member kind="variable">
      <type>List</type>
      <name>alias_store_jsr</name>
      <anchorfile>struct__invariant__address__access.html</anchorfile>
      <anchor>122c5159ff7863002b3a783e96b36014</anchor>
      <arglist></arglist>
    </member>
    <member kind="variable">
      <type>List</type>
      <name>alias_load_jsr</name>
      <anchorfile>struct__invariant__address__access.html</anchorfile>
      <anchor>dd7c47a672b2c4559998412db0a8a7d4</anchor>
      <arglist></arglist>
    </member>
    <member kind="variable">
      <type>int</type>
      <name>safe</name>
      <anchorfile>struct__invariant__address__access.html</anchorfile>
      <anchor>323b8914f032ba30d6b533b949f5203d</anchor>
      <arglist></arglist>
    </member>
    <member kind="variable">
      <type>int</type>
      <name>is_block_move</name>
      <anchorfile>struct__invariant__address__access.html</anchorfile>
      <anchor>55838ff27c6a739a6a437cfa52e0bbcc</anchor>
      <arglist></arglist>
    </member>
    <member kind="variable">
      <type>double</type>
      <name>weight</name>
      <anchorfile>struct__invariant__address__access.html</anchorfile>
      <anchor>71d6dc3f9210deb6eafd0f38ce7ccc12</anchor>
      <arglist></arglist>
    </member>
    <member kind="variable">
      <type>double</type>
      <name>benefit</name>
      <anchorfile>struct__invariant__address__access.html</anchorfile>
      <anchor>21af82d9b74b1159312243f3d4f706ce</anchor>
      <arglist></arglist>
    </member>
  </compound>
  <compound kind="struct">
    <name>_L_Jrg_Element</name>
    <filename>struct__L__Jrg__Element.html</filename>
    <member kind="variable">
      <type>double</type>
      <name>weight</name>
      <anchorfile>struct__L__Jrg__Element.html</anchorfile>
      <anchor>c090f988ed41bce6eba0d73a1b66738a</anchor>
      <arglist></arglist>
    </member>
    <member kind="variable">
      <type>int</type>
      <name>target</name>
      <anchorfile>struct__L__Jrg__Element.html</anchorfile>
      <anchor>035b811a1bd00ae7f364cecccd371c47</anchor>
      <arglist></arglist>
    </member>
    <member kind="variable">
      <type>int</type>
      <name>cc</name>
      <anchorfile>struct__L__Jrg__Element.html</anchorfile>
      <anchor>04de58fd03c12d7c5a866f421efc25ba</anchor>
      <arglist></arglist>
    </member>
    <member kind="variable">
      <type>int</type>
      <name>id</name>
      <anchorfile>struct__L__Jrg__Element.html</anchorfile>
      <anchor>1ca73312e0350031c9a6b5d99ea75fbf</anchor>
      <arglist></arglist>
    </member>
  </compound>
  <compound kind="struct">
    <name>_LiveRange</name>
    <filename>struct__LiveRange.html</filename>
    <member kind="variable">
      <type>short</type>
      <name>id</name>
      <anchorfile>struct__LiveRange.html</anchorfile>
      <anchor>1c774e7211270baaa42c477c58890525</anchor>
      <arglist></arglist>
    </member>
    <member kind="variable">
      <type>short</type>
      <name>vreg</name>
      <anchorfile>struct__LiveRange.html</anchorfile>
      <anchor>deffe1ffb108eaa5b142bb5337c70657</anchor>
      <arglist></arglist>
    </member>
    <member kind="variable">
      <type>short</type>
      <name>valid</name>
      <anchorfile>struct__LiveRange.html</anchorfile>
      <anchor>80db38b1ac80a11c6fe65c2bdc17775b</anchor>
      <arglist></arglist>
    </member>
    <member kind="variable">
      <type>short</type>
      <name>ctype</name>
      <anchorfile>struct__LiveRange.html</anchorfile>
      <anchor>7cb3477c39077fc409ab37ee6ce42147</anchor>
      <arglist></arglist>
    </member>
    <member kind="variable">
      <type>L_Oper *</type>
      <name>def_oper</name>
      <anchorfile>struct__LiveRange.html</anchorfile>
      <anchor>db76b55110467eff45f920f697969c81</anchor>
      <arglist></arglist>
    </member>
    <member kind="variable">
      <type>Set</type>
      <name>op</name>
      <anchorfile>struct__LiveRange.html</anchorfile>
      <anchor>feab7d1d99bc0b42463db2b7b44ba5bf</anchor>
      <arglist></arglist>
    </member>
    <member kind="variable">
      <type>Set</type>
      <name>def_op</name>
      <anchorfile>struct__LiveRange.html</anchorfile>
      <anchor>568ccfd7b8c87b07b0ec0ffdd785567b</anchor>
      <arglist></arglist>
    </member>
    <member kind="variable">
      <type>Set</type>
      <name>ref_op</name>
      <anchorfile>struct__LiveRange.html</anchorfile>
      <anchor>054873b498026a0ea42d0c5f3350e915</anchor>
      <arglist></arglist>
    </member>
    <member kind="variable">
      <type>Set</type>
      <name>intf</name>
      <anchorfile>struct__LiveRange.html</anchorfile>
      <anchor>029eab076b887ca9e66ea4ea26457028</anchor>
      <arglist></arglist>
    </member>
    <member kind="variable">
      <type>Set</type>
      <name>composition</name>
      <anchorfile>struct__LiveRange.html</anchorfile>
      <anchor>e65f38a5881512c0dff35db4c12ae933</anchor>
      <arglist></arglist>
    </member>
    <member kind="variable">
      <type>int</type>
      <name>flags</name>
      <anchorfile>struct__LiveRange.html</anchorfile>
      <anchor>50fcba914424dfa9d667dc55b1d63008</anchor>
      <arglist></arglist>
    </member>
  </compound>
  <compound kind="struct">
    <name>_operand_node</name>
    <filename>struct__operand__node.html</filename>
    <member kind="variable">
      <type>L_Operand *</type>
      <name>operand</name>
      <anchorfile>struct__operand__node.html</anchorfile>
      <anchor>f6df3f5a82aa1c5f427dd705f59ae7e5</anchor>
      <arglist></arglist>
    </member>
    <member kind="variable">
      <type>int</type>
      <name>DFS_search_flag</name>
      <anchorfile>struct__operand__node.html</anchorfile>
      <anchor>bbaff901cc46da779845a07ff54be6f7</anchor>
      <arglist></arglist>
    </member>
    <member kind="variable">
      <type>_operand_pair *</type>
      <name>def</name>
      <anchorfile>struct__operand__node.html</anchorfile>
      <anchor>0d38d7265cdfe7aa8af5fb9a571f831d</anchor>
      <arglist></arglist>
    </member>
    <member kind="variable">
      <type>int</type>
      <name>poison</name>
      <anchorfile>struct__operand__node.html</anchorfile>
      <anchor>e6d287365c187c796cddbd4f282b8e8b</anchor>
      <arglist></arglist>
    </member>
  </compound>
  <compound kind="struct">
    <name>_operand_pair</name>
    <filename>struct__operand__pair.html</filename>
    <member kind="variable">
      <type>int</type>
      <name>opc</name>
      <anchorfile>struct__operand__pair.html</anchorfile>
      <anchor>f149802ed8fc78ecf11a563d270d282b</anchor>
      <arglist></arglist>
    </member>
    <member kind="variable">
      <type>ITuint8</type>
      <name>com</name>
      <anchorfile>struct__operand__pair.html</anchorfile>
      <anchor>55a4e8bf10b2a13f86138859164ad6b5</anchor>
      <arglist>[L_MAX_CMPLTR]</arglist>
    </member>
    <member kind="variable">
      <type>_operand_node *</type>
      <name>operand1</name>
      <anchorfile>struct__operand__pair.html</anchorfile>
      <anchor>1912b83ccfe90a3b97a88c0924737496</anchor>
      <arglist></arglist>
    </member>
    <member kind="variable">
      <type>_operand_node *</type>
      <name>operand2</name>
      <anchorfile>struct__operand__pair.html</anchorfile>
      <anchor>acc49dce368e43b4dfefc1745681913b</anchor>
      <arglist></arglist>
    </member>
    <member kind="variable">
      <type>_operand_pair *</type>
      <name>next</name>
      <anchorfile>struct__operand__pair.html</anchorfile>
      <anchor>73a6c778863b65c9970691a8c777f700</anchor>
      <arglist></arglist>
    </member>
  </compound>
  <compound kind="struct">
    <name>_UniPath</name>
    <filename>struct__UniPath.html</filename>
    <member kind="variable">
      <type>HashTable</type>
      <name>map</name>
      <anchorfile>struct__UniPath.html</anchorfile>
      <anchor>2f8429d715f8c598f7dc8191c17f9239</anchor>
      <arglist></arglist>
    </member>
    <member kind="variable">
      <type>L_Cb *</type>
      <name>cb</name>
      <anchorfile>struct__UniPath.html</anchorfile>
      <anchor>2df3d05ea93aaf473555f70983e21525</anchor>
      <arglist></arglist>
    </member>
    <member kind="variable">
      <type>L_Oper *</type>
      <name>op</name>
      <anchorfile>struct__UniPath.html</anchorfile>
      <anchor>220a28bd0dbdee755abe826db607f59a</anchor>
      <arglist></arglist>
    </member>
    <member kind="variable">
      <type>L_Oper *</type>
      <name>mop</name>
      <anchorfile>struct__UniPath.html</anchorfile>
      <anchor>438853a0879db3d587600640d3f39f72</anchor>
      <arglist></arglist>
    </member>
    <member kind="variable">
      <type>Set</type>
      <name>def</name>
      <anchorfile>struct__UniPath.html</anchorfile>
      <anchor>3fe26a441e81619020eecf867340dfe3</anchor>
      <arglist></arglist>
    </member>
    <member kind="variable">
      <type>Set</type>
      <name>use</name>
      <anchorfile>struct__UniPath.html</anchorfile>
      <anchor>5c8dcaad8d8a38bac8e2a4317d82c1cd</anchor>
      <arglist></arglist>
    </member>
    <member kind="variable">
      <type>Set</type>
      <name>sync</name>
      <anchorfile>struct__UniPath.html</anchorfile>
      <anchor>3e11ac17a28d36a81e2a20cefa8e6a2a</anchor>
      <arglist></arglist>
    </member>
    <member kind="variable">
      <type>Set</type>
      <name>remap</name>
      <anchorfile>struct__UniPath.html</anchorfile>
      <anchor>39023fb5ff4d5d2804f733b0f93e4dad</anchor>
      <arglist></arglist>
    </member>
  </compound>
  <compound kind="union">
    <name>convert_float_union</name>
    <filename>unionconvert__float__union.html</filename>
    <member kind="variable">
      <type>float</type>
      <name>fval</name>
      <anchorfile>unionconvert__float__union.html</anchorfile>
      <anchor>115346d4d34ed77c6900faf1d546e8af</anchor>
      <arglist></arglist>
    </member>
    <member kind="variable">
      <type>int</type>
      <name>ival</name>
      <anchorfile>unionconvert__float__union.html</anchorfile>
      <anchor>41881a06f7d402ea2a1b85ad8e8cf940</anchor>
      <arglist></arglist>
    </member>
  </compound>
  <compound kind="union">
    <name>convert_union</name>
    <filename>unionconvert__union.html</filename>
    <member kind="variable">
      <type>double</type>
      <name>dval</name>
      <anchorfile>unionconvert__union.html</anchorfile>
      <anchor>12af0a81f6b87d65bd0adc82dc6197c7</anchor>
      <arglist></arglist>
    </member>
    <member kind="variable">
      <type>convert_union::@0</type>
      <name>hval</name>
      <anchorfile>unionconvert__union.html</anchorfile>
      <anchor>e9877bd88bdd606ee58325f7898b3c7f</anchor>
      <arglist></arglist>
    </member>
    <member kind="variable">
      <type>int</type>
      <name>h1</name>
      <anchorfile>structconvert__union_1_1@0.html</anchorfile>
      <anchor>bb528fae96426e2b119c59fae3e5dd16</anchor>
      <arglist></arglist>
    </member>
    <member kind="variable">
      <type>int</type>
      <name>h2</name>
      <anchorfile>structconvert__union_1_1@0.html</anchorfile>
      <anchor>1627d5228a19b5daa259bb1f0e61233c</anchor>
      <arglist></arglist>
    </member>
  </compound>
  <compound kind="struct">
    <name>L_Cb_Map</name>
    <filename>structL__Cb__Map.html</filename>
    <member kind="variable">
      <type>L_Cb **</type>
      <name>src</name>
      <anchorfile>structL__Cb__Map.html</anchorfile>
      <anchor>059dd0e067f40e26dfd1de5f1efc5e41</anchor>
      <arglist></arglist>
    </member>
    <member kind="variable">
      <type>L_Cb **</type>
      <name>copy</name>
      <anchorfile>structL__Cb__Map.html</anchorfile>
      <anchor>852d77e6349a2d49a4dd569d54932726</anchor>
      <arglist></arglist>
    </member>
    <member kind="variable">
      <type>int</type>
      <name>num_entries</name>
      <anchorfile>structL__Cb__Map.html</anchorfile>
      <anchor>001e18c81728a1a6cd13542ed6be7060</anchor>
      <arglist></arglist>
    </member>
  </compound>
  <compound kind="struct">
    <name>L_Danger_Ext</name>
    <filename>structL__Danger__Ext.html</filename>
    <member kind="variable">
      <type>int</type>
      <name>identifier</name>
      <anchorfile>structL__Danger__Ext.html</anchorfile>
      <anchor>d708d5477a00da12b85399db440b1a86</anchor>
      <arglist></arglist>
    </member>
    <member kind="variable">
      <type>Set</type>
      <name>sub_call_between</name>
      <anchorfile>structL__Danger__Ext.html</anchorfile>
      <anchor>ec420b0c8c0c810463333134c249f65c</anchor>
      <arglist></arglist>
    </member>
    <member kind="variable">
      <type>Set</type>
      <name>general_sub_call_between</name>
      <anchorfile>structL__Danger__Ext.html</anchorfile>
      <anchor>377df11987a399194d5879ad83dc53ad</anchor>
      <arglist></arglist>
    </member>
    <member kind="variable">
      <type>Set</type>
      <name>sync_between</name>
      <anchorfile>structL__Danger__Ext.html</anchorfile>
      <anchor>19a99bba16bc008c26d12d766048194a</anchor>
      <arglist></arglist>
    </member>
    <member kind="variable">
      <type>Set</type>
      <name>store_between</name>
      <anchorfile>structL__Danger__Ext.html</anchorfile>
      <anchor>4be82c07e1536613497fe809ab279531</anchor>
      <arglist></arglist>
    </member>
  </compound>
  <compound kind="struct">
    <name>L_Ind_Var_Branch_Info</name>
    <filename>structL__Ind__Var__Branch__Info.html</filename>
    <member kind="variable">
      <type>L_Oper *</type>
      <name>branch</name>
      <anchorfile>structL__Ind__Var__Branch__Info.html</anchorfile>
      <anchor>c653ea02c931857a3a9fa3ba9e4c58d0</anchor>
      <arglist></arglist>
    </member>
    <member kind="variable">
      <type>int</type>
      <name>branch_dir</name>
      <anchorfile>structL__Ind__Var__Branch__Info.html</anchorfile>
      <anchor>525571b97778782e2c10fd2992bf68b2</anchor>
      <arglist></arglist>
    </member>
    <member kind="variable">
      <type>int</type>
      <name>ind_var_index</name>
      <anchorfile>structL__Ind__Var__Branch__Info.html</anchorfile>
      <anchor>3c7f1770b5855b838f32bee7d56f9eba</anchor>
      <arglist></arglist>
    </member>
    <member kind="variable">
      <type>int</type>
      <name>needs_remainder</name>
      <anchorfile>structL__Ind__Var__Branch__Info.html</anchorfile>
      <anchor>13051805a803e56fa7064918a458a837</anchor>
      <arglist></arglist>
    </member>
    <member kind="variable">
      <type>int</type>
      <name>skip_branch</name>
      <anchorfile>structL__Ind__Var__Branch__Info.html</anchorfile>
      <anchor>0d49f965620153691c56f0e44aaa03b8</anchor>
      <arglist></arglist>
    </member>
  </compound>
  <compound kind="struct">
    <name>L_Ind_Var_Info</name>
    <filename>structL__Ind__Var__Info.html</filename>
    <member kind="variable">
      <type>int *</type>
      <name>var</name>
      <anchorfile>structL__Ind__Var__Info.html</anchorfile>
      <anchor>db69ad894116213e7986678b6cf5d97e</anchor>
      <arglist></arglist>
    </member>
    <member kind="variable">
      <type>int *</type>
      <name>inc</name>
      <anchorfile>structL__Ind__Var__Info.html</anchorfile>
      <anchor>4d11dc4929622b8e6faef98c5f9d2448</anchor>
      <arglist></arglist>
    </member>
    <member kind="variable">
      <type>int *</type>
      <name>op</name>
      <anchorfile>structL__Ind__Var__Info.html</anchorfile>
      <anchor>247a999459423127e051b96c77cf4b01</anchor>
      <arglist></arglist>
    </member>
    <member kind="variable">
      <type>L_Operand **</type>
      <name>preheader_dest</name>
      <anchorfile>structL__Ind__Var__Info.html</anchorfile>
      <anchor>793e643ded28156e8b4205752e4faf2c</anchor>
      <arglist></arglist>
    </member>
  </compound>
  <compound kind="struct">
    <name>L_Reg_Count</name>
    <filename>structL__Reg__Count.html</filename>
    <member kind="variable">
      <type>int</type>
      <name>int_count</name>
      <anchorfile>structL__Reg__Count.html</anchorfile>
      <anchor>1ada09081e8380b1616acbad4cdc656e</anchor>
      <arglist></arglist>
    </member>
    <member kind="variable">
      <type>int</type>
      <name>flt_count</name>
      <anchorfile>structL__Reg__Count.html</anchorfile>
      <anchor>3a7f8e68929f99edbf605420d0592a7e</anchor>
      <arglist></arglist>
    </member>
    <member kind="variable">
      <type>int</type>
      <name>dbl_count</name>
      <anchorfile>structL__Reg__Count.html</anchorfile>
      <anchor>15c430d68001b4e201031ba4d358501f</anchor>
      <arglist></arglist>
    </member>
    <member kind="variable">
      <type>int</type>
      <name>prd_count</name>
      <anchorfile>structL__Reg__Count.html</anchorfile>
      <anchor>b5fefc6f5bed9a89a5a93a2ae8c5fdb4</anchor>
      <arglist></arglist>
    </member>
  </compound>
  <compound kind="struct">
    <name>L_Short_Info</name>
    <filename>structL__Short__Info.html</filename>
    <member kind="variable">
      <type>L_Operand *</type>
      <name>alias_operand</name>
      <anchorfile>structL__Short__Info.html</anchorfile>
      <anchor>257b89914628f205ab51d85f50e8e0ce</anchor>
      <arglist>[MAX_SHORT_OPERANDS]</arglist>
    </member>
    <member kind="variable">
      <type>L_Operand *</type>
      <name>orig_operand</name>
      <anchorfile>structL__Short__Info.html</anchorfile>
      <anchor>a94c67f809c6757fcf6e7cb868d3317a</anchor>
      <arglist>[MAX_SHORT_OPERANDS]</arglist>
    </member>
    <member kind="variable">
      <type>L_Oper *</type>
      <name>oper</name>
      <anchorfile>structL__Short__Info.html</anchorfile>
      <anchor>f8ca1326a2fc751df6e64a8d7023f2b6</anchor>
      <arglist>[MAX_SHORT_OPERANDS]</arglist>
    </member>
    <member kind="variable">
      <type>int</type>
      <name>num_entries</name>
      <anchorfile>structL__Short__Info.html</anchorfile>
      <anchor>bde1d6aa499f80985f621dcef7f51908</anchor>
      <arglist></arglist>
    </member>
  </compound>
</tagfile>
