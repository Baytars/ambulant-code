//
//  PlaylistAppDelegate.h
//  Playlist
//
//  Created by Kees Blom on 7/31/10.
//  Copyright __MyCompanyName__ 2010. All rights reserved.
//

#import <UIKit/UIKit.h>

@class AmbulantViewController;

@interface PlaylistAppDelegate : NSObject <UIApplicationDelegate> {
    UIWindow *window;
    AmbulantViewController *ambulantViewController;
}

@property (nonatomic, retain) IBOutlet UIWindow *window;
@property (nonatomic, retain) IBOutlet AmbulantViewController *ambulantViewController;

@end

