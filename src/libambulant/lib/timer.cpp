/* 
 * @$Id$ 
 */

/*
 * The contents of this file are subject to the Public
 * License XXX. 
 *
 */

#include "ambulant/lib/timer.h"

#ifndef AM_DBG
#define AM_DBG if(0)
#endif

using namespace ambulant;

lib::abstract_timer::abstract_timer_vector::iterator
lib::abstract_timer::add_dependent(abstract_timer *child)
{
	lib::abstract_timer::abstract_timer_vector::iterator rv = m_dependents.end();
	m_dependents.push_back(child);
	return rv;
}

void
lib::abstract_timer::remove_dependent(abstract_timer_vector::iterator pos)
{
	m_dependents.erase(pos);
}

void
lib::abstract_timer::speed_changed()
{
	abstract_timer_vector::iterator i;
	for (i=m_dependents.begin(); i != m_dependents.end(); i++)
		(*i)->speed_changed();
}

lib::timer::timer(lib::abstract_timer* parent, double speed)
:   m_parent(parent),
	m_parent_epoch(parent->elapsed()),
	m_local_epoch(0),
	m_speed(speed)
{
}

lib::timer::timer(lib::abstract_timer* parent)
:   m_parent(parent),
	m_parent_epoch(parent->elapsed()),
	m_local_epoch(0),
	m_speed(1.0)
{
}

lib::timer::~timer()
{
}

lib::timer::time_type
lib::timer::elapsed() const
{
	return (lib::timer::time_type)(m_local_epoch + m_speed*(m_parent->elapsed()-m_parent_epoch));
}

void
lib::timer::set_speed(double speed)
{
	re_epoch();
	m_speed = speed;
}

double
lib::timer::get_realtime_speed() const
{
	return m_speed * m_parent->get_realtime_speed();
}

void
lib::timer::re_epoch()
{
	// Could be off by a little, but too lazy to do it better right now:-)
	m_local_epoch = elapsed();
	m_parent_epoch = m_parent->elapsed();
}
