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
 
#include "ambulant/gui/dg/dg_audio_player.h"
#include "ambulant/gui/dg/dg_mp3_decoder.h"
#include "ambulant/gui/dg/dg_audio_renderer.h"
#include "ambulant/lib/win32/win32_fstream.h"
#include "ambulant/lib/win32/win32_error.h"
#include "ambulant/lib/logger.h"

//#define AM_DBG if(1)

#ifndef AM_DBG
#define AM_DBG if(0)
#endif

using namespace ambulant;

static gui::dg::audio_renderer s_renderer;

gui::dg::audio_player::audio_player(const std::string& url) 
:	m_bbuf(read_size),
	m_decbuf(0) {
	memset(&m_wfx, 0, sizeof(WAVEFORMATEX));
	if(!m_ifs.open(url)) {
		lib::logger::get_logger()->show("Failed to open: %s", url);
		return;
	} 
	m_ifs.read(m_bbuf);
	m_bbuf.flip();
	m_decoder.get_wave_format(m_bbuf, m_wfx);
	if(s_renderer.can_play(m_wfx)) {
		if(s_renderer.open(m_wfx))
			update();
	}
}

gui::dg::audio_player::~audio_player() {
	if(m_ifs.is_open()) m_ifs.close();
	if(s_renderer.is_open()) s_renderer.stop();
	if(m_decbuf != 0) delete m_decbuf;
}

bool gui::dg::audio_player::can_play() const {
	return s_renderer.is_open();
}
	
void gui::dg::audio_player::start(double t) {
	if(s_renderer.is_open())
		s_renderer.start();
}

void gui::dg::audio_player::pause() {
	if(s_renderer.is_open())
		s_renderer.pause();
}
	
void gui::dg::audio_player::resume() {
	if(s_renderer.is_open())
		s_renderer.resume();
}
	
void gui::dg::audio_player::stop() {
	if(m_ifs.is_open()) m_ifs.close();
	if(s_renderer.is_open()) {
		s_renderer.stop();
	}
}
	
std::pair<bool, double> 
gui::dg::audio_player::get_dur() {
	return std::pair<bool, double>(false, 0.0);
}

bool gui::dg::audio_player::is_playing() {
	if(s_renderer.is_open()) update();
	return s_renderer.is_open() && s_renderer.has_audio_data();
}
	
void gui::dg::audio_player::update() {
	if(s_renderer.is_open()) s_renderer.update();
	if(!s_renderer.is_open() || !m_ifs.is_open()) return;
	if(s_renderer.get_audio_data_size() > lo_limit) return;
	while(m_ifs.is_open() && 
		s_renderer.get_audio_data_size()<hi_limit) {
		// Decode
		if(m_decbuf == 0) {
			m_decbuf = new std::basic_string<char>();
			m_decbuf->reserve(dec_size_estim);
		}
		m_decoder.decode(m_bbuf, m_decbuf);
		m_bbuf.compact();
				
		// Render buffer
		if(m_decbuf->size() > 0) {
			s_renderer.write(m_decbuf);
			m_decbuf = 0;
		}
		
		// fill read buffer
		int ppos = m_bbuf.get_position();
		m_ifs.read(m_bbuf);
		if(ppos == m_bbuf.get_position())
			m_ifs.close();
		m_bbuf.flip();
	}
}

void gui::dg::audio_player::resample() {
	bool resample = false;
	if(m_wfx.nSamplesPerSec == 48000) {
		m_wfx.nSamplesPerSec = 44100;
		resample = true;
	} else if(m_wfx.nSamplesPerSec != 22050) {
		m_wfx.nSamplesPerSec = 22050;
		resample = true;
	}
	if(resample) {
		m_decoder.reset();
		m_decoder.get_wave_format(m_bbuf, m_wfx);
	}
}


