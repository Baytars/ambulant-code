
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



#include "ambulant/net/datasource.h"


using namespace ambulant;
using namespace net;

// *********************** datasource_factory ***********************************************
  

void
global_datasource_factory::add_finder(datasource_finder *df)
{
	m_finders.push_back(df);
}

void
global_datasource_factory::add_audio_finder(audio_datasource_finder *df)
{
	m_audio_finders.push_back(df);
}

void
global_datasource_factory::add_audio_parser_finder(audio_parser_finder *df)
{
	m_audio_parser_finders.push_back(df);
}

void
global_datasource_factory::add_audio_filter_finder(audio_filter_finder *df)
{
	m_audio_filter_finders.push_back(df);
}


datasource*
global_datasource_factory::new_datasource(const std::string &url)
{
    std::vector<datasource_finder *>::iterator i;
    datasource *src;
    
    for(i=m_finders.begin(); i != m_finders.end(); i++) {
        src = (*i)->new_datasource(url);
        if (src) return src;
    }
    return NULL;
}

audio_datasource*
global_datasource_factory::new_audio_datasource(const std::string &url, audio_format_choices fmts)
{
    audio_datasource *src = NULL;

	// First try to see if anything supports the whole chain
    std::vector<audio_datasource_finder *>::iterator i;
    for(i=m_audio_finders.begin(); i != m_audio_finders.end(); i++) {
        src = (*i)->new_audio_datasource(url, fmts);
        if (src) return src;
    }

	// If that didn't work we try to first create a raw datasource, and
	// then stack a parser and possibly a filter
	datasource *rawsrc = new_datasource(url);
	if (rawsrc == NULL) return NULL;
	
	std::vector<audio_parser_finder*>::iterator ip;
	for(ip=m_audio_parser_finders.begin(); ip != m_audio_parser_finders.end(); ip++) {
		src = (*ip)->new_audio_parser(url, fmts, rawsrc);
		if (src) break;
	}
	if (src == NULL) {
		rawsrc->release();
		return NULL;
	}
	
	// Now stack a filter. Note that the first filter finder is the identity
	// filter.
	std::vector<audio_filter_finder*>::iterator ic;
	audio_datasource *convsrc = NULL;
	for(ic=m_audio_filter_finders.begin(); ic != m_audio_filter_finders.end(); ic++) {
		convsrc = (*ic)->new_audio_filter(src, fmts);
		if (convsrc) return convsrc;
	}
	
	// Failed to find a filter. Clean up.
	src->release(); // This will also release rawsrc
    return NULL;
}
