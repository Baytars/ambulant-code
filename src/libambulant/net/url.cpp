// This file is part of Ambulant Player, www.ambulantplayer.org.
//
// Copyright (C) 2003-2005 Stichting CWI, 
// Kruislaan 413, 1098 SJ Amsterdam, The Netherlands.
//
// Ambulant Player is free software; you can redistribute it and/or modify
// it under the terms of the GNU Lesser General Public License as published by
// the Free Software Foundation; either version 2.1 of the License, or
// (at your option) any later version.
//
// Ambulant Player is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU Lesser General Public License for more details.
//
// You should have received a copy of the GNU Lesser General Public License
// along with Ambulant Player; if not, write to the Free Software
// Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA  02110-1301  USA

/* 
 * @$Id$ 
 */

#ifndef AM_DBG
#define AM_DBG if(0)
#endif

#include "ambulant/net/url.h"
#include "ambulant/lib/logger.h"
#include "ambulant/lib/string_util.h"
#include "ambulant/lib/filesys.h"
 
#include <string>
#if !defined(AMBULANT_NO_IOSTREAMS) && !defined(AMBULANT_NO_STRINGSTREAM)
#include <sstream>
#endif


using namespace ambulant;

#ifndef AMBULANT_PLATFORM_WIN32_WCE
const std::string url_delim = ":/?#,";
#else
const std::string url_delim = ":/?#\\,";
#endif

// static 
//std::list< std::pair<std::string, net::url::HANDLER> > net::url::s_handlers;
// workaround for g++ 2.95
std::list< net::url_handler_pair* > s_handlers;

// static
void net::url::init_statics() {

	// workaround for g++ 2.95
	static url_handler_pair h1 = {"n://n:d/", &url::set_from_host_port_uri};
 	s_handlers.push_back(&h1);
 	
	static url_handler_pair h1a = {"n://n:d", &url::set_from_host_port_uri};
 	s_handlers.push_back(&h1a);
 	
	static url_handler_pair h1b = {"n://dn:d/", &url::set_from_numhost_port_uri};
 	s_handlers.push_back(&h1b);
 	
	static url_handler_pair h1c = {"n://dn:d", &url::set_from_numhost_port_uri};
 	s_handlers.push_back(&h1c);
 	
	static url_handler_pair h2 = {"n://n/", &url::set_from_host_uri};
 	s_handlers.push_back(&h2);
 	
	static url_handler_pair h2a = {"n://n", &url::set_from_host_uri};
 	s_handlers.push_back(&h2a);
 	
	static url_handler_pair h2b= {"n://dn/", &url::set_from_numhost_uri};
 	s_handlers.push_back(&h2b);
 	
	static url_handler_pair h2c = {"n://dn", &url::set_from_numhost_uri};
 	s_handlers.push_back(&h2c);
 	
	static url_handler_pair h3 = { "n:///", &url::set_from_localhost_file_uri};
 	s_handlers.push_back(&h3);
 	
	static url_handler_pair h4 = { "n:///", &url::set_from_localhost_file_uri};
 	s_handlers.push_back(&h4);

	static url_handler_pair h4a = { "n:,", &url::set_from_data_uri};
 	s_handlers.push_back(&h4a);

	static url_handler_pair h5 = {"/n", &url::set_from_absolute_path};
 	s_handlers.push_back(&h5);
	
	static url_handler_pair h9 = {"", &url::set_from_relative_path};
 	s_handlers.push_back(&h9);
	
	/*
	typedef std::pair<std::string, HANDLER> pair;
 	s_handlers.push_back(pair("n://n:n/",&url::set_from_host_port_uri));
  	s_handlers.push_back(pair("n://n/",&url::set_from_host_uri));
  	s_handlers.push_back(pair("n:///",&url::set_from_localhost_file_uri));
   	s_handlers.push_back(pair("/n",&url::set_from_unix_path));
 	s_handlers.push_back(pair("n:n",&url::set_from_windows_path));
  	s_handlers.push_back(pair("n:/n",&url::set_from_windows_path));
  	*/
 }
 
// static
net::url 
net::url::from_filename(const std::string& spec)
{
	lib::logger::get_logger()->debug("url::from_filename not implemented yet");
	return net::url();
}

net::url::url() 
:	m_absolute(true),
	m_port(0)
{
}
 
net::url::url(const string& spec) 
:	m_port(0)
{
	set_from_spec(spec);
}
	 
net::url::url(const string& protocol, const string& host, 
	const string& path) 
:	m_protocol(protocol),
	m_host(host),
	m_port(0),
	m_path(path)
{
	m_absolute = (m_protocol != "");
}

net::url::url(const string& protocol, const string& host, int port, 
	const string& path) 
:	m_protocol(protocol),
	m_host(host),
	m_port(short_type(port)),
	m_path(path)
{
	m_absolute = (m_protocol != "");
}

net::url::url(const string& protocol, const string& host, int port, 
	const string& path, const string& query, const string& ref) 
:	m_protocol(protocol),
	m_host(host),
	m_port(short_type(port)),
	m_path(path), 
	m_query(query), 
	m_ref(ref)
{
	m_absolute = (m_protocol != "");
}
 
net::url::string net::url::get_file() const {
	std::string file = get_path();
#ifdef AMBULANT_PLATFORM_WIN32
	// Sigh, this mix-n-match of filenames and URLs is really messing
	// us up. If this is a file URL we may need to take off the first
	// slash, but not always...
	if (is_local_file() && file[0] == '/') file = file.substr(1);
#endif
	if(!m_query.empty()) {
		file += '?';
		file += m_query;
	}
	return file;
}

void net::url::set_from_spec(const string& spec) {
	lib::scanner sc(spec, url_delim);
	sc.tokenize();
	std::string sig = sc.get_tokens();
	//std::list< std::pair<std::string, HANDLER> >::iterator it;
	std::list<url_handler_pair*>::iterator it;
	for(it=s_handlers.begin();it!=s_handlers.end();it++) {
		url_handler_pair *ph = (*it);
		if(*(ph->first) == '\0' || lib::starts_with(sig, ph->first)) {
			//HANDLER h = (*it).second;
			(this->*(ph->second))(sc, ph->first);
			return;
		}
	}
	lib::logger::get_logger()->error(gettext("%s: Cannot parse URL"), spec.c_str());
}

// pat: "n://n:d/"
void net::url::set_from_host_port_uri(lib::scanner& sc, const std::string& pat) {
	m_absolute = true;
	m_protocol = sc.val_at(0);
	m_host = sc.val_at(4);
	m_port = short_type(atoi(sc.val_at(6).c_str()));
	set_parts(sc, pat);
}
	
// pat: "n://dn:d/"
void net::url::set_from_numhost_port_uri(lib::scanner& sc, const std::string& pat) {
	m_absolute = true;
	m_protocol = sc.val_at(0);
	m_host = sc.join(4, 6);
	m_port = short_type(atoi(sc.val_at(7).c_str()));
	set_parts(sc, pat);
}
	
// pat: "n://n/"
void net::url::set_from_host_uri(lib::scanner& sc, const std::string& pat) {
	m_absolute = true;
	m_protocol = sc.val_at(0);
	m_host = sc.val_at(4);
	set_parts(sc, pat);
	if(m_protocol == "http")
		m_port = 80;
	else if(m_protocol == "ftp")
		m_port = 21;
}

// pat: "n://dn/"
void net::url::set_from_numhost_uri(lib::scanner& sc, const std::string& pat) {
	m_absolute = true;
	m_protocol = sc.val_at(0);
	m_host = sc.join(4, 6);
	set_parts(sc, pat);
	if(m_protocol == "http")
		m_port = 80;
	else if(m_protocol == "ftp")
		m_port = 21;
}

// pat: "n:///" for file:///
void net::url::set_from_localhost_file_uri(lib::scanner& sc, const std::string& pat) {
	m_absolute = true;
	m_protocol = sc.val_at(0);
	m_host = "localhost";
	m_port = 0;
	set_parts(sc, pat);
	// The initial / in the pathname has been eaten
}

// pat: "/n"
void net::url::set_from_absolute_path(lib::scanner& sc, const std::string& pat) {
	m_absolute = true;
	m_protocol = "file";
	m_host = "localhost";
	m_port = 0;
	m_path = sc.get_src();
}

void net::url::set_from_relative_path(lib::scanner& sc, const std::string& pat) {
	m_absolute = false;
	m_protocol = "";
	m_host = "";
	m_port = 0;
	set_parts(sc, pat);
	AM_DBG lib::logger::get_logger()->debug("url::set_from_relative_path: \"%s\" -> \"%s\"", pat.c_str(), m_path.c_str());
}

// pat: "data:,"
void net::url::set_from_data_uri(lib::scanner& sc, const std::string& pat) {
	m_absolute = true;
	m_protocol = "data";
	m_host = "";
	m_port = 0;
	m_path = sc.join(3);  // Skip data:,
}

void net::url::set_parts(lib::scanner& sc, const std::string& pat) {
	const std::string& toks = sc.get_tokens();
	size_type n = toks.length();
	size_type i1 = pat.length();
	// Most patterns include the initial / of the pathname, but we want it in
	// the pathname.
	if (i1 > 0 && pat[i1-1] == '/') i1--;
	size_type i2 = toks.find_last_of('?');
	size_type i3 = toks.find_last_of('#');
	i2 = (i2 == std::string::npos)?n:i2;
	i3 = (i3 == std::string::npos)?n:i3;
	size_type i4 = i2<i3?i2:i3;
	m_path = sc.join(i1, i4);
	m_query = sc.join(i2+1, i3);
	m_ref = sc.join(i3+1);
}

bool net::url::is_local_file() const
{
	if (m_protocol == "file" && (m_host == "localhost" || m_host == ""))
		return true;
	if (!m_absolute && m_protocol == "") {
		// We're not sure.
		AM_DBG lib::logger::get_logger()->trace("url::is_local_file: assume True for relative url: \"%s\"", repr(*this).c_str());
		return true;
	}
	return false;
}
	
std::string net::url::get_url() const
{
	std::string rv = repr(*this);
	if (!m_absolute)
		lib::logger::get_logger()->trace("url::get_url(): URL not absolute: \"%s\"", rv.c_str());
	return rv;
}

net::url net::url::join_to_base(const net::url &base) const
{
	// Note: this hasn't been checked against RFCxxxx. We pick up protocol, host, port
	// and initial pathname from base. Alll other items from base are ignored.
	if (m_absolute) return *this;
	std::string basepath = base.get_path();
	std::string newpath = get_path();
	if (newpath == "") {
		// New path is, for instance, only #anchor.
		newpath = basepath; 
	} else if (newpath[0] != '/') {
		// New_path is not absolute. Prepend base of basepath
		basepath = lib::filesys::get_base(basepath);
		// Convert basepath from Windows to URL, if needed.
		// XXXX Incomplete?
//		if (base.m_absolute && basepath[0] != '/')
//			basepath = "/" + basepath;
		std::string::iterator cp;
		for (cp=basepath.begin(); cp != basepath.end(); cp++) {
			char c = *cp;
			if (c == '\\') *cp = '/';
		}
		newpath = lib::filesys::join(basepath, newpath);
		// Now ad
		//newpath = lib::filesys::join(basepath, newpath, "/");
	}
	AM_DBG lib::logger::get_logger()->debug("url::join_to_base: old \"%s\" base \"%s\" newpath \"%s\"",
		repr(*this).c_str(), repr(base).c_str(), newpath.c_str());
	net::url rv = net::url(
		base.get_protocol(),
		base.get_host(),
		base.get_port(),
		newpath,
		m_query,
		m_ref);
	if (base.m_absolute)
		rv.m_absolute = true;
	return rv;
}

bool net::url::same_document(const net::url &base) const
{
	return (m_protocol == base.m_protocol &&
		m_host == base.m_host &&
		m_port == base.m_port &&
		m_path == base.m_path &&
		m_query == base.m_query);
}

#if defined(AMBULANT_PLATFORM_WIN32_WCE)
void set_url_from_spec(net::url& u, const char *spec) {
	net::url dummy(spec);
	u = dummy;
}

#endif

// Very limited guesstype (for now), only guesses some HTML documents.
std::string
net::url::guesstype() const
{
	size_t dotpos = m_path.find_last_of(".");
	if (dotpos <= 0) return "";
	std::string ext = m_path.substr(dotpos);
	
	if (ext == ".htm" || ext == ".HTM" || ext == ".html" || ext == ".HTML")
		return "text/html";
	return "";
}

#if defined(AMBULANT_PLATFORM_UNIX)

// Places where to look for (cached) datafiles
const char *datafile_locations[] = {
	".",		// Placeholder, to be replaced by set_datafile_directory()
	".",
	"..",
	"Extras",
	"../Extras",
#ifdef	AMBULANT_DATADIR
	AMBULANT_DATADIR ,
#else
	"/usr/local/share/ambulant",
#endif
	NULL
};

std::string datafile_directory;

void
net::url::set_datafile_directory(std::string pathname)
{
	datafile_directory = pathname;
	datafile_locations[0] = datafile_directory.c_str();
}

std::pair<bool, net::url>
net::url::get_local_datafile() const
{
	const char* result = NULL;
	if (!is_local_file()) return std::pair<bool, net::url>(false, net::url(*this));
	
	if (! is_absolute()) {
		string rel_path = get_path();
		const char **dir;
		for(dir = datafile_locations; *dir; dir++) {
			string abs_path(*dir);
			abs_path += "/" + rel_path;
			if (access(abs_path.c_str(), 0) >= 0) {
			  	result = abs_path.c_str();
				break;
			}
		}
	} else if (is_local_file() 
		   && access (get_file().c_str(), 0) >= 0) {
		result = get_file().c_str();
	}
	
	if (!result) return std::pair<bool, net::url>(false, net::url(*this));
	
	return std::pair<bool, net::url>(true, net::url("file", "", result));
}
#else // AMBULANT_PLATFORM_UNIX

// Hack: if it is not Unix it must be windows:-)

// Places where to look for (cached) datafiles
static const char *datafile_locations[] = {
	"Extras\\",
	"..\\..\\Extras\\",
	"",
	"..\\",
	"..\\Extras\\",
	NULL
};

static std::string datafile_directory;

void
net::url::set_datafile_directory(std::string pathname)
{
	datafile_directory = pathname;
}

std::pair<bool, net::url>
net::url::get_local_datafile() const
{
	// XXXX Needs work!
#ifdef AMBULANT_PLATFORM_WIN32_WCE
	// Too lazy to convert char to wide char right now
	return std::pair<bool, net::url>(false, net::url(*this));
#else
	if (datafile_directory == "") {
		set_datafile_directory(lib::win32::get_module_dir());
	}
	const char* result = NULL;
	string path;
	if (!is_local_file()) return std::pair<bool, net::url>(false, net::url(*this));
	
	if (! is_absolute()) {
		string rel_path = get_path();
		const char **dir;
		for(dir = datafile_locations; *dir; dir++) {
			path = datafile_directory + *dir;
			path += rel_path;
			if (lib::win32::file_exists(path)) {
			  	result = path.c_str();
				break;
			}
		}
	} else if (is_local_file()) {
		path = get_file();
		if (lib::win32::file_exists(path))
			result = path.c_str();
	}
	
	if (!result) return std::pair<bool, net::url>(false, net::url(*this));
	std::string *pathname = new std::string(result);
	return std::pair<bool, net::url>(true, net::url("file", "", *pathname));
#endif
}
#endif //AMBULANT_PLATFORM_UNIX
///////////////
// module private static initializer

class url_static_init {
  public:
	url_static_init() {
		net::url::init_statics();
	}
};

static url_static_init url_static_init_inst;
