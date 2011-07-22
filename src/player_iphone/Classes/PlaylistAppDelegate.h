/*
 * This file is part of Ambulant Player, www.ambulantplayer.org.
 *
 * Copyright (C) 2003-2011 Stichting CWI, 
 * Science Park 123, 1098 XG Amsterdam, The Netherlands.
 *
 * Ambulant Player is free software; you can redistribute it and/or modify
 * it under the terms of the GNU Lesser General Public License as published by
 * the Free Software Foundation; either version 2.1 of the License, or
 * (at your option) any later version.
 *
 * Ambulant Player is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public License
 * along with Ambulant Player; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA  02110-1301  USA
 */

#import <UIKit/UIKit.h>

@class PresentationViewController;
@class PlaylistItem;

@protocol PlaylistViewControllerDelegate
@property(readonly) BOOL autoCenter;
@property(readonly) BOOL autoResize;
@property(readonly) BOOL nativeRenderer;
- (void) auxViewControllerDidFinish: (UIViewController *)controller;
- (void) playURL: (NSString*) whatString;
- (void) playPresentation: (PlaylistItem*) item fromPresentationViewController: (PresentationViewController*) controller;
- (void) auxViewControllerDidFinish: (PresentationViewController*) controller;
- (void) setHistoryViewController: (PresentationViewController*) controller;
- (void) settingsHaveChanged: (UIViewController*) controller;
- (void) showAmbulantPlayer: (id) sender;
- (void) showPresentationViews: (id) sender;
- (BOOL) canShowRotatedUIViews;
- (void) playWelcome: (UIViewController*) controller;
@end
