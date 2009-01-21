/*
 * This file is part of Ambulant Player, www.ambulantplayer.org.
 *
 * Copyright (C) 2003-2008 Stichting CWI, 
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

#ifndef AMBULANT_LIB_TEXTPTR_H
#define AMBULANT_LIB_TEXTPTR_H

#include "ambulant/config/config.h"

#ifdef AMBULANT_PLATFORM_WIN32
	#ifndef _INC_WINDOWS
	#include <windows.h>
	#endif
#else 
	#include <stdio.h>
	#include <wchar.h>
#endif 

#ifndef text_str
	#ifdef UNICODE
	#define text_str(quote) L##quote
	#else
	#define text_str(quote) quote
	#endif
#endif

namespace ambulant {

namespace lib {

// This class is a two writing_modes converter 
// between UNICODE an MB strings.

class textptr {
  public:
	typedef wchar_t* wchar_ptr;
	typedef const wchar_t* const_wchar_ptr;
	
	typedef char* char_ptr;
	typedef const char* const_char_ptr;

	textptr(const char *pb) 
	:	m_pcb(pb), m_pcw(NULL), m_pb(NULL), m_pw(NULL), m_length(-1) {}

	textptr(const char *pb, size_t length) 
	:	m_pcb(pb), m_pcw(NULL), m_pb(NULL), m_pw(NULL), m_length(length) {}

	textptr(const wchar_t *pw) 
	:	m_pcb(NULL), m_pcw(pw), m_pb(NULL), m_pw(NULL), m_length(-1) {}

	textptr(const wchar_t *pw, size_t length) 
	:	m_pcb(NULL), m_pcw(pw), m_pb(NULL), m_pw(NULL), m_length(length) {}

	~textptr() {
		if(m_pw != NULL) delete[] m_pw;
		if(m_pb != NULL) delete[] m_pb;
	}
	
	wchar_ptr wstr() {
		if(m_pcw != NULL) return const_cast<wchar_ptr>(m_pcw);
		if(m_pw != NULL) return m_pw;
		if(m_pcb == NULL) return NULL;
		if (m_length < 0) m_length = strlen(m_pcb);
		int n = (int)m_length + 1;
		m_pw = new wchar_t[n];
		
#ifdef AMBULANT_PLATFORM_WIN32
		MultiByteToWideChar(CP_ACP, 0, m_pcb, n, m_pw, n);
#else
		mbstowcs(m_pw, m_pcb, n);
#endif
		return m_pw;
	}
	const_wchar_ptr c_wstr() { return wstr();}

	char_ptr str() {
		if(m_pcb != NULL) return const_cast<char_ptr>(m_pcb);
		if(m_pb != NULL) return m_pb;
		if(m_pcw == NULL) return NULL;
#ifdef AMBULANT_PLATFORM_WIN32
		//marisa added this code 11/11/08 
		//for double-byte characters the value of wcslen was not correct
		//the fix is to call WideCharToMultiByte twice -- once to get the length, once to convert
		BSTR unicodestr = 0;
		char *ansistr = 0;
		unicodestr = ::SysAllocString(m_pcw);
		int lenW = ::SysStringLen(unicodestr);
		int lenA = ::WideCharToMultiByte(CP_ACP, 0, unicodestr, lenW, 0, 0, NULL, NULL);
		if (lenA > 0){
			ansistr = new char[lenA + 1]; // allocate a final null terminator as well
			::WideCharToMultiByte(CP_ACP, 0, unicodestr, lenW, ansistr, lenA, NULL, NULL);
			ansistr[lenA] = 0; // Set the null terminator yourself
		}
		else {
			// handle the error
		}
		m_pb = ansistr;
		::SysFreeString(unicodestr);
#else
		if (m_length < 0) m_length = wcslen(m_pcw);
		int n = (int)m_length + 1;
		m_pb = new char[n];
		wcstombs(m_pb, m_pcw, n);

#endif
		return m_pb;
	}
	const_char_ptr c_str() { return str();}

	operator wchar_ptr() { return wstr();}
	operator const_wchar_ptr() { return c_wstr();}

	operator char_ptr() { return str();}
	operator const_char_ptr() { return c_str();}

	size_t length() {
		if(m_length>=0) return m_length;
		const_char_ptr pb = (m_pcb!=NULL)?m_pcb:m_pb;
		if(pb != NULL) 
			return (m_length = strlen(pb));
		const_wchar_ptr pw = (m_pcw!=NULL)?m_pcw:m_pw;
		if(pw != NULL) 
			return (m_length = wcslen(pw));
		return (m_length = 0);
	}

  private:
	const_char_ptr m_pcb;
	const_wchar_ptr m_pcw;
	char_ptr m_pb;
	wchar_ptr m_pw;
	ptrdiff_t m_length;
};


} // namespace lib

} // namespace ambulant

#endif // AMBULANT_LIB_TEXTPTR_H
