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

#include "ambulant/lib/logger.h"
#include "ambulant/lib/node.h"
#include "ambulant/smil2/transition.h"

#define AM_DBG
#ifndef AM_DBG
#define AM_DBG if(0)
#endif

using namespace ambulant;
using namespace smil2;

transition_engine::transition_engine(common::surface *dst, bool outtrans, lib::transition_info *info)
:   m_dst(dst),
	m_outtrans(outtrans),
	m_info(info),
	m_begin_time(0)
{
	AM_DBG lib::logger::get_logger()->trace("transition_engine::transition_engine()");
	m_progress = m_info->m_startProgress;
	lib::transition_info::time_type dur = m_info->m_dur;
	if (dur <= 0) {
		lib::logger::get_logger()->error("transition_engine: incorrect transition duration %f", float(dur));
		dur = 1;
	}
	m_time2progress = (m_info->m_endProgress - m_info->m_startProgress) / dur;
}

transition_engine::~transition_engine()
{
	AM_DBG lib::logger::get_logger()->trace("transition_engine::~transition_engine()");
}

void
transition_engine::begin(lib::transition_info::time_type now)
{
	AM_DBG lib::logger::get_logger()->trace("transition_engine::begin(%d)", now);
	m_begin_time = now;
}

void
transition_engine::end()
{
	AM_DBG lib::logger::get_logger()->trace("transition_engine::end()");
}

void
transition_engine::step(lib::transition_info::time_type now)
{
	AM_DBG lib::logger::get_logger()->trace("transition_engine::step(%d)", now);
	m_progress = (now-m_begin_time) * m_time2progress;
	if (m_progress > m_info->m_endProgress) m_progress = 1.0;
	/*AM_DBG*/ lib::logger::get_logger()->trace("transition_engine::step: delta_t=%d, progress=%f%%", now-m_begin_time, m_progress*100);
	update();
}

void
transition_engine::update()
{
	/*AM_DBG*/ lib::logger::get_logger()->trace("transition_engine::update()");
}

bool
transition_engine::is_done()
{
	return m_progress >= m_info->m_endProgress;
}

lib::transition_info::time_type
transition_engine::next_step_delay()
{
	return 50; // Show something 20 times per second
}

void
transition_engine_fade::compute()
{
}

void
transition_engine_barwipe::compute()
{
	lib::screen_rect<int> dstrect = m_dst->get_rect();
	int xcur = dstrect.m_left + int(m_progress*(dstrect.m_right + dstrect.m_left) + 0.5);
	int ycur = dstrect.m_top + int(m_progress*(dstrect.m_bottom + dstrect.m_top) + 0.5);
	m_oldrect = dstrect;
	m_newrect = lib::screen_rect(
		lib::point(dstrect.m_left, dstrect.m_top),
		lib::point(xcur, ycur));
}

