    //
//  AmbulantWebViewController.mm
//  player_iphone
//
//  Created by Kees Blom on 8/7/10.
//  Copyright 2010 CWI. All rights reserved.
//

#import "AmbulantWebViewController.h"


@implementation AmbulantWebViewController

/*
 // The designated initializer.  Override if you create the controller programmatically and want to perform customization that is not appropriate for viewDidLoad.
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
-(void) loadURL {
	NSAutoreleasePool *pool = [[NSAutoreleasePool alloc] init];
	NSString* urlstring;

	NSURL* url;
	if (urlField == nil) {
		NSBundle* thisBundle = [NSBundle bundleForClass:[self class]];
		urlstring = [thisBundle pathForResource:@"AmbulantHelp-iOS" ofType:@"html"];
//		urlstring = [@"file://" stringByAppendingString:urlstring];
		url = [[NSURL alloc] initFileURLWithPath: urlstring];
	} else {
		urlstring = [NSString stringWithString: urlField];
		url = [[NSURL alloc] initWithString: urlstring];
	}
	NSURLRequest *request = [[NSURLRequest alloc] initWithURL: url];
	[webView loadRequest: request]; 
//	[request release];
//	[url release];
//	[urlstring release];
	[pool release];
}
/* */
// Implement viewDidLoad to do additional setup after loading the view, typically from a nib.
- (void)viewDidLoad {
	NSLog(@"AmbulantWebViewController: viewDidLoad");
    [super viewDidLoad];
	webView.scalesPageToFit = YES;
	[self loadURL];
}

- (IBAction) handleBackTapped {
	if (webView.canGoBack) {
		[webView goBack];
	} else {
		[delegate auxViewControllerDidFinish:self];
	}
}

- (IBAction) handleDoneTapped {
	[delegate auxViewControllerDidFinish: self];
//	[self dismissModalViewControllerAnimated:YES];
}

// Override to allow orientations other than the default portrait orientation.
- (BOOL)shouldAutorotateToInterfaceOrientation:(UIInterfaceOrientation)interfaceOrientation {
    // Return YES for supported orientations
    return [delegate canShowRotatedUIViews];
}

- (void)didReceiveMemoryWarning {
    // Releases the view if it doesn't have a superview.
    [super didReceiveMemoryWarning];
    
    // Release any cached data, images, etc that aren't in use.
}

- (void)viewDidUnload {
    [super viewDidUnload];
    // Release any retained subviews of the main view.
    // e.g. self.myOutlet = nil;
}


- (void)dealloc {
    [super dealloc];
}


@end