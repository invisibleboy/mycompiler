<?xml version='1.0' encoding='ISO-8859-1' standalone='yes' ?>
<tagfile>
  <compound kind="file">
    <name>md_compiler.c</name>
    <path>/mnt/hgfs/mkt/Desktop/openimpact/src/machine/md_compiler/</path>
    <filename>md__compiler_8c</filename>
    <includes id="md__compiler_8h" name="md_compiler.h" local="yes" imported="no">md_compiler.h</includes>
    <member kind="function">
      <type>void</type>
      <name>print_usage</name>
      <anchorfile>md__compiler_8c.html</anchorfile>
      <anchor>62ac7a1cfa066f61a3317977c936b8f2</anchor>
      <arglist>(char *fmt,...)</arglist>
    </member>
    <member kind="function">
      <type>void</type>
      <name>read_command_line_parameters</name>
      <anchorfile>md__compiler_8c.html</anchorfile>
      <anchor>b5ad1e75f3e95a8372855846f911efe1</anchor>
      <arglist>(int argc, char **arg_ptr)</arglist>
    </member>
    <member kind="function">
      <type>int</type>
      <name>main</name>
      <anchorfile>md__compiler_8c.html</anchorfile>
      <anchor>3c04138a5bfe5d72780bb7e82a18e627</anchor>
      <arglist>(int argc, char **argv)</arglist>
    </member>
    <member kind="function">
      <type>Fptr *</type>
      <name>create_Fptr</name>
      <anchorfile>md__compiler_8c.html</anchorfile>
      <anchor>cd6abca353981f824cbc6b4b39b96326</anchor>
      <arglist>(Mfile *mfile)</arglist>
    </member>
    <member kind="function">
      <type>void</type>
      <name>free_Fptr</name>
      <anchorfile>md__compiler_8c.html</anchorfile>
      <anchor>0bb0eba7f807070cda672228ea3008d7</anchor>
      <arglist>(Fptr *fptr)</arglist>
    </member>
    <member kind="function">
      <type>void</type>
      <name>Ferror</name>
      <anchorfile>md__compiler_8c.html</anchorfile>
      <anchor>761c4386743ac261985b813ab1a8040c</anchor>
      <arglist>(Fptr *fptr, char *fmt,...)</arglist>
    </member>
    <member kind="function">
      <type>void</type>
      <name>vFerror</name>
      <anchorfile>md__compiler_8c.html</anchorfile>
      <anchor>d52c3f1409f7b9a5f521e34fa8c866a3</anchor>
      <arglist>(Fptr *fptr, char *fmt, va_list args)</arglist>
    </member>
    <member kind="function">
      <type>int</type>
      <name>Fpeekc</name>
      <anchorfile>md__compiler_8c.html</anchorfile>
      <anchor>c06c4367d84d0b622fea24b7c6702f47</anchor>
      <arglist>(Fptr *fptr)</arglist>
    </member>
    <member kind="function">
      <type>int</type>
      <name>Fgetc</name>
      <anchorfile>md__compiler_8c.html</anchorfile>
      <anchor>50725b05de9292d7eb2eaa36a6d6fdcb</anchor>
      <arglist>(Fptr *fptr)</arglist>
    </member>
    <member kind="function">
      <type>void</type>
      <name>Fget_line_directive</name>
      <anchorfile>md__compiler_8c.html</anchorfile>
      <anchor>95ddff2b06b579672c573d2ec46808de</anchor>
      <arglist>(Fptr *fptr)</arglist>
    </member>
    <member kind="function">
      <type>void</type>
      <name>Fadvance_to_next_token</name>
      <anchorfile>md__compiler_8c.html</anchorfile>
      <anchor>de728546c62e9c27434f5e16b9581308</anchor>
      <arglist>(Fptr *fptr)</arglist>
    </member>
    <member kind="function">
      <type>int</type>
      <name>Fis_token_char</name>
      <anchorfile>md__compiler_8c.html</anchorfile>
      <anchor>c297e377519f6ba4e2fd71db9b5b4fbf</anchor>
      <arglist>(int ch)</arglist>
    </member>
    <member kind="function">
      <type>int</type>
      <name>Fget_token</name>
      <anchorfile>md__compiler_8c.html</anchorfile>
      <anchor>eb1a56e9e7d7b8a58203813ef35c7452</anchor>
      <arglist>(Fptr *fptr, Mbuf *mbuf)</arglist>
    </member>
    <member kind="function">
      <type>int</type>
      <name>is_md_ident</name>
      <anchorfile>md__compiler_8c.html</anchorfile>
      <anchor>39ca8b26d2e0c556d2146f76b6c1f51a</anchor>
      <arglist>(char *ident)</arglist>
    </member>
    <member kind="function">
      <type>MD *</type>
      <name>build_md</name>
      <anchorfile>md__compiler_8c.html</anchorfile>
      <anchor>90fa8dea05b29f1ac530a80fb360bdf8</anchor>
      <arglist>(FILE *in, char *md_name)</arglist>
    </member>
    <member kind="function">
      <type>void</type>
      <name>build_field_decl</name>
      <anchorfile>md__compiler_8c.html</anchorfile>
      <anchor>0449f410c0cc6716a3783605146dcaf9</anchor>
      <arglist>(MD *md, MD_Section *section, Fptr *fptr, Mbuf *mbuf)</arglist>
    </member>
    <member kind="function">
      <type>int</type>
      <name>build_int</name>
      <anchorfile>md__compiler_8c.html</anchorfile>
      <anchor>d0b32dcf1d402933ef74da0913f1ae56</anchor>
      <arglist>(char *string, int *value)</arglist>
    </member>
    <member kind="function">
      <type>int</type>
      <name>build_double</name>
      <anchorfile>md__compiler_8c.html</anchorfile>
      <anchor>0532d81cd75c1447810194f860b849a2</anchor>
      <arglist>(char *string, double *value)</arglist>
    </member>
    <member kind="function">
      <type>void</type>
      <name>strip_first_and_last_char</name>
      <anchorfile>md__compiler_8c.html</anchorfile>
      <anchor>ba55bcbd6888bc153c67816b6bbf34a5</anchor>
      <arglist>(char *string)</arglist>
    </member>
    <member kind="function">
      <type>void</type>
      <name>strip_backslashes</name>
      <anchorfile>md__compiler_8c.html</anchorfile>
      <anchor>6c965136793f17f282274459e488269a</anchor>
      <arglist>(char *string)</arglist>
    </member>
    <member kind="function">
      <type>void</type>
      <name>build_element</name>
      <anchorfile>md__compiler_8c.html</anchorfile>
      <anchor>de61fda8263e72bad493eade04a924c8</anchor>
      <arglist>(MD_Field *field, MD_Element_Req *element_req, int element_index, Fptr *fptr, Mbuf *mbuf)</arglist>
    </member>
    <member kind="function">
      <type>void</type>
      <name>build_entry</name>
      <anchorfile>md__compiler_8c.html</anchorfile>
      <anchor>4a74b90f11f6771e28a937a67442c253</anchor>
      <arglist>(MD_Section *section, Fptr *fptr, Mbuf *mbuf)</arglist>
    </member>
    <member kind="function">
      <type>int</type>
      <name>check_md_for_ambiguous_links</name>
      <anchorfile>md__compiler_8c.html</anchorfile>
      <anchor>5dc602835cc716539a36da4c7fe7fa45</anchor>
      <arglist>(FILE *out, MD *md)</arglist>
    </member>
    <member kind="function">
      <type>void</type>
      <name>L_punt</name>
      <anchorfile>md__compiler_8c.html</anchorfile>
      <anchor>c8ccfa451c1bd1740a1b720ba5b4c3a9</anchor>
      <arglist>(char *fmt,...)</arglist>
    </member>
    <member kind="variable">
      <type>Mbuf *</type>
      <name>input_buf</name>
      <anchorfile>md__compiler_8c.html</anchorfile>
      <anchor>25678326d4f061f5d3e7640b7981d682</anchor>
      <arglist></arglist>
    </member>
    <member kind="variable">
      <type>char *</type>
      <name>program_name</name>
      <anchorfile>md__compiler_8c.html</anchorfile>
      <anchor>289c5900d90626d909f0a85d5a0ed61d</anchor>
      <arglist></arglist>
    </member>
    <member kind="variable">
      <type>char *</type>
      <name>input_file_name</name>
      <anchorfile>md__compiler_8c.html</anchorfile>
      <anchor>a91ff4ee3ee60baee9c097d4da3e3d18</anchor>
      <arglist></arglist>
    </member>
    <member kind="variable">
      <type>char *</type>
      <name>output_file_name</name>
      <anchorfile>md__compiler_8c.html</anchorfile>
      <anchor>cdcd0d11bb729145f180d52a0755cddf</anchor>
      <arglist></arglist>
    </member>
    <member kind="variable">
      <type>int</type>
      <name>using_stdin</name>
      <anchorfile>md__compiler_8c.html</anchorfile>
      <anchor>bc340a0cfa46e3f25a4d1adab28331c0</anchor>
      <arglist></arglist>
    </member>
    <member kind="variable">
      <type>char *</type>
      <name>output_mode</name>
      <anchorfile>md__compiler_8c.html</anchorfile>
      <anchor>14a61661c70b91f945e8456ead9fb6c1</anchor>
      <arglist></arglist>
    </member>
    <member kind="variable">
      <type>char *</type>
      <name>input_mode</name>
      <anchorfile>md__compiler_8c.html</anchorfile>
      <anchor>f81839da89e889adb517d1fc96a456f9</anchor>
      <arglist></arglist>
    </member>
    <member kind="variable">
      <type>int</type>
      <name>output_page_width</name>
      <anchorfile>md__compiler_8c.html</anchorfile>
      <anchor>bbe586720dc5d6766839fbdc260002c1</anchor>
      <arglist></arglist>
    </member>
  </compound>
  <compound kind="file">
    <name>md_compiler.h</name>
    <path>/mnt/hgfs/mkt/Desktop/openimpact/src/machine/md_compiler/</path>
    <filename>md__compiler_8h</filename>
    <class kind="struct">Fptr</class>
    <member kind="define">
      <type>#define</type>
      <name>MAXARGS</name>
      <anchorfile>md__compiler_8h.html</anchorfile>
      <anchor>41101847771d39a4f0a7f9395061c629</anchor>
      <arglist></arglist>
    </member>
    <member kind="define">
      <type>#define</type>
      <name>SECTION_START</name>
      <anchorfile>md__compiler_8h.html</anchorfile>
      <anchor>62f1bc5976cd195e942d569ea9bcb5ec</anchor>
      <arglist></arglist>
    </member>
    <member kind="define">
      <type>#define</type>
      <name>SECTION_END</name>
      <anchorfile>md__compiler_8h.html</anchorfile>
      <anchor>d5aae763518d851c59598275848e999a</anchor>
      <arglist></arglist>
    </member>
    <member kind="define">
      <type>#define</type>
      <name>FIELD_START</name>
      <anchorfile>md__compiler_8h.html</anchorfile>
      <anchor>7fbfa64753ca50463cbe92c1f585fa3b</anchor>
      <arglist></arglist>
    </member>
    <member kind="define">
      <type>#define</type>
      <name>FIELD_END</name>
      <anchorfile>md__compiler_8h.html</anchorfile>
      <anchor>135c2f21d955e76e6b67a2ffb392fd43</anchor>
      <arglist></arglist>
    </member>
    <member kind="define">
      <type>#define</type>
      <name>ENTRY_START</name>
      <anchorfile>md__compiler_8h.html</anchorfile>
      <anchor>caed8d0d130b5f5ae31d159672796994</anchor>
      <arglist></arglist>
    </member>
    <member kind="define">
      <type>#define</type>
      <name>ENTRY_END</name>
      <anchorfile>md__compiler_8h.html</anchorfile>
      <anchor>a979764be06bb7bdf7f69857b1600713</anchor>
      <arglist></arglist>
    </member>
    <member kind="define">
      <type>#define</type>
      <name>LINK_START</name>
      <anchorfile>md__compiler_8h.html</anchorfile>
      <anchor>230377f04a83fc1294f21bf7ddbf02d7</anchor>
      <arglist></arglist>
    </member>
    <member kind="define">
      <type>#define</type>
      <name>LINK_END</name>
      <anchorfile>md__compiler_8h.html</anchorfile>
      <anchor>51e12815879251fd7ab9d0133b8866c6</anchor>
      <arglist></arglist>
    </member>
    <member kind="define">
      <type>#define</type>
      <name>CREATE_SECTION</name>
      <anchorfile>md__compiler_8h.html</anchorfile>
      <anchor>99eda2c85fadad91c0314c01cb00276a</anchor>
      <arglist></arglist>
    </member>
    <member kind="define">
      <type>#define</type>
      <name>TERMINATOR</name>
      <anchorfile>md__compiler_8h.html</anchorfile>
      <anchor>9957e6bff526d6cefef9b0b4eb1c6501</anchor>
      <arglist></arglist>
    </member>
    <member kind="define">
      <type>#define</type>
      <name>KLEENE_STAR</name>
      <anchorfile>md__compiler_8h.html</anchorfile>
      <anchor>a32f5750538c8896fc03c7f60baf8a46</anchor>
      <arglist></arglist>
    </member>
    <member kind="define">
      <type>#define</type>
      <name>OR_MARKER</name>
      <anchorfile>md__compiler_8h.html</anchorfile>
      <anchor>a174135ccbe6da5bc1a46fd65f48e3d1</anchor>
      <arglist></arglist>
    </member>
    <member kind="define">
      <type>#define</type>
      <name>REPLACE_FIELD</name>
      <anchorfile>md__compiler_8h.html</anchorfile>
      <anchor>3cb7b88246cabacf6a03c9a5c76f8c90</anchor>
      <arglist></arglist>
    </member>
    <member kind="define">
      <type>#define</type>
      <name>CONCAT_FIELD</name>
      <anchorfile>md__compiler_8h.html</anchorfile>
      <anchor>5836ed34fa554df2ff49d0eef31bd88a</anchor>
      <arglist></arglist>
    </member>
    <member kind="function">
      <type>int</type>
      <name>Fpeekc</name>
      <anchorfile>md__compiler_8h.html</anchorfile>
      <anchor>c06c4367d84d0b622fea24b7c6702f47</anchor>
      <arglist>(Fptr *fptr)</arglist>
    </member>
    <member kind="function">
      <type>int</type>
      <name>Fgetc</name>
      <anchorfile>md__compiler_8h.html</anchorfile>
      <anchor>50725b05de9292d7eb2eaa36a6d6fdcb</anchor>
      <arglist>(Fptr *fptr)</arglist>
    </member>
    <member kind="function">
      <type>void</type>
      <name>Fadvance_to_next_token</name>
      <anchorfile>md__compiler_8h.html</anchorfile>
      <anchor>de728546c62e9c27434f5e16b9581308</anchor>
      <arglist>(Fptr *fptr)</arglist>
    </member>
    <member kind="function">
      <type>int</type>
      <name>is_md_ident</name>
      <anchorfile>md__compiler_8h.html</anchorfile>
      <anchor>39ca8b26d2e0c556d2146f76b6c1f51a</anchor>
      <arglist>(char *ident)</arglist>
    </member>
    <member kind="function">
      <type>void</type>
      <name>Ferror</name>
      <anchorfile>md__compiler_8h.html</anchorfile>
      <anchor>761c4386743ac261985b813ab1a8040c</anchor>
      <arglist>(Fptr *fptr, char *fmt,...)</arglist>
    </member>
    <member kind="function">
      <type>void</type>
      <name>vFerror</name>
      <anchorfile>md__compiler_8h.html</anchorfile>
      <anchor>d52c3f1409f7b9a5f521e34fa8c866a3</anchor>
      <arglist>(Fptr *fptr, char *fmt, va_list args)</arglist>
    </member>
    <member kind="function">
      <type>void</type>
      <name>L_punt</name>
      <anchorfile>md__compiler_8h.html</anchorfile>
      <anchor>c8ccfa451c1bd1740a1b720ba5b4c3a9</anchor>
      <arglist>(char *fmt,...)</arglist>
    </member>
    <member kind="function">
      <type>MD *</type>
      <name>build_md</name>
      <anchorfile>md__compiler_8h.html</anchorfile>
      <anchor>3c0442d466df2078d29749bbff847c0d</anchor>
      <arglist>(FILE *in, char *input_file_name)</arglist>
    </member>
    <member kind="function">
      <type>int</type>
      <name>check_md_for_ambiguous_links</name>
      <anchorfile>md__compiler_8h.html</anchorfile>
      <anchor>5dc602835cc716539a36da4c7fe7fa45</anchor>
      <arglist>(FILE *out, MD *md)</arglist>
    </member>
    <member kind="function">
      <type>void</type>
      <name>build_field_decl</name>
      <anchorfile>md__compiler_8h.html</anchorfile>
      <anchor>0449f410c0cc6716a3783605146dcaf9</anchor>
      <arglist>(MD *md, MD_Section *section, Fptr *fptr, Mbuf *mbuf)</arglist>
    </member>
    <member kind="function">
      <type>void</type>
      <name>build_entry</name>
      <anchorfile>md__compiler_8h.html</anchorfile>
      <anchor>4a74b90f11f6771e28a937a67442c253</anchor>
      <arglist>(MD_Section *section, Fptr *fptr, Mbuf *mbuf)</arglist>
    </member>
    <member kind="function">
      <type>void</type>
      <name>build_element</name>
      <anchorfile>md__compiler_8h.html</anchorfile>
      <anchor>de61fda8263e72bad493eade04a924c8</anchor>
      <arglist>(MD_Field *field, MD_Element_Req *element_req, int element_index, Fptr *fptr, Mbuf *mbuf)</arglist>
    </member>
  </compound>
  <compound kind="struct">
    <name>Fptr</name>
    <filename>structFptr.html</filename>
    <member kind="variable">
      <type>Mptr *</type>
      <name>mptr</name>
      <anchorfile>structFptr.html</anchorfile>
      <anchor>3e61b13d0cce1ed75ce08ba56a497b8e</anchor>
      <arglist></arglist>
    </member>
    <member kind="variable">
      <type>Mptr *</type>
      <name>checkpoint</name>
      <anchorfile>structFptr.html</anchorfile>
      <anchor>7252c599b20902b03825e5a9e977353b</anchor>
      <arglist></arglist>
    </member>
    <member kind="variable">
      <type>Mbuf *</type>
      <name>source_name</name>
      <anchorfile>structFptr.html</anchorfile>
      <anchor>a8d1874176483110dcd232ff6efea824</anchor>
      <arglist></arglist>
    </member>
    <member kind="variable">
      <type>int</type>
      <name>source_line</name>
      <anchorfile>structFptr.html</anchorfile>
      <anchor>212c70f02f3ebf06dd2effc3fafca5db</anchor>
      <arglist></arglist>
    </member>
  </compound>
</tagfile>
