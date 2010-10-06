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

#ifdef WITH_AVFOUNDATION


extern "C" void* call_C_function(void* args, void*(*fun)(void*arg)) {
	return fun(args);
};
						
#include "ambulant/gui/cg/cg_gui.h"
#include "ambulant/gui/cg/cg_avfoundation_video.h"
#include "ambulant/common/region_info.h"
#include "ambulant/common/smil_alignment.h"
#include "ambulant/smil2/test_attrs.h"

//#define AM_DBG
#ifndef AM_DBG
#define AM_DBG if(0)
#endif

#ifdef	__OBJC__
@interface pair : NSObject
{
	NSObject* first;
	NSObject* second;
}

- (pair*) initWithFirst:(NSObject*) first Second: (NSObject*) second;

@property (nonatomic, retain) NSObject*first;
@property (nonatomic, retain) NSObject*second;

@end

@implementation pair
@synthesize first, second;

- (pair*)
initWithFirst:(NSObject*) first_NSObject Second: (NSObject*) second_NSObject {
	[super init];
	self.first = first_NSObject;
	self.second = second_NSObject;
	return self;
}

@end

@implementation CGVideoAVPlayerManager

@synthesize duration, durationIsKnown, url, avplayer_item;

- (AVPlayer*)
s_avplayer {
	return s_avplayer;
}

- (void)
setS_avplayer:(AVPlayer*) x {
	return; // setter is no-op
}

- (void)
handleDurationDidChange {
	duration = s_avplayer.currentItem.asset.duration;
	AM_DBG NSLog(@"duration changed to:"); CMTimeShow(duration);
//X	[self updateControls];
}

- (void)
handlePlayerStatusDidChange {
	AVPlayerStatus playerStatus = s_avplayer.status;
	AM_DBG NSLog(@"status changed to: %d", playerStatus);
	if (playerStatus == AVPlayerStatusReadyToPlay) {
		[s_avplayer play];
	}
}

- (void)
handlePlayerError {
	AVPlayerStatus playerStatus = s_avplayer.status;
	AM_DBG NSLog(@"Error: status changed to: %d", playerStatus);
	NSError* error = s_avplayer.currentItem.error;
	if (error != NULL) {
		AM_DBG NSLog(@"Error is: %@", error);
//		[error release];
	}
	if (err_fun != NULL) {
		err_fun(err_arg);
	}
}


- (void)
handlePresentationSize {
	CGSize cgsz = avplayer_item.presentationSize;
	AM_DBG NSLog(@"presentationSize changed to: (%d,%d)", cgsz.width, cgsz.height);
	//X	[self updateControls];
}

- (void)
handlePlayerItemDidReachEnd:(NSNotification*) notification {
	AM_DBG NSLog(@"handlePlayerItemDidReachEnd: s_avplayer.status=%d",s_avplayer.status);
	if (eod_fun != NULL) {
		eod_fun(eod_arg);
	}
	[s_avplayer pause];
	[s_avplayer seekToTime:kCMTimeZero];	
	
}

/*
 * currently we don't use timeObserver, but it can be useful e.g. to accurately display video progress.
 * end-of-movie detection is now done by AVPlayerItemDidPlayToEndTimeNotification.
 
 @synthesize timeObserver;

 - (void)addTimeObserver {
 [self removeTimeObserver];
 timeObserver = [s_avplayer addPeriodicTimeObserverForInterval:CMTimeMakeWithSeconds(1, NSEC_PER_SEC) queue:nil usingBlock:^(CMTime time) {
 //TBD	[updateControls];
 //TBD	[checkIfAtEndOfMovie];
 }];
 
 }
 
 - (void)
 removeTimeObserver {
 if (timeObserver != nil) {
 [s_avplayer removeTimeObserver:timeObserver];
 timeObserver = nil;
 }
 }
*/

- (CGVideoAVPlayerManager*)
initWithURL:(NSURL*) nsurl {
	NSAutoreleasePool *pool = [[NSAutoreleasePool alloc] init];
	avplayer_item = [[AVPlayerItem alloc] initWithURL: nsurl];
	if (s_avplayer == NULL || s_avplayer.currentItem == NULL) {
		s_avplayer = [[[AVPlayer alloc] initWithPlayerItem:avplayer_item] retain];
	} else {
		[s_avplayer pause];
		@try { 
			// sometimes here I get an exception 'NSRangeException', reason:
			//'Cannot remove an observer ... for the key path "presentationSize" from <AVPlayerItem ...> because it is not registered as an observer.'
			[s_avplayer replaceCurrentItemWithPlayerItem: avplayer_item];
		}
		@catch (NSException * e) {
			AM_DBG NSLog(@"CGVideoAVPlayerManager.initWithURL() exception ignored");
		}
		@finally {
		}
	}
	AM_DBG NSLog(@"CGVideoAVPlayerManager.initWithURL(%@) self=0x%x self.retainCount=%d s_avplayer=0x%x [s_avplayer retainCount]=%d av_player_item=0x%x [avplayer_item retainCount]=%d", nsurl, self, [self retainCount], s_avplayer, [s_avplayer retainCount], avplayer_item, [avplayer_item retainCount]);
	[s_avplayer addObserver:self forKeyPath:@"status" options:0 context:nil];
	[s_avplayer addObserver:self forKeyPath:@"currentItem.asset.duration" options:0 context:nil];
	[s_avplayer addObserver:self forKeyPath:@"currentItem.error" options:0 context:nil];
	s_avplayer.actionAtItemEnd = AVPlayerActionAtItemEndPause;
	// prepare to react after keyboard show/hide
	[[NSNotificationCenter defaultCenter]
	 addObserver:self
	 selector:@selector(handlePlayerItemDidReachEnd:)
	 name:AVPlayerItemDidPlayToEndTimeNotification
	 object: [s_avplayer currentItem]];
//	call_C_function((void*)"Hello c-function\n", (void*(*)(void*)) printf); // how to call a C-function from Objective-C
	[self handleDurationDidChange];
	[self handlePlayerStatusDidChange];
//	[avplayer_item addObserver:avplayer_item forKeyPath:@"presentationSize" options:0 context:nil];

	url = nsurl;
//	[self addTimeObserver];
	[pool release];
	return self;
}

- (void)
dealloc {
//	[avplayer removeTimeObserver:timeObserver];
//	[timeObserver release];
	NSAutoreleasePool *pool = [[NSAutoreleasePool alloc] init];
	[[NSNotificationCenter defaultCenter]
	 removeObserver:self
	 name:AVPlayerItemDidPlayToEndTimeNotification
	 object: [s_avplayer currentItem]];
	[s_avplayer removeObserver:self forKeyPath:@"status"];
	[s_avplayer removeObserver:self forKeyPath:@"currentItem.asset.duration"];
	[s_avplayer removeObserver:self forKeyPath:@"currentItem.error"];
	AM_DBG NSLog(@"CGVideoAVPlayerManager.dealloc(0x%x) s_avplayer=0x%x [s_avplayer retainCount=%d av_player_item=0x%x [avplayer_item retainCount]=%d", self, s_avplayer, [s_avplayer retainCount], avplayer_item, [avplayer_item retainCount]);

	[avplayer_item release];
	[url release];
	[pool release];
	
	[super dealloc];
}

- (void)
onEndOfDataCall:(void*(*)(void*))fun withArg: (void*) arg {
	eod_arg = arg;
	eod_fun = fun;
}

- (void)
onErrorCall:(void*(*)(void*))fun withArg: (void*) arg {
	err_arg = arg;
	err_fun = fun;
}

- (void)
observeValueForKeyPath:(NSString *)keyPath ofObject:(id)object change:(NSDictionary *)change context:(void *)context {
	if ([keyPath isEqualToString:@"status"] && object == s_avplayer)
		[self handlePlayerStatusDidChange];
	else if ([keyPath isEqualToString:@"currentItem.asset.duration"] && object == s_avplayer)
		[self handleDurationDidChange];
	else if ([keyPath isEqualToString:@"currentItem.error"] && object == s_avplayer)
		[self handlePlayerError];
	else if ([keyPath isEqualToString:@"presentationSize"] && object == avplayer_item)
		[self handlePresentationSize];
}

- (void)
play {
	[s_avplayer play];
}

- (void)
pause {
	[s_avplayer pause];
}

- (void)
removeFromSuper:(pair*) objs {
	if (objs == NULL)
		return;
	else {
		[(AVPlayerLayer*) objs.second removeFromSuperlayer];
		return;
	}
}
										 
@end
#endif//__OBJC__
										  
// These two constants should match. Moreover, the optimal setting may depend on the
// specific hardware.
// XXXJACK: we should get rid of these, analoguous to what cocoa_dsvideo does:
// Get the information dynamically.
#if 0
#define MY_PIXEL_LAYOUT net::pixel_argb
#define MY_BITMAP_INFO (kCGImageAlphaNoneSkipFirst|kCGBitmapByteOrder32Host)
#define MY_BPP 4
#endif

namespace ambulant {

using namespace lib;

namespace gui {

namespace cg {

extern const char cg_avfoundation_video_playable_tag[] = "video";
	extern const char cg_avfoundation_video_playable_renderer_uri[] = AM_SYSTEM_COMPONENT("RendererCoreGraphics");
	extern const char cg_avfoundation_video_playable_renderer_uri2[] = AM_SYSTEM_COMPONENT("RendererAVFoundation");
	extern const char cg_avfoundation_video_playable_renderer_uri3[] = AM_SYSTEM_COMPONENT("RendererVideo");

common::playable_factory *
create_cg_avfoundation_video_playable_factory(common::factories *factory, common::playable_factory_machdep *mdp)
{

	smil2::test_attrs::set_current_system_component_value(AM_SYSTEM_COMPONENT("RendererCoreGraphics"), true);
	smil2::test_attrs::set_current_system_component_value(AM_SYSTEM_COMPONENT("RendererVideo"), true);
	smil2::test_attrs::set_current_system_component_value(AM_SYSTEM_COMPONENT("RendererAVFoundation"), true);
	return new common::single_playable_factory<
		cg_avfoundation_video_renderer,
		cg_avfoundation_video_playable_tag,
		cg_avfoundation_video_playable_renderer_uri,
		cg_avfoundation_video_playable_renderer_uri2,
		cg_avfoundation_video_playable_renderer_uri3>(factory, mdp);
}

cg_avfoundation_video_renderer::cg_avfoundation_video_renderer(
	playable_notification *context,
	playable_notification::cookie_type cookie,
	const lib::node *node,
	event_processor *evp,
	common::factories *factory,
	common::playable_factory_machdep *mdp)
:	renderer_playable(context, cookie, node, evp, factory, mdp),
	m_url(),
	m_avplayer_manager(NULL),
	m_avplayer_layer(NULL),
	m_avplayer_view(NULL),
	m_paused(false),
	m_previous_clip_position(-1),
	m_renderer_state(rs_created)
{
	AM_DBG lib::logger::get_logger()->debug("cg_avfoundation_video_renderer(): 0x%x created", (void*)this);
}
	
cg_avfoundation_video_renderer::~cg_avfoundation_video_renderer()
{
	if (m_renderer_state == rs_fullstopped) {
		
	}
	m_lock.enter();
	AM_DBG logger::get_logger()->debug("~cg_avfoundation_video_renderer(0x%x) [m_avplayer_manager retainCount]=%d", (void *)this, m_avplayer_manager == NULL ? -999 : [m_avplayer_manager retainCount]);
	pair* objs = [[pair alloc] initWithFirst:m_avplayer_view Second:m_avplayer_layer];	
	
	if (m_avplayer_view != NULL) {
		m_lock.leave();
		[m_avplayer_manager performSelectorOnMainThread:@selector(removeFromSuper:)withObject: objs waitUntilDone:YES];
		m_lock.enter();
		m_avplayer_layer = NULL;
		m_avplayer_view = NULL;
	}
	[objs release];

	if (m_avplayer_layer != NULL) {
		m_avplayer_layer == NULL;
	}
	if (m_avplayer_manager != NULL) {
		[m_avplayer_manager release];
		m_avplayer_manager = NULL;
	}
	m_lock.leave();
}

void
cg_avfoundation_video_renderer::init_with_node(const lib::node *n)
{
	m_lock.enter();
	NSAutoreleasePool *pool = [[NSAutoreleasePool alloc] init];
	renderer_playable::init_with_node(n);
	assert(m_renderer_state == rs_created || m_renderer_state == rs_prerolled || m_renderer_state == rs_stopped || m_renderer_state == rs_fullstopped);
	if ( ! (m_renderer_state == rs_created || m_renderer_state == rs_prerolled || m_renderer_state == rs_stopped || m_renderer_state == rs_fullstopped)) {
		lib::logger::get_logger()->debug("cg_avfoundation_video_renderer::init_with_node() called while m_renderer_state==%d", m_renderer_state);
		return;
	}
	m_renderer_state = rs_inited;
	CMTime cm_clip_begin, cm_clip_end;
	
	m_node = n;
	if (m_avplayer_manager == NULL) {
		assert(m_url.is_empty_path() || m_url.same_document(m_node->get_url("src")));
		// Apparently the first call.
		m_url = m_node->get_url("src");
		NSURL *nsurl = [NSURL URLWithString: [NSString stringWithCString: m_url.get_url().c_str() encoding: NSUTF8StringEncoding]];
		if (!nsurl) {
			lib::logger::get_logger()->error(gettext("%s: cannot convert to URL"), m_url.get_url().c_str());
			goto bad;
		}
		m_avplayer_manager = [[CGVideoAVPlayerManager alloc] initWithURL:nsurl];
		[m_avplayer_manager onEndOfDataCall: &ambulant::gui::cg::cg_avfoundation_video_renderer::eod_reached withArg: this];
		[m_avplayer_manager onErrorCall: &ambulant::gui::cg::cg_avfoundation_video_renderer::error_occurred withArg: this];
	}
	_init_clip_begin_end();
	cm_clip_begin = CMTimeMakeWithSeconds((Float64)m_clip_begin, 1);
	cm_clip_begin.timescale = USEC_PER_SEC;
	[s_avplayer seekToTime: cm_clip_begin];
	cm_clip_end = CMTimeMakeWithSeconds((Float64)m_clip_end, 1);
	cm_clip_end.timescale = USEC_PER_SEC;
	s_avplayer.currentItem.forwardPlaybackEndTime = cm_clip_end;
	AM_DBG lib::logger::get_logger()->debug("cg_avfoundation_video_renderer(0x%x)::init_with_node, s_avplayer=0x%x, url=%s, clipbegin=%d", this, s_avplayer, m_url.get_url().c_str(), m_clip_begin);
	
bad:
	[pool release];
	AM_DBG lib::logger::get_logger()->debug("cg_avfoundation_video_renderer(0x%x)::init_with_node, [m_avplayer_manager retainCount]=%d", (void *)this, m_avplayer_manager == NULL ? -999 : [m_avplayer_manager retainCount]);
	m_lock.leave();
}
	
void
cg_avfoundation_video_renderer::start(double where) {
	m_lock.enter();
	NSAutoreleasePool *pool = [[NSAutoreleasePool alloc] init];
	AM_DBG lib::logger::get_logger()->debug("cg_avfoundation_video_renderer(0x%x)::start, s_avplayer=0x%x, [m_avplayer_manager retainCount]=%d where=%lf", this, s_avplayer, [m_avplayer_manager retainCount], where);

	if (m_avplayer_manager) {
		[m_avplayer_manager play];
		m_dest->show(this);
		m_renderer_state = rs_started;
		m_context->started(m_cookie, where);
	}
	[pool release];

	m_lock.leave();
}

bool
cg_avfoundation_video_renderer::stop() {
	NSAutoreleasePool *pool = [[NSAutoreleasePool alloc] init];
	AM_DBG lib::logger::get_logger()->debug("cg_avfoundation_video_renderer(0x%x)::stop, s_avplayer=0x%x, [m_avplayer_manager retainCount]=%d", this, s_avplayer,  [m_avplayer_manager retainCount]);

	m_lock.enter();
	bool rv = true;
	[m_avplayer_manager pause];
	m_context->stopped(m_cookie);
	m_renderer_state = rs_stopped;
	m_dest->need_redraw();
	[pool release];
	m_lock.leave();

	return rv;
}
	
void
cg_avfoundation_video_renderer::post_stop() {
	AM_DBG lib::logger::get_logger()->debug("cg_avfoundation_video_renderer(0x%x)::post_stop, s_avplayer=0x%x, [m_avplayer_manager retainCount]=%d", this, s_avplayer, [m_avplayer_manager retainCount]);

	m_lock.enter();
	m_renderer_state = rs_fullstopped;
	if (m_dest != NULL) {
		m_dest->renderer_done(this); //already done by smil_player::stop_playable()
	}
	m_lock.leave();
}
	
void
cg_avfoundation_video_renderer::resume() {
	AM_DBG lib::logger::get_logger()->debug("cg_avfoundation_video_renderer(0x%x)::resume, s_avplayer=0x%x, rate=%lf", this, s_avplayer, [s_avplayer rate]);

	m_lock.enter();
	m_renderer_state = rs_started;
	if (m_dest != NULL) {
		m_dest->need_redraw();
	}
	m_lock.leave();
}
	
void
cg_avfoundation_video_renderer::pause(pause_display d) {

	m_lock.enter();
	AM_DBG lib::logger::get_logger()->debug("cg_avfoundation_video_renderer(0x%x)::pause, s_avplayer=0x%x, rate=%lf, pause_display=%d" , this, s_avplayer, [s_avplayer rate], d);
	m_renderer_state = rs_stopped;
	if (m_dest != NULL) {
		m_dest->need_redraw();
	}
	m_lock.leave();
}

common::duration 
cg_avfoundation_video_renderer::get_dur() {
	common::duration rv = common::duration(false, 0);
	if ([m_avplayer_manager durationIsKnown]) {
		CMTime cm_duration = [m_avplayer_manager duration]; 
		if (cm_duration.flags = kCMTimeFlags_Valid)
			rv = common::duration(true, (double) cm_duration.value * (double) cm_duration.timescale);
	}
	AM_DBG lib::logger::get_logger()->debug("cg_avfoundation_video_renderer(0x%x)::get_dur returns: %d, %lf " , this, (bool) rv.first, (double) rv.second);
	return rv;
}
	
void*
cg_avfoundation_video_renderer::eod_reached(void* arg) {
	cg_avfoundation_video_renderer* cavr = (cg_avfoundation_video_renderer*) arg;
	AM_DBG lib::logger::get_logger()->debug("cg_avfoundation_video_renderer(0x%x)::eod_eached, s_avplayer=0x%x" , cavr, s_avplayer);
	cavr->stop();
	return NULL;
}
	
void*
cg_avfoundation_video_renderer::error_occurred(void* arg) {
	cg_avfoundation_video_renderer* cavr = (cg_avfoundation_video_renderer*) arg;
	if (cavr->m_renderer_state == rs_error_state) {
		return NULL;
	}
	NSError* nserror = s_avplayer.currentItem.error;
	AM_DBG lib::logger::get_logger()->debug("cg_avfoundation_video_renderer(0x%x)::error_occurred, s_avplayer=0x%x, error=%s" , cavr, s_avplayer, [[nserror localizedDescription] UTF8String]);
	cavr->stop();
	cavr->m_renderer_state == rs_error_state;
	return NULL;
}
	
void
cg_avfoundation_video_renderer::redraw(const rect &dirty, gui_window *window)
{
	m_lock.enter();
	NSAutoreleasePool *pool = [[NSAutoreleasePool alloc] init];
	const rect &r = m_dest->get_rect();
	cg_window *cwindow = (cg_window *)window;
	AmbulantView *view = (AmbulantView *)cwindow->view();
	
	AM_DBG logger::get_logger()->debug("cg_avfoundation_video_renderer.redraw(0x%x, local_ltrb=(%d,%d,%d,%d)),  [m_avplayer_manager retainCount]=%d", (void *)this, r.left(), r.top(), r.right(), r.bottom(), [m_avplayer_manager retainCount]);
	
	assert(m_renderer_state == rs_started || m_renderer_state == rs_stopped || m_renderer_state == rs_fullstopped || m_renderer_state == rs_error_state);
	if (m_avplayer_manager == NULL || [s_avplayer error] != NULL) {
		[pool release];
		m_lock.leave();
		if (m_renderer_state != rs_error_state && m_avplayer_manager != NULL) {
			m_renderer_state = rs_error_state;
			NSString* ns_error = [[s_avplayer error] localizedDescription];
			lib::logger::get_logger()->error("cg_video: %s", [ns_error cStringUsingEncoding:NSUTF8StringEncoding]);
		}
		return;
	}
	if (m_avplayer_layer == NULL) {		
		// Determine current position and size.
		CALayer *superlayer = [view layer];
		m_avplayer_layer = [AVPlayerLayer playerLayerWithPlayer:s_avplayer];
		m_avplayer_view = view;
		[superlayer addSublayer:m_avplayer_layer];		
		CGSize cgsize = [m_avplayer_layer preferredFrameSize];
		if (cgsize.width == 0 || cgsize.height == 0) {
			cgsize.width = r.width();
			cgsize.height = r.height();
		}
		m_srcsize = size(int(cgsize.width), int(cgsize.height));
	}
	rect srcrect;
	rect dstrect = m_dest->get_fit_rect(m_srcsize, &srcrect, m_alignment);
	dstrect.translate(m_dest->get_global_topleft());
	CGRect frameRect = [view CGRectForAmbulantRectForLayout: &dstrect];
	m_avplayer_layer.frame = frameRect;

	if (m_renderer_state == rs_started) {
		[m_avplayer_manager play];
	} else if (m_renderer_state == rs_stopped) {
		[m_avplayer_manager pause];
	} 
	[pool release];
	m_lock.leave();
}

} // namespace cg

} // namespace gui

} //namespace ambulant
#endif // WITH_AVFOUNDATION

