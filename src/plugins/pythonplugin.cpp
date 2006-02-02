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

#include "ambulant/lib/logger.h"
#include "ambulant/version.h"
#include "ambulant/common/factory.h"
#include "ambulant/common/plugin_engine.h"
#include "ambulant/common/gui_player.h"

#include "Python.h"

#ifndef AM_DBG
#define AM_DBG if(0)
#endif
using namespace ambulant;

#define AMPYTHON_MODULE_NAME "pyamplugin"
#define AMPYTHON_METHOD_NAME "initialize"

static ambulant::common::factories * 
bug_workaround(ambulant::common::factories* factory)
{
	return factory;
}

extern "C" void initialize(
    int api_version,
    ambulant::common::factories* factory,
    ambulant::common::gui_player *player)
{
    if ( api_version != AMBULANT_PLUGIN_API_VERSION ) {
        lib::logger::get_logger()->warn("python_plugin: built for plugin-api version %d, current %d. Skipping.", 
            AMBULANT_PLUGIN_API_VERSION, api_version);
        return;
    }
    if ( !ambulant::check_version() )
        lib::logger::get_logger()->warn("python_plugin: built for different Ambulant version (%s)", AMBULANT_VERSION);
	factory = bug_workaround(factory);
    AM_DBG lib::logger::get_logger()->debug("python_plugin: loaded.");
    
    // Starting up Python is a bit difficult because we want to release the
    // lock before we return. So the first time we're here we initialze and then
    // release the GIL only to re-acquire it immediately.
    if (!PyEval_ThreadsInitialized()) {
	    Py_Initialize();
	    PyEval_InitThreads();
	    PyEval_SaveThread();
	}
    AM_DBG lib::logger::get_logger()->debug("python_plugin: initialized Python.");
    
    PyGILState_STATE _GILState = PyGILState_Ensure();
	AM_DBG lib::logger::get_logger()->debug("python_plugin: acquired GIL.");
    PyObject *mod = PyImport_ImportModule(AMPYTHON_MODULE_NAME);
    if (mod == NULL) {
        PyErr_Print();
        lib::logger::get_logger()->debug("python_plugin: import %s failed.", AMPYTHON_MODULE_NAME);
        return;
    }
    AM_DBG lib::logger::get_logger()->debug("python_plugin: imported %s.", AMPYTHON_MODULE_NAME);
    
    PyObject *rv = PyObject_CallMethod(mod, AMPYTHON_METHOD_NAME, "");
    if (rv == NULL) {
        PyErr_Print();
        lib::logger::get_logger()->debug("python_plugin: calling of %s failed.", AMPYTHON_METHOD_NAME);
        return;
    }
    AM_DBG lib::logger::get_logger()->debug("python_plugin: %s returned, about to release GIL", AMPYTHON_METHOD_NAME);
    PyGILState_Release(_GILState);
    Py_DECREF(rv);
    AM_DBG lib::logger::get_logger()->debug("python_plugin: returning.");
}
