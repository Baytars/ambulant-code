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

f = CxxMethodGenerator(node_interface_ptr, 'locate_node',
    (stringptr, 'path', InMode),
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
    (InBuffer, 'data', InMode),
)
methods_node_interface.append(f)

f = CxxMethodGenerator(void, 'set_attribute',
    (stringptr, 'name', InMode),
    (stringptr, 'value', InMode),
)
methods_node_interface.append(f)

f = CxxMethodGenerator(int, 'get_numid',
)
methods_node_interface.append(f)

f = CxxMethodGenerator(net_url, 'get_url',
    (stringptr, 'attrname', InMode),
)
methods_node_interface.append(f)

f = CxxMethodGenerator(unsigned_int, 'size',
)
methods_node_interface.append(f)

f = CxxMethodGenerator(std_string, 'get_path_display_desc',
)
methods_node_interface.append(f)

f = CxxMethodGenerator(std_string, 'get_sig',
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
    (std_string, 'prefix', InMode+RefMode),
    (std_string, 'uri', InMode+RefMode),
)
methods_node_context.append(f)

f = CxxMethodGenerator(net_url, 'resolve_url',
    (net_url, 'rurl', InMode+RefMode),
)
methods_node_context.append(f)

f = CxxMethodGenerator(const_node_ptr, 'get_node',
    (std_string, 'idd', InMode+RefMode),
)
methods_node_context.append(f)

