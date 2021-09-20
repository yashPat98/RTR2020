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
GLuint tessellationControlShaderObject; 
GLuint tessellationEvaluationShaderObject;      
GLuint fragmentShaderObject;
GLuint shaderProgramObject;

//unioform location variables
GLuint mvpMatrixUniform;                 
GLuint numberOfSegmentsUniform;
GLuint numberOfStripsUniform;
GLuint lineColorUniform;

//input data buffer variables
GLuint vao;
GLuint vbo_position;

//variable for uniform values
vmath::mat4 projectionMatrix;
unsigned int uiNumberOfSegments;

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
        [window setTitle:@"OpenGL : Tessellation Shader"];
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
            "#version 450 core"                                         \
            "\n"                                                        \
            "in vec2 vPosition;"                                        \
            "void main(void)"                                           \
            "{"                                                         \
            "   gl_Position = vec4(vPosition, 0.0f, 1.0f);"             \
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
        
        //tessellation control shader
        tessellationControlShaderObject = glCreateShader(GL_TESS_CONTROL_SHADER);
        
        const GLchar *tessellationControlShaderSource =
            "#version 450 core"                                                                 \
            "\n"                                                                                \
            
            "layout(vertices = 4)out;"                                                          \
            "uniform int u_numberOfSegments;"                                                   \
            "uniform int u_numberOfStrips;"                                                     \

            "void main(void)"                                                                   \
            "{"                                                                                 \
            "   gl_out[gl_InvocationID].gl_Position = gl_in[gl_InvocationID].gl_Position;"      \
            "   gl_TessLevelOuter[0] = float(u_numberOfStrips);"                                \
            "   gl_TessLevelOuter[1] = float(u_numberOfSegments);"                              \
            "}";
        
        //provide source code to fragment shader object
        glShaderSource(tessellationControlShaderObject, 1, (const GLchar**)&tessellationControlShaderSource, NULL);
        
        //compile shader
        glCompileShader(tessellationControlShaderObject);
        
        //shader compilation error checking
        glGetShaderiv(tessellationControlShaderObject, GL_COMPILE_STATUS, &shaderCompiledStatus);
        if(shaderCompiledStatus == GL_FALSE)
        {
            glGetShaderiv(tessellationControlShaderObject, GL_INFO_LOG_LENGTH, &infoLogLength);
            if(infoLogLength > 0)
            {
                szInfoLog = (GLchar*)malloc(sizeof(GLchar) * infoLogLength);
                if(szInfoLog != NULL)
                {
                    GLsizei written;
                    glGetShaderInfoLog(tessellationControlShaderObject, infoLogLength, &written, szInfoLog);
                    fprintf(gpFile, "-> tessellation control shader compilation log : %s\n", szInfoLog);
                    fprintf(gpFile, "-------------------------------------------------------------\n");
                    free(szInfoLog);
                    [self release];
                    [NSApp terminate:self];
                }
            }
        }
        fprintf(gpFile, "-> tessellation control shader compiled successfully\n");
        fprintf(gpFile, "-------------------------------------------------------------\n");
        
        //tessellation evaluation shader
        tessellationEvaluationShaderObject = glCreateShader(GL_TESS_EVALUATION_SHADER);
        
        const GLchar *tessellationEvaluationShaderSource =
            "#version 450 core"                             \
            "\n"                                            \
            
            "layout(isolines)in;"                           \
            "uniform mat4 u_mvpMatrix;"                     \

            "void main(void)"                               \
            "{"                                             \
            "   float tessCoord = gl_TessCoord.x;"          \
            
            "   vec3 p0 = gl_in[0].gl_Position.xyz;"        \
            "   vec3 p1 = gl_in[1].gl_Position.xyz;"        \
            "   vec3 p2 = gl_in[2].gl_Position.xyz;"        \
            "   vec3 p3 = gl_in[3].gl_Position.xyz;"        \

            "   vec3 p = (p0 * (1.0f - tessCoord) * (1.0f - tessCoord) * (1.0f - tessCoord)) + (p1 * 3.0f * tessCoord * (1.0f - tessCoord) * (1.0f - tessCoord)) + (p2 * 3.0f * tessCoord * tessCoord * (1.0f - tessCoord)) + (p3 * tessCoord * tessCoord * tessCoord);"    \
            "   gl_Position = u_mvpMatrix * vec4(p, 1.0);"   \
            "}";
        
        //provide source code to fragment shader object
        glShaderSource(tessellationEvaluationShaderObject, 1, (const GLchar**)&tessellationEvaluationShaderSource, NULL);
        
        //compile shader
        glCompileShader(tessellationEvaluationShaderObject);
        
        //shader compilation error checking
        glGetShaderiv(tessellationEvaluationShaderObject, GL_COMPILE_STATUS, &shaderCompiledStatus);
        if(shaderCompiledStatus == GL_FALSE)
        {
            glGetShaderiv(tessellationEvaluationShaderObject, GL_INFO_LOG_LENGTH, &infoLogLength);
            if(infoLogLength > 0)
            {
                szInfoLog = (GLchar*)malloc(sizeof(GLchar) * infoLogLength);
                if(szInfoLog != NULL)
                {
                    GLsizei written;
                    glGetShaderInfoLog(tessellationEvaluationShaderObject, infoLogLength, &written, szInfoLog);
                    fprintf(gpFile, "-> tessellation evaluation shader compilation log : %s\n", szInfoLog);
                    fprintf(gpFile, "-------------------------------------------------------------\n");
                    free(szInfoLog);
                    [self release];
                    [NSApp terminate:self];
                }
            }
        }
        fprintf(gpFile, "-> tessellation evaluation shader compiled successfully\n");
        fprintf(gpFile, "-------------------------------------------------------------\n");

        //fragment shader
        fragmentShaderObject = glCreateShader(GL_FRAGMENT_SHADER);
        
        const GLchar *fragmentShaderSource =
            "#version 450 core"                             \
            "\n"                                            \
            
            "out vec4 FragColor;"                           \
            "uniform vec4 u_lineColor;"                     \

            "void main(void)"                               \
            "{"                                             \
            "   FragColor = u_lineColor;"                   \
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
        glAttachShader(shaderProgramObject, tessellationControlShaderObject);
        glAttachShader(shaderProgramObject, tessellationEvaluationShaderObject);
        glAttachShader(shaderProgramObject, fragmentShaderObject);
        
        //bind shader program object with vertex shader attributes
        glBindAttribLocation(shaderProgramObject, AMC_ATTRIBUTE_POSITION, "vPositon");
        
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
            
        //get uniform locations
        mvpMatrixUniform = glGetUniformLocation(shaderProgramObject, "u_mvpMatrix");
        numberOfSegmentsUniform = glGetUniformLocation(shaderProgramObject, "u_numberOfSegments");
        numberOfStripsUniform = glGetUniformLocation(shaderProgramObject, "u_numberOfStrips"); 
        lineColorUniform = glGetUniformLocation(shaderProgramObject, "u_lineColor");
            
        //set up input data
        const GLfloat vertices[] = 
        {
            -1.0f, -1.0f,
            -0.5f, 1.0f,
            0.5f, -1.0f,
            1.0f, 1.0f
        };
        
        //set up vao and vbo
        glGenVertexArrays(1, &vao);
        glBindVertexArray(vao);
            glGenBuffers(1, &vbo_position);
            glBindBuffer(GL_ARRAY_BUFFER, vbo_position);
                glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);
                glVertexAttribPointer(AMC_ATTRIBUTE_POSITION, 2, GL_FLOAT, GL_FALSE, 0, NULL);
                glEnableVertexAttribArray(AMC_ATTRIBUTE_POSITION);
            glBindBuffer(GL_ARRAY_BUFFER, 0);
        glBindVertexArray(0);
        
        //color buffer clearing color
        glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
        
        perspectiveProjectionMatrix = vmath::mat4::identity();
        uiNumberOfSegments = 1;

        //bind cgl and cv
        CVDisplayLinkCreateWithActiveCGDisplays(&display_link);
        CVDisplayLinkSetOutputCallback(display_link, &MyDisplayLinkCallback, self);
        CGLContextObj cgl_context = (CGLContextObj)[[self openGLContext] CGLContextObj];
        CGLPixelFormatObj cgl_pixelformat = (CGLPixelFormatObj)[[self pixelFormat] CGLPixelFormatObj];
        CVDisplayLinkSetCurrentCGDisplayFromOpenGLContext(display_link, cgl_context, cgl_pixelformat);
        CVDisplayLinkStart(display_link);
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
        vmath::mat4 modelViewProjectionMatrix;
        vmath::mat4 translateMatrix;
        
        //code
        [[self openGLContext] makeCurrentContext];
        CGLLockContext((CGLContextObj)[[self openGLContext] CGLContextObj]);
        
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
        
        modelViewMatrix = vmath::mat4::identity();
        mvpMatrix = vmath::mat4::identity();
        
        glUseProgram(shaderProgramObject);
            translateMatrix = vmath::translate(0.0f, 0.0f, -4.0f);
            modelViewMatrix = translateMatrix;
            modelViewProjectionMatrix = projectionMatrix * modelViewMatrix;

            glUniformMatrix4fv(mvpMatrixUniform, 1, GL_FALSE, modelViewProjectionMatrix);
            glUniform1i(numberOfSegmentsUniform, uiNumberOfSegments);
            glUniform1i(numberOfStripsUniform, 1);
            glUniform4fv(lineColorUniform, 1, vmath::vec4(1.0f, 1.0f, 0.0f, 1.0f));

            glBindVertexArray(vao);
            glPatchParameteri(GL_PATCH_VERTICES, 4);
            glDrawArrays(GL_PATCHES, 0, 4);
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

            case 'w':
                uiNumberOfSegments++;
                if(uiNumberOfSegments > 30)
                    uiNumberOfSegments = 30;
                break;
            
            case 's':
                uiNumberOfSegments--;
                if(uiNumberOfSegments <= 0)
                    uiNumberOfSegments = 1;
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
        if(vbo_position)
        {
            glDeleteBuffers(1, &vbo_position);
            vbo_position = 0;
        }
    
        if(vao)
        {
            glDeleteVertexArrays(1, &vao);
            vao = 0;
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

