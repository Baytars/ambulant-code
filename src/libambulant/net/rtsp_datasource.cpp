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

#include "rtsp_datasource.h"
#include "ambulant/lib/logger.h"

ambulant::net::rtsp_demux::rtsp_demux(net::url& url) 
:	m_rtsp_client(NULL),
	m_sip_client(NULL),
	m_media_session(NULL),
	m_sdp(NULL),
	m_nstream(0)
{
	
	// setup the basics.
	TaskScheduler* scheduler = BasicTaskSchedular::createNew();
	if (!scheduler) {
		lib::logger::get_logger()->debug("ambulant::net::rtsp_demux(net::url& url) failed to create scheduler");
	    lib::logger::get_logger()->error("RTSP Connection Failed");		
		return;
	}
	
	UsageEnvironment* env = BasicUsageEnvironment::createNew(*scheduler);
	if (!env) {
		lib::logger::get_logger()->debug("ambulant::net::rtsp_demux(net::url& url) failed to create UsageEnvironment");
		lib::logger::get_logger()->error("RTSP Connection Failed");		
		return;
	}
	// setup a rtp session
	m_rtsp_client = RTSPClient::createNew(*env, verbose, "AmbulantPlayer");
	if (!m_rtsp_client) {
		lib::logger::get_logger()->debug("ambulant::net::rtsp_demux(net::url& url) failed to create  a RTSP Client");
		lib::logger::get_logger()->error("RTSP Connection Failed");		
		return;
	}
	
	char* ch_url = url.get_url().cstr();
	if (ch_url) {
		m_dsp = m_rtsp_client(ch_url);
		if (!m_dsp) {
			lib::logger::get_logger()->debug("ambulant::net::rtsp_demux(net::url& url) failed to get dsp description from rtsp server");
			lib::logger::get_logger()->error("RTSP Connection Failed");		
			return;
		}
	} else {
		lib::logger::get_logger()->debug("ambulant::net::rtsp_demux(net::url& url) failed to get url");
		lib::logger::get_logger()->error("Wrong URL !");		
		return;
	}
	
	m_media_session = MediaSession::CreateNew(*env, sdp_description);
	if (!m_media_session) {
		lib::logger::get_logger()->debug("ambulant::net::rtsp_demux(net::url& url) failed to create  a MediaSession");
		lib::logger::get_logger()->error("RTSP Connection Failed");		
		return;
	}	
	
	// next set up the rtp subsessions.
	
	unsigned int desired_buf_size;
	MediaSubsession* subsession;
	MediaSubsessionIterator iter(*m_media_session);
	
	// Only audio/video session need to apply for a job !
	while ((subsession = iter.next()) != NULL) {
		if (strcmp(subsession->mediumName(), "audio") == 0) {
			desired_buf_size = 100000;
		} else if (strcmp(subsession->mediumName(), "audio") == 0) {
			desired_buf_size = 200000;
		}
		
		if (!subsession->initiate()) {
			lib::logger::get_logger()->debug("ambulant::net::rtsp_demux(net::url& url) failed to initiate subsession");
			lib::logger::get_logger()->error("RTSP Connection Failed");
		}
		
		int rtp_sock_num = subsession->rtpSource()->RTPgs()->socketNum();
		int buf_size = increaseRecieveBufferTo(*env, rtp_sock_num, desired_buf_size);
		
		if(!m_rtsp_client->setupMediaSubsession(*subsession, FALSE, rtspStreamOverTCP)) {
			lib::logger::get_logger()->debug("ambulant::net::rtsp_demux(net::url& url) failed to send setup command to subsesion");
			lib::logger::get_logger()->error("RTSP Connection Failed");
			return;
		}
	}
		
}

unsigned long 
ambulant::net::rtsp_demux::run() 
{
	if(!m_rtsp_client->playMediaSession(*m_media_session)) {
		lib::logger::get_logger()->debug("ambulant::net::rtsp_demux(net::url& url) play failed");
		lib::logger::get_logger()->error("playing RTSP connection failed");
		return -1;
	
		m_
		TaskScheduler& scheduler =
}

void afterreading(void* clientData, unsigned frameSize,
			 unsigned /*numTruncatedBytes*/,
			 struct timeval presentationTime,
			 unsigned /*durationInMicroseconds*/) {
			 }
