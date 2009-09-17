//
//  AmbulantIphoneAppDelegate.h
//  AmbulantIphone
//
//  Created by Kees Blom on 9/17/09.
//  Copyright __MyCompanyName__ 2009. All rights reserved.
//

#import <UIKit/UIKit.h>

@class AmbulantIphoneViewController;

@interface AmbulantIphoneAppDelegate : NSObject <UIApplicationDelegate> {
    UIWindow *window;
    AmbulantIphoneViewController *viewController;
}

@property (nonatomic, retain) IBOutlet UIWindow *window;
@property (nonatomic, retain) IBOutlet AmbulantIphoneViewController *viewController;

@end

