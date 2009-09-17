//
//  AmbulantIphoneView.m
//  AmbulantIphone
//
//  Created by Kees Blom on 9/17/09.
//  Copyright 2009 __MyCompanyName__. All rights reserved.
//

#import "AmbulantIphoneView.h"


@implementation AmbulantIphoneView


- (id)initWithFrame:(CGRect)frame {
    if (self = [super initWithFrame:frame]) {
        // Initialization code
    }
    return self;
}


- (void)drawRect:(CGRect)rect {
    // Drawing code
	[[UIColor yellowColor] setFill];
	UIRectFill(rect);
	[[UIColor blackColor] setFill];
	[@"Welcome.smil" drawInRect:rect withFont:[UIFont systemFontOfSize:34]];
}


- (void)dealloc {
    [super dealloc];
}


@end
