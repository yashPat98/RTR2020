//packages
#import <Foundation/Foundation.h>
#import <Cocoa/Cocoa.h>

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
