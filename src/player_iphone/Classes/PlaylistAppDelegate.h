//
//  PlaylistAppDelegate.h
//  Playlist
//
//  Created by Kees Blom on 7/31/10.
//  Copyright CWI 2010. All rights reserved.
//

#import <UIKit/UIKit.h>

@class AmbulantViewController;
@class PresentationViewController;

@interface PlaylistAppDelegate : NSObject <UIApplicationDelegate> {
    UIWindow *window;
    AmbulantViewController *ambulantViewController;
}

@property (nonatomic, retain) IBOutlet UIWindow *window;
@property (nonatomic, retain) IBOutlet AmbulantViewController *ambulantViewController;

@end

@protocol PlaylistViewControllerDelegate
- (void) settingsHaveChanged: (UIViewController*) controller;
- (void) playlistViewControllerDidFinish: (UIViewController *)controller;
- (void) playPresentation:(NSString*) what;
- (NSString*) playURL;
- (IBAction) done: (id) sender;
- (void) setHistoryViewController: (PresentationViewController*) controller;
@end
