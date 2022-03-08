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
GLuint modelviewMatrixUniform;
GLuint projectionMatrixUniform;
GLuint lightDiffuseUniform;
GLuint lightPositionUniform;
GLuint textureSamplerUniform

GLuint toggleColorUniform;
GLuint toggleLightUniform;
GLuint toggleTextureUniform;

//input data buffer variables
GLuint vao;
GLuint vbo;

//texture variables
GLuint marble_texture;

//variable for uniform values
vmath::mat4 projectionMatrix;

int bAnimate;
int bLight;
int bColor;
int bTexture;

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
        [window setTitle:@"OpenGL : Diffuse Lighting"];
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
            "#version 410 core"                                                                         \
            "\n"                                                                                        \

            "in vec3 a_position;"                                                                       \
            "in vec3 a_color;"                                                                          \
            "in vec3 a_normal;"                                                                         \
            "in vec2 a_texcoord;"                                                                       \
            
            "out vec3 out_color;"                                                                       \
            "out vec2 out_texcoord;"                                                                    \

            "uniform mat4 modelview_matrix;"                                                            \
            "uniform mat4 projection_matrix;"                                                           \
            "uniform vec3 light_diffuse;"                                                               \
            "uniform vec4 light_position;"                                                              \
            
            "uniform int toggle_light;"                                                                 \
            "uniform int toggle_color;"                                                                 \

            "void main(void)"                                                                           \
            "{"                                                                                         \
            "   if(toggle_color == 1)"                                                                  \
            "   {"                                                                                      \
            "       out_color = a_color;"                                                               \
            "   }"                                                                                      \
            "   else"                                                                                   \
            "   {"                                                                                      \
            "       out_color = vec3(1.0f, 1.0f, 1.0f);"                                                \
            "   }"                                                                                      \

            "   if(toggle_light == 1)"                                                                  \
            "   {"                                                                                      \
            "       vec4 eye_coords = modelview_matrix * vec4(a_position, 1.0f);"                       \
            "       mat3 normal_matrix = mat3(transpose(inverse(modelview_matrix)));"                   \
            "       vec3 tnorm = normalize(normalize(normal_matrix * a_normal));"                       \
            "       vec3 s = normalize(vec3(light_position - eye_coords));"                             \
            "       out_color = light_diffuse * out_color * max(dot(s, tnorm), 0.0f);"                  \
            "   }"                                                                                      \
            
            "   out_texcoord = a_texcoord;"                                                             \
            "   gl_Position = projection_matrix * modelview_matrix * vec4(a_position, 1.0f);"           \
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
            "#version 410 core"                                                     \
            "\n"                                                                    \
            
            "in vec3 out_color;"                                                    \
            "in vec2 out_texcoord;"                                                 \
            "out vec4 frag_color;"                                                  \

            "uniform sampler2D texture_sampler;"                                    \
            "uniform int toggle_texture;"                                           \

            "void main(void)"                                                       \
            "{"                                                                     \
            "   vec3 color;"                                                        \
            "   if(toggle_texture == 1)"                                            \
            "   {"                                                                  \
            "       color = out_color * texture(texture_sampler, out_texcoord).rgb;"\
            "   }"                                                                  \
            "   else"                                                               \
            "   {"                                                                  \
            "       color = out_color;"                                             \
            "   }"                                                                  \

            "   frag_color = vec4(color, 1.0f);"                                    \
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
        glBindAttribLocation(shaderProgramObject, AMC_ATTRIBUTE_COLOR, "a_color");
        glBindAttribLocation(shaderProgramObject, AMC_ATTRIBUTE_NORMAL, "a_normal");
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
        modelviewMatrixUniform = glGetUniformLocation(shaderProgramObject, "modelview_matrix");
        projectionMatrixUniform = glGetUniformLocation(shaderProgramObject, "projection_matrix");
        lightDiffuseUniform = glGetUniformLocation(shaderProgramObject, "light_diffuse");
        lightPositionUniform = glGetUniformLocation(shaderProgramObject, "light_position");
        textureSamplerUniform = glGetUniformLocation(shaderProgramObject, "texture_sampler");

        toggleColorUniform = glGetUniformLocation(shaderProgramObject, "toggle_color");
        toggleLightUniform = glGetUniformLocation(shaderProgramObject, "toggle_light");
        toggleTextureUniform = glGetUniformLocation(shaderProgramObject, "toggle_texture");

        //set up input data
        const GLfloat cubePCNT[] = 
        {
            //near 
            +1.0f, +1.0f, +1.0f,    +1.0f, +0.0f, +0.0f,    +0.0f, +0.0f, +1.0f,    +1.0f, +1.0f,   
            -1.0f, +1.0f, +1.0f,    +1.0f, +0.0f, +0.0f,    +0.0f, +0.0f, +1.0f,    +0.0f, +1.0f,
            -1.0f, -1.0f, +1.0f,    +1.0f, +0.0f, +0.0f,    +0.0f, +0.0f, +1.0f,    +0.0f, +0.0f,
            +1.0f, -1.0f, +1.0f,    +1.0f, +0.0f, +0.0f,    +0.0f, +0.0f, +1.0f,    +1.0f, +0.0f,

            //right
            +1.0f, +1.0f, -1.0f,    +0.0f, +1.0f, +0.0f,    +1.0f, +0.0f, +0.0f,    +1.0f, +1.0f,
            +1.0f, +1.0f, +1.0f,    +0.0f, +1.0f, +0.0f,    +1.0f, +0.0f, +0.0f,    +0.0f, +1.0f,
            +1.0f, -1.0f, +1.0f,    +0.0f, +1.0f, +0.0f,    +1.0f, +0.0f, +0.0f,    +0.0f, +0.0f,
            +1.0f, -1.0f, -1.0f,    +0.0f, +1.0f, +0.0f,    +1.0f, +0.0f, +0.0f,    +1.0f, +0.0f,

            //far
            -1.0f, +1.0f, -1.0f,    +0.0f, +0.0f, +1.0f,    +0.0f, +0.0f, -1.0f,    +1.0f, +1.0f,
            +1.0f, +1.0f, -1.0f,    +0.0f, +0.0f, +1.0f,    +0.0f, +0.0f, -1.0f,    +0.0f, +1.0f,
            +1.0f, -1.0f, -1.0f,    +0.0f, +0.0f, +1.0f,    +0.0f, +0.0f, -1.0f,    +0.0f, +0.0f,
            -1.0f, -1.0f, -1.0f,    +0.0f, +0.0f, +1.0f,    +0.0f, +0.0f, -1.0f,    +1.0f, +0.0f,

            //left
            -1.0f, +1.0f, -1.0f,    +1.0f, +0.0f, +1.0f,    -1.0f, +0.0f, +0.0f,    +1.0f, +1.0f,
            -1.0f, +1.0f, +1.0f,    +1.0f, +0.0f, +1.0f,    -1.0f, +0.0f, +0.0f,    +0.0f, +1.0f,
            -1.0f, -1.0f, +1.0f,    +1.0f, +0.0f, +1.0f,    -1.0f, +0.0f, +0.0f,    +0.0f, +0.0f,
            -1.0f, -1.0f, -1.0f,    +1.0f, +0.0f, +1.0f,    -1.0f, +0.0f, +0.0f,    +1.0f, +0.0f,

            //top
            +1.0f, +1.0f, -1.0f,    +1.0f, +1.0f, +0.0f,    +0.0f, +1.0f, +0.0f,    +1.0f, +1.0f,
            -1.0f, +1.0f, -1.0f,    +1.0f, +1.0f, +0.0f,    +0.0f, +1.0f, +0.0f,    +0.0f, +1.0f,
            -1.0f, +1.0f, +1.0f,    +1.0f, +1.0f, +0.0f,    +0.0f, +1.0f, +0.0f,    +0.0f, +0.0f,
            +1.0f, +1.0f, +1.0f,    +1.0f, +1.0f, +0.0f,    +0.0f, +1.0f, +0.0f,    +1.0f, +0.0f,

            //bottom
            -1.0f, -1.0f, -1.0f,    +0.0f, +1.0f, +1.0f,    +0.0f, -1.0f, +0.0f,    +1.0f, +1.0f,
            +1.0f, -1.0f, -1.0f,    +0.0f, +1.0f, +1.0f,    +0.0f, -1.0f, +0.0f,    +0.0f, +1.0f,
            +1.0f, -1.0f, +1.0f,    +0.0f, +1.0f, +1.0f,    +0.0f, -1.0f, +0.0f,    +0.0f, +0.0f,
            -1.0f, -1.0f, +1.0f,    +0.0f, +1.0f, +1.0f,    +0.0f, -1.0f, +0.0f,    +1.0f, +0.0f,
        };

        //set up vao and vbo
        glGenVertexArrays(1, &vao);
        glBindVertexArray(vao);
            glGenBuffers(1, &vbo);
            glBindBuffer(GL_ARRAY_BUFFER, vbo);
                glBufferData(GL_ARRAY_BUFFER, 24 * 11 * sizeof(GLfloat), cubePCNT, GL_STATIC_DRAW);
                //position
                glVertexAttribPointer(AMC_ATTRIBUTE_POSITION, 3, GL_FLOAT, GL_FALSE, 11 * sizeof(GLfloat), (void*)0);
                glEnableVertexAttribArray(AMC_ATTRIBUTE_POSITION);
                //color
                glVertexAttribPointer(AMC_ATTRIBUTE_COLOR, 3, GL_FLOAT, GL_FALSE, 11 * sizeof(GLfloat), (void*)(3 * sizeof(GLfloat)));
                glEnableVertexAttribArray(AMC_ATTRIBUTE_COLOR);
                //normal
                glVertexAttribPointer(AMC_ATTRIBUTE_NORMAL, 3, GL_FLOAT, GL_FALSE, 11 * sizeof(GLfloat), (void*)(6 * sizeof(GLfloat)));
                glEnableVertexAttribArray(AMC_ATTRIBUTE_NORMAL);
                //texcoord
                glVertexAttribPointer(AMC_ATTRIBUTE_TEXCOORD, 2, GL_FLOAT, GL_FALSE, 11 * sizeof(GLfloat), (void*)(9 * sizeof(GLfloat)));
                glEnableVertexAttribArray(AMC_ATTRIBUTE_TEXCOORD);
            glBindBuffer(GL_ARRAY_BUFFER, 0);
        glBindVertexArray(0);

        //set OpenGL states
        glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
        glClearDepth(1.0f);                                     
        glEnable(GL_DEPTH_TEST);                                
        glDepthFunc(GL_LEQUAL);

        //load textures
        marble_texture = [self loadTextureFromBMPFile:"marble.bmp"];
        if(!marble_texture)
        {
            fprintf(gpFile, "Error : failed to load marble.bmp texture.\n");
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
        vmath::mat4 translateMatrix;
        vmath::mat4 rotateMatrix;

        static GLfloat cube_rotation_angle = 0.0f;

        //code
        [[self openGLContext] makeCurrentContext];
        CGLLockContext((CGLContextObj)[[self openGLContext] CGLContextObj]);
        
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
        
        modelViewMatrix = vmath::mat4::identity();
        translateMatrix = vmath::mat4::identity();
        rotateMatrix = vmath::mat4::identity();

        translateMatrix = vmath::translate(0.0f, 0.0f, -5.0f);
        rotateMatrix = vmath::rotate(cube_rotation_angle, 1.0f, 0.0f, 0.0f);
        rotateMatrix *= vmath::rotate(cube_rotation_angle, 0.0f, 1.0f, 0.0f);
        rotateMatrix *= vmath::rotate(cube_rotation_angle, 0.0f, 0.0f, 1.0f);
        modelViewMatrix = translateMatrix * rotateMatrix;
        
        glUseProgram(shaderProgramObject);
            glUniformMatrix4fv(modelviewMatrixUniform, 1, GL_FALSE, modelViewMatrix);
            glUniformMatrix4fv(projectionMatrixUniform, 1, GL_FALSE, projectionMatrix);

            if(bColor)
            {
                glUniform1i(toggleColorUniform, 1);
            }
            else 
            {
                glUniform1i(toggleColorUniform, 0);
            }

            if(bLight)
            {
                glUniform1i(toggleLightUniform, 1);
                glUniform3f(lightDiffuseUniform, 1.0f, 1.0f, 1.0f);
                glUniform4f(lightPositionUniform, 0.0f, 0.0f, 2.0f, 1.0f);
            }
            else
            {
                glUniform1i(toggleLightUniform, 0);
            }

            if(bTexture)
            {
                glUniform1i(toggleTextureUniform, 1);

                glActiveTexture(GL_TEXTURE0);
                glBindTexture(GL_TEXTURE_2D, marble_texture);
                glUniform1i(textureSamplerUniform, 0);
            }
            else 
            {
                glUniform1i(toggleTextureUniform, 0);
            }

            glBindVertexArray(vao_cube);
            glDrawArrays(GL_TRIANGLE_FAN, 0, 4);
            glDrawArrays(GL_TRIANGLE_FAN, 4, 4);
            glDrawArrays(GL_TRIANGLE_FAN, 8, 4);
            glDrawArrays(GL_TRIANGLE_FAN, 12, 4);
            glDrawArrays(GL_TRIANGLE_FAN, 16, 4);
            glDrawArrays(GL_TRIANGLE_FAN, 20, 4);
            glBindVertexArray(0);
        glUseProgram(0);
        
        //update 
        if(bAnimate)
        {
            cube_rotation_angle += 0.1f;
            if(cube_rotation_angle >= 360.0f)
                cube_rotation_angle = 0.0f;
        }

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

            case 'A':
            case 'a':
                if(bAnimate)
                {
                    bAnimate = false;
                }
                else
                {
                    bAnimate = true;
                }
                break;

            case 'C':
            case 'c':
                if(bColor)
                {
                    bColor = false;
                }
                else
                {
                    bColor = true;
                }
                break;

            case 'T':
            case 't':
                if(bTexture)
                {
                    bTexture = false;
                }
                else
                {
                    bTexture = true;
                }
                break;

            case 'L':
            case 'l':
                if(bLight)
                {
                    bLight = false;
                }
                else
                {
                    bLight = true;
                }
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
        if(marble_texture)
        {
            glDeleteTextures(1, &marble_texture);
            marble_texture = 0;
        }

        if(vbo)
        {
            glDeleteBuffers(1, &vbo);
            vbo = 0;
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

