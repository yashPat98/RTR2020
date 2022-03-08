
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
        "#version 300 es"                                                                                                                         \
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
        "#version 300 es"                                         \
        "\n"                                                        \
        "precision highp float;"                                    \
        
        "in vec3 phong_ads_light;"                                  \
        "out vec4 frag_color;"                                      \
    
        "void main(void)"                                           \
        "{"                                                         \
        "   frag_color = vec4(phong_ads_light, 1.0f);"              \
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
    glEnable(GL_DEPTH_TEST);
    
    //set light and material variables
    light_ambient = vmath::vec3(0.0f, 0.0f, 0.0f);
    light_diffuse = vmath::vec3(1.0f, 1.0f, 1.0f);
    light_specular = vmath::vec3(1.0f, 1.0f, 1.0f);
    light_position = vmath::vec4(100.0f, 100.0f, 100.0f, 1.0f);

    material_ambient = vmath::vec3(0.0f, 0.0f, 0.0f);
    material_diffuse = vmath::vec3(1.0f, 1.0f, 1.0f);
    material_specular = vmath::vec3(1.0f, 1.0f, 1.0f);
    material_shininess = 50.0f;
    
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
    
    //code
    [EAGLContext setCurrentContext:eagl_context];
    glBindFramebuffer(GL_FRAMEBUFFER, default_framebuffer);
    glBindRenderbuffer(GL_RENDERBUFFER, color_renderbuffer);

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


