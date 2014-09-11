<?xml version='1.0' encoding='ISO-8859-1' standalone='yes' ?>
<tagfile>
  <compound kind="file">
    <name>mia_compare.c</name>
    <path>/mnt/hgfs/mkt/Desktop/openimpact/src/Lcode/codegen/Mopti-ia64/</path>
    <filename>mia__compare_8c</filename>
    <includes id="mia__internal_8h" name="mia_internal.h" local="yes" imported="no">mia_internal.h</includes>
    <member kind="define">
      <type>#define</type>
      <name>VERBOSE</name>
      <anchorfile>mia__compare_8c.html</anchorfile>
      <anchor>42f8c497a1968074f38bf5055c650dca</anchor>
      <arglist></arglist>
    </member>
    <member kind="function" static="yes">
      <type>static L_Oper *</type>
      <name>find_add_for_compare</name>
      <anchorfile>mia__compare_8c.html</anchorfile>
      <anchor>f5cee414849cdaf3c81c578524b18c87</anchor>
      <arglist>(L_Oper *compare, int operand_num)</arglist>
    </member>
    <member kind="function" static="yes">
      <type>static int</type>
      <name>Switch_add_compare_order_if_needed</name>
      <anchorfile>mia__compare_8c.html</anchorfile>
      <anchor>fb93bfeee41849274565e37a09c2169a</anchor>
      <arglist>(L_Cb *cb, L_Oper *add, L_Oper *compare, int compare_operand_num)</arglist>
    </member>
    <member kind="function" static="yes">
      <type>static void</type>
      <name>Change_lte_to_lt</name>
      <anchorfile>mia__compare_8c.html</anchorfile>
      <anchor>97d71c12f70caacaadd256754007ee8a</anchor>
      <arglist>(L_Oper *compare)</arglist>
    </member>
    <member kind="function" static="yes">
      <type>static void</type>
      <name>Change_gte_to_gt</name>
      <anchorfile>mia__compare_8c.html</anchorfile>
      <anchor>213151dda23d2f8b5032cf5ccfa86aa3</anchor>
      <arglist>(L_Oper *compare)</arglist>
    </member>
    <member kind="function">
      <type>int</type>
      <name>Mopti_loop_compare_height_reduction</name>
      <anchorfile>mia__compare_8c.html</anchorfile>
      <anchor>3a3e726604ae78872e340ee4c5a252f2</anchor>
      <arglist>(L_Func *fn)</arglist>
    </member>
    <member kind="function">
      <type>int</type>
      <name>Mopti_compare_height_reduction</name>
      <anchorfile>mia__compare_8c.html</anchorfile>
      <anchor>df58ac098e0582c80135d341848632ae</anchor>
      <arglist>(L_Func *fn, L_Cb *cb, L_Loop *loop, int loop_n_cb, int *loop_cb)</arglist>
    </member>
  </compound>
  <compound kind="file">
    <name>mia_epi_merge.c</name>
    <path>/mnt/hgfs/mkt/Desktop/openimpact/src/Lcode/codegen/Mopti-ia64/</path>
    <filename>mia__epi__merge_8c</filename>
    <includes id="mia__internal_8h" name="mia_internal.h" local="yes" imported="no">mia_internal.h</includes>
    <member kind="define">
      <type>#define</type>
      <name>MERGE_PREDICATED_RET_BR</name>
      <anchorfile>mia__epi__merge_8c.html</anchorfile>
      <anchor>d26cdef0d2a5151acf14675af2711551</anchor>
      <arglist></arglist>
    </member>
    <member kind="define">
      <type>#define</type>
      <name>EPILOGUE_MERGE_THRESHOLD</name>
      <anchorfile>mia__epi__merge_8c.html</anchorfile>
      <anchor>d26a5a3e43e6c68684df7cb957bf1946</anchor>
      <arglist></arglist>
    </member>
    <member kind="define">
      <type>#define</type>
      <name>EPILOGUE_MERGE_MIN_OPERS</name>
      <anchorfile>mia__epi__merge_8c.html</anchorfile>
      <anchor>0ab090753065d494d494ed2af5e6bc79</anchor>
      <arglist></arglist>
    </member>
    <member kind="function">
      <type>int</type>
      <name>O_is_caller_save_predicate</name>
      <anchorfile>mia__epi__merge_8c.html</anchorfile>
      <anchor>e17026d5ab11147e0feec3b9488ccd79</anchor>
      <arglist>(L_Operand *pred)</arglist>
    </member>
    <member kind="function" static="yes">
      <type>static L_Cb *</type>
      <name>Find_epilogue_cb</name>
      <anchorfile>mia__epi__merge_8c.html</anchorfile>
      <anchor>d01b9d16cd567b06a9cdaf9c111d3ec8</anchor>
      <arglist>(L_Func *fn, int *size_of_cb)</arglist>
    </member>
    <member kind="function" static="yes">
      <type>static int</type>
      <name>Count_opers_in_merge_space</name>
      <anchorfile>mia__epi__merge_8c.html</anchorfile>
      <anchor>e51b4f844e8db816853f396e53840f12</anchor>
      <arglist>(L_Cb *cb, L_Oper *branch_oper)</arglist>
    </member>
    <member kind="function" static="yes">
      <type>static void</type>
      <name>Rev_copy_prop_return_value</name>
      <anchorfile>mia__epi__merge_8c.html</anchorfile>
      <anchor>3cc57e3095b9a8b762547b4573634dcf</anchor>
      <arglist>(L_Func *fn, Set modified_cbs)</arglist>
    </member>
    <member kind="function">
      <type>void</type>
      <name>Mopti_epilogue_merge</name>
      <anchorfile>mia__epi__merge_8c.html</anchorfile>
      <anchor>9bd4d0715f1c8006f178d8d2e1664cbc</anchor>
      <arglist>(L_Func *fn)</arglist>
    </member>
    <member kind="function">
      <type>void</type>
      <name>Mopti_epilogue_cleanup</name>
      <anchorfile>mia__epi__merge_8c.html</anchorfile>
      <anchor>b609816a640af52484e7da61b0140c69</anchor>
      <arglist>(L_Func *fn)</arglist>
    </member>
  </compound>
  <compound kind="file">
    <name>mia_internal.h</name>
    <path>/mnt/hgfs/mkt/Desktop/openimpact/src/Lcode/codegen/Mopti-ia64/</path>
    <filename>mia__internal_8h</filename>
    <includes id="mia__opti_8h" name="mia_opti.h" local="yes" imported="no">mia_opti.h</includes>
    <member kind="define">
      <type>#define</type>
      <name>MOD</name>
      <anchorfile>mia__internal_8h.html</anchorfile>
      <anchor>ca7d5718ab8c38506adb3bef2469b319</anchor>
      <arglist></arglist>
    </member>
    <member kind="define">
      <type>#define</type>
      <name>Ltahoe_new_int_reg</name>
      <anchorfile>mia__internal_8h.html</anchorfile>
      <anchor>4004d4835203f902cdd9d44e602d1ab5</anchor>
      <arglist>()</arglist>
    </member>
    <member kind="define">
      <type>#define</type>
      <name>Ltahoe_new_pred_reg</name>
      <anchorfile>mia__internal_8h.html</anchorfile>
      <anchor>925a3fd28f4c3b75cce791501937832a</anchor>
      <arglist>(ptype)</arglist>
    </member>
    <member kind="define">
      <type>#define</type>
      <name>Ltahoe_copy_or_new</name>
      <anchorfile>mia__internal_8h.html</anchorfile>
      <anchor>192b1bcba59c279acaadfb18bf57084d</anchor>
      <arglist>(cop, opd)</arglist>
    </member>
    <member kind="define">
      <type>#define</type>
      <name>Ltahoe_true_pred</name>
      <anchorfile>mia__internal_8h.html</anchorfile>
      <anchor>50998bf6558e6fe82f3084b82a3d82b0</anchor>
      <arglist>(ptype)</arglist>
    </member>
    <member kind="define">
      <type>#define</type>
      <name>Ltahoe_IMAC</name>
      <anchorfile>mia__internal_8h.html</anchorfile>
      <anchor>fd15ef5c94490f5d515ac342a45c28bd</anchor>
      <arglist>(mac)</arglist>
    </member>
    <member kind="define">
      <type>#define</type>
      <name>Ltahoe_PMAC</name>
      <anchorfile>mia__internal_8h.html</anchorfile>
      <anchor>c47e09bd5b8e0aa94abd17595678904a</anchor>
      <arglist>(mac)</arglist>
    </member>
    <member kind="define">
      <type>#define</type>
      <name>Ltahoe_FMAC</name>
      <anchorfile>mia__internal_8h.html</anchorfile>
      <anchor>af0ee82d3779c54db8523070f6ec45e1</anchor>
      <arglist>(mac)</arglist>
    </member>
    <member kind="define">
      <type>#define</type>
      <name>Ltahoe_DMAC</name>
      <anchorfile>mia__internal_8h.html</anchorfile>
      <anchor>3085b3491911228d07b9e9373763cfb0</anchor>
      <arglist>(mac)</arglist>
    </member>
    <member kind="define">
      <type>#define</type>
      <name>Ltahoe_BMAC</name>
      <anchorfile>mia__internal_8h.html</anchorfile>
      <anchor>47cba0409cc34950a5b2dfea55534ba9</anchor>
      <arglist>(mac)</arglist>
    </member>
    <member kind="function">
      <type>void</type>
      <name>Mopti_debug</name>
      <anchorfile>mia__internal_8h.html</anchorfile>
      <anchor>ca52ba4b3401fe69d84481a0a4e0cb94</anchor>
      <arglist>(char *fmt,...)</arglist>
    </member>
    <member kind="function">
      <type>int</type>
      <name>Mopti_loop_compare_height_reduction</name>
      <anchorfile>mia__internal_8h.html</anchorfile>
      <anchor>3a3e726604ae78872e340ee4c5a252f2</anchor>
      <arglist>(L_Func *fn)</arglist>
    </member>
    <member kind="function">
      <type>int</type>
      <name>Mopti_compare_height_reduction</name>
      <anchorfile>mia__internal_8h.html</anchorfile>
      <anchor>df58ac098e0582c80135d341848632ae</anchor>
      <arglist>(L_Func *fn, L_Cb *cb, L_Loop *loop, int loop_n_cb, int *loop_cb)</arglist>
    </member>
  </compound>
  <compound kind="file">
    <name>mia_jump_opti.c</name>
    <path>/mnt/hgfs/mkt/Desktop/openimpact/src/Lcode/codegen/Mopti-ia64/</path>
    <filename>mia__jump__opti_8c</filename>
    <includes id="mia__internal_8h" name="mia_internal.h" local="yes" imported="no">mia_internal.h</includes>
    <member kind="define">
      <type>#define</type>
      <name>DEBUG_BR_TGT_EXPN</name>
      <anchorfile>mia__jump__opti_8c.html</anchorfile>
      <anchor>c9255186c3fa5c0fbde7edd6712e1cd9</anchor>
      <arglist></arglist>
    </member>
    <member kind="function">
      <type>int</type>
      <name>Mopti_branch_target_expansion</name>
      <anchorfile>mia__jump__opti_8c.html</anchorfile>
      <anchor>ecc2271fa0113f92de8fd859a24e2e26</anchor>
      <arglist>(L_Func *fn)</arglist>
    </member>
  </compound>
  <compound kind="file">
    <name>mia_lp_prel.c</name>
    <path>/mnt/hgfs/mkt/Desktop/openimpact/src/Lcode/codegen/Mopti-ia64/</path>
    <filename>mia__lp__prel_8c</filename>
    <includes id="mia__internal_8h" name="mia_internal.h" local="yes" imported="no">mia_internal.h</includes>
    <member kind="define">
      <type>#define</type>
      <name>ERR</name>
      <anchorfile>mia__lp__prel_8c.html</anchorfile>
      <anchor>735563036dced0b7d6cc98f97ea4978b</anchor>
      <arglist></arglist>
    </member>
    <member kind="define">
      <type>#define</type>
      <name>MAX_LOOP_NESTING</name>
      <anchorfile>mia__lp__prel_8c.html</anchorfile>
      <anchor>884f45876d5ae7e312160cc16042bb06</anchor>
      <arglist></arglist>
    </member>
    <member kind="define">
      <type>#define</type>
      <name>MIN_LOOP_PRELOAD_WEIGHT</name>
      <anchorfile>mia__lp__prel_8c.html</anchorfile>
      <anchor>06c182314aa99477092b5ad98f2e4981</anchor>
      <arglist></arglist>
    </member>
    <member kind="define">
      <type>#define</type>
      <name>MIN_LOOP_ITER_FOR_PRELOAD</name>
      <anchorfile>mia__lp__prel_8c.html</anchorfile>
      <anchor>afa18da6c82702cd6b6b70cbe1fb2150</anchor>
      <arglist></arglist>
    </member>
    <member kind="define">
      <type>#define</type>
      <name>MAX_PRELOAD_CONST_PER_LOOP</name>
      <anchorfile>mia__lp__prel_8c.html</anchorfile>
      <anchor>aed64aa9c0e25b9678cae9cc3ae949cb</anchor>
      <arglist></arglist>
    </member>
    <member kind="define">
      <type>#define</type>
      <name>MAX_LOOP_SIZE_WITH_NESTED_LOOPS</name>
      <anchorfile>mia__lp__prel_8c.html</anchorfile>
      <anchor>bc069227c45c46cae6d764fe701257a7</anchor>
      <arglist></arglist>
    </member>
    <member kind="define">
      <type>#define</type>
      <name>MAX_LOOP_SIZE_FOR_PRELOAD</name>
      <anchorfile>mia__lp__prel_8c.html</anchorfile>
      <anchor>afb2c1d818f840b067728f20bc3c09bb</anchor>
      <arglist></arglist>
    </member>
    <member kind="define">
      <type>#define</type>
      <name>L_MAX_REQUEST</name>
      <anchorfile>mia__lp__prel_8c.html</anchorfile>
      <anchor>7c5f7f8fbb899748dc21279096396b11</anchor>
      <arglist></arglist>
    </member>
    <member kind="function" static="yes">
      <type>static int</type>
      <name>need_preload</name>
      <anchorfile>mia__lp__prel_8c.html</anchorfile>
      <anchor>d8767f4e13897f750ec4583bc24699a4</anchor>
      <arglist>(L_Oper *oper, L_Operand *operand, int src_num)</arglist>
    </member>
    <member kind="function" static="yes">
      <type>static void</type>
      <name>flush_preload_request</name>
      <anchorfile>mia__lp__prel_8c.html</anchorfile>
      <anchor>f9b3bc9bc46f68ad66bf5419d5810dbf</anchor>
      <arglist>()</arglist>
    </member>
    <member kind="function" static="yes">
      <type>static void</type>
      <name>request_for_preload</name>
      <anchorfile>mia__lp__prel_8c.html</anchorfile>
      <anchor>253f57304a67b2453b2ec4bb061666c7</anchor>
      <arglist>(L_Oper *oper, L_Operand **operand)</arglist>
    </member>
    <member kind="function" static="yes">
      <type>static void</type>
      <name>add_preload_code</name>
      <anchorfile>mia__lp__prel_8c.html</anchorfile>
      <anchor>586a3bf339800125349ab83fb8e5629f</anchor>
      <arglist>(L_Func *fn, L_Cb *preheader_cb)</arglist>
    </member>
    <member kind="function" static="yes">
      <type>static void</type>
      <name>replace_preload_operand</name>
      <anchorfile>mia__lp__prel_8c.html</anchorfile>
      <anchor>c00fb0eb1175b6e432c0db7b3f466ca2</anchor>
      <arglist>()</arglist>
    </member>
    <member kind="function" static="yes">
      <type>static void</type>
      <name>preload_loop</name>
      <anchorfile>mia__lp__prel_8c.html</anchorfile>
      <anchor>59ae35d80101b898bda2b30736db1065</anchor>
      <arglist>(L_Func *fn, L_Loop *loop)</arglist>
    </member>
    <member kind="function">
      <type>void</type>
      <name>Mopti_constant_generation</name>
      <anchorfile>mia__lp__prel_8c.html</anchorfile>
      <anchor>ce49d15c4fd47f651213d2f0a5df9f71</anchor>
      <arglist>(L_Func *fn)</arglist>
    </member>
    <member kind="variable">
      <type>double</type>
      <name>Lconst_min_preload_weight</name>
      <anchorfile>mia__lp__prel_8c.html</anchorfile>
      <anchor>f0df201664b603e5b52607e588b40269</anchor>
      <arglist></arglist>
    </member>
    <member kind="variable">
      <type>int</type>
      <name>Lconst_max_preload_const_per_loop</name>
      <anchorfile>mia__lp__prel_8c.html</anchorfile>
      <anchor>4970b15537e25c34343c0819cd183b73</anchor>
      <arglist></arglist>
    </member>
    <member kind="variable">
      <type>int</type>
      <name>Lconst_max_loop_size_with_nested_loops</name>
      <anchorfile>mia__lp__prel_8c.html</anchorfile>
      <anchor>59de3285284f791c68c738c8086406d3</anchor>
      <arglist></arglist>
    </member>
    <member kind="variable">
      <type>int</type>
      <name>Lconst_max_loop_size_for_preload</name>
      <anchorfile>mia__lp__prel_8c.html</anchorfile>
      <anchor>59948d23605b2eddc8dd0e0888247d92</anchor>
      <arglist></arglist>
    </member>
    <member kind="variable" static="yes">
      <type>static int</type>
      <name>n_request</name>
      <anchorfile>mia__lp__prel_8c.html</anchorfile>
      <anchor>af67953e47646ec9b39eae7c699b370b</anchor>
      <arglist></arglist>
    </member>
    <member kind="variable" static="yes">
      <type>static L_Oper *</type>
      <name>request_oper</name>
      <anchorfile>mia__lp__prel_8c.html</anchorfile>
      <anchor>fe951214eacc34396c8347408cc0a0b7</anchor>
      <arglist>[L_MAX_REQUEST]</arglist>
    </member>
    <member kind="variable" static="yes">
      <type>static L_Operand **</type>
      <name>request_operand</name>
      <anchorfile>mia__lp__prel_8c.html</anchorfile>
      <anchor>1bf5a3c6253a59a59b7b109fece0821b</anchor>
      <arglist>[L_MAX_REQUEST]</arglist>
    </member>
    <member kind="variable" static="yes">
      <type>static int</type>
      <name>n_preload</name>
      <anchorfile>mia__lp__prel_8c.html</anchorfile>
      <anchor>98c49189df14e601226192bb52d69b12</anchor>
      <arglist></arglist>
    </member>
    <member kind="variable" static="yes">
      <type>static L_Operand</type>
      <name>preload_constant</name>
      <anchorfile>mia__lp__prel_8c.html</anchorfile>
      <anchor>55cf12a2bcb4d565835bc04cac373988</anchor>
      <arglist>[L_MAX_REQUEST]</arglist>
    </member>
    <member kind="variable" static="yes">
      <type>static L_Operand *</type>
      <name>preload_register</name>
      <anchorfile>mia__lp__prel_8c.html</anchorfile>
      <anchor>0d335db781bd550c382dbc3b90d378cb</anchor>
      <arglist>[L_MAX_REQUEST]</arglist>
    </member>
    <member kind="variable" static="yes">
      <type>static double</type>
      <name>preload_weight</name>
      <anchorfile>mia__lp__prel_8c.html</anchorfile>
      <anchor>efd9a311b6f65396d2bbdfd427a623c7</anchor>
      <arglist>[L_MAX_REQUEST]</arglist>
    </member>
    <member kind="variable" static="yes">
      <type>static int</type>
      <name>preload</name>
      <anchorfile>mia__lp__prel_8c.html</anchorfile>
      <anchor>c0e0a3d5ef59f3fb14c50d1c6f427555</anchor>
      <arglist>[L_MAX_REQUEST]</arglist>
    </member>
  </compound>
  <compound kind="file">
    <name>mia_opti.c</name>
    <path>/mnt/hgfs/mkt/Desktop/openimpact/src/Lcode/codegen/Mopti-ia64/</path>
    <filename>mia__opti_8c</filename>
    <includes id="mia__internal_8h" name="mia_internal.h" local="yes" imported="no">mia_internal.h</includes>
    <member kind="define">
      <type>#define</type>
      <name>Mopti_imax_num_iterations</name>
      <anchorfile>mia__opti_8c.html</anchorfile>
      <anchor>995c4a8c06b5e07bb030f4076dfb24e2</anchor>
      <arglist></arglist>
    </member>
    <member kind="define">
      <type>#define</type>
      <name>Mopti_omax_num_iterations</name>
      <anchorfile>mia__opti_8c.html</anchorfile>
      <anchor>d3fac62b60f73e33cfe102020c40ea2f</anchor>
      <arglist></arglist>
    </member>
    <member kind="define">
      <type>#define</type>
      <name>MAX_NUM_GLOB_ITERATION</name>
      <anchorfile>mia__opti_8c.html</anchorfile>
      <anchor>7710e11cc3b7e2b41794e3e16b339039</anchor>
      <arglist></arglist>
    </member>
    <member kind="define">
      <type>#define</type>
      <name>MAX_RD</name>
      <anchorfile>mia__opti_8c.html</anchorfile>
      <anchor>1ae192442d9e050e872356cd095ae18f</anchor>
      <arglist></arglist>
    </member>
    <member kind="define">
      <type>#define</type>
      <name>TEST_GLOB_COPY_PROP</name>
      <anchorfile>mia__opti_8c.html</anchorfile>
      <anchor>af60ad1a1361c3d327f1125e6f3cd23b</anchor>
      <arglist></arglist>
    </member>
    <member kind="function" static="yes">
      <type>static void</type>
      <name>Mopti_init</name>
      <anchorfile>mia__opti_8c.html</anchorfile>
      <anchor>fd207341cfc48b1d9e2d578bbcd21f17</anchor>
      <arglist>()</arglist>
    </member>
    <member kind="function" static="yes">
      <type>static void</type>
      <name>Mopti_deinit</name>
      <anchorfile>mia__opti_8c.html</anchorfile>
      <anchor>df86bc56421172be873863d24f1c2c81</anchor>
      <arglist>()</arglist>
    </member>
    <member kind="function" static="yes">
      <type>static int</type>
      <name>M_global_code_optimization</name>
      <anchorfile>mia__opti_8c.html</anchorfile>
      <anchor>efb384e57ee7a4b575333ba00dcc9af4</anchor>
      <arglist>(L_Func *fn)</arglist>
    </member>
    <member kind="function" static="yes">
      <type>static int</type>
      <name>M_global_common_subexpression</name>
      <anchorfile>mia__opti_8c.html</anchorfile>
      <anchor>120aab48a0f35d564a275f386ee97ff7</anchor>
      <arglist>(L_Cb *cbA, L_Cb *cbB, int move_flags)</arglist>
    </member>
    <member kind="function" static="yes">
      <type>static int</type>
      <name>M_global_copy_propagation</name>
      <anchorfile>mia__opti_8c.html</anchorfile>
      <anchor>3fb0912ca5bfeb41eb69edd4e45911b9</anchor>
      <arglist>(L_Cb *cbA, L_Cb *cbB)</arglist>
    </member>
    <member kind="function" static="yes">
      <type>static int</type>
      <name>M_local_add_regrouping</name>
      <anchorfile>mia__opti_8c.html</anchorfile>
      <anchor>6f2dfadb3ed374c1b6a8569bcbf34e28</anchor>
      <arglist>(L_Cb *cb)</arglist>
    </member>
    <member kind="function" static="yes">
      <type>static int</type>
      <name>Mopti_global_dead_code_optimization</name>
      <anchorfile>mia__opti_8c.html</anchorfile>
      <anchor>565d7b26894474523f285a8b2769081b</anchor>
      <arglist>(L_Func *fn)</arglist>
    </member>
    <member kind="function" static="yes">
      <type>static int</type>
      <name>M_local_redmov_removal</name>
      <anchorfile>mia__opti_8c.html</anchorfile>
      <anchor>ed2068a7c6bbc404d8f5efc063bac047</anchor>
      <arglist>(L_Cb *cb)</arglist>
    </member>
    <member kind="function" static="yes">
      <type>static int</type>
      <name>M_local_cmp_copy_propagation</name>
      <anchorfile>mia__opti_8c.html</anchorfile>
      <anchor>e06573b2db767a2f5ddc594506a1e09a</anchor>
      <arglist>(L_Cb *cb)</arglist>
    </member>
    <member kind="function" static="yes">
      <type>static int</type>
      <name>M_local_cmp_rev_copy_propagation</name>
      <anchorfile>mia__opti_8c.html</anchorfile>
      <anchor>e8bc412e2658f0d8d627b02e5a720f4c</anchor>
      <arglist>(L_Cb *cb)</arglist>
    </member>
    <member kind="function" static="yes">
      <type>static int</type>
      <name>M_global_cmp_copy_propagation</name>
      <anchorfile>mia__opti_8c.html</anchorfile>
      <anchor>7b7e8106a32096cc469acedf6fd9cbd3</anchor>
      <arglist>(L_Cb *cbA, L_Cb *cbB)</arglist>
    </member>
    <member kind="function">
      <type>int</type>
      <name>M_local_constant_folding</name>
      <anchorfile>mia__opti_8c.html</anchorfile>
      <anchor>2f87ad062e11eb5db1b41aebeeb33243</anchor>
      <arglist>(L_Cb *cb)</arglist>
    </member>
    <member kind="function">
      <type>int</type>
      <name>M_local_copy_propagation</name>
      <anchorfile>mia__opti_8c.html</anchorfile>
      <anchor>c1d1622c65471a90dd6bc5177f3b781c</anchor>
      <arglist>(L_Cb *cb, int use_dem)</arglist>
    </member>
    <member kind="function">
      <type>int</type>
      <name>M_local_rev_copy_propagation</name>
      <anchorfile>mia__opti_8c.html</anchorfile>
      <anchor>fdb6e98593a83266abde366ac047f0a4</anchor>
      <arglist>(L_Cb *cb, int use_dem)</arglist>
    </member>
    <member kind="function">
      <type>int</type>
      <name>M_local_constant_propagation</name>
      <anchorfile>mia__opti_8c.html</anchorfile>
      <anchor>3c620088a0f7a670d6e77a6563761dc4</anchor>
      <arglist>(L_Cb *cb)</arglist>
    </member>
    <member kind="function">
      <type>void</type>
      <name>L_read_parm_mopti</name>
      <anchorfile>mia__opti_8c.html</anchorfile>
      <anchor>c4127ccba417f74a6dda847e3908e13f</anchor>
      <arglist>(Parm_Parse_Info *ppi)</arglist>
    </member>
    <member kind="function">
      <type>void</type>
      <name>Mopti_perform_optimizations_tahoe</name>
      <anchorfile>mia__opti_8c.html</anchorfile>
      <anchor>b608d83646fb9d2a6cb3198df341cb05</anchor>
      <arglist>(L_Func *fn)</arglist>
    </member>
    <member kind="function">
      <type>void</type>
      <name>Mopti_phase2_optimizations</name>
      <anchorfile>mia__opti_8c.html</anchorfile>
      <anchor>858bb385579a19e2bd869f96a17e52ee</anchor>
      <arglist>(L_Func *fn)</arglist>
    </member>
    <member kind="function" static="yes">
      <type>static int</type>
      <name>Mopti_redundant_ext_rev_set</name>
      <anchorfile>mia__opti_8c.html</anchorfile>
      <anchor>a3796c3a20e6502bbe4e88682fc485b6</anchor>
      <arglist>(L_Func *fn, L_Oper *oper, Set rd_set)</arglist>
    </member>
    <member kind="function">
      <type>int</type>
      <name>Mopti_global_sxt_elimination</name>
      <anchorfile>mia__opti_8c.html</anchorfile>
      <anchor>258d13a44eb860f3f27738a61d97f914</anchor>
      <arglist>(L_Func *fn)</arglist>
    </member>
    <member kind="function">
      <type>void</type>
      <name>Mopti_debug</name>
      <anchorfile>mia__opti_8c.html</anchorfile>
      <anchor>ca52ba4b3401fe69d84481a0a4e0cb94</anchor>
      <arglist>(char *fmt,...)</arglist>
    </member>
    <member kind="variable" static="yes">
      <type>static int</type>
      <name>Mopti_init_performed</name>
      <anchorfile>mia__opti_8c.html</anchorfile>
      <anchor>381daf2a188a9e511257baf6324a6c41</anchor>
      <arglist></arglist>
    </member>
    <member kind="variable">
      <type>int</type>
      <name>Mopti_print_stats</name>
      <anchorfile>mia__opti_8c.html</anchorfile>
      <anchor>8b2b3e7d98bd625d28e844932802ef08</anchor>
      <arglist></arglist>
    </member>
    <member kind="variable" static="yes">
      <type>static int</type>
      <name>Mopti_do_local_opti</name>
      <anchorfile>mia__opti_8c.html</anchorfile>
      <anchor>ecc68a11b0c65b24fdb4f45c54c07c56</anchor>
      <arglist></arglist>
    </member>
    <member kind="variable" static="yes">
      <type>static int</type>
      <name>Mopti_do_local_common_subs</name>
      <anchorfile>mia__opti_8c.html</anchorfile>
      <anchor>4234ca7ed3370ad04a9d1344b1dd986a</anchor>
      <arglist></arglist>
    </member>
    <member kind="variable" static="yes">
      <type>static int</type>
      <name>Mopti_do_local_copy_prop</name>
      <anchorfile>mia__opti_8c.html</anchorfile>
      <anchor>dab9fc8b7eef6f82391c1a1bbba0af86</anchor>
      <arglist></arglist>
    </member>
    <member kind="variable" static="yes">
      <type>static int</type>
      <name>Mopti_do_local_rev_copy_prop</name>
      <anchorfile>mia__opti_8c.html</anchorfile>
      <anchor>b12d2173535cfb38b1e6331b4dece929</anchor>
      <arglist></arglist>
    </member>
    <member kind="variable" static="yes">
      <type>static int</type>
      <name>Mopti_do_local_const_prop</name>
      <anchorfile>mia__opti_8c.html</anchorfile>
      <anchor>155dcb4cd0276c909547111069daa448</anchor>
      <arglist></arglist>
    </member>
    <member kind="variable" static="yes">
      <type>static int</type>
      <name>Mopti_do_local_const_fold</name>
      <anchorfile>mia__opti_8c.html</anchorfile>
      <anchor>5087c7715b6125da5a7ddba1849771b8</anchor>
      <arglist></arglist>
    </member>
    <member kind="variable" static="yes">
      <type>static int</type>
      <name>Mopti_do_local_add_regrouping</name>
      <anchorfile>mia__opti_8c.html</anchorfile>
      <anchor>728c5c38380b2b2061bb6c6a2ce8cce0</anchor>
      <arglist></arglist>
    </member>
    <member kind="variable" static="yes">
      <type>static int</type>
      <name>Mopti_do_dead_code</name>
      <anchorfile>mia__opti_8c.html</anchorfile>
      <anchor>319796a6594f619cc0efe25feb90c54d</anchor>
      <arglist></arglist>
    </member>
    <member kind="variable" static="yes">
      <type>static int</type>
      <name>Mopti_do_global_opti</name>
      <anchorfile>mia__opti_8c.html</anchorfile>
      <anchor>f02ec830d5166dbd8655a1434360c610</anchor>
      <arglist></arglist>
    </member>
    <member kind="variable" static="yes">
      <type>static int</type>
      <name>Mopti_do_global_common_subs</name>
      <anchorfile>mia__opti_8c.html</anchorfile>
      <anchor>d3250bc71ba3b4303a01b77d4e319b76</anchor>
      <arglist></arglist>
    </member>
    <member kind="variable" static="yes">
      <type>static int</type>
      <name>Mopti_do_global_copy_prop</name>
      <anchorfile>mia__opti_8c.html</anchorfile>
      <anchor>7e79cada54396ebd2f6ad61b82e592bd</anchor>
      <arglist></arglist>
    </member>
    <member kind="variable" static="yes">
      <type>static int</type>
      <name>Mopti_do_coalescing</name>
      <anchorfile>mia__opti_8c.html</anchorfile>
      <anchor>b33ccb053f8564ebe14188666d0b8400</anchor>
      <arglist></arglist>
    </member>
    <member kind="variable">
      <type>int</type>
      <name>Mopti_do_compare_height_reduction</name>
      <anchorfile>mia__opti_8c.html</anchorfile>
      <anchor>ec0d753452d49056ce88896a11e5c07a</anchor>
      <arglist></arglist>
    </member>
    <member kind="variable">
      <type>int</type>
      <name>Mopti_constant_preloading</name>
      <anchorfile>mia__opti_8c.html</anchorfile>
      <anchor>833cedb394552394eaa0a30f872527ea</anchor>
      <arglist></arglist>
    </member>
    <member kind="variable">
      <type>int</type>
      <name>Mopti_shift_add_merge</name>
      <anchorfile>mia__opti_8c.html</anchorfile>
      <anchor>3b37cc605784201bd63a25c531aa19c7</anchor>
      <arglist></arglist>
    </member>
    <member kind="variable">
      <type>int</type>
      <name>Mopti_do_epilogue_merge</name>
      <anchorfile>mia__opti_8c.html</anchorfile>
      <anchor>909d1a63e2b2648bd22f598ab4c7db89</anchor>
      <arglist></arglist>
    </member>
    <member kind="variable">
      <type>int</type>
      <name>Mopti2_redundant_memory_ops</name>
      <anchorfile>mia__opti_8c.html</anchorfile>
      <anchor>3080904fd0567421bcd0c45db18b2aab</anchor>
      <arglist></arglist>
    </member>
    <member kind="variable">
      <type>int</type>
      <name>Mopti_debug_messages</name>
      <anchorfile>mia__opti_8c.html</anchorfile>
      <anchor>a3765515b3e09be198267ef29d84be58</anchor>
      <arglist></arglist>
    </member>
  </compound>
  <compound kind="file">
    <name>mia_opti.h</name>
    <path>/mnt/hgfs/mkt/Desktop/openimpact/src/Lcode/codegen/Mopti-ia64/</path>
    <filename>mia__opti_8h</filename>
    <member kind="function">
      <type>void</type>
      <name>L_read_parm_mopti</name>
      <anchorfile>mia__opti_8h.html</anchorfile>
      <anchor>c4127ccba417f74a6dda847e3908e13f</anchor>
      <arglist>(Parm_Parse_Info *ppi)</arglist>
    </member>
    <member kind="function">
      <type>void</type>
      <name>Mopti_perform_optimizations_tahoe</name>
      <anchorfile>mia__opti_8h.html</anchorfile>
      <anchor>b608d83646fb9d2a6cb3198df341cb05</anchor>
      <arglist>(L_Func *fn)</arglist>
    </member>
    <member kind="function">
      <type>void</type>
      <name>Mopti_phase2_optimizations</name>
      <anchorfile>mia__opti_8h.html</anchorfile>
      <anchor>858bb385579a19e2bd869f96a17e52ee</anchor>
      <arglist>(L_Func *fn)</arglist>
    </member>
    <member kind="function">
      <type>int</type>
      <name>L_no_other_use_in_cb_after</name>
      <anchorfile>mia__opti_8h.html</anchorfile>
      <anchor>5cfad794d6ca81cea76676f32c668147</anchor>
      <arglist>(L_Cb *cb, L_Operand *operand, L_Oper *after_oper, L_Oper *except_oper)</arglist>
    </member>
    <member kind="function">
      <type>int</type>
      <name>L_no_other_def_use_in_cb</name>
      <anchorfile>mia__opti_8h.html</anchorfile>
      <anchor>efb3522868e2146cffcb05da824b08cb</anchor>
      <arglist>(L_Cb *cb, L_Operand *operand, L_Oper *except1, L_Oper *except2, L_Oper *except3)</arglist>
    </member>
    <member kind="function">
      <type>void</type>
      <name>Mopti_convert_ldf_to_ldi</name>
      <anchorfile>mia__opti_8h.html</anchorfile>
      <anchor>4afb2b153015f2c004a58a177fea24a4</anchor>
      <arglist>(L_Func *fn)</arglist>
    </member>
    <member kind="function">
      <type>int</type>
      <name>Mopti_global_sxt_elimination</name>
      <anchorfile>mia__opti_8h.html</anchorfile>
      <anchor>258d13a44eb860f3f27738a61d97f914</anchor>
      <arglist>(L_Func *fn)</arglist>
    </member>
    <member kind="function">
      <type>void</type>
      <name>Mopti_epilogue_merge</name>
      <anchorfile>mia__opti_8h.html</anchorfile>
      <anchor>9bd4d0715f1c8006f178d8d2e1664cbc</anchor>
      <arglist>(L_Func *fn)</arglist>
    </member>
    <member kind="function">
      <type>void</type>
      <name>Mopti_epilogue_cleanup</name>
      <anchorfile>mia__opti_8h.html</anchorfile>
      <anchor>b609816a640af52484e7da61b0140c69</anchor>
      <arglist>(L_Func *fn)</arglist>
    </member>
    <member kind="function">
      <type>int</type>
      <name>Mopti_branch_target_expansion</name>
      <anchorfile>mia__opti_8h.html</anchorfile>
      <anchor>ecc2271fa0113f92de8fd859a24e2e26</anchor>
      <arglist>(L_Func *fn)</arglist>
    </member>
    <member kind="function">
      <type>void</type>
      <name>Mia_post_increment_conversion</name>
      <anchorfile>mia__opti_8h.html</anchorfile>
      <anchor>c64f000349a77e579eab4fd04c6e35fd</anchor>
      <arglist>(L_Func *fn, int ld, int st)</arglist>
    </member>
    <member kind="function">
      <type>void</type>
      <name>Mia_softpipe_post_increment_conversion</name>
      <anchorfile>mia__opti_8h.html</anchorfile>
      <anchor>b166448104dc3404af2152b2103c8dde</anchor>
      <arglist>(L_Func *fn, int ld, int st)</arglist>
    </member>
    <member kind="function">
      <type>void</type>
      <name>Mopti_tahoe_reductions</name>
      <anchorfile>mia__opti_8h.html</anchorfile>
      <anchor>3595761e624a04c748a0d7d8f03578e8</anchor>
      <arglist>(L_Func *fn)</arglist>
    </member>
    <member kind="function">
      <type>void</type>
      <name>Mopti_shladd</name>
      <anchorfile>mia__opti_8h.html</anchorfile>
      <anchor>3d5b5f62b487eb1cdb88601414214ef5</anchor>
      <arglist>(L_Func *fn)</arglist>
    </member>
    <member kind="function">
      <type>int</type>
      <name>M_local_shladdp4</name>
      <anchorfile>mia__opti_8h.html</anchorfile>
      <anchor>8a0c64a065ae8bb6c6df800c60009ab5</anchor>
      <arglist>(L_Cb *cb, int *reaching_df_done)</arglist>
    </member>
    <member kind="function">
      <type>int</type>
      <name>M_local_constant_folding</name>
      <anchorfile>mia__opti_8h.html</anchorfile>
      <anchor>2f87ad062e11eb5db1b41aebeeb33243</anchor>
      <arglist>(L_Cb *cb)</arglist>
    </member>
    <member kind="function">
      <type>void</type>
      <name>Mopti_constant_generation</name>
      <anchorfile>mia__opti_8h.html</anchorfile>
      <anchor>ce49d15c4fd47f651213d2f0a5df9f71</anchor>
      <arglist>(L_Func *fn)</arglist>
    </member>
    <member kind="variable">
      <type>int</type>
      <name>Mopti_constant_preloading</name>
      <anchorfile>mia__opti_8h.html</anchorfile>
      <anchor>833cedb394552394eaa0a30f872527ea</anchor>
      <arglist></arglist>
    </member>
    <member kind="variable">
      <type>int</type>
      <name>Mopti_shift_add_merge</name>
      <anchorfile>mia__opti_8h.html</anchorfile>
      <anchor>3b37cc605784201bd63a25c531aa19c7</anchor>
      <arglist></arglist>
    </member>
    <member kind="variable">
      <type>int</type>
      <name>Mopti_do_epilogue_merge</name>
      <anchorfile>mia__opti_8h.html</anchorfile>
      <anchor>909d1a63e2b2648bd22f598ab4c7db89</anchor>
      <arglist></arglist>
    </member>
    <member kind="variable">
      <type>int</type>
      <name>Mopti_print_stats</name>
      <anchorfile>mia__opti_8h.html</anchorfile>
      <anchor>8b2b3e7d98bd625d28e844932802ef08</anchor>
      <arglist></arglist>
    </member>
    <member kind="variable">
      <type>int</type>
      <name>Mopti_do_predicate_opti</name>
      <anchorfile>mia__opti_8h.html</anchorfile>
      <anchor>51a650084d8993f0b36abc45b91a4adc</anchor>
      <arglist></arglist>
    </member>
    <member kind="variable">
      <type>int</type>
      <name>Mopti_do_opp_cond_combining</name>
      <anchorfile>mia__opti_8h.html</anchorfile>
      <anchor>1d2850f158cd76c13c056ae8aad6192d</anchor>
      <arglist></arglist>
    </member>
    <member kind="variable">
      <type>int</type>
      <name>Mopti_do_and_cmp_promotion</name>
      <anchorfile>mia__opti_8h.html</anchorfile>
      <anchor>f77d0c9b490432b3bddb0ec0c327ac75</anchor>
      <arglist></arglist>
    </member>
    <member kind="variable">
      <type>int</type>
      <name>Mopti_do_DeMorgan_combining</name>
      <anchorfile>mia__opti_8h.html</anchorfile>
      <anchor>8b19a256a4bb9e6007b16a0507972f26</anchor>
      <arglist></arglist>
    </member>
    <member kind="variable">
      <type>int</type>
      <name>Mopti_do_redundant_compare_removal</name>
      <anchorfile>mia__opti_8h.html</anchorfile>
      <anchor>fd8bb89459b6ca12b3f25a14c0ca5df0</anchor>
      <arglist></arglist>
    </member>
    <member kind="variable">
      <type>int</type>
      <name>Mopti_do_remove_decidable_compares</name>
      <anchorfile>mia__opti_8h.html</anchorfile>
      <anchor>c6a078debf78a572a737d66125482475</anchor>
      <arglist></arglist>
    </member>
    <member kind="variable">
      <type>int</type>
      <name>Mopti_do_pred_init_combining</name>
      <anchorfile>mia__opti_8h.html</anchorfile>
      <anchor>4334e7bf05f7a74fb5adc5cc1f3beaa6</anchor>
      <arglist></arglist>
    </member>
    <member kind="variable">
      <type>int</type>
      <name>Mopti_do_dead_pred_def_removal</name>
      <anchorfile>mia__opti_8h.html</anchorfile>
      <anchor>369d41cc2e4f427051545c5f8add5379</anchor>
      <arglist></arglist>
    </member>
    <member kind="variable">
      <type>int</type>
      <name>Mopti_do_pred_copy_removal</name>
      <anchorfile>mia__opti_8h.html</anchorfile>
      <anchor>bf26099647eb74b37695a525fc92176e</anchor>
      <arglist></arglist>
    </member>
    <member kind="variable">
      <type>int</type>
      <name>Mopti_do_sp_removal</name>
      <anchorfile>mia__opti_8h.html</anchorfile>
      <anchor>bc0b2438eff7cdb627bf0718bc47a26f</anchor>
      <arglist></arglist>
    </member>
    <member kind="variable">
      <type>int</type>
      <name>Mopti2_redundant_memory_ops</name>
      <anchorfile>mia__opti_8h.html</anchorfile>
      <anchor>3080904fd0567421bcd0c45db18b2aab</anchor>
      <arglist></arglist>
    </member>
    <member kind="variable">
      <type>int</type>
      <name>Mopti_debug_messages</name>
      <anchorfile>mia__opti_8h.html</anchorfile>
      <anchor>a3765515b3e09be198267ef29d84be58</anchor>
      <arglist></arglist>
    </member>
  </compound>
  <compound kind="file">
    <name>mia_post_inc.c</name>
    <path>/mnt/hgfs/mkt/Desktop/openimpact/src/Lcode/codegen/Mopti-ia64/</path>
    <filename>mia__post__inc_8c</filename>
    <includes id="mia__internal_8h" name="mia_internal.h" local="yes" imported="no">mia_internal.h</includes>
    <member kind="function" static="yes">
      <type>static void</type>
      <name>Mia_cb_post_increment</name>
      <anchorfile>mia__post__inc_8c.html</anchorfile>
      <anchor>819c0b592ff0708b08f669148b1d7994</anchor>
      <arglist>(L_Cb *cb, int conservative, int ld, int st)</arglist>
    </member>
    <member kind="function">
      <type>void</type>
      <name>Mia_post_increment_conversion</name>
      <anchorfile>mia__post__inc_8c.html</anchorfile>
      <anchor>c64f000349a77e579eab4fd04c6e35fd</anchor>
      <arglist>(L_Func *fn, int ld, int st)</arglist>
    </member>
    <member kind="function">
      <type>void</type>
      <name>Mia_softpipe_post_increment_conversion</name>
      <anchorfile>mia__post__inc_8c.html</anchorfile>
      <anchor>b166448104dc3404af2152b2103c8dde</anchor>
      <arglist>(L_Func *fn, int ld, int st)</arglist>
    </member>
    <member kind="function" static="yes">
      <type>static void</type>
      <name>Convert_mem_op_to_post_inc</name>
      <anchorfile>mia__post__inc_8c.html</anchorfile>
      <anchor>cc3b1a3c4ed81392623e05754cfc23c3</anchor>
      <arglist>(L_Oper *mem_oper, L_Operand *inc_operand)</arglist>
    </member>
    <member kind="function" static="yes">
      <type>static int</type>
      <name>L_live_out_inhibits_scheduling</name>
      <anchorfile>mia__post__inc_8c.html</anchorfile>
      <anchor>e0cf80e1d56b9e03208df238e7fe66ac</anchor>
      <arglist>(L_Cb *cb, L_Operand *operand, L_Oper *add)</arglist>
    </member>
  </compound>
  <compound kind="file">
    <name>mia_shladd.c</name>
    <path>/mnt/hgfs/mkt/Desktop/openimpact/src/Lcode/codegen/Mopti-ia64/</path>
    <filename>mia__shladd_8c</filename>
    <includes id="mia__internal_8h" name="mia_internal.h" local="yes" imported="no">mia_internal.h</includes>
    <member kind="function" static="yes">
      <type>static int</type>
      <name>M_local_shift_add_merge</name>
      <anchorfile>mia__shladd_8c.html</anchorfile>
      <anchor>d4b254622bd84ff523641dd56b2d27d2</anchor>
      <arglist>(L_Cb *cb)</arglist>
    </member>
    <member kind="function" static="yes">
      <type>static int</type>
      <name>M_global_shift_add_merge</name>
      <anchorfile>mia__shladd_8c.html</anchorfile>
      <anchor>a9d61f333746a1466931127f228aff26</anchor>
      <arglist>(L_Cb *cbA, L_Cb *cbB)</arglist>
    </member>
    <member kind="function" static="yes">
      <type>static int</type>
      <name>M_does_operand_contain_label_addr</name>
      <anchorfile>mia__shladd_8c.html</anchorfile>
      <anchor>b9a272e9168c70ced2c1e3b833b4ca26</anchor>
      <arglist>(L_Oper *oper, L_Operand *operand, int *reaching_df_done)</arglist>
    </member>
    <member kind="function">
      <type>void</type>
      <name>Mopti_shladd</name>
      <anchorfile>mia__shladd_8c.html</anchorfile>
      <anchor>3d5b5f62b487eb1cdb88601414214ef5</anchor>
      <arglist>(L_Func *fn)</arglist>
    </member>
  </compound>
</tagfile>
