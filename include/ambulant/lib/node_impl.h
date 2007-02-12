/*
 * This file is part of Ambulant Player, www.ambulantplayer.org.
 *
 * Copyright (C) 2003-2007 Stichting CWI, 
 * Kruislaan 413, 1098 SJ Amsterdam, The Netherlands.
 *
 * Ambulant Player is free software; you can redistribute it and/or modify
 * it under the terms of the GNU Lesser General Public License as published by
 * the Free Software Foundation; either version 2.1 of the License, or
 * (at your option) any later version.
 *
 * Ambulant Player is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public License
 * along with Ambulant Player; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA  02110-1301  USA
 */

/* 
 * @$Id$ 
 */

#ifndef AMBULANT_LIB_NODE_IMPL_H
#define AMBULANT_LIB_NODE_IMPL_H

#include "ambulant/config/config.h"
#include "ambulant/lib/node.h"


namespace ambulant {

namespace lib {

/// Simple tree node with tag, data and attributes.
/// The node trees are not fully DOM compliant, but should
/// be compatible with a bit of glue code.
/// The parent of each node is also its owner and container.
#ifdef AMBULANT_PLATFORM_WIN32_WCE_3
class node_impl { // WinCE3 compiler has trouble with baseclass
#else
class node_impl : public node_interface {
#endif

  public:
  
	///////////////////////////////
	// tree iterators
	typedef tree_iterator<node_impl> iterator;
	typedef const_tree_iterator<node_impl> const_iterator;
	
	///////////////////////////////
	// Constructors
	
	/// Construct a new, unconnected, node.
	/// Note: attrs are as per expat parser
	/// e.g. const char* attrs[] = {"attr_name", "attr_value", ..., 0};
	node_impl(const char *local_name, const char **attrs = 0, const node_context *ctx = 0);

	/// Construct a new, unconnected, node.
	/// Note: attrs are as per expat parser
	/// e.g. const char* attrs[] = {"attr_name", "attr_value", ..., 0};
	node_impl(const xml_string& local_name, const char **attrs = 0, const node_context *ctx = 0);

	/// Construct a new, unconnected, node.
	/// Note: attrs are as per expat parser
	/// e.g. const char* attrs[] = {"attr_name", "attr_value", ..., 0};
	node_impl(const q_name_pair& qn, const q_attributes_list& qattrs, const node_context *ctx = 0);
	
	// shallow copy from other.
	node_impl(const node_impl* other);

	/// Destruct this node and its contents.
	/// If this node is part of a tree, detach it first
	/// and then delete the node and its contents.
	virtual ~node_impl();


	/// Return first child of this node.
	const node_impl *down() const { return m_child;}
	
	/// Return parent of this node.
	const node_impl *up() const { return m_parent;}
	
	/// Return next sibling of this node.
	const node_impl *next() const { return m_next;}

	/// Return first child of this node.
	node_impl *down()  { return m_child;}
	
	/// Return parent of this node.
	node_impl *up()  { return m_parent;}
	
	/// Return next sibling of this node.
	node_impl *next()  { return m_next;}


	/// Set first child of this node.
	void down(node_impl *n)  { m_child = n;}

	/// Set first child of this node, after dynamic typecheck
	void down(node_interface *n);
	
	/// Set parent of this node.
	void up(node_impl *n)  { m_parent = n;}

	/// Set parent of this node, after dynamic typecheck
	void up(node_interface *n);
	
	/// Set next sibling of this node.
	void next(node_impl *n)  { m_next = n;}

	/// Set next sibling of this node, after dynamic typecheck
	void next(node_interface *n);
	
	/// Returns the previous sibling node 
	/// or null when this is the first child.
	const node_impl* previous() const;
	
	/// Returns the last child 
	/// or null when this has not any children.
	const node_impl* get_last_child() const;
	
	/// Appends the children of this node (if any) to the provided list.
	void get_children(std::list<const node*>& l) const;

	///////////////////////////////
	// search operations 
	// this section should be extented to allow for XPath selectors

	/// Find a node given a path of the form tag/tag/tag.
	node_impl* locate_node(const char *path);
	
	/// Find the first direct child with the given tag.
	node_impl *get_first_child(const char *name);
	
	/// Find the first direct child with the given tag.
	const node_impl *get_first_child(const char *name) const;
		
	/// Find the root of the tree to which this node belongs.
	node_impl* get_root();
	
	/// Get an attribute from this node or its nearest ancestor that has the attribute.
	const char *get_container_attribute(const char *name) const;
	///////////////////////////////
	// iterators

	/// Return iterator for this node and its subtree.
    iterator begin() { return iterator(this);}

	/// Return iterator for this node and its subtree.
    const_iterator begin() const { return const_iterator(this);}

    iterator end() { return iterator(0);}
    const_iterator end() const { return const_iterator(0);}

	///////////////////////
	// build tree functions
	
	/// Append a child node to this node.
	node_impl* append_child(node_impl* child);

	/// Append a child node to this node, after dynamic typecheck
	node_interface* append_child(node_interface* child);

	/// Append a new child node with the given name to this node.
	node_impl* append_child(const char *name);

	/// Detach this node and its subtree from its parent tree.
	node_impl* detach();
	
	/// Create a deep copy of this node and its subtree.
	node_impl* clone() const;
	
	/// Append data to the data of this node.
	void append_data(const char *data, size_t len);
	
	/// Append c_str to the data of this node.
	void append_data(const char *c_str);
	
	/// Append str to the data of this node.
	void append_data(const xml_string& str);

	/// Add an attribute/value pair.
	void set_attribute(const char *name, const char *value);

	/// Add an attribute/value pair.
	void set_attribute(const char *name, const xml_string& value);

	/// Set a number of attribute/value pairs.
	/// Note: attrs are as per expat parser
	/// e.g. const char* attrs[] = {"attr_name", "attr_value", ..., 0};
	void set_attributes(const char **attrs);
	
	/// Set the namespace for this node.
	void set_namespace(const xml_string& ns);
	
	/////////////////////
	// data queries

	/// Return the namespace part of the tag for this node.
	const xml_string& get_namespace() const { return m_qname.first;}
	
	/// Return the local part of the tag for this node.
	const xml_string& get_local_name() const { return m_qname.second;}
	
	/// Return namespace and local part of the tag for this node.
	const q_name_pair& get_qname() const { return m_qname;}
	
	/// Return the unique numeric ID for this node.
	int get_numid() const {return m_numid;}
	
	/// Return the data for this node.
	const xml_string& get_data() const { return m_data;}
	
	/// Return the trimmed data for this node.
	xml_string get_trimmed_data() const;

	bool has_graph_data() const;
	
	/// Return the value for the given attribute.
	const char *get_attribute(const char *name) const;
	
	/// Return the value for the given attribute.
	const char *get_attribute(const std::string& name) const;
	
	/// Return the value for the given attribute.
	void del_attribute(const char *name);
	
	/// Return the value for the given attribute, interpreted as a URL.
	/// Relative URLs are resolved against the document base URL, if possible.
	net::url get_url(const char *attrname) const;
	
	/// Return a reference to all attributes.
	const q_attributes_list& get_attrs() const { return m_qattrs;}
	

	/// Return the number of nodes of the xml (sub-)tree starting at this node.
	unsigned int size() const;
	
	/// Returns a "friendly" path desription of this node.
	std::string get_path_display_desc() const;
	
	/// Return a friendly string describing this node.
	/// The string will be of a form similar to \<tag id="...">
	std::string get_sig() const;
	
	/////////////////////
	// string repr
	
	/// Return the
	xml_string xmlrepr() const;
	xml_string to_string() const;
	xml_string to_trimmed_string() const;
	
#ifndef AMBULANT_NO_IOSTREAMS
	void dump(std::ostream& os) const;
#endif

	/////////////////////
	// node context
	
	/// Return the node_context for this node.
	const node_context* get_context() const { return m_context;}
	
	/// Set the node_context for this node.
	void set_context(node_context *c) { m_context = c;}
	
	/// Return the next unique ID.
	static int get_node_counter() {return node_counter;}
	
  /////////////
  protected:
	/// Fills in a map with node ids.
	/// the map may be used for retreiving nodes from their id.
	void create_idmap(std::map<std::string, node_impl*>& m) const;

	// node data 
	// sufficient for a generic xml element
	
	// the qualified name of this element as std::pair
	q_name_pair m_qname;
	
	// the qualified name of this element as std::pair
	q_attributes_list m_qattrs;
	
	// the text data of this node
	xml_string m_data;
	
	// the context of this node
	const node_context *m_context;
	
	// a magic id
	int m_numid;
	
//	const node& operator =(const node& o);
	
  private:
	// tree bonds
	node_impl *m_parent;
	node_impl *m_next;
	node_impl *m_child;
	
	// verifier
	static int node_counter;
};

} // namespace lib
} // namespace ambulant

#endif // AMBULANT_LIB_NODE_IMPL_H
