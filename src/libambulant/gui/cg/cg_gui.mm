// This file is part of Ambulant Player, www.ambulantplayer.org.
//
// Copyright (C) 2003-2007 Stichting CWI, 
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

#include "ambulant/gui/cg/cg_gui.h"
#include "ambulant/gui/cg/cg_text.h"
#include "ambulant/gui/cg/atsui_text.h"
//#include "ambulant/gui/cg/cg_html.h"
#include "ambulant/gui/cg/cg_image.h"
//#include "ambulant/gui/cg/cg_ink.h"
#include "ambulant/gui/cg/cg_fill.h"
//#include "ambulant/gui/cg/cg_video.h"
#include "ambulant/gui/cg/cg_dsvideo.h"
#ifdef WITH_SMIL30
//#include "ambulant/gui/cg/cg_smiltext.h"
#endif
#include "ambulant/lib/mtsync.h"
#include "ambulant/common/preferences.h"

//#include <CoreGraphics/CoreGraphics.h>

//#define AM_DBG
#ifndef AM_DBG
#define AM_DBG if(0)
#endif

#define USE_COCOA_BOTLEFT

namespace ambulant {

using namespace lib;

namespace gui {

namespace cg {

common::window_factory *
create_cg_window_factory(void *view)
{
    return new cg_window_factory(view);
}

common::playable_factory *
create_cg_renderer_factory(common::factories *factory)
{
    return new cg_renderer_factory(factory);
}

cg_window::~cg_window()
{
	if (m_view) {
		AmbulantView *my_view = (AmbulantView *)m_view;
		[my_view ambulantWindowClosed];
	}
	m_view = NULL;
}
	
void
cg_window::need_redraw(const rect &r)
{
	AM_DBG logger::get_logger()->debug("cg_window::need_redraw(0x%x, ltrb=(%d,%d,%d,%d))", (void *)this, r.left(), r.top(), r.right(), r.bottom());
	if (!m_view) {
		logger::get_logger()->fatal("cg_window::need_redraw: no m_view");
		return;
	}
	AmbulantView *my_view = (AmbulantView *)m_view;
	CGRect my_rect = [my_view CGRectForAmbulantRect: &r];
	NSRectHolder *arect = [[NSRectHolder alloc] initWithRect: my_rect];
	// XXX Is it safe to cast C++ objects to ObjC id's?
	[my_view performSelectorOnMainThread: @selector(asyncRedrawForAmbulantRect:) 
		withObject: arect waitUntilDone: NO];
}

void
cg_window::redraw_now()
{
	AmbulantView *my_view = (AmbulantView *)m_view;
	[my_view performSelectorOnMainThread: @selector(syncDisplayIfNeeded:) 
		withObject: nil waitUntilDone: YES];
}

void
cg_window::redraw(const rect &r)
{
	AM_DBG logger::get_logger()->debug("cg_window::redraw(0x%x, ltrb=(%d,%d,%d,%d))", (void *)this, r.left(), r.top(), r.right(), r.bottom());
	m_handler->redraw(r, this);
}

void
cg_window::user_event(const point &where, int what)
{
	AM_DBG logger::get_logger()->debug("cg_window::user_event(0x%x, (%d, %d), %d)", (void *)this, where.x, where.y, what);
	m_handler->user_event(where, what);
}

void
cg_window::need_events(bool want)
{
	// This code needs to be run in the main thread.
	NSAutoreleasePool *pool = [[NSAutoreleasePool alloc] init];

	AM_DBG logger::get_logger()->debug("cg_window::need_events(0x%x, %d)", (void *)this, want);
		
	AmbulantView *my_view = (AmbulantView *)m_view;
	[my_view ambulantNeedEvents: want];
	[pool release];

}

void
cg_window::set_size(lib::size bounds)
{

	AmbulantView *view = (AmbulantView *)m_view;
	[view ambulantSetSize: bounds];
}

common::playable *
cg_renderer_factory::new_playable(
	common::playable_notification *context,
	common::playable_notification::cookie_type cookie,
	const lib::node *node,
	event_processor *evp)
{
	common::playable *rv = NULL;
	
	xml_string tag = node->get_local_name();
	if (tag == "img") {
#if NOT_YET_UIKIT
		net::url url = net::url(node->get_url("src"));
		if (url.guesstype() == "image/vnd.ambulant-ink") {
			rv = new cg_ink_renderer(context, cookie, node, evp, m_factory);
			AM_DBG logger::get_logger()->debug("cg_renderer_factory: node 0x%x: returning cg_ink_renderer 0x%x", (void *)node, (void *)rv);
		} else
#endif
		{
			rv = new cg_image_renderer(context, cookie, node, evp, m_factory);
			AM_DBG logger::get_logger()->debug("cg_renderer_factory: node 0x%x: returning cg_image_renderer 0x%x", (void *)node, (void *)rv);
		}
	} else if ( tag == "text") {
#if NOT_YET_UIKIT
		net::url url = net::url(node->get_url("src"));
		if (url.guesstype() == "text/html") {
			rv = new cg_html_renderer(context, cookie, node, evp);
			AM_DBG logger::get_logger()->debug("cg_renderer_factory: node 0x%x: returning cg_html_renderer 0x%x", (void *)node, (void *)rv);
		} else
#endif
		{
#if 1 // def WITH_UIKIT
			rv = new cg_text_renderer(context, cookie, node, evp, m_factory);
			AM_DBG logger::get_logger()->debug("cg_renderer_factory: node 0x%x: returning cg_text_renderer 0x%x", (void *)node, (void *)rv);
#else
			rv = new atsui_text_renderer(context, cookie, node, evp, m_factory);
			AM_DBG logger::get_logger()->debug("cg_renderer_factory: node 0x%x: returning atsui_text_renderer 0x%x", (void *)node, (void *)rv);
#endif

		}
	} else if ( tag == "brush") {
		rv = new cg_fill_renderer(context, cookie, node, evp);
		AM_DBG logger::get_logger()->debug("cg_renderer_factory: node 0x%x: returning cg_fill_renderer 0x%x", (void *)node, (void *)rv);
	} else if ( tag == "video") {
		if (1 /*common::preferences::get_preferences()->m_prefer_ffmpeg */) {
			rv = new cg_dsvideo_renderer(context, cookie, node, evp, m_factory);
			if (rv) {
				logger::get_logger()->trace("video: using native Ambulant renderer");
				AM_DBG logger::get_logger()->debug("cg_renderer_factory: node 0x%x: returning cg_dsvideo_renderer 0x%x", (void *)node, (void *)rv);
			} else {
#if NOT_YET_UIKIT
				rv = new cg_video_renderer(context, cookie, node, evp);
				if (rv) logger::get_logger()->trace("video: using QuickTime renderer");
				AM_DBG logger::get_logger()->debug("cg_renderer_factory: node 0x%x: returning cg_video_renderer 0x%x", (void *)node, (void *)rv);
#endif
			}
		} else {
#if NOT_YET_UIKIT
			rv = new cg_video_renderer(context, cookie, node, evp);
			if (rv) {
				logger::get_logger()->trace("video: using QuickTime renderer");
				AM_DBG logger::get_logger()->debug("cg_renderer_factory: node 0x%x: returning cg_video_renderer 0x%x", (void *)node, (void *)rv);
			} else {
				rv = new cg_dsvideo_renderer(context, cookie, node, evp, m_factory);
				if (rv) logger::get_logger()->trace("video: using ffmpeg renderer");
				AM_DBG logger::get_logger()->debug("cg_renderer_factory: node 0x%x: returning cg_dsvideo_renderer 0x%x", (void *)node, (void *)rv);
			}
#endif
		}
	}
#ifdef WITH_SMIL30
	else if ( tag == "smilText") {
#if NOT_YET_UIKIT
		rv = new cg_smiltext_renderer(context, cookie, node, evp);
		AM_DBG logger::get_logger()->debug("cg_renderer_factory: node 0x%x: returning cg_smiltext_renderer 0x%x", (void *)node, (void *)rv);
#endif
#endif // WITH_SMIL30
	} else {
		// logger::get_logger()->error(gettext("cg_renderer_factory: no CoreGraphics renderer for tag \"%s\""), tag.c_str());
		return NULL;
	}
	return rv;
}

common::playable *
cg_renderer_factory::new_aux_audio_playable(
		common::playable_notification *context,
		common::playable_notification::cookie_type cookie,
		const lib::node *node,
		lib::event_processor *evp,
		net::audio_datasource *src)
{
	return NULL;
}

lib::size
cg_window_factory::get_default_size()
{
	if (m_defaultwindow_view == NULL)
		return lib::size(common::default_layout_width, common::default_layout_height);
	CGSize size = CGSizeFromViewSize([(AmbulantView *)m_defaultwindow_view bounds].size);
	return lib::size((int)size.width, (int)size.height);
}

common::gui_window *
cg_window_factory::new_window(const std::string &name, size bounds, common::gui_events *handler)
{
	if ([(AmbulantView *)m_defaultwindow_view isAmbulantWindowInUse]) {
		// XXXX Should create new toplevel window and put an ambulantview in it
		logger::get_logger()->error(gettext("Unsupported: AmbulantPlayer cannot open second toplevel window yet"));
		return NULL;
	}
	cg_window *window = new cg_window(name, bounds, m_defaultwindow_view, handler);
	// And we need to inform the object about it
	AmbulantView *view = (AmbulantView *)window->view();
	[view setAmbulantWindow: window];
	// And set the window size
	window->set_size(bounds);
	
	return (common::gui_window *)window;
}

common::bgrenderer *
cg_window_factory::new_background_renderer(const common::region_info *src)
{
	return new cg_background_renderer(src);
}

void
cg_gui_screen::get_size(int *width, int *height)
{
	AmbulantView *view = (AmbulantView *)m_view;
	CGRect bounds = CGRectFromViewRect([view bounds]);
	*width = int(bounds.size.width);
	*height = int(bounds.size.height);
}

bool
cg_gui_screen::get_screenshot(const char *type, char **out_data, size_t *out_size)
{
#if NOT_YET_UIKIT
	NSAutoreleasePool *pool = [[NSAutoreleasePool alloc] init];
	
	*out_data = NULL;
	*out_size = 0;
	NSBitmapImageFileType filetype;
	if ( strcmp(type, "tiff") == 0) filetype = NSTIFFFileType;
	else if (strcmp(type, "bmp") == 0) filetype = NSBMPFileType;
	else if (strcmp(type, "gif") == 0) filetype = NSGIFFileType;
	else if (strcmp(type, "jpeg") == 0) filetype = NSJPEGFileType;
	else if (strcmp(type, "png") == 0) filetype = NSPNGFileType;
	else {
		lib::logger::get_logger()->trace("get_screenshot: unknown filetype \"%s\"", type);
		goto bad;
	}
	NSData *data;
	AmbulantView *view = (AmbulantView *)m_view;
	NSImage *image = [view _getOnScreenImage];
	if (image == NULL) {
		lib::logger::get_logger()->trace("get_screenshot: cannot get screen shot");
		goto bad;
	}
	NSImageRep *rep = [image bestRepresentationForDevice: NULL];
	if (rep == NULL) {
		lib::logger::get_logger()->trace("get_screenshot: cannot get representation for screen shot");
//		[image release];
		goto bad;
	}
	data = [rep representationUsingType: filetype properties: NULL];
//	[image release];
	if (data == NULL) {
		lib::logger::get_logger()->trace("get_screenshot: cannot convert screenshot to %s format", type);
		goto bad;
	}
	*out_data = (char *)malloc([data length]);
	if (*out_data == NULL) {
		lib::logger::get_logger()->trace("get_screenshot: out of memory");
//		[data release];
		goto bad;
	}
	*out_size = [data length];
	[data getBytes: *out_data];
//	[data release];
	[pool release];
	return true;
bad:
	[pool release];
#endif
	return false;
}

} // namespace cg

} // namespace gui

} //namespace ambulant

#ifdef __OBJC__

// Helper class: flipped view
@interface MyFlippedView : VIEW_SUPERCLASS
- (BOOL) isFlipped;
@end
@implementation MyFlippedView
- (BOOL) isFlipped
{
#ifdef USE_COCOA_BOTLEFT
	return false;
#else
	return true;
#endif
}
@end

// Helper class: NSRect as an object
@implementation NSRectHolder

- (id) initWithRect: (CGRect)r
{
	rect = r;
	return self;
}

- (CGRect)rect
{
	return rect;
}
@end

@implementation AmbulantView

- (id)initWithFrame:(CGRect)frameRect
{
	[super initWithFrame: ViewRectFromCGRect(frameRect)];
	ambulant_window = NULL;
//	transition_surface = NULL;
//	transition_tmpsurface = NULL;
	transition_count = 0;
	fullscreen_count = 0;
//	fullscreen_previmage = NULL;
//	fullscreen_oldimage = NULL;
//	fullscreen_engine = NULL;
//	overlay_window = NULL;
//	overlay_window_needs_unlock = NO;
//	overlay_window_needs_reparent = NO;
//	overlay_window_needs_flush = NO;
//	overlay_window_needs_clear = NO;
	return self;
}

- (void)dealloc {
//	if (transition_surface) [transition_surface release];
//	transition_surface = NULL;
//	if (transition_tmpsurface) [transition_tmpsurface release];
//	transition_tmpsurface = NULL;
//	if (overlay_window) [overlay_window release];
//	overlay_window = NULL;
    [super dealloc];

}

- (CGContextRef) getCGContext
{
#ifdef WITH_UIKIT
	return UICurrentContext();
#else
	return (CGContextRef)[[NSGraphicsContext currentContext] graphicsPort];
#endif
}

- (CGRect) CGRectForAmbulantRect: (const ambulant::lib::rect *)arect
{
#ifdef USE_COCOA_BOTLEFT
	float bot_delta = CGRectGetMaxY(CGRectFromViewRect([self bounds])) - arect->bottom();
	return CGRectMake(arect->left(), bot_delta, arect->width(), arect->height());
#else
	return CGRectMake(arect->left(), arect->top(), arect->width(), arect->height());
#endif
}

- (ambulant::lib::rect) ambulantRectForCGRect: (const CGRect *)nsrect
{
#ifdef USE_COCOA_BOTLEFT
	float top_delta = CGRectGetMaxY(CGRectFromViewRect([self bounds])) - CGRectGetMaxY(*nsrect);
	ambulant::lib::rect arect = ambulant::lib::rect(
                ambulant::lib::point(int(CGRectGetMinX(*nsrect)), int(top_delta)),
				ambulant::lib::size(int(CGRectGetWidth(*nsrect)), int(CGRectGetHeight(*nsrect))));
#else
	ambulant::lib::rect arect = ambulant::lib::rect(
                ambulant::lib::point(int(CGRectGetMinX(*nsrect)), int(CGRectGetMinY(*nsrect))),
				ambulant::lib::size(int(CGRectGetWidth(*nsrect)), int(CGRectGetHeight(*nsrect))));
	 
#endif
	return arect;
}

- (void) asyncRedrawForAmbulantRect: (NSRectHolder *)arect
{
#if 1
	// Something goes wrong with flipping coordinate systems. For now, redraw everything always.
	CGRect my_rect = [self bounds];
#else
	CGRect my_rect = [arect rect];
#endif
	[arect release];
    AM_DBG NSLog(@"AmbulantView.asyncRedrawForAmbulantRect: self=0x%x rect=(%f,%f,%f,%f)", self, CGRectGetMinX(my_rect), CGRectGetMinY(my_rect), CGRectGetMaxX(my_rect), CGRectGetMaxY(my_rect));
	[self setNeedsDisplayInRect: ViewRectFromCGRect(my_rect)];
}

- (void) syncDisplayIfNeeded: (id) dummy
{
//	[self displayIfNeeded];
#if UIKIT_NOT_YET
	[self display];
#endif
}

- (void)drawRect:(CGRect)rect
{
    AM_DBG NSLog(@"AmbulantView.drawRect: self=0x%x rect=(%f,%f,%f,%f)", self, CGRectGetMinX(rect), CGRectGetMinY(rect), CGRectGetMaxX(rect), CGRectGetMaxY(rect));
//    redraw_lock.enter();
#ifdef WITH_QUICKTIME_OVERLAY
	// If our main view has been reparented since the last redraw we need
	// to move the overlay window.
	if (overlay_window_needs_reparent) {
		assert(overlay_window);
		NSWindow *window = [self window];
		overlay_window_needs_reparent = NO;
		[[overlay_window parentWindow] removeChildWindow: overlay_window];
		[window addChildWindow: overlay_window ordered: NSWindowAbove];

		// XXXJACK This goes wrong when going back to windowed mode: for some reason
		// [self frame].origin still has the position as it was during fullscreen mode...
		NSPoint baseOrigin = NSMakePoint([self frame].origin.x, [self frame].origin.y);
		NSPoint screenOrigin = [window convertBaseToScreen: baseOrigin];
		AM_DBG NSLog(@"viewDidMoveToWindow: new origin (%f, %f)", screenOrigin.x, screenOrigin.y);
		[overlay_window setFrameOrigin: screenOrigin];
	}
	
	// If something was drawn into the overlay window during the last redraw
	// we need to clear the overlay window.
	if (overlay_window_needs_clear) {
		assert(overlay_window);
		NSView *oview = [overlay_window contentView];
		[oview lockFocus];
		[[NSColor clearColor] set];
		CGRect area = [oview bounds];
		AM_DBG NSLog(@"clear %f %f %f %f in %@", area.origin.x, area.origin.y, area.size.width, area.size.height, oview);
		NSRectFill(area);
		overlay_window_needs_clear = NO;
		overlay_window_needs_flush = YES;
		[oview unlockFocus];
	}
#endif // WITH_QUICKTIME_OVERLAY

	if (!ambulant_window) {
        AM_DBG NSLog(@"Redraw AmbulantView: NULL ambulant_window");
    } else {
#ifdef WITH_UIKIT
		CGRect bounds = [self bounds];
		AM_DBG NSLog(@"ambulantview: bounds (%f, %f, %f, %f)", bounds.origin.x, bounds.origin.y, bounds.size.width, bounds.size.height);
		CGContextRef myContext = [self getCGContext];
		CGContextSaveGState(myContext);
		float view_height = CGRectGetHeight(CGRectFromViewRect([self bounds]));
		CGAffineTransform matrix = CGAffineTransformMake(1, 0, 0, -1, 0, view_height);
		CGContextConcatCTM(myContext, matrix);
#endif
		// If we have seen transitions we always redraw the whole view
		// XXXJACK interaction of fullscreen transitions and overlay windows
		// is completely untested, and probably broken.
		if (transition_count) rect = CGRectFromViewRect([self bounds]);
        ambulant::lib::rect arect = [self ambulantRectForCGRect: &rect];
//		[self _screenTransitionPreRedraw];
        ambulant_window->redraw(arect);
//		[self _screenTransitionPostRedraw];
#ifdef WITH_UIKIT
		CGContextRestoreGState(myContext);
#endif
#ifdef DUMP_REDRAW
		// Debug code: dump the contents of the view into an image
		[self dumpToImageID: "redraw"];
#endif
//		[self _releaseTransitionSurface];
    }
#ifdef WITH_QUICKTIME_OVERLAY
	// If the overlay window was actually used (and possibly drawn into)
	// we need to unlock it. We also prepare for flushing it, and clearing
	// it the next redraw cycle.
	if (overlay_window_needs_unlock) {
		assert(overlay_window);
		[[overlay_window contentView] unlockFocus];
		overlay_window_needs_unlock = NO;
		overlay_window_needs_flush = YES;
		overlay_window_needs_clear = YES;
	}
	// Finally we flush the window, if required.
	if (overlay_window_needs_flush) {
		assert(overlay_window);
		[overlay_window flushWindow];
		overlay_window_needs_flush = NO;
	}
#endif // WITH_QUICKTIME_OVERLAY

//	redraw_lock.leave();
}

- (void)setAmbulantWindow: (ambulant::gui::cg::cg_window *)window
{
//	[[self window] setAcceptsMouseMovedEvents: true];
    ambulant_window = window;
}

- (void)ambulantWindowClosed
{
    AM_DBG NSLog(@"ambulantWindowClosed called");
    ambulant_window = NULL;
	// XXXX Should we close the window too? Based on preference?
}

- (bool)isAmbulantWindowInUse
{
    return (ambulant_window != NULL);
}

- (bool)ignoreResize
{
	return false;
}

- (void)ambulantSetSize: (ambulant::lib::size) bounds
{
#if WITH_UIKIT
	NSLog(@"ambulantSetSize: not yet implemented for UIKit");
#else
	// Get the position of our view in window coordinates
	NSPoint origin = NSMakePoint(0,0);
	NSView *superview = [self superview];
	NSWindow *window = [self window];
	int32_t     shieldLevel = CGShieldingWindowLevel();
	if ([self ignoreResize] || [window level] >= shieldLevel) {
		// We don't muck around with fullscreen windows or windows in other apps (browsers, etc). 
		// What we should actually do is recenter the content, but that is for later.
	} else {
		if (superview) {
			NSRect rect = [superview convertRect: [self frame] toView: nil];
			origin = rect.origin;
		}
		// And set the window size
		AM_DBG NSLog(@"Size changed request: (%d, %d)", bounds.w, bounds.h);
		NSSize ns_size = NSMakeSize(bounds.w + origin.x, bounds.h + origin.y);
		[window setContentSize: ns_size];
		AM_DBG NSLog(@"Size changed on %@ to (%f, %f)", window, ns_size.width, ns_size.height);
	}
	[window makeKeyAndOrderFront: self];
#endif
}

- (void)ambulantNeedEvents: (bool)want
{
#if WITH_UIKIT
	NSLog(@"ambulantNeedEvents: not implemented yet for UIKit");
#else
	NSWindow *my_window = [self window];
	AM_DBG NSLog(@"my_window acceptsMouseMovedEvents = %d", [my_window acceptsMouseMovedEvents]);
	// See whether the mouse is actually in our area
	NSPoint where = [my_window mouseLocationOutsideOfEventStream];
	if (!NSPointInRect(where, [self frame])) {
		AM_DBG NSLog(@"mouse outside our frame");
		return;
	}
	// Get the main thread to do the real work
	[self performSelectorOnMainThread: @selector(pseudoMouseMove:) 
		withObject: nil waitUntilDone: NO];
#endif	
}

- (BOOL)isFlipped
{
#ifdef USE_COCOA_BOTLEFT
	return false;
#else
	return true;
#endif
}

- (void)tappedWithPoint: (CGPoint) where
{
	ambulant::lib::point amwhere = ambulant::lib::point((int)where.x, (int)where.y);
	if (ambulant_window) ambulant_window->user_event(amwhere);
}

#if NOT_YET_UIKIT
- (void)mouseDown: (NSEvent *)theEvent
{
	NSPoint where = [theEvent locationInWindow];
	where = [self convertPoint: where fromView: nil];
	if (!NSPointInRect(where, [self bounds])) {
		AM_DBG NSLog(@"0x%x: mouseDown outside our frame", (void*)self);
		return;
	}
	AM_DBG NSLog(@"0x%x: mouseDown at ambulant-point(%f, %f)", (void*)self, where.x, where.y);
	ambulant::lib::point amwhere = ambulant::lib::point((int)where.x, (int)where.y);
	if (ambulant_window) ambulant_window->user_event(amwhere);
}

- (void)mouseMoved: (NSEvent *)theEvent
{
	NSPoint where = [theEvent locationInWindow];
	where = [self convertPoint: where fromView: nil];
	if (!NSPointInRect(where, [self bounds])) {
		AM_DBG NSLog(@"mouseMoved outside our frame");
		return;
	}
	AM_DBG NSLog(@"0x%x: mouseMoved at ambulant-point(%f, %f)", (void*)self, where.x, where.y);
	ambulant::lib::point amwhere = ambulant::lib::point((int)where.x, (int)where.y);
	[[NSApplication sharedApplication] sendAction: SEL("resetMouse:") to: nil from: self];
	if (ambulant_window) ambulant_window->user_event(amwhere, 1);
	// XXX Set correct cursor
	[[NSApplication sharedApplication] sendAction: SEL("fixMouse:") to: nil from: self];
}

- (void)pseudoMouseMove: (id)dummy
{
	NSPoint where = [[self window] mouseLocationOutsideOfEventStream];
	where = [self convertPoint: where fromView: nil];
	AM_DBG NSLog(@"pseudoMouseMoved at (%f, %f)", where.x, where.y);
	if (!NSPointInRect(where, [self bounds])) {
		AM_DBG NSLog(@"mouseMoved outside our frame");
		return;
	}
	AM_DBG NSLog(@"0x%x: pseudoMouseMove at ambulant-point(%f, %f)", (void*)self, where.x, where.y);
	ambulant::lib::point amwhere = ambulant::lib::point((int)where.x, (int)where.y);
	[[NSApplication sharedApplication] sendAction: SEL("resetMouse:") to: nil from: self];
	if (ambulant_window) ambulant_window->user_event(amwhere, 1);
	// XXX Set correct cursor
	[[NSApplication sharedApplication] sendAction: SEL("fixMouse:") to: nil from: self];
}
#endif

- (BOOL)wantsDefaultClipping
{
#ifdef DUMP_REDRAW
	return NO;
#else
	return (transition_count == 0);
#endif
}

- (void) incrementTransitionCount
{
	transition_count++;
	AM_DBG NSLog(@"incrementTransitionCount: count=%d", transition_count);
}

- (void) decrementTransitionCount
{
	assert(transition_count > 0);
	transition_count--;
	AM_DBG NSLog(@"decrementTransitionCount: count=%d", transition_count);
	// XXXX Should we delete transition_surface?
	// XXXX Should we delete transition_tmpsurface?
}

#if NOT_YET_UIKIT
- (NSImage *)getTransitionSurface
{
	if (!transition_surface) {
		// It does not exist yet. Create it.
		transition_surface = [self getOnScreenImageForRect: [self bounds]];
		[transition_surface retain];
	}
	return transition_surface;
}

- (void)_releaseTransitionSurface
{
	if (transition_surface) {
		[transition_surface release];
		transition_surface = NULL;
	}
}

- (NSImage *)getTransitionTmpSurface
{
	if (!transition_tmpsurface) {
		// It does not exist yet. Create it.
		transition_tmpsurface = [self getOnScreenImageForRect: [self bounds]];
		[transition_tmpsurface retain];
		[transition_tmpsurface setFlipped: NO];
	}
	return transition_tmpsurface;
}

- (NSImage *)_getOnScreenImage
{
	NSView *src_view = self;
	NSWindow *tmp_window = NULL;
#ifdef WITH_QUICKTIME_OVERLAY
	if (overlay_window) {
		NSLog(@"Doing screenshot with overlay window");
		[[self window] makeKeyAndOrderFront: self];
		tmp_window = [[NSWindow alloc] initWithContentRect:[overlay_window frame] styleMask:NSBorderlessWindowMask
					backing:NSBackingStoreNonretained defer:NO];
		[tmp_window setBackgroundColor:[NSColor clearColor]];
		[tmp_window setLevel:[overlay_window level]];
		[tmp_window setHasShadow:NO];
		[tmp_window setAlphaValue:0.0];
		src_view = [[NSView alloc] initWithFrame:[self bounds]];
		[tmp_window setContentView:src_view];
		[tmp_window orderFront:self];
	}
#endif /* WITH_QUICKTIME_OVERLAY */
	CGRect bounds = [self bounds];
	CGSize size = NSMakeSize(NSWidth(bounds), NSHeight(bounds));
	NSImage *rv = [[NSImage alloc] initWithSize: size];
	[src_view lockFocus];
	NSBitmapImageRep *bits = [[NSBitmapImageRep alloc] initWithFocusedViewRect: [self bounds]];
	[src_view unlockFocus];
	if (tmp_window) {
		[tmp_window orderOut: self];
		[tmp_window close];
	}
	[rv addRepresentation: [bits autorelease]];
	[rv setFlipped: YES];
#ifdef DUMP_TRANSITION
	[self dump: rv toImageID: "oldsrc"];
#endif
	rv = [rv autorelease];
	return rv;
}

- (NSImage *)getOnScreenImageForRect: (CGRect)bounds
{
	// Note: this method does not take overlaying things such as Quicktime
	// movies into account.
	CGSize size = NSMakeSize(NSWidth(bounds), NSHeight(bounds));
	NSImage *rv = [[NSImage alloc] initWithSize: size];
	[self lockFocus];
	NSBitmapImageRep *bits = [[NSBitmapImageRep alloc] initWithFocusedViewRect: bounds];
	[self unlockFocus];
	[rv addRepresentation: [bits autorelease]];
	[rv setFlipped: YES];
#ifdef DUMP_TRANSITION
	[self dump: rv toImageID: "oldsrc"];
#endif
	rv = [rv autorelease];
	return rv;
}

- (NSImage *)getTransitionOldSource
{
	if (fullscreen_count && fullscreen_oldimage)
		return fullscreen_oldimage;
	return [self getOnScreenImageForRect: [self bounds]];
}

- (NSImage *)getTransitionNewSource
{
	CGRect bounds = [self bounds];
	CGSize size = NSMakeSize(NSWidth(bounds), NSHeight(bounds));
	NSImage *rv = [[NSImage alloc] initWithSize: size];
	[rv setFlipped: YES];
	[transition_surface lockFocus];
	NSBitmapImageRep *bits = [[NSBitmapImageRep alloc] initWithFocusedViewRect: [self bounds]];
	[transition_surface unlockFocus];
	[rv addRepresentation: [bits autorelease]];
#ifdef DUMP_TRANSITION
	[self dump: rv toImageID: "newsrc"];
#endif
	rv = [rv autorelease];
	return rv;
}

- (void) startScreenTransition
{
	AM_DBG NSLog(@"startScreenTransition");
	if (fullscreen_count)
		NSLog(@"Warning: multiple Screen transitions in progress");
	fullscreen_count++;
	if (fullscreen_oldimage) [fullscreen_oldimage release];
	fullscreen_oldimage = fullscreen_previmage;
	fullscreen_previmage = NULL;
}

- (void) endScreenTransition
{
	AM_DBG NSLog(@"endScreenTransition");
	assert(fullscreen_count > 0);
	fullscreen_count--;
}

- (void) screenTransitionStep: (ambulant::smil2::transition_engine *)engine
		elapsed: (ambulant::lib::transition_info::time_type)now
{
	AM_DBG NSLog(@"screenTransitionStep %d", (int)now);
	assert(fullscreen_count > 0);
	fullscreen_engine = engine;
	fullscreen_now = now;
}

- (void) _screenTransitionPreRedraw
{
	if (fullscreen_count == 0) return;
	// XXX setup drawing to transition surface
	AM_DBG NSLog(@"_screenTransitionPreRedraw: setup for transition redraw");
	[[self getTransitionSurface] lockFocus];
}

- (void) _screenTransitionPostRedraw
{
	if (fullscreen_count == 0 && fullscreen_oldimage == NULL) {
		// Neither in fullscreen transition nor wrapping one up.
		// Take a snapshot of the screen and return.
		if (fullscreen_previmage) [fullscreen_previmage release];
		fullscreen_previmage = [[self getOnScreenImageForRect: [self bounds]] retain];
		/*DBG	[self dump: fullscreen_previmage toImageID: "fsprev"]; */
		return;
	}
	if (fullscreen_oldimage == NULL) {
		// Just starting a new fullscreen transition. Get the
		// background bits from the snapshot saved during the previous
		// redraw.
		fullscreen_oldimage = fullscreen_previmage;
		fullscreen_previmage = NULL;
	}
	
	// Do the transition step, or simply copy the bits
	// if no engine available.
	AM_DBG NSLog(@"_screenTransitionPostRedraw: bitblit");
	[[self getTransitionSurface] unlockFocus];
//	/*DBG*/	[self dump: [self getTransitionOldSource] toImageID: "fsold"];
//	/*DBG*/	[self dump: [self getTransitionNewSource] toImageID: "fsnew"];
	CGRect bounds = [self bounds];
	if (fullscreen_engine) {
		[[self getTransitionOldSource] drawInRect: bounds
			fromRect: bounds
			operation: NSCompositeCopy
			fraction: 1.0];
		fullscreen_engine->step(fullscreen_now);
	} else {
		AM_DBG NSLog(@"_screenTransitionPostRedraw: no screen transition engine");
//		[[self getTransitionNewSource] compositeToPoint: NSZeroPoint
//			operation: NSCompositeCopy];
		[[self getTransitionNewSource] drawInRect: bounds
			fromRect: bounds
			operation: NSCompositeCopy
			fraction: 1.0];
	}

	if (fullscreen_count == 0) {
		// Finishing a fullscreen transition.
		AM_DBG NSLog(@"_screenTransitionPostRedraw: cleanup after transition done");
		if (fullscreen_oldimage) [fullscreen_oldimage release];
		fullscreen_oldimage = NULL;
		fullscreen_engine = NULL;
	}
}

// Called by a renderer if it requires an overlay window.
// The overlay window is refcounted.
- (void) requireOverlayWindow
{
	[self performSelectorOnMainThread:@selector(_createOverlayWindow:) withObject: nil waitUntilDone:YES];
}

- (void) _createOverlayWindow: (id)dummy
{
#ifdef WITH_QUICKTIME_OVERLAY
	AM_DBG NSLog(@"requireOverlayWindow");
	if (overlay_window) {
		// XXXJACK shoould inc refcount here
		return;
	}
	// Find out where to position the window
	NSPoint baseOrigin = NSMakePoint([self frame].origin.x, [self frame].origin.y);
	NSPoint screenOrigin = [[self window] convertBaseToScreen: baseOrigin];
	
	// Create the window
	overlay_window = [[NSWindow alloc] initWithContentRect: 
		CGRectMake(screenOrigin.x,screenOrigin.y,[self frame].size.width,[self frame].size.height) 
		styleMask:NSBorderlessWindowMask 
		backing:NSBackingStoreBuffered 
		defer:YES];
	NSView *oview = [[MyFlippedView alloc] initWithFrame: [self bounds]];
	[overlay_window setContentView: oview];
	[overlay_window setBackgroundColor: [NSColor clearColor]];
	//[overlay_window setBackgroundColor: [NSColor colorWithCalibratedRed: 0.5 green: 0.0 blue:0.0 alpha: 0.5]]; // XXXJACK
	[overlay_window setOpaque:NO];
	[overlay_window setHasShadow:NO];
	[overlay_window setIgnoresMouseEvents:YES];
	[[self window] addChildWindow: overlay_window ordered: NSWindowAbove];
#endif // WITH_QUICKTIME_OVERLAY
}

// Called by a renderer redraw() if subsequent redraws in the current redraw sequence
// should go to the overlay window
- (void) useOverlayWindow
{
#ifdef WITH_QUICKTIME_OVERLAY
	AM_DBG NSLog(@"useOverlayWindow");
	assert(overlay_window);
	if (overlay_window_needs_unlock) {
		NSLog(@"userOverlayWindow: already lockFocus'sed");
		return;
	}
	NSView *oview = [overlay_window contentView];
	overlay_window_needs_unlock = YES;
	[oview lockFocus];
	// No need to clear, did that in drawRect: already
#endif // WITH_QUICKTIME_OVERLAY
}

// Called by a renderer if the overlay window is no longer required.
- (void) releaseOverlayWindow
{
#ifdef WITH_QUICKTIME_OVERLAY
	AM_DBG NSLog(@"releaseOverlayWindow");
	// XXXJACK Currently we don't actually delete the overlay window once it
	// has been created (until the presentation stops). Need to work out whether
	// this is indeed a good idea.
#endif // WITH_QUICKTIME_OVERLAY
}

// Called by the window manager when our view has moved to a different window.
// XXXJACK not sure wheter implementing this or viewDidMoveToSuperview is better,
// both seem to work.
- (void) viewDidMoveToSuperview
{
#ifdef WITH_QUICKTIME_OVERLAY
	if (overlay_window == nil) return;
	AM_DBG NSLog(@"viewDidMoveToWindow");
	NSWindow *window = [self window];
	if (window == nil) {
		// Remove. Ignore, assume another call is coming when we're re-attached
		// to another window.
		AM_DBG NSLog(@"Ignore viewDidMoveToWindow -> nil");
		return;
	}
	overlay_window_needs_reparent = YES;
#endif // WITH_QUICKTIME_OVERLAY
}
#endif // NOT_YET_UIKIT
@end
#endif // __OBJC__
