// This file is part of Ambulant Player, www.ambulantplayer.org.
//
// Copyright (C) 2003-2007 Stichting CWI, 
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

#include "ambulant/net/databuffer.h"
#include "ambulant/lib/logger.h"

#ifndef AMBULANT_PLATFORM_WIN32
#include <unistd.h>
#endif

//#define AM_DBG
#ifndef AM_DBG
#define AM_DBG if(0)
#endif

// Define this to put random garbage into newly allocated buffer space
// and into buffer space that has been freed.
#undef RANDOM_BYTES
// ***********************************  C++  CODE  ***********************************

// data_buffer

#ifdef WITH_SMALL_BUFFERS
#define DEFAULT_MAX_BUF_SIZE 100000
#else
#define DEFAULT_MAX_BUF_SIZE 1000000
#endif
long int ambulant::net::databuffer::s_default_max_size = DEFAULT_MAX_BUF_SIZE;
long int ambulant::net::databuffer::s_default_max_unused_size = DEFAULT_MAX_BUF_SIZE;

using namespace ambulant;
using namespace net;

// Static methods:
void
databuffer::default_max_size(int max_size)
{
	s_default_max_size = max_size;
}

void
databuffer::default_max_unused_size(int max_unused_size)
{
	s_default_max_unused_size = max_unused_size;
}

databuffer::databuffer(int max_size)
:	m_buffer(NULL),
	m_reading(false),
	m_writing(false),
	m_old_buffer(NULL),
	m_rear(0),
	m_size(0),
	m_max_size(max_size),
	m_max_unused_size(s_default_max_unused_size),
	m_used(0),
	m_buffer_full(false)
{
	AM_DBG lib::logger::get_logger()->debug("databuffer::databuffer() -> 0x%x", (void*)this);
	if (m_max_size == 0) m_max_size = s_default_max_size;
}

bool
databuffer::buffer_full()
{
	m_lock.enter();
	bool rv = m_buffer_full;
	m_lock.leave();
	return rv;
}


bool
databuffer::buffer_not_empty()
{
	m_lock.enter();
	bool rv = (m_used > 0);
	m_lock.leave();
	return rv;
}

void
databuffer::set_max_size(int max_size)
{
	m_lock.enter();
	// Zero means: no limit, <0 means: default
    if (max_size >= 0) {
        m_max_size = max_size;
    } else {
        m_max_size = s_default_max_size;
    }
    m_buffer_full = (m_max_size > 0 && m_used > m_max_size);
	if (m_buffer_full) lib::logger::get_logger()->debug("databuffer::set_max_size(0x%x, %d): buffer now full (used=%d)",
		(void*)this, max_size, m_used);
	m_lock.leave();
}

databuffer::~databuffer()
{
	m_lock.enter();
	AM_DBG lib::logger::get_logger()->debug("databuffer::~databuffer(0x%x)", (void*)this);
	assert(!m_reading);
	assert(!m_writing);
	if (m_buffer) {
		free(m_buffer);
        m_buffer = NULL;
	}
	if (m_old_buffer) {
		free(m_old_buffer);
		m_old_buffer = NULL;
	}
	m_lock.leave();
}

int databuffer::size() const
{
	const_cast<databuffer*>(this)->m_lock.enter();
//XXXX	assert(!m_reading);
	int rv = m_used;
	assert(rv < 10000000); // TMP sanity check
	const_cast<databuffer*>(this)->m_lock.leave();
	return rv;
}

#ifndef AMBULANT_NO_IOSTREAMS_HEADERS
void databuffer::dump(std::ostream& os, bool verbose) const
{
	unsigned long int i;

	os << "BUFFER SIZE : " << m_size << " bytes" << std::endl;
	os << "BYTES USED : " << m_used << " bytes" << std::endl;
	os << "m_rear   : " << m_rear << std::endl;
	if (verbose) {
		if (m_buffer) {
			for (i = m_rear;i < m_size;i++) {
	   		os << m_buffer[i];
	   		}
		}
	} 
 	os << std::endl;
}
#endif

char *
databuffer::get_write_ptr(int sz)
{
	m_lock.enter();
	assert(!m_writing);
	assert(sz > 0);
	m_writing = true;
	
	char *rv = NULL;
	AM_DBG lib::logger::get_logger()->debug("databuffer(0x%x).get_write_ptr(%d): start ", (void*)this, sz);
	
    if(!m_buffer_full) {
			//AM_DBG lib::logger::get_logger()->debug("databuffer.get_write_ptr: returning m_front (%x)",m_buffer + m_size);
			_grow(sz);
			rv = m_buffer + m_size;
		
    } else {
        lib::logger::get_logger()->trace("databuffer::databuffer::get_write_ptr : buffer full but still trying to obtain write pointer ");
		rv = NULL;
    }
    m_lock.leave();
	return rv;
}

void
databuffer::_grow(int sz)
{
	assert(sz > 0);
	if (m_reading && m_old_buffer == NULL) {
		// We cannot realloc, the buffer is in use. Copy.
		m_old_buffer = m_buffer;
		m_buffer = (char*)malloc(m_size + sz);
		if (m_buffer) memcpy(m_buffer, m_old_buffer, m_size);
		AM_DBG lib::logger::get_logger()->debug("databuffer::get_write_ptr: buffer alloc/copy to from %d to %d bytes",m_size, m_size + sz);
	} else {
		m_buffer = (char*) realloc(m_buffer, m_size + sz);
		AM_DBG lib::logger::get_logger()->debug("databuffer::get_write_ptr: buffer realloc to from %d to %d bytes",m_size, m_size + sz);
	}
	AM_DBG lib::logger::get_logger()->debug("databuffer::get_write_ptr: buffer realloc done (%x)",m_buffer);
	if (!m_buffer) {
		lib::logger::get_logger()->fatal("databuffer::get_write_ptr(size=%d): out of memory", m_size+sz);
	}
#ifdef RANDOM_BYTES
	unsigned int i;
	for(i=m_size; i<m_size+sz; i++) m_buffer[i] = (char) rand();
#endif
}

void databuffer::pushdata(int sz)
{
	m_lock.enter();
	assert(m_writing);
	m_writing = false;
	AM_DBG lib::logger::get_logger()->debug("databuffer(0x%x)::pushdata(%d) m_size=%d", (void*)this, sz, m_size);
	if (m_buffer_full) {
        lib::logger::get_logger()->trace("databuffer::databuffer::pushdata : buffer full but still trying to fill it");
    }
	// std::cout << sz << "\n";
	assert(sz >= 0);
	assert(m_size >= 0);
	
	if (!m_reading) {
		// If we have a read pointer outstanding we simply not realloc. It will
		// happen the next time.
		m_buffer = (char*) realloc(m_buffer, m_size + sz);
	}
	AM_DBG lib::logger::get_logger()->debug("databuffer(0x%x)::pushdata(%d) realloc m_buffer=x%x, from %d bytes to %d bytes", (void*)this, sz, (void*) m_buffer, m_size, m_size + sz);

	m_size += sz;
	//AM_DBG lib::logger::get_logger()->debug("databuffer.pushdata:size = %d ",sz);
	assert(m_size >= m_rear);
	m_used = m_size - m_rear;
	 if (!m_buffer && (sz > 0)) {
		 lib::logger::get_logger()->fatal("databuffer::pushdata(size=%d): out of memory", m_size);
		 abort();
	 }
	if(m_max_size > 0 && m_used > m_max_size) {
		AM_DBG lib::logger::get_logger()->debug("databuffer.pushdata: buffer full [size = %d, max size = %d]",m_size, m_max_size);
		m_buffer_full = true;
	}
	m_lock.leave();
}


char *
databuffer::get_read_ptr()
{
	m_lock.enter();
	char *rv = (m_buffer + m_rear);
	AM_DBG lib::logger::get_logger()->debug("databuffer(0x%x)::get_read_ptr(): returning 0x%x (m_size = %d)", (void*)this, (void*)rv, m_size);

	assert(!m_reading);
	m_reading = true;
	m_lock.leave();
	return rv;
	
}


void
databuffer::readdone(int sz)
{
	m_lock.enter();

	assert(m_reading); // if this fails we have a readdone() without a prior get_read_ptr().
	m_reading = false;
	AM_DBG lib::logger::get_logger()->debug("databuffer(0x%x)::readdone(%d)", (void*)this, sz);
	assert((size_t)sz <= m_used);
#ifdef RANDOM_BYTES
	unsigned int i;
	for(i=m_rear; i<m_rear+sz; i++) m_buffer[i] = (char) rand();
#endif
	m_rear += sz;
	assert( m_size >= m_rear);
	m_used = m_size - m_rear;
	m_buffer_full = (m_max_size > 0 && m_used > m_max_size);
	
	// If the writer needed more space while m_reading was true it will
	// have left the old buffer for us to remove.
	if (m_old_buffer) {
		AM_DBG lib::logger::get_logger()->debug("databuffer(0x%x)::readdone: free old buffer 0x%x", this, m_old_buffer);
		free(m_old_buffer);
		m_old_buffer = NULL;
	}
	
	// Free the unused space in the buffer if the buffer is either empty
	// or underused. Skip this if there is a write outstanding, then it'll
	// happen the next time around.
	if (!m_writing && (m_used == 0 || (m_max_unused_size > 0 && m_rear > m_max_unused_size))) {
		 AM_DBG lib::logger::get_logger()->debug("databuffer(0x%x)::readdone(%d) resizing buffer (cur. size = %d)", (void*)this, sz, m_size);
		 if (m_used) memcpy(m_buffer, m_buffer+m_rear, m_used);
		 m_buffer = (char *)realloc(m_buffer, m_used);
		 m_size = m_used;
		 m_rear = 0;
		 AM_DBG lib::logger::get_logger()->debug("databuffer(0x%x)::readdone(%d) (m_buffer=x%x) resized to %d",  (void*)this, (void*)m_buffer, m_size);
		 if (m_buffer == NULL && m_used > 0) {
			 lib::logger::get_logger()->fatal("databuffer::readdone(size=%d): out of memory", m_size);
		 }
	}
	m_lock.leave();
}
