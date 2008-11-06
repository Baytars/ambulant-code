// This file is part of Ambulant Player, www.ambulantplayer.org.
//
// Copyright (C) 2003-2008 Stichting CWI, 
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

#include "ambulant/config/config.h"
#include "ambulant/common/plugin_engine.h"
#include "ambulant/lib/logger.h"
#include "ambulant/common/preferences.h"
#include "ambulant/lib/textptr.h"

//#include<dlfcn.h>
#include<stdlib.h>
#include<string.h>

#if defined(WITH_LTDL_PLUGINS) || defined(WITH_WINDOWS_PLUGINS)
#define WITH_PLUGINS 1
#endif

#ifdef WITH_LTDL_PLUGINS
#include<dirent.h>
#include <ltdl.h>

#ifdef AMBULANT_PLATFORM_MACOS
#include <CoreFoundation/CoreFoundation.h>
#define LIBRARY_PATH_ENVVAR "DYLD_LIBRARY_PATH"
#else
#define LIBRARY_PATH_ENVVAR "LD_LIBRARY_PATH"
#endif // AMBULANT_PLATFORM_MACOS

#endif // WITH_LTDL_PLUGINS

#ifdef WITH_WINDOWS_PLUGINS
#ifndef _T
#define _T(x) (x)
#endif
#endif // WITH_WINDOWS_PLUGINS

#ifndef AM_DBG
#define AM_DBG if(0)
#endif

#ifdef _DEBUG
#define PLUGIN_PREFIX "libampluginD_"
#define PYTHON_PLUGIN_ENGINE_PREFIX "libampluginD_python"
#else
#define PLUGIN_PREFIX "libamplugin_"
#define PYTHON_PLUGIN_ENGINE_PREFIX "libamplugin_python"
#endif
#define PYTHON_PLUGIN_PREFIX "pyamplugin_"

using namespace ambulant;
using namespace common;

plugin_engine *ambulant::common::plugin_engine::s_singleton = NULL;

plugin_engine *
plugin_engine::get_plugin_engine()
{
    if (s_singleton == NULL)
        s_singleton = new plugin_engine;
    return s_singleton;
}

plugin_engine::plugin_engine()
{
#ifdef WITH_PLUGINS
	bool use_plugins = common::preferences::get_preferences()->m_use_plugins;
    collect_plugin_directories();
#ifdef WITH_LTDL_PLUGINS
	lib::logger::get_logger()->trace("plugin_engine: using LTDL plugin loader");F
	int errors = lt_dlinit();
	if (errors) {
		lib::logger::get_logger()->trace("LTDL plugin loader: Cannot initialize: %d error(s)", errors);
		lib::logger::get_logger()->warn(gettext("Plugin loader encountered problem: plugins disabled"));
	    return;
	}
#endif // WITH_LTDL_PLUGINS
#ifdef WITH_WINDOWS_PLUGINS
	lib::logger::get_logger()->trace("plugin_engine: using Windows plugin loader");
#endif // WITH_WINDOWS_PLUGINS
	if (use_plugins) {
		int count = 0;
		std::vector< std::string >::iterator i;
		for (i=m_plugindirs.begin(); i!=m_plugindirs.end(); i++) {
			load_plugins(*i);
			count++;
		}
		if (count == 0) {
			lib::logger::get_logger()->trace("plugin_engine: no plugin directories configured");
		}
#ifdef WITH_PYTHON_PLUGIN
		if (m_python_plugins.size() > 0) {
			if (m_python_plugin_engine == "") {
				lib::logger::get_logger()->trace("plugin_engine: Python plugins not loaded: no engine found");
			} else {
				// Load the Python engine. It will use get_python_plugins() to get at the
				// pathnames for the plugins and load those.
				load_plugin(m_python_plugin_engine.c_str());
			}
		}
#endif // WITH_PYTHON_PLUGIN
	} else {
		lib::logger::get_logger()->trace("plugin_engine: plugins disabled by user preference");
	}
#else
	lib::logger::get_logger()->trace("plugin_engine: no plugin loader configured");
#endif // WITH_PLUGINS
}

void
plugin_engine::collect_plugin_directories()
{
#ifdef WITH_PLUGINS
#ifdef AMBULANT_PLATFORM_UNIX
	// First plugin dir is set through the environment
	const char *env_plugins = getenv("AMBULANT_PLUGIN_DIR");
	if (env_plugins)
		m_plugindirs.push_back(env_plugins);
#endif
	// Second dir to search is set per user preferences
	std::string& plugin_dir = common::preferences::get_preferences()->m_plugin_dir;
	if(plugin_dir != "")
		m_plugindirs.push_back(plugin_dir);

#ifdef AMBULANT_PLATFORM_MACOS
	// On MacOSX add the bundle's plugin dir
	// XXXX If Ambulant is used within a plugin, this probably needs to change,
	// maybe to something with CFBundleGetAllBundles?
	CFBundleRef bundle = CFBundleGetMainBundle();
	if (bundle) {
		CFURLRef plugin_url = CFBundleCopyBuiltInPlugInsURL(bundle);
		char plugin_pathname[1024];
		if (plugin_url &&
				CFURLGetFileSystemRepresentation(plugin_url, true, (UInt8 *)plugin_pathname, sizeof(plugin_pathname))) {
			m_plugindirs.push_back(plugin_pathname);
		}
		if (plugin_url) CFRelease(plugin_url);
	}
	// And if we are in a plugin we should get that bundle too. NOTE: the name used here
	// must match the name in the plist file of the plugin.
	bundle = CFBundleGetBundleWithIdentifier(CFSTR("org.ambulantplayer.ambulantplugin"));
	if (bundle) {
		CFURLRef plugin_url = CFBundleCopyBuiltInPlugInsURL(bundle);
		char plugin_pathname[1024];
		if (plugin_url &&
				CFURLGetFileSystemRepresentation(plugin_url, true, (UInt8 *)plugin_pathname, sizeof(plugin_pathname))) {
			m_plugindirs.push_back(plugin_pathname);
		}
		if (plugin_url) CFRelease(plugin_url);
	}
#elif defined(AMBULANT_PLATFORM_UNIX)
	// On other unix platforms add the pkglibdir
#ifdef AMBULANT_PLUGINDIR
	m_plugindirs.push_back(AMBULANT_PLUGINDIR);
#else
	m_plugindirs.push_back("/usr/local/lib/ambulant");
#endif // AMBULANT_PLUGINDIR
#elif defined(AMBULANT_PLATFORM_WIN32)
	// Add directory containing the main module (either main prog or dll)
	std::string main_dir = lib::win32::get_module_dir();
	m_plugindirs.push_back(main_dir);
#else
#error WITH_PLUGINS defined for unknown platform
#endif
#endif // WITH_PLUGINS
}

#ifdef WITH_LTDL_PLUGINS
#ifdef AMBULANT_PLATFORM_MACOS
#define MAYBE_CONST
#else
#define MAYBE_CONST const
#endif

static int filter(MAYBE_CONST struct dirent* filen)
{
	int len;
	len = strlen(filen->d_name);
	if (strncmp(filen->d_name+(len-3),".la",3) == 0 ||
			strncmp(filen->d_name, PYTHON_PLUGIN_PREFIX, sizeof(PYTHON_PLUGIN_PREFIX)-1) == 0) {
		return 1;
	} else {
		return 0;
	}
	return 0;
}

void plugin_engine::load_plugin(const char *filename)
{
	// Load the plugin
	lib::logger::get_logger()->trace("plugin_engine: loading %s", filename);
	lt_dlhandle handle = lt_dlopen(filename);
	if (handle) {
		AM_DBG lib::logger::get_logger()->debug("plugin_engine: reading plugin SUCCES [ %s ]",filename);
		AM_DBG lib::logger::get_logger()->debug("Registering  plugin's factory");
		initfuncptr init = (initfuncptr) lt_dlsym(handle,"initialize");
		if (!init) {
			lib::logger::get_logger()->trace("plugin_engine: %s: no initialize routine", filename);
			lib::logger::get_logger()->warn(gettext("Plugin skipped due to errors: %s "), filename);
		} else {
			m_initfuncs.push_back(init);
		}
		plugin_extra_data *extra = (plugin_extra_data *)lt_dlsym(handle, "plugin_extra_data");
		if (extra) {
			AM_DBG lib::logger::get_logger()->debug("plugin_engine: extra data \"%s\" is 0x%x", extra->m_plugin_name, extra->m_plugin_extra);
			std::string name = extra->m_plugin_name;
			m_extra_data[name] = extra;
		}
	} else {
		lib::logger::get_logger()->trace("plugin_engine: lt_dlopen(%s) failed: %s",filename, lt_dlerror());
		lib::logger::get_logger()->warn(gettext("Plugin skipped due to errors: %s "), filename);
	}
}

void
plugin_engine::load_plugins(std::string dirname)
{
	lib::logger::get_logger()->trace("plugin_engine: Scanning plugin directory: %s", dirname.c_str());
	char filename[1024];
	dirent **namelist;
	bool ldpath_added = false;
	
    int nr_of_files = scandir(dirname.c_str(), &namelist, &filter , NULL);
    if (nr_of_files < 0) {
        lib::logger::get_logger()->trace("Error reading plugin directory: %s: %s", dirname.c_str(), strerror(errno));
        return;
    } else {
        while (nr_of_files--) {
            //only normal files, not dots (. and ..)
            if (strcmp(namelist[nr_of_files]->d_name, ".")  &&
                    strcmp(namelist[nr_of_files]->d_name, "..")) {
                char *pluginname = namelist[nr_of_files]->d_name;
#ifdef WITH_PYTHON_PLUGIN
				bool is_python_plugin = false;
				bool is_python_engine = false;
#endif // WITH_PYTHON_PLUGIN
                
                // Check the name is valid
                if (strncmp(PYTHON_PLUGIN_PREFIX, pluginname, sizeof(PYTHON_PLUGIN_PREFIX)-1) == 0) {
#ifdef WITH_PYTHON_PLUGIN
					is_python_plugin = true;
                    lib::logger::get_logger()->trace("plugin_engine: recording Python plugin %s", pluginname);
#else
                    lib::logger::get_logger()->trace("plugin_engine: skipping Python plugin %s", pluginname);
					continue;
#endif // WITH_PYTHON_PLUGIN
                } else
				if (strncmp(PLUGIN_PREFIX, pluginname, sizeof(PLUGIN_PREFIX)-1) != 0) {
                    lib::logger::get_logger()->trace("plugin_engine: skipping %s", pluginname);
		    free(namelist[nr_of_files]);
		    continue;
                }
				// Check whether this is the Python engine
                if (strncmp(PYTHON_PLUGIN_ENGINE_PREFIX, pluginname, sizeof(PYTHON_PLUGIN_ENGINE_PREFIX)-1) == 0) {
#ifdef WITH_PYTHON_PLUGIN
					is_python_engine = true;
                    lib::logger::get_logger()->trace("plugin_engine: recording Python engine %s", pluginname);
#else
                    lib::logger::get_logger()->trace("plugin_engine: skipping Python engine %s", pluginname);
		    free(namelist[nr_of_files]);
                    continue;
#endif // WITH_PYTHON_PLUGIN
                }
                
                // Construct the full pathname
                strncpy(filename, dirname.c_str(), sizeof(filename));
                strncat(filename, "/", sizeof(filename));
                strncat(filename, pluginname, sizeof(filename));
				
				// Add the plugin dir to the search path
				if (!ldpath_added) {
					setenv(LIBRARY_PATH_ENVVAR, dirname.c_str(), 1);
					ldpath_added = true;
				}
#ifdef WITH_PYTHON_PLUGIN
				// If it is the Python engine we don't load it but remember it for later
				if (is_python_engine) {
					m_python_plugin_engine = filename;
					free(namelist[nr_of_files]);
					continue;
				}
				// And similar for a Python plugin
				if (is_python_plugin) {
					std::string filename_str = filename;
					m_python_plugins.push_back(filename_str);
					free(namelist[nr_of_files]);
					continue;
				}
#endif // WITH_PYTHON_PLUGIN
				// Finally we get to load the plugin
				load_plugin(filename);
            }
            free(namelist[nr_of_files]);
        }
        free(namelist);
    }
	lib::logger::get_logger()->trace("plugin_engine: Done with plugin directory: %s", dirname.c_str());
	if (ldpath_added)
		unsetenv(LIBRARY_PATH_ENVVAR);
}

#elif WITH_WINDOWS_PLUGINS
void plugin_engine::load_plugin(const char *filename)
{
	lib::textptr pn_conv(filename);
	// Load the plugin
	lib::logger::get_logger()->trace("plugin_engine: loading %s", filename);
	HMODULE handle = LoadLibrary(pn_conv);
	if (handle) {
		AM_DBG lib::logger::get_logger()->debug("plugin_engine: reading plugin SUCCES [ %s ]",filename);
		AM_DBG lib::logger::get_logger()->debug("Registering test plugin's factory");
		initfuncptr init = (initfuncptr) GetProcAddress(handle, _T("initialize"));
		if (!init) {
			lib::logger::get_logger()->trace("plugin_engine: %s: no initialize routine", filename);
			lib::logger::get_logger()->warn(gettext("Plugin skipped due to errors: %s "), filename);
		} else {
			m_initfuncs.push_back(init);
		}
		plugin_extra_data *extra = (plugin_extra_data *)GetProcAddress(handle, _T("plugin_extra_data"));
		if (extra) {
			std::string name = extra->m_plugin_name;
			m_extra_data[name] = extra;
		}
   } else {
		DWORD err = GetLastError();
		lib::logger::get_logger()->trace("plugin_engine: %s: LoadLibrary returned error 0x%x", filename, err);
		lib::logger::get_logger()->warn(gettext("Plugin skipped due to errors: %s"), filename);
	}
}

void
plugin_engine::load_plugins(std::string dirname)
{
	lib::logger::get_logger()->trace("plugin_engine: Scanning plugin directory: %s", dirname.c_str());
	std::string filepattern = 
		dirname +
//		"\\" +
		PLUGIN_PREFIX +
		"*.dll";
	lib::textptr fp_conv(filepattern.c_str());
	WIN32_FIND_DATA dirData;
	HANDLE dirHandle = FindFirstFile(fp_conv, &dirData);
    if (dirHandle == INVALID_HANDLE_VALUE) {
		DWORD err = GetLastError();
		if (err != ERROR_FILE_NOT_FOUND && err != ERROR_NO_MORE_FILES) // Don't report "No such file"
			lib::logger::get_logger()->error(gettext("Error reading plugin directory: %s: 0x%x"), dirname.c_str(), err);
        return;
    } else {
		do {
            // Construct the full pathname
			lib::textptr fn_conv(dirData.cFileName);
			std::string pathname =
				dirname +
 				"\\" +
				fn_conv.c_str();
			if (strncmp(PYTHON_PLUGIN_ENGINE_PREFIX, fn_conv.c_str(), sizeof(PYTHON_PLUGIN_ENGINE_PREFIX)-1) == 0) {
#ifdef WITH_PYTHON_PLUGIN
				m_python_plugin_engine = pathname;
#else
				lib::logger::get_logger()->trace("plugin_engine: skipping Python engine %s", pathname);
#endif // WITH_PYTHON_PLUGIN
				continue;
			}
			// Load the plugin
			load_plugin(pathname.c_str());
		} while(FindNextFile(dirHandle, &dirData));
	}
#ifdef WITH_PYTHON_PLUGIN
	{
		std::string filepattern = 
			dirname +
	//		"\\" +
			PYTHON_PLUGIN_PREFIX +
			"*";
		lib::textptr fp_conv(filepattern.c_str());
		WIN32_FIND_DATA dirData;
		HANDLE dirHandle = FindFirstFile(fp_conv, &dirData);
		if (dirHandle == INVALID_HANDLE_VALUE) {
			DWORD err = GetLastError();
			if (err != ERROR_FILE_NOT_FOUND && err != ERROR_NO_MORE_FILES) // Don't report "No such file"
				lib::logger::get_logger()->error(gettext("Error reading plugin directory: %s: 0x%x"), dirname.c_str(), err);
			return;
		} else {
			do {
				// Construct the full pathname
				lib::textptr fn_conv(dirData.cFileName);
				std::string pathname =
					dirname +
					"\\" +
					fn_conv.c_str();
				// Remember the plugin
				m_python_plugins.push_back(pathname);
			} while(FindNextFile(dirHandle, &dirData));
		}
	}
#endif // WITH_PYTHON_PLUGIN
 	lib::logger::get_logger()->trace("plugin_engine: Done with plugin directory: %s", dirname.c_str());
}

#else
void
plugin_engine::load_plugins(std::string dirname)
{
}
#endif // WITH_XXXX_PLUGINS

void
plugin_engine::add_plugins(common::factories* factory, common::gui_player *player)
{
    std::vector< initfuncptr >::iterator i;
    for(i=m_initfuncs.begin(); i!=m_initfuncs.end(); i++) {
        initfuncptr init;
        init = *i;
        (init)(AMBULANT_PLUGIN_API_VERSION, factory, player);
    }
}

void *
plugin_engine::get_extra_data(std::string name)
{
	if (m_extra_data.count(name) == 0)
		return NULL;
	return m_extra_data[name]->m_plugin_extra;
}
