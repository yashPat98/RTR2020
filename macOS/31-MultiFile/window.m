//packages
#import <Foundation/Foundation.h>
#import <Cocoa/Cocoa.h>

//forward declaration of class
@interface AppDelegate:NSObject <NSApplicationDelegate, NSWindowDelegate>

@end

//entry-point function
int main(int argc, char* argv[])
{
    //code
    //create auto release pool
    NSAutoreleasePool *pool = [[NSAutoreleasePool alloc] init];

    //get NSApp handle and set deligate
    NSApp = [NSApplication sharedApplication];
    [NSApp setDelegate:[[AppDelegate alloc] init]];
    
    //run loop or message loop
    [NSApp run];

    //release resources
    [pool release];

    return (0);
}

//forward declaration of class
@interface MyView:NSView

@end

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

//definition of class
@implementation MyView
{
    @private
        NSString *centralText;
}

    -(id)initWithFrame:(NSRect)frame
    {
        //code
        self = [super initWithFrame:frame];
        if(self)
        {
            centralText = @"Hello World !!!";
        }

        return (self);
    }

    -(void)drawRect:(NSRect)dirtyRect
    {
        //code
        NSColor *backgroundColor = [NSColor blackColor];
        [backgroundColor set];
        NSRectFill(dirtyRect);

        //set font attributes
        NSDictionary *dictionaryForTextAttributes = [NSDictionary dictionaryWithObjectsAndKeys:
            [NSFont fontWithName:@"Helvetica" size:32], NSFontAttributeName,
            [NSColor greenColor], NSForegroundColorAttributeName,
            nil
        ];
        NSSize textSize = [centralText sizeWithAttributes:dictionaryForTextAttributes];

        NSPoint point;
        point.x = (dirtyRect.size.width / 2) - (textSize.width / 2) + 12;
        point.y = (dirtyRect.size.height / 2) - (textSize.height / 2) + 12;

        [centralText drawAtPoint:point withAttributes:dictionaryForTextAttributes];
    }

    -(BOOL)acceptsFirstResponder
    {
        //code
        [[self window] makeFirstResponder:self];
        return (YES);
    }

    -(void)keyDown:(NSEvent*)theEvent
    {
        //code
        int key = [[theEvent characters] characterAtIndex:0];
        switch(key)
        {
            case 27:
                [self release];
                [NSApp terminate:self];
                break;
            
            case 'F':
            case 'f':
                [[self window] toggleFullScreen:self];
                break;
        }
    }

    -(void)mouseDown:(NSEvent*)theEvent
    {
        //code
        centralText = @"Left Mouse Button Is Clicked";
        [self setNeedsDisplay:YES];
    }

    -(void)rightMouseDown:(NSEvent*)theEvent
    {
        //code
        centralText = @"Right Mouse Button Is Clicked";
        [self setNeedsDisplay:YES];
    }

    -(void)otherMouseDown:(NSEvent*)theEvent
    {
        //code
        centralText = @"Hello World !!!";
        [self setNeedsDisplay:YES];
    }

    -(void)dealloc
    {
        [super dealloc];
    }

@end
