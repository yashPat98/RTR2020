//packages
#import <Foundation/Foundation.h>
#import <Cocoa/Cocoa.h>
#import <QuartzCore/CVDisplayLink.h>
#import <OpenGL/GL3.h>
#include "vmath.h"
#include "Sphere.h"

//type declarations
enum
{
    AMC_ATTRIBUTE_POSITION = 0,
    AMC_ATTRIBUTE_COLOR,
    AMC_ATTRIBUTE_NORMAL,
    AMC_ATTRIBUTE_TEXCOORD
};

//global function declarations
CVReturn MyDisplayLinkCallback(CVDisplayLinkRef, const CVTimeStamp*, const CVTimeStamp*,
                               CVOptionFlags, CVOptionFlags*, void*);
float distance(float x1, float y1, float x2, float y2);

//global variables
FILE *gpFile = NULL;

//shader program variables
GLuint vertexShaderObject;
GLuint fragmentShaderObject;
GLuint shaderProgramObject;

//unioform location variables
GLuint mvpMatrixUniform;   
GLuint colorUniform;   

//input data buffer variables
GLuint vao;                        
GLuint vbo_position;     

GLuint vao_x;
GLuint vbo_x_position;

GLuint vao_y;
GLuint vbo_y_position;

GLuint vao_circle;
GLuint vbo_circle_position;

GLuint vao_square;
GLuint vbo_square_position;

GLuint vao_triangle;
GLuint vbo_triangle_position;

GLuint vao_incircle;
GLuint vbo_incircle_position;

//variable for uniform values
vmath::mat4 projectionMatrix;

std::vector<GLfloat> vertices_line;
std::vector<GLfloat> vertices_circle;
std::vector<GLfloat> vertices_incircle;

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
        [window setTitle:@"OpenGL : Graph"];
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
            "in vec4 vPosition;"                                        \
            "uniform mat4 u_mvpMatrix;"                                 \
            "void main(void)"                                           \
            "{"                                                         \
            "   gl_Position = u_mvpMatrix * vPosition;"                 \
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
            "#version 450 core"                             \
            "\n"                                            \
            "out vec4 FragColor;"                           \
            "uniform vec3 color;"                           \
            "void main(void)"                               \
            "{"                                             \
            "   FragColor = vec4(color, 1.0f);"             \
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
        fprintf(gpFile, "-------------------------------------------------------------\n");
            
        //get uniform locations
        mvpMatrixUniform = glGetUniformLocation(shaderProgramObject, "u_mvpMatrix"); 
        colorUniform = glGetUniformLocation(shaderProgramObject, "color");  

        //set up input data
        GLfloat fInterval = 0.05f;
        for(float fStep = -20.0f; fStep <= 20.0f; fStep++)
        {   
            vertices_line.push_back(-1.0f);
            vertices_line.push_back(fInterval * fStep);
            vertices_line.push_back(0.0f);

            vertices_line.push_back(1.0f);
            vertices_line.push_back(fInterval * fStep);
            vertices_line.push_back(0.0f);

            vertices_line.push_back(fInterval * fStep);
            vertices_line.push_back(-1.0f);
            vertices_line.push_back(0.0f);

            vertices_line.push_back(fInterval * fStep);
            vertices_line.push_back(1.0f);
            vertices_line.push_back(0.0f);
        }

        const GLfloat x_axis[] = 
        {
            -1.0f, 0.0f, 0.0f,
            1.0f, 0.0f, 0.0f
        };

        const GLfloat y_axis[] = 
        {
            0.0f, -1.0f, 0.0f,
            0.0f, 1.0f, 0.0f
        };

        const GLfloat square[] = 
        {
            cosf(0.785375f), sinf(0.785375f), 0.0f,
            cosf(M_PI - 0.785375f), sinf(M_PI - 0.785375f), 0.0f,
            -cosf(0.785375f), -sinf(0.785375f), 0.0f,
            sinf(M_PI - 0.785375f), cosf(M_PI - 0.785375f), 0.0f
        };

        const GLfloat triangle[] = 
        {
            0.0f, (cosf(0.785375f) - cosf(M_PI - 0.785375f)) / 2.0f, 0.0f,
            -cosf(0.785375f), -sinf(0.785375f), 0.0f,
            sinf(M_PI - 0.785375f), cosf(M_PI - 0.785375f), 0.0f
        };

        for(float angle = 0.0f; angle <= (2.0f * M_PI); angle += 0.1f)
        {
            GLfloat x = sin(angle);
            GLfloat y = cos(angle);

            vertices_circle.push_back(x);
            vertices_circle.push_back(y);
            vertices_circle.push_back(0.0f);        
        }

        //incircle
        float lab = distance(0.0f, (cos(0.785375f) - cos(M_PI - 0.785375f)) / 2.0f, -cos(0.785375f), -sin(0.785375f));
        float lbc = distance(-cos(0.785375f), -sin(0.785375f), sin(M_PI - 0.785375f), cos(M_PI - 0.785375f));
        float lac = distance(0.0f, (cos(0.785375f) - cos(M_PI - 0.785375f)) / 2.0f, sin(M_PI - 0.785375f), cos(M_PI - 0.785375f));
        float sum = lab + lbc + lac;

        float xin = ((lbc * 0.0f) + (lac * (-cos(0.785375f))) + (lab * sin(M_PI - 0.785375f))) / sum;
        float yin = ((lbc * ((cos(0.785375f) - cos(M_PI - 0.785375f)) / 2.0f)) + (lac * (-sin(0.785375f))) + (lab * cos(M_PI - 0.785375f))) / sum;

        //radius of incircle = area / semi-perimeter;
        float semi = (lab + lbc + lac) / 2;
        float radius = sqrt(semi * (semi - lab) * (semi - lbc) * (semi - lac)) / semi;

        for(float angle = 0.0f; angle <= (2 * M_PI); angle += 0.1f)
        {
            float x = radius * sin(angle);
            float y = radius * cos(angle);

            vertices_incircle.push_back(x + xin);
            vertices_incircle.push_back(y + yin);
            vertices_incircle.push_back(0.0f);
        }

        //set up vao and vbo
        glGenVertexArrays(1, &vao);
        glBindVertexArray(vao);
            glGenBuffers(1, &vbo_position);
            glBindBuffer(GL_ARRAY_BUFFER, vbo_position);
                glBufferData(GL_ARRAY_BUFFER, vertices_line.size() * sizeof(GLfloat), vertices_line.data(), GL_STATIC_DRAW);
                glVertexAttribPointer(AMC_ATTRIBUTE_POSITION, 3, GL_FLOAT, GL_FALSE, 0, NULL);
                glEnableVertexAttribArray(AMC_ATTRIBUTE_POSITION);
            glBindBuffer(GL_ARRAY_BUFFER, 0);
        glBindVertexArray(0);

        glGenVertexArrays(1, &vao_x);
        glBindVertexArray(vao_x);
            glGenBuffers(1, &vbo_x_position);
            glBindBuffer(GL_ARRAY_BUFFER, vbo_x_position);
                glBufferData(GL_ARRAY_BUFFER, sizeof(x_axis), x_axis, GL_STATIC_DRAW);
                glVertexAttribPointer(AMC_ATTRIBUTE_POSITION, 3, GL_FLOAT, GL_FALSE, 0, NULL);
                glEnableVertexAttribArray(AMC_ATTRIBUTE_POSITION);
            glBindBuffer(GL_ARRAY_BUFFER, 0);
        glBindVertexArray(0);

        glGenVertexArrays(1, &vao_y);
        glBindVertexArray(vao_y);
            glGenBuffers(1, &vbo_y_position);
            glBindBuffer(GL_ARRAY_BUFFER, vbo_y_position);
                glBufferData(GL_ARRAY_BUFFER, sizeof(y_axis), y_axis, GL_STATIC_DRAW);
                glVertexAttribPointer(AMC_ATTRIBUTE_POSITION, 3, GL_FLOAT, GL_FALSE, 0, NULL);
                glEnableVertexAttribArray(AMC_ATTRIBUTE_POSITION);
            glBindBuffer(GL_ARRAY_BUFFER, 0);
        glBindVertexArray(0);

        glGenVertexArrays(1, &vao_circle);
        glBindVertexArray(vao_circle);
            glGenBuffers(1, &vbo_circle_position);
            glBindBuffer(GL_ARRAY_BUFFER, vbo_circle_position);
                glBufferData(GL_ARRAY_BUFFER, vertices_circle.size() * sizeof(GLfloat), vertices_circle.data(), GL_STATIC_DRAW);
                glVertexAttribPointer(AMC_ATTRIBUTE_POSITION, 3, GL_FLOAT, GL_FALSE, 0, NULL);
                glEnableVertexAttribArray(AMC_ATTRIBUTE_POSITION);
            glBindBuffer(GL_ARRAY_BUFFER, 0);
        glBindVertexArray(0);

        glGenVertexArrays(1, &vao_square);
        glBindVertexArray(vao_square);
            glGenBuffers(1, &vbo_square_position);
            glBindBuffer(GL_ARRAY_BUFFER, vbo_square_position);
                glBufferData(GL_ARRAY_BUFFER, sizeof(square), square, GL_STATIC_DRAW);
                glVertexAttribPointer(AMC_ATTRIBUTE_POSITION, 3, GL_FLOAT, GL_FALSE, 0, NULL);
                glEnableVertexAttribArray(AMC_ATTRIBUTE_POSITION);
            glBindBuffer(GL_ARRAY_BUFFER, 0);
        glBindVertexArray(0);

        glGenVertexArrays(1, &vao_triangle);
        glBindVertexArray(vao_triangle);
            glGenBuffers(1, &vbo_triangle_position);
            glBindBuffer(GL_ARRAY_BUFFER, vbo_triangle_position);
                glBufferData(GL_ARRAY_BUFFER, sizeof(triangle), triangle, GL_STATIC_DRAW);
                glVertexAttribPointer(AMC_ATTRIBUTE_POSITION, 3, GL_FLOAT, GL_FALSE, 0, NULL);
                glEnableVertexAttribArray(AMC_ATTRIBUTE_POSITION);
            glBindBuffer(GL_ARRAY_BUFFER, 0);
        glBindVertexArray(0);

        glGenVertexArrays(1, &vao_incircle);
        glBindVertexArray(vao_incircle);
            glGenBuffers(1, &vbo_incircle_position);
            glBindBuffer(GL_ARRAY_BUFFER, vbo_incircle_position);
                glBufferData(GL_ARRAY_BUFFER, vertices_incircle.size() * sizeof(GLfloat), vertices_incircle.data(), GL_STATIC_DRAW);
                glVertexAttribPointer(AMC_ATTRIBUTE_POSITION, 3, GL_FLOAT, GL_FALSE, 0, NULL);
                glEnableVertexAttribArray(AMC_ATTRIBUTE_POSITION);
            glBindBuffer(GL_ARRAY_BUFFER, 0);
        glBindVertexArray(0);
        
        //set OpenGL states
        glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
        glClearDepth(1.0f);
        glEnable(GL_DEPTH_TEST);
        glDepthFunc(GL_LEQUAL);

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
        modelViewProjectionMatrix = vmath::mat4::identity();
        translateMatrix = vmath::mat4::identity();

        glUseProgram(shaderProgramObject);
            translateMatrix = vmath::translate(0.0f, 0.0f, -2.5f);
            modelViewMatrix = translateMatrix;
            modelViewProjectionMatrix = perspectiveProjectionMatrix * modelViewMatrix;

            glUniformMatrix4fv(mvpMatrixUniform, 1, GL_FALSE, modelViewProjectionMatrix);
            glUniform3f(colorUniform, 0.0f, 0.0f, 1.0f);

            //bind vao
            glBindVertexArray(vao);
            glDrawArrays(GL_LINES, 0, vertices_line.size() / 3);
            glBindVertexArray(0);

            glUniform3f(colorUniform, 1.0f, 0.0f, 0.0f);
            glBindVertexArray(vao_x);
            glDrawArrays(GL_LINES, 0, 2);
            glBindVertexArray(0);

            glUniform3f(colorUniform, 0.0f, 1.0f, 0.0f);
            glBindVertexArray(vao_y);
            glDrawArrays(GL_LINES, 0, 2);
            glBindVertexArray(0);

            glUniform3f(colorUniform, 1.0f, 1.0f, 0.0f);
            glBindVertexArray(vao_circle);
            glDrawArrays(GL_LINE_LOOP, 0, vertices_circle.size() / 3);
            glBindVertexArray(0);

            glBindVertexArray(vao_square);
            glDrawArrays(GL_LINE_LOOP, 0, 4);
            glBindVertexArray(0);

            glBindVertexArray(vao_triangle);
            glDrawArrays(GL_LINE_LOOP, 0, 3);
            glBindVertexArray(0);

            glBindVertexArray(vao_incircle);
            glDrawArrays(GL_LINE_LOOP, 0, vertices_incircle.size() / 3);
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

            default:
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
        //release vao and vbo
        vertices_line.clear();
        vertices_circle.clear();

        //release vao 
        if(vao)
        {
            glDeleteVertexArrays(1, &vao);
            vao = 0;
        }

        //release vbo
        if(vbo_position)
        {
            glDeleteBuffers(1, &vbo_position);
            vbo_position = 0;
        }

        if(vao_x)
        {
            glDeleteVertexArrays(1, &vao_x);
            vao_x = 0;
        }

        //release vbo
        if(vbo_x_position)
        {
            glDeleteBuffers(1, &vbo_x_position);
            vbo_x_position = 0;
        }

            if(vao_y)
        {
            glDeleteVertexArrays(1, &vao_y);
            vao_y = 0;
        }

        //release vbo
        if(vbo_y_position)
        {
            glDeleteBuffers(1, &vbo_y_position);
            vbo_y_position = 0;
        }

        if(vao_square)
        {
            glDeleteVertexArrays(1, &vao_square);
            vao_square = 0;
        }

        //release vbo
        if(vbo_square_position)
        {
            glDeleteBuffers(1, &vbo_square_position);
            vbo_square_position = 0;
        }

        if(vao_triangle)
        {
            glDeleteVertexArrays(1, &vao_triangle);
            vao_triangle = 0;
        }

        //release vbo
        if(vbo_triangle_position)
        {
            glDeleteBuffers(1, &vbo_triangle_position);
            vbo_triangle_position = 0;
        }

        if(vao_incircle)
        {
            glDeleteVertexArrays(1, &vao_incircle);
            vao_incircle = 0;
        }

        //release vbo
        if(vbo_incircle_position)
        {
            glDeleteBuffers(1, &vbo_incircle_position);
            vbo_incircle_position = 0;
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

float distance(float x1, float y1, float x2, float y2)
{
    //code
    float result = ((x2 - x1) * (x2 - x1)) + ((y2 - y1) * (y2 - y1));
    return ((float)sqrt(result));
}
