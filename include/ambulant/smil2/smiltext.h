/*
 * This file is part of Ambulant Player, www.ambulantplayer.org.
 *
 * Copyright (C) 2003-2007 Stichting CWI, 
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

#ifndef AMBULANT_SMIL2_SMILTEXT_H
#define AMBULANT_SMIL2_SMILTEXT_H

#include "ambulant/config/config.h"
#include "ambulant/lib/node.h"
#include "ambulant/lib/colors.h"
#include "ambulant/lib/event_processor.h"

#ifdef WITH_SMIL30
namespace ambulant {

namespace smil2 {

/// A sequence of characters with a common set of attributes
/// such as font, color, etc
class smiltext_run {
  public:
	lib::xml_string m_data;
	const char *	m_font;
	int				m_fontsize;
	lib::color_t	m_color;
};

typedef std::vector<smiltext_run> smiltext_runs;

/// Interface that engine of the client should provide. The
/// engine will use this to notify the client that the text has
/// changed and should be redrawn.
class smiltext_notification {
  public:
	virtual ~smiltext_notification() {}
	
	/// Called whenever something has changed.
	virtual void smiltext_changed() = 0;
};

/// Engine to process smiltext
class smiltext_engine {
  public:
	smiltext_engine(const lib::node *n, lib::event_processor *ep, smiltext_notification *client)
	:	m_node(n),
		m_tree_iterator(n->begin()),
		m_event_processor(ep),
		m_client(client)
	{
	}
	~smiltext_engine() {}
	
	/// Start the engine.
	void start();
	
	/// Stop the engine.
	void stop();
	
	/// Returns true if all text has been received.
	bool is_finished() { return m_tree_iterator.is_end(); }
	
	/// Returns true if the text has changed since the last done() call.
	bool is_changed() { return m_newbegin != m_runs.end(); }
	
	/// Returns true if the text has been cleared and should be re-rendered from scratch.
	bool is_cleared() { return m_newbegin == m_runs.begin(); }
	
	/// Returns an iterator pointing to the first smiltext_run.
	smiltext_runs::const_iterator begin() { return m_runs.begin(); }
	
	/// Returns an iterator pointing to the first unseen smiltext_run.
	smiltext_runs::const_iterator newbegin() { return m_newbegin; }
	
	/// Returns an iterator pointing to the end of the smiltext_runs.
	smiltext_runs::const_iterator end() { return m_runs.end(); }
	
	/// Called when the client has processed all runs.
	void done() { m_newbegin = m_runs.end(); }
	
  private:
	void _update();
	
	const lib::node *m_node;
	lib::node::const_iterator m_tree_iterator;
	lib::event_processor *m_event_processor;
	smiltext_notification *m_client;
	bool m_finished;
	smiltext_runs m_runs;
	smiltext_runs::const_iterator m_newbegin;
};

} // namespace smil2
 
} // namespace ambulant

#endif // WITH_SMIL30

#endif // AMBULANT_SMIL2_SMILTEXT_H
