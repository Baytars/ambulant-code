
/* 
 * @$Id$ 
 */

/*
 * The contents of this file are subject to the Public
 * License XXX. 
 *
 */

 
#ifndef AMBULANT_LIB_UNIX_TIMER_H
#define AMBULANT_LIB_UNIX_TIMER_H

#include "ambulant/lib/timer.h"
#undef unix
namespace ambulant {

namespace lib {

namespace unix {

// XXX: time() returns secs. This timer should be msec based. 

// simple unix os timer
class unix_timer : public ambulant::lib::abstract_timer  {
  public:
	unix_timer() {};
	
	time_type elapsed() const;
	void set_speed(double speed);
  private:
	static time_type os_millitime();
};


} // namespace unix
 
} // namespace lib
 
} // namespace ambulant

#endif // AMBULANT_LIB_UNIX_TIMER_H
