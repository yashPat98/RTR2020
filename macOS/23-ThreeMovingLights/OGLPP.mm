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

//shader program variables for pv lighting
GLuint pv_vertexShaderObject;
GLuint pv_fragmentShaderObject;
GLuint pv_shaderProgramObject;

//unioform location variables
GLuint pv_modelMatrixUniform;
GLuint pv_viewMatrixUniform;
GLuint pv_projectionMatrixUniform;
GLuint pv_lightAmbientUniform[3];
GLuint pv_lightDiffuseUniform[3];
GLuint pv_lightSpecularUniform[3];
GLuint pv_lightPositionUniform[3];
GLuint pv_materialAmbientUniform;
GLuint pv_materialDiffuseUniform;
GLuint pv_materialSpecularUniform;
GLuint pv_materialShininessUniform;
GLuint pv_keyPressedUniform;

//shader program variables for pf lighting
GLuint pf_vertexShaderObject;
GLuint pf_fragmentShaderObject;
GLuint pf_shaderProgramObject;

//unioform location variables
GLuint pf_modelMatrixUniform;
GLuint pf_viewMatrixUniform;
GLuint pf_projectionMatrixUniform;
GLuint pf_lightAmbientUniform[3];
GLuint pf_lightDiffuseUniform[3];
GLuint pf_lightSpecularUniform[3];
GLuint pf_lightPositionUniform[3];
GLuint pf_materialAmbientUniform;
GLuint pf_materialDiffuseUniform;
GLuint pf_materialSpecularUniform;
GLuint pf_materialShininessUniform;
GLuint pf_keyPressedUniform;

//input data buffer variables
GLuint vao_sphere;
GLuint vbo_sphere_position;
GLuint vbo_sphere_normal;
GLuint vbo_sphere_indices;

//variable for uniform values
vmath::mat4 projectionMatrix;
struct LIGHT light[3];
struct MATERIAL material;
int key_pressed;
int toggle;

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
        [window setTitle:@"OpenGL : Three Moving Lights"];
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
        
        //per-vertex lighting
        fprintf(gpFile, "-> per-vertex lighting\n");
        fprintf(gpFile, "-------------------------------------------------------------\n");

        //vertex shader
        pv_vertexShaderObject = glCreateShader(GL_VERTEX_SHADER);
        
        const GLchar *pv_vertexShaderSource =
            "#version 450 core"                                                                                                         \
            "\n"                                                                                                                        \
            "in vec4 vPosition;"                                                                                                        \
            "in vec3 vNormal;"                                                                                                          \
            "uniform mat4 u_modelMatrix;"                                                                                               \
            "uniform mat4 u_viewMatrix;"                                                                                                \
            "uniform mat4 u_perspectiveProjectionMatrix;"                                                                               \
            "uniform vec3 u_La[3];"                                                                                                     \
            "uniform vec3 u_Ld[3];"                                                                                                     \
            "uniform vec3 u_Ls[3];"                                                                                                     \
            "uniform vec4 u_lightPosition[3];"                                                                                          \
            "uniform vec3 u_Ka;"                                                                                                        \
            "uniform vec3 u_Kd;"                                                                                                        \
            "uniform vec3 u_Ks;"                                                                                                        \
            "uniform float u_materialShininess;"                                                                                        \
            "uniform int u_LKeyPressed;"                                                                                                \
            "out vec3 phong_ads_light;"                                                                                                 \
            "void main(void)"                                                                                                           \
            "{"                                                                                                                         \
            "   phong_ads_light = vec3(0.0f, 0.0f, 0.0f);"                                                                              \
            "   if(u_LKeyPressed == 1)"                                                                                                 \
            "   {"                                                                                                                      \
            "       vec4 eye_coords = u_viewMatrix * u_modelMatrix * vPosition;"                                                        \
            "       mat3 normal_matrix = mat3(transpose(inverse(u_viewMatrix * u_modelMatrix)));"                                       \
            "       vec3 transformed_normal = normalize(normal_matrix * vNormal);"                                                      \
            "       vec3 view_vector = normalize(-eye_coords.xyz);"                                                                     \
            "       vec3 light_direction[3];"                                                                                           \
            "       vec3 reflection_vector[3];"                                                                                         \
            "       vec3 ambient[3];"                                                                                                   \
            "       vec3 diffuse[3];"                                                                                                   \
            "       vec3 specular[3];"                                                                                                  \
            "       for(int i = 0; i < 3; i++)"                                                                                         \
            "       {"                                                                                                                  \
            "           light_direction[i] = normalize(vec3(u_lightPosition[i] - eye_coords));"                                         \
            "           reflection_vector[i] = reflect(-light_direction[i], transformed_normal);"                                       \
            "           ambient[i] = u_La[i] * u_Ka;"                                                                                   \
            "           diffuse[i] = u_Ld[i] * u_Kd * max(dot(light_direction[i], transformed_normal), 0.0f);"                          \
            "           specular[i] = u_Ls[i] * u_Ks * pow(max(dot(reflection_vector[i], view_vector), 0.0f), u_materialShininess);"    \
            "           phong_ads_light = phong_ads_light + ambient[i] + diffuse[i] + specular[i];"                                     \
            "       }"                                                                                                                  \
            "   }"                                                                                                                      \
            "   else"                                                                                                                   \
            "   {"                                                                                                                      \
            "       phong_ads_light = vec3(1.0f, 1.0f, 1.0f);"                                                                          \
            "   }"                                                                                                                      \
            "   gl_Position = u_perspectiveProjectionMatrix * u_viewMatrix * u_modelMatrix * vPosition;"                                \
            "}";
        
        //provide source code to vertex shader object
        glShaderSource(pv_vertexShaderObject, 1, (const GLchar**)&pv_vertexShaderSource, NULL);
        
        //compile shader
        glCompileShader(pv_vertexShaderObject);
        
        //shader compilation error checking
        GLint infoLogLength = 0;
        GLint shaderCompiledStatus = 0;
        GLchar *szInfoLog = NULL;
        
        glGetShaderiv(pv_vertexShaderObject, GL_COMPILE_STATUS, &shaderCompiledStatus);
        if(shaderCompiledStatus == GL_FALSE)
        {
            glGetShaderiv(pv_vertexShaderObject, GL_INFO_LOG_LENGTH, &infoLogLength);
            if(infoLogLength > 0)
            {
                szInfoLog = (GLchar*)malloc(sizeof(GLchar) * infoLogLength);
                if(szInfoLog != NULL)
                {
                    GLsizei written;
                    glGetShaderInfoLog(pv_vertexShaderObject, infoLogLength, &written, szInfoLog);
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
        pv_fragmentShaderObject = glCreateShader(GL_FRAGMENT_SHADER);
        
        const GLchar *pv_fragmentShaderSource =
            "#version 450 core"                                 \
            "\n"                                                \
            "in vec3 phong_ads_light;"                          \
            "out vec4 FragColor;"                               \
            "void main(void)"                                   \
            "{"                                                 \
            "   FragColor = vec4(phong_ads_light, 1.0f);"       \
            "}";
        
        //provide source code to fragment shader object
        glShaderSource(pv_fragmentShaderObject, 1, (const GLchar**)&pv_fragmentShaderSource, NULL);
        
        //compile shader
        glCompileShader(pv_fragmentShaderObject);
        
        //shader compilation error checking
        glGetShaderiv(pv_fragmentShaderObject, GL_COMPILE_STATUS, &shaderCompiledStatus);
        if(shaderCompiledStatus == GL_FALSE)
        {
            glGetShaderiv(pv_fragmentShaderObject, GL_INFO_LOG_LENGTH, &infoLogLength);
            if(infoLogLength > 0)
            {
                szInfoLog = (GLchar*)malloc(sizeof(GLchar) * infoLogLength);
                if(szInfoLog != NULL)
                {
                    GLsizei written;
                    glGetShaderInfoLog(pv_fragmentShaderObject, infoLogLength, &written, szInfoLog);
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
        pv_shaderProgramObject = glCreateProgram();
        
        //attach shaders to program object
        glAttachShader(pv_shaderProgramObject, pv_vertexShaderObject);
        glAttachShader(pv_shaderProgramObject, pv_fragmentShaderObject);
        
        //bind shader program object with vertex shader attributes
        glBindAttribLocation(pv_shaderProgramObject, AMC_ATTRIBUTE_POSITION, "vPositon");
        glBindAttribLocation(pv_shaderProgramObject, AMC_ATTRIBUTE_NORMAL, "vNormal");
        
        //link program
        glLinkProgram(pv_shaderProgramObject);
        
        //shader linking error chechking
        GLint shaderProgramLinkStatus = 0;
        glGetProgramiv(pv_shaderProgramObject, GL_LINK_STATUS, &shaderProgramLinkStatus);
        if(shaderProgramLinkStatus == GL_FALSE)
        {
            glGetProgramiv(pv_shaderProgramObject, GL_INFO_LOG_LENGTH, &infoLogLength);
            if(infoLogLength > 0)
            {
                szInfoLog = (GLchar*)malloc(sizeof(GLchar) * infoLogLength);
                if(szInfoLog != NULL)
                {
                    GLsizei written;
                    glGetProgramInfoLog(pv_shaderProgramObject, infoLogLength, &written, szInfoLog);
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
        pv_modelMatrixUniform = glGetUniformLocation(pv_shaderProgramObject, "u_modelMatrix");
        pv_viewMatrixUniform = glGetUniformLocation(pv_shaderProgramObject, "u_viewMatrix");
        pv_projectionMatrixUniform = glGetUniformLocation(pv_shaderProgramObject, "u_perspectiveProjectionMatrix");

        pv_lightAmbientUniform[0] = glGetUniformLocation(pv_shaderProgramObject, "u_La[0]");
        pv_lightDiffuseUniform[0] = glGetUniformLocation(pv_shaderProgramObject, "u_Ld[0]");
        pv_lightSpecularUniform[0] = glGetUniformLocation(pv_shaderProgramObject, "u_Ls[0]");
        pv_lightPositionUniform[0] = glGetUniformLocation(pv_shaderProgramObject, "u_lightPosition[0]");

        pv_lightAmbientUniform[1] = glGetUniformLocation(pv_shaderProgramObject, "u_La[1]");
        pv_lightDiffuseUniform[1] = glGetUniformLocation(pv_shaderProgramObject, "u_Ld[1]");
        pv_lightSpecularUniform[1] = glGetUniformLocation(pv_shaderProgramObject, "u_Ls[1]");
        pv_lightPositionUniform[1] = glGetUniformLocation(pv_shaderProgramObject, "u_lightPosition[1]");

        pv_lightAmbientUniform[2] = glGetUniformLocation(pv_shaderProgramObject, "u_La[2]");
        pv_lightDiffuseUniform[2] = glGetUniformLocation(pv_shaderProgramObject, "u_Ld[2]");
        pv_lightSpecularUniform[2] = glGetUniformLocation(pv_shaderProgramObject, "u_Ls[2]");
        pv_lightPositionUniform[2] = glGetUniformLocation(pv_shaderProgramObject, "u_lightPosition[2]");

        pv_materialAmbientUniform = glGetUniformLocation(pv_shaderProgramObject, "u_Ka");
        pv_materialDiffuseUniform = glGetUniformLocation(pv_shaderProgramObject, "u_Kd");
        pv_materialSpecularUniform = glGetUniformLocation(pv_shaderProgramObject, "u_Ks");
        pv_materialShininessUniform = glGetUniformLocation(pv_shaderProgramObject, "u_materialShininess");
        pv_keyPressedUniform = glGetUniformLocation(pv_shaderProgramObject, "u_LKeyPressed");

        //per-fragment lighting
        fprintf(gpFile, "-> per-fragment lighting\n");
        fprintf(gpFile, "-------------------------------------------------------------\n");

        //vertex shader
        pf_vertexShaderObject = glCreateShader(GL_VERTEX_SHADER);
        
        const GLchar *pf_vertexShaderSource =
            "#version 450 core"                                                                                     \
            "\n"                                                                                                    \
            "in vec4 vPosition;"                                                                                    \
            "in vec3 vNormal;"                                                                                      \
            "uniform mat4 u_modelMatrix;"                                                                           \
            "uniform mat4 u_viewMatrix;"                                                                            \
            "uniform mat4 u_perspectiveProjectionMatrix;"                                                           \
            "uniform vec4 u_lightPosition[3];"                                                                      \
            "uniform int u_LKeyPressed;"                                                                            \
            "out vec3 transformed_normal;"                                                                          \
            "out vec3 light_direction[3];"                                                                          \
            "out vec3 view_vector;"                                                                                 \
            "void main(void)"                                                                                       \
            "{"                                                                                                     \
            "   if(u_LKeyPressed == 1)"                                                                             \
            "   {"                                                                                                  \
            "       vec4 eye_coords = u_viewMatrix * u_modelMatrix * vPosition;"                                    \
            "       mat3 normal_matrix = mat3(transpose(inverse(u_viewMatrix * u_modelMatrix))); "                  \
            "       transformed_normal = normal_matrix * vNormal;"                                                  \
            "       for(int i = 0; i < 3; i++)"                                                                     \
            "       {"                                                                                              \
            "           light_direction[i] = vec3(u_lightPosition[i] - eye_coords);"                                \
            "       }"                                                                                              \
            "       view_vector = -eye_coords.xyz;"                                                                 \
            "   }"                                                                                                  \
            "   gl_Position = u_perspectiveProjectionMatrix * u_viewMatrix * u_modelMatrix * vPosition;"            \
            "}";                                                                  
            
        //provide source code to vertex shader object
        glShaderSource(pf_vertexShaderObject, 1, (const GLchar**)&pf_vertexShaderSource, NULL);
        
        //compile shader
        glCompileShader(pf_vertexShaderObject);
        
        //shader compilation error checking
        glGetShaderiv(pf_vertexShaderObject, GL_COMPILE_STATUS, &shaderCompiledStatus);
        if(shaderCompiledStatus == GL_FALSE)
        {
            glGetShaderiv(pf_vertexShaderObject, GL_INFO_LOG_LENGTH, &infoLogLength);
            if(infoLogLength > 0)
            {
                szInfoLog = (GLchar*)malloc(sizeof(GLchar) * infoLogLength);
                if(szInfoLog != NULL)
                {
                    GLsizei written;
                    glGetShaderInfoLog(pf_vertexShaderObject, infoLogLength, &written, szInfoLog);
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
        pf_fragmentShaderObject = glCreateShader(GL_FRAGMENT_SHADER);
        
        const GLchar *pf_fragmentShaderSource =
            "#version 450 core"                                                                                                                     \
            "\n"                                                                                                                                    \
            "in vec3 transformed_normal;"                                                                                                           \
            "in vec3 light_direction[3];"                                                                                                           \
            "in vec3 view_vector;"                                                                                                                  \
            "uniform vec3 u_La[3];"                                                                                                                 \
            "uniform vec3 u_Ld[3];"                                                                                                                 \
            "uniform vec3 u_Ls[3];"                                                                                                                 \
            "uniform vec3 u_Ka;"                                                                                                                    \
            "uniform vec3 u_Kd;"                                                                                                                    \
            "uniform vec3 u_Ks;"                                                                                                                    \
            "uniform float u_materialShininess;"                                                                                                    \
            "uniform int u_LKeyPressed;"                                                                                                            \
            "out vec4 fragColor;"                                                                                                                   \
            "void main(void)"                                                                                                                       \
            "{"                                                                                                                                     \
            "   vec3 phong_ads_light = vec3(0.0f, 0.0f, 0.0f);"                                                                                     \
            "   if(u_LKeyPressed == 1)"                                                                                                             \
            "   {"                                                                                                                                  \
            "       vec3 normalized_transformed_normal = normalize(transformed_normal);"                                                            \
            "       vec3 normalized_view_vector = normalize(view_vector);"                                                                          \
            "       vec3 normalized_light_direction[3];"                                                                                            \
            "       vec3 reflection_vector[3];"                                                                                                     \
            "       vec3 ambient[3];"                                                                                                               \
            "       vec3 diffuse[3];"                                                                                                               \
            "       vec3 specular[3];"                                                                                                              \
            "       for(int i = 0; i < 3; i++)"                                                                                                     \
            "       {"                                                                                                                              \
            "           normalized_light_direction[i] = normalize(light_direction[i]);"                                                             \
            "           reflection_vector[i] = reflect(-normalized_light_direction[i], normalized_transformed_normal);"                             \
            "           ambient[i] = u_La[i] * u_Ka;"                                                                                               \
            "           diffuse[i] = u_Ld[i] * u_Kd * max(dot(normalized_light_direction[i], normalized_transformed_normal), 0.0f);"                \
            "           specular[i] = u_Ls[i] * u_Ks * pow(max(dot(reflection_vector[i], normalized_view_vector), 0.0f), u_materialShininess);"     \
            "           phong_ads_light = phong_ads_light + ambient[i] + diffuse[i] + specular[i];"                                                 \
            "       }"                                                                                                                              \
            "   }"                                                                                                                                  \
            "   else"                                                                                                                               \
            "   {"                                                                                                                                  \
            "       phong_ads_light = vec3(1.0f, 1.0f, 1.0f);"                                                                                      \
            "   }"                                                                                                                                  \
            "   fragColor = vec4(phong_ads_light, 1.0f);"                                                                                           \
            "}";
        
        //provide source code to fragment shader object
        glShaderSource(pf_fragmentShaderObject, 1, (const GLchar**)&pf_fragmentShaderSource, NULL);
        
        //compile shader
        glCompileShader(pf_fragmentShaderObject);
        
        //shader compilation error checking
        glGetShaderiv(pf_fragmentShaderObject, GL_COMPILE_STATUS, &shaderCompiledStatus);
        if(shaderCompiledStatus == GL_FALSE)
        {
            glGetShaderiv(pf_fragmentShaderObject, GL_INFO_LOG_LENGTH, &infoLogLength);
            if(infoLogLength > 0)
            {
                szInfoLog = (GLchar*)malloc(sizeof(GLchar) * infoLogLength);
                if(szInfoLog != NULL)
                {
                    GLsizei written;
                    glGetShaderInfoLog(pf_fragmentShaderObject, infoLogLength, &written, szInfoLog);
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
        pf_shaderProgramObject = glCreateProgram();
        
        //attach shaders to program object
        glAttachShader(pf_shaderProgramObject, pf_vertexShaderObject);
        glAttachShader(pf_shaderProgramObject, pf_fragmentShaderObject);
        
        //bind shader program object with vertex shader attributes
        glBindAttribLocation(pf_shaderProgramObject, AMC_ATTRIBUTE_POSITION, "vPositon");
        glBindAttribLocation(pf_shaderProgramObject, AMC_ATTRIBUTE_NORMAL, "vNormal");
        
        //link program
        glLinkProgram(pf_shaderProgramObject);
        
        //shader linking error chechking
        glGetProgramiv(pf_shaderProgramObject, GL_LINK_STATUS, &shaderProgramLinkStatus);
        if(shaderProgramLinkStatus == GL_FALSE)
        {
            glGetProgramiv(pf_shaderProgramObject, GL_INFO_LOG_LENGTH, &infoLogLength);
            if(infoLogLength > 0)
            {
                szInfoLog = (GLchar*)malloc(sizeof(GLchar) * infoLogLength);
                if(szInfoLog != NULL)
                {
                    GLsizei written;
                    glGetProgramInfoLog(pf_shaderProgramObject, infoLogLength, &written, szInfoLog);
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
        pf_modelMatrixUniform = glGetUniformLocation(pf_shaderProgramObject, "u_modelMatrix");
        pf_viewMatrixUniform = glGetUniformLocation(pf_shaderProgramObject, "u_viewMatrix");
        pf_projectionMatrixUniform = glGetUniformLocation(pf_shaderProgramObject, "u_perspectiveProjectionMatrix");

        pf_lightAmbientUniform[0] = glGetUniformLocation(pf_shaderProgramObject, "u_La[0]");
        pf_lightDiffuseUniform[0] = glGetUniformLocation(pf_shaderProgramObject, "u_Ld[0]");
        pf_lightSpecularUniform[0] = glGetUniformLocation(pf_shaderProgramObject, "u_Ls[0]");
        pf_lightPositionUniform[0] = glGetUniformLocation(pf_shaderProgramObject, "u_lightPosition[0]");

        pf_lightAmbientUniform[1] = glGetUniformLocation(pf_shaderProgramObject, "u_La[1]");
        pf_lightDiffuseUniform[1] = glGetUniformLocation(pf_shaderProgramObject, "u_Ld[1]");
        pf_lightSpecularUniform[1] = glGetUniformLocation(pf_shaderProgramObject, "u_Ls[1]");
        pf_lightPositionUniform[1] = glGetUniformLocation(pf_shaderProgramObject, "u_lightPosition[1]");

        pf_lightAmbientUniform[2] = glGetUniformLocation(pf_shaderProgramObject, "u_La[2]");
        pf_lightDiffuseUniform[2] = glGetUniformLocation(pf_shaderProgramObject, "u_Ld[2]");
        pf_lightSpecularUniform[2] = glGetUniformLocation(pf_shaderProgramObject, "u_Ls[2]");
        pf_lightPositionUniform[2] = glGetUniformLocation(pf_shaderProgramObject, "u_lightPosition[2]");

        pf_materialAmbientUniform = glGetUniformLocation(pf_shaderProgramObject, "u_Ka");
        pf_materialDiffuseUniform = glGetUniformLocation(pf_shaderProgramObject, "u_Kd");
        pf_materialSpecularUniform = glGetUniformLocation(pf_shaderProgramObject, "u_Ks");
        pf_materialShininessUniform = glGetUniformLocation(pf_shaderProgramObject, "u_materialShininess");
        pf_keyPressedUniform = glGetUniformLocation(pf_shaderProgramObject, "u_LKeyPressed");

        //set up input data
        float sphere_vertices[1146];
        float sphere_normals[1146];
        float sphere_textures[764];
        short sphere_elements[2280];

        Sphere *sphere = [[Sphere alloc] init];
        [sphere getSphereVertexData:sphere_vertices :sphere_normals :sphere_textures :sphere_elements];
        numVertices = [sphere getNumberOfSphereVertices];
        numElements = [sphere getNumberOfSphereElements];
        
        //set up vao and vbo
        glGenVertexArrays(1, &vao_sphere);
        glBindVertexArray(vao_sphere);
            glGenBuffers(1, &vbo_sphere_position);
            glBindBuffer(GL_ARRAY_BUFFER, vbo_sphere_position);
                glBufferData(GL_ARRAY_BUFFER, sizeof(sphere_vertices), sphere_vertices, GL_STATIC_DRAW);
                glVertexAttribPointer(AMC_ATTRIBUTE_POSITION, 3, GL_FLOAT, GL_FALSE, 0, NULL);
                glEnableVertexAttribArray(AMC_ATTRIBUTE_POSITION);
            glBindBuffer(GL_ARRAY_BUFFER, 0);

            glGenBuffers(1, &vbo_sphere_normal);
            glBindBuffer(GL_ARRAY_BUFFER, vbo_sphere_normal);
                glBufferData(GL_ARRAY_BUFFER, sizeof(sphere_normals), sphere_normals, GL_STATIC_DRAW);
                glVertexAttribPointer(AMC_ATTRIBUTE_NORMAL, 3, GL_FLOAT, GL_FALSE, 0, NULL);
                glEnableVertexAttribArray(AMC_ATTRIBUTE_NORMAL);
            glBindBuffer(GL_ARRAY_BUFFER, 0);

            glGenBuffers(1, &vbo_sphere_indices);
            glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, vbo_sphere_indices);
            glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(sphere_elements), sphere_elements, GL_STATIC_DRAW);
        glBindVertexArray(0);
        
        [sphere release];
        
        //set OpenGL states
        glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
        glClearDepth(1.0f);
        glEnable(GL_DEPTH_TEST);
        glDepthFunc(GL_LEQUAL);
        
        //set light and material variables
        light[0].lightAmbient = vmath::vec3(0.0f, 0.0f, 0.0f, 1.0f);
        light[0].lightDiffuse = vmath::vec3(1.0f, 0.0f, 0.0f, 1.0f);
        light[0].lightSpecular = vmath::vec3(1.0f, 0.0f, 0.0f, 1.0f);
        light[0].lightPosition = vmath::vec4(0.0f, 0.0f, 0.0f, 1.0f);

        light[1].lightAmbient = vmath::vec3(0.0f, 0.0f, 0.0f, 1.0f);
        light[1].lightDiffuse = vmath::vec3(0.0f, 1.0f, 0.0f, 1.0f);
        light[1].lightSpecular = vmath::vec3(0.0f, 1.0f, 0.0f, 1.0f);
        light[1].lightPosition = vmath::vec4(0.0f, 0.0f, 0.0f, 1.0f);

        light[2].lightAmbient = vmath::vec3(0.0f, 0.0f, 0.0f, 1.0f);
        light[2].lightDiffuse = vmath::vec3(0.0f, 0.0f, 1.0f, 1.0f);
        light[2].lightSpecular = vmath::vec3(0.0f, 0.0f, 1.0f, 1.0f);
        light[2].lightPosition = vmath::vec4(0.0f, 0.0f, 0.0f, 1.0f);

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

        const GLfloat radius = 5.0f;
        static GLfloat lightAngle = 0.0f;

        //code
        [[self openGLContext] makeCurrentContext];
        CGLLockContext((CGLContextObj)[[self openGLContext] CGLContextObj]);
        
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
            modelMatrix = vmath::translate(0.0f, 0.0f, -3.0f);
            viewMatrix = vmath::mat4::identity();

            //set light position
            light[0].lightPosition = vmath::vec4(0.0f, radius * sinf(lightAngle), radius * cosf(lightAngle), 1.0f);
            light[1].lightPosition = vmath::vec4(radius * sinf(lightAngle), 0.0f, radius * cosf(lightAngle), 1.0f);
            light[2].lightPosition = vmath::vec4(radius * sinf(lightAngle), radius * cosf(lightAngle), 0.0f, 1.0f);

            if(toggle == 0)
            {
                glUseProgram(pv_shaderProgramObject);
                
                glUniformMatrix4fv(pv_modelMatrixUniform, 1, GL_FALSE, modelMatrix);
                glUniformMatrix4fv(pv_viewMatrixUniform, 1, GL_FALSE, viewMatrix);
                glUniformMatrix4fv(pv_projectionMatrixUniform, 1, GL_FALSE, projectionMatrix);

                glUniform1i(pv_keyPressedUniform, key_pressed);

                if(key_pressed)
                {
                    glUniform3fv(pv_lightAmbientUniform[0], 1, light.lightAmbient[0]);
                    glUniform3fv(pv_lightDiffuseUniform[0], 1, light.lightDiffuse[0]);
                    glUniform3fv(pv_lightSpecularUniform[0], 1, light.lightSpecular[0]);
                    glUniform4fv(pv_lightPositionUniform[0], 1, light.lightPosition[0]);

                    glUniform3fv(pv_lightAmbientUniform[1], 1, light.lightAmbient[1]);
                    glUniform3fv(pv_lightDiffuseUniform[1], 1, light.lightDiffuse[1]);
                    glUniform3fv(pv_lightSpecularUniform[1], 1, light.lightSpecular[1]);
                    glUniform4fv(pv_lightPositionUniform[1], 1, light.lightPosition[1]);

                    glUniform3fv(pv_lightAmbientUniform[2], 1, light.lightAmbient[2]);
                    glUniform3fv(pv_lightDiffuseUniform[2], 1, light.lightDiffuse[2]);
                    glUniform3fv(pv_lightSpecularUniform[2], 1, light.lightSpecular[2]);
                    glUniform4fv(pv_lightPositionUniform[2], 1, light.lightPosition[2]);

                    glUniform3fv(pv_materialAmbientUniform, 1, material.materialAmbient);
                    glUniform3fv(pv_materialDiffuseUniform, 1, material.materialDiffuse);
                    glUniform3fv(pv_materialSpecularUniform, 1, material.materialSpecular);
                    glUniform1f(pv_materialShininessUniform, material.materialShininess);
                }
            }
            else
            {
                glUseProgram(pf_shaderProgramObject);
                
                glUniformMatrix4fv(pf_modelMatrixUniform, 1, GL_FALSE, modelMatrix);
                glUniformMatrix4fv(pf_viewMatrixUniform, 1, GL_FALSE, viewMatrix);
                glUniformMatrix4fv(pf_projectionMatrixUniform, 1, GL_FALSE, projectionMatrix);

                glUniform1i(pf_keyPressedUniform, key_pressed);

                if(key_pressed)
                {
                    glUniform3fv(pf_lightAmbientUniform[0], 1, light.lightAmbient[0]);
                    glUniform3fv(pf_lightDiffuseUniform[0], 1, light.lightDiffuse[0]);
                    glUniform3fv(pf_lightSpecularUniform[0], 1, light.lightSpecular[0]);
                    glUniform4fv(pf_lightPositionUniform[0], 1, light.lightPosition[0]);

                    glUniform3fv(pf_lightAmbientUniform[1], 1, light.lightAmbient[1]);
                    glUniform3fv(pf_lightDiffuseUniform[1], 1, light.lightDiffuse[1]);
                    glUniform3fv(pf_lightSpecularUniform[1], 1, light.lightSpecular[1]);
                    glUniform4fv(pf_lightPositionUniform[1], 1, light.lightPosition[1]);

                    glUniform3fv(pf_lightAmbientUniform[2], 1, light.lightAmbient[2]);
                    glUniform3fv(pf_lightDiffuseUniform[2], 1, light.lightDiffuse[2]);
                    glUniform3fv(pf_lightSpecularUniform[2], 1, light.lightSpecular[2]);
                    glUniform4fv(pf_lightPositionUniform[2], 1, light.lightPosition[2]);

                    glUniform3fv(pf_materialAmbientUniform, 1, material.materialAmbient);
                    glUniform3fv(pf_materialDiffuseUniform, 1, material.materialDiffuse);
                    glUniform3fv(pf_materialSpecularUniform, 1, material.materialSpecular);
                    glUniform1f(pf_materialShininessUniform, material.materialShininess);
                }
            }

            glBindVertexArray(vao_sphere);
            glDrawElements(GL_TRIANGLES, numElements, GL_UNSIGNED_SHORT, NULL);
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
            
            case 'W':
            case 'w':
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

            case 'V':
            case 'v':
                toggle = 0;
                break;

            case 'F':
            case 'f':
                toggle = 1;
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
        if(vbo_sphere_indices)
        {
            glDeleteBuffers(1, &vbo_sphere_indices);
            vbo_sphere_indices = 0;
        }

        if(vbo_sphere_normal)
        {
            glDeleteBuffers(1, &vbo_sphere_normal);
            vbo_sphere_normal = 0;
        }

        if(vbo_sphere_position)
        {
            glDeleteBuffers(1, &vbo_sphere_position);
            vbo_sphere_position = 0;
        }
    
        if(vao_sphere)
        {
            glDeleteVertexArrays(1, &vao_sphere);
            vao_sphere = 0;
        }
        
        //safe shader cleanup
        if(pv_shaderProgramObject)
        {
            GLsizei shader_count;
            GLuint* p_shaders = NULL;

            glUseProgram(pv_shaderProgramObject);
            glGetProgramiv(pv_shaderProgramObject, GL_ATTACHED_SHADERS, &shader_count);

            p_shaders = (GLuint*)malloc(shader_count * sizeof(GLuint));
            memset((void*)p_shaders, 0, shader_count * sizeof(GLuint));

            glGetAttachedShaders(pv_shaderProgramObject, shader_count, &shader_count, p_shaders);

            for(GLsizei i = 0; i < shader_count; i++)
            {
                glDetachShader(pv_shaderProgramObject, p_shaders[i]);
                glDeleteShader(p_shaders[i]);
                p_shaders[i] = 0;
            }

            free(p_shaders);
            p_shaders = NULL;
        
            glDeleteProgram(pv_shaderProgramObject);
            pv_shaderProgramObject = 0;
            glUseProgram(0);
        }
        
        if(pf_shaderProgramObject)
        {
            GLsizei shader_count;
            GLuint* p_shaders = NULL;

            glUseProgram(pf_shaderProgramObject);
            glGetProgramiv(pf_shaderProgramObject, GL_ATTACHED_SHADERS, &shader_count);

            p_shaders = (GLuint*)malloc(shader_count * sizeof(GLuint));
            memset((void*)p_shaders, 0, shader_count * sizeof(GLuint));

            glGetAttachedShaders(pf_shaderProgramObject, shader_count, &shader_count, p_shaders);

            for(GLsizei i = 0; i < shader_count; i++)
            {
                glDetachShader(pf_shaderProgramObject, p_shaders[i]);
                glDeleteShader(p_shaders[i]);
                p_shaders[i] = 0;
            }

            free(p_shaders);
            p_shaders = NULL;
        
            glDeleteProgram(pf_shaderProgramObject);
            pf_shaderProgramObject = 0;
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

