
/* 
 * @$Id$ 
 */

/*
 * The contents of this file are subject to the Public
 * License XXX. 
 *
 */

#ifndef AMBULANT_LIB_DOCUMENT_H
#define AMBULANT_LIB_DOCUMENT_H

#include <string>

#include "ambulant/lib/node.h"
#include "ambulant/lib/nscontext.h"

// XXX: temp, will go to document.cpp
#include "ambulant/lib/tree_builder.h"
#include "ambulant/lib/logger.h"
#include <ostream>

// A class respresenting an XML document.
//
// This class is reachable from node objects.
// and provides context services to them.
//

namespace ambulant {

namespace lib {

// Interface accesible to nodes.
class node_context {
  public:
	
	virtual void 
	set_prefix_mapping(const std::string& prefix, const std::string& uri) = 0;
	
	virtual const char* 
	get_namespace_prefix(const xml_string& uri) const = 0;
	
	virtual std::string 
	resolve_url(const node *n, const std::string& rurl) const = 0;
};


class document : public node_context {

  public:
	// A document factory function.
	// Creates documents from local files.
	static document* create_from_file(const std::string& str);
	
	// This class maybe extented to more specific documents.
	// Therfore, use the virtual table to invoke the destructor.
	virtual ~document();
	
	// Returns the root node of this document
	node* get_root();
	
	// Returns the source url of this docuemnt
	const std::string& get_src_url() const { return m_src_url;}
	
	// node_context interface
	void set_prefix_mapping(const std::string& prefix, const std::string& uri);
	const char* get_namespace_prefix(const xml_string& uri) const;
	std::string resolve_url(const node *n, const std::string& rurl) const;
	
  protected:
	document(node *root = 0);
	document(node *root, const std::string& src_url);
	
  private:
	// the root of this document
	node *m_root;
	
	// the external source url
	std::string m_src_url;
	
	// document namespaces registry
	nscontext m_namespaces;
};


/////////////////////////////////
// Inline implementation

inline 
document::document(node *root) 
:	m_root(root) {
}

inline
document::document(node *root, const std::string& src_url) 
:	m_root(root), m_src_url(src_url) {
}

inline
document::~document() {
	delete m_root;
}

inline node* 
document::get_root() {
	return m_root;
}

//static 
inline document* 
document::create_from_file(const std::string& str) {
	document *d = new document();
	tree_builder builder(d);
	if(!builder.build_tree_from_file(str.c_str())) {
		logger::get_logger()->error(
			"Could not build tree for file: %s", str.c_str());
		return 0;
	}
	d->m_root = builder.detach();
	d->m_src_url = str; // XXX: wrong
	return d;
}

inline void 
document::set_prefix_mapping(const std::string& prefix, const std::string& uri) {
	m_namespaces.set_prefix_mapping(prefix, uri);
}

inline const char* 
document::get_namespace_prefix(const xml_string& uri) const {
	return m_namespaces.get_namespace_prefix(uri);
}

inline std::string 
document::resolve_url(const node *n, const std::string& rurl) const {
	// locate node context xml:base
	// if none is found use source.
	// return join(base, rurl)
	return m_src_url + "/" + rurl; // XXX: WRONG, just return somethig for now.
}

} // namespace lib
 
} // namespace ambulant

inline 
std::ostream& operator<<(std::ostream& os, const ambulant::lib::document& d) {
	return os << "document(" << (void *)&d << ", \"" << d.get_src_url() << "\")";
}

#endif // AMBULANT_LIB_DOCUMENT_H


