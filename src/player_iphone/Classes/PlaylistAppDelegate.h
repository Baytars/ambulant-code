//
//  PlaylistAppDelegate.h
//  Playlist
//
//  Created by Kees Blom on 7/31/10.
//  Copyright CWI 2010. All rights reserved.
//

#import <UIKit/UIKit.h>

@class PresentationViewController;

@protocol PlaylistViewControllerDelegate
@property(readonly) BOOL autoCenter;
@property(readonly) BOOL autoResize;
@property(readonly) BOOL nativeRenderer;
- (void) auxViewControllerDidFinish: (UIViewController *)controller;
- (void) playPresentation:(NSString*) what fromPresentationViewController:(PresentationViewController*) controller;
- (void) auxViewControllerDidFinish: (PresentationViewController*) controller;
- (void) setHistoryViewController: (PresentationViewController*) controller;
- (void) settingsHaveChanged: (UIViewController*) controller;
- (void) showAmbulantPlayer: (id) sender;
- (void) showPresentationViews: (id) sender;
@end
