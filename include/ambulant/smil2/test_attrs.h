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

#ifndef AMBULANT_SMIL2_TEST_ATTRS_H
#define AMBULANT_SMIL2_TEST_ATTRS_H

#include "ambulant/config/config.h"
#include "ambulant/lib/document.h"
#include "ambulant/lib/node.h"

#include <string>
#include <map>

namespace ambulant {

namespace lib {
class logger;
class document;
} // namespace lib

namespace smil2 {

using lib::custom_test;

class AMBULANTAPI test_attrs {
  public:
	test_attrs(const lib::node *n);
	
	// Returns true when the target node is selected.
	bool selected() const;
	
	static bool load_test_attrs(const std::string& filename);
	static void set_default_tests_attrs();
	
	static void read_custom_attributes(const lib::document *doc, 
		std::map<std::string, custom_test>& custom_tests);
	
	static void read_custom_attributes(const lib::document *doc);
	static void update_doc_custom_attributes(std::map<std::string, custom_test>& custom_tests);

	const std::string& get_tag() const { return m_tag;}
	const std::string& get_id() const { return m_id;}

  private:
	typedef std::string::size_type size_type;
	bool test_on_off_attr(const std::string& attr,const char *value) const;
	bool test_exact_str_attr(const std::string& attr,const char *value) const;
	bool test_exact_str_list_attr(const std::string& attr,const char *value) const;
	
	bool test_system_language(const char *lang) const;
	bool test_system_component(const char *value) const;
	bool test_system_bitrate(const char *value) const;
	bool test_system_screen_depth(const char *value) const;
	bool test_system_screen_size(const char *value) const;
	bool test_custom_attribute(const char *value) const;
	
	// the target node
	const lib::node *m_node;
	const std::map<std::string, custom_test>* m_custom_tests;
	
	// tracing
	std::string m_id;
	std::string m_tag;
	lib::logger *m_logger;
};

} // namespace smil2
 
} // namespace ambulant

bool load_test_attrs(const char *filename);

#endif // AMBULANT_SMIL2_TEST_ATTRS_H
