/*
 * This file is part of Ambulant Player, www.ambulantplayer.org.
 *
 * Copyright (C) 2003-2005 Stichting CWI, 
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
#ifndef __GTK_SETTINGS_H__
#define __GTK_SETTINGS_H__

class gtk_settings
{
 public:
	void settings_finish();
	void settings_ok();
	GtkWidget* settings_select();

 private:
	// settings
	QVGroupBox*	m_settings_vg;
	// Log level:
	QHBox*		m_loglevel_hb;
	QLabel*		m_loglevel_lb;
	QComboBox*	m_loglevel_co;

	// XML parser:
	QHBox*		m_parser_hb;
	GtkLabel*	m_parser_lb;
	GtkComboBox*	m_parser_co;

	// xerces options:
	QVGroupBox*	m_xerces_vg;
	GtkCheckBox*	m_namespace_cb;	// Enable XML namespace support
	bool		m_namespace_val;
 	QHBox*		m_validation_hb;// Enable XML validation:
 	GtkLabel*		m_validation_lb;// Enable XML validation:
 	QComboBox*	m_validation_co;// Enable XML validation:
	//QCheckBox*	m_validation_cb;
	//bool		m_validation_val;
	QVBox*		m_validation_vb;
	QHButtonGroup*	m_declaration_bg;
	QRadioButton* 	m_schema_rb;	// Using Schema
	bool		m_schema_val;
	QRadioButton*	m_dtd_rb;	// Using DTD
	bool		m_dtd_val;
	QCheckBox*	m_full_check_cb;// Validation Schema full checking
	bool		m_full_check_val;
	QHBox*		m_finish_hb;
	GtkButton*	m_ok_pb;	// OK
	GtkButton*	m_cancel_pb;	// Cancel
	
	// plugin options
	QVGroupBox* m_plugin_vg;
	QCheckBox*  m_use_plugin_cb;
	QLineEdit*	m_plugin_dir_le;
	GtkLabel*	m_plugin_dir_lb;

	int index_in_string_array(const char* s, const char* sa[]);
};
#endif/*__GTK_SETTINGS_H__*/
