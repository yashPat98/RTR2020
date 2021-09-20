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

//shader program variables
GLuint vertexShaderObject;
GLuint fragmentShaderObject;
GLuint shaderProgramObject;

//unioform location variables
GLuint modelMatrixUniform;
GLuint viewMatrixUniform;
GLuint projectionMatrixUniform;
GLuint lightAmbientUniform;
GLuint lightDiffuseUniform;
GLuint lightSpecularUniform;
GLuint lightPositionUniform;
GLuint materialAmbientUniform;
GLuint materialDiffuseUniform;
GLuint materialSpecularUniform;
GLuint materialShininessUniform;
GLuint keyPressedUniform;

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
        [window setTitle:@"OpenGL : Per Vertex Shading"];
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
        glBindAttribLocation(shaderProgramObject, AMC_ATTRIBUTE_NORMAL, "a_normal");
        
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
        modelMatrixUniform = glGetUniformLocation(shaderProgramObject, "model_matrix");
        viewMatrixUniform = glGetUniformLocation(shaderProgramObject, "view_matrix");
        projectionMatrixUniform = glGetUniformLocation(shaderProgramObject, "projection_matrix");
        lightAmbientUniform = glGetUniformLocation(shaderProgramObject, "light_ambient");
        lightDiffuseUniform = glGetUniformLocation(shaderProgramObject, "light_diffuse");
        lightSpecularUniform = glGetUniformLocation(shaderProgramObject, "light_specular");
        lightPositionUniform = glGetUniformLocation(shaderProgramObject, "light_position");
        materialAmbientUniform = glGetUniformLocation(shaderProgramObject, "material_ambient");
        materialDiffuseUniform = glGetUniformLocation(shaderProgramObject, "material_diffuse");
        materialSpecularUniform = glGetUniformLocation(shaderProgramObject, "material_specular");
        materialShininessUniform = glGetUniformLocation(shaderProgramObject, "material_shininess");
        keyPressedUniform = glGetUniformLocation(shaderProgramObject, "key_pressed");

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

        glUseProgram(shaderProgramObject);
            glUniformMatrix4fv(modelMatrixUniform, 1, GL_FALSE, modelMatrix);
            glUniformMatrix4fv(viewMatrixUniform, 1, GL_FALSE, viewMatrix);
            glUniformMatrix4fv(projectionMatrixUniform, 1, GL_FALSE, projectionMatrix);

            glUniform1i(keyPressedUniform, key_pressed);

            if(key_pressed)
            {
                glUniform3fv(lightAmbientUniform, 1, light_ambient);
                glUniform3fv(lightDiffuseUniform, 1, light_diffuse);
                glUniform3fv(lightSpecularUniform, 1, light_specular);
                glUniform4fv(lightPositionUniform, 1, light_position);

                glUniform3fv(materialAmbientUniform, 1, material_ambient);
                glUniform3fv(materialDiffuseUniform, 1, material_diffuse);
                glUniform3fv(materialSpecularUniform, 1, material_specular);
                glUniform1f(materialShininessUniform, material_shininess);
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

