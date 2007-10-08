// This file is part of Ambulant Player, www.ambulantplayer.org.
//
// Copyright (C) 2003-2007 Stichting CWI, 
// Kruislaan 413, 1098 SJ Amsterdam, The Netherlands.
//
// Ambulant Player is free software; you can redistribute it and/or modify
// it under the terms of the GNU Lesser General Public License as published by
// the Free Software Foundation; either version 2.1 of the License, or
// (at your option) any later version.
//
// Ambulant Player is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU Lesser General Public License for more details.
//
// You should have received a copy of the GNU Lesser General Public License
// along with Ambulant Player; if not, write to the Free Software
// Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA  02110-1301  USA

/* 
 * @$Id$ 
 */

#include "ambulant/smil2/smiltext.h"
#include "ambulant/lib/callback.h"

//#define AM_DBG if(1)
#ifndef AM_DBG
#define AM_DBG if(0)
#endif

#ifdef WITH_SMIL30

namespace ambulant {
namespace smil2 {

typedef lib::no_arg_callback<smiltext_engine> update_callback;

smiltext_engine::smiltext_engine(const lib::node *n, lib::event_processor *ep, smiltext_notification *client, bool word_mode)
:	m_node(n),
	m_tree_iterator(n->begin()),
	m_event_processor(ep),
	m_client(client),
	m_word_mode(word_mode),
	m_newbegin_valid(false),
	m_process_lf(true),
	m_update_event(NULL)
{
	AM_DBG lib::logger::get_logger()->debug("smiltext_engine(0x%x).smiltext_engine(%s)", this, m_node->get_sig().c_str());
	// Initialize the iterators to the correct place
	m_tree_iterator++;
	m_newbegin = m_runs.end();
	m_newbegin_valid = false;
	
	// Initialize the global para
	// Initialize the default formatting and apply node attributes
	smiltext_run stdrun;
	stdrun.m_command = stc_data;
	_get_default_formatting(stdrun);
	_get_default_params(m_params);
	const char *rgn = n->get_attribute("region");
	if (rgn) {
		const lib::node *rgn_node = n->get_context()->get_node(rgn);
		if (rgn_node) {
			_get_formatting(stdrun, rgn_node);
			_get_params(m_params, rgn_node);
		}
	}
	_get_formatting(stdrun, n);
	_get_params(m_params, n);
	m_run_stack.push(stdrun);
}

smiltext_engine::~smiltext_engine()
{
	if (m_update_event&&m_event_processor)
		m_event_processor->cancel_event(m_update_event, lib::ep_med);
//	delete m_update_event;
	m_update_event = NULL;
	m_client = NULL;
	m_node = NULL;
}

/// Start the engine.
void
smiltext_engine::start(double t) {
	// XXX Need to allow for "t"
	AM_DBG lib::logger::get_logger()->debug("smiltext_engine(0x%x).start(%s)", this, m_node->get_sig().c_str());
	m_epoch = m_event_processor->get_timer()->elapsed();
	m_tree_time = 0;
	_update();
}

/// Seek the engine
void
smiltext_engine::seek(double t) {
	// To be provided
}

/// Stop the engine.
void
smiltext_engine::stop() {
	if (m_update_event&&m_event_processor)
		m_event_processor->cancel_event(m_update_event, lib::ep_med);
//	delete m_update_event;
	m_update_event = NULL;
	m_tree_iterator = m_node->end();
	m_runs.clear();
	m_newbegin = m_runs.end();
	m_newbegin_valid = false;
	m_node = NULL;
}

lib::xml_string
smiltext_engine::_split_into_lines(lib::xml_string data, size_t lf_pos, size_t limit) {
	if (lf_pos > 0) {
		smiltext_run run = m_run_stack.top();
		run.m_command = stc_data;
		run.m_data = data.substr(0, lf_pos);
		smiltext_runs::const_iterator where = m_runs.insert( m_runs.end(), run);
		if (!m_newbegin_valid) {
			m_newbegin = where;
			m_newbegin_valid = true;
		}
	}
	while (lf_pos++ < limit) {
		smiltext_run run = m_run_stack.top();
		run.m_command = stc_break;
		smiltext_runs::const_iterator where = m_runs.insert( m_runs.end(), run);
		if (!m_newbegin_valid) {
			m_newbegin = where;
			m_newbegin_valid = true;
		}
		if (data[lf_pos] != '\n') {
			break;  
		}
	}
	return data.substr(lf_pos);
}

void
smiltext_engine::_split_into_words(lib::xml_string data, smil2::smiltext_xml_space xml_space) {
	// same semantics as isspace, used by tree_builder
	std::string spacechar(" \f\n\r\t\v");
	size_t first_nonspace, first_char;
	while (data.length() > 0) {
		// find non-space after any leading space
		first_nonspace = data.find_first_not_of(spacechar);
		if (first_nonspace == std::string::npos)
			first_nonspace = data.length();
		// skip leading space if applicable
		first_char = xml_space == stx_preserve ? 0 : first_nonspace;
		// find trailing space
		size_t first_trailing_space = data.find_first_of(spacechar, first_nonspace);
		if (first_trailing_space == std::string::npos)
			first_trailing_space = data.length();
		if ((first_trailing_space - first_char) > 0) {
			size_t  lf_pos;
			if (m_process_lf && xml_space == stx_preserve
			    && (lf_pos = data.find('\n' != std::string::npos) )
			    && lf_pos <  first_nonspace) {
				// found line-feed characters in leading space
				data = _split_into_lines(data, lf_pos, first_nonspace);
				continue;
			}
			smiltext_run run = m_run_stack.top();
			run.m_command = stc_data;
			run.m_data = data.substr(first_char, first_trailing_space-first_char);
			AM_DBG lib::logger::get_logger()->debug("dx_smiltext_changed(): bg_col=0x%x, color=0x%x, data=%s", run.m_bg_color, run.m_color, run.m_data.c_str());
			smiltext_runs::const_iterator where = m_runs.insert( m_runs.end(), run);
			if (!m_newbegin_valid) {
				m_newbegin = where;
				m_newbegin_valid = true;
			}
			data = data.substr(first_trailing_space);
		} else {
			data = data.substr(data.length());
		}
	}
}

void
smiltext_engine::_update() {
	assert(m_node);
	lib::timer::time_type next_update_needed = 0;
	AM_DBG lib::logger::get_logger()->debug("smiltext_engine::_update()");
	m_update_event = NULL;
	for( ; !m_tree_iterator.is_end(); m_tree_iterator++) {
		assert(!m_run_stack.empty());
		const lib::node *item = (*m_tree_iterator).second;
		if (!(*m_tree_iterator).first) {
			// Pop the stack, if needed
			const lib::xml_string &tag = item->get_local_name();
			if (tag == "span" || tag == "pre")
				m_run_stack.pop();
			continue;
		}
		
		if (item->is_data_node()) {
			// Characters. Add them to the run with current font/size/etc
			smiltext_run run = m_run_stack.top();
			// Trim all space characters. BUT if there is whitespace at the
			// end leave one space there.
			lib::xml_string data = item->get_data();
			// XXX <pre>!
			if (m_word_mode) {
				_split_into_words(data, run.m_xml_space);
				continue;
			}
			run.m_data = data;
		} else {
			// Element. Check what it is.
			lib::xml_string tag = item->get_local_name();
			if (tag == "tev" || tag == "clear") {
				const char *time_str = item->get_attribute("begin");
				double time = 0;
				if (time_str) {
					time = atof(time_str); // XXXJACK
					m_tree_time = time;
				} else if (time_str = item->get_attribute("next")) {
					time = atof(time_str);
					m_tree_time = m_tree_time + time;
				} else  {
					lib::logger::get_logger()->trace("smiltext: tev without begin or next attribute ignored");
					continue;
				}
				lib::timer::time_type ttime = m_epoch + int(round(m_tree_time*1000));
				lib::timer::time_type now = m_event_processor->get_timer()->elapsed();
				//
				// If the node has an ID we raise a marker event. In the SMIL code this
				// is actually specified as a beginEvent but there's magic in the parsing
				// of the begin attribute to translate this to a marker event on the smiltext
				// node in case the beginEvent references a tev/clear.
				const char *id = item->get_attribute("id");
				if (id) {
					m_client->marker_seen(id);
				}
				if (tag == "clear" || m_params.m_mode == stm_replace) {
					m_runs.clear();
					m_newbegin = m_runs.end();
					m_newbegin_valid = false;
				}
				if (ttime > now) {
					next_update_needed = ttime-now;
					m_tree_iterator++;
					break;
				}
				// else this time has already passed and we continue the loop
			} else if (tag == "span") {
				smiltext_run run = m_run_stack.top();
				_get_formatting(run, item);
				m_run_stack.push(run);
			} else if (tag == "br") {
				smiltext_run run = m_run_stack.top();
				run.m_data = "";
				run.m_command = stc_break;
				smiltext_runs::const_iterator where = m_runs.insert( m_runs.end(), run);
				if (!m_newbegin_valid) {
					m_newbegin = where;
					m_newbegin_valid = true;
				}
			} else {
				lib::logger::get_logger()->trace("smiltext: unknown tag <%s>", tag.c_str());
			}
		}
	}
	if (m_client)
		m_client->smiltext_changed();
	if (m_params.m_rate > 0
	    && (m_params.m_mode == stm_crawl || m_params.m_mode == stm_jump
		|| m_params.m_mode == stm_scroll)) {
		// We need to schedule another update event to keep the scrolling/crawling going.
		// In principle we do a callback per pixel scrolled, but clamp at 25 per second.
		unsigned int delay = 1000 / m_params.m_rate;
		if (delay < 40) delay = 40;
		if (next_update_needed > delay || next_update_needed == 0) {
			next_update_needed = delay;
		}
	}
	if (next_update_needed > 0) {
		m_update_event = new update_callback(this, &smiltext_engine::_update);
		m_event_processor->add_event(m_update_event, next_update_needed, lib::ep_med);
	}
}

// Fill a run with the formatting parameters from a node.
void
smiltext_engine::_get_formatting(smiltext_run& dst, const lib::node *src)
{
	const char *style = src->get_attribute("textStyle");
	if (style) {
		const lib::node_context *ctx = src->get_context();
		assert(ctx);
		const lib::node *stylenode = ctx->get_node(style);
		if (stylenode) {
			// XXX check that stylenode.tag == textStyle
			_get_formatting(dst, stylenode);
		} else {
			lib::logger::get_logger()->trace("%s: textStyle=\"%s\": ID not found", src->get_sig().c_str(), style);
		}
	}
	const char *align = src->get_attribute("textAlign");
	if (align) {
		if (strcmp(align, "start") == 0) dst.m_align = sta_start;
		else if (strcmp(align, "end") == 0) dst.m_align = sta_end;
		else if (strcmp(align, "left") == 0) dst.m_align = sta_left;
		else if (strcmp(align, "right") == 0) dst.m_align = sta_right;
		else if (strcmp(align, "center") == 0) dst.m_align = sta_center;
		else if (strcmp(align, "inherit") == 0) /* no-op */;
		else {
			lib::logger::get_logger()->trace("%s: textAlign=\"%s\": unknown alignment", src->get_sig().c_str(), align);
		}
	}
	const char *bg_color = src->get_attribute("textBackgroundColor");
	if (bg_color) {
		dst.m_bg_transparent = false;
		dst.m_bg_color = lib::to_color(bg_color);
	}
	const char *color = src->get_attribute("textColor");
	if (color) {
		dst.m_transparent = false;
		dst.m_color = lib::to_color(color);
	}
	const char *direction = src->get_attribute("textDirection");
	if (direction) {
		if (strcmp(direction, "ltr") == 0) dst.m_direction = std_ltr;
		else if (strcmp(direction, "rtl") == 0) dst.m_direction = std_rtl;
		else if (strcmp(direction, "inherit") == 0) /* no-op */;
		else {
			lib::logger::get_logger()->trace("%s: textDirection=\"%s\": unknown direction", src->get_sig().c_str(), direction);
		}
		
	}
	const char *font_family = src->get_attribute("textFontFamily");
	if (font_family) {
		dst.m_font_family = font_family;
	}
	const char *font_size = src->get_attribute("textFontSize");
	if (font_size) {
		if (strcmp(font_size, "xx-small") == 0) dst.m_font_size = 8;
		else if (strcmp(font_size, "x-small") == 0) dst.m_font_size = 10;
		else if (strcmp(font_size, "small") == 0) dst.m_font_size = 12;
		else if (strcmp(font_size, "normal") == 0) dst.m_font_size = 14;
		else if (strcmp(font_size, "large") == 0) dst.m_font_size = 16;
		else if (strcmp(font_size, "x-large") == 0) dst.m_font_size = 18;
		else if (strcmp(font_size, "xx-large") == 0) dst.m_font_size = 20;
		else if (strcmp(font_size, "smaller") == 0) dst.m_font_size -= 2;
		else if (strcmp(font_size, "larger") == 0) dst.m_font_size += 2;
		else if (strcmp(font_size, "inherit") == 0) /* no-op */ ;
		else {
			lib::logger::get_logger()->trace("%s: textFontSize=\"%s\": unknown size", src->get_sig().c_str(), font_size);
		}
	}
	const char *font_weight = src->get_attribute("textFontWeight");
	if (font_weight) {
		if (strcmp(font_weight, "normal") == 0) dst.m_font_weight = stw_normal;
		else if (strcmp(font_weight, "bold") == 0) dst.m_font_weight = stw_bold;
		else if (strcmp(font_weight, "inherit") == 0) /* no-op */ ;
		else {
			lib::logger::get_logger()->trace("%s: textFontWeight=\"%s\": unknown weight", src->get_sig().c_str(), font_weight);
		}
	}
	const char *font_style = src->get_attribute("textFontStyle");
	if (font_style) {
		if (strcmp(font_style, "normal") == 0) dst.m_font_style = sts_normal;
		else if (strcmp(font_style, "italic") == 0) dst.m_font_style = sts_italic;
		else if (strcmp(font_style, "oblique") == 0) dst.m_font_style = sts_oblique;
		else if (strcmp(font_style, "reverseOblique") == 0) dst.m_font_style = sts_reverse_oblique;
		else if (strcmp(font_style, "inherit") == 0) /* no-op */ ;
		else {
			lib::logger::get_logger()->trace("%s: textFontStyle=\"%s\": unknown style", src->get_sig().c_str(), font_style);
		}
	}
	// xml:space attribute
	const char *xml_space = src->get_attribute("space");
	if (xml_space) {
		if (strcmp(xml_space, "preserve") == 0)     dst.m_xml_space = stx_preserve;
		else if (strcmp(xml_space, "default") == 0) dst.m_xml_space = stx_default;
		else if (strcmp(xml_space, "inherit") == 0) /* no-op */ ;
		else {
			lib::logger::get_logger()->trace("%s: xml:space=\"%s\": must be default or preserve", src->get_sig().c_str(), xml_space);
		}
	}
}

// Fill a run with the default formatting.
void
smiltext_engine::_get_default_formatting(smiltext_run& dst)
{
	dst.m_font_family = "monospace";
	dst.m_font_style = sts_normal;
	dst.m_font_weight = stw_normal;
	dst.m_font_size = 12;
	dst.m_transparent = false;
	dst.m_color = lib::color_t(0);
	dst.m_bg_transparent = true;
	dst.m_bg_color = lib::color_t(0);
	dst.m_align = sta_start;
	dst.m_direction = std_ltr;
}

// Fill smiltext_params from a node
void
smiltext_engine::_get_params(smiltext_params& params, const lib::node *src)
{
	const char *mode = src->get_attribute("textMode");
	if (mode) {
		if (strcmp(mode, "replace") == 0) params.m_mode = stm_replace;
		else if (strcmp(mode, "append") == 0) params.m_mode = stm_append;
		else if (strcmp(mode, "scroll") == 0) params.m_mode = stm_scroll;
		else if (strcmp(mode, "crawl") == 0) params.m_mode = stm_crawl;
		else if (strcmp(mode, "jump") == 0) params.m_mode = stm_jump;
		else if (strcmp(mode, "inherit") == 0) /* no-op */ ;
		else {
			lib::logger::get_logger()->trace("%s: textMode=\"%s\": unknown mode", src->get_sig().c_str(), mode);
		}
	}
	const char *loop = src->get_attribute("textLoop");
	if (loop) {
		if (strcmp(loop, "true") == 0) params.m_loop = true;
		else if (strcmp(loop, "false") == 0) params.m_loop = false;
		else if (strcmp(loop, "inherit") == 0) /* no-op */ ;
		else {
			lib::logger::get_logger()->trace("%s: textLoop=\"%s\": must be true or false", src->get_sig().c_str(), loop);
		}
	}
	const char *rate = src->get_attribute("textRate");
	if (rate) {
		int rate_i = atoi(rate); // XXXX
		params.m_rate = rate_i;
	}
	const char *wrap = src->get_attribute("textWrapOption");
	if (wrap) {
		if (strcmp(wrap, "wrap") == 0) params.m_wrap = true;
		else if (strcmp(wrap, "noWrap") == 0) params.m_wrap = false;
		else if (strcmp(wrap, "inherit") == 0) /* no-op */ ;
		else {
			lib::logger::get_logger()->trace("%s: textWrapOption=\"%s\": must be wrap or noWrap", src->get_sig().c_str(), wrap);
		}
	}
}

// Fill default smiltext_params
void
smiltext_engine::_get_default_params(smiltext_params& params)
{
	params.m_mode = stm_append;
	params.m_loop = false;
	params.m_rate = 0;
	params.m_wrap = true;
}

// smiltext_layout_engine
smiltext_layout_engine::smiltext_layout_engine(const lib::node *n, lib::event_processor *ep, smiltext_layout_provider* provider, smiltext_notification* client, bool process_lf)
  :	m_engine(smiltext_engine(n, ep, client, true)),
	m_finished(false),
	m_process_lf(process_lf),
	m_event_processor(ep),
	m_provider(provider),
	m_params(m_engine.get_params()),
	m_dest_rect()
{
}

void
smiltext_layout_engine::start(double t) {
//	m_lock.enter();
	m_epoch = m_event_processor->get_timer()->elapsed();
	m_engine.start(t);
//	m_lock.leave();
}
	
void
smiltext_layout_engine::seek(double t) {
	m_lock.enter();
	m_engine.seek(t);
	m_lock.leave();
}
	
void
smiltext_layout_engine::stop() {
	m_lock.enter();
	m_engine.stop();
	m_lock.leave();
}
	
void
smiltext_layout_engine::set_dest_rect( const lib::rect& r) {
	m_lock.enter();
	m_dest_rect = r;
	m_lock.leave();
}

#ifdef	NEW_LAYOUT_ENGINE
#else //NEW_LAYOUT_ENGINE
#endif//NEW_LAYOUT_ENGINE

#ifdef	NEW_LAYOUT_ENGINE
smiltext_layout_word::smiltext_layout_word(smiltext_run run, smiltext_metrics stm, int nbr)
  :	m_run(run),
	m_leading_breaks(nbr),
	m_metrics(stm)
{
	m_bounding_box = lib::rect(lib::point(0,0),
				   lib::size(stm.get_width(), 
					     stm.get_height()));
}

void
smiltext_layout_engine::smiltext_changed() {
	m_lock.enter();
	AM_DBG lib::logger::get_logger()->debug("smiltext_layout_engine::smiltext_changed(0x%x)", this);
	if (m_engine.is_changed()) {
		lib::xml_string data;
		smil2::smiltext_runs::const_iterator i;

		if (m_engine.is_cleared()) {
			// Completely new text, clear the copy.
	        	m_words.clear();
			i = m_engine.begin();
		} else {
			// Only additions. Don't clear, store new stuff.
			i = m_engine.newbegin();
		}
		int nbr = 0;
		while (i != m_engine.end()) {
			switch (i->m_command) {
			case stc_break:
				if (m_params.m_mode != stm_crawl )
					nbr++;
				break;
			case stc_data:
				smiltext_metrics stm =
					m_provider->get_smiltext_metrics (*i);
				smiltext_layout_word word_info(*i, stm, nbr);
				m_words.push_back(word_info);
				nbr = 0;	
			}			
			i++;
		}
	}
	m_lock.leave();
}

void
smiltext_layout_engine::get_initial_values(//JNK lib::point shifted_origin,
					   lib::rect rct,
					   smiltext_layout_word* stlw_p,
					   int* x_start_p,
					   int* y_start_p, 
					   int* x_dir_p,
					   int* y_dir_p) {
	switch (stlw_p->m_run.m_direction) {
	default:
	case std_ltr:
		*x_start_p = rct.left();
		*x_dir_p = 1;
		break;
	case std_rtl:
		// in right-to-left mode, everything is computed
		//  w.r.t. the right-top corner of the bounding box
		*x_start_p = rct.right();
		*x_dir_p = -1;
		break;
	}
	*y_start_p = rct.top();
	/* impl. textPlace attribute/
	switch (i->m_run.m_place) {
	default:
	case stp_from_top:
		*y_dir_p = 1;
		*y_start_p = r.top() + shifted_origin.y;
		break;
	case stp_from_bottom:
		//*y_dir_p = -1;
		*y_dir_p = 1;
		*y_start_p = r.bottom() + shifted_origin.y
				- stlw_p->m_bounding_box.h;
		break;
	}
	*/
}

void
smiltext_layout_engine::redraw(const lib::rect& r) {
	AM_DBG lib::logger::get_logger()->debug("smiltext_layout_engine::redraw(0x%x) r=(L=%d,T=%d,W=%d,H=%d", this,r.left(),r.top(),r.width(),r.height());
	int nbr = 0; // number of breaks (newlines) before current line
	m_lock.enter();

	int x_start = 0, y_start = 0, x_dir = 1, y_dir = 1;
	get_initial_values(r, &*m_words.begin(),
			   &x_start, &y_start, &x_dir, &y_dir);
	smil2::smiltext_align align = m_words.begin()->m_run.m_align;
	smil2::smiltext_direction direction = m_words.begin()->m_run.m_direction;
	if (direction == std_rtl)
	  switch (align) {
	  case sta_start:
	  	align = sta_right;
		break;
	  case sta_end:
	  	align = sta_left;
	  default:
	  	break;
	  }
	// Compute the shifted position of what we want to draw w.r.t. the visible origin
	lib::point shifted_origin(0, 0);
	if (m_params.m_mode == smil2::stm_crawl) {
		long int elapsed = m_event_processor->get_timer()->elapsed();
		double now = elapsed - m_epoch;
		shifted_origin.x += (int) now * m_params.m_rate / 1000 * x_dir;
		if (shifted_origin.x < 0)
			AM_DBG lib::logger::get_logger()->debug("smiltext_layout_engine::redraw(0x%x): strange: shifted_x=%d, m_epoch=%ld, elpased=%ld !", this, shifted_origin.x, m_epoch, elapsed);
		switch (align) {
		default:
		case sta_start:
		case sta_left:
			break;
		case sta_center:
			x_start = r.left() + (r.right() - r.left()) 
							/ 2;
			y_start = r.top() + (r.bottom() - r.top())
							/ 2
				- (m_words.begin()->m_bounding_box.height()) 
						/ 2;
			break;
		case sta_right:
		case sta_end:
			if (direction == std_rtl)
				x_start = r.left();
			else	x_start = r.right();
			break;
		}
	}
	if (m_params.m_mode == smil2::stm_scroll) {
		long int elapsed = m_event_processor->get_timer()->elapsed();
		double now = elapsed - m_epoch;
		shifted_origin.y += (int) now * m_params.m_rate / 1000 * y_dir;
	}
	AM_DBG lib::logger::get_logger()->debug("smiltext_layout_engine::redraw: shifted_origin(%d,%d)", shifted_origin.x, shifted_origin.y);

	bool linefeed_processing = m_params.m_mode != stm_crawl;

	//TBD implement Align, Direction, Place, etc. by giving
	// x_start, y_start, x_dir, y_dir proper initial values
	int prev_max_ascent = 0, prev_max_descent = 0; 
	std::vector<smiltext_layout_word>::iterator bol,// begin of line
						    eol,// end of line
						    word;
	for (bol = m_words.begin(); bol != m_words.end(); bol = eol) {
		unsigned int max_ascent = 0, max_descent = 0;
		int x = x_start;
		int y = y_start;
		bool first_word = true;

		// find end of line
		for (word = bol; word != m_words.end(); word++) {
			if (word != bol && word->m_leading_breaks != 0)
				break;
			// for each word on this line see if it fits
			// for rtl, x==word->m_bounding_box.right()
			if (word->m_run.m_direction == std_rtl)
				word->m_bounding_box.x =
					 x - word->m_bounding_box.w;
			else	word->m_bounding_box.x = x;
			word->m_bounding_box.y = y;
			// first word in a line is shown always,
			// because otherwise nothing would be shown:
			// if it doesn't fit on this line it won't fit
			// on the next line in the rectangle either
			if (linefeed_processing
			    && ! first_word
			    && m_params.m_wrap 
			    &&  ! smiltext_fits(word->m_bounding_box,r)) {
				if (word->m_leading_breaks == 0)
					word->m_leading_breaks++;
				break;
			}
			first_word = false;
			// compute x-position of next word
			x += (word->m_metrics.get_width() +
			      word->m_metrics.get_word_spacing()
			      ) * x_dir;
			// find max. height (ascent+descent) of all words
			if (word->m_metrics.get_ascent() > max_ascent)
				max_ascent = 
					word->m_metrics.get_ascent();
			if (word->m_metrics.get_descent() > max_descent)
				max_descent = 
					word->m_metrics.get_descent();
		}
		eol = word;
		std::vector<smiltext_layout_word>::iterator lwl =
					word - 1;  // last word on line
		// alignment processing
		int x_align = 0, x_min, x_max;
		if (bol->m_run.m_direction == std_rtl) {
	        	x_min = lwl->m_bounding_box.left();
			x_max = bol->m_bounding_box.right();
		} else {
			x_min = bol->m_bounding_box.left();
			x_max = lwl->m_bounding_box.right();
		}
		switch (align) {
		default:
		case sta_start:
		case sta_left:
			if (direction == std_rtl
			    && x_min > r.left())
		  		x_align = r.left() - x_min;
			break;
		case sta_center:
			if (direction == std_ltr) {
				if (x_max < r.right())
					x_align = (r.right() - x_max)
							  / 2;
			} else {
				if (x_min > r.left())
					x_align = (r.left() - x_min)
							  / 2;
			}
			break;
		case sta_end:
		case sta_right:
			if (direction == std_ltr
			    && x_max < r.right())
		  		x_align = r.right() - x_max;
			break;
		}
		if (linefeed_processing) {
			// if a run starts with blank lines, take the
			// height of the first line as line distance 
			if (prev_max_ascent == 0)
				prev_max_ascent = max_ascent;
			if (prev_max_descent == 0)
				prev_max_descent = max_descent;
			// compute y-position of next line
			y_start += (prev_max_ascent + prev_max_descent) *
				bol->m_leading_breaks * y_dir;
		}
		for (word = bol; word != eol; word++) {
			// alignment correction
			word->m_bounding_box.x += x_align;
			// baseline correction
			word->m_bounding_box.y = 
				y_start + max_ascent -
				word->m_metrics.get_ascent();
		}
		prev_max_ascent = max_ascent;
		prev_max_descent = max_descent;
	}
	// layout done, render the run
	for (word = m_words.begin(); word != m_words.end(); word++) {
		int word_spacing = 0;
		if (word != m_words.begin()) {
			word_spacing = word->m_metrics.get_word_spacing();
		}
		word->m_bounding_box -= shifted_origin;
		if (smiltext_disjunct (word->m_bounding_box, r))
			continue; // nothing to de displayed
		m_provider->render_smiltext(word->m_run,
					    word->m_bounding_box,
					    word_spacing);
	}
	m_lock.leave();
}

// return true if r1 completely fits horizontally in r2
bool
smiltext_layout_engine::smiltext_fits(const lib::rect& r1, const lib::rect& r2) {
  return r2.left() <=  r1.left() && r1.right() <= r2.right();
}

// return true if r1 and r2 have a zero intersection
bool
smiltext_layout_engine::smiltext_disjunct(const lib::rect& r1, const lib::rect& r2) {
	return (r1 & r2) == lib::rect();
}


#else //NEW_LAYOUT_ENGINE

void
smiltext_layout_engine::redraw(const lib::rect& r) {
	AM_DBG lib::logger::get_logger()->debug("smiltext_layout_engine::redraw(0x%x) r=(L=%d,T=%d,W=%d,H=%d", this,r.left(),r.top(),r.width(),r.height());
	int nbr = 0; // number of breaks (newlines) before current line

	// Compute the shifted position of what we want to draw w.r.t. the visible origin
	lib::point logical_origin(0, 0);
	if (m_params.m_mode == smil2::stm_crawl) {
		long int elapsed = m_event_processor->get_timer()->elapsed();
		double now = elapsed - m_epoch;
		logical_origin.x += (int) now * m_params.m_rate / 1000;
		if (logical_origin.x < 0)
			AM_DBG lib::logger::get_logger()->debug("smiltext_layout_engine::redraw(0x%x): strange: logical_x=%d, m_epoch=%ld, elpased=%ld !", this, logical_origin.x, m_epoch, elapsed);
	}
	if (m_params.m_mode == smil2::stm_scroll) {
		long int elapsed = m_event_processor->get_timer()->elapsed();
		double now = elapsed - m_epoch;
		logical_origin.y += (int) now * m_params.m_rate / 1000;
	}
	AM_DBG lib::logger::get_logger()->debug("smiltext_layout_engine::redraw: logical_origin(%d,%d)", logical_origin.x, logical_origin.y);

	// Always re-compute and re-render everything when new text is added.
	// E.g. word with bigger font may need adjustment of prior text 
	// Therefore, m_engine.newbegin() is can NOT be used
	smil2::smiltext_runs::const_iterator cur = m_engine.begin();
	lib::rect ldr = m_dest_rect; // logical destination rectangle

	AM_DBG lib::logger::get_logger()->debug("smiltext_layout_engine::redraw(0x%x): ldr=(L=%d,T=%d,W=%d,H=%d", this,ldr.left(),ldr.top(),ldr.width(),ldr.height());

	m_y = ldr.y;
	m_max_ascent = m_max_descent = 0;
	// count number of initial breaks before first line
	while (cur->m_command == smil2::stc_break) {
		nbr++;
		cur++;
	}
	while (cur != m_engine.end()) {
		// compute layout of next line
		smil2::smiltext_runs::const_iterator bol = cur; // begin of line pointer
		bool fits = true;
		m_x = ldr.x;
		if (nbr > 0 && (m_max_ascent != 0 || m_max_descent != 0)) {
			// correct m_y for proper size of previous line
			m_y += (m_max_ascent + m_max_descent);
			nbr--;
		}
		m_max_ascent = m_max_descent = 0;
		AM_DBG lib::logger::get_logger()->debug("smiltext_layout_engine::redraw(): command=%d data=%s",cur->m_command,cur->m_data.c_str()==NULL?"(null)":cur->m_data.c_str());
		while (cur != m_engine.end()) {
			fits = smiltext_fits(*cur, m_dest_rect);
			if ( ! fits || cur->m_command == smil2::stc_break)
				break;
			cur++;
		}
		m_x = ldr.x; // m_x was modified by smiltext_fits()
		// move down number of breaks times height of current line
		m_y += (m_max_ascent + m_max_descent) * nbr;
		if ((m_y-logical_origin.y) > m_dest_rect.bottom())
			// line falls outside visible rect
			break;
		// count number of breaks in front of next line
		nbr = 0;
		while (cur != m_engine.end() && cur->m_command == smil2::stc_break) {
			nbr++;
			cur++;
		}
		if ( ! fits && nbr == 0)
			nbr = 1;
		bool initial = true;
		while (bol != cur) {
			assert(bol != m_engine.end());
			// compute rectangle where to render this text
			unsigned int word_spacing;
			lib::rect cr = smiltext_compute(*bol, r, &word_spacing);
			cr.x -= logical_origin.x;
			cr.y -= logical_origin.y;
			AM_DBG lib::logger::get_logger()->debug("smiltext_layout_engine::redraw(): x=%d\ty=%d\tcommand=%d data=%s", cr.x, cr.y, bol->m_command,bol->m_data.c_str()==NULL?"(null)":bol->m_data.c_str());
			m_provider->render_smiltext(*bol, cr, initial?0:word_spacing);
			if (initial)
				initial = false;
			bol++;
		}
	}
	m_engine.done();
	m_finished = m_engine.is_finished();
}

bool
smiltext_layout_engine::smiltext_fits(const smil2::smiltext_run strun, const lib::rect& r) {

	smiltext_metrics stm =  m_provider->get_smiltext_metrics (strun);

	int x = m_x;
	x += stm.get_word_spacing();
	x += stm.get_width();
	if (x > r.right())
		return false;
	m_x = x;
 	if (stm.get_ascent() > m_max_ascent)
		m_max_ascent = stm.get_ascent();
	if (stm.get_descent() > m_max_descent)
		m_max_descent = stm.get_descent();
	return true;
}
lib::rect
smiltext_layout_engine::smiltext_compute(const smil2::smiltext_run strun, const lib::rect& r, unsigned int* word_spacing) {

	lib::rect rv = r;
	smiltext_metrics stm = m_provider->get_smiltext_metrics (strun);
	*word_spacing = stm.get_word_spacing();
	rv.x += m_x + *word_spacing;
	rv.w = stm.get_width();
	m_x  += rv.w + *word_spacing;

	rv.y += (m_y + m_max_ascent - stm.get_ascent());
	rv.h = stm.get_height();
	/* clip rectangle */
	if (rv.x >= r.x + (int) r.w)
		rv.w = 0;
	else if (rv.x + (int) rv.w > r.x + (int) r.w)
		rv.w = (unsigned int) (r.x + (int) r.w - rv.x);
	if (rv.y >= r.y + (int)r.h)
		rv.h = 0;
	else if (rv.y + (int)rv.h > r.y + (int)r.h)
		rv.h = (unsigned int) (r.y + (int)r.h - rv.y);

	AM_DBG lib::logger::get_logger()->debug("smiltext_compute(): x=%d\ty=%d\tcommand=%d data=%s", rv.x, rv.y, strun.m_command, strun.m_data.c_str()==NULL?"(null)":strun.m_data.c_str());

	return rv;
}
#endif//NEW_LAYOUT_ENGINE



}
}
#endif // WITH_SMIL30
