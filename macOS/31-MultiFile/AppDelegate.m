//packages
#import <Foundation/Foundation.h>
#import <Cocoa/Cocoa.h>
#import "MyView.h"

//definition of class
@implementation AppDelegate
{
    @private
        NSWindow *window;
        MyView *view;
}
    
    -(void)applicationDidFinishLaunching:(NSNotification*)aNotification
    {
        //code
        //window size
        NSRect win_rect = NSMakeRect(0.0, 0.0, 800.0, 600.0);
        
        //create window
        window = [[NSWindow alloc] initWithContentRect:win_rect
            styleMask:NSWindowStyleMaskTitled |
                      NSWindowStyleMaskClosable |
                      NSWindowStyleMaskMiniaturizable |
                      NSWindowStyleMaskResizable
            backing:NSBackingStoreBuffered
            defer:NO
        ];

        //set window properties
        [window setTitle:@"YIP : macOS window"];
        [window center];

        //create view
        view = [[MyView alloc] initWithFrame:win_rect];

        //set view
        [window setContentView:view];

        //set window delegate
        [window setDelegate:self];

        //set focus
        [window makeKeyAndOrderFront:view];
    }

    -(void)applicationWillTerminate:(NSNotification*)aNotification
    {
        //code
    }

    -(void)windowWillClose:(NSNotification*)aNotification
    {
        //code
        [NSApp terminate:self];
    }

    -(void)dealloc
    {
        //code
        [view release];
        [window release];
        [super dealloc];
    }

@end
