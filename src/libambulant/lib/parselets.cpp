/*
 * 
 * This file is part of Ambulant Player, www.ambulantplayer.org.
 * 
 * Copyright (C) 2003 Stiching CWI, 
 * Kruislaan 413, 1098 SJ Amsterdam, The Netherlands.
 * 
 * Ambulant Player is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 * 
 * Ambulant Player is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 * 
 * You should have received a copy of the GNU General Public License
 * along with Ambulant Player; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 * 
 * In addition, as a special exception, if you link Ambulant Player with
 * other files to produce an executable, this library does not by itself
 * cause the resulting executable to be covered by the GNU General Public
 * License. This exception does not however invalidate any other reason why
 * the executable file might be covered by the GNU General Public License.
 * 
 * As a special exception, the copyright holders of Ambulant Player give
 * you permission to link Ambulant Player with independent modules that
 * communicate with Ambulant Player solely through the region and renderer
 * interfaces, regardless of the license terms of these independent
 * modules, and to copy and distribute the resulting combined work under
 * terms of your choice, provided that every copy of the combined work is
 * accompanied by a complete copy of the source code of Ambulant Player
 * (the version of Ambulant Player used to produce the combined work),
 * being distributed under the terms of the GNU General Public License plus
 * this exception.  An independent module is a module which is not derived
 * from or based on Ambulant Player.
 * 
 * Note that people who make modified versions of Ambulant Player are not
 * obligated to grant this special exception for their modified versions;
 * it is their choice whether to do so.  The GNU General Public License
 * gives permission to release a modified version without this exception;
 * this exception also makes it possible to release a modified version
 * which carries forward this exception. 
 * 
 */

/* 
 * @$Id$ 
 */
 
//#define AM_DBG
#ifndef AM_DBG
#define AM_DBG if(0)
#endif

#include "ambulant/lib/parselets.h"
#include "ambulant/lib/logger.h"
#include "ambulant/lib/string_util.h"
#include <math.h>

using namespace ambulant;

//////////////////////
// and_p

std::ptrdiff_t 
lib::list_p::parse(const_iterator& it, const const_iterator& end) {
	const_iterator test_it = it;
	std::ptrdiff_t sd = 0;
	std::list<parselet*>::iterator rit;
	for(rit = m_result.begin(); rit !=  m_result.end(); rit++) {
		std::ptrdiff_t d = (*rit)->parse(test_it, end);
		if(d == -1) return -1;
		sd += d;
	}
	it = test_it;
	return sd;
}

lib::list_p::~list_p() {
	std::list<parselet*>::iterator rit;
	for(rit = m_result.begin(); rit !=  m_result.end(); rit++) 
		delete *rit;
}
	
//////////////////////
// options_p

std::ptrdiff_t 
lib::options_p::parse(const_iterator& it, const const_iterator& end) {
	const_iterator test_it = it;
	
	std::ptrdiff_t dmax = -1;
	
	std::ptrdiff_t *aptrdiff = new ptrdiff_t[m_options.size()];
		
	std::list<parselet*>::iterator rit;
	int ix = 0;
	for(rit = m_options.begin(); rit !=  m_options.end(); rit++, ix++)
		aptrdiff[ix] = (*rit)->parse(test_it, end);
	
	for(rit = m_options.begin(), ix=0; rit !=  m_options.end(); rit++, ix++) {
		std::ptrdiff_t cur = aptrdiff[ix];
		if(cur != -1 && (dmax == -1 || cur > dmax)) {
			m_result = *rit;
			dmax = cur;
		}
	}
	for(std::ptrdiff_t d=0;d<dmax;d++) it++;
	return dmax;
}

lib::options_p::~options_p() {
	std::list<parselet*>::iterator rit;
	for(rit = m_options.begin(); rit !=  m_options.end(); rit++) 
		delete *rit;
}

//////////////////////
// number_p
// Parses any decimal number not using scientific notation
// Includes: (+|-)?d+(.d*)? | .d+

std::ptrdiff_t 
lib::number_p::parse(const_iterator& it, const const_iterator& end) {
	const_iterator tit = it;
	std::ptrdiff_t d;
	std::ptrdiff_t sd = 0;
	bool needs_fraction = false;
	
	delimiter_p space(" \t\r\n");
	star_p<delimiter_p> opt_space_inst = make_star(space);

	// Pass over any optional space
	d = opt_space_inst.parse(tit, end);
	sd += (d == -1)?0:d;
	
	// Parse optional sign
	delimiter_p sign_inst("+-");
	d = sign_inst.parse(tit, end);
	int sign = (d == -1)?1:( (sign_inst.m_result == '+')?1:-1);
	sd += (d == -1)?0:d;
	
	// Pass over any optional space following sign
	if(d != -1) {
		d = opt_space_inst.parse(tit, end);
		sd += (d == -1)?0:d;
	}
		
	int_p i;
	if(*tit != '.') {
		// the number does not start with '.'
		d = i.parse(tit, end);
		if(d == -1) return -1;
		sd += d;
		d = literal_p<'.'>().parse(tit, end);
		if(d == -1) return (m_result = sign*i.m_result, it = tit, sd);
		sd += d;
	} else {
		// the number starts with '.'
		i.m_result = 0;
		tit++; sd += 1;
		needs_fraction = true;
	}
		
	// get the fractional part if it exists 
	// (may be mandatory if the num starts with '.')
	int_p f;
	d = f.parse(tit, end);
	if(d == -1) {
		 if(needs_fraction) return -1;
		 else f.m_result = 0;
	}
	sd += (d == -1)?0:d;
	m_result = sign*(i.m_result + double(f.m_result)/::pow(10.0, int(d)));
	return (it = tit, sd);
}

//////////////////////
// number_list_p
// Parses a list of numbers
// The list is sepatated with white space
// The parser stops to the first not number sequence or at end

std::ptrdiff_t 
lib::number_list_p::parse(const_iterator& it, const const_iterator& endit) {
	m_result.clear();
	const_iterator tit = it;
	std::ptrdiff_t sd = 0;
	std::ptrdiff_t d = 0;
	number_p nparser;
	while(d != -1) {
		d = nparser.parse(tit, endit);
		if(d != -1) {
			sd += d;
			m_result.push_back(nparser.m_result);
		}
	}
	if(m_result.empty()) return -1;
	return (it = tit, sd);
}

//////////////////////
// time_unit_p

std::ptrdiff_t 
lib::time_unit_p::parse(const_iterator& it, const const_iterator& end) {
	literal_cstr_p h_p("h");
	literal_cstr_p min_p("min");
	literal_cstr_p s_p("s");
	literal_cstr_p ms_p("ms");
	const_iterator tit = it;
	std::ptrdiff_t d;
	if((d = h_p.parse(tit, end)) != -1) return (m_result = tu_h, it = tit, 1);
	else if((d = min_p.parse(tit, end)) != -1) return (m_result = tu_min, it = tit, 3);
	else if((d = ms_p.parse(tit, end)) != -1) return (m_result = tu_ms, it = tit, 2);
	else if((d = s_p.parse(tit, end)) != -1) return (m_result = tu_s, it = tit, 1);
	return -1;
}

//////////////////////
// length_unit_p

std::ptrdiff_t 
lib::length_unit_p::parse(const_iterator& it, const const_iterator& end) {
	literal_cstr_p px_p("px");
	literal_cstr_p percent_p("%");
	const_iterator tit = it;
	std::ptrdiff_t d;
	if((d = px_p.parse(tit, end)) != -1) return (m_result = px, it = tit, 2);
	else if((d = percent_p.parse(tit, end)) != -1) return (m_result = percent, it = tit, 1);
	return -1;
}

//////////////////////
// full_clock_value_p

std::ptrdiff_t 
lib::full_clock_value_p::parse(const_iterator& it, const const_iterator& end) {
	const_iterator tit = it;
	std::ptrdiff_t d;
	std::ptrdiff_t sd = 0;
		
	// hours
	int_p hours;
	d = hours.parse(tit, end);
	if(d == -1) return -1;
	sd += d;
	m_result.hours = hours.m_result;
	
	// :
	d = literal_p<':'>().parse(tit, end);
	if(d == -1) return -1;
	sd += d;
	
	// minutes
	int_p minutes;
	d = minutes.parse(tit, end);
	if(d == -1) return -1;
	sd += d;
	m_result.minutes = minutes.m_result;
	
	// :
	d = literal_p<':'>().parse(tit, end);
	if(d == -1) return -1;
	sd += d;
	
	// seconds
	int_p seconds;
	d = seconds.parse(tit, end);
	if(d == -1) return -1;
	sd += d;
	m_result.seconds = seconds.m_result;
	
	/////////////////
	// optional part
	// . 
	d = literal_p<'.'>().parse(tit, end);
	if(d == -1) {
		m_result.fraction = -1;
		it = tit;
		return sd;
	}
	sd += d;
	
	// fraction
	int_p fraction;
	d = fraction.parse(tit, end);
	if(d == -1) return -1;
	sd += d;
	m_result.fraction = fraction.m_result;
	it = tit;
	return sd;
}

//////////////////////
// partial_clock_value_p

std::ptrdiff_t 
lib::partial_clock_value_p::parse(const_iterator& it, const const_iterator& end) {
	const_iterator tit = it;
	std::ptrdiff_t d;
	std::ptrdiff_t sd = 0;
	
	// minutes
	int_p minutes;
	d = minutes.parse(tit, end);
	if(d == -1) return -1;
	m_result.minutes = minutes.m_result;
	sd += d;
	
	// :
	d = literal_p<':'>().parse(tit, end);
	if(d == -1) return -1;
	sd += d;
	
	// seconds
	int_p seconds;
	d = seconds.parse(tit, end);
	if(d == -1) return -1;
	m_result.seconds = seconds.m_result;
	sd += d;
	
	/////////////////
	// optional part
	// . 
	d = literal_p<'.'>().parse(tit, end);
	if(d == -1) {
		m_result.fraction = -1;
		it = tit;
		return sd;
	}
	sd += d;
	
	// fraction
	int_p fraction;
	d = fraction.parse(tit, end);
	if(d == -1) return -1;
	m_result.fraction = fraction.m_result;
	sd += d;
	it = tit;
	return sd;
}

//////////////////////
// timecount_value_p

std::ptrdiff_t 
lib::timecount_value_p::parse(const_iterator& it, const const_iterator& end) {
	const_iterator tit = it;
	std::ptrdiff_t d;
	std::ptrdiff_t sd = 0;
	
	// parse value
	number_p p1;
	d = p1.parse(tit, end);
	if(d == -1) return -1;
	m_result.value = p1.m_result;
	sd += d;
	
	// parse optional units	
	time_unit_p p2;
	d = p2.parse(tit, end);
	if(d == -1) return (m_result.unit = time_unit_p::tu_s, it = tit, sd);
	sd += d;
	return (m_result.unit = p2.m_result, it = tit, sd);
}

//////////////////////
// clock_value_p and converter to ms

inline int fraction_to_ms(int f) {
	return (f<=0)?0:int(::floor(1000.0*f + 0.5));
}

std::ptrdiff_t 
lib::clock_value_p::parse(const_iterator& it, const const_iterator& end) {
	lib::clock_value_sel_p p = make_or_trio_p(full_clock_value_p(), 
			partial_clock_value_p(), timecount_value_p());
	const_iterator test_it = it;
	std::ptrdiff_t d = p.parse(test_it, end);
	if(d == -1) return -1;
	if(p.matched_first()) {
		full_clock_value_p::result_type r = p.get_first_result();
		m_result = r.hours * H_MS + r.minutes * MIN_MS + r.seconds * S_MS + fraction_to_ms(r.fraction);
	} else if(p.matched_second()) {
		partial_clock_value_p::result_type r = p.get_second_result();
		m_result = r.minutes * MIN_MS + r.seconds * S_MS + fraction_to_ms(r.fraction);
	} else if(p.matched_third()) {
		timecount_value_p::result_type r = p.get_third_result();
		switch(r.unit) {
			case time_unit_p::tu_h:
				m_result = long(::floor(r.value * H_MS + 0.5));
				break;
			case time_unit_p::tu_min:
				m_result = long(::floor(r.value * MIN_MS + 0.5));
				break;
			case time_unit_p::tu_s:
				m_result = long(::floor(r.value * S_MS + 0.5));
				break;
			case time_unit_p::tu_ms:
				m_result = long(::floor(r.value + 0.5));
				break;
		}
	} else {
		logger::get_logger()->trace("Internal error: clock_value_p logic error");
		logger::get_logger()->warn(gettext("Programmer error encountered, will attempt to continue"));
	}	
	it = test_it;
	return d;
}

//////////////////////
// offset_value_p and converter to ms
// offset-value ::= (( S? "+" | "-" S? )? ( Clock-value )

std::ptrdiff_t 
lib::offset_value_p::parse(const_iterator& it, const const_iterator& end) {
		const_iterator test_it = it;
		std::ptrdiff_t rd = 0;
		std::ptrdiff_t d;
		
		delimiter_p space(" \t\r\n");
		star_p<delimiter_p> opt_space_inst = make_star(space);

		d = opt_space_inst.parse(test_it, end);
		rd += (d == -1)?0:d;
		
		delimiter_p sign_inst("+-");
		d = sign_inst.parse(test_it, end);
		int sign = (d == -1)?1:( (sign_inst.m_result == '+')?1:-1 );
		rd += (d == -1)?0:d;
	
		d = opt_space_inst.parse(test_it, end);
		rd += (d == -1)?0:d;
		
		clock_value_p c;
		d = c.parse(test_it, end);
		if(d == -1) return -1;
		rd += d;
		it = test_it;
		m_result = sign*c.m_result;
		return rd;
}

//////////////////////
// coord_p

std::ptrdiff_t 
lib::coord_p::parse(const_iterator& it, const const_iterator& end) {
	const_iterator tit = it;
	std::ptrdiff_t d;
	std::ptrdiff_t sd = 0;
	
	// we need this if we want to allow optional space between value and units
	delimiter_p space(" \t\r\n");
	star_p<delimiter_p> opt_space_inst = make_star(space);
		
	// parse value
	number_p p1;
	d = p1.parse(tit, end);
	if(d == -1) return -1;
	m_result.value = p1.m_result;
	sd += d;
	
	// allow optional space between value and units
	d = opt_space_inst.parse(tit, end);
	sd += (d == -1)?0:d;
	
	// parse optional units	
	length_unit_p p2;
	d = p2.parse(tit, end);
	if(d == -1) return (m_result.unit = length_unit_p::px, it = tit, sd);
	sd += d;
	return (m_result.unit = p2.m_result, it = tit, sd);
}

//////////////////////
// region_dim_p

std::ptrdiff_t 
lib::region_dim_p::parse(const_iterator& it, const const_iterator& end) {
	const_iterator tit = it;
	std::ptrdiff_t d;
	std::ptrdiff_t sd = 0;
	
	delimiter_p space(" \t\r\n");
	star_p<delimiter_p> opt_space_inst = make_star(space);
		
	// S?
	d = opt_space_inst.parse(tit, end);
	sd += (d == -1)?0:d;
	
	// int | double
	number_p val_p;
	d = val_p.parse(tit, end);
	if(d == -1) return -1;
	sd += d;
	m_result.dbl_val = val_p.m_result;
	
	// S?
	d = opt_space_inst.parse(tit, end);
	sd += (d == -1)?0:d;
	
	// (px)? | %
	literal_cstr_p px_cstr_p("px");
	literal_cstr_p pc_cstr_p("%");
	or_pair_p<literal_cstr_p, literal_cstr_p> type_p(px_cstr_p, pc_cstr_p);
	d = type_p.parse(tit, end);
	if(d == -1 || type_p.matched_first()) {
		// interpret as int
		m_result.relative = false;
		m_result.int_val = int(floor(0.5+m_result.dbl_val));
	} else /*if(type_p.matched_second())*/ {
		m_result.relative = true;
		m_result.dbl_val *= 0.01;
	}
	sd += (d == -1)?0:d;
	return (it = tit, sd);
}

//////////////////////
// point_p

// S? (? d+ S? , S? d+  S? )?
std::ptrdiff_t 
lib::point_p::parse(const_iterator& it, const const_iterator& end) {
	const_iterator tit = it;
	std::ptrdiff_t d;
	std::ptrdiff_t sd = 0;
	
	delimiter_p space(" \t\r\n");
	star_p<delimiter_p> opt_space_inst = make_star(space);
	
	// S?
	d = opt_space_inst.parse(tit, end);
	sd += (d == -1)?0:d;
	
	// (?
	d = literal_p<'('>().parse(tit, end);
	sd += (d == -1)?0:d;
	bool expectRP = (d == -1)?false:true;
	
	// x value
	int_p ip;
	d = ip.parse(tit, end);
	if(d == -1) return -1;
	m_result.x = ip.m_result;
	sd += d;
	
	// S?
	d = opt_space_inst.parse(tit, end);
	sd += (d == -1)?0:d;
	
	// value sep opt
	d = literal_p<','>().parse(tit, end);
	sd += (d == -1)?0:d;

	// S?
	d = opt_space_inst.parse(tit, end);
	sd += (d == -1)?0:d;
	
	// y value
	d = ip.parse(tit, end);
	if(d == -1) return -1;
	m_result.y = ip.m_result;
	sd += d;
	
	if(expectRP) {
		// S?
		d = opt_space_inst.parse(tit, end);
		sd += (d == -1)?0:d;
		
		// )
		d = literal_p<')'>().parse(tit, end);
		if(d == -1) return -1;
		sd += d;
	}
	return (it = tit, sd);
}

//This parser parses smpte smpte-30-drop and smpte-25 time formats
std::ptrdiff_t 
lib::smpte_p::parse(const_iterator& it, const const_iterator& end)
{
	const_iterator tit = it;
	std::ptrdiff_t d;
	std::ptrdiff_t sd = 0;
	
	int smpte[5];
	int result;
	
	m_drop = false;
	m_frame_rate = 30;
	
	delimiter_p space(" \t\r\n");
	
	star_p<delimiter_p> opt_space_inst = make_star(space);
	d = opt_space_inst.parse(tit, end);
	sd += (d == -1)?0:d;
	if (d == -1) {
		logger::get_logger()->debug(gettext("Failed to parse optional space "));
	}
	
	d = literal_cstr_p("smpte").parse(tit,end);
	sd += (d == -1)?0:d;
	if (d == -1) 
		return -1;
	
	d = literal_p<'-'>().parse(tit,end);
	sd += (d == -1)?0:d;
	int_p ip;
	if (d != -1) {
		d = ip.parse(tit,end);
		int r = ip.m_result;
		if (r == 25) 
		{	
			AM_DBG logger::get_logger()->debug(gettext("smpte-25"));
			m_frame_rate = 25;
			m_drop = false;
		} else {
			AM_DBG logger::get_logger()->debug(gettext("smpte-30"));
			m_frame_rate = 30;
			m_drop = false;
		}
				
	} else {
		m_frame_rate = 30;
		m_drop = false;
	}
	
	d = literal_cstr_p("-drop").parse(tit,end);
	if (d == -1) {
		m_drop = false;
	} else {
		AM_DBG logger::get_logger()->debug(gettext("drop"));
		m_drop = true;
	}
	
	
	d = literal_p<'='>().parse(tit,end);
	sd += (d == -1)?0:d;
	if (d == -1) {
		AM_DBG logger::get_logger()->debug(gettext("smpte parser failed to parse literal = "));
	}
	
	

	//parse the actual smpte values
	for(int i=0; i<3; i++) {
		d = ip.parse(tit,end);
		if (d == -1) { 
			AM_DBG logger::get_logger()->debug(gettext("smpte parser failed to parse smtpe (i=%d)"),i);
			return -1;
		}
		
		m_result[i] = ip.m_result;
		if ( (i > 0) && ( (m_result[i] < 0) || (m_result[i] > 59)) ) {
			logger::get_logger()->warn(gettext("Failed to parse smpte minutes/seconds. Value out of range [0,59]"));
			m_result[i] = 0;
		}

		sd += d;
		
		d = literal_p<':'>().parse(tit,end);
		sd += (d == -1)?0:d;
	}
	
	
	
	d = ip.parse(tit,end);
	if (d != -1) { 
		result = ip.m_result;
		if ( (result >= 0) && (result < m_frame_rate)) // range [0, framerate-1]
			m_result[3] = ip.m_result;
		else {
			logger::get_logger()->warn(gettext("Failed to parse smpte frames. Value out of range [0,%d]"),m_frame_rate-1);
			m_result[3] = 0;
		}
			
	} else {
		m_result[3] = 0;
	}
	
	sd += (d == -1)?0:d;
			
	d = literal_p<'.'>().parse(tit,end);
	sd += (d == -1)?0:d;
	
	d = ip.parse(tit,end);
	if (d != -1) { 
		result = ip.m_result;
		if ( (result >= 0) && (result < 2) ) // range [0,1]
			m_result[4] = ip.m_result;
		else {
			logger::get_logger()->warn(gettext("Failed to parse smpte sub-frames. Value out of range [0,1]"));
			m_result[4] = 0;
		}
		m_result[4] = ip.m_result;
	} else {
		m_result[4] = 0;
	}

	sd += (d == -1)?0:d;
	return (it=tit, sd);
}

long int 
lib::smpte_p::get_time()
{
	long int time;
	double frame_duration;
	
	if ((m_frame_rate == 30)) 
		frame_duration = 1.001/30;
	else 
		frame_duration = 1.0/25;
	
	time = (m_result[0]*60*60*1000) + (m_result[1]*60*1000) + (m_result[2] *1000) + (long int) round( (m_result[3] * frame_duration  + m_result[4] * frame_duration/2) * 1000);
	
	return time;
}

//This parser parses npt time format
std::ptrdiff_t 
lib::npt_p::parse(const_iterator& it, const const_iterator& end)
{
	const_iterator tit = it;
	std::ptrdiff_t d;
	std::ptrdiff_t sd = 0;
	
	//int result;
	
	
	delimiter_p space(" \t\r\n ntp");
	
	star_p<delimiter_p> opt_space_inst = make_star(space);
	d = opt_space_inst.parse(tit, end);
	sd += (d == -1)?0:d;
	AM_DBG {
		if (d == -1) {
			logger::get_logger()->debug(gettext("ntp parser failed to parse optional space"));
		} else {
			logger::get_logger()->debug(gettext("ntp parser succeded to parse optional space"));
		}
	}
	
	d = literal_p<'='>().parse(tit,end);
	sd += (d == -1)?0:d;
	AM_DBG { 
		if (d == -1) {
			logger::get_logger()->debug(gettext("ntp parser failed to parse literal = "));
		} else {
			logger::get_logger()->debug(gettext("ntp parser succeded to parse literal = "));
		}
	}
	
		
	lib::clock_value_p parser;
		
	d = parser.parse(tit,end);
	sd+= (d == -1)?0:d;
	if (d == -1) {
		AM_DBG logger::get_logger()->debug(gettext("ntp parser failed to parse time"));
		return -1;
	} else {
		m_result = parser.get_value();
		AM_DBG logger::get_logger()->debug(gettext("ntp parser succeded to parse time %ld"),m_result);

	}
	
	return (it=tit, sd);
}


long int 
lib::npt_p::get_time()
{
	return m_result;
}

std::ptrdiff_t 
lib::mediaclipping_p::parse(const_iterator& it, const const_iterator& end)
{
	const_iterator tit = it;
	std::ptrdiff_t d;
	
	lib::smpte_p smpte_parser;
	lib::npt_p npt_parser;
	
	d = npt_parser.parse(tit, end);
	
	if (d != -1) {
		m_result = npt_parser.get_time();
		return d;
	}

	d = smpte_parser.parse(tit,end);
	if (d != -1) {
		m_result = smpte_parser.get_time();
		return d;
	}
	
	return -1;
	
}

long int 
lib::mediaclipping_p::get_time()
{
	return m_result;
}
