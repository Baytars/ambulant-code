
/* 
 * @$Id$ 
 */

/*
 * The contents of this file are subject to the Public
 * License XXX. 
 *
 */

 
#ifndef AMBULANT_LIB_TIMER_H
#define AMBULANT_LIB_TIMER_H

namespace ambulant {

namespace lib {

class abstract_timer {
  public:
	// this timer time type (assumed in msecs)
	typedef unsigned long time_type;
	
	virtual ~abstract_timer() {}
		
	// returns the time elapsed
	// e.g. return (time_now>ref_time)?time_now - ref_time:0;
	virtual time_type elapsed() const = 0;
	virtual void set_speed(double speed) = 0;
};

// Note: timer objects are not refcounted, because it is assumed that
// a parent timer will always automatically live longer than a child
// timer. If this does not hold in future then we should add refcounting.
class timer : public abstract_timer {
  public:
	timer(abstract_timer *parent, double speed);
	timer(abstract_timer *parent);
	~timer();
//	void add_dependent(timer& child);
		
	// returns the time elapsed
	// e.g. return (time_now>ref_time)?time_now - ref_time:0;
	time_type elapsed() const;
	void set_speed(double speed);
  private:
	void re_epoch();
	
	abstract_timer *m_parent;
//	std::vector<timer&> m_children;
	time_type m_parent_epoch;
	time_type m_local_epoch;
	double m_speed;
};

// A (machine-dependent) routine to create a timer object
abstract_timer *realtime_timer_factory();

} // namespace lib
 
} // namespace ambulant
#endif // AMBULANT_LIB_TIMER_H


