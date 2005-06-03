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


// If we are using an external DOM the nodes are virtual
#if WITH_EXTERNAL_DOM
#define VIRTUAL virtual
#define V_END = 0
#define V_INIT(x) ;
#else
#define VIRTUAL
#define V_END
#define V_INIT(x) x
#endif
#ifndef NODE_BASECLASS
#define NODE_BASECLASS
#endif

namespace ambulant {

namespace lib {

class node_context;

/// Simple tree node with tag, data and attributes.
/// The node trees are not fully DOM compliant, but should
/// be compatible with a bit of glue code.
/// The parent of each node is also its owner and container.

class node NODE_BASECLASS {

  public:
  
	///////////////////////////////
	// tree iterators
	typedef tree_iterator<node> iterator;
	typedef const_tree_iterator<node> const_iterator;
	
#if !WITH_EXTERNAL_DOM
	///////////////////////////////
	// Constructors
	
	/// Construct a new, unconnected, node.
	/// Note: attrs are as per expat parser
	/// e.g. const char* attrs[] = {"attr_name", "attr_value", ..., 0};
	node(const char *local_name, const char **attrs = 0, const node_context *ctx = 0);

	/// Construct a new, unconnected, node.
	/// Note: attrs are as per expat parser
	/// e.g. const char* attrs[] = {"attr_name", "attr_value", ..., 0};
	node(const xml_string& local_name, const char **attrs = 0, const node_context *ctx = 0);

	/// Construct a new, unconnected, node.
	/// Note: attrs are as per expat parser
	/// e.g. const char* attrs[] = {"attr_name", "attr_value", ..., 0};
	node(const q_name_pair& qn, const q_attributes_list& qattrs, const node_context *ctx = 0);
	
	// shallow copy from other.
	node(const node* other);
#endif

	/// Destruct this node and its contents.
	/// If this node is part of a tree, detach it first
	/// and then delete the node and its contents.
	virtual ~node() V_END;


	/// Return first child of this node.
	VIRTUAL const node *down() const V_INIT({ return m_child;})
	
	/// Return parent of this node.
	VIRTUAL const node *up() const V_INIT({ return m_parent;})
	
	/// Return next sibling of this node.
	VIRTUAL const node *next() const V_INIT({ return m_next;})

	/// Return first child of this node.
	VIRTUAL node *down()  V_INIT({ return m_child;})
	
	/// Return parent of this node.
	VIRTUAL node *up()  V_INIT({ return m_parent;})
	
	/// Return next sibling of this node.
	VIRTUAL node *next()  V_INIT({ return m_next;})


	/// Set first child of this node.
	VIRTUAL void down(node *n)  V_INIT({ m_child = n;})
	
	/// Set parent of this node.
	VIRTUAL void up(node *n)  V_INIT({ m_parent = n;})
	
	/// Set next sibling of this node.
	VIRTUAL void next(node *n)  V_INIT({ m_next = n;})
	
	/// Returns the previous sibling node 
	/// or null when this is the first child.
	VIRTUAL const node* previous() const V_END;
	
	/// Returns the last child 
	/// or null when this has not any children.
	VIRTUAL const node* get_last_child() const V_END;
	
	/// Appends the children of this node (if any) to the provided list.
	VIRTUAL void get_children(std::list<const node*>& l) const V_END;

	///////////////////////////////
	// search operations 
	// this section should be extented to allow for XPath selectors

	/// Find a node given a path of the form tag/tag/tag.
	VIRTUAL node* locate_node(const char *path) V_END;
	
	/// Find the first direct child with the given tag.
	VIRTUAL node *get_first_child(const char *name) V_END;
	
	/// Find the first direct child with the given tag.
	VIRTUAL const node *get_first_child(const char *name) const V_END;
	
	/// Find all descendants with the given tag.
	VIRTUAL void find_nodes_with_name(const xml_string& name, std::list<node*>& list) V_END;
	
	/// Find the root of the tree to which this node belongs.
	VIRTUAL node* get_root() V_END;
	
	/// Get an attribute from this node or its nearest ancestor that has the attribute.
	VIRTUAL const char *get_container_attribute(const char *name) const V_END;
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
	VIRTUAL node* append_child(node* child) V_END;
		
	
	/// Append a new child node with the given name to this node.
	VIRTUAL node* append_child(const char *name) V_END;

	/// Detach this node and its subtree from its parent tree.
	VIRTUAL node* detach() V_END;
	
	/// Create a deep copy of this node and its subtree.
	VIRTUAL node* clone() const V_END;
	
	/// Append data to the data of this node.
	VIRTUAL void append_data(const char *data, size_t len) V_END;
	
	/// Append c_str to the data of this node.
	VIRTUAL void append_data(const char *c_str) V_END;
	
	/// Append str to the data of this node.
	VIRTUAL void append_data(const xml_string& str) V_END;

	/// Add an attribute/value pair.
	VIRTUAL void set_attribute(const char *name, const char *value) V_END;

	/// Add an attribute/value pair.
	VIRTUAL void set_attribute(const char *name, const xml_string& value) V_END;

	/// Set a number of attribute/value pairs.
	/// Note: attrs are as per expat parser
	/// e.g. const char* attrs[] = {"attr_name", "attr_value", ..., 0};
	VIRTUAL void set_attributes(const char **attrs) V_END;
	
	/// Set the namespace for this node.
	VIRTUAL void set_namespace(const xml_string& ns) V_END;
	
	/////////////////////
	// data queries

	/// Return the namespace part of the tag for this node.
	VIRTUAL const xml_string& get_namespace() const V_INIT({ return m_qname.first;})
	
	/// Return the local part of the tag for this node.
	VIRTUAL const xml_string& get_local_name() const V_INIT({ return m_qname.second;})
	
	/// Return namespace and local part of the tag for this node.
	VIRTUAL const q_name_pair& get_qname() const V_INIT({ return m_qname;})
	
	/// Return the unique numeric ID for this node.
	VIRTUAL int get_numid() const V_INIT({return m_numid;})
	
	/// Return the data for this node.
	VIRTUAL const xml_string& get_data() const V_INIT({ return m_data;})
	
	/// Return the trimmed data for this node.
	VIRTUAL xml_string get_trimmed_data() const V_END;

	VIRTUAL bool has_graph_data() const V_END;
	
	/// Return the value for the given attribute.
	VIRTUAL const char *get_attribute(const char *name) const V_END;
	
	/// Return the value for the given attribute.
	VIRTUAL const char *get_attribute(const std::string& name) const V_END;
	
	/// Return the value for the given attribute, interpreted as a URL.
	/// Relative URLs are resolved against the document base URL, if possible.
	VIRTUAL net::url get_url(const char *attrname) const V_END;
	
	/// Return a reference to all attributes.
	VIRTUAL const q_attributes_list& get_attrs() const V_INIT({ return m_qattrs;})
	

	/// Return the number of nodes of the xml (sub-)tree starting at this node.
	VIRTUAL unsigned int size() const V_END;

	/// Fills in a map with node ids.
	/// the map may be used for retreiving nodes from their id.
	VIRTUAL void create_idmap(std::map<std::string, node*>& m) const V_END; 
	
	/// Returns a "friendly" path desription of this node.
	VIRTUAL std::string get_path_display_desc() const V_END;
	
	/// Return a friendly string describing this node.
	/// The string will be of a form similar to \<tag id="...">
	VIRTUAL std::string get_sig() const V_END;
	
	/////////////////////
	// string repr
	
	/// Return the
	VIRTUAL xml_string xmlrepr() const V_END;
	VIRTUAL xml_string to_string() const V_END;
	VIRTUAL xml_string to_trimmed_string() const V_END;
	
#ifndef AMBULANT_NO_IOSTREAMS
	VIRTUAL void dump(std::ostream& os) const V_END;
#endif

	/////////////////////
	// node context
	
	/// Return the node_context for this node.
	VIRTUAL const node_context* get_context() const V_INIT({ return m_context;})
	
	/// Set the node_context for this node.
	VIRTUAL void set_context(node_context *c) V_INIT({ m_context = c;})
	
#if !WITH_EXTERNAL_DOM
	/// Return the next unique ID.
	VIRTUAL static int get_node_counter() V_INIT({return node_counter;})
	
  /////////////
  protected:
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
	
	const node& operator =(const node& o);
	
  private:
	// tree bonds
	node *m_parent;
	node *m_next;
	node *m_child;
	
	// verifier
	static int node_counter;
#endif
};

#ifndef SKIP_NODE_FACTORIES
#if WITH_EXTERNAL_DOM
// Factory functions
node *node_factory(const char *local_name, const char **attrs = 0, const node_context *ctx = 0);

/// Construct a new, unconnected, node.
/// Note: attrs are as per expat parser
/// e.g. const char* attrs[] = {"attr_name", "attr_value", ..., 0};
node *node_factory(const xml_string& local_name, const char **attrs = 0, const node_context *ctx = 0);

/// Construct a new, unconnected, node.
/// Note: attrs are as per expat parser
/// e.g. const char* attrs[] = {"attr_name", "attr_value", ..., 0};
node *node_factory(const q_name_pair& qn, const q_attributes_list& qattrs, const node_context *ctx = 0);

// shallow copy from other.
node *node_factory(const node* other);
#else
// Factory functions
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
#endif // SKIP_NODE_FACTORIES

} // namespace lib
 
} // namespace ambulant

#ifndef AMBULANT_NO_IOSTREAMS
// global operator<< for node objects
std::ostream& operator<<(std::ostream& os, const ambulant::lib::node& n);
#endif

#endif // AMBULANT_LIB_TREE_NODE_H
