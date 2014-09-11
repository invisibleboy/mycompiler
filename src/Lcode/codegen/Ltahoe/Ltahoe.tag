<?xml version='1.0' encoding='ISO-8859-1' standalone='yes' ?>
<tagfile>
  <compound kind="file">
    <name>ltahoe_bitvec.h</name>
    <path>/mnt/hgfs/mkt/Desktop/openimpact/src/Lcode/codegen/Ltahoe/</path>
    <filename>ltahoe__bitvec_8h</filename>
    <member kind="define">
      <type>#define</type>
      <name>LT_ALL_ONES</name>
      <anchorfile>ltahoe__bitvec_8h.html</anchorfile>
      <anchor>9c3cc57a46dc45f89c143d29b2075338</anchor>
      <arglist></arglist>
    </member>
    <member kind="define">
      <type>#define</type>
      <name>LT_ONES_SET_LO</name>
      <anchorfile>ltahoe__bitvec_8h.html</anchorfile>
      <anchor>42fb675ebfcc442871444c274628fe5d</anchor>
      <arglist>(o)</arglist>
    </member>
    <member kind="define">
      <type>#define</type>
      <name>LT_ONES_SET_HI</name>
      <anchorfile>ltahoe__bitvec_8h.html</anchorfile>
      <anchor>2298ce08afd29ade390225ba7e5a8749</anchor>
      <arglist>(o)</arglist>
    </member>
    <member kind="define">
      <type>#define</type>
      <name>LT_ZEROS_SET_LO</name>
      <anchorfile>ltahoe__bitvec_8h.html</anchorfile>
      <anchor>ce6c7fcc1a1f7ca90885540e5fd03bec</anchor>
      <arglist>(o)</arglist>
    </member>
    <member kind="define">
      <type>#define</type>
      <name>LT_ZEROS_SET_HI</name>
      <anchorfile>ltahoe__bitvec_8h.html</anchorfile>
      <anchor>1e5fd7c79ecb5a27dbb9a63a3005e3c5</anchor>
      <arglist>(o)</arglist>
    </member>
    <member kind="define">
      <type>#define</type>
      <name>LT_EXTRACT</name>
      <anchorfile>ltahoe__bitvec_8h.html</anchorfile>
      <anchor>33e79a83b6a57b7e54dd3f9983d8e5f2</anchor>
      <arglist>(v, p, l)</arglist>
    </member>
    <member kind="define">
      <type>#define</type>
      <name>LT_SET_BIT</name>
      <anchorfile>ltahoe__bitvec_8h.html</anchorfile>
      <anchor>9c9038a46ef2dcf03801f2dbd4e99398</anchor>
      <arglist>(b)</arglist>
    </member>
    <member kind="typedef">
      <type>ITuint64</type>
      <name>LT_bit_vector</name>
      <anchorfile>ltahoe__bitvec_8h.html</anchorfile>
      <anchor>1a1643f080081c3168edd9c113b293b0</anchor>
      <arglist></arglist>
    </member>
  </compound>
  <compound kind="file">
    <name>ltahoe_codegen.c</name>
    <path>/mnt/hgfs/mkt/Desktop/openimpact/src/Lcode/codegen/Ltahoe/</path>
    <filename>ltahoe__codegen_8c</filename>
    <includes id="ltahoe__main_8h" name="ltahoe_main.h" local="yes" imported="no">ltahoe_main.h</includes>
    <includes id="phase1__func_8h" name="phase1_func.h" local="yes" imported="no">phase1_func.h</includes>
    <includes id="phase2__func_8h" name="phase2_func.h" local="yes" imported="no">phase2_func.h</includes>
    <includes id="phase3_8h" name="phase3.h" local="yes" imported="no">phase3.h</includes>
    <member kind="function" static="yes">
      <type>static void</type>
      <name>Ltahoe_process_input</name>
      <anchorfile>ltahoe__codegen_8c.html</anchorfile>
      <anchor>ae6408a9132cbe944cab2c84593c43e2</anchor>
      <arglist>(Parm_Macro_List *command_line_macro_list)</arglist>
    </member>
    <member kind="function">
      <type>void</type>
      <name>L_gen_code</name>
      <anchorfile>ltahoe__codegen_8c.html</anchorfile>
      <anchor>6346124c5fad5929833a7f7084f6be8c</anchor>
      <arglist>(Parm_Macro_List *command_line_macro_list)</arglist>
    </member>
  </compound>
  <compound kind="file">
    <name>ltahoe_completers.c</name>
    <path>/mnt/hgfs/mkt/Desktop/openimpact/src/Lcode/codegen/Ltahoe/</path>
    <filename>ltahoe__completers_8c</filename>
    <includes id="ltahoe__main_8h" name="ltahoe_main.h" local="yes" imported="no">ltahoe_main.h</includes>
    <includes id="ltahoe__completers_8h" name="ltahoe_completers.h" local="yes" imported="no">ltahoe_completers.h</includes>
    <includes id="phase3_8h" name="phase3.h" local="yes" imported="no">phase3.h</includes>
    <includes id="phase1__func_8h" name="phase1_func.h" local="yes" imported="no">phase1_func.h</includes>
    <includes id="phase2__func_8h" name="phase2_func.h" local="yes" imported="no">phase2_func.h</includes>
    <includes id="phase2__reg_8h" name="phase2_reg.h" local="yes" imported="no">phase2_reg.h</includes>
    <includes id="phase3__unwind_8h" name="phase3_unwind.h" local="yes" imported="no">phase3_unwind.h</includes>
    <includes id="phase3__oper_8h" name="phase3_oper.h" local="yes" imported="no">phase3_oper.h</includes>
    <member kind="function">
      <type>void</type>
      <name>print_standard_completers</name>
      <anchorfile>ltahoe__completers_8c.html</anchorfile>
      <anchor>bce52039c4a4f39f8eb9567bcfcac27e</anchor>
      <arglist>(L_Oper *op)</arglist>
    </member>
    <member kind="function">
      <type>void</type>
      <name>print_branch_whether_completer</name>
      <anchorfile>ltahoe__completers_8c.html</anchorfile>
      <anchor>8335b15afd27bfc5eb943072ec10f511</anchor>
      <arglist>(L_Oper *op)</arglist>
    </member>
    <member kind="function">
      <type>void</type>
      <name>print_load_type_completer</name>
      <anchorfile>ltahoe__completers_8c.html</anchorfile>
      <anchor>6ba3559fde6c0253cafb306a7837573c</anchor>
      <arglist>(L_Oper *op)</arglist>
    </member>
    <member kind="function">
      <type>void</type>
      <name>print_comparison_op_completer</name>
      <anchorfile>ltahoe__completers_8c.html</anchorfile>
      <anchor>0b8cacf271c0d7020ef6b85bd5196417</anchor>
      <arglist>(L_Oper *op)</arglist>
    </member>
    <member kind="function">
      <type>void</type>
      <name>print_comparison_type_completer</name>
      <anchorfile>ltahoe__completers_8c.html</anchorfile>
      <anchor>8a70823b4da9d90ce12a4266ee887996</anchor>
      <arglist>(L_Oper *op)</arglist>
    </member>
    <member kind="function">
      <type>void</type>
      <name>print_branch_hint_completer</name>
      <anchorfile>ltahoe__completers_8c.html</anchorfile>
      <anchor>80262af86773a7a3ea0881c92616a954</anchor>
      <arglist>(L_Oper *op, int whichHint)</arglist>
    </member>
    <member kind="function">
      <type>void</type>
      <name>print_temporal_locality_completer</name>
      <anchorfile>ltahoe__completers_8c.html</anchorfile>
      <anchor>904b2066e38bfb10ce440a083fb4bffa</anchor>
      <arglist>(L_Oper *op)</arglist>
    </member>
    <member kind="function">
      <type>void</type>
      <name>print_LFETCH_completers</name>
      <anchorfile>ltahoe__completers_8c.html</anchorfile>
      <anchor>db88acfe5c6d306709170208a321e43e</anchor>
      <arglist>(L_Oper *op)</arglist>
    </member>
    <member kind="function">
      <type>void</type>
      <name>print_LD_C_completers</name>
      <anchorfile>ltahoe__completers_8c.html</anchorfile>
      <anchor>cb4c48107365895a5d0a1738fa6c5bf2</anchor>
      <arglist>(L_Oper *op)</arglist>
    </member>
    <member kind="function">
      <type>void</type>
      <name>print_LDF_C_completers</name>
      <anchorfile>ltahoe__completers_8c.html</anchorfile>
      <anchor>fabead62813d15a25c4176c841a6ca07</anchor>
      <arglist>(L_Oper *op)</arglist>
    </member>
    <member kind="function">
      <type>void</type>
      <name>print_CHK_A_completers</name>
      <anchorfile>ltahoe__completers_8c.html</anchorfile>
      <anchor>dc9d063fe5a8f7ec475be02bf972c4f6</anchor>
      <arglist>(L_Oper *op)</arglist>
    </member>
    <member kind="function">
      <type>void</type>
      <name>print_FP_S_REG_completers</name>
      <anchorfile>ltahoe__completers_8c.html</anchorfile>
      <anchor>1696f9b3980b2fdfae4073fefb9a11f3</anchor>
      <arglist>(L_Oper *op)</arglist>
    </member>
    <member kind="function">
      <type>void</type>
      <name>print_MOV_TOBR_completers</name>
      <anchorfile>ltahoe__completers_8c.html</anchorfile>
      <anchor>3471d18fb069b0a9a03e51ff149da164</anchor>
      <arglist>(L_Oper *op)</arglist>
    </member>
    <member kind="function">
      <type>void</type>
      <name>print_CMP_completers</name>
      <anchorfile>ltahoe__completers_8c.html</anchorfile>
      <anchor>85b2ce19a312089af0ed22b32f5891da</anchor>
      <arglist>(L_Oper *op)</arglist>
    </member>
    <member kind="function">
      <type>void</type>
      <name>print_BR_completers</name>
      <anchorfile>ltahoe__completers_8c.html</anchorfile>
      <anchor>2c5909c59db2447fa1d646f35487a98d</anchor>
      <arglist>(L_Oper *op)</arglist>
    </member>
    <member kind="function">
      <type>void</type>
      <name>print_BRP_completers</name>
      <anchorfile>ltahoe__completers_8c.html</anchorfile>
      <anchor>09abf698a975c2c75d4d23312f5faac4</anchor>
      <arglist>(L_Oper *op)</arglist>
    </member>
    <member kind="function">
      <type>void</type>
      <name>print_FCMP_completers</name>
      <anchorfile>ltahoe__completers_8c.html</anchorfile>
      <anchor>473d1020c16fe0c55033f0fe04a45358</anchor>
      <arglist>(L_Oper *op)</arglist>
    </member>
    <member kind="function">
      <type>void</type>
      <name>print_FCVT_completers</name>
      <anchorfile>ltahoe__completers_8c.html</anchorfile>
      <anchor>01137feb7e8f2ba9d12b787e6d228aaf</anchor>
      <arglist>(L_Oper *op)</arglist>
    </member>
    <member kind="function">
      <type>void</type>
      <name>print_FPCMP_completers</name>
      <anchorfile>ltahoe__completers_8c.html</anchorfile>
      <anchor>c460b12ea698e767326fb48ce9c9ad85</anchor>
      <arglist>(L_Oper *op)</arglist>
    </member>
    <member kind="function">
      <type>int</type>
      <name>check_branch_hints</name>
      <anchorfile>ltahoe__completers_8c.html</anchorfile>
      <anchor>47a1f403490b5a5df618efb0f12ce995</anchor>
      <arglist>(L_Oper *op)</arglist>
    </member>
    <member kind="function">
      <type>int</type>
      <name>check_LD</name>
      <anchorfile>ltahoe__completers_8c.html</anchorfile>
      <anchor>f40d2a75aee581cee59136adbd9797ba</anchor>
      <arglist>(L_Oper *op)</arglist>
    </member>
    <member kind="function">
      <type>int</type>
      <name>check_ACQ_REL_BIAS_FILL</name>
      <anchorfile>ltahoe__completers_8c.html</anchorfile>
      <anchor>cb1fe4bc10719361ab4b76a90a87a477</anchor>
      <arglist>(L_Oper *op)</arglist>
    </member>
    <member kind="function">
      <type>int</type>
      <name>check_LDF</name>
      <anchorfile>ltahoe__completers_8c.html</anchorfile>
      <anchor>c58ceac6ef3c287b9de88ae97d22482e</anchor>
      <arglist>(L_Oper *op)</arglist>
    </member>
    <member kind="function">
      <type>int</type>
      <name>check_LDF_A</name>
      <anchorfile>ltahoe__completers_8c.html</anchorfile>
      <anchor>789c62d266a91868cf73b0d3587517e6</anchor>
      <arglist>(L_Oper *op)</arglist>
    </member>
    <member kind="function">
      <type>int</type>
      <name>check_MOV_TOBR</name>
      <anchorfile>ltahoe__completers_8c.html</anchorfile>
      <anchor>6d311925807456ff21c798397eb3a738</anchor>
      <arglist>(L_Oper *op)</arglist>
    </member>
    <member kind="function">
      <type>int</type>
      <name>check_CMP</name>
      <anchorfile>ltahoe__completers_8c.html</anchorfile>
      <anchor>67691387e5797c11fc912c4f60352a32</anchor>
      <arglist>(L_Oper *op)</arglist>
    </member>
    <member kind="function">
      <type>int</type>
      <name>check_TBIT_TNAT</name>
      <anchorfile>ltahoe__completers_8c.html</anchorfile>
      <anchor>1a6738119396eff0084c5c499c695dbf</anchor>
      <arglist>(L_Oper *op)</arglist>
    </member>
    <member kind="function">
      <type>int</type>
      <name>check_BRP</name>
      <anchorfile>ltahoe__completers_8c.html</anchorfile>
      <anchor>d80f0f6b9d2e3d45c5039f3c9a887d64</anchor>
      <arglist>(L_Oper *op)</arglist>
    </member>
    <member kind="function">
      <type>int</type>
      <name>check_BRP_RET</name>
      <anchorfile>ltahoe__completers_8c.html</anchorfile>
      <anchor>af814d750bd4e7c663e74376535e3bda</anchor>
      <arglist>(L_Oper *op)</arglist>
    </member>
    <member kind="function">
      <type>int</type>
      <name>check_FCLASS</name>
      <anchorfile>ltahoe__completers_8c.html</anchorfile>
      <anchor>8c1e58ad1b10e873558e1199a520675d</anchor>
      <arglist>(L_Oper *op)</arglist>
    </member>
    <member kind="function">
      <type>int</type>
      <name>check_FCMP</name>
      <anchorfile>ltahoe__completers_8c.html</anchorfile>
      <anchor>7ae91c09d6f8e2d1a11c00596eb7870c</anchor>
      <arglist>(L_Oper *op)</arglist>
    </member>
    <member kind="function">
      <type>int</type>
      <name>check_FPCMP</name>
      <anchorfile>ltahoe__completers_8c.html</anchorfile>
      <anchor>8fadb584c8c9afe33a82937ba33c13c4</anchor>
      <arglist>(L_Oper *op)</arglist>
    </member>
    <member kind="function">
      <type>int</type>
      <name>check_ST</name>
      <anchorfile>ltahoe__completers_8c.html</anchorfile>
      <anchor>991f81c8b0fd66f1f416d59e341e98d1</anchor>
      <arglist>(L_Oper *op)</arglist>
    </member>
  </compound>
  <compound kind="file">
    <name>ltahoe_completers.h</name>
    <path>/mnt/hgfs/mkt/Desktop/openimpact/src/Lcode/codegen/Ltahoe/</path>
    <filename>ltahoe__completers_8h</filename>
    <member kind="define">
      <type>#define</type>
      <name>TC_TEMPORAL_LOCALITY_START</name>
      <anchorfile>ltahoe__completers_8h.html</anchorfile>
      <anchor>3a9f37a4169a26d2b10ce33d84e096b4</anchor>
      <arglist></arglist>
    </member>
    <member kind="define">
      <type>#define</type>
      <name>TC_TEMPORAL_LOCALITY_MASK</name>
      <anchorfile>ltahoe__completers_8h.html</anchorfile>
      <anchor>fb195f6dface3e7b4e6cb3f493ee0b8e</anchor>
      <arglist></arglist>
    </member>
    <member kind="define">
      <type>#define</type>
      <name>TC_TEMPORAL_LOCALITY_NONE</name>
      <anchorfile>ltahoe__completers_8h.html</anchorfile>
      <anchor>21c9c17ba7042ea19d0978099c9fbd28</anchor>
      <arglist></arglist>
    </member>
    <member kind="define">
      <type>#define</type>
      <name>TC_TEMPORAL_LOCALITY_NT1</name>
      <anchorfile>ltahoe__completers_8h.html</anchorfile>
      <anchor>45fcc7c19c04243378b0dc88049dcac5</anchor>
      <arglist></arglist>
    </member>
    <member kind="define">
      <type>#define</type>
      <name>TC_TEMPORAL_LOCALITY_NT2</name>
      <anchorfile>ltahoe__completers_8h.html</anchorfile>
      <anchor>bec75c71c51b100c8575d0ac6319e12c</anchor>
      <arglist></arglist>
    </member>
    <member kind="define">
      <type>#define</type>
      <name>TC_TEMPORAL_LOCALITY_NTA</name>
      <anchorfile>ltahoe__completers_8h.html</anchorfile>
      <anchor>b87375ecdf0487c943ed81a1067f8ac1</anchor>
      <arglist></arglist>
    </member>
    <member kind="define">
      <type>#define</type>
      <name>TC_BR_HNT1_START</name>
      <anchorfile>ltahoe__completers_8h.html</anchorfile>
      <anchor>3aaf7e977290276844df7edd56d75f41</anchor>
      <arglist></arglist>
    </member>
    <member kind="define">
      <type>#define</type>
      <name>TC_BR_HNT1_MASK</name>
      <anchorfile>ltahoe__completers_8h.html</anchorfile>
      <anchor>a06d2e4e0f6e290fd248ca573308cb97</anchor>
      <arglist></arglist>
    </member>
    <member kind="define">
      <type>#define</type>
      <name>TC_BR_HNT2_START</name>
      <anchorfile>ltahoe__completers_8h.html</anchorfile>
      <anchor>40c716eb8a8b20b045d62bc7e0039ad1</anchor>
      <arglist></arglist>
    </member>
    <member kind="define">
      <type>#define</type>
      <name>TC_BR_HNT2_MASK</name>
      <anchorfile>ltahoe__completers_8h.html</anchorfile>
      <anchor>3cd9d2b459fe2a00a36705044f6254de</anchor>
      <arglist></arglist>
    </member>
    <member kind="define">
      <type>#define</type>
      <name>TC_BR_HNT_NONE</name>
      <anchorfile>ltahoe__completers_8h.html</anchorfile>
      <anchor>d5f34494ad2430efa542ed6bed2efacd</anchor>
      <arglist></arglist>
    </member>
    <member kind="define">
      <type>#define</type>
      <name>TC_BR_HNT_TK</name>
      <anchorfile>ltahoe__completers_8h.html</anchorfile>
      <anchor>124c37405233949f4db38a6ac91c61a2</anchor>
      <arglist></arglist>
    </member>
    <member kind="define">
      <type>#define</type>
      <name>TC_BR_HNT_NT</name>
      <anchorfile>ltahoe__completers_8h.html</anchorfile>
      <anchor>192342ba02b661883732823e2de45e7c</anchor>
      <arglist></arglist>
    </member>
    <member kind="define">
      <type>#define</type>
      <name>TC_BR_HNT_DC</name>
      <anchorfile>ltahoe__completers_8h.html</anchorfile>
      <anchor>8cc6a8ab149a78c45d090b5a700b0cb7</anchor>
      <arglist></arglist>
    </member>
    <member kind="define">
      <type>#define</type>
      <name>TC_BR_WTHR_START</name>
      <anchorfile>ltahoe__completers_8h.html</anchorfile>
      <anchor>4022481d012e6585d63cb5c744e6f8f9</anchor>
      <arglist></arglist>
    </member>
    <member kind="define">
      <type>#define</type>
      <name>TC_BR_WTHR_MASK</name>
      <anchorfile>ltahoe__completers_8h.html</anchorfile>
      <anchor>39d1b0386871fd0b0c9d4a35be9d4501</anchor>
      <arglist></arglist>
    </member>
    <member kind="define">
      <type>#define</type>
      <name>TC_BR_WTHR_NONE</name>
      <anchorfile>ltahoe__completers_8h.html</anchorfile>
      <anchor>2619d9342091c9664262659ac23f7ce8</anchor>
      <arglist></arglist>
    </member>
    <member kind="define">
      <type>#define</type>
      <name>TC_BR_WTHR_DPTK</name>
      <anchorfile>ltahoe__completers_8h.html</anchorfile>
      <anchor>eec224f55de287807509530d27e008d8</anchor>
      <arglist></arglist>
    </member>
    <member kind="define">
      <type>#define</type>
      <name>TC_BR_WTHR_DPNT</name>
      <anchorfile>ltahoe__completers_8h.html</anchorfile>
      <anchor>daab72743686717c9272c022252ee2b6</anchor>
      <arglist></arglist>
    </member>
    <member kind="define">
      <type>#define</type>
      <name>TC_BR_WTHR_SPTK</name>
      <anchorfile>ltahoe__completers_8h.html</anchorfile>
      <anchor>03a40c116f057b60ba1b53e51e4d0f09</anchor>
      <arglist></arglist>
    </member>
    <member kind="define">
      <type>#define</type>
      <name>TC_BR_WTHR_SPNT</name>
      <anchorfile>ltahoe__completers_8h.html</anchorfile>
      <anchor>403dc0ca532d0c7ddb115eac43626cfd</anchor>
      <arglist></arglist>
    </member>
    <member kind="define">
      <type>#define</type>
      <name>TC_CMP_OP_START</name>
      <anchorfile>ltahoe__completers_8h.html</anchorfile>
      <anchor>fdb6ac03699c2dc52e4bd1e4af5bafe1</anchor>
      <arglist></arglist>
    </member>
    <member kind="define">
      <type>#define</type>
      <name>TC_CMP_OP_MASK</name>
      <anchorfile>ltahoe__completers_8h.html</anchorfile>
      <anchor>5805f64306b92073cad577ba9fd5dbc5</anchor>
      <arglist></arglist>
    </member>
    <member kind="define">
      <type>#define</type>
      <name>TC_CMP_OP_NONE</name>
      <anchorfile>ltahoe__completers_8h.html</anchorfile>
      <anchor>dfa7d82ee17d1c1afc45a02a3e867866</anchor>
      <arglist></arglist>
    </member>
    <member kind="define">
      <type>#define</type>
      <name>TC_CMP_OP_EQ</name>
      <anchorfile>ltahoe__completers_8h.html</anchorfile>
      <anchor>b3f12364a9f85fa1d87d41317c07bd96</anchor>
      <arglist></arglist>
    </member>
    <member kind="define">
      <type>#define</type>
      <name>TC_CMP_OP_NE</name>
      <anchorfile>ltahoe__completers_8h.html</anchorfile>
      <anchor>9493097295da6fa8d9cf95148968f28a</anchor>
      <arglist></arglist>
    </member>
    <member kind="define">
      <type>#define</type>
      <name>TC_CMP_OP_LT</name>
      <anchorfile>ltahoe__completers_8h.html</anchorfile>
      <anchor>d1c66c8296c66d30915c960276e72eab</anchor>
      <arglist></arglist>
    </member>
    <member kind="define">
      <type>#define</type>
      <name>TC_CMP_OP_LE</name>
      <anchorfile>ltahoe__completers_8h.html</anchorfile>
      <anchor>664562c22e21386a301d9f1cca292015</anchor>
      <arglist></arglist>
    </member>
    <member kind="define">
      <type>#define</type>
      <name>TC_CMP_OP_GT</name>
      <anchorfile>ltahoe__completers_8h.html</anchorfile>
      <anchor>3699098ba194b08b207ce9be875529b5</anchor>
      <arglist></arglist>
    </member>
    <member kind="define">
      <type>#define</type>
      <name>TC_CMP_OP_GE</name>
      <anchorfile>ltahoe__completers_8h.html</anchorfile>
      <anchor>ec4ef04342f588e36c9ab833197a7eb9</anchor>
      <arglist></arglist>
    </member>
    <member kind="define">
      <type>#define</type>
      <name>TC_CMP_OP_LTU</name>
      <anchorfile>ltahoe__completers_8h.html</anchorfile>
      <anchor>f77e350efb84dd48954b7b97f79cdac2</anchor>
      <arglist></arglist>
    </member>
    <member kind="define">
      <type>#define</type>
      <name>TC_CMP_OP_LEU</name>
      <anchorfile>ltahoe__completers_8h.html</anchorfile>
      <anchor>cfc2fb3dd96432626576fb7dd2069b36</anchor>
      <arglist></arglist>
    </member>
    <member kind="define">
      <type>#define</type>
      <name>TC_CMP_OP_GTU</name>
      <anchorfile>ltahoe__completers_8h.html</anchorfile>
      <anchor>308175945c966d8a11d87399873f76e0</anchor>
      <arglist></arglist>
    </member>
    <member kind="define">
      <type>#define</type>
      <name>TC_CMP_OP_GEU</name>
      <anchorfile>ltahoe__completers_8h.html</anchorfile>
      <anchor>42a54ed6867dfaf27e1669b8e9255e35</anchor>
      <arglist></arglist>
    </member>
    <member kind="define">
      <type>#define</type>
      <name>TC_CMP_OP_NEQ</name>
      <anchorfile>ltahoe__completers_8h.html</anchorfile>
      <anchor>d2591b611093385028dd21ce93c40a8e</anchor>
      <arglist></arglist>
    </member>
    <member kind="define">
      <type>#define</type>
      <name>TC_CMP_OP_NLT</name>
      <anchorfile>ltahoe__completers_8h.html</anchorfile>
      <anchor>a6f6c05201c4969e4409db649fb04b65</anchor>
      <arglist></arglist>
    </member>
    <member kind="define">
      <type>#define</type>
      <name>TC_CMP_OP_NLE</name>
      <anchorfile>ltahoe__completers_8h.html</anchorfile>
      <anchor>4b5d7c1e5b19e0d3c8cf69ae329a2289</anchor>
      <arglist></arglist>
    </member>
    <member kind="define">
      <type>#define</type>
      <name>TC_CMP_OP_NGT</name>
      <anchorfile>ltahoe__completers_8h.html</anchorfile>
      <anchor>ac767818b95b375e4d35753f8f524f96</anchor>
      <arglist></arglist>
    </member>
    <member kind="define">
      <type>#define</type>
      <name>TC_CMP_OP_NGE</name>
      <anchorfile>ltahoe__completers_8h.html</anchorfile>
      <anchor>ae9da9c0fd4a6c5ce533167909f52124</anchor>
      <arglist></arglist>
    </member>
    <member kind="define">
      <type>#define</type>
      <name>TC_CMP_OP_ORD</name>
      <anchorfile>ltahoe__completers_8h.html</anchorfile>
      <anchor>ff85b922364d67d21e5096093064db6c</anchor>
      <arglist></arglist>
    </member>
    <member kind="define">
      <type>#define</type>
      <name>TC_CMP_OP_UNORD</name>
      <anchorfile>ltahoe__completers_8h.html</anchorfile>
      <anchor>9ca9413f9243862fe10464e62aa0d955</anchor>
      <arglist></arglist>
    </member>
    <member kind="define">
      <type>#define</type>
      <name>TC_CMP_OP_Z</name>
      <anchorfile>ltahoe__completers_8h.html</anchorfile>
      <anchor>d718b1f210d55c8b8c4bafdf28a08a6d</anchor>
      <arglist></arglist>
    </member>
    <member kind="define">
      <type>#define</type>
      <name>TC_CMP_OP_NZ</name>
      <anchorfile>ltahoe__completers_8h.html</anchorfile>
      <anchor>1341d481dfce6cf852ab89831d378e6d</anchor>
      <arglist></arglist>
    </member>
    <member kind="define">
      <type>#define</type>
      <name>TC_CMP_OP_M</name>
      <anchorfile>ltahoe__completers_8h.html</anchorfile>
      <anchor>8f40f4fc36e4a6cf43991f1ed4f253a3</anchor>
      <arglist></arglist>
    </member>
    <member kind="define">
      <type>#define</type>
      <name>TC_CMP_OP_NM</name>
      <anchorfile>ltahoe__completers_8h.html</anchorfile>
      <anchor>fe8b7f72186189e9eda81b4acaf93d65</anchor>
      <arglist></arglist>
    </member>
    <member kind="define">
      <type>#define</type>
      <name>TC_CMP_TYPE_START</name>
      <anchorfile>ltahoe__completers_8h.html</anchorfile>
      <anchor>53b3798186c4fbbd3622c3d9aeca2558</anchor>
      <arglist></arglist>
    </member>
    <member kind="define">
      <type>#define</type>
      <name>TC_CMP_TYPE_MASK</name>
      <anchorfile>ltahoe__completers_8h.html</anchorfile>
      <anchor>fea6d6d153752a014015a91c1214f1da</anchor>
      <arglist></arglist>
    </member>
    <member kind="define">
      <type>#define</type>
      <name>TC_CMP_TYPE_NONE</name>
      <anchorfile>ltahoe__completers_8h.html</anchorfile>
      <anchor>2c214efeb19e59901996ccd82e66551c</anchor>
      <arglist></arglist>
    </member>
    <member kind="define">
      <type>#define</type>
      <name>TC_CMP_TYPE_UNC</name>
      <anchorfile>ltahoe__completers_8h.html</anchorfile>
      <anchor>9f62663b3265af3e0200ecce200439d6</anchor>
      <arglist></arglist>
    </member>
    <member kind="define">
      <type>#define</type>
      <name>TC_CMP_TYPE_OR</name>
      <anchorfile>ltahoe__completers_8h.html</anchorfile>
      <anchor>90ff5481b48694f43bc2b0c34a31e6de</anchor>
      <arglist></arglist>
    </member>
    <member kind="define">
      <type>#define</type>
      <name>TC_CMP_TYPE_AND</name>
      <anchorfile>ltahoe__completers_8h.html</anchorfile>
      <anchor>59b23586ed8aafeb5795fd0c0ee35a99</anchor>
      <arglist></arglist>
    </member>
    <member kind="define">
      <type>#define</type>
      <name>TC_CMP_TYPE_OR_ANDCM</name>
      <anchorfile>ltahoe__completers_8h.html</anchorfile>
      <anchor>a22373495a522cbe51ffd83d20ddf5c9</anchor>
      <arglist></arglist>
    </member>
    <member kind="define">
      <type>#define</type>
      <name>TC_CMP_TYPE_ORCM</name>
      <anchorfile>ltahoe__completers_8h.html</anchorfile>
      <anchor>92c0c0d0849999939d7a339719187b17</anchor>
      <arglist></arglist>
    </member>
    <member kind="define">
      <type>#define</type>
      <name>TC_CMP_TYPE_ANDCM</name>
      <anchorfile>ltahoe__completers_8h.html</anchorfile>
      <anchor>b3936bf26611bd360706399367ad3251</anchor>
      <arglist></arglist>
    </member>
    <member kind="define">
      <type>#define</type>
      <name>TC_CMP_TYPE_AND_ORCM</name>
      <anchorfile>ltahoe__completers_8h.html</anchorfile>
      <anchor>f044a3d31c61965d51026f3c5c82adcd</anchor>
      <arglist></arglist>
    </member>
    <member kind="define">
      <type>#define</type>
      <name>TC_LD_TYPE_START</name>
      <anchorfile>ltahoe__completers_8h.html</anchorfile>
      <anchor>b503b5ccb7340f80abe84575b449b860</anchor>
      <arglist></arglist>
    </member>
    <member kind="define">
      <type>#define</type>
      <name>TC_LD_TYPE_MASK</name>
      <anchorfile>ltahoe__completers_8h.html</anchorfile>
      <anchor>3be5a2f6f0c4bc1940ba3b538bc540f1</anchor>
      <arglist></arglist>
    </member>
    <member kind="define">
      <type>#define</type>
      <name>TC_LD_TYPE_NONE</name>
      <anchorfile>ltahoe__completers_8h.html</anchorfile>
      <anchor>04f99993a57839f2ed7cac4a513435a6</anchor>
      <arglist></arglist>
    </member>
    <member kind="define">
      <type>#define</type>
      <name>TC_LD_TYPE_A</name>
      <anchorfile>ltahoe__completers_8h.html</anchorfile>
      <anchor>416df89242e5bfacb10708590cd15487</anchor>
      <arglist></arglist>
    </member>
    <member kind="define">
      <type>#define</type>
      <name>TC_LD_TYPE_S</name>
      <anchorfile>ltahoe__completers_8h.html</anchorfile>
      <anchor>b021ff4c96bd0687d0284c643dc32fdd</anchor>
      <arglist></arglist>
    </member>
    <member kind="define">
      <type>#define</type>
      <name>TC_LD_TYPE_SA</name>
      <anchorfile>ltahoe__completers_8h.html</anchorfile>
      <anchor>fed08b2bcdd58823122ca24783953992</anchor>
      <arglist></arglist>
    </member>
    <member kind="define">
      <type>#define</type>
      <name>TC_SET_TEMPORAL_LOCALITY</name>
      <anchorfile>ltahoe__completers_8h.html</anchorfile>
      <anchor>19ab11519f98b188256d848123b43655</anchor>
      <arglist>(f, x)</arglist>
    </member>
    <member kind="define">
      <type>#define</type>
      <name>TC_GET_TEMPORAL_LOCALITY</name>
      <anchorfile>ltahoe__completers_8h.html</anchorfile>
      <anchor>b61b8d26e7fb4fa929dc16d86849687e</anchor>
      <arglist>(f)</arglist>
    </member>
    <member kind="define">
      <type>#define</type>
      <name>TC_SET_BR_HNT1</name>
      <anchorfile>ltahoe__completers_8h.html</anchorfile>
      <anchor>08ce68aa6bd580e43e0272954f55a7b0</anchor>
      <arglist>(f, x)</arglist>
    </member>
    <member kind="define">
      <type>#define</type>
      <name>TC_GET_BR_HNT1</name>
      <anchorfile>ltahoe__completers_8h.html</anchorfile>
      <anchor>edcb2e8dfec4fe86abe0da77e432f019</anchor>
      <arglist>(f)</arglist>
    </member>
    <member kind="define">
      <type>#define</type>
      <name>TC_SET_BR_HNT2</name>
      <anchorfile>ltahoe__completers_8h.html</anchorfile>
      <anchor>6054c89673faef55c936a4d68bbcf49c</anchor>
      <arglist>(f, x)</arglist>
    </member>
    <member kind="define">
      <type>#define</type>
      <name>TC_GET_BR_HNT2</name>
      <anchorfile>ltahoe__completers_8h.html</anchorfile>
      <anchor>76fd08ea3d54cf19ad143986b1af8ef3</anchor>
      <arglist>(f)</arglist>
    </member>
    <member kind="define">
      <type>#define</type>
      <name>TC_SET_BR_WTHR</name>
      <anchorfile>ltahoe__completers_8h.html</anchorfile>
      <anchor>1c8e719e747b99a9a313bcb3e267b426</anchor>
      <arglist>(f, x)</arglist>
    </member>
    <member kind="define">
      <type>#define</type>
      <name>TC_GET_BR_WTHR</name>
      <anchorfile>ltahoe__completers_8h.html</anchorfile>
      <anchor>ff01ce74ed658c07987e44031da6038e</anchor>
      <arglist>(f)</arglist>
    </member>
    <member kind="define">
      <type>#define</type>
      <name>TC_SET_CMP_OP</name>
      <anchorfile>ltahoe__completers_8h.html</anchorfile>
      <anchor>54b8209efcb04deb3b2a08ff7839ecd1</anchor>
      <arglist>(f, x)</arglist>
    </member>
    <member kind="define">
      <type>#define</type>
      <name>TC_GET_CMP_OP</name>
      <anchorfile>ltahoe__completers_8h.html</anchorfile>
      <anchor>3929967fa214f085a547dbd4123972b9</anchor>
      <arglist>(f)</arglist>
    </member>
    <member kind="define">
      <type>#define</type>
      <name>TC_SET_CMP_TYPE</name>
      <anchorfile>ltahoe__completers_8h.html</anchorfile>
      <anchor>357e2ff75b10cd2649c433112570a1c5</anchor>
      <arglist>(f, x)</arglist>
    </member>
    <member kind="define">
      <type>#define</type>
      <name>TC_GET_CMP_TYPE</name>
      <anchorfile>ltahoe__completers_8h.html</anchorfile>
      <anchor>6ed62026a977cbc6f242c16d7ded9109</anchor>
      <arglist>(f)</arglist>
    </member>
    <member kind="define">
      <type>#define</type>
      <name>TC_SET_LD_TYPE</name>
      <anchorfile>ltahoe__completers_8h.html</anchorfile>
      <anchor>fb362405ee1454ad8f758459a12f71f3</anchor>
      <arglist>(f, x)</arglist>
    </member>
    <member kind="define">
      <type>#define</type>
      <name>TC_GET_LD_TYPE</name>
      <anchorfile>ltahoe__completers_8h.html</anchorfile>
      <anchor>543f8ce8b047aee7a745f9c1bb486a8a</anchor>
      <arglist>(f)</arglist>
    </member>
    <member kind="define">
      <type>#define</type>
      <name>TC_LFETCH_EXCL</name>
      <anchorfile>ltahoe__completers_8h.html</anchorfile>
      <anchor>7ccd2bba32e79a7e0cffe4d67f1cf5fd</anchor>
      <arglist></arglist>
    </member>
    <member kind="define">
      <type>#define</type>
      <name>TC_LD_C_NC</name>
      <anchorfile>ltahoe__completers_8h.html</anchorfile>
      <anchor>fb0eb9c1fae5395853de2f185261ec6b</anchor>
      <arglist></arglist>
    </member>
    <member kind="define">
      <type>#define</type>
      <name>TC_LD_C_CLR</name>
      <anchorfile>ltahoe__completers_8h.html</anchorfile>
      <anchor>7aa14b18747fb5da5b59a0a4bd5791eb</anchor>
      <arglist></arglist>
    </member>
    <member kind="define">
      <type>#define</type>
      <name>TC_LD_C_CLR_ACQ</name>
      <anchorfile>ltahoe__completers_8h.html</anchorfile>
      <anchor>8813ae88beb86a12d767fff5870aa968</anchor>
      <arglist></arglist>
    </member>
    <member kind="define">
      <type>#define</type>
      <name>TC_FP_STATUS_START</name>
      <anchorfile>ltahoe__completers_8h.html</anchorfile>
      <anchor>6493bb57aa0ad33734e3ff5bc734afdd</anchor>
      <arglist></arglist>
    </member>
    <member kind="define">
      <type>#define</type>
      <name>TC_FP_STATUS_MASK</name>
      <anchorfile>ltahoe__completers_8h.html</anchorfile>
      <anchor>d2dda8beb86841fc9ab498b4b4b0041e</anchor>
      <arglist></arglist>
    </member>
    <member kind="define">
      <type>#define</type>
      <name>TC_FP_STATUS_S0</name>
      <anchorfile>ltahoe__completers_8h.html</anchorfile>
      <anchor>157ee3ab46215c609afd67a05bd9173c</anchor>
      <arglist></arglist>
    </member>
    <member kind="define">
      <type>#define</type>
      <name>TC_FP_STATUS_S1</name>
      <anchorfile>ltahoe__completers_8h.html</anchorfile>
      <anchor>416943c6213a0c2e300301c7aba3b57e</anchor>
      <arglist></arglist>
    </member>
    <member kind="define">
      <type>#define</type>
      <name>TC_FP_STATUS_S2</name>
      <anchorfile>ltahoe__completers_8h.html</anchorfile>
      <anchor>292c8d41a86ad6e614471ef8830d39fc</anchor>
      <arglist></arglist>
    </member>
    <member kind="define">
      <type>#define</type>
      <name>TC_FP_STATUS_S3</name>
      <anchorfile>ltahoe__completers_8h.html</anchorfile>
      <anchor>30e7bb782085e4372f1c45cf39f293ff</anchor>
      <arglist></arglist>
    </member>
    <member kind="define">
      <type>#define</type>
      <name>TC_SET_FP_STATUS_REG</name>
      <anchorfile>ltahoe__completers_8h.html</anchorfile>
      <anchor>4bac282c70f58c4fcef85fd315604c2d</anchor>
      <arglist>(f, x)</arglist>
    </member>
    <member kind="define">
      <type>#define</type>
      <name>TC_GET_FP_STATUS_REG</name>
      <anchorfile>ltahoe__completers_8h.html</anchorfile>
      <anchor>b85aa1390ddf7d27cdebefa2a507651d</anchor>
      <arglist>(f)</arglist>
    </member>
    <member kind="define">
      <type>#define</type>
      <name>TC_FCVT_TRUNC</name>
      <anchorfile>ltahoe__completers_8h.html</anchorfile>
      <anchor>e85e2ef5e394dddc11cdecc3eed412c8</anchor>
      <arglist></arglist>
    </member>
    <member kind="define">
      <type>#define</type>
      <name>TC_MOV_TOBR_IMP</name>
      <anchorfile>ltahoe__completers_8h.html</anchorfile>
      <anchor>458916b909598863669978553de54ecb</anchor>
      <arglist></arglist>
    </member>
    <member kind="define">
      <type>#define</type>
      <name>TC_MOV_TOBR_RET</name>
      <anchorfile>ltahoe__completers_8h.html</anchorfile>
      <anchor>4b2fff992dedb80a9769ed2019f1865c</anchor>
      <arglist></arglist>
    </member>
    <member kind="define">
      <type>#define</type>
      <name>TC_MOV_TOBR_MANY</name>
      <anchorfile>ltahoe__completers_8h.html</anchorfile>
      <anchor>50b88f26331ec1d9a338cb14b2a5b313</anchor>
      <arglist></arglist>
    </member>
    <member kind="define">
      <type>#define</type>
      <name>TC_CMP_4</name>
      <anchorfile>ltahoe__completers_8h.html</anchorfile>
      <anchor>d21d5be27070eabfb3855d0904c45ced</anchor>
      <arglist></arglist>
    </member>
    <member kind="define">
      <type>#define</type>
      <name>TC_BR_NONE</name>
      <anchorfile>ltahoe__completers_8h.html</anchorfile>
      <anchor>295b06ea9fa2737f2304f4a0e824c69b</anchor>
      <arglist></arglist>
    </member>
    <member kind="define">
      <type>#define</type>
      <name>TC_BR_FEW</name>
      <anchorfile>ltahoe__completers_8h.html</anchorfile>
      <anchor>440d81e4562abad43cfcb9f5bccabf6b</anchor>
      <arglist></arglist>
    </member>
    <member kind="define">
      <type>#define</type>
      <name>TC_BR_MANY</name>
      <anchorfile>ltahoe__completers_8h.html</anchorfile>
      <anchor>9d930c6763bc6393c026bfa820d45173</anchor>
      <arglist></arglist>
    </member>
    <member kind="define">
      <type>#define</type>
      <name>TC_BR_FEW_MANY_MASK</name>
      <anchorfile>ltahoe__completers_8h.html</anchorfile>
      <anchor>83f03c206f52aecd4e5baf9ce7fec453</anchor>
      <arglist></arglist>
    </member>
    <member kind="define">
      <type>#define</type>
      <name>TC_BR_CLR</name>
      <anchorfile>ltahoe__completers_8h.html</anchorfile>
      <anchor>fad033a15cd4191ef05c1cc8be044f41</anchor>
      <arglist></arglist>
    </member>
    <member kind="define">
      <type>#define</type>
      <name>TC_BRP_LOOP</name>
      <anchorfile>ltahoe__completers_8h.html</anchorfile>
      <anchor>0c4ce87a628db292d435ddc3979008cf</anchor>
      <arglist></arglist>
    </member>
    <member kind="define">
      <type>#define</type>
      <name>TC_BRP_EXIT</name>
      <anchorfile>ltahoe__completers_8h.html</anchorfile>
      <anchor>9cef3f86267445078c1adfc083409211</anchor>
      <arglist></arglist>
    </member>
    <member kind="define">
      <type>#define</type>
      <name>TC_BRP_LOOP_EXIT_MASK</name>
      <anchorfile>ltahoe__completers_8h.html</anchorfile>
      <anchor>aa3a55694b99b51cc0840a89b82bb3c7</anchor>
      <arglist></arglist>
    </member>
    <member kind="define">
      <type>#define</type>
      <name>TC_BRP_MANY</name>
      <anchorfile>ltahoe__completers_8h.html</anchorfile>
      <anchor>1cef01375c8ea47d9f2e1fd2fa40f141</anchor>
      <arglist></arglist>
    </member>
    <member kind="define">
      <type>#define</type>
      <name>TC_BRP_IMP</name>
      <anchorfile>ltahoe__completers_8h.html</anchorfile>
      <anchor>f4835df95d9c3d8d154b1dda34b0b3ff</anchor>
      <arglist></arglist>
    </member>
    <member kind="define">
      <type>#define</type>
      <name>TC_BRP_MANY_IMP_MASK</name>
      <anchorfile>ltahoe__completers_8h.html</anchorfile>
      <anchor>94895797b26300a94999a59a603ae81b</anchor>
      <arglist></arglist>
    </member>
    <member kind="function">
      <type>void</type>
      <name>print_standard_completers</name>
      <anchorfile>ltahoe__completers_8h.html</anchorfile>
      <anchor>bce52039c4a4f39f8eb9567bcfcac27e</anchor>
      <arglist>(L_Oper *op)</arglist>
    </member>
    <member kind="function">
      <type>void</type>
      <name>print_branch_whether_completer</name>
      <anchorfile>ltahoe__completers_8h.html</anchorfile>
      <anchor>8335b15afd27bfc5eb943072ec10f511</anchor>
      <arglist>(L_Oper *op)</arglist>
    </member>
    <member kind="function">
      <type>void</type>
      <name>print_load_type_completer</name>
      <anchorfile>ltahoe__completers_8h.html</anchorfile>
      <anchor>6ba3559fde6c0253cafb306a7837573c</anchor>
      <arglist>(L_Oper *op)</arglist>
    </member>
    <member kind="function">
      <type>void</type>
      <name>print_comparison_op_completer</name>
      <anchorfile>ltahoe__completers_8h.html</anchorfile>
      <anchor>0b8cacf271c0d7020ef6b85bd5196417</anchor>
      <arglist>(L_Oper *op)</arglist>
    </member>
    <member kind="function">
      <type>void</type>
      <name>print_comparison_type_completer</name>
      <anchorfile>ltahoe__completers_8h.html</anchorfile>
      <anchor>8a70823b4da9d90ce12a4266ee887996</anchor>
      <arglist>(L_Oper *op)</arglist>
    </member>
    <member kind="function">
      <type>void</type>
      <name>print_branch_hint_completer</name>
      <anchorfile>ltahoe__completers_8h.html</anchorfile>
      <anchor>80262af86773a7a3ea0881c92616a954</anchor>
      <arglist>(L_Oper *op, int whichHint)</arglist>
    </member>
    <member kind="function">
      <type>void</type>
      <name>print_temporal_locality_completer</name>
      <anchorfile>ltahoe__completers_8h.html</anchorfile>
      <anchor>904b2066e38bfb10ce440a083fb4bffa</anchor>
      <arglist>(L_Oper *op)</arglist>
    </member>
    <member kind="function">
      <type>void</type>
      <name>print_LFETCH_completers</name>
      <anchorfile>ltahoe__completers_8h.html</anchorfile>
      <anchor>db88acfe5c6d306709170208a321e43e</anchor>
      <arglist>(L_Oper *op)</arglist>
    </member>
    <member kind="function">
      <type>void</type>
      <name>print_LD_C_completers</name>
      <anchorfile>ltahoe__completers_8h.html</anchorfile>
      <anchor>cb4c48107365895a5d0a1738fa6c5bf2</anchor>
      <arglist>(L_Oper *op)</arglist>
    </member>
    <member kind="function">
      <type>void</type>
      <name>print_LDF_C_completers</name>
      <anchorfile>ltahoe__completers_8h.html</anchorfile>
      <anchor>fabead62813d15a25c4176c841a6ca07</anchor>
      <arglist>(L_Oper *op)</arglist>
    </member>
    <member kind="function">
      <type>void</type>
      <name>print_CHK_A_completers</name>
      <anchorfile>ltahoe__completers_8h.html</anchorfile>
      <anchor>dc9d063fe5a8f7ec475be02bf972c4f6</anchor>
      <arglist>(L_Oper *op)</arglist>
    </member>
    <member kind="function">
      <type>void</type>
      <name>print_FP_S_REG_completers</name>
      <anchorfile>ltahoe__completers_8h.html</anchorfile>
      <anchor>1696f9b3980b2fdfae4073fefb9a11f3</anchor>
      <arglist>(L_Oper *op)</arglist>
    </member>
    <member kind="function">
      <type>void</type>
      <name>print_MOV_TOBR_completers</name>
      <anchorfile>ltahoe__completers_8h.html</anchorfile>
      <anchor>3471d18fb069b0a9a03e51ff149da164</anchor>
      <arglist>(L_Oper *op)</arglist>
    </member>
    <member kind="function">
      <type>void</type>
      <name>print_CMP_completers</name>
      <anchorfile>ltahoe__completers_8h.html</anchorfile>
      <anchor>85b2ce19a312089af0ed22b32f5891da</anchor>
      <arglist>(L_Oper *op)</arglist>
    </member>
    <member kind="function">
      <type>void</type>
      <name>print_BR_completers</name>
      <anchorfile>ltahoe__completers_8h.html</anchorfile>
      <anchor>2c5909c59db2447fa1d646f35487a98d</anchor>
      <arglist>(L_Oper *op)</arglist>
    </member>
    <member kind="function">
      <type>void</type>
      <name>print_BRP_completers</name>
      <anchorfile>ltahoe__completers_8h.html</anchorfile>
      <anchor>09abf698a975c2c75d4d23312f5faac4</anchor>
      <arglist>(L_Oper *op)</arglist>
    </member>
    <member kind="function">
      <type>void</type>
      <name>print_FCMP_completers</name>
      <anchorfile>ltahoe__completers_8h.html</anchorfile>
      <anchor>473d1020c16fe0c55033f0fe04a45358</anchor>
      <arglist>(L_Oper *op)</arglist>
    </member>
    <member kind="function">
      <type>void</type>
      <name>print_FCVT_completers</name>
      <anchorfile>ltahoe__completers_8h.html</anchorfile>
      <anchor>01137feb7e8f2ba9d12b787e6d228aaf</anchor>
      <arglist>(L_Oper *op)</arglist>
    </member>
    <member kind="function">
      <type>void</type>
      <name>print_FPCMP_completers</name>
      <anchorfile>ltahoe__completers_8h.html</anchorfile>
      <anchor>c460b12ea698e767326fb48ce9c9ad85</anchor>
      <arglist>(L_Oper *op)</arglist>
    </member>
    <member kind="function">
      <type>int</type>
      <name>check_branch_hints</name>
      <anchorfile>ltahoe__completers_8h.html</anchorfile>
      <anchor>47a1f403490b5a5df618efb0f12ce995</anchor>
      <arglist>(L_Oper *op)</arglist>
    </member>
    <member kind="function">
      <type>int</type>
      <name>check_LD</name>
      <anchorfile>ltahoe__completers_8h.html</anchorfile>
      <anchor>f40d2a75aee581cee59136adbd9797ba</anchor>
      <arglist>(L_Oper *op)</arglist>
    </member>
    <member kind="function">
      <type>int</type>
      <name>check_ACQ_REL_BIAS_FILL</name>
      <anchorfile>ltahoe__completers_8h.html</anchorfile>
      <anchor>cb1fe4bc10719361ab4b76a90a87a477</anchor>
      <arglist>(L_Oper *op)</arglist>
    </member>
    <member kind="function">
      <type>int</type>
      <name>check_LDF</name>
      <anchorfile>ltahoe__completers_8h.html</anchorfile>
      <anchor>c58ceac6ef3c287b9de88ae97d22482e</anchor>
      <arglist>(L_Oper *op)</arglist>
    </member>
    <member kind="function">
      <type>int</type>
      <name>check_LDF_A</name>
      <anchorfile>ltahoe__completers_8h.html</anchorfile>
      <anchor>789c62d266a91868cf73b0d3587517e6</anchor>
      <arglist>(L_Oper *op)</arglist>
    </member>
    <member kind="function">
      <type>int</type>
      <name>check_MOV_TOBR</name>
      <anchorfile>ltahoe__completers_8h.html</anchorfile>
      <anchor>6d311925807456ff21c798397eb3a738</anchor>
      <arglist>(L_Oper *op)</arglist>
    </member>
    <member kind="function">
      <type>int</type>
      <name>check_CMP</name>
      <anchorfile>ltahoe__completers_8h.html</anchorfile>
      <anchor>67691387e5797c11fc912c4f60352a32</anchor>
      <arglist>(L_Oper *op)</arglist>
    </member>
    <member kind="function">
      <type>int</type>
      <name>check_TBIT_TNAT</name>
      <anchorfile>ltahoe__completers_8h.html</anchorfile>
      <anchor>1a6738119396eff0084c5c499c695dbf</anchor>
      <arglist>(L_Oper *op)</arglist>
    </member>
    <member kind="function">
      <type>int</type>
      <name>check_BRP</name>
      <anchorfile>ltahoe__completers_8h.html</anchorfile>
      <anchor>d80f0f6b9d2e3d45c5039f3c9a887d64</anchor>
      <arglist>(L_Oper *op)</arglist>
    </member>
    <member kind="function">
      <type>int</type>
      <name>check_BRP_RET</name>
      <anchorfile>ltahoe__completers_8h.html</anchorfile>
      <anchor>af814d750bd4e7c663e74376535e3bda</anchor>
      <arglist>(L_Oper *op)</arglist>
    </member>
    <member kind="function">
      <type>int</type>
      <name>check_FCLASS</name>
      <anchorfile>ltahoe__completers_8h.html</anchorfile>
      <anchor>8c1e58ad1b10e873558e1199a520675d</anchor>
      <arglist>(L_Oper *op)</arglist>
    </member>
    <member kind="function">
      <type>int</type>
      <name>check_FCMP</name>
      <anchorfile>ltahoe__completers_8h.html</anchorfile>
      <anchor>7ae91c09d6f8e2d1a11c00596eb7870c</anchor>
      <arglist>(L_Oper *op)</arglist>
    </member>
    <member kind="function">
      <type>int</type>
      <name>check_FPCMP</name>
      <anchorfile>ltahoe__completers_8h.html</anchorfile>
      <anchor>8fadb584c8c9afe33a82937ba33c13c4</anchor>
      <arglist>(L_Oper *op)</arglist>
    </member>
    <member kind="function">
      <type>int</type>
      <name>check_ST</name>
      <anchorfile>ltahoe__completers_8h.html</anchorfile>
      <anchor>991f81c8b0fd66f1f416d59e341e98d1</anchor>
      <arglist>(L_Oper *op)</arglist>
    </member>
  </compound>
  <compound kind="file">
    <name>ltahoe_main.c</name>
    <path>/mnt/hgfs/mkt/Desktop/openimpact/src/Lcode/codegen/Ltahoe/</path>
    <filename>ltahoe__main_8c</filename>
    <includes id="ltahoe__main_8h" name="ltahoe_main.h" local="yes" imported="no">ltahoe_main.h</includes>
    <includes id="phase1__func_8h" name="phase1_func.h" local="yes" imported="no">phase1_func.h</includes>
    <includes id="phase2__func_8h" name="phase2_func.h" local="yes" imported="no">phase2_func.h</includes>
    <includes id="phase3_8h" name="phase3.h" local="yes" imported="no">phase3.h</includes>
    <member kind="function">
      <type>void</type>
      <name>Ltahoe_read_parm</name>
      <anchorfile>ltahoe__main_8c.html</anchorfile>
      <anchor>bc450c64103f3a6d22a7bae89c8f0432</anchor>
      <arglist>(Parm_Parse_Info *ppi)</arglist>
    </member>
    <member kind="function">
      <type>void</type>
      <name>Ltahoe_init_version</name>
      <anchorfile>ltahoe__main_8c.html</anchorfile>
      <anchor>37597cdc757df67da8ab06f7c3c48228</anchor>
      <arglist>(void)</arglist>
    </member>
    <member kind="function">
      <type>void</type>
      <name>LB_read_parm_lblock</name>
      <anchorfile>ltahoe__main_8c.html</anchorfile>
      <anchor>8023d2e2a2761bf40c56c5de8ff63a7e</anchor>
      <arglist>(Parm_Parse_Info *ppi)</arglist>
    </member>
    <member kind="function">
      <type>void</type>
      <name>Ltahoe_init</name>
      <anchorfile>ltahoe__main_8c.html</anchorfile>
      <anchor>40a5c5a62ead442c870b89499659557f</anchor>
      <arglist>(Parm_Macro_List *command_line_macro_list)</arglist>
    </member>
    <member kind="function">
      <type>void</type>
      <name>Ltahoe_init_phase</name>
      <anchorfile>ltahoe__main_8c.html</anchorfile>
      <anchor>bf04f83654ca81add3af699b507842a9</anchor>
      <arglist>(Parm_Macro_List *command_line_macro_list, int codegen_phase)</arglist>
    </member>
    <member kind="function">
      <type>void</type>
      <name>Ltahoe_cleanup</name>
      <anchorfile>ltahoe__main_8c.html</anchorfile>
      <anchor>26c947ecf767d4b7c93d69273d060938</anchor>
      <arglist>(void)</arglist>
    </member>
    <member kind="function">
      <type>void</type>
      <name>Ltahoe_debug</name>
      <anchorfile>ltahoe__main_8c.html</anchorfile>
      <anchor>f92897b9e3a495c09e5d1f41a9155ba4</anchor>
      <arglist>(char *fmt,...)</arglist>
    </member>
    <member kind="function">
      <type>void</type>
      <name>Ltahoe_process_func</name>
      <anchorfile>ltahoe__main_8c.html</anchorfile>
      <anchor>2c966c59f210489f82897e5ced75262c</anchor>
      <arglist>(L_Func *fn, Parm_Macro_List *command_line_macro_list)</arglist>
    </member>
    <member kind="function">
      <type>void</type>
      <name>Ltahoe_process_data</name>
      <anchorfile>ltahoe__main_8c.html</anchorfile>
      <anchor>5d86f3646f651fa5f1ca4bd9a52a072d</anchor>
      <arglist>(FILE *F_OUT, L_Data *data)</arglist>
    </member>
    <member kind="variable">
      <type>char *</type>
      <name>phase_message</name>
      <anchorfile>ltahoe__main_8c.html</anchorfile>
      <anchor>e7bb13d904d3cacf0d02d686b083ca47</anchor>
      <arglist>[8]</arglist>
    </member>
    <member kind="variable">
      <type>int</type>
      <name>Ltahoe_use_gp_rel_addressing</name>
      <anchorfile>ltahoe__main_8c.html</anchorfile>
      <anchor>0be4275ae96f91dc9ff765b0b9e1a46b</anchor>
      <arglist></arglist>
    </member>
    <member kind="variable">
      <type>int</type>
      <name>Ltahoe_prologue_merge</name>
      <anchorfile>ltahoe__main_8c.html</anchorfile>
      <anchor>1627ca379fd5c995eb550698644169c5</anchor>
      <arglist></arglist>
    </member>
    <member kind="variable">
      <type>int</type>
      <name>Ltahoe_input_param_subst</name>
      <anchorfile>ltahoe__main_8c.html</anchorfile>
      <anchor>a17f35594956c11f58ea668129467091</anchor>
      <arglist></arglist>
    </member>
    <member kind="variable">
      <type>int</type>
      <name>Ltahoe_do_lcode_peephole</name>
      <anchorfile>ltahoe__main_8c.html</anchorfile>
      <anchor>b09eb070161529709fab4a957f49b566</anchor>
      <arglist></arglist>
    </member>
    <member kind="variable">
      <type>int</type>
      <name>Ltahoe_postinc_ld</name>
      <anchorfile>ltahoe__main_8c.html</anchorfile>
      <anchor>667f165193538c8304f6ad5a7c3d43a1</anchor>
      <arglist></arglist>
    </member>
    <member kind="variable">
      <type>int</type>
      <name>Ltahoe_postinc_st</name>
      <anchorfile>ltahoe__main_8c.html</anchorfile>
      <anchor>1f8cead66a41e92e9d414430b1fee2a6</anchor>
      <arglist></arglist>
    </member>
    <member kind="variable">
      <type>int</type>
      <name>Ltahoe_print_opti_stats</name>
      <anchorfile>ltahoe__main_8c.html</anchorfile>
      <anchor>a82b2944dc13c7029283493bab519e31</anchor>
      <arglist></arglist>
    </member>
    <member kind="variable">
      <type>int</type>
      <name>Ltahoe_add_mov_ap</name>
      <anchorfile>ltahoe__main_8c.html</anchorfile>
      <anchor>1fb370764ed9f4684acc588f58fb5744</anchor>
      <arglist></arglist>
    </member>
    <member kind="variable">
      <type>int</type>
      <name>Ltahoe_do_lightweight_pred_opti</name>
      <anchorfile>ltahoe__main_8c.html</anchorfile>
      <anchor>686fc0d75fb4fdffa5e344684a81b8fd</anchor>
      <arglist></arglist>
    </member>
    <member kind="variable">
      <type>int</type>
      <name>Ltahoe_do_repeated_mopti</name>
      <anchorfile>ltahoe__main_8c.html</anchorfile>
      <anchor>28bd704c9c996fa7c3be55a9c30cbf50</anchor>
      <arglist></arglist>
    </member>
    <member kind="variable">
      <type>int</type>
      <name>Ltahoe_do_redux</name>
      <anchorfile>ltahoe__main_8c.html</anchorfile>
      <anchor>c697a256dfda36b340bad18136e4f602</anchor>
      <arglist></arglist>
    </member>
    <member kind="variable">
      <type>int</type>
      <name>Ltahoe_do_tbit_redux</name>
      <anchorfile>ltahoe__main_8c.html</anchorfile>
      <anchor>00ab73c4c006dd370e62825392e7b8e7</anchor>
      <arglist></arglist>
    </member>
    <member kind="variable">
      <type>int</type>
      <name>Ltahoe_do_extr_redux</name>
      <anchorfile>ltahoe__main_8c.html</anchorfile>
      <anchor>590f42ba7e9924d848eb742edeb0fbd4</anchor>
      <arglist></arglist>
    </member>
    <member kind="variable">
      <type>int</type>
      <name>Ltahoe_do_depo_redux</name>
      <anchorfile>ltahoe__main_8c.html</anchorfile>
      <anchor>7cf98ce1c2fc28a02f1422b8a488c27b</anchor>
      <arglist></arglist>
    </member>
    <member kind="variable">
      <type>int</type>
      <name>Ltahoe_do_ldf_redux</name>
      <anchorfile>ltahoe__main_8c.html</anchorfile>
      <anchor>9ec0ee9849f4eb2037a4f7c2b0ae50de</anchor>
      <arglist></arglist>
    </member>
    <member kind="variable">
      <type>int</type>
      <name>Ltahoe_do_sp_removal</name>
      <anchorfile>ltahoe__main_8c.html</anchorfile>
      <anchor>cec422d4c6e9d7abf837b14f99e94a99</anchor>
      <arglist></arglist>
    </member>
    <member kind="variable">
      <type>int</type>
      <name>Ltahoe_fp_ftz</name>
      <anchorfile>ltahoe__main_8c.html</anchorfile>
      <anchor>888ab4fd06f706f742ab2cfa47a6efd1</anchor>
      <arglist></arglist>
    </member>
    <member kind="variable">
      <type>int</type>
      <name>Ltahoe_bitopt</name>
      <anchorfile>ltahoe__main_8c.html</anchorfile>
      <anchor>793dbe675d137731768ec881d59909ac</anchor>
      <arglist></arglist>
    </member>
    <member kind="variable">
      <type>int</type>
      <name>Ltahoe_vulcan</name>
      <anchorfile>ltahoe__main_8c.html</anchorfile>
      <anchor>b0732e916d962843cc6854aaf3b72585</anchor>
      <arglist></arglist>
    </member>
    <member kind="variable">
      <type>int</type>
      <name>Ltahoe_correct_profile</name>
      <anchorfile>ltahoe__main_8c.html</anchorfile>
      <anchor>347951e43e0d471ace162812466429d2</anchor>
      <arglist></arglist>
    </member>
    <member kind="variable">
      <type>double</type>
      <name>Ltahoe_padding_threshold</name>
      <anchorfile>ltahoe__main_8c.html</anchorfile>
      <anchor>560592971eff99f749c37b9848cf7a06</anchor>
      <arglist></arglist>
    </member>
    <member kind="variable">
      <type>int</type>
      <name>Ltahoe_debug_stack_frame</name>
      <anchorfile>ltahoe__main_8c.html</anchorfile>
      <anchor>6ece5b85c0f7183f801312b04603bf89</anchor>
      <arglist></arglist>
    </member>
    <member kind="variable">
      <type>int</type>
      <name>Ltahoe_check_for_stop_bits</name>
      <anchorfile>ltahoe__main_8c.html</anchorfile>
      <anchor>3070ee4956c8b9d304add83413f2b236</anchor>
      <arglist></arglist>
    </member>
    <member kind="variable">
      <type>int</type>
      <name>Ltahoe_do_postreg_const_fold</name>
      <anchorfile>ltahoe__main_8c.html</anchorfile>
      <anchor>e962f8f9a79349ab1186244ae1dc8a8b</anchor>
      <arglist></arglist>
    </member>
    <member kind="variable">
      <type>int</type>
      <name>Ltahoe_clobber_unats</name>
      <anchorfile>ltahoe__main_8c.html</anchorfile>
      <anchor>65ebbd2df1e6a55a301ce401dc745eee</anchor>
      <arglist></arglist>
    </member>
    <member kind="variable">
      <type>int</type>
      <name>Ltahoe_insert_branch_hints</name>
      <anchorfile>ltahoe__main_8c.html</anchorfile>
      <anchor>7eaf9c8ac288050259cb19cb6ce50a9b</anchor>
      <arglist></arglist>
    </member>
    <member kind="variable">
      <type>int</type>
      <name>Ltahoe_print_hint_info</name>
      <anchorfile>ltahoe__main_8c.html</anchorfile>
      <anchor>73789f3d150d5887ba958d6f0a396b8f</anchor>
      <arglist></arglist>
    </member>
    <member kind="variable">
      <type>int</type>
      <name>Ltahoe_aggressive_hints</name>
      <anchorfile>ltahoe__main_8c.html</anchorfile>
      <anchor>8f75c82e7127fd05ee54cb601dbdcbed</anchor>
      <arglist></arglist>
    </member>
    <member kind="variable">
      <type>int</type>
      <name>Ltahoe_use_many_hint_on_all_branches</name>
      <anchorfile>ltahoe__main_8c.html</anchorfile>
      <anchor>87fcb411ffa1d2e0c3b6de111ba98f39</anchor>
      <arglist></arglist>
    </member>
    <member kind="variable">
      <type>int</type>
      <name>Ltahoe_use_many_hint_on_call</name>
      <anchorfile>ltahoe__main_8c.html</anchorfile>
      <anchor>9e2d9618ac2cf0fdab0a7c09e8973373</anchor>
      <arglist></arglist>
    </member>
    <member kind="variable">
      <type>int</type>
      <name>Ltahoe_use_many_hint_on_return</name>
      <anchorfile>ltahoe__main_8c.html</anchorfile>
      <anchor>b713cda7a8519f2291c444e7c5662691</anchor>
      <arglist></arglist>
    </member>
    <member kind="variable">
      <type>int</type>
      <name>Ltahoe_use_many_hint_on_brp</name>
      <anchorfile>ltahoe__main_8c.html</anchorfile>
      <anchor>f0eac149ef5bb1887a8d7f41cd41f819</anchor>
      <arglist></arglist>
    </member>
    <member kind="variable">
      <type>int</type>
      <name>Ltahoe_use_imp_hint_on_brp</name>
      <anchorfile>ltahoe__main_8c.html</anchorfile>
      <anchor>21e026e0eb0a5a0df54df871888ab91e</anchor>
      <arglist></arglist>
    </member>
    <member kind="variable">
      <type>int</type>
      <name>Ltahoe_dont_expand_for_hints</name>
      <anchorfile>ltahoe__main_8c.html</anchorfile>
      <anchor>162dd673bc4c1adf7df1386a4feaf074</anchor>
      <arglist></arglist>
    </member>
    <member kind="variable">
      <type>int</type>
      <name>Ltahoe_min_fe_cycles_for_prefetch_brp</name>
      <anchorfile>ltahoe__main_8c.html</anchorfile>
      <anchor>7e2257a010feb791639269ac3276de92</anchor>
      <arglist></arglist>
    </member>
    <member kind="variable">
      <type>int</type>
      <name>Ltahoe_use_counted_prefetch_hints</name>
      <anchorfile>ltahoe__main_8c.html</anchorfile>
      <anchor>36baa76561ca723ddd3326131f916635</anchor>
      <arglist></arglist>
    </member>
    <member kind="variable">
      <type>int</type>
      <name>Ltahoe_use_streaming_only</name>
      <anchorfile>ltahoe__main_8c.html</anchorfile>
      <anchor>1bcda5d73bd226e3ab63a420f948926a</anchor>
      <arglist></arglist>
    </member>
    <member kind="variable">
      <type>int</type>
      <name>Ltahoe_advanced_prefetch</name>
      <anchorfile>ltahoe__main_8c.html</anchorfile>
      <anchor>d60871903690b42ad0b94bf66500be92</anchor>
      <arglist></arglist>
    </member>
    <member kind="variable">
      <type>int</type>
      <name>Ltahoe_insert_with_full_coverage</name>
      <anchorfile>ltahoe__main_8c.html</anchorfile>
      <anchor>b6644f513d2986963dd9f7329ba1938c</anchor>
      <arglist></arglist>
    </member>
    <member kind="variable">
      <type>int</type>
      <name>Ltahoe_insert_with_retries</name>
      <anchorfile>ltahoe__main_8c.html</anchorfile>
      <anchor>bec619aa9fadedbe73ebdb5803fb3bdc</anchor>
      <arglist></arglist>
    </member>
    <member kind="variable">
      <type>int</type>
      <name>Ltahoe_mckinley_hints</name>
      <anchorfile>ltahoe__main_8c.html</anchorfile>
      <anchor>2934c33e5d30c28575360f5336922047</anchor>
      <arglist></arglist>
    </member>
    <member kind="variable">
      <type>double</type>
      <name>Ltahoe_dp_upper_prob</name>
      <anchorfile>ltahoe__main_8c.html</anchorfile>
      <anchor>4cc48d0f69cf60e3c327a96998f39159</anchor>
      <arglist></arglist>
    </member>
    <member kind="variable">
      <type>double</type>
      <name>Ltahoe_dp_lower_prob</name>
      <anchorfile>ltahoe__main_8c.html</anchorfile>
      <anchor>670a82d24bf8f372280f80dfe7354426</anchor>
      <arglist></arglist>
    </member>
    <member kind="variable">
      <type>int</type>
      <name>Ltahoe_force_recovery_execution</name>
      <anchorfile>ltahoe__main_8c.html</anchorfile>
      <anchor>924322d4e960794b33e3452854b42e88</anchor>
      <arglist></arglist>
    </member>
    <member kind="variable">
      <type>int</type>
      <name>Ltahoe_force_recovery_upper</name>
      <anchorfile>ltahoe__main_8c.html</anchorfile>
      <anchor>ac3d1ef1b0a2438e6bbff8e42392eabb</anchor>
      <arglist></arglist>
    </member>
    <member kind="variable">
      <type>int</type>
      <name>Ltahoe_force_recovery_lower</name>
      <anchorfile>ltahoe__main_8c.html</anchorfile>
      <anchor>63b211e3a518ec229d91903bb6df1bf1</anchor>
      <arglist></arglist>
    </member>
    <member kind="variable">
      <type>int</type>
      <name>Ltahoe_generate_unwind_directives</name>
      <anchorfile>ltahoe__main_8c.html</anchorfile>
      <anchor>9d5e08ba94523972bc641b6343adb7d7</anchor>
      <arglist></arglist>
    </member>
    <member kind="variable">
      <type>int</type>
      <name>Ltahoe_print_characteristics</name>
      <anchorfile>ltahoe__main_8c.html</anchorfile>
      <anchor>c94497c3ee9f6833307318eeb16c9d83</anchor>
      <arglist></arglist>
    </member>
    <member kind="variable">
      <type>int</type>
      <name>Ltahoe_print_live_registers</name>
      <anchorfile>ltahoe__main_8c.html</anchorfile>
      <anchor>f65fff7395276c5decedb88b1dcf5c18</anchor>
      <arglist></arglist>
    </member>
    <member kind="variable">
      <type>int</type>
      <name>Ltahoe_print_issue_time</name>
      <anchorfile>ltahoe__main_8c.html</anchorfile>
      <anchor>0ac39ba1a62fbaaa56a6aa657dfc8998</anchor>
      <arglist></arglist>
    </member>
    <member kind="variable">
      <type>int</type>
      <name>Ltahoe_print_latency</name>
      <anchorfile>ltahoe__main_8c.html</anchorfile>
      <anchor>57b5d9a12126f85b38ea6f968d040cef</anchor>
      <arglist></arglist>
    </member>
    <member kind="variable">
      <type>int</type>
      <name>Ltahoe_print_op_id</name>
      <anchorfile>ltahoe__main_8c.html</anchorfile>
      <anchor>a77445289d4d0baba88a4759ead6f22a</anchor>
      <arglist></arglist>
    </member>
    <member kind="variable">
      <type>int</type>
      <name>Ltahoe_print_offset</name>
      <anchorfile>ltahoe__main_8c.html</anchorfile>
      <anchor>f9d00407c35723fbf8e0074b451ea32b</anchor>
      <arglist></arglist>
    </member>
    <member kind="variable">
      <type>int</type>
      <name>Ltahoe_print_iteration</name>
      <anchorfile>ltahoe__main_8c.html</anchorfile>
      <anchor>322bc5b25f714ccd003d038c98f638fd</anchor>
      <arglist></arglist>
    </member>
    <member kind="variable">
      <type>int</type>
      <name>Ltahoe_print_real_regs</name>
      <anchorfile>ltahoe__main_8c.html</anchorfile>
      <anchor>c836bf203878cd0e638c72934f72eb37</anchor>
      <arglist></arglist>
    </member>
    <member kind="variable">
      <type>int</type>
      <name>Ltahoe_generate_map</name>
      <anchorfile>ltahoe__main_8c.html</anchorfile>
      <anchor>df125d119c11626835cf436abab4533f</anchor>
      <arglist></arglist>
    </member>
    <member kind="variable">
      <type>int</type>
      <name>Ltahoe_output_for_ias</name>
      <anchorfile>ltahoe__main_8c.html</anchorfile>
      <anchor>9a2a5aa136b8602ccf4d6333a19e7610</anchor>
      <arglist></arglist>
    </member>
    <member kind="variable">
      <type>int</type>
      <name>Ltahoe_print_cache_stats</name>
      <anchorfile>ltahoe__main_8c.html</anchorfile>
      <anchor>d4d778ce03d3b0ed44f9e3d7ad759176</anchor>
      <arglist></arglist>
    </member>
    <member kind="variable">
      <type>int</type>
      <name>Ltahoe_tag_loads</name>
      <anchorfile>ltahoe__main_8c.html</anchorfile>
      <anchor>75d5bc32aff632346ebcdac56a6ab731</anchor>
      <arglist></arglist>
    </member>
    <member kind="variable">
      <type>int</type>
      <name>Ltahoe_machine_opt_mask</name>
      <anchorfile>ltahoe__main_8c.html</anchorfile>
      <anchor>3746fd21386375a98650eae47e41e99b</anchor>
      <arglist></arglist>
    </member>
  </compound>
  <compound kind="file">
    <name>ltahoe_main.h</name>
    <path>/mnt/hgfs/mkt/Desktop/openimpact/src/Lcode/codegen/Ltahoe/</path>
    <filename>ltahoe__main_8h</filename>
    <member kind="define">
      <type>#define</type>
      <name>P_NONE</name>
      <anchorfile>ltahoe__main_8h.html</anchorfile>
      <anchor>85d9ffb730a902e2dc0e7a90621b2144</anchor>
      <arglist></arglist>
    </member>
    <member kind="define">
      <type>#define</type>
      <name>P_1</name>
      <anchorfile>ltahoe__main_8h.html</anchorfile>
      <anchor>f188598d0d6f186e4b5b0ec4b8da5be1</anchor>
      <arglist></arglist>
    </member>
    <member kind="define">
      <type>#define</type>
      <name>P_2</name>
      <anchorfile>ltahoe__main_8h.html</anchorfile>
      <anchor>bb9cce698fe6446abdf9d14a5e34dd55</anchor>
      <arglist></arglist>
    </member>
    <member kind="define">
      <type>#define</type>
      <name>P_3</name>
      <anchorfile>ltahoe__main_8h.html</anchorfile>
      <anchor>c4f0fb84ab0c3d0b9e9e2fb0bb29bc0c</anchor>
      <arglist></arglist>
    </member>
    <member kind="define">
      <type>#define</type>
      <name>P_1_2</name>
      <anchorfile>ltahoe__main_8h.html</anchorfile>
      <anchor>3a87e3d593d73ea2eb78006f0bd93b9e</anchor>
      <arglist></arglist>
    </member>
    <member kind="define">
      <type>#define</type>
      <name>P_ALL</name>
      <anchorfile>ltahoe__main_8h.html</anchorfile>
      <anchor>26b749d3a74e5845386a8fa5597579a7</anchor>
      <arglist></arglist>
    </member>
    <member kind="define">
      <type>#define</type>
      <name>USE_IMPLICIT_NOSCHED</name>
      <anchorfile>ltahoe__main_8h.html</anchorfile>
      <anchor>6a3b8b138a1eb0c5c527dd50261350a9</anchor>
      <arglist></arglist>
    </member>
    <member kind="define">
      <type>#define</type>
      <name>Ltahoe_new_reg</name>
      <anchorfile>ltahoe__main_8h.html</anchorfile>
      <anchor>664faafa4f4cdc05960d3404233b7d65</anchor>
      <arglist>(cty)</arglist>
    </member>
    <member kind="define">
      <type>#define</type>
      <name>Ltahoe_new_int_reg</name>
      <anchorfile>ltahoe__main_8h.html</anchorfile>
      <anchor>4004d4835203f902cdd9d44e602d1ab5</anchor>
      <arglist>()</arglist>
    </member>
    <member kind="define">
      <type>#define</type>
      <name>Ltahoe_new_pred_reg</name>
      <anchorfile>ltahoe__main_8h.html</anchorfile>
      <anchor>925a3fd28f4c3b75cce791501937832a</anchor>
      <arglist>(ptype)</arglist>
    </member>
    <member kind="define">
      <type>#define</type>
      <name>Ltahoe_copy_or_new</name>
      <anchorfile>ltahoe__main_8h.html</anchorfile>
      <anchor>192b1bcba59c279acaadfb18bf57084d</anchor>
      <arglist>(cop, opd)</arglist>
    </member>
    <member kind="define">
      <type>#define</type>
      <name>Ltahoe_true_pred</name>
      <anchorfile>ltahoe__main_8h.html</anchorfile>
      <anchor>50998bf6558e6fe82f3084b82a3d82b0</anchor>
      <arglist>(ptype)</arglist>
    </member>
    <member kind="define">
      <type>#define</type>
      <name>Ltahoe_IMAC</name>
      <anchorfile>ltahoe__main_8h.html</anchorfile>
      <anchor>fd15ef5c94490f5d515ac342a45c28bd</anchor>
      <arglist>(mac)</arglist>
    </member>
    <member kind="define">
      <type>#define</type>
      <name>Ltahoe_PMAC</name>
      <anchorfile>ltahoe__main_8h.html</anchorfile>
      <anchor>c47e09bd5b8e0aa94abd17595678904a</anchor>
      <arglist>(mac)</arglist>
    </member>
    <member kind="define">
      <type>#define</type>
      <name>Ltahoe_FMAC</name>
      <anchorfile>ltahoe__main_8h.html</anchorfile>
      <anchor>af0ee82d3779c54db8523070f6ec45e1</anchor>
      <arglist>(mac)</arglist>
    </member>
    <member kind="define">
      <type>#define</type>
      <name>Ltahoe_DMAC</name>
      <anchorfile>ltahoe__main_8h.html</anchorfile>
      <anchor>3085b3491911228d07b9e9373763cfb0</anchor>
      <arglist>(mac)</arglist>
    </member>
    <member kind="define">
      <type>#define</type>
      <name>Ltahoe_BMAC</name>
      <anchorfile>ltahoe__main_8h.html</anchorfile>
      <anchor>47cba0409cc34950a5b2dfea55534ba9</anchor>
      <arglist>(mac)</arglist>
    </member>
    <member kind="define">
      <type>#define</type>
      <name>CLEAR_FLAGS</name>
      <anchorfile>ltahoe__main_8h.html</anchorfile>
      <anchor>24a3e2e31944b53037de242feae9d8df</anchor>
      <arglist>(oper)</arglist>
    </member>
    <member kind="define">
      <type>#define</type>
      <name>L_get_local_space_size</name>
      <anchorfile>ltahoe__main_8h.html</anchorfile>
      <anchor>d94e8b1246130da606fe6eb8bd37fd2c</anchor>
      <arglist>(fn)</arglist>
    </member>
    <member kind="define">
      <type>#define</type>
      <name>LTD</name>
      <anchorfile>ltahoe__main_8h.html</anchorfile>
      <anchor>382d779c04d7c7c6aca82c02f0269645</anchor>
      <arglist></arglist>
    </member>
    <member kind="function">
      <type>void</type>
      <name>Ltahoe_init</name>
      <anchorfile>ltahoe__main_8h.html</anchorfile>
      <anchor>baa611a0d1e0554f6325fa73a01da092</anchor>
      <arglist>(Parm_Macro_List *)</arglist>
    </member>
    <member kind="function">
      <type>void</type>
      <name>Ltahoe_init_phase</name>
      <anchorfile>ltahoe__main_8h.html</anchorfile>
      <anchor>855cfdc2dc53e5965c6c0ab20d5b30a8</anchor>
      <arglist>(Parm_Macro_List *, int)</arglist>
    </member>
    <member kind="function">
      <type>void</type>
      <name>Ltahoe_cleanup</name>
      <anchorfile>ltahoe__main_8h.html</anchorfile>
      <anchor>26c947ecf767d4b7c93d69273d060938</anchor>
      <arglist>(void)</arglist>
    </member>
    <member kind="function">
      <type>void</type>
      <name>Ltahoe_read_parm</name>
      <anchorfile>ltahoe__main_8h.html</anchorfile>
      <anchor>75f6ca7c163346ad7f6f35fc858fc8e6</anchor>
      <arglist>(Parm_Parse_Info *)</arglist>
    </member>
    <member kind="function">
      <type>void</type>
      <name>Ltahoe_init_version</name>
      <anchorfile>ltahoe__main_8h.html</anchorfile>
      <anchor>37597cdc757df67da8ab06f7c3c48228</anchor>
      <arglist>(void)</arglist>
    </member>
    <member kind="function">
      <type>void</type>
      <name>Ltahoe_process_func</name>
      <anchorfile>ltahoe__main_8h.html</anchorfile>
      <anchor>a68bc5b703fb11e8695c5295c74d1877</anchor>
      <arglist>(L_Func *, Parm_Macro_List *)</arglist>
    </member>
    <member kind="function">
      <type>void</type>
      <name>Ltahoe_process_data</name>
      <anchorfile>ltahoe__main_8h.html</anchorfile>
      <anchor>a7241ac55f2055259571e8d296f74ee1</anchor>
      <arglist>(FILE *, L_Data *)</arglist>
    </member>
    <member kind="function">
      <type>void</type>
      <name>Ltahoe_debug</name>
      <anchorfile>ltahoe__main_8h.html</anchorfile>
      <anchor>3b9462468c8680888cd6b9852f0d25ed</anchor>
      <arglist>(char *,...)</arglist>
    </member>
    <member kind="variable">
      <type>int</type>
      <name>Ltahoe_use_gp_rel_addressing</name>
      <anchorfile>ltahoe__main_8h.html</anchorfile>
      <anchor>0be4275ae96f91dc9ff765b0b9e1a46b</anchor>
      <arglist></arglist>
    </member>
    <member kind="variable">
      <type>int</type>
      <name>Ltahoe_prologue_merge</name>
      <anchorfile>ltahoe__main_8h.html</anchorfile>
      <anchor>1627ca379fd5c995eb550698644169c5</anchor>
      <arglist></arglist>
    </member>
    <member kind="variable">
      <type>int</type>
      <name>Ltahoe_input_param_subst</name>
      <anchorfile>ltahoe__main_8h.html</anchorfile>
      <anchor>a17f35594956c11f58ea668129467091</anchor>
      <arglist></arglist>
    </member>
    <member kind="variable">
      <type>int</type>
      <name>Ltahoe_do_lcode_peephole</name>
      <anchorfile>ltahoe__main_8h.html</anchorfile>
      <anchor>b09eb070161529709fab4a957f49b566</anchor>
      <arglist></arglist>
    </member>
    <member kind="variable">
      <type>int</type>
      <name>Ltahoe_postinc_ld</name>
      <anchorfile>ltahoe__main_8h.html</anchorfile>
      <anchor>667f165193538c8304f6ad5a7c3d43a1</anchor>
      <arglist></arglist>
    </member>
    <member kind="variable">
      <type>int</type>
      <name>Ltahoe_postinc_st</name>
      <anchorfile>ltahoe__main_8h.html</anchorfile>
      <anchor>1f8cead66a41e92e9d414430b1fee2a6</anchor>
      <arglist></arglist>
    </member>
    <member kind="variable">
      <type>int</type>
      <name>Ltahoe_print_opti_stats</name>
      <anchorfile>ltahoe__main_8h.html</anchorfile>
      <anchor>a82b2944dc13c7029283493bab519e31</anchor>
      <arglist></arglist>
    </member>
    <member kind="variable">
      <type>int</type>
      <name>Ltahoe_add_mov_ap</name>
      <anchorfile>ltahoe__main_8h.html</anchorfile>
      <anchor>1fb370764ed9f4684acc588f58fb5744</anchor>
      <arglist></arglist>
    </member>
    <member kind="variable">
      <type>int</type>
      <name>Ltahoe_do_lightweight_pred_opti</name>
      <anchorfile>ltahoe__main_8h.html</anchorfile>
      <anchor>686fc0d75fb4fdffa5e344684a81b8fd</anchor>
      <arglist></arglist>
    </member>
    <member kind="variable">
      <type>int</type>
      <name>Ltahoe_do_repeated_mopti</name>
      <anchorfile>ltahoe__main_8h.html</anchorfile>
      <anchor>28bd704c9c996fa7c3be55a9c30cbf50</anchor>
      <arglist></arglist>
    </member>
    <member kind="variable">
      <type>int</type>
      <name>Ltahoe_do_redux</name>
      <anchorfile>ltahoe__main_8h.html</anchorfile>
      <anchor>c697a256dfda36b340bad18136e4f602</anchor>
      <arglist></arglist>
    </member>
    <member kind="variable">
      <type>int</type>
      <name>Ltahoe_do_tbit_redux</name>
      <anchorfile>ltahoe__main_8h.html</anchorfile>
      <anchor>00ab73c4c006dd370e62825392e7b8e7</anchor>
      <arglist></arglist>
    </member>
    <member kind="variable">
      <type>int</type>
      <name>Ltahoe_do_extr_redux</name>
      <anchorfile>ltahoe__main_8h.html</anchorfile>
      <anchor>590f42ba7e9924d848eb742edeb0fbd4</anchor>
      <arglist></arglist>
    </member>
    <member kind="variable">
      <type>int</type>
      <name>Ltahoe_do_depo_redux</name>
      <anchorfile>ltahoe__main_8h.html</anchorfile>
      <anchor>7cf98ce1c2fc28a02f1422b8a488c27b</anchor>
      <arglist></arglist>
    </member>
    <member kind="variable">
      <type>int</type>
      <name>Ltahoe_do_ldf_redux</name>
      <anchorfile>ltahoe__main_8h.html</anchorfile>
      <anchor>9ec0ee9849f4eb2037a4f7c2b0ae50de</anchor>
      <arglist></arglist>
    </member>
    <member kind="variable">
      <type>int</type>
      <name>Ltahoe_do_sp_removal</name>
      <anchorfile>ltahoe__main_8h.html</anchorfile>
      <anchor>cec422d4c6e9d7abf837b14f99e94a99</anchor>
      <arglist></arglist>
    </member>
    <member kind="variable">
      <type>int</type>
      <name>Ltahoe_fp_ftz</name>
      <anchorfile>ltahoe__main_8h.html</anchorfile>
      <anchor>888ab4fd06f706f742ab2cfa47a6efd1</anchor>
      <arglist></arglist>
    </member>
    <member kind="variable">
      <type>int</type>
      <name>Ltahoe_bitopt</name>
      <anchorfile>ltahoe__main_8h.html</anchorfile>
      <anchor>793dbe675d137731768ec881d59909ac</anchor>
      <arglist></arglist>
    </member>
    <member kind="variable">
      <type>int</type>
      <name>Ltahoe_vulcan</name>
      <anchorfile>ltahoe__main_8h.html</anchorfile>
      <anchor>b0732e916d962843cc6854aaf3b72585</anchor>
      <arglist></arglist>
    </member>
    <member kind="variable">
      <type>int</type>
      <name>Ltahoe_correct_profile</name>
      <anchorfile>ltahoe__main_8h.html</anchorfile>
      <anchor>347951e43e0d471ace162812466429d2</anchor>
      <arglist></arglist>
    </member>
    <member kind="variable">
      <type>double</type>
      <name>Ltahoe_padding_threshold</name>
      <anchorfile>ltahoe__main_8h.html</anchorfile>
      <anchor>560592971eff99f749c37b9848cf7a06</anchor>
      <arglist></arglist>
    </member>
    <member kind="variable">
      <type>int</type>
      <name>Ltahoe_debug_stack_frame</name>
      <anchorfile>ltahoe__main_8h.html</anchorfile>
      <anchor>6ece5b85c0f7183f801312b04603bf89</anchor>
      <arglist></arglist>
    </member>
    <member kind="variable">
      <type>int</type>
      <name>Ltahoe_check_for_stop_bits</name>
      <anchorfile>ltahoe__main_8h.html</anchorfile>
      <anchor>3070ee4956c8b9d304add83413f2b236</anchor>
      <arglist></arglist>
    </member>
    <member kind="variable">
      <type>int</type>
      <name>Ltahoe_do_postreg_const_fold</name>
      <anchorfile>ltahoe__main_8h.html</anchorfile>
      <anchor>e962f8f9a79349ab1186244ae1dc8a8b</anchor>
      <arglist></arglist>
    </member>
    <member kind="variable">
      <type>int</type>
      <name>Ltahoe_clobber_unats</name>
      <anchorfile>ltahoe__main_8h.html</anchorfile>
      <anchor>65ebbd2df1e6a55a301ce401dc745eee</anchor>
      <arglist></arglist>
    </member>
    <member kind="variable">
      <type>int</type>
      <name>Ltahoe_insert_branch_hints</name>
      <anchorfile>ltahoe__main_8h.html</anchorfile>
      <anchor>7eaf9c8ac288050259cb19cb6ce50a9b</anchor>
      <arglist></arglist>
    </member>
    <member kind="variable">
      <type>int</type>
      <name>Ltahoe_print_hint_info</name>
      <anchorfile>ltahoe__main_8h.html</anchorfile>
      <anchor>73789f3d150d5887ba958d6f0a396b8f</anchor>
      <arglist></arglist>
    </member>
    <member kind="variable">
      <type>int</type>
      <name>Ltahoe_aggressive_hints</name>
      <anchorfile>ltahoe__main_8h.html</anchorfile>
      <anchor>8f75c82e7127fd05ee54cb601dbdcbed</anchor>
      <arglist></arglist>
    </member>
    <member kind="variable">
      <type>int</type>
      <name>Ltahoe_use_many_hint_on_all_branches</name>
      <anchorfile>ltahoe__main_8h.html</anchorfile>
      <anchor>87fcb411ffa1d2e0c3b6de111ba98f39</anchor>
      <arglist></arglist>
    </member>
    <member kind="variable">
      <type>int</type>
      <name>Ltahoe_use_many_hint_on_call</name>
      <anchorfile>ltahoe__main_8h.html</anchorfile>
      <anchor>9e2d9618ac2cf0fdab0a7c09e8973373</anchor>
      <arglist></arglist>
    </member>
    <member kind="variable">
      <type>int</type>
      <name>Ltahoe_use_many_hint_on_return</name>
      <anchorfile>ltahoe__main_8h.html</anchorfile>
      <anchor>b713cda7a8519f2291c444e7c5662691</anchor>
      <arglist></arglist>
    </member>
    <member kind="variable">
      <type>int</type>
      <name>Ltahoe_use_many_hint_on_brp</name>
      <anchorfile>ltahoe__main_8h.html</anchorfile>
      <anchor>f0eac149ef5bb1887a8d7f41cd41f819</anchor>
      <arglist></arglist>
    </member>
    <member kind="variable">
      <type>int</type>
      <name>Ltahoe_use_imp_hint_on_brp</name>
      <anchorfile>ltahoe__main_8h.html</anchorfile>
      <anchor>21e026e0eb0a5a0df54df871888ab91e</anchor>
      <arglist></arglist>
    </member>
    <member kind="variable">
      <type>int</type>
      <name>Ltahoe_dont_expand_for_hints</name>
      <anchorfile>ltahoe__main_8h.html</anchorfile>
      <anchor>162dd673bc4c1adf7df1386a4feaf074</anchor>
      <arglist></arglist>
    </member>
    <member kind="variable">
      <type>int</type>
      <name>Ltahoe_min_fe_cycles_for_prefetch_brp</name>
      <anchorfile>ltahoe__main_8h.html</anchorfile>
      <anchor>7e2257a010feb791639269ac3276de92</anchor>
      <arglist></arglist>
    </member>
    <member kind="variable">
      <type>int</type>
      <name>Ltahoe_use_counted_prefetch_hints</name>
      <anchorfile>ltahoe__main_8h.html</anchorfile>
      <anchor>36baa76561ca723ddd3326131f916635</anchor>
      <arglist></arglist>
    </member>
    <member kind="variable">
      <type>int</type>
      <name>Ltahoe_use_streaming_only</name>
      <anchorfile>ltahoe__main_8h.html</anchorfile>
      <anchor>1bcda5d73bd226e3ab63a420f948926a</anchor>
      <arglist></arglist>
    </member>
    <member kind="variable">
      <type>int</type>
      <name>Ltahoe_advanced_prefetch</name>
      <anchorfile>ltahoe__main_8h.html</anchorfile>
      <anchor>d60871903690b42ad0b94bf66500be92</anchor>
      <arglist></arglist>
    </member>
    <member kind="variable">
      <type>int</type>
      <name>Ltahoe_insert_with_full_coverage</name>
      <anchorfile>ltahoe__main_8h.html</anchorfile>
      <anchor>b6644f513d2986963dd9f7329ba1938c</anchor>
      <arglist></arglist>
    </member>
    <member kind="variable">
      <type>int</type>
      <name>Ltahoe_insert_with_retries</name>
      <anchorfile>ltahoe__main_8h.html</anchorfile>
      <anchor>bec619aa9fadedbe73ebdb5803fb3bdc</anchor>
      <arglist></arglist>
    </member>
    <member kind="variable">
      <type>int</type>
      <name>Ltahoe_mckinley_hints</name>
      <anchorfile>ltahoe__main_8h.html</anchorfile>
      <anchor>2934c33e5d30c28575360f5336922047</anchor>
      <arglist></arglist>
    </member>
    <member kind="variable">
      <type>double</type>
      <name>Ltahoe_dp_upper_prob</name>
      <anchorfile>ltahoe__main_8h.html</anchorfile>
      <anchor>4cc48d0f69cf60e3c327a96998f39159</anchor>
      <arglist></arglist>
    </member>
    <member kind="variable">
      <type>double</type>
      <name>Ltahoe_dp_lower_prob</name>
      <anchorfile>ltahoe__main_8h.html</anchorfile>
      <anchor>670a82d24bf8f372280f80dfe7354426</anchor>
      <arglist></arglist>
    </member>
    <member kind="variable">
      <type>int</type>
      <name>Ltahoe_force_recovery_execution</name>
      <anchorfile>ltahoe__main_8h.html</anchorfile>
      <anchor>924322d4e960794b33e3452854b42e88</anchor>
      <arglist></arglist>
    </member>
    <member kind="variable">
      <type>int</type>
      <name>Ltahoe_force_recovery_upper</name>
      <anchorfile>ltahoe__main_8h.html</anchorfile>
      <anchor>ac3d1ef1b0a2438e6bbff8e42392eabb</anchor>
      <arglist></arglist>
    </member>
    <member kind="variable">
      <type>int</type>
      <name>Ltahoe_force_recovery_lower</name>
      <anchorfile>ltahoe__main_8h.html</anchorfile>
      <anchor>63b211e3a518ec229d91903bb6df1bf1</anchor>
      <arglist></arglist>
    </member>
    <member kind="variable">
      <type>int</type>
      <name>Ltahoe_generate_unwind_directives</name>
      <anchorfile>ltahoe__main_8h.html</anchorfile>
      <anchor>9d5e08ba94523972bc641b6343adb7d7</anchor>
      <arglist></arglist>
    </member>
    <member kind="variable">
      <type>int</type>
      <name>Ltahoe_print_characteristics</name>
      <anchorfile>ltahoe__main_8h.html</anchorfile>
      <anchor>c94497c3ee9f6833307318eeb16c9d83</anchor>
      <arglist></arglist>
    </member>
    <member kind="variable">
      <type>int</type>
      <name>Ltahoe_print_live_registers</name>
      <anchorfile>ltahoe__main_8h.html</anchorfile>
      <anchor>f65fff7395276c5decedb88b1dcf5c18</anchor>
      <arglist></arglist>
    </member>
    <member kind="variable">
      <type>int</type>
      <name>Ltahoe_print_issue_time</name>
      <anchorfile>ltahoe__main_8h.html</anchorfile>
      <anchor>0ac39ba1a62fbaaa56a6aa657dfc8998</anchor>
      <arglist></arglist>
    </member>
    <member kind="variable">
      <type>int</type>
      <name>Ltahoe_print_latency</name>
      <anchorfile>ltahoe__main_8h.html</anchorfile>
      <anchor>57b5d9a12126f85b38ea6f968d040cef</anchor>
      <arglist></arglist>
    </member>
    <member kind="variable">
      <type>int</type>
      <name>Ltahoe_print_op_id</name>
      <anchorfile>ltahoe__main_8h.html</anchorfile>
      <anchor>a77445289d4d0baba88a4759ead6f22a</anchor>
      <arglist></arglist>
    </member>
    <member kind="variable">
      <type>int</type>
      <name>Ltahoe_print_offset</name>
      <anchorfile>ltahoe__main_8h.html</anchorfile>
      <anchor>f9d00407c35723fbf8e0074b451ea32b</anchor>
      <arglist></arglist>
    </member>
    <member kind="variable">
      <type>int</type>
      <name>Ltahoe_print_iteration</name>
      <anchorfile>ltahoe__main_8h.html</anchorfile>
      <anchor>322bc5b25f714ccd003d038c98f638fd</anchor>
      <arglist></arglist>
    </member>
    <member kind="variable">
      <type>int</type>
      <name>Ltahoe_print_real_regs</name>
      <anchorfile>ltahoe__main_8h.html</anchorfile>
      <anchor>c836bf203878cd0e638c72934f72eb37</anchor>
      <arglist></arglist>
    </member>
    <member kind="variable">
      <type>int</type>
      <name>Ltahoe_generate_map</name>
      <anchorfile>ltahoe__main_8h.html</anchorfile>
      <anchor>df125d119c11626835cf436abab4533f</anchor>
      <arglist></arglist>
    </member>
    <member kind="variable">
      <type>int</type>
      <name>Ltahoe_output_for_ias</name>
      <anchorfile>ltahoe__main_8h.html</anchorfile>
      <anchor>9a2a5aa136b8602ccf4d6333a19e7610</anchor>
      <arglist></arglist>
    </member>
    <member kind="variable">
      <type>int</type>
      <name>Ltahoe_print_cache_stats</name>
      <anchorfile>ltahoe__main_8h.html</anchorfile>
      <anchor>d4d778ce03d3b0ed44f9e3d7ad759176</anchor>
      <arglist></arglist>
    </member>
    <member kind="variable">
      <type>int</type>
      <name>Ltahoe_tag_loads</name>
      <anchorfile>ltahoe__main_8h.html</anchorfile>
      <anchor>75d5bc32aff632346ebcdac56a6ab731</anchor>
      <arglist></arglist>
    </member>
    <member kind="variable">
      <type>int</type>
      <name>Ltahoe_machine_opt_mask</name>
      <anchorfile>ltahoe__main_8h.html</anchorfile>
      <anchor>3746fd21386375a98650eae47e41e99b</anchor>
      <arglist></arglist>
    </member>
  </compound>
  <compound kind="file">
    <name>ltahoe_op_query.c</name>
    <path>/mnt/hgfs/mkt/Desktop/openimpact/src/Lcode/codegen/Ltahoe/</path>
    <filename>ltahoe__op__query_8c</filename>
    <includes id="ltahoe__op__query_8h" name="ltahoe_op_query.h" local="yes" imported="no">ltahoe_op_query.h</includes>
    <includes id="ltahoe__completers_8h" name="ltahoe_completers.h" local="yes" imported="no">ltahoe_completers.h</includes>
    <member kind="function">
      <type>int</type>
      <name>LT_is_input_param_operand</name>
      <anchorfile>ltahoe__op__query_8c.html</anchorfile>
      <anchor>b2c3a2dfc74df1937eb50c073974b699</anchor>
      <arglist>(L_Operand *operand)</arglist>
    </member>
    <member kind="function">
      <type>int</type>
      <name>LT_is_int_output_param_operand</name>
      <anchorfile>ltahoe__op__query_8c.html</anchorfile>
      <anchor>eee98d98e35f73bdc6d6e6d0671e84e0</anchor>
      <arglist>(L_Operand *operand)</arglist>
    </member>
    <member kind="function">
      <type>int</type>
      <name>LT_tahoe_cmp_proc_opc</name>
      <anchorfile>ltahoe__op__query_8c.html</anchorfile>
      <anchor>2124b70fc83ef1494f9a5a0c6cc7f371</anchor>
      <arglist>(ITuint8 ctype, ITuint8 com)</arglist>
    </member>
    <member kind="function">
      <type>int</type>
      <name>LT_tahoe_cmp_completer</name>
      <anchorfile>ltahoe__op__query_8c.html</anchorfile>
      <anchor>8d75709cfaba37cdfc31417e1dd36b59</anchor>
      <arglist>(ITuint8 ctype, ITuint8 com, int cmp_type)</arglist>
    </member>
    <member kind="function">
      <type>L_Oper *</type>
      <name>LT_create_nop</name>
      <anchorfile>ltahoe__op__query_8c.html</anchorfile>
      <anchor>b0ac87c37542f6a98e90503cc964cbae</anchor>
      <arglist>(int tahoeop, int value)</arglist>
    </member>
    <member kind="function">
      <type>L_Oper *</type>
      <name>LT_create_template_op</name>
      <anchorfile>ltahoe__op__query_8c.html</anchorfile>
      <anchor>a9c300869c1ffb6aa593b3b9d5039af8</anchor>
      <arglist>(int template_type, int stop_bit_mask)</arglist>
    </member>
    <member kind="function">
      <type>int</type>
      <name>LT_is_float_output_param_operand</name>
      <anchorfile>ltahoe__op__query_8c.html</anchorfile>
      <anchor>552a1005d96b3cc7710cb92be2f1bb9f</anchor>
      <arglist>(L_Operand *operand)</arglist>
    </member>
    <member kind="variable">
      <type>int</type>
      <name>LT_SYLLABLE_TYPE_TABLE</name>
      <anchorfile>ltahoe__op__query_8c.html</anchorfile>
      <anchor>b89be6897ddf2a1a2fae26253830f2cb</anchor>
      <arglist>[NUMBER_OF_REAL_TEMPLATES][3]</arglist>
    </member>
    <member kind="variable">
      <type>char *</type>
      <name>LT_TEMPLATE_NAME</name>
      <anchorfile>ltahoe__op__query_8c.html</anchorfile>
      <anchor>dedba46869fb4f643f8cb2f65db4cafe</anchor>
      <arglist>[NUMBER_OF_REAL_TEMPLATES]</arglist>
    </member>
  </compound>
  <compound kind="file">
    <name>ltahoe_op_query.h</name>
    <path>/mnt/hgfs/mkt/Desktop/openimpact/src/Lcode/codegen/Ltahoe/</path>
    <filename>ltahoe__op__query_8h</filename>
    <member kind="define">
      <type>#define</type>
      <name>INT_2EXP</name>
      <anchorfile>ltahoe__op__query_8h.html</anchorfile>
      <anchor>a385fca6a42eb5507bff8f97ec1ea484</anchor>
      <arglist>(i)</arglist>
    </member>
    <member kind="define">
      <type>#define</type>
      <name>UIMM_4</name>
      <anchorfile>ltahoe__op__query_8h.html</anchorfile>
      <anchor>e5cebd9818b987efabb6158c49f7b10d</anchor>
      <arglist>(a)</arglist>
    </member>
    <member kind="define">
      <type>#define</type>
      <name>UIMM_6</name>
      <anchorfile>ltahoe__op__query_8h.html</anchorfile>
      <anchor>6755218e5872d4605ebb31b005ae3229</anchor>
      <arglist>(a)</arglist>
    </member>
    <member kind="define">
      <type>#define</type>
      <name>SIMM_8</name>
      <anchorfile>ltahoe__op__query_8h.html</anchorfile>
      <anchor>fac6ee8ff56927ebdaab0ab8f59ef419</anchor>
      <arglist>(a)</arglist>
    </member>
    <member kind="define">
      <type>#define</type>
      <name>UIMM_8</name>
      <anchorfile>ltahoe__op__query_8h.html</anchorfile>
      <anchor>7be783169b03dcda7d497f345e173e6e</anchor>
      <arglist>(a)</arglist>
    </member>
    <member kind="define">
      <type>#define</type>
      <name>SIMM_9</name>
      <anchorfile>ltahoe__op__query_8h.html</anchorfile>
      <anchor>8100b9829769f4ff89cbd4773fd45599</anchor>
      <arglist>(a)</arglist>
    </member>
    <member kind="define">
      <type>#define</type>
      <name>SIMM_14</name>
      <anchorfile>ltahoe__op__query_8h.html</anchorfile>
      <anchor>6579806003d629e0ea0041dfb468dae6</anchor>
      <arglist>(a)</arglist>
    </member>
    <member kind="define">
      <type>#define</type>
      <name>SIMM_22</name>
      <anchorfile>ltahoe__op__query_8h.html</anchorfile>
      <anchor>1dbd8e1f1486ec95d88b4f4fcd7c3ed1</anchor>
      <arglist>(a)</arglist>
    </member>
    <member kind="define">
      <type>#define</type>
      <name>LT_is_R0_operand</name>
      <anchorfile>ltahoe__op__query_8h.html</anchorfile>
      <anchor>deac7b1dd67f18d7c149b3aba6c62cb1</anchor>
      <arglist>(op)</arglist>
    </member>
    <member kind="define">
      <type>#define</type>
      <name>LT_is_zero_value</name>
      <anchorfile>ltahoe__op__query_8h.html</anchorfile>
      <anchor>c3a547b0d6f9ab0626258733bf7537a1</anchor>
      <arglist>(op)</arglist>
    </member>
    <member kind="define">
      <type>#define</type>
      <name>LT_is_P0_operand</name>
      <anchorfile>ltahoe__op__query_8h.html</anchorfile>
      <anchor>dc4e3b5339fad06bc3e8a790defaa94a</anchor>
      <arglist>(op)</arglist>
    </member>
    <member kind="define">
      <type>#define</type>
      <name>LT_is_non_instr</name>
      <anchorfile>ltahoe__op__query_8h.html</anchorfile>
      <anchor>f8b572f431746ecf507138ffd8c09d6c</anchor>
      <arglist>(op)</arglist>
    </member>
    <member kind="define">
      <type>#define</type>
      <name>LT_is_cond_br</name>
      <anchorfile>ltahoe__op__query_8h.html</anchorfile>
      <anchor>f6f9985a8d54fbc9236bce716fb0a53f</anchor>
      <arglist>(op)</arglist>
    </member>
    <member kind="define">
      <type>#define</type>
      <name>LT_is_indir_br</name>
      <anchorfile>ltahoe__op__query_8h.html</anchorfile>
      <anchor>dc2e972084393288f2d1e6bf55dead1b</anchor>
      <arglist>(op)</arglist>
    </member>
    <member kind="define">
      <type>#define</type>
      <name>LT_is_call_br</name>
      <anchorfile>ltahoe__op__query_8h.html</anchorfile>
      <anchor>9c851c37b677f4f034681451d4231b6b</anchor>
      <arglist>(op)</arglist>
    </member>
    <member kind="define">
      <type>#define</type>
      <name>LT_is_ret_br</name>
      <anchorfile>ltahoe__op__query_8h.html</anchorfile>
      <anchor>8811903e8809e8e1898d5f266ad07dc3</anchor>
      <arglist>(op)</arglist>
    </member>
    <member kind="define">
      <type>#define</type>
      <name>LT_is_setf</name>
      <anchorfile>ltahoe__op__query_8h.html</anchorfile>
      <anchor>89709d2ff021bf42da125bd941a08200</anchor>
      <arglist>(op)</arglist>
    </member>
    <member kind="define">
      <type>#define</type>
      <name>LT_is_brp</name>
      <anchorfile>ltahoe__op__query_8h.html</anchorfile>
      <anchor>1f4ab4fc298db1d46385116afeed2e6b</anchor>
      <arglist>(op)</arglist>
    </member>
    <member kind="define">
      <type>#define</type>
      <name>LT_is_mov_to_br</name>
      <anchorfile>ltahoe__op__query_8h.html</anchorfile>
      <anchor>67da01be27732a2b98d4ae55a41a4698</anchor>
      <arglist>(op)</arglist>
    </member>
    <member kind="define">
      <type>#define</type>
      <name>LT_is_label_op</name>
      <anchorfile>ltahoe__op__query_8h.html</anchorfile>
      <anchor>10873c5e9c39052b47060300335500ba</anchor>
      <arglist>(op)</arglist>
    </member>
    <member kind="define">
      <type>#define</type>
      <name>LT_is_template_op</name>
      <anchorfile>ltahoe__op__query_8h.html</anchorfile>
      <anchor>e18f82795d62a838b50217e4145ad960</anchor>
      <arglist>(op)</arglist>
    </member>
    <member kind="define">
      <type>#define</type>
      <name>LT_is_cmp_op</name>
      <anchorfile>ltahoe__op__query_8h.html</anchorfile>
      <anchor>981d3a99847eea15fb09aff2ce65b1fd</anchor>
      <arglist>(op)</arglist>
    </member>
    <member kind="define">
      <type>#define</type>
      <name>LT_is_fill_op</name>
      <anchorfile>ltahoe__op__query_8h.html</anchorfile>
      <anchor>160bf05028c2c4aab5a32e043dc5c847</anchor>
      <arglist>(op)</arglist>
    </member>
    <member kind="define">
      <type>#define</type>
      <name>LT_get_template</name>
      <anchorfile>ltahoe__op__query_8h.html</anchorfile>
      <anchor>9685355488b63406f8e242113b0fafc8</anchor>
      <arglist>(tmpl_op)</arglist>
    </member>
    <member kind="define">
      <type>#define</type>
      <name>LT_new_template</name>
      <anchorfile>ltahoe__op__query_8h.html</anchorfile>
      <anchor>2e3c3272e405b9dc23cc37f4b51c3555</anchor>
      <arglist>(tmpl_op, tmpl_type)</arglist>
    </member>
    <member kind="define">
      <type>#define</type>
      <name>LT_set_template</name>
      <anchorfile>ltahoe__op__query_8h.html</anchorfile>
      <anchor>983dc0bd0d59173b1a92aaf7a8457095</anchor>
      <arglist>(tmpl_op, tmpl_type)</arglist>
    </member>
    <member kind="define">
      <type>#define</type>
      <name>LT_get_stop_bit_mask</name>
      <anchorfile>ltahoe__op__query_8h.html</anchorfile>
      <anchor>f1725e561e8ada44e1f201d19407df01</anchor>
      <arglist>(tmpl_op)</arglist>
    </member>
    <member kind="define">
      <type>#define</type>
      <name>LT_new_stop_bit_mask</name>
      <anchorfile>ltahoe__op__query_8h.html</anchorfile>
      <anchor>f3ea49ad94f739c17725db8b78f43aff</anchor>
      <arglist>(tmpl_op, stop_mask)</arglist>
    </member>
    <member kind="define">
      <type>#define</type>
      <name>LT_set_stop_bit_mask</name>
      <anchorfile>ltahoe__op__query_8h.html</anchorfile>
      <anchor>5cca92b8d7a10ac6428eb98abc95133a</anchor>
      <arglist>(tmpl_op, stop_mask)</arglist>
    </member>
    <member kind="define">
      <type>#define</type>
      <name>LT_get_density</name>
      <anchorfile>ltahoe__op__query_8h.html</anchorfile>
      <anchor>9b8a9962eac530a6f8411e6685043dca</anchor>
      <arglist>(tmpl_op)</arglist>
    </member>
    <member kind="define">
      <type>#define</type>
      <name>LT_new_density</name>
      <anchorfile>ltahoe__op__query_8h.html</anchorfile>
      <anchor>58ee8c1c5973e60e6d0f2b53f10619b4</anchor>
      <arglist>(tmpl_op, density)</arglist>
    </member>
    <member kind="define">
      <type>#define</type>
      <name>LT_set_density</name>
      <anchorfile>ltahoe__op__query_8h.html</anchorfile>
      <anchor>6ccbf77ddf92d9d04f4f84c4553fbae5</anchor>
      <arglist>(tmpl_op, density)</arglist>
    </member>
    <member kind="define">
      <type>#define</type>
      <name>S_AFTER_3RD</name>
      <anchorfile>ltahoe__op__query_8h.html</anchorfile>
      <anchor>94f32bfe3af7e75ca33331334aab6eba</anchor>
      <arglist></arglist>
    </member>
    <member kind="define">
      <type>#define</type>
      <name>S_AFTER_2ND</name>
      <anchorfile>ltahoe__op__query_8h.html</anchorfile>
      <anchor>6719d08f6a2cd1c9ef0e9aecd046aa2e</anchor>
      <arglist></arglist>
    </member>
    <member kind="define">
      <type>#define</type>
      <name>S_AFTER_1ST</name>
      <anchorfile>ltahoe__op__query_8h.html</anchorfile>
      <anchor>136bde2670bedc18f63994ca32c2b361</anchor>
      <arglist></arglist>
    </member>
    <member kind="define">
      <type>#define</type>
      <name>NO_S_BIT</name>
      <anchorfile>ltahoe__op__query_8h.html</anchorfile>
      <anchor>b0dc9fb4b6d67c08adab9b6525de6591</anchor>
      <arglist></arglist>
    </member>
    <member kind="define">
      <type>#define</type>
      <name>MII</name>
      <anchorfile>ltahoe__op__query_8h.html</anchorfile>
      <anchor>46809378752cbd5e11e537b31a59d3da</anchor>
      <arglist></arglist>
    </member>
    <member kind="define">
      <type>#define</type>
      <name>MISI</name>
      <anchorfile>ltahoe__op__query_8h.html</anchorfile>
      <anchor>17629e3901ed7a32ee2b57ce6ab9481e</anchor>
      <arglist></arglist>
    </member>
    <member kind="define">
      <type>#define</type>
      <name>MLI</name>
      <anchorfile>ltahoe__op__query_8h.html</anchorfile>
      <anchor>d92003e2c25b3d14d5c4f6f663afaaac</anchor>
      <arglist></arglist>
    </member>
    <member kind="define">
      <type>#define</type>
      <name>RSVD_T1</name>
      <anchorfile>ltahoe__op__query_8h.html</anchorfile>
      <anchor>ba0cd9fda7cf2579f6b9e7fdcb2b21f2</anchor>
      <arglist></arglist>
    </member>
    <member kind="define">
      <type>#define</type>
      <name>MMI</name>
      <anchorfile>ltahoe__op__query_8h.html</anchorfile>
      <anchor>7beefba50058a040c3aee9f9140b8621</anchor>
      <arglist></arglist>
    </member>
    <member kind="define">
      <type>#define</type>
      <name>MSMI</name>
      <anchorfile>ltahoe__op__query_8h.html</anchorfile>
      <anchor>cad5b250082c52a049bfbd44b16e956a</anchor>
      <arglist></arglist>
    </member>
    <member kind="define">
      <type>#define</type>
      <name>MFI</name>
      <anchorfile>ltahoe__op__query_8h.html</anchorfile>
      <anchor>049408aca49f3369b5d29cf99f6c40e6</anchor>
      <arglist></arglist>
    </member>
    <member kind="define">
      <type>#define</type>
      <name>MMF</name>
      <anchorfile>ltahoe__op__query_8h.html</anchorfile>
      <anchor>2ab77414482e02af7d393090be0b5c44</anchor>
      <arglist></arglist>
    </member>
    <member kind="define">
      <type>#define</type>
      <name>MIB</name>
      <anchorfile>ltahoe__op__query_8h.html</anchorfile>
      <anchor>9d11258d2beb9cc8297f7c4eff586868</anchor>
      <arglist></arglist>
    </member>
    <member kind="define">
      <type>#define</type>
      <name>MBB</name>
      <anchorfile>ltahoe__op__query_8h.html</anchorfile>
      <anchor>482713d1bbf911f6116016a8db72d920</anchor>
      <arglist></arglist>
    </member>
    <member kind="define">
      <type>#define</type>
      <name>RSVD_T3</name>
      <anchorfile>ltahoe__op__query_8h.html</anchorfile>
      <anchor>c6541dd43889258d26f31f0022bc1960</anchor>
      <arglist></arglist>
    </member>
    <member kind="define">
      <type>#define</type>
      <name>BBB</name>
      <anchorfile>ltahoe__op__query_8h.html</anchorfile>
      <anchor>56f71db70e5d3eb35f197bd1499987b1</anchor>
      <arglist></arglist>
    </member>
    <member kind="define">
      <type>#define</type>
      <name>MMB</name>
      <anchorfile>ltahoe__op__query_8h.html</anchorfile>
      <anchor>48e7c1e96778d2b311ee4521ae9ad3f9</anchor>
      <arglist></arglist>
    </member>
    <member kind="define">
      <type>#define</type>
      <name>RSVD_T4</name>
      <anchorfile>ltahoe__op__query_8h.html</anchorfile>
      <anchor>f30e7ecef96cbd8dc3da60bf6b01618b</anchor>
      <arglist></arglist>
    </member>
    <member kind="define">
      <type>#define</type>
      <name>MFB</name>
      <anchorfile>ltahoe__op__query_8h.html</anchorfile>
      <anchor>809ec1d69f62594a702aa2376eee8a73</anchor>
      <arglist></arglist>
    </member>
    <member kind="define">
      <type>#define</type>
      <name>RSVD_T5</name>
      <anchorfile>ltahoe__op__query_8h.html</anchorfile>
      <anchor>8f63f02a74bd32ba4747b7deef82dfeb</anchor>
      <arglist></arglist>
    </member>
    <member kind="define">
      <type>#define</type>
      <name>M_SYLL</name>
      <anchorfile>ltahoe__op__query_8h.html</anchorfile>
      <anchor>11bf0b2cb1a4aa3690d0b5c10b0edd81</anchor>
      <arglist></arglist>
    </member>
    <member kind="define">
      <type>#define</type>
      <name>I_SYLL</name>
      <anchorfile>ltahoe__op__query_8h.html</anchorfile>
      <anchor>57850af198d93e5894bcdcd157c4d26d</anchor>
      <arglist></arglist>
    </member>
    <member kind="define">
      <type>#define</type>
      <name>F_SYLL</name>
      <anchorfile>ltahoe__op__query_8h.html</anchorfile>
      <anchor>04303032e8c1df3f1e20915af2117bbd</anchor>
      <arglist></arglist>
    </member>
    <member kind="define">
      <type>#define</type>
      <name>B_SYLL</name>
      <anchorfile>ltahoe__op__query_8h.html</anchorfile>
      <anchor>f197e078f9616fc0af21d43b0a354c73</anchor>
      <arglist></arglist>
    </member>
    <member kind="define">
      <type>#define</type>
      <name>L_SYLL</name>
      <anchorfile>ltahoe__op__query_8h.html</anchorfile>
      <anchor>4d538a90016c046b3ddd9354f003cfae</anchor>
      <arglist></arglist>
    </member>
    <member kind="define">
      <type>#define</type>
      <name>INVALID_SYLL</name>
      <anchorfile>ltahoe__op__query_8h.html</anchorfile>
      <anchor>e20b63200aaf41b1bac9b43df3f3f48b</anchor>
      <arglist></arglist>
    </member>
    <member kind="define">
      <type>#define</type>
      <name>NUMBER_OF_REAL_TEMPLATES</name>
      <anchorfile>ltahoe__op__query_8h.html</anchorfile>
      <anchor>021edf5ef4a2c448eefffcec86c8639e</anchor>
      <arglist></arglist>
    </member>
    <member kind="define">
      <type>#define</type>
      <name>LT_syllable_type</name>
      <anchorfile>ltahoe__op__query_8h.html</anchorfile>
      <anchor>4542f77cf5ce651628542f23391d8fe2</anchor>
      <arglist>(tmp, syl)</arglist>
    </member>
    <member kind="define">
      <type>#define</type>
      <name>LT_template_name</name>
      <anchorfile>ltahoe__op__query_8h.html</anchorfile>
      <anchor>eeb6b24d09fcbdfd01b8b4eafa311f4a</anchor>
      <arglist>(i)</arglist>
    </member>
    <member kind="function">
      <type>int</type>
      <name>LT_is_input_param_operand</name>
      <anchorfile>ltahoe__op__query_8h.html</anchorfile>
      <anchor>fe3f78977f4c59d0fc4e818032a704d6</anchor>
      <arglist>(L_Operand *)</arglist>
    </member>
    <member kind="function">
      <type>int</type>
      <name>LT_is_int_output_param_operand</name>
      <anchorfile>ltahoe__op__query_8h.html</anchorfile>
      <anchor>84c85748df34bce9af8c9ddb2e5e97de</anchor>
      <arglist>(L_Operand *)</arglist>
    </member>
    <member kind="function">
      <type>int</type>
      <name>LT_is_float_output_param_operand</name>
      <anchorfile>ltahoe__op__query_8h.html</anchorfile>
      <anchor>cb30ee2dd1b0e89a84dd2ba656d8749c</anchor>
      <arglist>(L_Operand *)</arglist>
    </member>
    <member kind="function">
      <type>int</type>
      <name>LT_tahoe_cmp_proc_opc</name>
      <anchorfile>ltahoe__op__query_8h.html</anchorfile>
      <anchor>15f34fa929c57a4335ac366623ec6812</anchor>
      <arglist>(ITuint8, ITuint8)</arglist>
    </member>
    <member kind="function">
      <type>int</type>
      <name>LT_tahoe_cmp_completer</name>
      <anchorfile>ltahoe__op__query_8h.html</anchorfile>
      <anchor>d13fdf2e7a164f5f59f215f7ed9eb163</anchor>
      <arglist>(ITuint8, ITuint8, int)</arglist>
    </member>
    <member kind="function">
      <type>L_Oper *</type>
      <name>LT_create_nop</name>
      <anchorfile>ltahoe__op__query_8h.html</anchorfile>
      <anchor>6291ed3b66a9e8de287951b47bd722b2</anchor>
      <arglist>(int, int)</arglist>
    </member>
    <member kind="function">
      <type>L_Oper *</type>
      <name>LT_create_template_op</name>
      <anchorfile>ltahoe__op__query_8h.html</anchorfile>
      <anchor>585fe980439f2d2b009123d133ba6433</anchor>
      <arglist>(int, int)</arglist>
    </member>
    <member kind="variable">
      <type>int</type>
      <name>LT_SYLLABLE_TYPE_TABLE</name>
      <anchorfile>ltahoe__op__query_8h.html</anchorfile>
      <anchor>b89be6897ddf2a1a2fae26253830f2cb</anchor>
      <arglist>[NUMBER_OF_REAL_TEMPLATES][3]</arglist>
    </member>
    <member kind="variable">
      <type>char *</type>
      <name>LT_TEMPLATE_NAME</name>
      <anchorfile>ltahoe__op__query_8h.html</anchorfile>
      <anchor>dedba46869fb4f643f8cb2f65db4cafe</anchor>
      <arglist>[NUMBER_OF_REAL_TEMPLATES]</arglist>
    </member>
  </compound>
  <compound kind="file">
    <name>ltahoe_redux.c</name>
    <path>/mnt/hgfs/mkt/Desktop/openimpact/src/Lcode/codegen/Ltahoe/</path>
    <filename>ltahoe__redux_8c</filename>
    <includes id="ltahoe__main_8h" name="ltahoe_main.h" local="yes" imported="no">ltahoe_main.h</includes>
    <class kind="struct">_MiaExtract</class>
    <member kind="define">
      <type>#define</type>
      <name>DEBUG_TBIT_REDU</name>
      <anchorfile>ltahoe__redux_8c.html</anchorfile>
      <anchor>87cf67b5d77f5c7c467e5ea6faff172b</anchor>
      <arglist></arglist>
    </member>
    <member kind="define">
      <type>#define</type>
      <name>DEBUG_EXTR_REDU</name>
      <anchorfile>ltahoe__redux_8c.html</anchorfile>
      <anchor>d95b5a4c1f0d4a18bb57a087d55f4ea3</anchor>
      <arglist></arglist>
    </member>
    <member kind="define">
      <type>#define</type>
      <name>DEBUG_SP_REMOVAL</name>
      <anchorfile>ltahoe__redux_8c.html</anchorfile>
      <anchor>b391b62f0769290c853ba286f45ce578</anchor>
      <arglist></arglist>
    </member>
    <member kind="define">
      <type>#define</type>
      <name>LTA_REDUX_MAX_ITER</name>
      <anchorfile>ltahoe__redux_8c.html</anchorfile>
      <anchor>7692de39f95dbb2ba0aa6af9fb20d9c6</anchor>
      <arglist></arglist>
    </member>
    <member kind="define">
      <type>#define</type>
      <name>db</name>
      <anchorfile>ltahoe__redux_8c.html</anchorfile>
      <anchor>6fed581face389bcfd4a32d6dbd921cd</anchor>
      <arglist>(x)</arglist>
    </member>
    <member kind="typedef">
      <type>_MiaExtract</type>
      <name>MiaExtract</name>
      <anchorfile>ltahoe__redux_8c.html</anchorfile>
      <anchor>84c9a5ad26487cf9f4423a2f94d2ea23</anchor>
      <arglist></arglist>
    </member>
    <member kind="function" static="yes">
      <type>static int</type>
      <name>Ltahoe_local_test_bit_reduction</name>
      <anchorfile>ltahoe__redux_8c.html</anchorfile>
      <anchor>793738fc7367b8db9bc048b9a7c98ed8</anchor>
      <arglist>(L_Cb *cb)</arglist>
    </member>
    <member kind="function" static="yes">
      <type>static int</type>
      <name>Ltahoe_local_extract_reduction</name>
      <anchorfile>ltahoe__redux_8c.html</anchorfile>
      <anchor>118b4082b757b9ddee968a2dea36dff5</anchor>
      <arglist>(L_Cb *cb)</arglist>
    </member>
    <member kind="function" static="yes">
      <type>static int</type>
      <name>Ltahoe_local_ldf_reduction</name>
      <anchorfile>ltahoe__redux_8c.html</anchorfile>
      <anchor>fcfa68d3921596fe0590d49238abd2a4</anchor>
      <arglist>(L_Cb *cb)</arglist>
    </member>
    <member kind="function" static="yes">
      <type>static int</type>
      <name>Ltahoe_local_deposit_reduction</name>
      <anchorfile>ltahoe__redux_8c.html</anchorfile>
      <anchor>92362ea91f22ee50ddf9b8e1e2d9effa</anchor>
      <arglist>(L_Cb *cb)</arglist>
    </member>
    <member kind="function">
      <type>int</type>
      <name>Ltahoe_reduce</name>
      <anchorfile>ltahoe__redux_8c.html</anchorfile>
      <anchor>5993359a0b507a86410dcf29fc12672a</anchor>
      <arglist>(L_Func *fn)</arglist>
    </member>
    <member kind="function">
      <type>void</type>
      <name>Ltahoe_sp_removal</name>
      <anchorfile>ltahoe__redux_8c.html</anchorfile>
      <anchor>e0353a5a2cfdbda767b64c195fc459b2</anchor>
      <arglist>(L_Func *fn)</arglist>
    </member>
    <member kind="function" static="yes">
      <type>static int</type>
      <name>pwr_n_minus_1</name>
      <anchorfile>ltahoe__redux_8c.html</anchorfile>
      <anchor>67bf1fa4a2636bb9c6a5670077c82a3a</anchor>
      <arglist>(ITintmax i)</arglist>
    </member>
    <member kind="function" static="yes">
      <type>static int</type>
      <name>Ltahoe_classify_extract</name>
      <anchorfile>ltahoe__redux_8c.html</anchorfile>
      <anchor>a07fe57652d42f3cf03a2cc1ea620419</anchor>
      <arglist>(L_Oper *op, MiaExtract *me)</arglist>
    </member>
    <member kind="function" static="yes">
      <type>static int</type>
      <name>Ltahoe_classify_deposit</name>
      <anchorfile>ltahoe__redux_8c.html</anchorfile>
      <anchor>90e5d8459846517ee0b7e36a86ff5889</anchor>
      <arglist>(L_Oper *op, MiaExtract *me)</arglist>
    </member>
  </compound>
  <compound kind="file">
    <name>ltahoe_table.c</name>
    <path>/mnt/hgfs/mkt/Desktop/openimpact/src/Lcode/codegen/Ltahoe/</path>
    <filename>ltahoe__table_8c</filename>
    <includes id="ltahoe__table_8h" name="ltahoe_table.h" local="yes" imported="no">./src/Lcode/codegen/Ltahoe/ltahoe_table.h</includes>
    <member kind="variable">
      <type>opcode_handler</type>
      <name>Ltahoe_table</name>
      <anchorfile>ltahoe__table_8c.html</anchorfile>
      <anchor>6083fc7d0960a0d4651f5d60b4d5c819</anchor>
      <arglist>[table_num_opcodes]</arglist>
    </member>
  </compound>
  <compound kind="file">
    <name>ltahoe_table.h</name>
    <path>/mnt/hgfs/mkt/Desktop/openimpact/src/Lcode/codegen/Ltahoe/</path>
    <filename>ltahoe__table_8h</filename>
    <includes id="ltahoe__completers_8h" name="ltahoe_completers.h" local="no" imported="no">ltahoe_completers.h</includes>
    <includes id="phase1__bitopt_8h" name="phase1_bitopt.h" local="no" imported="no">phase1_bitopt.h</includes>
    <includes id="phase3__oper_8h" name="phase3_oper.h" local="no" imported="no">phase3_oper.h</includes>
    <class kind="struct">_opcode_handler</class>
    <member kind="define">
      <type>#define</type>
      <name>table_num_opcodes</name>
      <anchorfile>ltahoe__table_8h.html</anchorfile>
      <anchor>9d620e05be40ce43b3d1d25372230127</anchor>
      <arglist></arglist>
    </member>
    <member kind="define">
      <type>#define</type>
      <name>table_offset</name>
      <anchorfile>ltahoe__table_8h.html</anchorfile>
      <anchor>c86bf204cb8d2307ab48e94996cdb8ed</anchor>
      <arglist></arglist>
    </member>
    <member kind="define">
      <type>#define</type>
      <name>LTAHOE_TABLE_ENTRY</name>
      <anchorfile>ltahoe__table_8h.html</anchorfile>
      <anchor>0c694dffae84dd51f7aaead857939a78</anchor>
      <arglist>(t, o)</arglist>
    </member>
    <member kind="typedef">
      <type>_opcode_handler</type>
      <name>opcode_handler</name>
      <anchorfile>ltahoe__table_8h.html</anchorfile>
      <anchor>6c4f4bd8beb192183e22b5a7f5cc247a</anchor>
      <arglist></arglist>
    </member>
    <member kind="variable">
      <type>opcode_handler</type>
      <name>Ltahoe_table</name>
      <anchorfile>ltahoe__table_8h.html</anchorfile>
      <anchor>6083fc7d0960a0d4651f5d60b4d5c819</anchor>
      <arglist>[table_num_opcodes]</arglist>
    </member>
  </compound>
  <compound kind="file">
    <name>phase1_bitopt.c</name>
    <path>/mnt/hgfs/mkt/Desktop/openimpact/src/Lcode/codegen/Ltahoe/</path>
    <filename>phase1__bitopt_8c</filename>
    <includes id="ltahoe__main_8h" name="ltahoe_main.h" local="yes" imported="no">ltahoe_main.h</includes>
    <includes id="ltahoe__op__query_8h" name="ltahoe_op_query.h" local="yes" imported="no">ltahoe_op_query.h</includes>
    <includes id="ltahoe__completers_8h" name="ltahoe_completers.h" local="yes" imported="no">ltahoe_completers.h</includes>
    <includes id="ltahoe__bitvec_8h" name="ltahoe_bitvec.h" local="yes" imported="no">ltahoe_bitvec.h</includes>
    <includes id="ltahoe__table_8h" name="ltahoe_table.h" local="yes" imported="no">ltahoe_table.h</includes>
    <includes id="phase1__bitopt_8h" name="phase1_bitopt.h" local="yes" imported="no">phase1_bitopt.h</includes>
    <member kind="define">
      <type>#define</type>
      <name>REG_BITS_USED_BY_CALLER</name>
      <anchorfile>phase1__bitopt_8c.html</anchorfile>
      <anchor>3a3aed925f822764bbe06b018deb5f36</anchor>
      <arglist></arglist>
    </member>
    <member kind="define">
      <type>#define</type>
      <name>REG_BITS_USED_BY_RETURN</name>
      <anchorfile>phase1__bitopt_8c.html</anchorfile>
      <anchor>dfd49d569cc89039ea0f364eb4a64278</anchor>
      <arglist></arglist>
    </member>
    <member kind="define">
      <type>#define</type>
      <name>REG_BITS_USED_BY_UNSAFE</name>
      <anchorfile>phase1__bitopt_8c.html</anchorfile>
      <anchor>da6c9d531e1e8c0427ce634b1fcb5482</anchor>
      <arglist></arglist>
    </member>
    <member kind="define">
      <type>#define</type>
      <name>IS_EFFECTIVE_INT_CONST</name>
      <anchorfile>phase1__bitopt_8c.html</anchorfile>
      <anchor>73d0f2bf1cb4368cab6f0b5dd3d58aa0</anchor>
      <arglist>(operand)</arglist>
    </member>
    <member kind="define">
      <type>#define</type>
      <name>INT_VALUE_OPERAND</name>
      <anchorfile>phase1__bitopt_8c.html</anchorfile>
      <anchor>22574ca181f6dbdd9d90c014674152b5</anchor>
      <arglist>(operand)</arglist>
    </member>
    <member kind="define">
      <type>#define</type>
      <name>ERR</name>
      <anchorfile>phase1__bitopt_8c.html</anchorfile>
      <anchor>735563036dced0b7d6cc98f97ea4978b</anchor>
      <arglist></arglist>
    </member>
    <member kind="define">
      <type>#define</type>
      <name>pbitGetSrc</name>
      <anchorfile>phase1__bitopt_8c.html</anchorfile>
      <anchor>2ad4906b35bb1980298af392cfe48d68</anchor>
      <arglist>(oper, i)</arglist>
    </member>
    <member kind="define">
      <type>#define</type>
      <name>pbitGetDest</name>
      <anchorfile>phase1__bitopt_8c.html</anchorfile>
      <anchor>2e078790397018b20c51ab76c4ed5466</anchor>
      <arglist>(oper, i)</arglist>
    </member>
    <member kind="function">
      <type>void</type>
      <name>L_optimize_bit_trace</name>
      <anchorfile>phase1__bitopt_8c.html</anchorfile>
      <anchor>3eb2331295e2f4b6467f5008b4a3bca9</anchor>
      <arglist>(L_Func *fn)</arglist>
    </member>
    <member kind="function">
      <type>void</type>
      <name>L_optimize_bit_trace_cb</name>
      <anchorfile>phase1__bitopt_8c.html</anchorfile>
      <anchor>a0695a40658334972633e09155ec73fd</anchor>
      <arglist>(L_Func *fn, L_Cb *cb)</arglist>
    </member>
    <member kind="function">
      <type>void</type>
      <name>Top_down_optimizations</name>
      <anchorfile>phase1__bitopt_8c.html</anchorfile>
      <anchor>d228410f092635c1c98b46adee90869f</anchor>
      <arglist>(L_Func *fn, L_Cb *cb)</arglist>
    </member>
    <member kind="function">
      <type>void</type>
      <name>ComputeKnownBitsFlow</name>
      <anchorfile>phase1__bitopt_8c.html</anchorfile>
      <anchor>e5228e202430c967929ab9dec953de72</anchor>
      <arglist>(L_Func *fn, L_Cb *cb, L_Oper *oper)</arglist>
    </member>
    <member kind="function">
      <type>void</type>
      <name>ComputeKnownBitsFlow_EXTR_U</name>
      <anchorfile>phase1__bitopt_8c.html</anchorfile>
      <anchor>31358ed05d5628c2b9423c5c57c1fdfc</anchor>
      <arglist>(L_Func *fn, L_Cb *cb, L_Oper *oper)</arglist>
    </member>
    <member kind="function">
      <type>void</type>
      <name>ComputeKnownBitsFlow_DEP_Z</name>
      <anchorfile>phase1__bitopt_8c.html</anchorfile>
      <anchor>d413ecf13b85c89b14212453c03365e2</anchor>
      <arglist>(L_Func *fn, L_Cb *cb, L_Oper *oper)</arglist>
    </member>
    <member kind="function">
      <type>void</type>
      <name>ComputeKnownBitsFlow_SHLADDP4</name>
      <anchorfile>phase1__bitopt_8c.html</anchorfile>
      <anchor>8b04f48ee007cad2466f01c47838b2be</anchor>
      <arglist>(L_Func *fn, L_Cb *cb, L_Oper *oper)</arglist>
    </member>
    <member kind="function">
      <type>void</type>
      <name>ComputeKnownBitsFlow_SXT1</name>
      <anchorfile>phase1__bitopt_8c.html</anchorfile>
      <anchor>c06b3e5c50bfbc517762b161dc749a88</anchor>
      <arglist>(L_Func *fn, L_Cb *cb, L_Oper *oper)</arglist>
    </member>
    <member kind="function">
      <type>void</type>
      <name>ComputeKnownBitsFlow_SXT2</name>
      <anchorfile>phase1__bitopt_8c.html</anchorfile>
      <anchor>2dd7f8bd100e06c7b94e61e4377f4768</anchor>
      <arglist>(L_Func *fn, L_Cb *cb, L_Oper *oper)</arglist>
    </member>
    <member kind="function">
      <type>void</type>
      <name>ComputeKnownBitsFlow_SXT4</name>
      <anchorfile>phase1__bitopt_8c.html</anchorfile>
      <anchor>3a8a12da0d6b86950dbb591fcc901a5d</anchor>
      <arglist>(L_Func *fn, L_Cb *cb, L_Oper *oper)</arglist>
    </member>
    <member kind="function">
      <type>void</type>
      <name>ComputeKnownBitsFlow_ZXT1</name>
      <anchorfile>phase1__bitopt_8c.html</anchorfile>
      <anchor>b7bf3c3f467f3038c067b5d9cf47909e</anchor>
      <arglist>(L_Func *fn, L_Cb *cb, L_Oper *oper)</arglist>
    </member>
    <member kind="function">
      <type>void</type>
      <name>ComputeKnownBitsFlow_ZXT2</name>
      <anchorfile>phase1__bitopt_8c.html</anchorfile>
      <anchor>c9aa22bb8f256c6d12371f57a0aae6cd</anchor>
      <arglist>(L_Func *fn, L_Cb *cb, L_Oper *oper)</arglist>
    </member>
    <member kind="function">
      <type>void</type>
      <name>ComputeKnownBitsFlow_ZXT4</name>
      <anchorfile>phase1__bitopt_8c.html</anchorfile>
      <anchor>1d36ab4d33fadb20521d4116bd4ccae8</anchor>
      <arglist>(L_Func *fn, L_Cb *cb, L_Oper *oper)</arglist>
    </member>
    <member kind="function">
      <type>void</type>
      <name>ComputeKnownBitsFlow_MOV</name>
      <anchorfile>phase1__bitopt_8c.html</anchorfile>
      <anchor>daf3789b337470b1b3313e84ea975bbf</anchor>
      <arglist>(L_Func *fn, L_Cb *cb, L_Oper *oper)</arglist>
    </member>
    <member kind="function">
      <type>void</type>
      <name>ComputeKnownBitsFlow_AND</name>
      <anchorfile>phase1__bitopt_8c.html</anchorfile>
      <anchor>584cb9f266b693519c932c1414712c70</anchor>
      <arglist>(L_Func *fn, L_Cb *cb, L_Oper *oper)</arglist>
    </member>
    <member kind="function">
      <type>void</type>
      <name>ComputeKnownBitsFlow_OR</name>
      <anchorfile>phase1__bitopt_8c.html</anchorfile>
      <anchor>9019299ae0db751560489c26b1e130ba</anchor>
      <arglist>(L_Func *fn, L_Cb *cb, L_Oper *oper)</arglist>
    </member>
    <member kind="function">
      <type>void</type>
      <name>ComputeKnownBitsFlow_ADDP4</name>
      <anchorfile>phase1__bitopt_8c.html</anchorfile>
      <anchor>de7f1b519159ab96ab00733dda6279ba</anchor>
      <arglist>(L_Func *fn, L_Cb *cb, L_Oper *oper)</arglist>
    </member>
    <member kind="function">
      <type>void</type>
      <name>ComputeKnownBitsFlow_LD1</name>
      <anchorfile>phase1__bitopt_8c.html</anchorfile>
      <anchor>2fcdd54bd865bb3cf992af2e3a600038</anchor>
      <arglist>(L_Func *fn, L_Cb *cb, L_Oper *oper)</arglist>
    </member>
    <member kind="function">
      <type>void</type>
      <name>ComputeKnownBitsFlow_LD2</name>
      <anchorfile>phase1__bitopt_8c.html</anchorfile>
      <anchor>a108b6f7d14d284a1b009d5548ecb1b7</anchor>
      <arglist>(L_Func *fn, L_Cb *cb, L_Oper *oper)</arglist>
    </member>
    <member kind="function">
      <type>void</type>
      <name>ComputeKnownBitsFlow_LD4</name>
      <anchorfile>phase1__bitopt_8c.html</anchorfile>
      <anchor>f151610f0665f82d0a1c836896d37a66</anchor>
      <arglist>(L_Func *fn, L_Cb *cb, L_Oper *oper)</arglist>
    </member>
    <member kind="function">
      <type>void</type>
      <name>ComputeKnownBitsFlow_LD8</name>
      <anchorfile>phase1__bitopt_8c.html</anchorfile>
      <anchor>33a470925434046fd6922067fe1c42fd</anchor>
      <arglist>(L_Func *fn, L_Cb *cb, L_Oper *oper)</arglist>
    </member>
    <member kind="function">
      <type>void</type>
      <name>ProcessOperUsingTD</name>
      <anchorfile>phase1__bitopt_8c.html</anchorfile>
      <anchor>5708f70825d7797ddcd76194716ea4f5</anchor>
      <arglist>(L_Func *fn, L_Cb *cb, L_Oper *oper)</arglist>
    </member>
    <member kind="function">
      <type>void</type>
      <name>ProcessOperUsingTD_SXT1</name>
      <anchorfile>phase1__bitopt_8c.html</anchorfile>
      <anchor>5e97845662ffb9500ebfb4096dfcbf9b</anchor>
      <arglist>(L_Func *fn, L_Cb *cb, L_Oper *oper)</arglist>
    </member>
    <member kind="function">
      <type>void</type>
      <name>ProcessOperUsingTD_SXT2</name>
      <anchorfile>phase1__bitopt_8c.html</anchorfile>
      <anchor>5e421f86f5040eb14cc9a44c04f46f18</anchor>
      <arglist>(L_Func *fn, L_Cb *cb, L_Oper *oper)</arglist>
    </member>
    <member kind="function">
      <type>void</type>
      <name>ProcessOperUsingTD_SXT4</name>
      <anchorfile>phase1__bitopt_8c.html</anchorfile>
      <anchor>0ee829fbae9b6479fa310f50ee82c528</anchor>
      <arglist>(L_Func *fn, L_Cb *cb, L_Oper *oper)</arglist>
    </member>
    <member kind="function">
      <type>void</type>
      <name>ProcessOperUsingTD_ZXT1</name>
      <anchorfile>phase1__bitopt_8c.html</anchorfile>
      <anchor>957d17ef71e4ec39d5cc339da4307694</anchor>
      <arglist>(L_Func *fn, L_Cb *cb, L_Oper *oper)</arglist>
    </member>
    <member kind="function">
      <type>void</type>
      <name>ProcessOperUsingTD_ZXT2</name>
      <anchorfile>phase1__bitopt_8c.html</anchorfile>
      <anchor>c301f1ed09765ac54e78772f8374b977</anchor>
      <arglist>(L_Func *fn, L_Cb *cb, L_Oper *oper)</arglist>
    </member>
    <member kind="function">
      <type>void</type>
      <name>ProcessOperUsingTD_ZXT4</name>
      <anchorfile>phase1__bitopt_8c.html</anchorfile>
      <anchor>03f8c8bf740115f29822b8db8bd6ecb6</anchor>
      <arglist>(L_Func *fn, L_Cb *cb, L_Oper *oper)</arglist>
    </member>
    <member kind="function">
      <type>void</type>
      <name>LookupTopDownBitFlow</name>
      <anchorfile>phase1__bitopt_8c.html</anchorfile>
      <anchor>b1e2e013a15d8636baf0ef6170ee08e7</anchor>
      <arglist>(L_Func *fn, L_Cb *cb, L_Oper *oper_in, int idxOperandSrc, LT_bit_vector *pSrcKnownZero, LT_bit_vector *pSrcKnownOne)</arglist>
    </member>
    <member kind="function">
      <type>void</type>
      <name>Bottom_up_optimizations</name>
      <anchorfile>phase1__bitopt_8c.html</anchorfile>
      <anchor>c5ac60034d9a1c86c94c066c9245e132</anchor>
      <arglist>(L_Func *fn, L_Cb *cb)</arglist>
    </member>
    <member kind="function">
      <type>void</type>
      <name>ComputeBitsUsedAndPropagate</name>
      <anchorfile>phase1__bitopt_8c.html</anchorfile>
      <anchor>9e4bc142cde1d4bdfb1fc5dcc62bbfae</anchor>
      <arglist>(L_Func *fn, L_Cb *cb, L_Oper *oper_in)</arglist>
    </member>
    <member kind="function">
      <type>void</type>
      <name>ComputeBitsUsedDest</name>
      <anchorfile>phase1__bitopt_8c.html</anchorfile>
      <anchor>cc7a0e32dd38dddd8e076f0c03a8a732</anchor>
      <arglist>(L_Func *fn, L_Cb *cb, L_Oper *oper_in, int idxOperandDest)</arglist>
    </member>
    <member kind="function">
      <type>void</type>
      <name>ComputeBitsUsedLocal</name>
      <anchorfile>phase1__bitopt_8c.html</anchorfile>
      <anchor>6f16a9508404780959292bfca5dcc486</anchor>
      <arglist>(L_Func *fn, L_Cb *cb, L_Oper *oper_in)</arglist>
    </member>
    <member kind="function">
      <type>void</type>
      <name>ComputeBitsUsedLocal_SXT1</name>
      <anchorfile>phase1__bitopt_8c.html</anchorfile>
      <anchor>a923390963f01e270884a74d05f9de38</anchor>
      <arglist>(L_Func *fn, L_Cb *cb, L_Oper *oper_in)</arglist>
    </member>
    <member kind="function">
      <type>void</type>
      <name>ComputeBitsUsedLocal_SXT2</name>
      <anchorfile>phase1__bitopt_8c.html</anchorfile>
      <anchor>afc4b485675d957f70e04057aa41466e</anchor>
      <arglist>(L_Func *fn, L_Cb *cb, L_Oper *oper_in)</arglist>
    </member>
    <member kind="function">
      <type>void</type>
      <name>ComputeBitsUsedLocal_SXT4</name>
      <anchorfile>phase1__bitopt_8c.html</anchorfile>
      <anchor>d3c5a9721094407e915e46cbc46f85e5</anchor>
      <arglist>(L_Func *fn, L_Cb *cb, L_Oper *oper_in)</arglist>
    </member>
    <member kind="function">
      <type>void</type>
      <name>ComputeBitsUsedLocal_ZXT1</name>
      <anchorfile>phase1__bitopt_8c.html</anchorfile>
      <anchor>edd6bb222589426f5aac814dad7ddb43</anchor>
      <arglist>(L_Func *fn, L_Cb *cb, L_Oper *oper_in)</arglist>
    </member>
    <member kind="function">
      <type>void</type>
      <name>ComputeBitsUsedLocal_ZXT2</name>
      <anchorfile>phase1__bitopt_8c.html</anchorfile>
      <anchor>bc9c8125293a54812c0dc6bba21894f8</anchor>
      <arglist>(L_Func *fn, L_Cb *cb, L_Oper *oper_in)</arglist>
    </member>
    <member kind="function">
      <type>void</type>
      <name>ComputeBitsUsedLocal_ZXT4</name>
      <anchorfile>phase1__bitopt_8c.html</anchorfile>
      <anchor>474a8803a3cf66954ff2c790887a600e</anchor>
      <arglist>(L_Func *fn, L_Cb *cb, L_Oper *oper_in)</arglist>
    </member>
    <member kind="function">
      <type>void</type>
      <name>ComputeBitsUsedLocal_MOV</name>
      <anchorfile>phase1__bitopt_8c.html</anchorfile>
      <anchor>da486342be0e0a76dccf63996d408d71</anchor>
      <arglist>(L_Func *fn, L_Cb *cb, L_Oper *oper_in)</arglist>
    </member>
    <member kind="function">
      <type>void</type>
      <name>ComputeBitsUsedLocal_ARITH_OP</name>
      <anchorfile>phase1__bitopt_8c.html</anchorfile>
      <anchor>28c543f9abb2bd2435162d5c59a402ae</anchor>
      <arglist>(L_Func *fn, L_Cb *cb, L_Oper *oper_in)</arglist>
    </member>
    <member kind="function">
      <type>void</type>
      <name>ComputeBitsUsedLocal_AND</name>
      <anchorfile>phase1__bitopt_8c.html</anchorfile>
      <anchor>f165ffc607d16aa1f84cf0a0e0be0ed7</anchor>
      <arglist>(L_Func *fn, L_Cb *cb, L_Oper *oper_in)</arglist>
    </member>
    <member kind="function">
      <type>void</type>
      <name>ComputeBitsUsedLocal_ST1</name>
      <anchorfile>phase1__bitopt_8c.html</anchorfile>
      <anchor>7577e57f342136820105c38f971cf556</anchor>
      <arglist>(L_Func *fn, L_Cb *cb, L_Oper *oper_in)</arglist>
    </member>
    <member kind="function">
      <type>void</type>
      <name>ComputeBitsUsedLocal_ST2</name>
      <anchorfile>phase1__bitopt_8c.html</anchorfile>
      <anchor>724af4148aed7a67e9e359b12052f612</anchor>
      <arglist>(L_Func *fn, L_Cb *cb, L_Oper *oper_in)</arglist>
    </member>
    <member kind="function">
      <type>void</type>
      <name>ComputeBitsUsedLocal_ST4</name>
      <anchorfile>phase1__bitopt_8c.html</anchorfile>
      <anchor>a21e9f6f8022cb81bc7ae8274e72e9da</anchor>
      <arglist>(L_Func *fn, L_Cb *cb, L_Oper *oper_in)</arglist>
    </member>
    <member kind="function">
      <type>void</type>
      <name>ComputeBitsUsedLocal_TBIT_TNAT</name>
      <anchorfile>phase1__bitopt_8c.html</anchorfile>
      <anchor>cdff84b0a77ee95163c34bd647ea3657</anchor>
      <arglist>(L_Func *fn, L_Cb *cb, L_Oper *oper_in)</arglist>
    </member>
    <member kind="function">
      <type>void</type>
      <name>ComputeBitsUsedLocal_CMP</name>
      <anchorfile>phase1__bitopt_8c.html</anchorfile>
      <anchor>0f410ee3b3e0dae06cc9a9d639a1192d</anchor>
      <arglist>(L_Func *fn, L_Cb *cb, L_Oper *oper_in)</arglist>
    </member>
    <member kind="function">
      <type>void</type>
      <name>SearchAndCombineBitsUsed</name>
      <anchorfile>phase1__bitopt_8c.html</anchorfile>
      <anchor>de8fba8f3f9ac5666e15d45529da68c1</anchor>
      <arglist>(L_Func *fn, L_Cb *cb, L_Oper *oper_in, L_Operand *operand_in, LT_bit_vector *pBitsUsed)</arglist>
    </member>
    <member kind="function">
      <type>L_Oper *</type>
      <name>operFindNextReader</name>
      <anchorfile>phase1__bitopt_8c.html</anchorfile>
      <anchor>2d778df2232943efe0bf2b1c7d1b0ce5</anchor>
      <arglist>(L_Oper *operReader, L_Operand *operandDest)</arglist>
    </member>
    <member kind="function">
      <type>void</type>
      <name>TryRedirect</name>
      <anchorfile>phase1__bitopt_8c.html</anchorfile>
      <anchor>391116bfc991eefeba7485a7c90248c1</anchor>
      <arglist>(L_Oper *operProducer, L_Oper *operReader, L_Operand *operandSrcReader, BIT_INFO *pbitSrcReader)</arglist>
    </member>
    <member kind="function">
      <type>void</type>
      <name>TryCMPRedirect</name>
      <anchorfile>phase1__bitopt_8c.html</anchorfile>
      <anchor>6dcc34953a3f56eb47929976da56e6ba</anchor>
      <arglist>(L_Oper *operProducer, L_Oper *operReader)</arglist>
    </member>
    <member kind="function">
      <type>void</type>
      <name>DeallocateBitinfoFields</name>
      <anchorfile>phase1__bitopt_8c.html</anchorfile>
      <anchor>b04d2d9b4c983b24f7aa9a7d32b5da0a</anchor>
      <arglist>(L_Func *fn)</arglist>
    </member>
    <member kind="function">
      <type>void</type>
      <name>AllocateBitinfoFields</name>
      <anchorfile>phase1__bitopt_8c.html</anchorfile>
      <anchor>4bc52bfae2af4e85dd87bcb78b0a50d5</anchor>
      <arglist>(L_Func *fn)</arglist>
    </member>
  </compound>
  <compound kind="file">
    <name>phase1_bitopt.h</name>
    <path>/mnt/hgfs/mkt/Desktop/openimpact/src/Lcode/codegen/Ltahoe/</path>
    <filename>phase1__bitopt_8h</filename>
    <includes id="ltahoe__bitvec_8h" name="ltahoe_bitvec.h" local="yes" imported="no">ltahoe_bitvec.h</includes>
    <class kind="struct">_BIT_INFO</class>
    <member kind="typedef">
      <type>_BIT_INFO</type>
      <name>BIT_INFO</name>
      <anchorfile>phase1__bitopt_8h.html</anchorfile>
      <anchor>e4beba52213f93668499665f53b59303</anchor>
      <arglist></arglist>
    </member>
    <member kind="function">
      <type>void</type>
      <name>L_optimize_bit_trace</name>
      <anchorfile>phase1__bitopt_8h.html</anchorfile>
      <anchor>3eb2331295e2f4b6467f5008b4a3bca9</anchor>
      <arglist>(L_Func *fn)</arglist>
    </member>
    <member kind="function">
      <type>void</type>
      <name>L_optimize_bit_trace_cb</name>
      <anchorfile>phase1__bitopt_8h.html</anchorfile>
      <anchor>a0695a40658334972633e09155ec73fd</anchor>
      <arglist>(L_Func *fn, L_Cb *cb)</arglist>
    </member>
    <member kind="function">
      <type>void</type>
      <name>ComputeKnownBitsFlow</name>
      <anchorfile>phase1__bitopt_8h.html</anchorfile>
      <anchor>e5228e202430c967929ab9dec953de72</anchor>
      <arglist>(L_Func *fn, L_Cb *cb, L_Oper *oper)</arglist>
    </member>
    <member kind="function">
      <type>void</type>
      <name>ProcessOperUsingTD</name>
      <anchorfile>phase1__bitopt_8h.html</anchorfile>
      <anchor>5708f70825d7797ddcd76194716ea4f5</anchor>
      <arglist>(L_Func *fn, L_Cb *cb, L_Oper *oper)</arglist>
    </member>
    <member kind="function">
      <type>void</type>
      <name>Top_down_optimizations</name>
      <anchorfile>phase1__bitopt_8h.html</anchorfile>
      <anchor>d228410f092635c1c98b46adee90869f</anchor>
      <arglist>(L_Func *fn, L_Cb *cb)</arglist>
    </member>
    <member kind="function">
      <type>void</type>
      <name>Bottom_up_optimizations</name>
      <anchorfile>phase1__bitopt_8h.html</anchorfile>
      <anchor>c5ac60034d9a1c86c94c066c9245e132</anchor>
      <arglist>(L_Func *fn, L_Cb *cb)</arglist>
    </member>
    <member kind="function">
      <type>void</type>
      <name>LookupTopDownBitFlow</name>
      <anchorfile>phase1__bitopt_8h.html</anchorfile>
      <anchor>8b97d48f777de67a2ccadccaf4136c6d</anchor>
      <arglist>(L_Func *fn, L_Cb *cb, L_Oper *oper_in, int idxOperandSrc, LT_bit_vector *pKnownZero, LT_bit_vector *pKnownOne)</arglist>
    </member>
    <member kind="function">
      <type>void</type>
      <name>ComputeBitsUsedLocal</name>
      <anchorfile>phase1__bitopt_8h.html</anchorfile>
      <anchor>6f16a9508404780959292bfca5dcc486</anchor>
      <arglist>(L_Func *fn, L_Cb *cb, L_Oper *oper_in)</arglist>
    </member>
    <member kind="function">
      <type>void</type>
      <name>ComputeBitsUsedAndPropagate</name>
      <anchorfile>phase1__bitopt_8h.html</anchorfile>
      <anchor>9e4bc142cde1d4bdfb1fc5dcc62bbfae</anchor>
      <arglist>(L_Func *fn, L_Cb *cb, L_Oper *oper_in)</arglist>
    </member>
    <member kind="function">
      <type>void</type>
      <name>SearchAndCombineBitsUsed</name>
      <anchorfile>phase1__bitopt_8h.html</anchorfile>
      <anchor>de8fba8f3f9ac5666e15d45529da68c1</anchor>
      <arglist>(L_Func *fn, L_Cb *cb, L_Oper *oper_in, L_Operand *operand_in, LT_bit_vector *pBitsUsed)</arglist>
    </member>
    <member kind="function">
      <type>L_Oper *</type>
      <name>operFindNextReader</name>
      <anchorfile>phase1__bitopt_8h.html</anchorfile>
      <anchor>2d778df2232943efe0bf2b1c7d1b0ce5</anchor>
      <arglist>(L_Oper *operReader, L_Operand *operandDest)</arglist>
    </member>
    <member kind="function">
      <type>void</type>
      <name>TryRedirect</name>
      <anchorfile>phase1__bitopt_8h.html</anchorfile>
      <anchor>d7226f44e9edb471ad06e881ea107878</anchor>
      <arglist>(L_Oper *operDest, L_Oper *operReader, L_Operand *operandSrc, BIT_INFO *pbitSrcReader)</arglist>
    </member>
    <member kind="function">
      <type>void</type>
      <name>ComputeBitsUsedDest</name>
      <anchorfile>phase1__bitopt_8h.html</anchorfile>
      <anchor>cc7a0e32dd38dddd8e076f0c03a8a732</anchor>
      <arglist>(L_Func *fn, L_Cb *cb, L_Oper *oper_in, int idxOperandDest)</arglist>
    </member>
    <member kind="function">
      <type>void</type>
      <name>TryCMPRedirect</name>
      <anchorfile>phase1__bitopt_8h.html</anchorfile>
      <anchor>6dcc34953a3f56eb47929976da56e6ba</anchor>
      <arglist>(L_Oper *operProducer, L_Oper *operReader)</arglist>
    </member>
    <member kind="function">
      <type>void</type>
      <name>DeallocateBitinfoFields</name>
      <anchorfile>phase1__bitopt_8h.html</anchorfile>
      <anchor>b04d2d9b4c983b24f7aa9a7d32b5da0a</anchor>
      <arglist>(L_Func *fn)</arglist>
    </member>
    <member kind="function">
      <type>void</type>
      <name>AllocateBitinfoFields</name>
      <anchorfile>phase1__bitopt_8h.html</anchorfile>
      <anchor>4bc52bfae2af4e85dd87bcb78b0a50d5</anchor>
      <arglist>(L_Func *fn)</arglist>
    </member>
    <member kind="function">
      <type>void</type>
      <name>ComputeKnownBitsFlow_EXTR_U</name>
      <anchorfile>phase1__bitopt_8h.html</anchorfile>
      <anchor>31358ed05d5628c2b9423c5c57c1fdfc</anchor>
      <arglist>(L_Func *fn, L_Cb *cb, L_Oper *oper)</arglist>
    </member>
    <member kind="function">
      <type>void</type>
      <name>ComputeKnownBitsFlow_DEP_Z</name>
      <anchorfile>phase1__bitopt_8h.html</anchorfile>
      <anchor>d413ecf13b85c89b14212453c03365e2</anchor>
      <arglist>(L_Func *fn, L_Cb *cb, L_Oper *oper)</arglist>
    </member>
    <member kind="function">
      <type>void</type>
      <name>ComputeKnownBitsFlow_SHLADDP4</name>
      <anchorfile>phase1__bitopt_8h.html</anchorfile>
      <anchor>8b04f48ee007cad2466f01c47838b2be</anchor>
      <arglist>(L_Func *fn, L_Cb *cb, L_Oper *oper)</arglist>
    </member>
    <member kind="function">
      <type>void</type>
      <name>ComputeKnownBitsFlow_SXT1</name>
      <anchorfile>phase1__bitopt_8h.html</anchorfile>
      <anchor>c06b3e5c50bfbc517762b161dc749a88</anchor>
      <arglist>(L_Func *fn, L_Cb *cb, L_Oper *oper)</arglist>
    </member>
    <member kind="function">
      <type>void</type>
      <name>ComputeKnownBitsFlow_SXT2</name>
      <anchorfile>phase1__bitopt_8h.html</anchorfile>
      <anchor>2dd7f8bd100e06c7b94e61e4377f4768</anchor>
      <arglist>(L_Func *fn, L_Cb *cb, L_Oper *oper)</arglist>
    </member>
    <member kind="function">
      <type>void</type>
      <name>ComputeKnownBitsFlow_SXT4</name>
      <anchorfile>phase1__bitopt_8h.html</anchorfile>
      <anchor>3a8a12da0d6b86950dbb591fcc901a5d</anchor>
      <arglist>(L_Func *fn, L_Cb *cb, L_Oper *oper)</arglist>
    </member>
    <member kind="function">
      <type>void</type>
      <name>ComputeKnownBitsFlow_ZXT1</name>
      <anchorfile>phase1__bitopt_8h.html</anchorfile>
      <anchor>b7bf3c3f467f3038c067b5d9cf47909e</anchor>
      <arglist>(L_Func *fn, L_Cb *cb, L_Oper *oper)</arglist>
    </member>
    <member kind="function">
      <type>void</type>
      <name>ComputeKnownBitsFlow_ZXT2</name>
      <anchorfile>phase1__bitopt_8h.html</anchorfile>
      <anchor>c9aa22bb8f256c6d12371f57a0aae6cd</anchor>
      <arglist>(L_Func *fn, L_Cb *cb, L_Oper *oper)</arglist>
    </member>
    <member kind="function">
      <type>void</type>
      <name>ComputeKnownBitsFlow_ZXT4</name>
      <anchorfile>phase1__bitopt_8h.html</anchorfile>
      <anchor>1d36ab4d33fadb20521d4116bd4ccae8</anchor>
      <arglist>(L_Func *fn, L_Cb *cb, L_Oper *oper)</arglist>
    </member>
    <member kind="function">
      <type>void</type>
      <name>ComputeKnownBitsFlow_MOV</name>
      <anchorfile>phase1__bitopt_8h.html</anchorfile>
      <anchor>daf3789b337470b1b3313e84ea975bbf</anchor>
      <arglist>(L_Func *fn, L_Cb *cb, L_Oper *oper)</arglist>
    </member>
    <member kind="function">
      <type>void</type>
      <name>ComputeKnownBitsFlow_AND</name>
      <anchorfile>phase1__bitopt_8h.html</anchorfile>
      <anchor>584cb9f266b693519c932c1414712c70</anchor>
      <arglist>(L_Func *fn, L_Cb *cb, L_Oper *oper)</arglist>
    </member>
    <member kind="function">
      <type>void</type>
      <name>ComputeKnownBitsFlow_OR</name>
      <anchorfile>phase1__bitopt_8h.html</anchorfile>
      <anchor>9019299ae0db751560489c26b1e130ba</anchor>
      <arglist>(L_Func *fn, L_Cb *cb, L_Oper *oper)</arglist>
    </member>
    <member kind="function">
      <type>void</type>
      <name>ComputeKnownBitsFlow_ADDP4</name>
      <anchorfile>phase1__bitopt_8h.html</anchorfile>
      <anchor>de7f1b519159ab96ab00733dda6279ba</anchor>
      <arglist>(L_Func *fn, L_Cb *cb, L_Oper *oper)</arglist>
    </member>
    <member kind="function">
      <type>void</type>
      <name>ComputeKnownBitsFlow_LD1</name>
      <anchorfile>phase1__bitopt_8h.html</anchorfile>
      <anchor>2fcdd54bd865bb3cf992af2e3a600038</anchor>
      <arglist>(L_Func *fn, L_Cb *cb, L_Oper *oper)</arglist>
    </member>
    <member kind="function">
      <type>void</type>
      <name>ComputeKnownBitsFlow_LD2</name>
      <anchorfile>phase1__bitopt_8h.html</anchorfile>
      <anchor>a108b6f7d14d284a1b009d5548ecb1b7</anchor>
      <arglist>(L_Func *fn, L_Cb *cb, L_Oper *oper)</arglist>
    </member>
    <member kind="function">
      <type>void</type>
      <name>ComputeKnownBitsFlow_LD4</name>
      <anchorfile>phase1__bitopt_8h.html</anchorfile>
      <anchor>f151610f0665f82d0a1c836896d37a66</anchor>
      <arglist>(L_Func *fn, L_Cb *cb, L_Oper *oper)</arglist>
    </member>
    <member kind="function">
      <type>void</type>
      <name>ComputeKnownBitsFlow_LD8</name>
      <anchorfile>phase1__bitopt_8h.html</anchorfile>
      <anchor>33a470925434046fd6922067fe1c42fd</anchor>
      <arglist>(L_Func *fn, L_Cb *cb, L_Oper *oper)</arglist>
    </member>
    <member kind="function">
      <type>void</type>
      <name>ProcessOperUsingTD_SXT1</name>
      <anchorfile>phase1__bitopt_8h.html</anchorfile>
      <anchor>5e97845662ffb9500ebfb4096dfcbf9b</anchor>
      <arglist>(L_Func *fn, L_Cb *cb, L_Oper *oper)</arglist>
    </member>
    <member kind="function">
      <type>void</type>
      <name>ProcessOperUsingTD_SXT2</name>
      <anchorfile>phase1__bitopt_8h.html</anchorfile>
      <anchor>5e421f86f5040eb14cc9a44c04f46f18</anchor>
      <arglist>(L_Func *fn, L_Cb *cb, L_Oper *oper)</arglist>
    </member>
    <member kind="function">
      <type>void</type>
      <name>ProcessOperUsingTD_SXT4</name>
      <anchorfile>phase1__bitopt_8h.html</anchorfile>
      <anchor>0ee829fbae9b6479fa310f50ee82c528</anchor>
      <arglist>(L_Func *fn, L_Cb *cb, L_Oper *oper)</arglist>
    </member>
    <member kind="function">
      <type>void</type>
      <name>ProcessOperUsingTD_ZXT1</name>
      <anchorfile>phase1__bitopt_8h.html</anchorfile>
      <anchor>957d17ef71e4ec39d5cc339da4307694</anchor>
      <arglist>(L_Func *fn, L_Cb *cb, L_Oper *oper)</arglist>
    </member>
    <member kind="function">
      <type>void</type>
      <name>ProcessOperUsingTD_ZXT2</name>
      <anchorfile>phase1__bitopt_8h.html</anchorfile>
      <anchor>c301f1ed09765ac54e78772f8374b977</anchor>
      <arglist>(L_Func *fn, L_Cb *cb, L_Oper *oper)</arglist>
    </member>
    <member kind="function">
      <type>void</type>
      <name>ProcessOperUsingTD_ZXT4</name>
      <anchorfile>phase1__bitopt_8h.html</anchorfile>
      <anchor>03f8c8bf740115f29822b8db8bd6ecb6</anchor>
      <arglist>(L_Func *fn, L_Cb *cb, L_Oper *oper)</arglist>
    </member>
    <member kind="function">
      <type>void</type>
      <name>ComputeBitsUsedLocal_SXT1</name>
      <anchorfile>phase1__bitopt_8h.html</anchorfile>
      <anchor>a923390963f01e270884a74d05f9de38</anchor>
      <arglist>(L_Func *fn, L_Cb *cb, L_Oper *oper_in)</arglist>
    </member>
    <member kind="function">
      <type>void</type>
      <name>ComputeBitsUsedLocal_SXT2</name>
      <anchorfile>phase1__bitopt_8h.html</anchorfile>
      <anchor>afc4b485675d957f70e04057aa41466e</anchor>
      <arglist>(L_Func *fn, L_Cb *cb, L_Oper *oper_in)</arglist>
    </member>
    <member kind="function">
      <type>void</type>
      <name>ComputeBitsUsedLocal_SXT4</name>
      <anchorfile>phase1__bitopt_8h.html</anchorfile>
      <anchor>d3c5a9721094407e915e46cbc46f85e5</anchor>
      <arglist>(L_Func *fn, L_Cb *cb, L_Oper *oper_in)</arglist>
    </member>
    <member kind="function">
      <type>void</type>
      <name>ComputeBitsUsedLocal_ZXT1</name>
      <anchorfile>phase1__bitopt_8h.html</anchorfile>
      <anchor>edd6bb222589426f5aac814dad7ddb43</anchor>
      <arglist>(L_Func *fn, L_Cb *cb, L_Oper *oper_in)</arglist>
    </member>
    <member kind="function">
      <type>void</type>
      <name>ComputeBitsUsedLocal_ZXT2</name>
      <anchorfile>phase1__bitopt_8h.html</anchorfile>
      <anchor>bc9c8125293a54812c0dc6bba21894f8</anchor>
      <arglist>(L_Func *fn, L_Cb *cb, L_Oper *oper_in)</arglist>
    </member>
    <member kind="function">
      <type>void</type>
      <name>ComputeBitsUsedLocal_ZXT4</name>
      <anchorfile>phase1__bitopt_8h.html</anchorfile>
      <anchor>474a8803a3cf66954ff2c790887a600e</anchor>
      <arglist>(L_Func *fn, L_Cb *cb, L_Oper *oper_in)</arglist>
    </member>
    <member kind="function">
      <type>void</type>
      <name>ComputeBitsUsedLocal_MOV</name>
      <anchorfile>phase1__bitopt_8h.html</anchorfile>
      <anchor>da486342be0e0a76dccf63996d408d71</anchor>
      <arglist>(L_Func *fn, L_Cb *cb, L_Oper *oper_in)</arglist>
    </member>
    <member kind="function">
      <type>void</type>
      <name>ComputeBitsUsedLocal_ARITH_OP</name>
      <anchorfile>phase1__bitopt_8h.html</anchorfile>
      <anchor>28c543f9abb2bd2435162d5c59a402ae</anchor>
      <arglist>(L_Func *fn, L_Cb *cb, L_Oper *oper_in)</arglist>
    </member>
    <member kind="function">
      <type>void</type>
      <name>ComputeBitsUsedLocal_AND</name>
      <anchorfile>phase1__bitopt_8h.html</anchorfile>
      <anchor>f165ffc607d16aa1f84cf0a0e0be0ed7</anchor>
      <arglist>(L_Func *fn, L_Cb *cb, L_Oper *oper_in)</arglist>
    </member>
    <member kind="function">
      <type>void</type>
      <name>ComputeBitsUsedLocal_ST1</name>
      <anchorfile>phase1__bitopt_8h.html</anchorfile>
      <anchor>7577e57f342136820105c38f971cf556</anchor>
      <arglist>(L_Func *fn, L_Cb *cb, L_Oper *oper_in)</arglist>
    </member>
    <member kind="function">
      <type>void</type>
      <name>ComputeBitsUsedLocal_ST2</name>
      <anchorfile>phase1__bitopt_8h.html</anchorfile>
      <anchor>724af4148aed7a67e9e359b12052f612</anchor>
      <arglist>(L_Func *fn, L_Cb *cb, L_Oper *oper_in)</arglist>
    </member>
    <member kind="function">
      <type>void</type>
      <name>ComputeBitsUsedLocal_ST4</name>
      <anchorfile>phase1__bitopt_8h.html</anchorfile>
      <anchor>a21e9f6f8022cb81bc7ae8274e72e9da</anchor>
      <arglist>(L_Func *fn, L_Cb *cb, L_Oper *oper_in)</arglist>
    </member>
    <member kind="function">
      <type>void</type>
      <name>ComputeBitsUsedLocal_TBIT_TNAT</name>
      <anchorfile>phase1__bitopt_8h.html</anchorfile>
      <anchor>cdff84b0a77ee95163c34bd647ea3657</anchor>
      <arglist>(L_Func *fn, L_Cb *cb, L_Oper *oper_in)</arglist>
    </member>
    <member kind="function">
      <type>void</type>
      <name>ComputeBitsUsedLocal_CMP</name>
      <anchorfile>phase1__bitopt_8h.html</anchorfile>
      <anchor>0f410ee3b3e0dae06cc9a9d639a1192d</anchor>
      <arglist>(L_Func *fn, L_Cb *cb, L_Oper *oper_in)</arglist>
    </member>
  </compound>
  <compound kind="file">
    <name>phase1_func.c</name>
    <path>/mnt/hgfs/mkt/Desktop/openimpact/src/Lcode/codegen/Ltahoe/</path>
    <filename>phase1__func_8c</filename>
    <includes id="ltahoe__main_8h" name="ltahoe_main.h" local="yes" imported="no">ltahoe_main.h</includes>
    <includes id="ltahoe__op__query_8h" name="ltahoe_op_query.h" local="yes" imported="no">ltahoe_op_query.h</includes>
    <includes id="ltahoe__completers_8h" name="ltahoe_completers.h" local="yes" imported="no">ltahoe_completers.h</includes>
    <includes id="phase1__bitopt_8h" name="phase1_bitopt.h" local="yes" imported="no">phase1_bitopt.h</includes>
    <includes id="phase1__func_8h" name="phase1_func.h" local="yes" imported="no">phase1_func.h</includes>
    <includes id="phase1__param_8h" name="phase1_param.h" local="yes" imported="no">phase1_param.h</includes>
    <includes id="phase1__varargs_8h" name="phase1_varargs.h" local="yes" imported="no">phase1_varargs.h</includes>
    <includes id="phase1__opgen_8h" name="phase1_opgen.h" local="yes" imported="no">phase1_opgen.h</includes>
    <includes id="phase2__func_8h" name="phase2_func.h" local="yes" imported="no">phase2_func.h</includes>
    <includes id="phase2__reg_8h" name="phase2_reg.h" local="yes" imported="no">phase2_reg.h</includes>
    <member kind="define">
      <type>#define</type>
      <name>Ltahoe_sign_extend</name>
      <anchorfile>phase1__func_8c.html</anchorfile>
      <anchor>48a7bc67e13a134c20f8b3d12488b031</anchor>
      <arglist>(o, s, d, z)</arglist>
    </member>
    <member kind="define">
      <type>#define</type>
      <name>Ltahoe_zero_extend</name>
      <anchorfile>phase1__func_8c.html</anchorfile>
      <anchor>d2acd85386e1380ae6d275c2f664cd1f</anchor>
      <arglist>(o, s, d, z)</arglist>
    </member>
    <member kind="define">
      <type>#define</type>
      <name>DEBUG_PEEPHOLE</name>
      <anchorfile>phase1__func_8c.html</anchorfile>
      <anchor>6dfff948479ffafb9bde3f1ce27b17d7</anchor>
      <arglist></arglist>
    </member>
    <member kind="function">
      <type>void</type>
      <name>Ltahoe_customize_lcode_compares</name>
      <anchorfile>phase1__func_8c.html</anchorfile>
      <anchor>a3174af3923b29e15b361c6bb83ea0fe</anchor>
      <arglist>(L_Func *fn)</arglist>
    </member>
    <member kind="function" static="yes">
      <type>static int</type>
      <name>Ltahoe_lcode_peephole</name>
      <anchorfile>phase1__func_8c.html</anchorfile>
      <anchor>0eca38abdf39a473982abb0aafd147c9</anchor>
      <arglist>(L_Func *fn)</arglist>
    </member>
    <member kind="function">
      <type>int</type>
      <name>Ltahoe_reduce</name>
      <anchorfile>phase1__func_8c.html</anchorfile>
      <anchor>5993359a0b507a86410dcf29fc12672a</anchor>
      <arglist>(L_Func *fn)</arglist>
    </member>
    <member kind="function">
      <type>void</type>
      <name>LB_hb_pred_merging</name>
      <anchorfile>phase1__func_8c.html</anchorfile>
      <anchor>ba970c2c53d05742fe7b1a94a9c3f95a</anchor>
      <arglist>(L_Func *fn)</arglist>
    </member>
    <member kind="function">
      <type>L_Oper *</type>
      <name>Ltahoe_extend</name>
      <anchorfile>phase1__func_8c.html</anchorfile>
      <anchor>eff8e9d17ae10734261e54274d6d9a13</anchor>
      <arglist>(int sz, L_Oper *oper, L_Operand *src, L_Operand *dest, int size)</arglist>
    </member>
    <member kind="function" static="yes">
      <type>static int</type>
      <name>Ltahoe_should_swap_operands</name>
      <anchorfile>phase1__func_8c.html</anchorfile>
      <anchor>9030d1189b16e316ef7fcd4ddcfbfff7</anchor>
      <arglist>(L_Operand *operand1, L_Operand *operand2)</arglist>
    </member>
    <member kind="function" static="yes">
      <type>static int</type>
      <name>L_get_num_reg_stack_input_regs</name>
      <anchorfile>phase1__func_8c.html</anchorfile>
      <anchor>0a5d87ce1e964b34636ad2e905359791</anchor>
      <arglist>(L_Func *fn)</arglist>
    </member>
    <member kind="function" static="yes">
      <type>static int</type>
      <name>L_get_num_reg_stack_output_regs</name>
      <anchorfile>phase1__func_8c.html</anchorfile>
      <anchor>9a2cc9bfce921601f2eb03397d6417ef</anchor>
      <arglist>(L_Func *fn)</arglist>
    </member>
    <member kind="function">
      <type>int</type>
      <name>L_get_reg_stack_info</name>
      <anchorfile>phase1__func_8c.html</anchorfile>
      <anchor>1ba72f85c141f55e1cf72b9e5d59a29e</anchor>
      <arglist>(L_Func *fn, int *inputs, int *locals, int *outputs, int *rots)</arglist>
    </member>
    <member kind="function">
      <type>void</type>
      <name>L_update_local_space_size</name>
      <anchorfile>phase1__func_8c.html</anchorfile>
      <anchor>1dfcddc49e259d2652b60e5e5bae1d9c</anchor>
      <arglist>(L_Func *fn, int new_value)</arglist>
    </member>
    <member kind="function" static="yes">
      <type>static int</type>
      <name>L_leaf_func</name>
      <anchorfile>phase1__func_8c.html</anchorfile>
      <anchor>bac33d35e3f4672b1a3c4aaf574b37b1</anchor>
      <arglist>(L_Func *fn)</arglist>
    </member>
    <member kind="function" static="yes">
      <type>static int</type>
      <name>L_scan_for_IMPACT_alloc</name>
      <anchorfile>phase1__func_8c.html</anchorfile>
      <anchor>22d88bb3fae8053ab26622750aeeec10</anchor>
      <arglist>(L_Func *fn)</arglist>
    </member>
    <member kind="function" static="yes">
      <type>static void</type>
      <name>L_fix_unallocated_output_regs</name>
      <anchorfile>phase1__func_8c.html</anchorfile>
      <anchor>cf03a4f40d4eefecebc994743c651a1c</anchor>
      <arglist>(L_Func *fn, int num_output_regs)</arglist>
    </member>
    <member kind="function" static="yes">
      <type>static void</type>
      <name>L_update_alloc</name>
      <anchorfile>phase1__func_8c.html</anchorfile>
      <anchor>83dfd4f38c1ff218796edfa22392e946</anchor>
      <arglist>(L_Func *fn)</arglist>
    </member>
    <member kind="function">
      <type>void</type>
      <name>Ltahoe_scan_prologue_defines</name>
      <anchorfile>phase1__func_8c.html</anchorfile>
      <anchor>3f918b00b2ec02f2f4e77ff854bf2fff</anchor>
      <arglist>(L_Func *fn, int *leaf, int *alloc_size)</arglist>
    </member>
    <member kind="function" static="yes">
      <type>static void</type>
      <name>L_annotate_prologue</name>
      <anchorfile>phase1__func_8c.html</anchorfile>
      <anchor>398b0fa4de46d15c1f4dc963e86a258b</anchor>
      <arglist>(L_Func *fn, L_Cb *cb, L_Oper *oper)</arglist>
    </member>
    <member kind="function" static="yes">
      <type>static void</type>
      <name>L_annotate_epilogue</name>
      <anchorfile>phase1__func_8c.html</anchorfile>
      <anchor>79b5da31f638058ce28f76c13336ad1b</anchor>
      <arglist>(L_Func *fn, L_Cb *cb, L_Oper *oper)</arglist>
    </member>
    <member kind="function" static="yes">
      <type>static L_Operand *</type>
      <name>L_float_constant_immed</name>
      <anchorfile>phase1__func_8c.html</anchorfile>
      <anchor>f063abac22942a00651a8ebf7d289205</anchor>
      <arglist>(L_Cb *cb, L_Oper *oper, L_Operand *src_operand)</arglist>
    </member>
    <member kind="function" static="yes">
      <type>static L_Operand *</type>
      <name>L_double_constant_immed</name>
      <anchorfile>phase1__func_8c.html</anchorfile>
      <anchor>f7f8983b708bd7d692ae695b626c9362</anchor>
      <arglist>(L_Cb *cb, L_Oper *oper, L_Operand *src_operand)</arglist>
    </member>
    <member kind="function">
      <type>int</type>
      <name>L_convert_to_depz</name>
      <anchorfile>phase1__func_8c.html</anchorfile>
      <anchor>451fcf6e540accde0bc4cffeb54b8448</anchor>
      <arglist>(ITintmax constant, ITintmax *pnum, int *ppos, int *plen)</arglist>
    </member>
    <member kind="function">
      <type>L_Oper *</type>
      <name>Ltahoe_int_constant_load</name>
      <anchorfile>phase1__func_8c.html</anchorfile>
      <anchor>c9313f8bbc4f15e40e3214048a898a71</anchor>
      <arglist>(L_Cb *cb, L_Oper *oper, L_Operand *src, L_Operand *dest)</arglist>
    </member>
    <member kind="function">
      <type>L_Oper *</type>
      <name>Ltahoe_label_load</name>
      <anchorfile>phase1__func_8c.html</anchorfile>
      <anchor>ee0ecd67279f561ed9763c3cf9e93b1b</anchor>
      <arglist>(L_Cb *cb, L_Oper *oper, L_Operand *src, L_Operand *dest)</arglist>
    </member>
    <member kind="function" static="yes">
      <type>static void</type>
      <name>L_convert_compares_to_predicates</name>
      <anchorfile>phase1__func_8c.html</anchorfile>
      <anchor>447a135ff8ef24f32b33885591af98ca</anchor>
      <arglist>(L_Func *fn)</arglist>
    </member>
    <member kind="function" static="yes">
      <type>static void</type>
      <name>L_annotate_extend</name>
      <anchorfile>phase1__func_8c.html</anchorfile>
      <anchor>8414d07f627d184dfc283d882d90e148</anchor>
      <arglist>(L_Cb *cb, L_Oper *oper)</arglist>
    </member>
    <member kind="function" static="yes">
      <type>static int</type>
      <name>pwr_n_minus_1</name>
      <anchorfile>phase1__func_8c.html</anchorfile>
      <anchor>67bf1fa4a2636bb9c6a5670077c82a3a</anchor>
      <arglist>(ITintmax i)</arglist>
    </member>
    <member kind="function" static="yes">
      <type>static void</type>
      <name>L_annotate_int_logic</name>
      <anchorfile>phase1__func_8c.html</anchorfile>
      <anchor>18e40c45698577aef245d3627492ce77</anchor>
      <arglist>(L_Cb *cb, L_Oper *oper)</arglist>
    </member>
    <member kind="function">
      <type>void</type>
      <name>L_annotate_int_move</name>
      <anchorfile>phase1__func_8c.html</anchorfile>
      <anchor>0a74eff0d79f6ff2ab812aa0f10b5975</anchor>
      <arglist>(L_Cb *cb, L_Oper *oper)</arglist>
    </member>
    <member kind="function" static="yes">
      <type>static void</type>
      <name>L_annotate_int_add</name>
      <anchorfile>phase1__func_8c.html</anchorfile>
      <anchor>7a2ee4f7c9f8f9e1d9aad7bfda0c0960</anchor>
      <arglist>(L_Cb *cb, L_Oper *oper)</arglist>
    </member>
    <member kind="function">
      <type>void</type>
      <name>L_annotate_int_sub</name>
      <anchorfile>phase1__func_8c.html</anchorfile>
      <anchor>c95632c0a2d9d0597593b520feebeaac</anchor>
      <arglist>(L_Cb *cb, L_Oper *oper)</arglist>
    </member>
    <member kind="function" static="yes">
      <type>static void</type>
      <name>L_annotate_abs</name>
      <anchorfile>phase1__func_8c.html</anchorfile>
      <anchor>056520aa15f8fa351715c2a4ede425ad</anchor>
      <arglist>(L_Cb *cb, L_Oper *oper)</arglist>
    </member>
    <member kind="function" static="yes">
      <type>static void</type>
      <name>L_annotate_shladd</name>
      <anchorfile>phase1__func_8c.html</anchorfile>
      <anchor>c883578609f7af65f838e6bb0fdc2d6a</anchor>
      <arglist>(L_Cb *cb, L_Oper *oper)</arglist>
    </member>
    <member kind="function" static="yes">
      <type>static void</type>
      <name>L_annotate_extract_oper</name>
      <anchorfile>phase1__func_8c.html</anchorfile>
      <anchor>75ef960901bc3c818507b0c5bdae6d08</anchor>
      <arglist>(L_Cb *cb, L_Oper *oper)</arglist>
    </member>
    <member kind="function" static="yes">
      <type>static void</type>
      <name>L_annotate_deposit_oper</name>
      <anchorfile>phase1__func_8c.html</anchorfile>
      <anchor>382238b1c3e10b07fec0bc4ca0bce3f8</anchor>
      <arglist>(L_Cb *cb, L_Oper *oper)</arglist>
    </member>
    <member kind="function" static="yes">
      <type>static void</type>
      <name>Ltahoe_annotate_shift</name>
      <anchorfile>phase1__func_8c.html</anchorfile>
      <anchor>0ab7883751defda299f9d655699fcff0</anchor>
      <arglist>(L_Cb *cb, L_Oper *oper)</arglist>
    </member>
    <member kind="function" static="yes">
      <type>static void</type>
      <name>L_annotate_check</name>
      <anchorfile>phase1__func_8c.html</anchorfile>
      <anchor>80e28c6816ab8c5e7fb20816d09c533a</anchor>
      <arglist>(L_Cb *cb, L_Oper *oper)</arglist>
    </member>
    <member kind="function" static="yes">
      <type>static void</type>
      <name>L_annotate_alloca</name>
      <anchorfile>phase1__func_8c.html</anchorfile>
      <anchor>4ba541d708bb1c0a7b313157323d50c6</anchor>
      <arglist>(L_Func *fn, L_Cb *cb, L_Oper *oper, int outparam_space)</arglist>
    </member>
    <member kind="function" static="yes">
      <type>static void</type>
      <name>L_annotate_jsr</name>
      <anchorfile>phase1__func_8c.html</anchorfile>
      <anchor>30455cc5573f749ca3399a0ac398f36d</anchor>
      <arglist>(L_Func *fn, L_Cb *cb, L_Oper *oper)</arglist>
    </member>
    <member kind="function" static="yes">
      <type>static void</type>
      <name>L_annotate_jump</name>
      <anchorfile>phase1__func_8c.html</anchorfile>
      <anchor>72bc14db1cd03668b0f0176d18ffd094</anchor>
      <arglist>(L_Cb *cb, L_Oper *oper)</arglist>
    </member>
    <member kind="function" static="yes">
      <type>static void</type>
      <name>L_annotate_br_indir</name>
      <anchorfile>phase1__func_8c.html</anchorfile>
      <anchor>4811d06aa472fba467eb923a361a52e3</anchor>
      <arglist>(L_Cb *cb, L_Oper *oper)</arglist>
    </member>
    <member kind="function" static="yes">
      <type>static void</type>
      <name>L_annotate_rts</name>
      <anchorfile>phase1__func_8c.html</anchorfile>
      <anchor>3963492a45be7f297546e54571d2cea2</anchor>
      <arglist>(L_Cb *cb, L_Oper *oper)</arglist>
    </member>
    <member kind="function">
      <type>L_Oper *</type>
      <name>Ltahoe_pred_init</name>
      <anchorfile>phase1__func_8c.html</anchorfile>
      <anchor>8356eedeb402e047735773ded6d29628</anchor>
      <arglist>(L_Oper *using, L_Operand *dest0, L_Operand *dest1)</arglist>
    </member>
    <member kind="function" static="yes">
      <type>static void</type>
      <name>Ltahoe_annotate_pred_init</name>
      <anchorfile>phase1__func_8c.html</anchorfile>
      <anchor>9289e722e97835d5f805cd98e4e4ffae</anchor>
      <arglist>(L_Cb *cb, L_Oper *oper)</arglist>
    </member>
    <member kind="function" static="yes">
      <type>static void</type>
      <name>Ltahoe_annotate_cmp</name>
      <anchorfile>phase1__func_8c.html</anchorfile>
      <anchor>c47e3d5cb6fa6b14622e3954afb3f244</anchor>
      <arglist>(L_Cb *cb, L_Oper *oper)</arglist>
    </member>
    <member kind="function" static="yes">
      <type>static L_Operand *</type>
      <name>L_gen_fp_src</name>
      <anchorfile>phase1__func_8c.html</anchorfile>
      <anchor>cd8a8bb4a88f1e36f1b1ef7018a72aa2</anchor>
      <arglist>(L_Cb *cb, L_Oper *oper, L_Operand *src)</arglist>
    </member>
    <member kind="function" static="yes">
      <type>static void</type>
      <name>Ltahoe_annotate_fcmp</name>
      <anchorfile>phase1__func_8c.html</anchorfile>
      <anchor>7251dd996ff629356d23fc9d86ea81fe</anchor>
      <arglist>(L_Cb *cb, L_Oper *oper)</arglist>
    </member>
    <member kind="function" static="yes">
      <type>static L_Operand *</type>
      <name>L_eff_addr_calc</name>
      <anchorfile>phase1__func_8c.html</anchorfile>
      <anchor>5b955f455f7fe9c0c15eff90ecbe6d20</anchor>
      <arglist>(L_Cb *cb, L_Oper *oper, L_Operand *src0, L_Operand *src1)</arglist>
    </member>
    <member kind="function" static="yes">
      <type>static void</type>
      <name>L_annotate_ld</name>
      <anchorfile>phase1__func_8c.html</anchorfile>
      <anchor>0f69496e8168b11305e0665e458db696</anchor>
      <arglist>(L_Cb *cb, L_Oper *oper)</arglist>
    </member>
    <member kind="function" static="yes">
      <type>static void</type>
      <name>L_annotate_st</name>
      <anchorfile>phase1__func_8c.html</anchorfile>
      <anchor>45875c1698a5849d64d3511b1ec86c46</anchor>
      <arglist>(L_Cb *cb, L_Oper *oper)</arglist>
    </member>
    <member kind="function">
      <type>int</type>
      <name>L_dbl_involved_opcode</name>
      <anchorfile>phase1__func_8c.html</anchorfile>
      <anchor>57b8817cdf0d83f812f425499cdf59f9</anchor>
      <arglist>(L_Oper *op)</arglist>
    </member>
    <member kind="function">
      <type>void</type>
      <name>L_convert_double_to_float</name>
      <anchorfile>phase1__func_8c.html</anchorfile>
      <anchor>814862ec5bfefdefe55eca02de117d06</anchor>
      <arglist>(L_Func *fn)</arglist>
    </member>
    <member kind="function">
      <type>void</type>
      <name>L_annotate_float_divide</name>
      <anchorfile>phase1__func_8c.html</anchorfile>
      <anchor>1b9ec573515f67b278e511c8a2a2c142</anchor>
      <arglist>(L_Cb *cb, L_Oper *oper)</arglist>
    </member>
    <member kind="function">
      <type>void</type>
      <name>L_annotate_double_divide</name>
      <anchorfile>phase1__func_8c.html</anchorfile>
      <anchor>beac7edf1fe4df263bfcc2210aa30852</anchor>
      <arglist>(L_Cb *cb, L_Oper *oper)</arglist>
    </member>
    <member kind="function" static="yes">
      <type>static void</type>
      <name>L_annotate_float_mul_oper</name>
      <anchorfile>phase1__func_8c.html</anchorfile>
      <anchor>107cde78b1d05bec13da96a20d977a8e</anchor>
      <arglist>(L_Cb *cb, L_Oper *oper, int op)</arglist>
    </member>
    <member kind="function" static="yes">
      <type>static void</type>
      <name>L_annotate_double_mul_oper</name>
      <anchorfile>phase1__func_8c.html</anchorfile>
      <anchor>eabe68dcf547b8488f0643c0483f8017</anchor>
      <arglist>(L_Cb *cb, L_Oper *oper, int op)</arglist>
    </member>
    <member kind="function" static="yes">
      <type>static void</type>
      <name>L_annotate_add_sub_oper</name>
      <anchorfile>phase1__func_8c.html</anchorfile>
      <anchor>a9a83824949083302f869c4d579f6585</anchor>
      <arglist>(L_Cb *cb, L_Oper *oper)</arglist>
    </member>
    <member kind="function" static="yes">
      <type>static void</type>
      <name>L_annotate_double_abs</name>
      <anchorfile>phase1__func_8c.html</anchorfile>
      <anchor>5118d32f1e1e2392c60934e0adc2365f</anchor>
      <arglist>(L_Cb *cb, L_Oper *oper, int op)</arglist>
    </member>
    <member kind="function" static="yes">
      <type>static void</type>
      <name>L_annotate_float_mul_add_sub_oper</name>
      <anchorfile>phase1__func_8c.html</anchorfile>
      <anchor>fe1250fb162b2de73e0aeced4a0ff245</anchor>
      <arglist>(L_Cb *cb, L_Oper *oper, int op)</arglist>
    </member>
    <member kind="function" static="yes">
      <type>static void</type>
      <name>L_annotate_double_max_min_oper</name>
      <anchorfile>phase1__func_8c.html</anchorfile>
      <anchor>80c4e573cccf1568d5b5f2c113d08865</anchor>
      <arglist>(L_Cb *cb, L_Oper *oper, int op)</arglist>
    </member>
    <member kind="function" static="yes">
      <type>static void</type>
      <name>L_annotate_float_move</name>
      <anchorfile>phase1__func_8c.html</anchorfile>
      <anchor>b4cb579491908417eab878533d5156bc</anchor>
      <arglist>(L_Cb *cb, L_Oper *oper)</arglist>
    </member>
    <member kind="function" static="yes">
      <type>static void</type>
      <name>L_annotate_double_move</name>
      <anchorfile>phase1__func_8c.html</anchorfile>
      <anchor>25de508e5b8d896a7c51bd1c525054e6</anchor>
      <arglist>(L_Cb *cb, L_Oper *oper)</arglist>
    </member>
    <member kind="function" static="yes">
      <type>static void</type>
      <name>L_annotate_float_oper</name>
      <anchorfile>phase1__func_8c.html</anchorfile>
      <anchor>1f54f30101c9570f1deb069dcdf6e184</anchor>
      <arglist>(L_Cb *cb, L_Oper *oper, int op)</arglist>
    </member>
    <member kind="function" static="yes">
      <type>static void</type>
      <name>L_annotate_double_oper</name>
      <anchorfile>phase1__func_8c.html</anchorfile>
      <anchor>09144c09518408ddaec09ec721d7831f</anchor>
      <arglist>(L_Cb *cb, L_Oper *oper, int op)</arglist>
    </member>
    <member kind="function">
      <type>void</type>
      <name>L_annotate_int_to_float</name>
      <anchorfile>phase1__func_8c.html</anchorfile>
      <anchor>10d3f28570f4de42ff8c2ff1f05328e9</anchor>
      <arglist>(L_Cb *cb, L_Oper *oper, int unsigned_conv)</arglist>
    </member>
    <member kind="function">
      <type>void</type>
      <name>L_annotate_int_to_double</name>
      <anchorfile>phase1__func_8c.html</anchorfile>
      <anchor>db7ec3a6196a60a85644cefe7f495362</anchor>
      <arglist>(L_Cb *cb, L_Oper *oper)</arglist>
    </member>
    <member kind="function">
      <type>void</type>
      <name>L_annotate_float_to_int</name>
      <anchorfile>phase1__func_8c.html</anchorfile>
      <anchor>54174b68bc32fa8103ca218e27cfd2e9</anchor>
      <arglist>(L_Cb *cb, L_Oper *oper)</arglist>
    </member>
    <member kind="function">
      <type>void</type>
      <name>L_annotate_double_to_int</name>
      <anchorfile>phase1__func_8c.html</anchorfile>
      <anchor>a4d28aafebdb69d4ad305e968d2a7e7c</anchor>
      <arglist>(L_Cb *cb, L_Oper *oper)</arglist>
    </member>
    <member kind="function" static="yes">
      <type>static void</type>
      <name>L_annotate_double_to_float_conversion</name>
      <anchorfile>phase1__func_8c.html</anchorfile>
      <anchor>674db0fafb4a5a0a72e48e9247b8549f</anchor>
      <arglist>(L_Cb *cb, L_Oper *oper)</arglist>
    </member>
    <member kind="function" static="yes">
      <type>static int</type>
      <name>Ltahoe_is_int_constant_add</name>
      <anchorfile>phase1__func_8c.html</anchorfile>
      <anchor>ec1a70122b3b3605d699d22916961fa3</anchor>
      <arglist>(L_Oper *oper)</arglist>
    </member>
    <member kind="function" static="yes">
      <type>static void</type>
      <name>L_annotate_nop</name>
      <anchorfile>phase1__func_8c.html</anchorfile>
      <anchor>e1f46b93ff713dad056d366a4e7d24ae</anchor>
      <arglist>(L_Cb *cb, L_Oper *oper)</arglist>
    </member>
    <member kind="function" static="yes">
      <type>static void</type>
      <name>L_convert_to_tahoe_oper</name>
      <anchorfile>phase1__func_8c.html</anchorfile>
      <anchor>18bb6e0432706b2add9660a998b853f9</anchor>
      <arglist>(L_Func *fn, L_Cb *cb, L_Oper *oper)</arglist>
    </member>
    <member kind="function">
      <type>void</type>
      <name>L_annotate_oper</name>
      <anchorfile>phase1__func_8c.html</anchorfile>
      <anchor>c4c3576982a78ba9dd8c70cf91908200</anchor>
      <arglist>(L_Func *fn, L_Cb *cb, L_Oper *oper)</arglist>
    </member>
    <member kind="function" static="yes">
      <type>static void</type>
      <name>L_mark_cb_as_hyperblock</name>
      <anchorfile>phase1__func_8c.html</anchorfile>
      <anchor>0945ceea216164f20d07af40b971b090</anchor>
      <arglist>(L_Func *fn, L_Cb *cb, L_Oper *oper)</arglist>
    </member>
    <member kind="function" static="yes">
      <type>static void</type>
      <name>L_opt_flt_constants</name>
      <anchorfile>phase1__func_8c.html</anchorfile>
      <anchor>8d26262dae154b7478224c1843055ca7</anchor>
      <arglist>(L_Func *fn)</arglist>
    </member>
    <member kind="function">
      <type>void</type>
      <name>L_cleanup_after_mopti</name>
      <anchorfile>phase1__func_8c.html</anchorfile>
      <anchor>5706ff5ce085a15e1d7ae06ea2ffba8b</anchor>
      <arglist>(L_Func *fn)</arglist>
    </member>
    <member kind="function">
      <type>void</type>
      <name>L_process_func</name>
      <anchorfile>phase1__func_8c.html</anchorfile>
      <anchor>4f96f29e6a615e8923be439e469ecc0a</anchor>
      <arglist>(L_Func *fn, Parm_Macro_List *command_line_macro_list)</arglist>
    </member>
    <member kind="function">
      <type>void</type>
      <name>L_init</name>
      <anchorfile>phase1__func_8c.html</anchorfile>
      <anchor>b13c730c9b45de9e0e77ca7d44a51e76</anchor>
      <arglist>(Parm_Macro_List *command_line_macro_list)</arglist>
    </member>
    <member kind="function">
      <type>void</type>
      <name>L_end</name>
      <anchorfile>phase1__func_8c.html</anchorfile>
      <anchor>572ee094cc37829b0a8025dbde6de3fe</anchor>
      <arglist>()</arglist>
    </member>
    <member kind="function">
      <type>void</type>
      <name>Ltahoe_annotate_stack_ref</name>
      <anchorfile>phase1__func_8c.html</anchorfile>
      <anchor>c717e009f4f445cf5078805fb08cf88b</anchor>
      <arglist>(L_Oper *op, int mac, int offset, int spill)</arglist>
    </member>
    <member kind="function" static="yes">
      <type>static int</type>
      <name>Ltahoe_mask_oper</name>
      <anchorfile>phase1__func_8c.html</anchorfile>
      <anchor>1686f3d437758e4247a6b5814cd93aab</anchor>
      <arglist>(L_Oper *oper, int *pmask_width, int *preg_indx)</arglist>
    </member>
    <member kind="variable" static="yes">
      <type>static L_Operand *</type>
      <name>pfs_save_operand</name>
      <anchorfile>phase1__func_8c.html</anchorfile>
      <anchor>87facc9ff21b10b49bfb0b84491454db</anchor>
      <arglist></arglist>
    </member>
    <member kind="variable" static="yes">
      <type>static L_Operand *</type>
      <name>retaddr_save_operand</name>
      <anchorfile>phase1__func_8c.html</anchorfile>
      <anchor>89884fdf0f7ce0d6951b0d64b346f4b8</anchor>
      <arglist></arglist>
    </member>
  </compound>
  <compound kind="file">
    <name>phase1_func.h</name>
    <path>/mnt/hgfs/mkt/Desktop/openimpact/src/Lcode/codegen/Ltahoe/</path>
    <filename>phase1__func_8h</filename>
    <member kind="function">
      <type>void</type>
      <name>L_print_fp_constant_data</name>
      <anchorfile>phase1__func_8h.html</anchorfile>
      <anchor>a6ea9c24b3aeddec5ef36d08d4aae334</anchor>
      <arglist>(void)</arglist>
    </member>
    <member kind="function">
      <type>void</type>
      <name>L_process_func</name>
      <anchorfile>phase1__func_8h.html</anchorfile>
      <anchor>4f96f29e6a615e8923be439e469ecc0a</anchor>
      <arglist>(L_Func *fn, Parm_Macro_List *command_line_macro_list)</arglist>
    </member>
    <member kind="function">
      <type>void</type>
      <name>L_init</name>
      <anchorfile>phase1__func_8h.html</anchorfile>
      <anchor>b13c730c9b45de9e0e77ca7d44a51e76</anchor>
      <arglist>(Parm_Macro_List *command_line_macro_list)</arglist>
    </member>
    <member kind="function">
      <type>void</type>
      <name>L_end</name>
      <anchorfile>phase1__func_8h.html</anchorfile>
      <anchor>c3f75f144fe5d1648018ee27a1489460</anchor>
      <arglist>(void)</arglist>
    </member>
    <member kind="function">
      <type>void</type>
      <name>L_scan_prologue_defines</name>
      <anchorfile>phase1__func_8h.html</anchorfile>
      <anchor>e2081495ab04ec1e0f34f5ae7d2b4e1b</anchor>
      <arglist>()</arglist>
    </member>
    <member kind="function">
      <type>int</type>
      <name>L_get_reg_stack_info</name>
      <anchorfile>phase1__func_8h.html</anchorfile>
      <anchor>1ba72f85c141f55e1cf72b9e5d59a29e</anchor>
      <arglist>(L_Func *fn, int *inputs, int *locals, int *outputs, int *rots)</arglist>
    </member>
    <member kind="function">
      <type>L_Oper *</type>
      <name>Ltahoe_int_constant_load</name>
      <anchorfile>phase1__func_8h.html</anchorfile>
      <anchor>c9313f8bbc4f15e40e3214048a898a71</anchor>
      <arglist>(L_Cb *cb, L_Oper *oper, L_Operand *src, L_Operand *dest)</arglist>
    </member>
    <member kind="function">
      <type>L_Oper *</type>
      <name>Ltahoe_label_load</name>
      <anchorfile>phase1__func_8h.html</anchorfile>
      <anchor>ee0ecd67279f561ed9763c3cf9e93b1b</anchor>
      <arglist>(L_Cb *cb, L_Oper *oper, L_Operand *src, L_Operand *dest)</arglist>
    </member>
    <member kind="function">
      <type>void</type>
      <name>L_annotate_double_divide</name>
      <anchorfile>phase1__func_8h.html</anchorfile>
      <anchor>beac7edf1fe4df263bfcc2210aa30852</anchor>
      <arglist>(L_Cb *cb, L_Oper *oper)</arglist>
    </member>
    <member kind="function">
      <type>void</type>
      <name>L_annotate_int_to_float</name>
      <anchorfile>phase1__func_8h.html</anchorfile>
      <anchor>c2c141f92c729f0917050e12d0cd771a</anchor>
      <arglist>(L_Cb *cb, L_Oper *oper, int)</arglist>
    </member>
    <member kind="function">
      <type>void</type>
      <name>L_annotate_float_to_int</name>
      <anchorfile>phase1__func_8h.html</anchorfile>
      <anchor>54174b68bc32fa8103ca218e27cfd2e9</anchor>
      <arglist>(L_Cb *cb, L_Oper *oper)</arglist>
    </member>
    <member kind="function">
      <type>void</type>
      <name>L_annotate_int_move</name>
      <anchorfile>phase1__func_8h.html</anchorfile>
      <anchor>0a74eff0d79f6ff2ab812aa0f10b5975</anchor>
      <arglist>(L_Cb *cb, L_Oper *oper)</arglist>
    </member>
    <member kind="function">
      <type>void</type>
      <name>L_annotate_int_sub</name>
      <anchorfile>phase1__func_8h.html</anchorfile>
      <anchor>c95632c0a2d9d0597593b520feebeaac</anchor>
      <arglist>(L_Cb *cb, L_Oper *oper)</arglist>
    </member>
    <member kind="function">
      <type>void</type>
      <name>L_annotate_int_multiply</name>
      <anchorfile>phase1__func_8h.html</anchorfile>
      <anchor>ef7cf3068f9704ab68ea0299972dcc4f</anchor>
      <arglist>(L_Cb *cb, L_Oper *oper)</arglist>
    </member>
    <member kind="function">
      <type>void</type>
      <name>L_annotate_int_divide</name>
      <anchorfile>phase1__func_8h.html</anchorfile>
      <anchor>95f74b5a6df66391ef2266c8a9a34af6</anchor>
      <arglist>(L_Cb *cb, L_Oper *oper)</arglist>
    </member>
    <member kind="function">
      <type>void</type>
      <name>L_annotate_int_remainder</name>
      <anchorfile>phase1__func_8h.html</anchorfile>
      <anchor>5ed4a0ab82dd9ef8d1ff7d28bf68bc50</anchor>
      <arglist>(L_Cb *cb, L_Oper *oper)</arglist>
    </member>
    <member kind="function">
      <type>void</type>
      <name>L_cleanup_after_mopti</name>
      <anchorfile>phase1__func_8h.html</anchorfile>
      <anchor>5706ff5ce085a15e1d7ae06ea2ffba8b</anchor>
      <arglist>(L_Func *fn)</arglist>
    </member>
    <member kind="function">
      <type>L_Oper *</type>
      <name>Ltahoe_extend</name>
      <anchorfile>phase1__func_8h.html</anchorfile>
      <anchor>eff8e9d17ae10734261e54274d6d9a13</anchor>
      <arglist>(int sz, L_Oper *oper, L_Operand *src, L_Operand *dest, int size)</arglist>
    </member>
    <member kind="function">
      <type>void</type>
      <name>Ltahoe_annotate_stack_ref</name>
      <anchorfile>phase1__func_8h.html</anchorfile>
      <anchor>c717e009f4f445cf5078805fb08cf88b</anchor>
      <arglist>(L_Oper *op, int mac, int offset, int spill)</arglist>
    </member>
    <member kind="function">
      <type>void</type>
      <name>L_annotate_oper</name>
      <anchorfile>phase1__func_8h.html</anchorfile>
      <anchor>fd8def20f61a7cd2a65cd6f3e89f233f</anchor>
      <arglist>(L_Func *, L_Cb *, L_Oper *)</arglist>
    </member>
    <member kind="function">
      <type>void</type>
      <name>Ltahoe_scan_prologue_defines</name>
      <anchorfile>phase1__func_8h.html</anchorfile>
      <anchor>3f918b00b2ec02f2f4e77ff854bf2fff</anchor>
      <arglist>(L_Func *fn, int *leaf, int *alloc_size)</arglist>
    </member>
    <member kind="function">
      <type>void</type>
      <name>L_update_local_space_size</name>
      <anchorfile>phase1__func_8h.html</anchorfile>
      <anchor>1dfcddc49e259d2652b60e5e5bae1d9c</anchor>
      <arglist>(L_Func *fn, int new_value)</arglist>
    </member>
  </compound>
  <compound kind="file">
    <name>phase1_idiv.c</name>
    <path>/mnt/hgfs/mkt/Desktop/openimpact/src/Lcode/codegen/Ltahoe/</path>
    <filename>phase1__idiv_8c</filename>
    <member kind="define">
      <type>#define</type>
      <name>OUT_OF_RANGE</name>
      <anchorfile>phase1__idiv_8c.html</anchorfile>
      <anchor>494da497b677da05f7a302152fafd248</anchor>
      <arglist></arglist>
    </member>
    <member kind="define">
      <type>#define</type>
      <name>STRING_TOO_SHORT</name>
      <anchorfile>phase1__idiv_8c.html</anchorfile>
      <anchor>f9c78d5dfbdd04669b9ccf98aa7e8f15</anchor>
      <arglist></arglist>
    </member>
    <member kind="define">
      <type>#define</type>
      <name>emit</name>
      <anchorfile>phase1__idiv_8c.html</anchorfile>
      <anchor>04f17dbc205bd2c15ba51b01347d2cba</anchor>
      <arglist>(c)</arglist>
    </member>
    <member kind="define">
      <type>#define</type>
      <name>binop</name>
      <anchorfile>phase1__idiv_8c.html</anchorfile>
      <anchor>7c6f1271a077a1f322a407263a6291fe</anchor>
      <arglist>(op, op1, op2)</arglist>
    </member>
    <member kind="define">
      <type>#define</type>
      <name>rshiftop</name>
      <anchorfile>phase1__idiv_8c.html</anchorfile>
      <anchor>37a72a35753d5f728265c0bb933e846c</anchor>
      <arglist>(n, op1)</arglist>
    </member>
    <member kind="define">
      <type>#define</type>
      <name>rashiftop</name>
      <anchorfile>phase1__idiv_8c.html</anchorfile>
      <anchor>6201dbe1b7b459e77c4e57ef95d6f6aa</anchor>
      <arglist>(n, op1)</arglist>
    </member>
    <member kind="function">
      <type>ITuint64</type>
      <name>div64_32</name>
      <anchorfile>phase1__idiv_8c.html</anchorfile>
      <anchor>315b49e50dfa235cffa7d2f303bc1719</anchor>
      <arglist>(ITuint64 numerator, ITuint32 denominator, int bits_numerator, int bits_denominator)</arglist>
    </member>
    <member kind="function" static="yes">
      <type>static ITuint64</type>
      <name>Choose_multiplier</name>
      <anchorfile>phase1__idiv_8c.html</anchorfile>
      <anchor>398e4b3b1393fd6d663454e03ccbb3a1</anchor>
      <arglist>(ITuint32 d, ITuint32 prec, int *sh, int *l)</arglist>
    </member>
    <member kind="function">
      <type>int</type>
      <name>Recip_unsigned_idiv</name>
      <anchorfile>phase1__idiv_8c.html</anchorfile>
      <anchor>dd45eea0606143e136dadc758c32ec1e</anchor>
      <arglist>(char *result, ITuint32 *multiplier, ITuint64 N, int result_len, int IA32)</arglist>
    </member>
    <member kind="function">
      <type>int</type>
      <name>Recip_signed_idiv</name>
      <anchorfile>phase1__idiv_8c.html</anchorfile>
      <anchor>3f5e8db5d12c362849641a8b248e2666</anchor>
      <arglist>(char *result, ITuint32 *multiplier, ITint64 N, int result_len, int IA32)</arglist>
    </member>
  </compound>
  <compound kind="file">
    <name>phase1_idiv.h</name>
    <path>/mnt/hgfs/mkt/Desktop/openimpact/src/Lcode/codegen/Ltahoe/</path>
    <filename>phase1__idiv_8h</filename>
    <member kind="function">
      <type>int</type>
      <name>Recip_unsigned_idiv</name>
      <anchorfile>phase1__idiv_8h.html</anchorfile>
      <anchor>dd45eea0606143e136dadc758c32ec1e</anchor>
      <arglist>(char *result, ITuint32 *multiplier, ITuint64 N, int result_len, int IA32)</arglist>
    </member>
    <member kind="function">
      <type>int</type>
      <name>Recip_signed_idiv</name>
      <anchorfile>phase1__idiv_8h.html</anchorfile>
      <anchor>3f5e8db5d12c362849641a8b248e2666</anchor>
      <arglist>(char *result, ITuint32 *multiplier, ITint64 N, int result_len, int IA32)</arglist>
    </member>
  </compound>
  <compound kind="file">
    <name>phase1_imath.c</name>
    <path>/mnt/hgfs/mkt/Desktop/openimpact/src/Lcode/codegen/Ltahoe/</path>
    <filename>phase1__imath_8c</filename>
    <includes id="ltahoe__main_8h" name="ltahoe_main.h" local="yes" imported="no">ltahoe_main.h</includes>
    <includes id="ltahoe__op__query_8h" name="ltahoe_op_query.h" local="yes" imported="no">ltahoe_op_query.h</includes>
    <includes id="phase1__func_8h" name="phase1_func.h" local="yes" imported="no">phase1_func.h</includes>
    <includes id="phase1__imult_8h" name="phase1_imult.h" local="yes" imported="no">phase1_imult.h</includes>
    <includes id="phase1__idiv_8h" name="phase1_idiv.h" local="yes" imported="no">phase1_idiv.h</includes>
    <includes id="phase1__opgen_8h" name="phase1_opgen.h" local="yes" imported="no">phase1_opgen.h</includes>
    <member kind="define">
      <type>#define</type>
      <name>OUTPUT_LEN</name>
      <anchorfile>phase1__imath_8c.html</anchorfile>
      <anchor>8eaf0d818e25f08acd990fa2366ef249</anchor>
      <arglist></arglist>
    </member>
    <member kind="define">
      <type>#define</type>
      <name>DO_PMPY</name>
      <anchorfile>phase1__imath_8c.html</anchorfile>
      <anchor>62a0cf8e3068cf37de3a745fa90ee8ad</anchor>
      <arglist></arglist>
    </member>
    <member kind="function" static="yes">
      <type>static int</type>
      <name>L_table_imul</name>
      <anchorfile>phase1__imath_8c.html</anchorfile>
      <anchor>0d22238cf86021c15d4a6ba5304d60a3</anchor>
      <arglist>(L_Cb *cb, L_Oper *oper, L_Operand *src_reg, L_Operand *src_int)</arglist>
    </member>
    <member kind="function" static="yes">
      <type>static void</type>
      <name>L_tahoe_int_register_multiply</name>
      <anchorfile>phase1__imath_8c.html</anchorfile>
      <anchor>24d2998ba95b86a152ffc4a5e8c1dbb5</anchor>
      <arglist>(L_Cb *cb, L_Oper *oper, L_Operand *src_reg0, L_Operand *src_reg1)</arglist>
    </member>
    <member kind="function">
      <type>void</type>
      <name>L_annotate_int_multiply</name>
      <anchorfile>phase1__imath_8c.html</anchorfile>
      <anchor>ef7cf3068f9704ab68ea0299972dcc4f</anchor>
      <arglist>(L_Cb *cb, L_Oper *oper)</arglist>
    </member>
    <member kind="function">
      <type>int</type>
      <name>L_table_idiv</name>
      <anchorfile>phase1__imath_8c.html</anchorfile>
      <anchor>b777eb35e4a7b894c10ece86da25ca63</anchor>
      <arglist>(L_Cb *cb, L_Oper *oper, L_Operand *src_reg, L_Operand *src_int)</arglist>
    </member>
    <member kind="function">
      <type>void</type>
      <name>L_annotate_int_complex_division</name>
      <anchorfile>phase1__imath_8c.html</anchorfile>
      <anchor>9e7da244a50fd08577321dae496bdbb9</anchor>
      <arglist>(L_Cb *cb, L_Oper *oper)</arglist>
    </member>
    <member kind="function">
      <type>void</type>
      <name>L_annotate_int_divide</name>
      <anchorfile>phase1__imath_8c.html</anchorfile>
      <anchor>95f74b5a6df66391ef2266c8a9a34af6</anchor>
      <arglist>(L_Cb *cb, L_Oper *oper)</arglist>
    </member>
    <member kind="function">
      <type>void</type>
      <name>L_annotate_int_remainder</name>
      <anchorfile>phase1__imath_8c.html</anchorfile>
      <anchor>5ed4a0ab82dd9ef8d1ff7d28bf68bc50</anchor>
      <arglist>(L_Cb *cb, L_Oper *oper)</arglist>
    </member>
  </compound>
  <compound kind="file">
    <name>phase1_imult.c</name>
    <path>/mnt/hgfs/mkt/Desktop/openimpact/src/Lcode/codegen/Ltahoe/</path>
    <filename>phase1__imult_8c</filename>
    <includes id="phase1__imult__tab_8h" name="phase1_imult_tab.h" local="yes" imported="no">phase1_imult_tab.h</includes>
    <class kind="struct">Imul_entry</class>
    <member kind="define">
      <type>#define</type>
      <name>OUT_OF_RANGE</name>
      <anchorfile>phase1__imult_8c.html</anchorfile>
      <anchor>494da497b677da05f7a302152fafd248</anchor>
      <arglist></arglist>
    </member>
    <member kind="define">
      <type>#define</type>
      <name>STRING_TOO_SHORT</name>
      <anchorfile>phase1__imult_8c.html</anchorfile>
      <anchor>f9c78d5dfbdd04669b9ccf98aa7e8f15</anchor>
      <arglist></arglist>
    </member>
    <member kind="define">
      <type>#define</type>
      <name>emit</name>
      <anchorfile>phase1__imult_8c.html</anchorfile>
      <anchor>04f17dbc205bd2c15ba51b01347d2cba</anchor>
      <arglist>(c)</arglist>
    </member>
    <member kind="typedef">
      <type>Imul_entry</type>
      <name>Imul_entry_t</name>
      <anchorfile>phase1__imult_8c.html</anchorfile>
      <anchor>25a5d80e64db59d7d8f721c2a808afb1</anchor>
      <arglist></arglist>
    </member>
    <member kind="function">
      <type>int</type>
      <name>visit_entries</name>
      <anchorfile>phase1__imult_8c.html</anchorfile>
      <anchor>2df5045bf040bc6c73178d2c367ddf4f</anchor>
      <arglist>(int N, char *result)</arglist>
    </member>
    <member kind="function">
      <type>char</type>
      <name>lookup</name>
      <anchorfile>phase1__imult_8c.html</anchorfile>
      <anchor>17d9a75aa14f2db16d0df1ff3e0b44f4</anchor>
      <arglist>(int N)</arglist>
    </member>
    <member kind="function">
      <type>void</type>
      <name>add_to_temps</name>
      <anchorfile>phase1__imult_8c.html</anchorfile>
      <anchor>5d9482dfff0fd1839a3f5da271219448</anchor>
      <arglist>(int N)</arglist>
    </member>
    <member kind="function">
      <type>int</type>
      <name>imul_sequence</name>
      <anchorfile>phase1__imult_8c.html</anchorfile>
      <anchor>be60d6bf121981d7cb54d90348b7111a</anchor>
      <arglist>(int N, char *result, int string_len, int *depth, int *i_count)</arglist>
    </member>
    <member kind="variable" static="yes">
      <type>static int</type>
      <name>temp_number</name>
      <anchorfile>phase1__imult_8c.html</anchorfile>
      <anchor>77bb05b11245f2c1f81f027d0d577e4d</anchor>
      <arglist>[100]</arglist>
    </member>
    <member kind="variable" static="yes">
      <type>static int</type>
      <name>num_temps</name>
      <anchorfile>phase1__imult_8c.html</anchorfile>
      <anchor>c383874339db017678da3e6318e0141a</anchor>
      <arglist></arglist>
    </member>
    <member kind="variable" static="yes">
      <type>static int</type>
      <name>cursor</name>
      <anchorfile>phase1__imult_8c.html</anchorfile>
      <anchor>f3b2b8ffe2b1effcf3983354beb28099</anchor>
      <arglist></arglist>
    </member>
    <member kind="variable" static="yes">
      <type>static int</type>
      <name>max_cursor</name>
      <anchorfile>phase1__imult_8c.html</anchorfile>
      <anchor>32b845b6923b20d56c8f893e37420fc6</anchor>
      <arglist></arglist>
    </member>
  </compound>
  <compound kind="file">
    <name>phase1_imult.h</name>
    <path>/mnt/hgfs/mkt/Desktop/openimpact/src/Lcode/codegen/Ltahoe/</path>
    <filename>phase1__imult_8h</filename>
    <member kind="function">
      <type>int</type>
      <name>imul_sequence</name>
      <anchorfile>phase1__imult_8h.html</anchorfile>
      <anchor>be60d6bf121981d7cb54d90348b7111a</anchor>
      <arglist>(int N, char *result, int string_len, int *depth, int *i_count)</arglist>
    </member>
  </compound>
  <compound kind="file">
    <name>phase1_imult_tab.h</name>
    <path>/mnt/hgfs/mkt/Desktop/openimpact/src/Lcode/codegen/Ltahoe/</path>
    <filename>phase1__imult__tab_8h</filename>
    <member kind="define">
      <type>#define</type>
      <name>IMUL_MAX</name>
      <anchorfile>phase1__imult__tab_8h.html</anchorfile>
      <anchor>c756db04e6f99ab5270ec36397026739</anchor>
      <arglist></arglist>
    </member>
    <member kind="variable">
      <type>Imul_entry</type>
      <name>Imul_table</name>
      <anchorfile>phase1__imult__tab_8h.html</anchorfile>
      <anchor>a5a9482d00ad4d390d43a1224c3bec9f</anchor>
      <arglist>[31870]</arglist>
    </member>
  </compound>
  <compound kind="file">
    <name>phase1_imult_tab2k.h</name>
    <path>/mnt/hgfs/mkt/Desktop/openimpact/src/Lcode/codegen/Ltahoe/</path>
    <filename>phase1__imult__tab2k_8h</filename>
    <member kind="define">
      <type>#define</type>
      <name>IMUL_MAX</name>
      <anchorfile>phase1__imult__tab2k_8h.html</anchorfile>
      <anchor>c756db04e6f99ab5270ec36397026739</anchor>
      <arglist></arglist>
    </member>
    <member kind="variable">
      <type>Imul_entry</type>
      <name>Imul_table</name>
      <anchorfile>phase1__imult__tab2k_8h.html</anchorfile>
      <anchor>50cd9a7d99da3b9c25425d31ba747757</anchor>
      <arglist>[20001]</arglist>
    </member>
  </compound>
  <compound kind="file">
    <name>phase1_opgen.c</name>
    <path>/mnt/hgfs/mkt/Desktop/openimpact/src/Lcode/codegen/Ltahoe/</path>
    <filename>phase1__opgen_8c</filename>
    <includes id="ltahoe__main_8h" name="ltahoe_main.h" local="yes" imported="no">ltahoe_main.h</includes>
    <includes id="ltahoe__completers_8h" name="ltahoe_completers.h" local="yes" imported="no">ltahoe_completers.h</includes>
    <includes id="phase1__opgen_8h" name="phase1_opgen.h" local="yes" imported="no">phase1_opgen.h</includes>
    <member kind="function">
      <type>int</type>
      <name>Ltahoe_get_fsf</name>
      <anchorfile>phase1__opgen_8c.html</anchorfile>
      <anchor>de0c9158f3806cb857b8140a950867b5</anchor>
      <arglist>(L_Oper *oper)</arglist>
    </member>
    <member kind="function">
      <type>void</type>
      <name>Ltahoe_set_fsf</name>
      <anchorfile>phase1__opgen_8c.html</anchorfile>
      <anchor>d61083513e3a14f6648856b9a1c90313</anchor>
      <arglist>(L_Oper *oper, int fsf)</arglist>
    </member>
    <member kind="function">
      <type>int</type>
      <name>Ltahoe_get_fpc</name>
      <anchorfile>phase1__opgen_8c.html</anchorfile>
      <anchor>3f07d9ac5afb8d7372b571050d34b5ef</anchor>
      <arglist>(L_Oper *oper)</arglist>
    </member>
    <member kind="function">
      <type>void</type>
      <name>Ltahoe_set_fpc</name>
      <anchorfile>phase1__opgen_8c.html</anchorfile>
      <anchor>413b7a016b9d692e3f392255cf7d3a46</anchor>
      <arglist>(L_Oper *oper, int fpc)</arglist>
    </member>
    <member kind="function">
      <type>L_Oper *</type>
      <name>Ltahoe_new_fma</name>
      <anchorfile>phase1__opgen_8c.html</anchorfile>
      <anchor>8c1079d396a777fd71a8691530ae3f2d</anchor>
      <arglist>(L_Operand *pred, L_Operand *dest, L_Operand *src0, L_Operand *src1, L_Operand *src2, int fsf, int fpc, L_Oper *using)</arglist>
    </member>
    <member kind="function">
      <type>L_Oper *</type>
      <name>Ltahoe_new_fnma</name>
      <anchorfile>phase1__opgen_8c.html</anchorfile>
      <anchor>74265e3366c1e52f0779021789c24757</anchor>
      <arglist>(L_Operand *pred, L_Operand *dest, L_Operand *src0, L_Operand *src1, L_Operand *src2, int fsf, int fpc, L_Oper *using)</arglist>
    </member>
    <member kind="function">
      <type>L_Oper *</type>
      <name>Ltahoe_new_fadd</name>
      <anchorfile>phase1__opgen_8c.html</anchorfile>
      <anchor>187e95ec8e4852d328944cacbdb59017</anchor>
      <arglist>(L_Operand *pred, L_Operand *dest, L_Operand *src0, L_Operand *src1, int fsf, int fpc, L_Oper *using)</arglist>
    </member>
    <member kind="function">
      <type>L_Oper *</type>
      <name>Ltahoe_new_frcpa</name>
      <anchorfile>phase1__opgen_8c.html</anchorfile>
      <anchor>24c3da9a113f30c7337bab6a9fbb51c2</anchor>
      <arglist>(L_Operand *pred, L_Operand *dest0, L_Operand *dest1, L_Operand *src0, L_Operand *src1, int fsf, L_Oper *using)</arglist>
    </member>
    <member kind="function">
      <type>L_Oper *</type>
      <name>Ltahoe_new_fcvt_fx_trunc</name>
      <anchorfile>phase1__opgen_8c.html</anchorfile>
      <anchor>391a81dc31aa9b99e79ba089ae0cc4da</anchor>
      <arglist>(L_Operand *pred, L_Operand *dest0, L_Operand *src0, int fsf, L_Oper *using)</arglist>
    </member>
    <member kind="function">
      <type>L_Oper *</type>
      <name>Ltahoe_new_getf_sig</name>
      <anchorfile>phase1__opgen_8c.html</anchorfile>
      <anchor>caddde0a98dccd97173ede3555630ea4</anchor>
      <arglist>(L_Operand *pred, L_Operand *dest0, L_Operand *src0, L_Oper *using)</arglist>
    </member>
    <member kind="function">
      <type>L_Oper *</type>
      <name>Ltahoe_new_movi</name>
      <anchorfile>phase1__opgen_8c.html</anchorfile>
      <anchor>a88e225e5fb73e21ae44afc2a3736cd3</anchor>
      <arglist>(L_Operand *pred, L_Operand *dest0, ITint64 value, L_Oper *using)</arglist>
    </member>
    <member kind="function">
      <type>L_Oper *</type>
      <name>Ltahoe_new_movl</name>
      <anchorfile>phase1__opgen_8c.html</anchorfile>
      <anchor>a6e5de7050520717ea3aeb5e079f4b64</anchor>
      <arglist>(L_Operand *pred, L_Operand *dest0, ITint64 value, L_Oper *using)</arglist>
    </member>
    <member kind="function">
      <type>L_Oper *</type>
      <name>Ltahoe_new_setf_exp</name>
      <anchorfile>phase1__opgen_8c.html</anchorfile>
      <anchor>fa76e7d98d16c17ea1a234131b514e80</anchor>
      <arglist>(L_Operand *pred, L_Operand *dest0, L_Operand *src0, L_Oper *using)</arglist>
    </member>
    <member kind="function">
      <type>L_Oper *</type>
      <name>Ltahoe_new_setf_s</name>
      <anchorfile>phase1__opgen_8c.html</anchorfile>
      <anchor>c153abd6bc214664f4b2b179d1156724</anchor>
      <arglist>(L_Operand *pred, L_Operand *dest0, L_Operand *src0, L_Oper *using)</arglist>
    </member>
    <member kind="function">
      <type>L_Oper *</type>
      <name>Ltahoe_new_setf_sig</name>
      <anchorfile>phase1__opgen_8c.html</anchorfile>
      <anchor>824f2400b7dcd527d4d294db2aa02454</anchor>
      <arglist>(L_Operand *pred, L_Operand *dest0, L_Operand *src0, L_Oper *using)</arglist>
    </member>
    <member kind="function">
      <type>L_Oper *</type>
      <name>Ltahoe_new_xma_l</name>
      <anchorfile>phase1__opgen_8c.html</anchorfile>
      <anchor>680f2a092638a600790fd865c3a5e7e1</anchor>
      <arglist>(L_Operand *pred, L_Operand *dest, L_Operand *src0, L_Operand *src1, L_Operand *src2, L_Oper *using)</arglist>
    </member>
    <member kind="function" static="yes">
      <type>static void</type>
      <name>Ltahoe_generate_temp_regs</name>
      <anchorfile>phase1__opgen_8c.html</anchorfile>
      <anchor>dc554665babacb11c455f9fe7df0bed9</anchor>
      <arglist>(L_Operand **opdarry, int count, int ctype, int ptype)</arglist>
    </member>
    <member kind="function" static="yes">
      <type>static void</type>
      <name>Ltahoe_free_temp_regs</name>
      <anchorfile>phase1__opgen_8c.html</anchorfile>
      <anchor>35250379dfe86d96fcab3c104cf3d150</anchor>
      <arglist>(L_Operand **opdarry, int count)</arglist>
    </member>
    <member kind="function">
      <type>void</type>
      <name>Ltahoe_annotate_EM_int_divide64_lat</name>
      <anchorfile>phase1__opgen_8c.html</anchorfile>
      <anchor>f067d6ee9b9116f32f0ceabca6ee5f8f</anchor>
      <arglist>(L_Cb *cb, L_Oper *oper)</arglist>
    </member>
    <member kind="function">
      <type>void</type>
      <name>Ltahoe_annotate_EM_int_divide64_thr</name>
      <anchorfile>phase1__opgen_8c.html</anchorfile>
      <anchor>78cd283e71d1cda4f9dd858ab72f71b0</anchor>
      <arglist>(L_Cb *cb, L_Oper *oper)</arglist>
    </member>
    <member kind="function">
      <type>void</type>
      <name>Ltahoe_annotate_EM_int_divide32</name>
      <anchorfile>phase1__opgen_8c.html</anchorfile>
      <anchor>d50431a9157b440b245fae6a7323314b</anchor>
      <arglist>(L_Cb *cb, L_Oper *oper)</arglist>
    </member>
    <member kind="function">
      <type>void</type>
      <name>Ltahoe_annotate_EM_int_divide16_thr</name>
      <anchorfile>phase1__opgen_8c.html</anchorfile>
      <anchor>56d5601422c10a505d645f0f1b5ac878</anchor>
      <arglist>(L_Cb *cb, L_Oper *oper)</arglist>
    </member>
    <member kind="function">
      <type>void</type>
      <name>Ltahoe_annotate_EM_int_divide8_thr</name>
      <anchorfile>phase1__opgen_8c.html</anchorfile>
      <anchor>27c01744432451e3e78fd6c15d1b8d4b</anchor>
      <arglist>(L_Cb *cb, L_Oper *oper)</arglist>
    </member>
  </compound>
  <compound kind="file">
    <name>phase1_opgen.h</name>
    <path>/mnt/hgfs/mkt/Desktop/openimpact/src/Lcode/codegen/Ltahoe/</path>
    <filename>phase1__opgen_8h</filename>
    <member kind="enumvalue">
      <name>FPC_NONE</name>
      <anchor>06fc87d81c62e9abb8790b6e5713c55b63fb7f0d1ad9435a7478a383920055b2</anchor>
      <arglist></arglist>
    </member>
    <member kind="enumvalue">
      <name>FPC_S</name>
      <anchor>06fc87d81c62e9abb8790b6e5713c55bd50066422a354eea9cf1cfb87b822e4d</anchor>
      <arglist></arglist>
    </member>
    <member kind="enumvalue">
      <name>FPC_D</name>
      <anchor>06fc87d81c62e9abb8790b6e5713c55b240557cc406389a483a88e2de97eeed5</anchor>
      <arglist></arglist>
    </member>
    <member kind="enumvalue">
      <name>FSF_S0</name>
      <anchor>df764cbdea00d65edcd07bb9953ad2b70b11c1e33be45ec2fd50a9a058119302</anchor>
      <arglist></arglist>
    </member>
    <member kind="enumvalue">
      <name>FSF_S1</name>
      <anchor>df764cbdea00d65edcd07bb9953ad2b78b65c29362ad7eaeb9311d529b7206f5</anchor>
      <arglist></arglist>
    </member>
    <member kind="enumvalue">
      <name>FSF_S2</name>
      <anchor>df764cbdea00d65edcd07bb9953ad2b7482763d83a03d9d258207e54129f3780</anchor>
      <arglist></arglist>
    </member>
    <member kind="enumvalue">
      <name>FSF_S3</name>
      <anchor>df764cbdea00d65edcd07bb9953ad2b761a22bd7eabac0d20ef40509c31ad18e</anchor>
      <arglist></arglist>
    </member>
    <member kind="function">
      <type>void</type>
      <name>Ltahoe_annotate_EM_int_divide8_thr</name>
      <anchorfile>phase1__opgen_8h.html</anchorfile>
      <anchor>27c01744432451e3e78fd6c15d1b8d4b</anchor>
      <arglist>(L_Cb *cb, L_Oper *oper)</arglist>
    </member>
    <member kind="function">
      <type>void</type>
      <name>Ltahoe_annotate_EM_int_divide16_thr</name>
      <anchorfile>phase1__opgen_8h.html</anchorfile>
      <anchor>56d5601422c10a505d645f0f1b5ac878</anchor>
      <arglist>(L_Cb *cb, L_Oper *oper)</arglist>
    </member>
    <member kind="function">
      <type>void</type>
      <name>Ltahoe_annotate_EM_int_divide32</name>
      <anchorfile>phase1__opgen_8h.html</anchorfile>
      <anchor>d50431a9157b440b245fae6a7323314b</anchor>
      <arglist>(L_Cb *cb, L_Oper *oper)</arglist>
    </member>
    <member kind="function">
      <type>void</type>
      <name>Ltahoe_annotate_EM_int_divide64_lat</name>
      <anchorfile>phase1__opgen_8h.html</anchorfile>
      <anchor>f067d6ee9b9116f32f0ceabca6ee5f8f</anchor>
      <arglist>(L_Cb *cb, L_Oper *oper)</arglist>
    </member>
    <member kind="function">
      <type>void</type>
      <name>Ltahoe_annotate_EM_int_divide64_thr</name>
      <anchorfile>phase1__opgen_8h.html</anchorfile>
      <anchor>78cd283e71d1cda4f9dd858ab72f71b0</anchor>
      <arglist>(L_Cb *cb, L_Oper *oper)</arglist>
    </member>
    <member kind="function">
      <type>L_Oper *</type>
      <name>Ltahoe_new_fma</name>
      <anchorfile>phase1__opgen_8h.html</anchorfile>
      <anchor>8c1079d396a777fd71a8691530ae3f2d</anchor>
      <arglist>(L_Operand *pred, L_Operand *dest, L_Operand *src0, L_Operand *src1, L_Operand *src2, int fsf, int fpc, L_Oper *using)</arglist>
    </member>
    <member kind="function">
      <type>L_Oper *</type>
      <name>Ltahoe_new_fnma</name>
      <anchorfile>phase1__opgen_8h.html</anchorfile>
      <anchor>74265e3366c1e52f0779021789c24757</anchor>
      <arglist>(L_Operand *pred, L_Operand *dest, L_Operand *src0, L_Operand *src1, L_Operand *src2, int fsf, int fpc, L_Oper *using)</arglist>
    </member>
    <member kind="function">
      <type>L_Oper *</type>
      <name>Ltahoe_new_fadd</name>
      <anchorfile>phase1__opgen_8h.html</anchorfile>
      <anchor>187e95ec8e4852d328944cacbdb59017</anchor>
      <arglist>(L_Operand *pred, L_Operand *dest, L_Operand *src0, L_Operand *src1, int fsf, int fpc, L_Oper *using)</arglist>
    </member>
    <member kind="function">
      <type>L_Oper *</type>
      <name>Ltahoe_new_frcpa</name>
      <anchorfile>phase1__opgen_8h.html</anchorfile>
      <anchor>24c3da9a113f30c7337bab6a9fbb51c2</anchor>
      <arglist>(L_Operand *pred, L_Operand *dest0, L_Operand *dest1, L_Operand *src0, L_Operand *src1, int fsf, L_Oper *using)</arglist>
    </member>
    <member kind="function">
      <type>L_Oper *</type>
      <name>Ltahoe_new_fcvt_fx_trunc</name>
      <anchorfile>phase1__opgen_8h.html</anchorfile>
      <anchor>391a81dc31aa9b99e79ba089ae0cc4da</anchor>
      <arglist>(L_Operand *pred, L_Operand *dest0, L_Operand *src0, int fsf, L_Oper *using)</arglist>
    </member>
    <member kind="function">
      <type>L_Oper *</type>
      <name>Ltahoe_new_getf_sig</name>
      <anchorfile>phase1__opgen_8h.html</anchorfile>
      <anchor>caddde0a98dccd97173ede3555630ea4</anchor>
      <arglist>(L_Operand *pred, L_Operand *dest0, L_Operand *src0, L_Oper *using)</arglist>
    </member>
    <member kind="function">
      <type>L_Oper *</type>
      <name>Ltahoe_new_movi</name>
      <anchorfile>phase1__opgen_8h.html</anchorfile>
      <anchor>a88e225e5fb73e21ae44afc2a3736cd3</anchor>
      <arglist>(L_Operand *pred, L_Operand *dest0, ITint64 value, L_Oper *using)</arglist>
    </member>
    <member kind="function">
      <type>L_Oper *</type>
      <name>Ltahoe_new_movl</name>
      <anchorfile>phase1__opgen_8h.html</anchorfile>
      <anchor>a6e5de7050520717ea3aeb5e079f4b64</anchor>
      <arglist>(L_Operand *pred, L_Operand *dest0, ITint64 value, L_Oper *using)</arglist>
    </member>
    <member kind="function">
      <type>L_Oper *</type>
      <name>Ltahoe_new_setf_exp</name>
      <anchorfile>phase1__opgen_8h.html</anchorfile>
      <anchor>fa76e7d98d16c17ea1a234131b514e80</anchor>
      <arglist>(L_Operand *pred, L_Operand *dest0, L_Operand *src0, L_Oper *using)</arglist>
    </member>
    <member kind="function">
      <type>L_Oper *</type>
      <name>Ltahoe_new_setf_s</name>
      <anchorfile>phase1__opgen_8h.html</anchorfile>
      <anchor>c153abd6bc214664f4b2b179d1156724</anchor>
      <arglist>(L_Operand *pred, L_Operand *dest0, L_Operand *src0, L_Oper *using)</arglist>
    </member>
    <member kind="function">
      <type>L_Oper *</type>
      <name>Ltahoe_new_setf_sig</name>
      <anchorfile>phase1__opgen_8h.html</anchorfile>
      <anchor>824f2400b7dcd527d4d294db2aa02454</anchor>
      <arglist>(L_Operand *pred, L_Operand *dest0, L_Operand *src0, L_Oper *using)</arglist>
    </member>
    <member kind="function">
      <type>L_Oper *</type>
      <name>Ltahoe_new_xma_l</name>
      <anchorfile>phase1__opgen_8h.html</anchorfile>
      <anchor>680f2a092638a600790fd865c3a5e7e1</anchor>
      <arglist>(L_Operand *pred, L_Operand *dest, L_Operand *src0, L_Operand *src1, L_Operand *src2, L_Oper *using)</arglist>
    </member>
    <member kind="function">
      <type>int</type>
      <name>Ltahoe_get_fsf</name>
      <anchorfile>phase1__opgen_8h.html</anchorfile>
      <anchor>de0c9158f3806cb857b8140a950867b5</anchor>
      <arglist>(L_Oper *oper)</arglist>
    </member>
    <member kind="function">
      <type>void</type>
      <name>Ltahoe_set_fsf</name>
      <anchorfile>phase1__opgen_8h.html</anchorfile>
      <anchor>d61083513e3a14f6648856b9a1c90313</anchor>
      <arglist>(L_Oper *oper, int fsf)</arglist>
    </member>
    <member kind="function">
      <type>int</type>
      <name>Ltahoe_get_fpc</name>
      <anchorfile>phase1__opgen_8h.html</anchorfile>
      <anchor>3f07d9ac5afb8d7372b571050d34b5ef</anchor>
      <arglist>(L_Oper *oper)</arglist>
    </member>
    <member kind="function">
      <type>void</type>
      <name>Ltahoe_set_fpc</name>
      <anchorfile>phase1__opgen_8h.html</anchorfile>
      <anchor>413b7a016b9d692e3f392255cf7d3a46</anchor>
      <arglist>(L_Oper *oper, int fpc)</arglist>
    </member>
  </compound>
  <compound kind="file">
    <name>phase1_param.c</name>
    <path>/mnt/hgfs/mkt/Desktop/openimpact/src/Lcode/codegen/Ltahoe/</path>
    <filename>phase1__param_8c</filename>
    <includes id="ltahoe__main_8h" name="ltahoe_main.h" local="yes" imported="no">ltahoe_main.h</includes>
    <includes id="ltahoe__op__query_8h" name="ltahoe_op_query.h" local="yes" imported="no">ltahoe_op_query.h</includes>
    <class kind="struct">ip_mac_subst_info</class>
    <member kind="define">
      <type>#define</type>
      <name>MAX_INPUT_PARAMS</name>
      <anchorfile>phase1__param_8c.html</anchorfile>
      <anchor>be855eab4fdf76e43d05c6135858253b</anchor>
      <arglist></arglist>
    </member>
    <member kind="typedef">
      <type>ip_mac_subst_info</type>
      <name>IP_MAC_SUBST_INFO</name>
      <anchorfile>phase1__param_8c.html</anchorfile>
      <anchor>dac8711d65bbd68267add286fe17ecd3</anchor>
      <arglist></arglist>
    </member>
    <member kind="function" static="yes">
      <type>static void</type>
      <name>check_input_params</name>
      <anchorfile>phase1__param_8c.html</anchorfile>
      <anchor>791bcb5424cd346c8974d371f9fc528b</anchor>
      <arglist>(L_Func *fn, IP_MAC_SUBST_INFO *ip_array, int ip_moves)</arglist>
    </member>
    <member kind="function" static="yes">
      <type>static int</type>
      <name>find_input_parameter_moves</name>
      <anchorfile>phase1__param_8c.html</anchorfile>
      <anchor>85991c7664ee5651fe95d36acdbf566a</anchor>
      <arglist>(L_Func *fn, IP_MAC_SUBST_INFO *ip_array)</arglist>
    </member>
    <member kind="function" static="yes">
      <type>static void</type>
      <name>free_ip_mov_array</name>
      <anchorfile>phase1__param_8c.html</anchorfile>
      <anchor>ae2804cd9608c4749fce18f81fb15e21</anchor>
      <arglist>(IP_MAC_SUBST_INFO *ip_array, int ip_moves)</arglist>
    </member>
    <member kind="function" static="yes">
      <type>static L_Operand *</type>
      <name>operand_match_any_ip_moves</name>
      <anchorfile>phase1__param_8c.html</anchorfile>
      <anchor>0a25d3ca45b6e0088ccbad749c4aa196</anchor>
      <arglist>(IP_MAC_SUBST_INFO *ip_array, int ip_moves, L_Operand *match_operand)</arglist>
    </member>
    <member kind="function" static="yes">
      <type>static void</type>
      <name>subst_input_macros</name>
      <anchorfile>phase1__param_8c.html</anchorfile>
      <anchor>ae2d5b9872b89acd3e2748a1a4d3e17f</anchor>
      <arglist>(L_Func *fn, IP_MAC_SUBST_INFO *ip_array, int ip_moves)</arglist>
    </member>
    <member kind="function">
      <type>void</type>
      <name>Ltahoe_ip_subst</name>
      <anchorfile>phase1__param_8c.html</anchorfile>
      <anchor>1dfc708f106bb25e6a042634fd2fa9f9</anchor>
      <arglist>(L_Func *fn)</arglist>
    </member>
  </compound>
  <compound kind="file">
    <name>phase1_param.h</name>
    <path>/mnt/hgfs/mkt/Desktop/openimpact/src/Lcode/codegen/Ltahoe/</path>
    <filename>phase1__param_8h</filename>
    <member kind="function">
      <type>void</type>
      <name>Ltahoe_ip_subst</name>
      <anchorfile>phase1__param_8h.html</anchorfile>
      <anchor>1dfc708f106bb25e6a042634fd2fa9f9</anchor>
      <arglist>(L_Func *fn)</arglist>
    </member>
  </compound>
  <compound kind="file">
    <name>phase1_pred.c</name>
    <path>/mnt/hgfs/mkt/Desktop/openimpact/src/Lcode/codegen/Ltahoe/</path>
    <filename>phase1__pred_8c</filename>
    <includes id="ltahoe__main_8h" name="ltahoe_main.h" local="yes" imported="no">ltahoe_main.h</includes>
    <includes id="phase1__func_8h" name="phase1_func.h" local="yes" imported="no">phase1_func.h</includes>
    <member kind="function" static="yes">
      <type>static void</type>
      <name>Ltahoe_split_lcode_pred_defines</name>
      <anchorfile>phase1__pred_8c.html</anchorfile>
      <anchor>e3b4d600af5939c53ef6346dc66300e8</anchor>
      <arglist>(L_Func *fn)</arglist>
    </member>
    <member kind="function" static="yes">
      <type>static int</type>
      <name>Ltahoe_compatible_ptypes</name>
      <anchorfile>phase1__pred_8c.html</anchorfile>
      <anchor>05e33c10be9b854fd4d14dd5e944bcd6</anchor>
      <arglist>(int pa, int pb)</arglist>
    </member>
    <member kind="function" static="yes">
      <type>static int</type>
      <name>Ltahoe_no_incongruent_defs_between</name>
      <anchorfile>phase1__pred_8c.html</anchorfile>
      <anchor>751810366affcb44e2e989fd15939cd4</anchor>
      <arglist>(L_Operand *operand, L_Oper *opA, L_Oper *opB)</arglist>
    </member>
    <member kind="function" static="yes">
      <type>static void</type>
      <name>Ltahoe_combine_lcode_compares</name>
      <anchorfile>phase1__pred_8c.html</anchorfile>
      <anchor>a081f09cb10c0037edcc6682d5a1f6f6</anchor>
      <arglist>(L_Func *fn)</arglist>
    </member>
    <member kind="function">
      <type>void</type>
      <name>Ltahoe_insert_pred_init</name>
      <anchorfile>phase1__pred_8c.html</anchorfile>
      <anchor>c0e26d163b8afd64b200cfe0d2ecda89</anchor>
      <arglist>(L_Cb *cb, int opc, int pred_reg_id)</arglist>
    </member>
    <member kind="function" static="yes">
      <type>static void</type>
      <name>Ltahoe_fixup_lcode_compares</name>
      <anchorfile>phase1__pred_8c.html</anchorfile>
      <anchor>79bae66e46d007ce1b0e8326ccf47d87</anchor>
      <arglist>(L_Func *fn)</arglist>
    </member>
    <member kind="function">
      <type>void</type>
      <name>Ltahoe_customize_lcode_compares</name>
      <anchorfile>phase1__pred_8c.html</anchorfile>
      <anchor>a3174af3923b29e15b361c6bb83ea0fe</anchor>
      <arglist>(L_Func *fn)</arglist>
    </member>
    <member kind="function">
      <type>void</type>
      <name>Ltahoe_combine_pred_inits</name>
      <anchorfile>phase1__pred_8c.html</anchorfile>
      <anchor>b63281edd3001990d5de26e63abae8f1</anchor>
      <arglist>(L_Func *fn)</arglist>
    </member>
  </compound>
  <compound kind="file">
    <name>phase1_varargs.c</name>
    <path>/mnt/hgfs/mkt/Desktop/openimpact/src/Lcode/codegen/Ltahoe/</path>
    <filename>phase1__varargs_8c</filename>
    <includes id="ltahoe__main_8h" name="ltahoe_main.h" local="yes" imported="no">ltahoe_main.h</includes>
    <includes id="ltahoe__op__query_8h" name="ltahoe_op_query.h" local="yes" imported="no">ltahoe_op_query.h</includes>
    <includes id="phase1__func_8h" name="phase1_func.h" local="yes" imported="no">phase1_func.h</includes>
    <includes id="phase1__varargs_8h" name="phase1_varargs.h" local="yes" imported="no">phase1_varargs.h</includes>
    <member kind="function">
      <type>int</type>
      <name>Ltahoe_is_vararg_func</name>
      <anchorfile>phase1__varargs_8c.html</anchorfile>
      <anchor>2be8c6cfda7054c063fc4f53c93063ac</anchor>
      <arglist>(L_Func *fn)</arglist>
    </member>
    <member kind="function" static="yes">
      <type>static void</type>
      <name>Ltahoe_insert_fp_param_moves_at_jsr</name>
      <anchorfile>phase1__varargs_8c.html</anchorfile>
      <anchor>06b64ea354fa36bc0896b96142d9942c</anchor>
      <arglist>(L_Cb *cb, L_Oper *jsr_oper)</arglist>
    </member>
    <member kind="function">
      <type>void</type>
      <name>Ltahoe_adjust_fp_parameter_passing</name>
      <anchorfile>phase1__varargs_8c.html</anchorfile>
      <anchor>0583ff175aff5568505662b1070ec4a8</anchor>
      <arglist>(L_Func *fn)</arglist>
    </member>
  </compound>
  <compound kind="file">
    <name>phase1_varargs.h</name>
    <path>/mnt/hgfs/mkt/Desktop/openimpact/src/Lcode/codegen/Ltahoe/</path>
    <filename>phase1__varargs_8h</filename>
    <member kind="function">
      <type>int</type>
      <name>Ltahoe_is_vararg_func</name>
      <anchorfile>phase1__varargs_8h.html</anchorfile>
      <anchor>2be8c6cfda7054c063fc4f53c93063ac</anchor>
      <arglist>(L_Func *fn)</arglist>
    </member>
    <member kind="function">
      <type>void</type>
      <name>Ltahoe_adjust_fp_parameter_passing</name>
      <anchorfile>phase1__varargs_8h.html</anchorfile>
      <anchor>0583ff175aff5568505662b1070ec4a8</anchor>
      <arglist>(L_Func *fn)</arglist>
    </member>
  </compound>
  <compound kind="file">
    <name>phase2_br_hint.c</name>
    <path>/mnt/hgfs/mkt/Desktop/openimpact/src/Lcode/codegen/Ltahoe/</path>
    <filename>phase2__br__hint_8c</filename>
    <includes id="ltahoe__main_8h" name="ltahoe_main.h" local="yes" imported="no">ltahoe_main.h</includes>
    <includes id="ltahoe__op__query_8h" name="ltahoe_op_query.h" local="yes" imported="no">ltahoe_op_query.h</includes>
    <includes id="ltahoe__completers_8h" name="ltahoe_completers.h" local="yes" imported="no">ltahoe_completers.h</includes>
    <includes id="phase1__func_8h" name="phase1_func.h" local="yes" imported="no">phase1_func.h</includes>
    <includes id="phase2__icache_8h" name="phase2_icache.h" local="yes" imported="no">phase2_icache.h</includes>
    <includes id="phase2__br__hint_8h" name="phase2_br_hint.h" local="yes" imported="no">phase2_br_hint.h</includes>
    <class kind="struct">_cb_info</class>
    <class kind="struct">_bh_hint_path</class>
    <class kind="struct">_bh_path_info</class>
    <class kind="struct">_bh_hints_list</class>
    <class kind="struct">_bh_br_hint</class>
    <member kind="define">
      <type>#define</type>
      <name>RECURSION_LEVEL</name>
      <anchorfile>phase2__br__hint_8c.html</anchorfile>
      <anchor>972df5ac079051b98ab0f2a0be78fdd3</anchor>
      <arglist></arglist>
    </member>
    <member kind="define">
      <type>#define</type>
      <name>TAR_WEIGHT_THRESHOLD</name>
      <anchorfile>phase2__br__hint_8c.html</anchorfile>
      <anchor>f23669fae50e80f87684d9133d831ba6</anchor>
      <arglist></arglist>
    </member>
    <member kind="define">
      <type>#define</type>
      <name>TAC_WEIGHT_THRESHOLD</name>
      <anchorfile>phase2__br__hint_8c.html</anchorfile>
      <anchor>70c239bbc2e70e5df92ab50e449fcc5e</anchor>
      <arglist></arglist>
    </member>
    <member kind="define">
      <type>#define</type>
      <name>MIN_HINTED_PATH_WEIGHT</name>
      <anchorfile>phase2__br__hint_8c.html</anchorfile>
      <anchor>30429387767476e7a40f6d1879b7ea42</anchor>
      <arglist></arglist>
    </member>
    <member kind="define">
      <type>#define</type>
      <name>TAR_PROB_THRESHOLD</name>
      <anchorfile>phase2__br__hint_8c.html</anchorfile>
      <anchor>6ca471fd8866f59a26c8f38e23d9fb23</anchor>
      <arglist></arglist>
    </member>
    <member kind="define">
      <type>#define</type>
      <name>TAC_PROB_THRESHOLD</name>
      <anchorfile>phase2__br__hint_8c.html</anchorfile>
      <anchor>4854e76194678c4b305aea7c0a73fa3b</anchor>
      <arglist></arglist>
    </member>
    <member kind="define">
      <type>#define</type>
      <name>MAX_BUNDLES_BETWEEN_HINT_AND_BR</name>
      <anchorfile>phase2__br__hint_8c.html</anchorfile>
      <anchor>d350485059d5ab8ca61e14de2682a973</anchor>
      <arglist></arglist>
    </member>
    <member kind="define">
      <type>#define</type>
      <name>MAX_INSERTION_DISTANCE</name>
      <anchorfile>phase2__br__hint_8c.html</anchorfile>
      <anchor>147a7c4118c3be8f76e5c1cdc9c183af</anchor>
      <arglist></arglist>
    </member>
    <member kind="define">
      <type>#define</type>
      <name>MAX_EXPANDS_PER_CB</name>
      <anchorfile>phase2__br__hint_8c.html</anchorfile>
      <anchor>57471e9b2516fed22e1863d1d881b7a9</anchor>
      <arglist></arglist>
    </member>
    <member kind="define">
      <type>#define</type>
      <name>MAX_INSERTS_PER_CB</name>
      <anchorfile>phase2__br__hint_8c.html</anchorfile>
      <anchor>57221ef4ecf999bd3626096f558fba4c</anchor>
      <arglist></arglist>
    </member>
    <member kind="define">
      <type>#define</type>
      <name>MAX_TARS_ON_PATH</name>
      <anchorfile>phase2__br__hint_8c.html</anchorfile>
      <anchor>f99aa6979924106c92fae28fedbc2832</anchor>
      <arglist></arglist>
    </member>
    <member kind="define">
      <type>#define</type>
      <name>BRP_MANY_HINT_MIN_SIZE</name>
      <anchorfile>phase2__br__hint_8c.html</anchorfile>
      <anchor>ef39cf5c6129ed4bd8d0f21639850064</anchor>
      <arglist></arglist>
    </member>
    <member kind="define">
      <type>#define</type>
      <name>BRP_CPREFETCH_MIN_SIZE</name>
      <anchorfile>phase2__br__hint_8c.html</anchorfile>
      <anchor>a63da9d2a1c0c47a57fb44b66dc52421</anchor>
      <arglist></arglist>
    </member>
    <member kind="define">
      <type>#define</type>
      <name>BRANCH_BUNDLE_COUNT_BIAS</name>
      <anchorfile>phase2__br__hint_8c.html</anchorfile>
      <anchor>31be034c3fce7cd200d2e4133163400e</anchor>
      <arglist></arglist>
    </member>
    <member kind="define">
      <type>#define</type>
      <name>PREFETCH_FE_CYCLES</name>
      <anchorfile>phase2__br__hint_8c.html</anchorfile>
      <anchor>a209fe790a0b6e08bc63f4e63fc508f2</anchor>
      <arglist></arglist>
    </member>
    <member kind="define">
      <type>#define</type>
      <name>PF_MANY_HINT_MIN_SIZE</name>
      <anchorfile>phase2__br__hint_8c.html</anchorfile>
      <anchor>e89ea2c91634526ce359c1ec625acd5e</anchor>
      <arglist></arglist>
    </member>
    <member kind="define">
      <type>#define</type>
      <name>L_is_br_indirect</name>
      <anchorfile>phase2__br__hint_8c.html</anchorfile>
      <anchor>fa327274058883e09d489860b707aaae</anchor>
      <arglist>(oper)</arglist>
    </member>
    <member kind="define">
      <type>#define</type>
      <name>DATAFILE</name>
      <anchorfile>phase2__br__hint_8c.html</anchorfile>
      <anchor>06e70837fbcaaca8503ffd61f0fa9c0a</anchor>
      <arglist></arglist>
    </member>
    <member kind="typedef">
      <type>_cb_info</type>
      <name>BH_CB_Info</name>
      <anchorfile>phase2__br__hint_8c.html</anchorfile>
      <anchor>d8b59762f77faffe243f38fd21d17dd5</anchor>
      <arglist></arglist>
    </member>
    <member kind="typedef">
      <type>_bh_hint_path</type>
      <name>BH_hint_path</name>
      <anchorfile>phase2__br__hint_8c.html</anchorfile>
      <anchor>77113edda58972beb1476165cd979b05</anchor>
      <arglist></arglist>
    </member>
    <member kind="typedef">
      <type>_bh_path_info</type>
      <name>BH_path_info</name>
      <anchorfile>phase2__br__hint_8c.html</anchorfile>
      <anchor>3443da2c90a890f0c4e3ce16a4734c94</anchor>
      <arglist></arglist>
    </member>
    <member kind="typedef">
      <type>_bh_hints_list</type>
      <name>BH_hints_list</name>
      <anchorfile>phase2__br__hint_8c.html</anchorfile>
      <anchor>257cded46be0e7737cdf59216fabf206</anchor>
      <arglist></arglist>
    </member>
    <member kind="typedef">
      <type>_bh_br_hint</type>
      <name>BH_Br_hint</name>
      <anchorfile>phase2__br__hint_8c.html</anchorfile>
      <anchor>a23c5a7f117d13e267fe8127aabea2b6</anchor>
      <arglist></arglist>
    </member>
    <member kind="function" static="yes">
      <type>static void</type>
      <name>BH_process_branches</name>
      <anchorfile>phase2__br__hint_8c.html</anchorfile>
      <anchor>70222a5a5bea23184b1a8053f3557a06</anchor>
      <arglist>(L_Func *fn)</arglist>
    </member>
    <member kind="function" static="yes">
      <type>static BH_Br_hint *</type>
      <name>BH_find_hint_instr</name>
      <anchorfile>phase2__br__hint_8c.html</anchorfile>
      <anchor>7a1bcf1a3577966fe4059b0a11c74227</anchor>
      <arglist>(L_Oper *oper, L_Oper *label_oper)</arglist>
    </member>
    <member kind="function" static="yes">
      <type>static void</type>
      <name>BH_add_to_existing_hint_list</name>
      <anchorfile>phase2__br__hint_8c.html</anchorfile>
      <anchor>81702fd61980fd42603ff741dd95a373</anchor>
      <arglist>(L_Cb *cb, L_Oper *oper)</arglist>
    </member>
    <member kind="function" static="yes">
      <type>static void</type>
      <name>BH_new_br_hint</name>
      <anchorfile>phase2__br__hint_8c.html</anchorfile>
      <anchor>3ad233c0fcbfbdd9668f6735832f4ac2</anchor>
      <arglist>(char *func_name, L_Cb *cb, L_Oper *br_oper, int num_bundles, int br_type, int ipwh, int ph, int pvec, int ih, float prob, float path_weight)</arglist>
    </member>
    <member kind="function" static="yes">
      <type>static void</type>
      <name>BH_delete_list_br_hints</name>
      <anchorfile>phase2__br__hint_8c.html</anchorfile>
      <anchor>872b92979809d8eea48d9d3ffa18b8de</anchor>
      <arglist>(BH_Br_hint *hint_list)</arglist>
    </member>
    <member kind="function" static="yes">
      <type>static BH_Br_hint **</type>
      <name>BH_prioritize_branch_hints</name>
      <anchorfile>phase2__br__hint_8c.html</anchorfile>
      <anchor>e6abfa2dd4f841f410d11569e4b116b2</anchor>
      <arglist>()</arglist>
    </member>
    <member kind="function">
      <type>int</type>
      <name>BH_hint_compare</name>
      <anchorfile>phase2__br__hint_8c.html</anchorfile>
      <anchor>862a6fecf10c829e6b1730d7e15f69bb</anchor>
      <arglist>(BH_Br_hint **hint1, BH_Br_hint **hint2)</arglist>
    </member>
    <member kind="function" static="yes">
      <type>static char *</type>
      <name>BH_hint_str</name>
      <anchorfile>phase2__br__hint_8c.html</anchorfile>
      <anchor>bd13f6992a26e2ee326b3e954fd1f2fd</anchor>
      <arglist>(BH_Br_hint *br_hint)</arglist>
    </member>
    <member kind="function" static="yes">
      <type>static void</type>
      <name>BH_print_hint_suggestions</name>
      <anchorfile>phase2__br__hint_8c.html</anchorfile>
      <anchor>b7e23e43715030faef0b33cee9fd8ed1</anchor>
      <arglist>(BH_Br_hint **hint_array, char *title)</arglist>
    </member>
    <member kind="function" static="yes">
      <type>static void</type>
      <name>free_CB_info</name>
      <anchorfile>phase2__br__hint_8c.html</anchorfile>
      <anchor>9322922e74a51df18483fc591d37ae56</anchor>
      <arglist>(void *ptr)</arglist>
    </member>
    <member kind="function" static="yes">
      <type>static int</type>
      <name>BH_cb_info</name>
      <anchorfile>phase2__br__hint_8c.html</anchorfile>
      <anchor>e7211d6d6bd03b0dd53901494504d0fa</anchor>
      <arglist>(int operation, L_Cb *cb1, L_Cb *cb2)</arglist>
    </member>
    <member kind="function" static="yes">
      <type>static int</type>
      <name>BH_static_distance</name>
      <anchorfile>phase2__br__hint_8c.html</anchorfile>
      <anchor>3c0f2ba63462e5121f7246e2f5514493</anchor>
      <arglist>(L_Cb *hint_cb, L_Oper *hint_oper, L_Cb *br_cb, L_Oper *br_oper, int *direction)</arglist>
    </member>
    <member kind="function" static="yes">
      <type>static int</type>
      <name>BH_insert_hint_in_paths</name>
      <anchorfile>phase2__br__hint_8c.html</anchorfile>
      <anchor>9a1417765a05c20d5b70c3f54b8e108f</anchor>
      <arglist>(L_Cb *cb, L_Oper *oper, BH_Br_hint *br_hint, BH_path_info *path_stats, int min_fe_cycles, int max_recursion, int indent, int test)</arglist>
    </member>
    <member kind="function" static="yes">
      <type>static int</type>
      <name>BH_insert_hint_in_block</name>
      <anchorfile>phase2__br__hint_8c.html</anchorfile>
      <anchor>1f579ec42cd8e9612296ba8b02284f61</anchor>
      <arglist>(L_Cb *cb, L_Oper *oper, BH_Br_hint *br_hint, BH_path_info *path_stats, int cycles_to_go, int indent, int test)</arglist>
    </member>
    <member kind="function" static="yes">
      <type>static int</type>
      <name>BH_insert_at_best_location</name>
      <anchorfile>phase2__br__hint_8c.html</anchorfile>
      <anchor>57d926c5ef82d0d886ca51dfa76fef5e</anchor>
      <arglist>(L_Cb *cb, L_Oper *top_tmpl, L_Oper *bottom_tmpl, BH_Br_hint *br_hint, BH_path_info *path_stats, int test)</arglist>
    </member>
    <member kind="function" static="yes">
      <type>static int</type>
      <name>BH_test_insert_hint_in_bundle</name>
      <anchorfile>phase2__br__hint_8c.html</anchorfile>
      <anchor>0166ae5a3b868882d2a9ce4383a4ab99</anchor>
      <arglist>(L_Cb *cb, L_Oper *bundle, BH_Br_hint *br_hint, BH_path_info *path_stats)</arglist>
    </member>
    <member kind="function" static="yes">
      <type>static int</type>
      <name>BH_insert_hint_in_bundle</name>
      <anchorfile>phase2__br__hint_8c.html</anchorfile>
      <anchor>eba8c6961cde0b97500512013b393fd0</anchor>
      <arglist>(L_Cb *cb, L_Oper *bundle, BH_Br_hint *br_hint, BH_path_info *path_stats)</arglist>
    </member>
    <member kind="function" static="yes">
      <type>static void</type>
      <name>BH_insert_branch_label</name>
      <anchorfile>phase2__br__hint_8c.html</anchorfile>
      <anchor>abf2fd161d0403e98a10bc04df0f970b</anchor>
      <arglist>(BH_Br_hint *br_hint)</arglist>
    </member>
    <member kind="function" static="yes">
      <type>static int</type>
      <name>BH_bundles_from_top</name>
      <anchorfile>phase2__br__hint_8c.html</anchorfile>
      <anchor>53d19d696e5ac1f8f5d87697b9cdf28f</anchor>
      <arglist>(L_Oper *oper)</arglist>
    </member>
    <member kind="function" static="yes">
      <type>static void</type>
      <name>BH_add_mov2br_instr_hints</name>
      <anchorfile>phase2__br__hint_8c.html</anchorfile>
      <anchor>3f1d33f0c5953d8040280c1327a1b9f4</anchor>
      <arglist>(L_Oper *oper, int mwh, int ph, int pvec, int ih)</arglist>
    </member>
    <member kind="function" static="yes">
      <type>static void</type>
      <name>BH_add_br_instr_hints</name>
      <anchorfile>phase2__br__hint_8c.html</anchorfile>
      <anchor>5a5ffd532cea3908ac52ea5d81484edc</anchor>
      <arglist>(L_Oper *oper, int bwh, int ph, int dh, float prob, float path_weight)</arglist>
    </member>
    <member kind="function" static="yes">
      <type>static void</type>
      <name>BH_add_cb_densities</name>
      <anchorfile>phase2__br__hint_8c.html</anchorfile>
      <anchor>a62bfa5359296445003e680e7e8f0dfa</anchor>
      <arglist>(L_Cb *cb)</arglist>
    </member>
    <member kind="function" static="yes">
      <type>static void</type>
      <name>BH_add_densities</name>
      <anchorfile>phase2__br__hint_8c.html</anchorfile>
      <anchor>bcd12c1f44c29b47101441129bbf8ebe</anchor>
      <arglist>(L_Func *fn)</arglist>
    </member>
    <member kind="function" static="yes">
      <type>static void</type>
      <name>BH_fix_flow_ccs</name>
      <anchorfile>phase2__br__hint_8c.html</anchorfile>
      <anchor>6d7e8decb4b799d5bc2141035bff4cea</anchor>
      <arglist>(L_Func *fn)</arglist>
    </member>
    <member kind="function" static="yes">
      <type>static void</type>
      <name>BH_add_instr_hints_only</name>
      <anchorfile>phase2__br__hint_8c.html</anchorfile>
      <anchor>c0bad6ab26428a7abf8f45e4c9b62444</anchor>
      <arglist>(L_Func *fn)</arglist>
    </member>
    <member kind="function" static="yes">
      <type>static void</type>
      <name>BH_specify_hint_on_mov2br</name>
      <anchorfile>phase2__br__hint_8c.html</anchorfile>
      <anchor>e1bdc1601a68cfceb101130d51b75d10</anchor>
      <arglist>(L_Cb *cb, L_Oper *oper)</arglist>
    </member>
    <member kind="function" static="yes">
      <type>static void</type>
      <name>BH_specify_hint_on_br</name>
      <anchorfile>phase2__br__hint_8c.html</anchorfile>
      <anchor>c09795a61bb3957a2b8bd22602a51866</anchor>
      <arglist>(L_Cb *cb, L_Oper *oper, L_Flow **flow, double *path_weight)</arglist>
    </member>
    <member kind="function" static="yes">
      <type>static void</type>
      <name>BH_suggest_hint_instr</name>
      <anchorfile>phase2__br__hint_8c.html</anchorfile>
      <anchor>f341a79bb46e20dab9962315f6ac7cb3</anchor>
      <arglist>(char *fn_name, L_Cb *cb, L_Oper *oper, L_Flow *flow, double path_weight)</arglist>
    </member>
    <member kind="function" static="yes">
      <type>static void</type>
      <name>BH_insert_branch_hints</name>
      <anchorfile>phase2__br__hint_8c.html</anchorfile>
      <anchor>c7274638f3e4087427d61585989ccd48</anchor>
      <arglist>(L_Func *fn, BH_Br_hint *hints[])</arglist>
    </member>
    <member kind="function" static="yes">
      <type>static void</type>
      <name>BH_insert_advanced_hint</name>
      <anchorfile>phase2__br__hint_8c.html</anchorfile>
      <anchor>2727779a276d2f8115346d0ef00eb1ac</anchor>
      <arglist>(BH_Br_hint *hint)</arglist>
    </member>
    <member kind="function" static="yes">
      <type>static void</type>
      <name>BH_print_stats</name>
      <anchorfile>phase2__br__hint_8c.html</anchorfile>
      <anchor>13e3dfb2371381a63678a84ad43e459b</anchor>
      <arglist>(BH_Br_hint **hints)</arglist>
    </member>
    <member kind="function" static="yes">
      <type>static void</type>
      <name>BH_copy_path_info</name>
      <anchorfile>phase2__br__hint_8c.html</anchorfile>
      <anchor>b4cbe5d466bae03827ad4ccae1970a61</anchor>
      <arglist>(BH_path_info *dest, BH_path_info *src)</arglist>
    </member>
    <member kind="function" static="yes">
      <type>static void</type>
      <name>BH_add_to_hint_list</name>
      <anchorfile>phase2__br__hint_8c.html</anchorfile>
      <anchor>6b0a3c65c4c67dfe4685ca9d9c131d05</anchor>
      <arglist>(BH_Br_hint *br_hint, BH_path_info *stats, L_Oper *hint_oper, L_Cb *hint_cb)</arglist>
    </member>
    <member kind="function" static="yes">
      <type>static void</type>
      <name>BH_add_cb_to_path</name>
      <anchorfile>phase2__br__hint_8c.html</anchorfile>
      <anchor>cd972cc0018c55993dfebe93063f6e19</anchor>
      <arglist>(L_Cb *cb, BH_path_info *path_info)</arglist>
    </member>
    <member kind="function" static="yes">
      <type>static int</type>
      <name>BH_ok_to_expand</name>
      <anchorfile>phase2__br__hint_8c.html</anchorfile>
      <anchor>11954113a140deb5f756acedda4fcce3</anchor>
      <arglist>(L_Cb *cb, L_Oper *oper, BH_Br_hint *br_hint)</arglist>
    </member>
    <member kind="function" static="yes">
      <type>static int</type>
      <name>BH_ok_to_insert</name>
      <anchorfile>phase2__br__hint_8c.html</anchorfile>
      <anchor>be9cae86881bfabf093c337e65995fa3</anchor>
      <arglist>(L_Cb *cb, BH_Br_hint *br_hint)</arglist>
    </member>
    <member kind="function" static="yes">
      <type>static void</type>
      <name>BH_insert_update</name>
      <anchorfile>phase2__br__hint_8c.html</anchorfile>
      <anchor>8e73cb1be939c8683c86a9694268f74c</anchor>
      <arglist>(L_Cb *cb, L_Oper *oper, BH_Br_hint *br_hint, int expanded)</arglist>
    </member>
    <member kind="function" static="yes">
      <type>static int</type>
      <name>BH_is_on_static_path</name>
      <anchorfile>phase2__br__hint_8c.html</anchorfile>
      <anchor>28e069c89d19740adf9eb2b0f0efd02a</anchor>
      <arglist>(L_Cb *cb, L_Oper *oper, L_Cb *hint_cb, L_Oper *hint_oper, L_Cb *br_cb, L_Oper *br_oper)</arglist>
    </member>
    <member kind="function" static="yes">
      <type>static int</type>
      <name>BH_is_on_dynamic_path</name>
      <anchorfile>phase2__br__hint_8c.html</anchorfile>
      <anchor>b4e76d262898dda93fb79b4938369d74</anchor>
      <arglist>(L_Cb *cb, BH_hints_list *hint_list)</arglist>
    </member>
    <member kind="function">
      <type>void</type>
      <name>O_count_bundles_before_branch_func</name>
      <anchorfile>phase2__br__hint_8c.html</anchorfile>
      <anchor>472043d102e6e73a6c36265747f57202</anchor>
      <arglist>(L_Func *fn)</arglist>
    </member>
    <member kind="function">
      <type>int</type>
      <name>O_count_bundles_before_branch</name>
      <anchorfile>phase2__br__hint_8c.html</anchorfile>
      <anchor>2da5ef7d2df307a0b2f579bb9417be08</anchor>
      <arglist>(L_Cb *starting_cb, L_Oper *starting_oper)</arglist>
    </member>
    <member kind="function">
      <type>int</type>
      <name>BH_num_bundles_in_call</name>
      <anchorfile>phase2__br__hint_8c.html</anchorfile>
      <anchor>203c4fdbb2da3f12df9f50e75298b15e</anchor>
      <arglist>(L_Oper *br_call)</arglist>
    </member>
    <member kind="function">
      <type>int</type>
      <name>BH_num_bundles_in_branch</name>
      <anchorfile>phase2__br__hint_8c.html</anchorfile>
      <anchor>072c133d388a74be483488c87c0af792</anchor>
      <arglist>(L_Oper *br_cond)</arglist>
    </member>
    <member kind="function">
      <type>void</type>
      <name>O_insert_branch_hints</name>
      <anchorfile>phase2__br__hint_8c.html</anchorfile>
      <anchor>f5d42f7fe70534108a5dea337fb11296</anchor>
      <arglist>(L_Func *fn)</arglist>
    </member>
    <member kind="function">
      <type>void</type>
      <name>O_insert_br_instr_hints_only</name>
      <anchorfile>phase2__br__hint_8c.html</anchorfile>
      <anchor>4b921c062341b082055ff1514566dd94</anchor>
      <arglist>(L_Func *fn)</arglist>
    </member>
    <member kind="function" static="yes">
      <type>static BH_Br_hint *</type>
      <name>BH_new_advanced_hint</name>
      <anchorfile>phase2__br__hint_8c.html</anchorfile>
      <anchor>0a2957970e6b3bf69c14ce0186ac775f</anchor>
      <arglist>(char *func_name, L_Cb *cb, L_Oper *br_oper, int num_bundles, int br_type, int ipwh, int ph, int pvec, int ih, float prob, float path_weight)</arglist>
    </member>
    <member kind="function" static="yes">
      <type>static L_Cb *</type>
      <name>BH_split_cb</name>
      <anchorfile>phase2__br__hint_8c.html</anchorfile>
      <anchor>9e1d98bb3d603eb0a976351c9e344eed</anchor>
      <arglist>(L_Cb *cb, L_Oper *oper)</arglist>
    </member>
    <member kind="function" static="yes">
      <type>static void</type>
      <name>BH_print_stats</name>
      <anchorfile>phase2__br__hint_8c.html</anchorfile>
      <anchor>a2504fbd050421c29540663d9380a790</anchor>
      <arglist>(BH_Br_hint *hints[])</arglist>
    </member>
    <member kind="variable" static="yes">
      <type>static BH_Br_hint **</type>
      <name>sorted_hints</name>
      <anchorfile>phase2__br__hint_8c.html</anchorfile>
      <anchor>a55d6b818b5da2614d9b25ce002bab85</anchor>
      <arglist></arglist>
    </member>
    <member kind="variable" static="yes">
      <type>static BH_Br_hint *</type>
      <name>list_br_hints</name>
      <anchorfile>phase2__br__hint_8c.html</anchorfile>
      <anchor>0daf5e8976b685326e53123e2e4e4bb2</anchor>
      <arglist></arglist>
    </member>
    <member kind="variable" static="yes">
      <type>static BH_Br_hint *</type>
      <name>existing_br_hints</name>
      <anchorfile>phase2__br__hint_8c.html</anchorfile>
      <anchor>cb4f0082cfbdb1853176f652f8a68920</anchor>
      <arglist></arglist>
    </member>
    <member kind="variable" static="yes">
      <type>static int</type>
      <name>br_hint_cnt</name>
      <anchorfile>phase2__br__hint_8c.html</anchorfile>
      <anchor>e15082bea90625de9cbcce3a67ba52d4</anchor>
      <arglist></arglist>
    </member>
    <member kind="variable" static="yes">
      <type>static int</type>
      <name>advanced_br_hint_cnt</name>
      <anchorfile>phase2__br__hint_8c.html</anchorfile>
      <anchor>6e76ea7e64f5bbc7a309ccd8585eb0d4</anchor>
      <arglist></arglist>
    </member>
    <member kind="variable" static="yes">
      <type>static int</type>
      <name>existing_br_hint_cnt</name>
      <anchorfile>phase2__br__hint_8c.html</anchorfile>
      <anchor>f70029448a9f38d14c61a6c00222f7d2</anchor>
      <arglist></arglist>
    </member>
    <member kind="variable" static="yes">
      <type>static Set</type>
      <name>cbs_looked_at_set</name>
      <anchorfile>phase2__br__hint_8c.html</anchorfile>
      <anchor>a113963425dd4958be57f397c9f47a97</anchor>
      <arglist></arglist>
    </member>
    <member kind="variable" static="yes">
      <type>static int</type>
      <name>num_branches</name>
      <anchorfile>phase2__br__hint_8c.html</anchorfile>
      <anchor>9ab17ee75c83480d31a3ac14395b3e26</anchor>
      <arglist></arglist>
    </member>
    <member kind="variable" static="yes">
      <type>static int</type>
      <name>hightakebranches</name>
      <anchorfile>phase2__br__hint_8c.html</anchorfile>
      <anchor>d53dd5f1e604cf1390405aaa3c854e26</anchor>
      <arglist></arglist>
    </member>
    <member kind="variable" static="yes">
      <type>static int</type>
      <name>lowweighttossed</name>
      <anchorfile>phase2__br__hint_8c.html</anchorfile>
      <anchor>3b4139a7543793ae0697d31c90162fbc</anchor>
      <arglist></arglist>
    </member>
    <member kind="variable" static="yes">
      <type>static int</type>
      <name>calls</name>
      <anchorfile>phase2__br__hint_8c.html</anchorfile>
      <anchor>5f1c773ac1d2f42241d25d73373885ab</anchor>
      <arglist></arglist>
    </member>
    <member kind="variable" static="yes">
      <type>static int</type>
      <name>conds</name>
      <anchorfile>phase2__br__hint_8c.html</anchorfile>
      <anchor>7829a4f6c9aeeeff6beae3eeebdf9422</anchor>
      <arglist></arglist>
    </member>
    <member kind="variable" static="yes">
      <type>static int</type>
      <name>returns</name>
      <anchorfile>phase2__br__hint_8c.html</anchorfile>
      <anchor>72aa4703e4c9576c3a19bd221b0f274d</anchor>
      <arglist></arglist>
    </member>
    <member kind="variable" static="yes">
      <type>static int</type>
      <name>total_attempted</name>
      <anchorfile>phase2__br__hint_8c.html</anchorfile>
      <anchor>5d9fc5f16a57a87c24cfb28708623439</anchor>
      <arglist></arglist>
    </member>
    <member kind="variable" static="yes">
      <type>static int</type>
      <name>total_inserted</name>
      <anchorfile>phase2__br__hint_8c.html</anchorfile>
      <anchor>f8ebcbca24fd44f12e76c50577d4de8d</anchor>
      <arglist></arglist>
    </member>
    <member kind="variable" static="yes">
      <type>static int</type>
      <name>stat_max_recursion</name>
      <anchorfile>phase2__br__hint_8c.html</anchorfile>
      <anchor>0e27052e62018e6bedebba7bc18beb12</anchor>
      <arglist></arglist>
    </member>
    <member kind="variable" static="yes">
      <type>static int</type>
      <name>stat_cb_too_far</name>
      <anchorfile>phase2__br__hint_8c.html</anchorfile>
      <anchor>837d372c59b4c0d45f1000403da76a23</anchor>
      <arglist></arglist>
    </member>
    <member kind="variable" static="yes">
      <type>static int</type>
      <name>stat_merged_path</name>
      <anchorfile>phase2__br__hint_8c.html</anchorfile>
      <anchor>04c0ffb28d179186b7d9eca9d681abdb</anchor>
      <arglist></arglist>
    </member>
    <member kind="variable" static="yes">
      <type>static int</type>
      <name>stat_tac_hints</name>
      <anchorfile>phase2__br__hint_8c.html</anchorfile>
      <anchor>300e4c89f45dbbfa2d6d94816d4b65ce</anchor>
      <arglist></arglist>
    </member>
    <member kind="variable" static="yes">
      <type>static int</type>
      <name>stat_tar_hints</name>
      <anchorfile>phase2__br__hint_8c.html</anchorfile>
      <anchor>fb8dcabd7ffacdd118dd88b45ee084ae</anchor>
      <arglist></arglist>
    </member>
    <member kind="variable" static="yes">
      <type>static int</type>
      <name>stat_max_expand</name>
      <anchorfile>phase2__br__hint_8c.html</anchorfile>
      <anchor>05d4f8bd757fc2246a8ca845b75c13ef</anchor>
      <arglist></arglist>
    </member>
    <member kind="variable" static="yes">
      <type>static int</type>
      <name>stat_max_inserted</name>
      <anchorfile>phase2__br__hint_8c.html</anchorfile>
      <anchor>dc1728f5776e0bb4e90586bf00606d52</anchor>
      <arglist></arglist>
    </member>
    <member kind="variable" static="yes">
      <type>static int</type>
      <name>hint_fit_table</name>
      <anchorfile>phase2__br__hint_8c.html</anchorfile>
      <anchor>818bedc8e2a6d0236da32245b9f16823</anchor>
      <arglist>[][8]</arglist>
    </member>
    <member kind="variable" static="yes">
      <type>static int</type>
      <name>pvec_table</name>
      <anchorfile>phase2__br__hint_8c.html</anchorfile>
      <anchor>cc20520e5af2028152b6629a53883524</anchor>
      <arglist>[3][3]</arglist>
    </member>
  </compound>
  <compound kind="file">
    <name>phase2_br_hint.h</name>
    <path>/mnt/hgfs/mkt/Desktop/openimpact/src/Lcode/codegen/Ltahoe/</path>
    <filename>phase2__br__hint_8h</filename>
    <member kind="define">
      <type>#define</type>
      <name>PVEC_NONE</name>
      <anchorfile>phase2__br__hint_8h.html</anchorfile>
      <anchor>08e662629650bf97f191f35e65c50ca2</anchor>
      <arglist></arglist>
    </member>
    <member kind="enumeration">
      <name>BTYPE_e</name>
      <anchor>af410f05957b8370def85b551f023f9c</anchor>
      <arglist></arglist>
    </member>
    <member kind="enumvalue">
      <name>BR_COND</name>
      <anchor>af410f05957b8370def85b551f023f9c03d4e9d1e4a5b64c7046568a63d8aa73</anchor>
      <arglist></arglist>
    </member>
    <member kind="enumvalue">
      <name>BR_CALL</name>
      <anchor>af410f05957b8370def85b551f023f9c9b4f0d9e6cef96c601d74635c1af46d6</anchor>
      <arglist></arglist>
    </member>
    <member kind="enumvalue">
      <name>BR_ADV</name>
      <anchor>af410f05957b8370def85b551f023f9c30cfa576a7f412f1bed97650cc1d6464</anchor>
      <arglist></arglist>
    </member>
    <member kind="enumvalue">
      <name>INSERTED_HINT</name>
      <anchor>99fb83031ce9923c84392b4e92f956b5fa64b4fcdfca42995b62816f39b18970</anchor>
      <arglist></arglist>
    </member>
    <member kind="enumvalue">
      <name>MISS_IN_CB</name>
      <anchor>99fb83031ce9923c84392b4e92f956b539bda4f0d9c31e89080d538bba203f68</anchor>
      <arglist></arglist>
    </member>
    <member kind="enumvalue">
      <name>INSERT_FAILED</name>
      <anchor>99fb83031ce9923c84392b4e92f956b5fdc55146c8e47e63dbdb3ec0341f4a70</anchor>
      <arglist></arglist>
    </member>
    <member kind="enumvalue">
      <name>BRP_TO_BR</name>
      <anchor>bc6126af1d45847bc59afa0aa3216b04dbeb3e5ca6f2fa2bd2ce82f07f6852cc</anchor>
      <arglist></arglist>
    </member>
    <member kind="enumvalue">
      <name>BR_TO_BRP</name>
      <anchor>bc6126af1d45847bc59afa0aa3216b044dad561a924b10d79664f677cf99c676</anchor>
      <arglist></arglist>
    </member>
    <member kind="enumvalue">
      <name>BUNDLES_IN_CB</name>
      <anchor>dc29c2ff13d900c2f185ee95427fb06c6578d484bedb13b2fbe33504d90a280d</anchor>
      <arglist></arglist>
    </member>
    <member kind="enumvalue">
      <name>INITIALIZE</name>
      <anchor>dc29c2ff13d900c2f185ee95427fb06cb0c48e95b68bff1f7820e23b8fd0bc98</anchor>
      <arglist></arglist>
    </member>
    <member kind="enumvalue">
      <name>CLEANUP</name>
      <anchor>dc29c2ff13d900c2f185ee95427fb06cd8b65b0d2820072fd0e88edd889085fa</anchor>
      <arglist></arglist>
    </member>
    <member kind="enumvalue">
      <name>UPDATE</name>
      <anchor>dc29c2ff13d900c2f185ee95427fb06c3912ed627c0090ccc7fa1b03fef04202</anchor>
      <arglist></arglist>
    </member>
    <member kind="enumvalue">
      <name>BETWEEN</name>
      <anchor>dc29c2ff13d900c2f185ee95427fb06c8a86033ada1240a0576a1ce24d8d307b</anchor>
      <arglist></arglist>
    </member>
    <member kind="enumvalue">
      <name>NUM_EXPANDED</name>
      <anchor>dc29c2ff13d900c2f185ee95427fb06c598d3f185a3f4252f417bb2e0ce31db2</anchor>
      <arglist></arglist>
    </member>
    <member kind="enumvalue">
      <name>NUM_INSERTED</name>
      <anchor>dc29c2ff13d900c2f185ee95427fb06ce627f20915348b2c50637fa90f0bfcbd</anchor>
      <arglist></arglist>
    </member>
    <member kind="enumvalue">
      <name>ADD_EXPANDED</name>
      <anchor>dc29c2ff13d900c2f185ee95427fb06c5bf1a73b8b8e783d8b22c00464e8397f</anchor>
      <arglist></arglist>
    </member>
    <member kind="enumvalue">
      <name>ADD_INSERTED</name>
      <anchor>dc29c2ff13d900c2f185ee95427fb06c3dfb4fc3e35b57293940cd3fecf1d7b1</anchor>
      <arglist></arglist>
    </member>
    <member kind="enumeration">
      <name>MWH_e</name>
      <anchor>c2dffca4d43d41b8c6e54e4b7195a048</anchor>
      <arglist></arglist>
    </member>
    <member kind="enumvalue">
      <name>MWH_NONE</name>
      <anchor>c2dffca4d43d41b8c6e54e4b7195a048c8443b72bb9e08f637746297116dd73b</anchor>
      <arglist></arglist>
    </member>
    <member kind="enumvalue">
      <name>MWH_SPTK</name>
      <anchor>c2dffca4d43d41b8c6e54e4b7195a048326363a55068a303eba89b5e9e6d9815</anchor>
      <arglist></arglist>
    </member>
    <member kind="enumvalue">
      <name>MWH_DPTK</name>
      <anchor>c2dffca4d43d41b8c6e54e4b7195a048afade4c0f1bcb66002a843ea59693636</anchor>
      <arglist></arglist>
    </member>
    <member kind="enumeration">
      <name>BWH_e</name>
      <anchor>97b49211a816e38aee8f91199e1917c9</anchor>
      <arglist></arglist>
    </member>
    <member kind="enumvalue">
      <name>BWH_SPNT</name>
      <anchor>97b49211a816e38aee8f91199e1917c9b51a5f439a31f4c2970970d73187bc9f</anchor>
      <arglist></arglist>
    </member>
    <member kind="enumvalue">
      <name>BWH_SPTK</name>
      <anchor>97b49211a816e38aee8f91199e1917c99ccd8b89e19a3dec5736ec9b1d8e0df7</anchor>
      <arglist></arglist>
    </member>
    <member kind="enumvalue">
      <name>BWH_DPNT</name>
      <anchor>97b49211a816e38aee8f91199e1917c9789f32463cc668b1eb3a83dc9095b084</anchor>
      <arglist></arglist>
    </member>
    <member kind="enumvalue">
      <name>BWH_DPTK</name>
      <anchor>97b49211a816e38aee8f91199e1917c9646275572e8cabc4be592cc1f78f6f6e</anchor>
      <arglist></arglist>
    </member>
    <member kind="enumeration">
      <name>PH_e</name>
      <anchor>44f41f58b99dde8471332e6d0b5e352c</anchor>
      <arglist></arglist>
    </member>
    <member kind="enumvalue">
      <name>PH_NONE</name>
      <anchor>44f41f58b99dde8471332e6d0b5e352ce3f7a29958ecc7d45ab585ce89946c68</anchor>
      <arglist></arglist>
    </member>
    <member kind="enumvalue">
      <name>PH_FEW</name>
      <anchor>44f41f58b99dde8471332e6d0b5e352c50215b1648e04df0ef9dc2cfa5a98fa1</anchor>
      <arglist></arglist>
    </member>
    <member kind="enumvalue">
      <name>PH_MANY</name>
      <anchor>44f41f58b99dde8471332e6d0b5e352c4ed7724a37f4e4926e891e70abd39c62</anchor>
      <arglist></arglist>
    </member>
    <member kind="enumeration">
      <name>DH_e</name>
      <anchor>277a82451cc57a59110dfa65a3f870a1</anchor>
      <arglist></arglist>
    </member>
    <member kind="enumvalue">
      <name>DH_NONE</name>
      <anchor>277a82451cc57a59110dfa65a3f870a1aa823a7c39b6586b4be8e68324e6661f</anchor>
      <arglist></arglist>
    </member>
    <member kind="enumvalue">
      <name>DH_CLR</name>
      <anchor>277a82451cc57a59110dfa65a3f870a154c79605258fb85d23812b06d3be2484</anchor>
      <arglist></arglist>
    </member>
    <member kind="enumeration">
      <name>IPWH_e</name>
      <anchor>9a726c7dbc033c1bc519a01ea6d5227b</anchor>
      <arglist></arglist>
    </member>
    <member kind="enumvalue">
      <name>IPWH_SPTK</name>
      <anchor>9a726c7dbc033c1bc519a01ea6d5227b668588fa73c16bc6c21b1e8af720b7a1</anchor>
      <arglist></arglist>
    </member>
    <member kind="enumvalue">
      <name>IPWH_LOOP</name>
      <anchor>9a726c7dbc033c1bc519a01ea6d5227b47b9637cc42c7f8dd7c4fe2580039722</anchor>
      <arglist></arglist>
    </member>
    <member kind="enumvalue">
      <name>IPWH_EXIT</name>
      <anchor>9a726c7dbc033c1bc519a01ea6d5227b8471abb948fc5f0e9c3c2b0836d5b686</anchor>
      <arglist></arglist>
    </member>
    <member kind="enumvalue">
      <name>IPWH_DPTK</name>
      <anchor>9a726c7dbc033c1bc519a01ea6d5227b88e9bb7ad8945d767917e5a31ff9aa77</anchor>
      <arglist></arglist>
    </member>
    <member kind="enumeration">
      <name>IPDWH_e</name>
      <anchor>a1ccfef9237923cedbf2d71fb7bccc89</anchor>
      <arglist></arglist>
    </member>
    <member kind="enumvalue">
      <name>INDWH_SPTK</name>
      <anchor>a1ccfef9237923cedbf2d71fb7bccc89e5ece04428b6d4d0699b1caaa905020f</anchor>
      <arglist></arglist>
    </member>
    <member kind="enumvalue">
      <name>INDWH_DPTK</name>
      <anchor>a1ccfef9237923cedbf2d71fb7bccc89a4bce4875dd373c6f059a70b974be702</anchor>
      <arglist></arglist>
    </member>
    <member kind="enumeration">
      <name>PVEC_e</name>
      <anchor>51eb5be333defd59f0d448df9d32a1d8</anchor>
      <arglist></arglist>
    </member>
    <member kind="enumvalue">
      <name>PVEC_DC_DC</name>
      <anchor>51eb5be333defd59f0d448df9d32a1d8606e0e705ddc4dc6aaf6123b5c3cd306</anchor>
      <arglist></arglist>
    </member>
    <member kind="enumvalue">
      <name>PVEC_DC_NT</name>
      <anchor>51eb5be333defd59f0d448df9d32a1d843667f28a8b028ffb915588cda420420</anchor>
      <arglist></arglist>
    </member>
    <member kind="enumvalue">
      <name>PVEC_TK_DC</name>
      <anchor>51eb5be333defd59f0d448df9d32a1d807af4b36693808c71f7d1a680bcdd23c</anchor>
      <arglist></arglist>
    </member>
    <member kind="enumvalue">
      <name>PVEC_TK_TK</name>
      <anchor>51eb5be333defd59f0d448df9d32a1d87d7a69297df926077812085bf901e059</anchor>
      <arglist></arglist>
    </member>
    <member kind="enumvalue">
      <name>PVEC_TK_NT</name>
      <anchor>51eb5be333defd59f0d448df9d32a1d80c53e38ceb9c9038ce41e3273d072693</anchor>
      <arglist></arglist>
    </member>
    <member kind="enumvalue">
      <name>PVEC_NT_DC</name>
      <anchor>51eb5be333defd59f0d448df9d32a1d88ec4281237dc4ebbb6f0f771ef7482a5</anchor>
      <arglist></arglist>
    </member>
    <member kind="enumvalue">
      <name>PVEC_NT_TK</name>
      <anchor>51eb5be333defd59f0d448df9d32a1d84cf7d765635005ec280e96e9d9b3f509</anchor>
      <arglist></arglist>
    </member>
    <member kind="enumvalue">
      <name>PVEC_NT_NT</name>
      <anchor>51eb5be333defd59f0d448df9d32a1d872f4c562636bfec0dad4aa736b3c13da</anchor>
      <arglist></arglist>
    </member>
    <member kind="enumeration">
      <name>IH_e</name>
      <anchor>f7b1a590c4efec17b1aa96c3926465d8</anchor>
      <arglist></arglist>
    </member>
    <member kind="enumvalue">
      <name>IH_NONE</name>
      <anchor>f7b1a590c4efec17b1aa96c3926465d8a0974f5648cecc92fbab3f7cbfc551bd</anchor>
      <arglist></arglist>
    </member>
    <member kind="enumvalue">
      <name>IH_IMP</name>
      <anchor>f7b1a590c4efec17b1aa96c3926465d81dd86843b8e2433b08eb6f3fa1c614f8</anchor>
      <arglist></arglist>
    </member>
    <member kind="function">
      <type>void</type>
      <name>O_count_bundles_before_branch_func</name>
      <anchorfile>phase2__br__hint_8h.html</anchorfile>
      <anchor>472043d102e6e73a6c36265747f57202</anchor>
      <arglist>(L_Func *fn)</arglist>
    </member>
    <member kind="function">
      <type>int</type>
      <name>O_count_bundles_before_branch</name>
      <anchorfile>phase2__br__hint_8h.html</anchorfile>
      <anchor>2da5ef7d2df307a0b2f579bb9417be08</anchor>
      <arglist>(L_Cb *starting_cb, L_Oper *starting_oper)</arglist>
    </member>
    <member kind="function">
      <type>void</type>
      <name>O_insert_branch_hints</name>
      <anchorfile>phase2__br__hint_8h.html</anchorfile>
      <anchor>f5d42f7fe70534108a5dea337fb11296</anchor>
      <arglist>(L_Func *fn)</arglist>
    </member>
    <member kind="function">
      <type>void</type>
      <name>O_insert_br_instr_hints_only</name>
      <anchorfile>phase2__br__hint_8h.html</anchorfile>
      <anchor>4b921c062341b082055ff1514566dd94</anchor>
      <arglist>(L_Func *fn)</arglist>
    </member>
    <member kind="variable">
      <type>double</type>
      <name>Ltahoe_dp_upper_prob</name>
      <anchorfile>phase2__br__hint_8h.html</anchorfile>
      <anchor>4cc48d0f69cf60e3c327a96998f39159</anchor>
      <arglist></arglist>
    </member>
    <member kind="variable">
      <type>double</type>
      <name>Ltahoe_dp_lower_prob</name>
      <anchorfile>phase2__br__hint_8h.html</anchorfile>
      <anchor>670a82d24bf8f372280f80dfe7354426</anchor>
      <arglist></arglist>
    </member>
  </compound>
  <compound kind="file">
    <name>phase2_func.c</name>
    <path>/mnt/hgfs/mkt/Desktop/openimpact/src/Lcode/codegen/Ltahoe/</path>
    <filename>phase2__func_8c</filename>
    <includes id="ltahoe__main_8h" name="ltahoe_main.h" local="yes" imported="no">ltahoe_main.h</includes>
    <includes id="ltahoe__op__query_8h" name="ltahoe_op_query.h" local="yes" imported="no">ltahoe_op_query.h</includes>
    <includes id="ltahoe__completers_8h" name="ltahoe_completers.h" local="yes" imported="no">ltahoe_completers.h</includes>
    <includes id="phase1__opgen_8h" name="phase1_opgen.h" local="yes" imported="no">phase1_opgen.h</includes>
    <includes id="phase1__func_8h" name="phase1_func.h" local="yes" imported="no">phase1_func.h</includes>
    <includes id="phase2__reg_8h" name="phase2_reg.h" local="yes" imported="no">phase2_reg.h</includes>
    <includes id="phase2__br__hint_8h" name="phase2_br_hint.h" local="yes" imported="no">phase2_br_hint.h</includes>
    <includes id="phase2__icache_8h" name="phase2_icache.h" local="yes" imported="no">phase2_icache.h</includes>
    <includes id="phase2__sync_8h" name="phase2_sync.h" local="yes" imported="no">phase2_sync.h</includes>
    <includes id="phase2__memstk_8h" name="phase2_memstk.h" local="yes" imported="no">phase2_memstk.h</includes>
    <includes id="phase2__func_8h" name="phase2_func.h" local="yes" imported="no">phase2_func.h</includes>
    <member kind="function" static="yes">
      <type>static void</type>
      <name>O_mark_speculative</name>
      <anchorfile>phase2__func_8c.html</anchorfile>
      <anchor>56cbea3dc6ae67969e90a7fd3d1f1168</anchor>
      <arglist>(L_Func *fn)</arglist>
    </member>
    <member kind="function">
      <type>void</type>
      <name>L_check_for_special_dep_violation</name>
      <anchorfile>phase2__func_8c.html</anchorfile>
      <anchor>4aece95dd0a5ee548818842e7548a224</anchor>
      <arglist>(L_Func *fn)</arglist>
    </member>
    <member kind="function">
      <type>void</type>
      <name>Ltahoe_sp_removal</name>
      <anchorfile>phase2__func_8c.html</anchorfile>
      <anchor>e0353a5a2cfdbda767b64c195fc459b2</anchor>
      <arglist>(L_Func *fn)</arglist>
    </member>
    <member kind="function">
      <type>int</type>
      <name>Ltahoe_remove_dead_pred_defs</name>
      <anchorfile>phase2__func_8c.html</anchorfile>
      <anchor>671e5d468ac0bb392112f066f8066f5a</anchor>
      <arglist>(L_Func *fn)</arglist>
    </member>
    <member kind="function">
      <type>void</type>
      <name>Ltahoe_repair_epilogue_defines</name>
      <anchorfile>phase2__func_8c.html</anchorfile>
      <anchor>eb4ddcc158e52a258344643b628b3e0c</anchor>
      <arglist>(L_Cb *cb)</arglist>
    </member>
    <member kind="function">
      <type>void</type>
      <name>Ltahoe_output_dependence_stall_removal</name>
      <anchorfile>phase2__func_8c.html</anchorfile>
      <anchor>0d3b9c61a6dc51cc0c65c47dea28bc71</anchor>
      <arglist>(L_Func *fn)</arglist>
    </member>
    <member kind="function" static="yes">
      <type>static int</type>
      <name>Ltahoe_mark_swp_spills</name>
      <anchorfile>phase2__func_8c.html</anchorfile>
      <anchor>6900cfaf30be504bc370023192520e2d</anchor>
      <arglist>(L_Func *fn)</arglist>
    </member>
    <member kind="function">
      <type>void</type>
      <name>Ltahoe_estimate_liveness</name>
      <anchorfile>phase2__func_8c.html</anchorfile>
      <anchor>25d417eb85b2835f7968b2899d928d7c</anchor>
      <arglist>(L_Func *fn, char *str)</arglist>
    </member>
    <member kind="function">
      <type>void</type>
      <name>Ltahoe_promote_eff_uncond_br</name>
      <anchorfile>phase2__func_8c.html</anchorfile>
      <anchor>3cbd8b6194b0610e8492209c2187c039</anchor>
      <arglist>(L_Func *fn)</arglist>
    </member>
    <member kind="function">
      <type>void</type>
      <name>O_red_load</name>
      <anchorfile>phase2__func_8c.html</anchorfile>
      <anchor>17b65596944c0d11ee88f46caf3fcedd</anchor>
      <arglist>(L_Func *fn)</arglist>
    </member>
    <member kind="function">
      <type>void</type>
      <name>O_process_func</name>
      <anchorfile>phase2__func_8c.html</anchorfile>
      <anchor>85ebf52e442b913b4827e4e3df7c7a40</anchor>
      <arglist>(L_Func *fn, Parm_Macro_List *command_line_macro_list)</arglist>
    </member>
    <member kind="function">
      <type>void</type>
      <name>O_init</name>
      <anchorfile>phase2__func_8c.html</anchorfile>
      <anchor>bccd17401e9b1b23aa95b7d0dd185546</anchor>
      <arglist>(Parm_Macro_List *command_line_macro_list)</arglist>
    </member>
    <member kind="function">
      <type>void</type>
      <name>O_finalize</name>
      <anchorfile>phase2__func_8c.html</anchorfile>
      <anchor>7497ea3ad837b2413e58919b4f047393</anchor>
      <arglist>()</arglist>
    </member>
    <member kind="function">
      <type>void</type>
      <name>S_machine_rts</name>
      <anchorfile>phase2__func_8c.html</anchorfile>
      <anchor>cec0c5f5a0d3c08aea116d6871befbe8</anchor>
      <arglist>(L_Oper *oper)</arglist>
    </member>
    <member kind="function">
      <type>void</type>
      <name>S_machine_jump</name>
      <anchorfile>phase2__func_8c.html</anchorfile>
      <anchor>3316c78b66394bf0677d38014a51c6dd</anchor>
      <arglist>(L_Oper *oper)</arglist>
    </member>
    <member kind="function">
      <type>int</type>
      <name>S_machine_check</name>
      <anchorfile>phase2__func_8c.html</anchorfile>
      <anchor>1cf3946c60d75e862ea4795e9a19fa2f</anchor>
      <arglist>(L_Oper *oper)</arglist>
    </member>
    <member kind="function" static="yes">
      <type>static void</type>
      <name>L_insert_padding_bw</name>
      <anchorfile>phase2__func_8c.html</anchorfile>
      <anchor>c99bd6f7596f0b6215dbe5e26a954c21</anchor>
      <arglist>(L_Cb *from_cb, L_Oper *from_op, L_Cb *to_cb, L_Oper *to_op, int cycles)</arglist>
    </member>
    <member kind="function" static="yes">
      <type>static int</type>
      <name>L_find_op_delta</name>
      <anchorfile>phase2__func_8c.html</anchorfile>
      <anchor>8700079615a7b54e571a0cb5e212d2cc</anchor>
      <arglist>(L_Cb *from_cb, L_Oper *from_op, L_Cb *to_cb, L_Oper *to_op)</arglist>
    </member>
    <member kind="function" static="yes">
      <type>static int</type>
      <name>L_check_for_dep_distance_bw</name>
      <anchorfile>phase2__func_8c.html</anchorfile>
      <anchor>0d8e7a2ff85e1f653116429add3fc83c</anchor>
      <arglist>(L_Func *fn, L_Oper *dest_op, L_Cb *dest_cb, int(*test_fn)(L_Oper *), int min_latency, char *msg, int detectonly)</arglist>
    </member>
    <member kind="function" static="yes">
      <type>static int</type>
      <name>IEU_def_test</name>
      <anchorfile>phase2__func_8c.html</anchorfile>
      <anchor>7ae3e9e8566fa655433d86602961193f</anchor>
      <arglist>(L_Oper *op)</arglist>
    </member>
    <member kind="function">
      <type>int</type>
      <name>OUTPUT_DEP_STALL_def_test</name>
      <anchorfile>phase2__func_8c.html</anchorfile>
      <anchor>5864b778e251d5efd4121819699292b9</anchor>
      <arglist>(L_Oper *op)</arglist>
    </member>
  </compound>
  <compound kind="file">
    <name>phase2_func.h</name>
    <path>/mnt/hgfs/mkt/Desktop/openimpact/src/Lcode/codegen/Ltahoe/</path>
    <filename>phase2__func_8h</filename>
    <member kind="function">
      <type>void</type>
      <name>O_init</name>
      <anchorfile>phase2__func_8h.html</anchorfile>
      <anchor>bccd17401e9b1b23aa95b7d0dd185546</anchor>
      <arglist>(Parm_Macro_List *command_line_macro_list)</arglist>
    </member>
    <member kind="function">
      <type>void</type>
      <name>O_process_func</name>
      <anchorfile>phase2__func_8h.html</anchorfile>
      <anchor>85ebf52e442b913b4827e4e3df7c7a40</anchor>
      <arglist>(L_Func *fn, Parm_Macro_List *command_line_macro_list)</arglist>
    </member>
    <member kind="function">
      <type>void</type>
      <name>O_finalize</name>
      <anchorfile>phase2__func_8h.html</anchorfile>
      <anchor>acfc48c4f5a40d1b49a60c85bcb0255c</anchor>
      <arglist>(void)</arglist>
    </member>
    <member kind="function">
      <type>int</type>
      <name>S_machine_check</name>
      <anchorfile>phase2__func_8h.html</anchorfile>
      <anchor>1cf3946c60d75e862ea4795e9a19fa2f</anchor>
      <arglist>(L_Oper *oper)</arglist>
    </member>
  </compound>
  <compound kind="file">
    <name>phase2_icache.c</name>
    <path>/mnt/hgfs/mkt/Desktop/openimpact/src/Lcode/codegen/Ltahoe/</path>
    <filename>phase2__icache_8c</filename>
    <includes id="ltahoe__main_8h" name="ltahoe_main.h" local="yes" imported="no">ltahoe_main.h</includes>
    <includes id="ltahoe__op__query_8h" name="ltahoe_op_query.h" local="yes" imported="no">ltahoe_op_query.h</includes>
    <includes id="phase2__icache_8h" name="phase2_icache.h" local="yes" imported="no">phase2_icache.h</includes>
    <member kind="define">
      <type>#define</type>
      <name>NOP_PAD_VALUE</name>
      <anchorfile>phase2__icache_8c.html</anchorfile>
      <anchor>3b9a264089040740bf04eaf62d3989a9</anchor>
      <arglist></arglist>
    </member>
    <member kind="define">
      <type>#define</type>
      <name>TAR_HINT_ATTR</name>
      <anchorfile>phase2__icache_8c.html</anchorfile>
      <anchor>5a327a77c7230e22e07c780740a0c03e</anchor>
      <arglist></arglist>
    </member>
    <member kind="define">
      <type>#define</type>
      <name>IS_NOT_CACHE_ALIGNED</name>
      <anchorfile>phase2__icache_8c.html</anchorfile>
      <anchor>79809e86d5569817d146cc4915de45fb</anchor>
      <arglist>(bundle_cnt)</arglist>
    </member>
    <member kind="define">
      <type>#define</type>
      <name>SECOND_BUNDLE_IN_CACHE_LINE</name>
      <anchorfile>phase2__icache_8c.html</anchorfile>
      <anchor>9abb07cbb36c18551c13a53741f7e7f9</anchor>
      <arglist>(bundle_cnt)</arglist>
    </member>
    <member kind="function">
      <type>L_Oper *</type>
      <name>Ltahoe_pad_bundle</name>
      <anchorfile>phase2__icache_8c.html</anchorfile>
      <anchor>352e2b02cd835b9a792c5cf7abaa8208</anchor>
      <arglist>(L_Cb *cb, L_Oper *bundle)</arglist>
    </member>
    <member kind="function" static="yes">
      <type>static int</type>
      <name>Ltahoe_pad_cb_for_cache_alignment</name>
      <anchorfile>phase2__icache_8c.html</anchorfile>
      <anchor>ea0fb9da79098f815f71dfa6b61bc14c</anchor>
      <arglist>(L_Cb *cb)</arglist>
    </member>
    <member kind="function">
      <type>int</type>
      <name>Check_and_align_for_TAR</name>
      <anchorfile>phase2__icache_8c.html</anchorfile>
      <anchor>cdb122b57f26a6af845275fa082ab18d</anchor>
      <arglist>(L_Cb *cb, L_Oper *template, int bundle_count)</arglist>
    </member>
    <member kind="function">
      <type>void</type>
      <name>Ltahoe_cache_align_cbs</name>
      <anchorfile>phase2__icache_8c.html</anchorfile>
      <anchor>30c79261a027c380038cf5d21d8d3652</anchor>
      <arglist>(L_Func *fn, double threshold)</arglist>
    </member>
  </compound>
  <compound kind="file">
    <name>phase2_icache.h</name>
    <path>/mnt/hgfs/mkt/Desktop/openimpact/src/Lcode/codegen/Ltahoe/</path>
    <filename>phase2__icache_8h</filename>
    <includes id="ltahoe__main_8h" name="ltahoe_main.h" local="yes" imported="no">ltahoe_main.h</includes>
    <member kind="function">
      <type>L_Oper *</type>
      <name>Ltahoe_pad_bundle</name>
      <anchorfile>phase2__icache_8h.html</anchorfile>
      <anchor>352e2b02cd835b9a792c5cf7abaa8208</anchor>
      <arglist>(L_Cb *cb, L_Oper *bundle)</arglist>
    </member>
    <member kind="function">
      <type>void</type>
      <name>Ltahoe_cache_align_cbs</name>
      <anchorfile>phase2__icache_8h.html</anchorfile>
      <anchor>30c79261a027c380038cf5d21d8d3652</anchor>
      <arglist>(L_Func *fn, double threshold)</arglist>
    </member>
  </compound>
  <compound kind="file">
    <name>phase2_memstk.c</name>
    <path>/mnt/hgfs/mkt/Desktop/openimpact/src/Lcode/codegen/Ltahoe/</path>
    <filename>phase2__memstk_8c</filename>
    <includes id="ltahoe__main_8h" name="ltahoe_main.h" local="yes" imported="no">ltahoe_main.h</includes>
    <includes id="ltahoe__op__query_8h" name="ltahoe_op_query.h" local="yes" imported="no">ltahoe_op_query.h</includes>
    <includes id="phase1__func_8h" name="phase1_func.h" local="yes" imported="no">phase1_func.h</includes>
    <includes id="phase1__varargs_8h" name="phase1_varargs.h" local="yes" imported="no">phase1_varargs.h</includes>
    <includes id="phase2__reg_8h" name="phase2_reg.h" local="yes" imported="no">phase2_reg.h</includes>
    <member kind="enumeration">
      <name>O_Stk_Update_Type</name>
      <anchor>63519aef0948e405add203af5d1d9c2c</anchor>
      <arglist></arglist>
    </member>
    <member kind="enumvalue">
      <name>O_STK_INC</name>
      <anchor>63519aef0948e405add203af5d1d9c2c543e5509ac2ef00656edfd6649cd2410</anchor>
      <arglist></arglist>
    </member>
    <member kind="enumvalue">
      <name>O_STK_DEC</name>
      <anchor>63519aef0948e405add203af5d1d9c2c7696258f8e0d81ae783b0ffe192d6fd7</anchor>
      <arglist></arglist>
    </member>
    <member kind="function" static="yes">
      <type>static void</type>
      <name>O_implement_unat_swapping</name>
      <anchorfile>phase2__memstk_8c.html</anchorfile>
      <anchor>96079366ec50ae7c8e20e835cd130c89</anchor>
      <arglist>(L_Func *fn, int int_swap_offset, int int_swap_space_size, int unat_swap_offset, int stack_frame_size)</arglist>
    </member>
    <member kind="function" static="yes">
      <type>static void</type>
      <name>O_update_stack_const_in_prev_oper</name>
      <anchorfile>phase2__memstk_8c.html</anchorfile>
      <anchor>7956c4a71e1a581361385130982a47aa</anchor>
      <arglist>(L_Func *fn, L_Cb *cb, L_Oper *stk_oper, L_Operand **stk_operand, L_Operand *var_operand, int offset_val, int variable_frame_size)</arglist>
    </member>
    <member kind="function" static="yes">
      <type>static void</type>
      <name>O_update_stack_references</name>
      <anchorfile>phase2__memstk_8c.html</anchorfile>
      <anchor>fd11e4b4726f8f18fe2a0df498a957ea</anchor>
      <arglist>(L_Func *fn, L_Cb *cb, L_Oper *oper, int local_offset, int int_swap_offset, int fp_swap_offset, int pred_swap_offset, int input_parm_offset, int output_parm_offset, int variable_frame_size)</arglist>
    </member>
    <member kind="function" static="yes">
      <type>static L_Oper *</type>
      <name>O_pred_save_operation</name>
      <anchorfile>phase2__memstk_8c.html</anchorfile>
      <anchor>e8b27599cd550519351e5663ed271667</anchor>
      <arglist>(L_Operand *pred, L_Operand *dest)</arglist>
    </member>
    <member kind="function" static="yes">
      <type>static L_Oper *</type>
      <name>O_pred_restore_operation</name>
      <anchorfile>phase2__memstk_8c.html</anchorfile>
      <anchor>07f079031db330b05de69510cd89c256</anchor>
      <arglist>(L_Operand *pred, L_Operand *src, ITintmax mask)</arglist>
    </member>
    <member kind="function" static="yes">
      <type>static void</type>
      <name>O_save_unat</name>
      <anchorfile>phase2__memstk_8c.html</anchorfile>
      <anchor>f6737d338f54da08f8c47569885cf929</anchor>
      <arglist>(L_Cb *cb, L_Oper *after, L_Oper *add_op, int variable_frame_size, int stack_frame_size)</arglist>
    </member>
    <member kind="function" static="yes">
      <type>static L_Oper *</type>
      <name>Create_add_to_unat_spill_location</name>
      <anchorfile>phase2__memstk_8c.html</anchorfile>
      <anchor>66ca19093c95e879dbdf09dd4b463a94</anchor>
      <arglist>(L_Cb *cb, int stack_frame_size, int local_offset, int unat_offset)</arglist>
    </member>
    <member kind="function" static="yes">
      <type>static void</type>
      <name>O_restore_unat</name>
      <anchorfile>phase2__memstk_8c.html</anchorfile>
      <anchor>dea5bf009784719e53372a6ddbfff8cc</anchor>
      <arglist>(L_Cb *cb, L_Oper *before, L_Oper *add_op)</arglist>
    </member>
    <member kind="function" static="yes">
      <type>static L_Oper *</type>
      <name>Create_add_to_unat_fill_location</name>
      <anchorfile>phase2__memstk_8c.html</anchorfile>
      <anchor>65d146d97d61f752bd1345b5928f90cc</anchor>
      <arglist>(L_Cb *cb, int unat_offset)</arglist>
    </member>
    <member kind="function" static="yes">
      <type>static L_Oper *</type>
      <name>O_int_preserved_store</name>
      <anchorfile>phase2__memstk_8c.html</anchorfile>
      <anchor>d26d3ca734cc45f9c0a4a1029193cb74</anchor>
      <arglist>(int int_reg_id)</arglist>
    </member>
    <member kind="function" static="yes">
      <type>static L_Oper *</type>
      <name>O_fp_preserved_store</name>
      <anchorfile>phase2__memstk_8c.html</anchorfile>
      <anchor>b3c1804bd83847ea9736a42230358fff</anchor>
      <arglist>(int fp_reg_id)</arglist>
    </member>
    <member kind="function" static="yes">
      <type>static L_Oper *</type>
      <name>O_btr_preserved_store</name>
      <anchorfile>phase2__memstk_8c.html</anchorfile>
      <anchor>c4494c4c3c7ab25f8bdb6cf2db05fdd7</anchor>
      <arglist>(void)</arglist>
    </member>
    <member kind="function" static="yes">
      <type>static void</type>
      <name>O_store_preserved_regs</name>
      <anchorfile>phase2__memstk_8c.html</anchorfile>
      <anchor>0fa7813d03b069f77b0d8978a44e186e</anchor>
      <arglist>(L_Cb *cb, L_Oper *oper, int *callee_int_array, int *callee_flt_array, int *callee_btr_array, int psp_offset, int stack_frame_size)</arglist>
    </member>
    <member kind="function" static="yes">
      <type>static L_Oper *</type>
      <name>O_int_preserved_load</name>
      <anchorfile>phase2__memstk_8c.html</anchorfile>
      <anchor>5bd5df1f8336383407ec795479da84d5</anchor>
      <arglist>(int int_reg_id, int post_inc_value)</arglist>
    </member>
    <member kind="function" static="yes">
      <type>static L_Oper *</type>
      <name>O_fp_preserved_load</name>
      <anchorfile>phase2__memstk_8c.html</anchorfile>
      <anchor>24a45c3a4d9cd08209a11ab3e65a8adb</anchor>
      <arglist>(int fp_reg_id, int post_inc_value)</arglist>
    </member>
    <member kind="function" static="yes">
      <type>static L_Oper *</type>
      <name>O_tmp_preserved_load</name>
      <anchorfile>phase2__memstk_8c.html</anchorfile>
      <anchor>b6342881141e5af4e73c496314d1a5aa</anchor>
      <arglist>(int post_inc_value)</arglist>
    </member>
    <member kind="function" static="yes">
      <type>static void</type>
      <name>O_load_preserved_regs</name>
      <anchorfile>phase2__memstk_8c.html</anchorfile>
      <anchor>2af3b078df28112fd8c442666092e1f1</anchor>
      <arglist>(L_Cb *cb, L_Oper *oper, int *callee_flt_array, int *callee_btr_array, int psp_offset, int stack_frame_size)</arglist>
    </member>
    <member kind="function" static="yes">
      <type>static void</type>
      <name>O_update_stack_pointer</name>
      <anchorfile>phase2__memstk_8c.html</anchorfile>
      <anchor>d9ead6f7917701e38b675e4b3dd73d6e</anchor>
      <arglist>(L_Cb *cb, L_Oper *oper, int stack_frame_size, int callee_space_size, O_Stk_Update_Type decrement, int variable_frame_size)</arglist>
    </member>
    <member kind="function">
      <type>void</type>
      <name>O_postpass_adjust_memory_stack</name>
      <anchorfile>phase2__memstk_8c.html</anchorfile>
      <anchor>9422bcd8edd93a91bc04b9aa9a05d77a</anchor>
      <arglist>(L_Func *fn, int int_swap_space_size, int fp_swap_space_size, int pred_swap_space_size, int *callee_flt_array, int psp)</arglist>
    </member>
  </compound>
  <compound kind="file">
    <name>phase2_memstk.h</name>
    <path>/mnt/hgfs/mkt/Desktop/openimpact/src/Lcode/codegen/Ltahoe/</path>
    <filename>phase2__memstk_8h</filename>
    <member kind="function">
      <type>void</type>
      <name>O_postpass_adjust_memory_stack</name>
      <anchorfile>phase2__memstk_8h.html</anchorfile>
      <anchor>ee1d91da1617be58d9a875849a7b067e</anchor>
      <arglist>(L_Func *fn, int int_swap_space_size, int fp_swap_space_size, int pred_swap_space_size, int *callee_flt_array)</arglist>
    </member>
  </compound>
  <compound kind="file">
    <name>phase2_reg.c</name>
    <path>/mnt/hgfs/mkt/Desktop/openimpact/src/Lcode/codegen/Ltahoe/</path>
    <filename>phase2__reg_8c</filename>
    <includes id="ltahoe__main_8h" name="ltahoe_main.h" local="yes" imported="no">ltahoe_main.h</includes>
    <includes id="ltahoe__completers_8h" name="ltahoe_completers.h" local="yes" imported="no">ltahoe_completers.h</includes>
    <includes id="phase2__func_8h" name="phase2_func.h" local="yes" imported="no">phase2_func.h</includes>
    <includes id="phase1__func_8h" name="phase1_func.h" local="yes" imported="no">phase1_func.h</includes>
    <includes id="phase2__reg_8h" name="phase2_reg.h" local="yes" imported="no">phase2_reg.h</includes>
    <includes id="ltahoe__op__query_8h" name="ltahoe_op_query.h" local="yes" imported="no">ltahoe_op_query.h</includes>
    <member kind="define">
      <type>#define</type>
      <name>LOAD_COST</name>
      <anchorfile>phase2__reg_8c.html</anchorfile>
      <anchor>ea98c43ee807f8cd12f4f581f9020ee1</anchor>
      <arglist></arglist>
    </member>
    <member kind="define">
      <type>#define</type>
      <name>STORE_COST</name>
      <anchorfile>phase2__reg_8c.html</anchorfile>
      <anchor>d596f0533da76b316a048a02da0ca6c6</anchor>
      <arglist></arglist>
    </member>
    <member kind="define">
      <type>#define</type>
      <name>NUM_CALLER_INT_REG</name>
      <anchorfile>phase2__reg_8c.html</anchorfile>
      <anchor>49f3436dc216867b820c9172ea73face</anchor>
      <arglist></arglist>
    </member>
    <member kind="define">
      <type>#define</type>
      <name>NUM_CALLEE_INT_STATIC_REG</name>
      <anchorfile>phase2__reg_8c.html</anchorfile>
      <anchor>98db5e8facae71facb71ee9b13339411</anchor>
      <arglist></arglist>
    </member>
    <member kind="define">
      <type>#define</type>
      <name>NUM_CALLER_FLOAT_REG</name>
      <anchorfile>phase2__reg_8c.html</anchorfile>
      <anchor>c773d916f998306a833407731f1fe9cd</anchor>
      <arglist></arglist>
    </member>
    <member kind="define">
      <type>#define</type>
      <name>NUM_CALLEE_FLOAT_REG</name>
      <anchorfile>phase2__reg_8c.html</anchorfile>
      <anchor>bdcba3ca66987293375db3d7e0f6b048</anchor>
      <arglist></arglist>
    </member>
    <member kind="define">
      <type>#define</type>
      <name>NUM_CALLER_FLOAT_MAC</name>
      <anchorfile>phase2__reg_8c.html</anchorfile>
      <anchor>cdf6df63ae418f3d95d682467ec68eba</anchor>
      <arglist></arglist>
    </member>
    <member kind="define">
      <type>#define</type>
      <name>NUM_CALLER_DOUBLE_REG</name>
      <anchorfile>phase2__reg_8c.html</anchorfile>
      <anchor>5bdc49c0beff39f87e3df1980e96b51d</anchor>
      <arglist></arglist>
    </member>
    <member kind="define">
      <type>#define</type>
      <name>NUM_CALLEE_DOUBLE_REG</name>
      <anchorfile>phase2__reg_8c.html</anchorfile>
      <anchor>6bc811b4751b7902c056978846d8cfd2</anchor>
      <arglist></arglist>
    </member>
    <member kind="define">
      <type>#define</type>
      <name>NUM_CALLER_DOUBLE_MAC</name>
      <anchorfile>phase2__reg_8c.html</anchorfile>
      <anchor>78167953c4154c0260acf3984c9094a3</anchor>
      <arglist></arglist>
    </member>
    <member kind="define">
      <type>#define</type>
      <name>NUM_CALLER_PRED_REG</name>
      <anchorfile>phase2__reg_8c.html</anchorfile>
      <anchor>c3491c893243e264fc12b1deaf26b3fd</anchor>
      <arglist></arglist>
    </member>
    <member kind="define">
      <type>#define</type>
      <name>NUM_CALLEE_PRED_REG</name>
      <anchorfile>phase2__reg_8c.html</anchorfile>
      <anchor>c2498d0e2ec38536088a77b4e67fceef</anchor>
      <arglist></arglist>
    </member>
    <member kind="define">
      <type>#define</type>
      <name>NUM_CALLER_BRANCH_REG</name>
      <anchorfile>phase2__reg_8c.html</anchorfile>
      <anchor>69529b203cc2571524a18c8611824e5a</anchor>
      <arglist></arglist>
    </member>
    <member kind="define">
      <type>#define</type>
      <name>NUM_CALLEE_BRANCH_REG</name>
      <anchorfile>phase2__reg_8c.html</anchorfile>
      <anchor>db6d862197d4c7ed06c09237b9ed22cd</anchor>
      <arglist></arglist>
    </member>
    <member kind="define">
      <type>#define</type>
      <name>NUM_CALLEE_INT_STATIC_REG</name>
      <anchorfile>phase2__reg_8c.html</anchorfile>
      <anchor>98db5e8facae71facb71ee9b13339411</anchor>
      <arglist></arglist>
    </member>
    <member kind="function">
      <type>int</type>
      <name>O_is_caller_save_predicate</name>
      <anchorfile>phase2__reg_8c.html</anchorfile>
      <anchor>e17026d5ab11147e0feec3b9488ccd79</anchor>
      <arglist>(L_Operand *pred)</arglist>
    </member>
    <member kind="function">
      <type>void</type>
      <name>O_update_alloc_operands</name>
      <anchorfile>phase2__reg_8c.html</anchorfile>
      <anchor>f8eccdd3b0900666853e71119ae5ced7</anchor>
      <arglist>(L_Oper *oper, int num_special)</arglist>
    </member>
    <member kind="function">
      <type>double</type>
      <name>R_callee_cost</name>
      <anchorfile>phase2__reg_8c.html</anchorfile>
      <anchor>5f536ef01f85bf289d46fcfb560e8358</anchor>
      <arglist>(int ctype, int leaf, int callee_allocated)</arglist>
    </member>
    <member kind="function">
      <type>double</type>
      <name>R_caller_cost</name>
      <anchorfile>phase2__reg_8c.html</anchorfile>
      <anchor>96afdc992285021e3e051a741a516912</anchor>
      <arglist>(int ctype, int leaf)</arglist>
    </member>
    <member kind="function">
      <type>double</type>
      <name>R_spill_load_cost</name>
      <anchorfile>phase2__reg_8c.html</anchorfile>
      <anchor>2a152dabe0ff0494d74d90aec6d7a00f</anchor>
      <arglist>(int ctype)</arglist>
    </member>
    <member kind="function">
      <type>double</type>
      <name>R_spill_store_cost</name>
      <anchorfile>phase2__reg_8c.html</anchorfile>
      <anchor>566eaf596727f931604c1bc91320f52c</anchor>
      <arglist>(int ctype)</arglist>
    </member>
    <member kind="function" static="yes">
      <type>static void</type>
      <name>O_append_op</name>
      <anchorfile>phase2__reg_8c.html</anchorfile>
      <anchor>816d97fcac9d299db3e65db34c6c7cbc</anchor>
      <arglist>(L_Oper **op_seq, L_Oper *new_op)</arglist>
    </member>
    <member kind="function" static="yes">
      <type>static void</type>
      <name>O_prepend_op</name>
      <anchorfile>phase2__reg_8c.html</anchorfile>
      <anchor>a2cb3ebce2a5609f4547e1b26744a96c</anchor>
      <arglist>(L_Oper **op_seq, L_Oper *new_op)</arglist>
    </member>
    <member kind="function" static="yes">
      <type>static L_Oper *</type>
      <name>O_address_add</name>
      <anchorfile>phase2__reg_8c.html</anchorfile>
      <anchor>883feecebb2670a088eef7494d5a232a</anchor>
      <arglist>(int stack_pointer, int offset, L_Operand **pred, int type_flag, int operand_ptype, L_Oper **oper_seq)</arglist>
    </member>
    <member kind="function">
      <type>L_Oper *</type>
      <name>O_fill_reg</name>
      <anchorfile>phase2__reg_8c.html</anchorfile>
      <anchor>2c40d48a8e8989cff32e73c9767e7eb2</anchor>
      <arglist>(int reg, int type, L_Operand *operand, int fill_offset, L_Operand **pred_array, int type_flag)</arglist>
    </member>
    <member kind="function">
      <type>L_Oper *</type>
      <name>O_spill_reg</name>
      <anchorfile>phase2__reg_8c.html</anchorfile>
      <anchor>b4e282bdadba19c2c29bba6e3c46d2a0</anchor>
      <arglist>(int reg, int type, L_Operand *operand, int spill_offset, L_Operand **pred_array, int type_flag)</arglist>
    </member>
    <member kind="function">
      <type>L_Oper *</type>
      <name>O_move_reg</name>
      <anchorfile>phase2__reg_8c.html</anchorfile>
      <anchor>7a0c9cc9975e308ac3e6fb0105e092ac</anchor>
      <arglist>(int dest_reg, int src_reg, int reg_type)</arglist>
    </member>
    <member kind="function">
      <type>L_Oper *</type>
      <name>O_jump_oper</name>
      <anchorfile>phase2__reg_8c.html</anchorfile>
      <anchor>53ded95a34f943d21b94d2ebb82b6ccf</anchor>
      <arglist>(int opc, L_Cb *dest_cb)</arglist>
    </member>
    <member kind="function">
      <type>R_Physical_Bank *</type>
      <name>O_locate_rot_reg_bank</name>
      <anchorfile>phase2__reg_8c.html</anchorfile>
      <anchor>50a0a78097d883cc35ed5656fd2b2182</anchor>
      <arglist>(L_Func *fn, R_Reg *vreg)</arglist>
    </member>
    <member kind="function">
      <type>void</type>
      <name>O_register_init</name>
      <anchorfile>phase2__reg_8c.html</anchorfile>
      <anchor>2828495f3bdf57b0c8d4a4dbe062cbc6</anchor>
      <arglist>()</arglist>
    </member>
    <member kind="function">
      <type>void</type>
      <name>O_register_cleanup</name>
      <anchorfile>phase2__reg_8c.html</anchorfile>
      <anchor>4640f74218c61514b8fbc94bcf50e50c</anchor>
      <arglist>()</arglist>
    </member>
    <member kind="function">
      <type>void</type>
      <name>O_register_allocation</name>
      <anchorfile>phase2__reg_8c.html</anchorfile>
      <anchor>ec6dd6ee1498a4b1c15f1312ec0d77da</anchor>
      <arglist>(L_Func *fn, Parm_Macro_List *command_line_macro_list, int *int_swap_space_size, int *fp_swap_space_size, int *pred_swap_space_size)</arglist>
    </member>
    <member kind="variable">
      <type>int *</type>
      <name>callee_int_array</name>
      <anchorfile>phase2__reg_8c.html</anchorfile>
      <anchor>8fea27c26b837fe09092da2297f46fef</anchor>
      <arglist></arglist>
    </member>
    <member kind="variable">
      <type>int *</type>
      <name>callee_flt_array</name>
      <anchorfile>phase2__reg_8c.html</anchorfile>
      <anchor>0bfcd637e7b5ca144a1998fc28ebcf97</anchor>
      <arglist></arglist>
    </member>
    <member kind="variable">
      <type>int *</type>
      <name>callee_pred_array</name>
      <anchorfile>phase2__reg_8c.html</anchorfile>
      <anchor>3f8ebeff198051f26de49eb8d6c3ce86</anchor>
      <arglist></arglist>
    </member>
    <member kind="variable">
      <type>int *</type>
      <name>callee_btr_array</name>
      <anchorfile>phase2__reg_8c.html</anchorfile>
      <anchor>dd2054d102e1f55e66d6e9997d2b7295</anchor>
      <arglist></arglist>
    </member>
    <member kind="variable" static="yes">
      <type>static int</type>
      <name>stacked_callee_int_num</name>
      <anchorfile>phase2__reg_8c.html</anchorfile>
      <anchor>9073da6dfc32d02db46a2ee370fc2263</anchor>
      <arglist></arglist>
    </member>
    <member kind="variable">
      <type>int</type>
      <name>callee_int_num</name>
      <anchorfile>phase2__reg_8c.html</anchorfile>
      <anchor>91455d0bdb8a6e6cc32c3f61a58a9632</anchor>
      <arglist></arglist>
    </member>
    <member kind="variable">
      <type>int</type>
      <name>callee_flt_num</name>
      <anchorfile>phase2__reg_8c.html</anchorfile>
      <anchor>e6044360f9e2f42fe6b12650f831929c</anchor>
      <arglist></arglist>
    </member>
    <member kind="variable">
      <type>int</type>
      <name>callee_pred_num</name>
      <anchorfile>phase2__reg_8c.html</anchorfile>
      <anchor>ece68970ce338ca2ddfdd25b411f32f2</anchor>
      <arglist></arglist>
    </member>
    <member kind="variable">
      <type>int</type>
      <name>callee_btr_num</name>
      <anchorfile>phase2__reg_8c.html</anchorfile>
      <anchor>497cd47f42be6a5e791cc3acb8da2a09</anchor>
      <arglist></arglist>
    </member>
    <member kind="variable" static="yes">
      <type>static Set</type>
      <name>callee_int_set</name>
      <anchorfile>phase2__reg_8c.html</anchorfile>
      <anchor>f5b779cfc7f97ebec25cbafe7b27a276</anchor>
      <arglist></arglist>
    </member>
    <member kind="variable" static="yes">
      <type>static Set</type>
      <name>caller_int_set</name>
      <anchorfile>phase2__reg_8c.html</anchorfile>
      <anchor>be7515ef85afea311918baa71db5d750</anchor>
      <arglist></arglist>
    </member>
    <member kind="variable" static="yes">
      <type>static Set</type>
      <name>callee_int_macro_set</name>
      <anchorfile>phase2__reg_8c.html</anchorfile>
      <anchor>248f772098af5bda644a9e9075d2e0bf</anchor>
      <arglist></arglist>
    </member>
    <member kind="variable" static="yes">
      <type>static Set</type>
      <name>caller_int_macro_set</name>
      <anchorfile>phase2__reg_8c.html</anchorfile>
      <anchor>4cedf0f07c8d4b3d279155a7b68cb53e</anchor>
      <arglist></arglist>
    </member>
    <member kind="variable" static="yes">
      <type>static Set</type>
      <name>callee_float_set</name>
      <anchorfile>phase2__reg_8c.html</anchorfile>
      <anchor>346ca271b76db98e48c83dfb0b2b29c8</anchor>
      <arglist></arglist>
    </member>
    <member kind="variable" static="yes">
      <type>static Set</type>
      <name>caller_float_set</name>
      <anchorfile>phase2__reg_8c.html</anchorfile>
      <anchor>a87973bf07a6c15a6fda8f8b0ef034c7</anchor>
      <arglist></arglist>
    </member>
    <member kind="variable" static="yes">
      <type>static Set</type>
      <name>caller_float_macro_set</name>
      <anchorfile>phase2__reg_8c.html</anchorfile>
      <anchor>58b02efc824e182e4abc20dedd4a85b9</anchor>
      <arglist></arglist>
    </member>
    <member kind="variable" static="yes">
      <type>static Set</type>
      <name>callee_double_set</name>
      <anchorfile>phase2__reg_8c.html</anchorfile>
      <anchor>31b6a7d7db41d1837b34eff66666043e</anchor>
      <arglist></arglist>
    </member>
    <member kind="variable" static="yes">
      <type>static Set</type>
      <name>caller_double_set</name>
      <anchorfile>phase2__reg_8c.html</anchorfile>
      <anchor>16b58b1ab41624e6d8c49037676dfd6e</anchor>
      <arglist></arglist>
    </member>
    <member kind="variable" static="yes">
      <type>static Set</type>
      <name>caller_double_macro_set</name>
      <anchorfile>phase2__reg_8c.html</anchorfile>
      <anchor>49035474ae3cef3302cf62ef224652c9</anchor>
      <arglist></arglist>
    </member>
    <member kind="variable" static="yes">
      <type>static Set</type>
      <name>callee_predicate_set</name>
      <anchorfile>phase2__reg_8c.html</anchorfile>
      <anchor>ab67425fad758620671596275762f244</anchor>
      <arglist></arglist>
    </member>
    <member kind="variable" static="yes">
      <type>static Set</type>
      <name>caller_predicate_set</name>
      <anchorfile>phase2__reg_8c.html</anchorfile>
      <anchor>f1709faff09ecb1231ad1a49fff804da</anchor>
      <arglist></arglist>
    </member>
    <member kind="variable" static="yes">
      <type>static Set</type>
      <name>callee_branch_set</name>
      <anchorfile>phase2__reg_8c.html</anchorfile>
      <anchor>e9d8bdeff00e268bc48a733cc7f99732</anchor>
      <arglist></arglist>
    </member>
    <member kind="variable" static="yes">
      <type>static Set</type>
      <name>caller_branch_set</name>
      <anchorfile>phase2__reg_8c.html</anchorfile>
      <anchor>4af443791bb3b7fdd4dd301d5dac7b57</anchor>
      <arglist></arglist>
    </member>
    <member kind="variable" static="yes">
      <type>static int</type>
      <name>caller_int_reg_map</name>
      <anchorfile>phase2__reg_8c.html</anchorfile>
      <anchor>ea2f27067dd8a133247070bcab0b83d1</anchor>
      <arglist>[NUM_CALLER_INT_REG]</arglist>
    </member>
    <member kind="variable" static="yes">
      <type>static int</type>
      <name>callee_int_reg_map</name>
      <anchorfile>phase2__reg_8c.html</anchorfile>
      <anchor>87d7617e259c83d2b6560f79d4d319e7</anchor>
      <arglist>[NUM_INT_STACKED_REG]</arglist>
    </member>
    <member kind="variable" static="yes">
      <type>static int</type>
      <name>callee_int_reg_map</name>
      <anchorfile>phase2__reg_8c.html</anchorfile>
      <anchor>87d7617e259c83d2b6560f79d4d319e7</anchor>
      <arglist>[NUM_INT_STACKED_REG]</arglist>
    </member>
    <member kind="variable" static="yes">
      <type>static int</type>
      <name>callee_int_macro_map</name>
      <anchorfile>phase2__reg_8c.html</anchorfile>
      <anchor>ed77290abb7c260ef7b28f41b1a85494</anchor>
      <arglist>[MAX_INT_INPUT_REGS]</arglist>
    </member>
    <member kind="variable" static="yes">
      <type>static int</type>
      <name>caller_int_macro_map</name>
      <anchorfile>phase2__reg_8c.html</anchorfile>
      <anchor>10caec6ce74db3214636625ec5e16b7e</anchor>
      <arglist>[MAX_INT_OUTPUT_REGS+MAX_INT_RETURN_REGS]</arglist>
    </member>
    <member kind="variable" static="yes">
      <type>static int</type>
      <name>caller_float_reg_map</name>
      <anchorfile>phase2__reg_8c.html</anchorfile>
      <anchor>a716c275267de436f12ac0a24f1ee25b</anchor>
      <arglist>[NUM_CALLER_FLOAT_REG]</arglist>
    </member>
    <member kind="variable" static="yes">
      <type>static int</type>
      <name>callee_float_reg_map</name>
      <anchorfile>phase2__reg_8c.html</anchorfile>
      <anchor>ffa59880bd4f96f4458ec0e5b5f5bc4d</anchor>
      <arglist>[NUM_CALLEE_FLOAT_REG]</arglist>
    </member>
    <member kind="variable" static="yes">
      <type>static int</type>
      <name>caller_float_macro_map</name>
      <anchorfile>phase2__reg_8c.html</anchorfile>
      <anchor>0545f2daa9e12fda0c5d2e4cb94feb14</anchor>
      <arglist>[NUM_CALLER_FLOAT_MAC]</arglist>
    </member>
    <member kind="variable" static="yes">
      <type>static int</type>
      <name>caller_double_reg_map</name>
      <anchorfile>phase2__reg_8c.html</anchorfile>
      <anchor>e56bcad1625c97b808add1f0be03d174</anchor>
      <arglist>[NUM_CALLER_DOUBLE_REG]</arglist>
    </member>
    <member kind="variable" static="yes">
      <type>static int</type>
      <name>callee_double_reg_map</name>
      <anchorfile>phase2__reg_8c.html</anchorfile>
      <anchor>3429b2042560a983cab83a524836e853</anchor>
      <arglist>[NUM_CALLEE_DOUBLE_REG]</arglist>
    </member>
    <member kind="variable" static="yes">
      <type>static int</type>
      <name>caller_double_macro_map</name>
      <anchorfile>phase2__reg_8c.html</anchorfile>
      <anchor>4d5f86f7d5184f9376d9199e588f168f</anchor>
      <arglist>[NUM_CALLER_DOUBLE_MAC]</arglist>
    </member>
    <member kind="variable" static="yes">
      <type>static int</type>
      <name>caller_predicate_reg_map</name>
      <anchorfile>phase2__reg_8c.html</anchorfile>
      <anchor>2da01e1b3ebb4130a69944c6180ad6a9</anchor>
      <arglist>[NUM_CALLER_PRED_REG]</arglist>
    </member>
    <member kind="variable" static="yes">
      <type>static int</type>
      <name>callee_predicate_reg_map</name>
      <anchorfile>phase2__reg_8c.html</anchorfile>
      <anchor>52568a3c037402465ac05c208ed951a1</anchor>
      <arglist>[NUM_CALLEE_PRED_REG]</arglist>
    </member>
    <member kind="variable" static="yes">
      <type>static int</type>
      <name>caller_branch_reg_map</name>
      <anchorfile>phase2__reg_8c.html</anchorfile>
      <anchor>7a8723711559f185b315d84e76420008</anchor>
      <arglist>[NUM_CALLER_BRANCH_REG]</arglist>
    </member>
    <member kind="variable" static="yes">
      <type>static int</type>
      <name>callee_branch_reg_map</name>
      <anchorfile>phase2__reg_8c.html</anchorfile>
      <anchor>2be7245654b4056e1da0dda962231c62</anchor>
      <arglist>[NUM_CALLEE_BRANCH_REG]</arglist>
    </member>
  </compound>
  <compound kind="file">
    <name>phase2_reg.h</name>
    <path>/mnt/hgfs/mkt/Desktop/openimpact/src/Lcode/codegen/Ltahoe/</path>
    <filename>phase2__reg_8h</filename>
    <member kind="define">
      <type>#define</type>
      <name>NUM_INT_REG</name>
      <anchorfile>phase2__reg_8h.html</anchorfile>
      <anchor>e7d8573dd9adfeb91887d3c2dc8d3477</anchor>
      <arglist></arglist>
    </member>
    <member kind="define">
      <type>#define</type>
      <name>NUM_INT_STATIC_REG</name>
      <anchorfile>phase2__reg_8h.html</anchorfile>
      <anchor>400f5a2ef5a1728c6a29098c21112a61</anchor>
      <arglist></arglist>
    </member>
    <member kind="define">
      <type>#define</type>
      <name>NUM_INT_STACKED_REG</name>
      <anchorfile>phase2__reg_8h.html</anchorfile>
      <anchor>d14ca7dd34e6b49409002b9bfe77a30d</anchor>
      <arglist></arglist>
    </member>
    <member kind="define">
      <type>#define</type>
      <name>NUM_PRED_REG</name>
      <anchorfile>phase2__reg_8h.html</anchorfile>
      <anchor>afd79879c30ad73cbb3728627d041a0d</anchor>
      <arglist></arglist>
    </member>
    <member kind="define">
      <type>#define</type>
      <name>NUM_FLOAT_REG</name>
      <anchorfile>phase2__reg_8h.html</anchorfile>
      <anchor>39d5f899a965e370dc1a5e2dfff2840b</anchor>
      <arglist></arglist>
    </member>
    <member kind="define">
      <type>#define</type>
      <name>NUM_BRANCH_REG</name>
      <anchorfile>phase2__reg_8h.html</anchorfile>
      <anchor>3ad32fe13b10153d1ee2e0c9ebf0d47d</anchor>
      <arglist></arglist>
    </member>
    <member kind="define">
      <type>#define</type>
      <name>MIN_INT_REGISTER_ID</name>
      <anchorfile>phase2__reg_8h.html</anchorfile>
      <anchor>9570581be6bfc26afb298b1ed0d4a1e8</anchor>
      <arglist></arglist>
    </member>
    <member kind="define">
      <type>#define</type>
      <name>MAX_INT_REGISTER_ID</name>
      <anchorfile>phase2__reg_8h.html</anchorfile>
      <anchor>2e302ba4abf5a7438fab83b3065a5822</anchor>
      <arglist></arglist>
    </member>
    <member kind="define">
      <type>#define</type>
      <name>MIN_INT_STATIC_REGISTER_ID</name>
      <anchorfile>phase2__reg_8h.html</anchorfile>
      <anchor>54ab338b7bf1fc85af1f0c1dff1e3a87</anchor>
      <arglist></arglist>
    </member>
    <member kind="define">
      <type>#define</type>
      <name>MAX_INT_STATIC_REGISTER_ID</name>
      <anchorfile>phase2__reg_8h.html</anchorfile>
      <anchor>cebaee09e0404de0263bdbf97396f44b</anchor>
      <arglist></arglist>
    </member>
    <member kind="define">
      <type>#define</type>
      <name>MIN_INT_STACKED_REGISTER_ID</name>
      <anchorfile>phase2__reg_8h.html</anchorfile>
      <anchor>32a2fda5f1131b91adf74c1489643736</anchor>
      <arglist></arglist>
    </member>
    <member kind="define">
      <type>#define</type>
      <name>MAX_INT_STACKED_REGISTER_ID</name>
      <anchorfile>phase2__reg_8h.html</anchorfile>
      <anchor>af9007f262eecb2ad797036a39aa5da3</anchor>
      <arglist></arglist>
    </member>
    <member kind="define">
      <type>#define</type>
      <name>MAX_PRED_REGISTER_NUMBER</name>
      <anchorfile>phase2__reg_8h.html</anchorfile>
      <anchor>4f47f21dff5fdf94649ed075d71def7d</anchor>
      <arglist></arglist>
    </member>
    <member kind="define">
      <type>#define</type>
      <name>MAX_FLOAT_REGISTER_NUMBER</name>
      <anchorfile>phase2__reg_8h.html</anchorfile>
      <anchor>d74d0d4092887598c58f83301c8c0379</anchor>
      <arglist></arglist>
    </member>
    <member kind="define">
      <type>#define</type>
      <name>MAX_BRANCH_REGISTER_NUMBER</name>
      <anchorfile>phase2__reg_8h.html</anchorfile>
      <anchor>e26c33779065d175aaa6599fac13aebe</anchor>
      <arglist></arglist>
    </member>
    <member kind="define">
      <type>#define</type>
      <name>NUM_SPECIAL_INT_REG</name>
      <anchorfile>phase2__reg_8h.html</anchorfile>
      <anchor>2f7d63b75547ddcf02c6630d097166ec</anchor>
      <arglist></arglist>
    </member>
    <member kind="define">
      <type>#define</type>
      <name>NUM_SPECIAL_PRED_REG</name>
      <anchorfile>phase2__reg_8h.html</anchorfile>
      <anchor>fd610579fbd869697f79868478406778</anchor>
      <arglist></arglist>
    </member>
    <member kind="define">
      <type>#define</type>
      <name>NUM_SPECIAL_FLOAT_REG</name>
      <anchorfile>phase2__reg_8h.html</anchorfile>
      <anchor>ed714cc159fd9e8cc82238b0cf09b907</anchor>
      <arglist></arglist>
    </member>
    <member kind="define">
      <type>#define</type>
      <name>NUM_SPECIAL_BRANCH_REG</name>
      <anchorfile>phase2__reg_8h.html</anchorfile>
      <anchor>e452e632995f0c235f0722aa0029fb06</anchor>
      <arglist></arglist>
    </member>
    <member kind="define">
      <type>#define</type>
      <name>INT_REG_BASE</name>
      <anchorfile>phase2__reg_8h.html</anchorfile>
      <anchor>ea0e1578e209f3050e9210bf70f063e8</anchor>
      <arglist></arglist>
    </member>
    <member kind="define">
      <type>#define</type>
      <name>INT_STACK_REG_BASE</name>
      <anchorfile>phase2__reg_8h.html</anchorfile>
      <anchor>5a3a901d07c45b354749887a33d1a259</anchor>
      <arglist></arglist>
    </member>
    <member kind="define">
      <type>#define</type>
      <name>INT_SPILL_REG_BASE</name>
      <anchorfile>phase2__reg_8h.html</anchorfile>
      <anchor>8c5abe0442d45d9ffaf02551527e9611</anchor>
      <arglist></arglist>
    </member>
    <member kind="define">
      <type>#define</type>
      <name>FLOAT_REG_BASE</name>
      <anchorfile>phase2__reg_8h.html</anchorfile>
      <anchor>106b086814c912a95b97ed09fbe79099</anchor>
      <arglist></arglist>
    </member>
    <member kind="define">
      <type>#define</type>
      <name>PRED_REG_BASE</name>
      <anchorfile>phase2__reg_8h.html</anchorfile>
      <anchor>2fae7140ed16492e60b9992871b2c8cf</anchor>
      <arglist></arglist>
    </member>
    <member kind="define">
      <type>#define</type>
      <name>BRANCH_REG_BASE</name>
      <anchorfile>phase2__reg_8h.html</anchorfile>
      <anchor>46c1491cdc1abc073d150deaa4d0ffaf</anchor>
      <arglist></arglist>
    </member>
    <member kind="define">
      <type>#define</type>
      <name>IS_INT_REGISTER</name>
      <anchorfile>phase2__reg_8h.html</anchorfile>
      <anchor>f34e14cd72d5e54af23b0a477db49c5b</anchor>
      <arglist>(id)</arglist>
    </member>
    <member kind="define">
      <type>#define</type>
      <name>IS_FP_REGISTER</name>
      <anchorfile>phase2__reg_8h.html</anchorfile>
      <anchor>2ea053d76c9976a08913424a38bc8cfe</anchor>
      <arglist>(id)</arglist>
    </member>
    <member kind="define">
      <type>#define</type>
      <name>IS_BRANCH_REGISTER</name>
      <anchorfile>phase2__reg_8h.html</anchorfile>
      <anchor>65f41fb9bba53ff6e21cbac1e9ca2a8d</anchor>
      <arglist>(id)</arglist>
    </member>
    <member kind="define">
      <type>#define</type>
      <name>IS_PREDICATE_REGISTER</name>
      <anchorfile>phase2__reg_8h.html</anchorfile>
      <anchor>9fb8c45f90a775eb7f002f0a97524ae9</anchor>
      <arglist>(id)</arglist>
    </member>
    <member kind="define">
      <type>#define</type>
      <name>MAX_INT_INPUT_REGS</name>
      <anchorfile>phase2__reg_8h.html</anchorfile>
      <anchor>0da21616515540a3d2e213bc3ad29544</anchor>
      <arglist></arglist>
    </member>
    <member kind="define">
      <type>#define</type>
      <name>MAX_INT_OUTPUT_REGS</name>
      <anchorfile>phase2__reg_8h.html</anchorfile>
      <anchor>0eb4d0b561d726390fbd81d75aad20e9</anchor>
      <arglist></arglist>
    </member>
    <member kind="define">
      <type>#define</type>
      <name>MAX_INT_RETURN_REGS</name>
      <anchorfile>phase2__reg_8h.html</anchorfile>
      <anchor>836ab455a3fe6f7261bb175df1485910</anchor>
      <arglist></arglist>
    </member>
    <member kind="define">
      <type>#define</type>
      <name>INT_RETURN_VALUE_REG</name>
      <anchorfile>phase2__reg_8h.html</anchorfile>
      <anchor>bec600921a2f4c0ae74f328a055cd624</anchor>
      <arglist></arglist>
    </member>
    <member kind="define">
      <type>#define</type>
      <name>FLT_RETURN_VALUE_REG</name>
      <anchorfile>phase2__reg_8h.html</anchorfile>
      <anchor>17910d8d929b5d6ba2d3ae59e422fb69</anchor>
      <arglist></arglist>
    </member>
    <member kind="define">
      <type>#define</type>
      <name>FLT_INPUT_PARMS_START</name>
      <anchorfile>phase2__reg_8h.html</anchorfile>
      <anchor>4490126b11d7d2569c169de036a386ae</anchor>
      <arglist></arglist>
    </member>
    <member kind="define">
      <type>#define</type>
      <name>INT_CALLEE_SAVE_SIZE</name>
      <anchorfile>phase2__reg_8h.html</anchorfile>
      <anchor>7f384fac1a209f4e52d0b7e21ba1b40f</anchor>
      <arglist></arglist>
    </member>
    <member kind="define">
      <type>#define</type>
      <name>DOUBLE_CALLEE_SAVE_SIZE</name>
      <anchorfile>phase2__reg_8h.html</anchorfile>
      <anchor>0329dd8cf7e17b7572f36c0a78e8d9e6</anchor>
      <arglist></arglist>
    </member>
    <member kind="define">
      <type>#define</type>
      <name>PRED_BLK_CALLEE_SAVE_SIZE</name>
      <anchorfile>phase2__reg_8h.html</anchorfile>
      <anchor>2295692878c5a51fca82e490a53be2de</anchor>
      <arglist></arglist>
    </member>
    <member kind="define">
      <type>#define</type>
      <name>BTR_CALLEE_SAVE_SIZE</name>
      <anchorfile>phase2__reg_8h.html</anchorfile>
      <anchor>0d07af699761d62afc86b1d622b69f34</anchor>
      <arglist></arglist>
    </member>
    <member kind="function">
      <type>int</type>
      <name>O_is_caller_save_predicate</name>
      <anchorfile>phase2__reg_8h.html</anchorfile>
      <anchor>e17026d5ab11147e0feec3b9488ccd79</anchor>
      <arglist>(L_Operand *pred)</arglist>
    </member>
    <member kind="function">
      <type>L_Oper *</type>
      <name>O_spill_reg</name>
      <anchorfile>phase2__reg_8h.html</anchorfile>
      <anchor>5249a2aa8d7d7d5dd3017f2381970f4e</anchor>
      <arglist>(int reg, int type, L_Operand *operand, int spill_offset, L_Operand **pred, int type_flag)</arglist>
    </member>
    <member kind="function">
      <type>L_Oper *</type>
      <name>O_fill_reg</name>
      <anchorfile>phase2__reg_8h.html</anchorfile>
      <anchor>9f7d468e025a4f50218cb5c0927aac50</anchor>
      <arglist>(int reg, int type, L_Operand *operand, int fill_offset, L_Operand **pred, int type_flag)</arglist>
    </member>
    <member kind="function">
      <type>L_Oper *</type>
      <name>O_jump_oper</name>
      <anchorfile>phase2__reg_8h.html</anchorfile>
      <anchor>53ded95a34f943d21b94d2ebb82b6ccf</anchor>
      <arglist>(int opc, L_Cb *dest_cb)</arglist>
    </member>
    <member kind="function">
      <type>double</type>
      <name>R_caller_cost</name>
      <anchorfile>phase2__reg_8h.html</anchorfile>
      <anchor>06a9b67aa7ff79099512bc9b45121ad0</anchor>
      <arglist>(int lcode_ctype, int leaf)</arglist>
    </member>
    <member kind="function">
      <type>double</type>
      <name>R_callee_cost</name>
      <anchorfile>phase2__reg_8h.html</anchorfile>
      <anchor>865fa697be175e9009dbfbd0ddbaded3</anchor>
      <arglist>(int lcode_ctype, int leaf, int callee_allocated)</arglist>
    </member>
    <member kind="function">
      <type>double</type>
      <name>R_spill_store_cost</name>
      <anchorfile>phase2__reg_8h.html</anchorfile>
      <anchor>b71237561fcf4fc259b3c85f949b14e7</anchor>
      <arglist>(int lcode_ctype)</arglist>
    </member>
    <member kind="function">
      <type>double</type>
      <name>R_spill_load_cost</name>
      <anchorfile>phase2__reg_8h.html</anchorfile>
      <anchor>81182c2e2cc5adf59bae84752b07ad94</anchor>
      <arglist>(int lcode_ctype)</arglist>
    </member>
    <member kind="function">
      <type>void</type>
      <name>O_register_init</name>
      <anchorfile>phase2__reg_8h.html</anchorfile>
      <anchor>438d9d7a62ea37581847967430e09d62</anchor>
      <arglist>(void)</arglist>
    </member>
    <member kind="function">
      <type>void</type>
      <name>O_register_cleanup</name>
      <anchorfile>phase2__reg_8h.html</anchorfile>
      <anchor>4640f74218c61514b8fbc94bcf50e50c</anchor>
      <arglist>()</arglist>
    </member>
    <member kind="function">
      <type>void</type>
      <name>O_register_allocation</name>
      <anchorfile>phase2__reg_8h.html</anchorfile>
      <anchor>ec6dd6ee1498a4b1c15f1312ec0d77da</anchor>
      <arglist>(L_Func *fn, Parm_Macro_List *command_line_macro_list, int *int_swap_space_size, int *fp_swap_space_size, int *pred_swap_space_size)</arglist>
    </member>
    <member kind="function">
      <type>void</type>
      <name>O_update_alloc_operands</name>
      <anchorfile>phase2__reg_8h.html</anchorfile>
      <anchor>f8eccdd3b0900666853e71119ae5ced7</anchor>
      <arglist>(L_Oper *oper, int num_special)</arglist>
    </member>
    <member kind="function">
      <type>void</type>
      <name>O_remove_spill_code</name>
      <anchorfile>phase2__reg_8h.html</anchorfile>
      <anchor>ca74956d19d4ef5d5a4b6906a13956ef</anchor>
      <arglist>(L_Cb *cb, L_Oper *oper)</arglist>
    </member>
    <member kind="variable">
      <type>int *</type>
      <name>callee_pred_array</name>
      <anchorfile>phase2__reg_8h.html</anchorfile>
      <anchor>3f8ebeff198051f26de49eb8d6c3ce86</anchor>
      <arglist></arglist>
    </member>
    <member kind="variable">
      <type>int *</type>
      <name>callee_int_array</name>
      <anchorfile>phase2__reg_8h.html</anchorfile>
      <anchor>8fea27c26b837fe09092da2297f46fef</anchor>
      <arglist></arglist>
    </member>
    <member kind="variable">
      <type>int *</type>
      <name>callee_flt_array</name>
      <anchorfile>phase2__reg_8h.html</anchorfile>
      <anchor>0bfcd637e7b5ca144a1998fc28ebcf97</anchor>
      <arglist></arglist>
    </member>
    <member kind="variable">
      <type>int *</type>
      <name>callee_btr_array</name>
      <anchorfile>phase2__reg_8h.html</anchorfile>
      <anchor>dd2054d102e1f55e66d6e9997d2b7295</anchor>
      <arglist></arglist>
    </member>
    <member kind="variable">
      <type>int</type>
      <name>callee_pred_num</name>
      <anchorfile>phase2__reg_8h.html</anchorfile>
      <anchor>ece68970ce338ca2ddfdd25b411f32f2</anchor>
      <arglist></arglist>
    </member>
    <member kind="variable">
      <type>int</type>
      <name>callee_int_num</name>
      <anchorfile>phase2__reg_8h.html</anchorfile>
      <anchor>91455d0bdb8a6e6cc32c3f61a58a9632</anchor>
      <arglist></arglist>
    </member>
    <member kind="variable">
      <type>int</type>
      <name>callee_flt_num</name>
      <anchorfile>phase2__reg_8h.html</anchorfile>
      <anchor>e6044360f9e2f42fe6b12650f831929c</anchor>
      <arglist></arglist>
    </member>
    <member kind="variable">
      <type>int</type>
      <name>callee_btr_num</name>
      <anchorfile>phase2__reg_8h.html</anchorfile>
      <anchor>497cd47f42be6a5e791cc3acb8da2a09</anchor>
      <arglist></arglist>
    </member>
  </compound>
  <compound kind="file">
    <name>phase2_sync.c</name>
    <path>/mnt/hgfs/mkt/Desktop/openimpact/src/Lcode/codegen/Ltahoe/</path>
    <filename>phase2__sync_8c</filename>
    <includes id="ltahoe__main_8h" name="ltahoe_main.h" local="yes" imported="no">ltahoe_main.h</includes>
    <includes id="phase2__reg_8h" name="phase2_reg.h" local="yes" imported="no">phase2_reg.h</includes>
    <includes id="phase1__func_8h" name="phase1_func.h" local="yes" imported="no">phase1_func.h</includes>
    <member kind="define">
      <type>#define</type>
      <name>IS_INT_MACRO_TO_SPILL</name>
      <anchorfile>phase2__sync_8c.html</anchorfile>
      <anchor>66eec31f6b6ea435d71f7e7b3986f76b</anchor>
      <arglist>(id)</arglist>
    </member>
    <member kind="define">
      <type>#define</type>
      <name>IS_FP_MACRO_TO_SPILL</name>
      <anchorfile>phase2__sync_8c.html</anchorfile>
      <anchor>34290ba6f05d24ec0638f62c82ac7dd4</anchor>
      <arglist>(id)</arglist>
    </member>
    <member kind="function" static="yes">
      <type>static int</type>
      <name>O_spill_fill_around_sync</name>
      <anchorfile>phase2__sync_8c.html</anchorfile>
      <anchor>6ea92460d8b69bbddf99ef95377dae32</anchor>
      <arglist>(L_Func *fn, L_Cb *cb, L_Oper *op, int *int_spill_start, int *int_spill_end)</arglist>
    </member>
    <member kind="function" static="yes">
      <type>static void</type>
      <name>O_insert_spill_sequence</name>
      <anchorfile>phase2__sync_8c.html</anchorfile>
      <anchor>b8659bb3b39acb9358268ad61abc1ce5</anchor>
      <arglist>(L_Cb *cb, L_Oper *before_op, L_Oper *op_list)</arglist>
    </member>
    <member kind="function" static="yes">
      <type>static void</type>
      <name>O_insert_fill_sequence</name>
      <anchorfile>phase2__sync_8c.html</anchorfile>
      <anchor>1483536edc8695e17312fe15d20344c7</anchor>
      <arglist>(L_Cb *cb, L_Oper *after_op, L_Oper *op_list)</arglist>
    </member>
    <member kind="function">
      <type>int</type>
      <name>O_handle_sync_opers</name>
      <anchorfile>phase2__sync_8c.html</anchorfile>
      <anchor>f646bef737ca1bb75a2d851296c139ab</anchor>
      <arglist>(L_Func *fn)</arglist>
    </member>
  </compound>
  <compound kind="file">
    <name>phase2_sync.h</name>
    <path>/mnt/hgfs/mkt/Desktop/openimpact/src/Lcode/codegen/Ltahoe/</path>
    <filename>phase2__sync_8h</filename>
    <member kind="function">
      <type>int</type>
      <name>O_handle_sync_opers</name>
      <anchorfile>phase2__sync_8h.html</anchorfile>
      <anchor>f646bef737ca1bb75a2d851296c139ab</anchor>
      <arglist>(L_Func *fn)</arglist>
    </member>
    <member kind="function">
      <type>int</type>
      <name>O_get_sync_int_spill_size</name>
      <anchorfile>phase2__sync_8h.html</anchorfile>
      <anchor>5b1940fbdb918aaf4a478c03837f1e78</anchor>
      <arglist>(L_Func *fn)</arglist>
    </member>
  </compound>
  <compound kind="file">
    <name>phase3.h</name>
    <path>/mnt/hgfs/mkt/Desktop/openimpact/src/Lcode/codegen/Ltahoe/</path>
    <filename>phase3_8h</filename>
    <class kind="union">CONVERT</class>
    <member kind="define">
      <type>#define</type>
      <name>LOCAL_LABELS</name>
      <anchorfile>phase3_8h.html</anchorfile>
      <anchor>7607c679e2c673a7e043f04952e8bd9f</anchor>
      <arglist></arglist>
    </member>
    <member kind="function">
      <type>void</type>
      <name>P_file_init</name>
      <anchorfile>phase3_8h.html</anchorfile>
      <anchor>4cec9d741770c44898d8d144e2111417</anchor>
      <arglist>(void)</arglist>
    </member>
    <member kind="function">
      <type>void</type>
      <name>P_file_end</name>
      <anchorfile>phase3_8h.html</anchorfile>
      <anchor>f644eeae2f7030c2de8032c785446fe3</anchor>
      <arglist>(void)</arglist>
    </member>
    <member kind="function">
      <type>void</type>
      <name>P_end</name>
      <anchorfile>phase3_8h.html</anchorfile>
      <anchor>306e6aeafa6c8c0d30c792ab6e5ad680</anchor>
      <arglist>(void)</arglist>
    </member>
    <member kind="function">
      <type>void</type>
      <name>P_init</name>
      <anchorfile>phase3_8h.html</anchorfile>
      <anchor>d782607017bab037639cdd6b84f2a448</anchor>
      <arglist>(Parm_Macro_List *command_line_macro_list)</arglist>
    </member>
    <member kind="function">
      <type>void</type>
      <name>P_process_func</name>
      <anchorfile>phase3_8h.html</anchorfile>
      <anchor>dd6f2ad9943286f3c6f36049c51977dc</anchor>
      <arglist>(L_Func *fn)</arglist>
    </member>
    <member kind="function">
      <type>void</type>
      <name>P_convert_reg_nums</name>
      <anchorfile>phase3_8h.html</anchorfile>
      <anchor>46be539d3369f6d42da520278ecb40f4</anchor>
      <arglist>(L_Func *fn)</arglist>
    </member>
    <member kind="function">
      <type>void</type>
      <name>P_print_oper</name>
      <anchorfile>phase3_8h.html</anchorfile>
      <anchor>0edb830c590a4a5e79f7895e83106ff3</anchor>
      <arglist>(L_Cb *cb, L_Oper *oper, unsigned int *instr_offset, unsigned int *issue_cycle)</arglist>
    </member>
    <member kind="function">
      <type>void</type>
      <name>P_set_explicit_bundling</name>
      <anchorfile>phase3_8h.html</anchorfile>
      <anchor>a2f96fa4c836b58d9acdd11473b9a706</anchor>
      <arglist>(void)</arglist>
    </member>
    <member kind="function">
      <type>void</type>
      <name>P_set_implicit_bundling</name>
      <anchorfile>phase3_8h.html</anchorfile>
      <anchor>8d6404cc9f626160af13aa4307cd71e2</anchor>
      <arglist>(void)</arglist>
    </member>
    <member kind="function">
      <type>void</type>
      <name>P_reset_bundle_indx</name>
      <anchorfile>phase3_8h.html</anchorfile>
      <anchor>9fb6c38964813b548e42ab7000d3952a</anchor>
      <arglist>(void)</arglist>
    </member>
    <member kind="function">
      <type>void</type>
      <name>P_fix_pred_compare_dests_func</name>
      <anchorfile>phase3_8h.html</anchorfile>
      <anchor>fa71e00ee22caa38338eb89fe4ba919b</anchor>
      <arglist>(L_Func *fn)</arglist>
    </member>
    <member kind="function">
      <type>void</type>
      <name>P_process_data</name>
      <anchorfile>phase3_8h.html</anchorfile>
      <anchor>3f2c09ee4cdbf79904493f05cc9d2719</anchor>
      <arglist>(FILE *F_OUT, L_Data *data)</arglist>
    </member>
    <member kind="function">
      <type>void</type>
      <name>P_print_standard_file_header</name>
      <anchorfile>phase3_8h.html</anchorfile>
      <anchor>4d27252daf10cd87ca246da97a241be5</anchor>
      <arglist>(void)</arglist>
    </member>
    <member kind="function">
      <type>void</type>
      <name>P_print_section_title</name>
      <anchorfile>phase3_8h.html</anchorfile>
      <anchor>19d335e82c51b4addf54ed403a50ec5c</anchor>
      <arglist>(int section_type)</arglist>
    </member>
    <member kind="function">
      <type>void</type>
      <name>P_symtab_add_label</name>
      <anchorfile>phase3_8h.html</anchorfile>
      <anchor>4db7e24ed1149bb2540576ec0c7ab06f</anchor>
      <arglist>(char *label, int isfunc)</arglist>
    </member>
    <member kind="function">
      <type>void</type>
      <name>P_symtab_add_def</name>
      <anchorfile>phase3_8h.html</anchorfile>
      <anchor>215a27cc5b3c9bcac7d5470cda46f5d3</anchor>
      <arglist>(char *label, int isfunc)</arglist>
    </member>
    <member kind="function">
      <type>void</type>
      <name>P_symtab_init</name>
      <anchorfile>phase3_8h.html</anchorfile>
      <anchor>900ce5202e4b08cce287eaa221052f4e</anchor>
      <arglist>(void)</arglist>
    </member>
    <member kind="function">
      <type>void</type>
      <name>P_symtab_deinit</name>
      <anchorfile>phase3_8h.html</anchorfile>
      <anchor>a1ef182d84239141f9f1d7a6fe89f5c9</anchor>
      <arglist>(void)</arglist>
    </member>
    <member kind="function">
      <type>void</type>
      <name>P_symtab_print_extern</name>
      <anchorfile>phase3_8h.html</anchorfile>
      <anchor>c9e8c6bb24f5db8a05b092577c56c078</anchor>
      <arglist>(void)</arglist>
    </member>
    <member kind="function">
      <type>void</type>
      <name>P_print_load_table</name>
      <anchorfile>phase3_8h.html</anchorfile>
      <anchor>0b7778744ba7ba16808455d0e51e9e3c</anchor>
      <arglist>(L_Oper *oper, L_Cb *cb, unsigned int instr_offset, int slot_no)</arglist>
    </member>
    <member kind="variable">
      <type>int</type>
      <name>num_reg_stack_inputs</name>
      <anchorfile>phase3_8h.html</anchorfile>
      <anchor>8ae51a9093172ee479dfec5e1cc39e31</anchor>
      <arglist></arglist>
    </member>
    <member kind="variable">
      <type>int</type>
      <name>num_reg_stack_locals</name>
      <anchorfile>phase3_8h.html</anchorfile>
      <anchor>a81f50b041ddc4292e3762f6f883592e</anchor>
      <arglist></arglist>
    </member>
    <member kind="variable">
      <type>int</type>
      <name>num_reg_stack_outputs</name>
      <anchorfile>phase3_8h.html</anchorfile>
      <anchor>a976e684b5d8b88de9b2a2c454f9b5ba</anchor>
      <arglist></arglist>
    </member>
    <member kind="variable">
      <type>int</type>
      <name>num_reg_stack_rots</name>
      <anchorfile>phase3_8h.html</anchorfile>
      <anchor>f731aa3c8a9b748e2b069c97ffc795a0</anchor>
      <arglist></arglist>
    </member>
    <member kind="variable">
      <type>FILE *</type>
      <name>LD_TABLE_OUT</name>
      <anchorfile>phase3_8h.html</anchorfile>
      <anchor>3f3005df004fe75ef0f32ef1f80d1d73</anchor>
      <arglist></arglist>
    </member>
  </compound>
  <compound kind="file">
    <name>phase3_data.c</name>
    <path>/mnt/hgfs/mkt/Desktop/openimpact/src/Lcode/codegen/Ltahoe/</path>
    <filename>phase3__data_8c</filename>
    <includes id="ltahoe__main_8h" name="ltahoe_main.h" local="yes" imported="no">ltahoe_main.h</includes>
    <includes id="phase3_8h" name="phase3.h" local="yes" imported="no">phase3.h</includes>
    <includes id="phase1__func_8h" name="phase1_func.h" local="yes" imported="no">phase1_func.h</includes>
    <class kind="struct">_P_Init_Data</class>
    <class kind="struct">_P_Data_Section</class>
    <member kind="typedef">
      <type>_P_Init_Data</type>
      <name>P_Init_Data</name>
      <anchorfile>phase3__data_8c.html</anchorfile>
      <anchor>7bcc2d91efe94cdd5ee080cfc705178f</anchor>
      <arglist></arglist>
    </member>
    <member kind="typedef">
      <type>_P_Data_Section</type>
      <name>P_Data_Section</name>
      <anchorfile>phase3__data_8c.html</anchorfile>
      <anchor>8c4c2fa998cef6dca758e6224b95fb35</anchor>
      <arglist></arglist>
    </member>
    <member kind="function" static="yes">
      <type>static P_Init_Data *</type>
      <name>P_create_initial_data</name>
      <anchorfile>phase3__data_8c.html</anchorfile>
      <anchor>e17eae279a813953e394c08df550b75d</anchor>
      <arglist>(int type)</arglist>
    </member>
    <member kind="function" static="yes">
      <type>static void</type>
      <name>P_init_data_section</name>
      <anchorfile>phase3__data_8c.html</anchorfile>
      <anchor>0abef8282c1d902ba75a0a13868ac953</anchor>
      <arglist>(P_Data_Section *section, char *label, int type, int global)</arglist>
    </member>
    <member kind="function" static="yes">
      <type>static void</type>
      <name>P_clean_data_section</name>
      <anchorfile>phase3__data_8c.html</anchorfile>
      <anchor>9219707bc71f60c7d1a558fb34b503c2</anchor>
      <arglist>(P_Data_Section *section)</arglist>
    </member>
    <member kind="function" static="yes">
      <type>static int</type>
      <name>P_string_length</name>
      <anchorfile>phase3__data_8c.html</anchorfile>
      <anchor>60ae18322e80585905fce06345fcb899</anchor>
      <arglist>(char *s)</arglist>
    </member>
    <member kind="function" static="yes">
      <type>static int</type>
      <name>P_process_expr</name>
      <anchorfile>phase3__data_8c.html</anchorfile>
      <anchor>9a0e950acfc4ae765a680e9dd8912964</anchor>
      <arglist>(L_Expr *expr)</arglist>
    </member>
    <member kind="function" static="yes">
      <type>static void</type>
      <name>P_add_label</name>
      <anchorfile>phase3__data_8c.html</anchorfile>
      <anchor>eea1d9f03a98164f0b9eb35f8fb5fde3</anchor>
      <arglist>(P_Init_Data *init_data, L_Expr *value)</arglist>
    </member>
    <member kind="function" static="yes">
      <type>static void</type>
      <name>P_process_expr_add</name>
      <anchorfile>phase3__data_8c.html</anchorfile>
      <anchor>57e77eb08b4864be8b0ba1cbb9de6010</anchor>
      <arglist>(P_Init_Data *init_data, L_Expr *expr)</arglist>
    </member>
    <member kind="function" static="yes">
      <type>static int</type>
      <name>P_next_data</name>
      <anchorfile>phase3__data_8c.html</anchorfile>
      <anchor>3f10f2110057dc240739a73741b854d9</anchor>
      <arglist>(void)</arglist>
    </member>
    <member kind="function" static="yes">
      <type>static int</type>
      <name>P_process_data_resv</name>
      <anchorfile>phase3__data_8c.html</anchorfile>
      <anchor>81b4cc1738e679478022ee73ab0355b6</anchor>
      <arglist>(P_Data_Section *section, int section_type)</arglist>
    </member>
    <member kind="function" static="yes">
      <type>static int</type>
      <name>P_bss_skippable_token_type</name>
      <anchorfile>phase3__data_8c.html</anchorfile>
      <anchor>4e36ec01ef09129460945774ab3d9300</anchor>
      <arglist>(int token_type)</arglist>
    </member>
    <member kind="function">
      <type>void</type>
      <name>P_print_section_title</name>
      <anchorfile>phase3__data_8c.html</anchorfile>
      <anchor>19d335e82c51b4addf54ed403a50ec5c</anchor>
      <arglist>(int section_type)</arglist>
    </member>
    <member kind="function" static="yes">
      <type>static void</type>
      <name>P_print_initialized_data</name>
      <anchorfile>phase3__data_8c.html</anchorfile>
      <anchor>9dd1c300f9728b54fbca5950cdfa9427</anchor>
      <arglist>(P_Data_Section *ptr)</arglist>
    </member>
    <member kind="function" static="yes">
      <type>static void</type>
      <name>P_print_reserve_data</name>
      <anchorfile>phase3__data_8c.html</anchorfile>
      <anchor>1642956f7c48eb5bd77317f9872fd3f2</anchor>
      <arglist>(P_Data_Section *ptr)</arglist>
    </member>
    <member kind="function" static="yes">
      <type>static int</type>
      <name>P_process_data_decl</name>
      <anchorfile>phase3__data_8c.html</anchorfile>
      <anchor>8eb204501145bfc5be236e1d4ac0ada3</anchor>
      <arglist>(int section_type, int section_pend)</arglist>
    </member>
    <member kind="function">
      <type>void</type>
      <name>P_clear_current_ms</name>
      <anchorfile>phase3__data_8c.html</anchorfile>
      <anchor>38b770e6213b961a4fcc0c61af9f8c84</anchor>
      <arglist>(void)</arglist>
    </member>
    <member kind="function">
      <type>void</type>
      <name>P_process_data</name>
      <anchorfile>phase3__data_8c.html</anchorfile>
      <anchor>3f2c09ee4cdbf79904493f05cc9d2719</anchor>
      <arglist>(FILE *F_OUT, L_Data *data)</arglist>
    </member>
    <member kind="variable" static="yes">
      <type>static int</type>
      <name>P_section_curr</name>
      <anchorfile>phase3__data_8c.html</anchorfile>
      <anchor>3cdcf7a93f7b06f1756cd20bc01e3e68</anchor>
      <arglist></arglist>
    </member>
    <member kind="variable" static="yes">
      <type>static int</type>
      <name>P_section_pend</name>
      <anchorfile>phase3__data_8c.html</anchorfile>
      <anchor>bd81c9c71f659b2a94f33ecc27df9b63</anchor>
      <arglist></arglist>
    </member>
  </compound>
  <compound kind="file">
    <name>phase3_func.c</name>
    <path>/mnt/hgfs/mkt/Desktop/openimpact/src/Lcode/codegen/Ltahoe/</path>
    <filename>phase3__func_8c</filename>
    <includes id="ltahoe__main_8h" name="ltahoe_main.h" local="yes" imported="no">ltahoe_main.h</includes>
    <includes id="ltahoe__op__query_8h" name="ltahoe_op_query.h" local="yes" imported="no">ltahoe_op_query.h</includes>
    <includes id="phase3_8h" name="phase3.h" local="yes" imported="no">phase3.h</includes>
    <includes id="phase3__unwind_8h" name="phase3_unwind.h" local="yes" imported="no">phase3_unwind.h</includes>
    <includes id="phase2__reg_8h" name="phase2_reg.h" local="yes" imported="no">phase2_reg.h</includes>
    <includes id="phase1__func_8h" name="phase1_func.h" local="yes" imported="no">phase1_func.h</includes>
    <includes id="phase1__varargs_8h" name="phase1_varargs.h" local="yes" imported="no">phase1_varargs.h</includes>
    <includes id="phase2__br__hint_8h" name="phase2_br_hint.h" local="yes" imported="no">phase2_br_hint.h</includes>
    <member kind="define">
      <type>#define</type>
      <name>BEST_LIVE_REGS_PER_LINE</name>
      <anchorfile>phase3__func_8c.html</anchorfile>
      <anchor>533419bdd6d4f11d1c7ca13f92afcf51</anchor>
      <arglist></arglist>
    </member>
    <member kind="define">
      <type>#define</type>
      <name>MAX_LIVE_REGS_PER_LINE</name>
      <anchorfile>phase3__func_8c.html</anchorfile>
      <anchor>ec56ed8a93f442ffe87699e1ae8b9b90</anchor>
      <arglist></arglist>
    </member>
    <member kind="define">
      <type>#define</type>
      <name>IS_ODD</name>
      <anchorfile>phase3__func_8c.html</anchorfile>
      <anchor>8dd26d1a8c0ceb8c3ec552ebf6e36c3a</anchor>
      <arglist>(a)</arglist>
    </member>
    <member kind="define">
      <type>#define</type>
      <name>IS_EVEN</name>
      <anchorfile>phase3__func_8c.html</anchorfile>
      <anchor>ce957c0ec0957075011c4db46c6edc50</anchor>
      <arglist>(a)</arglist>
    </member>
    <member kind="function">
      <type>void</type>
      <name>P_init</name>
      <anchorfile>phase3__func_8c.html</anchorfile>
      <anchor>d782607017bab037639cdd6b84f2a448</anchor>
      <arglist>(Parm_Macro_List *command_line_macro_list)</arglist>
    </member>
    <member kind="function">
      <type>void</type>
      <name>P_clear_current_ms</name>
      <anchorfile>phase3__func_8c.html</anchorfile>
      <anchor>38b770e6213b961a4fcc0c61af9f8c84</anchor>
      <arglist>(void)</arglist>
    </member>
    <member kind="function">
      <type>void</type>
      <name>P_file_init</name>
      <anchorfile>phase3__func_8c.html</anchorfile>
      <anchor>4cec9d741770c44898d8d144e2111417</anchor>
      <arglist>(void)</arglist>
    </member>
    <member kind="function">
      <type>void</type>
      <name>P_end</name>
      <anchorfile>phase3__func_8c.html</anchorfile>
      <anchor>306e6aeafa6c8c0d30c792ab6e5ad680</anchor>
      <arglist>(void)</arglist>
    </member>
    <member kind="function">
      <type>void</type>
      <name>P_file_end</name>
      <anchorfile>phase3__func_8c.html</anchorfile>
      <anchor>f644eeae2f7030c2de8032c785446fe3</anchor>
      <arglist>(void)</arglist>
    </member>
    <member kind="function" static="yes">
      <type>static char *</type>
      <name>P_int_reg_name</name>
      <anchorfile>phase3__func_8c.html</anchorfile>
      <anchor>5261a32b2177c531025ec0a8554b67be</anchor>
      <arglist>(int reg_id)</arglist>
    </member>
    <member kind="function" static="yes">
      <type>static char *</type>
      <name>P_macro_name</name>
      <anchorfile>phase3__func_8c.html</anchorfile>
      <anchor>49fb19d171cbd581199255d3f153730a</anchor>
      <arglist>(int reg_id)</arglist>
    </member>
    <member kind="function">
      <type>void</type>
      <name>P_print_register_set</name>
      <anchorfile>phase3__func_8c.html</anchorfile>
      <anchor>1f4b659497fef66276ec7086b2a0fc5e</anchor>
      <arglist>(Set reg_set)</arglist>
    </member>
    <member kind="function">
      <type>void</type>
      <name>P_print_cb_info</name>
      <anchorfile>phase3__func_8c.html</anchorfile>
      <anchor>ff8189ebbf003b5efeb1682d969a3ad4</anchor>
      <arglist>(L_Func *fn, L_Cb *cb)</arglist>
    </member>
    <member kind="function">
      <type>void</type>
      <name>P_force_recovery_execution</name>
      <anchorfile>phase3__func_8c.html</anchorfile>
      <anchor>ec905eb4bc065d6e5a62c6b8161f268c</anchor>
      <arglist>(L_Func *fn)</arglist>
    </member>
    <member kind="function">
      <type>void</type>
      <name>P_process_func</name>
      <anchorfile>phase3__func_8c.html</anchorfile>
      <anchor>dd6f2ad9943286f3c6f36049c51977dc</anchor>
      <arglist>(L_Func *fn)</arglist>
    </member>
    <member kind="variable">
      <type>int</type>
      <name>num_reg_stack_inputs</name>
      <anchorfile>phase3__func_8c.html</anchorfile>
      <anchor>8ae51a9093172ee479dfec5e1cc39e31</anchor>
      <arglist></arglist>
    </member>
    <member kind="variable">
      <type>int</type>
      <name>num_reg_stack_locals</name>
      <anchorfile>phase3__func_8c.html</anchorfile>
      <anchor>a81f50b041ddc4292e3762f6f883592e</anchor>
      <arglist></arglist>
    </member>
    <member kind="variable">
      <type>int</type>
      <name>num_reg_stack_outputs</name>
      <anchorfile>phase3__func_8c.html</anchorfile>
      <anchor>a976e684b5d8b88de9b2a2c454f9b5ba</anchor>
      <arglist></arglist>
    </member>
    <member kind="variable">
      <type>int</type>
      <name>num_reg_stack_rots</name>
      <anchorfile>phase3__func_8c.html</anchorfile>
      <anchor>f731aa3c8a9b748e2b069c97ffc795a0</anchor>
      <arglist></arglist>
    </member>
    <member kind="variable" static="yes">
      <type>static char</type>
      <name>P_macro_buf</name>
      <anchorfile>phase3__func_8c.html</anchorfile>
      <anchor>d526ffcba9caafb7f21f3fbcdfe0e20d</anchor>
      <arglist>[16]</arglist>
    </member>
  </compound>
  <compound kind="file">
    <name>phase3_load.c</name>
    <path>/mnt/hgfs/mkt/Desktop/openimpact/src/Lcode/codegen/Ltahoe/</path>
    <filename>phase3__load_8c</filename>
    <includes id="ltahoe__main_8h" name="ltahoe_main.h" local="yes" imported="no">ltahoe_main.h</includes>
    <includes id="ltahoe__completers_8h" name="ltahoe_completers.h" local="yes" imported="no">ltahoe_completers.h</includes>
    <includes id="ltahoe__table_8h" name="ltahoe_table.h" local="yes" imported="no">ltahoe_table.h</includes>
    <includes id="phase3_8h" name="phase3.h" local="yes" imported="no">phase3.h</includes>
    <member kind="function">
      <type>void</type>
      <name>P_print_load_table</name>
      <anchorfile>phase3__load_8c.html</anchorfile>
      <anchor>0b7778744ba7ba16808455d0e51e9e3c</anchor>
      <arglist>(L_Oper *oper, L_Cb *cb, unsigned int instr_offset, int slot_no)</arglist>
    </member>
  </compound>
  <compound kind="file">
    <name>phase3_oper.c</name>
    <path>/mnt/hgfs/mkt/Desktop/openimpact/src/Lcode/codegen/Ltahoe/</path>
    <filename>phase3__oper_8c</filename>
    <includes id="ltahoe__main_8h" name="ltahoe_main.h" local="yes" imported="no">ltahoe_main.h</includes>
    <includes id="ltahoe__table_8h" name="ltahoe_table.h" local="yes" imported="no">ltahoe_table.h</includes>
    <includes id="ltahoe__completers_8h" name="ltahoe_completers.h" local="yes" imported="no">ltahoe_completers.h</includes>
    <includes id="ltahoe__op__query_8h" name="ltahoe_op_query.h" local="yes" imported="no">ltahoe_op_query.h</includes>
    <includes id="phase3_8h" name="phase3.h" local="yes" imported="no">phase3.h</includes>
    <includes id="phase1__func_8h" name="phase1_func.h" local="yes" imported="no">phase1_func.h</includes>
    <includes id="phase2__func_8h" name="phase2_func.h" local="yes" imported="no">phase2_func.h</includes>
    <includes id="phase2__reg_8h" name="phase2_reg.h" local="yes" imported="no">phase2_reg.h</includes>
    <includes id="phase3__unwind_8h" name="phase3_unwind.h" local="yes" imported="no">phase3_unwind.h</includes>
    <member kind="define">
      <type>#define</type>
      <name>P_COMMENT_COL</name>
      <anchorfile>phase3__oper_8c.html</anchorfile>
      <anchor>25b029618f96713b73e181d9f9a41f20</anchor>
      <arglist></arglist>
    </member>
    <member kind="define">
      <type>#define</type>
      <name>P_TAB_SIZE</name>
      <anchorfile>phase3__oper_8c.html</anchorfile>
      <anchor>2e21206f54e02fc67cd8f0521421cd77</anchor>
      <arglist></arglist>
    </member>
    <member kind="define">
      <type>#define</type>
      <name>DEF_STR_LEN</name>
      <anchorfile>phase3__oper_8c.html</anchorfile>
      <anchor>c1ab652243daa1e27837f13a9633a33c</anchor>
      <arglist></arglist>
    </member>
    <member kind="function">
      <type>void</type>
      <name>P_line_print</name>
      <anchorfile>phase3__oper_8c.html</anchorfile>
      <anchor>b88b1a9ec3c719b325e49eb79744519d</anchor>
      <arglist>(char *format,...)</arglist>
    </member>
    <member kind="function">
      <type>void</type>
      <name>P_line_pad</name>
      <anchorfile>phase3__oper_8c.html</anchorfile>
      <anchor>17f8e1952d9f3c5ad18ab3a613c3bf61</anchor>
      <arglist>(int column)</arglist>
    </member>
    <member kind="function">
      <type>void</type>
      <name>P_set_explicit_bundling</name>
      <anchorfile>phase3__oper_8c.html</anchorfile>
      <anchor>a2f96fa4c836b58d9acdd11473b9a706</anchor>
      <arglist>(void)</arglist>
    </member>
    <member kind="function">
      <type>void</type>
      <name>P_set_implicit_bundling</name>
      <anchorfile>phase3__oper_8c.html</anchorfile>
      <anchor>8d6404cc9f626160af13aa4307cd71e2</anchor>
      <arglist>(void)</arglist>
    </member>
    <member kind="function">
      <type>void</type>
      <name>P_reset_bundle_indx</name>
      <anchorfile>phase3__oper_8c.html</anchorfile>
      <anchor>9fb6c38964813b548e42ab7000d3952a</anchor>
      <arglist>(void)</arglist>
    </member>
    <member kind="function">
      <type>void</type>
      <name>P_open_bundle</name>
      <anchorfile>phase3__oper_8c.html</anchorfile>
      <anchor>286dac5414eb809465a39733d259215d</anchor>
      <arglist>(L_Oper *oper)</arglist>
    </member>
    <member kind="function">
      <type>void</type>
      <name>P_bundle_instr</name>
      <anchorfile>phase3__oper_8c.html</anchorfile>
      <anchor>49625f015db0fe0d11d8a74a1f8b99cf</anchor>
      <arglist>(void)</arglist>
    </member>
    <member kind="function">
      <type>void</type>
      <name>P_print_comment</name>
      <anchorfile>phase3__oper_8c.html</anchorfile>
      <anchor>4c5628a8e74a3a65f7e2bb7a9af337c4</anchor>
      <arglist>(L_Oper *oper, unsigned int instr_offset, unsigned int issue_cycle)</arglist>
    </member>
    <member kind="function">
      <type>void</type>
      <name>P_print_int_reg_name</name>
      <anchorfile>phase3__oper_8c.html</anchorfile>
      <anchor>1e6c608f784fe6314e9f1cee88549763</anchor>
      <arglist>(int real_reg_id)</arglist>
    </member>
    <member kind="function">
      <type>void</type>
      <name>P_print_predicate</name>
      <anchorfile>phase3__oper_8c.html</anchorfile>
      <anchor>517d01e9e6b7dabd1d96aacb208dd601</anchor>
      <arglist>(L_Oper *oper)</arglist>
    </member>
    <member kind="function">
      <type>void</type>
      <name>P_print_register_operand_asm</name>
      <anchorfile>phase3__oper_8c.html</anchorfile>
      <anchor>0974521d9b77c3158e00fbcbe60f491e</anchor>
      <arglist>(L_Operand *operand, int real)</arglist>
    </member>
    <member kind="function">
      <type>void</type>
      <name>P_print_macro_operand_asm</name>
      <anchorfile>phase3__oper_8c.html</anchorfile>
      <anchor>de693f149a8e5642e742509056b59923</anchor>
      <arglist>(L_Oper *oper, L_Operand *operand, int real)</arglist>
    </member>
    <member kind="function">
      <type>void</type>
      <name>P_print_immed_operand_asm</name>
      <anchorfile>phase3__oper_8c.html</anchorfile>
      <anchor>ac77bbae0b58719bf8a8ab4b6c244e84</anchor>
      <arglist>(L_Oper *oper, L_Operand *operand)</arglist>
    </member>
    <member kind="function">
      <type>void</type>
      <name>P_print_label_operand_asm</name>
      <anchorfile>phase3__oper_8c.html</anchorfile>
      <anchor>e1c79f0969dece58a0f989d095d89ee1</anchor>
      <arglist>(L_Oper *oper, L_Operand *operand)</arglist>
    </member>
    <member kind="function">
      <type>void</type>
      <name>P_print_operand_asm</name>
      <anchorfile>phase3__oper_8c.html</anchorfile>
      <anchor>38aa88a40078e1ab5d12d503413f1639</anchor>
      <arglist>(L_Oper *oper, L_Operand *operand)</arglist>
    </member>
    <member kind="function">
      <type>void</type>
      <name>P_print_var_operand_asm</name>
      <anchorfile>phase3__oper_8c.html</anchorfile>
      <anchor>d4f0bbd099c192aeefd345c0b0317294</anchor>
      <arglist>(L_Oper *oper, L_Operand *operand, int real)</arglist>
    </member>
    <member kind="function">
      <type>void</type>
      <name>P_convert_reg_nums_operand</name>
      <anchorfile>phase3__oper_8c.html</anchorfile>
      <anchor>336085b049207ed7ea584811cf3cd8c2</anchor>
      <arglist>(L_Oper *oper, L_Operand *operand)</arglist>
    </member>
    <member kind="function">
      <type>void</type>
      <name>P_convert_reg_nums</name>
      <anchorfile>phase3__oper_8c.html</anchorfile>
      <anchor>46be539d3369f6d42da520278ecb40f4</anchor>
      <arglist>(L_Func *fn)</arglist>
    </member>
    <member kind="function">
      <type>void</type>
      <name>P_print_define_oper</name>
      <anchorfile>phase3__oper_8c.html</anchorfile>
      <anchor>1bd6eccfd7ff5613b2bbf30e78b02e49</anchor>
      <arglist>(L_Oper *oper)</arglist>
    </member>
    <member kind="function">
      <type>void</type>
      <name>P_print_nop</name>
      <anchorfile>phase3__oper_8c.html</anchorfile>
      <anchor>551736be6d603c5ac454883492d20303</anchor>
      <arglist>(L_Oper *oper)</arglist>
    </member>
    <member kind="function">
      <type>void</type>
      <name>P_print_break</name>
      <anchorfile>phase3__oper_8c.html</anchorfile>
      <anchor>8585276d57279881b1c8f74ccb3ef735</anchor>
      <arglist>(L_Oper *oper)</arglist>
    </member>
    <member kind="function">
      <type>void</type>
      <name>P_print_add</name>
      <anchorfile>phase3__oper_8c.html</anchorfile>
      <anchor>2783f06d9dab97723a0dc78255fab14f</anchor>
      <arglist>(L_Oper *oper)</arglist>
    </member>
    <member kind="function">
      <type>void</type>
      <name>P_print_load</name>
      <anchorfile>phase3__oper_8c.html</anchorfile>
      <anchor>00705ee253f8241d0497cb043cc6c377</anchor>
      <arglist>(L_Oper *oper)</arglist>
    </member>
    <member kind="function">
      <type>void</type>
      <name>P_print_fill_int_load</name>
      <anchorfile>phase3__oper_8c.html</anchorfile>
      <anchor>89ebaf20864266b3bdd71558149e13e7</anchor>
      <arglist>(L_Oper *oper)</arglist>
    </member>
    <member kind="function">
      <type>void</type>
      <name>P_print_lfetch</name>
      <anchorfile>phase3__oper_8c.html</anchorfile>
      <anchor>aa246f0abedbaadbd7dfe2fae164ec6b</anchor>
      <arglist>(L_Oper *oper)</arglist>
    </member>
    <member kind="function">
      <type>void</type>
      <name>P_print_check</name>
      <anchorfile>phase3__oper_8c.html</anchorfile>
      <anchor>46d4fa94a3caf2def142c07bd3315bfd</anchor>
      <arglist>(L_Oper *oper)</arglist>
    </member>
    <member kind="function">
      <type>void</type>
      <name>P_print_fchkf</name>
      <anchorfile>phase3__oper_8c.html</anchorfile>
      <anchor>3bd3ef3b93d9784c7a53e6fdc52204f5</anchor>
      <arglist>(L_Oper *oper)</arglist>
    </member>
    <member kind="function">
      <type>void</type>
      <name>P_print_fclrf</name>
      <anchorfile>phase3__oper_8c.html</anchorfile>
      <anchor>21773690ec22217b49425d7c3462b425</anchor>
      <arglist>(L_Oper *oper)</arglist>
    </member>
    <member kind="function">
      <type>void</type>
      <name>P_print_fsetc</name>
      <anchorfile>phase3__oper_8c.html</anchorfile>
      <anchor>f315a4eef304608fe27c58754897e30a</anchor>
      <arglist>(L_Oper *oper)</arglist>
    </member>
    <member kind="function">
      <type>void</type>
      <name>P_print_branch</name>
      <anchorfile>phase3__oper_8c.html</anchorfile>
      <anchor>b2ba562888e38b29a9f9ae4c50f9ffd0</anchor>
      <arglist>(L_Oper *oper)</arglist>
    </member>
    <member kind="function">
      <type>void</type>
      <name>P_print_branch_hint</name>
      <anchorfile>phase3__oper_8c.html</anchorfile>
      <anchor>c1839e53abec9c272bbfb6fe48559153</anchor>
      <arglist>(L_Oper *oper)</arglist>
    </member>
    <member kind="function">
      <type>void</type>
      <name>P_print_mnemonic_only</name>
      <anchorfile>phase3__oper_8c.html</anchorfile>
      <anchor>ffcd8f4191e9fd8bd418587104867fff</anchor>
      <arglist>(L_Oper *oper)</arglist>
    </member>
    <member kind="function">
      <type>void</type>
      <name>P_print_cmpxchg</name>
      <anchorfile>phase3__oper_8c.html</anchorfile>
      <anchor>b87e0f6d5def5f31ac208d9a763ff0cd</anchor>
      <arglist>(L_Oper *oper)</arglist>
    </member>
    <member kind="function">
      <type>void</type>
      <name>P_print_fetchadd_xchg</name>
      <anchorfile>phase3__oper_8c.html</anchorfile>
      <anchor>2cbe191f09f282a3d88eec0a40458de5</anchor>
      <arglist>(L_Oper *oper)</arglist>
    </member>
    <member kind="function">
      <type>void</type>
      <name>P_print_store</name>
      <anchorfile>phase3__oper_8c.html</anchorfile>
      <anchor>a7f37714988a92c13684e42e8c3258ce</anchor>
      <arglist>(L_Oper *oper)</arglist>
    </member>
    <member kind="function">
      <type>void</type>
      <name>P_print_invala</name>
      <anchorfile>phase3__oper_8c.html</anchorfile>
      <anchor>4e7921974d8e59569df0f66cc7588dab</anchor>
      <arglist>(L_Oper *oper)</arglist>
    </member>
    <member kind="function">
      <type>void</type>
      <name>P_print_cc</name>
      <anchorfile>phase3__oper_8c.html</anchorfile>
      <anchor>030d6adf47eb0e80226105085a27ca18</anchor>
      <arglist>(L_Oper *oper)</arglist>
    </member>
    <member kind="function">
      <type>void</type>
      <name>P_print_halt</name>
      <anchorfile>phase3__oper_8c.html</anchorfile>
      <anchor>0850d2f66b721e67963b460e9c472f99</anchor>
      <arglist>(L_Oper *oper)</arglist>
    </member>
    <member kind="function">
      <type>void</type>
      <name>P_print_alloc</name>
      <anchorfile>phase3__oper_8c.html</anchorfile>
      <anchor>52d6a7966bf670aec8b55b017a4e350c</anchor>
      <arglist>(L_Oper *oper)</arglist>
    </member>
    <member kind="function">
      <type>void</type>
      <name>P_print_fc</name>
      <anchorfile>phase3__oper_8c.html</anchorfile>
      <anchor>4eebe4af1e933f508988ad68f27eb83b</anchor>
      <arglist>(L_Oper *oper)</arglist>
    </member>
    <member kind="function">
      <type>void</type>
      <name>P_print_itc</name>
      <anchorfile>phase3__oper_8c.html</anchorfile>
      <anchor>ccb687f452f8e4ddbf5e01701a6d8561</anchor>
      <arglist>(L_Oper *oper)</arglist>
    </member>
    <member kind="function">
      <type>void</type>
      <name>P_print_itr</name>
      <anchorfile>phase3__oper_8c.html</anchorfile>
      <anchor>6017796276d6932fa0028fe49895ff9d</anchor>
      <arglist>(L_Oper *oper)</arglist>
    </member>
    <member kind="function">
      <type>void</type>
      <name>P_print_probe_fault</name>
      <anchorfile>phase3__oper_8c.html</anchorfile>
      <anchor>c1a85e2156b4d0bb61a29ad64c402d26</anchor>
      <arglist>(L_Oper *oper)</arglist>
    </member>
    <member kind="function">
      <type>void</type>
      <name>P_print_purge</name>
      <anchorfile>phase3__oper_8c.html</anchorfile>
      <anchor>04391b8eb0e740532d1a5aa5714a4df6</anchor>
      <arglist>(L_Oper *oper)</arglist>
    </member>
    <member kind="function">
      <type>void</type>
      <name>P_print_mask_op</name>
      <anchorfile>phase3__oper_8c.html</anchorfile>
      <anchor>e2ef4d108830e1f19671da8f9b423b76</anchor>
      <arglist>(L_Oper *oper)</arglist>
    </member>
    <member kind="function">
      <type>void</type>
      <name>P_print_mov_from_br</name>
      <anchorfile>phase3__oper_8c.html</anchorfile>
      <anchor>539d943d84844ac56625dd0ec090cbb4</anchor>
      <arglist>(L_Oper *oper)</arglist>
    </member>
    <member kind="function">
      <type>void</type>
      <name>P_print_mov_from_ar</name>
      <anchorfile>phase3__oper_8c.html</anchorfile>
      <anchor>c2d6d4f7b1d0b441d3e47ef773493712</anchor>
      <arglist>(L_Oper *oper)</arglist>
    </member>
    <member kind="function">
      <type>void</type>
      <name>P_print_mov_from_pr</name>
      <anchorfile>phase3__oper_8c.html</anchorfile>
      <anchor>de2f4c6475ba29144668a74013a6f90d</anchor>
      <arglist>(L_Oper *oper)</arglist>
    </member>
    <member kind="function">
      <type>int</type>
      <name>P_check_post_src_equal_dest</name>
      <anchorfile>phase3__oper_8c.html</anchorfile>
      <anchor>60cc4c1ab1597ffddde053cae3f0c482</anchor>
      <arglist>(L_Oper *oper, L_Operand *src, L_Operand *dest)</arglist>
    </member>
    <member kind="function">
      <type>void</type>
      <name>P_print_non_instr</name>
      <anchorfile>phase3__oper_8c.html</anchorfile>
      <anchor>3b9bea0887a3d5f1c6789c3e5230ee0e</anchor>
      <arglist>(L_Oper *oper)</arglist>
    </member>
    <member kind="function">
      <type>void</type>
      <name>P_fix_pred_compare_dests</name>
      <anchorfile>phase3__oper_8c.html</anchorfile>
      <anchor>10ef73f7641ed5a4dd97b78c6dcc7d29</anchor>
      <arglist>(L_Oper *oper)</arglist>
    </member>
    <member kind="function">
      <type>void</type>
      <name>P_fix_pred_compare_dests_func</name>
      <anchorfile>phase3__oper_8c.html</anchorfile>
      <anchor>fa71e00ee22caa38338eb89fe4ba919b</anchor>
      <arglist>(L_Func *fn)</arglist>
    </member>
    <member kind="function">
      <type>void</type>
      <name>P_print_pred_deps</name>
      <anchorfile>phase3__oper_8c.html</anchorfile>
      <anchor>50d3bacb61f1f5d35e36b9a559859d3c</anchor>
      <arglist>(L_Oper *oper, int softpipe)</arglist>
    </member>
    <member kind="function">
      <type>void</type>
      <name>P_print_oper</name>
      <anchorfile>phase3__oper_8c.html</anchorfile>
      <anchor>0edb830c590a4a5e79f7895e83106ff3</anchor>
      <arglist>(L_Cb *cb, L_Oper *oper, unsigned int *instr_offset, unsigned int *issue_cycle)</arglist>
    </member>
    <member kind="variable" static="yes">
      <type>static int</type>
      <name>P_line_length</name>
      <anchorfile>phase3__oper_8c.html</anchorfile>
      <anchor>8d6163c24c5cb378afbc324aebe858c8</anchor>
      <arglist></arglist>
    </member>
    <member kind="variable" static="yes">
      <type>static struct @7</type>
      <name>P_bundle_status</name>
      <anchorfile>phase3__oper_8c.html</anchorfile>
      <anchor>72fdcbf664bd7d49ad1212a1c7952318</anchor>
      <arglist></arglist>
    </member>
    <member kind="variable">
      <type>int</type>
      <name>explicit_mode</name>
      <anchorfile>struct@7.html</anchorfile>
      <anchor>3c285f6879785fd2215399ea0bd603c1</anchor>
      <arglist></arglist>
    </member>
    <member kind="variable">
      <type>int</type>
      <name>bundle_active</name>
      <anchorfile>struct@7.html</anchorfile>
      <anchor>7e154f819fe1c981dcc9d54626eef291</anchor>
      <arglist></arglist>
    </member>
    <member kind="variable">
      <type>int</type>
      <name>syllable_mask</name>
      <anchorfile>struct@7.html</anchorfile>
      <anchor>04c49466892b6e542811ee1b064b9802</anchor>
      <arglist></arglist>
    </member>
    <member kind="variable">
      <type>int</type>
      <name>stop_bit_mask</name>
      <anchorfile>struct@7.html</anchorfile>
      <anchor>a52a205a12794ef982c1e4cfb7bd50de</anchor>
      <arglist></arglist>
    </member>
    <member kind="variable">
      <type>int</type>
      <name>new_group</name>
      <anchorfile>struct@7.html</anchorfile>
      <anchor>728f1d8f9e3d40f98170638dcf90bc3f</anchor>
      <arglist></arglist>
    </member>
    <member kind="variable">
      <type>int</type>
      <name>instrs_remaining</name>
      <anchorfile>struct@7.html</anchorfile>
      <anchor>1cb4725743f87494d85a2015419e3e05</anchor>
      <arglist></arglist>
    </member>
    <member kind="variable">
      <type>int</type>
      <name>bundle_indx</name>
      <anchorfile>struct@7.html</anchorfile>
      <anchor>d3391f1bf24b2fd34ff7e7333f4c2273</anchor>
      <arglist></arglist>
    </member>
  </compound>
  <compound kind="file">
    <name>phase3_oper.h</name>
    <path>/mnt/hgfs/mkt/Desktop/openimpact/src/Lcode/codegen/Ltahoe/</path>
    <filename>phase3__oper_8h</filename>
    <member kind="function">
      <type>void</type>
      <name>P_line_print</name>
      <anchorfile>phase3__oper_8h.html</anchorfile>
      <anchor>b88b1a9ec3c719b325e49eb79744519d</anchor>
      <arglist>(char *format,...)</arglist>
    </member>
    <member kind="function">
      <type>void</type>
      <name>P_line_pad</name>
      <anchorfile>phase3__oper_8h.html</anchorfile>
      <anchor>17f8e1952d9f3c5ad18ab3a613c3bf61</anchor>
      <arglist>(int column)</arglist>
    </member>
    <member kind="function">
      <type>void</type>
      <name>P_set_explicit_bundling</name>
      <anchorfile>phase3__oper_8h.html</anchorfile>
      <anchor>a2f96fa4c836b58d9acdd11473b9a706</anchor>
      <arglist>(void)</arglist>
    </member>
    <member kind="function">
      <type>void</type>
      <name>P_set_implicit_bundling</name>
      <anchorfile>phase3__oper_8h.html</anchorfile>
      <anchor>8d6404cc9f626160af13aa4307cd71e2</anchor>
      <arglist>(void)</arglist>
    </member>
    <member kind="function">
      <type>void</type>
      <name>P_reset_bundle_indx</name>
      <anchorfile>phase3__oper_8h.html</anchorfile>
      <anchor>9fb6c38964813b548e42ab7000d3952a</anchor>
      <arglist>(void)</arglist>
    </member>
    <member kind="function">
      <type>void</type>
      <name>P_open_bundle</name>
      <anchorfile>phase3__oper_8h.html</anchorfile>
      <anchor>286dac5414eb809465a39733d259215d</anchor>
      <arglist>(L_Oper *oper)</arglist>
    </member>
    <member kind="function">
      <type>void</type>
      <name>P_bundle_instr</name>
      <anchorfile>phase3__oper_8h.html</anchorfile>
      <anchor>49625f015db0fe0d11d8a74a1f8b99cf</anchor>
      <arglist>(void)</arglist>
    </member>
    <member kind="function">
      <type>void</type>
      <name>P_print_comment</name>
      <anchorfile>phase3__oper_8h.html</anchorfile>
      <anchor>4c5628a8e74a3a65f7e2bb7a9af337c4</anchor>
      <arglist>(L_Oper *oper, unsigned int instr_offset, unsigned int issue_cycle)</arglist>
    </member>
    <member kind="function">
      <type>void</type>
      <name>P_print_int_reg_name</name>
      <anchorfile>phase3__oper_8h.html</anchorfile>
      <anchor>1e6c608f784fe6314e9f1cee88549763</anchor>
      <arglist>(int real_reg_id)</arglist>
    </member>
    <member kind="function">
      <type>void</type>
      <name>P_print_predicate</name>
      <anchorfile>phase3__oper_8h.html</anchorfile>
      <anchor>517d01e9e6b7dabd1d96aacb208dd601</anchor>
      <arglist>(L_Oper *oper)</arglist>
    </member>
    <member kind="function">
      <type>void</type>
      <name>P_print_register_operand_asm</name>
      <anchorfile>phase3__oper_8h.html</anchorfile>
      <anchor>0974521d9b77c3158e00fbcbe60f491e</anchor>
      <arglist>(L_Operand *operand, int real)</arglist>
    </member>
    <member kind="function">
      <type>void</type>
      <name>P_print_macro_operand_asm</name>
      <anchorfile>phase3__oper_8h.html</anchorfile>
      <anchor>de693f149a8e5642e742509056b59923</anchor>
      <arglist>(L_Oper *oper, L_Operand *operand, int real)</arglist>
    </member>
    <member kind="function">
      <type>void</type>
      <name>P_print_immed_operand_asm</name>
      <anchorfile>phase3__oper_8h.html</anchorfile>
      <anchor>ac77bbae0b58719bf8a8ab4b6c244e84</anchor>
      <arglist>(L_Oper *oper, L_Operand *operand)</arglist>
    </member>
    <member kind="function">
      <type>void</type>
      <name>P_print_label_operand_asm</name>
      <anchorfile>phase3__oper_8h.html</anchorfile>
      <anchor>e1c79f0969dece58a0f989d095d89ee1</anchor>
      <arglist>(L_Oper *oper, L_Operand *operand)</arglist>
    </member>
    <member kind="function">
      <type>void</type>
      <name>P_print_operand_asm</name>
      <anchorfile>phase3__oper_8h.html</anchorfile>
      <anchor>38aa88a40078e1ab5d12d503413f1639</anchor>
      <arglist>(L_Oper *oper, L_Operand *operand)</arglist>
    </member>
    <member kind="function">
      <type>void</type>
      <name>P_print_var_operand_asm</name>
      <anchorfile>phase3__oper_8h.html</anchorfile>
      <anchor>d4f0bbd099c192aeefd345c0b0317294</anchor>
      <arglist>(L_Oper *oper, L_Operand *operand, int real)</arglist>
    </member>
    <member kind="function">
      <type>void</type>
      <name>P_convert_reg_nums_operand</name>
      <anchorfile>phase3__oper_8h.html</anchorfile>
      <anchor>336085b049207ed7ea584811cf3cd8c2</anchor>
      <arglist>(L_Oper *oper, L_Operand *operand)</arglist>
    </member>
    <member kind="function">
      <type>void</type>
      <name>P_convert_reg_nums</name>
      <anchorfile>phase3__oper_8h.html</anchorfile>
      <anchor>46be539d3369f6d42da520278ecb40f4</anchor>
      <arglist>(L_Func *fn)</arglist>
    </member>
    <member kind="function">
      <type>void</type>
      <name>P_print_define_oper</name>
      <anchorfile>phase3__oper_8h.html</anchorfile>
      <anchor>1bd6eccfd7ff5613b2bbf30e78b02e49</anchor>
      <arglist>(L_Oper *oper)</arglist>
    </member>
    <member kind="function">
      <type>void</type>
      <name>P_print_nop</name>
      <anchorfile>phase3__oper_8h.html</anchorfile>
      <anchor>551736be6d603c5ac454883492d20303</anchor>
      <arglist>(L_Oper *oper)</arglist>
    </member>
    <member kind="function">
      <type>void</type>
      <name>P_print_break</name>
      <anchorfile>phase3__oper_8h.html</anchorfile>
      <anchor>8585276d57279881b1c8f74ccb3ef735</anchor>
      <arglist>(L_Oper *oper)</arglist>
    </member>
    <member kind="function">
      <type>void</type>
      <name>P_print_add</name>
      <anchorfile>phase3__oper_8h.html</anchorfile>
      <anchor>2783f06d9dab97723a0dc78255fab14f</anchor>
      <arglist>(L_Oper *oper)</arglist>
    </member>
    <member kind="function">
      <type>void</type>
      <name>P_print_load</name>
      <anchorfile>phase3__oper_8h.html</anchorfile>
      <anchor>00705ee253f8241d0497cb043cc6c377</anchor>
      <arglist>(L_Oper *oper)</arglist>
    </member>
    <member kind="function">
      <type>void</type>
      <name>P_print_fill_int_load</name>
      <anchorfile>phase3__oper_8h.html</anchorfile>
      <anchor>89ebaf20864266b3bdd71558149e13e7</anchor>
      <arglist>(L_Oper *oper)</arglist>
    </member>
    <member kind="function">
      <type>void</type>
      <name>P_print_lfetch</name>
      <anchorfile>phase3__oper_8h.html</anchorfile>
      <anchor>aa246f0abedbaadbd7dfe2fae164ec6b</anchor>
      <arglist>(L_Oper *oper)</arglist>
    </member>
    <member kind="function">
      <type>void</type>
      <name>P_print_check</name>
      <anchorfile>phase3__oper_8h.html</anchorfile>
      <anchor>46d4fa94a3caf2def142c07bd3315bfd</anchor>
      <arglist>(L_Oper *oper)</arglist>
    </member>
    <member kind="function">
      <type>void</type>
      <name>P_print_fchkf</name>
      <anchorfile>phase3__oper_8h.html</anchorfile>
      <anchor>3bd3ef3b93d9784c7a53e6fdc52204f5</anchor>
      <arglist>(L_Oper *oper)</arglist>
    </member>
    <member kind="function">
      <type>void</type>
      <name>P_print_fclrf</name>
      <anchorfile>phase3__oper_8h.html</anchorfile>
      <anchor>21773690ec22217b49425d7c3462b425</anchor>
      <arglist>(L_Oper *oper)</arglist>
    </member>
    <member kind="function">
      <type>void</type>
      <name>P_print_fsetc</name>
      <anchorfile>phase3__oper_8h.html</anchorfile>
      <anchor>f315a4eef304608fe27c58754897e30a</anchor>
      <arglist>(L_Oper *oper)</arglist>
    </member>
    <member kind="function">
      <type>void</type>
      <name>P_print_branch</name>
      <anchorfile>phase3__oper_8h.html</anchorfile>
      <anchor>b2ba562888e38b29a9f9ae4c50f9ffd0</anchor>
      <arglist>(L_Oper *oper)</arglist>
    </member>
    <member kind="function">
      <type>void</type>
      <name>P_print_branch_hint</name>
      <anchorfile>phase3__oper_8h.html</anchorfile>
      <anchor>c1839e53abec9c272bbfb6fe48559153</anchor>
      <arglist>(L_Oper *oper)</arglist>
    </member>
    <member kind="function">
      <type>void</type>
      <name>P_print_mnemonic_only</name>
      <anchorfile>phase3__oper_8h.html</anchorfile>
      <anchor>ffcd8f4191e9fd8bd418587104867fff</anchor>
      <arglist>(L_Oper *oper)</arglist>
    </member>
    <member kind="function">
      <type>void</type>
      <name>P_print_cmpxchg</name>
      <anchorfile>phase3__oper_8h.html</anchorfile>
      <anchor>b87e0f6d5def5f31ac208d9a763ff0cd</anchor>
      <arglist>(L_Oper *oper)</arglist>
    </member>
    <member kind="function">
      <type>void</type>
      <name>P_print_fetchadd_xchg</name>
      <anchorfile>phase3__oper_8h.html</anchorfile>
      <anchor>2cbe191f09f282a3d88eec0a40458de5</anchor>
      <arglist>(L_Oper *oper)</arglist>
    </member>
    <member kind="function">
      <type>void</type>
      <name>P_print_store</name>
      <anchorfile>phase3__oper_8h.html</anchorfile>
      <anchor>a7f37714988a92c13684e42e8c3258ce</anchor>
      <arglist>(L_Oper *oper)</arglist>
    </member>
    <member kind="function">
      <type>void</type>
      <name>P_print_invala</name>
      <anchorfile>phase3__oper_8h.html</anchorfile>
      <anchor>4e7921974d8e59569df0f66cc7588dab</anchor>
      <arglist>(L_Oper *oper)</arglist>
    </member>
    <member kind="function">
      <type>void</type>
      <name>P_print_cc</name>
      <anchorfile>phase3__oper_8h.html</anchorfile>
      <anchor>030d6adf47eb0e80226105085a27ca18</anchor>
      <arglist>(L_Oper *oper)</arglist>
    </member>
    <member kind="function">
      <type>void</type>
      <name>P_print_halt</name>
      <anchorfile>phase3__oper_8h.html</anchorfile>
      <anchor>0850d2f66b721e67963b460e9c472f99</anchor>
      <arglist>(L_Oper *oper)</arglist>
    </member>
    <member kind="function">
      <type>void</type>
      <name>P_print_alloc</name>
      <anchorfile>phase3__oper_8h.html</anchorfile>
      <anchor>52d6a7966bf670aec8b55b017a4e350c</anchor>
      <arglist>(L_Oper *oper)</arglist>
    </member>
    <member kind="function">
      <type>void</type>
      <name>P_print_fc</name>
      <anchorfile>phase3__oper_8h.html</anchorfile>
      <anchor>4eebe4af1e933f508988ad68f27eb83b</anchor>
      <arglist>(L_Oper *oper)</arglist>
    </member>
    <member kind="function">
      <type>void</type>
      <name>P_print_itc</name>
      <anchorfile>phase3__oper_8h.html</anchorfile>
      <anchor>ccb687f452f8e4ddbf5e01701a6d8561</anchor>
      <arglist>(L_Oper *oper)</arglist>
    </member>
    <member kind="function">
      <type>void</type>
      <name>P_print_itr</name>
      <anchorfile>phase3__oper_8h.html</anchorfile>
      <anchor>6017796276d6932fa0028fe49895ff9d</anchor>
      <arglist>(L_Oper *oper)</arglist>
    </member>
    <member kind="function">
      <type>void</type>
      <name>P_print_probe_fault</name>
      <anchorfile>phase3__oper_8h.html</anchorfile>
      <anchor>c1a85e2156b4d0bb61a29ad64c402d26</anchor>
      <arglist>(L_Oper *oper)</arglist>
    </member>
    <member kind="function">
      <type>void</type>
      <name>P_print_purge</name>
      <anchorfile>phase3__oper_8h.html</anchorfile>
      <anchor>04391b8eb0e740532d1a5aa5714a4df6</anchor>
      <arglist>(L_Oper *oper)</arglist>
    </member>
    <member kind="function">
      <type>void</type>
      <name>P_print_mask_op</name>
      <anchorfile>phase3__oper_8h.html</anchorfile>
      <anchor>e2ef4d108830e1f19671da8f9b423b76</anchor>
      <arglist>(L_Oper *oper)</arglist>
    </member>
    <member kind="function">
      <type>void</type>
      <name>P_print_mov_from_br</name>
      <anchorfile>phase3__oper_8h.html</anchorfile>
      <anchor>539d943d84844ac56625dd0ec090cbb4</anchor>
      <arglist>(L_Oper *oper)</arglist>
    </member>
    <member kind="function">
      <type>void</type>
      <name>P_print_mov_from_ar</name>
      <anchorfile>phase3__oper_8h.html</anchorfile>
      <anchor>c2d6d4f7b1d0b441d3e47ef773493712</anchor>
      <arglist>(L_Oper *oper)</arglist>
    </member>
    <member kind="function">
      <type>void</type>
      <name>P_print_mov_from_pr</name>
      <anchorfile>phase3__oper_8h.html</anchorfile>
      <anchor>de2f4c6475ba29144668a74013a6f90d</anchor>
      <arglist>(L_Oper *oper)</arglist>
    </member>
    <member kind="function">
      <type>int</type>
      <name>P_check_post_src_equal_dest</name>
      <anchorfile>phase3__oper_8h.html</anchorfile>
      <anchor>60cc4c1ab1597ffddde053cae3f0c482</anchor>
      <arglist>(L_Oper *oper, L_Operand *src, L_Operand *dest)</arglist>
    </member>
    <member kind="function">
      <type>void</type>
      <name>P_print_non_instr</name>
      <anchorfile>phase3__oper_8h.html</anchorfile>
      <anchor>3b9bea0887a3d5f1c6789c3e5230ee0e</anchor>
      <arglist>(L_Oper *oper)</arglist>
    </member>
    <member kind="function">
      <type>void</type>
      <name>P_fix_pred_compare_dests</name>
      <anchorfile>phase3__oper_8h.html</anchorfile>
      <anchor>10ef73f7641ed5a4dd97b78c6dcc7d29</anchor>
      <arglist>(L_Oper *oper)</arglist>
    </member>
    <member kind="function">
      <type>void</type>
      <name>P_fix_pred_compare_dests_func</name>
      <anchorfile>phase3__oper_8h.html</anchorfile>
      <anchor>fa71e00ee22caa38338eb89fe4ba919b</anchor>
      <arglist>(L_Func *fn)</arglist>
    </member>
    <member kind="function">
      <type>void</type>
      <name>P_print_pred_deps</name>
      <anchorfile>phase3__oper_8h.html</anchorfile>
      <anchor>50d3bacb61f1f5d35e36b9a559859d3c</anchor>
      <arglist>(L_Oper *oper, int softpipe)</arglist>
    </member>
    <member kind="function">
      <type>void</type>
      <name>P_print_oper</name>
      <anchorfile>phase3__oper_8h.html</anchorfile>
      <anchor>0edb830c590a4a5e79f7895e83106ff3</anchor>
      <arglist>(L_Cb *cb, L_Oper *oper, unsigned int *instr_offset, unsigned int *issue_cycle)</arglist>
    </member>
  </compound>
  <compound kind="file">
    <name>phase3_symbol.c</name>
    <path>/mnt/hgfs/mkt/Desktop/openimpact/src/Lcode/codegen/Ltahoe/</path>
    <filename>phase3__symbol_8c</filename>
    <includes id="ltahoe__main_8h" name="ltahoe_main.h" local="yes" imported="no">ltahoe_main.h</includes>
    <includes id="phase3_8h" name="phase3.h" local="yes" imported="no">phase3.h</includes>
    <member kind="define">
      <type>#define</type>
      <name>P_SYM_TY_OBJECT</name>
      <anchorfile>phase3__symbol_8c.html</anchorfile>
      <anchor>eb9cc307a94d57595033c6d7946aacc8</anchor>
      <arglist></arglist>
    </member>
    <member kind="define">
      <type>#define</type>
      <name>P_SYM_TY_FUNC</name>
      <anchorfile>phase3__symbol_8c.html</anchorfile>
      <anchor>8fab79c846576e69331ab9225c52d45b</anchor>
      <arglist></arglist>
    </member>
    <member kind="define">
      <type>#define</type>
      <name>P_SYM_FL_LOCALDEF</name>
      <anchorfile>phase3__symbol_8c.html</anchorfile>
      <anchor>af390e37e501b45963f396aeaeefed10</anchor>
      <arglist></arglist>
    </member>
    <member kind="function">
      <type>void</type>
      <name>P_symtab_init</name>
      <anchorfile>phase3__symbol_8c.html</anchorfile>
      <anchor>900ce5202e4b08cce287eaa221052f4e</anchor>
      <arglist>(void)</arglist>
    </member>
    <member kind="function">
      <type>void</type>
      <name>P_symtab_deinit</name>
      <anchorfile>phase3__symbol_8c.html</anchorfile>
      <anchor>a1ef182d84239141f9f1d7a6fe89f5c9</anchor>
      <arglist>(void)</arglist>
    </member>
    <member kind="function">
      <type>void</type>
      <name>P_symtab_add_label</name>
      <anchorfile>phase3__symbol_8c.html</anchorfile>
      <anchor>4db7e24ed1149bb2540576ec0c7ab06f</anchor>
      <arglist>(char *label, int isfunc)</arglist>
    </member>
    <member kind="function">
      <type>void</type>
      <name>P_symtab_add_def</name>
      <anchorfile>phase3__symbol_8c.html</anchorfile>
      <anchor>215a27cc5b3c9bcac7d5470cda46f5d3</anchor>
      <arglist>(char *label, int isfunc)</arglist>
    </member>
    <member kind="function">
      <type>int</type>
      <name>kcmp</name>
      <anchorfile>phase3__symbol_8c.html</anchorfile>
      <anchor>5fa8021f3b27cc305dc93316d9087772</anchor>
      <arglist>(const void *p1, const void *p2)</arglist>
    </member>
    <member kind="function">
      <type>void</type>
      <name>P_symtab_print_extern</name>
      <anchorfile>phase3__symbol_8c.html</anchorfile>
      <anchor>c9e8c6bb24f5db8a05b092577c56c078</anchor>
      <arglist>(void)</arglist>
    </member>
    <member kind="variable" static="yes">
      <type>static STRING_Symbol_Table *</type>
      <name>P_symtab</name>
      <anchorfile>phase3__symbol_8c.html</anchorfile>
      <anchor>7115c750c9199bf2b967fb88e25a9b81</anchor>
      <arglist></arglist>
    </member>
  </compound>
  <compound kind="file">
    <name>phase3_unwind.c</name>
    <path>/mnt/hgfs/mkt/Desktop/openimpact/src/Lcode/codegen/Ltahoe/</path>
    <filename>phase3__unwind_8c</filename>
    <includes id="ltahoe__main_8h" name="ltahoe_main.h" local="yes" imported="no">ltahoe_main.h</includes>
    <includes id="phase2__reg_8h" name="phase2_reg.h" local="yes" imported="no">phase2_reg.h</includes>
    <includes id="phase3_8h" name="phase3.h" local="yes" imported="no">phase3.h</includes>
    <includes id="phase3__unwind_8h" name="phase3_unwind.h" local="yes" imported="no">phase3_unwind.h</includes>
    <member kind="define">
      <type>#define</type>
      <name>UPDATEFIRSTLAST</name>
      <anchorfile>phase3__unwind_8c.html</anchorfile>
      <anchor>8b25af4a152725329b8d18af33ab79ae</anchor>
      <arglist>(un, op)</arglist>
    </member>
    <member kind="define">
      <type>#define</type>
      <name>ISTMPREG1</name>
      <anchorfile>phase3__unwind_8c.html</anchorfile>
      <anchor>528f3e7812b2c03484df8ddf96214a44</anchor>
      <arglist>(opd)</arglist>
    </member>
    <member kind="function">
      <type>void</type>
      <name>L_get_unwind_info</name>
      <anchorfile>phase3__unwind_8c.html</anchorfile>
      <anchor>96799f1dde6cd476dee4185624c88a87</anchor>
      <arglist>(L_Func *fn)</arglist>
    </member>
    <member kind="variable">
      <type>unwind_info</type>
      <name>unwind</name>
      <anchorfile>phase3__unwind_8c.html</anchorfile>
      <anchor>145433b62b9e8e43811e56ac7e726f17</anchor>
      <arglist></arglist>
    </member>
  </compound>
  <compound kind="file">
    <name>phase3_unwind.h</name>
    <path>/mnt/hgfs/mkt/Desktop/openimpact/src/Lcode/codegen/Ltahoe/</path>
    <filename>phase3__unwind_8h</filename>
    <class kind="struct">_UnwindReg</class>
    <class kind="struct">unwind_info</class>
    <member kind="typedef">
      <type>_UnwindReg</type>
      <name>UnwindReg</name>
      <anchorfile>phase3__unwind_8h.html</anchorfile>
      <anchor>ba844946023f0d202fd16e6d7cfadf10</anchor>
      <arglist></arglist>
    </member>
    <member kind="function">
      <type>void</type>
      <name>L_get_unwind_info</name>
      <anchorfile>phase3__unwind_8h.html</anchorfile>
      <anchor>96799f1dde6cd476dee4185624c88a87</anchor>
      <arglist>(L_Func *fn)</arglist>
    </member>
    <member kind="variable">
      <type>unwind_info</type>
      <name>unwind</name>
      <anchorfile>phase3__unwind_8h.html</anchorfile>
      <anchor>d52a68ebc80b55d94f7517e87322aa03</anchor>
      <arglist></arglist>
    </member>
  </compound>
  <compound kind="struct">
    <name>_bh_br_hint</name>
    <filename>struct__bh__br__hint.html</filename>
    <member kind="variable">
      <type>_bh_br_hint *</type>
      <name>next_br</name>
      <anchorfile>struct__bh__br__hint.html</anchorfile>
      <anchor>d18976a73b6f8ddfdda9114e65af208c</anchor>
      <arglist></arglist>
    </member>
    <member kind="variable">
      <type>_bh_br_hint *</type>
      <name>prev_br</name>
      <anchorfile>struct__bh__br__hint.html</anchorfile>
      <anchor>6aa6572d78564afc5011ddc79e3907cf</anchor>
      <arglist></arglist>
    </member>
    <member kind="variable">
      <type>int</type>
      <name>br_type</name>
      <anchorfile>struct__bh__br__hint.html</anchorfile>
      <anchor>2b3ff96c8daf2bbfdfaaf9b8a48c0ab9</anchor>
      <arglist></arglist>
    </member>
    <member kind="variable">
      <type>int</type>
      <name>ipwh</name>
      <anchorfile>struct__bh__br__hint.html</anchorfile>
      <anchor>0a6ab021143fe92991228edf8590afea</anchor>
      <arglist></arglist>
    </member>
    <member kind="variable">
      <type>int</type>
      <name>ph</name>
      <anchorfile>struct__bh__br__hint.html</anchorfile>
      <anchor>834d74bab1e319a4bac866b27b5b001b</anchor>
      <arglist></arglist>
    </member>
    <member kind="variable">
      <type>int</type>
      <name>pvec</name>
      <anchorfile>struct__bh__br__hint.html</anchorfile>
      <anchor>5a7ddf653dbb2e7cc3dd14b9683d236d</anchor>
      <arglist></arglist>
    </member>
    <member kind="variable">
      <type>int</type>
      <name>ih</name>
      <anchorfile>struct__bh__br__hint.html</anchorfile>
      <anchor>04f6e619eb9888b5f3a1756af8d9b70d</anchor>
      <arglist></arglist>
    </member>
    <member kind="variable">
      <type>int</type>
      <name>spec_ih</name>
      <anchorfile>struct__bh__br__hint.html</anchorfile>
      <anchor>7b59230ae3eff8bddd2ca6049768f159</anchor>
      <arglist></arglist>
    </member>
    <member kind="variable">
      <type>L_Oper *</type>
      <name>br_oper</name>
      <anchorfile>struct__bh__br__hint.html</anchorfile>
      <anchor>abe30bbcee4ea9430f75c7a723a2d9c1</anchor>
      <arglist></arglist>
    </member>
    <member kind="variable">
      <type>L_Cb *</type>
      <name>br_cb</name>
      <anchorfile>struct__bh__br__hint.html</anchorfile>
      <anchor>d1a5ec1fff65b694fc97743af37826ca</anchor>
      <arglist></arglist>
    </member>
    <member kind="variable">
      <type>float</type>
      <name>prob</name>
      <anchorfile>struct__bh__br__hint.html</anchorfile>
      <anchor>7b5ce93650b6f32415089e9b0d2d0b4c</anchor>
      <arglist></arglist>
    </member>
    <member kind="variable">
      <type>float</type>
      <name>path_weight</name>
      <anchorfile>struct__bh__br__hint.html</anchorfile>
      <anchor>8a19f52d78f3802499b1cbf5ac1026d1</anchor>
      <arglist></arglist>
    </member>
    <member kind="variable">
      <type>L_Operand *</type>
      <name>target</name>
      <anchorfile>struct__bh__br__hint.html</anchorfile>
      <anchor>5ef2f666842bb16d76ca65035ab30eab</anchor>
      <arglist></arglist>
    </member>
    <member kind="variable">
      <type>L_Oper *</type>
      <name>br_bundle</name>
      <anchorfile>struct__bh__br__hint.html</anchorfile>
      <anchor>32026b6f77494722d923f8e90407f7de</anchor>
      <arglist></arglist>
    </member>
    <member kind="variable">
      <type>L_Operand *</type>
      <name>label</name>
      <anchorfile>struct__bh__br__hint.html</anchorfile>
      <anchor>7f025f2b7a88c1c2bee8cca2f2fcd438</anchor>
      <arglist></arglist>
    </member>
    <member kind="variable">
      <type>L_Operand *</type>
      <name>num_bundles</name>
      <anchorfile>struct__bh__br__hint.html</anchorfile>
      <anchor>fa9cc684eb485a65665ad1c9f4724807</anchor>
      <arglist></arglist>
    </member>
    <member kind="variable">
      <type>_bh_br_hint *</type>
      <name>advanced_hint</name>
      <anchorfile>struct__bh__br__hint.html</anchorfile>
      <anchor>724978ccb8e52ead6c8f81f95224ede7</anchor>
      <arglist></arglist>
    </member>
    <member kind="variable">
      <type>BH_hints_list *</type>
      <name>list</name>
      <anchorfile>struct__bh__br__hint.html</anchorfile>
      <anchor>969c2c93bee9b5864dbebf2947499a6c</anchor>
      <arglist></arglist>
    </member>
    <member kind="variable">
      <type>BH_hints_list *</type>
      <name>list_tail</name>
      <anchorfile>struct__bh__br__hint.html</anchorfile>
      <anchor>62dedaf226ead51a1328ba2c9ca0d9bc</anchor>
      <arglist></arglist>
    </member>
    <member kind="variable">
      <type>int</type>
      <name>tar_hinted</name>
      <anchorfile>struct__bh__br__hint.html</anchorfile>
      <anchor>0439aea1794b5a794407fb9a087d698d</anchor>
      <arglist></arglist>
    </member>
    <member kind="variable">
      <type>int</type>
      <name>num_inserted</name>
      <anchorfile>struct__bh__br__hint.html</anchorfile>
      <anchor>9846eaa8fd22025dab70010e6f8ec2e1</anchor>
      <arglist></arglist>
    </member>
    <member kind="variable">
      <type>int</type>
      <name>paths_success</name>
      <anchorfile>struct__bh__br__hint.html</anchorfile>
      <anchor>cb775e185208fcf0b1eca4e3198040db</anchor>
      <arglist></arglist>
    </member>
    <member kind="variable">
      <type>int</type>
      <name>paths_tried</name>
      <anchorfile>struct__bh__br__hint.html</anchorfile>
      <anchor>1f4b9e815c769266f29375038954f796</anchor>
      <arglist></arglist>
    </member>
    <member kind="variable">
      <type>int</type>
      <name>cross_same_call</name>
      <anchorfile>struct__bh__br__hint.html</anchorfile>
      <anchor>d4e598f8bd361baec3d629714cde165f</anchor>
      <arglist></arglist>
    </member>
  </compound>
  <compound kind="struct">
    <name>_bh_hint_path</name>
    <filename>struct__bh__hint__path.html</filename>
    <member kind="variable">
      <type>_bh_hint_path *</type>
      <name>next_cb</name>
      <anchorfile>struct__bh__hint__path.html</anchorfile>
      <anchor>076bc6ae836c52062f8686fef29104a5</anchor>
      <arglist></arglist>
    </member>
    <member kind="variable">
      <type>L_Cb *</type>
      <name>cb</name>
      <anchorfile>struct__bh__hint__path.html</anchorfile>
      <anchor>34a08c2f9e2aceb79a8cc9726689a6e7</anchor>
      <arglist></arglist>
    </member>
  </compound>
  <compound kind="struct">
    <name>_bh_hints_list</name>
    <filename>struct__bh__hints__list.html</filename>
    <member kind="variable">
      <type>_bh_hints_list *</type>
      <name>next_hint</name>
      <anchorfile>struct__bh__hints__list.html</anchorfile>
      <anchor>c9ed51e7e85516db93e58a67b7207fb6</anchor>
      <arglist></arglist>
    </member>
    <member kind="variable">
      <type>L_Oper *</type>
      <name>hint_oper</name>
      <anchorfile>struct__bh__hints__list.html</anchorfile>
      <anchor>574c7158fe7dada502dad77dd4d492f1</anchor>
      <arglist></arglist>
    </member>
    <member kind="variable">
      <type>L_Cb *</type>
      <name>hint_cb</name>
      <anchorfile>struct__bh__hints__list.html</anchorfile>
      <anchor>2bf26748c330f9d16fc32f00c6c3278b</anchor>
      <arglist></arglist>
    </member>
    <member kind="variable">
      <type>BH_path_info</type>
      <name>path_info</name>
      <anchorfile>struct__bh__hints__list.html</anchorfile>
      <anchor>41d65d0a21786de2da5dc395b5e7b6c1</anchor>
      <arglist></arglist>
    </member>
  </compound>
  <compound kind="struct">
    <name>_bh_path_info</name>
    <filename>struct__bh__path__info.html</filename>
    <member kind="variable">
      <type>BH_hint_path *</type>
      <name>path_tail</name>
      <anchorfile>struct__bh__path__info.html</anchorfile>
      <anchor>56ee3e92527da544b8591c95f25ea86b</anchor>
      <arglist></arglist>
    </member>
    <member kind="variable">
      <type>BH_hint_path *</type>
      <name>path</name>
      <anchorfile>struct__bh__path__info.html</anchorfile>
      <anchor>73993aae3c23ea57886b64ff6fd325a1</anchor>
      <arglist></arglist>
    </member>
    <member kind="variable">
      <type>int</type>
      <name>num_br_crossed</name>
      <anchorfile>struct__bh__path__info.html</anchorfile>
      <anchor>53212922a98557bec18c5bb9fb2190e3</anchor>
      <arglist></arglist>
    </member>
    <member kind="variable">
      <type>int</type>
      <name>num_calls_crossed</name>
      <anchorfile>struct__bh__path__info.html</anchorfile>
      <anchor>ce1aa0bad8cfa166cd02f7b67380d70d</anchor>
      <arglist></arglist>
    </member>
    <member kind="variable">
      <type>int</type>
      <name>num_cbs_on_path</name>
      <anchorfile>struct__bh__path__info.html</anchorfile>
      <anchor>44824c982697f17c25634bb5ce2c7a6d</anchor>
      <arglist></arglist>
    </member>
    <member kind="variable">
      <type>int</type>
      <name>fe_cycles_away</name>
      <anchorfile>struct__bh__path__info.html</anchorfile>
      <anchor>07f16e146bf744f21c47bca7ff1f1170</anchor>
      <arglist></arglist>
    </member>
    <member kind="variable">
      <type>int</type>
      <name>brps_on_path</name>
      <anchorfile>struct__bh__path__info.html</anchorfile>
      <anchor>669b7de6d66149406c2eeed6fc2212b4</anchor>
      <arglist></arglist>
    </member>
    <member kind="variable">
      <type>int</type>
      <name>tars_on_path</name>
      <anchorfile>struct__bh__path__info.html</anchorfile>
      <anchor>c6dc1dbcb61e503375dafcee117bf973</anchor>
      <arglist></arglist>
    </member>
    <member kind="variable">
      <type>int</type>
      <name>expanded_bundle</name>
      <anchorfile>struct__bh__path__info.html</anchorfile>
      <anchor>5d43899e1cea47dd59c3ff8bb7ea5ce7</anchor>
      <arglist></arglist>
    </member>
    <member kind="variable">
      <type>int</type>
      <name>direction</name>
      <anchorfile>struct__bh__path__info.html</anchorfile>
      <anchor>6c8e430a3586217b4bfe24ce154e4898</anchor>
      <arglist></arglist>
    </member>
    <member kind="variable">
      <type>int</type>
      <name>static_distance</name>
      <anchorfile>struct__bh__path__info.html</anchorfile>
      <anchor>958762cbadca4671d93b6ed3e3bf1c4b</anchor>
      <arglist></arglist>
    </member>
  </compound>
  <compound kind="struct">
    <name>_BIT_INFO</name>
    <filename>struct__BIT__INFO.html</filename>
    <member kind="variable">
      <type>LT_bit_vector</type>
      <name>knownZero</name>
      <anchorfile>struct__BIT__INFO.html</anchorfile>
      <anchor>c5b2ee448a28bea0cf8ee15568c5d2bf</anchor>
      <arglist></arglist>
    </member>
    <member kind="variable">
      <type>LT_bit_vector</type>
      <name>knownOne</name>
      <anchorfile>struct__BIT__INFO.html</anchorfile>
      <anchor>f41a62804f68021c06c6b19ac35bb424</anchor>
      <arglist></arglist>
    </member>
    <member kind="variable">
      <type>LT_bit_vector</type>
      <name>bitsKnownUnchanged</name>
      <anchorfile>struct__BIT__INFO.html</anchorfile>
      <anchor>b0798f4c2a5474db7b9057e27355b725</anchor>
      <arglist></arglist>
    </member>
    <member kind="variable">
      <type>LT_bit_vector</type>
      <name>bitsUsed</name>
      <anchorfile>struct__BIT__INFO.html</anchorfile>
      <anchor>2c76d39b8daca5a2ee26fd48fd45003f</anchor>
      <arglist></arglist>
    </member>
  </compound>
  <compound kind="struct">
    <name>_cb_info</name>
    <filename>struct__cb__info.html</filename>
    <member kind="variable">
      <type>int</type>
      <name>bundles_in_cb</name>
      <anchorfile>struct__cb__info.html</anchorfile>
      <anchor>968d4a49d15d0b342ca6ffc247edd47a</anchor>
      <arglist></arglist>
    </member>
    <member kind="variable">
      <type>int</type>
      <name>num_expanded</name>
      <anchorfile>struct__cb__info.html</anchorfile>
      <anchor>1d2d66e03de1d9fe56310d1f8cbf6e34</anchor>
      <arglist></arglist>
    </member>
    <member kind="variable">
      <type>int</type>
      <name>num_inserted</name>
      <anchorfile>struct__cb__info.html</anchorfile>
      <anchor>edf4e645be34c841eeca78b2d1ac1f89</anchor>
      <arglist></arglist>
    </member>
  </compound>
  <compound kind="struct">
    <name>_MiaExtract</name>
    <filename>struct__MiaExtract.html</filename>
    <member kind="variable">
      <type>L_Oper *</type>
      <name>oper</name>
      <anchorfile>struct__MiaExtract.html</anchorfile>
      <anchor>94380612467089705c0e48540d43ef6b</anchor>
      <arglist></arglist>
    </member>
    <member kind="variable">
      <type>L_Operand *</type>
      <name>mopd</name>
      <anchorfile>struct__MiaExtract.html</anchorfile>
      <anchor>297a8b8300aa06c9b8128b6d959df075</anchor>
      <arglist></arglist>
    </member>
    <member kind="variable">
      <type>int</type>
      <name>uns</name>
      <anchorfile>struct__MiaExtract.html</anchorfile>
      <anchor>5e58624f371ea642e175b3fbf98eae28</anchor>
      <arglist></arglist>
    </member>
    <member kind="variable">
      <type>unsigned int</type>
      <name>len</name>
      <anchorfile>struct__MiaExtract.html</anchorfile>
      <anchor>e02a2830a8e0a6b6b08125f2ad516afa</anchor>
      <arglist></arglist>
    </member>
    <member kind="variable">
      <type>unsigned int</type>
      <name>pos</name>
      <anchorfile>struct__MiaExtract.html</anchorfile>
      <anchor>91526b8adadf18001c34246d477d3e92</anchor>
      <arglist></arglist>
    </member>
  </compound>
  <compound kind="struct">
    <name>_opcode_handler</name>
    <filename>struct__opcode__handler.html</filename>
    <member kind="variable">
      <type>char</type>
      <name>opc_str</name>
      <anchorfile>struct__opcode__handler.html</anchorfile>
      <anchor>1fa673bc9242480f0305a15efb618871</anchor>
      <arglist>[30]</arglist>
    </member>
    <member kind="variable">
      <type>int(*</type>
      <name>completer_sanity_check</name>
      <anchorfile>struct__opcode__handler.html</anchorfile>
      <anchor>42326bb6504da23819909dad789de40f</anchor>
      <arglist>)(L_Oper *)</arglist>
    </member>
    <member kind="variable">
      <type>void(*</type>
      <name>print_completers</name>
      <anchorfile>struct__opcode__handler.html</anchorfile>
      <anchor>4a1513500cb3f1d2679862cf161643e0</anchor>
      <arglist>)(L_Oper *)</arglist>
    </member>
    <member kind="variable">
      <type>void(*</type>
      <name>print_opcode</name>
      <anchorfile>struct__opcode__handler.html</anchorfile>
      <anchor>88bd32de3e506edd7aae1cbfb8152c0d</anchor>
      <arglist>)(L_Oper *)</arglist>
    </member>
    <member kind="variable">
      <type>void(*</type>
      <name>ComputeKnownBitsFlow</name>
      <anchorfile>struct__opcode__handler.html</anchorfile>
      <anchor>e9e1a113e0ff53ddfed96775b23a2fc3</anchor>
      <arglist>)(L_Func *, L_Cb *, L_Oper *)</arglist>
    </member>
    <member kind="variable">
      <type>void(*</type>
      <name>ProcessOperUsingTD</name>
      <anchorfile>struct__opcode__handler.html</anchorfile>
      <anchor>ee0d6be0d7165bf32c185bf5a9966e03</anchor>
      <arglist>)(L_Func *, L_Cb *, L_Oper *)</arglist>
    </member>
    <member kind="variable">
      <type>void(*</type>
      <name>ComputeBitsUsedLocal</name>
      <anchorfile>struct__opcode__handler.html</anchorfile>
      <anchor>cd748d8081347789c2f22e115ea2f713</anchor>
      <arglist>)(L_Func *, L_Cb *, L_Oper *)</arglist>
    </member>
  </compound>
  <compound kind="struct">
    <name>_P_Data_Section</name>
    <filename>struct__P__Data__Section.html</filename>
    <member kind="variable">
      <type>int</type>
      <name>type</name>
      <anchorfile>struct__P__Data__Section.html</anchorfile>
      <anchor>9b5617857731cf5bcfeab5ed4f98bc62</anchor>
      <arglist></arglist>
    </member>
    <member kind="variable">
      <type>int</type>
      <name>global</name>
      <anchorfile>struct__P__Data__Section.html</anchorfile>
      <anchor>0bad1ab43f3b24e8dba3f87097b531e2</anchor>
      <arglist></arglist>
    </member>
    <member kind="variable">
      <type>int</type>
      <name>align</name>
      <anchorfile>struct__P__Data__Section.html</anchorfile>
      <anchor>f6999f632c89383df15747084c21f820</anchor>
      <arglist></arglist>
    </member>
    <member kind="variable">
      <type>char *</type>
      <name>label</name>
      <anchorfile>struct__P__Data__Section.html</anchorfile>
      <anchor>96e6e9355d8e14ee65232ef5418d3d24</anchor>
      <arglist></arglist>
    </member>
    <member kind="variable">
      <type>char *</type>
      <name>string</name>
      <anchorfile>struct__P__Data__Section.html</anchorfile>
      <anchor>63d9a175e7c7470ee7f04dc0c5da304a</anchor>
      <arglist></arglist>
    </member>
    <member kind="variable">
      <type>int</type>
      <name>element_size</name>
      <anchorfile>struct__P__Data__Section.html</anchorfile>
      <anchor>cc4fbbe6813c0a71559198494c9d4d55</anchor>
      <arglist></arglist>
    </member>
    <member kind="variable">
      <type>int</type>
      <name>reserve</name>
      <anchorfile>struct__P__Data__Section.html</anchorfile>
      <anchor>49c3787960d4ef54bfd3b4d705ea1801</anchor>
      <arglist></arglist>
    </member>
    <member kind="variable">
      <type>List</type>
      <name>init_data</name>
      <anchorfile>struct__P__Data__Section.html</anchorfile>
      <anchor>db0c0a9c6d88fe381d4d2156bb513ee0</anchor>
      <arglist></arglist>
    </member>
  </compound>
  <compound kind="struct">
    <name>_P_Init_Data</name>
    <filename>struct__P__Init__Data.html</filename>
    <member kind="enumvalue">
      <type>@</type>
      <name>CLASS_NONE</name>
      <anchorfile>struct__P__Init__Data.html</anchorfile>
      <anchor>831f6a35e65b3a459cdee2af9766e9de99649b446db9a093bdec0aaec416a941</anchor>
      <arglist></arglist>
    </member>
    <member kind="enumvalue">
      <type>@</type>
      <name>CLASS_INT</name>
      <anchorfile>struct__P__Init__Data.html</anchorfile>
      <anchor>831f6a35e65b3a459cdee2af9766e9deff02842946db67bd67178e160d3478da</anchor>
      <arglist></arglist>
    </member>
    <member kind="enumvalue">
      <type>@</type>
      <name>CLASS_STRING_POINTER</name>
      <anchorfile>struct__P__Init__Data.html</anchorfile>
      <anchor>831f6a35e65b3a459cdee2af9766e9deb3890a8c3b8612d4e9d816100824e8ac</anchor>
      <arglist></arglist>
    </member>
    <member kind="enumvalue">
      <type>@</type>
      <name>CLASS_LABEL_POINTER</name>
      <anchorfile>struct__P__Init__Data.html</anchorfile>
      <anchor>831f6a35e65b3a459cdee2af9766e9dec44f62512d041daddb3f92a398e9cdde</anchor>
      <arglist></arglist>
    </member>
    <member kind="enumvalue">
      <type>@</type>
      <name>CLASS_FUNCTION_POINTER</name>
      <anchorfile>struct__P__Init__Data.html</anchorfile>
      <anchor>831f6a35e65b3a459cdee2af9766e9de0d21452dca0c8d7745c9637a7ffd7813</anchor>
      <arglist></arglist>
    </member>
    <member kind="enumvalue">
      <type>@</type>
      <name>CLASS_LABEL_POINTER_PLUS_OFFSET</name>
      <anchorfile>struct__P__Init__Data.html</anchorfile>
      <anchor>831f6a35e65b3a459cdee2af9766e9deedebeebd518f880488f5398410859105</anchor>
      <arglist></arglist>
    </member>
    <member kind="enumvalue">
      <name>CLASS_NONE</name>
      <anchor>831f6a35e65b3a459cdee2af9766e9de99649b446db9a093bdec0aaec416a941</anchor>
      <arglist></arglist>
    </member>
    <member kind="enumvalue">
      <name>CLASS_INT</name>
      <anchor>831f6a35e65b3a459cdee2af9766e9deff02842946db67bd67178e160d3478da</anchor>
      <arglist></arglist>
    </member>
    <member kind="enumvalue">
      <name>CLASS_STRING_POINTER</name>
      <anchor>831f6a35e65b3a459cdee2af9766e9deb3890a8c3b8612d4e9d816100824e8ac</anchor>
      <arglist></arglist>
    </member>
    <member kind="enumvalue">
      <name>CLASS_LABEL_POINTER</name>
      <anchor>831f6a35e65b3a459cdee2af9766e9dec44f62512d041daddb3f92a398e9cdde</anchor>
      <arglist></arglist>
    </member>
    <member kind="enumvalue">
      <name>CLASS_FUNCTION_POINTER</name>
      <anchor>831f6a35e65b3a459cdee2af9766e9de0d21452dca0c8d7745c9637a7ffd7813</anchor>
      <arglist></arglist>
    </member>
    <member kind="enumvalue">
      <name>CLASS_LABEL_POINTER_PLUS_OFFSET</name>
      <anchor>831f6a35e65b3a459cdee2af9766e9deedebeebd518f880488f5398410859105</anchor>
      <arglist></arglist>
    </member>
    <member kind="variable">
      <type>int</type>
      <name>type</name>
      <anchorfile>struct__P__Init__Data.html</anchorfile>
      <anchor>7a7700144b284f79d370e6d327596e54</anchor>
      <arglist></arglist>
    </member>
    <member kind="variable">
      <type>enum _P_Init_Data::@5</type>
      <name>class</name>
      <anchorfile>struct__P__Init__Data.html</anchorfile>
      <anchor>05214f5903d786b18ac02ff53d5ad64f</anchor>
      <arglist></arglist>
    </member>
    <member kind="variable">
      <type>_P_Init_Data::@6</type>
      <name>value</name>
      <anchorfile>struct__P__Init__Data.html</anchorfile>
      <anchor>ad451ebe20ed5833ee1423277507b3c7</anchor>
      <arglist></arglist>
    </member>
    <member kind="variable">
      <type>char *</type>
      <name>string</name>
      <anchorfile>union__P__Init__Data_1_1@6.html</anchorfile>
      <anchor>7358d652639f8dfa0287edd948d37fcb</anchor>
      <arglist></arglist>
    </member>
    <member kind="variable">
      <type>ITintmax</type>
      <name>i</name>
      <anchorfile>union__P__Init__Data_1_1@6.html</anchorfile>
      <anchor>f7e77390bcdb384ca7dea05ff4f2e2bb</anchor>
      <arglist></arglist>
    </member>
    <member kind="variable">
      <type>float</type>
      <name>f</name>
      <anchorfile>union__P__Init__Data_1_1@6.html</anchorfile>
      <anchor>dd2863a40d36d995f614fa14d53283e2</anchor>
      <arglist></arglist>
    </member>
    <member kind="variable">
      <type>double</type>
      <name>f2</name>
      <anchorfile>union__P__Init__Data_1_1@6.html</anchorfile>
      <anchor>8e836727d46f4346226355ba6e8a0059</anchor>
      <arglist></arglist>
    </member>
    <member kind="variable">
      <type>int</type>
      <name>offset</name>
      <anchorfile>struct__P__Init__Data.html</anchorfile>
      <anchor>ae62dd2e4dd3e089af7eb3bceffa9597</anchor>
      <arglist></arglist>
    </member>
  </compound>
  <compound kind="struct">
    <name>_UnwindReg</name>
    <filename>struct__UnwindReg.html</filename>
    <member kind="variable">
      <type>L_Oper *</type>
      <name>save_op</name>
      <anchorfile>struct__UnwindReg.html</anchorfile>
      <anchor>83788b27b29d396b1ac2d005d6227650</anchor>
      <arglist></arglist>
    </member>
    <member kind="variable">
      <type>int</type>
      <name>in_reg</name>
      <anchorfile>struct__UnwindReg.html</anchorfile>
      <anchor>8bce6cff01dfa5026f59c07721595242</anchor>
      <arglist></arglist>
    </member>
    <member kind="variable">
      <type>_UnwindReg::@8</type>
      <name>loc</name>
      <anchorfile>struct__UnwindReg.html</anchorfile>
      <anchor>4a9f3bb42de7d3b115f99d53db7bb366</anchor>
      <arglist></arglist>
    </member>
    <member kind="variable">
      <type>L_Operand *</type>
      <name>reg</name>
      <anchorfile>union__UnwindReg_1_1@8.html</anchorfile>
      <anchor>22be0d206d0db2341bc72f7b45af0251</anchor>
      <arglist></arglist>
    </member>
    <member kind="variable">
      <type>int</type>
      <name>ofst</name>
      <anchorfile>union__UnwindReg_1_1@8.html</anchorfile>
      <anchor>1a4739a322f7ca26d14f141907578ff0</anchor>
      <arglist></arglist>
    </member>
  </compound>
  <compound kind="union">
    <name>CONVERT</name>
    <filename>unionCONVERT.html</filename>
    <member kind="variable">
      <type>float</type>
      <name>sgl</name>
      <anchorfile>unionCONVERT.html</anchorfile>
      <anchor>47a3ca8efdaafd21b35009f06b14ef71</anchor>
      <arglist></arglist>
    </member>
    <member kind="variable">
      <type>double</type>
      <name>dbl</name>
      <anchorfile>unionCONVERT.html</anchorfile>
      <anchor>de580f1e2f3657c880af543f9a687eda</anchor>
      <arglist></arglist>
    </member>
    <member kind="variable">
      <type>ITint64</type>
      <name>q</name>
      <anchorfile>unionCONVERT.html</anchorfile>
      <anchor>8c456c03278159c3e69c3f31b828da8a</anchor>
      <arglist></arglist>
    </member>
  </compound>
  <compound kind="struct">
    <name>Imul_entry</name>
    <filename>structImul__entry.html</filename>
    <member kind="variable">
      <type>char</type>
      <name>depth</name>
      <anchorfile>structImul__entry.html</anchorfile>
      <anchor>04ddbc68604fb4b18bacc4131fec774e</anchor>
      <arglist></arglist>
    </member>
    <member kind="variable">
      <type>char</type>
      <name>i_count</name>
      <anchorfile>structImul__entry.html</anchorfile>
      <anchor>60d2857b86ed0a63206cf008eaaecb7a</anchor>
      <arglist></arglist>
    </member>
    <member kind="variable">
      <type>char</type>
      <name>opcode</name>
      <anchorfile>structImul__entry.html</anchorfile>
      <anchor>9601ddca8562acfb02b7ca9b49475ab4</anchor>
      <arglist></arglist>
    </member>
    <member kind="variable">
      <type>char</type>
      <name>shift_amt</name>
      <anchorfile>structImul__entry.html</anchorfile>
      <anchor>3168018384148f15f2d9ca6d505fec3d</anchor>
      <arglist></arglist>
    </member>
    <member kind="variable">
      <type>short int</type>
      <name>shiftee</name>
      <anchorfile>structImul__entry.html</anchorfile>
      <anchor>7dd3039331ca40fcbeaf7b9b562970c9</anchor>
      <arglist></arglist>
    </member>
    <member kind="variable">
      <type>short int</type>
      <name>addend</name>
      <anchorfile>structImul__entry.html</anchorfile>
      <anchor>5a42256bf36218c332d876d414208f4b</anchor>
      <arglist></arglist>
    </member>
  </compound>
  <compound kind="struct">
    <name>ip_mac_subst_info</name>
    <filename>structip__mac__subst__info.html</filename>
    <member kind="variable">
      <type>L_Operand *</type>
      <name>mac_operand</name>
      <anchorfile>structip__mac__subst__info.html</anchorfile>
      <anchor>1902b5f01a82fbe46b975cc4009d6498</anchor>
      <arglist></arglist>
    </member>
    <member kind="variable">
      <type>L_Operand *</type>
      <name>reg_operand</name>
      <anchorfile>structip__mac__subst__info.html</anchorfile>
      <anchor>07755ffb33bf584c47276e1f14b2f7e4</anchor>
      <arglist></arglist>
    </member>
    <member kind="variable">
      <type>int</type>
      <name>reg_value</name>
      <anchorfile>structip__mac__subst__info.html</anchorfile>
      <anchor>019d62a2973643e4da7698780ac42cb5</anchor>
      <arglist></arglist>
    </member>
  </compound>
  <compound kind="struct">
    <name>unwind_info</name>
    <filename>structunwind__info.html</filename>
    <member kind="variable">
      <type>UnwindReg</type>
      <name>pfs</name>
      <anchorfile>structunwind__info.html</anchorfile>
      <anchor>bdc04d5076fa53244bcdeba2faf28191</anchor>
      <arglist></arglist>
    </member>
    <member kind="variable">
      <type>UnwindReg</type>
      <name>rp</name>
      <anchorfile>structunwind__info.html</anchorfile>
      <anchor>ee08984a1c0ccf0122574e9704dbd84a</anchor>
      <arglist></arglist>
    </member>
    <member kind="variable">
      <type>UnwindReg</type>
      <name>unat</name>
      <anchorfile>structunwind__info.html</anchorfile>
      <anchor>75cb7f1d6327b0083e98e60261688a85</anchor>
      <arglist></arglist>
    </member>
    <member kind="variable">
      <type>UnwindReg</type>
      <name>rnat</name>
      <anchorfile>structunwind__info.html</anchorfile>
      <anchor>90d3140502394961145ade5559d11a8c</anchor>
      <arglist></arglist>
    </member>
    <member kind="variable">
      <type>UnwindReg</type>
      <name>lc</name>
      <anchorfile>structunwind__info.html</anchorfile>
      <anchor>dbf3d966867310f12d2f3a53866d2f67</anchor>
      <arglist></arglist>
    </member>
    <member kind="variable">
      <type>UnwindReg</type>
      <name>fpsr</name>
      <anchorfile>structunwind__info.html</anchorfile>
      <anchor>1ab9601319072882f11db56d371cd8bc</anchor>
      <arglist></arglist>
    </member>
    <member kind="variable">
      <type>UnwindReg</type>
      <name>pr</name>
      <anchorfile>structunwind__info.html</anchorfile>
      <anchor>4f8e976338ad4e2ded7eace346fed4da</anchor>
      <arglist></arglist>
    </member>
    <member kind="variable">
      <type>L_Oper *</type>
      <name>first_prologue_inst</name>
      <anchorfile>structunwind__info.html</anchorfile>
      <anchor>76f9c766aea87da9dc94e7177f13301d</anchor>
      <arglist></arglist>
    </member>
    <member kind="variable">
      <type>L_Oper *</type>
      <name>last_prologue_inst</name>
      <anchorfile>structunwind__info.html</anchorfile>
      <anchor>dfc7f5a23b725e87025bacb0a940f5dc</anchor>
      <arglist></arglist>
    </member>
    <member kind="variable">
      <type>L_Oper *</type>
      <name>mem_stack</name>
      <anchorfile>structunwind__info.html</anchorfile>
      <anchor>033b414b550e1a8760f5838c4a171e9b</anchor>
      <arglist></arglist>
    </member>
    <member kind="variable">
      <type>L_Oper *</type>
      <name>mem_stack_dealloc</name>
      <anchorfile>structunwind__info.html</anchorfile>
      <anchor>a3543c010faa1d0827feae5900cfefc3</anchor>
      <arglist></arglist>
    </member>
    <member kind="variable">
      <type>int</type>
      <name>mem_stack_size</name>
      <anchorfile>structunwind__info.html</anchorfile>
      <anchor>c76a5595d2501a81ba16a70bda60d18a</anchor>
      <arglist></arglist>
    </member>
    <member kind="variable">
      <type>short</type>
      <name>temp_reg_sp_rel</name>
      <anchorfile>structunwind__info.html</anchorfile>
      <anchor>f05d47d644b81bc52b5386a759e1735a</anchor>
      <arglist></arglist>
    </member>
    <member kind="variable">
      <type>int</type>
      <name>temp_reg_offset</name>
      <anchorfile>structunwind__info.html</anchorfile>
      <anchor>d561f838c67a71a5209f76630eb37070</anchor>
      <arglist></arglist>
    </member>
    <member kind="variable">
      <type>int</type>
      <name>temp_reg_absolute</name>
      <anchorfile>structunwind__info.html</anchorfile>
      <anchor>84bd0142433d6c3cd7520c32e7d4132a</anchor>
      <arglist></arglist>
    </member>
  </compound>
</tagfile>
