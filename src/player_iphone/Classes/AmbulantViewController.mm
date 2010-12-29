//
//  AmbulantViewController.mm
//  Ambulant
//
//  Created by Kees Blom on 7/12/10.
//  Copyright CWI 2010. All rights reserved.
//

#import "AmbulantViewController.h"
#import "AmbulantAppDelegate.h"
#import "SettingsViewController.h"
#import <QuartzCore/QuartzCore.h>

//#define AM_DBG
#ifndef AM_DBG
#define AM_DBG if(0)
#endif


void
document_embedder::show_file(const ambulant::net::url& href)
{
	AM_DBG ambulant::lib::logger::get_logger()->trace("document_embedder::show_file(0x%x) href=%s", this, href.get_url().c_str());
	document_embedder::open(href, true, NULL);
}

void
document_embedder::close(ambulant::common::player *p)
{
	AM_DBG ambulant::lib::logger::get_logger()->trace("document_embedder::close(0x%x) player=%0x%x", this, p);
	[m_mydocument performSelectorOnMainThread: @selector(close:) withObject: nil waitUntilDone: NO];
}

void
document_embedder::open(ambulant::net::url newdoc, bool start, ambulant::common::player *old)
{
	AM_DBG ambulant::lib::logger::get_logger()->trace("document_embedder::open(0x%x) new_doc=%s start=%d old_player=%0x%x", this,  newdoc.get_url().c_str(), start, old);
#ifdef WITH_OVERLAY_WINDOW
	if (newdoc.get_protocol() == "ambulant_aux") {
		std::string aux_url = newdoc.get_url();
		aux_url = aux_url.substr(13);
		ambulant::net::url auxdoc = ambulant::net::url::from_url(aux_url);
		aux_open(auxdoc);
	}
#endif
	
	if (old) {
		AM_DBG NSLog(@"performSelectorOnMainThread: close: on 0x%x", (void*)m_mydocument);
		[m_mydocument performSelectorOnMainThread: @selector(close:) withObject: nil waitUntilDone: NO];
	}
	NSAutoreleasePool *pool = [[NSAutoreleasePool alloc] init];
	NSString *str_url = [NSString stringWithUTF8String: newdoc.get_url().c_str()];
	AmbulantAppDelegate *delegate = [[UIApplication sharedApplication] delegate];
	[delegate performSelectorOnMainThread: @selector(openWebLink:)
							   withObject: str_url	waitUntilDone: NO];
	
	[pool release];
}

@implementation AmbulantViewController

//@synthesize delegate;
//@synthesize interactionView;
//@synthesize modeBar;
//@synthesize playerView;
//@synthesize myMainloop;
//@synthesize playPauseButton;
//@synthesize currentOrientation;
@synthesize autoCenter;
@synthesize autoResize;
@synthesize nativeRenderer;
//@synthesize play_active;
//@synthesize historyViewController;

/*
// The designated initializer. Override to perform setup that is required before the view is loaded.
- (id) initWithNibName:(NSString *)nibNameOrNil bundle:(NSBundle *)nibBundleOrNil {
    if ((self = [super initWithNibName:nibNameOrNil bundle:nibBundleOrNil])) {
        // Custom initialization
    }
    return self;
}
*/

/*
// Implement loadView to create a view hierarchy programmatically, without using a nib.
- (void) loadView {
}
*/

- (bool) canPlay
{
    return myMainloop != NULL;
}

// create a new instance of the smil player
- (void) doPlayURL: (NSString*) theUrl fromNode: (NSString*) ns_node_repr {
    if (theUrl) {
        if (currentURL) [currentURL release];
        currentURL = [theUrl retain];
    }
	AM_DBG ambulant::lib::logger::get_logger()->trace("AmbulantViewController doPlayURL(0x%x): url=%s ns_node_repr=%s", self, currentURL? [ currentURL UTF8String]: "NULL", ns_node_repr? [ns_node_repr UTF8String] : "NULL");
	if (myMainloop != NULL) {
		myMainloop->stop();
		delete myMainloop;
        
	}
    if (!playerView) {
        (void)self.view; // This loads the view
    }
    assert(currentURL);
    assert(playerView);
    assert(embedder);	
	myMainloop = new mainloop([currentURL UTF8String], playerView, embedder);	
	if (myMainloop) {
		if (ns_node_repr != NULL) {
			std::string node_repr = [ns_node_repr UTF8String];
			myMainloop->goto_node_repr(node_repr);
		}
		[self showInteractionView: NO];
//		[self play]; // This will be done in viewDidAppear
	}
}

// display the Control Panel (as a HUD) at the bottom of the player view 
- (void) showInteractionView: (BOOL) want_show {
	if (want_show && interactionView.hidden) {
		interactionView.hidden = false;
		interactionView.opaque = true;
        [NSObject cancelPreviousPerformRequestsWithTarget: self selector:@selector(autoHideInteractionView) object:nil];
        [self performSelector:@selector(autoHideInteractionView) withObject:nil afterDelay:(NSTimeInterval)5.0];
	} else {
		interactionView.hidden = true;
		interactionView.opaque = false;
        [NSObject cancelPreviousPerformRequestsWithTarget: self selector:@selector(autoHideInteractionView) object:nil];
	}
}

- (void) autoHideInteractionView
{
    interactionView.hidden = true;
    interactionView.opaque = false;
}


- (void) awakeFromNib
{
    AM_DBG NSLog(@"AmbulantViewController viewDidLoad(0x%x)", self);

	// prepare to react when device is rotated
	[[UIDevice currentDevice] beginGeneratingDeviceOrientationNotifications];
	[[NSNotificationCenter defaultCenter]
        addObserver:self
        selector:@selector(orientationChanged:)
        name:UIDeviceOrientationDidChangeNotification
        object: nil];
	ambulant::iOSpreferences::get_preferences()->load_preferences();
	ambulant::iOSpreferences* prefs = ambulant::iOSpreferences::get_preferences();
	autoCenter = prefs->m_auto_center;
	autoResize = prefs->m_auto_resize;
	nativeRenderer = ! prefs->m_prefer_ffmpeg;
	
	embedder = new document_embedder(self);
}

- (void) initGestures
{
    assert(playerView);
	// prepare to react on "tap" gesture (select object in playerView with 1 finger tap)
	UITapGestureRecognizer *tapGesture = [[UITapGestureRecognizer alloc]
										  initWithTarget:self action:@selector(handleTapGesture:)];
	[playerView addGestureRecognizer:tapGesture];
    [tapGesture release];
	
	// prepare to react on "double tap" gesture (select object in playerView with 1 finger tap)
	UITapGestureRecognizer *doubleTapGesture = [[UITapGestureRecognizer alloc]
										  initWithTarget:self action:@selector(handleDoubleTapGesture:)];
	doubleTapGesture.numberOfTapsRequired = 2;
	[playerView addGestureRecognizer:doubleTapGesture];
    [doubleTapGesture release];
	
	
	// prepare to react on "longPress" gesture (hold finger in one spot, longer than 0.4 sec.)
    UILongPressGestureRecognizer *longPressGesture = [[UILongPressGestureRecognizer alloc]
													  initWithTarget:self action:@selector(handleLongPressGesture:)];
    [playerView addGestureRecognizer:longPressGesture];
    [longPressGesture release];

	// prepare to react on "pinch" gesture (zoom playerView with 2 fingers)
	UIPinchGestureRecognizer *pinchGesture = [[UIPinchGestureRecognizer alloc]
											  initWithTarget:self action:@selector(handlePinchGesture:)];
	[playerView addGestureRecognizer:pinchGesture];
    [pinchGesture release];

	// prepare to react on "pan" gesture (move playerView with one finger)
    UIPanGestureRecognizer *panGesture = [[UIPanGestureRecognizer alloc]
										  initWithTarget:self action:@selector(handlePanGesture:)];
    [playerView addGestureRecognizer:panGesture];
    [panGesture release];
}

// Implement viewDidLoad to do additional setup after loading the view, typically from a nib.
// - install gesture recognizers
- (void) viewDidLoad {
	AM_DBG NSLog(@"AmbulantViewController viewDidLoad(0x%x)", self);
    [super viewDidLoad];
    [self initGestures];
}

- (void)viewWillAppear:(BOOL)animated
{
	/*AM_DBG*/ NSLog(@"AmbulantViewController viewWillAppear(0x%x)", self);
}

- (void)viewDidAppear:(BOOL)animated
{
	/*AM_DBG*/ NSLog(@"AmbulantViewController viewDidAppear(0x%x)", self);
    [self play];
}


- (IBAction) handlePlayOrPauseTapped {
	AM_DBG NSLog(@"AmbulantViewController handlePlayOrPauseTapped(0x%x)", self);
	if (myMainloop) {
		if (myMainloop->is_play_active()) {
			[self pause];
		} else {
			[self play];
		}
	} else {
		[self doPlayURL: nil fromNode: nil];
	}
}

- (IBAction) handleRestartTapped {
	AM_DBG NSLog(@"AmbulantViewController handleRestartTapped(0x%x)", self);
	if (myMainloop != NULL) {
		myMainloop->restart(false);
	} else {
		[self doPlayURL: nil fromNode: nil];
	}
} 

/*	Code derived from Apple's developer documentation "Gesture Recognizers"*/

- (IBAction) handleLongPressGesture:(UILongPressGestureRecognizer *)sender {
	AM_DBG NSLog(@"AmbulantViewController handleLongPressGesture(0x%x): sender=0x%x", self, sender);
	CGPoint location = [sender locationInView:playerView];
	if ( ! [playerView tappedAtPoint:location]) {
//		[delegate showPresentationViews:self];
	}
};

- (IBAction) handleTapGesture:(UITapGestureRecognizer *)sender { // select
	AM_DBG NSLog(@"AmbulantViewController handleTapGesture(0x%x): sender=0x%x", self, sender);
	[self showInteractionView: YES];
}

- (IBAction) handleDoubleTapGesture:(UITapGestureRecognizer *)sender { // select
	AM_DBG NSLog(@"AmbulantViewController handleDoubleTapGesture(0x%x): sender=0x%x", self, sender);
	CGPoint location = [sender locationInView:playerView];
	[playerView autoZoomAtPoint:location];
}

- (void) adjustAnchorPointForGestureRecognizer:(UIGestureRecognizer *)gestureRecognizer {
    if (gestureRecognizer.state == UIGestureRecognizerStateBegan) {
        UIView *piece = gestureRecognizer.view;
        CGPoint locationInView = [gestureRecognizer locationInView:piece];
        CGPoint locationInSuperview = [gestureRecognizer locationInView:piece.superview];
        
        piece.layer.anchorPoint = CGPointMake(locationInView.x / piece.bounds.size.width, locationInView.y / piece.bounds.size.height);
        piece.center = locationInSuperview;
    } else if (gestureRecognizer.state == UIGestureRecognizerStateEnded) {
        UIView *piece = gestureRecognizer.view;
        CGPoint centerInView = CGPointMake(piece.bounds.size.width/2, piece.bounds.size.height/2);
        CGPoint centerInSuperview = [piece convertPoint: centerInView toView: piece.superview];
        piece.layer.anchorPoint = CGPointMake(0.5, 0.5);
        piece.center = centerInSuperview;
    }
}

- (IBAction) handlePinchGesture:(UIPinchGestureRecognizer *)sender { // zoom
	AM_DBG NSLog(@"AmbulantViewController handlePinchGesture(0x%x): sender=0x%x", self, sender);
    [self adjustAnchorPointForGestureRecognizer: sender];
	CGFloat factor = [(UIPinchGestureRecognizer *)sender scale];
	[playerView zoomWithScale:factor inState: [sender state]];
}

- (IBAction) handlePanGesture:(UIPanGestureRecognizer *)sender {
	AM_DBG NSLog(@"AmbulantViewController handlePanGesture(0x%x): sender=0x%x", self, sender);
	CGPoint translate = [sender translationInView: playerView.superview];
	[playerView  translateWithPoint: (CGPoint) translate inState: [sender state]];
}
- (IBAction) showSettings:(id)sender { //JNK
	AM_DBG NSLog(@"AmbulantViewController showSettings(0x%x)", self);
	
	if (myMainloop != NULL) {
		myMainloop->pause();
	}
	SettingsViewController *controller = [[SettingsViewController alloc]
										  initWithNibName:@"SettingsViewController" bundle:nil];
	controller.title = @"Settings";
	controller.delegate = self;
	
	controller.modalTransitionStyle = UIModalTransitionStyleFlipHorizontal;
	[self presentModalViewController:controller animated:YES];
	
	[controller release];
}

- (IBAction) playNextItem {
	AM_DBG NSLog(@"AmbulantViewController playNextItem(0x%x)", self);
	if (currentPresentationViewController != NULL) {
		[currentPresentationViewController selectNextPresentation];
	}
}

- (IBAction) addFavorites:(id)sender
{
	AM_DBG NSLog(@"AmbulantViewController addFavorites(0x%x)", sender);
	PresentationViewController* favoritesVC = [ delegate getPresentationViewWithIndex: 1];	
#ifdef	FIRST_ITEM
	[favoritesVC insertCurrentItemAtIndexPath: [ NSIndexPath indexPathForRow:FIRST_ITEM inSection: 0 ]];
#else //FIRST_ITEM
	[favoritesVC insertCurrentItemAtIndexPath: [ NSIndexPath indexPathForRow:0 inSection: 0 ]];
#endif//FIRST_ITEM
#ifdef XXXJACK_IS_UNSURE
	[delegate showPresentationViewWithIndex: 1];
#endif
}


- (void) settingsHaveChanged:(SettingsViewController *)controller {
	AM_DBG NSLog(@"AmbulantViewController showSettings(0x%x)", self);
	// check we have the settings view
	if (controller.view.tag != 40) {
		return;
	}
	// get the values entered by the user
	autoCenter = [controller autoCenter];
	autoResize = [controller autoResize];
	nativeRenderer = [controller nativeRenderer];
	ambulant::iOSpreferences* prefs = ambulant::iOSpreferences::get_preferences();
	prefs->m_auto_center = autoCenter;
	prefs->m_auto_resize = autoResize;
	prefs->m_prefer_ffmpeg = ! nativeRenderer;
	if (myMainloop) {
		if (nativeRenderer) {
			myMainloop->get_playable_factory()->preferred_renderer(AM_SYSTEM_COMPONENT("RendererAVFoundation"));   
		} else {
			myMainloop->get_playable_factory()->preferred_renderer(AM_SYSTEM_COMPONENT("RendererOpen"));
		}
	}
	prefs->save_preferences();
}

- (void) playlistViewControllerDidFinish: (UIViewController *)controller {
    // XXXJACK: seems to be a duplicate of presentationViewControllDidFinish.
    // Even if it isn't: all the comments for that method also hold for this one.
	
	AM_DBG NSLog(@"playlistViewControllerDidFinish: controller=0x%x", controller);
	[self settingsHaveChanged: controller];	
	[delegate showAmbulantPlayer: (id) self];
	if (myMainloop != NULL) {
        [self play];
		ambulant::iOSpreferences* prefs = ambulant::iOSpreferences::get_preferences();
		UIDeviceOrientation orientation = [[UIDevice currentDevice] orientation];
		[playerView adaptDisplayAfterRotation: orientation withAutoCenter: prefs->m_auto_center withAutoResize: prefs->m_auto_resize];
	}
	if ( ! interactionView.hidden) {
		interactionView.hidden = true;
		interactionView.opaque = false;
		modeBar.hidden = true;
		modeBar.opaque = false;
	}
	playerView.alpha = 1.0;
}

- (IBAction) showHistory:(id)sender {
	AM_DBG NSLog(@"AmbulantViewController showHistory:(0x%x)", self);
	[self pause];
	[delegate showPresentationViews: self];
}

- (void) showAmbulantPlayer:(id)sender {
	[delegate showAmbulantPlayer:sender];
}

- (void) showPresentationViews:(id)sender {
    // XXXJACK: Needed?
	[delegate showPresentationViews:sender];
}

- (void) done: (id) sender {
    // XXXJACK: needed?
	AM_DBG NSLog(@"AmbulantViewController done(0x%x): sender=0x%x", self, sender);
	[self playlistViewControllerDidFinish: (UIViewController*) sender];
}

- (void) presentationViewControllerDidFinish: (PresentationViewController *)controller {
    // XXXJACK: some of the functionality here (ending editing) should move to the the originating
    // controller. The rest (showing the current view) to the AppDelegate.
	AM_DBG NSLog(@"AmbulantViewController presentationViewControllerDidFinish(0x%x): controller=0x%x", self, controller);
	ambulant::iOSpreferences* prefs = ambulant::iOSpreferences::get_preferences();
	[delegate showAmbulantPlayer: (id) self];
	if (controller != NULL && controller.editingStyle != UITableViewCellEditingStyleNone) {
		[controller toggleEditMode];
	}
	UIDeviceOrientation orientation = [[UIDevice currentDevice] orientation];
	[playerView adaptDisplayAfterRotation: orientation withAutoCenter: prefs->m_auto_center withAutoResize: prefs->m_auto_resize];
	if (myMainloop != NULL) {
		UIDeviceOrientation orientation = [[UIDevice currentDevice] orientation];
		[playerView adaptDisplayAfterRotation: orientation withAutoCenter: prefs->m_auto_center withAutoResize: prefs->m_auto_resize];
		
		[self play];
	}
	[self showInteractionView: NO];
	playerView.alpha = 1.0;
	prefs->save_preferences(); // save possible edits
}

- (void) setHistoryViewController:(PresentationViewController *)controller
{
	AM_DBG NSLog(@"AmbulantViewController setHistoryViewController(0x%x) controller=0x%x", self, controller);
	if ( ! controller.isFavorites) {
		historyViewController = controller;
	}
}
- (void) playPresentation: (NSString*) whatString fromPresentationViewController: (PresentationViewController*) controller {
    // XXXJACK: Change interface to get PlayListItem, which has the position as well.
	AM_DBG NSLog(@"AmbulantViewController (0x%x)", self);
	NSAutoreleasePool *pool = [[NSAutoreleasePool alloc] init];
	AM_DBG NSLog(@"Selected: %@",whatString);
	currentPresentationViewController = controller;
	if (myMainloop != NULL) {
		myMainloop->stop();
	}
	if ( ! ([whatString hasPrefix:@"file://"] || [whatString hasPrefix:@"http://"])) {
		// assume local file, check for aboslute path
		if ( ! [whatString hasPrefix:@"/"]) {
			NSString* homedir = NSHomeDirectory();
			homedir = [homedir stringByAppendingString:@"/player_iphone.app/"];
			whatString = [homedir stringByAppendingString:whatString];//[thisBundle pathForResource:whatString ofType:@"smil"];
		}
		whatString = [@"file://" stringByAppendingString:whatString];
	}
	if (whatString != NULL) {
		if ( ! [whatString hasSuffix:@".smil"]) {
			whatString = [whatString stringByAppendingString:@".smil"];
		}
		[self doPlayURL: whatString fromNode: nil];
		[historyViewController updatePlaylist];
	}
	[self done: self];
	[pool release];
}

- (BOOL) isSupportedOrientation: (UIDeviceOrientation) orientation {
	AM_DBG NSLog(@"AmbulantViewController isSupportedOrientation(0x%x) orientation=%d", self, orientation);
	return 
		orientation == UIDeviceOrientationPortrait
	||	orientation == UIDeviceOrientationPortraitUpsideDown
	||	orientation == UIDeviceOrientationLandscapeLeft
	||	orientation == UIDeviceOrientationLandscapeRight;
}

/* */
// Override to allow orientations other than the default portrait orientation.
- (BOOL) shouldAutorotateToInterfaceOrientation: (UIInterfaceOrientation) interfaceOrientation {
	AM_DBG NSLog(@"AmbulantViewController shouldAutorotateToInterfaceOrientation(0x%x): interfaceOrientation=%d", self, interfaceOrientation);
	return [self isSupportedOrientation:(UIDeviceOrientation) interfaceOrientation];
}

// react on device rotation
- (void) orientationChanged:(NSNotification *)notification {
//	AM_DBG NSLog(@"AmbulantViewController orientationChanged(0x%x):notification=%d", self, notification);
	UIDeviceOrientation orientation = [[UIDevice currentDevice] orientation];
	if (orientation == currentOrientation || ! [self isSupportedOrientation: orientation]) {
		return;
	}
	ambulant::iOSpreferences* prefs = ambulant::iOSpreferences::get_preferences();
	currentOrientation = orientation;
	if (playerView != NULL) {
		[playerView adaptDisplayAfterRotation: orientation withAutoCenter: prefs->m_auto_center withAutoResize: prefs->m_auto_resize];
	}
}

- (void) pause {
	AM_DBG NSLog(@"AmbulantViewController pause(0x%x)", self);
	if (myMainloop) {
		myMainloop->pause();
		UIImage* playImage = [UIImage imageNamed: @"Play_iPhone.png"];
	   [playPauseButton setImage:playImage forState:UIControlStateNormal];
	}
}

- (void) play {
	AM_DBG NSLog(@"AmbulantViewController play(0x%x)", self);
	if (myMainloop) {
		myMainloop->play();
		UIImage* pauseImage = [UIImage imageNamed: @"Pause_iPhone.png"];
		[playPauseButton setImage:pauseImage forState:UIControlStateNormal];
	}
}

- (PlaylistItem*) currentItem {
	AM_DBG NSLog(@"AmbulantViewController currentItem(0x%x)", self);
	if (myMainloop) {
		return myMainloop->get_current_item();
	}
	return NULL;
}

- (void)didReceiveMemoryWarning {
	// Releases the view if it doesn't have a superview.
 	AM_DBG NSLog(@"AmbulantViewController didReceiveMemoryWarning:self=0x%x", self);
   [super didReceiveMemoryWarning];
	
	// Release any cached data, images, etc that aren't in use.
}

- (void)viewDidUnload {
	// Release any retained subviews of the main view.
	// e.g. self.myOutlet = nil;
	AM_DBG NSLog(@"AmbulantViewController viewDidUnLoad:self=0x%x", self);
}

- (void) willTerminate
{
	if (myMainloop)
		delete myMainloop;
    myMainloop = NULL;
	[playerView release];
    playerView = nil;
	if (currentURL)
		[currentURL release];
    currentURL = nil;
}

- (void)dealloc {
	AM_DBG NSLog(@"AmbulantViewController dealloc:self=0x%x", self);
    [super dealloc];
	if (myMainloop)
		delete myMainloop;
	[playerView release];
	if (currentURL)
		[currentURL release];
}

@end
