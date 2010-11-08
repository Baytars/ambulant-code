//
//  Presentation.m
//  Presentation representation in a tabel cell
//
//  Created by Kees Blom on 4/11/10.
//  Copyright 2010 Stg.CWI. All rights reserved.
//

#import "Presentation.h"

@implementation Presentation
@synthesize title, duration, description, poster, nsurl;

-(id)initWithTitle:(NSString*) newTitle
			poster:(id)newPoster
		  duration:(NSString*) newDuration
	   description:(NSString*) newDescription
			 nsurl:(NSURL*) newUrl
{
	self = [super init];
	if (self != nil) {
		self.title = newTitle;
		self.poster = newPoster;
		self.duration = newDuration;
		self.description = newDescription;
		self.nsurl = newUrl;
	}
	return self;
}

-(void) dealloc {
	self.title = nil;
	self.poster = nil;
	self.duration = nil;
	self.description = nil;
	self.nsurl = nil;
	[super dealloc];
}
@end
