/* 
 * @$Id$ 
 */

/*
 * The contents of this file are subject to the Public
 * License XXX. 
 *
 */

#include "ambulant/lib/region.h"
#include "ambulant/lib/logger.h"

namespace ambulant {

namespace lib {


active_region *
passive_region::activate(event_processor *const evp, const node *node)
{
	return new active_region(evp, this, node);
}

void
active_region::start(event *playdone)
{
	log_trace_event("active_region.start(0x%x, \"%s\", playdone=0x%x)", (void *)this, m_source->m_name, (void *)playdone);
	if (playdone)
		m_event_processor->add_event(playdone, 0, event_processor::low);
}

void
active_region::stop()
{
	log_trace_event("active_region.stop(0x%x, \"%s\")", (void *)this, m_source->m_name);
}

} // namespace lib
 
} // namespace ambulant
