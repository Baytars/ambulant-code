
/* 
 * @$Id$ 
 */

/*
 * The contents of this file are subject to the Public
 * License XXX. 
 *
 */

#include "ambulant/lib/node.h"

// find_if, etc
#include <algorithm>

// ostringstream
#include <sstream>

// assert
#include <cassert>

// trim strings
#include "ambulant/lib/string_util.h"

// tree helper iterators and visitors
#include "ambulant/lib/node_navigator.h" 


using namespace ambulant;

// This module starts with a set of private node visitors
// and continues with the implementation of lib::node.

// Node visitors are used by lib::node member functions

////////////////////////
// private output_visitor
// Writes a tree to an ostream.

template <class Node>
class output_visitor {
	std::ostream& os;
	std::basic_string<char> writesp, strincr;
	size_t ns;

  public:
	output_visitor(std::ostream& os_) 
	:	os(os_), strincr("  ") {ns = strincr.length();}
	void operator()(std::pair<bool, const Node*> x);

  private:
	void write_start_tag_no_children(const Node*& pe);
	void write_start_tag_with_children(const Node*& pe);
	void write_end_tag_with_children(const Node*& pe);
};

////////////////////////
// private trimmed_output_visitor
// Writes a tree to an ostream without white space.

template <class Node>
class trimmed_output_visitor {
	std::ostream& os;

  public:
	trimmed_output_visitor(std::ostream& os_) : os(os_) {}
	void operator()(std::pair<bool, const Node*> x);
	
  private:
	void write_start_tag_no_children(const Node*& pe);
	void write_start_tag_with_children(const Node*& pe);
	void write_end_tag_with_children(const Node*& pe);
};

////////////////////////
// private count_visitor
// Counts tree nodes

template <class Node>
class count_visitor {
  public:
	count_visitor(unsigned int& count) : m_count(count) {}
	void operator()(std::pair<bool, const Node*> x) 
		{if(x.first) m_count++;}
  private:
	unsigned int& m_count;
};

////////////////////////
// private attr_collector
// Scans the tree and creates a map attr to nodes.
// Intented for unique attrs like name and id

template <class Node>
class attr_collector {
  public:
	attr_collector(std::map<std::string, Node*>& m, const char *attr = "id") : 
		m_map(m), m_attr(attr) {}
		
	void operator()(std::pair<bool, const Node*> x) {
		if(x.first) {
			const char *value = x.second->get_attribute(m_attr);
			if(value != 0) 
				m_map[value] = const_cast<Node*>(x.second);
		}
	}
  private:
	std::string m_attr;
	std::map<std::string, Node*>& m_map;
};

///////////////////////////////////////////////
// lib::node implementation

//////////////////////
// Node constructors

// Note: attrs are as per expat parser
// e.g. const char* attrs[] = {"attr_name", "attr_value", ..., 0};

lib::node::node(const char *local_name, const char **attrs)
:	m_parent(0), m_next(0), m_child(0), m_qname("",(local_name?local_name:"error")) {
	set_attributes(attrs);
}

lib::node::node(const xml_string& local_name, const char **attrs)
	:	m_parent(0), m_next(0), m_child(0), m_qname("", local_name) {
		set_attributes(attrs);
}

// shallow copy from other
lib::node::node(const node* other)
	:	m_parent(0), m_next(0), m_child(0), 
		m_qname(other->get_qname()),
		m_data(other->get_data()),
		m_qattrs(other->get_attrs()) {}

lib::node::node(const q_name_pair& qn, const q_attributes_list& qattrs)
	:	m_parent(0), m_next(0), m_child(0), m_qname(qn), m_qattrs(qattrs) {
}

//////////////////////
// Node destructor

lib::node::~node() { 
	node_navigator<node>::delete_tree(this); 
}

///////////////////////////////
// basic navigation
// inline

//////////////////////
// set down/up/next
// inline

///////////////////////////////
// deduced navigation

const lib::node* 
lib::node::previous() const { 
	return node_navigator<node>::previous(this); 
}

const lib::node* 
lib::node::get_last_child() const { 
	return node_navigator<node>::last_child(this);
}

void lib::node::get_children(std::list<const node*>& l) const {
	node_navigator<node>::get_children(this, l);
}

///////////////////////////////
// search operations 

lib::node* 
lib::node::get_first_child(const char *name) {
	node *e = down();
	if(!e) return 0;
	if(e->m_qname.second == name) return e;
	while((e=e->next())) if(e->m_qname.second == name) return e;
	return 0;
}

lib::node* 
lib::node::locate_node(const char *path) {
	string_record r(path, "/");
	node *n = this;
	for(string_record::iterator it = r.begin(); it != r.end() && n != 0;it++)
		n = n->get_first_child(*it);
	return n;
}

void lib::node::find_nodes_with_name(const xml_string& name, std::list<node*>& lst) {
	iterator last = end(); // call once
	for(iterator it = begin(); it != last; it++)
		if((*it).first && (*it).second->get_local_name() == name) lst.push_back((*it).second);
}

lib::node* 
lib::node::get_root() { 
	return node_navigator<node>::get_root(this); 
}

///////////////////////////////
// iterators
// inline

///////////////////////
// build tree functions
	
lib::node* 
lib::node::append_child(node* child) { 
	return node_navigator<node>::append_child(this, child);
}
		
lib::node* 
lib::node::append_child(const char *name) { 
	return append_child(new node(name));
}

lib::node* 
lib::node::detach() { 
	return node_navigator<node>::detach(this); 
}
	
void lib::node::append_data(const char *data, size_t len) { 
	if(len>0) m_data.append(data, len);
}

void lib::node::append_data(const char *c_str) { 
	if(c_str == 0 || c_str[0] == 0) return;
	m_data.append(c_str, strlen(c_str));
}

void lib::node::append_data(const xml_string& str)
	{ m_data += str;}

void lib::node::set_attribute(const char *name, const char *value){ 
	if(name && name[0]) {
		q_name_pair qn("", name);
		q_attribute_pair qattr(qn, (value?value:""));
		m_qattrs.push_back(qattr);
	}
}

void lib::node::set_attribute(const char *name, const xml_string& value) {
	if(name && name[0]) {
		q_name_pair qn("", name);
		q_attribute_pair qattr(qn, value);
		m_qattrs.push_back(qattr);
	}
}

// Note: attrs are as per expat parser
// e.g. const char* attrs[] = {"attr_name", "attr_value", ..., 0};
void lib::node::set_attributes(const char **attrs) {
	if(attrs == 0) return;
	for(int i=0;attrs[i];i+=2)
		set_attribute(attrs[i], attrs[i+1]);
}
	
void lib::node::set_namespace(const xml_string& ns) {
	m_qname.first = ns;
}

// create a deep copy of this
lib::node* 
lib::node::clone() const {
	node* c = new node(this);
	const node *e = down();
	if(e != 0) {
		c->append_child(e->clone());
		while((e = e->next())) c->append_child(e->clone());
	}
	return c;
}

////////////////////////
// data queries
// some are inline

lib::xml_string 
lib::node::get_trimmed_data() const { 
	return trim(m_data);
}

bool 
lib::node::has_graph_data() const { 
	if(m_data.empty()) return false;
	return std::find_if(m_data.begin(), m_data.end(), isgraph) != m_data.end();
}

const char *
lib::node::get_attribute(const char *name) const {
	if(!name || !name[0]) return 0;
	q_attributes_list::const_iterator it;
	for(it = m_qattrs.begin(); it != m_qattrs.end(); it++)
		if((*it).first.second == name) return (*it).second.c_str();
	return 0;
}

const char *
lib::node::get_attribute(const std::string& name) const {
	return get_attribute(name.c_str());
}

/////////////////////
// string repr

lib::xml_string 
lib::node::xmlrepr() const {
	xml_string s(m_qname.second);
	q_attributes_list::const_iterator it = m_qattrs.begin();
	while(it!=m_qattrs.end()) {
		s += " ";
		s += (*it).first.second;
		s += "=\"";
		s += (*it).second;
		s += "\"";
		it++;
	}
	return s;
}

lib::xml_string 
lib::node::to_string() const {
	std::ostringstream os;
	output_visitor<node> visitor(os);
	std::for_each(begin(), end(), visitor);
	return os.str();
}
	
lib::xml_string 
lib::node::to_trimmed_string() const {
	std::ostringstream os;
	trimmed_output_visitor<node> visitor(os);
	std::for_each(begin(), end(), visitor);
	return os.str();
}

unsigned int 
lib::node::size() const {
	unsigned int count = 0;
	count_visitor<node> visitor(count);
	std::for_each(begin(), end(), visitor);
	return count;
}



void lib::node::create_idmap(std::map<std::string, node*>& m) const {
	attr_collector<node> visitor(m);
	std::for_each(begin(), end(), visitor);
}

std::ostream& operator<<(std::ostream& os, const ambulant::lib::node& n) {
	output_visitor<ambulant::lib::node> visitor(os);
	std::for_each(n.begin(), n.end(), visitor);
	return os;
}

//////////////////////////////////////////////
//////////////////////////////////////////////
// Visitors implementations

////////////////////////
// output_visitor

template<class Node>
void output_visitor<Node>::operator()(std::pair<bool, const Node*> x) {
	const Node*& pe = x.second;
	if(x.first) {
		// start tag
		if(!pe->down()) 
			write_start_tag_no_children(pe);
		else 
			write_start_tag_with_children(pe);
		}
	else if(pe->down())
		write_end_tag_with_children(pe);
}

template<class Node>
void output_visitor<Node>::write_start_tag_no_children(const Node*& pe) {
	const std::string& data = pe->get_data();
	if(data.length()==0 || !pe->has_graph_data())
		os <<  writesp << "<" + pe->xmlrepr() + "/>" << std::endl;
	else {
		os <<  writesp << "<" + pe->xmlrepr() + ">";
		os << pe->get_data();
		os << "</" << pe->get_local_name() << ">" << std::endl;
	}
}

template<class Node>
void output_visitor<Node>::write_start_tag_with_children(const Node*& pe) {
	os <<  writesp << "<" + pe->xmlrepr() + ">";
	const std::string& data = pe->get_data();
	if(data.length()>0 && pe->has_graph_data())
		os << pe->get_data();
	os << std::endl;
	writesp += strincr;
}

template<class Node>
void output_visitor<Node>::write_end_tag_with_children(const Node*& pe) {
	writesp = writesp.substr(0,writesp.length()-ns);
	os << writesp << "</" + pe->get_local_name() << ">" << std::endl;
}


////////////////////////
// trimmed_output_visitor

template <class Node>
void trimmed_output_visitor<Node>::operator()(std::pair<bool, const Node*> x) {
	const Node*& pe = x.second;
	if(x.first) {
		// start tag
		if(!pe->down()) 
			write_start_tag_no_children(pe);
		else 
			write_start_tag_with_children(pe);
	}
	else if(pe->down())
		write_end_tag_with_children(pe);
}

template <class Node>
void trimmed_output_visitor<Node>::write_start_tag_no_children(const Node*& pe) {
	std::string data = pe->get_trimmed_data();
	if(data.length()==0)
		os <<  "<" + pe->xmlrepr() + "/>";
	else {
		os <<  "<" << pe->xmlrepr() << ">";
		os << data;
		os << "</" << pe->get_local_name() << ">";
	}
}

template <class Node>
void trimmed_output_visitor<Node>::write_start_tag_with_children(const Node*& pe) {
	os <<  "<" + pe->xmlrepr() + ">";
	std::string data = pe->get_trimmed_data();
	if(data.length()>0)
		os << data;
}

template <class Node>
void trimmed_output_visitor<Node>::write_end_tag_with_children(const Node*& pe) {
	os << "</" + pe->get_local_name() << ">";
}

