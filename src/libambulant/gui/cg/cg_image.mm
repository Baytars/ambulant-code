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
#ifdef	WITH_IPHONE
//void* _CFXMLNodeGetInfoPtr = NULL;
#include <CoreGraphics/CoreGraphics.h>
#include <ImageIO/ImageIO.h>
#endif//WITH_IPHONE
#include "ambulant/gui/cg/cg_gui.h"
#include "ambulant/gui/cg/cg_image.h"
#include "ambulant/common/region_dim.h"
#include "ambulant/common/region_info.h"
#include "ambulant/smil2/test_attrs.h"

//#define AM_DBG
#ifndef AM_DBG
#define AM_DBG if(0)
#endif

namespace ambulant {

using namespace lib;

namespace gui {

namespace cg {

extern const char cg_image_playable_tag[] = "img";
extern const char cg_image_playable_renderer_uri[] = AM_SYSTEM_COMPONENT("RendererCoreGraphics");
extern const char cg_image_playable_renderer_uri2[] = AM_SYSTEM_COMPONENT("RendererImg");

common::playable_factory *
create_cg_image_playable_factory(common::factories *factory, common::playable_factory_machdep *mdp)
{
	smil2::test_attrs::set_current_system_component_value(AM_SYSTEM_COMPONENT("RendererCoreGraphics"), true);
	smil2::test_attrs::set_current_system_component_value(AM_SYSTEM_COMPONENT("RendererImg"), true);
	return new common::single_playable_factory<
		cg_image_renderer,
		cg_image_playable_tag,
		cg_image_playable_renderer_uri,
		cg_image_playable_renderer_uri2,
		cg_image_playable_renderer_uri2>(factory, mdp);
}

cg_image_renderer::~cg_image_renderer()
{
	m_lock.enter();
	AM_DBG logger::get_logger()->debug("~cg_image_renderer(0x%x)", (void *)this);
	if (m_image) {
		CGImageRelease(m_image);
		m_image = NULL;
		m_nsdata = NULL;
	}
	CGImageRelease(m_image_cropped);
	m_image_cropped = NULL;
	m_lock.leave();
}

CGImage *
cg_image_renderer::_cropped_image(const lib::rect& rect)
{
	if (!m_image) return NULL;
	if (rect == lib::rect(lib::point(0,0), m_size)) return m_image;
	if (m_image_cropped && rect == m_rect_cropped) return m_image_cropped;
	CGImageRelease(m_image_cropped);
	m_image_cropped = NULL;
	m_rect_cropped = rect;
	CGRect cg_rect = CGRectMake(rect.left(), rect.top(), rect.width(), rect.height());
    /*AM_DBG*/ lib::logger::get_logger()->debug("cg_image_renderer: crop to (%d, %d, %d, %d)", rect.left(), rect.top(), rect.width(), rect.height());
	m_image_cropped = CGImageCreateWithImageInRect(m_image, cg_rect);
	return m_image_cropped;
}

void
cg_image_renderer::redraw_body(const rect &dirty, gui_window *window)
{
	m_lock.enter();
#ifdef WITH_SMIL30
	const common::region_info *ri = m_dest->get_info();
#endif
	const rect &r = m_dest->get_rect();
	AM_DBG logger::get_logger()->debug("cg_image_renderer.redraw(0x%x, local_ltrb=(%d,%d,%d,%d)", (void *)this, r.left(), r.top(), r.right(), r.bottom());

	// First we load the image data
	if (m_data && !m_image) {
		AM_DBG logger::get_logger()->debug("cg_image_renderer.redraw: creating image");
//#ifdef WITH_UIKIT
		// JACK: Unsure whether this exists on the iPhone. Kees: it does from iPhoneOS4.0, in ImageIO.
		if (m_nsdata != NULL) {
			CFRelease(m_nsdata);
		}
		m_nsdata = (CFDataRef)[NSData dataWithBytesNoCopy: m_data length: (unsigned int)m_data_size freeWhenDone: NO];
		CGImageSourceRef rdr = CGImageSourceCreateWithData(m_nsdata, NULL);
		if (rdr == NULL) {
			logger::get_logger()->error("%s: could not create image reader", m_node->get_url("src").get_url().c_str());
			return;
		}
		if (m_image != NULL) {
			CGImageRelease(m_image);
		}
		m_image = CGImageSourceCreateImageAtIndex(rdr, 0, NULL);
		CFRelease(rdr);
//#endif //WITH_UIKIT
		if (!m_image)
			logger::get_logger()->error("%s: could not create CGImage", m_node->get_url("src").get_url().c_str());
		m_size = lib::size(CGImageGetWidth(m_image), CGImageGetHeight(m_image));

	}

	if (!m_image) {
		AM_DBG logger::get_logger()->debug("cg_image_renderer.redraw: nothing to draw");
		m_lock.leave();
		return;
	}
#ifdef WITH_SMIL30
	// Next we apply chroma keying.
	// XXXJACK: by doing this here we disregard animation on chromaKeying
	if (ri->is_chromakey_specified()) {
		double opacity = ri->get_chromakeyopacity();
		if (opacity != 0.0 && opacity != 1.0) {
			lib::logger::get_logger()->trace("%s: only chromaKeyOpacity values 0.0 and 1.0 supported on MacOS", m_node->get_sig().c_str());
		}
		if (opacity < 0.5) {
			lib::color_t chromakey = ri->get_chromakey();
			lib::color_t tolerance = ri->get_chromakeytolerance();
			CGFloat components[8] = {
				redf(chromakey)-redf(tolerance), redf(chromakey)+redf(tolerance),
				greenf(chromakey)-greenf(tolerance), greenf(chromakey)+greenf(tolerance),
				bluef(chromakey)-bluef(tolerance), bluef(chromakey)+bluef(tolerance),
				0.0, 0.0
			};
			CGImageRef new_image = CGImageCreateWithMaskingColors(m_image, components);
			CGImageRelease(m_image);
			m_image = new_image;
		}
	}
#endif
	cg_window *cwindow = (cg_window *)window;
	AmbulantView *view = (AmbulantView *)cwindow->view();
	CGContextRef myContext = [view getCGContext];

	// Now find both source and destination area for the bitblit.
	rect srcrect;
	rect dstrect;
	CGRect cg_dstrect;
	CGImage *cropped_image;
	// While rendering background images only, check for tiling. This code is
	// convoluted, it knows that the node and the region we're painting to are
	// really the same node.
	if (m_node->get_attribute("backgroundImage") && m_dest->is_tiled()) {
		AM_DBG lib::logger::get_logger()->debug("cg_image_renderer.redraw: drawing tiled image");
		dstrect = m_dest->get_rect();
		dstrect.translate(m_dest->get_global_topleft());
		common::tile_positions tiles = m_dest->get_tiles(m_size, dstrect);
		common::tile_positions::iterator it;
		for(it=tiles.begin(); it!=tiles.end(); it++) {
			srcrect = (*it).first;
			dstrect = (*it).second;
			cropped_image = _cropped_image(srcrect);
			cg_dstrect = [view CGRectForAmbulantRect: &dstrect];
			// XXXX Need to do transform!
			CGContextDrawImage (myContext, cg_dstrect, cropped_image);
		}
		m_lock.leave();
		return;
	}
#ifdef WITH_SMIL30
	// Unfortunately (well, for us, in this case) Cocoa does some magic scaling on the image.
	// I.e. [m_image size] can lie about the size. We have to adjust our coordinates too.
	lib::rect croprect = m_dest->get_crop_rect(m_size);
	AM_DBG logger::get_logger()->debug("cg_image::redraw, clip 0x%x (%d %d) -> (%d, %d, %d, %d)", m_dest, m_size.w, m_size.h, croprect.x, croprect.y, croprect.w, croprect.h);

	dstrect = m_dest->get_fit_rect(croprect, m_size, &srcrect, m_alignment);
#else
	dstrect = m_dest->get_fit_rect(m_size, &srcrect, m_alignment);
#endif
	dstrect.translate(m_dest->get_global_topleft());
	cg_dstrect = [view CGRectForAmbulantRect: &dstrect];
	AM_DBG logger::get_logger()->debug("cg_image_renderer.redraw: draw image (ltrb) (%d, %d, %d, %d) -> (%f, %f, %f, %f)",
		srcrect.left(), srcrect.top(), srcrect.right(), srcrect.bottom(),
		CGRectGetMinX(cg_dstrect), CGRectGetMinY(cg_dstrect), CGRectGetMaxX(cg_dstrect), CGRectGetMaxY(cg_dstrect));
	double alfa = 1.0;
#ifdef WITH_SMIL30
	if (ri) alfa = ri->get_mediaopacity();
	// XXX Need to set alpha
#endif
#if 0
    // Attempt by Jack to draw without cropping image
    CGContextSaveGState(myContext);
    float x_scale = (float)dstrect.width() / (float)srcrect.width();
    float y_scale = (float)dstrect.height() / (float)srcrect.height();
    float x_orgin = (float)srcrect.left() / x_scale;
    float y_origin = (float)srcrect.top() / y_scale;
    CGAffineTransform matrix = CGAffineTransformMake(x_scale, 0, 0, y_scale, 0, 0);
    CGContextConcatCTM(myContext, matrix);
    CGContextClipToRect(myContext, cg_dstrect);
    CGContextDrawImage(myContext, xxxxx, m_image);
    CGContextRestoreGState(myContext);
#else
	cropped_image = _cropped_image(srcrect);
#ifndef WITH_UIKIT
	bool flipped = [view isFlipped];
	flipped = false; // XXXJACK
	if (flipped) {
		CGContextSaveGState(myContext);
		CGAffineTransform matrix = CGAffineTransformMake(1, 0, 0, -1, 0, cg_dstrect.origin.y);
		cg_dstrect.origin.y = 0;
		CGContextConcatCTM(myContext, matrix);
	}
#endif // WITH_UIKIT
	CGContextDrawImage(myContext, cg_dstrect, cropped_image);
#endif // Attempt by Jack

#ifndef WITH_UIKIT
	if (flipped) {
		CGContextRestoreGState(myContext);
	}
#endif // WITH_UIKIT
	m_lock.leave();
}

} // namespace cg

} // namespace gui

} //namespace ambulant

