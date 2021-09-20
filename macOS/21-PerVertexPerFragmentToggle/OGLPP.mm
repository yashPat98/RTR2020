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
GLuint pv_lightAmbientUniform;
GLuint pv_lightDiffuseUniform;
GLuint pv_lightSpecularUniform;
GLuint pv_lightPositionUniform;
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
GLuint pf_lightAmbientUniform;
GLuint pf_lightDiffuseUniform;
GLuint pf_lightSpecularUniform;
GLuint pf_lightPositionUniform;
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
vmath::vec3 light_ambient;
vmath::vec3 light_diffuse;
vmath::vec3 light_specular;
vmath::vec4 light_position;
vmath::vec3 material_ambient;
vmath::vec3 material_diffuse;
vmath::vec3 material_specular;
float material_shininess;
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
        [window setTitle:@"OpenGL : Per Vertex Per Fragment Toggle"];
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
                        "#version 410 core"                                                                                                                         \
            "\n"                                                                                                                                        \
            
            "in vec3 a_position;"                                                                                                                       \
            "in vec3 a_normal;"                                                                                                                         \
            "out vec3 phong_ads_light;"                                                                                                                 \

            "uniform mat4 model_matrix;"                                                                                                                \
            "uniform mat4 view_matrix;"                                                                                                                 \
            "uniform mat4 projection_matrix;"                                                                                                           \
            "uniform vec3 light_ambient;"                                                                                                               \
            "uniform vec3 light_diffuse;"                                                                                                               \
            "uniform vec3 light_specular;"                                                                                                              \
            "uniform vec4 light_position;"                                                                                                              \
            "uniform vec3 material_ambient;"                                                                                                            \
            "uniform vec3 material_diffuse;"                                                                                                            \
            "uniform vec3 material_specular;"                                                                                                           \
            "uniform float material_shininess;"                                                                                                          \
            "uniform int key_pressed;"                                                                                                                  \

            "void main(void)"                                                                                                                           \
            "{"                                                                                                                                         \
            "   if(key_pressed == 1)"                                                                                                                   \
            "   {"                                                                                                                                      \
            "       vec4 eye_coords = view_matrix * model_matrix * vec4(a_position, 1.0f);"                                                                         \
            "       mat3 normal_matrix = mat3(transpose(inverse(view_matrix * model_matrix)));"                                                         \
            "       vec3 transformed_normal = normalize(normal_matrix * a_normal);"                                                                     \
            "       vec3 light_direction = normalize(vec3(light_position - eye_coords));"                                                               \
            "       vec3 reflection_vector = reflect(-light_direction, transformed_normal);"                                                            \
            "       vec3 view_vector = normalize(-eye_coords.xyz);"                                                                                     \
            
            "       vec3 ambient = light_ambient * material_ambient;"                                                                                   \
            "       vec3 diffuse = light_diffuse * material_diffuse * max(dot(light_direction, transformed_normal), 0.0f);"                             \
            "       vec3 specular = light_specular * material_specular * pow(max(dot(reflection_vector, view_vector), 0.0f), material_shininess);"      \
            
            "       phong_ads_light = ambient + diffuse + specular;"                                                                                    \
            "   }"                                                                                                                                      \
            "   else"                                                                                                                                   \
            "   {"                                                                                                                                      \
            "       phong_ads_light = vec3(1.0f, 1.0f, 1.0f);"                                                                                          \
            "   }"                                                                                                                                      \

            "   gl_Position = projection_matrix * view_matrix * model_matrix * vec4(a_position, 1.0f);"                                                             \
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
            "#version 410 core"                                         \
            "\n"                                                        \
            
            "in vec3 phong_ads_light;"                                  \
            "out vec4 frag_color;"                                      \
        
            "void main(void)"                                           \
            "{"                                                         \
            "   frag_color = vec4(phong_ads_light, 1.0f);"              \
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
        glBindAttribLocation(pv_shaderProgramObject, AMC_ATTRIBUTE_POSITION, "a_position");
        glBindAttribLocation(pv_shaderProgramObject, AMC_ATTRIBUTE_NORMAL, "a_normal");
        
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
        pv_modelMatrixUniform = glGetUniformLocation(pv_shaderProgramObject, "model_matrix");
        pv_viewMatrixUniform = glGetUniformLocation(pv_shaderProgramObject, "view_matrix");
        pv_projectionMatrixUniform = glGetUniformLocation(pv_shaderProgramObject, "projection_matrix");
        pv_lightAmbientUniform = glGetUniformLocation(pv_shaderProgramObject, "light_ambient");
        pv_lightDiffuseUniform = glGetUniformLocation(pv_shaderProgramObject, "light_diffuse");
        pv_lightSpecularUniform = glGetUniformLocation(pv_shaderProgramObject, "light_specular");
        pv_lightPositionUniform = glGetUniformLocation(pv_shaderProgramObject, "light_position");
        pv_materialAmbientUniform = glGetUniformLocation(pv_shaderProgramObject, "material_ambient");
        pv_materialDiffuseUniform = glGetUniformLocation(pv_shaderProgramObject, "material_diffuse");
        pv_materialSpecularUniform = glGetUniformLocation(pv_shaderProgramObject, "material_specular");
        pv_materialShininessUniform = glGetUniformLocation(pv_shaderProgramObject, "material_shininess");
        pv_keyPressedUniform = glGetUniformLocation(pv_shaderProgramObject, "key_pressed");

        //per-fragment lighting
        fprintf(gpFile, "-> per-fragment lighting\n");
        fprintf(gpFile, "-------------------------------------------------------------\n");

        //vertex shader
        pf_vertexShaderObject = glCreateShader(GL_VERTEX_SHADER);
        
        const GLchar *pf_vertexShaderSource =
        "#version 410 core"                                                                                     \
            "\n"                                                                                                    \
            
            "in vec4 a_position;"                                                                                   \
            "in vec3 a_normal;"                                                                                     \
            "out vec3 transformed_normal;"                                                                          \
            "out vec3 light_direction;"                                                                             \
            "out vec3 view_vector;"                                                                                 \

            "uniform mat4 model_matrix;"                                                                            \
            "uniform mat4 view_matrix;"                                                                             \
            "uniform mat4 projection_matrix;"                                                                       \
            "uniform vec4 light_position;"
            "uniform int key_pressed;"                                                                              \
            
            "void main(void)"                                                                                       \
            "{"                                                                                                     \
            "   if(key_pressed == 1)"                                                                               \
            "   {"                                                                                                  \
            "       vec4 eye_coords = view_matrix * model_matrix * a_position;"                                     \
            "       mat3 normal_matrix = mat3(transpose(inverse(view_matrix * model_matrix))); "                    \
            "       transformed_normal = normal_matrix * a_normal;"                                                 \
            "       light_direction = vec3(light_position - eye_coords);"                                           \
            "       view_vector = -eye_coords.xyz;"                                                                 \
            "   }"                                                                                                  \
            "   gl_Position = projection_matrix * view_matrix * model_matrix * a_position;"                         \
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
            "#version 410 core"                                                                                                                                 \
            "\n"                                                                                                                                                \
            
            "in vec3 transformed_normal;"                                                                                                                       \
            "in vec3 light_direction;"                                                                                                                          \
            "in vec3 view_vector;"                                                                                                                              \
            "out vec4 fragColor;"                                                                                                                               \

            "uniform vec3 light_ambient;"                                                                                                                       \
            "uniform vec3 light_diffuse;"                                                                                                                       \
            "uniform vec3 light_specular;"                                                                                                                      \
            "uniform vec3 material_ambient;"                                                                                                                    \
            "uniform vec3 material_diffuse;"                                                                                                                    \
            "uniform vec3 material_specular;"                                                                                                                   \
            "uniform float material_shininess;"                                                                                                                 \
            "uniform int key_pressed;"                                                                                                                          \
            
            "void main(void)"                                                                                                                                   \
            "{"                                                                                                                                                 \
            "   vec3 phong_ads_light;"                                                                                                                          \
            "   if(key_pressed == 1)"                                                                                                                           \
            "   {"                                                                                                                                              \
            "       vec3 normalized_transformed_normal = normalize(transformed_normal);"                                                                        \
            "       vec3 normalized_view_vector = normalize(view_vector);"                                                                                      \
            "       vec3 normalized_light_direction = normalize(light_direction);"                                                                              \
            "       vec3 reflection_vector = reflect(-normalized_light_direction, normalized_transformed_normal);"                                              \
            
            "       vec3 ambient = light_ambient * material_ambient;"                                                                                           \
            "       vec3 diffuse = light_diffuse * material_diffuse  * max(dot(normalized_light_direction, normalized_transformed_normal), 0.0f);"              \
            "       vec3 specular = light_specular * material_specular * pow(max(dot(reflection_vector, normalized_view_vector), 0.0f), material_shininess);"   \
            "       phong_ads_light = ambient + diffuse + specular;"                                                                                            \
            "   }"                                                                                                                                              \
            "   else"                                                                                                                                           \
            "   {"                                                                                                                                              \
            "       phong_ads_light = vec3(1.0f, 1.0f, 1.0f);"                                                                                                  \
            "   }"                                                                                                                                              \
            "   fragColor = vec4(phong_ads_light, 1.0f);"                                                                                                       \
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
        glBindAttribLocation(pf_shaderProgramObject, AMC_ATTRIBUTE_POSITION, "a_position");
        glBindAttribLocation(pf_shaderProgramObject, AMC_ATTRIBUTE_NORMAL, "a_normal");
        
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
        pf_modelMatrixUniform = glGetUniformLocation(pf_shaderProgramObject, "model_matrix");
        pf_viewMatrixUniform = glGetUniformLocation(pf_shaderProgramObject, "view_matrix");
        pf_projectionMatrixUniform = glGetUniformLocation(pf_shaderProgramObject, "projection_matrix");
        pf_lightAmbientUniform = glGetUniformLocation(pf_shaderProgramObject, "light_ambient");
        pf_lightDiffuseUniform = glGetUniformLocation(pf_shaderProgramObject, "light_diffuse");
        pf_lightSpecularUniform = glGetUniformLocation(pf_shaderProgramObject, "light_specular");
        pf_lightPositionUniform = glGetUniformLocation(pf_shaderProgramObject, "light_position");
        pf_materialAmbientUniform = glGetUniformLocation(pf_shaderProgramObject, "material_ambient");
        pf_materialDiffuseUniform = glGetUniformLocation(pf_shaderProgramObject, "material_diffuse");
        pf_materialSpecularUniform = glGetUniformLocation(pf_shaderProgramObject, "material_specular");
        pf_materialShininessUniform = glGetUniformLocation(pf_shaderProgramObject, "material_shininess");
        pf_keyPressedUniform = glGetUniformLocation(pf_shaderProgramObject, "key_pressed");

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
        light_ambient = vmath::vec3(0.0f, 0.0f, 0.0f);
        light_diffuse = vmath::vec3(1.0f, 1.0f, 1.0f);
        light_specular = vmath::vec3(1.0f, 1.0f, 1.0f);
        light_position = vmath::vec4(100.0f, 100.0f, 100.0f, 1.0f);

        material_ambient = vmath::vec3(0.0f, 0.0f, 0.0f);
        material_diffuse = vmath::vec3(1.0f, 1.0f, 1.0f);
        material_specular = vmath::vec3(1.0f, 1.0f, 1.0f);
        material_shininess = 50.0f;

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

        //code
        [[self openGLContext] makeCurrentContext];
        CGLLockContext((CGLContextObj)[[self openGLContext] CGLContextObj]);
        
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
            modelMatrix = vmath::translate(0.0f, 0.0f, -2.0f);
            viewMatrix = vmath::mat4::identity();

            if(toggle == 0)
            {
                glUseProgram(pv_shaderProgramObject);
                
                glUniformMatrix4fv(pv_modelMatrixUniform, 1, GL_FALSE, modelMatrix);
                glUniformMatrix4fv(pv_viewMatrixUniform, 1, GL_FALSE, viewMatrix);
                glUniformMatrix4fv(pv_projectionMatrixUniform, 1, GL_FALSE, projectionMatrix);

                glUniform1i(pv_keyPressedUniform, key_pressed);

                if(key_pressed)
                {
                    glUniform3fv(pv_lightAmbientUniform, 1, light_ambient);
                    glUniform3fv(pv_lightDiffuseUniform, 1, light_diffuse);
                    glUniform3fv(pv_lightSpecularUniform, 1, light_specular);
                    glUniform4fv(pv_lightPositionUniform, 1, light_position);

                    glUniform3fv(pv_materialAmbientUniform, 1, material_ambient);
                    glUniform3fv(pv_materialDiffuseUniform, 1, material_diffuse);
                    glUniform3fv(pv_materialSpecularUniform, 1, material_specular);
                    glUniform1f(pv_materialShininessUniform, material_shininess);
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
                    glUniform3fv(pf_lightAmbientUniform, 1, light_ambient);
                    glUniform3fv(pf_lightDiffuseUniform, 1, light_diffuse);
                    glUniform3fv(pf_lightSpecularUniform, 1, light_specular);
                    glUniform4fv(pf_lightPositionUniform, 1, light_position);

                    glUniform3fv(pf_materialAmbientUniform, 1, material_ambient);
                    glUniform3fv(pf_materialDiffuseUniform, 1, material_diffuse);
                    glUniform3fv(pf_materialSpecularUniform, 1, material_specular);
                    glUniform1f(pf_materialShininessUniform, material_shininess);
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

