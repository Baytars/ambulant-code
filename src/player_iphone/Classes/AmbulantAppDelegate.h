//
//  AmbulantAppDelegate.h
//  Ambulant
//
//  Created by Kees Blom on 7/12/10.
//  Copyright CWI 2010. All rights reserved.
//

#import <UIKit/UIKit.h>
#import "AmbulantWebViewController.h"

@class AmbulantViewController;

@interface AmbulantAppDelegate : NSObject <UIApplicationDelegate> {
    UIWindow *window;
	IBOutlet UITabBarController* tabBarController;
    AmbulantViewController *viewController;
    AmbulantWebViewController *webViewController;
}
- (void) showAlert: (NSString*) msgtype;
- (void) openWebLink: (NSString*) url;
- (void) showAmbulantPlayer: (void*) id;
- (void) showPlaylists: (void*) id;

@property (nonatomic, retain) IBOutlet UIWindow *window;
@property (nonatomic, retain) IBOutlet UITabBarController* tabBarController;
@property (nonatomic, retain) IBOutlet AmbulantViewController *viewController;
@property (nonatomic, retain) IBOutlet AmbulantWebViewController* webViewController;
@end

