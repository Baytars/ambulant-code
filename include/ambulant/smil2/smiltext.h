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
#include "ambulant/lib/gtypes.h"
#include "ambulant/lib/node.h"
#include "ambulant/lib/colors.h"
#include "ambulant/lib/event_processor.h"

#include <stack>

#ifdef WITH_SMIL30
namespace ambulant {

namespace smil2 {

/// Various modes the smiltext can be displayed in.
enum smiltext_mode {
	stm_replace,		/// New text is timed, replaces everything, no scrolling
	stm_append,		/// New text is timed, appended to the end, no scrolling
	stm_scroll,		/// Timing is ignored, text auto-scrolls upward
	stm_crawl,		/// Timing is ignored, text scrolls right-to-left
	stm_jump,		/// New text is timed, appended to end, scrolls up if needed
};

/// Initial and ending psition of content when crawling/scrolling
enum smiltext_conceal {
	stc_none,	// textAlign/textPlace/content determine initial/final
	stc_initial,	// textAlign/textPlace ignored, rendering starts just outside
	stc_final,	// at the end, text flows out of region
	stc_both	// both initial and final effects are applied
};

/// Values for the textPlace attribute of smilText elements
enum smiltext_place {
	stp_from_start,
	stp_from_end,
	stp_from_center
};

/// Global parameters of a smiltext node.
/// These are the parameters for smiltext only, additionally the
/// renderer is expected to take care of all the normal timing/region/etc
/// attributes.
struct smiltext_params {
	smiltext_mode	m_mode;		/// How the text is rendered
	int		m_rate;		/// Rate, in pixels/second
	smiltext_place	m_text_place;	/// Where the text should start
	smiltext_conceal m_text_conceal; /// Initial/final effects for crawl/scroll
};
	
/// Layout commands that the engine can send to the renderer
enum smiltext_command {
	stc_data,		// Render some character
	stc_break,		// Unconditional line break
	stc_condbreak,	// Conditional break, will collapse with adjacent condbreaks
	stc_condspace	// Conditional space, will collapse with adjacent condspaces.
};

/// Values for the textAlign attribute of text spans
enum smiltext_align {
	sta_start,
	sta_end,
	sta_left,
	sta_right,
	sta_center
};

/// Values for the textFontSyle attribute of text spans
enum smiltext_font_style {
	sts_normal,
	sts_italic,
	sts_oblique,
	sts_reverse_oblique
};

/// Values for the textFontWeight attribute of text spans
enum smiltext_font_weight {
	stw_normal,
	stw_bold
};

/// Values for the textWritingMode attribute of smilText elements
enum smiltext_writing_mode {
	stw_lr_tb,
	stw_rl_tb,
	stw_tb_lr,
	stw_tb_rl
};

/// Values for the xml:space attribute of text spans
enum smiltext_xml_space {
	stx_default,
	stx_preserve
};

/// A sequence of characters with a common set of attributes
/// such as font, color, etc. Alternatively, if m_command
/// is not stc_data, it is a layout command (currently only
/// br is supported).
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
	smiltext_xml_space		m_xml_space;
	smiltext_align			m_align;
	smiltext_writing_mode	m_writing_mode;
	bool					m_wrap;		/// Text should be line-wrapped
};

/// A list of smiltext_run objects is what the renderer is
/// supposed to draw on the screen.
typedef std::list<smiltext_run> smiltext_runs;

/// Interface that engine of the client should provide. The
/// engine will use this to notify the client that the text has
/// changed and should be redrawn.
class smiltext_notification {
  public:
	virtual ~smiltext_notification() {}
	
	/// Called whenever something has changed.
	virtual void smiltext_changed() = 0;
	
	/// Called whenever a "marker" (tev/clear with id attribute) has been seen
	virtual void marker_seen(const char *name) = 0;
};

/// Engine to process smiltext.
class smiltext_engine {
  public:
	smiltext_engine(const lib::node *n, lib::event_processor *ep, smiltext_notification *client, bool word_mode);
	~smiltext_engine();
	
	/// Get the mode parameters
	
	const smiltext_params& get_params() const { return m_params; }
	
	/// Start the engine.
	void start(double t);
	
	/// Seek the engine in time.
	void seek(double t);
	
	/// Stop the engine.
	void stop();
	
	/// Returns true if all text has been received.
	bool is_finished() { return m_tree_iterator.is_end(); }
	
	/// Returns true if the text has changed since the last done() call.
	bool is_changed() { return m_newbegin_valid; }
	
	/// Returns true if the text has been cleared and should be re-rendered from scratch.
	bool is_cleared() { return m_newbegin_valid && m_newbegin == m_runs.begin(); }
	
	/// Returns true while computing textRate="auto"
	bool is_auto_rate() { return m_auto_rate; }

	/// Returns an iterator pointing to the first smiltext_run.
	smiltext_runs::const_iterator begin() { return m_runs.begin(); }
	
	/// Returns an iterator pointing to the first unseen smiltext_run.
	smiltext_runs::const_iterator newbegin() { return m_newbegin_valid ? m_newbegin : (smiltext_runs::const_iterator)m_runs.end(); }
	
	/// Returns an iterator pointing to the end of the smiltext_runs.
	smiltext_runs::const_iterator end() { return m_runs.end(); }
	
	/// Called when the client has processed all runs.
	void done() { m_newbegin = m_runs.end(); m_newbegin_valid = false; }

	/// Called as soon as the textRate is known. Turns off m_auto_rate.
	void set_rate(unsigned int new_rate);

	/// HACK! We simulate the ref_counted interface
	void add_ref() {}
	void release() {}
  private:
	// Callback routine that updates the text runs to the current state.
	void _update();
	// Fill a run with the formatting parameters from a node.
	void _get_formatting(smiltext_run& dst, const lib::node *src);
	// Fill a run with the default formatting.
	void _get_default_formatting(smiltext_run& dst);
	// Fill smiltext_params from a node
	void _get_params(smiltext_params& params, const lib::node *src);
	// Fill default smiltext_params
	void _get_default_params(smiltext_params& params);
	// generate a sequence of runs each containing a single word with
	// spacing if needed and with the formatting and styling parameters
	void _split_into_words(lib::xml_string data, smiltext_xml_space);	
	// generate a sequence of runs each containing a stc_break command
	// of a stc_data command without any line-feed (newline) character
	// but with the formatting and styling parameters
	lib::xml_string _split_into_lines(lib::xml_string data, size_t lf_pos, size_t limit);
	// Convenience method to ad a run to the end of the run list.
	inline void _insert_run_at_end(const smiltext_run &run) {
		smiltext_runs::const_iterator where =
		m_runs.insert( m_runs.end(), run);
		if (!m_newbegin_valid) {
			m_newbegin = where;
			m_newbegin_valid = true;
		}
	}

	const bool m_word_mode;
	const lib::node *m_node;			// The root of the smiltext nodes
	lib::node::const_iterator m_tree_iterator;	// Where we currently are in that tree
	lib::event_processor *m_event_processor;
	smiltext_notification *m_client;	// The renderer
	smiltext_runs m_runs;				// Currently active text runs
	std::stack<smiltext_run> m_run_stack;	// Stack of runs for nested spans
	smiltext_runs::const_iterator m_newbegin;	// Items in m_runs before this were seen previously
	bool m_newbegin_valid;				// True if m_newbegin is valid.
	lib::event *m_update_event;			// event_processor callback to _update
	lib::timer::time_type m_epoch;		// event_processor time corresponding to smiltext time=0
	double m_tree_time;					// smiltext time for m_tree_iterator. XXXJACK: unused and unneeded?
	smiltext_params m_params;			// global parameters
	bool m_process_lf;			// turn all \n chars int <br/> commands
	bool m_auto_rate;			// true while computing textRate="auto"
};

/// Extra classes for smiltext layout

/// Font information needed for smiltext layout
class smiltext_metrics {
  public:
	smiltext_metrics(unsigned int ascent, unsigned int descent, unsigned int height,
			 unsigned int width, unsigned int line_spacing, unsigned int word_spacing)
	  :	m_ascent(ascent), 
		m_descent(descent), 
	  	m_height(height),
		m_width(width),
		m_line_spacing(line_spacing),
		m_word_spacing(word_spacing) {}

	~smiltext_metrics() {}

	const unsigned int get_ascent()	{ return m_ascent; }
	const unsigned int get_descent(){ return m_descent; };
	unsigned int get_height()	{ return m_height; };
	unsigned int get_width()	{ return m_width; };
	unsigned int get_line_spacing() { return m_line_spacing; };
	unsigned int get_word_spacing() { return m_word_spacing; };

  private:
	unsigned int m_ascent;
	unsigned int m_descent;
	unsigned int m_height;	
	unsigned int m_width;	
	unsigned int m_line_spacing;	
	unsigned int m_word_spacing;	
};
/// Interface to be inherited by a renderer that wants to use smiltext_layout_engine
class smiltext_layout_provider {
  public:

	/// Return font information needed for for use smiltext_layout_engine
	virtual smiltext_metrics get_smiltext_metrics(const smiltext_run& str) = 0;

	/// Render the smiltext_run in the rectangle specified.
	/// 'word spacing' is the amount of whitespace pixels in front of the word. 
	virtual void render_smiltext(const smiltext_run& str, const lib::rect& r, unsigned int word_spacing) = 0;
	virtual void smiltext_stopped() = 0;
};

class smiltext_layout_word {

  public:
  smiltext_layout_word(const smiltext_run* run_p, smiltext_metrics stm, int sp, int nl);
	const smiltext_run* m_run_p;
	int m_leading_spaces;
	int m_leading_newlines;
	smiltext_metrics m_metrics;
	lib::rect m_bounding_box;
};

/// A helper class to be used by a renderer which can't do text layout
class smiltext_layout_engine {
  public:
  smiltext_layout_engine (const lib::node *n, lib::event_processor *ep, smiltext_layout_provider* provider, smiltext_notification* client, bool process_lf);
	/// Start the engine.
	void start(double t);
	
	/// Seek the engine in time.
	void seek(double t);
	
	/// Stop the engine.
	void stop();
	
	/// Returns true if all text has been received.
	bool is_finished() { return m_finished; }

	/// Redraw a rectangle.on screen 
	void redraw(const lib::rect& r);

	/// Set destination rectangle.on screen
	void set_dest_rect(const lib::rect& r);

	void smiltext_changed();

  private:
	void _get_initial_values(lib::rect rct, smiltext_layout_word* stlw_p, int* x_start_p, int* y_start_p, int* x_dir_p, int* y_dir_p);
	bool _smiltext_disjunct(const lib::rect& r1, const lib::rect& r2);
	bool _smiltext_fits(const lib::rect& r1, const lib::rect& r2);
//JNK	void _smiltext_render(const smil2::smiltext_run run, const lib::rect& r, const lib::point& p);
	unsigned int _compute_rate(lib::size size, lib::rect r, unsigned int dur);

	lib::critical_section m_lock;
	smiltext_engine m_engine;
	bool m_finished;
	bool m_process_lf;
	lib::event_processor *m_event_processor;
	lib::timer::time_type m_epoch;
	smiltext_params m_params;	// global parameters
	lib::rect m_dest_rect; // JNK
	smiltext_layout_provider* m_provider;
	bool m_needs_conditional_newline;
	bool m_needs_conditional_space;
	
	bool m_crawling;
	bool m_scrolling;
	lib::point m_shifted_origin; // moving origin for crawl/scroll
	lib::size m_size; 	     // space needed (for textRate="auto"

	std::vector<smiltext_layout_word> m_words;
};


} // namespace smil2
 
} // namespace ambulant

#endif // WITH_SMIL30

#endif // AMBULANT_SMIL2_SMILTEXT_H
