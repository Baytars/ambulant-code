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

#ifndef AMBULANT_LIB_TIMER_H
#define AMBULANT_LIB_TIMER_H

#include "ambulant/config/config.h"

#include <set>

namespace ambulant {

namespace lib {

/// Priorities of clocks. A child clock will update its parent time if it's priority is
/// at least as big
enum timer_priority {
	tp_free,	///< Free-running, does not influence other timers
	tp_default,	///< Normal priority, for interior nodes and static media
	tp_video,	///< Medium-high priority, for video nodes
	tp_audio,	///< High priority, for audio nodes
	tp_master,	///< Highest priority, for syncMaster=true nodes
};

/// Client interface to timer objects: allows you to get the
/// current time and the rate at which time passes.
class timer {
  public:
	/// The underline time type used by this timer. 
	/// Assumed to be an integral type.
	typedef long time_type;
	
	// Allows subclasses to be deleted using base pointers
	virtual ~timer() {}
		
	/// Returns the time elapsed.
	virtual time_type elapsed() const = 0;
	
	/// Set the priority of this timer.
	virtual void set_priority(timer_priority prio) = 0;

	// Returns the zero-based time elapsed for the provided parent elapsed time.
//	virtual time_type elapsed(time_type pt) const = 0;
	
	/// Starts ticking at t (t>=0).
	virtual void start(time_type t = 0) = 0;
	
	/// Stop ticking and reset elapsed time to zero.
	virtual void stop() = 0;
	
	/// Stop ticking but do not reset the elapsed time.
	/// While paused this timer's elapsed() returns the same value. 
	/// Speed remains unchanged and when resumed
	/// will be ticking at that speed.
	virtual void pause() = 0;
	
	/// Resumes ticking.
	virtual void resume() = 0;
	
	/// Sets the speed of this timer.
	/// At any state, paused or running, set_speed() 
	/// may be called to change speed.
	/// When paused, the new speed will be
	/// used when the timer is resumed else
	/// the new speed is applied immediately.
	/// The current elapsed time is not affected. 
	virtual void set_speed(double speed) = 0;
	
	/// Set the current elapsed time. The number returned is 0 if the
	/// clock was updated, otherwise the delta-t with which to adapt your own clock.
	virtual time_type set_time(time_type t, timer_priority prio=tp_master) = 0;
	
	// Returns the speed of this timer.
	virtual double get_speed() const = 0;
	
	/// Returns true when this timer is running.
	virtual bool running() const = 0;
	
};

// Base class for realtime timers. Basically stubs out everything.
class realtime_timer: public timer {
  public:
	virtual ~realtime_timer() {}
	void set_priority(timer_priority prio) { assert(0); };
	void start(time_type t = 0) { assert(0); };
	void stop() { assert(0); };
	void pause() { assert(0); };
	void resume() { assert(0); };
	void set_speed(double speed) { assert(0); };
	time_type set_time(time_type t, timer_priority prio=tp_master) { return t-elapsed(); };
	double get_speed() const { return 1.0; };
	bool running() const {return true; };
};

typedef timer timer_control;

/// An implementation of timer_control.
class timer_control_impl : public timer_control {
  public:	
	/// Creates a timer.
	/// Pass the parent timer, 
	/// the relative speed and
	/// initial run/pause status. 
	timer_control_impl(timer *parent, double speed = 1.0, bool run = true, bool owned = false);
	
	~timer_control_impl();
	
	/// Returns the zero-based elapsed time.
	/// Does not take periodicity into account.
	time_type elapsed() const;
	
	// Returns the zero-based time elapsed for the provided parent elapsed time.
//	time_type elapsed(time_type pt) const;
		
	void set_priority(timer_priority prio);

	/// Starts ticking at t (t>=0).
	void start(time_type t = 0);
	
	/// Stop ticking and reset elapsed time to zero.
	void stop();
	
	/// Stop ticking but do not reset the elapsed time.
	/// While paused this timer's elapsed() returns the same value. 
	/// Speed remains unchanged and when resumed
	/// will be ticking at that speed.
	void pause();
	
	/// Resumes ticking.
	void resume();
	
	/// Sets the speed of this timer.
	/// At any state, paused or running, set_speed() 
	/// may be called to change speed.
	/// When paused, the new speed will be
	/// used when the timer is resumed else
	/// the new speed is applied immediately.
	/// The current elapsed time is not affected. 
	void set_speed(double speed);
	
	/// Set the current elapsed time.
	time_type set_time(time_type t, timer_priority prio=tp_master);
	
	// Returns the speed of this timer.
	double get_speed() const { return m_speed;}
	
	/// Returns true when this timer is running.
	bool running() const { return m_running;}
	
	
  private:
	time_type apply_speed_manip(time_type dt) const;
	
	timer *m_parent;
	bool m_parent_owned;
	time_type m_parent_epoch;
	time_type m_local_epoch;
	double m_speed;
	bool m_running;
	long m_period;
	timer_priority m_priority;
};

/// Factory function that returns a machine-dependent timer implementation.
AMBULANTAPI timer *realtime_timer_factory();

} // namespace lib
 
} // namespace ambulant
#endif // AMBULANT_LIB_TIMER_H


