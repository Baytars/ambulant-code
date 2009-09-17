//
//  AmbulantIphoneAppDelegate.m
//  AmbulantIphone
//
//  Created by Kees Blom on 9/17/09.
//  Copyright __MyCompanyName__ 2009. All rights reserved.
//

#import "AmbulantIphoneAppDelegate.h"
#import "AmbulantIphoneViewController.h"

@implementation AmbulantIphoneAppDelegate

@synthesize window;
@synthesize viewController;


- (void) initialize_ambulant {
	
	// Install our preferences handler
	//TBD mypreferences::install_singleton();
	
	// Install our logger
	//TBD if (initialize_logger() == 0 && getenv("AMBULANT_LOGGER_NOWINDOW") == NULL) {
	// Show the logger window immedeately if log level is DEBUG
	//TBD	[self showLogWindow: self];
	//TBD}
	char* welcome_path = "/Users/kees/Projects/Ambulant/ambulant/Extras/Welcome/Welcome.smil";
	printf("initializing with %s\n", welcome_path);
}


- (void)applicationDidFinishLaunching:(UIApplication *)application {    
    
    // Override point for customization after app launch    
	[self initialize_ambulant ];
    [window addSubview:viewController.view];
    [window makeKeyAndVisible];
}


- (void)dealloc {
    [viewController release];
    [window release];
    [super dealloc];
}
@end
