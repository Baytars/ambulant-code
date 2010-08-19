//
//  AmbulantViewController.mm
//  Ambulant
//
//  Created by Kees Blom on 7/12/10.
//  Copyright __MyCompanyName__ 2010. All rights reserved.
//

#import "AmbulantViewController.h"
#import "AmbulantAppDelegate.h"
#import "PlaylistViewController.h"

void
document_embedder::show_file(const ambulant::net::url& href)
{
//	CFStringRef cfhref = CFStringCreateWithCString(NULL, href.get_url().c_str(), kCFStringEncodingUTF8);
//	CFURLRef url = CFURLCreateWithString(NULL, cfhref, NULL);
//	OSErr status;
	
//	if ((status=LSOpenCFURLRef(url, NULL)) != 0) {
//		ambulant::lib::logger::get_logger()->trace("Opening URL <%s>: LSOpenCFURLRef error %d", href.get_url().c_str());
//		ambulant::lib::logger::get_logger()->error(gettext("Cannot open: %s"), href.get_url().c_str());
//	}
	document_embedder::open(href, true, NULL);
}

void
document_embedder::close(ambulant::common::player *p)
{
	[m_mydocument performSelectorOnMainThread: @selector(close:) withObject: nil waitUntilDone: NO];
}

void
document_embedder::open(ambulant::net::url newdoc, bool start, ambulant::common::player *old)
{
#ifdef WITH_OVERLAY_WINDOW
	if (newdoc.get_protocol() == "ambulant_aux") {
		std::string aux_url = newdoc.get_url();
		aux_url = aux_url.substr(13);
		ambulant::net::url auxdoc = ambulant::net::url::from_url(aux_url);
		aux_open(auxdoc);
	}
#endif
	
	if (old) {
		NSLog(@"performSelectorOnMainThread: close: on 0x%x", (void*)m_mydocument);
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

@synthesize interactionView, originalPlayerViewFrame, originalInteractionViewFrame,
			playerView, myMainloop, URLEntryField, linkURL, playURL, keyboardIsShown,
			currentOrientation,	autoCenter, autoResize, play_active;

/*
// The designated initializer. Override to perform setup that is required before the view is loaded.
- (id)initWithNibName:(NSString *)nibNameOrNil bundle:(NSBundle *)nibBundleOrNil {
    if ((self = [super initWithNibName:nibNameOrNil bundle:nibBundleOrNil])) {
        // Custom initialization
    }
    return self;
}
*/

/*
// Implement loadView to create a view hierarchy programmatically, without using a nib.
- (void)loadView {
}
*/


/*
// Implement viewDidLoad to do additional setup after loading the view, typically from a nib.
- (void)viewDidLoad {
    [super viewDidLoad];
}
*/
- (void) doPlayURL {
	if (myMainloop != NULL) {
		myMainloop->stop();
		delete myMainloop;
	}		
	myMainloop = new mainloop([[self playURL] UTF8String], playerView, embedder);	
	if (myMainloop) {
		myMainloop->play();
		self.URLEntryField.text = [self playURL];
	}
}

// Implement viewDidLoad to do additional setup after loading the view, typically from a nib.
- (void)viewDidLoad {
    [super viewDidLoad];
	// prepare to react after keyboard show/hide
	[[NSNotificationCenter defaultCenter]
	 addObserver:self
	 selector:@selector(keyboardWillShow:)
	 name:UIKeyboardWillShowNotification
	 object: nil];
	[[NSNotificationCenter defaultCenter]
	 addObserver:self
	 selector:@selector(keyboardWillHide:)
	 name:UIKeyboardWillHideNotification
	 object: nil];

	// prepare to react when device is rotated
	[[UIDevice currentDevice] beginGeneratingDeviceOrientationNotifications];
	[[NSNotificationCenter defaultCenter]
	 addObserver:self
	 selector:@selector(orientationChanged:)
	 name:UIDeviceOrientationDidChangeNotification
	 object: nil];
	autoCenter = ambulant::common::preferences::get_preferences()->m_auto_center;
	autoResize = ambulant::common::preferences::get_preferences()->m_auto_resize;
	
	// prepare to react on "tap" gesture (select object in playerView with 1 finger tap)
	UITapGestureRecognizer *tapGesture = [[UITapGestureRecognizer alloc]
											  initWithTarget:self action:@selector(handleTapGesture:)];
	[self.playerView addGestureRecognizer:tapGesture];
    [tapGesture release];
	
	// prepare to react on "pinch" gesture (zoom playerView with 2 fingers)
	UIPinchGestureRecognizer *pinchGesture = [[UIPinchGestureRecognizer alloc]
											  initWithTarget:self action:@selector(handlePinchGesture:)];
	[self.playerView addGestureRecognizer:pinchGesture];
    [pinchGesture release];
	// prepare to react on "pan" gesture (move playerView with one finger)
    UIPanGestureRecognizer *panGesture = [[UIPanGestureRecognizer alloc]
										  initWithTarget:self action:@selector(handlePanGesture:)];
    [self.playerView addGestureRecognizer:panGesture];
    [panGesture release];
	
	embedder = new document_embedder(self);
	NSLog(@"View=%@ playUrl=%@", [self playerView], [self playURL]);
	if (self.playURL != nil) {
		[self handleURLEntered];
	} else {
		NSBundle *thisBundle = [NSBundle bundleForClass:[self class]];
		NSString *welcomePath = [thisBundle pathForResource:@"Welcome" ofType:@"smil"];
//		NSString *welcomePath = @"http://ambulantPlayer.org/Demos/Birthday/HappyBirthday.smil";
		NSLog (@ "%@", welcomePath);
		if (welcomePath) {
			void* theview = [self playerView];
			NSLog(@"view %@ responds %d", (NSObject *)theview, [(NSObject *)theview respondsToSelector: @selector(isAmbulantWindowInUse)]);
			playURL = [[NSString alloc] initWithString: welcomePath];
			[self doPlayURL ];
		}
	} 
}

- (IBAction) handlePlayTapped {
	if (myMainloop) {
		myMainloop->play();
	} else {
		[self doPlayURL];
	}
}

- (IBAction) handlePauseTapped {
	if (myMainloop) {
		myMainloop->pause();
	}
}

- (IBAction) handleStopTapped {
	if (myMainloop == NULL) {
		return;
	}
	myMainloop->stop();
	if (playerView == NULL)
		//XXXX for some reason the playerView is reset to 0 when play starts
		playerView = (id) myMainloop->get_view();
	delete myMainloop;
//	[playerView release];
	myMainloop = NULL;
}


/*	Code from Apple's developer documentation "Gesture Recognizers"*/
- (IBAction) handleTapGesture:(UIGestureRecognizer *)sender { // select
	CGPoint location = [(UITapGestureRecognizer *)sender locationInView:self.playerView];
	[self.playerView tappedAtPoint:location];
}

//  XXXX cleanup needed: move the this code into genuine member function of AmbulantPlayer
- (IBAction) handlePinchGesture:(UIGestureRecognizer *)sender { // zoom
	CGFloat factor = [(UIPinchGestureRecognizer *)sender scale];
	[self.playerView zoomWithScale:factor inState: [sender state]];
}

//  XXXX cleanup needed: move the this code into genuine member function of AmbulantPlayer
- (IBAction) handlePanGesture:(UIPanGestureRecognizer *)sender {
	CGPoint translate = [sender translationInView: playerView.superview];
	[self.playerView  translateWithPoint: (CGPoint) translate inState: [sender state]];
}

// dismiss the keyboard when the <Return> is tapped
- (BOOL)textFieldShouldReturn:(UITextField *)textField {
	[textField resignFirstResponder]; // dismiss keyboard
	return YES;
}

- (IBAction) handleURLEntered {
	if (URLEntryField.text.length == 0 || URLEntryField.text == playURL) {
		return;
	}
	playURL = [[NSString alloc] initWithString: URLEntryField.text];
	[self doPlayURL];
}

- (IBAction) showPlaylist:(id)sender {    
	
	if (myMainloop != NULL) {
		play_active = myMainloop->is_play_active();
		myMainloop->pause();
	}
	PlaylistViewController *controller = [[PlaylistViewController alloc]
										  initWithNibName:@"PlaylistView" bundle:nil];
	controller.title = @"Playlist";
	controller.delegate = self;
	
	controller.modalTransitionStyle = 
	UIModalTransitionStyleFlipHorizontal;
	[self presentModalViewController:controller animated:YES];
	
	[controller release];
}


- (void) playlistViewControllerDidFinish: (PlaylistViewController *)controller {
	// get the values entered by the user
	autoCenter = [controller autoCenter];
	autoResize = [controller autoResize];
	ambulant::common::preferences::get_preferences()->m_auto_center = autoCenter;
	ambulant::common::preferences::get_preferences()->m_auto_resize = autoResize;
	/*
	timeZoneName = [controller selectedTimeZone];
	BOOL uses24Hour = [controller uses24Hour];
	NSString* selected24HourDisplay = uses24Hour ? @"YES" : @"NO";
	[clockPrefs setObject:timeZoneName forKey:TIME_ZONE_PREF_KEY];
	[clockPrefs setObject:selected24HourDisplay
				   forKey:TWENTY_FOUR_HOUR_PREF_KEY];
	/// save prefs to Documents folder
	[self savePrefs];
	// update displays to possibly changed prefs
	[self setClockToTimeZoneName:timeZoneName uses24Hour:uses24Hour];
	[self updateClockView];
	*/
	[self dismissModalViewControllerAnimated:YES];
	
	if (myMainloop != NULL) {
		if (play_active) {
			myMainloop->play();
		}
		UIDeviceOrientation orientation = [[UIDevice currentDevice] orientation];
		[playerView adaptDisplayAfterRotation: orientation];
	}
	
}

- (void) playIt: (PlaylistViewController *)controller selected: (NSString*) whatString {
	NSLog(@"Selected: %@",whatString);
	self.handleStopTapped;
	if ( ! [whatString hasPrefix:@"http://"]) {
		NSString* homedir = NSHomeDirectory();
		homedir = [homedir stringByAppendingString:@"/player_iphone.app/Documents/"];
		whatString = [homedir stringByAppendingString:whatString];//[thisBundle pathForResource:whatString ofType:@"smil"];
		if ( ! [whatString hasSuffix:@".smil"]) {
			whatString = [whatString stringByAppendingString:@".smil"];
		}
	}
	if (whatString != NULL) {
		if (playURL) {
			[playURL release];
		}
		playURL = [[NSString alloc] initWithString: whatString];
		[self doPlayURL];
	}
	[controller done: self];
}

- (void)keyboardWillShow:(NSNotification *)notification {
    
    /*
     Reduce the size of the playerView so that it's not obscured by the keyboard.
     Animate the resize so that it's in sync with the appearance of the keyboard.
     */
	if (keyboardIsShown) {
		return;
	}
	keyboardIsShown = true;
    NSDictionary *userInfo = [notification userInfo];
    
    // Get the height of the keyboard when it's displayed.
    NSValue* aValue = [userInfo objectForKey:UIKeyboardFrameEndUserInfoKey];
	
    // Get the top of the keyboard as the y coordinate of its origin in self's view's coordinate system.
	// The bottom of the text view's frame should align with the top of the keyboard's final position.
    CGRect keyboardRect = [aValue CGRectValue];
    keyboardRect = [self.view convertRect:keyboardRect fromView:nil];
	
	NSLog(@"keyboardRect=(%f,%f,%f,%f",
		  keyboardRect.origin.x,keyboardRect.origin.y,
		  keyboardRect.size.width,keyboardRect.size.height);
    CGFloat keyboardHeight = keyboardRect.size.height;
	originalInteractionViewFrame = interactionView.frame;
    CGRect newInteractionViewFrame = interactionView.frame;
	NSLog(@"newInteractionViewFrame=(%f,%f,%f,%f",
		  newInteractionViewFrame.origin.x,newInteractionViewFrame.origin.y,
		  newInteractionViewFrame.size.width,newInteractionViewFrame.size.height);
	newInteractionViewFrame.origin.y -= keyboardHeight;
	originalPlayerViewFrame = playerView.frame;
    CGRect newPlayerViewFrame = originalPlayerViewFrame;
	newPlayerViewFrame.origin.y -= keyboardHeight;
    // Get the duration of the animation.
    NSValue *animationDurationValue = [userInfo objectForKey:UIKeyboardAnimationDurationUserInfoKey];
    NSTimeInterval animationDuration;
    [animationDurationValue getValue:&animationDuration];
    
    // Animate the resize of the playerView's frame and the repositioning of the
	// interactionView in sync with the keyboard's appearance.
	[UIView beginAnimations:nil context:NULL];
    [UIView setAnimationDuration:animationDuration];
    
    interactionView.frame = newInteractionViewFrame;
	playerView.frame = newPlayerViewFrame;
	
    [UIView commitAnimations];
}

- (void)keyboardWillHide:(NSNotification *)notification {
    
	if ( ! keyboardIsShown) {
		return;
	}	
	keyboardIsShown = false;

	[self handlePauseTapped];
	
    NSDictionary* userInfo = [notification userInfo];    
    /*
     Restore the size of the playerView and the position of the InteractionView.
     Animate the resize so that it's in sync with the disappearance of the keyboard.
     */
    NSValue *animationDurationValue = [userInfo objectForKey:UIKeyboardAnimationDurationUserInfoKey];
    NSTimeInterval animationDuration;
    [animationDurationValue getValue:&animationDuration];
    
    [UIView beginAnimations:nil context:NULL];
    [UIView setAnimationDuration:animationDuration];
	interactionView.frame = originalInteractionViewFrame;
	playerView.frame = originalPlayerViewFrame;
    [UIView commitAnimations];
	[self handlePlayTapped];
}	

- (BOOL) isSupportedOrientation: (UIDeviceOrientation) orientation {
	return 
		orientation == UIDeviceOrientationPortrait
	||	orientation == UIDeviceOrientationPortraitUpsideDown
	||	orientation == UIDeviceOrientationLandscapeLeft
	||	orientation == UIDeviceOrientationLandscapeRight;
}

/* */
// Override to allow orientations other than the default portrait orientation.
- (BOOL) shouldAutorotateToInterfaceOrientation: (UIInterfaceOrientation) interfaceOrientation {
	return [self isSupportedOrientation:(UIDeviceOrientation) interfaceOrientation];
}
// react on device rotation
- (void) orientationChanged:(NSNotification *)notification {
//	NSLog(@"orientationChanged");
	UIDeviceOrientation orientation = [[UIDevice currentDevice] orientation];
	if (orientation == currentOrientation || ! [self isSupportedOrientation: orientation]) {
		return;
	}
	currentOrientation = orientation;
	if (keyboardIsShown) {
		[[self URLEntryField] resignFirstResponder]; // dismiss keyboard
	}
	[playerView adaptDisplayAfterRotation: orientation];
}

- (void) close: (NSString*) id {
	//NSLog(@"AmbulantViewController-close: unimplemented");
	[self handleStopTapped];
}

- (void) pause {
	if (myMainloop) {
		myMainloop->pause();
	}
}

- (void)didReceiveMemoryWarning {
	// Releases the view if it doesn't have a superview.
    [super didReceiveMemoryWarning];
	
	// Release any cached data, images, etc that aren't in use.
}

- (void)viewDidUnload {
	// Release any retained subviews of the main view.
	// e.g. self.myOutlet = nil;
}


- (void)dealloc {
    [super dealloc];
	if (myMainloop)
		delete myMainloop;
	[playerView release];
	if (playURL)
		[playURL release];
}

@end