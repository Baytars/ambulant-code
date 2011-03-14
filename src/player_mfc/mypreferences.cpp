// This file is part of Ambulant Player, www.ambulantplayer.org.
//
// Copyright (C) 2003-2005 Stichting CWI, 
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

#include "stdafx.h"
#include "mypreferences.h"

void
mypreferences::install_singleton()
{
	set_preferences_singleton(new mypreferences);
	// XXX Workaround
	get_preferences()->load_preferences();
}

bool
mypreferences::load_preferences()
{
	USES_CONVERSION;
//	NSLog(@"Loading preferences");
	CWinApp* pApp = AfxGetApp();
	CString val = pApp->GetProfileString(_T("Settings"), _T("parser_id"), _T("any"));
	m_parser_id = T2CA((LPCTSTR)val);
	val = pApp->GetProfileString(_T("Settings"), _T("validation_scheme"), _T("auto"));
	m_validation_scheme = T2CA((LPCTSTR)val);
	m_do_namespaces = (bool)pApp->GetProfileInt(_T("Settings"), _T("do_namespaces"), 0);
	m_do_schema = (bool)pApp->GetProfileInt(_T("Settings"), _T("do_schema"), 0);
	m_validation_schema_full_checking = (bool)pApp->GetProfileInt(_T("Settings"), _T("validation_schema_full_checking"), 0);
	m_log_level = pApp->GetProfileInt(_T("Settings"), _T("log_level"), 0);
	m_use_plugins = (bool)pApp->GetProfileInt(_T("Settings"), _T("use_plugins"), 0);
	val = pApp->GetProfileString(_T("Settings"), _T("plugin_dir"),0);
	m_plugin_dir = T2CA((LPCTSTR)val);

	return true;
}

bool
mypreferences::save_preferences()
{
	USES_CONVERSION;
	CWinApp* pApp = AfxGetApp();

	pApp->WriteProfileString(_T("Settings"), _T("parser_id"), A2CT(m_parser_id.c_str()));
	pApp->WriteProfileString(_T("Settings"), _T("validation_scheme"), A2CT(m_validation_scheme.c_str()));
	pApp->WriteProfileInt(_T("Settings"), _T("do_namespaces"), (int)m_do_namespaces);
	pApp->WriteProfileInt(_T("Settings"), _T("do_schema"), (int)m_do_schema);
	pApp->WriteProfileInt(_T("Settings"), _T("validation_schema_full_checking"), (int)m_validation_schema_full_checking);
	pApp->WriteProfileInt(_T("Settings"), _T("log_level"), m_log_level);
	pApp->WriteProfileInt(_T("Settings"), _T("use_plugins"), m_use_plugins);
	pApp->WriteProfileString(_T("Settings"), _T("plugin_dir"), A2CT(m_plugin_dir.c_str()));
	
	return true;
}