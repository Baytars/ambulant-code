// This file is part of Ambulant Player, www.ambulantplayer.org.
//
// Copyright (C) 2003-2008 Stichting CWI, 
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

#include "ambulant/lib/timer.h"

#   if __GNUC__ == 2 && __GNUC_MINOR__ <= 97
#include "ambulant/compat/limits"
#else
#include <limits>
#endif

#include "ambulant/lib/logger.h"
#include <cmath>

#ifndef AM_DBG
#define AM_DBG if(0)
#endif

using namespace ambulant;

static long infinite = std::numeric_limits<long>::max();

lib::timer_control_impl::timer_control_impl(lib::timer* parent, double speed /* = 1.0 */, 
	bool run /* = true */, bool owned /* = false */)
:   m_parent(parent),
	m_parent_owned(owned),
	m_parent_epoch(parent->elapsed()),
	m_local_epoch(0),
	m_speed(speed),
	m_running(run),
	m_period(infinite),
	m_priority(tp_default)
{	
	AM_DBG lib::logger::get_logger()->debug("lib::timer_control_impl(0x%x), parent=0x%x", this, parent);
}

lib::timer_control_impl::~timer_control_impl()
{
	if (m_parent_owned) delete m_parent; // Is this correct?
	AM_DBG lib::logger::get_logger()->debug("~lib::timer_control_impl()");
}

void
lib::timer_control_impl::set_priority(timer_priority prio)
{
	if (prio == m_priority) return;
	m_priority = prio;
	if (prio == tp_free) {
		// Free-running timer is slaved to realtime clock.
		pause();
		if (m_parent_owned) delete m_parent;
		m_parent = realtime_timer_factory();
		resume();
	}
}
		
lib::timer_control_impl::time_type
lib::timer_control_impl::elapsed() const
{
	if(!m_running) return m_local_epoch;
	return m_local_epoch + apply_speed_manip(m_parent->elapsed() - m_parent_epoch);
}

#if 0
lib::timer_control_impl::time_type
lib::timer_control_impl::elapsed(time_type pe) const
{
	if(!m_running) return m_local_epoch;
	return m_local_epoch + apply_speed_manip(pe - m_parent_epoch);
}
#endif

void lib::timer_control_impl::start(time_type t /* = 0 */) {
	m_parent_epoch = m_parent->elapsed();
	m_local_epoch = t;
	m_running = true;
}

lib::timer_control_impl::time_type 
lib::timer_control_impl::apply_speed_manip(lib::timer::time_type dt) const 
{
	if(m_speed == 1.0) return dt;
	else if(m_speed == 0.0) return 0;
	return time_type(::floor(m_speed*dt + 0.5));
}

void
lib::timer_control_impl::stop()
{
	m_local_epoch = 0;
	m_running = false;
}
	
void
lib::timer_control_impl::pause()
{
	if(m_running) {
		m_local_epoch += apply_speed_manip(m_parent->elapsed() - m_parent_epoch);
		m_running = false;
	}
}
	
void
lib::timer_control_impl::resume()
{
	if(!m_running) {
		m_parent_epoch = m_parent->elapsed();
		m_running = true;
	}
}

lib::timer_control_impl::time_type
lib::timer_control_impl::set_time(time_type t, timer_priority prio)
{
	AM_DBG lib::logger::get_logger()->debug("0x%x.set_time(%d->%d, %d)", this, elapsed(), t, prio); 
	// If the priority of this request is lower than our priority: ignore it.
	if (prio < m_priority || m_priority == tp_free) return t-elapsed();
	if (prio == tp_default && m_priority == tp_default) {
		// Default can be either hard sync (in which case we set our clock) or soft sync
		// (in which case we ignore).
#if 0
		return t-elapsed();
#endif
	}
	time_type now = elapsed();
	// We're setting the time to what it already is. Do nothing.
	if (now == t) return 0;
	// Otherwise we pause the clock (which resets the epoch), update
	// the epoch, inform our parent.
	bool was_running = m_running;
	pause();
	time_type delta = t - m_local_epoch;
//	delta = m_parent->set_time(m_parent->elapsed() + delta, m_priority);
	m_local_epoch += delta;
	if (was_running) resume();
	return 0;
}	

#if 0
lib::timer_control_impl::time_type 
lib::timer_control_impl::get_time() const
{
	return (m_period == infinite)?elapsed():(elapsed() % m_period);
}

lib::timer_control_impl::time_type 
lib::timer_control_impl::get_repeat() const
{
	return (m_period == infinite)?0:(elapsed() / m_period);
}
#endif

void lib::timer_control_impl::set_speed(double speed)
{
	if(!m_running) {
		m_speed = speed;
	} else {
		pause();
		m_speed = speed;
		resume();
	}
}
