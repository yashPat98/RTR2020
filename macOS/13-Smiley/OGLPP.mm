//packages
#import <Foundation/Foundation.h>
#import <Cocoa/Cocoa.h>
#import <QuartzCore/CVDisplayLink.h>
#import <OpenGL/GL3.h>
#include "vmath.h"

//type declarations
enum
{
    AMC_ATTRIBUTE_POSITION = 0,
    AMC_ATTRIBUTE_COLOR,
    AMC_ATTRIBUTE_NORMAL,
    AMC_ATTRIBUTE_TEXCOORD
};

//global functions
CVReturn MyDisplayLinkCallback(CVDisplayLinkRef, const CVTimeStamp*, const CVTimeStamp*,
                               CVOptionFlags, CVOptionFlags*, void*);

//global variables
FILE *gpFile = NULL;

//shader program variables
GLuint vertexShaderObject;
GLuint fragmentShaderObject;
GLuint shaderProgramObject;

//unioform location variables
GLuint mvpMatrixUniform;
GLuint textureSamplerUniform;

//input data buffer variables
GLuint vao_square;
GLuint vbo_square_position;
GLuint vbo_square_texcoord;

//texture variables 
GLuint smiley_texture;

//variable for uniform values
vmath::mat4 projectionMatrix;

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
@interface OpenGLView:NSOpenGLView

@end

//definition of class
@implementation AppDelegate
{
    @private
        NSWindow *window;
        OpenGLView *gl_view;
}
    
    -(void)applicationDidFinishLaunching:(NSNotification*)aNotification
    {
        //code
        //get path of app folder
        NSBundle *app_bundle = [NSBundle mainBundle];
        NSString *app_dir_path = [app_bundle bundlePath];
        NSString *parent_dir_path = [app_dir_path stringByDeletingLastPathComponent];
        NSString *log_filename_with_path = [NSString stringWithFormat:@"%@/log.txt", parent_dir_path];
        const char *psz_log_filename = [log_filename_with_path cStringUsingEncoding:NSASCIIStringEncoding];

        //open/create log file
        gpFile = fopen(psz_log_filename, "w");
        if(gpFile == NULL)
        {
            [self release];
            [NSApp terminate:self];
        }

        fprintf(gpFile, "-------------------------------------------------------------\n");
        fprintf(gpFile, "-> program started successfully\n");
        fprintf(gpFile, "-------------------------------------------------------------\n");
        
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
        [window setTitle:@"OpenGL : Smiley"];
        [window center];

        //create view
        gl_view = [[OpenGLView alloc] initWithFrame:win_rect];

        //set view
        [window setContentView:gl_view];

        //set window delegate
        [window setDelegate:self];

        //set focus
        [window makeKeyAndOrderFront:gl_view];
    }

    -(void)applicationWillTerminate:(NSNotification*)aNotification
    {
        //code
        if(gpFile)
        {
            fprintf(gpFile, "-------------------------------------------------------------\n");
            fprintf(gpFile, "-> program terminated successfully\n");
            fprintf(gpFile, "-------------------------------------------------------------\n");
            fclose(gpFile);
            gpFile = NULL;
        }
    }

    -(void)windowWillClose:(NSNotification*)aNotification
    {
        //code
        [NSApp terminate:self];
    }

    -(void)dealloc
    {
        //code
        [gl_view release];
        [window release];
        [super dealloc];
    }

@end

//definition of class
@implementation OpenGLView
{
    @private
        CVDisplayLinkRef display_link;
}

    -(id)initWithFrame:(NSRect)frame
    {
        //code
        self = [super initWithFrame:frame];
        if(self)
        {
            NSOpenGLPixelFormatAttribute attributes[] =
            {
                NSOpenGLPFAOpenGLProfile, NSOpenGLProfileVersion4_1Core,
                NSOpenGLPFAScreenMask, CGDisplayIDToOpenGLDisplayMask(kCGDirectMainDisplay),
                NSOpenGLPFANoRecovery,
                NSOpenGLPFAAccelerated,
                NSOpenGLPFAColorSize, 24,
                NSOpenGLPFADepthSize, 24,
                NSOpenGLPFAAlphaSize, 8,
                NSOpenGLPFADoubleBuffer,
                0
            };

            NSOpenGLPixelFormat *pixel_format = [[[NSOpenGLPixelFormat alloc] initWithAttributes:attributes] autorelease];
            if(pixel_format == nil)
            {
                fprintf(gpFile, "Error : failed to get pixel format.\n");
                [self release];
                [NSApp terminate:self];
            }

            NSOpenGLContext *gl_context = [[[NSOpenGLContext alloc] initWithFormat:pixel_format shareContext:nil] autorelease];
            if(gl_context == nil)
            {
                fprintf(gpFile, "Error : failed to get OpenGL context.\n");
                [self release];
                [NSApp terminate:self];
            }

            [self setPixelFormat:pixel_format];
            [self setOpenGLContext:gl_context];
        }

        return (self);
    }

    -(CVReturn)getFrameForTime:(const CVTimeStamp*)outputTime
    {
        //code
        NSAutoreleasePool *pool = [[NSAutoreleasePool alloc] init];
        [self drawView];
        [pool release];

        return (kCVReturnSuccess);
    }

    -(void)prepareOpenGL
    {
        //code
        [super prepareOpenGL];
        [[self openGLContext] makeCurrentContext];
        
        //swap intervals
        GLint swap_interval = 1;
        [[self openGLContext] setValues:&swap_interval forParameter:NSOpenGLCPSwapInterval];

        //log OpenGL info
        fprintf(gpFile, "-> OpenGL Info\n");
        fprintf(gpFile, "   Vendor : %s\n", glGetString(GL_VENDOR));
        fprintf(gpFile, "   Renderer : %s\n", glGetString(GL_RENDERER));
        fprintf(gpFile, "   Version : %s\n", glGetString(GL_VERSION));
        fprintf(gpFile, "   GLSL Version : %s\n", glGetString(GL_SHADING_LANGUAGE_VERSION));
        fprintf(gpFile, "-------------------------------------------------------------\n");
        
        //pass-through shader program
        
        //vertex shader
        vertexShaderObject = glCreateShader(GL_VERTEX_SHADER);
        
        const GLchar *vertexShaderSource =
            "#version 410 core"                                         \
            "\n"                                                        \
            
            "in vec3 a_position;"                                       \
            "in vec2 a_texcoord;"                                       \
            "out vec2 v_texcoord;"                                      \
            "uniform mat4 mvp_matrix;"                                  \
        
            "void main(void)"                                           \
            "{"                                                         \
            "   v_texcoord = a_texcoord;"                               \
            "   gl_Position = mvp_matrix * vec4(a_position, 1.0f);"     \
            "}";
        
        //provide source code to vertex shader object
        glShaderSource(vertexShaderObject, 1, (const GLchar**)&vertexShaderSource, NULL);
        
        //compile shader
        glCompileShader(vertexShaderObject);
        
        //shader compilation error checking
        GLint infoLogLength = 0;
        GLint shaderCompiledStatus = 0;
        GLchar *szInfoLog = NULL;
        
        glGetShaderiv(vertexShaderObject, GL_COMPILE_STATUS, &shaderCompiledStatus);
        if(shaderCompiledStatus == GL_FALSE)
        {
            glGetShaderiv(vertexShaderObject, GL_INFO_LOG_LENGTH, &infoLogLength);
            if(infoLogLength > 0)
            {
                szInfoLog = (GLchar*)malloc(sizeof(GLchar) * infoLogLength);
                if(szInfoLog != NULL)
                {
                    GLsizei written;
                    glGetShaderInfoLog(vertexShaderObject, infoLogLength, &written, szInfoLog);
                    fprintf(gpFile, "-> vertex shader compilation log : %s\n", szInfoLog);
                    fprintf(gpFile, "-------------------------------------------------------------\n");
                    free(szInfoLog);
                    [self release];
                    [NSApp terminate:self];
                }
            }
        }
        fprintf(gpFile, "-> vertex shader compiled successfully\n");
        fprintf(gpFile, "-------------------------------------------------------------\n");
        
        //fragment shader
        fragmentShaderObject = glCreateShader(GL_FRAGMENT_SHADER);
        
        const GLchar *fragmentShaderSource =
            "#version 410 core"                                         \
            "\n"                                                        \
            
            "in vec2 v_texcoord;"                                       \
            "out vec4 frag_color;"                                      \
            "uniform sampler2D texture_sampler;"                        \

            "void main(void)"                                           \
            "{"                                                         \
            "   frag_color = texture(texture_sampler, v_texcoord);"     \
            "}";
        
        //provide source code to fragment shader object
        glShaderSource(fragmentShaderObject, 1, (const GLchar**)&fragmentShaderSource, NULL);
        
        //compile shader
        glCompileShader(fragmentShaderObject);
        
        //shader compilation error checking
        glGetShaderiv(fragmentShaderObject, GL_COMPILE_STATUS, &shaderCompiledStatus);
        if(shaderCompiledStatus == GL_FALSE)
        {
            glGetShaderiv(fragmentShaderObject, GL_INFO_LOG_LENGTH, &infoLogLength);
            if(infoLogLength > 0)
            {
                szInfoLog = (GLchar*)malloc(sizeof(GLchar) * infoLogLength);
                if(szInfoLog != NULL)
                {
                    GLsizei written;
                    glGetShaderInfoLog(fragmentShaderObject, infoLogLength, &written, szInfoLog);
                    fprintf(gpFile, "-> fragment shader compilation log : %s\n", szInfoLog);
                    fprintf(gpFile, "-------------------------------------------------------------\n");
                    free(szInfoLog);
                    [self release];
                    [NSApp terminate:self];
                }
            }
        }
        fprintf(gpFile, "-> fragment shader compiled successfully\n");
        fprintf(gpFile, "-------------------------------------------------------------\n");
        
        //shader program
        shaderProgramObject = glCreateProgram();
        
        //attach shaders to program object
        glAttachShader(shaderProgramObject, vertexShaderObject);
        glAttachShader(shaderProgramObject, fragmentShaderObject);
        
        //bind shader program object with vertex shader attributes
        glBindAttribLocation(shaderProgramObject, AMC_ATTRIBUTE_POSITION, "a_position");
        glBindAttribLocation(shaderProgramObject, AMC_ATTRIBUTE_TEXCOORD, "a_texcoord");

        //link program
        glLinkProgram(shaderProgramObject);
        
        //shader linking error chechking
        GLint shaderProgramLinkStatus = 0;
        glGetProgramiv(shaderProgramObject, GL_LINK_STATUS, &shaderProgramLinkStatus);
        if(shaderProgramLinkStatus == GL_FALSE)
        {
            glGetProgramiv(shaderProgramObject, GL_INFO_LOG_LENGTH, &infoLogLength);
            if(infoLogLength > 0)
            {
                szInfoLog = (GLchar*)malloc(sizeof(GLchar) * infoLogLength);
                if(szInfoLog != NULL)
                {
                    GLsizei written;
                    glGetProgramInfoLog(shaderProgramObject, infoLogLength, &written, szInfoLog);
                    fprintf(gpFile, "-> shader program link log : %s\n", szInfoLog);
                    free(szInfoLog);
                    [self release];
                    [NSApp terminate:self];
                }
            }
        }
        fprintf(gpFile, "-> shader program linked successfully\n");
        fprintf(gpFile, "-------------------------------------------------------------\n");
            
        //get uniform locations
        mvpMatrixUniform = glGetUniformLocation(shaderProgramObject, "mvp_matrix");
        textureSamplerUniform = glGetUniformLocation(shaderProgramObject, "texture_sampler");

        //set up input data
        const GLfloat squareVertices[] =
        {
            1.0f, 1.0f, 0.0f,
            -1.0f, 1.0f, 0.0f,
            -1.0f, -1.0f, 0.0f,
            1.0f, -1.0f, 0.0f
        };

        const GLfloat squareTexcoords[] = 
        {
            1.0f, 0.0f,
            0.0f, 0.0f,
            0.0f, 1.0f,
            1.0f, 1.0f
        };

        //set up vao and vbo
        glGenVertexArrays(1, &vao_square);
        glBindVertexArray(vao_square);
            glGenBuffers(1, &vbo_square_position);
            glBindBuffer(GL_ARRAY_BUFFER, vbo_square_position);
                glBufferData(GL_ARRAY_BUFFER, sizeof(squareVertices), squareVertices, GL_STATIC_DRAW);
                glVertexAttribPointer(AMC_ATTRIBUTE_POSITION, 3, GL_FLOAT, GL_FALSE, 0, NULL);
                glEnableVertexAttribArray(AMC_ATTRIBUTE_POSITION);
            glBindBuffer(GL_ARRAY_BUFFER, 0);

            glGenBuffers(1, &vbo_square_texcoord);
            glBindBuffer(GL_ARRAY_BUFFER, vbo_square_texcoord);
                glBufferData(GL_ARRAY_BUFFER, sizeof(squareTexcoords), squareTexcoords, GL_STATIC_DRAW);
                glVertexAttribPointer(AMC_ATTRIBUTE_TEXCOORD, 2, GL_FLOAT, GL_FALSE, 0, NULL);
                glEnableVertexAttribArray(AMC_ATTRIBUTE_TEXCOORD);
            glBindBuffer(GL_ARRAY_BUFFER, 0);
        glBindVertexArray(0);
        
        //set OpenGL states
        glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
        glClearDepth(1.0f);                                     
        glEnable(GL_DEPTH_TEST);                                
        glDepthFunc(GL_LEQUAL);
        glEnable(GL_CULL_FACE);
        glEnable(GL_TEXTURE_2D);

        //laod texture
        smiley_texture = [self loadTextureFromBMPFile:"smiley.bmp"];
        if(!smiley_texture)
        {
            fprintf(gpFile, "Error : failed to load smiley.bmp texture.\n");
            [self release];
            [NSApp terminate:self];
        }

        //bind cgl and cv
        CVDisplayLinkCreateWithActiveCGDisplays(&display_link);
        CVDisplayLinkSetOutputCallback(display_link, &MyDisplayLinkCallback, self);
        CGLContextObj cgl_context = (CGLContextObj)[[self openGLContext] CGLContextObj];
        CGLPixelFormatObj cgl_pixelformat = (CGLPixelFormatObj)[[self pixelFormat] CGLPixelFormatObj];
        CVDisplayLinkSetCurrentCGDisplayFromOpenGLContext(display_link, cgl_context, cgl_pixelformat);
        CVDisplayLinkStart(display_link);
    }

    -(GLuint)loadTextureFromBMPFile:(const char *)imageFilename
    {
        //variable declarations
        void *pixels = NULL;
        int imageWidth;
        int imageHeight;
        GLuint texture;

        //code
        //get image path
        NSBundle *app_bundle = [NSBundle mainBundle];
        NSString *app_dir_path = [app_bundle bundlePath];
        NSString *parent_dir_path = [app_dir_path stringByDeletingLastPathComponent];
        NSString *image_filename_with_path = [NSString stringWithFormat:@"%@/%s", parent_dir_path, imageFilename];

        //load bmp into NSImage
        NSImage *bmpImage = [[NSImage alloc] initWithContentsOfFile:image_filename_with_path];
        if(!bmpImage)    
            return (0);

        //get image data
        CGImageRef cgImage = [bmpImage CGImageForProposedRect:nil context:nil hints:nil];    
        CFDataRef imageData = CGDataProviderCopyData(CGImageGetDataProvider(cgImage));
        pixels = (void*)CFDataGetBytePtr(imageData);
        imageWidth = (int)CGImageGetWidth(cgImage);
        imageHeight = (int)CGImageGetHeight(cgImage);

        //set up texture object
        glPixelStorei(GL_UNPACK_ALIGNMENT, 1);
        glGenTextures(1, &texture);
        glBindTexture(GL_TEXTURE_2D, texture);

        //set up texture parameters
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
    
        //push the data to texture memory
        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, imageWidth, imageHeight, 0, GL_RGBA, GL_UNSIGNED_BYTE, pixels);
        glGenerateMipmap(GL_TEXTURE_2D);

        //free image data
        CFRelease(imageData);
        pixels = NULL;

        return (texture);
    }

    -(void)reshape
    {
        //code
        [super reshape];
        CGLLockContext((CGLContextObj)[[self openGLContext] CGLContextObj]);

        NSRect rect = [self bounds];
        
        if(rect.size.height < 0)
            rect.size.height = 1;

        glViewport(0, 0, (GLsizei)rect.size.width, (GLsizei)rect.size.height);
        
        projectionMatrix = vmath::perspective(45.0f, rect.size.width / rect.size.height, 0.1f, 100.0f);
        
        CGLUnlockContext((CGLContextObj)[[self openGLContext] CGLContextObj]);
    }

    -(void)drawRect:(NSRect)dirtyRect
    {
        //code
        [self drawView];
    }

    -(void)drawView
    {
        //variable declarations
        vmath::mat4 modelViewMatrix;
        vmath::mat4 mvpMatrix;
        
        //code
        [[self openGLContext] makeCurrentContext];
        CGLLockContext((CGLContextObj)[[self openGLContext] CGLContextObj]);
        
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
        
        modelViewMatrix = vmath::mat4::identity();
        mvpMatrix = vmath::mat4::identity();

        glUseProgram(shaderProgramObject);
            modelViewMatrix = vmath::translate(0.0f, 0.0f, -3.0f);
            mvpMatrix  = projectionMatrix * modelViewMatrix;

            glUniformMatrix4fv(mvpMatrixUniform, 1, GL_FALSE, mvpMatrix);

            glActiveTexture(GL_TEXTURE0);
            glBindTexture(GL_TEXTURE_2D, smiley_texture);
            glUniform1i(textureSamplerUniform, 0);

            glBindVertexArray(vao_square);
            glDrawArrays(GL_TRIANGLE_FAN, 0, 4);
            glBindVertexArray(0);
        glUseProgram(0);
        
        CGLFlushDrawable((CGLContextObj)[[self openGLContext] CGLContextObj]);
        CGLUnlockContext((CGLContextObj)[[self openGLContext] CGLContextObj]);
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
    }

    -(void)rightMouseDown:(NSEvent*)theEvent
    {
        //code
    }

    -(void)otherMouseDown:(NSEvent*)theEvent
    {
        //code
    }

    -(void)dealloc
    {
        //code
        //release textures
        if(smiley_texture)
        {
            glDeleteTextures(1, &smiley_texture);
            smiley_texture = 0;
        }

        //release vao and vbo
        if(vbo_square_texcoord)
        {
            glDeleteBuffers(1, &vbo_square_texcoord);
            vbo_square_texcoord = 0;
        }

        if(vbo_square_position)
        {
            glDeleteBuffers(1, &vbo_square_position);
            vbo_square_position = 0;
        }
    
        if(vao_square)
        {
            glDeleteVertexArrays(1, &vao_square);
            vao_square = 0;
        }
        
        //safe shader cleanup
        if(shaderProgramObject)
        {
            GLsizei shader_count;
            GLuint* p_shaders = NULL;

            glUseProgram(shaderProgramObject);
            glGetProgramiv(shaderProgramObject, GL_ATTACHED_SHADERS, &shader_count);

            p_shaders = (GLuint*)malloc(shader_count * sizeof(GLuint));
            memset((void*)p_shaders, 0, shader_count * sizeof(GLuint));

            glGetAttachedShaders(shaderProgramObject, shader_count, &shader_count, p_shaders);

            for(GLsizei i = 0; i < shader_count; i++)
            {
                glDetachShader(shaderProgramObject, p_shaders[i]);
                glDeleteShader(p_shaders[i]);
                p_shaders[i] = 0;
            }

            free(p_shaders);
            p_shaders = NULL;
        
            glDeleteProgram(shaderProgramObject);
            shaderProgramObject = 0;
            glUseProgram(0);
        }
        
        CVDisplayLinkStop(display_link);
        CVDisplayLinkRelease(display_link);
        [super dealloc];
    }

@end

CVReturn MyDisplayLinkCallback(CVDisplayLinkRef displayLink, const CVTimeStamp *now, const CVTimeStamp *outputTime,
                               CVOptionFlags flagsIn, CVOptionFlags *flagsOut, void *displayLinkContext)
{
    //code
    CVReturn result = [(OpenGLView*)displayLinkContext getFrameForTime:outputTime];
    return (result);
}

