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

#include "ambulant/gui/dx/dx_img_wic.h"
#include "ambulant/gui/dx/dx_window.h"
#include "ambulant/gui/dx/dx_image_renderer.h"
#include "ambulant/gui/dx/dx_transition.h"
#include "ambulant/lib/node.h"
#include "ambulant/lib/logger.h"
#include "ambulant/lib/textptr.h"
#include "ambulant/lib/colors.h"
#include "ambulant/common/region_info.h"
#include "ambulant/smil2/test_attrs.h"

#ifdef WITH_WIC
#include <math.h>
#include <ddraw.h>

//#define AM_DBG

#ifndef AM_DBG
#define AM_DBG if(0)
#endif

using namespace ambulant;

using namespace ambulant;
extern const char dx_img_wic_playable_tag[] = "img";
extern const char dx_img_wic_playable_renderer_uri[] = AM_SYSTEM_COMPONENT("RendererDirectX");
extern const char dx_img_wic_playable_renderer_uri2[] = AM_SYSTEM_COMPONENT("RendererWicImg");
extern const char dx_img_wic_playable_renderer_uri3[] = AM_SYSTEM_COMPONENT("RendererImg");

IWICImagingFactory *gui::dx::dx_img_wic_renderer::s_wic_factory = NULL;

common::playable_factory *
gui::dx::create_dx_image_wic_playable_factory(common::factories *factory, common::playable_factory_machdep *mdp)
{
	smil2::test_attrs::set_current_system_component_value(AM_SYSTEM_COMPONENT("RendererDirectX"), true);
	smil2::test_attrs::set_current_system_component_value(AM_SYSTEM_COMPONENT("RendererWicImg"), true);
	smil2::test_attrs::set_current_system_component_value(AM_SYSTEM_COMPONENT("RendererImg"), true);
	return new common::single_playable_factory<
		gui::dx::dx_img_wic_renderer,
		dx_img_wic_playable_tag,
		dx_img_wic_playable_renderer_uri,
		dx_img_wic_playable_renderer_uri2,
		dx_img_wic_playable_renderer_uri3 >(factory, mdp);
}

gui::dx::dx_img_wic_renderer::dx_img_wic_renderer(
	common::playable_notification *context,
	common::playable_notification::cookie_type cookie,
	const lib::node *node,
	lib::event_processor* evp,
	common::factories *factory,
	common::playable_factory_machdep *dxplayer)
:	dx_renderer_playable(context, cookie, node, evp, factory, dynamic_cast<dx_playables_context*>(dxplayer)),
	m_original(0),
	m_ddsurf(0),
	m_databuf(NULL),
	m_factory(factory)
{
	if (s_wic_factory == NULL) {
		// init wic factory
		HRESULT hr = CoCreateInstance(
			CLSID_WICImagingFactory,
			NULL,
			CLSCTX_INPROC_SERVER,
			IID_PPV_ARGS(&s_wic_factory)
			);
		assert(SUCCEEDED(hr));
	}

	AM_DBG lib::logger::get_logger()->debug("dx_img_wic_renderer::ctr(0x%x)", this);
}

gui::dx::dx_img_wic_renderer::~dx_img_wic_renderer() {
	AM_DBG lib::logger::get_logger()->debug("dx_img_wic_renderer::dtr(0x%x)", this);
	// delete m_image;
	if (m_databuf) free(m_databuf);
}


void gui::dx::dx_img_wic_renderer::start(double t) {
	AM_DBG lib::logger::get_logger()->debug("dx_img_wic_renderer::start(0x%x)", this);
	if (s_wic_factory == NULL) {
		lib::logger::get_logger()->error("Windows Imaging Component not initialized");
		return;
	}
	HRESULT hr = S_OK;
	IWICBitmapDecoder *decoder = NULL;
	IWICFormatConverter *converter = NULL;
	IWICBitmapFrameDecode *frame = NULL;
	IWICBitmapSource *source = NULL;

	net::url url = m_node->get_url("src");
	if (url.is_local_file()) {
		// Local file, let WIC access it directly
		std::string filename_str(url.get_file());
		lib::textptr filename(filename_str.c_str());
		hr = s_wic_factory->CreateDecoderFromFilename(
			filename.wstr(),
			NULL,
			GENERIC_READ,
			WICDecodeMetadataCacheOnDemand,
			&decoder);
	} else {
		// Remote file, go through data buffer, etc.
#if 0
		lib::logger::get_logger()->error("WIC renderer not yet implemented for non-local files");
		goto fail;
#else
		size_t size;
		assert(m_databuf == NULL);
		if ( !net::read_data_from_url(url, m_factory->get_datasource_factory(), &m_databuf, &size)) {
			m_context->stopped(m_cookie);
			return;
		}
		IWICStream *stream = NULL;
		hr = s_wic_factory->CreateStream(&stream);
		if (!SUCCEEDED(hr)) {
			ambulant::lib::logger::get_logger()->error("Cannot create Windows Imaging Component stream");
			goto fail;
		}
		hr = stream->InitializeFromMemory((BYTE *)m_databuf, size);
		if (!SUCCEEDED(hr)) {
			ambulant::lib::logger::get_logger()->error("Cannot create Windows Imaging Component stream from buffer");
			stream->Release();
			goto fail;
		}
		hr = s_wic_factory->CreateDecoderFromStream(
			stream,
			NULL,
			WICDecodeMetadataCacheOnDemand,
			&decoder);
		// We release the stream now, before testing success. It will be increffed
		// in case things worked fine.
		stream->Release();
		if (!SUCCEEDED(hr)) {
			ambulant::lib::logger::get_logger()->error("Cannot create Windows Imaging Component decoder from stream");
			goto fail;
		}
#endif
	}
	// Get the first from the bitmap file
	hr = decoder->GetFrame(0, &frame);
	if (!SUCCEEDED(hr)) {
		lib::logger::get_logger()->trace("WIC image renderer: GetFrame() returned 0x%x", hr);
		lib::logger::get_logger()->error("%s: cannot render image", url.get_url().c_str());
		goto fail;
	}

	// Store this frame as a WIC bitmap source
	hr = frame->QueryInterface(IID_IWICBitmapSource, (void**)&source);
	if (!SUCCEEDED(hr)) {
		lib::logger::get_logger()->trace("WIC image renderer: QueryInterface() returned 0x%x", hr);
		lib::logger::get_logger()->error("%s: cannot render image", url.get_url().c_str());
		goto fail;
	}

	// Convert to required pixel format
	hr = s_wic_factory->CreateFormatConverter(&converter);
	if (!SUCCEEDED(hr)) {
		lib::logger::get_logger()->trace("WIC image renderer: CreateFormatConverter() returned 0x%x", hr);
		lib::logger::get_logger()->error("%s: cannot render image", url.get_url().c_str());
		goto fail;
	}
	hr = converter->Initialize(
		source,
		dxparams::I()->wic_format(),
		WICBitmapDitherTypeNone,
		NULL,
		0.0,
		WICBitmapPaletteTypeCustom);
	if (!SUCCEEDED(hr)) {
		lib::logger::get_logger()->trace("WIC image renderer: FormatConverter::Initialize() returned 0x%x", hr);
		lib::logger::get_logger()->error("%s: cannot render image", url.get_url().c_str());
		goto fail;
	}

	// Get access to the converter as a bitmap source, and keep this
	// for future reference.
	assert(m_original == NULL);
	hr = converter->QueryInterface(IID_PPV_ARGS(&m_original));
	if (!SUCCEEDED(hr)) {
		lib::logger::get_logger()->trace("WIC image renderer: FormatConverter::QueryInterface() returned 0x%x", hr);
		lib::logger::get_logger()->error("%s: cannot render image", url.get_url().c_str());
		goto fail;
	}
	
	// Make sure the DD surface is created, next redraw
	assert(m_ddsurf == NULL);

	// Inform the scheduler, ask for a redraw
	m_context->started(m_cookie);
	m_dest->show(this);
	m_dest->need_events(m_wantclicks);
	//m_dest->need_redraw(); // show already did this

fail:
	// Release things
	if (converter) converter->Release();
	if (source) source->Release();
	if (frame) frame->Release();
	if (decoder) decoder->Release();

	// Notify scheduler that we're done playing
	m_context->stopped(m_cookie);
}

void
gui::dx::dx_img_wic_renderer::_create_ddsurf(viewport *v)
{
	if (m_ddsurf) return;
	assert(m_original);
	assert(s_wic_factory);
	HRESULT hr = S_OK;

	// Check format (redundant, really, but still...)
    WICPixelFormatGUID pixelFormat;
    hr = m_original->GetPixelFormat(&pixelFormat);
	if (!SUCCEEDED(hr) || (pixelFormat != dxparams::I()->wic_format())) {
		lib::logger::get_logger()->debug("WIC renderer: Unexpected pixel format");
		return;
	}

	// Create DIB section
	UINT w = 0, h = 0;
	hr = m_original->GetSize(&w, &h);
	if(!SUCCEEDED(hr)) {
		lib::logger::get_logger()->debug("WIC renderer: GetSize() failed 0x%x", hr);
		return;
	}
	struct myBITMAPINFO {
		BITMAPINFOHEADER bmiHeader;
		DWORD masks[4];
	} bminfo;
	ZeroMemory(&bminfo, sizeof(bminfo));
    bminfo.bmiHeader.biSize         = sizeof(BITMAPINFOHEADER);
    bminfo.bmiHeader.biWidth        = w;
    bminfo.bmiHeader.biHeight       = -(LONG)h;
    bminfo.bmiHeader.biPlanes       = 1;
    bminfo.bmiHeader.biBitCount     = 32;
    bminfo.bmiHeader.biCompression  = dxparams::I()->bmi_compression();
	bminfo.masks[0] = 0x00ff0000;
	bminfo.masks[1] = 0x0000ff00;
	bminfo.masks[2] = 0x000000ff;
	bminfo.masks[3] = 0xff000000;
	void *buffer;
	HBITMAP dib_bitmap = CreateDIBSection(
		NULL, 
		(BITMAPINFO *)&bminfo,
		DIB_RGB_COLORS,
		&buffer,
		NULL,
		0);
	assert(dib_bitmap);
	assert(buffer);
	if (dib_bitmap == NULL || buffer == NULL) {
		lib::logger::get_logger()->debug("WIC renderer: Could not allocate DIBSection");
		return;
	}


	// Copy data into the DIB
	LONG stride = 4*w;
	LONG buffer_size = h*stride;
	ZeroMemory(buffer, buffer_size); // DBG
	hr = m_original->CopyPixels(NULL, stride, buffer_size, (BYTE *)buffer);
	if (!SUCCEEDED(hr)) {
		lib::logger::get_logger()->debug("WIC renderer: CopyPixels failed 0x%x", hr);
		return;
	}

	// Create DD surface
	assert(m_ddsurf == NULL);
	m_ddsurf = v->create_surface(w, h);

	// Copy the pixels
	HDC surface_hdc = NULL;
	hr = m_ddsurf->GetDC(&surface_hdc);
	assert(SUCCEEDED(hr));
	HDC bitmap_hdc = CreateCompatibleDC(surface_hdc);
	assert(bitmap_hdc);
	HBITMAP hbmp_old = (HBITMAP) SelectObject(bitmap_hdc, dib_bitmap);
	::BitBlt(surface_hdc, 0, 0, w, h, bitmap_hdc, 0, 0, SRCCOPY);
	SelectObject(bitmap_hdc, hbmp_old);
	DeleteDC(bitmap_hdc);
	m_ddsurf->ReleaseDC(surface_hdc);

	// Clean up
	if (dib_bitmap) DeleteObject(dib_bitmap);
}

bool gui::dx::dx_img_wic_renderer::stop() {
	AM_DBG lib::logger::get_logger()->debug("dx_img_wic_renderer::stop(0x%x)", this);
	if (m_dest) m_dest->renderer_done(this);
	m_dest = NULL;
	m_activated = false;
	m_dxplayer->stopped(this);
//	m_dest->need_redraw();
	return true;
}

bool gui::dx::dx_img_wic_renderer::user_event(const lib::point& pt, int what) {
	if (!user_event_sensitive(pt)) return false;
	if(what == common::user_event_click)
		m_context->clicked(m_cookie);
	else if(what == common::user_event_mouse_over) {
		m_context->pointed(m_cookie);
	}
	return true;
}

void gui::dx::dx_img_wic_renderer::redraw(const lib::rect& dirty, common::gui_window *window) {
	// Get the top-level surface
	dx_window *dxwindow = static_cast<dx_window*>(window);
	viewport *v = dxwindow->get_viewport();
	if(!v) {
		AM_DBG lib::logger::get_logger()->debug("dx_img_wic_renderer::redraw NOT: no viewport %0x %s ", m_dest, m_node->get_url("src").get_url().c_str());
		return;
	}

	_create_ddsurf(v);

	if(!m_ddsurf) {
		// No bits available
		AM_DBG lib::logger::get_logger()->debug("dx_img_wic_renderer::redraw NOT: no image or cannot play %0x %s ", m_dest, m_node->get_url("src").get_url().c_str());
		return;
	}

	lib::rect img_rect1;
	lib::rect img_reg_rc;
	UINT w, h;
	HRESULT hr = m_original->GetSize(&w, &h);
	assert(hr == 0);
	lib::size srcsize(w, h);

	// This code could be neater: it could share quite a bit with the
	// code below (for non-tiled images). Also, support for tiled images
	// is specifically geared toward background images: stuff like the
	// dirty region and transitions are ignored.
	// Also, it knows that the node and the region we're painting to are
	// really the same node.
	if (m_node->get_attribute("backgroundImage") &&	 m_dest->is_tiled()) {
		AM_DBG lib::logger::get_logger()->debug("dx_img_wic_renderer.redraw: drawing tiled image");
		img_reg_rc = m_dest->get_rect();
		img_reg_rc.translate(m_dest->get_global_topleft());
		common::tile_positions tiles = m_dest->get_tiles(srcsize, img_reg_rc);
		common::tile_positions::iterator it;
		for(it=tiles.begin(); it!=tiles.end(); it++) {
			img_rect1 = (*it).first;
			img_reg_rc = (*it).second;
			v->draw(m_ddsurf, img_rect1, img_reg_rc, 0 /*m_image->is_transparent()*/);
		}

		if (m_erase_never) m_dest->keep_as_background();
		return;
	}
#ifdef WITH_SMIL30
	lib::rect croprect = m_dest->get_crop_rect(srcsize);
	AM_DBG lib::logger::get_logger()->debug("get_crop_rect(%d,%d) -> (%d, %d, %d, %d)", srcsize.w, srcsize.h, croprect.left(), croprect.top(), croprect.width(), croprect.height());
	img_reg_rc = m_dest->get_fit_rect(croprect, srcsize, &img_rect1, m_alignment);
	double alpha_media = 1.0, alpha_media_bg = 1.0, alpha_chroma = 1.0;
	lib::color_t chroma_low = lib::color_t(0x000000), chroma_high = lib::color_t(0xFFFFFF);
	const common::region_info *ri = m_dest->get_info();
	if (ri) {
		alpha_media = ri->get_mediaopacity();
//???		alpha_media_bg = ri->get_mediabgopacity();
//???		m_bgopacity = ri->get_bgopacity();
		if (ri->is_chromakey_specified()) {
			alpha_chroma = ri->get_chromakeyopacity();
			lib::color_t chromakey = ri->get_chromakey();
			lib::color_t chromakeytolerance = ri->get_chromakeytolerance();
			lib::compute_chroma_range(chromakey, chromakeytolerance, &chroma_low, &chroma_high);
		} else alpha_chroma = alpha_media;
	}
#else
	// Get fit rectangles
	img_reg_rc = m_dest->get_fit_rect(srcsize, &img_rect1, m_alignment);
#endif
	// Use one type of rect to do op
	lib::rect img_rect(img_rect1);

	// A complete repaint would be:
	// {img, img_rect } -> img_reg_rc

	// We have to paint only the intersection.
	// Otherwise we will override upper layers
	lib::rect img_reg_rc_dirty = img_reg_rc & dirty;
	if(img_reg_rc_dirty.empty()) {
		// this renderer has no pixels for the dirty rect
		AM_DBG lib::logger::get_logger()->debug("dx_img_wic_renderer::redraw NOT: empty dirty region %0x %s ", m_dest, m_node->get_url("src").get_url().c_str());
		return;
	}

	// Find the part of the image that is mapped to img_reg_rc_dirty
	lib::rect img_rect_dirty = reverse_transform(&img_reg_rc_dirty,
		&img_rect, &img_reg_rc);

	// Translate img_reg_rc_dirty to viewport coordinates
	lib::point topleft = m_dest->get_global_topleft();
	img_reg_rc_dirty.translate(topleft);

	// keep rect for debug messages
	m_msg_rect |= img_reg_rc_dirty;

	// Finally blit img_rect_dirty to img_reg_rc_dirty
	AM_DBG lib::logger::get_logger()->debug("dx_img_wic_renderer::redraw %0x %s ", m_dest, m_node->get_url("src").get_url().c_str());

	dx_transition *tr = get_transition();
	if (tr && tr->is_fullscreen()) {
		v->set_fullscreen_transition(tr);
		tr = NULL;
	}

	if(tr && tr->is_outtrans() && m_original) {
		// First draw the background color, if applicable
		const common::region_info *ri = m_dest->get_info();
		if(ri)
			v->clear(img_reg_rc_dirty,ri->get_bgcolor(), ri->get_bgopacity());
		// Next, take a snapshot of the relevant pixels as they are now, before we draw the image
		UINT w, h;
		HRESULT hr = m_original->GetSize(&w, &h);
		assert(hr == 0);
		lib::size image_size(w, h);
		IDirectDrawSurface *bgimage = v->create_surface(image_size);
		lib::rect dirty_screen = img_rect_dirty;
		dirty_screen.translate(topleft);
		RECT bgrect_image, bgrect_screen;
		set_rect(img_rect_dirty, &bgrect_image);
		set_rect(dirty_screen, &bgrect_screen);
#ifdef DDBLT_WAIT
#define WAITFLAG DDBLT_WAIT
#else
#define WAITFLAG DDBLT_WAITNOTBUSY
#endif
		bgimage->Blt(&bgrect_image, m_ddsurf, &bgrect_screen, WAITFLAG, NULL);
		// Then draw the image
		v->draw(m_ddsurf, img_rect_dirty, img_reg_rc_dirty, 0 /*m_image->is_transparent()*/, (dx_transition*)0);
		// And finally transition in the background bits saved previously
		v->draw(bgimage, img_rect_dirty, img_reg_rc_dirty, false, tr);
		bgimage->Release();
	} else {
#ifdef	WITH_SMIL30
		if (alpha_chroma != 1.0) {
			IDirectDrawSurface* screen_ddsurf = v->get_surface();
			IDirectDrawSurface* image_ddsurf = m_ddsurf;
			lib::rect rct0 (lib::point(0, 0), img_reg_rc_dirty.size());
			v->blend_surface(
				img_reg_rc_dirty,
				image_ddsurf,
				rct0,
				0 /*m_image->is_transparent()*/,
				alpha_chroma,
				alpha_media,
				chroma_low,
				chroma_high);
		} else {
			v->draw(m_ddsurf, img_rect_dirty, img_reg_rc_dirty,0 /* m_image->is_transparent()*/, tr);
		}
#else //WITH_SMIL30
		v->draw(m_ddsurf, img_rect_dirty, img_reg_rc_dirty, m_image->is_transparent(), tr);
#endif//WITH_SMIL30
	}
	if (m_erase_never) m_dest->keep_as_background();
}



#endif // WITH_WIC
