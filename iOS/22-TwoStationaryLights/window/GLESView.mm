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
    
    //pass-through shader program
    
    //vertex shader
    vertexShaderObject = glCreateShader(GL_VERTEX_SHADER);
    
    const GLchar *vertexShaderSource =
        "#version 300 es"                                                                                                     \
        "\n"                                                                                                                    \
        "precision highp int;"                                                                                                  \
        
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
    fragmentShaderObject = glCreateShader(GL_FRAGMENT_SHADER);
    
    const GLchar *fragmentShaderSource =
        "#version 300 es"                                 \
        "\n"                                                \
        "precision highp int;"                                                                                                  \
        "precision highp float;"                                                                                                  \

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
    glEnable(GL_DEPTH_TEST);
    
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
    vmath::mat4 translationMatrix;
    vmath::mat4 rotationMatrix;

    static GLfloat pyramid_rotation_angle = 0.0f;

    //code
    [EAGLContext setCurrentContext:eagl_context];
    glBindFramebuffer(GL_FRAMEBUFFER, default_framebuffer);
    glBindRenderbuffer(GL_RENDERBUFFER, color_renderbuffer);

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
    

    [eagl_context presentRenderbuffer:color_renderbuffer];
    
    glBindRenderbuffer(GL_RENDERBUFFER, 0);
    glBindFramebuffer(GL_FRAMEBUFFER, 0);
}

- (void) uninitialize
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
    if(key_pressed == 0)
    {
        key_pressed = 1;
    }
    else
    {
        key_pressed = 0;
    }

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


