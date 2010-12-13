//
//  SettingsViewController.h
//  Settings
//
//  Created by Kees Blom on 7/31/10.
//  Copyright CWI 2010. All rights reserved.
//

#import <UIKit/UIKit.h>
#import "PlaylistAppDelegate.h"

@interface SettingsViewController : UIViewController {
	id <PlaylistViewControllerDelegate> delegate;
	UISwitch* autoCenterSwitch;	
	UISwitch* autoRsizeSwitch;	
	UISwitch* nativeRendererSwitch;	
}

- (IBAction) done: (id) sender;
- (BOOL) autoCenter;
- (BOOL) autoResize;
- (BOOL) nativeRenderer;
- (IBAction) playWelcome;
- (IBAction) playNYC;
- (IBAction) playPanZoom;
- (IBAction) playVideoTests;
- (IBAction) playBirthday;
- (IBAction) playBirthdayRTSP;
- (IBAction) playEuros;
- (IBAction) playEurosRTSP;
- (IBAction) playFlashlight;
- (IBAction) playFlashlightRTSP;
- (IBAction) playNews;
- (IBAction) playNewsRTSP;

@property(nonatomic,assign) id <PlaylistViewControllerDelegate> delegate;
@property(nonatomic,retain) IBOutlet UISwitch* autoCenterSwitch;
@property(nonatomic,retain) IBOutlet UISwitch* autoResizeSwitch;
@property(nonatomic,retain) IBOutlet UISwitch* nativeRendererSwitch;

@end
