//
//  MyView.m
//  window
//
//  Created by Akshay Patel on 9/24/21.
//

#import "MyView.h"

@implementation MyView
{
    NSString *central_text;
}

- (void)drawRect:(CGRect)rect
{
    //code
    UIColor *background_color = [UIColor blackColor];
    [background_color set];
    
    UIRectFill(rect);
    
    NSDictionary *dictionary_for_text_attributes = [NSDictionary dictionaryWithObjectsAndKeys:
                                                    [UIFont fontWithName:@"Helvetica" size:32],
                                                    NSFontAttributeName,
                                                    [UIColor greenColor],
                                                    NSForegroundColorAttributeName,
                                                    nil];
    
    CGSize text_size = [central_text sizeWithAttributes:dictionary_for_text_attributes];
    CGPoint point;
    point.x = rect.size.width / 2 - text_size.width / 2;
    point.y = rect.size.height / 2 - text_size.height / 2;
                         
    [central_text drawAtPoint:point withAttributes:dictionary_for_text_attributes];
    
    UITapGestureRecognizer *single_tap_gesture_recognizer = [[UITapGestureRecognizer alloc] initWithTarget:self action:@selector(onSingleTap:)];
    
    [single_tap_gesture_recognizer setNumberOfTapsRequired:1];
    [single_tap_gesture_recognizer setNumberOfTouchesRequired:1];
    [single_tap_gesture_recognizer setDelegate:self];
    
    [self addGestureRecognizer:single_tap_gesture_recognizer];
    
    UITapGestureRecognizer *double_tap_gesture_recognizer = [[UITapGestureRecognizer alloc] initWithTarget:self action:@selector(onDoubleTap:)];
    
    [double_tap_gesture_recognizer setNumberOfTapsRequired:2];
    [double_tap_gesture_recognizer setNumberOfTouchesRequired:1];
    [double_tap_gesture_recognizer setDelegate:self];
    [single_tap_gesture_recognizer requireGestureRecognizerToFail:double_tap_gesture_recognizer];
    
    [self addGestureRecognizer:double_tap_gesture_recognizer];
    
    UISwipeGestureRecognizer *swipe_gesture_recognizer = [[UISwipeGestureRecognizer alloc] initWithTarget:self action:@selector(onSwipe:)];
    
    [swipe_gesture_recognizer setDelegate:self];
    
    [self addGestureRecognizer:swipe_gesture_recognizer];
    
    UILongPressGestureRecognizer *long_press_gesture_recognizer = [[UILongPressGestureRecognizer alloc] initWithTarget:self action:@selector(onLongPress:)];
    
    [long_press_gesture_recognizer setDelegate:self];
    
    [self addGestureRecognizer:long_press_gesture_recognizer];
}

- (id)initWithFrame:(CGRect)frame
{
    //code
    [super initWithFrame:frame];
    if(self)
    {
        central_text = @"Hello World !!!";
    }
    
    return (self);
}

- (void)onSingleTap:(UITapGestureRecognizer*)gr
{
    //code
    central_text = @"Single Tap";
    [self setNeedsDisplay];
}

- (void)onDoubleTap:(UITapGestureRecognizer*)gr
{
    //code
    central_text = @"Double Tap";
    [self setNeedsDisplay];
}

- (void)onSwipe:(UISwipeGestureRecognizer*)gr
{
    //code
    central_text = @"Swipe";
    [self setNeedsDisplay];
}

- (void) onLongPress:(UILongPressGestureRecognizer*)gr
{
    //code
    central_text = @"Long Press";
    [self setNeedsDisplay];
}

- (void)dealloc
{
    //code
    [super dealloc];
}

@end

