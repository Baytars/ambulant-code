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

/* 
 * A socket is an endpoint for communication
 * between two machines. 
 *
 * Actual implementations will exist for
 * each platform supported.
 *
 */
 
#ifndef AMBULANT_NET_SOCKET_H
#define AMBULANT_NET_SOCKET_H

#include "ambulant/config/config.h"

namespace ambulant {

namespace net {

class socket {
  public:
  virtual ~socket() {}
};
 
 
} // namespace net
 
} // namespace ambulant

#endif // AMBULANT_NET_SOCKET_H

 
 