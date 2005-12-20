//
//  AmbulantWebView.h
//  AmbulantWebKitPlugin
//
//  Created by Jack Jansen on 20-12-05.
//  Copyright 2005 __MyCompanyName__. All rights reserved.
//

#import <Cocoa/Cocoa.h>


@interface AmbulantWebView : NSView {
	NSDictionary *m_arguments;
}

- (void)setArguments:(NSDictionary *)arguments;

- (void)startPlayer;
- (void)stopPlayer;
- (void)testartPlayer;
- (void)pausePlayer;
- (void)resumePlayer;
@end
