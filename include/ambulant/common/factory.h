#ifndef _FACTORY_H
#define _FACTORY_H

#include "ambulant/net/datasource.h"
#include "ambulant/common/playable.h"

namespace ambulant {

namespace common {
	
struct factories {
	net::datasource_factory  *df;
	common::global_playable_factory *rf;
	ambulant::common::window_factory *wf;
};


} // end namespaces
} // end namespaces


	


	

#endif /* _FACTORY_H */
