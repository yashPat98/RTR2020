//
//  MyView.m
//  window
//
//  Created by Akshay Patel on 9/24/21.
//

#import <OpenGLES/ES3/gl.h>
#import <OpenGLES/ES3/glext.h>
#import "GLESView.h"

@implementation GLESView
{
    EAGLContext *eagl_context;
    
    GLuint default_framebuffer;
    GLuint color_renderbuffer;
    GLuint depth_renderbuffer;

    id display_link;
    NSInteger animation_frame_interval;
    bool is_animating;
}

/*
- (void)drawRect:(CGRect)rect
{
    //code
}
*/

- (void)startAnimation
{
    //code
    if(is_animating == NO)
    {
        display_link = [NSClassFromString(@"CADisplayLink") displayLinkWithTarget:self selector:@selector(drawView)];
        [display_link setPreferredFramesPerSecond:animation_frame_interval];
        [display_link addToRunLoop:[NSRunLoop currentRunLoop] forMode:NSDefaultRunLoopMode];
        
        is_animating = YES;
    }
}

- (void)stopAnimation
{
    //code
    if(is_animating == YES)
    {
        [display_link invalidate];
        display_link = nil;
        
        is_animating = NO;
    }
}

- (id)initWithFrame:(CGRect)frame
{
    //code
    [super initWithFrame:frame];
    
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
    
    CAEAGLLayer *eagl_layer;
    eagl_layer = (CAEAGLLayer*)[super layer];
    [eagl_layer setOpaque:YES];
    [eagl_layer setDrawableProperties:[NSDictionary dictionaryWithObjectsAndKeys:
                                       [NSNumber numberWithBool:NO],
                                       kEAGLDrawablePropertyRetainedBacking,
                                       kEAGLColorFormatRGBA8,
                                       kEAGLDrawablePropertyColorFormat,
                                       nil]];
    
    eagl_context = [[EAGLContext alloc] initWithAPI:kEAGLRenderingAPIOpenGLES3];
    if(eagl_context == nil)
    {
        printf("Failed to create OpenGL-ES context.\n");
        return (nil);
    }
    
    [EAGLContext setCurrentContext:eagl_context];
    
    glGenFramebuffers(1, &default_framebuffer);
    glBindFramebuffer(GL_FRAMEBUFFER, default_framebuffer);
        GLint backing_width;
        GLint backing_height;

        //color attachment
        glGenRenderbuffers(1, &color_renderbuffer);
        glBindRenderbuffer(GL_RENDERBUFFER, color_renderbuffer);

        [eagl_context renderbufferStorage:GL_RENDERBUFFER fromDrawable:eagl_layer];

            glGetRenderbufferParameteriv(GL_RENDERBUFFER, GL_RENDERBUFFER_WIDTH, &backing_width);
            glGetRenderbufferParameteriv(GL_RENDERBUFFER, GL_RENDERBUFFER_HEIGHT, &backing_height);
        glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_RENDERBUFFER, color_renderbuffer);
        glBindRenderbuffer(GL_RENDERBUFFER, 0);

        //depth attachment
        glGenRenderbuffers(1, &depth_renderbuffer);
        glBindRenderbuffer(GL_RENDERBUFFER, depth_renderbuffer);
            glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH_COMPONENT16, backing_width, backing_height);
        glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_RENDERBUFFER, depth_renderbuffer);
        glBindRenderbuffer(GL_RENDERBUFFER, 0);

        if(glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE)
        {
            printf("Framebuffer is incomplete\n");
            [self uninitialize];
            return (nil);
        }
    glBindFramebuffer(GL_FRAMEBUFFER, 0);
    
    animation_frame_interval = 60;
    is_animating = NO;
    
    glClearColor(0.0, 0.0, 1.0f, 1.0f);
    
    return (self);
}

+ (Class)layerClass
{
    //code
    return ([CAEAGLLayer class]);
}

- (void)layoutSubviews
{
    //variable declarations
    GLint width;
    GLint height;

    //code
    glBindFramebuffer(GL_FRAMEBUFFER, default_framebuffer);
        //color attachment
        glBindRenderbuffer(GL_RENDERBUFFER, color_renderbuffer);
        [eagl_context renderbufferStorage:color_renderbuffer fromDrawable:(CAEAGLLayer*)[self layer]];

            glGetRenderbufferParameteriv(GL_RENDERBUFFER, GL_RENDERBUFFER_WIDTH, &width);
            glGetRenderbufferParameteriv(GL_RENDERBUFFER, GL_RENDERBUFFER_HEIGHT, &height);
        glBindRenderbuffer(GL_RENDERBUFFER, 0);

        //depth attachment
        glBindRenderbuffer(GL_RENDERBUFFER, depth_renderbuffer);
            glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH_COMPONENT16, width, height);
        glBindRenderbuffer(GL_RENDERBUFFER, 0);

        if(glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE)
        {
            printf("Framebuffer is incomplete\n");
        }
    glBindFramebuffer(GL_FRAMEBUFFER, 0);

    if(height < 0)
        height = 1;

    glViewport(0, 0, (GLsizei)width, (GLsizei)height);

    [self drawView];
}

- (void)drawView
{
    //code
    [EAGLContext setCurrentContext:eagl_context];
    glBindFramebuffer(GL_FRAMEBUFFER, default_framebuffer);
    glBindRenderbuffer(GL_RENDERBUFFER, color_renderbuffer);

    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    
    [eagl_context presentRenderbuffer:color_renderbuffer];
    
    glBindRenderbuffer(GL_RENDERBUFFER, 0);
    glBindFramebuffer(GL_FRAMEBUFFER, 0);
}

- (void) uninitialize
{
    //code
    if(depth_renderbuffer)
    {
        glDeleteRenderbuffers(1, &depth_renderbuffer);
        depth_renderbuffer = 0;
    }

    if(color_renderbuffer)
    {
        glDeleteRenderbuffers(1, &color_renderbuffer);
        color_renderbuffer = 0;
    }

    if(default_framebuffer)
    {
        glDeleteFramebuffers(1, &default_framebuffer);
        default_framebuffer = 0;
    }

    if(eagl_context)
    {
        if([EAGLContext currentContext] == eagl_context)
        {
            [EAGLContext setCurrentContext:nil];
            [eagl_context release];
            eagl_context = nil;
        }
    }
}

- (void)onSingleTap:(UITapGestureRecognizer*)gr
{
    //code
    [self setNeedsDisplay];
}

- (void)onDoubleTap:(UITapGestureRecognizer*)gr
{
    //code
    [self setNeedsDisplay];
}

- (void)onSwipe:(UISwipeGestureRecognizer*)gr
{
    //code
    [self release];
    exit(0);
}

- (void) onLongPress:(UILongPressGestureRecognizer*)gr
{
    //code
    [self setNeedsDisplay];
}

- (void)dealloc
{
    //code
    [super dealloc];
    [self uninitialize];
    [self release];
}

@end

