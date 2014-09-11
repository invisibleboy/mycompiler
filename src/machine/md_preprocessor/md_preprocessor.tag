<?xml version='1.0' encoding='ISO-8859-1' standalone='yes' ?>
<tagfile>
  <compound kind="file">
    <name>md_preprocessor.c</name>
    <path>/mnt/hgfs/mkt/Desktop/openimpact/src/machine/md_preprocessor/</path>
    <filename>md__preprocessor_8c</filename>
    <includes id="md__preprocessor_8h" name="md_preprocessor.h" local="yes" imported="no">md_preprocessor.h</includes>
    <member kind="function">
      <type>void</type>
      <name>print_usage</name>
      <anchorfile>md__preprocessor_8c.html</anchorfile>
      <anchor>62ac7a1cfa066f61a3317977c936b8f2</anchor>
      <arglist>(char *fmt,...)</arglist>
    </member>
    <member kind="function">
      <type>void</type>
      <name>read_command_line_parameters</name>
      <anchorfile>md__preprocessor_8c.html</anchorfile>
      <anchor>b5ad1e75f3e95a8372855846f911efe1</anchor>
      <arglist>(int argc, char **arg_ptr)</arglist>
    </member>
    <member kind="function">
      <type>void</type>
      <name>L_punt</name>
      <anchorfile>md__preprocessor_8c.html</anchorfile>
      <anchor>c8ccfa451c1bd1740a1b720ba5b4c3a9</anchor>
      <arglist>(char *fmt,...)</arglist>
    </member>
    <member kind="function">
      <type>void</type>
      <name>Perror</name>
      <anchorfile>md__preprocessor_8c.html</anchorfile>
      <anchor>beffca6eda4a1c83ca9e7f1dd401fd99</anchor>
      <arglist>(Pptr *pptr, char *fmt,...)</arglist>
    </member>
    <member kind="function">
      <type>void</type>
      <name>Pputc</name>
      <anchorfile>md__preprocessor_8c.html</anchorfile>
      <anchor>d8d34f9d84875158116eecfd1f6a0c14</anchor>
      <arglist>(Pptr *pptr, int ch)</arglist>
    </member>
    <member kind="function">
      <type>int</type>
      <name>main</name>
      <anchorfile>md__preprocessor_8c.html</anchorfile>
      <anchor>647f21a28344e1d9c643f4115516d7c9</anchor>
      <arglist>(int argc, char **argv, char **envp)</arglist>
    </member>
    <member kind="function">
      <type>Pptr *</type>
      <name>process_body</name>
      <anchorfile>md__preprocessor_8c.html</anchorfile>
      <anchor>3c40c88a57af572f6e543e31a4b55768</anchor>
      <arglist>(Pptr *pptr)</arglist>
    </member>
    <member kind="function">
      <type>Pptr *</type>
      <name>skip_body</name>
      <anchorfile>md__preprocessor_8c.html</anchorfile>
      <anchor>29c698ea791dfee82424f8393cf5d814</anchor>
      <arglist>(Pptr *pptr)</arglist>
    </member>
    <member kind="function">
      <type>Pptr *</type>
      <name>process_def_directive</name>
      <anchorfile>md__preprocessor_8c.html</anchorfile>
      <anchor>fdaaf0c08b62cd43e5837509c73874ce</anchor>
      <arglist>(Pptr *pptr)</arglist>
    </member>
    <member kind="function">
      <type>Pptr *</type>
      <name>process_undef_directive</name>
      <anchorfile>md__preprocessor_8c.html</anchorfile>
      <anchor>c772ab2dc884637e79614531f700b0d3</anchor>
      <arglist>(Pptr *pptr)</arglist>
    </member>
    <member kind="function">
      <type>Value_List *</type>
      <name>get_for_value_list</name>
      <anchorfile>md__preprocessor_8c.html</anchorfile>
      <anchor>358bcc55e8945163d5b09dd2e3165a4e</anchor>
      <arglist>(Pptr *pptr, Pptr *placemark)</arglist>
    </member>
    <member kind="function">
      <type>Pptr *</type>
      <name>process_for_directive</name>
      <anchorfile>md__preprocessor_8c.html</anchorfile>
      <anchor>79a2bf4e345e50554d0015b3fcdb30e3</anchor>
      <arglist>(Pptr *pptr, Pptr *placemark)</arglist>
    </member>
    <member kind="function">
      <type>Pptr *</type>
      <name>process_if_directive</name>
      <anchorfile>md__preprocessor_8c.html</anchorfile>
      <anchor>232036d58bb8ca96bb75ebc2d0cdf197</anchor>
      <arglist>(Pptr *pptr, Pptr *placemark)</arglist>
    </member>
    <member kind="function">
      <type>Pptr *</type>
      <name>process_include_directive</name>
      <anchorfile>md__preprocessor_8c.html</anchorfile>
      <anchor>e93958001f8a6c56febf0557b9f3f7a3</anchor>
      <arglist>(Pptr *pptr, Pptr *placemark)</arglist>
    </member>
    <member kind="function">
      <type>Pptr *</type>
      <name>process_directive</name>
      <anchorfile>md__preprocessor_8c.html</anchorfile>
      <anchor>9fedc647fa9d620c00f1cf9cd84f2da4</anchor>
      <arglist>(Pptr *pptr)</arglist>
    </member>
    <member kind="function">
      <type>char *</type>
      <name>Pget_alnum_string</name>
      <anchorfile>md__preprocessor_8c.html</anchorfile>
      <anchor>c229bcda9d9321b3abe143a15e46b6ac</anchor>
      <arglist>(Pptr *pptr)</arglist>
    </member>
    <member kind="function">
      <type>char *</type>
      <name>Pget_identifier</name>
      <anchorfile>md__preprocessor_8c.html</anchorfile>
      <anchor>10640d24558b93d50dbd5118414cff80</anchor>
      <arglist>(Pptr *pptr)</arglist>
    </member>
    <member kind="function">
      <type>char *</type>
      <name>Pget_stripped_line</name>
      <anchorfile>md__preprocessor_8c.html</anchorfile>
      <anchor>d3ce20c32087bcde00f8e7d94653ddde</anchor>
      <arglist>(Pptr *pptr)</arglist>
    </member>
    <member kind="function">
      <type>char *</type>
      <name>Pget_quoted_string</name>
      <anchorfile>md__preprocessor_8c.html</anchorfile>
      <anchor>71c841602201ec9872d251a9445ba27b</anchor>
      <arglist>(Pptr *pptr, int strip_quotes)</arglist>
    </member>
    <member kind="function">
      <type>char *</type>
      <name>Pget_for_string</name>
      <anchorfile>md__preprocessor_8c.html</anchorfile>
      <anchor>01d6d37758718ad06459891e48ecac32</anchor>
      <arglist>(Pptr *pptr)</arglist>
    </member>
    <member kind="function">
      <type>char *</type>
      <name>Pget_bounded_string</name>
      <anchorfile>md__preprocessor_8c.html</anchorfile>
      <anchor>dc875874dad3e9abb6cb02d654e5e897</anchor>
      <arglist>(Pptr *pptr)</arglist>
    </member>
    <member kind="function">
      <type>void</type>
      <name>Pskip_whitespace</name>
      <anchorfile>md__preprocessor_8c.html</anchorfile>
      <anchor>c1fa9396790747922ef69a79f8c21e8c</anchor>
      <arglist>(Pptr *pptr)</arglist>
    </member>
    <member kind="function">
      <type>void</type>
      <name>Pskip_whitespace_no_nl</name>
      <anchorfile>md__preprocessor_8c.html</anchorfile>
      <anchor>f80ca7eae5033482494d22ef8ecdfbfb</anchor>
      <arglist>(Pptr *pptr)</arglist>
    </member>
    <member kind="function">
      <type>Pptr *</type>
      <name>create_Pptr</name>
      <anchorfile>md__preprocessor_8c.html</anchorfile>
      <anchor>03c35a85124487f604fced1fd4cb1413</anchor>
      <arglist>(Mfile *mfile)</arglist>
    </member>
    <member kind="function">
      <type>Pptr *</type>
      <name>copy_Pptr</name>
      <anchorfile>md__preprocessor_8c.html</anchorfile>
      <anchor>f0da883268c1e42b311b6d8673f6206e</anchor>
      <arglist>(Pptr *orig_pptr)</arglist>
    </member>
    <member kind="function">
      <type>void</type>
      <name>move_Pptr</name>
      <anchorfile>md__preprocessor_8c.html</anchorfile>
      <anchor>657cab1ededb1d24f0f14f022bcd094d</anchor>
      <arglist>(Pptr *old_pptr, Pptr *new_pptr)</arglist>
    </member>
    <member kind="function">
      <type>void</type>
      <name>free_Pptr</name>
      <anchorfile>md__preprocessor_8c.html</anchorfile>
      <anchor>4640aa292e2cee439780e0e2c231b8ed</anchor>
      <arglist>(Pptr *pptr)</arglist>
    </member>
    <member kind="function">
      <type>void</type>
      <name>Pexpand_directive</name>
      <anchorfile>md__preprocessor_8c.html</anchorfile>
      <anchor>f266761f9b2875c8e336313034133ea1</anchor>
      <arglist>(Pptr *source_pptr)</arglist>
    </member>
    <member kind="function">
      <type>int</type>
      <name>Pexpand_text</name>
      <anchorfile>md__preprocessor_8c.html</anchorfile>
      <anchor>96dc9d0c7b4e9f7872132290d1cf3ac7</anchor>
      <arglist>(Pptr *pptr)</arglist>
    </member>
    <member kind="function">
      <type>int</type>
      <name>Pgetc</name>
      <anchorfile>md__preprocessor_8c.html</anchorfile>
      <anchor>b8460f9e0b884163e92dad174040ead0</anchor>
      <arglist>(Pptr *pptr)</arglist>
    </member>
    <member kind="function">
      <type>int</type>
      <name>Ppeekc</name>
      <anchorfile>md__preprocessor_8c.html</anchorfile>
      <anchor>5b05ec5cf990254459f0b332eb08b6cd</anchor>
      <arglist>(Pptr *pptr)</arglist>
    </member>
    <member kind="function">
      <type>int</type>
      <name>Pcalc_C_int_expr</name>
      <anchorfile>md__preprocessor_8c.html</anchorfile>
      <anchor>d0cd379c806bdf45c36a3f8fe9f28201</anchor>
      <arglist>(Pptr *pptr, int current_precedence)</arglist>
    </member>
    <member kind="function">
      <type>int</type>
      <name>Pcalc_C_int_factor</name>
      <anchorfile>md__preprocessor_8c.html</anchorfile>
      <anchor>be976f6287689c26f1804b05a5604e1c</anchor>
      <arglist>(Pptr *pptr)</arglist>
    </member>
    <member kind="function">
      <type>double</type>
      <name>Pcalc_C_float_expr</name>
      <anchorfile>md__preprocessor_8c.html</anchorfile>
      <anchor>9f2325391833230381496bb431bd6787</anchor>
      <arglist>(Pptr *pptr, int current_precedence)</arglist>
    </member>
    <member kind="function">
      <type>double</type>
      <name>Pcalc_C_float_factor</name>
      <anchorfile>md__preprocessor_8c.html</anchorfile>
      <anchor>d5d57d0445efaf105fc993c973500e88</anchor>
      <arglist>(Pptr *pptr)</arglist>
    </member>
    <member kind="function">
      <type>Pdef *</type>
      <name>create_Pdef</name>
      <anchorfile>md__preprocessor_8c.html</anchorfile>
      <anchor>cf9d96a6259b42e99198e03a423214d7</anchor>
      <arglist>(char *name, char *val, int allow_implicit_replacement, int level)</arglist>
    </member>
    <member kind="function">
      <type>void</type>
      <name>free_Pdef</name>
      <anchorfile>md__preprocessor_8c.html</anchorfile>
      <anchor>89ffda2e246f0a64495be522f05cbadd</anchor>
      <arglist>(void *def_v)</arglist>
    </member>
    <member kind="function">
      <type>void</type>
      <name>add_Pdef</name>
      <anchorfile>md__preprocessor_8c.html</anchorfile>
      <anchor>a664d54fd7aa40b97a202097ac127738</anchor>
      <arglist>(char *name, char *val, int allow_implicit_replacement, int level)</arglist>
    </member>
    <member kind="function">
      <type>void</type>
      <name>delete_Pdef</name>
      <anchorfile>md__preprocessor_8c.html</anchorfile>
      <anchor>3cde2100eec552f5ae6db0940aef4d34</anchor>
      <arglist>(char *name)</arglist>
    </member>
    <member kind="function">
      <type>char *</type>
      <name>Plookup</name>
      <anchorfile>md__preprocessor_8c.html</anchorfile>
      <anchor>8ac3ceff2e23da42c21150cfc5d3ff21</anchor>
      <arglist>(char *name, int implicit_replacement)</arglist>
    </member>
    <member kind="variable">
      <type>FILE *</type>
      <name>out</name>
      <anchorfile>md__preprocessor_8c.html</anchorfile>
      <anchor>1277960b5f2b37137fe9b0b5a1ea0beb</anchor>
      <arglist></arglist>
    </member>
    <member kind="variable">
      <type>Mfile *</type>
      <name>current_file</name>
      <anchorfile>md__preprocessor_8c.html</anchorfile>
      <anchor>e691226eaa6fb3edc17ff2d541e1ce55</anchor>
      <arglist></arglist>
    </member>
    <member kind="variable">
      <type>Psymbol_Table *</type>
      <name>Pdef_table</name>
      <anchorfile>md__preprocessor_8c.html</anchorfile>
      <anchor>97741975f920c10105047ce529cf052a</anchor>
      <arglist></arglist>
    </member>
    <member kind="variable">
      <type>Psymbol_Table *</type>
      <name>Mfile_table</name>
      <anchorfile>md__preprocessor_8c.html</anchorfile>
      <anchor>134cbad6e978db177d4d05dff9fd74d2</anchor>
      <arglist></arglist>
    </member>
    <member kind="variable">
      <type>int</type>
      <name>allow_text_replacement</name>
      <anchorfile>md__preprocessor_8c.html</anchorfile>
      <anchor>ec53c8fb29375671fc5e7ab14c42371e</anchor>
      <arglist></arglist>
    </member>
    <member kind="variable">
      <type>int</type>
      <name>disable_implicit_text_replacement</name>
      <anchorfile>md__preprocessor_8c.html</anchorfile>
      <anchor>05d4fa71ca0b0662c39ceab758ce15d4</anchor>
      <arglist></arglist>
    </member>
    <member kind="variable">
      <type>Mbuf *</type>
      <name>temp_mbuf</name>
      <anchorfile>md__preprocessor_8c.html</anchorfile>
      <anchor>bcfb4b06283a7a8a72bf5c01c75b4e35</anchor>
      <arglist></arglist>
    </member>
    <member kind="variable">
      <type>Mbuf *</type>
      <name>pptr_mbuf</name>
      <anchorfile>md__preprocessor_8c.html</anchorfile>
      <anchor>a66fcb8ee43325dd5745131d0eca2b57</anchor>
      <arglist></arglist>
    </member>
    <member kind="variable">
      <type>Pptr *</type>
      <name>temp_placemark</name>
      <anchorfile>md__preprocessor_8c.html</anchorfile>
      <anchor>bc1de95fe5c53717a01e4d0edca5f40a</anchor>
      <arglist></arglist>
    </member>
    <member kind="variable">
      <type>Mptr *</type>
      <name>expand_placemark</name>
      <anchorfile>md__preprocessor_8c.html</anchorfile>
      <anchor>d1c53c5ecf7324077480c8560900b248</anchor>
      <arglist></arglist>
    </member>
    <member kind="variable">
      <type>L_Alloc_Pool *</type>
      <name>Pptr_pool</name>
      <anchorfile>md__preprocessor_8c.html</anchorfile>
      <anchor>c132912a82b79461cb2c962ae4c525df</anchor>
      <arglist></arglist>
    </member>
    <member kind="variable">
      <type>L_Alloc_Pool *</type>
      <name>Pdef_pool</name>
      <anchorfile>md__preprocessor_8c.html</anchorfile>
      <anchor>8f8faa045221340e2f32e6bb958db91b</anchor>
      <arglist></arglist>
    </member>
    <member kind="variable">
      <type>L_Alloc_Pool *</type>
      <name>String_Node_pool</name>
      <anchorfile>md__preprocessor_8c.html</anchorfile>
      <anchor>d47ec2b1abf9cb9d0128bc3fc27d94c5</anchor>
      <arglist></arglist>
    </member>
    <member kind="variable">
      <type>L_Alloc_Pool *</type>
      <name>Value_List_pool</name>
      <anchorfile>md__preprocessor_8c.html</anchorfile>
      <anchor>cb94a38a26e21dc0042efb3fe66cc3ac</anchor>
      <arglist></arglist>
    </member>
    <member kind="variable">
      <type>char *</type>
      <name>program_name</name>
      <anchorfile>md__preprocessor_8c.html</anchorfile>
      <anchor>289c5900d90626d909f0a85d5a0ed61d</anchor>
      <arglist></arglist>
    </member>
    <member kind="variable">
      <type>char *</type>
      <name>input_file_name</name>
      <anchorfile>md__preprocessor_8c.html</anchorfile>
      <anchor>a91ff4ee3ee60baee9c097d4da3e3d18</anchor>
      <arglist></arglist>
    </member>
    <member kind="variable">
      <type>char *</type>
      <name>output_file_name</name>
      <anchorfile>md__preprocessor_8c.html</anchorfile>
      <anchor>cdcd0d11bb729145f180d52a0755cddf</anchor>
      <arglist></arglist>
    </member>
    <member kind="variable">
      <type>int</type>
      <name>using_stdin</name>
      <anchorfile>md__preprocessor_8c.html</anchorfile>
      <anchor>bc340a0cfa46e3f25a4d1adab28331c0</anchor>
      <arglist></arglist>
    </member>
    <member kind="variable">
      <type>int</type>
      <name>print_line_directives</name>
      <anchorfile>md__preprocessor_8c.html</anchorfile>
      <anchor>51f80c177dc5130d3f3804774080d22c</anchor>
      <arglist></arglist>
    </member>
    <member kind="variable">
      <type>int</type>
      <name>print_alloc_usage</name>
      <anchorfile>md__preprocessor_8c.html</anchorfile>
      <anchor>bb8d45b771d5af70f76be0bc93f5baae</anchor>
      <arglist></arglist>
    </member>
  </compound>
  <compound kind="file">
    <name>md_preprocessor.h</name>
    <path>/mnt/hgfs/mkt/Desktop/openimpact/src/machine/md_preprocessor/</path>
    <filename>md__preprocessor_8h</filename>
    <includes id="psymbol_8h" name="psymbol.h" local="yes" imported="no">psymbol.h</includes>
    <class kind="struct">Pptr</class>
    <class kind="struct">Pdef</class>
    <class kind="struct">String_Node</class>
    <class kind="struct">Value_List</class>
    <member kind="function">
      <type>Pptr *</type>
      <name>create_Pptr</name>
      <anchorfile>md__preprocessor_8h.html</anchorfile>
      <anchor>03c35a85124487f604fced1fd4cb1413</anchor>
      <arglist>(Mfile *mfile)</arglist>
    </member>
    <member kind="function">
      <type>Pptr *</type>
      <name>copy_Pptr</name>
      <anchorfile>md__preprocessor_8h.html</anchorfile>
      <anchor>f0da883268c1e42b311b6d8673f6206e</anchor>
      <arglist>(Pptr *orig_pptr)</arglist>
    </member>
    <member kind="function">
      <type>void</type>
      <name>move_Pptr</name>
      <anchorfile>md__preprocessor_8h.html</anchorfile>
      <anchor>657cab1ededb1d24f0f14f022bcd094d</anchor>
      <arglist>(Pptr *old_pptr, Pptr *new_pptr)</arglist>
    </member>
    <member kind="function">
      <type>void</type>
      <name>free_Pptr</name>
      <anchorfile>md__preprocessor_8h.html</anchorfile>
      <anchor>4640aa292e2cee439780e0e2c231b8ed</anchor>
      <arglist>(Pptr *pptr)</arglist>
    </member>
    <member kind="function">
      <type>int</type>
      <name>Pexpand_text</name>
      <anchorfile>md__preprocessor_8h.html</anchorfile>
      <anchor>96dc9d0c7b4e9f7872132290d1cf3ac7</anchor>
      <arglist>(Pptr *pptr)</arglist>
    </member>
    <member kind="function">
      <type>int</type>
      <name>Pgetc</name>
      <anchorfile>md__preprocessor_8h.html</anchorfile>
      <anchor>b8460f9e0b884163e92dad174040ead0</anchor>
      <arglist>(Pptr *pptr)</arglist>
    </member>
    <member kind="function">
      <type>int</type>
      <name>Ppeekc</name>
      <anchorfile>md__preprocessor_8h.html</anchorfile>
      <anchor>5b05ec5cf990254459f0b332eb08b6cd</anchor>
      <arglist>(Pptr *pptr)</arglist>
    </member>
    <member kind="function">
      <type>void</type>
      <name>Pungetc</name>
      <anchorfile>md__preprocessor_8h.html</anchorfile>
      <anchor>f2ff8c54f9e8cdb9a10ab1580d80ea13</anchor>
      <arglist>(Pptr *pptr, int ch)</arglist>
    </member>
    <member kind="function">
      <type>void</type>
      <name>Pungets</name>
      <anchorfile>md__preprocessor_8h.html</anchorfile>
      <anchor>36b949666633221b85a46da0c6030e9e</anchor>
      <arglist>(Pptr *pptr, char *string)</arglist>
    </member>
    <member kind="function">
      <type>void</type>
      <name>Pputc</name>
      <anchorfile>md__preprocessor_8h.html</anchorfile>
      <anchor>d8d34f9d84875158116eecfd1f6a0c14</anchor>
      <arglist>(Pptr *pptr, int ch)</arglist>
    </member>
    <member kind="function">
      <type>char *</type>
      <name>Pget_alnum_string</name>
      <anchorfile>md__preprocessor_8h.html</anchorfile>
      <anchor>c229bcda9d9321b3abe143a15e46b6ac</anchor>
      <arglist>(Pptr *pptr)</arglist>
    </member>
    <member kind="function">
      <type>char *</type>
      <name>Pget_identifier</name>
      <anchorfile>md__preprocessor_8h.html</anchorfile>
      <anchor>10640d24558b93d50dbd5118414cff80</anchor>
      <arglist>(Pptr *pptr)</arglist>
    </member>
    <member kind="function">
      <type>char *</type>
      <name>Pget_stripped_line</name>
      <anchorfile>md__preprocessor_8h.html</anchorfile>
      <anchor>d3ce20c32087bcde00f8e7d94653ddde</anchor>
      <arglist>(Pptr *pptr)</arglist>
    </member>
    <member kind="function">
      <type>char *</type>
      <name>Pget_bounded_string</name>
      <anchorfile>md__preprocessor_8h.html</anchorfile>
      <anchor>dc875874dad3e9abb6cb02d654e5e897</anchor>
      <arglist>(Pptr *pptr)</arglist>
    </member>
    <member kind="function">
      <type>char *</type>
      <name>Pget_quoted_string</name>
      <anchorfile>md__preprocessor_8h.html</anchorfile>
      <anchor>71c841602201ec9872d251a9445ba27b</anchor>
      <arglist>(Pptr *pptr, int strip_quotes)</arglist>
    </member>
    <member kind="function">
      <type>char *</type>
      <name>Pget_for_string</name>
      <anchorfile>md__preprocessor_8h.html</anchorfile>
      <anchor>01d6d37758718ad06459891e48ecac32</anchor>
      <arglist>(Pptr *pptr)</arglist>
    </member>
    <member kind="function">
      <type>void</type>
      <name>Pskip_whitespace</name>
      <anchorfile>md__preprocessor_8h.html</anchorfile>
      <anchor>c1fa9396790747922ef69a79f8c21e8c</anchor>
      <arglist>(Pptr *pptr)</arglist>
    </member>
    <member kind="function">
      <type>void</type>
      <name>Pskip_whitespace_no_nl</name>
      <anchorfile>md__preprocessor_8h.html</anchorfile>
      <anchor>f80ca7eae5033482494d22ef8ecdfbfb</anchor>
      <arglist>(Pptr *pptr)</arglist>
    </member>
    <member kind="function">
      <type>void</type>
      <name>Perror</name>
      <anchorfile>md__preprocessor_8h.html</anchorfile>
      <anchor>beffca6eda4a1c83ca9e7f1dd401fd99</anchor>
      <arglist>(Pptr *pptr, char *fmt,...)</arglist>
    </member>
    <member kind="function">
      <type>void</type>
      <name>L_punt</name>
      <anchorfile>md__preprocessor_8h.html</anchorfile>
      <anchor>c8ccfa451c1bd1740a1b720ba5b4c3a9</anchor>
      <arglist>(char *fmt,...)</arglist>
    </member>
    <member kind="function">
      <type>int</type>
      <name>Pcalc_C_int_expr</name>
      <anchorfile>md__preprocessor_8h.html</anchorfile>
      <anchor>d0cd379c806bdf45c36a3f8fe9f28201</anchor>
      <arglist>(Pptr *pptr, int current_precedence)</arglist>
    </member>
    <member kind="function">
      <type>int</type>
      <name>Pcalc_C_int_factor</name>
      <anchorfile>md__preprocessor_8h.html</anchorfile>
      <anchor>be976f6287689c26f1804b05a5604e1c</anchor>
      <arglist>(Pptr *pptr)</arglist>
    </member>
    <member kind="function">
      <type>double</type>
      <name>Pcalc_C_float_expr</name>
      <anchorfile>md__preprocessor_8h.html</anchorfile>
      <anchor>9f2325391833230381496bb431bd6787</anchor>
      <arglist>(Pptr *pptr, int current_precedence)</arglist>
    </member>
    <member kind="function">
      <type>double</type>
      <name>Pcalc_C_float_factor</name>
      <anchorfile>md__preprocessor_8h.html</anchorfile>
      <anchor>d5d57d0445efaf105fc993c973500e88</anchor>
      <arglist>(Pptr *pptr)</arglist>
    </member>
    <member kind="function">
      <type>char *</type>
      <name>Plookup</name>
      <anchorfile>md__preprocessor_8h.html</anchorfile>
      <anchor>8ac3ceff2e23da42c21150cfc5d3ff21</anchor>
      <arglist>(char *name, int implicit_replacement)</arglist>
    </member>
    <member kind="function">
      <type>Pdef *</type>
      <name>create_Pdef</name>
      <anchorfile>md__preprocessor_8h.html</anchorfile>
      <anchor>cf9d96a6259b42e99198e03a423214d7</anchor>
      <arglist>(char *name, char *val, int allow_implicit_replacement, int level)</arglist>
    </member>
    <member kind="function">
      <type>void</type>
      <name>free_Pdef</name>
      <anchorfile>md__preprocessor_8h.html</anchorfile>
      <anchor>89ffda2e246f0a64495be522f05cbadd</anchor>
      <arglist>(void *def_v)</arglist>
    </member>
    <member kind="function">
      <type>void</type>
      <name>add_Pdef</name>
      <anchorfile>md__preprocessor_8h.html</anchorfile>
      <anchor>a664d54fd7aa40b97a202097ac127738</anchor>
      <arglist>(char *name, char *val, int allow_implicit_replacement, int level)</arglist>
    </member>
    <member kind="function">
      <type>void</type>
      <name>delete_Pdef</name>
      <anchorfile>md__preprocessor_8h.html</anchorfile>
      <anchor>3cde2100eec552f5ae6db0940aef4d34</anchor>
      <arglist>(char *name)</arglist>
    </member>
    <member kind="function">
      <type>Pptr *</type>
      <name>process_body</name>
      <anchorfile>md__preprocessor_8h.html</anchorfile>
      <anchor>3c40c88a57af572f6e543e31a4b55768</anchor>
      <arglist>(Pptr *pptr)</arglist>
    </member>
    <member kind="function">
      <type>Pptr *</type>
      <name>process_directive</name>
      <anchorfile>md__preprocessor_8h.html</anchorfile>
      <anchor>9fedc647fa9d620c00f1cf9cd84f2da4</anchor>
      <arglist>(Pptr *pptr)</arglist>
    </member>
    <member kind="function">
      <type>Pptr *</type>
      <name>process_def_directive</name>
      <anchorfile>md__preprocessor_8h.html</anchorfile>
      <anchor>fdaaf0c08b62cd43e5837509c73874ce</anchor>
      <arglist>(Pptr *pptr)</arglist>
    </member>
    <member kind="function">
      <type>Pptr *</type>
      <name>process_for_directive</name>
      <anchorfile>md__preprocessor_8h.html</anchorfile>
      <anchor>79a2bf4e345e50554d0015b3fcdb30e3</anchor>
      <arglist>(Pptr *pptr, Pptr *placemark)</arglist>
    </member>
    <member kind="function">
      <type>int</type>
      <name>Piscomment</name>
      <anchorfile>md__preprocessor_8h.html</anchorfile>
      <anchor>d9dade1489beeca1d82f0aeda068941a</anchor>
      <arglist>(char *token)</arglist>
    </member>
    <member kind="function">
      <type>void</type>
      <name>preprocess</name>
      <anchorfile>md__preprocessor_8h.html</anchorfile>
      <anchor>4ade4a7448fe7143090e668582be2e54</anchor>
      <arglist>(Pptr *ptr)</arglist>
    </member>
  </compound>
  <compound kind="file">
    <name>psymbol.c</name>
    <path>/mnt/hgfs/mkt/Desktop/openimpact/src/machine/md_preprocessor/</path>
    <filename>psymbol_8c</filename>
    <includes id="psymbol_8h" name="psymbol.h" local="yes" imported="no">psymbol.h</includes>
    <member kind="function">
      <type>Psymbol_Table *</type>
      <name>create_Psymbol_Table</name>
      <anchorfile>psymbol_8c.html</anchorfile>
      <anchor>8f6a98d5bf33556e5bbe7f8e940a0f81</anchor>
      <arglist>(char *name)</arglist>
    </member>
    <member kind="function">
      <type>void</type>
      <name>free_Psymbol_Table</name>
      <anchorfile>psymbol_8c.html</anchorfile>
      <anchor>6d76093f228e49068b42c187a149e669</anchor>
      <arglist>(Psymbol_Table *table, void(*free_routine)(void *))</arglist>
    </member>
    <member kind="function">
      <type>int</type>
      <name>hash_Psymbol_name</name>
      <anchorfile>psymbol_8c.html</anchorfile>
      <anchor>3eb26afb9e01015e862209de3eb019ff</anchor>
      <arglist>(char *name)</arglist>
    </member>
    <member kind="function">
      <type>void</type>
      <name>add_Psymbol</name>
      <anchorfile>psymbol_8c.html</anchorfile>
      <anchor>43c4935d67bb3583c3e7f4a50bc07bff</anchor>
      <arglist>(Psymbol_Table *table, char *name, void *data)</arglist>
    </member>
    <member kind="function">
      <type>Psymbol *</type>
      <name>find_Psymbol</name>
      <anchorfile>psymbol_8c.html</anchorfile>
      <anchor>dcef8f7f2490ff89644f958db15b07cd</anchor>
      <arglist>(Psymbol_Table *table, char *name)</arglist>
    </member>
    <member kind="function">
      <type>void</type>
      <name>delete_Psymbol</name>
      <anchorfile>psymbol_8c.html</anchorfile>
      <anchor>21ac4b8e5e7b5a95b9d3cac63c5a6562</anchor>
      <arglist>(Psymbol *symbol, void(*free_routine)(void *))</arglist>
    </member>
    <member kind="function">
      <type>void</type>
      <name>print_Psymbol_hash_table</name>
      <anchorfile>psymbol_8c.html</anchorfile>
      <anchor>73be357f45bd4e9041cd442895b011fc</anchor>
      <arglist>(FILE *out, Psymbol_Table *table)</arglist>
    </member>
    <member kind="variable">
      <type>L_Alloc_Pool *</type>
      <name>Psymbol_Table_pool</name>
      <anchorfile>psymbol_8c.html</anchorfile>
      <anchor>2d729e93a340ce1b90a4aba0da407dc3</anchor>
      <arglist></arglist>
    </member>
    <member kind="variable">
      <type>L_Alloc_Pool *</type>
      <name>Psymbol_pool</name>
      <anchorfile>psymbol_8c.html</anchorfile>
      <anchor>5083a45146f9908a31dac227cbe18aac</anchor>
      <arglist></arglist>
    </member>
  </compound>
  <compound kind="file">
    <name>psymbol.h</name>
    <path>/mnt/hgfs/mkt/Desktop/openimpact/src/machine/md_preprocessor/</path>
    <filename>psymbol_8h</filename>
    <class kind="struct">Psymbol</class>
    <class kind="struct">Psymbol_Table</class>
    <member kind="define">
      <type>#define</type>
      <name>PSYMBOL_HASH_SIZE</name>
      <anchorfile>psymbol_8h.html</anchorfile>
      <anchor>1ed9f6da28a1feac75a73da72e0937b9</anchor>
      <arglist></arglist>
    </member>
    <member kind="function">
      <type>Psymbol_Table *</type>
      <name>create_Psymbol_Table</name>
      <anchorfile>psymbol_8h.html</anchorfile>
      <anchor>8f6a98d5bf33556e5bbe7f8e940a0f81</anchor>
      <arglist>(char *name)</arglist>
    </member>
    <member kind="function">
      <type>void</type>
      <name>free_Psymbol_Table</name>
      <anchorfile>psymbol_8h.html</anchorfile>
      <anchor>6d76093f228e49068b42c187a149e669</anchor>
      <arglist>(Psymbol_Table *table, void(*free_routine)(void *))</arglist>
    </member>
    <member kind="function">
      <type>int</type>
      <name>hash_Psymbol_name</name>
      <anchorfile>psymbol_8h.html</anchorfile>
      <anchor>3eb26afb9e01015e862209de3eb019ff</anchor>
      <arglist>(char *name)</arglist>
    </member>
    <member kind="function">
      <type>void</type>
      <name>add_Psymbol</name>
      <anchorfile>psymbol_8h.html</anchorfile>
      <anchor>43c4935d67bb3583c3e7f4a50bc07bff</anchor>
      <arglist>(Psymbol_Table *table, char *name, void *data)</arglist>
    </member>
    <member kind="function">
      <type>Psymbol *</type>
      <name>find_Psymbol</name>
      <anchorfile>psymbol_8h.html</anchorfile>
      <anchor>dcef8f7f2490ff89644f958db15b07cd</anchor>
      <arglist>(Psymbol_Table *table, char *name)</arglist>
    </member>
    <member kind="function">
      <type>void</type>
      <name>delete_Psymbol</name>
      <anchorfile>psymbol_8h.html</anchorfile>
      <anchor>21ac4b8e5e7b5a95b9d3cac63c5a6562</anchor>
      <arglist>(Psymbol *symbol, void(*free_routine)(void *))</arglist>
    </member>
    <member kind="function">
      <type>void</type>
      <name>print_Psymbol_hash_table</name>
      <anchorfile>psymbol_8h.html</anchorfile>
      <anchor>73be357f45bd4e9041cd442895b011fc</anchor>
      <arglist>(FILE *out, Psymbol_Table *table)</arglist>
    </member>
    <member kind="variable">
      <type>L_Alloc_Pool *</type>
      <name>Psymbol_Table_pool</name>
      <anchorfile>psymbol_8h.html</anchorfile>
      <anchor>2d729e93a340ce1b90a4aba0da407dc3</anchor>
      <arglist></arglist>
    </member>
    <member kind="variable">
      <type>L_Alloc_Pool *</type>
      <name>Psymbol_pool</name>
      <anchorfile>psymbol_8h.html</anchorfile>
      <anchor>5083a45146f9908a31dac227cbe18aac</anchor>
      <arglist></arglist>
    </member>
  </compound>
  <compound kind="struct">
    <name>Pdef</name>
    <filename>structPdef.html</filename>
    <member kind="variable">
      <type>char *</type>
      <name>name</name>
      <anchorfile>structPdef.html</anchorfile>
      <anchor>ab44405ba4f8ff1f5b950c8706a339f9</anchor>
      <arglist></arglist>
    </member>
    <member kind="variable">
      <type>char *</type>
      <name>val</name>
      <anchorfile>structPdef.html</anchorfile>
      <anchor>046b8aa0a11e051866e66c93892c3b4a</anchor>
      <arglist></arglist>
    </member>
    <member kind="variable">
      <type>int</type>
      <name>allow_implicit_replacement</name>
      <anchorfile>structPdef.html</anchorfile>
      <anchor>e7e2956c4ab9f4348c3ff02da425c9d7</anchor>
      <arglist></arglist>
    </member>
    <member kind="variable">
      <type>int</type>
      <name>level</name>
      <anchorfile>structPdef.html</anchorfile>
      <anchor>585c031c810048a941a33e1873be3819</anchor>
      <arglist></arglist>
    </member>
  </compound>
  <compound kind="struct">
    <name>Pptr</name>
    <filename>structPptr.html</filename>
    <member kind="variable">
      <type>Mptr *</type>
      <name>mptr</name>
      <anchorfile>structPptr.html</anchorfile>
      <anchor>908df598f1fde8927c143cc25a42c8b7</anchor>
      <arglist></arglist>
    </member>
    <member kind="variable">
      <type>Mbuf *</type>
      <name>expanded</name>
      <anchorfile>structPptr.html</anchorfile>
      <anchor>bddcba4f437d414b2224b0320f1572d0</anchor>
      <arglist></arglist>
    </member>
    <member kind="variable">
      <type>int</type>
      <name>expanded_pos</name>
      <anchorfile>structPptr.html</anchorfile>
      <anchor>a9b9ed69324a3f9180a5b20a156f0f0e</anchor>
      <arglist></arglist>
    </member>
    <member kind="variable">
      <type>int</type>
      <name>quoted</name>
      <anchorfile>structPptr.html</anchorfile>
      <anchor>61a35abe3929d4bc1c5b8bc63c9c80bc</anchor>
      <arglist></arglist>
    </member>
    <member kind="variable">
      <type>int</type>
      <name>scanned</name>
      <anchorfile>structPptr.html</anchorfile>
      <anchor>1fb5d4feba9b800f7ff6d2a4acf22f8c</anchor>
      <arglist></arglist>
    </member>
  </compound>
  <compound kind="struct">
    <name>Psymbol</name>
    <filename>structPsymbol.html</filename>
    <member kind="variable">
      <type>char *</type>
      <name>name</name>
      <anchorfile>structPsymbol.html</anchorfile>
      <anchor>9e099101015e8a764a9759c071f4b0f3</anchor>
      <arglist></arglist>
    </member>
    <member kind="variable">
      <type>void *</type>
      <name>data</name>
      <anchorfile>structPsymbol.html</anchorfile>
      <anchor>473e052efc6e8d1e777844a035173647</anchor>
      <arglist></arglist>
    </member>
    <member kind="variable">
      <type>Psymbol_Table *</type>
      <name>table</name>
      <anchorfile>structPsymbol.html</anchorfile>
      <anchor>90ee72b6dc96db2d1c45ee084b1e1aa1</anchor>
      <arglist></arglist>
    </member>
    <member kind="variable">
      <type>Psymbol *</type>
      <name>next_hash</name>
      <anchorfile>structPsymbol.html</anchorfile>
      <anchor>4180fa3ddc9d8955e2d494e58a05cb23</anchor>
      <arglist></arglist>
    </member>
    <member kind="variable">
      <type>Psymbol *</type>
      <name>prev_hash</name>
      <anchorfile>structPsymbol.html</anchorfile>
      <anchor>2a1a444508e588886a97af0bad70b54e</anchor>
      <arglist></arglist>
    </member>
    <member kind="variable">
      <type>Psymbol *</type>
      <name>next_symbol</name>
      <anchorfile>structPsymbol.html</anchorfile>
      <anchor>57064d64de3f2c401ebd1f9c1fd35b90</anchor>
      <arglist></arglist>
    </member>
    <member kind="variable">
      <type>Psymbol *</type>
      <name>prev_symbol</name>
      <anchorfile>structPsymbol.html</anchorfile>
      <anchor>67605df0d0a21b952b56b518f50f8cf5</anchor>
      <arglist></arglist>
    </member>
  </compound>
  <compound kind="struct">
    <name>Psymbol_Table</name>
    <filename>structPsymbol__Table.html</filename>
    <member kind="variable">
      <type>char *</type>
      <name>name</name>
      <anchorfile>structPsymbol__Table.html</anchorfile>
      <anchor>57d15d110eac8af9030b032bbc6d90e9</anchor>
      <arglist></arglist>
    </member>
    <member kind="variable">
      <type>Psymbol *</type>
      <name>hash</name>
      <anchorfile>structPsymbol__Table.html</anchorfile>
      <anchor>fccbab9ae9c434eb2f241fe0b5315352</anchor>
      <arglist>[PSYMBOL_HASH_SIZE]</arglist>
    </member>
    <member kind="variable">
      <type>Psymbol *</type>
      <name>head</name>
      <anchorfile>structPsymbol__Table.html</anchorfile>
      <anchor>9444054db2884af67c064f1eefbe6bb4</anchor>
      <arglist></arglist>
    </member>
    <member kind="variable">
      <type>Psymbol *</type>
      <name>tail</name>
      <anchorfile>structPsymbol__Table.html</anchorfile>
      <anchor>9e9cc5825e600e614df0fa3626de4d6d</anchor>
      <arglist></arglist>
    </member>
    <member kind="variable">
      <type>int</type>
      <name>count</name>
      <anchorfile>structPsymbol__Table.html</anchorfile>
      <anchor>59a995d4478207a9fc0c9c28c98d042b</anchor>
      <arglist></arglist>
    </member>
  </compound>
  <compound kind="struct">
    <name>String_Node</name>
    <filename>structString__Node.html</filename>
    <member kind="variable">
      <type>char *</type>
      <name>string</name>
      <anchorfile>structString__Node.html</anchorfile>
      <anchor>a5482b05fc47b8918b7d686dab3e7fc2</anchor>
      <arglist></arglist>
    </member>
    <member kind="variable">
      <type>String_Node *</type>
      <name>next</name>
      <anchorfile>structString__Node.html</anchorfile>
      <anchor>85f2cf608f9c71fa0c5008af13c76cdf</anchor>
      <arglist></arglist>
    </member>
  </compound>
  <compound kind="struct">
    <name>Value_List</name>
    <filename>structValue__List.html</filename>
    <member kind="variable">
      <type>char *</type>
      <name>name</name>
      <anchorfile>structValue__List.html</anchorfile>
      <anchor>d6bca787563ebce2afdce6150a9ac8c5</anchor>
      <arglist></arglist>
    </member>
    <member kind="variable">
      <type>int</type>
      <name>allow_implicit_replacement</name>
      <anchorfile>structValue__List.html</anchorfile>
      <anchor>b33c43bf208d4f59aae44ddaf39b4ec9</anchor>
      <arglist></arglist>
    </member>
    <member kind="variable">
      <type>String_Node *</type>
      <name>first_value</name>
      <anchorfile>structValue__List.html</anchorfile>
      <anchor>16647cbcdf0842a60dbc418933b02f39</anchor>
      <arglist></arglist>
    </member>
    <member kind="variable">
      <type>String_Node *</type>
      <name>last_value</name>
      <anchorfile>structValue__List.html</anchorfile>
      <anchor>23265b17423c6aa570aa99ae8998b0b7</anchor>
      <arglist></arglist>
    </member>
    <member kind="variable">
      <type>int</type>
      <name>value_count</name>
      <anchorfile>structValue__List.html</anchorfile>
      <anchor>ea4537ab4084f0699a28b7fd6de59de1</anchor>
      <arglist></arglist>
    </member>
    <member kind="variable">
      <type>Value_List *</type>
      <name>next_list</name>
      <anchorfile>structValue__List.html</anchorfile>
      <anchor>efb807cb098572e1b4664b5902519723</anchor>
      <arglist></arglist>
    </member>
  </compound>
</tagfile>
