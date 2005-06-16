/*
 * 
 * This file is part of Ambulant Player, www.ambulantplayer.org.
 * 
 * Copyright (C) 2003 Stiching CWI, 
 * Kruislaan 413, 1098 SJ Amsterdam, The Netherlands.
 * 
 * Ambulant Player is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 * 
 * Ambulant Player is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 * 
 * You should have received a copy of the GNU General Public License
 * along with Ambulant Player; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 * 
 * In addition, as a special exception, if you link Ambulant Player with
 * other files to produce an executable, this library does not by itself
 * cause the resulting executable to be covered by the GNU General Public
 * License. This exception does not however invalidate any other reason why
 * the executable file might be covered by the GNU General Public License.
 * 
 * As a special exception, the copyright holders of Ambulant Player give
 * you permission to link Ambulant Player with independent modules that
 * communicate with Ambulant Player solely through the region and renderer
 * interfaces, regardless of the license terms of these independent
 * modules, and to copy and distribute the resulting combined work under
 * terms of your choice, provided that every copy of the combined work is
 * accompanied by a complete copy of the source code of Ambulant Player
 * (the version of Ambulant Player used to produce the combined work),
 * being distributed under the terms of the GNU General Public License plus
 * this exception.  An independent module is a module which is not derived
 * from or based on Ambulant Player.
 * 
 * Note that people who make modified versions of Ambulant Player are not
 * obligated to grant this special exception for their modified versions;
 * it is their choice whether to do so.  The GNU General Public License
 * gives permission to release a modified version without this exception;
 * this exception also makes it possible to release a modified version
 * which carries forward this exception. 
 * 
 */

/* 
 * @$Id$ 
 */

#ifndef AMBULANT_LIB_NODE_H
#define AMBULANT_LIB_NODE_H

#include "ambulant/config/config.h"
#include "ambulant/net/url.h"

// q_name_pair and qattr
#include "ambulant/lib/sax_types.h"

// tree iterators
#include "ambulant/lib/node_iterator.h" 

// attribute pair
#include <utility>

// q_attributes_list
#include <vector>

// return list of nodes
#include <list>

// return map of id -> nodes
#include <map>

#ifndef AMBULANT_NO_IOSTREAMS
// operator<<
#ifndef AMBULANT_NO_OSTREAM
#include <ostream>
#else /*AMBULANT_NO_OSTREAM*/
#include <ostream.h>
#endif/*AMBULANT_NO_OSTREAM*/
#endif

namespace ambulant {

namespace lib {

class custom_test;
class node_context;
class node_interface;
class node_impl;
#if WITH_EXTERNAL_DOM
typedef node_interface node;
#else
typedef node_impl node;
#endif

/// Simple tree node with tag, data and attributes.
/// The node trees are not fully DOM compliant, but should
/// be compatible with a bit of glue code.
/// The parent of each node is also its owner and container.

class node_interface {

  public:
  
	///////////////////////////////
	// tree iterators
	typedef tree_iterator<node_interface> iterator;
	typedef const_tree_iterator<node_interface> const_iterator;
	
	typedef std::list<const node*> const_node_list;
	
	/// Destruct this node and its contents.
	/// If this node is part of a tree, detach it first
	/// and then delete the node and its contents.
	virtual ~node_interface() {}


	/// Return first child of this node.
	virtual const node_interface *down() const = 0;
	
	/// Return parent of this node.
	virtual const node_interface *up() const = 0;
	
	/// Return next sibling of this node.
	virtual const node_interface *next() const = 0;

	/// Return first child of this node.
	virtual node_interface *down()  = 0;
	
	/// Return parent of this node.
	virtual node_interface *up()  = 0;
	
	/// Return next sibling of this node.
	virtual node_interface *next()  = 0;


	/// Set first child of this node.
	virtual void down(node_interface *n)  = 0;
	
	/// Set parent of this node.
	virtual void up(node_interface *n)  = 0;
	
	/// Set next sibling of this node.
	virtual void next(node_interface *n)  = 0;
	
	/// Returns the previous sibling node 
	/// or null when this is the first child.
	virtual const node_interface* previous() const = 0;
	
	/// Returns the last child 
	/// or null when this has not any children.
	virtual const node_interface* get_last_child() const = 0;
	
	/// Appends the children of this node (if any) to the provided list.
	virtual void get_children(const_node_list& l) const = 0;

	///////////////////////////////
	// search operations 
	// this section should be extented to allow for XPath selectors

	/// Find a node given a path of the form tag/tag/tag.
	virtual node_interface* locate_node(const char *path) = 0;
	
	/// Find the first direct child with the given tag.
	virtual node_interface *get_first_child(const char *name) = 0;
	
	/// Find the first direct child with the given tag.
	virtual const node_interface *get_first_child(const char *name) const = 0;
	
#if 0
	/// Find all descendants with the given tag.
	virtual void find_nodes_with_name(const xml_string& name, std::list<node_interface*>& list) = 0;
#endif
	
	/// Find the root of the tree to which this node belongs.
	virtual node_interface* get_root() = 0;
	
	/// Get an attribute from this node or its nearest ancestor that has the attribute.
	virtual const char *get_container_attribute(const char *name) const = 0;
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
	virtual node_interface* append_child(node_interface* child) = 0;		
	
	/// Append a new child node with the given name to this node.
	virtual node_interface* append_child(const char *name) = 0;

	/// Detach this node and its subtree from its parent tree.
	virtual node_interface* detach() = 0;
	
	/// Create a deep copy of this node and its subtree.
	virtual node_interface* clone() const = 0;
	
	/// Append data to the data of this node.
	virtual void append_data(const char *data, size_t len) = 0;
	
	/// Append c_str to the data of this node.
	virtual void append_data(const char *c_str) = 0;
	
	/// Append str to the data of this node.
	virtual void append_data(const xml_string& str) = 0;

	/// Add an attribute/value pair.
	virtual void set_attribute(const char *name, const char *value) = 0;

	/// Add an attribute/value pair.
	virtual void set_attribute(const char *name, const xml_string& value) = 0;

	/// Set a number of attribute/value pairs.
	/// Note: attrs are as per expat parser
	/// e.g. const char* attrs[] = {"attr_name", "attr_value", ..., 0};
	virtual void set_attributes(const char **attrs) = 0;
	
	/// Set the namespace for this node.
	virtual void set_namespace(const xml_string& ns) = 0;
	
	/////////////////////
	// data queries

	/// Return the namespace part of the tag for this node.
	virtual const xml_string& get_namespace() const = 0;
	
	/// Return the local part of the tag for this node.
	virtual const xml_string& get_local_name() const = 0;
	
	/// Return namespace and local part of the tag for this node.
	virtual const q_name_pair& get_qname() const = 0;
	
	/// Return the unique numeric ID for this node.
	virtual int get_numid() const = 0;
	
	/// Return the data for this node.
	virtual const xml_string& get_data() const = 0;
	
	/// Return the trimmed data for this node.
	virtual xml_string get_trimmed_data() const = 0;

#if 0
	virtual bool has_graph_data() const = 0;
#endif
	
	/// Return the value for the given attribute.
	virtual const char *get_attribute(const char *name) const = 0;
	
	/// Return the value for the given attribute.
	virtual const char *get_attribute(const std::string& name) const = 0;
	
	/// Return the value for the given attribute, interpreted as a URL.
	/// Relative URLs are resolved against the document base URL, if possible.
	virtual net::url get_url(const char *attrname) const = 0;
	
#if 0
	/// Return a reference to all attributes.
	virtual const q_attributes_list& get_attrs() const = 0;
#endif	

	/// Return the number of nodes of the xml (sub-)tree starting at this node.
	virtual unsigned int size() const = 0;

#if 0
	/// Fills in a map with node ids.
	/// the map may be used for retreiving nodes from their id.
	virtual void create_idmap(std::map<std::string, node_interface*>& m) const = 0;
#endif
	
	/// Returns a "friendly" path desription of this node.
	virtual std::string get_path_display_desc() const = 0;
	
	/// Return a friendly string describing this node.
	/// The string will be of a form similar to \<tag id="...">
	virtual std::string get_sig() const = 0;
	
	/////////////////////
	// string repr
	
	/// Return the
	virtual xml_string xmlrepr() const = 0;
#if 0
	virtual xml_string to_string() const = 0;
	virtual xml_string to_trimmed_string() const = 0;
#endif	
	/////////////////////
	// node context
	
	/// Return the node_context for this node.
	virtual const node_context* get_context() const = 0;
	
	/// Set the node_context for this node.
	virtual void set_context(node_context *c) = 0;
};

// Typedef trickery. Most of the code refers to "node", but this can be one of two things:
// - If we're not using an external DOM they get "node_impl", with inline methods and other
//   performance optimizations
// - If we are using an external DOM they get the abstract node_interface.
#if WITH_EXTERNAL_DOM
typedef node_interface node;
#else
} // namespace lib 
} // namespace ambulant
#include "ambulant/lib/node_impl.h"
namespace ambulant {
namespace lib {
typedef node_impl node;
#endif

/// Interface of document class accesible to nodes.
class node_context {
  public:
	typedef std::map<std::string, custom_test> custom_test_map;
	
	virtual void 
	set_prefix_mapping(const std::string& prefix, const std::string& uri) = 0;
	
	virtual const char* 
	get_namespace_prefix(const xml_string& uri) const = 0;
	
	/// Resolve relative URLs.
	virtual net::url 
	resolve_url(const net::url& rurl) const = 0;
	
	/// Returns name-indexed mapping of all custom tests used.
	virtual const custom_test_map*
	get_custom_tests() const = 0;

	/// Return the root of the document
	virtual const node* get_root() const = 0;

	/// Return node with a given ID.
	virtual const node* 
	get_node(const std::string& idd) const = 0;
};

#if WITH_EXTERNAL_DOM
// Factory functions. These are defined in node.cpp, and will return
// node_impl objects.
node_interface *node_factory(const char *local_name, const char **attrs = 0, const node_context *ctx = 0);

/// Construct a new, unconnected, node.
/// Note: attrs are as per expat parser
/// e.g. const char* attrs[] = {"attr_name", "attr_value", ..., 0};
node_interface *node_factory(const xml_string& local_name, const char **attrs = 0, const node_context *ctx = 0);

/// Construct a new, unconnected, node.
/// Note: attrs are as per expat parser
/// e.g. const char* attrs[] = {"attr_name", "attr_value", ..., 0};
node_interface *node_factory(const q_name_pair& qn, const q_attributes_list& qattrs, const node_context *ctx = 0);

// shallow copy from other.
node_interface *node_factory(const node* other);
#else
// Factory functions. These call node_impl constructors directly.
inline node *
node_factory(const char *local_name, const char **attrs = 0, const node_context *ctx = 0)
{
	return new node(local_name, attrs, ctx);
}

/// Construct a new, unconnected, node.
/// Note: attrs are as per expat parser
/// e.g. const char* attrs[] = {"attr_name", "attr_value", ..., 0};
inline node *
node_factory(const xml_string& local_name, const char **attrs = 0, const node_context *ctx = 0)
{
	return new node(local_name, attrs, ctx);
}

/// Construct a new, unconnected, node.
/// Note: attrs are as per expat parser
/// e.g. const char* attrs[] = {"attr_name", "attr_value", ..., 0};
inline node *
node_factory(const q_name_pair& qn, const q_attributes_list& qattrs, const node_context *ctx = 0)
{
	return new node(qn, qattrs, ctx);
}

// shallow copy from other.
inline node *
node_factory(const node* other)
{
	return new node(other);
}
#endif // WITH_EXTERNAL_DOM

} // namespace lib
 
} // namespace ambulant

#ifndef AMBULANT_NO_IOSTREAMS
// global operator<< for node objects
std::ostream& operator<<(std::ostream& os, const ambulant::lib::node& n);
#endif

#endif // AMBULANT_LIB_TREE_NODE_H
