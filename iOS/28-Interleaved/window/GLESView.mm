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
    GLuint modelviewMatrixUniform;
    GLuint projectionMatrixUniform;
    GLuint lightDiffuseUniform;
    GLuint lightPositionUniform;
    GLuint textureSamplerUniform;

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
        "#version 300 es"                                                                         \
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
        "#version 300 es"                                                     \
        "\n"                                                                    \
        "precision highp float;"                                                \
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
    glEnable(GL_DEPTH_TEST);                                

    marble_texture = [self loadTextureFromBMPFile:"window.app/marble.bmp"];
    if(!marble_texture)
    {
        printf("Error : failed to load marble.bmp texture.\n");
        [self release];
        return (nil);
    }
    return (self);
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
    UIImage *bmpImage = [[UIImage alloc] initWithContentsOfFile:image_filename_with_path];
    if(!bmpImage)
        return (0);

    printf("here");
    
    //get image data
    CGImageRef cgImage = [bmpImage CGImage];
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
    vmath::mat4 modelViewMatrix;
    vmath::mat4 translateMatrix;
    vmath::mat4 rotateMatrix;

    static GLfloat cube_rotation_angle = 0.0f;

    //code
    [EAGLContext setCurrentContext:eagl_context];
    glBindFramebuffer(GL_FRAMEBUFFER, default_framebuffer);
    glBindRenderbuffer(GL_RENDERBUFFER, color_renderbuffer);

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

        glBindVertexArray(vao);
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

    [eagl_context presentRenderbuffer:color_renderbuffer];
    
    glBindRenderbuffer(GL_RENDERBUFFER, 0);
    glBindFramebuffer(GL_FRAMEBUFFER, 0);
}

- (void) uninitialize
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
    if(bAnimate)
    {
        bAnimate = false;
    }
    else
    {
        bAnimate = true;
    }

    if(bColor)
    {
        bColor = false;
    }
    else
    {
        bColor = true;
    }

    [self setNeedsDisplay];
}

- (void)onDoubleTap:(UITapGestureRecognizer*)gr
{
    //code
    if(bTexture)
    {
        bTexture = false;
    }
    else
    {
        bTexture = true;
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
    if(bLight)
    {
        bLight = false;
    }
    else
    {
        bLight = true;
    }

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


