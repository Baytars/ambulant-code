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

#include "ambulant/lib/win32/win32_memfile.h"
#include "ambulant/lib/textptr.h"
#include "ambulant/lib/logger.h"

using namespace ambulant;

lib::win32::memfile::memfile(const std::basic_string<char>& url)
:	m_url(lib::textptr(url.c_str()) ), m_gptr(0) {
}

lib::win32::memfile::memfile(const std::basic_string<wchar_t>& url)
:	m_url(lib::textptr(url.c_str())), m_gptr(0) {
}
	
lib::win32::memfile::memfile(const text_char *url)
:	m_url(url?url:text_str("")), m_gptr(0) {
}
	
lib::win32::memfile::~memfile() {
}

// static 
bool lib::win32::memfile::exists(const std::basic_string<char>& url) {
	return iexists(lib::textptr(url.c_str()));
}

// static 
bool lib::win32::memfile::exists(const std::basic_string<wchar_t>& url) {
	return iexists(textptr(url.c_str()));
}

bool lib::win32::memfile::iexists(const text_char *url) {
	memfile mf(url);
	if(!mf.open()) return false;
	mf.close();
	return true;
}

bool lib::win32::memfile::open() {
	HANDLE hf = CreateFile(m_url.c_str(),  
		GENERIC_READ,  
		FILE_SHARE_READ,  // 0 = not shared or FILE_SHARE_READ  
		0,  // lpSecurityAttributes 
		OPEN_EXISTING,  
		FILE_ATTRIBUTE_READONLY,  
		NULL); 
	if(hf == INVALID_HANDLE_VALUE) {
		lib::logger::get_logger()->show("Failed to open file %s", textptr(m_url.c_str()));
		return false;
	}
	m_hf = hf;
	return true;
}

bool lib::win32::memfile::read() {
	if(!open()) {
		lib::logger::get_logger()->show("Failed to open file");
		return false;
	}
	const int buf_size = 1024;
	byte *buf = new byte[buf_size];
	DWORD nread = 0;
	while(ReadFile(m_hf, buf, buf_size, &nread, 0) && nread>0){
		m_buffer.append(buf, nread);
	}
	delete[] buf;
	m_gptr = 0;
	close();
	return true;
}

void lib::win32::memfile::close() {
	if(m_hf != INVALID_HANDLE_VALUE) 
		CloseHandle(m_hf);
}

void lib::win32::memfile::throw_range_error() {
#ifndef AMBULANT_PLATFORM_WIN32_WCE_3
	throw std::range_error("index out of range");
#else
	assert(false);
#endif
}

std::string lib::win32::memfile::repr() {
	std::string s("memfile[");
	//s += (!m_url.empty()?m_url:"NULL");
	s += "]";
	return s;
};
