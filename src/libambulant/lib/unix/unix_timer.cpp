
/* 
 * @$Id$ 
 */

/*
 * The contents of this file are subject to the Public
 * License XXX. 
 *
 */

 
#include "ambulant/lib/unix/unix_timer.h"
#include <sys/time.h>

using namespace ambulant;

lib::unix::os_timer::os_timer()
:	m_start_time(millitime())
{
}

lib::unix::os_timer::time_type
lib::unix::os_timer::elapsed() const
{
	return millitime()-m_start_time;
}

void
lib::unix::os_timer::restart()
{
	m_start_time = millitime();
}

lib::unix::os_timer::time_type
lib::unix::os_timer::millitime()
{
	struct timeval tv;
	
	if (gettimeofday(&tv, NULL) < 0) return 0;
	return (tv.tv_sec*1000 + tv.tv_usec / 1000);
}

// Factory routine for the machine-independent
// timer class
lib::timer *
lib::timer_factory()
{
	return (timer *)new lib::unix::os_timer();
}
