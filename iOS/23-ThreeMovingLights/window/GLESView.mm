
//
//  MyView.m
//  window
//
//  Created by Akshay Patel on 9/24/21.
//

#import <OpenGLES/ES3/gl.h>
#import <OpenGLES/ES3/glext.h>
#import "GLESView.h"
#import "vmath.h"
#import "Sphere.h"

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

@implementation GLESView
{
    EAGLContext *eagl_context;
    
    GLuint default_framebuffer;
    GLuint color_renderbuffer;
    GLuint depth_renderbuffer;

    id display_link;
    NSInteger animation_frame_interval;
    bool is_animating;

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
    
    //log OpenGL info
    printf("-> OpenGL Info\n");
    printf("   Vendor : %s\n", glGetString(GL_VENDOR));
    printf("   Renderer : %s\n", glGetString(GL_RENDERER));
    printf("   Version : %s\n", glGetString(GL_VERSION));
    printf("   GLSL Version : %s\n", glGetString(GL_SHADING_LANGUAGE_VERSION));
    printf("-------------------------------------------------------------\n");
    
    //vertex shader
    pv_vertexShaderObject = glCreateShader(GL_VERTEX_SHADER);
    
    const GLchar *pv_vertexShaderSource =
        "#version 300 es"                                                                                                         \
        "\n"                                                                                                                        \
        "precision highp int;"                                                                                                      \

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
                printf("-> vertex shader compilation log : %s\n", szInfoLog);
                printf("-------------------------------------------------------------\n");
                free(szInfoLog);
                [self release];
                return (nil);
            }
        }
    }
    printf("-> vertex shader compiled successfully\n");
    printf("-------------------------------------------------------------\n");
    
    //fragment shader
    pv_fragmentShaderObject = glCreateShader(GL_FRAGMENT_SHADER);
    
    const GLchar *pv_fragmentShaderSource =
        "#version 300 es"                                 \
        "\n"                                                \
        "precision highp int;"                                                                                                      \
        "precision highp float;"                                                                                                      \

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
                printf("-> fragment shader compilation log : %s\n", szInfoLog);
                printf("-------------------------------------------------------------\n");
                free(szInfoLog);
                [self release];
                return (nil);
            }
        }
    }
    printf("-> fragment shader compiled successfully\n");
    printf("-------------------------------------------------------------\n");
    
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
                printf("-> shader program link log : %s\n", szInfoLog);
                free(szInfoLog);
                [self release];
                return (nil);
            }
        }
    }
    printf("-> shader program linked successfully\n");
    printf("-------------------------------------------------------------\n");
        
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
    printf("-> per-fragment lighting\n");
    printf("-------------------------------------------------------------\n");

    //vertex shader
    pf_vertexShaderObject = glCreateShader(GL_VERTEX_SHADER);
    
    const GLchar *pf_vertexShaderSource =
        "#version 300 es"                                                                                     \
        "\n"                                                                                                    \
        "precision highp int;"                                                                                                      \

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
                printf("-> vertex shader compilation log : %s\n", szInfoLog);
                printf("-------------------------------------------------------------\n");
                free(szInfoLog);
                [self release];
                return (nil);
            }
        }
    }
    printf("-> vertex shader compiled successfully\n");
    printf("-------------------------------------------------------------\n");
    
    //fragment shader
    pf_fragmentShaderObject = glCreateShader(GL_FRAGMENT_SHADER);
    
    const GLchar *pf_fragmentShaderSource =
        "#version 300 es"                                                                                                                     \
        "\n"                                                                                                                                    \
        "precision highp int;"                                                                                                      \
        "precision highp float;"                                                                                                      \

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
                printf("-> fragment shader compilation log : %s\n", szInfoLog);
                printf("-------------------------------------------------------------\n");
                free(szInfoLog);
                [self release];
                return (nil);
            }
        }
    }
    printf("-> fragment shader compiled successfully\n");
    printf("-------------------------------------------------------------\n");
    
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
                printf("-> shader program link log : %s\n", szInfoLog);
                free(szInfoLog);
                [self release];
                return (nil);
            }
        }
    }
    printf("-> shader program linked successfully\n");
    printf("-------------------------------------------------------------\n");
        
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
    glEnable(GL_DEPTH_TEST);
    
    //set light and material variables
    light[0].lightAmbient = vmath::vec3(0.0f, 0.0f, 0.0f);
    light[0].lightDiffuse = vmath::vec3(1.0f, 0.0f, 0.0f);
    light[0].lightSpecular = vmath::vec3(1.0f, 0.0f, 0.0f);
    light[0].lightPosition = vmath::vec4(0.0f, 0.0f, 0.0f, 1.0f);

    light[1].lightAmbient = vmath::vec3(0.0f, 0.0f, 0.0f);
    light[1].lightDiffuse = vmath::vec3(0.0f, 1.0f, 0.0f);
    light[1].lightSpecular = vmath::vec3(0.0f, 1.0f, 0.0f);
    light[1].lightPosition = vmath::vec4(0.0f, 0.0f, 0.0f, 1.0f);

    light[2].lightAmbient = vmath::vec3(0.0f, 0.0f, 0.0f);
    light[2].lightDiffuse = vmath::vec3(0.0f, 0.0f, 1.0f);
    light[2].lightSpecular = vmath::vec3(0.0f, 0.0f, 1.0f);
    light[2].lightPosition = vmath::vec4(0.0f, 0.0f, 0.0f, 1.0f);

    material.materialAmbient = vmath::vec3(0.0f, 0.0f, 0.0f);
    material.materialDiffuse = vmath::vec3(1.0f, 1.0f, 1.0f);
    material.materialSpecular = vmath::vec3(1.0f, 1.0f, 1.0f);
    material.materialShininess = 50.0f;
    
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
    
    projectionMatrix = vmath::perspective(45.0f, width / height, 0.1f, 100.0f);
    
    [self drawView];
}

- (void)drawView
{
    //variable declarations
    vmath::mat4 modelMatrix;
    vmath::mat4 viewMatrix;

    const GLfloat radius = 5.0f;
    static GLfloat lightAngle = 0.0f;
    
    //code
    [EAGLContext setCurrentContext:eagl_context];
    glBindFramebuffer(GL_FRAMEBUFFER, default_framebuffer);
    glBindRenderbuffer(GL_RENDERBUFFER, color_renderbuffer);

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
            glUniform3fv(pv_lightAmbientUniform[0], 1, light[0].lightAmbient);
            glUniform3fv(pv_lightDiffuseUniform[0], 1, light[0].lightDiffuse);
            glUniform3fv(pv_lightSpecularUniform[0], 1, light[0].lightSpecular);
            glUniform4fv(pv_lightPositionUniform[0], 1, light[0].lightPosition);

            glUniform3fv(pv_lightAmbientUniform[1], 1, light[1].lightAmbient);
            glUniform3fv(pv_lightDiffuseUniform[1], 1, light[1].lightDiffuse);
            glUniform3fv(pv_lightSpecularUniform[1], 1, light[1].lightSpecular);
            glUniform4fv(pv_lightPositionUniform[1], 1, light[1].lightPosition);

            glUniform3fv(pv_lightAmbientUniform[2], 1, light[2].lightAmbient);
            glUniform3fv(pv_lightDiffuseUniform[2], 1, light[2].lightDiffuse);
            glUniform3fv(pv_lightSpecularUniform[2], 1, light[2].lightSpecular);
            glUniform4fv(pv_lightPositionUniform[2], 1, light[2].lightPosition);

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
            glUniform3fv(pf_lightAmbientUniform[0], 1, light[0].lightAmbient);
            glUniform3fv(pf_lightDiffuseUniform[0], 1, light[0].lightDiffuse);
            glUniform3fv(pf_lightSpecularUniform[0], 1, light[0].lightSpecular);
            glUniform4fv(pf_lightPositionUniform[0], 1, light[0].lightPosition);

            glUniform3fv(pf_lightAmbientUniform[1], 1, light[1].lightAmbient);
            glUniform3fv(pf_lightDiffuseUniform[1], 1, light[1].lightDiffuse);
            glUniform3fv(pf_lightSpecularUniform[1], 1, light[1].lightSpecular);
            glUniform4fv(pf_lightPositionUniform[1], 1, light[1].lightPosition);

            glUniform3fv(pf_lightAmbientUniform[2], 1, light[2].lightAmbient);
            glUniform3fv(pf_lightDiffuseUniform[2], 1, light[2].lightDiffuse);
            glUniform3fv(pf_lightSpecularUniform[2], 1, light[2].lightSpecular);
            glUniform4fv(pf_lightPositionUniform[2], 1, light[2].lightPosition);

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


//update
lightAngle += 0.1f;
if(lightAngle >= 360.0f)
    lightAngle = 0.0f;
    
    [eagl_context presentRenderbuffer:color_renderbuffer];
    
    glBindRenderbuffer(GL_RENDERBUFFER, 0);
    glBindFramebuffer(GL_FRAMEBUFFER, 0);
}

- (void) uninitialize
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
    if(key_pressed)
    {
        key_pressed = 0;
    }
    else
    {
        key_pressed = 1;
    }
    
    [self setNeedsDisplay];
}

- (void)onDoubleTap:(UITapGestureRecognizer*)gr
{
    //code
    if(toggle)
    {
        toggle = 0;
    }
    else
    {
        toggle  = 1;
    }
    
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


