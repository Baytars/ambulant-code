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

#ifndef AMBULANT_LIB_STRING_UTIL_H
#define AMBULANT_LIB_STRING_UTIL_H

#include "ambulant/config/config.h"

#include <string>
#include <vector>
#include <ctype.h>

namespace ambulant {

namespace lib {

class tokens_vector : public std::vector<std::string> {
  public:
    tokens_vector(const char* entry, const char* delims);
	std::string join(size_type i, char sep);
};

inline std::string trim(const std::string& s) {
	size_t i1 = s.find_first_not_of(" \r\n\t\v");
	if(i1==std::string::npos) return "";

	// XXXX Need a signed type here...
	size_t i2 = s.length();
	while(i2>0 && isspace(s[i2-1])) i2--;
	if(i2==0) return "";

	return std::string(s.c_str()+i1,s.c_str()+i2);
}

inline std::string trim(const char* psz) {
	std::string s(psz);
	return trim(s);
}

inline std::string xml_quote(const char *p) {
	std::string qs;
	if(p) {
		while(*p) {
			if(*p=='<') qs += "&lt;";
			else if(*p=='>') qs += "&gt;";
			else if(*p=='&') qs += "&amp;";
			else qs += *p;
			p++;
		}
	}
	return qs;
}

inline std::string xml_quote(const std::string& s)
	{ return xml_quote(s.c_str());}

inline bool starts_with(const std::string& s, int offset, const char *p) {
	if(!p) return true;
	for(std::string::size_type i=offset;i<s.length() && *p;i++,p++) {
		if(*p != s[i]) break;
	}
	return *p == 0;
}

inline bool starts_with(const std::string& s, const char *p) {
	return starts_with(s, 0, p);
}

inline bool starts_with(const std::string& s, const std::string& b) {
	return starts_with(s, b.c_str());
}

inline bool ends_with(const std::string& s, const char *p) {
	if(!p) return true;
	const char *rend = p - 1;
	p += strlen(p) - 1;
	for(std::string::size_type i = s.length()-1;i>=0 && p != rend;i++,p++) {
		if(*p != s[i]) break;
	}
	return p == rend;
}
inline bool ends_with(const std::string& s, const std::string& e) {
	return ends_with(s, e.c_str());
}

///////////////////////////
// A generic string scanner/tokenizer
// May be used to tokenize URLs when we pass ":/?#" as delimiters.

template <class CharType>
class basic_scanner {
  public:
 	typedef CharType char_type;
 	typedef std::basic_string<char_type> string_type;
	typedef typename string_type::size_type size_type;
	enum {EOS = 0, NOT_DELIM = 'n'};
	
	// Creates a basic_scanner for the source string 's' and delimiters 'd'.
	basic_scanner(const string_type& s, const string_type& d)
	:	src(s), 
		delims(d), 
		end(s.length()),
		pos(0), tok(EOS) {}
	
	// Returns the next token or EOS if none is available.
	// The current position is at the start of the next token or at end.
	// The token returned is either a delimiter character
	// or the meta-character NOT_DELIM.
	// The NOT_DELIM token represents the occurence of a 
	// substring matching the regex [^delim]+
	char next() {
		tok = EOS;
		tokval = "";
		if(pos == end) return tok;
		size_type ix = delims.find_first_of(src[pos]);
		if(vpos(ix)) {
			tokval = tok = delims[ix];
			pos++;
		} else {
			scan_not_in_set_as(delims.c_str(), NOT_DELIM);
		}
		toks += tok;
		vals.push_back(tokval);
		return tok;
	}
		
	// Returns true when there are more tokens
	bool has_more() const { return pos != end;}
	
	// Returns the current token
	char get_tok() const { return tok;}
	
	// Returns the current token value
	const string_type& get_tokval() const { return tokval;}
	
	// Returns the src string 
	const string_type& get_src() const { return src;}
	
	// Returns the tokens seen
	const string_type& get_tokens() const { return toks;}
	
	// Returns the token values seen.
	const std::vector<string_type>& get_values() const { return vals;}
		
	// Tokenizes the source string.
	void tokenize() {
		if(pos>0) reset();
		while(next() != EOS);
	}
	
	// Returns the i_th token value.
	string_type val_at(size_type i) const {
		return (i<vals.size())?vals[i]:"";
	}
	
	// Joins token values with indices in [b,e).
	string_type join(size_type b, size_type e) const {
		string_type s;
		for(size_type ix = b; ix<e && ix<vals.size();ix++)
			s += vals[ix];
		return s;
	}
	
	// Joins token values with indices >= b.
	string_type join(size_type b) const {
		return join(b, vals.size());
	}
	
  protected:
  
	// Scans chars in the set 'cstr' as token 't'.
	void scan_set_as(const char *cstr, char t) {
		tok = t; 
		size_type ni = src.find_first_not_of(cstr, pos);
		if(vpos(ni)) {
			tokval = string_type(src.c_str() + pos, ni-pos);
			pos = ni;
		} else {
			tokval = string_type(src.c_str() + pos);
			pos = end;
		}
	}
	
	// Scans chars not in the set 'cstr' as token 't'.
	void scan_not_in_set_as(const char *cstr, char t) {
		tok = t; 
		size_type ni = src.find_first_of(cstr, pos);
		if(vpos(ni)) {
			tokval = string_type(src.c_str() + pos, ni-pos);
			pos = ni;
		} else {
			tokval = string_type(src.c_str() + pos);
			pos = end;
		}
	}
	
	// Skips chars in set 'cstr'.
	void skip_set(const char *cstr) {
		size_type ni = src.find_first_not_of(pszset, pos);
		if(ni != std::basic_string<char_type>::npos) pos = ni;
		else pos = end;
	}
	
	// Skips space chars.
	void skip_space() { skip_set(" \t\r\n");}
	
	// Resets this scanner; erases its memory
	void reset() {
		pos = 0;
		tok = EOS;
		toks = "";
		vals.clear();
	}
	
	bool vpos(size_type ix)  const { 
		return ix != std::basic_string<char_type>::npos;}

  private:
	
	// The source of this scanner
	const string_type src;
	
	// The literals to recognize
	const string_type delims;
	
	// Source end position 
	const size_type end;
	
	// Current pos
	size_type pos;
	
	// Current token
	char tok;
	
	// Current token value
	string_type tokval;
	
	// The tokens seen
	string_type toks;
	
	// The tokens values seen
	std::vector<string_type> vals;
	
};

typedef basic_scanner<char> scanner;
typedef basic_scanner<wchar_t> wscanner;


} // namespace lib
 
} // namespace ambulant


//////////////////////////
//
// A simple not optimized NFA based regular expression matcher.
//
// May be used to match short strings created by 
// the scanner against hand made (coded) regex. 
//
// The intented usage is:
// a) shring the string to be matched into tokens using the scanner above
// b) use the matcher against the short string of the tokens.
// This way for example a decimal becomes the literals: d | d.d
// instead of [0-9] | [0-9].[0-9] resulting to an improvement 
// nore than an order of magnitude (~20 times).

#include <set>
#include <stack>
#include <cassert>

namespace ambulant {

namespace lib {

const int EPSILON = -1;
const int ACCEPT  = -9;
const int REPEAT_LIMIT  = 1024;
const int GROUP_BEGIN = 1;
const int GROUP_END = 2;

// An nfa node represents a node in a
// nondderministic finite automaton (NFA)
// and is associated with 2 transitions 
// that occur on symbol 'edge'.
//
// NFA nodes could be linearized in a
// continous memory buffer for efficiency.

struct nfa_node {
	typedef unsigned char uchar_t;
	typedef std::string::size_type size_type;
	
	nfa_node(int e, nfa_node *n1 = 0, nfa_node *n2 = 0)
	:	edge(e), next1(n1), next2(n2), anchor(0) {
		++nfa_nodes_counter; 
	}
	~nfa_node() { --nfa_nodes_counter;}

	void set_transition(int e, nfa_node *n1 = 0, nfa_node *n2 = 0)
		{ edge = e; next1 = n1; next2 = n2;}

	bool is_epsilon_trans() { return edge == EPSILON;}
	bool is_important_trans() { return edge != EPSILON;}
	bool is_accept_node() { return edge == ACCEPT;}

	// Used for expr construction
	nfa_node *clone(std::set<nfa_node*>& clones);
	
	int edge;
	nfa_node *next1;
	nfa_node *next2;
	int anchor;
	nfa_node *myclone;
	
	// verifier
	static int nfa_nodes_counter;
};

// An nfa_expr is a directed graph of nfa nodes that represent a NFA.
// An nfa_expr object keeps a reference to the start and the accept NFA nodes.
// An nfa_expr object is the owner of the NFA nodes and on destruction must delete them.
// To avoid unecessary creation and destruction of nfa nodes most operations
// absorb their arguments or their rhs. 
// This has a cost; makes this class more difficult to use.
class nfa_expr {
  public:
  	typedef unsigned char uchar_t;
	typedef std::string::size_type size_type;

	nfa_expr() : accept(0), start(0) {}
	
	explicit nfa_expr(int edge) : accept(0), start(0) {
		accept = new nfa_node(ACCEPT);
		start = new nfa_node(edge, accept);
	}
	
	explicit nfa_expr(char edge) : accept(0), start(0) {
		accept = new nfa_node(ACCEPT);
		start = new nfa_node(uchar_t(edge), accept);
	}
	
	nfa_expr(const char *psz)
	:	start(0), accept(0) {
		cat_expr(psz);
	}
	
	~nfa_expr() { free(); }

	// Assignment constructor.
	// Warning: this is a shallow copy and consumes 'other' 
	// which becomes null.
	// It is implemented this way in order to be 
	// inexpensive to return an expr by value from a function.
	// Use other.clone() as argument to preserve 'other'.
	// Note that due to this the expr "nfa_expr a = b" makes 'a' 
	// the owner of the NFA and nullifies b
	nfa_expr(nfa_expr& other) 
	:	start(other.start), 
		accept(other.accept) {
		other.clear();
	}
	
	// Copy constructor (deep copy: expr remains const)
	const nfa_expr& operator=(const nfa_expr& expr) {
		if(this == &expr) return *this;
		free();
		nfa_expr e = expr.clone(); 
		start = e.start; 
		accept = e.accept;
		e.clear(); 
		return *this;
	}
	
	// Free the NFA nodes reachable by this
	void free();
	
	// nullify this without deleting nodes
	void clear() { start = accept = 0;}

	// deep copy
	nfa_expr clone() const;

	bool empty() const {return start == 0;}
	
	size_type size() const { 
		std::set<nfa_node*> nodes; 
		return get_expr_nodes(nodes).size();
	}
	
	size_type memsize() const { 
		return size()*sizeof(nfa_node);
	}

	//////////////////////////
	// Match
	
	// Matches string against the regex that this nfa_expr represents. 
	bool match(const std::string& str);
	bool match(const std::string& str, bool anchors);
	
	
	//////////////////////////
	// Regex operations
	
	// expr expr
	// This consumes expr which becomes null
	const nfa_expr& cat_expr(nfa_expr& expr);
	
	// expr expr
	const nfa_expr& cat_expr_clone(const nfa_expr& expr);

	// expr | expr
	// This consumes expr which becomes null
	const nfa_expr& or_expr(nfa_expr& expr);
	
	// expr | expr
	const nfa_expr& or_expr_clone(const nfa_expr& expr);
	
	// expr?
	const nfa_expr& optional() { 
		nfa_expr e(EPSILON);
		or_expr(e);
		assert(e.empty());
		return *this;
		}

	// expr*
	const nfa_expr& star();

	// expr+
	const nfa_expr& plus() {
		nfa_expr e(clone());
		e.star();
		cat_expr(e);
		assert(e.empty());
		return *this;
	}

	// expr{n}
	const nfa_expr& power(int n);

	// expr{n, m}
	const nfa_expr& repeat(int n, int m);

	
	//////////////////
	// Regex operations shortcuts 
		
	const nfa_expr& cat_expr(char ch)	{ 
		nfa_expr e(ch);
		cat_expr(e);
		assert(e.empty());
		return *this;
	}

	const nfa_expr& cat_expr(const char *psz);


	const nfa_expr& or_expr(char ch) {
		nfa_expr e(ch);
		or_expr(e);
		assert(e.empty());
		return *this;
	}
	
	const nfa_expr& or_expr(const char *psz) {
		nfa_expr e(psz);
		or_expr(e);
		assert(e.empty());
		return *this;
	}
	
	// (*this) | e and nullifies e
	const nfa_expr& operator+=(nfa_expr& e) {
		if(this == &e) cat_expr(e.clone());
		else cat_expr(e);
		return *this;
	}

	// Alt cat_expr
	const nfa_expr& operator+=(char ch) { return cat_expr(ch);}
	const nfa_expr& operator+=(const char *psz) { return cat_expr(psz);}
	
	// Alt or_expr
	const nfa_expr& operator*=(char ch) { return or_expr(ch);}
	const nfa_expr& operator*=(const char *psz) { return or_expr(psz);}

	// Returns the nodes of this expression
	std::set<nfa_node*>& get_expr_nodes(std::set<nfa_node*>& nodes) const;

	////////////////////
	// Services
	
	static nfa_expr create_char_class_expr(const std::string& s);
	static nfa_expr create_ws_expr();
	static nfa_expr create_time_units_expr();
	static nfa_expr create_sign_expr();
	
  private:
	
	// NFA search algorithm helpers
	void move(std::set<nfa_node*>& nodes, int edge, std::stack<nfa_node*>& ststack);
	void closure(std::stack<nfa_node*>& ststack, std::set<nfa_node*>& nodes);
	
	// invariants checks
	void verify1() const;
  	
	nfa_node *accept;
	nfa_node *start;
};

//////////////////////////
// Some handy expressions for testing
// The matcher will not be used this way for real cases.  
// The string to match will be the tokens of the scanner.  

// static
inline nfa_expr nfa_expr::create_ws_expr() {
	return nfa_expr::create_char_class_expr(" \t\r\n");
}

// static
inline nfa_expr nfa_expr::create_time_units_expr() {
	nfa_expr e('s');
	e.or_expr("ms");
	e.or_expr('h');
	e.or_expr("min");
	return e;
}

// static
inline nfa_expr nfa_expr::create_sign_expr() {
	return create_char_class_expr("+-");
}

} // namespace lib
 
} // namespace ambulant

// alt for cat_expr
ambulant::lib::nfa_expr  
operator+(const ambulant::lib::nfa_expr& e1, const ambulant::lib::nfa_expr& e2);

// alt for or_expr
ambulant::lib::nfa_expr  
operator*(const ambulant::lib::nfa_expr& e1, const ambulant::lib::nfa_expr& e2);

#endif // AMBULANT_LIB_STRING_UTIL_H
