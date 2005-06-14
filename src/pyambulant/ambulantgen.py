# Generated from '/usr/local/include/ambulant/lib/node.h'

f = CxxMethodGenerator(void, 'down',
    (node_interface_ptr, 'n', InMode),
)
methods_node_interface.append(f)

f = CxxMethodGenerator(void, 'up',
    (node_interface_ptr, 'n', InMode),
)
methods_node_interface.append(f)

f = CxxMethodGenerator(void, 'next',
    (node_interface_ptr, 'n', InMode),
)
methods_node_interface.append(f)

f = CxxMethodGenerator(const_node_interface_ptr, 'previous',
)
methods_node_interface.append(f)

f = CxxMethodGenerator(const_node_interface_ptr, 'get_last_child',
)
methods_node_interface.append(f)

f = CxxMethodGenerator(void, 'get_children',
    (std_list<const_node_ptr>_ref, 'l', InMode),
)
methods_node_interface.append(f)

f = CxxMethodGenerator(node_interface_ptr, 'locate_node',
    (char_ptr, 'path', InMode),
)
methods_node_interface.append(f)

f = CxxMethodGenerator(void, 'find_nodes_with_name',
    (xml_string_ref, 'name', InMode),
    (std_list<node_interface_ptr>_ref, 'list', InMode),
)
methods_node_interface.append(f)

f = CxxMethodGenerator(node_interface_ptr, 'get_root',
)
methods_node_interface.append(f)

f = CxxMethodGenerator(node_interface_ptr, 'append_child',
    (node_interface_ptr, 'child', InMode),
)
methods_node_interface.append(f)

f = CxxMethodGenerator(node_interface_ptr, 'detach',
)
methods_node_interface.append(f)

f = CxxMethodGenerator(node_interface_ptr, 'clone',
)
methods_node_interface.append(f)

f = CxxMethodGenerator(void, 'append_data',
    (char_ptr, 'data', InMode),
    (size_t, 'len', InMode),
)
methods_node_interface.append(f)

f = CxxMethodGenerator(void, 'set_attribute',
    (char_ptr, 'name', InMode),
    (char_ptr, 'value', InMode),
)
methods_node_interface.append(f)

f = CxxMethodGenerator(void, 'set_attributes',
    (char_ptr_ptr, 'attrs', InMode),
)
methods_node_interface.append(f)

f = CxxMethodGenerator(void, 'set_namespace',
    (xml_string_ref, 'ns', InMode),
)
methods_node_interface.append(f)

f = CxxMethodGenerator(const_xml_string_ref, 'get_namespace',
)
methods_node_interface.append(f)

f = CxxMethodGenerator(const_xml_string_ref, 'get_local_name',
)
methods_node_interface.append(f)

f = CxxMethodGenerator(const_q_name_pair_ref, 'get_qname',
)
methods_node_interface.append(f)

f = CxxMethodGenerator(int, 'get_numid',
)
methods_node_interface.append(f)

f = CxxMethodGenerator(const_xml_string_ref, 'get_data',
)
methods_node_interface.append(f)

f = CxxMethodGenerator(xml_string, 'get_trimmed_data',
)
methods_node_interface.append(f)

f = CxxMethodGenerator(bool, 'has_graph_data',
)
methods_node_interface.append(f)

f = CxxMethodGenerator(net_url, 'get_url',
    (char_ptr, 'attrname', InMode),
)
methods_node_interface.append(f)

f = CxxMethodGenerator(const_q_attributes_list_ref, 'get_attrs',
)
methods_node_interface.append(f)

f = CxxMethodGenerator(unsigned_int, 'size',
)
methods_node_interface.append(f)

f = CxxMethodGenerator(void, 'create_idmap',
    (std_map<std, 'string', InMode),
    (node_interface_ptr>_ref, 'm', InMode),
)
methods_node_interface.append(f)

f = CxxMethodGenerator(std_string, 'get_path_display_desc',
)
methods_node_interface.append(f)

f = CxxMethodGenerator(std_string, 'get_sig',
)
methods_node_interface.append(f)

f = CxxMethodGenerator(xml_string, 'xmlrepr',
)
methods_node_interface.append(f)

f = CxxMethodGenerator(xml_string, 'to_string',
)
methods_node_interface.append(f)

f = CxxMethodGenerator(xml_string, 'to_trimmed_string',
)
methods_node_interface.append(f)

f = CxxMethodGenerator(const_node_context_ptr, 'get_context',
)
methods_node_interface.append(f)

f = CxxMethodGenerator(void, 'set_context',
    (node_context_ptr, 'c', InMode),
)
methods_node_interface.append(f)

f = CxxMethodGenerator(void, 'set_prefix_mapping',
    (std_string_ref, 'prefix', InMode),
    (std_string_ref, 'uri', InMode),
)
methods_node_context.append(f)

f = CxxMethodGenerator(const_char_ptr, 'get_namespace_prefix',
    (xml_string_ref, 'uri', InMode),
)
methods_node_context.append(f)

f = CxxMethodGenerator(net_url, 'resolve_url',
    (net_url_ref, 'rurl', InMode),
)
methods_node_context.append(f)

f = CxxMethodGenerator(const_node_ptr, 'get_node',
    (std_string_ref, 'idd', InMode),
)
methods_node_context.append(f)

