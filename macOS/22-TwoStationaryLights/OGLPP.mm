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

struct LIGHT
{
    vmath::vec3 lightAmbient;
    vmath::vec3 lightDiffuse;
    vmath::vec3 lightSpecular;
    vmath::vec4 lightPosition;
};

struct MATERIAL
{
    vmath::vec3 materialAmbient;
    vmath::vec3 materialDiffuse;
    vmath::vec3 materialSpecular;
    float materialShininess;
};

//global function declarations
CVReturn MyDisplayLinkCallback(CVDisplayLinkRef, const CVTimeStamp*, const CVTimeStamp*,
                               CVOptionFlags, CVOptionFlags*, void*);

//global variables
FILE *gpFile = NULL;

//shader program variables
GLuint vertexShaderObject;
GLuint fragmentShaderObject;
GLuint shaderProgramObject;

//unioform location variables
GLuint modelMatrixUniform;
GLuint viewMatrixUniform;
GLuint perspectiveProjectionMatrixUniform;
GLuint LaUniform[2];
GLuint LdUniform[2];
GLuint LsUniform[2];
GLuint lightPositionUniform[2];
GLuint KaUniform;
GLuint KdUniform;
GLuint KsUniform;
GLuint materialShininessUniform;
GLuint LKeyPressedUniform;

//input data buffer variables
GLuint vao_pyramid;
GLuint vbo_pyramid_position;
GLuint vbo_pyramid_normal;

//variable for uniform values
vmath::mat4 projectionMatrix;
struct LIGHT light[2];
struct MATERIAL material;
int key_pressed;

int numVertices;
int numElements;

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
        [window setTitle:@"OpenGL : Two Stationary Lights"];
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
            "#version 410 core"                                                                                                     \
            "\n"                                                                                                                    \
            "in vec4 vPosition;"                                                                                                    \
            "in vec3 vNormal;"                                                                                                      \
            "uniform mat4 u_modelMatrix;"                                                                                           \
            "uniform mat4 u_viewMatrix;"                                                                                            \
            "uniform mat4 u_perspectiveProjectionMatrix;"                                                                           \
            "uniform vec3 u_La[2];"                                                                                                 \
            "uniform vec3 u_Ld[2];"                                                                                                 \
            "uniform vec3 u_Ls[2];"                                                                                                 \
            "uniform vec4 u_lightPosition[2];"                                                                                      \
            "uniform vec3 u_Ka;"                                                                                                    \
            "uniform vec3 u_Kd;"                                                                                                    \
            "uniform vec3 u_Ks;"                                                                                                    \
            "uniform float u_materialShininess;"                                                                                    \
            "uniform int u_LKeyPressed;"                                                                                            \
            "out vec3 phong_ads_light;"                                                                                             \
            "void main(void)"                                                                                                       \
            "{"                                                                                                                     \
            "   phong_ads_light = vec3(0.0f, 0.0f, 0.0f);"                                                                          \
            "   if(u_LKeyPressed == 1)"                                                                                             \
            "   {"                                                                                                                  \
            "       vec4 eye_coords = u_viewMatrix * u_modelMatrix * vPosition;"                                                    \
            "       mat3 normal_matrix = mat3(transpose(inverse(u_viewMatrix * u_modelMatrix)));"                                   \
            "       vec3 transformed_normal = normalize(normal_matrix * vNormal);"                                                  \
            "       vec3 view_vector = normalize(-eye_coords.xyz);"                                                                 \
            "       vec3 light_direction[2];"                                                                                       \
            "       vec3 reflection_vector[2];"                                                                                     \
            "       vec3 ambient[2];"                                                                                               \
            "       vec3 diffuse[2];"                                                                                               \
            "       vec3 specular[2];"                                                                                              \
            "       for(int i = 0; i < 2; i++)"                                                                                     \
            "       {"                                                                                                              \
            "           light_direction[i] = normalize(vec3(u_lightPosition[i] - eye_coords));"                                     \
            "           reflection_vector[i] = reflect(-light_direction[i], transformed_normal);"                                   \
            "           ambient[i] = u_La[i] * u_Ka;"                                                                               \
            "           diffuse[i] = u_Ld[i] * u_Kd * max(dot(light_direction[i], transformed_normal), 0.0f);"                      \
            "           specular[i] = u_Ls[i] * u_Ks * pow(max(dot(reflection_vector[i], view_vector), 0.0f), u_materialShininess);"\
            "           phong_ads_light = phong_ads_light + ambient[i] + diffuse[i] + specular[i];"                                 \
            "       }"                                                                                                              \
            "   }"                                                                                                                  \
            "   else"                                                                                                               \
            "   {"                                                                                                                  \
            "       phong_ads_light = vec3(1.0f, 1.0f, 1.0f);"                                                                      \
            "   }"                                                                                                                  \
            "   gl_Position = u_perspectiveProjectionMatrix * u_viewMatrix * u_modelMatrix * vPosition;"                            \
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
            "#version 410 core"                                 \
            "\n"                                                \
            "in vec3 phong_ads_light;"                          \
            "out vec4 FragColor;"                               \
            "void main(void)"                                   \
            "{"                                                 \
            "   FragColor = vec4(phong_ads_light, 1.0f);"       \
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
        glBindAttribLocation(shaderProgramObject, AMC_ATTRIBUTE_NORMAL, "vNormal");
        
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
        modelMatrixUniform = glGetUniformLocation(shaderProgramObject, "u_modelMatrix");
        viewMatrixUniform = glGetUniformLocation(shaderProgramObject, "u_viewMatrix");
        perspectiveProjectionMatrixUniform = glGetUniformLocation(shaderProgramObject, "u_perspectiveProjectionMatrix");

        LaUniform[0] = glGetUniformLocation(shaderProgramObject, "u_La[0]");
        LdUniform[0] = glGetUniformLocation(shaderProgramObject, "u_Ld[0]");
        LsUniform[0] = glGetUniformLocation(shaderProgramObject, "u_Ls[0]");
        lightPositionUniform[0] = glGetUniformLocation(shaderProgramObject, "u_lightPosition[0]");

        LaUniform[1] = glGetUniformLocation(shaderProgramObject, "u_La[1]");
        LdUniform[1] = glGetUniformLocation(shaderProgramObject, "u_Ld[1]");
        LsUniform[1] = glGetUniformLocation(shaderProgramObject, "u_Ls[1]");
        lightPositionUniform[1] = glGetUniformLocation(shaderProgramObject, "u_lightPosition[1]");

        KaUniform = glGetUniformLocation(shaderProgramObject, "u_Ka");
        KdUniform = glGetUniformLocation(shaderProgramObject, "u_Kd");
        KsUniform = glGetUniformLocation(shaderProgramObject, "u_Ks");
        materialShininessUniform = glGetUniformLocation(shaderProgramObject, "u_materialShininess");

        LKeyPressedUniform = glGetUniformLocation(shaderProgramObject, "u_LKeyPressed");

        //set up input data
        const GLfloat pyramidVertices[] =
        {
            //near
            0.0f, 1.0f, 0.0f,
            -1.0f, -1.0f, 1.0f,
            1.0f, -1.0f, 1.0f,

            //right
            0.0f, 1.0f, 0.0f,
            1.0f, -1.0f, 1.0f,
            1.0f, -1.0f, -1.0f,
        
            //far
            0.0f, 1.0f, 0.0f,
            1.0f, -1.0f, -1.0f,
            -1.0f, -1.0f, -1.0f,

            //left
            0.0f, 1.0f, 0.0f,
            -1.0f, -1.0f, -1.0f,
            -1.0f, -1.0f, 1.0f
        };

        const GLfloat pyramidNormals[] = 
        {
            //near 
            0.0f, 0.447214f, 0.894427f,
            0.0f, 0.447214f, 0.894427f,
            0.0f, 0.447214f, 0.894427f,
            
            //right
            0.894427f, 0.447214f, 0.0f,
            0.894427f, 0.447214f, 0.0f,
            0.894427f, 0.447214f, 0.0f,

            //far
            0.0f, 0.447214f, -0.894427f,
            0.0f, 0.447214f, -0.894427f,
            0.0f, 0.447214f, -0.894427f,

            //left
            -0.894427f, 0.447214f, 0.0f,
            -0.894427f, 0.447214f, 0.0f,
            -0.894427f, 0.447214f, 0.0f
        };
        
        //set up vao and vbo
        glGenVertexArrays(1, &vao_pyramid);
        glBindVertexArray(vao_pyramid);
            glGenBuffers(1, &vbo_pyramid_position);
            glBindBuffer(GL_ARRAY_BUFFER, vbo_pyramid_position);
                glBufferData(GL_ARRAY_BUFFER, sizeof(pyramidVertices), pyramidVertices, GL_STATIC_DRAW);
                glVertexAttribPointer(AMC_ATTRIBUTE_POSITION, 3, GL_FLOAT, GL_FALSE, 0, NULL);
                glEnableVertexAttribArray(AMC_ATTRIBUTE_POSITION);
            glBindBuffer(GL_ARRAY_BUFFER, 0);

            glGenBuffers(1, &vbo_pyramid_normal);
            glBindBuffer(GL_ARRAY_BUFFER, vbo_pyramid_normal);
                glBufferData(GL_ARRAY_BUFFER, sizeof(pyramidNormals), pyramidNormals, GL_STATIC_DRAW);
                glVertexAttribPointer(AMC_ATTRIBUTE_NORMAL, 3, GL_FLOAT, GL_FALSE, 0, NULL);
                glEnableVertexAttribArray(AMC_ATTRIBUTE_NORMAL);
            glBindBuffer(GL_ARRAY_BUFFER, 0);
        glBindVertexArray(0);
        
        //set OpenGL states
        glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
        glClearDepth(1.0f);
        glEnable(GL_DEPTH_TEST);
        glDepthFunc(GL_LEQUAL);
        
        //set light and material variables
        light[0].lightAmbient = vmath::vec3(0.0f, 0.0f, 0.0f);
        light[0].lightDiffuse = vmath::vec3(1.0f, 0.0f, 0.0f);
        light[0].lightSpecular = vmath::vec3(1.0f, 0.0f, 0.0f);
        light[0].lightPosition = vmath::vec4(2.0f, 0.0f, 0.0f, 1.0f);

        light[1].lightAmbient = vmath::vec3(0.0f, 0.0f, 0.0f);
        light[1].lightDiffuse = vmath::vec3(0.0f, 0.0f, 1.0f);
        light[1].lightSpecular = vmath::vec3(0.0f, 0.0f, 1.0f);
        light[1].lightPosition = vmath::vec4(-2.0f, 0.0f, 0.0f, 1.0f);

        material.materialAmbient = vmath::vec3(0.0f, 0.0f, 0.0f);
        material.materialDiffuse = vmath::vec3(1.0f, 1.0f, 1.0f);
        material.materialSpecular = vmath::vec3(1.0f, 1.0f, 1.0f);
        material.materialShininess = 50.0f;

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
        vmath::mat4 modelMatrix;
        vmath::mat4 viewMatrix;
        vmath::mat4 translationMatrix;
        vmath::mat4 rotationMatrix;

        static GLfloat pyramid_rotation_angle = 0.0f;

        //code
        [[self openGLContext] makeCurrentContext];
        CGLLockContext((CGLContextObj)[[self openGLContext] CGLContextObj]);
        
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        glUseProgram(shaderProgramObject);

        //set matrices to identity
        modelMatrix = vmath::mat4::identity();
        viewMatrix = vmath::mat4::identity();
        translationMatrix = vmath::mat4::identity();
        rotationMatrix = vmath::mat4::identity();

        //model matrix transformations
        translationMatrix = vmath::translate(0.0f, 0.0f, -5.0f);
        rotationMatrix = vmath::rotate(pyramid_rotation_angle, 0.0f, 1.0f, 0.0f);
        modelMatrix = translationMatrix * rotationMatrix;

        //pass model, view and projection matrices to shader uniform variables
        glUniformMatrix4fv(modelMatrixUniform, 1, GL_FALSE, modelMatrix);
        glUniformMatrix4fv(viewMatrixUniform, 1, GL_FALSE, viewMatrix);
        glUniformMatrix4fv(perspectiveProjectionMatrixUniform, 1, GL_FALSE, projectionMatrix);


        //pass the uniform variables to vertex shader
        if(key_pressed == 1)
        {
            glUniform1i(LKeyPressedUniform, 1);

            glUniform3fv(LaUniform[0], 1, light[0].lightAmbient);
            glUniform3fv(LdUniform[0], 1, light[0].lightDiffuse);
            glUniform3fv(LsUniform[0], 1, light[0].lightSpecular);
            glUniform4fv(lightPositionUniform[0], 1, light[0].lightPosition);

            glUniform3fv(LaUniform[1], 1, light[1].lightAmbient);
            glUniform3fv(LdUniform[1], 1, light[1].lightDiffuse);
            glUniform3fv(LsUniform[1], 1, light[1].lightSpecular);
            glUniform4fv(lightPositionUniform[1], 1, light[1].lightPosition);

            glUniform3fv(KaUniform, 1, material.materialAmbient);
            glUniform3fv(KdUniform, 1, material.materialDiffuse);
            glUniform3fv(KsUniform, 1, material.materialSpecular);
            glUniform1f(materialShininessUniform, material.materialShininess);
        }
        else
        {
            glUniform1i(LKeyPressedUniform, 0);
        }

        glBindVertexArray(vao_pyramid);
        glDrawArrays(GL_TRIANGLES, 0, 12);

        glBindVertexArray(0);
        glUseProgram(0);

        //update 
        pyramid_rotation_angle += 0.1f;
        if(pyramid_rotation_angle > 360.0f)
            pyramid_rotation_angle = 0.0f;
        
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

            case 'L':
            case 'l':
                if(key_pressed == 0)
                {
                    key_pressed = 1;
                }
                else
                {
                    key_pressed = 0;
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
        if(vao_pyramid)
        {
            glDeleteVertexArrays(1, &vao_pyramid);
            vao_pyramid = 0;
        }

        if(vbo_pyramid_position)
        {
            glDeleteBuffers(1, &vbo_pyramid_position);
            vbo_pyramid_position = 0;
        }

        if(vbo_pyramid_normal)
        {
            glDeleteVertexArrays(1, &vbo_pyramid_normal);
            vbo_pyramid_normal = 0;
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

