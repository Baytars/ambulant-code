/*
 * This file is part of Ambulant Player, www.ambulantplayer.org.
 *
 * Copyright (C) 2003-2012 Stichting CWI, 
 * Science Park 123, 1098 XG Amsterdam, The Netherlands.
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

#ifndef AMBULANT_RECORDER_RECORDER_PLUGIN_H
#define AMBULANT_RECORDER_RECORDER_PLUGIN_H

#include "ambulant/common/factory.h"
#include "ambulant/common/recorder.h"
#include "ambulant/common/plugin_engine.h"
#include "ambulant/common/gui_player.h"
#include "ambulant/version.h"

namespace ambulant {

namespace common {

class recorder_plugin : recorder {
	/// Initialize for receiving video frames of a particular type
  virtual void initalize_frames(recorder::frame_type)  { assert(0); }

	/// Initialize for receiving audio packets of a particular type
	virtual void initalize_packets(packet_format)  { assert(0); }

	/// Initialize for producing an AV stream of a particular type
	virtual void initalize_output_stream(stream_type, const char* name="")  { assert(0); }

	/// Record a (video) frame.
	virtual void new_frame(void* data, size_t datasize, lib::timer::time_type documenttimestamp, frame_type=default_frame_type)  { assert(0); }

	/// Record a (audio) packet.
	virtual void new_packet(void* data, size_t datasize, lib::timer::timer::time_type _documentimestamp,  packet_format=default_format)  { assert(0); }


}; // class recorder_plugin

class recorder_plugin_factory : public recorder_factory {
  public:

	recorder_plugin_factory(common::factories* factory)
	:	m_factory(factory) {}
	~recorder_plugin_factory() {};

	recorder_plugin* new_recorder_plugin();

  private:
	common::factories* m_factory;

}; // class recorder_plugin_factory 

}; // namespace common

}; // namespace ambulant
#endif // AMBULANT_RECORDER_RECORDER_PLUGIN_H
