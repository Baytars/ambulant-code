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

#include "ambulant/net/datasource.h"
#include <unistd.h>

#ifndef AM_DBG
#define AM_DBG if(0)
#endif

#define DEFAULT_MAX_BUF_SIZE 4096

// ***********************************  C++  CODE  ***********************************

// data_buffer

using namespace ambulant;

	
net::databuffer::databuffer()
{
	m_used = 0;
    m_size = 0;
    m_rear = NULL;
    m_max_size = DEFAULT_MAX_BUF_SIZE;
	m_buffer = NULL;
    m_buffer_full = false;
}

bool
net::databuffer::is_full()
{
   return m_buffer_full;
}


bool
net::databuffer::not_empty()
{
    if (m_used > 0) {
        return true;
    } else {
        return false;
    }
}
            


net::databuffer::databuffer(int max_size)
{
    m_used = 0;
    m_size = 0;
    m_rear = NULL;
    m_front = NULL;
    if (max_size > 0) {
        m_max_size = max_size;
    } else {
        m_max_size = DEFAULT_MAX_BUF_SIZE;
    }
    m_buffer_full = false;
}


net::databuffer::~databuffer()
{
	if (m_buffer) {
		free(m_buffer);
		m_used = 0;
		m_size = 0;
                m_rear = NULL;
                m_front = NULL;
		m_buffer = NULL;
	}
}

int net::databuffer::used() const
{
	return m_used;
}

void net::databuffer::dump(std::ostream& os, bool verbose) const
{
int i;

os << "BUFFER SIZE : " << m_size << " bytes" << std::endl;
os << "BYTES USED : " << m_used << " bytes" << std::endl;
if (verbose) {
	if (m_buffer) {
		for (i = 0;i < m_used;i++) {
	   		os << m_buffer[i];
	   		}
	   	}
	} 
 os << std::endl;
}



void net::databuffer::prepare(char* in_ptr)
{
    if(!m_buffer_full) {
        in_ptr = m_front;
        m_buffer = (char*) realloc(m_buffer, m_size + BUFSIZ);
        if (!m_buffer) {
            lib::logger::get_logger()->fatal("databuffer::databuffer(size=%d): out of memory", m_size);
        }
    } else {
        lib::logger::get_logger()->warn("databuffer::databuffer::prepare : buffer full but still trying to fill it ");
    }
    
}

void net::databuffer::pushdata(int size)
{
    if(!m_buffer_full) {
        m_size  += size;
        m_front += size;
        m_used = m_front -m_rear;
        m_buffer = (char*) realloc(m_buffer, m_size);
         if (!m_buffer) {
             lib::logger::get_logger()->fatal("databuffer::databuffer(size=%d): out of memory", m_size);
         }
        if(m_size > m_max_size) {
            m_buffer_full = true;
        }
    } else {
        lib::logger::get_logger()->warn("databuffer::databuffer::pushdata : buffer full but still trying to fill it");
    }
}


char *
net::databuffer::get_read_ptr()
{
return m_rear;
}

void
net::databuffer::readdone(int size)
{
    if (size <= m_used) {
        m_rear += size;
        m_used = m_front - m_rear;
    }
}



// *********************** passive_datasource ***********************************************



net::active_datasource* net::passive_datasource::activate()
{
	int in;
	
	in = open(m_url.c_str(), O_RDONLY);
	if (in >= 0) {
		return new active_datasource(this, in);
	} else {
		lib::logger::get_logger()->error("passive_datasource.activate(url=\"%s\"): %s",  m_url.c_str(), strerror(errno));
	}
	return NULL;
		
}

net::passive_datasource::~passive_datasource()
{
}

// *********************** active_datasource ***********************************************


net::active_datasource::active_datasource(passive_datasource *const source, int file)
:	m_source(source)
{
	if (file >= 0) {
		m_stream = file;
		filesize();
		m_source = source;
		m_buffer = new databuffer(m_filesize);
		if (!m_buffer) {
			m_buffer = NULL;
 			lib::logger::get_logger()->fatal("active_datasource(): out of memory");
		}
		m_source->add_ref();
		AM_DBG m_buffer->dump(std::cout, false);
	}
}

net::active_datasource::~active_datasource()
{
	if (m_buffer) {
		delete m_buffer;
		m_buffer = NULL;
	}
	m_source->release();
	close(m_stream);
}

int
net::active_datasource::size() const
{
	return m_buffer->used();
}

void
net::active_datasource::filesize()
{
 		using namespace std;
		int dummy;
		if (m_stream >= 0) {
			// Seek to the end of the file, and get the filesize
			m_filesize=lseek(m_stream, 0, SEEK_END); 		
	 		dummy=lseek(m_stream, 0, SEEK_SET);						
			} else {
 			lib::logger::get_logger()->fatal("active_datasource.filesize(): no file openXX");
			m_filesize = 0;
			}
}


void
net::active_datasource::read(char *data, int size)
{
    char* in_ptr;
    if (size <= m_buffer->used()) {
            in_ptr = m_buffer->get_read_ptr();
            memcpy(in_ptr,data,size);
            m_buffer->readdone(size);
    }
}

void
net::active_datasource::read_file()
{
  	char *buf;
  	int n; 	
	if (m_stream >= 0) {

		do {
            	m_buffer->prepare(buf);
				n = ::read(m_stream, buf, BUFSIZ);
				if (n >= 0) m_buffer->pushdata(n); 
		} while (n > 0);

		if (n < 0) {
			lib::logger::get_logger()->error("active_datasource.read_file(): %s", strerror(errno));
			} 		
	}
}
 
char* 
net::active_datasource::read_ptr()
{
	return m_buffer->get_read_ptr();
}
  
void
net::active_datasource::start(ambulant::lib::event_processor *evp, ambulant::lib::event *callback)
 {
 	read_file();
        	if (evp && callback) {
				AM_DBG lib::logger::get_logger()->trace("active_datasource.start: trigger readdone callback");
				evp->add_event(callback, 0, ambulant::lib::event_processor::high);
        	}
}
 
void
net::active_datasource::readdone(int size)
{
	m_buffer->readdone(size);
}

