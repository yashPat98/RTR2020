//
//  MyView.m
//  window
//
//  Created by Akshay Patel on 9/24/21.
//

#import <OpenGLES/ES3/gl.h>
#import <OpenGLES/ES3/glext.h>
#import "GLESView.h"
#import <vector>
#include "vmath.h"

//type declarations
enum
{
    AMC_ATTRIBUTE_POSITION = 0,
    AMC_ATTRIBUTE_COLOR,
    AMC_ATTRIBUTE_NORMAL,
    AMC_ATTRIBUTE_TEXCOORD
};

float distance(float x1, float y1, float x2, float y2);

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
    vertexShaderObject = glCreateShader(GL_VERTEX_SHADER);
    
    const GLchar *vertexShaderSource =
        "#version 300 es"                                         \
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
        "#version 300 es"                             \
        "\n"                                            \
        "precision highp float;"                        \
        
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
    glEnable(GL_DEPTH_TEST);
    
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
    vmath::mat4 modelViewMatrix;
    vmath::mat4 modelViewProjectionMatrix;
    vmath::mat4 translateMatrix;

    //code
    [EAGLContext setCurrentContext:eagl_context];
    glBindFramebuffer(GL_FRAMEBUFFER, default_framebuffer);
    glBindRenderbuffer(GL_RENDERBUFFER, color_renderbuffer);

    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
        
    modelViewMatrix = vmath::mat4::identity();
    modelViewProjectionMatrix = vmath::mat4::identity();
    translateMatrix = vmath::mat4::identity();

    glUseProgram(shaderProgramObject);
        translateMatrix = vmath::translate(0.0f, 0.0f, -2.5f);
        modelViewMatrix = translateMatrix;
        modelViewProjectionMatrix = projectionMatrix * modelViewMatrix;

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

    [eagl_context presentRenderbuffer:color_renderbuffer];
    
    glBindRenderbuffer(GL_RENDERBUFFER, 0);
    glBindFramebuffer(GL_FRAMEBUFFER, 0);
}

- (void) uninitialize
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

float distance(float x1, float y1, float x2, float y2)
{
    //code
    float result = ((x2 - x1) * (x2 - x1)) + ((y2 - y1) * (y2 - y1));
    return ((float)sqrt(result));
}
