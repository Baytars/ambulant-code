// This file is part of Ambulant Player, www.ambulantplayer.org.
//
// Copyright (C) 2003-2010 Stichting CWI,
// Science Park 123, 1098 XG Amsterdam, The Netherlands.
//
// Ambulant Player is free software; you can redistribute it and/or modify
// it under the terms of the GNU Lesser General Public License as published by
// the Free Software Foundation; either version 2.1 of the License, or
// (at your option) any later version.
//
// Ambulant Player is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.	 See the
// GNU Lesser General Public License for more details.
//
// You should have received a copy of the GNU Lesser General Public License
// along with Ambulant Player; if not, write to the Free Software
// Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA  02110-1301  USA

/*
 * @$Id$
 */

#include "ambulant/config/config.h"
#include "ambulant/gui/d2/d2_player.h"
#include "ambulant/gui/d2/d2_window.h"
#include "ambulant/gui/dx/dx_wmuser.h"
#include "ambulant/gui/dx/dx_rgn.h"
#include "ambulant/gui/d2/d2_transition.h"

#include "ambulant/lib/event.h"
#include "ambulant/lib/event_processor.h"
#include "ambulant/lib/asb.h"
#include "ambulant/lib/document.h"
#include "ambulant/lib/textptr.h"
#include "ambulant/lib/logger.h"
#include "ambulant/lib/win32/win32_error.h"
#include "ambulant/lib/transition_info.h"
#include "ambulant/common/plugin_engine.h"
#include "ambulant/smil2/transition.h"
#include "ambulant/smil2/smil_player.h"
#include "ambulant/smil2/test_attrs.h"

// Renderer playables
//#include "ambulant/gui/d2/html_bridge.h"
#include "ambulant/gui/d2/d2_fill.h"
#include "ambulant/gui/d2/d2_text.h"
#ifdef	WITH_SMIL30
#include "ambulant/gui/d2/d2_smiltext.h"
#endif/*WITH_SMIL30*/
//#include "ambulant/gui/d2/d2_html_renderer.h"
#include "ambulant/gui/d2/d2_img.h"
//#include "ambulant/gui/d2/d2_img_wic.h"

// Select audio renderer to use.
// Multiple selections are possible.
#ifdef WITH_FFMPEG
// Use the datasource-based SDL audio renderer
#define USE_SDL_AUDIO
#endif
// Use the DirectX audio renderer
#define USE_D2_AUDIO

#ifdef USE_SDL_AUDIO
#include "ambulant/gui/SDL/sdl_audio.h"
#endif
#ifdef USE_D2_AUDIO
//#include "ambulant/gui/d2/d2_audio.h"
#endif/*WITH_FFMPEG*/

// Select video renderer to use.
// Multiple selections are possible.
// Define the next one to use a builtin open source video renderer.
//#define USE_DS_VIDEO
// Define this one to use the minimal DirectShow video renderer
//#define USE_BASIC_VIDEO
// Define this one to use the more full-featured DirectShow/Direct2D video renderer
#define USE_D2_VIDEO

#ifdef USE_DS_VIDEO
#include "ambulant/gui/d2/d2_dsvideo.h"
#endif
#ifdef USE_BASIC_VIDEO
#include "ambulant/gui/d2/d2_basicvideo.h"
#endif
#ifdef USE_D2_VIDEO
#include "ambulant/gui/d2/d2_d2video.h"
#endif

// "Renderer" playables
//#include "ambulant/gui/d2/d2_audio.h"

// Playables
//#include "ambulant/gui/d2/d2_area.h"

// Layout
#include "ambulant/common/region.h"
#include "ambulant/smil2/smil_layout.h"

// Xerces parser
#ifdef WITH_XERCES_BUILTIN
#include "ambulant/lib/xerces_parser.h"
#endif

// Datasources
#include "ambulant/net/datasource.h"
#include "ambulant/net/win32_datasource.h"
#ifdef WITH_FFMPEG
#include "ambulant/net/ffmpeg_factory.h"
#include "ambulant/net/rtsp_factory.h"
#endif

#include <d2d1.h>
#include <wincodec.h>

//#define AM_DBG
#ifndef AM_DBG
#define AM_DBG if(0)
#endif

using namespace ambulant;

inline D2D1_RECT_F d2_rectf(ambulant::lib::rect r) {
	return D2D1::RectF((float) r.left(), (float) r.top(), (float) r.right(), (float) r.bottom());
}

int gui::dx::dx_gui_region::s_counter = 0;

gui::d2::d2_player::d2_player(
	d2_player_callbacks &hoster,
	common::player_feedback *feedback,
	const net::url& u)
:	m_d2d(NULL),
	m_WICFactory(NULL),
	m_hoster(hoster),
	m_update_event(0),
	m_cur_wininfo(NULL),
	m_transition_rendertarget(NULL),
	m_fullscreen_count(0),
	m_fullscreen_engine(NULL),
	m_fullscreen_now(0),
	m_fullscreen_outtrans(false),
	m_fullscreen_ended(false),
	m_fullscreen_cur_bitmap(NULL),
	m_fullscreen_orig_bitmap(NULL),
	m_fullscreen_old_bitmap(NULL),
	m_fullscreen_rendertarget(NULL),
	m_logger(lib::logger::get_logger())
{
	set_embedder(this);
	// Fill the factory objects
	init_factories();
	init_plugins();

	// Create Direct2D factory
	HRESULT hr;
	hr = CoInitialize(NULL);
	assert(SUCCEEDED(hr));
	D2D1_FACTORY_OPTIONS options = { D2D1_DEBUG_LEVEL_INFORMATION };
	hr = D2D1CreateFactory(
		D2D1_FACTORY_TYPE_MULTI_THREADED,
		IID_ID2D1Factory,
		&options,
		(void**)&m_d2d);
	if (!SUCCEEDED(hr)) {
		m_logger->fatal("Cannot initialize Direct2D: error 0x%x", hr);
	}
	// Order the factories according to the preferences
	common::preferences *prefs = common::preferences::get_preferences();
	if (prefs->m_prefer_ffmpeg)
		get_playable_factory()->preferred_renderer(AM_SYSTEM_COMPONENT("RendererOpen"));
	else
		get_playable_factory()->preferred_renderer(AM_SYSTEM_COMPONENT("RendererDirectX"));

	// Parse the provided URL.
	AM_DBG m_logger->debug("Parsing: %s", u.get_url().c_str());
	m_doc = create_document(u);

	if(!m_doc) {
		// message already logged
		return;
	}

	// If there's a fragment ID remember the node it points to,
	// and when we first start playback we'll go there.
	const std::string& idd = u.get_ref();
	if (idd != "") {
		const lib::node *node = m_doc->get_node(idd);
		if (node) {
			goto_node(node);
		} else {
			m_logger->warn(gettext("%s: node ID not found"), idd.c_str());
		}
	}

	// Create a player instance
	AM_DBG m_logger->debug("Creating player instance for: %s", u.get_url().c_str());
	m_player = smil2::create_smil2_player(m_doc, this, m_embedder);

	if (feedback) m_player->set_feedback(feedback);
	m_player->initialize();
	lib::event_processor *evp = m_player->get_evp();
	assert(evp);
	evp->set_observer(this);

	// Create a worker processor instance
}

gui::d2::d2_player::~d2_player() {

	SafeRelease(&m_fullscreen_cur_bitmap);
	SafeRelease(&m_fullscreen_orig_bitmap);
	SafeRelease(&m_fullscreen_old_bitmap);
	set_fullscreen_rendertarget(NULL); // just to be sure

	lib::event_processor *evp = NULL;
	if(m_player) stop();
	if (m_player) {
		evp = m_player->get_evp();
		if (evp) evp->set_observer(NULL);
		m_player->terminate();
		m_player->release();
		m_player = NULL;
	}
	while(!m_frames.empty()) {
		frame *pf = m_frames.top();
		m_frames.pop();
		m_windows = pf->windows;
		m_player = pf->player;
		m_doc = pf->doc;
		delete pf;
		stop();
		if (m_player) {
			evp = m_player->get_evp();
			if (evp) evp->set_observer(NULL);
			m_player->terminate();
			m_player->release();
			m_player = NULL;
		}
		delete m_doc;
	}
	delete m_doc;
	m_player = NULL;
	assert(m_windows.empty());
	if(gui::dx::dx_gui_region::s_counter != 0)
		m_logger->warn("Undeleted gui regions: %d", dx::dx_gui_region::s_counter);
	assert(m_d2d);
	m_d2d->Release();
	m_d2d = NULL;
	CoUninitialize();
}

void
gui::d2::d2_player::cleanup()
{
	lib::nscontext::cleanup();
	common::global_playable_factory *pf = common::get_global_playable_factory();
	delete pf;
	lib::global_parser_factory *prf = lib::global_parser_factory::get_parser_factory();
	delete prf;
#ifdef WITH_SMIL30
	common::global_state_component_factory *scf = common::get_global_state_component_factory();
	delete scf;
#endif
}

void
gui::d2::d2_player::init_playable_factory()
{
	common::global_playable_factory *pf = common::get_global_playable_factory();
	set_playable_factory(pf);
	pf->add_factory(create_d2_fill_playable_factory(this, this));
	pf->add_factory(create_d2_image_playable_factory(this, this));
	pf->add_factory(create_d2_text_playable_factory(this, this));
	pf->add_factory(create_d2_smiltext_playable_factory(this, this));
#ifdef USE_BASIC_VIDEO
	pf->add_factory(create_d2_basicvideo_playable_factory(this, this));
#endif
#ifdef USE_D2_VIDEO
	pf->add_factory(create_d2_d2video_playable_factory(this, this));
#endif
#if 0
	// Add the playable factory
	pf->add_factory(create_d2_area_playable_factory(this, this));
#ifdef USE_D2_AUDIO
	pf->add_factory(create_d2_audio_playable_factory(this, this));
#endif
#ifdef USE_SDL_AUDIO
	pf->add_factory(gui::sdl::create_sdl_playable_factory(this));
#endif
#ifdef WITH_HTML_WIDGET
	pf->add_factory(create_d2_html_playable_factory(this, this));
#endif
#ifdef USE_DS_VIDEO
	pf->add_factory(create_d2_dsvideo_playable_factory(this, this));
#endif
#endif
}

void
gui::d2::d2_player::init_window_factory()
{
		set_window_factory(this);
}

void
gui::d2::d2_player::init_datasource_factory()
{
	net::datasource_factory *df = new net::datasource_factory();
	set_datasource_factory(df);
#ifdef WITH_LIVE
	AM_DBG m_logger->debug("d2_player: add live_audio_datasource_factory");
	df->add_video_factory(net::create_live_video_datasource_factory());
	df->add_audio_factory(net::create_live_audio_datasource_factory());
#endif
#ifdef WITH_FFMPEG
	AM_DBG m_logger->debug("d2_player: add ffmpeg_audio_datasource_factory");
	df->add_audio_factory(net::get_ffmpeg_audio_datasource_factory());
	AM_DBG m_logger->debug("d2_player: add ffmpeg_audio_decoder_finder");
	df->add_audio_decoder_finder(net::get_ffmpeg_audio_decoder_finder());
	AM_DBG m_logger->debug("d2_player: add ffmpeg_audio_filter_finder");
	df->add_audio_filter_finder(net::get_ffmpeg_audio_filter_finder());
	AM_DBG m_logger->debug("d2_player: add ffmpeg_video_datasource_factory");
	df->add_video_factory(net::get_ffmpeg_video_datasource_factory());
	AM_DBG m_logger->debug("d2_player: add ffmpeg_raw_datasource_factory");
	df->add_raw_factory(net::get_ffmpeg_raw_datasource_factory());
#endif
	df->add_raw_factory(net::get_win32_datasource_factory());

}

void
gui::d2::d2_player::init_parser_factory()
{
	lib::global_parser_factory *pf = lib::global_parser_factory::get_parser_factory();
	set_parser_factory(pf);
	// Add the xerces parser, if available
#ifdef WITH_XERCES_BUILTIN
	pf->add_factory(new lib::xerces_factory());
#endif
}

void
gui::d2::d2_player::_recreate_d2d(wininfo *wi)
{
	if (wi->m_rendertarget) return;
	assert(wi->m_hwnd);

	RECT rc;
	GetClientRect(wi->m_hwnd, &rc);
	D2D1_SIZE_U size = D2D1::SizeU(rc.right-rc.left, rc.bottom-rc.top);

	HRESULT hr = m_d2d->CreateHwndRenderTarget(
#ifdef	AM_DMP
		D2D1::RenderTargetProperties(D2D1_RENDER_TARGET_TYPE_SOFTWARE),
#else //AM_DMP
		D2D1::RenderTargetProperties(D2D1_RENDER_TARGET_TYPE_SOFTWARE),
#endif//AM_DMP
		D2D1::HwndRenderTargetProperties(wi->m_hwnd, size, D2D1_PRESENT_OPTIONS_RETAIN_CONTENTS),
		&wi->m_rendertarget);

	if (!SUCCEEDED(hr))
		lib::win32::win_trace_error("CreateHwndRenderTarget", hr);
}

void
gui::d2::d2_player::_discard_d2d()
{
	std::map<std::string, wininfo*>::iterator it;
	for(it=m_windows.begin();it!=m_windows.end();it++) {
		wininfo *wi = it->second;
		if (wi->m_rendertarget) {
			wi->m_rendertarget->Release();
			wi->m_rendertarget = NULL;
		}
	}
	std::set<d2_resources*>::iterator rit;
	for(rit=m_resources.begin(); rit!=m_resources.end(); rit++) {
		(*rit)->discard_d2d();
	}
}

void gui::d2::d2_player::play() {
	if(m_player) {
		lock_redraw();
		std::map<std::string, wininfo*>::iterator it;
		for(it=m_windows.begin();it!=m_windows.end();it++) {
			d2_window *d2win = (d2_window *)(*it).second->m_window;
			d2win->redraw();
		}
		common::gui_player::play();
		unlock_redraw();
	}
}

void gui::d2::d2_player::stop() {
	if(m_player) {
		m_update_event = 0;
		_clear_transitions();
		common::gui_player::stop();
	}
	_discard_d2d();
}

void gui::d2::d2_player::pause() {
	if(m_player) {
		lock_redraw();
		common::gui_player::pause();
		unlock_redraw();
	}
}

void gui::d2::d2_player::restart(bool reparse) {
	bool playing = is_play_active();
	stop();
	lib::event_processor *evp = m_player->get_evp();
	if (evp) evp->set_observer(NULL);

	assert(m_player);
	m_player->terminate();
	m_player->release();
	m_player = NULL;
	while(!m_frames.empty()) {
		frame *pf = m_frames.top();
		m_frames.pop();
		m_windows = pf->windows;
		evp = m_player->get_evp();
		if (evp) evp->set_observer(NULL);
		m_player = pf->player;
		assert(m_player);
		m_doc = pf->doc;
		delete pf;
		stop();
		m_player->terminate();
		m_player->release();
		m_player = NULL;
	}
	m_player = 0;
	if (reparse) {
		m_doc = create_document(m_url);
		if(!m_doc) {
			m_logger->show("Failed to parse document %s", m_url.get_url().c_str());
			return;
		}
	}
	AM_DBG m_logger->debug("Creating player instance for: %s", m_url.get_url().c_str());
	m_player = smil2::create_smil2_player(m_doc, this, m_embedder);
	m_player->initialize();
	evp = m_player->get_evp();
	if (evp) evp->set_observer(this);
	if(playing) play();
}

void gui::d2::d2_player::on_click(int x, int y, HWND hwnd) {
	if(!m_player) return;
	lib::point pt(x, y);
	d2_window *d2win = (d2_window *) _get_window(hwnd);
	if(!d2win) return;
	region *r = d2win->get_region();
	if(r)
		r->user_event(pt, common::user_event_click);
}

int gui::d2::d2_player::get_cursor(int x, int y, HWND hwnd) {
	if(!m_player) return 0;
	lib::point pt(x, y);
	d2_window *d2win = (d2_window *) _get_window(hwnd);
	if(!d2win) return 0;
	region *r = d2win->get_region();
	m_player->before_mousemove(0);
	if(r) r->user_event(pt, common::user_event_mouse_over);
	return m_player->after_mousemove();
}

std::string gui::d2::d2_player::get_pointed_node_str() {
#if 1
	return "";
#else
	if(!m_player || !is_playing()) return "";
	return m_player->get_pointed_node_str();
#endif
}

void gui::d2::d2_player::on_char(int ch) {
	if(m_player) m_player->on_char(ch);
}

bool gui::d2::d2_player::_calc_fit(
	const RECT& dstrect,
	const lib::size& srcsize,
	float& xoff,
	float& yoff,
	float& fac)
{
	// First some sanity checks
	if (srcsize.w == 0 || srcsize.h == 0) return false;
	float w = (float) dstrect.right-dstrect.left;
	float h = (float) dstrect.bottom-dstrect.top;
	if (w == 0 || h == 0) return false;

	// Compute sizes if we simply want to fill the area
	fac = w / srcsize.w;
	if (h / srcsize.h < fac) fac = h / srcsize.h;

	// Now check whether we should scale at all
	bool scale_up = false;
	bool scale_down = true;
	if (!scale_up && fac > 1) {
		fac = 1;
	}
	if (!scale_down && fac < 1) {
		fac = 1;
	}

	// Now compute "spare" pixels
	float spare_x = w - (fac*srcsize.w);
	float spare_y = h - (fac*srcsize.h);
	if (spare_x < 0) spare_x = 0;
	if (spare_y < 0) spare_y = 0;

	// Compute offset
	xoff = spare_x / 2;
	yoff = spare_y / 2;

	return xoff != 0 || yoff != 0 || fac != 1;
}

void gui::d2::d2_player::redraw(HWND hwnd, HDC hdc, RECT *dirty) {

	HRESULT hr = S_OK;
	// Create the Direct2D resources, in case they were lost
	wininfo *wi = _get_wininfo(hwnd);
	assert(wi);
	if (wi == NULL) return;
	_recreate_d2d(wi);
	m_cur_wininfo = wi;
	ID2D1HwndRenderTarget *rt = wi->m_rendertarget;
	if (rt == NULL) return;

	// Check whether our window changed size. If so: communicate to d2d and
	// paint background again.
	RECT client_rect;
	BOOL ok = GetClientRect(hwnd, &client_rect);
	assert(ok);
	BOOL changed_size = !EqualRect(&client_rect, &wi->m_rect);
	if (changed_size) {
		wi->m_rect = client_rect;
		dirty = NULL;
		D2D1_SIZE_U new_size = { client_rect.right-client_rect.left, client_rect.bottom-client_rect.top };
		rt->Resize(new_size);
	}
	rt->BeginDraw();

	// Set the transformation
	const lib::rect& wanted_rect = wi->m_window->get_rect();
	float xoff, yoff, factor;
	AM_DBG lib::logger::get_logger()->debug("d2_player::redraw(%d, %d, %d, %d)", client_rect.left, client_rect.top, client_rect.right, client_rect.bottom);
	if (_calc_fit(client_rect, wanted_rect.size(), xoff, yoff, factor)) {
		// WE have to do scaling. Setup the matrix.
		D2D1_MATRIX_3X2_F transform = {
			factor, 0,
			0, factor,
			xoff, yoff
		};
		AM_DBG lib::logger::get_logger()->debug("d2_player::redraw offset %f,%f factor %f", xoff, yoff, factor);
		rt->SetTransform(transform);
		// Lazy programmer alert: we cannot use the dirty rect as-is, 
		// need to do the transform, at some time. For now we clear it.
		dirty = NULL;
	} else {
		rt->SetTransform(D2D1::Matrix3x2F::Identity());
	}

	// Do the redraw
	_screenTransitionPreRedraw(rt);
	if (dirty) {
		lib::rect r(lib::point(dirty->left, dirty->top), lib::size(dirty->right-dirty->left, dirty->bottom-dirty->top));
		wi->m_window->redraw(r);
		_screenTransitionPostRedraw(&r);
	} else {
		wi->m_window->redraw();
		_screenTransitionPostRedraw(NULL);
	}
	hr = rt->EndDraw();
	// after each redraw a bitmap screen copy is made for possible fullscreen transitions
	_set_fullscreen_cur_bitmap(rt); 
#ifdef	AM_DMP
//	dump (rt, "d2_player-redraw2");
#endif//AM_DMP
	m_cur_wininfo = NULL;
	if (hr == D2DERR_RECREATE_TARGET) {
		// This happens if something serious changed (like move to a
		// different display). Throw away evertyhing that is device-dependent,
		// it will be re-created next time around.
		_discard_d2d();
	} else {
		// Handle capture callbacks.
		std::list<std::pair<lib::rect, d2_capture_callback *> >::iterator it;
		for(it=m_captures.begin(); it != m_captures.end(); it++) {
			d2_capture_callback *cb = it->second;
			IWICBitmap *bitmap = _capture_wic(it->first, rt);
			cb->captured(bitmap);
		}
		m_captures.clear();
	}
}

ID2D1Bitmap *
gui::d2::d2_player::_capture_bitmap(lib::rect r, ID2D1RenderTarget *src_rt, ID2D1RenderTarget *dst_rt)
{
	HRESULT hr;
	D2D1_SIZE_U src_size = { r.width(), r.height() };
	if (r.size() == lib::size(0,0)) {
		D2D1_SIZE_F rt_size = src_rt->GetSize();
		src_size.width = (UINT32) rt_size.width;
		src_size.height = (UINT32) rt_size.height;
	}
//	D2D1_RECT_U src_rect = { r.left(), r.top(), r.left()+src_size.width, r.top()+src_size.height };
	ID2D1Bitmap *bitmap;
	D2D1_BITMAP_PROPERTIES props = D2D1::BitmapProperties();
	D2D1_PIXEL_FORMAT rt_format = src_rt->GetPixelFormat();
	props.pixelFormat = rt_format;
	src_rt->GetDpi(&props.dpiX, &props.dpiY);
	hr = dst_rt->CreateBitmap(src_size, props, &bitmap);
	if (!SUCCEEDED(hr)) {
		lib::win32::win_trace_error("capture: CreateBitmap", hr);
		return NULL;
	}
//	D2D1_POINT_2U dst_point = { 0, 0};
	hr = bitmap->CopyFromRenderTarget(NULL, src_rt, NULL);
	if (!SUCCEEDED(hr)) {
		lib::win32::win_trace_error("capture: CopyFromRenderTarget", hr);
		bitmap->Release();
		return NULL;
	}
	return bitmap;
}

IWICBitmap *
gui::d2::d2_player::_capture_wic(lib::rect r, ID2D1RenderTarget *src_rt)
{
	if (src_rt == NULL)
		return NULL;
	D2D1_SIZE_F src_size = src_rt->GetSize();

	HRESULT hr;
	// Create the WIC factory, if it hasn'tbeen done earlier
	if (m_WICFactory == NULL) {
		hr = CoCreateInstance(
			CLSID_WICImagingFactory,
			NULL,
			CLSCTX_INPROC_SERVER,
			IID_IWICImagingFactory,
			reinterpret_cast<void **>(&m_WICFactory)
			);
		if (!SUCCEEDED(hr)) {
			lib::win32::win_trace_error("capture: CoCreateInstance(IWICImagingFactory)", hr);
			return NULL;
		}
	}

	// Create the WIC bitmap
	if (r.size() == lib::size(0,0)) {
		r = lib::rect(lib::point(0,0), lib::size(int(src_size.width), int(src_size.height)));
	}
	IWICBitmap *bitmap;
    hr = m_WICFactory->CreateBitmap(
        r.width(),
        r.height(),
        GUID_WICPixelFormat32bppBGR,
        WICBitmapCacheOnLoad,
        &bitmap
        );
	if (!SUCCEEDED(hr)) {
		lib::win32::win_trace_error("capture: WIC CreateBitmap", hr);
		return NULL;
	}

	// Create the D2D render target for the new bitmap
	ID2D1RenderTarget *dst_rt = NULL;
	hr = m_d2d->CreateWicBitmapRenderTarget(
        bitmap,
        D2D1::RenderTargetProperties(), //D2D1_RENDER_TARGET_TYPE_SOFTWARE,D2D1::PixelFormat(DXGI_FORMAT_B8G8R8X8_UNORM,D2D1_ALPHA_MODE_IGNORE)),
        &dst_rt
        );
	if (!SUCCEEDED(hr)) {
		lib::win32::win_trace_error("capture: CreateWicBitmapRenderTarget", hr);
		return NULL;
	}

	// Copy the data from the old render target to a bitmap in
	// the new target, then render it.
	ID2D1Bitmap *src_bitmap = _capture_bitmap(r, src_rt, dst_rt);
	if (src_bitmap) {
		dst_rt->BeginDraw();
		D2D1_RECT_F dst_rect =  d2_rectf(r); //X{ r.left(), r.top(), r.right(), r.bottom() };
		dst_rt->DrawBitmap(src_bitmap, dst_rect);
		hr = dst_rt->EndDraw();
		// Ignore result, nothing we can do.
	} else {
		// Creating the D2D bitmap failed. Release the result.
		bitmap->Release();
		bitmap = NULL;
	}

	// We can release the drawing stuff, now
	if (src_bitmap) src_bitmap->Release();
	dst_rt->Release();

	return bitmap;
}


void gui::d2::d2_player::on_done() {
#ifdef JNK
	std::map<std::string, wininfo*>::iterator it;
	for(it=m_windows.begin();it!=m_windows.end();it++) {
		(*it).second->v->clear();
		(*it).second->v->redraw();
	}
#endif
}

void gui::d2::d2_player::lock_redraw() {
	std::map<std::string, wininfo*>::iterator it;
	for(it=m_windows.begin();it!=m_windows.end();it++) {
		(*it).second->m_window->lock_redraw();
	}
}

void gui::d2::d2_player::unlock_redraw() {
	std::map<std::string, wininfo*>::iterator it;
	for(it=m_windows.begin();it!=m_windows.end();it++) {
		(*it).second->m_window->unlock_redraw();
	}
}

////////////////////
// common::window_factory implementation

common::gui_window *
gui::d2::d2_player::new_window(const std::string &name,
	lib::size bounds, common::gui_events *src) {

	AM_DBG lib::logger::get_logger()->debug("d2_window_factory::new_window(%s): %s",
		name.c_str(), ::repr(bounds).c_str());

	// wininfo struct that will hold the associated objects
	wininfo *winfo = new wininfo;

	// Create an os window
	winfo->m_hwnd = m_hoster.new_os_window();
	assert(winfo->m_hwnd);
#if 1
	// Set "old" rect to empty, so the first redraw will recalculate the matrix
	SetRectEmpty(&winfo->m_rect);
#else
	GetClientRect(winfo->m_hwnd, &winfo->m_rect);
#endif

	// Rendertarget will be created on-demand
	winfo->m_rendertarget = NULL;
	// Region?
	region *rgn = (region *) src;
	bool is_fullscreen = false;
	HWND parent_hwnd = GetParent(winfo->m_hwnd);
	if (parent_hwnd) {
		long parent_style = GetWindowLong(parent_hwnd, GWL_STYLE);
		if ( (parent_style & WS_CAPTION) == 0) 
			is_fullscreen = true;
	}
	if  (!is_fullscreen) {
		// Set window size
		int w = bounds.w;
		int h = bounds.h;
		int borders_w = 20; // XXXX
		int borders_h = 60; // XXXX
		float factor = 1.0;
		RECT desktop_rect;
#if 0
		GetWindowRect(::GetDesktopWindow(), &desktop_rect);
#else
		SystemParametersInfo(SPI_GETWORKAREA, 0, &desktop_rect, 0);
#endif
		if (w > desktop_rect.right-desktop_rect.left - borders_w) {
			factor = float(desktop_rect.right-desktop_rect.left - borders_w) / w;
		}
		if (h > desktop_rect.bottom - desktop_rect.top - borders_h) {
			float f2 = float(desktop_rect.bottom - desktop_rect.top - borders_h) / h;
			if (f2 < factor) factor = f2;
		}
		if (factor < 1.0) {
			w = int(w*factor);
			h = int(h*factor);
			// XXX To DO: position correctly on screen too.
		}
		PostMessage(winfo->m_hwnd, WM_SET_CLIENT_RECT, w, h);
	}

	// Create a concrete gui_window
	winfo->m_window = new d2_window(name, bounds, rgn, this, winfo->m_hwnd);

	// Store the wininfo struct
	m_windows[name] = winfo;
	AM_DBG m_logger->debug("windows: %d", m_windows.size());

	// Return gui_window
	return winfo->m_window;
}

void
gui::d2::d2_player::window_done(const std::string &name) {
	// called when the window is destructed (wi->w)
	std::map<std::string, wininfo*>::iterator it = m_windows.find(name);
	assert(it != m_windows.end());
	wininfo *wi = (*it).second;
	m_windows.erase(it);
#ifdef JNK
	wi->v->clear();
	wi->v->redraw();
	delete wi->v;
#endif
	if (wi->m_rendertarget) {
		wi->m_rendertarget->Release();
		wi->m_rendertarget = NULL;
	}
	m_hoster.destroy_os_window(wi->m_hwnd);
	delete wi;
	AM_DBG m_logger->debug("windows: %d", m_windows.size());
}

lib::size
gui::d2::d2_player::get_default_size() {
	SIZE sz = m_hoster.get_default_size();
	if (sz.cx != 0 && sz.cy != 0)
		return lib::size(sz.cx, sz.cy);
	return lib::size(common::default_layout_width, common::default_layout_height);
}

common::bgrenderer*
gui::d2::d2_player::new_background_renderer(const common::region_info *src) {
	return new d2_background_renderer(src, this);
}

gui::d2::d2_player::wininfo*
gui::d2::d2_player::_get_wininfo(HWND hwnd) {
	wininfo *winfo = 0;
	std::map<std::string, wininfo*>::iterator it;
	for(it=m_windows.begin();it!=m_windows.end();it++) {
		wininfo *wi = (*it).second;
		if(wi->m_hwnd == hwnd) {winfo = wi;break;}
	}
	return winfo;
}

gui::d2::d2_player::wininfo*
gui::d2::d2_player::_get_wininfo(d2_window *window) {
	wininfo *winfo = 0;
	std::map<std::string, wininfo*>::iterator it;
	for(it=m_windows.begin();it!=m_windows.end();it++) {
		wininfo *wi = (*it).second;
		if(wi->m_window == window) {winfo = wi;break;}
	}
	return winfo;
}

common::gui_window *
gui::d2::d2_player::_get_window(HWND hwnd) {
	wininfo *wi = _get_wininfo(hwnd);
	return wi?wi->m_window:0;
}

HWND
gui::d2::d2_player::_get_main_window() {
	// This assumes the main window is the first window
	std::map<std::string, wininfo*>::iterator it = m_windows.begin();
	if (it == m_windows.end()) return NULL;
	return (*it).second->m_hwnd;
}

void gui::d2::d2_player::set_intransition(common::playable *p, const lib::transition_info *info) {
#ifdef D2NOTYET
	AM_DBG lib::logger::get_logger()->debug("set_intransition : %s", repr(info->m_type).c_str());
	d2_transition *tr = set_transition(p, info, false);
	// XXXX Note by Jack: the next two steps really shouldn't be done for
	// intransitions, they should be done later (when playback starts).
	tr->first_step();
	if(!m_update_event) schedule_update();
#endif
}

void gui::d2::d2_player::start_outtransition(common::playable *p, const lib::transition_info *info) {
#ifdef D2NOTYET
	lib::logger::get_logger()->debug("start_outtransition : %s", repr(info->m_type).c_str());
	d2_transition *tr = set_transition(p, info, true);
	// XXXX Note by Jack: the next two steps really shouldn't be done for
	// intransitions, they should be done later (when playback starts).
	tr->first_step();
	if(!m_update_event) schedule_update();
#endif
}

gui::d2::d2_transition *
gui::d2::d2_player::_set_transition(
	common::playable *p,
	const lib::transition_info *info,
	bool is_outtransition)
{
#ifndef D2NOTYET
	assert(m_player);
	lib::timer_control *timer = new lib::timer_control_impl(m_player->get_timer(), 1.0, false);
// TBD see: dx_transition	d2_transition *tr = make_transition(info->m_type, p, timer);
//	m_trmap[p] = tr;
	common::surface *surf = p->get_renderer()->get_surface();
	if (info->m_scope == lib::scope_screen) surf = surf->get_top_surface();
//	tr->init(surf, is_outtransition, info);
//	return tr;
//#else
	return NULL;
#endif
}

bool gui::d2::d2_player::_has_transitions() const {
	return !m_trmap.empty();
}

void gui::d2::d2_player::_update_transitions() {
#ifdef D2NOTYET
	m_trmap_cs.enter();
	assert(m_player);
	lib::timer::time_type pt = m_player->get_timer()->elapsed();
	//lock_redraw();
	AM_DBG lib::logger::get_logger()->debug("update_transitions: updating %d transitions", m_trmap.size());
	// First make all transitions do a step.
	std::vector<common::playable*> finished_transitions;
	for(trmap_t::iterator it=m_trmap.begin();it!=m_trmap.end();it++) {
		if(!(*it).second->next_step(pt)) {
			finished_transitions.push_back((*it).first);
		}
	}
	// Next clean up all finished transitions
	std::vector<common::playable*>::iterator pit;
	for (pit=finished_transitions.begin(); pit!= finished_transitions.end(); pit++) {
		// Find the transition map entry and clear it
		trmap_t::iterator it=m_trmap.find(*pit);
		assert(it != m_trmap.end());
		if (it == m_trmap.end()) continue;
		delete (*it).second;
		it = m_trmap.erase(it);
		// Find the surface and tell it a transition has finished.
		common::renderer *r = (*pit)->get_renderer();
		if (r) {
			common::surface *surf = r->get_surface();
			if (surf) surf->transition_done();
		}
	}
	//unlock_redraw();
	m_trmap_cs.leave();
#endif
}

void gui::d2::d2_player::_clear_transitions() {
#ifdef D2NOTYET
	m_trmap_cs.enter();
	for(trmap_t::iterator it=m_trmap.begin();it!=m_trmap.end();it++)
		delete (*it).second;
	m_trmap.clear();
	m_trmap_cs.leave();
#endif
}

gui::d2::d2_transition *
gui::d2::d2_player::_get_transition(common::playable *p) {
	trmap_t::iterator it = m_trmap.find(p);
	return (it != m_trmap.end())?(*it).second:0;
}

void gui::d2::d2_player::stopped(common::playable *p) {
	m_trmap_cs.enter();
	trmap_t::iterator it = m_trmap.find(p);
	if(it != m_trmap.end()) {
		delete (*it).second;
		common::playable *p = (*it).first;
		it = m_trmap.erase(it);
		common::renderer *r = p->get_renderer();
		if (r) {
			common::surface *surf = r->get_surface();
			if (surf) surf->transition_done();
		}
	}
	m_trmap_cs.leave();
}

void gui::d2::d2_player::paused(common::playable *p) {
#ifdef D2NOTYET
	trmap_t::iterator it = m_trmap.find(p);
	if(it != m_trmap.end()) {
		(*it).second->pause();
	}
#endif
}

void gui::d2::d2_player::resumed(common::playable *p) {
#ifdef D2NOTYET
	trmap_t::iterator it = m_trmap.find(p);
	if(it != m_trmap.end()) {
		(*it).second->resume();
	}
#endif
}

void gui::d2::d2_player::_update_callback() {
	if(!m_update_event) return;
	if(_has_transitions()) {
		_update_transitions();
		_schedule_update();
	} else {
		m_update_event = 0;
	}
}

void gui::d2::d2_player::_schedule_update() {
	if(!m_player) return;
	lib::event_processor *evp = m_player->get_evp();
	m_update_event = new lib::no_arg_callback_event<d2_player>(this,
		&d2_player::_update_callback);
	evp->add_event(m_update_event, 50, lib::ep_high);
}

////////////////////////
// Layout helpers with a lot of hacks

typedef common::surface_template iregion;
typedef common::surface_impl region;

static const region*
get_top_layout(smil2::smil_layout_manager *layout, const lib::node* n) {
	iregion *ir = layout->get_region(n);
	if(!ir) return 0;
	const region *r = (const region*) ir;
	while(r->get_parent()) r = r->get_parent();
	return r;
}

static const char*
get_top_layout_name(smil2::smil_layout_manager *layout, const lib::node* n) {
	const region* r = get_top_layout(layout, n);
	if(!r) return 0;
	const common::region_info *ri = r->get_info();
	return ri?ri->get_name().c_str():0;
}

void gui::d2::d2_player::show_file(const net::url& href) {
	ShellExecute(GetDesktopWindow(), text_str("open"), lib::textptr(href.get_url().c_str()), NULL, NULL, SW_SHOWNORMAL);
}


void gui::d2::d2_player::done(common::player *p) {
	m_update_event = 0;
	_clear_transitions();
	if(!m_frames.empty()) {
		frame *pf = m_frames.top();
		m_frames.pop();
		m_windows = pf->windows;
		m_player = pf->player;
		m_doc = pf->doc;
		delete pf;
		assert(0); // resume();
		std::map<std::string, wininfo*>::iterator it;
		for(it=m_windows.begin();it!=m_windows.end();it++) {
			d2_window *d2win = (*it).second->m_window;
			d2win->redraw();
		}
	}
}

void gui::d2::d2_player::close(common::player *p) {
	PostMessage(_get_main_window(), WM_CLOSE, 0, 0);
}

void gui::d2::d2_player::open(net::url newdoc, bool startnewdoc, common::player *old) {
	std::string urlstr = newdoc.get_url();
	if(old) {
		// Replace the current document
		PostMessage(_get_main_window(), WM_REPLACE_DOC,
			startnewdoc?1:0, LPARAM(new std::string(urlstr)));
		return;
	}
	if(!lib::ends_with(urlstr, ".smil") && !lib::ends_with(urlstr, ".smi") &&
		!lib::ends_with(urlstr, ".grins")) {
		show_file(newdoc);
		return;
	}

	// Parse the document.
	m_doc = create_document(newdoc);
	if (!m_doc) return;

	// Push the old frame on the stack
	if(m_player) {
		pause();
		frame *pf = new frame();
		pf->windows = m_windows;
		pf->player = m_player;
		pf->doc = m_doc;
		m_windows.clear();
		m_player = 0;
		m_frames.push(pf);
	}

	// Create a player instance
	AM_DBG m_logger->debug("Creating player instance for: %s", newdoc.get_url().c_str());
	m_player = smil2::create_smil2_player(m_doc, this, m_embedder);
	m_player->initialize();
	lib::event_processor *evp = m_player->get_evp();
	assert(evp);
	evp->set_observer(this);
	if(startnewdoc) play();
}

// Full screen transition support
void
gui::d2::d2_player::start_screen_transition(bool outtrans)
{
	AM_DBG lib::logger::get_logger()->debug("d2_player::start_screen_transition()");
	if (m_fullscreen_count)
		logger::get_logger()->warn(gettext("%s:multiple screen transitions in progress (m_fullscreen_count=%d)"),"ambulant_qt_window::startScreenTransition()",m_fullscreen_count);
	m_fullscreen_count++;
	m_fullscreen_outtrans = outtrans;
}

void
gui::d2::d2_player::end_screen_transition()
{
//	assert(m_fullscreen_count > 0);
	if (m_fullscreen_count == 0) {
		return;
	}
	AM_DBG lib::logger::get_logger()->debug("d2_player::end_screen_transition()");
	m_fullscreen_count--;
	if (m_fullscreen_count == 0) {
		set_fullscreen_rendertarget(NULL);
	}
}

void
gui::d2::d2_player::_screenTransitionPreRedraw(ID2D1RenderTarget* rt)
{
	AM_DBG lib::logger::get_logger()->debug("d2_player::_screenTransitionPreRedraw()");
	if (m_fullscreen_count > 0 && m_fullscreen_engine == NULL) {
		// fullscreen in: at first call (m_fullscreen_engine == NULL), save the old screen image
		if (m_fullscreen_outtrans) {
			_set_fullscreen_orig_bitmap(rt);
		} else {
			_set_fullscreen_old_bitmap(rt);
		}
#ifdef	JNK
		RECT d2_rect = this->m_cur_wininfo->m_rect;
		lib::rect r = lib::rect(lib::size(d2_rect.right - d2_rect.left, d2_rect.bottom - d2_rect.top));
		this->m_cur_wininfo->m_window->need_redraw(r);
#endif//JNK
	}
}

void
gui::d2::d2_player::_screenTransitionPostRedraw(ambulant::lib::rect* r)
{
	if (m_fullscreen_count == 0 /*&& fullscreen_oldimage == NULL*/) {
		return;
	}
	if (r == NULL) {
		AM_DBG lib::logger::get_logger()->debug("d2_player::_screenTransitionPostRedraw() r=<NULL>");
	} else {
		AM_DBG lib::logger::get_logger()->debug("d2_player::_screenTransitionPostRedraw() *r=(%d,%d),(%d,%d)", r->left(), r->top(), r->right(), r->bottom());
	}
	AM_DBG lib::logger::get_logger()->debug("_screenTransitionPostRedraw: fullscreen_count=%d fullscreen_engine=0x%x", m_fullscreen_count,m_fullscreen_engine);
	if (m_fullscreen_engine && ! m_fullscreen_ended) {
		m_fullscreen_engine->step(m_fullscreen_now);
	} else {
		AM_DBG lib::logger::get_logger()->debug("_screenTransitionPostRedraw: no screen transition engine");
		if (m_fullscreen_ended) { // fix-up interrupted transition step
//			ambulant::lib::rect fullsrcrect = ambulant::lib::rect(ambulant::lib::point(0, 0), ambulant::lib::size(self.bounds.size.width,self.bounds.size.height));  // Original image size
//			CGRect cg_fullsrcrect = ambulant::gui::cg::CGRectFromAmbulantRect(fullsrcrect);
//			CGContextRef ctx = UIGraphicsGetCurrentContext();	
//			CGContextDrawLayerInRect(ctx, cg_fullsrcrect, [self getTransitionSurface]);
//			[self releaseTransitionSurfaces];
		}		
	}
	
	if (m_fullscreen_count == 0) {
		// Finishing a fullscreen transition.
		AM_DBG lib::logger::get_logger()->debug("_screenTransitionPostRedraw: cleanup after transition done");
		m_fullscreen_engine = NULL;
		m_transition_rendertarget = NULL;
		set_fullscreen_rendertarget(NULL);
	}
}

void
gui::d2::d2_player::screen_transition_step(smil2::transition_engine* engine, lib::transition_info::time_type now)
{
	AM_DBG lib::logger::get_logger()->debug("d2_player::screen_transition_step()");
//	assert(m_fullscreen_count > 0);
	if ( ! (m_fullscreen_count > 0)) {
		return;
	}
	m_fullscreen_engine = engine;
	m_fullscreen_now = now;
}

ID2D1Bitmap*
gui::d2::d2_player::_get_bitmap_from_render_target(ID2D1RenderTarget* rt) {
	// we need to use ID2D1Bitmap::CopyFromRenderTarget, therefore we must create the bitmap
	// where we put the data into ('bitmap_new') with equal properties as its data source ('old_rt')
	if (rt == NULL) {
		assert(rt == NULL);
		return NULL;
	}
	D2D1_SIZE_F d2_size_f= rt->GetSize();
	D2D1_SIZE_U d2_size_u = D2D1::SizeU((UINT32) d2_size_f.width, (UINT32) d2_size_f.height); 
	D2D1_RECT_U d2_rect_u = D2D1::RectU(0U, 0U, d2_size_u.width, d2_size_u.height); 
	D2D1_BITMAP_PROPERTIES props = D2D1::BitmapProperties();
	ID2D1Bitmap* rv = NULL;
	rt->GetDpi(&props.dpiX, &props.dpiY);
	props.pixelFormat = rt->GetPixelFormat();
	HRESULT hr = rt->CreateBitmap(d2_size_u, props, &rv);
	OnErrorGoto_cleanup(hr,"d2_player _get_bitmap_from_render_targetold_rt->CreateBitmap");
	// copy the bits of the old stuff (from 'old_rt') to the new destination
	hr = rv->CopyFromRenderTarget(NULL, rt, &d2_rect_u);
	if (SUCCEEDED(hr)) {
		return rv;
	}
	OnErrorGoto_cleanup(hr,"d2_player _get_bitmap_from_render_target bitmap_old->CopyFromRenderTarget");
cleanup:
	SafeRelease(&rv);
	return rv;
}
// screen capture support
void
gui::d2::d2_player::captured(IWICBitmap *bitmap)
{
	AM_DBG m_logger->debug("d2_player::captured called");
	if (bitmap) bitmap->Release();
}

void
gui::d2::d2_player::_set_fullscreen_cur_bitmap(ID2D1RenderTarget* rt) 
{
	SafeRelease(&this->m_fullscreen_cur_bitmap);
	this->m_fullscreen_cur_bitmap = this->_get_bitmap_from_render_target(rt);
#ifdef	AM_DMP
//	dump_bitmap(m_fullscreen_old_bitmap, rt, "cbm");
#endif//AM_DMP
}

void
gui::d2::d2_player::_set_fullscreen_orig_bitmap(ID2D1RenderTarget* rt) 
{
	SafeRelease(&this->m_fullscreen_orig_bitmap);
	m_fullscreen_orig_bitmap = m_fullscreen_cur_bitmap;
	m_fullscreen_cur_bitmap = NULL; // prevents Release (old_bitmap now has ownership)
#ifdef	AM_DMP
//	dump_bitmap(m_fullscreen_orig_bitmap, rt, "nbm");
#endif//AM_DMP
}

void
gui::d2::d2_player::_set_fullscreen_old_bitmap(ID2D1RenderTarget* rt) 
{
	SafeRelease(&this->m_fullscreen_old_bitmap);
	m_fullscreen_old_bitmap = m_fullscreen_cur_bitmap;
	m_fullscreen_cur_bitmap = NULL; // prevents Release (old_bitmap now has ownership)
#ifdef	AM_DMP
//	dump_bitmap(m_fullscreen_old_bitmap, rt, "obm");
#endif//AM_DMP
}

#ifdef	AM_DMP
// screen dump support (for debugging only).
int
gui::d2::d2_player::dump(ID2D1RenderTarget* rt, std::string id) {
	int rv = -1;
	if (rt == NULL)
		return rv;
	D2D1_SIZE_F sizeF = rt->GetSize();

	// create file name
	static int indx;
	int i = indx++;
	int rvi = i;
	if (indx == 9999) indx = 0;
	std::string filename = ".\\";
	char num[4];
	sprintf(num, "%.4d-", i);
	filename += std::string(num);
	filename += id;
	filename += ".png";
	std::wstring wide_filename = std::wstring(filename.begin(), filename.end());
	const wchar_t* wide_cstr_filename = wide_filename.c_str();

	// create the IWICBitmap from the ID2D1RenderTarget
    IWICBitmap* wicBitmap = NULL;
	IWICStream* wicStream = NULL;
	IWICBitmapEncoder* wicBitmapEncoder = NULL;
	IWICBitmapFrameEncode* wicBitmapFrameEncode = NULL;

	WICPixelFormatGUID format = GUID_WICPixelFormatDontCare;
	HRESULT hr = 0;
	if ((wicBitmap = _capture_wic(lib::rect(), rt)) == NULL)
		goto cleanup;
	hr = m_WICFactory->CreateStream(&wicStream);
	OnErrorGoto_cleanup(hr, "dump() m_WICFactory->CreateStream");
	hr = wicStream->InitializeFromFilename(wide_cstr_filename, GENERIC_WRITE);
	OnErrorGoto_cleanup(hr, "dump() wicStream->InitializeFromFilename");
	hr = m_WICFactory->CreateEncoder(GUID_ContainerFormatPng, NULL, &wicBitmapEncoder);
	OnErrorGoto_cleanup(hr, "dump() m_WICFactory->CreateEncoder");
	hr = wicBitmapEncoder->Initialize(wicStream, WICBitmapEncoderNoCache);
	OnErrorGoto_cleanup(hr, "dump() wicBitmapEncoder->Initialize");
	hr = wicBitmapEncoder->CreateNewFrame(&wicBitmapFrameEncode, NULL);
	OnErrorGoto_cleanup(hr, "dump() wicBitmapEncoder->CreateNewFrame");
	hr = wicBitmapFrameEncode->Initialize(NULL);
	OnErrorGoto_cleanup(hr, "dump() wicBitmapFrameEncode->Initialize");
	hr = wicBitmapFrameEncode->SetSize((UINT) sizeF.width, (UINT) sizeF.height);
	OnErrorGoto_cleanup(hr, "dump() wicBitmapFrameEncode->SetSize");
	hr = wicBitmapFrameEncode->SetPixelFormat(&format);
	OnErrorGoto_cleanup(hr, "dump() hr = wicBitmapFrameEncode->SetPixelFormat");
	hr = wicBitmapFrameEncode->WriteSource(wicBitmap, NULL);
	OnErrorGoto_cleanup(hr, "dump() wicBitmapFrameEncode->WriteSource");
	hr = wicBitmapFrameEncode->Commit();
	OnErrorGoto_cleanup(hr, "dump() wicBitmapFrameEncode->Commit");
	hr = wicBitmapEncoder->Commit();
	OnErrorGoto_cleanup(hr, "dump() wicBitmapEncoder->Commit");
	rv = rvi;
cleanup:
	SafeRelease(&wicBitmap);
	SafeRelease(&wicStream);
	SafeRelease(&wicBitmapEncoder);
	SafeRelease(&wicBitmapFrameEncode);

	return rv;
}

int
gui::d2::d2_player::dump_bitmap(ID2D1Bitmap* bmp, ID2D1RenderTarget* rt, std::string id)
{
	if (bmp == NULL || rt == NULL) {
		return -1;
	}
	int rv = -1;
	D2D1_SIZE_F size_f = bmp->GetSize();
	D2D1_RECT_F rect_f = D2D1::RectF(0.0F, 0.0F, size_f.width, size_f.height);
	ID2D1BitmapRenderTarget* bmrt = NULL;
	HRESULT hr = rt->CreateCompatibleRenderTarget(size_f, &bmrt);
	OnErrorGoto_cleanup(hr, "dump_bitmap() CreateCompatibleRenderTarget");
	bmrt->BeginDraw();
	bmrt->DrawBitmap(bmp, rect_f);
	hr = bmrt->EndDraw();
	OnErrorGoto_cleanup(hr, "dump_bitmap() DrawBitmap");
	rv = this->dump(rt, id);

cleanup:
	SafeRelease(&bmrt);
	return rv;
}

#endif 
