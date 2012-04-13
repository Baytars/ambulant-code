// This file is part of Ambulant Player, www.ambulantplayer.org.
//
// Copyright (C) 2003-2011 Stichting CWI, 
// Science Park 123, 1098 XG Amsterdam, The Netherlands.
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
 * ncl_plugin.cpp -- implements an Ambulant renderer for .ncl (Nested Context Language) files
 *
 * The plugin listens to ncl events and when the value of a port changes,
 * a Smil-State variable with the same name as the port's name is updated, if present.
 * 
 * To be compiled/linked with ginga headers/libraries (developer version only)
`* 
`* Applicable tags: "ref", "video" with renderer identifier <param/> "RendererNCLPlugin" e.g.
 *      <ref src="..." region="..." ... > 
 *        <param name="renderer" value="http://www.ambulantplayer.org/component/RendererNCLPlugin"/>
 *      </ref>
 */
#include "ambulant/lib/logger.h"
#include "ambulant/version.h"
#include "ambulant/common/factory.h"
#include "ambulant/common/renderer_impl.h"
#include "ambulant/common/plugin_engine.h"
#include "ambulant/common/gui_player.h"
#include "ambulant/smil2/test_attrs.h"
#include "ambulant/common/state.h"
#define WITH_GTK
#ifdef  WITH_GTK
#include "ambulant/gui/gtk/gtk_factory.h"
#include "../src/player_gtk/gtk_mainloop.h"
#include <gdk/gdkx.h>
#include <GL/glx.h>
#endif//WITH_GTK

using namespace ::ambulant;
using namespace ::ambulant::common;
using namespace ::ambulant::lib;

#include "util/functions.h"
using namespace ::br::pucrio::telemidia::util;

#include "gingalssm/IPresentationEngineManager.h"
using namespace ::br::pucrio::telemidia::ginga::lssm;

#include "cm/IComponentManager.h"
using namespace ::br::pucrio::telemidia::ginga::core::cm;

#include "mb/ILocalScreenManager.h"
using namespace ::br::pucrio::telemidia::ginga::core::mb;

#include "player/IPlayerListener.h"
using namespace ::br::pucrio::telemidia::ginga::core::player;

#define AM_DBG
#ifndef AM_DBG
#define AM_DBG if(0)
#endif
using namespace ambulant;

class ncl_plugin_factory : public common::playable_factory {
  public:

	ncl_plugin_factory(common::factories* factory, common::playable_factory_machdep *mdp)
	:	m_factory(factory),
		m_mdp(mdp)
	{}
	~ncl_plugin_factory() {};

	bool supports(common::renderer_select *rs);

	common::playable *new_playable(
		common::playable_notification *context,
		common::playable_notification::cookie_type cookie,
		const lib::node *node,
		lib::event_processor *evp);

	common::playable *new_aux_audio_playable(
		common::playable_notification *context,
		common::playable_notification::cookie_type cookie,
		const lib::node *node,
		lib::event_processor *evp,
		net::audio_datasource *src) { return NULL; }
  private:
	common::factories *m_factory;
	common::playable_factory_machdep *m_mdp;
};

class ncl_plugin : public common::renderer_playable, public common::state_change_callback, public IPlayerListener
{
  public:
  ncl_plugin(
	common::playable_notification *context,
	common::playable_notification::cookie_type cookie,
	const lib::node *node,
	lib::event_processor *evp,
	common::factories *factory,
	common::playable_factory_machdep *mdp);

	~ncl_plugin();
	// playable_imp
	void data_avail();
	void start(double where);
	void seek(double where) {};
	//void stop();
	bool stop();
	void pause(pause_display d=display_show);
	void resume();
	// renderer imp.
	void redraw(const rect&, ambulant::common::gui_window*);
	void set_intransition(const ambulant::lib::transition_info*) {}
	void start_outtransition(const ambulant::lib::transition_info*) {}
	// state_change_callback
	void on_state_change(const char *ref);
	// IPlayerListener
	void updateStatus(short code, string parameter, short type, string value);
	// callback: initialize/update Ginga in main thread since X11 is involved
	void ncl_init();
	void ncl_update();
  private:
	net::url m_url;
	const char* m_file;
	state_component* m_state;
	common::playable_factory_machdep* m_mdp;
	const lib::node* m_node;

	IComponentManager*	m_cm; // persisent static object
	ILocalScreenManager*	m_dm; // persisent static object
	GingaScreenID		m_screenId;
  	// In Ginga, a Presentation Engine Manager steers all presentations
	IPresentationEngineManager* m_pem;
	char** m_argv;
	int m_argc;
	long int m_x_winid;
 
	// ambulant-ncl event processor handling
	GtkWidget* m_ambulant_window;
	GdkWindow* m_toplevel_window;
};

bool
ncl_plugin_factory::supports(common::renderer_select *rs)
{
	const char *renderer_uri = rs->get_renderer_uri();
	// Only use this plugin if explicitly requested. The tag does not matter.
	if (renderer_uri != NULL && strcmp(renderer_uri, AM_SYSTEM_COMPONENT("RendererNCLPlugin")) == 0)
		return true;
	return false;
}

common::playable*
ncl_plugin_factory::new_playable(
		common::playable_notification *context,
		common::playable_notification::cookie_type cookie,
		const lib::node *node,
		lib::event_processor *evp)
{
	common::playable *rv;

	lib::xml_string tag = node->get_qname().second;
	AM_DBG lib::logger::get_logger()->debug("ncl_plugin_factory::new_playable(0x%x): node 0x%x:	inspecting %s\n", (void*)this, (void *)node, tag.c_str());
	if ( tag == "ref" || tag == "video" ) /*or any other valid tag, of course */ {
		rv = new ncl_plugin(context, cookie, node, evp, m_factory, m_mdp);
		AM_DBG lib::logger::get_logger()->debug("ncl_plugin_factory::new_playable(0x%x): node 0x%x: returning ncl_plugin 0x%x", (void*)this, (void *)node, (void *)rv);
	} else {
		AM_DBG lib::logger::get_logger()->debug("ncl_plugin_factory::new_playable(0x%x) : plugin does not support \"%s\"", (void*)this, tag.c_str());
		return NULL;
	}
	return rv;
}

#ifdef	JNK
string ultostr(Window vValue) {
	unsigned long int value;
	string strValue;

	char dst[32];
	char digits[32];
	unsigned long int i = 0, j = 0, n = 0;

	value = vValue;
	do {
		n = value % 10;
		digits[i++] = (n < 10 ? (char)n+'0' : (char)n-10+'a');
		value /= 10;

		if (i > 31) {
			break;
		}

	} while (value != 0);

	n = i;
	i--;

	while (i >= 0 && j < 32) {
		dst[j] = digits[i];
		i--;
		j++;
	}

	strValue.assign(dst, n);

	return strValue;
}
extern "C" {
void gtk_C_callback_init(void *userdata)
{
	((ncl_plugin*) userdata)->ncl_init();
}
void gtk_C_callback_update(void *userdata)
{
	((ncl_plugin*) userdata)->ncl_update();
}
};
#endif//JNK

ncl_plugin::ncl_plugin(
	common::playable_notification *context,
	common::playable_notification::cookie_type cookie,
	const lib::node *node,
	lib::event_processor *evp,
	common::factories* factory,
	common::playable_factory_machdep *mdp)
  :	m_node(node),
	m_state(NULL),
	m_pem(NULL),
	m_cm(NULL),
	m_dm(NULL),
	m_ambulant_window(NULL),
	m_toplevel_window(NULL),
	common::renderer_playable(context, cookie, node, evp, factory, mdp), m_mdp(mdp)
{
	// select xine audio, because the default (FusionSound) sounds ugly)
	static char* s_argv[] = {
//			(char*) "--asystem", (char*) "SDL2_ffmpeg",
//			(char*) "--enable-log", (char*) "stdout",
			(char*) "--vsystem", (char*) "sdl",
//			(char*) "--external-renderer",
			(char*) "--parent",
			(char*) NULL // --parent must be last one
	};
	static int s_argc = sizeof s_argv/sizeof s_argv[0];
	m_argv = s_argv;
	m_argc = s_argc;
	m_url  = node->get_url("src");
	AM_DBG lib::logger::get_logger()->debug("ncl_plugin::ncl_plugin(0x%x) : argc=%d src=\"%s\", file=\"%s\", protocol=\"%s\", host=\"%s\", ref=\"%s\", path=\"%s\"", (void*)this, m_argc, repr(m_url).c_str(), m_url.get_file().c_str(), m_url.get_protocol().c_str(), m_url.get_host().c_str(), m_url.get_ref().c_str(), m_url.get_path().c_str());
	m_file = m_url.get_path().c_str();
	// rest of initialization needs to be done in main thread, because X11 is called
	gtk_mainloop* ml = (gtk_mainloop*) m_mdp;
	m_ambulant_window = ml->get_document_widget();
	m_toplevel_window = gtk_widget_get_parent_window(m_ambulant_window);
/*JNK
//	ncl_init();
	if (m_signal_init == 0) {
		m_signal_init = g_signal_new (AM_NCL_SIGNAL_INIT, g_object_get_type(), G_SIGNAL_RUN_LAST, 0, 0, 0, g_cclosure_marshal_VOID__POINTER, GTK_TYPE_NONE, 1, G_TYPE_POINTER);
	}
	if (m_signal_update == 0) {
		m_signal_init = g_signal_new (AM_NCL_SIGNAL_UPDATE, g_object_get_type(), G_SIGNAL_RUN_LAST, 0, 0, 0, g_cclosure_marshal_VOID__POINTER, GTK_TYPE_NONE, 1, G_TYPE_POINTER);
	}
	m_handler_id_init = g_signal_connect_swapped (G_OBJECT(m_toplevel_window), AM_NCL_SIGNAL_INIT, G_CALLBACK (gtk_C_callback_init), (gpointer) this );
	m_handler_id_update = g_signal_connect_swapped (G_OBJECT(m_toplevel_window), AM_NCL_SIGNAL_UPDATE, G_CALLBACK (gtk_C_callback_update), (gpointer) this );
	g_signal_emit_by_name(G_OBJECT(m_toplevel_window), AM_NCL_SIGNAL_INIT, 0, (gpointer) this);
*/
}

ncl_plugin::~ncl_plugin() {

	if (m_pem != NULL) {
		stop();
		delete m_pem;
	}
/*JNK
	if (m_update_event != NULL) {
		// remove all references from event queue
		m_running = false;
		while (m_event_processor->cancel_event(m_update_event, lib::ep_med));
		delete m_update_event;
	}
JNK*/

	if (m_dm != NULL) {
//		delete m_dm; // static object, leak !
	}
	if (m_cm != NULL) {
//		delete m_cm; // static object, leak !
	}
//	if (m_argv != NULL && m_argc >= 1) {
//		char* p = m_argv[m_argc-1];
//		if (p != NULL) {
//			free(p);
//		}
//	}
}

void 
ncl_plugin::ncl_init() {
	AM_DBG lib::logger::get_logger()->debug("ncl_plugin::ncl_init(0x%x): ", (void*)  this);
	// disconnect from the signal that brought us here
	// g_signal_handler_disconnect(G_OBJECT(m_toplevel_window), m_handler_id_init);
	// For Ginga-SDL, we need to pass the X11 Window Id of the first child of the toplevel window
	Window* children; //JNK following
	unsigned int n_children;
	Window root;
	Window parent;
	XWindowAttributes attr;

//JNK   XQueryTree(gdk_x11_get_default_xdisplay(), GDK_WINDOW_XID (m_toplevel_window), &root, &parent, &children, &n_children);
//JNK	XGetWindowAttributes(gdk_x11_get_default_xdisplay(), (Window) m_x_winid, &attr); //JNK until here
	m_x_winid = GDK_WINDOW_XID (m_toplevel_window);// NOT JNK
	m_cm  = IComponentManager::getCMInstance();
	setLogToNullDev();
	m_dm = ((LocalScreenManagerCreator*)(m_cm->getObject("LocalScreenManager")))();
	char buf[256];
	lib::rect r = m_dest->get_rect();
	r.translate(m_dest->get_global_topleft());
	sprintf(buf,"%s,%ld,%d,%d,%d,%d", XDisplayName(NULL), m_x_winid, r.left(),r.top(),r.width(),r.height());
	AM_DBG lib::logger::get_logger()->debug("ncl_plugin::ncl_init(0x%x): %s,%ld,%d,%d,%d,%d", (void*)this, XDisplayName(NULL), m_x_winid, r.left(),r.top(),r.width(),r.height());
	m_argv[m_argc-1] = strdup(buf);
	unsigned long int x = strtoul(m_argv[m_argc-1], NULL, 10);

	m_screenId = m_dm->createScreen(m_argc, m_argv);
	m_pem = ((PEMCreator*)(m_cm->getObject("PresentationEngineManager")))(0, 0, 0, 0, 0, false, m_screenId);
	m_pem->setIsLocalNcl(false, NULL);
	if (m_pem->openNclFile(m_file)) {
		m_pem->addPlayerListener(m_file, this);
		m_pem->startPresentation(m_file, "");
		// ask m_pem which NCL ports are available for us
		set<string>* portIds = m_pem->createPortIdList(m_file);
		// Get the Smil-State engine
		m_state = m_node->get_context()->get_state();
		if (portIds != NULL && m_state != NULL) {
			AM_DBG lib::logger::get_logger()->debug("ncl_plugin::ncl_init(0x%x) : %d ports mapped:", (void*)this, portIds->size());
			// prepare to detect Smile State changes of variables with the same name  
			for (set<string>::iterator it = portIds->begin(); it != portIds->end(); it++) {
				AM_DBG lib::logger::get_logger()->debug("ncl_plugin::ncl_init(0x%x) : portId:\"%s\"", (void*)this, it->c_str());
				m_state->want_state_change(it->c_str(), this);
			}
		}
		AM_DBG lib::logger::get_logger()->debug("ncl_plugin::ncl_init(0x%x) : m_pem(0x%x)->startPresentation(%s)", (void*)this, m_pem, m_file);
	}
	setLogToStdoutDev();
}

void
ncl_plugin::data_avail()
{
	AM_DBG lib::logger::get_logger()->debug("ncl_plugin::data_avail(0x%x): ", (void*) this);
}

void
ncl_plugin::start(double t)
{
	AM_DBG lib::logger::get_logger()->debug("ncl_plugin::start(0x%x): t=%f", (void*) this, t);
	if (m_pem == NULL) {
		ncl_init();
	}
//	renderer_playable* base = this;
//	base->start(t);
	m_pem->startPresentation(m_file, "");
	m_dest->show(this);
	m_dest->need_redraw(m_dest->get_rect());
	m_context->started(m_cookie);
//JNK	m_running = true;
//JNK	_schedule_callback(AM_NCL_UPDATE_DELAY_MS, &ncl_plugin::_update);
}

bool
ncl_plugin::stop()
{
	AM_DBG lib::logger::get_logger()->debug("ncl_plugin::stop(0x%x): ", (void*) this);
	if (m_pem != NULL) {
		m_pem->stopPresentation(m_file);
		m_context->stopped(m_cookie);
	}
	return true;
}
void
ncl_plugin::pause(pause_display d)
{
	AM_DBG lib::logger::get_logger()->debug("ncl_plugin::pause(0x%x): ", (void*) this);
	if (m_pem != NULL) {
		m_pem->pausePresentation(m_file);
	}
}

void
ncl_plugin::resume()
{
	AM_DBG lib::logger::get_logger()->debug("ncl_plugin::resume(0x%x): ", (void*) this);
	if (m_pem != NULL) {
		m_pem->resumePresentation(m_file);
	}
}

void
ncl_plugin::on_state_change(const char *ref)
{
	AM_DBG lib::logger::get_logger()->debug("ncl_plugin::on_state_change(%s)", ref);
	if (m_state != NULL) {
		std::string value = m_state->string_expression(ref);
		AM_DBG lib::logger::get_logger()->debug("ncl_plugin::on_state_change(%s) value=%s", ref, value.c_str());
		m_pem->setPropertyValue(m_file, ref, value);

	}
}

void 
ncl_plugin::redraw(const rect& r, ambulant::common::gui_window* window) {
	AM_DBG lib::logger::get_logger()->debug("ncl_plugin::redraw(0x%x) LTWH=(%d,%d, %d,%d), window=0x%x", this, r.left(),r.top(),r.width(),r.height(), window);
//	if (m_dm != NULL) {
//		m_dm->refreshScreen(m_screenId); // for --external-renderer
//	}
}

void
ncl_plugin::updateStatus(short code, string parameter, short type, string value) {
	if (code == IPlayer::PL_NOTIFY_STOP && type == IPlayer::TYPE_ATTRIBUTION) {
	  /*AM_DBG*/ lib::logger::get_logger()->debug("ncl_plugin::updateStatus(0x%x) code=%d, parameter=%s, type=%d, value=%s: event received",(void*) this, code, parameter.c_str(), type, value.c_str());
		// test if state was specified in .smil source
  		if (m_state != NULL) {
			// test if  state variable "var_name" has changed
			string old_value = m_state->string_expression(parameter.c_str());
			if (old_value != value) {
				m_state->set_value (parameter.c_str(), value.c_str());
			}
		}
	}
}

static ambulant::common::factories *
bug_workaround(ambulant::common::factories* factory)
{
	return factory;
}

extern "C"
#ifdef AMBULANT_PLATFORM_WIN32
__declspec(dllexport)
#endif
void initialize(
	int api_version,
	ambulant::common::factories* factory,
	ambulant::common::gui_player *player)
{
	if ( api_version != AMBULANT_PLUGIN_API_VERSION ) {
		lib::logger::get_logger()->warn(gettext("%s: built for plugin-api version %d, current %d. Skipping."),"ncl_plugin", AMBULANT_PLUGIN_API_VERSION, api_version);
		return;
	}
	if ( !ambulant::check_version() ) {
		lib::logger::get_logger()->warn(gettext("%s: built for different Ambulant version (%s)"),"ncl_plugin", AMBULANT_VERSION);
	}
	factory = bug_workaround(factory);
	lib::logger::get_logger()->debug("ncl_plugin: loaded.");
	common::global_playable_factory *pf = factory->get_playable_factory();
	if (pf) {
		ncl_plugin_factory *bpf = new ncl_plugin_factory(factory,(playable_factory_machdep*) player);
		pf->add_factory(bpf);
		lib::logger::get_logger()->trace("ncl_plugin: registered");
	}
}
