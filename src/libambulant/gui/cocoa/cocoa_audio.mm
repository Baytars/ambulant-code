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

#include "ambulant/gui/cocoa/cocoa_audio.h"

#ifndef AM_DBG
#define AM_DBG if(0)
#endif

namespace ambulant {

using namespace lib;
using namespace common;

namespace gui {

namespace cocoa {

cocoa_audio_playable::cocoa_audio_playable(
	playable_notification *context,
	playable_notification::cookie_type cookie,
	const lib::node *node,
	lib::event_processor *evp)
:	playable_imp(context, cookie, node, evp),
	m_url(node->get_url("src")),
	m_sound(NULL)
{
    NSAutoreleasePool *pool = [[NSAutoreleasePool alloc] init];
	NSString *filename = [NSString stringWithCString: m_url.c_str()];
	m_sound = [[NSSound alloc] initWithContentsOfFile:filename byReference: YES];
	if (!m_sound)
		lib::logger::get_logger()->error("cocoa_audio_playable: cannot open soundfile: %s", m_url.c_str());
//	m_event_processor->get_timer()->add_listener(this);
	[pool release];
}

cocoa_audio_playable::~cocoa_audio_playable()
{
    NSAutoreleasePool *pool = [[NSAutoreleasePool alloc] init];
	m_lock.enter();
	AM_DBG logger::get_logger()->trace("~cocoa_audio_playable(0x%x)", (void *)this);
	if (m_sound)
		[m_sound release];
	m_sound = NULL;
//	m_event_processor->get_timer()->remove_listener(this);
	m_lock.leave();
	[pool release];
}
	
void
cocoa_audio_playable::start(double where)
{
    NSAutoreleasePool *pool = [[NSAutoreleasePool alloc] init];
	m_lock.enter();
	AM_DBG lib::logger::get_logger()->trace("cocoa_audio_playable.start(0x%x, %s, %f)", (void *)this, m_url.c_str(), where);
	if (where != 0.0)
		lib::logger::get_logger()->warn("cocoa_audio_playable: ignoring start time %f, starting at 0", where);
	if (m_sound)
		if (![m_sound play]) {
			lib::logger::get_logger()->error("cocoa_audio_playable: cannot start audio");
			[m_sound release];
			m_sound = NULL;
		}
	m_context->started(m_cookie, 0);
	if (!m_sound)
		m_context->stopped(m_cookie, 0);
	m_lock.leave();
	[pool release];
}

void
cocoa_audio_playable::stop()
{
    NSAutoreleasePool *pool = [[NSAutoreleasePool alloc] init];
	m_lock.enter();
	AM_DBG lib::logger::get_logger()->trace("cocoa_audio_playable.stop(0x%x)", (void *)this);
	if (m_sound) {
		if (![m_sound stop])
			lib::logger::get_logger()->error("cocoa_audio_playable: cannot stop audio");
		[m_sound release];
		m_sound = NULL;
		m_context->stopped(m_cookie, 0);
	}
	m_lock.leave();
	[pool release];
}

void
cocoa_audio_playable::pause()
{
//    NSAutoreleasePool *pool = [[NSAutoreleasePool alloc] init];
	m_lock.enter();
	AM_DBG lib::logger::get_logger()->trace("cocoa_audio_playable.pause(0x%x)", (void *)this);
	if (m_sound) {
		if (![m_sound pause])
			lib::logger::get_logger()->error("cocoa_audio_playable: cannot pause audio");
	}
	m_lock.leave();
//	[pool release];
}

void
cocoa_audio_playable::resume()
{
//    NSAutoreleasePool *pool = [[NSAutoreleasePool alloc] init];
	m_lock.enter();
	AM_DBG lib::logger::get_logger()->trace("cocoa_audio_playable.resume(0x%x)", (void *)this);
	if (m_sound) {
		if (![m_sound resume])
			lib::logger::get_logger()->error("cocoa_audio_playable: cannot resume audio");
	}
	m_lock.leave();
//	[pool release];
}

#if 0
void
cocoa_audio_playable::speed_changed()
{
	m_lock.enter();
	AM_DBG lib::logger::get_logger()->trace("cocoa_audio_playable.speed_changed(0x%x)", (void *)this);
	if (m_sound) {
		abstract_timer *our_timer = m_event_processor->get_timer();
		double rtspeed = our_timer->get_realtime_speed();
		
		if (rtspeed < 0.01) {
			if (![m_sound pause])
				lib::logger::get_logger()->error("cocoa_audio_playable: cannot pause audio");
		} else {
			if (rtspeed < 0.99)
				lib::logger::get_logger()->trace("cocoa_audio_playable: only speed 1.0 and 0.0 supported, not %f", rtspeed);
			if (![m_sound resume])
				lib::logger::get_logger()->error("cocoa_audio_playable: cannot resume audio");
		}
	}
	m_lock.leave();
}
#endif


} // namespace cocoa

} // namespace gui

} //namespace ambulant
