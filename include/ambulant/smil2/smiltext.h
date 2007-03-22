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

#include <stack>

#ifdef WITH_SMIL30
namespace ambulant {

namespace smil2 {

enum smiltext_command {
	stc_data,
	stc_break
};

enum smiltext_direction {
	std_ltr,
	std_rtl
};

enum smiltext_align {
	sta_start,
	sta_end,
	sta_left,
	sta_right,
	sta_center
};

enum smiltext_font_style {
	sts_normal,
	sts_italic,
	sts_oblique,
	sts_reverse_oblique
};

enum smiltext_font_weight {
	stw_normal,
	stw_bold
};

/// A sequence of characters with a common set of attributes
/// such as font, color, etc
class smiltext_run {
  public:
	smiltext_command m_command;
	lib::xml_string m_data;
	
	const char *			m_font_family;
	smiltext_font_style		m_font_style;
	smiltext_font_weight	m_font_weight;
	int						m_font_size;
	bool					m_transparent;
	lib::color_t			m_color;
	bool					m_bg_transparent;
	lib::color_t			m_bg_color;
	smiltext_align			m_align;
	smiltext_direction		m_direction;
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
	smiltext_engine(const lib::node *n, lib::event_processor *ep, smiltext_notification *client);
	~smiltext_engine();
	
	/// Start the engine.
	void start(double t);
	
	/// Seek the engine in time.
	void seek(double t);
	
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
	
	/// HACK! We simulate the ref_counted interface
	void add_ref() {}
	void release() {}
  private:
	// Callback routine that updates the text runs to the current state.
	void _update();
	// Fill a run with the formatting parameters from a node.
	void _get_formatting(smiltext_run& dst, const lib::node *src);
	// Fill a run with the defaulty formatting.
	void _get_default_formatting(smiltext_run& dst);
	
	const lib::node *m_node;			// The root of the smiltext nodes
	lib::node::const_iterator m_tree_iterator;	// Where we currently are in that tree
	lib::event_processor *m_event_processor;
	smiltext_notification *m_client;	// The renderer
	smiltext_runs m_runs;				// Currently active text runs
	std::stack<smiltext_run> m_run_stack;	// Stack of runs for nested spans
	smiltext_runs::const_iterator m_newbegin;	// Items in m_runs before this were seen previously
	lib::event *m_update_event;			// event_processor callback to _update
	lib::timer::time_type m_epoch;		// event_processor time corresponding to smiltext time=0
	double m_tree_time;					// smiltext time for m_tree_iterator
	bool m_append;
};

} // namespace smil2
 
} // namespace ambulant

#endif // WITH_SMIL30

#endif // AMBULANT_SMIL2_SMILTEXT_H
